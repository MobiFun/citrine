/*
+-----------------------------------------------------------------------------
|  Project :
|  Modul   :  J:\g23m-aci\aci_ext\aci_ext_pers.c
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

#include "config.h"
#include "fixedconf.h"
#include "condat-features.h"
#include "aci_conf.h"

#include "aci_all.h"

#include "aci_cmh.h"
#include "ati_cmd.h"
#include "aci_cmd.h"

#ifdef FAX_AND_DATA
#include "aci_fd.h"
#endif    /* of #ifdef FAX_AND_DATA */

#ifndef WIN32
#define ACI_PERSONALISTION_USE_FFS
#endif

#include "aci.h"
#include "psa.h"
#include "psa_sim.h"
#include "psa_sms.h"
#include "psa_mmi.h"
#include "cmh.h"
#include "phb.h"
#include "aoc.h"

#include "psa_sim.h"            /* simShrdPrm */

#include "aci_ext_pers.h"
#include "aci_slock.h"

#ifdef SIM_PERS
#include "general.h" /* inluded for UINT8 compilation error in sec_drv.h */
#include "sec_drv.h"
#endif

#include "pcm.h"                /* EF_SIMLCKEXT */

#ifdef MMI_SIMLOCK_TEST
  #define ACI_PERSONALISTION_USE_FFS
#endif

#ifdef ACI_PERSONALISTION_USE_FFS
#include "../../services/ffs/ffs.h"
#include "ffs_coat.h"
#endif /* ACI_PERSONALISTION_USE_FFS */



#define SLOCK_PCM_NRM 1         /* SIM lock data is stored as SIMLOCK to the PCM */
#define SLOCK_PCM_EXT 2         /* SIM lock data is stored as SIMLOCKEXT to the PCM */

#ifdef SIM_PERS
#define TRUNCATION_KEY_LEN 8         /*  */
#endif

#ifdef _SIMULATION_
EF_SIMLCKEXT simlock_pcm_memory;    /* emulates the PCM memory in WIN32 environment. */
#endif

#include "aci_ext_pers_cus.h" /* the customer specific personalisation data. */


#ifdef FF_PHONE_LOCK
  T_OPER_RET_STATUS aci_ext_set_phone_lock_satus(T_SEC_DRV_EXT_PHONE_LOCK_TYPE type,T_SEC_DRV_EXT_PHONE_LOCK_STATUS status,const char *passwd);
  T_OPER_RET_STATUS aci_ext_get_phone_lock_satus(T_SEC_DRV_EXT_PHONE_LOCK_TYPE type,T_SEC_DRV_EXT_PHONE_LOCK_STATUS *status);
  T_OPER_RET_STATUS aci_ext_set_phone_lock_key(const char *pOldKey,const char *pNewKey);
#endif
LOCAL void aci_ext_personalisation_MMI_save( void );

/*lint -esym(528,pre_firstsimpersonalisation ) pre_firstsimpersonalisation not referenced */
static BOOL pre_firstsimpersonalisation = FALSE;   /* this will be set when a FIRST_SIM_LOCK is applied. All further locks
                                                      will not be applied but only prepare the lock for the next power-cycle */
LOCAL void decodeB2A (U8 *bcd, U8 bcd_len, U8 *ascii);          /* decode a BCD to ASCII */
LOCAL void encodeA2B (char *ascii, UBYTE *bcd, UBYTE bcd_len);  /* encode ASCII to BCD, fill up with 0xf */
#ifdef SIM_PERS
EXTERN T_SEC_DRV_CONFIGURATION *cfg_data ;
EXTERN  T_ACI_SIM_CONFIG aci_slock_sim_config;     /* SIM configuration, initialised by a T_SIM_MMI_INSERT_IND */
#endif 




/*
+------------------------------------------------------------------------------
|  Function    : aci_ext_personalisation_init
+------------------------------------------------------------------------------
|  Description : Initialisation of this extension. Might be used to initialise variables, set temporary data to default values and so on.
  This method will be called once before any other public method of this extension.
|
|  Parameters  : void
|
|  Return      : void
|
+------------------------------------------------------------------------------
*/


void aci_ext_personalisation_init( void )
{
#ifdef SIM_PERS
  T_SIMLOCK_TYPE lock_type;
  T_SEC_DRV_RETURN  status ;
#endif

  TRACE_FUNCTION("aci_ext_personalisation_init()");

#ifndef MMI_SIMLOCK_TEST
 
#ifdef SIM_PERS
  for (lock_type=SIMLOCK_NETWORK;lock_type< SIMLOCK_LAST ;lock_type++)  /*prateek: changed loop till simlock_first_sim */
  {
    switch (lock_type)
    {
      case SIMLOCK_NETWORK:       
        status =sec_get_REC (lock_type, &personalisation_nw); 
        if (status NEQ SEC_DRV_RET_Ok)
           personalisation_nw = NULL;
        else 
        {
          AciSLockShrd.status[lock_type]=(T_SIMLOCK_STATUS)personalisation_nw->Header.Status;
          AciSLockShrd.dependency[lock_type]=personalisation_nw->Header.Dependency;
        }
        break;
              
      case SIMLOCK_NETWORK_SUBSET:   
        status =sec_get_REC (lock_type, &personalisation_ns); 
        if (status NEQ SEC_DRV_RET_Ok)
          personalisation_ns= NULL;
        else 
        {
          AciSLockShrd.status[lock_type]=(T_SIMLOCK_STATUS)personalisation_ns->Header.Status;
          AciSLockShrd.dependency[lock_type]=personalisation_ns->Header.Dependency;
        }
         break;
    
      case SIMLOCK_SERVICE_PROVIDER:  
        status =sec_get_REC (lock_type, &personalisation_sp); 
        if (status NEQ SEC_DRV_RET_Ok)
          personalisation_sp= NULL;
        else 
        {
          AciSLockShrd.status[lock_type]=(T_SIMLOCK_STATUS)personalisation_sp->Header.Status;
          AciSLockShrd.dependency[lock_type]=personalisation_sp->Header.Dependency;
        }
        break;
    
      case SIMLOCK_CORPORATE:      
        status =sec_get_REC (lock_type, &personalisation_cp); 
        if (status NEQ SEC_DRV_RET_Ok)
          personalisation_cp = NULL;
        else
        {
          AciSLockShrd.status[lock_type]=(T_SIMLOCK_STATUS)personalisation_cp->Header.Status;
          AciSLockShrd.dependency[lock_type]=personalisation_cp->Header.Dependency;
        }
        break;
    
      case SIMLOCK_SIM:                  
        status =sec_get_REC (lock_type, &personalisation_sim); 
        if (status NEQ SEC_DRV_RET_Ok)
          personalisation_sim = NULL;
        else 
        {
          AciSLockShrd.status[lock_type]=(T_SIMLOCK_STATUS)personalisation_sim->Header.Status;
          AciSLockShrd.dependency[lock_type]=personalisation_sim->Header.Dependency;
        }
        break;

       case SIMLOCK_BLOCKED_NETWORK:		
           status =sec_get_REC (lock_type, &personalisation_bnw); 
            if (status NEQ SEC_DRV_RET_Ok)
		   personalisation_bnw = NULL;
            else 
             {
                 AciSLockShrd.status[lock_type]=(T_SIMLOCK_STATUS)personalisation_bnw->Header.Status;
                AciSLockShrd.dependency[lock_type]=personalisation_bnw->Header.Dependency;
             }
        break;
    
        case SIMLOCK_FIRST_SIM:               
          status =sec_get_REC (lock_type, &personalisation_first_sim); 
          if (status NEQ SEC_DRV_RET_Ok)
          {
            TRACE_FUNCTION("personalisation_first_sim = NULL");
            personalisation_first_sim = NULL;
          }
          else 
          {
            AciSLockShrd.status[lock_type]=(T_SIMLOCK_STATUS)personalisation_first_sim->Header.Status;
            AciSLockShrd.dependency[lock_type]=personalisation_first_sim->Header.Dependency;
          }
          break;
     }
  }
#endif /*SIM_PERS */

#else /*for mmi testing, just read FFS  MMI_SIMLOCK_TEST  */
  
#endif /*MMI_SIMLOCK_TEST */
}

