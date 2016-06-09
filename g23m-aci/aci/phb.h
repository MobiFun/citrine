/* 
+----------------------------------------------------------------------------- 
|  Project :  MMI-Framework (8417)
|  Modul   :  PSA_PHB
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
|  Purpose :  Definitions for phonebook management of MMI framework
+----------------------------------------------------------------------------- 
*/ 
#ifndef PHB_H
#define PHB_H

#ifdef TI_PS_FFS_PHB

/*
 * Include the header files needed for the FFS based SIM phonebook.
 */
#include "db.h"
#include "phb_sim.h"
#include "phb_aci.h"

#else

/*
 * Old RAM based SIM phonebook.
 */

/* A definition to avoid TI_PS_FFS_PHB sometimes */
typedef UBYTE T_PHB_STAT;

#define PHB_BUSY                  4
#define PHB_READY                 5
#define PHB_UNKNOWN               6
#define PHB_WRITE_FAIL            7

#define PHB_NUMBER                1
#define PHB_NAME                  2
#define PHB_INDEX                 3

#define PHB_NO_EXT                0
#define PHB_NUMBER_EXT            1
#define PHB_SUBADDR1_EXT          2
#define PHB_SUBADDR2_EXT          3

/* SIM operation mode */

#define NO_OPERATION              0
#define FDN_DISABLE               1
#define FDN_ENABLE                2

/* definition for the maximal records in the phone book. */
#define MAX_PHB_LIST        10

#define MAX_ECC_RCD          5
#define PHB_MAX_TAG_LEN     20

#ifdef PHONEBOOK_EXTENSION
#define PHB_PACKED_NUM_LEN  22
#else
#define PHB_PACKED_NUM_LEN  10
#endif /* #ifdef PHONEBOOK_EXTENSION */

#define PHB_EXT_RCD          4

#define MAX_PHB_ENTRY        5
#if !defined(FAX_AND_DATA) AND !defined(GPRS)
  /*
  * for GoLite decreased to 200 entries
  */
  #define MAX_AFB_RECORDS    200
#else

#ifndef _SIMULATION
  /* 
  * Increased from 255 to 300 entries - 
  * now 16bit encoded - might be even 
  * larger to maintain all SIM phb entries 
  */
  #define MAX_AFB_RECORDS    300
#else /* _SIMULATION */

  /* The number is decreased iin case of simulation for
     phonebook testing purpose */    
  #define MAX_AFB_RECORDS     8
#endif /* _SIMULATION */

#endif

#define MAX_RDM_RECORDS     30

#define MAX_ADN_BITMAP      32
#define MAX_FDN_BITMAP      10
#define MAX_BDN_BITMAP       5
#define MAX_SDN_BITMAP       5
#define MAX_ECC_BITMAP       1
#define MAX_UPN_BITMAP       1
#define PHB_ELEMENT_FREE     0
#define PHB_ELEMENT_USED     1

#ifdef PHONEBOOK_EXTENSION
#define MAX_EXT1_BITMAP      8
#define MAX_EXT2_BITMAP      4
#define MAX_EXT3_BITMAP      4
#define MAX_EXT4_BITMAP      4

typedef enum
{
    EXT1 = 0,        /* ADN, LDN phonebook Extention */
    EXT2,            /* FDN phonebook Extention      */
    EXT3, 
    EXT4, 
    MAX_PHB_EXT
} T_PHB_EXT_TYPE;

typedef struct
{
  UBYTE mem;
  UBYTE max_rcd;
  UBYTE *rcd_bitmap;
} T_PHB_EXT_RECORDS;
#endif /* PHONEBOOK_EXTENSION */

typedef enum
{
    NO_PHB_ENTRY = 0,
    SIM_MEMORY,
    TE_MEMORY
} T_PHB_MEMORY;

#ifndef NO_ASCIIZ
#define NO_ASCIIZ
#endif

