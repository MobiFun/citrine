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
|  Purpose :  Decoding of Air Interface Messages.
+-----------------------------------------------------------------------------
*/

#ifndef RR_FORC_C
#define RR_FORC_C

#include "config.h"
#include "fixedconf.h"
#include "condat-features.h"

#define ENTITY_RR

/*==== INCLUDES ===================================================*/

#include <string.h>
#include <stdlib.h>
#include <stddef.h>     /* offsetof */
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
#include "rr_em.h"

/*==== EXPORT =====================================================*/

/*==== PRIVAT =====================================================*/

/*==== VARIABLES ==================================================*/


/*==== FUNCTIONS ==================================================*/



LOCAL void for_unitdata_ind_sys_info_5_5bis(T_SI_TYPE si_type, T_MPH_UNITDATA_IND *mph_unitdata_ind,
                                            UBYTE *ba_index, BUF_neigh_cell_desc  *neigh_cell_desc,
                                            T_LIST *list,T_VOID_STRUCT *sys_info_5_5bis);

LOCAL void for_unitdata_ind_sys_info_2_2bis(T_SI_TYPE si_type,  T_MPH_UNITDATA_IND  *mph_unitdata_ind,
                                            BUF_neigh_cell_desc  *neigh_cell_desc,
                                            T_LIST *list,    T_VOID_STRUCT   *sys_info_2_2bis);

LOCAL void for_unitdata_ind_si3_si4(T_SI_TYPE si_type,  T_MPH_UNITDATA_IND  *mph_unitdata_ind,
                                    T_loc_area_ident *loc_area_ident,
                                    T_VOID_STRUCT   *sys_info_3_4);
/*
 * -------------------------------------------------------------------
 * PRIMITIVE Processing functions
 * -------------------------------------------------------------------
 */

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : RR_FOR                     |
| STATE   : code                ROUTINE : for_dl_data_ind            |
+--------------------------------------------------------------------+

  PURPOSE : Process the primitive DL_DATA_IND received from RR.

*/

