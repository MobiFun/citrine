/* 
+----------------------------------------------------------------------------- 
|  Project :  PCO2
|  Modul   :  inc\pco_const.h
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
|  Purpose :  This header provides defines for all the messages send
|             between pco components
+----------------------------------------------------------------------------- 
*/ 


#ifndef _PCO_CONST_H_
#define _PCO_CONST_H_

/*==== INCLUDES ==================================================*/
#include "ipcapi.h"

/*==== GENERAL CONSTANTS ==================================================*/
enum T_PCOSTATUS {PCO_RUNNING,PCO_STOPPED,PCO_LOGFILE};

#define PCO_SEND_TIMEOUT    10000

#define PCO_TTYPE_UNKNOWN   0
#define PCO_TTYPE_MS        1 // milliseconds since reset
#define PCO_TTYPE_HMSM      2 // current time of day in ms
#define PCO_TTYPE_FRAME     3 // frame number since reset

#define CTRL_MSG_MAX_SIZE   MSG_MAX_SIZE /* from ipc */
#define CTRL_DATA_MAX_SIZE  CTRL_MSG_MAX_SIZE-sizeof(MSG_HEADER)

#define DATA_MSG_MAX_SIZE   65535
#define DATA_MSG_DEF_SIZE   2300

#define MAX_QNAME_LEN     50
#define MAX_PATH_LEN      2300
#define MAX_ENTITY_NLEN   5

#define MAX_PRIM_NAME     30

#define PCO_DEF_SRV_NAME  "PCOS"

#ifdef _DEBUG
#define PCO_CFG_PATH "../../cfg/"
#else
#define PCO_CFG_PATH "../cfg/"
#endif

#ifdef _WIN32
#define DEF_CCDDATA_PATH  "ccddata_dll.dll"
#endif


#define PCO_MAX_TRACECLASS  32
const char PCO_TC_NAME[PCO_MAX_TRACECLASS][MAX_PRIM_NAME]=
{
  "FUNCTION TRACE",
  "EVENT TRACE",
  "PRIMITIVE TRACE",
  "STATE TRACE",
  "SYSTEM TRACE",
  "INTERRUPT-SIGNAL TRACE",
  "ERROR TRACE",
  "CCD TRACE",
  "TIMER TRACE",
  "PROFILER TRACE",
  
  "<TRACE>",
  "<TRACE>",
  "<TRACE>",
  "<TRACE>",

  "USER1 TRACE",
  "USER2 TRACE",
  "USER3 TRACE",
  "USER4 TRACE",
  "USER5 TRACE",
  "USER6 TRACE",
  "USER7 TRACE",
  "USER8 TRACE",

  "<TRACE>",
  "<TRACE>",
  "<TRACE>",
  "<TRACE>",
  "<TRACE>",
  "<TRACE>",
  "<TRACE>",
  "<TRACE>",
  "<TRACE>",
  "<TRACE>"
};

/*==== PCO CONTROL MESSAGES ==================================================*/
#define MSG_ID( Group, Code )     (((Group) << 8) + ((Code) & 0xFF))

#define PCO_SRV_GROUP   0x1E
#define PCO_VIEW_GROUP  0x1F



#define PCO_CONNECT      		MSG_ID(PCO_SRV_GROUP,0x01) /* L3SRV_CONNECT */
/*
data:
char[]		name of CMS-queue in which the viewer wants to receive traces/primitives
          (zero terminated)

purpose:
sent from viewer to server to establish connection, server adds client to its list
*/

#define PCO_DISCONNECT     	MSG_ID(PCO_SRV_GROUP,0x02) /* L3SRV_DISCONNECT */
/*
data:

purpose:
sent from viewer to server to disconnect, server removes client from its list
*/

#define PCO_SUBSCRIBE     	MSG_ID(PCO_SRV_GROUP,0x03) /* L3SRV_SUBSCRIBE */
/*
data:
char[]		mobile ID (may be empty) (zero terminated)

purpose:
sent from viewer to server to receive live data of a dedicated mobile
*/


