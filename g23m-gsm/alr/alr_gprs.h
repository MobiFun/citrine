/*
+-----------------------------------------------------------------------------
|  Project :
|  Modul   :  J:\g23m-gsm\alr\alr_gprs.h
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
|  Purpose :
+-----------------------------------------------------------------------------
*/
typedef struct {
  UBYTE v_ptmsi;
  ULONG ptmsi;
  UBYTE v_ptmsi2;
  ULONG ptmsi2;
  UBYTE ign_pgm;
  UBYTE pbcch;
  UBYTE check_bsic;
  UBYTE ptm;
  UBYTE pim;
  UBYTE sync_only;
  UBYTE pcco_active;
} T_ALR_GPRS_DATA;

#define SI13_ON_NBCCH 0
#define SI13_ON_EBCCH 1

void gprs_alr_get_table            (const T_FUNC**      tab,
                                    USHORT*             n);
void gprs_alr_mon_ctrl_req         (T_MPH_MON_CTRL_REQ* ctrl_req);
BOOL gprs_alr_check_packet_paging  (UBYTE*              frame,
                                    UBYTE               which);
BOOL gprs_alr_check_packet_paging_2(UBYTE*              frame,
                                    UBYTE               which);
BOOL gprs_alr_check_ptmsi          (ULONG               ptmsi_pag);
void gprs_alr_store_ptmsi          (UBYTE indic,
                                    ULONG               tmsi);
void gprs_alr_store_ptmsi2         (UBYTE indic2,
                                    ULONG               tmsi2);

void gprs_alr_check_downlink_assign(T_MPHC_DATA_IND*    data_ind);
void gprs_alr_init                 (void);
BOOL gprs_check_read_si13_only     (void);
void gprs_check_page_mode          (T_MPHC_DATA_IND*    data_ind);
void set_gprs_support( UBYTE support );
GLOBAL BOOL gprs_alr_is_supported  (void);
