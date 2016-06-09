/* 
+------------------------------------------------------------------------------
|  File:       mux_pei.c
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
|  Purpose :  This module implements the process body interface
|             for the entity mux (used by tst for multiplexer driver).
+----------------------------------------------------------------------------- 
*/ 

#ifndef MUX_PEI_C
#define MUX_PEI_C
#endif

#ifndef ENTITY_MUX
#define ENTITY_MUX
#endif

#define ENTITY_UART /* test only */

/*==== INCLUDES =============================================================*/


#include "stdlib.h"
#include <stdio.h>
#include "string.h"
#include "typedefs.h"

#include "tools.h"
#include "os.h"
#include "gdi.h"
#include "vsi.h"
#include "glob_defs.h"
#include "frame.h"
#include "prim.h"       /* to get the definitions of used SAP and directions */
#include "gsm.h"
#include "pei.h"
#include "route.h"
#include "dti.h"        /* functionality of the dti library */
#include "mux.h"        /* to get the global entity definitions */

/*==== CONSTS ================================================================*/

#undef VSI_CALLER
#define VSI_CALLER        m_handle,

/* 
 * Wait as long as possible. This is the time in ms that is waited for a 
 * message in the input queue in the active variant. 
 * It can be decreased by the customer.  
 */   
#define MUX_TIMEOUT     0xffffffff   

/*==== TYPES =================================================================*/

typedef struct
{
  char              *version;
} T_MONITOR;

/*==== GLOBAL VARIABLES ======================================================*/
/*==== LOCALS ================================================================*/

static  int           m_first = 1;
static  T_MONITOR     m_mon;

static  int           m_uart_ready = 0;

static  T_desc2 *      m_send_data=NULL;
static  USHORT        m_send_len=0;

T_HANDLE              m_handle;
T_DRV_SIGNAL          m_signal;

GLOBAL DTI_HANDLE     mtst_hDTI;         /* handle for DTI library */
T_QMSG QueueMsg;
/*==== EXTERNAL DEFINED FUNCTIONS ==========================================*/

/*==== PRIMITIVE HANDLER FUNCTIONS ==========================================*/

static void primitive_not_supported (T_PRIM_HEADER *data);
static void dti_data_req (T_DTI2_DATA_REQ *ptr);

/*
 * Jumptables to primitive handler functions. One table per SAP.
 *
 * Use MAK_FUNC_0 for primitives which contains no SDU.
 * Use MAK_FUNC_S for primitives which contains a SDU.
 */

LOCAL const T_FUNC dti_ul_table[] = {
  MAK_FUNC_0( primitive_not_supported     ,    DTI2_CONNECT_REQ    ),  /* 3700x */
  MAK_FUNC_0( primitive_not_supported     ,    DTI2_CONNECT_RES    ),  /* 3701x */
  MAK_FUNC_0( primitive_not_supported     ,    DTI2_DISCONNECT_REQ ),  /* 3702x */
  MAK_FUNC_0( primitive_not_supported     ,    DTI2_GETDATA_REQ    ),  /* 3703x */
  MAK_FUNC_0( dti_data_req                ,    DTI2_DATA_REQ       )   /* 3704x */
#if defined (_SIMULATION_)
  ,
  MAK_FUNC_S( primitive_not_supported     ,   DTI2_DATA_TEST_REQ )
#endif  /* _SIMULATION_ */
};

LOCAL const T_FUNC dti_dl_table[] = {
  MAK_FUNC_0( pei_dti_dti_connect_ind     ,    DTI2_CONNECT_IND    ),  /* 7700x */
  MAK_FUNC_0( pei_dti_dti_connect_cnf     ,    DTI2_CONNECT_CNF    ),  /* 7701x */
  MAK_FUNC_0( pei_dti_dti_disconnect_ind  ,    DTI2_DISCONNECT_IND ),  /* 7702x */
  MAK_FUNC_0( pei_dti_dti_ready_ind       ,    DTI2_READY_IND      ),  /* 7703x */
  MAK_FUNC_0( pei_dti_dti_data_ind        ,    DTI2_DATA_IND       )   /* 7704x */
#if defined (_SIMULATION_)
  ,
  MAK_FUNC_S( pei_dti_dti_data_test_ind   ,    DTI2_DATA_TEST_IND )
#endif  /* _SIMULATION_ */
};


