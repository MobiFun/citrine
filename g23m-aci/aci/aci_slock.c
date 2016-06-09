/* 
+----------------------------------------------------------------------------- 
|  Project :  
|  Modul   :  J:\g23m-aci\aci\aci_slock.c
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

#ifdef SIM_PERS
#include "aci_all.h"

#include "aci_cmh.h"
#include "ati_cmd.h"
#include "aci_cmd.h"

#ifdef FAX_AND_DATA
#include "aci_fd.h"
#endif /* FAX_AND_DATA */

#include "aci.h"
#include "psa.h"
#include "psa_sim.h"
#include "psa_sms.h"
#include "psa_mmi.h"
#include "cmh.h"
#include "dti_conn_mng.h"
#include "cmh_sim.h"
#include "phb.h"
#include "aoc.h"

#include "psa_sim.h"            /* simShrdPrm */ 

#include "aci_ext_pers.h"       /* we are using personalisation extensions */
#include "aci_slock.h"          /* in order to asure interfaces */
#include "l4_tim.h"

/* Remember actual stati of already checked locks */
GLOBAL T_ACI_SLOCK_SHARED AciSLockShrd;

GLOBAL UBYTE sim_code_present_in_me; /* used by personalisation */
GLOBAL T_ACI_SIM_CONFIG aci_slock_sim_config;     /* SIM configuration, initialised by a T_SIM_MMI_INSERT_IND */

#include "general.h"  /* inluded for UINT8 compilation error in sec_drv.h */
#include "sec_drv.h" 

GLOBAL T_SEC_DRV_CONFIGURATION *cfg_data ;

EXTERN T_SEC_DRV_CATEGORY *personalisation_nw;

EXTERN T_SEC_DRV_CATEGORY *personalisation_ns;

EXTERN T_SEC_DRV_CATEGORY *personalisation_sp;

EXTERN T_SEC_DRV_CATEGORY *personalisation_cp;

EXTERN T_SEC_DRV_CATEGORY *personalisation_sim;

EXTERN T_SEC_DRV_CATEGORY *personalisation_bnw;


EXTERN T_SEC_DRV_CATEGORY *personalisation_first_sim;
EXTERN void  psaSAT_FUConfirm       ( int, USHORT );
void aci_slock_start_timer(void); 
const UBYTE mnc_mcc[6] = "00101"; /* 04-08-2005  */

LOCAL UBYTE aci_slock_sim_service_table [10];     /* SIM service table        */
LOCAL T_SIMLOCK_STATUS aci_slock_check_done(T_SIMLOCK_STATUS sl_status);

EXTERN T_SIM_MMI_INSERT_IND *last_sim_mmi_insert_ind;


/* 
 * Conversion Table for setting the CPIN response parameter
 */
const UBYTE aci_set_cpin_response [SIMLOCK_LAST][2] = {
{ CPIN_RSLT_PhNetPukReq    , CPIN_RSLT_PhNetPinReq    },
{ CPIN_RSLT_PhNetSubPukReq , CPIN_RSLT_PhNetSubPinReq },
{ CPIN_RSLT_PhSPPukReq     , CPIN_RSLT_PhSPPinReq     },
{ CPIN_RSLT_PhCorpPukReq   , CPIN_RSLT_PhCorpPinReq   },
{ CPIN_RSLT_PhSimFail      , CPIN_RSLT_PhSimPinReq    },
{ CPIN_RSLT_PhFSimPukReq   , CPIN_RSLT_PhFSimPinReq   },
{ CPIN_RSLT_PhBlockedNetPukReq	  , CPIN_RSLT_PhBlockedNetPinReq}
};

/* 
 * Conversion Table for setting the CME ERROR Code
 */
const UBYTE set_cme_error_code [SIMLOCK_LAST][2] = {
{ CME_ERR_NetworkPersPukReq       , CME_ERR_NetworkPersPinReq       },
{ CME_ERR_NetworkSubsetPersPukReq , CME_ERR_NetworkSubsetPersPinReq },
{ CME_ERR_ProviderPersPukReq      , CME_ERR_ProviderPersPinReq      },
{ CME_ERR_CorporatePersPukReq     , CME_ERR_CorporatePersPinReq     },
{ CME_ERR_PhoneFail               , CME_ERR_PhSimPinReq             },
{ CME_ERR_PhFSimPukReq            , CME_ERR_PhFSimPinReq            },
{ EXT_ERR_BlockedNetworkPersPukReq, EXT_ERR_BlockedNetworkPersPinReq }
};
LOCAL BOOL aci_slock_compare_codegroup_with_MEPD( UBYTE personalisation,
                                                  UINT16 *index,
                                                  UBYTE *imsi_sim,
                                                  UBYTE code_len,
                                                  UBYTE len,
                                                  UBYTE *pBody ,BOOL bnw_flag);

LOCAL UBYTE aci_slock_compare_NSCode ( UBYTE *me_nw_code,
                                       UBYTE *me_nw_ns_code_str,
                                       UBYTE *imsi_sim );
LOCAL UBYTE aci_slock_compare_MSINCode( UBYTE *me_nw_ns_msin_msin_code_str,
                                        UBYTE *imsi_sim );
LOCAL int aci_slock_extractCode( UBYTE *source_str, UBYTE len );
LOCAL UBYTE aci_slock_check_isNSCodePresent( UBYTE *me_nw_code,
                                             UBYTE *imsi_sim,
                                             UINT16 *index );
LOCAL UBYTE aci_slock_compare_gid_str( UBYTE *imsi_me,
                                       UBYTE *imsi_sim,
                                       UINT16 *index,
                                       T_SIM_SRV grp_lvl);
LOCAL UBYTE aci_slock_compare_gid( UBYTE *me_gid_str,
                                   UBYTE *sim_gidl,
                                   UBYTE *sim_gid_str,
                                   UBYTE gid_len,
                                   UBYTE *pBody,
                                   UINT16 *index );
LOCAL UBYTE aci_slock_check_isgidPresent( UBYTE *pBody,
                                          UBYTE *imsi_sim,
                                          UBYTE *imsi_me,
                                          T_SIM_SRV grp_lvl, UINT16 *index );
LOCAL void aci_slock_decode_MEPD( UBYTE *pBody,
                                  UINT16 *index,
                                  UBYTE code_len,
                                  UBYTE *imsi_ascii );
LOCAL void aci_slock_set_me_personalisation_status( UBYTE personalisation,
                                                    UBYTE status );



/*
+------------------------------------------------------------------------------
|  Function    : aci_slock_init
+------------------------------------------------------------------------------
|  Description : Initialising of this module. Has to be called first and *once only* before 
|                     calling any other method.
|
|  Parameters  : None
|
|  Return      : void
|
+------------------------------------------------------------------------------
*/

void aci_slock_init ( void )
{

  TRACE_FUNCTION("aci_slock_init()");

  /* Initialise the ACI Personalisation extension */
  aci_ext_personalisation_init();
  AciSLockShrd.blocked = FALSE;

}




/*
  Temporary Unlocks the lock of the given type after verifying the given password.
  The ACI extension for password verifying (see 2.3.9.1) will be used to
  determine if the given password is correct or not.
  On a successful unlock the actual status of the lock will be returned.
  If an error occurred SIMLOCK_FAIL will be returned.
  (Uses the ACI extension "personalisation data access".)
  */
T_SIMLOCK_STATUS aci_slock_authenticate ( T_SIMLOCK_TYPE type, char *passwd )
{
  T_SIMLOCK_STATUS result;

  TRACE_FUNCTION("aci_slock_authenticate()");

  if(!aci_slock_set_CFG())
  {
    return SIMLOCK_FAIL;
  }
  result = aci_ext_personalisation_verify_password(type, passwd);
  
  if ( (result EQ SIMLOCK_DISABLED) AND (AciSLockShrd.blocked)) /* If lock is checked as blocked at the moment uncheck it! */
  {
    AciSLockShrd.blocked = !(AciSLockShrd.current_lock EQ type);
  }

  MFREE(cfg_data);

  if(result EQ SIMLOCK_FAIL)
  {
     if(!aci_slock_set_CFG())
     {
        return SIMLOCK_FAIL;
      }
     if(cfg_data->FC_Current >= cfg_data->FC_Max)
      {
        MFREE(cfg_data);
        return SIMLOCK_BLOCKED;
      }  
     MFREE(cfg_data);
  }

  return result;
}


/*
 ACI method for retrieving the status of a single personalisation type. This method calls extension 
 methods which in turn calls Security Drv. API and retrn the status of a personalisation category.
 The personalisation status is stored in MEPD which is directly accessed by Security Drv. 
 Added on 11/03/2005
*/
T_SIMLOCK_STATUS aci_personalisation_get_status ( T_SIMLOCK_TYPE personalisation_type )
{
  TRACE_FUNCTION("aci_personalisation_get_status ()");
  return aci_ext_personalisation_get_status(personalisation_type);
}
/* #endif */


/*
+------------------------------------------------------------------------------
|  Function    : aci_slock_unlock
+------------------------------------------------------------------------------
|  Description : Unlocks the lock of the given type after verifying the given password.
|  The ACI extension for password verifying (see 2.3.9.1) will be used to
|  determine if the given password is correct or not.
|  On a successful unlock the actual status of the lock will be returned.
|  If an error occurred SIMLOCK_FAIL will be returned.
|  (Uses the ACI extension "personalisation data access".)
|
|  Parameters  : type       - Category Lock type
|         passwd   - lock Password
|
|  Return      : SIMLOCK_FAIL = -1,
|         SIMLOCK_DISABLED,      No SIM lock check has to be done  
|       SIMLOCK_PERM_DISABLED,
|       SIMLOCK_ENABLED,       A SIM lock check has to be executed
|       SIMLOCK_BLOCKED,       The SIM is blocked, i.e. because of a (or to many) wrong PIN(s) 
|        SIMLOCK_LOCKED         The ME is locked because of wrong SIM 
|
+------------------------------------------------------------------------------
*/
 
T_SIMLOCK_STATUS aci_slock_unlock ( T_SIMLOCK_TYPE type, char *passwd )
{
  T_SIMLOCK_STATUS result;

  TRACE_FUNCTION("aci_slock_unlock()");
   if(aci_slock_is_timer_support() EQ TRUE)
   {
     if(aci_slock_check_timer() EQ TIMER_RUNNING)
      {
        return SIMLOCK_BUSY; 
      }
   }
   if(!aci_slock_set_CFG())
   {
     return SIMLOCK_FAIL;  
   }
  result = aci_ext_personalisation_set_status(type, SIMLOCK_DISABLED, passwd);
  
  if ( (result EQ SIMLOCK_DISABLED) AND (AciSLockShrd.blocked)) /* If lock is checked as blocked at the moment uncheck it! */
  {
    AciSLockShrd.blocked = !(AciSLockShrd.current_lock EQ type);
  }
  MFREE(cfg_data);
  if((result EQ SIMLOCK_BLOCKED) OR (result EQ SIMLOCK_FAIL))
   {
     aci_slock_start_timer();
   }
  return result;
}


