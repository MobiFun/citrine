/********************************************************************************
 * Enhanced TestMode (ETM)
 * @file	etm_api.c
 *
 * API for ETM SWE.
 *
 * @author	Kim T. Peteren (ktp@ti.com)
 * @version 0.1
 *

 *
 * History:
 *
 * 	Date       	Modification
 *  ------------------------------------
 *  11/06/2003	Creation
 *
 * (C) Copyright 2003 by Texas Instruments Incorporated, All Rights Reserved
 *********************************************************************************/


#include "etm.h"
#include "etm_env.h"
#include "etm_messages_i.h"
#include "etm_trace.h"

#include "etm_misc.h"

#include "../../riviera/rvf/rvf_api.h"

#include <string.h> 

extern T_ETM_ENV_CTRL_BLK *etm_env_ctrl_blk;


/********************************************************************************
 * Registers the SWE to the ETM instance.
 *
 * This is a bridge function. It sends the T_ETM_REGISTRATION_REQ
 * message to the ETM SWE.
 * It is the first function that should be called.
 *
 * @return	ETM_NOMEM in case of a memory error,
 *			the return value of rvf_send_msg otherwise.
 *********************************************************************************/

int etm_register(char name[], int mid, int task_id, T_RVF_ADDR_ID addr_id,
		 ETM_CALLBACK_FUNC callback)
{
    /* Type for a registration event. */
    T_ETM_REGISTER_REQ *etm_registration_p;

    tr_etm(TgTrEtmLow, "ETM API: _register bridge function(%s)", name);

    /* Allocate the memory for the message to send */
    if ((etm_registration_p = (T_ETM_REGISTER_REQ*)
				etm_malloc(sizeof(T_ETM_REGISTER_REQ))) == NULL)
            return ETM_NOMEM; 

    /* Fill the message id */
    etm_registration_p->header.msg_id        = ETM_REGISTER_REQ;
 
    /* Fill the address source id */
    etm_registration_p->header.src_addr_id   = rvf_get_taskid(); 
    etm_registration_p->header.dest_addr_id  = etm_env_ctrl_blk->addr_id;
    etm_registration_p->header.callback_func = NULL;

    /* Fill the message parameters */
    memcpy(etm_registration_p->name, name, strlen(name));
    etm_registration_p->mid              = mid;
    etm_registration_p->task_id          = task_id;
    etm_registration_p->addr_id          = addr_id;
    etm_registration_p->rx_callback_func = callback; 

    /* Send the message using mailbox. */
    return rvf_send_msg(etm_env_ctrl_blk->addr_id, (void*) etm_registration_p);
}


/********************************************************************************
 * Cleans ETM register tables, i.e. set/clean the variable at their
 * initialization state for a specific entity.
 * This function can be used to reinitialize ETM register database without
 * having to start/stop it.
 *
 * This is a bridge function. It sends ETM_UNREGISTER message to ETM.
 *
 * @return	ETM_NOMEM in case of a memory error,
 *			the return value of rvf_send_msg otherwise.
 *********************************************************************************/

int etm_unregister(char name[], int mid, int task_id, T_RVF_ADDR_ID addr_id, ETM_CALLBACK_FUNC callback)
{
    /* Type for a start input event. */
    T_ETM_UNREGISTER *etm_unregister_p;

    tr_etm(TgTrEtmLow, "ETM API: _unregister bridge function");

    /* Allocate the memory for the message to send */
    if ((etm_unregister_p = (T_ETM_UNREGISTER*)
				etm_malloc(sizeof(T_ETM_UNREGISTER))) == NULL)
            return ETM_NOMEM; 

    /* Fill the message id */
    etm_unregister_p->header.msg_id        = ETM_UNREGISTER;
 
    /* Fill the address source id */
    etm_unregister_p->header.src_addr_id   = rvf_get_taskid(); 
    etm_unregister_p->header.dest_addr_id  = etm_env_ctrl_blk->addr_id;
    etm_unregister_p->header.callback_func = NULL;

    /* Fill the message parameters */
    memcpy(etm_unregister_p->name, name, strlen(name));
    etm_unregister_p->mid              = mid;
    etm_unregister_p->task_id          = task_id;
    etm_unregister_p->addr_id          = addr_id;
    etm_unregister_p->rx_callback_func = callback;

    /* Send the message using mailbox. */
    return rvf_send_msg(etm_env_ctrl_blk->addr_id, (void*) etm_unregister_p);
}


