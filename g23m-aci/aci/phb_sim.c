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

#include "phb_sim.h"
#include "phb_aci.h"

#include "ffs/ffs.h"

#ifdef SIM_TOOLKIT
#include "psa.h"
#include "psa_sim.h"
#include "psa_cc.h"
#include "psa_sat.h"
#endif /* #ifdef SIM_TOOLKIT */

#include "cmh.h"
#include "cmh_phb.h"
#include "pcm.h"

/*
 * Constants and enumerations
 */
 
#define MAX_ECC_RCD                5
#define FFS_IMSI_SIZE              8
#define MAX_EXTNS_PER_RECORD       9
#define MAX_ELEM_FILES            15
#define SIM_MAX_RECORD_SIZE      256 /* Maximum size of a SIM record */
#define RDM_DATA_FILE_ID      0xff04 /* File ID to store data related to LDN, LMN and LRN Phonebooks */
#define SIZE_DATA_DATE_TIME       12
#define MAX_EXT_RECORDS           10
#define PHB_PHY_NUM_LENGTH        10


/*
 * Type definitions
 */
 
typedef struct
{
	/* Handle for the Database */ 
  int db_handle;

  /* Maximum number of records */
  UBYTE max_record[MAX_PHONEBOOK];

  /* Number of used records */
  UBYTE used_record[MAX_PHONEBOOK];

  /* Records sizes */
  USHORT record_size[MAX_PHONEBOOK];
 
} T_PHB_SIM_DATA;

/* Maximum no of records for LRN, LDN and LMN Phonebooks. */
#define RDM_PHB_DATA_SIZE      10

/* Record numbers for LDN, LMN and LRN Phonebook data. */
#define LDN_DATA_RECNO         1
#define LMN_DATA_RECNO         2
#define LRN_DATA_RECNO         3

EXTERN UBYTE cphs_mb_ext_record_num[]; 
/* ECC records */
T_PHB_ECC_RECORD phb_ecc_element[MAX_ECC_RCD];

LOCAL T_PHB_SIM_DATA pbs_data;

/* Array to hold Reference count for Extension records. */
LOCAL UBYTE ext_ref_count[MAX_PHB_EXT][MAX_EXT_RECORDS];

/* Prototypes for search and compare functions */
int pb_sim_search_alpha_func(ULONG flags, const UBYTE *key, int db_handle, USHORT field_id, USHORT rec_num);
int pb_sim_search_num_func(ULONG flags, const UBYTE *key, int db_handle, USHORT field_id, USHORT rec_num);
int pb_sim_alpha_cmp (int db_handle, USHORT field_id, USHORT recno_1, USHORT recno_2, ULONG flags);
int pb_sim_number_cmp (int db_handle, USHORT field_id,USHORT recno_1,USHORT recno_2, ULONG flags);
/* 
 * Prototypes for local functions
 */

LOCAL BOOL pb_sim_record_empty (USHORT field_id, 
                                USHORT entry_size, const UBYTE *buffer);
LOCAL void pb_sim_read_eeprom_ecc (void);
LOCAL int pb_sim_nibblecopy (UBYTE dest[], int destlen, UBYTE src[], int count);
LOCAL void pb_sim_revString(char *);
LOCAL void pb_sim_read_ext(UBYTE *buffer, T_PHB_RECORD *entry);
LOCAL void pb_sim_prepare_ext_data(UBYTE *ext_data, int ext_count, UBYTE *number, UBYTE no_len, UBYTE *subaddr);
LOCAL USHORT pb_sim_get_field_id (T_PHB_TYPE type);
LOCAL USHORT pb_sim_get_ext_file (T_PHB_TYPE type);
LOCAL USHORT pb_sim_get_ext_file_id (USHORT field_id);
LOCAL T_EXT_TYPE pb_sim_get_ext_type (USHORT field_id);
LOCAL USHORT pb_sim_get_size_except_tag (USHORT field_id);

LOCAL int pb_sim_cmpString ( UBYTE* cur_tag, UBYTE* check_tag, UBYTE cmpLen );
LOCAL void pb_sim_cvt_alpha_for_cmp ( UBYTE* entry_tag, UBYTE* cur_tag, UBYTE len );
LOCAL int pb_sim_cmp2Bytes(UBYTE *s1, UBYTE *s2, UBYTE len, UBYTE flag);
LOCAL T_PHB_RETURN pb_sim_update_extn_records(USHORT ext_field_id, USHORT rec_num, SHORT ref_type);
LOCAL T_PHB_RETURN pb_sim_del_ext_records(T_PHB_TYPE type, USHORT field_id, USHORT db_recno);
LOCAL USHORT pb_sim_retrieve_rdm_recno (T_PHB_TYPE type);
LOCAL T_PHB_RETURN pb_sim_del_record_internal (T_PHB_TYPE type,
                                               USHORT phy_recno,
                                               T_DB_CHANGED *rec_affected,
                                               BOOL replacement);
LOCAL BOOL pb_sim_ext_records_used (T_PHB_TYPE type, 
                                    USHORT field_id, 
                                    USHORT db_recno);


LOCAL T_PHB_RETURN pb_sim_update_index (T_PHB_TYPE    type, 
                                        T_DB_CHANGED *rec_affected,
                                        USHORT        field_id,
                                        UBYTE         ext_rec_cnt);

/*
+----------------------------------------------------------------------+
| PROJECT: MMI-Framework                MODULE : PHB                   |
| STATE  : code                         ROUTINE: pb_sim_init       |
+----------------------------------------------------------------------+

  PURPOSE :   Initializes internal data structures for SIM Phonebook.
*/

void pb_sim_init (void)
{
  USHORT i;
  
  TRACE_FUNCTION ("pb_sim_init()");
  db_init();

  /* Initialise the data structures. */

  /* Initialise ECC Phonebook to contain no records. */
  for(i = 0; i < MAX_ECC_RCD; i++)
  {
    phb_ecc_element[i].phy_idx = 0;
    memset(phb_ecc_element[i].number,0xFF,ECC_NUM_LEN);
  }

  return;
}

/*
+----------------------------------------------------------------------+
| PROJECT: MMI-Framework                MODULE : PHB                   |
| STATE  : code                         ROUTINE: pb_sim_exit       |
+----------------------------------------------------------------------+

  PURPOSE :   This function is called by pb_exit() to inform the SIM part 
              of the phonebook to shut down.
*/
void pb_sim_exit (void)
{
  TRACE_FUNCTION ("pb_sim_exit()");

  db_exit();

  return;
}

/*
+----------------------------------------------------------------------+
| PROJECT: MMI-Framework                MODULE : PHB                   |
| STATE  : code                         ROUTINE: pb_sim_record_empty   |
+----------------------------------------------------------------------+

  PURPOSE :   This function is used to determine if a record is
              considered as empty.
*/
LOCAL BOOL pb_sim_record_empty (USHORT field_id, 
                                USHORT entry_size, const UBYTE *data)
{
  USHORT alpha_len;

  /* TRACE_FUNCTION ("pb_sim_record_empty()"); */ /* Trace load */

  /* If the entry contains an alpha identifier it is not empty */
  if (data[0] NEQ 0xFF)
    return FALSE;

  alpha_len = entry_size - pb_sim_get_size_except_tag (field_id);

//TISH for MMISIM sometime entry_size = 0, then alpha_len < 0;
#ifdef WIN32
  if (entry_size == 0)
  {
	  return TRUE;
  }
#endif
  if ((data[alpha_len] NEQ 0x00) AND (data[alpha_len + 2] NEQ 0xff))
    return FALSE;
  return TRUE; /* No alpha identifier and no phone number => record empty */
}


/*
+----------------------------------------------------------------------+
| PROJECT: MMI-Framework                MODULE : PHB                   |
| STATE  : code                         ROUTINE: pb_sim_set_ecc       |
+----------------------------------------------------------------------+

  PURPOSE :   The emergency call numbers are read from SIM card and 
              written to FFS.
*/
T_PHB_RETURN pb_sim_set_ecc (UBYTE ecc_len, const UBYTE *sim_ecc)
{
  USHORT ecc_rec_num;
  UBYTE  *data_ptr;
  
  TRACE_FUNCTION ("pb_sim_set_ecc()");

  /* Initialise used records for ECC. */ 
  pbs_data.used_record[ECC] = 0;
  
  /* if SIM ECC data is not empty, copy SIM ECC data to phonebook */
  if ( ecc_len NEQ 0)
  {
    data_ptr         = ( UBYTE *) sim_ecc;
    pbs_data.max_record[ECC] = (SHORT)((ecc_len/ECC_NUM_LEN) > MAX_ECC_RCD)? MAX_ECC_RCD: ecc_len/ECC_NUM_LEN;
    pbs_data.record_size[ECC] = ECC_NUM_LEN;

    /* Store ECC into RAM, since ECC records will be less in number. */
    /*
     * For the Issue OMAPS00081622
     * Replaced MAX_ECC_RCD with pbs_data.max_record[ECC] in the for loop
     */
    for (ecc_rec_num = 0; ecc_rec_num < pbs_data.max_record[ECC]; ecc_rec_num++)
    {
      if(*data_ptr NEQ 0xff)
      {
        memset(&phb_ecc_element[ecc_rec_num],0xFF,sizeof(T_PHB_ECC_RECORD));
        phb_ecc_element[ecc_rec_num].phy_idx = ecc_rec_num + 1;
        memcpy(phb_ecc_element[ecc_rec_num].number, data_ptr, ECC_NUM_LEN);
        data_ptr += ECC_NUM_LEN;
        (pbs_data.used_record[ECC])++;
      }
    }
  }
  else
  {
    pb_sim_read_eeprom_ecc();
  }

  return PHB_OK;
}

/*
+--------------------------------------------------------------------+
| PROJECT: MMI-Framework (8417)         MODULE: PHB                  |
| STATE  : code                         ROUTINE: pb_sim_create_ef          |
+--------------------------------------------------------------------+

    PURPOSE : Creates a SIM elementary file. 

*/
T_PHB_RETURN pb_sim_create_ef  (USHORT ef, USHORT record_size, USHORT records)
{
  T_DB_INFO_FIELD field_info;
  T_DB_TYPE db_type;
  int db_result;

  TRACE_FUNCTION ("pb_sim_create_ef()");

  TRACE_EVENT_P1("Elementary file ID = %x",ef);
  db_result = db_info_field(pbs_data.db_handle, ef, &field_info);

  /* Check whether file already exists. */
  if(db_result EQ DB_OK)
  {
    /* Check for Record size and No. of records in the present field. */
    if((field_info.record_size EQ record_size) AND (field_info.num_records EQ records))
      return PHB_OK;  /* Preserve the existing field. */
    else
    {  
      if(pb_sim_remove_ef(ef) EQ PHB_FAIL)  /* Remove the existing file and recreate the field. */
				return PHB_FAIL;
    }
  }
  
	/* Set DB_TYPE depending on the Elementary file. */ 
  switch(ef)
  {
    case SIM_ADN:
    case SIM_FDN:          
    case SIM_BDN:  
    case SIM_SDN:
    case SIM_EXT1:
    case SIM_EXT2:
    case SIM_EXT3:
    case SIM_EXT4:
    case SIM_LND:         
    case SIM_OCI:
    //case SIM_ICI:
    case FFS_LRN:
    case FFS_LMN:
    case FFS_EXT_LRN:
    case FFS_EXT_LMN:
    case SIM_EXT5:
      db_type = DB_FREELIST;
      break;
        
    case SIM_MSISDN:          
    case SIM_IMSI:
      db_type = DB_UNMANAGED;
      break;

    default:
      TRACE_ERROR("Invalid ef passed to pb_sim_create_ef()");
      return PHB_FAIL;        
  }

  db_result = db_create_field(pbs_data.db_handle, db_type, ef, record_size, records);

  if(db_result EQ DB_OK)
    return PHB_OK;

  /* Return PHB_FAIL since DB has failed to create File.  */
  return PHB_FAIL;
}

