/********************************************************************************
 * Enhanced TestMode (ETM)
 * @file	etm_at.c (Support for AT-commands)
 *
 * @author	Kim T. Peteren (ktp@ti.com)
 * @version 0.2
 *


 *
 * History:
 *
 * 	Date       	Modification
 *  ------------------------------------
 *  16/06/2003	Creation
 *  06/11/2003  Small updates 
 *  18/02/2004  Major updating, the event handling has been updated
 *
 *
 * (C) Copyright 2003 by Texas Instruments Incorporated, All Rights Reserved
 *********************************************************************************/

//#include "aci_msg.h"

#include "etm/etm.h"          
#include "etm/etm_api.h"
#include "etm/etm_at.h"
#include "etm/etm_trace.h"

#include "rv/rv_general.h"
#include "rvf/rvf_api.h"

#include "atp/atp_env.h"  // FixMe
#include "atp/atp_i.h"    // FixMe    
//#include "atp/atp_general.h"  // FixMe

#include "atp/atp_api.h"
#include "atp/atp_messages.h" 

#include <string.h>

// Defined in aci_msg.h
extern T_XAT_SYNC_MESSAGE;
extern SYNC_PORT_NUM;
extern ASYNC_PORT_NUM;
extern T_RAT_CALLBACK_MESSAGE;


/******************************************************************************
 * Globals
 *****************************************************************************/

static T_ATP_SW_ENTITY_NAME  ETM_AT_ADAPTER      = "ETMATA"; // max 5 caracter
static T_ATP_SW_ENTITY_ID    etm_at_id;     
static T_ATP_PORT_NB         etm_at_to_aaa_port  = 0x01;     // equal to SYNC_PORT_NUM

static T_ATP_SW_ENTITY_NAME  aaa_name            = "AAA";     // ACIA ADAPTER
//static T_ATP_SW_ENTITY_NAME  aaa_name            = "GSM";     // GSM ADAPTER
static T_ATP_SW_ENTITY_ID    aaa_entity_id;
static T_ATP_ENTITY_MODE     aaa_mode;       

static T_ATP_CALLBACK etm_at_return_path;
static T_ATP_ENTITY_MODE etm_at_mode;

//extern T_ETM_ENV_CTRL_BLK *etm_env_ctrl_blk;

static int etm_at_initialized  = 0;
static int etm_at_event_status = 0;

static char etm_at_latest_cmd[9] = { 0 };


/******************************************************************************
 * Internal prototypes 
 *****************************************************************************/

int  etm_at_init(void);
int  etm_at_reg_req(void);
int  etm_at_open_port_req(void);
//T_RV_HDR *etm_at_wait_for_atp_event (UINT16 event_code);
int etm_at_atp_txt_cmd_rdy( T_ATP_TXT_CMD_RDY *msg);
void etm_at_callback_for_ATP(void *event_from_atp_p);


/******************************************************************************
 * AT commands List 
 *****************************************************************************/

struct at_async_trans_s {
//    const short index;  // index for ...
    const char *name;   // parameter
};

static struct at_async_trans_s at_cmd[] =
{
    {  "TEST"  },
    {  "atd"   },
    {  NULL    }
};


int at_cmd_search(char *at_string)
{
    struct at_async_trans_s *at_p = at_cmd;
    int size, error;

    size = strlen(at_string);
    tr_etm(TgTrCore, "ETM CORE: _cmd_search: at_string size(%d)", size);

    if (size > 8)
        strncpy(&etm_at_latest_cmd[0], at_string, 8);
    else
        strncpy(&etm_at_latest_cmd[0], at_string, size);

//    if ((etm_at_latest_cmd[2]== '+') || (etm_at_latest_cmd[2] == '%'))
//        return ETM_OK;
    
    while (at_p->name) {
        error = strncmp(&etm_at_latest_cmd[0], at_p->name, strlen(at_p->name));
        if (error == 0)
            strcpy(&etm_at_latest_cmd[0], at_p->name);
        tr_etm(TgTrCore, "ETM CORE: _cmd_search: AT list(%s)", at_p->name);
        at_p++;
    }

    tr_etm(TgTrCore, "ETM CORE: _cmd_search: text(%s)", &etm_at_latest_cmd[0]);

    return ETM_OK;
}


/******************************************************************************
 * AT command to ACI
 *****************************************************************************/

