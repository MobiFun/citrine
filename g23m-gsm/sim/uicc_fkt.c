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
#include "8010_136_SIMDRV_SAP_inline.h"

/*==== EXPORT =====================================================*/

/*==== PRIVAT =====================================================*/

/*==== VARIABLES ==================================================*/
GLOBAL USHORT    stk_l_cmd = 0;
/*==== FUNCTIONS ===================================================*/

/* Implements Measure# 13 */
/*
+---------------------------------------------------------------------+
| PROJECT : GSM-PS (6302)       MODULE  : SIM_FKT                     |
| STATE   : code                ROUTINE : FKT_ChangeCHV_n_UnblockCHV  |
+---------------------------------------------------------------------+

  PURPOSE : Wrapping function for the SIM driver call SIM_ChangeCHV and 
            SIM_UnblockCHV

*/

LOCAL USHORT FKT_ChangeCHV_n_UnblockCHV ( UBYTE            * oldPin_UnblkCHV,
                                          UBYTE            * newPin_CHV,
                                          UBYTE              pinId_chvType,
                                          UBYTE              inst_code) 
{
  USHORT size = 0;
  USHORT sw1sw2;

  U8     reader_id;
  U8     i;
  U8     data[2*MAX_PIN_LEN];

  UBYTE  response[SIMDRV_MAX_RESULT];

  T_SIMDRV_cmd_header    cmd_header;
  T_SIMDRV_data_info     data_info;
  T_SIMDRV_result_info   result_info;

  TRACE_FUNCTION ("FKT_ChangeCHV_n_UnblockCHV()");

  reader_id      = SIMDRV_VAL_READER_ID__RANGE_MIN;
  cmd_header.cla = SIMDRV_GSM_CLASS_BYTE;
  cmd_header.ins = inst_code;      
  cmd_header.p1  = 0;
  cmd_header.p2  = pinId_chvType;  
  
  for (i=0;i<2*MAX_PIN_LEN;i++)
  {
    if (i < MAX_PIN_LEN)
      data[i] = oldPin_UnblkCHV[i];
    else
      data[i] = newPin_CHV[i-MAX_PIN_LEN];
  }
  
  data_info.data   = (U8 *)data;
  data_info.c_data = sizeof(data);
  
  result_info.result   = response;
  result_info.c_result = size;
  result_info.len      = NOT_PRESENT_16BIT;
  
  sw1sw2 =  simdrv_xch_apdu(reader_id, cmd_header, data_info, &result_info);
  
  return FKT_convert_error (sw1sw2,result_info.c_result);
}
  
  
/* Implements Measure# 16 */
/*
+------------------------------------------------------------------------+
| PROJECT : GSM-PS (6302)       MODULE  : SIM_FKT                        |
| STATE   : code                ROUTINE : FKT_Invalidate_n_Rehabilitate  |
+------------------------------------------------------------------------+

  PURPOSE : Wrapping function for the SIM driver call SIM_Invalidate and 
            SIM_Rehabilitate.

*/

LOCAL USHORT FKT_Invalidate_n_Rehabilitate ( UBYTE inst_code) 
{
  USHORT   size = 0;
  USHORT   sw1sw2;

  U8       reader_id;

  UBYTE    response[SIMDRV_MAX_RESULT];

  T_SIMDRV_cmd_header    cmd_header;
  T_SIMDRV_data_info     data_info;
  T_SIMDRV_result_info   result_info;
  
  TRACE_FUNCTION ("FKT_Invalidate_n_Rehabilitate()");
  
  reader_id      = SIMDRV_VAL_READER_ID__RANGE_MIN;
  cmd_header.cla = SIMDRV_GSM_CLASS_BYTE;
  cmd_header.ins = inst_code;
  cmd_header.p1  = 0;
  cmd_header.p2  = 0;
  
  data_info.data   = NULL;
  data_info.c_data = 0;
  
  result_info.result   = response;
  result_info.c_result = size;
  result_info.len      = NOT_PRESENT_16BIT;
  
  sw1sw2 = simdrv_xch_apdu(reader_id,cmd_header,data_info,&result_info);

  if (sw1sw2 EQ 0x9810)   /* already invalidated */
    return SIM_NO_ERROR;
  
  return FKT_convert_error (sw1sw2,result_info.c_result);
}
  
  
/* Implements Measure# 17 */
/*
+----------------------------------------------------------------------+
| PROJECT : GSM-PS (6302)       MODULE  : SIM_FKT                      |
| STATE   : code                ROUTINE : FKT_DisableCHV_n_EnableCHV   |
+----------------------------------------------------------------------+

  PURPOSE : Wrapping function for the SIM driver call SIM_DisableCHV  and 
            SIM_EnableCHV

*/

