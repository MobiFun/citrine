/*  
+-----------------------------------------------------------------------------
|  Project :  GSM-F&D (8411)
|  Modul   :  BAT library
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
|  Purpose :  API of the Generic Data aDapter (GDD) which connects the
|             application domain with the protocol stack.
+-----------------------------------------------------------------------------
*/

#ifndef GDD_H 
#define GDD_H

/*==== INCLUDES =============================================================*/

/*==== DEFINITIONS ==========================================================*/

/** Return and error values of GDD functions */
enum GDD_RESULT_CODES
{
  GDD_OK                    = 0,   /* OK, no error occured */
  GDD_BUSY                  = 1,   /* Currently busy */
  GDD_DIO_NOT_READY         = 2,   /* Dio not initialized or ready */
  GDD_NO_CONNECTION_SLOT    = 3,   /* Max number of connections already open */
  GDD_NO_BUF_AVAILABLE      = 4,   /* No buffer available (for sending) */
  GDD_INVALID_PARAMS        = 5,   /* Invalid input parameter */
  GDD_REQ_BUF_TOO_LARGE     = 6,   /* Requested buffer too large */
  GDD_INVALID_BUFFER        = 7,   /* Invalid buffer */
  GDD_ALREADY_INITIALIZED   = 97,  /* Instance already initialized */
  GDD_NO_MEMORY             = 98,  /* Memory allocation during initialization failed */
  GDD_INTERNAL_ERROR        = 99   /* General error code for all other errors */
};


/*==== CONSTS ===============================================================*/

/** This size that's required for maintenance of one single connection.
    Only needed if the application must allocated memory to be passed into
    gdd_init(). */
#define GDD_DIO_SIZEOF_CONDATA 64

/** Size of first buffer segment (used for internal purposes) */
#define GDD_DIO_PID_SEG_SIZE   2


/*==== TYPES =================================================================*/

/** Connection handle */
typedef U32 T_GDD_CON_HANDLE;


/** Identifier for the adapter instances - each instance corresponds to a
    DIO driver type */
typedef enum 
{
  GDD_INST_NONE = -1,
  GDD_INST_BAT     = 0,   /* The BAT library                */
  GDD_INST_APP,           /* Application                    */
  GDD_INST_TCP,           /* TCPIP entity                   */
  GDD_INST_SOCK,          /* Socket library                 */
  GDD_INST_SOCKCFG,       /* Socket library configuration   */
  
  /* Add new instances here and increase GDD_NUM_INSTS accordingly */     
  GDD_NUM_INSTS,
  /* Maximum number of instances is defined by GDD_MAX_INST_ID */
  GDD_MAX_INST_ID     = 255
} T_GDD_INST_ID;


/*
 *  GDD data format - identicial to DIOv4 data format.
 */
typedef struct
{
  U8             _align0;      /*<  0:  1> alignment */
  U8             _align1;      /*<  1:  1> alignment */
  U16            c_data;       /*<  2:  2> counter   */
  U8             *ptr_data;    /*<  4:  4> pointer to  pointer to the first
                                           byte of the data buffer segment */
} T_GDD_SEGMENT;

typedef struct
{
  U16            length;           /*<  0:  2> len of dio_ctrl */
  U8             _align0;          /*<  2:  1> alignment */
  U8             c_segment;        /*<  3:  1> counter of segments */
  T_GDD_SEGMENT  *ptr_gdd_segment; /*<  4:  4> pointer to Structured Element */
} T_GDD_BUF;



/** Capabilities of DIO BAT adapter, to be set by the client */
typedef struct
{
  U32               mtu_size;
} T_GDD_DIO_CAP;


/** Capabilities of the BAT adapter, to be set by the client */
typedef union
{
  T_GDD_DIO_CAP     dio_cap;
  /* Capabilities for future adapter implementations to be added here */
} T_GDD_CAP;


/** GDD result type */
typedef U16 GDD_RESULT;


/** Signal types */
typedef enum
{
  GDD_SIGTYPE_CONNECTION_OPENED  = 1, /* Connection is established and usable */
  GDD_SIGTYPE_CONNECTION_FAILED  = 2, /* Connection failed or closed down unexpectedly */
  GDD_SIGTYPE_SEND_BUF_AVAILABLE = 3, /* A new send buffer becomes available after an
                                         unsuccessfull call to gdd_get_send_buffer */
  GDD_SIGTYPE_BUF_SENT           = 4  /* Indicates that the last sent buffer has been
                                         successfully sent */ 
} T_GDD_SIGTYPE;



/** Data associated with a GDD signal */
typedef struct
{
  T_GDD_SIGTYPE     sig;
} T_GDD_SIGNAL;



/***************************************************************************
 * Call-back function definitions (to be implemented by the client)
 ***************************************************************************/
   

/*
+------------------------------------------------------------------------------
| Function    : gdd_signal_cb
+------------------------------------------------------------------------------
| Description : The adapter calls this call-back function to send received data
|               to the client. The client must copy the data contained in the
|               buffer, since the buffer will be invalidated upon returning
|               from this function.
|               Alternatively, the client can indicate that he is currently
|               busy. In this case, he has to call gdd_signal_ready_rcv() at a
|               later time to invoke this call-back function again.
|
| Parameters  : sig            - Signal
+------------------------------------------------------------------------------
*/
typedef void (* T_GDD_SIGNAL_CB)
( T_GDD_CON_HANDLE    con,
  T_GDD_SIGNAL        sig );

