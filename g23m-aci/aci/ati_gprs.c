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
|  Purpose :  GPRS AT Command Handler.
+-----------------------------------------------------------------------------
*/

#if defined (GPRS) && defined (DTI)

#ifndef GACI_CMD_C
#define GACI_CMD_C
#endif

#undef DUMMY_ATI_STRINGS

#include "aci_all.h"

/*includes*/
#include "dti.h"      /* functionality of the dti library */
#include "aci_cmh.h"
#include "ati_cmd.h"
#include "aci_io.h"
#include "aci_cmd.h"
#include "aci_lst.h"
#include "l4_tim.h"

#include "gdi.h"
#include "audio.h"
#include "p_sim.h"
#include "p_aci.h"
#include "aoc.h"
#include "aci.h"
#include "pcm.h"
#include "rx.h"
#include "pwr.h"

#include "dti_conn_mng.h"
#include "dti_cntrl_mng.h"

#include "gaci.h"
#include "gaci_cmh.h"
#include "psa.h"
#include "psa_sm.h"

#include "cmh.h"
#include "cmh_sm.h"

#include "aci_io.h"
#include "aci_mem.h"

#if defined(FF_WAP) OR defined (FF_GPF_TCPIP)  OR defined (FF_SAT_E)
#include "gaci_srcc.h"
#include "psa_tcpip.h"
#include "wap_aci.h"
#endif /* defined(FF_WAP) OR defined (FF_GPF_TCPIP)  OR defined (FF_SAT_E)*/

#include "gaci_cmd.h"
#include "cmh_gppp.h"
#include "psa_gppp.h"
#include "aci_prs.h"

#include "ati_int.h"

#include "dcm_f.h"

#ifdef FF_ATI_BAT

#include "typedefs.h"
#include "gdd.h"
#include "bat.h"

#include "ati_bat.h"

#endif /*FF_ATI_BAT*/

EXTERN char       *cmdErrStr;
EXTERN T_ACI_LIST *ati_src_list;

GLOBAL T_ATI_RSLT setatPlusCGDCONT (char *cl, UBYTE srcId);
GLOBAL T_ATI_RSLT setatPlusCGQREQ  (char *cl, UBYTE srcId);
GLOBAL T_ATI_RSLT setatPlusCGQMIN  (char *cl, UBYTE srcId);
GLOBAL T_ATI_RSLT setatPlusCGATT   (char *cl, UBYTE srcId);
GLOBAL T_ATI_RSLT setatPlusCGACT   (char *cl, UBYTE srcId);
GLOBAL T_ATI_RSLT setatPlusCGDATA  (char *cl, UBYTE srcId);
GLOBAL T_ATI_RSLT setatPlusCGPADDR (char *cl, UBYTE srcId);
GLOBAL T_ATI_RSLT setatPlusCGAUTO  (char *cl, UBYTE srcId);
GLOBAL T_ATI_RSLT setatPlusCGANS   (char *cl, UBYTE srcId);
GLOBAL T_ATI_RSLT setatPlusCGCLASS (char *cl, UBYTE srcId);
GLOBAL T_ATI_RSLT setatPlusCGEREP  (char *cl, UBYTE srcId);
GLOBAL T_ATI_RSLT setatPlusCGREG   (char *cl, UBYTE srcId);
GLOBAL T_ATI_RSLT setatPlusCGSMS   (char *cl, UBYTE srcId);
GLOBAL T_ATI_RSLT setatPercentCGEREP  (char *cl, UBYTE srcId);


GLOBAL T_ATI_RSLT queatPlusCGDCONT (char *cl, UBYTE srcId);
GLOBAL T_ATI_RSLT queatPlusCGQREQ  (char *cl, UBYTE srcId);
GLOBAL T_ATI_RSLT queatPlusCGQMIN  (char *cl, UBYTE srcId);
GLOBAL T_ATI_RSLT queatPlusCGATT   (char *cl, UBYTE srcId);
GLOBAL T_ATI_RSLT queatPlusCGACT   (char *cl, UBYTE srcId);
GLOBAL T_ATI_RSLT queatPlusCGAUTO  (char *cl, UBYTE srcId);
GLOBAL T_ATI_RSLT queatPlusCGCLASS (char *cl, UBYTE srcId);
GLOBAL T_ATI_RSLT queatPlusCGEREP  (char *cl, UBYTE srcId);
GLOBAL T_ATI_RSLT queatPlusCGREG   (char *cl, UBYTE srcId);
GLOBAL T_ATI_RSLT queatPlusCGSMS   (char *cl, UBYTE srcId);
GLOBAL T_ATI_RSLT queatPercentCGEREP  (char *cl, UBYTE srcId);


GLOBAL T_ATI_RSLT tesatPlusCGDCONT (char *cl, UBYTE srcId);
GLOBAL T_ATI_RSLT tesatPlusCGPADDR (char *cl, UBYTE srcId);
GLOBAL T_ATI_RSLT tesatPlusCGSMS   (char *cl, UBYTE srcId);

GLOBAL T_ATI_RSLT atGD (char *cl, UBYTE srcId, BOOL *gprs_command);

GLOBAL T_ATI_RSLT setatPercentCGAATT (char *cl, UBYTE srcId);
GLOBAL T_ATI_RSLT queatPercentCGAATT (char *cl, UBYTE srcId);

GLOBAL T_ATI_RSLT setatPercentCGREG (char *cl, UBYTE srcId);
GLOBAL T_ATI_RSLT queatPercentCGREG (char *cl, UBYTE srcId);
LOCAL T_ATI_RSLT queatPercentCGPCO (UBYTE srcId,USHORT cid);
GLOBAL T_ATI_RSLT setatPercentCGCLASS (CHAR *cl, UBYTE srcId);
GLOBAL T_ATI_RSLT queatPercentCGCLASS (CHAR *cl, UBYTE srcId);
#ifdef REL99
GLOBAL T_ATI_RSLT queatPlusCGEQREQ (char *cl, UBYTE srcId);
GLOBAL T_ATI_RSLT queatPlusCGEQMIN (char *cl, UBYTE srcId);
GLOBAL T_ATI_RSLT queatPlusCGTFT   (char *cl, UBYTE srcId);
GLOBAL T_ATI_RSLT queatPlusCGDSCONT(char *cl, UBYTE srcId);
GLOBAL T_ATI_RSLT tesatPlusCGEQNEG (char *cl, UBYTE srcId);
GLOBAL T_ATI_RSLT tesatPlusCGCMOD  (char *cl, UBYTE srcId);
GLOBAL T_ATI_RSLT tesatPlusCGTFT   (char *cl, UBYTE srcId);
GLOBAL T_ATI_RSLT tesatPlusCGDSCONT(char *cl, UBYTE srcId);
GLOBAL T_ATI_RSLT setatPlusCGEQMIN (char *cl, UBYTE srcId);
GLOBAL T_ATI_RSLT setatPlusCGEQREQ (char *cl, UBYTE srcId);
GLOBAL T_ATI_RSLT setatPlusCGEQNEG (char *cl, UBYTE srcId);
GLOBAL T_ATI_RSLT setatPlusCGCMOD  (char *cl, UBYTE srcId);
GLOBAL T_ATI_RSLT setatPlusCGTFT   (char *cl, UBYTE srcId);
GLOBAL T_ATI_RSLT setatPlusCGDSCONT(char *cl, UBYTE srcId);
#endif

EXTERN char *parseDSNP_U16      (char blocks, char *p_in, U16 *p_out);
EXTERN char *parseIPv4Address(char *p_in, U8 * p_ip_addr, U8 * is_ip_valid);
EXTERN char *parseIPv6Address(char *p_in, U8 * p_ip_addr, U8 * is_ip_valid);

EXTERN T_PDP_CONTEXT_INTERNAL *p_pdp_context_list;


/*
+------------------------------------------------------------------------------------+
| PROJECT : GPRS (8441)                 MODULE  : GACI_CMD                           |
| STATE   : finnished                   ROUTINE : gaci_ati_cmd_init_each_source      |
+------------------------------------------------------------------------------------+

  PURPOSE : initialize GACI relevant ATI parameters.
*/

GLOBAL void gaci_ati_cmd_init_each_source( UBYTE srcId )
{
  /* init +CGREG param */
  ati_creg_init(srcId, PlusCGREG_CMD);

  /* init %CGREG param */
  ati_creg_init(srcId, PercentCGREG_CMD);

  /* init %CGEREP param */
  ati_user_output_cfg[srcId].Percent_CGEREP_stat = 0;
  ati_user_output_cfg[srcId].CGEREP_mode = 0;
  ati_user_output_cfg[srcId].CGEREP_bfr = 0;
}

/*
+--------------------------------------------------------------------+
| PROJECT : GPRS (8441)                 MODULE  : GACI_CMD           |
| STATE   : finnished                   ROUTINE : atPlusCGDCONT      |
+--------------------------------------------------------------------+

  PURPOSE : +CGDCONT  command ( PDP context definition )
*/

#define MAGIC_EMPTY_APN "\x0ff"

GLOBAL T_ATI_RSLT setatPlusCGDCONT  (char *cl, UBYTE srcId)
{
  T_ACI_RETURN ret = AT_FAIL;
  U8            valid_ip;
  U8            cid          = PDP_CONTEXT_CID_OMITTED;  
  char         *p_pdp_addr   = NULL;
  USHORT        pdp_addr_len = 0;
  
  T_PDP_CONTEXT pdp_context_input = { "",
                                      MAGIC_EMPTY_APN,
                                      NAS_is_ip_not_present, 0,
                                      PDP_CONTEXT_D_COMP_OMITTED, 
                                      PDP_CONTEXT_H_COMP_OMITTED,
                                      PDP_CONTEXT_CID_OMITTED };

  
  T_ATI_SRC_PARAMS *src_params = find_element (ati_src_list, srcId, search_ati_src_id);
  TRACE_FUNCTION("setatPlusCGDCONT ()");

  cl = parse( cl, "csslcc", &cid,
                            (LONG)MAX_PDP_CONTEXT_TYPE_LEN, &pdp_context_input.pdp_type,
                            (LONG)MAX_PDP_CONTEXT_APN_LEN,  &pdp_context_input.pdp_apn,
                            &pdp_addr_len,                  &p_pdp_addr,
                            &pdp_context_input.d_comp,
                            &pdp_context_input.h_comp);

  if (!strcmp(pdp_context_input.pdp_apn, MAGIC_EMPTY_APN))
  {
    /* if APN was ommitted then parse leaves the string untouched */
    pdp_context_input.pdp_apn[0]='\0';    /* so clear the apn */
  }
  else if (pdp_context_input.pdp_apn[0] EQ '\0')
  {
    /* if entered APN was "", parse clears the buffer */
    strcpy(pdp_context_input.pdp_apn, MAGIC_EMPTY_APN);  /* fill magic empty apn */
  }

  if ( !cl )
  {
    cmdCmeError(CME_ERR_OpNotAllow);
    return (ATI_FAIL);
  }

  strupper( (char*) &pdp_context_input.pdp_type);

  /* Issue 27081 - GPRS context is transformed from lower cases into upper cases */
  /* strupper( (char*) &pdp_context_input.pdp_apn ); */

  if( p_pdp_addr )
  {
    if( pdp_addr_len >= 7 AND pdp_addr_len <= 15 )
    {
      p_pdp_addr = parseIPv4Address( p_pdp_addr, (U8*) &(pdp_context_input.pdp_addr.ip_address.ipv4_addr.a4), &valid_ip);
      if (valid_ip NEQ 0)
      {
        pdp_context_input.pdp_addr.ctrl_ip_address = NAS_is_ipv4;
      }
      else
      {
        /* If the IP address is not valid then we should send ERROR message. */
        cmdCmeError(CME_ERR_OpNotAllow);
        return (ATI_FAIL);
      }
    }
    else
    {
      if( pdp_addr_len >= 31 AND pdp_addr_len <= 63 )
      {
        p_pdp_addr = parseIPv6Address( p_pdp_addr, (U8*) &(pdp_context_input.pdp_addr.ip_address.ipv6_addr.a6), &valid_ip);
        if (valid_ip NEQ 0)
        {
          pdp_context_input.pdp_addr.ctrl_ip_address = NAS_is_ipv6;
        }
        else
        {
          /* If the IP address is not valid then we should send ERROR message. */
          cmdCmeError(CME_ERR_OpNotAllow);
          return (ATI_FAIL);
        }
      }
      else
      {
        cmdCmeError(CME_ERR_OpNotAllow);
        return (ATI_FAIL);
      }
    }
  }
  else
  {
    if( pdp_addr_len NEQ 0 )
    {
      cmdCmeError(CME_ERR_OpNotAllow);
      return (ATI_FAIL);
    }
    else
    {
      //if pdp_addr is omitted, ctrl must be set to not present...
      pdp_context_input.pdp_addr.ctrl_ip_address = NAS_is_ip_not_present;
    }
  }

#ifdef FF_ATI_BAT
  {
    T_BAT_cmd_send cmd;
    T_BAT_cmd_set_plus_cgdcont cgdcont;

    cmd.ctrl_params = BAT_CMD_SET_PLUS_CGDCONT;
    cmd.params.ptr_set_plus_cgdcont = &cgdcont;

    cgdcont.cid=(T_BAT_pdp_cid)cid;

    /* currently both IPv4,IPV6 supported */
    if (!utl_strcasecmp((const char *)pdp_context_input.pdp_type, 
                        (const char *)"ip"))
      cgdcont.pdp_type = BAT_PDP_TYPE_IPV4;
    else if (!utl_strcasecmp((const char *)pdp_context_input.pdp_type,
                             (const char *)"ipv6"))
      cgdcont.pdp_type = BAT_PDP_TYPE_IPV6;
    else
      cgdcont.pdp_type = BAT_PDP_TYPE_NOT_PRESENT;
    
    /* apn value = "" */
    if (!strcmp(pdp_context_input.pdp_apn, MAGIC_EMPTY_APN))
    {
      cgdcont.v_apn = TRUE;
      cgdcont.c_apn = 0;
    }
    /* apn value omitted */
    else if (pdp_context_input.pdp_apn[0] EQ '\0')
    {
      cgdcont.v_apn = FALSE;
    }
    /* apn with valid value (longer than 0) */
    else
    {
      cgdcont.v_apn = TRUE;
      cgdcont.c_apn = (U8)strlen(pdp_context_input.pdp_apn);
      memcpy(cgdcont.apn,pdp_context_input.pdp_apn, cgdcont.c_apn);
    }
    
        
    if (pdp_addr_len)
    {
      cgdcont.v_pdp_addr = TRUE;
      cgdcont.c_pdp_addr = (U8)pdp_addr_len;
      memcpy(cgdcont.pdp_addr,p_pdp_addr,pdp_addr_len);
    }
    else
    {
      cgdcont.v_pdp_addr = FALSE;
      cgdcont.c_pdp_addr = 0;
    }
    
    cgdcont.d_comp = (T_BAT_plus_cgdcont_d_comp)pdp_context_input.d_comp;
    cgdcont.h_comp = (T_BAT_plus_cgdcont_h_comp)pdp_context_input.h_comp;

    bat_send(ati_bat_get_client(srcId), &cmd);
    src_params->curAtCmd=AT_CMD_CGDCONT;
    return(ATI_EXCT);
  }
#else
  ret = sAT_PlusCGDCONT((T_ACI_CMD_SRC)srcId, cid, &pdp_context_input );
  switch (ret)
  {
    case (AT_CMPL):                         /*operation completed*/
      return (ATI_CMPL);
    case (AT_EXCT):
      src_params->curAtCmd    = AT_CMD_CGDCONT;
      return (ATI_EXCT);
    default:
      cmdCmeError(CME_ERR_Unknown);         /*Command failed*/
      return (ATI_FAIL);
  }
#endif
}

GLOBAL T_ATI_RSLT queatPlusCGDCONT  (char *cl, UBYTE srcId)
{
#ifndef FF_ATI_BAT
  T_ACI_RETURN ret = AT_FAIL;
  T_PDP_CONTEXT pdp_context_array[PDP_CONTEXT_CID_MAX];
  SHORT         pdp_context_cid_array[PDP_CONTEXT_CID_MAX];
  char          pdp_addr[64];
  char          message[255];
  int           i=0;
#endif

  TRACE_FUNCTION("queatPlusCGDCONT ()");

#ifdef FF_ATI_BAT
  {
    T_BAT_cmd_send cmd;
    T_BAT_no_parameter dummy;

    cmd.ctrl_params = BAT_CMD_QUE_PLUS_CGDCONT;
    dummy.bat_dummy = 0xFF;
    cmd.params.ptr_que_plus_cgdcont = &dummy;
    bat_send(ati_bat_get_client(srcId), &cmd);
    return(ATI_EXCT);
  }
#else

  memset( &pdp_context_array, 0, sizeof(pdp_context_array) );
  memset( &pdp_context_cid_array, PDP_CONTEXT_CID_OMITTED, sizeof(pdp_context_cid_array) );

  ret = qAT_PlusCGDCONT((T_ACI_CMD_SRC)srcId, pdp_context_array, pdp_context_cid_array);

  if (ret EQ AT_CMPL)
  {
    for ( i = 0; i < PDP_CONTEXT_CID_MAX AND pdp_context_cid_array[i] NEQ PDP_CONTEXT_CID_OMITTED; i++ )
    {
      if( pdp_context_addr_omitted(&(pdp_context_array[i].pdp_addr) ) )
      {
        sprintf( pdp_addr,"\"\"");
      }
      else
      {
        if( pdp_context_array[i].pdp_addr.ctrl_ip_address EQ NAS_is_ipv4 )
        {
          /* IP v4 address */
          sprintf( pdp_addr, "\"%d.%d.%d.%d\"", 
                             pdp_context_array[i].pdp_addr.ip_address.ipv4_addr.a4[0],
                             pdp_context_array[i].pdp_addr.ip_address.ipv4_addr.a4[1],
                             pdp_context_array[i].pdp_addr.ip_address.ipv4_addr.a4[2],
                             pdp_context_array[i].pdp_addr.ip_address.ipv4_addr.a4[3] );
        }
        else
        {
          /* IP v6 address */
          sprintf( pdp_addr, "\"%d.%d.%d.%d.%d.%d.%d.%d.%d.%d.%d.%d.%d.%d.%d.%d\"", 
                             pdp_context_array[i].pdp_addr.ip_address.ipv6_addr.a6[0],
                             pdp_context_array[i].pdp_addr.ip_address.ipv6_addr.a6[1],
                             pdp_context_array[i].pdp_addr.ip_address.ipv6_addr.a6[2],
                             pdp_context_array[i].pdp_addr.ip_address.ipv6_addr.a6[3],
                             pdp_context_array[i].pdp_addr.ip_address.ipv6_addr.a6[4],
                             pdp_context_array[i].pdp_addr.ip_address.ipv6_addr.a6[5],
                             pdp_context_array[i].pdp_addr.ip_address.ipv6_addr.a6[6],
                             pdp_context_array[i].pdp_addr.ip_address.ipv6_addr.a6[7],
                             pdp_context_array[i].pdp_addr.ip_address.ipv6_addr.a6[8],
                             pdp_context_array[i].pdp_addr.ip_address.ipv6_addr.a6[9],
                             pdp_context_array[i].pdp_addr.ip_address.ipv6_addr.a6[10],
                             pdp_context_array[i].pdp_addr.ip_address.ipv6_addr.a6[11],
                             pdp_context_array[i].pdp_addr.ip_address.ipv6_addr.a6[12],
                             pdp_context_array[i].pdp_addr.ip_address.ipv6_addr.a6[13],
                             pdp_context_array[i].pdp_addr.ip_address.ipv6_addr.a6[14],
                             pdp_context_array[i].pdp_addr.ip_address.ipv6_addr.a6[15] );
        }
        
      }

      if( pdp_context_apn_omitted( pdp_context_array[i].pdp_apn ) )
      {
        sprintf( message, "+CGDCONT: %d,\"%s\",,%s,%d,%d",
                          pdp_context_cid_array[i],
                          pdp_context_array[i].pdp_type,
                          pdp_addr,
                          pdp_context_array[i].d_comp,
                          pdp_context_array[i].h_comp );
      }
      else if (!strcmp(pdp_context_array[i].pdp_apn, MAGIC_EMPTY_APN)) 
      { /* special empty APN detected, printout "" */
        sprintf( message, "+CGDCONT: %d,\"%s\",\"\",%s,%d,%d",
                          pdp_context_cid_array[i],
                          pdp_context_array[i].pdp_type,
                          pdp_addr,
                          pdp_context_array[i].d_comp,
                          pdp_context_array[i].h_comp );
      }
      else
      {
        sprintf( message, "+CGDCONT: %d,\"%s\",\"%s\",%s,%d,%d",
                          pdp_context_cid_array[i],
                          pdp_context_array[i].pdp_type,
                          pdp_context_array[i].pdp_apn,
                          pdp_addr,
                          pdp_context_array[i].d_comp,
                          pdp_context_array[i].h_comp );
      }
                        
      io_sendMessage(srcId, message, ATI_NORMAL_OUTPUT);

    }
  }
  else
  {
    cmdCmeError(CME_ERR_Unknown);
    return (ATI_FAIL);
  }

  return (ATI_CMPL);
#endif /*FF_ATI_BAT*/
}


/*
+--------------------------------------------------------------------+
| PROJECT :                             MODULE  : GACI_CMD           |
| STATE   : finnished                   ROUTINE : testatPlusCGDCONT  |
+--------------------------------------------------------------------+

  PURPOSE : +CGQREQ  command (context QOS request)
*/

GLOBAL T_ATI_RSLT tesatPlusCGDCONT  (char *cl, UBYTE srcId)
{
  char *me = "+CGDCONT: ";

  sprintf( g_sa, "%s(1-%d),\"IP\",,,(0),(0-3)", me, PDP_CONTEXT_CID_MAX );
  io_sendMessage( srcId, g_sa, ATI_NORMAL_OUTPUT );

  sprintf( g_sa, "%s(1-%d),\"IPV6\",,,(0),(0-3)", me, PDP_CONTEXT_CID_MAX );
  io_sendMessage( srcId, g_sa, ATI_NORMAL_OUTPUT );
  
  /* As for EDGE we are not supporting PDP_Type as "PPP" */
/*  sprintf( g_sa, "%s(1-%d),\"PPP\",,,(0-2),(0-3)", me, PDP_CONTEXT_CID_MAX );
  io_sendMessage( srcId, g_sa, ATI_NORMAL_OUTPUT );*/

  return (ATI_CMPL);
}

#ifdef REL99
/*
+--------------------------------------------------------------------+
| PROJECT : UMTS                        MODULE  : ATI_GPRS           |
| STATE   :                             ROUTINE : atPlusCGDSCONT     |
+--------------------------------------------------------------------+

  PURPOSE : +CGDSCONT command ( secondary PDP context definition )
*/

GLOBAL T_ATI_RSLT setatPlusCGDSCONT( char *cl, UBYTE srcId )
{

  U8 cid = PDP_CONTEXT_CID_OMITTED;

 
  T_PDP_CONTEXT pdp_context_input = { "",
                                      "", 
                                      NAS_is_ip_not_present,0,
                                      PDP_CONTEXT_D_COMP_OMITTED, 
                                      PDP_CONTEXT_H_COMP_OMITTED,
                                      PDP_CONTEXT_CID_OMITTED }; 
  T_ATI_SRC_PARAMS *src_params = find_element (ati_src_list, srcId, search_ati_src_id);
  TRACE_FUNCTION("setatPlusCGDSCONT ()");

  cl = parse( cl, "cccc", 
                  &cid,
                  &(pdp_context_input.p_cid),
                  &pdp_context_input.d_comp,
                  &pdp_context_input.h_comp);

  if( !cl )
  {
    cmdCmeError(CME_ERR_OpNotAllow);
    return (ATI_FAIL);
  }

#ifdef FF_ATI_BAT
  {
    T_BAT_cmd_send cmd;
    T_BAT_cmd_set_plus_cgdscont cgdscont;
    
    cmd.ctrl_params  = BAT_CMD_SET_PLUS_CGDSCONT;
    cmd.params.ptr_set_plus_cgdscont = &cgdscont;

    cgdscont.cid=(T_BAT_pdp_cid)cid;
    cgdscont.p_cid = (U8)pdp_context_input.p_cid;

    cgdscont.d_comp = (T_BAT_plus_cgdcont_d_comp)pdp_context_input.d_comp;
    cgdscont.h_comp = (T_BAT_plus_cgdcont_d_comp)pdp_context_input.h_comp;

    bat_send(ati_bat_get_client(srcId), &cmd);
    src_params->curAtCmd=AT_CMD_CGDSCONT;
    return(ATI_EXCT);
}
#else
  switch( sAT_PlusCGDSCONT(( T_ACI_CMD_SRC)srcId, cid , &pdp_context_input) )
  {
    case (AT_CMPL):                   /*operation completed*/
      return (ATI_CMPL);
    case (AT_EXCT):     
//      curAtCmd    = AT_CMD_CGDSCONT;  /*operation in progress*/
      return (ATI_EXCT);
    default:
      cmdCmeError(CME_ERR_Unknown);   /*Command failed*/
      return (ATI_FAIL);
  }


#endif /* FF_ATI_BAT */

}

