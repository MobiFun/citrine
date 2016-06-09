/*
+-----------------------------------------------------------------------------
|  Project :
|  Modul   : sas_dmi.h
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
|  Purpose : External interface for SAS_DMI support module
+-----------------------------------------------------------------------------
*/

#ifndef ANITESAS_DMI__H
#define ANITESAS_DMI__H

/*==== INCLUDES =============================================================*/
/*==== CONSTS ===============================================================*/

/*
*   Nasty Tricks Department
*/

#define NULLSTRING    \0

#define DMIVSI_CALLER proxyVsiCallerId,
#define DMIVSI_CALLER_SINGLE proxyVsiCallerId
#define G23_COMP_REF_TYPE T_HANDLE
#define G23_NULL_COMP_REF (T_HANDLE)(0)

/* End of N.T.D. */

/*
*   SAS DMI - Activate / Deactivate Commands
*             (commands+values defined in Anite
*             DMID spec. I2477D)
*/

#define G23_COMP_RR         1
#define G23_COMP_ALR        2
#define G23_COMP_PHY        3
#define G23_COMP_CCD        9

#define SASRPT_ON           1
#define SASRPT_OFF          0
#define SASRPT_IS_UPLINK    0
#define SASRPT_IS_DOWNLINK  1
#define SASRPT_MSGRSP       0
#define SASRPT_MSGIGN       1
#define SASRPT_IDLEMODE     1
#define SASRPT_DEDIMODE     2

/*
*   SAS DMID - Activation/Deactivation Flags
*               (set by corresponding SAS DMID
*                commands - see above)
*/

#define SAS_RPRT_ACTIVE__IDLEMODE              0x0001
#define SAS_RPRT_ACTIVE__DEDICATEDMODE         0x0002
#define SAS_RPRT_ACTIVE__L3MSG                 0x0004
#define SAS_RPRT_ACTIVE__L1MSG                 0x0008
#define SAS_RPRT_ACTIVE__SYNC                  0x0010
#define SAS_RPRT_ACTIVE__SACCH                 0x0020
#define SAS_RPRT_ACTIVE__SACCHCOMPLETE         0x0040
#define SAS_RPRT_ACTIVE__CHANREQIMMEDASSIGN    0x0080
#define SAS_RPRT_ACTIVE__BCCH                  0x0100
#define SAS_RPRT_ACTIVE__PAGE                  0x0200

/*
*   extern procedure prototypes
*/

#define START_FMT1    1
#define START_FMT2    2

/*==== TYPES ================================================================*/
typedef struct _start_du
{
  U8    fmtDiscriminator;
  union
  {
    U8              dummyPlaceHolder;
#ifdef  __T_start__
    T_start*        fmt1_Start;
#endif
#ifdef  __T_start_time__
    T_start_time*   fmt2_Start;
#endif
  } u;
} start_u;

/*==== EXPORTS ==============================================================*/
extern  void  SasDmi( U8 g23Component );

extern  void  Dmi_SendReport_IdleMode( U8 g23Comp,
                                       G23_COMP_REF_TYPE g23CompRef,
                                       U16 Ssf,
                                       U8 Ssr,
                                       U16* Nf,
                                       U8* Nr,
                                       U8* Nb );
extern  void  Dmi_SendReport_DedMode( U8 g23Comp,
                                      G23_COMP_REF_TYPE g23CompRef,
                                      U16 Ta,
                                      U8 Pwr,
                                      U8 Rlf,
                                      U8 Rqf,
                                      U8 Rls,
                                      U8 Rqs, 
                                      U16* Nf,
                                      U8* Nr,
                                      U8* Nb );

extern  void  Dmi_SendReport_L3( U8 g23Comp,
                                 G23_COMP_REF_TYPE g23CompRef,
                                 U8 UlOrDl,
                                 U16 L3MsgLen,
                                 U8* L3Msg,
                                 U8 L3MsgId );

extern  void  Dmi_SendReport_L1( U8 g23Comp, G23_COMP_REF_TYPE g23CompRef );

extern  void  Dmi_SendReport_Sync( U8 g23Comp,
                                   G23_COMP_REF_TYPE g23CompRef,
                                   U16 Fff,
                                   U8 Bsic,
                                   U32 Toffs );
extern  void  Dmi_SendReport_Sacch( U8 g23Comp,
                                    G23_COMP_REF_TYPE g23CompRef,
                                    U8 Mrltc,
                                    U8 Crltc,
                                    U8 L3SiPdTi,
                                    U8 L3MsgTyp );

extern  void  Dmi_SendReport_SacchCompl( U8 g23Comp,
                                         G23_COMP_REF_TYPE g23CompRef,
                                         U16 MsgLen,
                                         U8 Mrltc,
                                         U8 Crltc,
                                         U8* L2Msg );

extern  void  Dmi_SendReport_ChanReq( U8 g23Comp,
                                      G23_COMP_REF_TYPE g23CompRef,
                                      U8 ChanReqData,
                                      U32 ChanReqFrNum );
extern  void  Dmi_SendReport_ImmedAssign( U8 g23Comp,
                                          G23_COMP_REF_TYPE g23CompRef,
                                          U16 L3MsgLen,
                                          U8 RspOrIgn,
                                          U8* L3Msg );
