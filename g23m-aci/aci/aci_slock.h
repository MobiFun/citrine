/* 
+----------------------------------------------------------------------------- 
|  Project :  
|  Modul   :  J:\g23m-aci\aci\aci_slock.h
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

#ifndef ACI_SLOCK_H
#define ACI_SLOCK_H

#ifdef SIM_PERS
/* Shared globals ------------------------------------------------ */

typedef struct{
  T_SIMLOCK_STATUS status[SIMLOCK_LAST];  /* lint */   /* one status for every personalisation lock type */
  UINT16 dependency[SIMLOCK_LAST];
  UBYTE blocked;                 /* blocked flag */
  T_SIMLOCK_TYPE current_lock;            /* currently checked lock */
  UBYTE pb_load ; 
  T_SIMLOCK_CHECK check_lock; 
  UBYTE cpin_query ; 
  T_SIMLOCK_TYPE lock_type; 
  CHAR  * lock_passwd; 
}T_ACI_SLOCK_SHARED;


EXTERN T_ACI_SLOCK_SHARED AciSLockShrd;
EXTERN UBYTE sim_code_present_in_me;
#endif

#define NOT_PRESENT_8BIT  0xFF
#define MAX_GID 5
#define CHECK_FAIL 2

#ifdef SIM_PERS
/*Max number of user codes for each category- To be defined*/
#define MAX_NW_USER_CODES  20
#define MAX_NS_USER_CODES  10
#define MAX_SP_USER_CODES  10
#define MAX_CP_USER_CODES  10
#define MAX_SIM_USER_CODES 10
/*Normal code length for each category*/ 
#define NW_USER_CODE_LEN  3
#define NS_USER_CODE_LEN  4
#define SP_USER_CODE_LEN  7
#define CP_USER_CODE_LEN  11
#define SIM_USER_CODE_LEN 8

#define MAX_NUM_USERCODE_SIZE 1  
#define NUM_OPCODE_SIZE 1  
#define NUM_USER_CODE_SIZE 1
#define CURR_USER_CODE_INDEX_SIZE 1
#define OPCODE_LEN_SIZE 1 /* can be 2  */
#define OPCODE_LEN_INDEX 2 
#define CODE_TYPE_SIZE 1

#define FC_MAX_INFINITE 0xff  /* 04-08-2005 */


 /*
 *  BCD length 
 */
#define NW_CODE_LEN 4    /* Parity +MNC + MCC may require 4 bytes in packed bcd format if MNC length is 3 */
#define NS_CODE_LEN 1   /* NS requires 1 byte  in packed bcd format  */
#define NW_NS_CODE_LEN 5
#define NW_NS_MSIN_CODE_LEN 8
#define NW_NS_MSIN_MSIN_CODE_LEN 13
#define NW_NS_NS_CODE_LEN 6
#define NW_DIGIT_MAP_TABLE_LEN 24  /* Network code (4) + digit mapping table (10 * 2)  */


#define NORMAL_CODE 0x0a
#define INTERVAL_TYPE1 0x0b  /* Range  */
#define INTERVAL_TYPE2 0x0d  /* digit interval */
#define REGULAR_EXP 0x0c /* Regular expession  */

#define GID1_LEN 4  /* GID1 is not stored in BCD format */
#define GID2_LEN 4  /* GID2 is not stored in BCD format */
#define GID_LEN_TO_BE_COMPARED 1  /* depends on what the length is in the sim */

 enum
{     
  ALWAYS_ACCEPTED=0x00,
  ALWAYS_REJECTED= 0x01,
  UNTIL_NORMAL_SIM=0x02
} ;

enum
{
  SEND_CPIN_REQ_CODE_NONE =0,
  SEND_CPIN_REQ_CODE,
  SEND_CPIN_REQ_CODE_RAT  
} ;

typedef enum
{
  SIM_NORMAL =0,
  SIM_TYPEAPPROVAL,
  SIM_TEST
}T_SIM_TYPE ;

#endif

/* SIM configuration information */
typedef struct
{
   UBYTE deper_key [16];
   UBYTE phase;
   UBYTE oper_mode;       /* SIM card functionality   */
   UBYTE pref_lang[5];
   UBYTE access_acm;
   UBYTE access_acmmax;
   UBYTE access_puct;
   UBYTE sim_gidl1[MAX_GID];
   UBYTE sim_gidl2[MAX_GID];
  #ifdef SIM_PERS
   T_SIM_TYPE sim_type;  /*   */
   UBYTE sim_read_gid1;  /* added for SP, CP */
   UBYTE sim_read_gid2;  /* added for SP, CP */
   UBYTE gid1_len; 
   UBYTE gid2_len; 
   UBYTE sim_read_ad_first_byte;
   #endif

} T_ACI_SIM_CONFIG; /* This is the same as T_MFW_SIM_CONFIG */


/* Prototypes ----------------------------------------------------*/




#ifdef SIM_PERS
/*
  Initialising of this module. Has to be called first and *once only* before calling any other method.
*/
void aci_slock_init ( void );

/*
  Reset the aci_slock variables
*/
void aci_slock_reset(); 


/*
  Unlocks the lock of the given type after verifying the given password.
  The ACI extension for password verifying (see 2.3.9.1) will be used to
  determine if the given password is correct or not.
  On a successful unlock the actual status of the lock will be returned.
  If an error occurred SIMLOCK_FAIL will be returned.
  (Uses the ACI extension "personalisation data access".)
  */