/*
+------------------------------------------------------------------------------
| Function    : gdd_received_data_cb
+------------------------------------------------------------------------------
| Description : The adapter calls this callback function to signal events to
|               the client.
|
| Parameters  : sig            - Signal
|
| Return      : GDD_OK         - Data was copied by client
|               GDD_BUSY       - Client was busy and could not receive data.
+------------------------------------------------------------------------------
*/
typedef GDD_RESULT (* T_GDD_RECEIVE_DATA_CB)
( T_GDD_CON_HANDLE    con,
  T_GDD_BUF *         buf );


/***************************************************************************
 * Function definitions
 ***************************************************************************/

/*
+------------------------------------------------------------------------------
| Function    : gdd_init
+------------------------------------------------------------------------------
| Description : This optional function can be used to provide the house-keeping
|               memory required for the given adapter instance. On platforms
|               where memory allocation is available, the function might be
|               called with the parameter mem set to 0. In this case, it is
|               just use to limit the number of connections via the parameter
|               num_con.
|
| Parameters  : inst           - Instance identifier of the adapter
|               mem            - Memory allocated by client for maintenance
|                                of the connections (Input parameter).
|                                If set to 0, the adapter will allocate
|                                the memory itself.
|               num_con        - Maximum number of connections
|
| Return      : GDD_OK         - Initialization successful
|               >GDD_OK        - Error (see manual for error codes)
+------------------------------------------------------------------------------
*/
typedef GDD_RESULT (* T_GDD_INIT)
( T_GDD_INST_ID     inst,
  void *            mem,
  U16               num_con);


/*
+------------------------------------------------------------------------------
| Function    : gdd_deinit
+------------------------------------------------------------------------------
| Description : This function de-initializes an adapter instance.
|               Any memory that the adapter has allocated internally is freed.
|
| Parameters  : inst           - Instance identifier of the adapter
|
| Return      : GDD_OK         - Initialization successful
|               >GDD_OK        - Error (see manual for error codes)
+------------------------------------------------------------------------------
*/
typedef GDD_RESULT (* T_GDD_DEINIT)
( T_GDD_INST_ID     inst);


/*
+------------------------------------------------------------------------------
| Function    : gdd_connect
+------------------------------------------------------------------------------
| Description : This function prepares a new connection for data transmission.
|               If the function succeeds, it will return a connection handle
|               in the parameter 'con_handle' to the caller. Each connection is
|               uniquely identified by its connection handle.
| 
| Note        : The connection cannot be used straight away. Instead, the
|               client receives a signal GDD_SIGTYPE_CONNECTION_OPENED when the
|               connection setup has been finished.
|
|               Each open connection should be closed using the function
|               gdd_close() when it is not used any longer.
|
| Parameters  : inst       - Instance identifier of the adapter
|               con_handle - New handle to the adapter passed back to the
|                            caller (output parameter).
|               cap        - capabilities of the adapter
|               rcv_cb     - Call-back for receiving data (provided by client)
|               sig_cb     - Call-back for signals (provided by client)
|
| Return      : GDD_OK     - Connecting is setup and being created. The client
|                            has to wait for signal GDD_SIGTYPE_CONNECTION_OPENED
|                            before the connection is usable.
|               >GDD_OK    - Error (see manual for error codes)
+------------------------------------------------------------------------------
*/
typedef GDD_RESULT (* T_GDD_CONNECT)
( T_GDD_INST_ID     inst,
  T_GDD_CON_HANDLE * con_handle,
  const T_GDD_CAP * cap,
  T_GDD_RECEIVE_DATA_CB rcv_cb,
  T_GDD_SIGNAL_CB   sig_cb );


/*
+------------------------------------------------------------------------------
| Function    : gdd_disconnect
+------------------------------------------------------------------------------
| Description : This function closes and deletes a connection.
|               The connection handle will not be valid any longer.
|
| Parameters  : con_handle     - Handle to the connection to be closed
|
| Return      : GDD_OK         - Initialization successful
|               >GDD_OK        - Error (see manual for error codes)
+------------------------------------------------------------------------------
*/
typedef GDD_RESULT (* T_GDD_DISCONNECT)
( T_GDD_CON_HANDLE  con_handle );


