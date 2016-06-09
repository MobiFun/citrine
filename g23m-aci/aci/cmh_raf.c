/* 
+----------------------------------------------------------------------------- 
|  Project :  GSM-PS (6147)
|  Modul   :  CMH_RAF
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
|             handler for RA.
+----------------------------------------------------------------------------- 
*/ 
#ifdef DTI
#ifndef CMH_RAF_C
#define CMH_RAF_C
#endif

#include "aci_all.h"
/*==== INCLUDES ===================================================*/
#include "aci_cmh.h"
#include "ati_cmd.h"
#include "aci_cmd.h"

#ifdef FAX_AND_DATA
#include "aci_fd.h"
#endif    /* of #ifdef FAX_AND_DATA */

#include "aci.h"
#include "psa.h"
#include "aci_io.h"
#include "psa_ra.h"
#include "psa_cc.h"
#include "cmh.h"
#include "cmh_ra.h"
#include "cmh_cc.h"

/*==== CONSTANTS ==================================================*/

/*==== TYPES ======================================================*/

/*==== EXPORT =====================================================*/

/*==== VARIABLES ==================================================*/

/*==== FUNCTIONS ==================================================*/



/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_RA                  |
|                                 ROUTINE : cmhRA_SelTrfProt        |
+-------------------------------------------------------------------+

  PURPOSE : select transfer protocol 

*/

