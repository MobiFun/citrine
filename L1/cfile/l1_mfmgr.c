
/************* Revision Controle System Header *************
 *                  GSM Layer 1 software
 * L1_MFMGR.C
 *
 *        Filename l1_mfmgr.c
 *  Copyright 2003 (C) Texas Instruments
 *
 ************* Revision Controle System Header *************/

#define  L1_MFMGR_C

#include "config.h"
#include "l1_confg.h"
#include "l1_macro.h"

#if (CODE_VERSION == SIMULATION)
  #include <string.h>
  #include "l1_types.h"
  #include "sys_types.h"
  #include "l1_const.h"
  #if TESTMODE
    #include "l1tm_defty.h"
  #endif
  #if (AUDIO_TASK == 1)
    #include "l1audio_const.h"
    #include "l1audio_cust.h"
    #include "l1audio_defty.h"
  #endif
  #if (L1_GTT == 1)
    #include "l1gtt_const.h"
    #include "l1gtt_defty.h"
  #endif
  #if (L1_MP3 == 1)
    #include "l1mp3_defty.h"
  #endif
  #if (L1_MIDI == 1)
    #include "l1midi_defty.h"
  #endif
  #if (L1_AAC == 1)
    #include "l1aac_defty.h"
  #endif
  #include "l1_defty.h"
  #include "cust_os.h"
  #include "l1_msgty.h"
  #include "l1_varex.h"
  #include "l1_proto.h"
#else
  #include <string.h>
  #include "l1_types.h"
  #include "sys_types.h"
  #include "l1_const.h"

  #if TESTMODE
    #include "l1tm_defty.h"
  #endif
  #if (AUDIO_TASK == 1)
    #include "l1audio_const.h"
    #include "l1audio_cust.h"
    #include "l1audio_defty.h"
  #endif
  #if (L1_GTT == 1)
    #include "l1gtt_const.h"
    #include "l1gtt_defty.h"
  #endif
  #if (L1_MP3 == 1)
    #include "l1mp3_defty.h"
  #endif
  #if (L1_MIDI == 1)
    #include "l1midi_defty.h"
  #endif
  #if (L1_AAC == 1)
    #include "l1aac_defty.h"
  #endif
  #include "l1_defty.h"
  #include "../../gpf/inc/cust_os.h"
  #include "l1_msgty.h"
  #include "l1_varex.h"
  #include "l1_proto.h"
#endif
#include "l1_tabs.h"

/*-------------------------------------------------------*/
/* l1s_clear_mftab()                                     */
/*-------------------------------------------------------*/
/* Parameters :                                          */
/* Return     :                                          */
/* Functionality :                                       */
/*-------------------------------------------------------*/
void l1s_clear_mftab(T_FRM  *frmlst)
{
  WORD32  j,k;

  #if (TRACE_TYPE==5)
    trace_mft("l1s_clear_mftab()", -1);
  #endif

  // Clear MFTAB.
  for (j=0; j<MFTAB_SIZE; j++)
  {
    for (k=0; k<L1_MAX_FCT; k++)
    {
      frmlst[j].fct[k].fct_ptr = NULL;  // Enough to clear the MFTAB.
    }
  }
}

/*-------------------------------------------------------*/
/* l1s_load_mftab()                                      */
/*-------------------------------------------------------*/
/* Parameters :                                          */
/* Return     :                                          */
/* Functionality :                                       */
/*-------------------------------------------------------*/
void l1s_load_mftab(const T_FCT *fct, const UWORD8 size, UWORD8 frame, T_FRM  *frmlst)
{
  UWORD8   i;
  UWORD8   frame_count;
  T_FRM   *current_frm;

  #if (TRACE_TYPE==5)
    trace_mft("l1s_load_mftab()", frame);
  #endif

  if(fct != NULL)
  // there is a Rom block available.
  {
    frame_count = 0;

    do
    {
      i=0;
      current_frm = &(frmlst[frame]);

      while (fct->fct_ptr != NULL)
      {
        // ROM block is downloaded to RAM, it is added to current block contents in MFTAB.
        // we have to look for a free place in the OPTIONAL struct. for current frame.
        while (current_frm->fct[i].fct_ptr != NULL) i++;

        current_frm->fct[i] = *fct++;
        i++;
      }

      // increment "fct" to skip the NULL function...
      fct++;

      // increment frame counter...
      frame_count++;

      // increment frame counter with round up...
      if(++frame >= MFTAB_SIZE) frame = 0;
    }
    while (frame_count < size);
    // end of permanent table when all frame read from ROM block.
  }
}

