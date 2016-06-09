/* 
+------------------------------------------------------------------------------
|  File:       usart.c
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
|  Purpose :  This Modul defines functions for actual or simulated 
|             USART comunication between two PS-Frames.
|             Use US_set_mode() to select actual USART under windows95 or 
|             under windowsNT, or to select simulated USART under win95/NT
+----------------------------------------------------------------------------- 
*/ 



#ifndef __USART_C__
#define __USART_C__
#endif

/*==== INCLUDES ===================================================*/

 #include <windows.h>
#ifndef _VXWORKS_
  #include <stdarg.h>
#endif
#include <stdio.h>
#include "typedefs.h"
#include "usart.h"
#include "printtofile.h"

/*==== CONSTANTS ==================================================*/

#define COM_ERROR		(CE_FRAME | CE_IOE | CE_OVERRUN)
#define XON				0x11
#define XOFF			0x13

#define READER_THREAD_EXIT_CODE 4711
#define USART_BUFFER_SIZE       0x10000   /* 65536 */
#define FILE_MAX_CHUNK          0x0ffff
#define FILE_SLOW_DOWN          0x01

#ifdef _TOOLS_ 
  #define USART_SEND_TIMEOUT 60000
  #define USART_RCV_TIMEOUT 120000
#else  /* _TOOLS_ */
  #define USART_SEND_TIMEOUT INFINITE
  #define USART_RCV_TIMEOUT INFINITE
#endif /* _TOOLS_ */

/*==== TYPES ======================================================*/
typedef struct
{
  UBYTE  Connects;
  UBYTE  Type;
  USHORT CH1_numOfBytes;
  USHORT CH2_numOfBytes;
  UBYTE  CH1_CTS;
  UBYTE  CH2_CTS;
  UBYTE  CH1_data[USART_BUFFER_SIZE];
  UBYTE  CH2_data[USART_BUFFER_SIZE];
} T_USARTStream;

/*==== EXPORT =====================================================*/
/*==== PRIVATE ====================================================*/
/*==== VARIABLES ==================================================*/

static int       m_mode=UT_MODE_NT;
static long int  m_send_timeout=(long int)USART_SEND_TIMEOUT;
static long int  m_rcv_timeout =(long int)USART_RCV_TIMEOUT;
static int first_ut_init = 1;
static FILE*     m_file=NULL;

static OVERLAPPED gWriteOverLap;
static OVERLAPPED gReadOverLap;

static  int   ReaderThreadExitRequest = FALSE;


LOCAL   void  (*ReceiveCallback)(void) = NULL;
static  int   initialized = FALSE;

#ifdef COM_AUTOSEARCH
static  int   P  = 0;
#endif

#ifdef DEBUG_USART
static  int   usart_in, usart_out;
#endif

T_USARTStream *Stream;

HANDLE         SemCH1_full;
HANDLE         SemCH2_full;
HANDLE         SemCH1_empty;
HANDLE         SemCH2_empty;
HANDLE         USARTMemHandle;
HANDLE         *semRCVFull=0, *semRCVEmpty=0;
HANDLE         *semSNDFull=0, *semSNDEmpty=0;
HANDLE         ut_sema_handle;

UBYTE          *InBuffer, *OutBuffer, *readPointer;
USHORT         *InCounter, *OutCounter;
UBYTE          *CTS = NULL;


static HANDLE hComDev = INVALID_HANDLE_VALUE;
static HANDLE hThread = INVALID_HANDLE_VALUE;
static int mem_closed = TRUE;
static int cls_cnt = 0;
static int snd_cnt = 0;

/*==== FUNCTIONS ==================================================*/

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)           MODULE  : USART                  |
| STATE   : code                    ROUTINE : US_set_mode            |
+--------------------------------------------------------------------+

  PURPOSE : With this function you can select the UART mode 
                #define US_MODE_95         1
                #define US_MODE_NT         2
                #define US_MODE_SIM        3  
                #define US_MODE_FILE       4
  
*/
void US_set_mode(int mode)
{
  m_mode=mode;
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)           MODULE  : USART                  |
| STATE   : code                    ROUTINE : US_set_mode            |
+--------------------------------------------------------------------+

  PURPOSE : With this function get the seleced UART mode  
                #define US_MODE_95         1
                #define US_MODE_NT         2
                #define US_MODE_SIM        3  
                #define US_MODE_FILE       4
*/
int US_get_mode()
{
  return m_mode;
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)           MODULE  : USARTSIM               |
| STATE   : code                    ROUTINE : unlockUSARTMemory      |
+--------------------------------------------------------------------+

  PURPOSE : unlocks the previously locked shared memory area.
  
*/
LOCAL void markRCVBufferEmpty (void)
{
  ReleaseSemaphore (*semRCVEmpty,
                    1,
                    NULL);
}

LOCAL void markSNDBufferFull (void)
{
  ReleaseSemaphore (*semSNDFull,
                    1,
                    NULL);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)           MODULE  : USARTSIM               |
| STATE   : code                    ROUTINE : waitForSNDBufferEmpty  |
+--------------------------------------------------------------------+

  PURPOSE : waits for send buffer becoming empty

  RETURNS :  0 .. send buffer is empty
            -1 .. given up

*/
LOCAL int waitForSNDBufferEmpty (void)
{
  if (WaitForSingleObject (*semSNDEmpty, m_send_timeout) NEQ WAIT_OBJECT_0)
  {
    int err = GetLastError();
    PrintToFile("USART: error code %d\n", err);
    PrintToFile("USART: giving up sending with %d ms timeout :-(\n", m_send_timeout);
    return -1; /* give up */
  }
  return 0;
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)           MODULE  : USARTSIM               |
| STATE   : code                    ROUTINE : createUSARTMemory      |
+--------------------------------------------------------------------+

  PURPOSE : Create two pipes for byte oriented data exchange
            between two win32 processes

*/