#ifndef TI_PS_FFS_PHB
typedef enum
{
    ECC = 0,         /* Emergency call numbers        */
    ADN,             /* Abbreviated dialing number    */
    FDN,             /* Fixed dialing number          */
    BDN,             /* Barred dialing number         */
    LDN,             /* Last dialing number           */
    LRN,             /* Last received number          */
    SDN,             /* Service dialing number        */
    LMN,             /* last missed number            */
    ADN_FDN,         /* merged ADN and FDN            */
    UPN,             /* User person number            */
    MAX_PHONEBOOK
} T_PHB_TYPE;
#endif

/* Phonebook functional return codes */
typedef enum
{
  PHB_FAIL = -1,         /* execution of command failed           */
  PHB_OK,                /* execution of command completed        */
  PHB_FULL,              /* Phonebook is full                     */
  PHB_EXCT,              /* execution (writing to SIM) is running */
  PHB_INVALID_IDX,       /* invalid index requested               */
  PHB_TAG_EXCEEDED,      /* entry is too long to fit on SIM       */
  PHB_EXT_FULL           /* Phonebook Extension is full */
} T_PHB_RETURN;

/* Phonebook search flag */
typedef enum
{
  PHB_NEW_SEARCH = 0,         /* execution of command failed    */
  PHB_NEXT_SEARCH             /* execution of command completed */
} T_PHB_SEARCH;

typedef struct
{
    UBYTE   book;
    UBYTE   index;                   /* record number ( might be > 255)           */ 
    UBYTE   tag_len;                 /* Length of Alpha identifier               */
    UBYTE   tag[PHB_MAX_TAG_LEN];    /* Alpha identifier                         */
    UBYTE   len;                     /* Length of BCD number/SSC contens         */
    UBYTE   ton_npi;                 /* TON and NPI                              */
    UBYTE   number[PHB_PACKED_NUM_LEN]; /* Dialing number/SSC string             */
    UBYTE   subaddr[PHB_PACKED_NUM_LEN];
    UBYTE   cc_id;                   /* Capability/Configuration identifier      */
    UBYTE   year;
    UBYTE   month;
    UBYTE   day;
    UBYTE   hour;
    UBYTE   minute;
    UBYTE   second;
    UBYTE   line;                    /* MC line call was made from/received upon */
} T_PHB_RECORD;

typedef struct
{
    UBYTE   index;                   /* record number                            */
    UBYTE   tag_len;                 /* Length of Alpha identifier               */
    UBYTE   tag[PHB_MAX_TAG_LEN];    /* Alpha identifier                         */
    UBYTE   len;                     /* Length of BCD number/SSC contens         */
    UBYTE   ton_npi;                 /* TON and NPI                              */
    UBYTE   number[PHB_PACKED_NUM_LEN]; /* Dialing number/SSC string             */
#ifdef PHONEBOOK_EXTENSION
    UBYTE   subaddr[PHB_PACKED_NUM_LEN];
    UBYTE   ext_rcd_num;             /* number of the extention record (0xFF => not used) */
#endif
    UBYTE   cc_id;                   /* Capability/Configuration identifier      */
} T_AFB_RECORD;

/* Phone book entries */

/* define UNUSED_INDEX  0xFFFF */
#define UNUSED_INDEX (-1) 
#define UNUSED_BYTE_INDEX 0xFF

typedef struct T_PHB_AFB_ELEMENT
{
    UBYTE        free;
    UBYTE        type;
    SHORT        prev_rcd;
    SHORT        next_rcd;
    SHORT        prev_trcd;
    SHORT        next_trcd;
    SHORT        prev_nrcd;
    SHORT        next_nrcd;
    SHORT        prev_mtrcd;
    SHORT        next_mtrcd;
    SHORT        prev_mnrcd;
    SHORT        next_mnrcd;
    T_AFB_RECORD entry;
} T_PHB_AFB_ELEMENT;

