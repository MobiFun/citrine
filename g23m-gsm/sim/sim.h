/* 
+----------------------------------------------------------------------------- 
|  Project :  GSM-PS (6302)
|  Modul   :  SIM
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
|  Purpose :  Definitions for the Protocol Stack Entity
|             SIM Application.
+----------------------------------------------------------------------------- 
*/ 

#ifndef SIM_H
#define SIM_H

#include <stdlib.h>
#include <stdio.h>
#ifdef TI_PS_HCOMM_CHANGE
#include "cl_hComm_handle.h"
#endif

/*==== CONSTANTS ==================================================*/
#if defined (_SIMULATION_)
#define SIM_TOOLKIT
#endif

#ifdef SIM_TOOLKIT
  #if defined (FF_WAP) OR defined (_SIMULATION_)
    #define FF_SAT_C
  #endif /* FF_WAP */
#endif /* SIM_TOOLKIT */

#ifdef FF_SAT_E
#include "dti.h"
#endif


#ifdef TI_PS_OP_VSI_NO_CALL_ID
#define TIMER_START(C,I,T)         vsi_t_start_nc(I,T)
#define TIMER_PSTART(C,I,T,R)      vsi_t_pstart_nc(I,T,R)
#define TIMER_STOP(C,I)            vsi_t_stop_nc(I)
#define TIMER_STATUS(C,I,T)        vsi_t_status_nc(I,T)
#define SUSPEND_SELF(C,T)          vsi_t_sleep_nc(T)
#define SYSTEM_TIME(C,T)           vsi_t_time_nc(T)               
#else /* TI_PS_OP_VSI_NO_CALL_ID */
#define TIMER_START(C,I,T)         vsi_t_start(C,I,T)
#define TIMER_PSTART(C,I,T,R)      vsi_t_pstart(C,I,T,R)
#define TIMER_STOP(C,I)            vsi_t_stop(C,I)
#define TIMER_STATUS(C,I,T)        vsi_t_status(C,I,T)
#define SUSPEND_SELF(C,T)          vsi_t_sleep(C,T)
#define SYSTEM_TIME(C,T)           vsi_t_time(C,T)               
#endif /* TI_PS_OP_VSI_NO_CALL_ID */


/*
 * To read 255 bytes the SIM driver shall not append SW1, SW2
 * to the end of the data. To be backward compatible, the
 * following define is used. One of both definitions must be used
 * depending on the SIM driver capabilities.
 */
//#define SIM_TI_DRV_X_BYTES  2   //  SIM driver adds SW1, SW2 to response!
#define SIM_TI_DRV_X_BYTES  0     //  SIM driver does not add SW1, SW2 to response!

#define NO_ALLOCATED              0
#define ALLOCATED                 1
#define ALLOCATED_AND_DEACTIVATED 2
#define ALLOCATED_AND_ACTIVATED   3

#define NO_BDN_SIM                0
#define BDN_ENABLED               1
#define BDN_DISABLED              2

#define NO_FDN_SIM                0
#define FDN_ENABLED               1
#define FDN_DISABLED              2

#define ABSOLUT_READING       0
#define SIM_TPDU_HEADER_LEN   5

/* parameterblocks */

/* #define MAX_IMSI                9  moved to SAP SIM */
#define MAX_KC_N                (MAX_KC+1) /* = 9 (SAP SIM) */
/* #define MAX_PREF_PLMN           96 moved to SAP SIM */
#define MAX_BCCH_INFO           16
#define MAX_ACCESS_CONTROL      2
#define MAX_FORB_PLMN           12
#define MAX_LOC_INFO            11
#define MAX_SIM_CHARGE          2
#define MAX_OP_MODE             1

#define MAX_PIN                 8
#define MAX_UNBLOCKING_KEY      8
#define MAX_ICC_IDENTIF         10

#define VFY_CHV1                1   // verify PIN 1
#define VFY_CHV2                2   // verify PIN 2
#define UNBL_CHV1               0   // unblock PIN 1
#define UNBL_CHV2               2   // unblock PIN 2

/*
 * Access conditions
 */
#define LRP_NONE                0
#define LRP_PIN_1               (PHASE_2_PIN_1)   // 1
#define LRP_PIN_2               (PHASE_2_PIN_2)   // 2
#define LRP_PUK_1               (PHASE_2_PUK_1)   // 3
#define LRP_PUK_2               (PHASE_2_PUK_2)   // 4
#define LRP_NEVER               5

#define PIN_1                   LRP_PIN_1
#define PIN_2                   LRP_PIN_2
#define ALWAYS                  LRP_NONE
#define NEVER                   0xF
#define PIN_1_OR_2              10

#define ACCESS_READ             0
#define ACCESS_UPDATE           1
#define ACCESS_INCREASE         2
#define ACCESS_REHABILITATE     3
#define ACCESS_INVALIDATE       4

#define LINEAR_FIXED            1

#define SIM_TIMER               0

/*
#ifdef SIM_TOOLKIT
  #define MAX_SAT_TIMER         8
  #define NUM_OF_SIM_TIMERS    (MAX_SAT_TIMER + 1)
#else
  #define NUM_OF_SIM_TIMERS     1
#endif
*/

#ifdef FF_SAT_E
/*
 * timer definitions
 * index 0    app_sim_timeout()
 * index 1..8 SAT timer
 * index 9    BIP timer
 */
#define MAX_SAT_TIMER                 8
#define NUM_OF_SIM_TIMERS       (MAX_SAT_TIMER + 2)
#define SIM_BIP_TIMER           (MAX_SAT_TIMER + 1)
/*
 * states of DTI connection
 */
#define SIM_DTI_CONNECTION_CLOSED     1
#define SIM_DTI_CONNECTION_BIND       2
#define SIM_DTI_CONNECTION_SETUP      3
#define SIM_DTI_CONNECTION_OPEN       4
/*
 * states of DTI RX direction
 */
#define SIM_DTI_RX_IDLE               5
#define SIM_DTI_RX_READY              6
/*
 * states of DTI TX direction
 */
#define SIM_DTI_TX_IDLE               7
#define SIM_DTI_TX_READY              8
/*
 * states of BIP channel
 */
#define SIM_BIP_CLOSED                9
#define SIM_BIP_OPEN                 10
#define SIM_BIP_CONNECTED            11
/*
 * states of BIP RX direction
 */
#define SIM_BIP_RX_IDLE              12
#define SIM_BIP_RX_DATA              13
/*
 * states of BIP TX direction
 */
