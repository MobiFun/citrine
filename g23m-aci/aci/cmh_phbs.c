/* 
+----------------------------------------------------------------------------- 
|  Project :  GSM-PS (6147)
|  Modul   :  CMH_PHBS
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
|  Purpose :  This module provides the set functions related to the 
|             protocol stack adapter for the phonebook management.
+----------------------------------------------------------------------------- 
*/ 

#ifndef CMH_PHBS_C
#define CMH_PHBS_C
#endif

#include "config.h"
#include "fixedconf.h"
#include "condat-features.h"
#include "aci_conf.h"

#include "aci_all.h"
/*==== INCLUDES ===================================================*/
#include "aci_cmh.h"
#include "ksd.h"

#ifdef FAX_AND_DATA
#include "aci_fd.h"
#endif    /* of #ifdef FAX_AND_DATA */

#include "psa.h"
#include "psa_sim.h"
#include "psa_sms.h"
#include "ati_cmd.h"
#include "aci_cmd.h"
#include "phb.h"
#include "cmh.h"
#include "cmh_phb.h"
#include "cmh_sms.h"
#include "pcm.h"
#include "aci_mem.h"

/*--------------------------------------------------------------------*
 *  The Below include files are included for the SIM functions called *
 *  in CPBS command handler                                           * 
 *--------------------------------------------------------------------*/

#include "dti_conn_mng.h"
#include "cmh_sim.h"



/*==== CONSTANTS ==================================================*/

#define PHB_CMH_CC_ID_NOT_USED 0xFF
#define PHB_CMH_EXT_NOT_USED   0xFF

/*==== TYPES ======================================================*/

typedef enum
{
  CMN_FULL_PRSNT,   /* all parameter present                       */
  CMN_PRTLY_PRSNT,  /* only some parameter present                 */
  CMN_NOT_PRSNT     /* no parameter present                        */
}
T_PHB_CMH_CMN_STAT;

typedef enum
{
  SNGL_VLD_PRSNT,   /* parameter is present and in valid range     */
  SNGL_INVLD_PRSNT, /* parameter is present and not in valid range */
  SNGL_NOT_PRSNT    /* parameter is not present                    */
}
T_PHB_CMH_SNGL_STAT;

#ifdef TI_PS_FFS_PHB
typedef T_PHB_RETURN T_FCT_READ ( T_PHB_TYPE    type,
                                  SHORT         index,
                                  T_PHB_RECORD* entry );
#else
typedef T_PHB_RETURN T_FCT_READ ( UBYTE         type,
                                  SHORT         index,
                                  T_PHB_RECORD* entry );
#endif

/*==== EXPORT =====================================================*/

/*==== VARIABLES ==================================================*/
/*==== FUNCTIONS ==================================================*/

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)    MODULE  : CMH_PHBS                     |
| STATE   : code             ROUTINE : sAT_PlusCPBS                 |
+-------------------------------------------------------------------+

  PURPOSE : This is the functional counterpart of the +CPBS
            AT command which is responsible for setting the 
            phonebook memory storage.

            <mem>: phonebook memory storage
*/
GLOBAL T_ACI_RETURN sAT_PlusCPBS ( T_ACI_CMD_SRC srcId, 
                                   T_ACI_PB_STOR mem,
                                   char*        pin2)
{
  T_PHB_CMD_PRM * pPHBCmdPrm; /* points to PHB command parameters */
  T_SIM_SET_PRM * pSIMSetPrm; /* To hold the SIM related values for CPBW
                                 command */ 
  T_PHB_TYPE chkMem;          /* checked and converted parameter <mem> */

  TRACE_FUNCTION ("sAT_PlusCPBS ()");

  /*
   *-----------------------------------------------------------------
   * check command source
   *-----------------------------------------------------------------
   */  
  if(!cmh_IsVldCmdSrc (srcId)) 
  { 
    return( AT_FAIL );
  }

  pPHBCmdPrm = &cmhPrm[srcId].phbCmdPrm;

  /*
   *-----------------------------------------------------------------
   * process the <mem> parameter
   *-----------------------------------------------------------------
   */  
  if ( mem NEQ PB_STOR_NotPresent )
  {
    if ( cmhPHB_cvtPhbType ( mem, &chkMem ) EQ TRUE )
    {
      if (pin2 EQ NULL OR
        (strlen ( pin2 ) EQ 0) )
      {
        pPHBCmdPrm -> cmhStor = mem;
        pPHBCmdPrm -> phbStor = chkMem;
      }
      else
      {
        if ( simShrdPrm.PINStat EQ PS_PUK2 )
        {
          ACI_ERR_DESC( ACI_ERR_CLASS_Cme, CME_ERR_SimPuk2Req );
          return( AT_FAIL );
        }
        /* check preselected phonebook storage for FDN or BDN */  
        if ( ( mem EQ PB_STOR_Fd OR mem EQ PB_STOR_Bd )
               AND
             ( simShrdPrm.pn2Stat EQ PS_PIN2       OR
               simShrdPrm.pn2Stat EQ NO_VLD_PS        ) )
        { 
          if ( simShrdPrm.PINStat EQ PS_RDY OR
               simShrdPrm.PINStat EQ PS_PIN2 )
          {
            simShrdPrm.PINStat = PS_PIN2;          
          }
          else
          {
            ACI_ERR_DESC( ACI_ERR_CLASS_Cme, CME_ERR_SimFail );
            return( AT_FAIL );
          }

         /*-------------------------------------------------------------------------*
          * The below code is added on 12/08/2003 as in R99 one more new paramter   *
          * is added in CPBS command <password>, this password is used to           *
          * verification of SIM PIN-2 before selecting any SIM related storages,    *
          * if it is already not done.                                              *
          * But now as password is provided by CPBS command. It is used for the     * 
          * verification if it not done.                                            * 
          *-------------------------------------------------------------------------*/

          
          pSIMSetPrm = &simShrdPrm.setPrm[srcId];

          cmhSIM_FillInPIN ( (char *)pin2, pSIMSetPrm -> curPIN,
                           PIN_LEN);
          pSIMSetPrm -> PINType = PHASE_2_PIN_2;
          simEntStat.curCmd  = AT_CMD_CPBS;
          simShrdPrm.owner = (T_OWN)srcId;
          simEntStat.entOwn  = srcId;

         /*---------------------------------------------------------------------*
          *   During verification is the Owner ID is worng, then return the     *
          *   AT_FAIL response.                                                 *
          *---------------------------------------------------------------------*/	   
          if ( psaSIM_VerifyPIN() < 0 )  
          {
            TRACE_EVENT( "FATAL RETURN psaSIM in +CPBS: Not a valid owner ID" );
            return AT_FAIL;
          }

         /*----------------------------------------------------------------*
          * Hold the storage types in the temprory fields unless SIM PIN-2 *
          * confirmation comes.                                            * 
          *----------------------------------------------------------------*/

          pPHBCmdPrm ->temp_cmhStor  = mem;
          pPHBCmdPrm ->temp_phbStor  = chkMem;

          return ( AT_EXCT );
        }
        else
        {
          pPHBCmdPrm -> cmhStor = mem;
          pPHBCmdPrm -> phbStor = chkMem;
        }
      }
    }
    else
    { 
      ACI_ERR_DESC( ACI_ERR_CLASS_Ext, EXT_ERR_Parameter );
      return( AT_FAIL );
    }
  }

  return ( AT_CMPL );
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)    MODULE  : CMH_PHBS                     |
| STATE   : code             ROUTINE : sAT_PlusCPBW                 |
+-------------------------------------------------------------------+

  PURPOSE : This is the functional counterpart of the +CPBW
            AT command which is responsible for writing a phonebook
            entry to memory.

            <index>:  location number of phonebook memory
            <number>: phone number
            <type>:   type of phone number
            <text>:   text associated with phone number 
