/* 
+----------------------------------------------------------------------------- 
|  Project :  GSM-PS (6147)
|  Modul   :  CMH_DMY
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
|  Purpose :  This module is used to implement all function which
|             are not yet supported by the command handler of the ACI.
+----------------------------------------------------------------------------- 
*/ 

#ifndef CMH_DMY_C
#define CMH_DMY_C
#endif

#include "config.h"
#include "fixedconf.h"
#include "condat-features.h"
#include "aci_conf.h"

#include "aci_all.h"
/*==== INCLUDES ===================================================*/
#include "aci_cmh.h"

#ifdef FAX_AND_DATA
#include "aci_fd.h"
#endif

#include "ati_cmd.h"
#include "aci_io.h"

/*==== CONSTANTS ==================================================*/

/*==== TYPES ======================================================*/

/*==== EXPORT =====================================================*/

/*==== VARIABLES ==================================================*/

/*==== FUNCTIONS ==================================================*/

#ifdef FF_FAX

GLOBAL T_ACI_RETURN sAT_PlusFND   (T_ACI_CMD_SRC        srcId,
                                   T_ACI_FND_VAL        value)
{
  return AT_FAIL;
}

GLOBAL T_ACI_RETURN qAT_PlusFND   (T_ACI_CMD_SRC        srcId,
                                   T_ACI_FND_VAL      * value)
{
  return AT_FAIL;
}

#endif /* FF_FAX */

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_DMY                  |
| STATE   : code                  ROUTINE : qAT_PlusCGI              |
+--------------------------------------------------------------------+

  PURPOSE :
*/

GLOBAL T_ACI_RETURN qAT_PlusGCI  (  T_ACI_CMD_SRC     srcId,
                                    UBYTE            *country)
{
  return AT_FAIL;
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_DMY                  |
| STATE   : code                  ROUTINE : sAT_PlusCGI              |
+--------------------------------------------------------------------+

  PURPOSE :
*/

GLOBAL T_ACI_RETURN sAT_PlusGCI  (  T_ACI_CMD_SRC     srcId,
                                    UBYTE             country)

{
  return AT_FAIL;
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_DMY                  |
| STATE   : code                  ROUTINE : sAT_PlusVTD              |
+--------------------------------------------------------------------+

  PURPOSE :
*/

GLOBAL T_ACI_RETURN sAT_PlusVTD  ( T_ACI_CMD_SRC     srcId,
                                   SHORT             duration)
{
  return AT_FAIL;
}
