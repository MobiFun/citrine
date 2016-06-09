/*
+-----------------------------------------------------------------------------
|  Project :
|  Modul   :
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
|  Purpose :  This module implements the process body interface
|             for the entity ALR.
+-----------------------------------------------------------------------------
*/

#ifndef ALR_PEI_C
#define ALR_PEI_C

#include "config.h"
#include "fixedconf.h"
#include "condat-features.h"

#define ENTITY_PL

/*==== INCLUDES ===================================================*/
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include "typedefs.h"
#include "pconst.cdg"
#include "mconst.cdg"
#include "message.h"
#include "ccdapi.h"
#include "vsi.h"
#include "custom.h"
#include "gsm.h"
#include "prim.h"
#include "cnf_alr.h"
#include "mon_alr.h"
#include "pei.h"
#include "tok.h"
#include "pcm.h"
#ifdef GPRS
#include "alr_gprs.h"
#endif
#if defined (_TMS470)
#include "inth/iq.h"
#endif  /* _TMS470 */

#include "alr.h"
#include "alr_em.h"

/*==== IMPORT =====================================================*/
/*==== EXPORT =====================================================*/
#ifdef TI_PS_HCOMM_CHANGE
#else
GLOBAL T_HANDLE  hCommL1  = VSI_ERROR; /* PL   Communication  */
GLOBAL T_HANDLE  hCommPL  = VSI_ERROR; /* PL   Communication  */
GLOBAL T_HANDLE  hCommDL  = VSI_ERROR; /* DL   Communication  */
GLOBAL T_HANDLE  hCommRR  = VSI_ERROR; /* RR   Communication  */
GLOBAL T_HANDLE  hCommMMI = VSI_ERROR; /* MMI  Communication  */
GLOBAL T_HANDLE  hCommCST = VSI_ERROR; /* CST  Communication  */
#endif


#ifdef GPRS
GLOBAL T_HANDLE  hCommGPL = VSI_ERROR; /* GPL  Communication  */
#endif /* GPRS */

GLOBAL T_HANDLE  pl_handle;

GLOBAL  T_ALR_DATA             alr_data_base;

#if !defined(DYNAMIC_POWER_MEAS)
T_POWER_MEAS1  alr_power_meas_result1;
T_POWER_MEAS2  alr_power_meas_result2;
#endif  /* !DYNAMIC_POWER_MEAS */