/*
+--------------------------------------------------------------------+
| PROJECT: MMI-Framework (8417)         MODULE: PHB                  |
| STATE  : code                         ROUTINE: pb_sim_write_ef     |
+--------------------------------------------------------------------+

    PURPOSE : Writes entry_size bytes content of buffer at index to the elementary file ef.

*/
T_PHB_RETURN pb_sim_write_ef (USHORT ef, USHORT phy_recno,
                                         USHORT entry_size, const UBYTE *buffer,
                                         BOOL *changed, USHORT *ext_record_ef, UBYTE  *ext_record_no)
{
  int i;
  T_DB_CHANGED records_affected;

  TRACE_FUNCTION ("pb_sim_write_ef()");

  /* Initialise changed to FALSE. */
  *changed = FALSE;

  /* Default is no extension record */
  *ext_record_ef = 0;
  *ext_record_no = 0;

  /* Check for extension records. */
  if (!pb_sim_record_empty(ef, entry_size, buffer))
  {
    /* The record is not empty (deleted) */
    switch (ef)
    {
      case SIM_ADN:
      case SIM_MSISDN:
      case SIM_LND:
        *ext_record_ef = SIM_EXT1;
        *ext_record_no = buffer[entry_size - 1];
        break;

      case SIM_FDN:
        *ext_record_ef = SIM_EXT2;
        *ext_record_no = buffer[entry_size - 1];
        break;

      case SIM_SDN:
        *ext_record_ef = SIM_EXT3;
        *ext_record_no = buffer[entry_size - 1];
        break;

      case SIM_BDN:
        *ext_record_ef = SIM_EXT4;
        *ext_record_no = buffer[entry_size - 1];
        break;

      case FFS_LRN:
      case FFS_LMN:
      case SIM_OCI: /* Release 1999 and above 31.102 clause 4.2.34 */
        *ext_record_ef = SIM_EXT5;
        *ext_record_no = buffer[entry_size - 15]; // Jirli, please check, 14 instead of 15?
        break;

      case SIM_EXT1:  /* Extension records can reference other extension records */
      case SIM_EXT2:
      case SIM_EXT3:
      case SIM_EXT4:
      case SIM_EXT5:
      case FFS_EXT_LRN:
      case FFS_EXT_LMN:
//TISH, patch for OMAPS00124850
#if 0
        *ext_record_ef = ef;
        *ext_record_no = buffer[entry_size - 1];
#else
	if (buffer[entry_size - 1] EQ 255 OR buffer[entry_size-1] EQ (UBYTE)phy_recno)
	{
		 *ext_record_ef =0;
		 *ext_record_no=0;
	}
	else
	{
	        *ext_record_ef = ef;
	        *ext_record_no = buffer[entry_size - 1];	
	}
#endif
//end
        break;

      default: /* No extension record defined for this field */
        break;
    }

    /* Record is not referring any extensions. So set ef and record_no to ZERO. */
    if (*ext_record_no EQ 0xff)
    {
      *ext_record_ef = 0;
      *ext_record_no = 0;
    }

    /* Handle usage counter of extension records */
    pb_sim_update_extn_records (*ext_record_ef, *ext_record_no, 1);

    /* Write record into FFS */
    if(db_write_record(pbs_data.db_handle, ef, phy_recno, 0, entry_size, buffer) > DB_OK)
    {
      if(db_read_change_log(pbs_data.db_handle, &records_affected) EQ DB_OK)
      {
        for(i = 0; i < records_affected.entries; i++)
        {
          /* Checking whether Elementary file in the database is changed. */
          if((records_affected.field_id[i] EQ ef) AND (records_affected.record[i] EQ  phy_recno))
          {
            *changed = TRUE;
            return PHB_OK;
          }
        }

        /* Write operation has not changed File in the database. So returning PHB_OK */
        return PHB_OK;
      }
      else    /* Unable to read change log from DB. So returning PHB_FAIL. */
        return PHB_FAIL;
    }
    else   /* Write failure in DB. So returning PHB_FAIL */
      return PHB_FAIL;
  }
  else
  {
    /* Empty record */
    if (db_delete_record (pbs_data.db_handle, ef, phy_recno) NEQ DB_OK)
      return PHB_FAIL;

    if(db_read_change_log (pbs_data.db_handle, &records_affected) NEQ DB_OK)
      return PHB_FAIL;

    *changed = (records_affected.entries NEQ 0);
    return PHB_OK;
  }
}


/*
+--------------------------------------------------------------------+
| PROJECT: MMI-Framework (8417)         MODULE: PHB                  |
| STATE  : code                         ROUTINE: pb_sim_open          |
+--------------------------------------------------------------------+

    PURPOSE : Opens the SIM phonebook for the given SIM determined by the IMSI.
*/
T_PHB_RETURN pb_sim_open (const T_imsi_field *imsi_field, BOOL *changed)
{
  T_DB_INFO database_info;
  UBYTE ffsIMSI[MAX_IMSI_LEN+1];
  UBYTE simIMSI[MAX_IMSI_LEN+1];
  UBYTE imsi[FFS_IMSI_SIZE];
  int db_result,rec_num;
  UBYTE buffer[RDM_PHB_DATA_SIZE];

  TRACE_FUNCTION("pb_sim_open()");

  /* Initially set SIM changed as TRUE. */
  *changed = TRUE;
   
  /* Open the database. */
  db_result = db_open(FFS_PHB_DIR);

  TRACE_EVENT_P1("DB handle is %d",db_result);

  if(db_result >= DB_OK)
  {
    pbs_data.db_handle = db_result;

    /* Get database info. */
    db_result = db_info(pbs_data.db_handle, &database_info);

    /* Read IMSI from the FFS if Database is clean. */
    if((db_result EQ DB_OK) AND (database_info.clean EQ TRUE))
    {
      db_result = db_read_record(pbs_data.db_handle, SIM_IMSI, 1, 0, FFS_IMSI_SIZE, imsi);

      /* Compare IMSI read from FFS with IMSI got from SIM. */
      if(db_result > DB_OK)
      { 
        psaSIM_decodeIMSI ((UBYTE*) imsi_field->field,(UBYTE)imsi_field->c_field, (char *)simIMSI);

        psaSIM_decodeIMSI (imsi, FFS_IMSI_SIZE, (char *)ffsIMSI);

        if (!strcmp((char *)simIMSI, (char *)ffsIMSI))
        { 
          *changed = FALSE;
          return PHB_OK;
        }
      }
      else
      {
        /* Unable to read IMSI, regenerate database */
        *changed = TRUE;
      }
    }

    /* Remove database whenever database is Inconsistent and SIM is changed. */
    if(db_close(pbs_data.db_handle) NEQ DB_OK)
      return PHB_FAIL;

    if(db_remove(FFS_PHB_DIR) NEQ DB_OK)
      return PHB_FAIL;
  }/*   if(db_result >= DB_OK) */

  /* Create database: For the first time, whenever SIM is changed 
              and whenever database is Inconsistent. */
  db_result = db_create(FFS_PHB_DIR, MAX_ELEM_FILES, TRUE);

  TRACE_EVENT_P1("DB handle is %d",db_result);

  /* Creating DB is successful and valid db_handle is returned */
  if(db_result >= DB_OK)  
  {
    if(db_create_field(pbs_data.db_handle, DB_UNMANAGED, SIM_IMSI, imsi_field->c_field, 1) NEQ DB_OK)
      return PHB_FAIL;  

    if(db_write_record(pbs_data.db_handle, SIM_IMSI, 1, 0, imsi_field->c_field, imsi_field->field) < DB_OK)
      return PHB_FAIL;

    /* Create Elementary file to store RDM Phonebook data */
    if(db_create_field(pbs_data.db_handle, DB_UNMANAGED, RDM_DATA_FILE_ID,RDM_PHB_DATA_SIZE,3) NEQ DB_OK)
      return PHB_FAIL;
    
    /* Initialise data for Recent call lists */
    for(rec_num = 1; rec_num <= 3; rec_num++)
    {
    memset(buffer,0x00, RDM_PHB_DATA_SIZE); 

      if(db_write_record(pbs_data.db_handle, RDM_DATA_FILE_ID, rec_num, 0, RDM_PHB_DATA_SIZE, buffer) < DB_OK)
      return PHB_FAIL;
    }
      
    return PHB_OK;
  }

  /* Unable to create Database. So returning PHB_FAIL. */ 
  return PHB_FAIL;
}

/*
+--------------------------------------------------------------------+
| PROJECT: MMI-Framework (8417)         MODULE: PHB                  |
| STATE  : code                         ROUTINE: pb_sim_read_ef          |
+--------------------------------------------------------------------+

    PURPOSE : Reads *buffer from elementary file ef at index.
*/
T_PHB_RETURN pb_sim_read_ef (USHORT ef, USHORT recno, USHORT *entry_size, UBYTE *buffer)
{
  int db_result;
  T_DB_INFO_FIELD field_info;

  TRACE_FUNCTION("pb_sim_read_ef()");

  if(db_info_field(pbs_data.db_handle, ef, &field_info) EQ DB_OK)
  {
    *entry_size = field_info.record_size;

    db_result = db_read_record (pbs_data.db_handle,
                                ef,
                                recno,
                                0,
                                field_info.record_size,
                                buffer);

    if (db_result > DB_OK)
      return PHB_OK;  /* Successfully read */

    if (db_result EQ DB_EMPTY_RECORD)
    {
      /* Return a deleted record content */
      memset (buffer, 0xff, *entry_size);
      return PHB_OK;
    }
    return PHB_FAIL; /* Some problem reading record */
  }

  /* Returning PHB_FAIL, since DB has failed to give Info about the field. */ 
  return PHB_FAIL;
}

/*
+--------------------------------------------------------------------+
| PROJECT: MMI-Framework (8417)         MODULE: PHB                  |
| STATE  : code                         ROUTINE: pb_sim_remove_ef          |
+--------------------------------------------------------------------+

    PURPOSE : Removes elementary file.
*/
T_PHB_RETURN pb_sim_remove_ef (USHORT ef)
{
  T_EXT_TYPE ext_type;
  USHORT rec_num;

  TRACE_FUNCTION("pb_sim_remove_ef()");

  if(db_remove_field(pbs_data.db_handle, ef) EQ DB_OK)
  
  {
    /* Get EXT Type for elementary file */
    ext_type = pb_sim_get_ext_type(ef);

    /* Reset reference count for extension records */
    if(ext_type NEQ INVALID_EXT)
    {
      for(rec_num = 0; rec_num < MAX_EXT_RECORDS; rec_num++)
      {
          ext_ref_count[ext_type][rec_num] = 0;
      }
    }
    return PHB_OK;
  }
   
  return PHB_FAIL;
}

/*
+--------------------------------------------------------------------+
| PROJECT: MMI-Framework (8417)         MODULE: PHB                  |
| STATE  : code                         ROUTINE: pb_sim_build_index          |
+--------------------------------------------------------------------+

    PURPOSE : Builds index for the given phonebook.
*/

T_PHB_RETURN pb_sim_build_index (T_PHB_TYPE type)
{
  USHORT field_id;
  
  TRACE_FUNCTION("pb_sim_build_index()");

  field_id = pb_sim_get_field_id(type);

  if(db_update_index(pbs_data.db_handle, field_id, NAME_IDX, pb_sim_alpha_cmp, PHB_MATCH_PARTIAL) NEQ DB_OK)
    return PHB_FAIL;

  if(db_update_index(pbs_data.db_handle, field_id, NUMBER_IDX, pb_sim_number_cmp, PHB_MATCH_PARTIAL) NEQ DB_OK)
    return PHB_FAIL;
  
  return PHB_OK;
}
  
/*
+--------------------------------------------------------------------+
| PROJECT: MMI-Framework (8417)         MODULE: PHB                  |
| STATE  : code                         ROUTINE: pb_sim_flush_data          |
+--------------------------------------------------------------------+

    PURPOSE : This function informs the SIM phonebook that SIM reading has become finished and we reached a consistent state.
*/

T_PHB_RETURN pb_sim_flush_data  (void)
{

  TRACE_FUNCTION("pb_sim_flush_data()");

  if(db_flush(pbs_data.db_handle) EQ DB_OK)
    return PHB_OK;

  return PHB_FAIL;
}

/*
+--------------------------------------------------------------------+
| PROJECT: MMI-Framework (8417)         MODULE: PHB                  |
| STATE  : code                         ROUTINE: pb_sim_add_record   |
+--------------------------------------------------------------------+

    PURPOSE : Add a given record at index to the given phonebook.
*/

