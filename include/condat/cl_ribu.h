/*
+-----------------------------------------------------------------------------
|  Project :  COMLIB
|  Modul   :  cl_ribu
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

#ifndef CL_RIBU_H
#define CL_RIBU_H

/**********************************************************************************/

typedef struct
{
  U8 ri;        /* ring buffer read index */
  U8 wi;        /* ring buffer write index */
  U8 depth;     /* ring buffer depth */
  U8 filled;    /* ring buffer filled flag */
} T_RIBU;

typedef struct
{
  U8 type;
  U8 status;
  U8 len;   /* buffer length */
  U8 *buf;  /* buffer start address */
} T_FD;     /* frame descriptor */

typedef struct
{
  T_RIBU idx;
  T_FD **pFDv;  /* frame descriptor vector */
} T_RIBU_FD;

/**********************************************************************************/

EXTERN void cl_ribu_create(T_RIBU_FD **ribu, const U8 buflen, const U8 depth);
EXTERN void cl_ribu_release(T_RIBU_FD **ribu);

EXTERN void cl_ribu_put(const T_FD fd, T_RIBU_FD *ribu);
EXTERN T_FD *cl_ribu_get(T_RIBU_FD *ribu);
EXTERN T_FD *cl_ribu_get_new_frame_desc(T_RIBU_FD *ribu);

EXTERN void cl_ribu_init(T_RIBU *ribu, const U8 depth);
EXTERN U8   cl_ribu_read_index(T_RIBU *ribu);
EXTERN U8   cl_ribu_write_index(T_RIBU *ribu);
EXTERN BOOL cl_ribu_data_avail(const T_RIBU_FD *ribu);

EXTERN  void cl_set_frame_desc(T_FRAME_DESC *frame_desc, U8 *A0, U16 L0, U8 *A1, U16 L1);
EXTERN  void cl_set_frame_desc_0(T_FRAME_DESC *frame_desc, U8 *A0, U16 L0);

#endif /* CL_RIBU_H */
