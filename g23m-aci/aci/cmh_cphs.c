/*
+--------------------------------------------------------------------+
| PROJECT:                              $Workfile:: cmh_cphs.c      $|
| $Author::                             $Revision::                 $|
| CREATED:                              $Modtime::                  $|
| STATE  : code                                                      |
+--------------------------------------------------------------------+

   MODULE  : CMH

   PURPOSE : CPHS related ACI functions.
*/


#ifndef CMH_CPHS_C
#define CMH_CPHS_C

#include "config.h"
#include "fixedconf.h"
#include "condat-features.h"
#include "aci_conf.h"

#include "aci_all.h"
#include "aci_cmh.h"
#include "psa.h"
#include "cmh.h"

#include "cphs.h"
#include "aci_cphs.h"

#include "ksd.h"
#include "cmh_ss.h"

#include "phb.h"
#include "cmh_phb.h" 

typedef enum
{
  CMN_FULL_PRSNT,   /* all parameter present                       */
  CMN_PRTLY_PRSNT,  /* only some parameter present                 */
  CMN_NOT_PRSNT     /* no parameter present                        */
}
T_CPHS_MB_CMH_CMN_STAT;

typedef enum
{
  SNGL_VLD_PRSNT,   /* parameter is present and in valid range     */
  SNGL_INVLD_PRSNT, /* parameter is present and not in valid range */
  SNGL_NOT_PRSNT    /* parameter is not present                    */
}
T_CPHS_MB_CMH_SNGL_STAT;

typedef struct CPHSShrdParm
{
  T_ACI_CMD_SRC srcId;

} T_CPHS_SHRD_PRM;

LOCAL T_CPHS_SHRD_PRM cphsShrdPrm;

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_CPHS                 |
| STATE   : code                  ROUTINE : cmhCPHS_user_cb          |
+--------------------------------------------------------------------+

  PURPOSE : This is the callback used by the CPHS module to inform ACI
            of the result of an asynchroneous operation.
