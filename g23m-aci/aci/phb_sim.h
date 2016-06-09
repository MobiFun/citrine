/* 
+----------------------------------------------------------------------------- 
|  Project :  MMI-Framework (8417)
|  Modul   :  PSA_PHB
+----------------------------------------------------------------------------- 
|  Copyright 2005 Texas Instruments Berlin, AG 
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
|  Purpose :  Definitions for SIM phonebook management block
|             The functions of this block are aware about SIM data structures,
|             they are not responsible for communication with the SIM itself.
|             
|             Details can be found in the design document 
|             "Phone Book Re-Architecture
+----------------------------------------------------------------------------- 
*/ 

#ifndef PHB_SIM_H
#define PHB_SIM_H

#include "db.h"
/*
 * Constants 
 */
#define PHB_MAX_TAG_LEN    20 /* Arbitrary, old phonebook, nobody complained */

/*
 * 04.08 clause 9.3.23.2 Called Party BCD allows for a 
 * PHB_PACKED_NUM_LEN of 40 as biggest definition which 
 * still could make any sense, but we have also to consider
 * ACI's MAX_PHB_NUM_LEN in aci_cmh.h, therefore a definition
 * of PHB_PACKED_NUM_LEN with size 20 makes more sense.
 * With this, an ADN entry plus exatly one EXT1 record is filled.
 */
#define PHB_PACKED_NUM_LEN 20 /* See MAX_PHB_NUM_LEN in aci_cmh.h */

#define PHB_PACKED_SUB_LEN 22 /* 11.11 clause 10.5.10, chained 2 EXT records */
#define ECC_NUM_LEN          3
#define FFS_PHB_DIR "/phb_db"

//#define SIM_ICI               0x6f80  /* Release 1999 LRN, LMN */
#define FFS_LRN               0xFF00  /* Structure of SIM_OCI, for LRN, arbitrary unique value */
#define FFS_LMN               0xFF01  /* Structure of SIM_OCI, for LMN, arbitrary unique value */
#define FFS_EXT_LRN           0xFF02  /* Structure of EXT5 for LRN, arbitrary unique value */
#define FFS_EXT_LMN           0xFF03  /* Structure of EXT5 for LMN, arbitrary unique value */
#define SIM_OCI               0x6f81  /* Release 1999 LDN */
// 0x6f4e is for the SIM EXT4, so we to assign another value for EXT5 here
//#define SIM_EXT5              0x6f4e  /* Release 1999 EXT5 */
#define SIM_EXT5              0x6f55  /* Arbitrary EXT5 value for EF_ICI/EF_OCI */

#define NAME_IDX  1
#define NUMBER_IDX 2

/* ECC Phonebook Structure */
typedef struct
{
  USHORT    phy_idx;					/* physical record number  */
  UBYTE     number[ECC_NUM_LEN]; /* Dialing number/SSC string  */
} T_PHB_ECC_RECORD;

/* Type of phonebook */
typedef enum
{ 
  INVALID_PHB = -1,        /* Invalid Phonebook Type. */
  ECC = 0,      /* Emergency call numbers      */  /* SIM => EF_ECC */
  ADN,          /* Abbreviated dialing number  */  /* SIM => EF_ADN */
  FDN,          /* Fixed dialing number        */  /* SIM => EF_FDN */
  BDN,          /* Barred dialing number       */  /* SIM => EF_BDN */
  LDN,          /* Last dialing number         */  /* FFS => "/pcm/LDN" (old phb) */
  LRN,          /* Last received number        */  /* FFS => "/pcm/LRN" (old phb) */
  SDN,          /* Service dialing number      */  /* SIM => "EF_SDN"  */
  LMN,          /* last missed number          */  /* FFS => "/pcm/LMN" (old phb) */
  UPN,          /* User person number          */  /* SIM => EF_MSISDN */
  ME,           /* ME phonebook, flash based   */  /* FFS => to be defined */
  ADN_ME,       /* ADN and ME phonebook merged */
  MAX_PHONEBOOK
} T_PHB_TYPE;

/* Type of extension */
typedef enum
{
  INVALID_EXT = -1,
  EXT1,             /* ADN, LDN phonebook Extention */
  EXT2,             /* FDN phonebook Extention      */
  EXT3,
  EXT4,
  EXT5,             /* Release 1999+ for EF_ICI, EF_OCI */
  EXT_LRN,          /* Extension for LRN */
  EXT_LMN,          /* Extension for LMN */
  MAX_PHB_EXT
} T_EXT_TYPE; 

/* Phonebook result codes. Note: Not every result code occurs within the 
 * SIM block of the phonebook (e.g. PHB_EXCT) */
typedef enum
{
  PHB_FAIL = -1,    /* execution of command failed */
  PHB_LOCKED,       /* needed elementary file locked */
  PHB_OK,           /* execution of command completed */
  PHB_FULL,         /* Phonebook is full */
  PHB_EXCT,         /* execution (writing to SIM) is running */
  PHB_INVALID_IDX,  /* invalid index requested */
  PHB_TAG_EXCEEDED, /* entry is too long to fit on SIM */
  PHB_EMPTY_RECORD, /* attempted to read a free record */
  PHB_EXT_FULL      /* Phonebook Extension is full */
} T_PHB_RETURN;


/* Phonebook entry match criteria */ // ### Is this PHB_SIM?
typedef enum
{
  PHB_MATCH_GE,       /* Match entry if greater or equal */
  PHB_MATCH_PARTIAL   /* Match entry if exact partial match */
} T_PHB_MATCH;


