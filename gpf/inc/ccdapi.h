/* 
+----------------------------------------------------------------------------- 
|  Project :  
|  Modul   :  ccdapi.h
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
|  Purpose :  Condat Coder Decoder Application Interface
|
|             Use this header to integrate the condat coder decoder
|             in your target system !
|
|             The application must define USE_DRIVER if CCD is
|             not linked but used as a driver
+----------------------------------------------------------------------------- 
*/ 

#ifndef CCDAPI_H
#define CCDAPI_H

#ifdef __cplusplus
extern  "C" {
#endif /*_cplusplus*/

/**************************************************************
  Types and constants for CCD
  used in application, in stub1 (api) and in driver body
 **************************************************************/

/*==== CONSTANTS ==================================================*/
/*
 * constants for the direction-parameter for coding/decoding
 */

#define UPLINK             0
#define DOWNLINK           1
#define BOTH		   UPLINK

                                /*  ccd Error codes */
#define  ccdOK              0
#define  ccdWarning         1
#define  ccdError           2
#ifdef _TOOLS_
#define  CCD_DLL_ERROR     23
#endif /* _TOOLS_ */

/* reentrancy */
#define CCD_REENTRANT      0

/*
 * Error codes to use with ccd_setError()
 */
#define ERR_NO_MORE_ERROR       0   /* Returned by ccd_getFirst/NextError   */
                                    /* in case of the end of the error list */              

#define ERR_INVALID_CALC        1   /* Invalid combination of calculations  */
                                    /* Error params: none                   */

#define ERR_PATTERN_MISMATCH    2   /* A spare pattern does not match with  */
                                    /* the specified content                */
                                    /* Error params[0] = bitposition        */

#define ERR_COMPREH_REQUIRED    3   /* Comprehension of an unknown TAG      */
                                    /* required.                            */
                                    /* Error params[0] = TAG                */
                                    /*             [1] = bitposition        */

#define ERR_IE_NOT_EXPECTED     4   /* Unknown or unexpected TAG found      */
                                    /* Error params[0] = TAG                */
                                    /*             [1] = bitposition        */

#define ERR_IE_SEQUENCE         5   /* A valid TAG is found but not in the  */
                                    /* defined sequence.                    */
                                    /* Error params[0] = TAG                */
                                    /*             [1] = bitposition        */

#define ERR_MAX_IE_EXCEED       6   /* A repeatable TAG is found but it     */
                                    /* occurs more often than it is defined */
                                    /* Error params[0] = TAG                */
                                    /*             [1] = bitposition        */

#define ERR_MAND_ELEM_MISS      7   /* A as mandatory defined element is    */
                                    /* not in the message                   */
                                    /* Error params: none                   */

#define ERR_EOC_TAG_MISSING     8   /* A indefinite length is specified for */
                                    /* an ASN.1 element but the correspond. */
                                    /* End of Component TAG is missing      */
                                    /* Error params[0] = TAG                */
                                    /*             [1] = bitposition        */
                          
#define ERR_INVALID_MID         9   /* The message starts with an undefined */
                                    /* message identifier; more exactly:    */
                                    /* with the three given parameters mId, */
                                    /* entity, and direction for the coding */
                                    /* or decoding of a Message functions   */
                                    /* CCD was not able to identify the     */
                                    /* Message                              */
                                    /* Error params[0] = Message ID         */

#define ERR_INVALID_TYPE       10   /* The actual element has an invalid    */
                                    /* type e.g. S_PADDING without a spare  */
                                    /* Error params[0] = bitposition        */

#define ERR_MAX_REPEAT         11   /* A repeatable element occurs to often */
                                    /* in the message                       */

#define ERR_NO_MEM             12   /* Memory allocation failed. May result */
                                    /* occur when using pointer types.      */
                                    /* (Dynamic array addition)             */

#define ERR_ADDR_INFO_PART     13   /* CCD recognizes a message with a      */
                                    /* disallowed addressing option         */

#define ERR_DISTRIB_PART       14   /* CCD recognizes an error during       */
                                    /* decoding of the message distribution */
                                    /* part                                 */

#define ERR_NON_DISTRIB_PART   15   /* CCD recognizes an error during       */
                                    /* decoding of the message non-         */
                                    /* distribution part                    */

#define ERR_MESSAGE_ESCAPE     16   /* CCD detects an unknown message part, */
                                    /* which is marked whith a message      */
                                    /* escape error label being used to     */
                                    /* provide an escape for, e.g. a future */
                                    /* modification of the message syntax   */

#define ERR_IGNORE             17   /* CCD detects a part of the message    */
                                    /* that is syntactically incorrect and  */
                                    /* is allowed to ignore this part       */

#define ERR_18                 18   /* dummy to complete error list -       */
                                    /* may be used in furture enhancement   */

#define ERR_19                 19   /* dummy to complete error list -       */
                                    /* may be used in furture enhancement   */

#define ERR_INTERNAL_ERROR     20   /* An internal error occured            */
                                    /* (hopefully not!)                     */
                                    /* Error params[0] = none               */
#define ERR_DEFECT_CCDDATA     21   /* The value read from a ccd table      */
                                    /* (one of the *.cdg files) is not      */
                                    /* expected by CCD, e.g. extGroup has   */ 
                                    /* another value than ' ', '+', '-',    */
                                    /* '!' or '#'.                          */ 
                                    /* Error params[0] = bitposition        */
                                    /* this error code is needed only in    */
                                    /* the development phase.               */
#define ERR_END_OF_BUF         22   /* According to the lenght indicator    */
                                    /* there must be more bits to read but  */
                                    /* end of bit buffer is reached         */

#define ERR_INT_VALUE          23   /* Error on encoding of integer value.  */
                                    /* IE of coding type ASN1_INTEGER is    */
                                    /* out of the given range.              */

#define ERR_LONG_MSG           24   /* UNUSED error code                    */
                                    /* According to l_buf and due to an     */
                                    /* unknown message extension there are  */
                                    /* more bits to be decoded than CCD or  */
                                    /* ccddata expects.                     */

#define ERR_ASN1_ENCODING      25   /* Error on IE in the outermost level.  */
#define ERR_ASN1_MAND_IE       26   /* Error on mandatory IE of type        */
                                    /* ASN1_INTEGER. The decoded value is   */
                                    /* out of the range given by the loaded */
                                    /* ccd data tables.                     */
                                    /* data tables.                         */
#define ERR_ASN1_OPT_IE        27   /* Error on optional IE of type.        */
                                    /* ASN1_INTEGER. The decoded value is   */
                                    /* out of the range given by the loaded */
                                    /* ccd data tables.                     */
#define ERR_ASN1_COND_IE       28   /* UNUSED error code                    */ 
#define ERR_COND_ELEM_MISS     29   /* UNUSED error code                    */
                                    /* Condition is met but the IE is not   */
                                    /* present in the message.              */
#define ERR_BUFFER_OF          30   /* On coding, more bits were written,   */
                                    /* than the input l_buf suggested.      */

#define ERR_NONCRITICAL_EXT    31   /* Warning (if decoding) or error (if   */
                                    /* encoding) on inappropriate usage of  */
                                    /* a nonCriticalExtensions element.     */
                                    /* The CCDTYPE_NONCRITICAL_EXTENSIONS   */
                                    /* says the IE is not extended yet from */
                                    /* a CCD point of view. So it needs     */
                                    /* neither to be encoded nor decoded.   */
                                    /* In the latter case additional parts  */
                                    /* at the end the message will be       */
                                    /* ignored by CCD. Known extensions are */
                                    /* to be encoded by CCD.                */

#define ERR_CRITICAL_EXT       32   /* Error on inappropriate usage of a    */
                                    /* nonCriticalExtensions element. The   */
                                    /* CCDTYPE_CRITICAL_EXTENSIONS says     */
                                    /* the IE is not extended yet, from a   */
                                    /* CCD point of view. So it needs not   */
                                    /* to be encoded or decoded. In latter  */
                                    /* case message is not comprehendable   */
                                    /* and must be rejected.                */
                                   
#define ERR_INVALID_CCDID      33   /* The ccd identifier for the element   */
                                    /* to be decoded did not result a valid */
                                    /* data table entry.                    */

#define ERR_MSG_LEN            34   /* Decoding is just finished. But the   */
                                    /* CCD internal bit pointer is beyond   */
                                    /* the given message length, l_buf e.g. */

#define ERR_INVALID_PTR        35   /* Pointer used in C struct for encoding*/
                                    /* is not valid.                        */
#define ERR_PROTOCOL_EXTENSION 36   /* IE of type S_PADDING_0 is preceded   */
                                    /* by 1 instead of 0.                   */
                                    /* Error params[0] = CCDID              */
                                    /*             [1] = bitposition        */

#define ERR_BITSTR_COMP        37   /* Length of bit array encoded as       */
                                    /* composition is bigger than expected  */
                                    /* Error params[0] = CCDID              */
                                    /*             [1] = addr in C-struct   */

#define ERR_ELEM_LEN           38   /* The current information element's    */
                                    /* length doesn't match to the length   */
                                    /* required by its nested elements      */

#define ERR_LEN_MISMATCH       39   /* The length of a superordinated       */
                                    /* doesn't match to read an GSM5_V      */
                                    /* element, which extends to the        */
                                    /* message end.                         */

#define ERR_CONCAT_LEN         40   /* Coding of a truncated concatenation  */
                                    /* doesn't fill the rest of message     */
                                    /* buffer.(length given by l_buf)       */

#define ERR_UNEXPECT_PAD       41   /* an inchoate byte is filled with      */
                                    /* padding bits which are not expected  */
                                   
#define ERR_CSN1_CHOICE        42   /* the number of CHOICE components      */
                                    /* doesn't match to number of possible  */
                                    /* alternatives                         */

#define MAX_CCD_ERROR 43

/*
 * max number of parameters per error for parlist
 * param ccd_getFirst/NextError
 */
#define MAX_ERR_PAR             3 
#define CCD_ERR_KIND_PARA_LIST  1
#define CCD_ERR_KIND_IE_TYPE    2


/*==== TYPES ======================================================*/

/*
 * T_MSGBUF contains the coded message.
 * o_buf specified the * offset (in Bits), * where the message
 *       starts in the buffer.
 * l_buf contains the * length (in Bits) of the coded message.
 * buf   contains the * bitcoded message as an array of Bytes.
 *
 * (Do not modify this structure !)
 */

typedef struct
{
  USHORT l_buf;
  USHORT o_buf;
  UBYTE  buf [1];
} T_MSGBUF;


/*
 * new style of error information container, used in umts
 */
#ifdef CCD_USE_ENUM_IDS
#include "ccdid.h"      /* to get enumeration for fault elements */
typedef T_CCD_ID T_ERR_INFO;
#else
typedef ULONG T_ERR_INFO;
#endif

typedef struct
{
  T_ERR_INFO  err_info;
  ULONG       err_IEaddr;
} T_CCD_ERR_TYPE;

/*
 * old style of error information container, used in gsm/gprs
 */
#define MAX_ERR_PAR 3
typedef struct
{
  UBYTE  num_para;
  USHORT err_list[MAX_ERR_PAR];
} T_CCD_PARA_LIST;

/* 
 * union supports old and new type of error information container
 */
typedef union
{
  T_CCD_PARA_LIST  para_list;
  T_CCD_ERR_TYPE   err_type;
} T_CCD_ERR_PARA;

/*
 * general structure for error information passed to CCD caller 
 */
typedef struct
{
  UBYTE error;
  UBYTE kind;
  T_CCD_ERR_PARA para;
} T_CCD_ERR_ENTRY;


/**************************************************************
  function codes for CCD interface functions
  used in stub1 (application side) and stub2 (driver side)
 **************************************************************/

#define CCD_INIT       1
#define CCD_CODMSG     2
#define CCD_DECMSG     3
#define CCD_DECBYTE    4
#define CCD_CODBYTE    5
#define CCD_DECLONG    6
#define CCD_CODLONG    7
#define CCD_BITCOPY    8
#define CCD_FIRST_ERR  9
#define CCD_NEXT_ERR   10

#if !defined (CCDDATA_PREF)
#if defined (_TOOLS_) && defined (CCDDATA_LOAD)
#define CCDDATA_PREF(ccd_fun) cddl_##ccd_fun
#else
#define CCDDATA_PREF(ccd_fun) ccd_fun
#endif /* _TOOLS_ && CCDDATA_LOAD */
#endif /* !CCDDATA_PREF */

#if defined _TOOLS_ || defined (CCD_TEST)
/*
 * Preparation for issue patching coded bits for generating
 * air messages with errors. This feature is not yet supported
 * by CCD.
 */
#define CCDP_NO_ERROR       0
#define CCDP_NOT_FOUND      1
#define CCDP_VALIDFLAG_SEEN 2
#define CCDP_UNKNOWN_ERROR  3
#define MAXREC 50
typedef struct
{
  U16 elemid[MAXREC];
  U16 numelems;
  U16 bitwidth;
  U8* bits;
  U8  errorcode;
} T_patch_info;
#endif /* _TOOLS_ */

/*==================== description  ===================================*/

/*
 * ccd_init
 *
 * Initializing the ccd-Module, must be called once before
 * any encoding/decoding
 */

/*
 * ccd_begin
 *
 * Returns the address of the CCD buffer for decoded
 * messages and locks it until ccd_end() is called
 */

/*
 * ccd_end
 *
 * Unlocks the CCD buffer for decoded
 * messages
 */

/*
 * ccd_codeMsg
 *
 * layer    : <IN>   	CCDENT_CMCE, CCDENT_MLE ..
 * direction: <IN>   	UPLINK, DOWNLINK
 * mBuf     : <IN>   	mBuf.o_buf - Offset in Bits from the
 *                                   start of mBuf.buf.
 *            <OUT>  	mBuf.l_buf - Contains the length of
 *                                   the message in Bits after
 *                                   coding.
 *            <OUT>  	mBuf.buf   - Contains the bitcoded message
 *                                   after coding.
 * mStruct:    <IN>     Pointer to the message specific structure.
 *                      The first element must contain the
 *                 	message-type (PDU-Type) as a UBYTE value.
 * 
 * pt       :  <IN>     If pt != 0xff the CCD will not decode the
 *                      the PDU-Type from the bitstream. In this
 *                      case pt is the coding of the PDU-Type.  
 */

/*
 * ccd_decodeMsg
 *
 * layer    :  <IN>    	CCDENT_CMCE, CCDENT_MLE, ...
 * direction:  <IN>    	UPLINK, DOWNLINK
 * mBuf     :  <IN>     mBuf.o_buf - Offset in Bits from start
 *                                   of mBuf.buf where the
 *                                   message starts.
 *             <IN>    	mBuf.l_buf - Contains the length of the
 *                                   message in Bits.
 *             <IN>     mBuf.buf   - Contains the bitcoded message.
 * mStruct  :  <OUT>    Pointer to the message specific structure.
 *                      The first element contains the	message-type
 *                      (PDU-Type) as a UBYTE value after decoding.
 * pt       :  <IN>     If pt != 0xff the CCD will not decode the
 *                      the PDU-Type from the bitstream. In this
 *                      case pt is the coding of the PDU-Type.
 */

/**************************************************************
  function prototypes for CCD interface functions
  used in application, stub2 (driver side) and driver body
 **************************************************************/
#ifndef CCD_PATCH_C
#ifdef _TOOLS_
  extern int    CCDDATA_PREF(ccd_set_patch_infos)   (T_patch_info* pinfo);
#endif /* _TOOLS_ */
#endif /* CCD_PATCH_C */

#ifndef CCD_C
  extern BYTE   CCDDATA_PREF(ccd_init)              (void);
  extern int    CCDDATA_PREF(ccd_exit)              (void);

  extern UBYTE* CCDDATA_PREF(ccd_begin)             (void);

  extern void   CCDDATA_PREF(ccd_end)               (void);

  extern BYTE   CCDDATA_PREF(ccd_decodeMsg)         (UBYTE           entity,
                                                     UBYTE           direction,
                                                     T_MSGBUF       *mBuf,
                                                     UBYTE          *mStruct,
                                                     UBYTE           pt);

  extern S8     CCDDATA_PREF(ccd_decodeMsgPtr)      (U8             entity,
                                                     U8             direction,
                                                     U16            l_buf,
                                                     U16            o_buf,
                                                     U8            *buf,
                                                     U8           **mStructPtr,
                                                     U8             pt);
 
  extern BYTE   CCDDATA_PREF(ccd_codeMsg)           (UBYTE           entity,
                                                     UBYTE           direction,
                                                     T_MSGBUF        *mBuf,
                                                     UBYTE           *mStruct,
                                                     UBYTE           pt);

  extern S8     CCDDATA_PREF(ccd_codeMsgPtr)        (U8           entity,
                                                     U8           direction,
                                                     U16         *l_buf,
                                                     U16          o_buf,
                                                     U8          *buf,
                                                     U8          *mStruct,
                                                     U8           pt);

  extern UBYTE  ccd_setStore          (ULONG           regNo,
                                                     ULONG           value);
#ifdef _TOOLS_
  extern ULONG  CCDDATA_PREF(ccd_init_ccddata)      (void);
#else
  extern int    ccd_register                        (int        decmsgbuf_size);
#endif /* _TOOLS_ */
#endif /*! CCD_C */

#ifndef CCD_ELEM_C
  extern int    CCDDATA_PREF(ccd_encodeElem)        (ULONG           ccdid,
                                                     USHORT         *l_buf,
                                                     USHORT          o_buf,
                                                     UCHAR          *buf,
                                                     UCHAR          *eStruct);

  extern int   CCDDATA_PREF(ccd_decodeElem)         (ULONG           ccdid,
                                                     USHORT          l_buf,
                                                     USHORT          o_buf,
                                                     UCHAR          *buf,
                                                     UCHAR          *eStruct);

#endif /* CCD_ELEM_C */

#ifndef CCD_ERR_C
  extern UBYTE  CCDDATA_PREF(ccd_getFirstError)     (UBYTE           entity,
                                                     USHORT         *parlist);

  extern UBYTE  CCDDATA_PREF(ccd_getNextError)      (UBYTE           entity,
                                                     USHORT         *parlist);

  extern ULONG  CCDDATA_PREF(ccd_getFirstFault)     
                                             (T_CCD_ERR_ENTRY **ccd_err_entry);

  extern ULONG  CCDDATA_PREF(ccd_getNextFault)  
                                             (T_CCD_ERR_ENTRY **ccd_err_entry);

  extern void   CCDDATA_PREF(ccd_free_faultlist)    (void);

  extern int    CCDDATA_PREF(ccd_get_numFaults)     (void);
#endif /*! CCD_ERR_C */

#ifndef CDC_STD_C
  extern BYTE   CCDDATA_PREF(ccd_decodeByte)        (UBYTE          *bitstream,
                                                     USHORT          startbit,
                                                     USHORT          bitlen,
                                                     UBYTE          *value);

  extern BYTE   CCDDATA_PREF(ccd_codeByte)          (UBYTE          *bitstream,
                                                     USHORT          startbit,
                                                     USHORT          bitlen,
                                                     UBYTE           val);

  extern BYTE   CCDDATA_PREF(ccd_codeLong)          (UBYTE          *bitstream,
                                                     USHORT          startbit,
                                                     USHORT          bitlen,
                                                     ULONG           value);

  extern BYTE   CCDDATA_PREF(ccd_decodeLong)        (UBYTE          *bitstream,
                                                     USHORT          startbit,
                                                     USHORT          bitlen,
                                                     ULONG          *value);

  extern void   CCDDATA_PREF(ccd_bitcopy)           (UBYTE          *dest,
                                                     UBYTE          *source,
                                                     USHORT          bitlen,
                                                     USHORT          offset);
#endif /*! CCD_STD_C */


#ifdef __cplusplus
}
#endif /*_cplusplus*/

#endif

