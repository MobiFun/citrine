/* 
+------------------------------------------------------------------------------
|  File:       socket.h
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
|  Purpose :  This file implements the socket specific definitions to be used by applications in order to
|             set up a connection(GPRS or CSD) or create sockets for data receiption.
|             For a description of the socket API read g23m\condat\doc\8462_601.doc
+----------------------------------------------------------------------------- 
*/ 

#ifndef __SOCKET_H__
#define __SOCKET_H__

#include "typedefs.h"
#include "vsi.h"
#include "gsm.h"
#include "prim.h" // to get the DCM defines


/***************** Defines added for TCPIP testing with the application. ***********/

/*
 * Value constants for VAL_bearer_select
 */
#define DCM_SOCK_BEARER_ANY            (0x1)      /* DCM will decide which connection type to be used */
#define DCM_SOCK_BEARER_GPRS           (0x2)      /* Use a GPRS context as bearer, DCM will decide which GPRS settings to be used */
#define DCM_SOCK_BEARER_GSM            (0x3)      /* Use GSM data connection as bearer DCM will decide which GSM settings to be used */
#define DCM_SOCK_BEARER_USE_PROFILE    (0x4)      /* Use a specific data account for this connection */
#define DCM_SOCK_BEARER_AS_SPECIFIED   (0x5)      /* Use the data account information which is which is delivered within this signal */

/*
 * Value constants for VAL_authtype
 */
#define DCM_SOCK_AUTH_PAP              (0x1)      /* PAP authentification protocol  */
#define DCM_SOCK_AUTH_CHAP             (0x2)      /* CHAP authentification protocol !!! NOT SUPPORTED */
#define DCM_SOCK_AUTH_NO               (0x3)      /* No authentication              */

/*
 * user defined constants
 */
#define CDCM_APN_MAX_LEN               (0x64)     
#define CDCM_PHONE_NR_LEN              (0x54) 
#define CDCM_USER_MAX_LEN              (0x19)     
#define CDCM_PASSWORD_MAX_LEN          (0x19)     

/************************************************************************************/

/*
 * Maximum length of the full-qualified domain name of an Internet host
 */
#define SOCK_MAXHOSTNAMELEN 255

/* Maximum length (in octets) of a GPRS Access Point Name (APN). */
#define SOCK_MAX_APN_LEN CDCM_APN_MAX_LEN

/* Maximum length (in octets) of a telephone number. */
#define SOCK_MAX_PHONENUM_LEN CDCM_PHONE_NR_LEN

/* Maximum length (in octets) of a user id. */
#define SOCK_MAX_USERID_LEN CDCM_USER_MAX_LEN

/* Maximum length (in octets) of a user password. */
#define SOCK_MAX_PASSWORD_LEN CDCM_PASSWORD_MAX_LEN

/*
 * Type of an IP protocol
 * The values of this type are used as the 'ipproto' argument when creating
 * a socket to specify if a UDP socket or a TCP socket shall be created.
 */
typedef enum {
  SOCK_IPPROTO_TCP =  6,
  SOCK_IPPROTO_UDP = 17
} T_SOCK_IPPROTO;


/*
 * Type of a socket descriptor
 */
typedef unsigned long       T_SOCK_SOCKET;

/*
 * Type of an API instance
 */
typedef unsigned long       T_SOCK_API_INSTANCE;

/*
 * Type of an IP version 4 addresses in network byte order
 */
typedef unsigned long       T_SOCK_IPADDR;

#define SOCK_IPADDR_ANY    (T_SOCK_IPADDR)0  /*  Unspecified IP address */

/*
 * UDP or TCP port number in network byte order
 */
typedef unsigned short     T_SOCK_PORT;

#define SOCK_PORT_ANY      (T_SOCK_PORT)0    /* Unspecified port number */

/*
 * Convert U32 value from host byte order to network byte order
 */
#define SOCK_HTONL( x )  \
        ((U32)((((U32)( x )  & 0x000000ffU) << 24) | \
               (((U32)( x )  & 0x0000ff00U) <<  8) | \
               (((U32)( x )  & 0x00ff0000U) >>  8) | \
               (((U32)( x )  & 0xff000000U) >> 24)))

/*
 * Convert U16 value from host byte order to network byte order
 */
#define SOCK_HTONS( x )  \
        ((U16)((((U16)( x )  & 0x00ff) << 8) | \
               (((U16)( x )  & 0xff00) >> 8)))

/*
 * Convert U32 value from network byte order to host byte order
 */
#define SOCK_NTOHL( x ) \
        ((U32)((((U32)( x ) & 0x000000ffU) << 24) | \
               (((U32)( x ) & 0x0000ff00U) <<  8) | \
               (((U32)( x ) & 0x00ff0000U) >>  8) | \
               (((U32)( x ) & 0xff000000U) >> 24)))

/*
 * Convert U16 value from network byte order to host byte order
 */