#define PCO_UNSUBSCRIBE     	MSG_ID(PCO_SRV_GROUP,0x04) /* L3SRV_UNSUBSCRIBE */
/*
data:

purpose:
sent from viewer to server to stop receiving of live data
*/


/* MSG_ID(PCO_SRV_GROUP,0x05) L3SRV_ENABLE_LOGGING */



#define PCO_OPEN_LOGFILE     MSG_ID(PCO_SRV_GROUP,0x06) /* L3SRV_LOAD_LOG_FILE */
/*
data:
char[]		name of sessionfile with full path or just a session name (zero terminated)
LONG		  first entry to be sent
LONG		  last entry to be sent

purpose:
sent from viewer to server to receive logged data of a specified session
sent from controller to open a session logfile -> new state of server: PCO_LOGFILE. 
*/


#define PCO_COPY_LOGFILE     MSG_ID(PCO_SRV_GROUP,0x07) /* L3SRV_COPY_LOG_FILE */
/*
data:
char[]		name of source sessionfile with full path (zero terminated)
char[]		name of destination sessionfile with full path (zero terminated)
LONG		  first entry to be copied
LONG		  last entry to be copied

purpose:
sent from viewer to server to make it copy a specified session into another
sessionsfile while applying the filter currently set
*/


#define PCO_SET_FILTER MSG_ID(PCO_SRV_GROUP,0x08)  /* L3SRV_SET_SERVER_FILTER */
/*
data:
char[] list   ... zero separated entity names ("\0\0"==end)
                  e.g.: "MM\0RR\0SS\0\0" or "+"MM\0RR\0\0"
                  - first entity=="+" -> only this entities will be forwarded
                  - default: specified entities will not be forwarded
prim_trace    ... optional parameter to disable/enable general forwarding of
                  primitives/traces:
                  0 .. fowarding of everything (default)
                  1 .. no primitives
                  2 .. no traces

purpose:
sent from viewer to server to set the entity filter for this viewer
*/


/* MSG_ID(PCO_SRV_GROUP,0x09) L3SRV_SET_MOBILE_FILTER */

/* MSG_ID(PCO_SRV_GROUP,0x0A) L3SRV_GET_MOBILE_FILTER */


#define PCO_EXIT		MSG_ID(PCO_SRV_GROUP,0x0B) /* L3SRV_EXIT */
/*
data:

purpose:
sent to server to make it exit, server will send this message 
to all connected viewers before exiting
*/


/* MSG_ID(PCO_SRV_GROUP,0x0C) L3SRV_LOG_FILE */



#define PCO_DATA		MSG_ID(PCO_SRV_GROUP,0x0D) /* L3SRV_DATA */
/*
data:
  rawdata ... depends on server type

purpose:   
sent from some servers to viewers (others send data without header)
contains rawdata which has to be interpreted depending on server type
*/


/* MSG_ID(PCO_SRV_GROUP,0x0E) L3SRV_FILTER */


#define PCO_CONNECTED		MSG_ID(PCO_SRV_GROUP,0x0F) /* L3SRV_CONNECTED */
/*
data:
  byte ... server type id (see server types)

purpose:   
sent from server to viewer to inform about an established connection and 
to tell its type
*/


#define PCO_OK		MSG_ID(PCO_SRV_GROUP,0x10) /* L3SRV_OK */
/*
data:
U16 	ID of message which will be confirmed by this PCO_OK

purpose:
sent by a receiver to the sender to confirm receiving and correct interpretation of a control message
*/


#define PCO_ERROR	MSG_ID(PCO_SRV_GROUP,0x11) /* L3SRV_ERROR */
/*
data:
U16  	ID of message which has produced the error
byte  error code (see error codes)

purpose:
sent by a receiver to the sender to inform about an error a received control message has raised
*/



#define PCO_STOP_TESTSESSION 	MSG_ID(PCO_SRV_GROUP,0x12)
/*
data:

purpose:   
sent to the server to stop a running testsession
*/

#define PCO_START_TESTSESSION   MSG_ID(PCO_SRV_GROUP,0x13)
/*
data:
Char[]	session name (zero terminated)
USHORT  (optional)
        0 .. don't create dbg-files
        1 .. create dbg-files

purpose:
sent to the server to start a new testsession
*/

