/*
+-----------------------------------------------------------------------------
|  Project :  $Workfile::
|  Modul   :  CONC_SMS
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
|  Purpose :  SMS Concatenation Handler
+-----------------------------------------------------------------------------
*/

#ifndef CONC_SMS_C
#define CONC_SMS_C
#endif

/*==== INCLUDES ===================================================*/

#include "config.h"
#include "fixedconf.h"
#include "condat-features.h"
#include "aci_conf.h"

#include "aci_all.h"

#include "aci_cmh.h"
#include "ati_cmd.h"
#include "aci_cmd.h"

#include "aci_fd.h"
#include "aci_mem.h"

#include "psa.h"
#include "psa_sms.h"

#include "cmh.h"
#include "cmh_sms.h"

#include "psa_cc.h"

#include "typedefs.h"
#include "aci_lst.h"

#include "psa_util.h"
#include "conc_sms.h"

#ifdef _CONC_TESTING_
#include "aci_io.h"
#include "aci_mfw.h"
#endif

/*==== VARIABLES ==================================================*/

GLOBAL T_SM_ASSEMBLY assembly_list[MAX_BUF_ELEMS];
GLOBAL T_SEG_BUF     segBuf_list  [MAX_BUF_ELEMS];
GLOBAL T_CONC_BUF    concBuf_list [MAX_CONC_BUF_ELEMS];
LOCAL USHORT RefNum_Del = 0xFF;
LOCAL BOOL dFLAG = FALSE;
LOCAL CHAR Addres[MAX_SMS_ADDR_DIG];



/*==== FUNCTIONS ==================================================*/

LOCAL void concSMS_printConcatList ();
LOCAL USHORT concSMS_findMaxRefNum(void); // Marcus: Issue 872: 03/10/2002



/*
+--------------------------------------------------------------------+
| PROJECT :                     MODULE  : CONC_SMS                   |
| STATE   : code                ROUTINE : concSMS_findSeqNumSB       |
+--------------------------------------------------------------------+

  PURPOSE : find 'seq_num' in segment buffer
*/
LOCAL BOOL concSMS_findSeqNumSB ( UBYTE critrerium,
                                  void *elem )
{
  T_SEG_BUF_ELEM *compared = (T_SEG_BUF_ELEM *)elem;

  if ( compared->seq_num == critrerium )
    return TRUE;
  else
    return FALSE;
}


/*
+--------------------------------------------------------------------+
| PROJECT :                     MODULE  : CONC_SMS                   |
| STATE   : code                ROUTINE : concSMS_findSeqNumElemCB   |
+--------------------------------------------------------------------+

  PURPOSE : find 'seq_num' in concatenation buffer
*/
LOCAL BOOL concSMS_findSeqNumElemCB  ( UBYTE critrerium,
                                       void *elem )
{
  T_CONC_BUF_ELEM *compared = (T_CONC_BUF_ELEM *)elem;

  if ( compared->seq_num == critrerium )
    return TRUE;
  else
    return FALSE;
}


/*
+--------------------------------------------------------------------+
| PROJECT :                     MODULE  : CONC_SMS                   |
| STATE   : code                ROUTINE : concSMS_findRecNumElemCB   |
+--------------------------------------------------------------------+

  PURPOSE : find 'rec_num' in concatenation buffer
*/
LOCAL BOOL concSMS_findRecNumElemCB  ( UBYTE critrerium,
                                       void *elem )
{
  T_CONC_BUF_ELEM *compared = (T_CONC_BUF_ELEM *)elem;

  if ( compared->rec_num == critrerium )
    return TRUE;
  else
    return FALSE;
}

/*
+--------------------------------------------------------------------+
| PROJECT :                     MODULE  : CONC_SMS                   |
| STATE   : code                ROUTINE : concSMS_getAsBuffer        |
+--------------------------------------------------------------------+

  PURPOSE : This functions searchs the assembly buffer for ref_num
            and address.
*/
LOCAL T_SM_ASSEMBLY* concSMS_getAsBuffer( USHORT ref_num, CHAR *address )
{
  UBYTE i;

  TRACE_FUNCTION ("concSMS_getAsBuffer()");
 
  /* search for the element */
  for (i=0; i<MAX_BUF_ELEMS; i++)
  {
    if ( (assembly_list[i].ref_num EQ ref_num) AND
         (assembly_list[i].in_use) )
    {
      if ((address NEQ NULL) /* AND (address[0] NEQ '\0') */)
      {
        if (!strcmp(assembly_list[i].address, address))
        {
          return &assembly_list[i];
        }
      }
      else
      {
       return &assembly_list[i];
      }
    }
  }

  return NULL;
}


/*
+--------------------------------------------------------------------+
| PROJECT :                     MODULE  : CONC_SMS                   |
| STATE   : code                ROUTINE : concSMS_addToAsBuffer      |
+--------------------------------------------------------------------+

  PURPOSE : This function adds data to the assembly buffer. It returns
            NULL if buffer is full.

*/
LOCAL T_SM_ASSEMBLY* concSMS_addToAsBuffer ( USHORT ref_num,
                                             CHAR   *address,
                                             UBYTE  max_num,
                                             T_SM_DATA_EXT *data )
{
  UBYTE i;
  T_SM_ASSEMBLY *assembly_elem = NULL;

  TRACE_FUNCTION ("concSMS_addToAsBuffer()");


  /* search for the element */
  assembly_elem = concSMS_getAsBuffer( ref_num, address );

#ifdef _CONC_TESTING_
  TRACE_EVENT_P1("addToAsBuffer:[0].in_use: %d", assembly_list[0].in_use);
  TRACE_EVENT_P1("addToAsBuffer:[1].in_use: %d", assembly_list[1].in_use);
#endif

  /* element not found */
  if (assembly_elem EQ NULL)
  {
    /* search for an unused list entry */
    for (i=0; i<MAX_BUF_ELEMS; i++)
    {
      if (assembly_list[i].in_use EQ FALSE)
      {
        assembly_elem = &assembly_list[i];
        break;
      }
    }

    /* buffer is full */
    if (assembly_elem EQ NULL)
      return NULL;


    /* create new assembly buffer for this ref_num*/
    assembly_elem->in_use       = TRUE;
    assembly_elem->ref_num      = ref_num;

    if ( (address NEQ NULL) AND (address[0] NEQ '\0') )
      strcpy(assembly_elem->address, address);
    else
      assembly_elem->address[0] = '\0';

    assembly_elem->next_exp_num = 1;
    assembly_elem->segs_left    = max_num;
    assembly_elem->seg_count    = 0;
  } /* if (assembly_elem EQ NULL) */

  if (assembly_elem->seg_count EQ 0)
  {
    /* alloc memory for data to assemble */

    UBYTE segs;

    segs = MINIMUM(assembly_elem->segs_left, CONC_MAX_SEGS);
    ACI_MALLOC(assembly_elem->data.data, (USHORT)(MAX_SM_LEN*segs));
    assembly_elem->segs_left -= segs;
    assembly_elem->data.len = 0;
  }

  memcpy(assembly_elem->data.data+assembly_elem->data.len,
         data->data, data->len);

  assembly_elem->data.len += data->len;
  assembly_elem->data.data[assembly_elem->data.len] = '\0';
  assembly_elem->next_exp_num++;
  assembly_elem->seg_count++;

#ifdef _CONC_TESTING_
  if (assembly_elem->data.len < TTRACE_LEN)
  {
    TRACE_EVENT_P1("addToAsBuffer:data.data:    %s", assembly_elem->data.data);
  }
  TRACE_EVENT_P1("addToAsBuffer:data.len:     %d", assembly_elem->data.len);
  TRACE_EVENT_P1("addToAsBuffer:next_exp_num: %d", assembly_elem->next_exp_num);
  TRACE_EVENT_P1("addToAsBuffer:seg_count:    %d", assembly_elem->seg_count);
#endif

  return assembly_elem;
}


/*
+---------------------------------------------------------------------+
| PROJECT :                     MODULE  : CONC_SMS                    |
| STATE   : code                ROUTINE : concSMS_removeFromAsBuffer  |
+---------------------------------------------------------------------+

  PURPOSE :  This functions gets data from the assembly buffer and
             sets it to 'unused'. The assembly is completed.

*/
LOCAL UBYTE concSMS_removeFromAsBuffer(T_SM_DATA_EXT  *data_conc,
                                       USHORT         ref_num,
                                       CHAR           *address)
{
  T_SM_ASSEMBLY *assembly_buf;

  TRACE_FUNCTION ("concSMS_removeFromAsBuffer()");


  /* search for the element */
  assembly_buf = concSMS_getAsBuffer( ref_num, address );

  if (assembly_buf EQ NULL)
    return FALSE;

  assembly_buf->in_use = FALSE;

  data_conc->data = assembly_buf->data.data;
  data_conc->len  = assembly_buf->data.len;

  return TRUE;
}


/*
+---------------------------------------------------------------------+
| PROJECT :                     MODULE  : CONC_SMS                    |
| STATE   : code                ROUTINE : concSMS_getFromAsBuffer     |
+---------------------------------------------------------------------+

  PURPOSE :  This functions gets data from the assembly buffer and
             resets the seg_count. The assembly buffer is still in use
             and the assembly is not completed.

*/
LOCAL UBYTE concSMS_getFromAsBuffer(T_SM_DATA_EXT  *data_conc,
                                    USHORT         ref_num,
                                    CHAR           *address)
{
  T_SM_ASSEMBLY *assembly_buf;

  TRACE_FUNCTION ("concSMS_getFromAsBuffer()");


  /* search for the element */
  assembly_buf = concSMS_getAsBuffer( ref_num, address );

  /* assemlby buffer not found */
  if (assembly_buf EQ NULL)
    return FALSE;

  assembly_buf->seg_count = 0;

  data_conc->data = assembly_buf->data.data;
  data_conc->len  = assembly_buf->data.len;

  return TRUE;
}


/*
+--------------------------------------------------------------------+
| PROJECT :                     MODULE  : CONC_SMS                   |
| STATE   : code                ROUTINE : concSMS_getSegBuffer       |
+--------------------------------------------------------------------+

  PURPOSE : This functions searchs the segment buffer for ref_num
            and address.
*/
LOCAL T_SEG_BUF* concSMS_getSegBuffer( USHORT ref_num, CHAR *address )
{
  UBYTE i;

  TRACE_FUNCTION ("concSMS_getSegBuffer()");


  /* search for the element */
  for (i=0; i<MAX_BUF_ELEMS; i++)
  {
    if ((segBuf_list[i].ref_num EQ ref_num) AND
        (segBuf_list[i].in_use))
    {

      if ((address NEQ NULL) /* AND (address[0] NEQ '\0') */)
      {
        if (!strcmp(segBuf_list[i].address, address))
          return &segBuf_list[i];
      }
      else
      {
        return &segBuf_list[i];
      }
    }
  }

  return NULL;
}


