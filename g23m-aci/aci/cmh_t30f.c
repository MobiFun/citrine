/* 
+----------------------------------------------------------------------------- 
|  Project :  GSM-PS (6147)
|  Modul   :  CMH_T30F
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
#if defined (DTI) || defined (FF_FAX)

#ifndef CMH_T30F_C
#define CMH_T30F_C
#endif

#include "aci_all.h"
/*==== INCLUDES ===================================================*/
#include "aci_cmh.h"
#include "ati_cmd.h"
#include "aci_cmd.h"

#include "l4_tim.h"
#ifdef FAX_AND_DATA
#include "aci_fd.h"

#endif    /* of #ifdef FAX_AND_DATA */

#include "dti.h"
#include "dti_conn_mng.h"

#include "aci.h"
#include "psa.h"
#include "psa_t30.h"
#include "psa_cc.h"
#include "cmh.h"
#include "cmh_t30.h"

#include "cmh_ra.h"

/*==== CONSTANTS ==================================================*/
#define BIT_SET       (0x01)    /* bit is set */
#define BIT_CLR       (0x00)    /* bit is cleared */
#define BIT_NOT_PRES  (0xFF)    /* bit is not present */

/*==== TYPES ======================================================*/

/*==== EXPORT =====================================================*/

/*==== VARIABLES ==================================================*/

/*==== FUNCTIONS ==================================================*/

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_T30                 |
|                                 ROUTINE : cmhT30_Chn2BitRate      |
+-------------------------------------------------------------------+

  PURPOSE : convert channel rate into bit rate

*/