#ifdef SIM_PERS

/*
+------------------------------------------------------------------------------
|  Function    : aci_ext_personalisation_free
+------------------------------------------------------------------------------
|  Description : Free the MEPD structure Memory
|
|  Parameters  : void
|
|  Return      : void
|
+------------------------------------------------------------------------------
*/
void aci_ext_personalisation_free( void )
{
   T_SIMLOCK_TYPE lock_type;

   for (lock_type=SIMLOCK_NETWORK;lock_type<SIMLOCK_LAST;lock_type++)
   {
      switch (lock_type)
      {
         case SIMLOCK_NETWORK:   
           if(personalisation_nw NEQ NULL)
           {
              if(personalisation_nw->pBody NEQ NULL)
              {
                MFREE(personalisation_nw->pBody);
              }
              MFREE(personalisation_nw);
            }
          break; 
      
          case SIMLOCK_NETWORK_SUBSET: 
            if(personalisation_ns NEQ NULL)
            {
               if(personalisation_ns->pBody NEQ NULL)
              {
                MFREE(personalisation_ns->pBody);
               }
               MFREE(personalisation_ns);
             }
          break ; 

          case SIMLOCK_SERVICE_PROVIDER:  
            if(personalisation_sp NEQ NULL)
            {
               if(personalisation_sp->pBody NEQ NULL)
              {
                MFREE(personalisation_sp->pBody);
              }
              MFREE(personalisation_sp);
            }
            break ; 
      
            case SIMLOCK_CORPORATE: 
              if(personalisation_cp NEQ NULL)
             {
                 if(personalisation_cp->pBody NEQ NULL)
                {
                  MFREE(personalisation_cp->pBody);
                }
                MFREE(personalisation_cp);
             }
             break ; 
       
             case SIMLOCK_SIM:   
               if(personalisation_sim NEQ NULL)
               {
                 if(personalisation_sim->pBody NEQ NULL)
                 {
                    MFREE(personalisation_sim->pBody);
                 }
                 MFREE(personalisation_sim);
               }
               break ; 
            case SIMLOCK_BLOCKED_NETWORK:   
              if(personalisation_bnw NEQ NULL)
              {
                 if(personalisation_bnw->pBody NEQ NULL)
                 {
                   MFREE(personalisation_bnw->pBody);
                 }
                 MFREE(personalisation_bnw);
               }
          break; 
             case SIMLOCK_FIRST_SIM: 
               if(personalisation_first_sim NEQ NULL)
               {
                  MFREE(personalisation_first_sim);
               }
               break; 
      }
   }
}



/*
  ACI extension method for retrieving the status of a single personalisation type.
  The personalisation status is stored in a separate customer managed memory area
  and the customer has to implement this extension method to return the actual status
  of the questioned personalisation type.
  */
T_SIMLOCK_STATUS aci_ext_personalisation_get_status( T_SIMLOCK_TYPE personalisation_type )
{
  TRACE_FUNCTION("aci_ext_personalisation_get_status()");
#ifdef SIM_PERS
  switch (personalisation_type)
  {
    case SIMLOCK_FIRST_SIM: /* should not occour */
    case SIMLOCK_SIM:         
      if(personalisation_sim NEQ NULL)      
        return (T_SIMLOCK_STATUS)personalisation_sim->Header.Status;
      else 
        return SIMLOCK_FAIL;
                
    case SIMLOCK_NETWORK:       
      if(personalisation_nw NEQ NULL)      
        return (T_SIMLOCK_STATUS)personalisation_nw->Header.Status;
      else 
        return SIMLOCK_FAIL;
                    
    case SIMLOCK_NETWORK_SUBSET:     
      if(personalisation_ns NEQ NULL)    
        return (T_SIMLOCK_STATUS)personalisation_ns->Header.Status;
      else 
        return SIMLOCK_FAIL;
                    
     case SIMLOCK_SERVICE_PROVIDER:  
       if(personalisation_sp NEQ NULL)     
         return (T_SIMLOCK_STATUS)personalisation_sp->Header.Status;
       else 
         return SIMLOCK_FAIL;
     
     case SIMLOCK_CORPORATE:      
       if(personalisation_cp NEQ NULL)    
         return (T_SIMLOCK_STATUS)personalisation_cp->Header.Status;
       else 
         return SIMLOCK_FAIL;

     case SIMLOCK_BLOCKED_NETWORK:		 
       if(personalisation_bnw NEQ NULL)	   
          return (T_SIMLOCK_STATUS)personalisation_bnw->Header.Status;
      else 
         return SIMLOCK_FAIL;


     default:  return SIMLOCK_FAIL; /* should not occour */
  }
#endif
}


/*
+------------------------------------------------------------------------------
|  Function    : aci_ext_personalisation_verify_password
+------------------------------------------------------------------------------
|  Description : ACI extension method for verifying the passowrd.
|f the password is false then the counter is increased. If maxcnt is reached then the key
|will been blocked. If the password is correct then the counter is set to 0
|
|  Parameters  : personalisation_type - category type (Network,Network subset, Service Provider,Corporate,SIM actegory)
|                     : passwd  - category password
|
|  Return      : T_SIMLOCK_STATUS   - Return SIMLOCK_DISABLED if compare key pass. 
|                                                     - Return SIMLOCK_FAIL if compare key fails.
|                                                     - Return SIMLOCK_BLOCKED if FC counter is greater than 
|                                                        FC Max.
+------------------------------------------------------------------------------
*/


