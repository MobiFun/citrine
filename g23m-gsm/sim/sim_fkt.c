/* 
+----------------------------------------------------------------------------- 
|  Project :  GSM-F&D (8411)
|  Modul   :  SIM_FKT
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
|  Purpose :  This modul defines the wrapping functions for the
|             SIM application.
+----------------------------------------------------------------------------- 
*/ 

#ifndef SIM_FKT_C
#define SIM_FKT_C

#include "config.h"
#include "fixedconf.h"
#include "condat-features.h"

#define ENTITY_SIM

/*==== INCLUDES ===================================================*/

#include <string.h>
#include "typedefs.h"
#include "pconst.cdg"
#include "message.h"
#include "ccdapi.h"
#include "vsi.h"
#include "custom.h"
#include "gsm.h"
#include "cnf_sim.h"
#include "mon_sim.h"
#include "prim.h"
#include "pei.h"
#include "tok.h"
#include "sim.h"
#include "sim_em.h"

/*==== EXPORT =====================================================*/
#ifdef _SIMULATION_
LOCAL USHORT    first_time_into_fkt_select = TRUE;
LOCAL USHORT    file_id;
#endif
GLOBAL USHORT    stk_l_cmd = 0;


/*==== PRIVAT =====================================================*/

/*==== VARIABLES ==================================================*/

/*==== FUNCTIONS ===================================================*/

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6302)       MODULE  : SIM_FKT                    |
| STATE   : code                ROUTINE : FKT_check_pin_count        |
+--------------------------------------------------------------------+

  PURPOSE : PIN/PUK count is checked for secret code initialisation.
            count is set to zero, if not initialised, otherwise the
            initialisation flag (most significant bit) is reset.
*/

GLOBAL UBYTE FKT_check_pin_count (UBYTE count)
{
  if ((count & 0x80) EQ 0)
    return 0;

  return count & 0x0F;
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6302)       MODULE  : SIM_FKT                    |
| STATE   : code                ROUTINE : FKT_convert_error          |
+--------------------------------------------------------------------+

  PURPOSE : Converts sw1 and sw2 to an unique error code for the
            SIM application.

*/

/*
 * PIN/PUK is wrong, remaining attempts
 */
const static USHORT sim_inv_chv_A [6] =
{
    /* last requested PIN no          error code  */
    /* none                  */       SIM_CAUSE_OTHER_ERROR,
    /* PIN 1                 */       SIM_CAUSE_PIN1_EXPECT,
    /* PIN 2                 */       SIM_CAUSE_PIN2_EXPECT,
    /* PUK 1                 */       SIM_CAUSE_PUK1_EXPECT,
    /* PUK 2                 */       SIM_CAUSE_PUK2_EXPECT,
    /* NEVER                 */       SIM_CAUSE_ACCESS_PROHIBIT
};


/*
 * PIN/PUK is wrong, no remaining attempts
 */
const static USHORT sim_inv_chv_B [6] =
{
    /* last requested PIN no          error code  */
    /* none                  */       SIM_CAUSE_OTHER_ERROR,
    /* PIN 1                 */       SIM_CAUSE_PIN1_BLOCKED,
    /* PIN 2                 */       SIM_CAUSE_PIN2_BLOCKED,
    /* PUK 1                 */       SIM_CAUSE_PUK1_BLOCKED,
    /* PUK 2                 */       SIM_CAUSE_PUK2_BLOCKED,
    /* NEVER                 */       SIM_CAUSE_OTHER_ERROR
};

