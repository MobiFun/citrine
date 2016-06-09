/* 
+----------------------------------------------------------------------------- 
|  Project :  GSM-F&D (8411)
|  Modul   :  KSD
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
|  Purpose :  This module is used for decoding of EMMI key sequences.
+----------------------------------------------------------------------------- 
*/ 

#ifndef KSD_C
#define KSD_C
#endif

#include "config.h"
#include "fixedconf.h"
#include "condat-features.h"
#include "aci_conf.h"

#include "aci_all.h"

/*==== INCLUDES ===================================================*/
#include "aci_cmh.h"
#include "psa.h"
#include "cmh.h" 
#include "ksd_utl.h"
#include "ksd.h"
#include "psa_mm.h"
#include "cmh_mm.h"

#ifdef SMI
#include "cus_smi.h"
#endif
//TISH, patch for OMAPS00110151/OMAPS00130693
//start
EXTERN SHORT psaCC_ctbCountActiveCall ( void );
//end

/*==== CONSTANTS ==================================================*/

#define MIN_USSD_LEN 2    /* the minimum length of a USSD         */

#define DATA_CALL   '?'  /* a phone number terminated with this  */
                          /* character leads to setup a data call */
#define START_SEQ_1 '*'   /* an USSD starts with a '*' or a '#'   */
#define START_SEQ_2 '#'   /* an USSD starts with a '*' or a '#'   */
#define START_SI    '*'   /* supplemetary information fields are  */
                          /* separated by this character          */
#define STOP_SEQ    '#'   /* any keystroke sequence is terminated */
                          /* by this character                    */
#define NULL_TERM   '\0'  /* string termination                   */
#define DONT_CARE   '?'   /* either '#' or '*'                    */
#define DIAL_IDX    '#'   /* indicates reading an index used      */
                          /* for abbreviated dialling             */

#define DATA_SPEECH_SEQ "##" /* keystroke sequence to start       */
                             /* data followed by speech mode      */
#define SPEECH_DATA_SEQ "**" /* keystroke sequence to start       */
                             /* speech followed by data mode      */

#define MAX_KEYSEQ_SIZE 170  /* must be equal to MAX_KEYSEQ_SIZE  */
                             /* of file aca.h in project SMI      */

/*==== TYPES ======================================================*/

typedef enum    /* indicates type of supplementary information */
{
  SI_UBYTE,
  SI_USHORT,
  SI_ULONG,
  SI_BYTE,
  SI_SHORT,
  SI_LONG,
  SI_CHAR_STAR
}
T_KSD_SI;

typedef union   /* decoded supplementary information */
{
  UBYTE  siUbyte;
  USHORT siUshort;
  ULONG  siUlong;
  BYTE   siByte;
  SHORT  siShort;
  LONG   siLong;
  CHAR*  siCharStar;
}
T_KSD_SIPARAM;

typedef struct  /* decoded parameter and predefined parameter type */
{
  T_KSD_SIPARAM* param;
  T_KSD_SI       type;
}
T_KSD_DCD_CPLE;

typedef BOOL T_KSD_SDF (USHORT          inIdx,
                        CHAR*           inSeq,
                        T_KSD_SEQGRP*   outSeqGrp,
                        CHAR**          outRestSeq,
                        T_KSD_SEQPARAM* outSeqParam);

typedef struct  /* structure of key sequence table */
{
  CHAR*        keySeq;
  T_KSD_SDF*   func;
}
T_KEY_SEQ_TABLE;

/*==== EXPORT =====================================================*/

/*==== PROTOTYPES==================================================*/

LOCAL BOOL ksd_extractSi (CHAR*           inSeq,
                          USHORT          inSiNum,
                          USHORT*         outCharsNum,
                          T_KSD_DCD_CPLE* inOutSi);

LOCAL T_KSD_SDF ksd_seqMsOff;
LOCAL T_KSD_SDF ksd_seqMsOn;

LOCAL T_KSD_SDF ksd_seqHookOff;
LOCAL T_KSD_SDF ksd_seqHookOn;

#ifdef SMI
LOCAL T_KSD_SDF ksd_seqKeypadInd;
LOCAL T_KSD_SDF ksd_seqStartLcdTest;
LOCAL T_KSD_SDF ksd_seqSetAbbrDial;
LOCAL T_KSD_SDF ksd_seqShowCallTable;

LOCAL T_KSD_SDF ksd_seqSmsSend;
LOCAL T_KSD_SDF ksd_seqSmsSendFromMem;
LOCAL T_KSD_SDF ksd_seqSmsWrite;
LOCAL T_KSD_SDF ksd_seqSmsDelete;
LOCAL T_KSD_SDF ksd_seqSmsRead;
LOCAL T_KSD_SDF ksd_seqSmsList;
LOCAL T_KSD_SDF ksd_seqSmsReadSingle;

LOCAL T_KSD_SDF ksd_seqSndCbst;
LOCAL T_KSD_SDF ksd_seqSndCrlp;
LOCAL T_KSD_SDF ksd_seqSndDs;

LOCAL T_KSD_SDF ksd_seqSpeechData;
LOCAL T_KSD_SDF ksd_seqDataSpeech;
#endif

#ifdef EM_MODE
LOCAL T_KSD_SDF ksd_em_mode;
#endif

LOCAL T_KSD_SDF ksd_seqDtmf;

LOCAL T_KSD_SDF ksd_seqStartReg;
LOCAL T_KSD_SDF ksd_seqAutoReg;
LOCAL T_KSD_SDF ksd_seqManReg;
LOCAL T_KSD_SDF ksd_seqAutoManReg;
LOCAL T_KSD_SDF ksd_seqSetManReg;
LOCAL T_KSD_SDF ksd_seqSetAutoReg;
LOCAL T_KSD_SDF ksd_seqStartPLMN;

LOCAL T_KSD_SDF ksd_seqActSimLock;
LOCAL T_KSD_SDF ksd_seqDeactSimLock;
LOCAL T_KSD_SDF ksd_seqIntrgtSimLock;

LOCAL T_KSD_SDF ksd_seqDial;
LOCAL T_KSD_SDF ksd_seqDialIdx;
LOCAL T_KSD_SDF ksd_seqUssd;

LOCAL T_KSD_SDF ksd_seqActAllCf;
LOCAL T_KSD_SDF ksd_seqDeactAllCf;
LOCAL T_KSD_SDF ksd_seqRegAllCf;
LOCAL T_KSD_SDF ksd_seqEraseAllCf;
LOCAL T_KSD_SDF ksd_seqIntrgtAllCf;

LOCAL T_KSD_SDF ksd_seqActAllCondCf;
LOCAL T_KSD_SDF ksd_seqDeactAllCondCf;
LOCAL T_KSD_SDF ksd_seqRegAllCondCf;
LOCAL T_KSD_SDF ksd_seqEraseAllCondCf;
LOCAL T_KSD_SDF ksd_seqIntrgtAllCondCf;

LOCAL T_KSD_SDF ksd_seqActUncondCf;
LOCAL T_KSD_SDF ksd_seqDeactUncondCf;
LOCAL T_KSD_SDF ksd_seqRegUncondCf;
LOCAL T_KSD_SDF ksd_seqEraseUncondCf;
LOCAL T_KSD_SDF ksd_seqIntrgtUncondCf;

LOCAL T_KSD_SDF ksd_seqActBusyCf;
LOCAL T_KSD_SDF ksd_seqDeactBusyCf;
LOCAL T_KSD_SDF ksd_seqRegBusyCf;
LOCAL T_KSD_SDF ksd_seqEraseBusyCf;
LOCAL T_KSD_SDF ksd_seqIntrgtBusyCf;

LOCAL T_KSD_SDF ksd_seqActNoRepCf;
LOCAL T_KSD_SDF ksd_seqDeactNoRepCf;
LOCAL T_KSD_SDF ksd_seqRegNoRepCf;
LOCAL T_KSD_SDF ksd_seqEraseNoRepCf;
LOCAL T_KSD_SDF ksd_seqIntrgtNoRepCf;

LOCAL T_KSD_SDF ksd_seqActNotReachCf;
LOCAL T_KSD_SDF ksd_seqDeactNotReachCf;
LOCAL T_KSD_SDF ksd_seqRegNotReachCf;
LOCAL T_KSD_SDF ksd_seqEraseNotReachCf;
LOCAL T_KSD_SDF ksd_seqIntrgtNotReachCf;

LOCAL T_KSD_SDF ksd_seqDeactOutBarrServ;

LOCAL T_KSD_SDF ksd_seqActBaoicExcHome;
LOCAL T_KSD_SDF ksd_seqDeactBaoicExcHome;
LOCAL T_KSD_SDF ksd_seqIntrgtBaoicExcHome;

LOCAL T_KSD_SDF ksd_seqActBaoic;
LOCAL T_KSD_SDF ksd_seqDeactBaoic;
LOCAL T_KSD_SDF ksd_seqIntrgtBaoic;

LOCAL T_KSD_SDF ksd_seqDeactAllBarrServ;

LOCAL T_KSD_SDF ksd_seqActBaoc;
LOCAL T_KSD_SDF ksd_seqDeactBaoc;
LOCAL T_KSD_SDF ksd_seqIntrgtBaoc;

LOCAL T_KSD_SDF ksd_seqDeactInBarrServ;

LOCAL T_KSD_SDF ksd_seqActBaicRoam;
LOCAL T_KSD_SDF ksd_seqDeactBaicRoam;
LOCAL T_KSD_SDF ksd_seqIntrgtBaicRoam;

LOCAL T_KSD_SDF ksd_seqActBaic;
LOCAL T_KSD_SDF ksd_seqDeactBaic;
LOCAL T_KSD_SDF ksd_seqIntrgtBaic;

LOCAL T_KSD_SDF ksd_seqIntrgtClir;
LOCAL T_KSD_SDF ksd_seqSupClir;
LOCAL T_KSD_SDF ksd_seqInvClir;

LOCAL T_KSD_SDF ksd_seqIntrgtClip;
LOCAL T_KSD_SDF ksd_seqSupClip;
LOCAL T_KSD_SDF ksd_seqInvClip;

LOCAL T_KSD_SDF ksd_seqIntrgtColr;
LOCAL T_KSD_SDF ksd_seqSupColr;
LOCAL T_KSD_SDF ksd_seqInvColr;

LOCAL T_KSD_SDF ksd_seqIntrgtColp;
LOCAL T_KSD_SDF ksd_seqSupColp;
LOCAL T_KSD_SDF ksd_seqInvColp;

LOCAL T_KSD_SDF ksd_seqSupTTY;
LOCAL T_KSD_SDF ksd_seqReqTTY;

LOCAL T_KSD_SDF ksd_seqRegPwdAllBarrServ;
LOCAL T_KSD_SDF ksd_seqRegPwdAllServ;

#if 0  /* For further study, so not yet used */
LOCAL T_KSD_SDF ksd_seqActCCBS;
#endif
LOCAL T_KSD_SDF ksd_seqDeactCCBS;
LOCAL T_KSD_SDF ksd_seqIntrgtCCBS;
LOCAL T_KSD_SDF ksd_seqIntrgtCNAP;

LOCAL T_KSD_SDF ksd_seqChngPin2;
LOCAL T_KSD_SDF ksd_seqChngPin;
LOCAL T_KSD_SDF ksd_seqUnblckPin2;
LOCAL T_KSD_SDF ksd_seqUnblckPin;

LOCAL T_KSD_SDF ksd_seqActWait;
LOCAL T_KSD_SDF ksd_seqDeactWait;
LOCAL T_KSD_SDF ksd_seqIntrgtWait;

LOCAL T_KSD_SDF ksd_seqPrsntImei;

LOCAL T_KSD_SDF ksd_seqSndChld0;
LOCAL T_KSD_SDF ksd_seqSndChld1;
LOCAL T_KSD_SDF ksd_seqSndChld2;
LOCAL T_KSD_SDF ksd_seqSndChld3;
LOCAL T_KSD_SDF ksd_seqSndChld4;
LOCAL T_KSD_SDF ksd_seqSndChld4Star;
LOCAL T_KSD_SDF ksd_seqSndChld5;

LOCAL T_KSD_SDF ksd_seqRegPwdOutBarrServ;
LOCAL T_KSD_SDF ksd_seqRegPwdInBarrServ;
LOCAL T_KSD_SDF ksd_seqActOutBarrServ;
LOCAL T_KSD_SDF ksd_seqActInBarrServ;
LOCAL T_KSD_SDF ksd_seqActAllBarrServ;

LOCAL T_KSD_SDF ksd_seqUnknown;


/*LOCAL BOOL ksd_isUSSD (CHAR *keySeq); */

/*==== VARIABLES ==================================================*/

/* 
 * keystroke sequence table, additionally used while at
 * least one call is in
 * CAL_STAT_Held, CAL_STAT_Active,
 * CAL_STAT_Dial (CCBS), 
 * CAL_STAT_Incomming (CD), CAL_STAT_Wait (CD)
 */
LOCAL const T_KEY_SEQ_TABLE keyWithinCall[] =
{
  /* 000 - 0 followed by send */
  "0",       ksd_seqSndChld0,
  /* 001 - 1 followed by send */
  "1",       ksd_seqSndChld1,
  /* 002 - 2 followed by send */
  "2",       ksd_seqSndChld2,
  /* 003 - 3 followed by send */
  "3",       ksd_seqSndChld3,
  /* 004 - 4 followed by star */
  "4*",      ksd_seqSndChld4Star,
  /* 005 - 4 followed by send */
  "4",       ksd_seqSndChld4,
  /* 006 - 5 followed by send */
  "5",       ksd_seqSndChld5,

  /* 004 - unknown */
  NULL,      ksd_seqUnknown
};

/* keystroke sequence table for sequences including     */
/* passwords                                            */
LOCAL const T_KEY_SEQ_TABLE keyPasswd[] =
{
  /* 000 - deactivate outgoing barring services */
  "#333?",     ksd_seqDeactOutBarrServ,
  /* 001 - activate BAOIC except home */
  "*332?",     ksd_seqActBaoicExcHome,
  /* 002 - deactivate BAOIC except home */
  "#332?",     ksd_seqDeactBaoicExcHome,
  /* 003 - interrogate BAOIC except home */
  "*#332?",    ksd_seqIntrgtBaoicExcHome,
  /* 004 - interrogate BAOIC */
  "*#331?",    ksd_seqIntrgtBaoic,
  /* 005 - activate BAOIC */
  "*331?",     ksd_seqActBaoic,
  /* 006 - deactivate BAOIC */
  "#331?",     ksd_seqDeactBaoic,
  /* 007 - deactivate all barring services */
  "#330?",     ksd_seqDeactAllBarrServ,
  /* 008 - activate BAOC */
  "*33?",      ksd_seqActBaoc,
  /* 009 - deactivate BAOC */
  "#33?",      ksd_seqDeactBaoc,
  /* 010 - interrogate BAOC */
  "*#33?",     ksd_seqIntrgtBaoc,
  /* 011 - deactivate incoming barring services */
  "#353?",     ksd_seqDeactInBarrServ,
  /* 012 - activate BAIC roaming */
  "*351?",     ksd_seqActBaicRoam,
  /* 013 - deactivate BAIC roaming */
  "#351?",     ksd_seqDeactBaicRoam,
  /* 014 - interrogate BAIC roaming */
  "*#351?",    ksd_seqIntrgtBaicRoam,
  /* 015 - activate BAIC */
  "*35?",      ksd_seqActBaic,
  /* 016 - deactivate BAIC */
  "#35?",      ksd_seqDeactBaic,
  /* 017 - interrogate BAIC */
  "*#35?",     ksd_seqIntrgtBaic,

  /* 018 - register password all barring */
  "*03*330?",  ksd_seqRegPwdAllBarrServ,
  /* 019 - register password for all services */
  "*03*?",     ksd_seqRegPwdAllServ,

  /* 020 - register password all barring */
  "**03*330?", ksd_seqRegPwdAllBarrServ,
  /* 021 - register password for all services */
  "**03*?",    ksd_seqRegPwdAllServ,

  /* 022 - change PIN2 */
  "**042?",    ksd_seqChngPin2,
  /* 023 - change PIN */
  "**04?",     ksd_seqChngPin,
  /* 024 - unblock PIN2 */
  "**052?",    ksd_seqUnblckPin2,
  /* 025 - unblock PIN */
  "**05?",     ksd_seqUnblckPin,
  
  /* 026 - register passwd for outgoing barring services */
  "**03*333?", ksd_seqRegPwdOutBarrServ,
  /* 027 - register passwd for incoming barring services */
  "**03*353?", ksd_seqRegPwdInBarrServ,
  /* 028 - activation for outgoing barring services */
  "*333?",    ksd_seqActOutBarrServ,
  /* 029 - activation for incoming barring services */
  "*353?",    ksd_seqActInBarrServ,
  /* 030 - activation for all barring services */
  "*330?",    ksd_seqActAllBarrServ,

  /* 031 - unknown */
  NULL,        ksd_seqUnknown
};

/* keystroke sequence table for sequences including     */
/* passwords                                            */
LOCAL const T_KEY_SEQ_TABLE keyPasswdNonGsm[] =
{
  /* 000 - lock SIM card */
  "*77742?",   ksd_seqActSimLock,
  /* 001 - unlock SIM card */
  "#77742?",   ksd_seqDeactSimLock,

  /* 002 - unknown */
  NULL,        ksd_seqUnknown
};

/* standard keystroke sequence table */
LOCAL const T_KEY_SEQ_TABLE keyNoFDNSeqTable[] =
{
  /* 001 - present IMEI */
  "*#06#",     ksd_seqPrsntImei,
  /* 002 - change PIN2 */
  "**042?",    ksd_seqChngPin2,
  /* 003 - change PIN */
  "**04?",     ksd_seqChngPin,
  /* 004 - unblock PIN2 */
  "**052?",    ksd_seqUnblckPin2,
  /* 005 - unblock PIN */
  "**05?",     ksd_seqUnblckPin,

  /* 006 - unknown */
  NULL,        ksd_seqUnknown
};


/*
 * defines for CLIR, this is used for
 * separate handling of CC by SAT
 */
#define KSD_SUPPRESS_CLIR "*31#"
#define KSD_INVOKE_CLIR   "#31#"

/* standard keystroke sequence table */
LOCAL const T_KEY_SEQ_TABLE keySeqTable[] =
{
  /* 000 - activate all CF */
  "*002?",   ksd_seqActAllCf,
  /* 001 - deactivate all CF */
  "#002?",   ksd_seqDeactAllCf,
  /* 002 - registrate all CF */
  "**002?",  ksd_seqRegAllCf,
  /* 003 - erase all CF */
  "##002?",  ksd_seqEraseAllCf,
  /* 004 - interrogate all CF */
  "*#002?",  ksd_seqIntrgtAllCf,

  /* 005 - activate all conditional CF */
  "*004?",   ksd_seqActAllCondCf,
  /* 006 - deactivate all conditional CF */
  "#004?",   ksd_seqDeactAllCondCf,
  /* 007 - registrate all conditional CF */
  "**004?",  ksd_seqRegAllCondCf,
  /* 008 - erase all conditional CF */
  "##004?",  ksd_seqEraseAllCondCf,
  /* 009 - interrogate all conditional CF */
  "*#004?",  ksd_seqIntrgtAllCondCf,

  /* 010 - activate unconditional CF */
  "*21?",    ksd_seqActUncondCf,
  /* 011 - deactivate unconditional CF */
  "#21?",    ksd_seqDeactUncondCf,
  /* 012 - registrate unconditional CF */
  "**21?",   ksd_seqRegUncondCf,
  /* 013 - erase unconditional CF */
  "##21?",   ksd_seqEraseUncondCf,
  /* 014 - interrogate unconditional CF */
  "*#21?",   ksd_seqIntrgtUncondCf,

  /* 015 - activate busy CF */
  "*67?",    ksd_seqActBusyCf,
  /* 016 - deactivate busy CF */
  "#67?",    ksd_seqDeactBusyCf,
  /* 017 - registrate busy CF */
  "**67?",   ksd_seqRegBusyCf,
  /* 018 - erase busy CF */
  "##67?",   ksd_seqEraseBusyCf,
  /* 019 - interrogate busy CF */
  "*#67?",   ksd_seqIntrgtBusyCf,

  /* 020 - activate no reply CF */
  "*61?",    ksd_seqActNoRepCf,
  /* 021 - deactivate no reply CF */
  "#61?",    ksd_seqDeactNoRepCf,
  /* 022 - registrate no reply CF */
  "**61?",   ksd_seqRegNoRepCf,
  /* 023 - erase no reply CF */
  "##61?",   ksd_seqEraseNoRepCf,
  /* 024 - interrogate no reply CF */
  "*#61?",   ksd_seqIntrgtNoRepCf,

  /* 025 - activate not reachable CF */
  "*62?",    ksd_seqActNotReachCf,
  /* 026 - deactivate not reachable CF */
  "#62?",    ksd_seqDeactNotReachCf,
  /* 027 - registrate not reachable CF */
  "**62?",   ksd_seqRegNotReachCf,
  /* 028 - erase not reachable CF */
  "##62?",   ksd_seqEraseNotReachCf,
  /* 029 - interrogate not reachable CF */
  "*#62?",   ksd_seqIntrgtNotReachCf,

  /* 030 - interrogate CLIR */
  "*#31#",   ksd_seqIntrgtClir,
  /* 031 - suppress CLIR */
  KSD_SUPPRESS_CLIR, ksd_seqSupClir,
  /* 032 - invoke CLIR */
  KSD_INVOKE_CLIR,   ksd_seqInvClir,

  /* 033 - interrogate CLIP */
  "*#30#",   ksd_seqIntrgtClip,
  /* 034 - suppress CLIP */
  "*30#",    ksd_seqSupClip,
  /* 035 - invoke CLIP */
  "#30#",    ksd_seqInvClip,

  /* 036 - interrogate COLR */
  "*#77#",   ksd_seqIntrgtColr,
  /* 037 - suppress COLR */
  "*77#",    ksd_seqSupColr,
  /* 038 - invoke COLR */
  "#77#",    ksd_seqInvColr,

  /* 039 - interrogate COLP */
  "*#76#",   ksd_seqIntrgtColp,
  /* 040 - suppress COLP */
  "*76#",    ksd_seqSupColp,
  /* 041 - invoke COLP */
  "#76#",    ksd_seqInvColp,

  /* 042 - activate wait */
  "*43?",    ksd_seqActWait,
  /* 043 - deactivate wait */
  "#43?",    ksd_seqDeactWait,
  /* 044 - interrogate wait */
//TISH, patch for GCF42.3.1.9  
//start
#if 0
  "*#43#",   ksd_seqIntrgtWait,
#else
  "*#43?",   ksd_seqIntrgtWait,
#endif
//end
  /* 045 - present IMEI */
  "*#06#",   ksd_seqPrsntImei,

#if 0  /* For further study, so not yet used */
  /*     - activate CCBS */
  "*37#",    ksd_seqActCCBS
#endif

  /* 046 - deactivate CCBS */
  "#37?",    ksd_seqDeactCCBS,
  /* 047 - interrogate CCBS */
  "*#37#",   ksd_seqIntrgtCCBS,

  /* 048 - request TTY service */
  "*55#",    ksd_seqReqTTY,
  /* 049 - suppress TTY service */
  "#55#",    ksd_seqSupTTY,

  /* 300 - CNAP */
  "*#300#", ksd_seqIntrgtCNAP,

  /* 050 - unknown */
  NULL,      ksd_seqUnknown
};

/* standard keystroke sequence table */
LOCAL const T_KEY_SEQ_TABLE keySeqTableNonGsm[] =
{
  /* 000 - MS off */
  "#*91*0#", ksd_seqMsOff,
  /* 001 - MS on */
  "#*91*1#", ksd_seqMsOn,
  /* 002 - hook off */
  "#*43*0#", ksd_seqHookOff,
  /* 003 - hook on */
  "#*43*1#", ksd_seqHookOn,
  /* 004 - DTMF */
  "#*88*",   ksd_seqDtmf,
#ifdef SMI
  /* 005 - keypad indication */
  "#*92?",   ksd_seqKeypadInd,
  /* 006 - start LCD test */
  "#*96#",   ksd_seqStartLcdTest,
  /* 007 - show call table */
  "#*93#",   ksd_seqShowCallTable,
  /* 008 - set phone number used for abbreviated dialing */
  "#*95?",   ksd_seqSetAbbrDial,
#endif

  /* 009 - restart registration */
  "*1#",     ksd_seqStartReg,
  /* 010 - automatic registration */
  "*1*0#",   ksd_seqAutoReg,
  /* 011 - manual registration */
  "*1*1#",   ksd_seqManReg,
  /* 012 - manual followed by automatic registration */
  "*1*4#",   ksd_seqAutoManReg,
  /* 013 - set manual registration */
  "MANUAL",  ksd_seqSetManReg,
  /* 014 - set automatic registration */
  "AUTO",    ksd_seqSetAutoReg,
  /* 015 - start cell selection for a selected PLMN */
  "SPLMN",   ksd_seqStartPLMN,

#ifdef SMI
  /* 016 - send short message */
  "#*94*0#", ksd_seqSmsSend,
  /* 017 - send short message from storage */
  "#*94*1#", ksd_seqSmsSendFromMem,
  /* 018 - write short message to memory */
  "#*94*2#", ksd_seqSmsWrite,
  /* 019 - delete short message */
  "#*94*3#", ksd_seqSmsDelete,
  /* 020 - read short message */
  "#*94*4#", ksd_seqSmsRead,
  /* 021 - list short messages */
  "#*94*5#", ksd_seqSmsList,
  /* 022 - read a single short message */
  "#*94*6#", ksd_seqSmsReadSingle,
#endif
  /* 023 - interrogate lock status of SIM card */
  "*#77742#",ksd_seqIntrgtSimLock,
#ifdef SMI
  /* 024 - send AT+CBST, GSM 07.07, chapter 6.7 */
  "*7767?",  ksd_seqSndCbst,
  /* 025 - send AT+CRLP, GSM 07.07, chapter 6.8 */
  "*7768?",  ksd_seqSndCrlp,

  /* 026 - send AT+DS, V25ter, chapter 6.6.1 */
  "*25661?", ksd_seqSndDs,
#endif
#ifdef EM_MODE
  /* 027 - engineering mode command */
  "#*99?", ksd_em_mode,
#endif
  /* 028 - unknown */
  NULL,      ksd_seqUnknown
};

