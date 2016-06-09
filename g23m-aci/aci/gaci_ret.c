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
|  Purpose :  GPRS AT Command Interpreter Call-Back Functions.
+----------------------------------------------------------------------------- 
*/ 

#ifdef GPRS

#ifndef GACI_RET_C
#define GACI_RET_C
#endif

#include "aci_all.h"

/*
 *  includes
 */
#include "dti.h"      /* functionality of the dti library */
#include "aci_cmh.h"
#include "ati_cmd.h"
#include "aci_cmd.h"
#include "aci_io.h"
#include "aci.h"
#include "l4_tim.h"
#include "aci_lst.h"

#include "dti_conn_mng.h"

#include "gaci.h"
#include "gaci_cmh.h"
#include "psa.h"
#include "psa_sm.h"

#include "cmh.h"
#include "cmh_sm.h"

#include "aci_io.h"
#include "aci_mem.h"

#include "ati_int.h"

/*
+--------------------------------------------------------------------+
| PROJECT : GPRS (8441)                 MODULE  : GACI_RET           |
| STATE   : code                        ROUTINE : rCI_PlusCGACT      |
+--------------------------------------------------------------------+

  PURPOSE : handles rCI_PlusCGACT call back

*/
GLOBAL void rCI_PlusCGACT ( SHORT link_id )
{
  UBYTE srcId = srcId_cb;
  T_ATI_SRC_PARAMS *src_params = find_element (ati_src_list, srcId, search_ati_src_id);

  TRACE_FUNCTION("rCI_PlusCGACT()");

  src_params->curAtCmd = AT_CMD_NONE;

}

/*
+--------------------------------------------------------------------+
| PROJECT : GPRS (8441)                 MODULE  : GACI_RET           |
| STATE   : code                        ROUTINE : rCI_PlusCGDATA     |
+--------------------------------------------------------------------+

  PURPOSE : handles rCI_PlusCGDATA call back

*/
GLOBAL void rCI_PlusCGDATA ( SHORT link_id )
{
  UBYTE srcId = srcId_cb;
  T_ATI_SRC_PARAMS *src_params = find_element (ati_src_list, srcId, search_ati_src_id);

  TRACE_FUNCTION("rCI_PlusCGDATA()");

  src_params->curAtCmd = AT_CMD_NONE;

}

/*
+--------------------------------------------------------------------+
| PROJECT : GPRS (8441)                 MODULE  : GACI_RET           |
| STATE   : code                        ROUTINE : rCI_PlusCGANS      |
+--------------------------------------------------------------------+

  PURPOSE : handles rCI_PlusCGANS call back

*/
GLOBAL void rCI_PlusCGANS ( SHORT link_id )
{
  UBYTE srcId = srcId_cb;
  T_ATI_SRC_PARAMS *src_params = find_element (ati_src_list, srcId, search_ati_src_id);

  TRACE_FUNCTION("rCI_PlusCGANS()");

  src_params->curAtCmd = AT_CMD_NONE;

}

