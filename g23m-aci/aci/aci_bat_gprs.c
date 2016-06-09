/* 
+----------------------------------------------------------------------------- 
|  Project :  GSM-F&D (8411)
|  Modul   :  ACI_BAT
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
|  Purpose :  This BAT wrapper modul is ...
| 
+----------------------------------------------------------------------------- 
*/ 

#ifdef GPRS
 
#include "aci_all.h"     /* includes prim.h, which includes p_bat.h */ 
#include "aci_cmh.h"     /* prototypes of sAT_,qAT_,tAT_    */ 
#include "aci_bat_cmh.h" /* prototypes of sBAT_,qBAT_,tBAT_ */ 
#include "ati_cmd.h"
#include "aci_cmd.h"
#include "gaci_cmh.h"
#include "aci_bat.h"
#include "aci_mem.h"
#include "gaci.h"

/*==== VARIABLES ==================================================*/
EXTERN T_PDP_CONTEXT_INTERNAL *p_pdp_context_list;

extern BOOL cmhSM_isContextActive( void );
extern T_ATI_RSLT setatPercentCGPCO_HEX (UBYTE srcId,USHORT cid,
                                    UBYTE *pco_hex_string, USHORT len);
extern T_ATI_RSLT setatPercentCGPCO_ASCII (UBYTE srcId, USHORT cid,
                                      UBYTE *pco_ascii_string, USHORT len);
extern void ipaddr_to_string(ULONG ipaddr, char *ipaddr_string);


#define MAGIC_EMPTY_APN "\x0ff"
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : ACI_BAT              |
| STATE   : code                      ROUTINE : sBAT_PlusCGDCONT     |
+--------------------------------------------------------------------+

  PURPOSE : 
