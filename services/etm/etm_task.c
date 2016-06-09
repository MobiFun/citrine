/********************************************************************************
 * Enhanced TestMode (ETM)
 * @file	etm.c
 *
 * @author	Kim T. Peteren (ktp@ti.com)
 * @version 0.1
 *


 *
 * History:
 *
 * 	Date       	Modification
 *  ------------------------------------
 *  16/06/2003	Creation
 *  03/03/2004  Upadted regarding the ATP + minor ETM DB handling updates
 *  28/07/2004  Fixed ETM database issue
 *
 * (C) Copyright 2003 by Texas Instruments Incorporated, All Rights Reserved
 *********************************************************************************/


#include "etm.h"
#include "etm_config.h"
#include "etm_api.h"
#include "etm_messages_i.h"
#include "etm_trace.h"
#include "etm_env.h"

#if ETM_AUDIO_SUPPORT
#include "audio/audio_api.h"
#endif

#include "../../riviera/rv/rv_general.h"
#include "../../riviera/rvf/rvf_api.h"
#include "../../riviera/rvt/rvt_gen.h"
#include "../../riviera/rvf/rvf_target.h" 
#include "../../riviera/rv/rv_defined_swe.h"

#include <string.h>

#if ETM_LCC_SUPPORT
    #include "lcc/lcc_api.h"
    #include "lcc/lcc_cfg_i.h"
    #include "lcc/lcc.h"
    #include "lcc/lcc_env.h"
    #include "lcc/lcc_trace.h"
#endif

// Version of the ETM TASK moved to etm_version.h
//const uint16 etm_task_revision = (1<<12) | (0x1);

/******************************************************************************
 * Globals 
 *****************************************************************************/

#define ETM_DB_MAX     16  // Limited of registered SWEntities into ETM DB
#define ETM_DB_NO_USE  0xFFFFFFFF  //

typedef struct
{	
    char                swe_name[ETM_NAME_MAX_LEN];
    int                 mid;
    int                 task_id;
    T_RVF_ADDR_ID       addr_id;    // Unique mailbox (ID) of the SWE which will
                                    //receive the message
    ETM_CALLBACK_FUNC	rx_callback_func;
} T_ETM_USER_DB;


static T_ETM_USER_DB etm_user_db[ETM_DB_MAX];

static int etm_db_counter = 0;


/******************************************************************************
 * Prototypes 
 *****************************************************************************/

extern T_ETM_ENV_CTRL_BLK *etm_env_ctrl_blk;
#if ETM_LCC_SUPPORT
extern T_PWR_CTRL_BLOCK *pwr_ctrl;
#endif

int etm_database_manager(T_RV_HDR *msg_p);
int etm_forward_packet(int mid, T_RV_HDR *msg);
void etm_error_packet_send(int mid, int error);
void etm_receive(unsigned char *inbuf, unsigned short size);

#if ETM_ATP_SUPPORT
extern int etm_at_atp_message(void *msg);
#endif

#if ETM_LCC_SUPPORT
extern int etm_pwr_ul(void *msg);
extern int etm_pwr_dl(T_ETM_PKT *pkt, uint8 *buf, int insize);
#endif

/******************************************************************************
 * Main Testmode Task Loop
 *****************************************************************************/

