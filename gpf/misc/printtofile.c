/* 
+------------------------------------------------------------------------------
|  File:       printToFile.c
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
| Purpose:     This module implements some stream output functions.
+----------------------------------------------------------------------------- 
*/ 

#ifndef __PRINTTOFILE_C__
#define __PRINTTOFILE_C__
#endif
 
#ifndef _TARGET_

/*==== INCLUDES ===================================================*/

#include "printtofile.h"

#include "typedefs.h"
#ifndef _LINUX_
#include <string.h>
#include <ctype.h>
#include <stdarg.h>
#include <stdio.h>
#include "windows.h"
#include "vsi.h"
#include "time.h"
#include <sys/types.h>
#include <sys/timeb.h>
#endif /* _LINUX_ */

/*==== TYPES ======================================================*/


/*==== CONSTANTS ==================================================*/


/*==== EXTERNALS ==================================================*/


/*==== VARIABLES ==================================================*/

#ifndef _LINUX_
static HANDLE mutEx;
#endif /* _LINUX_ */

/*==== FUNCTIONS ==================================================*/

#ifndef RUN_INT_RAM
/*
+------------------------------------------------------------------------------
|  Function     :  PrintToFile
+------------------------------------------------------------------------------
|  Description  :  print to file xx and do a printf() with ident. arguments
|
|  Parameters   :  const char *format, ... variable parameter list
|
|  Return       :  printf retval
|                  
+------------------------------------------------------------------------------
*/
void initPrintToFile()
{
#ifdef _LINUX_
  return;
#else /* _LINUX_ */
#ifndef _TARGET_
#ifdef _DEBUG
  FILE *fp;
#endif

  if ( (mutEx = OpenMutex (MUTEX_ALL_ACCESS, FALSE, "PrintToFile")) == NULL )
  {
    mutEx = CreateMutex( NULL, FALSE, "PrintToFile");
#ifdef _DEBUG
    fp = fopen("\\gpf\\BIN\\debug\\tstlog.txt", "w");
    PrintToFile("\n\nStart logging:\n");
#endif
  }

  if (mutEx == 0)
  {
    printf("PrintToFile semaphore creation failed!\n");
  }
#endif /* _LINUX_ */
#endif
}
#endif

#ifndef RUN_INT_RAM
/*
+------------------------------------------------------------------------------
|  Function     :  PrintToFile
+------------------------------------------------------------------------------
|  Description  :  print to file xx and do a printf() with ident. arguments
|
|  Parameters   :  const char *format, ... variable parameter list
|
|  Return       :  printf retval
|                  
+------------------------------------------------------------------------------
*/
int PrintToFile(const char *format, ... )
{
  int retval = 0;
#ifndef _LINUX_
#ifndef _TARGET_
  va_list unamedArgumentList;
  char* nextChar;
  int ival;
  char* sval;
#ifdef _DEBUG
  FILE *fp;
  struct _timeb timebuf;
  time_t seconds;
  unsigned long int t;
#endif
  
  if ( WaitForSingleObject (mutEx, INFINITE) == WAIT_FAILED )
  {
    printf("PrintToFile semaphore request failed! Is TST.exe up & running?\n");
    return -1;
  }
  va_start(unamedArgumentList, format);
#ifdef _DEBUG
  fp = fopen("\\gpf\\BIN\\debug\\tstlog.txt", "a");
  if ( fp != 0 )
  {
    time( &seconds );                   // seconds after 1.1.70 GMT
    _ftime( &timebuf );
    t = (unsigned long int)(seconds * 1000) + timebuf.millitm;

    /*lint -e668 */
    /*lint -e559 */
#ifdef _TOOLS_
  fprintf(fp, "TST   (%u): ", (unsigned int)t);
#else
  fprintf(fp, "Stack (%u): ", (unsigned int)t);
#endif
  }
#endif /* _DEBUG */

  /*lint -e662 */
  for (nextChar = (char*) format; *nextChar; nextChar++)
  {
    if (*nextChar != '%')
    {
      putchar(*nextChar);
#ifdef _DEBUG
      if (fp != 0)
      {
        fputc(*nextChar,fp);
      }
#endif
      continue;
    }

    switch (*++nextChar)
    {
      case 'd':
        ival = va_arg(unamedArgumentList, int);
        printf("%d", ival);
#ifdef _DEBUG
        if (fp != 0)
        {
          fprintf(fp, "%d", ival);
        }
#endif
        break;
      case 'x':
        ival = va_arg(unamedArgumentList, int);
        printf("%x", ival);
#ifdef _DEBUG
        if (fp != 0)
        {
          fprintf(fp, "%x", ival);
        }
#endif
        break;
      case 's':
        for(sval = va_arg(unamedArgumentList, char*); *sval; sval++)
        {
          putchar(*sval);
#ifdef _DEBUG
          if (fp != 0)
          {
            fputc(*sval,fp);
          }
#endif
        }
        break;
     default:
        putchar(*nextChar);
#ifdef _DEBUG
        if (fp != 0)
        {
          fputc(*nextChar,fp);
        }
#endif
        break;
    }
  }
  /*lint +e662 */
  va_end(unamedArgumentList);
#ifdef _DEBUG
  if (fp != 0)
  {
    fclose(fp);
  }
#endif
  ReleaseMutex(mutEx);
#endif /* !_LINUX_ */
#endif /* _TARGET_ */
  return retval;
}
#endif
#endif