/*
+--------------------------------------------------------------------+
| PROJECT :                     MODULE  : CONC_SMS                   |
| STATE   : code                ROUTINE : concSMS_addToSegBuffer     |
+--------------------------------------------------------------------+

  PURPOSE : This functions adds one segment to the buffer and returns
            FALSE if no segment buffer is available or the current
            seg buffer is full.

*/
LOCAL UBYTE concSMS_addToSegBuffer ( USHORT ref_num,
                                     CHAR  *address,
                                     UBYTE  seq_num,
                                     UBYTE  rec_num,
                                     T_ACI_SMS_STAT status,
                                     T_SM_DATA_EXT  *data )
{
  T_SEG_BUF *segBuf = NULL;
  T_SEG_BUF_ELEM *segBufElem;
  USHORT count;
  UBYTE i;

  TRACE_FUNCTION ("concSMS_addToSegBuffer()");


  /* search for the segment buffer */
  segBuf = concSMS_getSegBuffer( ref_num, address );

  /* element not found */
  if (segBuf EQ NULL)
  {
    /* search for an unused list entry */
    for (i=0; i<MAX_BUF_ELEMS; i++)
    {
      if (segBuf_list[i].in_use EQ FALSE)
      {
        segBuf = &segBuf_list[i];
        break;
      }
    }

    /* no segment buffer available */
    if ( segBuf EQ NULL)
      return FALSE;

    /* initialise new buffer */
    segBuf->in_use  = TRUE;
    segBuf->ref_num = ref_num;
    if ( (address) AND (address[0] NEQ '\0') )
      strcpy(segBuf->address, address);
    else
      segBuf->address[0]  = '\0';
    segBuf->list    = new_list();
  }

  count = get_list_count(segBuf->list);
  if ( count >= CONC_MAX_SEGS )
  {
    /* clean segment buffer before it overflows */
    while (1)
    {
      segBufElem = remove_first_element(segBuf->list);
      if (segBufElem EQ NULL)
        break;
      ACI_MFREE(segBufElem->data.data);
      ACI_MFREE(segBufElem);
    }
    segBuf->in_use = FALSE;
    return FALSE;
  }

  /* create new segment buffer element */
  ACI_MALLOC(segBufElem, sizeof(T_SEG_BUF_ELEM));
  memset(segBufElem, 0, sizeof(T_SEG_BUF_ELEM));

  /* fill new buffer element */
  segBufElem->seq_num = seq_num;
  segBufElem->rec_num = rec_num;
  segBufElem->status  = status;

  /* alloc memory and copy user data to segment buffer */
  ACI_MALLOC(segBufElem->data.data, data->len);
  segBufElem->data.len = data->len;
  memcpy(segBufElem->data.data, data->data, data->len);

  /* insert element (segment) into the segment buffer */
  insert_list(segBuf->list, segBufElem);

  return TRUE;
}


/*
+--------------------------------------------------------------------+
| PROJECT :                     MODULE  : CONC_SMS                   |
| STATE   : code                ROUTINE : concSMS_removeFromSegBuffer|
+--------------------------------------------------------------------+

  PURPOSE : This function finds and removes the segment with
            'seq_num' from the segment buffer.

*/
LOCAL T_SEG_BUF_ELEM* concSMS_removeFromSegBuffer ( USHORT ref_num,
                                                    CHAR*  address,
                                                    UBYTE  seq_num )
{
  T_SEG_BUF *segBuf = NULL;
  T_SEG_BUF_ELEM *segBufElem;

  USHORT count;

  TRACE_FUNCTION ("concSMS_removeFromSegBuffer()");


  /* search for the segment buffer */
  segBuf = concSMS_getSegBuffer( ref_num, address );


  /* segment buffer not found */
  if (segBuf EQ NULL)
    return NULL;

  segBufElem = remove_element(segBuf->list, seq_num, concSMS_findSeqNumSB);

  if (segBufElem EQ NULL)
  {
    return NULL; /* didn't find the segment buffer element for this seq_num */
  }

  count = get_list_count(segBuf->list);

  if (count EQ 0)
  {
    ACI_MFREE (segBuf->list);
    segBuf->list = NULL;
    segBuf->in_use = FALSE;
  }

  return segBufElem;

}


/*
+--------------------------------------------------------------------+
| PROJECT :                     MODULE  : CONC_SMS                   |
| STATE   : code                ROUTINE : concSMS_getConcBuffer      |
+--------------------------------------------------------------------+

  PURPOSE : This functions searchs the concatenations buffer for
            ref_num and address.
*/
LOCAL T_CONC_BUF* concSMS_getConcBuffer( USHORT ref_num, CHAR *address )
{
  UBYTE i;

  TRACE_FUNCTION ("concSMS_getConcBuffer()");

  /* search for the element */
  for (i=0; i<MAX_CONC_BUF_ELEMS; i++)
  {
    if ((concBuf_list[i].ref_num EQ ref_num) AND
        (concBuf_list[i].in_use))
    {
      if ((address NEQ NULL) /* AND (address[0] NEQ '\0') */)
      {
        if (!strcmp(concBuf_list[i].address, address))
        {
          return &concBuf_list[i];
        }
      }
      else
      {
        return &concBuf_list[i];
      }
    }
  }

  return NULL;
}


/*
+--------------------------------------------------------------------+
| PROJECT :                     MODULE  : CONC_SMS                   |
| STATE   : code                ROUTINE : concSMS_addToConcatList    |
+--------------------------------------------------------------------+

  PURPOSE :
*/
LOCAL BOOL concSMS_addToConcatList ( USHORT ref_num,
                                     CHAR   *address,
                                     UBYTE  max_num,
                                     UBYTE  seq_num,
                                     UBYTE  rec_num,
                                     T_ACI_SMS_STAT status,
                                     UBYTE  mem)
{
  T_CONC_BUF *concBuf;
  T_CONC_BUF_ELEM *concBufElem;
  UBYTE i;

  TRACE_FUNCTION ("concSMS_addToConcatList()");


  /* search for concatenation buffer */
  concBuf = concSMS_getConcBuffer( ref_num, address );


  /* element not found */
  if (concBuf EQ NULL)
  {
    
    /* search for an unused list entry */
    for (i=0; i<MAX_CONC_BUF_ELEMS; i++)
    {
      if (concBuf_list[i].in_use EQ FALSE)
      {
        concBuf = &concBuf_list[i];
        break;
      }
    }

    /* buffer is full */
    if ( concBuf EQ NULL)
      return FALSE;
    

    concBuf->in_use   = TRUE;
    concBuf->ref_num  = ref_num;

    if ( (address) AND (address[0] NEQ '\0') )
      strcpy(concBuf->address, address);
    else
      concBuf->address[0]  = '\0';

    concBuf->max_num  = max_num;
    concBuf->list     = new_list();
  }


  /* don't add elements with same seq_num to the Concatenation Buffer */
  concBufElem = find_element(concBuf->list,seq_num,concSMS_findSeqNumElemCB);
  if (concBufElem)
    return FALSE;
 


  /* create new conc. buffer element */
  ACI_MALLOC(concBufElem, sizeof(T_CONC_BUF_ELEM));

  /* increase total count of stored CSMS segments by 1*/
  concShrdPrm.elem_count++;

  concBufElem->seq_num = seq_num;
  concBufElem->rec_num = rec_num;
  concBufElem->status  = status;
  concBufElem->mem     = mem;

  /* insert element into the conc. buffer */
  insert_list(concBuf->list, concBufElem);

  concSMS_printConcatList();

  return TRUE;
}


/*
+---------------------------------------------------------------------+
| PROJECT :                     MODULE  : CONC_SMS                    |
| STATE   : code                ROUTINE : concSMS_removeFromConcatList|
+---------------------------------------------------------------------+

  PURPOSE : This function removes and FREES the memory for the element.
*/
LOCAL T_ACI_LIST *concSMS_removeFromConcatList ( USHORT ref_num,
                                          CHAR   *address,
                                          UBYTE  rec_num )
{
  T_CONC_BUF *concBuf = NULL;
  T_CONC_BUF_ELEM *concBufElem;
  USHORT count;

  TRACE_FUNCTION ("concSMS_removeFromConcatList()");


  /* search for concatenation buffer */
  concBuf = concSMS_getConcBuffer( ref_num, address );

  /* concatenation buffer not found */
  if (concBuf EQ NULL)
  {
    TRACE_EVENT_P1("conc_buf NULL: rec: %d", rec_num);
    return NULL;
  }

  concBufElem = remove_element(concBuf->list, rec_num, concSMS_findRecNumElemCB);

  if (concBufElem EQ NULL)
  {
    TRACE_EVENT_P1("concBufElem NULL: rec: %d", rec_num);
    return NULL;
  }

  /* free memory for this element */
  ACI_MFREE(concBufElem);

  /* decrease total count of stored CSMS segments by 1*/
  concShrdPrm.elem_count--;

  count = get_list_count(concBuf->list);

  if (count EQ 0)
  {
    ACI_MFREE (concBuf->list);
    concBuf->list = NULL;
    concBuf->in_use = FALSE;
    return NULL;
  }
  return concBuf->list;
}


/*
+---------------------------------------------------------------------+
| PROJECT :                     MODULE  : CONC_SMS                    |
| STATE   : code                ROUTINE : concSMS_sortConcatList      |
+---------------------------------------------------------------------+

  PURPOSE : This function sorts the concat. buffer acc. to its seq_num.
*/
LOCAL void concSMS_sortConcatList ( USHORT ref_num,
                                    CHAR   *address )
{
  T_CONC_BUF *concBuf = NULL;
  T_CONC_BUF_ELEM *concBufElem;
  UBYTE seq_num;
  UBYTE rec_num = 0;
  T_ACI_LIST *oldlist, *newlist, *current;
  USHORT count;

  TRACE_FUNCTION ("concSMS_sortConcatList()");


  /* search for concatenation buffer */
  concBuf = concSMS_getConcBuffer( ref_num, address );

  /* concatenation buffer not found */
  if (concBuf EQ NULL)
    return;

  newlist = new_list();
  oldlist = concBuf->list;

  count = get_list_count(oldlist);

  while (count)
  {
    seq_num = 255;
    current  = oldlist;
    while (current)
    {
      concBufElem = (T_CONC_BUF_ELEM*)current->msg;
      if ( concBufElem->seq_num < seq_num )
      {
        seq_num = concBufElem->seq_num;
        rec_num = concBufElem->rec_num;
      }
      current = current->next;
    }

    concBufElem = remove_element(oldlist, rec_num, concSMS_findRecNumElemCB);

    insert_list(newlist, concBufElem);

    count = get_list_count(oldlist);
  }
  if (concBuf->list)
  {
    ACI_MFREE (concBuf->list);
    concBuf->list = NULL;
  }
  concBuf->list = newlist;
}