int etm_at_adapter(char *command)
{
    int error;
    T_ATP_TXT_CMD txt_cmd_p = NULL;
       
    if (!etm_at_initialized){
        if ((etm_at_init() == ETM_OK) && (etm_at_event_status == ETM_OK)) {
            // read etm_at_event_status
            tr_etm(TgTrCore, "ETM CORE: _at_adapter: initialization - OK");
            etm_at_initialized = 1;
        }
        else {
            tr_etm(TgTrCore, "ETM CORE: _at_adapter: initialization - FAILED");
            return ETM_FATAL;
        }
    }

    // Creating ETM_AT data buffer, will be fread by atp_send_txt_cmd()
    if ((txt_cmd_p = etm_malloc(strlen(command)+1)) == NULL)
        return ETM_NOMEM;

    strcpy(txt_cmd_p, command);

    // Find AT command
    //at_cmd_search(command);

    // Send AT command to AAA
    if ((error = atp_send_txt_cmd(etm_at_id, etm_at_to_aaa_port, AT_CMD, txt_cmd_p)) != RV_OK) {
        tr_etm(TgTrCore, "ETM CORE: _at_adapter: send_txt_cmd - FAILED");
        return ETM_FATAL;
    }

    return ETM_OK;
}


/******************************************************************************
 * ETM AT Adapter Initialization
 *****************************************************************************/

int etm_at_init(void)
{
    int error;

    // Check if ATP has been started if NOT 
    // Turn ATP module ON - necessary for RVM 
    if (atp_swe_state != ATP_STARTED)
        if (atp_start() != RV_OK)
            return ETM_FATAL;
   
 
    // Registration of ETM_AT to ATP  
    if ((error = etm_at_reg_req()) != ETM_OK) {
        tr_etm(TgTrCore, "ETM CORE: _at_init: Registration ERROR(%d)", error); 
       return error;    
    }    

    // Open a port to ACI adapter
    if ((error = etm_at_open_port_req()) != ETM_OK) {
        tr_etm(TgTrCore, "ETM CORE: _at_init: Open port ERROR(%d)", error);
        return error;
    }

    return ETM_OK;
}


// Register of ETM AT adapter with ATP.

int etm_at_reg_req(void)
{
    int result;

     // Registration of ETM_AT in ATP
    etm_at_return_path.addr_id        = NULL; // mailbox identifier - unused when callback mechanism in use
    etm_at_return_path.callback_func  = etm_at_callback_for_ATP;  // Pointer to callback fn ...

    // Set modes supported by SWE 
    etm_at_mode.cmd_mode         = TXT_MODE;  // INTERPRETED_MODE/TXT_MODE
    etm_at_mode.cp_mode          = COPY_OFF;
    etm_at_mode.cmd_support_mode = CMD_SUPPORT_ON;

    // Registration of ETM_AT to ATP
    if ((result = (atp_reg(ETM_AT_ADAPTER, etm_at_return_path, etm_at_mode, &etm_at_id)))
        != RV_OK) {
        tr_etm(TgTrCore, "ETM CORE: _at_reg_req: ERROR(%d)", result);
        return ETM_RV_FATAL;    
    }

    // Check ETM_AT Registration
    if ((result = (atp_reg_info(ETM_AT_ADAPTER, &etm_at_id, &etm_at_mode)))
        != RV_OK){
        tr_etm(TgTrCore, "ETM CORE: _at_reg_req: FAILED");
        return ETM_RV_NOT_SUPPORTED;
    }

    return ETM_OK;
}


// Open a port with ATP.