/*==== PRIVATE FUNCTIONS ====================================================*/

/*
+------------------------------------------------------------------------------
|  Function     :  primitive_not_supported
+------------------------------------------------------------------------------
|  Description  :  This function handles unsupported primitives.
|
|  Parameters   :  data - not used
|
|  Return       :  void
+------------------------------------------------------------------------------
*/
static void primitive_not_supported (T_PRIM_HEADER *data)
{
  //TRACE_FUNCTION ("primitive_not_supported");

  PFREE (data);
}

/*
+------------------------------------------------------------------------------
|  Function     :  pei_primitive
+------------------------------------------------------------------------------
|  Description  :  Process protocol specific primitive.
|
|  Parameters   :  prim      - pointer to the received primitive
|
|  Return       :  PEI_OK    - function succeeded
|                  PEI_ERROR - function failed
+------------------------------------------------------------------------------

                          |           |
                         TST         UART               UPLINK
                          |           |
                   +------v-----------v-------+
                   |                          |
                   |            MUX           |
                   |                          |
                   +-------------^------------+
                                 |
                                UART                    DOWNLINK
                                 |

*/
static short pei_primitive (void *ptr)
{
  /*
   * the following line of code causes a warning on tms470 compiler, that cannot be avoided
   * without changing the PEI interface. Warning will not cause a problem 
   */
  T_PRIM * prim=(T_PRIM*)ptr;
  
  //TRACE_FUNCTION ("pei_primitive");

  if (prim != NULL)
  {
    unsigned short     opc = (unsigned short)prim->custom.opc;
    unsigned short     n;
    const T_FUNC       *table;

    /*
     * This must be called for Partition Pool supervision. Will be replaced
     * by another macro some time.
     */
    VSI_PPM_REC (&prim->custom, __FILE__, __LINE__);

    //PTRACE_IN (opc);

    switch (opc & OPC_MASK)
    {
      case DTI_DL:                     /* defined in prim.h */
        table = dti_dl_table;
        n = TAB_SIZE (dti_dl_table);
        /* 
         * to be able to distinguish DTI1/DTI2 opcodes,
         * the ones for DTI2 start at 0x50
         */
        opc -= 0x50;
        break;
      case DTI_UL:                     /* defined in prim.h */
        table = dti_ul_table;
        n = TAB_SIZE (dti_ul_table);
        /* 
         * to be able to distinguish DTI1/DTI2 opcodes,
         * the ones for DTI2 start at 0x50
         */
        opc -= 0x50;
        break;
      default:
        table = NULL;
        n = 0;
        break;
    }

    if (table != NULL)
    {
      if ((opc & PRM_MASK) < n)
      {
        table += opc & PRM_MASK;
        P_SDU(prim) = table->soff ? 
          (T_sdu*) (((char*)&prim->data) + table->soff) : 0;
        P_LEN(prim) = table->size + sizeof (T_PRIM_HEADER);
        JUMP (table->func) (P2D(prim));
      }
      else
      {
        primitive_not_supported (P2D(prim));
      }
      return PEI_OK;
    }

    /*
     * primitive is not a GSM primitive - forward it to the environment
     */
    if (opc & SYS_MASK)
      vsi_c_primitive (VSI_CALLER prim);
    else
    {
      PFREE (P2D(prim));
      return PEI_ERROR;
    }
  }
  return PEI_OK;
}

/*
+------------------------------------------------------------------------------
|  Function     : pei_init
+------------------------------------------------------------------------------
|  Description  :  Initialize Protocol Stack Entity
|
|  Parameters   :  handle    - task handle
|
|  Return       :  PEI_OK    - entity initialised
|                  PEI_ERROR - entity not (yet) initialised
+------------------------------------------------------------------------------
*/
static short pei_init (T_HANDLE handle)
{
  /* Initialize task handle */
  m_handle = handle;
  mux_data.dti_state = CLOSED;

  /*
   * initialize dtilib for this entity
   */
  mtst_hDTI = dti_init(
    1, /* max simultaneous connections */
    handle,
    DTI_NO_TRACE,
    pei_sig_callback
    );
  if(!mtst_hDTI)
    return PEI_ERROR;

  /* set no suspend for us and TST */
  vsi_trcsuspend ( VSI_CALLER vsi_c_open (0,"MTST"), 0);
  vsi_trcsuspend ( VSI_CALLER vsi_c_open (0,"TST"), 0);

  return (PEI_OK);
}

