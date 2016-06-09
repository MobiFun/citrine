/*
+-----------------------------------------------------------------------------
|  Project :  COMLIB
|  Modul   :  cl_shrd.c
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
|  Purpose :  Definitions of common library functions: Implementation of
              creation of Semaphores and usage of it by any entity in 
              PS
+-----------------------------------------------------------------------------
*/
/*
 *  Version 1.0
 */

/**********************************************************************************/

/*
NOTE:
*/

/**********************************************************************************/
#ifndef CL_SHRD_C
#define CL_SHRD_C
/*==== INCLUDES ===================================================*/

#include "config.h"
#include "fixedconf.h"

#include <string.h>
#include <stdio.h>
#include "typedefs.h"
#include "vsi.h"
#include "cl_shrd.h"

/*==== VARIABLES ==================================================*/

static T_HANDLE cl_handle;

#ifdef OPTION_MULTITHREAD
#define VSI_CALLER cl_handle,
#else
#define VSI_CALLER
#endif

/* Pointer is used for faster memory access */
static T_SHRD_DATA shrd_data_base;
T_SHRD_DATA *shared_data = &shrd_data_base;

static T_HANDLE  sem_SHARED = VSI_ERROR;
static BOOL is_initialized = FALSE;

/*==== FUNCTIONS ==================================================*/

/*
+---------------------------------------------------------------------------------
|  Function     : cl_shrd_init 
+---------------------------------------------------------------------------------
|  Description  : Opens counting semaphore specified by its name. 
|                 If semaphore doesnot exists, creates semaphore with count given.
|
|  Parameters   :  T_HANDLE
|
|  Return       :  void
|
+---------------------------------------------------------------------------------
*/
GLOBAL void cl_shrd_init (T_HANDLE handle)
{
  TRACE_FUNCTION ("cl_shrd_init()");

  if(is_initialized NEQ TRUE)
  {
    cl_handle = handle;

    memset(shared_data, 0, sizeof(T_SHRD_DATA));
    sem_SHARED  = vsi_s_open (VSI_CALLER "SHARED_SEM",1);

    if (sem_SHARED NEQ VSI_ERROR)
    {
      TRACE_EVENT ("Semaphore opened successfully \"SHARED_SEM\"");
      is_initialized = TRUE;
#ifdef TI_PS_FF_AT_P_CMD_CTREG
      /* 
       * Initialize the Two tables with the default values
       */
      memcpy(shared_data->no_serv_mod_time,&no_service_mode_time,
          sizeof(no_service_mode_time));

      memcpy(shared_data->lim_serv_mod_time,&lim_service_mode_time,
          sizeof(lim_service_mode_time));
#endif /* TI_PS_FF_AT_P_CMD_CTREG */
    }
    else
      TRACE_EVENT ("Cant open semaphore \"SHARED_SEM\"");
  }
}

/*
+------------------------------------------------------------------------------
|  Function     : cl_shrd_exit
+------------------------------------------------------------------------------
|  Description  :  Close the semaphore.
|
|  Parameters   :  void
|
|  Return       :  void
|
+------------------------------------------------------------------------------
*/

GLOBAL void cl_shrd_exit (void)
{
  TRACE_FUNCTION ("cl_shrd_exit()");
  if(is_initialized EQ TRUE)
  {
    if (sem_SHARED NEQ VSI_ERROR)
      vsi_s_close (VSI_CALLER sem_SHARED);
  
    memset(shared_data, 0, sizeof(T_SHRD_DATA));
    is_initialized = FALSE;
  }
}

/*
+------------------------------------------------------------------------------
|  Function     : cl_shrd_get_loc
+------------------------------------------------------------------------------
|  Description  : Copies the content from global T_LOC_INFO to the 
|                 passed parameter 
|
|  Parameters   : <loc_info>:  Location information
|
|  Return       : void
|
+------------------------------------------------------------------------------
*/

GLOBAL BOOL cl_shrd_get_loc (T_LOC_INFO *loc_info)
{
  BOOL ret = FALSE;
  TRACE_FUNCTION ("cl_shrd_get_loc()");

  if (sem_SHARED NEQ VSI_ERROR)
  {
    if (vsi_s_get (VSI_CALLER sem_SHARED) EQ VSI_OK)
    {
      if ( loc_info NEQ NULL )
        memcpy(loc_info, &shared_data->location_info, sizeof(T_LOC_INFO));
      vsi_s_release (VSI_CALLER sem_SHARED);
      ret = TRUE;
    }
    else
    {
      TRACE_EVENT ("Semaphore not free or Invalid handle \"sem_SHARED\"");
      return(ret);
    }
  }
  return(ret);
}

