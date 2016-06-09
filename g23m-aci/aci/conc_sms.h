/* 
+----------------------------------------------------------------------------- 
|  Project :  $Workfile::
|  Modul   :  CONC_SMS
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
|  Purpose :  SMS Concatenation Handler
+----------------------------------------------------------------------------- 
*/ 



#ifndef CONC_SMS_H
#define CONC_SMS_H

/* for target tests of Conc. SMS define _CONC_TESTING_ */
/*#ifndef _CONC_TESTING_
#define _CONC_TESTING_
#endif*/


/*==== CONSTANTS ==================================================*/

#define SMS_IEI_CONC_8BIT  0x00
#define SMS_IEI_CONC_16BIT 0x08


#define L_UNCOMP_8BIT_DATA 140
#define L_UNCOMP_7BIT_DATA 160
#define L_UNCOMP_8BIT_DATA_CONC 134
#define L_UNCOMP_7BIT_DATA_CONC 153

/* only for test purposes */
#define L_UNCOMP_8BIT_DATA_TST 14
#define L_UNCOMP_7BIT_DATA_TST 16
#define L_UNCOMP_8BIT_DATA_CONC_TST 13
#define L_UNCOMP_7BIT_DATA_CONC_TST 15

#define COMMAND_TYPE_DELETE 0x02

#define CONC_MAX_SEGS 10 /* maximum number of segment in one conc. SMS */

/* max number of conc. SMs in segment buffer and assembly buffer */
#define MAX_BUF_ELEMS 5

/* max number of conc. SMs in concatenation buf */
#define MAX_CONC_BUF_ELEMS 30


#define CONC_UDH_LEN 6

#define MAX_SEG_TOTAL 120

/* detection modes for SMS_getSMSType */
#define MODE1 1
#define MODE2 2
#define MODE3 3


/*==== TYPES ======================================================*/

#define SET_CONC \
  {concShrdPrm.isConcatenated = TRUE;}

#define UNSET_CONC \
  {concShrdPrm.isConcatenated = FALSE;}

#define ISSET_CONC \
  (concShrdPrm.isConcatenated EQ TRUE)


#ifdef _CONC_TESTING_
#define SET_OWNBUF_CONC \
  if (concShrdPrm.isConcatenated) ownBuf = CMD_SRC_LCL;
#else
#define SET_OWNBUF_CONC
#endif


typedef enum              /* for init functions */
{
  CONC_ERROR       = -1,       /* concatenation error */
  CONC_NOT_NEEDED,             /* no concatenation needed */
  CONC_NEEDED                  /* concatenation needed */
} T_CONC_INIT_RETURN;


typedef enum              /* for collect function */
{
  CONC_ERR_UNKN,               /* concatenation error unknown */
  CONC_ERR_BUF_FULL,           /* buffer full */
  CONC_CONTINUED,              /* concatenation continued */
  CONC_COMPLETED               /* concatenation completed */
} T_CONC_ASSEMBLY_RETURN;


typedef enum
{
  UNKNOWN = -1,
  NORMAL,
  CONCATE,
  VOICEMAIL,
  NORMAL_IND_CSMS
} T_SMS_TYPE;


typedef struct            /* data buffer for Assembly of Concat. SMS */
{
  UBYTE  in_use;
  USHORT ref_num;
  CHAR   address[MAX_SMS_ADDR_DIG+1];
  UBYTE  next_exp_num;
  UBYTE  segs_left;
  UBYTE  seg_count;
  T_SM_DATA_EXT data;
}
T_SM_ASSEMBLY;

typedef struct            /* segment buffer for Concat. SMS */
{
  UBYTE  in_use;
  USHORT ref_num;
  CHAR   address[MAX_SMS_ADDR_DIG+1];
  T_ACI_LIST *list;
}
T_SEG_BUF;

typedef struct            /* one segment buffer element for Concat. SMS */
{
  UBYTE seq_num;
  UBYTE rec_num;
  UBYTE status;           /* in CMH format */
  T_SM_DATA_EXT data;
}
T_SEG_BUF_ELEM;

typedef struct            /* concatenated buffer for Concat. SMS */
{
  UBYTE  in_use;
  USHORT ref_num;
  CHAR   address[MAX_SMS_ADDR_DIG+1];
  UBYTE  max_num;
  T_ACI_LIST *list;
}
T_CONC_BUF;

typedef struct            /* one concatenated buffer element for Concat. SMS */
{
  UBYTE seq_num;
  UBYTE rec_num;
  UBYTE status;           /* in CMH format */
  UBYTE mem;
}
T_CONC_BUF_ELEM;


