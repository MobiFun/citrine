/* 
+----------------------------------------------------------------------------- 
|  Project :  
|  Modul   :  J:\g23m-aci\aci_ext\aci_ext_pers.h
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
#ifndef ACI_EXT_PERS_H
#define ACI_EXT_PERS_H

/* ACI extension definitions */
#define PERSONALISATION_MAX 15  /* maximal amount of personalisation variants per personalisation check */

#ifdef SIM_PERS
enum E_AUTO_DEPENDECY_FLAG
{
  NW_AUTO_LOCK = 0x01,
  NS_AUTO_LOCK  = 0x02,
  SP_AUTO_LOCK  = 0x04,
  CP_AUTO_LOCK  = 0x08,
  SIM_AUTO_LOCK= 0x10
};
#endif
/* Globals ------------------------------------------------------------------------------ */


typedef enum
{
  MMILOCK_FAIL = -1,
  MMILOCK_DISABLED,      /* No lock check to be done */
  MMILOCK_ENABLED,       /* A lock check to be executed*/
  MMILOCK_BLOCKED,       /* MMI lock failed */
  MMILOCK_VERIFIED       /* MMI lock verified */
} T_MMILOCK_STATUS;

typedef enum
{
  SIMLOCK_FAIL = -1,
  SIMLOCK_DISABLED,     /* No SIM lock check has to be done */
  SIMLOCK_PERM_DISABLED,
  SIMLOCK_ENABLED,      /* A SIM lock check has to be executed*/
  SIMLOCK_BLOCKED,      /* The SIM is blocked, i.e. because of a (or to many) wrong PIN(s) */
  SIMLOCK_LOCKED,        /* The ME is locked because of wrong SIM */
  SIMLOCK_WAIT,          /* Wait to read  EF gid1 and gid2 */
  SIMLOCK_BUSY
} T_SIMLOCK_STATUS;

typedef enum
{
  AUTOLOCK_FAIL = -1,
  AUTOLOCK_CMPL,     /* No SIM lock check has to be done */
  AUTOLOCK_EXCT
 } T_AUTOLOCK_STATUS;

typedef enum 
{
  SIMLOCK_CHECK_LOCK =0,
  SIMLOCK_CHECK_PERS,
  SIMLOCK_CHECK_RESET_FC,
  SIMLOCK_CHECK_MASTER_UNLOCK
}T_SIMLOCK_CHECK;

typedef enum
{
  SIMLOCK_NETWORK = 0,              /* Network personalisation */
  SIMLOCK_NETWORK_SUBSET,      /* Network subset personalisation */
  SIMLOCK_SERVICE_PROVIDER,   /* Service provider personalisation */
  SIMLOCK_CORPORATE,               /* Corporate personalisation */
  SIMLOCK_SIM,                           /* SIM personalisation */
  SIMLOCK_FIRST_SIM  ,             /* ME is not personalized and will be personalisation to first 
                                             SIM inserted into the ME */
  SIMLOCK_BLOCKED_NETWORK,   /*Blocked network category */
  SIMLOCK_LAST                               
} T_SIMLOCK_TYPE;

 
typedef enum
{
  OPER_FAIL = -1,
  OPER_SUCCESS,  
  OPER_WRONG_PASSWORD,
  OPER_BUSY,
  OPER_NOT_ALLOWED,
  OPER_BLOCKED

} T_OPER_RET_STATUS;
 

#define NO_SP_DEFINED 255   /* If service provider (SP) code holds FF, no SP code is defined */
#define NO_CORP_DEFINED 255 /* If corporate code holds FF, no corporate code is defined */



  
#define T_ACI_PERS_STATUS_LEN 44
#define T_ACI_PERS_STATUS_PUK_LEN 52


typedef struct
{
  UBYTE State;
  UBYTE cnt;                                       /* No of retries  */
  UBYTE maxcnt;                                    /* Max count allowed  */
  UBYTE PwdLength;
  UBYTE Cur_code[3];                               /* Current lock  */
  UBYTE Org_code[3];                               /* Original lock  */
} T_ACI_PERS_MMI_DATAS;


/* prototypes of the aci_personalisation functions.-------------------------------------- */

/*
  Initialisation of this extension. Might be used to initialise variables, set temporary data to default values and so on.
  This method will be called before any other public method of this extension.
  */
void aci_ext_personalisation_init( void );



/*
  ACI extension method for retrieving the status of a single personalisation type.
  The personalisation status is stored in a separate customer managed memory area
  and the customer has to implement this extension method to return the actual status
  of the questioned personalisation type.
  */
T_SIMLOCK_STATUS aci_ext_personalisation_get_status( T_SIMLOCK_TYPE personalisation_type );

/* ACI extension to Get desired password length */
UBYTE aci_ext_personalisation_get_MMI_PwdLen( void );

/*
 ACI extension method for verifying the passowrd.
 If the password is false then the counter is increased. If maxcnt is reached then the key
 will been blocked.
 If the password is correct then the counter is set to 0
 */
T_SIMLOCK_STATUS aci_ext_personalisation_verify_password( T_SIMLOCK_TYPE personalisation_type, char *passwd);

/*
 ACI extension method for changind the password.
 */
T_OPER_RET_STATUS aci_ext_personalisation_change_password( T_SIMLOCK_TYPE personalisation_type, char *passwd, char *new_passwd );


/*
  ACI extension method for retrieving the status of a single personalisation type.
  The personalisation status is stored in a separate customer managed memory area
  and the customer has to implement this extension method to return the actual status
  of the questioned personalisation type.
  */
T_SIMLOCK_STATUS aci_ext_personalisation_set_status( T_SIMLOCK_TYPE personalisation_type, 
  T_SIMLOCK_STATUS lock, char* passwd);


#ifdef SIM_PERS
/*---------------------------------------------------
For Failure Counter reset. Uses Security Drv. method to reset FC
added on 11/03/2005
-----------------------------------------------------*/
T_OPER_RET_STATUS  aci_ext_slock_reset_fc(char *fckey);

/*--------------------------------------
For Supplementary Info( e.g. FC MAX, MC Current value).
Uses Security Drv api sec_get_CFG
added on 11/03/2005
----------------------------------------*/
T_OPER_RET_STATUS  aci_ext_slock_sup_info (T_SUP_INFO *sup_info);

T_OPER_RET_STATUS  aci_ext_slock_master_unlock (char *masterkey); 

void aci_ext_set_timer_flag(BOOL status);

UBYTE aci_ext_check_timer(void); 

/*  auto personalisation for first sim */
T_AUTOLOCK_STATUS aci_ext_auto_personalise();
void aci_ext_add_code(T_SIMLOCK_TYPE type);

#endif /* SIM_PERS */


/* MMI Lock handling functions */
#ifdef TI_PS_FF_AT_P_CMD_SECP
T_SIMLOCK_STATUS aci_ext_personalisation_CS_change_password( char *passwd, char *new_passwd );
#endif /* TI_PS_FF_AT_P_CMD_SECP */
#ifdef TI_PS_FF_AT_P_CMD_SECS
T_SIMLOCK_STATUS aci_ext_personalisation_CS_set_status(T_SIMLOCK_STATUS lock , char * passwd);
T_SIMLOCK_STATUS aci_ext_personalisation_CS_get_status();
#endif /* TI_PS_FF_AT_P_CMD_SECS */

#ifdef SIM_PERS
void aci_ext_personalisation_free( void );
#endif /*SIM_PERS */

#endif /* ACI_EXT_PERS_H */