/* File locking type */
typedef enum
{
  PHB_UNLOCKED = 0, /* Elementary file not locked */
  PHB_W_LOCK = 1,   /* Elementary file locked for writing */
  PHB_R_LOCK = 2,   /* Elementary file locked for reading */
  PHB_RW_LOCK = 3   /* Elementary file locked */
} T_PHB_LOCK;


/* 
 * Type definitions 
 */

/* Phonebook time */
typedef struct
{
  UBYTE year;
  UBYTE month;
  UBYTE day;
  UBYTE hour;
  UBYTE minute;
  UBYTE second;
  UBYTE time_zone;                    /* 0xff means not present */
  ULONG duration;                     /* 0 means not answered,  */
                                      /* 0xFFFFFFFF not present */
} T_PHB_TIME;


/* Phonebook record */
typedef struct
{
  USHORT  phy_recno;                  /* physical record number              */
  UBYTE   tag_len;                    /* Length of Alpha identifier          */
  UBYTE   tag[PHB_MAX_TAG_LEN];       /* Alpha identifier                    */
  UBYTE   len;                        /* Length of BCD number/SSC contents   */
  UBYTE   ton_npi;                    /* TON and NPI                         */
  UBYTE   number[PHB_PACKED_NUM_LEN]; /* Dialing number/SSC string           */
  UBYTE   subaddr[PHB_PACKED_SUB_LEN];
  UBYTE   cc_id;                      /* Capability/Configuration identifier */
  UBYTE   v_time;                     /* Valid flag */
  T_PHB_TIME time;                    /* Time of phone call */
  UBYTE   v_line;                     /* Valid flag */
  UBYTE   line;                       /* MC line call was made/received upon */
} T_PHB_RECORD;


/* 
 * Function prototypes 
 */

EXTERN void          pb_sim_init               (void);
EXTERN void          pb_sim_exit               (void);
EXTERN T_PHB_RETURN  pb_sim_set_ecc            (UBYTE ecc_len,
                                         const UBYTE *sim_ecc);
EXTERN T_PHB_RETURN  pb_sim_create_ef          (USHORT ef,
                                         USHORT record_size,
                                         USHORT records);
EXTERN T_PHB_RETURN pb_sim_write_ef         (USHORT ef,
                                         USHORT phy_recno,
                                         USHORT entry_size,
                                         const UBYTE *buffer,
                                         BOOL *changed,
                                         USHORT *ext_record_ef,
                                         UBYTE  *ext_record_no);

EXTERN T_PHB_RETURN  pb_sim_open               (const T_imsi_field *imsi_field,
                                         BOOL *changed);
EXTERN T_PHB_RETURN  pb_sim_read_ef            (USHORT ef,
                                         USHORT phy_recno,
                                         USHORT *entry_size,
                                         UBYTE *buffer);
EXTERN T_PHB_RETURN  pb_sim_remove_ef          (USHORT ef);
EXTERN T_PHB_RETURN  pb_sim_build_index        (T_PHB_TYPE type);
EXTERN T_PHB_RETURN  pb_sim_flush_data         (void);
EXTERN T_PHB_RETURN  pb_sim_add_record         (T_PHB_TYPE type,
                                         USHORT phy_recno,
                                         const T_PHB_RECORD *entry,
                                         T_DB_CHANGED *rec_affected);
EXTERN T_PHB_RETURN  pb_sim_del_record         (T_PHB_TYPE type,
                                         USHORT phy_recno,
                                         T_DB_CHANGED *rec_affected);
EXTERN T_PHB_RETURN  pb_sim_read_record        (T_PHB_TYPE type,
                                         USHORT phy_recno,
                                         T_PHB_RECORD *entry);
EXTERN T_PHB_RETURN pb_sim_read_alpha_num_record (T_PHB_TYPE    type, 
                                                  USHORT        order_num,
                                                  T_PHB_RECORD *entry,
                                                  UBYTE         sort_index);
EXTERN T_PHB_RETURN  pb_sim_search_name        (T_PHB_TYPE type,
                                         T_PHB_MATCH match,
                                         const T_ACI_PB_TEXT *search_tag,
                                         SHORT *order_num);
EXTERN T_PHB_RETURN  pb_sim_search_number      (T_PHB_TYPE type,
                                         const UBYTE *number,
                                         SHORT *order_num);
EXTERN T_PHB_RETURN  pb_sim_read_sizes         (T_PHB_TYPE type, /* IN */
                                                SHORT *max_rcd,  /* OUT */
                                                SHORT *used_rcd, /* OUT */
                                                UBYTE *tag_len,  /* OUT */
                                                SHORT *max_ext,  /* OUT */
                                                SHORT *used_ext);/* OUT */
EXTERN int           pb_sim_get_entry_len      (const UBYTE *pb_tag,
                                         UBYTE max_pb_len);
EXTERN int           pb_sim_find_free_record   (T_PHB_TYPE type);

EXTERN int           pb_sim_find_free_ext_record   ();

EXTERN void          pb_sim_update_ext_bitmap(UBYTE rec_num, BOOL flag);

EXTERN int           pb_sim_cmpWild            ( const char  *s1, 
                                                 const char *s2, 
                                                 size_t cmp_len );

EXTERN void          pb_update_cphs_mb_ext_record(void);

#endif /* #ifndef PHB_SIM_H */
