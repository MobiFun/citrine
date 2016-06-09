/*
 * This C module is a reconstruction based on the disassembly of
 * os_tim.obj in frame_na7_db_ir.lib from the Leonardo package,
 * subsequently reworked by Space Falcon.
 *
 * The original decompilation has been contributed by Das Signal.
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

extern T_OS_TIMER_ENTRY TimerTable[];
extern T_OS_TIMER_TABLE_ENTRY *p_list[];

extern unsigned os_time_to_tick_multiplier;
extern unsigned os_tick_to_time_multiplier;

extern unsigned volatile t_start_ticks;
extern T_OS_TIMER_TABLE_ENTRY *t_running;
extern int used_timers;
extern int next_t_handle;
extern int volatile t_list_access;
extern int max_used_timers;
extern NU_SEMAPHORE TimSemCB;
extern NU_TIMER os_timer_cb;

#define	BARRIER	asm volatile ("": : :"memory")

void
timer_error(int err)
{
}

/* forward declaration */
void os_Timeout(UNSIGNED t_handle);

static int
os_remove_timer_from_list(T_OS_TIMER_TABLE_ENTRY *timer)
{
	OS_TICK c_ticks;

	if (timer != t_running) {
		if (timer->next != t_running)
			timer->next->r_ticks += timer->r_ticks;
	} else {
		c_ticks = NU_Retrieve_Clock();
		if (timer->next == timer) {
			t_running = 0;
		} else {
			timer->next->r_ticks =
			    t_start_ticks + timer->r_ticks +
			    timer->next->r_ticks - c_ticks;
			t_running = timer->next;
		}
		NU_Control_Timer(&os_timer_cb, NU_DISABLE_TIMER);
		if (t_running != NULL) {
			t_start_ticks = c_ticks;
			if (t_running->r_ticks != 0)
				NU_Reset_Timer(&os_timer_cb, os_Timeout,
						t_running->r_ticks, 0,
						NU_ENABLE_TIMER);
		}
	}
	if (timer->next != timer) {
		timer->prev->next = timer->next;
		timer->next->prev = timer->prev;
	}
	timer->next = NULL;
	timer->prev = NULL;
	timer->status = TMR_USED;
	return TMR_USED;
}

static unsigned
os_add_timer_to_list(T_OS_TIMER_TABLE_ENTRY *timer, OS_TICK ticks)
{
	T_OS_TIMER_TABLE_ENTRY *t_list;
	OS_TICK c_ticks, e_ticks, r1_ticks, return_ticks;

	if (ticks == 0)
		ticks = 1;

	c_ticks = NU_Retrieve_Clock();
	t_list = t_running;
	if (t_list != NULL) {
		e_ticks = c_ticks - t_start_ticks;
		if (t_list->r_ticks >= e_ticks) {
			r1_ticks = t_list->r_ticks - e_ticks;
			t_list->r_ticks = r1_ticks;
		} else {
			r1_ticks = 0;
			t_list->r_ticks = 0;
		}
		t_start_ticks = c_ticks;
		return_ticks = 0;
		while (ticks >= r1_ticks) {
			ticks -= r1_ticks;
			t_list = t_list->next;
			if (t_list == t_running)
				goto out;
			r1_ticks = t_list->r_ticks;
		}
		t_list->r_ticks -= ticks;
		if (t_list == t_running) {
			t_running = timer;
			t_start_ticks = c_ticks;
			NU_Control_Timer(&os_timer_cb, NU_DISABLE_TIMER);
			return_ticks = ticks;
		}
out:
		timer->next = t_list;
		timer->prev = t_list->prev;
		t_list->prev->next = timer;
		t_list->prev = timer;
		timer->r_ticks = ticks;
	} else {
		timer->next = timer;
		timer->prev = timer;
		timer->r_ticks = ticks;
		t_start_ticks = c_ticks;
		t_running = timer;
		return_ticks = ticks;
	}
	timer->status = TMR_ACTIVE;
	return return_ticks;
}

void
os_Timeout(UNSIGNED t_handle)	/* argument is unused */
{
	ULONG s_ticks;
	OS_HANDLE task_handle, e_handle;
	USHORT t_index;
	int i, done;
	T_OS_TIMER_TABLE_ENTRY **t_r4;
	T_OS_TIMER_TABLE_ENTRY *timer;
	void (*timeout_func) (OS_HANDLE, OS_HANDLE, USHORT);

	if (t_list_access) {
		t_start_ticks++;
		NU_Reset_Timer(&os_timer_cb, os_Timeout, 1, 0,
				NU_ENABLE_TIMER);
		return;
	}

	timer = t_running;
	if (timer) {
		s_ticks = 0;
		done = 0;
		i = 0;
		do {
			timeout_func = timer->TimeoutProc;
			if (timer->p_ticks)
				p_list[i++] = timer;
			task_handle = timer->task_handle;
			e_handle = timer->entity_handle;
			t_index = timer->t_index;
			timer->status = TMR_USED;
			if (timer->next == timer) {
				t_running = NULL;
				done = 1;
			} else {
				timer->prev->next = timer->next;
				timer->next->prev = timer->prev;
				if (timer->next->r_ticks) {
					t_running = timer->next;
					s_ticks = timer->next->r_ticks;
					done = 1;
				} else
					timer = timer->next;
			}
			timeout_func(task_handle, e_handle, t_index);
		}
		while (!done);

		if (s_ticks) {
			t_start_ticks = NU_Retrieve_Clock();
			NU_Reset_Timer(&os_timer_cb, os_Timeout, s_ticks, 0,
				       NU_ENABLE_TIMER);
		}
	}
	for (t_r4 = p_list; *t_r4; t_r4++) {
		timer = *t_r4;
		s_ticks = os_add_timer_to_list(timer, timer->p_ticks);
		if (s_ticks)
			NU_Reset_Timer(&os_timer_cb, os_Timeout, s_ticks, 0,
					NU_ENABLE_TIMER);
		*t_r4 = NULL;
	}
}

