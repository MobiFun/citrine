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


#ifndef __ETM_API_H_
#define __ETM_API_H_

#include "etm_misc.h"
#include "etm_messages_i.h"

/******************************************************************************
 * ETM Packet
 *****************************************************************************/

typedef struct {	
    T_RV_HDR  header;
    char      size;
    uint8     data[255];
} T_ETM_MAIL;


/******************************************************************************
 * Messages
 *****************************************************************************/

/* Event return to entity */
#define ETM_DATA_READY              (ETM_MESSAGES_OFFSET | 0x010)
typedef struct
{
    T_RV_HDR  header;
    char      data[255];
} T_ETM_DATA_READY;


/******************************************************************************
 * Prototypes
 *****************************************************************************/

int etm_register(char name[], int mid, int task_id, T_RVF_ADDR_ID addr_id, ETM_CALLBACK_FUNC callback);
int etm_unregister(char name[], int mid, int task_id, T_RVF_ADDR_ID addr_id, ETM_CALLBACK_FUNC callback);

int etm_pkt_send(T_ETM_PKT *pkt);
int etm_pkt_putdata(T_ETM_PKT *pkt, const void *buf, int size);

int etm_get8(void *buf);
int etm_get16(void *buf);
int etm_get32(void *buf);

int etm_pkt_put8(T_ETM_PKT *p, int value);
int etm_pkt_put16(T_ETM_PKT *p, int value);
int etm_pkt_put32(T_ETM_PKT *p, int value);

#endif /* __ETM_API_H_ */