EXTERN UBYTE std;
EXTERN T_ACI_CUSCFG_PARAMS cuscfgParams;

/*==== FUNCTIONS ==================================================*/
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : KSD                      |
| STATE   : code                  ROUTINE : ksd_strlen               |
+--------------------------------------------------------------------+

  PURPOSE : This function calculates the length of a string.
            If the string ends with a '?' this character will be not
            taken into account.

            <string>: the length of this string will be calculated

            returns:  the number of characters in the string, except
                      an ending '?'
*/
LOCAL USHORT ksd_strlen (CHAR* string)
{
  USHORT len = strlen (string);

  if (string[len - 1] EQ DONT_CARE)
    len--;

  return len;
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : KSD                      |
| STATE   : code                  ROUTINE : ksd_seqMsOff             |
+--------------------------------------------------------------------+

  PURPOSE : This function sets the output parameter for the
            SEQGRP_MS_OFF key sequence.
*/
LOCAL BOOL ksd_seqMsOff (USHORT          inIdx,
                         CHAR*           inSeq,
                         T_KSD_SEQGRP*   outSeqGrp,
                         CHAR**          outRestSeq,
                         T_KSD_SEQPARAM* outSeqParam)
{
  TRACE_FUNCTION ("ksd_seqMsOff ()");

  *outSeqGrp  = SEQGRP_MS_OFF;
  *outRestSeq = inSeq + ksd_strlen (keySeqTableNonGsm[inIdx].keySeq);

  return TRUE;
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : KSD                      |
| STATE   : code                  ROUTINE : ksd_seqMsOn              |
+--------------------------------------------------------------------+

  PURPOSE : This function sets the ouput parameter for the
            SEQGRP_MS_ON key sequence.
*/
LOCAL BOOL ksd_seqMsOn (USHORT          inIdx,
                        CHAR*           inSeq,
                        T_KSD_SEQGRP*   outSeqGrp,
                        CHAR**          outRestSeq,
                        T_KSD_SEQPARAM* outSeqParam)
{
  TRACE_FUNCTION ("ksd_seqMsOn ()");

  *outSeqGrp  = SEQGRP_MS_ON;
  *outRestSeq = inSeq + ksd_strlen (keySeqTableNonGsm[inIdx].keySeq);

  return TRUE;
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : KSD                      |
| STATE   : code                  ROUTINE : ksd_seqHookOff           |
+--------------------------------------------------------------------+

  PURPOSE : This function sets the ouput parameter for the
            SEQGRP_HOOK_OFF key sequence.
*/
LOCAL BOOL ksd_seqHookOff (USHORT          inIdx,
                           CHAR*           inSeq,
                           T_KSD_SEQGRP*   outSeqGrp,
                           CHAR**          outRestSeq,
                           T_KSD_SEQPARAM* outSeqParam)
{
  TRACE_FUNCTION ("ksd_seqHookOff ()");

  *outSeqGrp  = SEQGRP_HOOK_OFF;
  *outRestSeq = inSeq + ksd_strlen (keySeqTableNonGsm[inIdx].keySeq);

  return TRUE;
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : KSD                      |
| STATE   : code                  ROUTINE : ksd_seqHookOn            |
+--------------------------------------------------------------------+

  PURPOSE : This function sets the ouput parameter for the
            SEQGRP_HOOK_ON key sequence.
*/
LOCAL BOOL ksd_seqHookOn (USHORT          inIdx,
                          CHAR*           inSeq,
                          T_KSD_SEQGRP*   outSeqGrp,
                          CHAR**          outRestSeq,
                          T_KSD_SEQPARAM* outSeqParam)
{
  TRACE_FUNCTION ("ksd_seqHookOn ()");

  *outSeqGrp  = SEQGRP_HOOK_ON;
  *outRestSeq = inSeq + ksd_strlen (keySeqTableNonGsm[inIdx].keySeq);

  return TRUE;
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : KSD                      |
| STATE   : code                  ROUTINE : ksd_seqDtmf              |
+--------------------------------------------------------------------+

  PURPOSE : This function sets the ouput parameter for the SEQGRP_DTMF
            key sequence.
*/
LOCAL BOOL ksd_seqDtmf (USHORT          inIdx,
                        CHAR*           inSeq,
                        T_KSD_SEQGRP*   outSeqGrp,
                        CHAR**          outRestSeq,
                        T_KSD_SEQPARAM* outSeqParam)
{
  CHAR* dtmf = inSeq + ksd_strlen (keySeqTableNonGsm[inIdx].keySeq);
                          /* points to the DTMF character          */
  BOOL  isSuccess;        /* holds whether decoding is successfull */

  TRACE_FUNCTION ("ksd_seqDtmf ()");

  *outSeqGrp = SEQGRP_DTMF;

  /*
   *-----------------------------------------------------------------
   * the following characters are valid DTMF characters: 0-9,A-D,*,#
   *-----------------------------------------------------------------
   */
  if ( *dtmf EQ '*'                   OR
       *dtmf EQ '#'                   OR
      (*dtmf >= 'A' AND *dtmf <= 'D') OR
      (*dtmf >= 'a' AND *dtmf <= 'd') OR
      (*dtmf >= '0' AND *dtmf <= '9'))
  {
    *outRestSeq       = dtmf + 1;
    outSeqParam->dtmf = *dtmf;
    isSuccess         = TRUE;
  }
  else
  {
    *outRestSeq = dtmf;
    isSuccess   = FALSE;
  }

  return isSuccess;
}

#ifdef SMI
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : KSD                      |
| STATE   : code                  ROUTINE : ksd_seqKeypadInd         |
+--------------------------------------------------------------------+

  PURPOSE : This function sets the ouput parameter for the
            SEQGRP_KEYPAD_IND key sequence.
*/
LOCAL BOOL ksd_seqKeypadInd (USHORT          inIdx,
                             CHAR*           inSeq,
                             T_KSD_SEQGRP*   outSeqGrp,
                             CHAR**          outRestSeq,
                             T_KSD_SEQPARAM* outSeqParam)
{
  static const struct T_KSD_KEYS /* key name / key code mapping */
  {
    const CHAR* longName;        /* long key name               */
    const UBYTE code;            /* key code                    */
  }
    keys[] =
  {
    "SEND",  KP_OFF_HOOK,
    "END",   KP_ON_HOOK,
    "UP",    KP_UP,
    "DOWN",  KP_DOWN,
    "SHIFT", KP_SHIFT,
    "CLEAR", KP_CLEAR,
    "HASH",  KP_HASH,
    "STAR",  KP_STAR,
    "POWER", KP_POWER,
    NULL,    -1
  };

  USHORT         i         = 0;
  const USHORT   siNum     = 1;
  T_KSD_DCD_CPLE dcdCple[1];
  T_KSD_SIPARAM  keyParam;
  USHORT         readChars;
  USHORT         lenKeySeq = ksd_strlen (keySeqTableNonGsm[inIdx].keySeq);
  BOOL           isSuccess = FALSE;

  TRACE_FUNCTION ("ksd_seqKeypadInd ()");

  dcdCple[0].type  = SI_CHAR_STAR;
  dcdCple[0].param = &keyParam;

  *outSeqGrp = SEQGRP_KEYPAD_IND;

  if (ksd_extractSi (inSeq + lenKeySeq, siNum,
                     &readChars, dcdCple)      EQ TRUE)
  {
    CHAR* key = keyParam.siCharStar;

    outSeqParam->keypad.keyStat = KEY_STAT_PRS;

    /*
     *---------------------------------------------------------------
     * searching for long key name
     *---------------------------------------------------------------
     */
    while (keys[i].longName NEQ NULL AND
           strcmp (keys[i].longName, key) NEQ 0)
      i++;

    if (keys[i].longName NEQ NULL)
    {
      /*
       *-------------------------------------------------------------
       * long key name found
       *-------------------------------------------------------------
       */
      outSeqParam->keypad.keyCode = keys[i].code;
      isSuccess = TRUE;
    }
    else if (strlen (key) EQ 1)
    {
      /*
       *-------------------------------------------------------------
       * only a single character found
       *-------------------------------------------------------------
       */
      outSeqParam->keypad.keyCode = *key;
      isSuccess = TRUE;
    }
    else
      isSuccess = FALSE;
  }

  *outRestSeq = inSeq + lenKeySeq + readChars;

  return isSuccess;
}
#endif

#ifdef SMI
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : KSD                      |
| STATE   : code                  ROUTINE : ksd_seqStartLcdTest      |
+--------------------------------------------------------------------+

  PURPOSE : This function sets the ouput parameter for the
            SEQGRP_LCD_TEST key sequence.
*/
LOCAL BOOL ksd_seqStartLcdTest (USHORT          inIdx,
                                CHAR*           inSeq,
                                T_KSD_SEQGRP*   outSeqGrp,
                                CHAR**          outRestSeq,
                                T_KSD_SEQPARAM* outSeqParam)
{
  TRACE_FUNCTION ("ksd_seqStartLcdTest ()");

  *outSeqGrp  = SEQGRP_LCD_TEST;
  *outRestSeq = inSeq + ksd_strlen (keySeqTableNonGsm[inIdx].keySeq);

  return TRUE;
}
#endif

#ifdef SMI
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : KSD                      |
| STATE   : code                  ROUTINE : ksd_seqShowCallTable     |
+--------------------------------------------------------------------+

  PURPOSE : This function sets the ouput parameter for the
            SEQGRP_SHOW_CALL_TABLE key sequence.
*/
LOCAL BOOL ksd_seqShowCallTable (USHORT          inIdx,
                                 CHAR*           inSeq,
                                 T_KSD_SEQGRP*   outSeqGrp,
                                 CHAR**          outRestSeq,
                                 T_KSD_SEQPARAM* outSeqParam)
{
  TRACE_FUNCTION ("ksd_seqShowCallTable ()");

  *outSeqGrp  = SEQGRP_SHOW_CALL_TABLE;
  *outRestSeq = inSeq + ksd_strlen (keySeqTableNonGsm[inIdx].keySeq);

  return TRUE;
}
#endif

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : KSD                      |
| STATE   : code                  ROUTINE : ksd_seqAutoReg           |
+--------------------------------------------------------------------+

  PURPOSE : This function sets the ouput parameter for the
            SEQGRP_AUTO_REG key sequence.
*/
LOCAL BOOL ksd_seqAutoReg (USHORT          inIdx,
                           CHAR*           inSeq,
                           T_KSD_SEQGRP*   outSeqGrp,
                           CHAR**          outRestSeq,
                           T_KSD_SEQPARAM* outSeqParam)
{
  TRACE_FUNCTION ("ksd_seqAutoReg ()");

  *outSeqGrp  = SEQGRP_CHANGE_REGISTER;
  *outRestSeq = inSeq + ksd_strlen (keySeqTableNonGsm[inIdx].keySeq);

  outSeqParam->cops.mode = COPS_MOD_Auto;
  outSeqParam->cops.frmt = COPS_FRMT_NotPresent;
  outSeqParam->cops.oper = NULL;

  return TRUE;
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : KSD                      |
| STATE   : code                  ROUTINE : ksd_seqManReg            |
+--------------------------------------------------------------------+

  PURPOSE : This function sets the ouput parameter for the
            SEQGRP_MAN_REG key sequence.
*/
LOCAL BOOL ksd_seqManReg (USHORT          inIdx,
                          CHAR*           inSeq,
                          T_KSD_SEQGRP*   outSeqGrp,
                          CHAR**          outRestSeq,
                          T_KSD_SEQPARAM* outSeqParam)
{
  TRACE_FUNCTION ("ksd_seqManReg ()");

  *outSeqGrp  = SEQGRP_CHANGE_REGISTER;
  *outRestSeq = inSeq + ksd_strlen (keySeqTableNonGsm[inIdx].keySeq);

  outSeqParam->cops.mode = COPS_MOD_Man;
  outSeqParam->cops.frmt = COPS_FRMT_NotPresent;
  outSeqParam->cops.oper = NULL;

  return TRUE;
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : KSD                      |
| STATE   : code                  ROUTINE : ksd_seqAutoManReg        |
+--------------------------------------------------------------------+

  PURPOSE : This function sets the ouput parameter for the
            AUTO_MAN_REG key sequence.
*/
LOCAL BOOL ksd_seqAutoManReg (USHORT          inIdx,
                              CHAR*           inSeq,
                              T_KSD_SEQGRP*   outSeqGrp,
                              CHAR**          outRestSeq,
                              T_KSD_SEQPARAM* outSeqParam)
{
  TRACE_FUNCTION ("ksd_seqAutoManReg ()");

  *outSeqGrp  = SEQGRP_CHANGE_REGISTER;
  *outRestSeq = inSeq + ksd_strlen (keySeqTableNonGsm[inIdx].keySeq);

  outSeqParam->cops.mode = COPS_MOD_Both;
  outSeqParam->cops.frmt = COPS_FRMT_NotPresent;
  outSeqParam->cops.oper = NULL;

  return TRUE;
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : KSD                      |
| STATE   : code                  ROUTINE : ksd_seqSetManReg         |
+--------------------------------------------------------------------+

  PURPOSE : This function sets the ouput parameter for the
            SEQGRP_SET_MAN_REG key sequence.
*/
LOCAL BOOL ksd_seqSetManReg (USHORT          inIdx,
                             CHAR*           inSeq,
                             T_KSD_SEQGRP*   outSeqGrp,
                             CHAR**          outRestSeq,
                             T_KSD_SEQPARAM* outSeqParam)
{
  TRACE_FUNCTION ("ksd_seqSetManReg ()");

  *outSeqGrp  = SEQGRP_SET_REGISTER;
  *outRestSeq = inSeq + ksd_strlen (keySeqTableNonGsm[inIdx].keySeq);

  outSeqParam->cops.mode = COPS_MOD_Man;
  outSeqParam->cops.frmt = COPS_FRMT_NotPresent;
  outSeqParam->cops.oper = NULL;

  return TRUE;
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : KSD                      |
| STATE   : code                  ROUTINE : ksd_seqSetAutoReg        |
+--------------------------------------------------------------------+

  PURPOSE : This function sets the ouput parameter for the
            SEQGRP_SET_AUTO_REG key sequence.
*/
LOCAL BOOL ksd_seqSetAutoReg (USHORT          inIdx,
                              CHAR*           inSeq,
                              T_KSD_SEQGRP*   outSeqGrp,
                              CHAR**          outRestSeq,
                              T_KSD_SEQPARAM* outSeqParam)
{
  TRACE_FUNCTION ("ksd_seqSetAutoReg ()");

  *outSeqGrp  = SEQGRP_SET_REGISTER;
  *outRestSeq = inSeq + ksd_strlen (keySeqTableNonGsm[inIdx].keySeq);

  outSeqParam->cops.mode = COPS_MOD_Auto;
  outSeqParam->cops.frmt = COPS_FRMT_NotPresent;
  outSeqParam->cops.oper = NULL;

  return TRUE;
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : KSD                      |
| STATE   : code                  ROUTINE : ksd_seqStartReg          |
+--------------------------------------------------------------------+

  PURPOSE : This function sets the ouput parameter for the
            SEQGRP_START_REGISTER key sequence.
*/
LOCAL BOOL ksd_seqStartReg (USHORT          inIdx,
                            CHAR*           inSeq,
                            T_KSD_SEQGRP*   outSeqGrp,
                            CHAR**          outRestSeq,
                            T_KSD_SEQPARAM* outSeqParam)
{
  TRACE_FUNCTION ("ksd_seqStartReg ()");

  *outSeqGrp  = SEQGRP_START_REGISTER;
  *outRestSeq = inSeq + ksd_strlen (keySeqTableNonGsm[inIdx].keySeq);

  outSeqParam->cops.mode = COPS_MOD_NotPresent;
  outSeqParam->cops.frmt = COPS_FRMT_NotPresent;
  outSeqParam->cops.oper = NULL;

  return TRUE;
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : KSD                      |
| STATE   : code                  ROUTINE : ksd_seqStartPLMN         |
+--------------------------------------------------------------------+

  PURPOSE : This function sets the ouput parameter for the
            SEQGRP_START_PLMN key sequence.
*/
LOCAL BOOL ksd_seqStartPLMN (USHORT          inIdx,
                             CHAR*           inSeq,
                             T_KSD_SEQGRP*   outSeqGrp,
                             CHAR**          outRestSeq,
                             T_KSD_SEQPARAM* outSeqParam)
{
  TRACE_FUNCTION ("ksd_seqStartPLMN ()");

  *outSeqGrp  = SEQGRP_START_REGISTER;
  *outRestSeq = inSeq + strlen (inSeq);

  outSeqParam->cops.mode = COPS_MOD_NotPresent;
  outSeqParam->cops.frmt = COPS_FRMT_Numeric;
  outSeqParam->cops.oper = inSeq +
                           ksd_strlen (keySeqTableNonGsm[inIdx].keySeq);

  return TRUE;
}

#ifdef SMI
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : KSD                      |
| STATE   : code                  ROUTINE : ksd_seqSmsSend           |
+--------------------------------------------------------------------+

  PURPOSE : This function sets the ouput parameter for the
            SEQGRP_SMS_SEND key sequence.
*/
LOCAL BOOL ksd_seqSmsSend (USHORT          inIdx,
                           CHAR*           inSeq,
                           T_KSD_SEQGRP*   outSeqGrp,
                           CHAR**          outRestSeq,
                           T_KSD_SEQPARAM* outSeqParam)
{
  TRACE_FUNCTION ("ksd_seqSmsSend ()");

  *outSeqGrp  = SEQGRP_SMS_SEND;
  *outRestSeq = inSeq + ksd_strlen (keySeqTableNonGsm[inIdx].keySeq);

  return TRUE;
}
#endif

#ifdef SMI
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : KSD                      |
| STATE   : code                  ROUTINE : ksd_seqSmsSendFromMem    |
+--------------------------------------------------------------------+

  PURPOSE : This function sets the ouput parameter for the
            SEQGRP_SMS_SEND_FROM_MEM key sequence.
*/
LOCAL BOOL ksd_seqSmsSendFromMem (USHORT          inIdx,
                                  CHAR*           inSeq,
                                  T_KSD_SEQGRP*   outSeqGrp,
                                  CHAR**          outRestSeq,
                                  T_KSD_SEQPARAM* outSeqParam)
{
  TRACE_FUNCTION ("ksd_seqSmsSendFromMem ()");

  *outSeqGrp  = SEQGRP_SMS_SEND_FROM_MEM;
  *outRestSeq = inSeq + ksd_strlen (keySeqTableNonGsm[inIdx].keySeq);

  return TRUE;
}
#endif

#ifdef SMI
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : KSD                      |
| STATE   : code                  ROUTINE : ksd_seqSmsWrite          |
+--------------------------------------------------------------------+

  PURPOSE : This function sets the ouput parameter for the
            SEQGRP_SMS_WRITE key sequence.
*/
LOCAL BOOL ksd_seqSmsWrite (USHORT          inIdx,
                            CHAR*           inSeq,
                            T_KSD_SEQGRP*   outSeqGrp,
                            CHAR**          outRestSeq,
                            T_KSD_SEQPARAM* outSeqParam)
{
  TRACE_FUNCTION ("ksd_seqSmsWrite ()");

  *outSeqGrp  = SEQGRP_SMS_WRITE;
  *outRestSeq = inSeq + ksd_strlen (keySeqTableNonGsm[inIdx].keySeq);

  return TRUE;
}
#endif

#ifdef EM_MODE
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : KSD                      |
| STATE   : code                  ROUTINE : ksd_em_mode              |
+--------------------------------------------------------------------+

  PURPOSE : This function sets the ouput parameter for the
            SEQGRP_EM_MODE key sequence.
*/
LOCAL BOOL ksd_em_mode     (USHORT          inIdx,
                            CHAR*           inSeq,
                            T_KSD_SEQGRP*   outSeqGrp,
                            CHAR**          outRestSeq,
                            T_KSD_SEQPARAM* outSeqParam)
{
  TRACE_FUNCTION ("ksd_em_mode ()");

  *outSeqGrp  = SEQGRP_EM_MODE;
  *outRestSeq = inSeq + ksd_strlen (keySeqTableNonGsm[inIdx].keySeq);

  return TRUE;
}
#endif

#ifdef SMI
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : KSD                      |
| STATE   : code                  ROUTINE : ksd_seqSmsDelete         |
+--------------------------------------------------------------------+

  PURPOSE : This function sets the ouput parameter for the
            SEQGRP_SMS_DELETE key sequence.
*/
LOCAL BOOL ksd_seqSmsDelete (USHORT          inIdx,
                             CHAR*           inSeq,
                             T_KSD_SEQGRP*   outSeqGrp,
                             CHAR**          outRestSeq,
                             T_KSD_SEQPARAM* outSeqParam)
{
  TRACE_FUNCTION ("ksd_seqSmsDelete ()");

  *outSeqGrp  = SEQGRP_SMS_DELETE;
  *outRestSeq = inSeq + ksd_strlen (keySeqTableNonGsm[inIdx].keySeq);

  return TRUE;
}
#endif

#ifdef SMI
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : KSD                      |
| STATE   : code                  ROUTINE : ksd_seqSmsRead           |
+--------------------------------------------------------------------+

  PURPOSE : This function sets the ouput parameter for the
            SEQGRP_SMS_READ key sequence.
*/
LOCAL BOOL ksd_seqSmsRead (USHORT          inIdx,
                           CHAR*           inSeq,
                           T_KSD_SEQGRP*   outSeqGrp,
                           CHAR**          outRestSeq,
                           T_KSD_SEQPARAM* outSeqParam)
{
  TRACE_FUNCTION ("ksd_seqSmsRead ()");

  *outSeqGrp  = SEQGRP_SMS_READ;
  *outRestSeq = inSeq + ksd_strlen (keySeqTableNonGsm[inIdx].keySeq);

  return TRUE;
}
#endif

#ifdef SMI
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : KSD                      |
| STATE   : code                  ROUTINE : ksd_seqSmsList           |
+--------------------------------------------------------------------+

  PURPOSE : This function sets the ouput parameter for the
            SEQGRP_SMS_LIST key sequence.
*/
LOCAL BOOL ksd_seqSmsList (USHORT          inIdx,
                           CHAR*           inSeq,
                           T_KSD_SEQGRP*   outSeqGrp,
                           CHAR**          outRestSeq,
                           T_KSD_SEQPARAM* outSeqParam)
{
  TRACE_FUNCTION ("ksd_seqSmsList ()");

  *outSeqGrp  = SEQGRP_SMS_LIST;
  *outRestSeq = inSeq + ksd_strlen (keySeqTableNonGsm[inIdx].keySeq);

  return TRUE;
}
#endif

#ifdef SMI
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : KSD                      |
| STATE   : code                  ROUTINE : ksd_seqSmsReadSingle     |
+--------------------------------------------------------------------+

  PURPOSE : This function sets the ouput parameter for the
            SEQGRP_SMS_READ_SINGLE key sequence.
*/
LOCAL BOOL ksd_seqSmsReadSingle (USHORT          inIdx,
                                 CHAR*           inSeq,
                                 T_KSD_SEQGRP*   outSeqGrp,
                                 CHAR**          outRestSeq,
                                 T_KSD_SEQPARAM* outSeqParam)
{
  TRACE_FUNCTION ("ksd_seqSmsReadSingle ()");

  *outSeqGrp  = SEQGRP_SMS_READ_SINGLE;
  *outRestSeq = inSeq + ksd_strlen (keySeqTableNonGsm[inIdx].keySeq);

  return TRUE;
}
#endif

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : KSD                      |
| STATE   : code                  ROUTINE : ksd_seqDial              |
+--------------------------------------------------------------------+

  PURPOSE : This function sets the ouput parameter for the SEQGRP_DIAL
            key sequence.
*/
LOCAL BOOL ksd_seqDial (USHORT          inIdx,
                        CHAR*           inSeq,
                        T_KSD_SEQGRP*   outSeqGrp,
                        CHAR**          outRestSeq,
                        T_KSD_SEQPARAM* outSeqParam)
{
  USHORT lenWholeNumber = strlen (inSeq);
  USHORT lenDialNumber  = lenWholeNumber;

  TRACE_FUNCTION ("ksd_seqDial ()");

  /*
   *-----------------------------------------------------------------
   * process type of call
   *-----------------------------------------------------------------
   */
  if (inSeq[lenDialNumber - 1] EQ DATA_CALL)
  {
    outSeqParam->dial.callType = D_TOC_Data;
    lenDialNumber--;
  }
  else
    outSeqParam->dial.callType = D_TOC_Voice;

  outSeqParam->dial.cugCtrl  = D_CUG_CTRL_NotPresent;
  outSeqParam->dial.clirOvrd = D_CLIR_OVRD_Default;
  outSeqParam->dial.number   = inSeq;
  inSeq[lenDialNumber]       = NULL_TERM;

  *outSeqGrp  = SEQGRP_DIAL;
  *outRestSeq = inSeq + lenWholeNumber;

  return TRUE;
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : KSD                      |
| STATE   : code                  ROUTINE : ksd_seqDialIdx           |
+--------------------------------------------------------------------+

  PURPOSE : This function sets the ouput parameter for the
            SEQGRP_DIAL_IDX key sequence.
*/
LOCAL BOOL ksd_seqDialIdx (USHORT          inIdx,
                           CHAR*           inSeq,
                           T_KSD_SEQGRP*   outSeqGrp,
                           CHAR**          outRestSeq,
                           T_KSD_SEQPARAM* outSeqParam)
{
  BOOL   isSuccess = FALSE;
  UBYTE  i         = 0;
  SHORT index      = 0;

  TRACE_FUNCTION ("ksd_seqDialIdx ()");

  *outSeqGrp  = SEQGRP_UNKNOWN;
  *outRestSeq = inSeq;

  /* when operating in a US band (1900 or 850), "0" and "00" shall 
   * be transmitted to the network as normal dialing numbers
   */
  if ( ksd_isBCDForUSBand(inSeq) )
  {
    return FALSE;
  }

  while ( i < strlen ( inSeq ) AND inSeq[i] NEQ DIAL_IDX )
  {
    if ( inSeq[i] >= '0' AND inSeq[i] <= '9' )
    {
      index *= 10;
      index += inSeq[i] - '0';
    }
    else
    {
      return isSuccess;
    }

    i++;
  }

  if ( i > 0                   AND
       i < 4                   AND
       inSeq[i+1] EQ NULL_TERM AND
       inSeq[i]   EQ DIAL_IDX      )
  {
    *outSeqGrp  = SEQGRP_DIAL_IDX;
    *outRestSeq = &inSeq[i+1];

    outSeqParam->dialIdx.str      = NULL;
    outSeqParam->dialIdx.mem      = PB_STOR_NotPresent;
    outSeqParam->dialIdx.index    = index;
    outSeqParam->dialIdx.clirOvrd = D_CLIR_OVRD_Default;
    outSeqParam->dialIdx.cugCtrl  = D_CUG_CTRL_NotPresent;
    outSeqParam->dialIdx.callType = D_TOC_Voice;

    isSuccess = TRUE;
  }

  return isSuccess;
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : KSD                      |
| STATE   : code                  ROUTINE : ksd_seqUssd              |
+--------------------------------------------------------------------+

  PURPOSE : This function sets the ouput parameter for the
            SEQGRP_USSD key sequence.
*/
LOCAL BOOL ksd_seqUssd (USHORT          inIdx,
                        CHAR*           inSeq,
                        T_KSD_SEQGRP*   outSeqGrp,
                        CHAR**          outRestSeq,
                        T_KSD_SEQPARAM* outSeqParam)
{
  TRACE_FUNCTION ("ksd_seqUssd ()");

  outSeqParam->ussd.ussd = (UBYTE *)inSeq;

  *outSeqGrp  = SEQGRP_USSD;
  *outRestSeq = inSeq + strlen (inSeq);

  return TRUE;
}

#ifdef SMI
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : KSD                      |
| STATE   : code                  ROUTINE : ksd_seqSetAbbrDial       |
+--------------------------------------------------------------------+

  PURPOSE : This function sets the ouput parameter for the
            SEQGRP_SET_ABBR_DIAL key sequence.
*/
LOCAL BOOL ksd_seqSetAbbrDial (USHORT          inIdx,
                               CHAR*           inSeq,
                               T_KSD_SEQGRP*   outSeqGrp,
                               CHAR**          outRestSeq,
                               T_KSD_SEQPARAM* outSeqParam)
{
  const USHORT   siNum     = 2;
  T_KSD_DCD_CPLE dcdCple[2];
  T_KSD_SIPARAM  idxParam;
  T_KSD_SIPARAM  dnParam;
  USHORT         readChars;
  USHORT         lenKeySeq = ksd_strlen (keySeqTableNonGsm[inIdx].keySeq);
  BOOL           isSuccess = FALSE;

  TRACE_FUNCTION ("ksd_seqSetAbbrDial ()");

  dcdCple[0].type  = SI_BYTE;
  dcdCple[0].param = &idxParam;

  dcdCple[1].type  = SI_CHAR_STAR;
  dcdCple[1].param = &dnParam;

  *outSeqGrp  = SEQGRP_SET_ABBR_DIAL;

  if (ksd_extractSi (inSeq + lenKeySeq, siNum,
                     &readChars, dcdCple)      EQ TRUE)
  {
    outSeqParam->abbrDial.index  = idxParam.siByte;
    outSeqParam->abbrDial.number = dnParam.siCharStar;

    isSuccess = TRUE;
  }

  *outRestSeq = inSeq + lenKeySeq + readChars;

  return isSuccess;
}
#endif

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : KSD                      |
| STATE   : code                  ROUTINE : ksd_getCcfcFromDnBsT     |
+--------------------------------------------------------------------+

  PURPOSE : This function extracts the parameters of the sAT_PlusCCFC
            function of the ACI out of the supplementary information.

            <inIdx>:       index of key sequence table
            <inSeq>:       key sequence, to be decoded
            <outRestSeq>:  rest key sequence, to be decoded by a
                           further call to this function
            <outSeqParam>: sequence parameter

            returns:       TRUE if output parameters are valid,
                           otherwise FALSE
*/
LOCAL BOOL ksd_getCcfcFromDnBsT (USHORT          inIdx,
                                 CHAR*           inSeq,
                                 CHAR**          outRestSeq,
                                 T_KSD_SEQPARAM* outSeqParam)
{
  const USHORT   siNum     = 3;
  T_KSD_DCD_CPLE dcdCple[3];
  T_KSD_SIPARAM  dnParam;
  T_KSD_SIPARAM  bsParam;
  T_KSD_SIPARAM  tParam;
  USHORT         readChars;
  USHORT         lenKeySeq = ksd_strlen (keySeqTable[inIdx].keySeq);
  USHORT         lenSubaddr;
  BOOL           international;
  BOOL           isSuccess = FALSE;

  TRACE_FUNCTION ("ksd_getCcfcFromDnBsT ()");

  dcdCple[0].type    = SI_CHAR_STAR;
  dcdCple[0].param   = &dnParam;
  dnParam.siCharStar = NULL;

  dcdCple[1].type    = SI_UBYTE;
  dcdCple[1].param   = &bsParam;
  bsParam.siUbyte    = KSD_BS_None;

  dcdCple[2].type    = SI_UBYTE;
  dcdCple[2].param   = &tParam;
  tParam.siUbyte     = KSD_TIME_NONE;

  if (ksd_extractSi (inSeq + lenKeySeq, siNum,
                     &readChars, dcdCple)      EQ TRUE)
  {
    CHAR* dn = dnParam.siCharStar;
    UBYTE bs = bsParam.siUbyte;
    UBYTE t  = tParam.siUbyte;

    /*
     *---------------------------------------------------------------
     * process dial number
     *---------------------------------------------------------------
     */
     utl_splitDialnumber ( dn,
                          (CHAR**)&outSeqParam->cf.num,
                           &international,
                          (CHAR**)&outSeqParam->cf.sub,
                           &lenSubaddr);

    /*
     *---------------------------------------------------------------
     * process main address and type of main address
     *---------------------------------------------------------------
     */
    if (outSeqParam->cf.num EQ NULL)
    {
      outSeqParam->cf.ton = KSD_TON_Unknown;
      outSeqParam->cf.npi = KSD_NPI_Unknown;
    }
    else
    {
      outSeqParam->cf.ton = (international ? KSD_TON_International :
                                             KSD_TON_Unknown);
      outSeqParam->cf.npi = KSD_NPI_IsdnTelephony;
    }

    /*
     *---------------------------------------------------------------
     * process subaddress and type of subaddress
     *---------------------------------------------------------------
     */
    if (outSeqParam->cf.sub EQ NULL)
    {
      outSeqParam->cf.tos = KSD_TOS_Nsap;
      outSeqParam->cf.oe  = KSD_OE_Even;
    }
    else
    {
      outSeqParam->cf.tos = KSD_TOS_Nsap;
      outSeqParam->cf.oe  = ((lenSubaddr % 2 EQ 0) ? KSD_OE_Even :
                                                     KSD_OE_Odd);
    }

    /*
     *---------------------------------------------------------------
     * process class and time
     *---------------------------------------------------------------
     */
    outSeqParam->cf.bsCd = bs;
    outSeqParam->cf.time = t;

    isSuccess = TRUE;
  }

  *outRestSeq = inSeq + lenKeySeq + readChars;

  return isSuccess;
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : KSD                      |
| STATE   : code                  ROUTINE : ksd_getCcfcFromDnBs      |
+--------------------------------------------------------------------+

  PURPOSE : This function extracts the parameters of the sAT_PlusCCFC
            function of the ACI out of the supplementary information.

            <inIdx>:       index of key sequence table
            <inSeq>:       key sequence, to be decoded
            <outRestSeq>:  rest key sequence, to be decoded by a
                           further call to this function
            <outSeqParam>: sequence parameter

            returns:       TRUE if output parameters are valid,
                           otherwise FALSE
*/
LOCAL BOOL ksd_getCcfcFromDnBs (USHORT          inIdx,
                                CHAR*           inSeq,
                                CHAR**          outRestSeq,
                                T_KSD_SEQPARAM* outSeqParam)
{
  const USHORT   siNum     = 2;
  T_KSD_DCD_CPLE dcdCple[2];
  T_KSD_SIPARAM  dnParam;
  T_KSD_SIPARAM  bsParam;
  USHORT         readChars;
  USHORT         lenKeySeq = ksd_strlen (keySeqTable[inIdx].keySeq);
  USHORT         lenSubaddr;
  BOOL           international;
  BOOL           isSuccess = FALSE;

  TRACE_FUNCTION ("ksd_getCcfcFromDnBs ()");

  dcdCple[0].type    = SI_CHAR_STAR;
  dcdCple[0].param   = &dnParam;
  dnParam.siCharStar = NULL;

  dcdCple[1].type    = SI_UBYTE;
  dcdCple[1].param   = &bsParam;
  bsParam.siUbyte    = KSD_BS_None;

  if (ksd_extractSi (inSeq + lenKeySeq, siNum,
                     &readChars, dcdCple)      EQ TRUE)
  {
    CHAR* dn = dnParam.siCharStar;
    UBYTE bs = bsParam.siUbyte;

    /*
     *---------------------------------------------------------------
     * process dial number
     *---------------------------------------------------------------
     */
    utl_splitDialnumber ( dn,
                         (CHAR**)&outSeqParam->cf.num,
                          &international,
                         (CHAR**)&outSeqParam->cf.sub,
                          &lenSubaddr);

    /*
     *---------------------------------------------------------------
     * process main address and type of main address
     *---------------------------------------------------------------
     */
    if (outSeqParam->cf.num EQ NULL)
    {
      outSeqParam->cf.ton = KSD_TON_Unknown;
      outSeqParam->cf.npi = KSD_NPI_Unknown;
    }
    else
    {
      outSeqParam->cf.ton = (international ? KSD_TON_International :
                                             KSD_TON_Unknown);
      outSeqParam->cf.npi = KSD_NPI_IsdnTelephony;
    }

    /*
     *---------------------------------------------------------------
     * process subaddress and type of subaddress
     *---------------------------------------------------------------
     */
    if (outSeqParam->cf.sub EQ NULL)
    {
      outSeqParam->cf.tos = KSD_TOS_Nsap;
      outSeqParam->cf.oe  = KSD_OE_Even;
    }
    else
    {
      outSeqParam->cf.tos = KSD_TOS_Nsap;
      outSeqParam->cf.oe  = ((lenSubaddr % 2 EQ 0) ? KSD_OE_Even :
                                                     KSD_OE_Odd);
    }

    /*
     *---------------------------------------------------------------
     * process class and time
     *---------------------------------------------------------------
     */
    outSeqParam->cf.bsCd = bs;
    outSeqParam->cf.time = KSD_TIME_NONE;

    isSuccess = TRUE;
  }

  *outRestSeq = inSeq + lenKeySeq + readChars;

  return isSuccess;
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : KSD                      |
| STATE   : code                  ROUTINE : ksd_seqActAllCf          |
+--------------------------------------------------------------------+

  PURPOSE : This function sets the ouput parameter for the
            SEQGRP_CF, KSD_OP_ACT, KSD_SS_ALL_FWSS key sequence.
*/
LOCAL BOOL ksd_seqActAllCf (USHORT          inIdx,
                            CHAR*           inSeq,
                            T_KSD_SEQGRP*   outSeqGrp,
                            CHAR**          outRestSeq,
                            T_KSD_SEQPARAM* outSeqParam)
{
  BOOL result;

  TRACE_FUNCTION ("ksd_seqActAllCf ()");

  result = ksd_getCcfcFromDnBsT (inIdx, inSeq, outRestSeq, outSeqParam);

  if ( outSeqParam->cf.num EQ NULL )
    outSeqParam->cf.opCd = KSD_OP_ACT;
  else
    outSeqParam->cf.opCd = KSD_OP_REG;

  outSeqParam->cf.ssCd = KSD_SS_ALL_FWSS;

  *outSeqGrp  = SEQGRP_CF;

  return result;
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : KSD                      |
| STATE   : code                  ROUTINE : ksd_seqDeactAllCf        |
+--------------------------------------------------------------------+

  PURPOSE : This function sets the ouput parameter for the
            SEQGRP_CF, KSD_OP_DEACT, KSD_SS_ALL_FWSS key sequence.
*/
LOCAL BOOL ksd_seqDeactAllCf (USHORT          inIdx,
                              CHAR*           inSeq,
                              T_KSD_SEQGRP*   outSeqGrp,
                              CHAR**          outRestSeq,
                              T_KSD_SEQPARAM* outSeqParam)
{

  TRACE_FUNCTION ("ksd_seqDeactAllCf ()");

  outSeqParam->cf.opCd = KSD_OP_DEACT;
  outSeqParam->cf.ssCd = KSD_SS_ALL_FWSS;

  *outSeqGrp  = SEQGRP_CF;

  return ksd_getCcfcFromDnBsT (inIdx, inSeq, outRestSeq, outSeqParam);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : KSD                      |
| STATE   : code                  ROUTINE : ksd_seqRegAllCf          |
+--------------------------------------------------------------------+

  PURPOSE : This function sets the ouput parameter for the
            SEQGRP_CF, KSD_OP_REG, KSD_SS_ALL_FWSS key sequence.
*/
LOCAL BOOL ksd_seqRegAllCf (USHORT          inIdx,
                            CHAR*           inSeq,
                            T_KSD_SEQGRP*   outSeqGrp,
                            CHAR**          outRestSeq,
                            T_KSD_SEQPARAM* outSeqParam)
{
  TRACE_FUNCTION ("ksd_seqRegAllCf ()");

  outSeqParam->cf.opCd = KSD_OP_REG;
  outSeqParam->cf.ssCd = KSD_SS_ALL_FWSS;

  *outSeqGrp  = SEQGRP_CF;

  return ksd_getCcfcFromDnBsT (inIdx, inSeq, outRestSeq, outSeqParam);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : KSD                      |
| STATE   : code                  ROUTINE : ksd_seqEraseAllCf        |
+--------------------------------------------------------------------+

  PURPOSE : This function sets the ouput parameter for the
            SEQGRP_CF, KSD_OP_ERS, KSD_SS_ALL_FWSS key sequence.
*/
LOCAL BOOL ksd_seqEraseAllCf (USHORT          inIdx,
                              CHAR*           inSeq,
                              T_KSD_SEQGRP*   outSeqGrp,
                              CHAR**          outRestSeq,
                              T_KSD_SEQPARAM* outSeqParam)
{
  TRACE_FUNCTION ("ksd_seqEraseAllCf ()");

  outSeqParam->cf.opCd = KSD_OP_ERS;
  outSeqParam->cf.ssCd = KSD_SS_ALL_FWSS;

  *outSeqGrp  = SEQGRP_CF;

  return ksd_getCcfcFromDnBsT (inIdx, inSeq, outRestSeq, outSeqParam);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : KSD                      |
| STATE   : code                  ROUTINE : ksd_seqIntrgtAllCf       |
+--------------------------------------------------------------------+

  PURPOSE : This function sets the ouput parameter for the
            SEQGRP_CF, KSD_OP_IRGT, KSD_SS_ALL_FWSS key sequence.
*/
LOCAL BOOL ksd_seqIntrgtAllCf (USHORT          inIdx,
                               CHAR*           inSeq,
                               T_KSD_SEQGRP*   outSeqGrp,
                               CHAR**          outRestSeq,
                               T_KSD_SEQPARAM* outSeqParam)
{
  TRACE_FUNCTION ("ksd_seqIntrgtAllCf ()");

  outSeqParam->cf.opCd = KSD_OP_IRGT;
  outSeqParam->cf.ssCd = KSD_SS_ALL_FWSS;

  *outSeqGrp  = SEQGRP_CF;

  return ksd_getCcfcFromDnBsT (inIdx, inSeq, outRestSeq, outSeqParam);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : KSD                      |
| STATE   : code                  ROUTINE : ksd_seqActAllCondCf      |
+--------------------------------------------------------------------+

  PURPOSE : This function sets the ouput parameter for the
            SEQGRP_CF, KSD_OP_ACT, KSD_SS_ALL_CFWSS key sequence.
*/
LOCAL BOOL ksd_seqActAllCondCf (USHORT          inIdx,
                                CHAR*           inSeq,
                                T_KSD_SEQGRP*   outSeqGrp,
                                CHAR**          outRestSeq,
                                T_KSD_SEQPARAM* outSeqParam)
{
  BOOL result;

  TRACE_FUNCTION ("ksd_seqActAllCondCf ()");

  result = ksd_getCcfcFromDnBsT (inIdx, inSeq, outRestSeq, outSeqParam);

  if ( outSeqParam->cf.num EQ NULL )
    outSeqParam->cf.opCd = KSD_OP_ACT;
  else
    outSeqParam->cf.opCd = KSD_OP_REG;

  outSeqParam->cf.ssCd = KSD_SS_ALL_CFWSS;

  *outSeqGrp  = SEQGRP_CF;

  return result;
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : KSD                      |
| STATE   : code                  ROUTINE : ksd_seqDeactAllCondCf    |
+--------------------------------------------------------------------+

  PURPOSE : This function sets the ouput parameter for the
            SEQGRP_CF, KSD_OP_DEACT, KSD_SS_ALL_CFWSS key sequence.
*/
LOCAL BOOL ksd_seqDeactAllCondCf (USHORT          inIdx,
                                  CHAR*           inSeq,
                                  T_KSD_SEQGRP*   outSeqGrp,
                                  CHAR**          outRestSeq,
                                  T_KSD_SEQPARAM* outSeqParam)
{
  TRACE_FUNCTION ("ksd_seqDeactAllCondCf ()");

  outSeqParam->cf.opCd = KSD_OP_DEACT;
  outSeqParam->cf.ssCd = KSD_SS_ALL_CFWSS;

  *outSeqGrp  = SEQGRP_CF;

  return ksd_getCcfcFromDnBsT (inIdx, inSeq, outRestSeq, outSeqParam);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : KSD                      |
| STATE   : code                  ROUTINE : ksd_seqRegAllCondCf      |
+--------------------------------------------------------------------+

  PURPOSE : This function sets the ouput parameter for the
            SEQGRP_CF, KSD_OP_REG, KSD_SS_ALL_CFWSS key sequence.
*/
LOCAL BOOL ksd_seqRegAllCondCf (USHORT          inIdx,
                                CHAR*           inSeq,
                                T_KSD_SEQGRP*   outSeqGrp,
                                CHAR**          outRestSeq,
                                T_KSD_SEQPARAM* outSeqParam)
{
  TRACE_FUNCTION ("ksd_seqRegAllCondCf ()");

  outSeqParam->cf.opCd = KSD_OP_REG;
  outSeqParam->cf.ssCd = KSD_SS_ALL_CFWSS;

  *outSeqGrp  = SEQGRP_CF;

  return ksd_getCcfcFromDnBsT (inIdx, inSeq, outRestSeq, outSeqParam);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : KSD                      |
| STATE   : code                  ROUTINE : ksd_seqEraseAllCondCf    |
+--------------------------------------------------------------------+

  PURPOSE : This function sets the ouput parameter for the
            SEQGRP_CF, KSD_OP_ERS, KSD_SS_ALL_CFWSS key sequence.
*/
LOCAL BOOL ksd_seqEraseAllCondCf (USHORT          inIdx,
                                  CHAR*           inSeq,
                                  T_KSD_SEQGRP*   outSeqGrp,
                                  CHAR**          outRestSeq,
                                  T_KSD_SEQPARAM* outSeqParam)
{
  TRACE_FUNCTION ("ksd_seqEraseAllCondCf ()");

  outSeqParam->cf.opCd = KSD_OP_ERS;
  outSeqParam->cf.ssCd = KSD_SS_ALL_CFWSS;

  *outSeqGrp  = SEQGRP_CF;

  return ksd_getCcfcFromDnBsT (inIdx, inSeq, outRestSeq, outSeqParam);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : KSD                      |
| STATE   : code                  ROUTINE : ksd_seqIntrgtAllCondCf   |
+--------------------------------------------------------------------+

  PURPOSE : This function sets the ouput parameter for the
            SEQGRP_CF, KSD_OP_IRGT, KSD_SS_ALL_CFWSS key sequence.
*/
LOCAL BOOL ksd_seqIntrgtAllCondCf (USHORT          inIdx,
                                   CHAR*           inSeq,
                                   T_KSD_SEQGRP*   outSeqGrp,
                                   CHAR**          outRestSeq,
                                   T_KSD_SEQPARAM* outSeqParam)
{
  TRACE_FUNCTION ("ksd_seqIntrgtAllCondCf ()");

  outSeqParam->cf.opCd = KSD_OP_IRGT;
  outSeqParam->cf.ssCd = KSD_SS_ALL_CFWSS;

  *outSeqGrp  = SEQGRP_CF;

  return ksd_getCcfcFromDnBsT (inIdx, inSeq, outRestSeq, outSeqParam);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : KSD                      |
| STATE   : code                  ROUTINE : ksd_seqActNoRepCf        |
+--------------------------------------------------------------------+

  PURPOSE : This function sets the ouput parameter for the
            SEQGRP_CF, KSD_OP_ACT, KSD_SS_CFNRY key sequence.
*/
LOCAL BOOL ksd_seqActNoRepCf (USHORT          inIdx,
                              CHAR*           inSeq,
                              T_KSD_SEQGRP*   outSeqGrp,
                              CHAR**          outRestSeq,
                              T_KSD_SEQPARAM* outSeqParam)
{
  BOOL result;

  TRACE_FUNCTION ("ksd_seqActNoRepCf ()");

  result = ksd_getCcfcFromDnBsT (inIdx, inSeq, outRestSeq, outSeqParam);

  if ( outSeqParam->cf.num EQ NULL )
    outSeqParam->cf.opCd = KSD_OP_ACT;
  else
    outSeqParam->cf.opCd = KSD_OP_REG;

  outSeqParam->cf.ssCd = KSD_SS_CFNRY;

  *outSeqGrp  = SEQGRP_CF;

  return result;
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : KSD                      |
| STATE   : code                  ROUTINE : ksd_seqDeactNoRepCf      |
+--------------------------------------------------------------------+

  PURPOSE : This function sets the ouput parameter for the
            SEQGRP_CF, KSD_OP_DEACT, KSD_SS_CFNRY key sequence.
*/
LOCAL BOOL ksd_seqDeactNoRepCf (USHORT          inIdx,
                                CHAR*           inSeq,
                                T_KSD_SEQGRP*   outSeqGrp,
                                CHAR**          outRestSeq,
                                T_KSD_SEQPARAM* outSeqParam)
{
  TRACE_FUNCTION ("ksd_seqDeactNoRepCf ()");

  outSeqParam->cf.opCd = KSD_OP_DEACT;
  outSeqParam->cf.ssCd = KSD_SS_CFNRY;

  *outSeqGrp  = SEQGRP_CF;

  return ksd_getCcfcFromDnBsT (inIdx, inSeq, outRestSeq, outSeqParam);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : KSD                      |
| STATE   : code                  ROUTINE : ksd_seqRegNoRepCf        |
+--------------------------------------------------------------------+

  PURPOSE : This function sets the ouput parameter for the
            SEQGRP_CF, KSD_OP_REG, KSD_SS_CFNRY key sequence.
*/
LOCAL BOOL ksd_seqRegNoRepCf (USHORT          inIdx,
                              CHAR*           inSeq,
                              T_KSD_SEQGRP*   outSeqGrp,
                              CHAR**          outRestSeq,
                              T_KSD_SEQPARAM* outSeqParam)
{
//TISH, patch for OMAPS00124821
//start
#if 0
  TRACE_FUNCTION ("ksd_seqRegNoRepCf ()");

  outSeqParam->cf.opCd = KSD_OP_REG;
  outSeqParam->cf.ssCd = KSD_SS_CFNRY;

  *outSeqGrp  = SEQGRP_CF;

  return ksd_getCcfcFromDnBsT (inIdx, inSeq, outRestSeq, outSeqParam);
#else
  BOOL result;
  char *tempInSeq;
  TRACE_FUNCTION ("ksd_seqRegNoRepCf ()");
  MALLOC(tempInSeq,strlen(inSeq)+1);
  strcpy(tempInSeq,inSeq);
  result=ksd_getCcfcFromDnBsT (inIdx, inSeq, outRestSeq, outSeqParam);
  if (result==TRUE)
  {
//TISH patch for OMAPS00129224
//start
//	  if (outSeqParam->cf.time>30 || outSeqParam->cf.time<5)
	  if ((outSeqParam->cf.time != KSD_TIME_NONE) && (outSeqParam->cf.time>30 || outSeqParam->cf.time<5))
//end	  
	  {
		  memset(&(outSeqParam->cf),0,sizeof(T_ACI_KSD_CF_PRM));
		  strcpy(inSeq,tempInSeq);
          result  = ksd_seqUssd (inIdx,
                                inSeq,
                                outSeqGrp,
                                outRestSeq,
                                outSeqParam);
	  }
	  else
	  {
		  outSeqParam->cf.opCd = KSD_OP_REG;
		  outSeqParam->cf.ssCd = KSD_SS_CFNRY;
		  *outSeqGrp  = SEQGRP_CF;
	  }
  }
  MFREE(tempInSeq);
  return result;
#endif
//end
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : KSD                      |
| STATE   : code                  ROUTINE : ksd_seqEraseNoRepCf      |
+--------------------------------------------------------------------+

  PURPOSE : This function sets the ouput parameter for the
            SEQGRP_CF, KSD_OP_ERS, KSD_SS_CFNRY key sequence.
*/
LOCAL BOOL ksd_seqEraseNoRepCf (USHORT          inIdx,
                                CHAR*           inSeq,
                                T_KSD_SEQGRP*   outSeqGrp,
                                CHAR**          outRestSeq,
                                T_KSD_SEQPARAM* outSeqParam)
{
  TRACE_FUNCTION ("ksd_seqEraseNoRepCf ()");

  outSeqParam->cf.opCd = KSD_OP_ERS;
  outSeqParam->cf.ssCd = KSD_SS_CFNRY;

  *outSeqGrp  = SEQGRP_CF;

  return ksd_getCcfcFromDnBsT (inIdx, inSeq, outRestSeq, outSeqParam);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : KSD                      |
| STATE   : code                  ROUTINE : ksd_seqIntrgtNoRepCf     |
+--------------------------------------------------------------------+

  PURPOSE : This function sets the ouput parameter for the
            SEQGRP_CF, KSD_OP_IRGT, KSD_SS_CFNRY key sequence.
*/
LOCAL BOOL ksd_seqIntrgtNoRepCf (USHORT          inIdx,
                                 CHAR*           inSeq,
                                 T_KSD_SEQGRP*   outSeqGrp,
                                 CHAR**          outRestSeq,
                                 T_KSD_SEQPARAM* outSeqParam)
{
  TRACE_FUNCTION ("ksd_seqIntrgtNoRepCf ()");

  outSeqParam->cf.opCd = KSD_OP_IRGT;
  outSeqParam->cf.ssCd = KSD_SS_CFNRY;

  *outSeqGrp  = SEQGRP_CF;

  return ksd_getCcfcFromDnBsT (inIdx, inSeq, outRestSeq, outSeqParam);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : KSD                      |
| STATE   : code                  ROUTINE : ksd_seqActUncondCf       |
+--------------------------------------------------------------------+

  PURPOSE : This function sets the ouput parameter for the
            SEQGRP_CF, KSD_OP_ACT, KSD_SS_CFU key sequence.
*/
LOCAL BOOL ksd_seqActUncondCf (USHORT          inIdx,
                               CHAR*           inSeq,
                               T_KSD_SEQGRP*   outSeqGrp,
                               CHAR**          outRestSeq,
                               T_KSD_SEQPARAM* outSeqParam)
{
  BOOL result;

  TRACE_FUNCTION ("ksd_seqActUncondCf ()");

  result = ksd_getCcfcFromDnBs (inIdx, inSeq, outRestSeq, outSeqParam);

  if ( outSeqParam->cf.num EQ NULL )
    outSeqParam->cf.opCd = KSD_OP_ACT;
  else
    outSeqParam->cf.opCd = KSD_OP_REG;

  outSeqParam->cf.ssCd = KSD_SS_CFU;

  *outSeqGrp  = SEQGRP_CF;

  return result;
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : KSD                      |
| STATE   : code                  ROUTINE : ksd_seqDeactUncondCf     |
+--------------------------------------------------------------------+

  PURPOSE : This function sets the ouput parameter for the
            SEQGRP_CF, KSD_OP_DEACT, KSD_SS_CFU key sequence.
*/
LOCAL BOOL ksd_seqDeactUncondCf (USHORT          inIdx,
                                 CHAR*           inSeq,
                                 T_KSD_SEQGRP*   outSeqGrp,
                                 CHAR**          outRestSeq,
                                 T_KSD_SEQPARAM* outSeqParam)
{
  TRACE_FUNCTION ("ksd_seqDeactUncondCf ()");

  outSeqParam->cf.opCd = KSD_OP_DEACT;
  outSeqParam->cf.ssCd = KSD_SS_CFU;

  *outSeqGrp  = SEQGRP_CF;

  return ksd_getCcfcFromDnBs (inIdx, inSeq, outRestSeq, outSeqParam);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : KSD                      |
| STATE   : code                  ROUTINE : ksd_seqRegUncondCf       |
+--------------------------------------------------------------------+

  PURPOSE : This function sets the ouput parameter for the
            SEQGRP_CF, KSD_OP_REG, KSD_SS_CFU key sequence.
*/
LOCAL BOOL ksd_seqRegUncondCf (USHORT          inIdx,
                               CHAR*           inSeq,
                               T_KSD_SEQGRP*   outSeqGrp,
                               CHAR**          outRestSeq,
                               T_KSD_SEQPARAM* outSeqParam)
{
  TRACE_FUNCTION ("ksd_seqRegUncondCf ()");

  outSeqParam->cf.opCd = KSD_OP_REG;
  outSeqParam->cf.ssCd = KSD_SS_CFU;

  *outSeqGrp  = SEQGRP_CF;

  return ksd_getCcfcFromDnBs (inIdx, inSeq, outRestSeq, outSeqParam);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : KSD                      |
| STATE   : code                  ROUTINE : ksd_seqEraseUncondCf     |
+--------------------------------------------------------------------+

  PURPOSE : This function sets the ouput parameter for the
            SEQGRP_CF, KSD_OP_ERS, KSD_SS_CFU key sequence.
*/
LOCAL BOOL ksd_seqEraseUncondCf (USHORT          inIdx,
                                 CHAR*           inSeq,
                                 T_KSD_SEQGRP*   outSeqGrp,
                                 CHAR**          outRestSeq,
                                 T_KSD_SEQPARAM* outSeqParam)
{
  TRACE_FUNCTION ("ksd_seqEraseUncondCf ()");

  outSeqParam->cf.opCd = KSD_OP_ERS;
  outSeqParam->cf.ssCd = KSD_SS_CFU;

  *outSeqGrp  = SEQGRP_CF;

  return ksd_getCcfcFromDnBs (inIdx, inSeq, outRestSeq, outSeqParam);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : KSD                      |
| STATE   : code                  ROUTINE : ksd_seqIntrgtUncondCf    |
+--------------------------------------------------------------------+

  PURPOSE : This function sets the ouput parameter for the
            SEQGRP_CF, KSD_OP_IRGT, KSD_SS_CFU key sequence.
*/
LOCAL BOOL ksd_seqIntrgtUncondCf (USHORT          inIdx,
                                  CHAR*           inSeq,
                                  T_KSD_SEQGRP*   outSeqGrp,
                                  CHAR**          outRestSeq,
                                  T_KSD_SEQPARAM* outSeqParam)
{
  TRACE_FUNCTION ("ksd_seqIntrgtUncondCf ()");

  outSeqParam->cf.opCd = KSD_OP_IRGT;
  outSeqParam->cf.ssCd = KSD_SS_CFU;

  *outSeqGrp  = SEQGRP_CF;

  return ksd_getCcfcFromDnBs (inIdx, inSeq, outRestSeq, outSeqParam);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : KSD                      |
| STATE   : code                  ROUTINE : ksd_seqActBusyCf         |
+--------------------------------------------------------------------+

  PURPOSE : This function sets the ouput parameter for the
            SEQGRP_CF, KSD_OP_ACT, KSD_SS_CFB key sequence.
*/
LOCAL BOOL ksd_seqActBusyCf (USHORT          inIdx,
                             CHAR*           inSeq,
                             T_KSD_SEQGRP*   outSeqGrp,
                             CHAR**          outRestSeq,
                             T_KSD_SEQPARAM* outSeqParam)
{
  BOOL result;

  TRACE_FUNCTION ("ksd_seqActBusyCf ()");

  result = ksd_getCcfcFromDnBs (inIdx, inSeq, outRestSeq, outSeqParam);

  if ( outSeqParam->cf.num EQ NULL )
    outSeqParam->cf.opCd = KSD_OP_ACT;
  else
    outSeqParam->cf.opCd = KSD_OP_REG;

  outSeqParam->cf.ssCd = KSD_SS_CFB;

  *outSeqGrp  = SEQGRP_CF;

  return result;
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : KSD                      |
| STATE   : code                  ROUTINE : ksd_seqDeactBusyCf       |
+--------------------------------------------------------------------+

  PURPOSE : This function sets the ouput parameter for the
            SEQGRP_CF, KSD_OP_DEACT, KSD_SS_CFB key sequence.
*/
LOCAL BOOL ksd_seqDeactBusyCf (USHORT          inIdx,
                               CHAR*           inSeq,
                               T_KSD_SEQGRP*   outSeqGrp,
                               CHAR**          outRestSeq,
                               T_KSD_SEQPARAM* outSeqParam)
{
  TRACE_FUNCTION ("ksd_seqDeactBusyCf ()");

  outSeqParam->cf.opCd = KSD_OP_DEACT;
  outSeqParam->cf.ssCd = KSD_SS_CFB;

  *outSeqGrp  = SEQGRP_CF;

  return ksd_getCcfcFromDnBs (inIdx, inSeq, outRestSeq, outSeqParam);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : KSD                      |
| STATE   : code                  ROUTINE : ksd_seqRegBusyCf         |
+--------------------------------------------------------------------+

  PURPOSE : This function sets the ouput parameter for the
            SEQGRP_CF, KSD_OP_REG, KSD_SS_CFB key sequence.
*/
LOCAL BOOL ksd_seqRegBusyCf (USHORT          inIdx,
                             CHAR*           inSeq,
                             T_KSD_SEQGRP*   outSeqGrp,
                             CHAR**          outRestSeq,
                             T_KSD_SEQPARAM* outSeqParam)
{
  TRACE_FUNCTION ("ksd_seqRegBusyCf ()");

  outSeqParam->cf.opCd = KSD_OP_REG;
  outSeqParam->cf.ssCd = KSD_SS_CFB;

  *outSeqGrp  = SEQGRP_CF;

  return ksd_getCcfcFromDnBs (inIdx, inSeq, outRestSeq, outSeqParam);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : KSD                      |
| STATE   : code                  ROUTINE : ksd_seqEraseBusyCf       |
+--------------------------------------------------------------------+

  PURPOSE : This function sets the ouput parameter for the
            SEQGRP_CF, KSD_OP_ERS, KSD_SS_CFB key sequence.
*/
LOCAL BOOL ksd_seqEraseBusyCf (USHORT          inIdx,
                               CHAR*           inSeq,
                               T_KSD_SEQGRP*   outSeqGrp,
                               CHAR**          outRestSeq,
                               T_KSD_SEQPARAM* outSeqParam)
{
  TRACE_FUNCTION ("ksd_seqEraseBusyCf ()");

  outSeqParam->cf.opCd = KSD_OP_ERS;
  outSeqParam->cf.ssCd = KSD_SS_CFB;

  *outSeqGrp  = SEQGRP_CF;

  return ksd_getCcfcFromDnBs (inIdx, inSeq, outRestSeq, outSeqParam);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : KSD                      |
| STATE   : code                  ROUTINE : ksd_seqIntrgtBusyCf      |
+--------------------------------------------------------------------+

  PURPOSE : This function sets the ouput parameter for the
            SEQGRP_CF, KSD_OP_IRGT, KSD_SS_CFB key sequence.
*/
LOCAL BOOL ksd_seqIntrgtBusyCf (USHORT          inIdx,
                                CHAR*           inSeq,
                                T_KSD_SEQGRP*   outSeqGrp,
                                CHAR**          outRestSeq,
                                T_KSD_SEQPARAM* outSeqParam)
{
  TRACE_FUNCTION ("ksd_seqIntrgtBusyCf ()");

  outSeqParam->cf.opCd = KSD_OP_IRGT;
  outSeqParam->cf.ssCd = KSD_SS_CFB;

  *outSeqGrp  = SEQGRP_CF;

  return ksd_getCcfcFromDnBs (inIdx, inSeq, outRestSeq, outSeqParam);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : KSD                      |
| STATE   : code                  ROUTINE : ksd_seqActNotReachCf     |
+--------------------------------------------------------------------+

  PURPOSE : This function sets the ouput parameter for the
            SEQGRP_CF, KSD_OP_ACT, KSD_SS_CFNRC key sequence.
*/
LOCAL BOOL ksd_seqActNotReachCf (USHORT          inIdx,
                                 CHAR*           inSeq,
                                 T_KSD_SEQGRP*   outSeqGrp,
                                 CHAR**          outRestSeq,
                                 T_KSD_SEQPARAM* outSeqParam)
{
  BOOL result;

  TRACE_FUNCTION ("ksd_seqActNotReachCf ()");

  result = ksd_getCcfcFromDnBs (inIdx, inSeq, outRestSeq, outSeqParam);

  if ( outSeqParam->cf.num EQ NULL )
    outSeqParam->cf.opCd = KSD_OP_ACT;
  else
    outSeqParam->cf.opCd = KSD_OP_REG;

  outSeqParam->cf.ssCd = KSD_SS_CFNRC;

  *outSeqGrp  = SEQGRP_CF;

  return result;
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : KSD                      |
| STATE   : code                  ROUTINE : ksd_seqDeactNotReachCf   |
+--------------------------------------------------------------------+

  PURPOSE : This function sets the ouput parameter for the
            SEQGRP_CF, KSD_OP_DEACT, KSD_SS_CFNRC key sequence.
*/
LOCAL BOOL ksd_seqDeactNotReachCf (USHORT          inIdx,
                                   CHAR*           inSeq,
                                   T_KSD_SEQGRP*   outSeqGrp,
                                   CHAR**          outRestSeq,
                                   T_KSD_SEQPARAM* outSeqParam)
{
  TRACE_FUNCTION ("ksd_seqDeactNotReachCf ()");

  outSeqParam->cf.opCd = KSD_OP_DEACT;
  outSeqParam->cf.ssCd = KSD_SS_CFNRC;

  *outSeqGrp  = SEQGRP_CF;

  return ksd_getCcfcFromDnBs (inIdx, inSeq, outRestSeq, outSeqParam);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : KSD                      |
| STATE   : code                  ROUTINE : ksd_seqRegNotReachCf     |
+--------------------------------------------------------------------+

  PURPOSE : This function sets the ouput parameter for the
            SEQGRP_CF, KSD_OP_REG, KSD_SS_CFNRC key sequence.
*/
LOCAL BOOL ksd_seqRegNotReachCf (USHORT          inIdx,
                                 CHAR*           inSeq,
                                 T_KSD_SEQGRP*   outSeqGrp,
                                 CHAR**          outRestSeq,
                                 T_KSD_SEQPARAM* outSeqParam)
{
  TRACE_FUNCTION ("ksd_seqRegNotReachCf ()");

  outSeqParam->cf.opCd = KSD_OP_REG;
  outSeqParam->cf.ssCd = KSD_SS_CFNRC;

  *outSeqGrp  = SEQGRP_CF;

  return ksd_getCcfcFromDnBs (inIdx, inSeq, outRestSeq, outSeqParam);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : KSD                      |
| STATE   : code                  ROUTINE : ksd_seqEraseNotReachCf   |
+--------------------------------------------------------------------+

  PURPOSE : This function sets the ouput parameter for the
            SEQGRP_CF, KSD_OP_ERS, KSD_SS_CFNRC key sequence.
*/
LOCAL BOOL ksd_seqEraseNotReachCf (USHORT          inIdx,
                                   CHAR*           inSeq,
                                   T_KSD_SEQGRP*   outSeqGrp,
                                   CHAR**          outRestSeq,
                                   T_KSD_SEQPARAM* outSeqParam)
{
  TRACE_FUNCTION ("ksd_seqEraseNotReachCf ()");

  outSeqParam->cf.opCd = KSD_OP_ERS;
  outSeqParam->cf.ssCd = KSD_SS_CFNRC;

  *outSeqGrp  = SEQGRP_CF;

  return ksd_getCcfcFromDnBs (inIdx, inSeq, outRestSeq, outSeqParam);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : KSD                      |
| STATE   : code                  ROUTINE : ksd_seqIntrgtNotReachCf  |
+--------------------------------------------------------------------+

  PURPOSE : This function sets the ouput parameter for the
            SEQGRP_CF, KSD_OP_IRGT, KSD_SS_CFNRC key sequence.
*/
LOCAL BOOL ksd_seqIntrgtNotReachCf (USHORT          inIdx,
                                    CHAR*           inSeq,
                                    T_KSD_SEQGRP*   outSeqGrp,
                                    CHAR**          outRestSeq,
                                    T_KSD_SEQPARAM* outSeqParam)
{
  TRACE_FUNCTION ("ksd_seqIntrgtNotReachCf ()");

  outSeqParam->cf.opCd = KSD_OP_IRGT;
  outSeqParam->cf.ssCd = KSD_SS_CFNRC;

  *outSeqGrp  = SEQGRP_CF;

  return ksd_getCcfcFromDnBs (inIdx, inSeq, outRestSeq, outSeqParam);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : KSD                      |
| STATE   : code                  ROUTINE : ksd_getClckFromPwBs      |
+--------------------------------------------------------------------+

  PURPOSE : This function extracts the parameters of the sAT_PlusCLCK
            function of the ACI out of the supplementary information.

            <inIdx>:       index of key sequence table
            <inSeq>:       key sequence, to be decoded
            <inGsm>:       indicates whether this SSC group is
                           GSM standard
            <outRestSeq>:  rest key sequence, to be decoded by a
                           further call to this function
            <outSeqParam>: sequence parameter

            returns:       TRUE if output parameters are valid,
                           otherwise FALSE
*/
LOCAL BOOL ksd_getClckFromPwBs (USHORT          inIdx,
                                CHAR*           inSeq,
                                BOOL            inGsm,
                                CHAR**          outRestSeq,
                                T_KSD_SEQPARAM* outSeqParam)
{
  const USHORT   siNum       = 2;
  T_KSD_DCD_CPLE dcdCple[2];
  T_KSD_SIPARAM  pwParam;
  T_KSD_SIPARAM  bsParam;
  USHORT         readChars = 0;
  USHORT         lenKeySeq;
  BOOL           isSuccess   = FALSE;

  TRACE_FUNCTION ("ksd_getClckFromPwBs ()");

  lenKeySeq = (inGsm ? ksd_strlen (keyPasswd[inIdx].keySeq) :
                       ksd_strlen (keyPasswdNonGsm[inIdx].keySeq));

  dcdCple[0].type    = SI_CHAR_STAR;
  dcdCple[0].param   = &pwParam;
  pwParam.siCharStar = NULL;

  dcdCple[1].type    = SI_UBYTE;
  dcdCple[1].param   = &bsParam;
  bsParam.siUbyte    = KSD_BS_None;

  if(*(inSeq+lenKeySeq)   EQ STOP_SEQ OR 
     *(inSeq+lenKeySeq+1) EQ 0x00)
  {
    /* no password given, eg a query, where it is not needed */
    TRACE_EVENT("ss code for password but none given");
    outSeqParam->cb.pwd  = 0x00;
    outSeqParam->cb.bsCd = bsParam.siUbyte;

    isSuccess = TRUE;
  }
  else if (ksd_extractSi (inSeq + lenKeySeq, siNum,
                     &readChars, dcdCple)      EQ TRUE)
  {
    outSeqParam->cb.pwd  = (UBYTE*)pwParam.siCharStar;
    outSeqParam->cb.bsCd = bsParam.siUbyte;

    isSuccess = TRUE;
  }

  *outRestSeq = inSeq + lenKeySeq + readChars;

  return isSuccess;
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : KSD                      |
| STATE   : code                  ROUTINE : ksd_seqActSimLock        |
+--------------------------------------------------------------------+

  PURPOSE : This function sets the ouput parameter for the
            SEQGRP_ACT_SIM_LOCK key sequence.
*/
LOCAL BOOL ksd_seqActSimLock (USHORT          inIdx,
                              CHAR*           inSeq,
                              T_KSD_SEQGRP*   outSeqGrp,
                              CHAR**          outRestSeq,
                              T_KSD_SEQPARAM* outSeqParam)
{
  TRACE_FUNCTION ("ksd_seqActSimLock ()");

  outSeqParam->clck.fac  = FAC_Sc;
  outSeqParam->clck.mode = CLCK_MOD_Lock;

  *outSeqGrp  = SEQGRP_ACT_SIM_LOCK;

  return ksd_getClckFromPwBs (inIdx, inSeq, FALSE, outRestSeq, outSeqParam);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : KSD                      |
| STATE   : code                  ROUTINE : ksd_seqDeactSimLock      |
+--------------------------------------------------------------------+

  PURPOSE : This function sets the ouput parameter for the
            SEQGRP_DEACT_SIM_LOCK key sequence.
*/
LOCAL BOOL ksd_seqDeactSimLock (USHORT          inIdx,
                                CHAR*           inSeq,
                                T_KSD_SEQGRP*   outSeqGrp,
                                CHAR**          outRestSeq,
                                T_KSD_SEQPARAM* outSeqParam)
{
  TRACE_FUNCTION ("ksd_seqDeactSimLock ()");

  outSeqParam->clck.fac  = FAC_Sc;
  outSeqParam->clck.mode = CLCK_MOD_Unlock;

  *outSeqGrp  = SEQGRP_DEACT_SIM_LOCK;

  return ksd_getClckFromPwBs (inIdx, inSeq, FALSE, outRestSeq, outSeqParam);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : KSD                      |
| STATE   : code                  ROUTINE : ksd_seqIntrgtSimLock     |
+--------------------------------------------------------------------+

  PURPOSE : This function sets the ouput parameter for the
            SEQGRP_INTRGT_SIM_LOCK key sequence.
*/
LOCAL BOOL ksd_seqIntrgtSimLock (USHORT          inIdx,
                                 CHAR*           inSeq,
                                 T_KSD_SEQGRP*   outSeqGrp,
                                 CHAR**          outRestSeq,
                                 T_KSD_SEQPARAM* outSeqParam)
{
  TRACE_FUNCTION ("ksd_seqIntrgtSimLock ()");

  *outSeqGrp  = SEQGRP_INTRGT_SIM_LOCK;
  *outRestSeq = inSeq + ksd_strlen (keySeqTableNonGsm[inIdx].keySeq);

  outSeqParam->clck_query.fac = FAC_Sc;

  return TRUE;
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : KSD                      |
| STATE   : code                  ROUTINE : ksd_seqDeactOutBarrServ  |
+--------------------------------------------------------------------+

  PURPOSE : This function sets the ouput parameter for the
            SEQGRP_CB, KSD_OP_DEACT, KSD_SS_BOC key sequence.
*/
LOCAL BOOL ksd_seqDeactOutBarrServ (USHORT          inIdx,
                                    CHAR*           inSeq,
                                    T_KSD_SEQGRP*   outSeqGrp,
                                    CHAR**          outRestSeq,
                                    T_KSD_SEQPARAM* outSeqParam)
{
  TRACE_FUNCTION ("ksd_seqDeactOutBarrServ ()");

  outSeqParam->cb.opCd = KSD_OP_DEACT;
  outSeqParam->cb.ssCd = KSD_SS_BOC;

  *outSeqGrp  = SEQGRP_CB;

  return ksd_getClckFromPwBs (inIdx, inSeq, TRUE, outRestSeq, outSeqParam);
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : KSD                      |
| STATE   : code                  ROUTINE : ksd_seqActBaoicExcHome   |
+--------------------------------------------------------------------+

  PURPOSE : This function sets the ouput parameter for the
            SEQGRP_CB, KSD_OP_ACT, KSD_SS_BOICXH key sequence.
*/
LOCAL BOOL ksd_seqActBaoicExcHome (USHORT          inIdx,
                                   CHAR*           inSeq,
                                   T_KSD_SEQGRP*   outSeqGrp,
                                   CHAR**          outRestSeq,
                                   T_KSD_SEQPARAM* outSeqParam)
{
  TRACE_FUNCTION ("ksd_seqActBaoicExcHome ()");

  outSeqParam->cb.opCd = KSD_OP_ACT;
  outSeqParam->cb.ssCd = KSD_SS_BOICXH;

  *outSeqGrp  = SEQGRP_CB;

  return ksd_getClckFromPwBs (inIdx, inSeq, TRUE, outRestSeq, outSeqParam);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : KSD                      |
| STATE   : code                  ROUTINE : ksd_seqDeactBaoicExcHome |
+--------------------------------------------------------------------+

  PURPOSE : This function sets the ouput parameter for the
            SEQGRP_CB, KSD_OP_DEACT, KSD_SS_BOICXH key sequence.
*/
LOCAL BOOL ksd_seqDeactBaoicExcHome (USHORT          inIdx,
                                     CHAR*           inSeq,
                                     T_KSD_SEQGRP*   outSeqGrp,
                                     CHAR**          outRestSeq,
                                     T_KSD_SEQPARAM* outSeqParam)
{
  TRACE_FUNCTION ("ksd_seqDeactBaoicExcHome ()");

  outSeqParam->cb.opCd = KSD_OP_DEACT;
  outSeqParam->cb.ssCd = KSD_SS_BOICXH;

  *outSeqGrp  = SEQGRP_CB;

  return ksd_getClckFromPwBs (inIdx, inSeq, TRUE, outRestSeq, outSeqParam);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : KSD                      |
| STATE   : code                  ROUTINE : ksd_seqIntrgtBaoicExcHome|
+--------------------------------------------------------------------+

  PURPOSE : This function sets the ouput parameter for the
            SEQGRP_CB, KSD_OP_IRGT, KSD_SS_BOICXH key sequence.
*/
LOCAL BOOL ksd_seqIntrgtBaoicExcHome (USHORT          inIdx,
                                      CHAR*           inSeq,
                                      T_KSD_SEQGRP*   outSeqGrp,
                                      CHAR**          outRestSeq,
                                      T_KSD_SEQPARAM* outSeqParam)
{
  TRACE_FUNCTION ("ksd_seqIntrgtBaoicExcHome ()");

  outSeqParam->cb.opCd = KSD_OP_IRGT;
  outSeqParam->cb.ssCd = KSD_SS_BOICXH;

  *outSeqGrp  = SEQGRP_CB;

  return ksd_getClckFromPwBs (inIdx, inSeq, TRUE, outRestSeq, outSeqParam);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : KSD                      |
| STATE   : code                  ROUTINE : ksd_seqActBaoic          |
+--------------------------------------------------------------------+

  PURPOSE : This function sets the ouput parameter for the
            SEQGRP_CB, KSD_OP_ACT, KSD_SS_BOIC key sequence.
*/
LOCAL BOOL ksd_seqActBaoic (USHORT          inIdx,
                            CHAR*           inSeq,
                            T_KSD_SEQGRP*   outSeqGrp,
                            CHAR**          outRestSeq,
                            T_KSD_SEQPARAM* outSeqParam)
{
  TRACE_FUNCTION ("ksd_seqActBaoic ()");

  outSeqParam->cb.opCd = KSD_OP_ACT;
  outSeqParam->cb.ssCd = KSD_SS_BOIC;

  *outSeqGrp  = SEQGRP_CB;

  return ksd_getClckFromPwBs (inIdx, inSeq, TRUE, outRestSeq, outSeqParam);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : KSD                      |
| STATE   : code                  ROUTINE : ksd_seqDeactBaoic        |
+--------------------------------------------------------------------+

  PURPOSE : This function sets the ouput parameter for the
            SEQGRP_CB, KSD_OP_DEACT, KSD_SS_BOIC key sequence.
*/
LOCAL BOOL ksd_seqDeactBaoic (USHORT          inIdx,
                              CHAR*           inSeq,
                              T_KSD_SEQGRP*   outSeqGrp,
                              CHAR**          outRestSeq,
                              T_KSD_SEQPARAM* outSeqParam)
{
  TRACE_FUNCTION ("ksd_seqDeactBaoic ()");

  outSeqParam->cb.opCd = KSD_OP_DEACT;
  outSeqParam->cb.ssCd = KSD_SS_BOIC;

  *outSeqGrp  = SEQGRP_CB;

  return ksd_getClckFromPwBs (inIdx, inSeq, TRUE, outRestSeq, outSeqParam);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : KSD                      |
| STATE   : code                  ROUTINE : ksd_seqIntrgtBaoic       |
+--------------------------------------------------------------------+

  PURPOSE : This function sets the ouput parameter for the
            SEQGRP_CB, KSD_OP_IRGT, KSD_SS_BOIC key sequence.
*/
LOCAL BOOL ksd_seqIntrgtBaoic (USHORT          inIdx,
                               CHAR*           inSeq,
                               T_KSD_SEQGRP*   outSeqGrp,
                               CHAR**          outRestSeq,
                               T_KSD_SEQPARAM* outSeqParam)
{
  TRACE_FUNCTION ("ksd_seqIntrgtBaoic ()");

  outSeqParam->cb.opCd = KSD_OP_IRGT;
  outSeqParam->cb.ssCd = KSD_SS_BOIC;

  *outSeqGrp  = SEQGRP_CB;

  return ksd_getClckFromPwBs (inIdx, inSeq, TRUE, outRestSeq, outSeqParam);
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : KSD                      |
| STATE   : code                  ROUTINE : ksd_seqDeactAllBarrServ  |
+--------------------------------------------------------------------+

  PURPOSE : This function sets the ouput parameter for the
            SEQGRP_CB, KSD_OP_DEACT, KSD_SS_ALL_CBSS key sequence.
*/
LOCAL BOOL ksd_seqDeactAllBarrServ (USHORT          inIdx,
                                    CHAR*           inSeq,
                                    T_KSD_SEQGRP*   outSeqGrp,
                                    CHAR**          outRestSeq,
                                    T_KSD_SEQPARAM* outSeqParam)
{
  TRACE_FUNCTION ("ksd_seqDeactAllBarrServ ()");

  outSeqParam->cb.opCd = KSD_OP_DEACT;
  outSeqParam->cb.ssCd = KSD_SS_ALL_CBSS;

  *outSeqGrp  = SEQGRP_CB;

  return ksd_getClckFromPwBs (inIdx, inSeq, TRUE, outRestSeq, outSeqParam);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : KSD                      |
| STATE   : code                  ROUTINE : ksd_seqActBaoc           |
+--------------------------------------------------------------------+

  PURPOSE : This function sets the ouput parameter for the
            SEQGRP_CB, KSD_OP_ACT, KSD_SS_BAOC key sequence.
*/
LOCAL BOOL ksd_seqActBaoc (USHORT          inIdx,
                           CHAR*           inSeq,
                           T_KSD_SEQGRP*   outSeqGrp,
                           CHAR**          outRestSeq,
                           T_KSD_SEQPARAM* outSeqParam)
{
  TRACE_FUNCTION ("ksd_seqActBaoc ()");

  outSeqParam->cb.opCd = KSD_OP_ACT;
  outSeqParam->cb.ssCd = KSD_SS_BAOC;

  *outSeqGrp  = SEQGRP_CB;

  return ksd_getClckFromPwBs (inIdx, inSeq, TRUE, outRestSeq, outSeqParam);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : KSD                      |
| STATE   : code                  ROUTINE : ksd_seqDeactBaoc         |
+--------------------------------------------------------------------+

  PURPOSE : This function sets the ouput parameter for the
            SEQGRP_CB, KSD_OP_DEACT, KSD_SS_BAOC key sequence.
*/
LOCAL BOOL ksd_seqDeactBaoc (USHORT          inIdx,
                             CHAR*           inSeq,
                             T_KSD_SEQGRP*   outSeqGrp,
                             CHAR**          outRestSeq,
                             T_KSD_SEQPARAM* outSeqParam)
{
  TRACE_FUNCTION ("ksd_seqDeactBaoc ()");

  outSeqParam->cb.opCd = KSD_OP_DEACT;
  outSeqParam->cb.ssCd = KSD_SS_BAOC;

  *outSeqGrp  = SEQGRP_CB;

  return ksd_getClckFromPwBs (inIdx, inSeq, TRUE, outRestSeq, outSeqParam);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : KSD                      |
| STATE   : code                  ROUTINE : ksd_seqIntrgtBaoc        |
+--------------------------------------------------------------------+

  PURPOSE : This function sets the ouput parameter for the
            SEQGRP_CB, KSD_OP_IRGT, KSD_SS_BAOC key sequence.
*/
LOCAL BOOL ksd_seqIntrgtBaoc (USHORT          inIdx,
                              CHAR*           inSeq,
                              T_KSD_SEQGRP*   outSeqGrp,
                              CHAR**          outRestSeq,
                              T_KSD_SEQPARAM* outSeqParam)
{
  TRACE_FUNCTION ("ksd_seqIntrgtBaoc ()");

  outSeqParam->cb.opCd = KSD_OP_IRGT;
  outSeqParam->cb.ssCd = KSD_SS_BAOC;

  *outSeqGrp  = SEQGRP_CB;

  return ksd_getClckFromPwBs (inIdx, inSeq, TRUE, outRestSeq, outSeqParam);
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : KSD                      |
| STATE   : code                  ROUTINE : ksd_seqDeactInBarrServ   |
+--------------------------------------------------------------------+

  PURPOSE : This function sets the ouput parameter for the
            SEQGRP_CB, KSD_OP_DEACT, KSD_SS_BIC key sequence.
*/
LOCAL BOOL ksd_seqDeactInBarrServ (USHORT          inIdx,
                                   CHAR*           inSeq,
                                   T_KSD_SEQGRP*   outSeqGrp,
                                   CHAR**          outRestSeq,
                                   T_KSD_SEQPARAM* outSeqParam)
{
  TRACE_FUNCTION ("ksd_seqDeactInBarrServ ()");

  outSeqParam->cb.opCd = KSD_OP_DEACT;
  outSeqParam->cb.ssCd = KSD_SS_BIC;

  *outSeqGrp  = SEQGRP_CB;

  return ksd_getClckFromPwBs (inIdx, inSeq, TRUE, outRestSeq, outSeqParam);
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : KSD                      |
| STATE   : code                  ROUTINE : ksd_seqActBaicRoam       |
+--------------------------------------------------------------------+

  PURPOSE : This function sets the ouput parameter for the
            SEQGRP_CB, KSD_OP_ACT, KSD_SS_BICRM key sequence.
*/
LOCAL BOOL ksd_seqActBaicRoam (USHORT          inIdx,
                               CHAR*           inSeq,
                               T_KSD_SEQGRP*   outSeqGrp,
                               CHAR**          outRestSeq,
                               T_KSD_SEQPARAM* outSeqParam)
{
  TRACE_FUNCTION ("ksd_seqActBaicRoam ()");

  outSeqParam->cb.opCd = KSD_OP_ACT;
  outSeqParam->cb.ssCd = KSD_SS_BICRM;

  *outSeqGrp  = SEQGRP_CB;

  return ksd_getClckFromPwBs (inIdx, inSeq, TRUE, outRestSeq, outSeqParam);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : KSD                      |
| STATE   : code                  ROUTINE : ksd_seqDeactBaicRoam     |
+--------------------------------------------------------------------+

  PURPOSE : This function sets the ouput parameter for the
            SEQGRP_CB, KSD_OP_DEACT, KSD_SS_BICRM key sequence.
*/
LOCAL BOOL ksd_seqDeactBaicRoam (USHORT          inIdx,
                                 CHAR*           inSeq,
                                 T_KSD_SEQGRP*   outSeqGrp,
                                 CHAR**          outRestSeq,
                                 T_KSD_SEQPARAM* outSeqParam)
{
  TRACE_FUNCTION ("ksd_seqDeactBaicRoam ()");

  outSeqParam->cb.opCd = KSD_OP_DEACT;
  outSeqParam->cb.ssCd = KSD_SS_BICRM;

  *outSeqGrp  = SEQGRP_CB;

  return ksd_getClckFromPwBs (inIdx, inSeq, TRUE, outRestSeq, outSeqParam);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : KSD                      |
| STATE   : code                  ROUTINE : ksd_seqIntrgtBaicRoam    |
+--------------------------------------------------------------------+

  PURPOSE : This function sets the ouput parameter for the
            SEQGRP_CB, KSD_OP_IRGT, KSD_SS_BICRM key sequence.
*/
LOCAL BOOL ksd_seqIntrgtBaicRoam (USHORT          inIdx,
                                  CHAR*           inSeq,
                                  T_KSD_SEQGRP*   outSeqGrp,
                                  CHAR**          outRestSeq,
                                  T_KSD_SEQPARAM* outSeqParam)
{
  TRACE_FUNCTION ("ksd_seqIntrgtBaicRoam ()");

  outSeqParam->cb.opCd = KSD_OP_IRGT;
  outSeqParam->cb.ssCd = KSD_SS_BICRM;

  *outSeqGrp  = SEQGRP_CB;

  return ksd_getClckFromPwBs (inIdx, inSeq, TRUE, outRestSeq, outSeqParam);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : KSD                      |
| STATE   : code                  ROUTINE : ksd_seqActBaic           |
+--------------------------------------------------------------------+

  PURPOSE : This function sets the ouput parameter for the
            SEQGRP_CB, KSD_OP_ACT, KSD_SS_BAIC key sequence.
*/
LOCAL BOOL ksd_seqActBaic (USHORT          inIdx,
                           CHAR*           inSeq,
                           T_KSD_SEQGRP*   outSeqGrp,
                           CHAR**          outRestSeq,
                           T_KSD_SEQPARAM* outSeqParam)
{
  TRACE_FUNCTION ("ksd_seqActBaic ()");

  outSeqParam->cb.opCd = KSD_OP_ACT;
  outSeqParam->cb.ssCd = KSD_SS_BAIC;

  *outSeqGrp  = SEQGRP_CB;

  return ksd_getClckFromPwBs (inIdx, inSeq, TRUE, outRestSeq, outSeqParam);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : KSD                      |
| STATE   : code                  ROUTINE : ksd_seqDeactBaic         |
+--------------------------------------------------------------------+

  PURPOSE : This function sets the ouput parameter for the
            SEQGRP_CB, KSD_OP_DEACT, KSD_SS_BAIC key sequence.
*/
LOCAL BOOL ksd_seqDeactBaic (USHORT          inIdx,
                             CHAR*           inSeq,
                             T_KSD_SEQGRP*   outSeqGrp,
                             CHAR**          outRestSeq,
                             T_KSD_SEQPARAM* outSeqParam)
{
  TRACE_FUNCTION ("ksd_seqDeactBaic ()");

  outSeqParam->cb.opCd = KSD_OP_DEACT;
  outSeqParam->cb.ssCd = KSD_SS_BAIC;

  *outSeqGrp  = SEQGRP_CB;

  return ksd_getClckFromPwBs (inIdx, inSeq, TRUE, outRestSeq, outSeqParam);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : KSD                      |
| STATE   : code                  ROUTINE : ksd_seqIntrgtBaic        |
+--------------------------------------------------------------------+

  PURPOSE : This function sets the ouput parameter for the
            SEQGRP_CB, KSD_OP_IRGT, KSD_SS_BAIC key sequence.
*/
LOCAL BOOL ksd_seqIntrgtBaic (USHORT          inIdx,
                              CHAR*           inSeq,
                              T_KSD_SEQGRP*   outSeqGrp,
                              CHAR**          outRestSeq,
                              T_KSD_SEQPARAM* outSeqParam)
{
  TRACE_FUNCTION ("ksd_seqIntrgtBaic ()");

  outSeqParam->cb.opCd = KSD_OP_IRGT;
  outSeqParam->cb.ssCd = KSD_SS_BAIC;

  *outSeqGrp  = SEQGRP_CB;

  return ksd_getClckFromPwBs (inIdx, inSeq, TRUE, outRestSeq, outSeqParam);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : KSD                      |
| STATE   : code                  ROUTINE : ksd_seqIntrgtClir        |
+--------------------------------------------------------------------+

  PURPOSE : This function sets the ouput parameter for the
            SEQGRP_CL, KSD_OP_IRGT, KSD_SS_CLIR key sequence.
*/
LOCAL BOOL ksd_seqIntrgtClir (USHORT          inIdx,
                              CHAR*           inSeq,
                              T_KSD_SEQGRP*   outSeqGrp,
                              CHAR**          outRestSeq,
                              T_KSD_SEQPARAM* outSeqParam)
{
  TRACE_FUNCTION ("ksd_seqIntrgtClir ()");

  outSeqParam->cb.opCd = KSD_OP_IRGT;
  outSeqParam->cb.ssCd = KSD_SS_CLIR;

  *outSeqGrp  = SEQGRP_CL;

  *outRestSeq = inSeq + ksd_strlen (keySeqTable[inIdx].keySeq);

  return TRUE;
}



/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : KSD                      |
| STATE   : code                  ROUTINE : ksd_seqSupClir           |
+--------------------------------------------------------------------+

  PURPOSE : This function sets the ouput parameter for the
            SEQGRP_SUP_CLIR key sequence.
*/
LOCAL BOOL ksd_seqSupClir (USHORT          inIdx,
                           CHAR*           inSeq,
                           T_KSD_SEQGRP*   outSeqGrp,
                           CHAR**          outRestSeq,
                           T_KSD_SEQPARAM* outSeqParam)
{
  TRACE_FUNCTION ("ksd_seqSupClir ()");

  *outSeqGrp  = SEQGRP_SUP_CLIR;
  *outRestSeq = inSeq + ksd_strlen (keySeqTable[inIdx].keySeq);

  outSeqParam->Clir.mode = CLIR_MOD_Supp;

  return TRUE;
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : KSD                      |
| STATE   : code                  ROUTINE : ksd_seqInvClir           |
+--------------------------------------------------------------------+

  PURPOSE : This function sets the ouput parameter for the
            SEQGRP_INV_CLIR key sequence.
*/
LOCAL BOOL ksd_seqInvClir (USHORT          inIdx,
                           CHAR*           inSeq,
                           T_KSD_SEQGRP*   outSeqGrp,
                           CHAR**          outRestSeq,
                           T_KSD_SEQPARAM* outSeqParam)
{
  TRACE_FUNCTION ("ksd_seqInvClir ()");

  *outSeqGrp  = SEQGRP_INV_CLIR;
  *outRestSeq = inSeq + ksd_strlen (keySeqTable[inIdx].keySeq);

  outSeqParam->Clir.mode = CLIR_MOD_Invoc;

  return TRUE;
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : KSD                      |
| STATE   : code                  ROUTINE : ksd_seqIntrgtClip        |
+--------------------------------------------------------------------+

  PURPOSE : This function sets the ouput parameter for the
            SEQGRP_CL, KSD_OP_IRGT, KSD_SS_CLIP key sequence.
*/
LOCAL BOOL ksd_seqIntrgtClip (USHORT          inIdx,
                              CHAR*           inSeq,
                              T_KSD_SEQGRP*   outSeqGrp,
                              CHAR**          outRestSeq,
                              T_KSD_SEQPARAM* outSeqParam)
{
  TRACE_FUNCTION ("ksd_seqIntrgtClip ()");

  outSeqParam->cb.opCd = KSD_OP_IRGT;
  outSeqParam->cb.ssCd = KSD_SS_CLIP;

  *outSeqGrp  = SEQGRP_CL;

  *outRestSeq = inSeq + ksd_strlen (keySeqTable[inIdx].keySeq);

  return TRUE;
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : KSD                      |
| STATE   : code                  ROUTINE : ksd_seqSupClip           |
+--------------------------------------------------------------------+

  PURPOSE : This function sets the ouput parameter for the
            SEQGRP_SUP_CLIP key sequence.
*/
LOCAL BOOL ksd_seqSupClip (USHORT          inIdx,
                           CHAR*           inSeq,
                           T_KSD_SEQGRP*   outSeqGrp,
                           CHAR**          outRestSeq,
                           T_KSD_SEQPARAM* outSeqParam)
{
  TRACE_FUNCTION ("ksd_seqSupClip ()");

  *outSeqGrp  = SEQGRP_SUP_CLIP;
  *outRestSeq = inSeq + ksd_strlen (keySeqTable[inIdx].keySeq);

  outSeqParam->Clip.mode = CLIP_MOD_Disable;

  return TRUE;
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : KSD                      |
| STATE   : code                  ROUTINE : ksd_seqInvClip           |
+--------------------------------------------------------------------+

  PURPOSE : This function sets the ouput parameter for the
            SEQGRP_INV_CLIP key sequence.
*/
LOCAL BOOL ksd_seqInvClip (USHORT          inIdx,
                           CHAR*           inSeq,
                           T_KSD_SEQGRP*   outSeqGrp,
                           CHAR**          outRestSeq,
                           T_KSD_SEQPARAM* outSeqParam)
{
  TRACE_FUNCTION ("ksd_seqInvClip ()");

  *outSeqGrp  = SEQGRP_INV_CLIP;
  *outRestSeq = inSeq + ksd_strlen (keySeqTable[inIdx].keySeq);

  outSeqParam->Clip.mode = CLIP_MOD_Enable;

  return TRUE;
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : KSD                      |
| STATE   : code                  ROUTINE : ksd_seqIntrgtColr        |
+--------------------------------------------------------------------+

  PURPOSE : This function sets the ouput parameter for the
            SEQGRP_CL, KSD_OP_IRGT, KSD_SS_COLR key sequence.
*/
LOCAL BOOL ksd_seqIntrgtColr (USHORT          inIdx,
                              CHAR*           inSeq,
                              T_KSD_SEQGRP*   outSeqGrp,
                              CHAR**          outRestSeq,
                              T_KSD_SEQPARAM* outSeqParam)
{
  TRACE_FUNCTION ("ksd_seqIntrgtColr ()");

  outSeqParam->cb.opCd = KSD_OP_IRGT;
  outSeqParam->cb.ssCd = KSD_SS_COLR;

  *outSeqGrp  = SEQGRP_CL;

  *outRestSeq = inSeq + ksd_strlen (keySeqTable[inIdx].keySeq);

  return TRUE;
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : KSD                      |
| STATE   : code                  ROUTINE : ksd_seqSupColr           |
+--------------------------------------------------------------------+

  PURPOSE : This function sets the ouput parameter for the
            SEQGRP_SUP_COLR key sequence.
*/
LOCAL BOOL ksd_seqSupColr (USHORT          inIdx,
                           CHAR*           inSeq,
                           T_KSD_SEQGRP*   outSeqGrp,
                           CHAR**          outRestSeq,
                           T_KSD_SEQPARAM* outSeqParam)
{
  TRACE_FUNCTION ("ksd_seqSupColr ()");

  *outSeqGrp  = SEQGRP_SUP_COLR;
  *outRestSeq = inSeq + ksd_strlen (keySeqTable[inIdx].keySeq);

  return TRUE;
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : KSD                      |
| STATE   : code                  ROUTINE : ksd_seqInvColr           |
+--------------------------------------------------------------------+

  PURPOSE : This function sets the ouput parameter for the
            SEQGRP_INV_COLR key sequence.
*/
LOCAL BOOL ksd_seqInvColr (USHORT          inIdx,
                           CHAR*           inSeq,
                           T_KSD_SEQGRP*   outSeqGrp,
                           CHAR**          outRestSeq,
                           T_KSD_SEQPARAM* outSeqParam)
{
  TRACE_FUNCTION ("ksd_seqInvColr ()");

  *outSeqGrp  = SEQGRP_INV_COLR;
  *outRestSeq = inSeq + ksd_strlen (keySeqTable[inIdx].keySeq);

  return TRUE;
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : KSD                      |
| STATE   : code                  ROUTINE : ksd_seqIntrgtColp        |
+--------------------------------------------------------------------+

  PURPOSE : This function sets the ouput parameter for the
            SEQGRP_CL, KSD_OP_IRGT, KSD_SS_COLP key sequence.
*/
LOCAL BOOL ksd_seqIntrgtColp (USHORT          inIdx,
                              CHAR*           inSeq,
                              T_KSD_SEQGRP*   outSeqGrp,
                              CHAR**          outRestSeq,
                              T_KSD_SEQPARAM* outSeqParam)
{
  TRACE_FUNCTION ("ksd_seqIntrgtColp ()");

  outSeqParam->cb.opCd = KSD_OP_IRGT;
  outSeqParam->cb.ssCd = KSD_SS_COLP;

  *outSeqGrp  = SEQGRP_CL;

  *outRestSeq = inSeq + ksd_strlen (keySeqTable[inIdx].keySeq);

  return TRUE;
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : KSD                      |
| STATE   : code                  ROUTINE : ksd_seqSupColp           |
+--------------------------------------------------------------------+

  PURPOSE : This function sets the ouput parameter for the
            SEQGRP_SUP_COLP key sequence.
*/
LOCAL BOOL ksd_seqSupColp (USHORT          inIdx,
                           CHAR*           inSeq,
                           T_KSD_SEQGRP*   outSeqGrp,
                           CHAR**          outRestSeq,
                           T_KSD_SEQPARAM* outSeqParam)
{
  TRACE_FUNCTION ("ksd_seqSupColp ()");

  *outSeqGrp  = SEQGRP_SUP_COLP;
  *outRestSeq = inSeq + ksd_strlen (keySeqTable[inIdx].keySeq);

  outSeqParam->Colp.mode = COLP_MOD_Disable;

  return TRUE;
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : KSD                      |
| STATE   : code                  ROUTINE : ksd_seqInvColp           |
+--------------------------------------------------------------------+

  PURPOSE : This function sets the ouput parameter for the
            SEQGRP_INV_COLP key sequence.
*/
LOCAL BOOL ksd_seqInvColp (USHORT          inIdx,
                           CHAR*           inSeq,
                           T_KSD_SEQGRP*   outSeqGrp,
                           CHAR**          outRestSeq,
                           T_KSD_SEQPARAM* outSeqParam)
{
  TRACE_FUNCTION ("ksd_seqInvColp ()");

  *outSeqGrp  = SEQGRP_INV_COLP;
  *outRestSeq = inSeq + ksd_strlen (keySeqTable[inIdx].keySeq);

  outSeqParam->Colp.mode = COLP_MOD_Enable;

  return TRUE;
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : KSD                      |
| STATE   : code                  ROUTINE : ksd_seqSupTTY            |
+--------------------------------------------------------------------+

  PURPOSE : This function sets the ouput parameter for the
            SEQGRP_TTY_SERV key sequence 'Suppress TTY Service'.
*/
LOCAL BOOL ksd_seqSupTTY  (USHORT          inIdx,
                           CHAR*           inSeq,
                           T_KSD_SEQGRP*   outSeqGrp,
                           CHAR**          outRestSeq,
                           T_KSD_SEQPARAM* outSeqParam)
{
  TRACE_FUNCTION ("ksd_seqSupTTY ()");

  *outSeqGrp  = SEQGRP_TTY_SERV;
  *outRestSeq = inSeq + ksd_strlen (keySeqTable[inIdx].keySeq);

  outSeqParam->ctty.req = CTTY_REQ_Off;

  return TRUE;
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : KSD                      |
| STATE   : code                  ROUTINE : ksd_seqReqTTY            |
+--------------------------------------------------------------------+

  PURPOSE : This function sets the ouput parameter for the
            SEQGRP_TTY_SERV key sequence 'Request TTY Service'.
*/
LOCAL BOOL ksd_seqReqTTY  (USHORT          inIdx,
                           CHAR*           inSeq,
                           T_KSD_SEQGRP*   outSeqGrp,
                           CHAR**          outRestSeq,
                           T_KSD_SEQPARAM* outSeqParam)
{
  TRACE_FUNCTION ("ksd_seqInvTTY ()");

  *outSeqGrp  = SEQGRP_TTY_SERV;
  *outRestSeq = inSeq + ksd_strlen (keySeqTable[inIdx].keySeq);

  outSeqParam->ctty.req = CTTY_REQ_On;

  return TRUE;
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : KSD                      |
| STATE   : code                  ROUTINE : ksd_getCpwdFromTriplePw  |
+--------------------------------------------------------------------+

  PURPOSE : This function extracts the parameters of the sAT_PlusCPWD
            function of the ACI out of the supplementary information.

            <inIdx>:       index of key sequence table
            <inSeq>:       key sequence, to be decoded
            <outRestSeq>:  rest key sequence, to be decoded by a
                           further call to this function
            <outSeqParam>: sequence parameter

            returns:       TRUE if output parameters are valid,
                           otherwise FALSE
*/
LOCAL BOOL ksd_getCpwdFromTriplePw (USHORT          inIdx,
                                    CHAR*           inSeq,
                                    CHAR**          outRestSeq,
                                    T_KSD_SEQPARAM* outSeqParam)
{
  const USHORT   siNum     = 3;
  T_KSD_DCD_CPLE dcdCple[3];
  T_KSD_SIPARAM  oldPwParam;
  T_KSD_SIPARAM  newPw1Param;
  T_KSD_SIPARAM  newPw2Param;
  USHORT         readChars;
  USHORT         lenKeySeq = ksd_strlen (keyPasswd[inIdx].keySeq);
  BOOL           isSuccess = FALSE;

  TRACE_FUNCTION ("ksd_getCpwdFromTriplePw ()");

  dcdCple[0].type        = SI_CHAR_STAR;
  dcdCple[0].param       = &oldPwParam;
  oldPwParam.siCharStar  = NULL;

  dcdCple[1].type        = SI_CHAR_STAR;
  dcdCple[1].param       = &newPw1Param;
  newPw1Param.siCharStar = NULL;

  dcdCple[2].type        = SI_CHAR_STAR;
  dcdCple[2].param       = &newPw2Param;
  newPw2Param.siCharStar = NULL;

  if (ksd_extractSi (inSeq + lenKeySeq, siNum,
                     &readChars, dcdCple)      EQ TRUE)
  {
    outSeqParam->pwd.oldPwd = (UBYTE*)oldPwParam.siCharStar;
    outSeqParam->pwd.newPwd = (UBYTE*)newPw1Param.siCharStar;
    outSeqParam->pwd.newPwd2 = (UBYTE*)newPw2Param.siCharStar;

    isSuccess = TRUE;
  }

  *outRestSeq = inSeq + lenKeySeq + readChars;

  return isSuccess;
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : KSD                      |
| STATE   : code                  ROUTINE : ksd_seqRegPwdAllBarrServ |
+--------------------------------------------------------------------+

  PURPOSE : This function sets the ouput parameter for the
            SEQGRP_PWD, KSD_SS_ALL_CBSS key sequence.
*/
LOCAL BOOL ksd_seqRegPwdAllBarrServ (USHORT          inIdx,
                                     CHAR*           inSeq,
                                     T_KSD_SEQGRP*   outSeqGrp,
                                     CHAR**          outRestSeq,
                                     T_KSD_SEQPARAM* outSeqParam)
{
  TRACE_FUNCTION ("ksd_seqRegPwdAllBarrServ ()");

  outSeqParam->pwd.ssCd = KSD_SS_ALL_CBSS;

  *outSeqGrp  = SEQGRP_PWD;

  return ksd_getCpwdFromTriplePw (inIdx, inSeq, outRestSeq, outSeqParam);
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : KSD                      |
| STATE   : code                  ROUTINE : ksd_seqRegPwdAllServ     |
+--------------------------------------------------------------------+

  PURPOSE : This function sets the ouput parameter for the
            SEQGRP_PWD, KSD_SS_... key sequence.
*/
LOCAL BOOL ksd_seqRegPwdAllServ (USHORT          inIdx,
                                 CHAR*           inSeq,
                                 T_KSD_SEQGRP*   outSeqGrp,
                                 CHAR**          outRestSeq,
                                 T_KSD_SEQPARAM* outSeqParam)
{
  TRACE_FUNCTION ("ksd_seqRegPwdAllServ ()");

  outSeqParam->pwd.ssCd = KSD_SS_ALL_SERV;

  *outSeqGrp  = SEQGRP_PWD;

  return ksd_getCpwdFromTriplePw (inIdx, inSeq, outRestSeq, outSeqParam);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : KSD                      |
| STATE   : code                  ROUTINE : ksd_seqChngPin2          |
+--------------------------------------------------------------------+

  PURPOSE : This function sets the ouput parameter for the
            SEQGRP_PWD, KSD_SS_PIN2 key sequence.
*/
LOCAL BOOL ksd_seqChngPin2 (USHORT          inIdx,
                            CHAR*           inSeq,
                            T_KSD_SEQGRP*   outSeqGrp,
                            CHAR**          outRestSeq,
                            T_KSD_SEQPARAM* outSeqParam)
{
  TRACE_FUNCTION ("ksd_seqChngPin2 ()");

  outSeqParam->pwd.ssCd = KSD_SS_PIN2;

  *outSeqGrp  = SEQGRP_PWD;

  return ksd_getCpwdFromTriplePw (inIdx, inSeq, outRestSeq, outSeqParam);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : KSD                      |
| STATE   : code                  ROUTINE : ksd_seqChngPin           |
+--------------------------------------------------------------------+

  PURPOSE : This function sets the ouput parameter for the
            SEQGRP_PWD, KSD_SS_PIN1 key sequence.
*/
LOCAL BOOL ksd_seqChngPin (USHORT          inIdx,
                           CHAR*           inSeq,
                           T_KSD_SEQGRP*   outSeqGrp,
                           CHAR**          outRestSeq,
                           T_KSD_SEQPARAM* outSeqParam)
{
  TRACE_FUNCTION ("ksd_seqChngPin ()");

  outSeqParam->pwd.ssCd = KSD_SS_PIN1;

  *outSeqGrp  = SEQGRP_PWD;

  return ksd_getCpwdFromTriplePw (inIdx, inSeq, outRestSeq, outSeqParam);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : KSD                      |
| STATE   : code                  ROUTINE : ksd_getUblkFromTriplePw  |
+--------------------------------------------------------------------+

  PURPOSE : This function extracts the parameters of the sAT_PlusCPWD
            function of the ACI out of the supplementary information.

            <inIdx>:       index of key sequence table
            <inSeq>:       key sequence, to be decoded
            <outRestSeq>:  rest key sequence, to be decoded by a
                           further call to this function
            <outSeqParam>: sequence parameter

            returns:       TRUE if output parameters are valid,
                           otherwise FALSE
*/
LOCAL BOOL ksd_getUblkFromTriplePw (USHORT          inIdx,
                                    CHAR*           inSeq,
                                    CHAR**          outRestSeq,
                                    T_KSD_SEQPARAM* outSeqParam)
{
  const USHORT   siNum     = 3;
  T_KSD_DCD_CPLE dcdCple[3];
  T_KSD_SIPARAM  oldPwParam;
  T_KSD_SIPARAM  newPw1Param;
  T_KSD_SIPARAM  newPw2Param;
  USHORT         readChars;
  USHORT         lenKeySeq = ksd_strlen (keyPasswd[inIdx].keySeq);
  BOOL           isSuccess = FALSE;

  TRACE_FUNCTION ("ksd_getUblkFromTriplePw ()");

  dcdCple[0].type        = SI_CHAR_STAR;
  dcdCple[0].param       = &oldPwParam;
  oldPwParam.siCharStar  = NULL;

  dcdCple[1].type        = SI_CHAR_STAR;
  dcdCple[1].param       = &newPw1Param;
  newPw1Param.siCharStar = NULL;

  dcdCple[2].type        = SI_CHAR_STAR;
  dcdCple[2].param       = &newPw2Param;
  newPw2Param.siCharStar = NULL;

  if (ksd_extractSi (inSeq + lenKeySeq, siNum,
                     &readChars, dcdCple)      EQ TRUE)
  {
    outSeqParam->ublk.puk = (UBYTE*)oldPwParam.siCharStar;
    outSeqParam->ublk.pin = (UBYTE*)newPw1Param.siCharStar;

    /*
     *---------------------------------------------------------------
     * the new password must be given twice
     *---------------------------------------------------------------
     */
    if (!strcmp ((CHAR*)outSeqParam->ublk.pin, newPw2Param.siCharStar))
      isSuccess = TRUE;
    else
      isSuccess = FALSE;
  }

  *outRestSeq = inSeq + lenKeySeq + readChars;

  return isSuccess;
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : KSD                      |
| STATE   : code                  ROUTINE : ksd_seqUnblckPin2        |
+--------------------------------------------------------------------+

  PURPOSE : This function sets the ouput parameter for the
            SEQGRP_UBLK, KSD_SS_PIN2 key sequence.
*/
LOCAL BOOL ksd_seqUnblckPin2 (USHORT          inIdx,
                              CHAR*           inSeq,
                              T_KSD_SEQGRP*   outSeqGrp,
                              CHAR**          outRestSeq,
                              T_KSD_SEQPARAM* outSeqParam)
{
  TRACE_FUNCTION ("ksd_seqUnblckPin2 ()");

  outSeqParam->pwd.ssCd = KSD_SS_PIN2;

  *outSeqGrp  = SEQGRP_UBLK;

  return ksd_getUblkFromTriplePw (inIdx, inSeq, outRestSeq, outSeqParam);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : KSD                      |
| STATE   : code                  ROUTINE : ksd_seqUnblckPin         |
+--------------------------------------------------------------------+

  PURPOSE : This function sets the ouput parameter for the
            SEQGRP_UBLK, KSD_SS_PIN1 key sequence.
*/
LOCAL BOOL ksd_seqUnblckPin (USHORT          inIdx,
                             CHAR*           inSeq,
                             T_KSD_SEQGRP*   outSeqGrp,
                             CHAR**          outRestSeq,
                             T_KSD_SEQPARAM* outSeqParam)
{
  TRACE_FUNCTION ("ksd_seqUnblckPin ()");

  outSeqParam->pwd.ssCd = KSD_SS_PIN1;

  *outSeqGrp  = SEQGRP_UBLK;

  return ksd_getUblkFromTriplePw (inIdx, inSeq, outRestSeq, outSeqParam);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : KSD                      |
| STATE   : code                  ROUTINE : ksd_getCcwaFromBs        |
+--------------------------------------------------------------------+

  PURPOSE : This function extracts the parameters of the sAT_PlusCCWA
            function of the ACI out of the supplementary information.

            <inIdx>:       index of key sequence table
            <inSeq>:       key sequence, to be decoded
            <outRestSeq>:  rest key sequence, to be decoded by a
                           further call to this function
            <outSeqParam>: sequence parameter

            returns:       TRUE if output parameters are valid,
                           otherwise FALSE
*/
LOCAL BOOL ksd_getCcwaFromBs (USHORT          inIdx,
                              CHAR*           inSeq,
                              CHAR**          outRestSeq,
                              T_KSD_SEQPARAM* outSeqParam)
{
  const USHORT   siNum     = 1;
  T_KSD_DCD_CPLE dcdCple[1];
  T_KSD_SIPARAM  bsParam;
  USHORT         readChars;
  USHORT         lenKeySeq = ksd_strlen (keySeqTable[inIdx].keySeq);
  BOOL           isSuccess = FALSE;

  TRACE_FUNCTION ("ksd_getCcwaFromBs ()");

  dcdCple[0].type  = SI_UBYTE;
  dcdCple[0].param = &bsParam;
  bsParam.siUbyte  = KSD_BS_None;

  if (ksd_extractSi (inSeq + lenKeySeq, siNum,
                     &readChars, dcdCple)      EQ TRUE)
  {
    outSeqParam->cw.bsCd = bsParam.siUbyte;

    isSuccess = TRUE;
  }

  *outRestSeq = inSeq + lenKeySeq + readChars;

  return isSuccess;
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : KSD                      |
| STATE   : code                  ROUTINE : ksd_seqActWait           |
+--------------------------------------------------------------------+

  PURPOSE : This function sets the ouput parameter for the
            SEQGRP_CW, KSD_OP_ACT key sequence.
*/
LOCAL BOOL ksd_seqActWait (USHORT          inIdx,
                           CHAR*           inSeq,
                           T_KSD_SEQGRP*   outSeqGrp,
                           CHAR**          outRestSeq,
                           T_KSD_SEQPARAM* outSeqParam)
{
  TRACE_FUNCTION ("ksd_seqActWait ()");

  outSeqParam->cw.opCd = KSD_OP_ACT;

  *outSeqGrp = SEQGRP_CW;

  return ksd_getCcwaFromBs (inIdx, inSeq, outRestSeq, outSeqParam);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : KSD                      |
| STATE   : code                  ROUTINE : ksd_seqDeactWait         |
+--------------------------------------------------------------------+

  PURPOSE : This function sets the ouput parameter for the
            SEQGRP_CW, KSD_OP_DEACT key sequence.
*/
LOCAL BOOL ksd_seqDeactWait (USHORT          inIdx,
                             CHAR*           inSeq,
                             T_KSD_SEQGRP*   outSeqGrp,
                             CHAR**          outRestSeq,
                             T_KSD_SEQPARAM* outSeqParam)
{
  TRACE_FUNCTION ("ksd_seqDeactWait ()");

  outSeqParam->cw.opCd = KSD_OP_DEACT;

  *outSeqGrp = SEQGRP_CW;

  return ksd_getCcwaFromBs (inIdx, inSeq, outRestSeq, outSeqParam);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : KSD                      |
| STATE   : code                  ROUTINE : ksd_seqIntrgtWait        |
+--------------------------------------------------------------------+

  PURPOSE : This function sets the ouput parameter for the
            SEQGRP_CW, KSD_OP_IRGT key sequence.
*/
LOCAL BOOL ksd_seqIntrgtWait (USHORT          inIdx,
                              CHAR*           inSeq,
                              T_KSD_SEQGRP*   outSeqGrp,
                              CHAR**          outRestSeq,
                              T_KSD_SEQPARAM* outSeqParam)
{
  TRACE_FUNCTION ("ksd_seqIntrgtWait ()");

  outSeqParam->cw.opCd = KSD_OP_IRGT;
//TISH, hot fix for OMAPS00130692
//start
#if 0  
  /* Interrogation of Call Waiting is always without any Basic Service Code, see 4.83 */
  outSeqParam->cw.bsCd = KSD_BS_None;

  *outSeqGrp = SEQGRP_CW;

  *outRestSeq = inSeq + ksd_strlen (keySeqTable[inIdx].keySeq);

  return TRUE;
#else
  *outSeqGrp = SEQGRP_CW;
  return ksd_getCcwaFromBs (inIdx, inSeq, outRestSeq, outSeqParam);
#endif
//end  
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : KSD                      |
| STATE   : code                  ROUTINE : ksd_seqIPrsntImei        |
+--------------------------------------------------------------------+

  PURPOSE : This function sets the ouput parameter for the
            SEQGRP_PRSNT_IMEI key sequence.
*/
LOCAL BOOL ksd_seqPrsntImei (USHORT          inIdx,
                             CHAR*           inSeq,
                             T_KSD_SEQGRP*   outSeqGrp,
                             CHAR**          outRestSeq,
                             T_KSD_SEQPARAM* outSeqParam)
{
  TRACE_FUNCTION ("ksd_seqPrsntImei ()");

  *outSeqGrp = SEQGRP_PRSNT_IMEI;
  *outRestSeq = inSeq + ksd_strlen (keySeqTable[inIdx].keySeq);

  return TRUE;
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : KSD                      |
| STATE   : code                  ROUTINE : ksd_seqIntrgtCCBS        |
+--------------------------------------------------------------------+

  PURPOSE : This function sets the ouput parameter for the
            SEQGRP_CCBS, KSD_OP_IRGT, KSD_SS_CCBS key sequence.
*/
LOCAL BOOL ksd_seqIntrgtCCBS (USHORT          inIdx,
                              CHAR*           inSeq,
                              T_KSD_SEQGRP*   outSeqGrp,
                              CHAR**          outRestSeq,
                              T_KSD_SEQPARAM* outSeqParam)
{
  TRACE_FUNCTION ("ksd_seqIntrgtCCBS ()");

  outSeqParam->ccbs.opCd = KSD_OP_IRGT;
  outSeqParam->ccbs.idx  = KSD_IDX_NONE;

  *outSeqGrp  = SEQGRP_CCBS;

  *outRestSeq = inSeq + ksd_strlen (keySeqTable[inIdx].keySeq);

  return TRUE;
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : KSD                      |
| STATE   : code                  ROUTINE : ksd_getCCBSFromN         |
+--------------------------------------------------------------------+

  PURPOSE : This function extracts the parameters of the sAT_PercentCCBS
            function of the ACI out of the supplementary information.

            <inIdx>:       index of key sequence table
            <inSeq>:       key sequence, to be decoded
            <outRestSeq>:  rest key sequence, to be decoded by a
                           further call to this function
            <outSeqParam>: sequence parameter

            returns:       TRUE if output parameters are valid,
                           otherwise FALSE
*/
LOCAL BOOL ksd_getCCBSFromN (USHORT          inIdx,
                              CHAR*           inSeq,
                              CHAR**          outRestSeq,
                              T_KSD_SEQPARAM* outSeqParam)
{
  const USHORT   siNum     = 1;
  T_KSD_DCD_CPLE dcdCple[1];
  T_KSD_SIPARAM  nParam;
  USHORT         readChars;
  USHORT         lenKeySeq = ksd_strlen (keySeqTable[inIdx].keySeq);
  BOOL           isSuccess = FALSE;

  TRACE_FUNCTION ("ksd_getCCBSFromN ()");

  dcdCple[0].type  = SI_UBYTE;
  dcdCple[0].param = &nParam;
  nParam.siUbyte  = KSD_IDX_NONE;

  if (ksd_extractSi (inSeq + lenKeySeq, siNum,
                     &readChars, dcdCple)      EQ TRUE)
  {
    outSeqParam->ccbs.idx = nParam.siUbyte;

    isSuccess = TRUE;
  }

  *outRestSeq = inSeq + lenKeySeq + readChars;

  return isSuccess;
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : KSD                      |
| STATE   : code                  ROUTINE : ksd_seqDeactCCBS         |
+--------------------------------------------------------------------+

  PURPOSE : This function sets the ouput parameter for the
            SEQGRP_CCBS, KSD_OP_DEACT key sequence.
*/
LOCAL BOOL ksd_seqDeactCCBS (USHORT          inIdx,
                             CHAR*           inSeq,
                             T_KSD_SEQGRP*   outSeqGrp,
                             CHAR**          outRestSeq,
                             T_KSD_SEQPARAM* outSeqParam)
{
  TRACE_FUNCTION ("ksd_seqDeactCCBS ()");

  outSeqParam->ccbs.opCd = KSD_OP_DEACT;
  outSeqParam->ccbs.idx  = KSD_IDX_NONE;

  *outSeqGrp = SEQGRP_CCBS;

  return ksd_getCCBSFromN (inIdx, inSeq, outRestSeq, outSeqParam);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : KSD                      |
| STATE   : code                  ROUTINE : ksd_seqIntrgtCNAP        |
+--------------------------------------------------------------------+

  PURPOSE : This function sets the ouput parameter for the
            SEQGRP_CL, KSD_OP_IRGT, KSD_SS_CNAP key sequence.
*/
LOCAL BOOL ksd_seqIntrgtCNAP (USHORT          inIdx,
                              CHAR*           inSeq,
                              T_KSD_SEQGRP*   outSeqGrp,
                              CHAR**          outRestSeq,
                              T_KSD_SEQPARAM* outSeqParam)
{
  TRACE_FUNCTION ("ksd_seqIntrgtCNAP ()");

  outSeqParam->cb.opCd = KSD_OP_IRGT;
  outSeqParam->cb.ssCd = KSD_SS_CNAP;

  *outSeqGrp  = SEQGRP_CL;

  *outRestSeq = inSeq + ksd_strlen (keySeqTable[inIdx].keySeq);

  return TRUE;
}

#if 0  /* For further study, so not yet used */
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : KSD                      |
| STATE   : code                  ROUTINE : ksd_seqActCCBS           |
+--------------------------------------------------------------------+

  PURPOSE : This function sets the ouput parameter for the
            SEQGRP_CCBS, KSD_OP_ACT key sequence.

    
LOCAL BOOL ksd_seqActCCBS (USHORT          inIdx,
                           CHAR*           inSeq,
                           T_KSD_SEQGRP*   outSeqGrp,
                           CHAR**          outRestSeq,
                           T_KSD_SEQPARAM* outSeqParam)
{
  TRACE_FUNCTION ("ksd_seqActCCBS ()");

  outSeqParam->ccbs.opCd = KSD_OP_ACT;
  outSeqParam->ccbs.idx  = KSD_IDX_NONE;

  *outSeqGrp = SEQGRP_CCBS;

  *outRestSeq = inSeq + ksd_strlen (keySeqTable[inIdx].keySeq);

  return TRUE;
}
#endif


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : KSD                      |
| STATE   : code                  ROUTINE : ksd_seqSndChld0          |
+--------------------------------------------------------------------+

  PURPOSE : This function sets the ouput parameter for the
            SEQGRP_CHLD key sequence.
*/
LOCAL BOOL ksd_seqSndChld0 (USHORT          inIdx,
                            CHAR*           inSeq,
                            T_KSD_SEQGRP*   outSeqGrp,
                            CHAR**          outRestSeq,
                            T_KSD_SEQPARAM* outSeqParam)
{
  BOOL   isSuccess = FALSE;
  USHORT lenInSeq  = strlen (inSeq);
  USHORT lenKeySeq = ksd_strlen (keyWithinCall[inIdx].keySeq);

  TRACE_FUNCTION ("ksd_seqSndChld0 ()");

  /*
   *-----------------------------------------------------------------
   * entering a single '0' is a valid +CHLD parameter
   *-----------------------------------------------------------------
   */
  if (lenInSeq EQ lenKeySeq)
  {
    *outSeqGrp             = SEQGRP_CHLD;
    *outRestSeq            = inSeq + lenInSeq;
    outSeqParam->chld.mode = CHLD_MOD_RelHldOrUdub;
    outSeqParam->chld.call = NULL;

    isSuccess = TRUE;
  }
  else
  {
    *outSeqGrp  = SEQGRP_UNKNOWN;
    *outRestSeq = inSeq;
  }

  return isSuccess;
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : KSD                      |
| STATE   : code                  ROUTINE : ksd_seqSndChld1          |
+--------------------------------------------------------------------+

  PURPOSE : This function sets the ouput parameter for the
            SEQGRP_CHLD key sequence.
*/
LOCAL BOOL ksd_seqSndChld1 (USHORT          inIdx,
                            CHAR*           inSeq,
                            T_KSD_SEQGRP*   outSeqGrp,
                            CHAR**          outRestSeq,
                            T_KSD_SEQPARAM* outSeqParam)
{
  BOOL   isSuccess = FALSE;
  USHORT lenInSeq  = strlen (inSeq);
  USHORT lenKeySeq = ksd_strlen (keyWithinCall[inIdx].keySeq);

  TRACE_FUNCTION ("ksd_seqSndChld1 ()");

  if (lenInSeq EQ lenKeySeq)
  {
    /*
     *---------------------------------------------------------------
     * entering a single '1' is a valid +CHLD parameter
     *---------------------------------------------------------------
     */
    *outSeqGrp             = SEQGRP_CHLD;
    *outRestSeq            = inSeq + lenInSeq;
    outSeqParam->chld.mode = CHLD_MOD_RelActAndAcpt;
    outSeqParam->chld.call = NULL;

    isSuccess = TRUE;
  }
  else if (lenInSeq EQ lenKeySeq + 1)
  {
    if (*(inSeq + lenKeySeq) >= '0' AND
        *(inSeq + lenKeySeq) <= '9')
    {
      /*
       *---------------------------------------------------------------
       * entering a '1X' is a valid +CHLD parameter, while X is a
       * number between '0' and '9'
       *---------------------------------------------------------------
       */
      *outSeqGrp             = SEQGRP_CHLD;
      *outRestSeq            = inSeq + lenInSeq;
      outSeqParam->chld.mode = CHLD_MOD_RelActSpec;
      outSeqParam->chld.call = inSeq + lenKeySeq;

      isSuccess = TRUE;
    }
    else
    {
      *outSeqGrp  = SEQGRP_UNKNOWN;
      *outRestSeq = inSeq;
    }
  }
  else
  {
    *outSeqGrp  = SEQGRP_UNKNOWN;
    *outRestSeq = inSeq;
  }

  return isSuccess;
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : KSD                      |
| STATE   : code                  ROUTINE : ksd_seqSndChld2          |
+--------------------------------------------------------------------+

  PURPOSE : This function sets the ouput parameter for the
            SEQGRP_CHLD key sequence.
*/
LOCAL BOOL ksd_seqSndChld2 (USHORT          inIdx,
                            CHAR*           inSeq,
                            T_KSD_SEQGRP*   outSeqGrp,
                            CHAR**          outRestSeq,
                            T_KSD_SEQPARAM* outSeqParam)
{
  BOOL   isSuccess = FALSE;
  USHORT lenInSeq  = strlen (inSeq);
  USHORT lenKeySeq = ksd_strlen (keyWithinCall[inIdx].keySeq);

  TRACE_FUNCTION ("ksd_seqSndChld2 ()");

  if (lenInSeq EQ lenKeySeq)
  {
    /*
     *---------------------------------------------------------------
     * entering a single '2' is a valid +CHLD parameter
     *---------------------------------------------------------------
     */
    *outSeqGrp             = SEQGRP_CHLD;
    *outRestSeq            = inSeq + lenInSeq;
    outSeqParam->chld.mode = CHLD_MOD_HldActAndAcpt;
    outSeqParam->chld.call = NULL;

    isSuccess = TRUE;
  }
  else if (lenInSeq EQ lenKeySeq + 1)
  {
    if (*(inSeq + lenKeySeq) >= '0' AND
        *(inSeq + lenKeySeq) <= '9')
    {
      /*
       *---------------------------------------------------------------
       * entering a '2X' is a valid +CHLD parameter, while X is a
       * number between '0' and '9'
       *---------------------------------------------------------------
       */
      *outSeqGrp             = SEQGRP_CHLD;
      *outRestSeq            = inSeq + lenInSeq;
      outSeqParam->chld.mode = CHLD_MOD_HldActExc;
      outSeqParam->chld.call = inSeq + lenKeySeq;

      isSuccess = TRUE;
    }
    else
    {
      *outSeqGrp  = SEQGRP_UNKNOWN;
      *outRestSeq = inSeq;
    }
  }
  else
  {
    *outSeqGrp  = SEQGRP_UNKNOWN;
    *outRestSeq = inSeq;
  }

  return isSuccess;
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : KSD                      |
| STATE   : code                  ROUTINE : ksd_seqSndChld3          |
+--------------------------------------------------------------------+

  PURPOSE : This function sets the ouput parameter for the
            SEQGRP_CHLD key sequence.
*/
LOCAL BOOL ksd_seqSndChld3 (USHORT          inIdx,
                            CHAR*           inSeq,
                            T_KSD_SEQGRP*   outSeqGrp,
                            CHAR**          outRestSeq,
                            T_KSD_SEQPARAM* outSeqParam)
{
  BOOL   isSuccess = FALSE;
  USHORT lenInSeq  = strlen (inSeq);
  USHORT lenKeySeq = ksd_strlen (keyWithinCall[inIdx].keySeq);

  TRACE_FUNCTION ("ksd_seqSndChld3 ()");

  /*
   *-----------------------------------------------------------------
   * entering a single '3' is a valid +CHLD parameter
   *-----------------------------------------------------------------
   */
  if (lenInSeq EQ lenKeySeq)
  {
    *outSeqGrp             = SEQGRP_CHLD;
    *outRestSeq            = inSeq + lenInSeq;
    outSeqParam->chld.mode = CHLD_MOD_AddHld;
    outSeqParam->chld.call = NULL;

    isSuccess = TRUE;
  }
  else
  {
    *outSeqGrp  = SEQGRP_UNKNOWN;
    *outRestSeq = inSeq;
  }

  return isSuccess;
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : KSD                      |
| STATE   : code                  ROUTINE : ksd_seqSndChld4          |
+--------------------------------------------------------------------+

  PURPOSE : This function sets the ouput parameter for the
            SEQGRP_CHLD key sequence.
*/
LOCAL BOOL ksd_seqSndChld4 (USHORT          inIdx,
                            CHAR*           inSeq,
                            T_KSD_SEQGRP*   outSeqGrp,
                            CHAR**          outRestSeq,
                            T_KSD_SEQPARAM* outSeqParam)
{
  BOOL   isSuccess = FALSE;
  USHORT lenInSeq  = strlen (inSeq);
  USHORT lenKeySeq = ksd_strlen (keyWithinCall[inIdx].keySeq);

  TRACE_FUNCTION ("ksd_seqSndChld4()");

  /*
   *-----------------------------------------------------------------
   * entering a single '4' is a valid +CHLD parameter
   *-----------------------------------------------------------------
   */
  if (lenInSeq EQ lenKeySeq)
  {
    *outSeqGrp             = SEQGRP_CHLD;
    *outRestSeq            = inSeq + lenInSeq;
    outSeqParam->chld.mode = CHLD_MOD_Ect;
    outSeqParam->chld.call = NULL;

    isSuccess = TRUE;
  }

  return isSuccess;
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : KSD                      |
| STATE   : code                  ROUTINE : ksd_seqSndChld4Star      |
+--------------------------------------------------------------------+

  PURPOSE : This function sets the ouput parameter for the
            SEQGRP_CHLD key sequence.
*/
LOCAL BOOL ksd_seqSndChld4Star (USHORT          inIdx,
                                CHAR*           inSeq,
                                T_KSD_SEQGRP*   outSeqGrp,
                                CHAR**          outRestSeq,
                                T_KSD_SEQPARAM* outSeqParam)
{
  USHORT lenInSeq  = strlen (inSeq);
  USHORT lenKeySeq = ksd_strlen (keyWithinCall[inIdx].keySeq);
  USHORT lenSubaddr;
  BOOL   international;
  BOOL   isSuccess = FALSE;

  TRACE_FUNCTION ("ksd_seqSndChld4Star()");

  /*
   *-----------------------------------------------------------------
   * entering a '4' and a '*' is a valid +CTFR parameter
   *-----------------------------------------------------------------
   */
  *outSeqGrp             = SEQGRP_CTFR;
  *outRestSeq            = inSeq + lenKeySeq;

  /*
   *-----------------------------------------------------------------
   * process dial DeflectedToNumber
   *-----------------------------------------------------------------
   */
  utl_splitDialnumber (*outRestSeq, 
                       &outSeqParam->ctfr.number,
                       &international,
                       &outSeqParam->ctfr.subaddr,
                       &lenSubaddr);

  /*
   *-----------------------------------------------------------------
   * process main address and type of main address
   *-----------------------------------------------------------------
   */
  if (outSeqParam->ctfr.number EQ NULL)
  {
    outSeqParam->ctfr.type.ton = TON_Unknown;
    outSeqParam->ctfr.type.npi = NPI_Unknown;
  }
  else
  {
    outSeqParam->ctfr.type.ton = (international) ? TON_International : 
                                                   TON_Unknown;
    outSeqParam->ctfr.type.npi = NPI_IsdnTelephony;
    isSuccess = TRUE;
  }

  /*
   *---------------------------------------------------------------
   * process subaddress and type of subaddress
   *---------------------------------------------------------------
   */
  if (outSeqParam->ctfr.subaddr EQ NULL)
  {
    outSeqParam->ctfr.satype.tos = TOS_Nsap;
    outSeqParam->ctfr.satype.oe  = OE_Even;
  }
  else
  {
    outSeqParam->ctfr.satype.tos = TOS_Nsap;
    outSeqParam->ctfr.satype.oe  = ((lenSubaddr & 1) EQ 0) ? OE_Even : 
                                                             OE_Odd;
  }

  *outRestSeq = *outRestSeq + strlen (*outRestSeq);

  return isSuccess;
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : KSD                      |
| STATE   : code                  ROUTINE : ksd_seqSndChld5          |
+--------------------------------------------------------------------+

  PURPOSE : This function sets the ouput parameter for the
            SEQGRP_CHLD key sequence.
*/
LOCAL BOOL ksd_seqSndChld5 (USHORT          inIdx,
                            CHAR*           inSeq,
                            T_KSD_SEQGRP*   outSeqGrp,
                            CHAR**          outRestSeq,
                            T_KSD_SEQPARAM* outSeqParam)
{
  BOOL   isSuccess = FALSE;
  USHORT lenInSeq  = strlen (inSeq);
  USHORT lenKeySeq = ksd_strlen (keyWithinCall[inIdx].keySeq);

  TRACE_FUNCTION ("ksd_seqSndChld5()");

  /*
   *-----------------------------------------------------------------
   * entering a single '5' is a valid +CHLD parameter
   *-----------------------------------------------------------------
   */
  if (lenInSeq EQ lenKeySeq)
  {
    *outSeqGrp             = SEQGRP_CHLD;
    *outRestSeq            = inSeq + lenInSeq;
    outSeqParam->chld.mode = CHLD_MOD_Ccbs;
    outSeqParam->chld.call = NULL;

    isSuccess = TRUE;
  }

  return isSuccess;
}

#ifdef SMI
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : KSD                      |
| STATE   : code                  ROUTINE : ksd_seqSndCbst           |
+--------------------------------------------------------------------+

  PURPOSE : This function sets the ouput parameter for the
            SEQGRP_SND_CBST key sequence.
*/
LOCAL BOOL ksd_seqSndCbst (USHORT          inIdx,
                           CHAR*           inSeq,
                           T_KSD_SEQGRP*   outSeqGrp,
                           CHAR**          outRestSeq,
                           T_KSD_SEQPARAM* outSeqParam)
{
  const USHORT   siNum     = 3;
  T_KSD_DCD_CPLE dcdCple[3];
  T_KSD_SIPARAM  speedParam;
  T_KSD_SIPARAM  nameParam;
  T_KSD_SIPARAM  ceParam;
  USHORT         readChars;
  USHORT         lenKeySeq = ksd_strlen (keySeqTableNonGsm[inIdx].keySeq);
  BOOL           isSuccess = FALSE;

  TRACE_FUNCTION ("ksd_seqSndCbst ()");

  dcdCple[0].type  = SI_SHORT;
  dcdCple[0].param = &speedParam;

  dcdCple[1].type  = SI_SHORT;
  dcdCple[1].param = &nameParam;

  dcdCple[2].type  = SI_SHORT;
  dcdCple[2].param = &ceParam;

  *outSeqGrp  = SEQGRP_SND_CBST;

  if (ksd_extractSi (inSeq + lenKeySeq, siNum,
                     &readChars, dcdCple)      EQ TRUE)
  {
    outSeqParam->cbst.speed = speedParam.siShort;
    outSeqParam->cbst.name  = nameParam.siShort;
    outSeqParam->cbst.ce    = ceParam.siShort;

    isSuccess = TRUE;
  }

  *outRestSeq = inSeq + lenKeySeq + readChars;

  return isSuccess;
}
#endif

#ifdef SMI
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : KSD                      |
| STATE   : code                  ROUTINE : ksd_seqSndCrlp           |
+--------------------------------------------------------------------+

  PURPOSE : This function sets the ouput parameter for the
            SEQGRP_SND_CRLP key sequence.
*/
LOCAL BOOL ksd_seqSndCrlp (USHORT          inIdx,
                           CHAR*           inSeq,
                           T_KSD_SEQGRP*   outSeqGrp,
                           CHAR**          outRestSeq,
                           T_KSD_SEQPARAM* outSeqParam)
{
  const USHORT   siNum     = 4;
  T_KSD_DCD_CPLE dcdCple[4];
  T_KSD_SIPARAM  iwsParam;
  T_KSD_SIPARAM  mwsParam;
  T_KSD_SIPARAM  t1Param;
  T_KSD_SIPARAM  n2Param;
  USHORT         readChars;
  USHORT         lenKeySeq = ksd_strlen (keySeqTableNonGsm[inIdx].keySeq);
  BOOL           isSuccess = FALSE;

  TRACE_FUNCTION ("ksd_seqSndCrlp ()");

  dcdCple[0].type  = SI_SHORT;
  dcdCple[0].param = &iwsParam;

  dcdCple[1].type  = SI_SHORT;
  dcdCple[1].param = &mwsParam;

  dcdCple[2].type  = SI_SHORT;
  dcdCple[2].param = &t1Param;

  dcdCple[3].type  = SI_SHORT;
  dcdCple[3].param = &n2Param;

  *outSeqGrp  = SEQGRP_SND_CRLP;

  if (ksd_extractSi (inSeq + lenKeySeq, siNum,
                     &readChars, dcdCple)      EQ TRUE)
  {
    outSeqParam->crlp.iws = iwsParam.siShort;
    outSeqParam->crlp.mws = mwsParam.siShort;
    outSeqParam->crlp.t1  = t1Param.siShort;
    outSeqParam->crlp.n2  = n2Param.siShort;

    isSuccess = TRUE;
  }

  *outRestSeq = inSeq + lenKeySeq + readChars;

  return isSuccess;
}
#endif

#ifdef SMI
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : KSD                      |
| STATE   : code                  ROUTINE : ksd_seqSndDs             |
+--------------------------------------------------------------------+

  PURPOSE : This function sets the ouput parameter for the
            SEQGRP_SND_DS key sequence.
*/
LOCAL BOOL ksd_seqSndDs (USHORT          inIdx,
                         CHAR*           inSeq,
                         T_KSD_SEQGRP*   outSeqGrp,
                         CHAR**          outRestSeq,
                         T_KSD_SEQPARAM* outSeqParam)
{
  const USHORT   siNum     = 4;
  T_KSD_DCD_CPLE dcdCple[4];
  T_KSD_SIPARAM  dirParam;
  T_KSD_SIPARAM  compParam;
  T_KSD_SIPARAM  maxDictParam;
  T_KSD_SIPARAM  maxStrParam;
  USHORT         readChars;
  USHORT         lenKeySeq = ksd_strlen (keySeqTableNonGsm[inIdx].keySeq);
  BOOL           isSuccess = FALSE;

  TRACE_FUNCTION ("ksd_seqSndDs ()");

  dcdCple[0].type  = SI_SHORT;
  dcdCple[0].param = &dirParam;

  dcdCple[1].type  = SI_SHORT;
  dcdCple[1].param = &compParam;

  dcdCple[2].type  = SI_LONG;
  dcdCple[2].param = &maxDictParam;

  dcdCple[3].type  = SI_SHORT;
  dcdCple[3].param = &maxStrParam;

  *outSeqGrp  = SEQGRP_SND_DS;

  if (ksd_extractSi (inSeq + lenKeySeq, siNum,
                     &readChars, dcdCple)      EQ TRUE)
  {
    outSeqParam->ds.dir     = dirParam.siShort;
    outSeqParam->ds.comp    = compParam.siShort;
    outSeqParam->ds.maxDict = maxDictParam.siLong;
    outSeqParam->ds.maxStr  = maxStrParam.siShort;

    isSuccess = TRUE;
  }

  *outRestSeq = inSeq + lenKeySeq + readChars;

  return isSuccess;
}
#endif

#ifdef SMI
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : KSD                      |
| STATE   : code                  ROUTINE : ksd_seqSpeechData        |
+--------------------------------------------------------------------+

  PURPOSE : This function sets the ouput parameter for the
            SEQGRP_SPEECH_DATA key sequence.
*/
LOCAL BOOL ksd_seqSpeechData (USHORT          inIdx,
                              CHAR*           inSeq,
                              T_KSD_SEQGRP*   outSeqGrp,
                              CHAR**          outRestSeq,
                              T_KSD_SEQPARAM* outSeqParam)
{
  TRACE_FUNCTION ("ksd_seqSpeechData ()");

  *outSeqGrp  = SEQGRP_SPEECH_DATA;
  *outRestSeq = inSeq + ksd_strlen(SPEECH_DATA_SEQ);

  return TRUE;
}
#endif

#ifdef SMI
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : KSD                      |
| STATE   : code                  ROUTINE : ksd_seqDataSpeech        |
+--------------------------------------------------------------------+

  PURPOSE : This function sets the ouput parameter for the
            SEQGRP_DATA_SPEECH key sequence.
*/
LOCAL BOOL ksd_seqDataSpeech (USHORT          inIdx,
                              CHAR*           inSeq,
                              T_KSD_SEQGRP*   outSeqGrp,
                              CHAR**          outRestSeq,
                              T_KSD_SEQPARAM* outSeqParam)
{
  TRACE_FUNCTION ("ksd_seqDataSpeech ()");

  *outSeqGrp  = SEQGRP_DATA_SPEECH;
  *outRestSeq = inSeq + ksd_strlen(DATA_SPEECH_SEQ);

  return TRUE;
}
#endif

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : KSD                      |
| STATE   : code                  ROUTINE : ksd_seqUnknown           |
+--------------------------------------------------------------------+

  PURPOSE : This function sets the ouput parameter for the
            SEQGRP_UNKNOWN key sequence.
*/
LOCAL BOOL ksd_seqUnknown (USHORT          inIdx,
                           CHAR*           inSeq,
                           T_KSD_SEQGRP*   outSeqGrp,
                           CHAR**          outRestSeq,
                           T_KSD_SEQPARAM* outSeqParam)
{
  TRACE_FUNCTION ("ksd_seqUnknown ()");

  *outSeqGrp  = SEQGRP_UNKNOWN;
  *outRestSeq = inSeq;

  return TRUE;
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : KSD                      |
| STATE   : code                  ROUTINE : ksd_isUSSD               |
+--------------------------------------------------------------------+

  PURPOSE : This function checks whether key sequence is an
            unstructured SS command.

            <keySeq>: buffer containing the key sequence
*/
GLOBAL BOOL ksd_isUSSD (CHAR *keySeq)
{
  USHORT i=0, lenKeySeq = strlen (keySeq); /* length of incoming        */
                                      /* sequence                  */
  BOOL  ussdStr=TRUE;

  TRACE_FUNCTION("ksd_isUSSD");                                      

  /* when operating in a US band (1900 or 850), "0" and "00" shall 
   * be transmitted to the network as normal dialing numbers
   */
  if ( ksd_isBCDForUSBand( keySeq ) )
  {
    return FALSE;
  }

  /* If the last key sequence character is equal to STOP_SEQ, then check whether key 
   * sequence contains dtmf separator('p','P','w','W'). If dtmf separator is present then
   * setup a call instead of sending the string as USSD to n/w.
   */
  if (keySeq[lenKeySeq - 1] EQ STOP_SEQ)
  {
    for (; i<lenKeySeq AND (ussdStr EQ TRUE); i++)
    {
      switch(keySeq[i])
      {
        case 'p':
        case 'P':
        case 'w':
        case 'W':
          ussdStr = FALSE;
          break;
      #ifdef FF_CPHS_REL4 
        /* If the suscriber wants to initiate a MO call using a profile( on a per 
           call basis) other than the registered profile,then setup a call instead 
           of sending the string as USSD to n/w 
        */
        case '*':
          if(i NEQ 0 AND keySeq[i+1] EQ '5' AND keySeq[i+2] EQ '9')
          {
            ussdStr = FALSE;
          }
          break;
      #endif
      }
    }
  }
  if (cuscfgParams.USSD_As_MO_Call EQ CUSCFG_MOD_Enable)
  {
    ussdStr = FALSE;
  }

  /*
   *-----------------------------------------------------------------
   * USSD starts with '*' or '#' and end with '#'
   *-----------------------------------------------------------------
   */
  if (lenKeySeq >= MIN_USSD_LEN         AND
      keySeq[lenKeySeq - 1] EQ STOP_SEQ AND
      ussdStr EQ TRUE)   
      /* 
       * As according to 2.30 :
       * "Entry of any characters defined in the GSM 03.38 [15] Default Alphabet 
       * (up to the maximum defined in GSM 04.80 [17]), followed by #SEND". 
       * is to be treated as USSD the startcheck is removed
      AND
      (keySeq[0] EQ START_SEQ_1 OR keySeq[0] EQ START_SEQ_2))
       */
  {
    return TRUE;
  }
  else
  {
    return FALSE;
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : KSD                      |
| STATE   : code                  ROUTINE : ksd_searchChr            |
+--------------------------------------------------------------------+

  PURPOSE : This function is used to search for the next appearance
            of a character in a given buffer.

            <from>:  start searching from this address
            <to>:    stop searching when reaching this address
            <c>:     the character to be searched
            <inclFirstChar>:
                     searching starts at address <from> when TRUE,
                     at address <from + 1> when FALSE

            returns: a pointer pointing to the found character
*/
LOCAL CHAR* ksd_searchChr (CHAR* from,
                           CHAR* to,
                           int   c,
                           BOOL  inclFirstChar)
{
  CHAR* result; /* pointer to the found character */

  /*
   *-----------------------------------------------------------------
   * valid starting point is needed
   *-----------------------------------------------------------------
   */
  if (from EQ NULL)
    return NULL;

  /*
   *-----------------------------------------------------------------
   * leave out the first character
   *-----------------------------------------------------------------
   */
  if (!inclFirstChar)
    from++;

  if ((result = strchr (from, c)) EQ NULL)
    return NULL;

  /*
   *-----------------------------------------------------------------
   * found characters after the given end point are not considered
   *-----------------------------------------------------------------
   */
  if (to NEQ NULL AND result >= to)
    result = NULL;

  return result;
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : KSD                      |
| STATE   : code                  ROUTINE : ksd_scanSi               |
+--------------------------------------------------------------------+

  PURPOSE : This function scans a buffer whether it contains a valid
            SI parameter.

            <inSeq>:   buffer containing the SI parameter
            <inLen>:   number of characters to be taken into account
            <inSiInd>: indicates the data type of the SI parameter
            <outSi>:   the scanned parameter

            returns:   TRUE if the scanning was successful,
                       otherwise false.
*/
LOCAL BOOL ksd_scanSi (CHAR*          inSeq,
                       USHORT         inLen,
                       T_KSD_SI       inSiInd,
                       T_KSD_SIPARAM* outSi)
{
  BOOL ret = FALSE; /* indicates the success of scanning */

  TRACE_FUNCTION ("ksd_scanSi ()");

  switch (inSiInd)
  {
    case (SI_UBYTE):
      ret = utl_string2UByte (inSeq, inLen, &outSi->siUbyte);
      break;

    case (SI_USHORT):
      break;

    case (SI_ULONG):
      break;

    case (SI_BYTE):
      ret = utl_string2Byte (inSeq, inLen, &outSi->siByte);
      break;

    case (SI_SHORT):
      ret = utl_string2Short (inSeq, inLen, &outSi->siShort);
      break;

    case (SI_LONG):
      ret = utl_string2Long (inSeq, inLen, &outSi->siLong);
      break;

    case (SI_CHAR_STAR):
      if (*inSeq NEQ START_SI AND *inSeq NEQ STOP_SEQ)
        outSi->siCharStar = inSeq;

      ret = TRUE;
      break;

    default:
      break;
  }

  return ret;
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : KSD                      |
| STATE   : code                  ROUTINE : ksd_extractSi            |
+--------------------------------------------------------------------+

  PURPOSE : This function extracts the SIs from the key sequence.

            <inSeq>:       key sequence to be decoded
            <inSiNum>:     number of SI to be decoded
            <outCharsNum>: number of characters read
            <inOutSi>:     decoded SI and predefined type of SI

            returns:       TRUE if SI are extracted successfully,
                           otherwise FALSE
*/
LOCAL BOOL ksd_extractSi (CHAR*           inSeq,
                          USHORT          inSiNum,
                          USHORT*         outCharsNum,
                          T_KSD_DCD_CPLE* inOutSi)
{
  USHORT readChars    = 0;     /* number of characters read        */
  CHAR*  siEnd;                /* points to the end of key sequence*/
                               /* ('#' character is searched)      */
  CHAR*  siNextStart;          /* points to the start of the next  */
                               /* SI field                         */
  CHAR*  siNextStop;           /* points to the end of the next SI */
                               /* field                            */
  BOOL   scanSuccess  = TRUE;  /* indicates whether scanning is    */
                               /* successfull                      */
  BOOL   siEndReached = FALSE; /* indicates whether the last SI    */
                               /* field is reached                 */
  USHORT i;                    /* used for counting                */
  BOOL   isSuccess    = FALSE; /* indicates whether the extraction */
                               /* process is successfull           */

  TRACE_FUNCTION ("ksd_extractSi ()");

  if ((siEnd = ksd_searchChr (inSeq, NULL, STOP_SEQ, TRUE)) NEQ NULL)
  {
    /*
     *---------------------------------------------------------------
     * a supplementary information delimiter closing the key
     * sequence was found
     *---------------------------------------------------------------
     */
    readChars++;

    if (siEnd NEQ inSeq)
    {
      /*
       *-------------------------------------------------------------
       * searching for the next supplementary information delimiter
       * but not for those closing the key sequence
       *-------------------------------------------------------------
       */
      siNextStart = ksd_searchChr (inSeq, siEnd, START_SI, TRUE);

      if (siNextStart NEQ inSeq)
        /*
         *-----------------------------------------------------------
         * the next supplementary information delimiter must be the
         * first character in string, otherwise a failure occured
         *-----------------------------------------------------------
         */
        readChars = 0;
      else
      {
        /*
         *-----------------------------------------------------------
         * searching for the next supplementary information delimiter
         * but not for those closing the key sequence
         *-----------------------------------------------------------
         */
        siNextStop = ksd_searchChr (siNextStart,
                                    siEnd,
                                    START_SI,
                                    FALSE);

        i = 0;

        /*
         *-----------------------------------------------------------
         * successive decoding of the supplementary information
         *-----------------------------------------------------------
         */
        while (scanSuccess AND i < inSiNum AND !siEndReached)
        {
          if (siNextStop NEQ NULL)
          {
            /*
             *-------------------------------------------------------
             * scan the next supplementary information
             *-------------------------------------------------------
             */
            scanSuccess = ksd_scanSi (siNextStart + 1,
                                      (USHORT)(  siNextStop
                                               - siNextStart
                                               - 1),
                                      inOutSi[i].type,
                                      inOutSi[i].param);
            readChars   += siNextStop - siNextStart;
            *siNextStop  = NULL_TERM;
            siNextStart  = siNextStop;
            siNextStop   = ksd_searchChr (siNextStart,
                                         siEnd,
                                         START_SI,
                                         FALSE);

          }
          else
          {
            /*
             *-------------------------------------------------------
             * scan the last supplementary information
             *-------------------------------------------------------
             */
            scanSuccess = ksd_scanSi (siNextStart + 1,
                                      (USHORT)(  siEnd
                                               - siNextStart
                                               - 1),
                                      inOutSi[i].type,
                                      inOutSi[i].param);
            readChars += siEnd - siNextStart;
            *siEnd     = NULL_TERM;

            siEndReached = TRUE;
          }

          i++;
        }

        if (!scanSuccess OR (i EQ inSiNum AND !siEndReached))
          isSuccess = FALSE;
        else
          isSuccess = TRUE;
      }
    }
    else
    {
      isSuccess = TRUE;
    }
  }

  *outCharsNum = readChars;

  return isSuccess;
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : KSD                      |
| STATE   : code                  ROUTINE : ksd_decode               |
+--------------------------------------------------------------------+

  PURPOSE : This function decodes the incoming keystroke sequence.

            <inSeq>:       key sequence, to be decoded
            <inCall>:      TRUE if MMI is within a call, otherwise
                           FALSE
            <outSeqGrp>:   sequence group
            <outRestSeq>:  rest key sequence, to be decoded by a
                           further call to this function
            <outSeqParam>: sequence parameter

            returns:       TRUE if decoding was successfull,
                           otherwise FALSE
*/
GLOBAL BOOL ksd_decode (CHAR*           inSeq,
                        BOOL            inCall,
                        T_KSD_SEQGRP*   outSeqGrp,
                        CHAR**          outRestSeq,
                        T_KSD_SEQPARAM* outSeqParam)
{
  USHORT lastIdx;           /* index of the last character in  key */
                            /* sequence stored in sequence table   */
  BOOL   isEqual = FALSE;   /* indicates whether incoming sequence */
                            /* matches key sequence in sequence    */
                            /* table                               */
  USHORT i;                 /* used for counting                   */
  USHORT j;                 /* counts the number of key sequence   */
                            /* tables already searched through     */
  BOOL   isSuccess = FALSE; /* indicates whether decoding was      */
                            /* successfull                         */
  const  T_KEY_SEQ_TABLE* table; /* actual key sequence table      */

  TRACE_FUNCTION ("ksd_decode ()");

  *outSeqGrp  = SEQGRP_UNKNOWN;
  *outRestSeq = inSeq;

  if (inSeq EQ NULL OR strlen (inSeq) EQ 0)
  {
    return TRUE;
  }

  /*
   *-----------------------------------------------------------------
   * determining whether incoming sequence starts with a well known
   * key sequence
   *-----------------------------------------------------------------
   * the four tables 'keySeqTable', 'keySeqTableNonGsm', 'keyPasswd'
   * and 'keyPasswdNonGsm' are searched through
   *-----------------------------------------------------------------
   */
  j     = 0;
  table = &keySeqTable[0];

  while (j < 4 AND !isEqual)
  {
    i = 0;

    while (table[i].keySeq NEQ NULL AND !isEqual)
    {
      lastIdx = strlen (table[i].keySeq) - 1;

      if (!strncmp (inSeq, table[i].keySeq, ksd_strlen (table[i].keySeq))
          AND (   inSeq[lastIdx] EQ START_SI
               OR inSeq[lastIdx] EQ STOP_SEQ
               OR table[i].keySeq[lastIdx] NEQ DONT_CARE) )
      {
         isEqual = TRUE;
      }
      else
      {
        i++;
      }
    }

    if (!isEqual)
    {
      switch (j)
      {
        case (0): table = &keySeqTableNonGsm[0];  break;
        case (1): table = &keyPasswd[0];          break;
        case (2): table = &keyPasswdNonGsm[0];    break;
        default :                                 break;
      }
    }

    j++;
  }

  if (isEqual)
    isSuccess = table[i].func (i,
                               inSeq,
                               outSeqGrp,
                               outRestSeq,
                               outSeqParam);
  else
  {
    if (ksd_isUSSD (inSeq))
      /*
       *-------------------------------------------------------------
       * incoming sequence is handled like an unstructured
       * SS command
       *-------------------------------------------------------------
       */
       isSuccess  = ksd_seqUssd (i,
                                inSeq,
                                outSeqGrp,
                                outRestSeq,
                                outSeqParam);
    else
    {
      /*
       *-------------------------------------------------------------
       * the handlings of the sequences for changing the call
       * mode are separated from the normal handling to avoid
       * an incorrect interpretation
       *-------------------------------------------------------------
       */
#ifdef SMI
      if (strncmp (inSeq, DATA_SPEECH_SEQ,
                   ksd_strlen(DATA_SPEECH_SEQ)) EQ 0)
        isSuccess  = ksd_seqDataSpeech (i,
                                        inSeq,
                                        outSeqGrp,
                                        outRestSeq,
                                        outSeqParam);
      else if (strncmp (inSeq, SPEECH_DATA_SEQ,
                        ksd_strlen(SPEECH_DATA_SEQ)) EQ 0)
        isSuccess  = ksd_seqSpeechData (i,
                                        inSeq,
                                        outSeqGrp,
                                        outRestSeq,
                                        outSeqParam);
      else
#endif
      {
       /*
        *-------------------------------------------------------------
        * determining whether incoming sequence starts with a
        * well known key sequence specific for in call mode
        *-------------------------------------------------------------
        */
//TISH, patch for OMAPS00110151/OMAPS00130693
//start
//		  if (inCall)
//		  if (inCall AND (strlen(inSeq) EQ 1)) //disabled by Jinshu Wang, 2007-05-05
		  if (inCall AND !((psaCC_ctbCountActiveCall() EQ 1) AND (strlen(inSeq) EQ 2) AND (inSeq[0] EQ '1'))) 
//end
        {
          i     = 0;
          table = &keyWithinCall[0];

          while (table[i].keySeq NEQ NULL AND
                 strncmp (inSeq,
                          table[i].keySeq,
                          ksd_strlen (table[i].keySeq)) NEQ 0)
            i++;

          if (table[i].keySeq NEQ NULL)
            isSuccess = table[i].func (i,
                                       inSeq,
                                       outSeqGrp,
                                       outRestSeq,
                                       outSeqParam);
        }

        if (!isSuccess)
          isSuccess = ksd_seqDialIdx (i,
                                      inSeq,
                                      outSeqGrp,
                                      outRestSeq,
                                      outSeqParam);

        if (!isSuccess)
          isSuccess = ksd_seqDial (i,
                                   inSeq,
                                   outSeqGrp,
                                   outRestSeq,
                                   outSeqParam);
      }
    }
  }

  return isSuccess;
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : KSD                      |
| STATE   : code                  ROUTINE : ksd_getPwdHidden         |
+--------------------------------------------------------------------+

  PURPOSE : This function is used to hide passwords which are being
            entered during input of a keystroke sequence.

            <inSeq>:   key sequence to be manipulated
            <replace>: character which is used for replacing of
                       passwords
*/
GLOBAL void ksd_getPwdHidden (CHAR* inSeq,
                              CHAR  replace)
{
  USHORT lastIdx;         /* index of the last character in  key */
  BOOL   isEqual = FALSE; /* indicates whether incoming sequence */
                          /* matches key sequence in sequence    */
                          /* table                               */
  USHORT i;               /* used for counting                   */
  USHORT j;               /* counts the number of key sequence   */
                          /* tables already searched through     */
  CHAR   bufSeq[MAX_KEYSEQ_SIZE]; /* local copy of incoming      */
                                  /* sequence                    */
  const  T_KEY_SEQ_TABLE* table;  /* actual key sequence table   */

  CHAR*          restSeq;         /* rest sequence               */
  T_KSD_SEQGRP   seqGrp;          /* seqeunce group              */
  T_KSD_SEQPARAM seqParam;        /* sequence parameter          */

  TRACE_FUNCTION ("ksd_getPwdHidden ()");

  if (inSeq EQ NULL)
    return;
  else if (strlen (inSeq) EQ 0)
    return;

  /*
   *-----------------------------------------------------------------
   * make a local copy of the incoming sequence
   *-----------------------------------------------------------------
   * decoding process used for identify the sequence group  might
   * change content of string
   *-----------------------------------------------------------------
   */
  strncpy (bufSeq, inSeq, MAX_KEYSEQ_SIZE-1);
  bufSeq[MAX_KEYSEQ_SIZE-1] = NULL_TERM;

  /*
   *-----------------------------------------------------------------
   * determining whether incoming sequence starts with a well known
   * key sequence which contains passwords
   *-----------------------------------------------------------------
   * the two tables 'keyPasswd' and 'keyPasswdNonGsm' are searched
   * through
   *-----------------------------------------------------------------
   */
  j     = 0;
  table = &keyPasswd[0];

  while (j < 2 AND !isEqual)
  {
    i = 0;

    while (table[i].keySeq NEQ NULL AND !isEqual)
    {
      lastIdx = strlen (table[i].keySeq) - 1;

      if (!strncmp (inSeq, table[i].keySeq, ksd_strlen (table[i].keySeq))
          AND (   inSeq[lastIdx] EQ START_SI
               OR inSeq[lastIdx] EQ STOP_SEQ
               OR table[i].keySeq[lastIdx] NEQ DONT_CARE) )
      {
         isEqual = TRUE;
      }
      else
      {
        i++;
      }
    }

    if (!isEqual)
    {
      table = &keyPasswdNonGsm[0];
    }

    j++;
  }

  if (isEqual)
  {
    CHAR* token    = inSeq + ksd_strlen(table[i].keySeq);
    CHAR* endToken = inSeq + strlen (inSeq) - 1;

    (void)table[i].func (i,
                   bufSeq,
                   &seqGrp,
                   &restSeq,
                   &seqParam);

    switch (seqGrp)
    {
      case (SEQGRP_ACT_SIM_LOCK):
      case (SEQGRP_DEACT_SIM_LOCK):
      case (SEQGRP_CB):
        {
          /*
           *---------------------------------------------------------
           * replace only the characters of the first supplementary
           * information field
           *---------------------------------------------------------
           */
          if (*token EQ START_SI)
          {
            token++;

            while (token  <=  endToken AND
                   *token NEQ STOP_SEQ AND
                   *token NEQ START_SI    )
            {
              *token = replace;
              token++;
            }
          }
        }
        break;

      case (SEQGRP_PWD):
      case (SEQGRP_UBLK):
        {
          j = 0;

          /*
           *---------------------------------------------------------
           * replace the characters of the first three supplementary
           * information fields
           *---------------------------------------------------------
           */
          while (j < 3)
          {
            if (*token EQ START_SI)
            {
              token++;

              while (token  <=  endToken AND
                     *token NEQ STOP_SEQ AND
                     *token NEQ START_SI    )
              {
                *token = replace;
                token++;
              }
            }

            j++;
          }
        }
        break;

      default:
        break;
    }
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : KSD                      |
| STATE   : code                  ROUTINE : ksd_isSATSscs            |
+--------------------------------------------------------------------+

  PURPOSE : This function return whether the given string is a
            supplementary service control string for SAT.

            <inSeq>:       key sequence

            returns:       TRUE if string is a supplementary service
                           control string, otherwise FALSE
*/
GLOBAL BOOL ksd_isSATSscs ( CHAR* inSeq )
{
  char *clirSeq;

  if ( !ksd_isSscs ( inSeq ) )
  {
    return ( FALSE );
  }

  if ( (clirSeq = strstr ( inSeq, KSD_SUPPRESS_CLIR )) NEQ NULL )
  {
    clirSeq += strlen ( KSD_SUPPRESS_CLIR );
  }
  else if ( (clirSeq = strstr ( inSeq, KSD_INVOKE_CLIR )) NEQ NULL )
  {
    clirSeq += strlen ( KSD_INVOKE_CLIR );
  }
  else
  {
    return ( TRUE ); /* no CLIR override found */
  }

  if ( *clirSeq NEQ '\0' )
  {
    /* number is provided */
    return ( FALSE );
  }
  return ( TRUE );
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : KSD                      |
| STATE   : code                  ROUTINE : ksd_isSscs               |
+--------------------------------------------------------------------+

  PURPOSE : This function return whether the given string is a !known!
            supplementary service control string. (ussd return FALSE !!)

            <inSeq>:       key sequence

            returns:       TRUE if string is a supplementary service
                           control string, otherwise FALSE
*/
GLOBAL BOOL ksd_isSscs (CHAR* inSeq)
{
  USHORT lastIdx;           /* index of the last character in  key */
                            /* sequence stored in sequence table   */
  BOOL   isEqual = FALSE;   /* indicates whether incoming sequence */
                            /* matches key sequence in sequence    */
                            /* table                               */
  USHORT i;                 /* used for counting                   */
  USHORT j;                 /* counts the number of key sequence   */
                            /* tables already searched through     */
  BOOL   isSscs = FALSE;    /* indicates whether decoding was      */
                            /* successfull                         */
  const  T_KEY_SEQ_TABLE* table; /* actual key sequence table      */

  TRACE_FUNCTION ("ksd_isSscs ()");

  if (inSeq EQ NULL OR strlen (inSeq) EQ 0)
  {
    return isSscs;
  }

  /*
   *-----------------------------------------------------------------
   * determining whether incoming sequence starts with a well known
   * key sequence
   *-----------------------------------------------------------------
   * the two tables 'keySeqTable' and 'keyPasswd' are searched
   * through
   *-----------------------------------------------------------------
   */
  j     = 0;
  table = &keySeqTable[0];

  while (j < 2 AND !isEqual)
  {
    i = 0;

    while (table[i].keySeq NEQ NULL AND !isEqual)
    {
      lastIdx = strlen (table[i].keySeq) - 1;

      if (!strncmp (inSeq, table[i].keySeq, ksd_strlen (table[i].keySeq))
          AND (   inSeq[lastIdx] EQ START_SI
               OR inSeq[lastIdx] EQ STOP_SEQ
               OR table[i].keySeq[lastIdx] NEQ DONT_CARE) )
      {
         isEqual = TRUE;
      }
      else
      {
        i++;
      }
    }

    if (!isEqual)
    {
      table = &keyPasswd[0];
    }

    j++;
  }

  if (isEqual /*OR ksd_isUSSD (inSeq)*/)  /* changed from clb 05/12/00 for SAT */
  {
    isSscs = TRUE;
  }

  return isSscs;
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : KSD                      |
| STATE   : code                  ROUTINE : ksd_isFDNCheckSeq        |
+--------------------------------------------------------------------+

  PURPOSE : This function return whether the given string must check
            in FDN phonebook

            <inSeq>:       key sequence

            returns:       TRUE if string must check,
                           otherwise FALSE
*/

GLOBAL BOOL ksd_isFDNCheckSeq (CHAR* inSeq)
{
  USHORT lastIdx;           /* index of the last character in  key */
                            /* sequence stored in sequence table   */
  USHORT i=0;               /* used for counting                   */

  TRACE_FUNCTION ("ksd_isFDNCheckSeq ()");

  if ( ( inSeq EQ NULL ) OR ( *inSeq EQ '\0' ) )
  {
    return ( FALSE );           /* no check */
  }
  /* check if the inSeq a key sequence */
  if ( ( *inSeq NEQ START_SEQ_1 ) AND ( *inSeq NEQ START_SEQ_2 ) )
  {
    /* normal number must check in FDN */
    return ( TRUE );            /* check */
  }

  /*
   *-----------------------------------------------------------------
   * determining whether incoming sequence starts with a well known
   * key sequence
   */
  while ( keyNoFDNSeqTable[i].keySeq NEQ NULL )
  {
    lastIdx = strlen (keyNoFDNSeqTable[i].keySeq) - 1;

    if ( strncmp ( inSeq,
                   keyNoFDNSeqTable[i].keySeq,
                   ksd_strlen ( keyNoFDNSeqTable[i].keySeq ) ) EQ 0 )
    {
      if ( keyNoFDNSeqTable[i].keySeq[lastIdx] EQ DONT_CARE )
      {
        if ( inSeq[lastIdx] EQ START_SI  OR  inSeq[lastIdx] EQ STOP_SEQ )
        {
          return ( FALSE );     /* no check */
        }
      }
      else
      {
        return ( FALSE );       /* no check */
      }
    }
    i++;
  }

   return ( TRUE );             /* check */
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : KSD                      |
| STATE   : code                  ROUTINE : ksd_isBCDForUSBand       |
+--------------------------------------------------------------------+

  PURPOSE : This function checks if US band is used and key sequence
            is "0" or "00", because these key sequences shall be send
            to network as normal dialing numbers when using US band.

            <inSeq>:       key sequence

            returns:       TRUE if US band is used and key sequence
                           is "0" or "00",
                           otherwise FALSE
*/
GLOBAL BOOL ksd_isBCDForUSBand (CHAR* inSeq)
{
SHORT mcc,mnc;

  TRACE_FUNCTION("ksd_isBCDForUSBand()");
 
  /*
  *   Get the current MCC (and MNC, although we are not interested
  *   in that) in SHORT form.
  */
  cmhMM_CnvrtPLMN2INT(mmShrdPrm.usedPLMN.mcc,mmShrdPrm.usedPLMN.mnc,&mcc,&mnc);

  /*
  *   Check if band is US band, based on whether the current MCC is
  *   a US MCC.
  */
  if ((mcc>=0x310) AND (mcc<=0x316))
  {
    /* US band is used, check key sequence */
    if ( !strcmp(inSeq, "0") OR !strcmp(inSeq, "00") )
    {
      TRACE_EVENT("US band detected, %s are normal dialing numbers");
      return TRUE;
    }
  }

  return FALSE;
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : KSD                      |
| STATE   : code                  ROUTINE : ksd_check_write_to_LDN   |
+--------------------------------------------------------------------+

  PURPOSE : This function is used to decide if the keystroke sequence needs to
            be stored in the LDN or not.

            <inSeq>:       key sequence

            returns:       TRUE if the keystroke sequence doesn't contain password
                           or it is not a local string
                           otherwise FALSE
*/
GLOBAL BOOL ksd_isLDNWriteCheckSeq (CHAR*  inSeq)
{

  USHORT lastIdx;           /* index of the last character in  key */
                            /* sequence stored in sequence table   */
  USHORT i;                 /* used for counting                   */
  USHORT j;                 /* counts the number of key sequence   */
                            /* tables already searched through     */
  const  T_KEY_SEQ_TABLE* table; /* actual key sequence table      */

  TRACE_FUNCTION("ksd_isLDNWriteCheckSeq()");

  if (inSeq EQ NULL OR strlen (inSeq) EQ 0)
  {
    return TRUE;
  }

  j     = 0;
  table = &keySeqTableNonGsm[0];

  while (j < 4)
  {
    i = 0;

    while (table[i].keySeq NEQ NULL)
    {
      lastIdx = strlen (table[i].keySeq) - 1;

      if (!strncmp (inSeq, table[i].keySeq, ksd_strlen (table[i].keySeq))
          AND (   inSeq[lastIdx] EQ START_SI
               OR inSeq[lastIdx] EQ STOP_SEQ
               OR table[i].keySeq[lastIdx] NEQ DONT_CARE) )
      {
         return FALSE;
      }
      else
      {
        i++;
      }
    }

    switch (j)
    {
      case (0): table = &keyPasswd[0];             break;
      case (1): table = &keyPasswdNonGsm[0];       break;
      case (2): table = &keyNoFDNSeqTable[0];      break;
      default :                                    break;
    }
    
    j++;
  }
  return TRUE;
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : KSD                      |
| STATE   : code                  ROUTINE : ksd_seqRegPwdOutBarrServ |
+--------------------------------------------------------------------+

  PURPOSE : This function sets the ouput parameter for the
            SEQGRP_PWD,  key sequence.
*/
LOCAL BOOL ksd_seqRegPwdOutBarrServ (USHORT          inIdx,
                                     CHAR*           inSeq,
                                     T_KSD_SEQGRP*   outSeqGrp,
                                     CHAR**          outRestSeq,
                                     T_KSD_SEQPARAM* outSeqParam)
{
  TRACE_FUNCTION ("ksd_seqRegPwdOutBarrServ ()");

  outSeqParam->pwd.ssCd = KSD_SS_BOC;

  *outSeqGrp  = SEQGRP_PWD;

  return ksd_getCpwdFromTriplePw (inIdx, inSeq, outRestSeq, outSeqParam);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : KSD                      |
| STATE   : code                  ROUTINE : ksd_seqRegPwdInBarrServ  |
+--------------------------------------------------------------------+

  PURPOSE : This function sets the ouput parameter for the
            SEQGRP_PWD,  key sequence.
*/
LOCAL BOOL ksd_seqRegPwdInBarrServ (USHORT          inIdx,
                                    CHAR*           inSeq,
                                    T_KSD_SEQGRP*   outSeqGrp,
                                    CHAR**          outRestSeq,
                                    T_KSD_SEQPARAM* outSeqParam)
{
  TRACE_FUNCTION ("ksd_seqRegPwdInBarrServ ()");

  outSeqParam->pwd.ssCd = KSD_SS_BIC;

  *outSeqGrp  = SEQGRP_PWD;

  return ksd_getCpwdFromTriplePw (inIdx, inSeq, outRestSeq, outSeqParam);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : KSD                      |
| STATE   : code                  ROUTINE : ksd_seqActOutBarrServ    |
+--------------------------------------------------------------------+

  PURPOSE : This function sets the ouput parameter for the
            SEQGRP_CB, KSD_OP_ACT, KSD_SS_BOC key sequence.
*/
LOCAL BOOL ksd_seqActOutBarrServ (USHORT          inIdx,
                                  CHAR*           inSeq,
                                  T_KSD_SEQGRP*   outSeqGrp,
                                  CHAR**          outRestSeq,
                                  T_KSD_SEQPARAM* outSeqParam)
{
  TRACE_FUNCTION ("ksd_seqActOutBarrServ ()");

  outSeqParam->cb.opCd = KSD_OP_ACT;
  outSeqParam->cb.ssCd = KSD_SS_BOC;

  *outSeqGrp  = SEQGRP_CB;

  return ksd_getClckFromPwBs (inIdx, inSeq, TRUE, outRestSeq, outSeqParam);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : KSD                      |
| STATE   : code                  ROUTINE : ksd_seqActInBarrServ     |
+--------------------------------------------------------------------+

  PURPOSE : This function sets the ouput parameter for the
            SEQGRP_CB, KSD_OP_ACT, KSD_SS_BIC key sequence.
*/
LOCAL BOOL ksd_seqActInBarrServ (USHORT          inIdx,
                                 CHAR*           inSeq,
                                 T_KSD_SEQGRP*   outSeqGrp,
                                 CHAR**          outRestSeq,
                                 T_KSD_SEQPARAM* outSeqParam)
{
  TRACE_FUNCTION ("ksd_seqActInBarrServ ()");

  outSeqParam->cb.opCd = KSD_OP_ACT;
  outSeqParam->cb.ssCd = KSD_SS_BIC;

  *outSeqGrp  = SEQGRP_CB;

  return ksd_getClckFromPwBs (inIdx, inSeq, TRUE, outRestSeq, outSeqParam);
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : KSD                      |
| STATE   : code                  ROUTINE : ksd_seqActAllBarrServ    |
+--------------------------------------------------------------------+

  PURPOSE : This function sets the ouput parameter for the
            SEQGRP_CB, KSD_OP_ACT, KSD_SS_ALL_CBSS key sequence.
*/
LOCAL BOOL ksd_seqActAllBarrServ (USHORT          inIdx,
                                  CHAR*           inSeq,
                                  T_KSD_SEQGRP*   outSeqGrp,
                                  CHAR**          outRestSeq,
                                  T_KSD_SEQPARAM* outSeqParam)
{
  TRACE_FUNCTION ("ksd_seqActAllBarrServ ()");

  outSeqParam->cb.opCd = KSD_OP_ACT;
  outSeqParam->cb.ssCd = KSD_SS_ALL_CBSS;

  *outSeqGrp  = SEQGRP_CB;

  return ksd_getClckFromPwBs (inIdx, inSeq, TRUE, outRestSeq, outSeqParam);
}