T_SIMLOCK_STATUS aci_ext_personalisation_verify_password( T_SIMLOCK_TYPE personalisation_type, char *passwd)
{

  T_SEC_DRV_RETURN ret;
  UINT8 key_len = 0; /* For truncation check -Samir */


  TRACE_FUNCTION("aci_ext_personalisation_verify_password()");
  
  if(cfg_data->Flags & SEC_DRV_HDR_FLAG_Truncation)
  {
    key_len = TRUNCATION_KEY_LEN; 
  }
  ret = sec_cmp_KEY(personalisation_type, passwd, key_len) ;

  switch (ret)
  {
    case SEC_DRV_RET_Ok :
      return SIMLOCK_DISABLED;
   /* part of code is returning before break ,so break is not required */ 
    case SEC_DRV_RET_KeyWrong :
    case SEC_DRV_RET_KeyMismatch:
    /*Increment failure counter since input password is wrong*/
      ret = sec_FC_Increment();
      return SIMLOCK_FAIL;  
 
   /* part of code is returning before break ,so break is not required */
    default:/*ERROR!- invalid input to sec_cmp_key OR some error in sec_cmp_key*/
      return SIMLOCK_FAIL;
  }
 }


/*
+------------------------------------------------------------------------------
|  Function    : aci_ext_personalisation_change_password
+------------------------------------------------------------------------------
|  Description : ACI extension method for verifying the passowrd.
|f the password is false then the counter is increased. If maxcnt is reached then the key
|will been blocked. If the password is correct then the counter is set to 0
|
|  Parameters  : personalisation_type - category type (Network,Network subset, Service Provider,Corporate,SIM actegory)
|                     : passwd  - category password
|                     : new_passwd - New Password

|  Return      : T_SIMLOCK_STATUS   - Return OPER_SUCCESS if changes password is successfull. 
|                                                     - Return OPER_FAIL if changes password is fail.
|                                                    
+------------------------------------------------------------------------------
*/

T_OPER_RET_STATUS  aci_ext_personalisation_change_password( T_SIMLOCK_TYPE personalisation_type, char *passwd, char *new_passwd )
{
  T_SEC_DRV_RETURN   retstat;

  retstat = sec_set_KEY (personalisation_type, passwd, new_passwd, 0);
  switch(retstat)
  {
    case SEC_DRV_RET_Ok : 
          return  OPER_SUCCESS;
    case  SEC_DRV_RET_FCExeeded : 
          AciSLockShrd.blocked = TRUE; 
          return OPER_BLOCKED ; 
   default : 
         return OPER_FAIL;
  }

}
 

/*
+------------------------------------------------------------------------------
|  Function    : aci_ext_personalisation_set_status
+------------------------------------------------------------------------------
|  Description : ACI extension method for lock/unlock the category record
|
|  Parameters   : personalisation_type - category type (Network,Network subset, Service Provider,Corporate,SIM actegory)
|                     : lock   -   SIMLOCK_ENABLED to lock the category
|                                -   SIMLOCK_DISABLED to unlock the category
|                     : passwd  - category lock/unlock password
|                     
|
|  Return      : T_SIMLOCK_STATUS   - Return SIMLOCK_ENABLED if lock is successfull. 
|                                                     - Return SIMLOCK_DISABLED if unlock is successfull.
|                                                     - Return SIMLOCK_FAIL if lock or unlock fails
|                                                     - Return SIMLOCK_BLOCKED if FC exeeded for wrong password
|                                                     - Return SIMLOCK_FAIL if lock/unlock fail
+------------------------------------------------------------------------------
*/
 
T_SIMLOCK_STATUS aci_ext_personalisation_set_status( T_SIMLOCK_TYPE personalisation_type,
  T_SIMLOCK_STATUS lock, char* passwd)
{

/* #ifdef SIM_PERS */
  int rec_num = personalisation_type;
  int key_len = 0;
  T_SEC_DRV_RETURN ret;

  TRACE_FUNCTION("aci_ext_personalisation_set_status()");

  if(lock EQ SIMLOCK_ENABLED)
  {
    ret=sec_rec_Lock(rec_num, passwd, key_len,0xffff/*depend mask*/);/*aci_ext_personalisation_verify_password(personalisation_type, passwd);*/
    if (ret EQ SEC_DRV_RET_Ok){
      return SIMLOCK_ENABLED;
    }
    else{
      return SIMLOCK_FAIL;
    }
  }

  if(lock EQ SIMLOCK_DISABLED)
  {
     if(cfg_data->Flags & SEC_DRV_HDR_FLAG_Truncation)
     {
        key_len = TRUNCATION_KEY_LEN; 
     }
     ret = sec_rec_Unlock(rec_num,TEMPORARY_UNLOCK,passwd,key_len,0xffff/*depend mask*/);

    switch(ret)
    {
      case  SEC_DRV_RET_Ok :
         return SIMLOCK_DISABLED;  
      case SEC_DRV_RET_FCExceeded :
         return SIMLOCK_BLOCKED;
      case SEC_DRV_RET_KeyMismatch :
      case SEC_DRV_RET_KeyWrong :
      case SEC_DRV_RET_NotPresent :
      case SEC_DRV_RET_Unknown :
        return SIMLOCK_FAIL;
    }
  } 
  return SIMLOCK_FAIL;
  /*#endif */
}




/*#ifdef SIM_PERS */

/*
+------------------------------------------------------------------------------
|  Function    : aci_ext_slock_reset_fc
+------------------------------------------------------------------------------
|  Description : For Failure Counter reset. Uses Security Drv. method to reset FC
|
|  Parameters   : fckey -  Password for resetting FC counter
|                     
|
|  Return      : T_OPER_RET_STATUS   - Return OPER_SUCCESS if FC reset is successfull. 
|                                                     - Return OPER_FAIL if FC reset is fail
|                                                    
+------------------------------------------------------------------------------
*/


T_OPER_RET_STATUS  aci_ext_slock_reset_fc(char *fckey)
{
  T_SEC_DRV_RETURN retstat;
  TRACE_FUNCTION("aci_ext_slock_reset_fc ()");

  retstat= sec_FC_Reset(fckey,0);

  switch(retstat)
   {
     case SEC_DRV_RET_Ok :
           return OPER_SUCCESS;
     case SEC_DRV_RET_FCExceeded:
     case SEC_DRV_RET_KeyWrong:
     case SEC_DRV_RET_KeyMismatch: 
           return OPER_WRONG_PASSWORD;
    default :
           return OPER_FAIL;
   }

}



/*
+------------------------------------------------------------------------------
|  Function    : aci_ext_slock_sup_info
+------------------------------------------------------------------------------
|  Description : For Supplementary Info( e.g. FC MAX, FC Current value).
|
|  Parameters   : sup_info -which has the element info type and Data value.  
|                     
|
|  Return      : T_OPER_RET_STATUS   - Return OPER_SUCCESS if successfully read the Info type data
|                                                         value from security driver. Info type data value is filled in this 
|                                                         Function
|                                                     - Return OPER_FAIL if unable to read Info type data value from 
|                                                        security driverl
|                                                    
+------------------------------------------------------------------------------
*/