T_PHB_RETURN pb_sim_add_record  (T_PHB_TYPE type, 
                                 USHORT phy_recno, 
                           const T_PHB_RECORD *entry,
                                 T_DB_CHANGED *rec_affected)
{
  USHORT field_id, ext_file_id, rec_no;
  UBYTE    tag_len, max_tag_len;
  UBYTE   data[SIM_MAX_RECORD_SIZE];
  UBYTE   buffer[RDM_PHB_DATA_SIZE];
  T_DB_INFO_FIELD info_field, ext_info_field;
  UBYTE ext_rec_cnt = 0;
  UBYTE ext_rec_num1, ext_rec_num2;
  BOOL  record_empty;
  int db_result,i;

  TRACE_FUNCTION("pb_sim_add_record()");

  /* Handling of ECC Phonebook */
  if(type EQ ECC)
  {
    if((phy_recno > 0 ) AND (phy_recno <= MAX_ECC_RCD))
    {
      phb_ecc_element[phy_recno - 1].phy_idx = phy_recno;
      memcpy(phb_ecc_element[phy_recno - 1].number,entry->number, ECC_NUM_LEN);

      return PHB_OK;
    }
    else
      return PHB_FAIL;
  }

  /* Get Elementary file ID for the Phonebook type. */
  field_id = pb_sim_get_field_id(type);
  TRACE_EVENT_P1("pb_sim_get_field_id->Field_id: %x", field_id);

  /* Get Extension file for the Phonebook type. */
  ext_file_id = pb_sim_get_ext_file(type);

  TRACE_EVENT_P1("Ext_Field_id: %x", ext_file_id);

  db_result = db_info_field(pbs_data.db_handle, field_id, &info_field);

  /* Get record size to calculate alpha_len for the entry */
  if(db_result NEQ DB_OK)
  {
    TRACE_EVENT_P1("db_result = %d",db_result);
    return PHB_FAIL;
  }

  /* Handling of LDN, LMN and LRN Phonebook records.  */
  if((type EQ LDN) OR (type EQ LMN) OR (type EQ LRN))
  {
    if(phy_recno NEQ 1)
      return PHB_FAIL;
    
    /* Find free record in the elementary file. */
    db_result = db_find_free_record( pbs_data.db_handle, field_id);

    /* Get record number for the type of Phonebook. */
    rec_no = pb_sim_retrieve_rdm_recno(type);

     /* Error handling. */
    if(rec_no EQ 0)
      return PHB_FAIL;

    /* Read data related to LDN, LMN and LRN Phonebook from FFS */
    if(db_read_record(pbs_data.db_handle, 
      RDM_DATA_FILE_ID, rec_no, 0, RDM_PHB_DATA_SIZE, buffer) < DB_OK)
      return PHB_FAIL;

    /* Database is unable to find the free record. So overwrite the oldest one. */
    if(db_result < 0)
      phy_recno = buffer[RDM_PHB_DATA_SIZE - 1];
    else
      phy_recno = db_result;

    if(info_field.used_records EQ RDM_PHB_DATA_SIZE)
      i = RDM_PHB_DATA_SIZE - 1;
    else
      i = info_field.used_records;

    /* Move the records as new record has to be added. */
    for(; i > 0; i--)
    {
      buffer[i] = buffer[i-1];  
    }

    /* Update the record number of new entry. */
    buffer[0] = (UBYTE) phy_recno;

    /* Write back RDM data back into the database. */
    if(db_write_record(pbs_data.db_handle, 
      RDM_DATA_FILE_ID, rec_no, 0, RDM_PHB_DATA_SIZE, buffer) < DB_OK)
      return PHB_FAIL;

    /* Drop information about changed RDM records from the database. */
    (void)db_read_change_log(pbs_data.db_handle, rec_affected);
    rec_affected->entries = 0;
  }
  
  /* Convert T_PHB_TYPE into data structures as described in ETSI 11.11                  */
  /*  Bytes        Description                                      M/O   Length            |
  ----------------------------------------------------------------------
       1 to X          Alpha Identifier                                O       X bytes
       X+1                Length of BCD number/SSC contents    M       1 byte
       X+2                TON and NPI                                         M    1 byte
       X+3 to X+12  Dialling Number/SSC String                   M     10 bytes
       X+13        Capability/Configuration Identifier          M      1 byte
       X+14        Extension Record Identifier                   M     1 byte
       -------------------------------------------------------
       Extra fields for LDN, LMN, and LMN Phonebook records (As per 31.102)
       -------------------------------------------------------
       X+15 to X+21 Outgoing call date and time                  M     7 bytes
       X+22 to X+24 Outgoing call duration                          M      3 bytes 
       X+26              Line of the call                                      M    1 byte (New field added to 
                                                                                                         store line of call)
  ----------------------------------------------------------------------*/

  tag_len = pb_sim_get_entry_len(entry->tag, PHB_MAX_TAG_LEN);

  max_tag_len = info_field.record_size - pb_sim_get_size_except_tag(field_id);

  if(tag_len > max_tag_len)
    return PHB_TAG_EXCEEDED;

  memset(data, 0xFF, sizeof(data));
  memcpy(data, entry->tag, tag_len);

  if(entry->number[10] NEQ 0xFF)
  {
    data[max_tag_len] = 11; /* max. length */
  }
  else
  {
    data[max_tag_len] = entry->len + 1;
  }

  data[max_tag_len+1] = entry->ton_npi;
  memcpy((char *)&data[max_tag_len+2], 
           (char *)entry->number, 10);
  data[max_tag_len+12] = entry->cc_id;

  /* Copy data specific to records of Phonebook Types (LRN, LDN and LMN). */  
  if((type EQ LDN) OR (type EQ LRN) OR (type EQ LMN))
  {
    if(entry->v_time)
    {
      data[max_tag_len+14] = entry->time.year;
      data[max_tag_len+15] = entry->time.month;
      data[max_tag_len+16] = entry->time.day;
      data[max_tag_len+17] = entry->time.hour;
      data[max_tag_len+18] = entry->time.minute;
      data[max_tag_len+19] = entry->time.second;
      data[max_tag_len+20] = entry->time.time_zone;

      data[max_tag_len+21] = (UBYTE)((entry->time.duration >> 16) & 0xff);
      data[max_tag_len+22] = (UBYTE)((entry->time.duration >> 8) & 0xff);
      data[max_tag_len+23] = (UBYTE)((entry->time.duration) & 0xff);

    }

    if(entry->v_line)
    {
      data[max_tag_len+24] = entry->line;
    }
    
  }

  /* Check how many extension records are needed for number */ 
  if(entry->number[10] NEQ 0xFF)
  {
    ext_rec_cnt = entry ->len - 10;

    if(ext_rec_cnt % 10)
      ext_rec_cnt = (ext_rec_cnt  / 10) + 1;
    else
      ext_rec_cnt = (ext_rec_cnt  / 10);
  }

  /* Check how many extension records are needed for subaddress */ 
  if(entry->subaddr[0] NEQ 0xFF)
  {
    ext_rec_cnt++;
  }

  if(entry->subaddr[11] NEQ 0xFF)
  {
    ext_rec_cnt++;
  } 

  TRACE_EVENT_P1("phy_recno = %d",phy_recno);

  TRACE_EVENT_P2("no_rec = %d, used_rec = %d",info_field.num_records,info_field.used_records);

  /* If record number is not mentioned, search for the free record and add the entry. */
  if(phy_recno EQ 0)
  {
    db_result = db_find_free_record( pbs_data.db_handle, field_id);

    /* Database is unable to find the free record. So returning PHB_FULL. */
    if(db_result EQ DB_RECORD_NOT_FOUND)
      return PHB_FULL;

    if(db_result < DB_OK)
      return PHB_FAIL;

    /* Database has returned the free record number. */
    phy_recno = db_result;
  }
  else
  {
    /* 
     * Delete the record, if present to get rid of associated 
     * ext records. This has the effect that when we afterwards are unable to
     * create the new record (e.g. lack of extension records) the original 
     * record remains deleted. Probably this is not a problem, at least it 
     * should not be a big one.
     */
    db_result = db_record_empty (pbs_data.db_handle, 
                                 field_id, phy_recno, &record_empty);
    if ((db_result EQ DB_OK) AND !record_empty)
    {
      /* 
       * In case ext_rec is needed and the existing record is not using EXT, 
       * check if there is ext_rec available before deleting the existing entry
       * ext_info_field is used to get this info
       */
      if (ext_rec_cnt NEQ 0 AND !pb_sim_ext_records_used(type, field_id,phy_recno))
      {
        db_result = db_info_field(pbs_data.db_handle, ext_file_id, &ext_info_field);
        TRACE_EVENT_P3("before delelet rec; db_result = %d, ext_info_field.num_records = %d,  ext_info_field.used_records = %d", db_result, ext_info_field.num_records, ext_info_field.used_records);

 	      if((ext_info_field.num_records - ext_info_field.used_records) < ext_rec_cnt)
			   return PHB_EXT_FULL;
      }

      (void)pb_sim_del_record_internal (type, phy_recno, rec_affected, TRUE);
    }
  }

  /* Write record if entry does not require extension records */
  if(ext_rec_cnt EQ 0)              
  {
    /* Write record into FFS  */
    if(db_write_record(pbs_data.db_handle, field_id, phy_recno, 0, info_field.record_size, data) < DB_OK)
      return PHB_FAIL;

    /* For LDN record also write to SIM_LDN (without date and time Details */
    if(type EQ LDN)
    {
      memset(&data[max_tag_len + pb_sim_get_size_except_tag(field_id)], 0xFF, 
        SIZE_DATA_DATE_TIME);

     /* if(db_info_field(pbs_data.db_handle, SIM_LND, &info_field) NEQ DB_OK)
        return PHB_FAIL;

      if(db_write_record(pbs_data.db_handle, SIM_LND, phy_recno, 0, info_field.record_size, data) < DB_OK)
        return PHB_FAIL;*/
    }

  }
  else        /* We need to find free extension record. */
  {
    TRACE_EVENT_P1("Extension records = %d",ext_rec_cnt);
    /* Search for free extension records */

    /* Check whether extension file has required no. of free records. */
    db_result = db_info_field(pbs_data.db_handle, ext_file_id, &info_field);

    if((info_field.num_records - info_field.used_records) < ext_rec_cnt)
    {
      return PHB_EXT_FULL;
    }

    /* Required no. of free extension records are found */
    db_result = db_find_free_record( pbs_data.db_handle,ext_file_id);

    /* DB has returned non-zero positive number. 
            (Valid record number which is free). */ 

    if(db_result EQ DB_RECORD_NOT_FOUND)
    {
      return PHB_EXT_FULL;
    }

    if(db_result > DB_OK) 
    {  /* Set Extension record Identifier */
      ext_rec_num1 = db_result;

      data[max_tag_len+13] = ext_rec_num1;    

      db_result = db_info_field(pbs_data.db_handle, field_id, &info_field);

      /* Write record into FFS */
      if(db_write_record(pbs_data.db_handle, 
        field_id, phy_recno, 0, info_field.record_size, data) < DB_OK)
        return PHB_FAIL;
        
    }
    else
      return PHB_FAIL;

    db_result = db_info_field(pbs_data.db_handle, ext_file_id, &info_field);

    /* Prepare extension data and write into Extension file after finding the free records. */
    for(i = 0; i < ext_rec_cnt; i++)
    {
//TISH, patch for ASTec34494, added by Jinshu Wang, 2007-08-09
//start
			memset(data,0xff,SIM_MAX_RECORD_SIZE);
//end

      pb_sim_prepare_ext_data(data, i, (UBYTE*)entry->number, entry->len, (UBYTE*)entry->subaddr);

      /* Find next free record to chain the extension records. */
      db_result = db_find_free_record( pbs_data.db_handle,ext_file_id);
     
      if(db_result EQ DB_RECORD_NOT_FOUND)
      {
        return PHB_EXT_FULL;
      }

      if(db_result > DB_OK)
      {
        ext_rec_num2 = db_result;
      }
      else
      {
        return PHB_FAIL;
      }

      /* Chain extension records and set last link to "FF" */
      if(i NEQ (ext_rec_cnt - 1))
        data[12] =  ext_rec_num2;
          
      /* Write extension record into FFS */
      if(db_write_record(pbs_data.db_handle, ext_file_id, ext_rec_num1, 0, info_field.record_size, data) < DB_OK)
        return PHB_FAIL;

      if(pb_sim_update_extn_records(ext_file_id, ext_rec_num1, 1) EQ PHB_FAIL)
        return PHB_FAIL;

      /* Preserve the previous free record number. */ 
      ext_rec_num1 = ext_rec_num2;
    }
    
  }

  /* Get information about changed records from the database. */
  if(db_read_change_log(pbs_data.db_handle, rec_affected) NEQ DB_OK)
    return PHB_FAIL;

  /* Implements Measure #167 */
  return pb_sim_update_index ( type, rec_affected,field_id, ext_rec_cnt);

}