*/
GLOBAL void cmhPHB_ksdDecodeToa(CHAR *number, CHAR **pNumber, T_PHB_RECORD *entry)
{
  T_KSD_SEQGRP   seqGrp                  = SEQGRP_UNKNOWN;
  T_KSD_SEQPARAM seqPrm;
  CHAR*          restSeq;
  T_ACI_TOA      stdToa;
  CHAR           *numBuf = NULL;  /* Phonebook number ASCII + '\0'  */

  TRACE_FUNCTION ("cmhksd_decode_toa()");
  ACI_MALLOC(numBuf, MAX_PHB_NUM_LEN);

  if(numBuf EQ NULL)
  {
    TRACE_EVENT("numbuff is NULL... returning from cmhPHB_ksdDecodeToa()");
    return;
  }
  memset( numBuf, 0, MAX_PHB_NUM_LEN);
  if ( strlen (number) EQ 0 )
  {
    TRACE_EVENT("number has length 0");
    return;
  }

  stdToa.ton = TON_Unknown;
  stdToa.npi = NPI_IsdnTelephony;

  strncpy ( numBuf, number, MAX_PHB_NUM_LEN - 1 );
  numBuf[MAX_PHB_NUM_LEN - 1] = '\0';
  

  if ( ksd_decode ( numBuf,
                    TRUE,
                    &seqGrp,
                    &restSeq,
                    &seqPrm ) EQ FALSE )
  {
    TRACE_EVENT("ksd_decode returned FALSE");
    return;
  }
    
  switch (seqGrp)
  {
    case (SEQGRP_USSD):
      if ( cmhPHB_rplcIntnl ( number, 
                              numBuf,
                              0xFF       ) EQ TRUE )
      {
        stdToa.ton = TON_International;
      }

      *pNumber = numBuf;/*lint !e684 (Warning:Passing address of auto variable 'numBuf' into caller space)*/
      cmhPHB_toaMrg ( &stdToa, &entry->ton_npi );
      break;
                          
    case (SEQGRP_DIAL):
      if ( number[0] EQ '+' )
      {
        *pNumber    = &number[1];
        stdToa.ton = TON_International;
      }
      /* 00 is not an indication for an international number */
      cmhPHB_toaMrg ( &stdToa, &entry->ton_npi ); 
      break;
                          
    case (SEQGRP_CF):
      {
        UBYTE cnt;

        if ( seqPrm.cf.num EQ NULL )
        {
          break;
        }

        if ( number [0] EQ '*' AND
             number [1] EQ '*'     )
        {
          cnt = 3;
        }
        else if ( number[0] EQ '*' )
        {
          cnt = 2;
        }
        else
        {
          cnt = 1;
        }

        if ( cmhPHB_rplcIntnl ( number, 
                                numBuf,
                                cnt       ) EQ TRUE )
        {  
          stdToa.ton = TON_International;
        }

        *pNumber = numBuf;
        cmhPHB_toaMrg ( &stdToa, &entry->ton_npi ); 
      }
      break;
                          
    case (SEQGRP_SUP_CLIR):
    case (SEQGRP_INV_CLIR):
      if (strlen (restSeq) NEQ 0)
      {
        if ( restSeq[0] EQ '+' )
        {
          strcpy ( &numBuf[4], &restSeq[1] );
          stdToa.ton = TON_International;
        }
        /* 00 is not an international number */
        else
        {
          strcpy ( &numBuf[4], &restSeq[0] );
        }

        *pNumber = numBuf;
        cmhPHB_toaMrg ( &stdToa, &entry->ton_npi ); 
      }
      break;
                          
    default:
      break;
  }
}

