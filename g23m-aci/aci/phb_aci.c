/* 
+----------------------------------------------------------------------------- 
|  Project :  MMI-Framework (8417)
|  Modul   :  PHB
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
|  Purpose :  This modul contains the functions to establish the phone book.
+----------------------------------------------------------------------------- 
*/

#ifdef TI_PS_FFS_PHB

#include "aci_all.h"
#include "aci_cmh.h"
#include "aci_mem.h"

#include "phb_aci.h"

#ifdef _SIMULATION_
  /*
   * Unfortunately we have a double definition of some preprocessor definitions
   * within windows.h and within the protocol stack, so we undefine here.
   */
  #undef PARITY_NONE
  #undef PARITY_ODD
  #undef PARITY_EVEN
  #undef DRV_OK
  #include "ffs_pc_api.h"
  #undef PARITY_NONE
  #undef PARITY_ODD
  #undef PARITY_EVEN
  #undef DRV_OK
#else
  #include "ffs/ffs.h"
#endif

#ifdef SIM_TOOLKIT
#include "psa.h"
#include "psa_sim.h"
#include "psa_cc.h"
#include "psa_sat.h"
#endif /* #ifdef SIM_TOOLKIT */

#include "cmh.h"
#include "cmh_phb.h"

#ifdef DTI
#include "dti_conn_mng.h"
#endif
#include "cmh_sim.h"


/*
 * Constants and enumerations
 */
/* #define NR_EF_ICI             20   LRN, LMN */
/* #define SIZE_EF_ICI           (28+16)  alpha id 16 bytes, 31.102 clause 4.2.33 */

#define NR_EF_OCI             10  /* Arbitrary, LDN */
#define SIZE_EF_OCI           (27+16)  /* alpha id 16 bytes, 31.102 clause 4.2.34 */

#define NR_EF_EXT5            10  /* Arbitrary */
#define SIZE_EF_EXT5          13  /* 31.102 clause 4.2.37 */

#define NR_EF_LRN             10  /* Arbitrary, LRN */
#define SIZE_EF_LRN           SIZE_EF_OCI

#define NR_EF_LMN             10  /* Arbitrary, LMN */
#define SIZE_EF_LMN           SIZE_EF_OCI

#define NR_EF_EXT_LRN         10  /* Arbitrary */
#define SIZE_EF_EXT_LRN       SIZE_EF_EXT5

#define NR_EF_EXT_LMN         10  /* Arbitrary */
#define SIZE_EF_EXT_LMN       SIZE_EF_EXT5

#define PHB_MAX_QUEUE         4   /* Queued LDN entries */
#define SIM_MAX_RECORD_SIZE   256 /* Maximum size of a SIM record */

#define PHB_STATE_NULL        0   /* NULL state before reading phonebook    */
#define PHB_STATE_IDLE        1   /* IDLE state, phonebook has been read    */
#define PHB_STATE_VERIFY      2   /* Verify SIM (Initialization, SAT)       */
#define PHB_STATE_READ        3   /* Reading from SIM (Initialization, SAT) */
#define PHB_STATE_WRITE       4   /* Write/Delete single record to SIM      */
#define PHB_STATE_DELETE_BOOK 5   /* Delete a whole SIM phonebook           */

#define SET_PHB_STATE(n)  pba_data.state = n;
#define GET_PHB_STATE()   pba_data.state

typedef struct 
{
  USHORT        index;
  T_PHB_RECORD  entry;
} T_PHB_QUEUE;

/*
 * Type definitions
 */
typedef struct
{
  /* Fixed Dialling Numbers mode */
  T_PHB_FDN_MODE fdn_mode;

  /* Fixed Dialling Number class type */
  T_ACI_CLASS fdn_classtype;

  /* T_PHB_STATE of the phonebook */
  T_PHB_STAT phb_stat;
  
  /* Current state of the phonebook PHB_STATE_XXX */
  UBYTE state;

  /* SIM data */
  UBYTE *data;
  
  /* SIM service table */
  UBYTE sim_service_table[MAX_SRV_TBL];

  /* Database has to be recreated (IMSI changed or DB unclean) */
  BOOL db_recreate;

  /* Paused elementary file if reading extension record, otherwise 0 */
  USHORT paused_ef;

  /* Paused record number if reading extension record, otherwise 0 */
  UBYTE paused_no;

  /* Delete all: Book to delete, Read: Current book reading */
  T_PHB_TYPE current_book;
  
  /* Delete all: current record */
  UBYTE del_record;

  /* Book created */
  BOOL book_created[MAX_PHONEBOOK];

  /* Extension created */
  BOOL ext_created[MAX_PHB_EXT];

  /* We are reading the 1st ext record only to get the number of records */
  BOOL ext_dummy_read;

  /* Read flags for SIM phonebooks. Set to TRUE when reading has started */
  BOOL book_read[MAX_PHONEBOOK];

  /* Maximum number of phonebook records */
  UBYTE phb_record_max[MAX_PHONEBOOK];
  /* Maximum number of extension records */
  UBYTE ext_record_max[MAX_PHB_EXT];

  /* Record sizes of phonebook records */
  UBYTE phb_record_len[MAX_PHONEBOOK];
  /* Record sizes of extension records */
  UBYTE ext_record_len[MAX_PHB_EXT];

  /* Queued LDN entries during startup */
  UBYTE c_queued;
  T_PHB_QUEUE *queued[PHB_MAX_QUEUE];
  
  /* Records to be synchronized to the SIM */
  T_DB_CHANGED records_changed;

  /* To keep track of read extension records */
  BOOL *read_ext_record;

} T_PHB_ACI_DATA;

/* We pack all internal data into one data structure */
LOCAL T_PHB_ACI_DATA pba_data;


/* 
 * Prototypes for local functions
 */
LOCAL UBYTE pb_get_max_num_len          (T_PHB_TYPE type);
LOCAL BOOL pb_sim_service               (UBYTE nr);
LOCAL BOOL pb_read_sim_record           (USHORT data_id,
                                         UBYTE rcd_num,
                                         UBYTE len);
LOCAL BOOL pb_read_sim (USHORT data_id, UBYTE len, UBYTE max_length);
LOCAL void pb_read_next_sim_book        (void);
LOCAL void pb_read_sim_record_cb        (SHORT table_id);
LOCAL void pb_read_sim_cb               (SHORT table_id);
LOCAL void pb_sat_update_reset          (USHORT ef);
LOCAL T_PHB_RETURN pb_sync_next_sim_record (BOOL first);
LOCAL T_PHB_RETURN pb_write_sim_record  (USHORT ef,
                                         UBYTE phy_recno,
                                         UBYTE entry_size,
                                   const UBYTE *buffer);
LOCAL void pb_write_sim_record_cb       (SHORT table_id);
T_EXT_TYPE pb_get_ext_type_from_ef      (USHORT ef);
LOCAL void pb_status_ind                (T_PHB_STAT phb_stat,
                                         SHORT cmeError);
LOCAL T_PHB_RETURN pb_write_queue       (const T_PHB_RECORD *entry);
LOCAL void pb_clear_queue               (void);
LOCAL T_PHB_RETURN pb_read_queue        (void);

/*
 * Functions - Interface functions to external modules
 */


/*
+----------------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : PHB                                |
| STATE   : code                ROUTINE : pb_init                            |
+----------------------------------------------------------------------------+

  PURPOSE : Power-on initialization of the phonebook module. This function
            is called at a point in time by pei_init() when it cannot be 
            expected that the protocol stack has been booted up completely,
            so it is undesirable to do anything here which might rely on
            a module still not started up like the PCM or the FFS.
  
*/
GLOBAL void pb_init (void)
{
  TRACE_FUNCTION ("pb_init()");

  memset (&pba_data, 0, sizeof (T_PHB_ACI_DATA));

  pba_data.fdn_mode = NO_OPERATION;
  pba_data.fdn_classtype = CLASS_VceDatFaxSms;

  pb_sim_init();

#ifdef SIM_TOOLKIT
  simShrdPrm.fuRef= - 1;
  psaSAT_FURegister (pb_update);
#endif /* #ifdef SIM_TOOLKIT */

  SET_PHB_STATE (PHB_STATE_NULL);
}


/*
+----------------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : PHB                                |
| STATE   : code                ROUTINE : pb_exit_phb                        |
+----------------------------------------------------------------------------+

  PURPOSE : This function synchronizes, if necessary, all unwritten data,
            shuts down the SIM phonebook layer and deallocates all 
            resources. The SIM itself is not deactivated here.
            

*/
LOCAL void pb_exit_phb (void)
{
  TRACE_FUNCTION ("pb_exit_phb()");

  /* 
   * Flush everything which has not yet been written.
   * Note: It's a good idea if everything is written immediately
   * so that we don't need to flush anything here. Competitor phones
   * do the job in this way, this has the advantage that even after
   * a phone crash immediately after dialling a number or missing
   * a call nothing is lost.
   */
  (void)pb_sim_flush_data ();

  /* Shutdown the lower layers */
  pb_sim_exit ();

  /*
   * Free all allocated resources
   */
  if (pba_data.data NEQ NULL)
  {
    ACI_MFREE (pba_data.data);
    pba_data.data = NULL;
  }

  if (pba_data.read_ext_record NEQ NULL)
  {
    ACI_MFREE (pba_data.read_ext_record);
    pba_data.read_ext_record = NULL;
  }


  pb_clear_queue ();

  /*
   * Set phonebook to no operation 
   */
  pba_data.fdn_mode = NO_OPERATION;
  pba_data.phb_stat = PHB_UNKNOWN;  
  SET_PHB_STATE (PHB_STATE_NULL);
}


/*
+----------------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : PHB                                |
| STATE   : code                ROUTINE : pb_exit                            |
+----------------------------------------------------------------------------+

  PURPOSE : Shutdown phonebook and the SIM itself also.

*/
GLOBAL void pb_exit (void)
{
  TRACE_FUNCTION ("pb_exit()");

  /* Shutdown SIM phonebook */
  pb_exit_phb ();

  /* Shutdown the SIM itself */
  simShrdPrm.synCs = SYNC_DEACTIVATE;
  psaSIM_SyncSIM();
}


/*
+----------------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : PHB                                |
| STATE   : code                ROUTINE : pb_reset                           |
+----------------------------------------------------------------------------+

  PURPOSE : Resets the phonebook, basically pb_exit() / pb_init().
  
*/
GLOBAL void pb_reset (void)
{
  TRACE_FUNCTION ("pb_reset()");

  if (GET_PHB_STATE() EQ PHB_STATE_NULL)
    return;

  pb_exit_phb();
  pb_init();
}

/*
+----------------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : PHB                                |
| STATE   : code                ROUTINE : pb_set_sim_ecc                     |
+----------------------------------------------------------------------------+

  PURPOSE : This function set's the emergency call numbers.
            This can only happen at startup time when the SIM when
            a SIM_ACTIVATE_CNF / SIM_ACTIVATE_IND is received.
            At this point in time the entering of PIN may be outstanding,
            the only information we may have gotten are the emergency call
            numbers.

*/
GLOBAL void pb_set_sim_ecc (USHORT cause,
                            UBYTE ecc_len,
                            const UBYTE *sim_ecc)
{
  TRACE_FUNCTION ("pb_set_sim_ecc()");

  if (pba_data.fdn_mode NEQ NO_OPERATION)
  {
     /* 
      * Setting of the emergency call numbers after having gotten the
      * IMSI of the SIM is ignored.
      */
    return;
  }

  if ((cause EQ SIM_CAUSE_OTHER_ERROR) OR
      (cause EQ SIM_CAUSE_CARD_REMOVED) OR
      (cause >= SIM_CAUSE_PARAM_WRONG AND cause <= SIM_CAUSE_DRV_TEMPFAIL))
  {
    /* 
     * Some error from the SIM. Indicate a fallback to the PCM/FFS.
     */
    (void)pb_sim_set_ecc (0, NULL);
  }
  else
  {
    (void)pb_sim_set_ecc (ecc_len, sim_ecc);
  }



}


