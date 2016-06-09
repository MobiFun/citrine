/* 
+------------------------------------------------------------------------------
|  File:       socket.c
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
|  Purpose :  This Modul contains the socket driver adaptation
+----------------------------------------------------------------------------- 
*/ 

#ifdef _VXWORKS_
#define GNU_COMPILER
#endif

#undef SOCK_TRACE

#undef SOCKET_DEBUG

/*==== INCLUDES =============================================================*/

#if defined _NUCLEUS_ || defined _TOOLS_   /* socket-driver running on windows OS */
/*lint -e717 suppress info do...while(0); */
/*lint -esym(550,rc) suppress info not accessed */
/*lint -e813, suppress Info 813: auto variable has size > 100 -> uncritical in this context */
/*lint -e801, suppress Info 801: Use of goto is deprecated */
  #include <windows.h>
  #include <winsock.h>
  #include <stdio.h>
  #ifdef SOCK_TRACE
    #include <stdio.h>
    #include <fcntl.h>
    #include <io.h>
  #endif  /* SOCK_TRACE */
#endif  /* #ifdef _NUCLEUS_ */

#ifdef _PSOS_                /* socket-driver running on pSOS */
  #define _PNA_30_BACK
  #include <psos.h>
  #include "bsp.h"
  #include <pna.h>
  #include <prepc.h>
  #include <rescfg.h>
#endif /* #ifdef _PSOS_ */

#ifdef _VXWORKS_
  #include "vxWorks.h"
  #include "sockLib.h"
  #include "inetLib.h"
  #include "ioLib.h"
  #include "selectLib.h"
  #include "errnoLib.h"
  #include "logLib.h"
/* undefine the MALLOC and FREE of VxWorks to avoid warnings */
#undef MALLOC
#undef FREE
#endif  /* _VXWORKS_ */

/* More operating systems go here */

#ifdef _LINUX_
  #include <sys/types.h>
  #include <sys/socket.h>
  #include <netinet/in.h>
  #include <netinet/tcp.h>
  #include <sys/ioctl.h>
  #include <errno.h>
  #include <netdb.h>
#endif

#ifdef _SOLARIS_
  #include <sys/types.h>
  #include <sys/socket.h>
  #include <netinet/in.h>
  #include <netinet/tcp.h>
  #include <sys/ioctl.h>
  #include <errno.h>
  #include <netdb.h>
  #include <sys/filio.h>
#endif

#ifndef OLD_FRAME
  #include "typedefs.h"
  #include "os.h"
#endif

#include "socket.h"
#include "tools.h"
#include "vsi.h"
#include "drvconf.h"
#include "tstheader.h"

/*==== DEFINITIONS ==========================================================*/

#define MAX_PENDING_CONNECTS  5          /* The backlog allowed for listen() */
#define SEND_TIMEOUTUSEC      500        /* minimal timeout value for sending*/
#define INVALID_HANDLE        NULL
#define INVALID_SIGNALTYPE    0
#define NO_FLAGS_SET          0          /* Used with recv()/send()          */
#define MAX_ETH_LEN           (1500-40)  /* Maximum size of a Ethernet packet without IP headers */
#define WRBUF_LEN             2048

#define ALLOWED_SOCKET_SIGNALS   (DRV_SIGTYPE_READ|DRV_SIGTYPE_CONNECT|DRV_SIGTYPE_DISCONNECT)

#if defined _NUCLEUS_ || defined _TOOLS_
    typedef ULONG SIZETYPE;
    #define GETTIME(t)           (t = GetTickCount())
    #define GETREADSIZE(sok,psiz) ioctlsocket(sok, FIONREAD,(ULONG*)psiz)
    #define READ_SOCKET(s,b,l)    recv((s),(b),(int)(l),NO_FLAGS_SET)
    #define WRITE_SOCKET(s,b,l)   send((s),(const char*)(b),(l),NO_FLAGS_SET)
    #define CLOSE_SOCKET(s)       closesocket(s)
#endif /* #ifdef _NUCLEUS_ */

#ifdef _PSOS_
  typedef int SOCKET;
  typedef int SIZETYPE;
  typedef struct hostent HOSTENT;
  #define h_addr  h_addr_list[0]
  typedef HOSTENT *  PHOSTENT;
  #define FAR /**/
  #define SOCKADDR_IN struct sockaddr_in
  #define SOCKET_ERROR   (-1)
  #define INVALID_SOCKET (-1)
  #define GETTIME(t)            os_GetTime (0, &(t))
  #define GETREADSIZE(sok,psiz) ioctl(sok, FIOREAD,(char*)psiz)
  #define READ_SOCKET(s,b,l)    recv((s),(b),(int)(l),NO_FLAGS_SET)
  #define WRITE_SOCKET(s,b,l)   send((s),(char *)(b),(int)(l),NO_FLAGS_SET)
  #define CLOSE_SOCKET(s)       close(s)
#endif /* #ifdef _PSOS_ */

#ifdef _VXWORKS_
  #define SOCKADDR_IN    struct sockaddr_in
  #define FAR /* nil */
  #define SOCKET_ERROR   ERROR
  #define INVALID_SOCKET ERROR
  typedef int SOCKET;
  typedef int SIZETYPE;
  #define GETTIME(t)            os_GetTime (0, &(t))
  #define GETREADSIZE(sok,psiz) ioctl(sok, FIONREAD, (int)psiz)
  #define READ_SOCKET(s,b,l)    recv((s), (b), (int)(l), NO_FLAGS_SET)
  #define WRITE_SOCKET(s,b,l)   send((s), (char *)(b), (int)(l), NO_FLAGS_SET)
  #define CLOSE_SOCKET(s)       close(s)
#endif /* _VXWORKS_ */

#if defined (_LINUX_) || defined (_SOLARIS_)
  #define SOCKADDR_IN    struct sockaddr_in
  #define FAR /* nil */
  #define SOCKET_ERROR   (-1)
  #define INVALID_SOCKET (-1)
  typedef int SIZETYPE;
  #define GETTIME(t)            os_GetTime (0, &(t))
  #define GETREADSIZE(sok,psiz) ioctl(sok, FIONREAD, (int)psiz)
  #define READ_SOCKET(s,b,l)    recv((s), (b), (int)(l), NO_FLAGS_SET)
  #define WRITE_SOCKET(s,b,l)   send((s), (char *)(b), (int)(l), NO_FLAGS_SET)
  #define CLOSE_SOCKET(s)       close(s)
#endif /* _VXWORKS_ */

#define PORT_NO              6392
#define TX_BUFFER_SIZE       8192
#define RX_BUFFER_SIZE       8192
#define TX_TIMEOUT_MSEC     10000
#define RX_TIMEOUT_MSEC     10000

#ifdef _VXWORKS_
#define SOCKET_PRIO    115
#define SOCKET_STACK  8192
#elif defined _PSOS_
#define SOCKET_PRIO    110
#define SOCKET_STACK  1024
#elif defined _TOOLS_
#define SOCKET_PRIO      1
#define SOCKET_STACK  1024
#else
#define SOCKET_PRIO      1
#define SOCKET_STACK  1024
#endif

#define TI_MODE     0x0001

/*==== TYPES ================================================================*/
typedef enum
{
  SST_PL0,     /* 0 bytes of packet length read*/
  SST_PL1,     /* 1 bytes of packet length read*/
  SST_DATA0,   /* 0 bytes of data read*/
  SST_DATAx    /* not all data read */
} T_SOCK_STATE;

typedef struct
{
  USHORT            Connect;
  SOCKET            Listener;
  SOCKET            Socket;
  USHORT            EnabledSignals;
  OS_HANDLE         ThreadID ;
  USHORT            SocketHandle;
  T_DRV_CB_FUNC     Callback;
} Client_Type;

/*==== EXTERNALS ============================================================*/
EXTERN BOOL           socket_flush;

/*==== LOCAL VARS ===========================================================*/

LOCAL socket_DCB_Type L_DCB ;
LOCAL Client_Type     L_ClientData ;
LOCAL BOOL            L_ThreadActive ;
LOCAL T_DRV_SIGNAL    SocketSignal;
LOCAL UBYTE    wrbuf[WRBUF_LEN+2];
LOCAL UBYTE   *wrbuf_pos = wrbuf;
T_SOCK_STATE   sock_state = SST_PL0;

