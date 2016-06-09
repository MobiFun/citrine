/*
+--------------------------------------------------------------------+
| PROJECT:                              $Workfile:: cphs_aci.c      $|
| $Author::                             $Revision::                 $|
| CREATED:                              $Modtime::                  $|
| STATE  : code                                                      |
+--------------------------------------------------------------------+

   MODULE  : CPHS

   PURPOSE : This module contains the functions adapting CPHS module to ACI.
*/

#ifndef CPHS_ACI_C
#define CPHS_ACI_C

#include "config.h"
#include "fixedconf.h"
#include "condat-features.h"
#include "aci_conf.h"

#include "aci_all.h"

#include "cphs.h"

#include "aci_cmh.h"
#include "psa.h"
#include "cmh.h"

#include "psa_util.h"
#include "psa_sim.h" /* cos' of simShrdPrm !!! */

#include "cmh_cc.h"
#include "psa_cc.h"

static UBYTE last_sim_operation;

/* ACI/CPHS adapt layer... Only part of the CPHS where cmh calls are allowed */
GLOBAL void cphs_aci_sim_access_cb(SHORT sim_shrd_id)
{
  BOOL sim_failure = FALSE;
  T_ACI_CME_ERR err;

  TRACE_FUNCTION("cphs_aci_sim_access_cb()");

  if( simShrdPrm.atb[sim_shrd_id].errCode NEQ SIM_NO_ERROR )
  {
    sim_failure = TRUE;
  }

  switch(last_sim_operation)
  {
  case(CPHS_SIM_READ_TRANSP_EF):
  case(CPHS_SIM_READ_RECORD):
    if(simShrdPrm.atb[sim_shrd_id].dataLen <= 0)
    {
      sim_failure = TRUE;
    }
    break;
  }

  /* free sim table entry */
  simShrdPrm.atb[sim_shrd_id].ntryUsdFlg = FALSE;

  if(sim_failure)
  {
    /* an error occured !!! */
    TRACE_EVENT("SIM Access Error !!!");
    err = (T_ACI_CME_ERR)cmhSIM_GetCmeFromSim(simShrdPrm.atb[sim_shrd_id].errCode);
    ACI_ERR_DESC( ACI_ERR_CLASS_Cme, err);
    cphs_sim_data_failure();
    return;
  }

  cphs_sim_data_accessed(simShrdPrm.atb[sim_shrd_id].recMax,
                         simShrdPrm.atb[sim_shrd_id].dataLen);

}