/*
+----------------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : PHB                                |
| STATE   : code                ROUTINE : pb_inserted_sim                    |
+----------------------------------------------------------------------------+

  PURPOSE : This function is called when the PIN has been entered and we have
            the IMSI and full access on the SIM phonebook. Reading/Verifying
            of phonebook data is not started at this point except for the 
            ECC phonebook.

*/
GLOBAL void pb_inserted_sim (UBYTE c_sim_serv,
                             UBYTE *sim_serv,
                             const T_imsi_field *imsi_field,
                             UBYTE adn_bdn_fdn_func,
                             UBYTE phase)       /* For further study */
{
  T_FFS_SIZE ffs_size;                /* FFS result code */
  T_ACI_CLASS classFDN;               /* FDN class */
  UBYTE ub_class = (UBYTE)CLASS_None; /* T_ACI_CLASS */

  TRACE_FUNCTION ("pb_inserted_sim()");

  /* Store SIM service table */
  if (c_sim_serv > MAX_SRV_TBL)
    c_sim_serv = MAX_SRV_TBL; /* Garbage protection */
  memcpy (pba_data.sim_service_table, sim_serv, c_sim_serv);

  /* 
   * Open SIM phonebook. Remember whether the database has to be recreated 
   * for some reason (SIM changed, database unclean).
   */
  if (pb_sim_open (imsi_field, &pba_data.db_recreate) NEQ PHB_OK)
  {
    /* We're here in really big trouble and can do nothing about it */
    TRACE_ERROR ("Fatal: pb_sim_open() NEQ PHB_OK");
  }

  /*
   * Update ECC phonebook
   */
  pba_data.book_read[ECC] = TRUE;
  if (!pb_read_sim (SIM_ECC, NOT_PRESENT_8BIT, (UBYTE)SIM_MAX_RECORD_SIZE))
  {
    /* Unexpected problem. Live with previously read values */
    TRACE_ERROR ("Unexpected: No free SIM slot");
  }

  switch (adn_bdn_fdn_func)
  {
    case SIM_ADN_ENABLED:
    case SIM_ADN_BDN_ENABLED:
      pba_data.fdn_mode = FDN_DISABLE;
      break;

    case SIM_FDN_ENABLED:
    case SIM_FDN_BDN_ENABLED:
      pba_data.fdn_mode = FDN_ENABLE;

      /* read last fdn_classtype from FFS */
      ffs_size = ffs_file_read ("/mmi/fdnClassType", &ub_class, sizeof(ub_class));
      
      if (ffs_size EQ sizeof(ub_class)) /* successful read */
      {
        classFDN = (T_ACI_CLASS)ub_class;
        
        /* only these two classes are currently supported */
        if (classFDN EQ CLASS_VceDatFax OR 
            classFDN EQ CLASS_VceDatFaxSms)
        {
          pba_data.fdn_classtype = classFDN;
        }
      }
      break;

    default: /* SIM_NO_OPERATION or trash */
      pba_data.fdn_mode = NO_OPERATION;
      break;
  }
}


/*
+----------------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : PHB                                |
| STATE   : code                ROUTINE : pb_start_build                     |
+----------------------------------------------------------------------------+

  PURPOSE : This function is called when the flash phonebook has to be 
            read/verified from/against the phonebook stored on the SIM card.
            This is triggered by the reception of the 
            MNSMS_REPORT_IND (SMS_STATE_READY) primitive.

*/
GLOBAL void pb_start_build (BOOL unchanged)
{
  TRACE_FUNCTION ("pb_start_build()");
  
  if (pba_data.db_recreate)
  {
    /* 
     * Indicate a busy phonebook. We have to re-read all from scratch 
     */
    pb_status_ind (PHB_BUSY, CME_ERR_NotPresent);
    SET_PHB_STATE (PHB_STATE_READ);
  }
  else
  {
    /* 
     * SIM (IMSI) not changed and database clean,
     * allow reading from the SIM phonebook by the MMI.
     */
    pb_status_ind (PHB_READY, CME_ERR_NotPresent);
    SET_PHB_STATE (PHB_STATE_VERIFY);
  }

  pba_data.ext_dummy_read = FALSE;

#ifdef _SIMULATION_
  TRACE_EVENT_P1 ("SRV_ADN: %s",  pb_sim_service (SRV_ADN)  ? "YES" : "NO");
  TRACE_EVENT_P1 ("SRV_FDN: %s",  pb_sim_service (SRV_FDN)  ? "YES" : "NO");
  TRACE_EVENT_P1 ("SRV_CCP: %s",  pb_sim_service (SRV_CCP)  ? "YES" : "NO");
  TRACE_EVENT_P1 ("SRV_MSISDN: %s",pb_sim_service (SRV_MSISDN) ? "YES" : "NO");
  TRACE_EVENT_P1 ("SRV_EXT1: %s", pb_sim_service (SRV_EXT1) ? "YES" : "NO");
  TRACE_EVENT_P1 ("SRV_EXT2: %s", pb_sim_service (SRV_EXT2) ? "YES" : "NO");
  TRACE_EVENT_P1 ("SRV_LDN: %s",  pb_sim_service (SRV_LDN)  ? "YES" : "NO");
  TRACE_EVENT_P1 ("SRV_SDN: %s",  pb_sim_service (SRV_SDN)  ? "YES" : "NO");
  TRACE_EVENT_P1 ("SRV_EXT3: %s", pb_sim_service (SRV_EXT3) ? "YES" : "NO");
  TRACE_EVENT_P1 ("SRV_BDN: %s",  pb_sim_service (SRV_BDN)  ? "YES" : "NO");
  TRACE_EVENT_P1 ("SRV_EXT4: %s", pb_sim_service (SRV_EXT4) ? "YES" : "NO");
#endif

  pb_read_next_sim_book();
}


/*
+----------------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : PHB                                |
| STATE   : code                ROUTINE : pb_update                          |
+----------------------------------------------------------------------------+

  PURPOSE : This function is called when the SIM wants to inform the 
            phonebook that some elementary files have been changed.
            Returns TRUE if no files aere changed affecting the phonebook,
            otherwise FALSE.

*/
GLOBAL BOOL pb_update (int ref,
                       T_SIM_FILE_UPDATE_IND *fu)
{
  unsigned i;           /* Index variable */
  T_PHB_TYPE phb_type;  /* Phonebook type */ 
  T_EXT_TYPE ext_type;  /* Extension type */ 
  USHORT ef;            /* Elementary file number */

  TRACE_FUNCTION ("pb_update()");

  simShrdPrm.fuRef = -1;  /* Assume phonebook is not affected */

  for (i = 0; i < fu->val_nr; i++)
  {
    ef = fu->file_info[i].datafield;

    if (ef EQ SIM_SST)
    {
      /*
       * When SIM service table is changed, then all SIM phonebooks
       * will be updated.
       */

      /* Mark all phonebooks as unread */
      pb_sat_update_reset (SIM_ECC);
      pb_sat_update_reset (SIM_ADN);
      pb_sat_update_reset (SIM_FDN);
      pb_sat_update_reset (SIM_BDN);
#ifdef SIM_LND_SUPPORT
      pb_sat_update_reset (SIM_LND);
#endif
      pb_sat_update_reset (SIM_MSISDN);
      pb_sat_update_reset (SIM_SDN);

      /* Mark all extensions as unread */
      pb_sat_update_reset (EXT1);
      pb_sat_update_reset (EXT2);
      pb_sat_update_reset (EXT3);
      pb_sat_update_reset (EXT4);

      if (!pb_read_sim (SIM_SST, NOT_PRESENT_8BIT, (UBYTE)SIM_MAX_RECORD_SIZE))
      {
        /*
         * We could not get an empty slot and are in trouble now as there is
         * no good error handling possible here. On the other hand, it is 
         * not expected ever that an error occurs here.
         */
        TRACE_ERROR ("Internal problem getting a SIM slot");
        return TRUE;  /* No update in progress */
      }

      simShrdPrm.fuRef = ref;   /* Something to do */

      SET_PHB_STATE (PHB_STATE_READ);
      return FALSE; /* Update in progress */
    }
  }

  /*
   * At least the SIM service table has not been changed, check now for
   * single phonebooks and extension records.
   */
  
  for (i = 0; i < fu->val_nr; i++)
  {
    ef = fu->file_info[i].datafield;
    phb_type = pb_get_phb_type_from_ef (ef);
    ext_type = pb_get_ext_type_from_ef (ef);

    if (phb_type NEQ INVALID_PHB)
    {
      /* Phonebook affected by change */
      simShrdPrm.fuRef = ref;

      /* Reset respective field */
      pb_sat_update_reset (ef);

      switch (phb_type)
      {
        case ADN:
          pb_sat_update_reset (SIM_FDN);
          break;

        case FDN:
          pb_sat_update_reset (SIM_ADN);
          break;
        
        default:
          break;
      }
    } 
    else if (ext_type NEQ INVALID_EXT)
    {
      /* Phonebook affected by change */
      simShrdPrm.fuRef = ref;

      /* Reset respective field */
      pb_sat_update_reset (ef);

      /* Mark also the appropriate book itself as unread */
      switch (ext_type)
      {
        case EXT1:
          pb_sat_update_reset (SIM_ADN);
          pb_sat_update_reset (SIM_MSISDN);
#ifdef SIM_LND_SUPPORT
          pb_sat_update_reset (SIM_LND);
#endif
          break;

        case EXT2:
          pb_sat_update_reset (SIM_FDN);
          break;

        case EXT3:
          pb_sat_update_reset (SIM_SDN);
          break;

        case EXT4:
          pb_sat_update_reset (SIM_EXT4);
          break;

        default:
          TRACE_ERROR ("Unexpected default"); /* Everything caught */
          break;
      }
    }
  }

  if (simShrdPrm.fuRef NEQ -1)
  {
    pb_start_build (FALSE);
    return FALSE; /* FALSE means update in progress */
  }

  return TRUE; /* TRUE means nothing to do */
}


/*
+----------------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : PHB                                |
| STATE   : code                ROUTINE : pb_flush_data                      |
+----------------------------------------------------------------------------+

  PURPOSE : This function is called when someone wants to flush some pending
            data to persistent memory. Probably a bad concept, it's always
            better to flush always internally if some operation is finished
            and to synchronize also to the SIM immediately. This has the
            advantage that even after a crash nothing is lost.

*/
T_PHB_RETURN pb_flush_data (void)
{
  TRACE_FUNCTION ("pb_flush_data()");

  /* This function is empty, and this should remain this way if possible */
  return PHB_OK;
}


