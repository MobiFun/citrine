/*
+--------------------------------------------------------------------+
| PROJECT:                              $Workfile:: cphs.c          $|
| $Author::                             $Revision::                 $|
| CREATED:                              $Modtime::                  $|
| STATE  : code                                                      |
+--------------------------------------------------------------------+

   MODULE  : CPHS

   PURPOSE : This module contains the CPHS functionalities.
*/


#ifndef CPHS_C
#define CPHS_C

#include "config.h"
#include "fixedconf.h"
#include "condat-features.h"
#include "aci_conf.h"

/* needed for all files that are part of ACI */
#include "aci_all.h"
#include "aci_cmh.h"
#include "phb.h"
#include "cphs.h"

#include "psa.h"
#include "cmh.h" 
#include "psa_sim.h"
#include "dti_conn_mng.h"
#include "cmh_sim.h" 

/* Definition of records indices in association with cphs lines */
#define CPHS_LINE1_REC     (1)
#define CPHS_LINE2_REC     (2)
#define CPHS_LINE_DATA_REC (3)
#define CPHS_LINE_FAX_REC  (4)

/* cphs initialising status */
typedef enum
{
  CPHS_NOT_INITIALISED = 0,
  CPHS_INITIALISING,           /* currently initialising */
  CPHS_INITIALISED,            /* initialised, currently does nothing */
  CPHS_REFRESHING,             /* refreshing cached data */
  CPHS_WRITING_CFU,            /* writing new CFU status on SIM */
  CPHS_WRITING_VWI,            /* writing new VWI status on SIM: user initiated */
  CPHS_WRITING_VWI_IND         /* writing new VWI status on SIM: network initiated */
} T_CPHS_STATUS;


UBYTE max_mb_records; 
UBYTE cphs_mb_ext_record_num[CPHS_MAX_MB_ENTRIES]; 


/* data send to SIM */
static UBYTE exchData[100];

/* CPHS status: provides information on what action is currently on progress */
static T_CPHS_STATUS cphs_status = CPHS_NOT_INITIALISED;

/* parameter describing at what step updating currently is */
static UBYTE sim_cache_update_state = CPHS_SIM_NOT_UPDATING;

#define CINF_SIZE (3)

/* VWI and CFU tables have to have the same size */
#define FLAG_TABLE_SIZE (2)
#define CFU_SIZE FLAG_TABLE_SIZE
#define VWI_SIZE FLAG_TABLE_SIZE

typedef struct
{
  T_CPHS_USER_CB  *user_cb;                /* callback used by CPHS module to pass results to ACI */
  UBYTE           current_record;          /* record currently read */
  UBYTE           *sim_read_record_buffer; /* buffer for record currently read */

  UBYTE           tmp_activate_state;      /* TEMP setting being currently written on SIM (CFU or VWI) */ 
  T_CPHS_LINES    tmp_lines;               /* TEMP lines being currently written on SIM (CFU or VWI) */

  UBYTE           tmp_flag_set[FLAG_TABLE_SIZE]; /* Flags Field (corresponding to tmp_lines) being currently
                                                    written on SIM (CFU or VWI) */
  UBYTE           tmp_info[CINF_SIZE];     /* voice mail waiting bitmask of CPHS lines */
} T_CPHS_INTERNAL_PARAMS;

LOCAL T_CPHS_INTERNAL_PARAMS *cphs_internal_params;
LOCAL BOOL  cphs_mb_ext_flag = FALSE;


/***** Parameters concerning ALS info ********/
/* how many different lines are there ? line1, line2, fax, data */
/* defines order of presentation of the lines when testing command AT%CPALS=? */
#define LINE1_ID    (0)
#define LINE2_ID    (1)
#define LINEDATA_ID (2)
#define LINEFAX_ID  (3)
#define MAX_LINE_ID (4)

#define DEFAULT_LINE1_NAME "Line 1"
#define DEFAULT_LINE2_NAME "Line 2"
#define DEFAULT_DATA_NAME "Data"
#define DEFAULT_FAX_NAME "Fax"
#define CPHS_MAX_SIZE_ALPHA (22)

typedef struct
{
  T_CPHS_LINES line;
  CHAR         line_desc[CPHS_MAX_SIZE_ALPHA];

} T_CPHS_ALS_NAMES;


/* structure containing the cached parameters from SIM */
typedef struct
{
  UBYTE           cfu_flags[CFU_SIZE];   /* same format as on the SIM: field 6F13 */
  UBYTE           vwi_flags[VWI_SIZE];   /* same format as on the SIM: field 6F11 */
  UBYTE           opn_long[CPHS_MAX_OPER_LONG];   /* operator long name */
  UBYTE           opn_short[CPHS_MAX_OPER_SHORT]; /* operator shortname */
  T_CPHS_INF_NUM  *info_numbers;         /* contains all info numbers entries */
  UBYTE           max_info_numbers;      /* index of last entry in info number list */
  T_CPHS_ALS_NAMES *als;
  UBYTE           max_als_names_entries;
  T_CPHS_CINF     info;                  /* cphs information */
  UBYTE           csp[CPHS_MAX_CSP];     /* same format as on the SIM: field 6F15 */
  UBYTE           csp_length;            /* Value given by SIM after Read action */
  UBYTE           orange_csp2[CPHS_MAX_CSP];    /* Orange specific - same format as on the SIM: field 0x6f98 */
  UBYTE           orange_csp2_length;            /* Orange specific - Value given by SIM after Read action */
  T_CPHS_MB       *mb_numbers;
  UBYTE           max_mb_numbers;
  UBYTE           max_mb_entry_length;   /* max length for mailbox entry */
} T_CPHS_CACHED_PARAMS;

LOCAL T_CPHS_CACHED_PARAMS *cphs_cached_params; /* memory allocated at initialisation */

LOCAL void cphs_cache_sim_data(UBYTE max_records, UBYTE latest_record_len);
LOCAL void cphs_set_next_update_state(void);
LOCAL T_CPHS_RET cphs_get_indicator_flag(UBYTE *flag_set, T_CPHS_LINES line, UBYTE indicator);
LOCAL void cphs_write_indicator_flag(UBYTE        flag_set, 
                                     T_CPHS_LINES lines, 
                                     UBYTE        indicator);
LOCAL void get_name_pointer_over_lineid(T_CPHS_LINES line, CHAR **line_desc, UBYTE max_lined);

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CPHS                     |
| STATE   : code                  ROUTINE : Internal functions       |
+--------------------------------------------------------------------+

  PURPOSE : Set of functions used internally.
*/

/* Informs user of the result of an operation */
GLOBAL void cphs_inform_user(T_CPHS_CB     callback_type, 
                             T_CPHS_RET    return_value,
                             UBYTE         set_flag,
                             T_CPHS_LINES  line)
{
  T_CPHS_PARAMS user_cb_params;

  TRACE_EVENT_P4("cphs_inform_user(): cb_type: %d, result: %d, flag: %d, line: %d", 
                                      callback_type, return_value, set_flag, line);

  user_cb_params.cb_type          = callback_type;
  user_cb_params.operation_result = return_value;

  user_cb_params.set_flag = set_flag;
  user_cb_params.line     = line;

  cphs_internal_params->user_cb(&user_cb_params);
}

/* Checks whether CPHS module has been initialised or if it is currently
being used.

*** All user interface function should use this function ***
*** to check cphs_status before processing               *** */
GLOBAL T_CPHS_RET cphs_check_status(void)
{
  TRACE_FUNCTION("cphs_check_status()");

  switch(cphs_status)
  {
  case(CPHS_NOT_INITIALISED):
    TRACE_EVENT("CPHS has not been initialised");
    return(CPHS_NOT_INIT);

  case(CPHS_INITIALISED):
    return(CPHS_OK);

  default:
    TRACE_EVENT("CPHS is BUSY: currently processing data");
    return(CPHS_BUSY);
  }
}


/* Aborts currently processed operation */
GLOBAL void cphs_abort_current_action(void)
{
  TRACE_EVENT_P1("cphs_abort_current_action(): action id: %d", cphs_status);

  switch(cphs_status)
  {
  case(CPHS_INITIALISING):
  case(CPHS_REFRESHING):
    /* CPHS module was currently updating cached data from the SIM */
    sim_cache_update_state = CPHS_SIM_NOT_UPDATING;

    cphs_inform_user(CPHS_INIT_RES, CPHS_FAIL, NOT_PRESENT_8BIT, CPHS_LINE_NULL);
    cphs_status = CPHS_NOT_INITIALISED; /* initialising aborted */
    return;

  default:
    cphs_status = CPHS_INITIALISED;
    return;
  }
}

#define RECORD_BUFF (0)
#define INFO_BUFF   (1)
#define ALS_BUFF    (2)
#define MB_BUFF     (3)

LOCAL void free_buffer(UBYTE buffer_id)
{
  switch(buffer_id)
  {
  case(RECORD_BUFF):
    if(cphs_internal_params->sim_read_record_buffer NEQ NULL)
    {
      MFREE(cphs_internal_params->sim_read_record_buffer);
      cphs_internal_params->sim_read_record_buffer = NULL;
    }
    break;

  case(INFO_BUFF):
    if(cphs_cached_params->info_numbers NEQ NULL)
    {
      //MFREE(cphs_cached_params->info_numbers);
      cphs_cached_params->info_numbers = NULL;
    }
    break;

  case(ALS_BUFF):
    if(cphs_cached_params->als NEQ NULL)
    {
      MFREE(cphs_cached_params->als);
      cphs_cached_params->als = NULL;
    }
    break;

  case(MB_BUFF):
    if(cphs_cached_params->mb_numbers NEQ NULL)
    {
      MFREE(cphs_cached_params->mb_numbers);
      cphs_cached_params->mb_numbers = NULL;
    }
    break;
  }
}

LOCAL void write_dummy_function(UBYTE info_number_index, UBYTE *sim_data, UBYTE record_len)
{
  TRACE_FUNCTION("write_dummy_function()");
}

