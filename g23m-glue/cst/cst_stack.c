/*******************************************************************************
 *
 * CST_STACK.C
 *
 * Tasks and HISRs stacks monitoring.
 *
 * (C) Texas Instruments 2000
 *
 ******************************************************************************/

#include <stdio.h>
#include <string.h>

#include "../../nucleus/nucleus.h"
#include "../../nucleus/cs_defs.h"
#include "../../nucleus/tc_defs.h"

// The following stack sizes are defined in int.s
#define IRQ_STACK_SIZE  128                // Number of bytes in IRQ stack
#define FIQ_STACK_SIZE  512                // Number of bytes in FIQ stack.
#define SYSTEM_SIZE     1024               // Define the system stack size

int vsi_o_trace (char * caller, unsigned long tclass, char * text);
int vsi_t_sleep (char * caller,	unsigned long tvalue);

void CST_stack_trace()
{
  int jndex, count, char_count;
  TC_TCB *head, *ptr;
  extern CS_NODE *TCD_Created_Tasks_List;
  extern CS_NODE *TCD_Created_HISRs_List;
  unsigned untouched;
  char *top_stack, *top, *bottom;
  extern char *TCT_System_Limit;
  char result[80];
  char name[9];

  vsi_o_trace("CST", 0x08, "Stack Info ...");
  vsi_o_trace("CST", 0x08, "    Name  top       bottom    current   untouched");
  vsi_o_trace("CST", 0x08, "-------------------------------------------------");

  for (jndex=0; jndex<2; jndex++)
  {
     // make use of TCB and HCB having same structure from beginning to past stack info
     if (jndex == 0)
         ptr = head = (TC_TCB *)TCD_Created_Tasks_List;
     else {
         ptr = head = (TC_TCB *)TCD_Created_HISRs_List;
         vsi_o_trace("CST", 0x08, "-------------------------------------------------");
     }

     count = 0;
     do
     {
       untouched = 0;
       top_stack = (char *)ptr->tc_stack_start;
       while (*top_stack == 0xFE && top_stack <= (char *)ptr->tc_stack_end)
       {
         untouched++;
         top_stack++;
       }

       // Avoid to get a spurious character when tasks names are 8 characters long
       memcpy (name, ptr->tc_name, 8);
       name[8] = 0;

       sprintf(result, "%8s: %08x  %08x  %08x  0x%x", 
                              name, 
                              ptr->tc_stack_start,
                              ptr->tc_stack_end,
                              ptr->tc_stack_pointer,
                              untouched);
       vsi_o_trace("CST", 0x08, result);
       vsi_t_sleep("",30);

       ptr = (TC_TCB*) ptr->tc_created.cs_next;
    } while (ptr != head && count++ < 50);  //count is protection from infinite loops
  } // end of  for (jndex=0; jndex<2; jndex++)

  // stack allocation algorithm from the int.s function INT_Initialize()
  //
  //   \          \
  //   \----------\
  //   \          \
  //   \          \
  //   \          \
  //   \ SYSSTACK \
  //   \          \
  //   \          \
  //   \          \
  //   \----------\
  //   \          \
  //   \ IRQSTACK \
  //   \          \
  //   \----------\
  //   \          \
  //   \          \
  //   \ FIQSTACK \
  //   \          \
  //   \          \
  //   \----------\
  //   \          \

  untouched = 0;
  top_stack = top = (char *)TCT_System_Limit;
  bottom = (char *) ((unsigned)TCT_System_Limit + SYSTEM_SIZE);
  while (*top_stack == 0xFE && top_stack <= bottom)
  {
     untouched++;
     top_stack++;
  }

  // "CST" being the current active task with its related CST_Stack,
  // the System Stack is unused (current sp_svc = System Stack Bottom)
  sprintf(result, "SYSSTACK: %08x  %08x  %08x  0x%x", 
                          top,
                          bottom,
						  bottom, // current sp_svc = System Stack Bottom
                          untouched);
  vsi_o_trace("CST", 0x08, "-------------------------------------------------");
  vsi_o_trace("CST", 0x08, result);
  vsi_t_sleep("",30);

  untouched = 0;
  top_stack = top = bottom + 4;
  bottom = (char *) ((unsigned)bottom + IRQ_STACK_SIZE);
  while (*top_stack == 0xFE && top_stack <= bottom)
  {
    untouched++;
    top_stack++;
  }

  // Since the processor is in supervisor mode (no IRQ & no FIQ),
  // current sp_irq = IRQ Stack bottom
  sprintf(result, "IRQSTACK: %08x  %08x  %08x  0x%x", 
                          top,
                          bottom,
						  bottom, // current sp_irq = IRQ Stack bottom
                          untouched);
  vsi_o_trace("CST", 0x08, result);
  vsi_t_sleep("",30);

  untouched = 0;
  top_stack = top = bottom + 4;
  bottom = (char *) ((unsigned)bottom + FIQ_STACK_SIZE);
  while (*top_stack == 0xFE && top_stack <= bottom)
  {
    untouched++;
    top_stack++;
  }

  // Since the processor is in supervisor mode (no IRQ & no FIQ),
  // current sp_fiq = FIQ Stack bottom
  sprintf(result, "FIQSTACK: %08x  %08x  %08x  0x%x", 
                          top,
                          bottom,
						  bottom, // current sp_fiq = FIQ Stack bottom
                          untouched);
  vsi_o_trace("CST", 0x08, result);
  vsi_o_trace("CST", 0x08, "-------------------------------------------------");
  vsi_t_sleep("",30);
}