/*
+----------------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : PHB                                |
| STATE   : code                ROUTINE : pb_add_record                      |
+----------------------------------------------------------------------------+

  PURPOSE : This function adds or replaces a phonebook entry.

*/
GLOBAL T_PHB_RETURN pb_add_record (T_PHB_TYPE type,
                                   USHORT phy_recno,
                                   const T_PHB_RECORD *entry)
{
  T_PHB_RETURN phb_result;
  T_DB_CHANGED dummy_changed;

  TRACE_FUNCTION ("pb_add_record()");

#ifdef _SIMULATION_
  {
    char name [PHB_MAX_TAG_LEN + 1];
    char number[MAX_PHB_NUM_LEN];

    memset (name, 0, PHB_MAX_TAG_LEN + 1);
    memcpy (name, entry->tag, entry->tag_len);
    cmhPHB_getAdrStr (number,
                      MAX_PHB_NUM_LEN - 1,
                      entry->number,
                      entry->len);
    if ((UBYTE)(name[0]) >= 0x80)
      strcpy (name, "<Some UCS2 coding>");

    TRACE_EVENT_P4 ("Adding number %s with name '%s' to book %d record %d",
                    number, name, type, phy_recno);
  }
#endif

  /* Fix phy_recno parameter for circular books */
  if ((type EQ LDN) OR (type EQ LMN) OR (type EQ LRN))
  {
    if (phy_recno EQ 0)
      phy_recno = 1;
  }

  switch (GET_PHB_STATE())
  {
    case PHB_STATE_IDLE:

      /* Add the data in the database */
      phb_result = pb_sim_add_record (type, 
                                      phy_recno, 
                                      entry, 
                                      &pba_data.records_changed);
      if (phb_result NEQ PHB_OK)
        return phb_result;  /* Record could not be added to the database */

      if (pba_data.records_changed.entries NEQ 0)
      {
        SET_PHB_STATE (PHB_STATE_WRITE);
      }

      /* Start synchronizing record(s) to the SIM */
      return pb_sync_next_sim_record (TRUE);

    case PHB_STATE_VERIFY:
    case PHB_STATE_READ:
      if ((type EQ LRN) OR (type EQ LMN))
      {
        /* 
         * In Release 1998- there is no counterpart on the SIM here,
         * we can write immediately as we don't have to synch to the SIM.
         */
        return (pb_sim_add_record (type, 
                                   phy_recno, 
                                   entry, 
                                   &dummy_changed));
      }
      else if (type EQ LDN)
      {
        /* We have to queue the LDN entry and will write later */
        return pb_write_queue (entry);
      }
      return PHB_FAIL;

    default:
      return PHB_FAIL;
  }
}


/*
+----------------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : PHB                                |
| STATE   : code                ROUTINE : pb_del_record                      |
+----------------------------------------------------------------------------+

  PURPOSE : This function deletes a phonebook entry.

*/
GLOBAL T_PHB_RETURN pb_del_record (T_PHB_TYPE type,
                                   USHORT phy_recno)
{
  T_PHB_RETURN phb_result;

  TRACE_FUNCTION ("pb_del_record()");

  if (GET_PHB_STATE() NEQ PHB_STATE_IDLE)
    return PHB_FAIL;

  /* Delete the data in the database */
  phb_result = pb_sim_del_record (type,
                                  phy_recno,
                                  &pba_data.records_changed);
  if (phb_result NEQ PHB_OK)
    return phb_result;  /* Record could not be deleted from the database */

  if (pba_data.records_changed.entries NEQ 0)
  {
    SET_PHB_STATE (PHB_STATE_WRITE);
  }

  /* Start synchronizing record(s) to the SIM */
  return pb_sync_next_sim_record (TRUE);
}


/*
+----------------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : PHB                                |
| STATE   : code                ROUTINE : pb_check_fdn                       |
+----------------------------------------------------------------------------+

  PURPOSE : This function checks whether a phone number is in FDN phonebook.

*/
#define RETURN(x) { retVal = x; goto cleanup_exit; }
/*lint -e{801} Use of goto*/
GLOBAL T_PHB_RETURN pb_check_fdn (UBYTE toa,
                                  const UBYTE *number)
{
  T_PHB_RETURN retVal;
  T_PHB_RECORD *phb_record;
  CHAR cur_number[MAX_PHB_NUM_LEN];
  UBYTE cur_num_len;
  USHORT phy_recno;
  SHORT max_rcd;
  SHORT dummy_used_rcd;
  UBYTE dummy_tag_len;
  SHORT dummy_max_ext, dummy_used_ext;

  ACI_MALLOC (phb_record, sizeof (T_PHB_RECORD));

  TRACE_FUNCTION ("pb_check_fdn()");

  retVal = pb_sim_read_sizes (FDN, 
                              &max_rcd, 
                              &dummy_used_rcd, 
                              &dummy_tag_len,
                              &dummy_max_ext,
                              &dummy_used_ext);
  if (retVal NEQ PHB_OK)
  {
    TRACE_ERROR ("Unexpected return from pb_sim_read_sizes()");
    RETURN (retVal)
  }
  
  /* 
   * Do a linear search through all the records applying the number comparision
   * rules which make us pass 51.010 test case 27.18.1.1.4.2.
   * Those rules are different from those which are to be applied if a matching
   * record has to be found for an incoming call, therefore we cannot use 
   * pb_sim_search_number() here.
   * As normally FDN phonebooks have not more than 10 entries (max_rcd) a 
   * simple linear search will fulfill here all timing requirements.
   */
  
  for (phy_recno = 1; phy_recno <= max_rcd; phy_recno++)
  {
    retVal = pb_sim_read_record (FDN, phy_recno, phb_record);
    if (retVal EQ PHB_OK)
    {
      cmhPHB_getAdrStr (cur_number,
                        MAX_PHB_NUM_LEN - 1,
                        phb_record->number,
                        phb_record->len);
      cur_num_len = strlen (cur_number);

      /* Compare number strings considering Wild card */ 
       if (pb_sim_cmpWild((char*)cur_number, (char *) number, cur_num_len) EQ 0)
      {
        /* Number matches. ACI-SPR-11927: check type of address */ 
        if ((toa EQ 0) OR
          (toa EQ phb_record->ton_npi) OR
          (phb_record->ton_npi EQ 0xFF))
        {
            RETURN (PHB_OK)
        }
        
      }
    }
  }
  RETURN (PHB_FAIL) /* Not found */

cleanup_exit:
  ACI_MFREE (phb_record);
  return retVal;
}


/*
+----------------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : PHB                                |
| STATE   : code                ROUTINE : pb_switch_adn_fdn                  |
+----------------------------------------------------------------------------+

  PURPOSE : This function switches FDN and ADN phonebooks (sAT_PlusCLCK()).

*/
GLOBAL T_PHB_RETURN pb_switch_adn_fdn (T_PHB_FDN_MODE mode,
                                       T_ACI_CLASS classFDN)
{
  TRACE_FUNCTION ("pb_switch_adn_fdn()");

  if (GET_PHB_STATE() NEQ PHB_STATE_IDLE)
    return PHB_FAIL;

  /* Check parameters */
  switch (mode)
  {
    case FDN_DISABLE:
    case FDN_ENABLE:
      break;

    default:
      return PHB_FAIL;
  }

  if (classFDN NEQ pba_data.fdn_classtype)
  {  
    pba_data.fdn_classtype = classFDN;

#ifndef _SIMULATION_
    if (ffs_fwrite("/mmi/fdnClassType", &classFDN, sizeof(UBYTE)) < 1)
    {
      TRACE_ERROR ("sAT_PlusCLCK: failed to write /mmi/fdnClassType");
    }
#endif /* #ifndef _SIMULATION_ */
  }

#if 1

  pba_data.fdn_mode = mode;

  /* Remove ADN from database */
  (void)pb_sim_remove_ef (SIM_ADN);
  pba_data.book_created[ADN] = FALSE;
  pba_data.book_read[ADN] = FALSE;

  /* Remove MSISDN database */
  (void)pb_sim_remove_ef (SIM_MSISDN);
  pba_data.book_created[UPN] = FALSE;
  pba_data.book_read[UPN] = FALSE;

#ifdef SIM_LND_SUPPORT
  /* Remove LND from database */
  (void)pb_sim_remove_ef (SIM_LND);
  pba_data.book_created[LDN] = FALSE;
  pba_data.book_read[LDN] = FALSE;
#endif

  /* Remove EXT1 from database */
  (void)pb_sim_remove_ef (SIM_EXT1);
  pba_data.ext_created[EXT1] = FALSE;
  
  /* Remove FDN from database */
  (void)pb_sim_remove_ef (SIM_FDN);
  pba_data.book_created[FDN] = FALSE;
  pba_data.book_read[FDN] = FALSE;

  /* Remove EXT2 from database */
  (void)pb_sim_remove_ef (SIM_EXT2);
  pba_data.ext_created[EXT2] = FALSE;

  /* Force cmhPHB_StatIndication (PHB_BUSY, CME_ERR_NotPresent) */
  pba_data.db_recreate = TRUE;

  SET_PHB_STATE (PHB_STATE_READ);

  /* Start reading changed books */
  pb_start_build (FALSE);
#else
  // This optimization does not comply with current test cases,
  // not understood perfectly whether the test cases can be
  // changed, so kept old behaviour.
  if (pba_data.fdn_mode NEQ mode)
  {
    pba_data.fdn_mode = mode;

    if (mode EQ FDN_ENABLE)
    {
      /* Remove ADN from database */
      (void)pb_sim_remove_ef (SIM_ADN);
      pba_data.book_created[ADN] = FALSE;
      pba_data.book_read[ADN] = FALSE;
    }
    else
    {
      /* Remove FDN from database */
      (void)pb_sim_remove_ef (SIM_FDN);
      pba_data.book_created[FDN] = FALSE;
      pba_data.book_read[FDN] = FALSE;

      /* Remove EXT2 from database */
      (void)pb_sim_remove_ef (SIM_EXT2);
      pba_data.ext_created[EXT2] = FALSE;
    }

    /* Force cmhPHB_StatIndication (PHB_BUSY, CME_ERR_NotPresent) */
    pba_data.db_recreate = TRUE;

    SET_PHB_STATE (PHB_STATE_READ);

    /* Start reading changed books */
    pb_start_build (FALSE);
  }
#endif
  return PHB_OK;
}


/*
+----------------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : PHB                                |
| STATE   : code                ROUTINE : pb_del_book                        |
+----------------------------------------------------------------------------+

  PURPOSE : This function deletes a whole SIM phonebook

*/
GLOBAL T_PHB_RETURN pb_del_book (T_PHB_TYPE book)
{
  T_PHB_RETURN phb_result;
  UBYTE max_record;

  TRACE_FUNCTION ("pb_del_book()");

  if (GET_PHB_STATE() NEQ PHB_STATE_IDLE)
    return PHB_FAIL;

  /* Remember book to delete, initialize record to delete */
  pba_data.current_book = book;
  pba_data.del_record = 1;
  max_record = pba_data.phb_record_max[pba_data.current_book];

  do
  {
    /* Delete the record in the database */
    phb_result = pb_sim_del_record (pba_data.current_book, 
                                    pba_data.del_record,
                                    &pba_data.records_changed);

    if ((phb_result NEQ PHB_OK) AND (phb_result NEQ PHB_EMPTY_RECORD))
      return phb_result;

    pba_data.del_record++;
  } 
  while ((pba_data.del_record <= max_record) AND 
         (pba_data.records_changed.entries EQ 0));

  if (pba_data.records_changed.entries NEQ 0)
  {
    /* Synchronize to SIM */
    SET_PHB_STATE (PHB_STATE_DELETE_BOOK);
  
    /* Start synchronizing record(s) to the SIM */
    return pb_sync_next_sim_record (TRUE);
  }

  return PHB_OK;
}


/*
+----------------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : PHB                                |
| STATE   : code                ROUTINE : pb_get_fdn_classtype               |
+----------------------------------------------------------------------------+

  PURPOSE : This function gets the FDN class type

*/
GLOBAL T_ACI_CLASS pb_get_fdn_classtype (void)
{
  TRACE_FUNCTION ("pb_get_fdn_classtype()");

  return pba_data.fdn_classtype;

}