#ifdef _SIMULATION_
/*lint -e785 (Info -- Too few initializers for aggregate)*/
T_POWER_MEAS  tap_rxlev_response_european[16] = {
   {4,{{1  , 105},{ 14, 220},{ 23, 280},{124, 125}}},  // 0
   {0,{{0  ,  0}, {  0,   0},{  0,   0},{  0,   0}}},  // 1
   {4,{{512, 105},{580, 220},{637, 280},{885, 125}}},  // 2
   {8,{{23 , 280},{512,  55},{14 , 220},{580, 215},    // 3
       {124, 125},{885, 115},{1  , 105},{637, 275}}},  
   {3,{{23 , 280},{512,  55},{0x8000 | 512, 100}}},    // 4
   {2,{{ 14, 220},{ 23, 280},{  0,   0},{  0,   0}}},  // 5
   {1,{{ 23, 280},{  0,   0},{  0,   0},{  0,   0}}},  // 6
  {10,{{  0,  5*60},{  1,   5*0},{ 10,  5*10},{  20,  5*20},{ 123,  5*30}, // 7
       {124,  5*40},{974,  5*50},{975, 5*-10},{1022,  5*70},{1023,  5*80}}},
   {1,{{ 32, 100},{  0,   0},{  0,   0},{  0,   0}}},  // 8
   {50, {{  0, 5*60},{  1, 5*10},{ 10,5*30},{  20,5*20},{ 123,5*30},  // 9
         {  3, 5*60},{  4, 5*10},{ 5, 5*30},{  31,5*20},{ 123,5*30},
         {  6, 5*60},{  7, 5*10},{ 8, 5*30},{  9, 5*20},{ 11, 5*30},
         {  12,5*60},{  13,5*10},{ 14,5*30},{  15,5*20},{ 16, 5*30},
         {  17,5*60},{  18,5*10},{ 19,5*30},{  21,5*20},{ 23, 5*30},
         {  22,5*60},{  24,5*10},{ 25,5*30},{  26,5*20},{ 28, 5*30},
         {  27,5*60},{  29,5*10},{ 30,5*30},{  32,5*20},{ 33, 5*30},
         {  36,5*60},{  37,5*50},{ 38,5*40},{  39,5*20},{ 34, 5*30},
         {  40,5*60},{  41,5*50},{ 42,5*40},{  43,5*20},{ 44, 5*30},
         {  45,5*60},{  46,5*50},{ 47,5*40},{  48,5*20},{ 49, 5*30}}},
   {0, {0}}, // 10
   {4,{{1  , -10},{ 14, -2},{ 23, -8},{124, -12}}},  // 12 - 11
   {90, {{  0, 5*60},{  1, 5*50},{ 10,5*40},{  20,5*20},{ 123,5*30},  // 12
         {  3, 5*60},{  4, 5*50},{ 5, 5*40},{  31,5*20},{ 50,5*30},
         {  6, 5*60},{  7, 5*50},{ 8, 5*40},{  9, 5*20},{ 11, 5*30},
         {  12,5*60},{  13,5*50},{ 14,5*40},{  15,5*20},{ 16, 5*30},
         {  17,5*60},{  18,5*50},{ 19,5*40},{  21,5*20},{ 23, 5*30},
         {  22,5*60},{  24,5*50},{ 25,5*40},{  26,5*20},{ 28, 5*30},
         {  27,5*60},{  29,5*50},{ 30,5*40},{  32,5*20},{ 33, 5*30},
         {  36,5*60},{  37,5*50},{ 38,5*40},{  39,5*20},{ 34, 5*30},
         {  40,5*60},{  41,5*50},{ 42,5*40},{  43,5*20},{ 44, 5*30},
         {  45,5*60},{  46,5*50},{ 47,5*40},{  48,5*20},{ 49, 5*30},
         { 512,5*60},{ 513,5*50},{533,5*40},{ 514,5*20},{515, 5*30},
         { 516,5*60},{ 517,5*50},{518,5*40},{ 519,5*20},{520, 5*30},
         { 521,5*60},{ 522,5*50},{523,5*40},{ 524,5*20},{525, 5*30},
         { 526,5*60},{ 527,5*50},{528,5*40},{ 529,5*20},{530, 5*30},
         { 531,5*60},{ 532,5*50},{541,5*40},{ 534,5*20},{535, 5*30},
         { 536,5*60},{ 537,5*50},{538,5*40},{ 539,5*20},{540, 5*30},
         { 542,5*60},{ 543,5*50},{544,5*40},{ 545,5*20},{546, 5*30},
         { 547,5*60},{ 548,5*50},{549,5*40},{ 550,5*20},{551, 5*30},
   }},
 // 35 Carriers in DCS_1800 & 50 Carriers in GSM_900
   {85, {{  0, 5*60},{  1, 5*50},{ 10,5*40},{  20,5*20},{ 123,5*30},  // 13
         {  3, 5*60},{  4, 5*50},{ 5, 5*40},{  31,5*20},{ 50,5*30},
         {  6, 5*60},{  7, 5*50},{ 8, 5*40},{  9, 5*20},{ 11, 5*30},
         {  12,5*60},{  13,5*50},{ 14,5*40},{  15,5*20},{ 16, 5*30},
         {  17,5*60},{  18,5*50},{ 19,5*40},{  21,5*20},{ 23, 5*30},
         {  22,5*60},{  24,5*50},{ 25,5*40},{  26,5*20},{ 28, 5*30},
         {  27,5*60},{  29,5*50},{ 30,5*40},{  32,5*20},{ 33, 5*30},
         {  36,5*60},{  37,5*50},{ 38,5*40},{  39,5*20},{ 34, 5*30},
         {  40,5*60},{  41,5*50},{ 42,5*40},{  43,5*20},{ 44, 5*30},
         {  125,5*60},{  126,5*50},{ 127,5*40},{  128,5*20},{ 129, 5*30},
         { 512,5*60},{ 513,5*50},{533,5*40},{ 514,5*20},{515, 5*30},
         { 516,5*60},{ 517,5*50},{518,5*40},{ 519,5*20},{520, 5*30},
         { 521,5*60},{ 522,5*50},{523,5*40},{ 524,5*20},{525, 5*30},
         { 526,5*60},{ 527,5*50},{528,5*40},{ 529,5*20},{530, 5*30},
         { 531,5*60},{ 532,5*50},{541,5*40},{ 534,5*20},{535, 5*30},
         { 536,5*60},{ 537,5*50},{538,5*40},{ 539,5*20},{540, 5*30},
         { 546,5*60},{ 542,5*50},{543,5*40},{ 544,5*20},{545, 5*30},
   }},
      // 80 GSM 900 Carriers & 20 PCS 1800 Carriers
   {100, {{  0, 5*60},{  1, 5*50},{ 10,5*40},{  20,5*25},{ 123,5*30},  // 14
         {  3, 5*60},{  4, 5*50},{ 5, 5*40},{  31,5*25},{ 50,5*30},
         {  6, 5*60},{  7, 5*50},{ 8, 5*40},{  9, 5*25},{ 11, 5*30},
         {  12,5*60},{  13,5*50},{ 14,5*40},{  15,5*25},{ 16, 5*30},
         {  17,5*60},{  18,5*50},{ 19,5*40},{  21,5*25},{ 23, 5*30},
         {  22,5*60},{  24,5*50},{ 25,5*40},{  26,5*25},{ 28, 5*30},
         {  27,5*60},{  29,5*50},{ 30,5*40},{  32,5*25},{ 33, 5*30},
         {  36,5*60},{  37,5*50},{ 38,5*40},{  39,5*25},{ 34, 5*30},
         {  40,5*60},{  41,5*50},{ 42,5*40},{  43,5*25},{ 44, 5*30},
         {  45,5*60},{  46,5*50},{ 47,5*40},{  48,5*25},{ 49, 5*30},
         {  51,5*60},{  52,5*50},{ 53,5*40},{  54,5*25},{ 55, 5*30},
         {  56,5*60},{  57,5*50},{ 58,5*40},{  59,5*25},{ 60, 5*30},
         {  61,5*60},{  62,5*50},{ 63,5*40},{  64,5*25},{ 65, 5*30},
         {  66,5*60},{  67,5*50},{ 68,5*40},{  69,5*25},{ 70, 5*30},
         {  72,5*60},{  73,5*50},{ 74,5*40},{  75,5*25},{ 76, 5*30},
         {  77,5*20},{  78,5*50},{ 79,5*40},{  80,5*25},{ 81, 5*30},
         { 512,5*60},{ 513,5*50},{533,5*40},{ 514,5*20},{515, 5*30},
         { 516,5*60},{ 517,5*50},{518,5*40},{ 519,5*20},{520, 5*30},
         { 521,5*60},{ 522,5*50},{523,5*40},{ 524,5*20},{525, 5*30},
         { 526,5*60},{ 527,5*50},{528,5*40},{ 529,5*20},{530, 5*30},
   }},
      // 80 GSM 900 Carriers & 20 PCS 1800 Carriers
   {100, {{  0, 5*60},{  1, 5*18},{ 10,5*40},{  20,5*18},{ 123,5*30},  // 15
         {  3, 5*60},{  4, 5*18},{ 5, 5*40},{  31,5*18},{ 50,5*30},
         {  6, 5*60},{  7, 5*18},{ 8, 5*40},{  9, 5*18},{ 11, 5*30},
         {  12,5*60},{  13,5*18},{ 14,5*40},{  15,5*18},{ 16, 5*30},
         {  17,5*60},{  18,5*18},{ 19,5*40},{  21,5*18},{ 23, 5*30},
         {  22,5*60},{  24,5*18},{ 25,5*40},{  26,5*18},{ 28, 5*30},
         {  27,5*60},{  29,5*18},{ 30,5*40},{  32,5*18},{ 33, 5*30},
         {  36,5*60},{  37,5*18},{ 38,5*40},{  39,5*18},{ 34, 5*30},
         {  40,5*60},{  41,5*18},{ 42,5*40},{  43,5*18},{ 44, 5*30},
         {  45,5*60},{  46,5*18},{ 47,5*40},{  48,5*18},{ 49, 5*30},
         {  51,5*60},{  52,5*18},{ 53,5*40},{  54,5*18},{ 55, 5*30},
         {  56,5*60},{  57,5*18},{ 58,5*40},{  59,5*18},{ 60, 5*30},
         {  61,5*60},{  62,5*18},{ 63,5*40},{  64,5*18},{ 65, 5*30},
         {  66,5*60},{  67,5*18},{ 68,5*40},{  69,5*18},{ 70, 5*30},
         {  72,5*60},{  73,5*18},{ 74,5*40},{  75,5*18},{ 76, 5*30},
         {  77,5*20},{  78,5*18},{ 79,5*40},{  80,5*18},{ 81, 5*30},
         { 512,5*60},{ 513,5*18},{533,5*40},{ 514,5*18},{515, 5*30},
         { 516,5*60},{ 517,5*18},{518,5*40},{ 519,5*18},{520, 5*30},
         { 521,5*60},{ 522,5*18},{523,5*40},{ 524,5*18},{525, 5*30},
         { 526,5*60},{ 527,5*18},{528,5*40},{ 529,5*18},{530, 5*30},
   }},
};