#define SIM_BIP_TX_IDLE              14
#define SIM_BIP_TX_STORE             15
#define SIM_BIP_TX_SEND              16
/*
 * states of BIP release timer
 */
#define SIM_BIP_TIMER_DISCONNECTED   18
#define SIM_BIP_TIMER_NOT_USED       19
#define SIM_BIP_TIMER_SUSPENDED      20
#define SIM_BIP_TIMER_STOPPED        21
#define SIM_BIP_TIMER_START          22
/*
 * CCD analysis states
 */
#define SIM_CCD_OK                   23
#define SIM_CCD_RETURN               24
#define SIM_CCD_DISCARD              25
/*
 * initial state for connection qualifier (no status change requested)
 */
#define SIM_DTI_CONN_OFF              0
/*
 * maximum nuber of bytes in Channel Data element in Termina Response message
 * message structure:
 * Command Details:     5 bytes
 * Device Identities:   4 bytes
 * Result:              3 bytes (without additional result)
 * Channel Data:        3 bytes overhead
 * Channel Data Length: 3 bytes
 *                    ---------
 *                     18 bytes
 * maximum length for any data sent to SIM is 255!
 */
#define SIM_TERM_RESP_MAX_CHANNEL_DATA  (255 - 18)
/*
 * transmit descriptor size
 */
#define SIM_BIP_TX_DESC_SIZE        (100 - sizeof(T_desc2))
/*
 * qualifier bits of SEND DATA message
 */
#define SIM_QLF_SEND_DATA_1                           0x01
/*
 * Definition whether a shared CCD buffer shall be used
 */
#define SHARED_CCD_BUF

#elif defined (SIM_TOOLKIT) /* else if FF_SAT_E */
    #define MAX_SAT_TIMER         8
    #define NUM_OF_SIM_TIMERS    (MAX_SAT_TIMER + 2)
#else /* SIM_TOOLKIT */
    #define NUM_OF_SIM_TIMERS     2
#endif

#define SLEEP_TIMER (NUM_OF_SIM_TIMERS - 1) 

#define MAX_STK_LENGTH       80

#ifdef TI_PS_UICC_CHIPSET_15
#define UICC_READER_ID 0x01
#endif

/* These are the only two peer second level standard defined DF's supported now.
   When more second level DF's are getting handled, this macro needs to be 
   adapted  */
#define SIM_IS_PEER_LEVEL2_DF(df1,df2) \
          ((df1 EQ SIM_DF_MEXE AND df2 EQ SIM_DF_SOLSA) || \
           (df1 EQ SIM_DF_SOLSA AND df2 EQ SIM_DF_MEXE) )

/* These are the only three second level standard defined DF's supported now.
   When more second level DF's are getting handled, this macro needs to be 
   adapted  */
#define SIM_IS_DF_LEVEL2_UNDER_DF_LEVEL1(df_level2, df_level1) \
          ((df_level2 EQ SIM_DF_MEXE AND \
              (df_level1 EQ SIM_DF_GSM || df_level1 EQ SIM_DF_1800))  || \
           (df_level2 EQ SIM_DF_SOLSA AND \
              (df_level1 EQ SIM_DF_GSM || df_level1 EQ SIM_DF_1800)) || \
           (df_level2 EQ SIM_DF_GRAPHICS AND df_level1 EQ SIM_DF_TELECOM) )

#define SIM_SYNC_AWAIT_MM_READ   1
#define SIM_SYNC_AWAIT_MMI_READ  2

/*
+********************************************************************+
| Moved from sim_stk.c - for CQ 34109 under feature flag SIM_TOOLKIT |
+********************************************************************+
*/

#ifdef SIM_TOOLKIT

#define STK_REFRESH                      0x01
#define STK_MORE_TIME                    0x02
#define STK_POLL_INTERVALL               0x03
#define STK_POLLING_OFF                  0x04
#define STK_SETUP_EVENT_LIST             0x05

#define STK_SET_UP_CALL                  0x10
#define STK_SEND_SS                      0x11
#define STK_SEND_USSD                    0x12
#define STK_SEND_SMS                     0x13
#define STK_SEND_DTMF                    0x14
#define STK_LAUNCH_BROWSER               0x15

#define STK_PLAY_TONE                    0x20
#define STK_DISPLAY_TEXT                 0x21
#define STK_GET_INKEY                    0x22
#define STK_GET_INPUT                    0x23
#define STK_SELECT_ITEM                  0x24
#define STK_SET_UP_MENU                  0x25
#define STK_PROVIDE_LOCAL_INFO           0x26
#define STK_TIMER_MANAGEMENT             0x27
#define STK_SETUP_IDLE_TEXT              0x28

#define STK_RUN_AT_CMD                   0x34
#define STK_LANGUAGE_NOTIFICATION        0x35

#ifdef FF_SAT_E
#define STK_OPEN_CHANNEL                 0x40
#define STK_CLOSE_CHANNEL                0x41
#define STK_RECEIVE_DATA                 0x42
#define STK_SEND_DATA                    0x43
#define STK_GET_CHANNEL_STAT             0x44
#endif /* FF_SAT_E */

#define STK_COMPREHENSION_REQUIRED       0x80

#define STK_COMMAND_DETAILS_TAG          0x01
#define STK_COMMAND_DETAILS_LEN             3

#define STK_DEVICE_IDENTITY_TAG          0x02
#define STK_DEVICE_IDENTITY_LEN             2

#define STK_RESULT_TAG                   0x03

#define STK_DURATION_TAG                 0x04
#define STK_DURATION_LEN                    2

#define STK_ALPHA_IDENTITY_TAG           0x05
#define STK_ALPHA_IDENTITY_LEN              1
                                         
#define STK_FILE_LIST_TAG                0x12
#define STK_FILE_LIST_LEN                   1

#define STK_EVENT_LIST_TAG               0x19
#define STK_EVENT_LIST_LEN                  1

#define STK_ICON_IDENTITY_TAG            0x1E
#define STK_ICON_IDENTITY_LEN               2

#define STK_TIMER_ID_TAG                 0x24
#define STK_TIMER_ID_LEN                    1

#define STK_TIMER_VALUE_TAG              0x25
#define STK_TIMER_VALUE_LEN                 3

#define STK_CHANNEL_DATA_TAG             0x36
#define STK_CHANNEL_DATA_LEN                1

#define STK_CHANNEL_DATA_LGTH_TAG        0x37
#define STK_CHANNEL_DATA_LGTH_LEN           1

#define STK_PROACTIVE_SIM_COMMAND_TAG    0xD0
#define STK_TIMER_EXPIRATION_TAG         0xD7

