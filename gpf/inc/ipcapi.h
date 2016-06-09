/* 
+----------------------------------------------------------------------------- 
|  Project : PCO
|  Modul   : inc\ipcapi.h
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
|  Purpose :  API definition for IPC
+----------------------------------------------------------------------------- 
*/ 
#ifndef IPCAPI_H
#define IPCAPI_H

#include "cms.h"

#undef EXPORT
#include "typedefs.h"

/* define extra function codes for driver ipc */
#define IPC_SELF        10
#define IPC_INITCH      11
#define IPC_EXITCH      12
#define IPC_OPENCH      13
#define IPC_CLOSECH     14
#define IPC_READCH      15
#define IPC_WRITECH     16
#define IPC_CREATEMSG   17
#define IPC_PARSEMSG    18
#define IPC_ISMYADDR    19
#define IPC_GETHANDLE   20
#define IPC_SETTIME     21


/**********************************************************************
 * macros
 *********************************************************************/

#define MSG_ID( Group, Code )     (((Group) << 8) + ((Code) & 0xFF))


/**********************************************************************
 * defines
 *********************************************************************/

/* return codes                                                      */
#define IPC_OK             CMS_OK      
#define IPC_ERROR          CMS_ERROR   
#define IPC_EXIST          CMS_EXIST      
#define IPC_FULL           CMS_FULL          
#define IPC_BADHANDLE      CMS_BADHANDLE     
#define IPC_EMPTY          CMS_EMPTY   
#define IPC_SIZE           CMS_SIZE    
#define IPC_TIMEOUT        CMS_TIMEOUT
#define IPC_NOMEM          CMS_NOMEM
#define IPC_INVALID        (-20)
#define IPC_MISALIGNED     (-21)



#define IPC_MAX_PATH_SIZE  120

/* msg IDs: */
#define IPC_GENERATED      MSG_ID(IPC,1)

#define ALIGNMENT         1
#define MSG_MAX_SIZE    512

/**********************************************************************
 * types
 *********************************************************************/


typedef int IPC_HANDLE;

typedef struct
{
   char *pcSender;
   char *pcReceiver;
   void *pvBuffer;
   U32   ulTime;
   U16   uwTenthOfMS;
   U16   uwSize;
   U16   uwID;
} MSG_HEADER;



#if   ALIGNMENT == 1
   typedef U8  MSG_BUFFER [MSG_MAX_SIZE];
#elif ALIGNMENT == 2
   typedef U16 MSG_BUFFER [(MSG_MAX_SIZE + 1) / 2];
#elif ALIGNMENT == 4
   typedef U32 MSG_BUFFER [(MSG_MAX_SIZE + 3) / 4];
#else
   #error "invalid alignment"
#endif


/* standard funtions: */


/*********************************************************************/
U16 ipc_createMsg (     /* @func Create a message.                   */
   void       *pvBuffer,/* @parm Buffer for the message to store.    */
   U16         uwSize,  /* @parm Size of the buffer.                 */
   MSG_HEADER  Msg      /* @parm Settings for the message to create. */
   );                   /* @returnvalue One of the values below.     */
/*---------------------------------------------------------------------
 * @description      The function creates a message out of the settings
 *                   that are stored in the structure. The resulting
 *                   message will be written to the buffer.
 *
 *                   Attention: The address of the buffer MUST be
 *                              aligned (multiple of ALIGNMENT)
 *
 *                   Sender and Receiver will be inserted in normalized
 *                   form (= absolute address) !
 *
 * @tablex           Return values:                                   |
 *                   Value          Description
 *                   --------------------------------------------------
 *                   > 0            Size of the message.
 *                   = 0            Buffer too small, invalid settings.
 *
 * @tablex           Content of pHdr:                                 |
 *                   Setting        Description
 *                   -------------- -----------------------------------
 *                   pcSender       NULL means that the calling process
 *                                  is the sender. Otherwise the string
 *                                  will be inserted into the message.
 *                   pcReceiver     Name of the receiver (string)
 *                                  or IPC_HANDLE of the receiver.
 *                   pvBuffer       Pointer to the data to append to
 *                                  the message.
 *                   uwSize         Size of the data to append to the
 *                                  message.
 *                   uwID           Message group and code. Use the
 *                                  macro MSG_ID(Group,Code) to set the
 *                                  value.
 *                   ulTime         Automatically set.
 *                   uwTenthOfMS    Automatically set.
 *********************************************************************/


/*********************************************************************/
S16 ipc_parseMsg (      /* @func Parse a message.                    */
   MSG_HEADER *pMsg,    /* @parm Structure to store the settings.    */
   void       *pvBuffer,/* @parm Buffer containing the message.      */
   U16         uwSize   /* @parm Size of pvBuffer.                   */
   );                   /* @returnvalue One of the values below.     */
/*---------------------------------------------------------------------
 * @description      The function parses a buffer that contains a
 *                   a message and writes all settings to the
 *                   structure.
 *
 *                   Attention: The address of the buffer MUST be
 *                              aligned (multiple of ALIGNMENT)
 *
 * @tablex           Return values:                                   |
 *                   Value          Description
 *                   --------------------------------------------------
 *                   IPC_INVALID    Invalid arguments or message.
 *                   IPC_OK         Success.
 *
 * @tablex           Content of pHdr:                                 |
 *                   Setting        Description
 *                   -------------- -----------------------------------
 *                   pcSender       Address of the sender.
 *                   pcReceiver     Address of the receiver (normally
 *                                  the process receiving the message).
 *                   pvBuffer       Pointer to the data of the message.
 *                   uwSize         Size of the data.
 *                   uwID           Message group and code. Use the
 *                                  macro MSG_ID(Group,Code) to compare
 *                                  the value with other IDs.
 *                   ulTime         Time [ms from 01.01.1970 GMT] of
 *                                  message's creation.
 *                   uwTenthOfMS    The tenth of [ms] of the message's
 *                                  creation.
 *********************************************************************/

#endif   /* IPCAPI_H */

