/*
+-----------------------------------------------------------------------------
|  Project :  DTILIB
|  Modul   :  DTI
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
|  Purpose :  Definitions for the Protocol Stack Library
|             DTI
+-----------------------------------------------------------------------------
*/

/*
*  Version 1.6
*/

#ifndef DTI_KERP_C
#define DTI_KERP_C
#endif

#include "config.h"
#include "fixedconf.h"
#include "condat-features.h"

#include <string.h>
#include "typedefs.h"
#include "pconst.cdg"
#include "vsi.h"
#include "pconst.cdg"
#include "custom.h"
#include "gsm.h"
#include "prim.h"
#include "pei.h"
#include "tok.h"
#include "ccdapi.h"
#include "dti.h"
#include "dti_int_def.h"
#include "dti_int_prot.h"


/*==== CONST =======================================================*/

/*==== TYPES =======================================================*/

/*==== VAR EXPORT ==================================================*/

/*==== VAR LOCAL ===================================================*/


/*==== MACROS ======================================================*/


/*==== prototypes ==================================================*/

/*==== FUNCTIONS ===================================================*/

/*
+--------------------------------------------------------------------+
| PROJECT : DTILIB                    MODULE  : DTI_KERP             |
| STATE   : code                      ROUTINE : dti_dti_connect_req  |
+--------------------------------------------------------------------+

PURPOSE : Process primitive DTI_CONNECT_REQ received from neighbour DTI.
*/

GLOBAL void dti_dti_connect_req (DTI_HANDLE hDTI,
                   T_DTI2_CONNECT_REQ *dti_connect_req)
{
#define VSI_CALLER hDTI->handle,

  trace_function(hDTI->handle,
                 "dti_dti_connect_req()",
                 hDTI->entity_options);

  if( !(hDTI->entity_options & DTI_NO_TRACE) )
  {
    PACCESS (dti_connect_req);
  }

  trace_message_link_id(hDTI->handle,
                        " IN: DTI2_CONNECT_REQ",
                        dti_connect_req->link_id,
                        hDTI->entity_options);

  connect_init_prim_received(hDTI,
                             dti_connect_req->link_id,
                             dti_connect_req->version,
                             DTI_CHANNEL_TO_HIGHER_LAYER);

  PFREE (dti_connect_req);

#undef VSI_CALLER
} /* dti_dti_connect_req() */



/*
+--------------------------------------------------------------------+
| PROJECT : DTILIB                    MODULE  : DTI_KERP             |
| STATE   : code                      ROUTINE : dti_dti_connect_ind  |
+--------------------------------------------------------------------+

PURPOSE : Process primitive DTI_CONNECT_IND received from neighbour DTI
*/

GLOBAL void dti_dti_connect_ind (DTI_HANDLE hDTI,
                   T_DTI2_CONNECT_IND *dti_connect_ind)
{
#define VSI_CALLER hDTI->handle,

  trace_function(hDTI->handle,
                 "dti_dti_connect_ind()",
                 hDTI->entity_options);

  if( !(hDTI->entity_options & DTI_NO_TRACE) )
  {
    PACCESS (dti_connect_ind);
  }

  trace_message_link_id(hDTI->handle,
                        " IN: DTI2_CONNECT_IND",
                        dti_connect_ind->link_id,
                        hDTI->entity_options);

  connect_init_prim_received(hDTI,
                             dti_connect_ind->link_id,
                             dti_connect_ind->version,
                             DTI_CHANNEL_TO_LOWER_LAYER);

  PFREE(dti_connect_ind);

#undef VSI_CALLER
} /* dti_dti_connect_ind() */



/*
+--------------------------------------------------------------------+
| PROJECT : DTILIB                     MODULE  : DTI_KERP            |
| STATE   : code                    ROUTINE : dti_dti_disconnect_req |
+--------------------------------------------------------------------+

PURPOSE : Process primitive DTI2_DISCONNECT_REQ received from neighbour DTI
*/