#define STK_RES_SUCCESS                  0x00
#define STK_RES_SUCC_PART_COMPR          0x01
#define STK_RES_SUCC_MISS_INFO           0x02
#define STK_RES_SUCC_ADD_EF_READ         0x03
#define STK_RES_SUCC_SESSION_END         0x10

#define STK_RES_BUSY_ME                  0x20
#define STK_RES_BUSY_NW                  0x21
#define STK_RES_BUSY_TIMER_STATE         0x24

#define STK_RES_ERR_NO_SUPPORT           0x30
#define STK_RES_ERR_CMD_TYPE             0x31
#define STK_RES_ERR_CMD_DATA             0x32
#define STK_RES_ERR_CMD_NR               0x33
#define STK_RES_ERR_MISS_VALUE           0x36

#define STK_RES_EXT_DEF                  0x00
#define STK_RES_EXT_BUSY_CALL            0x02
#define STK_RES_EXT_NO_SERVICE           0x04
#define STK_RES_SUCC_LIMITED_SERVICE     0x06

#define STK_MAX_EXP_TAG                  12   /* >= any max_tag entry */

#define item_of(_x_) (sizeof(_x_)/sizeof(_x_[0]))
#define BCD2INT(_x_) ((_x_&0xF)*10+(_x_>>4))
#define INT2BCD(_x_) (((_x_%10)<<4)|((_x_/10)&0xF))

#define STK_TP12_CLASS_E    (SAT_TP12_OPEN_CHANNEL | SAT_TP12_CLOSE_CHANNEL\
                             | SAT_TP12_RECEIVE_DATA | SAT_TP12_SEND_DATA\
                             | SAT_TP12_GET_CHANNEL_STAT)

/*
 *  Added for the purpose of checking service mode got from Common Library
 */
#define NO_SERVICE                     (0x0)      /* no service available           */
#define LIMITED_SERVICE                (0x1)      /* Limited service available      */
#define FULL_SERVICE                   (0x2)      /* Full service available         */

#endif /* SIM_TOOLKIT */

#ifdef GPRS
/*
 * provision of PCM definition as long as they are missing in L1
 */
#ifndef EF_LOCGPRS_ID
#define EF_LOCGPRS_ID   "LOCGPRS"       /* Location Inf. (GPRS)     */
typedef struct EFlocgprs        /* GPRS Location information        */
{
  UBYTE ptmsi[4];
  UBYTE ptmsi_signature[3];
  UBYTE rai[6];
  UBYTE ra_status;
} EF_LOCGPRS;

#define SIZE_EF_LOCGPRS sizeof (EF_LOCGPRS)
#define NR_EF_LOCGPRS   1
#endif

#ifndef EF_KCGPRS_ID
#define EF_KCGPRS_ID    "KCGPRS"        /* Ciphering Key (GPRS)     */
typedef struct EFkcgprs         /* GPRS Ciphering key               */
{
  UBYTE kc[8];
  UBYTE cksn;
} EF_KCGPRS;

#define SIZE_EF_KCGPRS sizeof (EF_KCGPRS)
#define NR_EF_KCGPRS   1
#endif

#ifndef EF_IMSIGPRS_ID
#define EF_IMSIGPRS_ID  "IMSIGPRS"      /* IMSI check for GPRS      */
typedef struct EFimsigprs       /* International Subscriber Id      */
{
  UBYTE len;
  UBYTE IMSI[8];
} EF_IMSIGPRS;

#define SIZE_EF_IMSIGPRS sizeof (EF_IMSIGPRS)
#define NR_EF_IMSIGPRS   1
#endif
#endif

/*==== TYPES ======================================================*/

/* CHIPCARD RESPONSES */

typedef struct {
    UBYTE filler1[4];
    UBYTE fileid[2];
    UBYTE filler1a[6];
    UBYTE length;
    UBYTE characteristics;
    UBYTE filler2[4];
    UBYTE pinstatus;
    UBYTE unbstatus;
    UBYTE pin2status;
    UBYTE unb2status;
    UBYTE sw1, sw2;           // needed by SIM_GetResponse()
} T_DIR_STATUS;
#define SIM_MIN_DMF_ST_LEN 22 // Mimimum length for response of SELECT MF/DF

typedef struct {
    UBYTE  filler1[2];
    UBYTE  field_size [2];
    UBYTE  filler2[4];
    UBYTE  access_1;
    UBYTE  access_2;
    UBYTE  access_3;
    UBYTE  file_status;
    UBYTE  filler4;
    UBYTE  field_type;
    UBYTE  record_length;
    UBYTE  sw1, sw2;            // needed by SIM_GetResponse()
} T_FIELD_STATUS;
#define SIM_MIN_EF_ST_LEN 15    // Minimum length for response of SELECT EF

typedef struct {
    UBYTE sres[4];
    UBYTE kc[8];
    UBYTE sw1, sw2;             // needed by SIM_GetResponse()
} T_SRES_KC;
#define SIM_GSM_ALG_LEN 12      // Minimum length for response of RUN GSM ALG

typedef struct {
    BYTE res_incr[3];
} T_RES_INCR;

typedef struct {                // format of SAT timer value, given in
  UBYTE hour;                   // hours,
  UBYTE minute;                 // minutes and
  UBYTE second;                 // seconds. Coding as for SCTS (GSM 03.40)
  UBYTE active;                 // TRUE, if timer is running
} T_SAT_TIMER;

#ifdef FF_SAT_E 
/*
 * struct to transfer IP addresses and UDP ports between UDP and SIM
 */
typedef struct {
  UBYTE src_ip[4];    /* source IP address in network byte order */
  UBYTE des_ip[4];    /* destination IP address in network byte order */
  UBYTE src_port[2];  /* source UDP port in network byte order */
  UBYTE des_port[2];  /* destination UDP port in network byte order */
} T_SRC_DES;
#endif /* FF_SAT_E */

/*
 * Used for requesting location information from RR
 */
typedef struct
{
  UBYTE stk_class;
  UBYTE stk_subclass;
  UBYTE stk_type;
  UBYTE stk_length;
  UBYTE stk_parameter [MAX_STK_LENGTH];
} stk_data_type;

#if defined (SIM_TOOLKIT)
typedef struct
{
  UBYTE          response[256];
  stk_data_type  stk_data;
}
T_STK_POLL_DATA;

/*
 * Used for the processing of SIM Refresh in stk_proactive_polling() and
 * stk_sim_refresh_user_res()
 */