T_RV_RET etm_task(void)
{  
    extern int etm_core_init(void);
    #if ETM_AUDIO_SUPPORT
	extern int etm_audio_init(void);
    #endif
    #if ETM_LCC_SUPPORT
	extern int etm_pwr_init(void);
    #endif

    T_RV_HDR  *msg = NULL;
    T_ETM_PKT *pkt = NULL; 
    UINT32 start_time = 0;
    UINT32 end_time   = 0;
    UINT16 recv_event, i;  

    int status = RV_OK, buf_size;
    
    /* Entity registration to ETM */
    /* This should be in the individual SWE init. function*/
    status = etm_core_init();
#if ETM_AUDIO_SUPPORT
    status = etm_audio_init();
#endif
#if ETM_LCC_SUPPORT
    status = etm_pwr_init();
#endif

    while (1)
    {
        recv_event = rvf_wait(0xffff,0); /* Wait (infinite) for all events. */

        start_time = rvf_get_tick_count();
        tr_etm(TgTrEtmLow,"ETM: _task: Time Waiting (%d) tick", start_time - end_time);
        tr_etm(TgTrEtmLow,"ETM: _task: Time Start(%d)", start_time);

        tr_etm(TgTrEtmLow,"ETM: _task: Got message passing to bit filter (0x%x)", recv_event);


        if (!(recv_event & RVF_TASK_MBOX_0_EVT_MASK))
            continue;
        
        /* Read the message in the ETM mailbox */
        if ((msg = rvf_read_mbox(ETM_MAILBOX)) == NULL)
            continue;
        
        tr_etm(TgTrEtmLow,"ETM: _task: msg_id(0x%x)", msg->msg_id);
        
            switch (msg->msg_id) {
                /* Entity registration request or unregistration */
            case ETM_REGISTER_REQ:
            case ETM_UNREGISTER:
                status = etm_database_manager(msg);
                break;
                /* ETM packet received */
            case ETM_DATA_FWR:
                if ((status = etm_forward_packet(((T_ETM_DATA_FWR *) msg)->mid, msg))
                    != ETM_OK)
                    etm_error_packet_send(((T_ETM_DATA_FWR *) msg)->mid, status);
                break;
                /* TM3 packet received */
            case ETM_TM3_DATA_FWR:
                if ((status = etm_forward_packet(ETM_TM3, msg)) != ETM_OK)
                    etm_error_packet_send(ETM_TM3, status);
                break;
            default:
                tr_etm(TgTrEtmLow,"ETM: _task: msg_id '0x%x' NOT supported",
                       msg->msg_id);
            }

        if (status != ETM_OK) {
            tr_etm(TgTrFatal,"ETM: _task: mid(0x%x) ERROR(%d)", 
                   ((T_ETM_DATA_FWR *) msg)->mid, status);
            etm_free(msg); // Free the message
        }
        
        end_time = rvf_get_tick_count();
        tr_etm(TgTrEtmLow,"ETM: _task: Time End (%d)", end_time);
        tr_etm(TgTrEtmLow,"ETM: _task: Time Total (%d) tick", end_time - start_time);
    }
    
    return RV_OK;
}


void etm_error_packet_send(int mid, int error)
{
    T_ETM_PKT *pkt;

    tr_etm(TgTrFatal,"ETM: _error_packet_send: Module(0x%x) ERROR(%d)", mid, error); 

    if (error == ETM_NOMEM) {
        rvf_dump_mem();  
    }
    
    if ((pkt = (T_ETM_PKT *) etm_malloc(sizeof(T_ETM_PKT))) == NULL) {
        rvf_dump_mem();  
        return;
    }
    
    // Init. of return packet
    pkt->mid    = mid;
    pkt->status = -error;
    pkt->size   = 0;
    pkt->index  = 0;

    etm_pkt_send(pkt);
    etm_free(pkt); // Free return packet
}


