/******************************************************************************
 *                                                                            *
 *  Name        create_RVtasks.c                                              *
 *                                                                            *
 *  Function    this file contains functions allowing tasks creation in       *
 *              the Riviera environment                                       *
 *                                                                            *
 *  Version     0.1                                                           *
 *                                                                            *
 *  Date        Modification                                                  *
 *  ------------------------------------                                      *
 *  03 August 2000      Create                                                *
 *                                                                            *
 *  Author      Pascal Puel                                                   *
 *                                                                            *
 * (C) Copyright 2000 by Texas Instruments Incorporated, All Rights Reserved  *
 *                                                                            *
 * -------------------------------------------------------------------------- *
 *                                                                            *
 *   History:                                                                 *
 *                                                                            *
 *   10/18/2001 - Updated for R2D by Christophe Favergeon                     *
 *   08/28/2002 - Clean-Up by Gerard Cauvy                                    *
 *                                                                            *
 *****************************************************************************/

#include "../../include/config.h"

#include "../rv/rv_general.h"
#include "../rvf/rvf_api.h"
#include "../rvm/rvm_api.h"
#include "../rvm/rvm_use_id_list.h"
#include "../rvt/rvt_gen.h"

#if 0
//sys_types.h is necessary for function prototypes in buzzer.h
#include "sys_types.h"
#include "buzzer/buzzer.h"
#endif

#include "../rv/rv_defined_swe.h"

#if 0 //#ifndef _WINDOWS
#  include "power/power.h"
#endif

#include <stdio.h>
#include <string.h>

#define START_TASK_ID      (MAX_RVF_TASKS-1)
#define RV_START_TASK_PRIO (249)

T_RVT_USER_ID rv_trace_user_id = 0xff;
T_RVT_USER_ID etm_trace_user_id;

extern void etm_receive(unsigned char *inbuf, unsigned short size);

#ifdef MIXED_TRACE
  T_RVT_USER_ID l23_trace_user_id;
  extern void ext_processExtInput (T_RVT_BUFFER, UINT16);
#endif

#ifdef RVM_RNET_BR_SWE
  T_RVT_USER_ID rnet_trace_user_id;
  extern void rnet_receive (UINT8 *inbuf, UINT16 size);
#endif

#if (TEST==1)

  // The name that uniquely identifies the Memory Bank MUST be
  // 'TEST1', whereas it might be used by some other software
  // entity for testing purpose.
  #define RVTEST_MENU_MB_NAME          ("TEST1")

  // Memory requirements.
  #define RVTEST_MENU_MB_SIZE          (5000)
  #define RVTEST_MENU_MB_WATERMARK     (4000)

  extern void rv_test (UINT32 p);
#endif


/*******************************************************************************
**
** Function         rvt_init_trace
**
** Description      This function is called by the RV_START task to register
**                  the Riviera Frame in the trace module
**
** Returns          void
**
*******************************************************************************/
void rvt_init_trace (void)
{
    rvt_register_id ("RV", &rv_trace_user_id, rvt_set_trace_level);
}

#ifdef RVM_ETM_SWE
/*******************************************************************************
**
** Function         etm_init_trace
**
** Description      This function is called by the RV_START task to register
**                  the ETM in the trace module
**
** Returns          void
**
*******************************************************************************/
void etm_init_trace (void)
{
    extern T_RVT_USER_ID tm_trace_user_id;

    rvt_register_id("TM", &etm_trace_user_id, etm_receive);

#if 0 //(PSP_STANDALONE != 1)
    tm_trace_user_id = etm_trace_user_id; // TML1 use the tm_trace_user_id
#endif
}
#endif

#ifdef MIXED_TRACE
/*******************************************************************************
**
** Function         l23_init_trace
**
** Description      This function is called by the RV_START task to register
**                  the Protocol Stack (Layers 2 & 3) in the trace module
**
** Returns          void
**
*******************************************************************************/
void l23_init_trace (void)
{
    rvt_register_id ("L23", &l23_trace_user_id, ext_processExtInput);
}
#endif