#define SOCK_NTOHS( x ) \
        ((U16)((((U16)( x ) & 0x00ff) << 8) | \
               (((U16)( x ) & 0xff00) >> 8)))

/*
 * Construct IP address in network byte order from single octets.
 */
#define SOCK_MK_IPADDR( a, b, c, d ) \
        ((T_SOCK_IPADDR)((a << 24) | (b << 16) | (c << 8) | d))

/*
 * Type of a Socket API event
 */
typedef enum {
  SOCK_CREATE_CNF      =  1, /* Result event of sock_create() */
  SOCK_CLOSE_CNF       =  2, /* Result event of sock_close() */
  SOCK_BIND_CNF        =  3, /* Result event of sock_bind() */
  SOCK_LISTEN_CNF      =  4, /* Result event of sock_listen() */
  SOCK_CONNECT_CNF     =  5, /* Result event of sock_connect() */
  SOCK_SOCKNAME_CNF    =  6, /* Result event of sock_getsockname() */
  SOCK_PEERNAME_CNF    =  7, /* Result event of sock_getpeername() */
  SOCK_HOSTINFO_CNF    =  8, /* Result event of sock_gethostbyname() or sock_gethostbyaddr() */
  SOCK_MTU_SIZE_CNF    =  9, /* Result event of sock_get_mtu_size() */
  SOCK_RECV_IND        = 10, /* Network event: data has been received */
  SOCK_CONNECT_IND     = 11, /* Network event: an incoming connection has been accepted. */
  SOCK_CONN_CLOSED_IND = 12, /* Network event: connection has been closed by the remote peer */
  SOCK_ERROR_IND       = 13, /* Network event: an asynchronous error has occurred */
  SOCK_FLOW_READY_IND  = 14, /* Flow control: the API is ready to send data again */

  SOCK_OPEN_BEARER_CNF,      // Result Event of sock_open_bearer()
  SOCK_CLOSE_BEARER_CNF,     // Result event of sock_close_bearer()
  SOCK_BEARER_INFO_CNF,      // Result event of sock_bearer_info()
  SOCK_BAERER_CLOSED_IND     // The bearer connection has been closed
} T_SOCK_EVENTTYPE;

/*
 * Result codes of the API functions to indicate success or an error condition.
 * This type is used as the result code of the function and as the result value
 * in the associated event. It is also used for the error codes of a
 * 'SOCK_ERROR_IND' event
 */
typedef enum {
  SOCK_RESULT_OK                =  0, /* No problem detected. a corresponding primitive has been sent to the TCP/IP entity */
  SOCK_RESULT_INVALID_PARAMETER =  1, /* A parameter given to the function is invalid */
  SOCK_RESULT_INTERNAL_ERROR    =  2, /* An internal error has happened */
  SOCK_RESULT_ADDR_IN_USE       =  3, /* The address or port is already in use */
  SOCK_RESULT_OUT_OF_MEMORY     =  4, /* There is not enough memory to fulfill the request */
  SOCK_RESULT_NOT_SUPPORTED     =  5, /* The socket is not of a type that can support this operation */
  SOCK_RESULT_UNREACHABLE       =  6, /* The specified host cannot be reached */
  SOCK_RESULT_CONN_REFUSED      =  7, /* The connection to the specified address was refused by the remote host */
  SOCK_RESULT_TIMEOUT           =  8, /* The connection attempt timed out without establishing a connection */
  SOCK_RESULT_IS_CONNECTED      =  9, /* The request could not be fulfilled because the socket is already connected */
  SOCK_RESULT_HOST_NOT_FOUND    = 10, /* The specified host could not be found in the DNS */
  SOCK_RESULT_DNS_TEMP_ERROR    = 11, /* A temporary DNS error has occurred. Retrying the query may be successful */
  SOCK_RESULT_DNS_PERM_ERROR    = 12, /* A permanent DNS error has occurred */
  SOCK_RESULT_NO_IPADDR         = 13, /* The specified name has been found in the DNS, but no IP address is available */
  SOCK_RESULT_NOT_CONNECTED     = 14, /* The socket has not been connected yet */
  SOCK_RESULT_MSG_TOO_BIG       = 15, /* The size of the data buffer is too large for a UDP socket */
  SOCK_RESULT_CONN_RESET        = 16, /* The connection has been reset by the remote peer */
  SOCK_RESULT_CONN_ABORTED      = 17, /* The connection was aborted due to timeout or some other error condition */
  SOCK_RESULT_NO_BUFSPACE       = 18, /* Sending failed temporarily because the space to buffer the message was exhausted. */
  SOCK_RESULT_NETWORK_LOST, //	As a result code: The operation failed because TCP/IP's bearer connection has been disconnected.As an asynchronous event code: The bearer connection has been closed.
  SOCK_RESULT_NOT_READY, //	The operation failed because the bearer connection has not been opened.
  SOCK_RESULT_BEARER_NOT_READY, //	The bearer connection could not be opened because the mobile is not yet completely attached to the network. A retry at a later time may be successful.
  SOCK_RESULT_IN_PROGRESS, //	The operation failed because a similar operation is already in progress.
  SOCK_RESULT_BEARER_ACTIVE//	The operation failed because a bearer connection is already open.
} T_SOCK_RESULT;