*/
GLOBAL void cmhCPHS_user_cb( T_CPHS_PARAMS *params )
{
  T_ACI_CMD_SRC cphsId = cphsShrdPrm.srcId;
  int i;
  BOOL final_result = FALSE;
  
  /* Deal with Final Results */
  switch(params->cb_type)
  {
  case(CPHS_INIT_RES):
    final_result = TRUE;
    break;

  case(CPHS_VOICE_MAIL_RES):
    final_result = TRUE;
    break;

  case(CPHS_CFU_RES):
    final_result = TRUE;
    break;
  }
    
  if(final_result)
  {
    if(params->operation_result EQ CPHS_OK)
    {
      R_AT( RAT_OK, cphsId )
        ( AT_CMD_CPHS );
    }
    else
    {
      R_AT( RAT_CME, cphsId )
        ( AT_CMD_CPHS, CME_ERR_NotPresent );
    }

    /* reset cphsId */
    cphsShrdPrm.srcId = CMD_SRC_NONE;
    return;
  }


  /* Deal with Unsolicited Messages */
  switch(params->cb_type)
  {
  case(CPHS_VOICE_MAIL_IND):
    for (i=0; i < CMD_SRC_MAX; i++)
    {
      R_AT( RAT_CPVWI, (T_ACI_CMD_SRC)i ) (params->set_flag, params->line);
    }
    return;

  case(CPHS_ROAM_IND):
    for (i=0; i < CMD_SRC_MAX; i++)
    {
      R_AT( RAT_CPROAM, (T_ACI_CMD_SRC)i ) (params->set_flag);
    }
    return;
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)        MODULE  : CMH_CPHS                 |
| STATE   : code                  ROUTINE : cmhCPHS_convertSimTag      |
+--------------------------------------------------------------------+

  PURPOSE : This function converts the actual tag string in the 
            format used when storing it on the SIM.
*/
LOCAL void cmhCPHS_convertSimTag ( T_CPHS_PB_TEXT*  inTag,
                                    UBYTE*           outTag,
                                    UBYTE*           outTagLen,
                                    UBYTE            maxOutLen )
{
  UBYTE i       = MINIMUM ( maxOutLen, inTag->len);
  SHORT restLen = maxOutLen - inTag->len;
  UBYTE j;

  /*
   *-----------------------------------------------------------------
   * Convert from internal GSM to default GSM alphabet
   *-----------------------------------------------------------------
   */
  for ( j = 0; j < i; j++ )
    outTag[j] = inTag->data[j];
  
  *outTagLen = i;

  if ( restLen > 0 )
    memset ( &outTag[i], NOT_PRESENT_8BIT, restLen);
}

/*
+----------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : CMH_CPHS              |
| STATE   : code                        ROUTINE : cmhCPHS_add_mb_record|  
+----------------------------------------------------------------------+

  PURPOSE : %CPMBW: write mailbox numbers to ME cache and SIM 
*/

LOCAL T_ACI_RETURN cmhCPHS_add_mb_record(  T_ACI_CMD_SRC           srcId,
                                           T_CPHS_MB_CMH_SNGL_STAT indexStat, 
                                           T_CPHS_PB_TEXT*         text, 
                                           CHAR*                   number, 
                                           T_ACI_TOA*              type, 
                                           SHORT                   index)
{
  
  T_PHB_RECORD entry; /* reuse of phonebook type */

  
  CHAR     *pNumber = NULL; /* pointer to the number that will be saved */
  T_CPHS_RET cphs_ret;

  memset(&entry,0,sizeof(T_PHB_RECORD));
  
  TRACE_FUNCTION ("cmhCPHS_add_mb_record()");
  
  switch ( indexStat )
  {
    case ( SNGL_NOT_PRSNT ):
    case ( SNGL_VLD_PRSNT ):
        {
          /* fill in the structure elements */
  
          /* process the <index> */
#ifdef TI_PS_FFS_PHB
          entry.phy_recno = ( indexStat EQ SNGL_NOT_PRSNT ? 
                                             0 : ( UBYTE ) index );
#else
          entry.index = ( indexStat EQ SNGL_NOT_PRSNT ? 
                                             0 : ( UBYTE ) index );
#endif
          
          /* process the <tag> */
          if (text NEQ NULL)
          {
            if (text->len > PHB_MAX_TAG_LEN)
            {
              TRACE_FUNCTION_P2("ERROR: text length (%d) > max tag length (%d)",
                               text->len, PHB_MAX_TAG_LEN);
              ACI_ERR_DESC( ACI_ERR_CLASS_Cme, CME_ERR_TxtToLong );
              return (AT_FAIL);
            }
            cmhCPHS_convertSimTag( text, entry.tag, &(entry.tag_len),
                                   (UBYTE)PHB_MAX_TAG_LEN );
          }

          /* calculate the TON/NPI field */
          entry.ton_npi = PHB_TONPI_NO_DIAL;
          pNumber       = number;
  
          if ( type EQ NULL )
          {
            cmhPHB_ksdDecodeToa(number, &pNumber, &entry);
          }
          else
          {
            cmhPHB_toaMrg ( type, &entry.ton_npi );
          }
  
          /* process the <number>, convert to BCD */
          if ( pNumber NEQ NULL)
          {
            cmhPHB_getAdrBcd ( entry.number, &entry.len,
                               PHB_PACKED_NUM_LEN, pNumber );
          }
          /*********************/ 
          /* update the record */
          /*********************/ 

          /* in SIM & Cache */
          cphs_ret = cphs_write_mb_number(srcId,
#ifdef TI_PS_FFS_PHB
                                   (UBYTE)entry.phy_recno,
#else
                                          entry.index,
#endif
                                          entry.tag,
                                          entry.tag_len,
                                          entry.len,
                                          entry.number,
                                          entry.ton_npi);

          /* map return value */
          switch (cphs_ret)
          {
            case CPHS_EXEC:
              return (AT_EXCT);
            case CPHS_OK:
              return (AT_CMPL);
            case CPHS_NOT_INIT:              
              ACI_ERR_DESC( ACI_ERR_CLASS_Cme, CME_ERR_OpNotAllow );
              return (AT_FAIL);
            case CPHS_BUSY:
              ACI_ERR_DESC( ACI_ERR_CLASS_Cme, CME_ERR_SimBusy );
              return (AT_FAIL);
            case CPHS_FAIL:
              ACI_ERR_DESC( ACI_ERR_CLASS_Cme, CME_ERR_TxtToLong );
              return (AT_FAIL);
            default:
              ACI_ERR_DESC( ACI_ERR_CLASS_Cme, CME_ERR_Unknown );
              return (AT_FAIL);
          }
        }
  
      case ( SNGL_INVLD_PRSNT ):
        ACI_ERR_DESC( ACI_ERR_CLASS_Ext, EXT_ERR_Parameter );
        return( AT_FAIL );
    }
    return(AT_FAIL);  
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_CPHS                 |
| STATE   : code                  ROUTINE : sAT_PercentCPHS          |
+--------------------------------------------------------------------+

  PURPOSE : This is the functional counterpart to the %CPHS AT command
            which is responsible for initialise/close/refresh the CPHS module.
*/

GLOBAL T_ACI_RETURN sAT_PercentCPHS( T_ACI_CMD_SRC   srcId,
                                     T_ACI_CPHS_INIT init_cphs )
{
  T_CPHS_RET cphs_ret = CPHS_FAIL;
  
  TRACE_FUNCTION ("sAT_PercentCPHS");

  /* check command source */
  if(!cmh_IsVldCmdSrc (srcId))
  {
    return( AT_FAIL );
  }

  /* check parameter <init_cphs> */
  switch(init_cphs)
  {
  case(ACI_CPHS_CLOSE):
    cphs_ret = cphs_stop( );
    break;

  case(ACI_CPHS_INIT):
    cphs_ret = cphs_start( cmhCPHS_user_cb );
    break;

  case(ACI_CPHS_REFRESH):
    cphs_ret = cphs_refresh_data( );
    break;

  default:
    ACI_ERR_DESC( ACI_ERR_CLASS_Ext, EXT_ERR_Parameter );
    break;
  }

  switch(cphs_ret)
  {
  case(CPHS_OK):
    return(AT_CMPL);

  case(CPHS_EXEC):
    cphsShrdPrm.srcId = srcId;
    return(AT_EXCT);

  case(CPHS_BUSY):
    return(AT_BUSY);

  default:
    return(AT_FAIL);
  }
}


GLOBAL T_ACI_RETURN qAT_PercentCPHS( T_ACI_CMD_SRC   srcId,
                                     T_ACI_CPHS_INIT *init_cphs )
{
  T_CPHS_RET cphs_ret;

  TRACE_FUNCTION ("qAT_PercentCPHS");

  /* check command source */
  if(!cmh_IsVldCmdSrc (srcId))
  {
    return( AT_FAIL );
  }

  cphs_ret = cphs_check_status( );

  switch(cphs_ret)
  {
  case(CPHS_OK):
    *init_cphs = ACI_CPHS_INIT;
    break;

  case(CPHS_BUSY):
    *init_cphs = ACI_CPHS_BUSY;
    break;

  case(CPHS_NOT_INIT):
    *init_cphs = ACI_CPHS_CLOSE;
    break;
  }
  
  return( AT_CMPL );
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_CPHS                 |
| STATE   : code                  ROUTINE : sAT_PercentCPNUMS        |
+--------------------------------------------------------------------+

  PURPOSE : This is the functional counterpart to the %CPNUMS AT command
            which is responsible for managing the CPHS information numbers feature.
*/

LOCAL T_ACI_RETURN cmhCPHS_get_info_number_element( T_ACI_CMD_SRC srcId,
                                                    UBYTE         element_id )
{
  T_CPHS_RET     cphs_ret;
  T_CPHS_INF_NUM info_number;   /* dynamic ??? */
  
  TRACE_FUNCTION ("cmhCPHS_get_info_number_element( )");

  cphs_ret = cphs_read_info_nb (element_id, &info_number);

  if(cphs_ret NEQ CPHS_OK)
  {
    TRACE_ERROR("cmhCPHS_get_info_number_element: unexpected error");
    return(AT_FAIL);
  }

  R_AT( RAT_CPNUMS, srcId )
    (element_id,
     info_number.index_level,
     info_number.alpha_tag,
     info_number.number,
     info_number.premium_flag,
     info_number.network_flag,
     info_number.type_of_address);

  return(AT_CMPL);
}

LOCAL T_ACI_RETURN cmhCPHS_get_folder_elements( T_ACI_CMD_SRC srcId,
                                                UBYTE         element_id )
{
  T_CPHS_RET     cphs_ret;
  UBYTE          *info_nums_list;
  UBYTE          list_size;
  UBYTE          i;
  T_ACI_RETURN   ret;
  
  TRACE_FUNCTION ("cmhCPHS_get_folder_elements( )");

  list_size = 10;         /* first try with 10 entries */
  MALLOC(info_nums_list, list_size); 
  cphs_ret = cphs_explore_info_nbs(element_id, info_nums_list, &list_size);

  TRACE_EVENT_P3("cphs_ret: %d, info_nums_list: %s, list_size: %d", cphs_ret, info_nums_list, list_size);

  switch(cphs_ret)
  {
  case(CPHS_OK):
    if((*info_nums_list EQ 0) AND (list_size NEQ 0))
    {
      TRACE_EVENT_P1("Folder element %d is empty", element_id);
      return(AT_FAIL);
    }
    break;

  case(CPHS_EXEC):
    /* list was too small: size needed has been written in list_size */
    TRACE_EVENT_P1("List was too small: needed: %d bytes", list_size);
    MFREE(info_nums_list);

    MALLOC(info_nums_list, list_size); 
    cphs_ret = cphs_explore_info_nbs(element_id, info_nums_list, &list_size);

    if(cphs_ret NEQ CPHS_OK)
    {
      TRACE_ERROR("cmhCPHS_get_folder_elements: unexpected error");
      MFREE(info_nums_list);
      return(AT_FAIL);
    }
    break;

  default:
    MFREE(info_nums_list);
    return(AT_FAIL);
  }

  /* Get all elements of folder and send to user */
  for(i=0;i<list_size;i++)
  {
    ret = cmhCPHS_get_info_number_element(srcId, info_nums_list[i]);

    if(ret NEQ AT_CMPL)
    {
      TRACE_EVENT_P1("Wrong Information Number: index ignored: %d...", info_nums_list[i]);
    }
  }
  MFREE(info_nums_list);
  return(AT_CMPL);
}

GLOBAL T_ACI_RETURN sAT_PercentCPNUMS( T_ACI_CMD_SRC srcId,
                                       UBYTE         element_id,
                                       UBYTE         mode )
{
  T_ACI_RETURN   ret;
  
  TRACE_FUNCTION ("sAT_PercentCPNUMS");

  TRACE_EVENT_P1("elemt: %d", element_id);

  /* check command source */
  if(!cmh_IsVldCmdSrc (srcId))
  {
    return( AT_FAIL );
  }

  /* check parameter <mode> */
  switch(mode)
  {
  case(CPNUMS_MODE_EXPLORE):
    TRACE_EVENT_P1("CPNUMS mode exploring: folder: %d", element_id);
    return(cmhCPHS_get_folder_elements(srcId, element_id));

  case(CPNUMS_MODE_QUERY):
    TRACE_EVENT_P1("CPNUMS mode querying: element: %d", element_id);

    ret = cmhCPHS_get_info_number_element(srcId, element_id);

    if(ret NEQ AT_CMPL)
    {
      TRACE_ERROR("sAT_PercentCPNUMS: unexpected error");
      return(AT_FAIL);
    }
    return(AT_CMPL);

  default:
    ACI_ERR_DESC( ACI_ERR_CLASS_Ext, EXT_ERR_Parameter );
    return(AT_FAIL);
  }
}

/* returns whole table */
GLOBAL T_ACI_RETURN tAT_PercentCPNUMS( T_ACI_CMD_SRC srcId )
{
  T_CPHS_RET   cphs_ret;
  T_ACI_RETURN ret;
  UBYTE        i;
  UBYTE        max_index;

  TRACE_FUNCTION ("tAT_PercentCPNUMS");

  /* check command source */
  if(!cmh_IsVldCmdSrc (srcId))
  {
    return( AT_FAIL );
  }

  cphs_ret = cphs_info_num_get_max(&max_index);

  if(cphs_ret NEQ CPHS_OK)
  {
    return(AT_FAIL);  
  }

  for(i=1; i<(max_index+1); i++)   
  {
    ret = cmhCPHS_get_info_number_element(srcId, i);

    if(ret NEQ AT_CMPL)
    {
      return(AT_FAIL);
    }
  }
  return( AT_CMPL );
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_CPHS                 |
| STATE   : code                  ROUTINE : qAT_PercentCPALS         |
+--------------------------------------------------------------------+

  PURPOSE : This is the functional counterpart to the %CPALS AT command
            which is responsible for managing the CPHS alternate line service feature.
*/

GLOBAL T_ACI_RETURN qAT_PercentCPALS( T_ACI_CMD_SRC srcId, 
                                      UBYTE         call_id, 
                                      T_CPHS_LINES  *line, 
                                      CHAR          *line_desc, 
                                      UBYTE         *max_line_desc)
{
  T_CPHS_RET ret_val; 
  TRACE_FUNCTION ("qAT_PercentCPALS");

  /* check command source */
  if(!cmh_IsVldCmdSrc (srcId))
  {
    return( AT_FAIL );
  }

  ret_val=cphs_get_line((UBYTE)srcId, call_id, line, line_desc, max_line_desc);
  switch(ret_val)
  {
     case(CPHS_OK):     return(AT_CMPL);
     case(CPHS_EXEC):  return(AT_EXCT);
     case(CPHS_BUSY):  
     case(CPHS_NOT_INIT): return(AT_BUSY);
     case(CPHS_FAIL):
     default: return(AT_FAIL);
  }
  	
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_CMD            |
| STATE   : code                        ROUTINE : AT_PercentCPVWI    |
+--------------------------------------------------------------------+

  PURPOSE : %CPVWI: set/clear/query voice message waiting flags
*/

GLOBAL T_ACI_RETURN sAT_PercentCPVWI( T_ACI_CMD_SRC srcId,
                                      UBYTE         flag_set, 
                                      USHORT        lines)
{
  T_CPHS_RET cphs_ret     = CPHS_OK;
  
  TRACE_FUNCTION ("sAT_PercentCPVWI()");

  /* check command source */
  if(!cmh_IsVldCmdSrc (srcId))
  {
    return( AT_FAIL );
  }

  /* check parameter <flag_set> */
  switch(flag_set)
  {
  case(CPHS_SET_WAITING_FLAG):
  case(CPHS_ERASE_WAITING_FLAG):
    cphs_ret = cphs_set_waiting_flag(flag_set, lines);
    break;

  default:
    ACI_ERR_DESC( ACI_ERR_CLASS_Ext, EXT_ERR_Parameter );
    return(AT_FAIL);
  }

  switch(cphs_ret)
  {
  case(CPHS_EXEC):
    cphsShrdPrm.srcId = srcId;
    return(AT_EXCT);

  case(CPHS_BUSY):
    return(AT_BUSY);

  default:
    return(AT_FAIL);
  }
}


GLOBAL T_ACI_RETURN qAT_PercentCPVWI( T_ACI_CMD_SRC srcId,
                                      UBYTE         *flag_set, 
                                      USHORT        line)
{
  T_CPHS_RET cphs_ret;
  
  TRACE_FUNCTION ("qAT_PercentCPVWI()");

  /* check command source */
  if(!cmh_IsVldCmdSrc (srcId))
  {
    return( AT_FAIL );
  }

  cphs_ret = cphs_get_waiting_flag(flag_set, line);

  switch(cphs_ret)
  {
  case(CPHS_OK):
    return(AT_CMPL);

  case(CPHS_BUSY):
    *flag_set = CPHS_FLAG_ERROR;
    return(AT_BUSY);

  default:
    *flag_set = CPHS_FLAG_ERROR;
    return(AT_FAIL);
  }

}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_CMD            |
| STATE   : code                        ROUTINE : AT_PercentCPOPN    |
+--------------------------------------------------------------------+

  PURPOSE : %CPOPN: query operator name long and short string
*/

GLOBAL T_ACI_RETURN qAT_PercentCPOPN( T_ACI_CMD_SRC srcId,
                                      CHAR         *longname, 
                                      UBYTE        *max_longname,
                                      CHAR         *shortname, 
                                      UBYTE        *max_shortname)
{
  T_CPHS_RET cphs_ret;
  
  TRACE_FUNCTION ("qAT_PercentCPOPN()");

  /* check command source */
  if(!cmh_IsVldCmdSrc (srcId))
  {
    return( AT_FAIL );
  }
  
  cphs_ret = cphs_get_opn( longname, max_longname, shortname, max_shortname);

  switch(cphs_ret)
  {
  case(CPHS_OK):
    return(AT_CMPL);

  default:
    return(AT_FAIL);
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_CMD            |
| STATE   : code                        ROUTINE : AT_PercentCPINF    |
+--------------------------------------------------------------------+

  PURPOSE : %CPINF: write CSP ( Customer Service Profile )
*/

GLOBAL T_ACI_RETURN sAT_PercentCPINF( T_ACI_CMD_SRC srcId, 
                                      UBYTE         *csp,
                                      UBYTE         csp_len)
{
  UBYTE  phase;
  USHORT sst;
  T_CPHS_RET cphs_ret;

  TRACE_FUNCTION ("sAT_PercentCPINF()");

  /* check command source */
  if(!cmh_IsVldCmdSrc (srcId))
  {
    return( AT_FAIL );
  }
  
  cphs_ret = cphs_get_cphs_info(&phase, &sst);
  
  if ( cphs_ret EQ CPHS_OK)
  {
    if ( CPHS_CHECK_SST(sst, CPHS_SERVICE_CSP, CPHS_SERVICE_ACTIVATED) )
    {
      cphs_ret = cphs_set_csp_value(srcId, csp, csp_len);
    }
    else
    {
      ACI_ERR_DESC( ACI_ERR_CLASS_Cme, CME_ERR_OpNotAllow );
      return (AT_FAIL);
    }
  }

  switch (cphs_ret)
  {
    case CPHS_EXEC:
      return (AT_EXCT);
    case CPHS_OK:
      return (AT_CMPL);
    case CPHS_BUSY:
      ACI_ERR_DESC( ACI_ERR_CLASS_Cme, CME_ERR_SimBusy );
      return (AT_FAIL);
    default:
      ACI_ERR_DESC( ACI_ERR_CLASS_Cme, CME_ERR_OpNotAllow );
      return (AT_FAIL);
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_CMD            |
| STATE   : code                        ROUTINE : AT_PercentCPINF    |
+--------------------------------------------------------------------+

  PURPOSE : %CPINF: query cphs information and customer service profile
*/

GLOBAL T_ACI_RETURN qAT_PercentCPINF( T_ACI_CMD_SRC srcId,
                                      UBYTE        *phase,
                                      USHORT       *sst,
                                      CHAR         *csp,
                                      CHAR         *csp2,
                                      UBYTE        *max_csp_size,
                                      UBYTE        *max_csp2_size)
{
  T_CPHS_RET cphs_ret;
  
  TRACE_FUNCTION ("qAT_PercentCPINF()");

  /* check command source */
  if(!cmh_IsVldCmdSrc (srcId))
  {
    return( AT_FAIL );
  }
  
  cphs_ret = cphs_get_cphs_info(phase, sst);

  if ( cphs_ret EQ CPHS_OK)
  {
    if ( CPHS_CHECK_SST(*sst, CPHS_SERVICE_CSP, CPHS_SERVICE_ACTIVATED) )
    {
      cphs_ret = cphs_get_csprof( csp, csp2, max_csp_size, max_csp2_size);
    }
    else
    {
      TRACE_EVENT("CSP is not activated in the SST");
      *max_csp_size = 0;
      *max_csp2_size = 0;
    }
  }
  else
  {
    *max_csp_size = 0;
    *max_csp2_size = 0;
  }

  switch(cphs_ret)
  {
  case(CPHS_OK):
    return(AT_CMPL);

  default:
    return(AT_FAIL);
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_CMD            |
| STATE   : code                        ROUTINE : sAT_PercentCPCFU   |
+--------------------------------------------------------------------+

  PURPOSE : %CPFCU: set call forwarding flags
*/

GLOBAL T_ACI_RETURN sAT_PercentCPCFU( T_ACI_CMD_SRC srcId, 
                                      UBYTE         cfu_set, 
                                      T_CPHS_LINES  lines )
{
  T_CPHS_RET cphs_ret;

  TRACE_FUNCTION ("sAT_PercentCPCFU()");

  /* check command source */
  if(!cmh_IsVldCmdSrc (srcId))
  {
    return( AT_FAIL );
  }

  /* check parameter <cfu_set> */
  switch(cfu_set)
  {
  case(CPHS_SET_CFU_FLAG):
  case(CPHS_ERASE_CFU_FLAG):
    cphs_ret = cphs_set_cfu_flag(cfu_set, lines);
    break;

  default:
    ACI_ERR_DESC( ACI_ERR_CLASS_Ext, EXT_ERR_Parameter );
    return(AT_FAIL);
  }

  switch(cphs_ret)
  {
  case(CPHS_EXEC):
    cphsShrdPrm.srcId = srcId;
    return(AT_EXCT);

  case(CPHS_BUSY):
    return(AT_BUSY);

  default:
    return(AT_FAIL);
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_CMD            |
| STATE   : code                        ROUTINE : AT_PercentCPCFU    |
+--------------------------------------------------------------------+

  PURPOSE : %CPCFU: query call forwarding flags
*/

GLOBAL T_ACI_RETURN qAT_PercentCPCFU(T_ACI_CMD_SRC srcId, UBYTE *cfu_set, T_CPHS_LINES line)
{
  T_CPHS_RET cphs_ret;
  
  TRACE_FUNCTION ("qAT_PercentCPCFU()");

  /* check command source */
  if(!cmh_IsVldCmdSrc (srcId))
  {
    return( AT_FAIL );
  }
  
  cphs_ret = cphs_get_fwd_flag(cfu_set, line);

  switch(cphs_ret)
  {
  case(CPHS_OK):
    return(AT_CMPL);

  case(CPHS_BUSY):
    *cfu_set = CPHS_FLAG_ERROR;
    return(AT_BUSY);

  case(CPHS_NOT_INIT):
    *cfu_set = CPHS_FLAG_ERROR;
    ACI_ERR_DESC( ACI_ERR_CLASS_Cme, CME_ERR_OpNotAllow );
    return(AT_FAIL);
  default:
    *cfu_set = CPHS_FLAG_ERROR;
    return(AT_FAIL);
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_CMD            |
| STATE   : code                        ROUTINE : AT_PercentCPMB     |
+--------------------------------------------------------------------+

  PURPOSE : %CPMB: query mailbox numbers

  This function deals with both the set and query aspects of the
  %CPMB command. If 'rec_id' is present, indicating a set operation, 
  it will return information about the specified mailbox number 
  (parameters 'line', 'number', 'ton', 'npi' and 'alpha_id'). If\
  'rec_id' is set to ACI_NumParmNotPresent, indicating a query 
  operation, it will return the index of the first unused entry in 
  the list (parameter 'first').
*/

GLOBAL T_ACI_RETURN qAT_PercentCPMB( T_ACI_CMD_SRC  srcId,
                                      UBYTE         rec_id,
                                      T_CPHS_LINES  *line,
                                      CHAR          *number,
                                      T_ACI_TOA_TON *ton,
                                      T_ACI_TOA_NPI *npi,
                                      CHAR          *alpha_id,
                                      UBYTE         *first)
{
  T_CPHS_MB  mb_number;
  T_CPHS_RET cphs_ret;
  
  TRACE_FUNCTION ("qAT_PercentCPMB()");

  /* check command source */
  if(!cmh_IsVldCmdSrc (srcId))
  {
    return( AT_FAIL );
  }

  /*
  *   Determine the first free location, and write it to the
  *   memory pointed to by 'first', having first checked that
  *   it is not set to NULL.
  */
  if (first NEQ NULL)
  {
    if (cphs_first_free(first) NEQ CPHS_OK)
    {
      /*
      *   It is assumed that cphs_first_free() will have filled in
      *   the error number (ACI_ERR_DESC).
      */
      return(AT_FAIL);
    }
  }

  /*
  *   No further processing needed for the 'query' command, so
  *   unless we are dealing with the 'set' command, get
  *   out now.
  */
  if (rec_id EQ (UBYTE)ACI_NumParmNotPresent)
    return(AT_CMPL);

  cphs_ret = cphs_read_mb_number(rec_id, &mb_number);

  switch(cphs_ret)
  {
  case(CPHS_OK):
    memcpy(number, mb_number.number, sizeof(mb_number.number));
    memcpy(alpha_id, mb_number.alpha_id, sizeof(mb_number.alpha_id));
    *line     = mb_number.line;
    if ( mb_number.toa EQ NOT_PRESENT_8BIT)
    {
      *ton = TON_NotPresent;
      *npi = NPI_NotPresent;
    }
    else
    {
      *ton = (T_ACI_TOA_TON)(( mb_number.toa & 0x70 ) >> 4);
      *npi = (T_ACI_TOA_NPI)(mb_number.toa & 0x0F);
    }
    return(AT_CMPL);
  
  default:
    /* All errors are filled in cphs_read_mb_number(), thus do not fill any CmeError,
       just returning AT_FAIL */
    return(AT_FAIL);
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_CMD            |
| STATE   : code                        ROUTINE : AT_PercentCPMBW    |
+--------------------------------------------------------------------+

  PURPOSE : %CPMBW: write mailbox numbers
*/

GLOBAL T_ACI_RETURN sAT_PercentCPMBW( T_ACI_CMD_SRC       srcId,
                                      SHORT               index,
                                      CHAR*               number,
                                      T_ACI_TOA*          type,
                                      T_CPHS_PB_TEXT*     text)
{

  T_CPHS_MB_CMH_SNGL_STAT indexStat; /* status of parameter <index>    */
  T_CPHS_MB_CMH_CMN_STAT  entryStat; /* status of parameter <number>,  */
                                     /* <type> and <text>              */
  T_ACI_RETURN aci_result;
  T_CPHS_RET cphs_result;
  
  TRACE_FUNCTION ("sAT_PercentCPMBW()");
  
#ifndef NO_ASCIIZ
  /* convert Text */
  if ( text NEQ NULL )
  {
    UBYTE   tmpBuf[CPHS_MAX_MB_ALPHA_LEN];
    USHORT  len;
    text->cs = CS_Sim;
    cmh_cvtToDefGsm ( (CHAR*)text->data, (CHAR*)tmpBuf, &len );
    text->len = (UBYTE)len;
    memcpy ( text->data, tmpBuf, text->len );
  }
#endif /* #ifndef NO_ASCIIZ */
  
  /* check command source */  
  if(!cmh_IsVldCmdSrc (srcId)) 
  { 
    return( AT_FAIL );
  }
  
  /* process the status of parameter <index> */
  if ( index EQ ACI_NumParmNotPresent )
  {
    indexStat = SNGL_NOT_PRSNT; /* NOT PRESENT */
  }
  else if( index > CPHS_MAX_MB_ENTRIES OR 
           index < CPHS_MIN_MB_ENTRIES )
  {
    indexStat = SNGL_INVLD_PRSNT; /* INVALID */
  }
  else
  {
    indexStat = SNGL_VLD_PRSNT; /* VALID */
  }

  /* process the status of parameter <number>, <type> and <text> */  
  if ( number EQ NULL AND 
       type   EQ NULL AND 
       text   EQ NULL )
  {
    entryStat = CMN_NOT_PRSNT;
  }
  else if ( number NEQ NULL AND 
            text   NEQ NULL )
  {
    entryStat = CMN_FULL_PRSNT;
  }
  else
  {
    entryStat = CMN_PRTLY_PRSNT;
  }

  /*
   *-----------------------------------------------------------------
   * process the parameter:
   * 
   * A -> number, type, text
   * B -> index
   * 
   * 0 -> all elements not present
   * 1 -> all elements present
   *
   * A | B | result
   * --+---+---------------------------
   * 0 | 0 | fail
   * 0 | 1 | delete entry
   * 1 | 0 | write to first empty entry
   * 1 | 1 | write to specific entry
   *-----------------------------------------------------------------
   */  

  switch ( entryStat )
  {
    case ( CMN_FULL_PRSNT):
    case ( CMN_PRTLY_PRSNT ):
      /* parameters are full or partly present --> add entry to phonebook */

      aci_result = cmhCPHS_add_mb_record(srcId, indexStat, text, number, type, index);      
      
      return(aci_result);

    /* process variable <entryStat> with value <CMN_NOT_PRSNT> */
    case ( CMN_NOT_PRSNT ):

      if ( indexStat NEQ SNGL_VLD_PRSNT )
      { 
        ACI_ERR_DESC( ACI_ERR_CLASS_Ext, EXT_ERR_Parameter );
        return( AT_FAIL );
      }

      if (indexStat EQ SNGL_VLD_PRSNT)
      {
        /* delete this very entry */        
        cphs_result = cphs_write_mb_number(srcId,
                                           (UBYTE)index,
                                           NULL, /* tag */
                                           0,
                                           0,
                                           NULL, /* number */
                                           0);
      }
      else
      {
        ACI_ERR_DESC( ACI_ERR_CLASS_Cme, CME_ERR_InvIdx );
        return (AT_FAIL);
      }
      
      switch (cphs_result)
      {
        case CPHS_EXEC:
          return (AT_EXCT);
        case CPHS_OK:
          return (AT_CMPL); 
        case CPHS_NOT_INIT:
           ACI_ERR_DESC( ACI_ERR_CLASS_Cme, CME_ERR_OpNotAllow );
          return (AT_FAIL);          
        case CPHS_FAIL:
          ACI_ERR_DESC( ACI_ERR_CLASS_Cme, CME_ERR_InvIdx );
          return (AT_FAIL);
        default:
          ACI_ERR_DESC( ACI_ERR_CLASS_Cme, CME_ERR_TxtToLong );
          return (AT_FAIL);
      }
  }
  
  return ( AT_CMPL );  

}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_CMD            |
| STATE   : code                        ROUTINE : AT_PercentCPMBW    |
+--------------------------------------------------------------------+

  PURPOSE : %CPMBW: test command 
*/

GLOBAL T_ACI_RETURN tAT_PercentCPMBW ( T_ACI_CMD_SRC srcId,
                                       SHORT*        firstIdx,
                                       SHORT*        lastIdx,
                                       UBYTE*        nlength,
                                       UBYTE*        tlength )
{ 
  T_CPHS_RET cphs_result;

  TRACE_FUNCTION("tAT_PercentCPMBW()");
        
  /* check command source */  
  if(!cmh_IsVldCmdSrc (srcId)) 
  { 
    return( AT_FAIL );
  }

  cphs_result = cphs_get_mb_parameter ( firstIdx, lastIdx, nlength, tlength );
  switch (cphs_result)
  {
    case CPHS_OK:
      return (AT_CMPL); 
    case CPHS_NOT_INIT:
      ACI_ERR_DESC( ACI_ERR_CLASS_Cme, CME_ERR_OpNotAllow );
      return (AT_FAIL);
    default:
      ACI_ERR_DESC( ACI_ERR_CLASS_Cme, CME_ERR_Unknown );
      return (AT_FAIL);
  }
}
#endif /* CPHS_C */