#ifdef RVM_RNET_BR_SWE
/*******************************************************************************
**
** Function         rnet_init_trace
**
** Description      This function is called by the RV_START task to register
**                  RNET in the trace module
**
** Returns          void
**
*******************************************************************************/
void rnet_init_trace (void)
{
    rvt_register_id ("RNET", &rnet_trace_user_id, rnet_receive);
}
#endif


/*******************************************************************************
**
** Function         rv_start_swe_and_check
**
** Description      This internal function is called by the stater task to
**                  start the basic SWEs in the system and to check if
**                  they started successfully or not.
**
** Returns          void
**
*******************************************************************************/
BOOLEAN rv_start_swe_and_check (T_RVM_USE_ID swe_use_id, T_RVM_NAME swe_name)
{
    T_RV_RETURN return_path    = {0};
    T_RV_HDR    *msg_ptr       = NULL;
    UINT16      rec_evt        = 0;
    char        error_msg[150] = "";

    /* temporary initialization of addr_id */
    return_path.addr_id = START_TASK_ID;
    return_path.callback_func = NULL;

    /* attempt to initialize the required SWE */
    if (rvm_start_swe (swe_use_id, return_path) != RVM_OK)
    {
        sprintf (error_msg,
                 "create_RVtasks: Unable to start %s (0x%.8x). Error in rvm_start_swe",
                 (char *)swe_name,
                 swe_use_id);
        rvf_send_trace ((char *)error_msg,
                        strlen((char *)error_msg),
                        NULL_PARAM,
                        RV_TRACE_LEVEL_WARNING, RVM_USE_ID);
        return FALSE;
    }

    /*
     * wait for the SWE to be actually started.
     * note that the 'RVM_EVT_TO_APPLI' notification is sent back
     * once xxx_start () is invoked.
     */
    while (rec_evt = rvf_evt_wait (START_TASK_ID, \
                                   0xFFFF, \
                                   0xFFFFFFFFL))
    {
        if (rec_evt & ~RVF_TASK_MBOX_0_EVT_MASK)
        {
            sprintf (error_msg,
                     "create_RVtasks: Starting %s (0x%.8x). Event ",
                     (char *)swe_name,
                     swe_use_id);
            rvf_send_trace ((char *)error_msg,
                            strlen((char *)error_msg),
                            (UINT32)rec_evt,
                            RV_TRACE_LEVEL_WARNING,
                            RVM_USE_ID);
        }
        if (rec_evt & RVF_TASK_MBOX_0_EVT_MASK)
        {
            if ((msg_ptr = (T_RV_HDR *) rvf_read_addr_mbox (START_TASK_ID, \
                                                            RVF_TASK_MBOX_0)) == NULL)
            {
                sprintf (error_msg,
                         "create_RVtasks: Starting %s (0x%.8x). Message NULL",
                         (char *)swe_name,
                         swe_use_id);
                rvf_send_trace ((char *)error_msg,
                                strlen((char *)error_msg),
                                NULL_PARAM,
                                RV_TRACE_LEVEL_WARNING,
                                RVM_USE_ID);
                continue;
            }
            if (msg_ptr->msg_id != RVM_EVT_TO_APPLI)
            {
                sprintf (error_msg,
                         "create_RVtasks: Starting %s (0x%.8x). Message ID ",
                         (char *)swe_name,
                         swe_use_id);
                rvf_send_trace ((char *)error_msg,
                                strlen((char *)error_msg),
                                msg_ptr->msg_id,
                                RV_TRACE_LEVEL_WARNING,
                                RVM_USE_ID);
                rvf_free_buf (msg_ptr);
                continue;
            }
            break;
        }
    }
    switch (((T_RVM_APPLI_RESULT *)msg_ptr)->result)
    {
        case RVM_OK:
            {
                sprintf (error_msg,
                         "create_RVtasks: %s (0x%.8x) started",
                         (char *)swe_name,
                         swe_use_id);
                rvf_send_trace ((char *)error_msg,
                                strlen ((char *)error_msg),
                                NULL_PARAM,
                                RV_TRACE_LEVEL_DEBUG_HIGH,
                                RVM_USE_ID);
                rvf_free_buf (msg_ptr);
                return TRUE;
            }
        case RVM_NOT_READY:
            {
                sprintf (error_msg,
                         "create_RVtasks: %s (0x%.8x) already started",
                         (char *)swe_name,
                         swe_use_id);
                rvf_send_trace ((char *)error_msg,
                                strlen ((char *)error_msg),
                                NULL_PARAM,
                                RV_TRACE_LEVEL_DEBUG_MEDIUM,
                                RVM_USE_ID);
                rvf_free_buf (msg_ptr);
                return TRUE;
            }
        default:
            {
                break;
            }
    }
    sprintf (error_msg,
             "create_RVtasks: Unable to start %s (0x%.8x). Error ",
             (char *)swe_name,
             swe_use_id);
    rvf_send_trace ((char *)error_msg,
                    strlen ((char *)error_msg),
                    ((T_RVM_APPLI_RESULT *)msg_ptr)->result,
                    RV_TRACE_LEVEL_WARNING,
                    RVM_USE_ID);
    rvf_free_buf (msg_ptr);
    return FALSE;
}

