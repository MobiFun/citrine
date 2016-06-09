/* 
+----------------------------------------------------------------------------- 
|  Project :  GSM-PS
|  Modul   :  DRV_PWR
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
|  Purpose :  This Module defines the power management device driver
|             for the G23 protocol stack.
|             
|             This driver is used to control all power related functions 
|             such as charger and battery control. The driver does support
|             multiple devices and therefore no open and close functionality
|             is supported. The driver can be configured to signal different
|             state transitions, for example battery level has reached the
|             "battery low" level. This is done by setting an OS signal or 
|             calling a specified call-back function.
+----------------------------------------------------------------------------- 
*/ 

#ifndef DRV_PWR_C
#define DRV_PWR_C
#endif

#include "config.h"
#include "fixedconf.h"
#include "condat-features.h"

/*==== INCLUDES ===================================================*/
#if defined (NEW_FRAME)

#include <string.h>
#include "typedefs.h"
#include "gdi.h"
#include "pwr.h"

#else

#include <string.h>
#include "stddefs.h"
#include "gdi.h"
#include "pwr.h"

#endif
/*==== EXPORT =====================================================*/
#if defined (_TMS470_NOT_YET)
EXTERN void BAT_Init         (void (*pwr_batlevel)(UBYTE level),
                              void (*pwr_batstatus)(UBYTE status));
#else
LOCAL  void BAT_Init         (void (*pwr_batlevel)(UBYTE level),
                              void (*pwr_batstatus)(UBYTE status));
#endif

LOCAL void pwr_batlevel      (UBYTE level);
LOCAL void pwr_batstatus     (UBYTE status);

/*==== VARIABLES ==================================================*/
drv_SignalCB_Type  pwr_signal_callback = NULL;
UBYTE              pwr_act_level;
UBYTE              pwr_act_status;
pwr_DCB_Type       pwr_DCB;
pwr_Status_Type    pwr_Status;
/*==== VARIABLES ==================================================*/

/*==== FUNCTIONS ==================================================*/

/*==== CONSTANTS ==================================================*/
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6103)       MODULE  : DRV_PWR                    |
| STATE   : code                ROUTINE : pwr_Init                   |
+--------------------------------------------------------------------+

  PURPOSE : The function initializes the driver´s internal data. 
            The function returns DRV_OK in case of a successful
            completition. The function returns DRV_INITIALIZED if
            the driver has already been initialized and is ready to
            be used or is already in use. In case of an initialization
            failure, which means the that the driver cannot be used,
            the function returns DRV_INITFAILURE.
            
*/

GLOBAL UBYTE pwr_Init (drv_SignalCB_Type in_SignalCBPtr)
{
  pwr_signal_callback = in_SignalCBPtr;    /* store call-back function */
  pwr_DCB.RangeMin    = 10;                /* 10 Percent               */
  pwr_DCB.RangeMax    = 100;               /* 100 Percent              */
  pwr_DCB.Steps       = 4;                 /* 4 Steps                  */

  pwr_Status.Status       = 0;
  pwr_Status.BatteryLevel = 0;   
  pwr_Status.ChargeLevel  = 0;

  /*
   * Initialise TI driver with internal callback functions
   *
   * pwr_batlevel is called after change of battery level
   * pwr_batstatus is called after change of external or charger
   * unit
   *
   */
  BAT_Init (pwr_batlevel, pwr_batstatus);
 
  return DRV_OK;
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6103)       MODULE  : DRV_PWR                    |
| STATE   : code                ROUTINE : pwr_Exit                   |
+--------------------------------------------------------------------+

  PURPOSE : The function is used to indicate PWR that the driver 
            and its functionality isn´t needed anymore.

*/

GLOBAL void pwr_Exit (void)
{
  pwr_signal_callback = NULL;
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6103)       MODULE  : DRV_PWR                    |
| STATE   : code                ROUTINE : pwr_SetSignal              |
+--------------------------------------------------------------------+

  PURPOSE : This function is used to define a single signal or multiple
            signals that is/are indicated to the process when the event
            identified in the signal information data type as SignalType
            occurs.
            To remove a signal, call the function pwr_ResetSignal().
            If one of the parameters of the signal information data is
            invalid, the function returns DRV_INVALID_PARAMS.
            If no signal call-back function has been defined at the
            time of initilization the driver returns DRV_SIGFCT_NOTAVAILABLE.

*/

GLOBAL UBYTE pwr_SetSignal (drv_SignalID_Type * in_SignalIDPtr)
{
  return DRV_OK;
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6103)       MODULE  : DRV_PWR                    |
| STATE   : code                ROUTINE : pwr_ResetSignal            |
+--------------------------------------------------------------------+

  PURPOSE : The function is used to remove a single or multiple signals
            that has previously been set. The signals that are removed
            are identified by the Signal Information Data element called
            SignalType. All other elements of the Signal Information Data 
            must be identical to the signal(s) that is/are to be 
            removed. If the SignalID provided can not be found, the 
            function returns DRV_INVALID_PARAMS.
            If no signal call-back function has beed defined at the 
            time of initialization, the driver returns DRV_SIGFCT_NOTAVAILABLE.

*/