typedef struct
{
    UBYTE   index;                   /* record number                            */
    UBYTE   tag_len;                 /* Length of Alpha identifier               */
    UBYTE   tag[PHB_MAX_TAG_LEN];    /* Alpha identifier                         */
    UBYTE   year;
    UBYTE   month;
    UBYTE   day;
    UBYTE   hour;
    UBYTE   minute;
    UBYTE   second;
    UBYTE   len;                     /* Length of BCD number/SSC contens         */
    UBYTE   ton_npi;                 /* TON and NPI                              */
    UBYTE   number[PHB_PACKED_NUM_LEN]; /* Dialing number/SSC string             */
#ifdef PHONEBOOK_EXTENSION
    UBYTE   subaddr[PHB_PACKED_NUM_LEN];
#endif
    UBYTE   cc_id;                   /* Capability/Configuration identifier      */
    UBYTE   line;                    /* line call was made from/received upon....*/
} T_RDM_RECORD;

/* Phone book entries */
typedef struct T_PHB_RDM_ELEMENT
{
    UBYTE        free;
    UBYTE        type;
    UBYTE        prev_rcd;
    UBYTE        next_rcd;
    T_RDM_RECORD entry;
} T_PHB_RDM_ELEMENT;

/*
typedef struct T_PHB_RDM_ELEMENT
{
    UBYTE        free;
    UBYTE        type;
    SHORT        prev_rcd;
    SHORT        next_rcd;
    T_RDM_RECORD entry;
} T_PHB_RDM_ELEMENT;
*/

/* Control block for phone book */
typedef struct
{
    UBYTE mem;
    UBYTE type;
    UBYTE service;
    UBYTE alpha_len;
    SHORT max_rcd;
    SHORT used_rcd;
    SHORT first_rcd;
    SHORT first_trcd;
    SHORT first_nrcd;
    SHORT first_mtrcd;
    SHORT first_mnrcd;
    UBYTE *rcd_bitmap;
} T_PHB_CTB;

/*
typedef struct
{
    UBYTE mem;
    UBYTE type;
    UBYTE service;
    UBYTE alpha_len;
    UBYTE max_rcd;
    UBYTE used_rcd;
    SHORT first_rcd;
    SHORT first_trcd;
    SHORT first_nrcd;
    SHORT first_mtrcd;
    SHORT first_mnrcd;
    UBYTE *rcd_bitmap;
} T_PHB_CTB;
*/

typedef SHORT (*T_PHB_EXT_CMP_FCT)(UBYTE*, USHORT, UBYTE*, USHORT);

EXTERN void         pb_set_compare_fct      (T_PHB_EXT_CMP_FCT fct_compare);
EXTERN void         phb_Init                (void);
EXTERN void         pb_init                 (void);
EXTERN void         pb_init_afb             (void);
EXTERN void         pb_exit                 (void);
EXTERN void         pb_reset                (void);
EXTERN T_PHB_RETURN pb_read_ecc             (USHORT error, UBYTE ecc_len, UBYTE *sim_ecc);
EXTERN BOOL         pb_read_sim             (USHORT data_id, UBYTE rcd_num, UBYTE len);
EXTERN void         pb_copy_sim_entry       (SHORT cur_index);
EXTERN void         pb_read_cb              (SHORT table_id);
EXTERN void         pb_read_ext_cb          (SHORT table_id);
EXTERN void         pb_build_req            (T_SIM_MMI_INSERT_IND *sim_mmi_insert_ind);
EXTERN T_PHB_RETURN pb_start_build          (BOOL unchanged);
EXTERN T_PHB_RETURN pb_read_eeprom_req      (void);
EXTERN void         pb_read_sim_req         (void);
EXTERN T_PHB_RETURN pb_write_sim            (UBYTE type, UBYTE rcd_num);
EXTERN void         pb_write_eeprom         (void);
EXTERN UBYTE        pb_ssc                  (UBYTE nr, UBYTE * serv_table);
EXTERN void         pb_record_sort          (SHORT cur_index);
EXTERN void         pb_l_record_sort        (SHORT cur_index);
EXTERN void         pb_alpha_sort           (SHORT cur_index);
EXTERN void         pb_num_sort             (SHORT cur_index);
EXTERN void         pb_malpha_sort          (SHORT cur_index);
EXTERN void         pb_mnum_sort            (SHORT cur_index);
EXTERN T_PHB_RETURN pb_add_record           (UBYTE type, UBYTE index, T_PHB_RECORD *entry);
EXTERN T_PHB_RETURN pb_delete_record        (UBYTE type, UBYTE index, UBYTE *ext_rcd_num, BOOL permanent);
EXTERN T_PHB_RETURN pb_read_phys_record     (UBYTE type, SHORT index, T_PHB_RECORD *entry);
EXTERN T_PHB_RETURN pb_read_index_record    (UBYTE type, SHORT index, T_PHB_RECORD *entry);
EXTERN T_PHB_RETURN pb_read_alpha_record    (UBYTE type, SHORT index, T_PHB_RECORD *entry);
EXTERN T_PHB_RETURN pb_read_number_record   (UBYTE type, SHORT index, T_PHB_RECORD *entry);
EXTERN T_PHB_RETURN pb_search_name          (T_ACI_CMD_SRC srcId,
                                             UBYTE         type,
                                             T_ACI_PB_TEXT *searchName,
                                             UBYTE mode,
                                             SHORT *first_ind,
                                             SHORT *result,
                                             T_PHB_RECORD *entry);
