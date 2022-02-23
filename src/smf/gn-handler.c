/*
 * Copyright (C) 2019 by Sukchan Lee <acetcom@gmail.com>
 * Copyright (C) 2022 by sysmocom - s.f.m.c. GmbH <info@sysmocom.de>
 *
 * This file is part of Open5GS.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include "event.h"
#include "context.h"
#include "gtp-path.h"
#include "fd-path.h"
#include "gn-build.h"
#include "gn-handler.h"
#include "pfcp-path.h"

#include "ipfw/ipfw2.h"

void smf_gn_handle_echo_request(
        ogs_gtp_xact_t *xact, ogs_gtp1_echo_request_t *req)
{
    ogs_assert(xact);
    ogs_assert(req);

    ogs_debug("[PGW] Receiving Echo Request");
    /* FIXME : Implementing recovery counter correctly */
    ogs_gtp1_send_echo_response(xact, 0);
}

void smf_gn_handle_echo_response(
        ogs_gtp_xact_t *xact, ogs_gtp1_echo_response_t *req)
{
    /* Not Implemented */
}

void smf_gn_handle_create_pdp_context_request(
        smf_sess_t *sess, ogs_gtp_xact_t *xact,
        ogs_gtp1_create_pdp_context_request_t *req)
{
    char buf1[OGS_ADDRSTRLEN];
    char buf2[OGS_ADDRSTRLEN];

    int rv;
    uint8_t cause_value = 0;

    ogs_gtp1_uli_t uli;

    smf_ue_t *smf_ue = NULL;
    ogs_eua_t *eua = NULL;
    smf_bearer_t *bearer = NULL;
    ogs_gtp1_qos_profile_decoded_t qos_pdec;
    uint8_t qci = 9;

    ogs_assert(xact);
    ogs_assert(req);

    ogs_debug("Create PDP Context Request");

    cause_value = OGS_GTP1_CAUSE_REQUEST_ACCEPTED;

    if (req->imsi.presence == 0) {
        ogs_error("No IMSI");
        cause_value = OGS_GTP1_CAUSE_MANDATORY_IE_MISSING;
    }
    if (req->tunnel_endpoint_identifier_data_i.presence == 0) {
        ogs_error("No TEID");
        cause_value = OGS_GTP1_CAUSE_MANDATORY_IE_MISSING;
    }
    if (req->nsapi.presence == 0) {
        ogs_error("No NSAPI");
        cause_value = OGS_GTP1_CAUSE_MANDATORY_IE_MISSING;
    }
    if (req->sgsn_address_for_signalling.presence == 0) {
        ogs_error("SGSN Address for signalling");
        cause_value = OGS_GTP1_CAUSE_MANDATORY_IE_MISSING;
    }
    if (req->sgsn_address_for_user_traffic.presence == 0) {
        ogs_error("SGSN Address for user traffic");
        cause_value = OGS_GTP1_CAUSE_MANDATORY_IE_MISSING;
    }
    if (req->quality_of_service_profile.presence == 0) {
        ogs_error("No QoS Profile");
        cause_value = OGS_GTP1_CAUSE_MANDATORY_IE_MISSING;
    }
    if (req->user_location_information.presence == 0) {
        ogs_error("No User Location Info");
        cause_value = OGS_GTP1_CAUSE_MANDATORY_IE_MISSING;
    }

    if (!sess) {
        ogs_error("No Context");
        cause_value = OGS_GTP1_CAUSE_CONTEXT_NOT_FOUND;
    } else {
        if (!ogs_diam_app_connected(OGS_DIAM_GX_APPLICATION_ID)) {
            ogs_error("No Gx Diameter Peer");
            cause_value = OGS_GTP1_CAUSE_NO_RESOURCES_AVAILABLE;
        }
    }

    if (cause_value != OGS_GTP1_CAUSE_REQUEST_ACCEPTED) {
        ogs_gtp1_send_error_message(xact, sess ? sess->sgw_s5c_teid : 0,
                OGS_GTP1_CREATE_PDP_CONTEXT_RESPONSE_TYPE, cause_value);
        return;
    }

    ogs_assert(sess);
    smf_ue = sess->smf_ue;
    ogs_assert(smf_ue);

    /* Store NSAPI */
    sess->gtp1.nsapi = req->nsapi.u8;

    if (ogs_gtp1_parse_uli(&uli, &req->user_location_information) == 0) {
        ogs_gtp1_send_error_message(xact, sess->sgw_s5c_teid,
                OGS_GTP1_CREATE_PDP_CONTEXT_RESPONSE_TYPE,
                OGS_GTP1_CAUSE_MANDATORY_IE_INCORRECT);
        return;
    }
    /* TODO: Copy uli->cgi/sai/rai into sess-> */
    switch (uli.geo_loc_type) {
    case OGS_GTP1_GEO_LOC_TYPE_CGI:
        ogs_nas_to_plmn_id(&sess->plmn_id, &uli.cgi.nas_plmn_id);
        break;
    case OGS_GTP1_GEO_LOC_TYPE_SAI:
        ogs_nas_to_plmn_id(&sess->plmn_id, &uli.sai.nas_plmn_id);
        break;
    case  OGS_GTP1_GEO_LOC_TYPE_RAI:
        ogs_nas_to_plmn_id(&sess->plmn_id, &uli.rai.nas_plmn_id);
        break;
    }

    /* Select PGW based on UE Location Information */
    smf_sess_select_upf(sess);

    /* Check if selected PGW is associated with SMF */
    ogs_assert(sess->pfcp_node);
    if (!OGS_FSM_CHECK(&sess->pfcp_node->sm, smf_pfcp_state_associated)) {
        ogs_gtp1_send_error_message(xact, sess->sgw_s5c_teid,
                OGS_GTP1_CREATE_PDP_CONTEXT_RESPONSE_TYPE,
                OGS_GTP1_CAUSE_NO_RESOURCES_AVAILABLE);
        return;
    }

    /* UE IP Address */
    eua = req->end_user_address.data;
    ogs_assert(eua);
    rv = ogs_gtp1_eua_to_ip(eua, req->end_user_address.len, &sess->session.ue_ip,
            &sess->session.session_type);
    if(rv != OGS_OK) {
        ogs_gtp1_send_error_message(xact, sess->sgw_s5c_teid,
                OGS_GTP1_CREATE_PDP_CONTEXT_RESPONSE_TYPE,
                OGS_GTP1_CAUSE_MANDATORY_IE_INCORRECT);
        return;
    }


    ogs_assert(OGS_PFCP_CAUSE_REQUEST_ACCEPTED == smf_sess_set_ue_ip(sess));

    ogs_info("UE IMSI[%s] APN[%s] IPv4[%s] IPv6[%s]",
	    smf_ue->imsi_bcd,
	    sess->session.name,
        sess->ipv4 ? OGS_INET_NTOP(&sess->ipv4->addr, buf1) : "",
        sess->ipv6 ? OGS_INET6_NTOP(&sess->ipv6->addr, buf2) : "");

    /* Remove all previous bearer */
    smf_bearer_remove_all(sess);

    /* Setup Default Bearer */
    bearer = smf_bearer_add(sess);
    ogs_assert(bearer);

    /* Set Bearer EBI */
    /* 3GPP TS 23.060 clause 9.2.1A: "1:1 mapping between NSAPI and EPS Bearer ID" */
    /* 3GPP TS 23.401 clause 5.2.1: "the same identity value is used for the EPS bearer ID and the NSAPI/RAB ID" */
    bearer->ebi = req->nsapi.u8;

    /* Control Plane(DL) : SGW-S5C */
    sess->sgw_s5c_teid = req->tunnel_endpoint_identifier_control_plane.u32;
    rv = ogs_gtp1_gsn_addr_to_ip(req->sgsn_address_for_signalling.data,
                                 req->sgsn_address_for_signalling.len,
                                  &sess->sgw_s5c_ip);
    ogs_assert(rv == OGS_OK);
    ogs_debug("    SGW_S5C_TEID[0x%x] SMF_N4_TEID[0x%x]",
            sess->sgw_s5c_teid, sess->smf_n4_teid);

    /* User Plane(DL) : SGW-S5C */
    bearer->sgw_s5u_teid = req->tunnel_endpoint_identifier_data_i.u32;
    rv = ogs_gtp1_gsn_addr_to_ip(req->sgsn_address_for_user_traffic.data,
                                 req->sgsn_address_for_user_traffic.len,
                                 &bearer->sgw_s5u_ip);
    ogs_assert(rv == OGS_OK);
    ogs_debug("    SGW_S5U_TEID[0x%x] PGW_S5U_TEID[0x%x]",
            bearer->sgw_s5u_teid, bearer->pgw_s5u_teid);

    /* Set Bearer QoS */
    rv = ogs_gtp1_parse_qos_profile(&qos_pdec,
        &req->quality_of_service_profile);
    if(rv < 0) {
        ogs_gtp1_send_error_message(xact, sess->sgw_s5c_teid,
                OGS_GTP1_CREATE_PDP_CONTEXT_RESPONSE_TYPE,
                OGS_GTP1_CAUSE_MANDATORY_IE_INCORRECT);
        return;
    }

    /* 3GPP TS 23.060 section 9.2.1A: "The QoS profiles of the PDP context and EPS bearer are mapped as specified in TS 23.401"
     * 3GPP TS 23.401 Annex E: "Mapping between EPS and Release 99 QoS parameters"
     */
    ogs_gtp1_qos_profile_to_qci(&qos_pdec, &qci);
    sess->session.qos.index = qci;
    sess->session.qos.arp.priority_level = qos_pdec.qos_profile.arp; /* 3GPP TS 23.401 Annex E Table E.2 */
    sess->session.qos.arp.pre_emption_capability = 0; /* ignored as per 3GPP TS 23.401 Annex E */
    sess->session.qos.arp.pre_emption_vulnerability = 0; /* ignored as per 3GPP TS 23.401 Annex E */
    if (qos_pdec.data_octet6_to_13_present) {
        sess->session.ambr.downlink = qos_pdec.dec_mbr_kbps_dl * 1000;
        sess->session.ambr.uplink = qos_pdec.dec_mbr_kbps_ul * 1000;
    } else {
        /* Set some sane default if infomation not present in Qos Profile IE: */
        sess->session.ambr.downlink = 102400000;
        sess->session.ambr.uplink = 102400000;
    }

    /* PCO */
    if (req->protocol_configuration_options.presence) {
        OGS_TLV_STORE_DATA(&sess->gtp.ue_pco,
                &req->protocol_configuration_options);
    }

#if 0
    /* Set User Location Information */
    /* TODO: the IE is probably different between GTPv1C and GTPv2, see what needs to be adapted */
    if (req->user_location_information.presence) {
        OGS_TLV_STORE_DATA(&sess->gtp.user_location_information,
                &req->user_location_information);
    }
#endif

    /* Set UE Timezone */
    if (req->ms_time_zone.presence) {
        /* value part is compatible between UE Time Zone and MS Time Zone */
        OGS_TLV_STORE_DATA(&sess->gtp.ue_timezone, &req->ms_time_zone);
    }

    smf_gx_send_ccr(sess, xact,
        OGS_DIAM_GX_CC_REQUEST_TYPE_INITIAL_REQUEST);
}