T_POWER_MEAS  tap_rxlev_response_american[16] = {
    {5,{{130, 105},{140, 220},{150, 250},{250, 125}, {155, -10}}}, // 0
    {0,{{0  ,  0}, {  0,   0},{  0,   0},{  0,   0}}},  // 1
    {0,{{0  ,  0}, {  0,   0},{  0,   0},{  0,   0}}},  // 2
    {0,{{0  ,  0}, {  0,   0},{  0,   0},{  0,   0}}},  // 3
    {0,{{0  ,  0}, {  0,   0},{  0,   0},{  0,   0}}},  // 4
    {0,{{0  ,  0}, {  0,   0},{  0,   0},{  0,   0}}},  // 5
    {0,{{0  ,  0}, {  0,   0},{  0,   0},{  0,   0}}},  // 6
    {0,{{0  ,  0}, {  0,   0},{  0,   0},{  0,   0}}},  // 7
    {0,{{0  ,  0}, {  0,   0},{  0,   0},{  0,   0}}},  // 8
    {0,{{0  ,  0}, {  0,   0},{  0,   0},{  0,   0}}},  // 9
    {0, {0}},  // 10
    {5,{{130, -1},{140, -2},{150, -5},{250, -12}, {155, -10}}},  // 11 
 // 40 Carriers in PCS_1900 & 50 Carriers in GSM_850
    {90,{{130, 105},{140, 220},{150, 250},{240, 125}, {160, 110},  // 12
         {131, 105},{141, 220},{151, 250},{241, 125}, {161, 110},
         {132, 105},{142, 220},{152, 250},{222, 125}, {162, 110},
         {133, 105},{143, 220},{153, 250},{223, 125}, {163, 110},
         {134, 105},{144, 220},{154, 250},{224, 125}, {164, 110},
         {135, 105},{145, 220},{155, 250},{225, 125}, {165, 110},
         {136, 105},{146, 220},{156, 250},{226, 125}, {166, 110},
         {137, 105},{147, 220},{157, 250},{227, 125}, {167, 110},
         {138, 105},{148, 220},{158, 250},{228, 125}, {168, 110},
         {139, 105},{149, 220},{159, 250},{229, 125}, {169, 110},
         { 553,105},{ 554,220},{534, 220},{514, 220}, {535, 220},
         { 536,105},{ 537,220},{538, 220},{539, 220}, {540, 220},
         { 541,105},{ 542,220},{543, 220},{544, 220}, {545, 220},
         { 546,105},{ 547,220},{548, 220},{549, 220}, {550, 220},
         { 551,105},{ 552,220},{555, 220},{556, 220}, {557, 220},
         { 558,105},{ 559,220},{560, 220},{561, 220}, {562, 220},
         { 563,105},{ 564,220},{565, 220},{566, 220}, {567, 220},
         { 568,105},{ 568,220},{570, 220},{571, 220}, {572, 220},
    }},
    // 35 Carriers in PCS_1900 & 50 Carriers in GSM_850
    {85,{{130, 105},{140, 220},{150, 250},{240, 125}, {160, 110},  // 13
         {131, 105},{141, 220},{151, 250},{241, 125}, {161, 110},
         {132, 105},{142, 220},{152, 250},{242, 125}, {162, 110},
         {133, 105},{143, 220},{153, 250},{243, 125}, {163, 110},
         {134, 105},{144, 220},{154, 250},{244, 125}, {164, 110},
         {135, 105},{145, 220},{155, 250},{245, 125}, {165, 110},
         {136, 105},{146, 220},{156, 250},{246, 125}, {166, 110},
         {137, 105},{147, 220},{157, 250},{247, 125}, {167, 110},
         {138, 105},{148, 220},{158, 250},{248, 125}, {168, 110},
         {139, 105},{149, 220},{159, 250},{249, 125}, {169, 110},
         { 125,105},{ 140,220},{150, 220},{160, 220}, {170, 220},
         { 126,105},{ 141,220},{151, 220},{161, 220}, {171, 220},
         { 127,105},{ 142,220},{152, 220},{162, 220}, {172, 220},
         { 128,105},{ 143,220},{153, 220},{163, 220}, {173, 220},
         { 129,105},{ 144,220},{154, 220},{164, 220}, {174, 220},
         { 130,105},{ 145,220},{155, 220},{165, 220}, {175, 220},
         { 131,105},{ 146,220},{156, 220},{166, 220}, {176, 220},
    }},
   // 80 GSM 850 Carriers & 20 PCS 1900 Carriers
    {100,{{130, 105},{140, 220},{150, 250},{240, 125}, {160, 110},  // 14
         {131, 105},{141, 220},{151, 250},{241, 125}, {161, 110},
         {132, 105},{142, 220},{152, 250},{242, 125}, {162, 110},
         {133, 105},{143, 220},{153, 250},{243, 125}, {163, 110},
         {134, 105},{144, 220},{154, 250},{244, 125}, {164, 110},
         {135, 105},{145, 220},{155, 250},{245, 125}, {165, 110},
         {136, 105},{146, 220},{156, 250},{246, 125}, {166, 110},
         {137, 105},{147, 220},{157, 250},{247, 125}, {167, 110},
         {138, 105},{148, 220},{158, 250},{248, 125}, {168, 110},
         {139, 105},{149, 220},{159, 250},{249, 125}, {169, 110},
         {170, 105},{180, 220},{190, 250},{200, 125}, {210, 110},
         {171, 105},{181, 220},{191, 250},{201, 125}, {211, 110},
         {172, 105},{182, 220},{192, 250},{202, 125}, {212, 110},
         {173, 105},{183, 220},{193, 250},{203, 125}, {213, 110},
         {174, 105},{184, 220},{194, 250},{204, 125}, {214, 110},
         {175, 105},{185, 220},{195, 250},{205, 125}, {215, 110},
         { 553,105},{ 554,220},{534, 220},{514, 220}, {535, 220},
         { 536,105},{ 537,220},{538, 220},{539, 220}, {540, 220},
         { 541,105},{ 542,220},{543, 220},{544, 220}, {545, 220},
         { 546,105},{ 547,220},{548, 220},{549, 220}, {550, 220},
    }},
   // 80 GSM 850 Carriers & 20 PCS 1900 Carriers
    {100,{{130, 105},{140, 80},{150, 250},{240, 90}, {160, 110},  // 15
         {131, 105},{141, 80},{151, 250},{241, 90}, {161, 110},
         {132, 105},{142, 80},{152, 250},{242, 90}, {162, 110},
         {133, 105},{143, 80},{153, 250},{243, 90}, {163, 110},
         {134, 105},{144, 80},{154, 250},{244, 90}, {164, 110},
         {135, 105},{145, 80},{155, 250},{245, 90}, {165, 110},
         {136, 105},{146, 80},{156, 250},{246, 90}, {166, 110},
         {137, 105},{147, 80},{157, 250},{247, 90}, {167, 110},
         {138, 105},{148, 80},{158, 250},{248, 90}, {168, 110},
         {139, 105},{149, 80},{159, 250},{249, 90}, {169, 110},
         {170, 105},{180, 80},{190, 250},{200, 90}, {210, 110},
         {171, 105},{181, 80},{191, 250},{201, 90}, {211, 110},
         {172, 105},{182, 80},{192, 250},{202, 90}, {212, 110},
         {173, 105},{183, 80},{193, 250},{203, 90}, {213, 110},
         {174, 105},{184, 80},{194, 250},{204, 90}, {214, 110},
         {175, 105},{185, 80},{195, 250},{205, 90}, {215, 110},
         { 553,105},{ 554,80},{534, 220},{514, 90}, {535, 220},
         { 536,105},{ 537,80},{538, 220},{539, 90}, {540, 220},
         { 541,105},{ 542,80},{543, 220},{544, 90}, {545, 220},
         { 546,105},{ 547,80},{548, 220},{549, 90}, {550, 220},
    }},
};

/*lint +e785 (Info -- Too few initializers for aggregate)*/
#endif  /* _SIMULATION_ */

/*==== PRIVATE ====================================================*/

LOCAL void pei_not_supported (void *);

#ifdef OPTION_RELATIVE
LOCAL ULONG offset;
#endif

#define TRACING
/*==== VARIABLES ==================================================*/
#ifdef _SIMULATION_
LOCAL BOOL              first_access = TRUE;
#endif /* _SIMULATION_ */

LOCAL T_MONITOR         alr_mon;

/*==== FUNCTIONS ==================================================*/