/*
+------------------------------------------------------------------------------
|  Function     :  pei_timeout
+------------------------------------------------------------------------------
|  Description  :  Process timeout.
|
|  Parameters   :  index     - timer index
|
|  Return       :  PEI_OK    - timeout processed
|                  PEI_ERROR - timeout not processed
+------------------------------------------------------------------------------
*/
static short pei_timeout (unsigned short index)
{
  //TRACE_FUNCTION ("pei_timeout");

  /* Process timeout */
  switch (index)
  {
    case 0:
      /* Call of timeout routine */
      break;
    default:
      //TRACE_ERROR("Unknown Timeout");
      return PEI_ERROR;
  }

  return PEI_OK;
}

/*
+------------------------------------------------------------------------------
|  Function     :  pei_signal
+------------------------------------------------------------------------------
|  Description  :  Process signal.
|
|  Parameters   :  opc       - signal operation code
|                  data      - pointer to primitive
|
|  Return       :  PEI_OK    - signal processed
|                  PEI_ERROR - signal not processed
+------------------------------------------------------------------------------
*/
static short pei_signal (unsigned long opc, void* data)
{
  //TRACE_FUNCTION ("pei_signal");

  /* Process signal */
  switch (opc)
  {
    default:
      //TRACE_ERROR("Unknown Signal OPC");
      return PEI_ERROR;
  }

  return PEI_OK;
}

/*
+------------------------------------------------------------------------------
|  Function     :  pei_exit
+------------------------------------------------------------------------------
|  Description  :  Close Resources and terminate.
|
|  Parameters   :            - 
|
|  Return       :  PEI_OK    - exit sucessful
+------------------------------------------------------------------------------
*/
static short pei_exit (void)
{
  //TRACE_FUNCTION ("pei_exit");

  /*
   * Close communication channels
   */

  if (mux_data.dti_state NEQ CLOSED)
  {    
    dti_close
    (
      mtst_hDTI,
      MTST_DTI_DN_INSTANCE,
      MTST_DTI_DN_INTERFACE,
      MTST_DTI_DN_CHANNEL,
      FALSE
    );
    mux_data.dti_state = CLOSED;
  }

  /*
   * Shut down dtilib
   */
  dti_deinit(mtst_hDTI);

  return PEI_OK;
}


/* for test only - begin */
#define MTST_TRACE 0x4f20
typedef struct {
  char str[80];
} T_MTST_TRACE;

void trace_aci(const char* str) 
{
  PALLOC(ptr,MTST_TRACE);
  strcpy(ptr->str,str);

  PSEND(vsi_c_open(VSI_CALLER "MMI"), ptr);
}
/* for test only - end */