LOCAL void write_info_number(UBYTE info_number_index, UBYTE *sim_data, UBYTE record_len)
{
  UBYTE          alpha_length, bcd_length;
  BOOL           check_network_specific_bit,
                 check_premium_service_bit;
  UBYTE          *ptr;
  T_CPHS_INF_NUM *current_info_number;

  TRACE_FUNCTION("write_info_number()");

  /* write data read from SIM for current_record */
  ptr = sim_data;

  /* get structure where data are to be written for this record */
  current_info_number = &cphs_cached_params->info_numbers[info_number_index];

  current_info_number->element_index = info_number_index;

  /* first byte: alpha_length */
  alpha_length = *ptr;
  if(alpha_length EQ 0xFF)
  {
    /* it has to be a NULL entry: Shall be ignored (see CPHS recom.). Jump to next record */
    return;
  }

  /* second byte */
  ptr++;
  check_network_specific_bit       = *ptr & 0x20;
  check_premium_service_bit        = *ptr & 0x10;
  current_info_number->index_level = *ptr & 0x0F;

  if(check_network_specific_bit)
  {
    /* info number is network specific */
    current_info_number->network_flag = TRUE;
  }
  else
    current_info_number->network_flag = FALSE;

  if(check_premium_service_bit)
  {
    /* info number is network specific */
    current_info_number->premium_flag = TRUE;
  }
  else
    current_info_number->premium_flag = FALSE;

  /* third byte to alpha_len+2 */
  /* Check 03.40 ?? */
  ptr++;

  if(alpha_length > sizeof(current_info_number->alpha_tag))
  {
    alpha_length = sizeof(current_info_number->alpha_tag);
  }

  /* initialize with 0s */
  memset(current_info_number->alpha_tag,
         0,
         sizeof(current_info_number->alpha_tag));
  memcpy(current_info_number->alpha_tag, ptr, alpha_length);

  /* Length of BCD byte */     /*********** ignore ??? ***************/
  ptr += alpha_length;
  if(*ptr EQ 0xFF)
  {
    /* this is a folder: no number information */
    bcd_length = 1; /* TON/NPI */
  }
  else
    bcd_length = *ptr;

  /* TON and NPI byte */
  ptr++;
  current_info_number->type_of_address = *ptr;
  bcd_length--;  /* TON and NPI are counted in bcd length */

  /* Digits section */
  ptr++;
  /* initialize with 0s */
  memset(current_info_number->number,
         0,
         sizeof(current_info_number->number));
  if(2*bcd_length > sizeof(current_info_number->number))
  {
    bcd_length = sizeof(current_info_number->number);
  }
  cphs_bcd2number(current_info_number->number, ptr, bcd_length);


  /*********** Trace info number read: ****************/
  TRACE_EVENT_P7("Info Number: add: %d, index: %d, alpha: %s, number: %s, index_level: %d, premium: %d, network: %d",
                 current_info_number,
                 current_info_number->element_index,
                 current_info_number->alpha_tag,
                 current_info_number->number,
                 current_info_number->index_level,
                 current_info_number->premium_flag,
                 current_info_number->network_flag);
}

LOCAL T_CPHS_LINES translate_index2line(UBYTE index)
{
  switch(index - 1)
  {
    case(LINE1_ID):
      return(CPHS_LINE1);

    case(LINE2_ID):
      return(CPHS_LINE2);

    case(LINEDATA_ID):
      return(CPHS_LINE_DATA);

    case(LINEFAX_ID):
      return(CPHS_LINE_FAX);

    default:
      TRACE_EVENT_P1("wrong index: %d", index);
      return(CPHS_LINE_NULL);
  }
}

LOCAL void write_als_names(UBYTE number_index, UBYTE *sim_data, UBYTE record_len)
{
  UBYTE max_entries, alpha_length;
  UBYTE i;
  CHAR  *line_desc;
  T_CPHS_LINES lineid;

  TRACE_FUNCTION("write_als_names( )");

  max_entries = cphs_cached_params->max_als_names_entries;

  lineid = translate_index2line(number_index);
  get_name_pointer_over_lineid(lineid, &line_desc, max_entries);

  if(line_desc EQ NULL)
  {
    TRACE_ERROR("line_desc is NULL");
    return;
  }

  alpha_length = 0;
  for(i=0 ; i<(record_len - CPHS_MIN_MB_LEN); i++)
  {
    if(sim_data[i] EQ 0xFF)
    {
      TRACE_EVENT_P1("length: %d", i);
      break;
    }
    alpha_length++;
  }

  if(alpha_length EQ 0)
  {
    TRACE_EVENT("No alpha tag on SIM");
    return;
  }
  memcpy(line_desc, sim_data, MINIMUM(alpha_length,CPHS_MAX_SIZE_ALPHA));
  line_desc[alpha_length] = '\0';

  /*********** Trace  ****************/
  TRACE_EVENT_P3("index: %d, line: %d, alpha: %s",
                 number_index,
                 lineid,
                 line_desc); 
}

LOCAL void write_mb_number(UBYTE number_index, UBYTE *sim_data, UBYTE record_len)
{
  UBYTE     i;
  UBYTE     pos;
  UBYTE     bcd_len;
  T_CPHS_MB *current_number;

  TRACE_FUNCTION("write_mb_number()");

  TRACE_EVENT_P3("idx: %d, ln: %d, data: %s", 
                 number_index, record_len, sim_data);

  /* get structure where data are to be written for this record */
  current_number = &cphs_cached_params->mb_numbers[number_index-1];
  memset(current_number->alpha_id, 0, sizeof(current_number->alpha_id));
  memset(current_number->number, 0, sizeof(current_number->number));

  if (cphs_cached_params->max_mb_numbers < number_index)
  {
    cphs_cached_params->max_mb_numbers = number_index;
  }

  /* map record index to lines */
  switch(number_index)
  {
  case CPHS_LINE1_REC:
    current_number->line = CPHS_LINE1;
    break;
  case CPHS_LINE2_REC:
    current_number->line = CPHS_LINE2;
    break;
  case CPHS_LINE_DATA_REC:
    current_number->line = CPHS_LINE_DATA;
    break;
  case CPHS_LINE_FAX_REC:
    current_number->line = CPHS_LINE_FAX;
    break;
  default:
    TRACE_EVENT("unexpected record index of cphs mailbox number");
    break;
  }
  
  /* get alpha identifier, if available */
  pos = record_len - CPHS_MIN_MB_LEN;
  if ( ( pos > 0 ) AND ( sim_data[0] NEQ 0xFF ) )
  {
    for (i=0; i<pos; i++)
    {
      if (sim_data[i] NEQ 0xFF)
        current_number->alpha_id[i] = sim_data[i];
    }
  }
  else
  {
    TRACE_EVENT("empty MB alpha");
  }
    
  /* get length of bcd */
  bcd_len = sim_data[pos];

  if ( bcd_len EQ 0xFF )
  {
    /* empty MB number */
    TRACE_EVENT_P2("empty MB number, pos: %d, bcd_len:%d", 
                   pos, bcd_len);
    
    current_number->toa = NOT_PRESENT_8BIT;
    return;
  }

  /* TON and NPI byte */
  pos++;
  current_number->toa = sim_data[pos];
  bcd_len--;  /* TON and NPI are counted in bcd length */

  /* Digits section */
  pos++;
  if(bcd_len > CPHS_MAX_MB_NUMBER_BYTES)
  {
    bcd_len = CPHS_MAX_MB_NUMBER_BYTES;
  }
  cphs_bcd2number(current_number->number, &(sim_data[pos]), bcd_len);


  if(number_index <= CPHS_MAX_MB_ENTRIES)
	  cphs_mb_ext_record_num[number_index-1] = sim_data[record_len-1]; 
  
	if(sim_data[record_len-1] NEQ 0xFF)
	 {
	   cphs_mb_ext_flag = TRUE; 
	   
	 }


  /*********** Trace mailbox number read: ****************/
  TRACE_EVENT_P5("index: %d, line: %d, alpha: %s, number: %s, toa: %d",
                 number_index,
                 current_number->line,
                 current_number->alpha_id,
                 current_number->number,
                 current_number->toa);
}

#define DEFAULT_MAXSIZE_OF_RECORD (100)
LOCAL void write_mb_ext_number(UBYTE number_index, UBYTE *sim_data, UBYTE record_len)
{
  UBYTE     i;
  UBYTE     pos;
  UBYTE     type; 
  UBYTE     bcd_len;
  T_CPHS_MB *current_number;
  UBYTE *current_record = &cphs_internal_params->current_record;

  TRACE_FUNCTION("write_mb_ext_number()");

  TRACE_EVENT_P3("idx: %d, ln: %d, data: %s",number_index, record_len, sim_data);

  /* get structure where data are to be written for this record */
  current_number = &cphs_cached_params->mb_numbers[number_index-1];
  pos =0; 
  /* get date type  */
  type = sim_data[pos];
  if(type EQ 2) /* additiona data*/
   {
      pos ++ ; 
      bcd_len = sim_data[pos];
      pos++; 
      if(bcd_len > CPHS_MAX_MB_NUMBER_BYTES)
       {
         bcd_len = CPHS_MAX_MB_NUMBER_BYTES; 
        }
      for (i =0; i<CPHS_MAX_MB_NUMBER_LEN; i++)
       {
          if(current_number->number[i] EQ 0)
           break; 
       }
      cphs_bcd2number(&(current_number->number[i]), &(sim_data[pos]), bcd_len);
   }
   cphs_mb_ext_flag = FALSE;


  /* last record read ?? */
   if(*current_record EQ max_mb_records) 
   {
     *current_record = 0;
     free_buffer(RECORD_BUFF);

	 /* END of mailbox/information numbers initialising */
      TRACE_EVENT("End of Reading Numbers");
      cphs_set_next_update_state();
      cphs_cache_sim_data(NOT_PRESENT_8BIT, NOT_PRESENT_8BIT);
   }
   else 
   {
      
     /* Read next record */
     (*current_record)++;
   
     cphs_sim_access_data( CPHS_SIM_READ_RECORD,
                           sim_cache_update_state,
                           *current_record,
                           cphs_internal_params->sim_read_record_buffer,
                           DEFAULT_MAXSIZE_OF_RECORD );
   }
   
 }



LOCAL void write_csp_ok(UBYTE *sim_data, UBYTE record_len)
{
  TRACE_FUNCTION("write_csp_ok()");

  if( sim_data NEQ NULL )
  {
    memcpy(cphs_cached_params->csp, sim_data, record_len );
  }
}
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CPHS                     |
| STATE   : code                  ROUTINE : cphs_first_free          |
+--------------------------------------------------------------------+

  PURPOSE : Find first free location in mailbox list
*/
GLOBAL T_CPHS_RET cphs_first_free(
  UBYTE *first_free)
{
  UBYTE rec_id;

  if(cphs_cached_params EQ NULL)
  {
    TRACE_ERROR("cphs_cached_params==NULL");
    ACI_ERR_DESC(ACI_ERR_CLASS_Cme,CME_ERR_OpNotAllow);
    return(CPHS_FAIL);
  }

  if (cphs_cached_params->mb_numbers EQ NULL)
  {
    TRACE_ERROR("cphs_cached_params->mb_numbers==NULL");
    ACI_ERR_DESC(ACI_ERR_CLASS_Cme,CME_ERR_OpNotAllow);
    return(CPHS_FAIL);
  }

  for (rec_id=0;rec_id<cphs_cached_params->max_mb_numbers;rec_id++)
  {
    if (cphs_cached_params->mb_numbers[rec_id].number[0] EQ 0)
    {
      *first_free=(UBYTE)(rec_id+1);
      return(CPHS_OK);
    }
  }

  /*
  *   List is full, indicate this with 0.
  */
  *first_free=0;
  return(CPHS_OK);
}

/* temp until we use lists */
/* Size has to be set to 600 considering the length alignment of the datastrct*/
UBYTE info_numbers_buffer[600];

LOCAL void cphs_init_records_info(UBYTE max_records)
{
  TRACE_FUNCTION("cphs_init_records_info( )");
  
  if(cphs_cached_params->info_numbers EQ NULL)    // should only be checked for first entry
  {
    /* This should happen only once: on the SIM are more records than thought at first.
    structure size has to be increased (and this has to be done before 1st entry is copied !!) */
    TRACE_EVENT_P1("cphs_cached_params->info_numbers list size: %d records on SIM", max_records);

/******* use lists !!!!!!!!!!!!! *****/        
    //MALLOC(cphs_cached_params->info_numbers, (ULONG)(max_records*sizeof(T_CPHS_INF_NUM)));
    cphs_cached_params->info_numbers = (T_CPHS_INF_NUM*)&info_numbers_buffer;
    cphs_cached_params->max_info_numbers = max_records;
  }
      
  /* First entry should be "ROOT" with index 0 */
  cphs_cached_params->info_numbers[0].element_index = 0;
  cphs_cached_params->info_numbers[0].index_level   = 0;
  sprintf(cphs_cached_params->info_numbers[0].alpha_tag, "ROOT");
  memset(cphs_cached_params->info_numbers[0].number,
         0,
         sizeof(cphs_cached_params->info_numbers[0].number));
}