GLOBAL USHORT FKT_convert_error (USHORT sw1sw2, USHORT  size)
{
  TRACE_FUNCTION ("FKT_convert_error()");

  sim_data.sw1 = (UBYTE)(sw1sw2 >> 8);
  sim_data.sw2 = (UBYTE)sw1sw2;

  TRACE_EVENT_P2 ("SW1=%02X SW2=%02X", sim_data.sw1, sim_data.sw2);

  switch (sim_data.sw1)
  {
    case 0x00:
      /*
       * SIM driver error
       */
      if (SIM_IS_FLAG_CLEARED(SIM_INSERT))
      {
        return SIM_CAUSE_CARD_REMOVED;
      }
      if (sim_data.sw2 EQ 14)
      {
        SIM_SET_FLAG(DRV_FAILED_RETRY);
        return SIM_CAUSE_DRV_TEMPFAIL;
      }
      return CAUSE_MAKE(DEFBY_CONDAT, ORIGSIDE_MS, SIM_ORIGINATING_ENTITY, sim_data.sw2);
#if defined SIM_TOOLKIT
    case 0x9E:
      /*
       * use SW2 as length indicator for
       * the following get response
       */
      if (sim_data.sim_phase >= 3)  /* Phase 2+ or higher */
      {
        if (sim_data.stk_profile[0] & SAT_TP1_9E_XX)
        {
          sim_data.sim_data_len = (sim_data.sw2 EQ 0)? 0x100: (USHORT)sim_data.sw2;
          return SIM_CAUSE_DNL_ERROR;
        }
        else
          return SIM_CAUSE_SAT_BUSY;
      }
      else
        return SIM_CAUSE_OTHER_ERROR;
    case 0x93:
      if (sim_data.sim_phase >= 3)  /* Phase 2+ or higher */
      {
        return SIM_CAUSE_SAT_BUSY;
      }
      else
        return SIM_CAUSE_OTHER_ERROR;
    case 0x91:
      if (sim_data.sim_phase >= 3)  /* Phase 2+ or higher */
      {
        sim_data.proactive_sim_data_len = (sim_data.sw2 EQ 0)? 0x100: (SHORT)sim_data.sw2;
        return SIM_NO_ERROR;
      }
      else
        return SIM_CAUSE_OTHER_ERROR;
#endif
    case 0x92:
      if (sim_data.sw2 > 0xF)
      {
#ifdef REL99
        if (sim_data.sw2 EQ SW2_MEMORY_PROBLEM)
        {
          return SIM_CAUSE_MEM_PROBLEM;
        }
#endif /* end of ifdef REL99 */
        return SIM_CAUSE_OTHER_ERROR;
      }
      /* no break (otherwise the command was successful) */
    case 0x90:
      if( (size >= 0) AND (size <= 0x100) )
      {
        sim_data.sim_data_len = size;
        TRACE_EVENT_P1 ("sim_data.sim_data_len updated size =%X ", size);
      }
      return SIM_NO_ERROR;
    case 0x9F:
      /*
       * use SW2 as length indicator for
       * the following get response
       */
      sim_data.sim_data_len = (sim_data.sw2 EQ 0)? 0x100: (USHORT)sim_data.sw2;
      return SIM_NO_ERROR;

    case 0x94:
      switch (sim_data.sw2)
      {
        case 0:
          return SIM_CAUSE_NO_SELECT;
        case 2:
          return SIM_CAUSE_ADDR_WRONG;
        case 4:
          return SIM_CAUSE_UNKN_FILE_ID;
        case 8:
          return SIM_CAUSE_CMD_INCONSIST;
        default:
          break;
      }
      break;

    case 0x98:
      TRACE_EVENT_P1 ("LRP = %d", (int)sim_data.last_requested_pin_no);

      switch (sim_data.sw2)
      {
        case 2:
          return SIM_CAUSE_CHV_NOTSET;
        case 8:
          return SIM_CAUSE_CHV_VALIDATED;
        case 0x10:
          return SIM_CAUSE_EF_INVALID;    /* contradiction cases */
        case 4:
            return sim_inv_chv_A [sim_data.last_requested_pin_no];
        case 0x40:
          return sim_inv_chv_B [sim_data.last_requested_pin_no];
        case 0x50:
          return SIM_CAUSE_MAX_INCREASE;
        default:
          break;
      }
      break;
  }

  return SIM_CAUSE_OTHER_ERROR;
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6302)       MODULE  : SIM_FKT                    |
| STATE   : code                ROUTINE : FKT_ChangeCHV              |
+--------------------------------------------------------------------+

  PURPOSE : Wrapping function for the SIM driver call SIM_ChangeCHV.

*/

GLOBAL USHORT FKT_ChangeCHV (UBYTE      * old_pin,
                             UBYTE      * new_pin,
                             UBYTE        pin_id)
{
  USHORT size = 0;
  USHORT sw1sw2;
  UBYTE  response[4];

  TRACE_FUNCTION ("FKT_ChangeCHV()");

  sw1sw2 = SIM_ChangeCHV ((UBYTE *)response,
                          (UBYTE *)old_pin,
                          (UBYTE *)new_pin,
                          (UBYTE)pin_id,
                          &size);
  
  return FKT_convert_error (sw1sw2, size);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6302)       MODULE  : SIM_FKT                    |
| STATE   : code                ROUTINE : FKT_DisableCHV             |
+--------------------------------------------------------------------+

  PURPOSE : Wrapping function for the SIM driver call SIM_DisableCHV.

*/

GLOBAL USHORT FKT_DisableCHV (UBYTE      * pin)
{
  USHORT size = 0;
  USHORT sw1sw2;
  UBYTE  response[4];

  TRACE_FUNCTION ("FKT_DisableCHV()");

  sw1sw2 = SIM_DisableCHV ((UBYTE *)response,
                           (UBYTE *)pin,
                            &size);

  return FKT_convert_error (sw1sw2, size);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6302)       MODULE  : SIM_FKT                    |
| STATE   : code                ROUTINE : FKT_EnableCHV              |
+--------------------------------------------------------------------+

  PURPOSE : Wrapping function for the SIM driver call SIM_EnableCHV.

*/

GLOBAL USHORT FKT_EnableCHV (UBYTE      * pin)
{ 
  USHORT size = 0;
  USHORT sw1sw2;
  UBYTE  response[4];

  TRACE_FUNCTION ("FKT_EnableCHV()");

  sw1sw2 = SIM_EnableCHV ((UBYTE *)response,
                          (UBYTE *)pin,
                          &size);

  return FKT_convert_error (sw1sw2, size);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6302)       MODULE  : SIM_FKT                    |
