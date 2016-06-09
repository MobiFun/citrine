/* 
+----------------------------------------------------------------------------- 
|  Project :  GSM-F&D (8411)
|  Modul   :  ATI
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
|  Purpose :  AT Command to connect UART with MTST in order to send 
|             traces on the channel
+----------------------------------------------------------------------------- 
*/ 

#ifndef ATI_MMI_C
#define ATI_MMI_C

#include "config.h"
#include "fixedconf.h"
#include "condat-features.h"
#include "aci_conf.h"

#include "aci_all.h"

#include "aci_cmh.h"
#include "ati_cmd.h"
#include "aci_io.h"
#include "aci_cmd.h"
#include "dti.h"      /* functionality of the dti library */

#include "aci_mem.h"
#include "aci_cmh.h"
#ifdef FF_PSI
#include "psa_psi.h"
#endif /*FF_PSI*/

#ifdef FF_TRACE_OVER_MTST

typedef enum {
  MTST_CONNECT  = 0,
  MTST_DISCONNECT  
} T_MTST_CONNECT_CMD;

LOCAL T_ACI_RETURN sAT_PercentMTST (T_ACI_CMD_SRC srcId,
                                    T_MTST_CONNECT_CMD connect_cmd); 
GLOBAL BOOL MTST_connect_dti_cb(UBYTE dti_id, T_DTI_MNG_CB_TYPE result_type);

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  :                    |
| STATE   : code                        ROUTINE :                    |
+--------------------------------------------------------------------+

  PURPOSE : %MTST command ( display trace funktion )
*/

GLOBAL T_ATI_RSLT setatPercentMTST ( char *cl, UBYTE srcId ) /* Function prototype is T_ATI_RSLT,so it is changed to T_ATI_RSLT */ 

{
  TRACE_FUNCTION( "setatPercentMTST()" );

  switch( *cl )
  {
    case '0':
      if (sAT_PercentMTST(srcId, MTST_DISCONNECT) EQ AT_FAIL)
      {
        cmdCmeError(CME_ERR_Unknown);
        return (ATI_FAIL);
      }
      break;

    case '1':
      if (sAT_PercentMTST (srcId, MTST_CONNECT) EQ AT_FAIL)
      {
        cmdCmeError(CME_ERR_Unknown);
        return (ATI_FAIL);
      }
      break;

    default:
      cmdCmeError(CME_ERR_Unknown);
      return (ATI_FAIL);
   }

  return (ATI_CMPL);
}

 /*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  :                    |
| STATE   : code                        ROUTINE :                    |
+--------------------------------------------------------------------+

  PURPOSE : 
*/


GLOBAL T_ACI_RETURN sAT_PercentMTST (T_ACI_CMD_SRC srcId,
                                     T_MTST_CONNECT_CMD connect_cmd)
                                   
{
  UBYTE                dti_id;
  T_DTI_MNG_CONN_STATE state;
#ifdef FF_PSI
  T_ACI_DTI_PRC_PSI *dti_prc=find_element (psi_src_params, srcId, cmhPSItest_srcId);
#endif /*FF_PSI*/
  TRACE_FUNCTION ("sAT_PercentMTST()");

  switch (connect_cmd)
  {
    case MTST_CONNECT:
      if (dti_mng_connection (UNIT_MTST, &dti_id, &state) NEQ UNIT_INVALID)
      {
        return( AT_FAIL );
      }
#ifdef FF_PSI
      if (dti_prc EQ NULL)
      {
         if (!dti_mng_connect((UBYTE) srcId, DTI_ENTITY_UART, DTI_ENTITY_MTST,MTST_connect_dti_cb))
         {
            return( AT_FAIL );
         }
         return(AT_CMPL); 
      }
      else
      {
         if (!dti_mng_connect((UBYTE) srcId, DTI_ENTITY_PSI, DTI_ENTITY_MTST,MTST_connect_dti_cb))
         {
            return( AT_FAIL );
         }
         return(AT_CMPL); 
      }
#else
      if (!dti_mng_connect((UBYTE) srcId, 
                           DTI_ENTITY_UART, DTI_ENTITY_MTST,
                           MTST_connect_dti_cb))
      {
        return( AT_FAIL );
      }
      return(AT_CMPL); 
#endif /*FF_PSI*/

    case MTST_DISCONNECT:
      if (dti_mng_connection (DTI_ENTITY_MTST, &dti_id, &state) NEQ DTI_ENTITY_INVALID)
      {
        if (!dti_mng_disconnect ((UBYTE) dti_id))
        {
          return (AT_FAIL);
        }
      }
      return (AT_CMPL);
  
    default:
      /* TRCCmeError(CME_ERR_Unknown);*/ 
      return (AT_FAIL);
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  :                    |
| STATE   : code                        ROUTINE : MTST_connect_dti_cb|
+--------------------------------------------------------------------+

  PURPOSE : Callback for connection between MTST and UART/PSI.  

*/
 
GLOBAL BOOL MTST_connect_dti_cb(UBYTE dti_id, T_DTI_MNG_CB_TYPE result_type) 
{ 
  return TRUE;
}

#endif /* FF_TRACE_OVER_MTST */

#endif /* ATI_MMI_C */
