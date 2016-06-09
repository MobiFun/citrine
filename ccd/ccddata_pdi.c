/* 
+----------------------------------------------------------------------------- 
|  Project :  
|  Modul   : ccddata_pdi.c
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
|  Purpose :  contains table with information to decode airmessages 
|             depending on the carrier-primitive
+----------------------------------------------------------------------------- 
*/ 

#include "pdi.h"
#include "ccdtable.h"
#include "mconst.cdg"
#include "pconst.cdg"

/* declaration of special ccdmsg preparation handlers */
int invoke_ccd_setStore(T_PDI_CCDMSG* ccdmsg, ULONG values[], int len);
int mphX_checker(T_PDI_CCDMSG* ccdmsg, ULONG values[], int len);
int mphp_sing_block_req_checker(T_PDI_CCDMSG* ccdmsg, ULONG values[], int len);
int mphp_sing_block_con_checker(T_PDI_CCDMSG* ccdmsg, ULONG values[], int len);
int mphp_sing_block_con_edge_checker(T_PDI_CCDMSG* ccdmsg, ULONG values[], int len);
int mphp_polling_resp_checker(T_PDI_CCDMSG* ccdmsg, ULONG values[], int len);
int mac_data_checker(T_PDI_CCDMSG* ccdmsg, ULONG values[], int len);
int mac_poll_checker(T_PDI_CCDMSG* ccdmsg, ULONG values[], int len);
int rlc_data_0_checker(T_PDI_CCDMSG* ccdmsg, ULONG values[], int len);
int rlc_data_1_checker(T_PDI_CCDMSG* ccdmsg, ULONG values[], int len);

/* declaration of primitive members to be collected */
static char* ccd_loadstore_elements[]=
{
  "loadstore_val0",
  "loadstore_val1",
  NULL
};

static char* mphX_constrainer[]=
{
  "error_flag",
  NULL
};

static char* mphp_sing_block_constrainer[]=
{
  "purpose",
  NULL
};

static char* mphp_sing_block_con_edge_constrainer[]=
{
  "purpose",
  "sb_status",
  "dl_error",
  NULL
};

static char* mphp_polling_resp_constrainer[] =
{
  "poll_resp_type",
  NULL
};

static char* mac_data_constrainer[] =
{
  "block_status",
  NULL
};

static char* rlc_data_constrainer[] =
{
  "rlc_id",
  NULL
};