/*
+----------------------------------------------------------------------------+
| PROJECT: MMI-Framework (8417)         MODULE: PHB                          |
| STATE  : code                         ROUTINE: pb_sim_del_record_internal  |
+----------------------------------------------------------------------------+

    PURPOSE : Delete a given record at phy_recno from the given phonebook.
              If this function is called internally by a replacement operation
              on the phonebook this is indicated by the replacement parameter,
              in this case neither the change log are read nor the ring buffer
              management is touched in case of circular phonebooks.
*/
LOCAL T_PHB_RETURN pb_sim_del_record_internal (T_PHB_TYPE type,
                                               USHORT phy_recno,
                                               T_DB_CHANGED *rec_affected,
                                               BOOL replacement)
{
  USHORT field_id, db_recno, rec_no;
  UBYTE   buffer[RDM_PHB_DATA_SIZE], rec_cnt;

  TRACE_FUNCTION("pb_sim_del_record_internal()");

  /* Handling of ECC Phonebook */
  if(type EQ ECC)
  {
    if((phy_recno > 0 ) AND (phy_recno <= MAX_ECC_RCD))
    {
      phb_ecc_element[phy_recno - 1].phy_idx = 0;
      memset(phb_ecc_element[phy_recno - 1].number, 0xFF, ECC_NUM_LEN);

      return PHB_OK;
    }
    else
      return PHB_FAIL;
  }

  TRACE_EVENT_P1("phy_recno = %d", phy_recno);

  /* Get Elementary file ID for the Phonebook type. */
  field_id = pb_sim_get_field_id(type);

  /* Handling of LDN, LMN and LRN Phonebook records.  */
  if((type EQ LDN) OR (type EQ LMN) OR (type EQ LRN))
  {
    /* Get record number for the type of Phonebook. */
    rec_no = pb_sim_retrieve_rdm_recno(type);

    /* Error handling. */
    if(rec_no EQ 0)
      return PHB_FAIL;

    /* Read data related to LDN, LMN and LRN Phonebook from FFS */
    if(db_read_record(pbs_data.db_handle, 
        RDM_DATA_FILE_ID,rec_no, 0, RDM_PHB_DATA_SIZE, (UBYTE *) buffer) < DB_OK)
        return PHB_FAIL;

    /* Retrieve record number in Database from the buffer. */
    if((buffer[phy_recno - 1] EQ 0) OR (buffer[phy_recno - 1] > RDM_PHB_DATA_SIZE))
      return PHB_FAIL;
    
    db_recno = buffer[phy_recno - 1];

    if (!replacement)
    {
      /* Move the records */
      for(rec_cnt = phy_recno -1; rec_cnt < RDM_PHB_DATA_SIZE - 1; rec_cnt ++)
      {
        buffer[rec_cnt] = buffer[rec_cnt + 1]; 
      }

      /* Update the deleted entry record number. */
      buffer[RDM_PHB_DATA_SIZE - 1] = 0;

      /* Write back RDM data back into the database. */
      if(db_write_record(pbs_data.db_handle, 
        RDM_DATA_FILE_ID, rec_no, 0, RDM_PHB_DATA_SIZE, buffer) < DB_OK)
        return PHB_FAIL;

      /* Drop information about changed RDM records from the database. */
      (void)db_read_change_log(pbs_data.db_handle, rec_affected);
      rec_affected->entries = 0;
    }
  }
  else
  {
    db_recno = phy_recno;
  }

  /* Delete extension records if not referenced. */
  (void)pb_sim_del_ext_records(type, field_id, db_recno);

  /* Delete record from FFS. */
  if(db_delete_record(pbs_data.db_handle, field_id,db_recno) NEQ DB_OK)
    return PHB_FAIL;

  /* Get final information about changed records from the database. */
  if (!replacement)
  {
    if(db_read_change_log(pbs_data.db_handle, rec_affected) NEQ DB_OK)
      return PHB_FAIL;
  }

  /* Implements Measure #167 */
  return pb_sim_update_index ( type, rec_affected,field_id, 0);
  
}


/*
+----------------------------------------------------------------------------+
| PROJECT: MMI-Framework (8417)         MODULE: PHB                          |
| STATE  : code                         ROUTINE: pb_sim_del_record           |
+----------------------------------------------------------------------------+

    PURPOSE : Delete a given record at phy_recno from the given phonebook.
              This function reads the change log.

*/
T_PHB_RETURN pb_sim_del_record (T_PHB_TYPE type, 
                                USHORT phy_recno, 
                                T_DB_CHANGED *rec_affected)
{
  TRACE_FUNCTION ("pb_sim_del_record()");
  return (pb_sim_del_record_internal (type, phy_recno, rec_affected, FALSE));
}


/*
+--------------------------------------------------------------------+
| PROJECT: MMI-Framework (8417)         MODULE: PHB                  |
| STATE  : code                         ROUTINE: pb_sim_read_record  |
+--------------------------------------------------------------------+

    PURPOSE : Read a given physical record from the flash based phonebook.
*/

T_PHB_RETURN pb_sim_read_record (T_PHB_TYPE type, USHORT phy_recno, T_PHB_RECORD *entry)
{
  USHORT field_id, ext_file_id, rec_no;
  USHORT db_recno;
  T_DB_INFO_FIELD info_field;
  UBYTE   buffer[RDM_PHB_DATA_SIZE];
  UBYTE *data=NULL, *ptr;
  UBYTE max_tag_len;
  UBYTE ext_rcd_num;
    

  TRACE_FUNCTION("pb_sim_read_record()");

  /* Handling of ECC Phonebook */
  if(type EQ ECC)
  {
    if((phy_recno > 0 ) AND (phy_recno <= MAX_ECC_RCD))
    {
      memset(entry,0xFF,sizeof(T_PHB_RECORD));
      /* Return PHB_FAIL whenever ECC entry is empty */
      if((phb_ecc_element[phy_recno - 1].number[0] & 0x0F) 
        EQ 0x0F)
      {
        return PHB_FAIL;
      }
      else
      {
        entry->phy_recno = phy_recno;
        entry->len = ECC_NUM_LEN;
        memcpy(entry->number, phb_ecc_element[phy_recno - 1].number, ECC_NUM_LEN);
        return PHB_OK;
      }
    }
    else
      return PHB_FAIL;
  }

  /* Get Elementary file ID for the Phonebook type. */
  field_id = pb_sim_get_field_id(type);

  /* Get Extension file for the Phonebook type. */
  ext_file_id = pb_sim_get_ext_file(type);

  /* Read record from the database. */
  if(db_info_field(pbs_data.db_handle, field_id, &info_field) NEQ DB_OK)
    return PHB_FAIL;
  
  if((phy_recno EQ 0 ) OR (phy_recno > info_field.num_records))
    return PHB_FAIL; 
  
  /* Handling of LDN, LMN and LRN Phonebook records.  */
  if((type EQ LDN) OR (type EQ LMN) OR (type EQ LRN))
  {
    /* Get record number for the type of Phonebook. */
    rec_no = pb_sim_retrieve_rdm_recno(type);

     /* Error handling. */
    if(rec_no EQ 0)
      return PHB_FAIL;

    /* Read data related to LDN,LMN and LRN Phonebook from FFS */
    if(db_read_record(pbs_data.db_handle, 
      RDM_DATA_FILE_ID, rec_no, 0, RDM_PHB_DATA_SIZE, (UBYTE *) buffer) < DB_OK)
      return PHB_FAIL;
    
    db_recno = buffer[phy_recno - 1]; 
  }
  else
    db_recno = phy_recno;

  ACI_MALLOC(data,SIM_MAX_RECORD_SIZE);

  if(db_read_record(pbs_data.db_handle, field_id, db_recno, 0, info_field.record_size, data) < DB_OK)
  { 
    ACI_MFREE(data);
    return PHB_FAIL;
  }

  /* Convert SIM data to the type T_PHB_RECORD. */
  ptr = data;
  max_tag_len = info_field.record_size - pb_sim_get_size_except_tag(field_id);
  if (max_tag_len > PHB_MAX_TAG_LEN)
    max_tag_len = PHB_MAX_TAG_LEN;

  entry->phy_recno = phy_recno;
  entry->tag_len = (UBYTE)pb_sim_get_entry_len(ptr, max_tag_len);

  memset(entry->tag, 0xFF, PHB_MAX_TAG_LEN); /* init the tag value */
  memcpy ( (char*)entry->tag, (char*)ptr, entry->tag_len );

  ptr += (info_field.record_size) - pb_sim_get_size_except_tag(field_id);

  max_tag_len = (info_field.record_size) - pb_sim_get_size_except_tag(field_id);
  
  entry->len     = *ptr - 1;
  ++ptr;
  entry->ton_npi = *ptr;
  ++ptr;
  
  /* 
   * This error handling is done to avoid the accidental incorrect 
   * record length stored in the test SIMs 
   */
  if (entry->len > PHB_PACKED_NUM_LEN)
  {
     entry->len = PHB_PACKED_NUM_LEN;
  }
	  
  memset(entry->number, 0xFF, PHB_PACKED_NUM_LEN);
  memcpy( (char*)entry->number, (char*)ptr, entry->len );
  ptr += 10;
  entry->cc_id     = *ptr;
  ++ptr;

  /* Copy data specific to records of LDN, LMN, and LRN phonebook types. */ 
  if((type EQ LDN) OR (type EQ LMN) OR (type EQ LRN))
  {
    entry->v_time = 1;
    
    entry->time.year=data[max_tag_len+14] ;
    entry->time.month=data[max_tag_len+15];
    entry->time.day=data[max_tag_len+16];
    entry->time.hour=data[max_tag_len+17] ;
    entry->time.minute= data[max_tag_len+18];
    entry->time.second=data[max_tag_len+19] ;
    entry->time.time_zone=data[max_tag_len+20];

    entry->time.duration = (data[max_tag_len+21] << 16) +
                           (data[max_tag_len+22] << 8) +
                           (data[max_tag_len+23] );

    entry->v_line = 1;
    entry->line=data[max_tag_len+24];

  }

  if (*ptr NEQ 0xFF) /* check for extention records */
  {
    ext_rcd_num =(UBYTE)*ptr;

    if(db_info_field(pbs_data.db_handle, ext_file_id, &info_field) NEQ DB_OK)
    {
      ACI_MFREE(data); 
      return PHB_FAIL;
    }

    /* Reset pointer to start location. */
    ptr = data;

    memset(ptr, 0xFF, info_field.record_size);

    if(db_read_record(pbs_data.db_handle, ext_file_id, ext_rcd_num, 0, info_field.record_size, ptr) < DB_OK) 
         {
               ACI_MFREE(data); 
//TISH, patch for OMAPS00123396
//start
//      return PHB_FAIL;
			   return PHB_OK;
//end
         }
    pb_sim_read_ext(ptr, entry);

    while(*(ptr + 12) NEQ 0xFF) /* check if a further EXT entry exists */
    {
      memset(ptr, 0xFF, info_field.record_size);

      db_read_record(pbs_data.db_handle, ext_file_id, (USHORT)*(ptr+12), 0, info_field.record_size,ptr); 

      pb_sim_read_ext(ptr, entry);     
    }
  }

  ACI_MFREE(data);
  
  return PHB_OK;
}

/* Implements Measure #30 */

/*
+--------------------------------------------------------------------+
| PROJECT: MMI-Framework (8417)         MODULE: PHB                  |
| STATE  : code                         ROUTINE: pb_sim_search_name        |
+--------------------------------------------------------------------+

    PURPOSE : Finds the first entry matching the search tag.
*/

T_PHB_RETURN pb_sim_search_name (T_PHB_TYPE type, T_PHB_MATCH match, const T_ACI_PB_TEXT *search_tag, SHORT *order_num)
{
  T_ACI_PB_TEXT key;
  int res;
  USHORT field_id;
  
  TRACE_FUNCTION("pb_sim_search_name()");

  /* Get Elementary file ID for the Phonebook type. */
  field_id = pb_sim_get_field_id(type);



  key.len = search_tag->len;
  key.cs  = search_tag->cs;
  pb_sim_cvt_alpha_for_cmp ((UBYTE*) search_tag->data, (UBYTE*) key.data, search_tag->len);

  res = db_search(pbs_data.db_handle, field_id, NAME_IDX, order_num, pb_sim_search_alpha_func, match, (const UBYTE*)&key);

  if(res > DB_OK)
  {
    return PHB_OK;
  }

  return PHB_FAIL;
}

/*
+--------------------------------------------------------------------+
| PROJECT: MMI-Framework (8417)         MODULE: PHB                  |
| STATE  : code                         ROUTINE: pb_sim_search_number         |
+--------------------------------------------------------------------+

    PURPOSE : Finds the first entry matching the number.
*/