EXTERN T_PHB_RETURN pb_search_number        (UBYTE type, UBYTE *number,
                                             UBYTE mode,
                                             SHORT *first_ind,
                                             SHORT *result,
                                             T_PHB_RECORD *entry);
EXTERN void         pb_rcd_chain            (UBYTE type,
                                             SHORT prev_index,
                                             SHORT cur_index,
                                             SHORT next_index);
EXTERN void         pb_l_rcd_chain          (UBYTE type,
                                             SHORT prev_index,
                                             SHORT cur_index,
                                             SHORT next_index);
EXTERN void         pb_name_chain           (UBYTE type,
                                             SHORT prev_index,
                                             SHORT cur_index,
                                             SHORT next_index);
EXTERN void         pb_num_chain            (UBYTE type,
                                             SHORT prev_index,
                                             SHORT cur_index,
                                             SHORT next_index);
EXTERN void         pb_mname_chain          (UBYTE type,
                                             SHORT prev_index,
                                             SHORT cur_index,
                                             SHORT next_index);
EXTERN void         pb_mnum_chain           (UBYTE type,
                                             SHORT prev_index,
                                             SHORT cur_index,
                                             SHORT next_index);
EXTERN T_PHB_RETURN pb_read_status          (UBYTE type, UBYTE *service,
                                             SHORT *max_rcd, SHORT *used_rcd,
                                             UBYTE *tag_len, SHORT *avail_rcd,
                                             SHORT *max_ext, SHORT *used_ext);
EXTERN void         pb_status_req           (UBYTE *mode);
EXTERN T_PHB_RETURN pb_first_free           (UBYTE type,SHORT *first_free);
EXTERN T_PHB_RETURN pb_check_fdn (UBYTE toa, const UBYTE *number);
EXTERN UBYTE        pb_check_number         (char *cur_number, char *number);
#ifdef PHONEBOOK_EXTENSION
EXTERN T_PHB_RETURN pb_write_sim_ext        (USHORT data_id, UBYTE rcd_num);
#endif
EXTERN T_PHB_RETURN pb_switch_adn_fdn       (UBYTE mode, T_ACI_CLASS classFDN);
EXTERN void         pb_copy_ldn_record      (SHORT index, UBYTE flag);
EXTERN void         pb_copy_lrn_record      (SHORT index, UBYTE flag);
EXTERN void         pb_copy_lmn_record      (SHORT index, UBYTE flag);
EXTERN T_PHB_RETURN pb_delete_book          (UBYTE book);
EXTERN void         pb_delete_sim_book      (UBYTE book);
EXTERN void         pb_update_ecc           (void);
EXTERN int          pb_get_entry_len        (const UBYTE *pb_tag, UBYTE max_pb_len);
EXTERN T_ACI_CLASS  pb_get_fdn_classtype    (void);
EXTERN void         pb_set_fdn_input_classtype    (T_ACI_CLASS classtype);
EXTERN UBYTE        pb_get_fdn_mode    (void);
EXTERN void         pb_set_fdn_mode   (UBYTE fdnmode);
#endif /* else, #ifdef TI_PS_FFS_PHB */
#endif /* #ifndef PHB_H */