GLOBAL UBYTE cmhRA_SelTrfProt( SHORT cId )
{
  switch( cmhCC_getcalltype(cId) )
  {
    case( TRANS_CALL ):
      return( RA_MODEL_TRANS );

    case( NON_TRANS_CALL ):
      return( RA_MODEL_RLP );
      
    case( FAX_CALL   ): 
      return( RA_MODEL_FAX   );
#ifdef CO_UDP_IP
    /* a WAP call should be a non-transparent call for RA */
    case( UDPIP_CALL ):
      return( RA_MODEL_RLP );
#endif
#ifdef FF_PPP
    case( PPP_CALL ):
      return( RA_MODEL_RLP );
#endif /* FF_PPP */
#if defined(FF_GPF_TCPIP)
    case TCPIP_CALL:
      return RA_MODEL_RLP ;
        
#endif /* FF_GPF_TCPIP */
    default: 
      TRACE_EVENT( "UNEXP TYPE OF CALL IN CTB" );
      return( NOT_PRESENT_8BIT );
  }
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_RA                  |
|                                 ROUTINE : cmhRA_SelChnRate        |
+-------------------------------------------------------------------+

  PURPOSE : select channel rate  

*/

GLOBAL UBYTE cmhRA_SelChnRate ( void )
{
  switch( ccShrdPrm.chType )
  {
    case( MNCC_CH_TCH_F ):
      switch( ccShrdPrm.chMod )
      {
      case( MNCC_CHM_DATA_14_4  ): return( TRA_FULLRATE_14400 ); 
      case( MNCC_CHM_DATA_9_6   ): return( TRA_FULLRATE_9600 );
      case( MNCC_CHM_DATA_4_8   ): return( TRA_FULLRATE_4800 );
      case( MNCC_CHM_DATA_2_4   ): return( TRA_FULLRATE_2400 );
      default:                TRACE_EVENT( "UNEXP CHN MODE IN CTB" );
                              return((UBYTE)-1); /* return type is UBYTE,so return type is typecasted with UBYTE */
      }

    case( MNCC_CH_TCH_H ):
      switch( ccShrdPrm.chMod )
      {
      case( MNCC_CHM_DATA_2_4   ): return( TRA_HALFRATE_2400 );
      case( MNCC_CHM_DATA_4_8   ): return( TRA_HALFRATE_4800 );
      case( MNCC_CHM_DATA_14_4  ):
      case( MNCC_CHM_DATA_9_6   ):
      default:                TRACE_EVENT( "UNEXP CHN MODE IN CTB" );
                              return((UBYTE)-1); /* return type is UBYTE,so return type is typecasted with UBYTE */
      }

    default:                  TRACE_EVENT( "UNEXP CHN TYPE IN CTB" );
                              return((UBYTE)-1); /* return type is UBYTE,so return type is typecasted with UBYTE */
  }
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_RA                  |
|                                 ROUTINE : cmhRA_SelChnRate        |
+-------------------------------------------------------------------+

  PURPOSE : select channel rate  

*/

GLOBAL UBYTE cmhRA_SelUsrRate( SHORT cId )
{
  if (ccShrdPrm.ctb[cId] EQ NULL)
    return NOT_PRESENT_8BIT;

  switch (psaCC_ctb(cId)->BC[psaCC_ctb(cId)->curBC].rate)
  {
    case( MNCC_UR_0_3_KBIT     ): return( URA_300     );
    case( MNCC_UR_1_2_KBIT     ): return( URA_1200    );
    case( MNCC_UR_2_4_KBIT     ): return( URA_2400    );
    case( MNCC_UR_4_8_KBIT     ): return( URA_4800    );
    case( MNCC_UR_9_6_KBIT     ): return( URA_9600    );
    case( MNCC_UR_1_2_KBIT_V23 ): return( URA_1200_75 );
    case( MNCC_UR_12_0_KBIT_TRANS): return( URA_14400   );
    case( 8               ): return( URA_14400   ); /* !!! change if SAP is updated */
    default:                 TRACE_EVENT( "UNEXP USER RATE IN CTB" );
                             return( NOT_PRESENT_8BIT );
  }
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_RA                  |
|                                 ROUTINE : cmhRA_SelDataBits       |
+-------------------------------------------------------------------+

  PURPOSE : select number of data bits

*/

GLOBAL UBYTE cmhRA_SelDataBits( SHORT cId )
{
  if (ccShrdPrm.ctb[cId] EQ NULL)
    return NOT_PRESENT_8BIT;

  switch (psaCC_ctb(cId)->BC[psaCC_ctb(cId)->curBC].data_bits)
  {
    case( MNCC_DATA_7_BIT ): return( 7 );
    case( MNCC_DATA_8_BIT ): return( 8 );
    default:            TRACE_EVENT( "UNEXP NR OF DATA BITS IN CTB" );
                        return( NOT_PRESENT_8BIT );
  }
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_RA                  |
|                                 ROUTINE : cmhRA_SelStopBits       |
+-------------------------------------------------------------------+

  PURPOSE : select number of stop bits

*/

GLOBAL UBYTE cmhRA_SelStopBits( SHORT cId ) 
{
  if (ccShrdPrm.ctb[cId] EQ NULL)
    return NOT_PRESENT_8BIT;

  switch (psaCC_ctb(cId)->BC[psaCC_ctb(cId)->curBC].stop_bits)
  {
    case( MNCC_STOP_1_BIT ): return( 1 );
    case( MNCC_STOP_2_BIT ): return( 2 );
    default:            TRACE_EVENT( "UNEXP NR OF STOP BITS IN CTB" );
                        return( NOT_PRESENT_8BIT );
  }
}

#ifdef FF_FAX

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_RA                  |
|                                 ROUTINE : cmhRA_SelBitOrder       |
+-------------------------------------------------------------------+

  PURPOSE : select bit order

*/

GLOBAL UBYTE cmhRA_SelBitOrder( T_ACI_CMD_SRC srcId ) 
{
  switch( fnd_cmhPrm[srcId].t30CmdPrm.FBOval )
  {
    case( FBO_VAL_DirCDirBD ): return( FBO_NRM_STATUS + FBO_NRM_DATA );
    case( FBO_VAL_RvrCDirBD ): return( FBO_NRM_STATUS + FBO_REV_DATA );
    case( FBO_VAL_DirCRvrBD ): return( FBO_REV_STATUS + FBO_NRM_DATA );
    case( FBO_VAL_RvrCRvrBD ): return( FBO_REV_STATUS + FBO_REV_DATA );
    default:            TRACE_EVENT( "UNEXP FBO VALUE" );
                        return( NOT_PRESENT_8BIT );
  }
}

#endif /* FF_FAX */
#endif /* DTI */
/*==== EOF ========================================================*/