/*
+------------------------------------------------------------------------------
|  Function    : aci_slock_lock
+------------------------------------------------------------------------------
|  Description :   Locks the lock of the given type.  On a successful lock the actual
|  status of the lock will be returned. If an error occurred SIMLOCK_FAIL
|  will be returned. This method will use the ACI extension for password
|  verifying (see 2.3.9.1) to determine if the given password is correct
|  or not. (Uses the ACI extension "personalisation data access".)
|
|  Parameters  : type       - Category Lock type
|         passwd   - lock Password
|
|  Return      : SIMLOCK_FAIL = -1,
|         SIMLOCK_DISABLED,      No SIM lock check has to be done  
|       SIMLOCK_PERM_DISABLED,
|       SIMLOCK_ENABLED,       A SIM lock check has to be executed
|       SIMLOCK_BLOCKED,       The SIM is blocked, i.e. because of a (or to many) wrong PIN(s) 
|        SIMLOCK_LOCKED         The ME is locked because of wrong SIM 
|
+------------------------------------------------------------------------------
*/
T_SIMLOCK_STATUS aci_slock_lock ( T_SIMLOCK_TYPE type, char *passwd )
{

  T_SIMLOCK_STATUS result;
  T_SEC_DRV_RETURN ret;
  UBYTE imsi_sim[MAX_IMSI_LEN+1];
  int numUserCodeIdx;
  BOOL lock = TRUE;
  TRACE_FUNCTION("aci_slock_lock()");
 
  if(aci_slock_sim_config.sim_type EQ SIM_NORMAL)
  {
    if(!aci_slock_set_CFG())
   {
      return SIMLOCK_FAIL;  
    }
    aci_ext_personalisation_init();
    result = aci_ext_personalisation_get_status( type );
     if(result EQ SIMLOCK_DISABLED)
    {  
        if(cfg_data->AddNewIMSI & ( 0x0001 << type) )   /* 1-Apr-05 Bitmap check */
        {
          sim_code_present_in_me = FALSE;
          psaSIM_decodeIMSI(simShrdPrm.imsi.field, simShrdPrm.imsi.c_field, (char *)imsi_sim);
          switch(type)
          {
            case SIMLOCK_NETWORK: aci_slock_check_NWlock(imsi_sim, 1);break;
            case SIMLOCK_NETWORK_SUBSET: aci_slock_check_NSlock(imsi_sim, 1);break;
            case SIMLOCK_SIM: aci_slock_check_SMlock(imsi_sim, 1); break;
            /*Added cases for SP & CP below: 2nd April - 2005*/
            case SIMLOCK_SERVICE_PROVIDER:
                          if (psaSIM_ChkSIMSrvSup(SRV_GrpLvl1) )
                           {
                             if(aci_slock_sim_config.sim_read_gid1 EQ FALSE)
                             {
                               aci_slock_sim_read_sim(SIM_GID1, NOT_PRESENT_8BIT, MAX_GID);
                               aci_ext_personalisation_free();
                               MFREE(cfg_data);
                               return SIMLOCK_WAIT;
                             }
                          }
                         else 
                          {
                             aci_ext_personalisation_free();
                             MFREE(cfg_data);
                             return SIMLOCK_FAIL;
                          }
                         aci_slock_check_SPlock(imsi_sim,1);break;
            case SIMLOCK_CORPORATE: 
              if (psaSIM_ChkSIMSrvSup(SRV_GrpLvl1))
              {
                if(aci_slock_sim_config.sim_read_gid1 EQ FALSE)
                {
                  aci_slock_sim_read_sim(SIM_GID1, NOT_PRESENT_8BIT, MAX_GID);
                  aci_ext_personalisation_free();
                  MFREE(cfg_data);
                  return SIMLOCK_WAIT; 
                }
              }
             else 
              {
                aci_ext_personalisation_free();
                MFREE(cfg_data);
                return SIMLOCK_FAIL;
              }
              if (psaSIM_ChkSIMSrvSup(SRV_GrpLvl2))
              {
                if(aci_slock_sim_config.sim_read_gid2 EQ FALSE)
                {
                  aci_slock_sim_read_sim(SIM_GID2, NOT_PRESENT_8BIT, MAX_GID);
                  aci_ext_personalisation_free();
                  MFREE(cfg_data);
                  return SIMLOCK_WAIT;
                }
              }
             else 
              {
                aci_ext_personalisation_free();
                MFREE(cfg_data);
                return SIMLOCK_FAIL;
              }
             aci_slock_check_CPlock(imsi_sim, 1);break;
	    case SIMLOCK_BLOCKED_NETWORK:  sim_code_present_in_me = TRUE;
		      break;
           default:TRACE_ERROR ("illegal type aci_slock_lock ()");
                 return SIMLOCK_FAIL;
          }  
          if (sim_code_present_in_me EQ FALSE)         
          {  
            /* changes for ETSI behavior  date : 01-Apr-05*/
            if (cfg_data->Flags & SEC_DRV_HDR_FLAG_ETSI_Flag)
            {
              ret = SEC_DRV_RET_Ok ; 
            }
            else 
            {
              ret = sec_cmp_KEY(type,passwd,0);
            }
            if(ret EQ SEC_DRV_RET_Ok)
            {  
              aci_ext_add_code(type);
            }
            else
            {
              aci_ext_personalisation_free();
              MFREE(cfg_data);
              return SIMLOCK_FAIL;
            }
          }
         else if(sim_code_present_in_me EQ CHECK_FAIL)
          {
              aci_ext_personalisation_free();
              MFREE(cfg_data);
              return SIMLOCK_FAIL;
           }
        }   

      /*
       * If there are no operator-defined codes or user-defined codes in the MEPD at all, do not lock the category
       */
       switch(type)
       {
         case SIMLOCK_NETWORK : 
           numUserCodeIdx =  OPCODE_LEN_INDEX + ((UBYTE *)personalisation_nw->pBody)[OPCODE_LEN_INDEX] + 1;
           if( (((UBYTE *) personalisation_nw->pBody)[OPCODE_LEN_INDEX] EQ 0) AND 
           (((UBYTE *) personalisation_nw->pBody)[numUserCodeIdx] EQ 0) )
           {
             /*Do not lock the category*/
             lock = FALSE;
            }
         break;
         case SIMLOCK_NETWORK_SUBSET : 
           numUserCodeIdx =  OPCODE_LEN_INDEX + ((UBYTE *)personalisation_ns->pBody)[OPCODE_LEN_INDEX] + 1;
           if( ( ((UBYTE *)personalisation_ns->pBody)[OPCODE_LEN_INDEX] EQ 0) AND 
           ( ((UBYTE *)personalisation_ns->pBody)[numUserCodeIdx] EQ 0) )
           {
             /*Do not lock the category*/
             lock = FALSE;
           }
         break;
         case SIMLOCK_SIM : 
           numUserCodeIdx =  OPCODE_LEN_INDEX +((UBYTE *) personalisation_sim->pBody)[OPCODE_LEN_INDEX] + 1;
           if( (((UBYTE *) personalisation_sim->pBody)[OPCODE_LEN_INDEX] EQ 0) AND 
           (((UBYTE *) personalisation_sim->pBody)[numUserCodeIdx] EQ 0) )
           {
             /*Do not lock the category*/
             lock = FALSE;
            }
         break;
         case SIMLOCK_SERVICE_PROVIDER: 
           numUserCodeIdx =  OPCODE_LEN_INDEX + ((UBYTE *)personalisation_sp->pBody)[OPCODE_LEN_INDEX] + 1;
           if( ( ((UBYTE *)personalisation_sp->pBody)[OPCODE_LEN_INDEX] EQ 0) AND 
           (((UBYTE *) personalisation_sp->pBody)[numUserCodeIdx] EQ 0) )
           {
             /*Do not lock the category*/
             lock = FALSE;
           }
         break;
         case SIMLOCK_CORPORATE: 
           numUserCodeIdx =  OPCODE_LEN_INDEX + ((UBYTE *)personalisation_cp->pBody)[OPCODE_LEN_INDEX] + 1;
           if( ( ((UBYTE *)personalisation_cp->pBody)[OPCODE_LEN_INDEX] EQ 0) AND 
           ( ((UBYTE *)personalisation_cp->pBody)[numUserCodeIdx] EQ 0) )
           {
             /*Do not lock the category*/
             lock = FALSE;
           }        
         break;
        case SIMLOCK_BLOCKED_NETWORK :
           numUserCodeIdx =  OPCODE_LEN_INDEX + ((UBYTE *)personalisation_bnw->pBody)[OPCODE_LEN_INDEX] + 1;
           if( (((UBYTE *) personalisation_bnw->pBody)[OPCODE_LEN_INDEX] EQ 0) AND 
           (((UBYTE *) personalisation_bnw->pBody)[numUserCodeIdx] EQ 0) )
           {
             /*Do not lock the category*/
             lock = FALSE;
           }
         break;

       }
       if(lock EQ TRUE)
       {
         result = aci_ext_personalisation_set_status(type, SIMLOCK_ENABLED, passwd);
       }

  aci_ext_personalisation_free();
       MFREE(cfg_data);
       return result;
    }
    else 
    {
      aci_ext_personalisation_free();
      MFREE(cfg_data);
      return SIMLOCK_FAIL;
    }
  }
  else 
    return SIMLOCK_FAIL;
}


/*
+------------------------------------------------------------------------------
|  Function    : aci_slock_reset_fc
+------------------------------------------------------------------------------
|  Description : This function is used to reset the Failure Counter
|
|  Parameters  : char* fcKey - Failure Reset Key
|
|  Return      :   OPER_FAIL 
|           OPER_SUCCESS  
|
+------------------------------------------------------------------------------
*/

T_OPER_RET_STATUS aci_slock_reset_fc ( char *fcKey )
{  
  T_OPER_RET_STATUS result; 
  TRACE_FUNCTION("aci_slock_reset_fc ()");
  if(!aci_slock_set_CFG())
    {
      return OPER_FAIL;  
    }
  if(cfg_data->Flags & SEC_DRV_HDR_FLAG_Unlock_Timer)
  {
    if(aci_slock_check_timer() EQ TIMER_RUNNING)
    {
      return OPER_BUSY; 
    }
  }
  result = aci_ext_slock_reset_fc(fcKey); 
  if(result EQ OPER_WRONG_PASSWORD)
  {
    aci_slock_start_timer();
  }
  return result ;
}

/*
+------------------------------------------------------------------------------
|  Function    : aci_slock_sup_info
+------------------------------------------------------------------------------
|  Description : This function is used to get the supplementary information like FCMAX,   
|      FCATTEMPTSLEFT,  FCRESETFAILMAX, FCRESETFAILATTEMPTSLEFT,
|        FCRESETSUCCESSMAX, FCRESETSUCCESSATTEMPTSLEFT, TIMERFLAG,
|      ETSIFLAG,  AIRTELINDFLAG
|
|  Parameters  : sup_info - Pointer to T_SUP_INFO
|
|  Return      :   OPER_FAIL 
|           OPER_SUCCESS  
|
+------------------------------------------------------------------------------
*/
T_OPER_RET_STATUS aci_slock_sup_info(T_SUP_INFO *sup_info)
{  
  TRACE_FUNCTION("aci_slock_sup_info()");
  return aci_ext_slock_sup_info (sup_info);
}


/*
+------------------------------------------------------------------------------
|  Function    : aci_slock_change_password
+------------------------------------------------------------------------------
|  Description : This function is used to change the password of a category
|
|  Parameters  :     type - Category lock type
|             passwd - Old  Control key
|        new_passwd - New  Control key
|
|  Return      :  SIMLOCK_FAIL = -1,
|         SIMLOCK_DISABLED,      No SIM lock check has to be done  
|       SIMLOCK_PERM_DISABLED,
|       SIMLOCK_ENABLED,       A SIM lock check has to be executed
|       SIMLOCK_BLOCKED,       The SIM is blocked, i.e. because of a (or to many) wrong PIN(s) 
|        SIMLOCK_LOCKED         The ME is locked because of wrong SIM 
|
+------------------------------------------------------------------------------
*/

T_OPER_RET_STATUS aci_slock_change_password ( T_SIMLOCK_TYPE type, char *passwd, char *new_passwd )
{
  TRACE_FUNCTION("aci_slock_change_password()");
  return aci_ext_personalisation_change_password(type, passwd, new_passwd);
}


/*
+------------------------------------------------------------------------------
|  Function    : aci_slock_psaSIM_decodeIMSI
+------------------------------------------------------------------------------
|  Description : convert imsi (packed bcd to ASCIIZ; ->11.11)
|
|  Parameters  :   imsi_field            - IMSI in BCD
|                    imsi_c_field         - 
|                    imsi_asciiz           - ASCII IMSI
|
|  Return      : void
|
+------------------------------------------------------------------------------
*/
GLOBAL void aci_slock_psaSIM_decodeIMSI (UBYTE* imsi_field,    
                    UBYTE  imsi_c_field,  CHAR* imsi_asciiz)
{

  UBYTE imsi_len;
  UBYTE i;
  UBYTE digit;


 /*
      
        | 8 | 7 | 6 | 5 | 4 | 3 | 2 | 1 |
      
    +---+---+---+---+---+---+---+---+
     imsi_c_field  = |         Length indicator      |     
     imsi_field[0] = | IMSI digit 1  | p | 0 | 0 | 1 |
     imsi_field[1] = | IMSI digit 3  | IMSI digit 2  |

     imsi_c_field = length indicator:
          
     The length indicator refers to the number of significant bytes, not including this length byte, 
     required for the IMSI.
     p = parity
         0: Even number of IMSI digits
         1: Odd number of IMSI digits

     If the number of IMSI digits is even then bits 5 to 8 of the last octet shall be filled with an end mark coded as 1111b
   */

  /*
   *
 Check length
   */
  if ((imsi_c_field EQ 0) OR (imsi_c_field > (MAX_IMSI-1)))  /* maybe 0xFF on some testcards */
  {
    TRACE_EVENT_P1("[WRN] imsi_c_field = %d is not valid", imsi_c_field);

    imsi_asciiz[0] = '\0';    /* return empty string in case of error */

    return;
  }

  /*
   * calculate number of digits
   */
  imsi_len = (imsi_c_field)*2-1;   /* -1 goes off for parity nibble */


  /*
   * if even number of digits then last upper nibble is an end mark '1111'
   */
  if ((imsi_field[0] & 0x08) EQ 0)
  {
    imsi_len--;
  }

 

  if ((imsi_field[0] & 0x08) EQ 0)
  {
    imsi_len--;
  }

  /*
   * extract all digits
   */
  for (i=0; i<imsi_len; i++)
  {
    if ((i & 1) EQ 0)
    {
      /* process IMSI digit 1,3,5,... at i=0,2,4,...*/
      digit = (imsi_field[(i+1)/2] & 0xf0) >> 4;   /* +1 is to skip parity nibble */
    }
    else
    {
      /* process IMSI digit 2,4,6,... at i=1,3,5,...*/
      digit = (imsi_field[(i+1)/2] & 0x0f); 
    }

    if (digit > 9)  
    {
      imsi_asciiz[i] = 'F';    
      /*  return;  */
    }
    else
    {
      imsi_asciiz[i] = '0' + digit;
    }
   
  }
  imsi_asciiz[i] = '\0';
  return;
}


/*
+------------------------------------------------------------------------------
|  Function    : aci_slock_psaSIM_decodeIMSI_without_parity
+------------------------------------------------------------------------------
|  Description : convert imsi (packed bcd to ASCIIZ; ->11.11)
|
|  Parameters  :   imsi_field            - IMSI in BCD
|                    imsi_c_field         - 
|                    imsi_asciiz           - ASCII IMSI
|
|  Return      : void
|
+------------------------------------------------------------------------------
*/

GLOBAL void aci_slock_psaSIM_decodeIMSI_without_parity(UBYTE* imsi_field, 
                               UBYTE  imsi_c_field, 
                               CHAR* imsi_asciiz)
{

  UBYTE imsi_len;
  UBYTE i;
  UBYTE digit;
 
  /* TRACE_FUNCTION ("aci_slock_sim_decodeIMSI()"); */

  /*
                     | 8 | 7 | 6 | 5 | 4 | 3 | 2 | 1 |
                     +---+---+---+---+---+---+---+---+
     imsi_c_field  = |         Length indicator      |
     imsi_field[0] = | IMSI digit 1  | p | 0 | 0 | 1 |
     imsi_field[1] = | IMSI digit 3  | IMSI digit 2  |
                    

     imsi_c_field = length indicator:
                    The length indicator refers to the number of significant bytes,
                    not including this length byte, required for the IMSI.
     p = parity
         0: Even number of IMSI digits
         1: Odd number of IMSI digits

     If the number of IMSI digits is even then bits 5 to 8 of the last octet
     shall be filled with an end mark coded as 1111b
   */

  /*
   * Check length
   */
  if ((imsi_c_field EQ 0) OR (imsi_c_field > (MAX_IMSI-1)))  /* maybe 0xFF on some testcards */
  {
    TRACE_EVENT_P1("[WRN] imsi_c_field = %d is not valid", imsi_c_field);
    imsi_asciiz[0] = '\0';    /* return empty string in case of error */
    return;
  }

  /*
   * calculate number of digits
   */
  imsi_len = (imsi_c_field)*2;   

  

  /*
   * extract all digits
   */
  for (i=0; i<imsi_len; i++)
  {
    if ((i & 1) EQ 0)
    {
      /* process IMSI digit 1,3,5,... at i=0,2,4,...*/
       digit = (imsi_field[i/2] & 0x0f); 
    }
    else
    {
      /* process IMSI digit 2,4,6,... at i=1,3,5,...*/
     digit = (imsi_field[i/2] & 0xf0) >> 4;   /* +1 is to skip parity nibble */
    }

    if (digit > 9)  
    {
     
      imsi_asciiz[i] = 'F';    
    /*  return;  */
    }
    else
    {
      imsi_asciiz[i] = '0' + digit;
    }

   
  }
  imsi_asciiz[i] = '\0';
  return;
}


