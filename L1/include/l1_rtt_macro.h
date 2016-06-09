/************* Revision Controle System Header *************
 *                  GSM Layer 1 software 
 * L1_RTT_MACRO.H
 *
 *        Filename %M%
 *  Copyright 2003 (C) Texas Instruments  
 *
 ************* Revision Controle System Header *************/

#if (defined RVM_RTT_SWE || (OP_L1_STANDALONE == 1))

#include "rvt_gen.h"
#include "rtt_gen.h"

extern T_TRACE_INFO_STRUCT trace_info;
                                                                    
/***********************************************************************************************************/
/* Macro for cell enabling checking                                                                        */
/***********************************************************************************************************/

#define SELECTED_BITMAP(enable_bit) \
  enable_bit < 32  ? (0x0001 << (enable_bit -   0)) & trace_info.current_config->rttl1_cell_enable[0] : \
  enable_bit < 64  ? (0x0001 << (enable_bit -  32)) & trace_info.current_config->rttl1_cell_enable[1] : \
  enable_bit < 96  ? (0x0001 << (enable_bit -  64)) & trace_info.current_config->rttl1_cell_enable[2] : \
  enable_bit < 128 ? (0x0001 << (enable_bit -  96)) & trace_info.current_config->rttl1_cell_enable[3] : \
  enable_bit < 160 ? (0x0001 << (enable_bit - 128)) & trace_info.current_config->rttl1_cell_enable[4] : \
  enable_bit < 192 ? (0x0001 << (enable_bit - 160)) & trace_info.current_config->rttl1_cell_enable[5] : \
  enable_bit < 224 ? (0x0001 << (enable_bit - 192)) & trace_info.current_config->rttl1_cell_enable[6] : \
                     (0x0001 << (enable_bit - 224)) & trace_info.current_config->rttl1_cell_enable[7]

/***********************************************************************************************************/
/* Macros for buffer filling                                                                               */
/***********************************************************************************************************/

//-----------------------------------------------------------------------------------------------------------
// L1 RTT cell filling: FN
                                     
#define RTTL1_FILL_FN(param1) \
  if(SELECTED_BITMAP(RTTL1_ENABLE_FN)) \
  { \
      T_RTT_PTR  *ptr; \
      T_RTTL1_FN *ptr2; \
\
      ptr = (T_RTT_PTR *) &ptr2; \
      if ((*ptr = trace_info.l1s_rtt_func.rtt_get_fill_ptr(trace_info.l1s_trace_user_id, sizeof(T_RTTL1_FN))) != NULL) \
      { \
        ptr2->fn          = param1; \
        ptr2->cell_id     = RTTL1_ENABLE_FN; \
      } \
  }

//-----------------------------------------------------------------------------------------------------------
// L1 RTT cell filling: Downlink burst
                                     
#define RTTL1_FILL_DL_BURST(param1,param2,param3,param4,param5,param6,param7) \
  if(SELECTED_BITMAP(RTTL1_ENABLE_DL_BURST))                                                                  \
  { \
      T_RTT_PTR  *ptr; \
      T_RTTL1_DL_BURST *ptr2; \
\
      ptr = (T_RTT_PTR *) &ptr2; \
      if ((*ptr = trace_info.l1s_rtt_func.rtt_get_fill_ptr(trace_info.l1s_trace_user_id, sizeof(T_RTTL1_DL_BURST))) != NULL) \
      { \
        ptr2->angle       = param1; \
        ptr2->snr         = param2; \
        ptr2->afc         = param3; \
        ptr2->task        = param4; \
        ptr2->pm          = param5; \
        ptr2->toa         = param6; \
        ptr2->input_level = param7; \
        ptr2->cell_id     = RTTL1_ENABLE_DL_BURST; \
      } \
  }                                                                                                           
                                                                                                              
//----------------------------------------------------------------------------------------------------------- 
// L1 RTT cell filling: Uplink Normal Burst                                                                   
                                                                                                              
#define RTTL1_FILL_UL_NB(param1, param2, param3) \
  if(SELECTED_BITMAP(RTTL1_ENABLE_UL_NB)) \
  { \
      T_RTT_PTR  *ptr; \
      T_RTTL1_UL_NB *ptr2; \
\
      ptr = (T_RTT_PTR *) &ptr2; \
      if ((*ptr = trace_info.l1s_rtt_func.rtt_get_fill_ptr(trace_info.l1s_trace_user_id, sizeof(T_RTTL1_UL_NB))) != NULL) \
      { \
        ptr2->task    = param1; \
        ptr2->ta      = param2; \
        ptr2->txpwr   = param3; \
        ptr2->cell_id = RTTL1_ENABLE_UL_NB; \
      } \
  }                                                                                                           
                                                                                                              
