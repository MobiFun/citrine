/*
 * This C module is a reconstruction based on the disassembly of
 * osx.obj in osx_na7_db.lib from the Leonardo package.
 */

/* reconstructed set of included headers from COFF symtab: */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "typedefs.h"
#include "vsi.h"
#include "cust_os.h"

int osx_config = 2;
int _caller;
T_OSX_REGISTER _osx_registry[MAX_OSX_QUEUE];

void
int_osx_send_sig(int caller, unsigned long opc, void *signal_ptr,
		 int queue_handle)
{
	T_QMSG Message;
	int status;

	TRACE_ASSERT(queue_handle != 0);
	Message.MsgType = MSG_SIGNAL;
	Message.Msg.Signal.SigOPC = opc;
	Message.Msg.Signal.SigBuffer = signal_ptr;
	Message.Msg.Signal.SigLen = sizeof(xSignalHeaderRec);
	status = vsi_c_send(caller, queue_handle, &Message FILE_LINE_MACRO);
	TRACE_ASSERT(status == VSI_OK);
}

void
osx_send_sig(unsigned long opc, void *signal_ptr, T_ENUM_OS_QUEUE queue_type)
{
	int caller, queue_handle;

	caller = _osx_registry[queue_type].caller;
	queue_handle = _osx_registry[queue_type].queue_handle;
	TRACE_ASSERT(queue_handle != 0);
	int_osx_send_sig(caller, opc, signal_ptr, queue_handle);
}

void
int_osx_free_prim(int caller, xSignalHeaderRec *prim_ptr)
{
	vsi_c_free(caller, (T_VOID_STRUCT **) &prim_ptr FILE_LINE_MACRO);
}

void
int_osx_send_prim(int caller, xSignalHeaderRec *prim_ptr, int queue_handle)
{
	T_QMSG Message;
	int status;

	if (osx_config & 1 && prim_ptr->SignalCode == 0x7D) {
		vsi_c_free(caller, (T_VOID_STRUCT **) &prim_ptr
				FILE_LINE_MACRO);
		return;
	}
	TRACE_ASSERT(queue_handle != 0);
	Message.MsgType = MSG_PRIMITIVE;
	Message.Msg.Primitive.Prim = (T_VOID_STRUCT *) prim_ptr;
	Message.Msg.Primitive.PrimLen = sizeof(xSignalHeaderRec);
	status = vsi_c_send(caller, queue_handle, &Message FILE_LINE_MACRO);
	TRACE_ASSERT(status == VSI_OK);
}

void
osx_send_prim(xSignalHeaderRec *prim_ptr, T_ENUM_OS_QUEUE queue_type)
{
	int_osx_send_prim(_osx_registry[queue_type].caller, prim_ptr,
			  _osx_registry[queue_type].queue_handle);
}

xSignalHeaderRec *
int_osx_receive_prim(int caller, int queue_handle)
{
	T_QMSG Message;
	unsigned opc;
	xSignalHeaderRec *message;
	int status;

	TRACE_ASSERT(queue_handle != OSX_ERROR);
	for (;;) {
		status = vsi_c_await(caller, queue_handle, &Message,
					0xFFFFFFFF);
		TRACE_ASSERT(status == VSI_OK);
		/*
		 * Disassembly reveals that the original code expects
		 * the received message to be a primitive, rather than
		 * a signal or a timeout.  If one of the latter comes
		 * in, the original code would go haywire.  Hence the
		 * following TRACE_ASSERT is a FreeCalypso addition.
		 */
		TRACE_ASSERT(Message.MsgType == MSG_PRIMITIVE);
		message = (xSignalHeaderRec *) Message.Msg.Primitive.Prim;
		opc = message->SignalCode & 0xFFFF;
		if (opc == 11 && osx_config & 2)
			message->SigP = *(DummyStruct **)(message + 1);
		else
			message->SigP = (DummyStruct *)(message + 1);
		if (opc != 0x8000)
			return(message);
		vsi_c_primitive(caller, message);
	}
}

xSignalHeaderRec *
osx_receive_prim(T_ENUM_OS_QUEUE queue_type)
{
	return int_osx_receive_prim(_osx_registry[queue_type].caller,
				    _osx_registry[queue_type].queue_handle);
}

void
osx_free_prim(xSignalHeaderRec *prim_ptr)
{
	vsi_c_free(_caller, (T_VOID_STRUCT **) &prim_ptr FILE_LINE_MACRO);
}

void
int_osx_free_mem(int caller, void *mem_ptr)
{
	int status;

	TRACE_ASSERT(mem_ptr != 0);
	status = D_FREE(mem_ptr);
	TRACE_ASSERT(status == VSI_OK);
}

void
osx_free_mem(void *mem_ptr)
{
	int_osx_free_mem(_caller, mem_ptr);
}

xSignalHeaderRec *
int_osx_alloc_prim(int caller, unsigned long len, int pool_group_handle)
{
	xSignalHeaderRec *prim_ptr;

	prim_ptr = (xSignalHeaderRec *) vsi_c_new(caller,
			(len & 0xFFFF) + sizeof(xSignalHeaderRec), 0
			FILE_LINE_MACRO);
	/* This check is a FreeCalypso addition */
	TRACE_ASSERT(prim_ptr != 0);
	prim_ptr->SigP = (DummyStruct *)(prim_ptr + 1);
	return(prim_ptr);
}

xSignalHeaderRec *
osx_alloc_prim(unsigned long len)
{
	xSignalHeaderRec *prim_ptr;

	prim_ptr = (xSignalHeaderRec *) vsi_c_new(_caller,
			(len & 0xFFFF) + sizeof(xSignalHeaderRec), 0
			FILE_LINE_MACRO);
	/* This check is a FreeCalypso addition */
	TRACE_ASSERT(prim_ptr != 0);
	prim_ptr->SigP = (DummyStruct *)(prim_ptr + 1);
	return(prim_ptr);
}

void *
int_osx_alloc_mem(int caller, unsigned long len)
{
	void *mem_ptr;

	DMALLOC(mem_ptr, len);
	TRACE_ASSERT(mem_ptr != 0);
	return(mem_ptr);
}

void *
osx_alloc_mem(unsigned long len)
{
	return int_osx_alloc_mem(_caller, len);
}

int
_osx_open(int caller, unsigned short queue_type, int queue_handle)
{
	if (!queue_type && !queue_handle) {
		_caller = caller;
		return(OSX_ERROR);
	}
	if (queue_type >= MAX_OSX_QUEUE)
		return(OSX_ERROR);
	if (_osx_registry[queue_type].queue_handle != queue_handle &&
	    _osx_registry[queue_type].queue_handle != -1)
		return(OSX_ERROR);
	_osx_registry[queue_type].queue_handle = queue_handle;
	_osx_registry[queue_type].caller = caller;
	return(OSX_OK);
}

void
_osx_init(void)
{
	USHORT i;

	for (i = 0; i < MAX_OSX_QUEUE; i++)
		_osx_registry[i].queue_handle = -1;
}

int
_osx_config(const char *config)
{
	if (!strcmp(config, L1S_TRACE_DISABLE)) {
		osx_config |= 1;
		return(OSX_OK);
	}
	if (!strcmp(config, L1S_TRACE_ENABLE)) {
		osx_config &= ~1;
		return(OSX_OK);
	}
	if (!strcmp(config, NO_SPECIAL_MPHC_RXLEV_REQ)) {
		osx_config &= ~2;
		return(OSX_OK);
	}
	return(OSX_ERROR);
}
