/* 
+----------------------------------------------------------------------------- 
|  Project :  GSM-PS (6147)
|  Modul   :  PSA_SSS
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
|  Purpose :  This module defines the signalling functions of the
|             protocol stack adapter for supplementary services.
+----------------------------------------------------------------------------- 
*/ 

#ifndef PSA_SSS_C
#define PSA_SSS_C
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

#include "ksd.h"

#include "aci.h"
#include "psa.h"
#include "psa_ss.h"
#include "cmh_ss.h"

#ifdef SIM_TOOLKIT
#include "psa_cc.h"
#include "psa_sat.h"
#endif /* of SIM_TOOLKIT */


/*==== CONSTANTS ==================================================*/


/*==== TYPES ======================================================*/


/*==== EXPORT =====================================================*/


/*==== VARIABLES ==================================================*/

EXTERN T_PCEER causeMod;
EXTERN SHORT causeCeer;

/*==== FUNCTIONS ==================================================*/
static UBYTE inv_opcode;


LOCAL BOOL psaSS_FillPrimForTrns ( T_SS_SRV_TBL  *pStbNtry, 
                                   T_fac_inf     *fac_inf,
                                   T_VOID_STRUCT *mnss_req,
                                   BOOL           end_trns);

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)    MODULE  : PSA_SSS                      |
|                            ROUTINE : psaSS_NewTrns                |
+-------------------------------------------------------------------+

  PURPOSE : start a new SS transaction

*/