| STATE   : code                ROUTINE : FKT_GetResponse            |
+--------------------------------------------------------------------+

  PURPOSE : Wrapping function for the SIM driver call SIM_GetResponse.

*/
GLOBAL USHORT FKT_GetResponse (UBYTE      * data,
                               USHORT       len)
{
  USHORT size = 0;
  USHORT sw1sw2;
  
  TRACE_FUNCTION ("FKT_GetResponse()");
  
  sw1sw2 = SIM_GetResponse ((UBYTE *)data,
                            len,
                            &size);
  return FKT_convert_error (sw1sw2, len);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6302)       MODULE  : SIM_FKT                    |
| STATE   : code                ROUTINE : FKT_Increase               |
+--------------------------------------------------------------------+

  PURPOSE : Wrapping function for the SIM driver call SIM_Increase.

*/

GLOBAL USHORT FKT_Increase (UBYTE * data)
{
    USHORT sw1sw2;
    USHORT size = 0;
    UBYTE  response[4];
    
    TRACE_FUNCTION ("FKT_Increase()");
    sw1sw2 = SIM_Increase    ((UBYTE *)response,
                (UBYTE *)data,
                &size);

  return FKT_convert_error (sw1sw2, size);
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6302)       MODULE  : SIM_FKT                    |
| STATE   : code                ROUTINE : FKT_Invalidate             |
+--------------------------------------------------------------------+

  PURPOSE : Wrapping function for the SIM driver call SIM_Invalidate.

*/

GLOBAL USHORT FKT_Invalidate (void)
{
    USHORT size = 0;
    USHORT sw1sw2;
    UBYTE  response[4];
  
    TRACE_FUNCTION ("FKT_Invalidate()");

    sw1sw2 = SIM_Invalidate  ((UBYTE *)response,
                &size);

    if (sw1sw2 EQ 0x9810)   /* already invalidated */
    return SIM_NO_ERROR;
  
  return FKT_convert_error (sw1sw2, size);
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6302)       MODULE  : SIM_FKT                    |
| STATE   : code                ROUTINE : FKT_ReadBinary             |
+--------------------------------------------------------------------+

  PURPOSE : Wrapping function for the SIM driver call SIM_ReadBinary.

*/

GLOBAL USHORT FKT_ReadBinary (UBYTE      * data,
                              USHORT       offset,
                              USHORT       length)
{
  USHORT sw1sw2;
  USHORT size = 0;
    
    TRACE_FUNCTION ("FKT_ReadBinary()");

    sw1sw2= SIM_ReadBinary ((UBYTE *)data,
                offset,
                length,
                &size);

  return FKT_convert_error (sw1sw2, size);
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6302)       MODULE  : SIM_FKT                    |
| STATE   : code                ROUTINE : FKT_ReadRecord             |
+--------------------------------------------------------------------+

  PURPOSE : Wrapping function for the SIM driver call SIM_ReadRecord.

*/

GLOBAL USHORT FKT_ReadRecord (UBYTE      * data,
                              UBYTE        mode,
                              USHORT       record,
                              USHORT       length)
{
  USHORT sw1sw2;
  USHORT size = 0;
    
#if !defined NTRACE
    char buf[48];
    sprintf (buf, "FKT_ReadRecord(): Nr. %hu", record);
    TRACE_FUNCTION (buf);
#endif
  
    sw1sw2 = SIM_ReadRecord ((UBYTE *)data,
                 (UBYTE)mode,
                 (UBYTE)record,
                 length,
                 &size);

  return FKT_convert_error (sw1sw2, size);
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6302)       MODULE  : SIM_FKT                    |
| STATE   : code                ROUTINE : FKT_Rehabilitate           |
+--------------------------------------------------------------------+

  PURPOSE : Wrapping function for the SIM driver call SIM_Rehabilitate.

*/

GLOBAL USHORT FKT_Rehabilitate (void)
{
    USHORT size = 0;
    USHORT sw1sw2;
    UBYTE  response[4];
  
    TRACE_FUNCTION ("FKT_Rehabilitate()");
  
    sw1sw2 = SIM_Rehabilitate ((UBYTE *)response,
                 &size);
  
    if (sw1sw2 EQ 0x9810)   /* already rehabilitated */
    return SIM_NO_ERROR;
  
  return FKT_convert_error (sw1sw2, size);
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6302)       MODULE  : SIM_FKT                    |
| STATE   : code                ROUTINE : FKT_Status                 |
+--------------------------------------------------------------------+

  PURPOSE : Wrapping function for the SIM driver call SIM_Status.

*/

