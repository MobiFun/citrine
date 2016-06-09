/* 
+----------------------------------------------------------------------------- 
|  Project :  GSM-PS (6147)
|  Modul   :  PSA_T30
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
|  Purpose :  Definitions for the protocol stack adapter T30
+----------------------------------------------------------------------------- 
*/ 

#ifndef PSA_T30_H
#define PSA_T30_H

#ifdef FF_FAX

#define REPORT_MAX    128
#define MSG_SIZE_BITS 2880

/*==== CONSTANTS ==================================================*/

typedef enum
{
  NO_VLD_FS = 0,                  /* not a valid fax status */
  FS_IDL,                         /* fax idle */
  FS_SND_DOC,                     /* sending fax document */
  FS_RCV_DOC,                     /* receiving fax document */
  FS_DOC_TRF                      /* fax document transfered */
} T_T30_FXST;

/*==== TYPES ======================================================*/

typedef struct
{
  UBYTE        dir;
  USHORT       l_buf;
  UBYTE        buf[REPORT_MAX];
} T_Report;

typedef struct T30ShrdParm
{
  SHORT         cId;              /* related call id */
  UBYTE         faxStat;          /* status of fax */
  USHORT        trans_rate;       /* t30_activate_req, t30_modify_req */
  UBYTE         half_rate;        /* t30_activate_req, t30_modify_req */
  UBYTE         threshold;        /* t30_activate_req */
  UBYTE         bitord;           /* t30_activate_req */
  USHORT        frames_per_prim;  /* t30_activate_req */
  USHORT        tbs;              /* t30_activate_cnf */
  USHORT        rbs;              /* t30_activate_cnf */
  T_hdlc_info   hdlc_rcv;         /* t30_cap_ind, t30_cap_req */
  T_hdlc_info   hdlc_snd;         /* t30_cap_ind, t30_cap_req */
  UBYTE         sgn_rcv;          /* t30_sgn_ind */
  UBYTE         sgn_snd;          /* t30_sgn_req */
  UBYTE         cmpl;             /* t30_cmpl_ind */
  UBYTE         hdlc_report;      /* t30_config_req */
  U16           test_mode;        /* t30_config_req */
  USHORT        err_cause;        /* t30_error_ind */
  USHORT        eol;              /* t30_info_ind */
  T_Report      report;           /* t30_report_ind */
  UBYTE         bcs_phase;        /* ks */
  UBYTE         msg_phase;
  BOOL          T30_is_activated;
} T_T30_SHRD_PRM;

typedef struct T30TstPrmRef
{
  const char * key;               /* keyword string */
  UBYTE  id;                      /* corresponding id */
} T30_TSTPRM_REF;

/*==== PROTOTYPES =================================================*/

EXTERN SHORT psaT30_Ppm          (void);

#ifdef DTI
EXTERN void psaT30_Dti_Req    (T_DTI_CONN_LINK_ID link_id, UBYTE dti_conn);
#endif /* DTI */

#ifdef FF_FAX
EXTERN void  psaT30_Init         (void);
#endif

EXTERN SHORT psaT30_Ppm       (void);
EXTERN void  psaT30_Modify       (void);
EXTERN void  psaT30_Disconnect   (void);
EXTERN void  psaT30_Config       (void);
EXTERN void  psaT30_Capabilities (void);
EXTERN void  psaT30_Activate     (void);
EXTERN void  psaT30_Deactivate   (void);

#ifdef TRACING
EXTERN void psaT30_shrPrmDump ( void );
#endif

/*==== EXPORT =====================================================*/

#ifdef PSA_T30F_C

GLOBAL T_T30_SHRD_PRM t30ShrdPrm;

#else

EXTERN T_T30_SHRD_PRM t30ShrdPrm;

#endif /* PSA_T30F_C */

#endif /* FF_FAX */

#endif /* PSA_T30_H */

/*==== EOF =======================================================*/