#ifdef SOCK_TRACE
ULONG                 fh_sock = -1;
LOCAL char            stbuf[80];
#endif

LOCAL BOOL     isLittleEndian = TRUE;
int tst_socket; /* for psos */
int tst_socket_initialized; /* flag for pSOS */
int tst_socket_in_TxLen;    /* for pSOS */


/*==== DIAGNOSTICS ==========================================================*/

#ifdef SOCKET_DEBUG
#include <STDIO.H>
static char *logfile = "socket.log";
static char *bufferfullfile = "bufferfull.log";
static char *inprocessfile = "inprogress.log";
static char *noconnectfile1 = "noconnect1.log";
static char *noconnectfile2 = "noconnect2.log";
static char *noconnectfile3 = "noconnect3.log";
static char *readerrorfile = "readerror.log";
static FILE *fp = NULL;
#endif /* SOCKET_DEBUG */

/*==== END DIAGNOSTICS ======================================================*/

extern USHORT ext_data_pool_handle;

GLOBAL ULONG drv_socket_task_stack_size = SOCKET_STACK;
GLOBAL USHORT drv_socket_task_prio = SOCKET_PRIO;

/*==== PRIVATE FUNCTIONS ====================================================*/
/*
+------------------------------------------------------------------------------
|  Function    : L_CreateThread
+------------------------------------------------------------------------------
|  Description : This function creates a thread.
|
|  Parameters  : ThreadFunc        - pointer to the function beeing a thread
|
|  Return      : FALSE             - Thread not created
|                TRUE              - Thread create (Thread ID stored in
|                                    L_ClientData.ThreadID
|
+------------------------------------------------------------------------------
*/
#ifdef OLD_FRAME
BOOL L_CreateThread (USHORT (*ThreadFunc)(USHORT,ULONG))
{
  return (CreateThread ((LPSECURITY_ATTRIBUTES) NULL,
                0,
                (LPTHREAD_START_ROUTINE)ThreadFunc,
                (LPVOID) NULL,
                0,
                &L_ClientData.ThreadID) != NULL);
}
#else
BOOL L_CreateThread (void (*ThreadFunc)(T_HANDLE,ULONG))
{
  if ( os_CreateTask (0, (char*)"SOCKET", ThreadFunc, drv_socket_task_stack_size, drv_socket_task_prio, &L_ClientData.ThreadID, ext_data_pool_handle) < 0L )
    return FALSE;
  if ( os_StartTask (0, L_ClientData.ThreadID, 0) < 0 )
    return FALSE ;

  return TRUE;
}
#endif

/*
+------------------------------------------------------------------------------
|  Function    : L_SetSocketBuffer
+------------------------------------------------------------------------------
|  Description : This function sets the IP read and write buffer
|
|  Parameters  : in_TxLen          - size of transmission buffer
|                in_RxLen          - size of receiver buffer
|
|  Return      : FALSE             - either the read or write buffer could not
|                                    be set.
|                TRUE              - OK
|
+------------------------------------------------------------------------------
*/
LOCAL BOOL L_SetSocketBuffer (USHORT  in_TxLen, USHORT in_RxLen)
{
  int in_TxLen1 = (int) in_TxLen;
  int in_RxLen1 = (int) in_RxLen;
#ifndef _PSOS_
  int nodelay = TRUE;
#if defined (_VXWORKS_) || defined (_LINUX_) || defined (_SOLARIS_)
  struct linger nolinger = {0, 0};
#else
  int linger = TRUE;
#endif
  int rc=0;

  if (in_TxLen > 0)
  {
    if ( (rc=setsockopt(L_ClientData.Socket, SOL_SOCKET, SO_SNDBUF,
                   (char*)&in_TxLen1, sizeof(in_TxLen1))) != 0)
    {
#ifdef _VXWORKS_
      rc = errnoGet ();
#else
  #if defined (_LINUX_) || defined (_SOLARIS_)
      rc = errno;
  #else
      rc = WSAGetLastError();
  #endif
#endif
#if defined _NUCLEUS_ || defined _TOOLS_
      printf("SOCKET: setsockopt() returned error code %d\n", rc); 
#endif
      return FALSE ;
    }
  }
#if defined (_VXWORKS_) || defined (_LINUX_) || defined (_SOLARIS_)
  if (setsockopt(L_ClientData.Socket, SOL_SOCKET, SO_LINGER,
               (char*)&nolinger, sizeof(nolinger)))
#else
  if (setsockopt(L_ClientData.Socket, SOL_SOCKET, (int)SO_DONTLINGER,
               (char*)&linger, sizeof(linger)))
#endif
    return FALSE ;
  if (setsockopt(L_ClientData.Socket, IPPROTO_TCP, TCP_NODELAY,
               (char*)&nodelay, sizeof(nodelay)))
    return FALSE ;

#else /* PSOS */
  tst_socket_in_TxLen = in_TxLen;
#endif /* PSOS */
  if (in_RxLen > 0)
  {
    if (setsockopt(L_ClientData.Socket, SOL_SOCKET, SO_RCVBUF,
                   (char *) &in_RxLen1, sizeof(in_RxLen1)))
      return FALSE ;
  }

  return TRUE ;
}

/*
+------------------------------------------------------------------------------
|  Function    : L_Disconnect
+------------------------------------------------------------------------------
|  Description : This function is called when the connection to the peer entity
|                is lost. If the release signal is set a signal is generated.
|
|  Parameters  :  -
|
|  Return      :  -
|
+------------------------------------------------------------------------------
*/
LOCAL void L_Disconnect (void)
{

  L_ClientData.Connect = FALSE;
  if (L_ClientData.EnabledSignals != INVALID_SIGNALTYPE)
  {
    SocketSignal.SignalType = DRV_SIGTYPE_DISCONNECT;
    SocketSignal.DrvHandle = L_ClientData.SocketHandle;
    (*L_ClientData.Callback)(&SocketSignal) ;
  }
}

/*
+------------------------------------------------------------------------------
|  Function    : L_AsyncSelect
+------------------------------------------------------------------------------
|  Description : This function waits (blocking) for either something to read
|                or an execption on the socket.
|
|  Parameters  : -
|
|  Return      : FALSE             - failure on the socket
|                TRUE              - a read event was signalled
|
+------------------------------------------------------------------------------
*/
LOCAL BOOL L_AsyncSelect (void)
{
  fd_set  fd_r ;
  fd_set  fd_e ;
  int     status, rc;

  /* wait for possibility to read */
  FD_ZERO(&fd_r);
  FD_SET(L_ClientData.Socket, &fd_r);
  FD_ZERO(&fd_e);
  FD_SET(L_ClientData.Socket, &fd_e);

  status = select(FD_SETSIZE, &fd_r, NULL, &fd_e, (struct timeval *) 0);

  if (status > 0)
  {
    if (!FD_ISSET (L_ClientData.Socket, &fd_e))
    {
      if (L_ClientData.Callback != NULL)
      {
        if (FD_ISSET (L_ClientData.Socket, &fd_r))
        {
          SIZETYPE size ;

          if ((rc = GETREADSIZE( L_ClientData.Socket, &size)) != 0 || !size)
          {
#ifdef SOCK_TRACE
            if (fh_sock != -1)
            {
              char sstop[10];
              ULONG stop = GetTickCount();

              sprintf(sstop, "%03d:%03d",  (stop/1000) % 1000, stop % 1000);
              sprintf(stbuf, "reset at %s rc=%d size=%d left %d \n",
                              sstop, rc, size,
                              wrbuf_pos-wrbuf);
              write (fh_sock, stbuf, strlen(stbuf));
/*              close(fh_sock);
              fh_sock = -1; */
            }
#endif
            os_SuspendTask ( 0, 2 );
            CLOSE_SOCKET (L_ClientData.Socket);
            L_ClientData.Socket = INVALID_SOCKET;
            wrbuf_pos = wrbuf;
            sock_state = SST_PL0;
            return FALSE ;
          }

          /* Indicate that it is possible to read something */
          if (L_ClientData.EnabledSignals & DRV_SIGTYPE_READ)
          {
            SocketSignal.SignalType = DRV_SIGTYPE_READ;
            SocketSignal.DrvHandle = L_ClientData.SocketHandle;
            (*L_ClientData.Callback)(&SocketSignal) ;
          }
        }
      }
      return TRUE ;
    }
  }
  return FALSE;
}