/*
+--------------------------------------------------------------------+
| PROJECT :                     MODULE  : CONC_SMS                   |
| STATE   : code                ROUTINE : concSMS_split              |
+--------------------------------------------------------------------+

  PURPOSE : return TRUE if splitting was done, otherwise FALSE
*/
LOCAL UBYTE concSMS_split (  T_ACI_SM_DATA*  tar_data,
                             T_SM_DATA_EXT*  src_data,
                             UBYTE           alphabet,
                             T_EXT_CMS_CMD_ID   id)
{
#ifndef _SIMULATION_
    T_TIME  time_val; /* Used for input to random generator */
#endif

  TRACE_FUNCTION ("concSMS_split ()");

  if (alphabet EQ 0x00)
  {
    /* 7-bit data coding scheme */
    if (src_data->len <= concShrdPrm.l_uncomp7bit_data)
    {
      tar_data->len = (UBYTE)src_data->len;
      memcpy ( tar_data->data, src_data->data, tar_data->len );
      return FALSE;
    }
    else
    {
      tar_data->len = concShrdPrm.l_uncomp7bit_data_conc;
      concShrdPrm.max_sms_len = concShrdPrm.l_uncomp7bit_data_conc;
    }
  }
  else
  {
    /* 8-bit data coding scheme */
    if (src_data->len <= concShrdPrm.l_uncomp8bit_data)
    {
      tar_data->len = (UBYTE)src_data->len;
      memcpy ( tar_data->data, src_data->data, tar_data->len );
      return FALSE;
    }
    else
    {
      tar_data->len = concShrdPrm.l_uncomp8bit_data_conc;
      concShrdPrm.max_sms_len = concShrdPrm.l_uncomp8bit_data_conc;
    }
  }

  /* copy first segment to 'tar_data' */
  memcpy ( tar_data->data, src_data->data, tar_data->len );

  concShrdPrm.udh.ref_num = (UBYTE)concSMS_findMaxRefNum(); /* Marcus: Issue 872: 03/10/2002 */
  concShrdPrm.udh.ref_num++;
  
  concShrdPrm.udh.max_num = (src_data->len+(concShrdPrm.max_sms_len-1)) / concShrdPrm.max_sms_len;
  concShrdPrm.udh.seq_num = 1;

  if (id EQ CMGS_CONC)
  {
#ifndef _SIMULATION_
    vsi_t_time (VSI_CALLER &time_val);
    srand((USHORT) time_val);         /* initialize random generator */

    /* For every conc sms going out, generate a random reference number and  
     * send it. Also when power cycled it will generate a new random number.
     */
    concShrdPrm.udh.ref_num = (UBYTE)rand();
#endif
    concShrdPrm.specPrm.concCMGS.data.len   = src_data->len;
    concShrdPrm.specPrm.concCMGS.data.data  = src_data->data;
    concShrdPrm.specPrm.concCMGS.offset     = tar_data->len;
    return TRUE;
  }

  if (id EQ CMGW_CONC)
  {
    concShrdPrm.specPrm.concCMGW.data.len  = src_data->len;
    concShrdPrm.specPrm.concCMGW.data.data = src_data->data;
    concShrdPrm.specPrm.concCMGW.offset    = tar_data->len;
    return TRUE;
  }

  return FALSE;
}


/*
+--------------------------------------------------------------------+
| PROJECT :                     MODULE  : CONC_SMS                   |
| STATE   : code                ROUTINE : concSMS_fillUDH            |
+--------------------------------------------------------------------+

  PURPOSE :
*/
LOCAL void concSMS_fillUDH ( T_ACI_UDH_DATA* udh,
                             UBYTE ref_num,
                             UBYTE max_num,
                             UBYTE seq_num )
{
  /* fill user data header structure for 8-bit ref number */

  udh->len     = 0x05;

  /* Information Element Identifier */
  udh->data[0] = SMS_IEI_CONC_8BIT;

  /* Information Element Identifier Length */
  udh->data[1] = 0x03;

  /* Information Element Data */
  udh->data[2] = (UBYTE)(ref_num & 0x00FF);  /* since we use only 8-Bit ref number */
  udh->data[3] = max_num;
  udh->data[4] = seq_num;
}


#ifdef TI_PS_FF_CONC_SMS
/********************** Init Functions *********************************/


/*
+--------------------------------------------------------------------+
| PROJECT :                     MODULE  : CONC_SMS                   |
| STATE   : code                ROUTINE : concSMS_retrieveConcBuf    |
+--------------------------------------------------------------------+

  PURPOSE : This function searches for the concatenation buffer
            which has 'index' in its first element. Returns
            'CONC_ERROR' if index is not the first element or list is
            incomplete.

*/
LOCAL T_CONC_INIT_RETURN concSMS_retrieveConcBuf ( T_CONC_BUF **concBuf,
                                                   UBYTE        index,
                                                   UBYTE        mem)
{
  UBYTE i;
  T_CONC_BUF_ELEM *concBufElem;

  TRACE_FUNCTION ("concSMS_retrieveConcBuf ()");

  *concBuf = NULL;

  for (i=0; i<MAX_CONC_BUF_ELEMS; i++)
  {
    concSMS_printConcatList();
    /* find conc. buffer element for this rec number */
    concBufElem = find_element(concBuf_list[i].list,
                               index,
                               concSMS_findRecNumElemCB);

    /* element was found and check if memory type of the first segment 
     * equals to the set memory type (mem1 or mem2) 
     */
    if ((concBufElem NEQ NULL) AND (concBufElem->mem EQ mem))

    {
      break;
    }
  }

  if (concBufElem EQ NULL)
  {
    /* no concatenation handler needed */
    return CONC_NOT_NEEDED;
  }

  *concBuf = &concBuf_list[i];


  /* check if rec number is the first segment (with seq_num == 1) */
  if ( ( concBufElem->seq_num EQ 1 ) AND 
         ( smsShrdPrm.status EQ CMGD_DEL_INDEX ) )
  {
    *concBuf = &concBuf_list[i];
    return CONC_NEEDED;

  }
  else if( smsShrdPrm.status > CMGD_DEL_INDEX )
  {
      /* The below check needs to be changed for deleting all 
      the concatmessages in case of DELET FLAG > 0. */

      *concBuf = &concBuf_list[i];
      return CONC_NEEDED;
  }
  /* rec number is not the first element in conc. buffer
   * allow reading of incomplete segments and tread them like "normal" SMS
   */
  return CONC_NOT_NEEDED;
}



/*
+--------------------------------------------------------------------+
| PROJECT :                     MODULE  : CONC_SMS                   |
| STATE   : code                ROUTINE : concSMS_initSendFromMem    |
+--------------------------------------------------------------------+

  PURPOSE : This function initialises shared parameter for CMSS.

*/
GLOBAL T_CONC_INIT_RETURN concSMS_initSendFromMem ( T_ACI_CMD_SRC  srcId,
                                                    UBYTE         *index,
                                                    CHAR          *da,
                                                    T_ACI_TOA     *toda )
{
  T_CONC_BUF *concBuf    = NULL;
  T_CONC_BUF_ELEM* elem  = NULL;
  T_CONC_INIT_RETURN ret = CONC_ERROR;
  T_CONC_CMSS *prm       = &concShrdPrm.specPrm.concCMSS;


  TRACE_FUNCTION ("concSMS_initSendFromMem ()");

  ret = concSMS_retrieveConcBuf ( &concBuf, *index, smsShrdPrm.mem2);

  if (ret EQ CONC_ERROR)
  {
    /* Error: segment is not the first segment of the SM */
    return CONC_ERROR;
  }

  if (ret EQ CONC_NOT_NEEDED)
  {
    /* no conatenation handler needed */
    return CONC_NOT_NEEDED;
  }

  concShrdPrm.sentSegs = 0;
  concShrdPrm.srcId = srcId;

  if (da)
  {
    memcpy(prm->da, da, strlen(da));
    prm->da[strlen(da)] = '\0';
    prm->p_da = prm->da;
  }
  else
  {
    prm->p_da = NULL;
  }

  if (toda)
  {
    memcpy(&prm->toda, toda, sizeof(T_ACI_TOA));
    prm->p_toda = &prm->toda;
  }
  else
  {
    prm->p_toda = NULL;
  }

  /* save the first concatenated buffer element */
  prm->currConcBufListElem = concBuf->list;

  prm->skipStoSent = TRUE;

  /* skip segments with status SMS_STAT_StoSent */
  while (prm->currConcBufListElem)
  {
    elem = (T_CONC_BUF_ELEM*)prm->currConcBufListElem->msg;

    if (elem->status EQ SMS_STAT_StoSent)
    {
      prm->currConcBufListElem = prm->currConcBufListElem->next;
    }
    else
    {
      break;
    }
  }

  /*
   * All elements were set to SMS_STAT_StoSent. Assume that this message was
   * sent completly and should be sent for the second time.
   */
  if (prm->currConcBufListElem EQ NULL)
  {
    prm->skipStoSent = FALSE;

    /* save the first concatenated buffer element */
    prm->currConcBufListElem = concBuf->list;

    elem = (T_CONC_BUF_ELEM*)prm->currConcBufListElem->msg;
  }

  *index = elem ? elem->rec_num : NULL;

  if (elem NEQ NULL)
  {
    elem->status = SMS_STAT_StoSent;
  }

  return CONC_NEEDED;

}


/*
+--------------------------------------------------------------------+
| PROJECT :                     MODULE  : CONC_SMS                   |
| STATE   : code                ROUTINE : concSMS_initReadFromMem    |
+--------------------------------------------------------------------+

  PURPOSE : This function initialises shared parameter for CMGR.

*/
GLOBAL T_CONC_INIT_RETURN concSMS_initReadFromMem ( T_ACI_CMD_SRC  srcId,
                                               UBYTE          index,
                                               T_ACI_SMS_READ rdMode )
{
  T_CONC_BUF *concBuf;
  T_CONC_INIT_RETURN ret;

  TRACE_FUNCTION ("concSMS_initReadFromMem ()");

  ret = concSMS_retrieveConcBuf ( &concBuf, index, smsShrdPrm.mem2);

  if (ret EQ CONC_ERROR)
  {
    /* Error: segment is not the first segment of the SM */
    return CONC_ERROR;
  }

  if (ret EQ CONC_NOT_NEEDED)
  {
    /* no conatenation handler needed */
    return CONC_NOT_NEEDED;
  }

  concShrdPrm.srcId  = srcId;
  concShrdPrm.specPrm.concCMGR.rdMode = rdMode;

  /* save the second concatenated buffer element */
  concShrdPrm.specPrm.concCMGR.currConcBufListElem = concBuf->list->next;

  return CONC_NEEDED;

}


/*
+--------------------------------------------------------------------+
| PROJECT :                     MODULE  : CONC_SMS                   |
| STATE   : code                ROUTINE : concSMS_initDeleteFromMem  |
+--------------------------------------------------------------------+

  PURPOSE : This function initialises shared parameter for CMGD.

*/
GLOBAL T_CONC_INIT_RETURN concSMS_initDeleteFromMem ( T_ACI_CMD_SRC  srcId,
                                                 UBYTE          index )
{
  T_CONC_BUF *concBuf;
  T_CONC_INIT_RETURN ret;


  TRACE_FUNCTION ("concSMS_initDeleteFromMem ()");


  ret = concSMS_retrieveConcBuf ( &concBuf, index, smsShrdPrm.mem1);

  if (ret EQ CONC_ERROR)
  {
    /* Error: segment is not the first segment of the SM */
    return CONC_ERROR;
  }

  if (ret EQ CONC_NOT_NEEDED)
  {
    if (concBuf NEQ NULL)
    {
      RefNum_Del = concBuf->ref_num;
    }
    /*else if (*/
    else if (dFLAG EQ TRUE)
    {
     TRACE_EVENT("BUFFER FULL");
    }
    else 
    {
     RefNum_Del = 0xFF;
    }
    /* no conatenation handler needed */
    return CONC_NOT_NEEDED;
  }


  /* save the concatenation list */
  concShrdPrm.specPrm.concCMGD.currConcBufListElem = concBuf->list;

  concShrdPrm.specPrm.concCMGD.ref_num = concBuf->ref_num;

  concShrdPrm.specPrm.concCMGD.address = concBuf->address;

  concShrdPrm.srcId  = srcId;

  concShrdPrm.specPrm.concCMGD.error_count = 0;

  return CONC_NEEDED;
}


