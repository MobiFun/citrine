/*
+-----------------------------------------------------------------------------
|  Project :  GSM-F&D (8411)
|  Modul   :  RA_L1INT
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
|  Purpose : This Modul defines the Layer 1 Functional Interface (L1INT) of the RA
+-----------------------------------------------------------------------------
*/

#ifndef RA_L1INT_H
#define RA_L1INT_H

#include "cl_ribu.h"

EXTERN void l1i_ra_activate_req
            (
              T_RA_ACTIVATE_REQ *ra_activate_req
            );

EXTERN void l1i_ra_datatrans_req
            (
              T_RA_DATATRANS_REQ *ra_datatrans_req
            );

EXTERN void l1i_ra_deactivate_req
            (
              T_RA_DEACTIVATE_REQ *ra_deactivate_req
            );

EXTERN void l1i_ra_data_req
            (
              T_RA_DATA_REQ *ra_data_req,
              T_FRAME_DESC  *frame_desc
            );

EXTERN void l1i_ra_data_req_new
            (
              T_FD *pFD
            );

EXTERN void l1i_ra_break_req
            (
              T_RA_BREAK_REQ *ra_break_req
            );

EXTERN void l1i_ra_detect_req
            (
              T_RA_DETECT_REQ *ra_detect_req
            );

EXTERN void l1i_ra_modify_req
            (
              T_RA_MODIFY_REQ *ra_modify_req
            );
#endif

