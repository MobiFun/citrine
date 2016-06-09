/*
+-----------------------------------------------------------------------------
|  Project :  GSM-PS (6147)
|  Modul   :  CUSTOM
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
|  Purpose :  Custom dependent definitions
|
|             Use this header for definitions to integrate the
|             protocol stack in your target system !
+-----------------------------------------------------------------------------
*/

#ifndef CUSTOM_H
#define CUSTOM_H

/*
 * OPTION_MULTITHREAD
 *
 * Description :  For Operationg systems where the entire protocol
 *                stack is linked as an process and the entitys are
 *                started as threads, this option must be set. In
 *                this case all the pei_ functions of one entity
 *                were prefixed by the entity name like cc_pei...
 *
 * Options:       #define OPTION_MULTITHREAD
 *                                       Multithread application
 *                #undef  OPTION_MULTITHREAD
 *                                       No multithread application.
 *                                       Each entity is linked seperatly.
 *
 */
/*
#undef OPTION_MULTITHREAD must be passed as compile switch
*/
/*==== ENTITY DEPENDENT CONFIGURATION PARAMETER ===================*/

#ifndef __PFRAME_C__ /* do not include the entity specific custom
                        header if we compile the frame */

#ifdef ENTITY_CST
#include "../../g23m-glue/cst/cus_cst.h"
#endif

#if defined (ENTITY_SMI) || defined (ENTITY_MFW) || defined (ENTITY_ACI) || defined (ENTITY_MMI)
#include "cus_aci.h"
#endif

#ifdef ENTITY_PAN
#endif

#ifdef ENTITY_DL
#include "cus_dl.h"
#endif

#ifdef ENTITY_RR
#include "cus_rr.h"
#endif

#ifdef ENTITY_MM
#include "cus_mm.h"
#endif

#ifdef ENTITY_CC
#include "cus_cc.h"
#endif

#ifdef ENTITY_SS
#include "cus_ss.h"
#endif

#ifdef ENTITY_ESIM
#include "cus_esim.h"
#endif

#ifdef ENTITY_SMS
#include "cus_sms.h"
#endif

#ifdef ENTITY_PL
  #ifdef ALR
    #include "cus_alr.h"
  #else
    #include "cus_til.h"
  #endif
#endif

#ifdef ENTITY_L1
#include "cus_l1.h"
#endif

#ifdef ENTITY_SIM
#include "cus_sim.h"
#endif

#ifdef ENTITY_L2R
#include "cus_l2r.h"
#endif

#ifdef ENTITY_RLP
#include "cus_rlp.h"
#endif

#ifdef ENTITY_T30
#include "cus_t30.h"
#endif

#ifdef ENTITY_FAD
#include "cus_fad.h"
#endif

#ifdef ENTITY_RA
#include "cus_ra.h"
#endif

#ifdef ENTITY_WAP
#ifdef FF_GPF_TCPIP
#include "cus_wapmic.h"
#endif
#ifdef CO_UDP_IP
#include "cus_wap.h"
#endif
#endif

#ifdef ENTITY_UDP
#include "cus_udp.h"
#endif

#ifdef ENTITY_IP
#include "cus_ip.h"
#endif

#ifdef ENTITY_TCP
#include "cus_tcp.h"
#endif

#endif

/*==== ENTITY NAMES ===============================================*/
/*
 *  The names are used as identifier for the communication resource
 */

#define NULL_NAME     "NULL"
#define L1_NAME       "L1"
#define PL_NAME       "PL"
#define DL_NAME       "DL"
#define RR_NAME       "RR"
#define MM_NAME       "MM"
#define CC_NAME       "CC"
#define SS_NAME       "SS"
#define ESIM_NAME       "ESIM"
#define SMS_NAME      "SMS"
#define SIM_NAME      "SIM"
#define PAN_NAME      "PAN"
#define DMYA_NAME     "DMYA"
#define DMYB_NAME     "DMYB"
#define CST_NAME      "CST"
#define GRR_NAME      "GRR"