/*
+--------------------------------------------------------------------+
| PROJECT :                     MODULE  : CONC_SMS                   |
| STATE   : code                ROUTINE : concSMS_initSend           |
+--------------------------------------------------------------------+

  PURPOSE : This function initialises shared parameter for CMGS.

*/
GLOBAL T_CONC_INIT_RETURN concSMS_initSend (
                                T_ACI_SM_DATA*  tar_data,
                                T_ACI_UDH_DATA* udh,
                                T_ACI_CMD_SRC   srcId,
                                CHAR*           da,
                                T_ACI_TOA*      toda,
                                T_SM_DATA_EXT*  src_data,
                                CHAR*           sca,
                                T_ACI_TOA*      tosca,
                                SHORT           isReply,
                                UBYTE           alphabet )
{
  UBYTE ret;
  T_CONC_CMGS *prm = &concShrdPrm.specPrm.concCMGS;

  TRACE_FUNCTION ("concSMS_initSend ()");


  ret = concSMS_split ( tar_data, src_data, alphabet, CMGS_CONC );

  if ( ret EQ FALSE )
    return CONC_NOT_NEEDED;

  concShrdPrm.srcId = srcId;

  if (da)
  {
    memcpy(prm->da, da, strlen(da));
    prm->da[strlen(da)] = '\0';
    prm->p_da = prm->da;
  }
  else
  {
    prm->p_da = NULL;
  }
  if (toda)
  {
    memcpy(&prm->toda, toda, sizeof(T_ACI_TOA));
    prm->p_toda = &prm->toda;
  }
  else
  {
    prm->p_toda = NULL;
  }

  prm->data.len  = src_data->len;
  prm->data.data = src_data->data;

  if (sca)
  {
    memcpy(prm->sca, sca, strlen(sca));
    prm->sca[strlen(sca)] = '\0';
    prm->p_sca = prm->sca;
  }
  else
  {
    prm->p_sca = NULL;
  }
  if (tosca)
  {
    memcpy(&prm->tosca, tosca, sizeof(T_ACI_TOA));
    prm->p_tosca = &prm->tosca;
  }
  else
  {
    prm->p_tosca = NULL;
  }

  prm->isReply = isReply;
  prm->sent_bytes = 0;

  concShrdPrm.sentSegs = 0;

  /* fill user data header structure */
  concSMS_fillUDH ( udh,
                    concShrdPrm.udh.ref_num,
                    concShrdPrm.udh.max_num,
                    concShrdPrm.udh.seq_num );

  return CONC_NEEDED;
}


/*
+--------------------------------------------------------------------+
| PROJECT :                     MODULE  : CONC_SMS                   |
| STATE   : code                ROUTINE : concSMS_initStoreInMem     |
+--------------------------------------------------------------------+

  PURPOSE : This function initialises shared parameter for CMGW.

*/
GLOBAL T_CONC_INIT_RETURN concSMS_initStoreInMem (  T_ACI_SM_DATA* tar_data,
                                               T_ACI_UDH_DATA* udh,
                                               T_ACI_CMD_SRC  srcId,
                                               SHORT          index,
                                               CHAR*          address,
                                               T_ACI_TOA*     toa,
                                               T_ACI_SMS_STAT stat,
                                               UBYTE          msg_ref,
                                               T_SM_DATA_EXT* src_data,
                                               CHAR*          sca,
                                               T_ACI_TOA*     tosca,
                                               SHORT          isReply,
                                               UBYTE          alphabet )
{
  T_CONC_INIT_RETURN ret;
  T_CONC_CMGW *prm = &concShrdPrm.specPrm.concCMGW;

  TRACE_FUNCTION ("concSMS_initStoreInMem ()");


  ret = (T_CONC_INIT_RETURN)concSMS_split ( tar_data, src_data, alphabet, CMGW_CONC );

  if ( ret EQ FALSE )
  {
    return CONC_NOT_NEEDED;
  }

  concShrdPrm.srcId = srcId;

  if (address)
  {
    memcpy(prm->da, address, strlen(address));
    prm->da[strlen(address)] = '\0';
    prm->p_da = prm->da;
  }
  else
  {
    prm->p_da = NULL;
  }
  if (toa)
  {
    memcpy(&prm->toda, toa, sizeof(T_ACI_TOA));
    prm->p_toda = &prm->toda;
  }
  else
  {
    prm->p_toda = NULL;
  }

  if ( stat NEQ SMS_STAT_NotPresent)
  {
    prm->stat = stat;
  }
  else
  {
    prm->stat = SMS_STAT_StoUnsent;
  }

  prm->msg_ref = msg_ref;
  prm->data.len  = src_data->len;
  prm->data.data = src_data->data;

  if (sca)
  {
    memcpy(prm->sca, sca, strlen(sca));
    prm->sca[strlen(sca)] = '\0';
    prm->p_sca = prm->sca;
  }
  else
  {
    prm->p_sca = NULL;
  }
  if (tosca)
  {
    memcpy(&prm->tosca, tosca, sizeof(T_ACI_TOA));
    prm->p_tosca = &prm->tosca;
  }
  else
  {
    prm->p_tosca = NULL;
  }

  prm->isReply = isReply;

  /* fill user data header structure */
  concSMS_fillUDH ( udh,
                    concShrdPrm.udh.ref_num,
                    concShrdPrm.udh.max_num,
                    concShrdPrm.udh.seq_num );

  return CONC_NEEDED;
}


/*
+--------------------------------------------------------------------+
| PROJECT :                     MODULE  : CONC_SMS                   |
| STATE   : code                ROUTINE : concSMS_initCommand        |
+--------------------------------------------------------------------+

  PURPOSE : This function initialises shared parameter for CMGC.

*/
GLOBAL T_CONC_INIT_RETURN concSMS_initCommand ( T_ACI_CMD_SRC   srcId,
                                           SHORT           fo,
                                           SHORT           ct,
                                           SHORT           pid,
                                           SHORT           mn,
                                           CHAR*           da,
                                           T_ACI_TOA*      toda,
                                           T_ACI_CMD_DATA* data )
{
  T_CONC_CMGC *prm = &concShrdPrm.specPrm.concCMGC;

  TRACE_FUNCTION ("concSMS_initCommand ()");


  if ( ct NEQ COMMAND_TYPE_DELETE)
    return CONC_NOT_NEEDED;

  if ((mn < concShrdPrm.first_mr) OR
      (mn > concShrdPrm.first_mr + concShrdPrm.sentSegs-1))
    return CONC_NOT_NEEDED;


  if ( mn NEQ concShrdPrm.first_mr)
  {
    /* Error: segment is not the first segment of the SM */
    return CONC_ERROR;
  }
  else
  {
    concShrdPrm.srcId = srcId;

    prm->command_count = 0;
    prm->fo = (UBYTE)fo;
    prm->ct = (UBYTE)ct;
    prm->pid = (UBYTE)pid;

    if (da)
    {
      memcpy(prm->da, da, strlen(da));
      prm->da[strlen(da)] = '\0';
      prm->p_da = prm->da;
    }
    else
    {
      prm->p_da = NULL;
    }

    if (toda)
    {
      memcpy(&prm->toda, toda, sizeof(T_ACI_TOA));
      prm->p_toda = &prm->toda;
    }
    else
    {
      prm->p_toda = NULL;
    }

    ACI_MALLOC(prm->data.data, MAX_SM_CMD_LEN);
    memcpy ( prm->data.data, data->data, data->len );
    prm->data.len = data->len;

  }
  return CONC_NEEDED;
}


/********************** RAT Callback Fucntions ****************************/


GLOBAL void rConcSMS_PlusCMSS (UBYTE mr, UBYTE numSeg)
{
  UBYTE index;
  T_CONC_BUF_ELEM* elem;
  T_CONC_CMSS *prm = &concShrdPrm.specPrm.concCMSS;

  TRACE_FUNCTION ("rConcSMS_PlusCMSS()");


  /* save the first message reference */
  if (concShrdPrm.sentSegs EQ 0)
  {
    concShrdPrm.first_mr = mr;
  }

  /* increment number of successfully sent elements */
  concShrdPrm.sentSegs++;

  /* get next concat. list element */
  prm->currConcBufListElem = prm->currConcBufListElem->next;

  if (prm->skipStoSent)
  {
    /* skip segments with status SMS_STAT_StoSent */
    while (prm->currConcBufListElem)
    {
      elem = (T_CONC_BUF_ELEM*)prm->currConcBufListElem->msg;

      if (elem->status EQ SMS_STAT_StoSent)
      {
        prm->currConcBufListElem = prm->currConcBufListElem->next;
      }
      else
      {
        break;
      }
    } /* while */
  }

  if (prm->currConcBufListElem NEQ NULL)
  {
    elem = (T_CONC_BUF_ELEM*)prm->currConcBufListElem->msg;
    index = elem->rec_num;

    /* set mem2 (memory to which writing and sending operations are made)
       temporary to the value stored in conc buffer */
    smsShrdPrm.mem2 = elem->mem;

    sAT_PlusCMSS_Gl((T_ACI_CMD_SRC)concShrdPrm.srcId, index, prm->p_da, prm->p_toda,
                    rConcSMS_PlusCMSS, rConcSMS_PlusCMS_CMSS);


    elem->status = SMS_STAT_StoSent;


  }
  else /* if (concShrdPrm.currConcBufListElem NEQ NULL) */
  {

#ifdef _CONC_TESTING_
    char *sa;
    ACI_MALLOC(sa,KEY + BYTE_LTH);
    sprintf(sa,"+CMSS: %d,%d",concShrdPrm.first_mr, concShrdPrm.sentSegs);
    io_sendMessage(concShrdPrm.srcId, sa, ATI_NORMAL_OUTPUT);
    ACI_MFREE(sa);
#endif

    rAT_PlusCMSS(concShrdPrm.first_mr, (UBYTE)(concShrdPrm.sentSegs));

     /* restore value for mem2 */
    smsShrdPrm.mem2 = concShrdPrm.mem_store;

    R_AT ( RAT_OK, (T_ACI_CMD_SRC) concShrdPrm.srcId ) ( AT_CMD_CMSS );
    UNSET_CONC;
  }
}

