/************* Revision Controle System Header *************
 *                  GSM Layer 1 software 
 * L1_MACRO.H
 *
 *        Filename l1_macro.h
 *  Copyright 2003 (C) Texas Instruments  
 *
 ************* Revision Controle System Header *************/

#include "l1_confg.h"

#if(L1_DYN_DSP_DWNLD == 1)
  #include "../dyn_dwl_include/l1_dyn_dwl_const.h"
#endif

#if (TRACE_TYPE==5) && NUCLEUS_TRACE
//WARNING : this type of trace takes a lot of space in data RAM (~16kB)

  // switch for Nucleus debugging messages.
  #define NU_ALLOC_ERR      0
  #define NU_DEALLOC_ERR    1
  #define NU_RCVE_QUEUE_ERR 2
  #define NU_SEND_QUEUE_ERR 3
  #define NU_OBTA_SEMA_ERR  4
  #define NU_RLSE_SEMA_ERR  5

  // Nucleus debug function.
    #define DEBUGMSG(status,type) \
    if(status) switch(type) \
    { \
      case NU_ALLOC_ERR: \
      printf("NU mem. allocation error %d file %s line %d\n", status,__FILE__,__LINE__); \
      exit(0);            \
      break; \
      \
      case NU_DEALLOC_ERR: \
      printf("NU mem. deallocation error %d file %s line %d\n", status,__FILE__,__LINE__); \
      exit(0);            \
      break; \
      \
      case NU_RCVE_QUEUE_ERR: \
      printf("NU rcve queue error %d file %s line %d\n", status,__FILE__,__LINE__); \
      exit(0);            \
      break; \
      \
      case NU_SEND_QUEUE_ERR: \
      printf("NU send queue error %d file %s line %d\n", status,__FILE__,__LINE__); \
      exit(0);            \
      break; \
      \
      case NU_OBTA_SEMA_ERR: \
      printf("NU obtain semaph. error %d file %s line %d\n", status,__FILE__,__LINE__); \
      exit(0);            \
      break; \
      \
      case NU_RLSE_SEMA_ERR: \
      printf("NU release semaph. error %d file %s line %d\n", status,__FILE__,__LINE__); \
      exit(0);            \
      break; \
      \
      default: \
      printf("Unknown error %d file %s line %d\n", status,__FILE__,__LINE__); \
      exit(0);            \
      break; \
    }
#else
  #define DEBUGMSG(status,type)
#endif

/************************************************************/
/* Macros for FAST INTEGER MODULO implementation.           */
/************************************************************/
#define IncMod(operand, increment, modulo) \
  if( (operand += increment) >= modulo ) operand -= modulo
  

// Define MACRO for selecting the min. time to next task.
#define Select_min_time(Task_Time, Min_Time) \
  if(Task_Time < Min_Time) Min_Time = Task_Time;


/************************************************************/
/* Macros for MCU/DSP API address conversion    .           */
/************************************************************/
#if(L1_DYN_DSP_DWNLD == 1)

#define API_address_dsp2mcu(dsp_address) \
  (MCU_API_BASE_ADDRESS + ((API)((dsp_address) - DSP_API_BASE_ADDRESS) * 2))

#define API_address_mcu2dsp(mcu_address) \
  (DSP_API_BASE_ADDRESS + ((UWORD32)((mcu_address) - MCU_API_BASE_ADDRESS) / 2))
#endif

