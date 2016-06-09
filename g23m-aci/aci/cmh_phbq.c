/* 
+----------------------------------------------------------------------------- 
|  Project :  GSM-PS (6147)
|  Modul   :  CMH_PHBQ
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
|  Purpose :  This module provides the query functions related to the 
|             protocol stack adapter for phonebook management.
+----------------------------------------------------------------------------- 
*/ 

#ifndef CMH_PHBQ_C
#define CMH_PHBQ_C
#endif

#include "config.h"
#include "fixedconf.h"
#include "condat-features.h"
#include "aci_conf.h"

#include "aci_all.h"
/*==== INCLUDES ===================================================*/
#include "aci_cmh.h"

#ifdef FAX_AND_DATA
#include "aci_fd.h"
#endif    /* of #ifdef FAX_AND_DATA */

#include "psa.h"
#include "psa_sim.h"
#include "phb.h"
#include "cmh.h"
#include "cmh_phb.h"
#include "pcm.h"
/*==== CONSTANTS ==================================================*/

/*==== EXPORT =====================================================*/

/*==== VARIABLES ==================================================*/

/*==== FUNCTIONS ==================================================*/
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_PHBQ                 |
| STATE   : code                  ROUTINE : cmh_QueryCPBS            |
+--------------------------------------------------------------------+

  PURPOSE : This is the functional counterpart to the +CPBS? and %CPBS?
            AT commands which return the current selected phonebook
            memrory storage.

            <storage>: pbonebook memory storage
            <used>:    number of used records
            <total>:   total number of records
            <first>:   first free location