typedef struct
{
  CHAR       da[MAX_SMS_ADDR_DIG+1];
  CHAR       *p_da;
  T_ACI_TOA  toda;
  T_ACI_TOA  *p_toda;
  T_ACI_LIST *currConcBufListElem; /* current element in concatenated buffer */
  UBYTE      skipStoSent;
} T_CONC_CMSS;

typedef struct
{
  CHAR       da[MAX_SMS_ADDR_DIG+1];
  CHAR       *p_da;
  T_ACI_TOA  toda;
  T_ACI_TOA  *p_toda;
  T_SM_DATA_EXT data;
  USHORT     offset;      /* byte offset in a conc. SM */
  CHAR       sca[MAX_SMS_ADDR_DIG+1];
  CHAR       *p_sca;
  T_ACI_TOA  tosca;
  T_ACI_TOA  *p_tosca;
  SHORT      isReply;
  USHORT     sent_bytes;
} T_CONC_CMGS;

typedef struct
{
  UBYTE      command_count;
  UBYTE      fo;
  UBYTE      ct;
  UBYTE      pid;
  CHAR       da[MAX_SMS_ADDR_DIG+1];
  CHAR       *p_da;
  T_ACI_TOA  toda;
  T_ACI_TOA  *p_toda;
  T_SM_DATA_EXT data;       /* user data */
} T_CONC_CMGC;

typedef struct
{
  UBYTE      rdMode;
  T_ACI_LIST *currConcBufListElem; /* current element in concatenated buffer */
} T_CONC_CMGR;

typedef struct
{
  CHAR       da[MAX_SMS_ADDR_DIG+1];
  CHAR       *p_da;
  T_ACI_TOA  toda;
  T_ACI_TOA  *p_toda;
  UBYTE      stat;
  UBYTE      msg_ref;
  T_SM_DATA_EXT data;       /* user data */
  USHORT     offset;      /* byte offset in a conc. SM */
  CHAR       sca[MAX_SMS_ADDR_DIG+1];
  CHAR       *p_sca;
  T_ACI_TOA  tosca;
  T_ACI_TOA  *p_tosca;
  SHORT      isReply;
  USHORT     sent_bytes;
} T_CONC_CMGW;

typedef struct
{
  USHORT     ref_num;              /* ref. number of the current conc. buffer */
  CHAR       *address;
  UBYTE      error_count;
  T_ACI_LIST *currConcBufListElem; /* current element in concatenated buffer */
} T_CONC_CMGD;





typedef struct            /* Shared Parameter for Concat. SMS */
{
  UBYTE      max_sms_len; /* max. length of one SM segment */
  UBYTE      srcId;       /* current source ID */
  UBYTE      first_mr;    /* message ref. of the first sent segment */
  UBYTE      sentSegs;    /* number of successfully sent segments,for "+CMGC"*/
  UBYTE      isConcatenated;
  UBYTE      concTesting;
  UBYTE      l_uncomp8bit_data;
  UBYTE      l_uncomp7bit_data;
  UBYTE      l_uncomp8bit_data_conc;
  UBYTE      l_uncomp7bit_data_conc;
  UBYTE      mem_store;
  UBYTE      elem_count;  /* total number of CSMS segments */

  struct
  {
    UBYTE      ref_num;     /* conc. SM reference number */
    UBYTE      max_num;     /* max. number of SMs in one conc. message */
    UBYTE      seq_num;     /* sequence number of the current SM */
  } udh;

  struct 
  {
  USHORT RefNum;
  UBYTE MaxNum;
  UBYTE SeqNum[CONC_MAX_SEGS];
  UBYTE RecNum[CONC_MAX_SEGS];
  UBYTE MemType[CONC_MAX_SEGS]; 
  BOOL Conc_Full;
  UBYTE Numsegs; 
  } full;

  union
  {
    T_CONC_CMSS concCMSS;
    T_CONC_CMGS concCMGS;
    T_CONC_CMGC concCMGC;
    T_CONC_CMGR concCMGR;
    T_CONC_CMGW concCMGW;
    T_CONC_CMGD concCMGD;
  } specPrm;
}
T_CONC_SHRD_PRM;


#ifdef TI_PS_FF_CONC_SMS
/****************************** FUNCTIONS ********************************/


/* Init Functions */

EXTERN T_CONC_INIT_RETURN concSMS_initSendFromMem ( T_ACI_CMD_SRC srcId,
                                               UBYTE         *index,
                                               CHAR*         da,
                                               T_ACI_TOA*    toda );

EXTERN T_CONC_INIT_RETURN concSMS_initReadFromMem ( T_ACI_CMD_SRC  srcId,
                                               UBYTE          index,
                                               T_ACI_SMS_READ rdMode );

EXTERN T_CONC_INIT_RETURN concSMS_initDeleteFromMem ( T_ACI_CMD_SRC  srcId,
                                                 UBYTE          index );