int etm_at_open_port_req(void)
{
    int result;
    T_ATP_NO_COPY_INFO etm_no_copy_info;
    T_ATP_PORT_INFO etm_port_info;
    T_ATP_CUSTOM_INFO *cust_info_p = NULL;
//    T_RV_HDR *message_p;
    T_RVF_MB_ID etm_mb_id;

    if (rvf_get_mb_id("ETM_PRIM", &etm_mb_id) != RVF_OK) {
        tr_etm(TgTrCore, "ETM CORE: _at_open_port_req: Memory bank ETM does not exist!");
        return ETM_RV_FATAL;
    }
         
	/* Test header and trailer removal from ATP so: trailers and headers equal 0 */
	etm_no_copy_info.tx_mb		    = etm_mb_id;   /* MB used by ATP is from ETM */ 
	etm_no_copy_info.rx_mb		    = etm_mb_id;   /* MB used by ATP is from ETM */
	etm_no_copy_info.rx_head_mode   = RX_HEADER_OFF;
    etm_no_copy_info.rx_head_size   = 0x00;
    etm_no_copy_info.rx_trail_size  = 0x00;
	etm_no_copy_info.tx_head_mode   = TX_HEADER_OFF;
    etm_no_copy_info.tx_head_size   = 0x00;
    etm_no_copy_info.tx_trail_size  = 0x00;
	etm_no_copy_info.packet_mode    = NORMAL_PACKET;     /* No L2CAP packet... */

    //  Port info 
    etm_port_info.port_config = NOT_DEFINED_CONFIG;
    etm_port_info.ring_type   = ATP_NO_RING_TYPE;
    etm_port_info.signal_mask = (T_ATP_SIGNAL_MASK) ATP_ALL_THE_SIGNAL_UNMASK; /* Get all signal changed event */
    etm_port_info.dce_mask[0] = 0x0000; /* No requirement in term of DCE */ 

    // Test AA Adapter Registration
    if (atp_reg_info(aaa_name, &aaa_entity_id, &aaa_mode) != RV_OK) {
        tr_etm(TgTrCore, "ETM CORE: _at_open_port_req: AAA is not registered to ATP");
        return ETM_RV_NOT_SUPPORTED;
    }

    // Open a virtual port between ETM AT adapter and ACI adapter 
    if ((result = atp_open_port_rqst(etm_at_id, aaa_entity_id, etm_at_to_aaa_port, 
                                     etm_port_info, etm_no_copy_info, cust_info_p)) != RV_OK) {
        tr_etm(TgTrCore, "ETM CORE: _at_open_port_req: FAILED");
        return ETM_RV_FATAL;
    }

    // etm_at_callback_for_ATP should receive event: ATP_OPEN_PORT_CFM 
    rvf_wait(0xffff, 100); // Timeout 100 ticks
    tr_etm(TgTrCore, "ETM CORE: _at_open_port_req: STATUS(%d)", etm_at_event_status);

    return ETM_OK;
}


/******************************************************************************
 * Close Port
 *****************************************************************************/

int etm_at_port_close_req(void)
{ 
    int error;

    error = atp_close_port(etm_at_id, etm_at_to_aaa_port);
    if (error != RV_OK) {
    tr_etm(TgTrCore, "ETM CORE: _at_port_close_req: FAILED");
        return ETM_FATAL;
    }        
    
    // etm_at_callback_for_ATP should receive event: ATP_PORT_CLOSED 
    rvf_wait(0xffff, 100); // Timeout 100 ticks
    tr_etm(TgTrCore, "ETM CORE: _at_open_port_req: STATUS(%d)", etm_at_event_status);

    etm_at_initialized = 0;
    return ETM_OK;
}


/******************************************************************************
 * Callback function for ATP
 *****************************************************************************/

//   PURPOSE : Decipher and route incoming messages from ATP.

void etm_at_callback_for_ATP(void *event_from_atp_p)
{
	// This function is ATP context.
	
//    tr_etm(TgTrEtmLow,"ETM: CORE: etm_at_callback_for_ATP: recv. event (0x%x)", ((T_RV_HDR *) event_from_atp_p)->msg_id);

    // What type of event?
    switch (((T_RV_HDR *) event_from_atp_p)->msg_id)
    {
    case ATP_CMD_RDY:
        tr_etm(TgTrCore,"ETM CORE: _at_callback_for_AT: UNSUPPORTED"); 
        break;
    case ATP_OPEN_PORT_CFM:
        if (((T_ATP_OPEN_PORT_CFM *) event_from_atp_p)->result == OPEN_PORT_OK) {
            tr_etm(TgTrCore, "ETM CORE: _at_callback_for_ATP: rev. event ATP_OPEN_PORT_CFM - OPEN_PORT_OK");
//            tr_etm(TgTrCore, "ETM CORE: _at_callback_for_ATP: rev. event ATP_OPEN_PORT_CFM - Port Number (%d)",
//                   ((T_ATP_OPEN_PORT_CFM *) event_from_atp_p)->initiator_port_nb);
        }
        else {
            tr_etm(TgTrCore, "ETM CORE: _at_callback_for_ATP: rev. event ATP_OPEN_PORT_CFM - OPEN_PORT_NOK");
            etm_at_event_status = ETM_FATAL;
        }
//            tr_etm(TgTrCore, "ETM CORE: _at_callback_for_ATP: rev. event ATP_OPEN_PORT_CFM - Port Number (%d)",
//                   ((T_ATP_OPEN_PORT_CFM *) event_from_atp_p)->initiator_port_nb);
        break;
    case ATP_TXT_CMD_RDY:
        etm_at_atp_txt_cmd_rdy((T_ATP_TXT_CMD_RDY *) event_from_atp_p);
        break;
    case ATP_PORT_CLOSED:
        tr_etm(TgTrCore, "ETM CORE: _at_callback_for_ATP: rev. event ATP_PORT_CLOSED");
        break;
    case ATP_OPEN_PORT_IND:
        tr_etm(TgTrCore, "ETM CORE: _at_callback_for_ATP: rev. event ATP_OPEN_PORT_IND");
        break;
    default:
        tr_etm(TgTrCore, "ETM CORE: _at_callback_for_ATP: rev. unknown event(0x%x)- UNSUPPORTED", 
               ((T_RV_HDR *) event_from_atp_p)->msg_id);
    }
    
    /* Free memmory that is allocated within ATP */
    etm_free(event_from_atp_p);
}      


