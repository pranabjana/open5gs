/*
 * Generated by asn1c-0.9.29 (http://lionet.info/asn1c)
 * From ASN.1 module "NGAP-IEs"
 * 	found in "../support/ngap-r17.3.0/38413-h30.asn"
 * 	`asn1c -pdu=all -fcompound-names -findirect-choice -fno-include-deps -no-gen-BER -no-gen-XER -no-gen-OER -no-gen-UPER -no-gen-JER`
 */

#ifndef	_NGAP_MulticastSessionDeactivationRequestTransfer_H_
#define	_NGAP_MulticastSessionDeactivationRequestTransfer_H_


#include <asn_application.h>

/* Including external dependencies */
#include "NGAP_MBS-SessionID.h"
#include <constr_SEQUENCE.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Forward declarations */
struct NGAP_ProtocolExtensionContainer;

/* NGAP_MulticastSessionDeactivationRequestTransfer */
typedef struct NGAP_MulticastSessionDeactivationRequestTransfer {
	NGAP_MBS_SessionID_t	 mBS_SessionID;
	struct NGAP_ProtocolExtensionContainer	*iE_Extensions;	/* OPTIONAL */
	/*
	 * This type is extensible,
	 * possible extensions are below.
	 */
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} NGAP_MulticastSessionDeactivationRequestTransfer_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_NGAP_MulticastSessionDeactivationRequestTransfer;

#ifdef __cplusplus
}
#endif

#endif	/* _NGAP_MulticastSessionDeactivationRequestTransfer_H_ */
#include <asn_internal.h>