GLOBAL USHORT FKT_Status (UBYTE * pin_cnt,
                          UBYTE * pin2_cnt,
                          UBYTE * puk_cnt,
                          UBYTE * puk2_cnt)
{
  USHORT size = 0;
  USHORT sw1sw2; 

 USHORT cause;
 USHORT fileid;
 union
 {
   T_DIR_STATUS status;
   UBYTE response [40];
 } dir;

  TRACE_FUNCTION ("FKT_Status()");

  memset (dir.response, 0, sizeof(dir.response));
  
  sw1sw2 = SIM_Status_Extended (dir.response,
                                sim_data.dir_status_len, &size);
  
  cause =  FKT_convert_error (sw1sw2, size);
  
  *pin_cnt  = 0;
  *puk_cnt  = 0;
  *pin2_cnt = 0;
  *puk2_cnt = 0;

  if (cause EQ SIM_NO_ERROR)
  {
    /*
     * Check file id on active call: if different from last directory
     * indicate failure during SIM Presence Detection (27.20).
     * A selection of a non-existent DF (possible with SIM_ACCESS_REQ)
     * leads to the loss of the current DF: this confuses the SIM
     * Presence Detection, therefore the validity of the actual DF
     * stored in 'sim_data.act_directory' must be considered.
     */
    fileid = (dir.status.fileid[0] << 8) | dir.status.fileid[1];

    if (SIM_IS_FLAG_SET (CALL_ACTIVE) AND             /* call active */
        sim_data.act_directory NEQ NOT_PRESENT_16BIT  /* actual DF known? */
        AND fileid NEQ sim_data.act_directory)        /* compare DF */
      return SIM_CAUSE_CARD_REMOVED;
    else
    {
      if (SIM_TI_DRV_X_BYTES > 0)   /* discard SW1, SW2 from response! */
        memset (&dir.response[sim_data.dir_status_len], 0, SIM_TI_DRV_X_BYTES);
      /*
       * Directory status is available
       */
      *pin_cnt  = FKT_check_pin_count (dir.status.pinstatus);
      *puk_cnt  = FKT_check_pin_count (dir.status.unbstatus);
      *pin2_cnt = FKT_check_pin_count (dir.status.pin2status);
      *puk2_cnt = FKT_check_pin_count (dir.status.unb2status);
    }
  }
  return cause;
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6302)       MODULE  : SIM_FKT                    |
| STATE   : code                ROUTINE : FKT_RunGSMAlgo             |
+--------------------------------------------------------------------+

  PURPOSE : Wrapping function for the SIM driver call SIM_RunGSMAlgo.

*/
GLOBAL USHORT FKT_RunGSMAlgo           (UBYTE      * rand, UBYTE      * data, USHORT       len)
{
  UBYTE  response[4];
  USHORT sw1sw2;
  USHORT error;
  USHORT size = 0;
  
    TRACE_FUNCTION ("FKT_RunGSMAlgo()");
  
    sw1sw2 = SIM_RunGSMAlgo ((UBYTE *)response,
                 (UBYTE *)rand,
                 &size);

  error = FKT_convert_error (sw1sw2, size);

  if (error EQ SIM_NO_ERROR)
     error = FKT_GetResponse (data, len);

  return error;
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6302)       MODULE  : SIM_FKT                    |
| STATE   : code                ROUTINE : FKT_Select                 |
+--------------------------------------------------------------------+

  PURPOSE : Wrapping function for the SIM driver call SIM_Select.

*/
LOCAL USHORT fkt_select_one (USHORT id, UBYTE * data, USHORT len)
{

  USHORT size = 0;
  USHORT sw1sw2;
  USHORT error;
  UBYTE  response[4];

  sw1sw2 = SIM_Select (id,
                       (UBYTE *)response,
                       &size);

  
  error = FKT_convert_error (sw1sw2, size);

  if (error EQ SIM_NO_ERROR)
  {
      if( (data NEQ NULL   )  
          AND (len  NEQ 0      ) 
         #ifdef _SIMULATION_          
          AND (id   EQ  file_id) 
         #endif          
                                       )
      {
      #ifdef _SIMULATION_
        len = (sim_data.sim_data_len < len)?
               sim_data.sim_data_len: len;
        first_time_into_fkt_select = TRUE;
      #endif

        error = FKT_GetResponse (data, sim_data.sim_data_len);
      }
  }

  return error;

}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6302)       MODULE  : SIM_FKT                    |
| STATE   : code                ROUTINE : sim_select_df              |
+--------------------------------------------------------------------+

  PURPOSE : Function to select the directory given the path.

*/
LOCAL USHORT sim_select_df(U16 dir_level,UBYTE *data,USHORT len)
{
  USHORT result = SIM_NO_ERROR;

  result = fkt_select_one (dir_level, data, len);
  if (result NEQ SIM_NO_ERROR)
  {
     sim_data.act_directory = NOT_PRESENT_16BIT;
     sim_data.act_field     = NOT_PRESENT_16BIT;
  }
  else
  {
     sim_data.act_directory = dir_level;
  }
  return result;
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6302)       MODULE  : SIM_FKT                    |
| STATE   : code                ROUTINE : FKT_Select                 |
+--------------------------------------------------------------------+

  PURPOSE : Wrapping function for the SIM driver call SIM_Select.