GLOBAL LONG os_StartTimer(OS_HANDLE TaskHandle, OS_HANDLE TimerHandle,
				USHORT Index, OS_TIME InitialTime,
				OS_TIME RescheduleTime)
{
	T_OS_TIMER_TABLE_ENTRY *timer;
	OS_TICK ticks;
	STATUS sts;

	if (TimerHandle > MaxSimultaneousTimer)
		return(OS_ERROR);
	timer = &TimerTable[TimerHandle].entry;
	sts = NU_Obtain_Semaphore(&TimSemCB, NU_SUSPEND);
	if (timer->status == TMR_FREE) {
		if (sts == NU_SUCCESS)
			NU_Release_Semaphore(&TimSemCB);
		return(OS_ERROR);
	}
	t_list_access = 1;
	BARRIER;
	if (timer->status == TMR_ACTIVE)
		os_remove_timer_from_list(timer);
	timer->t_handle = TimerHandle;
	timer->task_handle = os_MyHandle();
	timer->entity_handle = TaskHandle;
	timer->t_index = Index;
	timer->p_ticks = TIME_TO_SYSTEM_TICKS(RescheduleTime);
	ticks = os_add_timer_to_list(timer, TIME_TO_SYSTEM_TICKS(InitialTime));
	if (ticks)
		NU_Reset_Timer(&os_timer_cb, os_Timeout, ticks, 0,
				NU_ENABLE_TIMER);
	BARRIER;
	t_list_access = 0;
	if (sts == NU_SUCCESS)
		NU_Release_Semaphore(&TimSemCB);
	return OS_OK;
}

GLOBAL LONG os_StopTimer(OS_HANDLE TaskHandle, OS_HANDLE TimerHandle)
	/* TaskHandle argument is unused */
{
	T_OS_TIMER_ENTRY *timer_e;
	STATUS sts;

	if (TimerHandle > MaxSimultaneousTimer)
		return(OS_ERROR);
	timer_e = &TimerTable[TimerHandle];
	sts = NU_Obtain_Semaphore(&TimSemCB, NU_SUSPEND);
	if (timer_e->entry.status == TMR_FREE) {
		if (sts == NU_SUCCESS)
			NU_Release_Semaphore(&TimSemCB);
		return OS_ERROR;
	}
	t_list_access = 1;
	BARRIER;
	if (timer_e->entry.status == TMR_ACTIVE)
		os_remove_timer_from_list(&timer_e->entry);
	BARRIER;
	t_list_access = 0;
	if (sts == NU_SUCCESS)
		NU_Release_Semaphore(&TimSemCB);
	return OS_OK;
}

GLOBAL LONG os_IncrementTick(OS_TICK ticks)
{
	return OS_OK;
}

GLOBAL LONG os_DestroyTimer(OS_HANDLE TaskHandle, OS_HANDLE TimerHandle)
	/* TaskHandle argument is unused */
{
	STATUS sts;
	T_OS_TIMER_ENTRY *timer_e;

	if (TimerHandle > MaxSimultaneousTimer)
		return(OS_ERROR);
	sts = NU_Obtain_Semaphore(&TimSemCB, NU_SUSPEND);
	timer_e = &TimerTable[TimerHandle];
	if (timer_e->entry.status != TMR_USED) {
		if (sts == NU_SUCCESS)
			NU_Release_Semaphore(&TimSemCB);
		return OS_ERROR;
	}
	timer_e->next_t_handle = next_t_handle;
	next_t_handle = TimerHandle;
	timer_e->entry.status = TMR_FREE;
	used_timers--;
	if (sts == NU_SUCCESS)
		NU_Release_Semaphore(&TimSemCB);
	return OS_OK;
}

GLOBAL LONG os_CreateTimer(OS_HANDLE TaskHandle,
			   void (*TimeoutProc) (OS_HANDLE, OS_HANDLE, USHORT),
			   OS_HANDLE *TimerHandle, OS_HANDLE MemPoolHandle)
	/* TaskHandle and MemPoolHandle arguments are unused */
{
	STATUS sts;
	T_OS_TIMER_ENTRY *timer_e;

	sts = NU_Obtain_Semaphore(&TimSemCB, NU_SUSPEND);
	if (next_t_handle == 0) { /* no free timers left */
		if (sts == NU_SUCCESS)
			NU_Release_Semaphore(&TimSemCB);
		return OS_ERROR;
	}

	timer_e = &TimerTable[next_t_handle];
	timer_e->entry.status = TMR_USED;
	timer_e->entry.TimeoutProc = TimeoutProc;
	*TimerHandle = next_t_handle;
	next_t_handle = timer_e->next_t_handle;
	used_timers++;
	if (max_used_timers < used_timers)
		max_used_timers = used_timers;
	if (sts == NU_SUCCESS)
		NU_Release_Semaphore(&TimSemCB);
	return OS_OK;
}