T_PHB_RETURN pb_sim_search_number   (T_PHB_TYPE type, const UBYTE *number, SHORT *order_num)
{
  int res, i;
  CHAR  cur_number[MAX_PHB_NUM_LEN];
  CHAR  rev_number[MAX_PHB_NUM_LEN];
  int cmpLen, cmp_res;
  USHORT field_id;
   
  TRACE_FUNCTION("pb_sim_search_number()");

  /* Handling for ECC Phonebook. */
  if(type EQ ECC)
  {
    for(i = 0; i < pbs_data.max_record[ECC]; i++)
    {

      cmhPHB_getAdrStr(cur_number, MAX_PHB_NUM_LEN - 1,phb_ecc_element[i].number,pbs_data.record_size[ECC]);
  
      pb_sim_revString(cur_number);

      strcpy( rev_number,(const char*)number);

      pb_sim_revString(rev_number);

      /*cmpLen = MINIMUM(strlen((const char*) cur_number), strlen((const char*)number));*/
      cmpLen = strlen((const char*)number);
      if(cmpLen > 7)
      {
        cmpLen = 7;
      cmp_res = pb_sim_cmpString((UBYTE*)cur_number, (UBYTE*)rev_number, cmpLen);
      }
      else
      {
        cmp_res = strcmp(cur_number,rev_number);
      }

	  /*patch for ASTec29800 EFECC by dongfeng*/
      if(cmp_res EQ 0)
         return PHB_OK;
	  }

	  return PHB_FAIL;
  }

  /* Get Elementary file ID for the Phonebook type. */
  field_id = pb_sim_get_field_id(type);




  res = db_search(pbs_data.db_handle, field_id, NUMBER_IDX, order_num, pb_sim_search_num_func, PHB_MATCH_PARTIAL, number);

  if(res > DB_OK)
  {
    return PHB_OK;
  }

  return PHB_FAIL;
}

/*
+--------------------------------------------------------------------+
| PROJECT: MMI-Framework (8417)         MODULE: PHB                  |
| STATE  : code                         ROUTINE: pb_sim_read_sizes   |
+--------------------------------------------------------------------+

    PURPOSE : Reads the sizes of a given phonebook.
*/

T_PHB_RETURN pb_sim_read_sizes (T_PHB_TYPE type,
                                SHORT      *max_rcd,
                                SHORT      *used_rcd,
                                UBYTE      *tag_len,
                                SHORT      *max_ext,
                                SHORT      *used_ext)
{
  T_DB_INFO_FIELD info_field;
  USHORT field_id;
  T_DB_CODE db_result;

  TRACE_FUNCTION("pb_sim_read_sizes()");

  /* Set every output parameter to zero for (not there) */
  *max_rcd = 0;
  *used_rcd = 0;
  *tag_len = 0;
  *max_ext = 0;
  *used_ext = 0;

  /* Handling for ECC Phonebook */
  if(type EQ ECC)
  {
    *max_rcd = pbs_data.max_record[ECC];
    *used_rcd = pbs_data.used_record[ECC];
    *tag_len = 0; /* To Do:Alpha tag will not be there for ECC. So assigning zero here */
    return PHB_OK;
  }

  /* Get elementary file ID for the Phonebook type. */
  field_id = pb_sim_get_field_id (type);

  /* Query the database about the field */
  db_result = db_info_field (pbs_data.db_handle, field_id, &info_field);

  if (db_result EQ DB_OK)
  {
    /* We got results for the main phonebook entries. */
    *max_rcd  = info_field.num_records;
    *used_rcd = info_field.used_records;
    *tag_len  = info_field.record_size - pb_sim_get_size_except_tag(field_id);

    /* Get elementary file ID for the respective extension. */
    field_id = pb_sim_get_ext_file (type);
    if (field_id NEQ 0)
    {
      /* 
       * Extension records may exist for this kind of phonebook. 
       * Query the database about the ext records.
       */
      db_result = db_info_field (pbs_data.db_handle, field_id, &info_field);
      if (db_result EQ DB_OK)
      {
        /* There are also extension records present in the database. */
        *max_ext  = info_field.num_records;
        *used_ext = info_field.used_records;
      }
      /* There is no else as it is not a problem if there are no extension
       * records for the phonebook on the SIM as those are optional only. */
    }
    return PHB_OK;
  }
  return PHB_FAIL; /* The phonebook does not exist */
}

/*
+--------------------------------------------------------------------+
| PROJECT: MMI-Framework (8417)         MODULE: PHB                  |
| STATE  : code                         ROUTINE: pb_sim_get_entry_len          |
+--------------------------------------------------------------------+

    PURPOSE : Used to get the length in bytes of a given entry which is coded as given in 11.11 Annex B.
*/

int pb_sim_get_entry_len    (const UBYTE *pb_tag, UBYTE max_pb_len)
{
  
  int   pb_len    = 0;
  UBYTE inc_count = 1;
  BOOL  ucs2      = FALSE;
  UBYTE chars = 0;

  TRACE_FUNCTION("pb_sim_get_entry_len()");

  if (*pb_tag EQ 0x80)
  {
    ucs2 = TRUE;
    inc_count = 2;              /* check two bytes */
    pb_len = 1;                 /* skip the 0x80 */
  }
  else if (*pb_tag EQ 0x81 OR *pb_tag EQ 0x82)
  {
    if (*pb_tag EQ 0x81)
      pb_len = 3;                 /* 0x80 + len + pointer */
    else
      pb_len = 4;                 /* 0x80 + len + 2xpointer */

    chars = pb_tag[1];
    pb_tag += pb_len;                  /* go to data */
    while (chars)
    {
      if (*pb_tag++ & 0x80)
        pb_len+=2;
      else
        pb_len+=1;

      pb_tag++;
      chars--;
    }
    return MINIMUM(pb_len,max_pb_len);
  }

  while (pb_len < max_pb_len)
  {
    if (ucs2 EQ TRUE)
    {
      if (!(pb_len+1 < max_pb_len)) /* Check also if we traverse the upper bound */
        break;                      /* so only a "half" UCS2 element is remaining */
    }
    if (pb_tag[pb_len] EQ 0xFF)
    { 
      /* one 0xFF indicates the end of a non UCS2 string */
      if (ucs2 EQ FALSE)
      {
        break;
      }
      /* two 0xFF indicates the end of a UCS2 string */
      if (pb_tag[pb_len + 1] EQ 0xFF)
      {
        break;
      }
    }
    pb_len += inc_count;
  }

  return (pb_len);
}

/*
+--------------------------------------------------------------------+
| PROJECT: MMI-Framework (8417)         MODULE: PHB                  |
| STATE  : code                         ROUTINE: pb_sim_alpha_cmp           |
+--------------------------------------------------------------------+

    PURPOSE : Compares alpha identifier of two records.
*/
int pb_sim_alpha_cmp (int db_handle, USHORT field_id, USHORT recno_1, USHORT recno_2, ULONG flags)
{
  T_PHB_RECORD entry_1,entry_2;
  UBYTE cmpLen = 0;
  UBYTE *buffer;
  UBYTE max_tag_len;
  UBYTE cur_tag[PHB_MAX_TAG_LEN], check_tag[PHB_MAX_TAG_LEN];
  int   cmp_res;
  T_DB_INFO_FIELD info_field;
  
  TRACE_FUNCTION("pb_sim_alpha_cmp()");

  
  db_info_field(db_handle, field_id, &info_field);

  MALLOC(buffer, info_field.record_size);

  /* Read the first record. */
  db_read_record(db_handle, field_id, recno_1, 0, info_field.record_size, buffer);

  /* Unpack record 1 to do a string comparison on the alpha identifier field */
  max_tag_len = info_field.record_size - pb_sim_get_size_except_tag(field_id);
  if (max_tag_len > PHB_MAX_TAG_LEN)
    max_tag_len = PHB_MAX_TAG_LEN;

  entry_1.tag_len = (UBYTE)pb_sim_get_entry_len(buffer, max_tag_len);
  
  memset(entry_1.tag, 0xFF, PHB_MAX_TAG_LEN); /* init the tag value */
  memcpy ( (char*)entry_1.tag, (char*)buffer, entry_1.tag_len );

  pb_sim_cvt_alpha_for_cmp ( entry_1.tag, cur_tag, entry_1.tag_len );

  memset(buffer, 0, info_field.record_size);

  /* Read the second record. */
  db_read_record(db_handle, field_id, recno_2, 0, info_field.record_size, buffer);

  /* Unpack record 2 to do a string comparison on the alpha identifier field */
  max_tag_len = info_field.record_size - pb_sim_get_size_except_tag(field_id);
  if (max_tag_len > PHB_MAX_TAG_LEN)
    max_tag_len = PHB_MAX_TAG_LEN;

  entry_2.tag_len = (UBYTE)pb_sim_get_entry_len(buffer, max_tag_len);
  
  memset(entry_2.tag, 0xFF, PHB_MAX_TAG_LEN); /* init the tag value */
  memcpy ( (char*)entry_2.tag, (char*)buffer, entry_2.tag_len );

  pb_sim_cvt_alpha_for_cmp ( entry_2.tag, check_tag, entry_2.tag_len );

  cmpLen = MINIMUM ( entry_1.tag_len,
                           entry_2.tag_len );

  TRACE_EVENT_P1("%d", cmpLen);
      
  cmp_res = pb_sim_cmpString ( cur_tag, check_tag, cmpLen );

	if (cmp_res EQ 0) 
  {
    /* Correct result when length was different, ACIPHB201 */
    if (entry_1.tag_len < entry_2.tag_len)
      cmp_res = -1;
    else if (entry_1.tag_len > entry_2.tag_len)
      cmp_res = 1;
  }

  MFREE(buffer);

  return cmp_res;
}
/*
+--------------------------------------------------------------------+
| PROJECT: MMI-Framework (8417)         MODULE: PHB                  |
| STATE  : code                         ROUTINE: pb_sim_read_number  |
+--------------------------------------------------------------------+

    PURPOSE : Reads a phine number from a given record
*/
LOCAL void pb_sim_read_number (int db_handle,
                               USHORT field_id, 
                               USHORT recno,
                               CHAR *number)
{
  T_PHB_RECORD entry;
  T_DB_INFO_FIELD info_field;
  UBYTE *sim_buffer, *buffer;
  UBYTE ext_rcd_num;
  unsigned ext_rec_cnt;
  USHORT ext_id;

  TRACE_FUNCTION("pb_sim_read_number()");

  ACI_MALLOC(sim_buffer, SIM_MAX_RECORD_SIZE);
  buffer = sim_buffer;

  db_info_field(db_handle, field_id, &info_field);

  /* Read record recno_1 from the database using db_read_record() */
  db_read_record (db_handle, field_id, recno, 0, info_field.record_size, buffer);

  /* Read only number from the buffer. */
  buffer += (info_field.record_size) - pb_sim_get_size_except_tag(field_id);
  entry.len     = *(buffer++) - 1;
  entry.ton_npi = *buffer++;

  /* 
   * This error handling is done to avoid the accidental incorrect 
   * record length stored in the test SIMs 
   */
  if (entry.len > PHB_PHY_NUM_LENGTH)
  {
    entry.len = PHB_PHY_NUM_LENGTH;
  }

  memset (entry.number, 0xFF, PHB_PACKED_NUM_LEN);
  memcpy (entry.number, buffer, entry.len);
  buffer += 10; 
  entry.cc_id = *buffer++;
  ext_rcd_num = (UBYTE)*buffer;
  ext_id = pb_sim_get_ext_file_id (field_id);
  if ((ext_id NEQ 0) AND
      (db_info_field (db_handle, ext_id, &info_field) EQ DB_OK))
  {
    /* Extension records exist and we can obtain information about it */
    ext_rec_cnt = 0;
    while ((ext_rcd_num NEQ 0xFF) AND
           (ext_rcd_num NEQ 0) AND
           (ext_rcd_num <= info_field.num_records) AND
           (ext_rec_cnt < info_field.num_records))
    {
      /* 
       * Record not empty, in range 1..max num of ext records
       * Impossible to have read all records (avoid infinite loop)
       */
      ext_rec_cnt++;
      (void)db_read_record (db_handle, ext_id, ext_rcd_num,
                            0, info_field.record_size, 
                            buffer);
      pb_sim_read_ext (buffer, &entry);
      ext_rcd_num = sim_buffer[12];
    }
  }

  cmhPHB_getAdrStr(number,
                   MAX_PHB_NUM_LEN - 1,
                   entry.number,
                   entry.len);

  ACI_MFREE (sim_buffer);
}

/*
+--------------------------------------------------------------------+
| PROJECT: MMI-Framework (8417)         MODULE: PHB                  |
| STATE  : code                         ROUTINE: pb_sim_number_cmp   |
+--------------------------------------------------------------------+

    PURPOSE : Compares two numbers of two records.
*/
int pb_sim_number_cmp (int db_handle,
                       USHORT field_id, 
                       USHORT recno_1, 
                       USHORT recno_2,
                       ULONG  flags)
{
  CHAR cur_number[MAX_PHB_NUM_LEN];
  CHAR ref_number[MAX_PHB_NUM_LEN];

  TRACE_FUNCTION("pb_sim_number_cmp()");

  /* Read the numbers */
  pb_sim_read_number (db_handle, field_id, recno_1, cur_number);
  pb_sim_read_number (db_handle, field_id, recno_2, ref_number);

  /* Reverse the numbers to compare number from right. */
  pb_sim_revString(cur_number);
  pb_sim_revString(ref_number);
  
  return pb_sim_cmpWild ((char *)cur_number, (char *)ref_number, MAX_PHB_NUM_LEN);
}