/*
+------------------------------------------------------------------------------
|  Function    : L_ClientThread
+------------------------------------------------------------------------------
|  Description : This function is a thread used if the driver is configured to
|                be a client.
|
|  Parameters  : -
|
|  Return      : -
|
+------------------------------------------------------------------------------
*/
void L_ClientThread (T_HANDLE Handle, ULONG Value )
{
  L_ThreadActive = TRUE ;

  L_SetSocketBuffer (L_DCB.tx_buffer_size, L_DCB.rx_buffer_size) ;

  /*-------------------------------------
    Listen what's goning on on the socket
    -------------------------------------*/
  while (L_ThreadActive)
    if (!L_AsyncSelect())
    {
      L_ThreadActive = FALSE ;
      L_Disconnect() ;
    }
}


/*
+------------------------------------------------------------------------------
|  Function    : L_ServerThread
+------------------------------------------------------------------------------
|  Description : This function is a thread used if the driver is configured to
|                be a server. It serves only one client. It cycles between
|                two modes, wait for a client to connect and listening on the
|                connection for any further action.
|                The thread exits when the flag L_ThreadActive is set to false.
|
|  Parameters  : -
|
|  Return      : -
|
+------------------------------------------------------------------------------
*/
void L_ServerThread (T_HANDLE TaskHandle, ULONG Value)
{
  BOOL BufferSizeSet;
  SOCKADDR_IN local_sin;                      /* Local socket - internet style */
#ifdef _VXWORKS_
  SOCKADDR_IN clientAddr;                     /* client */
  int sockAddrSize = sizeof (struct sockaddr_in);

  memset ((char *) &local_sin, 0, sockAddrSize);
  local_sin.sin_len = (u_char) sockAddrSize;
#endif/* _VXWORKS_ */
  L_ClientData.Listener = socket (AF_INET, SOCK_STREAM, 0);
  if (L_ClientData.Listener == INVALID_SOCKET)
    goto error;

  if (L_DCB.port == SOCKET_INVALID_PORT)
    goto error;

  local_sin.sin_addr.s_addr = htonl(INADDR_ANY);
  local_sin.sin_family = AF_INET;
  local_sin.sin_port = htons(L_DCB.port);      /* Convert to network ordering */

  /*-------------------------------------------
    Associate an address with a socket. (bind)
    -------------------------------------------*/
#ifdef _PSOS_
  if (bind (L_ClientData.Listener, (struct sockaddr_in*) &local_sin, sizeof(local_sin)) != 0)
#else
#ifdef _VXWORKS_
  /* Look at the following cast of local_sin.
   * This is from VxWorks Network 5.4 Programmer’s Guide, example 7-1, page 131
   */
  if (bind (L_ClientData.Listener, (struct sockaddr*) &local_sin, sockAddrSize) == ERROR)
#else /* _VXWORKS_ */
  if (bind (L_ClientData.Listener, (const struct sockaddr FAR *) &local_sin, sizeof(local_sin)) != 0)
#endif /* _VXWORKS_ */
#endif
  {
    CLOSE_SOCKET (L_ClientData.Listener);
    goto error;
  }
#ifdef _VXWORKS_
  if (listen (L_ClientData.Listener, MAX_PENDING_CONNECTS) == ERROR)
#else /* _VXWORKS_ */
  if (listen (L_ClientData.Listener, MAX_PENDING_CONNECTS) != 0)
#endif /* _VXWORKS_ */
  {
    CLOSE_SOCKET (L_ClientData.Listener);
    goto error;
  }

  BufferSizeSet = FALSE ;
  L_ThreadActive = TRUE ;

  while (L_ThreadActive)
  {
    /*-------------------------------------
      Wait for somebody to connect
      -------------------------------------*/
    if (L_ClientData.Socket != INVALID_SOCKET)
    {
#ifdef SOCK_TRACE
      if (fh_sock != -1)
      {
        char sstop[10];
        ULONG stop = GetTickCount();

        sprintf(sstop, "%03d:%03d",  (stop/1000) % 1000, stop % 1000);
        sprintf(stbuf, "close at %s socket=%d\n",
                        sstop, L_ClientData.Socket);
        write (fh_sock, stbuf, strlen(stbuf));
/*      close(fh_sock);
        fh_sock = -1;   */
      }
#endif
      CLOSE_SOCKET (L_ClientData.Socket);
      L_ClientData.Socket = INVALID_SOCKET;
      wrbuf_pos = wrbuf;
      sock_state = SST_PL0;
    }
#ifdef _VXWORKS_
    if ((L_ClientData.Socket = accept (L_ClientData.Listener,
                                       (struct sockaddr *) &clientAddr,
                                       &sockAddrSize)) != ERROR)
#else /* _VXWORKS_ */
    L_ClientData.Socket = accept(L_ClientData.Listener, NULL, NULL) ;
    if (L_ClientData.Socket != INVALID_SOCKET)
#endif /* _VXWORKS_ */
    {
#ifdef SOCK_TRACE
      if (fh_sock == -1)
        fh_sock = open("SOCK_S.dbg", O_WRONLY| O_TEXT| O_TRUNC| O_CREAT, 0666);
      if (fh_sock != -1)
      {
        char sstop[10];
        ULONG stop = GetTickCount();

        sprintf(sstop, "%03d:%03d",  (stop/1000) % 1000, stop % 1000);
        sprintf(stbuf, "accept at %s socket=%d listener=%d\n",
                        sstop, L_ClientData.Socket, L_ClientData.Listener);
        write (fh_sock, stbuf, strlen(stbuf));
      }
#endif

#ifdef _PSOS_
      {
        ULONG tid;
        int err;
        /* for pSOS */
        /* wait for TST task */
        while( t_ident( FRM_TST_NAME, 0, &tid ) != 0 )
          tm_wkafter( 10 );

        tst_socket = shr_socket( L_ClientData.Socket, (int)tid );
        if( tst_socket < 0 )
        {
          err = errno;
          goto error;
        }
      }
#endif

      if (!BufferSizeSet)
        L_SetSocketBuffer (L_DCB.tx_buffer_size, L_DCB.rx_buffer_size) ;

      /* Signalisiere Connect */
      L_ClientData.Connect = TRUE;
      if (L_ClientData.EnabledSignals & DRV_SIGTYPE_CONNECT)
      {
        SocketSignal.SignalType = DRV_SIGTYPE_CONNECT;
        SocketSignal.DrvHandle = L_ClientData.SocketHandle;
        (*L_ClientData.Callback)(&SocketSignal) ;
      }

      /*-------------------------------------
        Listen what's goning on on the socket
        -------------------------------------*/
      while (L_ThreadActive)
        if (!L_AsyncSelect())
        {
          L_Disconnect() ;
          break ;
        }
    }
  }

error:
#ifndef OLD_FRAME
  for(;;)
    os_SuspendTask( 0, 1000 );
#endif
}