/********************************************************************************
 * Get data from a ETM packet structur. Get either 8, 16 or 32 value
 * Used to unpack data 
 * 
 * This is helpers
 *
 * @return	the return value of the point of the ETM packet.
 *********************************************************************************/

int etm_get8(void *buf)
{
    unsigned char *p = buf;

    int value = *p;

    tr_etm(TgTrEtmLow, "ETM API: _get8(%d)", value);

    return value;
}

int etm_get16(void *buf)
{
    unsigned char *p = buf;

    int value = (p[0] | (p[1] << 8));

    tr_etm(TgTrEtmLow, "ETM API: _get16(%d)", value);

    return value;
}

int etm_get32(void *buf)
{
    unsigned char *p = buf;
    int value = 0;
   
    value = *p;
    p++;
    value |= (*p << 8);
    p++;
    value |= (*p << 16);
    p++;
    value |= (*p << 24);

    tr_etm(TgTrEtmLow, "ETM API: _get32(%d)", value);

    return value;
}


/********************************************************************************
 * Put data into a ETM packet structur. Put either 8, 16 or 32 value
 * Used to pack data 
 * 
 * This is helpers
 *
 * @return	Return ETM_PACKET of ETM_OK.
 *********************************************************************************/

#define max_ul_data_size 240

int etm_pkt_put8(T_ETM_PKT *pkt, int value)
{
    tr_etm(TgTrEtmLow, "ETM API: _pkt_put8(*, %d)", value);

    if (pkt->index + 1 > max_ul_data_size) 
        return ETM_PACKET;
    
    pkt->data[pkt->index] = value;
    pkt->index += 1;

    pkt->size += 1;

    return ETM_OK;
}

int etm_pkt_put16(T_ETM_PKT *pkt, int value)
{
    tr_etm(TgTrEtmLow, "ETM API: _pkt_put16(*, %d)", value);

    if (pkt->index + 2 > max_ul_data_size) 
        return ETM_PACKET;

    memcpy(&pkt->data[pkt->index], &value, 2);
    pkt->index += 2;

    pkt->size += 2;

    return ETM_OK;
}

int etm_pkt_put32(T_ETM_PKT *pkt, int value)
{
    tr_etm(TgTrEtmLow, "ETM API: _pkt_put32(*, %d)", value);

    if (pkt->index + 4 > max_ul_data_size) 
        return ETM_PACKET;

    memcpy(&pkt->data[pkt->index], &value, 4);
    pkt->index += 4;

    pkt->size += 4;

    return ETM_OK;
}

int  etm_pkt_putdata(T_ETM_PKT *pkt, const void *buf, int size)
{
    tr_etm(TgTrEtmLow, "ETM API: _pkt_putdata(*, %d)", size);

    if (pkt->index + size > max_ul_data_size)
        return ETM_PACKET;
    memcpy(&pkt->data[pkt->index], buf, size);
    pkt->index += size;

    pkt->size += size;
  
    return ETM_OK;
}


/********************************************************************************
 * This function is used to send an ETM Packet to the PC
 *
 * This is helpers
 *
 * @return	Return value of rvf_send_trace_cpy.
 *********************************************************************************/

int etm_pkt_send(T_ETM_PKT *pkt)
{
    extern unsigned char etm_trace_user_id; 
    uint8 *buf, cksum = 0;
    uint16 sendsize, size;
    int error = ETM_OK;

    buf = (uint8 *) &pkt->mid;
    sendsize = size = pkt->size + 2; //one for mid, one for status

    tr_etm(TgTrEtmLow, "ETM API: _pkt_send: size(%d)", sendsize);

    while (size-- ) {
        cksum ^= *buf++;
    }
    *buf = cksum;

    sendsize += 1; // one for checksum

    tr_etm_hexdump(TgTrEtmLow, &pkt->mid, sendsize);

    error = rvt_send_trace_cpy((uint8 *) &pkt->mid, etm_trace_user_id,
                               sendsize, RVT_BINARY_FORMAT);
    if(error < 0)
        tr_etm(TgTrFatal, "ETM API: _pkt_send: ERROR(%d)", error);

    return error;
}
