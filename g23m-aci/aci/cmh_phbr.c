/* 
+----------------------------------------------------------------------------- 
|  Project :  GSM-PS (6147)
|  Modul   :  CMH_PHBR
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
|  Purpose :  This module defines the functions which are responsible
|             for the responses of the protocol stack adapter for 
|             the phonebook management.
+----------------------------------------------------------------------------- 
*/ 

#ifndef CMH_PHBR_C
#define CMH_PHBR_C
#endif

#include "config.h"
#include "fixedconf.h"
#include "condat-features.h"
#include "aci_conf.h"

#include "aci_all.h"

/*==== INCLUDES ===================================================*/
#include "aci_cmh.h"
#include "ati_cmd.h"
#include "aci_cmd.h"
#include "aci.h"

#ifdef FAX_AND_DATA
#include "aci_fd.h"
#endif    /* of #ifdef FAX_AND_DATA */

#include "psa.h"
#include "psa_cc.h"
#include "psa_sim.h"

#include "cmh.h"
#include "phb.h"

#include "aci_lst.h"
#include "conc_sms.h"

/*==== CONSTANTS ==================================================*/


/*==== TYPES ======================================================*/


/*==== EXPORT =====================================================*/


/*==== VARIABLES ==================================================*/


/*==== FUNCTIONS ==================================================*/

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)    MODULE  : CMH_PHBR                     |
|                            ROUTINE : cmhPHB_StatIndication        |
+-------------------------------------------------------------------+

  PURPOSE :

*/

GLOBAL SHORT cmhPHB_StatIndication ( T_PHB_STAT psaStatus, SHORT cmeError, BOOL indicate )
{
  UBYTE idx;               /* used for counting */
  T_PHB_CMD_PRM * pPHBCmdPrm;
  
  TRACE_FUNCTION ("cmhPHB_StatIndication()");

  switch ( psaStatus )
  {
    case ( PHB_READY ): 
    {
      simShrdPrm.pb_stat = PB_STAT_Ready;
      
      if (indicate)
      {
        percentCSTAT_indication(STATE_MSG_PBOOK, ENTITY_STATUS_Ready);
      }
      for (idx = 0; idx < CMD_SRC_MAX; idx++)
      {
        pPHBCmdPrm = &cmhPrm[idx].phbCmdPrm;
        if (pPHBCmdPrm->curCmd EQ AT_CMD_CPBW)
        {
          pPHBCmdPrm->curCmd = AT_CMD_NONE;
          R_AT (RAT_OK, (T_ACI_CMD_SRC)idx) (AT_CMD_CPBW);
          break;
        }
      }   
      break;
    }
    case ( PHB_WRITE_FAIL ): 
    {
      simShrdPrm.pb_stat = PB_STAT_Ready;
   
      if (indicate)
      {
        percentCSTAT_indication(STATE_MSG_PBOOK, ENTITY_STATUS_Ready);
      }
      for (idx = 0; idx < CMD_SRC_MAX; idx++)
      {
        pPHBCmdPrm = &cmhPrm[idx].phbCmdPrm;
        if (pPHBCmdPrm->curCmd EQ AT_CMD_CPBW)
        {
          pPHBCmdPrm->curCmd = AT_CMD_NONE;
          R_AT( RAT_CME, (T_ACI_CMD_SRC)idx)( AT_CMD_CPBW, cmeError );
          break;
        }
      }
      break;
    }   
    case ( PHB_BUSY    ):
    {
#ifdef TI_PS_FFS_PHB
      simShrdPrm.pb_stat = PB_STAT_Busy;
      if (indicate)
      {
        percentCSTAT_indication(STATE_MSG_PBOOK, ENTITY_STATUS_NotReady);
      }
      break;
#endif     
    }
    /*lint -fallthrough */
    case ( PHB_UNKNOWN ): 
    {
      simShrdPrm.pb_stat = PB_STAT_Blocked;

      if (indicate)
      {
        percentCSTAT_indication(STATE_MSG_PBOOK, ENTITY_STATUS_NotReady);
      }
      break;
    }
    default:
    {
      TRACE_EVENT("FATAL ERROR in cmhPHB_StatIndication"); 
      simShrdPrm.pb_stat = PB_STAT_Blocked;
      return -1;
    }
  }

#if defined _CONC_TESTING_ AND defined TI_PS_FF_CONC_SMS
  if (simShrdPrm.pb_stat EQ PB_STAT_Ready)
  {
    concSMS_delAllIncompleteMsg();
  }
#endif

  for( idx = 0; idx < CMD_SRC_MAX; idx++ )
  {
     R_AT( RAT_PHB_STATUS, (T_ACI_CMD_SRC)idx )( simShrdPrm.pb_stat );
  }

  return 0;
}

/*==== EOF ========================================================*/
 
