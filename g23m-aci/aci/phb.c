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
|  Purpose :  This modul contains the functions to establish the phone books.
+----------------------------------------------------------------------------- 
*/ 

#include "config.h"
#include "fixedconf.h"
#include "condat-features.h"
#include "aci_conf.h"

#ifndef TI_PS_FFS_PHB

#include "aci_all.h"

#include "aci_cmh.h"
#include "ati_cmd.h"
#include "aci_cmd.h"
#include "aci.h"
#include "p_sim.h"
#include "pcm.h"
#include "gdi.h"

#include "phb.h"
#include "psa.h"
#include "psa_sim.h"
#include "psa_cc.h"
#ifdef SIM_TOOLKIT
#include "psa_sat.h"
#endif

#ifdef FAX_AND_DATA
#include "aci_fd.h"
#endif    /* of #ifdef FAX_AND_DATA */

#include "cmh.h"
#include "cmh_phb.h"

#include "dti_conn_mng.h"


#include "cmh_sim.h"
#include "psa_mm.h"
#include "psa_ss.h"

#ifndef _SIMULATION_
#include "../../services/ffs/ffs.h"
#endif
/********* current define *******************************************/
static T_PHB_EXT_CMP_FCT   ext_compare_fct = NULL;  /* external compare function */
static T_PHB_CTB           phb_ctb[MAX_PHONEBOOK];
static T_PHB_AFB_ELEMENT   phb_element[MAX_AFB_RECORDS];
static T_PHB_RDM_ELEMENT   phb_l_element[MAX_RDM_RECORDS];
static UBYTE               adn_bitmap[MAX_ADN_BITMAP];
static UBYTE               fdn_bitmap[MAX_FDN_BITMAP];
static UBYTE               bdn_bitmap[MAX_BDN_BITMAP];
static UBYTE               sdn_bitmap[MAX_SDN_BITMAP];
static UBYTE               ecc_bitmap[MAX_ECC_BITMAP];
static UBYTE               upn_bitmap[MAX_UPN_BITMAP];
#ifdef PHONEBOOK_EXTENSION
static T_PHB_EXT_RECORDS   phb_ext_records[MAX_PHB_EXT];
static UBYTE               ext1_bitmap[MAX_EXT1_BITMAP];
static UBYTE               ext2_bitmap[MAX_EXT2_BITMAP];
static UBYTE               ext3_bitmap[MAX_EXT3_BITMAP];
static UBYTE               ext4_bitmap[MAX_EXT4_BITMAP];
#endif
static UBYTE   sim_service_table[MAX_SRV_TBL];  /* SIM service table */
static UBYTE   data [256];

static SHORT   ext_index;
static UBYTE   max_ext_chain_reads=0;
static UBYTE   phb_stat;
static UBYTE   fdn_mode;
static T_ACI_CLASS   fdn_classtype;
static T_ACI_CLASS   fdn_input_classtype;
static UBYTE   read_flag = 0;
static SHORT   db_index = UNUSED_INDEX; /* memory index for delete whole phonebook */
static BOOL    sstUpdateId = FALSE; /* SIM service table update indication (SAT) */

static UBYTE   max_sim_LDN_records = 0; /* to ensure that every physical record is written */

static int cmpString (UBYTE *s1, UBYTE *s2, UBYTE len);
static int pb_cmp_phb_entry ( UBYTE *pb_tag, UBYTE pb_len,
                              T_ACI_PB_TEXT *search_tag );
static int pb_cmp2Bytes(UBYTE *s1, UBYTE *s2, UBYTE len, UBYTE flag);
static void pb_cvt_alpha_for_cmp ( UBYTE *src,
                                   UBYTE *dst,
                                   UBYTE len );
static BOOL    imsiFlag;

static BOOL pause_pb_reading_while_EXT_reading = FALSE; /* pauses the loop while reading EXT-Records */
static SHORT paused_table_id = 0;                       /* the paused record */
EXTERN T_PCEER causeMod;
EXTERN SHORT causeCeer;

#ifdef SIM_TOOLKIT
BOOL pb_update (int ref, T_SIM_FILE_UPDATE_IND *fu);
BOOL pb_update_ecc_fu (int ref, T_SIM_FILE_UPDATE_IND *fu);
#endif
void pb_copy_ecc_entry (UBYTE *ecc, UBYTE num);
void pb_read_sim_ecc ( void );
void pb_read_eeprom_ecc (void);
T_PHB_RETURN pb_read_eeprom_req(void);
BOOL pb_read_sim_dat(USHORT data_id, UBYTE len, UBYTE max_length);
void pb_read_sim_dat_cb(SHORT table_id);
void pb_read_sim_req(void);
void pb_sat_update_reset (USHORT data_id);
void pb_init_afb(void);
void pb_init_ctb (T_PHB_TYPE book);
void pb_init_element (UBYTE book);
void pb_init_l_element (UBYTE book); /*CQ16301: Added support for LND Refresh*/

BOOL pb_init_sync_sim(UBYTE type);
BOOL pb_prepare_sync_sim(UBYTE type, UBYTE rcd_num);
BOOL pb_sync_sim(UBYTE type, UBYTE rcd_num);
void pb_sync_sim_cb(SHORT table_id);
void pb_finish_sync_sim(void);

void copy_phb_element ( T_PHB_RECORD *entry, T_PHB_AFB_ELEMENT phb_element );
void copy_phb_l_element ( T_PHB_RECORD *entry, T_PHB_RDM_ELEMENT phb_l_element );


LOCAL USHORT pb_get_ext_file_id (UBYTE pb_type);
LOCAL void pb_prepare_ext_data(UBYTE *number, UBYTE no_len,
                               UBYTE *subaddr, UBYTE sub_len,
                               USHORT file_id);
LOCAL void pb_free_used_record(UBYTE type, SHORT index, UBYTE rec_num);
/*
+--------------------------------------------------------------------+
| PROJECT: MMI-Framework (8417)         MODULE: PHB                  |
| STATE  : code                         ROUTINE: pb_sat_update_reset |
+--------------------------------------------------------------------+

    PURPOSE : Initialisation of phonebook field for SAT REFRESH
*/

void pb_sat_update_reset (USHORT data_id)
{
  TRACE_FUNCTION("pb_sat_update_reset()");

#ifdef SIM_TOOLKIT
  switch (data_id)
  {
    /* ACI-ENH-17240: Subissue of CQ 16303, ADN and FDN are updated 
       if one of them has changed */
    case SIM_ADN:
     /*  
      * pb_init_element (ADN);
      * pb_init_ctb (ADN);
      * phb_ctb[ADN].rcd_bitmap = adn_bitmap;
      * memset(phb_ctb[ADN].rcd_bitmap, 0, MAX_ADN_BITMAP);
      * break;
      */
    case SIM_FDN:
     /*
      * pb_init_element (FDN);
      * pb_init_ctb (FDN);
      * phb_ctb[FDN].rcd_bitmap = fdn_bitmap;
      * memset(phb_ctb[FDN].rcd_bitmap, 0, MAX_FDN_BITMAP);
      */
      pb_init_afb();
      break; 
      
    case SIM_BDN:
      pb_init_element (BDN);
      pb_init_ctb (BDN);
      phb_ctb[BDN].rcd_bitmap = bdn_bitmap;
      memset(phb_ctb[BDN].rcd_bitmap, 0, MAX_BDN_BITMAP);
      break;

    case SIM_SDN:
      pb_init_element (SDN);
      pb_init_ctb (SDN);
      phb_ctb[SDN].rcd_bitmap = sdn_bitmap;
      memset(phb_ctb[SDN].rcd_bitmap, 0, MAX_SDN_BITMAP);
      break;

    case SIM_MSISDN:
      pb_init_element (UPN);
      pb_init_ctb (UPN);
      phb_ctb[UPN].rcd_bitmap = upn_bitmap;
      memset(phb_ctb[UPN].rcd_bitmap, 0, MAX_UPN_BITMAP);
      break;
      
    /* CQ16301: Added support for LND refresh triggered by SAT */ 
    case SIM_LND:
      pb_init_l_element (LDN);
      pb_init_ctb (LDN);
      break;

    default:
      break;
  }
#endif
}

/*
+--------------------------------------------------------------------+
| PROJECT: MMI-Framework (8417)         MODULE: PHB                  |
| STATE  : code                         ROUTINE: pb_init_ctb         |
+--------------------------------------------------------------------+

    PURPOSE : Initialisation of the phonebook control block, 
              but not for bitmap field
*/

void pb_init_ctb (T_PHB_TYPE book)
{
  TRACE_FUNCTION("pb_init_ctb()");

  phb_ctb[book].mem         = NO_PHB_ENTRY;
  phb_ctb[book].alpha_len   = 0;
  phb_ctb[book].max_rcd     = 0;
  phb_ctb[book].used_rcd    = 0;
  phb_ctb[book].first_rcd   = UNUSED_INDEX;
  phb_ctb[book].first_trcd  = UNUSED_INDEX;
  phb_ctb[book].first_nrcd  = UNUSED_INDEX;
  phb_ctb[book].first_mtrcd = UNUSED_INDEX;
  phb_ctb[book].first_mnrcd = UNUSED_INDEX;
}

/*
+--------------------------------------------------------------------+
| PROJECT: MMI-Framework (8417)         MODULE: PHB                  |
| STATE  : code                         ROUTINE: pb_init_element     |
+--------------------------------------------------------------------+

    PURPOSE : Initialisation of the saved phonebook element
*/