typedef struct 
{
  UBYTE *tag2;                 /* File List. 3rd Tag in STK command */
  SHORT fl_len;                /* File List Length   */
  UBYTE *p_cmd;                /* STK command        */
  SHORT cmd_len;               /* STK command length */
  UBYTE res_code [2] ;                   
  T_SIM_TOOLKIT_IND *sig_ptr;                   
}T_CONTEXT_SWITCH;
#endif


typedef struct
{
  // General SIM Variables
  ULONG          flags;                   // contains several flags for internal handling
  USHORT         act_directory;           // last selected directory
  USHORT         act_field;               // last selected elementary field
  T_TIME         status_time;             // status poll time
  USHORT         act_length;              // actual file or record length
  USHORT         sim_data_len;            // response data len
  USHORT         dir_status_len;          // size of directory status information
  USHORT         remove_error;            // provides error code for SIM Remove
  UBYTE          sim_phase;               // sim card phase
  UBYTE          last_requested_pin_no;   // last requested pin number
  UBYTE          act_access;              // actual access condition
  UBYTE          max_record;              // records of actual file
  UBYTE          field_type;              // of the last selected record
  UBYTE          sw1;                     // last status code SW1 from SIM
  UBYTE          sw2;                     // last status code SW2 from SIM
  // SIM Toolkit Variables
#ifdef SIM_TOOLKIT
  UBYTE          sync_awaited;            /* Stores entities from which SYNC_REQ is awaited */
  UBYTE          file_change_resp;        // tracks response to FILE CHANGE
  T_CONTEXT_SWITCH *context_switch_ptr;   // Allows handling of SIM Refresh from different threads
  SHORT          proactive_sim_data_len;
  USHORT         stk_resp_len;
  UBYTE          stk_response[16];        // save TERMINAL RESPONSE
  UBYTE          stk_profile[MAX_STK_PRF];
  UBYTE          trmst[MAX_TRMST];        // Terminal Support Table
  USHORT         cell_identity;
  T_loc_info     location_info;
  T_SAT_TIMER    timer[MAX_SAT_TIMER];
  UBYTE          sat_session;             /* SAT session started */
  UBYTE          ext_sat_cmd;             /* indicator for external SAT commands */
  UBYTE          term_resp_sent;          /* indicator if Terminal Response was sent */
  UBYTE          idle_polling;            /* poll SIM in idle mode */
  UBYTE          chk_sat_avail;           /* check SAT command to be fetched */
#ifdef FF_SAT_E
  UBYTE          dti_connection_state;    /* state of DTI connection */
  UBYTE          dti_rx_state;            /* state of DTI reception */
  UBYTE          dti_tx_state;            /* state of DTI transmission */
  UBYTE          event_data_avail;        /* Data available event */
  UBYTE          bip_ch_id;               /* BIP channel identifier */
  UBYTE          bip_suspend;             /* BIP suspension state */
  UBYTE          bip_state;               /* state of BIP channel */
  UBYTE          bip_rx_state;            /* state of BIP reception */
  UBYTE          bip_tx_state;            /* state of BIP transmission */
  UBYTE          bip_timer_state;         /* state of BIP release timer */
  UBYTE          bip_general_result;      /* general result code for on demand link */
  UBYTE          bip_add_info_result;     /* additional information for on demand link */
  UBYTE          con_type;                /* connection type */
  USHORT         received_data_pos;       /* start of not yet delivered data */
  DTI_HANDLE     hDTI;                    /* DTILIB handle */
  T_TIME         bip_release_time;        /* BIP release time */
  ULONG          link_id;                 /* DTI link identifer */
  T_desc_list2   data_to_send;            /* DTI data to send */
  T_desc_list2   prev_data_to_send;       /* last values of DTI data to send */
  T_desc_list2   received_data;           /* received DTI data */
  T_SRC_DES      udp_parameters;          /* ports and IP addresses for UDP */
  T_SIM_DTI_REQ* sim_dti_req;             /* storage of SIM_DTI_REQ primitive */
  T_SIM_BIP_REQ* sim_bip_req;             /* storage of SIM_BIP_REQ primitive */
  T_SIM_BIP_CONFIG_REQ* sim_bip_config_req; /* storage of SIM_BIP_CONFIG_REQ primitive */
  T_cmd_details  bip_rx_cmd_details;      /* command details for RX */
  T_cmd_details  bip_tx_cmd_details;      /* command details for TX */
  BUF_cmd_prms   bip_cmd_prms;            /* parameter buffer for RX and TX */
#endif /* FF_SAT_E */
  UBYTE          cust_mode;               /* Customer mode, rcvd in SIM_ACTIVATE_REQ */
  UBYTE          user_confirmation_expected; /* confirmation for SIM REFRESH expected */
#endif /* SIM_TOOLKIT */
  // Windows Simulation Variables
#if defined (_SIMULATION_)
  USHORT         mode;
#ifdef FF_DUAL_SIM
  UBYTE          simu_sim_num;
#endif /*FF_DUAL_SIM*/
#endif /* _SIMULATION_ */
#ifdef FF_DUAL_SIM
  UBYTE          SIM_Selection;
  USHORT         sim_num;
#endif /*FF_DUAL_SIM*/
} T_SIM_DATA;

typedef struct
{
  UBYTE invers;
  UBYTE atr_size;
  UBYTE atr_data[MAX_SIM_ATR];
}
T_SIM_CARD;

#if defined (SIM_TOOLKIT)
typedef struct                          // simple TERMINAL RESPONSE without parameters
{
  UBYTE          cmd_tag;               // command details tag
  UBYTE          cmd_len;               // command details length = 3
  UBYTE          cmd_nr;                // command reference number
  UBYTE          cmd_type;              // command type
  UBYTE          cmd_qulf;              // command qualifier
  UBYTE          dev_tag;               // device identities tag
  UBYTE          dev_len;               // device identities length = 2
  UBYTE          dev_src;               // source device identity
  UBYTE          dev_dest;              // destination device identity
  UBYTE          res_tag;               // result length tag
  UBYTE          res_len;               // result length
  UBYTE          res_gnrl;              // general result
  UBYTE          res_add_info[1];       // additional information on result
} T_SIM_TRSP_SIMPLE;
#define SAT_TRSP_HEADER    9
#define SAT_TRSP_MIN_RES  12
#endif


/*
 * FLAGS
 *
 * The SIM entity contains several flags. To save RAM all flags
 * are handled in a bit array.
 *
 * The following Macros are used
 *
 * SIM_SET_FLAG (bit)         sets the corresponding bit
 * SIM_CLEAR_FLAG (bit)       clears the corresponding bit
 * SIM_IS_FLAG_SET (bit)      checks whether the corresponding bit is set
 * SIM_IS_FLAG_CLEARED (bit)  checks whether the corresponding bit is cleared
 *
 */
