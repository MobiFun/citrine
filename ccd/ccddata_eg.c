/* 
+----------------------------------------------------------------------------- 
|  Project : 
|  Modul   :  ccddata_eg.c
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
|  Purpose :  This module handles the entity graph functionality in ccddata.
+----------------------------------------------------------------------------- 
*/ 

#define CCDDATA_EG_C

/*==== INCLUDES ==============================================================*/
#include "typedefs.h"
#include "ccdtable.h"
#include "ccddata.h"
/*==== CONSTS ================================================================*/
#define MAXNODE 29
#define MAXSAPS 157
#define MAXCOMPAIRS 5

#define ENAME(e_index) #e_index

#define MMI    0
#define SIM    1
#define SMS    2
#define CC     3
#define SM     4
#define SS     5
#define MM     6
#define GMM    7
#define RR     8
#define GRR    9
#define DL    10
#define PL    11
#define L2R   12
#define T30   13
#define RLP   14
#define FAD   15
#define LLC   16
#define SND   17
#define PPP   18
#define UART  19
#define PKT   20
#define LC    21
#define RRLP  22
#define WAP   23
#define UDP   24
#define IP    25
#define L1    26
#define GRLC  27
#define UPM   28
/*==== TYPES =================================================================*/
/*==== LOCALS ================================================================*/
static char* sapnames[MAXSAPS] =
{
/* 0 */  "MPHC",
/* 1 */  "PH",
/* 2 */  "MPHP",
/* 3 */  "DL",
/* 4 */  "MDL",
/* 5 */  "SIM",
/* 6 */  "RR",
/* 7 */  "MMCC",
/* 8 */  "MMSS",
/* 9 */  "MMSMS",
/* 10 */  "MMREG",
/* 11 */  "MNCC",
/* 12 */  "MNSS",
/* 13 */  "MNSMS",
/* 14 */  "MMI",
/* 15 */  "MON",
/* 16 */  "RA",
/* 17 */  "RLP",
/* 18 */  "L2R",
/* 19 */  "FAD",
/* 20 */  "T30",
/* 21 */  "ACI",
/* 22 */  "CST",
/* 23 */  "MPH",
/* 24 */  "TB",
/* 25 */  "TRA",
/* 26 */  "DMI",
/* 27 */  "IDA",
/* 28 */  "DCM",
/* 29 */  "unused",
/* 30 */  "FRM",
/* 31 */  "GMMRR",
/* 32 */  "GRR",
/* 33 */  "LLGMM",
/* 34 */  "LL",
/* 35 */  "GMMSMS",
/* 36 */  "GMMSM",
/* 37 */  "unused",
/* 38 */  "SMREG",
/* 39 */  "SNSM",
/* 40 */  "SN",
/* 41 */  "GSIM, to be removed",
/* 42 */  "unused",
/* 43 */  "unused",
/* 44 */  "unused",
/* 45 */  "RRGRR",
/* 46 */  "MMGMM",
/* 47 */  "unused",
/* 48 */  "unused",
/* 49 */  "unused",
/* 50 */  "MAC",
/* 51 */  "GMMREG",
/* 52 */  "UART",
/* 53 */  "PPP",
/* 54 */  "CCI",
/* 55 */  "DTI",
/* 56 */  "PPC",
/* 57 */  /* ??? "TOM/IP" */ "IP",
/* 58 */  "BTP",
/* 59 */  "UDPA",
/* 60 */  "IPA",
/* 61 */  "WAP",
/* 62 */  "EM",
/* 63 */  "EXTDSPL/GTI",
/* 64 */  "RRLC",
/* 65 */  "RRRRLP",
/* 66 */  "RRLP",
/* 67 */  "CSRLC",
/* 68 */  "MNLC",
/* 69 */  /* "PKTIO", */ "PKT",
/* 70 */  "UDP",
/* 71 */  "AAA",
/* 72 */  "TCPIP",
/* 73 */  "unused",
/* 74 */  "unused",
/* 75 */  "unused",
/* 76 */  "unused",
/* 77 */  "unused",
/* 78 */  "unused",
/* 79 */  "unused",
/* 80 */  "customer 6379",
/* 81 */  "customer 6379",
/* 82 */  "customer 6379",
/* 83 */  "customer 6379",
/* 84 */  "customer 6379",
/* 85 */  "customer 6379",
/* 86 */  "customer 6379",
/* 87 */  "customer 6379",
/* 88 */  "customer 6379",
/* 89 */  "customer 6379",
/* 90 */  "customer 6379",
/* 91 */  "unused",
/* 92 */  "unused",
/* 93 */  "unused",
/* 94 */  "unused",
/* 95 */  "unused",
/* 96 */  "unused",
/* 97 */  "unused",
/* 98 */  "unused",
/* 99 */  "unused",
/* 100 */  "unused",
/* 101 */  "unused",
/* 102 */  "unused",
/* 103 */  "unused",
/* 104 */  "unused",
/* 105 */  "unused",
/* 106 */  "unused",
/* 107 */  "unused",
/* 108 */  "unused",
/* 109 */  "unused",
/* 110 */  "unused",
/* 111 */  "unused",
/* 112 */  "unused",
/* 113 */  "unused",
/* 114 */  "unused",
/* 115 */  "unused",
/* 116 */  "unused",
/* 117 */  "unused",
/* 118 */  "unused",
/* 119 */  "unused",
/* 120 */  "unused",
/* 121 */  "unused",
/* 122 */  "unused",
/* 123 */  "unused",
/* 124 */  "unused",
/* 125 */  "unused",
/* 126 */  "unused",
/* 127 */  "unused",
/* 128 */  "CPHY",
/* 129 */  "PHY",
/* 130 */  "CUMAC",
/* 131 */  "UMAC",
/* 132 */  "CRLC",
/* 133 */  "RLC",
/* 134 */  "CBM",
/* 135 */  "HC",
/* 136 */  "CPDCP",
/* 137 */  "PDCP",
/* 138 */  "RRC",
/* 139 */  "MEM",
/* 140 */  "RRRRC",
/* 141 */  "GRRRRC",
/* 142 */  "RCM",
/* 143 */  "GMMRABM",
/* 144 */  "SM",
/* 145 */  "PMMSMS",
/* 146 */  "PHYSTUB",
/* 147 */  "PHYTEST",
/* 148 */  "MMREG",
/* 149 */  "MMCM",
/* 150 */  "MMPM",
/* 151 */  "GRLC",
/* 152 */  "CGRLC",
/* 153 */  "EINFO",
/* 154 */  "SL2",
/* 155 */  "L1TEST",
/* 156 */  "CL"
};