T_OPER_RET_STATUS  aci_ext_slock_sup_info (T_SUP_INFO *sup_info)
{
  T_SEC_DRV_CONFIGURATION *cfg_data;

  TRACE_FUNCTION("aci_ext_slock_sup_info ()");
  if ((sec_get_CFG(&cfg_data)) NEQ SEC_DRV_RET_Ok)
  return OPER_FAIL;
  else 
  {
     switch(sup_info->infoType )
     {
        case(FCMAX ): sup_info->datavalue = cfg_data->FC_Max;     break;
        case(FCATTEMPTSLEFT): sup_info->datavalue = ((cfg_data->FC_Max) - (cfg_data->FC_Current));  break; 
        case(FCRESETFAILMAX ): sup_info->datavalue = cfg_data->FC_Reset_Fail_Max ;   break;
        case(FCRESETFAILATTEMPTSLEFT): sup_info->datavalue = ((cfg_data->FC_Reset_Fail_Max) - (cfg_data->FC_Reset_Fail_Current));  break;   
        case(FCRESETSUCCESSMAX ): sup_info->datavalue = cfg_data->FC_Reset_Success_Max ;   break;
        case(FCRESETSUCCESSATTEMPTSLEFT): sup_info->datavalue = ((cfg_data->FC_Reset_Success_Max) - (cfg_data->FC_Reset_Success_Current));  break; 
        case (TIMERFLAG): sup_info->datavalue =(cfg_data->Flags & SEC_DRV_HDR_FLAG_Unlock_Timer)?(1):(0) ;break; 
        case (ETSIFLAG) :  sup_info->datavalue =(cfg_data->Flags & SEC_DRV_HDR_FLAG_ETSI_Flag)?(1):(0) ;   break;
        case (AIRTELINDFLAG) :  sup_info->datavalue =(cfg_data->Flags & SEC_DRV_HDR_FLAG_Airtel_Ind)?(1):(0) ; break;
     }  /*end of switch*/

     MFREE(cfg_data);/*  deallocate configuration data allocated from Security Driver;    */  
     return OPER_SUCCESS;    
  }
}


/*
+------------------------------------------------------------------------------
|  Function    : aci_ext_auto_personalise
+------------------------------------------------------------------------------
|  Description : It Auto personalise the sim. If Add new IMSI flag set and If Code is not present
|                      then adds the code in the MEPD data  and lock the category ( Network, Sub Network,
|                      sim )
|  Parameters : void  
|                     
|
|  Return        :    void
|                                                    
+------------------------------------------------------------------------------
*/

T_AUTOLOCK_STATUS aci_ext_auto_personalise(T_SIMLOCK_TYPE   current_lock)
{
  UBYTE imsi_sim[MAX_IMSI_LEN+1];
  T_SIMLOCK_TYPE type;
  static UINT16 flag  = 0xffff;
  UINT16 dependMask = 0xffff;
 
  TRACE_FUNCTION("aci_ext_auto_personalise()");
 
  psaSIM_decodeIMSI(simShrdPrm.imsi.field, simShrdPrm.imsi.c_field, (char *)imsi_sim);
 
  if((AciSLockShrd.dependency[SIMLOCK_FIRST_SIM] & NW_AUTO_LOCK)AND (flag & NW_AUTO_LOCK))
  {
     type = SIMLOCK_NETWORK;
     sim_code_present_in_me = FALSE;
     aci_slock_check_NWlock(imsi_sim,1);
     flag &= ~(NW_AUTO_LOCK);

    if(sim_code_present_in_me EQ FALSE)
    {
      aci_ext_add_code(type);
    }
  }

  if((AciSLockShrd.dependency[SIMLOCK_FIRST_SIM] & NS_AUTO_LOCK)AND (flag & NS_AUTO_LOCK))
  {
     type = SIMLOCK_NETWORK_SUBSET;
     flag &= ~(NS_AUTO_LOCK);
     sim_code_present_in_me = FALSE;
     aci_slock_check_NSlock(imsi_sim,1);
     if(sim_code_present_in_me EQ FALSE)
     {
       aci_ext_add_code(type);
     }
  }

  if((AciSLockShrd.dependency[SIMLOCK_FIRST_SIM] & SIM_AUTO_LOCK ) AND (flag & SIM_AUTO_LOCK))
  {
    flag &= ~(SIM_AUTO_LOCK);
    type = SIMLOCK_SIM;
    sim_code_present_in_me = FALSE;
    aci_slock_check_SMlock(imsi_sim,1);
    if(sim_code_present_in_me EQ FALSE)
    {
      aci_ext_add_code(type);
    }
  }

 if((AciSLockShrd.dependency[SIMLOCK_FIRST_SIM] & SP_AUTO_LOCK) AND (flag & SP_AUTO_LOCK))
 {
  if (psaSIM_ChkSIMSrvSup(SRV_GrpLvl1))
  {
     if(aci_slock_sim_config.sim_read_gid1 EQ FALSE)
     {
      aci_slock_sim_read_sim(SIM_GID1, NOT_PRESENT_8BIT, MAX_GID);
      return AUTOLOCK_EXCT ;
     }
     else 
      {       
       type = SIMLOCK_SERVICE_PROVIDER;
       sim_code_present_in_me = FALSE;
       aci_slock_check_SPlock(imsi_sim,1);
       TRACE_FUNCTION_P1("SP sim_code_present_in_me = %d", sim_code_present_in_me);
       switch(sim_code_present_in_me)
        {
         case FALSE : 
               aci_ext_add_code(type);
               break;
         case CHECK_FAIL :
              dependMask &= ~(SP_AUTO_LOCK) ;
              break;
         case TRUE : 
              break;
         }
         flag &= ~(SP_AUTO_LOCK);
      
     }
  }
  else 
  {
     flag &= ~(SP_AUTO_LOCK);
     dependMask &= ~(SP_AUTO_LOCK) ;
  }
 } 


if((AciSLockShrd.dependency[SIMLOCK_FIRST_SIM] & CP_AUTO_LOCK)AND (flag & CP_AUTO_LOCK)) 
 {

   if (psaSIM_ChkSIMSrvSup(SRV_GrpLvl1))
     {
        if(aci_slock_sim_config.sim_read_gid1 EQ FALSE)
        {
         aci_slock_sim_read_sim(SIM_GID1, NOT_PRESENT_8BIT, MAX_GID);
         return AUTOLOCK_EXCT ;
        }
     }

  if (psaSIM_ChkSIMSrvSup(SRV_GrpLvl2))
     {
        if(aci_slock_sim_config.sim_read_gid2 EQ FALSE)
        {
         aci_slock_sim_read_sim(SIM_GID2, NOT_PRESENT_8BIT, MAX_GID);
         return AUTOLOCK_EXCT ;
        }
     }
   
 if(psaSIM_ChkSIMSrvSup(SRV_GrpLvl1) AND psaSIM_ChkSIMSrvSup(SRV_GrpLvl2))
 {
   type = SIMLOCK_CORPORATE;
   sim_code_present_in_me = FALSE;
   aci_slock_check_CPlock(imsi_sim,1);
   switch(sim_code_present_in_me)
    {
      case FALSE : 
           aci_ext_add_code(type);
           break;
      case CHECK_FAIL :
           dependMask &= ~(CP_AUTO_LOCK) ;
           break;
       case TRUE : 
            break;
    }
   flag &= ~(CP_AUTO_LOCK);
 }
  else 
  {
   flag &= ~(CP_AUTO_LOCK);
   dependMask &= ~(CP_AUTO_LOCK) ;
  }
  
}


 sec_rec_Lock(SIMLOCK_FIRST_SIM,NULL,0,dependMask); 
 return AUTOLOCK_CMPL ;
  
}