GLOBAL void dti_dti_disconnect_req (DTI_HANDLE hDTI,
                   T_DTI2_DISCONNECT_REQ *dti_disconnect_req)
{
#define VSI_CALLER hDTI->handle,

  trace_function(hDTI->handle,
                 "dti_dti_disconnect_req()",
                 hDTI->entity_options);

  if( !(hDTI->entity_options & DTI_NO_TRACE) )
  {
    PACCESS (dti_disconnect_req);
  }

  trace_message_link_id(hDTI->handle,
                        " IN: DTI2_DISCONNECT_REQ",
                        dti_disconnect_req->link_id,
                        hDTI->entity_options);

  disconnect_prim_received(hDTI,
                           dti_disconnect_req->link_id,
                           DTI_CHANNEL_TO_HIGHER_LAYER);

  PFREE (dti_disconnect_req);

#undef VSI_CALLER
} /* dti_dti_disconnect_req() */



/*
+--------------------------------------------------------------------+
| PROJECT : DTILIB                     MODULE  : DTI_KERP            |
| STATE   : code                    ROUTINE : dti_dti_disconnect_ind |
+--------------------------------------------------------------------+

PURPOSE : Process primitive DTI_DISCONNECT_IND received from neighbour DTI
*/

GLOBAL void dti_dti_disconnect_ind (DTI_HANDLE hDTI,
                   T_DTI2_DISCONNECT_IND *dti_disconnect_ind)
{
#define VSI_CALLER hDTI->handle,

  trace_function(hDTI->handle,
                 "dti_dti_disconnect_ind()",
                 hDTI->entity_options);

  if( !(hDTI->entity_options & DTI_NO_TRACE) )
  {
    PACCESS (dti_disconnect_ind);
  }

  trace_message_link_id(hDTI->handle,
                        " IN: DTI2_DISCONNECT_IND",
                        dti_disconnect_ind->link_id,
                        hDTI->entity_options);

  disconnect_prim_received(hDTI,
                           dti_disconnect_ind->link_id,
                           DTI_CHANNEL_TO_LOWER_LAYER);

  PFREE (dti_disconnect_ind);

#undef VSI_CALLER
} /* dti_dti_disconnect_ind() */



/*
+--------------------------------------------------------------------+
| PROJECT : DTILIB                    MODULE  : DTI_KERP             |
| STATE   : code                      ROUTINE : dti_dti_connect_cnf  |
+--------------------------------------------------------------------+

PURPOSE : Process primitive DTI_CONNECT_CNF received from neighbour DTI
*/

GLOBAL void dti_dti_connect_cnf (DTI_HANDLE hDTI,
                   T_DTI2_CONNECT_CNF *dti_connect_cnf)
{
#define VSI_CALLER hDTI->handle,

  trace_function(hDTI->handle,
                 "dti_dti_connect_cnf()",
                 hDTI->entity_options);

  if( !(hDTI->entity_options & DTI_NO_TRACE) )
  {
    PACCESS (dti_connect_cnf);
  }

  trace_message_link_id(hDTI->handle,
                        " IN: DTI2_CONNECT_CNF",
                        dti_connect_cnf->link_id,
                        hDTI->entity_options);

  connect_confirm_prim_received(hDTI,
                                dti_connect_cnf->link_id,
                                dti_connect_cnf->version,
                                DTI_CHANNEL_TO_LOWER_LAYER);

  PFREE (dti_connect_cnf);

#undef VSI_CALLER
} /* dti_dti_connect_cnf() */



/*
+--------------------------------------------------------------------+
| PROJECT : DTILIB                    MODULE  : DTI_KERP             |
| STATE   : code                      ROUTINE : dti_dti_connect_res  |
+--------------------------------------------------------------------+

  PURPOSE : Process primitive DTI_CONNECT_RES received from neighbour DTI
  */

GLOBAL void dti_dti_connect_res (DTI_HANDLE hDTI,
                   T_DTI2_CONNECT_RES *dti_connect_res)
{
#define VSI_CALLER hDTI->handle,

  trace_function(hDTI->handle,
                 "dti_dti_connect_res()",
                 hDTI->entity_options);

  if( !(hDTI->entity_options & DTI_NO_TRACE) )
  {
    PACCESS (dti_connect_res);
  }

  trace_message_link_id(hDTI->handle,
                        " IN: DTI2_CONNECT_RES",
                        dti_connect_res->link_id,
                        hDTI->entity_options);

  connect_confirm_prim_received(hDTI,
                                dti_connect_res->link_id,
                                dti_connect_res->version,
                                DTI_CHANNEL_TO_HIGHER_LAYER);

  PFREE (dti_connect_res);

#undef VSI_CALLER
} /* dti_dti_connect_res() */