/*
+----------------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : PHB                                |
| STATE   : code                ROUTINE : pb_get_fdn_mode                    |
+----------------------------------------------------------------------------+

  PURPOSE : This function gets the FDN mode

*/
GLOBAL T_PHB_FDN_MODE pb_get_fdn_mode (void)
{
  TRACE_FUNCTION ("pb_get_fdn_mode()");

  return pba_data.fdn_mode;

}


/*
 * Wrapper functions. Wrapper functions don't define own functionality 
 * except of routing to a different block which is the SIM part of 
 * the phonebook here. For a description of these functions have
 * a look into the SIM part of the phonebook.
 */

GLOBAL T_PHB_RETURN pb_read_record (T_PHB_TYPE type,
                                    SHORT phy_recno,
                                    T_PHB_RECORD *entry)
{
  T_PHB_RETURN phb_result;

  TRACE_FUNCTION ("pb_read_record()");

  if (phy_recno EQ 0)
    return PHB_FAIL;

  switch (GET_PHB_STATE())
  {
    case PHB_STATE_VERIFY:
      /* 
       * For LDN records, first have a look into the queue
       */
      if ((type EQ LDN) AND (phy_recno - 1 < pba_data.c_queued))
      {
        /* Entry freshly dialled and still in the queue */
        *entry = pba_data.queued[phy_recno - 1]->entry;
        return PHB_OK;
      }
      phb_result = pb_sim_read_record (type, 
                                       (USHORT)(phy_recno - pba_data.c_queued),
                                       entry);
      TRACE_EVENT_P3 ("type = %d, phy_recno = %d, phb_result = %d",
                      type, phy_recno, phb_result);
      return phb_result;

    case PHB_STATE_READ:
      /* 
       * For LDN records, first have a look into the queue
       */
      if ((type EQ LDN) AND (phy_recno - 1 < pba_data.c_queued))
      {
        /* Entry freshly dialled and still in the queue */
        *entry = pba_data.queued[phy_recno - 1]->entry;
        return PHB_OK;
      }
      return PHB_FAIL; /* We already know the SIM was changed */

    case PHB_STATE_IDLE:
      phb_result = pb_sim_read_record (type, phy_recno, entry);
      TRACE_EVENT_P3 ("type = %d, phy_recno = %d, phb_result = %d",
                      type, phy_recno, phb_result);
      return phb_result;

    default:
      return PHB_FAIL;
  }
}


GLOBAL T_PHB_RETURN pb_read_alpha_record (T_PHB_TYPE type,
                                          SHORT order_num,
                                          T_PHB_RECORD *entry)
{
  TRACE_FUNCTION ("pb_read_alpha_record()");

  switch (GET_PHB_STATE())
  {
    case PHB_STATE_IDLE:
    case PHB_STATE_VERIFY:
      /* Implements Measure #30 */
      return pb_sim_read_alpha_num_record (type, order_num, entry, NAME_IDX);
    default:
      return PHB_FAIL;
  }
}


GLOBAL T_PHB_RETURN pb_read_number_record (T_PHB_TYPE type,
                                           SHORT order_num,
                                           T_PHB_RECORD *entry)
{
  TRACE_FUNCTION ("pb_read_number_record()");

  switch (GET_PHB_STATE())
  {
    case PHB_STATE_IDLE:
    case PHB_STATE_VERIFY:
      /* Implements Measure #30 */
      return pb_sim_read_alpha_num_record (type, order_num, entry, NUMBER_IDX);
    default:
      return PHB_FAIL;
  }
}


GLOBAL T_PHB_RETURN pb_search_name (T_PHB_TYPE type,
                                    T_PHB_MATCH match,
                                    const T_ACI_PB_TEXT *search_tag,
                                    SHORT *order_num)
{
  T_PHB_RETURN phb_result;

  TRACE_FUNCTION ("pb_search_name()");

  switch (GET_PHB_STATE())
  {
    case PHB_STATE_IDLE:
    case PHB_STATE_VERIFY:
      TRACE_EVENT_P1 ("order_num = %d", *order_num);
      phb_result = pb_sim_search_name (type, match, search_tag, order_num);
      if (phb_result EQ PHB_OK)         /* #HM# Remove before production */
      {
        TRACE_EVENT ("Entry found");
      }
      else
      {
        TRACE_EVENT ("Entry not found");
      }
      return phb_result;

    default:
      return PHB_FAIL;
  }
}


GLOBAL T_PHB_RETURN pb_search_number (T_PHB_TYPE type,
                                      const UBYTE *number,
                                      SHORT *order_num)
{
  TRACE_FUNCTION ("pb_search_number()");

  if(type EQ ECC)
  {
    return pb_sim_search_number (type, number, order_num);
  }
  switch (GET_PHB_STATE())
  {
    case PHB_STATE_IDLE:
    case PHB_STATE_VERIFY:
      return pb_sim_search_number (type, number, order_num);
    default:
      return PHB_FAIL;
  }
}


/*
+----------------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : PHB                                |
| STATE   : code                ROUTINE : pb_get_max_num_len                 |
+----------------------------------------------------------------------------+

  PURPOSE : This function gets the maximum length of a number within a
            given phonebook. The length depends on the given phonebook and
            the availability of extension records (SIM service table).

            Question: The case that the extension records are present but 
            exhausted on the SIM is not considered here. Should this be done?

            Question: From an architectural point of view, belongs this into
            the ACI part of the phonebook or more into the SIM part of the 
            phonebook? SIM part until now has no information about SIM service
            table.

*/
LOCAL UBYTE pb_get_max_num_len (T_PHB_TYPE type)
{
  TRACE_FUNCTION ("pb_get_max_num_len()");

  switch (type)
  {
    case ECC:
      return 3;

    case ADN:
    case UPN:
    case ADN_ME:
      if (pb_sim_service (SRV_EXT1))
        return 2 * PHB_PACKED_NUM_LEN;
      break;

    case FDN:
      if (pb_sim_service (SRV_EXT2))
        return 2 * PHB_PACKED_NUM_LEN;
      break;

    case SDN:
      if (pb_sim_service (SRV_EXT3))
        return 2 * PHB_PACKED_NUM_LEN;
      break;
    
    case BDN:
      if (pb_sim_service (SRV_EXT4))
        return 2 * PHB_PACKED_NUM_LEN;
      break;

    case LDN:
    case LRN:
    case LMN:
    case ME:
      return 2 * PHB_PACKED_NUM_LEN;
    
    default:
      TRACE_ERROR ("Invalid phonebook type");
      return 0;
  }
  return 20; /* Default according to 11.11 if no EXT records and not ECC */
}


GLOBAL T_PHB_RETURN pb_read_sizes (T_PHB_TYPE type, /* IN */
                                   SHORT *max_rcd,  /* OUT */
                                   SHORT *used_rcd, /* OUT */
                                   UBYTE *num_len,  /* OUT */
                                   UBYTE *tag_len,  /* OUT */
                                   SHORT *max_ext,  /* OUT */
                                   SHORT *used_ext) /* OUT */
{
  T_PHB_RETURN phb_result;

  TRACE_FUNCTION ("pb_read_sizes()");

  if(type EQ ECC)
  {
    *num_len = pb_get_max_num_len (type);
    phb_result = pb_sim_read_sizes (type, max_rcd, used_rcd, tag_len, 
                                    max_ext, used_ext);
    if (phb_result NEQ PHB_OK)
    {
      *max_rcd  = 0;
      *used_rcd = 0;
      *num_len  = 0;
      *tag_len  = 0;
      *max_ext  = 0;
      *used_ext = 0;
    }
    return PHB_OK;
  }
  switch (GET_PHB_STATE())
  {
    case PHB_STATE_IDLE:
    case PHB_STATE_VERIFY:

      /* 
       * For a valid phonebook type, set all parameters to zero instead of 
       * reporting an error.
       */
      switch (type)
      {
        case ADN:
        case FDN:
        case BDN:
        case LDN:
        case LRN:
        case SDN:
        case LMN:
        case UPN:
      /*case ME:
        case ADN_ME:*/
          *num_len = pb_get_max_num_len (type);
          phb_result = pb_sim_read_sizes (type, max_rcd, used_rcd, tag_len, 
                                          max_ext, used_ext);
          if (phb_result NEQ PHB_OK)
          {
            *max_rcd  = 0;
            *used_rcd = 0;
            *num_len  = 0;
            *tag_len  = 0;
            *max_ext  = 0;
            *used_ext = 0;
          }
#ifdef _SIMULATION_
          TRACE_EVENT_P7 ("pb_read_sizes(): type=%d, "
                          "max_rcd=%d, used_rcd=%d, "
                          "num_len=%d, tag_len=%d, "
                          "max_ext=%d, used_ext=%d", 
                          type,
                          *max_rcd, *used_rcd, 
                          *num_len, *tag_len, 
                          *max_ext, *used_ext);
#endif /* #ifdef _SIMULATION_ */
          return PHB_OK;

        default: 
          break;
      }
      return PHB_FAIL;  /* Invalid phonebook */

    default:
      return PHB_FAIL;  /* Invalid state */
  }
}


GLOBAL int pb_get_entry_len (const UBYTE *pb_tag,
                             UBYTE max_pb_len)
{
  TRACE_FUNCTION ("pb_get_entry_len()");

  return pb_sim_get_entry_len (pb_tag, max_pb_len);
}


GLOBAL int pb_find_free_record (T_PHB_TYPE type)
{
  /* 
   * This function finds the number of the first free record in 
   * a given phonebook. This is new functionality as from S621.
   */
  TRACE_FUNCTION ("pb_find_free_record()");

  switch (GET_PHB_STATE())
  {
    case PHB_STATE_IDLE:
    case PHB_STATE_VERIFY:
      return pb_sim_find_free_record (type);
    default:
      return PHB_FAIL;
  }
}


GLOBAL int pb_find_ext_free_record ()
{
  /* 
   * This function finds the number of the first free record in 
   * a given phonebook. This is new functionality as from S621.
   */
  TRACE_FUNCTION ("pb_find_free_record()");

  switch (GET_PHB_STATE())
  {
    case PHB_STATE_IDLE:
    case PHB_STATE_VERIFY:
      return pb_sim_find_free_ext_record ();
    default:
      return PHB_FAIL;
  }
}

GLOBAL void  pb_update_ext_bitmap (UBYTE recu_num,BOOL flag)
{
    pb_sim_update_ext_bitmap(recu_num, flag); 
}





/* 
 * Internal functions, not part of the interface
 */



/*
+----------------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : PHB                                |
| STATE   : code                ROUTINE : pb_sim_service                     |
+----------------------------------------------------------------------------+

  PURPOSE : This function checks whether a given service is
            "allocated and activated" on the SIM, if so TRUE is returned, 
            otherwise the function returns FALSE.

            No, using psaSIM_ChkSIMSrvSup() is not a good alternative here
            as we may have to handle the situation where the SIM service 
            table itself has been changed, in this case we are not signalled
            by the ACI when we have again a valid SIM service table, therefore
            this function exist here within the phonebook.

*/
LOCAL BOOL pb_sim_service (UBYTE nr)
{
  UBYTE byte_offset;
  UBYTE bit_offset;
  UBYTE service;

  TRACE_FUNCTION ("pb_sim_service()");

  if (nr > MAX_SRV_TBL * 4)
    return FALSE; /* Table overflow */

  byte_offset =  (nr - 1) / 4;
  bit_offset  = ((nr - 1) & 0x03) * 2;
  service = (pba_data.sim_service_table[byte_offset] >> bit_offset) & 0x03;
  return (service EQ ALLOCATED_AND_ACTIVATED);
}