/*
+--------------------------------------------------------------------+
| PROJECT : UMTS                        MODULE  : ATI_GPRS           |
| STATE   :                             ROUTINE : queatPlusCGDSCONT  |
+--------------------------------------------------------------------+

  PURPOSE : +CGDSCONT=? command
*/

GLOBAL T_ATI_RSLT queatPlusCGDSCONT( char *cl, UBYTE srcId )
{
  T_ACI_RETURN  ret;
  T_PDP_CONTEXT pdp_context_array[PDP_CONTEXT_CID_MAX];
  U8            pdp_context_cid_array[PDP_CONTEXT_CID_MAX];
  int           i;

  TRACE_FUNCTION("queatPlusCGDSCONT ()");
#ifdef FF_ATI_BAT
{
    T_BAT_cmd_send cmd;
    T_BAT_no_parameter dummy;

    cmd.ctrl_params = BAT_CMD_SET_PLUS_CGDSCONT;
    dummy.bat_dummy = 0xFF;
    cmd.params.ptr_set_plus_cgdscont = &dummy;
    bat_send(ati_bat_get_client(srcId), &cmd);
    return(ATI_EXCT);
}
#else

  memset( &pdp_context_cid_array, PDP_CONTEXT_CID_OMITTED, sizeof(pdp_context_cid_array) );

  ret = qAT_PlusCGDSCONT(( T_ACI_CMD_SRC)srcId, pdp_context_array, pdp_context_cid_array);

  if (ret EQ AT_CMPL)
  {
    for ( i = 0; i < PDP_CONTEXT_CID_MAX AND pdp_context_cid_array[i] NEQ PDP_CONTEXT_CID_OMITTED; i++ )
    {
      sprintf( g_sa, "+CGDSCONT: %d,%d,%d,%d",
                     pdp_context_cid_array[i],
                     pdp_context_array[i].p_cid,
                     pdp_context_array[i].d_comp,
                     pdp_context_array[i].h_comp );

      io_sendMessage(srcId, g_sa, ATI_NORMAL_OUTPUT);
    }
  }
  else
  {
    cmdCmeError(CME_ERR_Unknown);
    return (ATI_FAIL);
  }

  return (ATI_CMPL);
#endif /* FF_ATI_BAT */
}

/*
+--------------------------------------------------------------------+
| PROJECT : UMTS                        MODULE  : ATI_GPRS           |
| STATE   :                             ROUTINE : testatPlusCGDSCONT |
+--------------------------------------------------------------------+

  PURPOSE : +CGDSCONT?
*/

GLOBAL T_ATI_RSLT tesatPlusCGDSCONT( char *cl, UBYTE srcId )
{
  char me[50];
  char cid_list[PDP_CONTEXT_CID_MAX + 1];
  BOOL active = FALSE;
  U8   i        = 0;
  U8   cid;

  cid_list[0] = 0;

  me[0] = 0;

  for( cid = PDP_CONTEXT_CID_MIN; cid <= PDP_CONTEXT_CID_MAX; cid++ )
  {
    if( tAT_PlusCGDSCONT(( T_ACI_CMD_SRC) srcId, cid, &active ) EQ AT_CMPL )
    {
      if( active EQ TRUE )
      {
        cid_list[i] = cid;
        i++;
        cid_list[i] = 0;
      }      
    }
    else
    {
      cmdCmeError( CME_ERR_Unknown );
      return (ATI_FAIL);
    }
  }

  i = 0;
  sprintf( me, "+CGDSCONT: (1-%d),", PDP_CONTEXT_CID_MAX);

  if( cid_list[i] NEQ 0 )
  {
    sprintf( me, "%s(%d", me, cid_list[i] );
    i++;
  
    while( cid_list[i] NEQ 0 )
    {
      sprintf( me, "%s,%d", me, cid_list[i] );
      i++;
    }
  }

  if( cid_list[0] NEQ 0 )
  {
    sprintf( me, "%s)", me );
  }
  

  /* As for EDGE we are not supporting PDP_Type as "PPP" */
  /*sprintf( g_sa, "%s,\"PPP\",(0-1),(0-1)", me );
  io_sendMessage(srcId, g_sa, ATI_NORMAL_OUTPUT);*/

  sprintf( g_sa, "%s,\"IP\",(0),(0-1)", me );
  io_sendMessage(srcId, g_sa, ATI_NORMAL_OUTPUT);
  
  sprintf( g_sa, "%s,\"IPV6\",(0),(0-1)", me );
  io_sendMessage(srcId, g_sa, ATI_NORMAL_OUTPUT);

  return (ATI_CMPL);
}


/*
+--------------------------------------------------------------------+
| PROJECT : UMTS                        MODULE  : ATI_GPRS           |
| STATE   :                             ROUTINE : atPlusCGTFT        |
+--------------------------------------------------------------------+

  PURPOSE : +CGTFT command ( Traffic Flow Template definition )
*/

GLOBAL T_ATI_RSLT setatPlusCGTFT( char *cl, UBYTE srcId )
{
  U8       cid                         = 0;
  U8       protocol_or_next_header     = 0;
  U8       is_ip_valid                 = FALSE;
  U16      tos_value                   = 0;
  U16      tos_mask                    = 0;
  U32      ipsec_spi                   = 0;
  U32      flow_label                  = 0;

  char     *p_addr_n_mask_str          = NULL;
  char     *p_dest_port_range_str      = NULL;
  char     *p_src_port_range_str       = NULL;
  char     *p_tos_mask_or_tc_mask_str  = NULL;

  USHORT   addr_n_mask_str_len         = 0;
  USHORT   dest_port_range_str_len     = 0;
  USHORT   src_port_range_str_len      = 0;
  USHORT   tos_mask_or_tc_mask_str_len = 0;
  T_NAS_tft_pf  tft_pf_input;

#ifdef FF_ATI_BAT
  T_ATI_SRC_PARAMS *src_params = find_element (ati_src_list, srcId, search_ati_src_id);
#endif
    
  TRACE_FUNCTION("setatPlusCGTFT ()");

  memset( &tft_pf_input, 0x00, sizeof(T_NAS_tft_pf) );
  
//                 +------------  1: <cid>
//                 |+-----------  2: <packet filter identifier>
//                 ||+----------  3: <evaluation precedence index>
//                 |||+---------  4: <source address and sub met mask>
//                 ||||+--------  5: <protocol number (IPv4) / next header (IPv6)>
//                 |||||+-------  6: <destenation port range>
//                 ||||||+------  7: <source port range>
//                 |||||||+-----  8: <ipsec security parameter index (SPI)>
//                 ||||||||+----  9: <type of service and mask (IPv4) / traffic class and mask (IPv6)>
//                 |||||||||+--- 10: <flow label (IPv6)>
//                 ||||||||||    
  cl = parse( cl, "ccclcllyly",
                  &cid,                                                     /*  1 */
                  &tft_pf_input.tft_pf_id,                                  /*  2 */
                  &tft_pf_input.tft_pf_precedence,                          /*  3 */
                  &addr_n_mask_str_len,         &p_addr_n_mask_str,         /*  4 */
                  &protocol_or_next_header,                                 /*  5 */
                  &dest_port_range_str_len,     &p_dest_port_range_str,     /*  6 */
                  &src_port_range_str_len,      &p_src_port_range_str,      /*  7 */
                  &ipsec_spi,                                               /*  8 */
                  &tos_mask_or_tc_mask_str_len, &p_tos_mask_or_tc_mask_str, /*  9 */
                  &flow_label                                               /* 10 */
                  );

  if( !cl )
  {
    cmdCmeError(CME_ERR_OpNotAllow);
    return (ATI_FAIL);
  }

  if( !p_addr_n_mask_str )
  {
    /* IP address and mask is omitted */
      
    if( flow_label EQ 0 )
    {
      /* IP v4 type */
      tft_pf_input.ctrl_tft_pf_entry = NAS_is_tft_pf_ipv4;
    }
    else
    {
      /* IP v6 type */
      tft_pf_input.ctrl_tft_pf_entry  = NAS_is_tft_pf_ipv6;
      tft_pf_input.tft_pf_valid_bits |= NAS_TFT_ID_FLOW_LABEL;
    }
  }
  else
  {

    if( addr_n_mask_str_len >= 15 AND addr_n_mask_str_len <= 31 )
    {
      // The length of the string is within the length of an IP v4 string.

      if( flow_label EQ 0 )
      {
        tft_pf_input.ctrl_tft_pf_entry = NAS_is_tft_pf_ipv4;
            
        // Parse IP v4 address 
        p_addr_n_mask_str = parseIPv4Address( p_addr_n_mask_str,
                                              (U8*) &(tft_pf_input.tft_pf_entry.tft_pf_ipv4.tft_ipv4_src_addr_mask.tft_ipv4_addr), &is_ip_valid );
        // Parse IP v4 sub net mask
        p_addr_n_mask_str = parseIPv4Address( p_addr_n_mask_str,
                                                (U8*) &(tft_pf_input.tft_pf_entry.tft_pf_ipv4.tft_ipv4_src_addr_mask.tft_ipv4_mask), &is_ip_valid);
      
        tft_pf_input.tft_pf_valid_bits |= NAS_TFT_ID_IPv4_SRC_ADDR_MASK;
      }
      else
      {
        cmdCmeError(CME_ERR_OpNotAllow);
        return (ATI_FAIL);
      }
    }
    else
    {
      if( addr_n_mask_str_len >= 63 AND addr_n_mask_str_len <= 127 )
      {
        // The length of the string is within the length of an IP v6 string.

        tft_pf_input.ctrl_tft_pf_entry = NAS_is_tft_pf_ipv6;
        tft_pf_input.tft_pf_entry.tft_pf_ipv6.tft_flow_label = flow_label;

        // Parse IP v6 address 
        p_addr_n_mask_str = parseIPv6Address( p_addr_n_mask_str,
                                              (U8*) &(tft_pf_input.tft_pf_entry.tft_pf_ipv6.tft_ipv6_src_addr_mask.tft_ipv6_addr), &is_ip_valid );
        // Parse IP v4 sub net mask
        p_addr_n_mask_str = parseIPv6Address( p_addr_n_mask_str,
                                              (U8*) &(tft_pf_input.tft_pf_entry.tft_pf_ipv6.tft_ipv6_src_addr_mask.tft_ipv6_mask), &is_ip_valid );
          
        tft_pf_input.tft_pf_valid_bits |= NAS_TFT_ID_IPv6_SRC_ADDR_MASK;
      }
      else
      {
        cmdCmeError(CME_ERR_OpNotAllow);
        return (ATI_FAIL);
      }
    }
      
    if( !p_addr_n_mask_str )
    {
      cmdCmeError(CME_ERR_OpNotAllow);
      return (ATI_FAIL);
    }
  } /* END: if( !p_addr_n_mask_str ) */

  switch( tft_pf_input.ctrl_tft_pf_entry )
  {
    /*** IP v4 parameters */
    case NAS_is_tft_pf_ipv4:
    {   
      if( protocol_or_next_header )
      {
        tft_pf_input.tft_pf_entry.tft_pf_ipv4.tft_protocol = protocol_or_next_header;
        tft_pf_input.tft_pf_valid_bits |= NAS_TFT_ID_PROTOCOL_OR_NEXT_HDR;        
      }
    
      /* Type of service and mask */
      if( p_tos_mask_or_tc_mask_str )
      {
        p_tos_mask_or_tc_mask_str = parseDSNP_U16( 1, p_tos_mask_or_tc_mask_str, 
                                                   &tos_value );

        tft_pf_input.tft_pf_entry.tft_pf_ipv4.tft_tos_and_mask.tos_value = (U8)tos_value;


        p_tos_mask_or_tc_mask_str = parseDSNP_U16( 1, p_tos_mask_or_tc_mask_str, 
                                                   &tos_mask );
        
        tft_pf_input.tft_pf_entry.tft_pf_ipv4.tft_tos_and_mask.tos_mask = (U8)tos_mask;
        
        tft_pf_input.tft_pf_valid_bits |= NAS_TFT_ID_TOS_AND_MASK;        
      }

      /* Destenation port */
      if( p_dest_port_range_str )
      {
        p_dest_port_range_str = parseDSNP_U16( 1, p_dest_port_range_str, 
                                               (U16*) &tft_pf_input.tft_pf_entry.tft_pf_ipv4.tft_dest_port_range.low_limit );
        p_dest_port_range_str = parseDSNP_U16( 1, p_dest_port_range_str, 
                                               (U16*) &tft_pf_input.tft_pf_entry.tft_pf_ipv4.tft_dest_port_range.high_limit );

        tft_pf_input.tft_pf_valid_bits |= NAS_TFT_ID_DEST_PORT_RANGE;
      }
        
      /* Source port */
      if( p_src_port_range_str )
      {
        p_src_port_range_str = parseDSNP_U16( 1, p_src_port_range_str, 
                                              (U16*)&tft_pf_input.tft_pf_entry.tft_pf_ipv4.tft_src_port_range.low_limit );
        p_src_port_range_str = parseDSNP_U16( 1, p_src_port_range_str, 
                                              (U16*)&tft_pf_input.tft_pf_entry.tft_pf_ipv4.tft_src_port_range.high_limit );

        tft_pf_input.tft_pf_valid_bits |= NAS_TFT_ID_SRC_PORT_RANGE;
      }
        
      /* IPSEC security parameter spi */
      if( ipsec_spi )
      {
        tft_pf_input.tft_pf_entry.tft_pf_ipv4.tft_ipsec_spi = ipsec_spi;
        tft_pf_input.tft_pf_valid_bits |= NAS_TFT_ID_IPSEC_SPI;
      }
      break;

    } /* END: case NAS_is_tft_pf_ipv4 */


    /*** IP v6 parameters ***/
    case NAS_is_tft_pf_ipv6:
    {
      /* Next header */
      if ( protocol_or_next_header )
      {
        tft_pf_input.tft_pf_entry.tft_pf_ipv6.tft_next_hdr = protocol_or_next_header;
        tft_pf_input.tft_pf_valid_bits |= NAS_TFT_ID_PROTOCOL_OR_NEXT_HDR;
      }

      /* Type of service and mask */
      if ( p_tos_mask_or_tc_mask_str )
      {
        p_tos_mask_or_tc_mask_str = parseDSNP_U16( 1, p_tos_mask_or_tc_mask_str, 
                                                 &tos_value );

        tft_pf_input.tft_pf_entry.tft_pf_ipv6.tft_tos_and_mask.tos_value = (U8)tos_value;
        
        p_tos_mask_or_tc_mask_str = parseDSNP_U16( 1, p_tos_mask_or_tc_mask_str, 
                                                 &tos_mask );

        tft_pf_input.tft_pf_entry.tft_pf_ipv6.tft_tos_and_mask.tos_mask = (U8)tos_mask;
        
        tft_pf_input.tft_pf_valid_bits |= NAS_TFT_ID_TOS_AND_MASK;
      }


      /* Destenation port */
      if ( p_dest_port_range_str )
      {
        p_dest_port_range_str = parseDSNP_U16( 1, p_dest_port_range_str, 
                                             (U16*)&tft_pf_input.tft_pf_entry.tft_pf_ipv6.tft_dest_port_range.low_limit );
        p_dest_port_range_str = parseDSNP_U16( 1, p_dest_port_range_str, 
                                             (U16*)&tft_pf_input.tft_pf_entry.tft_pf_ipv6.tft_dest_port_range.high_limit );
        tft_pf_input.tft_pf_valid_bits |= NAS_TFT_ID_DEST_PORT_RANGE;
      }
      
      /* Source port */
      if ( p_src_port_range_str )
      {
        p_src_port_range_str = parseDSNP_U16( 1, p_src_port_range_str, 
                                            (U16*)&tft_pf_input.tft_pf_entry.tft_pf_ipv6.tft_src_port_range.low_limit );
        p_src_port_range_str = parseDSNP_U16( 1, p_src_port_range_str, 
                                            (U16*)&tft_pf_input.tft_pf_entry.tft_pf_ipv6.tft_src_port_range.high_limit );
        tft_pf_input.tft_pf_valid_bits |= NAS_TFT_ID_SRC_PORT_RANGE;
      }

      /* IPSEC security parameter spi */
      if ( ipsec_spi )
      {
        tft_pf_input.tft_pf_entry.tft_pf_ipv6.tft_ipsec_spi = ipsec_spi;
        tft_pf_input.tft_pf_valid_bits |= NAS_TFT_ID_IPSEC_SPI;
      }

      break;
    } /* END: case NAS_is_tft_pf_ipv6 */
      
    default:
    {
      cmdCmeError(CME_ERR_OpNotAllow);
      return (ATI_FAIL);
    }
  } /* END: switch( tft_pf_input.ctrl_tft_pf_entry ) */

#ifdef FF_ATI_BAT
  {  
    T_BAT_cmd_send              bat_cmd;
    T_BAT_cmd_set_plus_cgtft bat_cgtft;
    bat_cmd.ctrl_params                  = BAT_CMD_SET_PLUS_CGTFT;
    bat_cmd.params.ptr_set_plus_cgtft = &bat_cgtft;
    
    bat_cgtft.cid               =  cid;
    bat_cgtft.tft_pf_id         = tft_pf_input.tft_pf_id;             
    bat_cgtft.tft_pf_precedence = tft_pf_input.tft_pf_precedence;     
    bat_cgtft.prt_num_or_nxt_hd = protocol_or_next_header;     
    
    /* addr_n_mask */
    bat_cgtft.v_addr_n_mask = (addr_n_mask_str_len AND
                               p_addr_n_mask_str);

    if (tft_pf_input.ctrl_tft_pf_entry EQ NAS_is_tft_pf_ipv4)
    {
      bat_cgtft.c_addr_n_mask = 2 * NAS_SIZE_IPv4_ADDR;
      memcpy(bat_cgtft.addr_n_mask,
             &tft_pf_input.tft_pf_entry.tft_pf_ipv4.tft_ipv4_src_addr_mask,
             2 * NAS_SIZE_IPv4_ADDR);

      /* dest_port_range */
      if (tft_pf_input.tft_pf_valid_bits & 
          NAS_TFT_ID_DEST_PORT_RANGE)
      {
        bat_cgtft.v_dest_port_range  = TRUE;
        bat_cgtft.c_dest_port_range  = 2;
        bat_cgtft.dest_port_range[0] = tft_pf_input.tft_pf_entry.
          tft_pf_ipv4.tft_dest_port_range.low_limit;
        bat_cgtft.dest_port_range[1] = tft_pf_input.tft_pf_entry.
          tft_pf_ipv4.tft_dest_port_range.high_limit;
      }    

      /* src_port_range */
      if (tft_pf_input.tft_pf_valid_bits & 
          NAS_TFT_ID_SRC_PORT_RANGE)
      {
        bat_cgtft.v_src_port_range  = TRUE;
        bat_cgtft.c_src_port_range  = 2;
        bat_cgtft.src_port_range[0] = tft_pf_input.tft_pf_entry.
          tft_pf_ipv4.tft_src_port_range.low_limit;
        bat_cgtft.src_port_range[1] = tft_pf_input.tft_pf_entry.
          tft_pf_ipv4.tft_src_port_range.high_limit;
      }    

      /* tos_mask_or_tc_mask */
      if (tft_pf_input.tft_pf_valid_bits &
          NAS_TFT_ID_TOS_AND_MASK)
      {
        bat_cgtft.v_tos_mask_or_tc_mask  = TRUE;
        bat_cgtft.c_tos_mask_or_tc_mask  = 2;
        bat_cgtft.tos_mask_or_tc_mask[0] = tft_pf_input.tft_pf_entry.
          tft_pf_ipv4.tft_tos_and_mask.tos_value;
        bat_cgtft.tos_mask_or_tc_mask[1] = tft_pf_input.tft_pf_entry.
          tft_pf_ipv4.tft_tos_and_mask.tos_mask;
      }
    }
    else if (tft_pf_input.ctrl_tft_pf_entry EQ NAS_is_tft_pf_ipv6)
    {
      bat_cgtft.c_addr_n_mask = 2 * NAS_SIZE_IPv6_ADDR;
      memcpy(bat_cgtft.addr_n_mask,
             &tft_pf_input.tft_pf_entry.tft_pf_ipv6.tft_ipv6_src_addr_mask,
             2 * NAS_SIZE_IPv6_ADDR);

      /* dest_port_range */
      if (tft_pf_input.tft_pf_valid_bits & 
          NAS_TFT_ID_DEST_PORT_RANGE)
      {
        bat_cgtft.v_dest_port_range  = TRUE;
        bat_cgtft.c_dest_port_range  = 2;
        bat_cgtft.dest_port_range[0] = tft_pf_input.tft_pf_entry.
          tft_pf_ipv6.tft_dest_port_range.low_limit;
        bat_cgtft.dest_port_range[1] = tft_pf_input.tft_pf_entry.
          tft_pf_ipv6.tft_dest_port_range.high_limit;
      }    

      /* src_port_range */
      if (tft_pf_input.tft_pf_valid_bits & 
          NAS_TFT_ID_SRC_PORT_RANGE)
      {
        bat_cgtft.v_src_port_range  = TRUE;
        bat_cgtft.c_src_port_range  = 2;
        bat_cgtft.src_port_range[0] = tft_pf_input.tft_pf_entry.
          tft_pf_ipv6.tft_src_port_range.low_limit;
        bat_cgtft.src_port_range[1] = tft_pf_input.tft_pf_entry.
          tft_pf_ipv6.tft_src_port_range.high_limit;
      }    

      /* tos_mask_or_tc_mask */
      if (tft_pf_input.tft_pf_valid_bits &
          NAS_TFT_ID_TOS_AND_MASK)
      {
        bat_cgtft.v_tos_mask_or_tc_mask  = TRUE;
        bat_cgtft.c_tos_mask_or_tc_mask  = 2;
        bat_cgtft.tos_mask_or_tc_mask[0] = tft_pf_input.tft_pf_entry.
          tft_pf_ipv6.tft_tos_and_mask.tos_value;
        bat_cgtft.tos_mask_or_tc_mask[1] = tft_pf_input.tft_pf_entry.
          tft_pf_ipv6.tft_tos_and_mask.tos_mask;
      }

    }

    /* ipsec_spi */
    //bat_cgtft.v_ipsec_spi = tft_pf_input.ipsec_spi_present;
    bat_cgtft.ipsec_spi   = ipsec_spi;
    
                                       
    /* flow_label */
    bat_cgtft.flow_label = flow_label;

    bat_send(ati_bat_get_client(srcId), &bat_cmd);
    src_params->curAtCmd=AT_CMD_CGTFT;
    return(ATI_EXCT);
}
#else
  switch( sAT_PlusCGTFT( ( T_ACI_CMD_SRC)srcId, cid , &tft_pf_input) )
  {
    case (AT_CMPL):                   /* Operation completed */
      return (ATI_CMPL);
    case (AT_EXCT):                   /* Operation in progress */
//      curAtCmd    = AT_CMD_CGTFT; 
      return (ATI_EXCT);
    default:
      cmdCmeError(CME_ERR_Unknown);   /* Command failed */
      return (ATI_FAIL);
  }

#endif /* FF_ATI_BAT */
}

/*
+--------------------------------------------------------------------+
| PROJECT : UMTS                        MODULE  : ATI_GPRS           |
| STATE   :                             ROUTINE : setatPlusCGTFT     |
+--------------------------------------------------------------------+

  PURPOSE : +CGTFT=? command
*/