/*
+------------------------------------------------------------------------------
|  Function    : aci_slock_check_NWlock
+------------------------------------------------------------------------------
|  Description : Check IMSI against network personalisation 
|
|  Parameters  :   imsi_sim            - IMSI
|                    personalisation   - 0-add the lock data
|                    1-verify the existing lock data  
|
|  Return      : void
|
+------------------------------------------------------------------------------
*/

void aci_slock_check_NWlock( UBYTE* imsi_sim, UBYTE  personalisation  )
{

  UBYTE num_op_codes;
  UBYTE num_user_codes;
  UBYTE code_type;

  UBYTE i;
  UINT16 index =0;
  UBYTE imsi_me[MAX_IMSI_LEN+1];
  UBYTE isCodeFound;

  int   me_nw_code_first, me_nw_code_last, sim_nw_code;

  TRACE_FUNCTION("aci_slock_check_NWlock()");

  /*
  personalisation_nw->pBody[0] -----maximum number of user codes 
  personalisation_nw->pBody[1] -----num of operator codes 
  personalisation_nw->pBody[2] -----Operator Code-group length (To be set during configuration)
where length =  total number of bytes occupied by operator codes


Code type = 0x0a ( Type identifier for normal code)  
Normal code (Including parity nibble)  -- 4 bytes

Code type = 0x0b (Type identifier for range)
Start value (Including parity nibble) ---- 4 byte
End value (Including parity nibble) -----4 byte 

Code type = 0x0c (Type identifier for regular expression)   
Regular Expression  -------------6 byte


Number of User Codes  --- 1 byte
Current user code index  --- 1 byte
User code1  ------------4 byte

  */
/* TRACE_FUNCTION_P1("personalisation = %d",personalisation);  */
  if((personalisation_nw NEQ NULL) AND ((UBYTE *)personalisation_nw->pBody NEQ NULL))
  {
    index += MAX_NUM_USERCODE_SIZE;
    num_op_codes = ((UBYTE *)personalisation_nw->pBody)[index];
    index += NUM_OPCODE_SIZE;

    index +=OPCODE_LEN_SIZE;
	
    /* Implements NEW Mesaure */
    aci_slock_set_me_personalisation_status( personalisation, FALSE );

  /*
   * check operator defined code-groups
   */
    for(i=0; i< num_op_codes; i++)
    {
      code_type =((UBYTE *) personalisation_nw->pBody)[index];
      index +=CODE_TYPE_SIZE;

      switch(code_type)
      {
        case NORMAL_CODE :
          /* Checks whether the NW Code is Matching with the 
            * Network Personalisation DataBase 
            */
           if(aci_slock_compare_codegroup_with_MEPD( personalisation,
                                                                                    &index,imsi_sim,
                                                                                    NW_CODE_LEN,
                                                                                    simShrdPrm.mnc_len+3,
                                                                                    (UBYTE*)personalisation_nw->pBody,FALSE))
           {
		return;
           }
           break;		   
    
         case INTERVAL_TYPE1 :
	    /* Implements New Measure */
           aci_slock_decode_MEPD( (UBYTE *)personalisation_nw->pBody, &index,
                                                  NW_CODE_LEN, imsi_me );
            /* to remove trailing F (if any) from imsi_me string */
            imsi_me[simShrdPrm.mnc_len+3] = '\0';
            me_nw_code_first = atoi((const char *)imsi_me);
		   
            aci_slock_decode_MEPD( (UBYTE *)personalisation_nw->pBody, &index,
                                                    NW_CODE_LEN, imsi_me );
		   
             imsi_me[simShrdPrm.mnc_len+3] = '\0';
             me_nw_code_last = atoi((const char *)imsi_me);
		   
             sim_nw_code = aci_slock_extractCode( imsi_sim, simShrdPrm.mnc_len+3 );
		   
             isCodeFound = (sim_nw_code >= me_nw_code_first ) AND 
                                   (sim_nw_code <= me_nw_code_last);
		   
             aci_slock_set_me_personalisation_status ( personalisation,
																isCodeFound );
		   
             if( isCodeFound )
              {
                 return;
              }
         break;
          
         case REGULAR_EXP :
         break;
    
         default :
         break;
      }
    }

    num_user_codes = ((UBYTE *)personalisation_nw->pBody)[index];
    index +=NUM_USER_CODE_SIZE;
    index +=CURR_USER_CODE_INDEX_SIZE;

  /*
   * check user defined code-groups
   */
    for(i=0; i< num_user_codes; i++)
    {

      	/* 
         * Checks whether the NW Code is Matching with the 
         * Network Personalisation DataBase 
         */
        if(aci_slock_compare_codegroup_with_MEPD( personalisation,
                                                                                    &index,imsi_sim,
                                                                                    NW_CODE_LEN,
                                                                                    simShrdPrm.mnc_len+3,
                                                                                    (UBYTE*)personalisation_nw->pBody,FALSE))
           {
		return;
           }
    }
  }
}


/*
+------------------------------------------------------------------------------
|  Function    : aci_slock_check_NSlock
+------------------------------------------------------------------------------
|  Description : Check IMSI against network subset personalisation 
|
|  Parameters  :   imsi_sim            - IMSI
|                    personalisation   - 0-add the lock data
|                    1-verify the existing lock data  
|
|  Return      : void
|
+------------------------------------------------------------------------------
*/
void aci_slock_check_NSlock( UBYTE* imsi_sim, UBYTE  personalisation  )
{
  UBYTE num_op_codes;
  UBYTE num_user_codes;
  UBYTE code_type;  
  UBYTE cmp_result;
  UINT16 index =0;
  UBYTE me_nw_code[MAX_IMSI_LEN+1];
  UBYTE me_nw_ns_ns_code_str[6+2+2+1];
    
  int i ; 
      
  TRACE_FUNCTION("aci_slock_check_NSlock()");

/*
  personalisation_ns->pBody[0] -----maximum number of user codes 
  personalisation_ns->pBody[1] -----num of operator codes 
  personalisation_ns->pBody[2] -----Operator Code-group length (To be set during configuration)
where length =  total number of bytes occupied by operator codes


Code type = 0x0a ( Type identifier for normal code)  
Normal code (Including parity nibble)  -- 5 bytes  (Nw code + NS code)

Code type = 0x0b (Type identifier for range)
Network code --------- 4 byte 
Start value of NS code ---- 1 byte
End value of NS code -----1 byte 

Code type = 0x0c (Type identifier for regular expression)   
Regular expression including Network + NS Code -------------8 byte

Code type = 0x0d
(8thdigit normal code/range)
Network code (Including parity nibble) ------ 4 byte
No. of  8th digit values for which normal codes and/or intervals are to be stored   ---- 1 byte
8th digit value  -- 1 byte
No. of normal codes to be associated with the digit (n1)  ---- 1 byte
No. of intervals to be associated with the digit (n2)  ----- 1 byte
Normal code 1   --- 1 byte 
            |
            |
            |
Normal code n1 ---- 1 byte 

Start value of interval 1  ---- 1 byte 
End value of interval 1   ---- 1 byte 
            |
            |
            |
 Start value of interval n2  ------ 1 byte 
  End value of interval n2  ------- 1 byte 

  Number of User Codes   ------- 1 byt e
  Current user code index 
(for FIFO based addition/deletion of user codes)
(should be set to 0xff if no user codes are present)  ---- 1 byte 

User code1             ---------- 5 byte 
User code 2      ---------- 5 byte 
         | 
         |
 User code n (n = Max number of user codes for NS)  -- 5 byte


  */

  /* Implements NEW Mesaure */
  aci_slock_set_me_personalisation_status( personalisation, FALSE );
  if((personalisation_ns NEQ NULL) AND ((UBYTE *)personalisation_ns->pBody NEQ NULL))
  {
    index += MAX_NUM_USERCODE_SIZE;
    num_op_codes =((UBYTE *) personalisation_ns->pBody)[index];
    index += NUM_OPCODE_SIZE;
    
    index +=OPCODE_LEN_SIZE;

    /*
     * check operator defined code-groups
     */
    for(i=0; i< num_op_codes; i++)
    {
      code_type = ((UBYTE *)personalisation_ns->pBody)[index];
      index +=CODE_TYPE_SIZE;
      switch(code_type)
      {
        case NORMAL_CODE :
	 /* 
          * Checks whether the NS Code is Matching with the 
          * Network Subset Personalisation DataBase 
          */
         if (aci_slock_compare_codegroup_with_MEPD( personalisation,
                                                                                      &index, imsi_sim,
                                                                                      NW_NS_CODE_LEN,
                                                                                      simShrdPrm.mnc_len+3+2,
                                                                                      (UBYTE*)personalisation_ns->pBody,FALSE))
             {
                return;
             }
	
        break;
      
        case INTERVAL_TYPE1 :

          aci_slock_decode_MEPD( (UBYTE *)personalisation_ns->pBody, &index,
                                                    NW_NS_NS_CODE_LEN, me_nw_ns_ns_code_str );
          memcpy(me_nw_code,me_nw_ns_ns_code_str,simShrdPrm.mnc_len+3);
          /* Compare the NS Code */
          cmp_result = aci_slock_compare_NSCode( me_nw_code,
                                                                              me_nw_ns_ns_code_str,
                                                                              imsi_sim );
		  
		  
           aci_slock_set_me_personalisation_status( personalisation,
                                                                                  cmp_result );
		  
           if ( cmp_result )
           {
              /* 
		  * The Network Code and the Network subset code matches,
		  * hence return 
		  */
                 return;
            }
        break;
      
        case INTERVAL_TYPE2 :
          TRACE_FUNCTION("INTERVAL_TYPE2");
          aci_slock_decode_MEPD( (UBYTE *)personalisation_ns->pBody, &index,
                                                   NW_CODE_LEN, me_nw_code );

          index += 1; 

	 /* Implements Measure 212 */
         cmp_result = aci_slock_check_isNSCodePresent( me_nw_code, imsi_sim, &index );
	 
         aci_slock_set_me_personalisation_status( personalisation, cmp_result );
	 
         if ( cmp_result )
         {
           return;
         }
        break;
     
        case REGULAR_EXP :
        break;

   default :
        break;
      }
    }

    num_user_codes = ((UBYTE *)personalisation_ns->pBody)[index];
    index +=NUM_USER_CODE_SIZE;
     
    index +=CURR_USER_CODE_INDEX_SIZE;
     /*
     * check user defined code-groups
     */
    for(i=0; i< num_user_codes; i++)
    {

        /* Checks whether the NS Code is Matching with the 
         * Network Subset Personalisation DataBase 
         */
        if(aci_slock_compare_codegroup_with_MEPD( personalisation,
                                                                                    &index, imsi_sim,
                                                                                    NW_NS_CODE_LEN,
                                                                                    simShrdPrm.mnc_len+3+2,
                                                                                    (UBYTE*)personalisation_ns->pBody,FALSE))
          {
            return;
          }
    }
  }
}


/*
+------------------------------------------------------------------------------
|  Function    : aci_slock_check_SPlock
+------------------------------------------------------------------------------
|  Description : Check IMSI against service provider personalisation
|
|  Parameters  :   imsi_sim            - IMSI
|                    personalisation   - 0-add the lock data
|                    1-verify the existing lock data  
|
|  Return      : void
|
+------------------------------------------------------------------------------
*/
void aci_slock_check_SPlock( UBYTE* imsi_sim, UBYTE  personalisation )
{
  UBYTE num_op_codes;
  UBYTE num_user_codes;
  UBYTE code_type;
  UBYTE cmp_result;
  UINT16 index =0;
  UBYTE imsi_me[MAX_IMSI_LEN+1];
  int  i;

  TRACE_FUNCTION("aci_slock_check_SPlock()");

#if 0 // TISH patch for OMAPS00120012
   if((aci_slock_sim_config.sim_gidl1[0] EQ NOT_PRESENT_8BIT) OR (aci_slock_sim_config.gid1_len NEQ cfg_data->GID1_Len))
#else
   if(aci_slock_sim_config.sim_gidl1[0] EQ NOT_PRESENT_8BIT) 
#endif
     {
       
        TRACE_FUNCTION_P1("aci_slock_sim_config.sim_gidl1[0] %d", aci_slock_sim_config.sim_gidl1[0]);
       if(!personalisation)
        {
          AciSLockShrd.blocked = TRUE;
        }
       else 
       {
          sim_code_present_in_me = CHECK_FAIL; 
       }
        return; 
   }
  else
   {

  
  /*
  personalisation_sp->pBody[0] -----maximum number of user codes 
  personalisation_sp->pBody[1] -----num of operator codes 
  personalisation_sp->pBody[2] -----Operator Code-group length (To be set during configuration)
where length =  total number of bytes occupied by operator codes


Code type = 0x0a ( Type identifier for normal code)  
Normal code (Including parity nibble)  -- 8 bytes  (Network code + GID1 value)

Code type = 0x0b (Type identifier for range)
Network code --------- 4 byte 
Start GID1 ---- -------4 byte
End GID1 -------------4 byte 

Code type = 0x0c (Type identifier for regular expression)   
Regular Expression (including NW code and GID1 value)  --- 10 byte

  Number of User Codes   ------- 1 byt e
  Current user code index 
(for FIFO based addition/deletion of user codes)
(should be set to 0xff if no user codes are present)  ---- 1 byte 

User code1             ---------- 8 byte 
User code 2      ---------- 8 byte 
         | 
         |
 User code n (n = Max number of user codes for NS)  -- 8 byte


  */
  aci_slock_set_me_personalisation_status( personalisation, FALSE );

  if((personalisation_sp NEQ NULL) AND ((UBYTE *)personalisation_sp->pBody NEQ NULL))
  {
    index += MAX_NUM_USERCODE_SIZE;
    num_op_codes = ((UBYTE *)personalisation_sp->pBody)[index];
    index += NUM_OPCODE_SIZE;
     
    index +=OPCODE_LEN_SIZE;
     
    /*
    * check operator defined code-groups
    */
    for(i=0; i< num_op_codes; i++)
    {
      code_type = ((UBYTE *)personalisation_sp->pBody)[index];
      index +=CODE_TYPE_SIZE;
      if( ((code_type)&0xf0)>>4)
       {
          imsi_me[0] = 0; 
      	}
      else
      	{
      	   aci_slock_decode_MEPD( (UBYTE *)personalisation_sp->pBody, &index,
                         	                            NW_CODE_LEN, imsi_me );
      	}
      switch(code_type)
      {
        case NORMAL_CODE :

          cmp_result = aci_slock_check_isgidPresent( (UBYTE *)personalisation_sp->pBody,
                                                                                 imsi_sim, imsi_me,
                                                                                 SRV_GrpLvl1, &index );
         aci_slock_set_me_personalisation_status( personalisation,
                                                                                cmp_result );
		 
         if (cmp_result)
         {
            return;
         }

        break;
                  
        case INTERVAL_TYPE1 :
         cmp_result = aci_slock_compare_gid_str ( imsi_me, imsi_sim, &index,SRV_GrpLvl1);
         aci_slock_set_me_personalisation_status( personalisation, cmp_result );
         if (cmp_result)
         {
            return;
          }
        break;
                  
        case REGULAR_EXP :
        break;
        default :
        break;
      }
    }

    num_user_codes = ((UBYTE *)personalisation_sp->pBody)[index];
    index +=NUM_USER_CODE_SIZE;
    index +=CURR_USER_CODE_INDEX_SIZE;
    /*
    * check user defined code-groups
    */
    for(i=0; i< num_user_codes; i++)
    {
 
       aci_slock_decode_MEPD( (UBYTE *)personalisation_sp->pBody, &index,
                                                   NW_CODE_LEN, imsi_me );
       cmp_result = aci_slock_check_isgidPresent( (UBYTE *)personalisation_sp->pBody,
                                                                              imsi_sim, imsi_me, SRV_GrpLvl1,&index );
      aci_slock_set_me_personalisation_status( personalisation,
                                                                             cmp_result );
      if (cmp_result)
      {
         return;
      }
    }
  }
}
}


