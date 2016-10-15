/*
+-----------------------------------------------------------------------------
|  Project :  COMLIB
|  Modul   :  cl_imei.c
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
|  Purpose :  Definitions of common library functions: IMEI decryption with
              DES algorithm
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

#ifndef CL_IMEI_C
#define CL_IMEI_C

#include "config.h"
#include "fixedconf.h"

#include "typedefs.h"
#include "vsi.h"        /* to get a lot of macros */

#include "../services/ffs/ffs.h"
#include "../bsp/mem.h"
#include "pcm.h"

#include "cl_imei.h"
#include "cl_des.h"
#include <string.h>

static UBYTE stored_imei[CL_IMEI_SIZE]; /* when the imei is read once, the value
                                             is stored in this buffer */
static UBYTE imei_flag = 0;  /* this flag indicates, if IMEI was successful read
                                and is  stored in the stored_imei buffer */

/* Default IMEISV for D-Sample 00440000-350-111-20 */
const  UBYTE C_DEFAULT_IMEISV_DSAMPLE[CL_IMEI_SIZE] =
             {0x00, 0x44, 0x00, 0x00, 0x35, 0x01, 0x11, 0x20};
#define CL_IMEI_FFS_PATH   "/etc/IMEISV"

/*==== FUNCTIONS ==================================================*/

#if CONFIG_TARGET_PIRELLI
/*
+------------------------------------------------------------------------------
| Function    : get_dieID
+------------------------------------------------------------------------------
| Description : the function reads the Die-ID from base band processor and
|               extracts it from 4 BYTEs to 8 BYTEs.
|
| Parameters  : inBufSize  - size of buffer where to store Die ID, min.8 BYTE
|               *outBufPtr - pointer to buffer where to store the Die ID
| Return      : void
+------------------------------------------------------------------------------
*/
LOCAL void get_dieID(USHORT inBufSize, UBYTE *outBufPtr)
{
  int i;
  USHORT *outBuf16 = (USHORT*)&outBufPtr[0];
  volatile USHORT *reg_p = (USHORT *) CL_IMEI_DIE_ID_REG;

  TRACE_FUNCTION("get_dieID()");

  if(inBufSize < CL_IMEI_DIE_ID_SIZE){
    TRACE_ERROR("CL IMEI ERROR: buffer size for Die ID to short!");
  }
#ifdef IMEI_DEBUG
  TRACE_EVENT_P1("CL IMEI INFO: Die-ID address(0x%x)", CL_IMEI_DIE_ID_REG);
#endif
  for (i = 0; i < CL_IMEI_DIE_ID_SIZE; i++) {
    /* Die ID is 4 BYTE long, extract it to 8 BYTE. */
    outBuf16[i] = (USHORT)(*(UINT8*)(reg_p)++);
  }
}

extern int pirelli_read_factory_record(uint32 offset, void *userbuf,
					T_FFS_SIZE size, int has_chksum);

#define	PIRELLI_IMEI_OFFSET	0x504

