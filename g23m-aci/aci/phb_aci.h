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
|  Purpose :  Interface of SIM phonebook to ACI (and MFW)
|             
|             Details can be found in the design document 
|             "Phone Book Re-Architecture
+----------------------------------------------------------------------------- 
*/ 

#ifndef PHB_ACI_H
#define PHB_ACI_H

#include "phb_sim.h"

/*
 * Constants 
 */

/* FDN operation mode */
typedef enum
{
  NO_OPERATION = 0,       /* */
  FDN_DISABLE,            /* FDN disabled */
  FDN_ENABLE              /* FDN enabled */
} T_PHB_FDN_MODE;


/* Phonebook callback event */
typedef enum
{
  PHB_CB_BOOK_LOCKED,     /* Phonebook locked by SIM activity */
  PHB_CB_BOOK_UNLOCKED,   /* Phonebook unlocked from SIM activity */
  PHB_CB_WRITE_FINISHED   /* Write / delete operation finished */
} T_PHB_CB_EVENT;


/* Phonebook status */
typedef enum
{
  PHB_UNKNOWN = 0,        /* Status unknown */
  PHB_BUSY,               /* Phonebook busy, e.g. SIM access in progress */
  PHB_READY,              /* Phonebook ready */
  PHB_WRITE_FAIL          /* Write failed */
} T_PHB_STAT;

/* 
 * Type definitions 
 */
/* The ACI part of the phonebook does not define it's own structures or data
 * types (except of enums which are considered as constants) but relies on 
 * data types defined by the SIM part of the phonebook respective the
 * database manager. */

/* 
 * Function prototypes 
 */

void           pb_init                  (void);
void           pb_exit                  (void);
void           pb_reset                 (void);
void           pb_set_sim_ecc           (USHORT cause,
                                         UBYTE ecc_len,
                                         const UBYTE *sim_ecc);
void           pb_inserted_sim          (UBYTE c_sim_serv,
                                         UBYTE *sim_serv,
                                         const T_imsi_field *imsi_field,
                                         UBYTE adn_bdn_fdn_func,
                                         UBYTE phase);
void           pb_start_build           (BOOL unchanged);
BOOL           pb_update                (int ref,
                                         T_SIM_FILE_UPDATE_IND *fu);
T_PHB_RETURN   pb_flush_data            (void);
T_PHB_RETURN   pb_add_record            (T_PHB_TYPE type,

                                         USHORT phy_recno,
                                         const T_PHB_RECORD *entry);
T_PHB_RETURN   pb_del_record            (T_PHB_TYPE type,
                                         USHORT phy_recno);
T_PHB_RETURN   pb_check_fdn             (UBYTE toa,
                                         const UBYTE *number);
T_PHB_RETURN   pb_switch_adn_fdn        (T_PHB_FDN_MODE mode,
                                         T_ACI_CLASS classFDN);
T_PHB_RETURN   pb_del_book              (T_PHB_TYPE book);
T_ACI_CLASS    pb_get_fdn_classtype     (void);
T_PHB_FDN_MODE pb_get_fdn_mode          (void);
EXTERN T_PHB_TYPE pb_get_phb_type_from_ef      (USHORT ef);
/*
 * Wrapper functions
 */

T_PHB_RETURN   pb_read_record           (T_PHB_TYPE type,
                                         SHORT phy_recno,
                                         T_PHB_RECORD *entry);
T_PHB_RETURN   pb_read_alpha_record     (T_PHB_TYPE type,
                                         SHORT order_num,
                                         T_PHB_RECORD *entry);
T_PHB_RETURN   pb_read_number_record    (T_PHB_TYPE type,
                                         SHORT order_num,
                                         T_PHB_RECORD *entry);
T_PHB_RETURN   pb_search_name           (T_PHB_TYPE type,
                                         T_PHB_MATCH match,
                                         const T_ACI_PB_TEXT *search_tag,
                                         SHORT *order_num);
T_PHB_RETURN   pb_search_number         (T_PHB_TYPE type,
                                         const UBYTE *number,
                                         SHORT *order_num);
T_PHB_RETURN   pb_read_sizes            (T_PHB_TYPE type, /* IN */
                                         SHORT *max_rcd,  /* OUT */
                                         SHORT *used_rcd, /* OUT */
                                         UBYTE *num_len,  /* OUT */
                                         UBYTE *tag_len,  /* OUT */
                                         SHORT *max_ext,  /* OUT */
                                         SHORT *used_ext);/* OUT */
int            pb_get_entry_len         (const UBYTE *pb_tag,
                                         UBYTE max_pb_len);
T_PHB_RETURN   pb_del_book              (T_PHB_TYPE type);

/* 
 * Improved functionality / backward compatibility
 */
T_PHB_RETURN   pb_search_number_ex      (T_PHB_TYPE type,
                                         const UBYTE *number,
                                         SHORT *order_num,
                                         SHORT *found,
                                         T_PHB_RECORD *entry);

T_PHB_RETURN   pb_search_name_ex        (T_PHB_TYPE type,
                                         T_PHB_MATCH match,
                                         const T_ACI_PB_TEXT *search_tag,
                                         SHORT *order_num,
                                         SHORT *found,
                                         T_PHB_RECORD *entry);

/* 
 * New functionality
 */
int            pb_find_free_record      (T_PHB_TYPE type);


/* 
 * New functionality
 */
int            pb_find_ext_free_record      ();

void           pb_update_ext_bitmap (UBYTE rec_num, BOOL flag); 


/* 
 * Functionality to update status of Phonebook to other modules.
 */
void         pb_status_req           (UBYTE *pb_stat);

#endif /* #ifndef PHB_ACI_H */