/*
+------------------------------------------------------------------------------
|  Function    : aci_slock_check_CPlock
+------------------------------------------------------------------------------
|  Description : Check IMSI against service corporate personalisation
|
|  Parameters  :   imsi_sim            - IMSI
|                    personalisation   - 0-add the lock data
|                    1-verify the existing lock data  
|
|  Return      : void
|
+------------------------------------------------------------------------------
*/
void aci_slock_check_CPlock( UBYTE* imsi_sim, UBYTE  personalisation)
{
  UBYTE num_op_codes;
  UBYTE num_user_codes;
  
  UBYTE code_type;
  UBYTE cmp_result;
 
  UINT16 index =0;
  UBYTE imsi_me[MAX_IMSI_LEN+1];

  int i;

  TRACE_FUNCTION("aci_slock_check_CPlock()");


  
#if 0  // TISH patch for OMAPS00120012 
  if((aci_slock_sim_config.sim_gidl1[0] EQ NOT_PRESENT_8BIT) OR (aci_slock_sim_config.sim_gidl2[0] EQ NOT_PRESENT_8BIT)
       OR (aci_slock_sim_config.gid1_len NEQ cfg_data->GID1_Len) OR (aci_slock_sim_config.gid2_len NEQ cfg_data->GID2_Len))
#else
  if((aci_slock_sim_config.sim_gidl1[0] EQ NOT_PRESENT_8BIT) OR (aci_slock_sim_config.sim_gidl2[0] EQ NOT_PRESENT_8BIT))
#endif
   {
     if(!personalisation)
      {
        AciSLockShrd.blocked = TRUE;
      }
     else 
     {
       sim_code_present_in_me = CHECK_FAIL; 
     }
     return; 
   }
  else
   {


    /*
  personalisation_cp->pBody[0] -----maximum number of user codes 
  personalisation_cp->pBody[1] -----num of operator codes 
  personalisation_cp->pBody[2] -----Operator Code-group length (To be set during configuration)
where length =  total number of bytes occupied by operator codes


Code type = 0x0a ( Type identifier for normal code)  
Normal code (Including parity nibble)  -- 12 bytes  (Network code + GID1 value + GID2 value)

Code type = 0x0b (Type identifier for range)
Network code --------- 4 byte 
GID1 value------------4 byte
Start GID1 ---- -------4 byte
End GID1 -------------4 byte 

Code type = 0x0c (Type identifier for regular expression)   
Regular Expression (including NW code, GID1 value and GID2 value)  --- 14 byte

  Number of User Codes   ------- 1 byt e
  Current user code index 
(for FIFO based addition/deletion of user codes)
(should be set to 0xff if no user codes are present)  ---- 1 byte 

User code1             ---------- 12 byte 
User code 2      ---------- 12 byte 
         | 
         |
 User code n (n = Max number of user codes for NS)  -- 12 byte


  */

    /* Implements NEW Mesaure */
  aci_slock_set_me_personalisation_status( personalisation, FALSE );

  if((personalisation_cp NEQ NULL) AND ((UBYTE *)personalisation_cp->pBody NEQ NULL))
  {
    index += MAX_NUM_USERCODE_SIZE;
    num_op_codes = ((UBYTE *)personalisation_cp->pBody)[index];
    index += NUM_OPCODE_SIZE;

    index +=OPCODE_LEN_SIZE;
    /*
    * check operator defined code-groups
    */
    for(i=0; i< num_op_codes; i++)
    {
      code_type = ((UBYTE *)personalisation_cp->pBody)[index];
      index +=CODE_TYPE_SIZE;
      if( ((code_type)&0xf0)>>4)
       {
          imsi_me[0] = 0; 
      	}
      else
      	{
      	   aci_slock_decode_MEPD( (UBYTE *)personalisation_sp->pBody, &index,
                         	                            NW_CODE_LEN, imsi_me );
      	}
      switch(code_type)
      { 
        case NORMAL_CODE :
         cmp_result = aci_slock_check_isgidPresent( (UBYTE *)personalisation_cp->pBody,
                                                                                imsi_sim, imsi_me,
                                                                                SRV_GrpLvl2, &index );
		
        aci_slock_set_me_personalisation_status( personalisation,
                                                                              cmp_result );
		
         if (cmp_result)
         {
           return;
         }

        break;
    
        case INTERVAL_TYPE1 :
          cmp_result = aci_slock_compare_gid_str ( imsi_me, imsi_sim, &index,SRV_GrpLvl2);
		 
          aci_slock_set_me_personalisation_status( personalisation, cmp_result );
		 
          if (cmp_result)
           {
              return;
            }
        break;

   case REGULAR_EXP :
        break;

   default :
        break;
      }
    }

    num_user_codes = ((UBYTE *)personalisation_cp->pBody)[index];
    index +=NUM_USER_CODE_SIZE;
    index +=CURR_USER_CODE_INDEX_SIZE;
    /*
     * check user defined code-groups
     */
    for(i=0; i< num_user_codes; i++)
    {
     
       aci_slock_decode_MEPD( (UBYTE *)personalisation_cp->pBody, &index,
                                                  NW_CODE_LEN, imsi_me );
    
       cmp_result = aci_slock_check_isgidPresent( (UBYTE *)personalisation_cp->pBody,
                                                                               imsi_sim, imsi_me,
                                                                               SRV_GrpLvl2,&index );
	  
        aci_slock_set_me_personalisation_status( personalisation,cmp_result );
	  
        if (cmp_result)
        {
          return;
        }
    }
  }
 }
 
}


/*
+------------------------------------------------------------------------------
|  Function    : aci_slock_check_SMlock
+------------------------------------------------------------------------------
|  Description :   Check IMSI against SIM personalisation
|
|  Parameters  :   imsi_sim            - IMSI
|                    personalisation   - 0-add the lock data
|                    1-verify the existing lock data  
|
|  Return      : void
|
+------------------------------------------------------------------------------
*/
 void aci_slock_check_SMlock( UBYTE* imsi_sim, UBYTE  personalisation)
{
  UBYTE num_op_codes;
  UBYTE num_user_codes;
  UBYTE cmp_result;
  UBYTE code_type;
  
  UBYTE i;
  UINT16 index =0;
  
  UBYTE me_nw_ns_msin_msin_code_str[6+2+8+8+1];
  
  
  TRACE_FUNCTION("aci_slock_check_SMlock()");

/*
  personalisation_sim->pBody[0] -----maximum number of user codes 
  personalisation_sim->pBody[1] -----num of operator codes 
  personalisation_sim->pBody[2] -----Operator Code-group length (To be set during configuration)
where length =  total number of bytes occupied by operator codes


Code type = 0x0a ( Type identifier for normal code)  
Normal code (Including parity nibble)  -- 8 bytes  (Network code + NS code + MSIN)

Code type = 0x0b (Type identifier for range)
Network code + NS code  --------- 5 byte 
Start MSIN ---- -------4 byte
End MSIN -------------4 byte 

Code type = 0x0c (Type identifier for regular expression)   
Regular Expression (including NW code NS code and MSIN)  --- 15 byte

  Number of User Codes   ------- 1 byt e
  Current user code index 
(for FIFO based addition/deletion of user codes)
(should be set to 0xff if no user codes are present)  ---- 1 byte 

User code1             ---------- 8 byte 
User code 2      ---------- 8 byte 
         | 
         |
 User code n (n = Max number of user codes for NS)  -- 8 byte


  */

  aci_slock_set_me_personalisation_status( personalisation, FALSE );

  if((personalisation_sim NEQ NULL) AND ((UBYTE *)personalisation_sim->pBody NEQ NULL))
  {
    index += MAX_NUM_USERCODE_SIZE;
    num_op_codes =((UBYTE *) personalisation_sim->pBody)[index];
    index += NUM_OPCODE_SIZE;
    
    index +=OPCODE_LEN_SIZE;
  
    /*
    * check operator defined code-groups
    */
    for(i=0; i< num_op_codes; i++)
    {
      code_type = ((UBYTE *)personalisation_sim->pBody)[index];
      index +=CODE_TYPE_SIZE;
      
      switch(code_type)
      {
        case NORMAL_CODE :
          if(aci_slock_compare_codegroup_with_MEPD( personalisation,
                                                                                       &index, imsi_sim,
                                                                                       NW_NS_MSIN_CODE_LEN,
                                                                                       MAX_IMSI_LEN,
                                                                                       (UBYTE*)personalisation_sim->pBody,FALSE))
             {
                return;
             }
	
        break;
                
        case INTERVAL_TYPE1 :
         aci_slock_decode_MEPD( (UBYTE *)personalisation_sim->pBody, &index,
                                                    NW_NS_MSIN_MSIN_CODE_LEN,
                                                    me_nw_ns_msin_msin_code_str );

         cmp_result = aci_slock_compare_MSINCode( me_nw_ns_msin_msin_code_str,
                                                                                 imsi_sim );
		   
          aci_slock_set_me_personalisation_status( personalisation,cmp_result );
		  
          if ( cmp_result EQ TRUE )
           {
                /* 
                  * The Network subset code and MSIN code matches,
                  * hence return 
                  */
                 return;
            }
        break;    
                
        case REGULAR_EXP :
        break;
        default :
        break;
      }
    }

    num_user_codes = ((UBYTE *)personalisation_sim->pBody)[index];
    index +=NUM_USER_CODE_SIZE;

    index +=CURR_USER_CODE_INDEX_SIZE;
    /*
     * check user defined code-groups
     */
    for(i=0; i< num_user_codes; i++)
    {
    
      /* 
       * Checks whether the MSIN Code is Matching with the 
       * SIM Personalisation DataBase 
       */
      if(aci_slock_compare_codegroup_with_MEPD( personalisation,
                                                &index, imsi_sim,
                                                NW_NS_MSIN_CODE_LEN,
                                                MAX_IMSI_LEN,
                                                (UBYTE*)personalisation_sim->pBody,FALSE))
      {
        return;
      }
    }
  }
}

/*
+------------------------------------------------------------------------------
|  Function    : aci_slock_check_BNWlock
+------------------------------------------------------------------------------
|  Description : Check IMSI against blocked network personalisation 
|
|  Parameters  :   imsi_sim            - IMSI
|                    personalisation   - 0-add the lock data
|                    1-verify the existing lock data  
|
|  Return      : void
|
+------------------------------------------------------------------------------
*/