#define SIM_SET_FLAG(bit)          (sim_data.flags |= (1UL<<bit))
#define SIM_CLEAR_FLAG(bit)        (sim_data.flags &= ~(1UL<<bit))
#define SIM_IS_FLAG_SET(bit)       (sim_data.flags & (1UL<<bit))
#define SIM_IS_FLAG_CLEARED(bit)  ((sim_data.flags & (1UL<<bit)) EQ 0)
/*
 * The following flags are defined:
 *
 * ADN_SUPPORT_BY_SIM (bit 0)
 * 1   The current SIM supports ADN
 * 0   The current SIM doesn't support ADN
 */
#define ADN_SUPPORT_BY_SIM 0
/*
 * FDN_SUPPORT_BY_SIM (bit 1)
 * 1   The current SIM supports FDN
 * 0   The current SIM doesn't support FDN
 */
#define FDN_SUPPORT_BY_SIM 1
/*
 * BDN_SUPPORT_BY_SIM (bit 2)
 * 1   The current SIM supports BDN
 * 0   The current SIM doesn't support BDN
 */
#define BDN_SUPPORT_BY_SIM 2
/*
 * FDN_SUPPORT_BY_MMI (bit 3)
 * 1   The current MMI supports FDN
 * 0   The current MMI doesn't support FDN
 */
#define FDN_SUPPORT_BY_MMI 3
/*
 * BDN_SUPPORT_BY_SIM (bit 4)
 * 1   The current MMI supports BDN
 * 0   The current MMI doesn't support BDN
 */
#define BDN_SUPPORT_BY_MMI 4
/*
 * CC_WITH_STK (bit 5)
 * 1   The current MMI supports Call Control with SIM Toolkit
 * 0   The current MMI supports Call Control with SIM Toolkit
 */
#define CC_WITH_STK 5
/*
 * SERVICE_2_SUPPORT (bit 6)
 * 1   The current SIM supports service 2 (FDN)
 * 0   The current SIM supports service 2 (FDN)
 */
#define SERVICE_2_SUPPORT 6
/*
 * SERVICE_3_SUPPORT (bit 7)
 * 1   The current SIM supports service 3 (ADN)
 * 0   The current SIM supports service 3 (ADN)
 */
#define SERVICE_3_SUPPORT 7
/*
 * SERVICE_4_SUPPORT (bit 8)
 * 1   The current SIM supports service 4 (SMS Status)
 * 0   The current SIM supports service 4 (SMS Status)
 */
#define SERVICE_4_SUPPORT 8
/*
 * SERVICE_7_SUPPORT (bit 9)
 * 1   The current SIM supports service 7 (Preferred PLMN)
 * 0   The current SIM supports service 7 (Preferred PLMN)
 */
#define SERVICE_7_SUPPORT 9
/*
 * SERVICE_26_SUPPORT (bit 10)
 * 1   The current SIM supports service 26 (Data Download via SMS)
 * 0   The current SIM supports service 26 (Data Download via SMS)
 */
#define SERVICE_26_SUPPORT 10
/*
 * SERVICE_31_SUPPORT (bit 11)
 * 1   The current SIM supports service 31 (BDN)
 * 0   The current SIM supports service 31 (BDN)
 */
#define SERVICE_31_SUPPORT 11
/*
 * GSM_DATAFIELD (bit 12)
 * 1   The initial directory is DF 900
 * 0   The initial directory is DF 1800
 */
#define GSM_DATAFIELD 12
/*
 * SIM_INSERT (bit 13)
 * 1   SIM is inserted
 * 0   SIM is not inserted
 */
#define SIM_INSERT 13
/*
 * MM_KNOWS_FROM_SIM (bit 14)
 * 1   MM knows from the SIM card content
 * 0   MM doesn't know from the SIM card content
 */
#define MM_KNOWS_FROM_SIM 14
/*
 * SIM_PIN_FLAG (bit 15)
 * 1   PIN entering is needed
 * 0   PIN entering is not needed
 */
#define SIM_PIN_FLAG 15
/*
 * PRO_ACTIVE_SIM (bit 16)
 * 1   The current SIM supports SIM Toolkit
 * 0   The current SIM doesn't support SIM Toolkit
 */
#define PRO_ACTIVE_SIM 16
/*
 * ACTIVATION_STARTED (bit 17)
 * 1   Activation started by MMI
 * 0   Activation started by SIM driver
 */
#define ACTIVATION_STARTED 17
/*
 * CALL_ACTIVE (bit 18)
 * 1   A call is active
 * 0   No call is active
 */
#define CALL_ACTIVE 18
/*
 * SERVICE_1_SUPPORT (bit 19)
 * 1   The current SIM supports service 1 (CHV1 disabling)
 * 0   The current SIM doesn't support service 1
 */
#define SERVICE_1_SUPPORT 19
/*
 * SERVICE_35_SUPPORT (bit 20)
 * 1   The current SIM supports service 35 (SM Status Report Storage)
 * 0   The current SIM doesn't support service 35
 */
#define SERVICE_35_SUPPORT 20
/*
 * SERVICE_39_SUPPORT (bit 21)
 * 1   Image files (icons) may exist on the SIM
 * 0   no Image files exist on the SIM
 */
#define DF_GRAPHICS_EXISTENT 21
/*
 * SERVICE_40_SUPPORT (bit 21)
 * 1   SOLSA files may exist on the SIM
 * 0   no SOLSA files exist on the SIM
 */
#define DF_SOLSA_EXISTENT 22
/*
 * SERVICE_38_SUPPORT (bit 23)
 * 1   The current SIM supports service 38 (GPRS Storage)
 * 0   The current SIM doesn't support service X
 */
#define SERVICE_38_SUPPORT 23

#ifdef REL99
/*
 * SERVICE_43_SUPPORT (bit 26)
 * 1   The current SIM supports service 43 (user controlled PLMN Selector with Access Technology)
 * 0   The current SIM supports service 43 (user controlled PLMN Selector with Access Technology)
 */
#define SERVICE_43_SUPPORT 26
/*
 * SERVICE_44_SUPPORT (bit 27)
 * 1   The current SIM supports service 44 (Operator controlled PLMN Selector with Access Technology)
 * 0   The current SIM supports service 44 (Operator controlled PLMN Selector with Access Technology)
 */
#define SERVICE_44_SUPPORT 27
#endif /* REL99 */