/*
+--------------------------------------------------------------------+
| PROJECT : GPRS (8441)                 MODULE  : GACI_RET           |
| STATE   : code                        ROUTINE : rCI_PlusCGEREP     |
+--------------------------------------------------------------------+

  PURPOSE : handles rCI_PlusCGEREP call back

*/
GLOBAL void rCI_PlusCGEREP ( T_CGEREP_EVENT event, T_CGEREP_EVENT_REP_PARAM *param )
{
  char *me="+CGEV: ";
  UBYTE srcId = srcId_cb;
  char  pdp_ip_addr_str[17];
  U8    idx;

  TRACE_FUNCTION("rCI_PlusCGEREP()");

  if (sm_cgerep_srcId NEQ srcId)
  {
    return; /* don't indicate +CGEV to a source on which AT+CGEREP was not set up */
  }

  if( CGEREP_MODE_DICARD_RESERVED EQ sm_cgerep_mode AND 
      uart_is_mt_te_link () NEQ FALSE )
  {
    return;
  }
  
  memset(pdp_ip_addr_str, 0x00, sizeof(pdp_ip_addr_str));
  
  switch(event)
  {
    case CGEREP_EVENT_NW_REACT:
    case CGEREP_EVENT_NW_DEACT:
    case CGEREP_EVENT_ME_DEACT:
      switch(param->act.pdp_addr.ctrl_ip_address)
      {
        case NAS_is_ipv4:
          sprintf( pdp_ip_addr_str, "%d.%d.%d.%d", 
                             param->act.pdp_addr.ip_address.ipv4_addr.a4[0],
                             param->act.pdp_addr.ip_address.ipv4_addr.a4[1],
                             param->act.pdp_addr.ip_address.ipv4_addr.a4[2],
                             param->act.pdp_addr.ip_address.ipv4_addr.a4[3]);
          break;
        case NAS_is_ipv6:
          for(idx = 0;idx < NAS_SIZE_IPv6_ADDR;idx++)
          sprintf( pdp_ip_addr_str, "%d.%d.%d.%d.%d.%d.%d.%d.%d.%d.%d.%d.%d.%d.%d.%d", 
                             param->act.pdp_addr.ip_address.ipv6_addr.a6[0],
                             param->act.pdp_addr.ip_address.ipv6_addr.a6[1],
                             param->act.pdp_addr.ip_address.ipv6_addr.a6[2],
                             param->act.pdp_addr.ip_address.ipv6_addr.a6[3],
                             param->act.pdp_addr.ip_address.ipv6_addr.a6[4],
                             param->act.pdp_addr.ip_address.ipv6_addr.a6[5],
                             param->act.pdp_addr.ip_address.ipv6_addr.a6[6],
                             param->act.pdp_addr.ip_address.ipv6_addr.a6[7],
                             param->act.pdp_addr.ip_address.ipv6_addr.a6[8],
                             param->act.pdp_addr.ip_address.ipv6_addr.a6[9],
                             param->act.pdp_addr.ip_address.ipv6_addr.a6[10],
                             param->act.pdp_addr.ip_address.ipv6_addr.a6[11],
                             param->act.pdp_addr.ip_address.ipv6_addr.a6[12],
                             param->act.pdp_addr.ip_address.ipv6_addr.a6[13],
                             param->act.pdp_addr.ip_address.ipv6_addr.a6[14],
                             param->act.pdp_addr.ip_address.ipv6_addr.a6[15] );
          break;
      }
      break;
    case CGEREP_EVENT_REJECT:
      switch(param->act.pdp_addr.ctrl_ip_address)
      {
        case NAS_is_ipv4:
          sprintf( pdp_ip_addr_str, "%d.%d.%d.%d", 
                             param->reject.pdp_addr.ip_address.ipv4_addr.a4[0],
                             param->reject.pdp_addr.ip_address.ipv4_addr.a4[1],
                             param->reject.pdp_addr.ip_address.ipv4_addr.a4[2],
                             param->reject.pdp_addr.ip_address.ipv4_addr.a4[3]);
          break;
        case NAS_is_ipv6:
          for(idx = 0;idx < NAS_SIZE_IPv6_ADDR;idx++)
          sprintf( pdp_ip_addr_str, "%d.%d.%d.%d.%d.%d.%d.%d.%d.%d.%d.%d.%d.%d.%d.%d", 
                             param->reject.pdp_addr.ip_address.ipv6_addr.a6[0],
                             param->reject.pdp_addr.ip_address.ipv6_addr.a6[1],
                             param->reject.pdp_addr.ip_address.ipv6_addr.a6[2],
                             param->reject.pdp_addr.ip_address.ipv6_addr.a6[3],
                             param->reject.pdp_addr.ip_address.ipv6_addr.a6[4],
                             param->reject.pdp_addr.ip_address.ipv6_addr.a6[5],
                             param->reject.pdp_addr.ip_address.ipv6_addr.a6[6],
                             param->reject.pdp_addr.ip_address.ipv6_addr.a6[7],
                             param->reject.pdp_addr.ip_address.ipv6_addr.a6[8],
                             param->reject.pdp_addr.ip_address.ipv6_addr.a6[9],
                             param->reject.pdp_addr.ip_address.ipv6_addr.a6[10],
                             param->reject.pdp_addr.ip_address.ipv6_addr.a6[11],
                             param->reject.pdp_addr.ip_address.ipv6_addr.a6[12],
                             param->reject.pdp_addr.ip_address.ipv6_addr.a6[13],
                             param->reject.pdp_addr.ip_address.ipv6_addr.a6[14],
                             param->reject.pdp_addr.ip_address.ipv6_addr.a6[15] );
          break;
      }
      break;

  }

  switch ( event )
  {
    case CGEREP_EVENT_REJECT:
      sprintf(g_sa,"%s%s \"%s\",\"%s\"", me, "REJECT", param->reject.pdp_type, pdp_ip_addr_str);
      break;
    case CGEREP_EVENT_NW_REACT:
      sprintf(g_sa,"%s%s \"%s\",\"%s\",%hd", me, "NW REACT", param->act.pdp_type, pdp_ip_addr_str, param->act.cid);
      break;
    case CGEREP_EVENT_NW_DEACT:
      sprintf(g_sa,"%s%s \"%s\",\"%s\",%hd", me, "NW DEACT", param->act.pdp_type, pdp_ip_addr_str, param->act.cid);
      break;
    case CGEREP_EVENT_ME_DEACT:
      sprintf(g_sa,"%s%s \"%s\",\"%s\",%hd", me, "ME DEACT", param->act.pdp_type, pdp_ip_addr_str, param->act.cid);
      break;
    case CGEREP_EVENT_NW_DETACH:
      sprintf(g_sa,"%s%s", me, "NW DETACH" );
      break;
    case CGEREP_EVENT_ME_DETACH:
      sprintf(g_sa,"%s%s", me, "ME DETACH" );
      break;
    case CGEREP_EVENT_NW_CLASS:
      sprintf(g_sa,"%s%s", me, "NW CLASS" );
      break;
    case CGEREP_EVENT_ME_CLASS:
      sprintf(g_sa,"%s%s", me, "ME CLASS" );
      break;
  }

  if ( CGEREP_MODE_BUFFER NEQ sm_cgerep_mode AND 
       uart_is_mt_te_link () EQ FALSE)
  {
    io_sendIndication(srcId, g_sa, ATI_FORCED_OUTPUT);
  }
  else
  { /* write in event reporting buffer */
    cmhSM_save_event(event, param);
  }
}


