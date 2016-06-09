/*
+-----------------------------------------------------------------------------
|  Project :
|  Modul   :
+-----------------------------------------------------------------------------
|  Copyright 2002 Texas Instruments Berlin, AG
|                 All rights reserved.
|
|                 This file is confidential and a trade secret of Texas
|                 Instruments Berlin, AG
|                 The receipt of or possession of this file does not convey
|                 any rights to reproduce or disclose its contents or to
|                 manufacture, use, or sell anything it may describe, in
|                 whole, or in part, without the specific written consent of
|                 Texas Instruments Berlin, AG.
+-----------------------------------------------------------------------------
|  Purpose :  Encoding of Air Interface Messages. Send AIMs to DL.
+-----------------------------------------------------------------------------
*/

#ifndef RR_FORS_C
#define RR_FORS_C

#include "config.h"
#include "fixedconf.h"
#include "condat-features.h"

#define ENTITY_RR

/*==== INCLUDES ===================================================*/

#include <string.h>
#include <stdlib.h>
#include "typedefs.h"
#include "pcm.h"
#include "pconst.cdg"
#include "mconst.cdg"
#include "message.h"
#include "ccdapi.h"
#include "vsi.h"
#include "custom.h"
#include "gsm.h"
#include "prim.h"
#include "tok.h"
#include "rr.h"

/*==== EXPORT =====================================================*/

/*==== PRIVAT =====================================================*/

/*==== VARIABLES ==================================================*/

/*==== FUNCTIONS ==================================================*/

/*
 * -------------------------------------------------------------------
 * SIGNAL Processing functions
 * -------------------------------------------------------------------
 */

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : RR_FOR                     |
| STATE   : code                ROUTINE : for_dat_encode_send        |
+--------------------------------------------------------------------+

  PURPOSE : Encode an Air Interface Message usinf CCD and send it do DL.

*/

