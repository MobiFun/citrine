/*
+--------------------------------------------------------------------+
| PROJECT:                              $Workfile:: cphs.h          $|
| $Author::                             $Revision::                 $|
| CREATED:                              $Modtime::                  $|
| STATE  : code                                                      |
+--------------------------------------------------------------------+

   MODULE  : CPHS

   PURPOSE : header file for CPHS functionalities.
*/

#ifndef CPHS_H
#define CPHS_H

/* CPHS functions return type */
typedef enum
{
  CPHS_FAIL 			= -1,
  CPHS_OK,
  CPHS_EXEC,
  CPHS_BUSY,                                        
  CPHS_NOT_INIT
} T_CPHS_RET;

/* Definition of parameter LINES */
#define T_CPHS_LINES USHORT

#define  CPHS_LINE_NULL (0x0000)
#define  CPHS_LINE1     (0x0001)
#define  CPHS_LINE_DATA (0x0002)
#define  CPHS_LINE_FAX  (0x0004)
#define  CPHS_LINE2     (0x0100)

/* Definition of max length for operator name */
#define CPHS_MAX_OPER_LONG  (30)
#define CPHS_MAX_OPER_SHORT (10)

/* Definition for customer service profile */
#define CPHS_MAX_CSP  (22) /* Phase 2: field should be always 22 bytes long (RFU = 2 bytes) */

//#define CPHS_SERVICE_CSP  (1)
//#define CPHS_SERVICE_SST  (2)
//#define CPHS_SERVICE_MB   (3)
//#define CPHS_SERVICE_OPNS (4)
//#define CPHS_SERVICE_NUMS (5)
#define CPHS_SERVICE_CSP  (5)
#define CPHS_SERVICE_SST  (6)
#define CPHS_SERVICE_MB   (7)
#define CPHS_SERVICE_OPNS (8)
#define CPHS_SERVICE_NUMS (1)
#define CPHS_SERVICE_ALLOCATED (1)
#define CPHS_SERVICE_ACTIVATED (2)
#define CPHS_CHECK_SST(sst,service,attribute) \
                      ((UBYTE) ((sst) >> ((service-1)*2)) & (attribute))

/* Definition for mailbox numbers */
#define CPHS_MIN_MB_ENTRIES     (1) 
#define CPHS_MAX_MB_ENTRIES     (4)
#define CPHS_MAX_MB_NUMBER_LEN (40) 
#define CPHS_MAX_MB_ALPHA_LEN  (22)
#define CPHS_MIN_MB_LEN        (14)

#define CPHS_MAX_MB_NUMBER_BYTES (10)

#define CPHS_MAX_INF_ALPHA_TAG  (16)
#define CPHS_MAX_INF_NUMBER     (12)

/* Definition of flag status for voice message waiting */
#define CPHS_ERASE_WAITING_FLAG (0)
#define CPHS_SET_WAITING_FLAG   (1)
#define CPHS_QUERY_WAITING_FLAG (2)

/* Definition of flag status for CFU */
#define CPHS_ERASE_CFU_FLAG (0)
#define CPHS_SET_CFU_FLAG   (1)
#define CPHS_QUERY_CFU_FLAG (2)

#define CPHS_FLAG_DEACTIVATED (0)
#define CPHS_FLAG_ACTIVATED   (1)
#define CPHS_FLAG_NOT_PRESENT (2)
#define CPHS_FLAG_ERROR       (3)

/* CPROAM related parameters */
typedef enum			/* Roaming indicator values */
{
  CPHS_ROAMING_STAT_NotPresent = -1,
  CPHS_ROAMING_OFF,
  CPHS_NATIONAL_ROAMING_ON,
  CPHS_INTERNATIONAL_ROAMING_ON
} T_CPHS_ROAMING_IND;

/* Information numbers feature related parameter */
typedef enum
{
  CPNUMS_MODE_EXPLORE	= 1,	// should apply to a folder to be explored
  CPNUMS_MODE_QUERY	= 2  	// queries information related to an entry
} T_CPHS_CPNUMS_MODE;