/*
+--------------------------------------------------------------------+
| PROJECT : GPRS (8441)                 MODULE  : GACI_RET           |
| STATE   : code                        ROUTINE : rCI_PercentCGEV     |
+--------------------------------------------------------------------+

  PURPOSE : handles rCI_PercentCGEREP call back

*/
GLOBAL void rCI_PercentCGEV ( T_CGEREP_EVENT event, T_CGEREP_EVENT_REP_PARAM *param )
{
  char *me="%CGEV: ";
  UBYTE srcId = srcId_cb;
  char  pdp_ip_addr_str[17];
  U8    idx = 0;
  
  TRACE_FUNCTION("rCI_PercentCGEV()");

  if (ati_user_output_cfg[srcId].Percent_CGEREP_stat == 0)
  {
    return; /* don't indicate %CGEV to a source on which AT%CGEREP was not set up */
  }

  memset(pdp_ip_addr_str, 0x00, sizeof(pdp_ip_addr_str));

  switch (event)
  {
    case CGEREP_EVENT_NW_REACT:
    case CGEREP_EVENT_NW_DEACT:
    case CGEREP_EVENT_ME_DEACT:
    case CGEREP_EVENT_NW_ACT:
    case CGEREP_EVENT_ME_ACT:
      switch(param->act.pdp_addr.ctrl_ip_address)
      {
        case NAS_is_ipv4:
          sprintf( pdp_ip_addr_str, "%d.%d.%d.%d", 
                             param->act.pdp_addr.ip_address.ipv4_addr.a4[0],
                             param->act.pdp_addr.ip_address.ipv4_addr.a4[1],
                             param->act.pdp_addr.ip_address.ipv4_addr.a4[2],
                             param->act.pdp_addr.ip_address.ipv4_addr.a4[3]);
          break;
        case NAS_is_ipv6:
          for(idx = 0;idx < NAS_SIZE_IPv6_ADDR;idx++)
          sprintf( pdp_ip_addr_str, "%d.%d.%d.%d.%d.%d.%d.%d.%d.%d.%d.%d.%d.%d.%d.%d", 
                             param->act.pdp_addr.ip_address.ipv6_addr.a6[0],
                             param->act.pdp_addr.ip_address.ipv6_addr.a6[1],
                             param->act.pdp_addr.ip_address.ipv6_addr.a6[2],
                             param->act.pdp_addr.ip_address.ipv6_addr.a6[3],
                             param->act.pdp_addr.ip_address.ipv6_addr.a6[4],
                             param->act.pdp_addr.ip_address.ipv6_addr.a6[5],
                             param->act.pdp_addr.ip_address.ipv6_addr.a6[6],
                             param->act.pdp_addr.ip_address.ipv6_addr.a6[7],
                             param->act.pdp_addr.ip_address.ipv6_addr.a6[8],
                             param->act.pdp_addr.ip_address.ipv6_addr.a6[9],
                             param->act.pdp_addr.ip_address.ipv6_addr.a6[10],
                             param->act.pdp_addr.ip_address.ipv6_addr.a6[11],
                             param->act.pdp_addr.ip_address.ipv6_addr.a6[12],
                             param->act.pdp_addr.ip_address.ipv6_addr.a6[13],
                             param->act.pdp_addr.ip_address.ipv6_addr.a6[14],
                             param->act.pdp_addr.ip_address.ipv6_addr.a6[15] );
          break;
      }
      break;
    case CGEREP_EVENT_REJECT:
      switch(param->act.pdp_addr.ctrl_ip_address)
      {
        case NAS_is_ipv4:
          sprintf( pdp_ip_addr_str, "%d.%d.%d.%d", 
                             param->reject.pdp_addr.ip_address.ipv4_addr.a4[0],
                             param->reject.pdp_addr.ip_address.ipv4_addr.a4[1],
                             param->reject.pdp_addr.ip_address.ipv4_addr.a4[2],
                             param->reject.pdp_addr.ip_address.ipv4_addr.a4[3]);
          break;
        case NAS_is_ipv6:
          for(idx = 0;idx < NAS_SIZE_IPv6_ADDR;idx++)
          sprintf( pdp_ip_addr_str, "%d.%d.%d.%d.%d.%d.%d.%d.%d.%d.%d.%d.%d.%d.%d.%d", 
                             param->reject.pdp_addr.ip_address.ipv6_addr.a6[0],
                             param->reject.pdp_addr.ip_address.ipv6_addr.a6[1],
                             param->reject.pdp_addr.ip_address.ipv6_addr.a6[2],
                             param->reject.pdp_addr.ip_address.ipv6_addr.a6[3],
                             param->reject.pdp_addr.ip_address.ipv6_addr.a6[4],
                             param->reject.pdp_addr.ip_address.ipv6_addr.a6[5],
                             param->reject.pdp_addr.ip_address.ipv6_addr.a6[6],
                             param->reject.pdp_addr.ip_address.ipv6_addr.a6[7],
                             param->reject.pdp_addr.ip_address.ipv6_addr.a6[8],
                             param->reject.pdp_addr.ip_address.ipv6_addr.a6[9],
                             param->reject.pdp_addr.ip_address.ipv6_addr.a6[10],
                             param->reject.pdp_addr.ip_address.ipv6_addr.a6[11],
                             param->reject.pdp_addr.ip_address.ipv6_addr.a6[12],
                             param->reject.pdp_addr.ip_address.ipv6_addr.a6[13],
                             param->reject.pdp_addr.ip_address.ipv6_addr.a6[14],
                             param->reject.pdp_addr.ip_address.ipv6_addr.a6[15] );
          break;
      }
      break;

  }


  switch ( event )
  {
    case CGEREP_EVENT_REJECT:
      sprintf(g_sa,"%s%s \"%s\",\"%s\"", me, "REJECT", param->reject.pdp_type, pdp_ip_addr_str);
      break;
    case CGEREP_EVENT_NW_REACT:
      sprintf(g_sa,"%s%s \"%s\",\"%s\",%hd", me, "NW REACT", param->act.pdp_type, pdp_ip_addr_str, param->act.cid);
      break;
    case CGEREP_EVENT_NW_DEACT:
      sprintf(g_sa,"%s%s \"%s\",\"%s\",%hd", me, "NW DEACT", param->act.pdp_type, pdp_ip_addr_str, param->act.cid);
      break;
    case CGEREP_EVENT_ME_DEACT:
      sprintf(g_sa,"%s%s \"%s\",\"%s\",%hd", me, "ME DEACT", param->act.pdp_type, pdp_ip_addr_str, param->act.cid);
      break;
    case CGEREP_EVENT_NW_DETACH:
      sprintf(g_sa,"%s%s", me, "NW DETACH" );
      break;
    case CGEREP_EVENT_ME_DETACH:
      sprintf(g_sa,"%s%s", me, "ME DETACH" );
      break;
    case CGEREP_EVENT_NW_CLASS:
      sprintf(g_sa,"%s%s", me, "NW CLASS" );
      break;
    case CGEREP_EVENT_ME_CLASS:
      sprintf(g_sa,"%s%s", me, "ME CLASS" );
      break;
    case CGEREP_EVENT_NW_ACT:
      sprintf(g_sa,"%s%s \"%s\",\"%s\",%hd", me, "NW ACT", param->act.pdp_type, pdp_ip_addr_str, param->act.cid);
        break;
    case CGEREP_EVENT_ME_ACT:
      sprintf(g_sa,"%s%s \"%s\",\"%s\",%hd", me, "ME ACT", param->act.pdp_type, pdp_ip_addr_str, param->act.cid);
        break;
  }

  io_sendIndication(srcId, g_sa, ATI_FORCED_OUTPUT);  
}