void aci_slock_check_BNWlock( UBYTE* imsi_sim, UBYTE  personalisation  )
{

  UBYTE num_op_codes;
  UBYTE num_user_codes;
  UBYTE code_type;

  UBYTE i;
  UINT16 index =0;
  UBYTE imsi_me[MAX_IMSI_LEN+1];
  UBYTE isCodeFound;

  int   me_bnw_code_first, me_bnw_code_last, sim_bnw_code;
  
  TRACE_FUNCTION("aci_slock_check_NWlock()");

  /*
  personalisation_bnw->pBody[0] -----maximum number of user codes 
  personalisation_bnw->pBody[1] -----num of operator codes 
  personalisation_bnw->pBody[2] -----Operator Code-group length (To be set during configuration)
where length =  total number of bytes occupied by operator codes


Code type = 0x0a ( Type identifier for normal code)  
Normal code (Including parity nibble)  -- 4 bytes

Code type = 0x0b (Type identifier for range)
Start value (Including parity nibble) ---- 4 byte
End value (Including parity nibble) -----4 byte 

Code type = 0x0c (Type identifier for regular expression)   
Regular Expression  -------------6 byte


Number of User Codes  --- 1 byte
Current user code index  --- 1 byte
User code1  ------------4 byte

  */
/* TRACE_FUNCTION_P1("personalisation = %d",personalisation);  */
  if((personalisation_bnw NEQ NULL) AND ((UBYTE *)personalisation_bnw->pBody NEQ NULL))
  {
    index += MAX_NUM_USERCODE_SIZE;
    num_op_codes = ((UBYTE *)personalisation_bnw->pBody)[index];
    index += NUM_OPCODE_SIZE;

    index +=OPCODE_LEN_SIZE;
	
    /* Implements NEW Mesaure */
    aci_slock_set_me_personalisation_status( personalisation, TRUE );

  /*
   * check operator defined code-groups
   */
    for(i=0; i< num_op_codes; i++)
    {
      code_type =((UBYTE *) personalisation_bnw->pBody)[index];
      index +=CODE_TYPE_SIZE;

      switch(code_type)
      {
        case NORMAL_CODE :
          /* Checks whether the NW Code is Matching with the 
            * Network Personalisation DataBase 
            */
           if (aci_slock_compare_codegroup_with_MEPD( personalisation,
                                                                                        &index, imsi_sim,
                                                                                        NW_CODE_LEN,
                                                                                        simShrdPrm.mnc_len+3,
                                                                                        (UBYTE*)personalisation_bnw->pBody,TRUE))
             {
		   return;
              }
           break;		   
    
         case INTERVAL_TYPE1 :
	    /* Implements New Measure */
           aci_slock_decode_MEPD( (UBYTE *)personalisation_bnw->pBody, &index,
                                                  NW_CODE_LEN, imsi_me );
            /* to remove trailing F (if any) from imsi_me string */
            imsi_me[simShrdPrm.mnc_len+3] = '\0';
            me_bnw_code_first = atoi((const char *)imsi_me);
		   
            aci_slock_decode_MEPD( (UBYTE *)personalisation_bnw->pBody, &index,
                                                    NW_CODE_LEN, imsi_me );
		   
             imsi_me[simShrdPrm.mnc_len+3] = '\0';
             me_bnw_code_last = atoi((const char *)imsi_me);
		   
             sim_bnw_code = aci_slock_extractCode( imsi_sim, simShrdPrm.mnc_len+3 );
		   
             isCodeFound = (sim_bnw_code >= me_bnw_code_first ) AND 
                                   (sim_bnw_code <= me_bnw_code_last);
		   
             aci_slock_set_me_personalisation_status ( personalisation, !isCodeFound );
		   
             if( isCodeFound )
              {
                 return;
              }
         break;
          
         case REGULAR_EXP :
         break;
    
         default :
         break;
      }
    }

    num_user_codes = ((UBYTE *)personalisation_bnw->pBody)[index];
    index +=NUM_USER_CODE_SIZE;

    index +=CURR_USER_CODE_INDEX_SIZE;

  /*
   * check user defined code-groups
   */
    for(i=0; i< num_user_codes; i++)
    {

      	/* 
         * Checks whether the NW Code is Matching with the 
         * Network Personalisation DataBase 
         */
        if(aci_slock_compare_codegroup_with_MEPD( personalisation,
                                                                                    &index,imsi_sim,
                                                                                    NW_CODE_LEN,
                                                                                    simShrdPrm.mnc_len+3,
                                                                                    (UBYTE*)personalisation_bnw->pBody,TRUE))
           {
		return;
           }
    }
  }
}



/*
+------------------------------------------------------------------------------
|  Function    : aci_slock_checkpersonalisation
+------------------------------------------------------------------------------
|  Description :   Checks for all kinds of personalisation locks (except sp & cp : Apr 03,2005). 
|        The given IMSI will be checked against the internal personalisation data. 
|        Returns SIMLOCK_BLOCKED, if one lock was not matching the given IMSI.
|
|  Parameters  :   None  
|
|  Return      : SIMLOCK_FAIL = -1,
|         SIMLOCK_DISABLED,      No SIM lock check has to be done  
|       SIMLOCK_PERM_DISABLED,
|       SIMLOCK_ENABLED,       A SIM lock check has to be executed
|       SIMLOCK_BLOCKED,       The SIM is blocked, i.e. because of a (or to many) wrong PIN(s) 
|        SIMLOCK_LOCKED         The ME is locked because of wrong SIM 
|
+------------------------------------------------------------------------------
*/
T_SIMLOCK_STATUS aci_slock_checkpersonalisation(T_SIMLOCK_TYPE   current_lock)
{
  T_SIMLOCK_TYPE type;

  UBYTE imsi_sim[MAX_IMSI_LEN+1];
  T_SIMLOCK_STATUS sl_status=SIMLOCK_FAIL;
  T_AUTOLOCK_STATUS al_status;

  TRACE_FUNCTION("aci_slock_checkpersonalisation()");
     
   psaSIM_decodeIMSI(simShrdPrm.imsi.field, simShrdPrm.imsi.c_field, (char *)imsi_sim); 
  
   /*
  *  Test SIM
  */
    if(memcmp(imsi_sim,mnc_mcc,5) EQ 0)
    {
      aci_slock_set_simtype(SIM_TEST);
       switch(cfg_data->TestCPHS)
         {
           case ALWAYS_ACCEPTED :
                  TRACE_FUNCTION("TEST SIM: always accepted");
                  return( aci_slock_check_done(SIMLOCK_ENABLED));
           case ALWAYS_REJECTED :  
                  TRACE_FUNCTION("TEST SIM: always rejected");
           break;
         }
    }
  /*   
  *  type approval
  */
    else if(aci_slock_sim_config.sim_read_ad_first_byte & 0x80)
    {
       TRACE_FUNCTION("sim type: Classic Test SIM (Type Approval SIM)");
       aci_slock_set_simtype(SIM_TYPEAPPROVAL);
     switch(cfg_data->TypeAprovalSIM)
       {
          case ALWAYS_ACCEPTED :
          TRACE_FUNCTION("type approval: always accepted");
          return( aci_slock_check_done(SIMLOCK_ENABLED));
          /* break is removed ,as case is returning before break so it is not needed */
          case ALWAYS_REJECTED :  
           TRACE_FUNCTION("type approval: always rejected");
            for (type=SIMLOCK_NETWORK; type<=SIMLOCK_SIM; type++)
            {
              if(AciSLockShrd.status[type] EQ SIMLOCK_ENABLED)
              {
                sl_status = SIMLOCK_BLOCKED;
                AciSLockShrd.blocked = TRUE;
                break;
               }
            }
           if(AciSLockShrd.blocked NEQ TRUE)
           {  
             sl_status = SIMLOCK_ENABLED ;
           }
          return(aci_slock_check_done(sl_status));
	   /* break is removed ,as case is returning before break so it is not needed */
      
         case UNTIL_NORMAL_SIM :
         if(AciSLockShrd.status[SIMLOCK_FIRST_SIM] EQ SIMLOCK_ENABLED) /*11_Apr_05*/
          {
            AciSLockShrd.blocked = TRUE;
            return (aci_slock_check_done(SIMLOCK_BLOCKED));
          }
         break;
       }
     }
     /*
       *Normal SIM 
       */
     else 
     {
       TRACE_FUNCTION("sim type: NORMAL SIM");
        aci_slock_set_simtype(SIM_NORMAL );
   
     }
#if 0//OMAPS00134957 & ASTec32151
if((cfg_data->FC_Current < cfg_data->FC_Max) OR (cfg_data->FC_Max EQ FC_MAX_INFINITE))
 {
       
  if((aci_slock_sim_config.sim_type EQ SIM_NORMAL) AND (personalisation_first_sim NEQ NULL)
       AND (AciSLockShrd.status[SIMLOCK_FIRST_SIM] EQ SIMLOCK_DISABLED) ) 
  {
           TRACE_FUNCTION("personalisation_first_sim NEQ NULL");
           al_status =  aci_ext_auto_personalise(current_lock);
           switch(al_status)
           {
            case AUTOLOCK_CMPL :
                   return (aci_slock_check_done(SIMLOCK_ENABLED));
            case AUTOLOCK_EXCT:
                    return SIMLOCK_WAIT;  /* wait for gid1 andgid2 read cnf*/
		case AUTOLOCK_FAIL:
            default : 
                    AciSLockShrd.blocked = TRUE;
                   return (aci_slock_check_done(SIMLOCK_BLOCKED)); 
            }
            
   }
  else
  {
  
  for (type=current_lock; type<SIMLOCK_LAST; type++)
  {
    AciSLockShrd.current_lock=type;

    if (AciSLockShrd.status[type] EQ SIMLOCK_ENABLED) /* initialised in aci_slock_init */
    {
      switch (type)
      {
        case SIMLOCK_NETWORK:          aci_slock_check_NWlock(imsi_sim, 0); break;
        case SIMLOCK_NETWORK_SUBSET:   aci_slock_check_NSlock(imsi_sim, 0); break;
        case SIMLOCK_SERVICE_PROVIDER:
           if (psaSIM_ChkSIMSrvSup(SRV_GrpLvl1) )
           {
             if(aci_slock_sim_config.sim_read_gid1 EQ FALSE)
             {
               aci_slock_sim_read_sim(SIM_GID1, NOT_PRESENT_8BIT, MAX_GID);
               return SIMLOCK_WAIT;
             }
           }
          else 
           {
             AciSLockShrd.blocked = TRUE; 
             return (aci_slock_check_done(SIMLOCK_BLOCKED));
           }
  
            aci_slock_check_SPlock(imsi_sim, 0); 
	      break;
        case SIMLOCK_CORPORATE:      
           if (psaSIM_ChkSIMSrvSup(SRV_GrpLvl1))
           {
            if(aci_slock_sim_config.sim_read_gid1 EQ FALSE)
            	{
               aci_slock_sim_read_sim(SIM_GID1, NOT_PRESENT_8BIT, MAX_GID);
               return SIMLOCK_WAIT; 
            	}
            }
           else 
           {
              AciSLockShrd.blocked = TRUE; 
             return (aci_slock_check_done(SIMLOCK_BLOCKED));
           }
           if (psaSIM_ChkSIMSrvSup(SRV_GrpLvl2))
           {
            if(aci_slock_sim_config.sim_read_gid2 EQ FALSE)
            	{
                aci_slock_sim_read_sim(SIM_GID2, NOT_PRESENT_8BIT, MAX_GID);
                return SIMLOCK_WAIT; 
            	}
           }
           else 
           {
             AciSLockShrd.blocked = TRUE; 
             return (aci_slock_check_done(SIMLOCK_BLOCKED));
            }
            aci_slock_check_CPlock(imsi_sim, 0); 
	      break;
		  
        case SIMLOCK_SIM:              aci_slock_check_SMlock(imsi_sim, 0); break;  
        case SIMLOCK_FIRST_SIM:              break;  
        case SIMLOCK_BLOCKED_NETWORK:          aci_slock_check_BNWlock(imsi_sim, 0); break;
        default:                       return (aci_slock_check_done(SIMLOCK_FAIL)); /* won't happen */
      }
    }

    if (AciSLockShrd.blocked)       /* if one lock isn't matching, don't try the others */
      return (aci_slock_check_done(SIMLOCK_BLOCKED));
  }
 
return (aci_slock_check_done(SIMLOCK_ENABLED));
  
 }
}
else 
{
  AciSLockShrd.blocked = TRUE;
  return (aci_slock_check_done(SIMLOCK_BLOCKED));
}
#else
{
  if((aci_slock_sim_config.sim_type EQ SIM_NORMAL) AND (personalisation_first_sim NEQ NULL)
       AND (AciSLockShrd.status[SIMLOCK_FIRST_SIM] EQ SIMLOCK_DISABLED) ) 
  {
           TRACE_FUNCTION("personalisation_first_sim NEQ NULL");
           al_status =  aci_ext_auto_personalise(current_lock);
           switch(al_status)
           {
            case AUTOLOCK_CMPL :
                   return(aci_slock_check_done(SIMLOCK_ENABLED));
            case AUTOLOCK_EXCT:
                    return SIMLOCK_WAIT;  /* wait for gid1 andgid2 read cnf*/
            case AUTOLOCK_FAIL:
            default : 
                    AciSLockShrd.blocked = TRUE;
                   return (aci_slock_check_done(SIMLOCK_BLOCKED)); 
            }
   }
  else
  {
  
  for (type=current_lock; type<=SIMLOCK_CORPORATE; type++)
  {
    AciSLockShrd.current_lock=type;

    if (AciSLockShrd.status[type] EQ SIMLOCK_ENABLED) /* initialised in aci_slock_init */
    {
      switch (type)
      {
        case SIMLOCK_NETWORK:          aci_slock_check_NWlock(imsi_sim, 0); break;
        case SIMLOCK_NETWORK_SUBSET:   aci_slock_check_NSlock(imsi_sim, 0); break;
        case SIMLOCK_SERVICE_PROVIDER:
           if (psaSIM_ChkSIMSrvSup(SRV_GrpLvl1) )
           {
             if(aci_slock_sim_config.sim_read_gid1 EQ FALSE)
             {
               aci_slock_sim_read_sim(SIM_GID1, NOT_PRESENT_8BIT, MAX_GID);
               return SIMLOCK_WAIT;
             }
           }
          else 
           {
             AciSLockShrd.blocked = TRUE; 
             return (aci_slock_check_done(SIMLOCK_BLOCKED));
           }
            aci_slock_check_SPlock(imsi_sim, 0); 
        break;
        case SIMLOCK_CORPORATE:      
           if (psaSIM_ChkSIMSrvSup(SRV_GrpLvl1))
           {
            if(aci_slock_sim_config.sim_read_gid1 EQ FALSE)
            {
               aci_slock_sim_read_sim(SIM_GID1, NOT_PRESENT_8BIT, MAX_GID);
               return SIMLOCK_WAIT; 
            }
            }
           else 
           {
              AciSLockShrd.blocked = TRUE; 
             return (aci_slock_check_done(SIMLOCK_BLOCKED));
           }
           if (psaSIM_ChkSIMSrvSup(SRV_GrpLvl2))
           {
            if(aci_slock_sim_config.sim_read_gid2 EQ FALSE)
            {
                aci_slock_sim_read_sim(SIM_GID2, NOT_PRESENT_8BIT, MAX_GID);
                return SIMLOCK_WAIT; 
            }
           }
           else 
           {
             AciSLockShrd.blocked = TRUE; 
             return (aci_slock_check_done(SIMLOCK_BLOCKED));
            }
            aci_slock_check_CPlock(imsi_sim, 0); 
         break;
        case SIMLOCK_SIM:              aci_slock_check_SMlock(imsi_sim, 0); break;  
        default:                       return (aci_slock_check_done(SIMLOCK_FAIL)); /* won't happen */
      }
    }

    if (AciSLockShrd.blocked)       /* if one lock isn't matching, don't try the others */
      return (aci_slock_check_done(SIMLOCK_BLOCKED));
  }
 
return (aci_slock_check_done(SIMLOCK_ENABLED));
  
 }
}
#endif
}




