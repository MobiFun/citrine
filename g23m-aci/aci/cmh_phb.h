/* 
+----------------------------------------------------------------------------- 
|  Project :  GSM-PS (6147)
|  Modul   :  CMH_PHB
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
|  Purpose :  Definitions for the command handler of the 
|             phonebook management.
+----------------------------------------------------------------------------- 
*/ 

#ifndef CMH_PHB_H
#define CMH_PHB_H

/*==== CONSTANTS ==================================================*/
  
#define PHB_CMH_IDX_MAX     (255) /* index maximum value          */
#define PHB_CMH_IDX_MIN     (1)   /* index minimum value          */

#define PHB_TONPI_NO_DIAL   0xFF  /* SSC String contains no       */
                                  /* dialling number              */
#define PHB_DIAL_WAIT       ('W') /* Dial str. wait for confirm   */
#define PHB_DIAL_PAUSE      ('P') /* DTMF 3 sec pause character   */
#define PHB_DTMF_SEP        ('P') /* DTMF separator character     */
#define PHB_WILD_CRD        ('?') /* PHB wild card character      */
#define PHB_EXP_DIGIT       ('E') /* 11.11 Expansion digit (n.a.) */

/*==== TYPES ======================================================*/

/*==== PROTOTYPES =================================================*/

GLOBAL BOOL  cmhPHB_rplcIntnl  ( CHAR*          inNumber, 
                                 CHAR*          outNumber,
                                 UBYTE          inCnt      );
GLOBAL BOOL  cmhPHB_cvtPhbType ( T_ACI_PB_STOR  inMem,
                                 T_PHB_TYPE*    outMem     );

GLOBAL void  cmhPHB_toaMrg     ( T_ACI_TOA*     type,
                                 UBYTE*         octet      );
GLOBAL void  cmhPHB_toaDmrg    ( UBYTE          octet,
                                 T_ACI_TOA*     type       );
GLOBAL void  cmhPHB_getAdrBcd  ( UBYTE*         pBcd,
                                 UBYTE*         pNumDigits,
                                 UBYTE          maxDigits,
                                 CHAR*          pStr       );
GLOBAL UBYTE cmhPHB_getAdrStr  ( CHAR*          pStr,
                                 UBYTE          maxIdx,
                           const UBYTE*         pBcd,
                                 UBYTE          numDigits  );
GLOBAL void cmhPHB_cpyRecEntr  ( T_ACI_PB_ENTR* entry, 
#ifdef TI_PS_FFS_PHB
                                 T_PHB_TYPE     book,
#endif
                           const T_PHB_RECORD*  record     );
GLOBAL void cmhPHB_invldEntr   ( T_ACI_PB_ENTR* entry      );

GLOBAL T_ACI_RETURN cmhPHB_PlusCPBF  ( T_ACI_CMD_SRC  srcId,
                                       T_ACI_PB_TEXT  *findtext,
                                       T_ACI_SR_TYP   type,
                                       T_ACI_CPBF_MOD mode,
                                       SHORT*         firstIdx,
                                       SHORT*         found,
                                       T_ACI_PB_ENTR* pbLst     );
GLOBAL T_ACI_RETURN cmhPHB_PlusCPBR  ( T_ACI_CMD_SRC  srcId,
                                       T_ACI_SR_TYP   type,
                                       SHORT          startIdx,
                                       SHORT          stopIdx,
                                       SHORT*         lastIdx,
                                       T_ACI_PB_ENTR* pbLst     );
GLOBAL void         cmhPHB_getTagNt  ( UBYTE*         inTag,
                                       UBYTE          maxInLen,
                                       CHAR*          outTag, 
                                       UBYTE          maxOutLen );
GLOBAL void         cmhPHB_getTagSim ( T_ACI_PB_TEXT  *inTag,
                                       UBYTE*         outTag, 
                                       UBYTE          maxOutLen );
GLOBAL void         cmhPHB_invldPhbDateTime
                                     ( T_PHB_RECORD*  record    );
GLOBAL void         cmhPHB_invldCmhDateTime
                                     ( T_ACI_VP_ABS*  dateTime  );
GLOBAL void         cmhPHB_cpyCmhDateTime
                                     ( const T_ACI_VP_ABS*  dateTime,
                                       T_PHB_RECORD*  record    );
GLOBAL void         cmhPHB_cpyPhbDateTime 
                                     ( const T_PHB_RECORD*  record,
                                       T_ACI_VP_ABS*  dateTime  );
GLOBAL void         cmhPHB_StatIndication 
                                     ( T_PHB_STAT     psaStatus,
                                       SHORT          cmeError, 
                                       BOOL           indicate  );

GLOBAL void cmhPHB_getMfwTagSim      ( T_ACI_PB_TEXT*  inTag,
                                       UBYTE*          outTag, 
                                       UBYTE*          outTagLen, 
                                       UBYTE           maxOutLen );
GLOBAL void cmhPHB_getMfwTagNt       ( const UBYTE*    inTag,
                                       UBYTE           maxInLen,
                                       UBYTE*          outTag, 
                                       UBYTE*          maxOutLen );

GLOBAL CHAR cmhPHB_convertBCD2char   ( UBYTE bcd );

GLOBAL void cmhPHB_ksdDecodeToa      (CHAR*         number, 
                                      CHAR**        pNumber, 
                                      T_PHB_RECORD* entry);

/*==== EXPORT =====================================================*/

#ifdef CMH_PHBF_C

GLOBAL T_ENT_STAT phbEntStat;
GLOBAL T_ACI_PBCF_LDN PBCFldn = PBCF_LDN_Enable;
GLOBAL T_ACI_PBCF_LRN PBCFlrn = PBCF_LRN_Enable;
GLOBAL T_ACI_PBCF_LMN PBCFlmn = PBCF_LMN_Enable;

#else

EXTERN T_ENT_STAT phbEntStat;
EXTERN T_ACI_PBCF_LDN PBCFldn;
EXTERN T_ACI_PBCF_LRN PBCFlrn;
EXTERN T_ACI_PBCF_LMN PBCFlmn;

#endif /* CMH_PHBF_C */

#endif /* CMH_PHB_H */

/*==== EOF =======================================================*/