/* Forwarding of DATA to the SWE can either be done by message/primitive or callback */
int etm_forward_packet(int mid, T_RV_HDR* msg)
{
    ETM_CALLBACK_FUNC rx_callback_func = NULL;
    int i, cid, status = ETM_OK;
    T_ETM_DATA_READY* message_p;
    T_RVF_ADDR_ID swe_addr_id = 0;

    // Search for supported MID in the table
    for (i = 0; etm_user_db[i].mid != TABLE_END; i++)
    {
        tr_etm(TgTrEtmLow,"ETM: _forward_packet: Lookup in db for mid(0x%x)", mid);
    
        if (etm_user_db[i].mid == mid) {
            rx_callback_func = etm_user_db[i].rx_callback_func;
            swe_addr_id = etm_user_db[i].addr_id;
            break;
        }
    }

    tr_etm(TgTrEtmLow,"ETM: _forward_packet: rx_callback_func(%d) swe_addr_id(%d)", 
           *rx_callback_func, swe_addr_id);

    // Invoke the SWE mailbox
    if (swe_addr_id) {
        /* Allocate the memory for the message to send */
        if ((message_p = (T_ETM_DATA_READY*) etm_malloc(sizeof(T_ETM_DATA_READY))) == NULL)
            return ETM_NOMEM; 

        /* Fill the header of the message */
        message_p->header.msg_id = ETM_DATA_READY;
        
        /* Fill the address source id */
        message_p->header.src_addr_id   = rvf_get_taskid(); 
        message_p->header.dest_addr_id  = etm_env_ctrl_blk->addr_id;
        message_p->header.callback_func = NULL;
        
        /* Fill the data in the message */
        memcpy(((T_ETM_DATA_READY*) message_p)->data, ((T_ETM_DATA_FWR*) msg)->data,
               ((T_ETM_DATA_FWR*) msg)->size);
        
        /* Send the message to the entity */
        if ((status = rvf_send_msg(swe_addr_id, message_p)) != RV_OK) {
            tr_etm(TgTrFatal,"ETM: _forward_packet: Failed to sent message - ERROR(%d)", 
                   status);
            return ETM_RV_FATAL;
        }
        etm_free(msg); // Free the message
    }
    // Invoke the SWE callback function
    else if (rx_callback_func) {
        tr_etm(TgTrEtmLow,"ETM: _forward_packet: to mid(0x%x)", mid);
        status = rx_callback_func (((T_ETM_DATA_FWR*) msg)->data, ((T_ETM_DATA_FWR*) msg)->size);
        rx_callback_func = NULL;
        if (status != ETM_OK)
            return status;        
        etm_free(msg); // Free the message
    }
    else {
        return ETM_NOSYS;
    }
    
    return ETM_OK;
}


/******************************************************************************
 * Get and Free buffer (Internal Functions)
 *****************************************************************************/

void *etm_malloc(int size)
{
    /* Memory bank status (red, yellow, green) */
    T_RVF_MB_STATUS mb_status;
    void *addr;

    mb_status = rvf_get_buf(etm_env_ctrl_blk->prim_id, size, &addr);

    /* The flag returned by rvf_get_buf is red, there is not enough
     * memory to allocate the buffer. */
    if (mb_status == RVF_RED) {
		tr_etm(TgTrFatal, "ETM: _malloc: Error to get memory");
        return NULL;
    }
    /* The flag is yellow, there will soon be not enough memory anymore. */
    else if (mb_status == RVF_YELLOW) {
	tr_etm(TgTrFatal, "ETM: _malloc: Getting short on memory");
    }

    tr_etm(TgTrEtmLow,"ETM: _malloc: size(%d) at addr(0x%x)", size, addr);
    return addr;
}

int etm_free(void *addr)
{
    int status;

    tr_etm(TgTrEtmLow,"ETM: _free: addr(0x%x)", addr);

    if ((status = rvf_free_buf(addr)) != RV_OK) {
        tr_etm(TgTrFatal, "ETM: _free: ERROR(%d)", status);
    }
    
    return ETM_RV_FATAL;
}


/******************************************************************************
 * ETM receive Functions API (Internal Functions)
 ******************************************************************************/

