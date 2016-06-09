/*
 * This C module is a reconstruction based on the disassembly of
 * os_tim.obj in frame_na7_db_ir.lib from the Leonardo package.
 */

/* set of included headers from COFF symtab: */
#include <stdio.h>
#include "gpfconf.h"		/* FreeCalypso addition */
#include "../../nucleus/nucleus.h"
#include "typedefs.h"
#include "os.h"
#include "gdi.h"
#include "os_types.h"
#include "os_glob.h"

extern unsigned os_time_to_tick_multiplier;
extern unsigned os_tick_to_time_multiplier;

extern T_OS_TIMER_ENTRY TimerTable[];
extern T_OS_TIMER_TABLE_ENTRY *p_list[];

extern int volatile t_list_access;
extern unsigned t_start_ticks;
extern T_OS_TIMER_TABLE_ENTRY * volatile t_running;
extern int used_timers;
extern int next_t_handle;
extern NU_SEMAPHORE TimSemCB;
extern NU_TIMER os_timer_cb;
extern int max_used_timers;

void os_Timeout(UNSIGNED t_handle);

void timer_error(int err)
{
	err = 0;
}

static int os_remove_timer_from_list(T_OS_TIMER_TABLE_ENTRY *timer)
{
	OS_TICK c_ticks;

	if (timer != t_running) {
		if (timer->next != t_running)
			timer->next->r_ticks += timer->r_ticks;
	}
	else {
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
	timer->status = 1;
	return 1;
}

static unsigned os_add_timer_to_list(T_OS_TIMER_TABLE_ENTRY *timer, OS_TICK ticks)
{
	T_OS_TIMER_TABLE_ENTRY *t_list;
	OS_TICK c_ticks, r1_ticks, return_ticks;

	if (ticks == 0)
		ticks = 1;

	c_ticks = NU_Retrieve_Clock();
	t_list = t_running;
	if (t_list != NULL) {
		if (t_running->r_ticks >= c_ticks - t_start_ticks) {
			r1_ticks =
			    t_running->r_ticks - c_ticks + t_start_ticks;
			t_running->r_ticks = r1_ticks;
		} else {
			r1_ticks = 0;
			t_running->r_ticks = 0;
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
	timer->status = 2;
	return return_ticks;
}

GLOBAL LONG os_StartTimer(OS_HANDLE TaskHandle, OS_HANDLE TimerHandle,
				USHORT Index, OS_TIME InitialTime,
				OS_TIME RescheduleTime)
{
	T_OS_TIMER_TABLE_ENTRY *timer;
	OS_TICK ticks;
	USHORT status;
	STATUS sts;

	t_list_access = 1;
	timer = &TimerTable[TimerHandle].entry;
	if (TimerHandle > MaxSimultaneousTimer || timer->status == 0) {
		t_list_access = 0;
		return OS_ERROR;
	}

	sts = NU_Obtain_Semaphore(&TimSemCB, NU_SUSPEND);
	/* FIXME: not sure about this
	if (sts != NU_SUCCESS)
		os_MyHandle(sts);
	*/
	status = timer->status;
	if (status == 2)
		status = os_remove_timer_from_list(timer);
	timer->t_handle = TimerHandle;
	timer->task_handle = os_MyHandle();
	timer->entity_handle = TaskHandle;
	timer->t_index = Index;
	timer->p_ticks = TIME_TO_SYSTEM_TICKS(RescheduleTime);
	ticks = os_add_timer_to_list(timer, TIME_TO_SYSTEM_TICKS(InitialTime));
	if (ticks)
		NU_Reset_Timer(&os_timer_cb, os_Timeout, ticks, 0, NU_ENABLE_TIMER);
	if (sts == NU_SUCCESS)
		NU_Release_Semaphore(&TimSemCB);
	t_list_access = 0;
	return OS_OK;
}

/* FIXME: t_handle is unused?! */
void os_Timeout(UNSIGNED t_handle)
{
	UNSIGNED s_ticks;
	OS_HANDLE task_handle;
	OS_HANDLE e_handle;
	int t_index, i, done;
	T_OS_TIMER_TABLE_ENTRY **t_r4;
	T_OS_TIMER_TABLE_ENTRY *timer;
	void (*timeout_func) (OS_HANDLE, OS_HANDLE, USHORT);
	OS_TIME InitialTime;

	if (t_list_access) {
		t_start_ticks++;
		NU_Reset_Timer(&os_timer_cb, os_Timeout, 1, 0,
				NU_ENABLE_TIMER);
		return;
	}

	t_list_access = 1;
	timer = t_running;
	if (t_running) {
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
			timer->status = 1;
			if (timer->next == timer) {
				t_running = NULL;
				done = 1;
			} else {
				timer->prev->next = timer->next;
				timer->next->prev = timer->prev;
				if (timer->next->r_ticks) {
					t_running = timer->next;
					s_ticks = timer->r_ticks;
					done = 1;
				} else
					timer = timer->next;
			}
			/* FIXME: IND$CALL() ? */
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
		InitialTime = SYSTEM_TICKS_TO_TIME(timer->p_ticks);
		os_StartTimer(timer->entity_handle, timer->t_handle,
				timer->t_index, InitialTime, InitialTime);
		*t_r4 = NULL;
	}

	t_list_access = 0;
	return;
}

/* FIXME: TaskHandle is unused?! */
GLOBAL LONG os_StopTimer(OS_HANDLE TaskHandle, OS_HANDLE TimerHandle)
{
	T_OS_TIMER_ENTRY *timer_e;
	STATUS sts;

	t_list_access = 1;
	timer_e = &TimerTable[TimerHandle];
	if (TimerHandle > MaxSimultaneousTimer || timer_e->entry.status == 0) {
		t_list_access = 0;
		return OS_ERROR;
	}
	sts = NU_Obtain_Semaphore(&TimSemCB, NU_SUSPEND);
	if (timer_e->entry.status == 2)
		os_remove_timer_from_list(&timer_e->entry);
	if (sts == NU_SUCCESS)
		NU_Release_Semaphore(&TimSemCB);
	t_list_access = 0;
	return OS_OK;
}

GLOBAL LONG os_IncrementTick(OS_TICK ticks)
{
	return OS_OK;
}

/* FIXME: TaskHandle is unused?! */
GLOBAL LONG os_DestroyTimer(OS_HANDLE TaskHandle, OS_HANDLE TimerHandle)
{
	STATUS sts;
	T_OS_TIMER_ENTRY *timer_e;

	t_list_access = 1;
	sts = NU_Obtain_Semaphore(&TimSemCB, NU_SUSPEND);
	timer_e = &TimerTable[TimerHandle];
	if (TimerHandle > MaxSimultaneousTimer || timer_e->entry.status == 0) {
		if (sts == NU_SUCCESS)
			NU_Release_Semaphore(&TimSemCB);
		t_list_access = 0;
		return OS_ERROR;
	}
	timer_e->next_t_handle = next_t_handle;
	timer_e->entry.status = 0;
	used_timers--;
	t_list_access = 0;
	if (sts == NU_SUCCESS)
		NU_Release_Semaphore(&TimSemCB);
	return OS_OK;
}

/* FIXME: TaskHandle and MemPoolHandle are unused?! */
GLOBAL LONG os_CreateTimer(OS_HANDLE TaskHandle,
			   void (*TimeoutProc) (OS_HANDLE, OS_HANDLE, USHORT),
			   OS_HANDLE *TimerHandle, OS_HANDLE MemPoolHandle)
{
	STATUS sts;
	OS_HANDLE orig_next_t_handle;
	T_OS_TIMER_ENTRY *timer_e;

	t_list_access = 1;
	sts = NU_Obtain_Semaphore(&TimSemCB, NU_SUSPEND);
	orig_next_t_handle = next_t_handle;
	if (next_t_handle == 0) { /* INVALID_HANDLE */
		if (sts == NU_SUCCESS)
			NU_Release_Semaphore(&TimSemCB);
		t_list_access = 0;
		return OS_ERROR;
	}

	timer_e = &TimerTable[next_t_handle];
	timer_e->entry.status = 1;
	timer_e->entry.TimeoutProc = TimeoutProc;
	*TimerHandle = orig_next_t_handle;
	next_t_handle = timer_e->next_t_handle;
	used_timers++;
	if (max_used_timers < used_timers)
		max_used_timers = used_timers;
	if (sts == NU_SUCCESS)
		NU_Release_Semaphore(&TimSemCB);
	t_list_access = 0;
	return OS_OK;
}