/* structure used to hold information numbers entry */
typedef struct
{
  UBYTE   element_index;    // index of element described in this structure
  UBYTE	  index_level;
  CHAR 		alpha_tag[CPHS_MAX_INF_ALPHA_TAG];
  CHAR		number[CPHS_MAX_INF_NUMBER];
  UBYTE   type_of_address;
  BOOL 		premium_flag;		  // should it be charged with premium price?
  BOOL 		network_flag;		  // is it network specific ?
} T_CPHS_INF_NUM;

/* structure used to hold cphs information */
typedef struct
{
  UBYTE	  phase;  
  USHORT  sst;    /* same format as on the SIM: field 6F16 */
} T_CPHS_CINF;

/* structure used to hold mailbox number entries */
typedef struct
{
  T_CPHS_LINES line;
  CHAR         number[CPHS_MAX_MB_NUMBER_LEN];
  UBYTE        toa;
  CHAR         alpha_id[CPHS_MAX_MB_ALPHA_LEN];
} T_CPHS_MB;

/* Callback types */
typedef enum
{
  CPHS_INIT_RES = 	1,
  CPHS_ROAM_IND,
  CPHS_VOICE_MAIL_IND,
  CPHS_VOICE_MAIL_RES,
  CPHS_CFU_RES
} T_CPHS_CB;

/* Callback parameters */
typedef struct
{
  T_CPHS_CB     cb_type;
  T_CPHS_RET    operation_result;
  UBYTE         set_flag;
  T_CPHS_LINES  line;
} T_CPHS_PARAMS;

/* data buffer for phonebook alpha */
typedef struct 
{
  UBYTE    data[CPHS_MAX_MB_ALPHA_LEN]; 
  UBYTE    len;
} T_CPHS_PB_TEXT;

/* Callback general prototype */
typedef void T_CPHS_USER_CB( T_CPHS_PARAMS *params );


/* user interface functions */
EXTERN T_CPHS_RET cphs_start (T_CPHS_USER_CB *cphs_user_cb);
EXTERN T_CPHS_RET cphs_refresh_data (void);
EXTERN T_CPHS_RET cphs_stop (void);
EXTERN T_CPHS_RET cphs_check_status(void);
EXTERN T_CPHS_RET cphs_explore_info_nbs (UBYTE element_idx, 
                                         UBYTE *inf_num_indexes, 
                                         UBYTE *max_elmts);
EXTERN T_CPHS_RET cphs_read_info_nb (UBYTE element_idx,
                                     T_CPHS_INF_NUM *inf_num);
EXTERN T_CPHS_RET cphs_info_num_get_max(UBYTE *max_index);
EXTERN T_CPHS_RET cphs_get_line(UBYTE        srcId, 
                                UBYTE        call_id, 
                                T_CPHS_LINES *line, 
                                CHAR         *line_desc, 
                                UBYTE        *max_line_desc);
EXTERN T_CPHS_RET cphs_get_fwd_flag(UBYTE *cfu_set, 
                                    T_CPHS_LINES line);
EXTERN T_CPHS_RET cphs_set_waiting_flag(UBYTE flag_set, T_CPHS_LINES lines);
EXTERN T_CPHS_RET cphs_get_waiting_flag(UBYTE *flag_set, T_CPHS_LINES line);
EXTERN T_CPHS_RET cphs_get_opn         (CHAR *longname,  UBYTE *max_longname,
                                        CHAR *shortname, UBYTE *max_shortname);
EXTERN T_CPHS_RET cphs_get_cphs_info   (UBYTE *phase, USHORT *sst);
EXTERN T_CPHS_RET cphs_get_csprof      (CHAR  *csp,
                                        CHAR  *csp2,
                                        UBYTE *max_csp_length,
                                        UBYTE *max_csp2_length);
