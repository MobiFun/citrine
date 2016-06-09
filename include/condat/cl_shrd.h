/*
+-----------------------------------------------------------------------------
|  Project :  COMLIB
|  Modul   :  cl_shrd
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
#ifndef CL_SHRD_H
#define CL_SHRD_H

#ifdef TI_PS_FF_AT_P_CMD_CTREG
#define MAX_CTREG_TAB_LEN 25
#endif /* TI_PS_FF_AT_P_CMD_CTREG */

/* ME STATUS for Timing advance structure */
#define ME_STATUS_IDLE         0
#define ME_STATUS_NOT_IDLE     1


/*====STRUCTURE DEFINITION=========================================*/

typedef struct
{
  UBYTE service_mode;
  U16   lac;
  U8    mcc [3];
  U8    mnc [3];
  USHORT cell_id;
}T_LOC_INFO;

typedef struct
{
  UBYTE me_status;
  UBYTE tm_adv;
}T_TIM_ADV;

#ifdef TI_PS_FF_AT_P_CMD_CTREG
typedef enum
{
    MODE_NotPresent = 0,
    TREG_READ_MODE,
    TREG_WRITE_MODE
}T_TREG_MODE;

typedef enum
{
    MODE_TIME_NotPresent = 0,
    NOSERVICE_MODE_TIME,
    LIMSERVICE_MODE_TIME
}T_TREG_TABLE;

typedef struct
{
    T_TREG_MODE mode;
    T_TREG_TABLE tab_id;
    UBYTE tab_val[25];
}T_TREG;
#endif /* TI_PS_FF_AT_P_CMD_CTREG */

typedef struct
{
  T_LOC_INFO location_info;
  T_TIM_ADV timing_advance;
#ifdef TI_PS_FF_AT_P_CMD_CTREG
  UBYTE no_serv_mod_time[25];
  UBYTE lim_serv_mod_time[25];
#endif /* TI_PS_FF_AT_P_CMD_CTREG */
}T_SHRD_DATA;

EXTERN T_SHRD_DATA *shared_data;

#ifdef TI_PS_FF_AT_P_CMD_CTREG
EXTERN  const UBYTE no_service_mode_time[25];
EXTERN  const UBYTE lim_service_mode_time[25];
#endif /* TI_PS_FF_AT_P_CMD_CTREG */

/*=================================================================*/

/*====FUNCTION PROTOTYPE===========================================*/

EXTERN void cl_shrd_init (T_HANDLE handle);
EXTERN void cl_shrd_exit (void);
EXTERN BOOL cl_shrd_get_loc (T_LOC_INFO *loc_info);
EXTERN void cl_shrd_set_loc (T_LOC_INFO *loc_info);
EXTERN BOOL cl_shrd_get_tim_adv(T_TIM_ADV *tim_adv);
EXTERN void cl_shrd_set_tim_adv(T_TIM_ADV *tim_adv);

#ifdef TI_PS_FF_AT_P_CMD_CTREG
EXTERN BOOL cl_shrd_set_treg_val(T_TREG *treg);
EXTERN BOOL cl_shrd_get_treg_val(T_TREG *treg);
EXTERN BOOL cl_shrd_get_treg (UBYTE tab_id, UBYTE offset, UBYTE *tab_val);
#endif /* TI_PS_FF_AT_P_CMD_CTREG */

/*=================================================================*/
#endif   /* CL_SHRD_H */
