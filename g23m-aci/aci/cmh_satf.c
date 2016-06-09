/* 
+----------------------------------------------------------------------------- 
|  Project :  GSM-PS (6147)
|  Modul   :  CMH_SATF
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
|  Purpose :  This module defines global functions of the command
|             handler.
+----------------------------------------------------------------------------- 
*/ 

#include "config.h"
#include "fixedconf.h"
#include "condat-features.h"
#include "aci_conf.h"

#ifdef SIM_TOOLKIT

#ifndef CMH_SATF_C
#define CMH_SATF_C
#endif

#define _PACK_DEF_ALPHA /* apply packing for GSM Default Alphabet */

#include "aci_all.h"
/*==== INCLUDES ===================================================*/
#ifdef DTI
#include "dti.h"
#endif /* DTI */

#include "pcm.h"

#include "aci.h"
#include "aci_cmh.h"
#include "ati_cmd.h"
#include "aci_mem.h"

#include "ati_src_sat.h"

#ifdef FAX_AND_DATA
#include "aci_fd.h"
#endif    /* FAX_AND_DATA */

#include "phb.h"
#include "ksd.h"
#include "aoc.h"

#include "psa.h"
#include "psa_cc.h" 
#include "psa_sat.h"
#include "psa_sim.h"
#include "psa_sms.h"
#include "psa_ss.h"
#include "psa_util.h"
#include "psa_mm.h"

#include "cmh.h" 
#include "cmh_ss.h"
#include "cmh_cc.h" 
#include "cmh_phb.h"

#ifdef DTI
#include "dti_conn_mng.h"
#include "dti_cntrl_mng.h"
#endif /* DTI */

#include "cmh_sim.h"
#include "cmh_sat.h"
#include "cmh_sms.h"

/* #include "cmh_sat.h" */

#if defined (GPRS) AND defined (DTI)
#include "gaci.h"
#include "gaci_cmh.h"
#include "gaci_srcc.h"
#include "psa_gmm.h"
#include "psa_sm.h"
#include "cmh_sm.h"
#endif


#if defined (FAX_AND_DATA) AND defined (DTI)
#include "psa_ra.h"
#include "cmh_ra.h"
#include "psa_l2r.h"
#include "cmh_l2r.h"
#endif    /* FAX_AND_DATA */

#ifdef CO_UDP_IP
#include "wap_aci.h"
#include "psa_tcpip.h"
#endif   /* CO_UDP_IP */
#include "rtcdrv.h"

/*==== CONSTANTS ==================================================*/
#define MAX_CBM_REMEMBERED 9
#define SIM_CLASS_E_MAX_BUFFER_SIZE  65535

/*==== TYPES ======================================================*/
typedef struct
{
  U16   msgIdent;
  U16   msgCode;
  U8    updtNum;
  U8    pageNum;
} T_SAT_CBM_SEND;


/*==== EXPORT =====================================================*/
LOCAL void cmhSAT_CBMInitList();

/*==== VARIABLES ==================================================*/
T_SAT_CBM_SEND *CBMsend = NULL;

EXTERN T_ACI_CUSCFG_PARAMS cuscfgParams;


/*==== FUNCTIONS ==================================================*/

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : SAT                     |
|                                 ROUTINE : cmhSAT_GetCmdPrfLocation    |
+-------------------------------------------------------------------+

  PURPOSE  : This function will return TRUE if the Command has been found and
                    FALSE if the Command or the Qualifier is unknown.
                    The byte location and  Mask for the given Stk Cmd are returned in the passed paramters,
                    a return value of TRUE with Byte Number of 0xFF and Mask of 0xFF indicates an
                    END_OF_SESSION, which isn't in the Profile
*/