/*
+------------------------------------------------------------------------------
|  Function    : aci_ext_add_code
+------------------------------------------------------------------------------
|  Description : It append the category code in MEPD. It foolows FIFO procedure while adding the code
|  Parameters : type -- category type (Network,Network subset, Service Provider,Corporate,SIM actegory)  
|  Return        :  void
|                                                    
+------------------------------------------------------------------------------
*/


void aci_ext_add_code(T_SIMLOCK_TYPE type)
{
  UINT16 index =0;
  UBYTE max_num_user_codes;
  UBYTE num_user_codes;
  UBYTE curr_user_code_index;
  UBYTE  imsi_field[20];

  TRACE_FUNCTION("aci_ext_add_code()");
  switch (type)
  {
    case SIMLOCK_NETWORK :
      max_num_user_codes =((UBYTE *) personalisation_nw->pBody)[index];
      index += MAX_NUM_USERCODE_SIZE;
      index += NUM_OPCODE_SIZE ;
      index += OPCODE_LEN_SIZE + ((UBYTE *) personalisation_nw->pBody)[OPCODE_LEN_INDEX];

      num_user_codes = ((UBYTE *)personalisation_nw->pBody)[index];
      index +=NUM_USER_CODE_SIZE;
      curr_user_code_index = ((UBYTE *)personalisation_nw->pBody)[index];
      index += CURR_USER_CODE_INDEX_SIZE ;
    
      memcpy(imsi_field,simShrdPrm.imsi.field,simShrdPrm.imsi.c_field);
      if(simShrdPrm.mnc_len EQ 3)
        imsi_field[NW_CODE_LEN-1] |= 0xf0;  
      else 
        imsi_field[NW_CODE_LEN-1] |= 0xff;
    
      /*
       * If no user code is present, curr_user_code_index will be ff. After that, curr_user_code_index starts from 0
       */
      if((curr_user_code_index EQ 0xff) || (curr_user_code_index EQ max_num_user_codes -1)) 
        curr_user_code_index = 0;  
      else 
        curr_user_code_index++;

      if(num_user_codes < max_num_user_codes)
        num_user_codes++;
    
      memcpy(&((UBYTE *)personalisation_nw->pBody)[index + curr_user_code_index*NW_CODE_LEN/*multiplication added*/],imsi_field,NW_CODE_LEN);
      ((UBYTE *)personalisation_nw->pBody)[index - CURR_USER_CODE_INDEX_SIZE] = curr_user_code_index;
      ((UBYTE *)personalisation_nw->pBody)[index - CURR_USER_CODE_INDEX_SIZE - NUM_USER_CODE_SIZE] = num_user_codes;
      sec_set_REC(type, personalisation_nw);
    break;

    
    case SIMLOCK_NETWORK_SUBSET :
      max_num_user_codes = ((UBYTE *)personalisation_ns->pBody)[index];
      index += MAX_NUM_USERCODE_SIZE;
      index += NUM_OPCODE_SIZE ;
      index += OPCODE_LEN_SIZE + ((UBYTE *) personalisation_ns->pBody)[OPCODE_LEN_INDEX];
  
      num_user_codes = ((UBYTE *) personalisation_ns->pBody)[index];
      index +=NUM_USER_CODE_SIZE;
      curr_user_code_index = ((UBYTE *)personalisation_ns->pBody)[index];
      index += CURR_USER_CODE_INDEX_SIZE ;
  
      memcpy(imsi_field,simShrdPrm.imsi.field,simShrdPrm.imsi.c_field);
      if(simShrdPrm.mnc_len EQ 3)
        imsi_field[NW_NS_CODE_LEN-1] |= 0xf0;  /* prateek: unused nibbles to be '00' and not 'FF' to avoid problems during decodeIMSI */
      else 
        imsi_field[NW_NS_CODE_LEN-1] |= 0xff;
    
      /*
       * If no user code is present, curr_user_code_index will be ff. After that, curr_user_code_index starts from 0
       */
      if((curr_user_code_index EQ 0xff) || (curr_user_code_index EQ max_num_user_codes -1)) 
        curr_user_code_index = 0;  
      else 
        curr_user_code_index++;
  
      if(num_user_codes < max_num_user_codes)
        num_user_codes++;
    
      memcpy(&((UBYTE *)personalisation_ns->pBody)[index + curr_user_code_index*NW_NS_CODE_LEN/*multiplication added*/],imsi_field,NW_NS_CODE_LEN);
      ((UBYTE *)personalisation_ns->pBody)[index - CURR_USER_CODE_INDEX_SIZE] = curr_user_code_index;
      ((UBYTE *)personalisation_ns->pBody)[index - CURR_USER_CODE_INDEX_SIZE - NUM_USER_CODE_SIZE] = num_user_codes;
      sec_set_REC(type, personalisation_ns);
    break;
    
    case SIMLOCK_SERVICE_PROVIDER :
      if(!psaSIM_ChkSIMSrvSup(SRV_GrpLvl1) OR (aci_slock_sim_config.sim_gidl1[0] EQ 0xff))
      {
        TRACE_FUNCTION("not adding sp as no service or maybe gid1 is ff");
        break;
      }
        
      max_num_user_codes =((UBYTE *) personalisation_sp->pBody)[index];
      index += MAX_NUM_USERCODE_SIZE;
      index += NUM_OPCODE_SIZE ;
      index += OPCODE_LEN_SIZE +  ((UBYTE *)personalisation_sp->pBody)[OPCODE_LEN_INDEX];
  
      num_user_codes = ((UBYTE *)personalisation_sp->pBody)[index];
      index +=NUM_USER_CODE_SIZE;
      curr_user_code_index = ((UBYTE *)personalisation_sp->pBody)[index];
      index += CURR_USER_CODE_INDEX_SIZE ;
      
      memcpy(imsi_field,simShrdPrm.imsi.field,simShrdPrm.imsi.c_field);
      if(simShrdPrm.mnc_len EQ 3)
        imsi_field[NW_CODE_LEN-1] |= 0xf0;  /* prateek: unused nibbles to be '00' and not 'FF' to avoid problems during decodeIMSI */
      else 
        imsi_field[NW_CODE_LEN-1] |= 0xff;
    
      if (psaSIM_ChkSIMSrvSup(SRV_GrpLvl1))
      {
        memcpy(imsi_field+NW_CODE_LEN,aci_slock_sim_config.sim_gidl1,GID1_LEN);
        /*
          * If no user code is present, curr_user_code_index will be ff. After that, curr_user_code_index starts from 0
              */
        if((curr_user_code_index EQ 0xff) || (curr_user_code_index EQ max_num_user_codes -1)) 
          curr_user_code_index = 0;  
        else 
          curr_user_code_index++;
  
        if(num_user_codes < max_num_user_codes)
          num_user_codes++;
      
        memcpy(&((UBYTE *)personalisation_sp->pBody)[index + curr_user_code_index*(NW_CODE_LEN+GID1_LEN)],imsi_field,NW_CODE_LEN+GID1_LEN);
        ((UBYTE *)personalisation_sp->pBody)[index - CURR_USER_CODE_INDEX_SIZE] = curr_user_code_index;
        ((UBYTE *)personalisation_sp->pBody)[index - CURR_USER_CODE_INDEX_SIZE - NUM_USER_CODE_SIZE] = num_user_codes;
        sec_set_REC(type, personalisation_sp);
       }
     break;
    
    case SIMLOCK_CORPORATE :
      if( !psaSIM_ChkSIMSrvSup(SRV_GrpLvl1) OR !psaSIM_ChkSIMSrvSup(SRV_GrpLvl2) OR (aci_slock_sim_config.sim_gidl1[0] EQ 0xff) OR (aci_slock_sim_config.sim_gidl2[0] EQ 0xff))
      {
        TRACE_FUNCTION("not adding cp as no service or maybe gid1/gid2 is ff");
        break;
      }
      
      max_num_user_codes = ((UBYTE *)personalisation_cp->pBody)[index];
      index += MAX_NUM_USERCODE_SIZE;
      index += NUM_OPCODE_SIZE ;
      index += OPCODE_LEN_SIZE +  ((UBYTE *)personalisation_cp->pBody)[OPCODE_LEN_INDEX];
    
      num_user_codes = ((UBYTE *)personalisation_cp->pBody)[index];
      index +=NUM_USER_CODE_SIZE;
      curr_user_code_index =((UBYTE *) personalisation_cp->pBody)[index];
      index += CURR_USER_CODE_INDEX_SIZE ;
      
      memcpy(imsi_field,simShrdPrm.imsi.field,simShrdPrm.imsi.c_field);
      if(simShrdPrm.mnc_len EQ 3)
        imsi_field[NW_CODE_LEN-1] |= 0xf0;  /* prateek: unused nibbles to be '00' and not 'FF' to avoid problems during decodeIMSI */
      else
        /* if(curr_user_code_index NEQ 0) */
        imsi_field[NW_CODE_LEN-1] |= 0xff;
    
      if (psaSIM_ChkSIMSrvSup(SRV_GrpLvl1))
      {
        memcpy(imsi_field+NW_CODE_LEN,aci_slock_sim_config.sim_gidl1,GID1_LEN);
        if (psaSIM_ChkSIMSrvSup(SRV_GrpLvl2))
        {
          memcpy(imsi_field+NW_CODE_LEN+GID1_LEN,aci_slock_sim_config.sim_gidl2,GID2_LEN);
          /*
          * If no user code is present, curr_user_code_index will be ff. After that, curr_user_code_index starts from 0
          */
          if((curr_user_code_index EQ 0xff) || (curr_user_code_index EQ max_num_user_codes -1)) 
            curr_user_code_index = 0;  
          else 
            curr_user_code_index++;
      
          if(num_user_codes < max_num_user_codes)
            num_user_codes++;
      
          memcpy(&((UBYTE *)personalisation_cp->pBody)[index + (curr_user_code_index*(NW_CODE_LEN+GID1_LEN+GID2_LEN))],imsi_field,NW_CODE_LEN+GID1_LEN+GID2_LEN);
          ((UBYTE *)personalisation_cp->pBody)[index - CURR_USER_CODE_INDEX_SIZE] = curr_user_code_index;
          ((UBYTE *)personalisation_cp->pBody)[index - CURR_USER_CODE_INDEX_SIZE - NUM_USER_CODE_SIZE] = num_user_codes;
          sec_set_REC(type, personalisation_cp);
        }
      }
    break;
    
    case SIMLOCK_SIM :
      max_num_user_codes = ((UBYTE *)personalisation_sim->pBody)[index];
      index += MAX_NUM_USERCODE_SIZE;
      index += NUM_OPCODE_SIZE ;
      index += OPCODE_LEN_SIZE +  ((UBYTE *)personalisation_sim->pBody)[OPCODE_LEN_INDEX];
  
      num_user_codes = ((UBYTE *)personalisation_sim->pBody)[index];
      index +=NUM_USER_CODE_SIZE;
      curr_user_code_index =((UBYTE *) personalisation_sim->pBody)[index];    
      index += CURR_USER_CODE_INDEX_SIZE ;
  
      memcpy(imsi_field,simShrdPrm.imsi.field,simShrdPrm.imsi.c_field);

      /*
       * If no user code is present, curr_user_code_index will be ff. After that, curr_user_code_index starts from 0
       */
      if((curr_user_code_index EQ 0xff) || (curr_user_code_index EQ max_num_user_codes -1)) 
        curr_user_code_index = 0;  
      else 
        curr_user_code_index++;
  
      if(num_user_codes < max_num_user_codes)
        num_user_codes++;
    
      memcpy(&((UBYTE *)personalisation_sim->pBody)[index + (curr_user_code_index*NW_NS_MSIN_CODE_LEN)],imsi_field,NW_NS_MSIN_CODE_LEN);
      ((UBYTE *)personalisation_sim->pBody)[index - CURR_USER_CODE_INDEX_SIZE] = curr_user_code_index;
      ((UBYTE *)personalisation_sim->pBody)[index - CURR_USER_CODE_INDEX_SIZE - NUM_USER_CODE_SIZE] = num_user_codes;
      sec_set_REC(type, personalisation_sim);
    break;
  }
}