/*
+----------------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : PHB                                |
| STATE   : code                ROUTINE : pb_read_sim_record                 |
+----------------------------------------------------------------------------+

    PURPOSE : This function sends a SIM_READ_RECORD_REQ to the SIM card.
              On success this function returns TRUE, otherwise FALSE.

*/
LOCAL BOOL pb_read_sim_record (USHORT data_id, UBYTE rcd_num, UBYTE len)
{
  SHORT table_id;

  TRACE_FUNCTION ("pb_read_sim_record()");

  table_id = psaSIM_atbNewEntry();

  if (table_id NEQ NO_ENTRY)
  {
    /* Allocate room for the SIM data */
    if (pba_data.data EQ NULL)
    {
      ACI_MALLOC (pba_data.data, SIM_MAX_RECORD_SIZE);
    }
    simShrdPrm.atb[table_id].ntryUsdFlg = TRUE;
    simShrdPrm.atb[table_id].v_path_info  = FALSE;
    simShrdPrm.atb[table_id].accType    = ACT_RD_REC;
    simShrdPrm.atb[table_id].reqDataFld = data_id;
    simShrdPrm.atb[table_id].recNr      = rcd_num;
    simShrdPrm.atb[table_id].dataLen    = len;
    simShrdPrm.atb[table_id].exchData   = pba_data.data;
    simShrdPrm.atb[table_id].rplyCB     = pb_read_sim_record_cb;

    simShrdPrm.aId = table_id;

    if (psaSIM_AccessSIMData() < 0)
    {
      TRACE_EVENT("FATAL ERROR");
      return FALSE;
    }
    return TRUE;
  }

  return FALSE;
}


/*
+----------------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : PHB                                |
| STATE   : code                ROUTINE : pb_read_sim                        |
+----------------------------------------------------------------------------+

    PURPOSE : This function sends a SIM_READ_REQ to the SIM card.
              On success this function returns TRUE, otherwise FALSE.

*/
LOCAL BOOL pb_read_sim (USHORT data_id, UBYTE len, UBYTE max_length)
{
  SHORT table_id;

  TRACE_FUNCTION ("pb_read_sim()");

  table_id = psaSIM_atbNewEntry();

  if (table_id NEQ NO_ENTRY)
  {
    /* Allocate room for the SIM data */
    if (pba_data.data EQ NULL)
    {
      ACI_MALLOC (pba_data.data, SIM_MAX_RECORD_SIZE);
    }

    simShrdPrm.atb[table_id].ntryUsdFlg = TRUE;
    simShrdPrm.atb[table_id].accType    = ACT_RD_DAT;
    simShrdPrm.atb[table_id].v_path_info  = FALSE;
    simShrdPrm.atb[table_id].reqDataFld = data_id;
    simShrdPrm.atb[table_id].dataOff    = 0;
    simShrdPrm.atb[table_id].dataLen    = len;
    simShrdPrm.atb[table_id].recMax     = max_length;
    simShrdPrm.atb[table_id].exchData   = NULL;
    simShrdPrm.atb[table_id].rplyCB     = pb_read_sim_cb;

    simShrdPrm.aId = table_id;

    if (psaSIM_AccessSIMData() < 0)
    {
      TRACE_EVENT("FATAL ERROR");
      return FALSE;
    }
    return TRUE;
  }
  return FALSE;
}


/*
+----------------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : PHB                                |
| STATE   : code                ROUTINE : pb_read_next_sim_book              |
+----------------------------------------------------------------------------+

    PURPOSE : This function starts reading the phonebooks based on the 
              information whether the phonebook is activated and allocated
              and still has to be read at all.

*/
LOCAL void pb_read_next_sim_book (void)
{
  TRACE_FUNCTION("pb_read_next_sim_book()");

  /*
   * The next three phonebooks ADN, MSISDN and LND share EXT1 records
   */
  if ((pba_data.fdn_mode NEQ FDN_ENABLE) AND
      pb_sim_service (SRV_ADN) AND !pba_data.book_read[ADN])
  {
    /* Read ADN from SIM card */
    pba_data.current_book = ADN;
    pba_data.book_read[ADN] = TRUE;
    pb_read_sim_record (SIM_ADN, 1, NOT_PRESENT_8BIT);
    return;
  }

  if (pb_sim_service (SRV_MSISDN) AND !pba_data.book_read[UPN])
  {
    /* Read MSISDN from SIM card */
    pba_data.current_book = UPN;
    pba_data.book_read[UPN] = TRUE;
    pb_read_sim_record (SIM_MSISDN, 1, NOT_PRESENT_8BIT);
    return;
  }

#ifdef SIM_LND_SUPPORT
  if (pb_sim_service (SRV_LDN) AND !pba_data.book_read[LDN])
  {
    /* Read LND from SIM card */
    pba_data.current_book = LDN;
    pba_data.book_read[LDN] = TRUE;
    pb_read_sim_record (SIM_LND, 1, NOT_PRESENT_8BIT);
    return;
  }
#endif

#ifdef SIM_LND_SUPPORT
  if (!pb_sim_service (SRV_LDN) AND !pba_data.book_read[LDN])
#else
  if (!pba_data.book_read[LDN])
#endif
  {
    /* 
     * SIM card without LDN service. Create SIM_ICI and SIM_OCI here.
     * Don't create obsolete SIM_LND
     */
    pba_data.book_read[LDN] = TRUE;

    /* Create now also some helper elementary files as of R99+ */
    /* (void)pb_sim_create_ef (SIM_ICI, SIZE_EF_ICI, NR_EF_ICI); */
    (void)pb_sim_create_ef (FFS_LRN, SIZE_EF_LRN, NR_EF_LRN);
    (void)pb_sim_create_ef (FFS_EXT_LRN, SIZE_EF_EXT_LRN, NR_EF_EXT_LRN);
    (void)pb_sim_create_ef (FFS_LMN, SIZE_EF_LMN, NR_EF_LMN);
    (void)pb_sim_create_ef (FFS_EXT_LMN, SIZE_EF_EXT_LMN, NR_EF_EXT_LMN);

    (void)pb_sim_create_ef (SIM_OCI, SIZE_EF_OCI, NR_EF_OCI);
    (void)pb_sim_create_ef (SIM_EXT5, SIZE_EF_EXT5, NR_EF_EXT5);

    pba_data.phb_record_max[LDN] = NR_EF_OCI;
    pba_data.phb_record_len[LDN] = SIZE_EF_OCI;
    pba_data.book_created[LDN] = TRUE;

    /* pba_data.phb_record_max[LRN] = NR_EF_ICI;  Shared! */
    /* pba_data.phb_record_len[LRN] = SIZE_EF_ICI; */
    pba_data.phb_record_max[LRN] = NR_EF_LRN;
    pba_data.phb_record_len[LRN] = SIZE_EF_LRN;
    pba_data.book_created[LRN] = TRUE;

    /* pba_data.phb_record_max[LMN] = NR_EF_ICI;  Shared! */
    /* pba_data.phb_record_len[LMN] = SIZE_EF_ICI; */
    pba_data.phb_record_max[LMN] = NR_EF_LMN;
    pba_data.phb_record_len[LMN] = SIZE_EF_LMN;
    pba_data.book_created[LMN] = TRUE;

    pba_data.ext_record_max[EXT5]    = NR_EF_EXT5;
    pba_data.ext_record_max[EXT_LRN] = NR_EF_EXT_LRN;
    pba_data.ext_record_max[EXT_LMN] = NR_EF_EXT_LMN;

    pba_data.ext_record_max[EXT5]    = SIZE_EF_EXT5;
    pba_data.ext_record_max[EXT_LRN] = SIZE_EF_EXT_LRN;
    pba_data.ext_record_max[EXT_LMN] = SIZE_EF_EXT_LMN;
  }

#ifdef SIM_LND_SUPPORT
  if (pba_data.book_read[ADN] OR
      pba_data.book_read[UPN] OR
      pba_data.book_read[LDN])
#else
  if (pba_data.book_read[ADN] OR
      pba_data.book_read[UPN])
#endif
  {
    /* At least one of the books referencing EXT1 has been read */
    if (!pba_data.ext_created[EXT1] AND
        pb_sim_service (SRV_EXT1))
    {
      /* Reading to get the number of records only */
      pba_data.ext_dummy_read = TRUE;
      pb_read_sim_record (SIM_EXT1, 1, NOT_PRESENT_8BIT);
      return;
    }
  }

  /*
   * FDN phonebook only to be read if ADN is blocked
   */
  if ((pba_data.fdn_mode EQ FDN_ENABLE) AND 
      pb_sim_service (SRV_FDN) AND !pba_data.book_read[FDN])
  {
    /* Read FDN from SIM card */
    pba_data.current_book = FDN;
    pba_data.book_read[FDN] = TRUE;
    pb_read_sim_record (SIM_FDN, 1, NOT_PRESENT_8BIT);
    return;
  }

  if (pba_data.book_read[FDN] AND
      !pba_data.ext_created[EXT2] AND
      pb_sim_service (SRV_EXT2))
  {
    /* Reading to get the number of records only */
    pba_data.ext_dummy_read = TRUE;
    pb_read_sim_record (SIM_EXT2, 1, NOT_PRESENT_8BIT);
    return;
  }

  if (pb_sim_service (SRV_BDN) AND !pba_data.book_read[BDN])
  {
    /* Read BDN from SIM card */
    pba_data.current_book = BDN;
    pba_data.book_read[BDN] = TRUE;
    pb_read_sim_record (SIM_BDN, 1, NOT_PRESENT_8BIT);
    return;
  }

  if (pba_data.book_read[BDN] AND 
      !pba_data.ext_created[EXT4] AND
      pb_sim_service (SRV_EXT4))
  {
    /* Reading to get the number of records only */
    pba_data.ext_dummy_read = TRUE;
    pb_read_sim_record (SIM_EXT4, 1, NOT_PRESENT_8BIT);
    return;
  }

  if (pb_sim_service (SRV_SDN) AND !pba_data.book_read[SDN])
  {
    /* Read SDN from SIM card */
    pba_data.current_book = SDN;
    pba_data.book_read[SDN] = TRUE;
    pb_read_sim_record (SIM_SDN, 1, NOT_PRESENT_8BIT);
    return;
  }

  if (pba_data.book_read[SDN] AND
      !pba_data.ext_created[EXT3] AND
      pb_sim_service (SRV_EXT3))
  {
    /* Reading to get the number of records only */
    pba_data.ext_dummy_read = TRUE;
    pb_read_sim_record (SIM_EXT3, 1, NOT_PRESENT_8BIT);
    return;
  }

  /* Update the CPHS extension1 records*/
	 pb_update_cphs_mb_ext_record(); 



  /*
   * For the phonebook entries which reside on SIM but don't have 
   * timestamps some checks could be done here.
   * For example, we could read based on EF_LDN, subsequently we
   * could read the informations solely stored in the flash and
   * then we could decide whether we through away the last dialled 
   * numbers or don't.
   * Probably this is overkill, old Nokia 5110 and more recent Nokia 6330i 
   * simply does not support EF_LND at all but stores the last dialled 
   * numbers in the flash.
   */

#ifdef SIM_TOOLKIT
  if (simShrdPrm.fuRef >= 0)
  {
    psaSAT_FUConfirm (simShrdPrm.fuRef, SIM_FU_SUCC_ADD);
  }
#endif

  /* Free SIM exchange data */
  if (pba_data.data NEQ NULL)
  {
    ACI_MFREE (pba_data.data);
    pba_data.data = NULL;
  }

  SET_PHB_STATE (PHB_STATE_IDLE);

  if (pb_read_queue() EQ PHB_EXCT)
    return; /* Queued LDN entry is written */

  /* Finished reading phonebooks. Flush data. */
  (void)pb_sim_flush_data ();

  /* Inform ACI that we're ready */
  pb_status_ind (PHB_READY, CME_ERR_NotPresent);
}


