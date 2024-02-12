/*
 * Generated by asn1c-0.9.29 (http://lionet.info/asn1c)
 * From ASN.1 module "NGAP-IEs"
 * 	found in "../support/ngap-r17.3.0/38413-h30.asn"
 * 	`asn1c -pdu=all -fcompound-names -findirect-choice -fno-include-deps -no-gen-BER -no-gen-XER -no-gen-OER -no-gen-UPER -no-gen-JER`
 */

#ifndef	_NGAP_NGRAN_CellReportItem_H_
#define	_NGAP_NGRAN_CellReportItem_H_


#include <asn_application.h>

/* Including external dependencies */
#include "NGAP_NGRAN-CGI.h"
#include "NGAP_EUTRAN-CompositeAvailableCapacityGroup.h"
#include "NGAP_NGRAN-NumberOfActiveUEs.h"
#include "NGAP_NGRAN-NoofRRCConnections.h"
#include <constr_SEQUENCE.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Forward declarations */
struct NGAP_NGRAN_RadioResourceStatus;
struct NGAP_ProtocolExtensionContainer;

/* NGAP_NGRAN-CellReportItem */
typedef struct NGAP_NGRAN_CellReportItem {
	NGAP_NGRAN_CGI_t	 nGRAN_CGI;
	NGAP_EUTRAN_CompositeAvailableCapacityGroup_t	 nGRAN_CompositeAvailableCapacityGroup;
	NGAP_NGRAN_NumberOfActiveUEs_t	*nGRAN_NumberOfActiveUEs;	/* OPTIONAL */
	NGAP_NGRAN_NoofRRCConnections_t	*nGRAN_NoofRRCConnections;	/* OPTIONAL */
	struct NGAP_NGRAN_RadioResourceStatus	*nGRAN_RadioResourceStatus;	/* OPTIONAL */
	struct NGAP_ProtocolExtensionContainer	*iE_Extensions;	/* OPTIONAL */
	/*
	 * This type is extensible,
	 * possible extensions are below.
	 */
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} NGAP_NGRAN_CellReportItem_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_NGAP_NGRAN_CellReportItem;
extern asn_SEQUENCE_specifics_t asn_SPC_NGAP_NGRAN_CellReportItem_specs_1;
extern asn_TYPE_member_t asn_MBR_NGAP_NGRAN_CellReportItem_1[6];

#ifdef __cplusplus
}
#endif

#endif	/* _NGAP_NGRAN_CellReportItem_H_ */
#include <asn_internal.h>