LOCAL void *createUSARTMemory (char *name, ULONG size)
{
  char newname[40];

  /*
   * create two Semaphores pairs to protect the send data to be
   * overwritten before they have read by the receiver
   */

  sprintf (newname, "%s_CH1empty", name);
  
  SemCH1_empty = CreateSemaphore (NULL,
                                  1,                               
                                  1,                           
                                  newname);
     
  if (SemCH1_empty EQ NULL)
    return NULL;

  sprintf (newname, "%s_CH1full", name);
  
  SemCH1_full = CreateSemaphore (NULL,
                                 0,                               
                                 1,                           
                                 newname);
     
  if (SemCH1_full EQ NULL)
    return NULL;

  sprintf (newname, "%s_CH2empty", name);
  
  SemCH2_empty = CreateSemaphore (NULL,
                                  1,                               
                                  1,                           
                                  newname);
     
  if (SemCH2_empty EQ NULL)
    return NULL;

  sprintf (newname, "%s_CH2full", name);
  
  SemCH2_full = CreateSemaphore (NULL,
                                 0,                               
                                 1,                           
                                 newname);
     
  if (SemCH2_full EQ NULL)
    return NULL;

  /*
   * create a shared memory area
   */
  sprintf (newname, "UT_Mem_%s", name);

  USARTMemHandle
      = CreateFileMapping (
                           (HANDLE) 0xffffffff,  /* memory-mapped     */
                            NULL,                /* no security       */
                            PAGE_READWRITE,      /* read/write access */
                           (DWORD) 0,
     /* memory size     */ (DWORD) size,
                           newname           /* name of sh. mem */
                          ); 
  
  if (USARTMemHandle EQ NULL)
    return NULL;
 
  /*
   * map the shared memory area into the address space of the process
   * and return the startaddress.
   */
  
  return MapViewOfFile (USARTMemHandle,
                        FILE_MAP_WRITE,
                        0,
                        0,
                        0);
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)           MODULE  : USARTSIM               |
| STATE   : code                    ROUTINE : openUSARTMemory        |
+--------------------------------------------------------------------+

  PURPOSE : open a shared memory area for character exchange
            between two WIN32 processes

*/

LOCAL void *openUSARTMemory (char *name, ULONG size)
{
  char newname[30];

  /*
   * open the Semaphores
   */

  sprintf (newname, "%s_CH1empty", name);

  SemCH1_empty = OpenSemaphore (SEMAPHORE_MODIFY_STATE
                                | SYNCHRONIZE,
                                FALSE,
                                newname);
     
  if (SemCH1_empty EQ NULL)
    return NULL;

  sprintf (newname, "%s_CH1full", name);

  SemCH1_full =  OpenSemaphore (SEMAPHORE_MODIFY_STATE
                                | SYNCHRONIZE,
                                FALSE,
                                newname);
     
  if (SemCH1_full EQ NULL)
    return NULL;

  sprintf (newname, "%s_CH2empty", name);

  SemCH2_empty = OpenSemaphore (SEMAPHORE_MODIFY_STATE
                                | SYNCHRONIZE,
                                FALSE,
                                newname);
     
  if (SemCH2_empty EQ NULL)
    return NULL;

  sprintf (newname, "%s_CH2full", name);

  SemCH2_full =  OpenSemaphore (SEMAPHORE_MODIFY_STATE
                                | SYNCHRONIZE,
                                FALSE,
                                newname);
     
  if (SemCH2_full EQ NULL)
    return NULL;

  /*
   * open the shared memory area
   */
  
  sprintf (newname, "UT_Mem_%s", name);

  USARTMemHandle =
    OpenFileMapping (FILE_MAP_WRITE,
                     FALSE,
                     newname);  /* name of sh. mem */
                 
  
  if (USARTMemHandle EQ NULL)
    return NULL;
 
  /*
   * map the shared memory area into the address space of the process
   * and return the startaddress.
   */
  
  return MapViewOfFile (USARTMemHandle,
                        FILE_MAP_WRITE,
                        0,
                        0,
                        0);
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)           MODULE  : USART                  |
| STATE   : code                    ROUTINE : waitForRCVBufferFull   |
+--------------------------------------------------------------------+

  PURPOSE : This function waits until an incoming character
            is signaled with the EV_RXCHAR Event 
  
*/

LOCAL void waitForRCVBufferFull (void)
{
  switch (m_mode) {
    case US_MODE_95: {
      static COMSTAT stComStat;
      static DWORD   dwErrors;
      static DWORD   EvtMask = 0;
      BOOL    validReceive = FALSE;

      do
      {
        SetCommMask (hComDev, EV_RXCHAR) ;
        WaitCommEvent (hComDev, &EvtMask, NULL);
        ClearCommError (hComDev, &dwErrors, &stComStat);
  
        if (dwErrors & COM_ERROR)
          PurgeComm (hComDev, PURGE_RXCLEAR|PURGE_RXABORT);
        else
          validReceive = TRUE;

      } while (!validReceive);
      break;
    }
    case US_MODE_NT: {
      static COMSTAT stComStat;
      static DWORD   dwErrors;
      static DWORD   EvtMask = 0;
      BOOL    validReceive = FALSE;

      do
      {
        SetCommMask (hComDev, EV_RXCHAR) ;
        WaitCommEvent (hComDev, &EvtMask, NULL);
        ClearCommError (hComDev, &dwErrors, &stComStat);

        if (dwErrors & COM_ERROR)
          PurgeComm (hComDev, PURGE_RXCLEAR|PURGE_RXABORT);
        else
          validReceive = TRUE;

      } while (!validReceive && !ReaderThreadExitRequest);
      break;
    }
    case US_MODE_SIM: {
      if (WaitForSingleObject (*semRCVFull, m_rcv_timeout) NEQ WAIT_OBJECT_0)
      {
        PrintToFile("USART: no stack connected\n");
      }
      break;
    }
    default:
      break;
  }
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)           MODULE  : USART                  |
| STATE   : code                    ROUTINE : _readerThread          |
+--------------------------------------------------------------------+

  PURPOSE : This function is the central signal handling function. It is
            installed as a thread and waits for the occurance of an
            event and then calls the installed callback function
            of the application.
  
*/

LOCAL void _readerThread (void)
{
  while (ReaderThreadExitRequest == FALSE)
  {
    waitForRCVBufferFull ();
    ReceiveCallback ();
  } 
  ReaderThreadExitRequest = FALSE;
  ExitThread(READER_THREAD_EXIT_CODE);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)           MODULE  : USART                  |