/*
+----------------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : PHB                                |
| STATE   : code                ROUTINE : pb_read_sim_record_cb              |
+----------------------------------------------------------------------------+

    PURPOSE : This function is the callback when a SIM record has been read
              (SIM_READ_RECORD_CNF).

*/
#define PHB_INCREMENTAL_SORT // Only locally and temporary, HM 18-Oct-2006
LOCAL void pb_read_sim_record_cb (SHORT table_id)
{
  T_PHB_RETURN phb_result;  /* Phonebook result code */
  T_PHB_TYPE phb_type;      /* Phonebook type */
  T_EXT_TYPE ext_type;      /* Extension record type */
  T_PHB_TYPE next_phb_type; /* Phonebook type of record to read */
  T_EXT_TYPE next_ext_type; /* Extension record type of record to read */
  USHORT  reqDataFld;       /* requested datafield identifier */
  UBYTE   recNr;            /* record number */
  UBYTE   dataLen;          /* Length of data */
  UBYTE   recMax;           /* Maximum number of records */
  USHORT  errCode;          /* Error code from the SIM */
  USHORT  ext_record_ef;    /* Extension record elementary file id */
  UBYTE   ext_record_no;    /* Extension record number */
  BOOL    changed;          /* Record changed by pb_sim_write_ef() */
  BOOL    write_record;     /* Write the record */

  TRACE_FUNCTION ("pb_read_sim_record_cb()");

  /* 
   * Currently we have a limitation of 254 records in the code.
   * This is because there is a note in 11.11 version 8.3.0 clause 6.4.2
   * that there is a limit of 254 records for linear fixed.
   * Same for USIM looking into 31.102 Annex G.
   * However this constraint is not valid for cyclic, and also this
   * note has not been present in 11.11 version 7.4.0 clause 6.2.4, 
   * there is a limit of 255 records mentioned.
   * Not supporting the 255th record is a minor issue for now.
   * To avoid crashing, we have to limit here for now to 254.
   * This is not a theoretical issue only as there exists SIMs
   * which have 255 ADN entries.
   */
  if (simShrdPrm.atb[table_id].recMax EQ 255)
    simShrdPrm.atb[table_id].recMax = 254;

  /* Get the important information from the read record from T_SIM_ACC_PRM */
  reqDataFld = simShrdPrm.atb[table_id].reqDataFld;
  recNr      = simShrdPrm.atb[table_id].recNr;
  dataLen    = simShrdPrm.atb[table_id].dataLen;
  recMax     = simShrdPrm.atb[table_id].recMax;
  errCode    = simShrdPrm.atb[table_id].errCode;

  phb_type = pb_get_phb_type_from_ef (reqDataFld);
  ext_type = pb_get_ext_type_from_ef (reqDataFld);

  if (phb_type EQ INVALID_PHB AND
      ext_type EQ INVALID_EXT)
  {
    TRACE_ERROR ("Invalid reqDataFld!");
    simShrdPrm.atb[table_id].ntryUsdFlg = FALSE;
    return;
  }

  /* We assume we write the record until we have certain errors */
  write_record = TRUE;

  if (errCode NEQ SIM_NO_ERROR)
  {
    TRACE_EVENT_P1 ("SIM indicated problem code %04x", errCode);
    /* 
     * At least use cleanly deleted data instead of trash.
     * Do memset only if pba_data.data is not equal to NULL.       
     */
    if (pba_data.data NEQ NULL)
    {
      memset (pba_data.data, 0xff, SIM_MAX_RECORD_SIZE);
    }
    else
    {
      TRACE_EVENT("PHB data has been cleared");
      simShrdPrm.atb[table_id].ntryUsdFlg = FALSE;
      return;
    }
  }

  if (ext_type NEQ INVALID_EXT) 
  {
    /*
     * Got an extension record. 
     */
    if (!pba_data.ext_created[ext_type])
    {
      if (errCode NEQ SIM_NO_ERROR)
      {
        /* We won't create a field on invalid data and won't write */
        write_record = FALSE;
        
        /* 
         * When SIM returns an error case when tried to read Extension
         * file, then set the ext_created field to TRUE so that further 
         * reading of Phonebook continues 
         */
        pba_data.ext_created[ext_type] = TRUE;
      }
      else
      {
        pba_data.ext_record_max[ext_type] = recMax;
        pba_data.ext_record_len[ext_type] = dataLen;

        /* Shadow field for extension record not yet created */
        (void)pb_sim_create_ef (reqDataFld, dataLen, recMax);
        pba_data.ext_created[ext_type] = TRUE;
      }

      if (pba_data.ext_dummy_read)
      {
        /* 
         * We are reading the first extension record which is assumed 
         * to be not used just to get the number of respective extension 
         * records.
         */
        write_record = FALSE;
        pba_data.ext_dummy_read = FALSE;
      }
    }
  }
  else
  {
    /*
     * Got a normal phonebook record
     */
    if (!pba_data.book_created[phb_type])
    {
      if (errCode NEQ SIM_NO_ERROR)
      {
        /* We won't create a field on invalid data */
        write_record = FALSE;
      }
      else
      {
        pba_data.phb_record_max[phb_type] = recMax;
        pba_data.phb_record_len[phb_type] = dataLen;

        /* Shadow field for normal phonebook record not yet created */
        (void)pb_sim_create_ef (reqDataFld, dataLen, recMax);
        pba_data.book_created[phb_type] = TRUE;

#ifdef SIM_LND_SUPPORT
        if (phb_type EQ LDN)
        {
          /* Create now also some helper elementary files as of R99+ */
          (void)pb_sim_create_ef (SIM_OCI, (USHORT)(dataLen + 14), recMax);
          pba_data.phb_record_max[LDN] = recMax;
          pba_data.phb_record_len[LDN] = dataLen;
          pba_data.book_created[LDN] = TRUE;

          (void)pb_sim_create_ef (SIM_ICI, (USHORT)(dataLen + 14), recMax);
          pba_data.phb_record_max[LRN] = recMax;
          pba_data.phb_record_len[LRN] = dataLen;
          pba_data.book_created[LRN] = TRUE;

          pba_data.phb_record_max[LMN] = recMax;
          pba_data.phb_record_len[LMN] = dataLen;
          pba_data.book_created[LMN] = TRUE;
        }
#endif
      }
    }
  }

  /* 
   * Write raw record. The SIM layer is aware about the data structures
   * and gives us the information whether we have to read an/another
   * extension record now or whether we can continue reading normal
   * phonebook entries.
   */
  if (write_record)
  {
    phb_result = pb_sim_write_ef(reqDataFld,
                                 recNr,
                                 dataLen,
                                 pba_data.data,
                                 &changed,
                                 &ext_record_ef,
                                 &ext_record_no);
  }
  else
  {
    /*
     * New field, record was trash, don't write it, continue reading
     * phonebook entries.
     */
    TRACE_EVENT ("Record not written (trash/dummy ext)");

    phb_result = PHB_OK;
    changed = FALSE;
    ext_record_ef = 0;
    ext_record_no = 0;
  }

  /* Mark SIM table as free now */
  simShrdPrm.atb[table_id].ntryUsdFlg = FALSE;

  if ((GET_PHB_STATE() EQ PHB_STATE_VERIFY) AND
      (phb_result EQ PHB_OK) AND changed)
  {
    /* We have a change detected. Prevent further reading until all read */
    SET_PHB_STATE (PHB_STATE_READ);
  }

  /*
   * Here an optimization is possible. This is, if an extension record
   * and already has been read we could skip reading the extension record
   * here. The optimization is not worth the effort, so we simply don't do
   * it.
   */

  /*
   * Do some protection against garbage
   */
  if (ext_record_ef NEQ 0)
  {
    /* Current record references an extension record */
    next_ext_type = pb_get_ext_type_from_ef (ext_record_ef);

    if ((next_ext_type EQ INVALID_EXT) OR
        (ext_record_no < 1) OR
        (ext_record_no > 254))
    {
      TRACE_ERROR ("pb_sim_write_ef() for ext delivered garbage EXT record");
      ext_record_ef = 0; /* Try to proceed without this garbage */
    }
    else if (ext_type NEQ INVALID_EXT)
    {
      /* We have read an extension record */
      if (pba_data.read_ext_record EQ NULL)
      {
        /* 1st extension record in the chain */
        ACI_MALLOC (pba_data.read_ext_record, pba_data.ext_record_max[ext_type]);
        memset (pba_data.read_ext_record, FALSE, pba_data.ext_record_max[ext_type]);
      }

      /* Mark current extension record as read */
      pba_data.read_ext_record[recNr - 1] = TRUE;
     
      if (pba_data.read_ext_record[ext_record_no - 1] EQ TRUE)
      {
        /* We've already read the next extension record */
        TRACE_ERROR ("EXT record already seen");
        ext_record_ef = 0; /* Mark as garbage */
      }
    }
  }
  
  if (ext_record_ef NEQ 0)
  {
    /* Current record references an extension record */
    next_ext_type = pb_get_ext_type_from_ef (ext_record_ef);

    /* 
     * We have to read an extension record.
     */
    if (phb_type NEQ INVALID_PHB) // Patch HM 21-Sep-2006
    {
      /*
       * It's the first extension record. Remember the paused phonebook.
       */
      TRACE_EVENT_P4 ("PHB EF %04x record %d references EXT EF %04x record %d",
                      reqDataFld,
                      recNr,
                      ext_record_ef,
                      ext_record_no);
      pba_data.paused_ef = reqDataFld;
      pba_data.paused_no = recNr;
    }

    /* Read the extension record. */
    if (pba_data.ext_created[next_ext_type])
    {
      (void)pb_read_sim_record (ext_record_ef,
                                ext_record_no, 
                                pba_data.ext_record_len[next_ext_type]);
    }
    else
    {
      (void)pb_read_sim_record (ext_record_ef,
                                ext_record_no, 
                                NOT_PRESENT_8BIT);
    }
  }
  else if (ext_type NEQ INVALID_EXT)
  {
    /* 
     * The record read was the last extension record in the chain.
     * Come back reading normal phonebook entries.
     */

    /* Free memory allocated for 
     * tracking EXT records in chain 
     * since EXT reading is over */
    if(pba_data.read_ext_record NEQ NULL)
    {
      ACI_MFREE(pba_data.read_ext_record);
      pba_data.read_ext_record = NULL;
    }

     if (!pba_data.ext_dummy_read)
    {
      next_phb_type = pb_get_phb_type_from_ef (pba_data.paused_ef);
      if (pba_data.paused_no < pba_data.phb_record_max[next_phb_type])
      {
        /* Continue reading the current phonebook */
        (void)pb_read_sim_record (pba_data.paused_ef,
                                  (UBYTE)(pba_data.paused_no + 1),
                                  pba_data.phb_record_len[next_phb_type]);

#ifdef PHB_INCREMENTAL_SORT
        /* Sort while reading the next record from the SIM */
        (void)pb_sim_build_index (next_phb_type);
#endif

      }
      else
      {
        (void)pb_sim_build_index (next_phb_type); /* Sort last entry of this book */

        /* Next book */
        pb_read_next_sim_book ();
      }
    }
    else
    {
      /* Next book after having read dummy extension record */
      pba_data.ext_dummy_read = FALSE;
      pb_read_next_sim_book ();
    }
    pba_data.paused_ef  = 0;
    pba_data.paused_no  = 0;

  }
  else if (simShrdPrm.atb[table_id].recNr < simShrdPrm.atb[table_id].recMax)
  {
    /*
     * Normal phonebook record read, no extension referenced and not
     * finished. Continue reading the current phonebook 
     */
    

    (void)pb_read_sim_record (simShrdPrm.atb[table_id].reqDataFld,
                              (UBYTE)(simShrdPrm.atb[table_id].recNr + 1),
                              simShrdPrm.atb[table_id].dataLen);
#ifdef PHB_INCREMENTAL_SORT
    /* Sort while reading the next record from the SIM */
    (void)pb_sim_build_index (phb_type);
#endif

  }
  else
  {
    /*
     * Normal phonebook record read, no extension referenced and finished
     * reading this book. Continue reading the next book.
     */
    (void)pb_sim_build_index (phb_type); /* Sort last entry of this book */

    /* Next book */
    pb_read_next_sim_book ();
  }
}
#undef PHB_INCREMENTAL_SORT // Only locally and temporary, HM 18-Oct-2006