LOCAL const T_FUNC mph_table[] = {
  MAK_FUNC_N ( pei_not_supported        , 0                        ),
  MAK_FUNC_N ( pei_not_supported        , 0                        ),
  MAK_FUNC_N ( pei_not_supported        , 0                        ),
  MAK_FUNC_0 ( ma_mph_idle_req          , MPH_IDLE_REQ             ),
  MAK_FUNC_0 ( ma_mph_neighbourcell_req , MPH_NEIGHBOURCELL_REQ    ),
  MAK_FUNC_0 ( ma_mph_emo_req           , MPH_EMO_REQ              ),
  MAK_FUNC_0 ( ma_mph_dedicated_req     , MPH_DEDICATED_REQ        ),
  MAK_FUNC_0 ( ma_mph_dedicated_fail_req, MPH_DEDICATED_FAIL_REQ   ),
  MAK_FUNC_0 ( ma_mph_ciphering_req     , MPH_CIPHERING_REQ        ),
  MAK_FUNC_0 ( ma_mph_freq_redef_req    , MPH_FREQ_REDEF_REQ       ),
  MAK_FUNC_0 ( ma_mph_channel_mode_req  , MPH_CHANNEL_MODE_REQ     ),
  MAK_FUNC_0 ( ma_mph_deactivate_req    , MPH_DEACTIVATE_REQ       ),
  MAK_FUNC_0 ( ma_mph_classmark_req     , MPH_CLASSMARK_REQ        ),
  MAK_FUNC_N ( ma_mph_ext_meas_req      , MPH_EXT_MEAS_REQ         ),
  MAK_FUNC_N ( ma_mph_ncell_pos_req     , MPH_NCELL_POS_REQ        ),
  MAK_FUNC_N ( pei_not_supported        , 0                        ),
  MAK_FUNC_N ( pei_not_supported        , 0                        ),
  MAK_FUNC_0 ( ma_mph_tch_loop_req      , MPH_TCH_LOOP_REQ         ),
  MAK_FUNC_0 ( ma_mph_dai_req           , MPH_DAI_REQ              ),
  MAK_FUNC_0 ( ma_mph_cbch_req          , MPH_CBCH_REQ             ),
  MAK_FUNC_0 ( ma_mph_identity_req      , MPH_IDENTITY_REQ         ),
  MAK_FUNC_0 ( ma_mph_power_req         , MPH_POWER_REQ            ),
  MAK_FUNC_0 ( ma_mph_bsic_req          , MPH_BSIC_REQ             ),
  MAK_FUNC_0 ( ma_mph_random_access_req , MPH_RANDOM_ACCESS_REQ    ),

#if defined (TI_PS_FF_EMR)
  MAK_FUNC_0 ( ma_mph_mon_ctrl_req      , MPH_MON_CTRL_REQ         ),
#elif defined (GPRS) 
  MAK_FUNC_0 ( gprs_alr_mon_ctrl_req    , MPH_MON_CTRL_REQ         ),
#else
  MAK_FUNC_N ( pei_not_supported        , 0                        ),
#endif
  MAK_FUNC_0 ( ma_mph_sync_req          , MPH_SYNC_REQ             ),
  MAK_FUNC_0 ( ma_mph_clean_buf_req     , MPH_CLEAN_BUF_REQ        ),
  MAK_FUNC_0 ( ma_mph_stop_dedi_req     , MPH_STOP_DEDICATED_REQ   ),
#ifdef GPRS
  MAK_FUNC_0 ( ma_mph_meas_rep_req      , MPH_MEAS_REP_REQ         ),
#else
  MAK_FUNC_N ( pei_not_supported        , 0                        ),
#endif
#if defined (REL99) && defined (TI_PS_FF_EMR)
  MAK_FUNC_0 ( ma_mph_enhpara_update_req, MPH_ENHPARA_UPDATE_REQ   )
#else
  MAK_FUNC_N ( pei_not_supported        , 0                        )
#endif
};

/*
 * The MPHC jump table uses the
 * original TI opcodes !!!
 */