LOCAL void cphs_init_records_mailbox(UBYTE max_records)
{
  TRACE_FUNCTION("cphs_init_records_mailbox( )");

  if(cphs_cached_params->mb_numbers EQ NULL)    // should only be checked for first entry
  {
    
  /* This should happen only once: on the SIM are more records than thought at first.
    structure size has to be increased (and this has to be done before 1st entry is copied !!) */
    TRACE_EVENT_P1("cphs_cached_params->mb_numbers list size: %d records on SIM", max_records);

    MALLOC(cphs_cached_params->mb_numbers, (USHORT)(max_records*sizeof(T_CPHS_MB)));

    cphs_cached_params->max_mb_numbers     = max_records;
  }
  
  cphs_cached_params->mb_numbers[0].line = CPHS_LINE_NULL;
  cphs_cached_params->mb_numbers[0].toa  = NOT_PRESENT_8BIT;
  memset(cphs_cached_params->mb_numbers[0].number, 0,
         sizeof(cphs_cached_params->info_numbers[0].number));
  memset(cphs_cached_params->mb_numbers[0].alpha_id, 0,
         sizeof(cphs_cached_params->mb_numbers[0].alpha_id));
  
  memset(cphs_mb_ext_record_num, NOT_PRESENT_8BIT,CPHS_MAX_MB_ENTRIES); 
}

LOCAL void initialize_als_info(UBYTE als_max_records)
{
  CHAR *line_desc;

  TRACE_FUNCTION("initialize_als_info( )");

  MALLOC(cphs_cached_params->als, (USHORT)(als_max_records* sizeof(T_CPHS_ALS_NAMES)));
  cphs_cached_params->max_als_names_entries = als_max_records;

  /* for SIMs with als_max_records > MAX_LINE_ID */
  if (als_max_records > MAX_LINE_ID)
      als_max_records = MAX_LINE_ID;

  switch (als_max_records)
  {
    case (LINEFAX_ID+1):
      cphs_cached_params->als[LINEFAX_ID].line = CPHS_LINE_FAX;
      get_name_pointer_over_lineid(CPHS_LINE_FAX, &line_desc, als_max_records);
      sprintf(line_desc, DEFAULT_FAX_NAME);
	  //lint -fallthrough
    case (LINEDATA_ID+1):
      cphs_cached_params->als[LINEDATA_ID].line = CPHS_LINE_DATA;
      get_name_pointer_over_lineid(CPHS_LINE_DATA, &line_desc, als_max_records);
      sprintf(line_desc, DEFAULT_DATA_NAME);
	  //lint -fallthrough
    case (LINE2_ID+1):
      cphs_cached_params->als[LINE2_ID].line = CPHS_LINE2;
      get_name_pointer_over_lineid(CPHS_LINE2, &line_desc, als_max_records);
      sprintf(line_desc, DEFAULT_LINE2_NAME);
	  //lint -fallthrough
    case (LINE1_ID+1):
      cphs_cached_params->als[LINE1_ID].line = CPHS_LINE1;
      get_name_pointer_over_lineid(CPHS_LINE1, &line_desc, als_max_records);
      sprintf(line_desc, DEFAULT_LINE1_NAME);
  }
}

LOCAL void cphs_init_records_alsnames(UBYTE max_records)
{
  TRACE_FUNCTION("cphs_init_records_alsnames( )");

  TRACE_EVENT_P1("cphs_cached_params->als list size: %d records on SIM", max_records);

  if(cphs_cached_params->als EQ NULL)
  {
  /* This should happen only once: on the SIM are more records than thought at first.
    structure size has to be increased (and this has to be done before 1st entry is copied !!) */

    /* init als name structure */
    initialize_als_info(max_records);
  }
}

LOCAL void cphs_init_records(UBYTE max_records, UBYTE sim_update_type)
{
  TRACE_FUNCTION("cphs_init_records( )");
  
  switch (sim_update_type)
  {
    case(CPHS_SIM_INFO_NUMS):
    case(CPHS_SIM_INFO_NUMS_EA01):
      cphs_init_records_info(max_records);
      break;

    case(CPHS_SIM_MB):
      cphs_init_records_mailbox(max_records);
      break;

    case(CPHS_SIM_ALSNAMES):
      cphs_init_records_alsnames(max_records);
      break;

    default:
      TRACE_EVENT_P1("Wrong sim_update_state: %d", sim_update_type);
      break;
  }

}


//#define DEFAULT_MAXSIZE_OF_RECORD (100)
typedef void T_CPHS_WRITE_NUMBER_FCT (UBYTE number_index, UBYTE *sim_data, UBYTE record_len);

LOCAL T_CPHS_WRITE_NUMBER_FCT *get_the_specific_data_update_type_dependant(UBYTE sim_update_type, 
                                                                           UBYTE *max_number)
{
  switch(sim_update_type)
  {
    case(CPHS_SIM_INFO_NUMS):
    case(CPHS_SIM_INFO_NUMS_EA01):
      TRACE_EVENT("Information Numbers");
      *max_number = cphs_cached_params->max_info_numbers;
      return(write_info_number);

    case(CPHS_SIM_MB):
      TRACE_EVENT("Mailbox Numbers");
      *max_number = cphs_cached_params->max_mb_numbers;
      return(write_mb_number);

    case(CPHS_SIM_ALSNAMES):
      TRACE_EVENT("MSISDN Names");
      *max_number = cphs_cached_params->max_als_names_entries;
      return(write_als_names);

    default:
      TRACE_ERROR("wrong sim_update_type value");
      break;
  }
  return(write_dummy_function);
}

/* returns TRUE if all records have been read... */
LOCAL BOOL cphs_read_sim_records(UBYTE max_records, 
                                 UBYTE sim_update_type, 
                                 UBYTE latest_record_len)
{
  UBYTE *current_record = &cphs_internal_params->current_record;
  T_CPHS_WRITE_NUMBER_FCT *write_number;
  UBYTE                   max_numbers;

  TRACE_FUNCTION("cphs_read_sim_records()");

  write_number = get_the_specific_data_update_type_dependant(sim_update_type, &max_numbers);

  switch(*current_record)
  {
  case(0):
    TRACE_EVENT("CPHS read records: START !!!");

    /* Begin of reading of the Information Numbers */
    if(cphs_internal_params->sim_read_record_buffer EQ NULL)
    {
      MALLOC(cphs_internal_params->sim_read_record_buffer, DEFAULT_MAXSIZE_OF_RECORD);   // buffer for the SIM to write data: temporary buffer
    }
    else
    {
      TRACE_EVENT("sim_read_record_buffer should be NULL: weird indeed...");
    }
    break;

  case(1):  
    cphs_init_records(max_records, sim_update_type);
	  //lint -fallthrough
  
  default:
    TRACE_EVENT_P1("number retrieved from SIM: record %d read", *current_record);
    if (write_number NEQ NULL)
    {
      write_number(*current_record, cphs_internal_params->sim_read_record_buffer, latest_record_len);
    }
    break;
  }

  
  if(cphs_mb_ext_flag EQ FALSE)
	{

  /* last record read ?? */
  if(*current_record EQ max_records) 
  {
    *current_record = 0;
    free_buffer(RECORD_BUFF);
    return(TRUE);
  }

   
  /* Read next record */
  (*current_record)++;

  cphs_sim_access_data( CPHS_SIM_READ_RECORD,
                        sim_update_type,
                        *current_record,
                        cphs_internal_params->sim_read_record_buffer,
                        DEFAULT_MAXSIZE_OF_RECORD );

   
   }
   else
   {
	  max_mb_records = max_records; 
	  cphs_sim_read_mb_ext_rcd(cphs_internal_params->sim_read_record_buffer[latest_record_len-1],cphs_internal_params->sim_read_record_buffer); 
   }

  return(FALSE);
}

LOCAL void get_name_pointer_over_lineid(T_CPHS_LINES line, CHAR **line_desc, UBYTE max_lined)
{
  UBYTE i;
  T_CPHS_ALS_NAMES *current_als;

  TRACE_FUNCTION("get_name_pointer_over_lineid()");

  for(i=0; i<max_lined; i++)
  {
    current_als = &(cphs_cached_params->als[i]);
    if(current_als->line EQ line)
    {
      *line_desc = current_als->line_desc;
      return;
    }
  }

  TRACE_ERROR("Ugly error: wrong line type");
  *line_desc = NULL;
}

/* Read data to be cached from SIM */
/* param_max_records: when reading first record, SIM return the number of records to be found
on the SIM: thus CPHS module can check whether allocated memory is enough */
LOCAL void cphs_cache_sim_data(UBYTE max_records, UBYTE latest_record_len)
{
  BOOL end_of_read;

  TRACE_FUNCTION("cphs_cache_sim_data()");

  /* operation to be performed */
  switch(sim_cache_update_state)
  {
  case(CPHS_SIM_NOT_UPDATING):
    /* START UPDATING !!! */
    cphs_set_next_update_state();
    cphs_cache_sim_data(NOT_PRESENT_8BIT, NOT_PRESENT_8BIT);
    return;
  
  case(CPHS_SIM_CFU):
    /* Read Call Forwarding Flag */
    cphs_sim_access_data( CPHS_SIM_READ_TRANSP_EF,
                          CPHS_SIM_CFU,
                          0,
                          cphs_cached_params->cfu_flags,
                          CFU_SIZE );
    return;

  case(CPHS_SIM_VWI):
    /* read waiting flags on SIM */
    cphs_sim_access_data( CPHS_SIM_READ_TRANSP_EF,
                          CPHS_SIM_VWI,
                          0,
                          cphs_cached_params->vwi_flags,
                          VWI_SIZE);
    return;

  case(CPHS_SIM_CINF):
    /* read cphs info on SIM */
    cphs_sim_access_data( CPHS_SIM_READ_TRANSP_EF,
                          CPHS_SIM_CINF,
                          0,
                          cphs_internal_params->tmp_info,
                          CINF_SIZE);
    return;

  case(CPHS_SIM_CSP):
    /* read customer service profile on SIM */
    cphs_sim_access_data( CPHS_SIM_READ_TRANSP_EF,
                          CPHS_SIM_CSP,
                          0,
                          cphs_cached_params->csp,
                          CPHS_MAX_CSP);
    return;

  /* Add support for Orange SIM's  */
  case(CPHS_SIM_ORANGE_CSP):
    /* read customer service profile on SIM */
    cphs_sim_access_data( CPHS_SIM_READ_TRANSP_EF,
                          CPHS_SIM_ORANGE_CSP,
                          0,
                          cphs_cached_params->orange_csp2,
                          CPHS_MAX_CSP);
    return;

  case(CPHS_SIM_OPNLONG):
    /* read operator name string on SIM */
    cphs_sim_access_data( CPHS_SIM_READ_TRANSP_EF,
                          CPHS_SIM_OPNLONG,
                          0,
                          cphs_cached_params->opn_long,
                          CPHS_MAX_OPER_LONG);
    return;

  case(CPHS_SIM_OPNSHORT):
    /* read operator name short string on SIM */
    cphs_sim_access_data( CPHS_SIM_READ_TRANSP_EF,
                          CPHS_SIM_OPNSHORT,
                          0,
                          cphs_cached_params->opn_short,
                          CPHS_MAX_OPER_SHORT);
    return;

  case(CPHS_SIM_ALSNAMES):
  case(CPHS_SIM_MB):
  case(CPHS_SIM_INFO_NUMS):
  case(CPHS_SIM_INFO_NUMS_EA01):
    /* Read mailbox/information numbers */

    end_of_read = cphs_read_sim_records(max_records, sim_cache_update_state, latest_record_len);

    if(end_of_read)
    {
      /* END of mailbox/information numbers initialising */
      TRACE_EVENT("End of Reading Numbers");
      cphs_set_next_update_state();
      cphs_cache_sim_data(NOT_PRESENT_8BIT, NOT_PRESENT_8BIT);
    }
    return;

  case(CPHS_SIM_STOP_INIT):
  default:
    break;
  }

  /* End of update */
  if(sim_cache_update_state NEQ CPHS_SIM_NOT_UPDATING)
  {
    TRACE_ERROR("wrong sim_cache_update_state state");
  }
  else
  {
    TRACE_EVENT("End of initialisation: Success");
    sim_cache_update_state = CPHS_SIM_NOT_UPDATING;
  }

  /* reset states */
  cphs_set_next_update_state();
  cphs_status = CPHS_INITIALISED;

  cphs_inform_user(CPHS_INIT_RES, CPHS_OK, NOT_PRESENT_8BIT, CPHS_LINE_NULL);
}