void smf_gn_handle_delete_pdp_context_request(
        smf_sess_t *sess, ogs_gtp_xact_t *xact,
        ogs_gtp1_delete_pdp_context_request_t *req)
{
    uint8_t cause_value = 0;

    ogs_debug("Delete PDP Context Request");

    ogs_assert(xact);
    ogs_assert(req);

    cause_value = OGS_GTP1_CAUSE_REQUEST_ACCEPTED;

    if (!sess) {
        ogs_warn("No Context");
        cause_value = OGS_GTP1_CAUSE_NON_EXISTENT;
    } else {
        if (!ogs_diam_app_connected(OGS_DIAM_GX_APPLICATION_ID)) {
            ogs_error("No Gx Diameter Peer");
            cause_value = OGS_GTP1_CAUSE_NO_RESOURCES_AVAILABLE;
        }
    }

    if (cause_value != OGS_GTP1_CAUSE_REQUEST_ACCEPTED) {
        ogs_gtp1_send_error_message(xact, sess ? sess->sgw_s5c_teid : 0,
                OGS_GTP1_DELETE_PDP_CONTEXT_RESPONSE_TYPE, cause_value);
        return;
    }

    ogs_debug("    SGW_S5C_TEID[0x%x] SMF_N4_TEID[0x%x]",
            sess->sgw_s5c_teid, sess->smf_n4_teid);
    smf_gx_send_ccr(sess, xact,
        OGS_DIAM_GX_CC_REQUEST_TYPE_TERMINATION_REQUEST);
}