LOCAL const T_FUNC mphc_table[] = {
  MAK_FUNC_N ( pei_not_supported               , 0                            ),
  MAK_FUNC_N ( pei_not_supported               , TST_TEST_HW_REQ              ), /*  1 */
  MAK_FUNC_0 ( csf_show_version                , TST_TEST_HW_CON              ), /*  2 */
  MAK_FUNC_N ( pei_not_supported               , TST_TIMESTAMP_MSG            ), /*  3 */
  MAK_FUNC_N ( pei_not_supported               , TST_SLEEP_REQ                ), /*  4 */
  MAK_FUNC_N ( pei_not_supported               , 0                            ), /*  5 */
  MAK_FUNC_N ( pei_not_supported               , 0                            ), /*  6 */
  MAK_FUNC_N ( pei_not_supported               , 0                            ), /*  7 */
  MAK_FUNC_N ( pei_not_supported               , 0                            ), /*  8 */
  MAK_FUNC_N ( pei_not_supported               , 0                            ), /*  9 */
  MAK_FUNC_N ( pei_not_supported               , 0                            ), /* 10 */
  MAK_FUNC_N ( pei_not_supported               , MPHC_RXLEV_REQ               ), /* 11 */
  MAK_FUNC_0 ( ma_mphc_rxlev_ind               , MPHC_RXLEV_IND               ),
  MAK_FUNC_N ( pei_not_supported               , MPHC_STOP_RXLEV_REQ          ),
  MAK_FUNC_0 ( ma_mphc_stop_rxlev_cnf          , MPHC_STOP_RXLEV_CON          ),
  MAK_FUNC_0 ( pei_not_supported               , MPHC_NETWORK_SYNC_REQ        ),
  MAK_FUNC_N ( ma_mphc_network_sync_ind        , MPHC_NETWORK_SYNC_IND        ),
  MAK_FUNC_0 ( pei_not_supported               , MPHC_STOP_NETWORK_SYNC_REQ   ),
#if defined(STOP_SYNC_TASK)
  MAK_FUNC_N ( ma_mphc_stop_network_sync_cnf   , MPHC_STOP_NETWORK_SYNC_CON   ),
#else /* STOP_SYNC_TASK */
  MAK_FUNC_N ( ma_mphc_empty_cnf               , MPHC_STOP_NETWORK_SYNC_CON   ),
#endif /* STOP_SYNC_TASK */
  MAK_FUNC_0 ( pei_not_supported               , MPHC_NEW_SCELL_REQ           ),
  MAK_FUNC_N ( ma_mphc_new_scell_cnf           , MPHC_NEW_SCELL_CON           ),
  MAK_FUNC_N ( pei_not_supported               , MPHC_START_CCCH_REQ          ),
  MAK_FUNC_N ( pei_not_supported               , MPHC_STOP_CCCH_REQ           ),
  MAK_FUNC_0 ( ma_mphc_empty_cnf               , MPHC_STOP_CCCH_CON           ),
  MAK_FUNC_N ( pei_not_supported               , MPHC_SCELL_NBCCH_REQ         ),
  MAK_FUNC_N ( pei_not_supported               , MPHC_SCELL_EBCCH_REQ         ),
  MAK_FUNC_N ( pei_not_supported               , MPHC_STOP_SCELL_BCCH_REQ     ),
  MAK_FUNC_0 ( ma_mphc_empty_cnf               , MPHC_STOP_SCELL_BCCH_CON     ),
  MAK_FUNC_N ( pei_not_supported               , MPHC_NCELL_BCCH_REQ          ),
  MAK_FUNC_0 ( ma_mphc_ncell_bcch_ind          , MPHC_NCELL_BCCH_IND          ),
  MAK_FUNC_N ( pei_not_supported               , MPHC_STOP_NCELL_BCCH_REQ     ),
  MAK_FUNC_0 ( ma_mphc_empty_cnf               , MPHC_STOP_NCELL_BCCH_CON     ),
  MAK_FUNC_N ( pei_not_supported               , MPHC_NCELL_SYNC_REQ          ),
  MAK_FUNC_0 ( ma_mphc_ncell_sync_ind          , MPHC_NCELL_SYNC_IND          ),
  MAK_FUNC_N ( pei_not_supported               , MPHC_STOP_NCELL_SYNC_REQ     ),
  MAK_FUNC_0 ( ma_mphc_stop_ncell_sync_cnf     ,  MPHC_STOP_NCELL_SYNC_CON    ),
  MAK_FUNC_N ( pei_not_supported               , MPHC_RXLEV_PERIODIC_REQ      ),
  MAK_FUNC_0 ( ma_mphc_rxlev_periodic_ind      , MPHC_RXLEV_PERIODIC_IND      ),
  MAK_FUNC_N ( pei_not_supported               , MPHC_STOP_RXLEV_PERIODIC_REQ ),
  MAK_FUNC_0 ( ma_mphc_empty_cnf               , MPHC_STOP_RXLEV_PERIODIC_CON ),
  MAK_FUNC_N ( pei_not_supported               , MPHC_CONFIG_CBCH_REQ         ),
  MAK_FUNC_N ( pei_not_supported               , MPHC_CBCH_SCHEDULE_REQ       ),
  MAK_FUNC_N ( pei_not_supported               , MPHC_CBCH_UPDATE_REQ         ),
  MAK_FUNC_N ( pei_not_supported               , MPHC_CBCH_INFO_REQ           ),
  MAK_FUNC_N ( pei_not_supported               , MPHC_STOP_CBCH_REQ           ),
  MAK_FUNC_N ( ma_mphc_empty_cnf               , MPHC_STOP_CBCH_CON           ),
  MAK_FUNC_N ( pei_not_supported               , MPHC_RA_REQ                  ),
  MAK_FUNC_0 ( ma_mphc_ra_cnf                  , MPHC_RA_CON                  ),
  MAK_FUNC_N ( pei_not_supported               , MPHC_STOP_RA_REQ             ),
  MAK_FUNC_0 ( ma_mphc_empty_cnf               , MPHC_STOP_RA_CON             ),
  MAK_FUNC_0 ( ma_mphc_data_ind                , MPHC_DATA_IND                ),
  MAK_FUNC_N ( pei_not_supported               , MPHC_IMMED_ASSIGN_REQ        ),
  MAK_FUNC_N ( pei_not_supported               , MPHC_CHANNEL_ASSIGN_REQ      ),
  MAK_FUNC_N ( pei_not_supported               , MPHC_ASYNC_HO_REQ            ),
  MAK_FUNC_N ( pei_not_supported               , MPHC_SYNC_HO_REQ             ),
  MAK_FUNC_N ( pei_not_supported               , MPHC_PRE_SYNC_HO_REQ         ),
  MAK_FUNC_N ( pei_not_supported               , MPHC_PSEUDO_SYNC_HO_REQ      ),
  MAK_FUNC_N ( pei_not_supported               , MPHC_STOP_DEDICATED_REQ      ),
  MAK_FUNC_0 ( ma_mphc_change_frequency_cnf    , MPHC_CHANGE_FREQUENCY_CON    ),
  MAK_FUNC_0 ( ma_mphc_async_ho_cnf            , MPHC_ASYNC_HO_CON            ),
  MAK_FUNC_0 ( ma_mphc_channel_assign_cnf      , MPHC_CHANNEL_ASSIGN_CON      ),
  MAK_FUNC_0 ( ma_mphc_empty_cnf               , MPHC_CHANNEL_MODE_MODIFY_CON ),
  MAK_FUNC_0 ( ma_mphc_handover_fail_cnf       , MPHC_HANDOVER_FAIL_CON       ),
  MAK_FUNC_0 ( ma_mphc_immed_assign_cnf        , MPHC_IMMED_ASSIGN_CON        ),
  MAK_FUNC_0 ( ma_mphc_pre_sync_ho_cnf         , MPHC_PRE_SYNC_HO_CON         ),
  MAK_FUNC_0 ( ma_mphc_empty_cnf               , MPHC_SET_CIPHERING_CON       ),
  MAK_FUNC_0 ( ma_mphc_sync_ho_cnf             , MPHC_SYNC_HO_CON             ),
  MAK_FUNC_0 ( ma_mphc_ta_fail_ind             , MPHC_TA_FAIL_IND             ),
  MAK_FUNC_0 ( ma_mphc_handover_finished_ind   , MPHC_HANDOVER_FINISHED       ),
  MAK_FUNC_N ( pei_not_supported               , MPHC_CHANGE_FREQUENCY        ),
  MAK_FUNC_N ( pei_not_supported               , MPHC_CHANNEL_MODE_MODIFY_REQ ),
  MAK_FUNC_N ( pei_not_supported               , MPHC_HANDOVER_FAIL_REQ       ),
  MAK_FUNC_N ( pei_not_supported               , MPHC_SET_CIPHERING_REQ       ),
  MAK_FUNC_0 ( ma_mphc_meas_report_ind         , MPHC_MEAS_REPORT             ),
  MAK_FUNC_N ( pei_not_supported               , MPHC_UPDATE_BA_LIST          ),
  MAK_FUNC_N ( pei_not_supported               , MPHC_NCELL_FB_SB_READ        ),
  MAK_FUNC_N ( pei_not_supported               , MPHC_NCELL_SB_READ           ),
  MAK_FUNC_N ( pei_not_supported               , MPHC_NCELL_BCCH_READ         ),
  MAK_FUNC_N ( pei_not_supported               , MPHC_INIT_IDS_REQ            ), /* 78 */
  MAK_FUNC_N ( pei_not_supported               , MPHC_INIT_IDS_CON            ), /* 79 = is not used */
  MAK_FUNC_N ( pei_not_supported               , 0                            ), /* 80 = used by L1 */
  MAK_FUNC_N ( pei_not_supported               , 0                            ), /* 81 */
  MAK_FUNC_N ( pei_not_supported               , 0                            ), /* 82 */
  MAK_FUNC_N ( pei_not_supported               , 0                            ), /* 83 */
  MAK_FUNC_N ( pei_not_supported               , 0                            ), /* 84 */
  MAK_FUNC_N ( pei_not_supported               , 0                            ), /* 85 */
  MAK_FUNC_N ( pei_not_supported               , 0                            ), /* 86 */
  MAK_FUNC_N ( pei_not_supported               , 0                            ), /* 87 */
  MAK_FUNC_N ( pei_not_supported               , 0                            ), /* 88 */
  MAK_FUNC_N ( pei_not_supported               , 0                            ), /* 89 */
  MAK_FUNC_N ( pei_not_supported               , 0                            ), /* 90 */
  MAK_FUNC_N ( pei_not_supported               , 0                            ), /* 91 */
  MAK_FUNC_N ( pei_not_supported               , 0                            ), /* 92 */
  MAK_FUNC_N ( pei_not_supported               , 0                            ), /* 93 */
  MAK_FUNC_N ( pei_not_supported               , 0                            ), /* 94 */
  MAK_FUNC_N ( pei_not_supported               , 0                            ), /* 95 */
  MAK_FUNC_N ( pei_not_supported               , 0                            ), /* 96 */
  MAK_FUNC_N ( pei_not_supported               , OML1_CLOSE_TCH_LOOP_REQ      ), /* 97 */
  MAK_FUNC_N ( pei_not_supported               , OML1_OPEN_TCH_LOOP_REQ       ),
  MAK_FUNC_N ( pei_not_supported               , OML1_START_DAI_TEST_REQ      ),
  MAK_FUNC_N ( pei_not_supported               , OML1_STOP_DAI_TEST__REQ      ),
  MAK_FUNC_0 ( ma_mphc_empty_cnf               , OML1_CLOSE_TCH_LOOP_CON      ),
  MAK_FUNC_0 ( ma_mphc_empty_cnf               , OML1_OPEN_TCH_LOOP_CON       ),
  MAK_FUNC_0 ( ma_mphc_empty_cnf               , OML1_START_DAI_TEST_CON      ),
  MAK_FUNC_0 ( ma_mphc_empty_cnf               , OML1_STOP_DAI_TEST_CON       ),
  MAK_FUNC_0 ( ma_mphc_adc_ind                 , MPHC_ADC_IND                 ), /* will be send to CST directly */
  MAK_FUNC_N ( pei_not_supported               , TRACE_INFO                   ),
  MAK_FUNC_N ( pei_not_supported               , L1_STATS_REQ                 ),
  MAK_FUNC_N ( pei_not_supported               , L1_DUMMY_FOR_SIM             ),
  MAK_FUNC_0 ( ma_mphc_data_ind                , PH_DATA_IND                  ),
  MAK_FUNC_N ( pei_not_supported               , MPHC_NETWORK_LOST_IND        ), /* 110 = send to L1 */
  MAK_FUNC_N ( pei_not_supported               , 0                            ), /* 111 = MMI_ADC_REQ */
  MAK_FUNC_N ( pei_not_supported               , 0                            ), /* 112 = MMI_STOP_ADC_REQ */
  MAK_FUNC_N ( pei_not_supported               , 0                            ), /* 113 = MMI_STOP_ADC_CON */
  MAK_FUNC_N ( pei_not_supported               , MPHC_INIT_L1_REQ             ), /* 114 */
  MAK_FUNC_N ( ma_mphc_init_l1_cnf             , MPHC_INIT_L1_CON             ), /* 115 */
  MAK_FUNC_N ( pei_not_supported               , 0                            ), /* 116 */
  MAK_FUNC_N ( pei_not_supported               , 0                            ), /* 117 */
  MAK_FUNC_N ( pei_not_supported               , 0                            ), /* 118 */
  MAK_FUNC_N ( pei_not_supported               , 0                            ), /* 119 = L1_TEST_HW_INFO */
  MAK_FUNC_N ( pei_not_supported               , 0                            ), /* 120 */
  MAK_FUNC_N ( pei_not_supported               , 0                            ), /* 121 */
  MAK_FUNC_N ( pei_not_supported               , MPHC_NCELL_LIST_SYNC_REQ     ), /* 122 */
  MAK_FUNC_N ( pei_not_supported               , 0                            ), /* 123 */
  MAK_FUNC_N ( pei_not_supported               , 0                            ), /* 124 */
  MAK_FUNC_N ( pei_not_supported               , 0                            ), /* 125 */
  MAK_FUNC_N ( pei_not_supported               , 0                            ), /* 126 */
  MAK_FUNC_N ( pei_not_supported               , 0                            ), /* 127 */
  MAK_FUNC_N ( ma_mphc_stop_dedi_con           , MPHC_STOP_DEDICATED_CON      ) /* 128 */
};