/*
+------------------------------------------------------------------------------
|  Function    : socket_Create
+------------------------------------------------------------------------------
|  Description : This function is used to set up the driver to act as a
|                server. The function tries to initialize the socket, creates
|                a thread in which it awaits first awaits the establishement
|                of a connection by a client. As soon as a client has
|                connected a signal (SOCKET_CONNECTED) is generated (call
|                socket_SetSignal() to activate a signal). From this time the
|                driver is able to send data (socket_write()) to the client and
|                to read received data (socket_read()). To get notified about
|                the reception of data the apropriate signal has to be set.
|                In the case of a successful completion the driver returns
|                DRV_OK.
|                If the driver is already busy DRV_INPROCESS is returned.
|                If the driver is not configured, the function returns
|                DRV_ NOTCONFIGURED.
|
|  Parameters  : -
|
|  Return      : DRV_OK             - Function successful
|                DRV_INPROCESS      - The driver is currently reading data.
|                                     The data is incomplete.
|                DRV_NOTCONFIGURED  - The driver is not yet configured
|                SOCKET_ERRORUNSPEC - Error occured during initialization
|
+------------------------------------------------------------------------------
*/
LOCAL UBYTE L_CreateServer (void)
{
  if (!L_CreateThread (L_ServerThread))
  {
    CLOSE_SOCKET (L_ClientData.Listener);
    return SOCKET_ERRUNSPEC;
  }
#ifdef _TOOLS_
  printf("SOCKET: now listening on port %i ...\n",L_DCB.port); 
#endif

  return DRV_OK ;
}
/*
+------------------------------------------------------------------------------
| Function    : socket_WriteToOS
+------------------------------------------------------------------------------
| Description : This function is used to write data to the driver of operating
|               system.
|               The parameter thr_BufferSize contains the number of
|               characters to write. In the case of a successful completion,
|               the function returns DRV_OK.
|
| Parameters  : in_BufferPtr          - This parameter points to the buffer
|                                       that is passed to the driver for
|                                       further processing
|               thr_BufferSize        - number of characters to write.
|
| Return      : DRV_OK            - Function successful
|               DRV_INPROCESS     - Driver is busy writing data
|               SOCKET_NOCONNECT  - Connection not available
|
+------------------------------------------------------------------------------
*/
LOCAL UBYTE socket_WriteToOS (void*  in_BufferPtr, USHORT thr_BufferSize)
{
  int             err;
  USHORT          c_written;
#ifndef _PSOS_
  fd_set          fd_w;
  fd_set          fd_e;
  struct timeval  tv;

  if (L_ClientData.Socket == INVALID_SOCKET)
    return SOCKET_NOCONNECT;

  if ( L_ClientData.Connect == FALSE )
    return SOCKET_NOCONNECT ;

  tv.tv_sec = (int)(L_DCB.tx_timeout_msec / 1000) ;
  tv.tv_usec = SEND_TIMEOUTUSEC ;

  FD_ZERO(&fd_w);
  FD_SET(L_ClientData.Socket, &fd_w);
  FD_ZERO(&fd_e);
  FD_SET(L_ClientData.Socket, &fd_e);

  if (select(FD_SETSIZE, NULL, &fd_w, &fd_e, &tv) <= 0)
  {
#ifdef SOCKET_DEBUG
    char buffer[200];
    char *ptr = in_BufferPtr;
    char c = ptr[21];
    fp = fopen(inprocessfile, "at");
    if ( *ptr == 'P' )
      ptr[21] = 0;
    else
      ptr[thr_BufferSize] = 0;
    strcpy (buffer, "errno:" );    
    sprintf (&buffer[6], "%8d", WSAGetLastError() );
    buffer[14] = ' ';
    memcpy (&buffer[15], ptr, (thr_BufferSize)+1);
    fprintf (fp, "%s\n", buffer );
    ptr[21] = c;
    fclose(fp);
#endif  /* SOCKET_DEBUG */
#ifdef _VXWORKS_
    err = errnoGet ();
#else /* _VXWORKS_ */
    err = errno;
    printf("SOCKET: socket write failed with error code: %d\n",err );
#endif /* _VXWORKS_ */

    return DRV_INPROCESS ;
  }

  if (FD_ISSET (L_ClientData.Socket, &fd_e))
  {
#ifdef SOCKET_DEBUG
    char buffer[200];
    char *ptr = in_BufferPtr;
    char c = ptr[21];
    fp = fopen(noconnectfile1, "at");
    if ( *ptr == 'P' )
      ptr[21] = 0;
    else
      ptr[thr_BufferSize] = 0;
    strcpy (buffer, "errno:" );
    sprintf (&buffer[6], "%8d", WSAGetLastError() );
    buffer[14] = ' ';
    memcpy (&buffer[15], ptr, (thr_BufferSize)+1);
    fprintf (fp, "%s\n", buffer );
    ptr[21] = c;
    fclose(fp);
#endif  /* SOCKET_DEBUG */
    return SOCKET_NOCONNECT ;
  }

  /*---------------------------------
    Send the data
    ---------------------------------*/
  c_written = (USHORT) WRITE_SOCKET(L_ClientData.Socket, in_BufferPtr, (USHORT)thr_BufferSize);

  if (c_written == (USHORT)SOCKET_ERROR || c_written != thr_BufferSize)
  {
#ifdef SOCKET_DEBUG
    char buffer[200];
    char *ptr = in_BufferPtr;
    char c = ptr[21];
    fp = fopen(noconnectfile2, "at");
    if ( *ptr == 'P' )
      ptr[21] = 0;
    else
      ptr[thr_BufferSize] = 0;
    strcpy (buffer, "errno:" );
    sprintf (&buffer[6], "%8d", WSAGetLastError() );
    buffer[14] = ' ';
    memcpy (&buffer[15], ptr, (thr_BufferSize)+1);
    fprintf (fp, "%s\n", buffer );
    ptr[21] = c;
    fclose(fp);
#endif  /* SOCKET_DEBUG */
    return SOCKET_NOCONNECT ;
  }

#ifdef SOCK_TRACE
  if (fh_sock != -1)
  {
    char sstop[10];
    ULONG stop = GetTickCount();

    sprintf(sstop, "%03d:%03d",  (stop/1000) % 1000, stop % 1000);
    sprintf(stbuf, "sent %d at %s\n",
                 thr_BufferSize,
                 sstop);
    write (fh_sock, stbuf, strlen(stbuf));
  }
#endif
  return DRV_OK ;

#else

  /* pSOS */

  /*---------------------------------
    Send the data
    ---------------------------------*/
  c_written = (USHORT) WRITE_SOCKET(tst_socket, in_BufferPtr, (USHORT)thr_BufferSize);

  if (c_written == (USHORT)SOCKET_ERROR || c_written != thr_BufferSize)
  {
    err = errno;
    return SOCKET_NOCONNECT ;
  }

  return DRV_OK ;
#endif  /* _PSOS_ */
}

/*==== PUBLIC FUNCTIONS =====================================================*/

/*
+------------------------------------------------------------------------------
|  Function    : socket_Exit
+------------------------------------------------------------------------------
|  Description : The function is called when the driver functionality is no
|                longer required. The function "de-allocates" the resources
|                and releases active connections. The driver terminates
|                regardless of any outstanding data to be sent.
|
|  Parameters  : -
|
|  Return      : -
|
+------------------------------------------------------------------------------
*/
void socket_Exit (void)
{
  os_DestroyTask ( 0, L_ClientData.ThreadID );
  socket_Close() ;
}