/* Type of the bearer_select parameter of sock_open_bearer(), used to select the
 * type of the bearer connection to be opened by the Data Connection Manager 
 * (DCM), and of the bearer_type field of the T_SOCK_BEARER_INFO struct.
 */
typedef enum {
  SOCK_BEARER_ANY = DCM_SOCK_BEARER_ANY,
  SOCK_BEARER_GPRS = DCM_SOCK_BEARER_GPRS,
  SOCK_BEARER_GSM = DCM_SOCK_BEARER_GSM,
  SOCK_BEARER_USE_PROFILE = DCM_SOCK_BEARER_USE_PROFILE,
  SOCK_BEARER_AS_SPECIFIED = DCM_SOCK_BEARER_AS_SPECIFIED
} T_SOCK_BEARER_TYPE;

// FST: ?????
typedef enum {
  SOCK_AUTH_PAP = DCM_SOCK_AUTH_PAP,
  SOCK_AUTH_CHAP = DCM_SOCK_AUTH_CHAP,
  SOCK_AUTH_NO= DCM_SOCK_AUTH_NO
} T_SOCK_AUTHTYPE;

/*
 * Type of the generic event data structure passed
 * to the callback function on an event.
 * The actual event structure may be bigger(depending on its type),
 * but it will contain these fields at the beginning
 */
typedef struct {
  T_SOCK_EVENTTYPE   event_type;  /* Type of the event. */
  T_SOCK_RESULT      result;      /* Result code of the operation */
  T_SOCK_SOCKET      socket;      /* Socket for which the event occurred */
} T_SOCK_EVENTSTRUCT;

/*
 * Pointer to the callback function specified by the application
 */
typedef void (*T_SOCK_CALLBACK)(T_SOCK_EVENTSTRUCT* event, void *context);

/* System wide handle of a bearer connection.
 * Variables of this type are used as handles to identify a bearer connection. 
 * !! NOT NEEDED FOR CURRENT IMPLEMENTATION ONLY MENTIONED FOR FUTURE SUPPOSE.!!
 */
typedef U16 T_SOCK_BEARER_HANDLE;

typedef struct {
  T_SOCK_BEARER_HANDLE bearer_handle;
  T_HANDLE             app_handle;
  T_SOCK_BEARER_TYPE   bearer_type;
  BOOL                 apn_valid;
  char                 apn[SOCK_MAX_APN_LEN+1];
  BOOL                 phone_nr_valid;
  char                 phone_nr[SOCK_MAX_PHONENUM_LEN+1];
  BOOL                 user_id_valid;
  char                 user_id[SOCK_MAX_USERID_LEN+1];
  BOOL                 password_valid;
  char                 password[SOCK_MAX_PASSWORD_LEN+1];
  int                  cid;
  T_SOCK_IPADDR        ip_address;
  T_SOCK_IPADDR        dns1;
  T_SOCK_IPADDR        dns2;
  T_SOCK_IPADDR        gateway;
  T_SOCK_AUTHTYPE      authtype;
  BOOL                 data_compr;
  BOOL                 header_comp;
  int                  precedence;
  int                  delay;
  int                  reliability;
  int                  peak_throughput;
  int                  mean_througput;
  BOOL                 shareable;
} T_SOCK_BEARER_INFO;

/* ========================================================================== */
/* ==============================  Result Events from TCPIP ================= */

typedef struct {
  T_SOCK_EVENTTYPE     event_type;  /* Type of the event. */
  T_SOCK_RESULT        result;      /* Result code of the operation */
  T_SOCK_SOCKET        socket;      /* Socket for which the event occurred */
  T_SOCK_BEARER_HANDLE bearer_handle;
} T_SOCK_OPEN_BEARER_CNF;

typedef T_SOCK_EVENTSTRUCT T_SOCK_CLOSE_BEARER_CNF;

typedef struct {
  T_SOCK_EVENTTYPE   event_type;  /* Type of the event. */
  T_SOCK_RESULT      result;      /* Result code of the operation */
  T_SOCK_SOCKET      socket;      /* Socket for which the event occurred */
  T_SOCK_BEARER_INFO bearer_params;
} T_SOCK_BEARER_INFO_CNF;

typedef struct {
  T_SOCK_EVENTTYPE   event_type;  /* Type of the event. */
  T_SOCK_RESULT      result;      /* Result code of the operation */
  T_SOCK_SOCKET      socket;      /* Socket for which the event occurred */
  U32                dcm_error;   /* The parameter contains errors received from
                                   * PS (ETSI Spec 07.07) */
} T_SOCK_BAERER_CLOSED_IND;