LOCAL BOOL cmhSAT_GetCmdPrfLocation (U8 cmd_typ, U8 cmd_qual, U8 *prf_byte_no, U8 *byte_mask)
{
    /*
    ** Initialise the byte and Mask settings to 0xFF
    */
    *prf_byte_no = 0xFF;
    *byte_mask = 0xFF;

    switch (cmd_typ)
    {
        case SAT_CMD_REFRESH:
            *prf_byte_no = 2;
            *byte_mask = SAT_TP3_REFRESH;
            break;

        case SAT_CMD_MORE_TIME:
            *prf_byte_no = 2;
            *byte_mask = SAT_TP3_MORE_TIME;
            break;

        case SAT_CMD_POLL_INTERVAL:
            *prf_byte_no = 2;
            *byte_mask = SAT_TP3_POLL_ITV;
            break;

        case SAT_CMD_POLL_OFF:
            *prf_byte_no = 2;
            *byte_mask = SAT_TP3_POLL_OFF;
            break;

        case SAT_CMD_EVENT_LIST:
            *prf_byte_no = 4;
            *byte_mask = SAT_TP5_EVENT_LIST;
            break;

        case SAT_CMD_SETUP_CALL:
            *prf_byte_no = 3;
            *byte_mask = SAT_TP4_SETUP_CALL;
            break;

        case SAT_CMD_SEND_SS:
            *prf_byte_no = 3;
            *byte_mask = SAT_TP4_SEND_SS;
            break;

        case SAT_CMD_SEND_USSD:
            *prf_byte_no = 3;
            *byte_mask = SAT_TP4_SEND_USSD;
            break;

        case SAT_CMD_SEND_SMS:
            *prf_byte_no = 3;
            *byte_mask = SAT_TP4_SEND_SMS;
            break;

        case SAT_CMD_SEND_DTMF:
            *prf_byte_no = 8;
            *byte_mask = SAT_TP9_DTMF_CMD;
            break;

        case SAT_CMD_LAUNCH_BROWSER:
            *prf_byte_no = 8;
            *byte_mask = SAT_TP9_LAUNCH_BROWSER;
            break;

        case SAT_CMD_PLAY_TONE:
            *prf_byte_no = 2;
            *byte_mask = SAT_TP3_PLAY_TONE;
            break;

        case SAT_CMD_DISPLAY_TEXT:
            *prf_byte_no = 2;
            *byte_mask = SAT_TP3_DSPL_TXT;
            break;

        case SAT_CMD_GET_INKEY:
            *prf_byte_no = 2;
            *byte_mask = SAT_TP3_GET_INKEY;
            break;

        case SAT_CMD_GET_INPUT:
            *prf_byte_no = 2;
            *byte_mask = SAT_TP3_GET_INPUT;
            break;

        case SAT_CMD_SEL_ITEM:
            *prf_byte_no = 3;
            *byte_mask = SAT_TP4_SEL_ITEM;
            break;

        case SAT_CMD_SETUP_MENU:
            *prf_byte_no = 3;
            *byte_mask = SAT_TP4_SETUP_MENU;
            break;

        case SAT_CMD_PROV_LOC_INFO:
            /*
            ** Depends on which type of Local Info is requested
            */
            switch (cmd_qual)
            {
                case 0:
                case 1:
                    *prf_byte_no = 3;
                    *byte_mask = SAT_TP4_PLI_PLMN_IMEI;
                    break;

                case 2:
                    *prf_byte_no = 3;
                    *byte_mask = SAT_TP4_PLI_NMR;
                    break;

                case 3:
                    *prf_byte_no = 7;
                    *byte_mask = SAT_TP8_PLI_DTT;
                    break;

                case 4:
                    *prf_byte_no = 8;
                    *byte_mask = SAT_TP9_PLI_LANG;
                    break;

                case 5:
                    *prf_byte_no = 8;
                    *byte_mask = SAT_TP9_PLI_TIMING_ADV;
                    break;
                    
                default:
            return (FALSE);     /* Command Not recognised */
            }
            break;

        case SAT_CMD_TIMER_MNG:
            *prf_byte_no = 7;
            *byte_mask = SAT_TP8_TMNG_ST | SAT_TP8_TMNG_VAL;
            break;

        case SAT_CMD_IDLE_TEXT:
            *prf_byte_no = 7;
            *byte_mask = SAT_TP8_IDLE_TXT;
            break;

        case SAT_CMD_RUN_AT:
            *prf_byte_no = 7;
            *byte_mask = SAT_TP8_AT_CMD;
            break;

        case SAT_CMD_OPEN_CHANNEL:
            *prf_byte_no = 11;
            *byte_mask = SAT_TP12_OPEN_CHANNEL;
            break;

        case SAT_CMD_CLOSE_CHANNEL:
            *prf_byte_no = 11;
            *byte_mask = SAT_TP12_CLOSE_CHANNEL;
            break;

        case SAT_CMD_RECEIVE_DATA:
            *prf_byte_no = 11;
            *byte_mask = SAT_TP12_RECEIVE_DATA;
            break;

        case SAT_CMD_SEND_DATA:
            *prf_byte_no = 11;
            *byte_mask = SAT_TP12_SEND_DATA;
            break;

        case SAT_CMD_GET_CHANNEL_STATUS:
            *prf_byte_no = 11;
            *byte_mask = SAT_TP12_GET_CHANNEL_STAT;
            break;

        case SAT_CMD_END_SESSION:
            /*
            ** Do nothing - The command should be handled by the MMI,
            ** but there is no entry in the terminal profile
            */
            break;

        default:
            return (FALSE);     /* Command Not recognised */
    }

    return (TRUE);
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : SAT                     |
|                                 ROUTINE : cmhSAT_IsStkCmdForMmi    |
+-------------------------------------------------------------------+

  PURPOSE : This function will check whether the command should be set to the MMI
                   and if so will trigger a SATI indication

*/

GLOBAL BOOL cmhSAT_IsStkCmdForMmi (U8 cmd_typ, U8 cmd_qual)
{
     BOOL is_handled = FALSE;
     U8 prf_byte_no;
     U8 byte_mask;
     int idx;

    /*
    ** End of Session is handled by the 
    */
     if (cmd_typ EQ SAT_CMD_END_SESSION)
     {
        is_handled = TRUE;
     }
     else
     {
        /*
        ** Convert the command type into a Stk Profile Byte Number and Offset
        */
        if (cmhSAT_GetCmdPrfLocation(cmd_typ, cmd_qual, &prf_byte_no, &byte_mask))
        {
            /*
            ** For each Attached Service
            */
            for (idx = 0; idx < CMD_SRC_MAX AND (is_handled EQ FALSE); idx++)
            {
                /*
                ** if the Command is handled
                */
                if (simShrdPrm.setPrm[idx].STKprof[prf_byte_no] & byte_mask)
                {
                    /*
                    ** Set a handled flag to TRUE
                    */
                    is_handled = TRUE;
                }
            }
        }
     }

    /*
    ** if the Command is handled
    */
    if (is_handled)
    {
        /*
        ** Send the SATI indication
        */
        cmhSAT_STKCmdInd();
    }

    /*
    ** return the value of the handled flag
    */
    return (is_handled);
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : SAT                     |
|                                 ROUTINE : cmhSAT_CalCntrlBySIM    |
+-------------------------------------------------------------------+

  PURPOSE : If call control by SIM is activated and allocated, the
            parameters for the call found in the call table are
            packed into a SIM envelope command and send to the SIM
            for checking.

*/

GLOBAL T_ACI_RETURN cmhSAT_CalCntrlBySIM ( SHORT cId )
{
  T_CC_CALL_TBL *ctb = ccShrdPrm.ctb[cId];

  TRACE_FUNCTION("cmhSAT_CalCntrlBySIM()");

/*
 *-------------------------------------------------------------------
 *  check if Call Control is disabled (Cust1 Customisation)
 *-------------------------------------------------------------------
 */
  if(simShrdPrm.setPrm[ctb->calOwn].sat_cc_mode EQ SATCC_CONTROL_BY_SIM_INACTIVE)
  {
      simShrdPrm.setPrm[ctb->calOwn].sat_cc_mode = SATCC_CONTROL_BY_SIM_ACTIVE;
      return( AT_CMPL );
  }

/*
 *-------------------------------------------------------------------
 *  check if service is activated and allocated
 *-------------------------------------------------------------------
 */
  if( ! psaSIM_ChkSIMSrvSup( SRV_CalCntrl )) return( AT_CMPL );

/*
 *-------------------------------------------------------------------
 *  check if Call Control customization is enabled... if yes return AT_CMPL
 *-------------------------------------------------------------------
 */
  
  if(cuscfgParams.MO_Call_Control_SIM EQ CUSCFG_STAT_Enabled)
  {
    return( AT_CMPL );
  }

/*
 *-------------------------------------------------------------------
 *  check if a call control request is in progress
 *-------------------------------------------------------------------
 */
  if( satShrdPrm.SIMCCParm.busy EQ TRUE ) return( AT_BUSY );

/*
 *-------------------------------------------------------------------
 *  build envelope call control command
 *-------------------------------------------------------------------
 */
  CCD_START;
  psaSAT_BuildEnvCC ( cId, NULL, NULL, NULL, NULL );

  satShrdPrm.SIMCCParm.cId   = cId;
  satShrdPrm.SIMCCParm.ccAct = CC_ACT_CAL;
  satShrdPrm.SIMCCParm.owner = ctb->calOwn;
  satShrdPrm.SIMCCParm.busy  = TRUE;

  satShrdPrm.owner = /*ctb->calOwn;*/ OWN_SRC_INV;

  satShrdPrm.Cbch_EvtDnl = FALSE;

  if( psaSAT_STKEnvelope (NULL) < 0 )  /* envelope STK command */
  {
    TRACE_EVENT( "FATAL RETURN SAT in send env" );
    CCD_END;
    return( AT_FAIL );
  }

  CCD_END;
  return (AT_EXCT);

}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : SAT                     |
|                                 ROUTINE : cmhSAT_SSCntrlBySIM     |
+-------------------------------------------------------------------+

  PURPOSE : If call control by SIM is activated and allocated, the
            the passed supplementary service control string is
            packed into a SIM envelope command and send to the SIM
            for checking.

*/

GLOBAL T_ACI_RETURN cmhSAT_SSCntrlBySIM ( T_CLPTY_PRM *cldPty, UBYTE own )
{
  TRACE_FUNCTION("cmhSAT_SSCntrlBySIM()");

/*
 *-------------------------------------------------------------------
 *  check if Call Control is disabled (Cust1 Customisation)
 *-------------------------------------------------------------------
 */
  if(simShrdPrm.setPrm[own].sat_cc_mode EQ SATCC_CONTROL_BY_SIM_INACTIVE)
  {
      simShrdPrm.setPrm[own].sat_cc_mode = SATCC_CONTROL_BY_SIM_ACTIVE;
      return( AT_CMPL );
  }

/*
 *-------------------------------------------------------------------
 *  check if service is activated and allocated
 *-------------------------------------------------------------------
 */
  if( ! psaSIM_ChkSIMSrvSup( SRV_CalCntrl )) return( AT_CMPL );

/*
 *-------------------------------------------------------------------
 *  check if Call Control customization is enabled... if yes return AT_CMPL
 *-------------------------------------------------------------------
 */
  
  if(cuscfgParams.MO_SS_Control_SIM EQ CUSCFG_STAT_Enabled)
  {
    return( AT_CMPL );
  }

/*
 *-------------------------------------------------------------------
 *  check if a call control request is in progress
 *-------------------------------------------------------------------
 */
  if( satShrdPrm.SIMCCParm.busy EQ TRUE ) return( AT_BUSY );

/*
 *-------------------------------------------------------------------
 *  build envelope call control command
 *-------------------------------------------------------------------
 */
  CCD_START;
  psaSAT_BuildEnvCC ( NO_ENTRY, cldPty, NULL, NULL, NULL);

  if( cldPty NEQ NULL )
    satShrdPrm.SIMCCParm.ccAct = CC_ACT_SS;

  satShrdPrm.SIMCCParm.owner = own;
  satShrdPrm.SIMCCParm.busy  = TRUE;

  satShrdPrm.owner = OWN_SRC_INV;

  satShrdPrm.Cbch_EvtDnl = FALSE;

  if( psaSAT_STKEnvelope (NULL) < 0 )  /* envelope STK command */
  {
    TRACE_EVENT( "FATAL RETURN SAT in send env" );
    CCD_END;
    return( AT_FAIL );
  }

  CCD_END;

/*
 *-------------------------------------------------------------------
 *  save SS string
 *-------------------------------------------------------------------
 */
  
  if( cldPty NEQ NULL )
    memcpy(&satPndSetup.clpty, cldPty, sizeof(T_CLPTY_PRM));
    
  return (AT_EXCT);

}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : SAT                     |
|                                 ROUTINE : cmhSAT_USSDCntrlBySIM   |
+-------------------------------------------------------------------+

  PURPOSE : If call control by SIM is activated and allocated, the
            the passed supplementary service control string is
            packed into a SIM envelope command and send to the SIM
            for checking.

*/

GLOBAL T_ACI_RETURN cmhSAT_USSDCntrlBySIM ( T_sat_ussd *ussd, UBYTE own )
{
  TRACE_FUNCTION("cmhSAT_USSDCntrlBySIM()");

/*
 *-------------------------------------------------------------------
 *  check if Call Control is disabled (Cust1 Customisation)
 *-------------------------------------------------------------------
 */
  if(simShrdPrm.setPrm[own].sat_cc_mode EQ SATCC_CONTROL_BY_SIM_INACTIVE)
  {
      simShrdPrm.setPrm[own].sat_cc_mode = SATCC_CONTROL_BY_SIM_ACTIVE;
      return( AT_CMPL );
  }

/*
 *-------------------------------------------------------------------
 *  check if service is activated and allocated
 *-------------------------------------------------------------------
 */
  if( ! psaSIM_ChkSIMSrvSup( SRV_CalCntrl )) return( AT_CMPL );

/*
 *-------------------------------------------------------------------
 *  check if Call Control customization is enabled... if yes return AT_CMPL
 *-------------------------------------------------------------------
 */
  
  if(cuscfgParams.MO_USSD_Control_SIM EQ CUSCFG_STAT_Enabled)
  {
    return( AT_CMPL );
  }

/*
 *-------------------------------------------------------------------
 *  check if a call control request is in progress
 *-------------------------------------------------------------------
 */
  if( satShrdPrm.SIMCCParm.busy EQ TRUE ) return( AT_BUSY );

/*
 *-------------------------------------------------------------------
 *  save SS string
 *-------------------------------------------------------------------
 */
  if( ussd )
  {
    satPndSetup.ussd_str.dcs = ussd->dcs;
    satPndSetup.ussd_str.c_ussd_str = ussd->c_ussd_str;
    memcpy( satPndSetup.ussd_str.ussd_str, ussd->ussd_str, satPndSetup.ussd_str.c_ussd_str);
    /* we have to move the ussd_str pointer since it still points to _decodedMsg
       and would be overwritten by the first memset in psaSAT_BuildEnvCC */
    ussd->ussd_str = satPndSetup.ussd_str.ussd_str;
  }

/*
 *-------------------------------------------------------------------
 *  build envelope call control command
 *-------------------------------------------------------------------
 */
  CCD_START;
  psaSAT_BuildEnvCC ( NO_ENTRY, NULL, ussd, NULL, NULL );

  if( ussd )
    satShrdPrm.SIMCCParm.ccAct = CC_ACT_USSD;

  satShrdPrm.SIMCCParm.owner = own;
  satShrdPrm.SIMCCParm.busy  = TRUE;

  satShrdPrm.owner = OWN_SRC_INV;

  satShrdPrm.Cbch_EvtDnl = FALSE;

  if( psaSAT_STKEnvelope (NULL) < 0 )  /* envelope STK command */
  {
    TRACE_EVENT( "FATAL RETURN SAT in send env" );
    CCD_END;
    return( AT_FAIL );
  }

  CCD_END;


  return (AT_EXCT);

}


/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : SAT                     |
|                                 ROUTINE : cmhSAT_CBMInitList      |
+-------------------------------------------------------------------+

  PURPOSE : Allocates and initializes CBM list.
*/

LOCAL void cmhSAT_CBMInitList()
{
  TRACE_FUNCTION("cmhSAT_CBMInitList()");

  if ( CBMsend EQ NULL )
  {
    ACI_MALLOC(CBMsend, sizeof(T_SAT_CBM_SEND) * MAX_CBM_REMEMBERED);
  }
  memset( CBMsend, NOT_PRESENT_8BIT, 
          sizeof(T_SAT_CBM_SEND) * MAX_CBM_REMEMBERED );
}


/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : SAT                     |
|                                 ROUTINE : cmhSAT_CBMDestroyList   |
+-------------------------------------------------------------------+

  PURPOSE : Frees memory of CBM list and sets list pointer to NULL.
*/

GLOBAL void cmhSAT_CBMDestroyList()
{
  TRACE_FUNCTION("cmhSAT_CBMDestroyList()");

  if ( CBMsend NEQ NULL )
  {
    ACI_MFREE( CBMsend );
    CBMsend = NULL;
  }
}


/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : SAT                     |
|                                 ROUTINE : cmhSAT_CBMAddToList     |
+-------------------------------------------------------------------+

  PURPOSE : Adds new CBM Identifiers to end of CBM list
*/

LOCAL void cmhSAT_CBMAddToList( UBYTE pos,
                                U16   msgIdent, 
                                U16   msgCode,
                                UBYTE updtNum,  
                                UBYTE pageNum )
{
  TRACE_FUNCTION("cmhSAT_CBMAddToList()");

  if ( pos >= MAX_CBM_REMEMBERED )
  {
    /* remove oldest:
       move all entries one entry to the beginning of the list */
    memcpy( CBMsend, CBMsend + 1, 
            (MAX_CBM_REMEMBERED - 1) * sizeof(T_SAT_CBM_SEND));
    pos = MAX_CBM_REMEMBERED - 1;
  }

  CBMsend[pos].msgIdent = msgIdent;
  CBMsend[pos].msgCode  = msgCode;
  CBMsend[pos].updtNum  = updtNum;
  CBMsend[pos].pageNum  = pageNum;
}


/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : SAT                     |
|                                 ROUTINE : cmhSAT_CBMUpdateList    |
+-------------------------------------------------------------------+

  PURPOSE : Moves last updated CBM to end of the CBM list
*/

LOCAL void cmhSAT_CBMUpdateList( UBYTE pos )
{
  T_SAT_CBM_SEND curCBM;

  TRACE_FUNCTION("cmhSAT_CBMUpdateList()");

  /* save data of current CBM */
  memcpy( &curCBM, CBMsend + pos, sizeof(T_SAT_CBM_SEND) );

  /* move all entries, starting after the current entry, one entry to 
     the beginning of the list */
  memcpy( CBMsend + pos, CBMsend + pos+1, 
          ( MAX_CBM_REMEMBERED - (pos+1)) * sizeof(T_SAT_CBM_SEND) );

  /* copy current CBM at last used position */
  while ( (pos < MAX_CBM_REMEMBERED-1) AND 
          (CBMsend[pos].msgIdent NEQ NOT_PRESENT_16BIT) )
  {
    pos++;
  }
  memcpy( CBMsend + pos, &curCBM, sizeof(T_SAT_CBM_SEND) );
  
}


/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : SAT                     |
|                                 ROUTINE : cmhSAT_CBMExtractData   |
+-------------------------------------------------------------------+

  PURPOSE : Extracts MessageCode, Messag Identifier, Update number,
            Page number and total numbers of pages from the CBM.
*/

LOCAL void cmhSAT_CBMExtractData( UBYTE* cbMsg,
                                  U16*   msgIdent, 
                                  U16*   msgCode,
                                  UBYTE* updtNum,  
                                  UBYTE* pageNum,  
                                  UBYTE* pageTotal )
{
  UBYTE cnvrt[2];

  TRACE_FUNCTION("cmhSAT_CBMExtractData()");

  /* extract message identifiers:
     cbMsg is not yet enveloped for SAT, so that Bytes 3 - 4 includes
     the message identifiers.
   */
  memcpy(cnvrt+1,  &cbMsg[2], 1);
  memcpy(cnvrt,    &cbMsg[3], 1);
  memcpy(msgIdent, cnvrt,     2);

  /* decode serial number:
     cbMsg is not yet enveloped for SAT, so that cbMsg starts 
     with the serial number.
     update number:
     The serial number includes the message code in bits 0 - 5 of the
     first Byte and in bits 4 -7 in the second Byte.
   */
  memcpy(cnvrt+1, &cbMsg[0], 1);
  memcpy(cnvrt,   &cbMsg[1], 1);
  memcpy(msgCode, cnvrt,     2);
  *msgCode = ( *msgCode >> 4 ) & 1023;

  /*
     update number:
     The second Byte of the serial number includes the 
     update number in bits 0 - 3.
     the bitwise AND operation with 15 will set bits 4 - 7 to zero.
   */
  *updtNum = cbMsg[1] & 15;

  /* extract page parameter:
     cbMsg is not yet enveloped for SAT, so that the sixth Byte is
     the page parameter.
     It includes the total number of pages in bits 0 - 3.
     The bitwise AND operation with 15 will set bits 4 - 7 to zero.
     The current page number is in bits 4 - 7.
     To get the current page number the bits have to be shifted 
     4 positions to the right.
   */
  *pageTotal = cbMsg[5] & 15;
  *pageNum   = cbMsg[5] >> 4;
}


/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : SAT                     |
|                                 ROUTINE : cmhSAT_CBMIsNewMsg      |
+-------------------------------------------------------------------+

  PURPOSE : Checks if CBM is a new CBM or if it already has been 
            send to SIM. The Identifiers of already received CBMs
            are stored in a list.
  RETURNS:  TRUE, if it is new and shall be send to SIM now.
            FALSE, if it already has been send to SIM.
*/

LOCAL BOOL cmhSAT_CBMIsNewMsg( UBYTE* cbMsg )
{

  U16            msgIdent;
  U16            msgCode;
  U8             updtNum;
  U8             pageTotal;
  U8             pageNum;
  UBYTE          i;


  TRACE_FUNCTION("cmhSAT_CBMIsNewMsg()");

  if ( CBMsend EQ NULL )
  {
    cmhSAT_CBMInitList();
  }

  /*----------------------------------------------------------------
   * extract CBM identifiers
   *----------------------------------------------------------------*/ 

  cmhSAT_CBMExtractData( cbMsg, &msgIdent, &msgCode, &updtNum,  
                         &pageNum, &pageTotal );

  /*----------------------------------------------------------------
   * check if message has already been send to SIM
   *----------------------------------------------------------------*/ 

  /* search for message identifiers in CB Messages already send to SIM */
  for ( i = 0; 
        (i < MAX_CBM_REMEMBERED) AND (CBMsend[i].msgCode NEQ NOT_PRESENT_16BIT);
        i++ )
  {
    if ( (msgIdent EQ CBMsend[i].msgIdent) AND
         (msgCode EQ CBMsend[i].msgCode) )
    {

      /* if update number has changed, CBM shall be send to SIM */
      /* 23.041 9.4.1.2.1:
         Any Update Number eight or less higher (modulo 16)
         than the last received Update Number will be considered more recent,
         and shall be treated as a new CBS message,
         provided the mobile has not been switched off. */

      U8 delta;

      delta = (updtNum - CBMsend[i].updtNum) % 16;  /* force explicit negative carry! */
      if (delta > 0 AND delta <= 8)
      {
        /* for the next check: set update number to new update number 
           and reset page number for new update */
        CBMsend[i].updtNum = updtNum;
        CBMsend[i].pageNum = 0;
        cmhSAT_CBMUpdateList( i );

        return TRUE;
      }


      /* check if message contains more than one page */
      if ( pageTotal > 1 )
      {
        /* check if page is send to SIM */
        if ( pageNum > CBMsend[i].pageNum )
        {
          /* mark page now as send for next CBM */
          CBMsend[i].pageNum = pageNum; 
          cmhSAT_CBMUpdateList( i );
          
          /* page is not yet send to SIM */
          return TRUE;
        }
      }

      /* CBM is not new */
      return FALSE;
    }
  }

  /*----------------------------------------------------------------
   * mark CBM as send to SIM: insert into list
   *---------------------------------------------------------------*/

  cmhSAT_CBMAddToList( i, msgIdent, msgCode, updtNum, pageNum );

  return TRUE;
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : SAT                     |
|                                 ROUTINE : cmhSAT_verifyNewLocInfo |
+-------------------------------------------------------------------+

  PURPOSE : This function verifies the current MM PLMN information
            with the one already stored by SAT and indicates TRUE if
            the information has changed

*/
LOCAL BOOL cmhSAT_verifyNewLocInfo ()
{
  if (satShrdPrm.locInfo.regStatus EQ mmShrdPrm.regStat)
  {
    /* If registration status is full service, check to proceed 
       if the location information has changed*/
    if (satShrdPrm.locInfo.regStatus EQ RS_FULL_SRV)
    {
      if( memcmp (mmShrdPrm.usedPLMN.mcc, satShrdPrm.locInfo.currPLMN.mcc, SIZE_MCC) EQ 0
      AND memcmp (mmShrdPrm.usedPLMN.mnc, satShrdPrm.locInfo.currPLMN.mnc, SIZE_MNC) EQ 0 
      AND mmShrdPrm.lac EQ satShrdPrm.locInfo.lac AND mmShrdPrm.cid EQ satShrdPrm.locInfo.cid)
      {
        return FALSE;
      }
    }
  }

  satShrdPrm.locInfo.regStatus = mmShrdPrm.regStat;

  /* The other information is updated only if ME is in full service mode */
  if (satShrdPrm.locInfo.regStatus EQ RS_FULL_SRV)
  {
    satShrdPrm.locInfo.cid             = mmShrdPrm.cid;
    satShrdPrm.locInfo.lac             = mmShrdPrm.lac;
    satShrdPrm.locInfo.currPLMN.v_plmn = mmShrdPrm.usedPLMN.v_plmn;
    memcpy(satShrdPrm.locInfo.currPLMN.mcc, mmShrdPrm.usedPLMN.mcc, SIZE_MCC);
    memcpy(satShrdPrm.locInfo.currPLMN.mnc, mmShrdPrm.usedPLMN.mnc, SIZE_MNC);
  }

  return TRUE;
}


/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : SAT                     |
|                                 ROUTINE : cmhSAT_DatDwnLdCB       |
+-------------------------------------------------------------------+

  PURPOSE : This function supports the data download to SIM via cell
            broadcast messages.

*/

GLOBAL T_ACI_RETURN cmhSAT_DatDwnLdCB ( UBYTE* cbMsg, SHORT cbLen )
{

  TRACE_FUNCTION("cmhSAT_DatDwnLdCB()");

/*
 *-------------------------------------------------------------------
 *  build envelope
 *-------------------------------------------------------------------
 */
  CCD_START;
  psaSAT_BuildEnvCB ( cbMsg, cbLen );

  satShrdPrm.owner = OWN_SRC_INV;

  satShrdPrm.Cbch_EvtDnl = TRUE;

  /* only if CBM is not yet send to SIM, CBM shall be send to SIM */
  if ( cmhSAT_CBMIsNewMsg( cbMsg ) )
  {
    if( psaSAT_STKEnvelope (NULL) < 0 )  /* envelope STK command */
    {
      TRACE_EVENT( "FATAL RETURN SAT in send env" );
      CCD_END;
      return( AT_FAIL );
    }
  }

  CCD_END;

  return( AT_CMPL );

}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : SAT                     |
|                                 ROUTINE : cmhSAT_MoSmCntr         |
+-------------------------------------------------------------------+

  PURPOSE : This function supports the MO SM control.

*/

GLOBAL T_ACI_RETURN cmhSAT_MoSmCntr ( T_rp_addr   sc_addr,
                                      T_tp_da     dest_addr,
                                      UBYTE       own)
{

  TRACE_FUNCTION("cmhSAT_MoSmCntr()");

/*
 *-------------------------------------------------------------------
 *  check if Call Control customization is enabled... if yes return AT_CMPL
 *-------------------------------------------------------------------
 */
  
  if(cuscfgParams.MO_SM_Control_SIM EQ CUSCFG_STAT_Enabled)
  {
    return( AT_CMPL );
  }


/*
 *-------------------------------------------------------------------
 *  check if a call control request is in progress
 *-------------------------------------------------------------------
 */
  if( satShrdPrm.SIMCCParm.busy EQ TRUE ) return( AT_BUSY );


/*
 *-------------------------------------------------------------------
 *  build envelope
 *-------------------------------------------------------------------
 */
  CCD_START;
  psaSAT_BuildEnvMoSmCntr ( sc_addr, dest_addr );

  satShrdPrm.SIMCCParm.ccAct = SMC_ACT_MO;
  satShrdPrm.SIMCCParm.owner = own;
  satShrdPrm.SIMCCParm.busy  = TRUE;

  satShrdPrm.owner = OWN_SRC_INV;

  satShrdPrm.Cbch_EvtDnl = FALSE;

  if( psaSAT_STKEnvelope (NULL) < 0 )  /* envelope STK command */
  {
    TRACE_EVENT( "FATAL RETURN SAT in send env" );
    CCD_END;
    return( AT_FAIL );
  }

  CCD_END;
  return( AT_EXCT );

}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : SAT                     |
|                                 ROUTINE : cmhSAT_EventDwn         |
+-------------------------------------------------------------------+

  PURPOSE : This function supports Event download.

*/

GLOBAL T_ACI_RETURN cmhSAT_EventDwn ( UBYTE event, SHORT callId , T_CC_INITIATER actionSrc )
{

  TRACE_FUNCTION("cmhSAT_EventDwn()");

  /* If the event is Location info download, event download is sent only 
     if the location information has changed or has been updated */
  if (event EQ EVENT_LOC_STATUS AND !cmhSAT_verifyNewLocInfo ())
  {
    TRACE_EVENT("No update for SIM app");
    return AT_CMPL;
  }


/*
 *-------------------------------------------------------------------
 *  build envelope
 *-------------------------------------------------------------------
 */
  CCD_START;

  /* if events were queued, send them */
  if (satShrdPrm.event.c_queued)
  {
    do
    {
      T_SAT_QUEUE *p_queue;

      satShrdPrm.event.c_queued--;
      p_queue = &satShrdPrm.event.queued[satShrdPrm.event.c_queued];

      satShrdPrm.owner = p_queue->owner;
      satShrdPrm.Cbch_EvtDnl = TRUE;
      if (psaSAT_STKEnvelope(p_queue->stk_cmd) < 0) /* envelope STK command */
      {
        /* ignore event if error */
        TRACE_EVENT( "FATAL RETURN SAT in send env" );
      }
      p_queue->stk_cmd = NULL;
    }
    while (satShrdPrm.event.c_queued);
  }
  else
  {
    if (!psaSAT_BuildEnvEventDwn ( event , callId, actionSrc ))
    {
      CCD_END;
      return AT_BUSY;
    }

    /*satShrdPrm.SIMCCParm.busy  = TRUE;*/

    satShrdPrm.owner = OWN_SRC_INV;

    satShrdPrm.Cbch_EvtDnl = TRUE;

    if( psaSAT_STKEnvelope (NULL) < 0 )  /* envelope STK command */
    {
      TRACE_EVENT( "FATAL RETURN SAT in send env" );
      CCD_END;
      return( AT_FAIL );
    }
  }

  CCD_END;
  return( AT_CMPL );

}

/*
+-------------------------------------------------------------------+
| PROJECT :                       MODULE  : SAT                     |
|                                 ROUTINE : cmhSAT_LaunchBrowser    |
+-------------------------------------------------------------------+

  PURPOSE :

*/
GLOBAL BOOL cmhSAT_launchBrowser (T_LAUNCH_BROWSER* launchBrowser)
{
  T_ACI_SAT_TERM_RESP resp_data;

  /* check for busy SS condition */
  if (psaSS_stbFindActSrv (NO_ENTRY) NEQ NO_ENTRY)
  {
    /* respond with "error, ME currently unable to process command" */
    resp_data.add_content = ADD_ME_SS_BUSY;
    psaSAT_SendTrmResp( RSLT_ME_UNAB_PROC, &resp_data );
    return( FALSE ); /* unused */
  }

#if defined(MFW) AND defined(FF_WAP)
  rAT_PercentSATBROW (launchBrowser); /* Info 718: Symbol undeclared, assumed to return int */
  return( TRUE ); /* unused */
#else
  cmhSAT_STKCmdInd();
  return( FALSE ); /* unused */
#endif

}


/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : SAT                     |
|                                 ROUTINE : cmhSAT_provLocalInfo    |
+-------------------------------------------------------------------+

  PURPOSE : This function supports Provide Local Information for 
            Date, time and time zone.

*/
GLOBAL BOOL cmhSAT_provLocalInfo ()
{
  T_ACI_SAT_TERM_RESP resp_data;
  rtc_time_type rtc_time;

  pcm_FileInfo_Type fileInfo;
  EF_CLNG lng;


  TRACE_FUNCTION("cmhSAT_provLocalInfo()");

  psaSAT_InitTrmResp( &resp_data );

#ifndef _SIMULATION_
  /* if no service is currently available */
  if ((mmShrdPrm.regStat EQ RS_NO_SRV) OR (mmShrdPrm.regStat EQ NO_VLD_RS))
  {
    psaSAT_SendTrmResp( RSLT_ME_UNAB_PROC, &resp_data );
    return(FALSE);
  }
#endif

  if (satShrdPrm.cmdDet.cmdQlf NEQ QLF_PLOI_DTT AND (satShrdPrm.cmdDet.cmdQlf NEQ QLF_PLOI_LANG_SET))
  {
    return (FALSE);
  }

  if (satShrdPrm.cmdDet.cmdQlf EQ QLF_PLOI_DTT)
  { 
    if ( rtc_read_time ( &rtc_time ) EQ TRUE )
    {
      int i = 0;
      UBYTE hex2bcd;
    
      resp_data.dtt_buf[0] = rtc_time.year;
      resp_data.dtt_buf[1] = rtc_time.month;
      resp_data.dtt_buf[2] = rtc_time.day;
      resp_data.dtt_buf[3] = rtc_time.hour;
      resp_data.dtt_buf[4] = rtc_time.minute;
      resp_data.dtt_buf[5] = rtc_time.second;
      while(i<TIME_STAMP_LENGTH)
      {
        hex2bcd = 0;
        hex2bcd = hex2bcd + resp_data.dtt_buf[i] / 0x0A;
        hex2bcd = hex2bcd << 4;
        hex2bcd = hex2bcd + resp_data.dtt_buf[i] % 0x0A;
        resp_data.dtt_buf[i] = ((hex2bcd & 0x0F) << 4) + ((hex2bcd & 0xF0) >> 4);
        i++;
      }
#ifdef _SIMULATION_
   memset(resp_data.dtt_buf, 0xA0, 6);
#endif /* _SIMULATION_ */

      resp_data.dtt_buf[6] = mmShrdPrm.tz;

      if (mmShrdPrm.regStat EQ RS_LMTD_SRV)
      {
        psaSAT_SendTrmResp( RSLT_PERF_LIM_SRV, &resp_data );
      }
      else
      {
        psaSAT_SendTrmResp( RSLT_PERF_SUCCESS, &resp_data );
      }
    }
    else
    {
      psaSAT_SendTrmResp( RSLT_ME_UNAB_PROC, &resp_data );
      return (FALSE);
    }
  }
  else /* Expecting LS Qualifier */
  {
    /* Implements Measure#32: Row 1023 */
    if (pcm_GetFileInfo ( ( UBYTE* ) ef_clng_id, &fileInfo) NEQ PCM_OK)
    {
      psaSAT_SendTrmResp( RSLT_ME_UNAB_PROC, &resp_data );
      return (FALSE);
    }
    else
    {
      /* Implements Measure#32: Row 1023 */
      if (pcm_ReadFile( (UBYTE*)ef_clng_id,
                         fileInfo.FileSize,
                         (UBYTE*) &lng,
                         &fileInfo.Version) EQ PCM_OK )
      {
                  
        /*-------------------------------------------------------------------
        *  Read EF ELP or LP from the sim if Automatic language is selected 
        *-------------------------------------------------------------------
        */  
 
        /* Implements Measure#32: Row 1024 */
        if (!memcmp(lng.data, au_str, CLAN_CODE_LEN))
        {
        /* Implements Measure 119 */
        if (cmhSIM_ReqLanguagePrf_LP_or_ELP ( SIM_ELP, ACI_LEN_LAN_FLD,
                                              CLANSimEfData,
                                              cmhSIM_RdCnfLangPrfELP) EQ FALSE)
          {
            psaSAT_SendTrmResp( RSLT_ME_UNAB_PROC, &resp_data );
            return (FALSE);
          }
        }
        else
        {
          memcpy(&resp_data.lang, &lng.data[0], CLAN_CODE_LEN);

#ifdef _SIMULATION_
          memset(resp_data.lang, 0xA0, 2);
#endif
          psaSAT_SendTrmResp( RSLT_PERF_SUCCESS, &resp_data );
        }
        
      }
      else
      {
        psaSAT_SendTrmResp( RSLT_ME_UNAB_PROC, &resp_data );
        return (FALSE);
      }  
    }
  }
  return(TRUE);
}


/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : SAT                     |
|                                 ROUTINE : cmhSAT_setupCall        |
+-------------------------------------------------------------------+

  PURPOSE :

*/

GLOBAL BOOL cmhSAT_setupCall ( T_SETUP_CALL * cal )
{
  T_CC_CALL_TBL *ctb;         /* pointer to call table associated with cId */
  T_ACI_RETURN  retVal;       /* holds return value */
  SHORT         cId;          /* holds call id */
  UBYTE         idx;          /* holds index */
  T_ACI_SAT_TERM_RESP resp_data;
#ifdef FF_SAT_E 
  T_ACI_SATA_ADD addPrm;
#endif /* FF_SAT_E */
  SHORT         decodeRet;

  psaSAT_InitTrmResp( &resp_data );

  TRACE_FUNCTION("cmhSAT_setupCall()");

//TISH, patch for OMAPS00141208&OMAPS00141392
//start
#if 0
//TISH patch
//start
    if(mmShrdPrm.regStat NEQ RS_FULL_SRV)
    {
       resp_data.add_content = ADD_ME_NO_SERV;
	psaSAT_SendTrmResp( RSLT_ME_UNAB_PROC, &resp_data );
	return FALSE ;
    }
//end
#else
    if(mmShrdPrm.regStat EQ RS_NO_SRV)
    {
		resp_data.add_content = ADD_ME_NO_SERV;
		psaSAT_SendTrmResp( RSLT_ME_UNAB_PROC, &resp_data );
		return FALSE ;
    }
#endif
// add end 
/*
 *-------------------------------------------------------------------
 *  get call table entry
 *-------------------------------------------------------------------
 */
  cId = psaCC_ctbNewEntry();

  if( cId EQ NO_ENTRY )
  {
    /* send SAT response */
    resp_data.add_content = ADD_NO_CAUSE;
    psaSAT_SendTrmResp( RSLT_ME_UNAB_PROC, &resp_data );
    return( FALSE );  /* primitive not needed anymore */
  }

  ctb = ccShrdPrm.ctb[cId];

/*
 *-------------------------------------------------------------------
 *  build setup parameters
 *-------------------------------------------------------------------
 */
  /* check dial number */
  if( !cal->v_addr )
  {
    /* respond with "error, required values are missing" */
    psaSAT_SendTrmResp( RSLT_ERR_REQ_VAL, &resp_data );
    psaCC_FreeCtbNtry (cId);
    return( FALSE );
  }

  cmhSAT_fillSetupPrm ( cId, /* cId is valid */
                        ((cal->v_addr)?&cal->addr:NULL),
                        ((cal->v_subaddr)?&cal->subaddr:NULL));

//TISH, patch for OMAPS00141208&OMAPS00141392
//start
    if(mmShrdPrm.regStat EQ RS_LMTD_SRV AND ctb->prio EQ 0)  //PRIO_NORM_CALL
    {
		resp_data.add_content = ADD_ME_NO_SERV;
		psaSAT_SendTrmResp( RSLT_ME_UNAB_PROC, &resp_data );
		psaCC_FreeCtbNtry (cId);
		return FALSE ;
    }
//end
  /* check aoc condition */
  if ((ctb->prio EQ MNCC_PRIO_NORM_CALL) AND
      (aoc_check_moc() EQ FALSE))
    /*
     * check ACM exceeds ACMmax
     * for non-emergency calls
     */
  {
    resp_data.add_content = ADD_NO_CAUSE;
    psaSAT_SendTrmResp( RSLT_ME_UNAB_PROC,  &resp_data );
    psaCC_FreeCtbNtry (cId);
    return( FALSE );
  }

  /* declare the owner and status of the call */
  ctb->calOwn     = OWN_SRC_SAT;
  ctb->calStat    = CS_SAT_REQ;
  ctb->curCmd     = AT_CMD_D;
  ctb->SATinv     = TRUE;
 
   /* Flag the Sat Call */
  satShrdPrm.ownSAT = TRUE;

  
  /* Update the Duration */
  cmhSAT_ChckRedial(cId, cal->v_dur, &cal->dur);

  /* bearer capabilities */
  if ( cal->v_cap_cnf_parms EQ 0 )
  {
    /*
     * no bearer capabilities => set to speech
     * this function was called for a SIM_TOOLKIT_IND
     */
    cmhSAT_fillSetupBC ( cId, MNCC_BEARER_SERV_SPEECH, MNCC_BEARER_SERV_NOT_PRES );
    memset( &satShrdPrm.stk_ccp, 0, sizeof(BUF_cap_cnf_parms));
  }
    else
  {
    satShrdPrm.ntfy          = USR_NTF_SETUP_CAL;
    satShrdPrm.capParm.cId   = cId;
    satShrdPrm.capParm.cntxt = CTX_SAT_SETUP;
    
    /* Store the cpp */
    memcpy( &satShrdPrm.stk_ccp, &cal->cap_cnf_parms, sizeof(BUF_cap_cnf_parms));

    /* check cc by sim service is activated and allocated */
    if ( ! psaSIM_ChkSIMSrvSup( SRV_CalCntrl ))
    {
         /* SetUpCall Contains one cap_cnf_parms */ 
        decodeRet = psaCC_BCapDecode ( BCRI_SAT, 
                                       cal->cap_cnf_parms.b_cap_cnf_parms[0],
                                       cal->cap_cnf_parms.b_cap_cnf_parms+1 ,
                                       0,
                                       NULL );
        if ( decodeRet < 0 )
        {
          psaSAT_SendTrmResp( RSLT_ME_UNAB_PROC, &resp_data );
          psaCC_FreeCtbNtry (cId);
          return( FALSE );
        }
        else
        {
          return ( TRUE );
        }
     }
  }

/*
 *-------------------------------------------------------------------
 *  check for emergency call
 *-------------------------------------------------------------------
 */
  if( ctb->prio EQ MNCC_PRIO_EMERG_CALL )
  {
#ifdef TI_PS_FF_AT_P_CMD_CUST
    if (simShrdPrm.overall_cust_mode EQ (UBYTE)CUST_MODE_BEHAVIOUR_1)
    {
      /*
       * Clear the call table entry as it will be re-created when the MMI
       * requests the emergency call setup
       */
      psaCC_FreeCtbNtry (cId);

      /*
      ** Send the original Setup Call Request to the MMI in a %SATI indication
      */
      cmhSAT_Cust1StkCmdInd();
    }
    else
#endif /* TI_PS_FF_AT_P_CMD_CUST */
    {
      /* alert user */
      for( idx = 0; idx < CMD_SRC_MAX; idx++ )
      {

#ifdef FF_SAT_E
        addPrm.chnType = SATA_CT_VOICE;
        addPrm.chnEst  = SATA_EST_IM;

        R_AT( RAT_SATA, (T_ACI_CMD_SRC)idx )( cId+1, 
                               cmhSAT_ChckRedial(cId, cal->v_dur, &cal->dur),
                               &addPrm);
#else  /* FF_SAT_E */
        R_AT( RAT_SATA, (T_ACI_CMD_SRC)idx )( cId+1, 
                               cmhSAT_ChckRedial(cId, cal->v_dur, &cal->dur));
#endif /* FF_SAT_E */ 
      }
    }
            
    satShrdPrm.ntfy = USR_NTF_SETUP_CAL;
    return( TRUE );
  }

/*
 *-------------------------------------------------------------------
 *  check for call control by SIM
 *-------------------------------------------------------------------
 */
  
    retVal = cmhSAT_CalCntrlBySIM( cId );
 

  switch( retVal )
  {
    case( AT_EXCT ):
      satShrdPrm.ntfy = USR_NTF_SETUP_CAL;
      return( TRUE );

    case( AT_BUSY ):
      /* respond with "Interaction with call control by SIM, temporary" */
      psaSAT_SendTrmResp( RSLT_CC_SIM_TMP, &resp_data );
      psaCC_FreeCtbNtry (cId);
      return( FALSE );

    case( AT_FAIL ):
      /* respond with "Interaction with call control by SIM, permanent" */
      resp_data.add_content = ADD_NO_CAUSE;
      psaSAT_SendTrmResp( RSLT_CC_SIM_PRM, &resp_data );
      psaCC_FreeCtbNtry (cId);
      return( FALSE );

    case( AT_CMPL):
      TRACE_EVENT_P1("Restoring the Cid = %d",cId);
      satShrdPrm.SIMCCParm.cId = cId;
      break;
  }

/*
 *-------------------------------------------------------------------
 *  alert user if command details are supported
 *-------------------------------------------------------------------
 */
#ifdef TI_PS_FF_AT_P_CMD_CUST
  if (simShrdPrm.overall_cust_mode EQ (UBYTE)CUST_MODE_BEHAVIOUR_1)
  {
    /*
     * Clear the call table entry as it will be re-created when the MMI
     * requests the emergency call setup
     */
    psaCC_FreeCtbNtry (cId);

    /*
    ** Send the original Setup Call Request to the MMI in a %SATI indication
    */
    cmhSAT_Cust1StkCmdInd();

    satShrdPrm.ntfy = USR_NTF_SETUP_CAL;
    return( TRUE );
  }
  else if( cmhSAT_ChckCmdDet() )
#else
  if( cmhSAT_ChckCmdDet() )
#endif /* TI_PS_FF_AT_P_CMD_CUST */
  {
    for( idx = 0; idx < CMD_SRC_MAX; idx++ )
    {

      /*
      ** If cust_mode is not NORMAL_BEHAVIOUR, then don't do anything yet, because the
      ** %SATI indication will be sent later in the process
      */
#ifdef TI_PS_FF_AT_P_CMD_CUST
      if (simShrdPrm.overall_cust_mode EQ (UBYTE)CUST_NORMAL_BEHAVIOUR)
#endif /* TI_PS_FF_AT_P_CMD_CUST */
      {
#ifdef FF_SAT_E
        addPrm.chnType = SATA_CT_VOICE;
        addPrm.chnEst  = SATA_EST_IM;

        R_AT( RAT_SATA, (T_ACI_CMD_SRC)idx )( cId+1, 
                               cmhSAT_ChckRedial(cId, cal->v_dur, &cal->dur),
                               &addPrm);
#else  /* FF_SAT_E */
        R_AT( RAT_SATA, (T_ACI_CMD_SRC)idx )( cId+1, 
                               cmhSAT_ChckRedial(cId, cal->v_dur, &cal->dur));
#endif /* FF_SAT_E */ 
  
      }
    }

    satShrdPrm.ntfy = USR_NTF_SETUP_CAL;
    return( TRUE );
  }

  psaCC_FreeCtbNtry (cId);
  return( FALSE );
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : SAT                     |
|                                 ROUTINE : cmhSAT_sendSS           |
+-------------------------------------------------------------------+

  PURPOSE : This function starts a SS transaction initiated by SAT.
            If the request is going to be processed, the function
            returns TRUE.

*/

GLOBAL BOOL cmhSAT_sendSS ( T_SEND_SS * ss )
{
  T_ACI_RETURN      retVal;       /* holds return value */
  T_ACI_D_CLIR_OVRD dummy2;       /* dummy value */
  T_ACI_D_TOC       dummy1;       /* dummy value */
  T_ACI_SAT_TERM_RESP resp_data;

  psaSAT_InitTrmResp( &resp_data );

  TRACE_FUNCTION("cmhSAT_sendSS()");

//  TISH patch for OMAPS00109462
    if(mmShrdPrm.regStat NEQ RS_FULL_SRV)
    {
       resp_data.add_content = ADD_ME_NO_SERV;
	psaSAT_SendTrmResp( RSLT_ME_UNAB_PROC, &resp_data );
	return FALSE ;
    }

// add end 
/*
 *-------------------------------------------------------------------
 *  build SS string
 *-------------------------------------------------------------------
 */
  if( (!ss->v_ss_string) OR
      (((ss->v_icon) AND (ss->icon.icon_qu & 0x1)) AND (!ss->v_alpha_id)))
  {
    /*----------------------------------------------------------------
     *  respond with "error, required values are missing" OR
     *  Icon information present in SIM_TOOLKIT_IND and icon qualifier is
     *  not self-explanatory(11.14) and no alpha identifier is present, 
     *  response should be given with Command data not understood by ME
     *----------------------------------------------------------------
     */
    psaSAT_SendTrmResp( RSLT_UNKN_DATA, &resp_data );
    return( FALSE );
  }
  
  if ( !ss->ss_string.c_ss_ctrl_string )
  {
   psaSAT_SendTrmResp(RSLT_ERR_REQ_VAL, &resp_data );
   return( FALSE );
  }


  cmhCC_init_cldPty( &satPndSetup.clpty );

  utl_BCD2DialStr( ss->ss_string.ss_ctrl_string, satPndSetup.clpty.num,
                   (UBYTE)MINIMUM(ss->ss_string.c_ss_ctrl_string,
                                  MAX_DIAL_LEN-1));

  satPndSetup.clpty.ton = ss->ss_string.noa;
  satPndSetup.clpty.npi = ss->ss_string.npi;

/*
 *-------------------------------------------------------------------
 *  check for call control by SIM
 *-------------------------------------------------------------------
 */
  retVal = cmhSAT_SSCntrlBySIM( &satPndSetup.clpty, OWN_SRC_SAT );

  switch( retVal )
  {
    case( AT_EXCT ):
      satShrdPrm.ntfy = USR_NTF_SEND_SS;
      return( TRUE );

    case( AT_BUSY ):
      /* respond with "Interaction with call control by SIM, temporary" */
      psaSAT_SendTrmResp( RSLT_CC_SIM_TMP, &resp_data );
      return( FALSE );

    case( AT_FAIL ):
      /* respond with "Interaction with call control by SIM, permanent" */
      resp_data.add_content = ADD_NO_CAUSE;
      psaSAT_SendTrmResp( RSLT_CC_SIM_PRM, &resp_data );
      return( FALSE );
  }

/*
 *-------------------------------------------------------------------
 *  check for busy SS condition
 *-------------------------------------------------------------------
 */
  if( psaSS_stbFindActSrv( NO_ENTRY ) NEQ NO_ENTRY )
  {
    /* respond with "error, ME currently unable to process command" */
    resp_data.add_content = ADD_ME_SS_BUSY;
    psaSAT_SendTrmResp( RSLT_ME_UNAB_PROC, &resp_data );
    return( FALSE );
  }

#ifdef TI_PS_FF_AT_P_CMD_CUST
  if (simShrdPrm.overall_cust_mode EQ (UBYTE)CUST_MODE_BEHAVIOUR_1)
  {
        /*
        ** Send the original SendSS Request to the MMI in a %SATI indication
        */
        cmhSAT_Cust1StkCmdInd();

        satShrdPrm.ntfy = USR_NTF_SEND_SS;
        return( TRUE );
  }
#endif /* TI_PS_FF_AT_P_CMD_CUST */
/*
 *-------------------------------------------------------------------
 *  decode and send SS string
 *-------------------------------------------------------------------
 */
  retVal = cmhCC_chkKeySeq ( (T_ACI_CMD_SRC)OWN_SRC_SAT,
                             &satPndSetup.clpty,
                             &dummy1,
                             &dummy2,
                             CC_SIM_NO );
  if( retVal NEQ AT_EXCT )
  {
    /* respond with "error, beyond ME capabilities" */
    psaSAT_SendTrmResp( RSLT_ME_CAP, &resp_data );
    return( FALSE );
  }

  satShrdPrm.ntfy = USR_NTF_SEND_SS;
  return( TRUE );

}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : SAT                     |
|                                 ROUTINE : cmhSAT_sendUSSD         |
+-------------------------------------------------------------------+

  PURPOSE : This function starts a USSD transaction initiated by SAT.
            If the request is going to be processed, the function
            returns TRUE.

*/

GLOBAL BOOL cmhSAT_sendUSSD ( T_SEND_USSD * ussd )
{
  T_ACI_RETURN         retVal;       /* holds return value */
  SHORT                sId;          /* holds service id */
  T_ACI_SAT_TERM_RESP  resp_data;
  T_sat_ussd           SATCC_ussd; /* to hold USSD string in case of SAT Control */

  psaSAT_InitTrmResp( &resp_data );

  TRACE_FUNCTION("cmhSAT_sendUSSD()");

//TISH, patch for OMAPS00115020
//start
    if(mmShrdPrm.regStat NEQ RS_FULL_SRV)
    {
       resp_data.add_content = ADD_ME_NO_SERV;
	psaSAT_SendTrmResp( RSLT_ME_UNAB_PROC, &resp_data );
	return FALSE ;
    }

// add end 
/*
 *-------------------------------------------------------------------
 *  build SS string
 *-------------------------------------------------------------------
 */
  if( !ussd->v_ussd_string )
  {
    /* respond with "error, required values are missing" */
    psaSAT_SendTrmResp( RSLT_ERR_REQ_VAL, &resp_data );
    return( FALSE );
  }

/*
 *-------------------------------------------------------------------
 *  Icon information present in SIM_TOOLKIT_IND and icon qualifier is
 *  not self-explanatory(11.14) and no alpha identifier is present, 
 *  response should be given with Command data not understood by ME.
 *-------------------------------------------------------------------
 */
  if(((ussd->v_icon) AND (ussd->icon.icon_qu & 0x1)) AND (!ussd->v_alpha_id))
  {
    psaSAT_SendTrmResp(RSLT_UNKN_DATA, &resp_data );
    return( FALSE );
  }
/*
 *-------------------------------------------------------------------
 *  check for call control by SIM
 *-------------------------------------------------------------------
 */
  if ( psaSIM_ChkSIMSrvSup(SRV_USSDsupportInCC) )
  {
    SATCC_ussd.dcs        = ussd->ussd_string.dcs;
    SATCC_ussd.c_ussd_str = ussd->ussd_string.c_ussd_str;
    SATCC_ussd.ussd_str   = ussd->ussd_string.ussd_str;

    retVal = cmhSAT_USSDCntrlBySIM( &SATCC_ussd, OWN_SRC_SAT );

    switch( retVal )
    {
      case( AT_EXCT ):
        satShrdPrm.ntfy = USR_NTF_SEND_USSD;
        return( TRUE );

      case( AT_BUSY ):
        /* respond with "Interaction with call control by SIM, temporary" */
        psaSAT_SendTrmResp( RSLT_CC_SIM_TMP, &resp_data );
        return( FALSE );

      case( AT_FAIL ):
        /* respond with "Interaction with call control by SIM, permanent" */
        resp_data.add_content = ADD_NO_CAUSE;
        psaSAT_SendTrmResp( RSLT_CC_SIM_PRM, &resp_data );
        return( FALSE );
    }
  }

  /***********************************************************
  check if a SS or a USSD transaction is already in process
  ***********************************************************/

  sId = psaSS_stbFindActSrv( NO_ENTRY );

  if( sId NEQ NO_ENTRY )
  {
    if (ssShrdPrm.stb[sId].curCmd EQ ((T_ACI_AT_CMD)KSD_CMD_USSD)
      OR ssShrdPrm.stb[sId].curCmd EQ AT_CMD_CUSD)
      resp_data.add_content = ADD_ME_USSD_BUSY;
    else
      resp_data.add_content = ADD_ME_SS_BUSY;

    resp_data.addLen = 1;

    psaSAT_SendTrmResp( RSLT_ME_UNAB_PROC, &resp_data );
    return( FALSE );
  }

#ifdef TI_PS_FF_AT_P_CMD_CUST
  if (simShrdPrm.overall_cust_mode EQ (UBYTE)CUST_MODE_BEHAVIOUR_1)
  {
        /*
        ** Send the original SendSS Request to the MMI in a %SATI indication
        */
        cmhSAT_Cust1StkCmdInd();

        satShrdPrm.ntfy = USR_NTF_SEND_USSD;
        return( TRUE );
  }
#endif /* TI_PS_FF_AT_P_CMD_CUST */

    /* get new service table entry */
    sId = psaSS_stbNewEntry();
    if( sId EQ NO_ENTRY )
    {
      resp_data.add_content = ADD_ME_USSD_BUSY;
      resp_data.addLen = 1;
      psaSAT_SendTrmResp( RSLT_ME_UNAB_PROC, &resp_data );
      return( FALSE );
    }

    CCD_START;

    /* TLU: save DCS for USSD v1 */
    ssShrdPrm.ussdDcs = ussd->ussd_string.dcs;

    /* set data coding scheme */
    /* patch !!!!! CLB 11/12/01      */
    if( (UBYTE)ussd->ussd_string.dcs EQ 0x40 )
    {
      /* 0x40 means basically default alphabet...
      yet some asian networks dont seem to accept it (although using it
      in their own STK !!!) */
      ussd->ussd_string.dcs = 0x0F;
    }
    /*********************************/

    psaSS_asmProcUSSDReq( (UBYTE)ussd->ussd_string.dcs,
                          ussd->ussd_string.ussd_str,
                          ussd->ussd_string.c_ussd_str );

    /* start new transaction */
    ssShrdPrm.stb[sId].ntryUsdFlg = TRUE;
    ssShrdPrm.stb[sId].curCmd     = AT_CMD_NONE;
    ssShrdPrm.stb[sId].srvOwn     = OWN_SRC_SAT;

    /* save ussd string for possible version 1 retry */
    /* if( cmhSMS_getAlphabetCb( (UBYTE)dcs ) EQ 0 )      NOT SURE ABOUT THAT !!!! clb */
    {
      if( ussd->ussd_string.c_ussd_str <= MAX_USSD_STRING )
      {
        ssShrdPrm.ussdLen = ussd->ussd_string.c_ussd_str;
        memcpy( ssShrdPrm.ussdBuf, 
                ussd->ussd_string.ussd_str,
                ussd->ussd_string.c_ussd_str );
      }
    }
    
    satShrdPrm.SentUSSDid = sId;  /* save for response message */

    psaSS_NewTrns(sId);

    CCD_END;

  return( TRUE );

}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : SAT                     |
|                                 ROUTINE : cmhSAT_sendDTMF         |
+-------------------------------------------------------------------+

  PURPOSE : This function sends a DTMF sequence to the network.

*/

LOCAL BOOL cmhSAT_initDTMF ( T_SEND_DTMF *dtmf )
{
  SHORT               cId;              /* holds call id */
  T_ACI_SAT_TERM_RESP resp_data;
  CHAR                num[MNCC_MAX_CC_CALLED_NUMBER];
  UBYTE               i;
  USHORT              length;


  TRACE_FUNCTION("cmhSAT_initDTMF()");

  psaSAT_InitTrmResp( &resp_data );

  cId = cmhCC_find_call_for_DTMF( );
  if (cId EQ NO_ENTRY)
  {
    /* respond with "error, no speech call in process" */
    resp_data.add_content = ADD_ME_NO_SPCH_CALL;
    psaSAT_SendTrmResp( RSLT_ME_UNAB_PROC, &resp_data );
    return( FALSE );
  }

  /* check arguments */
  if( dtmf AND !dtmf->v_dtmf_string )
  {
    /* respond with "error, required values are missing" */
    psaSAT_SendTrmResp( RSLT_ERR_REQ_VAL, &resp_data );
    return( FALSE );
  }
  else
  {
    length = MINIMUM((dtmf ? dtmf->dtmf_string.c_bcdDigit : 0), MNCC_MAX_CC_CALLED_NUMBER);
    if (dtmf NEQ NULL)
    {
      for(i=0; i<length; i++)
      { 
        num[i] = cmhPHB_convertBCD2char(dtmf->dtmf_string.bcdDigit[i]);
      }
    }
    else
      num[0] = '\0';
  
    cmhCC_chkDTMFDig( num,
                      cId,
                      length,
                      FALSE );

    ccShrdPrm.dtmf.cId = cId;               /* Update the global parameter with cId */
    psaCC_ctb(cId)->dtmfCmd = AT_CMD_NONE ;
    psaCC_ctb(cId)->dtmfSrc = OWN_SRC_SAT;  /* wait for confirmation */
    return TRUE;
  }
}


GLOBAL BOOL cmhSAT_sendDTMF ( T_SEND_DTMF *dtmf )
{
  SHORT               cId;           /* holds call id */
  T_ACI_SAT_TERM_RESP resp_data;
  BOOL                param_ok;

  TRACE_FUNCTION("cmhSAT_sendDTMF()");

  psaSAT_InitTrmResp( &resp_data );

//  TISH patch for OMAPS00109462
    if(mmShrdPrm.regStat NEQ RS_FULL_SRV)
    {
       resp_data.add_content = ADD_ME_NO_SERV;
	psaSAT_SendTrmResp( RSLT_ME_UNAB_PROC, &resp_data );
	return FALSE ;
    }

// add end 
  /* if no proceeding yet, check for a voice call */
  if( ccShrdPrm.dtmf.cId NEQ NO_ENTRY )
  {
    cId = ccShrdPrm.dtmf.cId;
    if (ccShrdPrm.ctb[cId] EQ NULL OR
        psaCC_ctb(cId)->dtmfSrc NEQ OWN_SRC_SAT )
    {
      /* respond with "error, busy on DTMF" */
      /* resp_data.add_content = ADD_ME_DTMF_BUSY; */
      psaSAT_SendTrmResp( RSLT_ME_UNAB_PROC, &resp_data );
      return( FALSE );
    }
  }
  else
  {
    /* initialize DTMF related parameters */
    if (!cmhSAT_initDTMF( dtmf ))
      return (FALSE);
    cId = ccShrdPrm.dtmf.cId;
  }

  /* send DTMF */
  param_ok = cmhCC_SendDTMFdig ( AT_CMD_NONE, cId,
                                 ccShrdPrm.dtmf.dig[ccShrdPrm.dtmf.cur],
                                 MNCC_DTMF_MOD_AUTO);
  ccShrdPrm.dtmf.cur++;

  if( !param_ok )
  {
    /* respond with "value not recognized" */
    psaSAT_SendTrmResp( RSLT_ERR_REQ_VAL, &resp_data );
    return( FALSE );
  }
  return( TRUE );
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : SAT                     |
|                                 ROUTINE : cmhSAT_runAt            |
+-------------------------------------------------------------------+

  PURPOSE : This function performs run at command for SAT.
*/

GLOBAL BOOL cmhSAT_runAt ( T_RUN_AT *run_at)
{
  T_ACI_SAT_TERM_RESP resp_data;

  psaSAT_InitTrmResp( &resp_data );

  TRACE_FUNCTION("cmhSAT_runAt()");

  if(!run_at->v_at_string)
  {
    /* respond with "error, required values are missing" */
    psaSAT_SendTrmResp( RSLT_ERR_REQ_VAL, &resp_data );
    return(FALSE);
  }

#ifdef FF_ATI
  if( sat_src_proc_chars (run_at->at_string) EQ FALSE )
  {
    return(FALSE);
  }
#endif /* FF_ATI */

  return(TRUE);
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : SAT                     |
|                                 ROUTINE : cmhSAT_setupEvent_Test  |
+-------------------------------------------------------------------+

  PURPOSE : This function updates the event list to be whatched for SAT.
*/

GLOBAL BOOL cmhSAT_setupEvent_Test ( T_SETUP_EVENT *set_event, 
                                      BOOL *aci_events_only )
{
  SHORT i;
  BOOL mt_done = FALSE;
  BOOL conn_done = FALSE;
  BOOL disc_done = FALSE;
  BOOL loc_done = FALSE;
#ifdef FF_SAT_E  
  BOOL dat_avail = FALSE;
  BOOL chn_stat = FALSE;
#endif /* FF_SAT_E */
  BOOL list_in_process;

  /* ACI-SPR-18200: temporary event list, not processed by ACI */
  UBYTE tmpEventList[MAX_EVENT_LIST_LEN];
  UBYTE tmpEventListLen = 0; 

  TRACE_FUNCTION("cmhSAT_setupEvent_Test()");

  if (set_event -> v_ev_list EQ TRUE)
  {


    /* erase previous list if received list is empty */
    if (set_event -> ev_list.c_event EQ 0L)
    {
      satShrdPrm.event.list = 0L;
      return TRUE;
    }

     /* test events whether they are supported by ACI or not...
     Moreover, every event shall take place only once in the list */
     /* supported by ACI:
                                EVENT_MT_CALL
                                EVENT_CALL_CONN
                                EVENT_CALL_DISC
                                EVENT_LOC_STATUS
                                EVENT_DATA_AVAIL (SAT E)
                                EVENT_CHAN_STAT  (SAT E) */

    list_in_process = ( satShrdPrm.event.temp_list & 0x8000 ) > 0;

    if ( list_in_process )
    {
      /*means a setup_event_list is already currently in process */
      return FALSE; /* TR(30) is sent by calling function */ 
    }

    for (i=0;i<set_event -> ev_list.c_event;i++)
    {
      switch( set_event -> ev_list.event[i] )
      {
      case( EVENT_MT_CALL ):
        if (! mt_done )
        {
          mt_done = TRUE;
        }
        else
        {
          satShrdPrm.event.temp_list = 0L;
          return FALSE;
        }
        break;

      case( EVENT_CALL_CONN ):
        if (! conn_done )
        {
          conn_done = TRUE;
        }
        else
        {
          satShrdPrm.event.temp_list = 0L;
          return FALSE;
        }
        break;

      case( EVENT_CALL_DISC ):
        if (! disc_done )
        {
          disc_done = TRUE;
        }
        else
        {
          satShrdPrm.event.temp_list = 0L;
          return FALSE;
        }
        break;

      case( EVENT_LOC_STATUS ):
        if (! loc_done )
        {
          loc_done = TRUE;
        }
        else
        {
          satShrdPrm.event.temp_list = 0L;
          return FALSE;
        }
        break;

#ifdef FF_SAT_E         
      case( EVENT_DATA_AVAIL ):
        if (! dat_avail )
        {
          dat_avail = TRUE;
        }
        else
        {
          satShrdPrm.event.temp_list = 0L;
          return FALSE;
        }
        break;
 
      case( EVENT_CHAN_STAT ):
        if (! chn_stat )
        {
          chn_stat = TRUE;
        }
        else
        {
          satShrdPrm.event.temp_list = 0L;
          return FALSE;
        }
        break;        
#endif /* FF_SAT_E */ 

      default:
        satShrdPrm.event.temp_list |= (0x01 << set_event->ev_list.event[i]);
        /* so that the corresponding bit won't actually be set in temp_list */
        /* ACI-SPR-18200: add to local temp event list */
        tmpEventList[tmpEventListLen] = set_event->ev_list.event[i];
        tmpEventListLen++;
        break;
      }
      satShrdPrm.event.temp_list |= (0x01 << set_event->ev_list.event[i]);
    }
  }
  else
  {
    satShrdPrm.event.temp_list = 0L;
    return FALSE;
  }

  satShrdPrm.event.temp_list |= 0x8000; /* to flag that a list is in process */

  /* ACI-SPR-18200: Modify the SAT cmd send to MMI */ 
  if ( cmhSAT_copyUnprocEvents ( tmpEventList,
                                 tmpEventListLen,
                                 set_event->ev_list.c_event ) ) 
  {
    *aci_events_only = FALSE;
  }
  else 
  {
    *aci_events_only = TRUE;
  }
  
  return TRUE;
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : SAT                     |
|                                 ROUTINE : cmhSAT_copyUnprocEvents |
+-------------------------------------------------------------------+

  PURPOSE : Modify the event list that will be send to MMI. 
            Insert the events that could not be processed by ACI.
            Returning TRUE means: Forward the current SAT command 
            to MMI for further analysis.
*/

GLOBAL BOOL cmhSAT_copyUnprocEvents ( UBYTE* eventList,
                                      UBYTE  eventListLen,
                                      UBYTE  oldEventListLen)
{
  USHORT totalLen; 
  UBYTE  lenDiff;
  UBYTE  i; 
  UBYTE  j;
  UBYTE  shift = 0; /* Holds the shift value */

  TRACE_FUNCTION("cmhSAT_copyUnprocEvents()");
  
  /* No modification needed, all events processed in ACI */ 
  if ( (eventListLen EQ 0) AND (oldEventListLen NEQ 0) )
  {    
    return FALSE;  
  } 
  /* No modifiction necessary, all events are forwarded */
  else if ( eventListLen EQ oldEventListLen )
  {
    return TRUE;
  }
  else 
  {
    /* modify SAT event list string */ 
    /* calculate difference between new and old length of event list */ 
    lenDiff = oldEventListLen - eventListLen;

    /* modify the total length, always second (and third) byte 
       Note: This is the only length that is critical, if the length 
       decrement changes the total len from 2 to 1 byte representation
       the whole array has to be shifted one position at the end of 
       modification. */
    if ( satShrdPrm.stkCmd[1] EQ 0x81 )
    {
      if ( (satShrdPrm.stkCmd[2] > 0x7F) AND
           ((satShrdPrm.stkCmd[2] - lenDiff) <= 0x7F )) /* mod changes repres.*/ 
      {
        /* forces a shift at the end of modification */
        shift = 1;
      }
      satShrdPrm.stkCmd[2] = satShrdPrm.stkCmd[2] - lenDiff;
      i = 3;
    }
    else
    {    
      satShrdPrm.stkCmd[1] = satShrdPrm.stkCmd[1] - lenDiff;
      i = 2;
    }

    /* go to index of SET UP EVENT LIST TAG */ 
    while( i < satShrdPrm.stkCmdLen ) 
    {
      /* Is tag set up event list TAG */ 
      if ( satShrdPrm.stkCmd[i] NEQ 0x99 )
      {
        /* go LENGTH steps ahead */
        /* jump over the VALUE field to the next TAG */
        if ( satShrdPrm.stkCmd[i+1] EQ 0x81 ) /* 2 byte length representation */ 
        {
          i = i + ( satShrdPrm.stkCmd[i+2] + 3 );
        }
        else
        {
          i = i + ( satShrdPrm.stkCmd[i+1] + 2 );
        }
      }
      else
      {
        /* found index */
        break;
      }    
    }

    /* insert new events and decrease length */
    if ( (i < satShrdPrm.stkCmdLen) AND  
         (satShrdPrm.stkCmd[i] EQ 0x99) )
    {      
      /* set index to LENGTH */
      i++;
      /* decrease length */
      if ( satShrdPrm.stkCmd[i] EQ 0x81 ) /* 2 byte length representation */
      {
        if ( satShrdPrm.stkCmd[i+1] - lenDiff <= 0x7F ) /*repres. changes to 1*/
        {
          satShrdPrm.stkCmd[i] = satShrdPrm.stkCmd[i+1] - lenDiff;
          /* set index just one byte ahead to overwrite second length byte */
          i++;
        }
        else 
        {
          satShrdPrm.stkCmd[i+1] = satShrdPrm.stkCmd[i+1] - lenDiff;
          /* set index to value */
          i+=2;
        }        
      }
      else /* just change the length */
      {
        satShrdPrm.stkCmd[i] = satShrdPrm.stkCmd[i] - lenDiff;      
        /* set index to VALUE */
        i++;      
      }
      /* write out events that have to be forwarded to MMI */
      for( j = 0; j < eventListLen; j++)
      {
        satShrdPrm.stkCmd[i+j] = eventList[j];        
      }
      
      /* set index to end */
      i = i + j;
      /* clear rest of array */
      if ( i < satShrdPrm.stkCmdLen )
      {
        while ( i < satShrdPrm.stkCmdLen )
        {
          satShrdPrm.stkCmd[i] = 0xFF;
          i++; 
        }
      }
      
    } 
    else
    {
      TRACE_EVENT("ERROR: Could not find SET UP EVENT LIST TAG");
      /* reset temp list to force error response */ 
      satShrdPrm.event.temp_list = 0L; 
      return FALSE;  
    } 

    /* if shift is needed */
    /* set index to first byte of total length */
    i = 2;
    if ( shift )
    {
      for ( i = 2; i < (satShrdPrm.stkCmdLen - 1); i++ ) 
      {
        satShrdPrm.stkCmd[i] = satShrdPrm.stkCmd[i+1];
      }
    }
    
    /* decrease the total length */
    totalLen = satShrdPrm.stkCmdLen - (lenDiff*8);
    satShrdPrm.stkCmdLen = totalLen;
    
    return TRUE;
  }    
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : SAT                     |
|                                 ROUTINE : cmhSAT_setupEvent_Perform|
+-------------------------------------------------------------------+

  PURPOSE : This function updates the event list to be whatched for SAT.
*/

GLOBAL void cmhSAT_setupEvent_Perform ( void )
{
  TRACE_FUNCTION("cmhSAT_setupEvent_Perform()");

  /* update list */
  satShrdPrm.event.list = satShrdPrm.event.temp_list;

  satShrdPrm.event.temp_list = 0L; /* reinitialize for next setup list */

#if defined (FF_SAT_E) AND defined (DTI)
  /* inform SIM about a possible DATA AVAIL event */
  psaSIM_EvDatAvail(psaSAT_ChkEventList( EVENT_DATA_AVAIL ));
#endif /* FF_SAT_E */

  /*psaSAT_SendTrmResp( RSLT_PERF_SUCCESS, 0, NULL, NULL );
  not needed... what comes from MMI is sent to SAT */
}
/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : SAT                     |
|                                 ROUTINE : cmhSAT_UserRejCall      |
+-------------------------------------------------------------------+

  PURPOSE : This function sends a terminal response, indicating that
            the user has not accept the SAT call.

*/

GLOBAL void cmhSAT_UserRejCall ( UBYTE calStat )
{
  T_ACI_SAT_TERM_RESP resp_data;

  TRACE_FUNCTION("cmhSAT_UserRejCall()");

  psaSAT_InitTrmResp( &resp_data );

#ifdef FF_SAT_E
  if( calStat EQ CS_SAT_CSD_REQ )
  {
    resp_data.bearDesc = TRUE;
    resp_data.bufSize = TRUE;
  }
  
#endif /* FF_SAT_E */

  psaSAT_SendTrmResp( RSLT_USR_REJ, &resp_data );

#ifdef FF_SAT_E 
  if( calStat EQ CS_SAT_CSD_REQ )
  {
    cmhSAT_cleanupOpChnPrms();
  }  
#endif /* FF_SAT_E */
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : SAT                     |
|                                 ROUTINE : cmhSAT_UserRejCntxt     |
+-------------------------------------------------------------------+

  PURPOSE : This function sends a terminal response, indicating that
            the user has not accepted the SAT GPRS context.

*/

#if defined (GPRS) && defined (FF_SAT_E)
GLOBAL void cmhSAT_UserRejCntxt( void )
{
  T_ACI_SAT_TERM_RESP resp_data;

  TRACE_FUNCTION("cmhSAT_UserRejCntxt()");

  psaSAT_InitTrmResp( &resp_data );
  resp_data.bearDesc = TRUE;
  resp_data.bufSize = TRUE;

  psaSAT_SendTrmResp( RSLT_USR_REJ, &resp_data );

  cmhSAT_cleanupOpChnPrms();

}
#endif /* GPRS && SAT E */

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : SAT                     |
|                                 ROUTINE : cmhSAT_CallCncted       |
+-------------------------------------------------------------------+

  PURPOSE : This function sends a terminal response, indicating that
            the SAT call was connected.

*/

GLOBAL void cmhSAT_CallCncted ( void )
{
  T_ACI_SAT_TERM_RESP resp_data;

  TRACE_FUNCTION("cmhSAT_CallCncted()");
  
  psaSAT_InitTrmResp( &resp_data );
  psaSAT_SendTrmResp( RSLT_PERF_SUCCESS, &resp_data );
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : SAT                     |
|                                 ROUTINE : cmhSAT_NtwErr           |
+-------------------------------------------------------------------+

  PURPOSE : This function sends a terminal response, indicating that
            the SAT call was not connected due to a network problem.
            The function will return TRUE if it is OK to send a AT
            result code. If a result code should be suppressed, the
            function will return FALSE.

*/

GLOBAL UBYTE cmhSAT_NtwErr ( UBYTE cs )
{

  T_ACI_SAT_TERM_RESP resp_data;
#if defined (FF_SAT_E) AND defined (DTI)
  T_SIM_SAT_CHN chnInf;
#endif 
  UBYTE ret = TRUE;

  TRACE_FUNCTION("cmhSAT_NtwErr()");

  psaSAT_InitTrmResp( &resp_data );

  resp_data.add_content = cs;

#if defined (FF_SAT_E) AND defined (DTI)
  /* in case of a SEND DATA command, close the channel. Terminal response
     will be sent by SIM entity */
  if( satShrdPrm.chnTb.chnUsdFlg AND 
      satShrdPrm.cmdDet.cmdType EQ SAT_CMD_SEND_DATA)
  {
    chnInf.dtiConn = SIM_DTI_UNKNOWN; 
    chnInf.bipConn = SIM_BIP_CLOSE_CHANNEL;
    chnInf.chnId   = CHANNEL_ID_1;
    chnInf.genRes  = RSLT_NTW_UNAB_PROC;
    chnInf.addRes  = cs;

    psaSIM_SATBIPChn( chnInf, cmhSAT_OpChnClose );
    ret = FALSE;
  }
  /* otherwise send a terminal response */
  else
  {
    if( satShrdPrm.opchStat EQ OPCH_EST_REQ AND
        satShrdPrm.opchType EQ B_CSD )
    {
       resp_data.chnStat = TRUE;
    }
#endif /* FF_SAT_E */

    psaSAT_SendTrmResp( RSLT_NTW_UNAB_PROC, &resp_data );

#if defined (FF_SAT_E) AND defined (DTI)
  }

  if( satShrdPrm.opchStat EQ OPCH_EST_REQ AND
      satShrdPrm.opchType EQ B_CSD )
  {
     satShrdPrm.chnTb.chnUsdFlg = FALSE;
     cmhSAT_cleanupOpChnPrms();
  }
#endif /* FF_SAT_E */

  return ret;
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : SAT                     |
|                                 ROUTINE : cmhSAT_UserClear        |
+-------------------------------------------------------------------+

  PURPOSE : This function sends a terminal response, indicating that
            the SAT call was not connected due to a user clearance.

*/

GLOBAL void cmhSAT_UserClear ( void )
{
  T_ACI_SAT_TERM_RESP resp_data;
#if defined (FF_SAT_E) AND defined (DTI)
  T_SIM_SAT_CHN chnInf;
#endif /* FF_SAT_E */

  TRACE_FUNCTION("cmhSAT_UserClear()");

  psaSAT_InitTrmResp( &resp_data );

#if defined (FF_SAT_E) AND defined (DTI)
  /* in case of a SEND DATA command, close the channel. Terminal response
     will be sent by SIM entity */
  if( satShrdPrm.chnTb.chnUsdFlg AND 
      satShrdPrm.cmdDet.cmdType EQ SAT_CMD_SEND_DATA)
  {
    chnInf.bipConn = SIM_BIP_CLOSE_CHANNEL;
    chnInf.dtiConn = SIM_DTI_UNKNOWN; 
    chnInf.chnId   = CHANNEL_ID_1;
    chnInf.genRes  = RSLT_USR_CLR_DWN;
    chnInf.addRes  = ADD_NO_CAUSE;

    psaSIM_SATBIPChn( chnInf, cmhSAT_OpChnClose );
  }
  /* otherwise send a terminal response */
  else
  {
    if( satShrdPrm.opchStat EQ OPCH_EST_REQ AND
        satShrdPrm.opchType EQ B_CSD )
    {
       resp_data.chnStat = TRUE;
    }
#endif /* FF_SAT_E */
    psaSAT_SendTrmResp( RSLT_USR_CLR_DWN, &resp_data );

#if defined (FF_SAT_E) AND defined (DTI)
  }

  if( satShrdPrm.opchStat EQ OPCH_EST_REQ AND
      satShrdPrm.opchType EQ B_CSD )
  {
     satShrdPrm.chnTb.chnUsdFlg = FALSE;
     cmhSAT_cleanupOpChnPrms();
  }
#endif /* FF_SAT_E */
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : SAT                     |
|                                 ROUTINE : cmhSAT_UserAcptCall     |
+-------------------------------------------------------------------+

  PURPOSE : This function performs the user acceptance of the pending
            SAT call. If command details do not allow the operation,
            the function returns FALSE.

*/

GLOBAL BOOL cmhSAT_UserAcptCall ( SHORT acptId, UBYTE srcId )
{
  T_CC_CMD_PRM * pCCCmdPrm; /* points to CC command parameters */
  SHORT actId;              /* holds id of active call */
  UBYTE ctbIdx;             /* holds call table index */
  BOOL found_call=FALSE;

#if defined (FAX_AND_DATA) AND defined (FF_SAT_E)
  T_SIM_SAT_CHN chnInf;
#endif

  TRACE_FUNCTION("cmhSAT_UserAcptCall()");

  pCCCmdPrm = &cmhPrm[srcId].ccCmdPrm;

  pCCCmdPrm -> mltyCncFlg = 0;
  pCCCmdPrm -> mltyDscFlg = 0;

  psaCC_ctb(acptId)->curCmd = AT_CMD_A;
  psaCC_ctb(acptId)->curSrc = srcId;

/*
 *-------------------------------------------------------------------
 *  perform according to command details for SETUP CALL command
 *-------------------------------------------------------------------
 */
  if (psaCC_ctb(acptId)->calStat EQ CS_SAT_REQ)
  {
    if( !cmhSAT_ChckCmdDet() )
    {
      psaCC_FreeCtbNtry (acptId);
      return( FALSE );
    }

    switch( satShrdPrm.cmdDet.cmdQlf )
    {
      case( QLF_CALL_HOLD ):
      case( QLF_CALL_HOLD_RDL ):

        actId = psaCC_ctbFindCall( OWN_SRC_INV, CS_ACT, NO_VLD_CT );

        if( actId NEQ NO_ENTRY )
        {
          psaCC_ctb(actId)->SATinv = TRUE;

          cmhCC_HoldCall(actId, (T_ACI_CMD_SRC)psaCC_ctb(acptId)->curSrc, AT_CMD_A);

          if( psaCC_ctb(actId)->mptyStat NEQ CS_ACT )
          {
            /* do not start building sat call, has to wait for answer for holding */
            return( TRUE );
          }
        }
        break;

      case( QLF_CALL_DISC ):
      case( QLF_CALL_DISC_RDL ):

        for( ctbIdx = 0; ctbIdx < MAX_CALL_NR; ctbIdx++ )
        {
          T_CC_CALL_TBL *ctbx = ccShrdPrm.ctb[ctbIdx];

          if (ctbx NEQ NULL AND
              ctbIdx NEQ acptId )
          {
            cmhCC_flagCall( ctbIdx, &(pCCCmdPrm -> mltyDscFlg));
            ctbx->nrmCs  = MNCC_CAUSE_CALL_CLEAR;
            ctbx->curCmd = AT_CMD_H;
            ctbx->curSrc = psaCC_ctb(acptId)->curSrc;
            ctbx->SATinv = TRUE;
            psaCC_ClearCall (ctbIdx);
            found_call   = TRUE;
          }
        }

        if (found_call)
        {
          /* do not start building sat call, has to wait for answer for clearing */
          return( TRUE );
        }
        break;
    }
  }

/*
 *-------------------------------------------------------------------
 *  perform according to command details for OPEN CHANNEL command
 *-------------------------------------------------------------------
 */
#if defined (FAX_AND_DATA) AND defined (FF_SAT_E) AND defined (DTI)

  if( psaCC_ctb(acptId)->calStat EQ CS_SAT_CSD_REQ )
  {

    /* reset own to SAT */
    psaCC_ctb(acptId)->calOwn = OWN_SRC_SAT;

    /* store current command source */
    satShrdPrm.opchAcptSrc = srcId;

    if( satShrdPrm.opchPrm AND 
        ((T_SAT_CSD_PRM*)satShrdPrm.opchPrm)->v_itl AND
        ((T_SAT_CSD_PRM*)satShrdPrm.opchPrm)->itl.trans_prot_type EQ UDP )
    {
      satShrdPrm.chnTb.chnTPL = ((T_SAT_CSD_PRM*)satShrdPrm.opchPrm)->itl.trans_prot_type;
    }
    else
    {
      satShrdPrm.chnTb.chnTPL = TPL_NONE;
    }

    /* check for on-demand channel establishment */
    if(!(satShrdPrm.cmdDet.cmdQlf & QLF_OPCH_IMMDT_LINK_EST))
    {
      /* CASE: ON DEMAND */
      /* set open channel status */
      satShrdPrm.opchStat = OPCH_ON_DMND;      
    }
    else /* immediately channel establishment */ 
    {
      /* CASE: IMMEDIATELY */
      /* check temporary problems */
      if( cmhSAT_OpChnChkTmpProblem() )
      {
        psaCC_FreeCtbNtry (acptId);
        satShrdPrm.chnTb.chnUsdFlg = FALSE;

        return( FALSE );
      }

      /* set open channel status */
      satShrdPrm.opchStat = OPCH_EST_REQ;

#ifdef DTI
#ifdef CO_UDP_IP
      if( satShrdPrm.chnTb.chnTPL EQ UDP )
      {
        /* enable establishment of UDP data chain */
        sAT_PercentWAP ( (T_ACI_CMD_SRC)psaCC_ctb(acptId)->curSrc , 1 );

        /* setup PPP parameters */
        cmhSAT_OpChnSetPPP( B_CSD );
      }
#endif /* CO_UDP_IP */
#endif /* DTI */
    }
    
    /* Establish BIP channel for immediate and on demand */
    chnInf.bipConn    = SIM_BIP_OPEN_CHANNEL;
    chnInf.dtiConn    = SIM_DTI_UNKNOWN; 
    chnInf.chnId      = CHANNEL_ID_1;
    chnInf.genRes     = RSLT_PERF_SUCCESS;
    chnInf.addRes     = ADD_NO_CAUSE;

    /* SAT_E_PATCH: send SIM_BIP_REQ to open BIP channel */
    psaSIM_SATBIPChn( chnInf, cmhSAT_OpBIPChnOpen);
    
    return (TRUE);      
  }
#endif  /* FAX_AND_DATA AND FF_SAT_E */
  
  /* finally set up call */
  cmhCC_flagCall( acptId, &(pCCCmdPrm->mltyCncFlg));
  cmhCC_NewCall(acptId, (T_ACI_CMD_SRC)srcId, AT_CMD_A);
  return( TRUE );
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : SAT                     |
|                                 ROUTINE : cmhSAT_UserAcptCntxt    |
+-------------------------------------------------------------------+

  PURPOSE : This function performs the user acceptance of the pending
            SAT GPRS context. If conditions occur, which prevent the
            establishment of the context, the function returns FALSE.

*/

#if defined (GPRS) AND defined (FF_SAT_E) AND defined (DTI)
GLOBAL BOOL cmhSAT_UserAcptCntxt( UBYTE srcId )
{
  T_SIM_SAT_CHN chnInf; /* holds channel information */

  TRACE_FUNCTION("cmhSAT_UserAcptCntxt()");

  /* store current command source */
  satShrdPrm.opchAcptSrc = srcId;

  if( satShrdPrm.opchPrm AND 
      ((T_SAT_GPRS_PRM*)satShrdPrm.opchPrm)->v_itl AND
      ((T_SAT_GPRS_PRM*)satShrdPrm.opchPrm)->itl.trans_prot_type EQ UDP )
  {
    satShrdPrm.chnTb.chnTPL = ((T_SAT_GPRS_PRM*)satShrdPrm.opchPrm)->itl.trans_prot_type;
  }
  else
  {
    satShrdPrm.chnTb.chnTPL = TPL_NONE;
  }


  /* check for on-demand channel establishment */
  if(!(satShrdPrm.cmdDet.cmdQlf & QLF_OPCH_IMMDT_LINK_EST))
  {
    /* CASE: ON DEMAND */
    
    /* set open channel status */
    satShrdPrm.opchStat = OPCH_ON_DMND;
  }
  else
  {
    /* CASE: IMMEDIATELY */
    
    /* check temporary problems */
    if( cmhSAT_OpChnChkTmpProblem() )
    {
      /* any reset of cid parameters should be placed here, if necessary */
      return( FALSE );
    }
    
    /* set open channel status */
    satShrdPrm.opchStat = OPCH_EST_REQ;    
  }
/* 
 * The activation of the GPRS context is triggered by cmhSAT_OpBIPChnOpen(),
 * a function processing the SIM_BIP_CNF. In the new design the BIP channel 
 * has to be established before the bearer is started (see also 
 * cmhSAT_UserAcptCall() for CSD case) 
 */ 

  /* Establish BIP channel for immediate and on demand */
  chnInf.bipConn    = SIM_BIP_OPEN_CHANNEL;
  chnInf.dtiConn    = SIM_DTI_UNKNOWN; 
  chnInf.chnId      = CHANNEL_ID_1;
  chnInf.genRes     = RSLT_PERF_SUCCESS;
  chnInf.addRes     = ADD_NO_CAUSE;

  /* SAT_E_PATCH: send SIM_BIP_REQ to open BIP channel */
  psaSIM_SATBIPChn( chnInf, cmhSAT_OpBIPChnOpen);

  return ( TRUE );
}
#endif  /* GPRS AND FF_SAT_E */
/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : SAT                     |
|                                 ROUTINE : cmhSAT_ChckRedial       |
+-------------------------------------------------------------------+

  PURPOSE : This function checks the redialling conditions and setup
            the appropriate parameters.

*/

GLOBAL LONG cmhSAT_ChckRedial ( SHORT cId, UBYTE v_dur, T_dur * dur )
{
  TRACE_FUNCTION("cmhSAT_ChckRedial()");

  satShrdPrm.dur = -1;

/*
 *-------------------------------------------------------------------
 *  check command qualifier for redialling
 *-------------------------------------------------------------------
 */
#ifdef FF_SAT_E  
  /* in case of SETUP CALL command */
  if( satShrdPrm.cmdDet.cmdType EQ SAT_CMD_SETUP_CALL )
#endif /* FF_SAT_E */
  {
    switch( satShrdPrm.cmdDet.cmdQlf )
    {
      case( QLF_CALL_IDLE_RDL ):
      case( QLF_CALL_HOLD_RDL ):
      case( QLF_CALL_DISC_RDL ):

        break;

      default:

        return( ACI_NumParmNotPresent );
    }
  }

#ifdef FF_SAT_E
  /* in case of OPEN CHANNEL command */
  if( satShrdPrm.cmdDet.cmdType EQ SAT_CMD_OPEN_CHANNEL )
  {
    if(!(satShrdPrm.cmdDet.cmdQlf & QLF_OPCH_AUTO_RECONNECT))

      return( ACI_NumParmNotPresent );
  }
#endif /* FF_SAT_E */

  /* calculate redial timeout */
  psaCC_ctb(cId)->SATinv |= SAT_REDIAL;

  if( v_dur )
  {
    switch( dur->time_unit )
    {
      /* scale to ms */
      case( TU_MIN ):
        satShrdPrm.dur = dur->time_ivl*60000;
        break;
      case( TU_SEC ):
        satShrdPrm.dur = dur->time_ivl*1000;
        break;
      case( TU_100_MSEC ):
        satShrdPrm.dur = dur->time_ivl*100;
        break;
      default:
        satShrdPrm.dur = -1;
    }
  }


  return((satShrdPrm.dur EQ -1)?ACI_NumParmNotPresent:satShrdPrm.dur);
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : SAT                     |
|                                 ROUTINE : cmhSAT_StartPendingCall |
+-------------------------------------------------------------------+

  PURPOSE : This function performs the actual start of the pending
            SAT call.
*/

GLOBAL BOOL cmhSAT_StartPendingCall ( void )
{
  T_CC_CMD_PRM * pCCCmdPrm; /* points to CC command parameters */
  SHORT satId;                /* id of sat pending call */
#if defined SMI OR defined MFW OR defined FF_MMI_RIV
  T_ACI_CLOG     cmdLog;      /* holds logging info */
#endif

  TRACE_FUNCTION("cmhSAT_StartPendingCall()");

  satId = psaCC_ctbFindCall( OWN_SRC_INV, CS_SAT_REQ, NO_VLD_CT );

  if (satId EQ NO_ENTRY)
    return FALSE;

  pCCCmdPrm = &cmhPrm[psaCC_ctb(satId)->curSrc].ccCmdPrm;
  TRACE_EVENT_P1("psaCC_ctb(cId)->curCmd: %d", psaCC_ctb(satId)->curCmd);

  cmhCC_flagCall( satId, &(pCCCmdPrm -> mltyCncFlg));
  psaCC_NewCall(satId);

#if defined SMI OR defined MFW OR defined FF_MMI_RIV
  cmdLog.atCmd                = AT_CMD_A;
  cmdLog.cmdType              = CLOG_TYPE_Set;
  cmdLog.retCode              = AT_EXCT;
  cmdLog.sId                  = ACI_NumParmNotPresent;
  cmdLog.cmdPrm.sA.srcId      = (T_ACI_CMD_SRC)psaCC_ctb(satId)->curSrc;
  cmdLog.cId                  = satId+1;
  rAT_PercentCLOG( &cmdLog );
#endif

  return TRUE;
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : SAT                     |
|                                 ROUTINE : cmhSAT_ChckCmdDet       |
+-------------------------------------------------------------------+

  PURPOSE : This function checks the command details against the
            current ME status. If the mobile status does not interfere
            with the command details, the function returns TRUE.

*/

GLOBAL BOOL cmhSAT_ChckCmdDet ( void )
{
  SHORT ctbIdx;             /* holds call table index */
  SHORT actId = NO_ENTRY;   /* identifier for active call */
  SHORT hldId = NO_ENTRY;   /* identifier for held call */
  SHORT cId   = satShrdPrm.SIMCCParm.cId; /* holds setup call id */
  T_ACI_SAT_TERM_RESP resp_data;

  TRACE_FUNCTION("cmhSAT_ChckCmdDet()");

  if (!psaCC_ctbIsValid (cId))
  {
    TRACE_ERROR ("Call table entry disappeared");
    return FALSE;
  }

  /*
   * From here it is guaranteed that cId describes an existing (non-NULL)
   * call table entry.
   */

  psaSAT_InitTrmResp( &resp_data );
/*
 *-------------------------------------------------------------------
 *  scan call table
 *-------------------------------------------------------------------
 */
  for( ctbIdx = 0; ctbIdx < MAX_CALL_NR; ctbIdx++ )
  {
    if (ccShrdPrm.ctb[ctbIdx] NEQ NULL)
    {
      switch( psaCC_ctb(ctbIdx)->calStat )
      {
        case( CS_ACT ):
        case( CS_ACT_REQ ):
        case( CS_MDF_REQ ):

          actId = ctbIdx;
          break;

        case( CS_HLD ):
        case( CS_HLD_REQ ):

          hldId = ctbIdx;
          break;
      }
    }
  }

/*
 *-------------------------------------------------------------------
 *  check command qualifier against ME status
 *-------------------------------------------------------------------
 */
  switch( satShrdPrm.cmdDet.cmdQlf )
  {
    case( QLF_CALL_IDLE ):
    case( QLF_CALL_IDLE_RDL ):

      if( actId NEQ NO_ENTRY OR hldId NEQ NO_ENTRY )
      {
        resp_data.add_content = ADD_ME_CALL_BUSY;
        psaSAT_SendTrmResp( RSLT_ME_UNAB_PROC, &resp_data );
        /* clear call ID */
        psaCC_retMOCTi(psaCC_ctb(cId)->ti);
        psaCC_chngCalTypCnt(cId, -1);
        psaCC_FreeCtbNtry (cId);

        TRACE_EVENT("cmhSAT_ChckCmdDet(): CALL_BUSY, RSLT_ME_UNAB_PROC");
        return( FALSE );
      }
      break;

    case( QLF_CALL_HOLD ):
    case( QLF_CALL_HOLD_RDL ):

      if( ( hldId NEQ NO_ENTRY AND actId NEQ NO_ENTRY ) OR
          ( actId NEQ NO_ENTRY AND
            psaCC_ctb(actId)->prio EQ MNCC_PRIO_NORM_CALL AND
            cmhCC_getcalltype(actId) NEQ VOICE_CALL ) )
      {
        psaSAT_SendTrmResp( RSLT_ME_CAP, &resp_data );
        TRACE_EVENT("cmhSAT_ChckCmdDet(): RSLT_ME_CAP");
        return( FALSE );
      }
      break;

    case( QLF_CALL_DISC ):
    case( QLF_CALL_DISC_RDL ):

      break;
  }

  return( TRUE );
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : SAT                     |
|                                 ROUTINE : cmhSAT_fillSetupBC      |
+-------------------------------------------------------------------+

  PURPOSE : This function fills the call table bentry with the
            necessary bearer service setup parameters.

*/

GLOBAL void cmhSAT_fillSetupBC ( SHORT  cId,
                                 UBYTE  bearer_serv_1,
                                 UBYTE  bearer_serv_2  )
{
  T_CC_CALL_TBL *ctb = ccShrdPrm.ctb[cId];

  TRACE_FUNCTION ("cmhSAT_fillSetupBC()");

  /*
   *-----------------------------------------------------------------
   * bearer service
   *-----------------------------------------------------------------
   */
  if ( bearer_serv_1 EQ MNCC_BEARER_SERV_NOT_PRES )
  {
    ctb->BC[0].rate        = MNCC_UR_NOT_PRES;
    ctb->BC[0].bearer_serv = MNCC_BEARER_SERV_SPEECH;
    ctb->BC[0].conn_elem   = MNCC_CONN_ELEM_NOT_PRES;
  }
  else
  {
    ctb->BC[0].rate        = MNCC_UR_NOT_PRES;
    ctb->BC[0].bearer_serv = (bearer_serv_1 EQ MNCC_BEARER_SERV_SPEECH) ?
      cmhCC_set_speech_serv (&cmhPrm[CMD_SRC_LCL].ccCmdPrm) : bearer_serv_1;
    ctb->BC[0].conn_elem   = MNCC_CONN_ELEM_NOT_PRES;
  }
  if ( bearer_serv_2 EQ MNCC_BEARER_SERV_NOT_PRES )
  {
    ctb->BC[1].rate        = MNCC_UR_NOT_PRES;
    ctb->BC[1].bearer_serv = MNCC_BEARER_SERV_NOT_PRES;
    ctb->BC[1].conn_elem   = MNCC_CONN_ELEM_NOT_PRES;
  }
  else
  {
    ctb->BC[1].rate        = MNCC_UR_NOT_PRES;
    ctb->BC[1].bearer_serv = (bearer_serv_2 EQ MNCC_BEARER_SERV_SPEECH) ?
      cmhCC_set_speech_serv (&cmhPrm[CMD_SRC_LCL].ccCmdPrm) : bearer_serv_2;
    ctb->BC[1].conn_elem   = MNCC_CONN_ELEM_NOT_PRES;
  }
}


/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : SAT                     |
|                                 ROUTINE : cmhSAT_convertBCDNum    |
+-------------------------------------------------------------------+

  PURPOSE : This function converts the BCD number to an ascii char
            string

*/
LOCAL void cmhSAT_convertBCDNum(U8 *bcdNum, U16 len, CHAR *bcdChar)
{
  int i;

  TRACE_FUNCTION("cmhSAT_convertBCDNum");

  memset(bcdChar, 0, MAX_PARTY_NUM_SAT);

  for ( i=0; i < len; i++)
  {
    bcdChar[i] = bcdNum[i] + '0';
  }
  
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : SAT                     |
|                                 ROUTINE : cmhSAT_fillSetupPrm     |
+-------------------------------------------------------------------+

  PURPOSE : This function fills the call table entry with the
            necessary SAT setup parameters.

*/
//TISH, patch for call control CLIR
//start
extern UBYTE getCLIRState(void);
//end
GLOBAL void cmhSAT_fillSetupPrm ( SHORT     cId,
                                  T_addr    *adr,
                                  T_subaddr *sub  )
{
  T_CC_CALL_TBL *ctb = ccShrdPrm.ctb[cId];
  UBYTE  i;
  USHORT dummy=0, length;
  CHAR   *buf_4_dtmf;
  BOOL   end_of_dial_string          = FALSE;
  CHAR   *number_buffer;
//TISH, patch for call control CLIR
//start
  T_CC_CMD_PRM  *pCCCmdPrm;  /* points to CC command parameters */
  pCCCmdPrm  = &cmhPrm[cId].ccCmdPrm;
//end
  TRACE_FUNCTION ("cmhSAT_fillSetupPrm()");

  ACI_MALLOC(number_buffer, MAX_PARTY_NUM_SAT);
  ACI_MALLOC(buf_4_dtmf, MNCC_MAX_CC_CALLED_NUMBER+1);

  /* called address */
  if( adr NEQ NULL )
  {
    /* check for emergency call code 112 */
    if( adr -> c_bcdDigit > 0 AND
        adr -> bcdDigit[adr -> c_bcdDigit-1] EQ 0x0f )
    {
      adr -> c_bcdDigit -= 1;
    }

    cmhSAT_convertBCDNum(adr->bcdDigit, adr->c_bcdDigit, number_buffer);
    
//TISH, patch for OMAPS00141208&OMAPS00141392
//According to JRDC's request, 911 should be treated as emergency call too.
//start
#if 0
    /* Only use 112 as an Emmergency Call on STK */
    /* Implements Measure#32: Row 1027 */
    if (!strcmp(number_buffer, num_112_str))
#else
    if (!strcmp(number_buffer,"112") OR !strcmp(number_buffer,"911"))
#endif
//end
    {
      ctb->prio = MNCC_PRIO_EMERG_CALL;
      TRACE_EVENT("EMERGENCY CALL");
    }
    else
      ctb->prio = MNCC_PRIO_NORM_CALL;

    /* Check for DTMF tones within dial string */
    length = MINIMUM(adr->c_bcdDigit, MNCC_MAX_CC_CALLED_NUMBER);
    for(i=0; i<length; i++)
    {
      /* convert address: PHB compliant */
      buf_4_dtmf[i] = cmhPHB_convertBCD2char( adr->bcdDigit[i] );
      if(buf_4_dtmf[i] EQ PHB_DTMF_SEP)
      {
        /* DTMF separator */
        if(!end_of_dial_string)
        {
          adr-> c_bcdDigit = i;    /* split number from DTMF tones */
          end_of_dial_string = TRUE;
        }
      }
    }

    buf_4_dtmf[length] = '\0';
    cmhCC_chkDTMFDig ( buf_4_dtmf, cId, dummy, TRUE );

    /* set address */
    ctb->cldPty.ton = adr -> noa;
    ctb->cldPty.npi = adr -> npi;

    ctb->cldPty.c_called_num = MINIMUM( MNCC_MAX_CC_CALLED_NUMBER, adr -> c_bcdDigit );
    if (ctb->cldPty.called_num NEQ NULL)
    {
      ACI_MFREE (ctb->cldPty.called_num);
      ctb->cldPty.called_num = NULL;
    }
    if (ctb->cldPty.c_called_num NEQ 0)
    {
      ACI_MALLOC (ctb->cldPty.called_num, ctb->cldPty.c_called_num);
      memcpy( ctb->cldPty.called_num, 
              adr -> bcdDigit,
              ctb->cldPty.c_called_num );
    }

    psaCC_phbSrchNum( cId, CT_MOC );    /* get alpha identifier */

  }

/*
 *-----------------------------------------------------------------
 * called subaddress
 *-----------------------------------------------------------------
 */
  if( sub NEQ NULL )
  {
    if( sub -> c_subadr_str > 0 AND
        sub -> subadr_str[sub -> c_subadr_str-1] EQ 0x0f )

      sub -> c_subadr_str -= 1;

    ctb->cldPtySub.c_subaddr =
      MINIMUM( MNCC_SUB_LENGTH, sub -> c_subadr_str );

    memcpy( ctb->cldPtySub.subaddr, sub -> subadr_str,
            ctb->cldPtySub.c_subaddr );

    ctb->cldPtySub.tos = sub -> tos;

    ctb->cldPtySub.odd_even = sub -> oei;
  }
  else            /* subaddress not found */
  {
    ctb->cldPtySub.tos       = MNCC_TOS_NOT_PRES;
    ctb->cldPtySub.odd_even  = MNCC_OE_EVEN;
    ctb->cldPtySub.c_subaddr = 0;
  }

  /*
   *-----------------------------------------------------------------
   * clir
   *-----------------------------------------------------------------
   */
//TISH, patch for call control CLIR
//start
// FreeCalypso: reverted
#if 1
  ctb->CLIRsup = NOT_PRESENT_8BIT;
#else
  switch( getCLIRState())
  {
    case( D_CLIR_OVRD_Supp ):
      ctb->CLIRsup = 0 ; //  CLR_SUP
      break;

    case( D_CLIR_OVRD_Invoc ):
      ctb->CLIRsup = 0xff; //CLR_SUP_NOT;
      break;

    case( D_CLIR_OVRD_Default ):

      switch( pCCCmdPrm -> CLIRmode )
      {
        case( CLIR_MOD_Subscript ):
          ctb->CLIRsup = 1;//CLR_NOT_PRES;
          break;
        case( CLIR_MOD_Invoc ):
          ctb->CLIRsup = 0xff;//CLR_SUP_NOT;
          break;
        case( CLIR_MOD_Supp ):
          ctb->CLIRsup = 0;//CLR_SUP;
          break;
      }
      break;
    default:
      ctb->CLIRsup = NOT_PRESENT_8BIT;
  }  
#endif
//end
  ctb->rptInd  = MNCC_RI_NOT_PRES;

  ACI_MFREE(number_buffer);
  ACI_MFREE(buf_4_dtmf);
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : SAT                     |
|                                 ROUTINE : cmhSAT_chkDTMF          |
+-------------------------------------------------------------------+

  PURPOSE : This function extracs DTMF digit from the dial string and
            store them in a buffer.
*/
#if 0
GLOBAL void cmhSAT_chkDTMF ( SHORT cId, T_addr* adr )
{
  UBYTE  cnt  = 0;
  UBYTE  dtmf = 0;
  USHORT len  = adr->c_bcdDigit;

  /* reset dtmf parameters */
  satDtmfBuf.cId = -1;
  satDtmfBuf.cnt = 0;
  satDtmfBuf.cur = 0;

  /* search first occurrence of dtmf digit */
  while( adr->bcdDigit[cnt] NEQ 0x0C AND cnt < len ) cnt++;

  /* if none return */
  if( cnt EQ adr->c_bcdDigit ) return;

  /* adjust num digit count */
  adr->c_bcdDigit = cnt;

  /* convert to IA5 */
  while( cnt < len AND dtmf < MAX_DTMF_DIG )
  {
    satDtmfBuf.dig[dtmf] = cmhPHB_convertBCD2char( adr->bcdDigit[cnt] );

    cnt++;
    if( satDtmfBuf.dig[dtmf] NEQ '\0' )
    {
      dtmf++; /* else erase it: that was an invalid parameter */
    }
  }

  /* update dtmf parameter */
  satDtmfBuf.cnt = dtmf+1;
  satDtmfBuf.cur = 0;
  satDtmfBuf.cId = cId;
}
#endif /* 0 */
/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : SAT                     |
|                                 ROUTINE : cmhSAT_ResCapCode       |
+-------------------------------------------------------------------+

  PURPOSE : This function proceeds after the reception of a capability
            configuration parameter coding.
*/

GLOBAL void cmhSAT_ResCapCode ( USHORT cause, T_MNCC_bcconf* bc1, T_MNCC_bcconf2* bc2)
{
  SHORT cId;              /* holds call id */
  UBYTE cmdBuf;           /* buffers command */
  UBYTE srcBuf;           /* buffers command source */

  cId = satShrdPrm.capParm.cId;

  if (!psaCC_ctbIsValid (cId))
  {
    TRACE_ERROR ("Call table entry disappeared");
    return;
  }

  switch( cause )
  {
    case( MNCC_CAUSE_SUCCESS ):

    /*
     *-------------------------------------------------------------------
     *  build envelope call control command
     *-------------------------------------------------------------------
     */
      CCD_START;
      psaSAT_BuildEnvCC ( cId, NULL, NULL, bc1, bc2);

      satShrdPrm.SIMCCParm.cId   = cId;
      satShrdPrm.SIMCCParm.ccAct = CC_ACT_CAL;
      satShrdPrm.SIMCCParm.owner = psaCC_ctb(cId)->calOwn;
      satShrdPrm.SIMCCParm.busy  = TRUE;

      satShrdPrm.owner = OWN_SRC_INV;

      satShrdPrm.Cbch_EvtDnl = FALSE;

      psaSAT_STKEnvelope (NULL);

      CCD_END;
      break;

    case( MNCC_CAUSE_MS_INCOMPAT_DEST ):
    default:

    /*
     *-------------------------------------------------------------------
     *  inform MMI about unsuccessful call setup
     *-------------------------------------------------------------------
     */
     /* at the moment only user caps need to be coded */
      cmhCC_PrepareCmdEnd (cId, &cmdBuf, &srcBuf);
      psaCC_FreeCtbNtry (cId);

      R_AT( RAT_CME, (T_ACI_CMD_SRC)srcBuf ) ( cmdBuf, CME_ERR_NotPresent );

      /* log result */
      cmh_logRslt ( (T_ACI_CMD_SRC)srcBuf, RAT_CME,(T_ACI_AT_CMD)cmdBuf,
                    -1, BS_SPEED_NotPresent, CME_ERR_NotPresent );
      break;
  }
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : SAT                     |
|                                 ROUTINE : cmhSAT_ResCapDecode     |
+-------------------------------------------------------------------+

  PURPOSE : This function proceeds after the reception of a capability
            configuration parameter decoding.
*/

GLOBAL void cmhSAT_ResCapDecode ( USHORT cause, T_MNCC_bcpara* bc1, T_MNCC_bcpara2* bc2 )
{
  T_CC_CALL_TBL *ctb;     /* Pointer to call table */
  SHORT cId;              /* holds call id */
  UBYTE cmdBuf;           /* buffers command */
  UBYTE srcBuf;           /* buffers command */
  UBYTE idx;              /* holds index */
  T_ACI_SAT_TERM_RESP resp_data;
#ifdef FF_SAT_E  
  T_ACI_SATA_ADD addPrm;
#endif /* FF_SAT_E */

  cId = satShrdPrm.capParm.cId;

  if (!psaCC_ctbIsValid (cId))
  {
    TRACE_ERROR ("Call table entry disappeared");
    return;
  }

  ctb = ccShrdPrm.ctb[cId];

  psaSAT_InitTrmResp( &resp_data );


  switch( cause )
  {
    case( MNCC_CAUSE_SUCCESS ):

      if( !satShrdPrm.ownSAT )
      {
        if( ccShrdPrm.BC0_send_flag )
        {
          ctb[cId].BC[0] = *bc1;
        }

        if( ccShrdPrm.BC1_send_flag )
        {
          ctb[cId].BC[1] = *bc2;
        }
      } 
      else
      {
        ctb[cId].BC[0] = *bc1;
        ctb[cId].BC[1] = *bc2;
      }


      /* check capability context */
      switch( satShrdPrm.capParm.cntxt )
      {
        case( CTX_SAT_SETUP ):
          /* alert user if command details are supported */
          if( cmhSAT_ChckCmdDet() )
          {
            for( idx = 0; idx < CMD_SRC_MAX; idx++ )
            {
#ifdef FF_SAT_E
              addPrm.chnType = SATA_CT_VOICE;
              addPrm.chnEst  = SATA_EST_IM;

              R_AT( RAT_SATA, (T_ACI_CMD_SRC)idx )( cId+1, satShrdPrm.dur, &addPrm );
#else  /* FF_SAT_E */
              R_AT( RAT_SATA, (T_ACI_CMD_SRC)idx )( cId+1, satShrdPrm.dur);
#endif /* FF_SAT_E */ 
            }
          }
          else
          {
            psaCC_FreeCtbNtry (cId);
          }
          break;

        case( CTX_CC_RESULT ):

          cmhSAT_SetupCalAfterCCRes ( !satShrdPrm.ownSAT, cId, satShrdPrm.capParm.CCres );
          break;
      }
      break;

    case( MNCC_CAUSE_MS_INCOMPAT_DEST ):
    default:

    /*
     *-------------------------------------------------------------------
     *  inform about unsuccessful call setup
     *-------------------------------------------------------------------
     */
       /* setup initiated by user */
      if( !satShrdPrm.ownSAT )
      {
        cmhCC_PrepareCmdEnd (cId, &cmdBuf, &srcBuf);
        psaCC_FreeCtbNtry (cId);

        R_AT( RAT_CME, (T_ACI_CMD_SRC)srcBuf ) ( cmdBuf, CME_ERR_NotPresent );

        /* log result */
        cmh_logRslt ( (T_ACI_CMD_SRC)srcBuf, RAT_CME, (T_ACI_AT_CMD)cmdBuf,
                      -1, BS_SPEED_NotPresent, CME_ERR_NotPresent );
      }
       /* setup initiated by SAT */
      else
      {
        /* send SAT response */
        resp_data.resCC = (satShrdPrm.capParm.cntxt EQ CTX_CC_RESULT)?
                        &satShrdPrm.capParm.CCres: NULL;

        psaSAT_SendTrmResp( RSLT_ME_CAP, &resp_data );
      }
      break;
  }
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : SAT                     |
|                                 ROUTINE : cmhSAT_SetupCalAfterCCRes|
+-------------------------------------------------------------------+

  PURPOSE : This function proceeds with call setup after call control
            by SIM result.
*/

GLOBAL BOOL cmhSAT_SetupCalAfterCCRes ( UBYTE ownNotSAT, SHORT cId,
                                        UBYTE CCres)
{
  SHORT actId;          /* holds active call id */
  UBYTE cmdBuf;         /* buffers command */
  UBYTE srcBuf;         /* buffers command source */
  UBYTE idx;            /* holds list index */
  T_ACI_SAT_TERM_RESP resp_data;
#ifdef FF_SAT_E 
  T_ACI_SATA_ADD addPrm;
#endif /* FF_SAT_E */

  psaSAT_InitTrmResp( &resp_data );

 /*
  *------------------------------------------------------------
  * perform call setup initiated by user
  *------------------------------------------------------------
  */
  if( ownNotSAT )
  {
    /* check for an active call */
    actId = psaCC_ctbFindCall( OWN_SRC_INV, CS_ACT, NO_VLD_CT );

    if( actId NEQ NO_ENTRY )
    {
      /* put active on hold if possible */
      if( psaCC_ctb(actId)->prio EQ MNCC_PRIO_NORM_CALL AND
          cmhCC_getcalltype(cId) EQ VOICE_CALL )
      {
        cmhCC_HoldCall(actId, (T_ACI_CMD_SRC)psaCC_ctb(cId)->curSrc, AT_CMD_D);
      }
      /* reject call setup */
      else
      {
        cmhCC_PrepareCmdEnd (cId, &cmdBuf, &srcBuf);;
        psaCC_FreeCtbNtry (cId);

        R_AT( RAT_CME, (T_ACI_CMD_SRC)srcBuf ) ( cmdBuf, CME_ERR_NotPresent );

        /* log result */
        cmh_logRslt ( (T_ACI_CMD_SRC)srcBuf, RAT_CME, (T_ACI_AT_CMD)cmdBuf,
                      -1,BS_SPEED_NotPresent, CME_ERR_NotPresent );
        return( FALSE );
      }
    }

    /* finally set up call */
    cmhCC_flagCall( cId,
                    &(cmhPrm[psaCC_ctb(cId)->calOwn].ccCmdPrm.mltyCncFlg));

    psaCC_NewCall(cId);

    return( FALSE );
  }

 /*
  *------------------------------------------------------------
  * perform call setup initiated by SAT
  *------------------------------------------------------------
  */
  else
  {
   /* alert user if command details are supported */
    if( cmhSAT_ChckCmdDet() )
    {
     /* check aoc condition */
      if ((psaCC_ctb(cId)->prio EQ MNCC_PRIO_NORM_CALL) AND
          (aoc_check_moc() EQ FALSE))
        /*
         * check ACM exceeds ACMmax
         * for non-emergency calls
         */
      {
        resp_data.add_content = ADD_NO_CAUSE;
        resp_data.resCC = (CCres EQ CCR_ALLW_WITH_MDFY)? &CCres: NULL;

        psaSAT_SendTrmResp( RSLT_ME_UNAB_PROC, &resp_data);
        psaCC_FreeCtbNtry (cId);
        return( FALSE );
      }

      for( idx = 0; idx < CMD_SRC_MAX; idx++ )
      {
#ifdef FF_SAT_E
        addPrm.chnType = SATA_CT_VOICE;
        addPrm.chnEst  = SATA_EST_IM;

        R_AT( RAT_SATA, (T_ACI_CMD_SRC)idx )( cId+1, satShrdPrm.dur, &addPrm );
#else  /* FF_SAT_E */ 
        R_AT( RAT_SATA, (T_ACI_CMD_SRC)idx )( cId+1, satShrdPrm.dur );
#endif /* FF_SAT_E */
      }

      satShrdPrm.ntfy = USR_NTF_SETUP_CAL;

      return( TRUE );
    }
    return( FALSE);
  }
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : SAT                     |
|                                 ROUTINE : cmhSAT_sendSM           |
+-------------------------------------------------------------------+

  PURPOSE : This function sends a Short Message requested by SAT.
            If the request is going to be processed, the function
            returns TRUE.

*/

GLOBAL BOOL cmhSAT_sendSM ( T_SEND_SM * sm )
{
  T_ACI_RETURN retVal;        /* holds return value */
  UBYTE        fo;
  USHORT       oct_nr;

  USHORT sca_addr_len;        /* length of service center address */
  T_SMS_SET_PRM * pSMSSetPrm; /* points to SMS parameter set */
  T_ACI_SM_DATA  pdu;

  UBYTE packed_data[MAX_SM_LEN];
  UBYTE header_len;
  UBYTE dcs, *p_dcs;
  UBYTE udl, *p_udl;
  UBYTE oct_udl;

  UBYTE        *sm_cont;      /* pointer to the current position in the  pdu */
  T_ACI_SAT_TERM_RESP resp_data;

  TRACE_FUNCTION("cmhSAT_sendSM()");

  psaSAT_InitTrmResp( &resp_data );
//  here , pinghua add this code for simtookit function : OMAPS00109462
    if(mmShrdPrm.regStat NEQ RS_FULL_SRV)
    {
       resp_data.add_content = ADD_ME_NO_SERV;
	psaSAT_SendTrmResp( RSLT_ME_UNAB_PROC, &resp_data );
	return FALSE ;
    }

// add end 
  pdu.len = 0;

  pSMSSetPrm = smsShrdPrm.pSetPrm[OWN_SRC_SAT];
 
/*
 *-------------------------------------------------------------------
 *  check presence of SMS TPDU
 *-------------------------------------------------------------------
 */
  if (!sm->v_smpdu)
  {
    /* respond with "error, required values are missing" */
    psaSAT_SendTrmResp( RSLT_ERR_REQ_VAL, &resp_data );
    return( FALSE );
  }
/*
 *-------------------------------------------------------------------
 *  get SCA
 *-------------------------------------------------------------------
 */
  if (sm->v_addr AND sm->addr.c_bcdDigit > 0)
  {
    if (sm->addr.c_bcdDigit >= MAX_SMS_NUM_LEN)
    {
      TRACE_EVENT("cmhSAT_sendSM: ERROR SMSC address  to long");
      /* SCA number too long */
      psaSAT_SendTrmResp( RSLT_ERR_REQ_VAL, &resp_data );
      return( FALSE );
    }

    /* copy the sca into the pdu */
    sca_addr_len = CodeRPAddress(pdu.data,
                              (UBYTE)sm->addr.c_bcdDigit,
                              sm->addr.noa,
                              sm->addr.npi,
                              sm->addr.bcdDigit);

  }
  else
  {
    /* ACI-SPR-16431: changes */
    if (  psaSIM_ChkSIMSrvSup( SRV_SMS_Parms ) )  /* verify that SMSP is provided */ 
    {
      if ( pSMSSetPrm -> sca.c_num > 0  )  /* verify that SMSP contains address */
      {
        /* ready to copy the sca into the pdu */
        sca_addr_len = CodeRPAddress(pdu.data,
                                  pSMSSetPrm -> sca.c_num,
                                  pSMSSetPrm -> sca.ton,
                                  pSMSSetPrm -> sca.npi,
                                  pSMSSetPrm -> sca.num);
      }
      else
      {
        /* ERROR: SMSP address EMPTY */
        TRACE_EVENT("cmhSAT_sendSM: ERROR: SMSP address EMPTY");  
        psaSAT_SendTrmResp( RSLT_ME_UNAB_PROC, &resp_data ); /* TR(20) */
        return( FALSE );
      }
    }
    else
    {
      /* ERROR: SMSP NOT available */
      TRACE_EVENT("cmhSAT_sendSM: ERROR: SMSP NOT available");
      psaSAT_SendTrmResp( RSLT_ME_UNAB_PROC, &resp_data ); /* TR(20) */
      return( FALSE );
    }
  }

/*
 *-------------------------------------------------------------------
 *  check and evaluate SMS TPDU
 *-------------------------------------------------------------------
 */
  if (sm->smpdu.c_tpdu_data > 0)
  {
    fo = sm->smpdu.tpdu_data[0];   /* first octet with Message Type */
    switch (fo & TP_MTI_MASK)
    {
    case TP_MTI_SMS_SUBMIT:
      if (sm->smpdu.c_tpdu_data < 7)
      {
        /* not the minimum length for TP-SUBMIT */
        psaSAT_SendTrmResp( RSLT_ERR_REQ_VAL, &resp_data );
        return( FALSE );
      }
      if (sm->smpdu.tpdu_data[2] >= MAX_SMS_NUM_LEN)
      {
        /* destination number too long */
        psaSAT_SendTrmResp( RSLT_ERR_REQ_VAL, &resp_data );
        return( FALSE );
      }

      /* oct_nr is the number of bytes in address value */
      oct_nr = (USHORT)((sm->smpdu.tpdu_data[2] + 1) / 2);
      if ((oct_nr + 7) > (USHORT)sm->smpdu.c_tpdu_data)
      {
        /* number length inconsistent with overall length for TP-SUBMIT */
        psaSAT_SendTrmResp( RSLT_ERR_REQ_VAL, &resp_data );
        return( FALSE );
      }

      /* sm_cont points now to TP-DCS */
      sm_cont = &sm->smpdu.tpdu_data[4 + oct_nr + 1];
      p_dcs = &pdu.data[sca_addr_len+4 + oct_nr + 1];

      /* sm_cont points now to TP-VP */
      dcs = *sm_cont++;


      switch (fo & TP_VPF_MASK)
      {
      case TP_VPF_NOT_PRESENT:
      default:
        /* oct_nr is length of TP-UD */
        oct_nr = (USHORT)sm->smpdu.c_tpdu_data - oct_nr - 7;
        break;

      case TP_VPF_RELATIVE:
        if ((oct_nr + 8) > (USHORT)sm->smpdu.c_tpdu_data)
        {
          /* not the required length for TP-SUBMIT with VP-RELATIVE */
          psaSAT_SendTrmResp( RSLT_ERR_REQ_VAL, &resp_data );
          return( FALSE );
        }
        /* sm_cont points now to TP-UDL */
        sm_cont++;

        /* oct_nr is length of TP-UD */
        oct_nr = (USHORT)sm->smpdu.c_tpdu_data - oct_nr - 8;
        break;

      case TP_VPF_ABSOLUTE:
      case TP_VPF_ENHANCED:     /* treat as VP-ABSOLUTE */
        if ((oct_nr + 14) > (USHORT)sm->smpdu.c_tpdu_data)
        {
          /* not the required length for TP-SUBMIT with VP-ABSOLUTE */
          psaSAT_SendTrmResp( RSLT_ERR_REQ_VAL, &resp_data );
          return( FALSE );
        }

        /* sm_cont points now to TP-UDL */
        sm_cont += 7;

        /* oct_nr is length of TP-UD */
        oct_nr = (USHORT)sm->smpdu.c_tpdu_data - oct_nr - 14;
        break;
      }


      header_len = sm->smpdu.c_tpdu_data - oct_nr;
      p_udl = &pdu.data[sca_addr_len + header_len - 1];


      switch (cmhSMS_getAlphabetPp (dcs))
      {
      default:      /* 7 bit default aphabet */
        if (*sm_cont > 160)
        {
          /* SM length inconsistent */
          psaSAT_SendTrmResp( RSLT_ERR_REQ_VAL, &resp_data );
          return( FALSE );
        }
#ifdef _PACK_DEF_ALPHA
        /* packing required */
        if (satShrdPrm.cmdDet.cmdQlf EQ QLF_SMS_PACK AND
          (USHORT)*sm_cont EQ oct_nr)   /* check if packing is valid at all
                                          (so *sm_cont must be equal of remaining bytes)
                                          otherwise do not repack the data since it seems that
                                          data has already been packed.
                                          This is CQ18268/MOBil57469 */
        {
          /* length in septets, sm_cont points now to TP-UD */
          udl = *sm_cont++;

          /* oct_udl is the length of packed data in octets */
          oct_udl = utl_cvt8To7 (sm_cont, udl, packed_data, 0);

          /* copy the header into the pdu */
          memcpy(&pdu.data[sca_addr_len], sm->smpdu.tpdu_data,
                  header_len);

          /* copy the packed user data into the pdu */
          memcpy (&pdu.data[sca_addr_len+header_len], packed_data, oct_udl);

          pdu.len = sca_addr_len+header_len+oct_udl;

          retVal = sAT_PlusCMGSPdu ((T_ACI_CMD_SRC)OWN_SRC_SAT, &pdu);
          switch (retVal)
          {
          case AT_CMPL:
            psaSAT_SendTrmResp( RSLT_PERF_SUCCESS, &resp_data );
            return TRUE;

          case AT_BUSY:
            psaSAT_SendTrmResp( RSLT_ME_UNAB_PROC, &resp_data );
            return( FALSE );

          case AT_FAIL:
            psaSAT_SendTrmResp( RSLT_ERR_REQ_VAL, &resp_data );
            return( FALSE );
          }
          return TRUE;
        } /* if (satShrdPrm.cmdDet.cmdQlf EQ QLF_SMS_PACK) */
        else
#endif /* _PACK_DEF_ALPHA: ignore PACKING REQUIRED for GSM default
          alphabet, when not set */
        {
          /* packing not required */
          udl = *sm_cont;
          oct_udl = ((*sm_cont + 1)*7)/8;
        }
        break;


      case 1:       /* 8 bit data */
        if (satShrdPrm.cmdDet.cmdQlf EQ QLF_SMS_PACK
            AND ((dcs & 0xE0) NEQ 0x20))
        {       /* packing only, when text is uncompressed */
          if ((USHORT)*sm_cont > oct_nr OR *sm_cont > 160)
          {
            /* SM length inconsistent */
            psaSAT_SendTrmResp( RSLT_ERR_REQ_VAL, &resp_data );
            return( FALSE );
          }
          if ((dcs & 0xC0) EQ 0)
            dcs &= 0xF3;    /* adjust coding group 00xx */
          else if ((dcs & 0xF0) EQ 0xF0)
            dcs &= 0xFB;    /* adjust coding group 1111 */

          /* length in septets, sm_cont points now to TP-UD */
          udl = *sm_cont++;

          /* oct_udl is the length of packed data in octets */
          oct_udl = utl_cvt8To7 (sm_cont, udl, packed_data, 0);

          /* copy the header into the pdu */
          memcpy(&pdu.data[sca_addr_len], sm->smpdu.tpdu_data,
                  header_len);

          /* copy the dcs into the pdu */
          *p_dcs = dcs;

          /* copy the packed user data into the pdu */
          memcpy (&pdu.data[sca_addr_len+header_len], packed_data, oct_udl);

          pdu.len = sca_addr_len+header_len+oct_udl;

          retVal = sAT_PlusCMGSPdu ((T_ACI_CMD_SRC)OWN_SRC_SAT, &pdu);
          switch (retVal)
          {
          case AT_CMPL:
            psaSAT_SendTrmResp( RSLT_PERF_SUCCESS, &resp_data );
            return TRUE;

          case AT_BUSY:
            psaSAT_SendTrmResp( RSLT_ME_UNAB_PROC, &resp_data );
            return( FALSE );

          case AT_FAIL:
            psaSAT_SendTrmResp( RSLT_ERR_REQ_VAL, &resp_data );
            return( FALSE );
          }
          return TRUE;
        }
         /*  sandip CQ - 18269
          *  To take care of a case where Command qualifier == 0x00( packing not required )
          *  DCS == 0x04( 8 -bit packing )
          *  TPDU length >140
          */
         
        /*
        else if((satShrdPrm.cmdDet.cmdQlf EQ QLF_SMS_NO_PACK) && (oct_nr > 140))
        {
                psaSAT_SendTrmResp(RSLT_UNKN_DATA,&resp_data);
                return(FALSE);
        }*/
        else
        {
                if(oct_nr > 140)
                {
                        psaSAT_SendTrmResp(RSLT_UNKN_DATA,&resp_data);
                        return(FALSE);
                }
          /* packing not required */
          udl = *sm_cont;
          oct_udl = *sm_cont;
        }
        break;

      case 2:       /* UCS2 */
        /*
         * Special consideration: are octets (incorrect) or UCS2
         * characters (correct) counted by SAT?
         */
        if ((USHORT)*sm_cont EQ oct_nr)
          oct_udl = *sm_cont++;     /* assume octet count */
        else
          oct_udl= *sm_cont++ * 2;  /* assume character count */
        udl = oct_udl;
        break;
      } /* switch (cmhSMS_getAlphabetPp (dcs)) */


      if ((USHORT)oct_udl > oct_nr OR oct_udl > MAX_SM_LEN)
      {
        /* SM length inconsistent */
        psaSAT_SendTrmResp( RSLT_ERR_REQ_VAL, &resp_data );
        return( FALSE );
      }

      /* copy the tpdu data into the pdu */
      memcpy(&pdu.data[sca_addr_len], sm->smpdu.tpdu_data, sm->smpdu.c_tpdu_data);

      /* copy the udl into the pdu */
      *p_udl = udl;

      pdu.len = sca_addr_len + sm->smpdu.c_tpdu_data;

      retVal = sAT_PlusCMGSPdu ((T_ACI_CMD_SRC)OWN_SRC_SAT, &pdu);
      switch (retVal)
      {
      case AT_CMPL:
        psaSAT_SendTrmResp( RSLT_PERF_SUCCESS, &resp_data );
        return TRUE;

      case AT_BUSY:
        psaSAT_SendTrmResp( RSLT_ME_UNAB_PROC, &resp_data );
        return( FALSE );

      case AT_FAIL:
        psaSAT_SendTrmResp( RSLT_ERR_REQ_VAL, &resp_data );
        return( FALSE );
      } /* end switch (retVal) */
      return TRUE;


    case TP_MTI_SMS_COMMAND:
      if (sm->smpdu.c_tpdu_data < 8)
      {
        /* not the minimum length for TP-COMMAND */
        psaSAT_SendTrmResp( RSLT_ERR_REQ_VAL, &resp_data );
        return( FALSE );
      }
      if (sm->smpdu.tpdu_data[5] >= MAX_SMS_NUM_LEN)
      {
        /* destination number too long */
        psaSAT_SendTrmResp( RSLT_ERR_REQ_VAL, &resp_data );
        return( FALSE );
      }
      oct_nr = (USHORT)((sm->smpdu.tpdu_data[5] + 1) / 2);
      if ((oct_nr + 8) > (USHORT)sm->smpdu.c_tpdu_data)
      {
        /* number length inconsistent with overall length for TP-SUBMIT */
        psaSAT_SendTrmResp( RSLT_ERR_REQ_VAL, &resp_data );
        return( FALSE );
      }

#ifdef TI_PS_FF_AT_P_CMD_CUST
      /*
      ** If the MMI is a Cust1 Application
      */
      if (simShrdPrm.overall_cust_mode EQ (UBYTE)CUST_MODE_BEHAVIOUR_1)
      {
          /*
          ** Having performed some checks,
          ** don't send the primitve, but pass the entire command to the MMI for processing.
          ** NOTE: An SMS Command is not passed to the SIM for MO SMS Control By SIM
          **
          ** Ensure that the SMS parameters are reset, so that the SMS Entity is freed to
          ** process the command later.
          */
        smsShrdPrm.smsEntStat.curCmd = AT_CMD_NONE;
        smsShrdPrm.owner = (T_OWN)CMD_SRC_NONE;
        smsShrdPrm.smsEntStat.entOwn =  CMD_SRC_NONE;

          cmhSAT_Cust1StkCmdInd();
          return TRUE;
      }
#endif /* TI_PS_FF_AT_P_CMD_CUST */
      
      memcpy(&pdu.data[sca_addr_len], sm->smpdu.tpdu_data, sm->smpdu.c_tpdu_data);
      retVal = sAT_PlusCMGCPdu ((T_ACI_CMD_SRC)OWN_SRC_SAT, &pdu);
      switch (retVal)
      {
      case AT_CMPL:
        psaSAT_SendTrmResp( RSLT_PERF_SUCCESS, &resp_data );
        return TRUE;

      case AT_BUSY:
        psaSAT_SendTrmResp( RSLT_ME_UNAB_PROC, &resp_data );
        return( FALSE );

      case AT_FAIL:
        psaSAT_SendTrmResp( RSLT_ERR_REQ_VAL, &resp_data );
        return( FALSE );
      }
      return TRUE;

    default:
      /* invalid message type */
      psaSAT_SendTrmResp( RSLT_UNKN_DATA, &resp_data );
      return( FALSE );
    } /* end switch (fo & TP_MTI_MASK) */
  }
  else  /* if (sm->smpdu.c_tpdu_data > 0) */
  {
    /* respond with "error, required values are missing" */
    psaSAT_SendTrmResp( RSLT_ERR_REQ_VAL, &resp_data );
    return( FALSE );
  }
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH                     |
|                                 ROUTINE : cmhSAT_CheckSetEventResp|
+-------------------------------------------------------------------+

  PURPOSE : check for an answer to a setup event list. function 
            returns TRUE if caller has to forward the checked
            SAT response 
*/

GLOBAL BOOL cmhSAT_CheckSetEventResp ( void )
{
  BOOL list_in_process;
  const UBYTE TermRespSetEventOk [12]  =
  {
   0x81,           /* command details tag            */
   0x03,           /* command details length         */
   0x13,           /* command number                 */
   0x05,           /* command SETUP EVENT LIST       */
   0x00,           /* not used                       */
   0x82,           /* device details tag             */
   0x02,           /* device details length          */
   0x82,           /* source ME                      */
   0x81,           /* destination SIM                */
   0x83,           /* result tag                     */
   0x01,           /* result length                  */
   0x00            /* result OK                      */
  };
  const UBYTE TermRespSetEvent [4]  =
  {
   0x81,           /* command details tag            */
   0x03,           /* command details length         */
   0x13,           /* command number                 */
   0x05,           /* command SETUP EVENT LIST       */
  };
  UBYTE *p;

  T_ACI_SAT_TERM_RESP resp_data;

  TRACE_FUNCTION("cmhSAT_CheckSetEventResp");
  
  psaSAT_InitTrmResp( &resp_data );
  
  list_in_process = ( satShrdPrm.event.temp_list & 0x8000 ) > 0L;

  if ( list_in_process )
  {
    
    p = (satShrdPrm.setPrm[satShrdPrm.owner].stkCmd);

    if ((!memcmp((const UBYTE*) p, TermRespSetEvent,2)) AND (p[3] EQ TermRespSetEvent[3]))  /* TRUE means: answer to a SETUP EVENT Command */
    {
      if (!memcmp((const UBYTE*) p+3, TermRespSetEventOk+3, 9))  /* TRUE means: answer ok */
      {
        cmhSAT_setupEvent_Perform();       /* accept list */
        TRACE_EVENT("New Download Event List accepted ");
        return TRUE; /* caller has to send/forward TR */ 
      }
      else
      {
        /* MMI couldn't perform the list: ACI doesn't perform it and returns
        Beyond Mobile Capabilities */
        psaSAT_SendTrmResp( RSLT_ME_CAP, &resp_data );
        TRACE_EVENT("New Download Event List refused by MMI");
        satShrdPrm.event.temp_list = 0L; /* reinitialize for next setup list */
        return FALSE; /* caller hasn't to send TR, already sent! */
      }
    }
    else
    {
      return TRUE; /* caller has to send/forward TR */ 
    }
  }
  return TRUE; /* caller has to send/forward TR */ 
}

/* SAT CLASS E FUNCTIONS START */
#ifdef FF_SAT_E 

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : SAT                     |
|                                 ROUTINE : cmhSAT_CloseChannel     |
+-------------------------------------------------------------------+

  PURPOSE : This function handles the request by SAT to close a
            desired channel. If the request is going to be processed,
            the function returns TRUE.

*/

GLOBAL BOOL cmhSAT_CloseChannel ( void )
{
  T_SIM_SAT_CHN chnInf;

  TRACE_FUNCTION("cmhSAT_CloseChannel()");
 
/*
 *-------------------------------------------------------------------
 *  notify about SAT command
 *-------------------------------------------------------------------
 */
  cmhSAT_STKUsrNtfy();

  
/*
 *-------------------------------------------------------------------
 *  check channel status
 *-------------------------------------------------------------------
 */
  if( satShrdPrm.chnTb.chnUsdFlg )
  {
    chnInf.bipConn = SIM_BIP_UNKNOWN;
    chnInf.dtiConn = SIM_DTI_UNKNOWN;
    
    if( satShrdPrm.chnTb.lnkStat EQ SIM_LINK_OPEN )
    {
      chnInf.bipConn = SIM_BIP_CLOSE_CHANNEL;
      
    }
    if( satShrdPrm.chnTb.lnkStat EQ SIM_LINK_CNCT )
    {
      chnInf.bipConn = SIM_BIP_CLOSE_CHANNEL; 
      chnInf.dtiConn = SIM_DTI_DISCONNECT;
    }

    satShrdPrm.opchStat = OPCH_CLS_REQ;

    if( chnInf.bipConn )
    {
#ifdef DTI

      chnInf.chnId   = CHANNEL_ID_1;
      chnInf.genRes  = RSLT_PERF_SUCCESS;
      chnInf.addRes  = ADD_NO_CAUSE;

      psaSIM_SATBIPChn( chnInf, NULL /*cmhSAT_OpChnClose*/);
#endif /* DTI */
    }
    else
    {
      cmhSAT_OpChnClose( SIM_BIP_CLOSE_CHANNEL, 1 );
    }
  }

  return TRUE;
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : SAT                     |
|                                 ROUTINE : cmhSAT_SendData         |
+-------------------------------------------------------------------+

  PURPOSE : This function handles the request by SAT to open a
            desired channel immediately. If the request is going to
            be processed, the function returns TRUE.

*/

GLOBAL BOOL cmhSAT_SendData ( void )
{
#ifdef GPRS
  SHORT cid_array[2] = { 0,PDP_CONTEXT_CID_INVALID };
#endif /* GPRS */

  TRACE_FUNCTION("cmhSAT_SendData()");

/*
 *-------------------------------------------------------------------
 *  notify about SAT command
 *-------------------------------------------------------------------
 */
  cmhSAT_STKUsrNtfy();

/*
 *-------------------------------------------------------------------
 *  check if on demand channel activation has to be performed
 *-------------------------------------------------------------------
 */
  if( satShrdPrm.opchStat EQ OPCH_ON_DMND AND 
      satShrdPrm.chnTb.lnkStat EQ SIM_LINK_OPEN AND
      satShrdPrm.cmdDet.cmdQlf & QLF_OPCH_IMMDT_LINK_EST )
  {
      satShrdPrm.opchStat = OPCH_EST_REQ;

    /*
     *-------------------------------------------------------------------
     * for a CSD channel
     *-------------------------------------------------------------------
     */
      if( satShrdPrm.opchType EQ B_CSD )
      {
        if (!psaCC_ctbIsValid (satShrdPrm.chnTb.chnRefId))
        {
          TRACE_ERROR ("Call table entry disappeared");
          satShrdPrm.chnTb.chnUsdFlg = FALSE;

          return ( FALSE );
        }

        /* check temporary problems */
        if( cmhSAT_OpChnChkTmpProblem() )
        {
          /* remove call table entry */
          psaCC_FreeCtbNtry (satShrdPrm.chnTb.chnRefId);
          satShrdPrm.chnTb.chnUsdFlg = FALSE;

          return( FALSE );
        }
#ifdef DTI
#ifdef CO_UDP_IP
        if( satShrdPrm.chnTb.chnTPL EQ UDP )
        {
          /* enable establishment of UDP data chain */
          sAT_PercentWAP ( (T_ACI_CMD_SRC)psaCC_ctb(satShrdPrm.chnTb.chnRefId)->curSrc , 1 );

          /* setup PPP parameters */
          cmhSAT_OpChnSetPPP( satShrdPrm.opchType );
        }
#endif  /* CO_UDP_IP */
#endif /* DTI */

        /* set up call */
        cmhPrm[satShrdPrm.opchAcptSrc].ccCmdPrm.mltyCncFlg = 0;

        cmhCC_flagCall( satShrdPrm.chnTb.chnRefId, &cmhPrm[satShrdPrm.opchAcptSrc].
                                                               ccCmdPrm.mltyCncFlg);
        /* ccShrdPrm.ctb[satShrdPrm.chnTb.chnRefId].curCmd = AT_CMD_A; */
        /* ccShrdPrm.ctb[satShrdPrm.chnTb.chnRefId].curSrc = satShrdPrm.opchAcptSrc; */
        /* psaCC_NewCall ( satShrdPrm.chnTb.chnRefId ); */
        cmhCC_NewCall( satShrdPrm.chnTb.chnRefId, 
                       (T_ACI_CMD_SRC)satShrdPrm.opchAcptSrc, 
                       AT_CMD_A);
        return( TRUE );
      }
    /*
     *-------------------------------------------------------------------
     * for a GPRS channel
     *-------------------------------------------------------------------
     */
#if defined (GPRS) AND defined (DTI)
      else if( satShrdPrm.opchType EQ B_GPRS )
      {
        cid_array[0] = satShrdPrm.chnTb.chnRefId;

        /* check temporary problems */
        if( cmhSAT_OpChnChkTmpProblem() )
        {
          /* any reset of cid parameters should be placed here,
           * if necessary 
           */ 
          return( FALSE );
        }

#ifdef DTI
#ifdef CO_UDP_IP
        /* activate the context with UDP */
        if( satShrdPrm.chnTb.chnTPL EQ UDP )
        {
          /* enable establishment of UDP data chain */
          sAT_PercentWAP ( (T_ACI_CMD_SRC)satShrdPrm.opchAcptSrc , 1 );

          /* setup PPP parameters */
          cmhSAT_OpChnSetPPP( B_GPRS );

          /* activate context with UDP */
          cmhSAT_OpChnUDPActivateGprs();
        }
        else
#endif /* CO_UDP_IP */
#endif /* DTI */

        /* activate context with no transport layer */
        if( sAT_PlusCGACT( (T_ACI_CMD_SRC)satShrdPrm.opchAcptSrc, CGACT_STATE_ACTIVATED, cid_array ) 
            NEQ AT_EXCT )

          return( FALSE );

        satShrdPrm.opchStat = OPCH_EST_REQ;

        return( TRUE );
      }
#endif  /* GPRS */
    /*
     *-------------------------------------------------------------------
     * invalid channel
     *-------------------------------------------------------------------
     */
      else
      { 
        T_ACI_SAT_TERM_RESP resp_data;

        TRACE_ERROR("invalid channel found, on demand est. failed");

        psaSAT_InitTrmResp( &resp_data );
        resp_data.add_content = ADD_NO_CAUSE;

        cmhSAT_OpChnFailed( RSLT_ME_UNAB_PROC, &resp_data );

        cmhSAT_cleanupOpChnPrms();

      }
  }

  return FALSE;
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : SAT                     |
|                                 ROUTINE : cmhSAT_GetChannelStatus |
+-------------------------------------------------------------------+

  PURPOSE : This function handles the request by SAT to return the
            current status of all channels. If the request is going
            to be processed, the function returns TRUE.

*/

GLOBAL BOOL cmhSAT_GetChannelStatus ( void )
{
  T_ACI_SAT_TERM_RESP resp_data;

  psaSAT_InitTrmResp( &resp_data );


  TRACE_FUNCTION("cmhSAT_GetChannelStatus()");

/*
 *-------------------------------------------------------------------
 *  notify about SAT command
 *-------------------------------------------------------------------
 */
  cmhSAT_STKUsrNtfy();

/*
 *-------------------------------------------------------------------
 *  return channel status
 *-------------------------------------------------------------------
 */
  resp_data.chnStat  = TRUE;
    
  psaSAT_SendTrmResp( RSLT_PERF_SUCCESS, &resp_data );

  return FALSE;
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : SAT                     |
|                                 ROUTINE : cmhSAT_OpenChannelReq   |
+-------------------------------------------------------------------+

  PURPOSE : This function handles the request by SAT to open a channel.
            If the request is going to be processed, the function 
            returns TRUE.

*/

GLOBAL BOOL cmhSAT_OpenChannelReq ( T_OPEN_CHANNEL *opchCmd )
{
#ifdef FAX_AND_DATA
  T_ACI_RETURN  retVal;               /* holds return value */
#endif /* FAX_AND_DATA */
  SHORT         cId;                  /* holds call id / context id */
  T_ACI_SAT_TERM_RESP resp_data;      /* holds terminal response parms */

  psaSAT_InitTrmResp( &resp_data );


  TRACE_FUNCTION("cmhSAT_OpenChannelReq()");

/*
 *-------------------------------------------------------------------
 *  check for basic reasons to reject the command
 *-------------------------------------------------------------------
 */
  resp_data.chnStat = TRUE;
  satShrdPrm.opchPrmMdf = FALSE;

  /* Flag the Sat Call */
  satShrdPrm.ownSAT = TRUE;

  /* ME is busy with another open channel command */
  if( satShrdPrm.opchStat NEQ OPCH_IDLE )
  {
    TRACE_EVENT("SAT OPCH FAIL, busy with other command");
    resp_data.add_content = ADD_NO_CAUSE;
    psaSAT_SendTrmResp( RSLT_ME_UNAB_PROC, &resp_data );
    return( FALSE );
  }

 
  /* is there a IPv6 address given ?*/
  if((opchCmd->v_other_addr AND opchCmd->other_addr.v_ipv6_addr) OR
     (opchCmd->v_data_dest_addr AND opchCmd->data_dest_addr.v_ipv6_addr))
  {
    TRACE_EVENT("SAT OPCH FAIL, IPv6 not supported");
    psaSAT_SendTrmResp( RSLT_ME_CAP, &resp_data );
    return( FALSE );
  }

  /* is TCP requested */
  if( opchCmd->v_if_transp_lev AND
      opchCmd->if_transp_lev.trans_prot_type EQ TCP )
  {
    TRACE_EVENT("SAT OPCH FAIL, TCP not supported");
    psaSAT_SendTrmResp( RSLT_ME_CAP, &resp_data );
    return( FALSE );
  }

  /* check UDP restrictions, WAP is a synonym for UDP */
  if( opchCmd->v_if_transp_lev AND
      opchCmd->if_transp_lev.trans_prot_type EQ UDP )
  {
#if !defined (CO_UDP_IP)
    /* is UDP requested, but WAP not set ?*/
    TRACE_EVENT("SAT OPCH FAIL, UDP not supported");
    resp_data.add_content = ADD_BIP_SIME_ITL_NAVAIL;
    psaSAT_SendTrmResp( RSLT_BEARIND_PERR, &resp_data );
    return( FALSE );
#endif  /* WAP */

    /* no destination address given */
    if( !opchCmd->v_data_dest_addr )
    {
      TRACE_EVENT("SAT OPCH FAIL, no destination address given");
      psaSAT_SendTrmResp( RSLT_ERR_REQ_VAL, &resp_data );
      return( FALSE );
    }
  }

  /* requested buffer size is greater than prefered bufffer size 1400 */
  if(opchCmd->buffer_size > SIM_CLASS_E_BUFFER_SIZE)
  { 
	satShrdPrm.opchPrmMdf = TRUE;
	satShrdPrm.buffer_size = SIM_CLASS_E_BUFFER_SIZE; 
  }
  else
  {
    satShrdPrm.buffer_size = opchCmd->buffer_size; 
  }
	
  /**** is CSD channel requested, but not supported ? ****/
#ifndef FAX_AND_DATA
  if( opchCmd->v_addr )
  {
    /* CSD call not supported by ME */
    TRACE_EVENT("SAT OPCH FAIL, CSD not supported");
    psaSAT_SendTrmResp( RSLT_ME_CAP, &resp_data );
    return( FALSE );
  }
#endif /* FAX_AND_DATA */

  /**** is GPRS channel requested, but not supported ? ****/
#ifndef GPRS
  if( !opchCmd->v_addr AND opchCmd->v_bear_desc )
  {
    /* GPRS call not supported by ME */
    TRACE_EVENT("SAT OPCH FAIL, GPRS not supported");
    psaSAT_SendTrmResp( RSLT_ME_CAP, &resp_data );
    return( FALSE );
  }
#endif /* GPRS */


/*
 *-------------------------------------------------------------------
 *  store channel parameters
 *-------------------------------------------------------------------
 */

  /*
   *-------------------------------------------------------------------
   *  for a CSD channel use the call table
   *-------------------------------------------------------------------
   */
#ifdef FAX_AND_DATA

  if( opchCmd->v_addr )
  {
    T_CC_CALL_TBL *ctb;

    /* get call table entry */
    cId = psaCC_ctbNewEntry();

    if( cId EQ NO_ENTRY )
    {
      /* send SAT response */
      TRACE_EVENT("SAT OPCH FAIL, call table full");
      resp_data.add_content = ADD_NO_CAUSE;
      psaSAT_SendTrmResp( RSLT_ME_UNAB_PROC, &resp_data );
      return( FALSE );  /* primitive not needed anymore */
    }

    ctb = ccShrdPrm.ctb[cId];

    /* build setup parameters */
    cmhSAT_fillSetupPrm ( cId,
                          &opchCmd->addr,
                          ((opchCmd->v_subaddr)?&opchCmd->subaddr:NULL));

    /* check aoc condition */
    if ((ctb->prio EQ MNCC_PRIO_NORM_CALL) AND
        (aoc_check_moc() EQ FALSE))
      /*
       * check ACM exceeds ACMmax
       * for non-emergency calls
       */
    {
      TRACE_EVENT("SAT OPCH FAIL, ACM exceeds");
      resp_data.add_content = ADD_NO_CAUSE;
      psaSAT_SendTrmResp( RSLT_ME_UNAB_PROC,  &resp_data );
      psaCC_FreeCtbNtry (cId);
      return( FALSE );
    }

    /* bearer capabilities */
    ctb->rptInd = MNCC_RI_NOT_PRES;

    if( opchCmd->bear_desc.v_csd_bear_prm )
    {
      UBYTE selVal;

      /* check and set speed parameter */
      selVal = cmhCC_SelRate((T_ACI_BS_SPEED)opchCmd->bear_desc.csd_bear_prm.csd_speed);

      if( selVal EQ MNCC_UR_NOT_PRES )
      {
        TRACE_EVENT("SAT OPCH FAIL, user rate not supported");
        psaSAT_SendTrmResp( RSLT_ME_CAP,  &resp_data );
        psaCC_FreeCtbNtry (cId);
        return( FALSE );
      }

      ctb->BC[0].rate = selVal;

      /* check and set name parameter */
      selVal = cmhCC_SelServ((T_ACI_CBST_NAM)opchCmd->bear_desc.csd_bear_prm.csd_name);

      if( selVal EQ MNCC_BEARER_SERV_NOT_PRES )
      {
        TRACE_EVENT("SAT OPCH FAIL, user service not supported");
        psaSAT_SendTrmResp( RSLT_ME_CAP,  &resp_data );
        psaCC_FreeCtbNtry (cId);
        return( FALSE );
      }

      ctb->BC[0].bearer_serv = selVal;
                                                            
      /* check and set ce parameter */
      selVal = cmhCC_SelCE((T_ACI_CBST_CE)opchCmd->bear_desc.csd_bear_prm.csd_ce);

      if( selVal EQ MNCC_CONN_ELEM_NOT_PRES )
      {
        TRACE_EVENT("SAT OPCH FAIL, ce not supported");
        psaSAT_SendTrmResp( RSLT_ME_CAP,  &resp_data );
        psaCC_FreeCtbNtry (cId);
        return( FALSE );
      }

      ctb->BC[0].conn_elem = selVal;
                                                            
      ctb->BC[0].modem_type = cmhCC_SelMT((T_ACI_BS_SPEED)opchCmd->bear_desc.
                                          csd_bear_prm.csd_speed);
    }

    else if ( opchCmd->bear_desc.bear_type EQ BT_DEFAULT )
    {
      /* default settings for CSD channel */
      ctb->BC[0].rate        = MNCC_UR_9_6_KBIT;
      ctb->BC[0].bearer_serv = MNCC_BEARER_SERV_ASYNC;
      ctb->BC[0].conn_elem   = MNCC_CONN_ELEM_NON_TRANS;
      ctb->BC[0].modem_type  = MNCC_MT_V32;

      /* store default settings in opchCmd, will be used for term response later on */
      opchCmd->bear_desc.csd_bear_prm.csd_speed = BS_SPEED_9600_V32;
      opchCmd->bear_desc.csd_bear_prm.csd_name  = CBST_NAM_Asynch;
      opchCmd->bear_desc.csd_bear_prm.csd_ce    = CBST_CE_NonTransparent;
    }

    else /* invalid bearer setting for CSD */
    {
      psaSAT_SendTrmResp( RSLT_ERR_REQ_VAL,  &resp_data );
      psaCC_FreeCtbNtry (cId);
      return( FALSE );
    }

    ctb->BC[0].stop_bits   = MNCC_STOP_1_BIT;
    ctb->BC[0].data_bits   = MNCC_DATA_8_BIT;
    ctb->BC[0].parity      = MNCC_PARITY_NONE;

    ctb->BC[1].rate        = MNCC_UR_NOT_PRES;
    ctb->BC[1].bearer_serv = MNCC_BEARER_SERV_NOT_PRES;
    ctb->BC[1].conn_elem   = MNCC_CONN_ELEM_NOT_PRES;

    /* declare the owner and status of the call */
    ctb->calOwn     = OWN_SRC_SAT;
    ctb->calStat    = CS_SAT_CSD_REQ;
    ctb->curCmd     = AT_CMD_D;
    ctb->SATinv     = TRUE;

    /* fill in channel table */
    satShrdPrm.chnTb.chnUsdFlg = TRUE;
    satShrdPrm.chnTb.chnRefId  = cId;
    satShrdPrm.chnTb.chnType   = B_CSD;
    satShrdPrm.chnTb.lnkStat   = SIM_NO_LINK;

    /* store bearer parameters for later use */
    cmhSAT_storeCSDPrms (opchCmd);
    
    /* check for call control by SIM */
    retVal = cmhSAT_CalCntrlBySIM( cId );

    switch( retVal )
    {
      case( AT_BUSY ):
        /* respond with "Interaction with call control by SIM, temporary" */
        TRACE_EVENT("SAT OPCH FAIL, CC by SIM busy");
        psaSAT_SendTrmResp( RSLT_CC_SIM_TMP, &resp_data );
        cmhSAT_cleanupOpChnPrms();
        psaCC_FreeCtbNtry (cId);
        return( FALSE );

      case( AT_FAIL ):
        /* respond with "Interaction with call control by SIM, permanent" */
        TRACE_EVENT("SAT OPCH FAIL, CC by SIM failure");
        resp_data.add_content = ADD_NO_CAUSE;
        psaSAT_SendTrmResp( RSLT_CC_SIM_PRM, &resp_data );
        cmhSAT_cleanupOpChnPrms();
        psaCC_FreeCtbNtry (cId);
        return( FALSE );

      case( AT_EXCT ):
        /* wait for SIM result */
        satShrdPrm.ntfy = USR_NTF_SETUP_CAL;
        satShrdPrm.opchStat = OPCH_CCSIM_REQ;
        break;
    }


    /* if call control check is performed, return here */
    if( satShrdPrm.opchStat EQ OPCH_CCSIM_REQ ) return( TRUE );
  }
  else
#endif  /* FAX_AND_DATA */

  /*
   *-------------------------------------------------------------------
   *  for a GPRS channel use a context id in GACI
   *-------------------------------------------------------------------
   */
#if defined (GPRS) AND defined (DTI)

  if( opchCmd->v_bear_desc )
  {
    T_PDP_CONTEXT_INTERNAL cntxt;

    /* get context id */
     cId = pdp_context_get_free_cid();//(U8) psaSAT_gprsFindFreeCntxt();

    if( cId EQ NO_ENTRY )
    {
      /* send SAT response */
      TRACE_EVENT("SAT OPCH FAIL, no free context");
      resp_data.add_content = ADD_NO_CAUSE;
      psaSAT_SendTrmResp( RSLT_ME_UNAB_PROC, &resp_data );
      return( FALSE );  /* primitive not needed anymore */
    }

    /* bearer capabilities */
    if( opchCmd->bear_desc.v_gprs_bear_prm )
    {
      /* only PDP type IP is supported */
      if( opchCmd->bear_desc.gprs_bear_prm.gprs_pdp_type NEQ PDP_TYPE_IP )
      {
        TRACE_EVENT("SAT OPCH FAIL, PDP type diff from IP");
        psaSAT_SendTrmResp( RSLT_ME_CAP,  &resp_data );
        return( FALSE );
      }

      cntxt.ctrl_qos                = PS_is_R97;
      cntxt.qos.qos_r97.preced   = opchCmd->bear_desc.gprs_bear_prm.gprs_prec;
      cntxt.qos.qos_r97.delay    = opchCmd->bear_desc.gprs_bear_prm.gprs_delay;
      cntxt.qos.qos_r97.relclass = opchCmd->bear_desc.gprs_bear_prm.gprs_rely;
      cntxt.qos.qos_r97.peak     = opchCmd->bear_desc.gprs_bear_prm.gprs_peak;
      cntxt.qos.qos_r97.mean     = opchCmd->bear_desc.gprs_bear_prm.gprs_mean;

    }

    else if ( opchCmd->bear_desc.bear_type EQ BT_DEFAULT )
    {
      /* default settings for GPRS channel = as subscribed */
      cntxt.ctrl_qos               = PS_is_R97;
      cntxt.qos.qos_r97.preced   = 0;
      cntxt.qos.qos_r97.delay    = 0;
      cntxt.qos.qos_r97.relclass = 0;
      cntxt.qos.qos_r97.peak     = 0;
      cntxt.qos.qos_r97.mean     = 0;
      /* store default settings in opchCmd, will be used for term response later on */
      opchCmd->bear_desc.gprs_bear_prm.gprs_prec  = 0;
      opchCmd->bear_desc.gprs_bear_prm.gprs_delay = 0;
      opchCmd->bear_desc.gprs_bear_prm.gprs_rely  = 0;
      opchCmd->bear_desc.gprs_bear_prm.gprs_peak  = 0;
      opchCmd->bear_desc.gprs_bear_prm.gprs_mean  = 0;
      opchCmd->bear_desc.gprs_bear_prm.gprs_pdp_type = PDP_TYPE_IP;
    } 
    else /* invalid bearer setting for GPRS */
    {
        TRACE_EVENT("SAT OPCH FAIL, invalid GPRS bearer settings");
      psaSAT_SendTrmResp( RSLT_ERR_REQ_VAL,  &resp_data );
      return( FALSE );
    }

    /* if an APN is given */
    if( opchCmd->v_nan_buf AND opchCmd->nan_buf.c_n_acc_name )
    {
     cmhSAT_cnvrtAPN2NetworkAdr( (UBYTE *)opchCmd->nan_buf.n_acc_name, 
                                  (UBYTE) opchCmd->nan_buf.c_n_acc_name, 
                                  (UBYTE *)cntxt.attributes.pdp_apn );

    }
    else  /* use default */
    {
      cntxt.attributes.pdp_apn[0] = 0;
    }

    /* if a local address is given */
    if( opchCmd->v_other_addr AND opchCmd->other_addr.v_ipv4_addr )
    {/* For ACISAT510 
         Populate the ip address
      */
      cntxt.attributes.pdp_addr.ctrl_ip_address = NAS_is_ipv4;
      memcpy(cntxt.attributes.pdp_addr.ip_address.ipv4_addr.a4 , 
             &opchCmd->other_addr.ipv4_addr, sizeof(cntxt.attributes.pdp_addr.ip_address.ipv4_addr.a4));

    }
    else  /* use default, dynamic address */
    {
      cntxt.attributes.pdp_addr.ctrl_ip_address = NAS_is_ip_not_present;
    }

    cntxt.attributes.d_comp = PDP_CONTEXT_D_COMP_OMITTED;
    cntxt.attributes.h_comp = PDP_CONTEXT_H_COMP_OMITTED;
    
    strncpy( cntxt.attributes.pdp_type, "IP", sizeof( T_PDP_CONTEXT_TYPE ));
    if( sAT_PlusCGDCONT( CMD_SRC_NONE,cId, &cntxt.attributes ) NEQ AT_CMPL )
    {
      TRACE_EVENT("SAT OPCH FAIL, invalid CGDCONT parms");
      psaSAT_SendTrmResp( RSLT_ME_CAP, &resp_data );
      return( FALSE );  /* primitive not needed anymore */
    }

     /* For ACISAT510 
       Set the working cid */
    work_cids[cid_pointer] = cId;

    /* don't handle R99 QoS */
    if( sAT_PlusCGQREQ( CMD_SRC_NONE, (U8)cId, &cntxt.qos) NEQ AT_CMPL )
    {
      TRACE_EVENT("SAT OPCH FAIL, invalid CGQREQ parms");
      psaSAT_SendTrmResp( RSLT_ME_CAP, &resp_data );
      return( FALSE );  /* primitive not needed anymore */
    }

    /* fill in channel table */
    satShrdPrm.chnTb.chnUsdFlg = TRUE;
    satShrdPrm.chnTb.chnRefId  = cId;
    satShrdPrm.chnTb.chnType   = B_GPRS;
    satShrdPrm.chnTb.lnkStat   = SIM_NO_LINK;

    /* store bearer parameters for later use */
    cmhSAT_storeGPRSPrms (opchCmd);

  }
  else

#endif  /* GPRS */

  /*
   *-------------------------------------------------------------------
   *  undefined channel 
   *-------------------------------------------------------------------
   */
  {
    /* respond with "error, required values are missing" */
    TRACE_EVENT("SAT OPCH FAIL, requires values missing");
    psaSAT_SendTrmResp( RSLT_ERR_REQ_VAL, &resp_data );
    return( FALSE );
  }


  
  if (  psaCC_ctbCallInUse ( ) )
   {
     /* send SAT response **/
       TRACE_EVENT("SAT ME BUSY on CALL");
       resp_data.add_content = ADD_ME_CALL_BUSY;
       resp_data.bufSize  = TRUE;
       resp_data.bearDesc = TRUE;
       resp_data.chnStat  = FALSE;
       psaSAT_SendTrmResp( RSLT_ME_UNAB_PROC, &resp_data );
      return( FALSE );
   }
/*
 *-------------------------------------------------------------------
 *  request for user confirmation
 *-------------------------------------------------------------------
 */
  cmhSAT_OpChnAlert( cId );

  return( TRUE );

}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : SAT                     |
|                                 ROUTINE : cmhSAT_storeCSDPrms     |
+-------------------------------------------------------------------+

  PURPOSE : This function stores the CSD open channel parameters for
            later use.

*/

GLOBAL void cmhSAT_storeCSDPrms ( T_OPEN_CHANNEL * opchCmd )
{
  T_SAT_CSD_PRM *p;

  TRACE_FUNCTION("cmhSAT_storeCSDPrms()");

  ACI_MALLOC(satShrdPrm.opchPrm, sizeof(T_SAT_CSD_PRM));
  TRACE_EVENT("ALLOC opchPrm");
  memset(satShrdPrm.opchPrm, 0, sizeof(T_SAT_CSD_PRM));

  p = (T_SAT_CSD_PRM*)satShrdPrm.opchPrm;

  if(opchCmd->bear_desc.bear_type EQ BT_DEFAULT)
    p->def_bear_prm = TRUE;
 
  p->csd_bear_prm = opchCmd->bear_desc.csd_bear_prm;

  if(opchCmd->v_dur)
  {
    p->v_dur = TRUE;
    p->dur = opchCmd->dur;
  }

  if(opchCmd->v_dur2)
  {
    p->v_dur2 = TRUE;
    p->dur2 = opchCmd->dur2;
  }

  if(opchCmd->v_other_addr)
  {
    p->v_other_addr = TRUE;
    p->other_addr = opchCmd->other_addr;
  }

  if(opchCmd->v_text)
  {
    p->v_log = TRUE;
    p->log = opchCmd->text;
  }

  if(opchCmd->v_text2)
  {
    p->v_pwd = TRUE;
    p->pwd = opchCmd->text2;
  }

  if(opchCmd->v_if_transp_lev)
  {
    p->v_itl = TRUE;
    p->itl = opchCmd->if_transp_lev;
  }

  if(opchCmd->v_data_dest_addr)
  {
    p->v_dda = TRUE;
    p->dda = opchCmd->data_dest_addr;
  }

  satShrdPrm.opchType = B_CSD;
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : SAT                     |
|                                 ROUTINE : cmhSAT_storeGPRSPrms    |
+-------------------------------------------------------------------+

  PURPOSE : This function stores the CSD open channel parameters for
            later use.

*/

GLOBAL void cmhSAT_storeGPRSPrms ( T_OPEN_CHANNEL * opchCmd )
{
  T_SAT_GPRS_PRM *p;

  TRACE_FUNCTION("cmhSAT_storeGPRSPrms()");

  ACI_MALLOC(satShrdPrm.opchPrm, sizeof(T_SAT_GPRS_PRM));
  TRACE_EVENT("ALLOC opchPrm");
  memset(satShrdPrm.opchPrm, 0, sizeof(T_SAT_GPRS_PRM));

  p = (T_SAT_GPRS_PRM*)satShrdPrm.opchPrm;

  if(opchCmd->bear_desc.bear_type EQ BT_DEFAULT)
    p->def_bear_prm = TRUE;

  p->gprs_bear_prm = opchCmd->bear_desc.gprs_bear_prm;

  if(opchCmd->v_nan_buf)
  {
    p->v_apn = TRUE;
    p->c_apn = opchCmd->nan_buf.c_n_acc_name;
    memcpy(p->apn, opchCmd->nan_buf.n_acc_name, opchCmd->nan_buf.c_n_acc_name);
  }

  if(opchCmd->v_other_addr)
  {
    p->v_other_addr = TRUE;
    p->other_addr = opchCmd->other_addr;
  }

  if(opchCmd->v_if_transp_lev)
  {
    p->v_itl = TRUE;
    p->itl = opchCmd->if_transp_lev;
  }

  if(opchCmd->v_data_dest_addr)
  {
    p->v_dda = TRUE;
    p->dda = opchCmd->data_dest_addr;
  }

  satShrdPrm.opchType = B_GPRS;
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : SAT                     |
|                                 ROUTINE : cmhSAT_cleanupOpChnPrms |
+-------------------------------------------------------------------+

  PURPOSE : This function cleans the Op channel parameters 

*/

GLOBAL void cmhSAT_cleanupOpChnPrms ( void )
{
  TRACE_FUNCTION("cmhSAT_cleanupOpChnPrms()");

  if( satShrdPrm.opchPrm ) 
  {  
    ACI_MFREE(satShrdPrm.opchPrm);
    satShrdPrm.opchPrm = NULL;
    TRACE_EVENT("FREE opchPrm");
  }

  satShrdPrm.opchPrmMdf = FALSE;
  satShrdPrm.opchCCMdfy = FALSE;
  satShrdPrm.opchStat   = OPCH_IDLE;
  satShrdPrm.opchType   = 0;
  satShrdPrm.gprsNtwCs  = 0;
  
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : SAT                      |
|                                 ROUTINE : cmhSAT_OpChnResetCnctFalg|
+--------------------------------------------------------------------+

  PURPOSE : This function cleans the Op channel parameters 

*/

GLOBAL void cmhSAT_OpChnResetCnctFlag ( void )
{
  TRACE_FUNCTION("cmhSAT_cleanupOpChnWAP()");
#ifdef CO_UDP_IP
  /* If transport layer is UDP, reset wap_call flag */
  if( satShrdPrm.chnTb.chnTPL EQ UDP )
  {
    sAT_PercentWAP ( CMD_SRC_NONE, 0 );
  }
#endif /* CO_UDP_IP */    
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : SAT                     |
|                                 ROUTINE : cmhSAT_OpChnFailed      |
+-------------------------------------------------------------------+

  PURPOSE : Indication that an open channel command has failed due to
            a reason indicated by cause and add_cause. If the failure
            occured in context with the OPEN CHANNEL command, a terminal 
            response will be sent and parameters will be cleaned up. 
            If the failure was in context with a SEND DATA command, the
            SIM BIP channel will be closed and parameters will be cleared
            up. If the failure was unexpected, a channel status event
            message will be sent, if enabled.

*/

GLOBAL void cmhSAT_OpChnFailed( UBYTE cause, T_ACI_SAT_TERM_RESP *resp_data )
{
  T_SIM_SAT_CHN chnInf;

  TRACE_FUNCTION("cmhSAT_OpChnFailed()");

  /* command context is OPEN CHANNEL */
  if((satShrdPrm.opchStat EQ OPCH_EST_REQ OR 
      satShrdPrm.opchStat EQ OPCH_CCSIM_REQ OR
      satShrdPrm.opchStat EQ OPCH_WAIT_CNF) AND
      satShrdPrm.cmdDet.cmdType EQ SAT_CMD_OPEN_CHANNEL )
  {
    resp_data->chnStat  = TRUE;
    psaSAT_SendTrmResp( cause, resp_data );

    if(!(satShrdPrm.opchStat EQ OPCH_WAIT_CNF))
    {
      R_AT( RAT_CME, (T_ACI_CMD_SRC)satShrdPrm.opchAcptSrc ) ( AT_CMD_A, CME_ERR_NotPresent );

      /* log result */
      cmh_logRslt ( (T_ACI_CMD_SRC)satShrdPrm.opchAcptSrc, RAT_CME, AT_CMD_A,
                    -1, BS_SPEED_NotPresent, CME_ERR_NotPresent );
    }
    cmhSAT_cleanupOpChnPrms();
  }

  /* command context is SEND DATA */
  else if( satShrdPrm.opchStat EQ OPCH_EST_REQ AND
      satShrdPrm.cmdDet.cmdType EQ SAT_CMD_SEND_DATA )
  {
    if( satShrdPrm.chnTb.lnkStat EQ SIM_LINK_OPEN )
    {
      /* check if user has cleared down the call */
      if( satShrdPrm.chnTb.chnType EQ B_CSD AND
          (!psaCC_ctbIsValid (satShrdPrm.chnTb.chnRefId) OR
           psaCC_ctb(satShrdPrm.chnTb.chnRefId)->curCmd EQ AT_CMD_H OR
           psaCC_ctb(satShrdPrm.chnTb.chnRefId)->curCmd EQ AT_CMD_CHUP))
      {
          cause = RSLT_USR_CLR_DWN;
      }
#ifdef DTI
      chnInf.bipConn = SIM_BIP_CLOSE_CHANNEL;
      chnInf.dtiConn = SIM_DTI_UNKNOWN;
      chnInf.chnId   = CHANNEL_ID_1;
      chnInf.genRes  = cause;
      chnInf.addRes  = resp_data->add_content;

      psaSIM_SATBIPChn( chnInf, cmhSAT_OpChnClose );
#endif /* DTI */
    }
  }
  /* unsolicited problem */
  else if( satShrdPrm.opchStat EQ OPCH_IDLE )
  {
    chnInf.bipConn  = SIM_BIP_UNKNOWN;
    chnInf.dtiConn = SIM_DTI_UNKNOWN;
    
    if( satShrdPrm.chnTb.lnkStat EQ SIM_LINK_OPEN )
    {
      chnInf.bipConn = SIM_BIP_CLOSE_CHANNEL;
    }
    if( satShrdPrm.chnTb.lnkStat EQ SIM_LINK_CNCT )
    {
      chnInf.bipConn = SIM_BIP_CLOSE_CHANNEL; 
      chnInf.dtiConn = SIM_DTI_DISCONNECT;
    }
#ifdef DTI
    if( chnInf.bipConn )
    {
      chnInf.chnId   = CHANNEL_ID_1;
      chnInf.genRes  = RSLT_BEARIND_PERR;
      chnInf.addRes  = ADD_BIP_CHAN_CLOSD;

      psaSIM_SATBIPChn( chnInf, cmhSAT_OpChnClose );

      cmhSAT_OpChnStatEvnt();
    }
#endif /* DTI */
  }
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : SAT                     |
|                                 ROUTINE : cmhSAT_OpChnAlert       |
+-------------------------------------------------------------------+

  PURPOSE : This function alerts the user about a pending SAT open
            channel command, which is ready for execution.

*/

GLOBAL void cmhSAT_OpChnAlert( SHORT cId )

{
  UBYTE idx;
  LONG rdl = ACI_NumParmNotPresent;
  T_ACI_SATA_ADD addPrm;

  if( satShrdPrm.opchType EQ B_CSD )
  {
    rdl = cmhSAT_ChckRedial(cId, ((T_SAT_CSD_PRM*)satShrdPrm.opchPrm)->v_dur, 
                            &(((T_SAT_CSD_PRM*)satShrdPrm.opchPrm)->dur));

    addPrm.chnType = SATA_CT_CSD;
    cId += 1;
  }
  else if ( satShrdPrm.opchType EQ B_GPRS )

    addPrm.chnType = SATA_CT_GPRS;

  addPrm.chnEst = ( satShrdPrm.cmdDet.cmdQlf & QLF_OPCH_IMMDT_LINK_EST )?
                    SATA_EST_IM:SATA_EST_OD;

  for( idx = 0; idx < CMD_SRC_MAX; idx++ )
  {
    R_AT( RAT_SATA, (T_ACI_CMD_SRC)idx )( cId, rdl, &addPrm );
  }

  satShrdPrm.ntfy = USR_NTF_SETUP_CAL;
  satShrdPrm.opchStat = OPCH_WAIT_CNF;
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : SAT                     |
|                                 ROUTINE : cmhSAT_OpChnChkTmpProblem|
+-------------------------------------------------------------------+

  PURPOSE : This function checks for a temporary situation, which
            prevents the open channel command to be completed. If such
            a condition exists, the open channel command will be aborted
            with a suitable terminal response and the function returns
            TRUE

*/

GLOBAL BOOL cmhSAT_OpChnChkTmpProblem( void )

{
  T_ACI_SAT_TERM_RESP resp_data;      /* holds terminal response parms */
  BOOL cleanup = FALSE;

  psaSAT_InitTrmResp( &resp_data );


  TRACE_FUNCTION("cmhSAT_OpChnChkTmpProblem()");

  /*
   *-------------------------------------------------------------------
   * check channel type independent conditions
   *-------------------------------------------------------------------
   */
#ifdef CO_UDP_IP
  /* check for a busy UDP situation, WAP is a synonym for UDP */
  if( satShrdPrm.chnTb.chnTPL EQ UDP AND
      Wap_Call )
  {
    TRACE_EVENT("SAT OPCH FAIL, UDP is busy");
    resp_data.add_content = ADD_NO_CAUSE;
    cmhSAT_OpChnFailed( RSLT_ME_UNAB_PROC, &resp_data );
    cleanup = TRUE;
  }
  /*
   *-------------------------------------------------------------------
   * check CSD channel conditions 
   *-------------------------------------------------------------------
   */
  else
#endif  /* CO_UDP_IP */

  if(  satShrdPrm.opchType EQ B_CSD )
  {
    /* check for busy SS condition */
    if( psaSS_stbFindActSrv( NO_ENTRY ) NEQ NO_ENTRY )
    {
      /* send SAT response */
      TRACE_EVENT("SAT OPCH FAIL, SS busy");
      resp_data.add_content = ADD_ME_SS_BUSY;
      cmhSAT_OpChnFailed( RSLT_ME_UNAB_PROC, &resp_data );

      cleanup =  TRUE;
    }

    /* check for busy call condition */
    else if( psaSAT_ctbFindActCall() NEQ NO_ENTRY )
    {
      /* send SAT response */
      TRACE_EVENT("SAT OPCH FAIL, busy on call");
      resp_data.add_content = ADD_ME_CALL_BUSY;
      cmhSAT_OpChnFailed( RSLT_ME_UNAB_PROC, &resp_data );

      cleanup =  TRUE;
    }
#if defined (GPRS) AND defined (DTI)
    /* CSD channel, but class CG */
    else if( gmmShrdPrm.mobile_class EQ GMMREG_CLASS_CG )
    {
      /* send SAT response */
      TRACE_EVENT("SAT OPCH FAIL, no channel avail");
      resp_data.add_content = ADD_BIP_NO_CHAN_AVAIL;
      cmhSAT_OpChnFailed( RSLT_BEARIND_PERR, &resp_data );

      cleanup =  TRUE;
    }
#endif /* GPRS */

  }

  /*
   *-------------------------------------------------------------------
   * check GPRS channel conditions 
   *-------------------------------------------------------------------
   */
#if defined (GPRS) AND defined (DTI)
  else if( satShrdPrm.opchType EQ B_GPRS )
  {
    /* GPRS channel, class B */
    if( gmmShrdPrm.mobile_class EQ GMMREG_CLASS_BC OR
        gmmShrdPrm.mobile_class EQ GMMREG_CLASS_BG    )
    {
      if( psaSS_stbFindActSrv( NO_ENTRY ) NEQ NO_ENTRY )
      {
        /* send SAT response */
        TRACE_EVENT("SAT OPCH FAIL, SS busy");
        resp_data.add_content = ADD_ME_SS_BUSY;
        cmhSAT_OpChnFailed( RSLT_ME_UNAB_PROC, &resp_data );

        cleanup =  TRUE;
      }

      /* check for busy call condition */
      else if( psaSAT_ctbFindActCall() NEQ NO_ENTRY )
      {
        /* send SAT response */
        TRACE_EVENT("SAT OPCH FAIL, busy on call");
        resp_data.add_content = ADD_ME_CALL_BUSY;
        cmhSAT_OpChnFailed( RSLT_ME_UNAB_PROC, &resp_data );

        cleanup =  TRUE;
      }
    }

    /* GPRS channel, but class CC */
    else if( gmmShrdPrm.mobile_class EQ GMMREG_CLASS_CC )
    {
      /* send SAT response */
      TRACE_EVENT("SAT OPCH FAIL, no channel avail");
      resp_data.add_content = ADD_BIP_NO_CHAN_AVAIL;
      cmhSAT_OpChnFailed( RSLT_BEARIND_PERR, &resp_data );

      cleanup =  TRUE;
    }

    /* no channel left for GPRS */
    else if( pdp_context_get_free_cid() EQ PDP_CONTEXT_CID_INVALID )
    {
      /* send SAT response */
      TRACE_EVENT("SAT OPCH FAIL, no channel avail");
      resp_data.add_content = ADD_BIP_NO_CHAN_AVAIL;
      cmhSAT_OpChnFailed( RSLT_BEARIND_PERR, &resp_data );

      cleanup =  TRUE;
    }

    /* check SM entity status */
    else if( smEntStat.entOwn NEQ CMD_SRC_NONE OR
             smEntStat.curCmd NEQ AT_CMD_NONE )
    {
      /* send SAT response */
      TRACE_EVENT("SAT OPCH FAIL, SM entity busy");
      resp_data.add_content = ADD_NO_CAUSE;
      cmhSAT_OpChnFailed( RSLT_ME_UNAB_PROC, &resp_data );

      cleanup =  TRUE;
    }
  }
#endif /* GPRS */

  /*
   *-------------------------------------------------------------------
   * cleanup if necessary
   *-------------------------------------------------------------------
   */
  if( cleanup )
  {
    if( satShrdPrm.opchType EQ B_CSD )
    {
      /* free ctb entry */
      psaCC_FreeCtbNtry (satShrdPrm.chnTb.chnRefId);
    }

    cmhSAT_cleanupOpChnPrms();

    return ( TRUE );
  }

  return( FALSE );
}


/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : SAT                     |
|                                 ROUTINE : cmhSAT_OpChnUDPActiveCsd|
+-------------------------------------------------------------------+

  PURPOSE : This callback function will be called when the UDP stack
            is active

*/

GLOBAL void cmhSAT_OpChnUDPActiveCsd(T_ACI_RETURN result)
{
  T_ACI_SAT_TERM_RESP resp_data;      /* holds terminal response parms */

  TRACE_FUNCTION("cmhSAT_OpChnUDPActiveCsd()");

  /* Ensure we never dereference NULL in the CC call table */
  if (!psaCC_ctbIsValid (tcpipShrdPrm.wap_call_id))
  {
    TRACE_ERROR ("Call table entry disappeared");
    ccShrdPrm.datStat = DS_IDL ;
    psaSAT_InitTrmResp( &resp_data );
    resp_data.add_content = ADD_NO_CAUSE;

    cmhSAT_OpChnFailed( RSLT_ME_UNAB_PROC, &resp_data );
    return;
  }

  /*
   * activate RA connection: in case of failure clear call !
   */
  ccShrdPrm.datStat = DS_ACT_REQ;

  if(cmhRA_Activate((T_ACI_CMD_SRC)psaCC_ctb(tcpipShrdPrm.wap_call_id)->curSrc,
                    (T_ACI_AT_CMD)psaCC_ctb(tcpipShrdPrm.wap_call_id)->curCmd,
                    tcpipShrdPrm.wap_call_id)
      NEQ AT_EXCT)
  {
    TRACE_EVENT("RA ACTIVATION FAILURE -> DISC CALL");
    ccShrdPrm.datStat = DS_IDL ;
    psaCC_ctb(tcpipShrdPrm.wap_call_id)->nrmCs = MNCC_CAUSE_CALL_CLEAR;
    psaCC_ClearCall (tcpipShrdPrm.wap_call_id);


    psaSAT_InitTrmResp( &resp_data );
    resp_data.add_content = ADD_NO_CAUSE;

    cmhSAT_OpChnFailed( RSLT_ME_UNAB_PROC, &resp_data );
  }
}


/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : SAT                     |
|                                 ROUTINE : cmhSAT_OpChnUDPConfCsd  |
+-------------------------------------------------------------------+

  PURPOSE : This callback function will be called when the UDP stack
            is configured

*/
GLOBAL void cmhSAT_OpChnUDPConfCsd(T_ACI_RETURN result)
{

  TRACE_FUNCTION("cmhSAT_OpChnUDPConfCsd()");
  /* 
   * send the SIM_BIP_CONFIG_REQ primitive providing details of the connection
   * to SIM 
   */
  psaSIM_Bip_Config_Req();
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : SAT                     |
|                                 ROUTINE : cmhSAT_OpChnUDPDeactCsd |
+-------------------------------------------------------------------+

  PURPOSE : This callback function will be called when the UDP stack
            is deactive

*/

GLOBAL void cmhSAT_OpChnUDPDeactCsd(T_ACI_RETURN result)
{
  T_ACI_SAT_TERM_RESP resp_data;      /* holds terminal response parms */
  
  TRACE_FUNCTION("cmhSAT_OpChnUDPDeactCsd()");

  psaSAT_InitTrmResp( &resp_data );

  /*
   *-------------------------------------------------------------------
   * check if deactivation was intended 
   *-------------------------------------------------------------------
   */
  if( satShrdPrm.opchStat EQ OPCH_CLS_REQ )
  {
     /* nothing to do here anymore */
  }

  /*
   *-------------------------------------------------------------------
   * deactivation during an establish request 
   *-------------------------------------------------------------------
   */
  else if( satShrdPrm.opchStat EQ OPCH_EST_REQ )
  {
    if (cmhL2R_Deactivate() NEQ AT_EXCT)
    {
      TRACE_EVENT("L2R DEACTIVATION FAILURE ");
      if (psaCC_ctbIsValid (tcpipShrdPrm.wap_call_id))
      {
        psaCC_ctb(tcpipShrdPrm.wap_call_id)->nrmCs = MNCC_CAUSE_CALL_CLEAR;
        psaCC_ClearCall(tcpipShrdPrm.wap_call_id);
      }
    }

    resp_data.add_content = ADD_NO_CAUSE;

    cmhSAT_OpChnFailed( RSLT_ME_UNAB_PROC, &resp_data );

  }
}
/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : SAT                     |
|                                 ROUTINE : cmhSAT_OpBIPChnOpen     |
+-------------------------------------------------------------------+

  PURPOSE : This callback function will be called when the SIM entity
            has tried to open a BIP channel

*/

GLOBAL void cmhSAT_OpBIPChnOpen( UBYTE bipConn, UBYTE chnId )
{
  T_CC_CMD_PRM * pCCCmdPrm;       /* points to CC command parameters */
  T_ACI_SAT_TERM_RESP resp_data;  /* holds terminal response parms */
  UBYTE srcId;                    /* holds source ID */
  UBYTE cause;
  SHORT cId;  /* holds caller ID for pending CSD call (case immediately) */
#ifdef GPRS   
  SHORT cid_array[2] = { 0,PDP_CONTEXT_CID_INVALID/*INVALID_CID*/ }; /* holds cids for GPRS context */   
#endif /* GPRS */  
  
  TRACE_FUNCTION("cmhSAT_OpBIPChnOpen()");

  /* get source id */ 
  srcId = satShrdPrm.opchAcptSrc;

  psaSAT_InitTrmResp( &resp_data );

  /* send TR in the case of on demand establishment */ 
  if( satShrdPrm.opchStat EQ OPCH_ON_DMND )
  {
    /* CASE: ON DEMAND */
    
    /* check if opening has been successful */
    if( bipConn & SIM_BIP_OPEN_CHANNEL )
    {
      /* BEARER: GPRS OR CSD */

      /* set link state to OPEN */ 
      satShrdPrm.chnTb.lnkStat = SIM_LINK_OPEN;

      resp_data.chnStat  = TRUE;
      resp_data.bearDesc = TRUE;
      resp_data.bufSize  = TRUE;
      cause = (satShrdPrm.opchCCMdfy)?RSLT_PERF_MDFY_SIM:RSLT_PERF_SUCCESS;

      psaSAT_SendTrmResp( cause, &resp_data );

      R_AT( RAT_OK, (T_ACI_CMD_SRC)satShrdPrm.opchAcptSrc ) ( AT_CMD_A );

      /* log result */
      cmh_logRslt ( (T_ACI_CMD_SRC)satShrdPrm.opchAcptSrc, RAT_OK, AT_CMD_A,
                               -1, BS_SPEED_NotPresent, CME_ERR_NotPresent );
    }
    /* unsuccessful channel opening */
    else
    {
      /* BEARER: GPRS OR CSD */

      TRACE_ERROR("cmhSAT_OpChnOpen: ERROR: OPEN BIP CHANNEL unsuccessful");
      
      resp_data.add_content = ADD_BIP_NO_CHAN_AVAIL;
      cause = RSLT_BEARIND_PERR;

      cmhSAT_cleanupOpChnPrms();

      psaSAT_SendTrmResp( cause, &resp_data );

      R_AT( RAT_CME, (T_ACI_CMD_SRC)satShrdPrm.opchAcptSrc ) ( AT_CMD_A, CME_ERR_NotPresent );

      /* log result */
      cmh_logRslt ( (T_ACI_CMD_SRC)satShrdPrm.opchAcptSrc, RAT_CME, AT_CMD_A,
                    -1, BS_SPEED_NotPresent, CME_ERR_NotPresent );
    }
      
  }
  else if ( satShrdPrm.opchStat EQ OPCH_EST_REQ )
  {
    /* CASE: IMMEDIATELY */
    /* Check whether OPEN BIP CHANNEL successful */
    if( bipConn & SIM_BIP_OPEN_CHANNEL )
    {        

      /* set link state to OPEN, since the BIP and DTI establishment has split 
       * this state is alse used for "immediately" OPEN CHANNEL command */ 
      satShrdPrm.chnTb.lnkStat = SIM_LINK_OPEN;

      /* check for BEARER */
      if (satShrdPrm.opchType EQ B_CSD) 
      {
        /* BEARER: CSD */        
        /* proceed to set up bearer for CSD */
        
        /* get SAT call cId */    
        cId = psaCC_ctbFindCall( OWN_SRC_SAT, CS_SAT_CSD_REQ, NO_VLD_CT );
        
        /* check if a call has been found */ 
        if ( cId NEQ (-1)) 
        {
          pCCCmdPrm = &cmhPrm[srcId].ccCmdPrm;
    
          if( satShrdPrm.chnTb.chnTPL EQ UDP )
          {
            /* TRANSPORT LAYER: UDP */

            /* enable establishment of UDP data chain */
            sAT_PercentWAP ( (T_ACI_CMD_SRC)psaCC_ctb(cId)->curSrc , 1 );

            /* setup PPP parameters */
            cmhSAT_OpChnSetPPP( B_CSD );
          }
          /* finally set up call */
          cmhCC_flagCall( cId, &(pCCCmdPrm->mltyCncFlg));
          cmhCC_NewCall(cId, (T_ACI_CMD_SRC)srcId, AT_CMD_A);    
        }
        else 
        {
          TRACE_FUNCTION("cmhSAT_OpChnOpen: No CSD call pending");
        }
      }
#if defined (GPRS) AND defined (DTI)
      else if (satShrdPrm.opchType EQ B_GPRS) 
      {
        /* BEARER: GPRS */

        /* copy channel reference ID */
        cid_array[0] = satShrdPrm.chnTb.chnRefId;

        if( satShrdPrm.chnTb.chnTPL EQ UDP )
        {
          /* TRANSPORT LAYER: UDP */

          /* enable establishment of UDP data chain */
          sAT_PercentWAP ( (T_ACI_CMD_SRC)srcId , 1 );

          /* setup PPP parameters */
          cmhSAT_OpChnSetPPP( B_GPRS );

          /* activate context with UDP */
          cmhSAT_OpChnUDPActivateGprs();
        }
        else
        {
          /* NO TRANSPORT LAYER: SNDCP only */
        
          /* activate context with no transport layer */
          if( sAT_PlusCGACT( (T_ACI_CMD_SRC)srcId, CGACT_STATE_ACTIVATED, cid_array ) NEQ AT_EXCT )
          {
            /* activate context command could not proccessed */ 
            /* throw error */
            TRACE_ERROR("cmhSAT_OpChnOpen: ERROR: activate context with no TRL failed");
            R_AT( RAT_CME, (T_ACI_CMD_SRC)satShrdPrm.opchAcptSrc ) ( AT_CMD_A, CME_ERR_NoConnect );
          }
        }
      } /* end bearer B_GPRS */
#endif /* GPRS AND DTI */
    }
    else /* OPEN BIP CHANNEL unsuccessful */
    {
      /* CASE: ON DEMAND and IMMEDIATELY */
      /* BEARER: CSD and GPRS */

      TRACE_ERROR("cmhSAT_OpChnOpen: ERROR: OPEN BIP CHANNEL unsuccessful");
      
      resp_data.add_content = ADD_BIP_NO_CHAN_AVAIL;
      cause = RSLT_BEARIND_PERR;

      /* store current status of BIP channel */
      simShrdPrm.sim_dti_chPrm->sat_chn_prm.bipConn = SIM_BIP_CLOSE_CHANNEL;

      cmhSAT_cleanupOpChnPrms();

      psaSAT_SendTrmResp( cause, &resp_data );

      R_AT( RAT_CME, (T_ACI_CMD_SRC)satShrdPrm.opchAcptSrc ) ( AT_CMD_A, CME_ERR_NotPresent );

      /* log result */
      cmh_logRslt ( (T_ACI_CMD_SRC)satShrdPrm.opchAcptSrc, RAT_CME, AT_CMD_A,
                    -1, BS_SPEED_NotPresent, CME_ERR_NotPresent );
    }
  } /* end if ( satShrdPrm.opchStat EQ OPCH_EST_REQ ) */
}


/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : SAT                     |
|                                 ROUTINE : cmhSAT_OpChnCnct        |
+-------------------------------------------------------------------+

  PURPOSE : This callback function will be called when the SIM entity
            has tried to connect a DTI channel

*/

GLOBAL void cmhSAT_OpChnCnct( UBYTE dtiConn, UBYTE chnId )
{
#ifdef DTI
  T_SIM_SAT_CHN chnInf;
  
#ifdef GPRS
  SHORT cid_array[2] = { 0, PDP_CONTEXT_CID_INVALID };
#endif /* GPRS */
  
  
  TRACE_FUNCTION("cmhSAT_OpChnCnct()");

  /* connection was successful */
  if( dtiConn & SIM_DTI_CONNECT )
  {
    satShrdPrm.chnTb.lnkStat = SIM_LINK_CNCT;
    TRACE_EVENT("cmhSAT_OpChnCnct: SIM-DTI connection successful");
  }
  else
  {
  /* unsuccessful connection */
    TRACE_EVENT("cmhSAT_OpChnCnct: ERROR: SIM-DTI connection unsuccessful!");

    /* if the channel is still open */

    /* SAT E patch: Since this information isn't available from incoming 
     * SIM_DTI_CNF look into the shrd parameters whether there has been a 
     * already BIP channel established within this channel connection    */
    /* former checked: dtiConn & SIM_BIP_OPEN_CHANNEL*/
    if( (satShrdPrm.chnTb.lnkStat EQ SIM_LINK_OPEN) )
    {
      /* check for SEND DATA/OPEN CHANNEL command context */
      if( satShrdPrm.opchStat EQ OPCH_EST_REQ AND
          (satShrdPrm.cmdDet.cmdType EQ SAT_CMD_OPEN_CHANNEL OR
           satShrdPrm.cmdDet.cmdType EQ SAT_CMD_SEND_DATA))
      {
        chnInf.bipConn = SIM_BIP_CLOSE_CHANNEL;
        chnInf.dtiConn = SIM_DTI_UNKNOWN; 
        chnInf.chnId   = CHANNEL_ID_1;
        chnInf.genRes  = RSLT_BEARIND_PERR;
        chnInf.addRes  = ADD_BIP_NO_CHAN_AVAIL;

        psaSIM_SATBIPChn( chnInf, cmhSAT_OpChnClose );
      }

      satShrdPrm.chnTb.lnkStat = SIM_LINK_OPEN;
    }
    /* if bearer is still in use */
    else if ( satShrdPrm.chnTb.chnUsdFlg )            
    {
      /* if a CSD is still in progress */
      if( satShrdPrm.chnTb.chnType EQ B_CSD )
      {
        SHORT dummy;

        cmhCC_ClearCall( satShrdPrm.chnTb.chnRefId,
                         MNCC_CAUSE_CALL_CLEAR,
                         (T_ACI_CMD_SRC)satShrdPrm.opchAcptSrc,
                         AT_CMD_NONE,
                         &dummy);
      }

#ifdef GPRS
      /* if a GPRS context is still in progress */
      if( satShrdPrm.chnTb.chnType EQ B_GPRS )
      {
        cid_array[0] = satShrdPrm.chnTb.chnRefId;

        sAT_PlusCGACT( (T_ACI_CMD_SRC)satShrdPrm.opchAcptSrc, 
                       CGACT_STATE_DEACTIVATED, cid_array );     
      }
#endif  /* GPRS */
    }
  }
#endif /* DTI */
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : SAT                     |
|                                 ROUTINE : cmhSAT_OpChnClose       |
+-------------------------------------------------------------------+

  PURPOSE : This callback function will be called when the SIM entity
            has tried to close a channel

*/

GLOBAL void cmhSAT_OpChnClose( UBYTE bipConn, UBYTE chnId )
{
  T_ACI_SAT_TERM_RESP resp_data;      /* holds terminal response parms */
  
  TRACE_FUNCTION("cmhSAT_OpChnClose()");

   /* channel closed */
  if( bipConn & SIM_BIP_CLOSE_CHANNEL )
  {
    satShrdPrm.chnTb.lnkStat = SIM_NO_LINK;

    psaSAT_InitTrmResp( &resp_data );

    /* check for OPEN CHANNEL command context, immediate channel */
    if( satShrdPrm.opchStat EQ OPCH_EST_REQ AND
        satShrdPrm.cmdDet.cmdType EQ SAT_CMD_OPEN_CHANNEL )
    {
      resp_data.add_content = ADD_BIP_NO_CHAN_AVAIL;
      resp_data.chnStat  = TRUE;

      cmhSAT_OpChnFailed(RSLT_BEARIND_PERR, &resp_data);
      cmhSAT_cleanupOpChnPrms();

    }

    /* check for SEND DATA command context, on-demand channel */
    else if( satShrdPrm.opchStat EQ OPCH_EST_REQ AND
             satShrdPrm.cmdDet.cmdType EQ SAT_CMD_SEND_DATA )
    {
      cmhSAT_cleanupOpChnPrms();
    }

    /* check for intented channel closing */
    else if( satShrdPrm.opchStat EQ OPCH_CLS_REQ AND
             satShrdPrm.cmdDet.cmdType EQ SAT_CMD_CLOSE_CHANNEL)
    {
      resp_data.chnStat  = FALSE;
      psaSAT_SendTrmResp( RSLT_PERF_SUCCESS, &resp_data );
      if ( dti_cntrl_is_dti_channel_connected (DTI_ENTITY_UDP, simShrdPrm.sat_class_e_dti_id) 
#ifdef GPRS 
          OR dti_cntrl_is_dti_channel_connected (DTI_ENTITY_SNDCP, simShrdPrm.sat_class_e_dti_id)
#endif /* GPRS */          
         )            
      {
        cmhSAT_cleanupOpChnPrms();
      }
      else /* L2R or TRA - dti channel has to be closed explicitly */ 
      {
        /* close existing DTI channel first */ 
        dti_cntrl_close_dpath_from_dti_id(simShrdPrm.sat_class_e_dti_id);
        cmhSAT_cleanupOpChnPrms();
      }
    }


    /* if bearer is still in use */     
    if( satShrdPrm.chnTb.chnUsdFlg )
    {
      /* if a CSD is still in progress */
      if( satShrdPrm.chnTb.chnType EQ B_CSD )
      {
        SHORT dummy;

        cmhCC_ClearCall( satShrdPrm.chnTb.chnRefId,
                         MNCC_CAUSE_CALL_CLEAR,
                         (T_ACI_CMD_SRC)satShrdPrm.opchAcptSrc,
                         AT_CMD_NONE,
                         &dummy);
      }
#if defined (GPRS) AND defined (DTI)
      /* if a GPRS context is still in progress */
      if( satShrdPrm.chnTb.chnType EQ B_GPRS )
      {
        SHORT cid_array[2] = { 0,PDP_CONTEXT_CID_INVALID };
        cid_array[0] = satShrdPrm.chnTb.chnRefId;

        sAT_PlusCGACT( (T_ACI_CMD_SRC)satShrdPrm.opchAcptSrc, 
                       CGACT_STATE_DEACTIVATED, cid_array );     
      }
#endif  /* GPRS */
    }
  }
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : SAT                     |
|                                 ROUTINE : cmhSAT_OpChnCSDDown     |
+-------------------------------------------------------------------+

  PURPOSE : This function is used to indictate that the CSD data
            connection is down. 

*/

GLOBAL void cmhSAT_OpChnCSDDown( SHORT cId, UBYTE tpl )
{
  T_ACI_SAT_TERM_RESP resp_data;      /* holds terminal response parms */

  TRACE_FUNCTION("cmhSAT_OpChnCSDDown()");

  psaSAT_InitTrmResp( &resp_data );
  resp_data.add_content = ADD_NO_CAUSE;

  if( satShrdPrm.chnTb.chnUsdFlg AND
      satShrdPrm.chnTb.chnType  EQ B_CSD AND
     (satShrdPrm.chnTb.chnTPL   EQ tpl OR tpl EQ TPL_DONT_CARE) AND
      satShrdPrm.chnTb.chnRefId EQ cId )
  {

    /* If transport layer is UDP, reset wap_call flag */
    if( satShrdPrm.chnTb.chnTPL EQ UDP )
    {
      sAT_PercentWAP ( CMD_SRC_NONE, 0 );

      /* clear parameters of dti connection */            
    }    

    if(simShrdPrm.sat_class_e_dti_id NEQ DTI_DTI_ID_NOTPRESENT)
    {
      dti_cntrl_erase_entry(simShrdPrm.sat_class_e_dti_id);
      dti_cntrl_clear_conn_parms(simShrdPrm.sat_class_e_dti_id);
      simShrdPrm.sat_class_e_dti_id = DTI_DTI_ID_NOTPRESENT;
      TRACE_EVENT("sat_class_e_dti_id reset");
    }
    
    cmhSAT_OpChnFailed( RSLT_ME_UNAB_PROC, &resp_data );
  }
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : SAT                     |
|                                 ROUTINE : cmhSAT_OpChnGPRSDeact   |
+-------------------------------------------------------------------+

  PURPOSE : This function is used to indictate that the CSD data
            connection is down. 

*/
#ifdef GPRS
GLOBAL void cmhSAT_OpChnGPRSDeact( void )
{

  SHORT cid;
  TRACE_FUNCTION("cmhSAT_OpChnGPRSDeact()");

  cid = gaci_get_cid_over_dti_id(wap_dti_id);
  
  /* If transport layer is UDP, reset wap_call flag */
  if( satShrdPrm.chnTb.chnTPL EQ UDP )
  {
    cmhSM_disconnect_cid(cid, GC_TYPE_WAP );
    sAT_PercentWAP ( CMD_SRC_NONE, 0 );
    if(work_cids[cid_pointer] EQ cid)
    {
      smEntStat.curCmd = AT_CMD_NONE;
      *work_cids = 0;
      cid_pointer = 0;
    }
  }    

  /* clear parameters of dti connection */            
  if(simShrdPrm.sat_class_e_dti_id NEQ DTI_DTI_ID_NOTPRESENT)
  {
    dti_cntrl_erase_entry(simShrdPrm.sat_class_e_dti_id);
    dti_cntrl_clear_conn_parms(simShrdPrm.sat_class_e_dti_id);
    simShrdPrm.sat_class_e_dti_id = DTI_DTI_ID_NOTPRESENT;
    TRACE_EVENT("sat_class_e_dti_id reset");
  }
  
}
#endif /* GPRS */
/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : SAT                     |
|                                 ROUTINE : cmhSAT_OpChnSIMFail     |
+-------------------------------------------------------------------+

  PURPOSE : This callback function will be called when the SIM entity
            has reported a failure

*/

GLOBAL void cmhSAT_OpChnSIMFail( UBYTE dtiConn, UBYTE bipConn, UBYTE chnId )
{
#ifdef DTI
  T_SIM_SAT_CHN chnInf;
  
  TRACE_FUNCTION("cmhSAT_OpChnSIMFail()");

  /* if the channel is still open */
  if( bipConn & SIM_BIP_OPEN_CHANNEL )
  {
    chnInf.bipConn = SIM_BIP_CLOSE_CHANNEL;
    chnInf.dtiConn = SIM_DTI_UNKNOWN; 
    chnInf.chnId   = CHANNEL_ID_1;
    chnInf.genRes  = RSLT_BEARIND_PERR;
    chnInf.addRes  = ADD_BIP_NO_CHAN_AVAIL;

    psaSIM_SATBIPChn( chnInf, cmhSAT_OpChnClose );

    satShrdPrm.chnTb.lnkStat = SIM_LINK_OPEN;
  }
  /* if bearer is still in use */
  else if ( satShrdPrm.chnTb.chnUsdFlg )            
  {
    /* if a CSD is still in progress */
    if( satShrdPrm.chnTb.chnType EQ B_CSD )
    {
      SHORT dummy;

      cmhCC_ClearCall( satShrdPrm.chnTb.chnRefId,
                       MNCC_CAUSE_CALL_CLEAR,
                       (T_ACI_CMD_SRC)satShrdPrm.opchAcptSrc,
                       AT_CMD_NONE,
                       &dummy);
    }

#ifdef GPRS
    /* if a GPRS context is still in progress */
    if( satShrdPrm.chnTb.chnType EQ B_GPRS )
    {
      SHORT cid_array[2] = { 0,INVALID_CID };
      cid_array[0] = satShrdPrm.chnTb.chnRefId;

      sAT_PlusCGACT( (T_ACI_CMD_SRC)satShrdPrm.opchAcptSrc, 
                     CGACT_STATE_DEACTIVATED, cid_array );     
    }
#endif  /* GPRS */
  }
#endif  /* DTI */
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : SAT                     |
|                                 ROUTINE : cmhSAT_OpChnChckCSD     |
+-------------------------------------------------------------------+

  PURPOSE : This function is used to check if an open channel command
            for CSD with the given TPL is pending

*/

GLOBAL BOOL cmhSAT_OpChnChckCSD( UBYTE tpl )
{

  /* in case of a SAT channel establishment */
  if( satShrdPrm.opchStat EQ OPCH_EST_REQ AND 
      satShrdPrm.chnTb.chnTPL  EQ tpl AND
      satShrdPrm.opchType EQ B_CSD )
  {
    return( TRUE );
  }

  return( FALSE );
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : SAT                     |
|                                 ROUTINE : cmhSAT_OpChnSIMCnctReq  |
+-------------------------------------------------------------------+

  PURPOSE : This function is used to request a connection between SIM
            and the specified unit
*/

GLOBAL void cmhSAT_OpChnSIMCnctReq(UBYTE unit)
{
#ifdef DTI
  T_SIM_SAT_CHN chnInf;

  TRACE_FUNCTION("cmhSAT_OpChnSIMCnctReq()");

  if( satShrdPrm.opchStat EQ OPCH_EST_REQ )
  {
    /* DTI CHANNEL ESTABLISHMENT requested */

    /* check for modification of bearer parameters */
    cmhSAT_OpChnChckBear();

    /* setup channel info for PSA */
    chnInf.dtiUnit = unit;
    chnInf.chnId   = CHANNEL_ID_1;
    chnInf.dtiConn = 0;    
    chnInf.dtiConn = SIM_DTI_CONNECT;
    
    if ( unit NEQ DTI_ENTITY_UDP )
    {
      /* init DTI channel for SIM -> SNDCP, L2R and TRA */
      psaSIM_SATChn( chnInf, cmhSAT_OpChnCnct );
    }
    else
    {
      /* prepare parameters for SIM -> UDP connection */
      /* copy settings into shrd prms */
      memcpy(&simShrdPrm.sim_dti_chPrm->sat_chn_prm,&chnInf,sizeof(T_SIM_SAT_CHN));
      /* callback for results of DTI connection has to be cmhSAT_OpChnCnct */
      simShrdPrm.sim_dti_chPrm->dti_cb = cmhSAT_OpChnCnct;
    }
  }
#endif /* DTI */
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : SAT                     |
|                                 ROUTINE : cmhSAT_OpChnChckBear    |
+-------------------------------------------------------------------+

  PURPOSE : This function is used to check whether the bearer parameters
            differ from the orignal settings by the SAT command.
            If so, the new parameters will overwrite the settings in 
            opchPrm and the opchPrmMdf will be set to TRUE.
            
*/

GLOBAL void cmhSAT_OpChnChckBear(void)
{
  UBYTE mt;

  TRACE_FUNCTION("cmhSAT_OpChnChckBear()");

    /*
   *-------------------------------------------------------------------
   * for CSD bearer parameters 
   *-------------------------------------------------------------------
   */
  if( satShrdPrm.chnTb.chnType EQ B_CSD  )
  { 
    T_CC_CALL_TBL * pCtbNtry;
    UBYTE cmdSpeed;
    UBYTE cmdName;
    UBYTE cmdCE;
    T_SAT_CSD_PRM *p = (T_SAT_CSD_PRM*)satShrdPrm.opchPrm;

    if (!psaCC_ctbIsValid (satShrdPrm.chnTb.chnRefId))
    {
      /* Avoid to dereference NULL */
      TRACE_ERROR ("Call table entry disappeared");
      return;
    }

    pCtbNtry = ccShrdPrm.ctb[satShrdPrm.chnTb.chnRefId];

    if( !p->def_bear_prm )
    {
      cmdSpeed = cmhCC_SelRate((T_ACI_BS_SPEED)p->csd_bear_prm.csd_speed);
      cmdName  = cmhCC_SelServ((T_ACI_CBST_NAM)p->csd_bear_prm.csd_name);
      cmdCE    = cmhCC_SelCE  ((T_ACI_CBST_CE)p->csd_bear_prm.csd_ce);

      if((cmdSpeed NEQ pCtbNtry->BC[0].rate) OR
         (cmdName  NEQ pCtbNtry->BC[0].bearer_serv) OR
         (cmdCE    NEQ pCtbNtry->BC[0].conn_elem ))
      {
        satShrdPrm.opchPrmMdf = TRUE;
      }
    }

    /* update bearer parms with current values */
    p->csd_bear_prm.csd_speed = (UBYTE)cmhCC_GetDataRate(&pCtbNtry->BC[0]);
    mt = pCtbNtry->BC[0].modem_type;

    switch( pCtbNtry->BC[0].rate )
    {
      case( MNCC_UR_0_3_KBIT  ): 
        if( mt EQ MNCC_MT_V21 ) p->csd_bear_prm.csd_speed = BS_SPEED_300_V21;
        break;
      case( MNCC_UR_1_2_KBIT  ):
        if      ( mt EQ MNCC_MT_V22 )  p->csd_bear_prm.csd_speed = BS_SPEED_1200_V22;
        else if ( mt EQ MNCC_MT_V23 )  p->csd_bear_prm.csd_speed = BS_SPEED_1200_75_V23;
        else if ( mt EQ MNCC_MT_NONE ) p->csd_bear_prm.csd_speed = BS_SPEED_1200_V110;
        break;
      case( MNCC_UR_2_4_KBIT  ):
        if      ( mt EQ MNCC_MT_V22_BIS )  p->csd_bear_prm.csd_speed = BS_SPEED_2400_V22bis;
        else if ( mt EQ MNCC_MT_V26_TER )  p->csd_bear_prm.csd_speed = BS_SPEED_2400_V26ter;
        else if ( mt EQ MNCC_MT_NONE )     p->csd_bear_prm.csd_speed = BS_SPEED_2400_V110;
        break;
      case( MNCC_UR_4_8_KBIT  ):
        if      ( mt EQ MNCC_MT_V32 )  p->csd_bear_prm.csd_speed = BS_SPEED_4800_V32;
        else if ( mt EQ MNCC_MT_NONE ) p->csd_bear_prm.csd_speed = BS_SPEED_4800_V110;
        break;
      case( MNCC_UR_9_6_KBIT  ):
        if      ( mt EQ MNCC_MT_V32 )  p->csd_bear_prm.csd_speed = BS_SPEED_9600_V32;
        else if ( mt EQ MNCC_MT_V34 )  p->csd_bear_prm.csd_speed = BS_SPEED_9600_V34;
        else if ( mt EQ MNCC_MT_NONE ) p->csd_bear_prm.csd_speed = BS_SPEED_9600_V110;
        break;
      case( MNCC_UR_14_4_KBIT ):
        if      ( mt EQ MNCC_MT_V34 )  p->csd_bear_prm.csd_speed = BS_SPEED_14400_V34;
        else if ( mt EQ MNCC_MT_NONE ) p->csd_bear_prm.csd_speed = BS_SPEED_14400_V110;
        break;
      default:
        break; /* leave it the way it is */
    }

    switch( pCtbNtry->BC[0].bearer_serv )
    {
      case( MNCC_BEARER_SERV_ASYNC ):
        p->csd_bear_prm.csd_name = CBST_NAM_Asynch; break;

      case( MNCC_BEARER_SERV_SYNC ):
        p->csd_bear_prm.csd_name = CBST_NAM_Synch; break;

      default: break; /* leave it the way it is */
    }

    switch( pCtbNtry->BC[0].conn_elem )
    {
      case( MNCC_CONN_ELEM_TRANS ): 
        p->csd_bear_prm.csd_ce = CBST_CE_Transparent; break;

      case( MNCC_CONN_ELEM_NON_TRANS ): 
        p->csd_bear_prm.csd_ce = CBST_CE_NonTransparent; break;

      default: break; /* leave it the way it is */
    }
  }
  /*
   *-------------------------------------------------------------------
   * for GPRS bearer parameters 
   *-------------------------------------------------------------------
   */
#ifdef GPRS
  if( satShrdPrm.chnTb.chnType EQ B_GPRS )
  {
   T_PS_qos       *curQOS;
    T_SAT_GPRS_PRM *p = (T_SAT_GPRS_PRM*)satShrdPrm.opchPrm;

    curQOS = cmhSM_getCurQOS( satShrdPrm.chnTb.chnRefId );

    if( !p->def_bear_prm )
    {
      if((curQOS->qos_r97.preced   NEQ p->gprs_bear_prm.gprs_prec)  OR
         (curQOS->qos_r97.delay    NEQ p->gprs_bear_prm.gprs_delay) OR
         (curQOS->qos_r97.relclass NEQ p->gprs_bear_prm.gprs_rely ) OR
         (curQOS->qos_r97.peak     NEQ p->gprs_bear_prm.gprs_peak ) OR
         (curQOS->qos_r97.mean     NEQ p->gprs_bear_prm.gprs_mean )    )
      {
        satShrdPrm.opchPrmMdf = TRUE;
      }
    }
    /* update bearer parms with current values */
    p->gprs_bear_prm.gprs_prec  = curQOS->qos_r97.preced;
    p->gprs_bear_prm.gprs_delay = curQOS->qos_r97.delay;
    p->gprs_bear_prm.gprs_rely  = curQOS->qos_r97.relclass;
    p->gprs_bear_prm.gprs_peak  = curQOS->qos_r97.peak;
    p->gprs_bear_prm.gprs_mean  = curQOS->qos_r97.mean;
  }
#endif  /* GPRS */
}
#ifdef DTI
/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : SAT                     |
|                                 ROUTINE : cmhSAT_OpChnSetPPP      |
+-------------------------------------------------------------------+

  PURPOSE : This function is used to set up PPP parameters.
            
*/

GLOBAL void cmhSAT_OpChnSetPPP(UBYTE chnType)
{

  CHAR log[PPP_LOGIN_NAME_LENGTH+1]; /* holds login name */
  CHAR pwd[PPP_LOGIN_NAME_LENGTH+1]; /* holds password */

  T_SAT_CSD_PRM *p = (T_SAT_CSD_PRM*)satShrdPrm.opchPrm;

  TRACE_FUNCTION("cmhSAT_OpChnSetPPP()");

  log[0] = 0; /* empty login name */
  pwd[0] = 0; /* empty user name */

  if( chnType EQ B_CSD )
  {
    /* set login name and password, if available */
    if(p->v_log AND p->log.c_text_str)
    {  
      strncpy(pwd,(const CHAR *)p->log.text_str,MINIMUM(p->log.c_text_str, PPP_LOGIN_NAME_LENGTH));
      log[MINIMUM(p->log.c_text_str, PPP_LOGIN_NAME_LENGTH)] ='\0';
    }

    if(p->v_pwd AND p->pwd.c_text_str)
    {
      strncpy(pwd,(const CHAR *)p->pwd.text_str,MINIMUM(p->pwd.c_text_str, PPP_LOGIN_NAME_LENGTH));
      pwd[MINIMUM(p->pwd.c_text_str, PPP_LOGIN_NAME_LENGTH)] ='\0';
    }
  }

  sAT_PercentPPP( (T_ACI_CMD_SRC)satShrdPrm.opchAcptSrc, A_PAP, log, pwd, USE_NO_PPP_FOR_AAA );
}
#endif /* DTI */

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : SAT                     |
|                                 ROUTINE : cmhSAT_OpChnStatEvnt    |
+-------------------------------------------------------------------+

  PURPOSE : This function is used to send a channel status event if
            enabled by SAT
            
*/

GLOBAL void cmhSAT_OpChnStatEvnt( void )
{

  TRACE_FUNCTION("cmhSAT_OpChnStatEvnt()");

  if( psaSAT_ChkEventList( EVENT_CHAN_STAT ) )
  {
    cmhSAT_EventDwn( EVENT_CHAN_STAT, -1, NEAR_END );
  }

}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : SAT                     |
|                                 ROUTINE : cmhSAT_OpChnGPRSPend    |
+-------------------------------------------------------------------+

  PURPOSE : This function is used to check for a pending GPRS open
            channel command.
            
*/
#ifdef GPRS
GLOBAL BOOL cmhSAT_OpChnGPRSPend( SHORT cid, UBYTE opchStat )
{
  if(  satShrdPrm.chnTb.chnUsdFlg AND
       satShrdPrm.chnTb.chnType EQ B_GPRS AND
      (opchStat EQ OPCH_NONE OR opchStat EQ satShrdPrm.opchStat)AND
      (cid EQ PDP_CONTEXT_CID_INVALID OR cid EQ satShrdPrm.chnTb.chnRefId))
     
    return( TRUE );
  
  else

    return( FALSE );
}
#endif  /* GPRS */

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : SAT                     |
|                                 ROUTINE : cmhSAT_OpChnUDPActivateGprs |
+-------------------------------------------------------------------+

  PURPOSE : This function is used to activate the UDP data chain and
            GPRS context.
            
*/
#if defined (GPRS) AND defined (DTI)
GLOBAL void cmhSAT_OpChnUDPActivateGprs( void )
{
  T_PDP_CONTEXT_INTERNAL *p_pdp_context_node = NULL;
  U8    cid_array[PDP_CONTEXT_CID_MAX + 1];
  T_ACI_SAT_TERM_RESP resp_data;      

  TRACE_FUNCTION("cmhSAT_OpChnUDPActivateGprs()");

  memset( &cid_array, PDP_CONTEXT_CID_INVALID, sizeof(cid_array) );

  cid_array[0] = (U8)satShrdPrm.chnTb.chnRefId;

  /* final check of context id and link status */
    /*if(!cmhSM_define_cid_list((T_ACI_CMD_SRC)satShrdPrm.opchAcptSrc, cid_array ) OR */
    if(cmhSM_make_active_cid_list((T_ACI_CMD_SRC)satShrdPrm.opchAcptSrc, cid_array ) OR
     !srcc_reserve_sources( SRCC_IP_SNDCP_LINK, satShrdPrm.chnTb.chnRefId))
  {
    psaSAT_InitTrmResp( &resp_data );
    resp_data.add_content = ADD_BIP_NO_CHAN_AVAIL;
    cmhSAT_OpChnFailed( RSLT_BEARIND_PERR, &resp_data );
  }

  srcc_new_count(SRCC_IP_SNDCP_LINK);

  /* create a SAT class E DTI ID if not present */
  if ( simShrdPrm.sat_class_e_dti_id EQ DTI_DTI_ID_NOTPRESENT )
  {
    simShrdPrm.sat_class_e_dti_id = dti_cntrl_new_dti(DTI_DTI_ID_NOTPRESENT);
    TRACE_EVENT_P1("sat_class_e_dti_id = %d", simShrdPrm.sat_class_e_dti_id);
  }

  /* get dti id for SNDCP/IP link */
  p_pdp_context_node = pdp_context_find_node_from_cid( cid_array[0] );
  if( p_pdp_context_node )
  {

    /*pdp_context[satShrdPrm.chnTb.chnRefId-1].link_id = */
    p_pdp_context_node->internal_data.link_id =    
    dti_conn_compose_link_id (0,0, simShrdPrm.sat_class_e_dti_id, DTI_TUPLE_NO_NOTPRESENT);
  }
  else
  {
    TRACE_FUNCTION ("cmhSAT_OpChnUDPActivateGprs()");
    TRACE_EVENT("Context Not Found");
  }

  /* send request to ACI WAP to activate WAP */
  smShrdPrm.owner  = satShrdPrm.opchAcptSrc;
  smEntStat.entOwn =  (T_ACI_CMD_SRC)satShrdPrm.opchAcptSrc;
  smEntStat.curCmd = AT_CMD_CGACT;

#if defined (FF_WAP) || defined (FF_SAT_E)
  psaTCPIP_Activate( satShrdPrm.opchAcptSrc, 
                     simShrdPrm.sat_class_e_dti_id,
                     0, TCPIP_ACT_OPTION_V4 | TCPIP_ACT_OPTION_UDP,
                     TCPIP_CONNECTION_TYPE_GPRS_WAP,
                     cmhSM_IP_activate_cb);
#endif /*WAP or SATE*/


}
#endif  /* GPRS */

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : SAT                     |
|                                 ROUTINE : cmhSAT_OpChnUDPConfGprs |
+-------------------------------------------------------------------+

  PURPOSE : This callback function will be called when the UDP stack
            is configured

*/

#if defined(GPRS)
GLOBAL void cmhSAT_OpChnUDPConfGprs(void)
{

  TRACE_FUNCTION("cmhSAT_OpChnUDPConfGprs()");
  /* 
   * send the SIM_BIP_CONFIG_REQ primitive providing details of the connection
   * to SIM 
   */
  psaSIM_Bip_Config_Req();
  
}
#endif  /* GPRS */

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : SAT                     |
|                                 ROUTINE : cmhSAT_OpChnUDPDeactGprs|
+-------------------------------------------------------------------+

  PURPOSE : This callback function will be called when the UDP stack
            is deactive

*/

#if defined(GPRS)
GLOBAL void cmhSAT_OpChnUDPDeactGprs(void)
{
  T_ACI_SAT_TERM_RESP resp_data;      /* holds terminal response parms */
  UBYTE cause;
  
  TRACE_FUNCTION("cmhSAT_OpChnUDPDeactGprs()");

  psaSAT_InitTrmResp( &resp_data );

  /*
   *-------------------------------------------------------------------
   * check if deactivation was intended 
   *-------------------------------------------------------------------
   */
  if( satShrdPrm.opchStat EQ OPCH_CLS_REQ )
  {
    /* nothing to do here anymore */
  }

  /*
   *-------------------------------------------------------------------
   * deactivation during an establish request 
   *-------------------------------------------------------------------
   */
  else if( satShrdPrm.opchStat EQ OPCH_EST_REQ )
  {
    if( satShrdPrm.gprsNtwCs )
    {
      resp_data.add_content = satShrdPrm.gprsNtwCs | 0x80;

      cause = RSLT_NTW_UNAB_PROC;
    }
    else
    {
      resp_data.add_content = ADD_NO_CAUSE;
      cause = RSLT_ME_UNAB_PROC;
    }

    cmhSAT_OpChnFailed( cause, &resp_data );
  }
}
#endif  /* GPRS */

/*
+-------------------------------------------------------------------+
| PROJECT : GPRS (8441)           MODULE  : CMH_SMF                 |
| STATE   : finished              ROUTINE : cmhSAT_OpChnGPRSStat    |
+-------------------------------------------------------------------+

  PURPOSE : This function will be called if the status of the requested
            GPRS context has changed.

*/
#if defined (GPRS) AND defined (DTI)
GLOBAL void cmhSAT_OpChnGPRSStat(T_SAT_GPRS_CB_STAT stat, UBYTE cause)
{
  T_ACI_SAT_TERM_RESP resp_data;      /* holds terminal response parms */
  T_SIM_SAT_CHN chnInf;

  TRACE_FUNCTION ("cmhSAT_OpChnGPRSStat()");

  if( cause ) satShrdPrm.gprsNtwCs = cause;

  /* in case of UDP, we will wait for the UDP status */
  if( satShrdPrm.chnTb.chnTPL EQ UDP AND
      stat NEQ SAT_GPRS_SUSPEND      AND
      stat NEQ SAT_GPRS_RESUME) 
      
      return;

  psaSAT_InitTrmResp( &resp_data );

  /*
   *-------------------------------------------------------------------
   * during a close channel request 
   *-------------------------------------------------------------------
   */
  if( satShrdPrm.opchStat EQ OPCH_CLS_REQ )
  {
     /* nothing to do here anymore */
  }

  /*
   *-------------------------------------------------------------------
   * during an open channel establish request 
   *-------------------------------------------------------------------
   */
  else if( satShrdPrm.opchStat EQ OPCH_EST_REQ )
  {
    switch ( stat )
    {
      case( SAT_GPRS_DEACT ):
      case( SAT_GPRS_ATT_FAILED ):
      case( SAT_GPRS_ACT_FAILED ):

        if( satShrdPrm.gprsNtwCs )
        {
          resp_data.add_content = satShrdPrm.gprsNtwCs | 0x80;

          cause = RSLT_NTW_UNAB_PROC;
        }
        else
        {
          resp_data.add_content = ADD_NO_CAUSE;
          cause = RSLT_ME_UNAB_PROC;
        }

        cmhSAT_OpChnFailed( cause, &resp_data );
        break;

      case( SAT_GPRS_SUSPEND ):

        if( psaSS_stbFindActSrv( NO_ENTRY ) NEQ NO_ENTRY )
        {
          TRACE_EVENT("SAT GPRS channel suspend, SS busy");
          resp_data.add_content = ADD_ME_SS_BUSY;
        }

        /* check for busy call condition */
        else if( psaSAT_ctbFindActCall() NEQ NO_ENTRY )
        {
          TRACE_EVENT("SAT GPRS channel suspend, busy on call");
          resp_data.add_content = ADD_ME_CALL_BUSY;
        }

        cmhSAT_OpChnFailed( RSLT_ME_UNAB_PROC, &resp_data );
        break;

      case( SAT_GPRS_ACT ):

        cmhSAT_OpChnSIMCnctReq( DTI_ENTITY_SNDCP );
        break;

      default:
        
        break;
    }
  }
  /*
   *-------------------------------------------------------------------
   * expected status change
   *-------------------------------------------------------------------
   */
  else
  {
    switch ( stat )
    {
      case( SAT_GPRS_DEACT ):
      case( SAT_GPRS_ATT_FAILED ):
      case( SAT_GPRS_ACT_FAILED ):

        resp_data.add_content = ADD_BIP_CHAN_CLOSD;
        cmhSAT_OpChnFailed( RSLT_BEARIND_PERR, &resp_data );
        break;

      case( SAT_GPRS_SUSPEND ):

        chnInf.addRes = ADD_NO_CAUSE;

        if( satShrdPrm.chnTb.lnkStat EQ SIM_LINK_OPEN OR
            satShrdPrm.chnTb.lnkStat EQ SIM_LINK_CNCT )
        {
          if( psaSS_stbFindActSrv( NO_ENTRY ) NEQ NO_ENTRY )
          {
            TRACE_EVENT("SAT GPRS channel suspend, SS busy");
            chnInf.addRes = ADD_ME_SS_BUSY;
          }

          /* check for busy call condition */
          else if( psaSAT_ctbFindActCall() NEQ NO_ENTRY )
          {
            TRACE_EVENT("SAT GPRS channel suspend, busy on call");
            chnInf.addRes = ADD_ME_CALL_BUSY;
          }

          /* setup channel info for PSA */
          chnInf.chnId   = CHANNEL_ID_1;
          chnInf.genRes  = RSLT_ME_UNAB_PROC;
          chnInf.bipConn = SIM_BIP_CHANNEL_SUSPENDED;
          chnInf.dtiConn = SIM_DTI_UNKNOWN; 

          psaSIM_SATBIPChn( chnInf, NULL );
        }
        break;

      case( SAT_GPRS_RESUME ):

        if( satShrdPrm.chnTb.lnkStat EQ SIM_LINK_OPEN OR
            satShrdPrm.chnTb.lnkStat EQ SIM_LINK_CNCT )
        {
          /* setup channel info for PSA */
          chnInf.chnId   = CHANNEL_ID_1;
          chnInf.genRes  = RSLT_PERF_SUCCESS;
          chnInf.addRes  = ADD_NO_CAUSE;
          chnInf.bipConn = SIM_BIP_CHANNEL_RESUMED;
          chnInf.dtiConn = SIM_DTI_UNKNOWN; 

          psaSIM_SATBIPChn( chnInf, NULL );
        }
        break;

      case( SAT_GPRS_ACT ):
      default:
        
        break;
    }
  }

}
#endif  /* GPRS */
/*
+-------------------------------------------------------------------+
| PROJECT : GPRS (8441)           MODULE  : CMH_SMF                 |
| STATE   : finished              ROUTINE : cmhSAT_cnvrtAPN2NetworkAdr|
+-------------------------------------------------------------------+

  PURPOSE : converts a APN with a given length into a null terminated
            domain name

*/

#ifdef GPRS
GLOBAL void cmhSAT_cnvrtAPN2NetworkAdr( UBYTE *apn, UBYTE c_apn, UBYTE *dom_name )
{
  UBYTE lblLen;
  UBYTE apnIdx = 0;
  UBYTE dnIdx = 0;

  TRACE_FUNCTION ("cmhSAT_cnvrtAPN2NetworkAdr()");

  while( apnIdx < c_apn )
  {
    lblLen = apn[apnIdx++];

    memcpy( &dom_name[dnIdx], &apn[apnIdx], lblLen );

    dnIdx  += lblLen;
    apnIdx += lblLen;

    dom_name[dnIdx++] = '.';
  }

  dom_name[dnIdx-1] = 0;
}
#endif  /* GPRS */

#endif /* FF_SAT_E */

#endif /* SIM_TOOLKIT */
/*==== EOF ========================================================*/