/* decode info filter */
static T_PDI_DECODEINFO m_dinfo[]={
  /* type */  /* attrib */ /* prim */ /* entity */ /* mt */ /* pdi_prepare_ccdmsg */ /* primmbr */
  {PDI_DECODETYPE_L3PDU_N,   "sdu", "PH_*", "", 0xff, NULL, NULL},
  {PDI_DECODETYPE_L3PDU_N,   "sdu", "MPH_*", "", 0xff, NULL, NULL},
  {PDI_DECODETYPE_RR_SHORT,   "sdu", "DL_SHORT*", "RR_SHORT", 0xff, NULL, NULL},
#ifdef DL_DATA_REQ
#if (DL_DATA_REQ & 0x4000)
  /* Toggle direction for old-style DL_* primtives */
  {PDI_DECODETYPE_L3PDU_N,   "sdu", "DL_*", "", 0xff, NULL, NULL},
#endif
#endif

  {PDI_DECODETYPE_NOPD_N, "sdu", "XX_CCD_ASN1_DECODE_REQ", "ASN1_MSG", 0xff, NULL, NULL},  
  {PDI_DECODETYPE_NOPD, "sdu", "XX_CCD_ASN1_CODE_IND", "ASN1_MSG", 0xff, NULL, NULL},  
  {PDI_DECODETYPE_NOPD_N, "sdu", "XX_DYNARR_DECODE_REQ", "ASN1_MSG", 0xff, NULL, NULL},  
  {PDI_DECODETYPE_NOPD, "sdu", "XX_DYNARR_CODE_IND", "ASN1_MSG", 0xff, NULL, NULL},  
  {PDI_DECODETYPE_MAC_H_CHECK,   "dl_block", "MAC_DATA_IND", "GRR", 0xff,
                                  mac_data_checker, mac_data_constrainer},
  {PDI_DECODETYPE_MAC_H_CHECK,   "dl_block", "MAC_DATA_IND_EGPRS", "GRR", 0xff,
                                  mac_data_checker, mac_data_constrainer},
  {PDI_DECODETYPE_MAC_H_CHECK,   "ul_block", "MAC_EGPRS_DATA_REQ", "GRR", 0xff,
                                  mac_data_checker, mac_data_constrainer},
  {PDI_DECODETYPE_MAC_H_CHECK,   "ul_block", "MAC_DATA_REQ", "GRR", 0xff,
                                  mac_data_checker, mac_data_constrainer},
  {PDI_DECODETYPE_MAC_H_CHECK,   "ul_block", "MAC_POLL_REQ", "GRR", 0xff,
                                  mac_poll_checker, mac_data_constrainer},
  {PDI_DECODETYPE_NOPD,    "sdu", "RRGRR_GPRS_SI13_IND", "RR", 0xff, NULL, NULL},
  {PDI_DECODETYPE_NOPD,    "sdu", "RRGRR_IA_IND", "RR", 0xff, NULL, NULL},
  {PDI_DECODETYPE_NOPD,    "sdu", "RRGRR_DATA_IND", "RR", 0xff, NULL, NULL},
  {PDI_DECODETYPE_NOPD,    "sdu", "RRGRR_IA_DOWNLINK_IND", "RR", 0xff, NULL, NULL},
  {PDI_DECODETYPE_NOPD_N,  "sdu", "RRGRR_SI_STATUS_IND", "GRR", 0xff, NULL, NULL},
  {PDI_DECODETYPE_NOPD_N,  "sdu", "TB_NCELL_SI_DATA_IND", "RR", 0xff, NULL, NULL},
  {PDI_DECODETYPE_NOPD,    "sdu", "RRGRR_IAEXT_IND", "RR", 0xff, NULL, NULL},
  {PDI_DECODETYPE_NOPD,    "sdu", "RRGRR_DATA_REQ", "RR", 0xff, NULL, NULL},
  {PDI_DECODETYPE_NOPD_N,  "sdu", "TB_SCELL_SI_DATA_IND", "RR", 0xff, NULL, NULL},
#ifdef MPHP_DATA_IND
/* toggle direction for old 16bit MPHP* */
#if (MPHP_DATA_IND & 0x80000000)
  {PDI_DECODETYPE_MAC_H_CHECK,   "frame_array", "MPHP_DATA_IND", "GRR", 0xff,
                                 mphX_checker, mphX_constrainer},
  {PDI_DECODETYPE_MAC_H_CHECK,   "data_array", "MPHP_SINGLE_BLOCK_CON", "GRR",
                           0xff, NULL, NULL},
  {PDI_DECODETYPE_MAC_H_CHECK,   "data_array", "MPHP_SINGLE_BLOCK_REQ", "GRR",
                           0xff, mphp_sing_block_req_checker,
                           mphp_sing_block_constrainer},
#else
  {PDI_DECODETYPE_MAC_H_N_CHECK,   "l2_frame", "MPHP_DATA_IND", "GRR", 0xff,
                                 mphX_checker, mphX_constrainer},
  {PDI_DECODETYPE_MAC_H_N_CHECK,   "l2_frame", "MPHP_SINGLE_BLOCK_CON", "GRR",
                           0xff, mphp_sing_block_con_edge_checker,
                           mphp_sing_block_con_edge_constrainer},
  {PDI_DECODETYPE_MAC_H_CHECK,   "l2_frame", "MPHP_SINGLE_BLOCK_REQ", "GRR",
                           0xff, mphp_sing_block_req_checker,
                           mphp_sing_block_constrainer},
#endif
  {PDI_DECODETYPE_MAC_H_CHECK,   "poll_data", "MPHP_POLLING_RESPONSE_REQ",
                          "GRR", 0xff, mphp_polling_resp_checker,
                           mphp_polling_resp_constrainer},
#endif
#ifdef MPHC_DATA_IND
/* toggle direction for old 16bit MPHC* */
#if (MPHC_DATA_IND & 0x80000000)
  {PDI_DECODETYPE_AIM_CHECK,   "l2_frame", "MPHC_NCELL_BCCH_IND", "RR", 0xff,
                                 mphX_checker, mphX_constrainer},
  {PDI_DECODETYPE_AIM_CHECK,   "l2_frame", "MPHC_DATA_IND", "RR", 0xff,
                                 mphX_checker, mphX_constrainer},
#else
  {PDI_DECODETYPE_AIM_N_CHECK,   "l2_frame", "MPHC_NCELL_BCCH_IND", "RR", 0xff,
                                 mphX_checker, mphX_constrainer},
  {PDI_DECODETYPE_AIM_N_CHECK,   "l2_frame", "MPHC_DATA_IND", "RR", 0xff,
                                 mphX_checker, mphX_constrainer},
#endif
#endif
  {PDI_DECODETYPE_MAC_H,   "data_array", "CGRLC_DATA_*", "GRR", 0xff, NULL, NULL},

#ifdef CCDENT_UMTS_AS_ASN1_MSG

// real sdu in test primitives 
  //  UMTS_AS_ASN1_UL_DCCH_MSG_MSG
  {PDI_DECODETYPE_NOPD_NOTYPE, "sdu", "RLC_TEST_SDU_UM_DATA_REQ", "UMTS_AS_ASN1_MSG", UMTS_AS_ASN1_UL_DCCH_MSG_MSG, rlc_data_1_checker, rlc_data_constrainer},  
  {PDI_DECODETYPE_NOPD_NOTYPE, "sdu", "RLC_UM_DATA_REQ", "UMTS_AS_ASN1_MSG", UMTS_AS_ASN1_UL_DCCH_MSG_MSG, rlc_data_1_checker, rlc_data_constrainer}, //rlc_id=1
  {PDI_DECODETYPE_NOPD_NOTYPE, "sdu", "RLC_UM_DATA_IND", "UMTS_AS_ASN1_MSG", UMTS_AS_ASN1_DL_DCCH_MSG_MSG, NULL, NULL},
  {PDI_DECODETYPE_NOPD_NOTYPE, "sdu", "RLC_TEST_SDU_AM_DATA_REQ", "UMTS_AS_ASN1_MSG", UMTS_AS_ASN1_UL_DCCH_MSG_MSG, NULL, NULL},  
  {PDI_DECODETYPE_NOPD_NOTYPE, "sdu", "RLC_TEST_SDU_AM_DATA_IND", "UMTS_AS_ASN1_MSG", UMTS_AS_ASN1_DL_DCCH_MSG_MSG, NULL, NULL},  
  {PDI_DECODETYPE_NOPD_NOTYPE, "sdu", "RLC_AM_DATA_REQ", "UMTS_AS_ASN1_MSG", UMTS_AS_ASN1_UL_DCCH_MSG_MSG, NULL, NULL},  
  {PDI_DECODETYPE_NOPD_NOTYPE, "sdu", "RLC_AM_DATA_IND", "UMTS_AS_ASN1_MSG", UMTS_AS_ASN1_DL_DCCH_MSG_MSG, NULL, NULL},  
  {PDI_DECODETYPE_NOPD_NOTYPE, "sdu", "RLC_TEST_SDU_UM_DATA_REQ", "UMTS_AS_ASN1_MSG", UMTS_AS_ASN1_UL_CCCH_MSG_MSG, rlc_data_0_checker, rlc_data_constrainer},
  {PDI_DECODETYPE_NOPD_NOTYPE, "sdu", "RLC_UM_DATA_REQ", "UMTS_AS_ASN1_MSG", UMTS_AS_ASN1_UL_CCCH_MSG_MSG, rlc_data_0_checker, rlc_data_constrainer}, //rlc_id=0 
  {PDI_DECODETYPE_NOPD_NOTYPE, "sdu", "RLC_TEST_SDU_TR_DATA_REQ", "UMTS_AS_ASN1_MSG", UMTS_AS_ASN1_UL_CCCH_MSG_MSG, NULL, NULL},  
  {PDI_DECODETYPE_NOPD_NOTYPE, "sdu", "RLC_TR_DATA_REQ", "UMTS_AS_ASN1_MSG", UMTS_AS_ASN1_UL_CCCH_MSG_MSG, NULL, NULL},
  {PDI_DECODETYPE_NOPD_NOTYPE, "sdu", "RLC_TR_DATA_IND", "UMTS_AS_ASN1_MSG", UMTS_AS_ASN1_DL_CCCH_MSG_MSG, NULL, NULL},
  //RLC_AM_RETRANSMIT_CNF
  //RLC_CTCH_DATA_IND

// only as mem handles, not suported yet
  //  UMTS_AS_ASN1_DL_CCCH_MSG_MSG
  //  UMTS_AS_ASN1_BCCH_BCH_MSG_MSG
  //  UMTS_AS_ASN1_DL_DCCH_MSG_MSG     
  //  UMTS_AS_ASN1_DL_SHCCH_MSG_MSG     
  //  UMTS_AS_ASN1_UL_SHCCH_MSG_MSG     
  //  UMTS_AS_ASN1_PCCH_MSG_MSG     
  //  UMTS_AS_ASN1_HO_TO_UTRANCOMMAND_MSG
  //  UMTS_AS_ASN1_MASTER_INFO_BLOCK_MSG 
  //  UMTS_AS_ASN1_BCCH_FACH_MSG_MSG     
  //  UMTS_AS_ASN1_PDSCH_SYS_INFO_LIST_MSG
  //  UMTS_AS_ASN1_PUSCH_SYS_INFO_LIST_MSG

/* the following make little sence to decode since they always 
  comes cutted up in small peaces inside a UMTS_AS_ASN1_BCCH_BCH_MSG_MSG */
  //  UMTS_AS_ASN1_SYS_INFO_TYPE_1_MSG    
  //  UMTS_AS_ASN1_SYS_INFO_TYPE_2_MSG    
  //  UMTS_AS_ASN1_SYS_INFO_TYPE_3_MSG    
  //  UMTS_AS_ASN1_SYS_INFO_TYPE_4_MSG    
  //  UMTS_AS_ASN1_SYS_INFO_TYPE_5_MSG    
  //  UMTS_AS_ASN1_SYS_INFO_TYPE_6_MSG    
  //  UMTS_AS_ASN1_SYS_INFO_TYPE_7_MSG    
  //  UMTS_AS_ASN1_SYS_INFO_TYPE_8_MSG    
  //  UMTS_AS_ASN1_SYS_INFO_TYPE_9_MSG    
  //  UMTS_AS_ASN1_SYS_INFO_TYPE_10_MSG   
  //  UMTS_AS_ASN1_SYS_INFO_TYPE_11_MSG   
  //  UMTS_AS_ASN1_SYS_INFO_TYPE_12_MSG   
  //  UMTS_AS_ASN1_SYS_INFO_TYPE_13_MSG   
  //  UMTS_AS_ASN1_SYS_INFO_TYPE_13_1_MSG
  //  UMTS_AS_ASN1_SYS_INFO_TYPE_13_2_MSG
  //  UMTS_AS_ASN1_SYS_INFO_TYPE_13_3_MSG
  //  UMTS_AS_ASN1_SYS_INFO_TYPE_13_4_MSG
  //  UMTS_AS_ASN1_SYS_INFO_TYPE_14_MSG   
  //  UMTS_AS_ASN1_SYS_INFO_TYPE_15_MSG   
  //  UMTS_AS_ASN1_SYS_INFO_TYPE_15_1_MSG
  //  UMTS_AS_ASN1_SYS_INFO_TYPE_15_2_MSG
  //  UMTS_AS_ASN1_SYS_INFO_TYPE_15_3_MSG
  //  UMTS_AS_ASN1_SYS_INFO_TYPE_15_4_MSG
  //  UMTS_AS_ASN1_SYS_INFO_TYPE_16_MSG  
  //  UMTS_AS_ASN1_SYS_INFO_TYPE_17_MSG  
  //  UMTS_AS_ASN1_SYS_INFO_TYPE_18_MSG  
  //  UMTS_AS_ASN1_SYS_INFO_TYPE_SB_1_MSG
  //  UMTS_AS_ASN1_SYS_INFO_TYPE_SB_2_MSG
#endif /* CCDENT_UMTS_AS_ASN1_MSG */

  /* test stack primitives */
  {PDI_DECODETYPE_L3PDU_N,   "sdu", "XX_TAP*", "", 0xff, NULL, NULL},
  {PDI_DECODETYPE_NOPD_N,    "sdu", "XX_LOADSTORE_*", "XX_CSN1", 0xff, invoke_ccd_setStore, ccd_loadstore_elements},
  {PDI_DECODETYPE_NOPD_N,    "sdu", "XX_CCD_2*", "XX_CSN1", 0xff, NULL, NULL},
  {PDI_DECODETYPE_NOPD_N,    "sdu", "XX_*", "XX", 0xff, NULL, NULL},

  /* all other primitives */
  {PDI_DECODETYPE_L3PDU,     "sdu", "*", "", 0xff, NULL, NULL}
};
#define DINFO_COUNT (sizeof(m_dinfo) / sizeof(*m_dinfo))