static T_COMENDPOINTS com_endpoints[MAXSAPS][MAXCOMPAIRS] =
{
/* 0 (MPHC) */  { {PL, L1}, {RR, L1}, {-1, -1} },
/* 1 (PH) */    { {DL, L1}, {-1, -1} },
/* 2 (MPHP) */  { {GRR, L1}, {RR, L1}, {-1, -1} },
/* 3 (DL) */    { {RR, DL}, {-1, -1} },
/* 4 (MDL) */  { {MM, DL}, {-1, -1} },
/* 5 (SIM) */  { {MMI, SIM}, {MM, SIM}, {GMM, SIM}, {SMS, SIM}, {-1, -1} },
/* 6 (RR) */  { {MM, RR}, {-1, -1} },
/* 7 (MMCC) */  { {CC, MM}, {-1, -1} },
/* 8 (MMSS) */  { {SS, MM}, {-1, -1} },
/* 9 (MMSMS) */  { {SMS, MM}, {-1, -1} },
/* 10 (MMREG) */  { {MMI, MM}, {-1, -1} },
/* 11 (MNCC) */  { {MMI, CC}, {-1, -1} },
/* 12 (MNSS) */  { {MMI, SS}, {-1, -1} },
/* 13 (MNSMS) */  { {MMI, SMS}, {-1, -1} },
/* 14 (MMI) */  { /* {???, ???}, */ {-1, -1} },
/* 15 (MON) */  { /* {???, ???}, */ {-1, -1} },
/* 16 (RA) */  { {RLP, L1}, {FAD, L1}, {-1, -1} },
/* 17 (RLP) */  { {L2R, RLP}, {-1, -1} },
/* 18 (L2R) */  { {MMI, L2R}, {-1, -1} },
/* 19 (FAD) */  { {T30, FAD}, {-1, -1} },
/* 20 (T30) */  { {MMI, T30}, {-1, -1} },
/* 21 (ACI) */  { /* {???, ???}, */ {-1, -1} },
/* 22 (CST) */  { /* {???, ???}, */ {-1, -1} },
/* 23 (MPH) */  { {RR, PL}, {-1, -1} },
/* 24 (TB) */  { {GRR, PL}, {-1, -1} },
/* 25 (TRA) */  { /* {???, ???}, */ {-1, -1} },
/* 26 (DMI) */  { /* {???, ???}, */ {-1, -1} },
/* 27 (IDA) */  { /* {???, ???}, */ {-1, -1} },
/* 28 (DCM) */  { /* {???, ???}, */ {-1, -1} },
/* 29 (unused) */  { {-1, -1} },
/* 30 (FRM) */  { /* {???, ???}, */ {-1, -1} },
/* 31 (GMMRR) */  { {GMM, GRR}, {-1, -1} },
/* 32 (GRR) */  { {LLC, GRR}, {-1, -1} },
/* 33 (LLGMM) */  { {GMM, LLC}, {MM, LLC}, {-1, -1} },
/* 34 (LL) */  { {GMM, LLC}, {SMS, LLC}, {SND, LLC}, {MM, LLC}, {-1, -1} },
/* 35 (GMMSMS) */  { {SMS, GMM}, {-1, -1} },
/* 36 (GMMSM) */  { {SM, GMM}, {-1, -1} },
/* 37 (unused) */  { {-1, -1} },
/* 38 (SMREG) */  { {MMI, SM}, {-1, -1} },
/* 39 (SNSM) */  { {SND, SM}, {-1, -1} },
/* 40 (SN) */  { {MMI, SND}, {-1, -1} },
/* 41 (GSIM, to be removed) */  { /* {???, ???}, */ {-1, -1} },
/* 42 (unused) */  { {-1, -1} },
/* 43 (unused) */  { {-1, -1} },
/* 44 (unused) */  { {-1, -1} },
/* 45 (RRGRR) */  { {RR, GRR}, {-1, -1} },
/* 46 (MMGMM) */  { {MM, GMM}, {-1, -1} },
/* 47 (unused) */  { {-1, -1} },
/* 48 (unused) */  { {-1, -1} },
/* 49 (unused) */  { {-1, -1} },
/* 50 (MAC) */  { /* {???, ???}, */ {-1, -1} },
/* 51 (GMMREG) */  { {MMI, GMM}, {-1, -1} },
/* 52 (UART) */  { {MMI, UART}, {-1, -1} },
/* 53 (PPP) */  { {MMI, PPP}, {-1, -1} },
/* 54 (CCI) */  { /* {???, ???}, */ {-1, -1} },
/* 55 (DTI) */  { /* {???, ???}, */ {-1, -1} },
/* 56 (PPC) */  { /* {???, ???}, */ {-1, -1} },
/* 57 (TOM/IP) */  { {UDP, IP}, {-1, -1} },
/* 58 (BTP) */  { /* {???, ???}, */ {-1, -1} },
/* 59 (UDPA) */  { {MMI, UDP}, {-1, -1} },
/* 60 (IPA) */  { {MMI, IP}, {-1, -1} },
/* 61 (WAP) */  { {MMI, WAP}, {-1, -1} },
/* 62 (EM) */  { /* {???, ???}, */ {-1, -1} },
/* 63 (EXTDSPL/GTI) */  { /* {???, ???}, */ {-1, -1} },
/* 64 (RRLC) */  { {LC, RR}, {-1, -1} },
/* 65 (RRRRLP) */  { {RRLP, RR}, {-1, -1} },
/* 66 (RRLP) */  { /* {???, ???}, */ {-1, -1} },
/* 67 (CSRLC) */  { /* {???, ???}, */ {-1, -1} },
/* 68 (MNLC) */  { {MM, LC}, {-1, -1} },
/* 69 (PKTIO) */  { {MMI, PKT}, {-1, -1} },
/* 70 (UDP) */  { {WAP, UDP}, {SIM, UDP}, {-1, -1} },
/* 71 (AAA) */  { /* {???, ???}, */ {-1, -1} },
/* 72 (TCPIP) */  { /* {???, ???}, */ {-1, -1} },
/* 73 (unused) */  { {-1, -1} },
/* 74 (unused) */  { {-1, -1} },
/* 75 (unused) */  { {-1, -1} },
/* 76 (unused) */  { {-1, -1} },
/* 77 (unused) */  { {-1, -1} },
/* 78 (unused) */  { {-1, -1} },
/* 79 (unused) */  { {-1, -1} },
/* 80 (customer 6379) */  { /* {???, ???}, */ {-1, -1} },
/* 81 (customer 6379) */  { /* {???, ???}, */ {-1, -1} },
/* 82 (customer 6379) */  { /* {???, ???}, */ {-1, -1} },
/* 83 (customer 6379) */  { /* {???, ???}, */ {-1, -1} },
/* 84 (customer 6379) */  { /* {???, ???}, */ {-1, -1} },
/* 85 (customer 6379) */  { /* {???, ???}, */ {-1, -1} },
/* 86 (customer 6379) */  { /* {???, ???}, */ {-1, -1} },
/* 87 (customer 6379) */  { /* {???, ???}, */ {-1, -1} },
/* 88 (customer 6379) */  { /* {???, ???}, */ {-1, -1} },
/* 89 (customer 6379) */  { /* {???, ???}, */ {-1, -1} },
/* 90 (customer 6379) */  { /* {???, ???}, */ {-1, -1} },
/* 91 (unused) */  { {-1, -1} },
/* 92 (unused) */  { {-1, -1} },
/* 93 (unused) */  { {-1, -1} },
/* 94 (unused) */  { {-1, -1} },
/* 95 (unused) */  { {-1, -1} },
/* 96 (unused) */  { {-1, -1} },
/* 97 (unused) */  { {-1, -1} },
/* 98 (unused) */  { {-1, -1} },
/* 99 (unused) */  { {-1, -1} },
/* 100 (unused) */  { {-1, -1} },
/* 101 (unused) */  { {-1, -1} },
/* 102 (unused) */  { {-1, -1} },
/* 103 (unused) */  { {-1, -1} },
/* 104 (unused) */  { {-1, -1} },
/* 105 (unused) */  { {-1, -1} },
/* 106 (unused) */  { {-1, -1} },
/* 107 (unused) */  { {-1, -1} },
/* 108 (unused) */  { {-1, -1} },
/* 109 (unused) */  { {-1, -1} },
/* 110 (unused) */  { {-1, -1} },
/* 111 (unused) */  { {-1, -1} },
/* 112 (unused) */  { {-1, -1} },
/* 113 (unused) */  { {-1, -1} },
/* 114 (unused) */  { {-1, -1} },
/* 115 (unused) */  { {-1, -1} },
/* 116 (unused) */  { {-1, -1} },
/* 117 (unused) */  { {-1, -1} },
/* 118 (unused) */  { {-1, -1} },
/* 119 (unused) */  { {-1, -1} },
/* 120 (unused) */  { {-1, -1} },
/* 121 (unused) */  { {-1, -1} },
/* 122 (unused) */  { {-1, -1} },
/* 123 (unused) */  { {-1, -1} },
/* 124 (unused) */  { {-1, -1} },
/* 125 (unused) */  { {-1, -1} },
/* 126 (unused) */  { {-1, -1} },
/* 127 (unused) */  { {-1, -1} },
/* 128 (CPHY) */  { /* {???, ???}, */ {-1, -1} },
/* 129 (PHY) */  { /* {???, ???}, */ {-1, -1} },
/* 130 (CUMAC) */  { /* {???, ???}, */ {-1, -1} },
/* 131 (UMAC) */  { /* {???, ???}, */ {-1, -1} },
/* 132 (CRLC) */  { /* {???, ???}, */ {-1, -1} },
/* 133 (RLC) */  { /* {???, ???}, */ {-1, -1} },
/* 134 (CBM) */  { /* {???, ???}, */ {-1, -1} },
/* 135 (HC) */  { /* {???, ???}, */ {-1, -1} },
/* 136 (CPDCP) */  { /* {???, ???}, */ {-1, -1} },
/* 137 (PDCP) */  { /* {???, ???}, */ {-1, -1} },
/* 138 (RRC) */  { /* {???, ???}, */ {-1, -1} },
/* 139 (MEM) */  { /* {???, ???}, */ {-1, -1} },
/* 140 (RRRRC) */  { /* {???, ???}, */ {-1, -1} },
/* 141 (GRRRRC) */  { /* {???, ???}, */ {-1, -1} },
/* 142 (RCM) */  { /* {???, ???}, */ {-1, -1} },
/* 143 (GMMRABM) */  { /* {???, ???}, */ {-1, -1} },
/* 144 (SM) */  { /* {???, ???}, */ {-1, -1} },
/* 145 (PMMSMS) */  { /* {???, ???}, */ {-1, -1} },
/* 146 (PHYSTUB) */  { /* {???, ???}, */ {-1, -1} },
/* 147 (PHYTEST) */  { /* {???, ???}, */ {-1, -1} },
/* 148 (MMREG) */  { /* {???, ???}, */ {-1, -1} },
/* 149 (MMCM) */  { /* {???, ???}, */ {-1, -1} },
/* 150 (MMPM) */  { {MM, SM},  {-1, -1} },
/* 151 (GRLC) */  { {LLC, GRLC}, {-1, -1} },
/* 152 (CGRLC) */ { {GMM, GRLC}, {GRR, GRLC}, {MM, GRLC}, {RR, GRLC}, {-1, -1}},
/* 153 (EINFO) */  { /* {???, ???}, */ {-1, -1} },
/* 154 (SL2) */  { /* {???, ???}, */ {-1, -1} },
/* 155 (L1TEST) */  { /* {???, ???}, */ {-1, -1} },
/* 156 (CL) */  { /* {???, ???}, */ {-1, -1} }
};

