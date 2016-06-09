/* 
+----------------------------------------------------------------------------- 
|  Project :  GSM-PS
|  Modul   :  DRV_RX
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
|  Purpose :  This Module defines the fieldstrength management
|             device driver for the G23 protocol stack.
|             
|             This driver is used to control all fieldstrength related
|             functions. The driver does support multiple devices and
|             therefore no open and close functionality is supported.
|             The driver can be configured to signal different state
|             transitions. This is done by setting an OS signal or
|             calling a specified call-back function.
+----------------------------------------------------------------------------- 
*/ 

#ifndef DRV_RX_C
#define DRV_RX_C
#endif
#define ENTITY_CST
/*==== INCLUDES ===================================================*/

#include "config.h"
#include "fixedconf.h"
#include "condat-features.h"

#if defined (NEW_FRAME)

#include <string.h>
#include "typedefs.h"
#include "vsi.h"
#include "custom.h"
#include "gsm.h"
#include "prim.h"
#include "gdi.h"
#include "rx.h"

#else

#include <string.h>
#include "stddefs.h"
#include "custom.h"
#include "gsm.h"
#include "prim.h"
#include "vsi.h"
#include "gdi.h"
#include "rx.h"

#endif
/*==== EXPORT =====================================================*/
EXTERN USHORT RX_GetValue  ( void );
EXTERN UBYTE  RX_GetRxQual ( void ); 
EXTERN  void RX_Enable (T_VOID_FUNC rx_indication);

#if defined (NEW_FRAME)
EXTERN USHORT cst_handle;
#endif  /* NEW_FRAME */

/*==== CONSTANTS ==================================================*/
#define RX_INDICATION_WITHOUT_TIMER
#if defined (NEW_FRAME)
#define CST_RXTIMER 1
#endif  /* NEW_FRAME */

/*==== VARIABLES ==================================================*/
drv_SignalCB_Type  rx_signal_callback = NULL;
rx_DCB_Type        rx_DCB;
rx_Status_Type     rx_Status;
#if !defined (NEW_FRAME)
T_VSI_THANDLE      rx_handle = VSI_ERROR;
#endif  /* !NEW_FRAME */

/*==== FUNCTIONS ==================================================*/
#if 0
  #if defined(NEW_FRAME)
    #define SYST_TRACE(a) vsi_o_ttrace(0, 0xFFFF,a)
    #define SYST           0, 0xffff
    #define SYST_TRACE_P(a) vsi_o_ttrace a
  #else /* NEW_FRAME */
    #define SYST_TRACE(a) vsi_o_trace("", 0xFFFF,a)
    #define SYST           "", 0xffff
    #define SYST_TRACE_P(a) vsi_o_trace a
  #endif /* NEW_FRAME */
  /*
   * use it as showed next line...
   * SYST_TRACE_P((SYST, "e.g. two parameter: %d %d", p1, p2));
   */
#else /* 0|1 */
  #define SYST_TRACE(a) 
  #define SYST_TRACE_P(a)
#endif  /* 0|1 */


