/***
;********************************************************************************
;*** File           : cl_inline.h
;*** Creation       : Wed Mar 11 09:58:09 CST 2009
;*** XSLT Processor : Apache Software Foundation / http://xml.apache.org/xalan-j / supports XSLT-Ver: 1
;*** Copyright      : (c) Texas Instruments AG, Berlin Germany 2002
;********************************************************************************
;*** Document Type  : Service Access Point Specification
;*** Document Name  : cl
;*** Document No.   : 8010.149.04.012
;*** Document Date  : 2004-06-08
;*** Document Status: SUBMITTED
;*** Document Author: rpk
;********************************************************************************
;*** !!! THIS INCLUDE FILE WAS GENERATED AUTOMATICALLY, DO NOT MODIFY !!!
;********************************************************************************
 ***/
#ifndef _CL_INLINE_H_
#define _CL_INLINE_H_



extern  void cl_nwrl_set_sgsn_release ( U8 sgsn_rel );

extern  U8 cl_nwrl_get_sgsn_release ( void );

extern  U8 cl_qos_convert_r99_to_r97 ( T_PS_qos_r99 *src_qos_r99, T_PS_qos_r97 *dst_qos_r97 );

extern  U8 cl_qos_convert_r97_to_r99 ( T_PS_qos_r97 *src_qos_r97, T_PS_qos_r99 *dst_qos_r99 );




#endif /* !_CL_INLINE_H_ */