/*
+------------------------------------------------------------------------------
|  Function    : socket_Open
+------------------------------------------------------------------------------
|  Description : This function is used to establish a connection to server or
|                activate the driver to act as a server, using the settings of
|                the socket_DCB. A hostname must be specified to open a
|                connection to a server, in this case the driver runs in the
|                client mode. If no hostname is specified the driver will run in
|                server mode. In the server mode it serves a single client. As
|                soon as a client is connected the CONNECT signal is generated.
|                In case of a successful completion the driver is no able to send
|                data (socket_write()) to the peer entity and to read data
|                received from the peer entity (socket_read()). To get notified
|                about the reception of data the appropriate signal has to be set
|                (socket_SetSignal()).
|                In the case of a successful completion the driver returns DRV_OK.
|                If the driver is already busy DRV_INPROCESS is returned.
|                If the driver is not configured, the function returns
|                DRV_NOTCONFIGURED.
|                If an error occurs while establishing the requested mode, the
|                function returns SOCKET_ERRUNSPEC.
|
|  Parameters  : -
|
|  Return      : DRV_OK            - Function successful
|                DRV_INPROCESS     - The driver is currently reading data.
|                                    The data is incomplete.
|                DRV_NOTCONFIGURED - The driver is not yet configured
|                SOCKET_ERRUNSPEC  - Error occured during initialization
|
+------------------------------------------------------------------------------
*/
USHORT socket_Open (void)
{
  SOCKADDR_IN dest_sin;  /* DESTination Socket INternet */
  SOCKET      connectsocket;
#ifdef _VXWORKS_
  int sockAddrSize;
#else /* _VXWORKS_ */
  #if defined (_LINUX_) || defined (_SOLARIS_)
    struct hostent* phe;
  #else
    PHOSTENT    phe;
  #endif
#endif /* _VXWORKS_ */
  if (L_ThreadActive)
  {
    return DRV_INPROCESS ;
  }

  /*---------------------------------
    if no hostname is specified we
    open as a server
    ---------------------------------*/
  if (!*L_DCB.hostname)
  {
    return L_CreateServer() ;
  }

  connectsocket = socket (AF_INET, SOCK_STREAM, 0);
  if (connectsocket == INVALID_SOCKET)
  {
    return SOCKET_ERRUNSPEC;
  }

#ifdef _PSOS_

  if ( !gethostbyname(L_DCB.hostname, phe))
  {
    CLOSE_SOCKET(connectsocket);
    return SOCKET_ERRUNSPEC;
  }

#else /* _PSOS_ */
  #ifdef _VXWORKS_
    sockAddrSize = sizeof (struct sockaddr_in);
    memset((char *) &dest_sin, 0, sockAddrSize);
    dest_sin.sin_len = (u_char) sockAddrSize;
    dest_sin.sin_family = AF_INET;
    dest_sin.sin_port = htons(L_DCB.port);
    if (((dest_sin.sin_addr.s_addr = inet_addr (L_DCB.hostname)) == ERROR) &&
    ((dest_sin.sin_addr.s_addr = hostGetByName (L_DCB.hostname)) == ERROR))
    {
      CLOSE_SOCKET(connectsocket);
      return SOCKET_ERRUNSPEC;
    }
  #else /* _VXWORKS_ */
  phe = gethostbyname(L_DCB.hostname);
  if (!phe)
  {
#ifdef _TOOLS_
    printf("SOCKET: host %s not found ;-(\n",L_DCB.hostname); 
#endif
    CLOSE_SOCKET(connectsocket);
    return SOCKET_ERRUNSPEC;
  }
  #endif /* _VXWORKS_ */
#endif  /* _PSOS_ */

#ifndef _VXWORKS_
  memset(&dest_sin, 0, sizeof(struct sockaddr_in));
  memcpy((char*)&(dest_sin.sin_addr), phe->h_addr, (unsigned int)((int)(phe->h_length)));
  dest_sin.sin_family = AF_INET;
  if ( L_DCB.config & TI_MODE )
    dest_sin.sin_port = L_DCB.port;
  else
    dest_sin.sin_port = htons(L_DCB.port);
#endif /* _VXWORKS_ */

#ifdef _PSOS_
  if (connect (connectsocket, (struct sockaddr_in*) &dest_sin, sizeof(dest_sin)))
#else /* _PSOS_ */
  #ifdef _VXWORKS_
  if (connect (connectsocket, (struct sockaddr*) &dest_sin, sockAddrSize) == ERROR)
  #else /* _VXWORKS_ */
    #if defined (_LINUX_) || defined (_SOLARIS_)
      if (connect (connectsocket, (struct sockaddr*) &dest_sin,
                   sizeof(dest_sin)))
    #else
      if (connect (connectsocket, (const PSOCKADDR) &dest_sin,
                   sizeof(dest_sin)))
    #endif
  #endif /* _VXWORKS_ */
#endif /* defined(_PSOS_) || defined(_VXWORKS_) */
  {
#ifdef _TOOLS_
    printf("SOCKET: connection to %s on port %i failed ;-(\n",L_DCB.hostname,L_DCB.port); 
#endif
    CLOSE_SOCKET(connectsocket);
    return SOCKET_ERRUNSPEC;
  }
#ifdef _TOOLS_
  printf("SOCKET: successfully connected to %s on port %i\n",L_DCB.hostname,L_DCB.port); 
#endif

  if (!L_CreateThread (L_ClientThread))
  {
    CLOSE_SOCKET (L_ClientData.Listener);
    return SOCKET_ERRUNSPEC;
  }

  L_ClientData.Socket = connectsocket ;
  L_ClientData.Connect = TRUE;
  if (L_ClientData.EnabledSignals & DRV_SIGTYPE_CONNECT)
  {
    SocketSignal.SignalType = DRV_SIGTYPE_CONNECT;
    SocketSignal.DrvHandle = L_ClientData.SocketHandle;
    (*L_ClientData.Callback)(&SocketSignal) ;
  }
  return DRV_OK ;
}

/*
+------------------------------------------------------------------------------
|  Function    : socket_Close
+------------------------------------------------------------------------------
|  Description : This function is used by a client to close the connection or
|                by server to shut down the server functionality.
|                In case of a successful completion the connection is shutdown
|                and neither socket_Read nor socket_Write will be successful.
|                To get notified about the termination of a connection the
|                appropriate signal has to be set (socket_SetSignal()).
|
|  Parameters  : -
|
|  Return      : -
|
+------------------------------------------------------------------------------
*/
USHORT socket_Close (void)
{
  L_ThreadActive = FALSE ;

#ifdef SOCK_TRACE
  if (fh_sock != -1)
  {
    char sstop[10];
    ULONG stop = GetTickCount();

    sprintf(sstop, "%03d:%03d",  (stop/1000) % 1000, stop % 1000);
    sprintf(stbuf, "socket_Close at %s socket=%d listener=%d\n",
                    sstop, L_ClientData.Socket, L_ClientData.Listener);
    write (fh_sock, stbuf, strlen(stbuf));
  }
#endif
  if (L_ClientData.Socket != INVALID_SOCKET)
  {
    CLOSE_SOCKET (L_ClientData.Socket);
    L_ClientData.Socket = INVALID_SOCKET;
  }

  if (L_ClientData.Listener != INVALID_SOCKET)
    CLOSE_SOCKET (L_ClientData.Listener);

  L_ClientData.ThreadID = 0 ;
  return DRV_OK;
}

/*
+------------------------------------------------------------------------------
| Function    : socket_Read_with_Timeout
+------------------------------------------------------------------------------
| Description : This function is used to read data from the USART driver.
|               For more details see socket_Read()
|
| Return:       >= 0         - Number of bytes read
|               SOCKET_ERROR - error occurred
+------------------------------------------------------------------------------
*/
LOCAL ULONG socket_Read_with_Timeout( void* destBuf, ULONG reqLen )
{
  ULONG timeout,
        totalBytes = 0,
        stop_timeout ;
  int readBytes,
        attempts = 0;

  /* calculate timeout time */
  GETTIME( stop_timeout );
  stop_timeout = stop_timeout + L_DCB.rx_timeout_msec ;

  /* read nonblocking until requested data is read or timeout */
  do
  {
    readBytes = (SHORT)READ_SOCKET( L_ClientData.Socket,
                                    ((char *)destBuf) + totalBytes,
                                    reqLen - totalBytes );

    if( readBytes < 0 || readBytes == SOCKET_ERROR )
    {
#ifdef SOCKET_DEBUG
#ifndef _PSOS_
      fp = fopen(readerrorfile, "at");
      fprintf (fp, "WSAGetLastError returned: %8d", WSAGetLastError() );
      fclose(fp);
#endif
#endif  /* SOCKET_DEBUG */
      return 0;
    }
    else
    {
      totalBytes += (unsigned int)readBytes;

      GETTIME (timeout) ;

      if( totalBytes < reqLen )
      {
        if (attempts++)   /* try to get data by two consecutive accesses
                             then sleep (but this should not be necessary) */
        {
#ifdef OLD_FRAME
          Sleep (1) ;
#else
          os_SuspendTask ( 0, 1 );
#endif
        }
      }
      else
      {
        return totalBytes;
      }
    }
  } while (timeout < stop_timeout);

  return 0;
}