/*
 * SERVICE_41_SUPPORT (bit 22)
 * 1   ORANGE files may exist on the SIM
 * 0   no ORANGE files exist on the SIM
 */
#define DF_ORANGE_EXISTENT 25
/*
 * Handling of SIM retry failure
 * 1   SIM driver cannot recover from a communication problem even
 *     after several retries of the last SIM command; SIM reset is
 *     required
 * 0   normal
 */
#define DRV_FAILED_RETRY 29
/*
 * Handling of Poll Interval
 * 1   MS uses Test SIM or Test Network, Polling not restarted
 * 0   normal, the last SIM access of any prcedure restarts Polling
 * Setting 1 is compliant with test case 27.22.4.6 of GSM 11.10-4,
 * Setting 0 is compliant with clause 11.2.1/11.2.8 in GSM 11.11
 */
#define TEST_MODE_POLLING 30
/*
 * Recognition of Test SIM
 * 1   Bit 8 of Byte 1 of EF(AD) indicates a Test SIM
 * 0   Bit 8 of Byte 1 of EF(AD) does not indicate a Test SIM
 * This can be used to control the behaviour of the SIM Entity
 */
#define TEST_SIM_INSERTED 31

/*==== EXPORT =====================================================*/

#ifdef FF_SAT_E
/*
 * CCD decode buffer
 */
#ifdef OPTION_MULTITHREAD
  #define _decodedMsg   _ENTITY_PREFIXED(_decodedMsg)
#endif /* OPTION_MULTITHREAD */
#ifdef SHARED_CCD_BUF
#define CCD_START _decodedMsg = ccd_begin();
#define CCD_END   ccd_end();
#else /* SHARED_CCD_BUF */
#define CCD_START
#define CCD_END
#endif /* else SHARED_CCD_BUF */

#ifdef SIM_PEI_C
#ifdef SHARED_CCD_BUF
GLOBAL UBYTE* _decodedMsg;
#else /* SHARED_CCD_BUF */
GLOBAL UBYTE _decodedMsg[MAX_MSTRUCT_LEN_SAT];
#endif  /* else SHARED_CCD_BUF */
#else /* SIM_PEI_C */
#ifdef SHARED_CCD_BUF
EXTERN UBYTE* _decodedMsg;
#else /* SHARED_CCD_BUF */
EXTERN UBYTE _decodedMsg[MAX_MSTRUCT_LEN_SAT];
#endif  /* else SHARED_CCD_BUF */
#endif /* else SIM_PEI_C */
#endif  /* FF_SAT_E */

/*
 * Prototypes SIM Driver
 */
GLOBAL void   SIM_Init                 (void (*insert)(T_SIM_CARD * p_atr),
                                        void (*remove)(void));
GLOBAL void   SIM_Initialize           (void);
GLOBAL USHORT SIM_Register             (void (*insert)(T_SIM_CARD * p_atr),
                                        void (*remove)(void));
GLOBAL USHORT SIM_Reset                (T_SIM_CARD * p_atr);
#ifdef _SIMULATION_
#define SIM_Restart                    SIM_Reset
#else
GLOBAL USHORT SIM_Restart              (T_SIM_CARD * p_atr);
#endif
GLOBAL void   SIM_PowerOff             (void);
#ifdef FF_DUAL_SIM
GLOBAL USHORT SIM_SwitchDualSIM        (UBYTE sim_num);
GLOBAL USHORT SIM_GetSIM               (void);
#endif /*FF_DUAL_SIM*/
GLOBAL USHORT SIM_ChangeCHV            (UBYTE      * result,
                                        UBYTE      * oldCHV,
                                        UBYTE      * newCHV,
                                        UBYTE        chvType,
                                        USHORT     * size);
GLOBAL USHORT SIM_DisableCHV           (UBYTE      * result,
                                        UBYTE      * CHV,
                                        USHORT     * size);
GLOBAL USHORT SIM_EnableCHV            (UBYTE      * result,
                                        UBYTE      * CHV,
                                        USHORT     * size);
GLOBAL USHORT SIM_GetResponse          (UBYTE      * result,
                                        USHORT       len,
                                        USHORT     * size);
GLOBAL USHORT SIM_Increase             (UBYTE      * result,
                                        UBYTE      * dat,
                                        USHORT     * size);
GLOBAL USHORT SIM_Invalidate           (UBYTE      * result,
                                        USHORT     * size);
GLOBAL USHORT SIM_ReadBinary           (UBYTE      * result,
                                        USHORT       offset,
                                        USHORT       len,
                                        USHORT     * size);
GLOBAL USHORT SIM_ReadRecord           (UBYTE      * result,
                                        UBYTE        mode,
                                        UBYTE        recNum,
                                        USHORT       len,
                                        USHORT     * size);
GLOBAL USHORT SIM_Rehabilitate         (UBYTE      * result,
                                        USHORT     * size);
GLOBAL USHORT SIM_RunGSMAlgo           (UBYTE      * result,
                                        UBYTE      * rand,
                                        USHORT     * size);
GLOBAL USHORT SIM_Select               (USHORT       id,
                                        UBYTE      * dat,
                                        USHORT     * size);
GLOBAL USHORT SIM_Status               (UBYTE      * result,
                                        USHORT     * size);
GLOBAL USHORT SIM_Status_Extended      (UBYTE      * result,
                                        USHORT       len,
                                        USHORT     * size);
GLOBAL USHORT SIM_UnblockCHV           (UBYTE      * result,
                                        UBYTE      * unblockCHV,
                                        UBYTE      * newCHV,
                                        UBYTE        chvType,
                                        USHORT     * size);
GLOBAL USHORT SIM_UpdateBinary         (UBYTE      * result,
                                        UBYTE      * dat,
                                        USHORT       offset,
                                        USHORT       len,
                                        USHORT     * size);
GLOBAL USHORT SIM_UpdateRecord         (UBYTE      * result,
                                        UBYTE      * dat,
                                        UBYTE        mode,
                                        UBYTE        recNum,
                                        USHORT       len,
                                        USHORT     * size);
GLOBAL USHORT SIM_VerifyCHV            (UBYTE      * result,
                                        UBYTE      * chv,
                                        UBYTE        chvType,
                                        USHORT     * size);
GLOBAL USHORT SIM_TerminalResponse     (UBYTE      * result,
                                        UBYTE      * dat,
                                        USHORT       len,
                                        USHORT     * size);
GLOBAL USHORT SIM_TerminalProfile      (UBYTE      * result,
                                        UBYTE      * data,
                                        USHORT       length,
                                        USHORT     * size);