/*******************************************************************************
**
** Function         rv_start
**
** Description      This function is called by the RV_START task. It starts the
**                  Riviera environment and the TRACE task. This start must be
**                  done after Application_initialize().
**
** Returns          void
**
*******************************************************************************/
void rv_start (void)
{
#if (TEST==1)
    T_RVF_MB_ID     mb_id           = RVF_INVALID_MB_ID;
    T_RVF_MB_PARAM  mb_requirements = {0};
    volatile UINT16 result          = 0;
#endif

    /* initialize the RVM and the RVF at the same time */
    rvm_start_environment ();
    /*
    ** Init trace module
    */
    rvt_init_trace ();
#ifdef RVM_ETM_SWE
    etm_init_trace ();
#endif

#if CONFIG_GSM
    #ifdef MIXED_TRACE
      l23_init_trace ();
    #endif
#endif // if (_GSM==1)

#ifdef RVM_RNET_BR_SWE
    rnet_init_trace ();
#endif

#if (REMU==1)
#ifdef RVM_LLS_SWE
    /* initialize LLS SWE */
    lls_init();
#endif

#ifdef RVM_RNG_SWE
    /* initialize RNG SWE */
    rng_init ();
#endif
#endif

#ifdef RVM_RVT_SWE
    /* initialize TRACE SWE */
    rv_start_swe_and_check (RVT_USE_ID, "RVT");
#endif

#ifdef RVM_I2C_SWE
	rv_start_swe_and_check (I2C_USE_ID, "I2C");
#endif

#ifdef RVM_DMA_SWE
	rv_start_swe_and_check (DMA_USE_ID, "DMA");
#endif

#ifdef RVM_DMG_SWE
	rv_start_swe_and_check (DMG_USE_ID, "DMG");
#endif

#ifdef RVM_NAN_SWE
	rv_start_swe_and_check (NAN_USE_ID, "NAN");
#endif

#ifdef RVM_MC_SWE
	rv_start_swe_and_check (MC_USE_ID, "MC");
#endif

#ifdef RVM_FFS_SWE
    /* initialize FFS SWE */
    rv_start_swe_and_check (FFS_USE_ID, "FFS");
#endif

#ifdef RVM_SPI_SWE
    /* initialize SPI SWE */
    rv_start_swe_and_check (SPI_USE_ID, "SPI");
#endif

#ifdef RVM_PWR_SWE
    /* initialize PWR SWE */
    rv_start_swe_and_check (PWR_USE_ID, "PWR");
#endif

#ifdef RVM_LCC_SWE
    /* initialize LCC(PWR) SWE */
    rv_start_swe_and_check (LCC_USE_ID, "LCC");
#endif

#ifdef RVM_KPD_SWE
    /* initialize KPD SWE */
    rv_start_swe_and_check (KPD_USE_ID, "KPD");
#endif

#ifdef RVM_DAR_SWE
    /* initialize DAR SWE */
    rv_start_swe_and_check (DAR_USE_ID, "DAR");
#endif

#ifdef RVM_R2D_SWE
    /* initialize R2D SWE */
    rv_start_swe_and_check (R2D_USE_ID, "R2D");
#endif

#ifdef RVM_LCD_SWE
    /* initialize LCD SWE */
    rv_start_swe_and_check (LCD_USE_ID, "LCD");
#endif


#ifdef RVM_ETM_SWE
    /* initialize ETM SWE */
    rv_start_swe_and_check (ETM_USE_ID, "ETM");
#endif

#ifdef RVM_TTY_SWE
    /* initialize TTY SWE */
    rv_start_swe_and_check (TTY_USE_ID, "TTY");
#endif


#ifdef RVM_AUDIO_MAIN_SWE
    /* initialize AUDIO SWE */
    rv_start_swe_and_check (AUDIO_USE_ID, "AUDIO");
#endif

#if 1 //(PSP_STANDALONE==0)
#ifdef RVM_AUDIO_BGD_SWE
    /* initialize AUDIO BACKGROUND SWE */
    rv_start_swe_and_check (AUDIO_BGD_USE_ID, "AUDIO_BGD");
#endif
#endif

#if 1 //(PSP_STANDALONE==0)
#ifdef RVM_BAE_SWE
    /* initialize BAE SWE */
    rv_start_swe_and_check (BAE_USE_ID, "BAE");
#endif
#endif

#ifdef RVM_AS_SWE
    /* initialize AS (Audio Services) SWE */
    rv_start_swe_and_check (AS_USE_ID, "AS");
#endif

#if 1 //(PSP_STANDALONE==0)
#ifdef RVM_BPR_SWE
    /* initialize sample BPR SWE */
    rv_start_swe_and_check (BPR_USE_ID, "BPR");
#endif
#endif /* PSP_STANDALONE */

#ifdef RVM_RTC_SWE
    /* initialize RTC SWE */
    rv_start_swe_and_check (RTC_USE_ID, "RTC");
#endif

#ifdef RVM_LLS_SWE
    /* initialize LLS SWE */
    rv_start_swe_and_check (LLS_USE_ID, "LLS");
#endif

#ifdef RVM_TUT_SWE
    /* initialize TUT SWE */
//	rv_start_swe_and_check (TUT_USE_ID, "TUT");
#endif

#ifdef RVM_RGUI_SWE
    /* initialize RGUI SWE */
	rv_start_swe_and_check (RGUI_USE_ID, "RGUI");
#endif

#ifdef RVM_ATP_SWE
	/* initialize ATP SWE */
	rv_start_swe_and_check (ATP_USE_ID, "ATP");
#endif

#ifdef RVM_MKS_SWE
	rv_start_swe_and_check (MKS_USE_ID, "MKS");
#endif

#ifdef RVM_IMG_SWE
	rv_start_swe_and_check (IMG_USE_ID, "IMG");
#endif

#ifdef RVM_GBI_SWE
	rv_start_swe_and_check (GBI_USE_ID, "GBI");
#endif

#ifdef RVM_CAMD_SWE
	rv_start_swe_and_check (CAMD_USE_ID, "CAMD");
#endif

#ifdef RVM_USB_SWE
    /* initialize USB SWE */
	rv_start_swe_and_check (USB_USE_ID, "USB");
#endif

#ifdef RVM_CAMA_SWE
	rv_start_swe_and_check (CAMA_USE_ID, "CAMA");
#endif

#ifdef RVM_MFW_SWE
    /* initialize MFW SWE */
    rv_start_swe_and_check (MFW_USE_ID, "MFW");
#endif

#ifdef RVM_SMBS_SWE
    /* initialize SMBS SWE */
    rv_start_swe_and_check (SMBS_USE_ID, "SMBS");
#endif

#ifdef RVM_USBFAX_SWE
    /* initialize USB SWE */
	rv_start_swe_and_check (USBFAX_USE_ID, "USBFAX");
#endif

#ifdef RVM_USBTRC_SWE
    /* initialize USBTRC SWE */
	rv_start_swe_and_check (USBTRC_USE_ID, "USBTRC");
#endif

#ifdef RVM_USBMS_SWE
    /* initialize USBMS SWE */
	rv_start_swe_and_check (USBMS_USE_ID, "USBMS");
#endif

#ifdef RVM_RFS_SWE
    /* initialize RFS SWE */
	rv_start_swe_and_check (RFS_USE_ID, "RFS");
#endif


#ifdef RVM_CCI_SWE
    /* initialize CCI SWE */
    rv_start_swe_and_check (CCI_USE_ID, "CCI");
#endif

#ifdef RVM_BTUI_SWE
    /* initialize sample BTUI SWE */
    rv_start_swe_and_check (BTUI_USE_ID, "BTUI");
#endif
#ifdef RVM_JPEG_SWE
    /* initialize sample JPEG SWE */
    rv_start_swe_and_check (JPEG_USE_ID, "JPEG");
#endif    
#ifdef RVM_JPEG_SWE
    /* initialize sample JPEG SWE */
    rv_start_swe_and_check (JPEG_USE_ID, "JPEG");
#endif    
// WARNING WARNING ----------------------------------------------------
// Do not perform any SWE initialization after this line !
// WARNING WARNING ----------------------------------------------------

#if 0 //(REMU==0)
/* moved this to kpd start function.  rv_start function for REMU. rv_start is called from Application Initialize 
context.  Since this is a blocking call, we cannot afford to block in Application_Initialization. */
#ifndef _WINDOWS
    // Perform switch ON processing.
    Switch_ON();
#endif

#if (_GSM==1)
    BZ_KeyBeep_ON ();          // Audio feedback if ON/OFF pushed
#endif // if (_GSM==1)

#endif

#if 1 //(CHIPSET!=15) || (REMU==0)
    /* dump the Riviera memory state */
    rvf_delay (RVF_MS_TO_TICKS (300)) ;
    rvf_dump_mem ();
    rvf_dump_pool();
    rvf_dump_tasks();
#endif

#if (TEST==1)
    // create a Memory Bank for the 'Test Selection Menu'.
    mb_requirements.size      = RVTEST_MENU_MB_SIZE;
    mb_requirements.watermark = RVTEST_MENU_MB_WATERMARK;
    mb_requirements.pool_id   = RVF_POOL_EXTERNAL_MEM;
    if (rvf_create_mb (RVTEST_MENU_MB_NAME,
                       mb_requirements,
                       &mb_id) != RVF_OK)
    {
        // error case.
        result++;
    }

// Go to the 'Test Selection Menu' (using rv_test ()).
    rv_test (0);
#endif // (TEST==1)

    // infinite wait
    rvf_evt_wait (START_TASK_ID,
                  0xFFFF,
                  0xFFFFFFFFL);
}

#if (TEST==1) 
  #define RV_START_TASK_STACK (4096)
#else
  #define RV_START_TASK_STACK (1024)
#endif
UINT8 stack_start[RV_START_TASK_STACK];


/*******************************************************************************
**
** Function         create_tasks
**
** Description      This function is called once at startup to allow task
**                  creation thanks to Riviera environment.
**
** Returns          void
**
*******************************************************************************/
void create_tasks (void)
{

   // Tasks creation
   rvf_create_legacy_task ((TASKPTR) rv_start, START_TASK_ID,
                    "RV_START", stack_start,
                    RV_START_TASK_STACK, RV_START_TASK_PRIO, 0, RUNNING);

}
