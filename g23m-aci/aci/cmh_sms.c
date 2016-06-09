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
|  Purpose :  This module implements the set fuinctions related to the
|             protocol stack adapter for GPRS session management ( SM ).
+----------------------------------------------------------------------------- 
*/ 

#if defined (GPRS) && defined (DTI)

#ifndef CMH_SMS_C
#define CMH_SMS_C
#endif

#include "aci_all.h"

/*==== INCLUDES ===================================================*/
#include "dti.h"      /* functionality of the dti library */
#include "aci_cmh.h"
#include "ati_cmd.h"
#include "aci_cmd.h"
#include "aci_io.h"

#include "dti_conn_mng.h"
#include "dti_cntrl_mng.h"

#include "gaci.h"
#include "gaci_cmh.h"
#include "psa.h"
#include "psa_sm.h"
#include "psa_gppp.h"
#include "psa_gmm.h"
#include "psa_tcpip.h"

#include "cmh.h"
#include "cmh_sm.h"
#include "cmh_gppp.h"
#include "cmh_gmm.h"
#include "gaci_srcc.h"
#include "aci_mem.h"

#include "cl_inline.h"
#include "phb.h"
#include "wap_aci.h"

/*==== CONSTANTS ==================================================*/

/*==== EXPORT =====================================================*/

/*==== VARIABLES ==================================================*/
EXTERN T_PDP_CONTEXT_INTERNAL *p_pdp_context_list;


/*==== FUNCTIONS ==================================================*/
LOCAL void string_to_dns(CHAR* dns, ULONG *dns_long);
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (8441)         MODULE  : CMH_SMS                  |
| STATE   : finnished             ROUTINE : sAT_PlusCGQREQ           |
+--------------------------------------------------------------------+

PURPOSE : This is the functional counterpart to the +CGQREQ= AT
          command which sets the requested QOS.
