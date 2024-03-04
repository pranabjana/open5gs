/*
 * Generated by asn1c-0.9.29 (http://lionet.info/asn1c)
 * From ASN.1 module "SBC-AP-IEs"
 * 	found in "../support/sbcap-r16/sbcap-r16.asn"
 * 	`asn1c -pdu=all -fcompound-names -findirect-choice -fno-include-deps -no-gen-BER -no-gen-XER -no-gen-OER -no-gen-UPER -no-gen-JER`
 */

#ifndef	_SBCAP_Cause_H_
#define	_SBCAP_Cause_H_


#include <asn_application.h>

/* Including external dependencies */
#include <NativeInteger.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Dependencies */
typedef enum SBCAP_Cause {
	SBCAP_Cause_message_accepted	= 0,
	SBCAP_Cause_parameter_not_recognised	= 1,
	SBCAP_Cause_parameter_value_invalid	= 2,
	SBCAP_Cause_valid_message_not_identified	= 3,
	SBCAP_Cause_tracking_area_not_valid	= 4,
	SBCAP_Cause_unrecognised_message	= 5,
	SBCAP_Cause_missing_mandatory_element	= 6,
	SBCAP_Cause_mME_capacity_exceeded	= 7,
	SBCAP_Cause_mME_memory_exceeded	= 8,
	SBCAP_Cause_warning_broadcast_not_supported	= 9,
	SBCAP_Cause_warning_broadcast_not_operational	= 10,
	SBCAP_Cause_message_reference_already_used	= 11,
	SBCAP_Cause_unspecifed_error	= 12,
	SBCAP_Cause_transfer_syntax_error	= 13,
	SBCAP_Cause_semantic_error	= 14,
	SBCAP_Cause_message_not_compatible_with_receiver_state	= 15,
	SBCAP_Cause_abstract_syntax_error_reject	= 16,
	SBCAP_Cause_abstract_syntax_error_ignore_and_notify	= 17,
	SBCAP_Cause_abstract_syntax_error_falsely_constructed_message	= 18
} e_SBCAP_Cause;

/* SBCAP_Cause */
typedef long	 SBCAP_Cause_t;

/* Implementation */
extern asn_per_constraints_t asn_PER_type_SBCAP_Cause_constr_1;
extern asn_TYPE_descriptor_t asn_DEF_SBCAP_Cause;
asn_struct_free_f SBCAP_Cause_free;
asn_struct_print_f SBCAP_Cause_print;
asn_constr_check_f SBCAP_Cause_constraint;
per_type_decoder_f SBCAP_Cause_decode_aper;
per_type_encoder_f SBCAP_Cause_encode_aper;

#ifdef __cplusplus
}
#endif

#endif	/* _SBCAP_Cause_H_ */
#include <asn_internal.h>