typedef struct {
  T_SOCK_EVENTTYPE   event_type;  /* Type of the event. */
  T_SOCK_RESULT      result;      /* Result code of the operation */
  T_SOCK_SOCKET      socket;      /* Socket for which the event occurred */
} T_SOCK_CREATE_CNF;

typedef T_SOCK_EVENTSTRUCT      T_SOCK_CLOSE_CNF;
typedef T_SOCK_EVENTSTRUCT      T_SOCK_BIND_CNF;
typedef T_SOCK_EVENTSTRUCT      T_SOCK_LISTEN_CNF;
typedef T_SOCK_EVENTSTRUCT      T_SOCK_CONNECT_CNF;

typedef struct {
  T_SOCK_EVENTTYPE   event_type;  /* Type of the event. */
  T_SOCK_RESULT      result;      /* Result code of the operation */
  T_SOCK_SOCKET      socket;      /* Socket for which the event occurred */
  T_SOCK_IPADDR      ipaddr;      /* The local IP address of the socket */
  T_SOCK_PORT        port;        /* The local port number of the socket */
} T_SOCK_SOCKNAME_CNF;

typedef T_SOCK_SOCKNAME_CNF   T_SOCK_PEERNAME_CNF;

typedef struct {
  T_SOCK_EVENTTYPE   event_type;  /* Type of the event. */
  T_SOCK_RESULT      result;      /* Result code of the operation */
  T_SOCK_SOCKET      socket;      /* unused */
  char               hostname[SOCK_MAXHOSTNAMELEN+1]; /* The name of the host as 
                                                      a zero-terminated string */
  T_SOCK_IPADDR      ipaddr;      /* The local IP address of the socket */
} T_SOCK_HOSTINFO_CNF;

typedef struct {
  T_SOCK_EVENTTYPE   event_type;  /* Type of the event. */
  T_SOCK_RESULT      result;      /* Result code of the operation */
  T_SOCK_SOCKET      socket;      /* Socket for which the event occurred */
  U16                mtu_size;    /* MTU size */
} T_SOCK_MTU_SIZE_CNF;

typedef struct {
  T_SOCK_EVENTTYPE   event_type;  /* Type of the event. */
  T_SOCK_RESULT      result;      /* Result code of the operation */
  T_SOCK_SOCKET      socket;      /* Socket for which the event occurred */
  U32                data_length; /* Length of the data portion received. */
  char               *data_buffer; /*  Pointer to the data received. The application
                                       shall free this data buffer after use. */
} T_SOCK_RECV_IND;


typedef struct {
  T_SOCK_EVENTTYPE   event_type;  /* Type of the event. */
  T_SOCK_RESULT      result;      /* Result code of the operation */
  T_SOCK_SOCKET      socket;      /* Socket for which the event occurred */
  T_SOCK_SOCKET      new_socket;  /* New socket allocated for the connection. */
  T_SOCK_IPADDR      peer_ipaddr; /* IP address of the remote peer. */
  T_SOCK_PORT        peer_port;   /* Port number on the remote side. */
} T_SOCK_CONNECT_IND;

typedef T_SOCK_EVENTSTRUCT      T_SOCK_CONN_CLOSED_IND;
typedef T_SOCK_EVENTSTRUCT      T_SOCK_ERROR_IND;
typedef T_SOCK_EVENTSTRUCT      T_SOCK_FLOW_READY_IND;



/* ========================================================================== */
/* ================== Prototypes of socket API ============================== */


/* ******************* API administrative functions ************************* */

/*------------------------------------------------------------------------------
   Function     : sock_api_initialize
   Parameter    : - T_SOCK_API_INSTANCE * :
                    'The function returns an API instance value. The value is needed
                     for several API functions.'
                  - T_HANDLE :
                    'Application task handle as passed to pei_init()'
                  - char* :
                    'Name of the application entity as used with vsi_c_open().'
   Return       : The function returns TRUE if the initialization was successful.
   Description  : Initializes the socket interface API.
------------------------------------------------------------------------------*/
BOOL sock_api_initialize(T_SOCK_API_INSTANCE *api_instance, 
                         T_HANDLE app_handle,
                         char* app_name);


/*------------------------------------------------------------------------------
   Function     : sock_api_deinitialize
   Parameter    : - T_SOCK_API_INSTANCE :
                    'API instance value.'
   Return       : None
   Description  : Deinitializes the socket interface API. The function releases 
                  all associated memory.
------------------------------------------------------------------------------*/
void sock_api_deinitialize(T_SOCK_API_INSTANCE api_instance);