/*
+------------------------------------------------------------------------------
| Function    : socket_Read
+------------------------------------------------------------------------------
| Description : This function is used to read data from the USART driver. The
|               data is copied into the buffer to which out_BufferPtr points.
|               The parameter *thr_BufferSizePtr contains the size of the
|               buffer in characters.
|               In the case of a successful completion, the driver's buffer
|               is cleared. The driver keeps the data available when calling
|               the function drv_Look().
|               If the driver is not configured, the function returns
|               DRV_NOTCONFIGURED.
|
|               NOTE: When calling the function with a buffer size of 0, the
|                     function will return DRV_OK. The size of the buffer
|                     needed to store the available data is stored in the
|                     parameter *thr_BufferSizePtr. In this case, the
|                     out_BufferPtr can be set to NULL.
|
|
| Parameters  : out_BufferPtr         - This parameter points to the buffer
|                                       wherein the data is to be copied
|               thr_BufferSizePtr     - On call: number of characters to
|                                       read. If the function returns DRV_OK,
|                                       it contains the number of characters
|                                       read. If the function returns
|                                       DRV_INPROCESS, it contains 0.
|
| Return      : DRV_OK             - Function successful
|               DRV_INPROCESS      - The driver is currently reading data.
|                                    The data is incomplete.
|               DRV_NOTCONFIGURED  - The driver is not yet configured
|               SOCKET_NOCONNECT   - Connection not available
|
+------------------------------------------------------------------------------
*/
USHORT socket_Read (void* out_BufferPtr, ULONG*  thr_BufferSizePtr)
{
#ifdef _VXWORKS_
  SIZETYPE           pending_data_size = 0;
#else
  ULONG              pending_data_size = 0;
#endif
  ULONG              bytesToRead, bytesReq;
  int                rc;
  static USHORT      packet_size;
  union
  {
    USHORT s;
    UBYTE  b[2];
  } conv;
#ifdef SOCKET_DEBUG
  static ULONG BytesRead = 0;
#endif
#ifdef SOCK_TRACE
  static ULONG start;
  static ULONG lasttime = 0;
#endif

  bytesReq = *thr_BufferSizePtr;
  *thr_BufferSizePtr = 0;      /* no bytes returned yet */
  while ( (rc = (GETREADSIZE( L_ClientData.Socket, &pending_data_size ))) == 0 &&
          pending_data_size > 0 )
  {
    switch (sock_state)
    {
      case SST_PL0:
#ifdef SOCK_TRACE
        GETTIME (start) ;
#endif
        packet_size = 0;
        if (pending_data_size >= 2)
        {
          bytesToRead = 2;
          sock_state = SST_DATA0;
        }
        else
        {
          bytesToRead = 1;
          sock_state = SST_PL1;
        }
        if ( !(L_DCB.config & TI_MODE) )
        {
          if (socket_Read_with_Timeout(&packet_size, bytesToRead) != bytesToRead)
          {
            sock_state = SST_PL0;
            return SOCKET_NOCONNECT;
          }
        }
        break;
      case SST_PL1:
        if (socket_Read_with_Timeout(((char*)&packet_size)+1, 1) != 1)
        {
          sock_state = SST_PL0;
          return SOCKET_NOCONNECT;
        }
        sock_state = SST_DATA0;
        break;
      case SST_DATA0:
        if ( !(L_DCB.config & TI_MODE) && isLittleEndian )
        {
          conv.b[0] = *(((UBYTE*)&packet_size)+1); /* LSB */
          conv.b[1] = *  (UBYTE*)&packet_size;     /* MSB */
          packet_size = conv.s;
        }  
        /*lint -fallthrough*/
      case SST_DATAx:
        /* now read the packet payload or a part of it */
        if ( L_DCB.config & TI_MODE )
        {
          bytesToRead = (pending_data_size < bytesReq) ? pending_data_size : bytesReq;
        }
        else
        {
          bytesToRead = (packet_size < bytesReq) ? packet_size : bytesReq;
          if (pending_data_size < bytesToRead)
            bytesToRead = pending_data_size;
        }
        if( (*thr_BufferSizePtr =
              socket_Read_with_Timeout( ((char *)out_BufferPtr), bytesToRead )) !=
              bytesToRead )
        {
          *thr_BufferSizePtr = 0;
          if ( !(L_DCB.config & TI_MODE) )
            sock_state = SST_PL0;
          return SOCKET_NOCONNECT;
        }
#ifdef SOCK_TRACE
        if (fh_sock != -1 && sock_state == SST_DATA0 && bytesToRead >= 13)
        {
          char split[10+1];
          char sstart[20], sstop[20];
          static ULONG c_received = 0;
          ULONG len = packet_size;
          ULONG stop = GetTickCount();

          if (((char*)out_BufferPtr)[13] == 'T')
          {
            memcpy(split, ((char*)out_BufferPtr)+9, 7);
            split[7] = '\0';
          }
          else
          {
            memcpy(split, ((char*)out_BufferPtr)+9, 4);
            split[4] = '\0';
          }
          if (start != lasttime)
          {
            if (lasttime - start > 11)
              sprintf(sstart, "->%d %03d:%03d\n", c_received, (start/1000) % 1000, start % 1000);
            else
              sprintf(sstart, "%03d:%03d\n", (start/1000) % 1000, start % 1000);
            c_received = 0;
          }
          c_received += packet_size + 2;
          if (start != stop)
          {
            sprintf(sstop, "->%d %03d:%03d\n", c_received, (stop/1000) % 1000, stop % 1000);
            c_received = 0;
          }
          sprintf(stbuf, "%s%d %s\n%s",
                       (start != lasttime) ? sstart : "",
                       packet_size + 2, split,
                       (start != stop) ? sstop : "");
          write (fh_sock, stbuf, strlen(stbuf));
          lasttime = stop;
        }
#endif
        if ( !(L_DCB.config & TI_MODE) )
        {
          if (*thr_BufferSizePtr == packet_size)
            sock_state = SST_PL0;
          else
          {
            packet_size -= (USHORT)*thr_BufferSizePtr;
            sock_state = SST_DATAx;
          }
        }
        return DRV_OK;
    /*lint -e527 suppress Warning -- Unreachable */
        break;
    /*lint +e527 */
    }  /* switch */
  } /* while */
  if (rc)
  {
    sock_state = SST_PL0;
    return SOCKET_NOCONNECT;
  }
#ifdef SOCKET_DEBUG
  {
    static char Buffer[2000];
    memcpy (Buffer+BytesRead,out_BufferPtr,*thr_BufferSizePtr);
    BytesRead += *thr_BufferSizePtr;
    if ( (Buffer[0] == 'P') )
    {
      OS_TIME time;
      char c = Buffer[21];
      fp = fopen(logfile, "at");
      Buffer[21] = 0;
      os_GetTime ( 0, &time );
      fprintf (fp, "P%ld IN:  %s\n", time/10, &Buffer[9] );
      Buffer[21] = c;
      fclose(fp);
    }
  }

#endif
  return DRV_OK ;
}