GLOBAL UBYTE pwr_ResetSignal (drv_SignalID_Type * in_SignalIDPtr)
{
  return DRV_OK;
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6103)       MODULE  : DRV_PWR                    |
| STATE   : code                ROUTINE : pwr_SetConfig              |
+--------------------------------------------------------------------+

  PURPOSE : This function is used to configure the driver. 
            If any value of this configuration is out of range or
            invalid in combination with any other value of the
            configuration, the function returns DRV_INVALID_PARAMS.
            Call the pwr_GetConfig() function to retrieve the drivers 
            configuration.

*/

GLOBAL UBYTE pwr_SetConfig (pwr_DCB_Type * in_DCBPtr)
{
  memcpy (&pwr_DCB, in_DCBPtr, sizeof (pwr_DCB_Type));
  pwr_Status.BatteryLevel = 0;   
  pwr_Status.ChargeLevel  = 0;

  return DRV_OK;
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6103)       MODULE  : DRV_PWR                    |
| STATE   : code                ROUTINE : pwr_GetConfig              |
+--------------------------------------------------------------------+

  PURPOSE : The function is used to retrieve the configuration of 
            the driver. The configuration is returned in the driver
            control block to which the pointer out_DCBPtr points.
            If the driver is not configured, the function returns
            DRV_NOTCONFIGURED.
            Call the pwr_SetConfig() function to configure the driver.
            
*/

GLOBAL UBYTE pwr_GetConfig (pwr_DCB_Type * out_DCBPtr)
{
  return DRV_OK;
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6103)       MODULE  : DRV_PWR                    |
| STATE   : code                ROUTINE : pwr_GetStatus              |
+--------------------------------------------------------------------+

  PURPOSE : This function is used to retrieve the status of the driver
            respectively the power unit.
            In case of a successful completion the driver returns 
            DRV_OK and the current status of the driver in the buffer
            out_StatusPtr points to.
            In case the driver is not configured yet, it returns
            DRV_NOTCONFIGURED. In this case the contents of the
            buffer out_StatusPtr is invalid.
            In case out_StatusPtr equals NULL the driver returns 
            DRV_INVALID_PARAMS.
            
*/

GLOBAL UBYTE pwr_GetStatus (pwr_Status_Type * out_StatusPtr)
{
  if ( out_StatusPtr EQ NULL )
  {
    return DRV_INVALID_PARAMS;
  }
  else
  {
    out_StatusPtr->Status       = pwr_Status.Status;
    out_StatusPtr->BatteryLevel = pwr_Status.BatteryLevel;
    out_StatusPtr->ChargeLevel  = pwr_Status.ChargeLevel;
  }

  return DRV_OK;
}



/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6103)       MODULE  : DRV_PWR                    |
| STATE   : code                ROUTINE : pwr_BatLevel               |
+--------------------------------------------------------------------+

  PURPOSE : This function is called by the low level driver after
            change of battery level.
*/

LOCAL void pwr_batlevel (UBYTE level)
{
  UBYTE               calculated_level;
  drv_SignalID_Type   signal_params;

  pwr_Status.ChargeLevel = level;

  if (level <= pwr_DCB.RangeMin)
    calculated_level = 0;
  if (level >= pwr_DCB.RangeMax)
    calculated_level = pwr_DCB.Steps +1;
  if (level > pwr_DCB.RangeMin AND
      level < pwr_DCB.RangeMax)
  {
    level -= pwr_DCB.RangeMin;
    calculated_level = ((level * pwr_DCB.Steps) / 
                        (pwr_DCB.RangeMax - pwr_DCB.RangeMin))+1;
  }

  if (calculated_level EQ pwr_Status.BatteryLevel)
    return;

  signal_params.SignalType  = PWR_SIGTYPE_BATLEVEL;
#if defined (NEW_FRAME)
  signal_params.UserData    = (void*)&pwr_Status;
#else
  signal_params.SignalValue = 0;
  signal_params.UserData    = (ULONG)&pwr_Status;
#endif
  pwr_Status.BatteryLevel   = calculated_level;

  if (pwr_signal_callback NEQ NULL)
    (*pwr_signal_callback)(&signal_params);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6103)       MODULE  : DRV_PWR                    |
| STATE   : code                ROUTINE : pwr_batstatus              |
+--------------------------------------------------------------------+

  PURPOSE : This function is called by the low level driver after
            detecting a status change.
*/

LOCAL void pwr_batstatus (UBYTE status)
{
}

/*******************************************************************
 *                                                                 *
 * PART II: Simulation for Windows                                 *
 *                                                                 *
 *******************************************************************/

/*#if defined (WIN32)*/
/*
 * Dummies for driver calls
 */
LOCAL void BAT_Init   (void (*pwr_batlevel)(UBYTE),
                       void (*pwr_batstatus)(UBYTE))
{
}

/*#endif*/

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6103)       MODULE  : DRV_PWR                    |
| STATE   : code                ROUTINE : pwr_PowerOffMobile             |
+--------------------------------------------------------------------+

  PURPOSE : This function is switching off the mobile
*/

GLOBAL UBYTE pwr_PowerOffMobile (void)
{

/* power off HW is not applicable in simulation */


#if !defined (WIN32)

	/* power-off the board / HW */
	ABB_Power_Off();

//#endif /* _TARGET_ */
#endif /* _TMS470 */

    return 1;
}