/*
+------------------------------------------------------------------------------
|  Function     :  pei_config
+------------------------------------------------------------------------------
|  Description  :  Dynamic Configuration.
|
|  Parameters   :  Buffer - configuration string (
|                    to start: "<DTI-partner> <tui> <c_id>"
|                    to stop:  "STOP")
|
|  Return       :  PEI_OK      - sucessful
|                  PEI_ERROR   - not successful
+------------------------------------------------------------------------------
*/
static short pei_config (char *Buffer)
{
  char token [10];
  char peer_name [10];

  USHORT len;

  ULONG link_id;

  //TRACE_FUNCTION ("pei_config");
  //TRACE_FUNCTION (Buffer);

  if ( ConfigTimer ( VSI_CALLER Buffer, NULL ) == VSI_OK )
    return PEI_OK;

  /*
  *  further dynamic configuration
  */
  
  /* interprete configuration string */
  if (!(len = GetNextToken (Buffer, token, " #")))
  {
    return DRV_INVALID_PARAMS;
  }
  else
  {
    Buffer += (len+1);
  }
  if (strcmp(token,"STOP")==0) 
  {
    dti_close
    (
      mtst_hDTI,
      MTST_DTI_DN_INSTANCE,
      MTST_DTI_DN_INTERFACE,
      MTST_DTI_DN_CHANNEL,
      FALSE
    );
    mux_data.dti_state=CLOSED;
    mux_data.h_comm_mtst=VSI_ERROR;
    m_uart_ready=0;
    if (mux_data.send_data_buf_count>0) {
      /* clear send_buffer */
      T_desc2 *d_new;
      T_desc2 *d=m_send_data;
      while (d)
      {
        d_new=(T_desc2 *)d->next;
        MFREE(d);
        d=d_new;
      }

      mux_data.send_data_buf_count=0;
    }

    return PEI_OK;
  }
  strcpy(peer_name, token);
  
  if (!(len = GetNextToken (Buffer, token, " #")))
  {
    return DRV_INVALID_PARAMS;
  }
  else
  {
    Buffer += (len+1);
  }
  link_id=atoi(token);

  if(
    dti_open (
      mtst_hDTI,
      MTST_DTI_DN_INSTANCE,
      MTST_DTI_DN_INTERFACE,
      MTST_DTI_DN_CHANNEL,
      0,
      DTI_CHANNEL_TO_LOWER_LAYER,
      DTI_QUEUE_UNUSED,
      DTI_VERSION_10,
      (U8*)peer_name,
      link_id
      ) EQ FALSE)
    return DRV_INVALID_PARAMS;

  /* reset send_data_buf counter */
  mux_data.send_data_buf_count=0;

  /* set internal communication handle */
  while ( (mux_data.h_comm_mtst=vsi_c_open (0, "MTST")) == VSI_ERROR)
  {
    vsi_t_sleep(0,100);
  };

  /* inform UART that we are ready to receive next data package */
  {
    dti_start
    (
      mtst_hDTI,
      MTST_DTI_DN_INSTANCE,
      MTST_DTI_DN_INTERFACE,
      MTST_DTI_DN_CHANNEL
    );
  }

  return PEI_OK;
}

/*
+------------------------------------------------------------------------------
|  Function     :  pei_monitor
+------------------------------------------------------------------------------
|  Description  :  Monitoring of physical Parameters.
|
|  Parameters   :  out_monitor - return the address of the data to be monitored
|
|  Return       :  PEI_OK      - sucessful (address in out_monitor is valid)
|                  PEI_ERROR   - not successful
+------------------------------------------------------------------------------
*/
static short pei_monitor (void ** out_monitor)
{
  //TRACE_FUNCTION ("pei_monitor");

  /*
   * Version = "0.S" (S = Step).
   */
  m_mon.version = "MTST 1.0";
  *out_monitor = &m_mon;

  return PEI_OK;
}

/*==== PUBLIC FUNCTIONS =====================================================*/

/*
+------------------------------------------------------------------------------
|  Function     :  pei_create
+------------------------------------------------------------------------------
|  Description  :  Create the Protocol Stack Entity.
|
|  Parameters   :  info        - Pointer to the structure of entity parameters
|
|  Return       :  PEI_OK      - entity created successfully
|                  
+------------------------------------------------------------------------------
*/
SHORT pei_create (T_PEI_INFO const **info)
{
  static T_PEI_INFO pei_info =
  {
    "MTST",         /* name */
    {              /* pei-table */
      pei_init,
      pei_exit,
      pei_primitive,
      pei_timeout,
      pei_signal,
      0,
      pei_config,
      pei_monitor
    },
    1024,          /* stack size */
    10,            /* queue entries */
    100,           /* priority (1->low, 255->high) */
    0,             /* number of timers */
    PASSIVE_BODY|COPY_BY_REF|TRC_NO_SUSPEND
                   /* flags: bit 0   active(0) body/passive(1) */
  };               /*        bit 1   com by copy(0)/reference(1) */

  //TRACE_FUNCTION ("pei_create");

  /*
   * Close Resources if open
   */
  if (!m_first) 
  {
    pei_exit();
  }

  m_first=0;

  /*
   * Export startup configuration data
   */
  *info = &pei_info;

  return PEI_OK;
}
/***********+++++-----------------+++++++*******++----++**********/

/*
+------------------------------------------------------------------------------
|  Function     :  sig_dti_pei_tx_buffer_ready_ind
+------------------------------------------------------------------------------
|  Description  :  sent from UART to indicate that it is ready to
|                  receive new data via DTI_DATA_REQ
|
|  Parameters   :
|
|  Return       :  
|                  
+------------------------------------------------------------------------------
*/
GLOBAL const void sig_dti_pei_tx_buffer_ready_ind ()
{
  m_uart_ready++;
}