LOCAL T_ACI_RETURN cmhPHB_add_record(T_ACI_CMD_SRC       srcId,
                                     T_PHB_CMH_SNGL_STAT indexStat,
                                     T_ACI_PB_TEXT       *text,
                                     CHAR                *number,
                                     T_ACI_TOA           *type,
                                     SHORT               index,
                                     T_ACI_VP_ABS        *dateTime,
                                     UBYTE               phonebook_storage)
{
  T_PHB_RECORD entry;
  CHAR         *pNumber = NULL; /* pointer to the number that will be saved */
  T_PHB_RETURN result;

  TRACE_FUNCTION ("cmhPHB_add_record()");

  switch ( indexStat )
  {
    case ( SNGL_NOT_PRSNT ):
    case ( SNGL_VLD_PRSNT ):
      {
        /* fill in the structure elements */

        /* process the <index> */
#ifdef TI_PS_FFS_PHB
        entry.phy_recno = ( indexStat EQ SNGL_NOT_PRSNT ? 
                                           0 : ( UBYTE ) index );
#else
        entry.index = ( indexStat EQ SNGL_NOT_PRSNT ? 
                                           0 : ( UBYTE ) index );
#endif
        
        /* process the <tag> */
        if (text->len > PHB_MAX_TAG_LEN)
        {
          ACI_ERR_DESC( ACI_ERR_CLASS_Cme, CME_ERR_TxtToLong );
          return (AT_FAIL);
        }

        cmhPHB_getMfwTagSim ( text, entry.tag, &entry.tag_len, 
                              PHB_MAX_TAG_LEN );

        /* calculate the TON/NPI field */
        entry.ton_npi = PHB_TONPI_NO_DIAL;
        pNumber       = number;

        if ( type EQ NULL )
        {
          cmhPHB_ksdDecodeToa(number, &pNumber, &entry);
        }
        else
        {
          cmhPHB_toaMrg ( type, &entry.ton_npi );
        }

        /* process the <number> */
        cmhPHB_getAdrBcd ( entry.number, &entry.len,
                           PHB_PACKED_NUM_LEN, pNumber );
/*        if (pNumber NEQ NULL)
        {
	        ACI_MFREE(pNumber);        
          pNumber = NULL;
        }
*/		
#if defined(TI_PS_FFS_PHB) OR defined(PHONEBOOK_EXTENSION)
        /* Clear the subaddress (until fully supported here) */
        memset (entry.subaddr, 0xFF, sizeof (entry.subaddr));
#endif

        /* process the <cc_id>  */
        entry.cc_id     = PHB_CMH_CC_ID_NOT_USED;

        /* process the <extension> */
/*          extension Mechansim not yet supported 
        entry.extension = PHB_CMH_EXT_NOT_USED; */

        /* process the <date and time> */
        if ( dateTime EQ NULL                 OR
             !cmhSMS_isVpabsVld ( dateTime )    )
        {
          cmhPHB_invldPhbDateTime ( &entry );
        }
        else
        {
          cmhPHB_cpyCmhDateTime ( dateTime, &entry );
        }

        /* add the record */
        result = pb_add_record ((T_PHB_TYPE)phonebook_storage,
#ifdef TI_PS_FFS_PHB

                                (UBYTE)entry.phy_recno,
#else
                                entry.index,
#endif
                                &entry);
        switch (result)
        {
          case PHB_EXCT:
            return (AT_EXCT);
            
          case PHB_OK:
            return (AT_CMPL);                
            
          case PHB_FULL:
            ACI_ERR_DESC( ACI_ERR_CLASS_Cme, CME_ERR_MemFull );
            return (AT_FAIL);

          case PHB_EXT_FULL:
            ACI_ERR_DESC( ACI_ERR_CLASS_Cme, CME_ERR_SimNoExtAvail);
            return (AT_FAIL);

          case PHB_TAG_EXCEEDED:
            ACI_ERR_DESC( ACI_ERR_CLASS_Cme, CME_ERR_TxtToLong );
            return (AT_FAIL);

#ifdef TI_PS_FFS_PHB
          case PHB_LOCKED:
            return (AT_BUSY);
#endif

          case PHB_FAIL:
          default:
            ACI_ERR_DESC( ACI_ERR_CLASS_Cme, CME_ERR_Unknown );
            return (AT_FAIL);
        }
      }

    case ( SNGL_INVLD_PRSNT ):
      ACI_ERR_DESC( ACI_ERR_CLASS_Ext, EXT_ERR_Parameter );
      return( AT_FAIL );
  }
  return(AT_FAIL);
}


GLOBAL T_ACI_RETURN sAT_PlusCPBW ( T_ACI_CMD_SRC  srcId,
                                   SHORT          index,
                                   CHAR*          number,
                                   T_ACI_TOA*     type,
                                   T_ACI_PB_TEXT* text,
                                   T_ACI_VP_ABS*  dateTime )
{
  T_PHB_CMD_PRM * pPHBCmdPrm; /* points to PHB command parameters  */
  
  T_PHB_CMH_SNGL_STAT indexStat; /* status of parameter <index>    */
  T_PHB_CMH_CMN_STAT  entryStat; /* status of parameter <number>,  */
                                 /* <type> and <text>              */
  T_PHB_RETURN        result;
  T_ACI_RETURN        aci_return;
  UBYTE               pb_status;

#ifndef TI_PS_FFS_PHB
  UBYTE               ext_rcd_num = 0xFF;
#endif

  TRACE_FUNCTION ("sAT_PlusCPBW ()");

#ifndef NO_ASCIIZ
    if ( text NEQ NULL )
    {
      UBYTE   tmpBuf[MAX_ALPHA_LEN];
      USHORT  len;

      if (text->data[0] NEQ 0x80 AND text->data[0] NEQ 0x81
        AND text->data[0] NEQ 0x82)
      {
        text->cs = CS_Sim;
        cmh_cvtToDefGsm ( (CHAR*)text->data, (CHAR*)tmpBuf, &len );
        text->len = (UBYTE)len;
        memcpy ( text->data, tmpBuf, text->len );
      }
    }
#endif /* #ifndef NO_ASCIIZ */

  /* check command source */  
  if(!cmh_IsVldCmdSrc (srcId)) 
  { 
    return( AT_FAIL );
  }

  /* If the phonebook status is not PHB_READY then SIM BUSY error is indicated to the user.*/
  pb_status_req(&pb_status); /* get phone book status */

  TRACE_EVENT_P1("Current phonebook status: %d", pb_status);
  
  if(pb_status EQ PHB_BUSY)
  {
      return AT_BUSY;
  }
  else if(pb_status EQ PHB_UNKNOWN)
  {
      ACI_ERR_DESC( ACI_ERR_CLASS_Cme, CME_ERR_Unknown );
      return AT_FAIL;
  }

  pPHBCmdPrm = &cmhPrm[srcId].phbCmdPrm;

   /* check preselected phonebook storage */  
  if ( pPHBCmdPrm -> cmhStor EQ PB_STOR_NotPresent )
  { 
    ACI_ERR_DESC( ACI_ERR_CLASS_Ext, EXT_ERR_Parameter );
    return( AT_FAIL );
  }

  /* check preselected phonebook storage for FDN or BDN */  
  if ( ( simShrdPrm.pn2Stat EQ PS_PIN2       OR
         simShrdPrm.pn2Stat EQ NO_VLD_PS        )
         AND
       ( pPHBCmdPrm -> cmhStor EQ PB_STOR_Fd OR
         pPHBCmdPrm -> cmhStor EQ PB_STOR_Bd    ) )
  { 
    if (  simShrdPrm.PINStat EQ PS_RDY )
    {
      simShrdPrm.PINStat = PS_PIN2;
    }
    ACI_ERR_DESC( ACI_ERR_CLASS_Cme, CME_ERR_SimPin2Req );
    return( AT_FAIL );
  }

  /* process the status of parameter <index> */
  if ( index EQ ACI_NumParmNotPresent )
  {
    indexStat = SNGL_NOT_PRSNT;
  }
  else if( index > PHB_CMH_IDX_MAX OR 
           index < PHB_CMH_IDX_MIN )
  {
    indexStat = SNGL_INVLD_PRSNT;
  }
  else
  {
    indexStat = SNGL_VLD_PRSNT;
  }

  /* process the status of parameter <number>, <type> and <text> */
  if ( number EQ NULL AND 
       type   EQ NULL AND 
       text   EQ NULL )
  {
    entryStat = CMN_NOT_PRSNT;
  }
  else if ( number NEQ NULL AND 
            text   NEQ NULL )
  {
    entryStat = CMN_FULL_PRSNT;
  }
  else
  {
    entryStat = CMN_PRTLY_PRSNT;
  }

  /*
   *-----------------------------------------------------------------
   * process the parameter:
   * 
   * A -> number, type, text
   * B -> index
   * 
   * 0 -> all elements not present
   * 1 -> all elements present
   *
   * A | B | result
   * --+---+---------------------------
   * 0 | 0 | fail
   * 0 | 1 | delete entry
   * 1 | 0 | write to first empty entry
   * 1 | 1 | write to specific entry
   *-----------------------------------------------------------------
   */  
  switch ( entryStat )
  {
    /* process variable <entryStat> with value <CMN_FULL_PRSNT> */
    case ( CMN_FULL_PRSNT):
    case ( CMN_PRTLY_PRSNT ):
      aci_return = cmhPHB_add_record(srcId, indexStat, text, number, type, index, dateTime, pPHBCmdPrm->phbStor);
      if(aci_return EQ AT_EXCT)
      {
        pPHBCmdPrm->curCmd = AT_CMD_CPBW;
      }
      return(aci_return);

    /* process variable <entryStat> with value <CMN_NOT_PRSNT> */
    case ( CMN_NOT_PRSNT ):

      if ( indexStat NEQ SNGL_VLD_PRSNT )
      { 
        ACI_ERR_DESC( ACI_ERR_CLASS_Ext, EXT_ERR_Parameter );
        return( AT_FAIL );
      }

#ifdef TI_PS_FFS_PHB
      result = pb_del_record ((T_PHB_TYPE)pPHBCmdPrm->phbStor,index);
#else
      result = pb_delete_record (pPHBCmdPrm->phbStor,
                                 (UBYTE) index,
                                 &ext_rcd_num,
                                 TRUE);
#endif

      switch (result)
      {
        case PHB_EXCT:
          pPHBCmdPrm->curCmd = AT_CMD_CPBW;
          return (AT_EXCT);

        case PHB_OK:
          return (AT_CMPL);                

#ifdef TI_PS_FFS_PHB
        case PHB_LOCKED:
          return (AT_BUSY);
#endif

        case PHB_FAIL:
          ACI_ERR_DESC( ACI_ERR_CLASS_Cme, CME_ERR_InvIdx );
          return (AT_FAIL);
        
        default:
          ACI_ERR_DESC( ACI_ERR_CLASS_Cme, CME_ERR_Unknown );
          return (AT_FAIL);
      }
  }
  
  return ( AT_CMPL );  
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)    MODULE  : CMH_PHBS                     |
| STATE   : code             ROUTINE : sAT_PlusCPBR                 |
+-------------------------------------------------------------------+

  PURPOSE : This is the functional counterpart of the +CPBR
            AT command which is responsible for reading phonebook
            entries from memory.

            <startIdx>: index of first location to be read
            <stopIdx>:  index of last location to be read
            <lastIdx>:  index of last location already read
            <pbLst>:    list of phonebook entries