/*
+--------------------------------------------------------------------+
| PROJECT : DTILIB                    MODULE  : DTI_KERP             |
| STATE   : code                      ROUTINE : dti_dti_data_req     |
+--------------------------------------------------------------------+

  PURPOSE : Process primitive DTI_DATA_REQ received from neighbour DTI
  */

GLOBAL void dti_dti_data_req (DTI_HANDLE hDTI,
                   T_DTI2_DATA_REQ *dti_data_req)
{
#define VSI_CALLER hDTI->handle,

  trace_function(hDTI->handle,
                 "dti_dti_data_req()",
                 hDTI->entity_options);

  if( !(hDTI->entity_options & DTI_NO_TRACE) )
  {
    PACCESS (dti_data_req);
  }

  trace_message_l_dl(hDTI->handle,
                     " IN: DTI2_DATA_REQ",
                     dti_data_req->link_id,
                     dti_data_req->desc_list2.list_len,
                     hDTI->entity_options);

  if(hDTI->entity_options & DTI_NO_TRACE)
  {
    PPASS_NTRACE(dti_data_req, dti_data_ind, DTI2_DATA_IND);
    data_prim_received (hDTI, dti_data_ind, DTI_CHANNEL_TO_HIGHER_LAYER);
  }
  else
  {
    PPASS(dti_data_req, dti_data_ind, DTI2_DATA_IND);
    data_prim_received (hDTI, dti_data_ind, DTI_CHANNEL_TO_HIGHER_LAYER);
  }

#undef VSI_CALLER
} /* dti_dti_data_req() */



/*
+--------------------------------------------------------------------+
| PROJECT : DTILIB                    MODULE  : DTI_KERP             |
| STATE   : code                      ROUTINE : dti_dti_data_ind     |
+--------------------------------------------------------------------+

PURPOSE : Process primitive DTI_DATA_IND received from neighbour DTI
*/

GLOBAL void dti_dti_data_ind (DTI_HANDLE hDTI,
                   T_DTI2_DATA_IND *dti_data_ind)
{
#define VSI_CALLER hDTI->handle,

  trace_function(hDTI->handle,
                 "dti_dti_data_ind()",
                 hDTI->entity_options);

  if( !(hDTI->entity_options & DTI_NO_TRACE) )
  {
    PACCESS (dti_data_ind);
  }

  trace_message_l_dl(hDTI->handle,
                     " IN: DTI2_DATA_IND",
                     dti_data_ind->link_id,
                     dti_data_ind->desc_list2.list_len,
                     hDTI->entity_options);

  data_prim_received (hDTI, dti_data_ind, DTI_CHANNEL_TO_LOWER_LAYER);

#undef VSI_CALLER
} /* dti_dti_data_ind() */



/*
+--------------------------------------------------------------------+
| PROJECT : DTILIB                    MODULE  : DTI_KERP             |
| STATE   : code                      ROUTINE : dti_dti_getdata_req  |
+--------------------------------------------------------------------+

PURPOSE : Process primitive DTI_GETDATA_REQ received from neighbour DTI
*/

GLOBAL void dti_dti_getdata_req (DTI_HANDLE hDTI,
                   T_DTI2_GETDATA_REQ *dti_getdata_req)
{
#define VSI_CALLER hDTI->handle,

  trace_function(hDTI->handle,
                 "dti_dti_getdata_req()",
                 hDTI->entity_options);

  if( !(hDTI->entity_options & DTI_NO_TRACE) )
  {
    PACCESS (dti_getdata_req);
  }

  trace_message_link_id(hDTI->handle,
                        " IN: DTI2_GETDATA_REQ",
                        dti_getdata_req->link_id,
                        hDTI->entity_options);

  flow_control_prim_received(hDTI,
                             dti_getdata_req->link_id,
                             DTI_CHANNEL_TO_HIGHER_LAYER);

  PFREE (dti_getdata_req);

#undef VSI_CALLER
} /* dti_dti_getdata_req() */