LOCAL void RX_Indication (void)
{
  if (rx_signal_callback)
  {
    USHORT         new_rx_value;
    UBYTE          calculated_level;
    drv_SignalID_Type   signal_params;

    new_rx_value       = RX_GetValue ();
    rx_Status.gsmLevel = ( UBYTE ) new_rx_value;

    calculated_level   = (new_rx_value * rx_DCB.Steps) / 64;

    SYST_TRACE_P ((SYST, "RX_Indication(): rx=%u new, %u old", 
      calculated_level, rx_Status.actLevel));
    
    if (calculated_level NEQ rx_Status.actLevel)
    {
      signal_params.SignalType  = RX_SIGTYPE_RXLEVEL;
      #if defined (NEW_FRAME)
        signal_params.UserData    = (void*)&rx_Status;
      #else
        signal_params.SignalValue = 0;
        signal_params.UserData    = (ULONG)&rx_Status;
      #endif  /* NEW_FRAME */
      rx_Status.actLevel        = calculated_level;
      rx_Status.rxQuality       = RX_GetRxQual ();

      #if !defined (WIN32)
        /*
         * suppress for windows to avoid disturb of regression tests
         */
        (*rx_signal_callback)(&signal_params);
      #endif  /* !WIN32 */
    }
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6103)       MODULE  : DRV_RX                     |
| STATE   : code                ROUTINE : rx_Init                    |
+--------------------------------------------------------------------+

  PURPOSE : The function initializes the driver´s internal data.
            The function returns DRV_OK in case of a successful
            completition. The function returns DRV_INITIALIZED if
            the driver has already been initialized and is ready to
            be used or is already in use. In case of an initialization
            failure, which means the that the driver cannot be used,
            the function returns DRV_INITFAILURE.

*/

GLOBAL UBYTE rx_Init (drv_SignalCB_Type in_SignalCBPtr)
{
  rx_signal_callback   = in_SignalCBPtr;     /* store call-back function */
  rx_DCB.Steps         = 4;                  /* 4 Steps                  */
  rx_Status.actLevel   = 255;
  rx_Status.gsmLevel   = 255;

#if defined (RX_INDICATION_WITHOUT_TIMER)
  if (in_SignalCBPtr)
  {
    SYST_TRACE_P ((SYST, "rx_Init(): enable RX_Indication (%p)", in_SignalCBPtr));
    RX_Enable (RX_Indication);
  }
  else
  {
    SYST_TRACE ("rx_Init() without in_SignalCBPtr");
  }
#else /* RX_INDICATION_WITHOUT_TIMER */
  #if !defined (NEW_FRAME)
    rx_handle = vsi_t_open (VSI_CALLER "RX");

    if (rx_handle < VSI_OK)
      return DRV_INITFAILURE;

    vsi_t_start (VSI_CALLER rx_handle, T_RX_VALUE);
  #else /* !NEW_FRAME */
    vsi_t_start (VSI_CALLER CST_RXTIMER, T_RX_VALUE);
  #endif  /* !NEW_FRAME */
#endif  /* RX_INDICATION_WITHOUT_TIMER */
  return DRV_OK;
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6103)       MODULE  : DRV_RX                     |
| STATE   : code                ROUTINE : rx_Exit                    |
+--------------------------------------------------------------------+

  PURPOSE : The function is used to indicate RX that the driver
            and its functionality isn´t needed anymore.

*/

GLOBAL void rx_Exit (void)
{
  rx_signal_callback = NULL;

#if !defined (RX_INDICATION_WITHOUT_TIMER)
  #if !defined (NEW_FRAME)
    vsi_t_close (VSI_CALLER rx_handle);
    rx_handle = VSI_ERROR;
  #else /* !NEW_FRAME */
    vsi_t_stop (VSI_CALLER CST_RXTIME);
  #endif  /* !NEW_FRAME */
#endif  /* RX_INDICATION_WITHOUT_TIMER */
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6103)       MODULE  : DRV_RX                     |
| STATE   : code                ROUTINE : rx_SetConfig               |
+--------------------------------------------------------------------+

  PURPOSE : This function is used to configure the driver.
            If any value of this configuration is out of range or
            invalid in combination with any other value of the
            configuration, the function returns DRV_INVALID_PARAMS.
            Call the rx_GetConfig() function to retrieve the drivers
            configuration.

*/

GLOBAL UBYTE rx_SetConfig (rx_DCB_Type * in_DCBPtr)
{
  memcpy (&rx_DCB, in_DCBPtr, sizeof (rx_DCB_Type));
  rx_Status.actLevel  = 255;
  rx_Status.gsmLevel  = 255;

  return DRV_OK;
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6103)       MODULE  : DRV_RX                     |
| STATE   : code                ROUTINE : rx_GetConfig               |
+--------------------------------------------------------------------+

  PURPOSE : The function is used to retrieve the configuration of
            the driver. The configuration is returned in the driver
            control block to which the pointer out_DCBPtr points.
            If the driver is not configured, the function returns
            DRV_NOTCONFIGURED.
            Call the rx_SetConfig() function to configure the driver.

*/

GLOBAL UBYTE rx_GetConfig (rx_DCB_Type * out_DCBPtr)
{
  return DRV_OK;
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6103)       MODULE  : DRV_RX                     |
| STATE   : code                ROUTINE : rx_GetStatus               |
+--------------------------------------------------------------------+

  PURPOSE : This function is used to retrieve the status of the driver.
            In case of a successful completion the driver returns
            DRV_OK and the current status of the driver in the buffer
            out_StatusPtr points to.
            In case the driver is not configured yet, it returns
            DRV_NOTCONFIGURED. In this case the contents of the
            buffer out_StatusPtr is invalid.
            In case out_StatusPtr equals NULL the driver returns
            DRV_INVALID_PARAMS.

*/

GLOBAL UBYTE rx_GetStatus (rx_Status_Type * out_StatusPtr)
{
  USHORT new_rx_value = RX_GetValue ();

  rx_Status.gsmLevel  = ( UBYTE ) new_rx_value;
  rx_Status.actLevel  = (new_rx_value * rx_DCB.Steps) / 64;
  rx_Status.rxQuality = RX_GetRxQual ();

  out_StatusPtr->actLevel  = rx_Status.actLevel;
  out_StatusPtr->gsmLevel  = rx_Status.gsmLevel;
  out_StatusPtr->rxQuality = rx_Status.rxQuality;

  return DRV_OK;
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6103)       MODULE  : DRV_RX                     |
| STATE   : code                ROUTINE : rx_timeout                 |
+--------------------------------------------------------------------+

  PURPOSE : This function calculates a new rxlevel after timeout.
            If a change has occured, the new level is forwarded to
            MMI using the callback function. The timer is started
            again.

*/
#if defined (NEW_FRAME)
GLOBAL void rx_timeout (USHORT index)
#else
GLOBAL void rx_timeout (T_VSI_THANDLE handle)
#endif
{
#if !defined (RX_INDICATION_WITHOUT_TIMER)
  #if defined (NEW_FRAME)
    if (index EQ CST_RXTIMER)
  #else
    if (handle EQ rx_handle)
  #endif
    {
      /*
       * only if it is the fieldstrength timer
       */
      SYST_TRACE ("rx_timeout()");
      
      RX_Indication ();
      
      #if defined (NEW_FRAME)
        vsi_t_start (VSI_CALLER CST_RXTIMER, T_RX_VALUE);
      #else
        vsi_t_start (VSI_CALLER rx_handle, T_RX_VALUE);
      #endif
    }
#endif  /* !RX_INDICATION_WITHOUT_TIMER */
}