#define PCO_CLOSE_LOGFILE		MSG_ID(PCO_SRV_GROUP,0x14)
/*
data:

purpose:
sent to server to disable logfile-mode -> new state PCO_STOPPED. 
*/

#define PCO_GET_LOGFILE_DATA	MSG_ID(PCO_SRV_GROUP,0x15)
/*
data:
ULONG	  start index
ULONG 	end index

purpose:
sent to server (which has to be in PCO_LOGFILE state) to make 
him forwarding all logged data from the current session 
file (which matches the index constraints) to the sender 
(has to be a connected viewer). 
*/

#define PCO_SEND_PRIM		    MSG_ID(PCO_SRV_GROUP,0x16)
/*
data:
char[] 	receiver (zero terminated)
char[]	text (zero terminated)

purpose:
to request server to send a CONFIG-primitive to TST, not supported by all servers (knowledge of FRAME is necessary)
*/


#define PCO_STATUS		MSG_ID(PCO_SRV_GROUP,0x17)
/*
data:
<none>         .. if sent from controller to server

T_PCOSTATUS	.. current status of server (see T_PCOSTATUS)
Char[]	.. name of testsession (can be empty, zero terminated)

purpose:
sent from controller to server to request its current status,
sent from server to controller to publish its current status
*/

#define PCO_GET_TESTSESSIONS MSG_ID(PCO_SRV_GROUP,0x18)
/*
data:

purpose:
sent from controller to server to acquire a list of available session names (which are stored in current server testsession directory)
*/

#define PCO_TESTSESSIONS MSG_ID(PCO_SRV_GROUP,0x19)
/*
data:
char[] 	.. zero separated testsession names ("\0\0"==end)
USHORT  .. 1 - more messages will follow
           0 - last messages with testsessions

purpose:
reply to PCO_GET_TESTSESSIONS
*/


#define PCO_SET_SESSIONPATH MSG_ID(PCO_SRV_GROUP,0x1A)
/*
data:
char[]  .. zero terminated path-string
USHORT  .. 1 - store new path in ini-file
           0 - don't store new path
           (optional, default is 0)

purpose:
sent to server to set new session path
*/

#define PCO_DISTRIB_LOGFILE MSG_ID(PCO_SRV_GROUP,0x1B)
/*
data:
LONG		  first entry to be sent
LONG		  last entry to be sent

purpose:
sent to server (in logfile mode) to make him send logged data to all clients
*/

#define PCO_GET_SESSIONPATH MSG_ID(PCO_SRV_GROUP,0x1C)
/*
data:

purpose:
sent to server to get its current session path
*/

#define PCO_SESSIONPATH MSG_ID(PCO_SRV_GROUP,0x1D)
/*
data:
char[]  .. zero terminated path-string

purpose:
sent from server as reply to PCO_GET_SESSIONPATH
*/

#define PCO_SET_TIME_STAMP_PERIOD MSG_ID(PCO_SRV_GROUP,0x1E)
/*
data:
UINT  .. new value for time stamp period
                       0 ... no time stamps
                       >0 .. period in minutes

purpose:
sent to server to set new time stamp period value
*/

#define PCO_RENAME_LOGFILE MSG_ID(PCO_SRV_GROUP,0x1F)
/*
data:
char[]		original name of sessionfile (evtl. with full path, zero terminated)
char[]		new name of sessionfile (evtl. with full path, zero terminated)

purpose:
sent to server to make it rename a session logfile
*/

#define PCO_GET_LOGFILE_INFO  MSG_ID(PCO_SRV_GROUP,0x20) 
/*
data:
Char[]	name of sessionfile with full path or just a session name (zero terminated)

purpose:
sent to server to request info's (e.g. count of entries) about the specified logfile 
*/