/***********+++++-----------------+++++++*******++----++**********/

/*
+------------------------------------------------------------------------------
|  Function     :  dti_data_req
+------------------------------------------------------------------------------
|  Description  :  sent from TST to deliver new data to MTST
|
|  Parameters   :  ptr         - Pointer to the structure of the primitive
|
|  Return       :  
|                  
+------------------------------------------------------------------------------
*/
static void dti_data_req (T_DTI2_DATA_REQ *ptr)
{
  T_desc2 *d=0;

  /* PPASS without traces */
  T_DTI2_DATA_IND *prim = (T_DTI2_DATA_IND*)ptr;
  D_OPC(prim) = (DTI2_DATA_IND);

  if (!m_uart_ready)
  {
    if (mux_data.send_data_buf_count > MAX_SEND_BUF_COUNT-2) 
    {
      /* free data */
      T_desc2 *d_new;
      T_desc2 *d=(T_desc2 *)prim->desc_list2.first;
      while (d)
      {
        d_new=(T_desc2 *)d->next;
        MFREE(d);
        d=d_new;
      }

      if (mux_data.send_data_buf_count == MAX_SEND_BUF_COUNT) 
      {
        /* buffer full -> free prim and exit */
        PFREE(prim);
        return; 
      } 
      else
      {
        /* buffer almost full -> send SYST information */
        char infostr[]="\x02T0036001FMTSTPCO ERROR: SEND-BUFFER FULL\x0a"; 
        USHORT len=sizeof(infostr)-1;
        MALLOC(d,(USHORT)(sizeof(T_desc2)+len-1));
        prim->desc_list2.first=(ULONG)d;
        prim->desc_list2.list_len=len;
        d->next = 0;
        d->len = len;
        d->size = len;
        d->offset = 0;        
        memcpy(d->buffer,infostr,len);
      }
    }

    if (m_send_data)
    {
      /* find last send_data descriptor */
      d=m_send_data;
      while (d->next)
      {
        d=(T_desc2 *)d->next;
      }
      /* concat new data */
      d->next=prim->desc_list2.first;
      m_send_len+=prim->desc_list2.list_len;
    } 
    else 
    {
      m_send_data=(T_desc2 *)prim->desc_list2.first;
      d=m_send_data;
      while (d)
      {
        m_send_len+=d->len;
        d=(T_desc2 *)d->next;
      }
    }
    mux_data.send_data_buf_count++;

    /* free prim */
    PFREE(prim);
    return;
  }

  m_uart_ready--;
  mux_data.send_data_buf_count=0;

  if (m_send_data)
  {
    /* find last data descriptor in stored data */
    d=m_send_data;
    while (d->next)
    {
      d=(T_desc2 *)d->next;
    }
    /* concat new data to stored data */
    d->next=prim->desc_list2.first;
    prim->desc_list2.first=(ULONG)m_send_data;
    prim->desc_list2.list_len+=m_send_len;

    /* clear send_data */
    m_send_data=(T_desc2 *)0;
    m_send_len=0;
  }

  /* send prim */
  dti_send_data
  (
    mtst_hDTI,
    MTST_DTI_DN_INSTANCE,
    MTST_DTI_DN_INTERFACE,
    MTST_DTI_DN_CHANNEL,
    prim
  );
}

/***********+++++-----------------+++++++*******++----++**********/

/*
+------------------------------------------------------------------------------
|  Function     :  sig_dti_pei_data_received_ind
+------------------------------------------------------------------------------
|  Description  :  sent from UART to deliver new data
|
|  Parameters   :  ptr         - Pointer to the structure of the primitive
|
|  Return       :  
|                  
+------------------------------------------------------------------------------
*/
GLOBAL const void sig_dti_pei_data_received_ind (T_DTI2_DATA_IND *ptr)
{
  T_DTI2_DATA_IND *prim = ptr;

  /* prevent dtilib from automatically sending flow control primitives */
  dti_stop
  (
    mtst_hDTI,
    MTST_DTI_DN_INSTANCE,
    MTST_DTI_DN_INTERFACE,
    MTST_DTI_DN_CHANNEL
  );

  if ( mux_data.EnabledSignalType & DRV_SIGTYPE_READ )
  {
    m_signal.SignalType = DRV_SIGTYPE_READ;
    m_signal.DrvHandle = mux_data.Handle;

    mux_data.recv_data=(T_desc2 *)prim->desc_list2.first;

    (mux_data.Callback)( &m_signal );
  }
  PFREE(prim);
}