/*
+------------------------------------------------------------------------------
|  Function    : aci_slock_sim_init
+------------------------------------------------------------------------------
|  Description :   Install information found in the primitive into configuration buffers
|
|  Parameters  :   sim_mmi_insert_ind- pointer to  T_SIM_MMI_INSERT_IND
|
|  Return      : void
|
+------------------------------------------------------------------------------
*/
void aci_slock_sim_init ( T_SIM_MMI_INSERT_IND *sim_mmi_insert_ind )
{
  TRACE_FUNCTION("aci_slock_sim_init()");
  
  aci_slock_sim_config.oper_mode = sim_mmi_insert_ind->func;
  memcpy(aci_slock_sim_service_table,sim_mmi_insert_ind->sim_serv,sizeof(aci_slock_sim_service_table));
  
  aci_slock_sim_config.phase = sim_mmi_insert_ind->phase;
  aci_slock_sim_config.access_acm = sim_mmi_insert_ind->access_acm;
  aci_slock_sim_config.access_acmmax = sim_mmi_insert_ind->access_acmmax;
  aci_slock_sim_config.access_puct = sim_mmi_insert_ind->access_puct;
  aci_slock_reset();

}


/*
+------------------------------------------------------------------------------
|  Function    : aci_slock_sim_read_sim_cb
+------------------------------------------------------------------------------
|  Description :   Call back for SIM read.
|  Parameters  :   table_id- 
|
|  Return      : void
|
+------------------------------------------------------------------------------
*/
void aci_slock_sim_read_sim_cb(SHORT table_id)
{

  TRACE_FUNCTION ("aci_slock_sim_read_sim_cb()");

   simShrdPrm.atb[table_id].ntryUsdFlg = FALSE;

  /* Implements Measure 84 */
   aci_slock_sim_gid_cnf( table_id );
 }


/*
+------------------------------------------------------------------------------
|  Function    : aci_slock_sim_read_sim
+------------------------------------------------------------------------------
|  Description :    Request to read SIM card.
|  Parameters  :   data_id    - data field identifier
|        len      - actual length of data
|        max_length  - max length of data
|
|  Return      : void
|
+------------------------------------------------------------------------------
*/
void aci_slock_sim_read_sim(USHORT data_id, UBYTE len, UBYTE max_length)
{
  SHORT table_id;

  TRACE_FUNCTION ("aci_slock_sim_read_sim()");
  
  table_id = psaSIM_atbNewEntry();

  if(table_id NEQ NO_ENTRY)
  {
    simShrdPrm.atb[table_id].ntryUsdFlg = TRUE;
    simShrdPrm.atb[table_id].accType    = ACT_RD_DAT;
    /* Called only for GID1/GID2, standard files. Hence full path not required. */
    simShrdPrm.atb[table_id].v_path_info = FALSE;
    simShrdPrm.atb[table_id].reqDataFld = data_id;
    simShrdPrm.atb[table_id].dataOff    = 0;
    simShrdPrm.atb[table_id].dataLen    = len;
    simShrdPrm.atb[table_id].recMax     = max_length;
    simShrdPrm.atb[table_id].exchData   = NULL;
    simShrdPrm.atb[table_id].rplyCB     = aci_slock_sim_read_sim_cb;

    simShrdPrm.aId = table_id;
    if(psaSIM_AccessSIMData() < 0)
    {
      TRACE_FUNCTION("abc FATAL ERROR");
    }
  }
}




/*
+------------------------------------------------------------------------------
|  Function    : aci_slock_set_simtype
+------------------------------------------------------------------------------
|  Description :    Setting the sim_type value (Normal, Test SIM, Type Approval SIM
|  Parameters  :   sim_type    -   SIM_NORMAL =0,
|                   SIM_TYPEAPPROVAL,
|                   SIM_TEST
|
|  Return      : void
|
+------------------------------------------------------------------------------
*/
void aci_slock_set_simtype(T_SIM_TYPE sim_type )
{
    aci_slock_sim_config.sim_type = sim_type; 
}


/*
+------------------------------------------------------------------------------
|  Function    : aci_slock_set_CFG
+------------------------------------------------------------------------------
|  Description :    To set global variable for configuration data ( part1 of Security Driver)
|  Parameters  :   none
|
|  Return      : void
|
+------------------------------------------------------------------------------
*/
BOOL aci_slock_set_CFG(void)
{
  T_SEC_DRV_RETURN status ;
  TRACE_FUNCTION("aci_slock_set_CFG()");  
  status = sec_get_CFG(&cfg_data);
  if(status NEQ SEC_DRV_RET_Ok)
  {
    cfg_data = NULL; 
    return FALSE; 
  }
  return TRUE; 
}






/*
+------------------------------------------------------------------------------
|  Function    : aci_set_cme_error
+------------------------------------------------------------------------------
|  Description :   Set the cme error using ACI_ERR_DESC
|  Parameters  :   slocktype - lock type
|
|  Return      : void
|
+------------------------------------------------------------------------------
*/
void aci_set_cme_error(T_SIMLOCK_TYPE slocktype)
{

  
  int index = (cfg_data->FC_Current < cfg_data->FC_Max)?1:0;

 if(slocktype EQ  SIMLOCK_BLOCKED_NETWORK)
  {
    ACI_ERR_DESC( ACI_ERR_CLASS_Ext,
                                    set_cme_error_code[slocktype][index] );
  }
 else
  {
    
    if ( (slocktype >= SIMLOCK_NETWORK) AND 
          (slocktype < SIMLOCK_LAST) )
       {
            ACI_ERR_DESC( ACI_ERR_CLASS_Cme,
                                      set_cme_error_code[slocktype][index] );
       }
     else
      {
          ACI_ERR_DESC( ACI_ERR_CLASS_Cme, CME_ERR_WrongPasswd );
       }
    
  }
}
/*
+------------------------------------------------------------------------------
|  Function    : aci_set_cme_error_code
+------------------------------------------------------------------------------
|  Description :   Set the cme error code 
|  Parameters  :   err_code - cme error code
|
|  Return      : void
|
+------------------------------------------------------------------------------
*/
void aci_set_cme_error_code(T_SIMLOCK_TYPE current_lock ,T_ACI_CME_ERR *err_code)
{
int index = (cfg_data->FC_Current < cfg_data->FC_Max)?1:0;

if ( (current_lock >= SIMLOCK_NETWORK) AND 
      (current_lock < SIMLOCK_LAST)  AND (current_lock NEQ SIMLOCK_BLOCKED_NETWORK))
  {
     *err_code =
     (T_ACI_CME_ERR) set_cme_error_code[current_lock][index];
  }
else
 {
     *err_code = CME_ERR_Unknown;
      aci_set_cme_error(current_lock); 
 }
}

/*
+------------------------------------------------------------------------------
|  Function    : aci_set_cpin_code
+------------------------------------------------------------------------------
|  Description :   Set the cpin request code 
|  Parameters  :  code - cpin request code 
|
|  Return      : void
|                    code -Set the cpin request code 
|
+------------------------------------------------------------------------------
*/
void aci_set_cpin_code(T_SIMLOCK_TYPE current_lock ,T_ACI_CPIN_RSLT *code)
{

  int index = (cfg_data->FC_Current < cfg_data->FC_Max)?1:0;
  
   if ( (current_lock >= SIMLOCK_NETWORK) AND 
		(current_lock < SIMLOCK_LAST) )
   {
	 *code =
	 (T_ACI_CPIN_RSLT) aci_set_cpin_response[current_lock][index];
   }
   else
   {
	 *code = CPIN_RSLT_NotPresent;
	 ACI_ERR_DESC( ACI_ERR_CLASS_Ext, EXT_ERR_DataCorrupt );
   }
}

/*
+------------------------------------------------------------------------------
|  Function    : aci_slock_reset
+------------------------------------------------------------------------------
|  Description :   Reset the lock
|  Parameters  :  None
|
|  Return      : void
|
+------------------------------------------------------------------------------
*/
void aci_slock_reset()
{
   aci_slock_sim_config.sim_gidl1[0] = NOT_PRESENT_8BIT;
   aci_slock_sim_config.sim_gidl2[0] = NOT_PRESENT_8BIT;
   aci_slock_sim_config.sim_read_gid1= FALSE; 
   aci_slock_sim_config.sim_read_gid2= FALSE;
}

/*
+------------------------------------------------------------------------------
|  Function    : aci_slock_check_done
+------------------------------------------------------------------------------
|  Description :   Check for lock
|  Parameters  :  Status
|
|  Return      : SIMLOCK_FAIL = -1,
|                SIMLOCK_DISABLED,
|                SIMLOCK_PERM_DISABLED,
|                SIMLOCK_ENABLED,
|                SIMLOCK_BLOCKED,
|                SIMLOCK_LOCKED,
|                SIMLOCK_WAIT,
|                SIMLOCK_BUSY
|
+------------------------------------------------------------------------------
*/

T_SIMLOCK_STATUS aci_slock_check_done(T_SIMLOCK_STATUS sl_status)
{

 USHORT cmdBuf = simEntStat.curCmd;   
 T_ACI_CME_ERR err_code = CME_ERR_NotPresent; /* code holding the correct error code calculated */

 TRACE_FUNCTION_P1 ("aci_slock_check_done() %d",simEntStat.curCmd );
  switch(cmdBuf )
  {
    case( AT_CMD_CFUN ):
    case( AT_CMD_CPIN ):
    case( AT_CMD_PVRF ):
    case(AT_CMD_NRG) : 
    case( AT_CMD_SIMRST):
    case( AT_CMD_NONE ):
    case (AT_CMD_CIMI):
    case( KSD_CMD_UBLK):
      /*
       * Start to build phonebook
       */
       AciSLockShrd.pb_load = TRUE;

#ifdef TI_PS_FFS_PHB
      if(last_sim_mmi_insert_ind NEQ NULL)
      {
        pb_reset();
        pb_inserted_sim (MAX_SRV_TBL,
                         last_sim_mmi_insert_ind->sim_serv,
                         &last_sim_mmi_insert_ind->imsi_field,
                         last_sim_mmi_insert_ind->func,
                         last_sim_mmi_insert_ind->phase);
      }
#else
      if(last_sim_mmi_insert_ind NEQ NULL)
      {
        pb_reset();
        pb_build_req(last_sim_mmi_insert_ind);
      }
#endif

       /* Request the Customer Service Profile  from the SIM (EF_CPHS_CSP) */
       cmhSIM_Get_CSP();
       if(last_sim_mmi_insert_ind  NEQ NULL)
        {
           #ifdef SIM_TOOLKIT
           cmhSMS_ReadCbDtaDwnl (last_sim_mmi_insert_ind);
           #endif   
          #ifdef FF_MMI_RIV
           rAT_PlusCFUNP (last_sim_mmi_insert_ind);
          #endif /* FF_MMI_RIV */
          PFREE (last_sim_mmi_insert_ind); /* 11_Apr_05 */
          last_sim_mmi_insert_ind= NULL;   
      }
      
      cmhSIM_SIMInserted();
      aci_ext_personalisation_free();
      MFREE(cfg_data);
      break;
  
    case (AT_CMD_CLCK):
      if((AciSLockShrd.check_lock  EQ SIMLOCK_CHECK_RESET_FC) OR (AciSLockShrd.check_lock  EQ SIMLOCK_CHECK_MASTER_UNLOCK))
      {
        
       simEntStat.curCmd = AT_CMD_NONE;
       if((sl_status EQ SIMLOCK_ENABLED) AND (AciSLockShrd.pb_load EQ FALSE))
         {
          /*
           * Start to build phonebook
           */
          AciSLockShrd.pb_load = TRUE;

#ifdef TI_PS_FFS_PHB
        if(last_sim_mmi_insert_ind NEQ NULL)
        {
          pb_reset();
          pb_inserted_sim (MAX_SRV_TBL,
                           last_sim_mmi_insert_ind->sim_serv,
                           &last_sim_mmi_insert_ind->imsi_field,
                           last_sim_mmi_insert_ind->func,
                           last_sim_mmi_insert_ind->phase);
        }
#else
        if(last_sim_mmi_insert_ind NEQ NULL)
        {
          pb_reset();
          pb_build_req(last_sim_mmi_insert_ind);
        }
#endif

            /* Request the Customer Service Profile  from the SIM (EF_CPHS_CSP) */
             cmhSIM_Get_CSP();

           if(last_sim_mmi_insert_ind NEQ NULL)
           {
              #ifdef SIM_TOOLKIT
                 cmhSMS_ReadCbDtaDwnl (last_sim_mmi_insert_ind);
              #endif   
              #ifdef FF_MMI_RIV
                 rAT_PlusCFUNP (last_sim_mmi_insert_ind);
              #endif /* FF_MMI_RIV */
              PFREE (last_sim_mmi_insert_ind); /* 11_Apr_05 */
              last_sim_mmi_insert_ind= NULL;  
            }
           R_AT( RAT_OK, simEntStat.entOwn ) ((T_ACI_AT_CMD) cmdBuf );
           cmh_logRslt ( simEntStat.entOwn, RAT_OK, (T_ACI_AT_CMD)cmdBuf, -1, BS_SPEED_NotPresent, CME_ERR_NotPresent );
         }
          if ( AciSLockShrd.blocked)
          {
             aci_set_cme_error(AciSLockShrd.current_lock); 
            /* @GBR: Alternativly CME_ERR_SimWrong might be returned, but this way is telling the MMI mor specific, what went wrong. */
             aci_set_cme_error_code_and_logRslt( cmdBuf );
          }
       aci_ext_personalisation_free();
       MFREE(cfg_data);
}
	
    else if(AciSLockShrd.check_lock  EQ SIMLOCK_CHECK_LOCK)
    {
   switch(sl_status)
       {
         case SIMLOCK_ENABLED :
                simEntStat.curCmd = AT_CMD_NONE;
                R_AT( RAT_OK, simEntStat.entOwn ) ( (T_ACI_AT_CMD)cmdBuf );
                cmh_logRslt ( simEntStat.entOwn, RAT_OK, (T_ACI_AT_CMD)cmdBuf, -1, BS_SPEED_NotPresent, CME_ERR_NotPresent );
                break; 
       case SIMLOCK_WAIT : 
                break; 
       case SIMLOCK_FAIL :
                 simEntStat.curCmd = AT_CMD_NONE;
                 err_code =CME_ERR_WrongPasswd;
                 R_AT( RAT_CME, simEntStat.entOwn )
                 ( (T_ACI_AT_CMD)cmdBuf, err_code );
                 cmh_logRslt ( simEntStat.entOwn, RAT_CME, (T_ACI_AT_CMD)cmdBuf, -1, BS_SPEED_NotPresent, err_code );
                break;
       }
     }
     break; 
          
  }
 return sl_status;
}
         