GLOBAL void rConcSMS_PlusCMGS (UBYTE mr, UBYTE numSeg)
{
  T_ACI_SM_DATA data;
  T_ACI_UDH_DATA udh;
  USHORT len_left;
  T_CONC_CMGS *prm = &concShrdPrm.specPrm.concCMGS;

  TRACE_FUNCTION ("rConcSMS_PlusCMGS()");


  /* save the first message reference */
  if (concShrdPrm.udh.seq_num EQ 1)
  {
    concShrdPrm.first_mr = mr;
  }

  /* increment number of successfully sent elements */
  len_left = prm->data.len - prm->offset;

  concShrdPrm.sentSegs++;

  if (len_left NEQ 0)
  {
    prm->sent_bytes += concShrdPrm.max_sms_len;

    if ( len_left > concShrdPrm.max_sms_len )
    {
      data.len  = concShrdPrm.max_sms_len;
    }
    else
    {
      data.len = (UBYTE)len_left;
    }

    memcpy (data.data, prm->data.data+prm->offset, data.len);
    prm->offset += data.len;

    concShrdPrm.udh.seq_num++;


    /* fill user data header structure */
    concSMS_fillUDH ( &udh,
                      concShrdPrm.udh.ref_num,
                      concShrdPrm.udh.max_num,
                      concShrdPrm.udh.seq_num );

    sAT_PlusCMGS_Gl((T_ACI_CMD_SRC)concShrdPrm.srcId, prm->p_da, prm->p_toda,
                    &data, &udh, prm->p_sca, prm->p_tosca,
                    prm->isReply, rConcSMS_PlusCMGS, rConcSMS_PlusCMS_CMGS);


  }
  else
  {

#ifdef _CONC_TESTING_
    char *sa;
    ACI_MALLOC(sa,KEY + BYTE_LTH);
    sprintf(sa,"+CMGS: %d,%d",concShrdPrm.first_mr, concShrdPrm.udh.seq_num);
    io_sendMessage(concShrdPrm.srcId, sa, ATI_NORMAL_OUTPUT);
    ACI_MFREE(sa);

    ACI_MFREE(prm->data.data);

#endif

    rAT_PlusCMGS (concShrdPrm.first_mr, (UBYTE)(concShrdPrm.udh.seq_num));
    R_AT ( RAT_OK, (T_ACI_CMD_SRC)concShrdPrm.srcId ) ( AT_CMD_CMGS );
    UNSET_CONC;
  }
}


GLOBAL void rConcSMS_PlusCMGR ( T_ACI_CMGL_SM*  sm,
                                T_ACI_CMGR_CBM* cbm )
{
  T_CONC_CMGR *prm = &concShrdPrm.specPrm.concCMGR;

  TRACE_FUNCTION ("rConcSMS_PlusCMGR ()");

  if (prm->currConcBufListElem NEQ NULL)
  {
    T_CONC_BUF_ELEM *elem;
    elem = (T_CONC_BUF_ELEM *)prm->currConcBufListElem->msg;

    /* set mem1 (memory from which messages are read and deleted)
    * temporary to the value stored in conc buffer */
    smsShrdPrm.mem1 = elem->mem;

    sAT_PlusCMGR_Gl((T_ACI_CMD_SRC)concShrdPrm.srcId, elem->rec_num,
                    (T_ACI_SMS_READ)concShrdPrm.specPrm.concCMGR.rdMode,
                    rConcSMS_PlusCMGR);

    prm->currConcBufListElem = prm->currConcBufListElem->next;

#ifdef _CONC_TESTING_
  rAT_PlusCMGR_Ext (sm, cbm);
#else
  rAT_PlusCMGR (sm, cbm);
#endif

  }
  else /* if (concShrdPrm.currConcBufListElem NEQ NULL) */
  {
#ifdef _CONC_TESTING_
  rAT_PlusCMGR_Ext (sm, cbm);
#else
  rAT_PlusCMGR (sm, cbm);
#endif

    /* restore value for mem1 */
    smsShrdPrm.mem1 = concShrdPrm.mem_store;


  R_AT ( RAT_OK, (T_ACI_CMD_SRC) concShrdPrm.srcId ) ( AT_CMD_CMGR );
    UNSET_CONC;
  }
}

GLOBAL void rConcSMS_PercentCMGMDU (void)
{
  T_CONC_CMGR *prm = &concShrdPrm.specPrm.concCMGR;

  TRACE_FUNCTION ("rConcSMS_PercentCMGMDU ()");

  if (prm->currConcBufListElem NEQ NULL)
  {
    T_CONC_BUF_ELEM *elem;
    elem = (T_CONC_BUF_ELEM *)prm->currConcBufListElem->msg;

    sAT_PercentCMGMDU_Gl((T_ACI_CMD_SRC)concShrdPrm.srcId, elem->rec_num,
                         rConcSMS_PercentCMGMDU);

    prm->currConcBufListElem = prm->currConcBufListElem->next;

  }
  else /* if (concShrdPrm.currConcBufListElem NEQ NULL) */
  {
    if( concShrdPrm.srcId NEQ CMD_SRC_LCL )
      R_AT ( RAT_OK, (T_ACI_CMD_SRC) concShrdPrm.srcId ) ( AT_CMD_P_CMGMDU );
    UNSET_CONC;
  }
}

GLOBAL void rConcSMS_PlusCMGW ( UBYTE index, UBYTE numSeg, UBYTE mem)
{
  T_ACI_SM_DATA data;
  T_ACI_UDH_DATA udh;
  USHORT len_left;
  T_CONC_CMGW *prm = &concShrdPrm.specPrm.concCMGW;

  static UBYTE first_rec;

  TRACE_FUNCTION ("rConcSMS_PlusCMGW ()");


  /* save the first index */
  if (concShrdPrm.udh.seq_num EQ 1)
  {
    first_rec = index;
  }

  concSMS_addToConcatList((USHORT)concShrdPrm.udh.ref_num,
                                  prm->p_da,
                                  concShrdPrm.udh.max_num,
                                  concShrdPrm.udh.seq_num,
                                  index,
                                  (T_ACI_SMS_STAT)prm->stat,
                                  mem);

  concSMS_printConcatList();

  len_left = prm->data.len - prm->offset;

  if (len_left NEQ 0)
  {
    prm->sent_bytes += concShrdPrm.max_sms_len;

    if ( len_left > concShrdPrm.max_sms_len )
    {
      data.len  = concShrdPrm.max_sms_len;
    }
    else
    {
      data.len = (UBYTE)len_left;
    }

    memcpy (data.data, prm->data.data+prm->offset, data.len);
    prm->offset += data.len;

    concShrdPrm.udh.seq_num++;

    /* fill user data header structure */
    concSMS_fillUDH ( &udh,
                      concShrdPrm.udh.ref_num,
                      concShrdPrm.udh.max_num,
                      concShrdPrm.udh.seq_num );

    sAT_PlusCMGW_Gl((T_ACI_CMD_SRC)concShrdPrm.srcId, CMGW_IDX_FREE_ENTRY, 
		    prm->p_da, prm->p_toda, (T_ACI_SMS_STAT)prm->stat, prm->msg_ref,
                    &data, &udh, prm->p_sca, prm->p_tosca,
                    prm->isReply, rConcSMS_PlusCMGW, rConcSMS_PlusCMS_CMGW);

  }
  else
  {

#ifdef _CONC_TESTING_
    char *sa;
    ACI_MALLOC(sa,KEY + BYTE_LTH);
    sprintf(sa,"+CMGW: %d,%d",first_rec, concShrdPrm.udh.seq_num);
    io_sendMessage(concShrdPrm.srcId, sa, ATI_NORMAL_OUTPUT);
    ACI_MFREE(sa);
#endif

    rAT_PlusCMGW (first_rec, concShrdPrm.udh.seq_num, mem);
    R_AT ( RAT_OK, (T_ACI_CMD_SRC)concShrdPrm.srcId ) ( AT_CMD_CMGW );
    UNSET_CONC;
  }
}


GLOBAL void rConcSMS_PlusCMGD ( )
{
  T_CONC_CMGD *prm = &concShrdPrm.specPrm.concCMGD;
  T_CONC_BUF_ELEM *elem;
  T_ACI_LIST *conc_list;

  TRACE_FUNCTION ("rConcSMS_PlusCMGD ()");


  elem = (T_CONC_BUF_ELEM *)prm->currConcBufListElem->msg;

  /* remove the old element from concatenation list and free its memory */
  conc_list = concSMS_removeFromConcatList(prm->ref_num, prm->address, elem->rec_num);

  concSMS_printConcatList();

  if (conc_list NEQ NULL)
  {
    TRACE_EVENT("conc_list not null");
    elem = (T_CONC_BUF_ELEM *)conc_list->msg;

    /* save the concatenation list */
    prm->currConcBufListElem= conc_list;
    
    /* set mem1 (memory from which messages are read and deleted)
       temporary to the value stored in conc buffer */
    smsShrdPrm.mem1 = elem->mem;

    sAT_PlusCMGD_Gl((T_ACI_CMD_SRC)concShrdPrm.srcId, elem->rec_num, smsShrdPrm.status,
                          rConcSMS_PlusCMGD, rConcSMS_PlusCMS_CMGD);

  }
  else /* if (concShrdPrm.currConcBufListElem NEQ NULL) */
  {
    if (concShrdPrm.full.Conc_Full EQ TRUE)
    {
      concSMS_AddtoconcBuff();
      concShrdPrm.full.Conc_Full = FALSE;
    }

    /* restore value for mem1 */
    smsShrdPrm.mem1 = concShrdPrm.mem_store;

    TRACE_EVENT("RAT_OK in rConcSMS_PlusCMGD");
    R_AT ( RAT_OK, (T_ACI_CMD_SRC) concShrdPrm.srcId ) ( AT_CMD_CMGD );
    UNSET_CONC;
  }
}


GLOBAL void rConcSMS_PlusCMGC ( UBYTE mr )
{
  UBYTE mn;
  T_CONC_CMGC *prm = &concShrdPrm.specPrm.concCMGC;

  TRACE_FUNCTION ("rConcSMS_PlusCMGC ()");

  /* save the first message reference */
  if (concShrdPrm.udh.seq_num EQ 1)
  {
    concShrdPrm.first_mr = mr;
  }

  prm->command_count++;

  if (prm->command_count < concShrdPrm.sentSegs)
  {
    mn = concShrdPrm.first_mr + prm->command_count;

    sAT_PlusCMGC_Gl((T_ACI_CMD_SRC)concShrdPrm.srcId, prm->fo, prm->ct,
                   prm->pid, mn, prm->p_da, prm->p_toda,
                   (T_ACI_CMD_DATA*)&prm->data, rConcSMS_PlusCMGC);
  }
  else
  {

#ifdef _CONC_TESTING_
    char *sa;
    ACI_MALLOC(sa,KEY + BYTE_LTH);
    sprintf(sa,"+CMGC: %d",concShrdPrm.first_mr /*, prm->command_count*/);
    io_sendMessage(concShrdPrm.srcId, sa, ATI_NORMAL_OUTPUT);
    ACI_MFREE(sa);
#endif

    ACI_MFREE( prm->data.data );
    rAT_PlusCMGC (concShrdPrm.first_mr/*, prm->command_count*/);
    R_AT ( RAT_OK,(T_ACI_CMD_SRC) concShrdPrm.srcId ) ( AT_CMD_CMGC );
    UNSET_CONC;
  }
}


