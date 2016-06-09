/* 
+----------------------------------------------------------------------------- 
|  Project :  GSM-PS (6147)
|  Modul   :  MESSAGE
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
|  Purpose :  Message Definitions, depending on entity
+----------------------------------------------------------------------------- 
*/ 

#ifndef MESSAGE_H
#define MESSAGE_H

/*
 *  Definitions for Message Types
 */
#include "mconst.cdg"

/*
 *  Define the messages depending on the entity
 */

#ifdef ENTITY_RR

#define ADD_BSIZE 8 /* additional 8 bits (Skip Indicator/PD) */
#include "m_rr.h"
#if defined (REL99) && defined (TI_PS_FF_EMR)
#include "m_rr_short_pd.h"
#endif

#endif

#ifdef ENTITY_RRLP

#include "m_rrlp_asn1_msg.h"

#endif

#ifdef ENTITY_MM

#include "m_mm.h"

#endif

#ifdef ENTITY_CC

#include "m_cc.h"

#endif

#ifdef ENTITY_SS

#include "m_ss.h"

#endif

#ifdef ENTITY_SMS

#include "m_sms.h"

#endif

#ifdef ENTITY_T30

#include "m_t30.h"

#endif

#if defined (ENTITY_SMI) || defined (ENTITY_MFW) || defined (ENTITY_ACI) || defined (ENTITY_CST)

#include "m_fac.h"
#include "m_sat.h"
#include "m_cc.val"
#include "m_sms.h"

#endif



#ifdef ENTITY_SIM

#include "m_sat.h"

#endif /* ENTITY_SIM */


#ifdef ENTITY_GRR

#include "m_grr.h"
#include "m_rr.h"

#endif


#ifdef ENTITY_GRLC

#include "m_grlc.h"
#include "m_rr.h"

#endif

#ifdef ENTITY_GMM

#include "m_gmm.h"
#include "m_tst.h"

#endif

#ifdef ENTITY_SM

#include "m_sm.h"

#endif



#endif