//----------------------------------------------------------------------------------------------------------- 
// L1 RTT cell filling: Uplink Access Burst                                                                   
                                                                                                              
#define RTTL1_FILL_UL_AB(param1, param2) \
  if(SELECTED_BITMAP(RTTL1_ENABLE_UL_AB)) \
  { \
      T_RTT_PTR  *ptr; \
      T_RTTL1_UL_AB *ptr2; \
\
      ptr = (T_RTT_PTR *) &ptr2; \
      if ((*ptr = trace_info.l1s_rtt_func.rtt_get_fill_ptr(trace_info.l1s_trace_user_id, sizeof(T_RTTL1_UL_AB))) != NULL) \
      { \
        ptr2->task    = param1; \
        ptr2->txpwr   = param2; \
        ptr2->cell_id = RTTL1_ENABLE_UL_AB; \
      } \
  }                                                                                                           
                                                                                                              
//----------------------------------------------------------------------------------------------------------- 
// L1 RTT cell filling: Full list measurement                                                                           
                                                                                                              
#define RTTL1_FILL_FULL_LIST_MEAS(param1, param2, param3, param4) \
  if(SELECTED_BITMAP(RTTL1_ENABLE_FULL_LIST_MEAS)) \
  { \
      T_RTT_PTR  *ptr; \
      T_RTTL1_FULL_LIST_MEAS *ptr2; \
\
      ptr = (T_RTT_PTR *) &ptr2; \
      if ((*ptr = trace_info.l1s_rtt_func.rtt_get_fill_ptr(trace_info.l1s_trace_user_id, sizeof(T_RTTL1_FULL_LIST_MEAS))) != NULL) \
      {                                                                                                       \
        ptr2->pm          = param1; \
        ptr2->input_level = param2; \
        ptr2->task        = param3; \
        ptr2->radio_freq  = param4; \
        ptr2->cell_id     = RTTL1_ENABLE_FULL_LIST_MEAS; \
      } \
  }                                                                                                           
                                                                                                              
//----------------------------------------------------------------------------------------------------------- 
// L1 RTT cell filling: Full list measurement                                                                           
                                                                                                              
#define RTTL1_FILL_MON_MEAS(param1, param2, param3, param4) \
  if(SELECTED_BITMAP(RTTL1_ENABLE_MON_MEAS)) \
  { \
      T_RTT_PTR  *ptr; \
      T_RTTL1_MON_MEAS *ptr2; \
\
      ptr = (T_RTT_PTR *) &ptr2; \
      if ((*ptr = trace_info.l1s_rtt_func.rtt_get_fill_ptr(trace_info.l1s_trace_user_id, sizeof(T_RTTL1_MON_MEAS))) != NULL) \
      { \
        ptr2->pm          = param1; \
        ptr2->input_level = param2; \
        ptr2->task        = param3; \
        ptr2->radio_freq  = param4; \
        ptr2->cell_id     = RTTL1_ENABLE_MON_MEAS; \
      } \
  }                                                                                                           

//----------------------------------------------------------------------------------------------------------- 
// L1 RTT cell filling: Downlink DCCH block                                                                   
                                                                                                              
#define RTTL1_FILL_DL_DCCH(param1, param2) \
  if(SELECTED_BITMAP(RTTL1_ENABLE_DL_DCCH)) \
  { \
      T_RTT_PTR  *ptr; \
      T_RTTL1_DL_DCCH *ptr2; \
\
      ptr = (T_RTT_PTR *) &ptr2; \
      if ((*ptr = trace_info.l1s_rtt_func.rtt_get_fill_ptr(trace_info.l1s_trace_user_id, sizeof(T_RTTL1_DL_DCCH))) != NULL) \
      { \
        ptr2->valid_flag    = param1; \
        ptr2->physical_info = param2; \
        ptr2->cell_id       = RTTL1_ENABLE_DL_DCCH; \
      } \
  }                                                                                                           
                                                                                                              
//----------------------------------------------------------------------------------------------------------- 
// L1 RTT cell filling: Downlink PTCCH block                                                                  
                                                                                                              