// This is called when the result of the AT cmd is received 
// (in a primetive) from the ATP entity.
int etm_at_atp_txt_cmd_rdy(T_ATP_TXT_CMD_RDY *msg)
{
/* Send reply to PC
   The status type depend of the event from ATP module: 
       last_result = 0, means more data is sent to HOST (PC)
       last_result = 1, means last data is sent to HOST (PC)
       last_result = 2, means data is not sent to HOST (PC)  */

    T_ETM_PKT  *pkt;
    char *text, last_result = 1;
    int error = 0, length;

    if ((pkt = (T_ETM_PKT *) etm_malloc(sizeof(T_ETM_PKT))) == NULL) {
        rvf_dump_mem();
        return ETM_NOMEM;
    }
    
    // Init. of return packet
    pkt->mid    = ETM_CORE;
    pkt->status = ETM_OK;
    pkt->size   = 0;
    pkt->index  = 0;
    etm_pkt_put8(pkt, 'G'); // 'G' is indcator for AT command respons


    tr_etm(TgTrCore, "ETM CORE: _at_atp_txt_cmd_rdy: ATP_TXT_CMD_RDY with cmd_type(%d)",
           msg->cmd_type);
 
    switch (msg->cmd_type){
    case AT_CMD:                                                   // Type: 0
    case CUSTOM_CMD:                                               // Type: 4
    case CMD_ABORT:                                                // Type: 5
    case UNKNOWN:                  error = ETM_MESSAGE; break;     // Type: 6
    case PRELIMINARY_RESULT_CODE:  last_result = 0;     break;     // Type: 7
//    case INFORMATION_TXT:          last_result = 0;     break;     // Type: 3
    case RESULT_CODE:              last_result = 1;     break;     // Type: 1
    case UNSOLICITED_RESULT:       last_result = 2;     break;     // Type: 2
    default:
        tr_etm(TgTrCore,"ETM CORE: _at_atp_txt_cmd_rdy: cmd_tpye(%d) - FAILED", msg->cmd_type); 
        error = ETM_NOSYS;
    }

    if (last_result == 2) 
        goto etm_at_atp_txt_cmd_rdy_end;
    
    text = ((char *) msg->txt_cmd_p);
    length = strlen(text);
    etm_pkt_putdata(pkt, text, length);
    tr_etm(TgTrCore, "ETM CORE: _at_atp_txt_cmd_rdy: text(%s) length(%d)", text, length);

    // Status will be set to ETM_OK_MORE when more data is send.
    // Add one because of string length is also returned as part of the result
    pkt->status = (last_result ? ETM_OK : -ETM_OK_MORE);

    if (error < 0) {
//        tr_etm(TgTrCore,"ETM CORE: _at_atp_txt_cmd_rdy: ERROR(%d)", error); 
        pkt->status = -error;
    }

    etm_pkt_send(pkt);

etm_at_atp_txt_cmd_rdy_end:
    etm_free(pkt);

    return ETM_OK;
}


/******************************************************************************
 * ETM AT - Main Task 
 *****************************************************************************/

// Structur of protocol data dl-link: |target|at_cmd|
int etm_at(T_ETM_PKT *pkt, char *buf)
{
    int error = ETM_NOSYS;
    int sw_entity;

//    sw_entity = *buf++;

    // FIXME pkt should be use in etm_at_adapter()
    error = etm_at_adapter((char *) buf);

#if 0 
    switch (sw_entity) {
    case GSM: 
  
        break;
    case BLUE: 
        //error = etm_at_blue(*buf++); 
        break;
    default:
        tr_etm(TgTrCore,"ETM CORE: _at: ERROR(%d)", error); 
        error = ETM_NOSYS; 
    }
#endif

    return error;
}