/*------------------------------------------------------------------------------
   Function     : sock_api_handles_primitive
   Parameter    : - T_SOCK_API_INSTANCE: API instance value
                  - T_PRIM:              Pointer to primitive received from 
                                         the primitive queue'
   Return       : The function returns TRUE if the primitive has been handled by 
                  this function.In this case the application should not do 
                  anything else with the primitive and should not call PFREE()
                  to free the memory block used for the primitive.
                  The function returns FALSE if the primitive has not been 
                  handled by this function. In this case the application should 
                  process the primitive as usual. 
   Description  : Handles primitives for the socket API.
                  To free the application from the handling of the event 
                  primitives sent by the TCP/IP entity, the Socket API provides
                  this function that checks if a primitive is to be handled by 
                  the Socket API and if it is, handles it. The application is 
                  supposed to call it immediately after receiving a primitive.
                  If the primitive is meant for the Socket API, it is handled by
                  this function. This will in most cases include calling the 
                  callback function supplied by the application to deliver an 
                  event. If the primitive is not meant for the Socket API,
                  no action is taken.
                  It is recommended to call this function early in the 
                  application entity's pei_primitive() function.
------------------------------------------------------------------------------*/
BOOL sock_api_handles_primitive(T_SOCK_API_INSTANCE api_instance, 
                                T_PRIM *prim);


/* ******************* Bearer related functions ***************************** */

/*------------------------------------------------------------------------------
   Function     : sock_open_bearer()
   Parameter    : - T_SOCK_API_INSTANCE: API instance value
                  - T_SOCK_BEARER_TYPE :  CSD or GPRS
                  - int:                  Number of the selected profile with a bearer 
                                          selection of SOCK_BEARER_USE_PROFILE.
                                          Unused in other cases.
                  - T_SOCK_BEARER_INFO:   requested parameters of the bearer connection
                  - T_SOCK_CALLBACK:      callback function to be called for return events.
                  - void*:                An arbitrary pointer to be passed to the 
                                          callback function when it is called
   Return       : T_SOCK_RESULT indicates successor a problem that happend
   Return Event : T_SOCK_OPEN_BEARER_CNF
   Description  : Opens a CSD or GPRS connection for use with TCP/IP.
                  This function a bearer connection for use with TCP/IP. It must be 
                  called after sock_api_initialize() and before any other 
                  TCP/IP-related functions.
------------------------------------------------------------------------------*/
T_SOCK_RESULT sock_open_bearer(T_SOCK_API_INSTANCE api_instance,
                               T_SOCK_BEARER_TYPE bearer_select,
                               int profile_number,
                               T_SOCK_BEARER_INFO *params,
                               T_SOCK_CALLBACK sock_cb,
                               void *context);


/*------------------------------------------------------------------------------
   Function     : sock_close_bearer()
   Parameter    : - T_SOCK_API_INSTANCE : API instance value
                  - T_SOCK_BEARER_HANDLE: returned by SOCK_OPEN_BEARER_CNF
                  - T_SOCK_CALLBACK:      Callback function to be called for return events.
                  - void*:                An arbitrary pointer to be passed to the 
                                          callback function when it is called
   Return       : T_SOCK_RESULT indicates successor a problem that happend
   Return Event : T_SOCK_CLOSE_BEARER_CNF
   Description  : Close a bearer connection that has been opened with sock_open_bearer().
------------------------------------------------------------------------------*/
T_SOCK_RESULT sock_close_bearer(T_SOCK_API_INSTANCE api_instance,
                                T_SOCK_BEARER_HANDLE bearer_handle,
                                T_SOCK_CALLBACK sock_cb,
                                void *context);


/*------------------------------------------------------------------------------
   Function     : sock_bearer_info()
   Parameter    : - T_SOCK_API_INSTANCE:  API instance value
                  - T_SOCK_BEARER_HANDLE: returned by SOCK_OPEN_BEARER_CNF
                  - T_SOCK_CALLBACK:      Callback function to be called for 
                                          return events.
                  - void*:                An arbitrary pointer to be passed to the 
                                          callback function when it is called
   Return       : T_SOCK_RESULT
   Return Event : T_SOCK_BEARER_INFO_CNF
   Description  : Get information about a bearer connection that has been opened 
                  with sock_open_bearer().
------------------------------------------------------------------------------*/
T_SOCK_RESULT sock_bearer_info(T_SOCK_API_INSTANCE api_instance,
                               T_SOCK_BEARER_HANDLE bearer_handle,
                               T_SOCK_CALLBACK sock_cb,
                               void *context);


/* ******************* Socket related functions ***************************** */