void smf_gn_handle_update_pdp_context_request(
        smf_sess_t *sess, ogs_gtp_xact_t *xact,
        ogs_gtp1_update_pdp_context_request_t *req)
{
    int rv;
    uint8_t cause_value = OGS_GTP1_CAUSE_REQUEST_ACCEPTED;

    ogs_gtp1_header_t h;
    ogs_pkbuf_t *pkbuf = NULL;
    smf_bearer_t *bearer = NULL;
    smf_ue_t *smf_ue = NULL;

    ogs_debug("Update PDP Context Request");

    ogs_assert(xact);
    ogs_assert(req);

    if (req->nsapi.presence == 0) {
        ogs_error("No NSAPI");
        cause_value = OGS_GTP1_CAUSE_MANDATORY_IE_MISSING;
    }
    if (req->tunnel_endpoint_identifier_data_i.presence == 0) {
        ogs_error("No TEID");
        cause_value = OGS_GTP1_CAUSE_MANDATORY_IE_MISSING;
    }
    if (req->sgsn_address_for_control_plane.presence == 0) {
        ogs_error("SGSN Address for signalling");
        cause_value = OGS_GTP1_CAUSE_MANDATORY_IE_MISSING;
    }
    if (req->sgsn_address_for_user_traffic.presence == 0) {
        ogs_error("SGSN Address for user traffic");
        cause_value = OGS_GTP1_CAUSE_MANDATORY_IE_MISSING;
    }
    if (req->quality_of_service_profile.presence == 0) {
        ogs_error("No QoS Profile");
        cause_value = OGS_GTP1_CAUSE_MANDATORY_IE_MISSING;
    }

    if (!sess) {
        ogs_warn("No Context");
        cause_value = OGS_GTP1_CAUSE_NON_EXISTENT;
    }

    if (cause_value != OGS_GTP1_CAUSE_REQUEST_ACCEPTED) {
        ogs_gtp1_send_error_message(xact, sess ? sess->sgw_s5c_teid : 0,
                OGS_GTP1_UPDATE_PDP_CONTEXT_RESPONSE_TYPE, cause_value);
        return;
    }

    ogs_assert(sess);
    smf_ue = sess->smf_ue;
    ogs_assert(smf_ue);

    ogs_debug("    SGW_S5C_TEID[0x%x] SMF_N4_TEID[0x%x]",
            sess->sgw_s5c_teid, sess->smf_n4_teid);

    bearer = smf_bearer_find_by_ebi(sess, req->nsapi.u8);
    if (!bearer) {
        ogs_warn("No bearer with id %u, taking default", req->nsapi.u8);
        bearer = smf_default_bearer_in_sess(sess);
        if (!bearer) {
            ogs_warn("No bearer");
            ogs_gtp1_send_error_message(xact, sess->sgw_s5c_teid,
                    OGS_GTP1_UPDATE_PDP_CONTEXT_RESPONSE_TYPE,
                    OGS_GTP1_CAUSE_NON_EXISTENT);
            return;
        }
    }

    /* Control Plane(DL) : SGW-S5C */
    if (req->tunnel_endpoint_identifier_control_plane.presence) {
        sess->sgw_s5c_teid = req->tunnel_endpoint_identifier_control_plane.u32;
        rv = ogs_gtp1_gsn_addr_to_ip(req->sgsn_address_for_control_plane.data,
                                     req->sgsn_address_for_control_plane.len,
                                      &sess->sgw_s5c_ip);
        ogs_assert(rv == OGS_OK);
        ogs_debug("    Updated SGW_S5C_TEID[0x%x] SMF_N4_TEID[0x%x]",
                sess->sgw_s5c_teid, sess->smf_n4_teid);
    }

    /* User Plane(DL) : SGW-S5C */
    bearer->sgw_s5u_teid = req->tunnel_endpoint_identifier_data_i.u32;
    rv = ogs_gtp1_gsn_addr_to_ip(req->sgsn_address_for_user_traffic.data,
                                 req->sgsn_address_for_user_traffic.len,
                                 &bearer->sgw_s5u_ip);
    ogs_assert(rv == OGS_OK);
    ogs_debug("    Updated SGW_S5U_TEID[0x%x] PGW_S5U_TEID[0x%x]",
            bearer->sgw_s5u_teid, bearer->pgw_s5u_teid);
    /* FIXME: UPF needs to be updated through PFCP here, see:
     * https://github.com/open5gs/open5gs/issues/1367
     */

    memset(&h, 0, sizeof(ogs_gtp_header_t));
    h.type = OGS_GTP1_UPDATE_PDP_CONTEXT_RESPONSE_TYPE;
    h.teid = sess->sgw_s5c_teid;

    pkbuf = smf_gn_build_update_pdp_context_response(h.type, sess, bearer);
    ogs_expect_or_return(pkbuf);

    rv = ogs_gtp1_xact_update_tx(xact, &h, pkbuf);
    ogs_expect_or_return(rv == OGS_OK);

    rv = ogs_gtp_xact_commit(xact);
    ogs_expect(rv == OGS_OK);
}
