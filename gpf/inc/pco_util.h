/* 
+----------------------------------------------------------------------------- 
|  Project :  PCO2
|  Modul   :  PCO_UTIL
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
|  Purpose :  This Modul provides utillity functions for pco
+----------------------------------------------------------------------------- 
*/ 

#ifndef _PCO_UTIL_H_
#define _PCO_UTIL_H_

/*==== INCLUDES ===================================================*/
#include <stdio.h>
#include "ipcapi.h"
#undef EXPORT
#include "typedefs.h"

/*==== TYPES ======================================================*/

/*==== CONSTANTS ==================================================*/
#define ZERO_COMPRESS_MIN 8

/*==== EXTERNALS ==================================================*/

/*==== VARIABLES ==================================================*/

/*==== PROTOTYPES =================================================*/

/*
+--------------------------------------------------------------------+
| PROJECT : PCO2                       MODULE  : PCO_UTIL            |
| STATE   : code                       ROUTINE : get_path            |
+--------------------------------------------------------------------+

  PURPOSE : Retrieves the pathname from a path/filename(path\filenames)-string.

  PARAMS: fname ... path/filename
          path  ... retrieved path string 
          slash ... 1 -> '/'s are used as separators.
                    0 -> '\'s are used as separators
*/
void  get_path(const char* fname, char* path, int slash=1);


/*
+------------------------------------------------------+
| PROJECT : PCO2                MODULE  : PCO_UTIL     |
| STATE   : code                ROUTINE : read_string  |
+------------------------------------------------------+

  PURPOSE : Reads one line from a text file.

  PARAMS:   stream  ... stream to read from
            buf     ... buffer to read into
            max_len ... maximum length of line to read into buf

  RETURNS:   0 ... no error
            -1 ... buffer to small

*/
int read_string (FILE * stream, char * buf, int max_len);


/*
+-------------------------------------------------------------------------------+
| PROJECT : PCO2                       MODULE  : PCO_UTIL                       |
| STATE   : code                       ROUTINE : send_ipcmsg                    |
+-------------------------------------------------------------------------------+

  PURPOSE : tries to send a message with ipc header to a receiver

  PARAMS:   buf     ... pointer to buffer
            size     .. size of buffer
            id      ... message id
            sender   .. queuename of sender
            receiver .. queuename of receiver

  RETURNS:  0 .. sucess
            -1 .. receiver not found
            -2 .. error while contacting receiver 
          
*/
int send_ipcmsg(void* buf, U16 size, U16 id, const char* sender, 
                const char* receiver);


/*
+-------------------------------------------------------------------------------+
| PROJECT : PCO2                       MODULE  : PCO_UTIL                       |
| STATE   : code                       ROUTINE : send_ipcmsg                    |
+-------------------------------------------------------------------------------+

  PURPOSE : tries to send a message with ipc header to a receiver

  PARAMS:   buf     ... pointer to buffer
            size     .. size of buffer
            id      ... message id
            sender   .. queuename of sender
            receiver .. queuename of receiver
            rqueue  ... handle of receiver queue

  RETURNS:  0 .. sucess
            -1 .. error while contacting receiver 
          
*/
int send_ipcmsg(void* buf, U16 size, U16 id, const char* sender, 
                const char* receiver, CMS_HANDLE rqueue);

/*
+-------------------------------------------------------------------------------+
| PROJECT : PCO2                       MODULE  : PCO_VIEW_STD                   |
| STATE   : code                       ROUTINE : create_hexdump                 |
+-------------------------------------------------------------------------------+

  PURPOSE : creates hexdump string of given data

  PARAMS:   data   .. the data
            dsize  .. size of data
            dump   .. buffer to receive hexdump
            size   .. size of dump
            reverse.. if !=0 the hexdump will represent the reverse data buffer
            zero_compress_min .. count of zeros from which on compression will be applied

  RETURNS:  0 .. success
           -1 .. dump buffer to small

*/
int create_hexdump(const void *data, unsigned dsize, char *dump, unsigned size,
                   int reverse=0, int zero_compress_min=ZERO_COMPRESS_MIN);

/*
+-------------------------------------------------------------------------------+
| PROJECT : PCO2                       MODULE  : PCO_UTIL                       |
| STATE   : code                       ROUTINE : interprete_hexdump             |
+-------------------------------------------------------------------------------+

  PURPOSE : tries to interprete a given string hexdump-like 
            Example: 00 44 (23*00) 47 11

  PARAMS:   dump   .. string containing hexdump
            buffer .. buffer to receive interpreted data
            bsize  .. size of buffer
            count  .. count of bytes writen to buffer

  RETURNS:  0 .. success
           -1 .. buffer to small
           -2 .. invalid hexdump

*/
int interprete_hexdump(const char *dump, void* buffer, unsigned bsize, unsigned& count);


/*
+-------------------------------------------------------------------------------+
| PROJECT : PCO2                       MODULE  : PCO_UTIL                       |
| STATE   : code                       ROUTINE : get_time_hmsm                  |
+-------------------------------------------------------------------------------+

  PURPOSE : calculates the current time in ms 
            (hour, minute, second and ms are taken into account)

  RETURNS:  current time in milliseconds

*/
ULONG get_time_hmsm();

/*
+-------------------------------------------------------------------------------+
| PROJECT : PCO2                       MODULE  : PCO_UTIL                       |
| STATE   : code                       ROUTINE : get_dll_size                   |
+-------------------------------------------------------------------------------+

  PURPOSE : calculates the actual count of bytes used in a given DLL-file

  PARAMS:   dll_fname .. name of dll-file

  RETURNS:  count of bytes used, 0 if file is no WIN32-DLL

*/
ULONG get_dll_size(const char* dll_fname);


#endif /* _PCO_UTIL_H_ */