GLOBAL void for_dl_data_ind (T_DL_DATA_IND *dl_data_ind_orig)
{
  GET_INSTANCE_DATA;
  UBYTE pd;
  UBYTE ti;
  UBYTE mt;
  U8 *payload;
  U16 length;

#if defined FF_EOTD
  UBYTE other_than_applic = 1;
#endif /* FF_EOTD */

  PPASS(dl_data_ind_orig, dl_data_ind, DL_DATA_IND);

  TRACE_FUNCTION ("for_dl_data_ind()");

  if (dl_data_ind->sdu.l_buf < 16)
  {
    /*
     * message too short, Ignore the message.
     */
    PFREE (dl_data_ind);
    return;
  }
  /*
   * get protocol discriminator and transaction identifier
   * from the message.
   */
  GET_PD (dl_data_ind->sdu, pd);
  GET_TI (dl_data_ind->sdu, ti);

  /*
   * check the protocol discriminator.
   */
  switch (pd)
  {
    case PD_TST:
      /*
       * Test messages are handled in RR.
       */
      if (ti NEQ 0)
      {
        /*
         * The skip-indicator (equal to transaction identifier in higher layer)
         * must be set to 0, else ignore the message.
         */
        PFREE (dl_data_ind);
        break;
      }

      /*
       * For test messages the handling is directly coded in RR without CCD.
       * Get the message type:
       */
      ccd_decodeByte (dl_data_ind->sdu.buf, (USHORT)(dl_data_ind->sdu.o_buf+8),8,
                      &mt);

      switch (mt)
      {
        case OPEN_LOOP_CMD:
          /*
           * Open a TCH loop
           */
          dat_for_open_loop_cmd (dl_data_ind);
          break;

        case CLOSE_TCH_LOOP_CMD:
          /*
           * Close a TCH loop
           */
          dat_for_close_loop_cmd (dl_data_ind, dl_data_ind->sdu.buf[
                                  (dl_data_ind->sdu.o_buf+16)>>3]);
          break;

        case TEST_INTERFACE:
          /*
           * Handling of the test interface message for DAI purposes.
           */
          dat_for_test_interface (dl_data_ind, dl_data_ind->sdu.buf[
                                  (dl_data_ind->sdu.o_buf+16)>>3]);
          break;

        default:
          /*
           * Unknown message type, Ignore the message.
           */
          PFREE (dl_data_ind);
          break;
      }
      break;

    case PD_RR:
      /*
       * RR Messages
       */
      if (ti NEQ 0)
      {
        /*
         * The skip-indicator (equal to transaction identifier in higher layer)
         * must be set to 0, else ignore the message.
         */
        PFREE (dl_data_ind);
        break;
      }

      payload = &(dl_data_ind->sdu.buf[0]);     /*Beginning of the buffer */
      payload += (dl_data_ind->sdu.o_buf) >> 3; /* Plus offset (bytes) */

      length = BYTELEN ( dl_data_ind->sdu.l_buf ); /* Length (bytes) */
      
  
      /*
       * modify length and offset of the incoming message toskip protocol
       * discriminator and transaction identifier. CCD starts with the
       * message type.
       */

      dl_data_ind->sdu.l_buf -= 8;
      dl_data_ind->sdu.o_buf += 8;

      /*
       * clean the error field and the output of CCD.
       */
      memset (&rr_data->ms_data.error, 0, sizeof (T_ERROR));
      memset (_decodedMsg, 0, MAX_MSTRUCT_LEN_RR);

      /*
       * decode the message with CCD.
       */ 
      if (ccd_decodeMsg (CCDENT_RR, DOWNLINK,
                        (T_MSGBUF *)&dl_data_ind->sdu,
                         _decodedMsg, NOT_PRESENT_8BIT) NEQ ccdOK)
      {
        /*
         * CCD has detected an error
         */
        USHORT parlist[6];
        UBYTE  ccd_err;

        /*
         * get the first detected error from CCD.
         */
        memset (parlist,0, sizeof (parlist));
        ccd_err = ccd_getFirstError (CCDENT_RR, parlist);

        do
        {
          /*
           * Error Handling
           */
          switch (ccd_err)
          {
            case ERR_MSG_LEN:             /* some exceeds entire message length */
            case ERR_COMPREH_REQUIRED:    /* Comprehension required             */
            case ERR_MAND_ELEM_MISS:      /* Mandatory elements missing         */
              if((_decodedMsg[0] EQ D_CHAN_REL ) AND (ccd_err EQ ERR_MAND_ELEM_MISS) )
              {
                 break;                   /* actions same as normal release     */ 
              } 
              else
              {
                 dat_send_rr_status_msg (RRC_INVALID_MAN_INFO);
                 PFREE (dl_data_ind);
                 return;
              }

            case ERR_INVALID_MID:         /* unknown message type is handled below */
              break;
            case ERR_IE_NOT_EXPECTED:
            case ERR_IE_SEQUENCE:
            case ERR_MAX_IE_EXCEED:
            case ERR_MAX_REPEAT:
              TRACE_EVENT_P1("CCD error=%d ignored here", ccd_err);
              break;
            default:
              TRACE_ERROR("unknow CCD return");
              break;
          }
          ccd_err = ccd_getNextError (CCDENT_RR, parlist);
        }
        while (ccd_err NEQ ERR_NO_MORE_ERROR);
      }

      /*
       * depending on the message type
       */

      RR_BINDUMP (payload,length,ARFCN_NOT_PRESENT,
                  FRAME_NUM_NOT_PRESENT,DOWNLINK);

      switch (_decodedMsg[0])
      {
        case D_ASSIGN_CMD:
          /*
           * Assignment command message to start an intracell
           * handover. Check the message content and configure layer 1.
           */
          for_check_assign_cmd (dl_data_ind, (T_D_ASSIGN_CMD *)_decodedMsg);
          break;

        case D_CHAN_REL:
        {
          /*
           * channel release message to release a RR-connection.
           */
          MCAST (chan_rel, D_CHAN_REL);

          if (chan_rel->v_ba_range)
          {
            /*
             * if the optional information element BA_RANGE is
             * included check the frequencies.
             */
            if (! for_check_ba_range (&chan_rel->ba_range))
            {
              /*
               * set the optional element as not available if
               * it contains a non-supported channel number.
               */
              chan_rel->v_ba_range = FALSE;
            }
          }
           
          /*
           * start processing of the channel release message.
           */
          dat_for_chan_rel (dl_data_ind, chan_rel);
          break;
        }

        case D_CHAN_MOD:
        {
          /*
           * channel mode modify message to handle a
           * changed channel mode.
           */
          MCAST (chmod, D_CHAN_MOD); /* T_D_CHAN_MOD */

          switch( chmod->chan_desc.chan_type )
          {
            case TCH_H_S0:
            case TCH_H_S1:
              switch(chmod->chan_mode)
              {
                case CM_DATA_12_0:   /* data 12  k         */
                case CM_DATA_6_0:    /* data 6   k         */
                case CM_DATA_3_6:    /* data 3.6 k         */
                case CM_DATA_14_4:   /* data 14.4 k        */
                  if(FldGet(rr_data->mscap.datCap2, DHRSup) EQ 0)
                  {
                    for_set_content_error (RRC_CHANNEL_MODE);
                    TRACE_EVENT("Half Rate Data NOT supported");
                  }
                  break;
                default:
                  break;
              }
              break;
            default:
              break;
          }

          /*
           * check the channel description
           */
          for_check_channel_descr (&chmod->chan_desc);

          /*
           * check the channel mode
           */
          for_check_channel_mode (chmod->chan_mode);

          /*
           *  if there any problem in channel mode or channel description 
           *  the CHANNEL MODE ACK message will be sent with old channel type/mode
           */
          if(rr_data->ms_data.error.val EQ RRC_CHANNEL_MODE) 
          {
            dat_for_chan_mod (dl_data_ind, chmod);
            break;
          }

          /*
           * if the message contains a channel mode information element which indicating AMR
           * and the multirate configuration IEI exists, check the multirate configuration IEI.
           */

          if ( (chmod->chan_mode EQ CM_AMR) AND (chmod->v_multirate_conf) )
          {
            /*
             * From 3GPP TS 04.08 
             *
             * "Channel Description IE" in Channel Mode Modify
             * 
             * This is sufficient to identify the channel in the case of a TCH/H + TCH/H configuration. 
             * If used for a multislot configuration, the IE shall describe the present channel configuration with 
             * TN indicating the main channel. 
             *
             * The IE shall not indicate a new channel configuration when included in the Channel Mode Modify message.
             */
            for_check_multirate_conf( &chmod->multirate_conf, rr_data->sc_data.chan_desc.chan_type);
          }

          /* 
           * From 3GPP TS 04.18  Sec. 3.4.6.1.3 Abnormal cases
           * If any inconsistencies in MultiRate IEs then ignore the Channel Mode Modify 
           *   and shall not send the CHANNEL MODE MODIFY 
           */
          if(rr_data->ms_data.error.val EQ RRC_CHANNEL_MODE)
          {
            PFREE(dl_data_ind);
            break;
          }

          /*
           * process the message
           * There is no error in Channel Type, Channel Descr and no inconsistency in Multirate IEs 
           */

          dat_for_chan_mod (dl_data_ind, chmod);
          break;
        }

        case D_CIPH_CMD:
        {
          /*
           * change the cipher mode setting
           */

          MCAST (ciph_cmd, D_CIPH_CMD); /* T_D_CIPH_CMD */

          /*
           * check the message content
           */
          for_check_cipher_mode_set (&ciph_cmd->ciph_mode_set);

          /*
           * process the message
           */

          dat_for_ciph_cmd (dl_data_ind, ciph_cmd);
          break;
        }

        case D_FREQ_REDEF:
        {
          /*
           * frequency redefinition message to change the frequency
           * hopping list during a connection.
           */

          /*lint -e813*/
          T_LIST cell_chan_desc;

          MCAST (freq_redef, D_FREQ_REDEF); /* T_D_FREQ_REDEF */

          /*
           * check the channel description
           */
          for_check_channel_descr (&freq_redef->chan_desc);

          if (freq_redef->v_cell_chan_desc)
          {
            /*
             * If the message contains a new cell channel description
             * build a channel number list. This new list will replace
             * the current list stored in RR.
             */
            for_create_channel_list ((T_f_range *)&freq_redef->cell_chan_desc,
                                     &cell_chan_desc);
          }


          /*
           * process the message.
           */
          dat_for_freq_redef (dl_data_ind, freq_redef, &cell_chan_desc);
          break;
        }

        case D_HANDOV_CMD:
          /*
           * check the message content and process the handover message.
           */

          for_check_handov_cmd (dl_data_ind, (T_D_HANDOV_CMD *)_decodedMsg);
          break;

        case B_RR_STATUS:
          /*
           * incoming status messages are ignored.
           */

          EM_RR_STATUS_RECEIVED;

          PFREE (dl_data_ind);
          break;

        case D_CLASS_ENQ:
          /*
           * the network requests the mobile station classmark.
           */
#ifdef REL99
          dat_for_class_enq (dl_data_ind, (T_D_CLASS_ENQ *)_decodedMsg);
#else
          dat_for_class_enq (dl_data_ind);
#endif
          break;

#ifdef GPRS
        case D_PDCH_ASS_CMD:
          dl_data_ind->sdu.l_buf += 8;
          dl_data_ind->sdu.o_buf -= 8;
          rr_data->gprs_data.tbf_est = TBF_EST_NONE;
          dat_rrgrr_data_ind (dl_data_ind);
          break;

        case D_CHANGE_ORDER:
          dl_data_ind->sdu.l_buf += 8;
          dl_data_ind->sdu.o_buf -= 8;
          rr_data->gprs_data.tbf_est = TBF_EST_NONE;
          dat_rrgrr_change_order (dl_data_ind, (T_D_CHANGE_ORDER *)_decodedMsg);
          break;
#endif

#if defined FF_EOTD
        case B_APPLIC_INFO:
        {
          MCAST (b_applic_info, B_APPLIC_INFO);
          dat_for_applic_info (b_applic_info);
          other_than_applic = 0;
          PFREE(dl_data_ind);
          break;
        }
#endif /* FF_EOTD */

        default: /* switch (_decodedMsg[0]) */
          /*
           * Unknown or not supported message
           * Answer with a RR STATUS message.
           */
          dat_send_rr_status_msg (RRC_MSG_NOT_IMPL);

          PFREE (dl_data_ind);
          break;
      }
      break;

    default: /* switch (pd) */
      /*
       * all other protocol discriminators are for upper layer
       * and will be forwarded to MM.
       */
      dat_for_l3_data_ind (dl_data_ind);
      break;
  }

#if defined FF_EOTD
  if ( other_than_applic AND rr_data->applic_rx.state EQ SAI_SEGM )
    rr_applic_rx_init ( &rr_data->applic_rx );
#endif /* FF_EOTD */
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : RR_FOR                     |
| STATE   : code                ROUTINE : for_mph_unitdata_ind       |
+--------------------------------------------------------------------+

  PURPOSE : Reception of unacknowledged message on BCCH, AGCH or PCH.

*/

GLOBAL void for_mph_unitdata_ind (T_MPH_UNITDATA_IND *mph_unitdata_ind_orig)
{
  GET_INSTANCE_DATA;
  T_LIST list;  /* used for storing a neighbourcell list */
  UBYTE  pd;    /* Protocol discriminator */
  UBYTE  ti;    /* Transaction identifier (equal to skip indicator) */
  U8 *payload;
  U16 length;

  PPASS(mph_unitdata_ind_orig, mph_unitdata_ind, MPH_UNITDATA_IND);

  if (mph_unitdata_ind->sdu.l_buf < 16)
  {
    PFREE (mph_unitdata_ind); /* message too short ignore message */
    return;
  }
  /*
   * Extract the protocol discriminator and the
   * skip indicator from the unacknowledged message.
   */
  GET_UI_PD (mph_unitdata_ind->sdu, pd);
  GET_UI_TI (mph_unitdata_ind->sdu, ti);

  payload = &(mph_unitdata_ind->sdu.buf[0]);     /*Beginning of the buffer */
  payload += (mph_unitdata_ind->sdu.o_buf) >> 3; /*Plus offset (bytes) */

  length = BYTELEN ( mph_unitdata_ind->sdu.l_buf ); /*Length (Bytes)*/
  
  /*
   * Skip both the PD and TI in the message by modifying the pointer
   * in the sdu to the begin of the message type.
   */
  mph_unitdata_ind->sdu.l_buf -= 8;
  mph_unitdata_ind->sdu.o_buf += 8;

  /*
   * The protocol discriminator must be RR.
   * The skip indicator (TI) must be 0. (GSM 04.08, 10.3.1)
   */
  if (! (pd EQ PD_RR AND ti EQ 0))
  {
    PFREE (mph_unitdata_ind);
    return;
  }

  /*
   * Clear error field and CCD result field.
   */
  memset (&rr_data->ms_data.error,0, sizeof(rr_data->ms_data.error));
  memset (_decodedMsg,0, sizeof(_decodedMsg));

  /*
   * Decode the message with CCD.
   */
  if (ccd_decodeMsg (CCDENT_RR, DOWNLINK,
                     (T_MSGBUF *)&mph_unitdata_ind->sdu,
                     (UBYTE *)_decodedMsg,
                     NOT_PRESENT_8BIT) NEQ ccdOK)
  {
    /*
     * CCD has detected an error
     */
    UBYTE ccd_err;
    USHORT parlist [6];

    /*
     * get the first error
     */
    ccd_err = ccd_getFirstError (CCDENT_RR, parlist);
    do
    {
      /*
       * Error Handling
       */
      switch (ccd_err)
      {
        case ERR_INVALID_MID:         /* unknown message type                   */
        case ERR_MSG_LEN:             /* some IE exceeds entire message length  */
        case ERR_COMPREH_REQUIRED:    /* comprehension required                 */
        case ERR_MAND_ELEM_MISS:      /* Mandatory elements missing             */
          TRACE_EVENT_P2("message with mt=%d ignored due to CCD error=%d",
                         mph_unitdata_ind->sdu.buf[mph_unitdata_ind->sdu.o_buf>>3],
                         ccd_err);
          PFREE (mph_unitdata_ind);   /* in unacknowledged mode ignore message  */
          return;

        case ERR_IE_NOT_EXPECTED:
        case ERR_IE_SEQUENCE:
        case ERR_MAX_IE_EXCEED:
        case ERR_MAX_REPEAT:
          TRACE_EVENT_P1("CCD error=%d ignored here", ccd_err);
          break;
        default:
          TRACE_ERROR("unknown CCD return");
          break;
      }
      ccd_err = ccd_getNextError (CCDENT_RR, parlist);
    }
    while (ccd_err NEQ ERR_NO_MORE_ERROR);
  }

  RR_BINDUMP (payload,length,mph_unitdata_ind->arfcn,
              mph_unitdata_ind->fn,DOWNLINK);

  /*
   * depending on the message type
   */
  switch (_decodedMsg[0])
  {
    case D_SYS_INFO_1:
    {
      /*
       * system information type 1 message on BCCH
       */
      MCAST (sys_info_1, D_SYS_INFO_1); /* T_D_SYS_INFO_1 */

      /*
       * successfully received system info
       * decrements the BCCH error counter
       */
      if (rr_data->bcch_error)
        rr_data->bcch_error--;

      /*
       * create a cell channel description and store it
       * for later use together with frequency hopping
       */
      for_create_channel_list ((T_f_range *)&sys_info_1->cell_chan_desc,
                               &list);

      /*
       * Process the message
       */
      att_for_sysinfo_type1 (mph_unitdata_ind->arfcn, sys_info_1, &list);
      break;
    }

    case D_SYS_INFO_2:
    {
       /*
        * System information type 2 message on BCCH
        */
/* Implements RR Clone findings #13 */

      MCAST (sys_info_2, D_SYS_INFO_2); /* T_D_SYS_INFO_2 */

/* Implements RR Clone findings #13 */
      for_unitdata_ind_sys_info_2_2bis(SI_TYPE_2, mph_unitdata_ind,
                                       &sys_info_2->neigh_cell_desc,&list,(T_VOID_STRUCT  *)sys_info_2);

       break;
    }

    case D_SYS_INFO_2BIS:
    {
       /*
        * system information type 2bis message on BCCH
        */
/* Implements RR Clone findings #13 */

      MCAST (sys_info_2bis, D_SYS_INFO_2BIS); /* T_D_SYS_INFO_2BIS */


/* Implements RR Clone findings #13 */
      for_unitdata_ind_sys_info_2_2bis(SI_TYPE_2BIS, mph_unitdata_ind,
                                       &sys_info_2bis->neigh_cell_desc,&list,(T_VOID_STRUCT *)sys_info_2bis);
       break;
     }

     case D_SYS_INFO_2TER:
     {
       /*
        * system information type 2ter message on BCCH
        */

       MCAST (sys_info_2ter, D_SYS_INFO_2TER); /* T_D_SYS_INFO_2TER */

       /*
        * successfully received system info
        * decrements the BCCH error counter
        */
       if (rr_data->bcch_error)
         rr_data->bcch_error--;

       switch (std)
       {
         case STD_EGSM:
         case STD_DUAL:
         case STD_DUAL_EGSM:
         case STD_DUAL_US:
#ifdef TI_PS_FF_QUAD_BAND_SUPPORT
         case STD_850_1800:
         case STD_900_1900:
         case STD_850_900_1800:
         case STD_850_900_1900:
#endif
           /*
            * only for extended GSM and dualband
            *
            * extract the multiband parameter
            */
           rr_data->ncell_mb  = (sys_info_2ter->neigh_cell_desc.b_neigh_cell_desc
                                [sys_info_2ter->neigh_cell_desc.o_neigh_cell_desc>>3]
                                 & 0x60) >> 5;

           /*
            * create a neighbourcell list
            */
           for_create_channel_list ((T_f_range *)&sys_info_2ter->neigh_cell_desc,
                                    &list);

           /*
            * process the message in the attachment process
            */
           att_for_sysinfo_type2ter (mph_unitdata_ind->arfcn, sys_info_2ter, &list);
           break;

         default:
           break;
       }
       break;
     }

#if defined (REL99) && defined (TI_PS_FF_EMR)
     case D_SYS_INFO_2QUATER:
     {
       /* Process and store only for serving cell..Since SI-2quater
          is required only for reporting*/
       UBYTE  index = att_get_index(mph_unitdata_ind->arfcn);

       if( index EQ SC_INDEX  )
       {
         MCAST (sys_info_2quater, D_SYS_INFO_2QUATER); /* T_D_SYS_INFO_2QUATER */
         if(for_process_si2quater(&sys_info_2quater->si_2qua_octets))
         {
           for_send_enh_para(&rr_data->sc_data.emr_data_current);
           /*we received all the instances successfully, we no longer need to monitor SI-2quater*/             
           for_mon_si2quater_req(STOP_MON_BCCH);
           rr_data->sc_data.cd.si2quater_status = SI2QUATER_ACQ_COMP;
         }
       }
       break;
     }
#endif

     case D_SYS_INFO_3:
     {
       /*
        * create system information type 3 message on BCCH
        */

       MCAST (sys_info_3, D_SYS_INFO_3); /* T_D_SYS_INFO_3 */

/* Implements RR Clone findings #16 */
       for_unitdata_ind_si3_si4(SI_TYPE_3, mph_unitdata_ind,
                                &sys_info_3->loc_area_ident,(T_VOID_STRUCT *)sys_info_3);
       
       break;
     }

     case D_SYS_INFO_4:
     {
       /*
        * system information type 4 message on BCCH
        */

       MCAST (sys_info_4, D_SYS_INFO_4); /* T_D_SYS_INFO_4 */

/* Implements RR Clone findings #16 */
       for_unitdata_ind_si3_si4(SI_TYPE_4, mph_unitdata_ind,
                                &sys_info_4->loc_area_ident,(T_VOID_STRUCT *)sys_info_4);
       break;
     }

     case D_SYS_INFO_5:
     {
       /*
        * system information type 5 message on SACCH
        */
/* Implements RR Clone findings #4 */

       MCAST (sys_info_5, D_SYS_INFO_5); /* T_D_SYS_INFO_5 */

/* Implements RR Clone findings #4 */
       for_unitdata_ind_sys_info_5_5bis(SI_TYPE_5, mph_unitdata_ind,&rr_data->sc_data.ba_index,
                                        &sys_info_5->neigh_cell_desc, &list,(T_VOID_STRUCT *)sys_info_5);
       break;
     }

     case D_SYS_INFO_5BIS:
     {
       /*
        * system information type 5bis message on SACCH
        */
/* Implements RR Clone findings #4 */

       MCAST (sys_info_5bis, D_SYS_INFO_5BIS); /* T_D_SYS_INFO_5BIS */

/* Implements RR Clone findings #4 */
       for_unitdata_ind_sys_info_5_5bis(SI_TYPE_5BIS, mph_unitdata_ind,&rr_data->sc_data.ba_index,
                                         &sys_info_5bis->neigh_cell_desc,&list,(T_VOID_STRUCT *)sys_info_5bis);
       break;
     }

     case D_SYS_INFO_5TER:
     {
       /*
        * system information type 5ter message on SACCH
        */

       MCAST (sys_info_5ter, D_SYS_INFO_5TER); /* T_D_SYS_INFO_5TER */

       switch (std)
       {
         case STD_EGSM:
         case STD_DUAL:
         case STD_DUAL_EGSM:
         case STD_DUAL_US:
#ifdef TI_PS_FF_QUAD_BAND_SUPPORT
         case STD_850_1800:
         case STD_900_1900:
         case STD_850_900_1800:
         case STD_850_900_1900:
#endif
           /*
            * only for extended GSM and Dualband
            *
            * dont use ba index from 5ter, because R&S sends inconsistent
            * BCCH data for multiband testcase 26.6.3.6
            *
            * extract multiband parameter
            */
           rr_data->ncell_mb  = (sys_info_5ter->neigh_cell_desc.b_neigh_cell_desc
                                 [sys_info_5ter->neigh_cell_desc.o_neigh_cell_desc>>3]
                                 & 0x60) >> 5;

           /*
            * create a neighbourcell list
            */
           for_create_channel_list ((T_f_range *)&sys_info_5ter->neigh_cell_desc,
                                    &list);

           /*
            * process the message in the attachment process
            */
           att_for_sysinfo_type5ter (mph_unitdata_ind->arfcn, &list);
           break;

         default:
           break;
       }
       break;
     }

     case D_SYS_INFO_6:
     {
       /*
        * system information type 6 message on SACCH
        */

       MCAST (sys_info_6, D_SYS_INFO_6); /* T_D_SYS_INFO_6 */

       if (sys_info_6->loc_area_ident.c_mnc EQ 2)
       {
         /* Internally G23 uses always 3-digit-MNC */
         sys_info_6->loc_area_ident.c_mnc = SIZE_MNC;
         sys_info_6->loc_area_ident.mnc[2] = 0xf;
       }

       /*
        * process the message in the attachment process
        */
       att_for_sysinfo_type6 (mph_unitdata_ind->arfcn, sys_info_6);
       break;
     }

     case D_SYS_INFO_7: /* T_D_SYS_INFO_7 */
     case D_SYS_INFO_8: /* T_D_SYS_INFO_8 */
       /*
        * successfully received system info
        * decrements the BCCH error counter
        */

       if (rr_data->bcch_error)
         rr_data->bcch_error--;

       /*
        * process the message in the attachment process
        */
       att_for_sysinfo_type7_8 (mph_unitdata_ind->arfcn, (T_D_SYS_INFO_8 *)_decodedMsg);
       break;
#ifdef GPRS
     case D_SYS_INFO_13: /* T_D_SYS_INFO_13 */
       {
         MCAST (sys_info_13, D_SYS_INFO_13);

         if (rr_data->bcch_error)
           rr_data->bcch_error--;

         att_for_sysinfo_type13 (mph_unitdata_ind, sys_info_13);
       }
      break;
#endif

     case D_IMM_ASSIGN:
     {
       /*
        * immediate assignment message on AGCH
        */

       MCAST (imm_assign, D_IMM_ASSIGN); /* T_D_IMM_ASSIGN */

       /*
        * process the message
        */
       dat_for_imm_assign (mph_unitdata_ind, imm_assign);
       break;
     }

     case D_IMM_ASSIGN_EXT:
     {
       /*
        * immediate assignment extended message on AGCH
        */

       MCAST (imm_assign_ext, D_IMM_ASSIGN_EXT); /* T_D_IMM_ASSIGN_EXT */

       /*
        * process the message.
        */
       dat_for_imm_assign_ext (mph_unitdata_ind, imm_assign_ext);
       break;
     }

     case D_IMM_ASSIGN_REJ:
     {
       MCAST (imm_assign_rej, D_IMM_ASSIGN_REJ); /* T_D_IMM_ASSIGN_REJ */

       /*
        * process the message
        */
       dat_for_imm_assign_rej (imm_assign_rej);
       break;
     }

     case D_EXT_MEAS_ORDER:
     {
       MCAST (ext_meas_order, D_EXT_MEAS_ORDER);

       dat_for_ext_meas_order (ext_meas_order);
       break;
     }

     default:
       TRACE_EVENT_P1 ( "unknown %02x", _decodedMsg[0] );
       break;
   }

   PFREE (mph_unitdata_ind);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : RR_FOR                     |
| STATE   : code                ROUTINE : for_rr_trace_message       |
+--------------------------------------------------------------------+

  PURPOSE : Function replaces RR_BINDUMP & TRACE_DATA_IND Macros

*/

GLOBAL void for_rr_trace_message (UBYTE *payload, USHORT length,
                                  USHORT arfcn, ULONG fn, UBYTE direction)
{
  GET_INSTANCE_DATA;

  if(direction EQ DOWNLINK)
  {
    switch(_decodedMsg[0])
    {
      case D_ASSIGN_CMD:
        TRACE_EVENT ( "ASSIGNMENT_COMMAND DL" );
        break;

      case D_CHAN_REL:
        TRACE_EVENT ( "CHANNEL_RELEASE DL" );
        break;

      case D_CHAN_MOD:
        TRACE_EVENT ( "CHANNEL_MODE_MODIFY DL" );
        break;

      case D_CIPH_CMD:
        TRACE_EVENT ( "CIPHERING_COMMAND DL" );
        break;

      case D_FREQ_REDEF:
        TRACE_EVENT ( "FREQ RE-DEFINITION DL" );
        break;

      case D_HANDOV_CMD:
        TRACE_EVENT ( "HANDOVER_COMMAND DL" );
        break;

      case B_RR_STATUS:
        TRACE_EVENT ( "RR_STATUS DL" );
        break;

      case D_CLASS_ENQ:
        TRACE_EVENT ( "CLASSMARK_ENQUIRY DL" );
        break;

    #ifdef GPRS
      case D_PDCH_ASS_CMD:
        TRACE_EVENT ( "PDCH_ASSIGNMENT_COMMAND DL" );
        break;

      case D_CHANGE_ORDER:
        TRACE_EVENT ( "CHANGE_ORDER DL" );
        break;
    #endif /*GPRS*/

      case D_SYS_INFO_1:
        TRACE_EVENT ( "SYS_INFO_1 DL" );
        break;

      case D_SYS_INFO_2:
        TRACE_EVENT ( "SYS_INFO_2 DL");
        break;

      case D_SYS_INFO_2BIS:
        TRACE_EVENT ( "SYS_INFO_2BIS DL" );
        break;

      case D_SYS_INFO_2TER:
        TRACE_EVENT ( "SYS_INFO_2TER DL" );
        break;

      case D_SYS_INFO_3:
        TRACE_EVENT ( "SYS_INFO_3 DL" );
        break;

      case D_SYS_INFO_4:
        TRACE_EVENT ( "SYS_INFO_4 DL" );
        break;

      case D_SYS_INFO_5:
        TRACE_EVENT ( "SYS_INFO_5 DL");
        break;

      case D_SYS_INFO_5BIS:
        TRACE_EVENT ( "SYS_INFO_5BIS DL" );
        break;

      case D_SYS_INFO_5TER:
        TRACE_EVENT ( "SYS_INFO_5TER DL" );
        break;

      case D_SYS_INFO_6:
        TRACE_EVENT ( "SYS_INFO_6 DL" );
        break;

      case D_SYS_INFO_7:
        TRACE_EVENT ( "SYS_INFO_7 DL" );
        break;

      case D_SYS_INFO_8:
        TRACE_EVENT ( "SYS_INFO_8 DL" );
        break;

    #ifdef GPRS
      case D_SYS_INFO_13:
        TRACE_EVENT ( "SYS_INFO_13 DL" );
        break;
    #endif/*GPRS*/

      case D_IMM_ASSIGN:
        TRACE_EVENT ( "IMM_ASSIGN DL" );
        break;

      case D_IMM_ASSIGN_EXT:
        TRACE_EVENT ( "IMM_ASSIGN_EXT DL" );
        break;

      case D_IMM_ASSIGN_REJ:
        TRACE_EVENT ( "IMM_ASSIGN_REJ DL" );
        break;

      case D_EXT_MEAS_ORDER:
        TRACE_EVENT ( "EXT_MEAS_ORDER" );
        break;

      default:
        return;
    }
  }
  else /*UPLINK*/
  {
    switch(_decodedMsg[0])
    {
      case U_PAG_RES:
        TRACE_EVENT ( "PAGING_RESPONSE UL" );
        break;

      case U_ASSIGN_FAIL:
        TRACE_EVENT ( "ASSIGNMENT_FAILURE UL" );
        break;

      case U_CIPH_COMP:
        TRACE_EVENT ( "CIPHERING_COMPLETE UL" );
        break;

      case U_HANDOV_FAIL:
        TRACE_EVENT ( "HANDOVER_FAILURE UL" );
        break;

      case U_HANDOV_COMP:
        TRACE_EVENT ( "HANDOVER_COMPLETE UL" );
        break;

      case U_ASSIGN_COMP:
        TRACE_EVENT ( "ASSIGNMENT_COMPLETE UL" );
        break;

      case U_CHAN_MOD_ACK:
        TRACE_EVENT ( "CHANNEL_MODE_ACKNOWLEDGE UL" );
        break;

      case U_MEAS_REP:
        TRACE_EVENT ( "MEASUREMENT_REPORT UL" );
        break;

      case U_CLASS_CHNG:
        TRACE_EVENT ( "CLASSMARK_CHANGE UL" );
        break;

      case D_RR_INIT_REQ:
        TRACE_EVENT ( "RR_INITIALISATION_REQ UL" );
        break;

      case U_PART_REL_COMP:
        TRACE_EVENT ( "PARTIAL_RELEASE_COMPLETE UL" );
        break;

      case U_GPRS_SUSP_REQ:
        TRACE_EVENT ( "GPRS_SUSPENSION_REQ UL" );
        break;

      case U_EXT_MEAS_REPORT:
        TRACE_EVENT ( "EXT_MEAS_REP UL" );
        break;

      default:
        return;
    }
  }

  TRACE_BINDUMP(rr_handle,TC_USER4,NULLSTRING,payload,length);

  TRACE_EVENT_P4 ("[%u] FN=%lu CR=%d SC=%d",
                            (arfcn)&ARFCN_MASK, fn,
                            (short)(rr_data->nc_data[CR_INDEX].arfcn),
                            (short)(rr_data->nc_data[SC_INDEX].arfcn));

}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : RR_FOR                     |
| STATE   : code    ROUTINE : for_unitdata_ind_sys_info_5_5bis       |
+--------------------------------------------------------------------+

  PURPOSE : This function handles the system information type 5 and 5 bis 
  common parameters.

*/
LOCAL void for_unitdata_ind_sys_info_5_5bis(T_SI_TYPE si_type, T_MPH_UNITDATA_IND *mph_unitdata_ind,
                                            UBYTE *ba_index,BUF_neigh_cell_desc   *neigh_cell_desc,
                                            T_LIST *list,  T_VOID_STRUCT   *sys_info_5_5bis )
{

  GET_INSTANCE_DATA;
  UBYTE  old_index;
  UBYTE  ncell_ext;
  
  TRACE_FUNCTION("for_unitdata_ind_sys_info_5_5bis()");

  /*
   * store the current band allocation value
   */
  old_index = *ba_index;

  /*
   * extract the extension indication
   */
  ncell_ext  = (neigh_cell_desc->b_neigh_cell_desc
               [neigh_cell_desc->o_neigh_cell_desc>>3]
               & 0x20) ? 1 : 0;
   /*
    * extract the new band allocation value
    */
#if defined (REL99) && defined (TI_PS_FF_EMR)
  rr_data->sc_data.new_ba_index  = (neigh_cell_desc->b_neigh_cell_desc
               [neigh_cell_desc->o_neigh_cell_desc>>3]
               & 0x10) ? 1 : 0;
#else
  *ba_index  = (neigh_cell_desc->b_neigh_cell_desc
               [neigh_cell_desc->o_neigh_cell_desc>>3]
               & 0x10) ? 1 : 0;
#endif

  /*
   * create a neighbourcell list
   */
  for_create_channel_list ((T_f_range *)neigh_cell_desc,
                          list);

  /*
   * process the message in the attachment process
   */
  att_for_sysinfo_type5_5bis (mph_unitdata_ind->arfcn, list, 
                              old_index, ncell_ext,si_type);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : RR_FOR                     
| STATE   : code                ROUTINE : for_unitdata_ind_sys_info_2_2bis   |
+--------------------------------------------------------------------+

  PURPOSE : This function handles the System Information 2 and 2bis common parameters

*/

LOCAL void  for_unitdata_ind_sys_info_2_2bis(T_SI_TYPE si_type,  T_MPH_UNITDATA_IND  *mph_unitdata_ind,
                                             BUF_neigh_cell_desc  *neigh_cell_desc,
                                             T_LIST *list,    T_VOID_STRUCT   *sys_info_2_2bis)
{
  GET_INSTANCE_DATA;
  UBYTE  ncell_ext;
#if defined (REL99) && defined (TI_PS_FF_EMR)
  UBYTE  ba_ind;
  UBYTE  index;
#endif

  TRACE_FUNCTION("for_unitdata_ind_sys_info_2_2bis()");

  /*
   * successfully received system info
   * decrements the BCCH error counter
   */
  if (rr_data->bcch_error)
      (rr_data->bcch_error)--;

  /*
   * extract the ncell extension flag. It indicates
   * whether the neighbourcell description is complete
   * or must be combined with the neighbourcell description
   * of system information 2bis.
   */
  ncell_ext = neigh_cell_desc->b_neigh_cell_desc
              [neigh_cell_desc->o_neigh_cell_desc>>3]
              & 0x20;

#if defined (REL99) && defined (TI_PS_FF_EMR)
  ba_ind    = (neigh_cell_desc->b_neigh_cell_desc
    [neigh_cell_desc->o_neigh_cell_desc>>3]
    & 0x10) ? 1 : 0;       
  index = att_get_index(mph_unitdata_ind->arfcn);
  /* Store BA_IND in appropriate context*/
  for_update_ba_ind (index, ba_ind);
#endif

  /*
   * create a neighbourcell list
   */
  for_create_channel_list ((T_f_range *)neigh_cell_desc,
                          list);
  /*
   * process the message in the attachment process
   */

  att_for_sysinfo_type2_2bis (mph_unitdata_ind->arfcn, sys_info_2_2bis, 
                                list, ncell_ext,  si_type);                                             
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : RR_FOR                     |
| STATE   : code                ROUTINE : for_unitdata_ind_si3_si4       |
+--------------------------------------------------------------------+

  PURPOSE : This function handles System information type 3 and 4
  common parameters.

*/
LOCAL  void   for_unitdata_ind_si3_si4(T_SI_TYPE si_type,  T_MPH_UNITDATA_IND  *mph_unitdata_ind,
                                       T_loc_area_ident *loc_area_ident,
                                       T_VOID_STRUCT   *sys_info_3_4)
                                          
{
  GET_INSTANCE_DATA;
  TRACE_FUNCTION("for_unitdata_ind_si3_si4()");
       
  if (loc_area_ident->c_mnc EQ 2)
  {
    /* Internally G23 uses always 3-digit-MNC */
    loc_area_ident->c_mnc = SIZE_MNC;
    loc_area_ident->mnc[2] = 0xf;
  }

  /*
   * successfully received system info
   * decrements the BCCH error counter
   */
  if (rr_data->bcch_error)
      (rr_data->bcch_error)--;


  if(si_type  EQ  SI_TYPE_4)
  {
    if (((T_D_SYS_INFO_4 *)sys_info_3_4)->v_chan_desc)
    {
     /*
      * If system information contains a CBCH channel
      * description check the CBCH channel description
      */
      for_check_channel_descr (&((T_D_SYS_INFO_4 *)sys_info_3_4)->chan_desc);
  
      /*
       * Consistency check: If the CBCH channel description
       * defines Frequency hopping but the mobile allocation
       * is not available, clear the CBCH channel description.
       */
      if (((T_D_SYS_INFO_4 *)sys_info_3_4)->chan_desc.hop EQ TRUE AND
          ((T_D_SYS_INFO_4 *)sys_info_3_4)->v_mob_alloc EQ FALSE)
             ((T_D_SYS_INFO_4 *)sys_info_3_4)->v_chan_desc = FALSE ;
    }
    att_for_sysinfo_type4 (mph_unitdata_ind->arfcn, (T_D_SYS_INFO_4 *)sys_info_3_4);
  } 

       /*
        * process the message in the attachment process
        */
  if(si_type  EQ  SI_TYPE_3)
     att_for_sysinfo_type3 (mph_unitdata_ind->arfcn, (T_D_SYS_INFO_3 *)sys_info_3_4);

}

#if defined (REL99) && defined (TI_PS_FF_EMR)
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : RR_FOR                     |
| STATE   : code                ROUTINE : for_dl_short_unitdata_ind       |
+--------------------------------------------------------------------+

  PURPOSE : Process the primitive DL_SHORT_UNITDATA_IND received from RR.

*/

GLOBAL void for_dl_short_unitdata_ind   (T_DL_SHORT_UNITDATA_IND *dl_short_unitdata_ind_orig)
{
  GET_INSTANCE_DATA;
  PPASS(dl_short_unitdata_ind_orig, short_unitdata_ind, DL_SHORT_UNITDATA_IND);
  /*
   * Clear error field and CCD result field.
   */
  memset (&rr_data->ms_data.error,0, sizeof(rr_data->ms_data.error));
  memset (_decodedMsg,0, sizeof(_decodedMsg));

  /*
   * Decode the message with CCD.
   *
   * The protocol discriminator bit is implicitely checked when trying
   * to decode the message
   */
  if (ccd_decodeMsg (CCDENT_RR_SHORT, DOWNLINK,
                     (T_MSGBUF *)&short_unitdata_ind->sdu,
                     (UBYTE *)_decodedMsg,
                     NOT_PRESENT_8BIT) NEQ ccdOK)
  {
    /*
     * CCD has detected an error
     */
    UBYTE ccd_err;
    USHORT parlist [6];

    /*
     * get the first error
     */
    ccd_err = ccd_getFirstError (CCDENT_RR, parlist);
    do 
    {
      /*
       * Error Handling
       */
      switch (ccd_err)
      {
        case ERR_INVALID_MID:         /* unknown message type */
        case ERR_MSG_LEN:             /* some exceeds entire message length */
        case ERR_MAND_ELEM_MISS:      /* Mandatory elements missing */
          TRACE_EVENT_P2("message with mt=%02x ignored due to CCD error=%d", 
                         ((short_unitdata_ind->sdu.buf[short_unitdata_ind->sdu.o_buf>>3]) >> 2) & 0x3F,
                         ccd_err);
          PFREE (short_unitdata_ind); /* in unacknowledged mode ignore message */
          return;

        case ERR_MAX_REPEAT:
          TRACE_EVENT_P1("CCD error=%d ignored here", ccd_err);
          break;
        default:
          TRACE_ERROR("unknown CCD return");
          break;
      }
      ccd_err = ccd_getNextError (CCDENT_RR, parlist);
    }
    while (ccd_err NEQ ERR_NO_MORE_ERROR);
  }

  /*
   * depending on the message type
   */
  switch (_decodedMsg[0])
  {
    case D_MEAS_INF:
    {
    /*
     * add handling of message here:*/
      MCAST (meas_inf, D_MEAS_INF);
      if (dat_for_meas_inf(meas_inf) )
      {
        /*Send Enhanced para to ALR*/
        attf_send_enh_para_to_alr(rr_data->sc_data.emr_data_current.rep_type,
          &rr_data->sc_data.emr_data_current.enh_para);        
      }
    }
      break;
    default:
      TRACE_EVENT_P1 ( "message %02x not supported", _decodedMsg[0] );
      break;
   }

   PFREE (short_unitdata_ind);
}
#endif        
#endif
