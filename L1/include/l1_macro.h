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
#include "l1_types.h"

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

#define API_address_dsp2mcu(dsp_address) \
  (MCU_API_BASE_ADDRESS + ((API)((dsp_address) - DSP_API_BASE_ADDRESS) * 2))

#define API_address_mcu2dsp(mcu_address) \
  (DSP_API_BASE_ADDRESS + ((UWORD32)((mcu_address) - MCU_API_BASE_ADDRESS) / 2))




  /* Added temporirly for RF_KEypad build */

  #if (L1_RF_KBD_FIX == 1)

  #if(OP_L1_STANDALONE == 1)

#if 0
  typedef struct
  {
  //   T_RVF_MB_ID    prim_id;
  //   T_RVF_ADDR_ID  addr_id;
  //   BOOL           swe_is_initialized;
  //   T_RVM_RETURN   (*error_ft)(T_RVM_NAME        swe_name,
  //                              T_RVM_RETURN      error_cause,
  //                              T_RVM_ERROR_TYPE  error_type,
  //                              T_RVM_STRING      error_msg);
  #if ((CHIPSET == 12) || (CHIPSET == 15))
  //  T_KPD_RECEIVED_KEY_INFO received_key_info[KPD_MAX_DETECTABLE];
    //UINT8                   nb_active_keys;
    UWORD16                  repeat_time;
    UWORD16                  long_time;
  #endif
  } T_KPD_ENV_CTRL_BLK_L1;

#endif

//  typedef  unsigned char KPD_CORRECTION_RATIO; //UWORD8 //omaps00090550
  void kpd_timer_modify(UWORD8 ratio,UWORD32 frameNumber); //omaps00090550
  #define KBR_DEBOUNCING_TIME           (MEM_KEYBOARD + 0x02) /* KBR debouncing time reg */
  #define KPD_DEBOUNCING_TIME   (0x3F)
  #define KBR_LONG_KEY_TIME             (MEM_KEYBOARD + 0x04) /* KBR long key time reg */
  #define KBR_TIME_OUT                  (MEM_KEYBOARD + 0x06) /* KBR Time out reg */
  #define KBR_CTRL_REG                  (MEM_KEYBOARD + 0x00) /* KBR control reg */
  #define KBR_STATE_MACHINE_STATUS      (MEM_KEYBOARD + 0x0E) /* KBR state machine status reg */
  #define KPD_CLK_DIV32 4
  #define KPD_CLOCK_DIVIDER     KPD_CLK_DIV32

    #define SetGroupBits16(registre,position,number,value) {\
                                                        UINT16 tmp=registre;\
                                                        volatile UINT16 tmpvalue;\
                                                        tmpvalue = (value<<(16-(number)));\
                                                        tmpvalue = (tmpvalue>>(16-(number)));\
                                                        tmp&=~((0xFFFF>>(16-(number)))<<(position));\
                                                        tmp|=((tmpvalue&(0xFFFF>>(16-(number))))<<(position));\
                                                        registre=tmp;\
                                                        }
  #endif/* #if(OP_L1_STANDALONE == 1) */

  #endif /* #if (L1_RF_KBD_FIX == 1) */
  /* Added temporirly for RF_KEypad build */






