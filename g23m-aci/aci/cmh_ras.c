/* 
+----------------------------------------------------------------------------- 
|  Project :  GSM-PS (6147)
|  Modul   :  CMH_RAS
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
|  Purpose :  This module provides the set functions related to the
|             protocol stack adapter for RA.
+----------------------------------------------------------------------------- 
*/ 

#ifndef CMH_RAS_C
#define CMH_RAS_C
#endif

#include "aci_all.h"
/*==== INCLUDES ===================================================*/
#include "aci_cmh.h"
#include "ati_cmd.h"
#include "aci_cmd.h"

#ifdef FAX_AND_DATA
#include "aci_fd.h"
#endif

#include "aci_io.h"

#include "psa.h"
#include "psa_ra.h"
#include "psa_cc.h"
#include "cmh.h"
#include "cmh_ra.h"

/*==== CONSTANTS ==================================================*/

/*==== EXPORT =====================================================*/

/*==== VARIABLES ==================================================*/

/*==== FUNCTIONS ==================================================*/

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_RAS                 |
|                                 ROUTINE : cmhRA_Activate          |
+-------------------------------------------------------------------+

  PURPOSE : set all nescessary parameters and activate RA

*/

GLOBAL T_ACI_RETURN cmhRA_Activate ( T_ACI_CMD_SRC srcId,
                                     T_ACI_AT_CMD cmdId,
                                     SHORT cId )
{
  T_RA_SET_PRM * pRASetPrm;  /* points to RA parameter set */
  UBYTE          prmTst;     /* for parameter testing */

  TRACE_FUNCTION ("cmhRA_Activate()");

/*
 *-------------------------------------------------------------------
 * check command source
 *-------------------------------------------------------------------
 */
  if(!cmh_IsVldCmdSrc (srcId))

    return( AT_FAIL );

  pRASetPrm = &raShrdPrm.set_prm[srcId];

/*
 *-------------------------------------------------------------------
 * check entity status
 *-------------------------------------------------------------------
 */
  if( raEntStat.curCmd NEQ AT_CMD_NONE )

    return( AT_BUSY );

/*
 *-------------------------------------------------------------------
 * fill in activate parameters
 *-------------------------------------------------------------------
 */
  pRASetPrm -> model      = prmTst = cmhRA_SelTrfProt( cId );
  if( prmTst EQ NOT_PRESENT_8BIT ) return( AT_FAIL );

  pRASetPrm -> tra_rate   = prmTst = (UBYTE)cmhRA_SelChnRate();
  if( prmTst EQ NOT_PRESENT_8BIT ) return( AT_FAIL );

  pRASetPrm -> user_rate  = prmTst = cmhRA_SelUsrRate( cId );
  if( prmTst EQ NOT_PRESENT_8BIT ) return( AT_FAIL );

  pRASetPrm -> ndb        = prmTst = cmhRA_SelDataBits( cId );
  if( prmTst EQ NOT_PRESENT_8BIT ) return( AT_FAIL );

  pRASetPrm -> nsb        = prmTst = cmhRA_SelStopBits( cId );
  if( prmTst EQ NOT_PRESENT_8BIT ) return( AT_FAIL );

#ifdef FF_FAX
  pRASetPrm -> bitord     = prmTst = cmhRA_SelBitOrder( srcId );
  if( prmTst EQ NOT_PRESENT_8BIT ) return( AT_FAIL );
#endif

  raShrdPrm.cId     = cId;
  raEntStat.curCmd  = cmdId;
  raShrdPrm.owner = (UBYTE)srcId;
  raEntStat.entOwn  = srcId;

  psaRA_Activate();

  return( AT_EXCT );

}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_RA                  |
|                                 ROUTINE : cmhRA_Deactivate        |
+-------------------------------------------------------------------+

  PURPOSE : set all nescessary parameters and deactivate RA

*/

GLOBAL T_ACI_RETURN cmhRA_Deactivate ( void )
{
  TRACE_FUNCTION ("cmhRA_Deactivate()");


 /* deactivate */
  psaRA_Deactivate();
  return( AT_EXCT );
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_RA                  |
|                                 ROUTINE : cmhRA_Modify            |
+-------------------------------------------------------------------+

  PURPOSE : set all nescessary parameters and modify RA

*/

GLOBAL T_ACI_RETURN cmhRA_Modify ( T_ACI_CMD_SRC srcId,
                                   SHORT         cId )
{
  T_RA_SET_PRM * pRASetPrm;  /* points to RA parameter set */
  UBYTE          prmTst;     /* for parameter testing */

  TRACE_FUNCTION ("cmhRA_Modify()");

  /* check command source */
  if(!cmh_IsVldCmdSrc (srcId))
  {
    return( AT_FAIL );
  }

  pRASetPrm = &raShrdPrm.set_prm[srcId];


  pRASetPrm -> tra_rate   = prmTst = (UBYTE)cmhRA_SelChnRate();
  if( prmTst EQ NOT_PRESENT_8BIT ) return( AT_FAIL );

  pRASetPrm -> user_rate  = prmTst = cmhRA_SelUsrRate( cId );
  if( prmTst EQ NOT_PRESENT_8BIT ) return( AT_FAIL );

  /* modify */
  raShrdPrm.owner = srcId;
  psaRA_Modify();
  return( AT_EXCT );
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)    MODULE  : CMH_RAR                      |
|                            ROUTINE : cmhRA_Escape                 |
+-------------------------------------------------------------------+

  PURPOSE : Escape from data mode

*/

GLOBAL T_ACI_RETURN cmhRA_Escape ( void )
{
  T_OWN calOwn;

  TRACE_FUNCTION ("cmhRA_Escape()");

/*
 *-------------------------------------------------------------------
 * return to command mode
 *-------------------------------------------------------------------
 */
/*  io_setIoMode( IO_MODE_CMD, rCI_IoMode ); */

  if (ccShrdPrm.ctb[raShrdPrm.cId] NEQ NULL)
    calOwn = psaCC_ctb(raShrdPrm.cId)->calOwn;
  else
    calOwn = (T_OWN)CMD_SRC_NONE;

  R_AT( RAT_OK, (T_ACI_CMD_SRC)calOwn )
    ( AT_CMD_NONE );

  return(AT_CMPL);
}

/*==== EOF ========================================================*/