/*
+------------------------------------------------------------------------------
|  Function    : aci_slock_master_unlock
+------------------------------------------------------------------------------
|  Description :   Unlock the master key
|  Parameters  :   Masterkey password
|
|  Return      :  OPER_FAIL = -1,
|                 OPER_SUCCESS,  
|                 OPER_WRONG_PASSWORD,
|                 OPER_BUSY,
|                 OPER_NOT_ALLOWED
|
+------------------------------------------------------------------------------
*/

T_OPER_RET_STATUS aci_slock_master_unlock(char *masterkey)
{
  T_OPER_RET_STATUS ret; 
  ret = aci_ext_slock_master_unlock(masterkey);
  if(ret EQ OPER_SUCCESS)
  {
    if((aci_slock_is_timer_support() EQ TRUE) AND (aci_slock_check_timer() EQ TIMER_RUNNING)
              AND (masterkey NEQ NULL))
    {
        aci_slock_unlock_timer_stopped();
    }
  }
  return ret;
}

/*
+------------------------------------------------------------------------------
|  Function    : aci_slock_start_timer
+------------------------------------------------------------------------------
|  Description :   Start the penalty Timer
|  Parameters  : None
|
|  Return      : Void
|
+------------------------------------------------------------------------------
*/
void aci_slock_start_timer(void)
{

  TRACE_FUNCTION("aci_slock_start_timer()");
  if(aci_slock_is_timer_support() EQ TRUE)
  { 
    if(!aci_slock_set_CFG())
    {
      return ;
    }
    if(cfg_data->FC_Current  NEQ 0xFF)
    {
      TIMERSTART((ACI_UNLOCK_TIMER*cfg_data->FC_Current),ACI_UNLOCK_TIMER_HANDLE);
      aci_slock_set_timer_flag(TIMER_RUNNING);
    }
    MFREE(cfg_data); 
  }
  return; 
}

/*
+------------------------------------------------------------------------------
|  Function    : aci_slock_check_timer
+------------------------------------------------------------------------------
|  Description :   Check for time active or not
|  Parameters  :   None
|
|  Return      :  timer status
|
+------------------------------------------------------------------------------
*/

GLOBAL  UBYTE aci_slock_check_timer(void)
{
  return (aci_ext_check_timer());
}


/*
+------------------------------------------------------------------------------
|  Function    : aci_slock_unlock_timer_stopped
+------------------------------------------------------------------------------
|  Description :   Check for time is stoped or not
|  Parameters  :   None
|
|  Return      :  void
|
+------------------------------------------------------------------------------
*/
GLOBAL void aci_slock_unlock_timer_stopped(void)
{
  aci_slock_set_timer_flag(TIMER_STOPPED); 
  return; 
}

/*
+------------------------------------------------------------------------------
|  Function    : aci_slock_set_timer_flag
+------------------------------------------------------------------------------
|  Description :   Set the timer flag
|  Parameters  :   Status
|
|  Return      :  void
|
+------------------------------------------------------------------------------
*/

GLOBAL void aci_slock_set_timer_flag(UBYTE status)
{
  aci_ext_set_timer_flag(status); 
}

/*
+------------------------------------------------------------------------------
|  Function    : aci_slock_is_timer_support
+------------------------------------------------------------------------------
|  Description :   Check the timer ,is support or not
|  Parameters  :   None
|
|  Return      :   Status of timer
|
+------------------------------------------------------------------------------
*/

GLOBAL UBYTE aci_slock_is_timer_support()
{ 
  UBYTE result; 
  if(!aci_slock_set_CFG())
  {
    return FALSE;
  }
  result = aci_ext_is_timer_support() ;
  MFREE(cfg_data);
  return result; 
}


/*
+------------------------------------------------------------------------------
|  Function    : aci_slock_send_RAT
+------------------------------------------------------------------------------
|  Description :   Send RAT_CME message to MMI
|  Parameters  :   cmdBuf  - Current AT command
|                          err_code - CME error code
|  Return      :   none
|
+------------------------------------------------------------------------------
*/

GLOBAL  void aci_slock_send_RAT(UBYTE cmdBuf,T_ACI_CME_ERR err_code )
{ 

   R_AT( RAT_CME, simEntStat.entOwn )
              ( (T_ACI_AT_CMD)cmdBuf, err_code );
   cmh_logRslt ( simEntStat.entOwn, RAT_CME, (T_ACI_AT_CMD)cmdBuf, -1, 
                              BS_SPEED_NotPresent, err_code );
  
}


/*
+------------------------------------------------------------------------------
|
|  Function    : aci_slock_sim_gid_cnf
+------------------------------------------------------------------------------
|  Description : read SIM group identifier 1 or 2 from SIM card
|
|  Parameters  : table_id - 
|
|  Return      : void
|
+------------------------------------------------------------------------------
*/

void aci_slock_sim_gid_cnf( SHORT table_id )
{
  T_SIMLOCK_STATUS sl_status;
  

  TRACE_FUNCTION( "aci_slock_sim_gid_cnf()" );

  if ( simShrdPrm.atb[table_id].errCode EQ SIM_NO_ERROR )
  {
    if(simShrdPrm.atb[table_id].reqDataFld EQ SIM_GID1)
    {
      /* The length needs to be modfied when there is no ERROR from SIM */
      aci_slock_sim_config.gid1_len = simShrdPrm.atb[table_id].dataLen;
      aci_slock_sim_config.sim_read_gid1 = TRUE;
      memcpy( aci_slock_sim_config.sim_gidl1,
              simShrdPrm.atb[table_id].exchData, MAX_GID );
    }
    else /*SIM_GID2*/
    {
      /* The length needs to be modfied when there is no ERROR from SIM */
      aci_slock_sim_config.gid2_len= simShrdPrm.atb[table_id].dataLen;
      aci_slock_sim_config.sim_read_gid2 = TRUE;
      memcpy( aci_slock_sim_config.sim_gidl2,
              simShrdPrm.atb[table_id].exchData, MAX_GID );
    }

    if( simEntStat.curCmd NEQ AT_CMD_NONE )
    {
      if( AciSLockShrd.check_lock EQ SIMLOCK_CHECK_LOCK )
      {
        sl_status = aci_slock_lock( AciSLockShrd.lock_type,
                                    AciSLockShrd.lock_passwd );
        aci_slock_check_done( sl_status );
      }
      else 
      {
        /*
         * Since AciSLockShrd.check_lock can have only 3 values , hence there
         * is no need to put 
         * else if( (AciSLockShrd.check_lock EQ SIMLOCK_CHECK_PERS) OR 
                    (AciSLockShrd.check_lock EQ SIMLOCK_CHECK_RESET_FC) )
         */

        /* check for SP and corporate personalisation.
           This is the first case, we can do this,
           as we have the gid1/gid2 file here */
        sl_status= aci_slock_checkpersonalisation(AciSLockShrd.current_lock);
      }
    }
  }
}

/* Implements Measure  82 */
/*
+------------------------------------------------------------------------------
|  Function    : aci_slock_compare_codegroup_with_MEPD
+------------------------------------------------------------------------------
|  Description : This function compares the different code group present in SIM
|                with the code group present in the different category of 
|                MEPD DataBase 
|
|                This function first extracts the code group present in the 
|                different category of MEPD DataBase from the argument passed
|                as pBody and then compares with the code group present in 
|                the SIM which is passed as an argument imsi_sim.
|
|                If matches returns TRUE otherwise FALSE.
|
|  Parameters  : personalisation - 0- add the lock data
|                index           - array index
|                num_user_codes  - No. of user codes
|                imsi_sim        - IMSI
|                code_len        - length
|                pBody           - Different Category of Personalisation Data.
|
|  Return      : BOOL - if the IMSI matches returns TRUE otherwise FASLE
+------------------------------------------------------------------------------
*/

LOCAL BOOL aci_slock_compare_codegroup_with_MEPD( UBYTE personalisation,
                                                  UINT16 *index,
                                                  UBYTE *imsi_sim,
                                                  UBYTE code_len,
                                                  UBYTE len,
                                                  UBYTE *pBody, BOOL bnw_flag )
{
  UBYTE imsi_me[MAX_IMSI_LEN+1];
  BOOL isCodeFound;

  TRACE_FUNCTION( "aci_slock_compare_codegroup_with_MEPD()" );

  aci_slock_decode_MEPD( pBody, index, code_len, imsi_me );

  isCodeFound = memcmp( imsi_sim, imsi_me, len ) EQ 0;
 if(bnw_flag)
 {
   aci_slock_set_me_personalisation_status ( personalisation, !isCodeFound );
 }
 else 
 {
   aci_slock_set_me_personalisation_status ( personalisation, isCodeFound );
 }


  if (isCodeFound)
  {
    return TRUE;
  }
  return FALSE;
}

/* Implements Measure  26 */
/*
+------------------------------------------------------------------------------
|  Function    : aci_set_cme_error_code_and_logRslt
+------------------------------------------------------------------------------
|  Description : Sets the cme error code and logs the result code
|
|  Parameters  : cmdBuf -  Current Command
|
|  Return      : void
+------------------------------------------------------------------------------
*/

GLOBAL void aci_set_cme_error_code_and_logRslt( UBYTE cmdBuf )
{
  T_ACI_CME_ERR err_code;

  TRACE_FUNCTION( "aci_set_cme_error_code_and_logRslt()" );

  aci_set_cme_error_code( AciSLockShrd.current_lock, &err_code );  
  aci_slock_send_RAT(cmdBuf, err_code ); 
  
  }

/* Implements NEW Measure */
/*
+------------------------------------------------------------------------------
|  Function    : aci_slock_compare_NSCode
+------------------------------------------------------------------------------
|  Description : This function will compare the network subset code present 
|                in SIM with the network subset code which is present in MEPD 
|                DataBase
|
|                IF the comparison matches the function will return TRUE, 
|                otherwise FALSE
|
|  Parameters  : me_nw_code        - Network Code from MEPD DataBase
|                me_nw_ns_code_str - Network Subset Code String from
|                                    MEPD DataBase.
|
|                imsi_sim          - IMSI from SIM
|
|  Return      : BOOL - Returns the Result of the comparison
+------------------------------------------------------------------------------
*/

LOCAL UBYTE aci_slock_compare_NSCode ( UBYTE *me_nw_code,
                                       UBYTE *me_nw_ns_code_str,
                                       UBYTE *imsi_sim )
{
  int me_ns_code_first, me_ns_code_last, sim_ns_code;
  BOOL cmp_result;

  TRACE_FUNCTION( "aci_slock_compare_NSCode()" );

  cmp_result = memcmp( imsi_sim, me_nw_code, simShrdPrm.mnc_len+3 ) EQ 0;

  if ( cmp_result EQ TRUE )
  {
    me_ns_code_first = aci_slock_extractCode( me_nw_ns_code_str+7, 2 );
    
    me_ns_code_last = aci_slock_extractCode( me_nw_ns_code_str+7+2, 2 );

    sim_ns_code = aci_slock_extractCode( imsi_sim+simShrdPrm.mnc_len+3, 2 );
    
    if((sim_ns_code >= me_ns_code_first ) AND (sim_ns_code <= me_ns_code_last))
    {
      return TRUE; /* Matches */
    }
    else
    {
      return FALSE;
    }
  }
  return ( cmp_result );
}

/* Implements NEW Measure */
/*
+------------------------------------------------------------------------------
|  Function    : aci_slock_compare_MSINCode
+------------------------------------------------------------------------------
|  Description : This function will compare MSIN Code from MEPD DataBase
|                with the MSIN Code present in SIM 
|
|                IF the comparison matches the function will return TRUE, 
|                otherwise FALSE
|
|  Parameters  : me_nw_ns_msin_msin_code_str  - msin Code String from
|                                               MEPD DataBase.
|                imsi_sim                     - IMSI from SIM
|
|  Return      : BOOL - Returns the Result of the comparison 
+------------------------------------------------------------------------------
*/

LOCAL UBYTE aci_slock_compare_MSINCode( UBYTE *me_nw_ns_msin_msin_code_str,
                                        UBYTE *imsi_sim )
{
  UBYTE me_nw_ns_code_str[6+2+1];
  BOOL cmp_result;
  int me_msin_code_first, me_msin_code_last, sim_msin_code;

  TRACE_FUNCTION( "aci_slock_compare_MSINCode()" );

  memcpy(me_nw_ns_code_str,me_nw_ns_msin_msin_code_str,simShrdPrm.mnc_len+3+2);

  cmp_result = memcmp( imsi_sim, me_nw_ns_code_str,
                       simShrdPrm.mnc_len+3+2 ) EQ 0;

  if ( cmp_result EQ TRUE )
  {
    me_msin_code_first = aci_slock_extractCode( me_nw_ns_msin_msin_code_str+9,
                                                (MAX_IMSI_LEN - simShrdPrm.mnc_len+3+2) );

    me_msin_code_last = aci_slock_extractCode( me_nw_ns_msin_msin_code_str+9+8, 
                                               (MAX_IMSI_LEN - simShrdPrm.mnc_len+3+2) );

    sim_msin_code = aci_slock_extractCode( imsi_sim+simShrdPrm.mnc_len+3+2, 
                                           (MAX_IMSI_LEN - cfg_data->MNC_Len+3+2) );

    if( (sim_msin_code >= me_msin_code_first ) AND 
        (sim_msin_code <= me_msin_code_last) )
    {
      return TRUE; /* Matches */
    }
    else 
    {
      return FALSE;
    }
  }
  return ( cmp_result );
}

/*
+------------------------------------------------------------------------------
|  Function    : aci_slock_compare_NSCode
+------------------------------------------------------------------------------
|  Description : This function will extract the code in Integer format 
|                from the string that has been passed as a parameter source_str
|
|  Parameters  : souce_str        - Source String from which the code is 
|                                   extracted out.
|                len              - Length
|
|  Return      : BOOL - Returns the Code in Integer Format
+------------------------------------------------------------------------------
*/

