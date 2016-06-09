/*
+-----------------------------------------------------------------------------
|  Project :  COMLIB
|  Modul   :  cl_imei
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
|  Purpose :  Definitions of global types used by common library functions
|             and the prototypes of those functions.
+-----------------------------------------------------------------------------
*/
/*
 *  Version 1.0
 */

/**********************************************************************************/

#ifndef CL_IMEI_H
#define CL_IMEI_H

/**********************************************************************************/

/*
 * Compiler switches
 */

  /* Chipset Type          Value          Value name
   * Gemini/Pole Star        0              _ge
   * Hercules                2              _he
   * Ulysse                  3              _ul
   * Samson                  4              _sa
   * Ulysse G1 13MHz         5              _g1_13
   * Ulysse G1 26MHz         6              _g1_26
   * Calypso C05 (rev. A)    7              _cal
   * Calypso C05 (rev. B)    8              _calb
   * Ulysse C035             9              _ul35
   * Calypso C035            10             _cal35
   * Calypso C035 Lite       11             _lite
   * Calypso+                12             _cplus
   */
/******************************************************************************
 * DIE ID and Platform settings
 *****************************************************************************/
/* DIE ID register
  #define MEM_DEV_ID0     0xFFFEF000
  #define MEM_DEV_ID1     0xFFFEF002
*/
/* For D-Sample: $CHIPSET  =  7 or 8 (=10 for D-sample AMR). */

#ifdef _SIMULATION_
#undef FF_PROTECTED_IMEI
#endif

#if (CHIPSET == 7) || (CHIPSET == 8) || (CHIPSET == 10) || (CHIPSET == 11)
#define CL_IMEI_DIE_ID_REG    (MEM_DEV_ID0 | 0xF010) //+ 0xFFFEF010 for Calypso
#endif

/* DIE ID SIZE is 4 words (16 bits)long */
#define CL_IMEI_SIZE                  8
#define CL_IMEI_ISDID_SIZE            16
#define CL_IMEI_DIE_ID_SIZE           4

/* Return values */
#define CL_IMEI_OK                    0
#define CL_IMEI_ERROR                -1
#define CL_IMEI_INVALID_DIE_ID       -2
#define CL_IMEI_READ_IMEI_FAILED     -3

/* Possible values for imeiType */
#define CL_IMEI_GET_SECURE_IMEI       0
#define CL_IMEI_GET_STORED_IMEI       1
#define CL_IMEI_CONTROL_IMEI          2

/*
+------------------------------------------------------------------------------
| Function    : cl_get_imeisv
+------------------------------------------------------------------------------
| Description : Common IMEI getter function
|
| Parameters  : imeiBufSize  - size of buffer where to store IMEI, min 8 BYTEs
|               *imeiBufPtr  - pointer to buffer where to store the IMEI
|               imeiType     - indicates, if the IMEI should be read from
|                              FFS/Secure ROM (value=CL_IMEI_GET_SECURE_IMEI) or
|                              if the already read and stored IMEI (if available)
|                              should be delivered (value=CL_IMEI_GET_STORED_IMEI)
|                              The second option should be used only by ACI or
|                              BMI to show the IMEISV on mobile's display or
|                              in terminal window, e.g. if user calls *#06#.
|                              For IMEI Control reason (user by ACI), the value
|                              has to be CL_IMEI_CONTROL_IMEI
| Return      :           OK - 0
|                      ERROR - negative values
+------------------------------------------------------------------------------
*/
extern BYTE cl_get_imeisv(USHORT imeiBufSize, UBYTE *imeiBufPtr, UBYTE imeiType);

/**********************************************************************************/


#endif /* CL_IMEI_H */
