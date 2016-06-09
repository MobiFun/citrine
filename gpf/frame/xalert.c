/* 
+------------------------------------------------------------------------------
|  File:       xalert.c
+------------------------------------------------------------------------------
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
|  Purpose :  
+----------------------------------------------------------------------------- 
*/ 
/*lint -emacro(718,va_start)   Symbol  'Symbol' undeclared, assumed to return  int */

#include <stdarg.h>
#include <stdio.h>
#include "typedefs.h"
#include "vsi.h"
#include "frm_glob.h"

/* To store values from _Alert to alert_info */
T_HANDLE AlertCallerStore; 
ULONG    AlertClassStore;

BOOL _Alert(char *msg, T_HANDLE Caller, ULONG alertclass)
{ 
	int max = TextTracePartitionSize - sizeof(T_S_HEADER) - sizeof(T_PRIM_HEADER);
	int len;
	int end_filepos=0;
	max=80;
	while (msg[end_filepos] && msg[end_filepos] != ')')
	{
		end_filepos++;
	}
	len = end_filepos;
	while (msg[len])
	{
		len++;
	}

	if ( len < max) 
	{
		/* nice short message */
		vsi_o_ttrace(Caller, alertclass, "%s", msg);
	}
	else
	{
		/* if string is to long we need to cut of */
		if (end_filepos < max - 3)
		{
			/* if there is room for the file location we cut of at the end */
			vsi_o_ttrace(Caller, alertclass, "%.*s...", max - 3, msg);
		}
		else
		{
			/* if there is not even room for the file location we cut of as many directories at the start as needed */
			char *separator = NULL;
			char *p = msg;
			int l = end_filepos;
			max -= 6; /* make room for ... at both end */
			/* find filename */
			while (l >= max)
			{
				separator = p;
				while (l > 0 && *p != '/' && *p != '\\')
				{
					l--;
					p++;
				}
			}
			vsi_o_ttrace(Caller, alertclass, "...%.*s...", max - 6, separator);
		}
	}
 
  AlertCallerStore = Caller;
  AlertClassStore = alertclass;
  /*
   * Note: this return value controls if alert_info is called, 
   * i.e. alert_info is not called if FALSE is returned.
   */
  return TRUE;
}
    
int int_vsi_o_ttrace ( T_HANDLE Caller, ULONG TraceClass, const char * const format, va_list varpars );
											 
/* 
 * Alert Information 
 */
BOOL alert_info(char *format, ...)
{
  va_list args;
 
  if ( (format != NULL) && (format[0] != 0) && (AlertCallerStore >= 0) && (TraceMask[AlertCallerStore] & AlertClassStore))
  {
    va_start(args, format);
    int_vsi_o_ttrace(AlertCallerStore, AlertClassStore, format, args);
    va_end(args);
  }
 
  /* The expression evaluated to FALSE by definition if this was called */            
  return FALSE; 
}