/*
+------------------------------------------------------------------------------
| Function    : pirelli_get_imeisv
+------------------------------------------------------------------------------
| Description : This function attempts to read and decrypt a valid IMEISV
|		record from Pirelli's factory data block.
|
| Parameters  : inBufSize  - size of buffer where to store IMEI, min. 8 BYTE
|               *outBufPtr - pointer to buffer where to store the IMEI
| Return      :              0 - OK
|                           <0 - ERROR
+------------------------------------------------------------------------------
*/
LOCAL BYTE pirelli_get_imeisv (USHORT inBufSize, UBYTE *outBufPtr)
{
  UBYTE isdid_buf[CL_IMEI_ISDID_SIZE];
  UBYTE r_dieId[CL_DES_KEY_SIZE]; /* read Die ID */
  UBYTE d_dieId[CL_DES_KEY_SIZE]; /* deciphered Die ID */
  SHORT ret;

  TRACE_FUNCTION("pirelli_get_imeisv()");

  if(inBufSize < CL_IMEI_SIZE){
    TRACE_ERROR("CL IMEI ERROR: buffer size for IMEI to short!");
    return CL_IMEI_ERROR;
  }

  /*
   * Read ISDID(enciphered IMEISV+DieID) from FFS.
   * (changed to read from Pirelli's factory data block instead)
   */
  if((ret = pirelli_read_factory_record(PIRELLI_IMEI_OFFSET, isdid_buf,
					CL_IMEI_ISDID_SIZE, 0)) >= EFFS_OK)
  {
   /*
    * Read Die ID for using as DES key
    */
    get_dieID(CL_DES_KEY_SIZE, r_dieId);
   /*
    * Call DES algorithm routine
    */
    /* decipher first 8 BYTEs */
    cl_des(&isdid_buf[0], r_dieId, outBufPtr, CL_DES_DECRYPTION);
    /* decipher the rest 8 BYTEs */
    cl_des(&isdid_buf[CL_DES_BUFFER_SIZE], r_dieId, d_dieId, CL_DES_DECRYPTION);
    if(!memcmp(d_dieId, r_dieId, CL_DES_KEY_SIZE))
    {
      /* Die ID is valid  */
      ret = CL_IMEI_OK;
    } else {/* Die ID is corrupted */
      char pr_buf[126];
      TRACE_ERROR("CL IMEI ERROR: Die ID is corrupted");
      sprintf(pr_buf,"Read DieID: %02x %02x %02x %02x %02x %02x %02x %02x",
                      r_dieId[0], r_dieId[1], r_dieId[2], r_dieId[3],
                      r_dieId[4], r_dieId[5], r_dieId[6], r_dieId[7]);
      TRACE_ERROR(pr_buf);
      sprintf(pr_buf,"Deciphered DieID: %02x %02x %02x %02x %02x %02x %02x %02x",
                      d_dieId[0], d_dieId[1], d_dieId[2], d_dieId[3],
                      d_dieId[4], d_dieId[5], d_dieId[6], d_dieId[7]);
      TRACE_ERROR(pr_buf);

      ret = CL_IMEI_INVALID_DIE_ID;
    }
  } else {
    ret = CL_IMEI_READ_IMEI_FAILED;
  }

  return ret;

}/* pirelli_get_imeisv() */
#endif	/* CONFIG_TARGET_PIRELLI */


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
|                              For IMEI Control reason (used by ACI), the value
|                              has to be CL_IMEI_CONTROL_IMEI
| Return      :           OK - 0
|                      ERROR - negative values
+------------------------------------------------------------------------------
*/
extern BYTE cl_get_imeisv(USHORT imeiBufSize, UBYTE *imeiBufPtr, UBYTE imeiType)
{
  UBYTE buf[SIZE_EF_IMEI];
  BYTE ret;

  TRACE_FUNCTION("cl_get_imeisv()");

  /*
   * The user has required a stored IMEI. If it has been already read
   * and stored, so return stored IMEI
   */
  if((imeiType == CL_IMEI_GET_STORED_IMEI) && (imei_flag == 1)){
    memcpy(imeiBufPtr, stored_imei, CL_IMEI_SIZE);
    return CL_IMEI_OK;
  }

  /*
   * The user has required a "secure" IMEI.  Look in /etc/IMEISV first,
   * then in /pcm/IMEI.  And if we are running on the Pirelli target,
   * try their factory IMEI record last.
   */

  if (ffs_file_read(CL_IMEI_FFS_PATH, imeiBufPtr, CL_IMEI_SIZE) >= EFFS_OK) {
    TRACE_EVENT("CL IMEI INFO: return IMEI-SV number from ffs:/etc/IMEISV");
    memcpy(stored_imei, imeiBufPtr, CL_IMEI_SIZE);
    imei_flag = 1;
    return CL_IMEI_OK;
  }

  if (ffs_file_read("/pcm/IMEI", buf, CL_IMEI_SIZE) >= EFFS_OK) {
    TRACE_EVENT("CL IMEI INFO: return IMEI-SV number from ffs:/pcm/IMEI");
    /*
     * swap digits
     */
    imeiBufPtr[0] = ((buf[0] & 0xf0) >> 4) | ((buf[0] & 0x0f) << 4);
    imeiBufPtr[1] = ((buf[1] & 0xf0) >> 4) | ((buf[1] & 0x0f) << 4);
    imeiBufPtr[2] = ((buf[2] & 0xf0) >> 4) | ((buf[2] & 0x0f) << 4);
    imeiBufPtr[3] = ((buf[3] & 0xf0) >> 4) | ((buf[3] & 0x0f) << 4);
    imeiBufPtr[4] = ((buf[4] & 0xf0) >> 4) | ((buf[4] & 0x0f) << 4);
    imeiBufPtr[5] = ((buf[5] & 0xf0) >> 4) | ((buf[5] & 0x0f) << 4);
    imeiBufPtr[6] = ((buf[6] & 0xf0) >> 4) | ((buf[6] & 0x0f) << 4);
    imeiBufPtr[7] = ((buf[7] & 0xf0) >> 4) | ((buf[7] & 0x0f) << 4);
    /* store IMEI */
    memcpy(stored_imei, imeiBufPtr, CL_IMEI_SIZE);
    imei_flag = 1;
    return CL_IMEI_OK;
  }

#if CONFIG_TARGET_PIRELLI
  ret = pirelli_get_imeisv (imeiBufSize, imeiBufPtr);
  if (ret == CL_IMEI_OK) {
    memcpy(stored_imei, imeiBufPtr, CL_IMEI_SIZE);
    imei_flag = 1;
    return ret;
  }
#else
  ret = CL_IMEI_READ_IMEI_FAILED;
#endif
  TRACE_ERROR("CL IMEI FATAL ERROR: IMEI not available!");
  memcpy(imeiBufPtr, C_DEFAULT_IMEISV_DSAMPLE, CL_IMEI_SIZE);
  return ret;
}

#endif /* CL_IMEI_C */
