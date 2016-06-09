/* 
+----------------------------------------------------------------------------- 
|  Project :  GSM-PS (6147)
|  Modul   :  CMH_L2RF
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
|  Purpose :  This module defines the functions used by the command
|             handler for L2R.
+----------------------------------------------------------------------------- 
*/ 
#ifdef DTI

#ifndef CMH_L2RF_C
#define CMH_L2RF_C
#endif

#include "aci_all.h"

/*==== INCLUDES ===================================================*/
#include "aci_cmh.h"
#include "ati_cmd.h"
#include "aci_cmd.h"
#include "dti.h"      /* functionality of the dti library */

#ifdef FAX_AND_DATA
#include "aci_fd.h"
#endif    /* of #ifdef FAX_AND_DATA */

#include "dti_conn_mng.h"

#include "aci.h"
#include "psa.h"
#include "psa_l2r.h"
#include "psa_cc.h"
#include "cmh.h"
#include "cmh_ra.h"
#include "cmh_l2r.h"

/*==== CONSTANTS ==================================================*/

/*==== TYPES ======================================================*/

/*==== EXPORT =====================================================*/

/*==== VARIABLES ==================================================*/

/*==== FUNCTIONS ==================================================*/



/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_L2R                 |
|                                 ROUTINE : cmhL2R_GetCompDir       |
+-------------------------------------------------------------------+

  PURPOSE : get data compression direction out of p0 value 

*/

GLOBAL SHORT cmhL2R_GetCompDir ( void )
{
  switch( l2rShrdPrm.set_prm_use.p0 )
  {
  case( L2R_COMP_DIR_NONE     ): return( DR_TYP_None );
  case( L2R_COMP_DIR_TRANSMIT ): return( DR_TYP_TxOnly );
  case( L2R_COMP_DIR_RECEIVE  ): return( DR_TYP_RxOnly );
  case( L2R_COMP_DIR_BOTH     ): return( DR_TYP_Both );
  default:                       TRACE_EVENT( "UNEXP BEARER SERVICE IN CTB" );
                                 return( DR_TYP_NotPresent );
  }
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_L2R                 |
|                                 ROUTINE : cmhL2R_SelCompDir       |
+-------------------------------------------------------------------+

  PURPOSE : select data compression direction  

*/

GLOBAL UBYTE cmhL2R_SelCompDir ( T_L2R_CMD_PRM * pCmdPrm )
{
  switch( pCmdPrm -> DSdir )
  {
  case( DS_DIR_Negotiated ): return( L2R_COMP_DIR_NONE     );
  case( DS_DIR_TxOnly     ): return( L2R_COMP_DIR_TRANSMIT );
  case( DS_DIR_RxOnly     ): return( L2R_COMP_DIR_RECEIVE  );
  case( DS_DIR_Both       ): return( L2R_COMP_DIR_BOTH     );
  default:                   TRACE_EVENT( "UNEXP BEARER SERVICE IN CTB" );
                             return((UBYTE)-1); /* return type is UBYTE,so return type is typecasted with UBYTE */
  }
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_L2R                 |
|                                 ROUTINE : cmhL2R_SelChnRate       |
+-------------------------------------------------------------------+

  PURPOSE : select channel rate  

*/

GLOBAL UBYTE cmhL2R_SelChnRate ( void )
{
  switch( ccShrdPrm.chType )
  {
    case( MNCC_CH_TCH_F ):
      switch( ccShrdPrm.chMod )
      {
      case( MNCC_CHM_DATA_14_4  ): return( L2R_FULLRATE_14400 ); 
      case( MNCC_CHM_DATA_9_6   ): return( L2R_FULLRATE_9600 );
      case( MNCC_CHM_DATA_4_8   ): return( L2R_FULLRATE_4800 );
      case( MNCC_CHM_DATA_2_4   ): 
      default:                TRACE_EVENT( "UNEXP CHN MODE IN CTB" );
                              return((UBYTE)-1); /* return type is UBYTE,so return type is typecasted with UBYTE */
      }

    case( MNCC_CH_TCH_H ):
      switch( ccShrdPrm.chMod )
      {
      case( MNCC_CHM_DATA_4_8   ): return( L2R_HALFRATE_4800 );
      case( MNCC_CHM_DATA_14_4  ):
      case( MNCC_CHM_DATA_9_6   ):
      case( MNCC_CHM_DATA_2_4   ): 
      default:                TRACE_EVENT( "UNEXP CHN MODE IN CTB" );
                              return((UBYTE)-1); /* return type is UBYTE,so return type is typecasted with UBYTE */
      }

    default:                  TRACE_EVENT( "UNEXP CHN TYPE IN CTB" );
                              return((UBYTE)1); /* return type is UBYTE,so return type is typecasted with UBYTE */
  }
}

#endif /* DTI */
/*==== EOF ========================================================*/