GLOBAL T_ATI_RSLT queatPlusCGTFT( char *cl, UBYTE srcId )
{
  T_ACI_RETURN            ret;
  U8                      pdp_context_cid_array[PDP_CONTEXT_CID_MAX];
  T_PDP_CONTEXT_INTERNAL *p_pdp_context_node = NULL;
  T_TFT_INTERNAL         *p_tft_pf_node = NULL;
  int                     i = 0;
  USHORT                  pos = 0;

  TRACE_FUNCTION("queatPlusCGTFT()");

#ifdef FF_ATI_BAT
{
    T_BAT_cmd_send cmd;
    T_BAT_no_parameter dummy;

    cmd.ctrl_params = BAT_CMD_SET_PLUS_CGTFT;
    dummy.bat_dummy = 0xFF;
    cmd.params.ptr_set_plus_cgtft = &dummy;
    bat_send(ati_bat_get_client(srcId), &cmd);
    return(ATI_EXCT);
}
#else

  memset( &pdp_context_cid_array, PDP_CONTEXT_CID_OMITTED, sizeof(pdp_context_cid_array) );

  ret = qAT_PlusCGTFT(( T_ACI_CMD_SRC)srcId, (U8*) &pdp_context_cid_array[0]);

  if (ret EQ AT_CMPL)
  {
    for ( i = 0; i < PDP_CONTEXT_CID_MAX AND pdp_context_cid_array[i] NEQ PDP_CONTEXT_CID_OMITTED; i++ )
    {
      p_pdp_context_node = pdp_context_find_node_from_cid( pdp_context_cid_array[i] );
      p_tft_pf_node = p_pdp_context_node->p_tft_pf;
      while( p_tft_pf_node )
      {
        switch( p_tft_pf_node->pf_attributes.ctrl_tft_pf_entry )
        {
          case NAS_is_tft_pf_ipv4:
            pos = sprintf( g_sa, "+CGTFT: %d,%d,%d,",
                           pdp_context_cid_array[i],
                           p_tft_pf_node->pf_attributes.tft_pf_id,
                           p_tft_pf_node->pf_attributes.tft_pf_precedence);

            if( p_tft_pf_node->pf_attributes.tft_pf_valid_bits & NAS_TFT_ID_IPv4_SRC_ADDR_MASK )
            {
              pos += sprintf( g_sa + pos , "\"%d.%d.%d.%d.%d.%d.%d.%d\",",
                             p_tft_pf_node->pf_attributes.tft_pf_entry.tft_pf_ipv4.tft_ipv4_src_addr_mask.tft_ipv4_addr[0],
                             p_tft_pf_node->pf_attributes.tft_pf_entry.tft_pf_ipv4.tft_ipv4_src_addr_mask.tft_ipv4_addr[1],
                             p_tft_pf_node->pf_attributes.tft_pf_entry.tft_pf_ipv4.tft_ipv4_src_addr_mask.tft_ipv4_addr[2],
                             p_tft_pf_node->pf_attributes.tft_pf_entry.tft_pf_ipv4.tft_ipv4_src_addr_mask.tft_ipv4_addr[3],
                             p_tft_pf_node->pf_attributes.tft_pf_entry.tft_pf_ipv4.tft_ipv4_src_addr_mask.tft_ipv4_mask[0],
                             p_tft_pf_node->pf_attributes.tft_pf_entry.tft_pf_ipv4.tft_ipv4_src_addr_mask.tft_ipv4_mask[1],
                             p_tft_pf_node->pf_attributes.tft_pf_entry.tft_pf_ipv4.tft_ipv4_src_addr_mask.tft_ipv4_mask[2],
                             p_tft_pf_node->pf_attributes.tft_pf_entry.tft_pf_ipv4.tft_ipv4_src_addr_mask.tft_ipv4_mask[3] );
            }
            else
            {
              pos += sprintf( g_sa + pos, "," );
            }

            if( p_tft_pf_node->pf_attributes.tft_pf_valid_bits & NAS_TFT_ID_PROTOCOL_OR_NEXT_HDR )
            {
              pos += sprintf( g_sa + pos, "%d,",
                             p_tft_pf_node->pf_attributes.tft_pf_entry.tft_pf_ipv4.tft_protocol );
            }
            else
            {
              pos += sprintf( g_sa + pos, "," );
            }

            if( p_tft_pf_node->pf_attributes.tft_pf_valid_bits & NAS_TFT_ID_DEST_PORT_RANGE )
            {
              pos += sprintf( g_sa + pos, "\"%d.%d\",",
                             p_tft_pf_node->pf_attributes.tft_pf_entry.tft_pf_ipv4.tft_dest_port_range.low_limit,
                             p_tft_pf_node->pf_attributes.tft_pf_entry.tft_pf_ipv4.tft_dest_port_range.high_limit );
            }
            else
            {
              pos += sprintf( g_sa + pos, "," );
            }

            if( p_tft_pf_node->pf_attributes.tft_pf_valid_bits & NAS_TFT_ID_SRC_PORT_RANGE )
            {
              pos += sprintf( g_sa + pos, "\"%d.%d\",",
                             p_tft_pf_node->pf_attributes.tft_pf_entry.tft_pf_ipv4.tft_src_port_range.low_limit,
                             p_tft_pf_node->pf_attributes.tft_pf_entry.tft_pf_ipv4.tft_src_port_range.high_limit );
            }
            else
            {
              pos += sprintf( g_sa + pos, "," );
            }

            if( p_tft_pf_node->pf_attributes.tft_pf_valid_bits & NAS_TFT_ID_IPSEC_SPI )
            {
              pos += sprintf( g_sa + pos, "%X,",
                             p_tft_pf_node->pf_attributes.tft_pf_entry.tft_pf_ipv4.tft_ipsec_spi );
            }
            else
            {
              pos += sprintf( g_sa + pos, "," );
            }
              
            if( p_tft_pf_node->pf_attributes.tft_pf_valid_bits & NAS_TFT_ID_TOS_AND_MASK )
            {
              pos += sprintf( g_sa + pos, "\"%d.%d\",",
                             p_tft_pf_node->pf_attributes.tft_pf_entry.tft_pf_ipv4.tft_tos_and_mask.tos_value,
                             p_tft_pf_node->pf_attributes.tft_pf_entry.tft_pf_ipv4.tft_tos_and_mask.tos_mask );
            }
            else
            {
              pos += sprintf( g_sa + pos, "," );
            }

            break;

          case NAS_is_tft_pf_ipv6:
            pos = sprintf( g_sa, "+CGTFT: %d,%d,%d,",
                          pdp_context_cid_array[i],
                          p_tft_pf_node->pf_attributes.tft_pf_id,
                          p_tft_pf_node->pf_attributes.tft_pf_precedence);
            
            if( p_tft_pf_node->pf_attributes.tft_pf_valid_bits & NAS_TFT_ID_IPv6_SRC_ADDR_MASK )
            {
              pos += sprintf( g_sa + pos, "\"%d.%d.%d.%d.%d.%d.%d.%d.%d.%d.%d.%d.%d.%d.%d.%d.%d.%d.%d.%d.%d.%d.%d.%d.%d.%d.%d.%d.%d.%d.%d.%d\",",
                             p_tft_pf_node->pf_attributes.tft_pf_entry.tft_pf_ipv6.tft_ipv6_src_addr_mask.tft_ipv6_addr[0],
                             p_tft_pf_node->pf_attributes.tft_pf_entry.tft_pf_ipv6.tft_ipv6_src_addr_mask.tft_ipv6_addr[1],
                             p_tft_pf_node->pf_attributes.tft_pf_entry.tft_pf_ipv6.tft_ipv6_src_addr_mask.tft_ipv6_addr[2],
                             p_tft_pf_node->pf_attributes.tft_pf_entry.tft_pf_ipv6.tft_ipv6_src_addr_mask.tft_ipv6_addr[3],
                             p_tft_pf_node->pf_attributes.tft_pf_entry.tft_pf_ipv6.tft_ipv6_src_addr_mask.tft_ipv6_addr[4],
                             p_tft_pf_node->pf_attributes.tft_pf_entry.tft_pf_ipv6.tft_ipv6_src_addr_mask.tft_ipv6_addr[5],
                             p_tft_pf_node->pf_attributes.tft_pf_entry.tft_pf_ipv6.tft_ipv6_src_addr_mask.tft_ipv6_addr[6],
                             p_tft_pf_node->pf_attributes.tft_pf_entry.tft_pf_ipv6.tft_ipv6_src_addr_mask.tft_ipv6_addr[7],
                             p_tft_pf_node->pf_attributes.tft_pf_entry.tft_pf_ipv6.tft_ipv6_src_addr_mask.tft_ipv6_addr[8],
                             p_tft_pf_node->pf_attributes.tft_pf_entry.tft_pf_ipv6.tft_ipv6_src_addr_mask.tft_ipv6_addr[9],
                             p_tft_pf_node->pf_attributes.tft_pf_entry.tft_pf_ipv6.tft_ipv6_src_addr_mask.tft_ipv6_addr[10],
                             p_tft_pf_node->pf_attributes.tft_pf_entry.tft_pf_ipv6.tft_ipv6_src_addr_mask.tft_ipv6_addr[11],
                             p_tft_pf_node->pf_attributes.tft_pf_entry.tft_pf_ipv6.tft_ipv6_src_addr_mask.tft_ipv6_addr[12],
                             p_tft_pf_node->pf_attributes.tft_pf_entry.tft_pf_ipv6.tft_ipv6_src_addr_mask.tft_ipv6_addr[13],
                             p_tft_pf_node->pf_attributes.tft_pf_entry.tft_pf_ipv6.tft_ipv6_src_addr_mask.tft_ipv6_addr[14],
                             p_tft_pf_node->pf_attributes.tft_pf_entry.tft_pf_ipv6.tft_ipv6_src_addr_mask.tft_ipv6_addr[15],

                             p_tft_pf_node->pf_attributes.tft_pf_entry.tft_pf_ipv6.tft_ipv6_src_addr_mask.tft_ipv6_mask[0],
                             p_tft_pf_node->pf_attributes.tft_pf_entry.tft_pf_ipv6.tft_ipv6_src_addr_mask.tft_ipv6_mask[1],
                             p_tft_pf_node->pf_attributes.tft_pf_entry.tft_pf_ipv6.tft_ipv6_src_addr_mask.tft_ipv6_mask[2],
                             p_tft_pf_node->pf_attributes.tft_pf_entry.tft_pf_ipv6.tft_ipv6_src_addr_mask.tft_ipv6_mask[3],
                             p_tft_pf_node->pf_attributes.tft_pf_entry.tft_pf_ipv6.tft_ipv6_src_addr_mask.tft_ipv6_mask[4],
                             p_tft_pf_node->pf_attributes.tft_pf_entry.tft_pf_ipv6.tft_ipv6_src_addr_mask.tft_ipv6_mask[5],
                             p_tft_pf_node->pf_attributes.tft_pf_entry.tft_pf_ipv6.tft_ipv6_src_addr_mask.tft_ipv6_mask[6],
                             p_tft_pf_node->pf_attributes.tft_pf_entry.tft_pf_ipv6.tft_ipv6_src_addr_mask.tft_ipv6_mask[7],
                             p_tft_pf_node->pf_attributes.tft_pf_entry.tft_pf_ipv6.tft_ipv6_src_addr_mask.tft_ipv6_mask[8],
                             p_tft_pf_node->pf_attributes.tft_pf_entry.tft_pf_ipv6.tft_ipv6_src_addr_mask.tft_ipv6_mask[9],
                             p_tft_pf_node->pf_attributes.tft_pf_entry.tft_pf_ipv6.tft_ipv6_src_addr_mask.tft_ipv6_mask[10],
                             p_tft_pf_node->pf_attributes.tft_pf_entry.tft_pf_ipv6.tft_ipv6_src_addr_mask.tft_ipv6_mask[11],
                             p_tft_pf_node->pf_attributes.tft_pf_entry.tft_pf_ipv6.tft_ipv6_src_addr_mask.tft_ipv6_mask[12],
                             p_tft_pf_node->pf_attributes.tft_pf_entry.tft_pf_ipv6.tft_ipv6_src_addr_mask.tft_ipv6_mask[13],
                             p_tft_pf_node->pf_attributes.tft_pf_entry.tft_pf_ipv6.tft_ipv6_src_addr_mask.tft_ipv6_mask[14],
                             p_tft_pf_node->pf_attributes.tft_pf_entry.tft_pf_ipv6.tft_ipv6_src_addr_mask.tft_ipv6_mask[15]);
            }
            else
            {
              pos += sprintf( g_sa + pos, "," );
            }

            if( p_tft_pf_node->pf_attributes.tft_pf_valid_bits & NAS_TFT_ID_PROTOCOL_OR_NEXT_HDR )
            {
              pos += sprintf( g_sa + pos, "%d,",
                             p_tft_pf_node->pf_attributes.tft_pf_entry.tft_pf_ipv6.tft_next_hdr );
            }
            else
            {
              pos += sprintf( g_sa + pos, "," );
            }

            if( p_tft_pf_node->pf_attributes.tft_pf_valid_bits & NAS_TFT_ID_DEST_PORT_RANGE )
            {
              pos += sprintf( g_sa + pos, "\"%d.%d\",",
                             p_tft_pf_node->pf_attributes.tft_pf_entry.tft_pf_ipv6.tft_dest_port_range.low_limit,
                             p_tft_pf_node->pf_attributes.tft_pf_entry.tft_pf_ipv6.tft_dest_port_range.high_limit );
            }
            else
            {
              pos += sprintf( g_sa + pos, "," );
            }

            if( p_tft_pf_node->pf_attributes.tft_pf_valid_bits & NAS_TFT_ID_SRC_PORT_RANGE )
            {
              pos += sprintf( g_sa + pos, "\"%d.%d\",",
                             p_tft_pf_node->pf_attributes.tft_pf_entry.tft_pf_ipv6.tft_src_port_range.low_limit,
                             p_tft_pf_node->pf_attributes.tft_pf_entry.tft_pf_ipv6.tft_src_port_range.high_limit );
            }
            else
            {
              pos += sprintf( g_sa + pos, "," );
            }

            if( p_tft_pf_node->pf_attributes.tft_pf_valid_bits & NAS_TFT_ID_IPSEC_SPI )
            {
              pos += sprintf( g_sa + pos, "%X,",
                             p_tft_pf_node->pf_attributes.tft_pf_entry.tft_pf_ipv6.tft_ipsec_spi );
            }
            else
            {
              pos += sprintf( g_sa + pos, "," );
            }
              
            if( p_tft_pf_node->pf_attributes.tft_pf_valid_bits & NAS_TFT_ID_TOS_AND_MASK )
            {
              pos += sprintf( g_sa + pos, "\"%d.%d\",",
                             p_tft_pf_node->pf_attributes.tft_pf_entry.tft_pf_ipv6.tft_tos_and_mask.tos_value,
                             p_tft_pf_node->pf_attributes.tft_pf_entry.tft_pf_ipv6.tft_tos_and_mask.tos_mask );
            }
            else
            {
              pos += sprintf( g_sa + pos, "," );
            }

            if( p_tft_pf_node->pf_attributes.tft_pf_valid_bits & NAS_TFT_ID_FLOW_LABEL )
            {
              pos += sprintf( g_sa + pos, "%d",
                             p_tft_pf_node->pf_attributes.tft_pf_entry.tft_pf_ipv6.tft_flow_label );
            }
            
            break;            
        }

        io_sendMessage(srcId, g_sa, ATI_NORMAL_OUTPUT);
        p_tft_pf_node = p_tft_pf_node->p_next;
      }
      
    }
  }
  else
  {
    cmdCmeError(CME_ERR_Unknown);
    return (ATI_FAIL);
  }

  return (map_aci_2_ati_rslt(ret));
#endif /* FF_ATI_BAT */
}

/*
+--------------------------------------------------------------------+
| PROJECT : UMTS                        MODULE  : ATI_GPRS           |
| STATE   :                             ROUTINE : testatPlusCGTFT    |
+--------------------------------------------------------------------+

  PURPOSE : +CGTFT?
*/
GLOBAL T_ATI_RSLT tesatPlusCGTFT( char *cl, UBYTE srcId )
{
  U8   pf_id_max;

  pf_id_max = TFT_PF_ID_MAX;

  TRACE_FUNCTION("tesatPlusCGTFT()");

  sprintf( g_sa, "+CGTFT: <PDP_type = \"IP\">,   (1-%d),(0-255),\"a1.a2.a3.a4.m1.m2.m3.m4\" (0-255),(0-255),\"f.t\" (0-65535),\"f.t\" (0-65535),(0-FFFFFFFF),\"t.m\" (0-255),(0)", pf_id_max );
  io_sendMessage( srcId, g_sa, ATI_NORMAL_OUTPUT);
  
  sprintf( g_sa, "+CGTFT: <PDP_type = \"IPV6\">, (1-%d),(0-255),\"a1.a2...a15.m1.m2...m15\" (0-255),(0-255),\"f.t\" (0-65535),\"f.t\" (0-65535),(0-FFFFFFFF),\"t.m\" (0-255),(0-FFFF)", pf_id_max );
  io_sendMessage( srcId, g_sa, ATI_NORMAL_OUTPUT);

  /* As for EDGE we are not supporting PDP_Type as "PPP" */
  /*sprintf( g_sa, "+CGTFT: <PDP_type = \"PPP\">,  (1-%d),(0-255),\"a1.a2...a15.m1.m2...m15\" (0-255),(0-255),\"f.t\" (0-65535),\"f.t\" (0-65535),(0-FFFFFFFF),\"t.m\" (0-255),(0-FFFF)", pf_id_max );
  io_sendMessage( srcId, g_sa, ATI_NORMAL_OUTPUT);*/

  return (ATI_CMPL);
}
#endif /* REL99 */

/*
+--------------------------------------------------------------------+
| PROJECT : GPRS (8441)                 MODULE  : GACI_CMD           |
| STATE   : finnished                   ROUTINE : setatPlusCGQREQ    |
+--------------------------------------------------------------------+

  PURPOSE : +CGQREQ  command (context QOS request)
*/

GLOBAL T_ATI_RSLT setatPlusCGQREQ (char *cl, UBYTE srcId)
{
  T_ACI_RETURN ret = AT_FAIL;
  T_PS_qos     input_qos;
  U8           cid;
  T_ATI_SRC_PARAMS *src_params = find_element (ati_src_list, srcId, search_ati_src_id);
  TRACE_FUNCTION("setatPlusCGQREQ ()");

  cid = PDP_CONTEXT_CID_OMITTED;

  input_qos.qos_r97.delay    = (U8) PDP_CONTEXT_QOS_OMITTED;
  input_qos.qos_r97.relclass = (U8) PDP_CONTEXT_QOS_OMITTED;
  input_qos.qos_r97.peak     = (U8) PDP_CONTEXT_QOS_OMITTED;
  input_qos.qos_r97.preced   = (U8) PDP_CONTEXT_QOS_OMITTED;
  input_qos.qos_r97.mean     = (U8) PDP_CONTEXT_QOS_OMITTED;

  cl = parse( cl,"caaaar", &cid,
                           1L ,&input_qos.qos_r97.preced,
                           1L ,&input_qos.qos_r97.delay,
                           1L ,&input_qos.qos_r97.relclass,
                           1L ,&input_qos.qos_r97.peak,
                           &input_qos.qos_r97.mean );
  if ( !cl )
  {
    cmdCmeError(CME_ERR_OpNotAllow);
    return (ATI_FAIL);
  }

  if ( input_qos.qos_r97.preced NEQ PDP_CONTEXT_QOS_OMITTED )
    input_qos.qos_r97.preced    -= '0';
  if ( input_qos.qos_r97.delay NEQ PDP_CONTEXT_QOS_OMITTED )
    input_qos.qos_r97.delay     -= '0';
  if ( input_qos.qos_r97.relclass NEQ PDP_CONTEXT_QOS_OMITTED )
    input_qos.qos_r97.relclass  -= '0';
  if ( input_qos.qos_r97.peak NEQ PDP_CONTEXT_QOS_OMITTED )
    input_qos.qos_r97.peak      -= '0';
  if ( input_qos.qos_r97.mean EQ 0xff )
    input_qos.qos_r97.mean       = 0;

#ifdef FF_ATI_BAT

  {
    T_BAT_cmd_send cmd;
    T_BAT_cmd_set_plus_cgqreq cgqreq;

    cmd.ctrl_params = BAT_CMD_SET_PLUS_CGQREQ;
    cmd.params.ptr_set_plus_cgqreq=&cgqreq;

    cgqreq.cid=(T_BAT_pdp_cid)cid;
    cgqreq.precedence  = (T_BAT_precedence)input_qos.qos_r97.preced;
    cgqreq.delay       = (T_BAT_delay)input_qos.qos_r97.delay;
    cgqreq.reliability = (T_BAT_reliability)input_qos.qos_r97.relclass;
    cgqreq.peak        = (T_BAT_peak)input_qos.qos_r97.peak;
    cgqreq.mean        = (T_BAT_mean)input_qos.qos_r97.mean;

    bat_send(ati_bat_get_client(srcId), &cmd);

    src_params->curAtCmd=AT_CMD_CGQREQ;
    return(ATI_EXCT);
  }
#else
  ret = sAT_PlusCGQREQ ((T_ACI_CMD_SRC)srcId, cid ,&input_qos);
  switch (ret)
  {
    case (AT_CMPL):                         /*operation completed*/
      return (ATI_CMPL);
    case (AT_EXCT):
//      src_params->curAtCmd    = AT_CMD_CGQREQ;
      return (ATI_EXCT);
    default:
      cmdCmeError(CME_ERR_Unknown);         /*Command failed*/
      return (ATI_FAIL);
  }
  #endif
}

/*
+--------------------------------------------------------------------+
| PROJECT : GPRS (8441)                 MODULE  : GACI_CMD           |
| STATE   : finnished                   ROUTINE : queatPlusCGQREQ    |
+--------------------------------------------------------------------+

  PURPOSE : Reading +CGQREQ  command (context QOS request)
*/

GLOBAL T_ATI_RSLT queatPlusCGQREQ (char *cl, UBYTE srcId)
{
#ifndef FF_ATI_BAT
  T_PS_qos                pdef_qos;
  BOOL                    qos_valid          = FALSE;
  T_PDP_CONTEXT_INTERNAL *p_pdp_context_node = NULL;
#endif

  TRACE_FUNCTION("queatPlusCGQREQ ()");
#ifdef FF_ATI_BAT
  {
    T_BAT_cmd_send cmd;
    T_BAT_no_parameter dummy;

    cmd.ctrl_params = BAT_CMD_QUE_PLUS_CGQREQ;
    dummy.bat_dummy = 0xFF;
    cmd.params.ptr_que_plus_cgqreq = &dummy;
    bat_send(ati_bat_get_client(srcId), &cmd);
    return(ATI_EXCT);
  }
#else
  p_pdp_context_node = p_pdp_context_list;

  while( p_pdp_context_node )
  {
    if (qAT_PlusCGQREQ ((T_ACI_CMD_SRC)srcId, p_pdp_context_node->cid, &qos_valid, &pdef_qos ) EQ AT_CMPL)
    {
      if( qos_valid )
      {
        sprintf( g_sa, "+CGQREQ: %hd,%d,%d,%d,%d,%d", 
                       p_pdp_context_node->cid,
                       pdef_qos.qos_r97.preced,
                       pdef_qos.qos_r97.delay,
                       pdef_qos.qos_r97.relclass,
                       pdef_qos.qos_r97.peak,
                       pdef_qos.qos_r97.mean );

        io_sendMessage( srcId, g_sa, ATI_NORMAL_OUTPUT );
      }
    }
    else
    {
      /* AT_FAIL has been returned */
      cmdCmeError(CME_ERR_Unknown); /* The command is invalid for srcId */
      return (ATI_FAIL);
    }

    p_pdp_context_node = p_pdp_context_node->p_next;
  }

  return (ATI_CMPL);
#endif
}

/*
+--------------------------------------------------------------------+
| PROJECT : GPRS (8441)                 MODULE  : GACI_CMD           |
| STATE   : finished                    ROUTINE : atPlusCGQMIN       |
+--------------------------------------------------------------------+

  PURPOSE : +CGQMIN command (context min QOS requirement)
*/