*/
GLOBAL T_ACI_BAT_RSLT sBAT_PlusCGDCONT     (T_ACI_DTI_PRC_PSI *src_infos_psi,
                                            T_BAT_cmd_send    *cmd)
{
  T_ACI_BAT_RSLT ret = ACI_BAT_FAIL;
  T_BAT_pdp_cid  cid = BAT_PDP_CID_NOT_PRESENT ;
  T_PDP_CONTEXT  inputCtxt; /*lint !e813*/
  
  TRACE_FUNCTION ("sBAT_PlusCGDCONT()");

  memset(&inputCtxt,0,sizeof(T_PDP_CONTEXT));
  
  cid = cmd->params.ptr_set_plus_cgdcont->cid;
  inputCtxt.pdp_addr.ctrl_ip_address = NAS_is_ip_not_present;
  inputCtxt.pdp_addr.ip_address.ip_not_present = BAT_PARAMETER_NOT_PRESENT;
  
  if (cid EQ BAT_PDP_CID_NOT_PRESENT )
  {
    cid = (T_BAT_pdp_cid)PDP_CONTEXT_CID_OMITTED ;
  }

  switch (cmd->params.ptr_set_plus_cgdcont->pdp_type)
  {
    case BAT_PDP_TYPE_IPV4:
      strcpy(inputCtxt.pdp_type, "IP");
      if(cmd->params.ptr_set_plus_cgdcont->v_pdp_addr)
      {
        inputCtxt.pdp_addr.ctrl_ip_address = NAS_is_ipv4;
        memcpy(inputCtxt.pdp_addr.ip_address.ipv4_addr.a4,cmd->params.ptr_set_plus_cgdcont->pdp_addr,cmd->params.ptr_set_plus_cgdcont->c_pdp_addr);
      }
      break;
    case BAT_PDP_TYPE_IPV6:
      strcpy(inputCtxt.pdp_type, "IPV6");
      if(cmd->params.ptr_set_plus_cgdcont->v_pdp_addr)
      {
        inputCtxt.pdp_addr.ctrl_ip_address = NAS_is_ipv6;
        memcpy(inputCtxt.pdp_addr.ip_address.ipv6_addr.a6,cmd->params.ptr_set_plus_cgdcont->pdp_addr,cmd->params.ptr_set_plus_cgdcont->c_pdp_addr);
      }
      break;
    default:
      {
        break;
      }
      //return ret;
  }
  if(cmd->params.ptr_set_plus_cgdcont->v_apn)
  {
    memcpy(inputCtxt.pdp_apn,cmd->params.ptr_set_plus_cgdcont->apn,cmd->params.ptr_set_plus_cgdcont->c_apn);
  }
  inputCtxt.d_comp = cmd->params.ptr_set_plus_cgdcont->d_comp;
  inputCtxt.h_comp = cmd->params.ptr_set_plus_cgdcont->h_comp;
  
  ret = (T_ACI_BAT_RSLT) sAT_PlusCGDCONT ((T_ACI_CMD_SRC)src_infos_psi->srcId, (U8)cid ,&inputCtxt);
  return(ret);
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : ACI_BAT              |
| STATE   : code                      ROUTINE : qBAT_PlusCGDCONT     |
+--------------------------------------------------------------------+

  PURPOSE : 
*/
GLOBAL T_ACI_BAT_RSLT qBAT_PlusCGDCONT     (T_ACI_DTI_PRC_PSI *src_infos_psi,
                                            T_BAT_cmd_send    *cmd)
{
  T_ACI_BAT_RSLT ret = ACI_BAT_FAIL;
  T_BAT_cmd_response resp;
  T_BAT_res_que_plus_cgdcont que_cgdcont_buffer; /*lint !e813*/
  T_PDP_CONTEXT defCtxts[PDP_CONTEXT_CID_MAX]; /*lint !e813*/
  SHORT cid_array[PDP_CONTEXT_CID_MAX];
  U8 i ;
  
  TRACE_FUNCTION ("qBAT_PlusCGDCONT()");
  resp.ctrl_response = BAT_RES_QUE_PLUS_CGDCONT;
  resp.response.ptr_que_plus_cgdcont = &que_cgdcont_buffer;
  
  ret = (T_ACI_BAT_RSLT)qAT_PlusCGDCONT ((T_ACI_CMD_SRC)src_infos_psi->srcId, defCtxts, cid_array);

  if (ret EQ ACI_BAT_CMPL)
  {
    for ( i = 0; i < PDP_CONTEXT_CID_MAX AND cid_array[i] NEQ PDP_CONTEXT_CID_OMITTED; i++ )
    {
      que_cgdcont_buffer.cid = (T_BAT_pdp_cid)cid_array[i];
      if(!(utl_strcasecmp((const char *)defCtxts[i].pdp_type, (const char *)"ip")))
      {
          que_cgdcont_buffer.pdp_type = BAT_PDP_TYPE_IPV4;
          memcpy(que_cgdcont_buffer.pdp_addr,defCtxts[i].pdp_addr.ip_address.ipv4_addr.a4,NAS_SIZE_IPv4_ADDR);
          que_cgdcont_buffer.c_pdp_addr = NAS_SIZE_IPv4_ADDR;
      }

      else if(!(utl_strcasecmp((const char *)defCtxts[i].pdp_type, (const char *)"ipv6")))
      {
          que_cgdcont_buffer.pdp_type = BAT_PDP_TYPE_IPV6;
          memcpy(que_cgdcont_buffer.pdp_addr,defCtxts[i].pdp_addr.ip_address.ipv6_addr.a6,NAS_SIZE_IPv6_ADDR);
          que_cgdcont_buffer.c_pdp_addr = NAS_SIZE_IPv6_ADDR;
      }
      else
      {
        que_cgdcont_buffer.pdp_type = BAT_PDP_TYPE_NOT_PRESENT;
      }

      que_cgdcont_buffer.c_apn = strlen(defCtxts[i].pdp_apn);
      memcpy(que_cgdcont_buffer.apn,defCtxts[i].pdp_apn,strlen(defCtxts[i].pdp_apn));
      que_cgdcont_buffer.d_comp = (T_BAT_plus_cgdcont_d_comp)defCtxts[i].d_comp;
      que_cgdcont_buffer.h_comp = (T_BAT_plus_cgdcont_h_comp)defCtxts[i].h_comp;
      aci_bat_send(src_infos_psi,&resp);
    }
    return ACI_BAT_CMPL;
  }
  return(ret);
}


#ifdef REL99
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : ACI_BAT              |
| STATE   : code                      ROUTINE : sBAT_PlusCGCMOD      |
+--------------------------------------------------------------------+

  PURPOSE :
*/
GLOBAL T_ACI_BAT_RSLT sBAT_PlusCGCMOD    (T_ACI_DTI_PRC_PSI *src_infos_psi,
                                            T_BAT_cmd_send    *cmd)
{
   T_ACI_BAT_RSLT ret = ACI_BAT_FAIL;
   SHORT count = 0;
   U8 cid_array[PDP_CONTEXT_CID_MAX];

   TRACE_FUNCTION ("sBAT_PlusCGCMOD()");
    
   if(cmd->params.ptr_set_plus_cgact->c_cid EQ 0)
    cid_array[0] = PDP_CONTEXT_CID_INVALID;
   for(count=0;count<cmd->params.ptr_set_plus_cgact->c_cid;count++)
   {
     cid_array[count] = (cmd->params.ptr_set_plus_cgact->cid[count] EQ BAT_PDP_CID_NOT_PRESENT)
                   ? PDP_CONTEXT_CID_INVALID : cmd->params.ptr_set_plus_cgact->cid[count];
   }
   if(count > 0)
     cid_array[count]= PDP_CONTEXT_CID_INVALID;
   ret = (T_ACI_BAT_RSLT) sAT_PlusCGCMOD ((T_ACI_CMD_SRC) src_infos_psi->srcId, cid_array );
   return(ret);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : ACI_BAT              |
| STATE   : code                      ROUTINE : sBAT_PlusCGDSCONT    |
+--------------------------------------------------------------------+

  PURPOSE :
*/
GLOBAL T_ACI_BAT_RSLT sBAT_PlusCGDSCONT     (T_ACI_DTI_PRC_PSI *src_infos_psi,
                                            T_BAT_cmd_send    *cmd)
{
  T_ACI_BAT_RSLT ret = ACI_BAT_FAIL;
  T_BAT_pdp_cid  cid = BAT_PDP_CID_NOT_PRESENT;
  T_PDP_CONTEXT  inputCtxt; /*lint !e813*/

  TRACE_FUNCTION ("sBAT_PlusCGDSCONT()");

  memset(&inputCtxt,0,sizeof(T_PDP_CONTEXT));

  cid = cmd->params.ptr_set_plus_cgdscont->cid;
  inputCtxt.p_cid = cmd->params.ptr_set_plus_cgdscont->p_cid;
  
  if (cid EQ BAT_PDP_CID_NOT_PRESENT )
  {
    cid = (T_BAT_pdp_cid)PDP_CONTEXT_CID_OMITTED;
  }

  if (cmd->params.ptr_set_plus_cgdscont->p_cid EQ (U8)BAT_PDP_CID_NOT_PRESENT )
//  if (inputCtxt.p_cid EQ (U8)BAT_PDP_CID_NOT_PRESENT )
  {
    inputCtxt.p_cid = PDP_CONTEXT_CID_OMITTED;
  }

  inputCtxt.d_comp = cmd->params.ptr_set_plus_cgdscont->d_comp;
  inputCtxt.h_comp = cmd->params.ptr_set_plus_cgdscont->h_comp;

  ret =(T_ACI_BAT_RSLT) sAT_PlusCGDSCONT ((T_ACI_CMD_SRC)src_infos_psi->srcId, (U8)cid ,&inputCtxt);
  if(ret EQ ACI_BAT_CMPL)
  {
    return ACI_BAT_CMPL;
  }
  return(ret);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : ACI_BAT              |
| STATE   : code                      ROUTINE : qBAT_PlusCGDSCONT    |
+--------------------------------------------------------------------+

  PURPOSE :
*/
GLOBAL T_ACI_BAT_RSLT qBAT_PlusCGDSCONT    (T_ACI_DTI_PRC_PSI *src_infos_psi,
                                            T_BAT_cmd_send    *cmd)
{
  T_ACI_BAT_RSLT ret = ACI_BAT_FAIL;
  T_BAT_cmd_response resp;
  T_PDP_CONTEXT pdp_context_array[PDP_CONTEXT_CID_MAX];
  U8            pdp_context_cid_array[PDP_CONTEXT_CID_MAX];
  int           i;

  TRACE_FUNCTION("qBAT_PlusCGDSCONT()");
  resp.ctrl_response = BAT_RES_QUE_PLUS_CGDSCONT;

  memset( pdp_context_cid_array, 0, sizeof(pdp_context_cid_array) );
  memset( pdp_context_array, 0, sizeof(pdp_context_array) );

  ret = (T_ACI_BAT_RSLT) qAT_PlusCGDSCONT((T_ACI_CMD_SRC)src_infos_psi->srcId, pdp_context_array, pdp_context_cid_array);

  if (ret EQ ACI_BAT_CMPL)
  {
    for ( i = 0; i < PDP_CONTEXT_CID_MAX AND pdp_context_cid_array[i] NEQ PDP_CONTEXT_CID_OMITTED; i++ )
    {
      resp.response.ptr_que_plus_cgdscont->cid = (T_BAT_pdp_cid)pdp_context_cid_array[i];
      resp.response.ptr_que_plus_cgdscont->p_cid = (U8)pdp_context_array[i].p_cid;
      resp.response.ptr_que_plus_cgdscont->d_comp = (T_BAT_plus_cgdcont_d_comp)pdp_context_array[i].d_comp;
      resp.response.ptr_que_plus_cgdscont->h_comp = (T_BAT_plus_cgdcont_h_comp)pdp_context_array[i].h_comp;

      aci_bat_send(src_infos_psi,&resp);
    }
    return ACI_BAT_CMPL;
  }
  return ret;
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : ACI_BAT              |
| STATE   : code                      ROUTINE : sBAT_PlusCGTFT       |
+--------------------------------------------------------------------+

  PURPOSE :
*/
GLOBAL T_ACI_BAT_RSLT sBAT_PlusCGTFT     (T_ACI_DTI_PRC_PSI *src_infos_psi,
                                          T_BAT_cmd_send    *cmd)
{
  T_ACI_BAT_RSLT            ret = ACI_BAT_FAIL;
  T_NAS_tft_pf              tft_pf_input;
  T_BAT_cmd_set_plus_cgtft *cgtft_prm;
  
  TRACE_FUNCTION ("sBAT_PlusCGTFT()");
  
  memset( &tft_pf_input, 0x00, sizeof(T_NAS_tft_pf) );
  cgtft_prm = cmd->params.ptr_set_plus_cgtft;
  
  /* cid */
  if (cgtft_prm->cid EQ BAT_PDP_CID_NOT_PRESENT)
  {
    cmdCmeError(CME_ERR_OpNotAllow);
    return (ACI_BAT_FAIL);
  }
  
  /* pf id, precedence */
  if ((cgtft_prm->tft_pf_id >= BAT_MIN_TFT_PF_ID) AND
      (cgtft_prm->tft_pf_id <= BAT_MAX_TFT_PF_ID))
  {
    tft_pf_input.tft_pf_id = (U8)cgtft_prm->tft_pf_id;
    /* precedence has to be present in case of valid pf id */
    if ( cgtft_prm->tft_pf_precedence >= BAT_MAX_TFT_PRECEDENCE )
    {
      cmdCmeError(CME_ERR_OpNotAllow);
      return (ACI_BAT_FAIL);
    }
    tft_pf_input.tft_pf_precedence = (U8)cgtft_prm->tft_pf_precedence;
  }
  else
  {
    tft_pf_input.tft_pf_id         = (U8)-1;
    tft_pf_input.tft_pf_precedence = (U8)-1;// valid val (more suspicious than 0)
  }
  
  /* addr_n_mask */
  if( cgtft_prm->v_addr_n_mask AND cgtft_prm->c_addr_n_mask )
  {
    /* IP4 addr_n_mask */
    if( cgtft_prm->c_addr_n_mask EQ 8)
    {
      tft_pf_input.ctrl_tft_pf_entry = NAS_is_tft_pf_ipv4;            
      memcpy(tft_pf_input.tft_pf_entry.tft_pf_ipv4.tft_ipv4_src_addr_mask.tft_ipv4_addr,
             cgtft_prm->addr_n_mask,
             NAS_SIZE_IPv4_ADDR);
      memcpy(tft_pf_input.tft_pf_entry.tft_pf_ipv4.tft_ipv4_src_addr_mask.tft_ipv4_mask,
             cgtft_prm->addr_n_mask+NAS_SIZE_IPv4_ADDR,
             NAS_SIZE_IPv4_ADDR); 
      
      tft_pf_input.tft_pf_valid_bits |= NAS_TFT_ID_IPv4_SRC_ADDR_MASK;
    }
    
    /* IP6 addr_n_mask */
    else if(  cgtft_prm->c_addr_n_mask EQ 32)
    {
      tft_pf_input.ctrl_tft_pf_entry = NAS_is_tft_pf_ipv6;
      memcpy(tft_pf_input.tft_pf_entry.tft_pf_ipv6.tft_ipv6_src_addr_mask.tft_ipv6_addr, 
             cgtft_prm->addr_n_mask,
             NAS_SIZE_IPv6_ADDR);
      memcpy(tft_pf_input.tft_pf_entry.tft_pf_ipv6.tft_ipv6_src_addr_mask.tft_ipv6_mask, 
             cgtft_prm->addr_n_mask+NAS_SIZE_IPv6_ADDR, 
             NAS_SIZE_IPv6_ADDR); 
      
      tft_pf_input.tft_pf_valid_bits |= NAS_TFT_ID_IPv6_SRC_ADDR_MASK;
    }
    else
    {
      cmdCmeError(CME_ERR_OpNotAllow);
      return (ACI_BAT_FAIL);
    }
  }
  else
  {
    /* The following does not make sense according 27.007:
       if addr_n_mask isn't present, flow label isn't present either, 
       but +CGTFT behaviour should not change */
    if ( cgtft_prm->flow_label > BAT_MAX_TFT_FLOW_LABEL )
    {
      /* IP v4 type */
      tft_pf_input.ctrl_tft_pf_entry = NAS_is_tft_pf_ipv4;
    }
    else
    {
      /* IP v6 type */
      tft_pf_input.ctrl_tft_pf_entry = NAS_is_tft_pf_ipv6;
    }
  }
  
  switch( tft_pf_input.ctrl_tft_pf_entry )
  {
    /*** IP v4 prmeters */
    case NAS_is_tft_pf_ipv4:
    {
       /* Protocol number */
       tft_pf_input.tft_pf_entry.tft_pf_ipv4.tft_protocol = 
          (U8)cgtft_prm->prt_num_or_nxt_hd;
        tft_pf_input.tft_pf_valid_bits |= NAS_TFT_ID_PROTOCOL_OR_NEXT_HDR;
      
      
      /* Destination port */
      if( cgtft_prm->v_dest_port_range AND cgtft_prm->c_dest_port_range)
      {
        tft_pf_input.tft_pf_entry.tft_pf_ipv4.tft_dest_port_range.low_limit = 
          cgtft_prm->dest_port_range[0];
        tft_pf_input.tft_pf_entry.tft_pf_ipv4.tft_dest_port_range.high_limit = 
          cgtft_prm->dest_port_range[1];
        tft_pf_input.tft_pf_valid_bits |= NAS_TFT_ID_DEST_PORT_RANGE;
      }
        
      /* Source port */
      if( cgtft_prm->v_src_port_range AND cgtft_prm->c_src_port_range)
      {
        tft_pf_input.tft_pf_entry.tft_pf_ipv4.tft_src_port_range.low_limit = 
          cgtft_prm->src_port_range[0];
        tft_pf_input.tft_pf_entry.tft_pf_ipv4.tft_src_port_range.high_limit = 
          cgtft_prm->src_port_range[1];
        tft_pf_input.tft_pf_valid_bits |= NAS_TFT_ID_SRC_PORT_RANGE;
      }
        
      /* IPSEC security parameter spi */
      /* valid val=[0..ffffffff] -> no way to find out if parameter was
         omitted, but can't change BAT interface for PlusCGTFT -> open issue */
      tft_pf_input.tft_pf_entry.tft_pf_ipv4.tft_ipsec_spi = cgtft_prm->ipsec_spi;
      tft_pf_input.tft_pf_valid_bits |= NAS_TFT_ID_IPSEC_SPI;
      
      /* Type of service and mask */
      if( cgtft_prm->v_tos_mask_or_tc_mask AND cgtft_prm->c_tos_mask_or_tc_mask )
      {
        tft_pf_input.tft_pf_entry.tft_pf_ipv4.tft_tos_and_mask.tos_value = 
          cgtft_prm->tos_mask_or_tc_mask[0];
        tft_pf_input.tft_pf_entry.tft_pf_ipv4.tft_tos_and_mask.tos_mask =
          cgtft_prm->tos_mask_or_tc_mask[1];
        tft_pf_input.tft_pf_valid_bits |= NAS_TFT_ID_TOS_AND_MASK;        
      }

      break;
      
    } /* END: case NAS_is_tft_pf_ipv4 */
    
    /*** IP v6 parameters ***/
    case NAS_is_tft_pf_ipv6:
    {
      /* Next header */
       tft_pf_input.tft_pf_entry.tft_pf_ipv6.tft_next_hdr = 
          (U8)cgtft_prm->prt_num_or_nxt_hd;
        tft_pf_input.tft_pf_valid_bits |= NAS_TFT_ID_PROTOCOL_OR_NEXT_HDR;
           
      /* Destination port */
      if( cgtft_prm->v_dest_port_range AND cgtft_prm->c_dest_port_range)
      {
        tft_pf_input.tft_pf_entry.tft_pf_ipv6.tft_dest_port_range.low_limit = 
          cgtft_prm->dest_port_range[0];
        tft_pf_input.tft_pf_entry.tft_pf_ipv6.tft_dest_port_range.high_limit = 
          cgtft_prm->dest_port_range[1];
        tft_pf_input.tft_pf_valid_bits |= NAS_TFT_ID_DEST_PORT_RANGE;
      }
        
      /* Source port */
      if( cgtft_prm->v_src_port_range AND cgtft_prm->c_src_port_range)
      {
        tft_pf_input.tft_pf_entry.tft_pf_ipv6.tft_src_port_range.low_limit = 
          cgtft_prm->src_port_range[0];
        tft_pf_input.tft_pf_entry.tft_pf_ipv6.tft_src_port_range.high_limit = 
          cgtft_prm->src_port_range[1];
        tft_pf_input.tft_pf_valid_bits |= NAS_TFT_ID_SRC_PORT_RANGE;
      }
        
      /* IPSEC security parameter spi */
      /* valid val=[0..ffffffff] -> no way to find out if parameter was
         omitted, but can't change BAT interface for PlusCGTFT -> open issue */
      tft_pf_input.tft_pf_entry.tft_pf_ipv6.tft_ipsec_spi = cgtft_prm->ipsec_spi;
      tft_pf_input.tft_pf_valid_bits |= NAS_TFT_ID_IPSEC_SPI;
      
      /* Type of service and mask */
      if( cgtft_prm->v_tos_mask_or_tc_mask AND cgtft_prm->c_tos_mask_or_tc_mask )
      {
        tft_pf_input.tft_pf_entry.tft_pf_ipv6.tft_tos_and_mask.tos_value = 
          cgtft_prm->tos_mask_or_tc_mask[0];
        tft_pf_input.tft_pf_entry.tft_pf_ipv6.tft_tos_and_mask.tos_mask =
          cgtft_prm->tos_mask_or_tc_mask[1];
        tft_pf_input.tft_pf_valid_bits |= NAS_TFT_ID_TOS_AND_MASK;        
      }
      
      /* Flow label */
      if (cgtft_prm->flow_label <= BAT_MAX_TFT_FLOW_LABEL)
      {
        tft_pf_input.tft_pf_entry.tft_pf_ipv6.tft_flow_label = cgtft_prm->flow_label;
        tft_pf_input.tft_pf_valid_bits |= NAS_TFT_ID_FLOW_LABEL;
      }
      
      break;
    } /* END: case NAS_is_tft_pf_ipv6 */
    
    default:
    {
      cmdCmeError(CME_ERR_OpNotAllow);
      return (ACI_BAT_FAIL);
    }
  } /* END: switch( tft_pf_input.ctrl_tft_pf_entry ) */
  
  ret = (T_ACI_BAT_RSLT) sAT_PlusCGTFT ((T_ACI_CMD_SRC)src_infos_psi->srcId, (U8)cgtft_prm->cid, &tft_pf_input);
  
  switch( ret )
  {
    case (ACI_BAT_CMPL):              /* Operation completed */
    case (ACI_BAT_EXCT):              /* Operation in progress */
      return ret;
    default:
      cmdCmeError(CME_ERR_Unknown);   /* Command failed */
      return (ACI_BAT_FAIL);
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : ACI_BAT              |
| STATE   : code                      ROUTINE : qBAT_PlusCGTFT       |
+--------------------------------------------------------------------+

  PURPOSE :
*/
GLOBAL T_ACI_BAT_RSLT qBAT_PlusCGTFT     (T_ACI_DTI_PRC_PSI *src_infos_psi,
                                          T_BAT_cmd_send    *cmd)
{
  T_ACI_BAT_RSLT            ret = ACI_BAT_FAIL;
  T_PDP_CONTEXT_INTERNAL    *p_pdp_context_node = NULL;
  T_TFT_INTERNAL            *p_tft_pf_node = NULL;
  U8                        pdp_context_cid_array[PDP_CONTEXT_CID_MAX];
  T_BAT_cmd_response        resp;
  T_BAT_res_que_plus_cgtft  que_cgtft_buffer;
  int i;

  TRACE_FUNCTION("qBAT_PlusCGTFT()");
  resp.ctrl_response = BAT_RES_QUE_PLUS_CGTFT;
  resp.response.ptr_que_plus_cgtft = &que_cgtft_buffer;
  
  memset( &pdp_context_cid_array, PDP_CONTEXT_CID_OMITTED, sizeof(pdp_context_cid_array) );
  ret = (T_ACI_BAT_RSLT) qAT_PlusCGTFT((T_ACI_CMD_SRC)src_infos_psi->srcId, (U8*) &pdp_context_cid_array[0]);

  if (ret EQ ACI_BAT_CMPL)
  {
    for ( i = 0; i < PDP_CONTEXT_CID_MAX AND pdp_context_cid_array[i] NEQ PDP_CONTEXT_CID_OMITTED; i++ )
    {
      p_pdp_context_node = pdp_context_find_node_from_cid( pdp_context_cid_array[i] );
      p_tft_pf_node = p_pdp_context_node->p_tft_pf;

      while( p_tft_pf_node )
      {
        memset(&que_cgtft_buffer, 0,sizeof(T_BAT_res_que_plus_cgtft));
        que_cgtft_buffer.cid = (T_BAT_pdp_cid)pdp_context_cid_array[i];
        que_cgtft_buffer.tft_pf_id = p_tft_pf_node->pf_attributes.tft_pf_id;
        que_cgtft_buffer.tft_pf_precedence = p_tft_pf_node->pf_attributes.tft_pf_precedence;

        switch( p_tft_pf_node->pf_attributes.ctrl_tft_pf_entry )
        {
          case NAS_is_tft_pf_ipv4:
            if( p_tft_pf_node->pf_attributes.tft_pf_valid_bits & NAS_TFT_ID_IPv4_SRC_ADDR_MASK )
            {            
              que_cgtft_buffer.v_addr_n_mask = TRUE;
              que_cgtft_buffer.c_addr_n_mask = 8;
              memcpy(que_cgtft_buffer.addr_n_mask, p_tft_pf_node->pf_attributes.tft_pf_entry.tft_pf_ipv4.tft_ipv4_src_addr_mask.tft_ipv4_addr,que_cgtft_buffer.c_addr_n_mask);
            }
            if( p_tft_pf_node->pf_attributes.tft_pf_valid_bits & NAS_TFT_ID_PROTOCOL_OR_NEXT_HDR )
            {
              que_cgtft_buffer.prt_num_or_nxt_hd = p_tft_pf_node->pf_attributes.tft_pf_entry.tft_pf_ipv4.tft_protocol;
            }
            else
            {
              que_cgtft_buffer.prt_num_or_nxt_hd = (U8)TFT_PROT_OR_NXT_HDR_INVALID;
            }
            if( p_tft_pf_node->pf_attributes.tft_pf_valid_bits & NAS_TFT_ID_DEST_PORT_RANGE )
            {
              que_cgtft_buffer.c_dest_port_range = 2;
              que_cgtft_buffer.dest_port_range[0] = p_tft_pf_node->pf_attributes.tft_pf_entry.tft_pf_ipv4.tft_dest_port_range.low_limit;
              que_cgtft_buffer.dest_port_range[1] = p_tft_pf_node->pf_attributes.tft_pf_entry.tft_pf_ipv4.tft_dest_port_range.high_limit;
            }
            if( p_tft_pf_node->pf_attributes.tft_pf_valid_bits & NAS_TFT_ID_SRC_PORT_RANGE )
            {
              que_cgtft_buffer.c_src_port_range = 2;
              que_cgtft_buffer.src_port_range[0] = p_tft_pf_node->pf_attributes.tft_pf_entry.tft_pf_ipv4.tft_src_port_range.low_limit;
              que_cgtft_buffer.src_port_range[1] = p_tft_pf_node->pf_attributes.tft_pf_entry.tft_pf_ipv4.tft_src_port_range.high_limit;
            }
            if( p_tft_pf_node->pf_attributes.tft_pf_valid_bits & NAS_TFT_ID_IPSEC_SPI )
            {
              que_cgtft_buffer.ipsec_spi = p_tft_pf_node->pf_attributes.tft_pf_entry.tft_pf_ipv4.tft_ipsec_spi;
            }
            if( p_tft_pf_node->pf_attributes.tft_pf_valid_bits & NAS_TFT_ID_TOS_AND_MASK )
            {
              que_cgtft_buffer.c_tos_mask_or_tc_mask = 2;
              que_cgtft_buffer.tos_mask_or_tc_mask[0] = p_tft_pf_node->pf_attributes.tft_pf_entry.tft_pf_ipv4.tft_tos_and_mask.tos_value;
              que_cgtft_buffer.tos_mask_or_tc_mask[1] = p_tft_pf_node->pf_attributes.tft_pf_entry.tft_pf_ipv4.tft_tos_and_mask.tos_mask;
            }
            que_cgtft_buffer.flow_label = (U8)TFT_FLOW_LABEL_INVALID;
            break;

          case NAS_is_tft_pf_ipv6:
            if( p_tft_pf_node->pf_attributes.tft_pf_valid_bits & NAS_TFT_ID_IPv6_SRC_ADDR_MASK )
            {            
              que_cgtft_buffer.v_addr_n_mask = TRUE;
              que_cgtft_buffer.c_addr_n_mask = 32;
              memcpy(que_cgtft_buffer.addr_n_mask, p_tft_pf_node->pf_attributes.tft_pf_entry.tft_pf_ipv6.tft_ipv6_src_addr_mask.tft_ipv6_addr,que_cgtft_buffer.c_addr_n_mask);
            }
            if( p_tft_pf_node->pf_attributes.tft_pf_valid_bits & NAS_TFT_ID_PROTOCOL_OR_NEXT_HDR )
            {
              que_cgtft_buffer.prt_num_or_nxt_hd = p_tft_pf_node->pf_attributes.tft_pf_entry.tft_pf_ipv6.tft_next_hdr;
            }
            else
            {
              que_cgtft_buffer.prt_num_or_nxt_hd = (U8)TFT_PROT_OR_NXT_HDR_INVALID;
            }
            if( p_tft_pf_node->pf_attributes.tft_pf_valid_bits & NAS_TFT_ID_DEST_PORT_RANGE )
            {
              que_cgtft_buffer.c_dest_port_range = 2;
              que_cgtft_buffer.dest_port_range[0] = p_tft_pf_node->pf_attributes.tft_pf_entry.tft_pf_ipv6.tft_dest_port_range.low_limit;
              que_cgtft_buffer.dest_port_range[1] = p_tft_pf_node->pf_attributes.tft_pf_entry.tft_pf_ipv6.tft_dest_port_range.high_limit;
            }
            if( p_tft_pf_node->pf_attributes.tft_pf_valid_bits & NAS_TFT_ID_SRC_PORT_RANGE )
            {
              que_cgtft_buffer.c_src_port_range = 2;
              que_cgtft_buffer.src_port_range[0] = p_tft_pf_node->pf_attributes.tft_pf_entry.tft_pf_ipv6.tft_src_port_range.low_limit;
              que_cgtft_buffer.src_port_range[1] = p_tft_pf_node->pf_attributes.tft_pf_entry.tft_pf_ipv6.tft_src_port_range.high_limit;
            }
            if( p_tft_pf_node->pf_attributes.tft_pf_valid_bits & NAS_TFT_ID_IPSEC_SPI )
            {
              que_cgtft_buffer.ipsec_spi = p_tft_pf_node->pf_attributes.tft_pf_entry.tft_pf_ipv6.tft_ipsec_spi;
            }
            if( p_tft_pf_node->pf_attributes.tft_pf_valid_bits & NAS_TFT_ID_TOS_AND_MASK )
            {
              que_cgtft_buffer.c_tos_mask_or_tc_mask = 2;
              que_cgtft_buffer.tos_mask_or_tc_mask[0] = p_tft_pf_node->pf_attributes.tft_pf_entry.tft_pf_ipv6.tft_tos_and_mask.tos_value;
              que_cgtft_buffer.tos_mask_or_tc_mask[1] = p_tft_pf_node->pf_attributes.tft_pf_entry.tft_pf_ipv6.tft_tos_and_mask.tos_mask;
            }
            if( p_tft_pf_node->pf_attributes.tft_pf_valid_bits & NAS_TFT_ID_FLOW_LABEL )
            {
              que_cgtft_buffer.flow_label = p_tft_pf_node->pf_attributes.tft_pf_entry.tft_pf_ipv6.tft_flow_label;
            }
            else
            {
              que_cgtft_buffer.flow_label = (U8)TFT_FLOW_LABEL_INVALID;
            }
            break;
        }
        aci_bat_send(src_infos_psi,&resp);
        
        p_tft_pf_node = p_tft_pf_node->p_next;
      }
    }
  }
  else
  {
    return (ret);
  }
  return ACI_BAT_CMPL;  
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : ACI_BAT              |
| STATE   : code                      ROUTINE : sBAT_PlusCGEQREQ     |
+--------------------------------------------------------------------+

  PURPOSE :
*/
GLOBAL T_ACI_BAT_RSLT sBAT_PlusCGEQREQ     (T_ACI_DTI_PRC_PSI *src_infos_psi,
                                            T_BAT_cmd_send    *cmd)
{
  T_ACI_BAT_RSLT ret = ACI_BAT_FAIL;
  T_PS_qos     input_qos;
  U8           cid = PDP_CONTEXT_CID_OMITTED;

  TRACE_FUNCTION ("sBAT_PlusCGEQREQ()");

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

  if (cmd->params.ptr_set_plus_cgeqreq->cid NEQ BAT_PDP_CID_NOT_PRESENT)
    cid = cmd->params.ptr_set_plus_cgeqreq->cid;
  if (cmd->params.ptr_set_plus_cgeqreq->tc NEQ NOT_PRESENT_8BIT)
    input_qos.qos_r99.tc = cmd->params.ptr_set_plus_cgeqreq->tc;
  if (cmd->params.ptr_set_plus_cgeqreq->order NEQ NOT_PRESENT_8BIT)
    input_qos.qos_r99.order = cmd->params.ptr_set_plus_cgeqreq->order;
  if (cmd->params.ptr_set_plus_cgeqreq->del_err_sdu NEQ NOT_PRESENT_8BIT)
    input_qos.qos_r99.del_err_sdu = cmd->params.ptr_set_plus_cgeqreq->del_err_sdu;
  if (cmd->params.ptr_set_plus_cgeqreq->max_sdu NEQ NOT_PRESENT_16BIT)
    input_qos.qos_r99.max_sdu = (U16)cmd->params.ptr_set_plus_cgeqreq->max_sdu;
  if (cmd->params.ptr_set_plus_cgeqreq->max_rate_ul NEQ NOT_PRESENT_16BIT)
    input_qos.qos_r99.max_rate_ul = (U16)cmd->params.ptr_set_plus_cgeqreq->max_rate_ul;
  if (cmd->params.ptr_set_plus_cgeqreq->max_rate_dl NEQ NOT_PRESENT_16BIT)
    input_qos.qos_r99.max_rate_dl = (U16)cmd->params.ptr_set_plus_cgeqreq->max_rate_dl;
  if (cmd->params.ptr_set_plus_cgeqreq->xfer_delay NEQ NOT_PRESENT_16BIT)
    input_qos.qos_r99.xfer_delay = (U16)cmd->params.ptr_set_plus_cgeqreq->xfer_delay;
  if (cmd->params.ptr_set_plus_cgeqreq->handling_pri NEQ NOT_PRESENT_8BIT)
    input_qos.qos_r99.handling_pri = (U8)cmd->params.ptr_set_plus_cgeqreq->handling_pri;
  if (cmd->params.ptr_set_plus_cgeqreq->guar_br_ul NEQ NOT_PRESENT_16BIT)
    input_qos.qos_r99.guar_br_ul = (U16)cmd->params.ptr_set_plus_cgeqreq->guar_br_ul;
  if (cmd->params.ptr_set_plus_cgeqreq->guar_br_dl NEQ NOT_PRESENT_16BIT)
    input_qos.qos_r99.guar_br_dl = (U16)cmd->params.ptr_set_plus_cgeqreq->guar_br_dl;
  if (cmd->params.ptr_set_plus_cgeqreq->c_ber NEQ 0)
  {
    input_qos.qos_r99.ber.ratio_mant = cmd->params.ptr_set_plus_cgeqreq->ber[0];
    input_qos.qos_r99.ber.ratio_exp = cmd->params.ptr_set_plus_cgeqreq->ber[1];
  }
  if (cmd->params.ptr_set_plus_cgeqreq->c_sdu_err_ratio NEQ 0)
  {
    input_qos.qos_r99.sdu_err_ratio.ratio_mant = cmd->params.ptr_set_plus_cgeqreq->sdu_err_ratio[0];
    input_qos.qos_r99.sdu_err_ratio.ratio_exp = cmd->params.ptr_set_plus_cgeqreq->sdu_err_ratio[1];
  }


  ret = (T_ACI_BAT_RSLT)sAT_PlusCGEQREQ ((T_ACI_CMD_SRC)src_infos_psi->srcId, (U8)cid, &input_qos);
  return(ret);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : ACI_BAT              |
| STATE   : code                      ROUTINE : qBAT_PlusCGEQREQ     |
+--------------------------------------------------------------------+

  PURPOSE :
*/
GLOBAL T_ACI_BAT_RSLT qBAT_PlusCGEQREQ     (T_ACI_DTI_PRC_PSI *src_infos_psi,
                                            T_BAT_cmd_send    *cmd)
{
  T_ACI_BAT_RSLT ret = ACI_BAT_FAIL;
  T_BAT_cmd_response resp;
  T_BAT_res_que_plus_cgeqreq que_cgeqreq_buffer;
  T_PS_qos     pdef_qos;
  U8           cid;
  BOOL         qos_valid = FALSE;

  TRACE_FUNCTION ("qBAT_PlusCGEQREQ()");

  resp.ctrl_response = BAT_RES_QUE_PLUS_CGEQREQ;
  resp.response.ptr_que_plus_cgeqreq = &que_cgeqreq_buffer;

  for ( cid = PDP_CONTEXT_CID_MIN; cid <= PDP_CONTEXT_CID_MAX; cid++ )
  {
    ret = (T_ACI_BAT_RSLT) qAT_PlusCGEQREQ ((T_ACI_CMD_SRC)src_infos_psi->srcId, cid, &qos_valid, &pdef_qos);
    if (ret EQ ACI_BAT_CMPL AND qos_valid)
    {
      que_cgeqreq_buffer.cid = (T_BAT_pdp_cid)cid;
      que_cgeqreq_buffer.tc = pdef_qos.qos_r99.tc;
      que_cgeqreq_buffer.order = pdef_qos.qos_r99.order;
      que_cgeqreq_buffer.del_err_sdu = pdef_qos.qos_r99.del_err_sdu;
      que_cgeqreq_buffer.max_sdu = pdef_qos.qos_r99.max_sdu;
      que_cgeqreq_buffer.max_rate_ul = pdef_qos.qos_r99.max_rate_ul;
      que_cgeqreq_buffer.max_rate_dl = pdef_qos.qos_r99.max_rate_dl;
      que_cgeqreq_buffer.xfer_delay = pdef_qos.qos_r99.xfer_delay;
      que_cgeqreq_buffer.handling_pri = pdef_qos.qos_r99.handling_pri;
      que_cgeqreq_buffer.guar_br_ul = pdef_qos.qos_r99.guar_br_ul;
      que_cgeqreq_buffer.guar_br_dl = pdef_qos.qos_r99.guar_br_dl;
      que_cgeqreq_buffer.ber[0] = pdef_qos.qos_r99.ber.ratio_mant;
      que_cgeqreq_buffer.ber[1] = pdef_qos.qos_r99.ber.ratio_exp;
      que_cgeqreq_buffer.sdu_err_ratio[0] = pdef_qos.qos_r99.sdu_err_ratio.ratio_mant;
      que_cgeqreq_buffer.sdu_err_ratio[1] = pdef_qos.qos_r99.sdu_err_ratio.ratio_exp;
      aci_bat_send(src_infos_psi,&resp);
    }
    else
      return ret;
  }
  return ACI_BAT_CMPL;
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : ACI_BAT              |
| STATE   : code                      ROUTINE : sBAT_PlusCGEQNEG     |
+--------------------------------------------------------------------+

  PURPOSE :
*/
GLOBAL T_ACI_BAT_RSLT sBAT_PlusCGEQNEG     (T_ACI_DTI_PRC_PSI *src_infos_psi,
                                            T_BAT_cmd_send    *cmd)
{
  T_ACI_BAT_RSLT ret = ACI_BAT_FAIL;
  T_BAT_res_que_plus_cgeqneg que_cgeqneg_buffer;
  T_BAT_cmd_response resp;
  T_PS_qos     pdef_qos;
  BOOL         qos_valid;
  U8           *cid_array;
  U8           current_cid;
  int          no_of_cids, i;

  TRACE_FUNCTION ("sBAT_PlusCGEQNEG()");

  resp.ctrl_response = BAT_RES_QUE_PLUS_CGEQNEG;
  resp.response.ptr_que_plus_cgeqneg = &que_cgeqneg_buffer;

  no_of_cids = cmd->params.ptr_set_plus_cgeqneg->c_cid;
  cid_array = (U8*)cmd->params.ptr_set_plus_cgeqneg->cid;

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

    ret = (T_ACI_BAT_RSLT) sAT_PlusCGEQNEG ((T_ACI_CMD_SRC) src_infos_psi->srcId, current_cid, &qos_valid, &pdef_qos);

    if (ret EQ ACI_BAT_CMPL)
    {
      if (qos_valid)
      {
        que_cgeqneg_buffer.cid = (T_BAT_pdp_cid)cid_array[i];
        que_cgeqneg_buffer.tc = (U8)pdef_qos.qos_r99.tc;
        que_cgeqneg_buffer.order = pdef_qos.qos_r99.order;
        que_cgeqneg_buffer.del_err_sdu = pdef_qos.qos_r99.del_err_sdu;
        que_cgeqneg_buffer.max_sdu = pdef_qos.qos_r99.max_sdu;
        que_cgeqneg_buffer.max_rate_ul = pdef_qos.qos_r99.max_rate_ul;
        que_cgeqneg_buffer.max_rate_dl = pdef_qos.qos_r99.max_rate_dl;
        que_cgeqneg_buffer.xfer_delay = pdef_qos.qos_r99.xfer_delay;
        que_cgeqneg_buffer.handling_pri = pdef_qos.qos_r99.handling_pri;
        que_cgeqneg_buffer.guar_br_ul = pdef_qos.qos_r99.guar_br_ul;
        que_cgeqneg_buffer.guar_br_dl = pdef_qos.qos_r99.guar_br_dl;
        que_cgeqneg_buffer.ber[0] = pdef_qos.qos_r99.ber.ratio_mant;
        que_cgeqneg_buffer.ber[1] = pdef_qos.qos_r99.ber.ratio_exp;
        que_cgeqneg_buffer.sdu_err_ratio[0] = pdef_qos.qos_r99.sdu_err_ratio.ratio_mant;
        que_cgeqneg_buffer.sdu_err_ratio[1] = pdef_qos.qos_r99.sdu_err_ratio.ratio_exp;

        aci_bat_send(src_infos_psi,&resp);
      }
    }
    else
      return ret;
  }
  return ACI_BAT_CMPL;
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : ACI_BAT              |
| STATE   : code                      ROUTINE : sBAT_PlusCGEQMIN     |
+--------------------------------------------------------------------+

  PURPOSE :
*/
GLOBAL T_ACI_BAT_RSLT sBAT_PlusCGEQMIN     (T_ACI_DTI_PRC_PSI *src_infos_psi,
                                            T_BAT_cmd_send    *cmd)
{
  T_ACI_BAT_RSLT ret = ACI_BAT_FAIL;
  T_PS_qos       input_qos;
  U8             cid;
  
  TRACE_FUNCTION ("sBAT_PlusCGEQMIN()");
  
  memset(&input_qos, 0, sizeof(T_PS_qos));
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

 if (cmd->params.ptr_set_plus_cgeqreq->cid NEQ BAT_PDP_CID_NOT_PRESENT)
    cid =  cmd->params.ptr_set_plus_cgeqmin->cid;

  if (cmd->params.ptr_set_plus_cgeqreq->tc NEQ NOT_PRESENT_8BIT)
    input_qos.qos_r99.tc = cmd->params.ptr_set_plus_cgeqmin->tc;

  if (cmd->params.ptr_set_plus_cgeqreq->order NEQ NOT_PRESENT_8BIT)
    input_qos.qos_r99.order = cmd->params.ptr_set_plus_cgeqmin->order;

  if (cmd->params.ptr_set_plus_cgeqreq->del_err_sdu NEQ NOT_PRESENT_8BIT)
    input_qos.qos_r99.del_err_sdu = cmd->params.ptr_set_plus_cgeqmin->del_err_sdu;

  if (cmd->params.ptr_set_plus_cgeqreq->max_sdu NEQ NOT_PRESENT_16BIT)
    input_qos.qos_r99.max_sdu = (U16)cmd->params.ptr_set_plus_cgeqmin->max_sdu;

  if (cmd->params.ptr_set_plus_cgeqreq->max_rate_ul NEQ NOT_PRESENT_16BIT)
    input_qos.qos_r99.max_rate_ul = (U16)cmd->params.ptr_set_plus_cgeqmin->max_rate_ul;

  if (cmd->params.ptr_set_plus_cgeqreq->max_rate_dl NEQ NOT_PRESENT_16BIT)
    input_qos.qos_r99.max_rate_dl = (U16)cmd->params.ptr_set_plus_cgeqmin->max_rate_dl;

  if (cmd->params.ptr_set_plus_cgeqreq->xfer_delay NEQ NOT_PRESENT_16BIT)
    input_qos.qos_r99.xfer_delay = (U16)cmd->params.ptr_set_plus_cgeqmin->xfer_delay;

  if (cmd->params.ptr_set_plus_cgeqreq->handling_pri NEQ NOT_PRESENT_8BIT)
    input_qos.qos_r99.handling_pri = (U8)cmd->params.ptr_set_plus_cgeqmin->handling_pri;

  if (cmd->params.ptr_set_plus_cgeqreq->guar_br_ul NEQ NOT_PRESENT_16BIT)
    input_qos.qos_r99.guar_br_ul = (U16)cmd->params.ptr_set_plus_cgeqmin->guar_br_ul;

  if (cmd->params.ptr_set_plus_cgeqreq->guar_br_dl NEQ NOT_PRESENT_16BIT)
    input_qos.qos_r99.guar_br_dl = (U16)cmd->params.ptr_set_plus_cgeqmin->guar_br_dl;

  if (cmd->params.ptr_set_plus_cgeqreq->c_ber NEQ 0)
  {
    input_qos.qos_r99.ber.ratio_mant = cmd->params.ptr_set_plus_cgeqmin->ber[0];
    input_qos.qos_r99.ber.ratio_exp = cmd->params.ptr_set_plus_cgeqmin->ber[1];
  }
  if (cmd->params.ptr_set_plus_cgeqreq->c_sdu_err_ratio NEQ 0)
  {
    input_qos.qos_r99.sdu_err_ratio.ratio_mant = cmd->params.ptr_set_plus_cgeqmin->sdu_err_ratio[0];
    input_qos.qos_r99.sdu_err_ratio.ratio_exp = cmd->params.ptr_set_plus_cgeqmin->sdu_err_ratio[1];
  }

  ret = (T_ACI_BAT_RSLT)sAT_PlusCGEQMIN  ((T_ACI_CMD_SRC)src_infos_psi->srcId, cid ,&input_qos);

  return ret;
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : ACI_BAT              |
| STATE   : code                      ROUTINE : qBAT_PlusCGEQMIN     |
+--------------------------------------------------------------------+

  PURPOSE :
*/
GLOBAL T_ACI_BAT_RSLT qBAT_PlusCGEQMIN     (T_ACI_DTI_PRC_PSI *src_infos_psi,
                                            T_BAT_cmd_send    *cmd)
{
  T_ACI_BAT_RSLT              ret = ACI_BAT_FAIL;
  T_BAT_cmd_response          resp;
  T_BAT_res_que_plus_cgeqmin  que_cgeqmin_buffer;
  T_PS_min_qos                pdef_qos;
  U8                          cid;
  BOOL                        qos_valid;

  TRACE_FUNCTION ("qBAT_PlusCGEQMIN()");

  resp.ctrl_response = BAT_RES_QUE_PLUS_CGEQMIN;
  resp.response.ptr_que_plus_cgeqmin = &que_cgeqmin_buffer;

  for ( cid = PDP_CONTEXT_CID_MIN; cid <= PDP_CONTEXT_CID_MAX; cid++ )
  {
    ret = (T_ACI_BAT_RSLT) qAT_PlusCGEQMIN ((T_ACI_CMD_SRC) src_infos_psi->srcId, cid, &qos_valid, &pdef_qos);
    if ( ret EQ ACI_BAT_CMPL)
    {
      if (qos_valid)
      {
        que_cgeqmin_buffer.cid = (T_BAT_pdp_cid)cid;
        que_cgeqmin_buffer.tc = (U8)pdef_qos.qos_r99.tc;
        que_cgeqmin_buffer.order = pdef_qos.qos_r99.order;
        que_cgeqmin_buffer.del_err_sdu = pdef_qos.qos_r99.del_err_sdu;
        que_cgeqmin_buffer.max_sdu = pdef_qos.qos_r99.max_sdu;
        que_cgeqmin_buffer.max_rate_ul = pdef_qos.qos_r99.max_rate_ul;
        que_cgeqmin_buffer.max_rate_dl = pdef_qos.qos_r99.max_rate_dl;
        que_cgeqmin_buffer.xfer_delay = pdef_qos.qos_r99.xfer_delay;
        que_cgeqmin_buffer.handling_pri = pdef_qos.qos_r99.handling_pri;
        que_cgeqmin_buffer.guar_br_ul = pdef_qos.qos_r99.guar_br_ul;
        que_cgeqmin_buffer.guar_br_dl = pdef_qos.qos_r99.guar_br_dl;
        que_cgeqmin_buffer.ber[0] = pdef_qos.qos_r99.ber.ratio_mant;
        que_cgeqmin_buffer.ber[1] = pdef_qos.qos_r99.ber.ratio_exp;
        que_cgeqmin_buffer.sdu_err_ratio[0] = pdef_qos.qos_r99.sdu_err_ratio.ratio_mant;
        que_cgeqmin_buffer.sdu_err_ratio[1] = pdef_qos.qos_r99.sdu_err_ratio.ratio_exp;

        aci_bat_send(src_infos_psi,&resp);
      }
    }
    else
      return ret;
  }
  return ACI_BAT_CMPL;
}

#endif /* REL99 */

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : ACI_BAT              |
| STATE   : code                      ROUTINE : sBAT_PlusCGQREQ      |
+--------------------------------------------------------------------+

  PURPOSE : 
*/
GLOBAL T_ACI_BAT_RSLT sBAT_PlusCGQREQ      (T_ACI_DTI_PRC_PSI *src_infos_psi,
                                            T_BAT_cmd_send    *cmd)
{
  T_ACI_BAT_RSLT ret = ACI_BAT_FAIL;
  T_PS_qos input_qos;
  T_BAT_pdp_cid  cid = BAT_PDP_CID_NOT_PRESENT ;
  
  TRACE_FUNCTION ("sBAT_PlusCGQREQ()");
  
  cid = cmd->params.ptr_set_plus_cgqreq->cid;
  if (cid EQ BAT_PDP_CID_NOT_PRESENT )
  {
    cid = (T_BAT_pdp_cid)GPRS_CID_OMITTED ;
  }
  input_qos.qos_r97.preced = (U8)cmd->params.ptr_set_plus_cgqreq->precedence;
  input_qos.qos_r97.delay = (U8)cmd->params.ptr_set_plus_cgqreq->delay;
  input_qos.qos_r97.relclass = (U8)cmd->params.ptr_set_plus_cgqreq->reliability;
  input_qos.qos_r97.peak  = (U8)cmd->params.ptr_set_plus_cgqreq->peak;
  input_qos.qos_r97.mean = (U8)cmd->params.ptr_set_plus_cgqreq->mean;
  
  if (input_qos.qos_r97.preced EQ (U8)BAT_PRECEDENCE_NOT_PRESENT)
  {
    input_qos.qos_r97.preced = PDP_CONTEXT_QOS_OMITTED ;
  }
  if (input_qos.qos_r97.delay EQ (U8)BAT_DELAY_NOT_PRESENT)
  {
    input_qos.qos_r97.delay = PDP_CONTEXT_QOS_OMITTED ;
  }
  if (input_qos.qos_r97.relclass EQ (U8)BAT_RELIABILITY_NOT_PRESENT)
  {
    input_qos.qos_r97.relclass = PDP_CONTEXT_QOS_OMITTED ;
  }
  if (input_qos.qos_r97.peak EQ (U8)BAT_PEAK_NOT_PRESENT)
  {
    input_qos.qos_r97.peak= PDP_CONTEXT_QOS_OMITTED ;
  }
  if (input_qos.qos_r97.mean EQ (U8)BAT_MEAN_NOT_PRESENT)
  {
    input_qos.qos_r97.mean = PDP_CONTEXT_QOS_OMITTED ;
  }
  ret = (T_ACI_BAT_RSLT) sAT_PlusCGQREQ ((T_ACI_CMD_SRC)src_infos_psi->srcId, (U8)cid ,&input_qos);
  return(ret);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : ACI_BAT              |
| STATE   : code                      ROUTINE : qBAT_PlusCGQREQ      |
+--------------------------------------------------------------------+

  PURPOSE : 
*/
GLOBAL T_ACI_BAT_RSLT qBAT_PlusCGQREQ      (T_ACI_DTI_PRC_PSI *src_infos_psi,
                                            T_BAT_cmd_send    *cmd)
{
  T_PS_qos pdef_qos;
  BOOL qos_valid = FALSE;
  T_PDP_CONTEXT_INTERNAL *p_pdp_context_node = NULL;
  T_BAT_cmd_response resp;
  T_BAT_res_que_plus_cgqreq que_cgqreq_buffer;
 
  TRACE_FUNCTION ("qBAT_PlusCGQREQ()");

  resp.ctrl_response = BAT_RES_QUE_PLUS_CGQREQ;
  resp.response.ptr_que_plus_cgqreq = &que_cgqreq_buffer;
  
  p_pdp_context_node = p_pdp_context_list;

  while( p_pdp_context_node )
  {
    if (qAT_PlusCGQREQ ( (T_ACI_CMD_SRC)src_infos_psi->srcId, p_pdp_context_node->cid, &qos_valid, &pdef_qos ) EQ AT_CMPL)
    {
      if( qos_valid )
      {
        resp.response.ptr_que_plus_cgqreq->cid = (T_BAT_pdp_cid)p_pdp_context_node->cid;
        resp.response.ptr_que_plus_cgqreq->precedence = (T_BAT_precedence)pdef_qos.qos_r97.preced;
        resp.response.ptr_que_plus_cgqreq->delay = (T_BAT_delay)pdef_qos.qos_r97.delay;
        resp.response.ptr_que_plus_cgqreq->reliability= (T_BAT_reliability)pdef_qos.qos_r97.relclass;
        resp.response.ptr_que_plus_cgqreq->peak = (T_BAT_peak)pdef_qos.qos_r97.peak;
        resp.response.ptr_que_plus_cgqreq->mean = (T_BAT_mean)pdef_qos.qos_r97.mean ;

        aci_bat_send(src_infos_psi,&resp);
      }
    }
    else
    {
      ACI_ERR_DESC( ACI_ERR_CLASS_Cme, CME_ERR_Unknown);
      return (ACI_BAT_FAIL);
    }
    p_pdp_context_node = p_pdp_context_node->p_next;
  }
  return ACI_BAT_CMPL;
}
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : ACI_BAT              |
| STATE   : code                      ROUTINE : sBAT_PlusCGQMIN      |
+--------------------------------------------------------------------+

  PURPOSE : 
*/
GLOBAL T_ACI_BAT_RSLT sBAT_PlusCGQMIN      (T_ACI_DTI_PRC_PSI *src_infos_psi,
                                            T_BAT_cmd_send    *cmd)
{
  T_ACI_BAT_RSLT ret = ACI_BAT_FAIL;
  T_PS_min_qos input_qos;
  T_BAT_pdp_cid  cid = BAT_PDP_CID_NOT_PRESENT ;
  
  TRACE_FUNCTION ("sBAT_PlusCGQMIN()");
  cid = cmd->params.ptr_set_plus_cgqmin->cid;
  if (cid EQ BAT_PDP_CID_NOT_PRESENT )
  {
    cid = (T_BAT_pdp_cid)PDP_CONTEXT_CID_OMITTED ;
  }

  input_qos.qos_r97.delay    = (U8) PDP_CONTEXT_QOS_OMITTED;
  input_qos.qos_r97.relclass = (U8) PDP_CONTEXT_QOS_OMITTED;
  input_qos.qos_r97.peak     = (U8) PDP_CONTEXT_QOS_OMITTED;
  input_qos.qos_r97.preced   = (U8) PDP_CONTEXT_QOS_OMITTED;
  input_qos.qos_r97.mean     = (U8) PDP_CONTEXT_QOS_OMITTED;

  input_qos.qos_r97.preced = cmd->params.ptr_set_plus_cgqmin->precedence;
  input_qos.qos_r97.delay = cmd->params.ptr_set_plus_cgqmin->delay;
  input_qos.qos_r97.relclass = cmd->params.ptr_set_plus_cgqmin->reliability;
  input_qos.qos_r97.peak  = cmd->params.ptr_set_plus_cgqmin->peak;
  input_qos.qos_r97.mean = cmd->params.ptr_set_plus_cgqmin->mean;

  ret = (T_ACI_BAT_RSLT) sAT_PlusCGQMIN ((T_ACI_CMD_SRC)src_infos_psi->srcId, (U8)cid ,&input_qos);
  return(ret);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : ACI_BAT              |
| STATE   : code                      ROUTINE : qBAT_PlusCGQMIN      |
+--------------------------------------------------------------------+

  PURPOSE : 
*/
GLOBAL T_ACI_BAT_RSLT qBAT_PlusCGQMIN      (T_ACI_DTI_PRC_PSI *src_infos_psi,
                                            T_BAT_cmd_send    *cmd)
{
  T_BAT_cmd_response resp;
  T_BAT_res_que_plus_cgqmin que_cgqmin_buffer;
  T_PS_qos                pdef_qos;
  BOOL                    qos_valid          = FALSE;
  T_PDP_CONTEXT_INTERNAL *p_pdp_context_node = NULL;
  
  TRACE_FUNCTION ("qBAT_PlusCGQMIN()");

  resp.ctrl_response = BAT_RES_QUE_PLUS_CGQMIN;
  resp.response.ptr_que_plus_cgqmin = &que_cgqmin_buffer;  
  p_pdp_context_node = p_pdp_context_list;

  while( p_pdp_context_node )
  {
    if( qAT_PlusCGQMIN((T_ACI_CMD_SRC) src_infos_psi->srcId, p_pdp_context_node->cid, &qos_valid, &pdef_qos ) EQ AT_CMPL )
    {
      if (qos_valid)
      {
        resp.response.ptr_que_plus_cgqmin->cid = (T_BAT_pdp_cid)p_pdp_context_node->cid;
        resp.response.ptr_que_plus_cgqmin->precedence = (T_BAT_precedence)pdef_qos.qos_r97.preced;
        resp.response.ptr_que_plus_cgqmin->delay = (T_BAT_delay)pdef_qos.qos_r97.delay;
        resp.response.ptr_que_plus_cgqmin->reliability= (T_BAT_reliability)pdef_qos.qos_r97.relclass;
        resp.response.ptr_que_plus_cgqmin->peak = (T_BAT_peak)pdef_qos.qos_r97.peak;
        resp.response.ptr_que_plus_cgqmin->mean = (T_BAT_mean)pdef_qos.qos_r97.mean;
        aci_bat_send(src_infos_psi,&resp);
      }
    }
    else
    {
      ACI_ERR_DESC( ACI_ERR_CLASS_Cme, CME_ERR_Unknown);
      return (ACI_BAT_FAIL);
    }
    p_pdp_context_node = p_pdp_context_node->p_next;
  }
  return ACI_BAT_CMPL;
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : ACI_BAT              |
| STATE   : code                      ROUTINE : sBAT_PlusCGATT       |
+--------------------------------------------------------------------+

  PURPOSE : 
*/
GLOBAL T_ACI_BAT_RSLT sBAT_PlusCGATT       (T_ACI_DTI_PRC_PSI *src_infos_psi,
                                            T_BAT_cmd_send    *cmd)
{
  T_ACI_BAT_RSLT ret = ACI_BAT_FAIL;
  T_CGATT_STATE state = CGATT_STATE_OMITTED;
 
  TRACE_FUNCTION ("sBAT_PlusCGATT()");
  
  state = (T_CGATT_STATE)cmd->params.ptr_set_plus_cgatt->state;
  ret = (T_ACI_BAT_RSLT)sAT_PlusCGATT ( (T_ACI_CMD_SRC)src_infos_psi->srcId, state );
  return(ret);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : ACI_BAT              |
| STATE   : code                      ROUTINE : qBAT_PlusCGATT       |
+--------------------------------------------------------------------+

  PURPOSE : 
*/
GLOBAL T_ACI_BAT_RSLT qBAT_PlusCGATT       (T_ACI_DTI_PRC_PSI *src_infos_psi,
                                            T_BAT_cmd_send    *cmd)
{
  T_ACI_BAT_RSLT ret = ACI_BAT_FAIL;
  T_BAT_cmd_response resp;
  T_BAT_res_que_plus_cgatt que_cgatt_buffer;
  T_CGATT_STATE state = CGATT_STATE_OMITTED;
  
  TRACE_FUNCTION ("qBAT_PlusCGATT()");
  resp.ctrl_response = BAT_RES_QUE_PLUS_CGATT;
  resp.response.ptr_que_plus_cgatt = &que_cgatt_buffer;
  
  ret = (T_ACI_BAT_RSLT)qAT_PlusCGATT((T_ACI_CMD_SRC)src_infos_psi->srcId, &state);
  if (ret EQ ACI_BAT_CMPL)
  {
    resp.response.ptr_que_plus_cgatt->state = (T_BAT_plus_cgatt_state)state;
    aci_bat_send(src_infos_psi,&resp); 
    return ACI_BAT_CMPL;
  }
  ACI_ERR_DESC( ACI_ERR_CLASS_Cme, CME_ERR_Unknown);
  return (ACI_BAT_FAIL);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : ACI_BAT              |
| STATE   : code                      ROUTINE : sBAT_PlusCGCLASS     |
+--------------------------------------------------------------------+

  PURPOSE : 
*/
GLOBAL T_ACI_BAT_RSLT sBAT_PlusCGCLASS     (T_ACI_DTI_PRC_PSI *src_infos_psi,
                                            T_BAT_cmd_send    *cmd)
{
  T_ACI_BAT_RSLT ret = ACI_BAT_FAIL;
  T_CGCLASS_CLASS m_class = CGCLASS_CLASS_OMITTED;
  
  TRACE_FUNCTION ("sBAT_PlusCGCLASS()");
  
  m_class = (T_CGCLASS_CLASS)cmd->params.ptr_set_plus_cgclass->mobile_class;

  if (m_class EQ CGCLASS_CLASS_OMITTED)
  {
    ACI_ERR_DESC(ACI_ERR_CLASS_Cme,CME_ERR_OpNotAllow);
    return (ACI_BAT_FAIL);
  }
  ret =(T_ACI_BAT_RSLT) sAT_PlusCGCLASS ((T_ACI_CMD_SRC)src_infos_psi->srcId, m_class );
  return(ret);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : ACI_BAT              |
| STATE   : code                      ROUTINE : qBAT_PlusCGCLASS     |
+--------------------------------------------------------------------+

  PURPOSE : 
*/
GLOBAL T_ACI_BAT_RSLT qBAT_PlusCGCLASS     (T_ACI_DTI_PRC_PSI *src_infos_psi,
                                            T_BAT_cmd_send    *cmd)
{
  T_ACI_BAT_RSLT ret;
  T_CGCLASS_CLASS m_class;
  
  TRACE_FUNCTION ("qBAT_PlusCGCLASS()");

  ret=(T_ACI_BAT_RSLT)qAT_PlusCGCLASS((T_ACI_CMD_SRC)src_infos_psi->srcId,&m_class);

  /*
  *   If the query completes, send the response now.
  */
  if (ret EQ ACI_BAT_CMPL)
  {
    T_BAT_cmd_response resp;
    T_BAT_res_que_plus_cgclass cgclass_data;
 
    resp.ctrl_response=BAT_RES_QUE_PLUS_CGCLASS;
    resp.response.ptr_que_plus_cgclass=&cgclass_data;

    /*
    *   This relies on T_BAT_mobile_class and T_CGCLASS_CLASS
    *   being identical.
    */
    resp.response.ptr_que_plus_cgclass->mobile_class=(T_BAT_mobile_class)m_class;

    aci_bat_send(src_infos_psi,&resp);
    return ACI_BAT_CMPL;
  }
  ACI_ERR_DESC( ACI_ERR_CLASS_Cme, CME_ERR_Unknown);
  return (ACI_BAT_FAIL);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : ACI_BAT              |
| STATE   : code                      ROUTINE : sBAT_PlusCGACT       |
+--------------------------------------------------------------------+

  PURPOSE : 
*/
GLOBAL T_ACI_BAT_RSLT sBAT_PlusCGACT       (T_ACI_DTI_PRC_PSI *src_infos_psi,
                                            T_BAT_cmd_send    *cmd)
{
  T_ACI_BAT_RSLT ret = ACI_BAT_FAIL;
  T_CGACT_STATE state = CGACT_STATE_OMITTED;
  SHORT count = 0;
  SHORT cid_array[PDP_CONTEXT_CID_MAX];
  
  TRACE_FUNCTION ("sBAT_PlusCGACT()");
 
  state =(T_CGACT_STATE) cmd->params.ptr_set_plus_cgact->state;  
  if(cmd->params.ptr_set_plus_cgact->c_cid EQ 0)
    cid_array[0] = PDP_CONTEXT_CID_INVALID;
  for(count=0;count<cmd->params.ptr_set_plus_cgact->c_cid;count++)
  {
    cid_array[count] = (cmd->params.ptr_set_plus_cgact->cid[count] EQ BAT_PDP_CID_NOT_PRESENT)
                   ? PDP_CONTEXT_CID_INVALID : cmd->params.ptr_set_plus_cgact->cid[count];
  }
  if(count > 0)
    cid_array[count]= PDP_CONTEXT_CID_INVALID;
  ret =(T_ACI_BAT_RSLT) sAT_PlusCGACT ((T_ACI_CMD_SRC)src_infos_psi->srcId, state, cid_array );
  return(ret);
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : ACI_BAT              |
| STATE   : code                      ROUTINE : qBAT_PlusCGACT       |
+--------------------------------------------------------------------+

  PURPOSE : 
*/
GLOBAL T_ACI_BAT_RSLT qBAT_PlusCGACT       (T_ACI_DTI_PRC_PSI *src_infos_psi,
                                            T_BAT_cmd_send    *cmd)
{
  T_ACI_BAT_RSLT ret = ACI_BAT_FAIL;  
  SHORT cid_array[PDP_CONTEXT_CID_MAX];
  SHORT count = 0;
  BOOL  states[PDP_CONTEXT_CID_MAX];
  T_BAT_cmd_response resp;
  T_BAT_res_que_plus_cgact que_cgact_buffer;
  
  TRACE_FUNCTION ("qBAT_PlusCGACT()");
  resp.ctrl_response = BAT_RES_QUE_PLUS_CGACT;
  resp.response.ptr_que_plus_cgact = &que_cgact_buffer;  
  
  ret=(T_ACI_BAT_RSLT)qAT_PlusCGACT((T_ACI_CMD_SRC)src_infos_psi->srcId, states, cid_array);
  if(ret EQ ACI_BAT_CMPL)
  {
    for ( count = 0; count < PDP_CONTEXT_CID_MAX AND cid_array[count] NEQ PDP_CONTEXT_CID_INVALID; count++ )
    {
      resp.response.ptr_que_plus_cgact->state = (T_BAT_plus_cgact_state)states[count];    
      resp.response.ptr_que_plus_cgact->cid = (T_BAT_pdp_cid)cid_array[count];
      aci_bat_send(src_infos_psi,&resp);
    }
    return(ret);
  }
  ACI_ERR_DESC( ACI_ERR_CLASS_Cme, CME_ERR_Unknown);
  return (ACI_BAT_FAIL);  
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : ACI_BAT              |
| STATE   : code                      ROUTINE : sBAT_PlusCGDATA      |
+--------------------------------------------------------------------+

  PURPOSE : 
*/
GLOBAL T_ACI_BAT_RSLT sBAT_PlusCGDATA      (T_ACI_DTI_PRC_PSI *src_infos_psi,
                                            T_BAT_cmd_send    *cmd)
{
  T_ACI_BAT_RSLT ret = ACI_BAT_FAIL;
  char L2P[MAX_L2P_LENGTH] = "";  
  U8 cid_array[PDP_CONTEXT_CID_MAX] = { GPRS_CID_OMITTED };
  
  TRACE_FUNCTION ("sBAT_PlusCGDATA()");
  
  if(cmd->params.ptr_set_plus_cgdata->v_l2p)
  {
    memcpy(L2P,cmd->params.ptr_set_plus_cgdata->l2p,cmd->params.ptr_set_plus_cgdata->c_l2p);
  }
  else
  {
    L2P[0] = 0;
  }
 
  if(cmd->params.ptr_set_plus_cgdata->v_cid)
  {
    memcpy(cid_array,cmd->params.ptr_set_plus_cgdata->cid,2*cmd->params.ptr_set_plus_cgdata->c_cid);
    cid_array[cmd->params.ptr_set_plus_cgdata->c_cid] = PDP_CONTEXT_CID_INVALID;
  }
  ret =(T_ACI_BAT_RSLT) sAT_PlusCGDATA ((T_ACI_CMD_SRC)src_infos_psi->srcId, L2P, cid_array ); 
  return(ret);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : ACI_BAT              |
| STATE   : code                      ROUTINE : sBAT_PlusCGPADDR     |
+--------------------------------------------------------------------+

  PURPOSE : 
*/
GLOBAL T_ACI_BAT_RSLT sBAT_PlusCGPADDR     (T_ACI_DTI_PRC_PSI *src_infos_psi,
                                            T_BAT_cmd_send    *cmd)
{
  T_ACI_BAT_RSLT ret = ACI_BAT_FAIL;
  T_NAS_ip                    *pdp_addresses;
  SHORT         count = 0;
  SHORT                         cid_array[PDP_CONTEXT_CID_MAX + 1];
  T_BAT_cmd_set_plus_cgpaddr *cgpaddr;

  TRACE_FUNCTION ("sBAT_PlusCGPADDR()");

  ACI_MALLOC (pdp_addresses, sizeof (T_NAS_ip) * PDP_CONTEXT_CID_MAX);

  /*
  *   Get a pointer to the CGPADDR data for convenience.
  */
  cgpaddr=cmd->params.ptr_set_plus_cgpaddr;
  
  if (cgpaddr->c_cid)
  {
    for (count=0; ((count<PDP_CONTEXT_CID_MAX) AND (count<cgpaddr->c_cid)); count++)
      cid_array[count]=(U8)cgpaddr->cid[count];

    cid_array[count] = PDP_CONTEXT_CID_INVALID;
  }
  else
  {
    cid_array[0] = PDP_CONTEXT_CID_OMITTED;
  }

  ret = (T_ACI_BAT_RSLT)sAT_PlusCGPADDR((T_ACI_CMD_SRC)src_infos_psi->srcId, cid_array, pdp_addresses );

  if (ret EQ ACI_BAT_CMPL)
  {
    T_BAT_cmd_response resp;
    T_BAT_res_set_plus_cgpaddr cgpaddr_data;
    UBYTE len = 0;
 
    resp.ctrl_response=BAT_RES_SET_PLUS_CGPADDR;
    resp.response.ptr_set_plus_cgpaddr=&cgpaddr_data;

    for (count=0;count<PDP_CONTEXT_CID_MAX;count++)
    {
      if (cid_array[count] EQ PDP_CONTEXT_CID_INVALID)
        break;

      cgpaddr_data.cid=(T_BAT_pdp_cid)cid_array[count];

      switch( pdp_addresses[count].ctrl_ip_address )
      {
        case NAS_is_ip_not_present:
          len = 0;
          break;

        case NAS_is_ipv4:
          len = 4;
          memcpy(cgpaddr_data.pdp_addr, pdp_addresses[count].ip_address.ipv4_addr.a4,len);
          break;

        case NAS_is_ipv6:
          len = 16;
          memcpy(cgpaddr_data.pdp_addr, pdp_addresses[count].ip_address.ipv6_addr.a6,len);
          break;
      }

      cgpaddr_data.c_pdp_addr=(U8)len;
      aci_bat_send(src_infos_psi,&resp);
    }
  }

  ACI_MFREE(pdp_addresses);
  return(ret);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : ACI_BAT              |
| STATE   : code                      ROUTINE : sBAT_PlusCGANS       |
+--------------------------------------------------------------------+

  PURPOSE : 
*/
GLOBAL T_ACI_BAT_RSLT sBAT_PlusCGANS       (T_ACI_DTI_PRC_PSI *src_infos_psi,
                                            T_BAT_cmd_send    *cmd)
{
  T_ACI_BAT_RSLT ret = ACI_BAT_FAIL;
  char L2P[MAX_L2P_LENGTH] = "" ;
  SHORT cid = GPRS_CID_OMITTED, response = -1 ;
  
  TRACE_FUNCTION ("sBAT_PlusCGANS()");
 
    response = cmd->params.ptr_set_plus_cgans->response;
    if(cmd->params.ptr_set_plus_cgans->v_l2p)
    {
      memcpy(L2P,cmd->params.ptr_set_plus_cgans->l2p,cmd->params.ptr_set_plus_cgans->c_l2p);
    }
    else
   {
     L2P[0] = 0;
   }
    
   cid = cmd->params.ptr_set_plus_cgans->cid;
   if (cid EQ BAT_PDP_CID_NOT_PRESENT )
  {
    cid = GPRS_CID_OMITTED ;
  }    
  ret =(T_ACI_BAT_RSLT)sAT_PlusCGANS ((T_ACI_CMD_SRC) src_infos_psi->srcId, response, L2P, cid );
  return(ret);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : ACI_BAT              |
| STATE   : code                      ROUTINE : qBAT_PlusCGREG       |
+--------------------------------------------------------------------+

  PURPOSE : 
*/
GLOBAL T_ACI_BAT_RSLT qBAT_PlusCGREG       (T_ACI_DTI_PRC_PSI *src_infos_psi,
                                            T_BAT_cmd_send    *cmd)
{
  T_ACI_BAT_RSLT ret = ACI_BAT_FAIL;
  T_CGREG_STAT   stat;
  USHORT         lac, cid;    
 
  TRACE_FUNCTION ("qBAT_PlusCGREG()");
 
  ret=(T_ACI_BAT_RSLT)qAT_PlusCGREG((T_ACI_CMD_SRC)src_infos_psi->srcId, &stat, &lac, &cid);

  if (ret==ACI_BAT_CMPL)
  {
    T_BAT_cmd_response resp;
    T_BAT_res_que_plus_cgreg que_cgreg_buffer;
 
    resp.ctrl_response = BAT_RES_QUE_PLUS_CGREG;
    resp.response.ptr_que_plus_cgreg = &que_cgreg_buffer;

    resp.response.ptr_que_plus_cgreg->stat = (T_BAT_plus_cgreg_stat)stat;
    resp.response.ptr_que_plus_cgreg->lac = lac;
    resp.response.ptr_que_plus_cgreg->ci = cid;
    aci_bat_send(src_infos_psi,&resp);
  }

  return(ret);  
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : ACI_BAT              |
| STATE   : code                      ROUTINE : qBAT_PercentCGREG    |
+--------------------------------------------------------------------+

  PURPOSE : 
*/
GLOBAL T_ACI_BAT_RSLT qBAT_PercentCGREG    (T_ACI_DTI_PRC_PSI *src_infos_psi,
                                            T_BAT_cmd_send    *cmd)
{
  T_ACI_BAT_RSLT ret = ACI_BAT_FAIL;
  T_P_CGREG_STAT  stat;
  USHORT          lac, cid;
 
  TRACE_FUNCTION ("qBAT_PercentCGREG()");
  ret=(T_ACI_BAT_RSLT)qAT_PercentCGREG((T_ACI_CMD_SRC)src_infos_psi->srcId, &stat, &lac, &cid);

  if (ret EQ ACI_BAT_CMPL)
  {
    T_BAT_cmd_response resp;
    T_BAT_res_que_percent_cgreg que_cgreg_buffer;

    resp.ctrl_response = BAT_RES_QUE_PERCENT_CGREG;
    resp.response.ptr_que_percent_cgreg = &que_cgreg_buffer;

    resp.response.ptr_que_percent_cgreg->stat = (T_BAT_percent_cgreg_stat)stat;
    resp.response.ptr_que_percent_cgreg->lac = lac;
    resp.response.ptr_que_percent_cgreg->ci = cid;
    resp.response.ptr_que_percent_cgreg->state = (cmhSM_isContextActive() ? BAT_P_CGREG_STATE_STAT_1 
                                                                        : BAT_P_CGREG_STATE_STAT_0 );
    aci_bat_send(src_infos_psi,&resp);
  }

  return(ret);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : ACI_BAT              |
| STATE   : code                      ROUTINE : tBAT_PlusCGPADDR     |
+--------------------------------------------------------------------+

  PURPOSE : 
*/
GLOBAL T_ACI_BAT_RSLT tBAT_PlusCGPADDR     (T_ACI_DTI_PRC_PSI *src_infos_psi,
                                            T_BAT_cmd_send    *cmd)
{
  T_ACI_BAT_RSLT ret = ACI_BAT_FAIL;
  U8         count = 0;
  U8         cid_array[PDP_CONTEXT_CID_MAX];
  T_BAT_cmd_response resp;
  T_BAT_res_tst_plus_cgpaddr tst_cgpaddr_buffer;
  
  TRACE_FUNCTION ("tBAT_PlusCGPADDR()");

  ret = (T_ACI_BAT_RSLT)tAT_PlusCGPADDR ((T_ACI_CMD_SRC) src_infos_psi->srcId, cid_array );
  resp.ctrl_response = BAT_RES_TST_PLUS_CGPADDR; 
  resp.response.ptr_tst_plus_cgpaddr = &tst_cgpaddr_buffer;
  if (ret EQ ACI_BAT_CMPL)
  {
    while ( cid_array[count] NEQ PDP_CONTEXT_CID_INVALID )
    {
      resp.response.ptr_tst_plus_cgpaddr->cid[count]=(T_BAT_pdp_cid)cid_array[count];
      count ++;
    }
    if(count EQ 0)
      resp.response.ptr_tst_plus_cgpaddr->cid[0] = BAT_PDP_CID_NOT_PRESENT;
    
    resp.response.ptr_tst_plus_cgpaddr->c_cid = count;  
    aci_bat_send(src_infos_psi,&resp);
    return(ACI_BAT_CMPL);
  }
  ACI_ERR_DESC( ACI_ERR_CLASS_Cme, CME_ERR_Unknown);
  return(ACI_BAT_FAIL);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : ACI_BAT              |
| STATE   : code                      ROUTINE : sBAT_PlusCGSMS       |
+--------------------------------------------------------------------+

  PURPOSE : 
*/
GLOBAL T_ACI_BAT_RSLT sBAT_PlusCGSMS       (T_ACI_DTI_PRC_PSI *src_infos_psi,
                                            T_BAT_cmd_send    *cmd)
{
  T_ACI_BAT_RSLT ret = ACI_BAT_FAIL;
  T_CGSMS_SERVICE service = CGSMS_SERVICE_OMITTED;
  
  TRACE_FUNCTION ("sBAT_PlusCGSMS()");
  
  service =(T_CGSMS_SERVICE) cmd->params.ptr_set_plus_cgsms->service;
  ret = (T_ACI_BAT_RSLT)sAT_PlusCGSMS ((T_ACI_CMD_SRC) src_infos_psi->srcId, service );
  return(ret);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : ACI_BAT              |
| STATE   : code                      ROUTINE : qBAT_PlusCGSMS       |
+--------------------------------------------------------------------+

  PURPOSE : 
*/
GLOBAL T_ACI_BAT_RSLT qBAT_PlusCGSMS       (T_ACI_DTI_PRC_PSI *src_infos_psi,
                                            T_BAT_cmd_send    *cmd)
{
  T_ACI_BAT_RSLT ret = ACI_BAT_FAIL;
  T_BAT_cmd_response resp;
  T_BAT_res_que_plus_cgsms que_cgsms_buffer;
  T_CGSMS_SERVICE service = CGSMS_SERVICE_OMITTED;
  
  TRACE_FUNCTION ("qBAT_PlusCGSMS()");
  
  ret = (T_ACI_BAT_RSLT)qAT_PlusCGSMS((T_ACI_CMD_SRC)src_infos_psi->srcId, &service);
  resp.ctrl_response = BAT_RES_QUE_PLUS_CGSMS;
  resp.response.ptr_que_plus_cgsms = &que_cgsms_buffer;
  if(ret EQ ACI_BAT_CMPL)
  {
    resp.response.ptr_que_plus_cgsms->service = (T_BAT_plus_cgsms_service)service;
    aci_bat_send(src_infos_psi,&resp);
    return ret;
  }
  else
  {
    ACI_ERR_DESC( ACI_ERR_CLASS_Cme, CME_ERR_Unknown);
    return (ACI_BAT_FAIL);
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : ACI_BAT              |
| STATE   : code                      ROUTINE : sBAT_PercentCGAATT   |
+--------------------------------------------------------------------+

  PURPOSE : 
*/
GLOBAL T_ACI_BAT_RSLT sBAT_PercentCGAATT   (T_ACI_DTI_PRC_PSI *src_infos_psi,
                                            T_BAT_cmd_send    *cmd)
{
  T_ACI_BAT_RSLT ret = ACI_BAT_FAIL;
  T_BAT_percent_cgaatt_att_m att_m;
  T_BAT_percent_cgaatt_det_m det_m;   
  
  TRACE_FUNCTION ("sBAT_PercentCGAATT()");
  
  att_m = cmd->params.ptr_set_percent_cgaatt->att_m;
  det_m = cmd->params.ptr_set_percent_cgaatt->det_m;  
  ret = (T_ACI_BAT_RSLT)sAT_PercentCGAATT ((T_ACI_CMD_SRC)src_infos_psi->srcId,(T_CGAATT_ATTACH_MODE)att_m, (T_CGAATT_DETACH_MODE)det_m );
  return(ret);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : ACI_BAT              |
| STATE   : code                      ROUTINE : qBAT_PercentCGAATT   |
+--------------------------------------------------------------------+

  PURPOSE : 
*/
GLOBAL T_ACI_BAT_RSLT qBAT_PercentCGAATT   (T_ACI_DTI_PRC_PSI *src_infos_psi,
                                            T_BAT_cmd_send    *cmd)
{
  T_ACI_BAT_RSLT ret = ACI_BAT_FAIL;
  T_BAT_percent_cgaatt_att_m att_m;
  T_BAT_percent_cgaatt_det_m det_m;   
 
  TRACE_FUNCTION ("qBAT_PercentCGAATT()");
  
  ret = (T_ACI_BAT_RSLT)qAT_PercentCGAATT((T_ACI_CMD_SRC)
    src_infos_psi->srcId, (T_CGAATT_ATTACH_MODE *)&att_m, (T_CGAATT_DETACH_MODE *)&det_m);
 
  if(ret EQ ACI_BAT_CMPL)
  {
    T_BAT_cmd_response resp;
    T_BAT_res_que_percent_cgaatt que_cgaatt_buffer;
 
    resp.ctrl_response = BAT_RES_QUE_PERCENT_CGAATT;
    resp.response.ptr_que_percent_cgaatt = &que_cgaatt_buffer;

    resp.response.ptr_que_percent_cgaatt->att_m = att_m;
    resp.response.ptr_que_percent_cgaatt->det_m = det_m;
    aci_bat_send(src_infos_psi,&resp);
  }

  return ret;
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : ACI_BAT              |
| STATE   : code                      ROUTINE : sBAT_PercentSNCNT    |
+--------------------------------------------------------------------+

  PURPOSE : 
*/
GLOBAL T_ACI_BAT_RSLT sBAT_PercentSNCNT    (T_ACI_DTI_PRC_PSI *src_infos_psi,
                                            T_BAT_cmd_send    *cmd)
{
  T_ACI_BAT_RSLT ret = ACI_BAT_FAIL;
  
  TRACE_FUNCTION ("sBAT_PercentSNCNT()");
  ret =(T_ACI_BAT_RSLT) sAT_PercentSNCNT( (T_ACI_CMD_SRC)src_infos_psi->srcId, TRUE );
  return(ret);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : ACI_BAT              |
| STATE   : code                      ROUTINE : qBAT_PercentSNCNT    |
+--------------------------------------------------------------------+

  PURPOSE : 
*/
GLOBAL T_ACI_BAT_RSLT qBAT_PercentSNCNT    (T_ACI_DTI_PRC_PSI *src_infos_psi,
                                            T_BAT_cmd_send    *cmd)
{ 
  T_ACI_BAT_RSLT ret = ACI_BAT_FAIL;
  
  TRACE_FUNCTION ("qBAT_PercentSNCNT()");
  ret = (T_ACI_BAT_RSLT)sAT_PercentSNCNT((T_ACI_CMD_SRC) src_infos_psi->srcId, FALSE );
  return(ret);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : ACI_BAT              |
| STATE   : code                      ROUTINE : sBAT_PercentCGMM     |
+--------------------------------------------------------------------+

  PURPOSE : 
*/
GLOBAL T_ACI_BAT_RSLT sBAT_PercentCGMM     (T_ACI_DTI_PRC_PSI *src_infos_psi,
                                            T_BAT_cmd_send    *cmd)
{
  T_ACI_BAT_RSLT ret = ACI_BAT_FAIL;
  UBYTE cipher_on , tlli_handling ;
  
  TRACE_FUNCTION ("sBAT_PercentCGMM()");
  
  cipher_on = cmd->params.ptr_set_percent_cgmm->cipher_on;
  tlli_handling  = cmd->params.ptr_set_percent_cgmm->tlli_handling;

  ret =(T_ACI_BAT_RSLT) sAT_PercentCGMM((T_ACI_CMD_SRC) src_infos_psi->srcId, cipher_on, tlli_handling );
  return(ret);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : ACI_BAT              |
| STATE   : code                      ROUTINE : sBAT_PercentCGPCO    |
+--------------------------------------------------------------------+

  PURPOSE : 
*/
GLOBAL T_ACI_BAT_RSLT sBAT_PercentCGPCO    (T_ACI_DTI_PRC_PSI *src_infos_psi,
                                            T_BAT_cmd_send    *cmd)
{
  T_ATI_RSLT ret = ATI_FAIL;
  USHORT              cid = 0;
  T_ACI_CGPCO_FORMAT  format;
  CHAR                pco_string[BAT_MAX_CGPCO_PCO_LEN+1];
  T_ACI_CGPCO_MODE    mode;
  
  TRACE_FUNCTION ("sBAT_PercentCGPCO()");

  memset(pco_string,0,sizeof(pco_string));

  mode = (T_ACI_CGPCO_MODE)cmd->params.ptr_set_percent_cgpco->mode;
  format = (T_ACI_CGPCO_FORMAT)cmd->params.ptr_set_percent_cgpco->format;

  if (cmd->params.ptr_set_percent_cgpco->v_pco)
  {
    memcpy(pco_string,cmd->params.ptr_set_percent_cgpco->pco,cmd->params.ptr_set_percent_cgpco->c_pco);
  }
  cid = cmd->params.ptr_set_percent_cgpco->cid;
  if(cid EQ (USHORT)BAT_PDP_CID_NOT_PRESENT)
  {
    cid = (USHORT) INVALID_CID ;
  }
  if (cid > MAX_CID)
  {
    ACI_ERR_DESC( ACI_ERR_CLASS_Cme, CME_ERR_OpNotSupp);
    return ACI_BAT_FAIL ;
  }
    
  switch (mode)
  {
    case CGPCO_MODE_QUERY_PCO:
      {
        ULONG gateway,dns1,dns2;
        char dns1_str[sizeof("255.255.255.255")];
        char dns2_str[sizeof("255.255.255.255")];
        char gatew_str[sizeof("255.255.255.255")];
        USHORT i;

        for (i=1;i<MAX_CID;i++)
        {
          if ((i EQ cid) OR (cid EQ INVALID_CID))
          {
            gateway=dns1=dns2=0;

            /*
            *   Ignore the return value of the ACI function, which is
            *   always AT_CMPL.
            */
            qAT_PercentCGPCO((T_ACI_CMD_SRC)src_infos_psi->srcId,&gateway,&dns1,&dns2,i);

            if (dns1 OR dns2 OR gateway)
            {
              T_BAT_cmd_response resp;
              T_BAT_res_set_percent_cgpco cgpco;
 
              resp.ctrl_response=BAT_RES_SET_PERCENT_CGPCO;
              resp.response.ptr_set_percent_cgpco=&cgpco;

              ipaddr_to_string(dns1, dns1_str);
              ipaddr_to_string(dns2, dns2_str);
              ipaddr_to_string(gateway, gatew_str);

              cgpco.cid=(T_BAT_pdp_cid)i;

              sprintf((char *)cgpco.pco,"%s,%s,%s",dns1_str,dns2_str,gatew_str);
              cgpco.c_pco=strlen((char *)cgpco.pco);

              aci_bat_send(src_infos_psi,&resp);
            }
          }
        }
      }
      return (ACI_BAT_CMPL);

    case CGPCO_MODE_SET_PCO:
    {
      switch (format)
      {
        case CGPCO_FORMAT_HEX:
          ret = setatPercentCGPCO_HEX (src_infos_psi->srcId,cid, (UBYTE*)pco_string, (USHORT)strlen (pco_string));
          break;
        case CGPCO_FORMAT_ASCII:
          ret = setatPercentCGPCO_ASCII (src_infos_psi->srcId, cid, (UBYTE*)pco_string, (USHORT)strlen (pco_string));
          break;
        default:
          ACI_ERR_DESC( ACI_ERR_CLASS_Cme, CME_ERR_OpNotSupp);
          break;
       }
      break;
    }
    default:
      ACI_ERR_DESC( ACI_ERR_CLASS_Cme, CME_ERR_OpNotSupp);
      break;
  }

  /*
  *   Convert the ATI return value into ACI/BAT format.
  */
  switch (ret)
  {
    default:
    case ATI_FAIL:
    case ATI_FAIL_NO_OUTPUT:
      return(ACI_BAT_FAIL);

    case ATI_BUSY:
      return(ACI_BAT_BUSY);

    case ATI_EXCT:
      return(ACI_BAT_EXCT);

    case ATI_CMPL:
    case ATI_CMPL_NO_OUTPUT:
      return(ACI_BAT_CMPL);
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : ACI_BAT              |
| STATE   : code                      ROUTINE : sBAT_PercentCGPPP    |
+--------------------------------------------------------------------+

  PURPOSE : 
*/
GLOBAL T_ACI_BAT_RSLT sBAT_PercentCGPPP    (T_ACI_DTI_PRC_PSI *src_infos_psi,
                                            T_BAT_cmd_send    *cmd)
{
  T_ACI_BAT_RSLT ret = ACI_BAT_FAIL;
  T_ACI_PPP_PROT  protocol;
  
  TRACE_FUNCTION ("sBAT_PercentCGPPP()");
  
  protocol =(T_ACI_PPP_PROT)cmd->params.ptr_set_percent_cgppp->protocol;
  ret = (T_ACI_BAT_RSLT)sAT_PercentCGPPP((T_ACI_CMD_SRC)src_infos_psi->srcId, protocol);
  return(ret);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : ACI_BAT              |
| STATE   : code                      ROUTINE : qBAT_PercentCGPPP    |
+--------------------------------------------------------------------+

  PURPOSE : 
*/
GLOBAL T_ACI_BAT_RSLT qBAT_PercentCGPPP    (T_ACI_DTI_PRC_PSI *src_infos_psi,
                                            T_BAT_cmd_send    *cmd)
{
  T_ACI_BAT_RSLT ret = ACI_BAT_FAIL;
  T_ACI_PPP_PROT  protocol;
  T_BAT_cmd_response resp;
  T_BAT_res_que_percent_cgppp que_cgppp_buffer;

  TRACE_FUNCTION ("qBAT_PercentCGPPP()");
  
  ret =(T_ACI_BAT_RSLT) qAT_PercentCGPPP((T_ACI_CMD_SRC)src_infos_psi->srcId, &protocol);
  resp.ctrl_response = BAT_RES_QUE_PERCENT_CGPPP;
  resp.response.ptr_que_percent_cgppp = &que_cgppp_buffer;
  if( ret EQ ((T_ACI_BAT_RSLT)ATI_FAIL))
  {
    ACI_ERR_DESC( ACI_ERR_CLASS_Cme, CME_ERR_Unknown);
    return (ACI_BAT_FAIL);
  }
  resp.response.ptr_que_percent_cgppp->protocol = (T_BAT_percent_cgppp_protocol)protocol;
  aci_bat_send(src_infos_psi,&resp);
  return(ret);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : ACI_BAT              |
| STATE   : code                      ROUTINE : sBAT_PercentCGCLASS  |
+--------------------------------------------------------------------+

  PURPOSE : 
*/
GLOBAL T_ACI_BAT_RSLT sBAT_PercentCGCLASS  (T_ACI_DTI_PRC_PSI *src_infos_psi,
                                            T_BAT_cmd_send    *cmd)
{
  T_ACI_BAT_RSLT ret = ACI_BAT_FAIL;
  T_PERCENT_CGCLASS m_class = PERCENT_CGCLASS_OMITTED;
  
  TRACE_FUNCTION ("sBAT_PercentCGCLASS()");

  m_class=(T_PERCENT_CGCLASS)cmd->params.ptr_set_percent_cgclass->p_mobile_class;

  if (m_class EQ PERCENT_CGCLASS_OMITTED)
  {
    ACI_ERR_DESC( ACI_ERR_CLASS_Cme, CME_ERR_OpNotAllow);
    return (ACI_BAT_FAIL);
  }
  ret = (T_ACI_BAT_RSLT)sAT_PercentCGCLASS ((T_ACI_CMD_SRC) src_infos_psi->srcId, m_class );
  return(ret);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)            MODULE  : ACI_BAT              |
| STATE   : code                      ROUTINE : qBAT_PercentCGCLASS  |
+--------------------------------------------------------------------+

  PURPOSE : 
*/
GLOBAL T_ACI_BAT_RSLT qBAT_PercentCGCLASS  (T_ACI_DTI_PRC_PSI *src_infos_psi,
                                            T_BAT_cmd_send    *cmd)
{
  T_ACI_BAT_RSLT ret = ACI_BAT_FAIL;
  T_PERCENT_CGCLASS m_class, currentClass;
  T_BAT_cmd_response resp;
  T_BAT_res_que_percent_cgclass que_cgclass_buffer;
  
  TRACE_FUNCTION ("qBAT_PercentCGCLASS()");

  resp.ctrl_response = BAT_RES_QUE_PERCENT_CGCLASS;
  resp.response.ptr_que_percent_cgclass = &que_cgclass_buffer;
  ret =(T_ACI_BAT_RSLT) qAT_PercentCGCLASS((T_ACI_CMD_SRC)src_infos_psi->srcId, &m_class, &currentClass);
  if (ret EQ ACI_BAT_CMPL)
  {
    resp.response.ptr_que_percent_cgclass->p_mobile_class = (T_BAT_p_mobile_class)m_class;
    resp.response.ptr_que_percent_cgclass->current_mobile_class = (T_BAT_p_mobile_class)currentClass;    
    aci_bat_send(src_infos_psi,&resp);
  }
  return(ret);
}

#endif /* GPRS */