/*
+--------------------------------------------------------------------+
| PROJECT : DTILIB                    MODULE  : DTI_KERP             |
| STATE   : code                      ROUTINE : dti_dti_ready_ind    |
+--------------------------------------------------------------------+

PURPOSE : Process primitive DTI_READY_IND received from neighbour DTI
*/

GLOBAL void dti_dti_ready_ind (DTI_HANDLE hDTI,
                   T_DTI2_READY_IND *dti_ready_ind)
{
#define VSI_CALLER hDTI->handle,

  trace_function(hDTI->handle,
                 "dti_dti_ready_ind()",
                 hDTI->entity_options);

  if( !(hDTI->entity_options & DTI_NO_TRACE) )
  {
    PACCESS (dti_ready_ind);
  }

  trace_message_link_id(hDTI->handle,
                        " IN: DTI2_READY_IND",
                        dti_ready_ind->link_id,
                        hDTI->entity_options);

  flow_control_prim_received(hDTI,
                             dti_ready_ind->link_id,
                             DTI_CHANNEL_TO_LOWER_LAYER);

  PFREE (dti_ready_ind);

#undef VSI_CALLER
} /* dti_dti_ready_ind() */


#ifdef _SIMULATION_

/*
+--------------------------------------------------------------------+
| PROJECT : WAP                       MODULE  : ip_kerp.c            |
| STATE   : code                      ROUTINE : ker_dti_data_test_ind|
+--------------------------------------------------------------------+

  PURPOSE : Process primitive T_DTI2_DATA_TEST_IND
  */

GLOBAL void dti_dti_data_test_ind (DTI_HANDLE hDTI,
                   T_DTI2_DATA_TEST_IND *dti_data_test_ind)
{
#define VSI_CALLER hDTI->handle,

  trace_function(hDTI->handle,
                 "dti_dti_data_test_ind()",
                 hDTI->entity_options);

  if( !(hDTI->entity_options & DTI_NO_TRACE) )
  {
    PACCESS (dti_data_test_ind);
  }

  trace_message_l_dl(hDTI->handle,
                     " IN: DTI2_DATA_TEST_IND",
                     dti_data_test_ind->link_id,
                     (U16)(dti_data_test_ind->sdu.l_buf>>3),
                     hDTI->entity_options);

  data_test_prim_received(hDTI,
                          dti_data_test_ind,
                          DTI_CHANNEL_TO_LOWER_LAYER);

#undef VSI_CALLER
} /* dti_dti_data_test_ind() */



/*
+--------------------------------------------------------------------+
| PROJECT : WAP                       MODULE  : ip_kerp.c            |
| STATE   : code                      ROUTINE : ker_dti_data_test_req|
+--------------------------------------------------------------------+

  PURPOSE : Process primitive T_DTI2_DATA_TEST_REQ
*/

GLOBAL void dti_dti_data_test_req (DTI_HANDLE hDTI,
                   T_DTI2_DATA_TEST_REQ *dti_data_test_req)
{
#define VSI_CALLER hDTI->handle,

  trace_function(hDTI->handle,
                 "dti_dti_data_test_req()",
                 hDTI->entity_options);

  trace_message_l_dl(hDTI->handle,
                     " IN: DTI2_DATA_TEST_IND",
                     dti_data_test_req->link_id,
                     (U16)(dti_data_test_req->sdu.l_buf>>3),
                     hDTI->entity_options);

  if(hDTI->entity_options & DTI_NO_TRACE)
  {
    PPASS_NTRACE(dti_data_test_req, dti_data_test_ind, DTI2_DATA_TEST_IND);
    data_test_prim_received(hDTI,
                            dti_data_test_ind,
                            DTI_CHANNEL_TO_HIGHER_LAYER);
  }
  else
  {
    PACCESS (dti_data_test_req);
    {
      PPASS(dti_data_test_req, dti_data_test_ind, DTI2_DATA_TEST_IND);
      data_test_prim_received(hDTI,
                              dti_data_test_ind,
                              DTI_CHANNEL_TO_HIGHER_LAYER);
    }
  }

#undef VSI_CALLER
} /* dti_dti_data_test_req() */

#endif /*_SIMULATION_ */