/*
+------------------------------------------------------------------------------
| Function    : socket_Write
+------------------------------------------------------------------------------
| Description : This function is used to write data to the driver. The
|               parameter *thr_BufferSizePtr contains the number of
|               characters to write. In the case of a successful completion,
|               the function returns DRV_OK.
|               If the data cannot be written because the storage capacity of
|               the driver has been exhausted, the function returns
|               DRV_BUFFER_FULL and the maximum number of characters that can
|               be written in *thr_BufferSizePtr.
|               If the driver is currently busy writing data and therefore
|               cannot accept further data to be written, it returns
|               DRV_INPROCESS and sets the parameter *thr_BufferSizePtr to 0.
|               If the driver is not configured, the function returns
|               DRV_ NOTCONFIGURED.
|
|               NOTE:  When calling the function with a buffer size of 0, the
|                     function will return the number of characters that can be
|                     written in the parameter *thr_BufferSizePtr. In this
|                     case, the in_BufferPtr can be set to NULL.
|
| Parameters  : in_BufferPtr          - This parameter points to the buffer
|                                       that is passed to the driver for
|                                       further processing
|               thr_BufferSizePtr     - On call: number of characters to
|                                       write. If the function returns
|                                       DRV_BUFFER_FULL, it contains the
|                                       maximum number of characters that can
|                                       be written. If the function returns
|                                       DRV_OK, it contains the number of
|                                       characters written. If the function
|                                       returns DRV_INPROCESS, it contains 0.
|
|
| Return      : DRV_OK            - Function successful
|               DRV_BUFFER_FULL   - Not enough space
|               DRV_INPROCESS     - Driver is busy writing data
|               DRV_NOTCONFIGURED - The driver is not yet configured
|               SOCKET_NOCONNECT  - Connection not available
|
+------------------------------------------------------------------------------
*/
/*lint -esym(613,auxb) suppress warning possibly use off NULL pointer auxb */
/*lint -esym(644,auxb) suppress warning possibly not initialized */
/*lint -e668           suppress warning possibly passing NULL pointer to memcpy */
USHORT socket_Write (void*  in_BufferPtr, ULONG*  thr_BufferSizePtr)
{
  UBYTE    rc = DRV_OK;
  int   max_len, rest;
  char *   auxb;
  ULONG buffer_size;
  
  buffer_size = *thr_BufferSizePtr & ~PRIM_FLAG_MASK;

#ifndef _PSOS_
#ifdef SOCKET_DEBUG
  UBYTE Prim = 0;
  {
    char *ptr = in_BufferPtr;
    if ( *ptr == 'P' )
    {
      OS_TIME time;
      char c = ptr[21];
      fp = fopen(logfile, "at");
      ptr[21] = 0;
      os_GetTime ( 0, &time );
      fprintf (fp, "P%ld OUT: %s\n", time/10, &ptr[9] );
      ptr[21] = c;
      fclose(fp);
      Prim = 1;
    }
  }
#endif  /* SOCKET_DEBUG */
#else /* _PSOS */
  if( !tst_socket_initialized )
  {
    if (tst_socket_in_TxLen > 0)
    {
      int nodelay = TRUE;
      if (setsockopt(tst_socket, SOL_SOCKET, SO_SNDBUF,
                     (char*)&tst_socket_in_TxLen, sizeof(tst_socket_in_TxLen)))
        return DRV_INITFAILURE;
      if (setsockopt(tst_socket, IPPROTO_TCP, TCP_NODELAY,
                   (char*)&nodelay, sizeof(nodelay)))
        return DRV_INITFAILURE;
    }
    tst_socket_initialized = 1;
  }
#endif /* _PSOS_ */

  if ( L_DCB.config & TI_MODE )
  {
    /* add TI-MUX header */
    auxb=(char*)malloc(buffer_size+2);
    memcpy((void*)(auxb+2),in_BufferPtr,buffer_size);
    auxb[0]=19; /* assigned to L23 */
    auxb[1]=(char)buffer_size;
    in_BufferPtr=auxb;
    buffer_size+=2;
  }

  max_len = (L_DCB.tx_buffer_size < WRBUF_LEN) ? L_DCB.tx_buffer_size : WRBUF_LEN;
  if (max_len < (int)buffer_size)
  {
#ifndef _PSOS_
#ifdef SOCKET_DEBUG
    char buffer[200];
    char *ptr = in_BufferPtr;
    char c = ptr[21];
    fp = fopen(bufferfullfile, "at");
    if ( *ptr == 'P' )
      ptr[21] = 0;
    else
      ptr[buffer_size] = 0;
    strcpy (buffer, "errno:" );
    sprintf (&buffer[6], "%8d", WSAGetLastError() );
    buffer[14] = ' ';
    memcpy (&buffer[15], ptr, (buffer_size)+1);
    fprintf (fp, "%s\n", buffer );
    ptr[21] = c;
    fclose(fp);
#endif  /* SOCKET_DEBUG */
#endif  /* _PSOS_ */
    if ( L_DCB.config & TI_MODE )
      free(auxb);
    *thr_BufferSizePtr = (unsigned int)max_len |
                                      (*thr_BufferSizePtr & PRIM_FLAG_MASK) ;
    return DRV_BUFFER_FULL ;
  }

  rest = MAX_ETH_LEN - (wrbuf_pos - wrbuf);
  if (buffer_size + 2 >= (unsigned int)rest)    /* more than maximum ethernet packet size needed ?*/
  {
  /*---------------------------------
    send the buffer
    ---------------------------------*/
    rc = socket_WriteToOS(wrbuf, (USHORT)(wrbuf_pos - wrbuf));
    wrbuf_pos = wrbuf;

    if (rc != DRV_OK)
    {
      if ( L_DCB.config & TI_MODE )
        free(auxb);
      *thr_BufferSizePtr = (*thr_BufferSizePtr & PRIM_FLAG_MASK) ;
      return rc;
    }
  }

  if ( !(L_DCB.config & TI_MODE) )
  {
    /*------------------------------------------------
      put the size of the data into buffer (MSB first)
      ------------------------------------------------*/
    if (isLittleEndian)
    {
      *wrbuf_pos     = *(((UBYTE*)&buffer_size)+1); /* MSB */
      *(wrbuf_pos+1) = *  (UBYTE*)&buffer_size;     /* LSB */
    }
    else
      *((USHORT*)wrbuf_pos) = (USHORT)buffer_size;
    wrbuf_pos += 2;
  }
  /*---------------------------------
    put the data itself into buffer
    ---------------------------------*/
  memcpy(wrbuf_pos, in_BufferPtr, (size_t)buffer_size);
  wrbuf_pos += buffer_size;

  if (socket_flush)
  {
    /*---------------------------------
      send the buffer
      ---------------------------------*/
    rc = socket_WriteToOS(wrbuf, (USHORT)(wrbuf_pos - wrbuf));
    wrbuf_pos = wrbuf;
    socket_flush = 0;

    if (rc != DRV_OK)
    {
      *thr_BufferSizePtr = (*thr_BufferSizePtr & PRIM_FLAG_MASK) ;
    }
  }

  if ( L_DCB.config & TI_MODE )
  {
    free(auxb);
  }
  return rc ;
}

/*
+------------------------------------------------------------------------------
| Function    : socket_Flush
+------------------------------------------------------------------------------
| Description : This function flushes the socket write buffer where data
|               is stored until a complete packet can be sent or the
|               transmission is forced by an external setting of the
|               socket_flush flag. 
|
| Parameters  : ---
|
| Return      : DRV_OK                  - Function completed successfully
|
+------------------------------------------------------------------------------
*/
USHORT socket_Flush ( void )
{
ULONG len = 0;

  socket_flush = 1;
  return ( socket_Write(NULL, &len) );
}

/*
+------------------------------------------------------------------------------
| Function    : socket_SetSignal
+------------------------------------------------------------------------------
| Description : This function is used to define a single or multiple signals
|               that is/are indicated to the process when the event identified
|               in the signal information data type as SignalType occurs. The
|               USART uses only the standard signals defined in [C_8415.0026].
|               To remove a signal, call the function socket_ResetSignal().
|               If one of the parameters of the signal information data is
|               invalid, the function returns DRV_INVALID_PARAMS.
|               If no signal call-back function has been defined at the time
|               of initialization, the driver returns DRV_SIGFCT_NOTAVAILABLE.
|
| Parameters  : in_SignalIDPtr        - Pointer to the signal information
|                                       data
|
| Return      : DRV_OK                  - Function completed successfully
|               DRV_INVALID_PARAMS      - One or more parameters are out of
|                                         range or invalid
|               DRV_SIGFCT_NOTAVAILABLE - Event signaling functionality is
|                                         not available
|
+------------------------------------------------------------------------------
*/
USHORT socket_SetSignal (USHORT SignalType)
{
  if (L_ClientData.Callback == NULL)
    return DRV_SIGFCT_NOTAVAILABLE ;

  if (SignalType & ALLOWED_SOCKET_SIGNALS)
    L_ClientData.EnabledSignals |= SignalType;
  else
    return DRV_INVALID_PARAMS ;
  return DRV_OK ;
}

/*
+------------------------------------------------------------------------------
| Function    : socket_ResetSignal
+------------------------------------------------------------------------------
| Description : This function is used to remove previously set single or
|               multiple signals. The signals that are removed are identified
|               by the Signal Information Data element called SignalType. All
|               other elements of the Signal Information Data must be
|               identical to the signal(s) that are to be removed (process
|               handle and signal value). If the SignalID provided cannot be
|               located, the function returns DRV_INVALID_PARAMS.
|               If no signal call-back function has been defined at the time
|               of initialization, the driver returns DRV_SIGFCT_NOTAVAILABLE.
|
| Parameters  : in_SignalIDPtr        - Pointer to the signal information
|                                       data
|
| Return      : DRV_OK                    - Function completed successfully
|               DRV_INVALID_PARAMS        - One or more parameters are out of
|                                           range or invalid
|               DRV_SIGFCT_NOTAVAILABLE   - Event signaling functionality is
|                                           not available
|
+------------------------------------------------------------------------------
*/
USHORT socket_ResetSignal (USHORT SignalType)
{
  if (L_ClientData.Callback == NULL)
    return DRV_SIGFCT_NOTAVAILABLE ;

  if (SignalType & ALLOWED_SOCKET_SIGNALS)
    L_ClientData.EnabledSignals &= ~SignalType;
  else
    return DRV_INVALID_PARAMS ;

  return DRV_OK ;
}