LOCAL int aci_slock_extractCode( UBYTE *source_str, UBYTE len )
{
    UBYTE dest_str[MAX_IMSI_LEN+1];

    memcpy(dest_str,source_str, len);
    dest_str[len] = '\0';
    return( atoi((const char *)dest_str));
}

/* Implements Measure 212 */
/*
+------------------------------------------------------------------------------
|  Function    : aci_slock_check_isNSCodePresent
+------------------------------------------------------------------------------
|  Description : This function is intended to compare the Network Subset Code
|                from the Network Subset Personalisation Database and SIM.
|
|                First it will compare the Network Code between the 
|                Network Subset Personalisation Database and SIM.
|
|                IF that matches then it will compare the Network Subset Code
|                between the Network Subset Personalisation Database and SIM.
|
|                If the comparison matches the function will return TRUE,
|                otherwise FALSE
|
|  Parameters  : me_nw_code       - Network Subset Code from 
|                                   Network Subset Personalisation
|                imsi_sim         - IMSI from SIM
|                index            - 
|
|  Return      : BOOL - Returns the Result of the comparison 
+------------------------------------------------------------------------------
*/

LOCAL UBYTE aci_slock_check_isNSCodePresent( UBYTE *me_nw_code,
                                             UBYTE *imsi_sim,
                                             UINT16 *index )
{
  UBYTE imsi_sim_8or9th_digit_str[1+1];
  UBYTE me_ns_code_bcd[NS_CODE_LEN];
  UBYTE me_ns_code[2 +1];
  

  UBYTE no_of_normal_8th_digit_ns_code; 
  UBYTE digit_8th_value;
  UBYTE no_of_digit_8th_value;
  UBYTE no_of_interval_8th_digit_ns_code;

  BOOL cmp_result;
  int me_ns_code_first,
  me_ns_code_last,
  sim_ns_code,
  sim_8or9th_digit_code,
  j,k;

  TRACE_FUNCTION( "aci_slock_check_isNSCodePresent()" );

  cmp_result = memcmp(imsi_sim,me_nw_code,simShrdPrm.mnc_len+3) EQ 0;
  no_of_digit_8th_value = ((UBYTE *)personalisation_ns->pBody)[*index] ; 
  *index += 1; 
  if ( cmp_result EQ TRUE )
  {
    cmp_result = FALSE;

    imsi_sim_8or9th_digit_str[0] = imsi_sim[simShrdPrm.mnc_len+3+2];
    imsi_sim_8or9th_digit_str[1]  ='\0';
    sim_8or9th_digit_code = atoi((const char *)imsi_sim_8or9th_digit_str);

    for(j=0; j<no_of_digit_8th_value; j++ )
    {
      digit_8th_value = ((UBYTE *)personalisation_ns->pBody)[*index];
      *index += 1;
      no_of_normal_8th_digit_ns_code  =
                        ((UBYTE *)personalisation_ns->pBody)[*index];
      *index += 1;
      no_of_interval_8th_digit_ns_code  =
                        ((UBYTE *)personalisation_ns->pBody)[*index];
      *index += 1;
      
      if( sim_8or9th_digit_code EQ digit_8th_value )
      {
        for( k =0; k < no_of_normal_8th_digit_ns_code; k++ )
        {
          memcpy( me_ns_code_bcd, &(((UBYTE *)personalisation_ns->pBody)[*index]),
                  NS_CODE_LEN );
          *index += NS_CODE_LEN; 
          aci_slock_psaSIM_decodeIMSI_without_parity( me_ns_code_bcd,
                                                      NS_CODE_LEN,
                                                      (char *)me_ns_code);

          cmp_result = memcmp( imsi_sim + simShrdPrm.mnc_len+3,
                               me_ns_code, 2 ) EQ 0;

          if( cmp_result EQ TRUE )
          {
            return TRUE; /* Matches */
          }
        }
        if( cmp_result EQ FALSE )
        {
          for(k =0; k < no_of_interval_8th_digit_ns_code; k++)
          {
            memcpy( me_ns_code_bcd,
                    &(((UBYTE *)personalisation_ns->pBody)[*index]),NS_CODE_LEN);
            *index += NS_CODE_LEN; 
            aci_slock_psaSIM_decodeIMSI_without_parity( me_ns_code_bcd,
                                                        NS_CODE_LEN,
                                                        (char *)me_ns_code );
            
            me_ns_code[2] = '\0';
            me_ns_code_first =  atoi((const char *)me_ns_code);

            memcpy( me_ns_code_bcd,
                    &(((UBYTE *)personalisation_ns->pBody)[*index]),
                    NS_CODE_LEN );

            *index += NS_CODE_LEN;
            aci_slock_psaSIM_decodeIMSI_without_parity( me_ns_code_bcd,
                                                        NS_CODE_LEN,
                                                        (char *)me_ns_code ); 
            
            me_ns_code[2] = '\0';
            me_ns_code_last =  atoi((const char *)me_ns_code);

            sim_ns_code = 
                aci_slock_extractCode( &imsi_sim[simShrdPrm.mnc_len+3], 2 );

            if( ( sim_ns_code >= me_ns_code_first ) AND 
                ( sim_ns_code <= me_ns_code_last ) )
            {
              return TRUE; /* Matches */
            }
            else 
            {
              return FALSE;
            }
          }
        }
        else
        {
          *index += ( NS_CODE_LEN*no_of_normal_8th_digit_ns_code + 
                     2*NS_CODE_LEN*no_of_interval_8th_digit_ns_code ) ;
        }
      }
    }
  }
  return FALSE;
}

/* Implements Measure 146 */
/*
+------------------------------------------------------------------------------
|  Function    : aci_slock_compare_gid_str
+------------------------------------------------------------------------------
|  Description : This function will compare the gid str present in the SIM 
|                and the gid str present in the MEPD Database.
|                
|                This function can compare both the gid str i.e for 
|                Group Identifier Level 1 and Group Identifier Level 2.
|                depending upon the Group Identifier Level passed as an
|                argument grp_lvl.
|
|
|  Parameters  : imsi_me  - IMSI from Personalisation DataBase
|                imsi_sim - IMSI from SIM
|                index    - 
|                grp_lvl  -  Group Identifier Level 1 or
|                            Group Identifier Level 2
|
|  Return      : BOOL - Returns the Result of the comparison 
+------------------------------------------------------------------------------
*/

LOCAL UBYTE aci_slock_compare_gid_str( UBYTE *imsi_me,
                                       UBYTE *imsi_sim,
                                       UINT16 *index,
                                       T_SIM_SRV grp_lvl)
{
  UBYTE cmp_result = TRUE;
  UBYTE me_gid1_str[GID1_LEN+1];
  UBYTE me_gid2_str[GID2_LEN+1];
  UBYTE sim_gid1_str[GID1_LEN+1];
  UBYTE sim_gid2_str[GID2_LEN+1];  

  TRACE_FUNCTION( "aci_slock_compare_gid_str()" );
  if(imsi_me[0] NEQ 0)
  {
    cmp_result = memcmp( imsi_sim, imsi_me,simShrdPrm.mnc_len+3 ) EQ 0;
  }

  if ( cmp_result EQ TRUE )
  {
    if ( psaSIM_ChkSIMSrvSup(SRV_GrpLvl1) )
    {
      if( grp_lvl EQ SRV_GrpLvl1 )
      {
        return ( aci_slock_compare_gid( me_gid1_str, 
                                        aci_slock_sim_config.sim_gidl1,
                                        sim_gid1_str, GID1_LEN,
                                        (UBYTE *)personalisation_sp->pBody,
                                        index ) );
      }
      else
      {
        memcpy( me_gid1_str,&(((UBYTE *)personalisation_cp->pBody)[*index]),
                GID1_LEN);
        *index +=GID1_LEN;

        cmp_result = memcmp( aci_slock_sim_config.sim_gidl1, me_gid1_str,
                             cfg_data->GID1_Len ) EQ 0;

        if ( cmp_result EQ TRUE )
        {
          if ( psaSIM_ChkSIMSrvSup(SRV_GrpLvl2) )
          {
            return ( aci_slock_compare_gid( me_gid2_str,
                                            aci_slock_sim_config.sim_gidl2,
                                            sim_gid2_str, GID2_LEN,
                                            (UBYTE *)personalisation_cp->pBody,
                                            index ) );
          }
        }        
      }
    }  
  }
  return FALSE;
}

/*
+------------------------------------------------------------------------------
|  Function    : aci_slock_compare_gid
+------------------------------------------------------------------------------
|  Description : This function will compare gid str of SIM and the MEPD
|                DataBase and returns TRUE if it matches otherwise FALSE
|
|
|  Parameters  : me_gid_str  - 
|                sim_gidl    - gid string from aci_slock_sim_config
|                sim_gid_str - 
|                gid_len     - gid length
|                pBody       - Personalisation Data
|                index       - 
| 
|  Return      : BOOL - Returns the Result of the comparison 
+------------------------------------------------------------------------------
*/

LOCAL UBYTE aci_slock_compare_gid( UBYTE *me_gid_str, UBYTE *sim_gidl,
                                   UBYTE *sim_gid_str, UBYTE gid_len,
                                   UBYTE *pBody, UINT16 *index )
{
  int sim_gid, me_gid_first, me_gid_last;

  TRACE_FUNCTION( "aci_slock_compare_gid()" );

  me_gid_first = aci_slock_extractCode( &(pBody[*index]), gid_len );
  *index +=gid_len;

  me_gid_last = aci_slock_extractCode( &(pBody[*index]), gid_len );
  *index +=gid_len;

  sim_gid = aci_slock_extractCode( sim_gidl, gid_len );

  if((sim_gid >= me_gid_first) AND (sim_gid <= me_gid_last))
  {
    return TRUE; /* Matches */
  }
  else
  {
    return FALSE;
  }
}

/* Implements Measure 160 */
/*
+------------------------------------------------------------------------------
|  Function    : aci_slock_check_isgidPresent
+------------------------------------------------------------------------------
|  Description : This function will compare gid str of SIM and the MEPD
|                DataBase for the NORMAL CODE type 
|                and returns TRUE if it matches otherwise FALSE
|
|
|  Parameters  : pBody    - Personalisation Data
|                imsi_sim - IMSI from SIM
|                imsi_me  - IMSI from Personalisation DataBase
|                grp_lvl  - Group Identifier Level 1 or
|                           Group Identifier Level 2
|                index    - 
| 
|  Return      : BOOL - Returns the Result of the comparison 
+------------------------------------------------------------------------------
*/

LOCAL UBYTE aci_slock_check_isgidPresent( UBYTE *pBody, UBYTE *imsi_sim,
                                          UBYTE *imsi_me, T_SIM_SRV grp_lvl, UINT16 *index )
{
  UBYTE me_gid1_str[GID1_LEN+1];
  UBYTE me_gid2_str[GID2_LEN+1];
  UBYTE cmp_result = TRUE;

  TRACE_FUNCTION( "aci_slock_check_isgidPresent()" );
  if(imsi_me[0] NEQ 0)
  {
    cmp_result = memcmp(imsi_sim,imsi_me,simShrdPrm.mnc_len+3) EQ 0;
  }

  if ( cmp_result EQ TRUE )
  {
    if ( psaSIM_ChkSIMSrvSup(SRV_GrpLvl1) )
    {
      memcpy( me_gid1_str,&(pBody[*index]), GID1_LEN );
      *index +=GID1_LEN;
      cmp_result = memcmp( aci_slock_sim_config.sim_gidl1, 
                           me_gid1_str, cfg_data->GID1_Len) EQ 0;

      if ( cmp_result EQ TRUE )
      {
        if( grp_lvl EQ SRV_GrpLvl2 )
        {
          if (psaSIM_ChkSIMSrvSup(SRV_GrpLvl2))
          {
            memcpy( me_gid2_str, &(pBody[*index]), GID2_LEN);
            *index +=GID2_LEN;
            cmp_result = memcmp( aci_slock_sim_config.sim_gidl2,
                                 me_gid2_str, cfg_data->GID2_Len ) EQ 0; 
            if ( cmp_result EQ TRUE )
            {
              return TRUE;
            }
          }
        }
        else
        {
          return TRUE;
        }
      }
    }
  }
  return FALSE;
}

/* Implements NEW Measure */
/*
+------------------------------------------------------------------------------
|  Function    : aci_slock_decode_MEPD
+------------------------------------------------------------------------------
|  Description : This function will convert IMSI of MEPD DataBase 
|                from BCD to ASCII format.
|
|  Parameters  : pBody       - Personalisation Data
|                index       -
|                code_len    - Code Length
|                imsi_ascii  - IMSI in ASCII Format (OutPut parameter)
| 
|  Return      : void
+------------------------------------------------------------------------------
*/

LOCAL void aci_slock_decode_MEPD( UBYTE *pBody, UINT16 *index, UBYTE code_len,
                                  UBYTE *imsi_ascii )
{
  UBYTE me_code_bcd[NW_NS_MSIN_MSIN_CODE_LEN];
  /* 
   * NW_NS_MSIN_MSIN_CODE_LEN has been used because this is the biggest size 
   * needed by the callers of the this function
   */
  TRACE_FUNCTION( "aci_slock_decode_MEPD()" );

  memcpy( me_code_bcd, &(pBody[*index]), code_len );
  *index += code_len;
  aci_slock_psaSIM_decodeIMSI( me_code_bcd, code_len, (char *)imsi_ascii );
}

/* Implements NEW Measure */
/*
+------------------------------------------------------------------------------
|  Function    : aci_slock_set_me_personalisation_status
+------------------------------------------------------------------------------
|  Description : This function will set the Global Flag AciSLockShrd.blocked
|                or sim_code_present_in_me.
|
|  Parameters  : personalisation - Personalisation 
|                status          - TRUE or FALSE
| 
|  Return      : void 
+------------------------------------------------------------------------------
*/

LOCAL void aci_slock_set_me_personalisation_status( UBYTE personalisation,
                                                    UBYTE status )
{
  TRACE_FUNCTION( "aci_slock_set_me_personalisation_status()" );

  if( !personalisation )
  {
    AciSLockShrd.blocked = !status;
  }
  else 
  {
    sim_code_present_in_me = status;
  }

}

#endif
