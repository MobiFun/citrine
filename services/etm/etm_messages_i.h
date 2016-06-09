/********************************************************************************
 * Enhanced TestMode (ETM)
 * @file	etm_message_i.h 
 *
 * Data structures that ETM SWE can receive.
 *
 * These messages are send by the bridge function. There are not available
 * out of the SWE - Internaly messages.
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
 *
 * (C) Copyright 2003 by Texas Instruments Incorporated, All Rights Reserved
 *********************************************************************************/


#ifndef _ETM_MESSAGES_I_H_
#define _ETM_MESSAGES_I_H_

#include "../../riviera/rv/rv_general.h"
#include "../../riviera/rvm/rvm_use_id_list.h"


typedef int (*ETM_CALLBACK_FUNC)(uint8*, int);


/******************************************************************************
 * Constants
 *****************************************************************************/

#define ETM_NAME_MAX_LEN 10

#define ETM_MESSAGES_OFFSET BUILD_MESSAGE_OFFSET(ETM_USE_ID)


/******************************************************************************
 * Internal messages
 *****************************************************************************/

/**
 * @name ETM_REGISTER_REQ
 *
 * Internal message.
 *
 * Message issued by TMETM to ETM task.
 * This message is used to ...
 */
/*@{*/
/** Message ID. */
#define ETM_REGISTER_REQ (ETM_MESSAGES_OFFSET | 0x001)

/** Message structure. */
typedef struct 
{
    /** Message header. */
    T_RV_HDR  header;

    /** ETM specifics. */
    char                name[ETM_NAME_MAX_LEN];
    int                 mid;
    int                 task_id;
    T_RVF_ADDR_ID       addr_id;
    ETM_CALLBACK_FUNC	rx_callback_func;

}  T_ETM_REGISTER_REQ;
/*@}*/


/**
 * @name ETM_UNREGISTER
 *
 * Internal message.
 *
 * Message issued by TMETM to ETM task.
 * This message is used to ...
 */
/*@{*/
/** Message ID. */
#define ETM_UNREGISTER (ETM_MESSAGES_OFFSET | 0x002)

/** Message structure. */
typedef struct 
{
    /** Message header. */
    T_RV_HDR  header;
    /** ETM specifics. */
    char                name[ETM_NAME_MAX_LEN];
    int                 mid;
    int                 task_id;
    T_RVF_ADDR_ID       addr_id;      
    ETM_CALLBACK_FUNC	rx_callback_func;
    
}  T_ETM_UNREGISTER;
/*@}*/


/**
 * @name ETM_DATA_FWR
 *
 * Internal message.
 *
 * Message issued by TMETM to ETM task.
 * This message is used to ...
 */
/*@{*/
/** Message ID. */
#define ETM_DATA_FWR (ETM_MESSAGES_OFFSET | 0x003)

/** Message structure. */
typedef struct 
{
    /** Message header. */
    T_RV_HDR  header;

    /** ETM specifics. */
    int          size;
    char         mid;
    uint8        data[255];
    T_RV_RETURN  return_path;

}  T_ETM_DATA_FWR;
/*@}*/

/**
 * @name TM3_DATA_FWR
 *
 * Internal message.
 *
 * Message issued by TMETM to ETM task.
 * This message is used to ...
 */
/*@{*/
/** Message ID. */
#define ETM_TM3_DATA_FWR (ETM_MESSAGES_OFFSET | 0x004)

/** Message structure. */
typedef struct 
{
    /** Message header. */
    T_RV_HDR  header;
    /** ETM specifics. */
    int          size;
    char         cid;
    uint8        data[255];
    T_RV_RETURN  return_path;

}  T_ETM_TM3_DATA_FWR;
/*@}*/



#endif /* _ETM_MESSAGES_I_H_ */