EXTERN T_CONC_INIT_RETURN concSMS_initSend     ( T_ACI_SM_DATA*  tar_data,
                                            T_ACI_UDH_DATA* udh,
                                            T_ACI_CMD_SRC   srcId,
                                            CHAR*           da,
                                            T_ACI_TOA*      toda,
                                            T_SM_DATA_EXT*  src_data,
                                            CHAR*           sca,
                                            T_ACI_TOA*      tosca,
                                            SHORT           isReply,
                                            UBYTE           alphabet );

EXTERN T_CONC_INIT_RETURN concSMS_initStoreInMem (  T_ACI_SM_DATA* tar_data,
                                               T_ACI_UDH_DATA* udh,
                                               T_ACI_CMD_SRC  srcId,
                                               SHORT          index,
                                               CHAR*          address,
                                               T_ACI_TOA*     toa,
                                               T_ACI_SMS_STAT stat,
                                               UBYTE          msg_ref,
                                               T_SM_DATA_EXT* src_data,
                                               CHAR*          sca,
                                               T_ACI_TOA*     tosca,
                                               SHORT          isReply,
                                               UBYTE          alphabet );

EXTERN T_CONC_INIT_RETURN concSMS_initCommand ( T_ACI_CMD_SRC   srcId,
                                           SHORT           fo,
                                           SHORT           ct,
                                           SHORT           pid,
                                           SHORT           mn,
                                           CHAR*           da,
                                           T_ACI_TOA*      toda,
                                           T_ACI_CMD_DATA* data );



/* Callback Functions */

EXTERN void rConcSMS_PlusCMSS  (UBYTE mr, UBYTE numSeg);

EXTERN void rConcSMS_PlusCMGS  (UBYTE mr, UBYTE numSeg);

EXTERN void rConcSMS_PlusCMGR  (T_ACI_CMGL_SM*  sm,
                                T_ACI_CMGR_CBM* cbm);

EXTERN void rConcSMS_PlusCMGW  (UBYTE index, UBYTE numSeg, UBYTE mem);

EXTERN void rConcSMS_PlusCMGD  ();

EXTERN void rConcSMS_PlusCMGC  (UBYTE mr);

EXTERN void rConcSMS_PercentCMGMDU  (void);

/* Error Callback Functions */

EXTERN void rConcSMS_PlusCMS_CMSS (T_ACI_AT_CMD cmdId, T_ACI_CMS_ERR err,
                                   T_EXT_CMS_ERROR *conc_error);

EXTERN void rConcSMS_PlusCMS_CMGS (T_ACI_AT_CMD cmdId, T_ACI_CMS_ERR err,
                                   T_EXT_CMS_ERROR *conc_error);

EXTERN void rConcSMS_PlusCMS_CMGW (T_ACI_AT_CMD cmdId, T_ACI_CMS_ERR err,
                                   T_EXT_CMS_ERROR *conc_error);

EXTERN void rConcSMS_PlusCMS_CMGD (T_ACI_AT_CMD cmdId, T_ACI_CMS_ERR err,
                                   T_EXT_CMS_ERROR *conc_error);
#endif /* TI_PS_FF_CONC_SMS */


EXTERN void concSMS_Init();

EXTERN void concSMS_clearIncompleteMsg();

EXTERN void concSMS_delAllIncompleteMsg();

EXTERN void concSMS_AddtoconcBuff();

EXTERN T_SMS_TYPE SMS_getSMSType ( T_ACI_UDH_DATA* udh, char *address, UBYTE detMode);

EXTERN UBYTE concSMS_GetFirstIndex ( USHORT msg_ref, CHAR* address );

/* provide mem_type as well */
EXTERN T_CONC_BUF_ELEM *concSMS_GetFirstIndex_ext ( USHORT msg_ref, CHAR* address );

EXTERN USHORT concSMS_GetMsgRef ( T_ACI_CMGL_SM  *sm );

EXTERN T_CONC_ASSEMBLY_RETURN concSMS_Collect ( T_SM_DATA_EXT *data_conc,
                                               T_ACI_CMGL_SM  *sm,
                                               UBYTE          isStored,
                                               T_ACI_SMS_STOR mem );

#ifdef TI_PS_FF_CONC_SMS
EXTERN void rConcSMS_PercentCMGR  (T_ACI_CMGL_SM*  sm,
                                   T_ACI_CMGR_CBM* cbm);
#endif /* TI_PS_FF_CONC_SMS */



#ifdef CONC_SMS_C

GLOBAL T_CONC_SHRD_PRM      concShrdPrm;
#else

EXTERN T_CONC_SHRD_PRM      concShrdPrm;
#endif /* CONC_SMS_C */

EXTERN void concSMS_DeleteConcList();
EXTERN BOOL concSMS_concBufferAvail();

#endif