/* The input pointer buf point at a complete TM3 packet. */
int etm_tm3_data_forward(uint8 *buf, int size)
{
    /* Type for a registration event. */
    T_ETM_TM3_DATA_FWR *msg;

    tr_etm(TgTrEtmLow, "ETM: _tm3_data_forward: cid(0x%x) size(%d)", *buf, size);

    /* Allocate the memory for the message to send */
    if ((msg = (T_ETM_TM3_DATA_FWR*) etm_malloc(sizeof(T_ETM_TM3_DATA_FWR))) == NULL)
            return ETM_NOMEM; 

    /* Fill the message id */
    msg->header.msg_id        = ETM_TM3_DATA_FWR;

    /* Fill the address source id */
    msg->header.src_addr_id   = rvf_get_taskid();
    msg->header.dest_addr_id  = etm_env_ctrl_blk->addr_id;
    msg->header.callback_func = NULL;

    /* Fill the message parameters */
    msg->size = size;
    msg->cid  = *buf;
    memcpy(&msg->data, buf, size);

// At this point, all the data have been parsed and copied into
// the ETM primitive.  Now we send the primitive to the ETM task.
    if (rvf_send_msg(etm_env_ctrl_blk->addr_id, msg) != RV_OK) {
        tr_etm(TgTrFatal, "ETM: _tm3_data_forward: FAILED");
        return ETM_RV_FATAL;  // msg is auto freed by rvf_send_msg() if error
    }

    return ETM_OK;
}


/* The input pointer buf point at payload of the TM packet, minus mid and cksum. */
int etm_data_forward(char mid, uint8 *inbuf, int size)
{
    /* Type for a registration event. */
    T_ETM_DATA_FWR *msg;

    tr_etm(TgTrEtmLow, "ETM: _data_forward: mid(0x%x) size(%d)", mid, size);

    /* Allocate the memory for the message to send */
    if ((msg = (T_ETM_DATA_FWR*) etm_malloc(sizeof(T_ETM_DATA_FWR))) == NULL)
            return ETM_NOMEM; 

    /* Fill the message id */
    msg->header.msg_id        = ETM_DATA_FWR;

    /* Fill the address source id */
    msg->header.src_addr_id   = rvf_get_taskid(); 
    msg->header.dest_addr_id  = etm_env_ctrl_blk->addr_id;
    msg->header.callback_func = NULL;

    /* Fill the message parameters */
    msg->size = size;
    msg->mid  = mid;
    memcpy(&msg->data, inbuf, size);

// At this point, all the data have been parsed and copied into
// the ETM primitive.  Now we send the primitive to the ETM task.
    if (rvf_send_msg(etm_env_ctrl_blk->addr_id, msg) != RV_OK) {
        tr_etm(TgTrFatal, "ETM: _data_forward: FAILED");
        return ETM_RV_FATAL;  // msg is auto freed by rvf_send_msg() if error
    }

    return ETM_OK;
}


/* This function is registred in the RVT module as the TestMode receive function */
/* It's called every time a TestMode packet is received on the UART and the */
/* data is forwarded to the ETM Entity via a message/primitiv */
/* The function is a callback func. used by the RVT TASK -> UART RX. */

void etm_receive(uint8 *inbuf, unsigned short size)
{
    int error = ETM_NOSYS, i, index;
    char mid;
    unsigned char cksum;
    T_ETM_PKT *pkt;

    tr_etm(TgTrEtmLow, "ETM: _receive: inbuf size(%d)", size);

    // Copy data payload size (size minus MID/CID byte and checksum byte)
    mid = *inbuf++;

    cksum = mid;
    for (i = 0; i < size - 1; i++) {
        cksum ^= inbuf[i];
    }
    
    if (cksum != 0) {
        error = ETM_PACKET;
        goto ETM_RECEIVE_END;
    }
    
    // Check it's a TM3 packet
    if ((0x20 <= mid && mid < 0x27) ||
        (0x30 <= mid && mid < 0x3A) ||
        (0x40 <= mid && mid < 0x49) ||
        (0x50 <= mid && mid < 0x57)) {
        // Forward complete TM3 packet
        error = etm_tm3_data_forward(--inbuf, size);
    }
    else {
        /* Controlling of receptor for regisration */
        for (index=0; index < etm_db_counter; index++)
        {
            if (etm_user_db[index].mid == mid) {
                // Forward ETM packet without <mid> and <cksum>, -2 in size
                error = etm_data_forward(mid, inbuf, size - 2);
                break;
            }
            else if ((index == etm_db_counter) && (etm_user_db[index].mid != mid)) {
                tr_etm(TgTrFatal, "ETM: _receive: mid(0x%x) not supported", mid);
            }
        }
    }

ETM_RECEIVE_END:
    /* Fill in Error status in ETM packet and send */
    if (error) {
        if (error == ETM_NOMEM) {
            rvf_dump_mem();
            return;
        }

        if ((pkt = (T_ETM_PKT *) etm_malloc(sizeof(T_ETM_PKT))) == NULL)
            return; 
        pkt->size   = 0;
        pkt->mid    = mid;
        pkt->status = -error;
        
        etm_pkt_send(pkt);
        etm_free(pkt); // Free return Packet
    }
}