LOCAL void cphs_set_next_update_state(void)
{
  switch(sim_cache_update_state)
  {
    /****** Mandatory Fields *******/
  case(CPHS_SIM_NOT_UPDATING): /* START ! */
    sim_cache_update_state = CPHS_SIM_CFU;
    return;

  case(CPHS_SIM_CFU):
    sim_cache_update_state = CPHS_SIM_VWI;
    return;

  case(CPHS_SIM_VWI):
    sim_cache_update_state = CPHS_SIM_OPNLONG;
    return;
    
  case(CPHS_SIM_OPNLONG):
    sim_cache_update_state = CPHS_SIM_CINF;
    return;
    
    /****** Optional Fields *******/
  case(CPHS_SIM_CINF):  
    sim_cache_update_state = CPHS_SIM_CSP;
    return;

  case(CPHS_SIM_CSP):
    sim_cache_update_state = CPHS_SIM_ORANGE_CSP;
    return;

  /* Add support for Orange SIM's  */
  case(CPHS_SIM_ORANGE_CSP):
    sim_cache_update_state = CPHS_SIM_OPNSHORT;
    return;

  case(CPHS_SIM_OPNSHORT):
    sim_cache_update_state = CPHS_SIM_MB;    
    return;

  case(CPHS_SIM_MB):
    sim_cache_update_state = CPHS_SIM_INFO_NUMS;    
    return;

  case(CPHS_SIM_INFO_NUMS):
  case(CPHS_SIM_INFO_NUMS_EA01):
    sim_cache_update_state = CPHS_SIM_ALSNAMES;
    break;

  case(CPHS_SIM_ALSNAMES):
    sim_cache_update_state = CPHS_SIM_STOP_INIT;
    break;

  case(CPHS_SIM_STOP_INIT):
  default:
    sim_cache_update_state = CPHS_SIM_NOT_UPDATING;
    break;
  }
}

/* Should action (e.g cache updating) be aborted if field
is not present ? */
LOCAL BOOL is_cphs_field_mandatory(UBYTE field_type)
{
  TRACE_FUNCTION("is_cphs_field_mandatory()");
  
  switch(field_type)
  {
    /****** Mandatory Fields *******/
  case(CPHS_SIM_CFU):
  case(CPHS_SIM_VWI):
  case(CPHS_SIM_CINF):
  case(CPHS_SIM_OPNLONG):
    return(TRUE);

    /****** Optional fields ********/
  case(CPHS_SIM_OPNSHORT):
  case(CPHS_SIM_MB):
  case(CPHS_SIM_INFO_NUMS):
  case(CPHS_SIM_INFO_NUMS_EA01):
  case(CPHS_SIM_ALSNAMES):
  case(CPHS_SIM_CSP):
    /* Add support for Orange SIM's  */
  case(CPHS_SIM_ORANGE_CSP):
    return(FALSE);

  default:
    TRACE_EVENT_P1("Unexpected field type: %d", field_type);
    return(TRUE);
  }
}

/* Positive Result of data SIM data access while updating cached data */
LOCAL void cphs_init_sim_ok(UBYTE max_records, UBYTE sim_data_len)
{
  UBYTE i;
  BOOL set_next_state = TRUE;
  USHORT temp_sst_ushort2;
  
  TRACE_FUNCTION("cphs_init_sim_ok()");

  TRACE_EVENT_P1("cphs_init_sim_ok(): data_len: %d", sim_data_len);

  /* set next operation if needed */
  switch(sim_cache_update_state)
  {
  case(CPHS_SIM_CFU):
    TRACE_EVENT_P2("cfu_flags: %02X, %02X", cphs_cached_params->cfu_flags[0], cphs_cached_params->cfu_flags[1]);
    if(sim_data_len < CFU_SIZE)
    {
      /* Typically there are 2 bytes in CFU Ef_File: Yet only the first one is
      mandatory. Set non-present optional bytes to 0x00 */
      for(i=sim_data_len; i<CFU_SIZE; i++)
      {
        cphs_cached_params->cfu_flags[i] = 0x00;        
      }
    }
    break;

  case(CPHS_SIM_VWI):
    TRACE_EVENT_P2("vwi_flags: %02X, %02X", cphs_cached_params->vwi_flags[0], cphs_cached_params->vwi_flags[1]);
    if(sim_data_len < VWI_SIZE)
    {
      /* Typically there are 2 bytes in VWI Ef_File: Yet only the first one is
      mandatory. Set non-present optional bytes to 0x00 */
      for(i=sim_data_len; i<VWI_SIZE; i++)
      {
        cphs_cached_params->vwi_flags[i] = 0x00;        
      }
    }
    break;

  case(CPHS_SIM_MB):
    TRACE_EVENT_P1("store max_mb_entry_length: %d",sim_data_len);
    cphs_cached_params->max_mb_entry_length = sim_data_len;
    set_next_state = FALSE;
    break;
  case(CPHS_SIM_ALSNAMES):
  case(CPHS_SIM_INFO_NUMS):
  case(CPHS_SIM_INFO_NUMS_EA01):
    /* decision is made within cphs_cache_sim_data( )
       whether next action should be started */
    set_next_state = FALSE;
    break;

  case(CPHS_SIM_CSP):
    if(sim_data_len > CPHS_MAX_CSP)
    {
      TRACE_EVENT_P1("Risk of loss of CSP data: sim_data_len: %d", sim_data_len);
      cphs_cached_params->csp_length = CPHS_MAX_CSP;
    }
    else
    {
      cphs_cached_params->csp_length = sim_data_len;
      TRACE_EVENT_P1("cphs_cached_params->csp_length: %d", 
                     cphs_cached_params->csp_length);
    }
    break;

    /* Add support for Orange SIM's  */
  case(CPHS_SIM_ORANGE_CSP):
    if(sim_data_len > CPHS_MAX_CSP)
    {
      TRACE_EVENT_P1("Risk of loss of Orange CSP2 data: sim_data_len: %d", sim_data_len);
      cphs_cached_params->orange_csp2_length = CPHS_MAX_CSP;
    }
    else
    {
      cphs_cached_params->orange_csp2_length = sim_data_len;
      TRACE_EVENT_P1("cphs_cached_params->orange_csp2_length: %d", 
                     cphs_cached_params->orange_csp2_length);
    }
    break;

  case(CPHS_SIM_CINF):
    TRACE_EVENT_P3("CPHS info phase: %d sst: %02X, %02X", 
                   cphs_internal_params->tmp_info[0],
                   cphs_internal_params->tmp_info[1],
                   cphs_internal_params->tmp_info[2]);

    cphs_cached_params->info.phase = cphs_internal_params->tmp_info[0];
    
    cphs_cached_params->info.sst = 0x0000;

    if(sim_data_len > 1)
    {
      cphs_cached_params->info.sst = (cphs_internal_params->tmp_info[1] << 8);
    }
    
    if(sim_data_len < CINF_SIZE)
    {
      /* Typically there should be at least 3 bytes in CPHS information Ef_File:
      however, there are some SIMs with only 2 bytes... :-( Maybe with Phase 3 ?? */
    }
    else
    {
      temp_sst_ushort2 = 0x00FF & (USHORT)(cphs_internal_params->tmp_info[2]);
      cphs_cached_params->info.sst |= temp_sst_ushort2; 
    }

    TRACE_EVENT_P1("cphs_cached_params->info.sst: %04X", 
                   cphs_cached_params->info.sst);
    break;
  }

  if(set_next_state)
  {
    cphs_set_next_update_state();
  }

  /* Continue cache from SIM */
  cphs_cache_sim_data(max_records, sim_data_len);
}

LOCAL void writing_vwi_ok(void)
{
  TRACE_FUNCTION("writing_vwi_ok()");
  
  memcpy(cphs_cached_params->vwi_flags, cphs_internal_params->tmp_flag_set, VWI_SIZE);

  TRACE_EVENT_P2("vwi_flags: %02X %02X", cphs_cached_params->vwi_flags[0], 
                                         cphs_cached_params->vwi_flags[1]);

  if(cphs_status EQ CPHS_WRITING_VWI)
  {
    /* if this was initiated by user, then send final result */
    cphs_inform_user(CPHS_VOICE_MAIL_RES, CPHS_OK, NOT_PRESENT_8BIT, CPHS_LINE_NULL);
  }
  else
  {
    /* if this was initiated by network (upon receiving of an SMS, then send indication
    to the user */
    cphs_inform_user( CPHS_VOICE_MAIL_IND, CPHS_OK, cphs_internal_params->tmp_activate_state, 
                                                    cphs_internal_params->tmp_lines );
  }
}

LOCAL void writing_cfu_ok(void)
{
  TRACE_FUNCTION("writing_cfu_ok()");

  memcpy(cphs_cached_params->cfu_flags, cphs_internal_params->tmp_flag_set, CFU_SIZE);
  
  TRACE_EVENT_P2("cfu_flags: %02X %02X", cphs_cached_params->cfu_flags[0], 
                                         cphs_cached_params->cfu_flags[1]);

  cphs_inform_user(CPHS_CFU_RES, CPHS_OK, NOT_PRESENT_8BIT, CPHS_LINE_NULL);
}

/* Positive Result of data SIM data access */
GLOBAL void cphs_sim_data_accessed(UBYTE max_records, UBYTE data_len)
{
  TRACE_FUNCTION("cphs_sim_data_accessed()");

  switch(cphs_status)
  {
  case(CPHS_INITIALISING):
  case(CPHS_REFRESHING):
    cphs_init_sim_ok(max_records, data_len);
    return;

  case(CPHS_WRITING_VWI_IND):
  case(CPHS_WRITING_VWI):
    TRACE_EVENT_P1("vwi_flags length: %d bytes", data_len);
    writing_vwi_ok();
    cphs_status = CPHS_INITIALISED;
    return;
  
  case(CPHS_WRITING_CFU):
    /* cfu_flags is only updated if writing on SIM has been successful */
    TRACE_EVENT_P1("cfu_flags: %d bytes length", data_len);
    writing_cfu_ok();
    cphs_status = CPHS_INITIALISED;
    return;

  default:
    TRACE_EVENT("cphs_sim_data_accessed(): unexpected status");
    cphs_status = CPHS_INITIALISED;
    return;
  }
}