/*
+--------------------------------------------------------------------+
| PROJECT: MMI-Framework (8417)         MODULE: PHB                  |
| STATE  : code                         ROUTINE: pb_sim_read_ext     |
+--------------------------------------------------------------------+

    PURPOSE : Reads extension records from the database and copies them to number.
*/
/* Extesion records will be stored as per 11.11.
          Bytes         Description      M/O    Length
          --------------------------------------
              1        Record type         M    1 byte
           2 to 12  Extension data  M   11 bytes
             13        Identifier                 M 1 byte
          --------------------------------------*/

void pb_sim_read_ext(UBYTE *buffer, T_PHB_RECORD *entry)
{
  UBYTE data_len;
  UBYTE data_type;
  UBYTE *data;
  
  TRACE_FUNCTION("pb_sim_read_ext()");

  /* If this extension record is not empty, it is written in phonebook. */
  data = buffer;
 
  data_type = *data;
  data_len = *(data+1);

  switch (data_type)
  {
   
    case 1: /* Called Party Subaddress */
    {
			int sa_len = 0;
      while (sa_len<PHB_PACKED_NUM_LEN)  /* get length of possible already stored subaddr if more than one EXT is used */
      {
        if (entry->subaddr[sa_len] EQ 0xFF)
					break;
        else if ((entry->subaddr[sa_len] & 0xF0) EQ 0xF0)
        {
          sa_len++;
          break;
        }
        else
          sa_len++;
      }

      pb_sim_nibblecopy (entry->subaddr,
                         sa_len,
                         data + 2,
                         data_len);
    }
    break;
        
    case 2: /* Additional data */
      entry->len =
      pb_sim_nibblecopy (entry->number,
                         entry->len,
                         data + 2,
                         data_len);
      break;

      default: /* unknown type */
        break;
  }

  return;
}

/*
+--------------------------------------------------------------------+
| PROJECT: MMI-Framework (8417)         MODULE: PHB                  |
| STATE  : code                         ROUTINE: pb_sim_revString                      |
+--------------------------------------------------------------------+

    PURPOSE : Reverses a string within the same variable.
*/