/*
+----------------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : PHB                                |
| STATE   : code                ROUTINE : pb_sat_update_reset                |
+----------------------------------------------------------------------------+

    PURPOSE : This function marks the given elementary file as unread.
    
*/
LOCAL void pb_sat_update_reset (USHORT ef)
{
  T_PHB_TYPE phb_type;

  TRACE_FUNCTION ("pb_sat_update_reset()");

  phb_type = pb_get_phb_type_from_ef (ef);

  if (phb_type NEQ INVALID_PHB)
  {
    /* Mark book as unread */
    pba_data.book_read[phb_type] = FALSE;
  }
}


/*
+----------------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : PHB                                |
| STATE   : code                ROUTINE : pb_read_sim_cb                     |
+----------------------------------------------------------------------------+

    PURPOSE : This function is the callback when a SIM elementary file has 
              been read (SIM_READ_CNF).
              There are two fields which can have been read here, those are
              the emergency call numbers and the SIM service table.

*/


LOCAL void pb_read_sim_cb (SHORT table_id)
{
  TRACE_FUNCTION ("pb_read_sim_cb()");

  switch (simShrdPrm.atb[table_id].reqDataFld)
  {
    case SIM_ECC:
      if (simShrdPrm.atb[table_id].errCode EQ SIM_NO_ERROR)
      {
        (void)pb_sim_set_ecc (simShrdPrm.atb[table_id].dataLen,
                              simShrdPrm.atb[table_id].exchData);

#ifdef SIM_TOOLKIT
        if (simShrdPrm.fuRef >= 0)
        {
          psaSAT_FUConfirm (simShrdPrm.fuRef, SIM_FU_SUCCESS);
        }
#endif /* #ifdef SIM_TOOLKIT */

      }        
      else
      {
        /* 
         * Some error from the SIM. Indicate a fallback to the PCM/FFS.
         */
        (void)pb_sim_set_ecc (0, NULL);

#ifdef SIM_TOOLKIT
        if (simShrdPrm.fuRef >= 0)
        {
          psaSAT_FUConfirm (simShrdPrm.fuRef, SIM_FU_ERROR);
        }
#endif

      }

      if (pba_data.data NEQ NULL)
      {
        ACI_MFREE (pba_data.data);
        pba_data.data = NULL;
      }
      break;

    case SIM_SST:
      if (simShrdPrm.atb[table_id].errCode EQ SIM_NO_ERROR)
      {
        /* copy SIM service table */
        memset(pba_data.sim_service_table, 0, MAX_SRV_TBL);
        memcpy(pba_data.sim_service_table, 
               simShrdPrm.atb[table_id].exchData, 
               MINIMUM(MAX_SRV_TBL, simShrdPrm.atb[table_id].dataLen));

        /* start reading SIM phonebook */
        simShrdPrm.atb[table_id].ntryUsdFlg = FALSE;
        pb_start_build (FALSE);
      }
      else
      {
        TRACE_ERROR ("Cannot read SIM service table, using old table");

#ifdef SIM_TOOLKIT
        if (simShrdPrm.fuRef >= 0)
        {
          psaSAT_FUConfirm (simShrdPrm.fuRef, SIM_FU_ERROR);
        }
#endif

        /* start reading SIM phonebook (fallback action, best effort) */
        simShrdPrm.atb[table_id].ntryUsdFlg = FALSE;
        pb_start_build (FALSE);
      }
      break;

    default:
      TRACE_ERROR ("Unexpected SIM_READ_CNF");
      break;
  }
  simShrdPrm.atb[table_id].ntryUsdFlg = FALSE;
}


/*
+----------------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : PHB                                |
| STATE   : code                ROUTINE : pb_is_sim_field                    |
+----------------------------------------------------------------------------+

    PURPOSE : This function determines wheter an elementary file has to 
              be synchronized to the SIM or only exists locally in the FFS.

*/
LOCAL BOOL pb_is_sim_field (USHORT ef)
{
  TRACE_FUNCTION ("pb_is_sim_field()");

  switch (ef)
  {
    /* case SIM_ICI: */
    case FFS_LRN:
    case FFS_LMN:
    case FFS_EXT_LRN:
    case FFS_EXT_LMN:
    case SIM_OCI:
    case SIM_EXT5:
      return FALSE; /* Only in FFS */

    default:
      return TRUE;  /* Counterpart in SIM exists */
  }
}


/*
+----------------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : PHB                                |
| STATE   : code                ROUTINE : pb_sync_next_sim_record            |
+----------------------------------------------------------------------------+

    PURPOSE : This function synchronizes a changed record to the SIM.
              
              The first parameter is given to determine whether it's the first
              record to be synchronized, in this case ACI has to be informed.
  
              If there was no record to synchronize, PHB_OK is returned.
              If there is a record to synchronize, PHB_EXCT is returned.
              If there was an internal failure we get PHB_FAIL 
              (which is unexpected).

*/

LOCAL T_PHB_RETURN pb_sync_next_sim_record (BOOL first)
{
  T_PHB_RETURN phb_result;
  USHORT field_id;
  USHORT record;
  USHORT entry_size;

  TRACE_FUNCTION ("pb_sync_next_sim_record()");

  do
  {
    if (pba_data.records_changed.entries EQ 0)
    {
      if (pb_read_queue() EQ PHB_EXCT)
        return PHB_EXCT; /* Queued LDN entry is written */

      /* Inform SIM layer we're done synchronizing */
      (void)pb_sim_flush_data ();

      /* Don't change the state if phb_state is PHB_STATE_DELETE_BOOK
       * since state will be changed in pb_write_sim_record_cb after
       * all the records are deleted */
      if(GET_PHB_STATE() NEQ PHB_STATE_DELETE_BOOK)
      {
        SET_PHB_STATE (PHB_STATE_IDLE);
      
        /* Inform ACI that we're ready */
        pb_status_ind (PHB_READY, CME_ERR_NotPresent);
      }

      return PHB_OK; /* No record to synchronize to the SIM */
    }

    /* Get the next record from the list */
    pba_data.records_changed.entries--;
    field_id = pba_data.records_changed.field_id[0];
    record   = pba_data.records_changed.record[0];

    /* Remove the record from the list */
    memmove (&pba_data.records_changed.field_id[0],
             &pba_data.records_changed.field_id[1],
             sizeof (USHORT) * (DB_MAX_AFFECTED - 1));
    memmove (&pba_data.records_changed.record[0],
             &pba_data.records_changed.record[1],
             sizeof (USHORT) * (DB_MAX_AFFECTED - 1));
  } 
  while (!pb_is_sim_field(field_id));

  TRACE_EVENT_P3 ("Synch ef %04X record %d, still %d entries to do",
                  field_id, record, pba_data.records_changed.entries);

  /* Allocate room for the SIM data  #HM# Check for free */
  if (pba_data.data EQ NULL)
  {
    ACI_MALLOC (pba_data.data, SIM_MAX_RECORD_SIZE);
  }

  /* Fetch record in raw form from the database */
  phb_result = pb_sim_read_ef (field_id,
                               record,
                               &entry_size,
                               pba_data.data);

  if (phb_result EQ PHB_OK)
  {
    /* And write it to the SIM */
    phb_result = pb_write_sim_record (field_id,
                                      (UBYTE)record, 
                                      (UBYTE)entry_size,
                                      pba_data.data);

    if (phb_result NEQ PHB_EXCT)
    {
      /* Inform ACI that we've had a problem */
      pb_status_ind ( PHB_WRITE_FAIL, CME_ERR_NotPresent );
      return phb_result; /* Some unexpected internal failure */
    }

    if (first)
    {
      /* Inform ACI that we're busy synchronizing SIM entries */
      pb_status_ind ( PHB_BUSY, CME_ERR_NotPresent );
    }

    return PHB_EXCT;
  }
  return phb_result;
}


/*
+----------------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : PHB                                |
| STATE   : code                ROUTINE : pb_write_sim_record                |
+----------------------------------------------------------------------------+

    PURPOSE : This function writes a SIM record to the SIM.

*/

LOCAL T_PHB_RETURN pb_write_sim_record (USHORT ef,
                                        UBYTE phy_recno,
                                        UBYTE entry_size,
                                  const UBYTE *buffer)
{
  SHORT           table_id;


  TRACE_FUNCTION("pb_write_sim_record()");

  table_id = psaSIM_atbNewEntry();
  if (table_id EQ NO_ENTRY)
  {
    TRACE_ERROR ("pb_write_sim_record(): no more table entries");
    return (PHB_FAIL);
  }

  simShrdPrm.atb[table_id].ntryUsdFlg = TRUE;
  simShrdPrm.atb[table_id].accType    = ACT_WR_REC;
  simShrdPrm.atb[table_id].v_path_info  = FALSE;
  simShrdPrm.atb[table_id].reqDataFld = ef;
  simShrdPrm.atb[table_id].recNr      = phy_recno;
  simShrdPrm.atb[table_id].dataLen    = entry_size;
  simShrdPrm.atb[table_id].exchData   = (UBYTE *)buffer;
  simShrdPrm.atb[table_id].rplyCB     = pb_write_sim_record_cb;

  simShrdPrm.aId = table_id;

  if (psaSIM_AccessSIMData() < 0)
  {
    TRACE_EVENT("pb_write_sim_record(): psaSIM_AccessSIMData() failed");
    return (PHB_FAIL);
  }

  return (PHB_EXCT);
}


/*
+----------------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : PHB                                |
| STATE   : code                ROUTINE : pb_write_sim_record_cb             |
+----------------------------------------------------------------------------+

    PURPOSE : This function is the callback when a SIM record has been
              written to the SIM.

*/