*/
GLOBAL USHORT FKT_Select  (USHORT       id,
                           BOOL         path_info_present, T_path_info * path_info_ptr,
                           UBYTE      * data, USHORT len)
{
  USHORT result;
  USHORT directory = (SIM_IS_FLAG_SET (GSM_DATAFIELD))?
                     SIM_DF_GSM: SIM_DF_1800;   /* default parent directory */

#if !defined NTRACE
  char buf[32];

  sprintf (buf, "FKT_Select(): id = %04X", id);
  TRACE_FUNCTION (buf);
#endif

#ifdef _SIMULATION_
 if( (first_time_into_fkt_select EQ TRUE ) AND
      (data NEQ NULL) AND (len NEQ 0) )
      file_id = id;
#endif


  switch (id & 0xFF00)  /* selecting DF deselects EF */
  {
  case 0x3F00:
  case 0x7F00:
  case 0x5F00:
    sim_data.act_field = NOT_PRESENT_16BIT;
    break;
  }

  if( path_info_present NEQ FALSE )
  {
    if((sim_data.act_directory & 0xFF00) NEQ 0x5F00 AND 
        sim_data.act_directory NEQ NOT_PRESENT_16BIT)
    {
      /* Currently selected df is MF or a first level directory */
      if(sim_data.act_directory NEQ path_info_ptr->df_level1)
      {
        result = sim_select_df(path_info_ptr->df_level1,data,len);
        if(result NEQ SIM_NO_ERROR)
          return result;
      }
    }
    /* Currently selected df is a second level directory */
    else
    {
      result = sim_select_df(SIM_MF,data,len);
      if(result NEQ SIM_NO_ERROR)
        return result;
      result = sim_select_df(path_info_ptr->df_level1,data,len);
      if(result NEQ SIM_NO_ERROR)
        return result;
    }
    if(path_info_ptr->v_df_level2)
    {
      result = sim_select_df(path_info_ptr->df_level2,data,len);
      if(result NEQ SIM_NO_ERROR)
        return result;
    }
    result = fkt_select_one (id, data, len);
    if (result NEQ SIM_NO_ERROR)
    {

      sim_data.act_field = NOT_PRESENT_16BIT;
    }
    else
    {
      sim_data.act_field = id;
    }
    return result;
  }
  else
  {
  switch (id)
  {
  case SIM_MF:
   result = fkt_select_one (SIM_MF, data, len);
    if (result NEQ SIM_NO_ERROR)
      sim_data.act_directory = NOT_PRESENT_16BIT;
    else
    {
      sim_data.dir_status_len = sim_data.sim_data_len;
      sim_data.act_directory = id;
    }
    return result;

  default:
          /* All the standard defined EFs would be handled before coming to this
           * point. Complete path information should be given for non-standard 2nd level EFs 
           * and they also would be handled before coming to this point.
           * This statement will be hit for non-standard files 
           * without path information. Hence returning error */
          return SIM_CAUSE_UNKN_FILE_ID;
      break;                /* any other elementary file */

      /* continue here in case of any first level directory */
  case SIM_DF_GSM:
  case SIM_DF_1800:
  case SIM_DF_TELECOM:
  case SIM_DF_VI:
  case SIM_DF_ORANGE:
    if (sim_data.act_directory EQ id)
      return SIM_NO_ERROR;  /* already selected */
    else
    {
          /* If current directory has not been selected or it is a 
             2nd level and under another 1st level DF */        
          if (((sim_data.act_directory & 0xFF00) EQ 0x5F00 AND
               ! SIM_IS_DF_LEVEL2_UNDER_DF_LEVEL1(sim_data.act_directory,id))
               OR sim_data.act_directory EQ NOT_PRESENT_16BIT)
      {                     /* MF has to be selected first */
             result = sim_select_df(SIM_MF,data,len);
             if(result NEQ SIM_NO_ERROR)
               return result;
      }
       result = fkt_select_one (id, data, len);
      if (result NEQ SIM_NO_ERROR)
      {
        sim_data.act_directory = NOT_PRESENT_16BIT;
        sim_data.act_field = NOT_PRESENT_16BIT;
      }
      else
      {
        sim_data.dir_status_len = sim_data.sim_data_len;
        sim_data.act_directory = id;
      }
      return result;
    }
  case SIM_DF_GRAPHICS:
    directory = SIM_DF_TELECOM;   /* parent directory */
    /* no break */
    /*lint -fallthrough*/
  case SIM_DF_SOLSA:
      case SIM_DF_MEXE:
    if (sim_data.act_directory EQ id)
      return SIM_NO_ERROR;  /* already selected */

    if (sim_data.act_directory NEQ directory)
    {                       /* not the parent directory */
          /* If current directory is MF or first level OR if the 2nd level 
           directory is under another 1st level df, select the parent directory */
          if((sim_data.act_directory & 0xFF00) NEQ 0x5F00 ||
             (!SIM_IS_DF_LEVEL2_UNDER_DF_LEVEL1(sim_data.act_directory,directory)) )
        {
            result = FKT_Select (directory, FALSE, NULL, data, len);
            if (result NEQ SIM_NO_ERROR)
            {
              sim_data.act_directory = NOT_PRESENT_16BIT;
              return result;
            }
      }
    }
    result = fkt_select_one (id, data, len);
    if (result NEQ SIM_NO_ERROR)
    {
      sim_data.act_directory = NOT_PRESENT_16BIT;
      sim_data.act_field = NOT_PRESENT_16BIT;
    }
    else
    {
      sim_data.dir_status_len = sim_data.sim_data_len;
      sim_data.act_directory = id;
    }
    return result;

  case SIM_ICCID:
  case SIM_ELP:
    /*
     * Access to Root Directory
     */
    directory = SIM_MF;
    break;

  case SIM_ADN:
  case SIM_FDN:
  case SIM_SMS:
  case SIM_CCP:
  case SIM_MSISDN:
  case SIM_SMSP:
  case SIM_SMSS:
  case SIM_LND:
  case SIM_SMSR:
  case SIM_SDN:
  case SIM_EXT1:
  case SIM_EXT2:
  case SIM_EXT3:
  case SIM_BDN:
  case SIM_EXT4:
/* VO temp PATCH: Needed for reading CPHS info num from old SIMs */
  case SIM_CPHS_INFN2:
/* VO temp PATCH end */
    /*
     * Access to Telecom Directory
     */
    directory = SIM_DF_TELECOM;
    break;

      case SIM_LP:
      case SIM_IMSI:
      case SIM_KC:
      case SIM_PLMNSEL:
      case SIM_HPLMN:
      case SIM_ACMMAX:
      case SIM_SST:
      case SIM_ACM:
      case SIM_GID1:
      case SIM_GID2:
      case SIM_PUCT:
      case SIM_CBMI:
      case SIM_SPN:
      case SIM_CBMID:
      case SIM_BCCH:
      case SIM_ACC:
      case SIM_FPLMN:
      case SIM_LOCI:
      case SIM_AD:
      case SIM_PHASE:
      case SIM_VGCS:
      case SIM_VGCSS:
      case SIM_VBS:
      case SIM_VBSS:
      case SIM_EMLPP:
      case SIM_AAEM:
      case SIM_ECC:
      case SIM_CBMIR:
      case SIM_DCK:
      case SIM_CNL:
      case SIM_NIA:
      case SIM_KCGPRS:
      case SIM_LOCGPRS:
      case SIM_SUME:
      case SIM_CPHS_VMW:
      case SIM_CPHS_SST:
      case SIM_CPHS_CFF:
      case SIM_CPHS_ONSTR:
      case SIM_CPHS_CSP:
      case SIM_CPHS_CINF:
      case SIM_CPHS_MBXN:
      case SIM_CPHS_ONSHF:
      case SIM_CPHS_INFN:

#ifdef REL99
      case SIM_UCPS_ACTEC:
      case SIM_OCPS_ACTEC:
      case SIM_HPLMN_ACT:
      case SIM_CPBCCH:
      case SIM_INV_SCAN:
      case SIM_RPLMN_ACT:
#endif /* REL99 */
      case SIM_PNN:
      case SIM_OPL:

        /*
         * Access to GSM
         */
        directory = (SIM_IS_FLAG_SET (GSM_DATAFIELD))?
                     SIM_DF_GSM: SIM_DF_1800;
        break;

#ifndef REL99
      case SIM_VI_HZ_PARAM:
      case SIM_VI_HZ_CACHE_1:
      case SIM_VI_HZ_CACHE_2:
      case SIM_VI_HZ_CACHE_3:
      case SIM_VI_HZ_CACHE_4:
       directory = SIM_DF_VI;
       break;
#endif /* ifndef REL99 */

      case SIM_MEXE_ST:
      case SIM_ORPK:
      case SIM_ARPK:
      case SIM_TPRPK:
        /*
         * Access to MExE Directory
         */
        directory = SIM_DF_MEXE;
        break;
  case SIM_IMG:
    /*
     * Access to Icon Directory
     */
    directory = SIM_DF_GRAPHICS;
    break;

  case SIM_ORANGE_DYN2_FLAG:
  case SIM_ORANGE_CSP2:
  case SIM_ORANGE_ACM2:
  case SIM_ORANGE_DYN_FLAGS:
    /*
     * Access to Orange Directory
     */
    directory = SIM_DF_ORANGE;
    break;

  case SIM_SAI:
  case SIM_SLL:
    /*
     * Access to SoLSA Directory
     */
    directory = SIM_DF_SOLSA;
    break;
    }
  }

  if (sim_data.act_directory NEQ directory)
  {
    /*
     * select directory
     */
#ifdef _SIMULATION_

      if( (data NEQ NULL) AND (len NEQ 0) )
        first_time_into_fkt_select = FALSE;
     #endif
      result = FKT_Select (directory, FALSE, NULL, data, len);
      if (result NEQ SIM_NO_ERROR)    /* directory selection fails */
      {
        sim_data.act_directory = NOT_PRESENT_16BIT;
        sim_data.act_field = NOT_PRESENT_16BIT;
        return result;
      }
  }

  if (sim_data.act_field NEQ id)
  {
    /*
     * secondly select elementary field
     */
     result = fkt_select_one (id, data, len);
    if (result NEQ SIM_NO_ERROR)    /* EF selection fails */
    {
      sim_data.act_field = NOT_PRESENT_16BIT;
#ifdef __INVALID    /* more sophisticated SELECT error handling */

      sw1 = (UBYTE)(SIM_Status_Extended ((UBYTE *)response, 6, &size) >> 8);

      if (sw1 NEQ 0x90 AND sw1 NEQ 0x91)
        /*
         * SIM Status request failed
         */
        sim_data.act_directory = NOT_PRESENT_16BIT;
      else if (((response[4] << 8) | response[5])
                NEQ sim_data.act_directory)
        /*
         * SIM Presence Detection indicates invalid SIM
         */
        sim_data.act_directory = NOT_PRESENT_16BIT;
#endif
    }
    else
    {
      sim_data.act_directory = directory;
      sim_data.act_field = id;
    }
    return result;
  }
  else
    /*
     * field is already selected
     */
    return SIM_NO_ERR_FILE_ALREADY_SELECTED;
}