GLOBAL void cphs_sim_access_data(UBYTE cphs_sim_operation,
                                 UBYTE cphs_sim_field,
                                 UBYTE record,
                                 UBYTE *data_buffer,
                                 UBYTE data_buffer_size)
{
  USHORT sim_field_id = 0;

  TRACE_FUNCTION("cphs_sim_access_data()");

  /* The CPHS files are located under GSM standard DFs, therefore
   * it is not possible that their identifier values conflict with
   * standard values within the same DF. SIM code has incorporated 
   * the CPHS spec so that we are able to find these files without 
   * path information. We cannot totally rule out the possibility 
   * that future versions of 3GPP 51.011 (GSM 11.11) create file
   * identifiers in DF(TELECOM), which are the same as CPHS identifiers 
   * in DF(GSM) or vice versa, but this is very unlikely, 
   * because CPHS is widely known and used by various operators 
   * in contrast to HOMEZONE, which is specific to one operator. */

  switch(cphs_sim_field)
  {
  case(CPHS_SIM_CFU):
    TRACE_EVENT("CPHS Access sim field: SIM_CPHS_CFF");
    sim_field_id = SIM_CPHS_CFF;
    break;

  case(CPHS_SIM_VWI):
    TRACE_EVENT("CPHS Access sim field: SIM_CPHS_VMW");
    sim_field_id = SIM_CPHS_VMW; 
    break;

  case(CPHS_SIM_CINF):
    TRACE_EVENT("CPHS Access sim field: SIM_CPHS_CINF");
    sim_field_id = SIM_CPHS_CINF; 
    break;

  case(CPHS_SIM_CSP):
    TRACE_EVENT("CPHS Access sim field: SIM_CPHS_CSP");
    sim_field_id = SIM_CPHS_CSP; 
    break;

  case(CPHS_SIM_ORANGE_CSP):
    TRACE_EVENT("CPHS Access sim field: SIM_ORANGE_CSP2");
    sim_field_id = SIM_ORANGE_CSP2;
    break;

  case(CPHS_SIM_OPNLONG):
    TRACE_EVENT("CPHS Access sim field: SIM_CPHS_ONSTR");
    sim_field_id = SIM_CPHS_ONSTR; 
    break;

  case(CPHS_SIM_OPNSHORT):
    TRACE_EVENT("CPHS Access sim field: SIM_CPHS_ONSHF");
    sim_field_id = SIM_CPHS_ONSHF; 
    break;

  case(CPHS_SIM_MB):
    TRACE_EVENT("CPHS Access sim field: SIM_CPHS_MBXN");
    sim_field_id = SIM_CPHS_MBXN;
    break;

  case(CPHS_SIM_INFO_NUMS):
    TRACE_EVENT("CPHS Access sim field: SIM_CPHS_INFN");
    sim_field_id = SIM_CPHS_INFN;
    break;

  case(CPHS_SIM_INFO_NUMS_EA01):   /* in case of old sims */
    TRACE_EVENT("CPHS Access sim field: SIM_CPHS_INFN2");
    sim_field_id = SIM_CPHS_INFN2;
    break;

  case(CPHS_SIM_ALSNAMES):
    TRACE_EVENT("CPHS Access sim field: SIM_MSISDN");
    sim_field_id = SIM_MSISDN;
    break;

  default:
    TRACE_ERROR("cphs_sim_access_data(): wrong cphs_sim_field value !!!");
    break;
  }

  last_sim_operation = cphs_sim_operation;

  switch(cphs_sim_operation)
  {
  case(CPHS_SIM_READ_TRANSP_EF):
    cmhSIM_ReadTranspEF( CMD_SRC_NONE,
                         AT_CMD_NONE,
                         FALSE,
                         NULL,
                         sim_field_id,
                         0,
                         data_buffer_size,
                         data_buffer,
                         cphs_aci_sim_access_cb );
    break;

  case(CPHS_SIM_WRITE_TRANSP_EF):
    cmhSIM_WriteTranspEF(CMD_SRC_NONE,
                         AT_CMD_NONE,
                         FALSE,
                         NULL,
                         sim_field_id,
                         0,
                         data_buffer_size,
                         data_buffer,
                         cphs_aci_sim_access_cb );
    break;

  case(CPHS_SIM_READ_RECORD):
    cmhSIM_ReadRecordEF( CMD_SRC_NONE,
                         AT_CMD_CPHS,
                         FALSE,
                         NULL,
                         sim_field_id,
                         record,
                         data_buffer_size,
                         data_buffer,
                         cphs_aci_sim_access_cb );
    break;

  case(CPHS_SIM_WRITE_RECORD):
    cmhSIM_WriteRecordEF( CMD_SRC_NONE,
                          AT_CMD_CPHS,
                          FALSE,
                          NULL,
                          sim_field_id,
                          record,
                          data_buffer_size,
                          data_buffer,
                          cphs_aci_sim_access_cb );
    break;

  default:
    TRACE_ERROR("cphs_sim_access_data(): wrong cphs_sim_operation value !!!");
    break;
  }

}

/* converts a BCD number bcd of bcd_size length
into a string of ascii digits and writes the result in number */
GLOBAL void cphs_bcd2number(CHAR  *number,
                            UBYTE *bcd,
                            UBYTE bcd_size)
{
  int   i;
  UBYTE *num_buffer;

  TRACE_FUNCTION("cphs_bcd2number()");

  MALLOC(num_buffer, 2*bcd_size);

  /* Decode BCD */
  for(i=0;i<bcd_size;i++)
  {
    num_buffer[2*i] = (bcd[i] & 0x0F);
    num_buffer[2*i + 1] = (bcd[i] & 0xF0)>>4;
  }

  utl_BCD2DialStr((const UBYTE *)num_buffer, number, (UBYTE)(2*bcd_size));

  MFREE(num_buffer);
}