#define RTTL1_FILL_DL_PTCCH(param1, param2) \
  if(SELECTED_BITMAP(RTTL1_ENABLE_DL_PTCCH)) \
  { \
      T_RTT_PTR  *ptr; \
      T_RTTL1_DL_PTCCH *ptr2; \
\
      ptr = (T_RTT_PTR *) &ptr2; \
      if ((*ptr = trace_info.l1s_rtt_func.rtt_get_fill_ptr(trace_info.l1s_trace_user_id, sizeof(T_RTTL1_DL_PTCCH))) != NULL) \
      { \
        ptr2->crc        = param1; \
        ptr2->ordered_ta = param2; \
        ptr2->cell_id    = RTTL1_ENABLE_DL_PTCCH; \
      } \
  }                                                                                                           
                                                                                                              
//----------------------------------------------------------------------------------------------------------- 
// L1 RTT cell filling: Uplink DCCH block                                                                     
                                                                                                              
#define RTTL1_FILL_UL_DCCH \
  if(SELECTED_BITMAP(RTTL1_ENABLE_UL_DCCH)) \
  { \
      T_RTT_PTR  *ptr; \
      T_RTTL1_UL_DCCH *ptr2; \
\
      ptr = (T_RTT_PTR *) &ptr2; \
      if ((*ptr = trace_info.l1s_rtt_func.rtt_get_fill_ptr(trace_info.l1s_trace_user_id, sizeof(T_RTTL1_UL_DCCH))) != NULL) \
      { \
        ptr2->cell_id = RTTL1_ENABLE_UL_DCCH; \
      } \
  }                                                                                                           
                                                                                                              
//----------------------------------------------------------------------------------------------------------- 
// L1 RTT cell filling: Uplink SACCH block                                                                    
                                                                                                              
#define RTTL1_FILL_UL_SACCH(param1, param2, param3) \
  if(SELECTED_BITMAP(RTTL1_ENABLE_UL_SACCH)) \
  { \
      T_RTT_PTR  *ptr; \
      T_RTTL1_UL_SACCH *ptr2; \
\
      ptr = (T_RTT_PTR *) &ptr2; \
      if ((*ptr = trace_info.l1s_rtt_func.rtt_get_fill_ptr(trace_info.l1s_trace_user_id, sizeof(T_RTTL1_UL_SACCH))) != NULL) \
      { \
        ptr2->data_present   = param1; \
        ptr2->reported_ta    = param2; \
        ptr2->reported_txpwr = param3; \
        ptr2->cell_id        = RTTL1_ENABLE_UL_SACCH; \
      } \
  }                                                                                                           
                                                                                                              
//----------------------------------------------------------------------------------------------------------- 
// L1 RTT cell filling: Downlink PDTCH block                                                                  
                                                                                                              
#define RTTL1_FILL_DL_PDTCH(param1, param2, param3, param4, param5) \
  if(SELECTED_BITMAP(RTTL1_ENABLE_DL_PDTCH)) \
  { \
      T_RTT_PTR  *ptr; \
      T_RTTL1_DL_PDTCH *ptr2; \
\
      ptr = (T_RTT_PTR *) &ptr2; \
      if ((*ptr = trace_info.l1s_rtt_func.rtt_get_fill_ptr(trace_info.l1s_trace_user_id, sizeof(T_RTTL1_DL_PDTCH))) != NULL) \
      { \
        ptr2->mac_header = param1; \
        ptr2->tfi_result = param2; \
        ptr2->crc        = param3; \
        ptr2->cs_type    = param4; \
        ptr2->timeslot   = param5; \
        ptr2->cell_id    = RTTL1_ENABLE_DL_PDTCH; \
      } \
  }                                                                                                           
                                                                                                              
//----------------------------------------------------------------------------------------------------------- 
// L1 RTT cell filling: Uplink PDTCH block                                                                    
                                                                                                              
#define RTTL1_FILL_UL_PDTCH(param1, param2, param3) \
  if(SELECTED_BITMAP(RTTL1_ENABLE_UL_PDTCH)) \
  { \
      T_RTT_PTR  *ptr; \
      T_RTTL1_UL_PDTCH *ptr2; \
\
      ptr = (T_RTT_PTR *) &ptr2; \
      if ((*ptr = trace_info.l1s_rtt_func.rtt_get_fill_ptr(trace_info.l1s_trace_user_id, sizeof(T_RTTL1_UL_PDTCH))) != NULL) \
      { \
        ptr2->cs_type      = param1; \
        ptr2->data_allowed = param2; \
        ptr2->timeslot     = param3; \
        ptr2->cell_id      = RTTL1_ENABLE_UL_PDTCH; \
      } \
  }                                                                                                           
                                                                                                              
//----------------------------------------------------------------------------------------------------------- 
// L1 RTT cell filling: MAC-S error                                                                           
                                                                                                              