| STATE   : code                    ROUTINE : convertBaudrate        |
+--------------------------------------------------------------------+

  PURPOSE : This function convert the constants for the baudrates in
            usart.h into the equvalent constants for WIN32 comm.
        
*/
LOCAL DWORD convertBaudrate (DWORD br)
{   
  switch (br)
  {
    case US_BAUD_256000:
      return CBR_256000;

    case US_BAUD_128000:
      return CBR_128000;

    case US_BAUD_115200:
      return CBR_115200;

    case US_BAUD_57600:
      return CBR_57600;

    case US_BAUD_38400:
      return CBR_38400;
    
    case US_BAUD_19200:
      return CBR_19200;

    case US_BAUD_14400:
      return CBR_14400;

    case US_BAUD_9600:
      return CBR_9600;

    case US_BAUD_4800:
      return CBR_4800;

    case US_BAUD_2400:
      return CBR_2400;

    case US_BAUD_1200:
      return CBR_1200;

    case US_BAUD_600:
      return CBR_600;

    case US_BAUD_300:
      return CBR_300;

    default:
      /* no CBR_xxx constant found -> return value itsself */
      return br;
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)           MODULE  : USART                  |
| STATE   : code                    ROUTINE : sioInit                |
+--------------------------------------------------------------------+

  PURPOSE : This function opens the given comm port and initiializes
            the DCB with baudrate and flowcontrol parameters.
        
*/

LOCAL BOOL sioInit (int  portNr,
                    unsigned int  baudrate,
                    unsigned int  bufSize,
                    char flowCtrl)
{
  char         szPort[10];
  DCB          stDCB;
  COMMTIMEOUTS stTimeout ;
  DWORD        dwErrorFlags;
  COMSTAT      stComStat;

  sprintf (szPort, "\\\\.\\COM%d", portNr) ;

  // ------------------------------------
  // open the communication device
  // ------------------------------------
  if (m_mode==UT_MODE_NT) {
    hComDev = CreateFile
            ( 
              szPort,
              GENERIC_READ | GENERIC_WRITE,
              0,                       /* exclusive access  */
              NULL,                    /* no security attrs */
              OPEN_EXISTING,
              FILE_FLAG_OVERLAPPED,
              NULL
            );
  } else {
    hComDev = CreateFile
            ( 
              szPort,
              GENERIC_READ | GENERIC_WRITE,
              0,                       // exclusive access
              NULL,                    // no security attrs
              OPEN_EXISTING,
              0,
              NULL
            );
  }

  if (hComDev EQ INVALID_HANDLE_VALUE)
    return FALSE;   // device not available

  if (m_mode==UT_MODE_NT) {
    gReadOverLap.hEvent = CreateEvent (NULL, TRUE, FALSE, NULL);
    if (!gReadOverLap.hEvent)
	    return ( FALSE );

    gReadOverLap.Offset = 0;
    gReadOverLap.OffsetHigh = 0;

    gWriteOverLap.hEvent = CreateEvent (NULL, TRUE, FALSE, NULL);
    if (!(gWriteOverLap.hEvent))
	    return ( FALSE );

    gWriteOverLap.Offset = 0;
    gWriteOverLap.OffsetHigh = 0;
  }

  // ------------------------------------
  // get any early notifications
  // ------------------------------------
  SetCommMask (hComDev, EV_RXCHAR);

  // ------------------------------------
  // setup device buffers
  // ------------------------------------
  SetupComm (hComDev,
             bufSize,
             bufSize
            );

  // ------------------------------------
  // purge any information in the buffer
  // ------------------------------------
  PurgeComm (hComDev,
             PURGE_TXABORT | PURGE_RXABORT |
             PURGE_TXCLEAR | PURGE_RXCLEAR
            );

  // ------------------------------------
  // setup up and enable communication
  // device. If not possible close
  // communication and abort function.
  // ------------------------------------
  if (!GetCommState (hComDev, &stDCB))
  {
    SetCommMask (hComDev, 0);
    CloseHandle (hComDev);
    return FALSE;
  }

  stDCB.DCBlength = sizeof (stDCB);  // sizeof(DCB) 

  switch (flowCtrl)
  {
    case 'N':
      stDCB.fOutxCtsFlow = FALSE;
      stDCB.fOutxDsrFlow = FALSE;
      stDCB.fDtrControl = DTR_CONTROL_DISABLE;
      stDCB.fRtsControl = RTS_CONTROL_DISABLE;
      break;
    case 'D':
      stDCB.fOutxCtsFlow = FALSE;
      stDCB.fOutxDsrFlow = TRUE;
      stDCB.fDtrControl = DTR_CONTROL_HANDSHAKE;
      stDCB.fRtsControl = RTS_CONTROL_DISABLE;
      stDCB.XonLim  = 0;
      stDCB.XoffLim = 50;
      break;
    case 'R':
      stDCB.fOutxCtsFlow = TRUE;
      stDCB.fOutxDsrFlow = FALSE;
      stDCB.fDtrControl = DTR_CONTROL_DISABLE;
      stDCB.fRtsControl = RTS_CONTROL_HANDSHAKE;
      stDCB.XonLim  = 0;
      stDCB.XoffLim = 50;
      break;
    case 'P':
      stDCB.fOutxCtsFlow = FALSE;
      stDCB.fOutxDsrFlow = FALSE;
      stDCB.fDtrControl = DTR_CONTROL_ENABLE;
      stDCB.fRtsControl = RTS_CONTROL_DISABLE;
      break;
    case 'V':
      if (m_mode==US_MODE_NT) {
        stDCB.fOutxCtsFlow = FALSE;
        stDCB.fOutxDsrFlow = FALSE;
        stDCB.fDtrControl = DTR_CONTROL_ENABLE;
        stDCB.fRtsControl = RTS_CONTROL_ENABLE;
        break;
      }
      /*lint -fallthrough*/
      /* go on if not US_MODE_NT */
    default:
      return FALSE;
  }
  fprintf (stdout,"flow control: %c ...", flowCtrl);

  stDCB.BaudRate          = baudrate;  // current baud rate 
  stDCB.fBinary           = TRUE;      // binary mode, no EOF check 
  stDCB.fParity           = FALSE;     // enable parity checking 
  stDCB.fDsrSensitivity   = FALSE;     // DSR sensitivity 
  stDCB.fTXContinueOnXoff = FALSE;     // XOFF continues Tx 
  stDCB.fOutX             = FALSE;     // XON/XOFF out flow control 
  stDCB.fInX              = FALSE;     // XON/XOFF in flow control 
  stDCB.fErrorChar        = FALSE;     // enable error replacement 
  stDCB.fNull             = FALSE;     // enable null stripping 
  stDCB.fAbortOnError     = FALSE;     // abort reads/writes on error 
  stDCB.ByteSize          = 8;         // number of bits/byte, 4-8 
  stDCB.Parity            = NOPARITY;  // 0-4=no,odd,even,mark,space 
  stDCB.StopBits          = ONESTOPBIT;// 0,1,2 = 1, 1.5, 2 
  stDCB.XonChar           = 0;         // Tx and Rx XON character 
  stDCB.XoffChar          = 0;         // Tx and Rx XOFF character 
  stDCB.ErrorChar         = 0;         // error replacement character 
  stDCB.EofChar           = 0;         // end of input character 
  stDCB.EvtChar           = 0;         // received event character 

  if (!SetCommState (hComDev, &stDCB))
  {
    SetCommMask (hComDev, 0);
    CloseHandle (hComDev);
    return FALSE;
  }

  if (!GetCommTimeouts (hComDev, &stTimeout))
    return FALSE;

  stTimeout.WriteTotalTimeoutConstant   = 0xffff;
  stTimeout.WriteTotalTimeoutMultiplier = 0xffff;
  stTimeout.ReadTotalTimeoutConstant    = 0xffff;
  stTimeout.ReadIntervalTimeout         = 0;
  stTimeout.ReadTotalTimeoutMultiplier  = 0xffff;

  if (!SetCommTimeouts (hComDev, &stTimeout))
    return FALSE;

  PurgeComm (hComDev, PURGE_RXCLEAR
                     |PURGE_TXCLEAR
                     |PURGE_TXABORT
                     |PURGE_RXABORT);

  ClearCommError (hComDev, &dwErrorFlags, &stComStat);

  return TRUE;
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : USART                      |
| STATE   : code                ROUTINE : sioRead                    |
+--------------------------------------------------------------------+

  PURPOSE : 

*/

LOCAL BOOL sioRead (BYTE  *bpRXBuffer, // RX Buffer
                    DWORD *pdwLength)  // IN:  Bytes to read
{
  DWORD   dwBytesToRead = *pdwLength ;
  DWORD   dwErrorFlags;
  COMSTAT stComStat;

  if (hComDev == INVALID_HANDLE_VALUE
    OR *pdwLength == 0) 
    return FALSE;  // device not available

  if (m_mode==US_MODE_NT) {
    if (!ReadFile (hComDev,
                 bpRXBuffer,
                 dwBytesToRead,
                 pdwLength,
                 &gReadOverLap))
    {
      // if there was a problem
		  if (GetLastError() == ERROR_IO_PENDING)
   	  {
	  	  /* asynchronous i/o is still in progress         */
		
   			/* do something else for a while                 */
	   		/* check on the results of the asynchronous read */
	    	if (GetOverlappedResult(gReadOverLap.hEvent, &gReadOverLap, 
	    		pdwLength, TRUE))
			    return TRUE;
      }

      ClearCommError(hComDev, &dwErrorFlags, &stComStat);
      return FALSE;
    } else {
      ClearCommError(hComDev, &dwErrorFlags, &stComStat);
    }
  } else {
    if (!ReadFile (hComDev,
                 bpRXBuffer,
                 dwBytesToRead,
                 pdwLength,
                 NULL))
    {
      // if there was a problem
      ClearCommError(hComDev, &dwErrorFlags, &stComStat);
      return FALSE;
    }
  }
#ifdef DEBUG_USART
  {
    char traceBuf[255];
    unsigned int  i;

    traceBuf[0] = '\0';

    if (bpRXBuffer[0] EQ 0xff)
    {
      Sleep(1);
    }

    for (i=0; i<*pdwLength; i++)
    {
      if (!isprint (bpRXBuffer[i]))
      {
        sprintf (traceBuf+strlen(traceBuf),
                 "[%02x]%c",
                 (USHORT) bpRXBuffer[i],
                 ((bpRXBuffer[i] EQ '\n') ? '\n' : ' ')
                 );
      }
      else
        sprintf (traceBuf+strlen(traceBuf),
                 "%c",
                 bpRXBuffer[i]);
      if (strlen (traceBuf) > 200)
      {
        write (usart_in, traceBuf, strlen (traceBuf));
        traceBuf[0] = '\0';
      }
    }
    /*
     * write the string to the tracefile
     */
    write (usart_in, traceBuf, strlen (traceBuf));
  }
#endif
  return TRUE;
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : USART                      |
| STATE   : code                ROUTINE : sioWrite                   |
+--------------------------------------------------------------------+

  PURPOSE : 

*/

LOCAL BOOL sioWrite (BYTE  *bpTXBuffer, // TX   Buffer
                     DWORD *pdwLength)  // IN:  Bytes to read
{
  DWORD   dwBytesToSend = *pdwLength, toSend, realySend;
  int     ret;
  DWORD   dwErrorFlags;
  COMSTAT stComStat;
  DWORD   dwNumberOfBytesTransferred;   /* Windows 95:                                       */
                                        /* The number of transferred bytes returned by the   */
                                        /* GetOverlappedResult function is always zero, also */
                                        /* if there are bytes received by the communication  */ 
                                        /* partner via the serial line.                      */

  if (hComDev EQ INVALID_HANDLE_VALUE
    OR !bpTXBuffer
    OR *pdwLength ==0)
     return FALSE; // device not available

  realySend = 0;
  toSend    = dwBytesToSend;

  while (toSend > 0)
  {
    if (m_mode==US_MODE_NT) {
    	if (toSend > 20) {
	    	ret = WriteFile (hComDev,
                       bpTXBuffer+realySend,
                       20,
                       pdwLength,
                       &gWriteOverLap);
		    dwNumberOfBytesTransferred = 20;     
  	  } else {
  		  ret = WriteFile (hComDev,
                       bpTXBuffer+realySend,
                       toSend,
                       pdwLength,
                       &gWriteOverLap);
		    dwNumberOfBytesTransferred = toSend;
	    }
    	/* if there was a problem, or the async. operation's still pending ... */
	    if (!ret)
  	  {
  	  	/* deal with the error code */
	  	  if (GetLastError() == ERROR_IO_PENDING)
  	  	{
	  	  	/* asynchronous i/o is still in progress         */
		
  		  	/* do something else for a while                 */
	  		  /* check on the results of the asynchronous read */
  		  	while (!GetOverlappedResult(gWriteOverLap.hEvent, &gWriteOverLap, pdwLength, TRUE))
	    		{
			    	if(GetLastError() == ERROR_IO_INCOMPLETE) {
    					continue;
            } else {
				    	break ;
            }
      		}
		    } else {
			    ClearCommError(hComDev, &dwErrorFlags, &stComStat);
    			return FALSE;
		    }
      }
    } else {
      if (toSend > 20) {
        ret = WriteFile (hComDev,
                       bpTXBuffer+realySend,
                       20,
                       pdwLength,
                       NULL);
        dwNumberOfBytesTransferred = 20; /* US_MODE_NT */
      } else {
        ret = WriteFile (hComDev,
                       bpTXBuffer+realySend,
                       toSend,
                       pdwLength,
                       NULL);
  	    dwNumberOfBytesTransferred = toSend; /* US_MODE_NT */
      }
    }

#ifdef DEBUG_USART
    {
      char traceBuf[255];
      unsigned int  i;

      traceBuf[0] = '\0';

      for (i=0; i<*pdwLength; i++)
      {
        if (!isprint (bpTXBuffer[realySend+i]))
        {
          sprintf (traceBuf+strlen(traceBuf),
                   "[%02x]%c",
                   (USHORT) bpTXBuffer[realySend+i],
                   ((bpTXBuffer[realySend+i] EQ '\n') ? '\n': ' ')
                   );
        }
        else
          sprintf (traceBuf+strlen(traceBuf),
                   "%c",
                   bpTXBuffer[realySend+i]);
        if (strlen (traceBuf) > 200)
        {
          write (usart_out, traceBuf, strlen (traceBuf));
          traceBuf[0] = '\0';
        }
      }
      /*
       * write the string to the tracefile
       */
      write (usart_out, traceBuf, strlen (traceBuf));
    }
#endif

    switch (m_mode) {
      case US_MODE_NT:
       	realySend += dwNumberOfBytesTransferred;
      	toSend    -= dwNumberOfBytesTransferred;
        break;
      case US_MODE_95:
        realySend += *pdwLength;
        toSend    -= *pdwLength;
        break;
      default:
        break;
    }
  }

  *pdwLength = dwBytesToSend;

  return TRUE;
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : USART                      |
| STATE   : code                ROUTINE : UT_Init                    |
+--------------------------------------------------------------------+

  PURPOSE : initializes the USART driver

  RETURNS : 0 ... initialization succeeded
           -1 ... error

*/


int UT_Init (unsigned int baudRate, int fifoSize, char flow_ctrl, void (func(void)), const char* fname )
{
  if (initialized == TRUE)
  {
		return 0;
  }

  switch (m_mode) 
  {
    case US_MODE_FILE:
      if (!fname || !strlen(fname) || (m_file=fopen(fname,"rb"))==NULL)
      {
        fprintf (stdout, "USART: failed to open %s :-(\n",fname);
        return -1;
      }
      fprintf (stdout, "USART: \"%s\" opened\n",fname);
      break;
    case US_MODE_95:
    case US_MODE_NT: {
      BOOL   ret;
      DWORD  dwThreadID;
#ifdef COM_AUTOSEARCH
      int    from, to;
      int portNr;
#else
      EXTERN int extPort;
#endif

      ReceiveCallback = func;

#ifdef COM_AUTOSEARCH
      if (P NEQ 0) {
        from = to = P;
      } else {
        from = 1;
        to = 4;
      }

      /*
       * try COMn: to COMm: where n is from and m is to
      */
      for (portNr = from; portNr <= to; portNr++)
      {
        fprintf (stdout,"USART: Trying COM%d ... ", portNr);

        if ((ret = sioInit (portNr,
                        convertBaudrate (baudRate),
                        10000 /*fifoSize*/,
                        flow_ctrl)) EQ FALSE) {
          fprintf (stdout, "fail\n");
        } else {
          fprintf (stdout, "success!\n");
          break;
        }
      }

#else
      fprintf (stdout, "USART: Trying COM%d ... ", extPort);

      if ((ret = sioInit (extPort,
                        convertBaudrate (baudRate),
                        10000 /*fifoSize*/,
                        flow_ctrl)) EQ FALSE) {
        fprintf (stdout, "fail\n");
      }
#endif

      if (ret)
      {
        fprintf (stdout, "success!\n");

        if (ReceiveCallback NEQ NULL)
        {
          hThread = CreateThread ((LPSECURITY_ATTRIBUTES) NULL,
                              0,
                              (LPTHREAD_START_ROUTINE) _readerThread,
                              (LPVOID) NULL,
                               0,
                              &dwThreadID
                             );
        }
        initialized = TRUE;
      }
      else {
        fprintf (stdout, "USART: COM-port not free or baudrate not supported !\n");
        return -1;
      }
      break;  
    }

    case US_MODE_SIM: {
      int    i;

      if ( first_ut_init )
      {
        if ( (ut_sema_handle = OpenSemaphore (SEMAPHORE_MODIFY_STATE | SYNCHRONIZE, FALSE, "UT_SIM_SEMA")) == NULL )
        {
          ut_sema_handle = CreateSemaphore (NULL, 1, 1, "UT_SIM_SEMA");
        }
        first_ut_init = 0;
      }

      WaitForSingleObject (ut_sema_handle, INFINITE);

      if ((Stream = (T_USARTStream *) openUSARTMemory
                                      (
                                        (char*)"GSM",
                                        sizeof (T_USARTStream)
                                      )) EQ NULL)
      {
        if ((Stream = (T_USARTStream *) createUSARTMemory
                                      (
                                        (char*)"GSM",
                                        sizeof (T_USARTStream)
                                      )) EQ NULL)
        {
          PrintToFile ("USART: simulation could not create a shared memory area\n");
          return -1;
        }
        PrintToFile ("USART: shared memory area created\n");
  
        Stream->CH1_numOfBytes = 0;
        Stream->CH2_numOfBytes = 0;

        Stream->CH1_CTS = 0;
        Stream->CH2_CTS = 0;
  
        for (i=0; i<USART_BUFFER_SIZE; i++)
        {
          Stream->CH1_data[i] = 0;
          Stream->CH2_data[i] = 0;
        }

        Stream->Connects = 0;  /* init connection counter (!! CURRENTLY NOT USED !!) */
        Stream->Type=1;        /* signaling new type */ 
      }
      else
      {
        PrintToFile ("USART: shared memory area opened\n");
      }

      /* set pointers to semaphores and data buffers */
#ifdef _TOOLS_
      if (Stream->Type==0) // shared mem created by old stack
      {
        PrintToFile ("USART: connecting to old stack !\n");

        Stream->CH1_CTS = 1; // (baudRate NEQ -1); removed because baudrate never negative

        InBuffer      = Stream->CH1_data;
        OutBuffer     = Stream->CH2_data;
        InCounter     = &Stream->CH1_numOfBytes;
        OutCounter    = &Stream->CH2_numOfBytes;
        semRCVFull    = &SemCH1_full;
        semRCVEmpty   = &SemCH1_empty;
        semSNDFull    = &SemCH2_full;
        semSNDEmpty   = &SemCH2_empty;
        CTS           = &Stream->CH2_CTS;
      }
      else // shared mem created by us or new stack
      {
        Stream->CH2_CTS  = 1;  // (baudRate NEQ -1); removed because baudrate never negative 

        InBuffer      = Stream->CH2_data;
        OutBuffer     = Stream->CH1_data;
        InCounter     = &Stream->CH2_numOfBytes;
        OutCounter    = &Stream->CH1_numOfBytes;
        semRCVFull    = &SemCH2_full;
        semRCVEmpty   = &SemCH2_empty;
        semSNDFull    = &SemCH1_full;
        semSNDEmpty   = &SemCH1_empty;
        CTS           = &Stream->CH1_CTS;
      }
#else  /* _TOOLS_ */
      Stream->CH1_CTS = 1;  // (baudRate NEQ -1); removed because baudrate never negative 

      InBuffer      = Stream->CH1_data;
      OutBuffer     = Stream->CH2_data;
      InCounter     = &Stream->CH1_numOfBytes;
      OutCounter    = &Stream->CH2_numOfBytes;
      semRCVFull    = &SemCH1_full;
      semRCVEmpty   = &SemCH1_empty;
      semSNDFull    = &SemCH2_full;
      semSNDEmpty   = &SemCH2_empty;
      CTS           = &Stream->CH2_CTS;
#endif /* _TOOLS_ */

      readPointer = InBuffer;

      ReceiveCallback = func;

      Stream->Connects++;      /* mark connection (!! CURRENTLY NOT USED !!) */
      }
        
      ReleaseSemaphore (ut_sema_handle, 1, NULL);

      break;
    default:
      break;
  }
  initialized = TRUE;
  mem_closed = FALSE;

#ifdef DEBUG_USART
  /*
   * Open protocol file and initialize
   */

  usart_in  = open ("USART.IN", O_WRONLY| O_TEXT| O_TRUNC| O_CREAT, 0666);
  usart_out = open ("USART.OUT", O_WRONLY| O_TEXT| O_TRUNC| O_CREAT, 0666);
#endif

  return 0;
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : USARTSIM                   |
| STATE   : code                ROUTINE : UT_Close                   |
+--------------------------------------------------------------------+

  PURPOSE : 

*/

GLOBAL BOOL UT_Close(void)
{

  if (initialized == FALSE)
		return FALSE;

  switch (m_mode) {
    case US_MODE_FILE:
      if (m_file)
      {
        fclose(m_file);
        m_file=NULL;
      }
      break;
    case US_MODE_95:
    case US_MODE_NT: {
    	DWORD ExitCode;
    
      if (ReceiveCallback != NULL) {
    	  /* Initialize stop _readerThread */
    	  ReaderThreadExitRequest = TRUE;
    	  while (ReaderThreadExitRequest == TRUE)
		      SetCommMask (hComDev, 0);
      }

    	/* Close Communication port. */
    	PurgeComm (hComDev,
			   PURGE_TXABORT | PURGE_RXABORT |
			   PURGE_TXCLEAR | PURGE_RXCLEAR ) ;
      if (m_mode==UT_MODE_NT) {
      	CloseHandle (gReadOverLap.hEvent);
    	  gReadOverLap.hEvent = INVALID_HANDLE_VALUE ;
    	  CloseHandle (gWriteOverLap.hEvent);
    	  gWriteOverLap.hEvent = INVALID_HANDLE_VALUE ;
      }
    	CloseHandle (hComDev);
    	hComDev = INVALID_HANDLE_VALUE;
	
      if (ReceiveCallback != NULL) {
      	/* Stop _readerThread */
      	do {
		      GetExitCodeThread(hThread, (LPDWORD) &ExitCode);
    	  }
    	  while (ExitCode == STILL_ACTIVE);
      	CloseHandle (hThread);
      	hThread = INVALID_HANDLE_VALUE;
      }
    
      break;
    }
    case US_MODE_SIM: {
      PrintToFile("USART: shared memory closed (%d)\n",cls_cnt);
      mem_closed = TRUE;
     /* mark disconnection  */
      Stream->Connects=(Stream->Connects>1) ? 1 : 0; /* (!! CURRENTLY NOT USED !!) */

#ifdef _TOOLS_
      if (Stream->Type==0) /* shared mem created by old stack */
      {
        fprintf (stdout, "USART: disconnecting from old stack !\n");
        Stream->CH1_CTS = 0; 
      }
      else /* shared mem created by us or new stack */
      {
        Stream->CH2_CTS = 0; 
      }
#else /* _TOOLS_ */
      Stream->CH1_CTS = 0; 
#endif /* _TOOLS_ */

      CTS = NULL;
      /* close all handles */
      UnmapViewOfFile((void*)Stream);
      CloseHandle(USARTMemHandle);
      CloseHandle(SemCH1_full);
      CloseHandle(SemCH2_full);
      CloseHandle(SemCH1_empty);
      CloseHandle(SemCH2_empty);
      cls_cnt++;
      break;
    default:
      break;
    }
  }

#ifdef DEBUG_USART
 	/* close tracefiles for usart-in and out */
 	close(usart_in);
 	close(usart_out);
#endif

	/* Deinitialize */
	ReceiveCallback = NULL;
  initialized     = FALSE;

	return TRUE;
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : USART                      |
| STATE   : code                ROUTINE : UT_InitBlk                 |
+--------------------------------------------------------------------+

  PURPOSE : Initialize the USART for reading blocks

*/

int UT_InitBlk ( unsigned int baudRate, int fifoSize, char flow_ctrl, void *hP)
{
  return UT_Init (baudRate, fifoSize, flow_ctrl, NULL, NULL);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : USART                      |
| STATE   : code                ROUTINE : UT_IsChar                  |
+--------------------------------------------------------------------+

  PURPOSE : 

*/

GLOBAL int UT_IsChar (void)
{
  int ret;

  switch (m_mode) {
    case US_MODE_FILE:
      Sleep(FILE_SLOW_DOWN);
      while (feof(m_file))
      {
        Sleep(1000);
        fseek(m_file,0,SEEK_CUR);
      }
      ret=1;
      break;
    case US_MODE_95:
    case US_MODE_NT: {
      static COMSTAT stComStat;
      static DWORD   dwErrors;

      if (!initialized)
        return FALSE;

      waitForRCVBufferFull ();

      ClearCommError (hComDev, &dwErrors, &stComStat);

      ret= (stComStat.cbInQue > 0);
      break;
    }
    
    case US_MODE_SIM: {
      waitForRCVBufferFull ();
      ret = (*InCounter NEQ 0);

      if (ret EQ 0)
        readPointer = InBuffer;
      break;
    default: 
      ret = 0;
      break;
    }
  }
 
  return ret;
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : USART                      |
| STATE   : code                ROUTINE : UT_ReadChar                |
+--------------------------------------------------------------------+

  PURPOSE : 

*/

GLOBAL BYTE UT_ReadChar (void)
{
  BYTE ret=0;

  switch (m_mode) {
    case US_MODE_95:
    case US_MODE_NT: {

      BYTE  buffer[1];
      ULONG bytesRead;

      if (!initialized)
        return 0;

      sioRead (buffer, &bytesRead);

      if (!bytesRead)
        buffer[0] = 0xff;

      ret=buffer[0];
      break;
    }

    case US_MODE_SIM: {
      if (*InCounter NEQ 0)
      {
        ret = *readPointer++;

#ifdef DEBUG_USART
        {
          BYTE buf[20];
          sprintf (buf, "R[1 of %d]: ", *InCounter);
          write (usart_in,
            buf,
            strlen (buf));
          if (isprint (ret))
          {
            sprintf (buf, "%c\n", ret);
          }
          else
          {
            sprintf (buf, "(%02X)\n", ret);
          }
          write (usart_in,
            buf,
            strlen (buf));
		    }
#endif

        (*InCounter)--;

        if (*InCounter EQ 0)
        {
          readPointer = InBuffer;
          markRCVBufferEmpty ();
        }
      }
      break;
    default:
      break;
    }
  }
  return ret;
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : USART                      |
| STATE   : code                ROUTINE : UT_ReadNChars              |
+--------------------------------------------------------------------+

  PURPOSE : Called from the HISR - Reads a block of characters
            Parameters : buffer for holding received characters,
                         max. size of buffer
            Returns the number of characters read

*/

GLOBAL ULONG UT_ReadNChars (int usart_id, BYTE *buffer, ULONG bufferSize)
{
  ULONG   bytes;

  switch (m_mode) {
    case US_MODE_FILE:
      if (bufferSize>FILE_MAX_CHUNK)
      {
        bufferSize=FILE_MAX_CHUNK;
      }
      bytes=fread(buffer,1,bufferSize,m_file);
      break;
    case US_MODE_95:
    case US_MODE_NT: {

      COMSTAT stComStat;
      DWORD   dwErrors;

      if (!initialized)
        return 0L;

      ClearCommError (hComDev, &dwErrors, &stComStat);
  
      bytes = MINIMUM (stComStat.cbInQue, bufferSize);

      if (bytes EQ 0)
        return 0L;

      sioRead (buffer, &bytes);
      break;
    }

    case US_MODE_SIM: {
      if ((bytes = MINIMUM (*InCounter, bufferSize)) NEQ 0)
      {
#ifdef DEBUG_USART
        unsigned int i;
        char buf[50];
#endif
    
        memcpy (buffer, readPointer, bytes);

#ifdef DEBUG_USART
        sprintf (buf, "R[%d of %d]: ", bytes, *InCounter);
        write (usart_in, buf, strlen (buf));

        for (i=0; i<bytes; i++)
        {
          if (isprint (buffer[i]))
            sprintf (buf, "%c", buffer[i]);
          else
            sprintf (buf, "(%02X)", buffer[i]);
          write (usart_in,
             buf,
             strlen (buf));
        }
        write (usart_in, "\n", 1);
#endif

        (*InCounter) -= (USHORT)bytes;

        if (*InCounter EQ 0)
        {
          readPointer = InBuffer;
          markRCVBufferEmpty ();
        }
        else {
          readPointer += bytes;
        }
      }
      else {
        markRCVBufferEmpty ();
      }
      break;
    default: bytes = 0;
      break;
    }
  }

  return bytes;
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : USART                      |
| STATE   : code                ROUTINE : UT_WriteChar               |
+--------------------------------------------------------------------+

  PURPOSE : 

*/

GLOBAL void UT_WriteChar (int usart_id, char ch)
{
  switch (m_mode) {
    case US_MODE_95:
    case US_MODE_NT: {
      BYTE  buffer[1];
      ULONG bytesWritten = 1;

      if (!initialized)
        return;

      buffer[0] = (BYTE)ch;

      sioWrite (buffer, &bytesWritten);
      break;
    }

    case US_MODE_SIM: {
#ifdef DEBUG_USART
      char buf[50];
#endif

      if ( CTS == NULL || !*CTS)  /* no testtools connected */
      {
        return;
      }

      if (waitForSNDBufferEmpty () != 0)
      {
        markSNDBufferFull ();
        return;   /* we gave up sending to avoid dead lock */
      }
      
#ifdef DEBUG_USART
      sprintf (buf, "W[1]: %02X", ch);
      write (usart_out, buf, strlen (buf));
#endif

      *OutBuffer  = (UBYTE)ch;
      *OutCounter = 1;
   
      markSNDBufferFull ();
      break;
    default:
      break;
    }
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : USART                      |
| STATE   : code                ROUTINE : UT_WriteString             |
+--------------------------------------------------------------------+

  PURPOSE : 

*/

GLOBAL void UT_WriteString (int usart_id, char *s)
{
  switch (m_mode) {
    case US_MODE_95:
    case US_MODE_NT: {
      ULONG bytesWritten = strlen (s);

      if (!initialized)
        return;

      sioWrite ((BYTE *) s, &bytesWritten);
      break;
    }

    case US_MODE_SIM: {
      unsigned int numOfChars;
#ifdef DEBUG_USART
      int i;
      char buf[50];
#endif

      if ( CTS == NULL || !*CTS)  /* no testtools connected */
      {
        return;
      }

      if (waitForSNDBufferEmpty () != 0)
      {
        markSNDBufferFull ();
        return;   /* we gave up sending to avoid dead lock */
      }

      numOfChars = strlen (s);

      memcpy (OutBuffer, s, numOfChars);
      *OutCounter = numOfChars;

#ifdef DEBUG_USART
      sprintf (buf, "W[%d]:", numOfChars);
      write (usart_out, buf, strlen (buf));

      for (i=0; i<numOfChars; i++)
      {
        if (isprint (OutBuffer[i]))
          sprintf (buf, "%c", OutBuffer[i]);
        else
          sprintf (buf, "(%02X)", OutBuffer[i]);
        write (usart_out,
               buf,
               strlen (buf));

      }
      write (usart_out,"\n", 1);
#endif

      markSNDBufferFull ();
      break;
    default:
      break;
    }
  }
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : USART                      |
| STATE   : code                ROUTINE : UT_WriteNChars             |
+--------------------------------------------------------------------+

  PURPOSE : 

*/

GLOBAL void UT_WriteNChars (int usart_id, BYTE *s, unsigned int n)
{
  switch (m_mode) {
    case US_MODE_95:
    case US_MODE_NT: {
      ULONG bytesWritten = (ULONG) n;

      if (!initialized)
        return;

      if (!sioWrite ((BYTE *) s, &bytesWritten))
        fprintf (stderr, "USART: Error1\n");
      if (bytesWritten NEQ (ULONG) n)
        fprintf (stderr, "USART: Error2\n");
      break;
    }

    case US_MODE_SIM: {
#ifdef DEBUG_USART
      int i;
      char buf[50];
#endif
      
      if ( CTS == NULL || !*CTS)  /* no testtools connected */
      {
#ifdef DEBUG_USART
        printf("-");
#endif
        snd_cnt++;
        return;
      }

      if ( mem_closed == TRUE )
      {
        PrintToFile("USART:tried to write on closed memory (%d)\n",snd_cnt);
        return;
      }
      if (waitForSNDBufferEmpty () != 0)
      {
        markSNDBufferFull ();
        PrintToFile("USART: gave up sending\n");
        snd_cnt++;
        return;   /* we gave up sending to avoid dead lock */
      }
      memcpy (OutBuffer, s, n);
      *OutCounter = n;

#ifdef DEBUG_USART
      sprintf (buf, "W[%d]:", n);
      write (usart_out, buf, strlen (buf));

      for (i=0; i<n; i++)
      {
        if (isprint (OutBuffer[i]))
          sprintf (buf, "%c", OutBuffer[i]);
        else
          sprintf (buf, "(%02X)", OutBuffer[i]);
        write (usart_out,
               buf,
               strlen (buf));
      }
      write (usart_out,"\n", 1);
#endif

      if ( mem_closed == TRUE )
      {
        PrintToFile("USART: written on closed memory (%d)\n",snd_cnt);
        snd_cnt++;
        return;
      }
#ifdef DEBUG_USART
      printf("+");
#endif
      markSNDBufferFull ();
      snd_cnt++;
      break;
    default:
      break;
    }
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : USART                      |
| STATE   : code                ROUTINE : UT_SetFlowCtrl             |
+--------------------------------------------------------------------+

  PURPOSE : 

*/

GLOBAL void UT_SetFlowCtrl (char flowCtrl)
{
  switch (m_mode) {
    case US_MODE_95:
    case US_MODE_NT: {
      DCB stDCB;

      if (!GetCommState (hComDev, &stDCB))
        return;

      stDCB.DCBlength = sizeof (stDCB);  // sizeof(DCB) 

      switch (flowCtrl)
      {
        case 'N':
          stDCB.fOutxCtsFlow = FALSE;
          stDCB.fOutxDsrFlow = FALSE;
          stDCB.fDtrControl = DTR_CONTROL_DISABLE;
          stDCB.fRtsControl = RTS_CONTROL_DISABLE;
          break;
        case 'D':
          stDCB.fOutxCtsFlow = FALSE;
          stDCB.fOutxDsrFlow = TRUE;
          stDCB.fDtrControl = DTR_CONTROL_HANDSHAKE;
          stDCB.fRtsControl = RTS_CONTROL_DISABLE;
          stDCB.XonLim  = 0;
          stDCB.XoffLim = 50;
          break;
        case 'R':
          stDCB.fOutxCtsFlow = TRUE;
          stDCB.fOutxDsrFlow = FALSE;
          stDCB.fDtrControl = DTR_CONTROL_DISABLE;
          stDCB.fRtsControl = RTS_CONTROL_HANDSHAKE;
          stDCB.XonLim  = 0;
          stDCB.XoffLim = 50;
          break;
        case 'P':
          stDCB.fOutxCtsFlow = FALSE;
          stDCB.fOutxDsrFlow = FALSE;
          stDCB.fDtrControl = DTR_CONTROL_ENABLE;
          stDCB.fRtsControl = RTS_CONTROL_DISABLE;
          break;
        default:
          break;
      }

      SetCommState (hComDev, &stDCB);
      break;
    }

    case US_MODE_SIM:
      break;
    default:
      break;
  }
}