void pb_init_element (UBYTE book)
{
  SHORT index;

  TRACE_FUNCTION("pb_init_element()");

  index = phb_ctb[book].first_rcd;
  while (index NEQ UNUSED_INDEX)
  {
    phb_element[index].free = PHB_ELEMENT_FREE;
    index = phb_element[index].next_rcd;
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT: MMI-Framework (8417)         MODULE: PHB                  |
| STATE  : code                         ROUTINE: pb_init_l_element     |
+--------------------------------------------------------------------+

    PURPOSE : Initialisation of the saved phonebook element (ME) 
*/

void pb_init_l_element (UBYTE book)
{
  SHORT index;

  TRACE_FUNCTION("pb_init_l_element()");

  index = phb_ctb[book].first_rcd;
  while (index NEQ UNUSED_INDEX)
  {
    phb_l_element[index].free = PHB_ELEMENT_FREE;
    index = phb_element[index].next_rcd;
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT: MMI-Framework (8417)         MODULE: PHB                  |
| STATE  : code                         ROUTINE: phb_Init            |
+--------------------------------------------------------------------+

    PURPOSE : Power-on initialisation of the phonebook module
*/

void phb_Init (void)
{
  fdn_mode = NO_OPERATION;
   /* set fdn_classtype to default value */
  fdn_classtype = CLASS_VceDatFaxSms;
  fdn_input_classtype = fdn_classtype;
#ifdef SIM_TOOLKIT
  simShrdPrm.fuRef=-1;
  if (!psaSAT_FURegister (pb_update))
  {
    TRACE_EVENT ("FAILED to register the handler pb_update() for FU");
  }
  if (!psaSAT_FURegister (pb_update_ecc_fu))
  {
    TRACE_EVENT ("FAILED to register the handler pb_update_ecc_fu() for FU");
  }

#endif
}

/*
+--------------------------------------------------------------------+
| PROJECT: MMI-Framework (8417)         MODULE: PHB                  |
| STATE  : code                         ROUTINE: pb_init             |
+--------------------------------------------------------------------+

    PURPOSE :
*/

void pb_init (void)
{
  TRACE_FUNCTION ("pb_init()");

  if (fdn_mode EQ NO_OPERATION)
  {
    {
      T_PHB_TYPE i;

      /* Initiate the bitmap in control block */
      for (i=0; i<MAX_PHONEBOOK; i++)
      {
        pb_init_ctb(i);

        switch(i)
        {
          case ECC:
            phb_ctb[i].rcd_bitmap = ecc_bitmap;
            memset(phb_ctb[i].rcd_bitmap, 0, MAX_ECC_BITMAP);
            break;
          case ADN:
            phb_ctb[i].rcd_bitmap = adn_bitmap;
            memset(phb_ctb[i].rcd_bitmap, 0, MAX_ADN_BITMAP);
            break;
          case FDN:
            phb_ctb[i].rcd_bitmap = fdn_bitmap;
            memset(phb_ctb[i].rcd_bitmap, 0, MAX_FDN_BITMAP);
            break;
          case BDN:
            phb_ctb[i].rcd_bitmap = bdn_bitmap;
            memset(phb_ctb[i].rcd_bitmap, 0, MAX_BDN_BITMAP);
            break;
          case SDN:
            phb_ctb[i].rcd_bitmap = sdn_bitmap;
            memset(phb_ctb[i].rcd_bitmap, 0, MAX_SDN_BITMAP);
            break;
          case UPN:
            phb_ctb[i].rcd_bitmap = upn_bitmap;
            memset(phb_ctb[i].rcd_bitmap, 0, MAX_UPN_BITMAP);
            break;
        }
      }
    }
#ifdef PHONEBOOK_EXTENSION
    {
      T_PHB_EXT_TYPE i;
      /* Initiate the bitmap for the phonebook extention records */
      for (i = 0; i < MAX_PHB_EXT; i++)
      {
        phb_ext_records[i].mem     = NO_PHB_ENTRY;
        phb_ext_records[i].max_rcd = 0;
        switch (i)
        {
          case EXT1:
            phb_ext_records[i].rcd_bitmap = ext1_bitmap;
            memset(phb_ext_records[i].rcd_bitmap, 0, MAX_EXT1_BITMAP); /* ADN; LDN */
            break;

          case EXT2:
            phb_ext_records[i].rcd_bitmap = ext2_bitmap;
            memset(phb_ext_records[i].rcd_bitmap, 0, MAX_EXT2_BITMAP); /* FDN */
            break;
          case EXT3:
            phb_ext_records[i].rcd_bitmap = ext3_bitmap;
            memset(phb_ext_records[i].rcd_bitmap, 0, MAX_EXT3_BITMAP); /* SDN */
            break;
          case EXT4:
            phb_ext_records[i].rcd_bitmap = ext4_bitmap;
            memset(phb_ext_records[i].rcd_bitmap, 0, MAX_EXT4_BITMAP); /* BDN */
            break;
        }
      }
    }
#endif
    {
      int i;
      /* Initiate the free element */
      for (i=0; i<MAX_AFB_RECORDS; i++)
        phb_element[i].free = PHB_ELEMENT_FREE;
      for (i=0; i<MAX_RDM_RECORDS; i++)
        phb_l_element[i].free = PHB_ELEMENT_FREE;
    }

    phb_stat = PHB_UNKNOWN;
    cmhPHB_StatIndication ( PHB_UNKNOWN, CME_ERR_NotPresent, TRUE );
  }
}


/*
+--------------------------------------------------------------------+
| PROJECT: MMI-Framework (8417)         MODULE: PHB                  |
| STATE  : code                         ROUTINE: pb_init_afb         |
+--------------------------------------------------------------------+


    PURPOSE :

*/

void pb_init_afb(void)
{
  T_PHB_TYPE i;
  SHORT index;
  SHORT cur_index;

  TRACE_FUNCTION ("pb_init_afb()");

  /* Initiate the bitmap in control block */
  for (i=0; i<MAX_PHONEBOOK; i++)
  {
    switch (i)
    {
      case ADN:
      case FDN:
        index = phb_ctb[i].first_rcd;
        while (index != UNUSED_INDEX)
        {
          cur_index = index;
          index = phb_element[cur_index].next_rcd;
          phb_element[cur_index].free = PHB_ELEMENT_FREE;
          phb_element[cur_index].prev_rcd = UNUSED_INDEX;
          phb_element[cur_index].next_rcd = UNUSED_INDEX;
        }
        pb_init_ctb(i);
        switch(i)
        {
          case ADN:
            phb_ctb[i].rcd_bitmap = adn_bitmap;
            memset(phb_ctb[i].rcd_bitmap, 0, MAX_ADN_BITMAP);
            break;
          case FDN:
            phb_ctb[i].rcd_bitmap = fdn_bitmap;
            memset(phb_ctb[i].rcd_bitmap, 0, MAX_FDN_BITMAP);
            break;
        }
        break;
      case ADN_FDN:
        pb_init_ctb(i);
        break;
      default:
        break;
    }
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT: MMI-Framework (8417)         MODULE: PHB                  |
| STATE  : code                         ROUTINE: pb_reset            |
+--------------------------------------------------------------------+

    PURPOSE : Invalidate phonebook buffered in RAM

*/

void pb_reset (void)
{
  TRACE_FUNCTION("pb_reset()");

  pb_write_eeprom();

  fdn_mode = NO_OPERATION;  /* some more stuff may be needed */
  /* set fdn_classtype to default value */
  fdn_classtype = CLASS_VceDatFaxSms;
  fdn_input_classtype = fdn_classtype;
  cmhPHB_StatIndication ( PHB_UNKNOWN, CME_ERR_NotPresent, TRUE );
}

/*
+--------------------------------------------------------------------+
| PROJECT: MMI-Framework (8417)         MODULE: PHB                  |
| STATE  : code                         ROUTINE: pb_create_memory    |
+--------------------------------------------------------------------+


    PURPOSE : Find the next free entry

*/

T_PHB_RETURN pb_create_memory(SHORT *index)
{
  int i;

  /* TRACE_FUNCTION("pb_create_memory()"); */

  for (i=0; i<MAX_AFB_RECORDS; i++)
  {
    if (phb_element[i].free EQ PHB_ELEMENT_FREE)
    {
      memset ((char *)&phb_element[i].entry, 0xff, sizeof (T_AFB_RECORD));
      phb_element[i].free = PHB_ELEMENT_USED;
      *index = i;
      return PHB_OK;
    }
  }

  return PHB_FULL;
}


/*
+--------------------------------------------------------------------+
| PROJECT: MMI-Framework (8417)         MODULE: PHB                  |
| STATE  : code                         ROUTINE: pb_create_l_memory  |
+--------------------------------------------------------------------+


    PURPOSE : Find the next free entry

*/

T_PHB_RETURN pb_create_l_memory(SHORT *index)
{
  SHORT i;

  /* TRACE_FUNCTION("pb_create_l_memory()"); */

  for (i=0; i<MAX_RDM_RECORDS; i++)
  {
    if (phb_l_element[i].free EQ PHB_ELEMENT_FREE)
    {
      memset ((char *)&phb_l_element[i].entry, 0xff, sizeof (T_RDM_RECORD));
      phb_l_element[i].free = PHB_ELEMENT_USED;
      *index = i;
      return PHB_OK;
    }
  }

  return PHB_FULL;
}


/*
+--------------------------------------------------------------------+
| PROJECT: MMI-Framework (8417)         MODULE: PHB                  |
| STATE  : code                         ROUTINE: pb_read_ecc         |
+--------------------------------------------------------------------+


    PURPOSE :   Build emergency call phonebook.

*/
T_PHB_RETURN pb_read_ecc (USHORT error, UBYTE ecc_len, UBYTE *sim_ecc)
{
  UBYTE             *data_ptr;
  int                i;

  TRACE_FUNCTION ("pb_read_ecc()");

  if (fdn_mode != NO_OPERATION)
    return PHB_OK;

  /* If SIM card is not active, the emergency call numbers are read from EEPROM */
  if (   ( error EQ SIM_CAUSE_OTHER_ERROR )
      OR ( error EQ SIM_CAUSE_CARD_REMOVED)
      OR ( error >= SIM_CAUSE_PARAM_WRONG AND error <= SIM_CAUSE_DRV_TEMPFAIL) )
  {
    pb_read_eeprom_ecc();
  }

  /* SIM card is active, the emergency call numbers are read from SIM card */
  else
  {
    /* if SIM ECC data is not empty, copy SIM ECC data to phonebook */
    if ( strcmp((CHAR*)sim_ecc,"") )
    {
      phb_ctb[ECC].mem = SIM_MEMORY;
      data_ptr         = sim_ecc;
      phb_ctb[ECC].max_rcd = (SHORT)((ecc_len/3) > MAX_ECC_RCD)? MAX_ECC_RCD: ecc_len/3;

      phb_ctb[ECC].type    = ECC;
      phb_ctb[ECC].first_trcd = UNUSED_INDEX;

      /* Read emergency call number */
      for (i=0; i<MAX_ECC_RCD; i++)
      {
        pb_copy_ecc_entry (data_ptr, (UBYTE)i);
        data_ptr += 3;
      }
    }
  }

  return PHB_OK;
}


/*
+----------------------------------------------------------------------+
| PROJECT: MMI-Framework (8417)         MODULE: PHB                    |
| STATE  : code                         ROUTINE: pb_read_sim           |
+----------------------------------------------------------------------+


    PURPOSE :   SIM card informs the numbers of phonebook record.

*/

BOOL pb_read_sim(USHORT data_id, UBYTE rcd_num, UBYTE len)
{
  SHORT table_id;

  TRACE_FUNCTION ("pb_read_sim()");

  table_id = psaSIM_atbNewEntry();

  if(table_id NEQ NO_ENTRY)
  {
    simShrdPrm.atb[table_id].ntryUsdFlg = TRUE;
    simShrdPrm.atb[table_id].accType    = ACT_RD_REC;
    simShrdPrm.atb[table_id].v_path_info  = FALSE;
    simShrdPrm.atb[table_id].reqDataFld = data_id;
    simShrdPrm.atb[table_id].recNr      = rcd_num;
    if (rcd_num EQ 1)
      simShrdPrm.atb[table_id].dataLen    = NOT_PRESENT_8BIT;
    else
      simShrdPrm.atb[table_id].dataLen    = len;
    simShrdPrm.atb[table_id].exchData   = data;
    simShrdPrm.atb[table_id].rplyCB     = pb_read_cb;

    simShrdPrm.aId = table_id;

    if (pause_pb_reading_while_EXT_reading EQ TRUE)   /* Read request must be paused while EXT is read */
    {
      paused_table_id = simShrdPrm.aId;               /* save the aId for later SIM Access */
    }
    else
    {
      if(psaSIM_AccessSIMData() < 0)
      {
        TRACE_EVENT("FATAL ERROR");
        return FALSE;
      }
    }
  }
  else
    return FALSE;

  return TRUE;
}



/*
+----------------------------------------------------------------------+
| PROJECT: MMI-Framework (8417)         MODULE: PHB                    |
| STATE  : code                         ROUTINE: pb_read_sim_ext       |
+----------------------------------------------------------------------+


    PURPOSE :   SIM card informs the numbers of phonebook record.

*/

#ifdef PHONEBOOK_EXTENSION
void pb_read_sim_ext(USHORT data_id, UBYTE rcd_num)
{
  SHORT table_id;

  TRACE_FUNCTION ("pb_read_sim_ext()");

  table_id = psaSIM_atbNewEntry();

  if(table_id NEQ NO_ENTRY)
  {
    simShrdPrm.atb[table_id].ntryUsdFlg = TRUE;
    simShrdPrm.atb[table_id].accType    = ACT_RD_REC;
    simShrdPrm.atb[table_id].v_path_info  = FALSE;
    simShrdPrm.atb[table_id].reqDataFld = data_id;
    simShrdPrm.atb[table_id].recNr      = rcd_num;
    simShrdPrm.atb[table_id].dataLen    = 13;
    simShrdPrm.atb[table_id].exchData   = data;
    simShrdPrm.atb[table_id].rplyCB     = pb_read_ext_cb;

    simShrdPrm.aId = table_id;

    pause_pb_reading_while_EXT_reading = TRUE;    /* Suspend further ADN reading while EXT is read */

    if(psaSIM_AccessSIMData() < 0)
    {
      TRACE_EVENT("pb_read_sim_ext (): FATAL ERROR");
    }
  }
}
#endif



/*
+----------------------------------------------------------------------+
| PROJECT: MMI-Framework (8417)         MODULE: PHB                    |
| STATE  : code                         ROUTINE: pb_init_sync_sim      |
+----------------------------------------------------------------------+


    PURPOSE :   Sync the local LDN entries to SIM
*/


BOOL pb_init_sync_sim(UBYTE type)
{
  TRACE_FUNCTION ("pb_init_sync_sim()");

  switch (type)
  {
    case LDN:
      break;
    default:    /* Only supported for LDN */
      return FALSE;
  }

  if (phb_ctb[type].service EQ ALLOCATED_AND_ACTIVATED
      AND phb_ctb[type].mem != NO_PHB_ENTRY)
  {
    if (max_sim_LDN_records)    /* start with oldest record */
      return (pb_prepare_sync_sim(type, max_sim_LDN_records));
  }
  return FALSE;
}


/*
+----------------------------------------------------------------------+
| PROJECT: MMI-Framework (8417)         MODULE: PHB                    |
| STATE  : code                         ROUTINE: pb_prepare_sync_sim   |
+----------------------------------------------------------------------+


    PURPOSE :   Sync the local LDN entries to SIM
*/

BOOL pb_prepare_sync_sim(UBYTE type, UBYTE rcd_num)
{
  T_PHB_RECORD   entry;
  T_PHB_RETURN   sim_result;
  UBYTE          tag_len;

  TRACE_FUNCTION ("pb_prepare_sync_sim()");

  switch (type)
  {
    case LDN:
      break;
    default:
      return FALSE;
  }

  memset(data, 0xFF, sizeof(data));
  
  if (pb_read_phys_record( type, rcd_num, &entry) NEQ PHB_OK)
  {
    sim_result = pb_sync_sim(type, rcd_num);   /* Write an empty record */
  }
  else
  {
    tag_len = MINIMUM ( phb_ctb[type].alpha_len, entry.tag_len );
    memcpy(data, entry.tag, tag_len);
    data[phb_ctb[type].alpha_len] = entry.len+1;
/*#if PHONEBOOK_EXTENSION*/
#if 0
    if (entry.number[10] NEQ 0xFF)
    {
      data[phb_ctb[type].alpha_len] = 11; /* max. length */
    }
    else
    {
      data[phb_ctb[type].alpha_len] = entry.len+1;
    }
#else
    data[phb_ctb[type].alpha_len] = entry.len+1;
#endif
    data[phb_ctb[type].alpha_len+1] = entry.ton_npi;
    memcpy((char *)&data[phb_ctb[type].alpha_len+2], 
           (char *)entry.number, 10);
    data[phb_ctb[type].alpha_len+12] = entry.cc_id;

/*#ifdef PHONEBOOK_EXTENSION*/
#if 0
    if (entry->number[10] NEQ 0xFF)
    {
      file_id = pb_get_ext_file_id (type);
      if (old_ext_rcd_num NEQ 0xFF)
      {
        /* use the old extention record */
        phb_element[new_index].entry.ext_rcd_num = old_ext_rcd_num;
      }
      else
      {
        phb_element[new_index].entry.ext_rcd_num = pb_get_ext_record_number (type);
      }
      data[phb_ctb[type].alpha_len+13] = phb_element[new_index].entry.ext_rcd_num;
    }
    /* only number extention or subaddress could be store */
    else if (entry->subaddr[0] NEQ 0xFF)
    {
      file_id = pb_get_ext_file_id (type);
      if (old_ext_rcd_num NEQ 0xFF)
      {
        /* use the old extention record */
        phb_element[new_index].entry.ext_rcd_num = old_ext_rcd_num;
      }
      else
      {
        phb_element[new_index].entry.ext_rcd_num = pb_get_ext_record_number (0xFF);
      }
      data[phb_ctb[type].alpha_len+13] = phb_element[new_index].entry.ext_rcd_num;
    }
#endif
    sim_result = pb_sync_sim(type, rcd_num);   /* Record is always 1 for cyclic files */

/*#ifdef PHONEBOOK_EXTENSION*/
#if 0
    if (sim_result NEQ PHB_FAIL)
    {
      if (phb_element[new_index].entry.ext_rcd_num NEQ 0xFF)
      {
        pb_prepare_ext_data (phb_element[new_index].entry.number,
                             phb_element[new_index].entry.len,
                             phb_element[new_index].entry.subaddr,
                             10,
                             file_id);
        sim_result = pb_write_sim_ext(file_id, phb_element[new_index].entry.ext_rcd_num);
      }
      else if (old_ext_rcd_num NEQ 0xFF)
      {
        /* delete the old extention record */
        pb_rem_ext_record_flag (type, old_ext_rcd_num);
        pb_prepare_ext_data (NULL, 0, NULL, 0, file_id);
        sim_result = pb_write_sim_ext(SIM_EXT1, old_ext_rcd_num);
      }
    }
#endif /* PHONEBOOK_EXTENSION */
  }
  if (sim_result NEQ PHB_FAIL)
    return (TRUE);
  return TRUE;
}


/*
+----------------------------------------------------------------------+
| PROJECT: MMI-Framework (8417)         MODULE: PHB                    |
| STATE  : code                         ROUTINE: pb_sync_sim           |
+----------------------------------------------------------------------+


    PURPOSE :   Sync the local LDN entries to SIM
*/

BOOL pb_sync_sim (UBYTE type, UBYTE rcd_num)
{
  SHORT table_id;
  USHORT data_id;

  TRACE_FUNCTION ("pb_sync_sim()");

  switch (type)
  {
    case LDN:
      data_id = SIM_LND;
      break;
    default:
      return FALSE;
  }

  table_id = psaSIM_atbNewEntry();
  if (table_id EQ NO_ENTRY)
  {
    TRACE_ERROR ("pb_sync_sim(): no more table entries");
    return (FALSE);
  }

  simShrdPrm.atb[table_id].ntryUsdFlg = TRUE;
  simShrdPrm.atb[table_id].accType    = ACT_WR_REC;
  simShrdPrm.atb[table_id].v_path_info  = FALSE;
  simShrdPrm.atb[table_id].reqDataFld = data_id;
  simShrdPrm.atb[table_id].recNr      = rcd_num;
  simShrdPrm.atb[table_id].dataLen    = phb_ctb[type].alpha_len + 14;
  simShrdPrm.atb[table_id].exchData   = data;
  simShrdPrm.atb[table_id].rplyCB     = pb_sync_sim_cb;

  simShrdPrm.aId = table_id;


  if(psaSIM_AccessSIMData() < 0)
  {
    return (FALSE);
  }

  phb_stat = PHB_BUSY;
  cmhPHB_StatIndication ( PHB_BUSY, CME_ERR_NotPresent, TRUE );

  /*  return (PHB_EXCT);*/
  return (TRUE);
}




/*
+------------------------------------------------------------------+
| PROJECT : MMI-Framework (8417)         MODULE  : PHB             |
| STATE   : code                         ROUTINE : pb_sync_sim_cb  |
+------------------------------------------------------------------+


    PURPOSE :   Call back for sync phonebook in SIM card.

*/

void pb_sync_sim_cb(SHORT table_id)
{
  UBYTE   type;
  USHORT  type_id;
  UBYTE   rcd_num;

  TRACE_FUNCTION("pb_sync_sim_cb()");

  simShrdPrm.atb[table_id].ntryUsdFlg = FALSE;

  if (simShrdPrm.atb[table_id].errCode NEQ SIM_NO_ERROR)
  {
    TRACE_ERROR("pb_sync_sim_cb(): error for writing");
    /* return; */    /* dont stop writing here if one record fails since must reach?
                        pb_finish_sync_sim to deactivate the sim */
  }

  /* Inform the data of record */
  switch (simShrdPrm.atb[table_id].reqDataFld)
  {
    case SIM_LND:         /* Up to now only LDN supported */
      type    = LDN;
      type_id = SIM_LND;
      break;
    default:
      TRACE_FUNCTION("pb_sync_sim_cb() invalid callback");
      return;
  }

  rcd_num = simShrdPrm.atb[table_id].recNr;
  if (--rcd_num)
  {
    pb_prepare_sync_sim(type, rcd_num);      /* sync next record */
    return;
  }
  else             /* Last record copied to SIM */
  {
    pb_finish_sync_sim();
  }
  return;
}



/*
+---------------------------------------------------------------------+
| PROJECT : MMI-Framework (8417)         MODULE  : PHB                |
| STATE   : code                         ROUTINE : pb_finish_sync_sim |
+---------------------------------------------------------------------+


    PURPOSE :   Call back for sync phonebook in SIM card.

*/

void pb_finish_sync_sim()
{
    phb_stat = PHB_READY;

    cmhPHB_StatIndication ( PHB_READY, CME_ERR_NotPresent, TRUE );

    pb_reset();
    pb_init();

    simShrdPrm.synCs = SYNC_DEACTIVATE;     /* This was moved from pb_sync_sim_ldn */
    psaSIM_SyncSIM();

    return;
}





/*
+-------------------------------------------------------------------+
| PROJECT : MMI-Framework (8417)         MODULE  : PHB              |
| STATE   : code                         ROUTINE : pb_copy_sim_entry|
+-------------------------------------------------------------------+


    PURPOSE :   SIM card informs the numbers of phonebook record.

*/

void pb_copy_sim_entry(SHORT cur_index)
{
  UBYTE *ptr;
  UBYTE max_tag_len;
#ifdef PHONEBOOK_EXTENSION
  USHORT file_id;
#endif

  TRACE_FUNCTION ("pb_copy_sim_entry()");

  ptr = data;
  max_tag_len = MINIMUM (phb_ctb[phb_element[cur_index].type].alpha_len,
                         PHB_MAX_TAG_LEN);
  phb_element[cur_index].entry.tag_len = (UBYTE)pb_get_entry_len(ptr, max_tag_len);
  memset(phb_element[cur_index].entry.tag, 0xFF, PHB_MAX_TAG_LEN); /* init the tag value */
  memcpy ( (char*)phb_element[cur_index].entry.tag, 
           (char*)ptr, 
           phb_element[cur_index].entry.tag_len );

  ptr += phb_ctb[phb_element[cur_index].type].alpha_len;
  phb_element[cur_index].entry.len     = *(ptr++) - 1;
  phb_element[cur_index].entry.ton_npi = *ptr++;

  /* 
   * This error handling is done to avoid the accidental incorrect 
   * record length stored in the test SIMs 
   */
  if (phb_element[cur_index].entry.len > PHB_PACKED_NUM_LEN)
  {
     phb_element[cur_index].entry.len = PHB_PACKED_NUM_LEN;
  }  

  memset(phb_element[cur_index].entry.number, 0xFF, PHB_PACKED_NUM_LEN);
  memcpy( (char*)phb_element[cur_index].entry.number, (char*)ptr, phb_element[cur_index].entry.len );
  ptr += 10;
  phb_element[cur_index].entry.cc_id     = *ptr++;

#ifdef PHONEBOOK_EXTENSION
  if (*ptr != 0xFF) /* check for extention records */
  {
    file_id = pb_get_ext_file_id(phb_element[cur_index].type);
    if (file_id != 0xFFFF)
    {
      phb_element[cur_index].entry.ext_rcd_num = (UBYTE)*ptr;
      ext_index = cur_index;
      max_ext_chain_reads=5;    /* Limit the number of additional EXT reads per ADN record to avoid a possible endless loop */
      pb_read_sim_ext(file_id, phb_element[cur_index].entry.ext_rcd_num);
    }
  }
  else
  {
    phb_element[cur_index].entry.ext_rcd_num = 0xFF;
  }
#endif
}


/*
+-------------------------------------------------------------------+
| PROJECT : MMI-Framework (8417)         MODULE  : PHB              |
| STATE   : code                         ROUTINE : pb_copy_sim_entry|
+-------------------------------------------------------------------+


    PURPOSE :   SIM card informs the numbers of phonebook record.

*/

void pb_copy_sim_ldn_entry(SHORT cur_index)
{
  UBYTE *ptr;
  UBYTE max_tag_len;
#ifdef PHONEBOOK_EXTENSION
/*  USHORT file_id;*/
#endif

  TRACE_FUNCTION ("pb_copy_sim_ldn_entry()");

  ptr = data;
  max_tag_len = MINIMUM (phb_ctb[phb_l_element[cur_index].type].alpha_len,
                         PHB_MAX_TAG_LEN);
  phb_l_element[cur_index].entry.tag_len = (UBYTE)pb_get_entry_len(ptr, max_tag_len);
  memset(phb_l_element[cur_index].entry.tag, 0xFF, PHB_MAX_TAG_LEN); /* init the tag value */
  memcpy ( (char*)phb_l_element[cur_index].entry.tag, 
           (char*)ptr, 
           phb_l_element[cur_index].entry.tag_len );

  ptr += phb_ctb[phb_l_element[cur_index].type].alpha_len;
  phb_l_element[cur_index].entry.len     = *(ptr++) - 1;
  phb_l_element[cur_index].entry.ton_npi = *ptr++;
  memset(phb_l_element[cur_index].entry.number, 0xFF, PHB_PACKED_NUM_LEN);
  memcpy( (char*)phb_l_element[cur_index].entry.number, (char*)ptr, 10 );
  ptr += 10;
  phb_l_element[cur_index].entry.cc_id     = *ptr++;

  phb_l_element[cur_index].entry.year = 0xff;       /* This is not on SIM */
  phb_l_element[cur_index].entry.month = 0xff;
  phb_l_element[cur_index].entry.day = 0xff;
  phb_l_element[cur_index].entry.hour = 0xff;
  phb_l_element[cur_index].entry.minute = 0xff;
  phb_l_element[cur_index].entry.second = 0xff;

/*#ifdef PHONEBOOK_EXTENSION */
#if 0
  if (*ptr != 0xFF) /* check for extention records */
  {
    file_id = pb_get_ext_file_id(phb_l_element[cur_index].type);
    if (file_id != 0xFFFF)
    {
      phb_l_element[cur_index].entry.ext_rcd_num = (UBYTE)*ptr;
      ext_index = cur_index;
      pb_read_sim_ext(file_id, phb_l_element[cur_index].entry.ext_rcd_num);
    }
  }
  else
  {
    phb_l_element[cur_index].entry.ext_rcd_num = 0xFF;
  }
#endif
}


/*
+---------------------------------------------------------------------+
| PROJECT: MMI-Framework (8417)         MODULE: PHB                   |
| STATE  : code                         ROUTINE: pb_read_sim_record   |
+---------------------------------------------------------------------+


    PURPOSE :   Build emergency call phonebook.

*/

T_PHB_RETURN pb_read_sim_record(UBYTE type, USHORT type_id, UBYTE rcd_num)
{
  SHORT index;
  UBYTE  n,m;

  /*  TRACE_FUNCTION ("pb_read_sim_record()");*/

  if (phb_ctb[type].used_rcd >= phb_ctb[type].max_rcd)
  {
    TRACE_FUNCTION("Used rcd full!");
    return PHB_FULL;
  }

  if (type NEQ LDN)
  {
    /* search a free element in phonebook element table */
    if (pb_create_memory(&index) NEQ PHB_OK)
    {
      TRACE_FUNCTION("Memory full");
      pb_read_eeprom_req();
      return PHB_FULL;
    }

    phb_ctb[type].used_rcd++;
    n = (UBYTE)(rcd_num-1)/8;
    m = (rcd_num-1)%8;
    phb_ctb[type].rcd_bitmap[n] |= 0x01 << m;
    phb_element[index].type        = type;
    phb_element[index].entry.index = rcd_num;

    pb_copy_sim_entry(index);

    pb_record_sort(index);
    pb_alpha_sort(index);
    pb_num_sort(index);

    if ((type EQ ADN) OR (type EQ FDN))
    {
      pb_malpha_sort(index);
      pb_mnum_sort(index);
    }
  }
  else /* special handling for LDN entries from SIM */
  {
    if (pb_create_l_memory(&index) NEQ PHB_OK)
    {
      TRACE_FUNCTION("Memory full");
      pb_read_eeprom_req();
      return PHB_OK;
    }
    phb_ctb[type].used_rcd++;
    phb_l_element[index].type        = type;
    phb_l_element[index].entry.index = rcd_num;

    pb_copy_sim_ldn_entry(index);

    pb_l_record_sort(index);
    /*  pb_l_alpha_sort(index);*//* not possible with RDM Structure */
    /*  pb_l_num_sort(index);*/
  }

  return PHB_OK;
}


/*
+---------------------------------------------------------------------+
| PROJECT:                              MODULE: PHB                   |
| STATE  : code                         ROUTINE: pb_get_ext_file_id   |
+---------------------------------------------------------------------+


    PURPOSE :   Find and gives the extention SIM file ID for the 
                phonebook entry.

*/
#ifdef PHONEBOOK_EXTENSION
LOCAL USHORT pb_get_ext_file_id (UBYTE pb_type)
{
  switch (pb_type)
  {
    case 0xFF:
    case ADN:
    case LDN:
//TISH, EXT1 is also used to save UPN extension number. See 11.11 10.5.10
    case UPN:
      return (SIM_EXT1);

    case FDN:
      return (SIM_EXT2);

    case SDN:
      return (SIM_EXT3);

    case BDN:
      return (SIM_EXT4);

    default:
      TRACE_ERROR ("pb_get_free_ext_record(): invalid type");
      return (0xFFFF);
  }
}


/*
+-----------------------------------------------------------------------+
| PROJECT:                              MODULE: PHB                     |
| STATE  : code                         ROUTINE: pb_rem_ext_record_flag |
+-----------------------------------------------------------------------+


    PURPOSE :   Removes the flag for the extention record.

*/

LOCAL void pb_rem_ext_record_flag (UBYTE pb_type, UBYTE rcd_num)
{
  UBYTE *rcd_bitmap;
  UBYTE pos, bit, len;

  switch (pb_type)
  {
    case 0xFF:
    case ADN:
//TISH, EXT1 is also used to save UPN extension number. See 11.11 10.5.10
	case UPN:
      rcd_bitmap = phb_ext_records[EXT1].rcd_bitmap;
      len = MAX_EXT1_BITMAP;
      break;

    case FDN:
      rcd_bitmap = phb_ext_records[EXT2].rcd_bitmap;
      len = MAX_EXT2_BITMAP;
      break;

    case SDN:
      rcd_bitmap = phb_ext_records[EXT3].rcd_bitmap;
      len = MAX_EXT3_BITMAP;
      break;
    case BDN:
      rcd_bitmap = phb_ext_records[EXT4].rcd_bitmap;
      len = MAX_EXT4_BITMAP;
      break;
    default:
      TRACE_ERROR ("pb_rem_free_ext_record(): invalid type");
      return;
  }

  pos = (UBYTE)(rcd_num - 1) / 8;
  bit = (rcd_num - 1) % 8;

  rcd_bitmap[pos] &= (UBYTE)(~(1u << bit));

}

//TISH set extension rcd_bitmap flag
LOCAL void pb_set_ext_record_flag (UBYTE pb_type, UBYTE rcd_num)
{
  UBYTE *rcd_bitmap;
  UBYTE pos, bit, len;

  switch (pb_type)
  {
    case 0xFF:
    case ADN:
    case UPN:
      rcd_bitmap = phb_ext_records[EXT1].rcd_bitmap;
      len = MAX_EXT1_BITMAP;
      break;

    case FDN:
      rcd_bitmap = phb_ext_records[EXT2].rcd_bitmap;
      len = MAX_EXT2_BITMAP;
      break;

    case BDN:
    case SDN:
    default:
      TRACE_ERROR ("pb_rem_free_ext_record(): invalid type");
      return;
  }

  pos = (UBYTE)(rcd_num - 1) / 8;
  bit = (rcd_num - 1) % 8;

  rcd_bitmap[pos] |= (UBYTE)((1u << bit));

}
/*
+-------------------------------------------------------------------------+
| PROJECT:                              MODULE: PHB                       |
| STATE  : code                         ROUTINE: pb_rem_ext_record_number |
+-------------------------------------------------------------------------+


    PURPOSE :   Gives the extention record number for the phonebook entry.

*/

LOCAL UBYTE pb_get_ext_record_number (UBYTE pb_type)
{
  UBYTE *rcd_bitmap;
  UBYTE len, pos, bit, rcd_num, max_rcd;

  switch (pb_type)
  {
    case 0xFF:
    case ADN:
//TISH, EXT1 is also used to save UPN extension number. See 11.11 10.5.10
	case UPN:
      rcd_bitmap = phb_ext_records[EXT1].rcd_bitmap;
      len = MAX_EXT1_BITMAP;
      max_rcd = phb_ext_records[EXT1].max_rcd;
      break;

    case FDN:
      rcd_bitmap = phb_ext_records[EXT2].rcd_bitmap;
      len = MAX_EXT2_BITMAP;
      max_rcd = phb_ext_records[EXT2].max_rcd;
      break;

    case SDN:
      rcd_bitmap = phb_ext_records[EXT3].rcd_bitmap;
      len = MAX_EXT3_BITMAP;
      max_rcd = phb_ext_records[EXT3].max_rcd;
      break;
	  
    case BDN:
      rcd_bitmap = phb_ext_records[EXT4].rcd_bitmap;
      len = MAX_EXT4_BITMAP;
      max_rcd = phb_ext_records[EXT4].max_rcd;
      break;
   
    default:
      TRACE_ERROR ("pb_get_free_ext_number(): invalid type");
      return (0xFF);
  }

	for (pos = 0; pos < len; pos++)
	{
		if ((UBYTE)~(rcd_bitmap[pos]))
		{
			int flag;
			for(bit=0;bit<8;bit++)
			{
				flag = rcd_bitmap[pos] & (0x01 << bit);
				rcd_num = (pos * 8) + bit + 1;
				if(flag)continue;
				
      /* Check for maximum extension records supported */
		      if (rcd_num > max_rcd )
		      {
		        return (0xFF);
		      }

		      return (rcd_num);
			}
	    }
  }
  TRACE_ERROR ("pb_get_free_ext_record(): no more extention records free");

  return (0xFF);
}


/*
+-----------------------------------------------------------------------+
| PROJECT:                              MODULE: PHB                     |
| STATE  : code                         ROUTINE: pb_read_ext_records    |
+-----------------------------------------------------------------------+


    PURPOSE :   Store the extention record flag and read the next record.

*/

LOCAL void pb_read_ext_records (T_PHB_EXT_TYPE type, 
                                USHORT sim_id, 
                                SHORT table_id)
{
  UBYTE rcd_num;
//  UBYTE n, m;

  rcd_num = simShrdPrm.atb[table_id].recNr;

  if (rcd_num EQ 1)
  {
    phb_ext_records[type].max_rcd = simShrdPrm.atb[table_id].recMax;
  }

  /* If this record is not empty EQ> set used flag */
//TISH: the ext record flag will be set at pb_read_ext_cb.
/*
  if (data[0] NEQ 0xFF)
  {
    n = (UBYTE)(rcd_num - 1) / 8;
    m = (rcd_num - 1) % 8;
    phb_ext_records[type].rcd_bitmap[n] |= (0x01 << m);
  }
*/
  simShrdPrm.atb[table_id].ntryUsdFlg = FALSE;

  if (rcd_num < phb_ext_records[type].max_rcd)
  {
    pb_read_sim(sim_id, ++rcd_num, simShrdPrm.atb[table_id].dataLen);
  }
  else
  {
    pb_read_sim_req();
  }
}

/*
+-----------------------------------------------------------------------+
| PROJECT:                              MODULE: PHB                     |
| STATE  : code                         ROUTINE: pb_get_ext_type    |
+-----------------------------------------------------------------------+


    PURPOSE :    gives the extention type  for the 
                phonebook entry.

*/
LOCAL UBYTE  pb_get_ext_type(UBYTE type)
 {
   switch (type)
   {
    case 0xFF:
    case ADN:
    case LDN:
      return ((UBYTE)EXT1);

    case FDN:
      return ((UBYTE)EXT2);

    case SDN:
      return ((UBYTE)EXT3);
	  
    case BDN:
      return ((UBYTE)EXT4);
    default:
      TRACE_ERROR ("pb_get_free_ext_record(): invalid type");
      return (0xFF);
   }
 }

/*
+-----------------------------------------------------------------------+
| PROJECT:                              MODULE: PHB                     |
| STATE  : code                         ROUTINE: pb_read_ext_records    |
+-----------------------------------------------------------------------+


    PURPOSE :  Gives the value of maximum extension records and used extension records

*/
 LOCAL  void  pb_read_ext_status(UBYTE ext_type, SHORT * max_ext, SHORT * used_ext)
 {
    UBYTE rec_num; 
    UBYTE *rcd_bitmap;
    UBYTE bit, pos; 

    TRACE_FUNCTION ("pb_read_ext_status()");
	
    *max_ext = phb_ext_records[ext_type].max_rcd; 
     rcd_bitmap = phb_ext_records[ext_type].rcd_bitmap; 
	 
    for(rec_num=1; rec_num<= *max_ext; rec_num++)
    {
     /* if bit is set to 1 then it is used record else if bit is set to 0 then it is free record  */
     pos = (UBYTE)(rec_num-1)/8; 
     bit = (rec_num - 1) % 8;
      if(rcd_bitmap[pos] & (0x01 << bit))
         *used_ext +=1; 
    }

 }


#endif

/*
+---------------------------------------------------------------------+
| PROJECT: MMI-Framework (8417)         MODULE: PHB                   |
| STATE  : code                         ROUTINE: pb_read_cb           |
+---------------------------------------------------------------------+


    PURPOSE :   read sim callback function

*/
void pb_read_cb(SHORT table_id)
{
  UBYTE          type = NOT_PRESENT_8BIT;
  USHORT         type_id;
  UBYTE          rcd_num;               /* record number */

  TRACE_FUNCTION ("pb_read_cb()");

  /* Inform the data of record */
  switch (simShrdPrm.atb[table_id].reqDataFld)
  {
#ifdef PHONEBOOK_EXTENSION
  case SIM_EXT1:
    pb_read_ext_records (EXT1, SIM_EXT1, table_id);
    return;

  case SIM_EXT2:
    pb_read_ext_records (EXT2, SIM_EXT2, table_id);
    return;
	
  case SIM_EXT3:
    pb_read_ext_records (EXT3, SIM_EXT3, table_id);
    return;

 case SIM_EXT4:
    pb_read_ext_records (EXT4, SIM_EXT4, table_id);
    return;
	
#endif    
  case SIM_ADN:
    type    = ADN;
    type_id = SIM_ADN;
    break;

  case SIM_FDN:
    type    = FDN;
    type_id = SIM_FDN;
    break;

  case SIM_LND:         /* Support for SIM_LDN */
    type    = LDN;      /* Caution: different identifiers LDN and LND */
    type_id = SIM_LND;
    break;

  case SIM_SDN:
    type    = SDN;
    type_id = SIM_SDN;
    break;

  case SIM_BDN:
    type    = BDN;
    type_id = SIM_BDN;
    break;

  case SIM_MSISDN:
    type    = UPN;
    type_id = SIM_MSISDN;
    break;
  
  default:
    TRACE_ERROR ("Invalid reqDataFld!");
    return;

  }
  TRACE_EVENT_P1("Callback of SIM reading Phonebook: %d", type);

  rcd_num = simShrdPrm.atb[table_id].recNr;

    
  if (rcd_num EQ 1)
  {
    if (type EQ LDN)
    {
      /* remember physical count for writing of correct number of LND on CFUN=0 */
      max_sim_LDN_records = simShrdPrm.atb[table_id].recMax;
    }
    phb_ctb[type].alpha_len = simShrdPrm.atb[table_id].dataLen - 14;
    phb_ctb[type].max_rcd   = simShrdPrm.atb[table_id].recMax;
  }

  simShrdPrm.atb[table_id].ntryUsdFlg = FALSE;

  /*
   * workaround for invalid phonebook entries (the entire entry is filled with 0x00)
   * this entries have been received which a very slow SIM
   * check if the Length of BCD number/SSC contents is 0x00 then discard
   */

  /* If this record is not empty, this record is written in phonebook. */
  if((phb_ctb[type].max_rcd NEQ 0)
    AND ((*data NEQ 0xff) OR (*(data + phb_ctb[type].alpha_len + 2) NEQ 0xff))
    AND (*(data + phb_ctb[type].alpha_len) NEQ 0x00))
  {
    if (pb_read_sim_record(type, type_id, rcd_num) EQ PHB_FULL)
    {
#ifdef SIM_TOOLKIT
      if (simShrdPrm.fuRef >= 0)
      {
        psaSAT_FUConfirm (simShrdPrm.fuRef, SIM_FU_SUCC_ADD);
      }
#endif
      phb_stat = PHB_READY;
      cmhPHB_StatIndication ( PHB_READY, CME_ERR_NotPresent, TRUE );
      return;
    }
  }

  if (rcd_num < phb_ctb[type].max_rcd)
  {
	  if(simShrdPrm.atb[table_id].reqDataFld != type_id) //TISH: if last field id is not current field id
		pb_read_sim(type_id, ++rcd_num, NOT_PRESENT_8BIT);
	  else
		pb_read_sim(type_id, ++rcd_num, simShrdPrm.atb[table_id].dataLen);
  }
  else
    pb_read_sim_req();
}




/*
+---------------------------------------------------------------------+
| PROJECT: MMI-Framework (8417)         MODULE: PHB                   |
| STATE  : code                         ROUTINE: pb_nibblecopy        |
+---------------------------------------------------------------------+


    PURPOSE :   appends a packed BCD number to an existing entry.
*/

#ifdef PHONEBOOK_EXTENSION
int pb_nibblecopy (UBYTE dest[], int destlen, UBYTE src[], int count)
{

  int i;
  int nibble;

  int destnibble=destlen*2;
  if (destnibble)
  {
    if ((dest[destlen-1] & 0xF0) EQ 0xF0)    /* check if there is space in last nibble */
      destnibble--;
  }

  for ( i=0; i<count*2; i++ )
  {
    /* check if we access out of bounds */
    if (destnibble/2 >= PHB_PACKED_NUM_LEN)
      return PHB_PACKED_NUM_LEN;

    /* get nibble */
    if (i%2 EQ 0)
      nibble = src[i/2] & 0x0F;
    else
      nibble = (src[i/2] & 0xF0) >> 4;

    if (nibble EQ 0xF)      /* end of number detected */
      break;

    /* put nibble */
    if (destnibble%2 EQ 0)
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
#endif


/*
+---------------------------------------------------------------------+
| PROJECT: MMI-Framework (8417)         MODULE: PHB                   |
| STATE  : code                         ROUTINE: pb_read_ext_cb       |
+---------------------------------------------------------------------+


    PURPOSE :   read sim callback function

*/

#ifdef PHONEBOOK_EXTENSION
void pb_read_ext_cb(SHORT table_id)
{
  USHORT         type_id;
  /*  UBYTE          buf[11];*/
  UBYTE          data_len;
  UBYTE          data_type;

  TRACE_FUNCTION ("pb_read_ext_cb()");

  simShrdPrm.atb[table_id].ntryUsdFlg = FALSE;

  if (simShrdPrm.atb[table_id].errCode NEQ SIM_NO_ERROR)
  {
    TRACE_EVENT_P1 ("SIM returned error 0x%04X", simShrdPrm.atb[table_id].errCode);
  }
  else
  {
    type_id = simShrdPrm.atb[table_id].reqDataFld;

    /* If this extension record is not empty, it is written in phonebook. */
    data_type = data[0];
    data_len = data[1];
//TISH set extension rcd_bitmap flag
    pb_set_ext_record_flag(phb_element[ext_index].type, simShrdPrm.atb[table_id].recNr);

    switch (data_type)
    {
      case 1: /* Called Party Subaddress */
        {
          int sa_len = 0;
          while (sa_len<PHB_PACKED_NUM_LEN)  /* get length of possible already stored subaddr if more than one EXT is used */
          {
            if (phb_element[ext_index].entry.subaddr[sa_len] EQ 0xFF)
              break;
            else if ((phb_element[ext_index].entry.subaddr[sa_len] & 0xF0) EQ 0xF0)
            {
              sa_len++;
              break;
            }
            else
              sa_len++;
          }

          pb_nibblecopy (phb_element[ext_index].entry.subaddr,
                         sa_len,
                         data + 2,
                         data_len);
        }
        break;

      case 2: /* Additional data */
        phb_element[ext_index].entry.len =
          pb_nibblecopy (phb_element[ext_index].entry.number,
                         phb_element[ext_index].entry.len,
                         data + 2,
                         data_len);
        break;

      default: /* unknown type */
        break;
    }

    if (data[12] != 0xFF) /* check if a further EXT entry exists */
    {
      if (max_ext_chain_reads)  /* limit reached? */
      {
        max_ext_chain_reads--;
        pb_read_sim_ext(type_id, data[12]);
        return;
      }
    }
  }

  /* Continue reading the last by EXT interrupted phonebook */
  pause_pb_reading_while_EXT_reading = FALSE;

  if (paused_table_id)
  {
    simShrdPrm.aId = paused_table_id;
    paused_table_id = 0;
    if(psaSIM_AccessSIMData() < 0)
    {
      TRACE_EVENT("FATAL ERROR");
      return;
    }
  }
}
#endif

/*
+--------------------------------------------------------------------+
| PROJECT: MMI-Framework (8417)         MODULE: PHB                  |
| STATE  : code                         ROUTINE: pb_read_eeprom_req  |
+--------------------------------------------------------------------+


    PURPOSE :   Request to build phonebook.

*/
T_PHB_RETURN pb_read_eeprom_req(void)
{
  UBYTE          version;
  UBYTE          i, n, m;
  SHORT         index;
  USHORT         max_rcd;
  EF_UPN        *efupn;

  TRACE_FUNCTION ("pb_read_eeprom_req()");

  phb_ctb[ADN_FDN].mem = SIM_MEMORY;
  phb_ctb[ADN_FDN].type = ADN_FDN;
  if (phb_ctb[ADN].alpha_len)
    phb_ctb[ADN_FDN].alpha_len = phb_ctb[ADN].alpha_len;
  else
    phb_ctb[ADN_FDN].alpha_len = phb_ctb[FDN].alpha_len;
  
  phb_ctb[ADN_FDN].max_rcd = phb_ctb[ADN].max_rcd + phb_ctb[FDN].max_rcd;
  phb_ctb[ADN_FDN].used_rcd = phb_ctb[ADN].used_rcd + phb_ctb[FDN].used_rcd;

  if (read_flag)
  {
    phb_stat = PHB_READY;
    cmhPHB_StatIndication ( PHB_READY, CME_ERR_NotPresent, TRUE );
    return PHB_OK;
  }

  /* Read Last Dialing Numbers */

  if (phb_ctb[LDN].mem EQ NO_PHB_ENTRY)   /* No phonebook loaded from SIM? */
  {
    phb_ctb[LDN].mem     = TE_MEMORY;
    phb_ctb[LDN].type    = LDN;
    phb_ctb[LDN].max_rcd = MAX_RDM_RECORDS/3;
    phb_ctb[LDN].used_rcd = 0;
    phb_ctb[LDN].first_rcd = UNUSED_INDEX;
    phb_ctb[LDN].first_trcd = UNUSED_INDEX;
    phb_ctb[LDN].first_nrcd = UNUSED_INDEX;
    phb_ctb[LDN].first_mtrcd = UNUSED_INDEX;
    phb_ctb[LDN].first_mnrcd = UNUSED_INDEX;
  }
  else
  {
                                              /* either:                                                    */
  /*  phb_ctb[LDN].max_rcd = MAX_RDM_RECORDS/3;*//* adjust in case SIM_max_record was smaller than PCM_max_rcd */
                                              /* or:                                                        */
    ;                                         /* use max_rcd as read from SIM.                              */

  }
  if (imsiFlag EQ FALSE )                /* SIM has changed? */
  {
    memset (data, 0xFF, SIZE_EF_LDN);
    i=0;
    while (i < NR_EF_LDN)
    {
      if (pcm_WriteRecord((UBYTE *)EF_LDN_ID,   /* Wipe entire LDN-PCM */
                      (USHORT)(i+1),
                      SIZE_EF_LDN,
                      data) NEQ DRV_OK)
        break;
      i++;
    }
  }
  else                                    
  {                                       /* Merge timestamp with existing SIM-entries */
    BOOL all_records_match = TRUE;
    SHORT records_from_sim = phb_ctb[LDN].used_rcd;   /* =0 in case of no records from SIM read */
    EF_LDN *p = (EF_LDN *)data;

    index = phb_ctb[LDN].first_rcd;
    for (i=0; i<(UBYTE)phb_ctb[LDN].max_rcd; i++)
    {
      if ((i+1) > NR_EF_LDN)
        break;                                      /* end of phonebook */
      if (pcm_ReadRecord((UBYTE *)EF_LDN_ID,
                         (USHORT)(i+1),
                         SIZE_EF_LDN,
                         (UBYTE *)&data[0],
                         &version,
                         &max_rcd) NEQ DRV_OK)
        break;                                      /* read error */
      else
      {
        if (p->len NEQ 0 AND p->len NEQ 0xFF)
        {
          if ((i+1) <= records_from_sim)
          {
            if (index EQ UNUSED_INDEX)
            {
              all_records_match = FALSE;
              break;
            }
            if ( !memcmp((char *)phb_l_element[index].entry.number, (char *)&p->dldNum, 10)
                 AND phb_l_element[index].entry.ton_npi EQ p->numTp
                 AND phb_l_element[index].entry.len EQ p->len
                 AND phb_l_element[index].entry.cc_id EQ p->ccp)         /* Number matches? */
            {
              pb_copy_ldn_record(index, 0);         /* then update the record with Timestamps and cc_id from PCM */
            }
            else
            {
              all_records_match = FALSE;            /* one (or more) record failed */
              break;                                /* stop processing of further records */
            }
            index = phb_l_element[index].next_rcd;
          }
          else                                      /* PCM has more entries than on SIM */
          {
            /* search a free element in phonebook element table */
            if (pb_create_l_memory(&index) NEQ PHB_OK)
                return PHB_FULL;

            phb_ctb[LDN].used_rcd++;
            phb_l_element[index].type        = LDN;
            phb_l_element[index].entry.index = i+1;

            pb_copy_ldn_record((SHORT)index, 0);
            pb_l_record_sort(index);
          }
        }
      }
    }
    if (all_records_match NEQ TRUE)                 /* some elements did not match */
    {
      index = phb_ctb[LDN].first_rcd;
      for (i=0; i<phb_ctb[LDN].used_rcd; i++)
      {
        if (index EQ UNUSED_INDEX)
        {
          break;
        }
        phb_l_element[index].entry.year =
        phb_l_element[index].entry.month =
        phb_l_element[index].entry.day =
        phb_l_element[index].entry.hour =
        phb_l_element[index].entry.minute =
        phb_l_element[index].entry.second = 0xFF;    /* remove previous merged data from PCM */
        index = phb_l_element[index].next_rcd;
      }
    }
  }


  /* Read Last received Numbers from EEPROM */
  phb_ctb[LRN].mem     = TE_MEMORY;
  phb_ctb[LRN].type    = LRN;
  phb_ctb[LRN].max_rcd = MAX_RDM_RECORDS/3;
  phb_ctb[LRN].used_rcd = 0;
  phb_ctb[LRN].first_rcd = UNUSED_INDEX;
  phb_ctb[LRN].first_trcd = UNUSED_INDEX;
  phb_ctb[LRN].first_nrcd = UNUSED_INDEX;
  phb_ctb[LRN].first_mtrcd = UNUSED_INDEX;
  phb_ctb[LRN].first_mnrcd = UNUSED_INDEX;

  if (imsiFlag EQ FALSE )
  {
    memset (data, 0xFF, SIZE_EF_LRN);
    i=0;
    while (i < NR_EF_LRN)
    {
      if (pcm_WriteRecord((UBYTE *)EF_LRN_ID,   /* Wipe entire LRN-PCM */
                      (USHORT)(i+1),
                      SIZE_EF_LRN,
                      data) NEQ DRV_OK)
        break;
      i++;
    }
  }
  else
  {
    EF_LRN *p = (EF_LRN *)data;
    for (i=0; i<(UBYTE)phb_ctb[LRN].max_rcd; i++)
    {
      if ((i+1) > NR_EF_LRN)
        break;
      if (pcm_ReadRecord((UBYTE *)EF_LRN_ID,
                         (USHORT)(i+1),
                         SIZE_EF_LRN,
                         (UBYTE *)&data[0],
                         &version,
                         &max_rcd) NEQ DRV_OK)
        break;
      else
      {
        if (p->len NEQ 0 AND p->len NEQ 0xFF)
        {
          /* search a free element in phonebook element table */
          if (pb_create_l_memory(&index) NEQ PHB_OK)
            return PHB_FULL;

          phb_ctb[LRN].used_rcd++;
          phb_l_element[index].type        = LRN;
          phb_l_element[index].entry.index = i+1;

          pb_copy_lrn_record(index, 0);
          pb_l_record_sort(index);
        }
      }
    }
  }

  /* Read Last missed Numbers from EEPROM */
  phb_ctb[LMN].mem     = TE_MEMORY;
  phb_ctb[LMN].type    = LMN;
  phb_ctb[LMN].max_rcd = MAX_RDM_RECORDS/3;
  phb_ctb[LMN].used_rcd = 0;
  phb_ctb[LMN].first_rcd = UNUSED_INDEX;
  phb_ctb[LMN].first_trcd = UNUSED_INDEX;
  phb_ctb[LMN].first_nrcd = UNUSED_INDEX;
  phb_ctb[LMN].first_mtrcd = UNUSED_INDEX;
  phb_ctb[LMN].first_mnrcd = UNUSED_INDEX;

  if (imsiFlag EQ FALSE )
  { 
    memset (data, 0xFF, SIZE_EF_LMN);
    i=0;
    while (i < NR_EF_LMN)
    {
      if (pcm_WriteRecord((UBYTE *)EF_LMN_ID,   /* Wipe entire LMN-PCM */
                      (USHORT)(i+1),
                      SIZE_EF_LMN,
                      data) NEQ DRV_OK)
        break;
      i++;
    }
  }
  else
  {
    EF_LMN *p = (EF_LMN *)data;
    for (i=0; i<(UBYTE)phb_ctb[LMN].max_rcd; i++)
    {
      if ((i+1) > NR_EF_LMN)
        break;
      if (pcm_ReadRecord((UBYTE *)EF_LMN_ID,
                         (USHORT)(i+1),
                         SIZE_EF_LMN,
                         (UBYTE *)&data[0],
                         &version,
                         &max_rcd) NEQ DRV_OK)
        break;
      else
      {
        if (p->len NEQ 0 AND p->len NEQ 0xFF)
        {
          /* search a free element in phonebook element table */
          if (pb_create_l_memory(&index) NEQ PHB_OK)
            return PHB_FULL;

          phb_ctb[LMN].used_rcd++;
          phb_l_element[index].type        = LMN;
          phb_l_element[index].entry.index = i+1;

          pb_copy_lmn_record(index, 0);
          pb_l_record_sort(index);
        }
      }
    }
  }

  if (phb_ctb[UPN].mem EQ NO_PHB_ENTRY)
  {
    phb_ctb[UPN].mem     = TE_MEMORY;
    phb_ctb[UPN].type    = UPN;
    phb_ctb[UPN].max_rcd = NR_EF_UPN;
    phb_ctb[UPN].used_rcd = 0;
    phb_ctb[UPN].first_rcd = UNUSED_INDEX;
    phb_ctb[UPN].first_trcd = UNUSED_INDEX;
    phb_ctb[UPN].first_nrcd = UNUSED_INDEX;
    phb_ctb[UPN].first_mtrcd = UNUSED_INDEX;
    phb_ctb[UPN].first_mnrcd = UNUSED_INDEX;

    for (i=0; i<NR_EF_UPN; i++)
    {
      if (pcm_ReadRecord((UBYTE *)EF_UPN_ID,
                         (USHORT)(i+1),
                         SIZE_EF_UPN,
                         (UBYTE *)&data[0],
                         &version,
                         &max_rcd) NEQ DRV_OK)
      {
        phb_ctb[UPN].mem = NO_PHB_ENTRY;
        phb_ctb[UPN].max_rcd = 0;
      }
      else
      {
        efupn = (EF_UPN *)&data[0];
        if (efupn->usrNum[0] NEQ 0xff)
        {
          /* search a free element in phonebook element table */
          if (pb_create_memory(&index) NEQ PHB_OK)
            return PHB_FULL;

          phb_ctb[UPN].used_rcd++;
          n = (UBYTE)i/8;
          m = i%8;
          phb_ctb[UPN].rcd_bitmap[n] |= 0x01 << m;

          phb_element[index].type        = UPN;
          phb_element[index].entry.index = i+1;

          /* copy record */
          memset(phb_element[index].entry.tag, 0xFF, sizeof(phb_element[index].entry.tag));
          memset(phb_element[index].entry.number, 0xFF, sizeof(phb_element[index].entry.number));
          memcpy(phb_element[index].entry.tag,
                 efupn->alphId,
                 10*sizeof(UBYTE));
          phb_element[index].entry.tag_len = pb_get_entry_len(efupn->alphId, 10);
          phb_element[index].entry.len = efupn->len;
          phb_element[index].entry.ton_npi = efupn->numTp;
          memcpy(phb_element[index].entry.number,
                 efupn->usrNum,
                 10*sizeof(UBYTE));
          phb_element[index].entry.cc_id     = efupn->ccp;

          pb_record_sort(index);
          pb_alpha_sort(index);
          pb_num_sort(index);
        }
      }
    }
  }
  phb_stat = PHB_READY;
  cmhPHB_StatIndication ( PHB_READY, CME_ERR_NotPresent, TRUE );
  read_flag = 1;
  return PHB_OK;
}


/*
+--------------------------------------------------------------------+
| PROJECT: MMI-Framework (8417)         MODULE: PHB                  |
| STATE  : code                         ROUTINE: pb_read_sim_req     |
+--------------------------------------------------------------------+


PURPOSE :   Request to build phonebook.

*/

void pb_read_sim_req(void)
{
  UBYTE serv_stat;

  TRACE_FUNCTION("pb_read_sim_req()");

  /* Read Fixed Dialing Numbers from SIM card */
  if ((serv_stat = pb_ssc(SRV_FDN,sim_service_table)) EQ ALLOCATED_AND_ACTIVATED
    AND phb_ctb[FDN].mem EQ NO_PHB_ENTRY)
  {
    phb_ctb[FDN].mem     = SIM_MEMORY;
    phb_ctb[FDN].type    = FDN;
    phb_ctb[FDN].service = serv_stat;
    pb_read_sim(SIM_FDN, 1, NOT_PRESENT_8BIT);
    return;
  }

  if (read_flag)
  {
    pb_read_eeprom_req();
    return;
  }

  /* Read MSISDN from SIM card */
  if ((serv_stat = pb_ssc(SRV_MSISDN,sim_service_table)) EQ ALLOCATED_AND_ACTIVATED
      AND phb_ctb[UPN].mem EQ NO_PHB_ENTRY)
  {
    phb_ctb[UPN].mem     = SIM_MEMORY;
    phb_ctb[UPN].type    = UPN;
    phb_ctb[UPN].service = serv_stat;
    pb_read_sim(SIM_MSISDN, 1, NOT_PRESENT_8BIT);
    return;
  }

  /* Read Barred Dialing Numbers from SIM card */
  if ((serv_stat = pb_ssc(SRV_BDN,sim_service_table)) EQ ALLOCATED_AND_ACTIVATED
    AND phb_ctb[BDN].mem EQ NO_PHB_ENTRY)
  {
    phb_ctb[BDN].mem     = SIM_MEMORY;
    phb_ctb[BDN].type    = BDN;
    phb_ctb[BDN].service = serv_stat;
    pb_read_sim(SIM_BDN, 1, NOT_PRESENT_8BIT);
    return;
  }

  /* Read Service Dialing Numbers from SIM card */
  if ((serv_stat = pb_ssc(SRV_SDN,sim_service_table)) EQ ALLOCATED_AND_ACTIVATED
    AND phb_ctb[SDN].mem EQ NO_PHB_ENTRY)
  {
    phb_ctb[SDN].mem     = SIM_MEMORY;
    phb_ctb[SDN].type    = SDN;
    phb_ctb[SDN].service = serv_stat;
    pb_read_sim(SIM_SDN, 1, NOT_PRESENT_8BIT);
    return;
  }


  /* Read Last Numbers Dialed from SIM card */
  if ((serv_stat = pb_ssc(SRV_LDN,sim_service_table)) EQ ALLOCATED_AND_ACTIVATED
    AND phb_ctb[LDN].mem EQ NO_PHB_ENTRY)
  {
    phb_ctb[LDN].mem      = TE_MEMORY;
    phb_ctb[LDN].type     = LDN;
    phb_ctb[LDN].service  = serv_stat;
    phb_ctb[LDN].max_rcd = MAX_RDM_RECORDS/3;
    phb_ctb[LDN].used_rcd = 0;
    phb_ctb[LDN].first_rcd = UNUSED_INDEX;
    phb_ctb[LDN].first_trcd = UNUSED_INDEX;
    phb_ctb[LDN].first_nrcd = UNUSED_INDEX;
    phb_ctb[LDN].first_mtrcd = UNUSED_INDEX;
    phb_ctb[LDN].first_mnrcd = UNUSED_INDEX;
    pb_read_sim(SIM_LND, 1, NOT_PRESENT_8BIT);
    return;
  }


#ifdef PHONEBOOK_EXTENSION
  /* Read Ext1 Records from SIM card */
  if ((serv_stat = pb_ssc(SRV_EXT1,sim_service_table)) EQ ALLOCATED_AND_ACTIVATED
    AND phb_ext_records[EXT1].mem EQ NO_PHB_ENTRY)
  {
    TRACE_EVENT ("Start reading EXT1");
    phb_ext_records[EXT1].mem  = SIM_MEMORY;
    pb_read_sim(SIM_EXT1, 1, NOT_PRESENT_8BIT);
    return;
  }

  /* Read Ext2 Records from SIM card */
  if ((serv_stat = pb_ssc(SRV_EXT2,sim_service_table)) EQ ALLOCATED_AND_ACTIVATED
    AND phb_ext_records[EXT2].mem EQ NO_PHB_ENTRY)
  {
    TRACE_EVENT ("Start reading EXT2");
    phb_ext_records[EXT2].mem  = SIM_MEMORY;
    pb_read_sim(SIM_EXT2, 1, NOT_PRESENT_8BIT);
    return;
  }

    /* Read Ext3 Records from SIM card */
  if ((serv_stat = pb_ssc(SRV_EXT3,sim_service_table)) EQ ALLOCATED_AND_ACTIVATED
    AND phb_ext_records[EXT3].mem EQ NO_PHB_ENTRY)
  {
    TRACE_EVENT ("Start reading EXT3");
    phb_ext_records[EXT3].mem  = SIM_MEMORY;
    pb_read_sim(SIM_EXT3, 1, NOT_PRESENT_8BIT);
    return;
  }

      /* Read Ext4 Records from SIM card */
  if ((serv_stat = pb_ssc(SRV_EXT4,sim_service_table)) EQ ALLOCATED_AND_ACTIVATED
    AND phb_ext_records[EXT4].mem EQ NO_PHB_ENTRY)
  {
    TRACE_EVENT ("Start reading EXT4");
    phb_ext_records[EXT4].mem  = SIM_MEMORY;
    pb_read_sim(SIM_EXT4, 1, NOT_PRESENT_8BIT);
    return;
  }
  
#endif

  /* Read phonebook from EEPROM */
  pb_read_eeprom_req();

#ifdef SIM_TOOLKIT
  if (simShrdPrm.fuRef >= 0)
  {
    psaSAT_FUConfirm (simShrdPrm.fuRef, SIM_FU_SUCC_ADD);
  }
#endif

}

/*
+--------------------------------------------------------------------+
| PROJECT: MMI-Framework (8417)         MODULE: PHB                  |
| STATE  : code                         ROUTINE: pb_build_req        |
+--------------------------------------------------------------------+

    PURPOSE : Request to build phonebook.

*/

void pb_build_req(T_SIM_MMI_INSERT_IND *sim_mmi_insert_ind)
{
  UBYTE simIMSI[MAX_IMSI_LEN+1];
  UBYTE pcmIMSI[MAX_IMSI_LEN+1];
  EF_IMSI imsi;
  UBYTE version;

  #ifndef _SIMULATION_  
  UBYTE classFDN = (UBYTE) CLASS_None;
  T_FFS_RET ret_ffs; /* FFS handle */
  #endif

  TRACE_FUNCTION ("pb_build_req()");

  if (fdn_mode EQ NO_OPERATION)
  {
    /* Read SIM service table from SIM card */
    memcpy(sim_service_table, sim_mmi_insert_ind -> sim_serv, MAX_SRV_TBL);

    /* Compare IMSI field between SIM and PCM */
    imsiFlag = FALSE;
    psaSIM_decodeIMSI (sim_mmi_insert_ind->imsi_field.field,
                       sim_mmi_insert_ind->imsi_field.c_field,
                       (char *)simIMSI);

    if (pcm_ReadFile((UBYTE *) EF_IMSI_ID,SIZE_EF_IMSI,
                     (UBYTE *) &imsi, &version) EQ PCM_OK)
    {
      psaSIM_decodeIMSI (imsi.IMSI, imsi.len, (char *)pcmIMSI);
      if (!strcmp((char *)simIMSI, (char *)pcmIMSI))
        imsiFlag = TRUE;
      else
      {
        /* write the IMSI in PCM */
        imsi.len = sim_mmi_insert_ind->imsi_field.c_field;
        memcpy(imsi.IMSI, sim_mmi_insert_ind->imsi_field.field, MAX_IMSI-1);
        pcm_WriteFile((UBYTE *) EF_IMSI_ID,SIZE_EF_IMSI,
                      (UBYTE *) &imsi);
      }
    }

    switch (sim_mmi_insert_ind -> func)
    {
      case SIM_ADN_ENABLED:
      case SIM_ADN_BDN_ENABLED:
        fdn_mode = FDN_DISABLE;
        break;
      case SIM_FDN_ENABLED:
      case SIM_FDN_BDN_ENABLED:
        fdn_mode = FDN_ENABLE;
        #ifndef _SIMULATION_
        /* read last fdn_classtype from FFS */
        ret_ffs = ffs_fread("/mmi/fdnClassType",
                            &classFDN,
                            sizeof(classFDN));

        if(!(ret_ffs < 1)) /* successful read */
        {
          /* only these two classes are currently supported */
          if ( classFDN EQ (UBYTE) CLASS_VceDatFax OR 
               classFDN EQ (UBYTE) CLASS_VceDatFaxSms )
          {
            fdn_classtype = classFDN; 
            fdn_input_classtype = fdn_classtype; 
          }
        }
        #endif 
        break;
      default:
        fdn_mode = NO_OPERATION;
        break;
    }
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT: MMI-Framework (8417)         MODULE: PHB                  |
| STATE  : code                         ROUTINE: pb_start_build      |
+--------------------------------------------------------------------+

    PURPOSE : Start reading the phonebook.

*/

T_PHB_RETURN pb_start_build (BOOL unchanged)
{
  UBYTE  serv_stat;

  TRACE_FUNCTION ("pb_start_build()");

  read_flag = 0;
  phb_stat = PHB_BUSY;
  cmhPHB_StatIndication ( PHB_BUSY, CME_ERR_NotPresent, TRUE );

  /* Read Abbreviated Dialing Numbers */
  if ((serv_stat = pb_ssc(SRV_ADN,sim_service_table)) EQ ALLOCATED_AND_ACTIVATED)
  {
    if (fdn_mode EQ FDN_ENABLE)
    {
      pb_read_sim_req();
      return PHB_OK;
    }

    if ( phb_ctb[ADN].mem EQ NO_PHB_ENTRY )
    {
      phb_ctb[ADN].mem     = SIM_MEMORY;
      phb_ctb[ADN].type    = ADN;
      phb_ctb[ADN].service = serv_stat;

      pb_read_sim(SIM_ADN, 1, NOT_PRESENT_8BIT);
    }
    else
      pb_read_sim_req();
  }

  else
  {
    pb_read_sim_req();
  }
  return PHB_OK;
}

/*
+--------------------------------------------------------------------+
| PROJECT : MMI-Framework (8417)         MODULE: PHB                 |
| STATE   : code                         ROUTINE : pb_update         |
+--------------------------------------------------------------------+

  PURPOSE : Phonebook update on File Change Notification.

*/

#ifdef SIM_TOOLKIT
BOOL pb_update (int ref, T_SIM_FILE_UPDATE_IND *fu)
{
  BOOL found = FALSE;
  int i;

  TRACE_FUNCTION ("pb_update ()");

  for (i = 0; i < (int)fu->val_nr; i++)
  {
    if ( (fu->file_info[i].v_path_info EQ TRUE AND
          fu->file_info[i].path_info.df_level1 EQ SIM_DF_TELECOM AND
          fu->file_info[i].path_info.v_df_level2 EQ FALSE AND
          (fu->file_info[i].datafield EQ SIM_ADN OR    /* the extension datafields */
           fu->file_info[i].datafield EQ SIM_FDN OR    /* have to be added, when */
           fu->file_info[i].datafield EQ SIM_BDN OR    /* they are really used */
           fu->file_info[i].datafield EQ SIM_SDN OR
           fu->file_info[i].datafield EQ SIM_MSISDN OR
           fu->file_info[i].datafield EQ SIM_LND)) OR  /* CQ16301: Added support for LND refresh */

         (fu->file_info[i].v_path_info EQ TRUE AND
          fu->file_info[i].path_info.df_level1 EQ SIM_DF_GSM AND
          fu->file_info[i].path_info.v_df_level2 EQ FALSE AND  
          fu->file_info[i].datafield EQ SIM_SST) )
    {
      found = TRUE;

      /* when SIM service table is changed, the all SIM-phonebooks
         will be updated.                                          */
      if (fu->file_info[i].datafield EQ SIM_SST)
      {
        sstUpdateId = TRUE;
        break;
      }

      pb_sat_update_reset(fu->file_info[i].datafield);
    }
  }

  if (found)
  {
    simShrdPrm.fuRef = ref;
    if (sstUpdateId)
    {
      sstUpdateId = FALSE;

      /* Update SIM service table */
      if (pb_read_sim_dat(SIM_SST, NOT_PRESENT_8BIT, (UBYTE)256) EQ FALSE )
        pb_start_build (FALSE);
    }
    else
      pb_start_build (FALSE);
    return FALSE;
  }
  else
  {
    simShrdPrm.fuRef = -1;         /* no update needed */
    return TRUE;
  }
}
#endif

/*
+--------------------------------------------------------------------+
| PROJECT : MMI-Framework (8417)         MODULE: PHB                 |
| STATE   : code                         ROUTINE : pb_ssc            |
+--------------------------------------------------------------------+

  PURPOSE : Check SIM service status.

*/

UBYTE pb_ssc (UBYTE nr, UBYTE * serv_table)
{
  TRACE_FUNCTION ("pb_ssc()");

  if (nr > MAX_SRV_TBL*4)
  {
    TRACE_ERROR ("serv_table overflow in pb_ssc()");
    return NO_ALLOCATED;
  }

  /* SDN and BDN are not used */
  /*   if ((nr EQ 18) || (nr EQ31)) */
  /*  return(NO_ALLOCATED);*/

  return ( *(serv_table+(nr-1)/4) >> (((nr-1)&3)*2)  & 0x03);
}


/*
+---------------------------------------------------------------------+
| PROJECT : MMI-Framework (8417)         MODULE: PHB                  |
| STATE   : code                         ROUTINE : pb_record_sort     |
+---------------------------------------------------------------------+

  PURPOSE :

*/

void pb_record_sort(SHORT cur_index)
{
  SHORT          ref_index;
  SHORT          ptr_index;
  UBYTE          flag;

  /*  TRACE_FUNCTION ("pb_record_sort()"); */

  if (phb_ctb[phb_element[cur_index].type].used_rcd EQ 1)
  {
    phb_element[cur_index].prev_rcd  = UNUSED_INDEX;
    phb_element[cur_index].next_rcd  = UNUSED_INDEX;
    phb_ctb[phb_element[cur_index].type].first_rcd = cur_index;
  }
  else
  {
    flag = 0;

    ref_index = phb_ctb[phb_element[cur_index].type].first_rcd;
    phb_ctb[phb_element[cur_index].type].first_rcd = cur_index;

    phb_element[cur_index].prev_rcd = UNUSED_INDEX;
    phb_element[cur_index].next_rcd = ref_index;
    phb_element[ref_index].prev_rcd = cur_index;

    while (ref_index NEQ UNUSED_INDEX)
    {
      if (phb_element[cur_index].entry.index > phb_element[ref_index].entry.index)
      {
        ptr_index = phb_element[ref_index].next_rcd;
        if (ptr_index != UNUSED_INDEX)
          phb_element[ptr_index].prev_rcd = cur_index;
        phb_element[cur_index].next_rcd = ptr_index;

        ptr_index = phb_element[cur_index].prev_rcd;
        if (ptr_index != UNUSED_INDEX)
          phb_element[ptr_index].next_rcd = ref_index;

        phb_element[ref_index].prev_rcd = phb_element[cur_index].prev_rcd;
        phb_element[ref_index].next_rcd = cur_index;
        phb_element[cur_index].prev_rcd = ref_index;

        /* set the first record in control block */
        if(!flag)
        {
          phb_ctb[phb_element[cur_index].type].first_rcd = ref_index;
          flag = 1;
        }
        ref_index = phb_element[cur_index].next_rcd;
      }

      else
        ref_index = phb_element[ref_index].next_rcd;
    }
  }
}


/*
+---------------------------------------------------------------------+
| PROJECT : MMI-Framework (8417)         MODULE: PHB                  |
| STATE   : code                         ROUTINE : pb_l_record_sort   |
+---------------------------------------------------------------------+

  PURPOSE :

*/

void pb_l_record_sort(SHORT cur_index)
{
  SHORT          ref_index;
  SHORT          ptr_index;
  UBYTE          flag;

  TRACE_FUNCTION ("pb_l_record_sort()");

  if (phb_ctb[phb_l_element[cur_index].type].used_rcd EQ 1)
  {
    phb_l_element[cur_index].prev_rcd  = UNUSED_BYTE_INDEX;
    phb_l_element[cur_index].next_rcd  = UNUSED_BYTE_INDEX;
    phb_ctb[phb_l_element[cur_index].type].first_rcd = cur_index;
  }
  else
  {
    flag = 0;

    ref_index = phb_ctb[phb_l_element[cur_index].type].first_rcd;
    phb_ctb[phb_l_element[cur_index].type].first_rcd = cur_index;

    phb_l_element[cur_index].prev_rcd = UNUSED_BYTE_INDEX;
    phb_l_element[cur_index].next_rcd = (UBYTE)ref_index;
    phb_l_element[ref_index].prev_rcd = (UBYTE)cur_index;

    while ((UBYTE)ref_index NEQ UNUSED_BYTE_INDEX)
    {
      if (phb_l_element[cur_index].entry.index > phb_l_element[ref_index].entry.index)
      {
        ptr_index = (SHORT)phb_l_element[ref_index].next_rcd;
        if ((UBYTE)ptr_index != UNUSED_BYTE_INDEX)
          phb_l_element[ptr_index].prev_rcd = (UBYTE)cur_index;
        phb_l_element[cur_index].next_rcd = (UBYTE)ptr_index;

        ptr_index = (SHORT)phb_l_element[cur_index].prev_rcd;
        if ((UBYTE)ptr_index != UNUSED_BYTE_INDEX)
          phb_l_element[ptr_index].next_rcd = (UBYTE)ref_index;

        phb_l_element[ref_index].prev_rcd = phb_l_element[cur_index].prev_rcd;
        phb_l_element[ref_index].next_rcd = (UBYTE)cur_index;
        phb_l_element[cur_index].prev_rcd = (UBYTE)ref_index;

        /* set the first record in control block */
        if(!flag)
        {
          phb_ctb[phb_l_element[cur_index].type].first_rcd = ref_index;
          flag = 1;
        }
        ref_index = (SHORT)phb_l_element[cur_index].next_rcd;
      }

      else
        ref_index = (SHORT)phb_l_element[ref_index].next_rcd;
    }
  }
}


/*
+---------------------------------------------------------------------+
| PROJECT : MMI-Framework (8417)       MODULE  : PHB                  |
| STATE   : code                       ROUTINE : pb_cvt_alpha_for_cmp |
+---------------------------------------------------------------------+

  PURPOSE : convert alpha to lower case when not unicode

*/
static void pb_cvt_alpha_for_cmp ( UBYTE *src,
                            UBYTE *dst,
                            UBYTE len )
{
  int i;

  if ( *src NEQ 0x80 )
  {
    for ( i = 0; i < len; i++ )
      dst[i] = (UBYTE)tolower((int)src[i]);

    return;
  }

  for ( i = 0; i < len; i++ )
    dst[i] = src[i];
}

/*
+---------------------------------------------------------------------+
| PROJECT : MMI-Framework (8417)         MODULE: PHB                  |
| STATE   : code                         ROUTINE : pb_alpha_sort      |
+---------------------------------------------------------------------+

  PURPOSE : Insert a new record to alpha sorted chain.

*/

void pb_alpha_sort(SHORT cur_index)
{
  SHORT ref_index;
  SHORT ptr_index;
  UBYTE flag, cmpLen = 0;
  UBYTE cur_tag[PHB_MAX_TAG_LEN], check_tag[PHB_MAX_TAG_LEN];
  int   cmp_res;

  /* set the new record as first element */
  if (phb_ctb[phb_element[cur_index].type].used_rcd EQ 1)
  {
    phb_element[cur_index].prev_trcd = UNUSED_INDEX;
    phb_element[cur_index].next_trcd = UNUSED_INDEX;

    phb_ctb[phb_element[cur_index].type].first_trcd = cur_index;
  }

  if (phb_ctb[phb_element[cur_index].type].used_rcd > 1)
  {
    ref_index = phb_ctb[phb_element[cur_index].type].first_trcd;
    phb_ctb[phb_element[cur_index].type].first_trcd = cur_index;

    phb_element[cur_index].prev_trcd = UNUSED_INDEX;
    phb_element[cur_index].next_trcd = ref_index;
    phb_element[ref_index].prev_trcd = cur_index;

    /* insert the new record in the alpha order */
    flag = 0;
    while (ref_index NEQ UNUSED_INDEX)
    {
      memset(cur_tag, 0, sizeof ( cur_tag ) );
      memset(check_tag, 0, sizeof ( check_tag ) );

      /*
        this should not cause problems, because in both alphabets 
        (GSM and ASCII) the most important chars are at the same 
        positions (A-Z: 65-90 a-z:97-122)
      */

      if( ext_compare_fct != NULL )
      {
        cmp_res = ext_compare_fct ( phb_element[cur_index].entry.tag, 
                                    phb_element[cur_index].entry.tag_len,
                                    phb_element[ref_index].entry.tag,
                                    phb_element[ref_index].entry.tag_len );
      }
      else
      {
        pb_cvt_alpha_for_cmp ( phb_element[cur_index].entry.tag,
                               cur_tag,
                               phb_element[cur_index].entry.tag_len );
        pb_cvt_alpha_for_cmp ( phb_element[ref_index].entry.tag,
                               check_tag, 
                               phb_element[ref_index].entry.tag_len );
        cmpLen = MINIMUM ( phb_element[cur_index].entry.tag_len,
                           phb_element[ref_index].entry.tag_len );
      
        cmp_res = cmpString ( cur_tag, check_tag, cmpLen );
      }

      if (cmp_res EQ 0)  /* MINIMUM character match, so check if one string is longer */
      {                  /* ABC should come after AB */
        if (phb_element[cur_index].entry.tag_len NEQ phb_element[ref_index].entry.tag_len)
        {
          if ((phb_element[cur_index].entry.tag_len - phb_element[ref_index].entry.tag_len) > 0)
            cmp_res = cmpLen + 1;
          else
            cmp_res = -cmpLen - 1;
        }
      }

      if(cmp_res > 0)
      {       
        ptr_index = phb_element[ref_index].next_trcd;
        if (ptr_index != UNUSED_INDEX)
        {
          phb_element[ptr_index].prev_trcd = cur_index;
        }
        phb_element[cur_index].next_trcd = ptr_index;

        ptr_index = phb_element[cur_index].prev_trcd;
        if (ptr_index != UNUSED_INDEX)
        {
          phb_element[ptr_index].next_trcd = ref_index;
        }

        phb_element[ref_index].prev_trcd = phb_element[cur_index].prev_trcd;
        phb_element[ref_index].next_trcd = cur_index;
        phb_element[cur_index].prev_trcd = ref_index;

        /* set the first record in control block */
        if(!flag)
        {
          phb_ctb[phb_element[cur_index].type].first_trcd = ref_index;
          flag = 1;
        }
        ref_index = phb_element[cur_index].next_trcd;
      }
      else
      {
        ref_index = phb_element[ref_index].next_trcd;
      }
    }
  }
}


/*
+---------------------------------------------------------------------+
| PROJECT : MMI-Framework (8417)         MODULE: PHB                  |
| STATE   : code                         ROUTINE : pb_num_sort        |
+---------------------------------------------------------------------+

  PURPOSE :

*/

void pb_num_sort(SHORT cur_index)
{
  SHORT          ref_index;
  SHORT          ptr_index;
  UBYTE          flag;
  CHAR           cur_number[MAX_PHB_NUM_LEN];
  CHAR           ref_number[MAX_PHB_NUM_LEN];

  /* TRACE_FUNCTION ("pb_num_sort()");*/

  /* set the new record as first element */
  if (phb_ctb[phb_element[cur_index].type].used_rcd EQ 1)
  {
    phb_element[cur_index].prev_nrcd = UNUSED_INDEX;
    phb_element[cur_index].next_nrcd = UNUSED_INDEX;

    phb_ctb[phb_element[cur_index].type].first_nrcd = cur_index;
  }

  if (phb_ctb[phb_element[cur_index].type].used_rcd > 1)
  {
    ref_index = phb_ctb[phb_element[cur_index].type].first_nrcd;
    phb_ctb[phb_element[cur_index].type].first_nrcd = cur_index;

    phb_element[cur_index].prev_nrcd = UNUSED_INDEX;
    phb_element[cur_index].next_nrcd = ref_index;
    phb_element[ref_index].prev_nrcd = cur_index;

    /* insert the new record in the number order */
    flag = 0;
    while (ref_index NEQ UNUSED_INDEX)
    {
      /* convert the number in BCD to string */
      cmhPHB_getAdrStr(cur_number,
        MAX_PHB_NUM_LEN - 1,
        phb_element[cur_index].entry.number,
        phb_element[cur_index].entry.len);
      cmhPHB_getAdrStr(ref_number,
        MAX_PHB_NUM_LEN - 1,
        phb_element[ref_index].entry.number,
        phb_element[ref_index].entry.len);

      if (strcmp((char *)cur_number, (char *)ref_number) > 0)
      {
        ptr_index = phb_element[ref_index].next_nrcd;
        if (ptr_index != UNUSED_INDEX)
          phb_element[ptr_index].prev_nrcd = cur_index;
        phb_element[cur_index].next_nrcd = ptr_index;

        ptr_index = phb_element[cur_index].prev_nrcd;
        if (ptr_index != UNUSED_INDEX)
          phb_element[ptr_index].next_nrcd = ref_index;

        phb_element[ref_index].prev_nrcd = phb_element[cur_index].prev_nrcd;
        phb_element[ref_index].next_nrcd = cur_index;
        phb_element[cur_index].prev_nrcd = ref_index;

        /* set the first logic number record in control block */
        if(!flag)
        {
          phb_ctb[phb_element[cur_index].type].first_nrcd = ref_index;
          flag = 1;
        }
        ref_index = phb_element[cur_index].next_nrcd;
      }
      else
      {
        ref_index = phb_element[ref_index].next_nrcd;
      }
    }
  }
}


/*
+---------------------------------------------------------------------+
| PROJECT : MMI-Framework (8417)         MODULE: PHB                  |
| STATE   : code                         ROUTINE : pb_malpha_sort     |
+---------------------------------------------------------------------+

  PURPOSE : Insert a new record to alpha sorted chain.

*/

void pb_malpha_sort(SHORT cur_index)
{
  SHORT ref_index;
  SHORT ptr_index;
  UBYTE flag, cmpLen = 0;
  UBYTE cur_tag[PHB_MAX_TAG_LEN], check_tag[PHB_MAX_TAG_LEN];
  int   cmp_res;
  
  if (((phb_ctb[ADN].used_rcd EQ 1) AND (phb_ctb[FDN].used_rcd EQ 0))
    OR ((phb_ctb[ADN].used_rcd EQ 0) AND (phb_ctb[FDN].used_rcd EQ 1)))
  {
    phb_element[cur_index].prev_mtrcd = UNUSED_INDEX;
    phb_element[cur_index].next_mtrcd = UNUSED_INDEX;

    phb_ctb[phb_element[cur_index].type].first_mtrcd = cur_index;
    return;
  }
  else
  {
    if (phb_ctb[ADN].used_rcd NEQ 0)
    {
      /* Test whether ADN's first_mtrcd index is already in use or not.
      If not and FDN entries exist, take FDN's first_mtrcd. */      
      if ((phb_ctb[ADN].first_mtrcd EQ UNUSED_INDEX) AND (phb_ctb[FDN].used_rcd NEQ 0))
       ref_index = phb_ctb[FDN].first_mtrcd;
      else
       ref_index = phb_ctb[ADN].first_mtrcd;
      
      phb_ctb[ADN].first_mtrcd = cur_index;
    }
    else
    {
      ref_index = phb_ctb[FDN].first_mtrcd;
      phb_ctb[FDN].first_mtrcd = cur_index;
    }
  }

  phb_element[cur_index].prev_mtrcd = UNUSED_INDEX;
  phb_element[cur_index].next_mtrcd = ref_index;
  phb_element[ref_index].prev_mtrcd = cur_index;

  /* insert the new record in the alpha order */
  flag = 0;
  while (ref_index NEQ UNUSED_INDEX)
  {
    memset(cur_tag, 0, sizeof ( cur_tag ) );
    memset(check_tag, 0, sizeof ( check_tag ) );

    /*
      this should not cause problems, because in both alphabets 
      (GSM and ASCII) the most important chars are at the same 
      positions (A-Z: 65-90 a-z:97-122)
    */
    if( ext_compare_fct != NULL )
    {
      cmp_res = ext_compare_fct ( phb_element[cur_index].entry.tag, 
                                  phb_element[cur_index].entry.tag_len,
                                  phb_element[ref_index].entry.tag,
                                  phb_element[ref_index].entry.tag_len );
    }
    else
    {
      pb_cvt_alpha_for_cmp ( phb_element[cur_index].entry.tag,
                             cur_tag,
                             phb_element[cur_index].entry.tag_len );
      pb_cvt_alpha_for_cmp ( phb_element[ref_index].entry.tag,
                             check_tag,
                             phb_element[ref_index].entry.tag_len );
      cmpLen = MINIMUM ( phb_element[cur_index].entry.tag_len,
                         phb_element[ref_index].entry.tag_len );

      cmp_res = cmpString ( cur_tag, check_tag, cmpLen );
    }

    if (cmp_res EQ 0)  /* MINIMUM character match, so check if one string is longer */
    {                  /* ABC should come after AB */
      if (phb_element[cur_index].entry.tag_len NEQ phb_element[ref_index].entry.tag_len)
      {
        if ((phb_element[cur_index].entry.tag_len - phb_element[ref_index].entry.tag_len) > 0)
          cmp_res = cmpLen + 1;
        else
          cmp_res = -cmpLen - 1;
      }
    }

    if (cmp_res > 0)
    {
      ptr_index = phb_element[ref_index].next_mtrcd;
      if (ptr_index != UNUSED_INDEX)
        phb_element[ptr_index].prev_mtrcd = cur_index;
      phb_element[cur_index].next_mtrcd = ptr_index;

      ptr_index = phb_element[cur_index].prev_mtrcd;
      if (ptr_index != UNUSED_INDEX)
        phb_element[ptr_index].next_mtrcd = ref_index;

      phb_element[ref_index].prev_mtrcd = phb_element[cur_index].prev_mtrcd;
      phb_element[ref_index].next_mtrcd = cur_index;
      phb_element[cur_index].prev_mtrcd = ref_index;

      /* set the first record in control block */
      if(!flag)
      {
        if (phb_ctb[ADN].used_rcd != 0)
          phb_ctb[ADN].first_mtrcd = ref_index;
        else
          phb_ctb[FDN].first_mtrcd = ref_index;

        flag = 1;
      }
      ref_index = phb_element[cur_index].next_mtrcd;
    }
    else
      ref_index = phb_element[ref_index].next_mtrcd;
  }
}


/*
+---------------------------------------------------------------------+
| PROJECT : MMI-Framework (8417)         MODULE: PHB                  |
| STATE   : code                         ROUTINE : pb_mnum_sort       |
+---------------------------------------------------------------------+

  PURPOSE :

*/

void pb_mnum_sort(SHORT cur_index)
{
  SHORT          ref_index;
  SHORT          ptr_index;
  UBYTE          flag;
  CHAR           cur_number[MAX_PHB_NUM_LEN];
  CHAR           ref_number[MAX_PHB_NUM_LEN];

  /*  TRACE_FUNCTION ("pb_num_sort()");*/

  if (((phb_ctb[ADN].used_rcd EQ 1) AND (phb_ctb[FDN].used_rcd EQ 0))
    OR ((phb_ctb[ADN].used_rcd EQ 0) AND (phb_ctb[FDN].used_rcd EQ 1)))
  {
    phb_element[cur_index].prev_mnrcd = UNUSED_INDEX;
    phb_element[cur_index].next_mnrcd = UNUSED_INDEX;

    phb_ctb[phb_element[cur_index].type].first_mnrcd = cur_index;
    return;
  }
  else
  {
    if (phb_ctb[ADN].used_rcd != 0)
    {
      /* Test whether ADN's first_mtrcd index is already in use or not.
      If not and FDN entries exist, take FDN's first_mtrcd. */      
      if ((phb_ctb[ADN].first_mnrcd EQ UNUSED_INDEX) AND (phb_ctb[FDN].used_rcd NEQ 0))
       ref_index = phb_ctb[FDN].first_mnrcd;
      else
       ref_index = phb_ctb[ADN].first_mnrcd;
      
      phb_ctb[ADN].first_mnrcd = cur_index;
    }
    else
    {
      ref_index = phb_ctb[FDN].first_mnrcd;
      phb_ctb[FDN].first_mnrcd = cur_index;
    }
  }

  phb_element[cur_index].prev_mnrcd = UNUSED_INDEX;
  phb_element[cur_index].next_mnrcd = ref_index;
  phb_element[ref_index].prev_mnrcd = cur_index;

  /* insert the new record in the number order */
  flag = 0;
  while (ref_index NEQ UNUSED_INDEX)
  {
    /* convert the number in BCD to string */
    cmhPHB_getAdrStr(cur_number,
      MAX_PHB_NUM_LEN - 1,
      phb_element[cur_index].entry.number,
      phb_element[cur_index].entry.len);
    cmhPHB_getAdrStr(ref_number,
      MAX_PHB_NUM_LEN - 1,
      phb_element[ref_index].entry.number,
      phb_element[ref_index].entry.len);

    if (strcmp((char *)cur_number, (char *)ref_number) > 0)
    {
      ptr_index = phb_element[ref_index].next_mnrcd;
      if (ptr_index != UNUSED_INDEX)
        phb_element[ptr_index].prev_mnrcd = cur_index;
      phb_element[cur_index].next_mnrcd = ptr_index;

      ptr_index = phb_element[cur_index].prev_mnrcd;
      if (ptr_index != UNUSED_INDEX)
        phb_element[ptr_index].next_mnrcd = ref_index;

      phb_element[ref_index].prev_mnrcd = phb_element[cur_index].prev_mnrcd;
      phb_element[ref_index].next_mnrcd = cur_index;
      phb_element[cur_index].prev_mnrcd = ref_index;

      /* set the first logic number record in control block */
      if(!flag)
      {
        if (phb_ctb[ADN].used_rcd != 0)
          phb_ctb[ADN].first_mnrcd = ref_index;
        else
          phb_ctb[FDN].first_mnrcd = ref_index;
        flag = 1;
      }
      ref_index = phb_element[cur_index].next_mnrcd;
    }
    else
      ref_index = phb_element[ref_index].next_mnrcd;
  }
}


/*
+---------------------------------------------------------------------+
| PROJECT : MMI-Framework (8417)         MODULE: PHB                  |
| STATE   : code                         ROUTINE : pb_add_record      |
+---------------------------------------------------------------------+

  PURPOSE :

*/

T_PHB_RETURN pb_add_record(UBYTE type, UBYTE index, T_PHB_RECORD *entry)
{
  UBYTE          bit = 0;
  UBYTE          tag_len;
  SHORT         new_index;
  SHORT         cur_index;
  SHORT          first_id;
  SHORT          result;
  T_PHB_RECORD   found_entry;
  UBYTE          n,m;
  UBYTE          number[MAX_PHB_NUM_LEN];
  T_PHB_RETURN   sim_result;
  UBYTE          old_ext_rcd_num = 0xFF;
  int            entrylen;
#ifdef PHONEBOOK_EXTENSION
  USHORT         file_id = 0;
#endif

  TRACE_FUNCTION ("pb_add_record()");

  /* check whether this phonebook exists */
  if (phb_ctb[type].mem EQ NO_PHB_ENTRY)
    return PHB_FAIL;

  if (type EQ ECC
       OR type EQ ADN
       OR type EQ FDN
       OR type EQ BDN
       OR type EQ UPN)
  {
    /* Phonebook is full. */
    if (phb_ctb[type].used_rcd >= phb_ctb[type].max_rcd AND !index)
    {
      return PHB_FULL;
    }

//TI-SH-TEST-PATCH for CSR OMAPS00168884 
    tag_len = entry->tag_len;//pb_get_entry_len( entry->tag, PHB_MAX_TAG_LEN );
    /* tag_len = MINIMUM ( tag_len, phb_ctb[type].alpha_len); */
    /* Don't truncate a tag if it does not fit onto SIM but raise an error */
    if (tag_len > phb_ctb[type].alpha_len)
    {
      return PHB_TAG_EXCEEDED;
    }

    /* Search the free record number for this record */
    if (!index)
    {
      T_PHB_RETURN ret;
      SHORT first_free;

      switch (type)
      {
        case ADN:
        case FDN:
        case BDN:
        case UPN:
          /*
          *   Use the function pb_first_free() to determine
          *   the index of the first free record.
          */
          ret=pb_first_free(type,&first_free);

          /*
          *   Get out if there was a problem, or the phonebook
          *   is full.
          */
          if (ret NEQ PHB_OK)
            return(ret);

          bit=(UBYTE)(first_free-1);
          break;

        default:
          bit=0;
      }
    }

    /* Delete the information in the record, in order to overwrite */
    if (index)
    {
      bit = index - 1;
      pb_delete_record(type, index, &old_ext_rcd_num, FALSE); 
    }

    /* search a free element in phonebook element table */
    if (pb_create_memory(&new_index) NEQ PHB_OK)
      return PHB_FULL; 
    phb_element[new_index].type     = type;
#ifdef PHONEBOOK_EXTENSION    
    phb_element[new_index].entry.ext_rcd_num = 0xFF;
#endif    
    phb_ctb[type].used_rcd++;

    if (type EQ ADN OR type EQ FDN)
      phb_ctb[ADN_FDN].used_rcd++;
    n = (UBYTE)bit/8;
    m = bit%8;
    phb_ctb[type].rcd_bitmap[n] |= 0x01 << m;
    phb_element[new_index].entry.index = bit + 1;

    /* Copy entry */
    phb_element[new_index].entry.tag_len = tag_len;
    phb_element[new_index].entry.len     = entry->len;
    phb_element[new_index].entry.ton_npi = entry->ton_npi;
    memset(phb_element[new_index].entry.tag, 0xFF, PHB_MAX_TAG_LEN);
    memcpy((char *)phb_element[new_index].entry.tag, 
           (char *)entry->tag, 
           tag_len);
    memcpy((char *)phb_element[new_index].entry.number,
           (char *)entry->number, 
           PHB_PACKED_NUM_LEN);  /* allow number (10 bytes) + a extension (11 bytes) */
#ifdef PHONEBOOK_EXTENSION
    memcpy((char *)phb_element[new_index].entry.subaddr,
           (char *)entry->subaddr, PHB_PACKED_NUM_LEN);
#endif
    phb_element[new_index].entry.cc_id     = entry->cc_id;

    pb_record_sort(new_index);
    pb_alpha_sort(new_index);
    pb_num_sort(new_index);

    if ((type EQ ADN) OR (type EQ FDN))
    {
      pb_malpha_sort(new_index);
      pb_mnum_sort(new_index);
    }

    if (phb_ctb[type].mem EQ SIM_MEMORY)
    {
      /* write this record in SIM card */
      memset(data, 0xFF, sizeof(data));
      memcpy(data, entry->tag, tag_len);
      
#if PHONEBOOK_EXTENSION
      if (entry->number[10] != 0xFF)
      {
        data[phb_ctb[type].alpha_len] = 11; /* max. length */
      }
      else
      {
        data[phb_ctb[type].alpha_len] = entry->len+1;
      }
#else
      data[phb_ctb[type].alpha_len] = entry->len+1;
#endif
      data[phb_ctb[type].alpha_len+1] = entry->ton_npi;
      memcpy((char *)&data[phb_ctb[type].alpha_len+2], 
             (char *)entry->number, 10);
      data[phb_ctb[type].alpha_len+12] = entry->cc_id;

#ifdef PHONEBOOK_EXTENSION
      if (entry->number[10] != 0xFF)
      {
        file_id = pb_get_ext_file_id (type);
        if (old_ext_rcd_num NEQ 0xFF)
        {
          /* use the old extention record */
          phb_element[new_index].entry.ext_rcd_num = old_ext_rcd_num;
        }
        else
        {
          phb_element[new_index].entry.ext_rcd_num = pb_get_ext_record_number (type);
        }

        /* Not able to find free record in extension file */
        if(phb_element[new_index].entry.ext_rcd_num EQ 0xff)
        {
          /* Free the used record for normal phonebook
           * since unable to find free record for extension data */
          pb_free_used_record(type, new_index, bit);

          return PHB_EXT_FULL;
        }

        data[phb_ctb[type].alpha_len+13] = phb_element[new_index].entry.ext_rcd_num;
      }
      /* only number extention or subaddress could be store */
      else if (entry->subaddr[0] NEQ 0xFF)
      {
        file_id = pb_get_ext_file_id (type);
        if (old_ext_rcd_num NEQ 0xFF)
        {
          /* use the old extention record */
          phb_element[new_index].entry.ext_rcd_num = old_ext_rcd_num;
        }
        else
        {
          phb_element[new_index].entry.ext_rcd_num = pb_get_ext_record_number (0xFF);
        }
        
        /* Not able to find free record in extension file */
        if(phb_element[new_index].entry.ext_rcd_num EQ 0xff)
        {
          /* Free the used record for normal phonebook
           * since unable to find free record for extension data */
          pb_free_used_record(type, new_index, bit);

          return PHB_EXT_FULL;
        }

        data[phb_ctb[type].alpha_len+13] = phb_element[new_index].entry.ext_rcd_num;
     }
#endif
      sim_result = pb_write_sim(type, phb_element[new_index].entry.index);
#ifdef PHONEBOOK_EXTENSION
      if (sim_result NEQ PHB_FAIL)
      {
        if (phb_element[new_index].entry.ext_rcd_num != 0xFF)
        {
          pb_prepare_ext_data (phb_element[new_index].entry.number,
                               phb_element[new_index].entry.len,
                               phb_element[new_index].entry.subaddr,
                               10,
                               file_id);
          sim_result = pb_write_sim_ext(file_id, phb_element[new_index].entry.ext_rcd_num);
		//TISH set ext record flag
		  if(sim_result NEQ PHB_FAIL)
				pb_set_ext_record_flag(type, phb_element[new_index].entry.ext_rcd_num);

        }
        else if (old_ext_rcd_num NEQ 0xFF)
        {
          /* delete the old extention record */
          pb_rem_ext_record_flag (type, old_ext_rcd_num);
          pb_prepare_ext_data (NULL, 0, NULL, 0, file_id);
          sim_result = pb_write_sim_ext(SIM_EXT1, old_ext_rcd_num);
        }
      }
#endif /* PHONEBOOK_EXTENSION */
      return(sim_result);
    }
    return PHB_OK;
  }

  else if ((type EQ LDN) OR (type EQ LRN) OR (type EQ LMN))
  {
    cmhPHB_getAdrStr((char *)number,
      MAX_PHB_NUM_LEN - 1,
      entry->number,
      entry->len);
    if (pb_search_number(type, number, PHB_NEW_SEARCH,
      &first_id, &result, &found_entry) EQ PHB_OK)
    {
      if (result)
        pb_delete_record(type, (UBYTE)found_entry.index, &old_ext_rcd_num, FALSE);
    }

    if (phb_ctb[type].used_rcd >= phb_ctb[type].max_rcd)
    {
      if ((cur_index = phb_ctb[type].first_rcd) EQ UNUSED_INDEX)
        return PHB_FAIL;

      while (phb_l_element[cur_index].next_rcd NEQ UNUSED_BYTE_INDEX)
        cur_index = phb_l_element[cur_index].next_rcd;
      if (phb_l_element[cur_index].prev_rcd != UNUSED_BYTE_INDEX)
        phb_l_element[phb_l_element[cur_index].prev_rcd].next_rcd = UNUSED_BYTE_INDEX;
    }

    else
    {
      if (pb_create_l_memory(&new_index) NEQ PHB_OK)
        return PHB_FAIL;
      phb_ctb[type].used_rcd++;
      cur_index =  new_index;
    }

    phb_l_element[cur_index].type = type;

    /* copy record */
    /*    if ((type EQ LDN) OR (type EQ LRN)) */
    /* copy call duration - not jet done   */
    /*    if ((type EQ LRN) OR (type EQ LMN)) */
    /* copy call identifier - not jet done */

    memcpy((char *)phb_l_element[cur_index].entry.tag,
           (char *)entry->tag,
           PHB_MAX_TAG_LEN);
    phb_l_element[cur_index].entry.tag_len = entry->tag_len;

    phb_l_element[cur_index].entry.year = entry->year;
    phb_l_element[cur_index].entry.month = entry->month;
    phb_l_element[cur_index].entry.day = entry->day;
    phb_l_element[cur_index].entry.hour = entry->hour;
    phb_l_element[cur_index].entry.minute = entry->minute;
    phb_l_element[cur_index].entry.second = entry->second;
    phb_l_element[cur_index].entry.len     = entry->len;
    phb_l_element[cur_index].entry.ton_npi = entry->ton_npi;
    phb_l_element[cur_index].entry.line = entry->line;
    memcpy((char *)phb_l_element[cur_index].entry.number,
      (char *)entry->number,
      PHB_PACKED_NUM_LEN);
    phb_l_element[cur_index].entry.cc_id     = entry->cc_id;

    phb_l_element[cur_index].prev_rcd = UNUSED_BYTE_INDEX;
    if (phb_ctb[type].first_rcd EQ cur_index)
    {
      phb_l_element[cur_index].next_rcd = UNUSED_BYTE_INDEX;
    }
    else
    {
      phb_l_element[cur_index].next_rcd = (UBYTE)phb_ctb[type].first_rcd;
    }

    /* If it is -1, gardening ! This is possible for the 1rst time a entry is added ??? */
    if (phb_ctb[type].first_rcd != UNUSED_BYTE_INDEX)
    {
      phb_l_element[phb_ctb[type].first_rcd].prev_rcd = (UBYTE)cur_index;
    }

    phb_ctb[type].first_rcd = cur_index;

    new_index = 1;
    while (cur_index NEQ UNUSED_BYTE_INDEX)
    {
      phb_l_element[cur_index].entry.index = (UBYTE)new_index;
      new_index++;
      cur_index = phb_l_element[cur_index].next_rcd;
    }
    if ( type EQ LDN )
    {      
      /*  ACI-SPR-16301: Write LND entry to 1 record of SIM, to keep it up 
          to date Actually, the SIM entries will be overwritten by the LND data 
          in RAM during switch off of the ME. This mechanism here is just to 
          ensure the we retrieve more current data in the case of a SAT REFRESH 
          or unexpected RESET. */

      /* prepare entry */
      entrylen =  pb_get_entry_len( entry->tag, PHB_MAX_TAG_LEN );
      tag_len = MINIMUM ( phb_ctb[type].alpha_len, 
                          entrylen) ;
      memset(data, 0xFF, sizeof(data));
      memcpy(data, entry->tag, tag_len);
      data[phb_ctb[type].alpha_len] = entry->len+1;
      data[phb_ctb[type].alpha_len+1] = entry->ton_npi;
      memcpy((char *)&data[phb_ctb[type].alpha_len+2], 
             (char *)entry->number, 10);
      data[phb_ctb[type].alpha_len+12] = entry->cc_id; 

      /* always update the first entry */
      sim_result = pb_write_sim(type, 1);

      return sim_result;
    }
    else
    {
      return PHB_OK;
    }
  }
  else    /* unknown type */
    return PHB_FAIL;
}


/*
+------------------------------------------------------------------------+
| PROJECT : MMI-Framework (8417)         MODULE: PHB                     |
| STATE   : code                         ROUTINE : pb_delete_record      |
+------------------------------------------------------------------------+

  PURPOSE : Delete a record from phonebook.

*/

T_PHB_RETURN pb_delete_record(UBYTE type, UBYTE index, UBYTE *ext_rcd_num, BOOL permanent)
{
  SHORT cur_index;
  UBYTE cur_byte_index, new_byte_index;
  UBYTE m,n;
  T_PHB_RETURN sim_result;

  TRACE_FUNCTION ("pb_delete_record()");

  /* check whether this phonebook exists */
  if (phb_ctb[type].mem EQ NO_PHB_ENTRY)
    return PHB_FAIL;

  if ((type EQ LDN) OR (type EQ LRN) OR (type EQ LMN))
  {
    cur_byte_index = (UBYTE)phb_ctb[type].first_rcd;
    while (cur_byte_index != UNUSED_BYTE_INDEX)
    {
      if (phb_l_element[cur_byte_index].entry.index EQ index)
      {
        phb_l_element[cur_byte_index].free = PHB_ELEMENT_FREE;
        pb_l_rcd_chain(type,
                       (SHORT)phb_l_element[cur_byte_index].prev_rcd,
                       (SHORT)cur_byte_index,
                       (SHORT)phb_l_element[cur_byte_index].next_rcd);
        phb_ctb[type].used_rcd--;

        /*
         * Delete Record in PCM, else the
         * record is back after restart
         */
        if (phb_ctb[type].mem EQ TE_MEMORY)
        {
          UBYTE data[SIZE_EF_LDN];

          memset (data, 0xFF, SIZE_EF_LDN);

          switch (type)
          {
            case LDN:
              TRACE_EVENT ("Delete LDN entry");
              pcm_WriteRecord((UBYTE *)EF_LDN_ID,
                              (USHORT)(cur_byte_index+1),
                              SIZE_EF_LDN,
                              data);
              break;
            case LRN:
              TRACE_EVENT ("Delete LRN entry");
              pcm_WriteRecord((UBYTE *)EF_LRN_ID,
                              (USHORT)(cur_byte_index+1),
                              SIZE_EF_LRN,
                              data);
              break;
            case LMN:
              TRACE_EVENT ("Delete LMN entry");
              pcm_WriteRecord((UBYTE *)EF_LMN_ID,
                              (USHORT)(cur_byte_index+1),
                              SIZE_EF_LMN,
                              data);
              break;
          }
        }
        cur_byte_index = (UBYTE)phb_ctb[type].first_rcd;
        new_byte_index = 1;
        while (cur_byte_index != UNUSED_BYTE_INDEX)
        {
          phb_l_element[cur_byte_index].entry.index = new_byte_index;
          new_byte_index++;
          cur_byte_index = phb_l_element[cur_byte_index].next_rcd;
        }
        return PHB_OK;
      }
      cur_byte_index = phb_l_element[cur_byte_index].next_rcd;
    }
  }
  else /* SIM related phonebooks */
 {
  cur_index = phb_ctb[type].first_rcd;

  while (cur_index NEQ UNUSED_INDEX)
    {
      if (phb_element[cur_index].entry.index EQ index)
      {
        phb_element[cur_index].free = PHB_ELEMENT_FREE;
        pb_rcd_chain(type,
          phb_element[cur_index].prev_rcd,
          cur_index,
          phb_element[cur_index].next_rcd);
        pb_name_chain(type,
          phb_element[cur_index].prev_trcd,
          cur_index,
          phb_element[cur_index].next_trcd);
        pb_num_chain(type,
          phb_element[cur_index].prev_nrcd,
          cur_index,
          phb_element[cur_index].next_nrcd);

        if ((type EQ ADN) OR (type EQ FDN))
        {
          pb_mname_chain(type,
            phb_element[cur_index].prev_mtrcd,
            cur_index,
            phb_element[cur_index].next_mtrcd);
          pb_mnum_chain(type,
            phb_element[cur_index].prev_mnrcd,
            cur_index,
            phb_element[cur_index].next_mnrcd);

          phb_ctb[ADN_FDN].used_rcd--;
        }

        phb_ctb[type].used_rcd--;

        n = (UBYTE)(phb_element[cur_index].entry.index - 1)/8;
        m = (UBYTE)(phb_element[cur_index].entry.index - 1)%8;
        phb_ctb[type].rcd_bitmap[n] ^= 0x01 << m;

        if (phb_ctb[type].mem EQ SIM_MEMORY)
        {
#ifdef PHONEBOOK_EXTENSION
          /* store the extention record */
          *ext_rcd_num = phb_element[cur_index].entry.ext_rcd_num;
#endif
          /* write this record in SIM card */
          memset(data, 0xFF, sizeof(data));
          if (permanent)
          {
            sim_result = pb_write_sim(type, index);
#ifdef PHONEBOOK_EXTENSION            
            if ((sim_result NEQ PHB_FAIL) AND (*ext_rcd_num NEQ 0xFF))
            {
              pb_rem_ext_record_flag (type, *ext_rcd_num);
              pb_prepare_ext_data (NULL, 0, NULL, 0, pb_get_ext_file_id(type));
              sim_result = pb_write_sim_ext (pb_get_ext_file_id(type), *ext_rcd_num);            
            }
#endif            
            return (sim_result);
          }
        }
        return PHB_OK;
      }
      cur_index = phb_element[cur_index].next_rcd;
    }
  }

  return PHB_FAIL;
}



void copy_phb_element ( T_PHB_RECORD *entry, T_PHB_AFB_ELEMENT phb_element )
{
  memset (entry, 0xff, sizeof(T_PHB_RECORD));

  entry->book = phb_element.type;
  entry->index = phb_element.entry.index;
  entry->tag_len = phb_element.entry.tag_len;
  memcpy((char *)entry->tag, 
         (char *)phb_element.entry.tag,
         PHB_MAX_TAG_LEN);
  entry->len = phb_element.entry.len;
  entry->ton_npi = phb_element.entry.ton_npi;
  memcpy((char *)entry->number,
         (char *)phb_element.entry.number,
         PHB_PACKED_NUM_LEN);
#ifdef PHONEBOOK_EXTENSION
  memcpy((char *)entry->subaddr,
         (char *)phb_element.entry.subaddr,
         PHB_PACKED_NUM_LEN);
#endif
  entry->cc_id = phb_element.entry.cc_id;
}


void copy_phb_l_element ( T_PHB_RECORD *entry, T_PHB_RDM_ELEMENT phb_l_element )
{
  memset (entry, 0xff, sizeof(T_PHB_RECORD));

  entry->book = phb_l_element.type;
  entry->index = phb_l_element.entry.index;
  entry->tag_len = phb_l_element.entry.tag_len;
  memcpy((char *)entry->tag,
         (char *)phb_l_element.entry.tag,
         PHB_MAX_TAG_LEN);
  entry->len = phb_l_element.entry.len;
  entry->ton_npi = phb_l_element.entry.ton_npi;
  memcpy((char *)entry->number,
         (char *)phb_l_element.entry.number,
         PHB_PACKED_NUM_LEN);
#ifdef PHONEBOOK_EXTENSION
  memcpy((char *)entry->subaddr,
         (char *)phb_l_element.entry.subaddr,
         PHB_PACKED_NUM_LEN);
#endif
  entry->cc_id = phb_l_element.entry.cc_id;
  entry->line = phb_l_element.entry.line;

  entry->year = phb_l_element.entry.year;
  entry->month = phb_l_element.entry.month;
  entry->day = phb_l_element.entry.day;
  entry->hour = phb_l_element.entry.hour;
  entry->minute = phb_l_element.entry.minute;
  entry->second = phb_l_element.entry.second;
}




/*
+------------------------------------------------------------------------+
| PROJECT : MMI-Framework (8417)         MODULE: PHB                     |
| STATE   : code                         ROUTINE : pb_read_phys_record   |
+------------------------------------------------------------------------+

  PURPOSE : Read one record according to physical index from phonebook.

*/
T_PHB_RETURN pb_read_phys_record(UBYTE type, SHORT index, T_PHB_RECORD *entry)
{
  SHORT cur_index;
  SHORT i;

  /*  TRACE_FUNCTION ("pb_read_phys_record()");*/

  /* check whether this phonebook exists */
  if (phb_ctb[type].mem EQ NO_PHB_ENTRY)
    return PHB_FAIL;

  if (index > phb_ctb[type].max_rcd)
    return PHB_INVALID_IDX;

  cur_index = phb_ctb[type].first_rcd;
  if (cur_index EQ UNUSED_INDEX)
    return PHB_FAIL;

  if (type EQ ECC
      OR type EQ ADN
      OR type EQ FDN
      OR type EQ BDN
      OR type EQ UPN
      OR type EQ SDN )
  {
    for (i=0; i<phb_ctb[type].used_rcd; i++)
    {
      if (phb_element[cur_index].entry.index EQ index)
      {
        copy_phb_element (entry, phb_element[cur_index]);
        return PHB_OK;
      }
      cur_index = phb_element[cur_index].next_rcd;
      if (cur_index EQ UNUSED_INDEX)
        break;
    }
  }

  else if (type EQ LDN
      OR type EQ LRN
      OR type EQ LMN)
  {
    for (i=0; i<phb_ctb[type].used_rcd; i++)
    {
      if (phb_l_element[cur_index].entry.index EQ index)
      {
        copy_phb_l_element (entry, phb_l_element[cur_index]);
        return PHB_OK;
      }
      cur_index = phb_l_element[cur_index].next_rcd;
      if (cur_index EQ UNUSED_INDEX)
        break;
    }
  }

  return PHB_FAIL;
}

/*
+------------------------------------------------------------------------+
| PROJECT : MMI-Framework (8417)         MODULE: PHB                     |
| STATE   : code                         ROUTINE : pb_read_index_record  |
+------------------------------------------------------------------------+

  PURPOSE : Read one record according to index from phonebook.

*/

T_PHB_RETURN pb_read_index_record(UBYTE type, SHORT index, T_PHB_RECORD *entry)
{
  SHORT cur_index;
  SHORT i;

  /*  TRACE_FUNCTION ("pb_read_index_record()");*/

  if (phb_ctb[type].mem EQ NO_PHB_ENTRY)
    return PHB_FAIL;

  if (index > phb_ctb[type].max_rcd)
    return PHB_INVALID_IDX;

  cur_index = phb_ctb[type].first_rcd;
  if (cur_index EQ UNUSED_INDEX)
    return PHB_FAIL;

  if (type EQ ECC
      OR type EQ ADN
      OR type EQ FDN
      OR type EQ BDN
      OR type EQ UPN
      OR type EQ SDN)
  {
    for (i=1; i<index; i++)
    {
      cur_index = phb_element[cur_index].next_rcd;
      if (cur_index EQ UNUSED_INDEX)
        return PHB_FAIL;
    }
    copy_phb_element (entry, phb_element[cur_index]);
    return PHB_OK;
  }

  else if (type EQ LDN
           OR type EQ LRN
           OR type EQ LMN)
  {
    for (i=1; i<index; i++)
    {
      cur_index = phb_l_element[cur_index].next_rcd;
      if (cur_index EQ UNUSED_BYTE_INDEX)
        return PHB_FAIL;
    }
    copy_phb_l_element (entry, phb_l_element[cur_index]);
    return PHB_OK;
  }

  return PHB_FAIL;
}

/*
+------------------------------------------------------------------------+
| PROJECT : MMI-Framework (8417)         MODULE: PHB                     |
| STATE   : code                         ROUTINE : pb_read_alpha_record  |
+------------------------------------------------------------------------+

  PURPOSE : Read one record according to alpha index from phonebook.

*/

T_PHB_RETURN pb_read_alpha_record(UBYTE type, SHORT index, T_PHB_RECORD *entry)
{
  SHORT cur_index;
  SHORT i;

  /*  TRACE_FUNCTION ("pb_read_alpha_record()");*/

  if (type EQ ADN_FDN)
  {
    /* check whether this phonebook exists */
    if ((phb_ctb[ADN].mem EQ NO_PHB_ENTRY)
      AND (phb_ctb[FDN].mem EQ NO_PHB_ENTRY))
      return PHB_FAIL;

    cur_index = phb_ctb[ADN].first_mtrcd;
    if (cur_index EQ UNUSED_INDEX)
      cur_index = phb_ctb[FDN].first_mtrcd;
    if (cur_index EQ UNUSED_INDEX)
      return PHB_FAIL;

    for (i=1; i<index; i++)
    {
      cur_index = phb_element[cur_index].next_mtrcd;
      if (cur_index EQ UNUSED_INDEX)
        return PHB_FAIL;
    }
    copy_phb_element (entry, phb_element[cur_index]);
    return PHB_OK;
  }
  else if (type EQ ECC
           OR type EQ ADN
           OR type EQ FDN
           OR type EQ BDN
           OR type EQ SDN
           OR type EQ UPN)
  {
    /* check whether this phonebook exists */
    if (phb_ctb[type].mem EQ NO_PHB_ENTRY)
      return PHB_FAIL;

    cur_index = phb_ctb[type].first_trcd;
    if (cur_index EQ UNUSED_INDEX)
      return PHB_FAIL;

    for (i=1; i<index; i++)
    {
      cur_index = phb_element[cur_index].next_trcd;
      if (cur_index EQ UNUSED_INDEX)
        return PHB_FAIL;
    }
    copy_phb_element (entry, phb_element[cur_index]);
    return PHB_OK;
  }
  else  /* LDN, LRN, LMN */
    return PHB_FAIL;
}

/*
+------------------------------------------------------------------------+
| PROJECT : MMI-Framework (8417)         MODULE: PHB                     |
| STATE   : code                         ROUTINE : pb_read_number_record |
+------------------------------------------------------------------------+

  PURPOSE : Read one record according to number index from phonebook.

*/

T_PHB_RETURN pb_read_number_record(UBYTE type, SHORT index, T_PHB_RECORD *entry)
{
  SHORT cur_index;
  SHORT          i;

  /*  TRACE_FUNCTION ("pb_read_number_record()");*/

  if (type EQ ADN_FDN)
  {
    /* check whether this phonebook exists */
    if ((phb_ctb[ADN].mem EQ NO_PHB_ENTRY)
      AND (phb_ctb[FDN].mem EQ NO_PHB_ENTRY))
      return PHB_FAIL;

    
    cur_index = phb_ctb[ADN].first_mnrcd;
    if (cur_index EQ UNUSED_INDEX)
      cur_index = phb_ctb[FDN].first_mnrcd;
    if (cur_index EQ UNUSED_INDEX)
      return PHB_FAIL;

    for (i=1; i<index; i++)
    {
      cur_index = phb_element[cur_index].next_mnrcd;
      if (cur_index EQ UNUSED_INDEX)
        return PHB_FAIL;
    }
    copy_phb_element (entry, phb_element[cur_index]);
    return PHB_OK;
  }
  else if (type EQ ECC
           OR type EQ ADN
           OR type EQ FDN
           OR type EQ BDN
           OR type EQ SDN
           OR type EQ UPN)
  {
    /* check whether this phonebook exists */
    if (phb_ctb[type].mem EQ NO_PHB_ENTRY)
      return PHB_FAIL;

    cur_index = phb_ctb[type].first_nrcd;
    if (cur_index EQ UNUSED_INDEX)
      return PHB_FAIL;

    for (i=1; i<index; i++)
    {
      cur_index = phb_element[cur_index].next_nrcd;
      if (cur_index EQ UNUSED_INDEX)
        return PHB_FAIL;
    }
    copy_phb_element (entry, phb_element[cur_index]);
    return PHB_OK;
  }
  else
    /* LDN, LRN, LMN */
    return PHB_FAIL;
}

/*
+------------------------------------------------------------------------+
| PROJECT : MMI-Framework (8417)         MODULE: PHB                     |
| STATE   : code                         ROUTINE : pb_search_name        |
+------------------------------------------------------------------------+

  PURPOSE : Search the string from phonebook

*/

T_PHB_RETURN pb_search_name(T_ACI_CMD_SRC srcID,
                            UBYTE         type,
                            T_ACI_PB_TEXT *search_tag,
                            UBYTE         mode,
                            SHORT         *first_ind,
                            SHORT         *result,
                            T_PHB_RECORD  *entry)
{
  static SHORT  ptr_index = UNUSED_INDEX;
  SHORT         cur_index;
  SHORT         index;
  int           cmp_res;
  int           cmp_len;
  int           max_cmp_len;

  /*  TRACE_FUNCTION ("pb_search_name()");*/

  if (type EQ ADN_FDN)
  {
    /* check whether this phonebook exists */
    if ((phb_ctb[ADN].mem EQ NO_PHB_ENTRY)
      AND (phb_ctb[FDN].mem EQ NO_PHB_ENTRY))
      return PHB_FAIL;
    
    if ((phb_ctb[ADN].first_mtrcd EQ UNUSED_INDEX)
      AND (phb_ctb[FDN].first_mtrcd EQ UNUSED_INDEX))
      return PHB_FAIL;
  }
  else
  {
    /* check whether this phonebook exists */
    if (phb_ctb[type].mem EQ NO_PHB_ENTRY)
      return PHB_FAIL;
  }

  /* search string */
  if ( mode EQ PHB_NEW_SEARCH )
  {
    TRACE_EVENT("pb_search_name() DEB:  mode EQ PHB_NEW_SEARCH");
    ptr_index = UNUSED_INDEX;
    max_cmp_len = 0;
    index = 1;
    *result = 0;

    if (type EQ ADN_FDN)
    {
      if (phb_ctb[ADN].first_mtrcd != UNUSED_INDEX)
        cur_index = phb_ctb[ADN].first_mtrcd;
      else
        cur_index = phb_ctb[FDN].first_mtrcd;
    }
    else
      cur_index = phb_ctb[type].first_trcd;

    while (cur_index != UNUSED_INDEX)
    {
      /*
         this should not cause problems, because in both alphabets
         (GSM and ASCII) the most important chars are at the same
         positions (A-Z: 65-90 // a-z:97-122)
      */
      if (ext_compare_fct != NULL)
      {
        cmp_res = ext_compare_fct (phb_element[cur_index].entry.tag,
                                   phb_element[cur_index].entry.tag_len,
                                   search_tag->data,
                                   search_tag->len);
      }
      else
      {
        cmp_res = pb_cmp_phb_entry ( phb_element[cur_index].entry.tag,
                                     phb_element[cur_index].entry.tag_len,
                                     search_tag );
      }

#if defined(MFW) OR defined (FF_MMI_RIV)
      if (srcID EQ CMD_SRC_LCL)
      {
        /* if search string is before entry in alphabet */
        if (search_tag->len EQ 1 AND
            (cmp_res >= 0) AND (phb_element[cur_index].entry.tag[0] NEQ '\0'))
        {
          cmp_res = 0;
        }
      }
      else
#endif
      {
        if (cmp_res EQ 0)  /* If Searchstring matches the first letters of Phonebook */
        {
          if (search_tag->len <= phb_element[cur_index].entry.tag_len)
            ;    /* MATCH */
          else
            cmp_res = search_tag->len + 1;
        }
      }

      if (cmp_res EQ 0)
      {
        cmp_len = MINIMUM(phb_element[cur_index].entry.tag_len, search_tag->len);
        if ( cmp_len > max_cmp_len )
        {
          max_cmp_len = cmp_len;
          if ( ptr_index EQ UNUSED_INDEX )
          {
            /* save the index of the first found record */
            TRACE_EVENT("pb_search_name() DEB: first index");
          }
          ptr_index = cur_index;
          *first_ind = index;
        }
        (*result)++;

      }
      if (type EQ ADN_FDN)
        cur_index = phb_element[cur_index].next_mtrcd;
      else
        cur_index = phb_element[cur_index].next_trcd;
      index++;
    }
  }
  else
  {
    TRACE_EVENT("pb_search_name() DEB:  mode EQ PHB_NEXT_SEARCH");
  }

  if ( ptr_index EQ UNUSED_INDEX )
  {
    TRACE_EVENT("pb_search_name() ERR: name not found");
    return PHB_FAIL;
  }

  /* copy the found record */
  copy_phb_element( entry, phb_element[ptr_index] );

  if (type EQ ADN_FDN)
    ptr_index = phb_element[ptr_index].next_mtrcd;
  else
    ptr_index = phb_element[ptr_index].next_trcd;

  return PHB_OK;
}

/*
+------------------------------------------------------------------------+
| PROJECT : MMI-Framework (8417)         MODULE: PHB                     |
| STATE   : code                         ROUTINE : pb_search_number      |
+------------------------------------------------------------------------+

  PURPOSE :

*/

T_PHB_RETURN pb_search_number(UBYTE         type,
                              UBYTE        *number,
                              UBYTE         mode,
                              SHORT        *first_ind,
                              SHORT        *result,
                              T_PHB_RECORD *entry)
{
  static SHORT count = 0;
  static SHORT ptr_index;
  SHORT        cur_index;
  UBYTE         cur_byte_index;

  UBYTE        flag;
  SHORT        index;
  CHAR         cur_number[MAX_PHB_NUM_LEN];
  static CHAR  search_number[MAX_PHB_NUM_LEN];

  TRACE_FUNCTION ("pb_search_number()");

  if (type EQ ADN_FDN)
  {
    /* check whether this phonebook exists */
    if ((phb_ctb[ADN].mem EQ NO_PHB_ENTRY)
      AND (phb_ctb[FDN].mem EQ NO_PHB_ENTRY))
      return PHB_FAIL;

    if ((phb_ctb[ADN].first_mnrcd EQ UNUSED_INDEX)
      AND (phb_ctb[FDN].first_mnrcd EQ UNUSED_INDEX))
      return PHB_FAIL;
  }
  else
  {
    /* check whether this phonebook exists */
    if (phb_ctb[type].mem EQ NO_PHB_ENTRY AND type NEQ ECC)
      return PHB_FAIL;
  }

  if (type EQ LDN
    OR type EQ LRN
    OR type EQ LMN)
  {
    *result = 0;
    if (!phb_ctb[type].used_rcd)
      return PHB_OK;

    cur_byte_index = (UBYTE) phb_ctb[type].first_rcd;

    while (cur_byte_index != UNUSED_BYTE_INDEX)
    {
      /* convert the number in BCD to string */
      cmhPHB_getAdrStr(cur_number,
                       MAX_PHB_NUM_LEN - 1,
                       phb_l_element[cur_byte_index].entry.number,
                       phb_l_element[cur_byte_index].entry.len);
      if (!strcmp((char *)cur_number, (char *)number))
      {
        copy_phb_l_element(entry, phb_l_element[cur_byte_index]);

        *result = 1;
        return PHB_OK;
      }
      cur_byte_index = phb_l_element[cur_byte_index].next_rcd;
    }
  }

  /* check emergency call */
  if (type EQ ECC)
  {
    *result = 0;

   /*    if (!strcmp("112", (char *)number) || !strcmp("911", (char *)number) || !strcmp("999", (char *)number))
    {
      entry->book = ECC;
      entry->index = 1;
      entry->tag_len = 0;
      entry->tag[0] = 0;
      cmhPHB_getAdrBcd ( entry->number,
        &entry->len,
      PHB_PACKED_NUM_LEN, (char *)number );
      entry->ton_npi = 0xFF;
      entry->cc_id = 0xFF;
      *result = 1;
      return PHB_OK;
    }*/

    if (!phb_ctb[type].used_rcd)
      return PHB_OK;

    cur_index = phb_ctb[type].first_nrcd;

    while (cur_index != UNUSED_INDEX)
    {
      /* convert the number in BCD to string */
      cmhPHB_getAdrStr(cur_number,
        PHB_PACKED_NUM_LEN - 1,
        phb_element[cur_index].entry.number,
        phb_element[cur_index].entry.len);
      if (!strcmp((char *)cur_number, (char *)number))
      {
        copy_phb_element (entry, phb_element[cur_index]);
        *result = 1;
        return PHB_OK;
      }
      cur_index = phb_element[cur_index].next_nrcd;
    }
    return PHB_OK;
  }

  /* search phone number */
  if (mode EQ PHB_NEW_SEARCH)
  {
    count = 0;
    flag  = 0;
    index = 1;
    strncpy(search_number, (char *)number, MAX_PHB_NUM_LEN-1);
    search_number[MAX_PHB_NUM_LEN-1] = '\0';

    if (type EQ ADN_FDN)
    {
      if (phb_ctb[ADN].first_mnrcd != UNUSED_INDEX)
        cur_index = phb_ctb[ADN].first_mnrcd;
      else
        cur_index = phb_ctb[FDN].first_mnrcd;
    }
    else
      cur_index = phb_ctb[type].first_nrcd;

    while (cur_index != UNUSED_INDEX)
    {
      /* convert the number in BCD to string */
      cmhPHB_getAdrStr(cur_number,
        MAX_PHB_NUM_LEN - 1,
        phb_element[cur_index].entry.number,
        phb_element[cur_index].entry.len);

      if (pb_check_number(cur_number, (char *)number))
      {
        if (!flag)
        {
          ptr_index = cur_index;    /* save the index of the first found record */
          *first_ind = index;
          flag = 1;
        }
        count++;
      }
      if (type EQ ADN_FDN)
        cur_index = phb_element[cur_index].next_mnrcd;
      else
        cur_index = phb_element[cur_index].next_nrcd;
      index++;
    }

    *result = count;
  }

  if (mode EQ PHB_NEXT_SEARCH AND count)
  {
    while (ptr_index != UNUSED_INDEX)
    {
      /* convert the number in BCD to string */
      cmhPHB_getAdrStr(cur_number,
        MAX_PHB_NUM_LEN - 1,
        phb_element[ptr_index].entry.number,
        phb_element[ptr_index].entry.len);

      if (pb_check_number(cur_number, search_number))
        break;

      if (type EQ ADN_FDN)
        ptr_index = phb_element[ptr_index].next_mnrcd;
      else
        ptr_index = phb_element[ptr_index].next_nrcd;
    }
  }

  /* copy the found record */
  if (count)
  {
    copy_phb_element( entry, phb_element[ptr_index] );
    if (type EQ ADN_FDN)
      ptr_index = phb_element[ptr_index].next_mnrcd;
    else
      ptr_index = phb_element[ptr_index].next_nrcd;
    count--;
  }
  else
    return PHB_FAIL;

  return PHB_OK;
}

/*
+------------------------------------------------------------------------+
| PROJECT : MMI-Framework (8417)         MODULE: PHB                     |
| STATE   : code                         ROUTINE : pb_rcd_chain          |
+------------------------------------------------------------------------+

  PURPOSE : Chain the element according to record number.

*/

void pb_rcd_chain(UBYTE type,
      SHORT prev_index,
      SHORT cur_index,
      SHORT next_index)
{
  /*  TRACE_FUNCTION ("pb_rcd_chain()");*/

  if ((phb_element[cur_index].prev_rcd EQ UNUSED_INDEX)
    AND (phb_element[cur_index].next_rcd EQ UNUSED_INDEX))
  {
    phb_ctb[type].first_rcd = UNUSED_INDEX;
  }
  else if ((phb_element[cur_index].prev_rcd NEQ UNUSED_INDEX)
    AND (phb_element[cur_index].next_rcd EQ UNUSED_INDEX))
  {
    phb_element[prev_index].next_rcd = UNUSED_INDEX;
  }
  else if ((phb_element[cur_index].prev_rcd EQ UNUSED_INDEX)
    AND (phb_element[cur_index].next_rcd NEQ UNUSED_INDEX))
  {
    phb_element[next_index].prev_rcd  = UNUSED_INDEX;
    phb_element[cur_index].next_rcd = UNUSED_INDEX;   /* ??? */
    phb_ctb[type].first_rcd = next_index;
  }
  else if ((phb_element[cur_index].prev_rcd NEQ UNUSED_INDEX)
    AND (phb_element[cur_index].next_rcd NEQ UNUSED_INDEX))
  {
    phb_element[prev_index].next_rcd = phb_element[cur_index].next_rcd;
    phb_element[next_index].prev_rcd = phb_element[cur_index].prev_rcd;
  }
}

/*
+------------------------------------------------------------------------+
| PROJECT : MMI-Framework (8417)         MODULE: PHB                     |
| STATE   : code                         ROUTINE : pb_l_rcd_chain        |
+------------------------------------------------------------------------+

  PURPOSE : Chain the element according to record number.

*/

void pb_l_rcd_chain(UBYTE type,
                    SHORT prev_index,
                    SHORT cur_index,
                    SHORT next_index)
{
  /* TRACE_FUNCTION ("pb_l_rcd_chain()"); */

  if ((phb_l_element[cur_index].prev_rcd EQ UNUSED_BYTE_INDEX)
    AND (phb_l_element[cur_index].next_rcd EQ UNUSED_BYTE_INDEX))
  {
    phb_ctb[type].first_rcd = UNUSED_INDEX;
  }
  else if ((phb_l_element[cur_index].prev_rcd NEQ UNUSED_BYTE_INDEX)
    AND (phb_l_element[cur_index].next_rcd EQ UNUSED_BYTE_INDEX))
  {
    phb_l_element[prev_index].next_rcd = UNUSED_BYTE_INDEX;
    phb_l_element[cur_index].prev_rcd = UNUSED_BYTE_INDEX;
  }
  else if ((phb_l_element[cur_index].prev_rcd EQ UNUSED_BYTE_INDEX)
    AND (phb_l_element[cur_index].next_rcd NEQ UNUSED_BYTE_INDEX))
  {
    phb_l_element[next_index].prev_rcd  = UNUSED_BYTE_INDEX;
    phb_l_element[cur_index].next_rcd = UNUSED_BYTE_INDEX;
    phb_ctb[type].first_rcd = (UBYTE)next_index;
  }
  else if ((phb_l_element[cur_index].prev_rcd NEQ UNUSED_BYTE_INDEX)
    AND (phb_l_element[cur_index].next_rcd NEQ UNUSED_BYTE_INDEX))
  {
    phb_l_element[prev_index].next_rcd = phb_l_element[cur_index].next_rcd;
    phb_l_element[next_index].prev_rcd = phb_l_element[cur_index].prev_rcd;
    phb_l_element[cur_index].prev_rcd = UNUSED_BYTE_INDEX;
    phb_l_element[cur_index].prev_rcd = UNUSED_BYTE_INDEX;
  }
}


/*
+------------------------------------------------------------------------+
| PROJECT : MMI-Framework (8417)         MODULE: PHB                     |
| STATE   : code                         ROUTINE : pb_name_chain         |
+------------------------------------------------------------------------+

  PURPOSE : Chain the element according to alpha string.

*/

void pb_name_chain(UBYTE type,
                   SHORT prev_index,
                   SHORT cur_index,
                   SHORT next_index)
{
  SHORT ref_index;
  UBYTE flag;

  /*  TRACE_FUNCTION ("pb_name_chain()");*/

  if ((phb_element[cur_index].prev_trcd EQ UNUSED_INDEX)
    AND (phb_element[cur_index].next_trcd EQ UNUSED_INDEX))
  {
    phb_ctb[type].first_trcd = UNUSED_INDEX;
  }
  else  if ((phb_element[cur_index].prev_trcd NEQ UNUSED_INDEX)
    AND (phb_element[cur_index].next_trcd EQ UNUSED_INDEX))
  {
    phb_element[prev_index].next_trcd = UNUSED_INDEX;
  }
  else if ((phb_element[cur_index].prev_trcd EQ UNUSED_INDEX)
    AND (phb_element[cur_index].next_trcd NEQ UNUSED_INDEX))
  {
    phb_element[next_index].prev_trcd  = UNUSED_INDEX;
    phb_ctb[type].first_trcd = next_index;
    phb_element[cur_index].next_trcd = UNUSED_INDEX;
  }
  else if ((phb_element[cur_index].prev_trcd NEQ UNUSED_INDEX)
    AND (phb_element[cur_index].next_trcd NEQ UNUSED_INDEX))
  {
    phb_element[prev_index].next_trcd = phb_element[cur_index].next_trcd;
    phb_element[next_index].prev_trcd = phb_element[cur_index].prev_trcd;
  }

  if (phb_ctb[type].first_trcd EQ cur_index)
  {
    flag = 0;

    ref_index = phb_element[cur_index].next_trcd;
    while (phb_element[ref_index].next_trcd)
    {
      if (!flag)
      {
        phb_ctb[type].first_trcd = ref_index;
        flag = 1;
      }
      ref_index = phb_element[ref_index].next_trcd;
    }

    if (!phb_element[ref_index].next_trcd AND !flag)
      phb_ctb[type].first_trcd = UNUSED_INDEX;
  }
}

/*
+------------------------------------------------------------------------+
| PROJECT : MMI-Framework (8417)         MODULE: PHB                     |
| STATE   : code                         ROUTINE : pb_num_chain          |
+------------------------------------------------------------------------+

  PURPOSE : Chain the element according to phone number.

*/

void pb_num_chain(UBYTE type,
                  SHORT prev_index,
                  SHORT cur_index,
                  SHORT next_index)
{
  SHORT ref_index;
  UBYTE flag;

  /*  TRACE_FUNCTION ("pb_num_chain()");*/

  if ((phb_element[cur_index].prev_nrcd EQ UNUSED_INDEX)
    AND (phb_element[cur_index].next_nrcd EQ UNUSED_INDEX))
  {
    phb_ctb[type].first_nrcd = UNUSED_INDEX;
  }
  else if ((phb_element[cur_index].prev_nrcd NEQ UNUSED_INDEX)
    AND (phb_element[cur_index].next_nrcd EQ UNUSED_INDEX))
  {
    phb_element[prev_index].next_nrcd = UNUSED_INDEX;
  }
  else if ((phb_element[cur_index].prev_nrcd EQ UNUSED_INDEX)
    AND (phb_element[cur_index].next_nrcd NEQ UNUSED_INDEX))
  {
    phb_element[next_index].prev_nrcd  = UNUSED_INDEX;
    phb_ctb[type].first_nrcd = next_index;
  }
  else if ((phb_element[cur_index].prev_nrcd NEQ UNUSED_INDEX)
    AND (phb_element[cur_index].next_nrcd NEQ UNUSED_INDEX))
  {
    phb_element[prev_index].next_nrcd = phb_element[cur_index].next_nrcd;
    phb_element[next_index].prev_nrcd = phb_element[cur_index].prev_nrcd;
  }

  if (phb_ctb[type].first_nrcd EQ cur_index)
  {
    flag = 0;

    ref_index = phb_element[cur_index].next_nrcd;
    while (phb_element[ref_index].next_nrcd)
    {
      if (!flag)
      {
        phb_ctb[type].first_nrcd = ref_index;
        flag = 1;
      }
      ref_index = phb_element[ref_index].next_nrcd;
    }

    if (!phb_element[ref_index].next_nrcd AND !flag)
      phb_ctb[type].first_nrcd = UNUSED_INDEX;
  }
}


/*
+------------------------------------------------------------------------+
| PROJECT : MMI-Framework (8417)         MODULE: PHB                     |
| STATE   : code                         ROUTINE : pb_mname_chain        |
+------------------------------------------------------------------------+

  PURPOSE : Chain the element according to merged alpha string.

*/

void pb_mname_chain(UBYTE type,
                    SHORT prev_index,
                    SHORT cur_index,
                    SHORT next_index)
{
  SHORT ref_index;
  UBYTE flag;

  /*  TRACE_FUNCTION ("pb_mname_chain()");*/

  if ((phb_element[cur_index].prev_mtrcd EQ UNUSED_INDEX)
    AND (phb_element[cur_index].next_mtrcd EQ UNUSED_INDEX))
  {
    phb_ctb[type].first_mtrcd = UNUSED_INDEX;
  }
  else if ((phb_element[cur_index].prev_mtrcd NEQ UNUSED_INDEX)
    AND (phb_element[cur_index].next_mtrcd EQ UNUSED_INDEX))
  {
    phb_element[prev_index].next_mtrcd = UNUSED_INDEX;
  }
  else if ((phb_element[cur_index].prev_mtrcd EQ UNUSED_INDEX)
    AND (phb_element[cur_index].next_mtrcd NEQ UNUSED_INDEX))
  {
    phb_element[next_index].prev_mtrcd  = UNUSED_INDEX;
    if (phb_ctb[ADN].used_rcd != 0)
      phb_ctb[ADN].first_mtrcd = next_index;
    else
      phb_ctb[FDN].first_mtrcd = next_index;
  }
  else if ((phb_element[cur_index].prev_mtrcd NEQ UNUSED_INDEX)
    AND (phb_element[cur_index].next_mtrcd NEQ UNUSED_INDEX))
  {
    phb_element[prev_index].next_mtrcd = phb_element[cur_index].next_mtrcd;
    phb_element[next_index].prev_mtrcd = phb_element[cur_index].prev_mtrcd;
  }

  if ((phb_ctb[ADN].first_mtrcd EQ cur_index)
    OR (phb_ctb[FDN].first_mtrcd EQ cur_index))
  {
    flag = 0;

    ref_index = phb_element[cur_index].next_mtrcd;
    while (phb_element[ref_index].next_mtrcd)
    {
      if (!flag)
      {
        if (phb_ctb[ADN].used_rcd != 0)
          phb_ctb[ADN].first_mtrcd = ref_index;
        else
          phb_ctb[FDN].first_mtrcd = ref_index;
        flag = 1;
      }
      ref_index = phb_element[ref_index].next_mtrcd;
    }

    if (!phb_element[ref_index].next_mtrcd AND !flag)
    {
      phb_ctb[ADN].first_mtrcd = UNUSED_INDEX;
      phb_ctb[FDN].first_mtrcd = UNUSED_INDEX;
    }
  }
}

/*
+------------------------------------------------------------------------+
| PROJECT : MMI-Framework (8417)         MODULE: PHB                     |
| STATE   : code                         ROUTINE : pb_mnum_chain         |
+------------------------------------------------------------------------+

  PURPOSE : Chain the element according to phone number.

*/

void pb_mnum_chain(UBYTE type,
                   SHORT prev_index,
                   SHORT cur_index,
                   SHORT next_index)
{
  SHORT ref_index;
  UBYTE flag;

  /*  TRACE_FUNCTION ("pb_mnum_chain()");*/

  if ((phb_element[cur_index].prev_mnrcd EQ UNUSED_INDEX)
    AND (phb_element[cur_index].next_mnrcd EQ UNUSED_INDEX))
  {
    phb_ctb[ADN].first_mnrcd = UNUSED_INDEX;
    phb_ctb[FDN].first_mnrcd = UNUSED_INDEX;
  }
  else if ((phb_element[cur_index].prev_mnrcd NEQ UNUSED_INDEX)
    AND (phb_element[cur_index].next_mnrcd EQ UNUSED_INDEX))
  {
    phb_element[prev_index].next_mnrcd = UNUSED_INDEX;
  }
  else if ((phb_element[cur_index].prev_mnrcd EQ UNUSED_INDEX)
    AND (phb_element[cur_index].next_mnrcd NEQ UNUSED_INDEX))
  {
    phb_element[next_index].prev_mnrcd  = UNUSED_INDEX;
    if (phb_ctb[ADN].used_rcd != 0)
      phb_ctb[ADN].first_mnrcd = next_index;
    else
      phb_ctb[FDN].first_mnrcd = next_index;
  }
  else if ((phb_element[cur_index].prev_mnrcd NEQ UNUSED_INDEX)
    AND (phb_element[cur_index].next_mnrcd NEQ UNUSED_INDEX))
  {
    phb_element[prev_index].next_mnrcd = phb_element[cur_index].next_mnrcd;
    phb_element[next_index].prev_mnrcd = phb_element[cur_index].prev_mnrcd;
  }

  if ((phb_ctb[ADN].first_mnrcd EQ cur_index)
    OR (phb_ctb[FDN].first_mnrcd EQ cur_index))
  {
    flag = 0;

    ref_index = phb_element[cur_index].next_mnrcd;
    while (phb_element[ref_index].next_mnrcd)
    {
      if (!flag)
      {
        if (phb_ctb[ADN].used_rcd != 0)
          phb_ctb[ADN].first_mnrcd = ref_index;
        else
          phb_ctb[FDN].first_mnrcd = ref_index;
        flag = 1;
      }
      ref_index = phb_element[ref_index].next_mnrcd;
    }

    if (!phb_element[ref_index].next_mnrcd AND !flag)
    {
      phb_ctb[ADN].first_mnrcd = UNUSED_INDEX;
      phb_ctb[FDN].first_mnrcd = UNUSED_INDEX;
    }
  }
}

/*
+------------------------------------------------------------------------+
| PROJECT : MMI-Framework (8417)         MODULE: PHB                     |
| STATE   : code                         ROUTINE : pb_read_status        |
+------------------------------------------------------------------------+

  PURPOSE :

*/

T_PHB_RETURN pb_read_status(UBYTE type, UBYTE *service,
                            SHORT *max_rcd, SHORT *used_rcd,
                            UBYTE *tag_len, SHORT *avail_rcd,
                            SHORT *max_ext, SHORT *used_ext)
{
  SHORT i;
  SHORT count;
  UBYTE ext_type; 

  TRACE_FUNCTION ("pb_read_status()");

  TRACE_EVENT_P1("PHB get status of phonebook: %d", type);
  
  /* check whether this phonebook exists */
  if (phb_ctb[type].mem EQ NO_PHB_ENTRY)
  {
    TRACE_EVENT("Phonebook is empty: return PHB_FAIL");
    return PHB_FAIL;
  }
  *service = phb_ctb[type].service;
  *used_rcd = phb_ctb[type].used_rcd;
  /* ACI-SPR-9421 (mdf): reinstalled after storage increasement for phonebook */
  *max_rcd = phb_ctb[type].max_rcd;
  *tag_len = MINIMUM ( phb_ctb[type].alpha_len, PHB_MAX_TAG_LEN );

  *max_ext = 0;
  *used_ext = 0;


  count = 0;
  switch (type)
  {
  case ECC:
  case ADN:
  case FDN:
  case SDN:
  case BDN:
  case ADN_FDN:
  case UPN:
    for (i=0; i<MAX_AFB_RECORDS; i++)
    {
      if (phb_element[i].free EQ PHB_ELEMENT_FREE)
        count++;
    }
    TRACE_EVENT_P1("free records from count=%d",count);

    if ((phb_ctb[type].max_rcd-phb_ctb[type].used_rcd) >=count)
    {
      /* avail_rcd should be equal to free records not used records!!! */
      *avail_rcd = count;
    }
    else
    {
      *avail_rcd = phb_ctb[type].max_rcd - phb_ctb[type].used_rcd;
    }
    break;
  case LDN:
  case LRN:
  case LMN:
    *max_rcd = phb_ctb[type].max_rcd;
    *avail_rcd = phb_ctb[type].max_rcd - phb_ctb[type].used_rcd;
    break;
  }
  
#ifdef PHONEBOOK_EXTENSION
   ext_type = pb_get_ext_type(type); 

   if(ext_type NEQ 0xFF)
   {
   pb_read_ext_status(ext_type, max_ext, used_ext);
    }
#endif

  return PHB_OK;
}


/*
+------------------------------------------------------------------+
| PROJECT : MMI-Framework (8417)         MODULE: PHB               |
| STATE   : code                         ROUTINE : pb_get_fdn_mode |
+------------------------------------------------------------------+

  PURPOSE :

*/
void pb_status_req(UBYTE *mode)
{
  /*  TRACE_FUNCTION ("pb_status_req()");*/

  *mode = phb_stat;
}


/*
+------------------------------------------------------------------+
| PROJECT : MMI-Framework (8417)         MODULE: PHB               |
| STATE   : code                         ROUTINE : pb_first_free   |
+------------------------------------------------------------------+

  PURPOSE : On exit, "first_free" contains the index of the first free 
            location in the specified phonebook.

*/
T_PHB_RETURN pb_first_free(
  UBYTE type,
  SHORT *first_free)
{
  SHORT i,bit;
  UBYTE max_bitmap;
  UBYTE pos;

  TRACE_FUNCTION ("pb_first_free()");

  if (first_free EQ NULL)
  {
    ACI_ERR_DESC(ACI_ERR_CLASS_Cme,CME_ERR_Unknown);
    return(PHB_FAIL);
  }

  switch (type)
  {
    case SDN:
    case ADN:
    case FDN:
    case BDN:
    case UPN:
    case ECC:
      switch (type)
      {
        case ADN:   max_bitmap=MAX_ADN_BITMAP;    break;
        case FDN:   max_bitmap=MAX_FDN_BITMAP;    break;
        case BDN:   max_bitmap=MAX_BDN_BITMAP;    break;
        case UPN:   max_bitmap=MAX_UPN_BITMAP;    break;
        case SDN:   max_bitmap=MAX_SDN_BITMAP;    break;
        case ECC:   max_bitmap=MAX_ECC_BITMAP;    break;

        default:
          ACI_ERR_DESC(ACI_ERR_CLASS_Cme,CME_ERR_Unknown);
          return(PHB_FAIL);
      }

      bit = 0;
      for (i=0; i<max_bitmap; i++)
      {
        pos = 0;
        while ((phb_ctb[type].rcd_bitmap[i] & (1<<pos)))
        {
          bit++;
          pos++;
        }

        if ((bit%8) OR !pos)
        {
          if (bit>=phb_ctb[type].max_rcd)
          {
            *first_free=0;
            return(PHB_FULL);
          }

          *first_free=bit+1;
          return(PHB_OK);
        }
      }

      *first_free=0;
      return(PHB_FULL);

    case LDN:
    case LMN:
    case LRN:
      /*
      *   It is not possible to specify an index when writing to these
      *   phonebooks. Whenever a number is added, it automatically goes
      *   in the first entry of the list, hence it could be said that
      *   the first free entry is always 1.
      */
      *first_free=1;
      return(PHB_OK);

    default:
      break;
  }

  ACI_ERR_DESC(ACI_ERR_CLASS_Cme,CME_ERR_Unknown);
  return(PHB_FAIL);
}


/*
+--------------------------------------------------------------------+
| PROJECT : MMI-Framework (8417)         MODULE: PHB                 |
| STATE   : code                         ROUTINE : pb_switch_adn_fdn |
+--------------------------------------------------------------------+

  PURPOSE :

*/

T_PHB_RETURN pb_switch_adn_fdn(UBYTE mode, T_ACI_CLASS aci_classFDN)
{
  UBYTE classFDN = (UBYTE) CLASS_None;
  TRACE_FUNCTION("pb_switch_adn_fdn()");

  pb_set_fdn_input_classtype (aci_classFDN);

  if ((mode != FDN_DISABLE) AND (mode != FDN_ENABLE))
    return PHB_FAIL;  /* never used */

  fdn_mode = mode;

  if ( fdn_classtype NEQ  fdn_input_classtype)
  {
      fdn_classtype = fdn_input_classtype;
      classFDN = fdn_input_classtype;
                /* write to ffs */
#ifndef _SIMULATION_
      if( ffs_fwrite("/mmi/fdnClassType", &classFDN, sizeof(classFDN)) < 1)
      {
          TRACE_FUNCTION("sAT_PlusCLCK: ME- failed to write ffs");
      }
#endif
  }
  
  pb_init_afb();
  pb_start_build(FALSE);
  return PHB_OK;  /* never used */
}


/*
+--------------------------------------------------------------------+
| PROJECT : MMI-Framework (8417)        MODULE  : PHB                |
| STATE   : code                        ROUTINE : cmpWild            |
+--------------------------------------------------------------------+

  PURPOSE : compare two strings with wild character ('?') recognition
  in string1. Returns 0 on mismatch, otherwise 1. An empty
  string always match.

*/

UBYTE cmpWild (char *s1, char *s2)
{
  int i1, i2;

  i1 = strlen(s1);
  i2 = strlen(s2);

  if ( i1 != i2 )
    return 0;

  while (i1 AND i2)
  {
    i1--;
    i2--;
    if (s1[i1] != s2[i2] AND s1[i1] != '?')
      return 0;
  }

  return 1;
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : PHB                |
| STATE   : code                        ROUTINE : cmpPHBString       |
+--------------------------------------------------------------------+

  PURPOSE : get the length for PHB entry
*/
int pb_get_entry_len ( const UBYTE *pb_tag, UBYTE max_pb_len )
{
  int   pb_len    = 0;
  UBYTE inc_count = 1;
  BOOL  ucs2      = FALSE;
  UBYTE chars = 0;

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
| PROJECT : MMI-Framework (8417)        MODULE  : PHB                |
| STATE   : code                        ROUTINE : pb_cmp_phb_entry   |
+--------------------------------------------------------------------+

  PURPOSE : compare PHB entry (SIM) with T_ACI_PB_TEXT.
            cannot compare pb_tag with search_tag return - 1
            if pb_tag < search_tag return value < 0
            if pb_tag = search_tag return value = 0
            if pb_tag > search_tag return value > 0
*/
static int pb_cmp_phb_entry ( UBYTE          *pb_tag,
                       UBYTE          pb_len,
                       T_ACI_PB_TEXT  *search_tag )
{
  UBYTE cmp_len;
  UBYTE pb_tag_buf[PHB_MAX_TAG_LEN];
  UBYTE *cmp_pb_tag;
  UBYTE search_tag_buf[PHB_MAX_TAG_LEN];
  UBYTE *cmp_search_tag;
  int   i;

  cmp_len = MINIMUM ( pb_len, search_tag->len );

  /* convert to lower cases, when not Unicode */
  if ( ( search_tag->cs EQ CS_Sim ) AND
       ( ( *search_tag->data NEQ 0x80 ) AND ( *pb_tag NEQ 0x80 ) ) )
  {
    for (i = 0; i < cmp_len; i++)
      pb_tag_buf[i] = (UBYTE)tolower((int)pb_tag[i]);
    cmp_pb_tag = pb_tag_buf;

    for (i = 0; i < cmp_len; i++)
      search_tag_buf[i] = (UBYTE)tolower((int)search_tag->data[i]);
    cmp_search_tag = search_tag_buf;
  }
  else
  {
    cmp_pb_tag = pb_tag;
    cmp_search_tag = search_tag->data;
  }
  /* check the types */
  if ( search_tag->cs EQ CS_Sim )
    return ( cmpString ( cmp_pb_tag, cmp_search_tag, cmp_len ) );

  if ( ( search_tag->cs EQ CS_Ucs2 ) AND ( *pb_tag EQ 0x80 ) )
    return ( memcmp ( cmp_pb_tag + 1, cmp_search_tag, cmp_len ) );

  if ( ( search_tag->cs EQ CS_Ucs2 ) AND ( *pb_tag NEQ 0x80 ) )
    return pb_cmp2Bytes ( cmp_search_tag, cmp_pb_tag, cmp_len, 1 );

  if ( ( search_tag->cs EQ CS_GsmDef ) AND ( *pb_tag EQ 0x80 ) )
    return pb_cmp2Bytes ( cmp_search_tag, cmp_pb_tag + 1 , cmp_len, 2 );

  if ( ( search_tag->cs EQ CS_GsmDef ) AND ( *pb_tag NEQ 0x80 ) )
  {
    return ( memcmp ( cmp_pb_tag, cmp_search_tag, cmp_len ) );
  }

  return ( -1 );
}


/*
+--------------------------------------------------------------------+
| PROJECT : MMI-Framework (8417)        MODULE  : PHB                |
| STATE   : code                        ROUTINE : cmpString          |
+--------------------------------------------------------------------+

  PURPOSE : compare two strings.
            if s1 < s2 return value < 0
            if s1 = s2 return value = 0
            if s1 > s2 return value > 0
*/

static int cmpString ( UBYTE *s1, UBYTE *s2, UBYTE len )
{
  int n = 0;

  /* TRACE_FUNCTION("cmpString()"); */ /* Called too often to trace */

  if ((*s1 EQ 0x80) AND
      (*s2 NEQ 0x80)    )
  {
    s1++;
    len--;
    return pb_cmp2Bytes(s1, s2, len, 1);
  }
  else if ((*s1 NEQ 0x80) AND
           (*s2 EQ 0x80)     )
  {
    s2++;
    len--;
    return pb_cmp2Bytes(s1, s2, len, 2);
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
| STATE   : code                        ROUTINE : pb_cmp2Bytes       |
+--------------------------------------------------------------------+

  PURPOSE : compare two strings.
            if s1 < s2 return value < 0
            if s1 = s2 return value = 0
            if s1 > s2 return value > 0

            flag = 1, s1 is unicode
            flag = 2, s2 is unicode
*/

static int pb_cmp2Bytes(UBYTE *s1, UBYTE *s2, UBYTE len, UBYTE flag)
{
  int n = 0;

  /*  TRACE_FUNCTION("pb_cmp2Bytes()"); */

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
+--------------------------------------------------------------------+
| PROJECT : MMI-Framework (8417)        MODULE  : PHB                |
| STATE   : code                        ROUTINE : pb_check_number    |
+--------------------------------------------------------------------+

  PURPOSE : compare two numbers. If they are same, return 1.

*/

UBYTE pb_check_number(char *cur_number, char *number)
{
  UBYTE        len1;
  UBYTE        len2;

  len1 = strlen(cur_number);
  len2 = strlen((char *)number);

  if (!len2)
    return 0;                   /* Bug Fix: the return value was "1" */

  if ((len1>=6) AND (len2>=6))
  {
    if (cmpWild (&cur_number[len1-6], (char *)&number[len2-6]) EQ 1)
      return 1;
  }
  else
  {
    if (cmpWild ((char *)cur_number, (char *)number) EQ 1)
      return 1;
  }
  return 0;
}


/*
+--------------------------------------------------------------------+
| PROJECT : MMI-Framework (8417)        MODULE  : PHB                |
| STATE   : code                        ROUTINE : pb_check_fdn       |
+--------------------------------------------------------------------+

  PURPOSE : check whether phone number is in FDN phonebook.

*/
GLOBAL T_PHB_RETURN pb_check_fdn (UBYTE toa,
                                  const UBYTE *number)
{
  SHORT        cur_index;
  CHAR         cur_number[MAX_PHB_NUM_LEN];
  CHAR         new_number[MAX_PHB_NUM_LEN];
  UBYTE        len1;
  UBYTE        len2;

  len1 = strlen((char *)number);

  cur_index = phb_ctb[FDN].first_nrcd;
  while (cur_index != UNUSED_INDEX)
  {
    memset(new_number, 0, sizeof(new_number));
    cmhPHB_getAdrStr(cur_number,
      MAX_PHB_NUM_LEN - 1,
      phb_element[cur_index].entry.number,
      phb_element[cur_index].entry.len);

    len2 = strlen((char *)cur_number);
    if (len1 < len2 OR phb_element[cur_index].entry.len EQ 0)
    {
      cur_index = phb_element[cur_index].next_nrcd;
      continue;
    }
    else
    {
      strncpy(new_number, (char *)number, len2);
      if (cmpWild ((char *)&cur_number, new_number) EQ 1)
      {

        if ( toa NEQ 0 ) /* ACI-SPR-11927:check whether to test toa or not */ 
        {
          if ((toa NEQ phb_element[cur_index].entry.ton_npi) AND
              (phb_element[cur_index].entry.ton_npi NEQ 0xFF)    ) /* VO patch 02.03.01 */  
          {
            cur_index = phb_element[cur_index].next_nrcd;
            continue;
          }
        }
        
        causeMod  = P_CEER_mod;      /* Clear module which was set for ceer */
        causeCeer = CEER_NotPresent; /* Reset cause - number to dial found in FDN list. */
        return PHB_OK;
      }
    }

    cur_index = phb_element[cur_index].next_nrcd;
  }
  causeMod  = P_CEER_sim;      /* Set ceer module to sim */
  causeCeer = P_CEER_InvalidFDN;   /* Set cause - number to dial not found in FDN list.*/
  return PHB_FAIL;
}

/*
+------------------------------------------------- -------------+
| PROJECT : MMI-Framework (8417)         MODULE  : PHB          |
| STATE   : code                         ROUTINE : pb_exit      |
+---------------------------------------------------------------+

  PURPOSE : Save the phonebook in SIM card.

*/

void pb_exit()
{
  TRACE_FUNCTION("pb_exit()");

  pb_write_eeprom();
  if(pb_init_sync_sim(LDN) EQ FALSE)
    pb_finish_sync_sim();     /* this will otherwise be called after the last record written*/
}


/*
+------------------------------------------------------------------+
| PROJECT : MMI-Framework (8417)         MODULE  : PHB             |
| STATE   : code                         ROUTINE : pb_write_sim_cb |
+------------------------------------------------------------------+


    PURPOSE :   Call back for write phonebook in SIM card.

*/

void pb_write_sim_cb(SHORT table_id)
{
  UBYTE type;

  TRACE_FUNCTION("pb_write_sim_cb()");

  simShrdPrm.atb[table_id].ntryUsdFlg = FALSE;

  if (simShrdPrm.atb[table_id].errCode NEQ SIM_NO_ERROR)
  {
    TRACE_ERROR("pb_write_sim_cb(): error for writing");
    db_index = UNUSED_INDEX;
    phb_stat = PHB_READY;
    cmhPHB_StatIndication( PHB_WRITE_FAIL,
                           (SHORT)cmhSIM_GetCmeFromSim( simShrdPrm.atb[table_id].errCode ),
                           FALSE ); /* don't indicate for write */
    return;
  }
  if (db_index NEQ UNUSED_INDEX)
  {
    switch (simShrdPrm.atb[table_id].reqDataFld)
    {
    case SIM_ADN:
      type = ADN;
      break;
    case SIM_FDN:
      type = FDN;
      break;
    case SIM_BDN:
      type = BDN;
      break;
    case SIM_SDN:
      type = SDN;
      break;
    case SIM_MSISDN:
      type = UPN;
      break;
    default:
      return;
    }
    pb_delete_sim_book(type);
  }
  else
  {
    phb_stat = PHB_READY;
    cmhPHB_StatIndication ( PHB_READY, CME_ERR_NotPresent, FALSE );
  }
}

/*
+-----------------------------------------------------------------+
| PROJECT : MMI-Framework (8417)         MODULE  : PHB            |
| STATE   : code                         ROUTINE : pb_write_sim   |
+-----------------------------------------------------------------+


    PURPOSE :   Write phonebook in SIM card.

*/

T_PHB_RETURN pb_write_sim(UBYTE type, UBYTE rcd_num)
{
  SHORT           table_id;
  USHORT          data_id;

  TRACE_FUNCTION("pb_write_sim()");

  switch (type)
  {
  case ADN:
    data_id = SIM_ADN;
    break;
  case FDN:
    data_id = SIM_FDN;
    break;
  case BDN:
    data_id = SIM_BDN;
    break;
  case SDN:
    data_id = SIM_SDN;
    break;
   case UPN:
    data_id = SIM_MSISDN;
    break;
   case LDN:
    data_id = SIM_LND;
    break;
  default:
    return PHB_FAIL;
  }

  table_id = psaSIM_atbNewEntry();
  if(table_id EQ NO_ENTRY)
  {
    TRACE_ERROR ("pb_write_sim(): no more table entries");
    return (PHB_FAIL);
  }

  simShrdPrm.atb[table_id].ntryUsdFlg = TRUE;
  simShrdPrm.atb[table_id].accType    = ACT_WR_REC;
  simShrdPrm.atb[table_id].v_path_info  = FALSE;
  simShrdPrm.atb[table_id].reqDataFld = data_id;
  simShrdPrm.atb[table_id].recNr      = rcd_num;
  simShrdPrm.atb[table_id].dataLen    = phb_ctb[type].alpha_len + 14;
  simShrdPrm.atb[table_id].exchData   = data;
  simShrdPrm.atb[table_id].rplyCB     = pb_write_sim_cb;

  simShrdPrm.aId = table_id;

  if(psaSIM_AccessSIMData() < 0)
  {
    TRACE_EVENT("pb_write_sim(): psaSIM_AccessSIMData() failed");
    return (PHB_FAIL);
  }
  
  phb_stat = PHB_BUSY;
  cmhPHB_StatIndication ( PHB_BUSY, CME_ERR_NotPresent, FALSE );

  return (PHB_EXCT);
}



#ifdef PHONEBOOK_EXTENSION
/*
+----------------------------------------------------------------------+
| PROJECT :                              MODULE  : PHB                 |
| STATE   : code                         ROUTINE : pb_prepare_ext_data |
+----------------------------------------------------------------------+


    PURPOSE :   Prepare the data for the extention record.
                If NULL pointer is given for number and subaddress
                then the extention record will marked as unused

*/


LOCAL void pb_prepare_ext_data(UBYTE *number, UBYTE no_len,
                               UBYTE *subaddr, UBYTE sub_len,
                               USHORT file_id)
{
  memset(data, 0xFF, sizeof(data));

  if ((subaddr != NULL) AND (*subaddr != 0xFF))
  {
    TRACE_EVENT ("SUBADDRESS EXTENTION");
    data[0] = 1;
    data[1] = sub_len;
    memcpy (data + 2, subaddr, 11);
  }
  else if (number != NULL)
  {
    data[0] = 2;
    data[1] = no_len - 10;
    memcpy (data + 2, number + 10, no_len);
  }
}


/*
+----------------------------------------------------------------------+
| PROJECT : MMI-Framework (8417)         MODULE  : PHB                 |
| STATE   : code                         ROUTINE : pb_write_sim_ext_cb |
+----------------------------------------------------------------------+


    PURPOSE :   Call back for write phonebook in SIM card.

*/

void pb_write_sim_ext_cb(SHORT table_id)
{
   /*  USHORT ext_type;
    UBYTE  rcd_num;*/

  TRACE_FUNCTION("pb_write_sim_ext_cb");

  simShrdPrm.atb[table_id].ntryUsdFlg = FALSE;

  if (simShrdPrm.atb[table_id].errCode NEQ SIM_NO_ERROR)
  {
    TRACE_ERROR("pb_write_sim_ext_cb (): FATAL ERROR");
  }
}


/*
+-------------------------------------------------------------------+
| PROJECT : MMI-Framework (8417)         MODULE  : PHB              |
| STATE   : code                         ROUTINE : pb_write_sim_ext |
+-------------------------------------------------------------------+


    PURPOSE :   Write phonebook in SIM card.

*/

T_PHB_RETURN pb_write_sim_ext(USHORT data_id, UBYTE rcd_num)
{
  SHORT           table_id;

  table_id = psaSIM_atbNewEntry();

  if(table_id EQ NO_ENTRY)
  {
    TRACE_ERROR ("pb_write_sim_ext(): no more table entries");
    return (PHB_FAIL);
  }

  simShrdPrm.atb[table_id].ntryUsdFlg = TRUE;
  simShrdPrm.atb[table_id].accType    = ACT_WR_REC;
  simShrdPrm.atb[table_id].v_path_info  = FALSE;
  simShrdPrm.atb[table_id].reqDataFld = data_id;
  simShrdPrm.atb[table_id].recNr      = rcd_num;
  simShrdPrm.atb[table_id].dataLen    = 13;
  simShrdPrm.atb[table_id].exchData   = data;
  simShrdPrm.atb[table_id].rplyCB     = pb_write_sim_ext_cb;

  simShrdPrm.aId = table_id;

  if (psaSIM_AccessSIMData() < 0)
  {
    TRACE_ERROR("pb_write_sim_ext(): psaSIM_AccessSIMData() failed");
    return (PHB_FAIL);
  }

  return (PHB_EXCT);
}
#endif /* PHONEBOOK_EXTENSION */

/*
+------------------------------------------------------------------+
| PROJECT : MMI-Framework (8417)     MODULE  : PHB                 |
| STATE   : code                     ROUTINE : pb_write_eepron_req |
+------------------------------------------------------------------+


    PURPOSE :   Write phonebook in EEPROM.

*/

void pb_write_eeprom()
{
  SHORT ptr_index;
  UBYTE ptr_byte_index;
  int   i;
  UBYTE count;
  EF_UPN *p = (EF_UPN *)data;

  /* TRACE_FUNCTION ("pb_write_eeprom_req()"); */

  /* Write Last Dialing Numbers */
  ptr_byte_index = (UBYTE)phb_ctb[LDN].first_rcd;

  count = 0;
  for (i=0; i<phb_ctb[LDN].max_rcd; i++)
  {
    if (count < phb_ctb[LDN].used_rcd AND ptr_byte_index NEQ UNUSED_BYTE_INDEX)
      {
        TRACE_EVENT_P1("--- LDN: copy to eeprom for ptr_byte_index %d", ptr_byte_index);
         pb_copy_ldn_record((SHORT)ptr_byte_index, 1);
      }
    else
      memset(&data[0], 0xFF, SIZE_EF_LDN);

    if (pcm_WriteRecord((UBYTE *)EF_LDN_ID,
                  (USHORT)(i+1),
                  SIZE_EF_LDN,
                  &data[0]) NEQ DRV_OK)
      break;
    if (ptr_byte_index NEQ UNUSED_BYTE_INDEX)
      ptr_byte_index = phb_l_element[ptr_byte_index].next_rcd;
    count++;
  }

  /* Write Last received Numbers */
  ptr_byte_index = (UBYTE)phb_ctb[LRN].first_rcd;

  count = 0;
  for (i=0; i<phb_ctb[LRN].max_rcd; i++)
  {
    if (count < phb_ctb[LRN].used_rcd AND ptr_byte_index NEQ UNUSED_BYTE_INDEX)
      pb_copy_lrn_record(ptr_byte_index, 1);
    else
      memset(&data[0], 0xFF, SIZE_EF_LRN);

    if (pcm_WriteRecord((UBYTE *)EF_LRN_ID,
                    (USHORT)(i+1),
                    SIZE_EF_LRN,
                    &data[0]) NEQ DRV_OK)
      break;
    if (ptr_byte_index NEQ UNUSED_BYTE_INDEX)
      ptr_byte_index = phb_l_element[ptr_byte_index].next_rcd;
    count++;
  }

  /* Write Last missed Numbers */
  ptr_byte_index = (UBYTE)phb_ctb[LMN].first_rcd;

  count = 0;
  for (i=0; i<phb_ctb[LMN].max_rcd; i++)
  {
    if (count < phb_ctb[LMN].used_rcd AND ptr_byte_index NEQ UNUSED_BYTE_INDEX)
      pb_copy_lmn_record(ptr_byte_index, 1);
    else
      memset(&data[0], 0xFF, SIZE_EF_LMN);

    if (pcm_WriteRecord((UBYTE *)EF_LMN_ID,
                    (USHORT)(i+1),
                    SIZE_EF_LMN,
                    &data[0]) NEQ DRV_OK)
      break;
    if (ptr_byte_index NEQ UNUSED_BYTE_INDEX)
      ptr_byte_index = phb_l_element[ptr_byte_index].next_rcd;
    count++;
  }
    
  /* Write user person Numbers */
  if (phb_ctb[UPN].mem EQ TE_MEMORY)
  {
    ptr_index = phb_ctb[UPN].first_rcd;

    count = 0;
    for (i=0; i<phb_ctb[UPN].max_rcd; i++)
    {
      if (count < phb_ctb[UPN].used_rcd AND ptr_index NEQ UNUSED_INDEX)
      {
        /* copy record */
        memcpy(p->alphId, 
               phb_element[ptr_index].entry.tag,
               10*sizeof(UBYTE));
        p->len = phb_element[ptr_index].entry.len;
        p->numTp = phb_element[ptr_index].entry.ton_npi;
        memcpy(p->usrNum, 
               phb_element[ptr_index].entry.number,
               10*sizeof(UBYTE));
        p->ccp = phb_element[ptr_index].entry.cc_id;
      }
      else
        memset(&data[0], 0xFF, SIZE_EF_UPN);

      if (pcm_WriteRecord((UBYTE *)EF_UPN_ID,
                      (USHORT)(i+1),
                      SIZE_EF_UPN,
                      &data[0]) NEQ DRV_OK)
        break;
      if (ptr_index NEQ UNUSED_INDEX)
        ptr_index = phb_element[ptr_index].next_rcd;
      count++;
    }
  }
}


/*
+------------------------------------------------------------------+
| PROJECT : MMI-Framework (8417)     MODULE  : PHB                 |
| STATE   : code                     ROUTINE : pb_copy_ldn_record  |
+------------------------------------------------------------------+


    PURPOSE :   Copy a LDN record.
    flag = 0: from EEPROM to local
    flag = 1: from local to EEPROM

*/

void pb_copy_ldn_record(SHORT index, UBYTE flag)
{
  EF_LDN *p = (EF_LDN *)data;

  /*  TRACE_FUNCTION ("pb_copy_ldn_record()");*/
  if (!flag)
  {
    phb_l_element[index].entry.year = p->year;
    phb_l_element[index].entry.month = p->month;
    phb_l_element[index].entry.day = p->day;
    phb_l_element[index].entry.hour = p->hour;
    phb_l_element[index].entry.minute = p->minute;
    phb_l_element[index].entry.second = p->second;
    phb_l_element[index].entry.len = p->len;
    phb_l_element[index].entry.ton_npi = p->numTp;
    memcpy((char *)phb_l_element[index].entry.number,
           (char *)&p->dldNum, 10);
    phb_l_element[index].entry.cc_id     = p->ccp;
  }
  if (flag EQ 1)
  {
    p->calDrMsb = 0xFF;
    p->calDrLsb = 0xFF;
    p->year = phb_l_element[index].entry.year;
    p->month = phb_l_element[index].entry.month;
    p->day = phb_l_element[index].entry.day;
    p->hour = phb_l_element[index].entry.hour;
    p->minute = phb_l_element[index].entry.minute;
    p->second = phb_l_element[index].entry.second;
    p->len = phb_l_element[index].entry.len;
    p->numTp = phb_l_element[index].entry.ton_npi;
    memcpy((char *)p->dldNum,
           (char *)phb_l_element[index].entry.number, 10);
    p->ccp = phb_l_element[index].entry.cc_id;
    p->ext1 = 0xFF;
  }
}


/*
+------------------------------------------------------------------+
| PROJECT : MMI-Framework (8417)     MODULE  : PHB                 |
| STATE   : code                     ROUTINE : pb_copy_lrn_record  |
+------------------------------------------------------------------+


    PURPOSE :   Copy a LRN record.
    flag = 0: from EEPROM to local
    flag = 1: from local to EEPROM

*/

void pb_copy_lrn_record(SHORT index, UBYTE flag)
{
  EF_LRN *p = (EF_LRN *)data;

  /*  TRACE_FUNCTION ("pb_copy_lrn_record()");*/

  if (!flag)
  {
    phb_l_element[index].entry.year = p->year;
    phb_l_element[index].entry.month = p->month;
    phb_l_element[index].entry.day = p->day;
    phb_l_element[index].entry.hour = p->hour;
    phb_l_element[index].entry.minute = p->minute;
    phb_l_element[index].entry.second = p->second;
    phb_l_element[index].entry.len = p->len;
    phb_l_element[index].entry.ton_npi = p->numTp;
    memcpy((char *)phb_l_element[index].entry.number,
           (char *)p->dldNum, 10);
    phb_l_element[index].entry.cc_id     = p->ccp;
  }
  if (flag EQ 1)
  {
    p->calDrMsb = 0xFF;
    p->calDrLsb = 0xFF;
    p->year = phb_l_element[index].entry.year;
    p->month = phb_l_element[index].entry.month;
    p->day = phb_l_element[index].entry.day;
    p->hour = phb_l_element[index].entry.hour;
    p->minute = phb_l_element[index].entry.minute;
    p->second = phb_l_element[index].entry.second;
    p->id = 0xFF;
    p->len = phb_l_element[index].entry.len;
    p->numTp = phb_l_element[index].entry.ton_npi;
    memcpy((char *)p->dldNum,
           (char *)phb_l_element[index].entry.number, 10);
    p->ccp = phb_l_element[index].entry.cc_id;
    p->ext1 = 0xFF;
  }
}


/*
+------------------------------------------------------------------+
| PROJECT : MMI-Framework (8417)     MODULE  : PHB                 |
| STATE   : code                     ROUTINE : pb_copy_lmn_record  |
+------------------------------------------------------------------+


    PURPOSE :   Copy a LMN record.
    flag = 0: from EEPROM to local
    flag = 1: from local to EEPROM

*/

void pb_copy_lmn_record(SHORT index, UBYTE flag)
{
  EF_LMN *p = (EF_LMN *)data;

  /* TRACE_FUNCTION ("pb_copy_lmn_record()");*/

  if (!flag)
  {
    phb_l_element[index].entry.year = p->year;
    phb_l_element[index].entry.month = p->month;
    phb_l_element[index].entry.day = p->day;
    phb_l_element[index].entry.hour = p->hour;
    phb_l_element[index].entry.minute = p->minute;
    phb_l_element[index].entry.second = p->second;
    phb_l_element[index].entry.len = p->len;
    phb_l_element[index].entry.ton_npi = p->numTp;
    memcpy((char *)phb_l_element[index].entry.number,
           (char *)p->dldNum, 10);
    phb_l_element[index].entry.cc_id     = p->ccp;
  }
  if (flag EQ 1)
  {
    p->year = phb_l_element[index].entry.year;
    p->month = phb_l_element[index].entry.month;
    p->day = phb_l_element[index].entry.day;
    p->hour = phb_l_element[index].entry.hour;
    p->minute = phb_l_element[index].entry.minute;
    p->second = phb_l_element[index].entry.second;
    p->id = 0xFF;
    p->len = phb_l_element[index].entry.len;
    p->numTp = phb_l_element[index].entry.ton_npi;
    memcpy((char *)p->dldNum,
           (char *)phb_l_element[index].entry.number, 10);
    p->ccp = phb_l_element[index].entry.cc_id;
    p->ext1 = 0xFF;
  }
}

/*
+------------------------------------------------------------------------+
| PROJECT : MMI-Framework (8417)         MODULE: PHB                     |
| STATE   : code                         ROUTINE : pb_delete_book        |
+------------------------------------------------------------------------+

  PURPOSE : Delete a phonebook in SIM card.

*/

T_PHB_RETURN pb_delete_book(UBYTE book)
{
  SHORT cur_index;

  TRACE_FUNCTION("pb_delete_book()");

  /* check whether this phonebook exists */
  if (phb_ctb[book].mem EQ NO_PHB_ENTRY)
    return PHB_FAIL;

  phb_stat = PHB_BUSY;
  cmhPHB_StatIndication ( PHB_BUSY, CME_ERR_NotPresent, FALSE );

  cur_index = phb_ctb[book].first_rcd;
  while (cur_index != UNUSED_INDEX)
  {
    phb_element[cur_index].free = PHB_ELEMENT_FREE;
    if ((book EQ ADN) OR (book EQ FDN))
    {
      pb_mname_chain(book,
        phb_element[cur_index].prev_mtrcd,
        cur_index,
        phb_element[cur_index].next_mtrcd);
      pb_mnum_chain(book,
        phb_element[cur_index].prev_mnrcd,
        cur_index,
        phb_element[cur_index].next_mnrcd);
    }
    cur_index = phb_element[cur_index].next_rcd;
  }
  phb_ctb[book].used_rcd    = 0;
  /*  phb_ctb[book].first_rcd   = UNUSED_INDEX;*/
  phb_ctb[book].first_trcd  = UNUSED_INDEX;
  phb_ctb[book].first_nrcd  = UNUSED_INDEX;
  phb_ctb[book].first_mtrcd = UNUSED_INDEX; /* ??? */
  phb_ctb[book].first_mnrcd = UNUSED_INDEX; /* ??? */
  switch (book)
  {
  case ADN:
    memset(phb_ctb[book].rcd_bitmap, 0, MAX_ADN_BITMAP);
    break;
  case FDN:
    memset(phb_ctb[book].rcd_bitmap, 0, MAX_FDN_BITMAP);
    break;
  case BDN:
    memset(phb_ctb[book].rcd_bitmap, 0, MAX_BDN_BITMAP);
    break;
  case SDN:
    memset(phb_ctb[book].rcd_bitmap, 0, MAX_SDN_BITMAP);
    break;
  case UPN:
    memset(phb_ctb[book].rcd_bitmap, 0, MAX_UPN_BITMAP);
    break;
  default:
    break;
  }
  pb_delete_sim_book(book);
  return PHB_OK;
}

/*
+------------------------------------------------------------------------+
| PROJECT : MMI-Framework (8417)         MODULE: PHB                     |
| STATE   : code                         ROUTINE : pb_delete_sim_book    |
+------------------------------------------------------------------------+

  PURPOSE : Delete a phonebook in SIM card.

*/

void pb_delete_sim_book(UBYTE book)
{
  if (db_index EQ UNUSED_INDEX)
  {
    db_index = phb_ctb[book].first_rcd;
    phb_ctb[book].first_rcd = UNUSED_INDEX;
  }
  else
    db_index = phb_element[db_index].next_rcd;
  if (db_index NEQ UNUSED_INDEX)
  {
    memset(data, 0xFF, sizeof(data));
    if (pb_write_sim(book, phb_element[db_index].entry.index) EQ PHB_FAIL)
    {
      db_index = UNUSED_INDEX;
      phb_stat = PHB_READY;
      cmhPHB_StatIndication ( PHB_READY, CME_ERR_NotPresent, FALSE );
    }
//TISH delete related EXT element
#ifdef PHONEBOOK_EXTENSION
	else
	{
		if (phb_element[db_index].entry.ext_rcd_num!=0xFF)
		{
	        USHORT         file_id = 0;
            pb_rem_ext_record_flag (book, phb_element[db_index].entry.ext_rcd_num);
			file_id = pb_get_ext_file_id (book);
			pb_prepare_ext_data (NULL, 0, NULL, 0, file_id);
			pb_write_sim_ext(file_id, phb_element[db_index].entry.ext_rcd_num);	
		}
	}
#endif
 }
  else
  {
    db_index = UNUSED_INDEX;
    phb_stat = PHB_READY;
    cmhPHB_StatIndication ( PHB_READY, CME_ERR_NotPresent, FALSE );
  }
}



/*
+------------------------------------------------------------------------+
| PROJECT : MMI-Framework (8417)         MODULE: PHB                     |
| STATE   : code                         ROUTINE : pb_update_ecc_fu      |
+------------------------------------------------------------------------+

  PURPOSE : Read emergency call numbers from SIM card.

*/
#ifdef SIM_TOOLKIT
BOOL pb_update_ecc_fu(int ref, T_SIM_FILE_UPDATE_IND *fu)
{

  BOOL found = FALSE;
  UBYTE i;
  TRACE_FUNCTION ("pb_update_ecc_fu()");

  for (i = 0; i < (int)fu->val_nr; i++)
  {
    if (!found AND fu->file_info[i].v_path_info EQ TRUE AND
          fu->file_info[i].path_info.df_level1 EQ SIM_DF_GSM      AND
          fu->file_info[i].path_info.v_df_level2 EQ FALSE         AND
          fu->file_info[i].datafield EQ SIM_ECC)
    {
      found = TRUE;
    }
  }

  if (found)
  {
    simShrdPrm.fuRef = ref;
    pb_update_ecc();
    return FALSE; /* reading files */
   }
  else
  {
    return TRUE;  /* nothing to do */
  }
}
#endif

/*
+------------------------------------------------------------------------+
| PROJECT : MMI-Framework (8417)         MODULE: PHB                     |
| STATE   : code                         ROUTINE : pb_update_ecc         |
+------------------------------------------------------------------------+

  PURPOSE : Read emergency call numbers from SIM card.

*/

void pb_update_ecc(void)
{
  TRACE_FUNCTION("pb_update_ecc()");
  if ((fdn_mode != NO_OPERATION) AND (simShrdPrm.fuRef EQ -1))
    return;

  phb_ctb[ECC].mem = SIM_MEMORY;

  phb_ctb[ECC].type    = ECC;
  phb_ctb[ECC].first_trcd = UNUSED_INDEX;
  pb_read_sim_ecc();

}

/*
+------------------------------------------------------------------------+
| PROJECT : MMI-Framework (8417)         MODULE: PHB                     |
| STATE   : code                         ROUTINE : pb_read_sim_ecc       |
+------------------------------------------------------------------------+

  PURPOSE : Read EC number from SIM card.

*/

void pb_read_sim_ecc (void)
{
  TRACE_FUNCTION ("pb_read_sim_ecc()");

  if (pb_read_sim_dat(SIM_ECC, NOT_PRESENT_8BIT, (UBYTE)256) EQ FALSE )  /* give the max length 256 */
    pb_read_eeprom_ecc();
}

/*
+------------------------------------------------------------------------+
| PROJECT : MMI-Framework (8417)         MODULE: PHB                     |
| STATE   : code                         ROUTINE : pb_read_eeprom_ecc    |
+------------------------------------------------------------------------+

  PURPOSE : Read EC number from EEPROM.

*/

void pb_read_eeprom_ecc (void)
{
  EF_ECC       efecc;
  UBYTE        *data_ptr;
  UBYTE        version;
  int          i;

  phb_ctb[ECC].mem        = TE_MEMORY;
  phb_ctb[ECC].type       = ECC;
  phb_ctb[ECC].max_rcd    = MAX_ECC_RCD;
  phb_ctb[ECC].first_trcd = UNUSED_INDEX;

  if (pcm_ReadFile((UBYTE *)EF_ECC_ID,
                    SIZE_EF_ECC,
                    (UBYTE *)&efecc,
                    &version) EQ DRV_OK)
  {
    

    
    { /* workaround when invalid data stored on PCM */
      CHAR  ecc_number[MAX_PHB_NUM_LEN];
      int   j;

      data_ptr = &efecc.ecc1[0];
      
      for (i=0; i < phb_ctb[ECC].max_rcd; i++)
      {
        if (*data_ptr NEQ 0xFF)
        {
          cmhPHB_getAdrStr (ecc_number,
                            MAX_PHB_NUM_LEN - 1,
                            data_ptr,
                            3);
          for (j = 0; j < 3; j++)
          {
            if (!isdigit (ecc_number[j]))
            {
              TRACE_EVENT_P2 ("[ERR] pb_read_eeprom_ecc(): invalid character found %c (%d)",
                              ecc_number[j], i);
              return;
            }
          }
        }
        data_ptr += 3;
      }
    } /* workaround end */  
    
    data_ptr = &efecc.ecc1[0];

    for (i=0; i < phb_ctb[ECC].max_rcd; i++)
    {
      pb_copy_ecc_entry(data_ptr, (UBYTE)i);
      data_ptr += 3;
    }
  }
}

/*
+------------------------------------------------------------------------+
| PROJECT : MMI-Framework (8417)         MODULE: PHB                     |
| STATE   : code                         ROUTINE : pb_copy_ecc_entry     |
+------------------------------------------------------------------------+

  PURPOSE : Read EC number from EEPROM.

*/

void pb_copy_ecc_entry (UBYTE *ecc, UBYTE num)
{
  SHORT index;
  /* TRACE_FUNCTION("pb_copy_ecc_entry()"); */
  if (*ecc NEQ 0xff)
  {
    /* search a free element in phonebook element table */
    if (pb_create_memory(&index) NEQ PHB_OK)
      return;

    phb_ctb[ECC].used_rcd++;
    phb_ctb[ECC].rcd_bitmap[0] |= (UBYTE)(0x01 << num);

    phb_element[index].type   = ECC;
    phb_element[index].entry.index  = (UBYTE)(num+1);
    phb_element[index].entry.len = 3;
    memcpy(phb_element[index].entry.number, ecc, 3);

    pb_record_sort(index);
    pb_num_sort(index);

    phb_element[index].prev_trcd  = UNUSED_INDEX;
    phb_element[index].next_trcd  = UNUSED_INDEX;
  }
}


/*
+----------------------------------------------------------------------+
| PROJECT: MMI-Framework (8417)         MODULE: PHB                    |
| STATE  : code                         ROUTINE: pb_read_sim_dat       |
+----------------------------------------------------------------------+


   PURPOSE :   Request to read SIM card.

*/

BOOL pb_read_sim_dat(USHORT data_id, UBYTE len, UBYTE max_length)
{
  SHORT table_id;

  TRACE_FUNCTION ("pb_read_sim_dat()");

  table_id = psaSIM_atbNewEntry();

  if(table_id NEQ NO_ENTRY)
  {
    simShrdPrm.atb[table_id].ntryUsdFlg = TRUE;
    simShrdPrm.atb[table_id].accType    = ACT_RD_DAT;
    simShrdPrm.atb[table_id].v_path_info  = FALSE;
    simShrdPrm.atb[table_id].reqDataFld = data_id;
    simShrdPrm.atb[table_id].dataOff    = 0;
    simShrdPrm.atb[table_id].dataLen    = len;
    simShrdPrm.atb[table_id].recMax     = max_length;
    simShrdPrm.atb[table_id].exchData   = NULL;
    simShrdPrm.atb[table_id].rplyCB     = pb_read_sim_dat_cb;

    simShrdPrm.aId = table_id;

    if(psaSIM_AccessSIMData() < 0)
    {
      TRACE_EVENT("FATAL ERROR");
      return FALSE;
    }
    return TRUE;
  }
  return FALSE;
}

/*
+----------------------------------------------------------------------+
| PROJECT: MMI-Framework (8417)         MODULE: PHB                    |
| STATE  : code                         ROUTINE: pb_read_sim_dat_cb    |
+----------------------------------------------------------------------+


   PURPOSE :   Call back for SIM read.

*/
void pb_read_sim_dat_cb(SHORT table_id)
{
  int i;
  UBYTE *data_ptr;

  TRACE_FUNCTION ("pb_read_sim_dat_cb()");

  switch (simShrdPrm.atb[table_id].reqDataFld)
  {
    case SIM_ECC:
      data_ptr = simShrdPrm.atb[table_id].exchData;
      if ( simShrdPrm.atb[table_id].errCode EQ SIM_NO_ERROR )
      {
        phb_ctb[ECC].max_rcd = (SHORT)simShrdPrm.atb[table_id].dataLen/3;

        for (i=0; i < phb_ctb[ECC].max_rcd; i++)
        {
          pb_copy_ecc_entry(data_ptr, (UBYTE)i);
          data_ptr += 3;
        }
#ifdef SIM_TOOLKIT
        if (simShrdPrm.fuRef >= 0)
        {
          psaSAT_FUConfirm (simShrdPrm.fuRef, SIM_FU_SUCCESS);
        }
#endif
      }
      else
      {
#ifdef SIM_TOOLKIT
        if (simShrdPrm.fuRef >= 0)
        {
          psaSAT_FUConfirm (simShrdPrm.fuRef, SIM_FU_ERROR);
        }
#endif
        pb_read_eeprom_ecc();
      }
      break;

    case SIM_SST:
      data_ptr = simShrdPrm.atb[table_id].exchData;
      if ( simShrdPrm.atb[table_id].errCode EQ SIM_NO_ERROR )
      {
        /* copy SIM service table */
        memset(sim_service_table, 0, sizeof(sim_service_table));
        memcpy(sim_service_table, 
               simShrdPrm.atb[table_id].exchData, 
               MINIMUM(MAX_SRV_TBL, simShrdPrm.atb[table_id].dataLen));

        /* initialisation of the all SIM phonebooks */
        pb_sat_update_reset(SIM_ADN);
        pb_sat_update_reset(SIM_FDN);
        pb_sat_update_reset(SIM_BDN);
        pb_sat_update_reset(SIM_SDN);
        pb_sat_update_reset(SIM_MSISDN);

        /* start reading SIM phonebook */
        pb_start_build(FALSE);
      }
      else
        pb_start_build(FALSE);
      break;

    default:
      break;
  }
  simShrdPrm.atb[table_id].ntryUsdFlg = FALSE;
}

/*
+----------------------------------------------------------------------+
| PROJECT: MMI-Framework                MODULE : PHB                   |
| STATE  : code                         ROUTINE: pb_set_compare        |
+----------------------------------------------------------------------+


   PURPOSE :   set a external MMI compare function for phonebook entries
*/

void pb_set_compare_fct (T_PHB_EXT_CMP_FCT compare_fct)
{
  ext_compare_fct = compare_fct;
}

/*
+----------------------------------------------------------------------+
| PROJECT: MMI-Framework                MODULE : PHB                   |
| STATE  : code                         ROUTINE: pb_get_fdn_input_classtype  |
+----------------------------------------------------------------------+


   PURPOSE :   get fdn_input_classtype
*/

T_ACI_CLASS pb_get_fdn_input_classtype (void)
{
  return fdn_input_classtype;
}

/*
+----------------------------------------------------------------------+
| PROJECT: MMI-Framework                MODULE : PHB                   |
| STATE  : code                         ROUTINE: pb_set_fdn_input_classtype  |
+----------------------------------------------------------------------+


   PURPOSE :   set fdn_input_classtype
*/

void pb_set_fdn_input_classtype (T_ACI_CLASS classtype)
{
   fdn_input_classtype = classtype;
}

/*
+----------------------------------------------------------------------+
| PROJECT: MMI-Framework                MODULE : PHB                   |
| STATE  : code                         ROUTINE: pb_get_fdn_classtype  |
+----------------------------------------------------------------------+


   PURPOSE :   get fdn_classtype
*/

T_ACI_CLASS pb_get_fdn_classtype (void)
{
  return fdn_classtype;
}

/*
+----------------------------------------------------------------------+
| PROJECT: MMI-Framework                MODULE : PHB                   |
| STATE  : code                         ROUTINE: pb_set_fdn_classtype  |
+----------------------------------------------------------------------+


   PURPOSE :   set fdn_classtype
*/

void pb_set_fdn_classtype (T_ACI_CLASS classtype)
{
  fdn_classtype = classtype;
}


/*
+----------------------------------------------------------------------+
| PROJECT: MMI-Framework                MODULE : PHB                   |
| STATE  : code                         ROUTINE: pb_get_fdn_mode       |
+----------------------------------------------------------------------+

  PURPOSE :   get fdn_mode
*/

UBYTE  pb_get_fdn_mode    (void)
 
{
  return fdn_mode;
}
/*
+----------------------------------------------------------------------+
| PROJECT: MMI-Framework                MODULE : PHB                   |
| STATE  : code                         ROUTINE: pb_set_fdn_mode       |
+----------------------------------------------------------------------+

  PURPOSE :   set fdn_mode
*/

void pb_set_fdn_mode   (UBYTE fdnmode)
  
{
  fdn_mode = fdnmode;
}

/*
+--------------------------------------------------------------------+
| PROJECT: MMI-Framework (8417)         MODULE : PHB                 |
| STATE  : code                         ROUTINE: pb_free_used_record |
+--------------------------------------------------------------------+

    PURPOSE : Making used record as free, whenever extension record 
              is not free while adding an entry with extension data
*/

LOCAL void pb_free_used_record(UBYTE type, SHORT index, UBYTE rec_num)
{
  UBYTE  n,m;

  TRACE_FUNCTION ("pb_free_used_record()");

  phb_element[index].free = PHB_ELEMENT_FREE;

  /* Update used records */
  phb_ctb[type].used_rcd--;

  if ((type EQ ADN) OR (type EQ FDN))
  {
    phb_ctb[ADN_FDN].used_rcd--;
  }
  
  /* Update record bitmap */
  n = (UBYTE)rec_num/8;
  m = rec_num%8;
  
  phb_ctb[type].rcd_bitmap[n] ^= 0x01 << m;
}


#endif /* #ifndef TI_PS_FFS_PHB */