/*------------------------------------------------------------------------------
   Function     : sock_create
   Parameter    : - T_SOCK_API_INSTANCE :
                    'API instance value.'
                  - T_SOCK_IPPROTO :
                      'The protocol (UDP or TCP) to be used with this socket'
                  - T_SOCK_CALLBACK :
                      'The callback function to be called for events on this socket'
                  - void * :
                      'An arbitrary pointer to be passed to 
                       the callback function when it is called'
   Return       : (T_SOCK_RESULT)
   Return Event : 'SOCK_CREATE_CNF'
                    - T_SOCK_RESULT : Result code
                    - T_SOCK_SOCKET : The socket descriptor returned by the TCP/IP entity
   Description  : - Create a new UDP or TCP socket
                  - This function creates a socket that can subsequently be used with 
                    the other Socket API functions
------------------------------------------------------------------------------*/
T_SOCK_RESULT sock_create(T_SOCK_API_INSTANCE api_instance,
                          T_SOCK_IPPROTO ipproto, 
                          T_SOCK_CALLBACK callback,
                          void *context);


/*------------------------------------------------------------------------------
   Function     : sock_close
   Parameter    :  - T_SOCK_SOCKET :
                      'The socket descriptor to be closed'
   Return       : (T_SOCK_RESULT)
   Return Event : 'SOCK_CLOSE_CNF'
                    - T_SOCK_RESULT : Result code
                    - T_SOCK_SOCKET : The socket descriptor to be  closed
   Description  : - Close socket, shutdown connection if present
                  - This function closes a socket that has previously 
                    been created with sock_create(). 
                    If a connection is open for this socket, it will be closed. 
                    If this socket has listened for connections, 
                    no further connections will be accepted
   History      : 0001    03.08.11    shkim    Created
------------------------------------------------------------------------------*/
T_SOCK_RESULT sock_close(T_SOCK_SOCKET socket);


/*------------------------------------------------------------------------------
   Function     : sock_bind
   Parameter    :  - T_SOCK_SOCKET :
                      'The socket descriptor to be closed'
                   - T_SOCK_PORT   :
                      'The port number to bind the socket to'
   Return       : (T_SOCK_RESULT)
   Return Event : 'SOCK_BIND_CNF'
                    - T_SOCK_RESULT : Result code
                    - T_SOCK_SOCKET : The socket descriptor to be bound
   Description  : - Bind socket to a specific local port number
                  - This function binds a socket to the specified local port
   History      : 0001    03.08.11    shkim    Created
------------------------------------------------------------------------------*/
T_SOCK_RESULT sock_bind(T_SOCK_SOCKET socket,
                        T_SOCK_PORT port);


/*------------------------------------------------------------------------------
   Function     : sock_listen
   Parameter    :  - T_SOCK_SOCKET :
                      'The socket descriptor to listen on'
   Return       : (T_SOCK_RESULT)
   Return Event : 'SOCK_LISTEN_CNF'
                    - T_SOCK_RESULT : Result code
                    - T_SOCK_SOCKET : The socket descriptor to listen on
   Description  : - Accept TCP connections on this socket
                  - This function makes TCP/IP listen for 
                    TCP connections on this socket. 
                    The socket should have been bound to a specific port 
                    using sock_bind() before.
   History      : 0001    03.08.11    shkim    Created
------------------------------------------------------------------------------*/
T_SOCK_RESULT sock_listen(T_SOCK_SOCKET socket);


/*------------------------------------------------------------------------------
   Function     : sock_connect
   Parameter    :  - T_SOCK_SOCKET :
                      'The socket descriptor to connect'
                   - T_SOCK_IPADDR :
                      'The IP address to connect to'
                   - T_SOCK_PORT   :
                      'The port number to connect to'
   Return       : (T_SOCK_RESULT)
   Return Event : 'SOCK_CONNECT_CNF'
                    - T_SOCK_RESULT : Result code
                    - T_SOCK_SOCKET : The socket descriptor to connect
   Description  : - Connect the socket to a remote endpoint
                  - With TCP sockets, a TCP connection is established to 
                      the specified IP address and the specified port. 
                      The connection can then be used to send data using sock_send(); 
                      received data is indicated by a SOCK_RECV_IND event.
                    With UDP sockets, the specified IP address and port number 
                      are stored with the socket; 
                      subsequent UDP messages can be sent to this address 
                      using sock_send(); 
                      only messages from this address will be received.
   History      : 0001    03.08.11    shkim    Created
------------------------------------------------------------------------------*/
T_SOCK_RESULT sock_connect(T_SOCK_SOCKET socket, 
                           T_SOCK_IPADDR ipaddr,
                           T_SOCK_PORT port);


/*------------------------------------------------------------------------------
   Function     : sock_getsockname
   Parameter    :  - T_SOCK_SOCKET :
                      'The socket descriptor to retrieve information about'
   Return       : (T_SOCK_RESULT)
   Return Event : 'SOCK_SOCKNAME_CNF'
                    - T_SOCK_RESULT : Result code
                    - T_SOCK_SOCKET : The socket descriptor to connect
                    - T_SOCK_IPADDR : The local IP address of the socket
                    - T_SOCK_PORT   : The local port number of the socket
   Description  : - Retrieve local address information
                  - The function retrieves local address information of the socket. 
                    If the socket has not yet been bound to an address using sock_bind(), 
                    the port number is unspecified (SOCK_PORT_ANY)
   History      : 0001    03.08.11    shkim    Created
------------------------------------------------------------------------------*/
T_SOCK_RESULT sock_getsockname(T_SOCK_SOCKET socket);