GLOBAL T_CPHS_RET cphs_get_als_active_line(UBYTE srcId, T_CPHS_LINES *line)
{
  T_ACI_ALS_MOD ALSmode;

  TRACE_FUNCTION("cphs_get_als_active_line()");

  cmhCC_get_active_als_mode((T_ACI_CMD_SRC)srcId, &ALSmode );

  switch( ALSmode )
  {
  case(ALS_MOD_SPEECH):
    *line = CPHS_LINE1;
    break;

  case(ALS_MOD_AUX_SPEECH):
    *line = CPHS_LINE2;
    break;

  case(ALS_MOD_NOTPRESENT):
  default:
    *line = CPHS_LINE_NULL;
    break;
  }

  return(CPHS_OK);
}


/* if( ! psaSIM_ChkSIMSrvSup( SRV_MSISDN )) return( AT_CMPL ); */


GLOBAL T_CPHS_LINES als_get_call_info(SHORT call_id)
{
  T_CC_CALL_TBL  *ctb;             /* Pointer to call table entry */
  SHORT           cId = call_id-1; /* tarnslation from +CLCC-like call_id and ACI cId */
  T_ACI_CRING_SERV_TYP cring_type;      /* type of bearer of call */

  TRACE_FUNCTION("als_get_call_info()");

/* ==== if the CPHS module is to be ported, then AT+CLCC shall be used...
   Extra info might need to be added to differentiate AuxVoice from Voice... ====
  MALLOC(calLst, MAX_CALL_NR * sizeof(T_ACI_CLCC_CALDESC));
  if(qAT_PlusCLCC(srcId, calLst) NEQ AT_CMPL)
  {
    TRACE_ERROR("+CLCC: error");
    return(CPHS_FAIL);
  }*/

  if (!psaCC_ctbIsValid (cId))
  {
    /* Call is not active ! */
    return(CPHS_LINE_NULL);
  }

  ctb = ccShrdPrm.ctb[cId];

  switch(cmhCC_getcalltype(cId))
  {
    case(VOICE_CALL):
      cring_type = (T_ACI_CRING_SERV_TYP)cmhCC_GetCallType_from_bearer (&ctb->BC[ctb->curBC]);
      if(cring_type EQ CRING_SERV_TYP_Voice)
      {
        return(CPHS_LINE1);
      }
      else if(cring_type EQ CRING_SERV_TYP_AuxVoice)
      {
        return(CPHS_LINE2);
      }
      break;
#ifdef FAX_AND_DATA
    case( TRANS_CALL ):
    case( NON_TRANS_CALL ):
      return(CPHS_LINE_DATA);

    case( FAX_CALL ):
      return(CPHS_LINE_FAX);
#endif
  }

/*  MFREE(calLst);  */
  return(CPHS_LINE_NULL);
}

void cphs_sim_read_mb_ext_rcd_cb(SHORT table_id)
{
  BOOL sim_failure = FALSE;
  T_ACI_CME_ERR err;
  
  if( simShrdPrm.atb[table_id].errCode NEQ SIM_NO_ERROR )
  {
    sim_failure = TRUE;
  }

  switch(last_sim_operation)
  {
  case(CPHS_SIM_READ_TRANSP_EF):
  case(CPHS_SIM_READ_RECORD):
    if(simShrdPrm.atb[table_id].dataLen <= 0)
    {
      sim_failure = TRUE;
    }
    break;
  }

  /* free sim table entry */
  simShrdPrm.atb[table_id].ntryUsdFlg = FALSE;

  if(sim_failure)
  {
    /* an error occured !!! */
    TRACE_EVENT("SIM Access Error !!!");
    err = (T_ACI_CME_ERR)cmhSIM_GetCmeFromSim(simShrdPrm.atb[table_id].errCode);
    ACI_ERR_DESC( ACI_ERR_CLASS_Cme, err);
    cphs_sim_data_failure();
    return;
  }

  
  cphs_write_sim_mb_ext_data(simShrdPrm.atb[table_id].dataLen);

}

#define DEFAULT_MAXSIZE_OF_RECORD (100)
GLOBAL void cphs_sim_read_mb_ext_rcd(UBYTE record,UBYTE *data_buffer)
{

  cmhSIM_ReadRecordEF ( CMD_SRC_NONE,
                              AT_CMD_NONE,
                              FALSE,
                              NULL,
                              SIM_EXT1,
                              record,
                              DEFAULT_MAXSIZE_OF_RECORD,
                              data_buffer,
                              cphs_sim_read_mb_ext_rcd_cb); 
    
   
}



#endif /* CPHS_ACI_C */