GLOBAL void rConcSMS_PlusCMS_CMSS (T_ACI_AT_CMD cmdId, T_ACI_CMS_ERR err,
                                   T_EXT_CMS_ERROR *ce)
{
  T_EXT_CMS_ERROR conc_error;
#ifdef _CONC_TESTING_
    char *sa;
#endif

  TRACE_FUNCTION ("rConcSMS_PlusCMS_CMSS ()");


  conc_error.id = CMSS_CONC;
  conc_error.specErr.errConcCMSS.segs =
    concShrdPrm.udh.max_num - concShrdPrm.sentSegs;

#ifdef _CONC_TESTING_
  ACI_MALLOC(sa,KEY + BYTE_LTH);
  sprintf(sa,"+CMS ERROR: %d,%d",err, conc_error.specErr.errConcCMSS.segs);
  io_sendMessage(concShrdPrm.srcId, sa, ATI_NORMAL_OUTPUT);
  ACI_MFREE(sa);
  rCI_PlusCMS ( cmdId, err, NULL );
#endif

  /* restore value for mem2 */
  smsShrdPrm.mem2 = concShrdPrm.mem_store;

  rAT_PlusCMS (cmdId, err, &conc_error);
  UNSET_CONC;
}


GLOBAL void rConcSMS_PlusCMS_CMGS (T_ACI_AT_CMD cmdId, T_ACI_CMS_ERR err,
                                   T_EXT_CMS_ERROR *ce)
{
  T_EXT_CMS_ERROR conc_error;
#ifdef _CONC_TESTING_
    char *sa;
#endif

  TRACE_FUNCTION ("rConcSMS_PlusCMS_CMGS ()");


  conc_error.id = CMGS_CONC;
  conc_error.specErr.errConcCMGS.sent_chars =
    concShrdPrm.specPrm.concCMGS.sent_bytes;
  conc_error.specErr.errConcCMGS.ref_num    = concShrdPrm.udh.ref_num;
  conc_error.specErr.errConcCMGS.next_seg   = concShrdPrm.udh.seq_num;
  conc_error.specErr.errConcCMGS.max_num    = concShrdPrm.udh.max_num;

#ifdef _CONC_TESTING_
  ACI_MALLOC(sa,KEY + BYTE_LTH);
  sprintf(sa,"+CMS ERROR: %d,%d,%d,%d,%d",err,
                          conc_error.specErr.errConcCMGS.sent_chars,
                          conc_error.specErr.errConcCMGS.ref_num,
                          conc_error.specErr.errConcCMGS.next_seg,
                          conc_error.specErr.errConcCMGS.max_num);

  io_sendMessage(concShrdPrm.srcId, sa, ATI_NORMAL_OUTPUT);
  ACI_MFREE(sa);
  rCI_PlusCMS ( cmdId, err, NULL );
#endif

  rAT_PlusCMS (cmdId, err, &conc_error);
  UNSET_CONC;
}


GLOBAL void rConcSMS_PlusCMS_CMGW (T_ACI_AT_CMD cmdId, T_ACI_CMS_ERR err,
                                   T_EXT_CMS_ERROR *ce)
{
  T_EXT_CMS_ERROR conc_error;
#ifdef _CONC_TESTING_
  char *sa;
#endif

  TRACE_FUNCTION ("rConcSMS_PlusCMS_CMGW ()");


  conc_error.id = CMGW_CONC;
  conc_error.specErr.errConcCMGW.sent_chars =
    concShrdPrm.specPrm.concCMGW.sent_bytes;
  conc_error.specErr.errConcCMGW.ref_num    = concShrdPrm.udh.ref_num;
  conc_error.specErr.errConcCMGW.next_seg   = concShrdPrm.udh.seq_num;
  conc_error.specErr.errConcCMGW.max_num    = concShrdPrm.udh.max_num;

#ifdef _CONC_TESTING_
  ACI_MALLOC(sa,KEY + BYTE_LTH);
  sprintf(sa,"+CMS ERROR: %d,%d,%d,%d,%d",err,
                          conc_error.specErr.errConcCMGW.sent_chars,
                          conc_error.specErr.errConcCMGW.ref_num,
                          conc_error.specErr.errConcCMGW.next_seg,
                          conc_error.specErr.errConcCMGW.max_num);
  io_sendMessage(concShrdPrm.srcId, sa, ATI_NORMAL_OUTPUT);
  ACI_MFREE(sa);
  rCI_PlusCMS ( cmdId, err, NULL );
#endif

  rAT_PlusCMS (cmdId, err, &conc_error);
  UNSET_CONC;
}

GLOBAL void rConcSMS_PlusCMS_CMGD (T_ACI_AT_CMD cmdId, T_ACI_CMS_ERR err,
                                   T_EXT_CMS_ERROR *ce)
{
  T_CONC_CMGD *prm = &concShrdPrm.specPrm.concCMGD;
  T_CONC_BUF_ELEM *elem;
  T_EXT_CMS_ERROR conc_error;

  TRACE_FUNCTION ("rConcSMS_PlusCMS_CMGD ()");

  conc_error.id = EMPTY;

  prm->error_count++;
  if (prm->error_count EQ concShrdPrm.udh.max_num)
  {

#ifdef _CONC_TESTING_
    char *sa;
    ACI_MALLOC(sa,KEY + BYTE_LTH);
    sprintf(sa,"+CMS ERROR: %d",err);
    io_sendMessage(concShrdPrm.srcId, sa, ATI_NORMAL_OUTPUT);
    ACI_MFREE(sa);
    rCI_PlusCMS ( cmdId, err, NULL );
#endif

    elem = (T_CONC_BUF_ELEM *)prm->currConcBufListElem->msg;
    /* remove the old element from concatenation list and free its memory */
    concSMS_removeFromConcatList(prm->ref_num, prm->address, elem->rec_num);
    concSMS_printConcatList();

    /* restore value for mem1 */
    smsShrdPrm.mem1 = concShrdPrm.mem_store;

    rAT_PlusCMS ( cmdId, err, &conc_error);
    UNSET_CONC;
  }
  else
  {
    /* continue with the next segment */
    rConcSMS_PlusCMGD();
  }
}
#endif /* TI_PS_FF_CONC_SMS */


/*************** Functions which must be called by MFW ***************/