LOCAL void for_dat_encode_send (void *prim, T_sdu *sdu)
{
  U8 *payload;
  U16 length;

  if ( sdu NEQ NULL )
  {
    ccd_codeMsg (CCDENT_RR, UPLINK, (T_MSGBUF *)sdu, _decodedMsg, NOT_PRESENT_8BIT);

    sdu->buf[sdu->o_buf/8-1] = PD_RR_TI_0; /* RR Protocol Discriminator, Skip Indicator */
    sdu->l_buf += 8;
    sdu->o_buf -= 8;

    payload = &(sdu->buf[0]);     /* Beginning of the buffer */
    payload += (sdu->o_buf) >> 3; /* Plus offset (bytes) */
    
    length = BYTELEN(sdu->l_buf); /* Length (bytes) */

    RR_BINDUMP (payload,length,ARFCN_NOT_PRESENT,
                FRAME_NUM_NOT_PRESENT,UPLINK);
  }

  PSENDX (DL, prim);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : RR_FOR                     |
| STATE   : code                ROUTINE : for_dat_data_req           |
+--------------------------------------------------------------------+

  PURPOSE : Encoding of a Layer 3 message and forwarding to Layer 2.

*/

GLOBAL void for_dat_data_req (T_DL_DATA_REQ *dl_data_req)
{
  TRACE_FUNCTION ("for_dat_data_req()");
  for_dat_encode_send ( dl_data_req, &dl_data_req->sdu );
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : RR_FOR                     |
| STATE   : code                ROUTINE : for_dat_est_req            |
+--------------------------------------------------------------------+

  PURPOSE : Start establishment for SMS on SAPI 3.

*/

GLOBAL void for_dat_est_req (T_DL_ESTABLISH_REQ *dl_establish_req)
{
  TRACE_FUNCTION ("for_dat_est_req()");
  dl_establish_req->sdu.l_buf = 0;
  dl_establish_req->sdu.o_buf = ENCODE_OFFSET;
  for_dat_encode_send ( dl_establish_req, NULL );
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : RR_FOR                     |
| STATE   : code                ROUTINE : for_dat_est_req_not_coding |
+--------------------------------------------------------------------+

  PURPOSE : Start a connection for an upper layer.

*/

GLOBAL void for_dat_est_req_not_coding (T_DL_ESTABLISH_REQ *dl_establish_req)
{
  TRACE_FUNCTION ("for_dat_est_req_not_coding()");
  for_dat_encode_send ( dl_establish_req, NULL );
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : RR_FOR                     |
| STATE   : code                ROUTINE : for_dat_est_req_content    |
+--------------------------------------------------------------------+

  PURPOSE : Start a connection with a layer 3 message from RR.

*/

GLOBAL void for_dat_est_req_content (T_DL_ESTABLISH_REQ *dl_establish_req)
{
  TRACE_FUNCTION ("for_dat_est_req_content()");
  for_dat_encode_send ( dl_establish_req, &dl_establish_req->sdu );
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : RR_FOR                     |
| STATE   : code                ROUTINE : for_dat_l3_data_req        |
+--------------------------------------------------------------------+

  PURPOSE : send an upper layer message to layer 2.

*/

GLOBAL void for_dat_l3_data_req (T_DL_DATA_REQ *dl_data_req)
{
  TRACE_FUNCTION ("for_dat_l3_data_req()");
  for_dat_encode_send ( dl_data_req, NULL );
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : RR_FOR                     |
| STATE   : code                ROUTINE : for_dat_reconnect_req      |
+--------------------------------------------------------------------+

  PURPOSE : After a failed handover or channel assignment RR
            switches back to the old channel. This function starts
            the reconnection of the layer 2 link.

*/

GLOBAL void for_dat_reconnect_req (T_DL_RECONNECT_REQ *dl_reconnect_req)
{
  TRACE_FUNCTION ("for_dat_reconnect_req()");
  TRACE_EVENT ("DL_RECONNECT_REQ");
  for_dat_encode_send ( dl_reconnect_req, &dl_reconnect_req->sdu );
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : RR_FOR                     |
| STATE   : code                ROUTINE : for_dat_resume_req         |
+--------------------------------------------------------------------+

  PURPOSE : After reception of a handover command or assignment command
            message, layer 1 is configured to a new channel and the
            layer 2 link is resumed.

*/

GLOBAL void for_dat_resume_req (T_DL_RESUME_REQ *dl_resume_req)
{
  TRACE_FUNCTION ("for_dat_resume_req()");
  for_dat_encode_send ( dl_resume_req, &dl_resume_req->sdu );
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : RR_FOR                     |
| STATE   : code                ROUTINE : for_dat_unitdata_req       |
+--------------------------------------------------------------------+

  PURPOSE : Send an unacknowledged message on the uplink SACCH.
            This message is a measurement report message from RR.

*/

GLOBAL void for_dat_unitdata_req (T_DL_UNITDATA_REQ *dl_unitdata_req)
{
  TRACE_FUNCTION ("for_dat_unitdata_req()");
  for_dat_encode_send ( dl_unitdata_req, &dl_unitdata_req->sdu );
}

#if defined (REL99) && defined (TI_PS_FF_EMR)
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : RR_FOR                     |
| STATE   : code                ROUTINE : for_dat_spd_unitdata_req       |
+--------------------------------------------------------------------+

  PURPOSE : Send an unacknowledged message on the uplink SACCH with short PD.
            This is a enhanced measurement report message from RR.

*/

GLOBAL void for_dat_spd_unitdata_req (T_DL_SHORT_UNITDATA_REQ *p_msg)
{
  T_sdu     *sdu = &p_msg->sdu;
  TRACE_FUNCTION ("for_dat_spd_unitdata_req()");
  if ( sdu NEQ NULL )
  {
    ccd_codeMsg (CCDENT_RR_SHORT, UPLINK, (T_MSGBUF *)sdu, _decodedMsg, NOT_PRESENT_8BIT);    
  }
  p_msg->ch_type=L2_CHANNEL_SACCH;
  PSENDX (DL, p_msg);  
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : RR_FOR                     |
| STATE   : code                ROUTINE : for_dat_process_common_emr_data       |
+--------------------------------------------------------------------+

  PURPOSE : DAT process requests FOR for processing the common EMR
  parameters
*/

GLOBAL BOOL for_dat_process_common_emr_data (T_gprs_rep_prio *p_rep,
                                             T_gprs_bsic *p_bl,
                                             UBYTE  msg_index,
                                             BOOL ba_available)
{
  TRACE_FUNCTION ("for_dat_process_common_emr_data()");
  return(for_process_common_emr_data ( p_rep,p_bl,msg_index, ba_available));
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : RR_FOR                     |
| STATE   : code                ROUTINE : for_att_update_ba2bsic_mapping       |
+--------------------------------------------------------------------+

  PURPOSE : 
*/
GLOBAL void for_att_update_ba2bsic_mapping(T_rr_enh_para *p_enh)
{
  GET_INSTANCE_DATA;
  UBYTE             i;  
  USHORT            *p_list = rr_data->act_ncell_list;
  T_enh_cell_list   *p_enh_list = p_enh->enh_para.enh_cell_list;
  
  for (i = 0; i < p_enh->enh_para.num_valid_cells ; i++ )
  {
    if ( (((p_enh->ba2bsic_map_pending >> i) & 0x1) EQ TRUE) AND
         ( p_list[(UBYTE)p_enh_list[i].arfcn] NEQ NOT_PRESENT_16BIT ) )    
      p_enh_list[i].arfcn = p_list[(UBYTE)p_enh_list[i].arfcn];      
  }
  p_enh->ba2bsic_map_pending = 0;
  return;
}
#endif
#endif