/***********+++++-----------------+++++++*******++----++**********/

/* 
 * dtilib wrapping and support functions 
 */

/*
+------------------------------------------------------------------------------
|    Function    : pei_dti_dti_connect_req
+------------------------------------------------------------------------------
|    PURPOSE : Call the process function dti_dti_connect_req
+------------------------------------------------------------------------------
*/

GLOBAL const void pei_dti_dti_connect_req (
                    T_DTI2_CONNECT_REQ   *dti_connect_req
                  )
{
  dti_dti_connect_req (mtst_hDTI, dti_connect_req);
}

/*
+------------------------------------------------------------------------------
|    Function    : pei_dti_dti_connect_cnf
+------------------------------------------------------------------------------
|    PURPOSE : Call the process function dti_dti_connect_cnf
+------------------------------------------------------------------------------
*/

GLOBAL const void pei_dti_dti_connect_cnf (
                    T_DTI2_CONNECT_CNF   *dti_connect_cnf
                  )
{
    dti_dti_connect_cnf(mtst_hDTI, dti_connect_cnf);  
}

/*
+------------------------------------------------------------------------------
|    Function    : pei_dti_dti_connect_ind
+------------------------------------------------------------------------------
|    PURPOSE : Call the process function dti_dti_connect_ind
+------------------------------------------------------------------------------
*/

GLOBAL const void pei_dti_dti_connect_ind (
                    T_DTI2_CONNECT_IND   *dti_connect_ind
                  )
{
    dti_dti_connect_ind(mtst_hDTI, dti_connect_ind);  
}

/*
+------------------------------------------------------------------------------
|    Function    : pei_dti_dti_connect_res
+------------------------------------------------------------------------------
|    PURPOSE : Call the process function dti_dti_connect_res
+------------------------------------------------------------------------------
*/

GLOBAL const void pei_dti_dti_connect_res (
                    T_DTI2_CONNECT_RES   *dti_connect_res
                  )
{
    dti_dti_connect_res(mtst_hDTI, dti_connect_res);  
}

/*
+------------------------------------------------------------------------------
|    Function    : pei_dti_dti_disconnect_req
+------------------------------------------------------------------------------
|    PURPOSE : Call the process function dti_dti_disconnect_req
+------------------------------------------------------------------------------
*/

GLOBAL const void pei_dti_dti_disconnect_req (
                    T_DTI2_DISCONNECT_REQ   *dti_disconnect_req
                  )
{
    dti_dti_disconnect_req (mtst_hDTI, dti_disconnect_req);
}

/*
+------------------------------------------------------------------------------
|    Function    : pei_dti_dti_disconnect_ind
+------------------------------------------------------------------------------
|    PURPOSE : Call the process function dti_dti_disconnect_ind
+------------------------------------------------------------------------------
*/

GLOBAL const void pei_dti_dti_disconnect_ind (
                    T_DTI2_DISCONNECT_IND   *dti_disconnect_ind
                  )
{
    dti_dti_disconnect_ind (mtst_hDTI, dti_disconnect_ind);
}

/*
+------------------------------------------------------------------------------
|    Function    : pei_dti_dti_data_req
+------------------------------------------------------------------------------
|    PURPOSE : Call the process function dti_dti_data_req
+------------------------------------------------------------------------------
*/

GLOBAL const void pei_dti_dti_data_req (
                    T_DTI2_DATA_REQ   *dti_data_req
                  )
{
    dti_dti_data_req (mtst_hDTI, dti_data_req);
}

/*
+------------------------------------------------------------------------------
|    Function    : pei_dti_dti_getdata_req
+------------------------------------------------------------------------------
|    PURPOSE : Call the process function dti_dti_data_req
+------------------------------------------------------------------------------
*/