/*
+------------------------------------------------------------------------------
|  Function     : cl_shrd_set_loc
+------------------------------------------------------------------------------
|  Description  : Copies the content from passed parameter to the 
|                 global structure 
|
|  Parameters   : <loc_info>:  Location information
|
|  Return       : void
|
+------------------------------------------------------------------------------
*/

GLOBAL void cl_shrd_set_loc (T_LOC_INFO *loc_info)
{
  TRACE_FUNCTION ("cl_shrd_set_loc()");

  if (sem_SHARED NEQ VSI_ERROR)
  {
    if (vsi_s_get (VSI_CALLER sem_SHARED) EQ VSI_OK)
    {
      if ( loc_info NEQ NULL )
        memcpy(&shared_data->location_info, loc_info, sizeof(T_LOC_INFO));
      vsi_s_release (VSI_CALLER sem_SHARED);
    }
    else
    {
      TRACE_EVENT ("Semaphore not free or Invalid handle \"sem_SHARED\"");
    }
  }
}

/*
+------------------------------------------------------------------------------
|  Function     : cl_shrd_get_tim_adv
+------------------------------------------------------------------------------
|  Description  : Copies the content from global T_TIM_ADV to the 
|                 passed parameter 
|
|  Parameters   : <tim_adv>:  Timing Advance and ME status.
|
|  Return       : void
|
+------------------------------------------------------------------------------
*/

GLOBAL BOOL cl_shrd_get_tim_adv (T_TIM_ADV *tim_adv)
{
  BOOL ret = FALSE;
  TRACE_FUNCTION ("cl_shrd_get_tim_adv()");

  if (sem_SHARED NEQ VSI_ERROR)
  {
    if (vsi_s_get (VSI_CALLER sem_SHARED) EQ VSI_OK)
    {
      if ( tim_adv NEQ NULL )
        memcpy(tim_adv, &shared_data->timing_advance, sizeof(T_TIM_ADV));
      vsi_s_release (VSI_CALLER sem_SHARED);
      ret = TRUE;
    }
    else
    {
      TRACE_EVENT ("Semaphore not free or Invalid handle \"sem_SHARED\"");
      return(ret);
    }
  }
  return(ret);
}

/*
+------------------------------------------------------------------------------
|  Function     : cl_shrd_set_tim_adv
+------------------------------------------------------------------------------
|  Description  : Copies the content from passed parameter to the 
|                 global structure 
|
|  Parameters   : <tim_adv>:  Timing Advance and ME status.
|
|  Return       : void
|
+------------------------------------------------------------------------------
*/

GLOBAL void cl_shrd_set_tim_adv (T_TIM_ADV *tim_adv)
{
  TRACE_FUNCTION ("cl_shrd_set_tim_adv()");

  if (sem_SHARED NEQ VSI_ERROR)
  {
    if (vsi_s_get (VSI_CALLER sem_SHARED) EQ VSI_OK)
    {
      if ( tim_adv NEQ NULL )
        memcpy(&shared_data->timing_advance, tim_adv, sizeof(T_TIM_ADV));
      vsi_s_release (VSI_CALLER sem_SHARED);
    }
    else
    {
      TRACE_EVENT ("Semaphore not free or Invalid handle \"sem_SHARED\"");
    }
  }
}
#ifdef TI_PS_FF_AT_P_CMD_CTREG
/*
+------------------------------------------------------------------------------
|  Function     : cl_shrd_set_treg_val
+------------------------------------------------------------------------------
|  Description  : Copies the content from passed parameter to the 
|                 global structure  Used for %CTREG setting the values.
|
|  Parameters   : <mode>   : Selects the mode of operation read or write
|                 <tab_id> : Selects either no_service_mode_time or 
|                             lim_service_mode_time for updating.
|                 <tab_val>: Table values to be updated in the selcted table.
|
|  Return       : BOOL
|
+------------------------------------------------------------------------------
*/