/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6302)       MODULE  : SIM_FKT                    |
| STATE   : code                ROUTINE : FKT_UnblockCHV             |
+--------------------------------------------------------------------+

  PURPOSE : Wrapping function for the SIM driver call SIM_UnblockCHV.

*/

GLOBAL USHORT FKT_UnblockCHV (UBYTE      * unblockCHV,
                              UBYTE      * new_CHV,
                              UBYTE        chvType)
{
  USHORT size = 0;
  USHORT sw1sw2;
  UBYTE  response[4];

  TRACE_FUNCTION ("FKT_UnblockCHV()");

  sw1sw2 = SIM_UnblockCHV ((UBYTE *)response,
                           (UBYTE *)unblockCHV,
                           (UBYTE *)new_CHV,
                           (UBYTE)chvType,
                            &size);
  
  return FKT_convert_error (sw1sw2, size);
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6302)       MODULE  : SIM_FKT                    |
| STATE   : code                ROUTINE : FKT_UpdateBinary           |
+--------------------------------------------------------------------+

  PURPOSE : Wrapping function for the SIM driver call SIM_UpdateBinary.

*/

GLOBAL USHORT FKT_UpdateBinary (UBYTE      * data,
                                USHORT       length,
                                USHORT       offset)
{
    USHORT size = 0;
    USHORT sw1sw2;
    UBYTE response [4];
  
    TRACE_FUNCTION ("FKT_UpdateBinary()");
    
    sw1sw2 = SIM_UpdateBinary ((UBYTE *)response,
                 (UBYTE *)data,
                 offset,
                 length,
                 &size);
  
  return FKT_convert_error (sw1sw2, size);
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6302)       MODULE  : SIM_FKT                    |
| STATE   : code                ROUTINE : FKT_UpdateRecord           |
+--------------------------------------------------------------------+

  PURPOSE : Wrapping function for the SIM driver call SIM_UpdateRecord.

