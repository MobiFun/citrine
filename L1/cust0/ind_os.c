/************* Revision Controle System Header *************
 *                  GSM Layer 1 software 
 * IND_OS.C
 *
 *        Filename ind_os.c
 *        Version  1.1
 *        Date     04/26/00
 *
 ************* Revision Controle System Header *************/

#include "../../include/config.h"

// Nucleus include files.
#include "../../nucleus/nucleus.h"
#include "../../nucleus/tc_defs.h"
#include "../../include/sys_types.h"
#include "ind_os.h"

#include "l1_confg.h"
// Layer1 and debug include files.
#include "l1_types.h"
#include "l1_const.h"

#if (L1_GTT == 1)
  #include "l1gtt_const.h"
  #include "l1gtt_defty.h"
#endif

/* the newer LoCosto cust1 version has these lines commented out */
//#if ((ANALOG == 1) || (ANALOG == 2))
//  #include "spi_drv.h"
//#endif

#if TESTMODE
  #include "l1tm_defty.h"
#endif

#include "l1audio_const.h"
#include "l1audio_cust.h"
#include "l1audio_defty.h"
#include "l1_defty.h"
#include "l1_msgty.h"
#include "l1_varex.h"

#if (CHIPSET == 2 || CHIPSET == 3 || CHIPSET == 4 || CHIPSET == 5 || CHIPSET == 6 || CHIPSET == 7 || CHIPSET == 8 || CHIPSET == 10 || CHIPSET == 11 || CHIPSET == 12)
  #include "../../bsp/ulpd.h"
#endif

extern UWORD32    TCD_Priority_Groups;
extern TC_HCB     *TCD_Active_HISR_Heads[TC_HISR_PRIORITIES];
extern VOID       *TCD_Current_Thread;
extern TC_HCB     *TCD_Active_HISR_Tails[TC_HISR_PRIORITIES];
extern INT        TMD_Timer_State;
extern UWORD32    TMD_Timer;               // for big sleep 
extern TC_PROTECT TCD_System_Protect;

 
 /*-------------------------------------------------------*/
 /* int ind_os_sleep()                                    */
 /*-------------------------------------------------------*/ 
 /* Parameters : none                                     */
 /* Return     :                                          */     
 /* Functionality : Suspend the thread an interval        */
 /*                 of millisecs.                         */
 /* Limitation :                                          */
 /*-------------------------------------------------------*/

 T_OS_RETURN  ind_os_sleep (SYS_UWORD32 millisecs)
{
  NU_Sleep ((SYS_UWORD32) millisecs);
  return OS_OK;
}


 /*-------------------------------------------------------*/
 /* int OS_get_inactivity_ticks()                         */
 /*-------------------------------------------------------*/ 
 /* Parameters : none                                     */
 /* Return     : Number of ticks of inactivity            */
 /*              0 means immediate activity planned       */
 /*              -1 means no activity planned             */     
 /* Functionality : Evaluates the OS activity planned     */
 /*                 by looking at ready tasks, activated  */
 /*                HISR and the elapsed time of the timers*/
 /* Limitation : Must be protected from H/W interrupts    */
 /*-------------------------------------------------------*/
 int OS_get_inactivity_ticks(void)
 {
   int i;

   // Returns immediate activity if a task is ready
   if (TCD_Priority_Groups) 
	return 0;
	
   //for all HISR priorities
   for (i = 0; i < TC_HISR_PRIORITIES ; i++)
   {	
     // if no hisr of priority "i" ==> go to next priority
     if (TCD_Active_HISR_Heads[i] == 0) 	
       continue;
	  			
     // the first hisr is NOT the current one (frame hisr) so it may be
     // with other priority ==> abort
     if (TCD_Active_HISR_Heads[i] != TCD_Current_Thread) 
       return 0;
	  
     // the last hisr is NOT the current one (frame hisr) so there is
     // at least another hisr with same priority ==> abort
     if (TCD_Active_HISR_Tails[i] != TCD_Current_Thread)
       return 0;
	  
     // the first and last hisrs are the current one (frame hisr) but 
     // there are several occurences of it ! ==> abort          
     if (  (TCD_Active_HISR_Heads[i]->tc_activation_count != 1)) 
       return 0;
   }

   // Returns remaining ticks number if any timer is active
   if (TMD_Timer_State == TM_ACTIVE) 	// any active timer ?
   {
     if (TMD_Timer <= MIN_SLEEP_TIME) 
        return(0);
     else 	
   	return TMD_Timer;
   }

   // Returns not activity if no timer active
   if (TMD_Timer_State == TM_NOT_ACTIVE) 
   	return -1;
   else				
   // otherwise, returns immediate activity if a timer is expired (TM_EXPIRED)
        return(0);
 }

 /*-------------------------------------------------------*/
 /* int OS_system_protect()                               */
 /*-------------------------------------------------------*/ 
 /* Parameters : none                                     */
 /* Return     : The Thread Control Block of the thread   */
 /*              which already owns the protection or     */
 /*              0 if no protection                       */
 /* Functionality : Checks whether the system structures  */
 /*                 are already protected or not          */
 /*-------------------------------------------------------*/
 void OS_system_protect (void)
 {
   NU_Protect((NU_PROTECT*) &TCD_System_Protect);
 }

 /*-------------------------------------------------------*/
 /* int OS_system_Unprotect()                             */
 /*-------------------------------------------------------*/ 
 /* Parameters : none                                     */
 /* Return     :                                          */
 /* Functionality : unprotect the system structures       */
 /*-------------------------------------------------------*/
 void OS_system_Unprotect (void)
 {
   NU_Unprotect();
 }