GLOBAL USHORT SIM_Fetch                (UBYTE      * result,
                                        USHORT       length,
                                        USHORT     * size);
GLOBAL USHORT SIM_Envelope             (UBYTE      * result,
                                        UBYTE      * dat,
                                        USHORT       len,
                                        USHORT     * size);
GLOBAL USHORT SIM_XchTPDU              (UBYTE      * dat,
                                        USHORT       trxLen,
                                        UBYTE      * result,
                                        USHORT       rcvLen,
                                        USHORT     * rcvSize);
GLOBAL void SIM_lock_cr17689(void);    /* force driver version supporting ATR provision */
/*
 * Prototypes Wrapping Functions
 */
GLOBAL UBYTE  FKT_check_pin_count      (UBYTE        count);
GLOBAL USHORT FKT_convert_error        (USHORT       sw1sw2, USHORT       size);
GLOBAL USHORT FKT_ChangeCHV            (UBYTE      * old_pin,
                                        UBYTE      * new_pin,
                                        UBYTE        pin_id);
GLOBAL USHORT FKT_DisableCHV           (UBYTE      * pin);
GLOBAL USHORT FKT_EnableCHV            (UBYTE      * pin);
#ifndef TI_PS_UICC_CHIPSET_15
GLOBAL USHORT FKT_GetResponse          (UBYTE      * data,
                                        USHORT       len);
#endif
GLOBAL USHORT FKT_Increase             (UBYTE      * data);
GLOBAL USHORT FKT_Invalidate           (void);
GLOBAL USHORT FKT_ReadBinary           (UBYTE      * data,
                                        USHORT       offset,
                                        USHORT       len);
GLOBAL USHORT FKT_ReadRecord           (UBYTE      * data,
                                        UBYTE        mode,
                                        USHORT       record,
                                        USHORT       length);
GLOBAL USHORT FKT_Rehabilitate         (void);
GLOBAL USHORT FKT_RunGSMAlgo           (UBYTE      * rand, UBYTE      * data, USHORT       len);
GLOBAL USHORT FKT_Select               (USHORT       id,
                                        BOOL         path_info_present, T_path_info * path_info_ptr,
                                        UBYTE      * data, USHORT  len);
GLOBAL USHORT FKT_UnblockCHV           (UBYTE      * unblockCHV,
                                        UBYTE      * new_CHV,
                                        UBYTE        chvType);
GLOBAL USHORT FKT_UpdateBinary         (UBYTE      * data,
                                        USHORT       length,
                                        USHORT       offset);
GLOBAL USHORT FKT_UpdateRecord         (UBYTE      * data,
                                        USHORT       length,
                                        UBYTE        mode,
                                        USHORT       record);
GLOBAL USHORT FKT_VerifyCHV            (UBYTE      * pin,
                                        UBYTE        pin_id);

GLOBAL USHORT FKT_Status               (UBYTE      * pin_cnt,
                                        UBYTE      * pin2_cnt,
                                        UBYTE      * puk_cnt,
                                        UBYTE      * puk2_cnt);
GLOBAL USHORT FKT_TerminalResponse     (UBYTE      * data,
                                        USHORT       length);
GLOBAL USHORT FKT_TerminalProfile      (UBYTE      * data,
                                        USHORT       length);
GLOBAL USHORT FKT_Fetch                (UBYTE      * cmd,
                                        USHORT       length);
GLOBAL USHORT FKT_Envelope             (UBYTE      * data_out, UBYTE      * data_in,
                                        USHORT       in_length,  USHORT       out_length);
/*
 * Prototypes Application
 */

GLOBAL void app_init_sim_data        (void);
GLOBAL void app_sim_read_req         (T_SIM_READ_REQ           * sim_read_req);
GLOBAL void app_sim_read_record_req  (T_SIM_READ_RECORD_REQ    * sim_read_record_req);
GLOBAL void app_sim_update_req       (T_SIM_UPDATE_REQ         * sim_update_req);
GLOBAL void app_sim_update_record_req(T_SIM_UPDATE_RECORD_REQ  * sim_update_record_req);
GLOBAL void app_sim_increment_req    (T_SIM_INCREMENT_REQ      * sim_increment_req);
GLOBAL void app_sim_verify_pin_req   (T_SIM_VERIFY_PIN_REQ     * sim_verify_pin_req);
GLOBAL UBYTE app_sim_mm_insert_ind    (T_SIM_MMI_INSERT_IND     * sim_mmi_insert_ind);
GLOBAL void app_sim_mmi_insert_ind   (T_SIM_MMI_INSERT_IND     * sim_mmi_insert_ind,
                                            UBYTE                      func);
GLOBAL void app_sim_sms_insert_ind   (void);
GLOBAL void app_sim_change_pin_req   (T_SIM_CHANGE_PIN_REQ     * sim_change_pin_req);
GLOBAL void app_sim_disable_pin_req  (T_SIM_DISABLE_PIN_REQ    * sim_disable_pin_req);
GLOBAL void app_sim_enable_pin_req   (T_SIM_ENABLE_PIN_REQ     * sim_enable_pin_req);
GLOBAL void app_sim_unblock_req      (T_SIM_UNBLOCK_REQ        * sim_unblock_req);
GLOBAL void app_sim_auth_req         (T_SIM_AUTHENTICATION_REQ * sim_authentication_req);
#ifndef TI_PS_UICC_CHIPSET_15
GLOBAL void app_sim_insert (T_SIM_CARD *p_atr);
#endif /*!TI_PS_UICC_CHIPSET_15*/
GLOBAL void app_sim_remove           (void);
GLOBAL void app_sim_timeout          (U16 timer);
GLOBAL void app_sim_mm_update_req    (T_SIM_MM_UPDATE_REQ      * sim_mm_update_req);
GLOBAL void app_sim_sync_req         (T_SIM_SYNC_REQ           * sim_sync_req);
GLOBAL void app_sim_activate_req     (T_SIM_ACTIVATE_REQ       * sim_activate_req);
GLOBAL void app_sim_access_req       (T_SIM_ACCESS_REQ         * sim_access_req);
GLOBAL void app_sim_read_parameters  (void);
GLOBAL void app_sim_phase            (void);
GLOBAL void app_sim_card_error       (USHORT                     error);
GLOBAL UBYTE app_sim_check_service    (UBYTE                      nr,
                                            UBYTE                    * serv_table);