*/
GLOBAL T_ACI_RETURN sAT_PlusCPBR ( T_ACI_CMD_SRC  srcId,
                                   SHORT          startIdx,
                                   SHORT          stopIdx,
                                   SHORT*         lastIdx,
                                   T_ACI_PB_ENTR* pbLst)
{
  TRACE_FUNCTION ("sAT_PlusCPBR ()");

  return cmhPHB_PlusCPBR ( srcId,
                           SR_TYP_Physical,
                           startIdx,
                           stopIdx,
                           lastIdx,
                           pbLst );
}



/*
  PURPOSE : This is the functional counterpart of the %CPBR
            AT command which is responsible for reading phonebook
            entries from memory.

**************************************************
Added by Shen,Chao March.18th.2003
**************************************************
*/
GLOBAL T_ACI_RETURN sAT_PercentCPBR ( T_ACI_CMD_SRC srcId,
                                SHORT startIdx,
                                SHORT stopIdx,
                                T_ACI_SR_TYP searchMode,
                                SHORT* lastIdx,
                                T_ACI_PB_ENTR* pbLst )
{
  TRACE_FUNCTION ("sAT_PercentCPBR ()");

  return cmhPHB_PlusCPBR ( srcId,
                           searchMode,
                           startIdx,
                           stopIdx,
                           lastIdx,
                           pbLst );
}

#ifdef TI_PS_FFS_PHB
/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)    MODULE  : CMH_PHBS                     |
| STATE   : code             ROUTINE : cmhPHB_read_index_record     |
+-------------------------------------------------------------------+

  PURPOSE : This function reads the nth non-free record determined
            by index.
            This function could be implemented more efficiently
            within the phonebook itself, on the other hand, 
            it is not used in the code for the big phonebooks like ADN,
            so we should be able to live with this at least for a while.

*/
LOCAL T_PHB_RETURN cmhPHB_read_index_record (T_PHB_TYPE type,
                                             SHORT index,
                                             T_PHB_RECORD *entry)
{
  T_PHB_RETURN phb_result;
  USHORT phy_recno;

  TRACE_FUNCTION ("cmhPHB_read_index_record()");

  if (index < PHB_CMH_IDX_MIN)
    return PHB_INVALID_IDX;

  phy_recno = PHB_CMH_IDX_MIN;

  do
  {
    do 
    {
      phb_result = pb_read_record (type, phy_recno, entry);
      phy_recno++;
    } 
    while (phb_result EQ PHB_EMPTY_RECORD);
    index--;
  }
  while (index NEQ 0);
  return phb_result;
}
#endif

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)    MODULE  : CMH_PHBS                     |
| STATE   : code             ROUTINE : cmhPHB_PlusCPBR              |
+-------------------------------------------------------------------+

  PURPOSE : This is the functional counterpart of the +CPBR
            AT command which is responsible for reading phonebook
            entries from memory.

            <type>:     search type
            <startIdx>: index of first location to be read
            <stopIdx>:  index of last location to be read
            <lastIdx>:  index of last location already read
            <pbLst>:    list of phonebook entries