*/
LOCAL T_ACI_RETURN cmh_QueryCPBS ( T_ACI_CMD_SRC  srcId,
                                   T_ACI_PB_STOR* storage,
                                   SHORT*         used,
                                   SHORT*         total,
                                   SHORT*         first,
                                   SHORT*         used_ext,
                                   SHORT*         total_ext)
{
  T_PHB_CMD_PRM * pPHBCmdPrm; /* points to PHB command parameter */

  TRACE_FUNCTION ("cmh_QueryCPBS ()");

  /*
   *-----------------------------------------------------------------
   * check command source
   *-----------------------------------------------------------------
   */  
  if( ! cmh_IsVldCmdSrc (srcId) ) 
  { 
    return( AT_FAIL );
  }

  pPHBCmdPrm = &cmhPrm[srcId].phbCmdPrm;

  *storage    = pPHBCmdPrm -> cmhStor;
  *used       = 0;
  *total      = 0;
  *first      = 0;
  *used_ext   = 0;
  *total_ext  = 0;

#ifdef TI_PS_FFS_PHB
  if ( pPHBCmdPrm -> cmhStor NEQ PB_STOR_NotPresent )
  {
    UBYTE dmyNumLen;
    UBYTE dmyTagLen;

    switch (simShrdPrm.pb_stat)
    {
      case PB_STAT_Ready:
        break;

      case PB_STAT_Busy:
        TRACE_EVENT("Error: Phonebook busy accessing SIM");
        ACI_ERR_DESC(ACI_ERR_CLASS_Cme, CME_ERR_SimBusy);
        return AT_FAIL;

      case PB_STAT_Blocked:
      default:
        TRACE_EVENT("Error: Phonebook blocked");
        ACI_ERR_DESC(ACI_ERR_CLASS_Cme,CME_ERR_Unknown);
        return AT_FAIL;
    }

    switch (pb_read_sizes ((T_PHB_TYPE)pPHBCmdPrm->phbStor,
                           total,
                           used,
                           &dmyNumLen,
                           &dmyTagLen,
                           total_ext,
                           used_ext))
    {
      case PHB_OK:
        break;

      case PHB_FAIL:
      default:
        return (AT_FAIL); 
    }

    /*
     *   Now try and determine the first free location in the
     *   currently selected phonebook, but only if the output pointer
     *   is valid.
     */
    if (first NEQ NULL)
    {
      *first = (SHORT)pb_find_free_record ((T_PHB_TYPE)pPHBCmdPrm->phbStor);
      if (*first < 0)
      {
        ACI_ERR_DESC(ACI_ERR_CLASS_Cme,CME_ERR_Unknown);
        *first = 0;
      }
    }
  }
#else
  if ( pPHBCmdPrm -> cmhStor NEQ PB_STOR_NotPresent )
  {
    UBYTE dmySrvc;
    UBYTE dmyTagLen;
    SHORT dmyAvail;
    T_PHB_RETURN ret = PHB_FAIL;
    
    ret = pb_read_status (pPHBCmdPrm -> phbStor,
                          &dmySrvc,
                          total, 
                          used,
                          &dmyTagLen,
                          &dmyAvail,
                          total_ext,
                          used_ext);
    switch (ret)
    {
      case (PHB_FAIL):
      {
        if ((*used EQ 0) AND (*total EQ 0))
        {
          /*
           * this kind of phonebook has no entries, respectively no memory at all
           * output should be +CPBS: <storage>,0,0
           * and not +CME: 100  <=== unknown error
           */
          break;
        }
        return (AT_FAIL);
      }
      case (PHB_OK):
      {
        break;
      }
      default:
      {
        return (AT_FAIL); 
      }
    }

    /*
    *   Now try and determine the first free location in the
    *   currently selected phonebook, but only if the output pointer
    *   is valid.
    */
    if (first NEQ NULL)
    {
      if (ret EQ PHB_OK)
      {
        ret=pb_first_free(pPHBCmdPrm->phbStor,first);

        switch(ret)
        {
          default:
          case PHB_FAIL:
            /*
            *   It is assumed that pb_first_free() will have filled in
            *   the error number (ACI_ERR_DESC).
            */
            return(AT_FAIL);

          case PHB_FULL:
          case PHB_OK:
            break;
        }
      }
    }
  }
#endif

  return ( AT_CMPL );
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_PHBQ                 |
| STATE   : code                  ROUTINE : qAT_PercentCPBS          |
+--------------------------------------------------------------------+

  PURPOSE : This is the functional counterpart to the %CPBS? 
            AT command which returns the current selected phonebook
            memrory storage.

            <storage>:    phonebook memory storage
            <used>:       number of used records
            <total>:      total number of records
            <first>:      first free location
            <used_ext>:   Number of used extension records
            <total_ext>:  Number of total extension records
*/
GLOBAL T_ACI_RETURN qAT_PercentCPBS ( T_ACI_CMD_SRC  srcId,
                                      T_ACI_PB_STOR* storage,
                                      SHORT*         used,
                                      SHORT*         total,
                                      SHORT*         first,
                                      SHORT*         used_ext,
                                      SHORT*         total_ext)
{
  TRACE_FUNCTION ("qAT_PercentCPBS ()");

  return cmh_QueryCPBS (srcId,
                        storage,
                        used, total,
                        first,
                        used_ext, total_ext);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_PHBQ                 |
| STATE   : code                  ROUTINE : qAT_PlusCPBS             |
+--------------------------------------------------------------------+

  PURPOSE : This is the functional counterpart to the +CPBS? 
            AT command which returns the current selected phonebook
            memrory storage.

            <storage>: pbonebook memory storage
            <used>:    number of used records
            <total>:   total number of records
*/
GLOBAL T_ACI_RETURN qAT_PlusCPBS ( T_ACI_CMD_SRC  srcId,
                                   T_ACI_PB_STOR* storage,
                                   SHORT*         used,
                                   SHORT*         total )
{
  SHORT dummy_first;
  SHORT dummy_total_ext, dummy_used_ext;

  TRACE_FUNCTION ("qAT_PlusCPBS ()");

  return cmh_QueryCPBS (srcId,
                        storage,
                        used, total,
                        &dummy_first,
                        &dummy_total_ext, &dummy_used_ext);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_PHBQ                 |
| STATE   : code                  ROUTINE : qAT_PercentPBCF          |
+--------------------------------------------------------------------+

  PURPOSE : This is the functional counterpart to the %PBCF? 
            AT command which returns the current phonebook 
            configuration.

            <ldn>: last dialed number configuration.
            <lrn>: last received number configuration.
*/
GLOBAL T_ACI_RETURN qAT_PercentPBCF( T_ACI_CMD_SRC srcId,
                                     T_ACI_PBCF_LDN *ldn,
                                     T_ACI_PBCF_LRN *lrn,
                                     T_ACI_PBCF_LMN *lmn )
{

  TRACE_FUNCTION ("qAT_PercentPBCF ()");

/*
 *-----------------------------------------------------------------
 * fill in parameters
 *-----------------------------------------------------------------
 */  
  *ldn = PBCFldn;
  *lrn = PBCFlrn;
  *lmn = PBCFlmn;

  return ( AT_CMPL );
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_CCQ                  |
| STATE   : code                  ROUTINE : qAT_PlusCSVM             |
+--------------------------------------------------------------------+

  PURPOSE : This is the functional counterpart to the +CSVM? AT command
            which returns the current setting of mode, number and type.

            <mode>:   Enable or Disable the voice mail number
            <number>: Number of the voice mail server
            <num_len>:Number length
            <toa>:    type of address.
*/

GLOBAL T_ACI_RETURN qAT_PlusCSVM (T_ACI_CMD_SRC  srcId,
                                  T_ACI_CSVM_MOD *mode,
                                  CHAR           *number,
                                  UBYTE          num_len,
                                  SHORT          *toa_val)
                                
{
  CHAR*             ef = EF_VMN_ID;
  pcm_FileInfo_Type fileInfo;
  EF_VMN vmn;
    
  T_PHB_CMD_PRM * pPHBCmdPrm; /* points to PHB command parameter */ 
  
  TRACE_FUNCTION ("qAT_PlusCSVM()");
 
/*
 *-------------------------------------------------------------------
 * check command source
 *-------------------------------------------------------------------
 */  
  if(!cmh_IsVldCmdSrc (srcId)) 
  { 
    return( AT_FAIL );
  }

  pPHBCmdPrm = &cmhPrm[srcId].phbCmdPrm;

/*
 *-------------------------------------------------------------------
 * fill parameter mode
 *-------------------------------------------------------------------
 */  
   *mode= pPHBCmdPrm -> CSVMmode;

/*
 *-------------------------------------------------------------------
 * fill in parameters number
 *-------------------------------------------------------------------
 */   
  
   if (pcm_GetFileInfo ( ( UBYTE* ) ef, &fileInfo) NEQ PCM_OK)
   {
     ACI_ERR_DESC( ACI_ERR_CLASS_Cme, CME_ERR_MemFail );
     return( AT_FAIL );
   }
   else      
   { 
     if ( pcm_ReadFile ( ( UBYTE* )ef,fileInfo.FileSize,
                         ( UBYTE*) &vmn,
                          &fileInfo.Version) EQ PCM_OK )
     {
       USHORT i = 0;
       while (i<sizeof(vmn.vmNum))
       {
         if ((UBYTE)vmn.vmNum[i] EQ 0xFF)
           break;
         i++;
       }

       if( i>num_len)
       {
         TRACE_EVENT("Error: Number buffer is not big enough");
         ACI_ERR_DESC( ACI_ERR_CLASS_Cme, CME_ERR_Unknown );
         return (AT_FAIL);
       }

       if(i NEQ 0) /* there is a number on PCM */
       {
         memcpy (number, vmn.vmNum, i);
         *toa_val=(SHORT)vmn.numTp;
       }
       else
       {
         *toa_val = 145; /* default international, see 07.07 */
       }
       if (i<num_len)    /* Check if destination is big enough */
         number[i]='\0';
     }
     else 
     {
       ACI_ERR_DESC( ACI_ERR_CLASS_Cme, CME_ERR_MemFail );
       return( AT_FAIL );
     }
   }
  
  return( AT_CMPL );
}

#ifdef TI_PS_FFS_PHB
SHORT cmh_Query_free_ext_record()
{
   return((SHORT)pb_find_ext_free_record()); 
}
#endif	

/*==== EOF ========================================================*/