LOCAL void pb_write_sim_record_cb (SHORT table_id)
{
  UBYTE max_record;
  T_PHB_RETURN phb_result;

  TRACE_FUNCTION("pb_write_sim_record_cb()");

  simShrdPrm.atb[table_id].ntryUsdFlg = FALSE;

  if (simShrdPrm.atb[table_id].errCode NEQ SIM_NO_ERROR)
  {
    /* 
     * Data in the database and on the SIM probably now differ,
     * we can do not much about this.
     */
    TRACE_ERROR("pb_write_cb(): error for writing");
    pb_status_ind (PHB_WRITE_FAIL,
                   (SHORT)cmhSIM_GetCmeFromSim (simShrdPrm.atb[table_id].errCode));
    return;
  }

  switch (GET_PHB_STATE())
  {
    case PHB_STATE_WRITE:
      /* Synchronize next SIM record */
      switch (pb_sync_next_sim_record (FALSE))
      {
        case PHB_OK:
          /* PHB_READY has been already indicated in pb_sync_next_sim_record */
          break;

        case PHB_EXCT:
          break; /* Next record => SIM */

        case PHB_FAIL:
        default:
          TRACE_ERROR ("Unexpected phb_result");      
          SET_PHB_STATE (PHB_STATE_IDLE);
          pb_status_ind (PHB_WRITE_FAIL, CME_ERR_NotPresent);
          break;
      }
      break;

    case PHB_STATE_DELETE_BOOK:
      /* Synchronize next SIM record */
      switch (pb_sync_next_sim_record (FALSE))
      {
        case PHB_OK:
          /* 
           * Record synched. Delete next record of phonebook, if available
           */
          max_record = pba_data.phb_record_max[pba_data.current_book];
          while ((pba_data.del_record <= max_record) AND
                 (pba_data.records_changed.entries EQ 0))
          {
            /* Delete the record in the database */
            phb_result = pb_sim_del_record (pba_data.current_book, 
                                   pba_data.del_record,
                                   &pba_data.records_changed); 
            
            if ((phb_result NEQ PHB_OK) AND (phb_result NEQ PHB_EMPTY_RECORD))
            {
              TRACE_ERROR ("Unexpected problem from pb_sim_del_record()");

              /* Inform SIM layer we're done synchronizing.
               * Should we indicate also there was a problem? */
              (void)pb_sim_flush_data ();

              pb_status_ind (PHB_WRITE_FAIL, CME_ERR_NotPresent);

              SET_PHB_STATE (PHB_STATE_IDLE);
              return;
            }
            pba_data.del_record++;
          } 

          if (pba_data.records_changed.entries NEQ 0)
          {
            /* Synchronizing record(s) to the SIM */
            if (pb_sync_next_sim_record (FALSE) NEQ PHB_EXCT)
            {
              TRACE_ERROR ("Unexpected problem from pb_sync_next_sim_record()");

              /* Inform SIM layer we're done synchronizing.
               * Should we indicate also there was a problem? */
              (void)pb_sim_flush_data ();

              pb_status_ind (PHB_WRITE_FAIL, CME_ERR_NotPresent);

              SET_PHB_STATE (PHB_STATE_IDLE);
              return;
            }
          }

          /* 
           * We're through
           */
          if(pba_data.del_record > max_record)
          {
            (void)pb_sim_flush_data ();

            SET_PHB_STATE (PHB_STATE_IDLE);
	    
            /* Inform the ACI that we're done */
            pb_status_ind (PHB_READY, CME_ERR_NotPresent);
          }
          break;

        case PHB_EXCT:
          break; /* Next record => SIM */

        case PHB_FAIL:
        default:
          TRACE_ERROR ("Unexpected problem synching SIM");      

          (void)pb_sim_flush_data ();

          pb_status_ind (PHB_WRITE_FAIL, CME_ERR_NotPresent);
          /* Inform SIM layer we're done synchronizing.
           * Should we indicate also there was a problem? */

          SET_PHB_STATE (PHB_STATE_IDLE);
          break;
      }
      break;

    default:
      TRACE_ERROR ("Unexpected default");
      break;
  }
}


/*
+----------------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : PHB                                |
| STATE   : code                ROUTINE : pb_get_phb_type_from_ef            |
+----------------------------------------------------------------------------+

  PURPOSE : This function converts a SIM elementary file number to the 
            appropriate constant of type T_PHB_TYPE, if applicable.
            Non applicable => INVALID_PHB

*/
T_PHB_TYPE pb_get_phb_type_from_ef (USHORT ef)
{
  TRACE_FUNCTION ("pb_get_phb_type_from_ef()");

  switch (ef)
  {
    case SIM_ECC:
      return ECC;

    case SIM_ADN:
      return ADN;

    case SIM_FDN:
      return FDN;

    case SIM_BDN:
      return BDN;

    case SIM_LND:
      return LDN;

    case SIM_MSISDN:
      return UPN;

    case SIM_SDN:
      return SDN;

    default:
      return INVALID_PHB;
  }
}

 
/*
+----------------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : PHB                                |
| STATE   : code                ROUTINE : pb_get_ext_type_from_ef            |
+----------------------------------------------------------------------------+

  PURPOSE : This function converts a SIM elementary file number to the
            appropriate constant of T_PHB_EXT_TYPE, if applicable.
            Non applicable => INVALID_EXT

*/
T_EXT_TYPE pb_get_ext_type_from_ef (USHORT ef)
{
  TRACE_FUNCTION ("pb_get_ext_type_from_ef()");

  switch (ef)
  {
    case SIM_EXT1:
      return EXT1;

    case SIM_EXT2:
      return EXT2;

    case SIM_EXT3:
      return EXT3;

    case SIM_EXT4:
      return EXT4;

    case SIM_EXT5:
      return EXT5;

    default:
      return INVALID_EXT;
  }
}


/* ------------------------------------------------------------------------ */

/*
+----------------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : PHB                                |
| STATE   : code                ROUTINE : pb_search_number_ex                |
+----------------------------------------------------------------------------+

  PURPOSE : At some places a more sohisticated number searching function 
            is desirable which can be based on existing phonebook 
            functionality.

*/
GLOBAL T_PHB_RETURN pb_search_number_ex (T_PHB_TYPE type,
                                         const UBYTE *number,
                                         SHORT *order_num,
                                         SHORT *found,
                                         T_PHB_RECORD *entry)
{
  T_PHB_RETURN phb_result;
  SHORT index;

  TRACE_FUNCTION ("pb_search_number_ex()");

  /* Search for first entry */
  phb_result = pb_search_number (type,
                                 number,
                                 order_num);
  if (phb_result NEQ PHB_OK)
    return phb_result;

  /* 
   * Determine number of found entries. We are doing a linear algorithm, 
   * a binary algorithm is possible here to speed up things.
   */
  if (found NEQ NULL)
  {
    *found = 1;
    index = *order_num;

    while (pb_search_number (type, number, &index) EQ PHB_OK)
    { 
      ++(*found);
      /* index should not be incremented as lower layer (DB) will take
         care of it */
    }
  }

  /* Read the first found entry, if desired */
  if (entry NEQ NULL)
    return (pb_read_number_record (type, *order_num, entry));
  return PHB_OK;
}


/*
+----------------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : PHB                                |
| STATE   : code                ROUTINE : pb_search_name_ex                  |
+----------------------------------------------------------------------------+

  PURPOSE : At some places a more sohisticated name searching function 
            is desirable which can be based on existing phonebook 
            functionality.

*/
GLOBAL T_PHB_RETURN pb_search_name_ex (T_PHB_TYPE type,
                                       T_PHB_MATCH match,
                                       const T_ACI_PB_TEXT *search_tag,
                                       SHORT *order_num,
                                       SHORT *found,
                                       T_PHB_RECORD *entry)
{
  T_PHB_RETURN phb_result;
  SHORT index;

  TRACE_FUNCTION ("pb_search_name_ex()");

  /* Search for first entry */
  phb_result = pb_search_name (type,
                               match,
                               search_tag,
                               order_num);
  if (phb_result NEQ PHB_OK)
    return phb_result;

  /* 
   * Determine number of found entries. We are doing a linear algorithm, 
   * a binary algorithm is possible here to speed up things.
   */
  if (found NEQ NULL)
  {
    *found = 1;
    index = *order_num;

    while (pb_search_name (type, match, search_tag, &index) EQ PHB_OK)
    { 
      ++(*found);
      /* index should not be incremented as lower layer (DB) will take
         care of it */
    }
      
  }

  /* Read the first found entry, if desired */
  if (entry NEQ NULL)
    return (pb_read_alpha_record (type, *order_num, entry));
  return PHB_OK;
}


/*
+----------------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : PHB                                |
| STATE   : code                ROUTINE : pb_status_ind                      |
+----------------------------------------------------------------------------+

  PURPOSE : This function indicates the new phonebook status to the ACI 
            preventing we're indication twice PHB_READY or PHB_BUSY.

*/
LOCAL void pb_status_ind (T_PHB_STAT phb_stat, SHORT cmeError)
{
  TRACE_FUNCTION ("pb_status_ind()");

  /* Avoid to indicate twice busy or ready */
  if ((pba_data.phb_stat NEQ phb_stat) OR
      ((phb_stat NEQ PHB_READY) AND
       (phb_stat NEQ PHB_BUSY)))
  {
    pba_data.phb_stat = phb_stat;
    cmhPHB_StatIndication (phb_stat, cmeError,TRUE);
  }
}


/*
+----------------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : PHB                                |
| STATE   : code                ROUTINE : pb_write_queue                     |
+----------------------------------------------------------------------------+

  PURPOSE : This function writes a LDN entry to the queue if the SIM is busy.

*/
LOCAL T_PHB_RETURN pb_write_queue (const T_PHB_RECORD *entry)
{
  TRACE_FUNCTION ("pb_write_queue()");

  if (pba_data.c_queued < PHB_MAX_QUEUE)
  {
    memmove (&pba_data.queued[1],
             &pba_data.queued[0],
             pba_data.c_queued * sizeof (T_PHB_QUEUE *));
    ACI_MALLOC (pba_data.queued[pba_data.c_queued], 
                sizeof (T_PHB_QUEUE));
    pba_data.queued[0]->entry = *entry;
    pba_data.c_queued++;
    return PHB_EXCT;
  }
  
  return PHB_FAIL; /* Queue full */
}


/*
+----------------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : PHB                                |
| STATE   : code                ROUTINE : pb_clear_queue                     |
+----------------------------------------------------------------------------+

  PURPOSE : This function clears the LDN queue.

*/
LOCAL void pb_clear_queue (void)
{
  TRACE_FUNCTION ("pb_clear_queue()");

  while (pba_data.c_queued NEQ 0)
  {
    pba_data.c_queued--;
    ACI_MFREE (pba_data.queued[pba_data.c_queued]);
    pba_data.queued[pba_data.c_queued] = NULL;
  }
}


/*
+----------------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)       MODULE  : PHB                                |
| STATE   : code                ROUTINE : pb_read_queue                      |
+----------------------------------------------------------------------------+

  PURPOSE : This function reads the queue and adds the queued phonebook 
            entries.

*/
LOCAL T_PHB_RETURN pb_read_queue (void)
{
  T_PHB_RETURN phb_result;

  TRACE_FUNCTION ("pb_write_queue()");

  while (pba_data.c_queued NEQ 0)
  {
    /*
     * LDN entries have been queued. Write them now.
     */
    pba_data.c_queued--;
    phb_result = pb_add_record (LDN,
                                pba_data.queued[pba_data.c_queued]->index,
                                &pba_data.queued[pba_data.c_queued]->entry);
    ACI_MFREE (pba_data.queued[pba_data.c_queued]);
    pba_data.queued[pba_data.c_queued] = NULL;
    switch (phb_result)
    {
      case PHB_EXCT:
        return phb_result;
      case PHB_OK:
        break;
      default:
        pb_clear_queue();
        return phb_result;
    }
  }
  return PHB_OK;
}

/*
+------------------------------------------------------------------+
| PROJECT : MMI-Framework (8417)         MODULE: PHB               |
| STATE   : code                         ROUTINE : pb_status_req |
+------------------------------------------------------------------+

  PURPOSE : To update status of Phonebook to MMI

*/
void pb_status_req(UBYTE *pb_stat)
{
 
  *pb_stat = pba_data.phb_stat;
}

#endif /* #ifdef TI_PS_FFS_PHB */