*/
GLOBAL T_ACI_RETURN cmhPHB_PlusCPBR ( T_ACI_CMD_SRC  srcId,
                                      T_ACI_SR_TYP   type,
                                      SHORT          startIdx,
                                      SHORT          stopIdx,
                                      SHORT*         lastIdx,
                                      T_ACI_PB_ENTR* pbLst)
{
  T_PHB_CMD_PRM * pPHBCmdPrm; /* points to PHB command parameters */
  SHORT        toIdx;         /* last used index                  */
  SHORT        maxIdx;        /* holds upper index bound          */
  SHORT        i;             /* used for counting                */
  SHORT        readRec = 0;   /* records already read             */
  T_PHB_RECORD rec;           /* read record                      */
  T_FCT_READ*  readFct;       /* selected read function           */
  T_PHB_RETURN result;
#ifdef TI_PS_FFS_PHB
  T_PHB_TYPE   book;          /* Phonebook                        */
  SHORT        dummy_used_rcd;
  UBYTE        dummy_num_len;
  UBYTE        dummy_tag_len;
  SHORT        dummy_max_ext;
  SHORT        dummy_used_ext;
#endif

  /*
   *-----------------------------------------------------------------
   * check command source
   *-----------------------------------------------------------------
   */  
  if(!cmh_IsVldCmdSrc (srcId)) 
  { 
    return( AT_FAIL );
  }

  pPHBCmdPrm = &cmhPrm[srcId].phbCmdPrm;

  /*
   *-----------------------------------------------------------------
   * check preselected phonebook storage
   *-----------------------------------------------------------------
   */  
#ifdef TI_PS_FFS_PHB
  if (!cmhPHB_cvtPhbType (pPHBCmdPrm->cmhStor, &book))
#else
  if ( pPHBCmdPrm -> cmhStor EQ PB_STOR_NotPresent )
#endif
  { 
    ACI_ERR_DESC( ACI_ERR_CLASS_Ext, EXT_ERR_Parameter );
    return( AT_FAIL );
  }

  /*
   *-----------------------------------------------------------------
   * check input parameter <startIdx>
   *-----------------------------------------------------------------
   */
#ifdef TI_PS_FFS_PHB
  (void)pb_read_sizes (book, &maxIdx, 
                       &dummy_used_rcd, &dummy_num_len, &dummy_tag_len,
                       &dummy_max_ext, &dummy_used_ext);
#else
  if( pPHBCmdPrm->cmhStor EQ PB_STOR_Af ) /* ADN-FDN phb */
     maxIdx = MAX_AFB_RECORDS;
  else 
     maxIdx = PHB_CMH_IDX_MAX;
#endif

  if( startIdx > maxIdx OR startIdx < PHB_CMH_IDX_MIN )
  { 
    ACI_ERR_DESC( ACI_ERR_CLASS_Cme, CME_ERR_InvIdx );
    return( AT_FAIL );
  }

  /*
   *-----------------------------------------------------------------
   * check input parameter <stopIdx>
   *-----------------------------------------------------------------
   */  
  if ( stopIdx EQ ACI_NumParmNotPresent )

    toIdx = startIdx;
  
  else
  {
    if( stopIdx > maxIdx OR stopIdx < PHB_CMH_IDX_MIN OR
        stopIdx < startIdx )
    { 
      ACI_ERR_DESC( ACI_ERR_CLASS_Cme, CME_ERR_InvIdx );
      return( AT_FAIL );
    }

    else

      toIdx = stopIdx;
  }

  /*
   *-----------------------------------------------------------------
   * calculating the read function
   *-----------------------------------------------------------------
   */  
  switch ( type )
  {
    case ( SR_TYP_Name     ): readFct = pb_read_alpha_record;  break;
    case ( SR_TYP_Number   ): readFct = pb_read_number_record; break;
#ifdef TI_PS_FFS_PHB
    /*case ( SR_TYP_Index    ): readFct = pb_read_index_record;  break;*/
    case ( SR_TYP_Index    ): readFct = cmhPHB_read_index_record;  break;
    case ( SR_TYP_Physical ): readFct = pb_read_record;        break;
#else
    case ( SR_TYP_Index    ): readFct = pb_read_index_record;  break;
    case ( SR_TYP_Physical ): readFct = pb_read_phys_record;   break;
#endif

    default:
      ACI_ERR_DESC( ACI_ERR_CLASS_Ext, EXT_ERR_Parameter );
      return( AT_FAIL );
  }
    
  /*
   *-----------------------------------------------------------------
   * reading of records
   *-----------------------------------------------------------------
   */  
  i = startIdx;
  
  while ( i <= toIdx AND readRec < MAX_PB_ENTR )
  {
    result = readFct ((T_PHB_TYPE)pPHBCmdPrm->phbStor, i, &rec);
    /* check for invalid index */
    if ((result EQ PHB_INVALID_IDX) AND (i EQ startIdx))
    {
      ACI_ERR_DESC (ACI_ERR_CLASS_Cme, CME_ERR_InvIdx);
      return( AT_FAIL );
    }
    if (result EQ PHB_OK)
    {
       cmhPHB_cpyRecEntr (&pbLst[readRec], 
#ifdef TI_PS_FFS_PHB
                          (T_PHB_TYPE)pPHBCmdPrm->phbStor,
#endif
                          &rec);

       readRec++;
    }
    if ( i EQ toIdx) /* this check is used in order to prevent the index = 0xFF */
    {
      i++;
      break;
    }
    else
      i++;      
  }

  /*
   *-----------------------------------------------------------------
   * mark the first invalid entry
   *-----------------------------------------------------------------
   */  
  if ( readRec < MAX_PB_ENTR )

    cmhPHB_invldEntr ( &pbLst[readRec] );

  /*
   *-----------------------------------------------------------------
   * process the parameter <lastIdx>
   *-----------------------------------------------------------------
   */  
  /* *lastIdx = i - 1; */
  *lastIdx =(SHORT)(i - 1); /* set -1 to 255 */
  
  return ( AT_CMPL );
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)    MODULE  : CMH_PHBS                     |
| STATE   : code             ROUTINE : sAT_PlusCPBF                 |
+-------------------------------------------------------------------+

  PURPOSE : This is the functional counterpart of the +CPBF
            AT command which is responsible for finding phonebook
            entries in memory.

            <findText>: search string
            <mode>:     search mode
            <found>:    number of entries found
            <pbLst>:    list of found phonebook entries