GLOBAL T_ATI_RSLT setatPlusCGQMIN (char *cl, UBYTE srcId)
{
  T_ACI_RETURN ret = AT_FAIL;
  T_PS_min_qos input_qos;
  U8           cid;
  T_ATI_SRC_PARAMS *src_params = find_element (ati_src_list, srcId, search_ati_src_id);

  TRACE_FUNCTION("setatPlusCGQMIN ()");

  cid = PDP_CONTEXT_CID_OMITTED;

  input_qos.qos_r97.delay    = (U8) PDP_CONTEXT_QOS_OMITTED;
  input_qos.qos_r97.relclass = (U8) PDP_CONTEXT_QOS_OMITTED;
  input_qos.qos_r97.peak     = (U8) PDP_CONTEXT_QOS_OMITTED;
  input_qos.qos_r97.preced   = (U8) PDP_CONTEXT_QOS_OMITTED;
  input_qos.qos_r97.mean     = (U8) PDP_CONTEXT_QOS_OMITTED;

  cl = parse( cl, "caaaar", &cid,
                1L, &input_qos.qos_r97.preced,
              1L, &input_qos.qos_r97.delay,
              1L, &input_qos.qos_r97.relclass,
              1L, &input_qos.qos_r97.peak,
                  &input_qos.qos_r97.mean );

  if ( !cl )
  {
    cmdCmeError(CME_ERR_OpNotAllow);
    return (ATI_FAIL);
  }

#ifdef FF_ATI_BAT

  {
    T_BAT_cmd_send cmd;
    T_BAT_cmd_set_plus_cgqmin cgqmin;

    cmd.ctrl_params = BAT_CMD_SET_PLUS_CGQMIN;
    cmd.params.ptr_set_plus_cgqmin=&cgqmin;

    cgqmin.cid=(T_BAT_pdp_cid)cid;
/*
    if (input_qos.preced!=GPRS_QOS_OMITTED)
      cgqmin.precedence=(T_BAT_precedence)input_qos.preced;
    else
      cgqmin.precedence=BAT_PRECEDENCE_NOT_PRESENT;

    if (input_qos.delay!=GPRS_QOS_OMITTED)
      cgqmin.delay=(T_BAT_delay)input_qos.delay;
    else
      cgqmin.delay=BAT_DELAY_NOT_PRESENT;

    if (input_qos.relclass!=GPRS_QOS_OMITTED)
      cgqmin.reliability=(T_BAT_reliability)input_qos.relclass;
    else
      cgqmin.reliability=BAT_RELIABILITY_NOT_PRESENT;

    if (input_qos.peak!=GPRS_QOS_OMITTED)
      cgqmin.peak=(T_BAT_peak)input_qos.peak;
    else
      cgqmin.peak=BAT_PEAK_NOT_PRESENT;

    if (input_qos.mean)
      cgqmin.mean=(T_BAT_mean)input_qos.mean;
    else
      cgqmin.mean=BAT_MEAN_NOT_PRESENT;
*/

   cgqmin.precedence  = (T_BAT_precedence)input_qos.qos_r97.preced;
   cgqmin.delay       = (T_BAT_delay)input_qos.qos_r97.delay;
   cgqmin.reliability = (T_BAT_reliability)input_qos.qos_r97.relclass;
   cgqmin.peak        = (T_BAT_peak)input_qos.qos_r97.peak;
   cgqmin.mean        = (T_BAT_mean)input_qos.qos_r97.mean;

    bat_send(ati_bat_get_client(srcId), &cmd);

    src_params->curAtCmd=AT_CMD_CGQMIN;
    return(ATI_EXCT);
  }

#else


  if ( input_qos.qos_r97.preced NEQ PDP_CONTEXT_QOS_OMITTED )
    input_qos.qos_r97.preced    -= '0';
  if ( input_qos.qos_r97.delay NEQ PDP_CONTEXT_QOS_OMITTED )
    input_qos.qos_r97.delay     -= '0';
  if ( input_qos.qos_r97.relclass NEQ PDP_CONTEXT_QOS_OMITTED )
    input_qos.qos_r97.relclass  -= '0';
  if ( input_qos.qos_r97.peak NEQ PDP_CONTEXT_QOS_OMITTED )
    input_qos.qos_r97.peak      -= '0';
  if ( input_qos.qos_r97.mean EQ 0xff )
    input_qos.qos_r97.mean       = 0;



  ret = sAT_PlusCGQMIN((T_ACI_CMD_SRC)srcId, cid, &input_qos );
  switch (ret)
  {
    case (AT_CMPL):                         /*operation completed*/
      return (ATI_CMPL);
    case (AT_EXCT):
      src_params->curAtCmd    = AT_CMD_CGQMIN;
      return (ATI_EXCT);
    default:
      cmdCmeError(CME_ERR_Unknown);         /*Command failed*/
      return (ATI_FAIL);
  }
#endif
}

/*
+--------------------------------------------------------------------+
| PROJECT : GPRS (8441)                 MODULE  : GACI_CMD           |
| STATE   : finished                    ROUTINE : queatPlusCGQMIN    |
+--------------------------------------------------------------------+

  PURPOSE : Reading +CGQMIN command (context min QOS requirement)
*/
GLOBAL T_ATI_RSLT queatPlusCGQMIN (char *cl, UBYTE srcId)

{
#ifndef FF_ATI_BAT
  T_PS_qos                pdef_qos;
  BOOL                    qos_valid          = FALSE;
  T_PDP_CONTEXT_INTERNAL *p_pdp_context_node = NULL;

#endif

#ifdef FF_ATI_BAT
  {
    T_BAT_cmd_send cmd;
    T_BAT_no_parameter dummy;

    cmd.ctrl_params = BAT_CMD_QUE_PLUS_CGQMIN;
    dummy.bat_dummy = 0xFF;
    cmd.params.ptr_que_plus_cgqmin = &dummy;

    bat_send(ati_bat_get_client(srcId), &cmd);

    return(ATI_EXCT);
  }
#else

  TRACE_FUNCTION("queatPlusCGQMIN ()");

  p_pdp_context_node = p_pdp_context_list;

  while( p_pdp_context_node )
  {
    if( qAT_PlusCGQMIN((T_ACI_CMD_SRC)srcId, p_pdp_context_node->cid, &qos_valid, &pdef_qos ) EQ AT_CMPL )
    {
      if (qos_valid)
      {

        /* In the privous version of alborg code is the paramters are ommited then
           the code is sending ',' instead of default values. Now it changed 
         */
        sprintf( g_sa, "+CGQMIN: %hd,%d,%d,%d,%d,%d", 
                       p_pdp_context_node->cid,
                       pdef_qos.qos_r97.preced,
                       pdef_qos.qos_r97.delay,
                       pdef_qos.qos_r97.relclass,
                       pdef_qos.qos_r97.peak,
                       pdef_qos.qos_r97.mean );
        io_sendMessage( srcId, g_sa, ATI_NORMAL_OUTPUT );
      }
    }
    else
    {
      /* AT_FAIL has been returned */
      cmdCmeError(CME_ERR_Unknown); /* The command is invalid for srcId */
      return (ATI_FAIL);
    }

    p_pdp_context_node = p_pdp_context_node->p_next;
    
  }

  return (ATI_CMPL);
#endif
}


/*
+--------------------------------------------------------------------+
| PROJECT : GPRS (8441)                 MODULE  : GACI_CMD           |
| STATE   : finished                    ROUTINE : atPlusCGATT        |
+--------------------------------------------------------------------+

  PURPOSE : +CGATT command ( GPRS attach/detach )
*/

GLOBAL T_ATI_RSLT setatPlusCGATT (char *cl, UBYTE srcId)
{
  T_ACI_RETURN ret = AT_FAIL;
  T_CGATT_STATE state = CGATT_STATE_OMITTED;
  T_ATI_SRC_PARAMS *src_params = find_element (ati_src_list, srcId, search_ati_src_id);

  TRACE_FUNCTION("setatPlusCGATT ()");

  cl=parse(cl,"d",&state);

  if ( !cl )
  {
    cmdCmeError(CME_ERR_OpNotAllow);
    return (ATI_FAIL);
  }
#ifdef FF_ATI_BAT

  {
    T_BAT_cmd_send cmd;
    T_BAT_cmd_set_plus_cgatt cgatt;

    cmd.ctrl_params = BAT_CMD_SET_PLUS_CGATT;
    cmd.params.ptr_set_plus_cgatt=&cgatt;

    /*
    *   This relies on T_CGATT_STATE being identical to
    *   T_BAT_plus_cgatt_state.
    */
    cgatt.state=(T_BAT_plus_cgatt_state)state;

    bat_send(ati_bat_get_client(srcId), &cmd);

    src_params->curAtCmd=AT_CMD_CGATT;
    return(ATI_EXCT);
  }
#else

  ret = sAT_PlusCGATT ((T_ACI_CMD_SRC)srcId, state );
  switch (ret)
  {
    case (AT_CMPL):                         /*operation completed*/
      return (ATI_CMPL);
    case (AT_EXCT):
      src_params->curAtCmd    = AT_CMD_CGATT;
      return (ATI_EXCT);
    default:
      cmdCmeError(CME_ERR_Unknown);         /*Command failed*/
      return (ATI_FAIL);
  }
#endif
}

GLOBAL T_ATI_RSLT queatPlusCGATT (char *cl, UBYTE srcId)
{
#ifndef FF_ATI_BAT
  char *me="+CGATT: ";
  T_ACI_RETURN ret = AT_FAIL;
  T_CGATT_STATE state = CGATT_STATE_OMITTED;
#endif

  TRACE_FUNCTION("queatPlusCGATT ()");

#ifdef FF_ATI_BAT
  {
    T_BAT_cmd_send cmd;
    T_BAT_no_parameter dummy;

    cmd.ctrl_params = BAT_CMD_QUE_PLUS_CGATT;
    dummy.bat_dummy = 0xFF;
    cmd.params.ptr_que_plus_cgatt = &dummy;

    bat_send(ati_bat_get_client(srcId), &cmd);

    return(ATI_EXCT);
  } 
#else

  ret = qAT_PlusCGATT((T_ACI_CMD_SRC)srcId, &state);
  if (ret EQ AT_CMPL)
  {
    sprintf(g_sa,"%s%d", me, state);
    io_sendMessage(srcId, g_sa, ATI_NORMAL_OUTPUT);
  }
  else
  {
    cmdCmeError(CME_ERR_Unknown);
    return (ATI_FAIL);
  }

  return (map_aci_2_ati_rslt(ret));
#endif
}


/*
+--------------------------------------------------------------------+
| PROJECT : GPRS (8441)                 MODULE  : GACI_CMD           |
| STATE   : finished                    ROUTINE : atPlusCGCLASS      |
+--------------------------------------------------------------------+

  PURPOSE : +CGCLASS command  (GPRS class change)
*/

GLOBAL T_ATI_RSLT setatPlusCGCLASS (char *cl, UBYTE srcId)
{
  T_ACI_RETURN ret = AT_FAIL;
  T_CGCLASS_CLASS m_class = CGCLASS_CLASS_OMITTED;
  char test_class[3]="";
  T_ATI_SRC_PARAMS *src_params = find_element (ati_src_list, srcId, search_ati_src_id);

  TRACE_FUNCTION("atPlusCGCLASS ()");

  cl = parse (cl,"s", (LONG)3, &test_class );

  switch (*test_class)
  {
    case 'A':
      break;
    case 'B':
      m_class = CGCLASS_CLASS_B;
      break;
    case 'C':
      switch(*(test_class + 1))
      {
        case 'G':
          m_class = CGCLASS_CLASS_CG;
          break;
        case 'C':
          m_class = CGCLASS_CLASS_CC;
          break;
        default:
          break;
      }
      break;
    default:
      break;
  }

  if ( !cl OR m_class EQ CGCLASS_CLASS_OMITTED )
  {
    cmdCmeError(CME_ERR_OpNotAllow);
    return (ATI_FAIL);
  }
#ifdef FF_ATI_BAT

  {
    T_BAT_cmd_send cmd;
    T_BAT_cmd_set_plus_cgclass cgclass;

    cmd.ctrl_params = BAT_CMD_SET_PLUS_CGCLASS;
    cmd.params.ptr_set_plus_cgclass=&cgclass;

    /*
    *   This relies on T_BAT_mobile_class and T_CGCLASS_CLASS
    *   being identical.
    */
    cgclass.mobile_class=(T_BAT_mobile_class)m_class;

    bat_send(ati_bat_get_client(srcId), &cmd);

    src_params->curAtCmd=AT_CMD_CGCLASS;
    return(ATI_EXCT);
  }

#else

  ret = sAT_PlusCGCLASS ((T_ACI_CMD_SRC)srcId, m_class );
  switch (ret)
  {
    case (AT_CMPL):                         /*operation completed*/
      return (ATI_CMPL);
    case (AT_EXCT):
      src_params->curAtCmd    = AT_CMD_CGCLASS;
      return (ATI_EXCT);
    default:
      cmdCmeError(CME_ERR_Unknown);         /*Command failed*/
      return (ATI_FAIL);
  }

#endif /*FF_ATI_BAT*/
}

GLOBAL T_ATI_RSLT queatPlusCGCLASS (char *cl, UBYTE srcId)
{
//#ifndef FF_ATI_BAT
  char *me="+CGCLASS: ";
  char *classes[] = {"A","B","CG","CC"};
  T_ACI_RETURN ret = AT_FAIL;
  T_CGCLASS_CLASS m_class = CGCLASS_CLASS_OMITTED;
//#endif

  TRACE_FUNCTION("queatPlusCGCLASS ()");
#ifdef FF_ATI_BAT
  {
  T_BAT_cmd_send cmd;
  T_BAT_no_parameter dummy;

  cmd.ctrl_params = BAT_CMD_QUE_PLUS_CGCLASS;
  dummy.bat_dummy = 0xFF;
  cmd.params.ptr_que_plus_cgclass = &dummy;
  bat_send(ati_bat_get_client(srcId), &cmd);
  return(ATI_EXCT);
  }
#else

  ret = qAT_PlusCGCLASS((T_ACI_CMD_SRC)srcId, &m_class);
  if (ret EQ AT_CMPL)
  {
    sprintf(g_sa,"%s\"%s\"", me, classes[m_class]);
    io_sendMessage(srcId, g_sa, ATI_NORMAL_OUTPUT);
  }
  else
  {
    cmdCmeError(CME_ERR_Unknown);
    return (ATI_FAIL);
  }

  return (map_aci_2_ati_rslt(ret));

#endif /*FF_ATI_BAT*/
}


/*
+--------------------------------------------------------------------+
| PROJECT : GPRS (8441)                 MODULE  : GACI_CMD           |
| STATE   : finnished                   ROUTINE : atPlusCGACT        |
+--------------------------------------------------------------------+

  PURPOSE : +CGACT command (context activation/deactivation)
*/

GLOBAL T_ATI_RSLT setatPlusCGACT (char *cl, UBYTE srcId)
{
  char *end;
  T_ACI_RETURN ret = AT_FAIL;
  T_CGACT_STATE state = CGACT_STATE_OMITTED;
  U8             count = 0;
  U8             cid;
  SHORT             cid_array[PDP_CONTEXT_CID_MAX + 1];
  T_ATI_SRC_PARAMS *src_params = find_element (ati_src_list, srcId, search_ati_src_id);

  TRACE_FUNCTION("setatPlusCGACT ()");

  end = cl;

  if ( *cl NEQ ',' )
    state = (T_CGACT_STATE) strtol(cl, &end, 10);

  while ( *end EQ ',' )
  {
    cl = end;           /* advace to the next delimiter */
    cl ++;              /* skip the delimiter */
    cid = PDP_CONTEXT_CID_INVALID;

    cid = (U8) strtol (cl, &end, 10);

    if (cid < PDP_CONTEXT_CID_MIN OR cid > PDP_CONTEXT_CID_MAX OR count > PDP_CONTEXT_CID_MAX)
    {
      cmdCmeError(CME_ERR_OpNotAllow);
      return (ATI_FAIL);
    }
    cid_array[count] = cid;
    count ++;
  }

  if(count<12)
  {
    cid_array[count] = PDP_CONTEXT_CID_INVALID;
  }
  cl = end;

#ifdef FF_ATI_BAT
  {
    T_BAT_cmd_send cmd;
    T_BAT_cmd_set_plus_cgact cgact;
    UBYTE n;

    cmd.ctrl_params = BAT_CMD_SET_PLUS_CGACT;
    cmd.params.ptr_set_plus_cgact=&cgact;

    /*
    *   This relies on T_BAT_plus_cgact_state and T_CGACT_STATE
    *   being identical.
    */
    cgact.state=(T_BAT_plus_cgact_state)state;

    for (n=0;((n<count) AND (n<BAT_MAX_PDP_CID));n++)
      cgact.cid[n]=(T_BAT_pdp_cid)cid_array[n];

    cgact.c_cid=(U8)n;
 
    bat_send(ati_bat_get_client(srcId), &cmd);

    src_params->curAtCmd=AT_CMD_CGACT;
    return(ATI_EXCT);
  }

#else

  /* (De) Activate necessary contexts */
  ret = sAT_PlusCGACT ((T_ACI_CMD_SRC)srcId, state, cid_array );
  switch (ret)
  {
    case (AT_CMPL):                         /*operation completed*/
      return (ATI_CMPL);
    case (AT_EXCT):
      src_params->curAtCmd    = AT_CMD_CGACT;
      return (ATI_EXCT);
    default:
      cmdCmeError(CME_ERR_Unknown);         /*Command failed*/
      return (ATI_FAIL);
  }

#endif /*FF_ATI_BAT*/
}

GLOBAL T_ATI_RSLT queatPlusCGACT (char *cl, UBYTE srcId)
{
#ifndef FF_ATI_BAT
  char *me="+CGACT: ";
  T_ACI_RETURN ret = AT_FAIL;
  U8    count = 0;
  SHORT    cid_array[PDP_CONTEXT_CID_MAX + 1];
  BOOL  states[PDP_CONTEXT_CID_MAX + 1];
#endif

  TRACE_FUNCTION("queatPlusCGACT ()");

#ifdef FF_ATI_BAT
  {
    T_BAT_cmd_send cmd;
    T_BAT_no_parameter dummy;

    cmd.ctrl_params = BAT_CMD_QUE_PLUS_CGACT;
    dummy.bat_dummy = 0xFF;
    cmd.params.ptr_que_plus_cgact = &dummy;
    bat_send(ati_bat_get_client(srcId), &cmd);
    return(ATI_EXCT);
  }
#else

  ret=qAT_PlusCGACT((T_ACI_CMD_SRC)srcId, states, cid_array);
  if (ret EQ AT_CMPL)
  {
    for( count = 0; count < PDP_CONTEXT_CID_MAX AND cid_array[count] NEQ PDP_CONTEXT_CID_INVALID; count++ )
    {
      sprintf(g_sa,"%s%hd,%d", me, cid_array[count], states[count] EQ TRUE? 1 : 0);
      io_sendMessage(srcId, g_sa, ATI_NORMAL_OUTPUT);
    }
  }
  else
  {
    cmdCmeError(CME_ERR_Unknown);
    return (ATI_FAIL);
  }

  return (map_aci_2_ati_rslt(ret));

#endif /*FF_ATI_BAT*/
}


/*
+--------------------------------------------------------------------+
| PROJECT : GPRS (8441)                 MODULE  : GACI_CMD           |
| STATE   : finnished                   ROUTINE : atPlusCGDATA       |
+--------------------------------------------------------------------+

  PURPOSE : +CGDATA command (data connect)
*/

GLOBAL T_ATI_RSLT setatPlusCGDATA (char *cl, UBYTE srcId)
{
  char L2P[MAX_L2P_LENGTH] = "", *end;
  T_ACI_RETURN ret = AT_FAIL;
  U8 count = 0, stop = 1;
  U8 cid_array[PDP_CONTEXT_CID_MAX];
  T_ATI_SRC_PARAMS *src_params = find_element (ati_src_list, srcId, search_ati_src_id);

  TRACE_FUNCTION("setatPlusCGDATA ()");

  memset( &cid_array, PDP_CONTEXT_CID_INVALID, sizeof(cid_array) );

  switch ( *cl )
  {
    case ',':
      /* the L2P is unspecified */
      *L2P = 0;
      break;
    case '\"':
      cl++;
      while (*cl AND stop)
      {
        if ( *cl NEQ '\"' AND count < MAX_L2P_LENGTH - 1 )
        {
          L2P[count] = *cl;
          count ++;
          cl    ++;
        }
        else
        {
          L2P[count] = 0;
          stop = 0;
        }
      }

      if ( *cl NEQ '\"' )
      { /* Bad user input, abort */
        cmdCmeError(CME_ERR_OpNotAllow);
        return (ATI_FAIL);
      }

      cl++;
      break;
    case 0:
      break;
    default:
      /* Bad user input, abort */
      cmdCmeError(CME_ERR_OpNotAllow);
      return (ATI_FAIL);
  }

  end   = cl;
  count = 0;
  while ( *end EQ ',' )
  {
    cl = end;
    cl ++;

    cid_array[count] = (U8) strtol (cl, &end, 10);

    if ( count >= PDP_CONTEXT_CID_MAX )
    {
      cmdCmeError(CME_ERR_OpNotAllow);
      return (ATI_FAIL);
    }
    count ++;
  }

  cl = end;
#ifdef FF_ATI_BAT

  {
    T_BAT_cmd_send cmd;
    T_BAT_cmd_set_plus_cgdata cgdata;
    UBYTE n;
    UBYTE len;

    cmd.ctrl_params = BAT_CMD_SET_PLUS_CGDATA;
    cmd.params.ptr_set_plus_cgdata=&cgdata;

    cgdata.v_cid=FALSE;

    if (count)
    {
      for (n=0;((n<count) AND (n<BAT_MAX_PDP_CID));n++)
        cgdata.cid[n]=(T_BAT_pdp_cid)cid_array[n];

      cgdata.c_cid=(U8)n;
      cgdata.v_cid=TRUE;
    }

    cgdata.v_l2p=FALSE;

    if (L2P[0]) 
    {
      if ((len=strlen(L2P))<=BAT_MAX_CGDATA_L2P_LEN)
      {
        cgdata.v_l2p=TRUE;
        memcpy(cgdata.l2p,L2P,len);
        cgdata.c_l2p=len;
      }
    }
    bat_send(ati_bat_get_client(srcId), &cmd);

    src_params->curAtCmd=AT_CMD_CGDATA;
    return(ATI_EXCT);
  }

#else


/*
 *  if no cid is given or if there is no matching context definition
 *  the function sAT_PlusCGDATA will process this
 */

  /* establish communication between the TE and the network */
  ret = sAT_PlusCGDATA ((T_ACI_CMD_SRC)srcId, L2P, cid_array );
  switch (ret)
  {
    case (AT_CMPL):                         /*operation completed*/
      return (ATI_CMPL);
    case (AT_EXCT):
      cmdErrStr   = NULL;
      src_params->curAtCmd    = AT_CMD_CGDATA;
      return (ATI_EXCT);
    default:
      cmdCmeError(CME_ERR_Unknown);         /*Command failed*/
      return (ATI_FAIL);
  }
#endif /*FF_ATI_BAT*/
}

/*
+--------------------------------------------------------------------+
| PROJECT : GPRS (8441)                 MODULE  : GACI_CMD           |
| STATE   : code                        ROUTINE : atPlusCGPADDR      |
+--------------------------------------------------------------------+

  PURPOSE : +CGPADDR command (show PDP addresses)
  active command returns a list of last used  PDP addresses
  for specified cids. If unspecified, returns list for all
  defined cids.
*/