LOCAL const T_FUNC mmi_table[] = {
  MAK_FUNC_N ( pei_not_supported        , 0                        ),
  MAK_FUNC_N ( pei_not_supported        , 0                        ),
  MAK_FUNC_N ( pei_not_supported        , 0                        ),
  MAK_FUNC_N ( pei_not_supported        , 0                        ),
  MAK_FUNC_N ( pei_not_supported        , 0                        ),
  MAK_FUNC_N ( pei_not_supported        , 0                        ),
  MAK_FUNC_N ( pei_not_supported        , 0                        ),
  MAK_FUNC_0 ( ma_mmi_cbch_req          , MMI_CBCH_REQ             ),
  MAK_FUNC_N ( pei_not_supported        , 0                        ),
  MAK_FUNC_N ( pei_not_supported        , 0                        ),
  MAK_FUNC_N ( pei_not_supported        , 0                        ),
  MAK_FUNC_0 ( ma_mmi_sat_cbch_dwnld_req, MMI_SAT_CBCH_DWNLD_REQ   )
};

#ifdef FF_EM_MODE
LOCAL const T_FUNC em_ul_table[] = {
  MAK_FUNC_N( pei_not_supported,              0                         ), /* 0x00*/
  MAK_FUNC_N( pei_not_supported,              0                         ), /* 0x01*/
  MAK_FUNC_N( pei_not_supported,              0                         ), /* 0x02*/
  MAK_FUNC_N( pei_not_supported,              0                         ), /* 0x03*/
  MAK_FUNC_N( pei_not_supported,              0                         ), /* 0x04*/
  MAK_FUNC_N( pei_not_supported,              0                         ), /* 0x05*/
  MAK_FUNC_N( pei_not_supported,              0                         ), /* 0x06*/
  MAK_FUNC_N( pei_not_supported,              0                         ), /* 0x07*/
  MAK_FUNC_N( pei_not_supported,              0                         ), /* 0x08*/
  MAK_FUNC_0( l1_em_l1_event_req,             EM_L1_EVENT_REQ           ), /* 0x09*/
  MAK_FUNC_N( pei_not_supported,              0                         ), /* 0x0A*/
  MAK_FUNC_N( pei_not_supported,              0                         )  /* 0x0B*/
};
#endif /* FF_EM_MODE */


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6103)              MODULE  : ALR_PEI             |
| STATE   : code                       ROUTINE : pei_primitive       |
+--------------------------------------------------------------------+

  PURPOSE : Process protocol specific primitive.