*/
#ifdef NO_ASCIIZ
GLOBAL T_ACI_RETURN sAT_PlusCPBF ( T_ACI_CMD_SRC  srcId,
                                   T_ACI_PB_TEXT  *findtext,
                                   T_ACI_CPBF_MOD mode,
                                   SHORT          *found,
                                   T_ACI_PB_ENTR  *pbLst )
#else /* ifdef NO_ASCIIZ */
GLOBAL T_ACI_RETURN sAT_PlusCPBF ( T_ACI_CMD_SRC  srcId,
                                   CHAR           *findtext_str,
                                   T_ACI_CPBF_MOD mode,
                                   SHORT          *found,
                                   T_ACI_PB_ENTR  *pbLst )
#endif /* ifdef NO_ASCIIZ */
{
  SHORT         firstIdx; /* dummy variable, not used */
  T_ACI_PB_TEXT *find_text;
#ifndef NO_ASCIIZ
  T_ACI_PB_TEXT findtext;
#endif /* #ifndef NO_ASCIIZ */

  TRACE_FUNCTION ("sAT_PlusCPBF ()");

#ifdef NO_ASCIIZ
  find_text = findtext;
#else /* ifdef NO_ASCIIZ */
  find_text = &findtext;

  if ( findtext_str NEQ NULL ) 
  {
    USHORT len = (USHORT)strlen( findtext_str );

    cmh_cvtToDefGsm ( findtext_str, (CHAR*)find_text->data, &len );
    find_text->cs = CS_Sim;
    find_text->len = (UBYTE)len;
  }
  else
  {
    find_text->cs = CS_NotPresent;
    find_text->len = 0;
  }
#endif /* ifdef NO_ASCIIZ */

 return (cmhPHB_PlusCPBF ( srcId,
                           find_text,
                           SR_TYP_Name,
                           mode,
                           &firstIdx,
                           found,
                           pbLst ));
}



#ifdef NO_ASCIIZ
GLOBAL T_ACI_RETURN sAT_PercentCPBF ( T_ACI_CMD_SRC srcId,
                                T_ACI_PB_TEXT* findtext,
                                T_ACI_CPBF_MOD mode,
                                T_ACI_SR_TYP searchMode, 
   				    U8 direction,
                                SHORT* found,
                                T_ACI_PB_ENTR* pbLst )
                                
#else /* ifdef NO_ASCIIZ */

GLOBAL T_ACI_RETURN sAT_PercentCPBF ( T_ACI_CMD_SRC srcId,
                                T_ACI_PB_TEXT* findtext,
                                T_ACI_CPBF_MOD mode,
                                T_ACI_SR_TYP searchMode, 
   				    U8 direction,
                                SHORT* found,
                                T_ACI_PB_ENTR* pbLst )
#endif /* ifdef NO_ASCIIZ */
{
  SHORT         firstIdx; /* dummy variable, not used */
  T_ACI_PB_TEXT *find_text;
#ifndef NO_ASCIIZ
  T_ACI_PB_TEXT findtext;
#endif /* #ifndef NO_ASCIIZ */

  TRACE_FUNCTION ("sAT_PercentCPBF ()");

#ifdef NO_ASCIIZ
  find_text = findtext;
#else /* ifdef NO_ASCIIZ */
  find_text = &findtext;

  if ( findtext_str NEQ NULL ) 
  {
    USHORT len = (USHORT)strlen( findtext_str );

    cmh_cvtToDefGsm ( findtext_str, (CHAR*)find_text->data, &len );
    find_text->cs = CS_Sim;
    find_text->len = (UBYTE)len;
  }
  else
  {
    find_text->cs = CS_NotPresent;
    find_text->len = 0;
  }
#endif /* ifdef NO_ASCIIZ */

 return (cmhPHB_PlusCPBF ( srcId,
                           find_text,
                           searchMode,
                           mode,
                           &firstIdx,
                           found,
                           pbLst ));
}


/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)    MODULE  : CMH_PHBS                     |
| STATE   : code             ROUTINE : cmhPHB_PlusCPBF              |
+-------------------------------------------------------------------+

  PURPOSE : This is the functional counterpart of the +CPBF
            AT command which is responsible for finding phonebook
            entries in memory.

            <findText>: search string
            <type>:     search type
            <mode>:     search mode
            <firstIdx>: first found index
            <found>:    number of entries found
            <pbLst>:    list of found phonebook entries
            
  HM 7-Sep-2005: Warning, MFW is using this function directly!