GLOBAL SHORT psaSS_NewTrns ( SHORT sId )
{
  T_SS_SRV_TBL * pStbNtry;     /* holds pointer to service table entry */
  BYTE ccdRet;                 /* holds CCD return value */

  TRACE_FUNCTION ("psaSS_NewTrns()");

/*
 *-------------------------------------------------------------------
 * get a valid ti for a MOS, otherwise return
 *-------------------------------------------------------------------
 */
  if( psaSS_getMOSTi( sId ) < 0 ) return ( -1 );

  causeMod  = P_CEER_mod;      /* Clear module which was set for ceer */
  causeCeer = CEER_NotPresent; /* Clear extended error cause */

/*
 *-------------------------------------------------------------------
 * create and send primitive
 *-------------------------------------------------------------------
 */
  {
    PALLOC (mnss_begin_req, MNSS_BEGIN_REQ);

    /*
     * fill in primitive parameter: begin request
     */
    pStbNtry = &ssShrdPrm.stb[sId];

    mnss_begin_req -> ti      = pStbNtry -> ti;

    if( pStbNtry -> SSver NEQ NOT_PRESENT_8BIT )
    {
      mnss_begin_req -> ss_ver.len    = 1;
      mnss_begin_req -> ss_ver.ver[0] = pStbNtry -> SSver;
    }
    else

      mnss_begin_req -> ss_ver.len = 0;

    {
      MCAST( com, COMPONENT );

      memset( com, 0, sizeof( T_COMPONENT ));
      switch( ssShrdPrm.cmpType )
      {
        case( CT_INV ):
          com -> v_inv_comp = TRUE;
          com -> inv_comp.v_inv_id  = TRUE;
          com -> inv_comp.inv_id    = pStbNtry -> iId
                                    = ssShrdPrm.iIdNxt++;
          com -> inv_comp.v_op_code = TRUE;
          com -> inv_comp.op_code   = pStbNtry -> opCode
                                    = ssFIECodeBuf.buf[0];

          inv_opcode = com->inv_comp.op_code;

          com -> inv_comp.v_params  = TRUE;
          com -> inv_comp.params.l_params = ssFIECodeBuf.l_buf-8;
          com -> inv_comp.params.o_params = 8;
          memcpy( com -> inv_comp.params.b_params,
                  ssFIECodeBuf.buf, ssFIECodeBuf.l_buf>>3 );
          break;
        case( CT_RET_RSLT ):
          break;
        case( CT_RET_ERR ):
          break;
        case( CT_RET_REJ ):
          break;
      }
      mnss_begin_req -> fac_inf.l_fac = MNCC_FACILITY_LEN<<3;
      mnss_begin_req -> fac_inf.o_fac = 0;
      ccdRet = ccd_codeMsg (CCDENT_FAC,
                            UPLINK,
                            (T_MSGBUF *) &mnss_begin_req -> fac_inf,
                            (UBYTE    *) _decodedMsg,
                            COMPONENT);

      if( ccdRet NEQ ccdOK )
      {
/* Implements Measure#32: Row 1316 */
        TRACE_EVENT_P1("CCD Coding Error: %d",ccdRet );
        PFREE( mnss_begin_req );
        return( -1 );
      }
    }

    pStbNtry -> srvType = ST_MOS;

    psaSS_DumpFIE ( &mnss_begin_req -> fac_inf  );
#ifdef TI_PS_FF_AT_P_CMD_CSCN
    cmhSS_sendFie ( CSCN_FACILITY_DIRECTION_OUT,
                    CSCN_FACILITY_TRANS_TYPE_BEGIN,
                    &mnss_begin_req -> fac_inf );
#endif /* TI_PS_FF_AT_P_CMD_CSCN */

    if ( ssShrdPrm.mltyTrnFlg EQ 0 )
    {
      PSENDX (SS, mnss_begin_req);     /* send primitive */
    }
    else
      pStbNtry -> save_prim = mnss_begin_req;  /* already one transaction in process */


    /*
     * update service status
     */
    pStbNtry -> srvStat = SSS_ACT;
  }

  return 0;
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : PSA_SSS                 |
|                                 ROUTINE : psaSS_EndTrns           |
+-------------------------------------------------------------------+

  PURPOSE : stop an existing SS transaction

*/

GLOBAL SHORT psaSS_EndTrns ( SHORT sId )
{
  T_SS_SRV_TBL * pStbNtry;     /* holds pointer to service table entry */

  TRACE_FUNCTION ("psaSS_EndTrns()");

/*
 *-------------------------------------------------------------------
 * create and send primitive
 *-------------------------------------------------------------------
 */
  {
    PALLOC (mnss_end_req, MNSS_END_REQ);

    /*
     * fill in primitive parameter: end request
     */
    pStbNtry = &ssShrdPrm.stb[sId];

    mnss_end_req -> ti       = pStbNtry -> ti;

    /* Implements Measure# 103 */
    if ( psaSS_FillPrimForTrns ( pStbNtry, &(mnss_end_req -> fac_inf),
                                 (T_VOID_STRUCT *)mnss_end_req, TRUE))
    {
        return(-1);
    }

    psaSS_DumpFIE ( &mnss_end_req -> fac_inf  );
#ifdef TI_PS_FF_AT_P_CMD_CSCN
    cmhSS_sendFie ( CSCN_FACILITY_DIRECTION_OUT,
                    CSCN_FACILITY_TRANS_TYPE_END,
                    &mnss_end_req -> fac_inf );
#endif /* TI_PS_FF_AT_P_CMD_CSCN */

    PSENDX (SS, mnss_end_req);

#ifdef SIM_TOOLKIT
    if ( sId EQ satShrdPrm.SentUSSDid                   AND 
         satShrdPrm.cmdDet.cmdType EQ SAT_CMD_SEND_USSD )
    {
      satShrdPrm.USSDterm = TRUE;
    }
#endif

    /*
     * update call status
     */
    pStbNtry -> srvStat = SSS_IDL;

  }

  psaSS_retMOSTi (sId);

  return 0;
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : PSA_SSS                 |
|                                 ROUTINE : psaSS_CntTrns           |
+-------------------------------------------------------------------+

  PURPOSE : continue with an existing SS transaction

*/

GLOBAL SHORT psaSS_CntTrns ( SHORT sId )
{
  T_SS_SRV_TBL * pStbNtry;     /* holds pointer to service table entry */

  TRACE_FUNCTION ("psaSS_CntTrns()");

/*
 *-------------------------------------------------------------------
 * create and send primitive
 *-------------------------------------------------------------------
 */
  {
    PALLOC (mnss_facility_req, MNSS_FACILITY_REQ);

    /*
     * fill in primitive parameter: facility request
     */
    pStbNtry = &ssShrdPrm.stb[sId];

    mnss_facility_req -> ti      = pStbNtry -> ti;

    /* Implements Measure# 103 */
    if ( psaSS_FillPrimForTrns ( pStbNtry, &(mnss_facility_req -> fac_inf),
                                 (T_VOID_STRUCT *)mnss_facility_req, FALSE))
    {
        return(-1);
    }

    psaSS_DumpFIE ( &mnss_facility_req -> fac_inf  );
#ifdef TI_PS_FF_AT_P_CMD_CSCN
    cmhSS_sendFie ( CSCN_FACILITY_DIRECTION_OUT,
                    CSCN_FACILITY_TRANS_TYPE,
                    &mnss_facility_req -> fac_inf );
#endif /* TI_PS_FF_AT_P_CMD_CSCN */

    PSENDX (SS, mnss_facility_req);
  }

  return 0;
}

/* Implements Measure# 103 */
/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : PSA_SSS                 |
|                                 ROUTINE : psaSS_CntTrns           |
+-------------------------------------------------------------------+

  PURPOSE : End Or continue with an existing SS transaction

*/
LOCAL BOOL psaSS_FillPrimForTrns ( T_SS_SRV_TBL  *pStbNtry, 
                                   T_fac_inf     *fac_inf,
                                   T_VOID_STRUCT *mnss_req,
                                   BOOL           end_trns)
{
  TRACE_FUNCTION ("psaSS_FillPrimForTrns()");
  {
    UBYTE ccdRet;
    MCAST( com, COMPONENT );

    memset( com, 0, sizeof( T_COMPONENT ));
    switch( ssShrdPrm.cmpType )
    {
      case( CT_RET_RSLT ):
        com -> v_res_comp = TRUE;
        com -> res_comp.v_inv_id   = TRUE;
        com -> res_comp.inv_id     = pStbNtry -> iId;
        /* patch for FTA 31.9.2.1:
         * When empty facility should be returned, then message
         * should contain ONLY the invoke id. CLB 061201 
         */
        if( ssFIECodeBuf.l_buf NEQ 8 OR end_trns)  
        /* means there actually are parameters */
        {
          com -> res_comp.v_sequence = TRUE;
          com -> res_comp.sequence.v_op_code = TRUE;
          com -> res_comp.sequence.op_code   = pStbNtry -> opCode;

          if (!end_trns) 
          {
            pStbNtry -> opCode = inv_opcode;
          }

          com -> res_comp.sequence.v_params  = TRUE;
          com -> res_comp.sequence.params.l_params = ssFIECodeBuf.l_buf-8;
          com -> res_comp.sequence.params.o_params = 8;
          memcpy( com -> res_comp.sequence.params.b_params,
          ssFIECodeBuf.buf, ssFIECodeBuf.l_buf>>3 );
        }
        break;
      case( CT_RET_ERR ):
        com -> v_err_comp = TRUE;
        com -> err_comp.v_inv_id   = TRUE;
        com -> err_comp.inv_id     = pStbNtry -> iId;
        com -> err_comp.v_err_code = TRUE;
        com -> err_comp.err_code   = pStbNtry -> errCd;
        break;
      case( CT_RET_REJ ):
        break;
    }
    fac_inf->l_fac = FACILITY_LEN<<3;
    fac_inf->o_fac = 0;
    ccdRet = ccd_codeMsg (CCDENT_FAC,
                          UPLINK,
                          (T_MSGBUF *) fac_inf,
                          (UBYTE    *) _decodedMsg,
                          COMPONENT);

    if( ccdRet NEQ ccdOK )
    {
      TRACE_EVENT_P1 ("CCD Coding Error: %d",ccdRet);
      PFREE( mnss_req );
      return( TRUE );
    }
  }
  return ( FALSE );
}

/*==== EOF ========================================================*/