#define RTTL1_FILL_MACS_STATUS(param1, param2) \
  if(SELECTED_BITMAP(RTTL1_ENABLE_MACS_STATUS)) \
  { \
      T_RTT_PTR  *ptr; \
      T_RTTL1_MACS_STATUS *ptr2; \
\
      ptr = (T_RTT_PTR *) &ptr2; \
      if ((*ptr = trace_info.l1s_rtt_func.rtt_get_fill_ptr(trace_info.l1s_trace_user_id, sizeof(T_RTTL1_MACS_STATUS))) != NULL) \
      { \
        ptr2->status   = param1; \
        ptr2->timeslot = param2; \
        ptr2->cell_id  = RTTL1_ENABLE_MACS_STATUS; \
      } \
  }

//----------------------------------------------------------------------------------------------------------- 
// L1 RTT cell filling: L1S task enable                                                                       
                                                                                                              
#define RTTL1_FILL_L1S_TASK_ENABLE(param1, param2) \
  if(SELECTED_BITMAP(RTTL1_ENABLE_L1S_TASK_ENABLE)) \
  { \
      T_RTT_PTR  *ptr; \
      T_RTTL1_L1S_TASK_ENABLE *ptr2; \
\
      ptr = (T_RTT_PTR *) &ptr2; \
      if ((*ptr = trace_info.l1s_rtt_func.rtt_get_fill_ptr(trace_info.l1s_trace_user_id, sizeof(T_RTTL1_L1S_TASK_ENABLE))) != NULL) \
      { \
        ptr2->bitmap1  = param1; \
        ptr2->bitmap2  = param2; \
        ptr2->cell_id  = RTTL1_ENABLE_L1S_TASK_ENABLE; \
      } \
  }

//----------------------------------------------------------------------------------------------------------- 
// L1 RTT cell filling: MFTAB trace                                                                       
                                                                                                              
#define RTTL1_FILL_MFTAB(param1) \
  if(SELECTED_BITMAP(RTTL1_ENABLE_MFTAB)) \
  { \
      T_RTT_PTR  *ptr; \
      T_RTTL1_MFTAB *ptr2; \
\
      ptr = (T_RTT_PTR *) &ptr2; \
      if ((*ptr = trace_info.l1s_rtt_func.rtt_get_fill_ptr(trace_info.l1s_trace_user_id, sizeof(T_RTTL1_MFTAB))) != NULL) \
      { \
        ptr2->func     = param1; \
        ptr2->cell_id  = RTTL1_ENABLE_MFTAB; \
      } \
  }

/***********************************************************************************************************/ 
/* Macro for events                                                                                        */ 
/***********************************************************************************************************/
                                                                                                              
#define RTTL1_EVENT(id,size) \
  if (trace_info.current_config->rttl1_event_enable & (0x1 << id)) \
    trace_info.l1s_rtt_func.rtt_dump_buffer(trace_info.l1s_trace_user_id, size);
#else // RVM_RTT_SWE || OP_L1_STANDALONE

// No RTT: all macros are empty
#define SELECTED_BITMAP(enable_bit) (0)                                   
#define RTTL1_FILL_FN(param1)                                      
#define RTTL1_FILL_DL_BURST(param1,param2,param3,param4,param5,param6,param7)                                                                                                               
#define RTTL1_FILL_UL_NB(param1, param2, param3)                                                                                                               
#define RTTL1_FILL_UL_AB(param1, param2)                                                                                                               
#define RTTL1_FILL_FULL_LIST_MEAS(param1, param2, param3, param4)                                                                                                               
#define RTTL1_FILL_MON_MEAS(param1, param2, param3, param4)                                                                                                               
#define RTTL1_FILL_DL_DCCH(param1, param2)                                                                                                               
#define RTTL1_FILL_DL_PTCCH(param1, param2)                                                                                                               
#define RTTL1_FILL_UL_DCCH                                                                                                               
#define RTTL1_FILL_UL_SACCH(param1, param2, param3)                                                                                                              
#define RTTL1_FILL_DL_PDTCH(param1, param2, param3, param4, param5)                                                                                                              
#define RTTL1_FILL_UL_PDTCH(param1, param2, param3)                                                                                                              
#define RTTL1_FILL_MACS_STATUS(param1, param2)                                                                                                              
#define RTTL1_FILL_L1S_TASK_ENABLE(param1, param2)                                                                                                              
#define RTTL1_FILL_MFTAB(param1)
#define RTTL1_EVENT(id,size)

#endif // RVM_RTT_SWE || OP_L1_STANDALONE