LOCAL USHORT FKT_DisableCHV_n_EnableCHV (UBYTE  * pin, UBYTE inst_code) 
{
  USHORT  size = 0;
  USHORT  sw1sw2;

  U8      reader_id;

  T_SIMDRV_cmd_header   cmd_header;
  T_SIMDRV_data_info    data_info;
  T_SIMDRV_result_info  result_info;

  UBYTE   response[SIMDRV_MAX_RESULT];

  TRACE_FUNCTION ("FKT_DisableCHV_n_EnableCHV()");

  reader_id      = SIMDRV_VAL_READER_ID__RANGE_MIN;
  cmd_header.cla = SIMDRV_GSM_CLASS_BYTE;
  cmd_header.ins = inst_code; 
  cmd_header.p1  = 0;
  cmd_header.p2  = 1;

  data_info.data   = (U8 *)pin;
  data_info.c_data = MAX_PIN_LEN;
  
  result_info.result   = response;
  result_info.c_result = size;
  result_info.len      = NOT_PRESENT_16BIT;

  sw1sw2 =  simdrv_xch_apdu(reader_id, cmd_header, data_info, &result_info);

  return FKT_convert_error (sw1sw2,result_info.c_result);
}
  

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
static const  USHORT sim_inv_chv_A [6] =
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
static const  USHORT sim_inv_chv_B [6] =
{
    /* last requested PIN no          error code  */
    /* none                  */       SIM_CAUSE_OTHER_ERROR,
    /* PIN 1                 */       SIM_CAUSE_PIN1_BLOCKED,
    /* PIN 2                 */       SIM_CAUSE_PIN2_BLOCKED,
    /* PUK 1                 */       SIM_CAUSE_PUK1_BLOCKED,
    /* PUK 2                 */       SIM_CAUSE_PUK2_BLOCKED,
    /* NEVER                 */       SIM_CAUSE_OTHER_ERROR
};

