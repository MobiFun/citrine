/* 
+----------------------------------------------------------------------------- 
|  Project :  GSM-PS (6147)
|  Modul   :  PSA_MTST
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
|  Purpose :  send primitiv to MTST in order to connect him with UART
+----------------------------------------------------------------------------- 
*/ 


#ifndef ATI_MMI_C
#define ATI_MMI_C
#endif

#include "aci_all.h"
/*===== INCLUDES ===================================================*/
#include "aci_cmh.h"
#include "ati_cmd.h"
#include "aci_io.h"
#include "aci_cmd.h"
#include "dti.h"      /* functionality of the dti library */

#include "aci_mem.h"
#include "aci_cmd.h"
#include "aci.h"

#ifdef FF_TRACE_OVER_MTST

#define CONFIG_DTI_MTST     0x4711
typedef struct {
  char str[100];
} T_CONFIG_DTI_MTST;

GLOBAL SHORT psaMTST_Switch_Trace (UBYTE dti_id, 
                                   T_DTI_ENTITY_ID conn_peer_Id,
                                   BOOL switch_on)
{
  char primString[100];
  CHAR *p_ent_name;

  TRACE_FUNCTION("psaMTST_Switch_Trace");

  switch( conn_peer_Id )
  {
    case( DTI_ENTITY__UART ):
      p_ent_name = &UART_NAME[0];
      break;

    case( DTI_ENTITY_MTST ):
      p_ent_name = &MTST_NAME[0];
      break;

    default:
      return PEI_ERROR;
  }

  if (switch_on) 
  {
    /* build config string */
    sprintf (primString, "CONFIG %s %d %d", p_ent_name, 0/*tui*/, dti_id);
  }
  else
  {
    sprintf (primString, "CONFIG STOP");
  }

  {
    /* send config primitive to MTST */
    PALLOC (prim, CONFIG_DTI_MTST);
    D2P(prim)->opc |= SYS_MASK;
    strcpy (prim->str, primString);

    PSEND (hCommMTST, prim);
  }
  /* */
  return PEI_OK;
}
#endif /* FF_TRACE_OVER_MTST */