/*
+--------------------------------------------------------------------+
| PROJECT : GPRS (8441)                 MODULE  : GACI_RET           |
| STATE   : code                        ROUTINE : rCI_PlusCGREG      |
+--------------------------------------------------------------------+

  PURPOSE : handles rCI_PlusCGREG call back

*/
GLOBAL void rCI_PlusCGREG ( T_CGREG_STAT stat, USHORT lac, USHORT ci )
{
  U8    rt       = 0;
  TRACE_FUNCTION("rCI_PlusCGREG()");

  r_plus_percent_CREG  ( srcId_cb,
                         stat,
                         lac,
                         ci,
                         PlusCGREG_CMD,
                         P_CREG_GPRS_Support_Unknown, /*ACI-SPR-17218: ignored*/
                         rt,
                         FALSE );
}

/*
+--------------------------------------------------------------------+
| PROJECT : GPRS (8441)                 MODULE  : GACI_RET           |
| STATE   : code                        ROUTINE : rCI_changedQOS     |
+--------------------------------------------------------------------+

  PURPOSE : dummy function

*/
GLOBAL void rCI_changedQOS    ( SHORT cid, T_PS_qos *qos )
{
}

/*
+--------------------------------------------------------------------+
| PROJECT : GPRS (8441)                 MODULE  : GACI_RET           |
| STATE   : code                        ROUTINE : rCI_PercentSNCNT   |
+--------------------------------------------------------------------+

  PURPOSE : handles rCI_PercentSNCNT call back

*/
GLOBAL void rCI_PercentSNCNT ( UBYTE c_id,
                               ULONG octets_uplink,
                               ULONG octets_downlink,
                               ULONG packets_uplink,
                               ULONG packets_downlink )
{
  TRACE_FUNCTION("rCI_PercentSNCNT()");
  
  sprintf(g_sa,"%s: %d, %d, %d, %d, %d", "%SNCNT", c_id,
                                         octets_uplink, octets_downlink, 
                                         packets_uplink, packets_downlink);
      
  io_sendMessage(srcId_cb, g_sa, ATI_NORMAL_OUTPUT);

}