GLOBAL const void pei_dti_dti_getdata_req (
                    T_DTI2_GETDATA_REQ   *dti_getdata_req
                  )
{
  dti_dti_getdata_req (mtst_hDTI, dti_getdata_req);
}
 
/*
+------------------------------------------------------------------------------
|    Function    : pei_dti_dti_data_ind
+------------------------------------------------------------------------------
|    PURPOSE : Call the process function dti_dti_data_ind
+------------------------------------------------------------------------------
*/

GLOBAL const void pei_dti_dti_data_ind (
                    T_DTI2_DATA_IND   *dti_data_ind
                  )
{
    dti_dti_data_ind (mtst_hDTI, dti_data_ind);
}

/*
+------------------------------------------------------------------------------
|    Function    : pei_dti_dti_ready_ind
+------------------------------------------------------------------------------
|    PURPOSE : Call the process function dti_dti_ready_ind
+------------------------------------------------------------------------------
*/

GLOBAL const void pei_dti_dti_ready_ind (
                    T_DTI2_READY_IND   *dti_ready_ind
                  )
{
  dti_dti_ready_ind (mtst_hDTI, dti_ready_ind);
}
 
#ifdef _SIMULATION_

/*
+------------------------------------------------------------------------------
|    Function    : pei_dti_dti_data_test_req
+------------------------------------------------------------------------------
|    PURPOSE : Call the process function dti_dti_data_test_req
+------------------------------------------------------------------------------
*/
    
GLOBAL const void pei_dti_dti_data_test_req (
                    T_DTI2_DATA_TEST_REQ   *dti_data_test_req
                  )
{
    dti_dti_data_test_req (mtst_hDTI, dti_data_test_req);
}

/*
+------------------------------------------------------------------------------
|    Function    : pei_dti_dti_data_test_ind
+------------------------------------------------------------------------------
|    PURPOSE : Call the process function dti_dti_data_test_ind
+------------------------------------------------------------------------------
*/

GLOBAL const void pei_dti_dti_data_test_ind (
                    T_DTI2_DATA_TEST_IND   *dti_data_test_ind
                  )
{
  dti_dti_data_test_ind (mtst_hDTI, dti_data_test_ind);   
}

#endif /* _SIMULATION_ */


/*
+------------------------------------------------------------------------------
|    Function: pei_sig_callback
+------------------------------------------------------------------------------
|    PURPOSE : Callback function for DTILIB
+------------------------------------------------------------------------------
*/

GLOBAL void pei_sig_callback(U8 instance, U8 interfac, U8 channel, 
                               U8 reason, T_DTI2_DATA_IND *dti_data_ind)
{
  TRACE_FUNCTION("pei_sig_callback");

#ifdef _SIMULATION_
  if(instance NEQ MTST_DTI_UP_INSTANCE || 
     interfac NEQ MTST_DTI_UP_INTERFACE || 
     channel  NEQ MTST_DTI_UP_CHANNEL) 
  {
    TRACE_ERROR("[PEI_SIG_CALLBACK] invalid parameters!");
    return; /* error, not found */
  }
#endif /* _SIMULATION_ */

  if (mtst_hDTI NEQ D_NO_DATA_BASE)
  {
    switch (reason)
    {
      case DTI_REASON_CONNECTION_OPENED:
        sig_dti_pei_connection_opened_ind();
        break;

      case DTI_REASON_CONNECTION_CLOSED:
        sig_dti_pei_connection_closed_ind();
        break;

      case DTI_REASON_DATA_RECEIVED:

        /*
         * DTI2_DATA_IND is interpreted as DTI2_DATA_REQ
         */
        PACCESS (dti_data_ind);
        sig_dti_pei_data_received_ind(dti_data_ind);
        break;

      case DTI_REASON_TX_BUFFER_FULL:
        sig_dti_pei_tx_buffer_full_ind();
        break;
      
      case DTI_REASON_TX_BUFFER_READY:
        sig_dti_pei_tx_buffer_ready_ind();
        break;

      default:
        TRACE_ERROR("unknown DTILIB reason parameter");
        break;
    } /* end switch */
  } /* end if */
  else 
  {
    TRACE_ERROR("Pointer to DTILIB database not existing");
  }
} /* pei_sig_callback() */

/*==== END OF FILE ==========================================================*/