*/

GLOBAL USHORT FKT_UpdateRecord (UBYTE      * data,
                                USHORT       length,
                                UBYTE        mode,
                                USHORT       record)
{
    USHORT size = 0;
    USHORT sw1sw2;
    UBYTE response [4];
  
#if !defined NTRACE
    char buf[48];
    sprintf (buf, "FKT_UpdateRecord(): Nr. %hu", record);
    TRACE_FUNCTION (buf);
#endif

    sw1sw2 = SIM_UpdateRecord ((UBYTE *)response,
                 (UBYTE *)data,
                 (UBYTE)mode,
                 (UBYTE)record,
                 length,
                 &size);

  return FKT_convert_error (sw1sw2, size);
}



/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6302)       MODULE  : SIM_FKT                    |
| STATE   : code                ROUTINE : FKT_VerifyCHV              |
+--------------------------------------------------------------------+

  PURPOSE : Wrapping function for the SIM driver call SIM_VerifyCHV.

*/

GLOBAL USHORT FKT_VerifyCHV (UBYTE      * pin,
                             UBYTE        pin_id)
{
  USHORT size = 0;
  USHORT sw1sw2;
  UBYTE  response[4];

  TRACE_FUNCTION ("FKT_VerifyCHV()");

  sw1sw2 = SIM_VerifyCHV ((UBYTE *)response,
                          (UBYTE *)pin,
                          (UBYTE)pin_id,
                          &size);

  return FKT_convert_error (sw1sw2, size);
}

/*
+********************************************************************+
| Moved from sim_stk.c - for CQ 34109 under feature flag SIM_TOOLKIT |
+********************************************************************+
*/

#ifdef SIM_TOOLKIT

static const  UBYTE timer_env[] = {
  STK_TIMER_EXPIRATION_TAG,
  STK_DEVICE_IDENTITY_LEN+STK_TIMER_ID_LEN+STK_TIMER_VALUE_LEN+6,
  STK_DEVICE_IDENTITY_TAG|STK_COMPREHENSION_REQUIRED, STK_DEVICE_IDENTITY_LEN, 0x82, 0x81,
  STK_TIMER_ID_TAG|STK_COMPREHENSION_REQUIRED, STK_TIMER_ID_LEN, 0,
  STK_TIMER_VALUE_TAG|STK_COMPREHENSION_REQUIRED, STK_TIMER_VALUE_LEN, 0, 0, 0
};