void pb_sim_revString(char *str)
{
  UBYTE i, j,str_len;
  char ch;

  TRACE_FUNCTION("pb_sim_revString()");

  str_len = strlen(str);
  
  for(i = 0, j = str_len - 1;i < (str_len / 2); i++, j--)
  {
    ch = *(str + i);
    *(str + i) = *(str + j);
    *(str + j) = ch;
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT: MMI-Framework (8417)         MODULE: PHB                  |
| STATE  : code                         ROUTINE: pb_sim_search_alpha_func                      |
+--------------------------------------------------------------------+

    PURPOSE : Searches for a given alpha key in the database.
*/

int pb_sim_search_alpha_func(ULONG flags, const UBYTE *key, int db_handle, USHORT field_id, USHORT rec_num)
{
  T_ACI_PB_TEXT *search_key;
  T_PHB_RECORD entry;
  UBYTE cmpLen = 0;
  T_PHB_TYPE phb_type;
  UBYTE cur_tag[PHB_MAX_TAG_LEN];
  int   cmp_res;

  TRACE_FUNCTION("pb_sim_search_alpha_func()");

  /* Cast search key to appropriate data structure */
  search_key = (T_ACI_PB_TEXT *)key;

  /* Get PHB type from field ID using PHB_ACI function. */ 
  phb_type = pb_get_phb_type_from_ef(field_id);

  /* Read record from the database. */
  pb_sim_read_record(phb_type, rec_num, &entry);
	
  pb_sim_cvt_alpha_for_cmp ( entry.tag, cur_tag, entry.tag_len );

  cmpLen = search_key->len;
  
  if(flags EQ PHB_MATCH_PARTIAL)
    cmpLen = MINIMUM ( entry.tag_len, cmpLen);

  TRACE_EVENT_P1( "cmpLen=%d", cmpLen );
      
  cmp_res = pb_sim_cmpString ( cur_tag, search_key->data, cmpLen );
  
  return cmp_res;
}


/*
+----------------------------------------------------------------------------+
| PROJECT: MMI-Framework (8417)         MODULE: PHB                          |
| STATE  : code                         ROUTINE: pb_sim_search_num_func      |
+----------------------------------------------------------------------------+

    PURPOSE : Searches for a given number key in the database.
*/
int pb_sim_search_num_func(ULONG flags, const UBYTE *key, int db_handle,
                           USHORT field_id, USHORT rec_num)
{
  T_PHB_RECORD entry;
  UBYTE cmpLen = 0;
  T_PHB_TYPE phb_type;
  CHAR  cur_number[MAX_PHB_NUM_LEN];
  CHAR  rev_key[MAX_PHB_NUM_LEN];
  int   cmp_res;
  size_t strg_len;
   
  TRACE_FUNCTION("pb_sim_search_num_func()");

  /* Get PHB type from field ID using PHB_ACI function. */ 
  phb_type = pb_get_phb_type_from_ef(field_id);

  /* Read record from the database. */
  if(pb_sim_read_record(phb_type, rec_num, &entry) NEQ PHB_OK)
    return -1;

  cmhPHB_getAdrStr(cur_number,
        MAX_PHB_NUM_LEN - 1,
        entry.number,
        entry.len);
  
  /* Reverse the first number to compare number from right. */
  pb_sim_revString(cur_number);

  /* Reverse the second number to compare number from right. */
  
  strcpy (rev_key, (const char*) key);
  
  pb_sim_revString(rev_key);

  cmpLen = strlen(rev_key);

  if(flags EQ PHB_MATCH_PARTIAL)
  {
    strg_len = strlen(cur_number);
    cmpLen = MINIMUM(strg_len, cmpLen);
  }

  TRACE_EVENT_P1("Number to be compared: %s", cur_number);
  TRACE_EVENT_P1("Number to be searched: %s", rev_key);
  if(cmpLen > 7)
  {
    cmpLen = 7;
  }
  
  cmp_res = pb_sim_cmpWild((char*)cur_number, (char*)rev_key, cmpLen);
  

  TRACE_EVENT_P1("Result of the comparison: %d", cmp_res);

  return cmp_res;
  
}

/*
+--------------------------------------------------------------------+
| PROJECT: MMI-Framework (8417)         MODULE: PHB                  |
| STATE  : code                         ROUTINE: pb_sim_nibblecopy                      |
+--------------------------------------------------------------------+

    PURPOSE : Used to convert BCD nibbles to string.
*/
    
LOCAL int pb_sim_nibblecopy (UBYTE dest[], int destlen, UBYTE src[], int count)
{

  int i;
  int nibble;

  int destnibble=destlen*2;

  TRACE_FUNCTION("pb_sim_nibblecopy()");

  if (destnibble)
  {
    if ((dest[destlen-1] & 0xF0) == 0xF0)    /* check if there is space in last nibble */
      destnibble--;
  }

  for ( i=0; i<count*2; i++ )
  {
    /* check if we access out of bounds */
    if (destnibble/2 >= PHB_PACKED_NUM_LEN)
      return PHB_PACKED_NUM_LEN;

    /* get nibble */
    if (i%2 == 0)
      nibble = src[i/2] & 0x0F;
    else
      nibble = (src[i/2] & 0xF0) >> 4;

    if (nibble == 0xF)      /* end of number detected */
      break;

    /* put nibble */
    if (destnibble%2 == 0)
    {
      dest[destnibble/2] &= 0xF0;
      dest[destnibble/2] |= nibble;
    }
    else
    {
      dest[destnibble/2] &= 0x0F;
      dest[destnibble/2] |= nibble << 4;
    }

    destnibble++;
  }
  return destnibble/2 + destnibble%2; /* round up */
}

/*
+------------------------------------------------------------------------+
| PROJECT : MMI-Framework (8417)         MODULE: PHB                     |
| STATE   : code                         ROUTINE : pb_sim_read_eeprom_ecc    |
+------------------------------------------------------------------------+

  PURPOSE : Read EC number from EEPROM.

*/
LOCAL void pb_sim_read_eeprom_ecc (void)
{
  EF_ECC       efecc;
  UBYTE        *data_ptr;
  UBYTE        version;
  int          rec_ctr;

  TRACE_FUNCTION("pb_sim_read_eeprom_ecc()");

  /* Initialise ECC Phonebook Info. */
  pbs_data.max_record[ECC] = MAX_ECC_RCD;
  pbs_data.used_record[ECC] = 0;

  if (pcm_ReadFile((UBYTE *)EF_ECC_ID,
                    SIZE_EF_ECC,
                    (UBYTE *)&efecc,
                    &version) EQ DRV_OK)
  {   
    { /* workaround when invalid data stored on PCM */
      CHAR  ecc_number[MAX_PHB_NUM_LEN];
      int   num_len;

      data_ptr = efecc.ecc1;
      
      for (rec_ctr=0; rec_ctr < pbs_data.max_record[ECC]; rec_ctr++)
      {
        if (*data_ptr NEQ 0xFF)
        {
          cmhPHB_getAdrStr (ecc_number,
                            MAX_PHB_NUM_LEN - 1,
                            data_ptr,
                            ECC_NUM_LEN);
          for (num_len = 0; num_len < ECC_NUM_LEN; num_len++)
          {
            if (!isdigit (ecc_number[num_len]))
            {
              TRACE_EVENT_P2 ("[ERR] pb_read_eeprom_ecc(): invalid character found %c (%d)",
                              ecc_number[num_len], rec_ctr);
              return;
            }
          }
        }
        data_ptr += ECC_NUM_LEN;
      }
    } /* workaround end */  
    
    data_ptr = &efecc.ecc1[0];

    memset( phb_ecc_element,0xFF, (pbs_data.max_record[ECC] * sizeof(T_PHB_ECC_RECORD)) );

    for (rec_ctr=0; rec_ctr < pbs_data.max_record[ECC]; rec_ctr++)
    {
      if(*data_ptr NEQ 0xff)
      {
        phb_ecc_element[rec_ctr].phy_idx = rec_ctr + 1;
        memcpy(phb_ecc_element[rec_ctr].number, data_ptr, ECC_NUM_LEN);
        data_ptr += ECC_NUM_LEN;
        (pbs_data.used_record[ECC])++;
      }
    }
  }
}

/*
+----------------------------------------------------------------------+
| PROJECT :                              MODULE  : PHB                 |
| STATE   : code                         ROUTINE : pb_sim_prepare_ext_data |
+----------------------------------------------------------------------+


    PURPOSE :   Prepare the data for the extention record.
                If NULL pointer is given for number and subaddress
                then the extention record will marked as unused

*/
/* Extesion records will be stored as per 11.11.
          Bytes         Description      M/O    Length
          --------------------------------------
              1        Record type         M    1 byte
           2 to 12  Extension data  M   11 bytes
             13        Identifier                 M 1 byte
          --------------------------------------*/

LOCAL void pb_sim_prepare_ext_data(UBYTE *ext_data, int ext_count, UBYTE *number, UBYTE no_len, UBYTE *subaddr)
{
  UBYTE *data_num = NULL;
  UBYTE *data_subadd = NULL;

  TRACE_FUNCTION("pb_sim_prepare_ext_data()");

  if(number[10] NEQ 0xFF)
    data_num = number + ((ext_count + 1) * 10);
  
  data_subadd = subaddr + (ext_count * 11);
  
  memset(ext_data, 0xFF, sizeof(ext_data));

  if ((data_num NEQ NULL) AND (*data_num NEQ 0xFF))
  {
    /* Set record type to 2 which corresponds to Additional data.  Record type as per 11.11 */
    ext_data[0] = 2;
    ext_data[1] = no_len - ((ext_count + 1) * 10);
    memcpy (ext_data + 2, data_num, 10);
  }
  else if ((data_subadd NEQ NULL) AND (*data_subadd NEQ 0xFF))
  {
    TRACE_EVENT ("SUBADDRESS EXTENTION");
	 /* Set record type to 1 which corresponds to Called Party Subaddress. Record type as per 11.11 */
    ext_data[0] = 1;
    memcpy (ext_data + 1, data_subadd, 11);
  }
}

/*
+----------------------------------------------------------------------+
| PROJECT :                              MODULE  : PHB                 |
| STATE   : code                         ROUTINE : pb_sim_get_field_id |
+----------------------------------------------------------------------+


    PURPOSE :   Returns field ID for the corresponding Phonebook type.

*/
LOCAL USHORT pb_sim_get_field_id (T_PHB_TYPE type)
{
  USHORT field_id;

  TRACE_FUNCTION("pb_sim_get_field_id()");

  /* Get Elementary file ID for the Phonebook type. */
  switch(type)
  {
    case ADN:
      field_id = SIM_ADN;
      break;

    case LDN:
      field_id = SIM_OCI;
      break;

    case LRN:
      field_id = FFS_LRN;
      break;

    case LMN:
      field_id = FFS_LMN;
      break;

    case UPN:
      field_id = SIM_MSISDN;
      break;

    case FDN:
      field_id = SIM_FDN;
      break;

    case SDN:
      field_id = SIM_SDN;
      break;

    case BDN:
      field_id = SIM_BDN;
      break;

    default:
      TRACE_ERROR ("No such field");
      TRACE_EVENT_P1 ("No such field for type = %04X", type); 
      field_id = 0;
      break;
  }

  return field_id;
}

/*
+----------------------------------------------------------------------+
| PROJECT :                              MODULE  : PHB                 |
| STATE   : code                         ROUTINE : pb_sim_get_ext_file |
+----------------------------------------------------------------------+


    PURPOSE :   Returns field ID for the corresponding Phonebook type.

*/
LOCAL USHORT pb_sim_get_ext_file (T_PHB_TYPE type)
{
  USHORT ext_file_id;

  TRACE_FUNCTION("pb_sim_get_ext_file()");

  /* Get Extension Elementary file ID for the Phonebook type. */
  switch(type)
  {
    case ADN:
    case UPN:
      ext_file_id = SIM_EXT1;
      break;

    case FDN:
      ext_file_id = SIM_EXT2;
      break;

    case SDN:
      ext_file_id = SIM_EXT3;
      break;

    case BDN:
      ext_file_id = SIM_EXT4;
      break;

    case LDN:
      ext_file_id = SIM_EXT5;
      break;

    case LRN:
      ext_file_id = FFS_EXT_LRN;
      break;

    case LMN:
      ext_file_id = FFS_EXT_LMN;
      break;

    default:
      ext_file_id = 0; /* No extension records available */
      break;
  }

  return ext_file_id;
}

/*
+---------------------------------------------------------------------+
| PROJECT : MMI-Framework (8417)       MODULE  : PHB                  |
| STATE   : code                       ROUTINE : pb_sim_cvt_alpha_for_cmp |
+---------------------------------------------------------------------+

  PURPOSE : convert alpha to lower case when not unicode

*/
LOCAL void pb_sim_cvt_alpha_for_cmp ( UBYTE *src,
                                      UBYTE *dst,
                                      UBYTE len )
{
  int i;

  TRACE_FUNCTION("pb_sim_cvt_alpha_for_cmp()");

  if (*src NEQ 0x80) /* 11.11 Annex B 0x80 is the UCS2 indicator */
  {
    for ( i = 0; i < len; i++ )
      dst[i] = (UBYTE)tolower((int)src[i]);
  }
  else
  {
    memcpy (dst, src, len);
  }
}

/*
+----------------------------------------------------------------------+
| PROJECT :                              MODULE  : PHB                 |
| STATE   : code                         ROUTINE : pb_sim_get_ext_file_id                           |
+----------------------------------------------------------------------+


    PURPOSE :   Returns Extension field ID for the corresponding field ID of a Phonebook.

*/
LOCAL USHORT pb_sim_get_ext_file_id (USHORT field_id)
{
  USHORT ext_file_id;

  TRACE_FUNCTION("pb_sim_get_ext_file_id()");

  /* Get Extension Elementary file ID for the Phonebook type. */
  switch(field_id)
  {
    case SIM_ADN:
    case SIM_LND:
    case SIM_MSISDN:
      ext_file_id = SIM_EXT1;
      break;

    case SIM_FDN:
      ext_file_id = SIM_EXT2;
      break;

    case SIM_SDN:
      ext_file_id = SIM_EXT3;
      break;

    case SIM_BDN:
      ext_file_id = SIM_EXT4;
      break;

    case SIM_OCI:
      ext_file_id = SIM_EXT5;
      break;

    case FFS_LRN:
      ext_file_id = FFS_EXT_LRN;
      break;

    case FFS_LMN:
      ext_file_id = FFS_EXT_LMN;
      break;

    default:
      ext_file_id = 0; 
      break;
  }

  return ext_file_id;
}

/*
+----------------------------------------------------------------------+
| PROJECT :                              MODULE  : PHB                 |
| STATE   : code                         ROUTINE : pb_sim_get_ext_type |
+----------------------------------------------------------------------+


    PURPOSE :   Returns Extension Type ID for the corresponding 
                field ID of a Phonebook.

*/
LOCAL T_EXT_TYPE pb_sim_get_ext_type (USHORT field_id)
{
  T_EXT_TYPE ext_type;
  
  /* Get Extension Type for the Phonebook field ID. */
  switch(field_id)
  {
    case SIM_ADN:
    case SIM_LND:
    case SIM_MSISDN:
      ext_type = EXT1;
      break;

    case SIM_FDN:
      ext_type = EXT2;
      break;

    case SIM_SDN:
      ext_type = EXT3;
      break;

    case SIM_BDN:
      ext_type = EXT4;
      break;

    case SIM_OCI:
      ext_type = EXT5;
      break;

    case FFS_LRN:
      ext_type = EXT_LRN;
      break;

    case FFS_LMN:
      ext_type = EXT_LMN;
      break;

    default:
      ext_type = INVALID_EXT; 
      break;
  }

  return ext_type;
}

/*
+----------------------------------------------------------------------+
| PROJECT :                              MODULE  : PHB                 |
| STATE   : code                         ROUTINE : pb_sim_find_free_record                        |
+----------------------------------------------------------------------+


    PURPOSE :   Returns free record number for the Phonebook type.

*/
GLOBAL int  pb_sim_find_free_record   (T_PHB_TYPE type)
{
  int db_result;
  unsigned i;
  USHORT field_id;

  TRACE_FUNCTION("pb_sim_find_free_record()");

  switch (type)
  {
    case ECC: /* ECC not stored in DB, special handling */
      for (i = 0; i < pbs_data.max_record[ECC]; i++)
      {
        if (phb_ecc_element[i].phy_idx EQ 0)
          return i + 1;
      }
      return 0; /* No free record found */

    case LDN:
    case LRN:
    case LMN:
      return 1; /* For cyclic entries always the first */

    default:
      /* Get Elementary file ID for the Phonebook type. */
      field_id = pb_sim_get_field_id(type);

      db_result = db_find_free_record(pbs_data.db_handle, field_id);
      
			if (db_result <= 0)
        return 0; /* No free record */
      
			return db_result;
  }
}

GLOBAL int  pb_sim_find_free_ext_record   ()
{
  int db_result;
  unsigned i;
  USHORT field_id;

  TRACE_FUNCTION("pb_find_ext_free_record()");

      field_id =SIM_EXT1;

      db_result = db_find_free_record(pbs_data.db_handle, field_id);
      
      if (db_result <= 0)
         return 0; /* No free record */
      
      return db_result;
}





/*
+-------------------------------------------------------------------------------+
| PROJECT : MMI-Framework (8417)        MODULE  : PHB                           |
| STATE   : code                        ROUTINE : pb_sim_get_size_except_tag    |
+-------------------------------------------------------------------------------+

  PURPOSE : Returns size of data excluding length of tag (alpha identifier)
*/
USHORT pb_sim_get_size_except_tag (USHORT field_id)
{

  TRACE_FUNCTION("pb_sim_get_size_except_tag()");
  switch(field_id)
  {
    case SIM_ADN:
    case SIM_FDN:
    case SIM_BDN:
    case SIM_MSISDN:
    case SIM_SDN:
      return 14; /* 11.11 */

    case FFS_LRN:
    case FFS_LMN:
    case SIM_OCI:
      return 27; /* Using EF_OCI, 31.102 4.2.34 */

    //case SIM_ICI:
    //  return 28; /* Using EF_ICI, 31.102 4.2.33 */

    default:
      TRACE_ERROR("Invalid field ID passed !");
      return 0;
  }
}
/*
+--------------------------------------------------------------------+
| PROJECT : MMI-Framework (8417)        MODULE  : PHB                |
| STATE   : code                        ROUTINE : pb_sim_cmpString          |
+--------------------------------------------------------------------+

  PURPOSE : compare two strings.
            if s1 < s2 return value < 0
            if s1 = s2 return value = 0
            if s1 > s2 return value > 0
*/

static int pb_sim_cmpString ( UBYTE *s1, UBYTE *s2, UBYTE len )
{
  int n = 0;

  /* TRACE_FUNCTION("pb_sim_cmpString()"); */ /* Called too often to trace */

  if ((*s1 EQ 0x80) AND
      (*s2 NEQ 0x80)    )
  {
    s1++;
    len--;
    return pb_sim_cmp2Bytes(s1, s2, len, 1);
  }
  else if ((*s1 NEQ 0x80) AND
           (*s2 EQ 0x80)     )
  {
    s2++;
    len--;
    return pb_sim_cmp2Bytes(s1, s2, len, 2);
  }
  else
  {
    while (*s1 EQ *s2)
    {
      if (*s1 EQ 0xff)
        return 0;
      s1++;
      s2++;
      n++;
      if (n EQ len)
        return 0;
    }

    if ((*s1 > *s2) AND (*s1 NEQ 0xff))
      return 1;

    return -1;
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : MMI-Framework (8417)        MODULE  : PHB                |
| STATE   : code                        ROUTINE : pb_sim_cmp2Bytes       |
+--------------------------------------------------------------------+

  PURPOSE : compare two strings.
            if s1 < s2 return value < 0
            if s1 = s2 return value = 0
            if s1 > s2 return value > 0

            flag = 1, s1 is unicode
            flag = 2, s2 is unicode
*/

LOCAL int pb_sim_cmp2Bytes(UBYTE *s1, UBYTE *s2, UBYTE len, UBYTE flag)
{
  int n = 0;

  /*  TRACE_FUNCTION("pb_sim_cmp2Bytes()"); */

  if (flag EQ 1)
  {
    while ( (*s1 EQ 0x00 OR *s1 EQ 0xFF ) AND ( *(s1+1) EQ *s2) )
    {
      if (*s1 EQ 0xff AND *(s1+1) EQ 0xFF)
        return 0;
      
			s1 += 2;
      s2++;
      n += 2;

      if (n >= len)
        return 0;
    }

    if ( ( *s1 > 0 AND *s1 NEQ 0xff ) OR
         ( !*s1 AND ( *(s1+1) > *s2 ) ) )
      return 1;

    return -1;
  }

  if (flag EQ 2)
  {
    while ((*s2 EQ 0x00 OR *s2 EQ 0xFF) AND (*s1 EQ *(s2+1)))
    {
      if (*s2 EQ 0xff AND *(s2+1) EQ 0xFF)
        return 0;
      
			s1++;
      s2 += 2;
      n += 2;

      if (n >= len)
        return 0;
    }

    if ((*s2 > 0 AND *s2 NEQ 0xff) OR
        (!*s2 AND (*(s2+1) > *s1))    )
      return -1;

    return 1;
  }
  return -1;
}


/*
+----------------------------------------------------------------------------+
| PROJECT : MMI-Framework (8417)        MODULE  : PHB                        |
| STATE   : code                        ROUTINE : pb_sim_update_extn_records |
+----------------------------------------------------------------------------+

  PURPOSE : Update reference count for extension record and delete if record 
            is not referenced at all.
            ETSI 11.11 clause 11.5.1 states that reference counts not only 
            have to be maintained for EXT1 but also for other EXT records.
*/
LOCAL T_PHB_RETURN pb_sim_update_extn_records(USHORT ext_field_id, 
                                              USHORT rec_num, 
                                              SHORT ref_type)
{
  T_EXT_TYPE ext_type;
  UBYTE *refptr;
  int db_result;
  UBYTE dummy_ref;

  switch (ext_field_id)
  {
    case SIM_EXT1:    ext_type = EXT1;        break;
    case SIM_EXT2:    ext_type = EXT2;        break;
    case SIM_EXT3:    ext_type = EXT3;        break;
    case SIM_EXT4:    ext_type = EXT4;        break;
    case SIM_EXT5:    ext_type = EXT5;        break;
    case FFS_EXT_LRN: ext_type = EXT_LRN;     break;
    case FFS_EXT_LMN: ext_type = EXT_LMN;     break;
    default:
      return PHB_FAIL;
  }

  if (rec_num <= MAX_EXT_RECORDS)
  {
    refptr = &ext_ref_count[ext_type][rec_num - 1];
  }
  else
  {
    /* Could become improved by using dynamic memory but better as using an
     * illegal array index. Only ref counters will have a problem here. */
    TRACE_ERROR ("SIM exceeds MAX_EXT_RECORDS"); 
    dummy_ref = 1;
    refptr = &dummy_ref;
  }

  if (ref_type EQ -1)
  {
    /* Decrement usage counter */
    if (*refptr > 0)
    {
      *refptr += ref_type;
    }
    else
    {
      TRACE_ERROR ("EXT usage counter below zero");
    }

    if (*refptr EQ 0)
    {
      db_result = db_delete_record(pbs_data.db_handle, ext_field_id, rec_num);
      if(db_result < DB_OK)
        return PHB_FAIL;
    }
  }
  else if (ref_type EQ 1)
  {
    *refptr += ref_type;
  }
  else
    return PHB_FAIL; /* add_val not in -1, +1 */

  TRACE_EVENT_P3 ("Usage count of EXT %04X, record %d = %d", 
                  ext_field_id, rec_num, *refptr);

  return PHB_OK;
}

/*
+----------------------------------------------------------------------------+
| PROJECT : MMI-Framework (8417)        MODULE  : PHB                        |
| STATE   : code                        ROUTINE : pb_sim_del_ext_records     |
+----------------------------------------------------------------------------+

  PURPOSE : Read extension records and update the records.
  
*/
#define RETURN(x) { retVal = x; goto cleanup_exit; }
/*lint -e{801} Use of goto*/
LOCAL T_PHB_RETURN pb_sim_del_ext_records (T_PHB_TYPE type, 
                                           USHORT field_id, 
                                           USHORT db_recno)
{
  int db_result;
  T_PHB_RETURN retVal;
  USHORT ext_file_id;
  T_DB_INFO_FIELD info_field;
  UBYTE *data;
  UBYTE ext_rcd_num;

  TRACE_FUNCTION("pb_sim_del_ext_records()");

  ACI_MALLOC (data, SIM_MAX_RECORD_SIZE);

  /* Get Extension file for the Phonebook type. */
  ext_file_id = pb_sim_get_ext_file(type);

  /* Read record from the database. */
  if(db_info_field(pbs_data.db_handle, field_id, &info_field) NEQ DB_OK)
    RETURN (PHB_FAIL)

  db_result = db_read_record(pbs_data.db_handle, field_id, db_recno, 0, info_field.record_size, data);
  if (db_result EQ DB_EMPTY_RECORD)
    RETURN (PHB_OK) /* Nothing to do */

  if (db_result < DB_OK)
    RETURN (PHB_FAIL) /* Some other problem */

  /* Get extension record identifier */
  ext_rcd_num = data[info_field.record_size - 1];

  while (ext_rcd_num NEQ 0xFF) /* check for extension records */
  {
    if(db_info_field (pbs_data.db_handle, ext_file_id, &info_field) NEQ DB_OK)
      RETURN (PHB_FAIL)

    if ((ext_rcd_num EQ 0) OR (ext_rcd_num > info_field.num_records))
    {
      TRACE_EVENT_P1 ("illegal ext record number %d ignored", ext_rcd_num);
      RETURN (PHB_OK)
    }

    if(db_read_record(pbs_data.db_handle, ext_file_id, ext_rcd_num, 0, info_field.record_size, data) < DB_OK) 
      RETURN (PHB_FAIL)

    if (pb_sim_update_extn_records(ext_file_id, ext_rcd_num, -1) EQ PHB_FAIL)
      RETURN (PHB_FAIL)
    
    ext_rcd_num = data[12];
  }
  RETURN (PHB_OK)

cleanup_exit:
  ACI_MFREE (data);
  return retVal;
}
#undef RETURN

/*
+----------------------------------------------------------------------------+
| PROJECT : MMI-Framework (8417)        MODULE  : PHB                        |
| STATE   : code                        ROUTINE : pb_sim_ext_records_used     |
+----------------------------------------------------------------------------+

  PURPOSE : check if extension records is used by one phone book record (ADN & FDN).
  
*/
#define RETURN(x) { retVal = x; goto cleanup_exit; }
/*lint -e{801} Use of goto*/
LOCAL BOOL pb_sim_ext_records_used (T_PHB_TYPE type, 
                                    USHORT field_id, 
                                    USHORT db_recno)
{
  int db_result;
  BOOL retVal;
  USHORT ext_file_id;
  T_DB_INFO_FIELD info_field;
  UBYTE *data;
  UBYTE ext_rcd_num;

  TRACE_FUNCTION("pb_sim_ext_records_used()");

  ACI_MALLOC (data, SIM_MAX_RECORD_SIZE);

  /* Get Extension file for the Phonebook type. */
  ext_file_id = pb_sim_get_ext_file(type);

  /* Read record from the database. */
  if(db_info_field(pbs_data.db_handle, field_id, &info_field) NEQ DB_OK)
    RETURN (FALSE)

  db_result = db_read_record(pbs_data.db_handle, field_id, db_recno, 0, info_field.record_size, data);
  if (db_result < DB_OK)
    RETURN (FALSE) /* record can not be read - so no EXTn used  */

  /* Get extension record identifier */
  ext_rcd_num = data[info_field.record_size - 1];

  TRACE_EVENT_P1("pb_sim_ext_records_used() --1-- ext_rcd_num = %d", ext_rcd_num);

  if (ext_rcd_num NEQ 0xFF) /* check for extension records */
  {
    if(db_info_field (pbs_data.db_handle, ext_file_id, &info_field) NEQ DB_OK)
      RETURN (FALSE)

    if ((ext_rcd_num EQ 0) OR (ext_rcd_num > info_field.num_records))
    {
      TRACE_EVENT_P1 ("illegal ext record number %d ignored", ext_rcd_num);
      RETURN (FALSE)
    }

    if(db_read_record(pbs_data.db_handle, ext_file_id, ext_rcd_num, 0, info_field.record_size, data) < DB_OK) 
      RETURN (FALSE)
    
    ext_rcd_num = data[12];
  TRACE_EVENT_P1("pb_sim_ext_records_used() --2-- ext_rcd_num = %d", ext_rcd_num);

	 RETURN (TRUE) //return TRUE here since ext_rcd_num is valid
  }
  else
  {
  	RETURN (FALSE)
  }
cleanup_exit:
  ACI_MFREE (data);
  return retVal;
}
#undef RETURN

/*
+--------------------------------------------------------------------+
| PROJECT : MMI-Framework (8417)        MODULE  : PHB                                       |
| STATE   : code                       ROUTINE : pb_sim_retrieve_rdm_recno       |
+--------------------------------------------------------------------+

  PURPOSE : Retrieving record number for LDN, LMN and LRN Phonebook.
  
*/

LOCAL USHORT pb_sim_retrieve_rdm_recno (T_PHB_TYPE type)
{
  USHORT rec_no;

  TRACE_FUNCTION("pb_sim_retrieve_rdm_recno()");

  switch(type)
  {
    case LDN:
      rec_no = LDN_DATA_RECNO;
      break;

    case LMN:
      rec_no = LMN_DATA_RECNO;
      break;
  
    case LRN:
      rec_no = LRN_DATA_RECNO;
      break;

    default:
      rec_no = 0;
      break;
  }

  return rec_no;
}

/*
+--------------------------------------------------------------------+
| PROJECT : MMI-Framework (8417)        MODULE  : PHB                |
| STATE   : code                        ROUTINE : pb_sim_cmpWild     |
+--------------------------------------------------------------------+

  PURPOSE : compare two strings considering wildcard.
            if s1 < s2 return value < 0
            if s1 = s2 return value = 0
            if s1 > s2 return value > 0
*/

GLOBAL int pb_sim_cmpWild ( const char *s1, const char *s2, size_t cmp_len )
{
  TRACE_FUNCTION("pb_sim_cmpWild()");

  if (cmp_len EQ 0)
    return 0; /* Nothing to compare */

  while ((*s1 EQ *s2) OR (*s1 EQ PHB_WILD_CRD) OR (*s2 EQ PHB_WILD_CRD))
  {
    /* Character matches */
    cmp_len--;

    if (cmp_len EQ 0)
      return 0;

    if ((*s1 EQ '\0') AND (*s2 EQ '\0'))
      return 0;

    if (*s2 EQ '\0') /* Means *s1 > '\0' */
      return 1;

    if (*s1 EQ '\0') /* Means *s2 > '\0' */
      return -1;

    s1++;
    s2++;
  }
  /* Character does not match */
  if (*s1 > *s2)
    return 1;
  else
    return -1;

}


/* Implements Measure #30 */
/*
+------------------------------------------------------------------------+
| PROJECT: MMI-Framework (8417) MODULE: PHB                              |
| STATE  : code                 ROUTINE: pb_sim_read_alpha_num_record    |
+------------------------------------------------------------------------+

    PURPOSE : Read a given physical record from the flash based phonebook
              in alphabetical order Or in number sorted order
*/

GLOBAL T_PHB_RETURN pb_sim_read_alpha_num_record (T_PHB_TYPE    type, 
                                                  USHORT        order_num,
                                                  T_PHB_RECORD *entry,
                                                  UBYTE         sort_index)
{
  int db_result;
  USHORT field_id;
  
  TRACE_FUNCTION("pb_sim_read_alpha_num_record()");

  /* Get Elementary file ID for the Phonebook type. */
  field_id = pb_sim_get_field_id(type);

  /* Read record from the FFS. */
  db_result = db_get_phy_from_idx(pbs_data.db_handle, field_id, 
                                  sort_index, order_num);

  if(db_result > DB_OK)
  {
    return pb_sim_read_record(type, (USHORT)db_result, entry) ;
  }

  /* Check whether index is vaild. */
  if(db_result EQ DB_INVALID_INDEX)
  {
    return PHB_INVALID_IDX;
  }

  /* 
   * Unable to get record from the database. 
   * Hence returning PHB_FAIL.
   */
  return PHB_FAIL;
}

/* Implements Measure #167 */
/*
+------------------------------------------------------------------------+
| PROJECT: MMI-Framework (8417) MODULE: PHB                              |
| STATE  : code                 ROUTINE: pb_sim_read_alpha_num_record    |
+------------------------------------------------------------------------+

    PURPOSE : Function check for extention records
*/
LOCAL T_PHB_RETURN pb_sim_update_index (T_PHB_TYPE    type, 
                                        T_DB_CHANGED *rec_affected,
                                        USHORT        field_id,
                                        UBYTE         ext_rec_cnt)
{
  TRACE_FUNCTION("pb_sim_update_index()");

  if((type NEQ LDN) AND (type NEQ LMN) AND (type NEQ LRN))
  {
    /* Update the sorted indexes. */
    if(db_update_index(pbs_data.db_handle, field_id, NAME_IDX, 
                       pb_sim_alpha_cmp, PHB_MATCH_PARTIAL) NEQ DB_OK)
    {
      return PHB_FAIL;
    }

    if(db_update_index(pbs_data.db_handle, field_id, NUMBER_IDX, 
                       pb_sim_alpha_cmp, PHB_MATCH_PARTIAL) NEQ DB_OK)
    {
      return PHB_FAIL;
    }
  }
  return PHB_OK;
}

void      pb_update_cphs_mb_ext_record(void)
{
   UBYTE i; 
   USHORT ext_file_id; 

/* Get Extension file for the Phonebook type. */
  ext_file_id = SIM_EXT1; 

  for(i =0; i< 4; i++)
  {
     if(cphs_mb_ext_record_num[i] NEQ 0xff)
     {
        db_update_ext_bitmap(pbs_data.db_handle,ext_file_id,cphs_mb_ext_record_num[i],TRUE); 
        pb_sim_update_extn_records(ext_file_id, cphs_mb_ext_record_num[i], 1); 
     }
  }
}
GLOBAL void  pb_sim_update_ext_bitmap(UBYTE rec_num, BOOL flag)
 {
  USHORT ext_file_id; 

/* Get Extension file for the Phonebook type. */
  ext_file_id = SIM_EXT1;

    db_update_ext_bitmap(pbs_data.db_handle,ext_file_id,rec_num, flag); 
    if(flag EQ TRUE)
     {
        pb_sim_update_extn_records(ext_file_id, rec_num, 1);
     }
    else
     {
       pb_sim_update_extn_records(ext_file_id, rec_num,  -1);
     }
 }


#endif /* #ifdef TI_PS_FFS_PHB */


