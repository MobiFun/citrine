/*
+-------------------------------------------------------------------+
| PROJECT: GSM-F&D (8411)               $Workfile:: cst_csf.c       $|
| $Author:: Be $ CONDAT GmbH            $Revision:: 7               $|
| CREATED: 01.02.99                     $Modtime:: 1.12.99 13:16    $|
| STATE  : code                                                      |
+--------------------------------------------------------------------+


   MODULE  : CST_CSF

   PURPOSE : This Modul defines the custom specific functionalitys
             for the entity CST.
*/

#ifndef CST_CSF_C
#define CST_CSF_C
#endif

#include "config.h"
#include "fixedconf.h"
#include "condat-features.h"

#define ENTITY_CST

/*==== INCLUDES ===================================================*/

#include <string.h>
#include "typedefs.h"
#include "vsi.h"
#include "gsm.h"
#include "p_cst.h"
#include "cst.h"

/*==== CONSTANTS ==================================================*/

/*==== TYPES ======================================================*/

/*==== EXPORT =====================================================*/

/*==== VARIABLES ==================================================*/

LOCAL USHORT ADC_CONVERSIONS[9];

/*==== FUNCTIONS ==================================================*/

#ifdef ALR
extern void madc_hex_2_physical (USHORT *adc_hex, USHORT *adc_phy);
#endif

#if (BOARD==34)
extern inline unsigned char GC_GetAdcInfo(void);
#endif

/*
+--------------------------------------------------------------------+
| PROJECT : XXX                         MODULE  : CST_CSF            |
| STATE   : code                        ROUTINE : csf_adc_start      |
+--------------------------------------------------------------------+

  PURPOSE : This function sends a primitive to L1A to activate the ADC */


GLOBAL void csf_adc_start (UBYTE tx_flag, UBYTE traffic_period, UBYTE idle_period)
{

  // This structure and the #define have to be implemented in the MMI.
  // This is a temporary location for test !!!!!!!
  // TO BE REMOVED BY USER
  typedef struct
  {
    UBYTE  tx_flag;
    UBYTE  traffic_period;
    UBYTE  idle_period;
  }
  T_MMI_ADC_REQ;

  #define MMI_ADC_REQ  (111)


  PALLOC(adc_req, MMI_ADC_REQ);

  adc_req->tx_flag  = tx_flag;
  adc_req->traffic_period  = traffic_period;
  adc_req->idle_period  = idle_period;

  PSENDX (L1, adc_req);

}


/*
+--------------------------------------------------------------------+
| PROJECT : XXX                         MODULE  : CST_CSF            |
| STATE   : code                        ROUTINE : csf_adc_process    |
+--------------------------------------------------------------------+

  PURPOSE : This function processes ADC results. It stores 10 consecutive
  results in an array.
*/


GLOBAL void csf_adc_process (T_CST_ADC_IND *adc_results)
{
#ifdef ALR
 volatile USHORT adc_converted[9];
#endif

#ifdef ALR
/* convert adc value into physical values */
   madc_hex_2_physical (adc_results->adc_values, (USHORT*) adc_converted);

 ADC_CONVERSIONS[0] = adc_converted[0];        /* Battery Voltage */
 ADC_CONVERSIONS[1] = adc_converted[1];        /* Charger Voltage */
 ADC_CONVERSIONS[2] = adc_converted[2];        /* I Charger */
 ADC_CONVERSIONS[3] = adc_converted[3];        /* I Charger */
 ADC_CONVERSIONS[4] = adc_converted[4];        /* Battery Type */
 ADC_CONVERSIONS[5] = adc_converted[5];        /* Battery Temperature */
 ADC_CONVERSIONS[6] = adc_converted[6];        /* RF temperature */
 ADC_CONVERSIONS[7] = adc_converted[7];        /* Touch Screen X */
 ADC_CONVERSIONS[8] = adc_converted[8];        /* Touch Screen Y */
#else
 ADC_CONVERSIONS[0] = adc_results->adc_values[0];
 ADC_CONVERSIONS[1] = adc_results->adc_values[1];
 ADC_CONVERSIONS[2] = adc_results->adc_values[2];
 ADC_CONVERSIONS[3] = adc_results->adc_values[3];
 ADC_CONVERSIONS[4] = adc_results->adc_values[4];
#endif

// WCS Patch :Avenger 2 uses adc index 5 to store battery informations bit field
#if (BOARD == 34)
 ADC_CONVERSIONS[5] = GC_GetAdcInfo();
#endif

PFREE (adc_results);
}


/*
+--------------------------------------------------------------------+
| PROJECT : XXX                         MODULE  : CST_CSF            |
| STATE   : code                        ROUTINE : csf_aec_enable     |
+--------------------------------------------------------------------+

  PURPOSE : This function sends a primitive to L1A to activate the AEC */


GLOBAL void csf_aec_enable (USHORT aec_ctrl_reg)
{

  // This structure and the #define have to be implemented in the MMI.
  // This is a temporary location for test !!!!!!!
  // TO BE REMOVED BY USER
  typedef struct
  {
    USHORT aec_control;
  }
  T_MMI_AEC_REQ;

  #define MMI_AEC_REQ  ( ( ( 0x18 ) << 8 ) |  40 )

  PALLOC(aec_req, MMI_AEC_REQ);

  aec_req->aec_control  = aec_ctrl_reg;

  PSENDX (L1, aec_req);
}


/*
    Returns the ADC conversions results array
*/

GLOBAL USHORT * csf_return_adc (void)
{
 return &(ADC_CONVERSIONS[0]);
}



#ifdef ALR
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (8403)       MODULE  : CST_CSF                    |
| STATE   : code                ROUTINE : csf_vm_record              |
+--------------------------------------------------------------------+

  PURPOSE : activate the Voice Memo recording process for a ten seconds duration

*/

GLOBAL void csf_vm_record (CHAR *output, UBYTE vm_flash_index)
{
  // Dummy function
}




/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (8403)       MODULE  : CST_CSF                    |
| STATE   : code                ROUTINE : csf_vm_play                |
+--------------------------------------------------------------------+

  PURPOSE : activate the Voice Memo playing process

*/

GLOBAL void csf_vm_play (CHAR *output, UBYTE vm_flash_index)
{
  // Dummy function
}
#endif