GLOBAL USHORT FKT_convert_error (USHORT sw1sw2, USHORT size)
{
  TRACE_FUNCTION ("FKT_convert_error()");

  sim_data.sw1 = (UBYTE)(sw1sw2 >> 8);
  sim_data.sw2 = (UBYTE)sw1sw2;

  TRACE_EVENT_P1 ("Data returned from SIM, Size =%X", size);
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
      /* no break (otherwise the command was successful)*/
    case 0x90:
      if( (size>0) AND (size <= 0x100) )
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
//TISH, patch for OMAPS00115342&OMAPS00123717
//start
#if 0        	
          /* Check whether PIN1 is entered/Verified */
          if (SIM_IS_FLAG_SET(PIN1_VERIFIED))
            return SIM_CAUSE_ACCESS_PROHIBIT;
          else
#endif
//end          	
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
/* Implements Measure# 13 */
  return (FKT_ChangeCHV_n_UnblockCHV (old_pin, new_pin, pin_id, SIMDRV_INS_CHANGE_CHV)); 
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
/* Implements Measure# 17 */
  return (FKT_DisableCHV_n_EnableCHV (pin, SIMDRV_INS_DISABLE_CHV)); 
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
/* Implements Measure# 17 */
  return (FKT_DisableCHV_n_EnableCHV (pin, SIMDRV_INS_ENABLE_CHV)); 
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
    U8           reader_id;
    T_SIMDRV_cmd_header       cmd_header;
    T_SIMDRV_data_info   data_info;
    T_SIMDRV_result_info  result_info;
    UBYTE response[SIMDRV_MAX_RESULT];
    
    TRACE_FUNCTION ("FKT_Increase()");
   reader_id = SIMDRV_VAL_READER_ID__RANGE_MIN;
   cmd_header.cla = SIMDRV_GSM_CLASS_BYTE ;
   cmd_header.ins = SIMDRV_INS_INCREASE;
   cmd_header.p1 = 0;
   cmd_header.p2 = 0;
   
   data_info.data  = (U8 *)data;
   data_info.c_data =  3;
   
   result_info.result = response;
   result_info.c_result = size;
   result_info.len  = NOT_PRESENT_16BIT;
  
   sw1sw2 = simdrv_xch_apdu(reader_id,cmd_header,data_info,&result_info);
  return FKT_convert_error (sw1sw2,result_info.c_result);
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
/* Implements Measure# 16 */
    return (FKT_Invalidate_n_Rehabilitate(SIMDRV_INS_INVALIDATE));
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
    U8           reader_id;
    T_SIMDRV_cmd_header       cmd_header;
    T_SIMDRV_data_info   data_info;
    T_SIMDRV_result_info  result_info;
    U8 offset_high;
    U8 offset_low;
    
    TRACE_FUNCTION ("FKT_ReadBinary()");
  offset_high = (U8)((offset &0x7F00)>>8); /* to make the 8th bit 0 as per 102.221 */ 
  offset_low = (U8)(offset & 0x00FF); 
  
  reader_id = SIMDRV_VAL_READER_ID__RANGE_MIN;
  cmd_header.cla = SIMDRV_GSM_CLASS_BYTE;
  cmd_header.ins = SIMDRV_INS_READ_BINARY;
  cmd_header.p1 =offset_high ;
  cmd_header.p2 =offset_low ;
  
  data_info.data   = NULL;
  data_info.c_data = 0;
  
  result_info.result    = (U8 *)data;
  result_info.c_result = size;
  result_info.len  = (USHORT)length;
  
   sw1sw2= simdrv_xch_apdu (reader_id,cmd_header,data_info,&result_info);
  return FKT_convert_error (sw1sw2,result_info.c_result);
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
    U8           reader_id;
    T_SIMDRV_cmd_header       cmd_header;
    T_SIMDRV_data_info   data_info;
    T_SIMDRV_result_info  result_info;
    
#if !defined NTRACE
/* Implements Measure#32: Row 37 */
  TRACE_EVENT_P1("FKT_ReadRecord(): Nr. %hu", record);
#endif
  
  reader_id = SIMDRV_VAL_READER_ID__RANGE_MIN;
  cmd_header.cla = SIMDRV_GSM_CLASS_BYTE;
  cmd_header.ins = SIMDRV_INS_READ_RECORD;
  cmd_header.p1 = (U8)record;
  cmd_header.p2 = (U8)mode;
  
  data_info.data   = NULL;
  data_info.c_data = 0;
  
  result_info.result   = (U8 *)data ;
  result_info.c_result = size;
  result_info.len  =(USHORT)length;
  
   sw1sw2= simdrv_xch_apdu (reader_id,cmd_header,data_info,&result_info);
  return FKT_convert_error (sw1sw2,result_info.c_result);
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
/* Implements Measure# 16 */
    return (FKT_Invalidate_n_Rehabilitate(SIMDRV_INS_REHABILITATE));
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
  U8  reader_id;
  T_SIMDRV_cmd_header  cmd_header;
  T_SIMDRV_data_info  data_info;
  T_SIMDRV_result_info    result_info;
 
 USHORT cause;
 USHORT fileid;
 union
 {
   T_DIR_STATUS status;
   UBYTE response [40];
 } dir;

  TRACE_FUNCTION ("FKT_Status()");

  memset (dir.response, 0, sizeof(dir.response));
  
  reader_id = SIMDRV_VAL_READER_ID__RANGE_MIN;

  cmd_header.cla = SIMDRV_GSM_CLASS_BYTE;
  cmd_header.ins = SIMDRV_INS_STATUS;
  cmd_header.p1 = 0;
  cmd_header.p2 = 0;
  
  data_info.data   = NULL;
  data_info.c_data = 0;

  result_info.result = (U8 *)dir.response;
  result_info.c_result = size;
  result_info.len = sim_data.dir_status_len;

  sw1sw2 =  simdrv_xch_apdu(reader_id, cmd_header, data_info, &result_info);
  
  cause =  FKT_convert_error (sw1sw2,result_info.c_result);
  
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
  USHORT sw1sw2;
  USHORT size = 0;
    U8           reader_id;
    T_SIMDRV_cmd_header       cmd_header;
    T_SIMDRV_data_info   data_info;
    T_SIMDRV_result_info  result_info;
  
    TRACE_FUNCTION ("FKT_RunGSMAlgo()");
  
  reader_id = SIMDRV_VAL_READER_ID__RANGE_MIN;
  cmd_header.cla = SIMDRV_GSM_CLASS_BYTE ;
  cmd_header.ins = SIMDRV_INS_AUTHENTICATE;
  cmd_header.p1 = 0;
  cmd_header.p2 = 0;
  
  data_info.data   = (U8*)rand;
  data_info.c_data = MAX_RAND;
  
  result_info.result = data;
  result_info.c_result = size;
  result_info.len  = len;
  
  sw1sw2= simdrv_xch_apdu (reader_id,cmd_header,data_info,&result_info);

  return FKT_convert_error (sw1sw2,result_info.c_result);
  }


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6302)       MODULE  : SIM_FKT                    |
| STATE   : code                ROUTINE : FKT_Select                 |
+--------------------------------------------------------------------+

  PURPOSE : Wrapping function for the SIM driver call SIM_Select.

