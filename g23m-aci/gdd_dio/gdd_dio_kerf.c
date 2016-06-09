/*
+-----------------------------------------------------------------------------
|  File     : gdd_dio_kerf.c
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
|  Purpose  : This modul is part of the entity gdd_dio and implements the
|             kernel service functions.
+-----------------------------------------------------------------------------
*/


#define ENTITY_GDD_DIO

/*==== INCLUDES =============================================================*/

#include "typedefs.h"   /* to get Condat data types */
#include "vsi.h"        /* to get a lot of macros */
#include "prim.h"       /* to get the definitions of used SAP and directions */
#include "dti.h"
#include "gdd_dio.h"       /* to get the global entity definitions */


/*==== CONST ================================================================*/

/*==== LOCAL VARS ===========================================================*/

/*==== PRIVATE FUNCTIONS ====================================================*/

/*==== PUBLIC FUNCTIONS =====================================================*/


/*
+------------------------------------------------------------------------------
| Function    : ker_init
+------------------------------------------------------------------------------
| Description : The function ker_init() initializes the state of the 
|               kernel service. The state of DIO is initialized independently
|               when dio_init_bat() is called.
+------------------------------------------------------------------------------
*/
GLOBAL int gdd_dio_ker_init ()
{
  TRACE_FUNCTION( "ker_init" );

  /* Nothing to do currently. The state of each kernel
     (gdd_dio_data_base[<instance>].ker.state is static and automatically
     set to GDD_DIO_KER_DEAD on start-up. */

  return 1;
} /* ker_init() */