/*
+------------------------------------------------------------------------------
| Function    : socket_SetConfig
+------------------------------------------------------------------------------
| Description : This function is used to configure the driver (port,
|               transmission rate, flow control, etc). The driver can be
|               configured at any one time before a connection is opened. The
|               parameters that can be configured are included in the USART's
|               device control block socket_DCB_Type. For detailed information
|               about the contents of the device control block, refer to
|               Chapter 2.1.1. If any value of the configuration is out of
|               range or invalid in combination with any other value of the
|               configuration, the function returns DRV_INVALID_PARAMS.
|               Call the socket_GetConfig() function to retrieve the driver's
|               configuration.
|               The driver needs to be configured after initialization. Only
|               the following functions can be called while the driver is not
|               configured: socket_Clear, socket_SetSignal and socket_GetSignal.
|               All other functions return DRV_NOTCONFIGURED.
|
| Parameters  : in_DCBPtr             - Pointer to the driver control block
|
| Return      : DRV_OK                    - Function completed successfully
|               DRV_INVALID_PARAMS        - One or more parameters are out of
|                                           range or invalid
|
+------------------------------------------------------------------------------
*/
USHORT socket_SetConfig (char* in_DCBPtr)
{
char token [SOCKET_MAX_LEN_HOSTNAME+1];
USHORT port;
unsigned int len;

  if ( (len = GetNextToken (in_DCBPtr, token, " #")) == 0)
  {
    return DRV_INVALID_PARAMS;
  }
  else
  {
    in_DCBPtr += (len+1);
  }

  port = (USHORT)atoi(token);

  if ( (len = GetNextToken (in_DCBPtr, token, " #")) == 0) 
  {
    return DRV_INVALID_PARAMS;
  }
  else
  {
    in_DCBPtr += (len+1);
  }

  if ( !strcmp ( DRV_TI_MODE, token ) )
  {
    L_DCB.config = TI_MODE;
    sock_state = SST_DATAx;
  }
  else if ( !strcmp ( DRV_DEFAULT, token ) )
  {
    L_DCB.config = 0;
    sock_state = SST_PL0;
  }
  else
    return DRV_INVALID_PARAMS;

  if ( (len = GetNextToken (in_DCBPtr, token, " #")) == 0 ) 
  {
#ifdef _TOOLS_
    gethostname (token,SOCKET_MAX_LEN_HOSTNAME);
#endif /* _TOOLS_ */
  }

  if (L_ThreadActive)
  {
    /* check if host is already used */
    if (strcmp(L_DCB.hostname, token)==0 && L_DCB.port==port) 
    {
#ifdef _TOOLS_
      printf("SOCKET: keeping connection to host %s on port %u\n",L_DCB.hostname,L_DCB.port); 
#endif
      return DRV_OK;
    }

    socket_Close();
  }
  
  L_DCB.port=port;
  strcpy ( L_DCB.hostname, token);
  
  if (socket_Open () != DRV_OK)
  {
    return DRV_INVALID_PARAMS ;
  }

  return DRV_OK ;
}

/*
+------------------------------------------------------------------------------
| Function    : socket_GetConfig
+------------------------------------------------------------------------------
| Description : This function is used to retrieve the configuration of the
|               driver. The configuration is returned in the driver control
|               block to which the pointer provided out_DCBPtr points. For
|               detailed information about the contents of the device control
|               block, refer to Chapter 2.1.1. The configuration can be
|               requested at any one time.
|               If the driver is not configured, the function returns
|               DRV_ NOTCONFIGURED.
|
| Parameters  : out_DCBPtr                - Pointer to the driver control block
|
| Return      : DRV_OK                    - Function completed successfully
|               DRV_INVALID_PARAMS        - One or more parameters are out of
|                                           range or invalid
|
+------------------------------------------------------------------------------
*/
USHORT socket_GetConfig (socket_DCB_Type*  out_DCBPtr)
{
  if (out_DCBPtr == NULL)
    return DRV_INVALID_PARAMS ;

  memcpy (out_DCBPtr, &L_DCB, sizeof (L_DCB)) ;
  return DRV_OK ;
}

/*
+------------------------------------------------------------------------------
| Function    : socket_Init
+------------------------------------------------------------------------------
| Description : The function initializes the internal data of the driver.
|               The function returns DRV_INITIALIZED if the driver has
|               already been initialized and is ready to be used or is
|               already in use. In the case of an initialization failure,
|               i.e. the driver cannot be used, the function returns
|               DRV_INITFAILURE.
|
| Parameters  : in_SignalCBPtr    - This parameter points to the function that
|                                   is called at the time an event that is to
|                                   be signaled occurs. This value can be set
|                                   to NULL if event signaling should not be
|                                   possible.
|
| Return      : DRV_OK            - Initialization successful
|               DRV_INITIALIZED   - Driver already initialized
|               DRV_INITFAILURE   - Initialization failed
|
+------------------------------------------------------------------------------
*/
GLOBAL USHORT socket_Init ( USHORT DrvHandle, T_DRV_CB_FUNC CallbackFunc, T_DRV_EXPORT const **DrvInfo )
{
static const T_DRV_EXPORT Socket_Info =
{
  "SOCKET",
  0,
  {
#ifdef _TOOLS_
    socket_Init,
#endif
    socket_Exit,
    socket_Read,
    socket_Write,
    NULL,
    NULL,
    socket_Flush,
    socket_SetSignal,
    socket_ResetSignal,
    socket_SetConfig,
    NULL,
    NULL,
  }
};
union
{
  USHORT s;
  UBYTE b[2];
} test;

#ifndef _PSOS_
#if defined (_VXWORKS_) || defined (_LINUX_) || defined (_SOLARIS_)
/* ToDo: vxWorks-libs initiieren
  sockLib
  inetLib
*/
#else /* _VXWORKS_ */
  WSADATA WSAData;
  if (WSAStartup(MAKEWORD(1,1), &WSAData))
    return DRV_INITFAILURE;
#endif /* _VXWORKS_ */
#endif

  test.s = 1;
  isLittleEndian = (test.b[0] == 1);

  L_ThreadActive = FALSE ;
  L_DCB.config = 0;
  L_DCB.port = 6392;
  L_DCB.tx_buffer_size = TX_BUFFER_SIZE;
  L_DCB.rx_buffer_size = RX_BUFFER_SIZE ;
  L_DCB.tx_timeout_msec = TX_TIMEOUT_MSEC;
  L_DCB.rx_timeout_msec = RX_TIMEOUT_MSEC;
  *L_DCB.hostname = 0 ;
  L_ClientData.Connect = FALSE;
  L_ClientData.SocketHandle = DrvHandle;
  L_ClientData.Socket = INVALID_SOCKET ;
  L_ClientData.Listener = INVALID_SOCKET ;
#ifdef _PSOS_
  tst_socket = INVALID_SOCKET ;
  tst_socket_initialized = 0;
  tst_socket_in_TxLen = 0;
#endif

  L_ClientData.Callback   = CallbackFunc ;
  L_ClientData.EnabledSignals = ALLOWED_SOCKET_SIGNALS ;
  *DrvInfo = &Socket_Info;
#ifdef SOCKET_DEBUG
  fp = fopen(logfile,"wt");
  fclose(fp);
  fp = fopen(inprocessfile,"wt");
  fclose(fp);
  fp = fopen(bufferfullfile,"wt");
  fclose(fp);
  fp = fopen(noconnectfile1,"wt");
  fclose(fp);
  fp = fopen(noconnectfile2,"wt");
  fclose(fp);
  fp = fopen(noconnectfile3,"wt");
  fclose(fp);
  fp = fopen(readerrorfile,"wt");
  fclose(fp);
#endif /* SOCKET_DEBUG */
  return DRV_OK ;
}

/*==== END OF FILE ==========================================================*/