*/
LOCAL SHORT pei_primitive (void * ptr)
{
  T_PRIM * prim = ptr;
  /*
   *                |     |      |
   *               MPH   PH     MMI       UPLINK
   *                |     |      |
   *      +---------v-----v------v---+
   *      |                          |
   *      |           ALR            |
   *      |                          |
   *      +------------^-------------+
   *                   |
   *                  MPHC                DOWNLINK
   *                   |
   *
   */

  if (prim NEQ NULL)
  {
    ULONG            opc = prim->custom.opc;
    USHORT           n;
    const T_FUNC    *table;

    VSI_PPM_REC ((T_PRIM_HEADER *)prim, __FILE__, __LINE__);

#if defined TRACING
    PTRACE_IN (opc);
#endif

    switch (SAP_NR(opc))
    {
      case    MPH_DL:
          table =  mph_table ;
          n = TAB_SIZE(mph_table);
        break;
      case    0x0000:
      case    0x0100:
        /*
         * use same table as PH_DATA_IND is the only prim with 0x0100
         * and is at location 109
         */
        table =     mphc_table;    n = TAB_SIZE (mphc_table);    break;
      case    0x0E00: table =     mmi_table ;    n = TAB_SIZE (mmi_table);     break;
#ifdef GPRS
      case    TB_UL : gprs_alr_get_table (&table, &n); break;
#endif /* GPRS */
#ifdef FF_EM_MODE
      case    EM_Ul:  table =     em_ul_table;   n = TAB_SIZE (em_ul_table);   break;
#endif /* FF_EM_MODE*/
      default       : table =     NULL ;         n = 0;                        break;
    }

    if (table NEQ NULL )
    {

      if (PRIM_NR(opc) < n)
      {

        table += PRIM_NR(opc);
#ifdef PALLOC_TRANSITION
        P_SDU(prim) = table->soff ? (T_sdu*) (((char*)&prim->data) + table->soff) : 0;
#ifndef NO_COPY_ROUTING
        P_LEN(prim) = table->size + sizeof (T_PRIM_HEADER);
#endif /* NO_COPY_ROUTING */
#endif /* PALLOC_TRANSITION */
        JUMP (table->func) (P2D(prim));
      }
      else
      {
        pei_not_supported (P2D(prim));
      }
      return PEI_OK;
    }

#ifdef GSM_ONLY
    PFREE (P2D(prim))
    return PEI_ERROR;
#else
    if (opc & SYS_MASK)
      vsi_c_primitive (VSI_CALLER prim);
    else
    {
      PFREE (P2D(prim))
      return PEI_ERROR;
    }
#endif
  }
  return PEI_OK;
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6103)              MODULE  : ALR_PEI             |
| STATE   : code                       ROUTINE : pei_not_supported   |
+--------------------------------------------------------------------+

  PURPOSE : An unsupported primitive is received.

*/

static void pei_not_supported (void *data)
{
  PFREE (data);
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6103)              MODULE  : ALR_PEI             |
| STATE   : code                       ROUTINE : pei_init            |
+--------------------------------------------------------------------+

  PURPOSE : Initialize Protocol Stack Entity

*/
LOCAL SHORT pei_init (T_HANDLE handle)
{
  GET_INSTANCE_DATA;
  UBYTE version;
  pl_handle = handle;

#ifdef TI_PS_HCOMM_CHANGE
  if (!cl_hcom_all_handles_open())
  {
    return PEI_ERROR;
  }

#else /* for hCommHandles backward compatibility */
  if (hCommPL < VSI_OK)
  {
    if ((hCommPL = vsi_c_open (VSI_CALLER PL_NAME)) < VSI_OK)
      return PEI_ERROR;
  }
  if (hCommL1 < VSI_OK)
  {
    if ((hCommL1 = vsi_c_open (VSI_CALLER L1_NAME)) < VSI_OK)
      return PEI_ERROR;
  }
  if (hCommDL < VSI_OK)
  {
    if ((hCommDL = vsi_c_open (VSI_CALLER DL_NAME)) < VSI_OK)
      return PEI_ERROR;
  }

  if (hCommRR < VSI_OK)
  {
    if ((hCommRR = vsi_c_open (VSI_CALLER RR_NAME)) < VSI_OK)
      return PEI_ERROR;
  }

  if (hCommCST < VSI_OK)
  {
    if ((hCommCST = vsi_c_open (VSI_CALLER CST_NAME)) < VSI_OK)
      return PEI_ERROR;
  }

  if (hCommMMI < VSI_OK)
  {
    /*
     * Open MMI (Layer 4)
     */

    if ((hCommMMI = vsi_c_open (VSI_CALLER ACI_NAME)) < VSI_OK)
      return PEI_ERROR;
  }
#endif

#ifdef GPRS

  if (hCommGPL < VSI_OK)
  {
    if ((hCommGPL = vsi_c_open (VSI_CALLER GPL_NAME)) < VSI_OK)
      return PEI_ERROR;
  }

#endif /* #ifdef GPRS */

#if defined (_SIMULATION_)
  ccd_init();
#endif  /* _SIMULATION_ */

  ma_init ();
  cs_init ();
  pch_init ();
  dedi_init ();
  nc_init ();
  pch_init ();
  rach_init ();
  dedi_init ();
  cb_init ();
  tim_init();


#ifdef FF_EM_MODE
  /*
     initialise event flags
  */
  em_init_l1_event_trace();
  /*
    initialise alr semaphor for EM event tracing
  */
  em_l1_sem_init();
#endif /* FF_EM_MODE */


  pcm_Init ();
  pcm_ReadFile ((UBYTE *)EF_MSCAP_ID, SIZE_EF_MSCAP,
                (UBYTE *)&alr_data->mscap, &version);


  alr_trc_init ();

  return PEI_OK;
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6103)              MODULE  : ALR_PEI             |
| STATE   : code                       ROUTINE : pei_timeout         |
+--------------------------------------------------------------------+

  PURPOSE : Process timeout

*/
LOCAL SHORT pei_timeout (USHORT index)
{
#if !defined(TIMER_TRACE)
  TRACE_FUNCTION ("pei_timeout ()");
#endif  /* !TIMER_TRACE */

  /*
   * Handle Timeouts
   */
  tim_exec_timeout (index);

  return PEI_OK;
}

#ifdef _SIMULATION_
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6103)              MODULE  : ALR_PEI             |
| STATE   : code                       ROUTINE : pei_exit            |
+--------------------------------------------------------------------+

  PURPOSE : Close Resources and terminate

*/

LOCAL SHORT pei_exit (void)
{
  /*
   * clean up communication
   */
#ifdef TI_PS_HCOMM_CHANGE
#else /* for hCommHandles backward compatibility */
  vsi_c_close (VSI_CALLER hCommL1);
  hCommL1 = VSI_ERROR;

  vsi_c_close (VSI_CALLER hCommPL);
  hCommPL = VSI_ERROR;

  vsi_c_close (VSI_CALLER hCommDL);
  hCommDL = VSI_ERROR;

  vsi_c_close (VSI_CALLER hCommRR);
  hCommRR = VSI_ERROR;

  vsi_c_close (VSI_CALLER hCommMMI);
  hCommMMI = VSI_ERROR;

  vsi_c_close (VSI_CALLER hCommCST);
  hCommCST = VSI_ERROR;
#endif

#ifdef GPRS

  vsi_c_close (VSI_CALLER hCommGPL);
  hCommGPL = VSI_ERROR;

#endif /* #ifdef GPRS */
  alr_trc_exit ();

#ifdef FF_EM_MODE
  em_l1_sem_exit();
#endif /* FF_EM_MODE */

#if defined (_SIMULATION_)
  ccd_exit();
#endif  /* _SIMULATION_ */

  return PEI_OK;
}
#endif /* _SIMULATION_ */

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6103)              MODULE  : ALR_PEI             |
| STATE   : code                       ROUTINE : pei_config          |
+--------------------------------------------------------------------+

  PURPOSE : Dynamic Configuration

*/
#if !defined (NCONFIG)
LOCAL const KW_DATA kwtab[] = {
                   ALR_CONFIG,            ID_CONFIG,
                   ALR_MON_COUNT_IDLE,    ID_MON_COUNTER_IDLE,
                   ALR_MON_COUNT_DEDI,    ID_MON_COUNTER_DEDI,
                   ALR_TRC_DATA_IND,      ID_TRC_DATA_IND,
#if defined (_SIMULATION_)
                   ALR_STD,               ID_STD,
                   ALR_MB_TESTING,        ID_MB_TESTING,
#endif  /* _SIMULATION_ */
                   ALR_RACH_FAILURE,      ID_RACH_FAILURE,
                   ALR_EOTD,              ID_EOTD,
                   "",                    0
                  };