/*
+------------------------------------------------------------------------------
| Function    : gdd_get_send_buffer
+------------------------------------------------------------------------------
| Description : This function request a send buffer of a minimum size data_size
|               for sending on the connection specified with con. The buffer is
|               passed back in the parameter 'buf'.
|               After a successful call to this function, it is the
|               responsibility of the caller to subsequently fill the buffer
|               and call gdd_send_data(), before another call to
|               gdd_get_send_buffer() is done. In other words, the client must
|               make sure that calls to these two functions are synchronized.
|
| Parameters  : con_handle  - Connection handle
|               buf         - Pointer to pointer to send buffer (out parameter)
|               data_size   - Minimum size of requested buffer
|
| Return      : GDD_OK                - Initialization successful
|               >GDD_OK               - Error (see manual for error codes)
|               GDD_NO_BUF_AVAILABLE  - No buffer available. The client
|                                       will be notified with a signal
|                                       GDD_SIGTYPE_SEND_BUF_AVAILABLE when a
|                                       buffer is ready and can be requested.
+------------------------------------------------------------------------------
*/
typedef GDD_RESULT (* T_GDD_GET_SEND_BUFFER)
( T_GDD_CON_HANDLE  con_handle,
  T_GDD_BUF **      buf,
  U16               data_size );


/*
+------------------------------------------------------------------------------
| Function    : gdd_send_data
+------------------------------------------------------------------------------
| Description : This functions sends a buffer over the specified connection.
|               Please see also description of gdd_get_send_buffer().
|
|
| Parameters  : con_handle  - Connection handle
|               buf         - Buffer to be sent
|
| Return      : GDD_OK             - Initialization successful
|               >GDD_OK            - Error (see manual for error codes)
|               GDD_INVALID_BUFFER - The buffer pointed to is incorrect, i.e.
|                                    not the one that was previously requested
+------------------------------------------------------------------------------
*/
typedef GDD_RESULT (* T_GDD_SEND_DATA)
( T_GDD_CON_HANDLE  con,
  T_GDD_BUF *       buf);


/*
+------------------------------------------------------------------------------
| Function    : gdd_signal_ready_rcv
+------------------------------------------------------------------------------
| Description : This function is called from the client to indicate that he
|               is ready to receive. Typically, it is called from the client
|               when he becomes ready after a preceding result callback
|               (result_cb) was returned with GDD_BUSY.
|
| Parameters  : con_handle  - Connection handle
+------------------------------------------------------------------------------
*/
typedef void (* T_GDD_SIGNAL_READY_RCV)
( T_GDD_CON_HANDLE  con );


/** GDD Function pointer table */
typedef struct
{
  T_GDD_INIT             gdd_init;
  T_GDD_DEINIT           gdd_deinit;
  T_GDD_CONNECT          gdd_connect;
  T_GDD_DISCONNECT       gdd_disconnect;
  T_GDD_GET_SEND_BUFFER  gdd_get_send_buffer;
  T_GDD_SEND_DATA        gdd_send_data;
  T_GDD_SIGNAL_READY_RCV gdd_signal_ready_rcv;
} T_GDD_FUNC;


/** Needed for initialization of client of BAT adapter (BAT library) */
extern T_GDD_FUNC gdd_func_dio;


/***************************************************************************
 * Helper function definitions
 ***************************************************************************/

/*
+------------------------------------------------------------------------------
| Function    : gdd_write_buf
+------------------------------------------------------------------------------
| Description : Write data from a client buffer into into a GDD buffer
|               The datagram protocol ID for the following link layer (PPP) is
|               set internally to 0x21 (IPv4). If you need to set the protocol
|               ID to something else, please use the function 
|               gdd_write_buf_with_pid()..
|
| Parameters  : src_buf    - Source buffer (client owned)
|               src_size   - Size of source buffer
|               dest_buf   - Destination buffer
|
| Return      : 0 if all bytes have been written to the destination buffer.
|               If not all bytes could be written, the return value is larger
|               than 0 and denotes the number of remaining (unwritten) bytes.
|               A negative return value indicates an error.
+------------------------------------------------------------------------------
*/
S32 gdd_write_buf(const U8 * src_buf, U16 src_size, T_GDD_BUF * dest_buf);

/*
+------------------------------------------------------------------------------
| Description : Write user data and a datagram protocol ID used by the link 
|               layer (PPP) into a GDD buffer.
|
| Parameters  : src_buf     - Source buffer (client owned)
|               src_size    - Size of source buffer
|               dest_buf    - Destination buffer
|               protocol_id - datagram protocol identifier
|
| Return      : 0 if all bytes have been written to the destination buffer.
|               If not all bytes could be written, the return value is larger
|               than 0 and denotes the number of remaining (unwritten) bytes.
|               A negative return value indicates an error.
+------------------------------------------------------------------------------
*/
S32 gdd_write_buf_with_pid(const U8 * src_buf,  U16 src_size,
                           T_GDD_BUF * dest_buf,U8 protocol_id);


/*
+------------------------------------------------------------------------------
| Function    : gdd_read_buf
+------------------------------------------------------------------------------
| Description : Read data from a GDD buffer into a client buffer.
|
| Parameters  : src_buf    - Source buffer received from GDD
|               dest_buf   - Destination buffer (client owned)
|               dest_size  - Size of destination buffer
|
| Return      : 0 if all bytes have been copied into the destination buffer.
|               If not all bytes could be copied into the destination buffer,
|               the return value is larger than 0 and denotes the number of
|               remaining (unread) bytes. A negative return value indicase an
|               error.
+------------------------------------------------------------------------------
*/
S32 gdd_read_buf(const T_GDD_BUF * src_buf, U8 * dest_buf, U16 dest_size);


#endif /* GDD_H */