T_SIMLOCK_STATUS aci_slock_authenticate ( T_SIMLOCK_TYPE type, char *passwd );


/*
 ACI method for retrieving the status of a single personalisation type. This method calls extension 
 methods which in turn calls Security Drv. API and retrn the status of a personalisation category.
 The personalisation status is stored in MEPD which is directly accessed by Security Drv. 
 Added on 11/03/2005
*/

T_SIMLOCK_STATUS aci_personalisation_get_status ( T_SIMLOCK_TYPE personalisation_type );
#endif
#ifdef SIM_PERS


/*
  Locks the lock of the given type.  On a successful lock the actual
  status of the lock will be returned. If an error occurred SIMLOCK_FAIL
  will be returned. This method will use the ACI extension for password
  verifying (see 2.3.9.1) to determine if the given password is correct
  or not. (Uses the ACI extension "personalisation data access".)
  */
T_SIMLOCK_STATUS aci_slock_lock ( T_SIMLOCK_TYPE type, char *passwd );

/*
For Failure Counter reset. Use extension methods
added on 11/03/2005
*/
T_OPER_RET_STATUS aci_slock_reset_fc ( char *fcKey );

/*--------------------------------------
For Supplementary Info( e.g. FC MAX, MC Current value).
Uses extension method aci_slock_sup_info
added on 11/03/2005
----------------------------------------*/
T_OPER_RET_STATUS aci_slock_sup_info(T_SUP_INFO *sup_info);

/*------------------------------------------
For master unlock. Unlock can be in bootup menu
or in security menu. 
-------------------------------------------*/
T_OPER_RET_STATUS aci_slock_master_unlock(char *masterkey);

/* change the password */
T_OPER_RET_STATUS aci_slock_change_password ( T_SIMLOCK_TYPE type, char *passwd, char *new_passwd );



#endif
/*
  Unlocks the lock of the given type after verifying the given password.
  The ACI extension for password verifying (see 2.3.9.1) will be used to
  determine if the given password is correct or not.
  On a successful unlock the actual status of the lock will be returned.
  If an error occurred SIMLOCK_FAIL will be returned.
  (Uses the ACI extension "personalisation data access".)
  */
T_SIMLOCK_STATUS aci_slock_unlock ( T_SIMLOCK_TYPE type, char *passwd );
/*
  Checks for all kinds of personalisation locks. The given IMSI will be checked against the internal 
  personalisation data. Returns SIMLOCK_BLOCKED, if one lock was not matching the given IMSI.
  */
T_SIMLOCK_STATUS aci_slock_checkpersonalisation( T_SIMLOCK_TYPE   current_lock );


/*
  PURPOSE : Install information found in the primitive into configuration buffers.
*/
void aci_slock_sim_init ( T_SIM_MMI_INSERT_IND *sim_mmi_insert_ind );


/*
   PURPOSE :   Call back for SIM read.

*/
void aci_slock_sim_read_sim_cb(SHORT table_id);

/*
   PURPOSE :   Request to read SIM card.

*/
void aci_slock_sim_read_sim(USHORT data_id, UBYTE len, UBYTE max_length);

#ifdef SIM_PERS
/*

PURPOSE :   Setting the sim_type value (Normal, Test SIM, Type Approval SIM)

*/

void aci_slock_set_simtype(T_SIM_TYPE sim_type );

/*
PURPOSE : To set global variable for configuration data ( part1 of Security Driver) 
*/
BOOL aci_slock_set_CFG(void);


void aci_slock_check_NWlock( UBYTE* imsi_sim, UBYTE  personalisation  );

void aci_slock_check_NSlock( UBYTE* imsi_sim, UBYTE  personalisation  );

void aci_slock_check_SPlock( UBYTE* imsi_sim, UBYTE  personalisation );
void aci_slock_check_CPlock( UBYTE* imsi_sim, UBYTE  personalisation);

void aci_slock_check_SMlock( UBYTE* imsi_sim, UBYTE  personalisation);

GLOBAL void aci_slock_psaSIM_decodeIMSI (UBYTE* imsi_field,  UBYTE  imsi_c_field, CHAR* imsi_asciiz);

GLOBAL void aci_slock_psaSIM_decodeIMSI_without_parity(UBYTE* imsi_field,  UBYTE  imsi_c_field,  CHAR* imsi_asciiz);

GLOBAL void aci_set_cme_error(T_SIMLOCK_TYPE slocktype);
GLOBAL void aci_set_cme_error_code(T_SIMLOCK_TYPE current_lock , T_ACI_CME_ERR *err_code);
GLOBAL void aci_set_cpin_code(T_SIMLOCK_TYPE current_lock ,T_ACI_CPIN_RSLT *code); 

/*
  PURPOSE : read SIM group identifier 1 or 2 from SIM card
*/
void   aci_slock_sim_gid_cnf( SHORT table_id );
GLOBAL void aci_set_cme_error_code_and_logRslt( UBYTE cmdBuf );

GLOBAL void  aci_slock_unlock_timer_stopped(void);
GLOBAL UBYTE aci_slock_check_timer(void);
GLOBAL void aci_slock_unlock_timer_stopped(void); 
GLOBAL UBYTE aci_slock_is_timer_support(void);
GLOBAL void  aci_slock_set_timer_flag(UBYTE status);
GLOBAL  void aci_slock_send_RAT(UBYTE cmdBuf,T_ACI_CME_ERR err_code ); 

#endif /* SIM_PERS */
#endif /* ACI_SLOCK_H */