UBYTE pending_timers[9] = {0,0,0,0,0,0,0,0,0};
UBYTE next_pos_to_fill = 0;
UBYTE next_pos_to_send = 0;

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (8419)       MODULE  : SIM_STK                    |
| STATE   : code                ROUTINE : FKT_TerminalResponse       |
+--------------------------------------------------------------------+

  PURPOSE : Wrapping function for the SIM driver call SIM_TerminalResponse

*/

GLOBAL USHORT FKT_TerminalResponse (UBYTE * data,
                                    USHORT  length)
{
      USHORT size = 0;
      USHORT sw1sw2;
      UBYTE  response[4];
      UBYTE env[sizeof(timer_env)];
      USHORT index;
      UBYTE dummy[4];
      USHORT error;
      USHORT i;

      TRACE_FUNCTION ("FKT_TerminalResponse()");

      sw1sw2 = SIM_TerminalResponse (response,
                                     data,
                                     length,
                                     &size);
      sim_data.term_resp_sent = TRUE;

      SIM_EM_TERMINAL_RESPONSE;
      /*
      * if SIM response is OK, try resending pending timer expiry envelopes (send updated envelopes)
      */
     if( 0x9000 == sw1sw2 )
     { 
       for(i=0;i<8;i++)
       { 
         if (next_pos_to_fill != next_pos_to_send)
         { 
           /*
           * some timer expiry envelopes are pending
           */
           index = pending_timers[next_pos_to_send];
           memcpy (env, timer_env, sizeof(timer_env));
           env[8] = (UBYTE)(index + 1);  /* Timer number range is 1..8 */
           env[11] = sim_data.timer[index].hour;
           env[12] = sim_data.timer[index].minute;
           env[13] = sim_data.timer[index].second;		    
           error = FKT_Envelope (dummy, env, sizeof(timer_env),0);
           if(8 == next_pos_to_send)
           {
             next_pos_to_send = 0;
           }
           else
           {
             next_pos_to_send++;
           }
           if (error NEQ SIM_NO_ERROR)
           {
             break;
           }
         } 
       } 
     } 

      return FKT_convert_error (sw1sw2, size);
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (8419)       MODULE  : SIM_STK                    |
| STATE   : code                ROUTINE : FKT_Envelope               |
+--------------------------------------------------------------------+

  PURPOSE : Wrapping function for the SIM driver call SIM_Envelope

*/

GLOBAL USHORT FKT_Envelope  (UBYTE      * data_out, UBYTE      * data_in,
                             USHORT       in_length, USHORT       out_length)
{

     USHORT    size = 0;
     USHORT    sw1sw2;
     USHORT    error;


     TRACE_FUNCTION ("FKT_Envelope()");
  
     sw1sw2 = SIM_Envelope (data_out,
                            data_in,
                            in_length,
                            &size);

  SIM_EM_ENVELOPE;

  error = FKT_convert_error (sw1sw2, size);

  if(sw1sw2 EQ 0x9000)
  	 sim_data.sim_data_len = 0;

  if (error EQ SIM_NO_ERROR)
  {
    if (data_out AND out_length)
    {
      if (sim_data.sim_data_len > 0)
      {
	stk_l_cmd = sim_data.sim_data_len << 3;
	out_length = (USHORT) (stk_l_cmd >> 3);
        error = FKT_GetResponse (data_out, out_length);
      }
    }
  }

  return error;
  
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (8419)       MODULE  : SIM_STK                    |
| STATE   : code                ROUTINE : FKT_TerminalProfile        |
+--------------------------------------------------------------------+

  PURPOSE : Wrapping function for the SIM driver call SIM_TerminalProfile

*/

GLOBAL USHORT FKT_TerminalProfile (UBYTE * data,
                                   USHORT  length)
{
      USHORT    size = 0;
      USHORT    sw1sw2;
      UBYTE    response[4];

      TRACE_FUNCTION ("FKT_TerminalProfile()");

      sw1sw2 = SIM_TerminalProfile (response,
                                    data,
                                    length,
                                    &size);

  return FKT_convert_error (sw1sw2, size);
  
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (8419)       MODULE  : SIM_STK                    |
| STATE   : code                ROUTINE : FKT_Fetch                  |
+--------------------------------------------------------------------+

  PURPOSE : Wrapping function for the SIM driver call SIM_Fetch

*/

GLOBAL USHORT FKT_Fetch (UBYTE * cmd,
                         USHORT  length)
{
     USHORT    size = 0;
     USHORT    sw1sw2;

     TRACE_FUNCTION ("FKT_Fetch()");

     sw1sw2 = SIM_Fetch (cmd,
                         length,
                         &size);
  
  return FKT_convert_error (sw1sw2, size);
  
}

#endif /* SIM_TOOLKIT */

#endif