/*
+------------------------------------------------------------------------------
|  Function    : aci_ext_slock_master_unlock
+------------------------------------------------------------------------------
|  Description   : 
|  Parameters    : masterkey
|  Return        : T_OPER_RET_STATUS
|                                                    
+------------------------------------------------------------------------------
*/
T_OPER_RET_STATUS aci_ext_slock_master_unlock(char *masterkey)
{
    T_SEC_DRV_RETURN ret;
    ret = sec_master_Unlock( masterkey,0,0xffff );
    switch (ret)
    {
      case SEC_DRV_RET_Ok :
        return( OPER_SUCCESS );
      case SEC_DRV_RET_Unknown :
        return( OPER_FAIL );
      case SEC_DRV_RET_KeyMismatch  :
      case SEC_DRV_RET_KeyWrong :
        return( OPER_WRONG_PASSWORD );
      case SEC_DRV_RET_NotPresent :
        return( OPER_NOT_ALLOWED ); 
    }
    return( OPER_FAIL );
}

/*
+------------------------------------------------------------------------------
|  Function    : aci_ext_check_timer
+------------------------------------------------------------------------------
|  Description   : Check the timer status,is active or not 
|  Parameters    : None
|  Return        : Status of timer
|                                                    
+------------------------------------------------------------------------------
*/
UBYTE aci_ext_check_timer()
{ 
    T_SEC_DRV_RETURN ret;
    UBYTE  pStatus;
    ret = sec_get_timer(&pStatus);
    switch(ret)
    { 
     case  SEC_DRV_RET_Ok : 
        return(pStatus);
     case SEC_DRV_RET_Unknown : 
        return(TIMER_RUNNING);
     case SEC_DRV_RET_NotPresent :
        return(TIMER_STOPPED);
    }
    return(TIMER_STOPPED);
}