static char* node[MAXNODE] =
{
   ENAME(MMI),
   ENAME(SIM),
   ENAME(SMS),
   ENAME(CC),
   ENAME(SM),
   ENAME(SS),
   ENAME(MM),
   ENAME(GMM),
   ENAME(RR),
   ENAME(GRR),
   ENAME(DL),
   ENAME(PL),
   ENAME(L2R),
   ENAME(T30),
   ENAME(RLP),
   ENAME(FAD),
   ENAME(LLC),
   ENAME(SND),
   ENAME(PPP),
   ENAME(UART),
   ENAME(PKT),
   ENAME(LC),
   ENAME(RRLP),
   ENAME(WAP),
   ENAME(UDP),
   ENAME(IP),
   ENAME(L1),
   ENAME(GRLC),
   ENAME(UPM)
};

/* adjacence matrix. 1 = directly connected, 0 = not
   here symmtric = undirected graph,
   could later  be changed to a directed graph */

static char ad[MAXNODE][MAXNODE] =
{        /* M S S C S S M G R G D P L T R F L S P U P L R W U I L G U
            M I M C M S M M R R L L 2 3 L A L N P A K C R A D P 1 R P
            I M S         M   R     R 0 P D C D P R T   L P P     L M
                                                  T     P         C
                                                                      */
/* MMI  */  1,1,1,1,1,1,1,1,0,0,0,1,1,1,0,0,0,1,1,1,1,0,0,1,1,1,1,0,1,
/* SIM  */  1,1,1,0,0,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,
/* SMS  */  1,1,1,0,0,0,1,1,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,
/* CC   */  1,0,0,1,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
/* SM   */  1,0,0,0,1,0,1,1,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,1,
/* SS   */  1,0,0,0,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
/* MM   */  1,1,1,1,1,1,1,1,1,0,1,0,0,0,0,0,1,0,0,0,0,1,0,0,0,0,0,1,0,
/* GMM  */  1,1,1,0,1,0,1,1,0,1,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,1,1,
/* RR   */  0,0,0,0,0,0,1,0,1,1,1,1,0,0,0,0,0,0,0,0,0,1,1,0,0,0,1,1,0,
/* GRR  */  0,0,0,0,0,0,0,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,0,
/* DL   */  0,0,0,0,0,0,1,0,1,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,
/* PL   */  1,0,0,0,0,0,0,0,1,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,
/* L2R  */  1,0,0,0,0,0,0,0,0,0,0,0,1,0,1,0,0,0,1,1,0,0,0,0,0,0,0,0,0,
/* T30  */  1,0,0,0,0,0,0,0,0,0,0,0,0,1,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,
/* RLP  */  0,0,0,0,0,0,0,0,0,0,0,0,1,0,1,0,0,0,0,0,0,0,0,0,0,0,1,0,0,
/* FAD  */  0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,1,0,0,0,0,0,0,0,0,0,0,1,0,0,
/* LLC  */  0,0,1,0,0,0,1,1,0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,0,1,0,
/* SND  */  1,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,1,1,1,0,1,0,0,0,0,0,0,0,1,
/* PPP  */  1,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,1,1,1,0,0,0,0,0,1,0,0,0,
/* UART */  1,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,0,
/* PKT  */  1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,1,0,0,0,0,0,0,0,0,
/* LC   */  0,0,0,0,0,0,1,0,1,0,0,0,0,0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,
/* RRLP */  0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,
/* WAP  */  1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,0,0,0,0,
/* UDP  */  1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,0,0,0,
/* IP   */  1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,1,1,0,0,0,
/* L1   */  1,0,0,0,0,0,0,0,1,1,1,1,0,0,1,1,0,0,0,0,0,0,0,0,0,0,1,0,0,
/* GRLC */  0,0,0,0,0,0,1,1,1,1,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,1,0,
/* UPM  */  1,0,0,0,1,0,0,1,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,1,
};