*/
LOCAL USHORT fkt_select_one (USHORT id, UBYTE      * data, USHORT       len)
{

  USHORT size = 0;
  USHORT sw1sw2;
  U8  reader_id;
  T_SIMDRV_cmd_header  cmd_header;
  U8  field[2];
  T_SIMDRV_data_info  data_info;
  T_SIMDRV_result_info    result_info;

  TRACE_EVENT_P1 ("fkt_select_one() : File id =%X ", id);

  reader_id = SIMDRV_VAL_READER_ID__RANGE_MIN;

  cmd_header.cla = SIMDRV_GSM_CLASS_BYTE;
  cmd_header.ins = SIMDRV_INS_SELECT;
  cmd_header.p1 = 0;
  cmd_header.p2 = 0;

  field[0] =(U8) (id>>8);  /* high byte */
  field[1] =(U8) id;       /* low byte */
  
  data_info.data = field;
  data_info.c_data = sizeof(field);
  
  result_info.result = data;
  result_info.c_result = size;
  result_info.len = len;

  TRACE_EVENT_P1 ("Expected result size from SIM =%X ", result_info.len);

  sw1sw2 =  simdrv_xch_apdu(reader_id, cmd_header, data_info, &result_info);  
  
  return FKT_convert_error (sw1sw2,result_info.c_result);
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
GLOBAL USHORT FKT_Select (USHORT       id,
                          BOOL         path_info_present, T_path_info * path_info_ptr,
                          UBYTE      * data, USHORT  len)
{
  USHORT result;
  USHORT directory = (SIM_IS_FLAG_SET (GSM_DATAFIELD))?
                     SIM_DF_GSM: SIM_DF_1800;   /* default parent directory */

#if !defined NTRACE
/* Implements Measure#32: Row 39 */
  TRACE_EVENT_P1("FKT_Select(): id = %04X", id);
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

      /* continue here in case of any first level directory */
  case SIM_DF_GSM:
  case SIM_DF_1800:
  case SIM_DF_TELECOM:
  case SIM_DF_VI:
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
#endif
      case SIM_PNN:
      case SIM_OPL:
        
        /*
         * Access to GSM
         */
        directory = (SIM_IS_FLAG_SET (GSM_DATAFIELD))?
                     SIM_DF_GSM: SIM_DF_1800;
        break;

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
/* Implements Measure# 13 */
  return (FKT_ChangeCHV_n_UnblockCHV (unblockCHV, new_CHV, chvType, SIMDRV_INS_UNBLOCK_CHV)); 
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
    U8           reader_id;
    T_SIMDRV_cmd_header       cmd_header;
    T_SIMDRV_data_info   data_info;
    T_SIMDRV_result_info  result_info;
    U8 offset_high;
    U8 offset_low;
    UBYTE response[SIMDRV_MAX_RESULT];
  
    TRACE_FUNCTION ("FKT_UpdateBinary()");
    
  offset_high = (U8)((offset &0x7F00)>>8);/*to make the 8th bit 0 as per 102.221*/
  offset_low = (U8)(offset & 0x00FF);
  
  reader_id = SIMDRV_VAL_READER_ID__RANGE_MIN;
  cmd_header.cla = SIMDRV_GSM_CLASS_BYTE ;
  cmd_header.ins = SIMDRV_INS_UPDATE_BINARY;
  cmd_header.p1 = offset_high;
  cmd_header.p2 = offset_low;
  
  data_info.data   = (U8 *)data;
  data_info.c_data = (U8)length;
  
  result_info.result = response;
  result_info.c_result = size;
  result_info.len  = NOT_PRESENT_16BIT;
  
  sw1sw2= simdrv_xch_apdu (reader_id,cmd_header,data_info,&result_info);
  return FKT_convert_error (sw1sw2,result_info.c_result);
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
    U8           reader_id;
    T_SIMDRV_cmd_header       cmd_header;
    T_SIMDRV_data_info   data_info;
    T_SIMDRV_result_info  result_info;
    UBYTE response[SIMDRV_MAX_RESULT];
  
#if !defined NTRACE
/* Implements Measure#32: Row 40 */
    TRACE_EVENT_P1("FKT_UpdateRecord(): Nr. %hu", record);
#endif
  reader_id = SIMDRV_VAL_READER_ID__RANGE_MIN;
  cmd_header.cla = SIMDRV_GSM_CLASS_BYTE ;
  cmd_header.ins = SIMDRV_INS_UPDATE_RECORD;
  cmd_header.p1 = (U8)record;
  cmd_header.p2 = (U8)mode;
  
  data_info.data   = (U8*)data;
  data_info.c_data = (U8)length;
  
  result_info.result = response;
  result_info.c_result = size;
  result_info.len  = NOT_PRESENT_16BIT;
  
   sw1sw2= simdrv_xch_apdu (reader_id,cmd_header,data_info,&result_info);
  return FKT_convert_error (sw1sw2,result_info.c_result);
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
  U8  reader_id;
  T_SIMDRV_cmd_header  cmd_header;
  T_SIMDRV_data_info  data_info;
  T_SIMDRV_result_info    result_info;
  UBYTE response[SIMDRV_MAX_RESULT];

  TRACE_FUNCTION ("FKT_VerifyCHV()");

  reader_id = SIMDRV_VAL_READER_ID__RANGE_MIN;

  cmd_header.cla = SIMDRV_GSM_CLASS_BYTE;
  cmd_header.ins = SIMDRV_INS_VERIFY_CHV;
  cmd_header.p1 = 0x00;
  cmd_header.p2 = pin_id;

  data_info.data = (U8 *)pin;
  data_info.c_data = MAX_PIN_LEN;
  
  result_info.result = response;
  result_info.c_result = size;
  result_info.len = NOT_PRESENT_16BIT;

  sw1sw2 =  simdrv_xch_apdu(reader_id, cmd_header, data_info, &result_info);

  return FKT_convert_error (sw1sw2,result_info.c_result);
  }

/*
+********************************************************************+
| Moved from sim_stk.c - for CQ 34109 under feature flag SIM_TOOLKIT |
+********************************************************************+
*/

#ifdef SIM_TOOLKIT

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (8419)       MODULE  : SIM_STK                    |
| STATE   : code                ROUTINE : FKT_TerminalResponse       |
+--------------------------------------------------------------------+

  PURPOSE : Wrapping function for the SIM driver call SIM_TerminalResponse

*/

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

GLOBAL USHORT FKT_TerminalResponse (UBYTE * data,
                                    USHORT  length)
{
      USHORT size = 0;
      USHORT sw1sw2;
      U8 reader_id;
      T_SIMDRV_cmd_header cmd_header;
      T_SIMDRV_data_info data_info;
      T_SIMDRV_result_info result_info;
      UBYTE response[SIMDRV_MAX_RESULT];
      UBYTE env[sizeof(timer_env)];
      USHORT index;
      UBYTE dummy[4];
      USHORT error;
      USHORT i;

      TRACE_FUNCTION ("FKT_TerminalResponse()");

      reader_id = SIMDRV_VAL_READER_ID__RANGE_MIN;
      cmd_header.cla = SIMDRV_GSM_CLASS_BYTE ;
      cmd_header.ins = SIMDRV_INS_TERMINAL_RESPONSE;
      cmd_header.p1 = 0;
      cmd_header.p2 = 0;
    
      data_info.data = data;
      data_info.c_data = (U8)length;
    
      result_info.result = response;
      result_info.c_result = size;
      result_info.len = NOT_PRESENT_16BIT;

      sw1sw2 = simdrv_xch_apdu(reader_id,cmd_header,data_info,&result_info);

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

  return FKT_convert_error (sw1sw2,result_info.c_result);

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
      U8 reader_id;
      T_SIMDRV_cmd_header cmd_header;
      T_SIMDRV_data_info data_info;
      T_SIMDRV_result_info result_info;

     TRACE_FUNCTION ("FKT_Envelope()");
  
     reader_id = SIMDRV_VAL_READER_ID__RANGE_MIN;
 
     cmd_header.cla = SIMDRV_GSM_CLASS_BYTE;
     cmd_header.ins = SIMDRV_INS_ENVELOPE;
     cmd_header.p1 = 0;
     cmd_header.p2 = 0;
 
     data_info.data = data_in;
     data_info.c_data = (U8)in_length;

     result_info.result = data_out;
     result_info.c_result = size;
     result_info.len = out_length;
 
     sw1sw2 = simdrv_xch_apdu(reader_id,cmd_header,data_info,&result_info);

  SIM_EM_ENVELOPE;

  sim_data.sim_data_len = 0;
  TRACE_EVENT("sim_data.sim_data_len initialised as 0"); /* for debug only - to be removed */

  error = FKT_convert_error (sw1sw2,result_info.c_result);

  stk_l_cmd = sim_data.sim_data_len << 3;

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
      U8 reader_id;
      T_SIMDRV_cmd_header cmd_header;
      T_SIMDRV_data_info data_info;
      T_SIMDRV_result_info result_info;
      UBYTE response[SIMDRV_MAX_RESULT];

      TRACE_FUNCTION ("FKT_TerminalProfile()");

      reader_id = SIMDRV_VAL_READER_ID__RANGE_MIN;
      cmd_header.cla = SIMDRV_GSM_CLASS_BYTE;
      cmd_header.ins = SIMDRV_INS_TERMINAL_PROFILE;
      cmd_header.p1 = 0;
      cmd_header.p2 = 0;
    
      data_info.data = data;
      data_info.c_data = (U8)length;
    
      result_info.result = response;
      result_info.c_result = size;
      result_info.len = NOT_PRESENT_16BIT;

      sw1sw2= simdrv_xch_apdu(reader_id,cmd_header,data_info,&result_info);

  return FKT_convert_error (sw1sw2,result_info.c_result);
  
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
      U8 reader_id;
      T_SIMDRV_cmd_header cmd_header;
      T_SIMDRV_data_info data_info;
      T_SIMDRV_result_info result_info;

     TRACE_FUNCTION ("FKT_Fetch()");

     reader_id = SIMDRV_VAL_READER_ID__RANGE_MIN;
     cmd_header.cla = SIMDRV_GSM_CLASS_BYTE;
     cmd_header.ins = SIMDRV_INS_FETCH;
     cmd_header.p1 = 0;
     cmd_header.p2 = 0;

     data_info.data = NULL;
     data_info.c_data = 0;

     result_info.result = cmd;
     result_info.c_result = size;
     result_info.len = (USHORT)length;

     sw1sw2 = simdrv_xch_apdu(reader_id,cmd_header,data_info,&result_info);
  
  return FKT_convert_error (sw1sw2,result_info.c_result);
  
}

#endif /* SIM_TOOLKIT */

#endif
