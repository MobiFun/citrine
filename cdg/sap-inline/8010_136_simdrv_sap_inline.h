/***
;********************************************************************************
;*** File           : 8010_136_simdrv_sap_inline.h
;*** Creation       : Wed Mar 11 09:57:50 CST 2009
;*** XSLT Processor : Apache Software Foundation / http://xml.apache.org/xalan-j / supports XSLT-Ver: 1
;*** Copyright      : (c) Texas Instruments AG, Berlin Germany 2002
;********************************************************************************
;*** Document Type  : Service Access Point Specification
;*** Document Name  : 8010_136_simdrv_sap
;*** Document No.   : 8010.136.03.009
;*** Document Date  : 2004-06-10
;*** Document Status: BEING_PROCESSED
;*** Document Author: FDU
;********************************************************************************
;*** !!! THIS INCLUDE FILE WAS GENERATED AUTOMATICALLY, DO NOT MODIFY !!!
;********************************************************************************
 ***/
#ifndef _8010_136_SIMDRV_SAP_INLINE_H_
#define _8010_136_SIMDRV_SAP_INLINE_H_



extern  void simdrv_poweroff ( U8 reader_id );

extern  U8 simdrv_reset ( U8 reader_id, U8 voltage_select );

extern  U16 simdrv_xch_apdu ( U8 reader_id, T_SIMDRV_cmd_header cmd_header, T_SIMDRV_data_info data_info, T_SIMDRV_result_info *result_info );

extern  void simdrv_register ( void ( *simdrv_insert )( T_SIMDRV_atr_string_info *atr_string_info, U8 config_requested, T_SIMDRV_config_characteristics *config_characteristics ), void ( *simdrv_remove )( void ) );

extern  void simdrv_insert ( T_SIMDRV_atr_string_info *atr_string_info, U8 config_requested, T_SIMDRV_config_characteristics *config_characteristics );

extern  void simdrv_remove ( void );




#endif /* !_8010_136_SIMDRV_SAP_INLINE_H_ */