#define ACI_NAME      "MMI"
#define AAA_NAME      "AAA"
#define RIV_NAME      "RIV"

#define L2R_NAME      "L2R"
#define TRA_NAME      L2R_NAME  /* TRA running in L2R task */
#define RLP_NAME      "RLP"
#define T30_NAME      "T30"
#define FAD_NAME      "FAD"
#define RA_NAME       "RA"
#define WAP_NAME      "WAP"
#define UDP_NAME      "UDP"
#define IP_NAME       "IP"
#define PPP_NAME      "PPP"
#define UART_NAME     "UART"
#define PSI_NAME      "PSI"
#define DTI_NAME      "DTI"
#define TAP_NAME      "TAP"
#define BTI_NAME      "BTI"   /* BTI is not an entity, but the name is used to identify BTI */
#define LC_NAME       "LC"
#define RRLP_NAME     "RRLP"


#ifdef FF_TRACE_OVER_MTST
#define MTST_NAME     "MTST"
#endif /* FF_TRACE_OVER_MTST */

#ifdef GPRS
#define GMM_NAME      "GMM"
#define GRR_NAME      "GRR"
#define GRLC_NAME     "GRLC"
#define PKTIO_NAME    "PKT"
#define GPL_NAME      "GRR"
#endif /* #ifdef GPRS */

#define DCM_NAME      "MMI"
#define TCPIP_NAME    "TCP"

#define APP_NAME      "APP"
#define GDD_DIO_NAME  "GDDI"

#ifndef NEW_FRAME
/*==== STATIC CONFIGURATION =======================================*/
/*
 * TRACE_FKT
 *
 * Description :  A trace string is send to the environment when a
 *                function is called.
 *
 * Options:       #define TRACE_FKT     configuration is active
 *                #undef  TRACE_FKT     configuration is not active
 *                #define TC_FUNC  <n>  used trace class
 *
 * Function Traces only under Windows and for MMI
 */

/*
 * It is nearly impossible to debug ACI/MFW/SMI without function traces
 */
#if defined(WIN32) OR defined(ENTITY_MFW) OR defined(ENTITY_ACI) OR defined(ENTITY_SMI)
#define  TRACE_FKT
#define TC_FUNC             1
#else
#undef TRACE_FKT
#endif

/*
 * TRACE_EVE
 *
 * Description :  A trace string is given to the environment when
 *                an event has happened, for example start of cell
 *                selection.
 *
 * Options:       #define TRACE_EVE     configuration is active
 *                #undef  TRACE_EVE     configuration is not active
 *                #define TC_EVENT  <n> used trace class
 */

#if !defined (NTRACE)
#define TRACE_EVE
#define TC_EVENT           2
#else
#undef TRACE_EVE
#endif

/*
 * TRACE_ERR
 *
 * valid for   :  Mobile and Base Station
 *
 * Description :  A trace string is given to the environment when
 *                an error has occured, for example parameter error
 *                by dynamic configuration.
 *
 * Options:       #define TRACE_ERR     configuration is active
 *                #undef  TRACE_ERR     configuration is not active
 *                #define TC_ERROR  <n> used trace class
 */

#if !defined (NTRACE)
#define TRACE_ERR
#define TC_ERROR           3
#else
#undef TRACE_ERR
#endif

/*
 * TRACE_PRIM
 *
 * Description :  A trace string is send to the environment when a
 *                primitive is received or send.
 *
 * Options:       #define TRACE_PRIM     configuration is active
 *                #undef  TRACE_PRIM     configuration is not active
 *                #define TC_PRIM   <n>  used trace class
 */

#if !defined (NTRACE)
#define TRACE_PRIM
#define TC_PRIM            4
#else
#undef TRACE_PRIM
#endif

/*
 * TRACE_STATE
 *
 * Description :  A trace string is send to the environment when a
 *                state variable is changed or retrieved.
 *
 * Options:       #define TRACE_STATE     configuration is active
 *                #undef  TRACE_STATE     configuration is not active
 *                #define TC_STATE   <n>  used trace class
 */