/******************************************************************************
 * Registration manager Functions API (Internal function)
 ******************************************************************************/

int etm_database_add(T_ETM_REGISTER_REQ *msg_p, int index)
{

    memcpy(etm_user_db[index].swe_name, msg_p->name, strlen(msg_p->name));
    etm_user_db[index].mid              =  msg_p->mid;
    etm_user_db[index].task_id          =  msg_p->task_id;
    etm_user_db[index].addr_id          =  msg_p->addr_id;
    etm_user_db[index].rx_callback_func =  msg_p->rx_callback_func;

    etm_user_db[index+1].mid            =  TABLE_END;

    return ETM_OK;
}


int etm_database_manager(T_RV_HDR *msg_p)
{
    int index, mid, status;

    if (msg_p->msg_id == ETM_REGISTER_REQ) {
        mid = ((T_ETM_REGISTER_REQ *) msg_p)->mid;
        
        tr_etm(TgTrEtmLow,"ETM: _database_manager: _REGISTER_REQ reguest is received from (%s)", 
               ((T_ETM_REGISTER_REQ *) msg_p)->name); 
        
        /* Lookup in the ETM DB array */
        for (index=0; index < ETM_DB_MAX; index++)
        {
            /* Use unregistrered space */
            if ((etm_user_db[index].mid == ETM_DB_NO_USE) && (etm_user_db[index].addr_id == 0) && 
                (etm_user_db[index].rx_callback_func == NULL)) {
                status = etm_database_add((T_ETM_REGISTER_REQ*) msg_p, index);
                etm_db_counter++;
                etm_free(msg_p); // Free Message
                return ETM_OK;
            }
            /* Reject double registration */
            else if ((etm_user_db[index].mid == mid) &&
                     ((etm_user_db[index].addr_id != 0) || (etm_user_db[index].rx_callback_func != NULL))) {
                tr_etm(TgTrFatal,"ETM: _database_manager: The Module(0x%x) is registrered", mid);
                etm_free(msg_p); // Free Message
                return ETM_OK;
            }
        }
                    
        /* Add the entity to the etm database */
        if (etm_db_counter < ETM_DB_MAX) {
            status = etm_database_add((T_ETM_REGISTER_REQ*) msg_p, etm_db_counter);
            etm_db_counter++;
            etm_free(msg_p); // Free Message
            return ETM_OK;
        }
        
        etm_free(msg_p); // Free Message
        return ETM_DB_LIMIT;
    }
    
    
    if (msg_p->msg_id == ETM_UNREGISTER) {
        mid = ((T_ETM_UNREGISTER *) msg_p)->mid;
        tr_etm(TgTrEtmLow,"ETM: _database_manager: _UNREGISTER reguest is received from (%s)",
               ((T_ETM_REGISTER_REQ *) msg_p)->name);
        
        /* Lookup in the array, if the SWE is stocked then clean it*/
        for (index=0; index < ETM_DB_MAX; index++) {
            if (etm_user_db[index].mid == mid) {
                etm_user_db[index].mid              = ETM_DB_NO_USE;
                etm_user_db[index].addr_id          = 0;
                etm_user_db[index].rx_callback_func = NULL;
                etm_db_counter--;
                etm_free(msg_p); // Free Message
                return ETM_OK;
            }			
        }

        etm_free(msg_p); // Free Message
        return ETM_INVAL;
    }
    
    etm_free(msg_p); // Free Message
    return ETM_OK;
}
