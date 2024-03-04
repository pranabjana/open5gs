/*
 * Generated by asn1c-0.9.29 (http://lionet.info/asn1c)
 * From ASN.1 module "SBC-AP-IEs"
 * 	found in "../support/sbcap-r16/sbcap-r16.asn"
 * 	`asn1c -pdu=all -fcompound-names -findirect-choice -fno-include-deps -no-gen-BER -no-gen-XER -no-gen-OER -no-gen-UPER -no-gen-JER`
 */

#ifndef	_SBCAP_Message_Identifier_H_
#define	_SBCAP_Message_Identifier_H_


#include <asn_application.h>

/* Including external dependencies */
#include <BIT_STRING.h>

#ifdef __cplusplus
extern "C" {
#endif

/* SBCAP_Message-Identifier */
typedef BIT_STRING_t	 SBCAP_Message_Identifier_t;

/* Implementation */
extern asn_per_constraints_t asn_PER_type_SBCAP_Message_Identifier_constr_1;
extern asn_TYPE_descriptor_t asn_DEF_SBCAP_Message_Identifier;
asn_struct_free_f SBCAP_Message_Identifier_free;
asn_struct_print_f SBCAP_Message_Identifier_print;
asn_constr_check_f SBCAP_Message_Identifier_constraint;
per_type_decoder_f SBCAP_Message_Identifier_decode_aper;
per_type_encoder_f SBCAP_Message_Identifier_encode_aper;

#ifdef __cplusplus
}
#endif

#endif	/* _SBCAP_Message_Identifier_H_ */
#include <asn_internal.h>