#if !defined (NTRACE)
#define TRACE_STATE
#define TC_STATE           8
#else
#undef TRACE_STATE
#endif

#endif /* NEW_FRAME */

/*
 * SIM Application Toolkit
 *
 * Description :  Depending of the general activation of SIM application
 *                Toolkit, specific parts can be activated
 *
 * Options:       #define SAT_CBM_DNL_SUPPORT   Cell Broadcast Data
 *                                              Download shall be supported
 *                #define SAT_SMS_DNL_SUPPORT   SMS Data Download shall
 *                                              be supported
 *                #define SAT_CALL_CTRL_SUPPORT Call Control by SIM shall
 *                                              be supported
 *                #define SAT_CALL_REQ_SUPPORT  Setup Call and Send SS/USSD
 *                                              shall be supported
 */

#ifdef SIM_TOOLKIT
#define SAT_CBM_DNL_SUPPORT
#define SAT_SMS_DNL_SUPPORT
#define SAT_CALL_CTRL_SUPPORT
#define SAT_CALL_REQ_SUPPORT
#endif

/*
 * Engineering Mode
 *
 * Description :  The configuration enables feature flag for engineering mode under WIN32.
 *
 * Options:       #define FF_EM_MODE       engineering mode is supported
 *
 */

#if defined (WIN32)
#define FF_EM_MODE
#endif

/*
 * OPTION_REF
 *
 * Description :  There are two ways defined to communicate
 *                between protocol stack entities. Either by
 *                copying buffers or by exchanging buffer
 *                addresses. This options defines which
 *                way is used.
 *
 * Options:       #define OPTION_REF    communication is
 *                                      carried out by
 *                                      exchanging buffer addresses
 *                #undef  OPTION_REF    communication is
 *                                      carried out by
 *                                      copying buffers
 */

#define OPTION_REF

/*
 * OPTION_LENGTH
 *
 * Description :  If the communication is carried out by
 *                exchanging buffer addresses it is not
 *                necessary to calculate the length of
 *                the buffer. This option suppresses
 *                the calculation of the size parameter
 *                of the sending communication buffer.
 *
 * Options:       #undef  OPTION_LENGTH  size parameter is set
 *                                       to zero.
 *                #define OPTION_LENGTH  size parameter is set
 *                                       to buffer size
 */

#define OPTION_LENGTH

/*
 * OPTION_SET_CONFIG_ONLY
 *
 * Description :  The pei_config () function is used
 *                to set and/or to read dynamic configuration.
 *                The possibility to read a dynamic configuration
 *                is switched off by this configuration.
 *
 * Options:       #define OPTION_SET_CONFIG_ONLY
 *                                       It is not possible to read
 *                                       a dynamic configuration
 *                #undef  OPTION_SET_CONFIG_ONLY
 *                                       It is possible to read
 *                                       a dynamic configuration
 */

#define OPTION_SET_CONFIG_ONLY

/*
 * OPTION_GSM_ONLY
 *
 * Description :  If the environment ensures that only GSM
 *                primitives are forwarded to the protocol
 *                stack entity this option suppresses the
 *                use of the vsi_c_primitive() function.
 *
 * Options:       #define OPTION_GSM_ONLY
 *                                       The environment sends
 *                                       only GSM primitives
 *                #undef  OPTION_GSM_ONLY
 *                                       The environment sends
 *                                       GSM and SYSTEM primitives
 */

#undef OPTION_GSM_ONLY

/*
 * OPTION_TIMEOUT_SYNC
 *
 * Description :  If the environment ensures that the
 *                pei_primitive and pei_timeout function
 *                not called at the same time this option
 *                suppresses the use of the vsi_c_awake()
 *                function. The timeout-handling is started
 *                directly by the pei_timeout() function.
 *
 * Options:       #define OPTION_TIMOUT_SYNC
 *                                       direct timeout-handling
 *                #undef  OPTION_TIMEOUT_SYNC
 *                                       indirect timeout-handling
 *                                       by using vsi_c_awake ()
 */