/*
+--------------------------------------------------------------------+
| PROJECT : GPRS (8441)                 MODULE  : GACI_RET           |
| STATE   : code                        ROUTINE : rCI_PercentCGREG   |
+--------------------------------------------------------------------+

  PURPOSE : handles rCI_PercentCGREG call back

*/
GLOBAL void rCI_PercentCGREG ( T_P_CGREG_STAT stat, USHORT lac, USHORT ci, BOOL bActiveContext )
{
  U8    rt       = 0;
  TRACE_FUNCTION("rCI_PercentCGREG()");

  r_plus_percent_CREG  ( srcId_cb,
                         stat,
                         lac,
                         ci,
                         PercentCGREG_CMD,
                         P_CREG_GPRS_Support_Unknown, /*ACI-SPR-17218: ignored*/
                         rt,
                         bActiveContext);

}


#ifdef REL99
/*
+--------------------------------------------------------------------+
| PROJECT : UMTS                        MODULE  : GACI_RET           |
| STATE   : code                        ROUTINE : rCI_PlusCGCMOD     |
+--------------------------------------------------------------------+

  PURPOSE : handles rCI_PlusCGCMOD call back

*/
GLOBAL void rCI_PlusCGCMOD ( void )
{
  UBYTE srcId = srcId_cb;
  T_ATI_SRC_PARAMS *src_params = find_element (ati_src_list, srcId, search_ati_src_id);

  TRACE_FUNCTION("rCI_PlusCGCMOD()");

  src_params->curAtCmd = AT_CMD_NONE;
}