/*
+------------------------------------------------------------------------------
|  Function    : aci_ext_set_timer_flag
+------------------------------------------------------------------------------
|  Description   : check the timer status
|  Parameters    : status
|  Return        : None
|                                                    
+------------------------------------------------------------------------------
*/
void aci_ext_set_timer_flag(UBYTE status)
{
    sec_set_timer(status);
}

/*
+------------------------------------------------------------------------------
|  Function    : aci_ext_is_timer_support
+------------------------------------------------------------------------------
|  Description   : Check wheather the timer support or not
|  Parameters    : None
|  Return        : Timer status
|                                                    
+------------------------------------------------------------------------------
*/

UBYTE aci_ext_is_timer_support()
{
 
  if(cfg_data->Flags & SEC_DRV_HDR_FLAG_Unlock_Timer)
  {
      return TRUE; 
  }
  else
  {
      return FALSE; 
  }
}

#endif /* SIM_PERS */



/*
  PURPOSE : convert bcd to ASCII

*/


LOCAL void decodeB2A (U8 *bcd, U8 bcd_len, U8 *ascii)
{
  int i;

  TRACE_FUNCTION("decodeB2A()");

  for (i=0; i<bcd_len*2; i++)
  {
    ascii[i] = (i & 1) ? (bcd[i/2]>>4)+'0' : (bcd[i/2]&0x0f)+'0';
    if (ascii[i]>'9')
      break;  /* stop if invalid digit */
  }
  ascii[i] = 0;
}

LOCAL void encodeA2B (char *ascii, UBYTE *bcd, UBYTE bcd_len)
{
  UBYTE i;
  UBYTE digit;

  TRACE_FUNCTION("encodeA2B()");

  for (i=0; i<bcd_len*2; i++)                 /* fill the whole field, pad remaining with 0xf */
  {
    if (i<strlen(ascii))
    {
      digit = ascii[i];
      if (digit >= '0' OR digit <= '9')
      {
        digit-= '0';
      }
      else
      {
        TRACE_EVENT_P1("[WRN] invalid digit in PIN \"%d\", skipping!", digit);
        digit = 0x0f;
      }
    }
    else
    {
      digit = 0x0f;  /* unused nibbles are set to 'F'. */
    }

    if ((i & 1) EQ 0)
    {
      /* process PIN digit 1,3,5,... at i=0,2,4,...*/
      bcd[i/2]  = digit;
    }
    else
    {
      /* process PIN digit 2,4,6,... at i=1,3,5,...*/
      bcd[i/2] |= digit << 4;
    }
  }
}

/* That is a workaround to buld simulation test cases */
#ifdef _SIMULATION_ 
  #define effs_t int
  #define EFFS_OK 0
  #define FFS_fread(a,  b, c) 0
  #define FFS_fwrite(a,  b, c) 0
#endif /*_SIMULATION_*/ 

#define FFS_MMILOCK_STATUS "/gsm/MELOCK/SecCode"

#ifdef TI_PS_FF_AT_P_CMD_SECS
/*
+------------------------------------------------------------------------------
|  Function    : aci_ext_personalisation_CS_get_status
+------------------------------------------------------------------------------
|  Description : Reads value of CS lock from file and returns it
|  Parameters : void  
|                     
|
|  Return        :    status of CS lock
|  Remark:     :  It is proposed to implement through security driver
|                                                    
+------------------------------------------------------------------------------
*/
T_SIMLOCK_STATUS aci_ext_personalisation_CS_get_status()
{
  effs_t ffs_rslt;
  TRACE_FUNCTION("aci_ext_personalisation_CS_get_status()");

  ffs_rslt = FFS_fread(FFS_MMILOCK_STATUS,  &MMI_personalisation_status, sizeof(MMI_personalisation_status));
  if(ffs_rslt < EFFS_OK)   /* error */
  {
    TRACE_EVENT_P1("unable to read %s", FFS_MMILOCK_STATUS);
    return SIMLOCK_FAIL;
  }
  return (T_SIMLOCK_STATUS) MMI_personalisation_status.State;
}