*/
GLOBAL T_ACI_RETURN cmhPHB_PlusCPBF ( T_ACI_CMD_SRC   srcId,
                                      T_ACI_PB_TEXT   *findtext,
                                      T_ACI_SR_TYP    type,
                                      T_ACI_CPBF_MOD  mode,
                                      SHORT*          firstIdx,
                                      SHORT*          found,
                                      T_ACI_PB_ENTR*  pbLst )
{
  T_PHB_CMD_PRM * pPHBCmdPrm; /* points to PHB command parameters */
#ifdef TI_PS_FFS_PHB
  T_PHB_MATCH  match_criteria;
#else
  UBYTE        cvtMode;       /* converted parameter <mode>       */
#endif
  USHORT       i;             /* used for counting                */
  T_PHB_RECORD fndRec;        /* found records                    */
  T_PHB_RETURN res;

  /*
   *-----------------------------------------------------------------
   * check command source
   *-----------------------------------------------------------------
   */  
  if(!cmh_IsVldCmdSrc (srcId)) 
  { 
    return( AT_FAIL );
  }

  pPHBCmdPrm = &cmhPrm[srcId].phbCmdPrm;

  /*
   *-----------------------------------------------------------------
   * check preselected phonebook storage
   *-----------------------------------------------------------------
   */  
  if ( pPHBCmdPrm -> cmhStor EQ PB_STOR_NotPresent )
  { 
    ACI_ERR_DESC( ACI_ERR_CLASS_Ext, EXT_ERR_Parameter );
    return( AT_FAIL );
  }

  /*
   *-----------------------------------------------------------------
   * check input parameter <findtext>
   *-----------------------------------------------------------------
   */  
  if ( ( findtext EQ NULL ) OR ( findtext->len EQ 0 ) )
  { 
    ACI_ERR_DESC( ACI_ERR_CLASS_Ext, EXT_ERR_Parameter );
    return( AT_FAIL );
  }

  /*
   *-----------------------------------------------------------------
   * check input parameter <mode>
   *-----------------------------------------------------------------
   */  
  switch ( mode )
  {
#ifdef TI_PS_FFS_PHB
    case CPBF_MOD_NewSearch:
    case CPBF_MOD_NextSearch:
      break;
#else
    case ( CPBF_MOD_NewSearch  ): cvtMode = PHB_NEW_SEARCH;  break;
    case ( CPBF_MOD_NextSearch ): cvtMode = PHB_NEXT_SEARCH; break;
#endif

    default:
      ACI_ERR_DESC( ACI_ERR_CLASS_Ext, EXT_ERR_Parameter );
      return( AT_FAIL );
  }

  /*
   *-----------------------------------------------------------------
   * searching for first records and reset information about found
   * and written records
   *-----------------------------------------------------------------
   */  
#ifdef TI_PS_FFS_PHB
  if ( mode EQ CPBF_MOD_NewSearch )
  {
    pPHBCmdPrm->order_num = 0;
    *firstIdx = 0;
    *found    = 0;
    switch ( type )
    {
      case ( SR_TYP_Name   ): 
        if (srcId EQ CMD_SRC_LCL)
          match_criteria = PHB_MATCH_GE;
        else
          match_criteria = PHB_MATCH_PARTIAL;

        res = pb_search_name_ex ((T_PHB_TYPE)pPHBCmdPrm -> phbStor,
                                 match_criteria,
                                 findtext,
                                 firstIdx,
                                 found,
                                 &fndRec);
        break;

      case ( SR_TYP_Number ): 
        res = pb_search_number_ex ((T_PHB_TYPE)pPHBCmdPrm -> phbStor,
                                   findtext->data,
                                   firstIdx,
                                   found,
                                   &fndRec);
        break;

      default:
        ACI_ERR_DESC( ACI_ERR_CLASS_Ext, EXT_ERR_Parameter );
        return( AT_FAIL );
    }

    switch (res)
    {
      case PHB_OK:
        break;
    
      case PHB_LOCKED:
        return AT_BUSY;

      default:
        ACI_ERR_DESC( ACI_ERR_CLASS_Cme, CME_ERR_NotFound );
        return( AT_FAIL );
    }

    pPHBCmdPrm -> fndRec = *found;
    pPHBCmdPrm -> wrtRec = 0;
    i                    = 0;

    if ( pPHBCmdPrm -> fndRec > 0 )
    {
      cmhPHB_cpyRecEntr (&pbLst[i],
                        (T_PHB_TYPE)pPHBCmdPrm->phbStor,
                         &fndRec);

      pPHBCmdPrm -> wrtRec++;

      i++;
    }
    pPHBCmdPrm->order_num = *firstIdx + pPHBCmdPrm->wrtRec;
  }
  else
  {  
    /* CPBF_MOD_NextSearch */
    i = 0;
  }
#else
  if ( mode EQ CPBF_MOD_NewSearch )
  {
    *firstIdx = 0;
    *found    = 0;
    switch ( type )
    {
      case ( SR_TYP_Name   ): 
        res = pb_search_name ( srcId,
                               pPHBCmdPrm -> phbStor, 
                               findtext, 
                               cvtMode,
                               firstIdx,
                               found,
                               &fndRec                );   
        break;
      case ( SR_TYP_Number ): 
        res = pb_search_number ( pPHBCmdPrm -> phbStor, 
                                 findtext->data, 
                                 cvtMode,
                                 firstIdx,
                                 found,
                                 &fndRec                ); 
        break;
      default:
        ACI_ERR_DESC( ACI_ERR_CLASS_Ext, EXT_ERR_Parameter );
        return( AT_FAIL );
    }

    if ( res EQ PHB_FAIL )
    { 
      ACI_ERR_DESC( ACI_ERR_CLASS_Cme, CME_ERR_NotFound );
      return( AT_FAIL );
    }

    pPHBCmdPrm -> fndRec = *found;
    pPHBCmdPrm -> wrtRec = 0;
    i                    = 0;

    if ( pPHBCmdPrm -> fndRec > 0 )
    {
      cmhPHB_cpyRecEntr ( &pbLst[i], &fndRec );

      pPHBCmdPrm -> wrtRec++;

      i++;
    }
  }
  else
    
    i = 0;
#endif

  /*
   *-----------------------------------------------------------------
   * copying and searching of records
   *-----------------------------------------------------------------
   */  
  while ( i < MAX_PB_ENTR AND 
          pPHBCmdPrm -> wrtRec < pPHBCmdPrm -> fndRec)
  {
    switch ( type )
    {
#ifdef TI_PS_FFS_PHB
      case ( SR_TYP_Name   ): 
        res = pb_read_alpha_record ((T_PHB_TYPE)pPHBCmdPrm->phbStor,
                                    (SHORT)pPHBCmdPrm->order_num, 
                                    &fndRec);
        break;

      case ( SR_TYP_Number ): 
        res = pb_read_number_record ((T_PHB_TYPE)pPHBCmdPrm->phbStor,
                                     (SHORT)pPHBCmdPrm->order_num,
                                     &fndRec);
        break;
#else
      case ( SR_TYP_Name   ): 
        res = pb_search_name ( srcId,
                               pPHBCmdPrm -> phbStor, 
                               findtext, 
                               CPBF_MOD_NextSearch,
                               firstIdx,
                               found,
                               &fndRec                );   
        break;
      case ( SR_TYP_Number ): 
        res = pb_search_number ( pPHBCmdPrm -> phbStor, 
                                 findtext->data, 
                                 CPBF_MOD_NextSearch,
                                 firstIdx,
                                 found,
                                 &fndRec                ); 
        break;
#endif

      default:
        ACI_ERR_DESC( ACI_ERR_CLASS_Ext, EXT_ERR_Parameter );
        return( AT_FAIL );
    }
    if (res NEQ PHB_OK)
    { 
      pPHBCmdPrm->order_num = 0;
      ACI_ERR_DESC( ACI_ERR_CLASS_Cme, CME_ERR_NotFound );
      return( AT_FAIL );
    }

    cmhPHB_cpyRecEntr (&pbLst[i],
#ifdef TI_PS_FFS_PHB
                       (T_PHB_TYPE)pPHBCmdPrm->phbStor,
#endif
                       &fndRec);

    pPHBCmdPrm -> wrtRec++;
    pPHBCmdPrm -> order_num++;

    i++;
  }

  if (pPHBCmdPrm->wrtRec >= pPHBCmdPrm->fndRec) // #HM#
    pPHBCmdPrm->order_num = 0;                  // #HM#

  /*
   *-----------------------------------------------------------------
   * mark the first invalid entry
   *-----------------------------------------------------------------
   */  
  if ( i < MAX_PB_ENTR )

    cmhPHB_invldEntr ( &pbLst[i] );

  return ( AT_CMPL );
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)    MODULE  : CMH_PHBS                     |
| STATE   : code             ROUTINE : sAT_PercentPBCF              |
+-------------------------------------------------------------------+

  PURPOSE : This is the functional counterpart of the %PBCF
            AT command which is responsible for phone configuration.

            <ldn>: last dialed number configuration.
            <lrn>: last received number configuration.