int ccddata_get_pdi_dinfo (const T_PDI_DECODEINFO* (*dinfo) )
{
  *dinfo=m_dinfo;
  return DINFO_COUNT;
}

/* definition of special ccdmsg preparation handlers */
int invoke_ccd_setStore(T_PDI_CCDMSG* ccdmsg, ULONG values[], int len)
{
  int q;
  for (q=0; q<len; q++)
  {
    ccd_setStore(q,values[q]);
  }

  return PDI_CCDMSG;
}

int mphX_checker(T_PDI_CCDMSG* ccdmsg, ULONG values[], int len)
{
  if (len >= 1 && values[0] == 0) //error_flag
  {
    return PDI_CCDMSG;
  }
  else
  {
    return PDI_NONE;
  }
}

int mphp_sing_block_req_checker(T_PDI_CCDMSG* ccdmsg, ULONG values[], int len)
{
  if (len >= 1 && (values[0] == 4 || values[0] == 5)) //purpose
  {
    return PDI_CCDMSG;
  }
  else
  {
    return PDI_NONE;
  }
}

int mphp_sing_block_con_checker(T_PDI_CCDMSG* ccdmsg, ULONG values[], int len)
{
  if (len >= 1 && values[0] == 3) //purpose
  {
    return PDI_CCDMSG;
  }
  else
  {
    return PDI_NONE;
  }
}