/*
+--------------------------------------------------------------------+
| PROJECT :                     MODULE  : CONC_SMS                   |
| STATE   : code                ROUTINE : SMS_getSMSType                 |
+--------------------------------------------------------------------+

  PURPOSE : This function must be called by the MFW to detect the SMS
            type from the information element identifier.

*/
GLOBAL T_SMS_TYPE SMS_getSMSType( T_ACI_UDH_DATA* udh, char *address, UBYTE detMode)
{
  USHORT ref_num = 0;
  UBYTE seq_num = 0;
  UBYTE max_num = 0;
  USHORT count;
  T_CONC_BUF* concBuf;

  TRACE_FUNCTION ("SMS_getSMSType()");
  TRACE_EVENT_P1("SMS_getSMSType, mode: %d", detMode);


  if (udh->len EQ 0)
  {
    /* SMS does not contain UDH --> normal SMS */
    return NORMAL;
  }

  /* check if IE is conc SMS */
  if ((udh->data[0] EQ SMS_IEI_CONC_8BIT) OR (udh->data[0] EQ SMS_IEI_CONC_16BIT))
  {
  if (udh->data[0] EQ SMS_IEI_CONC_8BIT)
    {
      ref_num = udh->data[2];
      max_num = udh->data[3];
      seq_num = udh->data[4];
    }

  if (udh->data[0] EQ SMS_IEI_CONC_16BIT)
    {
      ref_num = (udh->data[2] & 0x00FF) << 8u;
      ref_num += udh->data[3];
      max_num = udh->data[4];
      seq_num = udh->data[5];
    }

    switch (detMode)
    {
    
    case MODE1:
    /* This mode is for rAT_PlusCMT. No Concatenation buffer is needed at all */
      break;

    case MODE2:
    /* This mode is for rAT_PlusCMTI. This mode requires the allocation of a new 
     * Concatenation buffer (later in concSMS_Collect). Make sure that in case 
     * the conc buffer is full no new CSMS is handled anymore 
     */
      concBuf = concSMS_getConcBuffer( ref_num, address );

      /* if a new conc buffer is be needed, check if available */
      if (concBuf EQ NULL)
      {
        if (concSMS_concBufferAvail() EQ FALSE)
        {
          return NORMAL_IND_CSMS;
        }
         
        /* Limit the maximum number of CSMS segments to MAX_SEG_TOTAL. Check 
         * only if a new Concatenation buffer is required.
         */
        if (concShrdPrm.elem_count+max_num > MAX_SEG_TOTAL)
        {
          return NORMAL_IND_CSMS;
        }
      }
      break;

    case MODE3:
    /* This mode is for rAT_PlusCMGL, rAT_PlusCMGR, sms_store_new_msg_info and 
     * sms_store_new_msg_info. Only segments that have been previously stored 
     * in the Concatenation buffer can be handled as CSMS. 
     */
      concBuf = concSMS_getConcBuffer( ref_num, address );
      if (concBuf EQ NULL)
      {
        return NORMAL_IND_CSMS;
      }

      /* check if conc buffer is incomplete */
      count = get_list_count(concBuf->list);
      if ((count < concBuf->max_num) AND (count < CONC_MAX_SEGS))
      {
        return NORMAL_IND_CSMS;
      }
      break;

    default:
      TRACE_ERROR("Wrong detection mode in SMS_getSMSType");
      return UNKNOWN;
    }

    /* check if sequence number is in range */
    if (seq_num <= CONC_MAX_SEGS)
    {
      return CONCATE;
    }
    else
    {
      return NORMAL_IND_CSMS;
    }
  }
  else
  {
    /* unknown IE in UDH --> no CSMS */
    return UNKNOWN;
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT :                     MODULE  : CONC_SMS                   |
| STATE   : code                ROUTINE : concSMS_GetFirstIndex      |
+--------------------------------------------------------------------+

  PURPOSE : This function provides MFW with the first index of a given
            concatenated SMS (identified by its message reference).

            returns first index: (0 means no message found...)
*/
#define FIRST_SEQ_NUM (1)

GLOBAL T_CONC_BUF_ELEM *concSMS_GetFirstIndex_ext ( USHORT msg_ref, char *address  )
{


  T_CONC_BUF      *concBuf;
  T_CONC_BUF_ELEM *concBufElem;

  TRACE_FUNCTION ("concSMS_GetFirstIndex()");

  /* search for concatenation buffer */
  concBuf = concSMS_getConcBuffer( msg_ref, address );

  if( concBuf EQ NULL )
  {
    TRACE_EVENT_P1("ERROR: unknown msg_ref: 0x%04x", msg_ref);
    return( NULL );
  }

  /* search for the first sequence */
  concBufElem = find_element(concBuf->list, FIRST_SEQ_NUM, concSMS_findSeqNumElemCB);

  if( concBufElem EQ NULL )
  {
    TRACE_EVENT("ERROR: first sequence not found");
    return( NULL );
  }

  TRACE_EVENT_P1("first rec_num: %d", concBufElem->rec_num);
  TRACE_EVENT_P1 ("concSMS_GetFirstIndex_ext(), rec_num=%d", concBufElem->rec_num);

  /* return index of first segment */
  return(concBufElem);

}

GLOBAL UBYTE concSMS_GetFirstIndex ( USHORT msg_ref, char *address  )
{
  T_CONC_BUF_ELEM *concBufElem = concSMS_GetFirstIndex_ext(msg_ref, address);

  if( concBufElem EQ NULL )
  {
    TRACE_EVENT("ERROR: first sequence not found");
    return(0);
  }
  else
    return(concBufElem->rec_num);
}


/*
+--------------------------------------------------------------------+
| PROJECT :                     MODULE  : CONC_SMS                   |
| STATE   : code                ROUTINE : concSMS_GetMsgRef          |
+--------------------------------------------------------------------+

  PURPOSE : This function provides MFW with the message reference of a given
            concatenated SMS (decoded from the header).

            returns msg ref.
*/

GLOBAL USHORT concSMS_GetMsgRef ( T_ACI_CMGL_SM  *sm )
{
  USHORT ref_num;

  TRACE_FUNCTION ("concSMS_GetMsgRef()");

  if( sm EQ NULL )
  {
    TRACE_ERROR("sm is NULL");
    return 0;
  }

    /* 8-bit reference number */
  if (sm->udh.data[0] EQ SMS_IEI_CONC_8BIT)
  {
    ref_num = sm->udh.data[2];
  }

  /* 16-bit reference number */
  else if (sm->udh.data[0] EQ SMS_IEI_CONC_16BIT)
  {
    /* MSB */
    ref_num = (sm->udh.data[2] & 0x00FF) << 8u;   /* 23.040 9.1.2.1 */

    /* LSB */
    ref_num |= sm->udh.data[3];
  }
  else
  {
    TRACE_ERROR("sm->udh.data unknown");
    return 0;
  }

  TRACE_EVENT_P1("ref_num: 0x%04x", ref_num);

  return(ref_num);
}

/*
+--------------------------------------------------------------------+
| PROJECT :                     MODULE  : CONC_SMS                   |
| STATE   : code                ROUTINE : concSMS_Collect            |
+--------------------------------------------------------------------+

  PURPOSE : This function must be called by the MFW. The function
            reassembles the received SM segments.

            Set 'isStored' to:
                            TRUE,  if rAT_PlusCMTI() was called
                            FALSE, if rAT_PlusCMT()  was called
                            FALSE, if rAT_PlusCMGR() was called

*/
GLOBAL T_CONC_ASSEMBLY_RETURN concSMS_Collect ( T_SM_DATA_EXT *data_conc,
                                               T_ACI_CMGL_SM  *sm,
                                               UBYTE          isStored,
                                               T_ACI_SMS_STOR mem_aci)
{
  USHORT ref_num = 0;
  UBYTE  max_num = 0;
  UBYTE  seq_num = 0;
  UBYTE  rec_num;
  T_SM_ASSEMBLY  *assembly_elem;
  T_SEG_BUF_ELEM *segBufElem;
  T_SM_DATA_EXT  data;
  T_ACI_SMS_STAT status;
  UBYTE next_exp_num;
  UBYTE ret;
  static UBYTE i = 0;
  UBYTE mem_psa;

 

  CHAR *address;

  TRACE_FUNCTION ("concSMS_Collect()");

  /* initialize data_conc->len */
  data_conc->len = 0; /* ACI-SPR-16372 */
  /* extract parameters from user data header */

  /* 8-bit reference number */
  if (sm->udh.data[0] EQ SMS_IEI_CONC_8BIT)
  {
    ref_num = sm->udh.data[2];
    max_num = sm->udh.data[3];
    seq_num = sm->udh.data[4];
  }

  /* 16-bit reference number */
  else if (sm->udh.data[0] EQ SMS_IEI_CONC_16BIT)
  {
    /* MSB */
    ref_num = (sm->udh.data[2] & 0x00FF) << 8u;    /* 23.040 9.1.2.1 */

    /* LSB */
    ref_num += sm->udh.data[3];

    max_num = sm->udh.data[4];
    seq_num = sm->udh.data[5];
  }

  rec_num = sm->msg_ref;
  status  = sm->stat;
  address = sm->adress;

  data.data = sm->data.data;
  data.len  = sm->data.len;

  TRACE_EVENT_P1("rec_num:%d", rec_num);
  TRACE_EVENT_P1("concSMS_GetFirstIndex():%d",
                  concSMS_GetFirstIndex (ref_num, sm->adress));
  TRACE_EVENT_P1("ref_num:0x%04x", ref_num);
  TRACE_EVENT_P1("concSMS_GetMsgRef():0x%04x", concSMS_GetMsgRef ( sm ));
  TRACE_EVENT_P1("max_num:%d", max_num);
  TRACE_EVENT_P1("seq_num:%d", seq_num);

  /* try to get an existing assembly buffer */
  assembly_elem = concSMS_getAsBuffer(ref_num, address);

  /* if no assembly buffer found, then assume that is a new conc. SM */
  if (assembly_elem EQ NULL)
  {
    next_exp_num = 1;
  }
  else
  {
    next_exp_num = assembly_elem->next_exp_num;
  }


  /* the received seq_num is not valid */
  if ( (seq_num EQ 0) OR (seq_num > max_num) OR (seq_num < next_exp_num) )
  {
    return CONC_ERR_UNKN;
  }



  /*
   * If CMTI, then add every received segment to concatenation buffer.
   * The segments in the concatenation buffer were also stored in non-volatile
   * memory.
   */
  if (isStored)
  {
     if (cmhSMS_getMemPsa(mem_aci, &mem_psa) EQ FALSE)
    {
       return CONC_ERR_UNKN;
    }

    ret = concSMS_addToConcatList(ref_num, address, max_num,
                                  seq_num, rec_num, status, mem_psa);

    if (ret EQ FALSE)
    {
      dFLAG = TRUE;
      RefNum_Del = ref_num;
      concShrdPrm.full.Conc_Full = TRUE;
      concShrdPrm.full.RefNum = ref_num;
      memcpy(Addres,address,sizeof(Addres));
      concShrdPrm.full.MaxNum = max_num;
      concShrdPrm.full.SeqNum[i] = seq_num;
      concShrdPrm.full.RecNum[i] = rec_num;
       concShrdPrm.full.MemType[i] = mem_psa;
      i++;
      concShrdPrm.full.Numsegs = i;
      /* error: concatenation buffer full */
      return CONC_ERR_BUF_FULL;
    }
  }
  i = 0;
  if ( seq_num > next_exp_num )
  {
    /*
     * the received segment is not in sequence
     *  -> add it to the segment buffer
     */
    ret = concSMS_addToSegBuffer(ref_num, address, seq_num,
                                 rec_num, status, &data);
    if (ret EQ FALSE)
    {
      /* error: segment buffer full */
      return CONC_ERR_BUF_FULL;
    }
    return CONC_CONTINUED;
  }
  else
  {
    /*
     * the received segment is in sequence
     */

    while (1)
    {

      /* add segment to assembly buffer */
      assembly_elem = concSMS_addToAsBuffer (ref_num, address, max_num, &data);


      /* free data memory only for data from segment buffer */
      if (data.data NEQ sm->data.data)
      {
        ACI_MFREE (data.data);
      }

      /* error: no assembly buffer available */
      if (assembly_elem EQ NULL)
      {
        return CONC_ERR_BUF_FULL;
      }

      /* assembly of concatenated SM is completed */
      if (assembly_elem->next_exp_num EQ max_num+1)
      {
        concSMS_removeFromAsBuffer(data_conc, ref_num, address);
        concSMS_sortConcatList(ref_num, address);
        return CONC_COMPLETED;
      }

      /* maximum reached in assembly buffer , assembly is NOT completed */
      if (assembly_elem->seg_count EQ CONC_MAX_SEGS)
      {
        concSMS_getFromAsBuffer(data_conc, ref_num, address);
        concSMS_sortConcatList(ref_num, address);
        return CONC_COMPLETED;
      }

      /* search and remove the next segment from segment buffer */
      segBufElem = concSMS_removeFromSegBuffer(ref_num,
                                               address,
                                               assembly_elem->next_exp_num);
      if ( segBufElem EQ NULL )
      {
        /* no segment found */
        return CONC_CONTINUED;
      }

      /* for adding to concatenation list */
      rec_num   = segBufElem->rec_num;
      status    = (T_ACI_SMS_STAT)segBufElem->status;

      /* for adding to assembly buffer */
      data.data = segBufElem->data.data;
      data.len  = segBufElem->data.len;

      ACI_MFREE(segBufElem);
    } /* while */
  } /* if ( seq_num EQ next_exp_num ) */
}



GLOBAL void concSMS_Init()
{
  concShrdPrm.isConcatenated = FALSE;

  memset (&concShrdPrm, 0, sizeof(T_CONC_SHRD_PRM) );

  memset (&assembly_list, 0, sizeof(T_SM_ASSEMBLY)*MAX_BUF_ELEMS);
  memset (&segBuf_list,   0, sizeof(T_SEG_BUF)    *MAX_BUF_ELEMS);
  memset (&concBuf_list,  0, sizeof(T_CONC_BUF)   *MAX_CONC_BUF_ELEMS);

  concShrdPrm.l_uncomp8bit_data      = L_UNCOMP_8BIT_DATA;
  concShrdPrm.l_uncomp7bit_data      = L_UNCOMP_7BIT_DATA;
  concShrdPrm.l_uncomp8bit_data_conc = L_UNCOMP_8BIT_DATA_CONC;
  concShrdPrm.l_uncomp7bit_data_conc = L_UNCOMP_7BIT_DATA_CONC;
  concShrdPrm.elem_count = 0;
#ifdef _CONC_TESTING_
#ifndef _SIMULATION_
  concSMS_InitForTesting();
#endif
#endif
}


/* only for test purposes */
GLOBAL void concSMS_InitForTesting()
{
  concShrdPrm.concTesting            = TRUE;
  concShrdPrm.l_uncomp8bit_data      = L_UNCOMP_8BIT_DATA_TST;
  concShrdPrm.l_uncomp7bit_data      = L_UNCOMP_7BIT_DATA_TST;
  concShrdPrm.l_uncomp8bit_data_conc = L_UNCOMP_8BIT_DATA_CONC_TST;
  concShrdPrm.l_uncomp7bit_data_conc = L_UNCOMP_7BIT_DATA_CONC_TST;
}


/*
+--------------------------------------------------------------------+
| PROJECT :                     MODULE  : CONC_SMS                   |
| STATE   : code                ROUTINE : concSMS_delAllIncompleteMsg|
+--------------------------------------------------------------------+

  PURPOSE : This function must be called by the MFW to clean all
            segment buffers, assembly buffers and remove all
            incompleted SM from concat. buffer and non-volatile memory.

*/
GLOBAL void concSMS_delAllIncompleteMsg( )
{
  T_SEG_BUF       *segBuf = NULL;
  T_SEG_BUF_ELEM  *segBufElem;
  T_CONC_BUF      *concBuf = NULL;
  T_CONC_BUF_ELEM *concBufElem;
  T_SM_ASSEMBLY   *assembly_elem;
  USHORT count;

  UBYTE i;

  TRACE_FUNCTION ("concSMS_delAllIncompleteMsg()");


  concShrdPrm.srcId = CMD_SRC_LCL;

  for (i=0; i<MAX_BUF_ELEMS; i++)
  {
    /* delete assembly buffer */
    if (assembly_list[i].in_use)
    {
      assembly_elem = &assembly_list[i];
      assembly_elem->in_use = FALSE;
      ACI_MFREE(assembly_elem->data.data);
    }


    /* delete segment buffer */
    if (segBuf_list[i].in_use)
    {
      segBuf = &segBuf_list[i];

      /* remove element from segment buffer */
      segBufElem = remove_first_element(segBuf->list);

      if (segBufElem NEQ NULL)
      {
        /* free memory */
        ACI_MFREE(segBufElem->data.data);
        ACI_MFREE(segBufElem);
      }
      else
      {
        segBuf->in_use = FALSE;
      }
    }
  } /* for */


  for (i=0; i<MAX_CONC_BUF_ELEMS; i++)
  {
    /* find concat. buffer in use */
    if (concBuf_list[i].in_use)
    {

      /* get number of segments in this concat. buffer */
      count = get_list_count(concBuf_list[i].list);

      /* conc buffer is incomplete */
      if ((count < concBuf_list[i].max_num) AND (count < CONC_MAX_SEGS))
      {
        if (count EQ 0)
        {
          /* last element of one conc buffer was deleted */
          concBuf_list[i].in_use = FALSE;
          R_AT ( RAT_OK,(T_ACI_CMD_SRC) concShrdPrm.srcId ) ( AT_CMD_CMGC );
          continue;
        }
        concBuf = &concBuf_list[i];
        break;
      }
    }
  }

  if (concBuf)
  {
    /* remove element from concat. buffer */
    concBufElem = remove_first_element(concBuf->list);

    if (concBufElem)
    {

      /* delete segment from non-volatile memory */
#ifdef TI_PS_FF_CONC_SMS
      sAT_PlusCMGD_Gl((T_ACI_CMD_SRC)concShrdPrm.srcId, concBufElem->rec_num, 
               smsShrdPrm.status, concSMS_delAllIncompleteMsg, rConcSMS_PlusCMS_CMGD);
#else
      sAT_PlusCMGD_Gl(concShrdPrm.srcId, concBufElem->rec_num, smsShrdPrm.status,
                      concSMS_delAllIncompleteMsg, rAT_PlusCMS);
#endif /* TI_PS_FF_CONC_SMS */

      /* free memory */
      ACI_MFREE(concBufElem);
       /* decrease total count of stored CSMS segments by 1*/
      concShrdPrm.elem_count--;
    }
  }
}



/************** Help Functions ******************************/


/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CONC_SMS                |
|                                 ROUTINE : concSMS_printConcatList |
+-------------------------------------------------------------------+

  PURPOSE :
*/

LOCAL void concSMS_printConcatList ()
{
  T_ACI_LIST    *current;
  UBYTE i;
  T_CONC_BUF *concBuf;
  T_CONC_BUF_ELEM *elem;


  TRACE_EVENT("Concatenation List:");
  for (i=0; i<MAX_CONC_BUF_ELEMS; i++)
  {
    if (concBuf_list[i].in_use EQ TRUE)
    {
      concBuf = &concBuf_list[i];
      TRACE_EVENT_P2("   ref_num: 0x%04x , max_num: %u",
                      concBuf->ref_num, concBuf->max_num);
      current = concBuf->list;
      while (current)
      {
        elem = (T_CONC_BUF_ELEM*) current->msg;
        TRACE_EVENT_P4("   seq_num: %d , rec_num: %d , status: %d, mem= %d",
                       elem->seq_num, elem->rec_num, elem->status, elem->mem);
        current = current->next;
      }
    }
  }
}

/* Marcus: Issue 872: 03/10/2002
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CONC_SMS                |
|                                 ROUTINE : concSMS_findMaxRefNum   |
+-------------------------------------------------------------------+

  PURPOSE : Find the highest value of concBuf_list[i].ref_num
*/

LOCAL USHORT concSMS_findMaxRefNum(void)
{
  UBYTE i;
  USHORT ref_num = 0;
  TRACE_FUNCTION("concSMS_findMaxRefNum()");
  for (i=0; i<MAX_CONC_BUF_ELEMS; i++)
  {
    if (concBuf_list[i].in_use EQ TRUE)
    {
      if ((concBuf_list[i].ref_num & 0x00ff)> ref_num)     /* since we use only 8-Bit ref_num */
        ref_num = concBuf_list[i].ref_num & 0x00ff;
    }
  }
  return ref_num;
}


GLOBAL void concSMS_clearIncompleteMsg()
{
  
  T_SEG_BUF       *segBuf = NULL;
  T_SEG_BUF_ELEM  *segBufElem;
  T_SM_ASSEMBLY   *assembly_elem;
  T_CONC_BUF      *concBuf = NULL;
  T_CONC_BUF_ELEM *concBufElem;
  UBYTE i;
  

 
  TRACE_FUNCTION ("concSMS_clearIncompleteMsg()");


  if (RefNum_Del EQ concShrdPrm.full.RefNum)
  {
    concShrdPrm.full.Conc_Full = FALSE;
    concShrdPrm.full.RefNum = 0xFF;
  }


  for (i=0; i<MAX_BUF_ELEMS; i++)
  {
    /* delete assembly buffer */
    if (assembly_list[i].in_use AND assembly_list[i].ref_num EQ RefNum_Del)
    {
      assembly_elem = &assembly_list[i];
      assembly_elem->in_use = FALSE;
      ACI_MFREE(assembly_elem->data.data);
      break;
    }


    /* delete segment buffer */
    if (segBuf_list[i].in_use AND segBuf_list[i].ref_num EQ RefNum_Del)
    {
      
      segBuf = &segBuf_list[i];
     

      /*remove element from segment buffer */
      segBufElem = remove_first_element(segBuf->list);

      if (segBufElem NEQ NULL)
      {     
        segBuf->in_use = FALSE;
        /* free memory */
        ACI_MFREE(segBufElem->data.data);
        ACI_MFREE(segBufElem);
        
        
      }
      else
      {
        
        segBuf->in_use = FALSE;
        
      }
      break;
    }
  
  }

  for (i=0; i<MAX_CONC_BUF_ELEMS; i++)
  {
    /* find concat. buffer in use */
    if (concBuf_list[i].in_use AND concBuf_list[i].ref_num EQ RefNum_Del)
    {
        concBuf_list[i].in_use = FALSE;
        concBuf = &concBuf_list[i];            
        break;
     
    }
  }

  if (concBuf)
  {   
    /* remove element from concat. buffer */
    concBufElem = remove_first_element(concBuf->list);

    if (concBufElem)
    {       
      /* free memory */
      ACI_MFREE(concBufElem);
      /* decrease total count of stored CSMS segments by 1*/
      concShrdPrm.elem_count--;

    }
  }
}


GLOBAL void concSMS_AddtoconcBuff(void)
{
  UBYTE i;


  TRACE_FUNCTION("concSMS_AddtoconcBuff()");

  for (i=0;i<concShrdPrm.full.Numsegs;i++)
  {
    concSMS_addToConcatList(concShrdPrm.full.RefNum,Addres,
                  concShrdPrm.full.MaxNum,concShrdPrm.full.SeqNum[i],
                  concShrdPrm.full.RecNum[i],SMS_STAT_RecRead,concShrdPrm.full.MemType[i]);
  }
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CONC_SMS                |
|                                 ROUTINE : concSMS_DeleteConcList  |
+-------------------------------------------------------------------+

  PURPOSE  : This function is used to delete the concatinated SMS, when
             the CMGD command contails the delete flag greater than ZERO.

*/

GLOBAL void concSMS_DeleteConcList ( )
{
  T_CONC_CMGD *prm = &concShrdPrm.specPrm.concCMGD;
  T_CONC_BUF_ELEM *elem;
  T_ACI_LIST *conc_list;

  TRACE_FUNCTION ("concSMS_DeleteConcList()");


  elem = (T_CONC_BUF_ELEM *)prm->currConcBufListElem->msg;

  /* remove the old element from concatenation list and free its memory */
  conc_list = concSMS_removeFromConcatList(prm->ref_num, prm->address, elem->rec_num);

  concSMS_printConcatList();

  if (conc_list NEQ NULL)
  {
    TRACE_EVENT("conc_list not null");
    elem = (T_CONC_BUF_ELEM *)conc_list->msg;

    /* save the concatenation list */
    prm->currConcBufListElem= conc_list;  
  }
  else /* if (concShrdPrm.currConcBufListElem NEQ NULL) */
  {
    TRACE_EVENT("Concat List is NULL : concSMS_DeleteConcList");
    UNSET_CONC;
  }
}

#ifdef TI_PS_FF_CONC_SMS
GLOBAL void rConcSMS_PercentCMGR ( T_ACI_CMGL_SM*  sm,
                                   T_ACI_CMGR_CBM* cbm )
{
  T_CONC_CMGR *prm = &concShrdPrm.specPrm.concCMGR;

  TRACE_FUNCTION ("rConcSMS_PercentCMGR ()");

  if (prm->currConcBufListElem NEQ NULL)
  {
    T_CONC_BUF_ELEM *elem;
    elem = (T_CONC_BUF_ELEM *)prm->currConcBufListElem->msg;

    sAT_PercentCMGR_Gl((T_ACI_CMD_SRC)concShrdPrm.srcId, elem->rec_num,
                   (T_ACI_SMS_READ) concShrdPrm.specPrm.concCMGR.rdMode,
                    rConcSMS_PercentCMGR);

    prm->currConcBufListElem = prm->currConcBufListElem->next;

#ifdef _CONC_TESTING_
  rAT_PercentCMGR_Ext (sm, cbm);
#else
  rAT_PercentCMGR (sm, cbm);
#endif

  }
  else /* if (concShrdPrm.currConcBufListElem NEQ NULL) */
  {
#ifdef _CONC_TESTING_
  rAT_PercentCMGR_Ext (sm, cbm);
#else
  rAT_PercentCMGR (sm, cbm);
#endif

  R_AT ( RAT_OK, (T_ACI_CMD_SRC)concShrdPrm.srcId ) ( AT_CMD_P_CMGR );
    UNSET_CONC;
  }
}   
    
#endif /* TI_PS_FF_CONC_SMS */   
/* 
 * This functions checks if the conc can be extended. If the maximum 
 * capacity is reached the CSMS will be handled as normal (single) SMS.
 */
BOOL concSMS_concBufferAvail()
{
  USHORT count=0;
  int i;

  TRACE_FUNCTION("concSMS_concBufferAvail");

  /* check if there is a free conc buffer available */
  for (i=0; i<MAX_CONC_BUF_ELEMS; i++)
  {  
    if (concBuf_list[i].in_use EQ TRUE)
    {
      count++;
    }
  }
  if (count >= MAX_CONC_BUF_ELEMS)
  {
    return FALSE; /* all conc buffer in use, no free available */
  }

  return TRUE;
}