GLOBAL T_ATI_RSLT setatPlusCGPADDR (char *cl, UBYTE srcId)
{
  char               *me = "+CGPADDR: ";
  char               *end;

  U8                 index = 0;
#ifndef FF_ATI_BAT
  T_ACI_RETURN       ret;
  T_NAS_ip           pdp_addresses[PDP_CONTEXT_CID_MAX];
#endif
  SHORT              cid_array[PDP_CONTEXT_CID_MAX + 1];

  TRACE_FUNCTION("setatPlusCGPADDR ()");

  do
  {
    cid_array[index] = (U8) strtol (cl, &end, 10);
    
    if (cid_array[index] EQ 0x00) 
      cid_array[index] = PDP_CONTEXT_CID_OMITTED;

    index++;
    cl = end;
    if ( *cl EQ ',' )
      cl++;
  }
  while ( *end EQ ',' );

  cid_array[index] = PDP_CONTEXT_CID_INVALID;
  cl = end;

  index = 0;

#ifdef FF_ATI_BAT
  {
    T_BAT_cmd_send             cmd;

    T_BAT_cmd_set_plus_cgpaddr cgpaddr;
    UBYTE n;

    cmd.ctrl_params = BAT_CMD_SET_PLUS_CGPADDR;
    cmd.params.ptr_set_plus_cgpaddr = &cgpaddr;

    cgpaddr.c_cid = (U8)index;

    if(cgpaddr.c_cid > 0)
    {
      for (n=0;((n<cgpaddr.c_cid) AND (n<BAT_MAX_PDP_CID));n++)
      {
        cgpaddr.cid[n]=(T_BAT_pdp_cid)cid_array[n];
      }

      cgpaddr.c_cid=(U8)n;
    }
 
    bat_send(ati_bat_get_client(srcId), &cmd);

    return(ATI_EXCT);
  }
#else
  /* return a list of PDP addresses for the specified context identifier */
  ret = sAT_PlusCGPADDR ((T_ACI_CMD_SRC)srcId, cid_array, pdp_addresses );
  
  if( ret EQ AT_CMPL )
  {
    while( cid_array[index] NEQ PDP_CONTEXT_CID_INVALID )
    {
      if (index NEQ 0) 
      {//to put a , between pdp adresses, if more than one exists.
        sprintf( g_sa, ",");
      }
      switch( pdp_addresses[index].ctrl_ip_address )
      {
        case NAS_is_ip_not_present:
          sprintf( g_sa, "%s%hd", me, cid_array[index] );
          break;
            
        case NAS_is_ipv4:
          sprintf( g_sa, "%s%hd,\"%d.%d.%d.%d\"", 
                   me, 
                   cid_array[index],
                   pdp_addresses[index].ip_address.ipv4_addr.a4[0],
                   pdp_addresses[index].ip_address.ipv4_addr.a4[1],
                   pdp_addresses[index].ip_address.ipv4_addr.a4[2],
                   pdp_addresses[index].ip_address.ipv4_addr.a4[3] );
          break;
            
        case NAS_is_ipv6:
          sprintf( g_sa, "%s%hd,\"%d.%d.%d.%d.%d.%d.%d.%d.%d.%d.%d.%d.%d.%d.%d.%d\"", 
                   me, 
                   cid_array[index],
                   pdp_addresses[index].ip_address.ipv6_addr.a6[ 0],
                   pdp_addresses[index].ip_address.ipv6_addr.a6[ 1],
                   pdp_addresses[index].ip_address.ipv6_addr.a6[ 2],
                   pdp_addresses[index].ip_address.ipv6_addr.a6[ 3],
                   pdp_addresses[index].ip_address.ipv6_addr.a6[ 4],
                   pdp_addresses[index].ip_address.ipv6_addr.a6[ 5],
                   pdp_addresses[index].ip_address.ipv6_addr.a6[ 6],
                   pdp_addresses[index].ip_address.ipv6_addr.a6[ 7],
                   pdp_addresses[index].ip_address.ipv6_addr.a6[ 8],
                   pdp_addresses[index].ip_address.ipv6_addr.a6[ 9],
                   pdp_addresses[index].ip_address.ipv6_addr.a6[10],
                   pdp_addresses[index].ip_address.ipv6_addr.a6[11],
                   pdp_addresses[index].ip_address.ipv6_addr.a6[12],
                   pdp_addresses[index].ip_address.ipv6_addr.a6[13],
                   pdp_addresses[index].ip_address.ipv6_addr.a6[14],
                   pdp_addresses[index].ip_address.ipv6_addr.a6[15] );
          break;
      }

      io_sendMessage(srcId, g_sa, ATI_NORMAL_OUTPUT);
      index ++;
    }
      
    return (ATI_CMPL);    
  }
  else
  {
    cmdCmeError(CME_ERR_Unknown);         /*Command failed*/
    return (ATI_FAIL);
  }
#endif /*FF_ATI_BAT*/
}






GLOBAL T_ATI_RSLT tesatPlusCGPADDR (char *cl, UBYTE srcId)
{
#ifndef FF_ATI_BAT
  char          *me="+CGPADDR: ";
  T_ACI_RETURN  ret;
  U8            index = 0;
  U8            cid_array[PDP_CONTEXT_CID_MAX + 1];
  int           pos = 0;
#endif

  TRACE_FUNCTION("tesatPlusCGPADDR ()");
#ifdef FF_ATI_BAT
  {
    T_BAT_cmd_send cmd;
    T_BAT_no_parameter dummy;

    cmd.ctrl_params = BAT_CMD_TST_PLUS_CGPADDR;
    dummy.bat_dummy = 0xFF;
    cmd.params.ptr_tst_plus_cgpaddr = &dummy;
    bat_send(ati_bat_get_client(srcId), &cmd);
    return(ATI_EXCT);
  }
#else

  /* return a list of defined context identifier */
  ret = tAT_PlusCGPADDR ((T_ACI_CMD_SRC)srcId, cid_array );
  switch (ret)
  {
    case (AT_CMPL):                         /*operation completed*/
      pos = sprintf(g_sa,"%s(", me);
      while( cid_array[index] NEQ PDP_CONTEXT_CID_INVALID )
      {
        if( index NEQ 0 )
        {
          g_sa[pos] = ','; pos++;
        }
        pos += sprintf(g_sa + pos,"%hd", cid_array[index]);
        index ++;
      }
      
      g_sa[pos] = ')'; pos++;
      g_sa[pos] = 0;
      io_sendMessage(srcId, g_sa, ATI_NORMAL_OUTPUT);
      return (ATI_CMPL);
      
    default:
      cmdCmeError(CME_ERR_Unknown);         /*Command failed*/
      return (ATI_FAIL);
  }
#endif /*FF_ATI_BAT*/
}

/*
+--------------------------------------------------------------------+
| PROJECT : GPRS (8441)                 MODULE  : GACI_CMD           |
| STATE   : finnished                   ROUTINE : atPlusCGAUTO       |
+--------------------------------------------------------------------+

  PURPOSE : +CGAUTO command (auto answer/modem compatibility control)
*/

GLOBAL T_ATI_RSLT setatPlusCGAUTO (char *cl, UBYTE srcId)
{
  T_ACI_RETURN ret = AT_FAIL;
  T_CGAUTO_N n = CGAUTO_N_OMITTED;       /* Invalid range value to detect an empty parameter */
  T_ATI_SRC_PARAMS *src_params = find_element (ati_src_list, srcId, search_ati_src_id);

  TRACE_FUNCTION("setatPlusCGAUTO ()");

  cl  =  parse (cl,"d", &n );
  ret = sAT_PlusCGAUTO ((T_ACI_CMD_SRC)srcId, n );
  switch (ret)
  {
    case (AT_CMPL):                         /*operation completed*/
      at.rngPrms.srcID_CGAUTO = srcId;
      return (ATI_CMPL);
    case (AT_EXCT):
      src_params->curAtCmd    = AT_CMD_CGATT;
      at.rngPrms.srcID_CGAUTO = srcId;
      return (ATI_EXCT);
    default:
      cmdCmeError(CME_ERR_Unknown);         /*Command failed*/
      return (ATI_FAIL);
  }
}

GLOBAL T_ATI_RSLT queatPlusCGAUTO (char *cl, UBYTE srcId)
{
  char *me="+CGAUTO: ";
  T_ACI_RETURN ret = AT_FAIL;
  T_CGAUTO_N n = CGAUTO_N_OMITTED;       /* Invalid range value to detect an empty parameter */

  TRACE_FUNCTION("queatPlusCGAUTO ()");

  ret = qAT_PlusCGAUTO((T_ACI_CMD_SRC)srcId, &n);
  if (ret EQ AT_CMPL)
  {
    sprintf(g_sa,"%s%hd", me, n);
    io_sendMessage(srcId, g_sa, ATI_NORMAL_OUTPUT);
  }
  else
  {
    cmdCmeError(CME_ERR_Unknown);
    return (ATI_FAIL);
  }

  return (map_aci_2_ati_rslt(ret));
}

/*
+--------------------------------------------------------------------+
| PROJECT : GPRS (8441)                 MODULE  : GACI_CMD           |
| STATE   : code                        ROUTINE : atPlusCGANS        |
+--------------------------------------------------------------------+

  PURPOSE : +CGANS command (answer NW context req)
*/

GLOBAL T_ATI_RSLT setatPlusCGANS   (char *cl, UBYTE srcId)
{
  char          L2P[MAX_L2P_LENGTH] = "";
  char         *end;
  T_ACI_RETURN  ret = AT_FAIL;
  U8            response;
  U8            cid      = PDP_CONTEXT_CID_OMITTED;
  U8            count    = 0;
  U8            stop     = 1;
  T_ATI_SRC_PARAMS *src_params = find_element (ati_src_list, srcId, search_ati_src_id);

  TRACE_FUNCTION("setatPlusCGANS ()");

/*
 * Process parameters
 */

  response = (U8) strtol (cl, &end, 10);
  cl = end;

  if( *cl EQ ',' )
  {
    cl++;
    switch ( *cl )
    {
      case ',':
        /* the L2P is unspecified/omitted */
        *L2P = 0;
        break;
      case '\"':
        cl++;
        while (*cl AND stop)
        {
          if ( *cl NEQ '\"' AND count < MAX_L2P_LENGTH - 1 )
          {
            L2P[count] = *cl;
            count ++;
            cl    ++;
          }
          else
          {
            L2P[count] = 0;
            stop = 0;
          }
        }

        if ( *cl NEQ '\"' )
        { /* Bad user input, abort */
          cmdCmeError(CME_ERR_OpNotAllow);
          return (ATI_FAIL);
        }

        cl++;
        break;
      default:
        /* Bad user input, abort */
        cmdCmeError(CME_ERR_OpNotAllow);
        return (ATI_FAIL);
    }
    
    if( *cl EQ ',' )
    {
      cl++; /* Remove the "," and convert the cid */
      cid = ( U8 ) strtol( cl, &end, 10 );
      cl = end;
    }
  }

#ifdef FF_ATI_BAT
  {
    T_BAT_cmd_send cmd;
    T_BAT_cmd_set_plus_cgans cgans;
    UBYTE len;

    cmd.ctrl_params = BAT_CMD_SET_PLUS_CGANS;
    cmd.params.ptr_set_plus_cgans=&cgans;

    cgans.cid=(T_BAT_pdp_cid)cid;

    /*
    *   This relies on T_BAT_plus_cgans_response and T_CGANS_RESPONSE
    *   being identical.
    */
    cgans.response=(T_BAT_plus_cgans_response)response;

    len=strlen(L2P);

    cgans.v_l2p=FALSE;

    if ((len) AND (len<=BAT_MAX_CGANS_L2P_LEN))
    {
      memcpy(cgans.l2p,L2P,len);
      cgans.c_l2p=(U8)len;
      cgans.v_l2p=TRUE;
    }
    bat_send(ati_bat_get_client(srcId), &cmd);

    src_params->curAtCmd=AT_CMD_CGANS;

    if ((at.rngPrms.isRng EQ TRUE) AND (at.rngPrms.mode EQ CRING_MOD_Gprs))
      ati_stop_ring();

    return(ATI_EXCT);
  }

#else


  /* return a list of PDP addresses for the specified context identifier */
  ret = sAT_PlusCGANS ((T_ACI_CMD_SRC)srcId, response, L2P, cid );
  switch (ret)
  {
    case (AT_CMPL):                         /*operation completed*/
      return (ATI_CMPL);

    case (AT_EXCT):
      src_params->curAtCmd    = AT_CMD_CGANS;
      if ( at.rngPrms.isRng EQ TRUE       AND
           at.rngPrms.mode  EQ CRING_MOD_Gprs )
      {
        ati_stop_ring();
      }
      return (ATI_EXCT);

    default:
      cmdCmeError(CME_ERR_Unknown);         /*Command failed*/
      return (ATI_FAIL);
  }
#endif /*FF_ATI_BAT*/
}

/*
+--------------------------------------------------------------------+
| PROJECT : GPRS (8441)                 MODULE  : GACI_CMD           |
| STATE   : code                        ROUTINE : atPlusCGEREP       |
+--------------------------------------------------------------------+

  PURPOSE : +CGEREP command
*/

GLOBAL T_ATI_RSLT setatPlusCGEREP (char *cl, UBYTE srcId)
{
  T_ACI_RETURN  ret;
  T_CGEREP_MODE mode  = CGEREP_MODE_OMITTED;
  T_CGEREP_BFR  bfr   = CGEREP_BFR_OMITTED;
  T_ATI_SRC_PARAMS *src_params = find_element (ati_src_list, srcId, search_ati_src_id);

  TRACE_FUNCTION("setatPlusCGEREP ()");

  cl=parse(cl,"dd",&mode, &bfr);

  ret = sAT_PlusCGEREP ( (T_ACI_CMD_SRC)srcId, mode, bfr );
  switch (ret)
  {
    case (AT_CMPL):                         /*operation completed*/
      return (ATI_CMPL);
    case (AT_EXCT):
      src_params->curAtCmd    = AT_CMD_CGEREP;
      return (ATI_EXCT);
    default:
      cmdCmeError(CME_ERR_Unknown);         /*Command failed*/
      return (ATI_FAIL);
  }
}

GLOBAL T_ATI_RSLT queatPlusCGEREP (char *cl, UBYTE srcId)
{
  char *me="+CGEREP: ";
  T_ACI_RETURN  ret;
  T_CGEREP_MODE mode  = CGEREP_MODE_OMITTED;
  T_CGEREP_BFR  bfr   = CGEREP_BFR_OMITTED;

  TRACE_FUNCTION("queatPlusCGEREP ()");

  ret=qAT_PlusCGEREP((T_ACI_CMD_SRC)srcId, &mode, &bfr);
  if (ret EQ AT_CMPL)
  {
    sprintf(g_sa,"%s%d,%d", me, mode, bfr);
    io_sendMessage(srcId, g_sa, ATI_NORMAL_OUTPUT);
  }
  else
  {
    cmdCmeError(CME_ERR_Unknown);
    return (ATI_FAIL);
  }

  return (map_aci_2_ati_rslt(ret));
}

/*
+--------------------------------------------------------------------+
| PROJECT : GPRS (8441)                 MODULE  : GACI_CMD           |
| STATE   : code                        ROUTINE : atPercentCGEREP       |
+--------------------------------------------------------------------+

  PURPOSE : %CGEREP command
*/

GLOBAL T_ATI_RSLT setatPercentCGEREP (char *cl, UBYTE srcId)
{
  TRACE_FUNCTION("setatPercentCGEREP ()");

  switch( *cl )
  {
    case '0':
        ati_user_output_cfg[srcId].Percent_CGEREP_stat = 0;
        return (ATI_CMPL);

    case '1':
        ati_user_output_cfg[srcId].Percent_CGEREP_stat = 1;
        return (ATI_CMPL);

    default:
      cmdCmeError(CME_ERR_OpNotAllow);
      return (ATI_FAIL);
   }
 }

GLOBAL T_ATI_RSLT queatPercentCGEREP (char *cl, UBYTE srcId)
{
  char *me="%CGEREP: ";
  UBYTE mode;

  TRACE_FUNCTION("queatPercentCGEREP ()");

  mode = ati_user_output_cfg[srcId].Percent_CGEREP_stat;

  sprintf(g_sa,"%s%d", me, mode);
  io_sendMessage(srcId, g_sa, ATI_NORMAL_OUTPUT);

  return (ATI_CMPL);
}


/*
+--------------------------------------------------------------------+
| PROJECT : GPRS (8441)                 MODULE  : GACI_CMD           |
| STATE   : code                        ROUTINE : setatPlusCGREG     |
+--------------------------------------------------------------------+

  PURPOSE : +CGREG command
*/

GLOBAL T_ATI_RSLT setatPlusCGREG (char *cl, UBYTE srcId)
{
  T_ATI_CREG_MOD mode=CREG_MOD_OFF;

  TRACE_FUNCTION("setatPlusCGREG()");

  cl=parse(cl,"d",&mode);

  switch(mode)
  {
    case CREG_MOD_OFF:
    case CREG_MOD_ON:
    case CREG_MOD_LOC_INF_ON:
      /* user wants to set mode: reset CREG related ATI parameters */
      ati_creg_init(srcId, PlusCGREG_CMD);
      ati_gprs_user_output_cfg[srcId].plus_cgreg.mod_lac_cid.pres_mode = mode;
      return ATI_CMPL;

    default:
      cmdCmeError(CME_ERR_OpNotAllow);
      return ATI_FAIL;
  }
}

GLOBAL T_ATI_RSLT queatPlusCGREG (char *cl, UBYTE srcId)
{
#ifndef FF_ATI_BAT
  T_CGREG_STAT   stat;
  USHORT         lac, cid;
  T_ATI_CREG_MOD mode;
  short          pos;
#endif

  TRACE_FUNCTION("queatPlusCGREG()");

#ifdef FF_ATI_BAT
  {
    T_BAT_cmd_send cmd;
    T_BAT_no_parameter dummy;

    cmd.ctrl_params = BAT_CMD_QUE_PLUS_CGREG;
    dummy.bat_dummy = 0xFF;
    cmd.params.ptr_que_plus_cgreg = &dummy;
    bat_send(ati_bat_get_client(srcId), &cmd);
    return(ATI_EXCT);
  }
#else

  mode = ati_gprs_user_output_cfg[srcId].plus_cgreg.mod_lac_cid.pres_mode;

  if (qAT_PlusCGREG((T_ACI_CMD_SRC)srcId, &stat, &lac, &cid) NEQ AT_CMPL)
  {
    cmdCmeError(CME_ERR_Unknown);
    return ATI_FAIL;
  }

  pos = sprintf(g_sa, "+CGREG: %d,%d", mode, stat);

  if(mode EQ CREG_MOD_LOC_INF_ON)
  {
    if(stat EQ CGREG_STAT_REG_HOME  OR  stat EQ CGREG_STAT_REG_ROAM)
      sprintf(g_sa+pos, ",\"%04X\",\"%04X\"", lac, cid);
  }

  io_sendMessage(srcId, g_sa, ATI_NORMAL_OUTPUT);
  return ATI_CMPL;
#endif /*FF_ATI_BAT*/
}


/*
+--------------------------------------------------------------------+
| PROJECT : GPRS (8441)                 MODULE  : GACI_CMD           |
| STATE   : code                        ROUTINE : setatPercentCGREG  |
+--------------------------------------------------------------------+

  PURPOSE : %CGREG command
*/

GLOBAL T_ATI_RSLT setatPercentCGREG (char *cl, UBYTE srcId)
{
  T_ATI_CREG_MOD mode=CREG_MOD_OFF;

  TRACE_FUNCTION("setatPercentCGREG()");

  cl=parse(cl,"d",&mode);

  switch(mode)
  {
    case CREG_MOD_OFF:
    case CREG_MOD_ON:
    case CREG_MOD_LOC_INF_ON:
    case CREG_MOD_LOC_INF_ON_CTXACT:
      /* user wants to set mode: reset CREG related ATI parameters */
      ati_creg_init(srcId, PercentCGREG_CMD);
      ati_gprs_user_output_cfg[srcId].percent_cgreg.mod_lac_cid.pres_mode = mode;
      return ATI_CMPL;

    default:
      cmdCmeError(CME_ERR_OpNotAllow);
      return ATI_FAIL;
  }
}

GLOBAL T_ATI_RSLT queatPercentCGREG (char *cl, UBYTE srcId)
{
#ifndef FF_ATI_BAT
  T_P_CGREG_STAT  stat;
  USHORT          lac, cid;
  T_ATI_CREG_MOD  mode;
  SHORT           pos;
#endif

  TRACE_FUNCTION("queatPercentCGREG()");
#ifdef FF_ATI_BAT
  {
    T_BAT_cmd_send cmd;
    T_BAT_no_parameter dummy;

    cmd.ctrl_params = BAT_CMD_QUE_PERCENT_CGREG;
    dummy.bat_dummy = 0xFF;
    cmd.params.ptr_que_plus_cgreg = &dummy;

    bat_send(ati_bat_get_client(srcId), &cmd);
    return(ATI_EXCT);
  } 
#else

  mode = ati_gprs_user_output_cfg[srcId].percent_cgreg.mod_lac_cid.pres_mode;

  if (qAT_PercentCGREG((T_ACI_CMD_SRC)srcId, &stat, &lac, &cid) NEQ AT_CMPL)
  {
    cmdCmeError(CME_ERR_Unknown);
    return ATI_FAIL;
  }

  pos = sprintf(g_sa,"%s: %d,%d", "%CGREG", mode, stat);

  if (mode EQ CREG_MOD_LOC_INF_ON  OR  mode EQ CREG_MOD_LOC_INF_ON_CTXACT)
  {
    if (stat EQ P_CGREG_STAT_REG_HOME  OR  stat EQ P_CGREG_STAT_REG_ROAM)
      pos += sprintf(g_sa+pos, ",\"%04X\",\"%04X\"", lac, cid);
    else if (mode EQ CREG_MOD_LOC_INF_ON_CTXACT)
      pos += sprintf(g_sa+pos, ",,");   /* if CTXACT follows so lac and cid are ommitted */
  }

  if (mode EQ CREG_MOD_LOC_INF_ON_CTXACT)
    sprintf(g_sa+pos, (cmhSM_isContextActive() ? ",1" : ",0"));

  io_sendMessage(srcId, g_sa, ATI_NORMAL_OUTPUT);
  return ATI_CMPL;
#endif /*FF_ATI_BAT*/
}


/*
+--------------------------------------------------------------------+
| PROJECT : GPRS (8441)                 MODULE  : GACI_CMD           |
| STATE   : code                        ROUTINE : atPlusCGSMS        |
+--------------------------------------------------------------------+

  PURPOSE : +CGSMS command (define MO SMS behaviour)
*/

GLOBAL T_ATI_RSLT setatPlusCGSMS (char *cl, UBYTE srcId)
{
#ifndef FF_ATI_BAT
  T_ACI_RETURN    ret;
#endif
  T_CGSMS_SERVICE service = CGSMS_SERVICE_OMITTED;  /* Invalid range value to detect an empty parameter */
  T_ATI_SRC_PARAMS *src_params = find_element (ati_src_list, srcId, search_ati_src_id);

  TRACE_FUNCTION("setatPlusCGSMS ()");

  cl=parse(cl,"d",&service);
#ifdef FF_ATI_BAT

  {
    T_BAT_cmd_send cmd;
    T_BAT_cmd_set_plus_cgsms cgsms;

    cmd.ctrl_params = BAT_CMD_SET_PLUS_CGSMS;
    cmd.params.ptr_set_plus_cgsms=&cgsms;

    /*
    *   This relies on T_BAT_plus_cgsms_service and T_CGSMS_SERVICE
    *   being identical.
    */
    cgsms.service=(T_BAT_plus_cgsms_service)service;

    bat_send(ati_bat_get_client(srcId), &cmd);

    src_params->curAtCmd=AT_CMD_CGSMS;
    return(ATI_EXCT);
  }

#else
  if( !cl )
  {
    cmdCmeError(CME_ERR_OpNotAllow);
    return (ATI_FAIL);
  }

  ret = sAT_PlusCGSMS ( (T_ACI_CMD_SRC)srcId, service );
  switch (ret)
  {
    case (AT_CMPL):                         /*operation completed*/
      return (ATI_CMPL);
    case (AT_EXCT):
      src_params->curAtCmd    = AT_CMD_CGSMS;
      return (ATI_EXCT);
    default:
      cmdCmeError(CME_ERR_Unknown);         /*Command failed*/
      return (ATI_FAIL);
  }
#endif /*FF_ATI_BAT*/
}

GLOBAL T_ATI_RSLT queatPlusCGSMS (char *cl, UBYTE srcId)
{
#ifndef FF_ATI_BAT
  char *me="+CGSMS: ";
  T_ACI_RETURN    ret;
  T_CGSMS_SERVICE service = CGSMS_SERVICE_OMITTED;  /* Invalid range value to detect an empty parameter */
#endif

  TRACE_FUNCTION("queatPlusCGSMS ()");
#ifdef FF_ATI_BAT
  {
    T_BAT_cmd_send cmd;
    T_BAT_no_parameter dummy;

    cmd.ctrl_params = BAT_CMD_QUE_PLUS_CGSMS;
    dummy.bat_dummy = 0xFF;
    cmd.params.ptr_que_plus_cgsms = &dummy;
    bat_send(ati_bat_get_client(srcId), &cmd);
    return(ATI_EXCT);
  }
#else

  ret = qAT_PlusCGSMS((T_ACI_CMD_SRC)srcId, &service);
  if (ret EQ AT_CMPL)
  {
    sprintf(g_sa,"%s%hd", me, service);
    io_sendMessage(srcId, g_sa, ATI_NORMAL_OUTPUT);
    return (ATI_CMPL);
  }
  else
  {
    cmdCmeError(CME_ERR_Unknown);
    return (ATI_FAIL);
  }
#endif /*FF_ATI_BAT*/
}