/* Positive Result of data SIM data access */
GLOBAL void cphs_write_sim_mb_ext_data(UBYTE data_len)
{
  UBYTE *current_record = &cphs_internal_params->current_record;

  TRACE_FUNCTION("cphs_sim_mb_ext_data_accessed()");

 write_mb_ext_number(*current_record, cphs_internal_params->sim_read_record_buffer, data_len);
  
}


LOCAL void cphs_init_sim_failure(void)
{
  TRACE_FUNCTION("cphs_init_sim_failure()");

  /* set next operation if needed */
  switch(sim_cache_update_state)
  {
  case(CPHS_SIM_INFO_NUMS):
    /* Special case: when reading information number, if file 6F19
       is not present, another try should be done with EA01 for old SIMs */
    cphs_internal_params->current_record = 0;
    free_buffer(RECORD_BUFF);
    free_buffer(INFO_BUFF);
    sim_cache_update_state = CPHS_SIM_INFO_NUMS_EA01;
    cphs_cache_sim_data(NOT_PRESENT_8BIT, NOT_PRESENT_8BIT);
    return;

  case(CPHS_SIM_INFO_NUMS_EA01):
    cphs_internal_params->current_record = 0;
    free_buffer(RECORD_BUFF);
    free_buffer(INFO_BUFF);
    /* No information numbers found... */
    cphs_cached_params->max_info_numbers = NOT_PRESENT_8BIT;
    break;

  case(CPHS_SIM_ALSNAMES):
    cphs_internal_params->current_record = 0;
    free_buffer(ALS_BUFF);
    initialize_als_info(MAX_LINE_ID);
    break;

  case(CPHS_SIM_MB):
    cphs_internal_params->current_record = 0;
    free_buffer(RECORD_BUFF);
    free_buffer(MB_BUFF);
    /* No mailbox numbers found... */
    cphs_cached_params->max_mb_numbers = NOT_PRESENT_8BIT;
    break;
  }

  if(is_cphs_field_mandatory(sim_cache_update_state))
  {
    cphs_abort_current_action( );
    return;
  }

  cphs_set_next_update_state();
  cphs_cache_sim_data(NOT_PRESENT_8BIT, NOT_PRESENT_8BIT);
}