/*
+--------------------------------------------------------------------+
| PROJECT : UMTS                        MODULE  : GACI_RET           |
| STATE   : code                        ROUTINE : rAT_PlusCGCMOD     |
+--------------------------------------------------------------------+

  PURPOSE : handles rAT_PlusCGCMOD call back

*/
GLOBAL void rAT_PlusCGCMOD ( void )
{
  UBYTE srcId = srcId_cb;
  T_ATI_SRC_PARAMS *src_params = find_element (ati_src_list, srcId, search_ati_src_id);

  TRACE_FUNCTION("rAT_PlusCGCMOD()");

  src_params->curAtCmd = AT_CMD_NONE;
}

#endif /*REL99 */

/* sbh...15/01/02.  If MFW defined, then these functions are duplicated */

#if !defined(MFW) && !defined(FF_MMI_RIV)

/*
+--------------------------------------------------------------------+
| PROJECT : GPRS (8441)                 MODULE  : GACI_RET           |
| STATE   : code                        ROUTINE : rAT_PlusCGACT      |
+--------------------------------------------------------------------+

  PURPOSE : handles rAT_PlusCGACT call back

*/
GLOBAL void rAT_PlusCGACT   ( SHORT link_id )
{

  TRACE_FUNCTION("rAT_PlusCGACT()");


}

/*
+--------------------------------------------------------------------+
| PROJECT : GPRS (8441)                 MODULE  : GACI_RET           |
| STATE   : code                        ROUTINE : rAT_PlusCGDATA     |
+--------------------------------------------------------------------+

  PURPOSE : handles rAT_PlusCGDATA call back

*/
GLOBAL void rAT_PlusCGDATA   ( SHORT link_id )
{

  TRACE_FUNCTION("rAT_PlusCGDATA()");


}