int mphp_polling_resp_checker(T_PDI_CCDMSG* ccdmsg, ULONG values[], int len)
{
  if (len >= 1 && values[0] == 3) //poll_resp_type
  {
    return PDI_CCDMSG;
  }
  else
  {
    return PDI_NONE;
  }
}

int mphp_sing_block_con_edge_checker(T_PDI_CCDMSG* ccdmsg, ULONG values[], int len)
{
  if (len >= 3 && values[0] == 3 //purpose
      && values[1] == 3          //sb_status
      && values[2] == 0)         //dl_error
  {
    return PDI_CCDMSG;
  }
  else
  {
    return PDI_NONE;
  }
}

int mac_data_checker(T_PDI_CCDMSG* ccdmsg, ULONG values[], int len)
{
  if (len >= 1 && values[0] == 2) //block_status
  {
    return PDI_CCDMSG;
  }
  else
  {
    return PDI_NONE;
  }
}

int mac_poll_checker(T_PDI_CCDMSG* ccdmsg, ULONG values[], int len)
{
  if (len >= 1 && values[0] == 3) //block_status
  {
    return PDI_CCDMSG;
  }
  else
  {
    return PDI_NONE;
  }
}

int rlc_data_0_checker(T_PDI_CCDMSG* ccdmsg, ULONG values[], int len)
{
  if (len >= 1 && values[0] == 0) //rlc_id
  {
    return PDI_CCDMSG;
  }
  else
  {
    return PDI_NONE;
  }
}

int rlc_data_1_checker(T_PDI_CCDMSG* ccdmsg, ULONG values[], int len)
{
  if (len >= 1 && values[0] == 1) //rlc_id
  {
    return PDI_CCDMSG;
  }
  else
  {
    return PDI_NONE;
  }
}