UBYTE v_mon_trc_data_ind = 0;
UBYTE v_cfg_rach_failure = 0;
#endif
/* Implements Measure#36 */
#ifndef NCONFIG
LOCAL SHORT pei_config (T_PEI_CONFIG inString)
{
  GET_INSTANCE_DATA;
  SHORT ret = PEI_OK;

#if !defined (NCONFIG)
  {
    char    * s = inString;
    SHORT     valno;
    SHORT     keyno;
    char    * keyw;
    char    * val [10];

    tok_init(s);

    /*
     * Parse next keyword and number of variables
     */
    while ((valno = tok_next(&keyw,val)) NEQ TOK_EOCS)
    {
      keyno = tok_key((KW_DATA *)kwtab,keyw);
      switch (keyno)
      {
        case TOK_NOT_FOUND:
          TRACE_ERROR ("[PEI_CONFIG]: Illegal Keyword");
          ret = PEI_ERROR;
          break;

        case ID_CONFIG:
          if (valno EQ 3)
          {
            alr_data->mmi         = (UBYTE)atoi (val[0]);
            alr_data->keypad      = (UBYTE)atoi (val[1]);
            alr_data->ext_display = (UBYTE)atoi (val[2]);
          }
          else
          {
            TRACE_ERROR ("[PEI_CONFIG]: Wrong Number of Parameters");
            ret = PEI_ERROR;
          }
          break;
        case ID_RACH_FAILURE:
          v_cfg_rach_failure = !v_cfg_rach_failure;
          TRACE_EVENT_P1("rach failure %d", v_cfg_rach_failure);
          break;
#if !defined (NTRACE)
        case ID_MON_COUNTER_IDLE:
          if (valno EQ 1)
            v_mon_counter_idle = (UBYTE)(atoi(val[0]) & 1);
          break;

        case ID_MON_COUNTER_DEDI:
          if (valno EQ 1)
            v_mon_counter_dedi = (UBYTE)(atoi(val[0]) & 1);
          break;
#endif  /* !NTRACE */

#if defined (_SIMULATION_)
        case ID_STD:
          if (valno EQ 1)
          {
            std = (UBYTE)atoi (val[0]);
            pcm_Init ();
            {
              rr_csf_check_rfcap (TRUE);
            }
          }
          else
          {
            TRACE_ERROR ("[PEI_CONFIG]: Wrong Number of Parameters");
            ret = PEI_ERROR;
          }
          break;
        case ID_MB_TESTING:
          if (valno EQ 1)
          {
            alr_data->mb_testing = (UBYTE)atoi (val[0]);
          }
          else
          {
            TRACE_ERROR ("[PEI_CONFIG]: Wrong Number of Parameters");
            ret = PEI_ERROR;
          }
          break;
#endif  /* _SIMULATION_ */

#if !defined (NTRACE)
        case ID_TRC_DATA_IND:
          if (valno EQ 1)
            v_mon_trc_data_ind = (UBYTE)atoi(val[0]);
          else
            v_mon_trc_data_ind = ID_TRC_DATA_IND_INIT2;
          SYST_TRACE_P((SYST,"[PEI_CONFIG] TRC_DATA_IND %02x", v_mon_trc_data_ind));
          break;
#endif  /* !NTRACE */
        case ID_EOTD:
          v_eotd = 1;
          break;
        default:
          ret = PEI_ERROR;
          SYST_TRACE_P((0,0xffff,"[PEI_CONFIG] \"%s\" unknown ID %u", s, keyno));
          break;
      }
    }
  }
#endif  /* end !defined (NCONFIG) */
 return ret;
}
#endif /* NCONFIG */

#if !defined(WIN32)
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6103)              MODULE  : ALR_PEI             |
| STATE   : code                       ROUTINE : l1_version          |
+--------------------------------------------------------------------+

  PURPOSE : maps the function alr_version

*/
GLOBAL CHAR* l1_version (void)
{
  return alr_version();
}
#endif  /* !WIN32 */

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6103)              MODULE  : ALR_PEI             |
| STATE   : code                       ROUTINE : pl_pei_config       |
+--------------------------------------------------------------------+

  PURPOSE : Dynamic Configuration

*/
/* Implements Measure#36 */
#ifndef NCONFIG
GLOBAL SHORT pl_pei_config (char * inString, char * dummy)
{

  pei_config (inString);
  return PEI_OK;
}
#endif /* NCONFIG */

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6103)              MODULE  : ALR_PEI             |
| STATE   : code                       ROUTINE : pei_monitor         |
+--------------------------------------------------------------------+

  PURPOSE : Monitoring of physical Parameters

*/

LOCAL SHORT pei_monitor (void ** monitor)
{
#if defined (_TMS470)
  static char buf[60];

  USHORT build, jtag, rev, hw;

  /*
   * Retrieve hardware info and build from library
   */
  hw = 0;

  build = IQ_GetBuild();
  jtag  = IQ_GetJtagId();
  rev   = IQ_GetRevision();

  sprintf (buf, "HW=%04x Build=%04X, jtag=%04X, rev=%04X", hw, build,jtag, rev);
  TRACE_EVENT (buf);

  alr_mon.version = buf;
#else
  alr_mon.version = VERSION_ALR;
#endif  /* _TMS470 */

  *monitor = &alr_mon;

  return PEI_OK;
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6103)              MODULE  : ALR_PEI             |
| STATE   : code                       ROUTINE : pei_create          |
+--------------------------------------------------------------------+

  PURPOSE : Create the Protocol Stack Entity

*/
GLOBAL SHORT pl_pei_create (T_PEI_INFO **info)
{
  static const T_PEI_INFO pei_info =
  {
    "PL",
    {
      pei_init,
#ifdef _SIMULATION_
      pei_exit,
#else
      NULL,
#endif
      pei_primitive,
      pei_timeout,     /* timer expiry function   */
      NULL,            /* no signal function */
      NULL,            /* no run function    */
/* Implements Measure#36 */
#ifdef NCONFIG
      NULL,            /* no pei_config function */
#else /* not NCONFIG */
      pei_config,
#endif /* NCONFIG */
      pei_monitor
    },
#if defined (GPRS)
    2436,              /* Stacksize          */
#else
#if defined (FAX_AND_DATA)
    1436,              /* Stacksize          */
#else
    1066,              /* Stacksize          */
#endif
#endif
    10,                /* Queue entries      */
    225,               /* Priority           */
    NUM_OF_ALR_TIMERS, /* number of timers   */
                       /* flags              */
#ifdef _TARGET_
#ifdef GPRS
    PASSIVE_BODY|COPY_BY_REF|TRC_NO_SUSPEND|PRIM_NO_SUSPEND
#else
    PASSIVE_BODY|COPY_BY_REF|TRC_NO_SUSPEND|PRIM_NO_SUSPEND
#endif
#else
    PASSIVE_BODY|COPY_BY_REF
#endif

  };

  TRACE_FUNCTION ("pl_pei_create()");

  /*
   * close resources if open
   */
#ifdef _SIMULATION_
  if (first_access)
    first_access = FALSE;
  else
    pei_exit ();
#endif
  /*
   * Export startup configuration data
   */
  *info = (T_PEI_INFO *)&pei_info;

  return PEI_OK;
}

#endif
