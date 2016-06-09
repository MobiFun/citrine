/***
;********************************************************************************
;*** File           : DIO_inline.h
;*** Creation       : Wed Mar 11 09:57:46 CST 2009
;*** XSLT Processor : Apache Software Foundation / http://xml.apache.org/xalan-j / supports XSLT-Ver: 1
;*** Copyright      : (c) Texas Instruments AG, Berlin Germany 2002
;********************************************************************************
;*** Document Type  : Service Access Point Specification
;*** Document Name  : DIO
;*** Document No.   : ...
;*** Document Date  : 2004-03-19
;*** Document Status: BEING_PROCESSED
;*** Document Author: RM
;********************************************************************************
;*** !!! THIS INCLUDE FILE WAS GENERATED AUTOMATICALLY, DO NOT MODIFY !!!
;********************************************************************************
 ***/
#ifndef _DIO_INLINE_H_
#define _DIO_INLINE_H_

#include "gdi.h"


extern  U16 dio_init ( void );

extern  U16 dio_user_init ( U32 user_name, U16 drv_handle, T_DRV_CB_FUNC signal_callback  );

extern  U16 dio_user_exit ( U32 user_name );

extern  void dio_exit ( void );

extern  U16 dio_set_rx_buffer ( U32 device, T_dio_buffer *buffer );

extern  U16 dio_read ( U32 device, T_DIO_CTRL *control_info, T_dio_buffer **buffer );

extern  U16 dio_write ( U32 device, T_DIO_CTRL *control_info, T_dio_buffer *buffer );

extern  U16 dio_get_tx_buffer ( U32 device, T_dio_buffer **buffer );

extern  U16 dio_clear ( U32 device );

extern  U16 dio_flush ( U32 device );

extern  U16 dio_get_capabilities ( U32 device, T_DIO_CAP **capabilities );

extern  U16 dio_set_config ( U32 device, T_DIO_DCB *dcb );

extern  U16 dio_get_config ( U32 device, T_DIO_DCB *dcb );

extern  U16 dio_close_device ( U32 device );




#endif /* !_DIO_INLINE_H_ */