/*
+--------------------------------------------------------------------+
| PROJECT : GPRS (8441)                 MODULE  : GACI_RET           |
| STATE   : code                        ROUTINE : rAT_PlusCGANS      |
+--------------------------------------------------------------------+

  PURPOSE : handles rAT_PlusCGANS call back

*/
GLOBAL void rAT_PlusCGANS ( SHORT link_id )
{

  TRACE_FUNCTION("rAT_PlusCGANS()");


}

/*
+--------------------------------------------------------------------+
| PROJECT : GPRS (8441)                 MODULE  : GACI_RET           |
| STATE   : code                        ROUTINE : rAT_PlusCGEREP     |
+--------------------------------------------------------------------+

  PURPOSE : handles rAT_PlusCGEREP call back

*/
GLOBAL void rAT_PlusCGEREP ( T_CGEREP_EVENT event, T_CGEREP_EVENT_REP_PARAM *param )
{

  TRACE_FUNCTION("rAT_PlusCGEREP()");


}

/*
+--------------------------------------------------------------------+
| PROJECT : GPRS (8441)                 MODULE  : GACI_RET           |
| STATE   : code                        ROUTINE : rAT_PercentCGEV     |
+--------------------------------------------------------------------+

  PURPOSE : handles rAT_PercentCGEV call back

*/
GLOBAL void rAT_PercentCGEV (T_CGEREP_EVENT event, T_CGEREP_EVENT_REP_PARAM *param )
{

  TRACE_FUNCTION("rAT_PercentCGEV()");

}
/*
+--------------------------------------------------------------------+
| PROJECT : GPRS (8441)                 MODULE  : GACI_RET           |
| STATE   : code                        ROUTINE : rAT_PlusCGREG      |
+--------------------------------------------------------------------+

  PURPOSE : handles rAT_PlusCGREG call back

*/
GLOBAL void rAT_PlusCGREG ( T_CGREG_STAT stat, USHORT lac, USHORT ci )
{

  TRACE_FUNCTION("rAT_PlusCGREG()");


}
/*
+--------------------------------------------------------------------+
| PROJECT : GPRS (8441)                 MODULE  : GACI_RET           |
| STATE   : code                        ROUTINE : rAT_changedQOS     |
+--------------------------------------------------------------------+

  PURPOSE : inform over the network initiated QOS modification

*/
GLOBAL void rAT_changedQOS( U8 cid, T_PS_qos *qos )
{

  TRACE_FUNCTION("rAT_changedQOS()");


}

/*
+--------------------------------------------------------------------+
| PROJECT : GPRS (8441)                 MODULE  : GACI_RET           |
| STATE   : code                        ROUTINE : rCI_PercentSNCNT   |
+--------------------------------------------------------------------+

  PURPOSE : handles rCI_PercentSNCNT call back

*/
GLOBAL void rAT_PercentSNCNT ( UBYTE c_id,
                               ULONG octets_uplink,
                               ULONG octets_downlink,
                               ULONG packets_uplink,
                               ULONG packets_downlink )
{
  TRACE_FUNCTION("rAT_PercentSNCNT()");

}


/*
+--------------------------------------------------------------------+
| PROJECT : GPRS (8441)                 MODULE  : GACI_RET           |
| STATE   : code                        ROUTINE : rAT_PercentCGREG      |
+--------------------------------------------------------------------+

  PURPOSE : handles rAT_PercentCGREG call back

*/
GLOBAL void rAT_PercentCGREG ( T_P_CGREG_STAT stat, USHORT lac, USHORT ci, BOOL bActiveContext )
{

  TRACE_FUNCTION("rAT_PercentCGREG()");


}



#endif /* MFW || FF_MMI_RIV */


#endif  /* GPRS */