/*------------------------------------------------------------------------------
   Function     : sock_getpeername
   Parameter    :  - T_SOCK_SOCKET :
                      'The socket descriptor to retrieve information about'
   Return       : (T_SOCK_RESULT)
   Return Event : 'SOCK_PEERNAME_CNF'
                    - T_SOCK_RESULT : Result code
                    - T_SOCK_SOCKET : The socket descriptor to connect
                    - T_SOCK_IPADDR : The IP address of the remote peer
                    - T_SOCK_PORT   : The port number at the remote peer
   Description  : - Retrieve remote address information
                  - The function retrieves address information of 
                    the connection at the remote peer.
                    If the socket is not connected, 
                    the IP address and port number are unspecified
------------------------------------------------------------------------------*/
T_SOCK_RESULT sock_getpeername(T_SOCK_SOCKET socket);


/*------------------------------------------------------------------------------
   Function     : sock_gethostbyname
   Parameter    : - T_SOCK_API_INSTANCE :
                      'API instance value.'
                  - char * :
                      'The name of the referenced host'
                   - T_SOCK_CALLBACK :
                      'The callback function to be called for the result event'
                   - void * :
                      'An arbitrary pointer to be passed to the callback
                       function when it is called'
   Return       : (T_SOCK_RESULT)
   Return Event : 'SOCK_HOSTINFO_CNF'
                    - T_SOCK_RESULT : Result code
                    - char hostname[SOCK_MAXHOSTNAMELEN+1] : The name of the 
                      host as a zero-terminated string
                    - T_SOCK_IPADDR : The IP address of the host
   Description  : - Get the IP address of a host
                  - The function queries the IP address information of 
                    the specified host from the DNS.
                    Because the function is not associated to any socket, 
                    a callback function and a context 
                    pointer must be specified separately
------------------------------------------------------------------------------*/
T_SOCK_RESULT sock_gethostbyname(T_SOCK_API_INSTANCE api_instance, 
                                 char *hostname,
                                 T_SOCK_CALLBACK callback, 
                                 void *context);


/*------------------------------------------------------------------------------
   Function     : sock_gethostbyaddr
   Parameter    : - T_SOCK_API_INSTANCE :
                      'API instance value.'
                  - T_SOCK_IPADDR  :
                      'The IP address of the referenced host'
                   - T_SOCK_CALLBACK  :
                      'The callback function to be called for the result event'
                   - void * :
                      'An arbitrary pointer to be passed to the callback function 
                      when it is called'
   Return       : (T_SOCK_RESULT)
   Return Event : 'SOCK_HOSTINFO_CNF'
                    - T_SOCK_RESULT : Result code
                    - char hostname[SOCK_MAXHOSTNAMELEN+1] : The name of the 
                      host as a zero-terminated string
                    - T_SOCK_IPADDR : The IP address of the host
   Description  : - Get the name of a host
                  - The function queries the hostname for the specified 
                    IP address from the DNS.
                    Because the function is not associated to any socket, 
                    a callback function and a context pointer must be specified separately
------------------------------------------------------------------------------*/
T_SOCK_RESULT sock_gethostbyaddr(T_SOCK_API_INSTANCE api_instance,
                                 T_SOCK_IPADDR ipaddr,
                                 T_SOCK_CALLBACK callback,
                                 void *context);


/*------------------------------------------------------------------------------
   Function     : sock_send
   Parameter    :  - T_SOCK_SOCKET :
                      'The socket to send the data on'
                   - char * :
                      'The data buffer to be sent'
                   - U16 :
                      'The length of the data buffer in bytes'
   Return       : (T_SOCK_RESULT)
   Return Event : None
   Description  : - Send data on a socket ( TCP only )
                  - The function sends the specified data buffer over the socket. 
                    The socket must be connected, 
                    i. e. it must have been connected to a remote peer 
                    using sock_connect() or been created when accepting 
                    a connection as indicated by SOCK_SONNECT_IND.
                    Implementation note: In order to send the payload data via DTI, 
                        the data must be copied into a DTI descriptor by the Socket API; 
                        there is no way to avoid the copy operation without 
                        putting the burden of knowing DTI-internal data 
                        structures on the application. 
                        It has been decided to pay the cost of the copy operation 
                        in order to free the application from this responsibility
------------------------------------------------------------------------------*/
T_SOCK_RESULT sock_send(T_SOCK_SOCKET socket, 
                        char *buffer, 
                        U16 buffer_length);