EXTERN T_CPHS_RET cphs_read_mb_number  (BYTE rec_id, T_CPHS_MB *mailbox_entry);
EXTERN SHORT cphs_state_indication     (UBYTE psaStatus, SHORT cmeError);
EXTERN void cphs_write_mb_number_cb    (SHORT table_id);
GLOBAL T_CPHS_RET cphs_write_mb_number (UBYTE         srcId,
                                        UBYTE         rec_id,
                                        UBYTE        *tag,
                                        UBYTE         tag_len,
                                        UBYTE         bcd_len,
                                        UBYTE        *number,
                                        UBYTE         ton_npi);
EXTERN T_CPHS_RET cphs_first_free      (UBYTE *first_free);
GLOBAL T_CPHS_RET cphs_get_mb_parameter (  SHORT*        firstIdx,
                                           SHORT*        lastIdx,
                                           UBYTE*        nlength,
                                           UBYTE*        tlength );


EXTERN BOOL       cphs_line_makes_sense(T_CPHS_LINES line);
EXTERN void cphs_write_csp_cb    (SHORT table_id);
EXTERN T_CPHS_RET cphs_set_csp_value(UBYTE srcId,
                                     UBYTE *csp,
                                     UBYTE csp_len);

/* control interface functions */
EXTERN void cphs_voice_mail_ind   (UBYTE flag_set, USHORT line);
EXTERN void cphs_roaming_ind      (UBYTE roaming_status);
EXTERN T_CPHS_RET cphs_set_cfu_flag(UBYTE cfu_set, T_CPHS_LINES lines);


/* ACI/CPHS adapter layer */
EXTERN void cphs_sim_access_data(UBYTE cphs_sim_operation,
                                 UBYTE cphs_sim_field,
                                 UBYTE record,
                                 UBYTE *data_buffer, 
                                 UBYTE data_buffer_size);

EXTERN void cphs_sim_read_mb_ext_rcd(UBYTE record,UBYTE *data_buffer); 


EXTERN void cphs_bcd2number(CHAR  *number,
                            UBYTE *bcd,
                            UBYTE bcd_size);
EXTERN T_CPHS_RET cphs_get_als_active_line(UBYTE srcId, T_CPHS_LINES *line);

EXTERN T_CPHS_LINES als_get_call_info(SHORT call_id);




EXTERN void cphs_abort_current_action(void);
EXTERN void cphs_sim_data_accessed(UBYTE max_records, UBYTE data_len);
EXTERN void cphs_write_sim_mb_ext_data(UBYTE data_len);
EXTERN void cphs_sim_data_failure(void);


/* structure containing internal parameters of CPHS */
/* describes SIM operation in progress */
#define CPHS_SIM_NO_ACTION       (0)
#define CPHS_SIM_READ_TRANSP_EF  (1)
#define CPHS_SIM_WRITE_TRANSP_EF (2)
#define CPHS_SIM_READ_RECORD     (3)
#define CPHS_SIM_WRITE_RECORD    (4)

/* describes SIM updating operation status */
#define CPHS_SIM_NOT_UPDATING   (0)
#define CPHS_SIM_CFU            (1)
#define CPHS_SIM_INFO_NUMS      (2)
#define CPHS_SIM_INFO_NUMS_EA01 (3)
#define CPHS_SIM_CSP            (4)
#define CPHS_SIM_VWI            (5)   /* when updating SIM user initiated */
#define CPHS_SIM_OPNLONG        (6)
#define CPHS_SIM_OPNSHORT       (7)
#define CPHS_SIM_ALSNAMES       (8)
#define CPHS_SIM_CINF           (9)
#define CPHS_SIM_MB            (10)
#define CPHS_SIM_STOP_INIT     (11)
/* Add support for Orange SIM's  */
#define CPHS_SIM_ORANGE_CSP    (12)

/* decribes SIM write failures */
#define CPHS_SIM_WRITE_FAIL     (0)
#define CPHS_SIM_WRITE_OK       (1)

#endif /* CPHS_H */