GLOBAL BOOL cl_shrd_set_treg_val ( T_TREG *treg )
{
  UBYTE i;
  BOOL ret = FALSE;

  TRACE_FUNCTION ("cl_shrd_set_treg_val()");

  if (sem_SHARED NEQ VSI_ERROR)
  {
    if (vsi_s_get (VSI_CALLER sem_SHARED) EQ VSI_OK)
    {
      if ( treg NEQ NULL )
      {
        switch(treg->tab_id)
        {
          case NOSERVICE_MODE_TIME:
            memcpy(shared_data->no_serv_mod_time, treg->tab_val,
                    MAX_CTREG_TAB_LEN);
            break;
          case LIMSERVICE_MODE_TIME:
            memcpy(shared_data->lim_serv_mod_time, treg->tab_val,
                    MAX_CTREG_TAB_LEN);
            break;
          default:
            break;
        }
        ret = TRUE;
      }
      vsi_s_release (VSI_CALLER sem_SHARED);
    }
    else
    {
      TRACE_EVENT ("Semaphore not free or Invalid handle \"sem_SHARED\"");
    }
  }
  return(ret);
}

/*
+------------------------------------------------------------------------------
|  Function     : cl_shrd_get_treg_val
+------------------------------------------------------------------------------
|  Description  : Reads the content from passed parameter to the 
|                 global structure.
|
|  Parameters   : <mode>   : Selects the mode of operation read or write
|                 <tab_id> : Selects either no_service_mode_time or 
|                             lim_service_mode_time for updating.
|                 <tab_val>: Table values to be read from the selected table.
|
|  Return       : BOOL
|
+------------------------------------------------------------------------------
*/

GLOBAL BOOL cl_shrd_get_treg_val ( T_TREG *treg )
{
  UBYTE i;
  BOOL ret = FALSE;

  TRACE_FUNCTION ("cl_shrd_get_treg_val()");

  if (sem_SHARED NEQ VSI_ERROR)
  {
    if (vsi_s_get (VSI_CALLER sem_SHARED) EQ VSI_OK)
    {
      if ( treg NEQ NULL )
      {
        switch(treg->tab_id)
        {
          case NOSERVICE_MODE_TIME:
            memcpy(treg->tab_val, shared_data->no_serv_mod_time,
                    MAX_CTREG_TAB_LEN);
            break;
          case LIMSERVICE_MODE_TIME:
            memcpy(treg->tab_val, shared_data->lim_serv_mod_time,
                    MAX_CTREG_TAB_LEN);
            break;
          default:
            break;
        }
        ret = TRUE;
      }
      vsi_s_release (VSI_CALLER sem_SHARED);
    }
    else
    {
      TRACE_EVENT ("Semaphore not free or Invalid handle \"sem_SHARED\"");
    }
  }
  return(ret);
}

/*
+------------------------------------------------------------------------------
|  Function     : cl_shrd_get_treg
+------------------------------------------------------------------------------
|  Description  : Reads the TREG Timer value from the selected Table and 
|                 returns the data to called Entity (RR)
|
|  Parameters   : <tab_id> : Selects either no_service_mode_time or
|                             lim_service_mode_time for getting TREG value.
|                 <offset> : Offset value to point at exact position in the 
|                             selected Table for getting TREG value.
|                 <tab_val>: Table value in the selcted table.
|
|  Return       : BOOL
|
+------------------------------------------------------------------------------
*/

GLOBAL BOOL cl_shrd_get_treg (UBYTE tab_id, UBYTE offset, UBYTE *tab_val)
{
  BOOL ret = FALSE;

  TRACE_FUNCTION ("cl_shrd_get_treg()");

  if (sem_SHARED NEQ VSI_ERROR)
  {
    if (vsi_s_get (VSI_CALLER sem_SHARED) EQ VSI_OK)
    {
      /* Check for the proper value of offset, it should be between 0 to 24 */
      if(offset > (MAX_CTREG_TAB_LEN - 1))
      {
        return(ret);
      }
      switch(tab_id)
      {
        case NOSERVICE_MODE_TIME:
          *tab_val = shared_data->no_serv_mod_time[offset];
          break;
        case LIMSERVICE_MODE_TIME:
          *tab_val = shared_data->lim_serv_mod_time[offset];
          break;
        default:
          break;
      }
      ret = TRUE;
      vsi_s_release (VSI_CALLER sem_SHARED);
    }
    else
    {
      TRACE_EVENT ("Semaphore not free or Invalid handle \"sem_SHARED\"");
    }
  }
  return(ret);
}
#endif /* TI_PS_FF_AT_P_CMD_CTREG */

#endif   /* CL_SHRD_C */