/*==== PRIVATE FUNCTIONS =====================================================*/
/*==== PUBLIC FUNCTIONS ======================================================*/

/*
+------------------------------------------------------------------------------
|  Function     :  ccddata_eg_nodes
+------------------------------------------------------------------------------
|  Description  :  Returns the number of nodes in the entity graph.
|
|  Parameters   :  -
|
|  Return       :  The number of nodes.
|
+------------------------------------------------------------------------------
*/

int ccddata_eg_nodes (void)
{
    return MAXNODE;
}

/*
+------------------------------------------------------------------------------
|  Function     :  ccddata_eg_nodenames
+------------------------------------------------------------------------------
|  Description  :  Returns a pointer to the node name table.
|
|  Parameters   :  -
|
|  Return       :  The address of the nodename table.
|
+------------------------------------------------------------------------------
*/
char** ccddata_eg_nodenames (void)
{
  return node;
}

/*
+------------------------------------------------------------------------------
|  Function     :  ccddata_eg_adjacent
+------------------------------------------------------------------------------
|  Description  :  Returns a pointer to one row in the adjacence matrix.
|
|  Parameters   :  idx - line in matrix (0..nodes-1).
|
|  Return       :  The address of the selected row.
|
+------------------------------------------------------------------------------
*/
char* ccddata_eg_adjacent (int idx)
{
   return ad[idx];
}

/*
+------------------------------------------------------------------------------
|  Function     :  ccddata_eg_saps
+------------------------------------------------------------------------------
|  Description  :  Returns the number of SAPs (including gaps).
|
|  Parameters   :  -
|
|  Return       :  The number of SAPs.
|
+------------------------------------------------------------------------------
*/

int ccddata_eg_saps (void)
{
    return MAXSAPS;
}

/*
+------------------------------------------------------------------------------
|  Function     :  ccddata_eg_sapnames
+------------------------------------------------------------------------------
|  Description  :  Returns a pointer to the SAP name table.
|
|  Parameters   :  -
|
|  Return       :  The address of the sapname table.
|
+------------------------------------------------------------------------------
*/
char** ccddata_eg_sapnames (void)
{
  return sapnames;
}

/*
+------------------------------------------------------------------------------
|  Function     :  ccddata_eg_comendpoints
+------------------------------------------------------------------------------
|  Description  :  Returns a pointer to one row in the comendpoint list.
|
|  Parameters   :  idx - line in list (0..SAPs-1).
|
|  Return       :  The address of the selected row.
|
+------------------------------------------------------------------------------
*/
T_COMENDPOINTS* ccddata_eg_comendpoints (int idx)
{
   return com_endpoints[idx];
}