#define PCO_LOGFILE_INFO  MSG_ID(PCO_SRV_GROUP,0x21) 
/*
data:
LONG		  count of entries in logfile specified in last PCO_GET_LOGFILE_INFO
Char[]	  optional name of ccddata-file used during logging (zero terminated)
ULONG     optional str2ind version
Char[]	  optional name of str2ind-table-file used during logging (zero terminated)

purpose:
sent from server to a sender of PCO_GET_LOGFILE_INFO 
*/


#define PCO_TO_FRONT MSG_ID(PCO_VIEW_GROUP,0x01) /* L3VWR_TO_FRONT */
/*
data:

purpose:
sent from GUI-controller to server to indicate activation of ctrl window
sent from server to all viewers to indicate activation of ctrl window
sent from server back to controller to indicate activation of viewers
*/

#define PCO_SYNCHRONIZE MSG_ID(PCO_VIEW_GROUP,0x05) 
/*
data:
ULONG	  time in ms

purpose:
sent from a viewer A to server to indicate change in the view to a new time stamp
sent from server to all viewers except A to synchronize them with A
*/

#define PCO_LOGFILE_COMPLETE MSG_ID(PCO_VIEW_GROUP,0x06) 
/*
data:

purpose:
sent from server to a viewers after complete forwarding of a requested logfile
*/

#define PCO_INIFILE_CHANGED MSG_ID(PCO_VIEW_GROUP,0x07) 
/*
data:

purpose:
sent from viewer to server and then to all other viewers after an ini-file change
which should be handled immediatly by all viewers
*/

#define PCO_CLEAN MSG_ID(PCO_VIEW_GROUP,0x08) 
/*
data:

purpose:
sent from viewer to server and then to all other viewers to inform them that the user
has cleaned the viewer content
*/

#define PCO_STOP_LOGFILE 	MSG_ID(PCO_SRV_GROUP,0x22)
/*
data:
Char[]	queue name (zero terminated)

purpose:   
sent to the server to stop logging into the logfile connected with given queue 
*/

#define PCO_START_LOGFILE   MSG_ID(PCO_SRV_GROUP,0x23)
/*
data:
Char[]	logfile name (zero terminated)
Char[]	queue name (zero terminated)

purpose:
sent to the server to start logging all data which will be received in the given queue
(the queue will be created by the server)
*/

#define PCO_LOAD_CCDDATA  MSG_ID(PCO_SRV_GROUP,0x24) 
/*
data:
Char[]	name of ccddata-file to be loaded (zero terminated)

purpose:
sent to server to command it to load the specified ccddata-DLL
*/


/*==== PCO ERROR CODES ==================================================*/

#define PCO_ERR_NONE                  0x00

#define PCO_ERR_TOO_MANY_VIEWERS      0x01
#define PCO_ERR_ALREADY_CONNECTED     0x02
#define PCO_ERR_ALREADY_SUBSCRIBED    0x03
#define PCO_ERR_NOT_CONNECTED         0x04
#define PCO_ERR_NOT_SUBSCRIBED        0x05
#define PCO_ERR_FILE_OPEN_ERROR       0x06
//0x07: log file not for this viewer
#define PCO_ERR_FILE_READ_ERROR       0x08
#define PCO_ERR_FILE_WRITE_ERROR      0x09

#define PCO_ERR_TSESSION_NOT_RUNNING  0x10
#define PCO_ERR_TSESSION_RUNNING      0x11
#define PCO_ERR_TSFILE_OPEN           0x12
#define PCO_ERR_TSFILE_NOT_OPEN       0x14
#define PCO_ERR_WRONG_FILE_TYPE       0x15
#define PCO_ERR_TSFILE_TOO_NEW        0x16
#define PCO_ERR_TOO_MANY_LOGGERS      0x17
#define PCO_ERR_QUEUE_OPEN_ERROR      0x18
#define PCO_ERR_LOGGER_NOT_FOUND      0x19

#define PCO_ERR_TST_NOT_AVAILABLE     0x20

/*==== PCO SERVER TYPES ==================================================*/

#define PCO_STYPE_NONE      0x00
#define PCO_STYPE_PCOS      0x23
#define PCO_STYPE_L3SRV     0x1E


#endif /*_PCO_CONST_H_*/