/*------------------------------------------------------------------------------
   Function     : sock_sendto
   Parameter    :  - T_SOCK_SOCKET :
                      'The socket to send the data on'
                   - char * :
                      'The data buffer to be sent'
                   - U16 :
                      'The length of the data buffer'
                   - T_SOCK_IPADDR :
                      'IP address of the host to send data to'
                   - T_SOCK_PORT :
                      'Remote port to send data to'
   Return       : (T_SOCK_RESULT)
   Return Event : None
   Description  : - Send data on a socket( UDP only )
                  - The function sends the specified data buffer 
                    over the socket to the specified address. The socket must be
                    a UDP socket.
                    Implementation note: In order to send the payload data via DTI, 
                        the data must be copied into a DTI descriptor by the Socket API; 
                        there is no way to avoid the copy operation without putting 
                        the burden of knowing DTI-internal data structures on 
                        the application. It has been decided to pay the cost of 
                        the copy operation in order to free the application from 
                        this responsibility
------------------------------------------------------------------------------*/
T_SOCK_RESULT sock_sendto(T_SOCK_SOCKET socket, 
                          char *buffer, 
                          U16 buffer_length, 
                          T_SOCK_IPADDR ipaddr, 
                          T_SOCK_PORT port);


/*------------------------------------------------------------------------------------
   Function     : sock_set_callback
   Parameter    : - T_SOCK_SOCKET :
                      'Socket to set callback and context for'
                  - T_SOCK_CALLBACK :
                      'New callback function for the socket'
                  - void * :
                      'New context pointer for the socket'
   Return       : (T_SOCK_RESULT)
   Return Event : None
   Description  : - Set a new callback function and context pointer for the socket
                  - The function defines a new callback function and a new context
                    pointer for the socket. All socket events after this call will be
                    delivered using the new callback function and the new context
                    pointer
------------------------------------------------------------------------------*/
T_SOCK_RESULT sock_set_callback(T_SOCK_SOCKET socket,
                                T_SOCK_CALLBACK callback, 
                                void *context);


/*------------------------------------------------------------------------------
   Function     : sock_get_callback
   Parameter    : - T_SOCK_SOCKET :
                      'Socket to get callback and context from'
                  - T_SOCK_CALLBACK * :
                      'Return callback function pointer for the socket'
                  - void ** :
                      'Return context pointer for the socket'
   Return       : (T_SOCK_RESULT)
   Return Event : None
   Description  : - Get callback function pointer and context pointer for the socket
                  - The function returns callback function pointer and context pointer
                    for the socket.
                    pointer
------------------------------------------------------------------------------*/
T_SOCK_RESULT sock_get_callback(T_SOCK_SOCKET socket,
                                T_SOCK_CALLBACK *callback_p, 
                                void **context_p);


/*------------------------------------------------------------------------------
   Function     : sock_flow_xoff
   Parameter    :  - T_SOCK_SOCKET :
                      'Socket to switch to "xoff" status'
   Return       : (T_SOCK_RESULT)
   Return Event : None
   Description  : - Flow control: make TCP/IP stop sending data
                  - This function makes the Socket API stop TCP/IP sending data.
                    If TCP/IP has already been stopped, the function has no effect.
   History      : 0001    03.08.11    shkim    Created
                  0002    03.09.12    STW      T_SOCK_RESULT added
------------------------------------------------------------------------------*/
T_SOCK_RESULT sock_flow_xoff(T_SOCK_SOCKET socket);


/*------------------------------------------------------------------------------
   Function     : sock_flow_xon
   Parameter    :  - T_SOCK_SOCKET :
                      'Socket to switch to "xon" status'
   Return       : (T_SOCK_RESULT)
   Return Event : None
   Description  : - Flow control: make TCP/IP resume sending data
                  - This function makes TCP/IP resume sending data.
                    If TCP/IP has not been stopped, the function has no effect
   History      : 0001    03.08.11    shkim    Created
                  0002    03.09.12    STW      T_SOCK_RESULT added
------------------------------------------------------------------------------*/
T_SOCK_RESULT sock_flow_xon(T_SOCK_SOCKET socket);


/*------------------------------------------------------------------------------
   Function     : sock_get_mtu_size
   Parameter    :  - T_SOCK_SOCKET :
                      'Socket to get MTU size from'
   Return       : (T_SOCK_RESULT)
   Return Event : 'SOCK_MTU_SIZE_CNF'
                    - T_SOCK_RESULT : Result code
                    - T_SOCK_SOCKET : The socket descriptor (unused).
                    - U16           : MTU size
   Description  : - Get MTU size of network connection
                  - The function retrieves the size of 
                    the Maximum Transfer Unit(MTU) of the network connection.
   History      : 0001    03.08.11    shkim    Created
                  0002    03.09.12    STW      T_SOCK_SOCKET added
------------------------------------------------------------------------------*/
T_SOCK_RESULT sock_get_mtu_size(T_SOCK_SOCKET socket);


#endif  /* __SOCKET_H__ */