GLOBAL T_ACI_F_BR cmhT30_Chn2BitRate ( void )
{
  switch( ccShrdPrm.chMod )
  {
  case( MNCC_CHM_DATA_14_4   ): return( F_BR_14400 );
  case( MNCC_CHM_DATA_9_6    ): return( F_BR_9600 );
  case( MNCC_CHM_DATA_4_8    ): return( F_BR_4800 );
  case( MNCC_CHM_DATA_2_4    ): return( F_BR_2400 );
  default:                 TRACE_EVENT( "UNEXP CHN MODE IN CTB" );
                           return( F_BR_NotPresent );
  }
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_T30                 |
|                                 ROUTINE : cmhT30_SelChnRate       |
+-------------------------------------------------------------------+

  PURPOSE : select channel rate

*/

GLOBAL USHORT cmhT30_SelChnRate ( void )
{
  switch( ccShrdPrm.chMod )
  {
  case( MNCC_CHM_DATA_14_4 ):  return( 14400 );
  case( MNCC_CHM_DATA_9_6  ):  return( 9600 );
  case( MNCC_CHM_DATA_4_8  ):  return( 4800 );
  case( MNCC_CHM_DATA_2_4  ):  return( 2400 );
  default:                TRACE_EVENT( "UNEXP CHN MODE IN CTB" );
                          return( NOT_PRESENT_16BIT );
  }
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_T30                 |
|                                 ROUTINE : cmhT30_SelUsrRate       |
+-------------------------------------------------------------------+

  PURPOSE : select user rate

*/

GLOBAL USHORT cmhT30_SelUsrRate ( void )
{
  switch( t30NgtPrms.FCSbr )
  {
    case( F_BR_2400  ):   return( 2400  );
    case( F_BR_4800  ):   return( 4800  );
    case( F_BR_7200  ):   return( 7200  );
    case( F_BR_9600  ):   return( 9600  );
    case( F_BR_12000 ):   return( 12000 );
    case( F_BR_14400 ):   return( 14000 );
    default:              TRACE_EVENT( "UNEXP BIT RATE IN NGT PARMS" );
                          return( NOT_PRESENT_16BIT );
  }
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_T30                 |
|                                 ROUTINE : cmhT30_SelHlfRate       |
+-------------------------------------------------------------------+

  PURPOSE : select half rate

*/

GLOBAL UBYTE cmhT30_SelHlfRate ( void )
{
  switch( ccShrdPrm.chType )
  {
  case( MNCC_CH_TCH_F  ): return( FALSE );
  case( MNCC_CH_TCH_H  ): return( TRUE  );
  default:                TRACE_EVENT( "UNEXP CHN TYPE IN CTB" );
                          return((BYTE)-1);
  }
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_RA                  |
|                                 ROUTINE : cmhT30_SelBitOrder      |
+-------------------------------------------------------------------+

  PURPOSE : select bit order

*/

GLOBAL UBYTE cmhT30_SelBitOrder( T_ACI_CMD_SRC srcId )
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

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_T30                 |
|                                 ROUTINE : cmhT30_Chk4TCHAdpt      |
+-------------------------------------------------------------------+

  PURPOSE : check for TCH adaptation. Returns true if an adaptation
            of the TCH is expected.

*/

GLOBAL BOOL cmhT30_Chk4TCHAdpt ( void )
{
  TRACE_FUNCTION("cmhT30_Chk4TCHAdpt");

  TRACE_EVENT_P2( "CHN MD: %d, FCS BR: %d", ccShrdPrm.chMod, t30NgtPrms.FCSbr );

  switch( ccShrdPrm.chMod )
  {
  case( MNCC_CHM_DATA_14_4   ):

    switch( t30NgtPrms.FCSbr )
    {
      case( F_BR_2400  ):
      case( F_BR_4800  ):
      case( F_BR_7200  ):
      case( F_BR_9600  ):   return( TRUE );
      case( F_BR_12000 ):
      case( F_BR_14400 ):   return( FALSE );
      default:              TRACE_EVENT( "UNEXP BIT RATE IN NGT PARMS" );
                            return( FALSE );
    }
  case( MNCC_CHM_DATA_9_6    ):

    switch( t30NgtPrms.FCSbr )
    {
      case( F_BR_2400  ):
      case( F_BR_4800  ):
      case( F_BR_12000 ):
      case( F_BR_14400 ):   return( TRUE );
      case( F_BR_7200  ):
      case( F_BR_9600  ):   return( FALSE );
      default:              TRACE_EVENT( "UNEXP BIT RATE IN NGT PARMS" );
                            return( FALSE );
    }
  case( MNCC_CHM_DATA_4_8    ):

    switch( t30NgtPrms.FCSbr )
    {
      case( F_BR_2400  ):
      case( F_BR_7200  ):
      case( F_BR_9600  ):
      case( F_BR_12000 ):
      case( F_BR_14400 ):   return( TRUE );
      case( F_BR_4800  ):   return( FALSE );
      default:              TRACE_EVENT( "UNEXP BIT RATE IN NGT PARMS" );
                            return( FALSE );
    }
  case( MNCC_CHM_DATA_2_4    ):

    switch( t30NgtPrms.FCSbr )
    {
      case( F_BR_4800  ):
      case( F_BR_7200  ):
      case( F_BR_9600  ):
      case( F_BR_12000 ):
      case( F_BR_14400 ):   return( TRUE );
      case( F_BR_2400  ):   return( FALSE );
      default:              TRACE_EVENT( "UNEXP BIT RATE IN NGT PARMS" );
                            return( FALSE );
    }
  default:                  TRACE_EVENT( "UNEXP CHN MODE IN CTB" );
                            return( FALSE );
  }
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_T30                 |
|                                 ROUTINE : cmhT30_GetDataRate      |
+-------------------------------------------------------------------+

  PURPOSE : get negotiated data rate

*/

GLOBAL T_ACI_BS_SPEED cmhT30_GetDataRate ( void )
{
  /* do not return a data rate in FAX mode, due to adaptation to
     FAX host software */
  return( BS_SPEED_NotPresent );

#if(0)
  switch( t30NgtPrms.FCSbr )
  {
  case( F_BR_2400  ):   return( BS_SPEED_2400_V110  );
  case( F_BR_4800  ):   return( BS_SPEED_4800_V110  );
  case( F_BR_7200  ):   return( -1 );
  case( F_BR_9600  ):   return( BS_SPEED_9600_V110  );
  case( F_BR_12000 ):   return( BS_SPEED_12000_V110 );
  case( F_BR_14400 ):   return( BS_SPEED_14400_V110 );
  default:              TRACE_EVENT( "UNEXP BIT RATE IN NGT PARMS" );
                        return( F_BR_NotPresent );
  }
#endif
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_T30                 |
|                                 ROUTINE : cmhT30_GetPpr           |
+-------------------------------------------------------------------+

  PURPOSE : interpret post page response according TABLE23/T.32
            and return the post page response notation of T30.

*/

GLOBAL UBYTE cmhT30_GetPpr( T_ACI_FPS_PPR ppr )
{
  switch( ppr )
  {
    case( FPS_PPR_Mcf ): return( SGN_MCF );
    case( FPS_PPR_Rtn ): return( SGN_RTN );
    case( FPS_PPR_Rtp ): return( SGN_RTP );
    case( FPS_PPR_Pin ): return( SGN_PIN );
    case( FPS_PPR_Pip ): return( SGN_PIP );
  }

  TRACE_EVENT("INVALID SETTING FOR FAX CAPS: PPR");
  return( SGN_NOT_USED );
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_T30                 |
|                                 ROUTINE : cmhT30_GetResolution    |
+-------------------------------------------------------------------+

  PURPOSE : interpret resolution capabilities according TABLE2/T.32
            and return the resolution in CLASS 2.0 notation.

*/

GLOBAL T_ACI_F_VR cmhT30_GetResolution( void * p, T_T30_FRTP frmTyp )
{
  T_dis * pDCECaps = (T_dis *)p;  /* points to DCE capabilities */
  T_ACI_F_VR reso = F_VR_R8X3_85;            /* holds resolution capabilities */

/*
 *------------------------------------------------------------------
 *  for DCS frames
 *------------------------------------------------------------------
 */
  if( frmTyp EQ FRT_DCS )
  {
  /*
   *----------------------------------------------------------------
   *  TABLE 2/T.32 BIT 15 and BIT 44
   *----------------------------------------------------------------
   */
    if( pDCECaps -> R8_lines_pels EQ BIT_CLR ) /* BIT 15 EQ 0 */
    {
      if( pDCECaps -> resolution_type EQ BIT_CLR OR
          pDCECaps -> resolution_type EQ BIT_NOT_PRES ) /* BIT 44 -> metric based */
      {
        if( pDCECaps -> R16_lines_pels EQ BIT_SET ) /* BIT 43 EQ 1 */
          return( F_VR_R16X15_4 );
        else if( pDCECaps -> R8_lines EQ BIT_SET )  /* BIT 41 EQ 1 */
          return( F_VR_R8X15_4 );
        else
          return( F_VR_R8X3_85 );
      }
      else  /* BIT 44 EQ 1 -> inch based */
      {
        if( pDCECaps -> R16_lines_pels EQ BIT_SET ) /* BIT 43 */
          return( F_VR_200X400 );
        else
          return( F_VR_200X100 );
      }
    }
    else  /* BIT 15 EQ 1 */ 
    {
      if( pDCECaps -> resolution_type EQ BIT_CLR OR
        pDCECaps -> resolution_type EQ BIT_NOT_PRES ) /* BIT 44 -> metric based */
      {
        return( F_VR_R8X7_7 );
      }
      else  /* BIT 44 EQ 1 -> inch based */
      {
        if( pDCECaps -> r_300_pels EQ BIT_SET ) /* BIT 42 */
          return( F_VR_300X300 );
        else
          return( F_VR_200X200 );
      }
    }
  }

/*
 *------------------------------------------------------------------
 *  for DIS/DTC frames
 *------------------------------------------------------------------
 */
/*
 *------------------------------------------------------------------
 *  TABLE 2/T.32 Note 11, bottom line
 *------------------------------------------------------------------
 */
#ifdef _SIMULATION_
  if( pDCECaps -> R8_lines_pels     EQ BIT_SET AND
      pDCECaps -> R8_lines          EQ BIT_CLR AND
      pDCECaps -> r_300_pels        EQ BIT_CLR AND
      pDCECaps -> R16_lines_pels    EQ BIT_CLR AND
      pDCECaps -> i_res_pref        EQ BIT_CLR AND
      pDCECaps -> m_res_pref        EQ BIT_CLR AND
      pDCECaps -> min_scan_time_hr  EQ BIT_CLR     )
#else
  if( pDCECaps -> R8_lines_pels     EQ BIT_SET AND
      pDCECaps -> R8_lines          EQ BIT_NOT_PRES AND
      pDCECaps -> r_300_pels        EQ BIT_NOT_PRES AND
      pDCECaps -> R16_lines_pels    EQ BIT_NOT_PRES AND
      pDCECaps -> i_res_pref        EQ BIT_NOT_PRES AND
      pDCECaps -> m_res_pref        EQ BIT_NOT_PRES AND
      pDCECaps -> min_scan_time_hr  EQ BIT_NOT_PRES     )
#endif
  {
    reso |= F_VR_R8X7_7;
    return( reso );
  }

/*
 *------------------------------------------------------------------
 *  TABLE 2/T.32 BIT 41
 *------------------------------------------------------------------
 */
  if( pDCECaps -> R8_lines EQ BIT_SET )
    reso |= F_VR_R8X15_4;


/*
 *------------------------------------------------------------------
 *  TABLE 2/T.32 BIT 42
 *------------------------------------------------------------------
 */
  if( pDCECaps -> r_300_pels EQ BIT_SET )

    reso |= F_VR_300X300;

/*
 *------------------------------------------------------------------
 *  TABLE 2/T.32 Note 11 & 12
 *------------------------------------------------------------------
 */
  if( pDCECaps -> i_res_pref EQ BIT_SET )       /* BIT 44 */
  {
    if( pDCECaps -> R8_lines_pels EQ BIT_CLR )  /* BIT 15 */

      reso |= F_VR_200X100;

    if( pDCECaps -> R8_lines_pels EQ BIT_SET )  /* BIT 15 */

      reso |= F_VR_200X200;

    if( pDCECaps -> R16_lines_pels EQ BIT_SET ) /* BIT 43 */

      reso |= F_VR_200X400;
  }

  if( pDCECaps -> m_res_pref EQ BIT_SET )       /* BIT 45 */
  {
    if( pDCECaps -> R8_lines_pels EQ BIT_SET )  /* BIT 15 */

      reso |= F_VR_R8X7_7;

    if( pDCECaps -> R16_lines_pels EQ BIT_SET ) /* BIT 43 */

      reso |= F_VR_R16X15_4;
  }

  if( pDCECaps -> m_res_pref EQ BIT_CLR AND     /* BIT 44 */
      pDCECaps -> i_res_pref EQ BIT_CLR     )   /* BIT 45 */
  {
    TRACE_EVENT("INVALID SETTING FOR FAX CAPS:RESOLUTION");
  }

/*
 *------------------------------------------------------------------
 *  return resolution
 *------------------------------------------------------------------
 */
  return( F_VR_R8X7_7 );
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_T30                 |
|                                 ROUTINE : cmhT30_BitRate          |
+-------------------------------------------------------------------+

  PURPOSE : interpret bit rate capabilities according TABLE2/T.32
            and return the bit rate in CLASS 2.0 notation.

*/

GLOBAL T_ACI_F_BR cmhT30_GetBitRate( void * p, T_T30_FRTP frmTyp )
{
  T_dis * pDCECaps = (T_dis *)p;  /* points to DCE capabilities */

/*
 *------------------------------------------------------------------
 *  TABLE 2/T.32 BIT 11 - 14
 *------------------------------------------------------------------
 */
  switch( pDCECaps -> data_sig_rate )
  {
    case( 0 ):
      return( F_BR_2400 );

    case( 8 ):
      return( F_BR_9600 );

    case( 4 ):
      return( F_BR_4800 );

    case( 12 ):
      if( frmTyp EQ FRT_DCS )
        return( F_BR_7200 );
      else
        return( F_BR_9600 );

    case( 1 ):
      if( frmTyp EQ FRT_DCS )
        return( F_BR_14400 );
      break;

    case( 9 ):
      if( frmTyp EQ FRT_DCS )
        return( F_BR_9600 );
      break;

    case( 5 ):
      if( frmTyp EQ FRT_DCS ) 
        return( F_BR_12000 );
      break;

    case( 13 ):
      if( frmTyp EQ FRT_DCS )
        return( F_BR_7200 );
      else
        return( F_BR_14400 );

    default:
      break;
  }

  TRACE_EVENT("INVALID SETTING FOR FAX CAPS:BIT RATE");
  return( F_BR_NotPresent );
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_T30                 |
|                                 ROUTINE : cmhT30_GetPageWidth     |
+-------------------------------------------------------------------+

  PURPOSE : interpret page width capabilities according TABLE2/T.32
            and return the page width in CLASS 2.0 notation.

*/

GLOBAL T_ACI_F_WD cmhT30_GetPageWidth( void * p )
{
  T_dis * pDCECaps = (T_dis *)p;  /* points to DCE capabilities */

/*
 *------------------------------------------------------------------
 *  TABLE 2/T.32 BIT 17 - 18
 *------------------------------------------------------------------
 */
  switch( pDCECaps -> rec_width )
  {
    case( 0 ):
      return( F_WD_1728 );
    case( 2 ):
      return( F_WD_2048 );
    case( 1 ):
    case( 3 ):
      return( F_WD_2432 );
  }

  TRACE_EVENT("INVALID SETTING FOR FAX CAPS:PAGE WIDTH");
  return( F_WD_NotPresent );
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_T30                 |
|                                 ROUTINE : cmhT30_GetPageLength    |
+-------------------------------------------------------------------+

  PURPOSE : interpret page length capabilities according TABLE2/T.32
            and return the page length in CLASS 2.0 notation.

*/

GLOBAL T_ACI_F_LN cmhT30_GetPageLength( void * p )
{
  T_dis * pDCECaps = (T_dis *)p;  /* points to DCE capabilities */

/*
 *------------------------------------------------------------------
 *  TABLE 2/T.32 BIT 19 - 20
 *------------------------------------------------------------------
 */
  switch( pDCECaps -> max_rec_len )
  {
    case( 0 ):
      return( F_LN_A4 );
    case( 2 ):
      return( F_LN_B4 );
    case( 1 ):
      return( F_LN_Unlimited );
  }

  TRACE_EVENT("INVALID SETTING FOR FAX CAPS:PAGE LENGTH");
  return( F_LN_NotPresent );
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_T30                 |
|                                 ROUTINE : cmhT30_GetDataComp      |
+-------------------------------------------------------------------+

  PURPOSE : interpret data compression capabilities according
            TABLE2/T.32 and return the data compression in CLASS 2.0
            notation.

*/

GLOBAL T_ACI_F_DF cmhT30_GetDataComp( void * p )
{
  T_dis * pDCECaps = (T_dis *)p;  /* points to DCE capabilities */

/*
 *------------------------------------------------------------------
 *  TABLE 2/T.32 BIT 16, 26, 27 and 31
 *------------------------------------------------------------------
 */
  if( pDCECaps -> two_dim_coding EQ BIT_CLR )       /* BIT 16 */

    return( F_DF_1D_MdfHuff );

  if( pDCECaps -> two_dim_coding EQ BIT_SET )       /* BIT 16 */
  {
    if( pDCECaps -> uncomp_mode EQ BIT_CLR OR       /* BIT 26 */
       (pDCECaps -> uncomp_mode EQ BIT_NOT_PRES AND
        pDCECaps -> err_corr_mode EQ BIT_NOT_PRES AND
        pDCECaps -> err_corr_mode EQ BIT_NOT_PRES AND
        pDCECaps -> t6_coding     EQ BIT_NOT_PRES))

      return( F_DF_2D_MdfRd_T4 );

    if( pDCECaps -> uncomp_mode EQ BIT_SET )        /* BIT 26 */

      return( F_DF_2D_Uncomp );

    if( pDCECaps -> err_corr_mode EQ BIT_SET AND    /* BIT 27 */
        pDCECaps -> t6_coding     EQ BIT_SET     )  /* BIT 31 */

      return( F_DF_2D_MdfRd_T6 );
  }

  TRACE_EVENT("INVALID SETTING FOR FAX CAPS:DATA COMPRESSION");
  return( F_DF_NotPresent );
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_T30                 |
|                                 ROUTINE : cmhT30_GetErrCorr       |
+-------------------------------------------------------------------+

  PURPOSE : interpret error correction capabilities according
            TABLE2/T.32 and return the error correction in CLASS 2.0
            notation.

*/

GLOBAL T_ACI_F_EC cmhT30_GetErrCorr( void * p )
{
  T_dis * pDCECaps = (T_dis *)p;  /* points to DCE capabilities */

/*
 *------------------------------------------------------------------
 *  TABLE 2/T.32 BIT 27 and 67
 *------------------------------------------------------------------
 */
  if( pDCECaps -> err_corr_mode EQ BIT_CLR )     /* BIT 27 */

    return( F_EC_DisableECM );

  if( pDCECaps -> err_corr_mode EQ BIT_SET )     /* BIT 27 */
  {
    if( pDCECaps -> duplex EQ BIT_NOT_PRES  )    /* BIT 67 */

      return( F_EC_EnableECM );

    if( pDCECaps -> duplex EQ BIT_CLR )          /* BIT 67 */

      return( F_EC_EnableHalfDup );

    if( pDCECaps -> duplex EQ BIT_SET )          /* BIT 67 */

      return( F_EC_EnableFullDup );
  }

  return( F_EC_DisableECM );
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_T30                 |
|                                 ROUTINE : cmhT30_GetFileTrnsfr    |
+-------------------------------------------------------------------+

  PURPOSE : interpret file transfer mode according TABLE2/T.32
            and return the file transfer mode in CLASS 2.0 notation.

*/

GLOBAL T_ACI_F_BF cmhT30_GetFileTrnsfr( void * p )
{
  T_dis * pDCECaps = (T_dis *)p;  /* points to DCE capabilities */
  T_ACI_F_BF ftm = F_BF_DisableFileTrnsf;             /* holds file trans. capabilities */

/*
 *------------------------------------------------------------------
 *  TABLE 2/T.32 BIT 53
 *------------------------------------------------------------------
 */
  if( pDCECaps -> bft EQ BIT_CLR  )

    ftm += F_BF_DisableFileTrnsf;

/*
 *------------------------------------------------------------------
 *  TABLE 2/T.32 BIT 27 and 53, 54, 55, 57, 60, 62, 65
 *------------------------------------------------------------------
 */
  if( pDCECaps -> err_corr_mode EQ BIT_SET  ) /* BIT 27 */
  {
    if( pDCECaps -> bft EQ BIT_SET  )         /* BIT 53 */

      ftm += F_BF_EnableBFT;

    if( pDCECaps -> dtm EQ BIT_SET  )         /* BIT 54 */

      ftm += F_BF_DocuTrnsfMode;

    if( pDCECaps -> edi EQ BIT_SET  )         /* BIT 55 */

      ftm += F_BF_EdifactMode;

    if( pDCECaps -> btm EQ BIT_SET  )         /* BIT 57 */

      ftm += F_BF_BasicTrnsfMode;

    if( pDCECaps -> char_mode EQ BIT_SET  )   /* BIT 60 */

      ftm += F_BF_CharMode;

    if( pDCECaps -> mixed_mode EQ BIT_SET  )  /* BIT 62 */

      ftm += F_BF_MixMode;

    if( pDCECaps -> proc_mode_26 EQ BIT_SET  )/* BIT 65 */

      ftm += F_BF_ProcMode;
  }
/*
 *------------------------------------------------------------------
 *  return resolution
 *------------------------------------------------------------------
 */
  return( ftm );
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_T30                 |
|                                 ROUTINE : cmhT30_GetScanTime      |
+-------------------------------------------------------------------+

  PURPOSE : interpret scan time capabilities according TABLE2/T.32
            and return the scan time in CLASS 2.0 notation.

*/

GLOBAL T_ACI_F_ST cmhT30_GetScanTime( void * p )
{
  T_dis * pDCECaps = (T_dis *)p;  /* points to DCE capabilities */

/*
 *------------------------------------------------------------------
 *  TABLE 2/T.32 BIT 21 - 23
 *------------------------------------------------------------------
 */
  switch( pDCECaps -> min_scan_time )
  {
    case( 0 ):
      return( F_ST_20_20 );
    case( 4 ):
      return( F_ST_5_5 );
    case( 2 ):
      return( F_ST_10_10 );
    case( 6 ):
      return( F_ST_20_10 );
    case( 1 ):
      return( F_ST_40_40 );
    case( 5 ):
      return( F_ST_40_20 );
    case( 3 ):
      return( F_ST_10_5 );
    case( 7 ):
      return( F_ST_0_0 );
  }

  TRACE_EVENT("INVALID SETTING FOR FAX CAPS:SCAN TIME");
  return( F_ST_NotPresent );
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_T30                 |
|                                 ROUTINE : cmhT30_GetJPEG          |
+-------------------------------------------------------------------+

  PURPOSE : interpret JPEG mode according TABLE2/T.32
            and return the JPEG mode in CLASS 2.0 notation.

*/

GLOBAL T_ACI_F_JP cmhT30_GetJPEG( void * p )
{
  T_dis * pDCECaps = (T_dis *)p;  /* points to DCE capabilities */
  T_ACI_F_JP jpeg = F_JP_DisableJPEG;            /* holds JPEG capabilities */

/*
 *------------------------------------------------------------------
 *  TABLE 2/T.32 BIT 68
 *------------------------------------------------------------------
 */
  if( pDCECaps -> jpeg EQ BIT_CLR  )

    jpeg += F_JP_DisableJPEG;

/*
 *------------------------------------------------------------------
 *  TABLE 2/T.32 BIT 68 and 69, 70, 71, 73, 74, 75
 *------------------------------------------------------------------
 */
  if( pDCECaps -> jpeg EQ BIT_SET  )                /* BIT 68 */
  {
    jpeg += F_JP_EnableJPEG;

    if( pDCECaps -> full_colour EQ BIT_SET )        /* BIT 69 */

      jpeg += F_JP_FullColor;

    if( pDCECaps -> huffman_tables EQ BIT_SET )     /* BIT 70 */

      jpeg += F_JP_EnablePrefHuff;

    if( pDCECaps -> r_12_bits_pel_comp EQ BIT_SET ) /* BIT 71 */

      jpeg += F_JP_12BitsPelComp;

    if( pDCECaps -> no_subsamp EQ BIT_SET  )        /* BIT 73 */

      jpeg += F_JP_NoSubsmpl;

    if( pDCECaps -> cust_illum EQ BIT_SET  )        /* BIT 74 */

      jpeg += F_JP_CustIllum;

    if( pDCECaps -> cust_gamut EQ BIT_SET  )        /* BIT 75 */

      jpeg += F_JP_CustGamutRange;
  }

/*
 *------------------------------------------------------------------
 *  return resolution
 *------------------------------------------------------------------
 */
  return( jpeg );
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_T30                 |
|                                 ROUTINE : cmhT30_RstNgtPrms       |
+-------------------------------------------------------------------+

  PURPOSE : reset negotiated parameters.

*/

GLOBAL void cmhT30_RstNgtPrms( void )
{
  t30NgtPrms.FCSvr = F_VR_NotPresent;
  t30NgtPrms.FCSbr = F_BR_NotPresent;
  t30NgtPrms.FCSwd = F_WD_NotPresent;
  t30NgtPrms.FCSln = F_LN_NotPresent;
  t30NgtPrms.FCSdf = F_DF_NotPresent;
  t30NgtPrms.FCSec = F_EC_NotPresent;
  t30NgtPrms.FCSbf = F_BF_NotPresent;
  t30NgtPrms.FCSst = F_ST_NotPresent;
  t30NgtPrms.FCSjp = F_JP_NotPresent;
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_T30                 |
|                                 ROUTINE : cmhT30_InitFAXPrms      |
+-------------------------------------------------------------------+

  PURPOSE : intialize FAX parameter for the passed command id

*/

GLOBAL void cmhT30_InitFAXPrms ( T_ACI_CMD_SRC srcId )
{

  T_T30_CMD_PRM * pT30CmdPrm; /* points to T30 command parameters */

  pT30CmdPrm = &fnd_cmhPrm[srcId].t30CmdPrm;


  pT30CmdPrm -> FCLASSclass = FCLASS_CLASS_Data;
  pT30CmdPrm -> FCRval      = FCR_VAL_RcvCap;
  pT30CmdPrm -> FCCvr       = F_VR_R8X3_85;
  pT30CmdPrm -> FCCbr       = F_BR_9600;
  pT30CmdPrm -> FCCwd       = F_WD_1728;
  pT30CmdPrm -> FCCln       = F_LN_A4;
  pT30CmdPrm -> FCCdf       = F_DF_1D_MdfHuff;
  pT30CmdPrm -> FCCec       = F_EC_DisableECM;
  pT30CmdPrm -> FCCbf       = F_BF_DisableFileTrnsf;
  pT30CmdPrm -> FCCst       = F_ST_0_0;
  pT30CmdPrm -> FCCjp       = F_JP_DisableJPEG;
  pT30CmdPrm -> FISvr       = F_VR_R8X3_85;
  pT30CmdPrm -> FISbr       = F_BR_9600;
  pT30CmdPrm -> FISwd       = F_WD_1728;
  pT30CmdPrm -> FISln       = F_LN_A4;
  pT30CmdPrm -> FISdf       = F_DF_1D_MdfHuff;
  pT30CmdPrm -> FISec       = F_EC_DisableECM;
  pT30CmdPrm -> FISbf       = F_BF_DisableFileTrnsf;
  pT30CmdPrm -> FISst       = F_ST_0_0;
  pT30CmdPrm -> FISjp       = F_JP_DisableJPEG;
  pT30CmdPrm -> FLIstr[0]   = 0x0;
  pT30CmdPrm -> FPIstr[0]   = 0x0;
  pT30CmdPrm -> FLPval      = FLP_VAL_NoPollDoc;
  pT30CmdPrm -> FAPsub      = FAP_VAL_Disabled;
  pT30CmdPrm -> FAPsep      = FAP_VAL_Disabled;
  pT30CmdPrm -> FAPpwd      = FAP_VAL_Disabled;
  pT30CmdPrm -> FSAsub[0]   = 0x0;
  pT30CmdPrm -> FPAsep[0]   = 0x0;
  pT30CmdPrm -> FPWpwd[0]   = 0x0;
  pT30CmdPrm -> FNSlen      = 0;
  pT30CmdPrm -> FCQrq       = FCQ_RQ_CQCEnabled;
  pT30CmdPrm -> FMSbr       = F_BR_2400;
  pT30CmdPrm -> FPSppr      = FPS_PPR_Mcf;
  pT30CmdPrm -> FSPval      = FSP_VAL_PollDisabled;
  pT30CmdPrm -> FIEval      = FIE_VAL_IgnorePRI;
  pT30CmdPrm -> FITact      = FIT_ACT_OnHookRst;
  pT30CmdPrm -> FITtime     = 0;
  pT30CmdPrm -> FBOval      = FBO_VAL_DirCDirBD;

}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_T30                 |
|                                 ROUTINE : cmhT30_NgtDCEPrms       |
+-------------------------------------------------------------------+

  PURPOSE : negotiate DCE parameters.

*/

GLOBAL void cmhT30_NgtDCEPrms( T_ACI_CMD_SRC srcId )
{
  T_dis * pDCECaps;            /* points to DCE capabilities */
  T_T30_CMD_PRM * pT30CmdPrm;  /* points to T30 command parameters */
  SHORT bitPos = 0;            /* holds bit position */
  BOOL lclMtrcFlg, lclInchFlg; /* flags found local metric and
                                  inch resolution */
  BOOL lclCapFlg;              /* flags found local capability */
  BOOL rmtMtrcFlg, rmtInchFlg; /* flags found remote metric and
                                  inch resolution */
  BOOL rmtCapFlg;              /* flags found remote capability */
  UBYTE lclMtrc, rmtMtrc;      /* holds local and remote metric
                                  resolutions */
  UBYTE lclInch, rmtInch;      /* holds local and remote inch
                                  resolutions */
  UBYTE lclCap, rmtCap;        /* holds local and remote capability */
  UBYTE vrMtrc;                /* holds common metric resolution */
  UBYTE vrInch;                /* holds common inch resolution */
  T_ACI_F_BR TCHbr;            /* holds bit rate of TCH */

  pT30CmdPrm = &fnd_cmhPrm[srcId].t30CmdPrm;

  if( t30ShrdPrm.hdlc_rcv.v_dis EQ TRUE )
    pDCECaps = (T_dis *)&t30ShrdPrm.hdlc_rcv.dis;

  else if( t30ShrdPrm.hdlc_rcv.v_dtc EQ TRUE )
    pDCECaps = (T_dis *)&t30ShrdPrm.hdlc_rcv.dtc;

  else
  {
    TRACE_EVENT("NO CAPABILITES AVAILABLE FOR NEGOTIATION");
    return;
  }

/*
 *------------------------------------------------------------------
 *  negotiate resolution capabilities
 *------------------------------------------------------------------
 */
  lclMtrcFlg = 0;
  lclInchFlg = 0;
  rmtMtrcFlg = 0;
  rmtInchFlg = 0;
  lclMtrc = (UBYTE)(pT30CmdPrm -> FISvr & 0x0F);
  rmtMtrc = (UBYTE)(cmhT30_GetResolution(pDCECaps,FRT_DIS)
                    & 0x0F);
  lclInch = (UBYTE)((pT30CmdPrm -> FISvr & 0xF0)>>4);
  rmtInch = (UBYTE)((cmhT30_GetResolution(pDCECaps,FRT_DIS)
                     & 0xF0)>>4);
  vrMtrc = 0;
  vrInch = 0;

  for( bitPos = 3; bitPos >= 0; bitPos-- )
  {
    if( !lclMtrcFlg ) lclMtrcFlg = ((lclMtrc>>bitPos)&0x01);
    if( !rmtMtrcFlg ) rmtMtrcFlg = ((rmtMtrc>>bitPos)&0x01);
    if( !lclInchFlg ) lclInchFlg = ((lclInch>>bitPos)&0x01);
    if( !rmtInchFlg ) rmtInchFlg = ((rmtInch>>bitPos)&0x01);

    if( lclMtrcFlg AND rmtMtrcFlg AND !vrMtrc )
      vrMtrc = 0x01<<bitPos;

    if( lclInchFlg AND rmtInchFlg AND !vrInch )
      vrInch = 0x01<<bitPos;
  }

  if( vrMtrc AND vrInch)
  {
    t30NgtPrms.FCSvr = (T_ACI_F_VR)((vrMtrc <= vrInch)?vrMtrc:vrInch);
  }
  else if( vrMtrc ) 
  {
    t30NgtPrms.FCSvr = (T_ACI_F_VR)vrMtrc;
  }
  else 
  {
    t30NgtPrms.FCSvr = (T_ACI_F_VR)vrInch;
  }

/*
 *------------------------------------------------------------------
 *  negotiate bit rate capabilities
 *------------------------------------------------------------------
 */
  t30NgtPrms.FCSbr = cmhT30_Chn2BitRate();
  t30NgtPrms.FCSbr = MINIMUM(pT30CmdPrm->FISbr, t30NgtPrms.FCSbr);
  TCHbr = cmhT30_GetBitRate(pDCECaps, FRT_DIS);
  t30NgtPrms.FCSbr = MINIMUM(t30NgtPrms.FCSbr, TCHbr);

/*
 *------------------------------------------------------------------
 *  negotiate page width capabilities
 *------------------------------------------------------------------
 */
  t30NgtPrms.FCSwd = cmhT30_GetPageWidth(pDCECaps);
  t30NgtPrms.FCSwd = MINIMUM(pT30CmdPrm->FISwd, t30NgtPrms.FCSwd);
/*
 *------------------------------------------------------------------
 *  negotiate page length capabilities
 *------------------------------------------------------------------
 */
  t30NgtPrms.FCSln = cmhT30_GetPageLength(pDCECaps);
  t30NgtPrms.FCSln = MINIMUM(pT30CmdPrm->FISln, t30NgtPrms.FCSln);
/*
 *------------------------------------------------------------------
 *  negotiate data compression capabilities
 *------------------------------------------------------------------
 */
  t30NgtPrms.FCSdf = cmhT30_GetDataComp(pDCECaps);
  t30NgtPrms.FCSdf = MINIMUM(pT30CmdPrm->FISdf, t30NgtPrms.FCSdf);
/*
 *------------------------------------------------------------------
 *  negotiate error correction capabilities
 *------------------------------------------------------------------
 */
  t30NgtPrms.FCSec = cmhT30_GetErrCorr(pDCECaps);
  t30NgtPrms.FCSec = MINIMUM(pT30CmdPrm -> FISec, t30NgtPrms.FCSec);
/*
 *------------------------------------------------------------------
 *  negotiate file transfer capabilities
 *------------------------------------------------------------------
 */
  lclCapFlg = 0;
  rmtCapFlg = 0;
  lclCap = (UBYTE)pT30CmdPrm -> FISbf;
  rmtCap = (UBYTE)cmhT30_GetFileTrnsfr(pDCECaps);
  t30NgtPrms.FCSbf = F_BF_DisableFileTrnsf;

  for( bitPos = 7; bitPos >= 0; bitPos-- )
  {
    if( !lclCapFlg ) lclCapFlg = ((lclCap>>bitPos)&0x01);
    if( !rmtCapFlg ) rmtCapFlg = ((rmtCap>>bitPos)&0x01);

    if( lclCapFlg AND rmtCapFlg AND !t30NgtPrms.FCSbf )
    {
      t30NgtPrms.FCSbf = (T_ACI_F_BF)(0x01<<bitPos);
      break;
    }
  }

/*
 *------------------------------------------------------------------
 *  negotiate scan time capabilities
 *------------------------------------------------------------------
 */
  t30NgtPrms.FCSst = cmhT30_GetScanTime(pDCECaps);
  t30NgtPrms.FCSst = MAXIMUM(pT30CmdPrm->FISst, t30NgtPrms.FCSst);
/*
 *------------------------------------------------------------------
 *  negotiate JPEG capabilities
 *------------------------------------------------------------------
 */
  lclCapFlg = 0;
  rmtCapFlg = 0;
  lclCap = (UBYTE)pT30CmdPrm -> FISjp;
  rmtCap = (UBYTE)cmhT30_GetJPEG(pDCECaps);
  t30NgtPrms.FCSjp = F_JP_DisableJPEG;

  for( bitPos = 7; bitPos >= 0; bitPos-- )
  {
    if( !lclCapFlg ) lclCapFlg = ((lclCap>>bitPos)&0x01);
    if( !rmtCapFlg ) rmtCapFlg = ((rmtCap>>bitPos)&0x01);

    if( lclCapFlg AND rmtCapFlg AND !t30NgtPrms.FCSjp )
    {
      t30NgtPrms.FCSjp = (T_ACI_F_JP)(0x01<<bitPos);
      break;
    }
  }
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_T30                 |
|                                 ROUTINE : cmhT30_BuildSndFrm      |
+-------------------------------------------------------------------+

  PURPOSE : build send frame according the passed frame type with the
            settings of the passed owner.

*/

GLOBAL void cmhT30_BuildSndFrm( T_ACI_CMD_SRC srcId,
                                T_T30_FRTP frmTyp )
{
  T_dis * pFrm;                 /* points to frame */
  T_T30_CMD_PRM * pT30CmdPrm;   /* points to T30 command parameters */
  T_ACI_F_VR vr;                /* holds selected resolution */
  T_ACI_F_BR br;                /* holds selected bit rate */
  T_ACI_F_WD wd;                /* holds selected page width */
  T_ACI_F_LN ln;                /* holds selected page length */
  T_ACI_F_DF df;                /* holds selected data compression */
  T_ACI_F_EC ec;                /* holds selected error correction */
  T_ACI_F_BF bf;                /* holds selected file transfer mode */
  T_ACI_F_ST st;                /* holds selected scan time */
  T_ACI_F_JP jp;                /* holds selected JPEG mode */

  pT30CmdPrm = &fnd_cmhPrm[srcId].t30CmdPrm;

/*
 *------------------------------------------------------------------
 *  determine frame type to build
 *------------------------------------------------------------------
 */
  switch( frmTyp )
  {
    case( FRT_DIS ):

      t30ShrdPrm.hdlc_snd.v_dis = TRUE;
      pFrm = (T_dis *)&t30ShrdPrm.hdlc_snd.dis;
      vr = pT30CmdPrm -> FISvr;
      br = pT30CmdPrm -> FISbr;
      wd = pT30CmdPrm -> FISwd;
      ln = pT30CmdPrm -> FISln;
      df = pT30CmdPrm -> FISdf;
      ec = pT30CmdPrm -> FISec;
      bf = pT30CmdPrm -> FISbf;
      st = pT30CmdPrm -> FISst;
      jp = pT30CmdPrm -> FISjp;
      break;

    case( FRT_DTC ):

      t30ShrdPrm.hdlc_snd.v_dtc = TRUE;
      pFrm = (T_dis *)&t30ShrdPrm.hdlc_snd.dtc;
      vr = pT30CmdPrm -> FISvr;
      br = pT30CmdPrm -> FISbr;
      wd = pT30CmdPrm -> FISwd;
      ln = pT30CmdPrm -> FISln;
      df = pT30CmdPrm -> FISdf;
      ec = pT30CmdPrm -> FISec;
      bf = pT30CmdPrm -> FISbf;
      st = pT30CmdPrm -> FISst;
      jp = pT30CmdPrm -> FISjp;
      break;

    case( FRT_DCS ):

      t30ShrdPrm.hdlc_snd.v_dcs = TRUE;
      pFrm = (T_dis *)&t30ShrdPrm.hdlc_snd.dcs;
      vr = t30NgtPrms.FCSvr;
      br = t30NgtPrms.FCSbr;
      wd = t30NgtPrms.FCSwd;
      ln = t30NgtPrms.FCSln;
      df = t30NgtPrms.FCSdf;
      ec = t30NgtPrms.FCSec;
      bf = t30NgtPrms.FCSbf;
      st = t30NgtPrms.FCSst;
      jp = t30NgtPrms.FCSjp;
      break;

    default:
      TRACE_ERROR("WRONG FRAME TYPE in cmhT30_BuildSndFrm()");
      return;
  }

/*
 *------------------------------------------------------------------
 *  build frame
 *------------------------------------------------------------------
 */
  /*
   *----------------------------------------------------------------
   *  default settings
   *----------------------------------------------------------------
   */
  pFrm -> v8                          = 0;  /* BIT 6 */
  pFrm -> n_byte                      = 0;  /* BIT 7 */
  pFrm -> frame_size                  = 0;  /* BIT 28 */
  pFrm -> min_scan_time_hr            = 0;  /* BIT 46 */
  pFrm -> ready_tx_doc                = 0;  /* BIT 51 */
  pFrm -> ready_tx_mixed              = 0;  /* BIT 59 */
  pFrm -> dig_network_cap             = 0;  /* BIT 66 */
  pFrm -> na_letter                   = 0;  /* BIT 76 */
  pFrm -> na_legal                    = 0;  /* BIT 77 */
  pFrm -> sing_prog_seq_coding_basic  = 0;  /* BIT 78 */
  pFrm -> sing_prog_seq_coding_L0     = 0;  /* BIT 79 */


  /*
   *----------------------------------------------------------------
   *  bit settings different for DIS/DTC and DCS frames
   *----------------------------------------------------------------
   */
  pFrm -> ready_tx_fax = ( frmTyp EQ FRT_DCS )?     /* BIT 9 */
                           0 : pT30CmdPrm -> FLPval;
  pFrm -> rec_fax_op   = ( frmTyp EQ FRT_DCS )?     /* BIT 10 */
                           1 : pT30CmdPrm -> FCRval;
  pFrm -> sel_polling =  (frmTyp EQ FRT_DCS)?       /* BIT 47 */
                           0 : pT30CmdPrm -> FAPsep;

  if( frmTyp EQ FRT_DCS )                           /* BIT 49 */
    pFrm -> subaddr = ( pT30CmdPrm -> FSAsub[0] NEQ 0x0 )? 1 : 0;
  else
    pFrm -> subaddr = pT30CmdPrm -> FAPsub;

  if( frmTyp EQ FRT_DCS )                           /* BIT 50 */
    pFrm -> password = ( pT30CmdPrm -> FPWpwd[0] NEQ 0x0 )? 1 : 0;
  else
    pFrm -> password = pT30CmdPrm -> FAPpwd;

  /*
   *----------------------------------------------------------------
   *  bit settings concerning the bit rate
   *----------------------------------------------------------------
   */
  switch( br )                                  /* BIT 11-14 */
  {
    case( F_BR_2400 ):
      pFrm -> data_sig_rate = 0; break;
    case( F_BR_4800 ):
      pFrm -> data_sig_rate = 4; break;
    case( F_BR_7200 ):
      pFrm -> data_sig_rate = (frmTyp EQ FRT_DCS)? 12: 4; break;
    case( F_BR_9600 ):
      pFrm -> data_sig_rate = 8; break;
    case( F_BR_12000 ):
      pFrm -> data_sig_rate = (frmTyp EQ FRT_DCS)?  5: 8; break;
    case( F_BR_14400 ):
      pFrm -> data_sig_rate = (frmTyp EQ FRT_DCS)?  1:13; break;
  }

  /*
   *----------------------------------------------------------------
   *  bit settings concerning the resolution
   *----------------------------------------------------------------
   */
  pFrm -> R8_lines_pels  = 0;                 /* BIT 15 */
  pFrm -> m_res_pref     = 0;                 /* BIT 45 */
  pFrm -> i_res_pref     = 0;                 /* BIT 44 */
  pFrm -> R8_lines       = 0;                 /* BIT 41 */
  pFrm -> r_300_pels     = 0;                 /* BIT 42 */
  pFrm -> R16_lines_pels = 0;                 /* BIT 43 */

  if( frmTyp EQ FRT_DCS )                     /* for a DCS-frame */
  {
    switch( vr )
    {
      case( F_VR_R8X3_85 ):
        pFrm -> m_res_pref    = 1; break;

      case( F_VR_R8X7_7 ):
        pFrm -> R8_lines_pels = 1; pFrm -> m_res_pref     = 1; break;

      case( F_VR_R8X15_4 ):
        pFrm -> m_res_pref    = 1; pFrm -> R8_lines       = 1; break;

      case( F_VR_R16X15_4 ):
        pFrm -> m_res_pref    = 1; pFrm -> R16_lines_pels = 1; break;

      case( F_VR_200X100 ):
        pFrm -> i_res_pref    = 1; break;

      case( F_VR_200X200 ):
        pFrm -> R8_lines_pels = 1; pFrm -> i_res_pref     = 1; break;

      case( F_VR_200X400 ):
        pFrm -> i_res_pref    = 1; pFrm -> R16_lines_pels = 1; break;

      case( F_VR_300X300 ):
        pFrm -> i_res_pref    = 1; pFrm -> r_300_pels     = 1; break;
    }
  }
  else                                        /* for a DIS/DTC-frame */
  {
    pFrm -> R8_lines_pels  = 0;
    pFrm -> m_res_pref     = 1;

    if( vr & F_VR_R8X7_7 )
    { pFrm -> R8_lines_pels  = 1; }

    if( vr & F_VR_R8X15_4 )
    { pFrm -> R8_lines       = 1; }

    if( vr & F_VR_R16X15_4 )
    { pFrm -> R16_lines_pels = 1; }

    if( vr & F_VR_200X100 )
    { pFrm -> R8_lines_pels  = 0; pFrm -> i_res_pref = 1; }

    if( vr & F_VR_200X200 )
    { pFrm -> R8_lines_pels  = 1; pFrm -> i_res_pref = 1; }

    if( vr & F_VR_200X400 )
    { pFrm -> R16_lines_pels = 1; pFrm -> i_res_pref = 1; }

    if( vr & F_VR_300X300 )
    { pFrm -> r_300_pels     = 1; pFrm -> i_res_pref = 1; }
  }

  /*
   *----------------------------------------------------------------
   *  bit settings concerning the data compression format
   *----------------------------------------------------------------
   */
  pFrm -> two_dim_coding = 0;                 /* BIT 16 */
  pFrm -> uncomp_mode    = 0;                 /* BIT 26 */
  pFrm -> err_corr_mode  = 0;                 /* BIT 27 */
  pFrm -> t6_coding      = 0;                 /* BIT 31 */

  switch( df )
  {
    case( F_DF_1D_MdfHuff ):
      break;

    case( F_DF_2D_MdfRd_T4 ):
      pFrm -> two_dim_coding = 1;
      break;

    case( F_DF_2D_Uncomp ):
      pFrm -> two_dim_coding = 1;
      pFrm -> uncomp_mode    = 1;
      break;

    case( F_DF_2D_MdfRd_T6 ):
      pFrm -> two_dim_coding = 1;
      pFrm -> err_corr_mode  = 1;
      pFrm -> t6_coding      = 1;
      break;
  }

  /*
   *----------------------------------------------------------------
   *  bit settings concerning the page width
   *----------------------------------------------------------------
   */
  switch( wd )                            /* BIT 17-18 */
  {
    case( F_WD_1728 ): pFrm -> rec_width      = 0; break;

    case( F_WD_2048 ): pFrm -> rec_width      = 2; break;

    case( F_WD_2432 ): pFrm -> rec_width      = 1; break;

    case( F_WD_1216 ):
    case( F_WD_864 ):  pFrm -> rec_width      = 3; break;
  }

  /*
   *----------------------------------------------------------------
   *  bit settings concerning the page length
   *----------------------------------------------------------------
   */
  switch( ln )                            /* BIT 19-20 */
  {
    case( F_LN_A4 ):        pFrm -> max_rec_len = 0; break;

    case( F_LN_B4 ):        pFrm -> max_rec_len = 2; break;

    case( F_LN_Unlimited ): pFrm -> max_rec_len = 1; break;
  }

  /*
   *----------------------------------------------------------------
   *  bit settings concerning the scan time
   *----------------------------------------------------------------
   */
  switch( st )                            /* BIT 21-23 */
  {
    case( F_ST_0_0 ):
      pFrm -> min_scan_time = 7; break;

    case( F_ST_5_5 ):
      pFrm -> min_scan_time = 4; break;

    case( F_ST_10_5 ):
      pFrm -> min_scan_time = (frmTyp EQ FRT_DCS)? 2 : 3; break;

    case( F_ST_10_10 ):
      pFrm -> min_scan_time = 2; break;

    case( F_ST_20_10 ):
      pFrm -> min_scan_time = (frmTyp EQ FRT_DCS)? 0 : 6; break;

    case( F_ST_20_20 ):
      pFrm -> min_scan_time = 0; break;

    case( F_ST_40_20 ):
      pFrm -> min_scan_time = ( frmTyp EQ FRT_DCS)? 1 : 5; break;

    case( F_ST_40_40 ):
      pFrm -> min_scan_time = 1; break;
  }

  /*
   *----------------------------------------------------------------
   *  bit settings concerning the error correction mode
   *----------------------------------------------------------------
   */
  pFrm -> err_corr_mode = 0;              /* BIT 27*/
  pFrm -> duplex        = 0;              /* BIT 67*/

  switch( ec )
  {
    case( F_EC_DisableECM ):
      break;

    case( F_EC_EnableECM ):
      pFrm -> err_corr_mode = 1; pFrm -> duplex = 0; break;

    case( F_EC_EnableHalfDup ):
      pFrm -> err_corr_mode = 1; pFrm -> duplex = 0; break;

    case( F_EC_EnableFullDup ):
      pFrm -> err_corr_mode = 1; pFrm -> duplex = 1; break;
  }

  /*
   *----------------------------------------------------------------
   *  bit settings concerning the file transfer mode
   *----------------------------------------------------------------
   */
  pFrm -> bft          = 0;               /* BIT 53 */
  pFrm -> dtm          = 0;               /* BIT 54 */
  pFrm -> edi          = 0;               /* BIT 55 */
  pFrm -> btm          = 0;               /* BIT 57 */
  pFrm -> char_mode    = 0;               /* BIT 60 */
  pFrm -> mixed_mode   = 0;               /* BIT 62 */
  pFrm -> proc_mode_26 = 0;               /* BIT 65 */

  if( bf & F_BF_EnableBFT )       pFrm -> bft = 1;

  if( bf & F_BF_DocuTrnsfMode )   pFrm -> dtm = 1;

  if( bf & F_BF_EdifactMode )     pFrm -> edi = 1;

  if( bf & F_BF_BasicTrnsfMode )  pFrm -> btm = 1;

  if( bf & F_BF_CharMode )        pFrm -> char_mode = 1;

  if( bf & F_BF_MixMode )         pFrm -> mixed_mode = 1;

  if( bf & F_BF_ProcMode )        pFrm -> proc_mode_26 = 1;

  /*
   *----------------------------------------------------------------
   *  bit settings concerning the JPEG mode
   *----------------------------------------------------------------
   */
  pFrm -> jpeg               = 0;           /* BIT 68 */
  pFrm -> full_colour        = 0;           /* BIT 69 */
  pFrm -> huffman_tables     = 0;           /* BIT 70 */
  pFrm -> r_12_bits_pel_comp = 0;           /* BIT 71 */
  pFrm -> no_subsamp         = 0;           /* BIT 73 */
  pFrm -> cust_illum         = 0;           /* BIT 74 */
  pFrm -> cust_gamut         = 0;           /* BIT 75 */

  if( jp & F_JP_EnableJPEG )         pFrm -> jpeg = 1;

  if( jp & F_JP_FullColor )          pFrm -> full_colour = 1;

  if( jp & F_JP_EnablePrefHuff AND
      frmTyp EQ FRT_DCS            ) pFrm -> huffman_tables = 1;

  if( jp & F_JP_12BitsPelComp )      pFrm -> r_12_bits_pel_comp = 1;

  if( jp & F_JP_NoSubsmpl )          pFrm -> no_subsamp = 1;

  if( jp & F_JP_CustIllum )          pFrm -> cust_illum = 1;

  if( jp & F_JP_CustGamutRange )     pFrm -> cust_gamut = 1;

}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_T30                 |
|                                 ROUTINE : cmhT30_StartFIT         |
+-------------------------------------------------------------------+

  PURPOSE : start FAX inactivity timer

*/

GLOBAL void cmhT30_StartFIT ( void )
{
  TRACE_FUNCTION( "cmhT30_StartFIT()" );

  if( t30ShrdPrm.faxStat NEQ NO_VLD_FS AND
      fnd_cmhPrm[t30EntStat.entOwn].t30CmdPrm.FITtime NEQ 0 )
  {
    TIMERSTART( fnd_cmhPrm[t30EntStat.entOwn].t30CmdPrm.FITtime, FIT_RUN );
    FITRunFlg = TRUE;
  }
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_T30                 |
|                                 ROUTINE : cmhT30_StopFIT          |
+-------------------------------------------------------------------+

  PURPOSE : stop FAX inactivity timer

*/

GLOBAL void cmhT30_StopFIT ( void )
{
  TRACE_FUNCTION( "cmhT30_StopFIT()" );

  if( FITRunFlg )
  {
    TIMERSTOP (FIT_RUN);
    FITRunFlg = FALSE;
  }
}
#endif /* DTI OR FF_FAX*/
/*==== EOF ========================================================*/