*/
GLOBAL T_ACI_RETURN sAT_PlusCGQREQ( T_ACI_CMD_SRC srcId, U8 cid ,T_PS_qos *qos)
{
  T_ACI_RETURN            retCd;          /* holds return code */
  T_PDP_CONTEXT_STATE     context_state;  /* state of context */
  T_PS_qos_r99            temp_qos;       /* Temp. var to hold R99 input parameters for conversion */
  T_PDP_CONTEXT_INTERNAL *p_pdp_context_node = NULL;

  TRACE_FUNCTION ("sAT_PlusCGQREQ()");

/*    Need work to fit into R99 QoS SMNEW 05032001
 *-------------------------------------------------------------------
 * check entity status
 *-------------------------------------------------------------------
 */
  if( smEntStat.curCmd NEQ AT_CMD_NONE )

    return( AT_BUSY );

/*
 *-------------------------------------------------------------------
 * check parameter
 *-------------------------------------------------------------------
 */
  if ( ((cid < PDP_CONTEXT_CID_MIN) OR (cid > PDP_CONTEXT_CID_MAX)) AND (cid NEQ PDP_CONTEXT_CID_OMITTED) )
  {
    ACI_ERR_DESC( ACI_ERR_CLASS_Ext, EXT_ERR_Parameter );
    return AT_FAIL;
  }

  if( qos )
  {
    if( (qos->qos_r97.preced >  3) AND 
        (qos->qos_r97.preced NEQ PDP_CONTEXT_QOS_OMITTED) )
    {
      ACI_ERR_DESC( ACI_ERR_CLASS_Ext, EXT_ERR_Parameter );
      return( AT_FAIL );
    }

    if( (qos->qos_r97.delay >  4) AND
        (qos->qos_r97.delay NEQ PDP_CONTEXT_QOS_OMITTED ))
    {
      ACI_ERR_DESC( ACI_ERR_CLASS_Ext, EXT_ERR_Parameter );
      return( AT_FAIL );
    }
    if( (qos->qos_r97.relclass >  5) AND
        (qos->qos_r97.relclass NEQ PDP_CONTEXT_QOS_OMITTED ))
    {
      ACI_ERR_DESC( ACI_ERR_CLASS_Ext, EXT_ERR_Parameter );
      return( AT_FAIL );
    }
    if( (qos->qos_r97.peak >  9) AND
        (qos->qos_r97.peak NEQ PDP_CONTEXT_QOS_OMITTED ))
    {
      ACI_ERR_DESC( ACI_ERR_CLASS_Ext, EXT_ERR_Parameter );
      return( AT_FAIL );
    }
    if( (qos->qos_r97.mean >  18) AND 
        (qos->qos_r97.mean NEQ 31) AND
        (qos->qos_r97.mean NEQ PDP_CONTEXT_QOS_OMITTED ))
    {
      ACI_ERR_DESC( ACI_ERR_CLASS_Ext, EXT_ERR_Parameter );
      return( AT_FAIL );
    }
  }

/*
 *-------------------------------------------------------------------
 * process parameter
 *-------------------------------------------------------------------
 */
  if (cid EQ PDP_CONTEXT_CID_OMITTED )
 /*
  *   cid omitted: A special form of the set command that is not defined in the Spec.
  *   This set the default value of QoS.
  */
  {
    cmhSM_change_def_QOS( qos, PS_is_R97 );

    retCd = AT_CMPL;
  }
  else
  {
    context_state = pdp_context_get_state_for_cid( cid ); 

    if ( !qos OR ( qos->qos_r97.preced   EQ PDP_CONTEXT_QOS_OMITTED AND
                   qos->qos_r97.delay    EQ PDP_CONTEXT_QOS_OMITTED AND
                   qos->qos_r97.relclass EQ PDP_CONTEXT_QOS_OMITTED AND
                   qos->qos_r97.peak     EQ PDP_CONTEXT_QOS_OMITTED AND
                   qos->qos_r97.mean     EQ PDP_CONTEXT_QOS_OMITTED ) )

    { /* QoS omitted ->  undefine the requested QOS */
      if ( context_state EQ PDP_CONTEXT_STATE_INVALID )
      {
        ACI_ERR_DESC( ACI_ERR_CLASS_Ext, EXT_ERR_Parameter );
        return( AT_FAIL );
      }

      cmhSM_Set_default_QOS( cid );

      retCd = AT_CMPL;
    }
    else
    { /* define the requested QOS */
      if ( context_state EQ PDP_CONTEXT_STATE_INVALID )
      {
        ACI_ERR_DESC( ACI_ERR_CLASS_Ext, EXT_ERR_Parameter );
        return( AT_FAIL );
      }

      p_pdp_context_node = pdp_context_find_node_from_cid( cid );
      if( p_pdp_context_node )
      {
        if( p_pdp_context_node->ctrl_qos EQ PS_is_R99 ) 
        {
          /* Convert existing R99 parameters to R97 parameters */
          memcpy( &temp_qos, &p_pdp_context_node->qos.qos_r99, sizeof(T_PS_qos_r99) );
          if( !cl_qos_convert_r99_to_r97( &temp_qos,  &p_pdp_context_node->qos.qos_r97) )
            return( AT_FAIL );
        } 

        p_pdp_context_node->ctrl_qos = PS_is_R97;
            
        if( qos->qos_r97.preced NEQ PDP_CONTEXT_QOS_OMITTED )
          p_pdp_context_node->qos.qos_r97.preced = qos->qos_r97.preced;
        if( qos->qos_r97.delay NEQ PDP_CONTEXT_QOS_OMITTED )
          p_pdp_context_node->qos.qos_r97.delay = qos->qos_r97.delay;
        if( qos->qos_r97.relclass NEQ PDP_CONTEXT_QOS_OMITTED )
          p_pdp_context_node->qos.qos_r97.relclass = qos->qos_r97.relclass;
        if( qos->qos_r97.peak NEQ  PDP_CONTEXT_QOS_OMITTED )
          p_pdp_context_node->qos.qos_r97.peak = qos->qos_r97.peak;
        if( qos->qos_r97.mean NEQ  PDP_CONTEXT_QOS_OMITTED )
          p_pdp_context_node->qos.qos_r97.mean = qos->qos_r97.mean;

        retCd = AT_CMPL;
      }
      else
      {
        retCd = AT_FAIL;
      }

    }

  }

  return retCd;
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (8441)         MODULE  : CMH_SMS                  |
| STATE   : finnished             ROUTINE : sAT_PlusCGQMIN           |
+--------------------------------------------------------------------+

PURPOSE : This is the functional counterpart to the +CGQMIN= AT
          command which sets the minimum acceptable QOS.
*/
GLOBAL T_ACI_RETURN sAT_PlusCGQMIN( T_ACI_CMD_SRC srcId, U8 cid ,T_PS_min_qos *qos)
{
  T_ACI_RETURN            retCd;         /* holds return code */
  T_PDP_CONTEXT_STATE     context_state; /* state of context */
  T_PS_qos_r99            temp_qos;      /* Temp. var to hold R99 input parameters for conversion */
  T_PDP_CONTEXT_INTERNAL *p_pdp_context_node = NULL;

  TRACE_FUNCTION ("sAT_PlusCGQMIN()");

/*
 *-------------------------------------------------------------------
 * check entity status
 *-------------------------------------------------------------------
 */
  if( smEntStat.curCmd NEQ AT_CMD_NONE )

    return( AT_BUSY );

/*
 *-------------------------------------------------------------------
 * check parameter
 *-------------------------------------------------------------------
 */
  if ( ((cid < PDP_CONTEXT_CID_MIN) OR (cid > PDP_CONTEXT_CID_MAX)) AND (cid NEQ PDP_CONTEXT_CID_OMITTED) )
  {
    ACI_ERR_DESC( ACI_ERR_CLASS_Ext, EXT_ERR_Parameter );
    return AT_FAIL;
  }

  if ( qos )
  {
    if( (qos->qos_r97.preced >  3) AND
        (qos->qos_r97.preced NEQ PDP_CONTEXT_QOS_OMITTED ))
    {
      ACI_ERR_DESC( ACI_ERR_CLASS_Ext, EXT_ERR_Parameter );
      return( AT_FAIL );
    }

    if( (qos->qos_r97.delay >  4) AND
        (qos->qos_r97.delay NEQ PDP_CONTEXT_QOS_OMITTED ))
    {
      ACI_ERR_DESC( ACI_ERR_CLASS_Ext, EXT_ERR_Parameter );
      return( AT_FAIL );
    }
    if( (qos->qos_r97.relclass >  5) AND
        (qos->qos_r97.relclass NEQ PDP_CONTEXT_QOS_OMITTED ))
    {
      ACI_ERR_DESC( ACI_ERR_CLASS_Ext, EXT_ERR_Parameter );
      return( AT_FAIL );
    }
    if( (qos->qos_r97.peak >  9) AND 
        (qos->qos_r97.peak NEQ PDP_CONTEXT_QOS_OMITTED ))
    {
      ACI_ERR_DESC( ACI_ERR_CLASS_Ext, EXT_ERR_Parameter );
      return( AT_FAIL );
    }
    if( (qos->qos_r97.mean >  18) AND 
        (qos->qos_r97.mean NEQ 31) AND
        (qos->qos_r97.mean NEQ PDP_CONTEXT_QOS_OMITTED ))
    {
      ACI_ERR_DESC( ACI_ERR_CLASS_Ext, EXT_ERR_Parameter );
      return( AT_FAIL );
    }
  }

/*
*-------------------------------------------------------------------
* process parameter
*-------------------------------------------------------------------
*/
  if( cid EQ PDP_CONTEXT_CID_OMITTED )
 /*
  *   cid omitted: A special form of the set command that is not defined in the Spec.
  *   This set the default value of QoS.
  */
  {
    cmhSM_change_def_QOS_min( qos, (T_PS_ctrl_min_qos)PS_is_R97 );

    retCd = (T_ACI_RETURN)AT_CMPL;
  }
  else
  {
    context_state = pdp_context_get_state_for_cid( cid );

    if ( !qos OR ( qos->qos_r97.preced   EQ PDP_CONTEXT_QOS_OMITTED AND
                   qos->qos_r97.delay    EQ PDP_CONTEXT_QOS_OMITTED AND
                   qos->qos_r97.relclass EQ PDP_CONTEXT_QOS_OMITTED AND
                   qos->qos_r97.peak     EQ PDP_CONTEXT_QOS_OMITTED AND
                   qos->qos_r97.mean     EQ PDP_CONTEXT_QOS_OMITTED ) )
    { /* QoS omitted ->  undefine the requested QOS */
    
      if ( context_state EQ PDP_CONTEXT_STATE_INVALID )
      {
        ACI_ERR_DESC( ACI_ERR_CLASS_Ext, EXT_ERR_Parameter );
        return( AT_FAIL );
      }

      cmhSM_Set_default_QOS_min( cid );

      retCd = AT_CMPL;
    }
    else
    { /* define the requested QOS */
    
      if ( context_state EQ PDP_CONTEXT_STATE_INVALID )
      {
        ACI_ERR_DESC( ACI_ERR_CLASS_Ext, EXT_ERR_Parameter );
        return( AT_FAIL );
      }

      p_pdp_context_node = pdp_context_find_node_from_cid( cid );

      if( p_pdp_context_node )
      {

        if( p_pdp_context_node->ctrl_min_qos EQ (T_PS_ctrl_min_qos)PS_is_R99 ) 
        {
          /* Convert existing R99 parameters to R97 parameters */
          memcpy( &temp_qos, &p_pdp_context_node->min_qos.qos_r99, sizeof(T_PS_qos_r99) );
          if( !cl_qos_convert_r99_to_r97( &temp_qos, &p_pdp_context_node->min_qos.qos_r97) )
            return( AT_FAIL );
        } 

        p_pdp_context_node->ctrl_min_qos = (T_PS_ctrl_min_qos)PS_is_R97;
      
        /* Set input values for context or keep the previous/converted values if omitted */
        if( qos->qos_r97.preced EQ PDP_CONTEXT_QOS_OMITTED )
          p_pdp_context_node->min_qos.qos_r97.preced = PS_PRECED_SUB;
        else 
          p_pdp_context_node->min_qos.qos_r97.preced = qos->qos_r97.preced;
        
        if( qos->qos_r97.delay EQ PDP_CONTEXT_QOS_OMITTED ) 
          p_pdp_context_node->min_qos.qos_r97.delay = PS_DELAY_SUB;
        else
          p_pdp_context_node->min_qos.qos_r97.delay = qos->qos_r97.delay;
        
        if( qos->qos_r97.relclass EQ PDP_CONTEXT_QOS_OMITTED ) 
          p_pdp_context_node->min_qos.qos_r97.relclass = PS_RELCLASS_SUB;
        else 
          p_pdp_context_node->min_qos.qos_r97.relclass = qos->qos_r97.relclass;
        
        if( qos->qos_r97.peak EQ PDP_CONTEXT_QOS_OMITTED ) 
          p_pdp_context_node->min_qos.qos_r97.peak = PS_PEAK_SUB;
        else 
          p_pdp_context_node->min_qos.qos_r97.peak = qos->qos_r97.peak;
        
        if( qos->qos_r97.mean EQ PDP_CONTEXT_QOS_OMITTED ) 
          p_pdp_context_node->min_qos.qos_r97.mean = PS_MEAN_SUB;
        else 
          p_pdp_context_node->min_qos.qos_r97.mean = qos->qos_r97.mean;

        retCd = AT_CMPL;
      }
      else
      {
        retCd = AT_FAIL;
      }
      
    }

  }

  return retCd;
}

#ifdef REL99
/*
+--------------------------------------------------------------------+
| PROJECT : UMTS                  MODULE  : CMH_SMS                  |
| STATE   : devellopment          ROUTINE : sAT_PlusCGEQREQ          |
+--------------------------------------------------------------------+

PURPOSE : This is the functional counterpart to the +CGEQREQ= AT
          command which sets the 3G requested QOS parameters.
*/
GLOBAL T_ACI_RETURN sAT_PlusCGEQREQ( T_ACI_CMD_SRC srcId, U8 cid, T_PS_qos *qos )
{
  T_ACI_RETURN            retCd;        /* holds return code */
  T_PDP_CONTEXT_STATE     c_state;      /* state of context  */
  BOOL                    outOfRange;   /* TRUE if one or more parameters are out of range */
  U16                     tempComp;     /* Used for range check of SDU error ratio and bit error ratio */
  T_PS_qos_r97            temp_qos;     /* Temp. var to hold R97 input parameters for conversion */
  T_PDP_CONTEXT_INTERNAL *p_pdp_context_node = NULL;

  TRACE_FUNCTION( "sAT_PlusCGEQREQ()" );

  outOfRange = FALSE;

  if( smEntStat.curCmd NEQ AT_CMD_NONE )
    return( AT_BUSY );

/*
 *-------------------------------------------------------------------
 * Parameter range check.
 *-------------------------------------------------------------------
 */
  if ( ((cid < PDP_CONTEXT_CID_MIN) OR (cid > PDP_CONTEXT_CID_MAX)) AND (cid NEQ PDP_CONTEXT_CID_OMITTED) )
  {
    outOfRange = TRUE;
  }

  if ( qos AND (outOfRange EQ FALSE) )
  {
    if( (qos->qos_r99.tc > 4 ) AND  
        (qos->qos_r99.tc NEQ QOS_R99_TC_OMITTED) )                     /* 4 = subscribed value */
      outOfRange = TRUE;

    if( (qos->qos_r99.max_rate_ul > 2048 ) AND
        (qos->qos_r99.max_rate_ul NEQ QOS_R99_MAX_BR_UL_OMITTED) )     /* 0 = subscribed value */
      outOfRange = TRUE;
    
    if( (qos->qos_r99.max_rate_dl > 2048 ) AND
        (qos->qos_r99.max_rate_dl NEQ QOS_R99_MAX_BR_DL_OMITTED ))     /* 0 = subscribed value */
      outOfRange = TRUE;
    
    if( (qos->qos_r99.guar_br_ul > 2048 ) AND
        (qos->qos_r99.guar_br_ul NEQ QOS_R99_GUAR_BR_UL_OMITTED ))     /* 0 = subscribed value */
      outOfRange = TRUE;
    
    if( (qos->qos_r99.guar_br_dl > 2048 ) AND
        (qos->qos_r99.guar_br_dl NEQ QOS_R99_GUAR_BR_DL_OMITTED ))     /* 0 = subscribed value */
      outOfRange = TRUE;    
    
    if( (qos->qos_r99.order > 2 ) AND
        (qos->qos_r99.order NEQ QOS_R99_ORDER_OMITTED ))               /* 2 = subscribed value */
      outOfRange = TRUE;
    
    if( (qos->qos_r99.xfer_delay > 65534 ) AND
        (qos->qos_r99.xfer_delay NEQ QOS_R99_XFER_DELAY_OMITTED ))     /* 0 = subscribed value */
      outOfRange = TRUE;
    
    if( (qos->qos_r99.del_err_sdu > 3 ) AND
        (qos->qos_r99.del_err_sdu NEQ QOS_R99_DEL_ERR_SDU_OMITTED ))   /* 3 = subscribed value */
      outOfRange = TRUE;
    
    if( (qos->qos_r99.handling_pri > 3 ) AND
        (qos->qos_r99.handling_pri NEQ QOS_R99_HANDLING_PRIO_OMITTED )) /* 0 = subscribed value */
      outOfRange = TRUE;

    if( cid NEQ PDP_CONTEXT_CID_OMITTED )
    {
      p_pdp_context_node = pdp_context_find_node_from_cid( cid );
      if( p_pdp_context_node )
      {
        if( !strcmp( p_pdp_context_node->attributes.pdp_type, "PPP") )
        {
          if( (qos->qos_r99.max_sdu > 1502 ) AND
              (qos->qos_r99.max_sdu NEQ QOS_R99_MAX_SDU_OMITTED ))       /* 0 = subscribed value, 1502 for PPP */
            outOfRange = TRUE;
        }
        else
        {
          if( (qos->qos_r99.max_sdu > 1500 ) AND
              (qos->qos_r99.max_sdu NEQ QOS_R99_MAX_SDU_OMITTED ))       /* 0 = subscribed value */
            outOfRange = TRUE;
        }
      }
      else
      {
        TRACE_ERROR( "ERROR: PDP context not found, in function sAT_PlusCGEQREQ" );
        outOfRange = TRUE;
      }
    }
    else
    { 
      /* It is decided that the max_sdu for the default qos is 1500 and NOT 1502 since 1500 is valid for both ppp and ip */
      if( (qos->qos_r99.max_sdu > 1500 ) AND
          (qos->qos_r99.max_sdu NEQ QOS_R99_MAX_SDU_OMITTED ))       /* 0 = subscribed value, 1502 for PPP */
         outOfRange = TRUE;
    }
    
    /* to simplify the parameter range check the to bytes are joined in to a U16. */

    tempComp = (U16) qos->qos_r99.sdu_err_ratio.ratio_mant;
    tempComp = tempComp << 8;
    tempComp = tempComp | qos->qos_r99.sdu_err_ratio.ratio_exp;

    if( tempComp NEQ 0 AND 
      tempComp NEQ ((QOS_R99_RATIO_MANT_OMITTED<<8) + QOS_R99_RATIO_EXP_OMITTED) ) /* Check parameter range if "sdu_err_ratio" NEQ subscribed value or omitted. */
    {
      switch( qos->qos_r99.tc )
      {
        case PS_TC_CONV: /* Conversational class */
          if( tempComp NEQ 0x0102 AND 
              tempComp NEQ 0x0703 AND 
              tempComp NEQ 0x0103 AND 
              tempComp NEQ 0x0104 AND 
              tempComp NEQ 0x0105 )   /* 0x0102 = 1 * 10^-2 */
            outOfRange = TRUE;
          break;
          
        case PS_TC_STREAM: /* Streaming class */
          if( tempComp NEQ 0x0101 AND 
              tempComp NEQ 0x0102 AND 
              tempComp NEQ 0x0703 AND 
              tempComp NEQ 0x0103 AND 
              tempComp NEQ 0x0104 AND 
              tempComp NEQ 0x0105 )
            outOfRange = TRUE;
          break;
          
        case PS_TC_INTER: /* Interactive class */
        case PS_TC_BG:    /* Background class */
          if( tempComp NEQ 0x0103 AND 
              tempComp NEQ 0x0104 AND 
              tempComp NEQ 0x0106 )
            outOfRange = TRUE;
          break;
          
        case PS_TC_SUB: /* Subscribed value */
        case QOS_R99_TC_OMITTED:
          if( tempComp NEQ 0x0101 AND 
              tempComp NEQ 0x0102 AND 
              tempComp NEQ 0x0703 AND 
              tempComp NEQ 0x0103 AND 
              tempComp NEQ 0x0104 AND 
              tempComp NEQ 0x0105 AND 
              tempComp NEQ 0x0106 )
            outOfRange = TRUE;
          break;
          
        default:
          outOfRange = TRUE;
          break;
      }
    }

    tempComp = (U16) qos->qos_r99.ber.ratio_mant;
    tempComp = tempComp << 8;
    tempComp = tempComp | qos->qos_r99.ber.ratio_exp;

    if( tempComp NEQ 0 AND 
    	tempComp NEQ ((QOS_R99_RATIO_MANT_OMITTED<<8) + QOS_R99_RATIO_EXP_OMITTED) ) /* Check parameter range if "ber" NEQ subscribed value or omitted. */
    { 
      switch( qos->qos_r99.tc )
      {
        case PS_TC_CONV: /* Conversational class */
          if( tempComp NEQ 0x0502 AND 
              tempComp NEQ 0x0102 AND 
              tempComp NEQ 0x0503 AND 
              tempComp NEQ 0x0103 AND 
              tempComp NEQ 0x0104 AND 
              tempComp NEQ 0x0106 )
            outOfRange = TRUE;
          break;
          
        case PS_TC_STREAM: /* Streaming class */
          if( tempComp NEQ 0x0502 AND 
              tempComp NEQ 0x0102 AND 
              tempComp NEQ 0x0503 AND 
              tempComp NEQ 0x0103 AND 
              tempComp NEQ 0x0104 AND 
              tempComp NEQ 0x0105 AND 
              tempComp NEQ 0x0106 )
            outOfRange = TRUE;
          break;

        case PS_TC_INTER: /* Interactive class */
        case PS_TC_BG: /* Background class */
          if( tempComp NEQ 0x0403 AND 
              tempComp NEQ 0x0105 AND 
              tempComp NEQ 0x0608 )
            outOfRange = TRUE;
          break;

        case PS_TC_SUB: /* Subscribed value */
        case QOS_R99_TC_OMITTED:
          if( tempComp NEQ 0x0502 AND
              tempComp NEQ 0x0102 AND
              tempComp NEQ 0x0503 AND
              tempComp NEQ 0x0403 AND
              tempComp NEQ 0x0103 AND
              tempComp NEQ 0x0104 AND
              tempComp NEQ 0x0105 AND
              tempComp NEQ 0x0106 )
            outOfRange = TRUE;
          break;
          
        default:
          outOfRange = TRUE;
          break;
      }
    }
  }

  if( outOfRange EQ TRUE )
  {
    ACI_ERR_DESC( ACI_ERR_CLASS_Ext, EXT_ERR_Parameter );
    return( AT_FAIL );
  }

/*
 *-------------------------------------------------------------------
 * process parameter
 *-------------------------------------------------------------------
 */
  if(cid EQ PDP_CONTEXT_CID_OMITTED )
 /*
  *   cid omitted: A special form of the set command that is not defined in the Spec.
  *   This set the default value of QoS.
  */
  {
    cmhSM_change_def_QOS( qos, PS_is_R99 );

    retCd = AT_CMPL;
  }
  else
  {
/*
 *    Check that a context is defined before changing the QoS.
 */
    p_pdp_context_node = pdp_context_find_node_from_cid( cid );
    if( ! p_pdp_context_node )
    {
      return AT_FAIL;
    }

    c_state = get_state_over_cid( cid );
    switch (c_state)
    {
      case PDP_CONTEXT_STATE_INVALID:
        /* Wrong context state (not defined): Reject command */
        ACI_ERR_DESC( ACI_ERR_CLASS_Ext, EXT_ERR_Parameter );
        return( AT_FAIL );
      default: ;
        /* Context is defined: Continue */
    }

    if( !qos OR ( qos->qos_r99.tc           EQ QOS_R99_TC_OMITTED            AND
                  qos->qos_r99.order        EQ QOS_R99_ORDER_OMITTED         AND
                  qos->qos_r99.del_err_sdu  EQ QOS_R99_DEL_ERR_SDU_OMITTED   AND
                  qos->qos_r99.max_sdu      EQ QOS_R99_MAX_SDU_OMITTED       AND
                  qos->qos_r99.max_rate_ul  EQ QOS_R99_MAX_BR_UL_OMITTED     AND
                  qos->qos_r99.max_rate_dl  EQ QOS_R99_MAX_BR_DL_OMITTED     AND
                  qos->qos_r99.xfer_delay   EQ QOS_R99_XFER_DELAY_OMITTED    AND
                  qos->qos_r99.handling_pri EQ QOS_R99_HANDLING_PRIO_OMITTED AND
                  qos->qos_r99.guar_br_ul   EQ QOS_R99_GUAR_BR_UL_OMITTED    AND
                  qos->qos_r99.guar_br_dl   EQ QOS_R99_GUAR_BR_DL_OMITTED    AND
                  qos->qos_r99.ber.ratio_mant EQ QOS_R99_RATIO_MANT_OMITTED  AND
                  qos->qos_r99.ber.ratio_exp  EQ QOS_R99_RATIO_EXP_OMITTED   AND
                  qos->qos_r99.sdu_err_ratio.ratio_mant EQ QOS_R99_RATIO_MANT_OMITTED AND
                  qos->qos_r99.sdu_err_ratio.ratio_exp  EQ QOS_R99_RATIO_EXP_OMITTED ) )

    {
/*
 *   QoS omitted ->  undefine the requested QOS
 */
      cmhSM_Set_default_QOS( cid );
      
      /* If the default parameters is in R97 format, it must be converted to R99 */
      if( p_pdp_context_node->ctrl_qos EQ PS_is_R97)
      {
        memcpy( &temp_qos, &p_pdp_context_node->qos.qos_r97, sizeof(T_PS_qos_r97) );
        cl_qos_convert_r97_to_r99( &temp_qos, &p_pdp_context_node->qos.qos_r99 );
        p_pdp_context_node->ctrl_qos = PS_is_R99;
      }

      retCd = AT_CMPL;
    }
    else
    {
/*
 *    Define the requested QOS
 */
      p_pdp_context_node = pdp_context_find_node_from_cid( cid );
      if( p_pdp_context_node )
      {
      
        if( p_pdp_context_node->ctrl_qos EQ PS_is_R97 ) 
        {
          /* Convert existing R97 parameters to R99 parameters */
          p_pdp_context_node->ctrl_qos = PS_is_R99;
          memcpy( &temp_qos, &p_pdp_context_node->qos.qos_r97, sizeof(T_PS_qos_r97) );
          if( !cl_qos_convert_r97_to_r99( &temp_qos, &p_pdp_context_node->qos.qos_r99) )
            return( AT_FAIL );
        } 
        else
          p_pdp_context_node->ctrl_qos = PS_is_R99; //FDU - 13082003

    
        /* Set the new values for context or keep the old values if omitted */
        if( qos->qos_r99.tc NEQ QOS_R99_TC_OMITTED )
          p_pdp_context_node->qos.qos_r99.tc = qos->qos_r99.tc;

        if( qos->qos_r99.order NEQ QOS_R99_ORDER_OMITTED )
          p_pdp_context_node->qos.qos_r99.order = qos->qos_r99.order;
      
        if( qos->qos_r99.del_err_sdu NEQ QOS_R99_DEL_ERR_SDU_OMITTED )
          p_pdp_context_node->qos.qos_r99.del_err_sdu = qos->qos_r99.del_err_sdu;
      
        if( qos->qos_r99.max_sdu NEQ QOS_R99_MAX_SDU_OMITTED )
          p_pdp_context_node->qos.qos_r99.max_sdu = qos->qos_r99.max_sdu;
      
        if( qos->qos_r99.max_rate_ul NEQ QOS_R99_MAX_BR_UL_OMITTED )
          p_pdp_context_node->qos.qos_r99.max_rate_ul = qos->qos_r99.max_rate_ul;
      
        if( qos->qos_r99.max_rate_dl NEQ QOS_R99_MAX_BR_DL_OMITTED )
          p_pdp_context_node->qos.qos_r99.max_rate_dl = qos->qos_r99.max_rate_dl;
      
        if( qos->qos_r99.xfer_delay NEQ QOS_R99_XFER_DELAY_OMITTED )
          p_pdp_context_node->qos.qos_r99.xfer_delay = qos->qos_r99.xfer_delay;
      
        if( qos->qos_r99.handling_pri NEQ QOS_R99_HANDLING_PRIO_OMITTED )
          p_pdp_context_node->qos.qos_r99.handling_pri = qos->qos_r99.handling_pri;
      
        if( qos->qos_r99.guar_br_ul NEQ QOS_R99_GUAR_BR_UL_OMITTED )
          p_pdp_context_node->qos.qos_r99.guar_br_ul = qos->qos_r99.guar_br_ul;
      
        if( qos->qos_r99.guar_br_dl NEQ QOS_R99_GUAR_BR_DL_OMITTED )
          p_pdp_context_node->qos.qos_r99.guar_br_dl = qos->qos_r99.guar_br_dl;
      
        if( qos->qos_r99.ber.ratio_mant NEQ QOS_R99_RATIO_MANT_OMITTED )
          p_pdp_context_node->qos.qos_r99.ber.ratio_mant = qos->qos_r99.ber.ratio_mant;
      
        if( qos->qos_r99.ber.ratio_exp NEQ QOS_R99_RATIO_EXP_OMITTED )
          p_pdp_context_node->qos.qos_r99.ber.ratio_exp = qos->qos_r99.ber.ratio_exp;
      
        if( qos->qos_r99.sdu_err_ratio.ratio_mant NEQ QOS_R99_RATIO_MANT_OMITTED )
          p_pdp_context_node->qos.qos_r99.sdu_err_ratio.ratio_mant = qos->qos_r99.sdu_err_ratio.ratio_mant;
      
        if( qos->qos_r99.sdu_err_ratio.ratio_exp NEQ QOS_R99_RATIO_EXP_OMITTED )
          p_pdp_context_node->qos.qos_r99.sdu_err_ratio.ratio_exp = qos->qos_r99.sdu_err_ratio.ratio_exp;
      }
      else
      {
        return( AT_FAIL );
      }
    }

    retCd = AT_CMPL;

  }

  return retCd;
}


/*
+--------------------------------------------------------------------+
| PROJECT : UMTS                  MODULE  : CMH_SMS                  |
| STATE   : devellopment          ROUTINE : sAT_PlusCGEQMIN          |
+--------------------------------------------------------------------+

PURPOSE : This is the functional counterpart to the +CGEQMIN= AT
          command which sets the 3G requested QOS min. parameters.
*/
GLOBAL T_ACI_RETURN sAT_PlusCGEQMIN( T_ACI_CMD_SRC srcId, U8 cid, T_PS_qos *qos )
{
  T_ACI_RETURN            retCd;        /* holds return code */
  T_PDP_CONTEXT_STATE     c_state;      /* state of context  */
  BOOL                    outOfRange;   /* TRUE if one or more parameters are out of range */
  U16                     tempComp;     /* Used for range check of SDU error ratio and bit error ratio */
  T_PS_qos_r97            temp_qos;     /* Temp. var to hold R97 input parameters for conversion */  
  T_PDP_CONTEXT_INTERNAL *p_pdp_context_node = NULL;

  TRACE_FUNCTION( "sAT_PlusCGEQMIN()" );

  outOfRange = FALSE;

  if( smEntStat.curCmd NEQ AT_CMD_NONE )
    return( AT_BUSY );

/*
 *-------------------------------------------------------------------
 * check parameter
 *-------------------------------------------------------------------
 */
  if ( ((cid < PDP_CONTEXT_CID_MIN) OR (cid > PDP_CONTEXT_CID_MAX)) AND (cid NEQ PDP_CONTEXT_CID_OMITTED) )
  {
    outOfRange = TRUE;
  }
  if ( qos EQ NULL)
  {
    return( AT_FAIL );
  }
  if ( qos AND (outOfRange EQ FALSE) )
  {
    if( (qos->qos_r99.tc >  3) AND
        (qos->qos_r99.tc NEQ QOS_R99_TC_OMITTED ))
      outOfRange = TRUE;

    if( (qos->qos_r99.max_rate_ul <  1     OR
         qos->qos_r99.max_rate_ul >  2048) AND
         qos->qos_r99.max_rate_ul NEQ QOS_R99_MAX_BR_UL_OMITTED )
      outOfRange = TRUE;
    
    if( (qos->qos_r99.max_rate_dl <  1     OR 
         qos->qos_r99.max_rate_dl >  2048) AND
         qos->qos_r99.max_rate_dl NEQ QOS_R99_MAX_BR_DL_OMITTED )
      outOfRange = TRUE;
    
    if( (qos->qos_r99.guar_br_ul <  1     OR 
         qos->qos_r99.guar_br_ul >  2048) AND
         qos->qos_r99.guar_br_ul NEQ QOS_R99_GUAR_BR_UL_OMITTED )
      outOfRange = TRUE;
    
    if( (qos->qos_r99.guar_br_dl < 1     OR 
         qos->qos_r99.guar_br_dl > 2048) AND
         qos->qos_r99.guar_br_dl NEQ QOS_R99_GUAR_BR_DL_OMITTED )
      outOfRange = TRUE;
    
    if( (qos->qos_r99.order >  1) AND
        (qos->qos_r99.order NEQ QOS_R99_ORDER_OMITTED ))
      outOfRange = TRUE;
    
    if( (qos->qos_r99.xfer_delay >  65534) AND
        (qos->qos_r99.xfer_delay NEQ QOS_R99_XFER_DELAY_OMITTED ))
      outOfRange = TRUE;
    
    if( (qos->qos_r99.del_err_sdu >  2) AND
        (qos->qos_r99.del_err_sdu NEQ QOS_R99_DEL_ERR_SDU_OMITTED ))
      outOfRange = TRUE;
    
    if( (qos->qos_r99.handling_pri <  1  OR 
         qos->qos_r99.handling_pri >  3) AND
         qos->qos_r99.handling_pri NEQ QOS_R99_HANDLING_PRIO_OMITTED )
      outOfRange = TRUE;

    if( cid NEQ PDP_CONTEXT_CID_OMITTED )
    {
      p_pdp_context_node = pdp_context_find_node_from_cid( cid );
      if( p_pdp_context_node )
      {
        if( !strcmp( p_pdp_context_node->attributes.pdp_type, "PPP") )
        {
          if( (qos->qos_r99.max_sdu >  1502) AND
              (qos->qos_r99.max_sdu NEQ QOS_R99_MAX_SDU_OMITTED ))     /* 0 = subscribed value, 1502 for PPP */
            outOfRange = TRUE;
        }
        else
        {
          if( (qos->qos_r99.max_sdu >  1500) AND
              (qos->qos_r99.max_sdu NEQ QOS_R99_MAX_SDU_OMITTED ))     /* 0 = subscribed value */
            outOfRange = TRUE;
        }   
      }
      else
      {
        TRACE_ERROR( "ERROR: PDP context not found, in function sAT_PlusCGEQMIN" );
        outOfRange = TRUE;
      }
    }
    else
    {
      if( (qos->qos_r99.max_sdu >  1500) AND
          (qos->qos_r99.max_sdu NEQ QOS_R99_MAX_SDU_OMITTED ))     /* 0 = subscribed value */
        outOfRange = TRUE;
    }
    
    /* to simplify the parameter range check the to bytes are joined in to a U16. */

    tempComp = (U16) qos->qos_r99.sdu_err_ratio.ratio_mant;
    tempComp = tempComp << 8;
    tempComp = tempComp | qos->qos_r99.sdu_err_ratio.ratio_exp;
    
    if( tempComp NEQ 0 AND 
      tempComp NEQ ((QOS_R99_RATIO_MANT_OMITTED<<8) + QOS_R99_RATIO_EXP_OMITTED) ) /* Check parameter range if "sdu_err_ratio" NEQ subscribed value or omitted. */
    {
      switch( qos->qos_r99.tc )
      {
        case PS_TC_CONV: /* Conversational class */
          if( tempComp NEQ 0x0102 AND 
              tempComp NEQ 0x0703 AND 
              tempComp NEQ 0x0103 AND 
              tempComp NEQ 0x0104 AND 
              tempComp NEQ 0x0105 )   /* 0x0102 = 1 * 10^-2 */
            outOfRange = TRUE;
          break;
          
        case PS_TC_STREAM: /* Streaming class */
          if( tempComp NEQ 0x0101 AND 
              tempComp NEQ 0x0102 AND 
              tempComp NEQ 0x0703 AND 
              tempComp NEQ 0x0103 AND 
              tempComp NEQ 0x0104 AND 
              tempComp NEQ 0x0105 )
            outOfRange = TRUE;
          break;

        case PS_TC_INTER: /* interactive class */
          if( tempComp NEQ 0x0103 AND 
              tempComp NEQ 0x0104 AND 
              tempComp NEQ 0x0106 )
            outOfRange = TRUE;
          break;
          
        case PS_TC_BG: /* Background class */
          if( tempComp NEQ 0x0103 AND 
              tempComp NEQ 0x0104 AND 
              tempComp NEQ 0x0106 )
            outOfRange = TRUE;
          break;
          
        case PS_TC_SUB: /* Subscribed value */
        case QOS_R99_TC_OMITTED:
          if( tempComp NEQ 0x0101 AND 
              tempComp NEQ 0x0102 AND 
              tempComp NEQ 0x0703 AND 
              tempComp NEQ 0x0103 AND 
              tempComp NEQ 0x0104 AND 
              tempComp NEQ 0x0105 AND 
              tempComp NEQ 0x0106 )
            outOfRange = TRUE;
          break;
          
        default:
          outOfRange = TRUE;
          break;
      }
    }

    tempComp = (U16) qos->qos_r99.ber.ratio_mant;
    tempComp = tempComp << 8;
    tempComp = tempComp | qos->qos_r99.ber.ratio_exp;
    
    if( tempComp NEQ 0 AND
      tempComp NEQ ((QOS_R99_RATIO_MANT_OMITTED<<8) + QOS_R99_RATIO_EXP_OMITTED) ) /* Check parameter range if "ber" NEQ subscribed value or omitted. */
    { 
      switch( qos->qos_r99.tc )
      {
        case PS_TC_CONV: /* Conversational class */
          if( tempComp NEQ 0x0502 AND 
              tempComp NEQ 0x0102 AND 
              tempComp NEQ 0x0503 AND 
              tempComp NEQ 0x0103 AND 
              tempComp NEQ 0x0104 AND 
              tempComp NEQ 0x0106 )
            outOfRange = TRUE;
          break;
          
        case PS_TC_STREAM: /* Streaming class */
          if( tempComp NEQ 0x0502 AND 
              tempComp NEQ 0x0102 AND 
              tempComp NEQ 0x0503 AND 
              tempComp NEQ 0x0103 AND 
              tempComp NEQ 0x0104 AND 
              tempComp NEQ 0x0105 AND 
              tempComp NEQ 0x0106 )
            outOfRange = TRUE;
          break;
          
        case PS_TC_INTER: /* Interactive class */
          if( tempComp NEQ 0x0403 AND 
              tempComp NEQ 0x0105 AND 
              tempComp NEQ 0x0608 )
            outOfRange = TRUE;
          break;
          
        case PS_TC_BG: /* Background class */
          if( tempComp NEQ 0x0403 AND 
              tempComp NEQ 0x0105 AND 
              tempComp NEQ 0x0608 )
            outOfRange = TRUE;
          break;
          
        case PS_TC_SUB: /* Subscribed value */
        case QOS_R99_TC_OMITTED:
          if( tempComp NEQ 0x0502 AND 
              tempComp NEQ 0x0102 AND 
              tempComp NEQ 0x0503 AND 
              tempComp NEQ 0x0403 AND 
              tempComp NEQ 0x0103 AND 
              tempComp NEQ 0x0104 AND 
              tempComp NEQ 0x0105 AND 
              tempComp NEQ 0x0106 )
            outOfRange = TRUE;
          break;
          
        default:
          outOfRange = TRUE;
         break;
      }
    }

  }

  if( outOfRange EQ TRUE )
  {
    ACI_ERR_DESC( ACI_ERR_CLASS_Ext, EXT_ERR_Parameter );
    return( AT_FAIL );
  }

/*
 *-------------------------------------------------------------------
 * process parameter
 *-------------------------------------------------------------------
 */
  if(cid EQ PDP_CONTEXT_CID_OMITTED )
 /*
  *   cid omitted: A special form of the set command that is not defined in the Spec.
  *   This set the default value of QoS.
  */
  {
    /* Replace omitted values with subscribed values to avoid check in SM (subscribed=accept parameter from network unconditionally) */
    if( qos->qos_r99.tc EQ QOS_R99_TC_OMITTED )                               qos->qos_r99.tc                       = PS_TC_SUB;   
    if( qos->qos_r99.order EQ QOS_R99_ORDER_OMITTED )                         qos->qos_r99.order                    = PS_ORDER_SUB;    
    if( qos->qos_r99.del_err_sdu EQ QOS_R99_DEL_ERR_SDU_OMITTED )             qos->qos_r99.del_err_sdu              = PS_DEL_ERR_SUB;
    if( qos->qos_r99.max_sdu EQ QOS_R99_MAX_SDU_OMITTED )                     qos->qos_r99.max_sdu                  = PS_MAX_SDU_SUB;
    if( qos->qos_r99.max_rate_ul EQ QOS_R99_MAX_BR_UL_OMITTED )               qos->qos_r99.max_rate_ul              = PS_MAX_BR_UL_SUB;     
    if( qos->qos_r99.max_rate_dl EQ QOS_R99_MAX_BR_DL_OMITTED )               qos->qos_r99.max_rate_dl              = PS_MAX_BR_DL_SUB;      
    if( qos->qos_r99.xfer_delay EQ QOS_R99_XFER_DELAY_OMITTED )               qos->qos_r99.xfer_delay               = PS_XFER_DELAY_SUB;      
    if( qos->qos_r99.handling_pri EQ QOS_R99_HANDLING_PRIO_OMITTED )          qos->qos_r99.handling_pri             = PS_HANDLING_PRI_SUB;      
    if( qos->qos_r99.guar_br_ul EQ QOS_R99_GUAR_BR_UL_OMITTED )               qos->qos_r99.guar_br_ul               = PS_GUAR_BR_UL_SUB;      
    if( qos->qos_r99.guar_br_dl EQ QOS_R99_GUAR_BR_DL_OMITTED )               qos->qos_r99.guar_br_dl               = PS_GUAR_BR_DL_SUB;      
    if( qos->qos_r99.ber.ratio_mant EQ QOS_R99_RATIO_MANT_OMITTED )           qos->qos_r99.ber.ratio_mant           = 0;  /* '0' is the subscribed value */      
    if( qos->qos_r99.ber.ratio_exp EQ QOS_R99_RATIO_EXP_OMITTED )             qos->qos_r99.ber.ratio_exp            = 0;  /* '0' is the subscribed value */      
    if( qos->qos_r99.sdu_err_ratio.ratio_mant EQ QOS_R99_RATIO_MANT_OMITTED ) qos->qos_r99.sdu_err_ratio.ratio_mant = 0;  /* '0' is the subscribed value */      
    if( qos->qos_r99.sdu_err_ratio.ratio_exp EQ QOS_R99_RATIO_EXP_OMITTED )   qos->qos_r99.sdu_err_ratio.ratio_exp  = 0;  /* '0' is the subscribed value */
    cmhSM_change_def_QOS_min( (T_PS_min_qos *)qos, (T_PS_ctrl_min_qos)PS_is_R99 );

    retCd = AT_CMPL;
  }
  else
  {
/*
 *    Check that a context is defined before changing the QoS.
 */
    p_pdp_context_node = pdp_context_find_node_from_cid( cid );
    if( !p_pdp_context_node )
    {
      return AT_FAIL;
    }

    c_state = get_state_over_cid( cid );
    switch (c_state)
    {
      case PDP_CONTEXT_STATE_INVALID:
        /* Wrong context state (not defined): Reject command */
        ACI_ERR_DESC( ACI_ERR_CLASS_Ext, EXT_ERR_Parameter );
        return( AT_FAIL );
      default: ;
        /* Context is defined: Continue */
    }

    if( !qos OR ( qos->qos_r99.tc           EQ QOS_R99_TC_OMITTED            AND
                  qos->qos_r99.order        EQ QOS_R99_ORDER_OMITTED         AND
                  qos->qos_r99.del_err_sdu  EQ QOS_R99_DEL_ERR_SDU_OMITTED   AND
                  qos->qos_r99.max_sdu      EQ QOS_R99_MAX_SDU_OMITTED       AND
                  qos->qos_r99.max_rate_ul  EQ QOS_R99_MAX_BR_UL_OMITTED     AND
                  qos->qos_r99.max_rate_dl  EQ QOS_R99_MAX_BR_DL_OMITTED     AND
                  qos->qos_r99.xfer_delay   EQ QOS_R99_XFER_DELAY_OMITTED    AND
                  qos->qos_r99.handling_pri EQ QOS_R99_HANDLING_PRIO_OMITTED AND
                  qos->qos_r99.guar_br_ul   EQ QOS_R99_GUAR_BR_UL_OMITTED    AND
                  qos->qos_r99.guar_br_dl   EQ QOS_R99_GUAR_BR_DL_OMITTED    AND
                  qos->qos_r99.ber.ratio_mant EQ QOS_R99_RATIO_MANT_OMITTED  AND
                  qos->qos_r99.ber.ratio_exp  EQ QOS_R99_RATIO_EXP_OMITTED   AND
                  qos->qos_r99.sdu_err_ratio.ratio_mant EQ QOS_R99_RATIO_MANT_OMITTED AND
                  qos->qos_r99.sdu_err_ratio.ratio_exp  EQ QOS_R99_RATIO_EXP_OMITTED ) )

    {
/*
 *    QoS omitted ->  undefine the requested QOS
 */
      cmhSM_Set_default_QOS_min( cid );

      /* If the default parameters is in R97 format, is must be converted to R99 */
      if( p_pdp_context_node->ctrl_min_qos EQ (T_PS_ctrl_min_qos)PS_is_R97)
      {
        memcpy( &temp_qos, &p_pdp_context_node->min_qos.qos_r97, sizeof(T_PS_qos_r97) );
        cl_qos_convert_r97_to_r99( &temp_qos, &p_pdp_context_node->min_qos.qos_r99 );
        p_pdp_context_node->ctrl_min_qos = (T_PS_ctrl_min_qos)PS_is_R99;
      }

      retCd = AT_CMPL;
    }
    else
    {
/*
 *    Define the requested QOS
 */

      p_pdp_context_node = pdp_context_find_node_from_cid( cid );
      if( p_pdp_context_node )
      {
        /* Conversion of old R97 to R99 is not necessary because +CGEQMIN always specifies all parameters (omitting=not checking). */
        p_pdp_context_node->ctrl_min_qos = (T_PS_ctrl_min_qos)PS_is_R99;

        /* Copy the checked parameters directly to pdp_context */
        memcpy( &p_pdp_context_node->min_qos.qos_r99, qos, sizeof(T_PS_qos_r99) );
      
        /* Replace omitted values with subscribed values to avoid check in SM (subscribed=accept parameter from network unconditionally) */
        if( qos->qos_r99.tc EQ QOS_R99_TC_OMITTED )
          p_pdp_context_node->min_qos.qos_r99.tc = PS_TC_SUB;
      
        if( qos->qos_r99.order EQ QOS_R99_ORDER_OMITTED )
          p_pdp_context_node->min_qos.qos_r99.order = PS_ORDER_SUB;
      
        if( qos->qos_r99.del_err_sdu EQ QOS_R99_DEL_ERR_SDU_OMITTED )
          p_pdp_context_node->min_qos.qos_r99.del_err_sdu = PS_DEL_ERR_SUB;
      
        if( qos->qos_r99.max_sdu EQ QOS_R99_MAX_SDU_OMITTED )
          p_pdp_context_node->min_qos.qos_r99.max_sdu = PS_MAX_SDU_SUB;
      
        if( qos->qos_r99.max_rate_ul EQ QOS_R99_MAX_BR_UL_OMITTED )
          p_pdp_context_node->min_qos.qos_r99.max_rate_ul = PS_MAX_BR_UL_SUB;
      
        if( qos->qos_r99.max_rate_dl EQ QOS_R99_MAX_BR_DL_OMITTED )
          p_pdp_context_node->min_qos.qos_r99.max_rate_dl = PS_MAX_BR_DL_SUB;
      
        if( qos->qos_r99.xfer_delay EQ QOS_R99_XFER_DELAY_OMITTED )
          p_pdp_context_node->min_qos.qos_r99.xfer_delay = PS_XFER_DELAY_SUB;
      
        if( qos->qos_r99.handling_pri EQ QOS_R99_HANDLING_PRIO_OMITTED )
          p_pdp_context_node->min_qos.qos_r99.handling_pri = PS_HANDLING_PRI_SUB;
      
        if( qos->qos_r99.guar_br_ul EQ QOS_R99_GUAR_BR_UL_OMITTED )
          p_pdp_context_node->min_qos.qos_r99.guar_br_ul = PS_GUAR_BR_UL_SUB;
      
        if( qos->qos_r99.guar_br_dl EQ QOS_R99_GUAR_BR_DL_OMITTED )
          p_pdp_context_node->min_qos.qos_r99.guar_br_dl = PS_GUAR_BR_DL_SUB;
      
        if( qos->qos_r99.ber.ratio_mant EQ QOS_R99_RATIO_MANT_OMITTED )
          p_pdp_context_node->min_qos.qos_r99.ber.ratio_mant = 0; /* '0' is the subscribed value */
      
        if( qos->qos_r99.ber.ratio_exp EQ QOS_R99_RATIO_EXP_OMITTED )
          p_pdp_context_node->min_qos.qos_r99.ber.ratio_exp = 0;  /* '0' is the subscribed value */
      
        if( qos->qos_r99.sdu_err_ratio.ratio_mant EQ QOS_R99_RATIO_MANT_OMITTED )
          p_pdp_context_node->min_qos.qos_r99.sdu_err_ratio.ratio_mant = 0; /* '0' is the subscribed value */
      
        if( qos->qos_r99.sdu_err_ratio.ratio_exp EQ QOS_R99_RATIO_EXP_OMITTED )
          p_pdp_context_node->min_qos.qos_r99.sdu_err_ratio.ratio_exp = 0;  /* '0' is the subscribed value */
      }
      else
      {
        return( AT_FAIL );
      }

      retCd = AT_CMPL;
    }

  }

  return retCd;
}
#endif /* REL99 */

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (8441)         MODULE  : CMH_SMS                  |
| STATE   : finnished             ROUTINE : sAT_PlusCGDCONT          |
+--------------------------------------------------------------------+

PURPOSE : This is the functional counterpart to the +CGDCONT= AT
          command which sets the current setting for each context.

          GACI Context Definition GSM - 7.60 10.2.1

          Special case +CGDCONT=<n> undefines context n
          is handled as separate function call

          otherwise:

+CGDCONT=[<cid> [,<PDP_TYPE> [,<APN> [,<PDP_addr> [,<h_comp> [,<d_comp>]]]]]]

 Issue of what happens if user changes data of an active context.

 Take simple approach, do not renegotiate current context.
 Undefinition is more complex, reject attempt if context is active?

 Current pdp address is left alone and only reset when context is
 explicitly undefined.
 See GSM 7.60 10.2.7.

*/
GLOBAL T_ACI_RETURN sAT_PlusCGDCONT( T_ACI_CMD_SRC srcId, U8 cid, T_PDP_CONTEXT *pdp_context_input )
{
  T_PDP_CONTEXT_INTERNAL *p_pdp_context_node = NULL;
  T_PDP_CONTEXT_STATE     pdp_context_state;
  
  enum
  {
    PDP_CONTEXT_UNDEFINE = 0,
    PDP_CONTEXT_DEFINE   = 1,
    PDP_CONTEXT_DEFINE_DEFAULT = 2
  } pdp_context_action = PDP_CONTEXT_DEFINE;

  TRACE_FUNCTION ("sAT_PlusCGDCONT()");

  

/*
 *-------------------------------------------------------------------
 * check parameter
 *-------------------------------------------------------------------
 */
  if( (cid < PDP_CONTEXT_CID_MIN OR 
      cid > PDP_CONTEXT_CID_MAX) AND (cid NEQ PDP_CONTEXT_CID_OMITTED ))
  {
    ACI_ERR_DESC( ACI_ERR_CLASS_Ext, EXT_ERR_Parameter );
    return AT_FAIL;
  }
  
/* right now data compression is not supported, add PDP_CONTEXT_D_COMP_ON condition 
 * if enabled in the future                                                            
 */
  if( pdp_context_input->d_comp NEQ PDP_CONTEXT_D_COMP_OMITTED AND
      pdp_context_input->d_comp >= PDP_CONTEXT_D_COMP_INVALID)
  {
    ACI_ERR_DESC( ACI_ERR_CLASS_Ext, EXT_ERR_Parameter );
    return( AT_FAIL );
  }

  if( pdp_context_input->h_comp NEQ PDP_CONTEXT_H_COMP_OMITTED AND
      pdp_context_input->h_comp >= PDP_CONTEXT_H_COMP_INVALID)
  {
    ACI_ERR_DESC( ACI_ERR_CLASS_Ext, EXT_ERR_Parameter );
    return( AT_FAIL );
  }

  if( ! pdp_context_type_omitted( pdp_context_input->pdp_type ) )
  {
    if( ! pdp_context_type_valid( pdp_context_input->pdp_type ) )
    {
      ACI_ERR_DESC( ACI_ERR_CLASS_Ext, EXT_ERR_Parameter );
      return( AT_FAIL );
    }
  }

  if( ! pdp_context_apn_omitted( pdp_context_input->pdp_apn ) )
  {
    if( ! pdp_context_apn_valid( pdp_context_input->pdp_apn ) )
    {
      ACI_ERR_DESC( ACI_ERR_CLASS_Ext, EXT_ERR_Parameter );
      return( AT_FAIL );
    }
  }

  if( ! pdp_context_addr_omitted( &pdp_context_input->pdp_addr ) )
  {
    if( ! pdp_context_addr_valid( &pdp_context_input->pdp_addr ) )
    {
      ACI_ERR_DESC( ACI_ERR_CLASS_Ext, EXT_ERR_Parameter );
      return( AT_FAIL );
    }
  }

/*
 *-------------------------------------------------------------------
 * A special form of set command
 *-------------------------------------------------------------------
 */
  if( cid NEQ PDP_CONTEXT_CID_OMITTED AND
      pdp_context_type_omitted( pdp_context_input->pdp_type ) AND 
      pdp_context_apn_omitted(  pdp_context_input->pdp_apn  ) AND 
      pdp_context_addr_omitted( &pdp_context_input->pdp_addr ) AND
      pdp_context_input->d_comp EQ PDP_CONTEXT_D_COMP_OMITTED AND 
      pdp_context_input->h_comp EQ PDP_CONTEXT_H_COMP_OMITTED ) 
  {
    pdp_context_action = PDP_CONTEXT_UNDEFINE;
  } /* end if ... the special form of the set command */
  else if( (cid EQ PDP_CONTEXT_CID_OMITTED) AND(
          !pdp_context_type_omitted( pdp_context_input->pdp_type )  OR 
          !pdp_context_apn_omitted(  pdp_context_input->pdp_apn  )  OR 
          !pdp_context_addr_omitted( &pdp_context_input->pdp_addr ) OR
          !((T_PDP_CONTEXT_D_COMP)pdp_context_input->d_comp EQ PDP_CONTEXT_D_COMP_OMITTED)  OR 
          !((T_PDP_CONTEXT_H_COMP)pdp_context_input->h_comp EQ PDP_CONTEXT_H_COMP_OMITTED) )) 
  {  
    pdp_context_action = PDP_CONTEXT_DEFINE_DEFAULT;
  }
  else
  {
  
  /*
   *-------------------------------------------------------------------
   * default parameter
   *-------------------------------------------------------------------
   */
    if( pdp_context_input->d_comp EQ PDP_CONTEXT_D_COMP_OMITTED )
      pdp_context_input->d_comp = PDP_CONTEXT_D_COMP_OFF;

    if( pdp_context_input->h_comp EQ PDP_CONTEXT_H_COMP_OMITTED )
      pdp_context_input->h_comp = PDP_CONTEXT_H_COMP_OFF;

    if ( pdp_context_type_omitted( pdp_context_input->pdp_type ) )
      strcpy(pdp_context_input->pdp_type, "IP");
   }
   
/*
 *-------------------------------------------------------------------
 * process parameter
 *-------------------------------------------------------------------
 */

  switch ( pdp_context_action )
  {
    case PDP_CONTEXT_UNDEFINE:
    {      
      pdp_context_state = pdp_context_get_state_for_cid( cid );

      if( pdp_context_state EQ PDP_CONTEXT_STATE_DEFINED )
      {
        if( !pdp_context_cid_used_by_other( cid ) )
        {
          pdp_context_remove_node( cid );
        }
        else
        {
          ACI_ERR_DESC( ACI_ERR_CLASS_Ext, EXT_ERR_Parameter );
          return( AT_FAIL );
        }
      }
      else
      {
        ACI_ERR_DESC( ACI_ERR_CLASS_Cme, CME_ERR_OpNotAllow );
        return( AT_FAIL );
      }
      break;
    }
    case PDP_CONTEXT_DEFINE:
    {
      p_pdp_context_node = pdp_context_create_node( cid );
      if( p_pdp_context_node )
      {
        set_state_over_cid( cid, PDP_CONTEXT_STATE_DEFINED );
      }
      else
      {
        p_pdp_context_node = pdp_context_find_node_from_cid( cid );

        if( p_pdp_context_node)
        {
          switch(p_pdp_context_node->internal_data.state)
      {
          /* every time allowed */
          case PDP_CONTEXT_STATE_INVALID:
          case PDP_CONTEXT_STATE_DEFINED:
            p_pdp_context_node->internal_data.state = PDP_CONTEXT_STATE_DEFINED;
          break;
          /* allowed during context deactivation, but
             WITHOUT state change                     */
          case PDP_CONTEXT_STATE_ABORT_ESTABLISH:
          case PDP_CONTEXT_STATE_DEACTIVATE_NORMAL:
          case PDP_CONTEXT_STATE_BREAKDOWN_LINK_NORMAL:
          case PDP_CONTEXT_STATE_REACTIVATION_1:
          case PDP_CONTEXT_STATE_REACTIVATION_2:
          break;
          /* Not allowed during context activation or
             for activated context                    */
          case PDP_CONTEXT_STATE_ATTACHING:
          case PDP_CONTEXT_STATE_ESTABLISH_1:
          case PDP_CONTEXT_STATE_ESTABLISH_2:
          case PDP_CONTEXT_STATE_ESTABLISH_3:
          case PDP_CONTEXT_STATE_ACTIVATING:
          case PDP_CONTEXT_STATE_ACTIVATED:
          case PDP_CONTEXT_STATE_DATA_LINK:
          default:
            ACI_ERR_DESC( ACI_ERR_CLASS_Cme, CME_ERR_OpNotAllow );
            return( AT_FAIL );
          }
        }
      }
      
      cmhSM_Set_default_QOS( cid );
      cmhSM_Set_default_QOS_min( cid );

      sAT_PlusCGDCONT_exec( cid, pdp_context_input );
	  
      if ( p_pdp_context_node NEQ NULL )
      {
         set_state_over_cid( cid, p_pdp_context_node->internal_data.state );
      	}
      
      break;
    }
    case PDP_CONTEXT_DEFINE_DEFAULT:
    {
      memcpy(&pdp_context_default.attributes, pdp_context_input, sizeof(T_PDP_CONTEXT));
    }
  }

  return AT_CMPL;
}


/*
+--------------------------------------------------------------------+
| PROJECT : UMTS                  MODULE  : CMH_SMS                  |
| STATE   : -                     ROUTINE : sAT_PlusCGDCONT_exec     |
+--------------------------------------------------------------------+
*/
GLOBAL void sAT_PlusCGDCONT_exec( U8 cid, T_PDP_CONTEXT *p_pdp_context_input)
{

  T_PDP_CONTEXT_INTERNAL *p_pdp_context_node = NULL;
/*lint -e662 (Warning:Possible creation of out-of-bounds pointe)*/

  TRACE_FUNCTION("sAT_PlusCGDCONT_exec");

  p_pdp_context_node = pdp_context_find_node_from_cid( cid );
  if( p_pdp_context_node )
  {
    p_pdp_context_node->type = PDP_CONTEXT_TYPE_PRIMARY;
    
    //if context type is omitted, use the one from the default context
    if(pdp_context_type_omitted( p_pdp_context_input->pdp_type ))
    {
      memcpy( &(p_pdp_context_node->attributes.pdp_type), 
        &(pdp_context_default.attributes.pdp_type), 
        sizeof(T_PDP_CONTEXT_PDP_TYPE) );
    }
    else
    {
      memcpy( &(p_pdp_context_node->attributes.pdp_type), 
        &(p_pdp_context_input->pdp_type), 
        sizeof(T_PDP_CONTEXT_PDP_TYPE) );
    }
    
    //if pdp apn is omitted, use the one from the default context
    if (pdp_context_apn_omitted(  p_pdp_context_input->pdp_apn  ))
    {
      memcpy( &(p_pdp_context_node->attributes.pdp_apn), 
        &(pdp_context_default.attributes.pdp_apn), 
        sizeof(T_PDP_CONTEXT_APN) );
    }
    else
    {
      memcpy( &(p_pdp_context_node->attributes.pdp_apn), 
        &(p_pdp_context_input->pdp_apn), 
        sizeof(T_PDP_CONTEXT_APN) );
    }
    
    //if context address is omitted, use the one from the default context
    if (pdp_context_addr_omitted( &p_pdp_context_input->pdp_addr ) )
    {
      memcpy( &(p_pdp_context_node->attributes.pdp_addr), 
        &(pdp_context_default.attributes.pdp_addr), 
        sizeof(T_NAS_ip) );
    }
    else
    {
      memcpy( &(p_pdp_context_node->attributes.pdp_addr), 
        &(p_pdp_context_input->pdp_addr), 
        sizeof(T_NAS_ip) );
    }
    
    //copy default pco setting to context 
    memcpy(&(p_pdp_context_node->internal_data.network_pco), &(pdp_context_default.internal_data.network_pco), sizeof(T_PDP_CONTEXT_PCO));
    memcpy(&(p_pdp_context_node->internal_data.user_pco), &(pdp_context_default.internal_data.user_pco), sizeof(T_PDP_CONTEXT_PCO));

    if(p_pdp_context_input->d_comp EQ PDP_CONTEXT_D_COMP_OMITTED)
      p_pdp_context_node->attributes.d_comp = pdp_context_default.attributes.d_comp;
    else
      p_pdp_context_node->attributes.d_comp = p_pdp_context_input->d_comp;

    if (p_pdp_context_input->h_comp EQ PDP_CONTEXT_H_COMP_OMITTED) 
      p_pdp_context_node->attributes.h_comp = pdp_context_default.attributes.h_comp;
    else    
      p_pdp_context_node->attributes.h_comp = p_pdp_context_input->h_comp;
  }
}

#ifdef REL99
/*
+--------------------------------------------------------------------+
| PROJECT : UMTS                  MODULE  : CMH_SMS                  |
| STATE   : -                     ROUTINE : sAT_PlusCGDSCONT         |
+--------------------------------------------------------------------+

PURPOSE : This is the functional counterpart to the +CGDSCONT= 
          AT command which sets the current setting for each secondary 
          PDP context.

          Special case: +CGDSCONT=<cid> undefines a secondary PDP context

          +CGDSCONT=<cid> [,<p_cid> [,<h_comp> [,<d_comp>]]]
*/
GLOBAL T_ACI_RETURN sAT_PlusCGDSCONT( T_ACI_CMD_SRC srcId, U8 cid, T_PDP_CONTEXT *p_pdp_context_input )
{

  T_PDP_CONTEXT_INTERNAL *p_pdp_context_node = NULL;
  
  enum
  {
    PDP_CONTEXT_UNDEFINE = 0,
    PDP_CONTEXT_DEFINE   = 1
    
  } pdp_context_action = PDP_CONTEXT_DEFINE;

  TRACE_FUNCTION ("sAT_PlusCGDSCONT()");

/*
 *-------------------------------------------------------------------
 * check entity status
 *-------------------------------------------------------------------
 */
  if( smEntStat.curCmd NEQ AT_CMD_NONE )
    return( AT_BUSY );

/*
 *-------------------------------------------------------------------
 * check parameter
 *-------------------------------------------------------------------
 */
  if ( ((cid < PDP_CONTEXT_CID_MIN) OR (cid > PDP_CONTEXT_CID_MAX)) AND (cid NEQ PDP_CONTEXT_CID_OMITTED) )
  {
    ACI_ERR_DESC( ACI_ERR_CLASS_Ext, EXT_ERR_Parameter );
    return AT_FAIL;
  }

  if( p_pdp_context_input->p_cid NEQ PDP_CONTEXT_CID_OMITTED )
  {
    if( p_pdp_context_input->p_cid >= PDP_CONTEXT_CID_MIN AND 
        p_pdp_context_input->p_cid <= PDP_CONTEXT_CID_MAX )
    {
      p_pdp_context_node = pdp_context_find_node_from_cid( p_pdp_context_input->p_cid );
      if( p_pdp_context_node )
      {
        if( p_pdp_context_node->type NEQ PDP_CONTEXT_TYPE_PRIMARY )
        {
          /* The PDP context found is not a primary PDP context */
          ACI_ERR_DESC( ACI_ERR_CLASS_Ext, EXT_ERR_Parameter );
          return AT_FAIL;
        }
      }
      else
      {
        /* The primary PDP context has not been created */
        ACI_ERR_DESC( ACI_ERR_CLASS_Ext, EXT_ERR_Parameter );
        return AT_FAIL;
      }
    }
  }
  
/* right now data compression is not supported, add PDP_CONTEXT_D_COMP_ON condition 
 * if enabled in the future                                                            
 */
  if( p_pdp_context_input->d_comp NEQ PDP_CONTEXT_D_COMP_OMITTED AND
      p_pdp_context_input->d_comp >= PDP_CONTEXT_D_COMP_INVALID)
  {
    /* d_comp out of range */
    ACI_ERR_DESC( ACI_ERR_CLASS_Ext, EXT_ERR_Parameter );
    return( AT_FAIL );
  }

  if( p_pdp_context_input->h_comp NEQ PDP_CONTEXT_H_COMP_OMITTED AND
      p_pdp_context_input->h_comp >= PDP_CONTEXT_H_COMP_INVALID )
  {
    /* h_copm out of range */
    ACI_ERR_DESC( ACI_ERR_CLASS_Ext, EXT_ERR_Parameter );
    return( AT_FAIL );
  }
/*
  p_pdp_context_node = pdp_context_find_node_from_cid( p_pdp_context_input->p_cid );
  if( !p_pdp_context_node )
  {
    ACI_ERR_DESC( ACI_ERR_CLASS_Ext, EXT_ERR_Parameter );
    return( AT_FAIL );
  }
*/
/*
 *-------------------------------------------------------------------
 * A special form of set command, that causes the context to become
 * undefined.
 *-------------------------------------------------------------------
 */
 
  if( cid NEQ PDP_CONTEXT_CID_OMITTED AND
      p_pdp_context_input->p_cid  EQ PDP_CONTEXT_CID_OMITTED AND
      p_pdp_context_input->d_comp EQ PDP_CONTEXT_D_COMP_OMITTED AND 
      p_pdp_context_input->h_comp EQ PDP_CONTEXT_H_COMP_OMITTED ) 
  {
    pdp_context_action = PDP_CONTEXT_UNDEFINE;
  } /* end if ... the special form of the set command */
  
/*
 *-------------------------------------------------------------------
 * default parameter
 *-------------------------------------------------------------------
 */
  if( pdp_context_action EQ PDP_CONTEXT_DEFINE )    
  {
    if( p_pdp_context_input->d_comp EQ PDP_CONTEXT_D_COMP_OMITTED )
    {
      p_pdp_context_input->d_comp = PDP_CONTEXT_D_COMP_OFF;
    }
  
    if( p_pdp_context_input->h_comp EQ PDP_CONTEXT_H_COMP_OMITTED )
    {
      p_pdp_context_input->h_comp = PDP_CONTEXT_H_COMP_OFF;
    }
  }

/*
 *-------------------------------------------------------------------
 * process parameter
 *-------------------------------------------------------------------
 */

  switch ( pdp_context_action )
  {
    case PDP_CONTEXT_UNDEFINE:
    {      
      p_pdp_context_node = pdp_context_find_node_from_cid( cid );

      if( !p_pdp_context_node->p_tft_pf )
      {
        if( !pdp_context_remove_node( cid ) )
        {
          TRACE_ERROR( "ERROR: Failed to remove secondary PDP context" );
          ACI_ERR_DESC( ACI_ERR_CLASS_Ext, EXT_ERR_Parameter );
          return( AT_FAIL );
        }
      }
      else
      {
        TRACE_ERROR( "ERROR: TFT Packet Filter not removed" );
        ACI_ERR_DESC( ACI_ERR_CLASS_Ext, EXT_ERR_Parameter );
        return( AT_FAIL );
      }

      return( AT_CMPL );
    }

    case PDP_CONTEXT_DEFINE:
    {
      if( sAT_PlusCGDSCONT_exec( cid, p_pdp_context_input ) )
      {
        set_state_over_cid( cid, PDP_CONTEXT_STATE_DEFINED );
        cmhSM_Set_default_QOS( cid );
        cmhSM_Set_default_QOS_min( cid );
        return( AT_CMPL );
      }
      else
        return( AT_FAIL );
    }
  }

  return( AT_FAIL );
}


/*
+--------------------------------------------------------------------+
| PROJECT : UMTS                  MODULE  : CMH_SMS                  |
| STATE   : -                     ROUTINE : sAT_PlusCGDSCONT_exec    |
+--------------------------------------------------------------------+
    If the function fails to create the secondary PDP context
    FALSE is returned otherwise TRUE.

*/
GLOBAL BOOL sAT_PlusCGDSCONT_exec( U8 cid, T_PDP_CONTEXT *p_pdp_context_input)
{

  T_PDP_CONTEXT_INTERNAL *p_pdp_context_node         = NULL;
  T_PDP_CONTEXT_INTERNAL *p_pdp_context_node_primary = NULL;
  
  TRACE_FUNCTION("sAT_PlusCGDSCONT_exec");

  p_pdp_context_node = pdp_context_create_node( cid );
  p_pdp_context_node_primary = pdp_context_find_node_from_cid( p_pdp_context_input->p_cid );

  if( p_pdp_context_node AND p_pdp_context_node_primary )
  {
    p_pdp_context_node->type = PDP_CONTEXT_TYPE_SECONDARY;
    p_pdp_context_node->attributes.p_cid = p_pdp_context_input->p_cid;
    
    memcpy( &(p_pdp_context_node->attributes.pdp_type), 
            &(p_pdp_context_node_primary->attributes.pdp_type), 
            sizeof(T_PDP_CONTEXT_PDP_TYPE) );

    memcpy( &(p_pdp_context_node->attributes.pdp_apn), 
            &(p_pdp_context_node_primary->attributes.pdp_apn), 
            sizeof(T_PDP_CONTEXT_APN) );

    memcpy( &(p_pdp_context_node->attributes.pdp_addr), 
            &(p_pdp_context_node_primary->attributes.pdp_addr), 
            sizeof(T_NAS_ip) );

    p_pdp_context_node->attributes.d_comp = p_pdp_context_input->d_comp;
    p_pdp_context_node->attributes.h_comp = p_pdp_context_input->h_comp;
  }
  else
  {
    return FALSE;
  }

  return TRUE;
  
}


/*
+--------------------------------------------------------------------+
| PROJECT : UMTS                  MODULE  : CMH_SMS                  |
| STATE   : -                     ROUTINE : sAT_PlusCGTFT            |
+--------------------------------------------------------------------+

PURPOSE : This is the functional counterpart to the +CGTFT= AT command 
          which defines a PF for a TFT.


*/
GLOBAL T_ACI_RETURN sAT_PlusCGTFT( T_ACI_CMD_SRC srcId, U8 cid, T_NAS_tft_pf *p_tft_pf_input )
{

  T_PDP_CONTEXT_INTERNAL *p_pdp_context_node = NULL;
  T_TFT_INTERNAL         *p_tft_pf           = NULL;

  U8 pdp_context_cids[PDP_CONTEXT_CID_MAX]; /* containds cids of contexts associated to the same PDP address */
  int i = 0;

  TRACE_FUNCTION("sAT_PlusCGTFT()");

  memset( pdp_context_cids, 0, sizeof( pdp_context_cids ) );

/*
 *-------------------------------------------------------------------
 * check entity status
 *-------------------------------------------------------------------
 */

  if( smEntStat.curCmd NEQ AT_CMD_NONE )
  {
    return( AT_BUSY );
  }


/*
 *-------------------------------------------------------------------
 * check parameter
 *-------------------------------------------------------------------
 */

  p_pdp_context_node = pdp_context_find_node_from_cid( cid );
  if( !p_pdp_context_node )
  {
    TRACE_EVENT("ERROR: PDP context not defined");
    return( AT_FAIL );
  }

  if( p_tft_pf_input->tft_pf_id < TFT_PF_ID_MIN OR p_tft_pf_input->tft_pf_id > TFT_PF_ID_MAX )
  {
    TRACE_EVENT("ERROR: PF identifier out of range");
    return( AT_FAIL );
  }

  /* Check the evaluation precedence index.                          */
  /* The precedence index must be unike within the same PDP address. */
  
  /* Is is a secondary PDP context */
  switch( p_pdp_context_node->type )
  {
    case PDP_CONTEXT_TYPE_PRIMARY:
      pdp_context_cids[i++] = p_pdp_context_node->cid;
      break;
      
    case PDP_CONTEXT_TYPE_SECONDARY:
      pdp_context_cids[i++] = p_pdp_context_node->attributes.p_cid;
      p_pdp_context_node = pdp_context_find_node_from_cid( p_pdp_context_node->attributes.p_cid );
      break;

    default:
      TRACE_EVENT("ERROR: PDP context type error");
      return( AT_FAIL );
  }


  /* Search throug all defined PDP contexts.                                 */
  /* Since the primary PDP context must be defined before defining a         */
  /* secondary PDP context, the search is performed form the primary context */
  while( p_pdp_context_node )
  {
    if( p_pdp_context_node->attributes.p_cid EQ pdp_context_cids[0] )
    {
      pdp_context_cids[i] = p_pdp_context_node->cid;
      if( p_pdp_context_node->p_next )
        i++;
    }
    p_pdp_context_node = p_pdp_context_node->p_next;
  }

  i--;
  while( i >= (PDP_CONTEXT_CID_MIN - 1) AND i < PDP_CONTEXT_CID_MAX )
  {
    p_pdp_context_node = pdp_context_find_node_from_cid( pdp_context_cids[i] );
    
    p_tft_pf = p_pdp_context_node->p_tft_pf;

    while( p_tft_pf )
    {
      if( p_tft_pf->pf_attributes.tft_pf_precedence NEQ p_tft_pf_input->tft_pf_precedence )
      {
        p_tft_pf = p_tft_pf->p_next;
      }
      else
      {
        TRACE_EVENT("ERROR: Precedence index in use");
        return( AT_FAIL );
      }
    }

    i--;
  }


  /*
   * Check for omitted parameters. If all, except cid and tft_pf_id, are omitted the TFT PF must be removed.
   */
  if(  cid                               AND
       p_tft_pf_input->tft_pf_id         AND
     ! p_tft_pf_input->tft_pf_precedence AND
     ! p_tft_pf_input->tft_pf_valid_bits )
  {
    if( ! pdp_context_del_tft_pf( cid, p_tft_pf_input->tft_pf_id ) )
    {
      return AT_FAIL;
    }
  }
  else
  {
  
    /* Check if the TFT PF is already created */
    p_tft_pf = pdp_context_find_tft_pf( cid, p_tft_pf_input->tft_pf_id );
    if( p_tft_pf )
    {
      /* The TFT PF is already created, overwrite existing attrributes */

      memset( &p_tft_pf->pf_attributes, 0, sizeof(T_NAS_tft_pf) );
      memcpy( &p_tft_pf->pf_attributes, p_tft_pf_input, sizeof( T_NAS_tft_pf ) );

      if( get_state_over_cid(cid) EQ PDP_CONTEXT_STATE_ACTIVATED OR
          get_state_over_cid(cid) EQ PDP_CONTEXT_STATE_DATA_LINK )
      {
      
        p_pdp_context_node = pdp_context_find_node_from_cid( cid );
        if( p_pdp_context_node )
        {
          p_pdp_context_node->tft_changed = TRUE;
        }
      }
       
    }
    else
    {
      p_tft_pf = pdp_context_add_tft_pf( cid, p_tft_pf_input->tft_pf_id );
      if( p_tft_pf )
      {
        memcpy( &p_tft_pf->pf_attributes, p_tft_pf_input, sizeof( T_NAS_tft_pf ) );
        
        p_pdp_context_node = pdp_context_find_node_from_cid( cid );
        if( p_pdp_context_node )
        {
          p_pdp_context_node->tft_changed = TRUE;
        }
        
      }
      else
      {
        return( AT_FAIL );
      }
    }
  }
  
  return AT_CMPL;
   
}
#endif /* REL99 */
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (8441)         MODULE  : CMH_SMS                  |
| STATE   : finnished             ROUTINE : sAT_PlusCGACT            |
+--------------------------------------------------------------------+

PURPOSE : This is the functional counterpart to the +CGACT= AT
          command which causes the cids specified in the cids list to
          be activated or deactivated according to state.

          An empty list will cause all defined contexts to be
          activated or deactivated. If taken literally, this means that
          if more contexts are defined than supported, each will be
          activated, resulting in 'no resource' errors for the late ones
          as the GACI SEM will reject requests for more activations
          than it can cope with.

          Note that the context is/are activated, but no CONNECT is sent
          to the TE. This is the difference between ACT and DATA commands.

          SMREG activate req does not need l2p to be sent, but in this case
          the PDP config options are undefined (see below).

          How does a DATA call bind these 'orphan' connections to a TE given
          that a cid definition is a 'template' due to its ambiguity.

          Practically, the activate form of this command has little meaning in the
          case of PPP and loopback protocols (only ones supported at present).

          Simplest option at the moment is not to support the activate form until
          a protocol type is supported can make real use of it. The deactivate form
          is of use in switching off a Loopback connection.

          If activation before protocol establishment is supported, a NULL protocol service
          will have to be provided which supplies a default (empty?) PCO list to SMREG for
          activation and stores the network PCO response until a CGDATA is issued, must then
          convert the protocol into that requested by the CGDATA command.
          For future implementation


          Other issues for multiple context activation :

          - need to add a para onto GACI activate to tell it whether
          to do a CONNECT or an OK callback on activation.

*/
GLOBAL T_ACI_RETURN sAT_PlusCGACT( T_ACI_CMD_SRC srcId, T_CGACT_STATE state, SHORT *cids )
{
  T_PDP_CONTEXT_STATE     pdp_context_state;
  T_PDP_CONTEXT_INTERNAL *p_pdp_context_node = NULL;
  U8  i = 0;
  U8  j = 0;


  char * number= "*99#";
  T_ACI_RETURN ret_val;
  TRACE_FUNCTION ("sAT_PlusCGACT()");

/*
 *-------------------------------------------------------------------
 * check command source
 *-------------------------------------------------------------------
 */
  if(!cmh_IsVldCmdSrc (srcId))
  {
    ACI_ERR_DESC( ACI_ERR_CLASS_Ext, EXT_ERR_Parameter );
    return( AT_FAIL );
  }

/*
 *-------------------------------------------------------------------
 * check parameter
 *-------------------------------------------------------------------
 */
  if ( (state < CGACT_STATE_OMITTED) OR (state >= CGACT_STATE_INVALID) )
  {
    ACI_ERR_DESC( ACI_ERR_CLASS_Ext, EXT_ERR_Parameter );
    return AT_FAIL;
  }

  i = 0;
  while ((i<PDP_CONTEXT_CID_MAX) AND (cids[i] NEQ PDP_CONTEXT_CID_INVALID))
  {
    if ( (cids[i] < PDP_CONTEXT_CID_MIN OR cids[i] > PDP_CONTEXT_CID_MAX) )
    {
      ACI_ERR_DESC( ACI_ERR_CLASS_Ext, EXT_ERR_Parameter );
      return( AT_FAIL );
    }
    i++;
  }

/*
 *-------------------------------------------------------------------
 * default parameter
 *-------------------------------------------------------------------
 */
  if ( state EQ CGACT_STATE_OMITTED )
  { /* state is not optional */
    ACI_ERR_DESC( ACI_ERR_CLASS_Ext, EXT_ERR_Parameter );
    return AT_FAIL;
  }

/*
 *-------------------------------------------------------------------
 * enable +CGACT to deactivate a context during activation
 *-------------------------------------------------------------------
 */
  if( CGACT_STATE_DEACTIVATED EQ state )
  {
    ret_val = cmhSM_deactivateContexts(srcId, cids);
    switch(ret_val)
    {
      case AT_EXCT:
       smEntStat.curCmd = AT_CMD_CGACT;
       smEntStat.entOwn = srcId;
       smShrdPrm.owner  = (UBYTE) srcId;
       return AT_EXCT;
      default:
       return ret_val;
    }
  }

/*
 *-------------------------------------------------------------------
 * check entity status
 *-------------------------------------------------------------------
 */
  if( smEntStat.curCmd NEQ AT_CMD_NONE )
    return( AT_BUSY );

  if( gpppEntStat.curCmd EQ AT_CMD_CGDATA )
    return( AT_BUSY );

  /* FDN check */
  if (pb_get_fdn_mode () EQ FDN_ENABLE AND state EQ CGACT_STATE_ACTIVATED)
  {
    if (pb_check_fdn (FDN, (UBYTE*)number) NEQ 1)
    {
      TRACE_EVENT("sAT_PlusCGACT: Entry not found in FDN, GPRS not allowed.");
      ACI_ERR_DESC( ACI_ERR_CLASS_Cme, CME_ERR_OpNotAllow);
      return (AT_FAIL);
    }
    TRACE_EVENT("sAT_PlusCGACT: Entry found in FDN, GPRS allowed.");
  }

/*
 *-------------------------------------------------------------------
 * process parameter
 *-------------------------------------------------------------------
 */

  cid_pointer = 0;

  if( *cids EQ PDP_CONTEXT_CID_INVALID ) /* all defined or activated contexts (dependent by state) */
  {
    p_pdp_context_node = p_pdp_context_list;
    while( p_pdp_context_node )
    {
      if( p_pdp_context_node->internal_data.state EQ PDP_CONTEXT_STATE_DEFINED )
      {
        work_cids[j] = p_pdp_context_node->cid;
        j++;
      }
      p_pdp_context_node = p_pdp_context_node->p_next;
    }
    work_cids[j] = PDP_CONTEXT_CID_INVALID;

    if( work_cids[0] EQ PDP_CONTEXT_CID_INVALID )
    {
      return AT_CMPL;
    }
  }
  else   /* all declarated contexts */
  {
    /* copy cid list */
    for( i = 0; cids[i] NEQ PDP_CONTEXT_CID_INVALID; i++ )
    {
      if( (cids[i] < PDP_CONTEXT_CID_MIN OR cids[i] > PDP_CONTEXT_CID_MAX) OR i >= PDP_CONTEXT_CID_MAX )
      {
        TRACE_EVENT("not a valid cid!!!");
        work_cids[0] = PDP_CONTEXT_CID_INVALID;
        return AT_FAIL;
      }
      else
      {
        work_cids[i] = (U8)cids[i];
      }
    }
    work_cids[i] = PDP_CONTEXT_CID_INVALID;

    for(j = 0; work_cids[j] NEQ PDP_CONTEXT_CID_INVALID; j++)
    {
      pdp_context_state = get_state_over_cid( work_cids[j] );
      if( pdp_context_state EQ PDP_CONTEXT_STATE_INVALID )
      { /* Context not defined, define it. */
        if( sAT_PlusCGDCONT( srcId, work_cids[j], &pdp_context_default.attributes ) EQ AT_FAIL )
        {
          return AT_FAIL;
        }
      }
      else 
      {
        if ( pdp_context_state NEQ PDP_CONTEXT_STATE_DEFINED )
        {
          cid_pointer  = 0;
          work_cids[0] = PDP_CONTEXT_CID_INVALID;
                    return ( AT_FAIL );
        }
      }
    }
  }
  TRACE_EVENT("activating context!");

  smEntStat.curCmd = AT_CMD_CGACT;
  smEntStat.entOwn = srcId;
  smShrdPrm.owner  = (UBYTE) srcId;

  smShrdPrm.direc = CGEREP_EVENT_ME_ACT;

     set_conn_param_on_all_working_cids( (UBYTE)srcId, DTI_ENTITY_INVALID );

  /*
   *-------------------------------------------------------------------
   * Check the PS attach state and attach if necessary.
   *-------------------------------------------------------------------
   */
  switch (cmhGMM_attach_if_necessary( srcId, AT_CMD_CGACT ))
  {
    case AT_EXCT:
    {
      /* Performing the attach procedure */
      set_state_working_cid( PDP_CONTEXT_STATE_ATTACHING );
      break;
    }
    case AT_CMPL:
    {
      /* Already attached -> activate the context */
      cmhSM_activate_context();
      break;
    }
    default:
    { 
      smEntStat.curCmd  = AT_CMD_NONE;
      return AT_FAIL;
    }
  }

  return AT_EXCT;
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (8441)         MODULE  : CMH_SMS                  |
| STATE   : finnished             ROUTINE : sAT_PlusCGDATA           |
+--------------------------------------------------------------------+

PURPOSE : This is the functional counterpart to the +CGDATA= AT
          command which establish the communication.
*/
GLOBAL T_ACI_RETURN sAT_PlusCGDATA( T_ACI_CMD_SRC srcId, char *L2P, U8 *p_cid_array )
{
  U8             i = 0;
  U8             j = 0;
  U8             k = 0;

  U8             contexts_to_activate[PDP_CONTEXT_CID_MAX]; // old work_cid
  T_DTI_ENTITY_ID connectToEntity = DTI_ENTITY_INVALID;
  T_PDP_CONTEXT_INTERNAL *p_pdp_context_prim_node = NULL; /* Primary PDP context node   */
  T_PDP_CONTEXT_INTERNAL *p_pdp_context_sec_node  = NULL; /* Secondary PDP context node */
  T_PDP_CONTEXT_STATE     pdp_context_state_prim;

  TRACE_FUNCTION ("sAT_PlusCGDATA()");

  memset( &contexts_to_activate, PDP_CONTEXT_CID_INVALID, sizeof( contexts_to_activate ) );

/*
 *-------------------------------------------------------------------
 * check command source
 *-------------------------------------------------------------------
 */
  if(!cmh_IsVldCmdSrc (srcId))
  {
    ACI_ERR_DESC( ACI_ERR_CLASS_Ext, EXT_ERR_Parameter );
    return( AT_FAIL );
  }


/*
 *-------------------------------------------------------------------
 * check entity status
 *-------------------------------------------------------------------
 */
  if( smEntStat.curCmd NEQ AT_CMD_NONE )

    return( AT_BUSY );

  if( gpppEntStat.curCmd NEQ AT_CMD_NONE )

    return( AT_BUSY );

  if (pb_get_fdn_mode () EQ FDN_ENABLE)
  {



    if (pb_check_fdn (0, (const UBYTE *)"*99#") NEQ PHB_OK)
    {
      TRACE_EVENT("sAT_PlusCGDATA: Entry not found in FDN, GPRS not allowed.");
      ACI_ERR_DESC( ACI_ERR_CLASS_Cme, CME_ERR_OpNotAllow);
      return (AT_FAIL);
    }
    TRACE_EVENT("sAT_PlusCGDATA: Entry found in FDN, GPRS allowed.");
  }

/*
 *-------------------------------------------------------------------
 * check parameter
 *-------------------------------------------------------------------
 */
  if ( L2P[0] EQ 0 )
    strcpy (L2P, "PPP"); /* default value */
  
  if ( (!strcmp(L2P, "PPP")) OR (!strcmp(L2P, "IP")) )
  {
#if defined(FF_PKTIO) OR defined(FF_TCP_IP)
    if ( TRUE NEQ srcc_reserve_sources( SRCC_PKTIO_SNDCP_LINK, 1 ) )
      return ( AT_FAIL );
#endif
/*    if ( FALSE EQ srcc_reserve_sources( SRCC_PPPS_SNDCP_LINK, 1 ) )
      return ( AT_FAIL );*/
    connectToEntity = DTI_ENTITY_PPPS;
  }
#if defined(FF_PKTIO) OR defined(FF_TCP_IP) OR defined(FF_PSI)
  else if( !strcmp(L2P, "M-PKT") OR !strcmp(L2P, "M-IP")) 
  {
    if ( FALSE EQ srcc_reserve_sources( SRCC_PKTIO_SNDCP_LINK, 1 ) )
      return ( AT_FAIL );
  }
#endif /* FF_PKTIO OR FF_TCP_IP OR FF_PSI */
  else 
  {
    return ( AT_FAIL );
  }

/*
  *-----------------------------------------------------------------
  * cid parameter range check
  *-----------------------------------------------------------------
  */
  i=0;
  while ((i<PDP_CONTEXT_CID_MAX) AND (p_cid_array[i] NEQ PDP_CONTEXT_CID_INVALID))
  {
    if (p_cid_array[i] > PDP_CONTEXT_CID_INVALID)
      return (AT_FAIL);
    i++;
  }

 /*
  *-----------------------------------------------------------------
  * Validate the cid array, primary PDP before secondary contexts
  *-----------------------------------------------------------------
  */

  while( p_cid_array[i] NEQ PDP_CONTEXT_CID_INVALID )
  {
    p_pdp_context_sec_node = pdp_context_find_node_from_cid( p_cid_array[i] );
    if( p_pdp_context_sec_node )
    {
      if( p_pdp_context_sec_node->type EQ PDP_CONTEXT_TYPE_SECONDARY )
      {
        pdp_context_state_prim = pdp_context_get_state_for_cid( p_pdp_context_sec_node->attributes.p_cid );
        if( pdp_context_state_prim NEQ PDP_CONTEXT_STATE_ACTIVATED AND 
            pdp_context_state_prim NEQ PDP_CONTEXT_STATE_DATA_LINK )
        {
          k = 0;
          while( k <= j )
          {
            if( p_pdp_context_sec_node->attributes.p_cid EQ contexts_to_activate[k] )
            {
              contexts_to_activate[j] = p_cid_array[i];
              j++;
              break;
            }
            else
            {
              k++;
            }
          }
          if( k > j )
          {
            /* since k > j the primary context was not privious entered */
            return( AT_FAIL );
          }
        }
        p_pdp_context_prim_node = pdp_context_find_node_from_cid( p_pdp_context_sec_node->attributes.p_cid );
        if( p_pdp_context_prim_node )
        {
          if( !p_pdp_context_sec_node->p_tft_pf AND !p_pdp_context_prim_node->p_tft_pf )
          {
            return( AT_FAIL );
          }
        }
        else
        {
          return( AT_FAIL );
        }
      }
      else
      {
        contexts_to_activate[j] = p_cid_array[i];
        j++;
      }
    }

    i++;
   
  }
  

/*
 *-------------------------------------------------------------------
 * process parameter
 *-------------------------------------------------------------------
 */
  
  memset (work_cids, PDP_CONTEXT_CID_INVALID, PDP_CONTEXT_CID_MAX);
  cid_pointer=0;

  i = 0;
  while( i < PDP_CONTEXT_CID_MAX AND p_cid_array[i] NEQ PDP_CONTEXT_CID_INVALID )
  {
    switch( pdp_context_get_state_for_cid( p_cid_array[i] ) )
    {
      case PDP_CONTEXT_STATE_INVALID:
        /* define pdp context and                     */
        /* add pdp context cid to the work_cids array */

        if( sAT_PlusCGDCONT( srcId, p_cid_array[i], &pdp_context_default.attributes ) EQ AT_FAIL )
        {
          return AT_FAIL;
        }
        else
        {
          work_cids[i] = p_cid_array[i];
        }

        break;

      case PDP_CONTEXT_STATE_DEFINED:
      case PDP_CONTEXT_STATE_ACTIVATED:
        /* add pdp context cid to the activation list */

        work_cids[i] = p_cid_array[i];
        break;

      default:
        TRACE_FUNCTION("sAT_PlusCGDATA: Invalid state for cid");
        break;
    }

    i++;
  }

/*
 *-------------------------------------------------------------------
 * check if any cids to activate in work_cids.
 *-------------------------------------------------------------------
 */
  if (work_cids[0] EQ PDP_CONTEXT_CID_INVALID)
    return AT_FAIL;


/*
 *-------------------------------------------------------------------
 * check number of context
 *-------------------------------------------------------------------
 */
#if defined(FF_PKTIO) OR defined(FF_TCP_IP)
  if ( TRUE NEQ srcc_reserve_sources( SRCC_PKTIO_SNDCP_LINK, j ) )            // j is not used !!! 
    return ( AT_FAIL );
#endif
/* process function
 *-------------------------------------------------------------------
 */

  /* APN validation */

/*
 *-------------------------------------------------------------------
 * Store the command and entity parameters.
 *-------------------------------------------------------------------
 */
  set_conn_param_on_working_cid( (UBYTE)srcId, connectToEntity );
  if (connectToEntity EQ DTI_ENTITY_PPPS)
  {
    gpppEntStat.curCmd = AT_CMD_CGDATA;
    gpppEntStat.entOwn = srcId;
    gpppShrdPrm.owner  = (UBYTE) srcId;
  }
  else
  {
    smEntStat.curCmd = AT_CMD_CGDATA;
    smEntStat.entOwn = srcId;
    smShrdPrm.owner  = (UBYTE) srcId;  
  }

  smShrdPrm.direc = CGEREP_EVENT_ME_ACT;

/*
 *-------------------------------------------------------------------
 * Check the PS attach state and attach if necessary.
 *-------------------------------------------------------------------
 */
  switch (cmhGMM_attach_if_necessary( srcId, AT_CMD_CGDATA ))
  {
    case AT_EXCT:
    {
      /* Performing the attach procedure */
      set_state_working_cid( PDP_CONTEXT_STATE_ATTACHING );
      break;
    }
    case AT_CMPL:
    {
      cmhSM_data_link_context();
      break;
    }
    default:
    {
      smEntStat.curCmd  = AT_CMD_NONE;
      gpppEntStat.curCmd = AT_CMD_NONE;
      return AT_FAIL;
    }
  }
  return AT_EXCT;

}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (8441)         MODULE  : CMH_SMS                  |
| STATE   : finnished             ROUTINE : sAT_PlusCGPADDR          |
+--------------------------------------------------------------------+

PURPOSE : This is the functional counterpart to the +CGPADDR= AT
          command which give the PDP address back.
*/
GLOBAL T_ACI_RETURN sAT_PlusCGPADDR ( T_ACI_CMD_SRC srcId, SHORT *cids, T_NAS_ip *pdp_adress )
{
  U8 index = 0;
  T_PDP_CONTEXT_INTERNAL *p_pdp_context_node = NULL;

  TRACE_FUNCTION ("sAT_PlusCGPADDR()");

  p_pdp_context_node = p_pdp_context_list;

/*
 *-------------------------------------------------------------------
 * check entity status
 *-------------------------------------------------------------------
 */
  if( smEntStat.curCmd NEQ AT_CMD_NONE )
    return( AT_BUSY );

/*
 *-------------------------------------------------------------------
 * process parameter
 *-------------------------------------------------------------------
 */
  if( *cids EQ PDP_CONTEXT_CID_OMITTED )
  {
    /*
     *  the PDP addresse for all defined contexts are returned
     */

    while( p_pdp_context_node )
    {
      cids[index] = cmhSM_get_pdp_addr_for_CGPADDR( p_pdp_context_node->cid, &pdp_adress[index] );
      
      p_pdp_context_node = p_pdp_context_node->p_next;
      index++;
    }
    
    cids[index] = PDP_CONTEXT_CID_INVALID;

  }
  else
  {
    /*
     *  the PDP addresse for all specified contexts are returned
     */

    while( cids[index] NEQ PDP_CONTEXT_CID_INVALID )
    {
      p_pdp_context_node = pdp_context_find_node_from_cid( (U8)cids[index] );
      if( p_pdp_context_node )
      {
        cids[index] = cmhSM_get_pdp_addr_for_CGPADDR( p_pdp_context_node->cid, &pdp_adress[index] );
      }
      else
      {
        return (AT_FAIL);
      }
      index++;
    }
  }

  return AT_CMPL;
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (8441)         MODULE  : CMH_SMS                  |
| STATE   : finnished             ROUTINE : sAT_PlusCGAUTO           |
+--------------------------------------------------------------------+

PURPOSE : This is the functional counterpart to the +CGAUTO= AT
          command which set the mode of automatic response to
          network request for PDP context activation.
*/
GLOBAL T_ACI_RETURN sAT_PlusCGAUTO ( T_ACI_CMD_SRC srcId, T_CGAUTO_N n )
{
  TRACE_FUNCTION ("sAT_PlusCGAUTO()");

/*
 *-------------------------------------------------------------------
 * check entity status
 *-------------------------------------------------------------------
 */
  if( smEntStat.curCmd NEQ AT_CMD_NONE )

    return( AT_BUSY );

/*
 *-------------------------------------------------------------------
 * check parameter
 *-------------------------------------------------------------------
 */
  if ( (n < CGAUTO_N_OMITTED) OR (n >= CGAUTO_N_INVALID) )
  {
    ACI_ERR_DESC( ACI_ERR_CLASS_Ext, EXT_ERR_Parameter );
    return AT_FAIL;
  }

/*
 *-------------------------------------------------------------------
 * default parameter
 *-------------------------------------------------------------------
 */
  if ( n EQ CGAUTO_N_OMITTED )
    n = CGAUTO_N_MCM_GPRS_CSC;

/*
 *-------------------------------------------------------------------
 * process parameter
 *-------------------------------------------------------------------
 */
  automatic_response_mode = (SHORT) n;

  /* the MT shall attempt to perform a GPRS attach if it is not already attached */
  if ( n EQ 1 )
  {
    return sAT_PlusCGATT ( srcId, CGATT_STATE_ATTACHED );
  }
  return AT_CMPL;
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (8441)         MODULE  : CMH_SMS                  |
| STATE   : finnished             ROUTINE : sAT_PlusCGANS            |
+--------------------------------------------------------------------+

PURPOSE : This is the functional counterpart to the +CGANS= AT
          command to respond manual to a network request for
          PDP context activation.
*/
GLOBAL T_ACI_RETURN sAT_PlusCGANS ( T_ACI_CMD_SRC srcId, USHORT response, char *l2p, U8 cid )
{
  char L2P[MAX_L2P_LENGTH];
  T_PDP_CONTEXT           pdp_context;
  T_PDP_CONTEXT_INTERNAL *p_pdp_context_node = NULL;
  U8                      i                  = 0; 
  U8                      cid_to_activate    = PDP_CONTEXT_CID_INVALID;

  TRACE_FUNCTION ("sAT_PlusCGANS()");

  memcpy( &pdp_context, &pdp_context_default, sizeof( pdp_context ) );

/*
 *-------------------------------------------------------------------
 * check entity status
 *-------------------------------------------------------------------
 */
  if( smEntStat.curCmd NEQ AT_CMD_NONE )

    return( AT_BUSY );

/*
 *-------------------------------------------------------------------
 * check command source
 *-------------------------------------------------------------------
 */
  if(!cmh_IsVldCmdSrc (srcId))
  {
    ACI_ERR_DESC( ACI_ERR_CLASS_Ext, EXT_ERR_Parameter );
    return( AT_FAIL );
  }

/*
 *-------------------------------------------------------------------
 * check parameter
 *-------------------------------------------------------------------
 */
  if (response >= CGANS_RESPONSE_INVALID)
  {
    ACI_ERR_DESC( ACI_ERR_CLASS_Ext, EXT_ERR_Parameter );
    return( AT_FAIL );
  }

/*
 *-------------------------------------------------------------------
 * check call table
 *-------------------------------------------------------------------
 */
  if( gprs_ct_index EQ current_gprs_ct_index )
    return ( AT_FAIL );

/*
 *-------------------------------------------------------------------
 * stop ringing
 *-------------------------------------------------------------------
 */
  for( i = 0; i < CMD_SRC_MAX; i++ )
  {
    R_AT( RAT_CRING_OFF, (T_ACI_CMD_SRC)i )( 0 );
  }
#ifdef FF_ATI
   io_setRngInd ( IO_RING_OFF, CRING_SERV_TYP_NotPresent, CRING_SERV_TYP_NotPresent ); /* V.24 Ring Indicator Line */
#endif

/*
 *-------------------------------------------------------------------
 * process parameter
 *-------------------------------------------------------------------
 */
  switch( response )
  {
    case CGANS_RESPONSE_REJECT:
      psaSM_PDP_No_activate(gprs_call_table[current_gprs_ct_index].sm_ind.ti, CAUSE_NWSM_ACTIVATE_REJECTED_UNSPECIFIED);

      cmhSM_next_call_table_entry();

      return AT_CMPL;
      
    case CGANS_RESPONSE_ACCEPT:
     /*
      *-------------------------------------------------------------------
      * check number of context
      *-------------------------------------------------------------------
      */
#if defined(FF_PKTIO) OR defined(FF_TCP_IP)
      if( srcc_reserve_sources( SRCC_PKTIO_SNDCP_LINK, 1 ) EQ FALSE )
        return ( AT_FAIL );
#endif
     /*
      *-------------------------------------------------------------------
      * check the last two command arguments
      *-------------------------------------------------------------------
      */
      if( !gprs_call_table[current_gprs_ct_index].L2P[0] )
      {
        if ( l2p NEQ NULL )
        {
          if( !(*l2p) )
            strcpy(L2P, "PPP");
          else
          {
            strncpy(L2P, l2p, MAX_L2P_LENGTH - 1);
            L2P[MAX_L2P_LENGTH - 1] = 0;
          }
        }
        else
          strcpy(L2P, "PPP");

        if ( strcmp(L2P, "PPP") )
        {
          ACI_ERR_DESC( ACI_ERR_CLASS_Ext, EXT_ERR_Parameter );
          return( AT_FAIL );
        }

        cid_to_activate = cid;
      }
      else
      {
        cid_to_activate = (U8) gprs_call_table[current_gprs_ct_index].cid;
      }

      /* find first free position in work_cids array */
      i = 0;
      while( i < PDP_CONTEXT_CID_MAX AND work_cids[i] NEQ PDP_CONTEXT_CID_INVALID )
      {
        i++;
      }
  
      if( i >= PDP_CONTEXT_CID_MAX )
      {
        return AT_FAIL;
      }
  
      cid_pointer = i;

      switch( cid_to_activate )
      {
        case PDP_CONTEXT_CID_OMITTED:
          p_pdp_context_node = pdp_context_find_matching_node( 
                                  (T_SMREG_VAL_pdp_type) gprs_call_table[current_gprs_ct_index].sm_ind.pdp_type,
                                  gprs_call_table[current_gprs_ct_index].sm_ind.ctrl_ip_address,
                                 &gprs_call_table[current_gprs_ct_index].sm_ind.ip_address );
                                   
          if( p_pdp_context_node AND p_pdp_context_node->type EQ PDP_CONTEXT_TYPE_PRIMARY )                                            
          {
            cid_to_activate = p_pdp_context_node->cid;
            work_cids[cid_pointer] = cid_to_activate;
          }
          else
          {
            // No defined context found, with matching parameters.

            cid_to_activate = pdp_context_get_free_cid();

            // Define PDP context with default parameters.

            if( sAT_PlusCGDCONT( srcId, cid_to_activate, &pdp_context_default.attributes ) EQ AT_FAIL )
            {
              return AT_FAIL;
            }
            else
            {
              work_cids[cid_pointer] = cid_to_activate;
            }

          }
              
          break;
          
        case PDP_CONTEXT_CID_INVALID:
          break;
          
        default:
          switch( pdp_context_get_state_for_cid( cid_to_activate ) )
          {
            case PDP_CONTEXT_STATE_INVALID:
              /* define pdp context and                     */
              /* add pdp context cid to the work_cids array */

              if( sAT_PlusCGDCONT( srcId, cid_to_activate, &pdp_context_default.attributes ) EQ AT_FAIL )
              {
                return AT_FAIL;
              }
              else
              {
                work_cids[cid_pointer] = cid_to_activate;
              }
              break;

            case PDP_CONTEXT_STATE_DEFINED:
              /* add pdp context cid to the activation list */

              work_cids[cid_pointer] = cid_to_activate;
              break;
          }
      }

     /*
      *-------------------------------------------------------------------
      * set the actually context data
      *-------------------------------------------------------------------
      */
/*
      ctx.qos.preced    =  gprs_call_table[current_gprs_ct_index].sm_ind.smreg_qos.preced;
      ctx.qos.delay     =  gprs_call_table[current_gprs_ct_index].sm_ind.smreg_qos.delay;
      ctx.qos.relclass  =  gprs_call_table[current_gprs_ct_index].sm_ind.smreg_qos.relclass;
      ctx.qos.peak      =  gprs_call_table[current_gprs_ct_index].sm_ind.smreg_qos.peak;
      ctx.qos.mean      =  gprs_call_table[current_gprs_ct_index].sm_ind.smreg_qos.mean;
*/
      strncpy(pdp_context.pdp_apn, (const char *) gprs_call_table[current_gprs_ct_index].sm_ind.apn.apn_buf,
                                    gprs_call_table[current_gprs_ct_index].sm_ind.apn.c_apn_buf);

      /* IP v4 address */
      pdp_context.pdp_addr.ctrl_ip_address = gprs_call_table[current_gprs_ct_index].sm_ind.ctrl_ip_address;
      switch( gprs_call_table[current_gprs_ct_index].sm_ind.ctrl_ip_address)
      {
        case NAS_is_ipv4: /* IPv4 address */
          memcpy( &pdp_context.pdp_addr.ip_address.ipv4_addr.a4, &gprs_call_table[current_gprs_ct_index].sm_ind.ip_address.ipv4_addr.a4, NAS_SIZE_IPv4_ADDR );
/*          
          sprintf( ctx.pdp_addr, "%hd.%hd.%hd.%hd", 
                   gprs_call_table[current_gprs_ct_index].sm_ind.pdp_address.pdp_buf[0],
                   gprs_call_table[current_gprs_ct_index].sm_ind.pdp_address.pdp_buf[1],
                   gprs_call_table[current_gprs_ct_index].sm_ind.pdp_address.pdp_buf[2],
                   gprs_call_table[current_gprs_ct_index].sm_ind.pdp_address.pdp_buf[3]);
*/
          break;
        case NAS_is_ipv6: /* IPv6 address */
          memcpy( &pdp_context.pdp_addr.ip_address.ipv6_addr.a6, &gprs_call_table[current_gprs_ct_index].sm_ind.ip_address.ipv6_addr.a6, NAS_SIZE_IPv6_ADDR );
/*          
          sprintf( ctx.pdp_addr, "%hd.%hd.%hd.%hd.%hd.%hd.%hd.%hd.%hd.%hd.%hd.%hd.%hd.%hd.%hd.%hd",
                   gprs_call_table[current_gprs_ct_index].sm_ind.pdp_address.pdp_buf[ 0],
                   gprs_call_table[current_gprs_ct_index].sm_ind.pdp_address.pdp_buf[ 1],
                   gprs_call_table[current_gprs_ct_index].sm_ind.pdp_address.pdp_buf[ 2],
                   gprs_call_table[current_gprs_ct_index].sm_ind.pdp_address.pdp_buf[ 3],
                   gprs_call_table[current_gprs_ct_index].sm_ind.pdp_address.pdp_buf[ 4],
                   gprs_call_table[current_gprs_ct_index].sm_ind.pdp_address.pdp_buf[ 5],
                   gprs_call_table[current_gprs_ct_index].sm_ind.pdp_address.pdp_buf[ 6],
                   gprs_call_table[current_gprs_ct_index].sm_ind.pdp_address.pdp_buf[ 7],
                   gprs_call_table[current_gprs_ct_index].sm_ind.pdp_address.pdp_buf[ 8],
                   gprs_call_table[current_gprs_ct_index].sm_ind.pdp_address.pdp_buf[ 9],
                   gprs_call_table[current_gprs_ct_index].sm_ind.pdp_address.pdp_buf[10],
                   gprs_call_table[current_gprs_ct_index].sm_ind.pdp_address.pdp_buf[11],
                   gprs_call_table[current_gprs_ct_index].sm_ind.pdp_address.pdp_buf[12],
                   gprs_call_table[current_gprs_ct_index].sm_ind.pdp_address.pdp_buf[13],
                   gprs_call_table[current_gprs_ct_index].sm_ind.pdp_address.pdp_buf[14],
                   gprs_call_table[current_gprs_ct_index].sm_ind.pdp_address.pdp_buf[15]);
*/
          break;
        default:
          return( AT_FAIL );
      }

      switch( (T_SMREG_VAL_pdp_type) gprs_call_table[current_gprs_ct_index].sm_ind.pdp_type )
      {
        case SMREG_PDP_PPP:
          strcpy( pdp_context.pdp_type, "PPP" );
          break;
        case SMREG_PDP_IPV4:
          strcpy( pdp_context.pdp_type, "IP" );
          break;
        case SMREG_PDP_IPV6:
          strcpy( pdp_context.pdp_type, "IPV6" );
          break;
        default:
          return( AT_FAIL );
      }

     /*
      *-------------------------------------------------------------------
      * set some parameter of the call table
      *-------------------------------------------------------------------
      */
      if ( !gprs_call_table[current_gprs_ct_index].L2P[0])
      {
        /*lint -e{645} */ /* L2P is initialized within the same if-construct some lines above */
        strcpy (gprs_call_table[current_gprs_ct_index].L2P, L2P);
        gprs_call_table[current_gprs_ct_index].cid = cid_to_activate;
      }

      sAT_PlusCGDCONT_exec( cid_to_activate, &pdp_context );
      
      p_pdp_context_node = pdp_context_find_node_from_cid( cid_to_activate );
      if( p_pdp_context_node )
      {
        p_pdp_context_node->internal_data.smreg_ti = gprs_call_table[current_gprs_ct_index].sm_ind.ti;
      }
      else
      {
        TRACE_ERROR( "ERROR: PDP context not found, in function sAT_PlusCGANS" );
      }

     /*
      *-------------------------------------------------------------------
      * process function
      *-------------------------------------------------------------------
      */
      gpppEntStat.curCmd = AT_CMD_CGDATA;
      gpppEntStat.entOwn = srcId;
      gpppShrdPrm.owner  = (UBYTE) srcId;

      smShrdPrm.direc = CGEREP_EVENT_NW_ACT;

      set_conn_param_on_working_cid( (UBYTE)srcId, DTI_ENTITY_PPPS );

      cmhSM_data_link_context();
      return AT_EXCT;
  }

  return AT_FAIL;
}

GLOBAL T_ACI_RETURN sAT_PlusCGEREP  ( T_ACI_CMD_SRC srcId, T_CGEREP_MODE mode, T_CGEREP_BFR bfr )
{

  TRACE_FUNCTION ("sAT_PlusCGEREP()");

/*
 *-------------------------------------------------------------------
 * check entity status
 *-------------------------------------------------------------------
 */
  if( smEntStat.curCmd NEQ AT_CMD_NONE )

    return( AT_BUSY );

/*
 *-------------------------------------------------------------------
 * check first command argument
 *-------------------------------------------------------------------
 */
  if ( mode < CGEREP_MODE_OMITTED OR mode >= CGEREP_MODE_INVALID )
  {
    ACI_ERR_DESC( ACI_ERR_CLASS_Ext, EXT_ERR_Parameter );
    return( AT_FAIL );
  }

  if ( bfr < CGEREP_BFR_OMITTED OR bfr >= CGEREP_BFR_INVALID )
  {
    ACI_ERR_DESC( ACI_ERR_CLASS_Ext, EXT_ERR_Parameter );
    return( AT_FAIL );
  }

/*
 *-------------------------------------------------------------------
 * process parameter
 *-------------------------------------------------------------------
 */

  /* only the last source which used CGEREP will receive those indications */
  sm_cgerep_srcId = srcId;

  if ( mode NEQ CGEREP_MODE_OMITTED )
  sm_cgerep_mode = mode;

  if ( bfr NEQ CGEREP_BFR_OMITTED )
  sm_cgerep_bfr  = bfr;

  switch ( mode )
  {
    case CGEREP_MODE_BUFFER:
    case CGEREP_MODE_DICARD_RESERVED:
      cmhSM_cgerep_buffer ( );
      break;
    case CGEREP_MODE_BUFFER_RESERVED:
      break;
    case CGEREP_MODE_INVALID:
    case CGEREP_MODE_OMITTED:
    default:
      break;
  }

  return AT_CMPL;
}

#ifdef DTI
GLOBAL T_ACI_RETURN sAT_PlusCGSMS   ( T_ACI_CMD_SRC srcId, T_CGSMS_SERVICE service )
{
  T_ACI_RETURN    retCd = AT_CMPL;    /* holds return code */

  TRACE_FUNCTION ("sAT_PlusCGSMS()");

/*
 *-------------------------------------------------------------------
 * check entity status
 *-------------------------------------------------------------------
 */
  if( smEntStat.curCmd NEQ AT_CMD_NONE )

    return( AT_BUSY );

/*
 *-------------------------------------------------------------------
 * check first command argument
 *-------------------------------------------------------------------
 */
  if ( service < CGSMS_SERVICE_OMITTED OR service >= CGSMS_SERVICE_INVALID )
  {
    ACI_ERR_DESC( ACI_ERR_CLASS_Ext, EXT_ERR_Parameter );
    return( AT_FAIL );
  }

/*
 *-------------------------------------------------------------------
 * process parameter
 *-------------------------------------------------------------------
 */
  if ( service EQ CGSMS_SERVICE_OMITTED )
    service = sm_cgsms_service;

  if ( service NEQ sm_cgsms_service )
  {
    smEntStat.curCmd = AT_CMD_CGSMS;
    smEntStat.entOwn = srcId;
    smShrdPrm.owner  = (UBYTE) srcId;

    cmhSM_set_sms_service ( service );

    retCd = AT_EXCT;
  }

  return retCd;
}
#endif
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (8441)               MODULE  : CMH_SMS            |
| STATE   : finished                    ROUTINE : string_to_dns      |
+--------------------------------------------------------------------+

PURPOSE : 
*/
LOCAL void string_to_dns(CHAR* dns, ULONG *dns_long)
{
     UBYTE dns_len = 4;
     CHAR dns_adrtest[3];
     UBYTE dns_adr [4];
     UBYTE i = 0;

     memset(&dns_adrtest,0,dns_len-1);
     memset(&dns_adr,0,dns_len);

     if(strlen(dns) NEQ 0)
    {
        for(i=0;i<dns_len;i++)
        {
          strncpy(dns_adrtest,dns,dns_len-1);
          dns_adr[i] = (UBYTE)atoi(dns_adrtest);
          dns = dns+dns_len;
         }
         for(i=0;i<dns_len;i++)
         {
           *dns_long= *dns_long + dns_adr[i];
           if (i<(dns_len-1))
             *dns_long = *dns_long<<8;
        }
    }    
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (8441)         MODULE  : CMH_SMS                  |
| STATE   : finished              ROUTINE : sAT_PercentCGPCO_HEX     |
+--------------------------------------------------------------------+

PURPOSE : This is the functional counterpart to the ?CGPCO= AT
          command to set protocol configuration options for the
          PDP context activation.
*/

GLOBAL T_ACI_RETURN sAT_PercentCGPCO_HEX ( T_ACI_CMD_SRC srcId, UBYTE cid, UBYTE *pco_array, UBYTE pco_len)
{
  T_ACI_RETURN ret;

  TRACE_FUNCTION("sAT_PercentCGPCO_HEX");
   /*
    *-------------------------------------------------------------------
    * check command source
    *-------------------------------------------------------------------
   */
   if ( !cmh_IsVldCmdSrc (srcId) )
   {
     ACI_ERR_DESC( ACI_ERR_CLASS_Ext, EXT_ERR_Parameter );
     return( AT_FAIL );
   }
     ret = cmhSM_CGPCO_HEX (cid, pco_array, pco_len);
  return ret;
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (8441)         MODULE  : CMH_SMS                  |
| STATE   : finished              ROUTINE : sAT_PercentCGPCO         |
+--------------------------------------------------------------------+

PURPOSE : This is the functional counterpart to the ?CGPCO= AT
          command to set protocol configuration options for the
          PDP context activation.
*/

GLOBAL T_ACI_RETURN sAT_PercentCGPCO( T_ACI_CMD_SRC srcId, 
                                      U8 cid, USHORT protocol, 
                                      CHAR *user, CHAR *pwd, CHAR *dns1, CHAR *dns2 )
{
  U8      pco_len = ACI_PCO_MAX_LEN;
  UBYTE   *pco_array;
  T_ACI_RETURN ret;


  /*
   * Due to introduction of dynamic data structures the functionality of the command is changed.
   * NOTE: User PCO is only applied to already defined PDP contexts.
   */
  ULONG dns_adr1 = 0x00000000;
  ULONG dns_adr2 = 0x00000000;


  TRACE_FUNCTION("sAT_PercentCGPCO");
   /*
    *-------------------------------------------------------------------
    * check command source
    *-------------------------------------------------------------------
   */
   if ( !cmh_IsVldCmdSrc (srcId) )
   {
     ACI_ERR_DESC( ACI_ERR_CLASS_Ext, EXT_ERR_Parameter );
     return( AT_FAIL );
   }
  
  ACI_MALLOC (pco_array, ACI_PCO_MAX_LEN);

  string_to_dns( dns1, &dns_adr1 );
  string_to_dns( dns2, &dns_adr2 );
  
  ret = (T_ACI_RETURN) utl_create_pco( pco_array, (USHORT*)&pco_len,
                        ACI_PCO_CONTENTMASK_AUTH | 
                        ACI_PCO_CONTENTMASK_DNS1 |
                        ACI_PCO_CONTENTMASK_DNS2,
                        ACI_PCO_CONFIG_PROT_PPP,
                        protocol, (UBYTE*)user, (UBYTE*)pwd, dns_adr1, dns_adr2);
  if (ret < 0)
  {
    TRACE_EVENT_P1 ("sAT_PercentCGPCO(): invalid protocol=%d", protocol);

    ACI_MFREE (pco_array);
    return (AT_FAIL);
  }
  ret = cmhSM_CGPCO_HEX (cid, pco_array, pco_len);
  ACI_MFREE (pco_array);
  return ret;
}

/*
+--------------------------------------------------------------------+
| PROJECT : GPRS                         MODULE  : CMH_SMS           |
| STATE   : finished                     ROUTINE : qAT_PercentCGPCO  |
+--------------------------------------------------------------------+

  PURPOSE : %CGPCO command
              * analyze network PCO a cid
*/

GLOBAL T_ACI_RETURN qAT_PercentCGPCO ( T_ACI_CMD_SRC srcId, ULONG * gateway, 
                                       ULONG * dns1,ULONG * dns2, USHORT cid)
{  
  T_PDP_CONTEXT_INTERNAL *p_pdp_context_node = NULL;


  TRACE_FUNCTION("qAT_PercentCGPCO()");

  p_pdp_context_node = pdp_context_find_node_from_cid( (U8)cid);
  if( !p_pdp_context_node)
  {
    return (AT_FAIL);
  }

  switch(p_pdp_context_node->internal_data.state)
  {
     case PDP_CONTEXT_STATE_ACTIVATED:
     case PDP_CONTEXT_STATE_ESTABLISH_3:
     case PDP_CONTEXT_STATE_DATA_LINK:
       utl_analyze_pco(p_pdp_context_node->internal_data.network_pco.pco, p_pdp_context_node->internal_data.network_pco.len, dns1, dns2, gateway);
          break;
     default:
          break;
   }
   return (AT_CMPL);
}

#ifdef REL99
/*
+--------------------------------------------------------------------+
| PROJECT : UMTS                  MODULE  : CMH_SMS                  |
| STATE   : finished              ROUTINE : sAT_PlusCGEQNEG          |
+--------------------------------------------------------------------+

  PURPOSE : This is the functional counterpart to the +CGEQNEG= AT
            command and returns current settings for the specified
            PDP context. The function might be called multiple times
            if more cids are specified.
            The QoS returned is always in Release 99 format (3GPP).
  RETURNS:  - AT_CMPL : Completed.
            - AT_FAIL : Command not valid for srcId.
  UPDATES:  - qos: Quality of service for cid. Not updated if cid is undefined.
            - qos_valid: Indicates whether qos is updated not.
*/
GLOBAL T_ACI_RETURN sAT_PlusCGEQNEG ( T_ACI_CMD_SRC srcId, U8 cid, BOOL *qos_valid, T_PS_qos *qos)
{
  T_PDP_CONTEXT_INTERNAL *p_pdp_context_node = NULL;
  TRACE_FUNCTION ("sAT_PlusCGEQNEG()");

/*
 *-------------------------------------------------------------------
 * check command source - should be Serial link ?
 *-------------------------------------------------------------------
 */
   if ( !cmh_IsVldCmdSrc (srcId) )
   {
     ACI_ERR_DESC( ACI_ERR_CLASS_Ext, EXT_ERR_Parameter );
     return( AT_FAIL );
   }

   p_pdp_context_node = pdp_context_find_node_from_cid( cid );
   if( ! p_pdp_context_node )
   {
     *qos_valid = FALSE;
     return AT_CMPL;
   }

/*
 *-------------------------------------------------------------------
 * fill in parameters
 *-------------------------------------------------------------------
 */
  switch ( get_state_over_cid(cid) )
  {
    case PDP_CONTEXT_STATE_ACTIVATED:
    case PDP_CONTEXT_STATE_DATA_LINK:
    {
      if( p_pdp_context_node->ctrl_neg_qos EQ PS_is_R99 )
      {
        memcpy( qos, &p_pdp_context_node->neg_qos, sizeof(T_PS_qos) ); 
      }
      else
      {
        /* The QoS is in Release 97 format and must be converted first. */    
        if( !cl_qos_convert_r97_to_r99( &p_pdp_context_node->neg_qos.qos_r97, &(qos->qos_r99)) )
        {
          /* Failed to convert to Release 99. Never end here !!!! */
          return( AT_FAIL );
        }
      }
      *qos_valid = TRUE;
      break;
    }
    default :
      *qos_valid = FALSE;
  }

  return( AT_CMPL );
}

/*
+--------------------------------------------------------------------+
| PROJECT : UMTS                  MODULE  : CMH_SMS                  |
| STATE   : finished              ROUTINE : sAT_PlusCGCMOD           |
+--------------------------------------------------------------------+

  PURPOSE : This is the functional counterpart to the +CGCMOD= AT
            command.
  RETURNS:  - AT_EXCT : Executing.
            - AT_FAIL : One of the cids is not valid or not active.
*/
GLOBAL T_ACI_RETURN sAT_PlusCGCMOD ( T_ACI_CMD_SRC srcId, U8 *cid)
{


  TRACE_FUNCTION ("sAT_PlusCGCMOD()");

/*
 *-------------------------------------------------------------------
 * check command source
 *-------------------------------------------------------------------
 */
   if ( !cmh_IsVldCmdSrc (srcId) )
   {
     ACI_ERR_DESC( ACI_ERR_CLASS_Ext, EXT_ERR_Parameter );
     return( AT_FAIL );
   }

/*
 *-------------------------------------------------------------------
 * check entity status
 *-------------------------------------------------------------------
 */
  if( smEntStat.curCmd NEQ AT_CMD_NONE )
    return( AT_BUSY );

  if( gpppEntStat.curCmd NEQ AT_CMD_NONE )
    return( AT_BUSY );

/*
 *-------------------------------------------------------------------
 * check all context and fill work_cids list
 *-------------------------------------------------------------------
 */
  if (!cmhSM_make_active_cid_list ( srcId, cid ))
    return( AT_FAIL );

/*
 *-------------------------------------------------------------------
 * fill in parameters command parameters and send the modify request
 *-------------------------------------------------------------------
 */
  switch( get_state_over_cid(work_cids[cid_pointer]) )
  {
    case PDP_CONTEXT_STATE_ACTIVATED:
      set_state_over_cid(work_cids[cid_pointer], PDP_CONTEXT_STATE_ACTIVATED_MODIFYING);
      break;

    case PDP_CONTEXT_STATE_DATA_LINK:
      set_state_over_cid(work_cids[cid_pointer], PDP_CONTEXT_STATE_DATA_LINK_MODIFYING);
      break;

    default:
      return( AT_FAIL ); /* Obsolete: State already checked in cmhSM_make_active_cid_list */
  }
 
  /* Send the modify request */
  psaSM_PDP_Modify();
  smEntStat.curCmd = AT_CMD_CGCMOD;
  smEntStat.entOwn = srcId;
  smShrdPrm.owner  = (UBYTE) srcId;
  return( AT_EXCT );
}

#endif /* REL99 */

#endif  /* GPRS */
/*==== EOF ========================================================*/