/* Failure Result of data SIM data access */
GLOBAL void cphs_sim_data_failure(void)
{
  TRACE_FUNCTION("cphs_sim_data_failure()");

  switch(cphs_status)
  {
  case(CPHS_INITIALISING):
  case(CPHS_REFRESHING):
    cphs_init_sim_failure();
    break;

  case(CPHS_WRITING_VWI):
    TRACE_ERROR("Voice Mail Indicator setting: SIM FAILURE !!!!");
    cphs_status = CPHS_INITIALISED;
    cphs_inform_user(CPHS_VOICE_MAIL_RES, CPHS_FAIL, 
                     NOT_PRESENT_8BIT, CPHS_LINE_NULL);
    return;
  
  case(CPHS_WRITING_VWI_IND):
    TRACE_ERROR("Voice Mail Indication: SIM FAILURE !!!!");
    cphs_status = CPHS_INITIALISED;
    return;

  case(CPHS_WRITING_CFU):
    TRACE_ERROR("Call Fwd Flags: SIM FAILURE !!!!");
    cphs_inform_user(CPHS_CFU_RES, CPHS_FAIL, 
                     NOT_PRESENT_8BIT, CPHS_LINE_NULL);
    cphs_status = CPHS_INITIALISED;
    return;

  default:
    TRACE_EVENT("cphs_sim_data_failure(): unexpected status");
    return;
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CPHS                     |
| STATE   : code                  ROUTINE : starts/ends/refreshes    |
+--------------------------------------------------------------------+

  PURPOSE : Set of functions enabling initialising, ending or refreshing
            the CPHS module.
*/

GLOBAL T_CPHS_RET cphs_start (T_CPHS_USER_CB *cphs_user_cb)
{
  TRACE_FUNCTION("cphs_start()");

  /* Check CPHS status */
  switch(cphs_status)
  {
  case(CPHS_NOT_INITIALISED):
    TRACE_EVENT("CPHS starting !!!");
    break;

  case(CPHS_INITIALISED):
    TRACE_EVENT("cphs_start: CPHS module already initialised");
    return(CPHS_OK);

  default:
    TRACE_EVENT("cphs_start: CPHS has already been initialised: BUSY !!!");
    return(CPHS_BUSY);
  }

  if(cphs_user_cb EQ NULL)
  {
    TRACE_ERROR("cphs_start: cphs_user_cb is NULL. CPHS module cannot be initialised");
    return(CPHS_FAIL);
  }

  /* allocate memory for parameters to be cached from the SIM */
  MALLOC(cphs_cached_params, sizeof(T_CPHS_CACHED_PARAMS));

  cphs_cached_params->max_mb_numbers         = NOT_PRESENT_8BIT; //MAX_LINE_ID;
  cphs_cached_params->mb_numbers             = NULL;
  cphs_cached_params->max_info_numbers       = NOT_PRESENT_8BIT;
  cphs_cached_params->info_numbers           = NULL;
  cphs_cached_params->als                    = NULL;
  cphs_cached_params->max_als_names_entries  = NOT_PRESENT_8BIT;

  memset(cphs_cached_params->opn_long,  0, sizeof(cphs_cached_params->opn_long));
  memset(cphs_cached_params->opn_short, 0, sizeof(cphs_cached_params->opn_short));
  memset(cphs_cached_params->csp,       0, sizeof(cphs_cached_params->csp));
  /* Add support for Orange SIM's  */
  memset(cphs_cached_params->orange_csp2,  0, sizeof(cphs_cached_params->orange_csp2));
  cphs_cached_params->orange_csp2_length = 0; /* Set this to 0, will indicate if vaild */
  
  MALLOC(cphs_internal_params, sizeof(T_CPHS_INTERNAL_PARAMS));
  /* Set user callback */
  cphs_internal_params->user_cb = cphs_user_cb;

  cphs_internal_params->current_record         = 0;
  cphs_internal_params->sim_read_record_buffer = NULL;


  /****** Get params from SIM *******/
  cphs_status = CPHS_INITIALISING;

  cphs_cache_sim_data(NOT_PRESENT_8BIT, NOT_PRESENT_8BIT);

  return(CPHS_EXEC);
}


GLOBAL T_CPHS_RET cphs_refresh_data (void)
{
  T_CPHS_RET ret_status;

  TRACE_FUNCTION("cphs_refresh_data()");

  /* Check CPHS status */
  if((ret_status = cphs_check_status( )) NEQ CPHS_OK)
  {
    return(ret_status);
  }

  cphs_status = CPHS_REFRESHING;

  /****** Get params from SIM *******/
  cphs_cache_sim_data(NOT_PRESENT_8BIT, NOT_PRESENT_8BIT);

  return(CPHS_EXEC);
}

GLOBAL T_CPHS_RET cphs_stop (void)
{
  T_CPHS_RET ret_status;

  TRACE_FUNCTION("cphs_stop()");

  /* Check CPHS status */
  if((ret_status = cphs_check_status( )) NEQ CPHS_OK)
  {
    return(ret_status);
  }

  /* abort current action */
  cphs_abort_current_action( );

  /* reinitialise CPHS status */
  cphs_status = CPHS_NOT_INITIALISED;

  /* free memory allocated for CPHS */
  free_buffer(INFO_BUFF);
  free_buffer(ALS_BUFF);
  free_buffer(MB_BUFF);

  MFREE(cphs_cached_params);
  MFREE(cphs_internal_params);

  return(CPHS_OK);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CPHS                     |
| STATE   : code                  ROUTINE :                          |
+--------------------------------------------------------------------+

  PURPOSE : Functions related to CPHS information numbers.
*/

/*
  Returns elements contained in a folder.

  element_idx     Index of folder element to be explored
  inf_num_list    returns list of indexes of elements contained in folder
  max_elmts	      limits amount of bytes to be written in inf_num_indexes.
                  If numbers of elements to be written is bigger, then function returns
                  with CPHS_EXEC and amount of elements will be written in max_elmts
                  (so that function caller knows how many memory it has to allocate
                  to retrieve the whole list).     */

GLOBAL T_CPHS_RET cphs_explore_info_nbs(UBYTE element_idx, UBYTE *inf_num_indexes, UBYTE *max_elmts)
{
  T_CPHS_RET     ret_status;
  T_CPHS_INF_NUM *explored_folder,
                 *try_element;
  UBYTE          i = 0, index;

  TRACE_FUNCTION("cphs_explore_info_nbs()");

  /* Check CPHS status */
  if((ret_status = cphs_check_status( )) NEQ CPHS_OK)
  {
    /* CPHS status not initialized, set error as CME_ERR_OpNotAllow */
    ACI_ERR_DESC( ACI_ERR_CLASS_Cme, CME_ERR_OpNotAllow );
    return(ret_status);
  }
  else
    ret_status = CPHS_OK;   // unless we then find out buffer was too small...

  if(cphs_cached_params EQ NULL OR
     inf_num_indexes    EQ NULL OR
     max_elmts          EQ NULL)
  {
    TRACE_ERROR("cphs_explore_info_nbs: some needed pointers have value NULL");
    return(CPHS_FAIL);
  }

  /* init */
  *inf_num_indexes = 0;

  if(element_idx > cphs_cached_params->max_info_numbers)
  {
    TRACE_ERROR("wrong element_idx");
    return(CPHS_FAIL);
  }

  /* Check for info_numbers, set CME_ERR_NotFound return CPHS_FAIL */
  if (cphs_cached_params->info_numbers EQ NULL)
  {
    /* information numbers have not been read from SIM */
    *max_elmts = 0;
    ACI_ERR_DESC( ACI_ERR_CLASS_Cme, CME_ERR_NotFound );
    return (CPHS_FAIL);
  }

  explored_folder = &cphs_cached_params->info_numbers[element_idx];

  if(explored_folder->number[0] NEQ 0)
  {
    TRACE_EVENT_P1("Cannot explore: element %d is not a folder", element_idx);
    return(CPHS_FAIL);
  }

  index = element_idx + 1;
  try_element = &cphs_cached_params->info_numbers[index];

  while(try_element->index_level NEQ explored_folder->index_level) // search for next element of same index_level
  {
    if( try_element->index_level EQ (explored_folder->index_level+1) )  // element is direct under folder
    {
      if(i < *max_elmts)
      {
        inf_num_indexes[i] = index;
      }
      else
        ret_status = CPHS_EXEC; // buffer is too small: but we need to count how many bytes are then needed

      i++;
    }
    index++;
    try_element = &cphs_cached_params->info_numbers[index];

    if(index > cphs_cached_params->max_info_numbers)
    {
      break; /* end of list: can happen, e.g when exploring the last folder of a given index_level */
    }
  }

  *max_elmts = i;
  return(ret_status);
}

/*
  Returns information related to element id

  element_idx   Index of element chosen
  inf_num		    structure containing information related to an element */

GLOBAL T_CPHS_RET cphs_read_info_nb (UBYTE element_idx, T_CPHS_INF_NUM *inf_num)
{
  T_CPHS_RET ret_status;
  UBYTE i;
  BOOL flag = FALSE;

  TRACE_FUNCTION("cphs_read_info_nb()");

  /* Check CPHS status */
  if((ret_status = cphs_check_status( )) NEQ CPHS_OK)
  {
    /* CPHS status not initialized, set error as CME_ERR_OpNotAllow */
    ACI_ERR_DESC( ACI_ERR_CLASS_Cme, CME_ERR_OpNotAllow );
    return(ret_status);
  }

  if(cphs_cached_params EQ NULL)
  {
    TRACE_ERROR("cphs_cached_params EQ NULL");
    return(CPHS_FAIL);
  }
  
  /* Check Information numbers are cached or not */
  if(cphs_cached_params->info_numbers EQ NULL)
  {
    ACI_ERR_DESC( ACI_ERR_CLASS_Cme, CME_ERR_NotFound );
    return(CPHS_FAIL);    
  }
  
  if(element_idx < 1             OR
     element_idx > cphs_cached_params->max_info_numbers)
  {
    TRACE_ERROR("wrong element_idx");
    /* Element index not in the range, set CME_ERR_InvIdx */
    ACI_ERR_DESC( ACI_ERR_CLASS_Cme, CME_ERR_InvIdx );
    return(CPHS_FAIL);
  }

  /* The information numbers is presented to the user only when the information field is
     present in the CSP */

  for (i = 0; i < cphs_cached_params->csp_length; i += 2) 
  {
    if (cphs_cached_params->csp[i] EQ 0xD5 AND cphs_cached_params->csp[i+1] EQ 0xFF)
    {
      flag = 1;
    }
  }

  if (!flag)
  {
    ACI_ERR_DESC( ACI_ERR_CLASS_Cme, CME_ERR_OpNotAllow );
    return (CPHS_FAIL);
  }

  memcpy(inf_num, &cphs_cached_params->info_numbers[element_idx], sizeof(T_CPHS_INF_NUM));

  return(CPHS_OK);
}

GLOBAL T_CPHS_RET cphs_info_num_get_max(UBYTE *max_index)
{
  T_CPHS_RET ret_status;

  TRACE_FUNCTION("cphs_info_num_get_max()");

  /* Check CPHS status */
  if((ret_status = cphs_check_status( )) NEQ CPHS_OK)
  {
    /* CPHS status not initialized, set error as CME_ERR_OpNotAllow */
    ACI_ERR_DESC( ACI_ERR_CLASS_Cme, CME_ERR_OpNotAllow );
    return(ret_status);
  }

  if(cphs_cached_params NEQ NULL                             AND
     cphs_cached_params->max_info_numbers NEQ NOT_PRESENT_8BIT)
  {
    *max_index = cphs_cached_params->max_info_numbers;
    return(CPHS_OK);
  }
  
  ACI_ERR_DESC( ACI_ERR_CLASS_Cme, CME_ERR_NotFound );
  return(CPHS_FAIL);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CPHS                     |
| STATE   : code                  ROUTINE :                          |
+--------------------------------------------------------------------+

  PURPOSE : Functions related to CPHS alternate line service.
*/

/*
  Returns elements contained in a folder.

  call_id         call id of line being queried. This value is the same as the call id
                  described in GSM 02.30 subclause 4.5.5.1 (also see AT+CLCC). If not present
                  (value 255), then current active line is to be returned.
  line		        contains line of queried call, or current active line.
  line_desc	      line identification in SIM
  max_line_desc	  maximal length of line_desc:
                      IN: should contain the size of memory allocated for line_desc.
                      OUT: If memory is not enough, then function returns with CPHS_FAIL
                           and max_line_desc will contain the amount of memory needed.
                           Otherwise contains amount of written characters. */


GLOBAL T_CPHS_RET cphs_get_line(UBYTE srcId, UBYTE call_id, T_CPHS_LINES *line, 
                                CHAR *line_desc, UBYTE *max_line_desc)
{
  T_CPHS_RET      ret_status;
  T_CPHS_LINES    call_line;
  CHAR            *cached_line_name;

  TRACE_FUNCTION("cphs_get_line()");

  /* Check CPHS status */
  if((ret_status = cphs_check_status( )) NEQ CPHS_OK)
  {
    return(ret_status);
  }

  if(line EQ NULL)
  {
    TRACE_ERROR("wrong parameter: line is NULL");
    return(CPHS_FAIL);
  }

  if(call_id EQ NOT_PRESENT_8BIT)
  {
    /* Get current active line */
    cphs_get_als_active_line(srcId, &call_line);
  }
  else
  {
    /* get line associated to call_id */
    call_line = als_get_call_info(call_id);
  }

  if( call_line EQ CPHS_LINE_NULL )
  {
    TRACE_ERROR("als_get_call_info: not a valid call");
    return(CPHS_FAIL);
  }
  *line = call_line;

  if(max_line_desc EQ NULL)
  {
    TRACE_ERROR("max_line_desc EQ NULL");
    return(CPHS_FAIL);
  }

  if(*max_line_desc < CPHS_MAX_SIZE_ALPHA)
  {
    // buffer too small
    *max_line_desc = CPHS_MAX_SIZE_ALPHA;
    return(CPHS_EXEC);
  }

  get_name_pointer_over_lineid(call_line, &cached_line_name,
                               cphs_cached_params->max_als_names_entries);
  if(cached_line_name NEQ NULL)
  {
    memcpy(line_desc, cached_line_name, *max_line_desc);
    return(CPHS_OK);
  }
  else
  {
    memset(line_desc, 0, *max_line_desc);
    return(CPHS_FAIL);
  }
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CPHS                     |
| STATE   : code                  ROUTINE : cphs_roaming_ind         |
+--------------------------------------------------------------------+

  PURPOSE :
*/

GLOBAL void cphs_roaming_ind(UBYTE roaming_status)
{

  /* Check CPHS status */
  if((cphs_check_status()) NEQ CPHS_OK)
  {
    return;
  }

  TRACE_FUNCTION("cphs_roaming_ind()");

  /* inform user */
  cphs_inform_user(CPHS_ROAM_IND, CPHS_OK, roaming_status, CPHS_LINE_NULL);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CPHS                     |
| STATE   : code                  ROUTINE : cphs_voice_mail_ind      |
+--------------------------------------------------------------------+

  PURPOSE :
*/

GLOBAL void cphs_voice_mail_ind (UBYTE flag_set, USHORT line)
{

  /* Check CPHS status */
  if((cphs_check_status( )) NEQ CPHS_OK)
  {
    TRACE_ERROR("cannot proceed Voice Mail Indication: CPHS module is busy");
    return;
  }

  TRACE_FUNCTION("cphs_voice_mail_ind()");

  /* write to SIM and in cached data */
  cphs_status = CPHS_WRITING_VWI_IND;

  cphs_write_indicator_flag(flag_set, line, CPHS_SIM_VWI);

  return;
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CPHS                     |
| STATE   : code                  ROUTINE : cphs_set_waiting_flag    |
+--------------------------------------------------------------------+

  PURPOSE : set/clear waiting flag for given lines
*/

GLOBAL T_CPHS_RET cphs_set_waiting_flag(UBYTE flag_set, T_CPHS_LINES lines)
{
  T_CPHS_RET   ret_status;

  TRACE_FUNCTION("cphs_set_waiting_flag()");

  /* Check CPHS status */
  if((ret_status = cphs_check_status( )) NEQ CPHS_OK)
  {
    TRACE_ERROR("cannot proceed SET of Voice Mail Indicator: CPHS module is busy");
    ACI_ERR_DESC( ACI_ERR_CLASS_Cme, CME_ERR_OpNotAllow );
    return(ret_status);
  }

  cphs_status = CPHS_WRITING_VWI;

  cphs_write_indicator_flag(flag_set, lines, CPHS_SIM_VWI);

  return(CPHS_EXEC);
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CPHS                     |
| STATE   : code                  ROUTINE : cphs_get_waiting_flag    |
+--------------------------------------------------------------------+

  PURPOSE : look up if waiting flag for queried line is set
*/

GLOBAL T_CPHS_RET cphs_get_waiting_flag(UBYTE *flag_set, T_CPHS_LINES line)
{
  T_CPHS_RET ret_status;

  TRACE_FUNCTION("cphs_get_waiting_flag()");

  /* Check CPHS status */
  if((ret_status = cphs_check_status( )) NEQ CPHS_OK)
  {
    return(ret_status);
  }

  ret_status = cphs_get_indicator_flag(flag_set, line, CPHS_SIM_VWI);

  return(ret_status);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CPHS                     |
| STATE   : code                  ROUTINE : cphs_get_opn             |
+--------------------------------------------------------------------+

  PURPOSE : look up operator name string and if available operator
            name short string
*/

GLOBAL T_CPHS_RET cphs_get_opn( CHAR  *longname,
                                UBYTE *max_longname,
                                CHAR  *shortname,
                                UBYTE *max_shortname)

{
  UBYTE      len, i;
  T_CPHS_RET ret_status;
  BOOL       buffer_too_small = FALSE;

  TRACE_FUNCTION("cphs_get_opn()");

  /* Check CPHS status */
  if((ret_status = cphs_check_status( )) NEQ CPHS_OK)
  {
    ACI_ERR_DESC( ACI_ERR_CLASS_Cme, CME_ERR_OpNotAllow );
    return(ret_status);
  }

  /* Length of Long Operator Name */
  len = strlen((CHAR *)cphs_cached_params->opn_long);
  for(i=0; i<len; i++)
  {
    if(cphs_cached_params->opn_long[i] EQ 0xFF)
    {
      len = i;
      break;
    }
  }

  TRACE_EVENT_P1("opn_long length %d", len);
  
  if( len > *max_longname )
  {
    TRACE_EVENT_P1("buffer for long name is not big enough: needed: %d", len);
    *max_longname = len;
    buffer_too_small = TRUE;
  }
  else
  {
    *max_longname = len;
    memcpy(longname, cphs_cached_params->opn_long, (int) len);
    longname[len] = '\0';
  }

  /* Length of SHORT Operator Name */
  len = strlen((CHAR *)cphs_cached_params->opn_short);
  for(i=0; i<len; i++)
  {
    if(cphs_cached_params->opn_short[i] EQ 0xFF)
    {
      len = i;
      break;
    }
  }

  TRACE_EVENT_P1("opn_short length %d", len);

  if( len > *max_shortname )
  {
    TRACE_EVENT_P1("buffer for short name is not big enough: needed: %d", len);
    *max_shortname = len;
    buffer_too_small = TRUE;
  }
  else
  {
    *max_shortname = len;
    memcpy(shortname, cphs_cached_params->opn_short, (int) len);
    shortname[len] = '\0';
  }

  if(buffer_too_small)
  {
    return(CPHS_FAIL);
  }
  else
    return(CPHS_OK);
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CPHS                     |
| STATE   : code                  ROUTINE : cphs_get_cphs_info       |
+--------------------------------------------------------------------+

  PURPOSE : look up cphs info
*/

GLOBAL T_CPHS_RET cphs_get_cphs_info( UBYTE  *phase,
                                      USHORT *sst)
{ 
  T_CPHS_RET ret_status;

  TRACE_FUNCTION("cphs_get_cphs_info()");

  /* Check CPHS status */
  if((ret_status = cphs_check_status( )) NEQ CPHS_OK)
  {
    return(ret_status);
  }

  *phase = cphs_cached_params->info.phase;
  *sst   = cphs_cached_params->info.sst;
 
  return CPHS_OK;
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CPHS                     |
| STATE   : code                  ROUTINE : cphs_parse_csp           |
+--------------------------------------------------------------------+

  PURPOSE : Look up CSP information
*/
LOCAL T_CPHS_RET cphs_parse_csp ( UBYTE  *cached_csp,
                                  CHAR   *parsed_csp,
                                  UBYTE  cached_csp_length,
                                  UBYTE  *parsed_csp_length)
{ 
  UBYTE pos;

  TRACE_FUNCTION("cphs_parse_csp()");

  /* Cached CSP not present return CPHS_FAIL */
  if (cached_csp EQ NULL)
  {
    return(CPHS_FAIL);
  }

  /* analyze max size of csp hex string */
  if(*parsed_csp_length < (2*cached_csp_length + 1))
  {
    TRACE_EVENT_P1("buffer for CSP is too small: needed : %d", 
                    cached_csp_length);
    *parsed_csp_length = (2*cached_csp_length + 1);
    return(CPHS_FAIL);
  }

  /* convert to hex string */
  for(pos=0,*parsed_csp_length = 0; pos < cached_csp_length; pos++,*parsed_csp_length = pos*2)
  {
    sprintf(parsed_csp + (*parsed_csp_length), "%02X", cached_csp[pos]);
  }

  /* terminate string */
  parsed_csp[*parsed_csp_length] = 0;

  /* set actual size of csp hex string */
 (void) *parsed_csp_length++;
  
  return CPHS_OK;
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CPHS                     |
| STATE   : code                  ROUTINE : cphs_get_csprof          |
+--------------------------------------------------------------------+

  PURPOSE : look up cphs info
*/

GLOBAL T_CPHS_RET cphs_get_csprof( CHAR  *csp,
                                   CHAR  *csp2,
                                   UBYTE *max_csp_length,
                                   UBYTE *max_csp2_length)
{ 
  T_CPHS_RET ret_val = CPHS_OK;

  TRACE_FUNCTION("cphs_get_csprof()");

  ret_val = cphs_parse_csp( cphs_cached_params->csp,
                            csp,
                            cphs_cached_params->csp_length,
                            max_csp_length);

  if (ret_val EQ CPHS_OK)
  {
    ret_val = cphs_parse_csp( cphs_cached_params->orange_csp2,
                              csp2,
                              cphs_cached_params->orange_csp2_length,
                              max_csp2_length);
  }
  return ret_val;
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CPHS                     |
| STATE   : code                  ROUTINE : cphs_read_mb_number      |
+--------------------------------------------------------------------+

  PURPOSE : read mailbox number for given record id
*/

GLOBAL T_CPHS_RET cphs_read_mb_number( UBYTE     rec_id, 
                                       T_CPHS_MB *mailbox_entry)
{ 
  T_CPHS_RET ret_status;

  TRACE_FUNCTION("cphs_read_mb_number()");

  /* Check CPHS status */
  if((ret_status = cphs_check_status( )) NEQ CPHS_OK)
  {
    ACI_ERR_DESC( ACI_ERR_CLASS_Cme, CME_ERR_OpNotAllow);
    return(ret_status);
  }
  
  if(cphs_cached_params EQ NULL)
  {
    TRACE_ERROR("cphs_cached_params EQ NULL");
    ACI_ERR_DESC( ACI_ERR_CLASS_Cme, CME_ERR_Unknown);
    return CPHS_FAIL;
  }

  if ( (rec_id < 1) OR (rec_id > cphs_cached_params->max_mb_numbers) )
  {
    TRACE_ERROR("invalid record id, return CPHS_FAIL");
    ACI_ERR_DESC( ACI_ERR_CLASS_Cme, CME_ERR_InvIdx);
    return CPHS_FAIL;
  }

  if (cphs_cached_params->mb_numbers NEQ NULL)
  {
    memcpy(mailbox_entry, 
           &cphs_cached_params->mb_numbers[rec_id-1], 
           sizeof(T_CPHS_MB));
  }
  else
  { /* requested mb number has not been read from SIM, 
       return empty mb number */
    memset(mailbox_entry, 0, sizeof(T_CPHS_MB));
  }

  return CPHS_OK;
}

/*
+----------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : CPHS                 |
| STATE   : code                        ROUTINE : cphs_state_indication|  
+----------------------------------------------------------------------+

  PURPOSE : state the result to the user 
*/

GLOBAL SHORT cphs_state_indication ( UBYTE psaStatus, SHORT cmeError )
{
  UBYTE cmdBuf;
  UBYTE ownBuf;
    
  
  TRACE_FUNCTION ("cphs_state_indication()");

  cmdBuf = simEntStat.curCmd;
  ownBuf = simEntStat.entOwn;

  switch ( psaStatus )
  {
    case ( CPHS_SIM_WRITE_OK ): 
    {
      R_AT (RAT_OK,(T_ACI_CMD_SRC) ownBuf) (cmdBuf);
      break;
    }
    case ( CPHS_SIM_WRITE_FAIL ): 
    {
      TRACE_EVENT("cphs_state_indication: SIM write failed");
      R_AT( RAT_CME,(T_ACI_CMD_SRC) ownBuf)( cmdBuf, cmeError );
      break;
    }   
    default:
    {
      TRACE_EVENT("FATAL ERROR in cmhCHPS_StatIndication"); 
      return -1;
    }
  }

  /* trigger RAT indications to the user */
  /*
  for( idx = 0; idx < CMD_SRC_MAX; idx++ )
  {
    R_AT( RAT_PHB_STATUS, idx )( cmhStatus );
  }
  */
  simEntStat.curCmd = AT_CMD_NONE;
  simShrdPrm.owner  = (T_OWN)CMD_SRC_NONE ;
  simEntStat.entOwn = CMD_SRC_NONE;
  
  return 0;
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CPHS                     |
| STATE   : code                  ROUTINE : cphs_write_mb_number_cb  |
+--------------------------------------------------------------------+

  PURPOSE : write mailbox entry callback 
  
*/

GLOBAL void cphs_write_mb_number_cb( SHORT table_id )
{

  TRACE_FUNCTION("cphs_write_mb_number_cb()");

  simShrdPrm.atb[table_id].ntryUsdFlg = FALSE;

  if (simShrdPrm.atb[table_id].errCode NEQ SIM_NO_ERROR)
  {
    TRACE_ERROR("cphs_write_mb_number_cb(): error for writing");
    cphs_state_indication( CPHS_SIM_WRITE_FAIL,
                           (SHORT)cmhSIM_GetCmeFromSim( simShrdPrm.atb[table_id].errCode ));
    return;
  }
  else
  {  
    /* write to ME cache */
    write_mb_number(simShrdPrm.atb[table_id].recNr, 
                    exchData, 
                    cphs_cached_params->max_mb_entry_length);
    /* reset exchange data */
    memset(exchData, 0xFF, 100);
    cphs_state_indication ( CPHS_SIM_WRITE_OK, CME_ERR_NotPresent );
  }
  return;
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CPHS                     |
| STATE   : code                  ROUTINE : cphs_write_mb_number      |
+--------------------------------------------------------------------+

  PURPOSE : write mailbox number for given record id
*/

GLOBAL T_CPHS_RET cphs_write_mb_number( UBYTE         srcId,
                                        UBYTE         rec_id,
                                        UBYTE        *tag,
                                        UBYTE         tag_len,
                                        UBYTE         bcd_len,
                                        UBYTE        *number,
                                        UBYTE         ton_npi)
{ 
  UBYTE tmp_tag_len   = 0;
  UBYTE max_aplha_len = 0;
  UBYTE max_entry_len = 0;

  T_ACI_RETURN result;

  TRACE_FUNCTION("cphs_write_mb_number()");

  /* test state of CPHS */
  if(cphs_cached_params EQ NULL)
  {
    TRACE_ERROR("cphs_cached_params EQ NULL");
    return CPHS_NOT_INIT;
  }

  max_aplha_len = cphs_cached_params->max_mb_entry_length - CPHS_MIN_MB_LEN;
  max_entry_len = cphs_cached_params->max_mb_entry_length;

  /* test whether alpha tag is too long */
  if ( tag_len > max_aplha_len )
  {
    return CPHS_FAIL;
  }
  
  /* test if mb numbers are supported/initialized */
  if ((cphs_cached_params->mb_numbers NEQ NULL) AND
      ( rec_id <= cphs_cached_params->max_mb_numbers ))
  {
    
    /* write mb number to SIM - prepare data */     
    /* get length of alpha tag */    
    tmp_tag_len = pb_get_entry_len( tag, max_aplha_len );

    /* reset exchData */
    memset(exchData, 0xFF, DEFAULT_MAXSIZE_OF_RECORD);
    if ((number NEQ NULL)||(tag NEQ NULL)) /* NULL causes delete of record */
    {
      /* alpha tag */
      memcpy(exchData, tag, tmp_tag_len);
      /* length of bcd number content */
      exchData[max_aplha_len] = bcd_len+1;
      /* ton_npi */
      exchData[max_aplha_len+1] = ton_npi;
      /* BCD number */
      if (number NEQ NULL)
      {
        memcpy((char *)&exchData[max_aplha_len+2], 
            (char *)number, CPHS_MAX_MB_NUMBER_BYTES);
      }
      /* Capability field (empty) */
      exchData[max_aplha_len+12] = NOT_PRESENT_8BIT; 
    }
    /* write to SIM */    
    result= cmhSIM_WriteRecordEF( (T_ACI_CMD_SRC)srcId,
                                  AT_CMD_CPMBW,
                                  FALSE,
                                  NULL,
                                  SIM_CPHS_MBXN,
                                  rec_id,
                                  max_entry_len,
                                  exchData,
                                  cphs_write_mb_number_cb );    
    /* write to ME cache */
    if (result EQ AT_EXCT)
    {
      write_mb_number(rec_id, exchData, max_entry_len);
    }
  }
  else /* no mb supported */
  {
    return CPHS_NOT_INIT;
  }
  return CPHS_EXEC;
}


/*
+----------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : CPHS                 |
| STATE   : code                        ROUTINE : cphs_get_mb_parameter|  
+----------------------------------------------------------------------+

  PURPOSE : replies the CPHS mailbox parameters for test command 
*/

GLOBAL T_CPHS_RET cphs_get_mb_parameter (SHORT*        firstIdx,
                                         SHORT*        lastIdx,
                                         UBYTE*        nlength,
                                         UBYTE*        tlength )
{

  UBYTE max_entry_len = 0;
  UBYTE max_entries  = 0;
  
  TRACE_FUNCTION ("cphs_get_mb_parameter()");
  
  if(cphs_cached_params EQ NULL)
  {
    TRACE_ERROR("cphs_cached_params EQ NULL");
    return CPHS_NOT_INIT;
  }

  max_entries = cphs_cached_params->max_mb_numbers;
  max_entry_len = cphs_cached_params->max_mb_entry_length;

  /* handle index */
  if ( max_entries > 0 )
  {
    *firstIdx = (SHORT) CPHS_MIN_MB_ENTRIES;
    *lastIdx =  (SHORT) max_entries;
  }
  else
  {
    *lastIdx = *firstIdx = 0;
  }
  /* handle entry length */
  if ( max_entry_len >= CPHS_MIN_MB_LEN )
  {
    *tlength = max_entry_len - CPHS_MIN_MB_LEN;
  }
  else
  {
    *tlength = 0;
  }

  /* handle number length */
  
  *nlength = CPHS_MAX_MB_NUMBER_BYTES;

  return CPHS_OK;
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CPHS                     |
| STATE   : code                  ROUTINE : cphs_get_fwd_flag        |
+--------------------------------------------------------------------+

  PURPOSE : returns setting of CFU flag for given line
*/

#define SIM_FLAG_ON   (0x0A)
#define SIM_FLAG_OFF  (0x05)
#define SIM_FLAG_NONE (0x00)

/* if nibble = 0 (1st nibble)
#define GET_FLAG_VALUE(flag, nibble) ( (flag & (0xF0))  >>  4 )
if nibble=1 (2nd nibble)
#define GET_FLAG_VALUE(flag, nibble) (flag & 0x0F)
*/
#define GET_FLAG_VALUE(flag, nibble) ( (flag & (0x0F << (4 * (nibble ? 0 : 1))))  \
                                                               >>  (4 * (nibble ? 0 : 1)) )

LOCAL void get_relevant_flag_struct(UBYTE indicator, UBYTE **flag_table /*, UBYTE **temp_flag_table*/)
{
  UBYTE *intern_flag_table;

  switch(indicator)
  {
  case(CPHS_SIM_CFU):
    intern_flag_table      = cphs_cached_params->cfu_flags;
    break;

  case(CPHS_SIM_VWI):
    intern_flag_table = cphs_cached_params->vwi_flags;
    break;

  default:     /* shall be eventually removed ! */
    TRACE_ERROR("tttt: Wrong indicator value !!!");
    return;
  }

  if(flag_table NEQ NULL)
  {
    *flag_table = intern_flag_table;
  }
}

LOCAL void get_byte_index_and_nibble(T_CPHS_LINES line, UBYTE *byte_index, UBYTE *nibble)
{
  /* byte: nibble1/nibble2   (nibble1(LSB) has value 1, 
                               nibble2(MSB) has value 0) */
  /* first byte: line1/line2 (first  byte has value byte_index 0)*/
  /* second byte: fax/data   (second byte has value byte_index 1)*/
  switch(line)
  {
  case(CPHS_LINE1):
    *byte_index = 0;
    *nibble = 1;
    break;

  case(CPHS_LINE2):
    *byte_index = 0;
    *nibble = 0;
    break;
  
  case(CPHS_LINE_FAX):
    *byte_index = 1;
    *nibble = 1;
    break;

  case(CPHS_LINE_DATA):
    *byte_index = 1;
    *nibble = 0;
    break;
  }
}

LOCAL T_CPHS_RET cphs_get_indicator_flag(UBYTE *flag_set, T_CPHS_LINES line, UBYTE indicator)
{
  UBYTE      setting;
  UBYTE      byte_index, nibble;
  UBYTE      *flag_table;

  TRACE_FUNCTION("cphs_get_indicator_flag()");

  if(!cphs_line_makes_sense(line))
  {
    TRACE_EVENT_P1("Queried line %d does not exist", line);
    return(CPHS_FAIL);
  }

  get_byte_index_and_nibble(line, &byte_index, &nibble);

  get_relevant_flag_struct(indicator, &flag_table /*, NULL*/ );

  setting = GET_FLAG_VALUE(flag_table[byte_index], nibble);

  switch(setting)
  {
  case(SIM_FLAG_ON):
    *flag_set = CPHS_FLAG_ACTIVATED;
    break;
  
  case(SIM_FLAG_OFF):
    *flag_set = CPHS_FLAG_DEACTIVATED;
    break;
  
  case(SIM_FLAG_NONE):
    *flag_set = CPHS_FLAG_NOT_PRESENT;
    break;
  
  default:
    TRACE_EVENT_P1("cphs: get flag value unexpected: %02X", setting);
    *flag_set = CPHS_FLAG_ERROR;
    return(CPHS_FAIL);
  }

  TRACE_EVENT_P3("Get Indicator Flag: line: %d, setting: %X, flag_set: %d", line, setting, *flag_set);

  return(CPHS_OK);
}

GLOBAL T_CPHS_RET cphs_get_fwd_flag(UBYTE *cfu_set, T_CPHS_LINES line)
{
  T_CPHS_RET ret_status;

  TRACE_FUNCTION("cphs_get_fwd_flag()");

  /* Check CPHS status */
  if((ret_status = cphs_check_status( )) NEQ CPHS_OK)
  {
    return(ret_status);
  }

  return(cphs_get_indicator_flag(cfu_set, line, CPHS_SIM_CFU));
}


#define WRITE_FLAG_VALUE(flag, nibble, sim_state) \
                         (flag = (flag & (0x0F << (4 * nibble))) | \
                                 (sim_state  << (4 * (nibble ? 0 : 1)))) 
/*
#define WRITE_FLAG_VALUE(cfu, 0, state) (cfu = (cfu & 0x0F) | (state << 4))
#define WRITE_FLAG_VALUE(cfu, 1, state) (cfu = (cfu & 0xF0) | state)
*/

LOCAL void cphs_write_indicator_flag(UBYTE        flag_set, 
                                     T_CPHS_LINES lines, 
                                     UBYTE        indicator)
{
  UBYTE setting;
  UBYTE byte_index, nibble, table_length, i, sim_flag_status;
  UBYTE *flag_table;
  T_CPHS_LINES tested_line;
  T_CPHS_LINES really_written_lines;

  TRACE_FUNCTION("cphs_write_indicator_flag()");

  really_written_lines = CPHS_LINE_NULL; /* init */

  if(flag_set EQ CPHS_FLAG_ACTIVATED)
  {
    sim_flag_status = SIM_FLAG_ON;
  }
  else
    sim_flag_status = SIM_FLAG_OFF;

  get_relevant_flag_struct(indicator, &flag_table);

  /* init temp flags */
  memcpy(cphs_internal_params->tmp_flag_set, flag_table, FLAG_TABLE_SIZE);

  if(flag_table[1] EQ 0x00)
  {
    /* this means the optional FAX and DATA part is not present: write only 1st byte ! */
    table_length = 1;
  }
  else
  {
    table_length = FLAG_TABLE_SIZE;
  }

  for(i=0;i<8*sizeof(T_CPHS_LINES);i++)
  {
    tested_line = CPHS_LINE1 << i;
    
    if(tested_line & lines          AND
       cphs_line_makes_sense(tested_line) )
    {
      get_byte_index_and_nibble(tested_line, &byte_index, &nibble);

      setting = GET_FLAG_VALUE(flag_table[byte_index], nibble);

      /* Only fields with an expected value (A or 5) can be written... It is assumed
      here that other values mean the corresponding Indicator class is not relevant.
      Therefore it will be ignored */
      if(setting EQ SIM_FLAG_ON OR
         setting EQ SIM_FLAG_OFF)
      {
        WRITE_FLAG_VALUE(cphs_internal_params->tmp_flag_set[byte_index], 
                         nibble, sim_flag_status);
        really_written_lines |= tested_line;
      }
      else
      {
        TRACE_EVENT_P2("Unexpected value for this indicator: Writing ignored: line: %d, setting: %X", 
                       tested_line, setting);
      }
    }
  }

  /* This is what will be shown to the user when writing on SIM has been successful */
  cphs_internal_params->tmp_activate_state = flag_set;
  cphs_internal_params->tmp_lines          = really_written_lines; 
    
  cphs_sim_access_data( CPHS_SIM_WRITE_TRANSP_EF,
                        indicator,
                        0,
                        cphs_internal_params->tmp_flag_set,
                        table_length );
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CPHS                     |
| STATE   : code                  ROUTINE : cphs_set_cfu_flag        |
+--------------------------------------------------------------------+

  PURPOSE : set/clear cfu flag for given lines
*/

GLOBAL T_CPHS_RET cphs_set_cfu_flag(UBYTE cfu_set, T_CPHS_LINES lines)
{
  T_CPHS_RET ret_status;

  /* Check CPHS status */
  if((ret_status = cphs_check_status( )) NEQ CPHS_OK)
  {
    TRACE_ERROR("cannot proceed SET of Diverted Call Flag: CPHS module is busy");
    return(ret_status);
  }

  TRACE_FUNCTION("cphs_set_cfu_flag()");

  /* write to SIM and in cached data */
  cphs_status = CPHS_WRITING_CFU;

  cphs_write_indicator_flag(cfu_set, lines, CPHS_SIM_CFU);

  return(CPHS_EXEC);
}

GLOBAL BOOL cphs_line_makes_sense(T_CPHS_LINES line)
{
  switch(line)
  {
  case(CPHS_LINE1):
  case(CPHS_LINE2):
  case(CPHS_LINE_DATA):
  case(CPHS_LINE_FAX):
    return(TRUE);

  }

  return(FALSE);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CPHS                     |
| STATE   : code                  ROUTINE : cphs_write_csp_cb        |
+--------------------------------------------------------------------+

  PURPOSE : write CSP entry callback 
  
*/
GLOBAL void cphs_write_csp_cb(SHORT table_id)
{
  TRACE_FUNCTION("cphs_write_csp_cb()");

  simShrdPrm.atb[table_id].ntryUsdFlg = FALSE;

  if (simShrdPrm.atb[table_id].errCode NEQ SIM_NO_ERROR)
  {
    TRACE_ERROR("cphs_write_csp_cb(): error for writing");
    cphs_state_indication( CPHS_SIM_WRITE_FAIL,
                           (SHORT)cmhSIM_GetCmeFromSim( simShrdPrm.atb[table_id].errCode ));
  }
  else
  {  
    /* write to ME cache */
    write_csp_ok(exchData,
                 cphs_cached_params->csp_length);
    /* reset exchange data */
    memset(exchData, 0xFF, sizeof(exchData));
    cphs_state_indication ( CPHS_SIM_WRITE_OK, CME_ERR_NotPresent );
  }
}
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CPHS                     |
| STATE   : code                  ROUTINE : cphs_set_csp_value       |
+--------------------------------------------------------------------+

  PURPOSE : write customer service profile
*/
GLOBAL T_CPHS_RET cphs_set_csp_value(UBYTE srcId,
                                     UBYTE  *csp,
                                     UBYTE  csp_len)
{
  T_ACI_RETURN result;
  USHORT i,j;
  UBYTE max_csp_len = 0;
  TRACE_FUNCTION("cphs_set_csp_value()");

  /* test state of CPHS */
  if(cphs_cached_params EQ NULL)
  {
    TRACE_ERROR("cphs_cached_params EQ NULL");
    return CPHS_NOT_INIT;
  }
  
  /* set exchData and max csp length*/
  max_csp_len = cphs_cached_params->csp_length;
  memset(exchData, 0xFF, DEFAULT_MAXSIZE_OF_RECORD);
  memcpy(exchData, cphs_cached_params->csp, max_csp_len );

  /*
     only for valid service groups, associated services are written to SIM
   */
  if( csp NEQ NULL )
  {
    for( i=0; i < csp_len; i+=2 )
    {
      for( j=0; j < max_csp_len; j+=2 )
      {
        if ( csp[i] EQ exchData[j] )
        {
          exchData[j+1] = csp[i+1];
        }
      }
    }
  }

  /* write to SIM */
    result = cmhSIM_WriteTranspEF((T_ACI_CMD_SRC)srcId,
                                  AT_CMD_CPINF,
                                  FALSE,
                                  NULL,
                                  SIM_CPHS_CSP,
                                  0,
                                  max_csp_len,
                                  exchData,
                                  cphs_write_csp_cb );
    
  /* If error occured while writing to SIM */
  if (result EQ AT_FAIL)
  {
    return(CPHS_FAIL);
  }
  return(CPHS_EXEC);
}

#endif /* CPHS_C */