/*
+------------------------------------------------------------------------------
|  Function    : aci_ext_personalisation_CS_set_status
+------------------------------------------------------------------------------
|  Description : Reads value of CS lock from file compare passwords and if correct, sets CS lock status
|  Parameters : lock - enable or disable
|                      passwd - password to change lock
|                     
|
|  Return        :  status of operation
|  Remark:     :  It is proposed to implement through security driver
|                                                    
+------------------------------------------------------------------------------
*/
T_SIMLOCK_STATUS aci_ext_personalisation_CS_set_status(T_SIMLOCK_STATUS lock , char * passwd)
{
  effs_t ffs_rslt;
  U8 exp_pin[6+1];   /* +1 for '\0' */    
  T_SIMLOCK_STATUS result;
  TRACE_FUNCTION("aci_ext_personalisation_CS_set_status()");    

  /* Read data from FFS */ 

  ffs_rslt = FFS_fread(FFS_MMILOCK_STATUS,  &MMI_personalisation_status, sizeof(MMI_personalisation_status));

  if(ffs_rslt < EFFS_OK)   /* error */
  {
    TRACE_EVENT_P1("unable to read %s", FFS_MMILOCK_STATUS);
    return SIMLOCK_FAIL;
  }
 
  decodeB2A(MMI_personalisation_status.Cur_code, 3, exp_pin);
  
  if (strncmp((char *)exp_pin, (char *)passwd, 3))  
  {    
     /* Block personalisation lock, if tried too often! */
     MMI_personalisation_status.cnt++;   
     if (MMI_personalisation_status.cnt >= MMI_personalisation_status.maxcnt)    
     {      
        result = SIMLOCK_BLOCKED;
     }    
     else
     {
        result = SIMLOCK_LOCKED;
     }
   }  
   else 
   {    
      /* correct pin passed */
     MMI_personalisation_status.cnt = 0; 
     MMI_personalisation_status.State = lock; 
     result = lock;
    }
    ffs_rslt = FFS_fwrite(FFS_MMILOCK_STATUS,  &MMI_personalisation_status, sizeof(MMI_personalisation_status));

    if (ffs_rslt < EFFS_OK)
    {
      TRACE_EVENT_P1("unable to write %s", FFS_MMILOCK_STATUS);
      return SIMLOCK_FAIL;
    }
    return result;
}
#endif /* TI_PS_FF_AT_P_CMD_SECP */
#ifdef TI_PS_FF_AT_P_CMD_SECP
/*
+------------------------------------------------------------------------------
|  Function    : aci_ext_personalisation_CS_change_password
+------------------------------------------------------------------------------
|  Description : Reads value of CS lock from file compare passwords and if correct, sets new password
|  Parameters : passwd - old password
|                      new_passwd - new password
|                     
|
|  Return        :  status of operation
|  Remark:     :  It is proposed to implement through security driver
|                                                    
+------------------------------------------------------------------------------
*/
T_SIMLOCK_STATUS aci_ext_personalisation_CS_change_password( char *passwd, char *new_passwd )
{
  effs_t ffs_rslt;
  U8 exp_pin[6+1];   /* +1 for '\0' */    
  T_SIMLOCK_STATUS result;

  TRACE_FUNCTION("aci_ext_personalisation_CS_change_password()");

  ffs_rslt = FFS_fread(FFS_MMILOCK_STATUS,  &MMI_personalisation_status, sizeof(MMI_personalisation_status));
  if(ffs_rslt < EFFS_OK)   /* error */
  {
    TRACE_EVENT_P1("unable to read %s", FFS_MMILOCK_STATUS);  
    return SIMLOCK_FAIL;
  }


  decodeB2A(MMI_personalisation_status.Cur_code, 3, exp_pin);

  if (strncmp((char *)exp_pin, (char *)passwd, 3))
  {
    /* Block personalisation lock, if tried too often! */
    MMI_personalisation_status.cnt++;
    if (MMI_personalisation_status.cnt >= MMI_personalisation_status.maxcnt)
     {
        result = SIMLOCK_BLOCKED;
     }
     else
     {
        result = SIMLOCK_LOCKED;
     }
  }
  else
  {
    /* correct pin passed */
    MMI_personalisation_status.cnt=0;
    if ((new_passwd NEQ NULL) AND (strlen(new_passwd) > 0))
    {
      encodeA2B(new_passwd, MMI_personalisation_status.Cur_code, 3);
    }
    result = (T_SIMLOCK_STATUS) MMI_personalisation_status.State;  
  }
   ffs_rslt = FFS_fwrite(FFS_MMILOCK_STATUS,  &MMI_personalisation_status, sizeof(MMI_personalisation_status));

  if (ffs_rslt < EFFS_OK)
  {
    TRACE_EVENT_P1("unable to write %s", FFS_MMILOCK_STATUS);
    return SIMLOCK_FAIL;
  }
  return result;
}
#endif /* TI_PS_FF_AT_P_CMD_SECP */

#ifdef FF_PHONE_LOCK

/*
+------------------------------------------------------------------------------
|  Function    : aci_ext_set_phone_lock_satus
+------------------------------------------------------------------------------
|  Description : Sets the Phone Lock status value
|  Parameters : type - Phone Lock or Auto Phone Lock
|                      status - Enable or Disable
|                      passwd - Unlock passwd
|                    
|
|  Return        :  status of operation OPER_SUCCESS or OPER_WRONG_PASSWORD
|  |                                                    
+------------------------------------------------------------------------------
*/
T_OPER_RET_STATUS aci_ext_set_phone_lock_satus(T_SEC_DRV_EXT_PHONE_LOCK_TYPE type,T_SEC_DRV_EXT_PHONE_LOCK_STATUS status,const char *passwd)
 {
     T_SEC_DRV_RETURN results; 

     results = sec_set_phone_lock_status(type,status,passwd,0); 
     if(results EQ SEC_DRV_RET_Ok)
       return(OPER_SUCCESS); 
     else if(results EQ SEC_DRV_RET_KeyMismatch)
       return(OPER_WRONG_PASSWORD); 
     else 
       return(OPER_NOT_ALLOWED); 
 }

/*
+------------------------------------------------------------------------------
|  Function    : aci_ext_get_phone_lock_satus
+------------------------------------------------------------------------------
|  Description : Gets the Phone Lock status value
|  Parameters : type - Phone Lock or Auto Phone Lock
|                      status - Address of the Status.
|                    
|
|  Return        :  Plone Lock or Auto Lock  status value is set in the  address 
|  |                                                    
+------------------------------------------------------------------------------
*/
T_OPER_RET_STATUS aci_ext_get_phone_lock_satus(T_SEC_DRV_EXT_PHONE_LOCK_TYPE type,T_SEC_DRV_EXT_PHONE_LOCK_STATUS *status)
 {
    T_SEC_DRV_RETURN results; 
	
    results = sec_get_phone_lock_status(type,status);
    if(results EQ SEC_DRV_RET_Ok)
       return(OPER_SUCCESS); 
   else 
       return(OPER_NOT_ALLOWED); 
 }


/*
+------------------------------------------------------------------------------
|  Function    : aci_ext_set_phone_lock_key
+------------------------------------------------------------------------------
|  Description : Sets the Phone Lock key value
|  Parameters : pOldKey - Old key
|                      pNewKey - New Key.
|                    
|
|  Return        :  OPER_SUCCESS if change key is success
|                       OPER_WRONG_PASSWORD if wrong old key 
|  |                                                    
+------------------------------------------------------------------------------
*/

T_OPER_RET_STATUS aci_ext_set_phone_lock_key(const char *pOldKey,const char *pNewKey)
{
  T_SEC_DRV_RETURN results;

  results = sec_set_phone_lock_key(pOldKey, pNewKey, 0); 
 if(results EQ SEC_DRV_RET_Ok)
       return(OPER_SUCCESS); 
     else if((results EQ SEC_DRV_RET_KeyMismatch) OR (results EQ SEC_DRV_RET_KeyWrong))
       return(OPER_WRONG_PASSWORD); 
     else 
       return(OPER_NOT_ALLOWED);
  
}
#endif