GLOBAL void app_perform_profile_download (void);
GLOBAL UBYTE app_fdn_bdn_procedures       (T_SIM_MMI_INSERT_IND * sim_mmi_insert_ind);
GLOBAL UBYTE app_check_imsi_loci_validation (void);
GLOBAL UBYTE app_bdn_capability_request     (void);
GLOBAL UBYTE app_fdn_capability_request     (void);
GLOBAL UBYTE app_rehabilitate_imsi_loci     (void);
GLOBAL UBYTE app_get_fdn_status             (void);
GLOBAL void app_start_status_timer         (BOOL condx);
GLOBAL BOOL  app_check_access_conditions    (UBYTE proc,
                                                  T_FIELD_STATUS *field_status);

#define SERVICE(n,p) app_sim_check_service(n,p)

EXTERN T_SIM_DATA sim_data;

/*
*  These Functions are only temporary valid and should replaced as soon as possible
*/
EXTERN UBYTE get_network_meas   (UBYTE * chan_list);
EXTERN UBYTE get_bcch_chan_list (stk_data_type * out_stk_data);


#ifdef SIM_TOOLKIT
#ifdef TI_PS_FF_AT_P_CMD_CUST
GLOBAL void stk_sim_refresh_user_res      (T_SIM_REFRESH_USER_RES * sim_refresh_user_res);
#endif /* TI_PS_FF_AT_P_CMD_CUST */
GLOBAL void stk_check_tp                  (UBYTE *out_prf,
                                                 UBYTE *in_prf,
                                                 USHORT len);
GLOBAL void stk_perform_profile_download  (void);
GLOBAL void stk_proactive_polling         (void);
GLOBAL void stk_stop_all_sat_timers       (void);
GLOBAL void stk_init_sim_data             (void);
#ifdef FF_SAT_E
GLOBAL void stk_dti_connection_opened     (void);
GLOBAL void stk_dti_connection_closed     (void);
GLOBAL void stk_dti_data_received         (T_DTI2_DATA_IND* dti_data_ind);
GLOBAL void stk_dti_tx_buffer_full        (void);
GLOBAL void stk_dti_tx_buffer_ready       (void);
GLOBAL UBYTE stk_dti_bip_send_data         (T_sdu* message);

GLOBAL void stk_udp_bind_cnf              (T_UDP_BIND_CNF* udp_bind_cnf);
GLOBAL void stk_udp_closeport_cnf         (T_UDP_CLOSEPORT_CNF* udp_closeport_cnf);
GLOBAL void stk_udp_error_ind             (T_UDP_ERROR_IND* udp_error_ind);
GLOBAL void stk_udp_shutdown_ind          (T_UDP_SHUTDOWN_IND* udp_shutdown_ind);

GLOBAL void stk_sim_dti_req               (T_SIM_DTI_REQ* sim_dti_req);
GLOBAL void stk_sim_bip_req               (T_SIM_BIP_REQ* sim_bip_req);
GLOBAL void stk_sim_bip_config_req        (T_SIM_BIP_CONFIG_REQ* sim_bip_config_req);
GLOBAL void stk_sim_eventlist_req         (T_SIM_EVENTLIST_REQ* sim_eventlist_req);
#endif /* FF_SAT_E */
GLOBAL void stk_sim_toolkit_res           (T_SIM_TOOLKIT_RES * sim_toolkit_res);
GLOBAL void stk_sim_toolkit_req           (T_SIM_TOOLKIT_REQ * sim_toolkit_req);
GLOBAL void stk_file_update_res           (T_SIM_FILE_UPDATE_RES * file_update_res);
GLOBAL void stk_timeout (USHORT index);
#endif /* SIM_TOOLKIT */

#if defined (GPRS)
GLOBAL void gprs_gmm_insert_ind           (T_SIM_MM_INSERT_IND *sim_mm_insert_ind);
GLOBAL void gprs_sim_gmm_update_req       (T_SIM_GMM_UPDATE_REQ *sim_gmm_update_req);
GLOBAL       BOOL gprs_check_pcm_data           (T_imsi_field *sim_imsi);
#endif

/*
* for test cases only
*/
#if defined _SIMULATION_ && defined FF_SAT_E
EXTERN USHORT csf_sim_pei_timeout (USHORT index);
#endif /* _SIMULATION &&_FF_SAT_E */


/*
 * If all entities are linked into one module this definitions
 * prefixes the global data with the enity name
 */


#ifdef TI_PS_HCOMM_CHANGE
#ifdef OPTION_MULTITHREAD
#ifdef FF_SAT_E
  #define hCommUDP        _ENTITY_PREFIXED(hCommUDP)
#endif /* FF_SAT_E */
#if defined (GPRS)
  #define hCommGMM        _ENTITY_PREFIXED(hCommGMM)
#endif /* GPRS */
#endif /* OPTION_MULTITHREAD */

#ifdef FF_SAT_E
EXTERN T_HANDLE hCommUDP;                /* UDP  Communication       */
#endif /* FF_SAT_E */
#if defined (GPRS)
#define GMM hCommGMM
EXTERN T_HANDLE hCommGMM;                /* SMS  Communication       */
#endif /* GPRS */
EXTERN T_HANDLE sim_handle;

#else
#ifdef OPTION_MULTITHREAD
  #define hCommSMS        _ENTITY_PREFIXED(hCommSMS)
  #define hCommMM         _ENTITY_PREFIXED(hCommMM)
  #define hCommMMI        _ENTITY_PREFIXED(hCommMMI)
#ifdef FF_SAT_E
  #define hCommUDP        _ENTITY_PREFIXED(hCommUDP)
#endif /* FF_SAT_E */
#ifdef _SIMULATION_
  #define hCommSIM        _ENTITY_PREFIXED(hCommSIM)
#endif
#if defined (GPRS)
  #define hCommGMM        _ENTITY_PREFIXED(hCommGMM)
#endif /* GPRS */
#endif /* OPTION_MULTITHREAD */
EXTERN T_HANDLE hCommMM;                 /* MM   Communication       */
EXTERN T_HANDLE hCommMMI;                /* MMI  Communication       */
EXTERN T_HANDLE hCommSMS;                /* SMS  Communication       */
#ifdef FF_SAT_E
EXTERN T_HANDLE hCommUDP;                /* UDP  Communication       */
#endif /* FF_SAT_E */
#ifdef _SIMULATION_
EXTERN T_HANDLE hCommSIM;                /* TEST CASES: TAP Communication */
#endif
#if defined (GPRS)
#define GMM hCommGMM
EXTERN T_HANDLE hCommGMM;                /* SMS  Communication       */
#endif /* GPRS */
EXTERN T_HANDLE sim_handle;
#endif /* TI_PS_HCOMM_CHANGE */

#endif /* !SIM_H */