extern  void  Dmi_SendReport_Bcch( U8 g23Comp,
                                   G23_COMP_REF_TYPE g23CompRef,
                                   U16 L3MsgLen,
                                   U16 BcchFreq,
                                   U8* L3BcchMsg );
extern  void  Dmi_SendReport_Page( U8 g23Comp,
                                   G23_COMP_REF_TYPE g23CompRef,
                                   U16 L3MsgLen,
                                   U8 Mdsc,
                                   U8 Cdsc,
                                   U8* L3PageMsg );

extern  void  Dmi_SendReport_IdleChan( U8 g23Comp,
                                       G23_COMP_REF_TYPE g23CompRef,
                                       U16 Fff,
                                       U8 Bsic,
                                       U8 CombFlag,
                                       U8 CchC,
                                       U8 Mfrm,
                                       U8 AgRes,
                                       U8 CchG,
                                       U8 Pmfrm,
                                       U8 Pbi );

extern  void  Dmi_SendReport_DedChan_NonHop( U8 g23Comp,
                                             G23_COMP_REF_TYPE g23CompRef,
                                             U16 ScFreq,
                                             U8 ScBsic,
                                             U8 ChTyp, 
                                             U8 Tslot,
                                             U8 SubCh,
                                             U8 Tsc,
                                             U8 GsmBa,
                                             U16 DedFreq );

extern  void  Dmi_SendReport_DedChan_Hop( U8 g23Comp,
                                          G23_COMP_REF_TYPE g23CompRef,
                                          U16 ScFreq,
                                          U8 ScBsic,
                                          U8 ChTyp,
                                          U8 Tslot, 
                                          U8 SubCh,
                                          U8 Tsc,
                                          U8 Maio,
                                          U8 Hsn,
                                          U8 GsmBa, 
                                          start_u* StartTime,
                                          U8 MaxLenChanList, 
                                          U8* BeforeChanList,
                                          U8* AfterChanList );

/*---------------------------------------------------*/

/*+Obsolete*/
/*
*   Features potentially obsoleted by Issue 1.4 of
*   Anite DMI spec.
*
*   ** LEAVE IN CODE SINCE REFERENCED IN dmi.c
*
*               
*/

/* ...continuous reports... */

#define SAS_RPRT_START__IDLEMODE              '1'
#define SAS_RPRT_STOP__IDLEMODE               '6'
#define SAS_RPRT_START__DEDICATEDMODE         '2'
#define SAS_RPRT_STOP__DEDICATEDMODE          '7'
#define SAS_RPRT_START__L3MSG                 '3'
#define SAS_RPRT_STOP__L3MSG                  '8'
#define SAS_RPRT_START__L1MSG                 '4'
#define SAS_RPRT_STOP__L1MSG                  '9'
#define SAS_RPRT_START__SYNC                  'S'
#define SAS_RPRT_STOP__SYNC                   'T'
#define SAS_RPRT_START__SACCH                 'A'
#define SAS_RPRT_STOP__SACCH                  'B'
#define SAS_RPRT__SACCHCOMPLETE               '\\'
#define SAS_RPRT_START__SACCHCOMPLETE         'A'
#define SAS_RPRT_STOP__SACCHCOMPLETE          'B'
#define SAS_RPRT_START__CHANREQIMMEDASSIGN    'C'
#define SAS_RPRT_STOP__CHANREQIMMEDASSIGN     'D'
#define SAS_RPRT_START__BCCH                  'E'
#define SAS_RPRT_STOP__BCCH                   'F'
#define SAS_RPRT_START__PAGE                  'P'
#define SAS_RPRT_STOP__PAGE                   'Q'

/* ...once-off reports... */

#define SAS_RPRT_ONCE__IDLECHAN               I
#define SAS_RPRT_ONCE__DEDICATEDCHAN          J

/* ...status getters/setters... */

extern  U16  GetSasRptStatus_IdleMode( void );
extern  U16  GetSasRptStatus_DedMode( void );
extern  U16  GetSasRptStatus_L3( void );
extern  U16  GetSasRptStatus_L1( void );
extern  U16  GetSasRptStatus_Sync( void );
extern  U16  GetSasRptStatus_Sacch( void );
extern  U16  GetSasRptStatus_SacchCompl( void );
extern  U16  GetSasRptStatus_ChanReq( void );
extern  U16  GetSasRptStatus_Bcch( void );
extern  U16  GetSasRptStatus_Page( void );

extern  U16  HandleSasCmd( U8* SasCmd );

extern  void  SetSasRptStatus_IdleMode( U16 OnOff );
extern  void  SetSasRptStatus_DedMode( U16 OnOff );
extern  void  SetSasRptStatus_L3( U16 OnOff );
extern  void  SetSasRptStatus_L1( U16 OnOff );
extern  void  SetSasRptStatus_Sync( U16 OnOff );
extern  void  SetSasRptStatus_Sacch( U16 OnOff );
extern  void  SetSasRptStatus_SacchCompl( U16 OnOff );
extern  void  SetSasRptStatus_ChanReq( U16 OnOff );
extern  void  SetSasRptStatus_Bcch( U16 OnOff );
extern  void  SetSasRptStatus_Page( U16 OnOff );

/*-Obsolete*/

/*---------------------------------------------------*/



#endif  /* ANITESAS_DMI__H */