GLOBAL T_ATI_RSLT tesatPlusCGSMS (char *cl, UBYTE srcId)
{
  char *me="+CGSMS: ", list[10] = "(0-3)";
  T_ACI_RETURN    ret;
  T_CGSMS_SERVICE service = CGSMS_SERVICE_OMITTED;  /* Invalid range value to detect an empty parameter */
  SHORT           service_list;

  TRACE_FUNCTION("tesatPlusCGSMS ()");

  ret = tAT_PlusCGSMS ( (T_ACI_CMD_SRC)srcId, &service_list );
  switch (ret)
  {
  case (AT_CMPL):                         /*operation completed*/
    if ( service_list NEQ 15 )
    {
      list[1] = service EQ 1 ? '0':'1';
      list[2] = ')';
      list[3] = 0;
    }
    cl++;
    sprintf(g_sa,"%s%s", me, list);
    io_sendMessage(srcId, g_sa, ATI_NORMAL_OUTPUT);
    return (ATI_CMPL);
  default:
    cmdCmeError(CME_ERR_Unknown);         /*Command failed*/
    return (ATI_FAIL);
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GPRS (8441)                 MODULE  : GACI_CMD           |
| STATE   : finnished                   ROUTINE : atGD               |
+--------------------------------------------------------------------+

  PURPOSE : +ATD command
*/

GLOBAL T_ATI_RSLT atGD (char *cl, UBYTE srcId, BOOL *gprs_command)
{

  enum
  {
    ATD_CMD_NOT_VALID = 0,
    ATD_CMD_98        = 1,
    ATD_CMD_99        = 2
  } atd_cmd_type = ATD_CMD_NOT_VALID;


  U8            i          = 0;

  U8            star_count = 0; 

  char          tmp_cmd[40];
  char          *p_tmp_cmd = tmp_cmd;
  char          c_addr[MAX_B_SUBSCR_NUM_LEN];
  char          L2P[MAX_L2P_LENGTH];
  U8            cid_array[PDP_CONTEXT_CID_MAX];
  U8            cid_length = PDP_CONTEXT_CID_MAX + 1;

#if defined (FF_WAP) OR defined (FF_SAT_E)
  BOOL          src_flag = FALSE;
#endif

  T_PDP_TYPE    pdp_type = "IP";
  T_PDP_CONTEXT pdp_context;
  T_ACI_RETURN ret = AT_FAIL;
  T_PDP_CONTEXT_INTERNAL *p_pdp_context_node = NULL;

  T_ATI_SRC_PARAMS *src_params = find_element (ati_src_list, srcId, search_ati_src_id);

  TRACE_FUNCTION("atGD ()");
  
  TRACE_EVENT ("ati_gprs, atGD, start");
  //TRACE_EVENT (cl);
  
  memset( L2P, 0, sizeof( L2P ) );
  memset( cid_array, PDP_CONTEXT_CID_INVALID, sizeof( cid_array ) );
  cid_array[0] = PDP_CONTEXT_CID_OMITTED;

  memset( tmp_cmd, 0, sizeof(tmp_cmd) );

#if defined (FF_WAP) OR defined (FF_SAT_E)
    if( (ati_is_src_type((UBYTE)srcId, ATI_SRC_TYPE_TST) EQ TRUE) OR
        (srcId EQ CMD_SRC_LCL) )
    {
      TRACE_EVENT ("Source Found is LOCAL");
      /* if call not yet marked as WAP call, mark it now */
      if (Wap_Call)
      {
        TRACE_EVENT ("ati_gprs, atGD, WAP Call for atd*98# OR atd*99#");
        src_flag = TRUE;
      }
    }  
#endif

  if (cl EQ NULL)
  {
    *gprs_command = FALSE;
    return (ATI_FAIL);
  }

  /* ignore modifier */
  switch ( *cl )
  {
    case 'T':
    case 't':
    case 'P':
    case 'p':
      if ( *(cl + 1) EQ '*' )
      {
        cl+=2;
      }
      else
      {
        *gprs_command = FALSE;
        return (ATI_FAIL);
      }
      break;
      
    case '*':
      cl++;
      break;
      
    default:
      *gprs_command = FALSE;
      return (ATI_FAIL);
  }

  do
  {
    switch( *(cl + i) )
    {
      case '*': 
        tmp_cmd[i] = ','; 
        star_count++;  
        break;

      case '#': 
        tmp_cmd[i] = 0;
        break;

      default : 
        tmp_cmd[i] = *(cl + i); 
        break;
          }

    i++;
  }
  while( *(cl + i) NEQ '#' AND i < 40 );

  if( i EQ 40 )
  {
    *gprs_command = FALSE;
    return (ATI_FAIL);
  }

  
  if( tmp_cmd[0] EQ '9' AND
      tmp_cmd[1] EQ '8' )
  {
    /* Character after *98 should be * or # for GPRS; otherwise
       this is USSD (refer issue 16289) */
    if( tmp_cmd[2] NEQ ',' AND
        tmp_cmd[2] NEQ 0   )
    {
      *gprs_command = FALSE;
      return (ATI_FAIL);
    }

    if ( tmp_cmd[2] NEQ 0  )
      p_tmp_cmd = parse( &tmp_cmd[3], "i", &cid_length, &cid_array );
    
    if(p_tmp_cmd)
      atd_cmd_type = ATD_CMD_98;
  }
  else if( tmp_cmd[0] EQ '9' AND
           tmp_cmd[1] EQ '9' )
  {
    /* Character after *99 should be * or # for GPRS; otherwise
       this is USSD (refer issue 16289) */
    if( tmp_cmd[2] NEQ ',' AND
        tmp_cmd[2] NEQ 0   )
    {
      *gprs_command = FALSE;
      return (ATI_FAIL);
    }

    switch( star_count )
    {
      case 1:
        tmp_cmd[i]   = ',';
        tmp_cmd[i+1] = ',';
        tmp_cmd[i+1] = 0;
        break;

      case 2:
        tmp_cmd[i]   = ',';
        tmp_cmd[i+1] = 0;
        break;
    }

    if ( tmp_cmd[2] NEQ 0  )
      p_tmp_cmd = parse( &tmp_cmd[3], "aai", (LONG)MAX_B_SUBSCR_NUM_LEN, c_addr,
                                             (LONG)MAX_L2P_LENGTH, L2P, 
                                             &cid_length, &cid_array );
    if(p_tmp_cmd)
      atd_cmd_type = ATD_CMD_99;
  }
  else
  {
    *gprs_command = FALSE;
    return (ATI_FAIL);
  }


  switch( atd_cmd_type )
  {
    case ATD_CMD_98:
    {  
      strcpy( L2P, "PPP" );
      strcpy( pdp_type, "IP" );

      break;
    }

    case ATD_CMD_99:
    {
      if( ! strcmp(L2P,"") )
      {
        /* L2P is omitted use PPP as L2 protocol*/
        strcpy( L2P, "PPP" );
      }
      
      else if( !(strcmp(L2P, "1") AND strcmp(L2P,"PPP")) )
      {
        strcpy( L2P, "PPP" );
      }
            
#if defined(FF_PKTIO) OR defined(FF_TCP_IP) OR defined (FF_PSI)
      else if( !(strcmp(L2P, "901") AND strcmp(L2P, "M-PKT")) )
      {
        strcpy( L2P, "M-PKT" );
      }
#endif /* FF_PKTIO or FF_TCP_IP or FF_PSI */
      else
      {
        cmdCmeError(CME_ERR_OpNotAllow);
        return (ATI_FAIL);
      }
              
      strcpy( pdp_type, "IP" );

      break;
    }
  }

  if( !cl )
  {
    cmdCmeError(CME_ERR_OpNotAllow);
    *gprs_command = FALSE;
    return (ATI_FAIL);
  }

#if defined (FF_WAP) OR defined (FF_SAT_E)
  if (src_flag)
  {
    memset (work_cids, PDP_CONTEXT_CID_INVALID, PDP_CONTEXT_CID_MAX);
    cid_pointer=0;
  }
#endif
  /*
   * If the cid is omitted a matching defined context is found or a new context is created.
   */
  if( cid_array[0] EQ PDP_CONTEXT_CID_OMITTED )
  {
    p_pdp_context_node = p_pdp_context_list;
    while(p_pdp_context_node AND
          strcmp(p_pdp_context_node->attributes.pdp_type, pdp_type) AND
          p_pdp_context_node->internal_data.state NEQ PDP_CONTEXT_STATE_DEFINED )
    {
      p_pdp_context_node = p_pdp_context_node->p_next;
    }
    if(p_pdp_context_node)
    {
      cid_array[0] = p_pdp_context_node->cid;

#if defined (FF_WAP) OR defined (FF_SAT_E)
      if (src_flag)
      {
        work_cids[cid_pointer] = cid_array[0];
      }
#endif
    }
    else
    {
      cid_array[0] = pdp_context_get_free_cid();
      
      memcpy( &pdp_context, &pdp_context_default.attributes, sizeof(T_PDP_CONTEXT) );
      memcpy(  pdp_context.pdp_type, pdp_type, sizeof(T_PDP_TYPE));

      sAT_PlusCGDCONT((T_ACI_CMD_SRC) srcId, cid_array[0], &pdp_context );

#if defined (FF_WAP) OR defined (FF_SAT_E)
      if (src_flag)
      {
        work_cids[cid_pointer] = cid_array[0];
      }
#endif
    }
  }
  else
  {
    i=0;
    while( cid_array[i] NEQ PDP_CONTEXT_CID_INVALID )
    {
      p_pdp_context_node = pdp_context_find_node_from_cid( cid_array[i] );

      if( p_pdp_context_node )
      {
        if( strcmp( p_pdp_context_node->attributes.pdp_type, pdp_type ) )
          strcpy( p_pdp_context_node->attributes.pdp_type, pdp_type );

#if defined (FF_WAP) OR defined (FF_SAT_E)
        if (src_flag)
        {
          work_cids[i] = cid_array[i];
          cid_pointer = i;
        }
#endif
      }
      else
      {
        memcpy( &pdp_context, &pdp_context_default.attributes, sizeof(T_PDP_CONTEXT) );
        memcpy(  pdp_context.pdp_type, pdp_type, sizeof(T_PDP_TYPE));
          
        if( sAT_PlusCGDCONT((T_ACI_CMD_SRC) srcId, cid_array[i], &pdp_context ) EQ AT_FAIL )
        {
          *gprs_command = FALSE;
          return (ATI_FAIL); 
        }

#if defined (FF_WAP) OR defined (FF_SAT_E)
        if (src_flag)
        {
          work_cids[i] = cid_array[i];
          cid_pointer = i;
        }
#endif
      }
      i++;
    }
  }

  *gprs_command = TRUE;

#if defined (FF_WAP) OR defined (FF_SAT_E)
  if (src_flag)
  {
    BYTE  options;
    SRCC_LINK_NO link_no = SRCC_INVALID_LINK;
    /* process WAP over GPRS */

  /*
   *-------------------------------------------------------------------
   * check entity status
   *-------------------------------------------------------------------
   */
    if( smEntStat.curCmd NEQ AT_CMD_NONE )
    {
      return( ATI_BUSY );
    }


    if(is_gpf_tcpip_call()) {
      GPF_TCPIP_STATEMENT(link_no = SRCC_TCPIP_SNDCP_LINK);
    }
    else {
      link_no = SRCC_IP_SNDCP_LINK;
    }
    /* check number of context */
    if ( TRUE NEQ srcc_reserve_sources( link_no, 1 ) )
    {
      TRACE_EVENT ("srcc_reserve_sources failed!");
      cmdCmeError(CME_ERR_Unknown);
      return ( ATI_FAIL );
    }

    /* get dti id */
    srcc_new_count(link_no);
    
    wap_dti_id = dti_cntrl_new_dti(DTI_DTI_ID_NOTPRESENT);
    p_pdp_context_node = pdp_context_find_node_from_cid( cid_array[0] );
    p_pdp_context_node->internal_data.link_id_new =
      dti_conn_compose_link_id (0,0, wap_dti_id, DTI_TUPLE_NO_NOTPRESENT );
    p_pdp_context_node->internal_data.owner = ( T_ACI_CMD_SRC)srcId;

    TRACE_EVENT_P1 ("ati_gprs, atGD, link_id_new = %0X", p_pdp_context_node->internal_data.link_id_new);

    /* set IP options */
    if(is_gpf_tcpip_call()) {
      options = TCPIP_ACT_OPTION_V4 | TCPIP_ACT_OPTION_TCPIP; 
    }
    else {
      options = TCPIP_ACT_OPTION_V4 | TCPIP_ACT_OPTION_UDP;
    }

    /* send request to ACI WAP to activate WAP */
    smShrdPrm.owner  = srcId;
    smEntStat.entOwn = (T_ACI_CMD_SRC)smShrdPrm.owner;
    smEntStat.curCmd = AT_CMD_CGDATA;

    gpppEntStat.curCmd = AT_CMD_CGDATA;
    gpppEntStat.entOwn = ( T_ACI_CMD_SRC)srcId;
    gpppShrdPrm.owner  = (UBYTE) srcId;

    if(src_params)
    {
      src_params->curAtCmd = AT_CMD_CGDATA;
    }

    psaTCPIP_Activate( srcId, wap_dti_id,
                       wapId, options, TCPIP_CONNECTION_TYPE_GPRS_WAP,
                       cmhSM_IP_activate_cb);
    return (ATI_EXCT);
  }
#endif /* of WAP OR SAT E */

  if( cmhSM_sAT_A(( T_ACI_CMD_SRC) srcId, &ret ) EQ TRUE )
  {
    switch (ret)
    {
      case (AT_CMPL):                 /* Operation completed */
        return (ATI_CMPL);
      
      case (AT_EXCT):
/*      curAtCmd    = AT_CMD_CGANS; */
        return (ATI_EXCT);
        
      default:
        cmdCmeError(CME_ERR_Unknown); /* Command failed */
        return (ATI_FAIL);
    }
  }
  else
  {

    ret = sAT_PlusCGDATA ((T_ACI_CMD_SRC) srcId, L2P, cid_array );
    switch (ret)
    {
      case (AT_CMPL):                 /* Operation completed */
        return (ATI_CMPL);
        
      case (AT_EXCT):
        if(src_params)
        {        
          src_params->curAtCmd = AT_CMD_CGDATA;
        }
        return (ATI_EXCT);

      case (AT_BUSY):
        cmdCmeError(CME_ERR_Unknown);         /*Command failed*/
        return (ATI_FAIL);
        
      default:
        cmdCmeError(CME_ERR_NotPresent);         /*Command failed*/
        return (ATI_FAIL);
    }
  }
}


/*
+--------------------------------------------------------------------+
| PROJECT : GPRS (8441)                 MODULE  : GACI_CMD           |
| STATE   : finished                    ROUTINE : setatPercentCGAATT |
+--------------------------------------------------------------------+

  PURPOSE : %CGAATT command ( GPRS automatic attach mode )
*/

GLOBAL T_ATI_RSLT setatPercentCGAATT (char *cl, UBYTE srcId)
{

  T_ACI_RETURN ret = AT_FAIL;
  T_CGAATT_ATTACH_MODE att_m = CGAATT_ATTACH_MODE_OMITTED;
  T_CGAATT_DETACH_MODE det_m = CGAATT_DETACH_MODE_OMITTED;

  TRACE_FUNCTION("setatPercentCGAATT ()");

  cl=parse(cl,"dd",&att_m, &det_m);

#ifdef FF_ATI_BAT
  {
    T_BAT_cmd_send cmd;
    T_BAT_cmd_set_percent_cgaatt cgaatt;

    cmd.ctrl_params = BAT_CMD_SET_PERCENT_CGAATT;
    cmd.params.ptr_set_percent_cgaatt=&cgaatt;

    /*
    *   This relies on T_CGAATT_ATTACH_MODE and T_GGAATT_DETACH_MODE being
    *   identical to T_BAT_percent_cgaatt_att_m and T_BAT_percent_cgaatt_det_m.
    */
    cgaatt.att_m=(T_BAT_percent_cgaatt_att_m)att_m;
    cgaatt.det_m=(T_BAT_percent_cgaatt_det_m)det_m;

    bat_send(ati_bat_get_client(srcId), &cmd);
    return(ATI_EXCT);
  }

#else

  if( !cl )
  {
    cmdCmeError(CME_ERR_OpNotAllow);
    return (ATI_FAIL);
  }

  ret = sAT_PercentCGAATT ((T_ACI_CMD_SRC)srcId, att_m, det_m );
  switch (ret)
  {
    case (AT_CMPL):                         /*operation completed*/
      return (ATI_CMPL);
    case (AT_EXCT):
    default:
      cmdCmeError(CME_ERR_Unknown);         /*Command failed*/
      return (ATI_FAIL);
  }
#endif /*FF_ATI_BAT*/
}

GLOBAL T_ATI_RSLT queatPercentCGAATT (char *cl, UBYTE srcId)
{
#ifndef FF_ATI_BAT
  T_ACI_RETURN    ret;
  T_CGAATT_ATTACH_MODE att_m = CGAATT_ATTACH_MODE_OMITTED;
  T_CGAATT_DETACH_MODE det_m = CGAATT_DETACH_MODE_OMITTED;
#endif

  TRACE_FUNCTION("atPercentCGAATT ()");
#ifdef FF_ATI_BAT
  {
    T_BAT_cmd_send cmd;
    T_BAT_no_parameter dummy;

    cmd.ctrl_params = BAT_CMD_QUE_PERCENT_CGAATT;
    dummy.bat_dummy = 0xFF;
    cmd.params.ptr_que_percent_cgaatt = &dummy;
    bat_send(ati_bat_get_client(srcId), &cmd);
    return(ATI_EXCT);
  }
#else

  ret = qAT_PercentCGAATT((T_ACI_CMD_SRC)srcId, &att_m, &det_m);
  if (ret EQ AT_CMPL)
  {
    sprintf(g_sa,"%s: %d,%d", "%CGAATT", att_m, det_m);
    io_sendMessage(srcId, g_sa, ATI_NORMAL_OUTPUT);
	return (ATI_CMPL);
  }
  else
  {
    cmdCmeError(CME_ERR_Unknown);
    return (ATI_FAIL);
  }
#endif /*FF_ATI_BAT*/
}

/*
+--------------------------------------------------------------------+
| PROJECT : GPRS (8441)                 MODULE  : GACI_CMD           |
| STATE   : finished                    ROUTINE : setatPercentSNCNT  |
+--------------------------------------------------------------------+

  PURPOSE : %SNCNT command ( SNDCP Counter )
            = 0: reset SNCNT counter.
            ?: ask for current count.
*/

GLOBAL T_ATI_RSLT setatPercentSNCNT (CHAR *cl, UBYTE srcId)
{
  T_ACI_RETURN ret = AT_CMPL;

  TRACE_FUNCTION("setatPercentSNCNT ()");

  if( *cl NEQ '0' )
  {
    cmdCmeError(CME_ERR_OpNotAllow);
    return (ATI_FAIL);
  }

  cl++;
#ifdef FF_ATI_BAT

  {
    T_BAT_cmd_send cmd;
    T_BAT_cmd_set_percent_sncnt sncnt;

    cmd.ctrl_params = BAT_CMD_SET_PERCENT_SNCNT;
    cmd.params.ptr_set_percent_sncnt=&sncnt;

    sncnt.rst=BAT_P_SNCNT_RESET_RESET;

    bat_send(ati_bat_get_client(srcId), &cmd);
    return(ATI_EXCT);
  }

#else

  /* AT%SNCNT = 0 resets counter */
  ret = sAT_PercentSNCNT((T_ACI_CMD_SRC)srcId, TRUE );

  return (map_aci_2_ati_rslt(ret));
#endif /*FF_ATI_BAT*/
}

GLOBAL T_ATI_RSLT queatPercentSNCNT (CHAR *cl, UBYTE srcId)
{
  T_ACI_RETURN ret = AT_CMPL;

  TRACE_FUNCTION("queatPercentSNCNT ()");

#ifdef FF_ATI_BAT
  {
    T_BAT_cmd_send cmd;
    T_BAT_no_parameter dummy;

    cmd.ctrl_params = BAT_CMD_QUE_PERCENT_SNCNT;
    dummy.bat_dummy = 0xFF;
    cmd.params.ptr_que_percent_sncnt = &dummy;
    bat_send(ati_bat_get_client(srcId), &cmd);
    return(ATI_EXCT);
  }
#else

  /* AT%SNCNT? returns current counter without resetting */
  ret = sAT_PercentSNCNT( (T_ACI_CMD_SRC)srcId, FALSE );

  return (map_aci_2_ati_rslt(ret));
#endif /*FF_ATI_BAT*/
}

/*
+--------------------------------------------------------------------+
| PROJECT : GPRS (8441)                 MODULE  : GACI_CMD           |
| STATE   : finished                    ROUTINE : setatPercentCGMM   |
+--------------------------------------------------------------------+

  PURPOSE : %CGMM command ( Configure GMM )
            %CGMM = cipher_on, tlli_handling
*/

GLOBAL T_ATI_RSLT setatPercentCGMM(CHAR *cl, UBYTE srcId)
{
  T_ACI_RETURN ret = AT_CMPL;
  USHORT cipher_on, tlli_handling;

  TRACE_FUNCTION("setatPercentCGMM()");

  cl=parse(cl, "rr", &cipher_on, &tlli_handling);
  if(!cl)
  {
    cmdCmeError(CME_ERR_OpNotAllow);
    return (ATI_FAIL);
  }
#ifdef FF_ATI_BAT
  {
  T_BAT_cmd_send cmd;
  T_BAT_cmd_set_percent_cgmm my_bat_set_percent_cgmm;

  TRACE_FUNCTION("setatPercentCGMM() calls bat_send() <=== as APPLICATION");

  memset(&my_bat_set_percent_cgmm, 0, sizeof(my_bat_set_percent_cgmm));
  cmd.ctrl_params = BAT_CMD_SET_PERCENT_CGMM;
  cmd.params.ptr_set_percent_cgmm = &my_bat_set_percent_cgmm;

  my_bat_set_percent_cgmm.cipher_on = cipher_on;
  my_bat_set_percent_cgmm.tlli_handling = tlli_handling;

  bat_send(ati_bat_get_client(srcId), &cmd);

  return ATI_EXCT; /* executing, because response is passed by callback function */
  }
#else /* OLD FUNCTION BODY */

  TRACE_FUNCTION("setatPercentCGMM()");

  ret = sAT_PercentCGMM( (T_ACI_CMD_SRC)srcId, (UBYTE)cipher_on, (UBYTE)tlli_handling );

  if (ret NEQ AT_CMPL)
  {
    cmdCmeError(CME_ERR_Unknown);
    return (ATI_FAIL);
  }
  return (ATI_CMPL);
#endif /* no FF_ATI_BAT*/
}


T_ATI_RSLT setatPercentCGPCO_HEX (UBYTE srcId, USHORT cid,
                                    UBYTE *pco_hex_string, USHORT len)
{
  UBYTE pco_len;
  UBYTE   *pco_array;




  TRACE_FUNCTION("setatPercentCGPCO_HEX():");
  /* 
   * The functionality of the command is changed due to the 
   * introduction of dynamic data structures for PCO.
   */

  ACI_MALLOC (pco_array, ACI_PCO_MAX_LEN + 1);

  pco_len = (UBYTE) utl_HexStrToBin( pco_hex_string, len, pco_array, ACI_PCO_MAX_LEN + 1 );

  if(sAT_PercentCGPCO_HEX ((T_ACI_CMD_SRC)srcId, (U8) cid, pco_array, pco_len) EQ AT_FAIL )
  {
    cmdCmeError (CME_ERR_Unknown);
    ACI_MFREE (pco_array);
    return (ATI_FAIL);
  }

  ACI_MFREE (pco_array);
  return (ATI_CMPL);  

}



T_ACI_RETURN setatPercentCGPCO_ASCII (UBYTE srcId, U8 cid, UBYTE *pco_ascii_string, USHORT len)
{
  CHAR          aut_prot[5];
  CHAR          user[MAX_USER_LEN+1];   /* + '\0' */
  CHAR          pwd[MAX_PSWD_LEN+1];     /* + '\0' */
  CHAR          dns1[MAX_DNS_LEN];
  CHAR          dns2[MAX_DNS_LEN];
  CHAR          *cl;

  TRACE_FUNCTION("setatPercentCGPCO_ASCII():");

  /* Initialize arrays */
  user[0] = '\0';
  pwd[0] = '\0';
  dns1[0] = '\0'; 
  dns2[0] = '\0'; 

  cl = parse ((CHAR*)pco_ascii_string, "qqqqq",
              sizeof (aut_prot), aut_prot,
              sizeof (user), user, sizeof (pwd), pwd, sizeof(dns1), dns1, sizeof(dns2), dns2);

  if(cl EQ NULL)
  {
    cmdCmeError (CME_ERR_InvalidTxtChar);
    return ((T_ACI_RETURN)ATI_FAIL);
  }
  strupper (aut_prot);
  if (strcmp (aut_prot, "PAP"))
  {
    cmdCmeError (CME_ERR_OpNotSupp);
    return ((T_ACI_RETURN)ATI_FAIL);
  }

  if (sAT_PercentCGPCO ((T_ACI_CMD_SRC)srcId, cid,
                        ACI_PCO_AUTH_PROT_PAP, user, pwd, dns1, dns2) EQ AT_FAIL)
  {
    cmdCmeError (CME_ERR_Unknown);
    return ((T_ACI_RETURN)ATI_FAIL);
  }
  
  return ((T_ACI_RETURN)ATI_CMPL);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GPRS                         MODULE  : ATI_GPRS          |
| STATE   : finished                     ROUTINE : ipaddr_to_string  |
+--------------------------------------------------------------------+

  PURPOSE : print the IP address (little endian!) to the supplied string.

*/
GLOBAL void ipaddr_to_string(ULONG ipaddr, char *ipaddr_string)
{
  UBYTE *addrbyte_p ;        /* Pointer to single address bytes. */
  
  if (ipaddr_string)
  {
    addrbyte_p = (UBYTE *) &ipaddr ;
    if (ipaddr NEQ 0)
    {
      /* This is for little endian *only*! */
      sprintf(ipaddr_string,"%u.%u.%u.%u",
              addrbyte_p[3], addrbyte_p[2], addrbyte_p[1], addrbyte_p[0]);
    }
    else
    {
      *ipaddr_string = '\0' ;
    }
  }
}


/*
+--------------------------------------------------------------------+
| PROJECT : GPRS                        MODULE  : GACI_CMD           |
| STATE   : finished                    ROUTINE : queatPercentCGPCO  |
+--------------------------------------------------------------------+

  PURPOSE : %CGPCO command
              * query network pco for cid
*/

LOCAL T_ATI_RSLT queatPercentCGPCO (UBYTE srcId, USHORT cid)
{
  char *me="%CGPCO:";
  ULONG gateway,dns1,dns2;
  char dns1_str[sizeof("255.255.255.255")];
  char dns2_str[sizeof("255.255.255.255")];
  char gatew_str[sizeof("255.255.255.255")];
  USHORT i;                     /* Counter for loop over context IDs. */

  TRACE_FUNCTION("queatPercentCGPCO( )");

  if( cid  EQ PDP_CONTEXT_CID_OMITTED )  /* for all available cid's: recurse */
  {
    for( i=1; i<=PDP_CONTEXT_CID_MAX; i++ )
    {
      queatPercentCGPCO(srcId, i);
    }
  }
  else /* for a specific cid */
  {
    gateway = dns1 = dns2 = 0 ; /* Just in case... */
    
    if( qAT_PercentCGPCO((T_ACI_CMD_SRC)srcId, &gateway, &dns1, &dns2, cid) EQ AT_CMPL )
    {
      ipaddr_to_string(dns1, dns1_str);
      ipaddr_to_string(dns2, dns2_str);
      ipaddr_to_string(gateway, gatew_str);
      sprintf( g_sa, "%s \"%s,%s,%s\",%d",
               me, dns1_str, dns2_str, gatew_str, cid );
    
      io_sendMessage (srcId, g_sa, ATI_NORMAL_OUTPUT);
      
      memset(g_sa,0,MAX_CMD_LEN);
    }
   }
   return (ATI_CMPL);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GPRS                        MODULE  : GACI_CMD           |
| STATE   : finished                    ROUTINE : setatPercentCGPCO  |
+--------------------------------------------------------------------+

  PURPOSE : %CGPCO command
              * set the PCO for context activation
*/

GLOBAL T_ATI_RSLT setatPercentCGPCO(CHAR *cl, UBYTE srcId)
{
  U8                  cid = PDP_CONTEXT_CID_OMITTED;
  T_ACI_CGPCO_FORMAT  format;
  T_ATI_RSLT          ret = ATI_FAIL;
  CHAR               *pco_string;
  T_ACI_CGPCO_MODE    mode;

  TRACE_FUNCTION("setatPercentCGPCO()");

  ACI_MALLOC (pco_string, ACI_PCO_MAX_LEN * 2 + 1);

  cl = parse(cl, "ddsr",&mode, &format, ACI_PCO_MAX_LEN * 2 + 1, pco_string, &cid);
  if(cl EQ NULL)
  {
    cmdCmeError (CME_ERR_OpNotAllow);
    ACI_MFREE (pco_string);
    return (ATI_FAIL);
  }
  
  if( !((cid >= PDP_CONTEXT_CID_MIN AND cid <= PDP_CONTEXT_CID_MAX) OR cid EQ PDP_CONTEXT_CID_OMITTED) )
  {
    cmdCmeError (CME_ERR_OpNotSupp);
    ACI_MFREE (pco_string);
    return (ATI_FAIL);
  }

#ifdef FF_ATI_BAT
{

    T_BAT_cmd_send cmd;
    T_BAT_cmd_set_percent_cgpco cgpco;
    SHORT len;
    cmd.ctrl_params = BAT_CMD_SET_PERCENT_CGPCO;
    cmd.params.ptr_set_percent_cgpco = &cgpco;

    cgpco.cid=(T_BAT_pdp_cid)cid;
    cgpco.v_pco=FALSE;

    ret=ATI_EXCT;

    switch (mode)
    {
      case CGPCO_MODE_QUERY_PCO:
        cgpco.mode=BAT_P_CGPCO_MODE_QUERY_PCO;
        cgpco.format=(T_BAT_percent_cgpco_format)format;
        break;

      case CGPCO_MODE_SET_PCO:
        cgpco.mode=BAT_P_CGPCO_MODE_SET_PCO;
        cgpco.format=(T_BAT_percent_cgpco_format)format;

        len=strlen(pco_string);

        if (len>BAT_MAX_CGPCO_PCO_LEN)
        {
          ret=ATI_FAIL;
        }
        else
        {
          memcpy(cgpco.pco,pco_string,len);
          cgpco.c_pco=(U8)len;
          cgpco.v_pco=TRUE;
        }
        break;

      default:
        cmdCmeError(CME_ERR_OpNotSupp);
        ret=ATI_FAIL;
        break;
    }

    if (ret EQ ATI_EXCT)
    {
      bat_send(ati_bat_get_client(srcId), &cmd);
    }
} /* FF_ATI_BAT */
#else

    switch (mode)
    {
      case CGPCO_MODE_QUERY_PCO:

        ret = queatPercentCGPCO (srcId,cid);
        break;
      
    case CGPCO_MODE_SET_PCO:
    {
        switch (format)
        {
          case CGPCO_FORMAT_HEX:
          ret = (T_ATI_RSLT)setatPercentCGPCO_HEX (srcId,cid, (UBYTE*)pco_string, (USHORT)strlen (pco_string));
            break;

          case CGPCO_FORMAT_ASCII:
            ret = (T_ATI_RSLT)setatPercentCGPCO_ASCII (srcId, cid, (UBYTE*)pco_string, (USHORT)strlen (pco_string));
            break;

         default:
            cmdCmeError (CME_ERR_OpNotSupp);
          ret = ATI_FAIL;
            break;
         }
    }
    break;
    
    default:
      cmdCmeError (CME_ERR_OpNotSupp);
      ret = ATI_FAIL;
      break;   
    }
#endif

  ACI_MFREE (pco_string);

  return (ret);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GPRS                        MODULE  : GACI_CMD           |
| STATE   : finished                    ROUTINE : setatPercentCGPPP  |
+--------------------------------------------------------------------+

  PURPOSE : %CGPPP command
              * set PPP values in GPRS case

PPP_AP_NO                      0x0         no authentication
PPP_AP_PAP                     0x1         password authentication protocol
PPP_AP_CHAP                    0x2         challenge handshake authentication protocol
PPP_AP_AUTO                    0x3         automatic authentication

*/

GLOBAL T_ATI_RSLT setatPercentCGPPP (CHAR *cl, UBYTE srcId)
{
  T_ACI_PPP_PROT  protocol;
  T_ACI_RETURN    ret = AT_FAIL;

  TRACE_FUNCTION("setatPercentCGPPP( )");

  cl=parse(cl,"d",&protocol);
  if(!cl)
  {
    cmdCmeError(CME_ERR_OpNotAllow);
    return (ATI_FAIL);
  }
#ifdef FF_ATI_BAT
  {
  T_BAT_cmd_send cmd;
  T_BAT_cmd_set_percent_cgppp my_bat_set_percent_cgppp;

  TRACE_FUNCTION("setatPercentCGPPP() calls bat_send() <=== as APPLICATION");

  memset(&my_bat_set_percent_cgppp, 0, sizeof(my_bat_set_percent_cgppp));
  cmd.ctrl_params = BAT_CMD_SET_PERCENT_CGPPP;
  cmd.params.ptr_set_percent_cgppp = &my_bat_set_percent_cgppp;

  my_bat_set_percent_cgppp.protocol = protocol;

  bat_send(ati_bat_get_client(srcId), &cmd);

  return ATI_EXCT; /* executing, because response is passed by callback function */
  }
#else /* OLD FUNCTION BODY */

  TRACE_FUNCTION("setatPercentCGPPP( )");

  ret = sAT_PercentCGPPP((T_ACI_CMD_SRC)srcId, protocol);
  if (ret EQ AT_FAIL)
  {
    cmdCmeError(CME_ERR_Unknown);
    return (ATI_FAIL);
  }
  return (map_aci_2_ati_rslt(ret));
#endif /* no FF_ATI_BAT*/
}

/*
+--------------------------------------------------------------------+
| PROJECT : GPRS                        MODULE  : GACI_CMD           |
| STATE   : finished                    ROUTINE : queatPercentCGPPP  |
+--------------------------------------------------------------------+

  PURPOSE : %CGPPP command
              * query PPP values in GPRS case

PPP_AP_NO                      0x0         no authentication
PPP_AP_PAP                     0x1         password authentication protocol
PPP_AP_CHAP                    0x2         challenge handshake authentication protocol
PPP_AP_AUTO                    0x3         automatic authentication

*/

GLOBAL T_ATI_RSLT queatPercentCGPPP (CHAR *cl, UBYTE srcId)
{
#ifndef FF_ATI_BAT
  T_ACI_PPP_PROT  protocol;
  T_ACI_RETURN    ret;
#endif

  TRACE_FUNCTION("queatPercentCGPPP( )");
#ifdef FF_ATI_BAT
  {
    T_BAT_cmd_send cmd;
    T_BAT_no_parameter dummy;

    cmd.ctrl_params = BAT_CMD_QUE_PERCENT_CGPPP;
    dummy.bat_dummy = 0xFF;
    cmd.params.ptr_que_percent_cgppp = &dummy;
    bat_send(ati_bat_get_client(srcId), &cmd);
    return(ATI_EXCT);
  }
#else

  ret = qAT_PercentCGPPP((T_ACI_CMD_SRC)srcId, &protocol);
  if (ret EQ AT_FAIL)
  {
    cmdCmeError(CME_ERR_Unknown);
    return (ATI_FAIL);
  }

  sprintf (g_sa, "%s: %d", "%CGPPP", protocol);
  io_sendMessage (srcId, g_sa, ATI_NORMAL_OUTPUT);

  return (map_aci_2_ati_rslt(ret));
#endif /*FF_ATI_BAT*/
}
/*
+--------------------------------------------------------------------+
| PROJECT : GPRS (8441)                 MODULE  : GACI_CMD           |
| STATE   : finished                    ROUTINE : atPercentCGCLASS   |
+--------------------------------------------------------------------+

  PURPOSE : %CGCLASS command  (GPRS class change)
*/

GLOBAL T_ATI_RSLT setatPercentCGCLASS (CHAR *cl, UBYTE srcId)
{
#ifndef FF_ATI_BAT
  T_ACI_RETURN ret;
#endif
  char test_class[3]="";
  T_PERCENT_CGCLASS m_class = PERCENT_CGCLASS_OMITTED;

  TRACE_FUNCTION("setatPercentCGCLASS ()");

  cl = parse (cl,"s", (LONG)3, &test_class );

  switch (*test_class)
  {
    case 'A':
      break;
    case 'B':
      switch(*(test_class + 1))
      {
        case 'G':
          m_class = PERCENT_CGCLASS_BG;
          break;
        case 'C':
          m_class = PERCENT_CGCLASS_BC;
          break;
        case 'X':
          m_class = PERCENT_CGCLASS_BX;
          break;
        case '\0':
          m_class = PERCENT_CGCLASS_B;
          break;
        default:
          break;
      }
      break;
    case 'C':
      switch(*(test_class + 1))
      {
        case 'G':
          m_class = PERCENT_CGCLASS_CG;
          break;
        case 'C':
          m_class = PERCENT_CGCLASS_CC;
          break;
        default:
          break;
      }
      break;
    default:
      break;
  }

  if ( !cl OR m_class EQ PERCENT_CGCLASS_OMITTED )
  {
    cmdCmeError(CME_ERR_OpNotAllow);
    return (ATI_FAIL);
  }
#ifdef FF_ATI_BAT

  {
    T_BAT_cmd_send cmd;
    T_BAT_cmd_set_percent_cgclass cgclass;

    cmd.ctrl_params = BAT_CMD_SET_PERCENT_CGCLASS;
    cmd.params.ptr_set_percent_cgclass=&cgclass;

    /*
    *   BAT and ACI versions of mobile class are different enough
    *   to require this conversion.
    */

    /*
    *   This relies on T_BAT_p_mobile_class and T_PERCENT_CGCLASS
    *   being identical.
    */
    cgclass.p_mobile_class=(T_BAT_p_mobile_class)m_class;

    bat_send(ati_bat_get_client(srcId), &cmd);
    return(ATI_EXCT);
  }

#else

  ret = sAT_PercentCGCLASS ( (T_ACI_CMD_SRC)srcId, m_class );
  if (ret EQ AT_FAIL)
  {
     cmdCmeError (CME_ERR_OpNotAllow);
     return (ATI_FAIL);
  }
  return (map_aci_2_ati_rslt(ret));
#endif
}

GLOBAL T_ATI_RSLT queatPercentCGCLASS (CHAR *cl, UBYTE srcId)
{
#ifndef FF_ATI_BAT
  char *classes[] = {"A","B","BG","BC","BX","CG","CC"};
  T_ACI_RETURN ret;
  T_PERCENT_CGCLASS m_class, currentClass;
#endif

  TRACE_FUNCTION("queatPercentCGCLASS ()");
#ifdef FF_ATI_BAT
  {
    T_BAT_cmd_send cmd;
    T_BAT_no_parameter dummy;

    cmd.ctrl_params = BAT_CMD_QUE_PERCENT_CGCLASS;
    dummy.bat_dummy = 0xFF;
    cmd.params.ptr_que_plus_cgclass = &dummy;
    bat_send(ati_bat_get_client(srcId), &cmd);
    return(ATI_EXCT);
  }
#else

  ret = qAT_PercentCGCLASS((T_ACI_CMD_SRC)srcId, &m_class, &currentClass);
  if (ret EQ AT_CMPL)
  {
    if(m_class NEQ currentClass)
    {
      sprintf(g_sa,"%s: \"%s\",\"%s\"", "%CGCLASS", classes[m_class], classes[currentClass]);
    }
    else
    {
      sprintf(g_sa,"%s: \"%s\"", "%CGCLASS", classes[m_class]);
    }
    io_sendMessage(srcId, g_sa, ATI_NORMAL_OUTPUT);
  }

  return (map_aci_2_ati_rslt(ret));
#endif /*FF_ATI_BAT*/
}

#ifdef REL99
/*
+--------------------------------------------------------------------+
| PROJECT : UMTS                        MODULE  : GACI_CMD           |
| STATE   : devellopment                ROUTINE : setatPlusCGEQREQ   |
+--------------------------------------------------------------------+

  PURPOSE : +CGEQREQ command
            Set 3G Quality of Service Profile (Requested)
*/
GLOBAL T_ATI_RSLT setatPlusCGEQREQ( char *cl, UBYTE srcId )
{
  T_ACI_RETURN ret;
  T_PS_qos     input_qos;
  U8           cid;
#ifdef FF_ATI_BAT
  T_ATI_SRC_PARAMS *src_params = find_element (ati_src_list, srcId, search_ati_src_id);
#endif
  TRACE_FUNCTION("setatPlusCGEQREQ ()");

  cid = PDP_CONTEXT_CID_OMITTED;

  input_qos.qos_r99.tc           = QOS_R99_TC_OMITTED;
  input_qos.qos_r99.order        = QOS_R99_ORDER_OMITTED;
  input_qos.qos_r99.del_err_sdu  = QOS_R99_DEL_ERR_SDU_OMITTED;
  input_qos.qos_r99.max_sdu      = QOS_R99_MAX_SDU_OMITTED;
  input_qos.qos_r99.max_rate_ul  = QOS_R99_MAX_BR_UL_OMITTED;
  input_qos.qos_r99.max_rate_dl  = QOS_R99_MAX_BR_DL_OMITTED;
  input_qos.qos_r99.xfer_delay   = QOS_R99_XFER_DELAY_OMITTED;
  input_qos.qos_r99.handling_pri = QOS_R99_HANDLING_PRIO_OMITTED;
  input_qos.qos_r99.guar_br_ul   = QOS_R99_GUAR_BR_UL_OMITTED;
  input_qos.qos_r99.guar_br_dl   = QOS_R99_GUAR_BR_DL_OMITTED;
  input_qos.qos_r99.ber.ratio_mant = QOS_R99_RATIO_MANT_OMITTED;        /* Residual bit error ratio */
  input_qos.qos_r99.ber.ratio_exp  = QOS_R99_RATIO_EXP_OMITTED;
  input_qos.qos_r99.sdu_err_ratio.ratio_mant = QOS_R99_RATIO_MANT_OMITTED;
  input_qos.qos_r99.sdu_err_ratio.ratio_exp  = QOS_R99_RATIO_EXP_OMITTED;

  cl = parse( cl,"ccrrrrcreecrc", &cid,
                                  &input_qos.qos_r99.tc,
                                  &input_qos.qos_r99.max_rate_ul,
                                  &input_qos.qos_r99.max_rate_dl,
                                  &input_qos.qos_r99.guar_br_ul,
                                  &input_qos.qos_r99.guar_br_dl,
                                  &input_qos.qos_r99.order,
                                  &input_qos.qos_r99.max_sdu,
                                  &input_qos.qos_r99.sdu_err_ratio,
                                  &input_qos.qos_r99.ber,            /* Residual bit error ratio */
                                  &input_qos.qos_r99.del_err_sdu,
                                  &input_qos.qos_r99.xfer_delay,
                                  &input_qos.qos_r99.handling_pri );
  if( !cl )
  {
    cmdCmeError(CME_ERR_OpNotAllow);
    return (ATI_FAIL);
  }

#ifdef FF_ATI_BAT
  {
    T_BAT_cmd_send cmd;
    T_BAT_cmd_set_plus_cgeqreq cgeqreq;
    cmd.ctrl_params  = BAT_CMD_SET_PLUS_CGEQREQ;
    cmd.params.ptr_set_plus_cgeqreq = &cgeqreq;
    
    cgeqreq.cid          = cid;
    cgeqreq.tc           = input_qos.qos_r99.tc;
    cgeqreq.max_rate_ul  = input_qos.qos_r99.max_rate_ul;
    cgeqreq.max_rate_dl  = input_qos.qos_r99.max_rate_dl;
    cgeqreq.guar_br_ul   = input_qos.qos_r99.guar_br_ul;
    cgeqreq.guar_br_dl   = input_qos.qos_r99.guar_br_dl; 
    cgeqreq.order        = input_qos.qos_r99.order;
    cgeqreq.max_sdu      = input_qos.qos_r99.max_sdu;
    
    /* sdu_err_ratio */
    if (input_qos.qos_r99.sdu_err_ratio.ratio_mant NEQ 
        QOS_R99_RATIO_MANT_OMITTED)
    {
      cgeqreq.v_sdu_err_ratio    = TRUE;
      cgeqreq.c_sdu_err_ratio    = 2;
      cgeqreq.sdu_err_ratio[0]   = input_qos.qos_r99.sdu_err_ratio.ratio_mant; 
      if (input_qos.qos_r99.sdu_err_ratio.ratio_exp NEQ 
          QOS_R99_RATIO_EXP_OMITTED)
        cgeqreq.sdu_err_ratio[1] = input_qos.qos_r99.sdu_err_ratio.ratio_exp;
      else
        cgeqreq.sdu_err_ratio[1] = 1;
    }
    else
    {
      cgeqreq.v_sdu_err_ratio = FALSE;
      cgeqreq.c_sdu_err_ratio = 0;      
    }

    /* ber */
    if (input_qos.qos_r99.ber.ratio_mant NEQ QOS_R99_RATIO_MANT_OMITTED)
    {
      cgeqreq.v_ber    = TRUE;
      cgeqreq.c_ber    = 2;
      cgeqreq.ber[0]   = input_qos.qos_r99.ber.ratio_mant; 
      if (input_qos.qos_r99.ber.ratio_exp NEQ QOS_R99_RATIO_EXP_OMITTED)
        cgeqreq.ber[1] = input_qos.qos_r99.ber.ratio_exp;
      else
        cgeqreq.ber[1] = 1;
    }
    else
    {
      cgeqreq.v_ber = FALSE;
      cgeqreq.c_ber = 0;      
    }
    
    cgeqreq.del_err_sdu      = input_qos.qos_r99.del_err_sdu;
    cgeqreq.xfer_delay       = input_qos.qos_r99.xfer_delay;
    cgeqreq.handling_pri     = input_qos.qos_r99.handling_pri;

    bat_send(ati_bat_get_client(srcId), &cmd);
    src_params->curAtCmd=AT_CMD_CGEQREQ;
    return(ATI_EXCT);
}
#else
  ret = (T_ACI_RETURN)sAT_PlusCGEQREQ ((T_ACI_CMD_SRC)srcId, (U8)cid ,&input_qos); 
  switch (ret)
  {
    case (AT_CMPL):                         /*operation completed*/
      return (ATI_CMPL);
    case (AT_EXCT):
//      curAtCmd    = AT_CMD_CGEQREQ;
      return (ATI_EXCT);
    default:
      cmdCmeError(CME_ERR_Unknown);         /*Command failed*/
      return (ATI_FAIL);
  }
#endif /* FF_ATI_BAT */
}

/*
+--------------------------------------------------------------------+
| PROJECT : UMTS                        MODULE  : GACI_CMD           |
| STATE   : devellopment                ROUTINE : setatPlusCGEQMIN   |
+--------------------------------------------------------------------+

  PURPOSE : +CGEQMIN command
            Set 3G Quality of Service Profile (Minimum)
*/
GLOBAL T_ATI_RSLT setatPlusCGEQMIN( char *cl, UBYTE srcId )
{
  T_ACI_RETURN ret;
  T_PS_qos     input_qos;
  U8           cid;
#ifdef FF_ATI_BAT
  T_ATI_SRC_PARAMS *src_params = find_element (ati_src_list, srcId, search_ati_src_id);
#endif
  TRACE_FUNCTION("setatPlusCGEQMIN ()");

  cid = PDP_CONTEXT_CID_OMITTED;

  input_qos.qos_r99.tc           = QOS_R99_TC_OMITTED;
  input_qos.qos_r99.order        = QOS_R99_ORDER_OMITTED;
  input_qos.qos_r99.del_err_sdu  = QOS_R99_DEL_ERR_SDU_OMITTED;
  input_qos.qos_r99.max_sdu      = QOS_R99_MAX_SDU_OMITTED;
  input_qos.qos_r99.max_rate_ul  = QOS_R99_MAX_BR_UL_OMITTED;
  input_qos.qos_r99.max_rate_dl  = QOS_R99_MAX_BR_DL_OMITTED;
  input_qos.qos_r99.xfer_delay   = QOS_R99_XFER_DELAY_OMITTED;
  input_qos.qos_r99.handling_pri = QOS_R99_HANDLING_PRIO_OMITTED;
  input_qos.qos_r99.guar_br_ul   = QOS_R99_GUAR_BR_UL_OMITTED;
  input_qos.qos_r99.guar_br_dl   = QOS_R99_GUAR_BR_DL_OMITTED;
  input_qos.qos_r99.ber.ratio_mant = QOS_R99_RATIO_MANT_OMITTED;        /* Residual bit error ratio */
  input_qos.qos_r99.ber.ratio_exp  = QOS_R99_RATIO_EXP_OMITTED;
  input_qos.qos_r99.sdu_err_ratio.ratio_mant = QOS_R99_RATIO_MANT_OMITTED;
  input_qos.qos_r99.sdu_err_ratio.ratio_exp  = QOS_R99_RATIO_EXP_OMITTED;

  cl = parse( cl,"ccrrrrcreecrc", &cid,
                                  &input_qos.qos_r99.tc,
                                  &input_qos.qos_r99.max_rate_ul,
                                  &input_qos.qos_r99.max_rate_dl,
                                  &input_qos.qos_r99.guar_br_ul,
                                  &input_qos.qos_r99.guar_br_dl,
                                  &input_qos.qos_r99.order,
                                  &input_qos.qos_r99.max_sdu,
                                  &input_qos.qos_r99.sdu_err_ratio,
                                  &input_qos.qos_r99.ber,            /* Residual bit error ratio */
                                  &input_qos.qos_r99.del_err_sdu,
                                  &input_qos.qos_r99.xfer_delay,
                                  &input_qos.qos_r99.handling_pri );
  if( !cl )
  {
    cmdCmeError(CME_ERR_OpNotAllow);
    return (ATI_FAIL);
  }
 
#ifdef FF_ATI_BAT
{
    T_BAT_cmd_send cmd;
    T_BAT_cmd_set_plus_cgeqmin cgeqmin;
    cmd.ctrl_params                 = BAT_CMD_SET_PLUS_CGEQMIN;
    cmd.params.ptr_set_plus_cgeqmin = &cgeqmin;
    
    cgeqmin.cid          = (T_BAT_pdp_cid)cid;
    cgeqmin.tc           = input_qos.qos_r99.tc;
    cgeqmin.max_rate_ul  = input_qos.qos_r99.max_rate_ul;
    cgeqmin.max_rate_dl  = input_qos.qos_r99.max_rate_dl;
    cgeqmin.guar_br_ul   = input_qos.qos_r99.guar_br_ul;
    cgeqmin.guar_br_dl   = input_qos.qos_r99.guar_br_dl; 
    cgeqmin.order        = input_qos.qos_r99.order;
    cgeqmin.max_sdu      = input_qos.qos_r99.max_sdu;
    
    /* sdu_err_ratio */
    if (input_qos.qos_r99.sdu_err_ratio.ratio_mant NEQ 
        QOS_R99_RATIO_MANT_OMITTED)
    {
      cgeqmin.v_sdu_err_ratio    = TRUE;
      cgeqmin.c_sdu_err_ratio    = 2;
      cgeqmin.sdu_err_ratio[0]   = input_qos.qos_r99.sdu_err_ratio.ratio_mant; 
      if (input_qos.qos_r99.sdu_err_ratio.ratio_exp NEQ 
          QOS_R99_RATIO_EXP_OMITTED)
        cgeqmin.sdu_err_ratio[1] = input_qos.qos_r99.sdu_err_ratio.ratio_exp;
      else
        cgeqmin.sdu_err_ratio[1] = 1;
    }
    else
    {
      cgeqmin.v_sdu_err_ratio = FALSE;
      cgeqmin.c_sdu_err_ratio = 0;      
    }

    /* ber */
    if (input_qos.qos_r99.ber.ratio_mant NEQ QOS_R99_RATIO_MANT_OMITTED)
    {
      cgeqmin.v_ber    = TRUE;
      cgeqmin.c_ber    = 2;
      cgeqmin.ber[0]   = input_qos.qos_r99.ber.ratio_mant; 
      if (input_qos.qos_r99.ber.ratio_exp NEQ QOS_R99_RATIO_EXP_OMITTED)
        cgeqmin.ber[1] = input_qos.qos_r99.ber.ratio_exp;
      else
        cgeqmin.ber[1] = 1;
    }
    else
    {
      cgeqmin.v_ber = FALSE;
      cgeqmin.c_ber = 0;      
    }
    
    cgeqmin.del_err_sdu      = input_qos.qos_r99.del_err_sdu;
    cgeqmin.xfer_delay       = input_qos.qos_r99.xfer_delay;
    cgeqmin.handling_pri     = input_qos.qos_r99.handling_pri;

    bat_send(ati_bat_get_client(srcId), &cmd);
    src_params->curAtCmd=AT_CMD_CGEQMIN;
    return(ATI_EXCT);
}
#else
  ret = sAT_PlusCGEQMIN ((T_ACI_CMD_SRC)srcId, cid ,&input_qos); 
  switch (ret)
  {
    case (AT_CMPL):                         /*operation completed*/
      return (ATI_CMPL);
    case (AT_EXCT):
//      curAtCmd    = AT_CMD_CGEQMIN;
      return (ATI_EXCT);
    default:
      cmdCmeError(CME_ERR_Unknown);         /*Command failed*/
      return (ATI_FAIL);
  }
#endif /* FF_ATI_BAT */

}

/*
+--------------------------------------------------------------------+
| PROJECT : UMTS                        MODULE  : ATI_GPRS           |
| STATE   : finished                    ROUTINE : setatPlusCGEQNEG   |
+--------------------------------------------------------------------+

  PURPOSE : Query of 3G QoS Profile (Negotiated)
*/
GLOBAL T_ATI_RSLT setatPlusCGEQNEG (char *cl, UBYTE srcId)
{
  char        *me="+CGEQNEG: ";
  T_PS_qos     pdef_qos;
  int          i;
  BOOL         qos_valid;
  U8           cid_array[PDP_CONTEXT_CID_MAX+1], current_cid;
  int          no_of_cids;
  char         *end;
#ifdef FF_ATI_BAT
  T_ATI_SRC_PARAMS *src_params = find_element (ati_src_list, srcId, search_ati_src_id);
#endif
  TRACE_FUNCTION("setatPlusCGEQNEG ()");

  /* Parse all the cids into the cid array */
  no_of_cids = 0;

  --cl; /* Decreased due to no comma the first time! */
  do
  {
    /* Remove the "," */
    cl++;

    /* Set the entry to omitted. */
    cid_array[no_of_cids] = PDP_CONTEXT_CID_OMITTED;

    /* Parse the value */
    cid_array[no_of_cids] = ( U8 ) strtol (cl, &end, 10);
    cl = end;

    if( cid_array[no_of_cids] EQ 0 )
      cid_array[no_of_cids] = PDP_CONTEXT_CID_OMITTED;
    
    /* If a cid is omitted no cids must be specified at all. */
    if( cid_array[no_of_cids] EQ PDP_CONTEXT_CID_OMITTED )
    {
      if( no_of_cids NEQ 0 )
      {
        /* Don't accept omitted cids inside if not the first. (AT+CGCMOD=1,2,,3 is not allowed). */
        cmdCmeError(CME_ERR_OpNotSupp);
        return (ATI_FAIL);
      }
      /* Break the while */
      break;
    }
    if( (!cl) OR (cid_array[no_of_cids] < PDP_CONTEXT_CID_MIN) OR (cid_array[no_of_cids] > PDP_CONTEXT_CID_MAX) OR (no_of_cids >= PDP_CONTEXT_CID_MAX) )
    {
      cmdCmeError(CME_ERR_OpNotSupp);
      return(ATI_FAIL);
    }
    no_of_cids++;
  } while ( *cl EQ ',' );
  cid_array[no_of_cids] = PDP_CONTEXT_CID_OMITTED;

#ifdef FF_ATI_BAT
{
  T_BAT_cmd_send cmd;
  T_BAT_cmd_set_plus_cgeqneg cgeqneg;
  UBYTE n;
  cmd.ctrl_params = BAT_CMD_SET_PLUS_CGEQNEG;
  cmd.params.ptr_set_plus_cgeqneg=&cgeqneg;

  for (n=0;((n<no_of_cids) AND (n<BAT_MAX_PDP_CID));n++)

    cgeqneg.cid[n]=(T_BAT_pdp_cid)cid_array[n];
    bat_send(ati_bat_get_client(srcId), &cmd);
    src_params->curAtCmd=AT_CMD_CGEQNEG;
    return(ATI_EXCT);
}
#else
  /* Calculate number of cids: If the first cid is omitted it is treated as all cids. */
  if (cid_array[0] EQ PDP_CONTEXT_CID_OMITTED)
    no_of_cids = PDP_CONTEXT_CID_MAX;
  
  for(i = 0; i<no_of_cids; i++)
  {
    /* If the first cid is omitted it is treated as all cids */ 
    if (cid_array[0] EQ PDP_CONTEXT_CID_OMITTED)
      current_cid = i+1;
    else
      current_cid = cid_array[i];

    if (sAT_PlusCGEQNEG ((T_ACI_CMD_SRC)srcId, current_cid, &qos_valid, &pdef_qos) EQ AT_CMPL)
    {
      if (qos_valid)
      {
        /* Format the return string with the Release 99 QoS */
        sprintf(g_sa,"%s%hd,%d,%d,%d,%d,%d,%d,%d,%dE%d,%dE%d,%d,%d,%d", me, current_cid,
          pdef_qos.qos_r99.tc,                       /*< traffic class                                      */
          pdef_qos.qos_r99.max_rate_ul,              /*< max bit-rate uplink                                */
          pdef_qos.qos_r99.max_rate_dl,              /*< max bit-rate downlink                              */
          pdef_qos.qos_r99.guar_br_ul,               /*< guaranteed bit-rate uplink                         */
          pdef_qos.qos_r99.guar_br_dl,               /*< guaranteed bit-rate downlink                       */
          pdef_qos.qos_r99.order,                    /*< delivery order                                     */
          pdef_qos.qos_r99.max_sdu,                  /*< max SDU size                                       */
          pdef_qos.qos_r99.sdu_err_ratio.ratio_mant, /*< SDU error ratio (format is the same as ber)        */
          pdef_qos.qos_r99.sdu_err_ratio.ratio_exp,  /*<                                                    */
          pdef_qos.qos_r99.ber.ratio_mant,           /*< Residual bit error ratio (ratio_mant E ratio_exp   */
          pdef_qos.qos_r99.ber.ratio_exp,            /*< (Ex. 10E2)                                         */
          pdef_qos.qos_r99.del_err_sdu,              /*< delivery of erroneous SDU                          */
          pdef_qos.qos_r99.xfer_delay,               /*< transfer delay                                     */
          pdef_qos.qos_r99.handling_pri);            /*< traffic handling priority                          */
      
        io_sendMessage(srcId, g_sa, ATI_NORMAL_OUTPUT);
      }
    }
    else
    {
      cmdCmeError(CME_ERR_Unknown); /* The command is invalid for srcId */
      return (ATI_FAIL);
    }
  }

  return (ATI_CMPL);
#endif /* FF_ATI_BAT */
}

/*
+--------------------------------------------------------------------+
| PROJECT : UMTS                        MODULE  : ATI_GPRS           |
| STATE   : finished                    ROUTINE : setatPlusCGCMOD    |
+--------------------------------------------------------------------+

  PURPOSE : Modify indicated or all contexts.
*/
GLOBAL T_ATI_RSLT setatPlusCGCMOD (char *cl, UBYTE srcId)
{
  U8           cid_array[PDP_CONTEXT_CID_MAX + 1];
  int          no_of_cids;
  char         *end;
  T_ACI_RETURN ret;
#ifdef FF_ATI_BAT
  T_ATI_SRC_PARAMS *src_params = find_element (ati_src_list, srcId, search_ati_src_id);
#endif
   
  TRACE_FUNCTION("setatPlusCGCMOD ()");

  --cl; /* No comma the first time!!! */

  /* Parse all the cids into the cid array */
  no_of_cids = 0;
  do
  {
    /* Remove the "," */
    cl++;

    /* Set the entry to omitted. */
    cid_array[no_of_cids] = PDP_CONTEXT_CID_OMITTED;

    /* Parse the value */
    cid_array[no_of_cids] = (U8) strtol (cl, &end, 10);
    cl = end;

    if( cid_array[no_of_cids] EQ 0 )
      cid_array[no_of_cids] = PDP_CONTEXT_CID_OMITTED;

    /* If a cid is omitted no cids must be specified at all. */
    if( cid_array[no_of_cids] EQ PDP_CONTEXT_CID_OMITTED )
    {
      if (no_of_cids NEQ 0)
      {
        /* Don't accept omitted cids inside if not the first. (AT+CGCMOD=1,2,,3 is not allowed). */
        cmdCmeError(CME_ERR_OpNotSupp);
        return (ATI_FAIL);
      }
      /* Break the while */
      break;
    }
    if ( (!cl) OR (cid_array[no_of_cids] < PDP_CONTEXT_CID_MIN) OR (cid_array[no_of_cids] > PDP_CONTEXT_CID_MAX) OR (no_of_cids >= PDP_CONTEXT_CID_MAX) )
    {
      cmdCmeError(CME_ERR_OpNotSupp);
      return (ATI_FAIL);
    }
    no_of_cids++;
  } while ( *cl EQ ',' );
  cid_array[no_of_cids] = PDP_CONTEXT_CID_INVALID;

#ifdef FF_ATI_BAT
{
  T_BAT_cmd_send cmd;
  T_BAT_cmd_set_plus_cgcmod cgcmod;
  UBYTE n;
  cmd.ctrl_params = BAT_CMD_SET_PLUS_CGCMOD;
  cmd.params.ptr_set_plus_cgcmod=&cgcmod;

  for (n=0;((n<no_of_cids) AND (n<BAT_MAX_PDP_CID));n++)
    cgcmod.cid[n]=(T_BAT_pdp_cid)cid_array[n];
    bat_send(ati_bat_get_client(srcId), &cmd);

    src_params->curAtCmd=AT_CMD_CGCMOD;
    return(ATI_EXCT);
}
#else
  ret = sAT_PlusCGCMOD ((T_ACI_CMD_SRC)srcId, cid_array);
  switch ( ret )
  {
    case AT_EXCT:
      cmdErrStr   = NULL;
//      curAtCmd    = AT_CMD_CGCMOD;
      return (ATI_EXCT);
    case AT_CMPL:
      /* The command can not return AT_CMPL!! This is an error! */
      cmdCmeError(CME_ERR_Unknown);
      return (ATI_FAIL);
  }
  return (map_aci_2_ati_rslt(ret));
#endif /*FF_ATI_BAT */
}

/*
+--------------------------------------------------------------------+
| PROJECT : UMTS                        MODULE  : ATI_GPRS           |
| STATE   : finished                    ROUTINE : queatPlusCGEQREQ   |
+--------------------------------------------------------------------+

  PURPOSE : Query of 3G QoS Profile (Requested)
*/
GLOBAL T_ATI_RSLT queatPlusCGEQREQ (char *cl, UBYTE srcId)
{
  char        *me="+CGEQREQ: ";
  T_PS_qos     pdef_qos;
  U8           cid;
  BOOL         qos_valid = FALSE;

  TRACE_FUNCTION("queatPlusCGEQREQ ()");
#ifdef FF_ATI_BAT
  {
    T_BAT_cmd_send cmd;
    T_BAT_no_parameter dummy;

    cmd.ctrl_params = BAT_CMD_SET_PLUS_CGEQREQ;
    dummy.bat_dummy = 0xFF;
    cmd.params.ptr_set_plus_cgeqreq = &dummy;
    bat_send(ati_bat_get_client(srcId), &cmd);
    return(ATI_EXCT);
  }
#else

  for ( cid = PDP_CONTEXT_CID_MIN; cid <= PDP_CONTEXT_CID_MAX; cid++ )
  {
    if (qAT_PlusCGEQREQ ((T_ACI_CMD_SRC)srcId, cid, &qos_valid, &pdef_qos) EQ AT_CMPL)
    {
      if (qos_valid)
      {
        /* Format the return string with the Release 99 QoS */
        sprintf(g_sa,"%s%hd,%d,%d,%d,%d,%d,%d,%d,\"%dE%d\",\"%dE%d\",%d,%d,%d", me, cid,
          pdef_qos.qos_r99.tc,                       /*< traffic class                                      */
          pdef_qos.qos_r99.max_rate_ul,              /*< max bit-rate uplink                                */
          pdef_qos.qos_r99.max_rate_dl,              /*< max bit-rate downlink                              */
          pdef_qos.qos_r99.guar_br_ul,               /*< guaranteed bit-rate uplink                         */
          pdef_qos.qos_r99.guar_br_dl,               /*< guaranteed bit-rate downlink                       */
          pdef_qos.qos_r99.order,                    /*< delivery order                                     */
          pdef_qos.qos_r99.max_sdu,                  /*< max SDU size                                       */
          pdef_qos.qos_r99.sdu_err_ratio.ratio_mant, /*< SDU error ratio (format is the same as ber)        */
          pdef_qos.qos_r99.sdu_err_ratio.ratio_exp,  /*<                                                    */
          pdef_qos.qos_r99.ber.ratio_mant,           /*< Residual bit error ratio (ratio_mant E ratio_exp   */
          pdef_qos.qos_r99.ber.ratio_exp,            /*< (Ex. 10E2)                                         */
          pdef_qos.qos_r99.del_err_sdu,              /*< delivery of erroneous SDU                          */
          pdef_qos.qos_r99.xfer_delay,               /*< transfer delay                                     */
          pdef_qos.qos_r99.handling_pri);            /*< traffic handling priority                          */
      
        io_sendMessage(srcId, g_sa, ATI_NORMAL_OUTPUT);
      }
    }
    else
    {
      /* AT_FAIL has been returned */
      cmdCmeError(CME_ERR_Unknown); /* The command is invalid for srcId */
      return (ATI_FAIL);
    }
  }
  return (ATI_CMPL);
#endif /* FF_ATI_BAT */
}

/*
+--------------------------------------------------------------------+
| PROJECT : UMTS                        MODULE  : ATI_GPRS           |
| STATE   : finished                    ROUTINE : queatPlusCGEQMIN   |
+--------------------------------------------------------------------+

  PURPOSE : Query of 3G QoS Profile (Minimum)
*/
GLOBAL T_ATI_RSLT queatPlusCGEQMIN (char *cl, UBYTE srcId)
{
  char         *me="+CGEQMIN: ";
  T_PS_min_qos pdef_qos;
  U8           cid;
  BOOL         qos_valid;

  TRACE_FUNCTION("queatPlusCGEQMIN ()");
#ifdef FF_ATI_BAT
  {
    T_BAT_cmd_send cmd;
    T_BAT_no_parameter dummy;

    cmd.ctrl_params = BAT_CMD_SET_PLUS_CGEQMIN;
    dummy.bat_dummy = 0xFF;
    cmd.params.ptr_set_plus_cgeqmin = &dummy;
    bat_send(ati_bat_get_client(srcId), &cmd);
    return(ATI_EXCT);
  }
#else

  for ( cid = PDP_CONTEXT_CID_MIN; cid <= PDP_CONTEXT_CID_MAX; cid++ )
  {
    if (qAT_PlusCGEQMIN ((T_ACI_CMD_SRC)srcId, cid, &qos_valid, &pdef_qos) EQ AT_CMPL)
    {
      if (qos_valid)
      {
        /* Format the return string with the Release 99 QoS */
        sprintf(g_sa,"%s%hd", me, cid);

        /* Format the return string with the Release 99 QoS */
        sprintf(g_sa,"%s%hd,%d,%d,%d,%d,%d,%d,%d,\"%dE%d\",\"%dE%d\",%d,%d,%d", me, cid,
          pdef_qos.qos_r99.tc,                       /*< traffic class                                      */
          pdef_qos.qos_r99.max_rate_ul,              /*< max bit-rate uplink                                */
          pdef_qos.qos_r99.max_rate_dl,              /*< max bit-rate downlink                              */
          pdef_qos.qos_r99.guar_br_ul,               /*< guaranteed bit-rate uplink                         */
          pdef_qos.qos_r99.guar_br_dl,               /*< guaranteed bit-rate downlink                       */
          pdef_qos.qos_r99.order,                    /*< delivery order                                     */
          pdef_qos.qos_r99.max_sdu,                  /*< max SDU size                                       */
          pdef_qos.qos_r99.sdu_err_ratio.ratio_mant, /*< SDU error ratio (format is the same as ber)        */
          pdef_qos.qos_r99.sdu_err_ratio.ratio_exp,  /*<                                                    */
          pdef_qos.qos_r99.ber.ratio_mant,           /*< Residual bit error ratio (ratio_mant E ratio_exp   */
          pdef_qos.qos_r99.ber.ratio_exp,            /*< (Ex. 10E2)                                         */
          pdef_qos.qos_r99.del_err_sdu,              /*< delivery of erroneous SDU                          */
          pdef_qos.qos_r99.xfer_delay,               /*< transfer delay                                     */
          pdef_qos.qos_r99.handling_pri);            /*< traffic handling priority                          */

        io_sendMessage(srcId, g_sa, ATI_NORMAL_OUTPUT);
      }
    }
    else
    {
      /* AT_FAIL has been returned */
      cmdCmeError(CME_ERR_Unknown); /* The command is invalid for srcId */
      return (ATI_FAIL);
    }
  }
  return (ATI_CMPL);
#endif /* FF_ATI_BAT */
}

/*
+--------------------------------------------------------------------+
| PROJECT : UMTS                        MODULE  : ATI_GPRS           |
| STATE   : finished                    ROUTINE : tesatPlusCGEQNEG   |
+--------------------------------------------------------------------+

  PURPOSE : Test function for 3G QoS Profile (Negotiated)
            Return cids for all active contexts.
*/
GLOBAL T_ATI_RSLT tesatPlusCGEQNEG (char *cl, UBYTE srcId)
{
  char        *me="+CGEQNEG: ";
  BOOL         context_activated, first_time = TRUE;
  U8           cid;

  TRACE_FUNCTION("tesatPlusCGEQNEG ()");

  for (cid = 1; cid<=PDP_CONTEXT_CID_MAX; cid++)
  {
    if (tAT_PlusCGEQNEG_CGCMOD ((T_ACI_CMD_SRC)srcId, cid, &context_activated) EQ AT_CMPL)
    {
      if (context_activated)
      {
        if (first_time)
        {
          /* First time: Place command name and no comma in front of cid */
          sprintf(g_sa,"%s%d", me, cid);
          first_time = FALSE;
        }
        else
        {
          /* Subsequent time: Append sa with ",<cid>" */
          sprintf(g_sa,"%s,%d", g_sa, cid);
        }
      }
    }
    else
    {
      /* AT_FAIL has been returned */
      cmdCmeError(CME_ERR_Unknown);
      return (ATI_FAIL);
    }
  }
  if (!first_time)
  {
    /* Only send something if any contexts were activated (first_time NEQ TRUE) */
    io_sendMessage(srcId, g_sa, ATI_NORMAL_OUTPUT);
  }

  return (ATI_CMPL);
}

/*
+--------------------------------------------------------------------+
| PROJECT : UMTS                        MODULE  : ATI_GPRS           |
| STATE   : finished                    ROUTINE : tesatPlusCGCMOD    |
+--------------------------------------------------------------------+

  PURPOSE : Test function for 3G Context Modifify command.
            Return cids for all active contexts.
*/
GLOBAL T_ATI_RSLT tesatPlusCGCMOD (char *cl, UBYTE srcId)
{
  char        *me="+CGCMOD: ";
  BOOL         context_activated, first_time = TRUE;
  U8           cid;

  TRACE_FUNCTION("tesatPlusCGCMOD ()");

  for (cid = PDP_CONTEXT_CID_MIN; cid <= PDP_CONTEXT_CID_MAX; cid++)
  {
    if (tAT_PlusCGEQNEG_CGCMOD ((T_ACI_CMD_SRC)srcId, cid, &context_activated) EQ AT_CMPL)
    {
      if (context_activated)
      {
        if (first_time)
        {
          /* First time: Place command name and no comma in front of cid */
          sprintf(g_sa,"%s%d", me, cid);
          first_time = FALSE;
        }
        else
        {
          /* Subsequent time: Append sa with ",<cid>" */
          sprintf(g_sa,"%s,%d", g_sa, cid);
        }
      }
    }
    else
    {
      /* AT_FAIL has been returned */
      cmdCmeError(CME_ERR_Unknown);
      return (ATI_FAIL);
    }
  }
  if (!first_time)
  {
    /* Only send something if any contexts were activated (first_time NEQ TRUE) */
    io_sendMessage(srcId, g_sa, ATI_NORMAL_OUTPUT);
  }

  return (ATI_CMPL);
}

#endif /* REL99 */

#endif /* GPRS */

