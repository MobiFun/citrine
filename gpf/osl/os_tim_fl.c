/*
 * This C module is a reconstruction based on the disassembly of
 * os_tim.obj in frame_na7_db_fl.lib from the Leonardo package,
 * subsequently reworked by Space Falcon.
 */

/* set of included headers from COFF symtab: */
#include <stdio.h>
#include "gpfconf.h"	/* FreeCalypso addition */
#include "../../nucleus/nucleus.h"
#include "typedefs.h"
#include "os.h"
#include "gdi.h"
#include "os_types.h"
#include "os_glob.h"

extern UNSIGNED TMD_Timer;
extern INT      TMD_Timer_State;

extern T_OS_TIMER_ENTRY TimerTable[];
extern T_OS_TIMER_TABLE_ENTRY *p_list[];

extern void os_Timeout(UNSIGNED t_handle);
extern void timer_error(int err);

unsigned os_time_to_tick_multiplier = TIME_TO_TICK_TDMA_FRAME_MULTIPLIER;
unsigned os_tick_to_time_multiplier = TICK_TO_TIME_TDMA_FRAME_MULTIPLIER;

unsigned volatile t_start_ticks;
T_OS_TIMER_TABLE_ENTRY *t_running;
int used_timers;
int next_t_handle;
int volatile t_list_access;
int max_used_timers;
NU_SEMAPHORE TimSemCB;
NU_TIMER os_timer_cb;

#define	BARRIER	asm volatile ("": : :"memory")

GLOBAL LONG
os_set_tick(int os_system_tick)
{
	switch (os_system_tick) {
	case SYSTEM_TICK_TDMA_FRAME:
		os_time_to_tick_multiplier = TIME_TO_TICK_TDMA_FRAME_MULTIPLIER;
		os_tick_to_time_multiplier = TICK_TO_TIME_TDMA_FRAME_MULTIPLIER;
		return(OS_OK);
	case SYSTEM_TICK_10_MS:
		os_time_to_tick_multiplier = TIME_TO_TICK_10MS_MULTIPLIER;
		os_tick_to_time_multiplier = TICK_TO_TIME_10MS_MULTIPLIER;
		return(OS_OK);
	default:
		return(OS_ERROR);
	}
}

GLOBAL LONG
os_TimerInformation(USHORT Index, char *Buffer)
{
	static int t_info_read;

	if (t_info_read) {
		t_info_read = 0;
		return(OS_ERROR);
	}
	sprintf(Buffer, "Maximum %d of %d available timers running",
		max_used_timers, MaxSimultaneousTimer);
	t_info_read = 1;
	return(OS_OK);
}

GLOBAL LONG
os_TimInit(void)
{
	int i;

	if (NU_Create_Semaphore(&TimSemCB, "TIMSEM", 1, NU_PRIORITY)
			!= NU_SUCCESS)
		return(OS_ERROR);
	if (NU_Create_Timer(&os_timer_cb, "OS_TIMER", os_Timeout, 0, 1, 0,
			    NU_DISABLE_TIMER) != NU_SUCCESS)
		return(OS_ERROR);
	used_timers = 0;
	max_used_timers = 0;
	next_t_handle = 1;
	t_list_access = 0;
	t_start_ticks = 0;
	p_list[0] = 0;
	for (i = 1; i < MaxSimultaneousTimer; i++) {
		TimerTable[i].entry.status = TMR_FREE;
		TimerTable[i].entry.next = 0;
		TimerTable[i].entry.prev = 0;
		TimerTable[i].next_t_handle = i + 1;
		p_list[i] = 0;
	}
	TimerTable[MaxSimultaneousTimer].entry.status = TMR_FREE;
	TimerTable[MaxSimultaneousTimer].next_t_handle = 0;
	t_running = 0;
	return(OS_OK);
}

GLOBAL LONG
os_RecoverTick(OS_TICK ticks)
{
	UNSIGNED current_system_clock;

	current_system_clock = NU_Retrieve_Clock();
	NU_Set_Clock(current_system_clock + ticks);
	if (TMD_Timer_State == TM_ACTIVE) {
		if (TMD_Timer <= ticks) {
			TMD_Timer_State = TM_EXPIRED;
			TMD_Timer = 0;
		} else
			TMD_Timer -= ticks;
	}
	return(OS_OK);
}

GLOBAL LONG
os_QueryTimer(OS_HANDLE TaskHandle, OS_HANDLE TimerHandle,
		OS_TIME *RemainingTime)
{
	T_OS_TIMER_TABLE_ENTRY *timer, *t_iter;
	OS_TICK c_ticks, r_ticks, e_ticks;
	STATUS sts;

	if (TimerHandle > MaxSimultaneousTimer)
		return(OS_ERROR);
	sts = NU_Obtain_Semaphore(&TimSemCB, NU_SUSPEND);
	timer = &TimerTable[TimerHandle].entry;
	if (timer->status == TMR_FREE) {
		if (sts == NU_SUCCESS)
			NU_Release_Semaphore(&TimSemCB);
		return(OS_ERROR);
	}
	t_list_access = 1;
	BARRIER;
	if (!t_running) {
		r_ticks = 0;
		goto out;
	}
	c_ticks = NU_Retrieve_Clock();
	e_ticks = c_ticks - t_start_ticks;
	t_iter = t_running;
	if (t_iter->r_ticks >= e_ticks)
		r_ticks = t_iter->r_ticks - e_ticks;
	else
		r_ticks = 0;
	while (t_iter != timer) {
		t_iter = t_iter->next;
		if (t_iter == t_running) {
			r_ticks = 0;
			goto out;
		}
		r_ticks += t_iter->r_ticks;
	}
out:	BARRIER;
	t_list_access = 0;
	if (sts == NU_SUCCESS)
		NU_Release_Semaphore(&TimSemCB);
	*RemainingTime = SYSTEM_TICKS_TO_TIME(r_ticks);
	return(OS_OK);
}

GLOBAL LONG
os_InactivityTicks(int *next_event, OS_TICK *next_event_ticks)
{
	*next_event = 1;
	switch (TMD_Timer_State) {
	case TM_ACTIVE:
		*next_event_ticks = TMD_Timer;
		return(OS_OK);
	case TM_NOT_ACTIVE:
		*next_event_ticks = 0;
		*next_event = 0;
		return(OS_OK);
	default:
		*next_event_ticks = 0;
		return(OS_OK);
	}
}