*/
GLOBAL T_ACI_RETURN sAT_PercentPBCF( T_ACI_CMD_SRC srcId,
                                     T_ACI_PBCF_LDN ldn,
                                     T_ACI_PBCF_LRN lrn,
                                     T_ACI_PBCF_LMN lmn )
{

  TRACE_FUNCTION ("sAT_PercentPBCF ()");

/*
 *-----------------------------------------------------------------
 * process the <ldn> parameter
 *-----------------------------------------------------------------
 */  
  switch( ldn  )
  {
    case( PBCF_LDN_NotPresent ):

      ldn = (T_ACI_PBCF_LDN)PBCFldn;
      break;

    case( PBCF_LDN_Enable  ):
    case( PBCF_LDN_Disable ):

      break;

    default:
      ACI_ERR_DESC( ACI_ERR_CLASS_Ext, EXT_ERR_Parameter );
      return( AT_FAIL );
  }

/*
 *-----------------------------------------------------------------
 * process the <lrn> parameter
 *-----------------------------------------------------------------
 */  
  switch( lrn  )
  {
    case( PBCF_LRN_NotPresent ):

      lrn = PBCFlrn;
      break;

    case( PBCF_LRN_Enable  ):
    case( PBCF_LRN_Disable ):

      break;

    default:
      ACI_ERR_DESC( ACI_ERR_CLASS_Ext, EXT_ERR_Parameter );
      return( AT_FAIL );
  }

/*
 *-----------------------------------------------------------------
 * process the <lmn> parameter
 *-----------------------------------------------------------------
 */  
  switch( lmn  )
  {
    case( PBCF_LMN_NotPresent ):

      lmn = PBCFlmn;
      break;

    case( PBCF_LMN_Enable  ):
    case( PBCF_LMN_Disable ):

      break;

    default:
      ACI_ERR_DESC( ACI_ERR_CLASS_Ext, EXT_ERR_Parameter );
      return( AT_FAIL );
  }

/*
 *--------------------------------------------------------------------
 * assign the parameters
 *--------------------------------------------------------------------
 */  
  PBCFldn = ldn;
  PBCFlrn = lrn;
  PBCFlmn = lmn;

  return ( AT_CMPL );
}

/*
+---------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)    MODULE  : CMH_CCS                        |
| STATE   : code             ROUTINE : sAT_PlusCSVM                   |
+---------------------------------------------------------------------+

  PURPOSE : This is the functional counterpart of the +CSVM AT command
            which is responsible for Setting a Voice Mail Number.
           
            <mode>:   Enable or Disable the voice mail number
            <pnumber>: Number of the voice mail server
            <num_len>: Number length
            <type>:   type of number
*/

GLOBAL T_ACI_RETURN sAT_PlusCSVM ( T_ACI_CMD_SRC  srcId,
                                   T_ACI_CSVM_MOD mode,
                                   CHAR           *pnumber,
                                   UBYTE          num_len,
                                   T_ACI_TOA*     toa)
{
  CHAR*          ef          = EF_VMN_ID;
  UBYTE          toa_val= 0;
  
  T_PHB_CMD_PRM * pPHBCmdPrm; /* points to PHB command parameter */  
  EF_VMN vmn; 
  
  TRACE_FUNCTION ("sAT_PlusCSVM ()");
  /*
   *-----------------------------------------------------------------
   * check command source
   *-----------------------------------------------------------------
   */  
  if(!cmh_IsVldCmdSrc (srcId)) 
  { 
    return( AT_FAIL );
  }

  pPHBCmdPrm = &cmhPrm[srcId].phbCmdPrm;
  /*
   *-------------------------------------------------------------------
   * process the mode parameter
   *-------------------------------------------------------------------
   */
  switch( mode )
  {
    case( CSVM_MOD_NotPresent ):
      break;

    case( CSVM_MOD_Disable ):
    case( CSVM_MOD_Enable  ):
      
      pPHBCmdPrm -> CSVMmode = mode;
      break;

    default:
      ACI_ERR_DESC( ACI_ERR_CLASS_Cme, CME_ERR_Unknown );
      return( AT_FAIL );
  }
       
/*
 *-------------------------------------------------------------------
 * process the toa parameter
 *-------------------------------------------------------------------
 */
  if( ! toa ) 
    return( AT_CMPL );

  switch( toa -> ton )
  {
    case( TON_Unknown       ):
    case( TON_International ):
    case( TON_National      ):
    case( TON_NetSpecific   ):
    case( TON_DedAccess     ):
       
      break;

    default:
      ACI_ERR_DESC( ACI_ERR_CLASS_Cme, CME_ERR_Unknown );
      return( AT_FAIL );
  }

  switch( toa -> npi )
  {
    case( NPI_Unknown       ):
    case( NPI_IsdnTelephony ):
    case( NPI_Data          ):
    case( NPI_Telex         ):
    case( NPI_National      ):
    case( NPI_Private       ):
      
      break;

    default:
      ACI_ERR_DESC( ACI_ERR_CLASS_Cme, CME_ERR_Unknown );
      return( AT_FAIL );
  }
   
/*
 *-------------------------------------------------------------------
 * process the phone number 
 *-------------------------------------------------------------------
 */
 /*init*/
 memset( vmn.vmNum, 0xFF, sizeof(vmn.vmNum) );

 memcpy ( vmn.vmNum, pnumber, num_len );
 toa_val = toa_merge (*toa);
 
 vmn.numTp = toa_val;

 if (pcm_WriteFile((UBYTE*)ef, SIZE_EF_VMN, (UBYTE*)&vmn) EQ PCM_OK)
 {
   return( AT_CMPL );
 }
 else 
 {
   ACI_ERR_DESC( ACI_ERR_CLASS_Cme, CME_ERR_MemFail );
   return( AT_FAIL );
 }

}

#ifdef TI_PS_FFS_PHB
void        cmh_PHB_update_ext_record(UBYTE rec_num, BOOL flag )
{
             pb_sim_update_ext_bitmap(rec_num, flag); 
}
#endif

/*==== EOF ========================================================*/