/*-------------------------------------------------------*/
/* l1s_exec_mftab()                                      */
/*-------------------------------------------------------*/
/* Parameters :                                          */
/* Return     :                                          */
/* Functionality :                                       */
/*-------------------------------------------------------*/
void l1s_exec_mftab()
{
  UWORD8  i=0;
  T_FCT  *current_fct;

  // Point to the first function for current frame.
  current_fct = &(l1s.mftab.frmlst[l1s.afrm].fct[0]);

  do
  /********************************************/
  /*** look at all fcts  until L1_MAX_FCT  ***/
  /********************************************/
  {
    if (current_fct->fct_ptr != NULL)
    /****************************************************/
    /* Check function is not NULL                       */
    /* -> execute functions and reset fct field         */
    /****************************************************/
    {
      UWORD8 param1 = current_fct->param1;
      UWORD8 param2 = current_fct->param2;

      (*current_fct->fct_ptr)(param1,param2);    // execute fction.
      current_fct->fct_ptr = NULL;               // clear executed fction.
      current_fct->param1  = NO_PAR;             // clear complexe function parameter.
      current_fct->param2  = NO_PAR;             // clear complexe function parameter.
    }

    // Increment "i" and function pointer.
    current_fct++;                 // point to next fction.
    i++;                           // increment fction counter.

  }   // end do.
  while (i < L1_MAX_FCT);
}

#if (FF_L1_FAST_DECODING  == 1 )
/*-------------------------------------------------------
 l1s_clean_mftab()
-------------------------------------------------------
 Parameters :-current task in MFTAB
             -current_tsk_frm : frame of current task
               from which functions  should be erase
 Return     :
 Functionality : Clean a task being execute
first step of function is to look for the frame
from which we want to erase functions of the task.
Second step is to identify function to clean up : we
want clean up only functions of current task starting
from current_tsk_frm.Hence, a test is done to identify
function from TASK_ROM_MFTAB inside MFTAB by checking
 fct pointer amd the two parameters param1 and param2.
If test is true (i.e parameters and function pointer
 are used for the current task executuion ) , a clean
is done by setfct pointer to NULL
in order to erase this function of MFTAB .The clean up
is applied until the last frame used by current task
(number of frames used by a task is defined by size
variable).
When clean up is done , reset active frame if current
frame is the last frame of MFTAB.
This function allow to pipeline fast signaling blocks
with non fast signaling blocks without lose blocks.
-------------------------------------------------------*/
void l1s_clean_mftab(UWORD8 task, UWORD8 current_tsk_frm)
{
  T_FRM  *p_current_frm;
  const T_FCT *fct;
  UWORD8  size;
  UWORD8  frame;
  UWORD8  i = 1;/*i refers to l1_mftab.h where we have frame 1,frame2,frame3... so i starts to 1*/
  UWORD8  j;
  UWORD8  k;

#if (TRACE_TYPE==5)
  trace_mft("l1s_clean_mftab()", -1);
#endif

  fct = TASK_ROM_MFTAB[task].address;
  size = TASK_ROM_MFTAB[task].size;
  frame =  l1s.afrm;

  /* Get the good frame in function block */
  while(i < current_tsk_frm)
  {
    while(fct->fct_ptr != NULL)
    {
      fct++;/* Skip non-nulll functions */
    }
    fct++;/*Skip null function */
    i++;
  }

  /* Search in MFTAB all functions relative to the block we want to erase*/
  while(i <= size)
  {
    p_current_frm = &(l1s.mftab.frmlst[frame]);

    while(fct->fct_ptr != NULL)
    {
      for(j = 0; j < L1_MAX_FCT; j++)
      {
        if(((p_current_frm->fct[j].param1 == task) && (p_current_frm->fct[j].param2 == fct->param2))
               && ( p_current_frm->fct[j].fct_ptr == fct->fct_ptr))
        {
            p_current_frm->fct[j].fct_ptr = NULL;

          /*l1s_load_mftab function inserts a block  just by looking if the function pointer is null : to avoid this issue, a remove dowwn */
          /*of all functions pointers of the frme is done so there is no hole and hence no possible block insertion*/
          if(frame != l1s.afrm) /* condition for shifting*/
            {
            for( k = j; k < L1_MAX_FCT - 1; k ++)
            {
              p_current_frm->fct[k] = p_current_frm->fct[k + 1];/* alls pointers are going up of one position in the frame*/
            }
            p_current_frm->fct[L1_MAX_FCT-1].fct_ptr = NULL; /*last pointer is setting to NULL to avoid to remove the last function fct[L1_MAX_FCT] */
            }
            j = L1_MAX_FCT;/* Function found, exit to save time */
        }
      }
      fct++;
    }
    fct++;
    i++;
    if(++ frame >= MFTAB_SIZE)
    {
      frame = 0;
    }
  }
}
#endif /* if (FF_L1_FAST_DECODING == 1) */