#undef OPTION_TIMEOUT_SYNC

/*
 * OPTION_SIGNAL
 *
 * Description :  The options defines whether signal processing
 *                in the entity is possible or not.
 *
 * Options:       #define OPTION_SIGNAL
 *                                       Signal processing is possible
 *                #undef  OPTION_SIGNAL
 *                                       Signal processing is not possible
 *
 */

#define OPTION_SIGNAL

/*
 * OPTION_TIMER
 *
 * Description :  The options defines whether timer values are
 *                changeable by dynamic configuration or not.
 *
 * Options:       #define OPTION_TIMER
 *                                       Timer values are changeable
 *                #undef  OPTION_TIMER
 *                                       Timer values are not
 *                                       changeable
 */

#if !defined (NCONFIG)
#define OPTION_TIMER
#else
#undef OPTION_TIMER
#endif

/*
 * OPTION_RELATIVE
 *
 * Description :  If the compiler for the target system
 *                uses relative addressing it is possible
 *                that the offset at run-time must be
 *                added to some jump tables containing only
 *                the offset at compile-time.
 *
 * Options:       #define OPTION_RELATIVE
 *                                       add run-time offset
 *                #undef  OPTION_RELATIVE
 *                                       don't add run-time offset
 */

#undef OPTION_RELATIVE

/*
 * OPTION_MULTI_INSTANCE
 * MAX_INSTANCES
 *
 *
 * Description :  The option is used if multiple instances are
 *                used. The routing information is stored in the
 *                header (T_ROUTE route). The routing information
 *                consists of instance number (inst_no),
 *                channel number (chan_no) and timeslot number
 *                (ts_no). The upper layer uses instance number
 *                for routing, at the interface to physical layer
 *                the channel and timeslot number are used. The
 *                constant MAX_INSTANCES defines the maximum of
 *                instances.
 *
 * Options:       #define OPTION_MULTI_INSTANCE
 *                                       use multi instances
 *                #undef  OPTION_MULTI_INSTANCE
 *                                       only one instance
 */

#undef OPTION_MULTI_INSTANCE
#define MAX_INSTANCES      1

/*==== TYPES ======================================================*/
/*
 * T_PRIM_HEADER
 *
 * Description :  This type definition defines the custom specific
 *                part of a primitive. All primitives have the
 *                general format: header followed by data. The
 *                header of a primitive is changeable according to
 *                the requirements of the target system.
 * Hints:         Only the operation code opc as a USHORT value must
 *                be present. For multi-instance protocol stacks
 *                the routing information must be include in the
 *                header (T_ROUTE route).
 */

#ifdef OPTION_MULTI_INSTANCES

typedef struct
{
  USHORT  inst_no;
  USHORT  chan_no;
  UBYTE   ts_no;
} T_ROUTE;

#endif

#if !defined (T_PRIM_HEADER_DEFINED)

#define T_PRIM_HEADER_DEFINED

#if 1 //#ifdef _TMS470

typedef struct
{
   USHORT opc;         /* equal to int SignalCode */
   USHORT opc2;
   USHORT len;
   USHORT idx;
   T_sdu *sdu;
   UBYTE * Sender;     /* not used                */
   UBYTE * SigP;       /* Pointer to data area    */
} T_PRIM_HEADER;

#else

typedef struct
{
   UBYTE  ps;
   UBYTE  fill;
   UBYTE  snd;
   UBYTE  rcv;
   ULONG  timestamp;
   USHORT len;
   USHORT idx;
   T_sdu *sdu;
   USHORT lng;
#ifdef OPTION_MULTI_INSTANCES
   T_ROUTE route;
#endif
   USHORT opc;
   USHORT opc2;
} T_PRIM_HEADER;

#endif

#endif /* T_PRIM_HEADER_DEFINED */

#endif /* CUSTOM_H */
