/*
+------------------------------------------------------------------------------
|  File:       tst_primitives.h
+------------------------------------------------------------------------------
|                 Copyright Texas Instruments 2002
|                 All rights reserved.
|
+------------------------------------------------------------------------------
| Purpose:     Definitions for the IDLE entity.
| $Identity:$
+------------------------------------------------------------------------------
*/

#ifndef TST_PRIMITIVES
#define TST_PRIMITIVES

/*==== INCLUDES ==============================================================*/

/*==== CONSTS ================================================================*/

#define IDLE_REQ   0x00000010
#define IDLE_CNF   0x00010010

#define SYSPRIM_EXT_TICK_MODE_REQ	"EXT_TICK_MODE_REQ"
#define SYSPRIM_EXT_TICK_MODE_CNF	"EXT_TICK_MODE_CNF"
#define SYSPRIM_INT_TICK_MODE_REQ	"INT_TICK_MODE_REQ"
#define SYSPRIM_INT_TICK_MODE_CNF	"INT_TICK_MODE_CNF"
#define SYSPRIM_TIMER_TICK_REQ		"TIMER_TICK_REQ"
#define SYSPRIM_TIMER_TICK_CNF		"TIMER_TICK_CNF"

#define SYSPRIM_IDLE_REQ			"IDLE_REQ"
#define SYSPRIM_IDLE_CNF			"IDLE_CNF"


/*==== TYPES =================================================================*/

#ifndef __T_IDLE_CNF__
#define __T_IDLE_CNF__
typedef struct
{
  U8                        dummy;                       /*<  0:  1>                              */
} T_IDLE_CNF;
#endif

#ifndef __T_IDLE_REQ__
#define __T_IDLE_REQ__
typedef struct
{
  U8                        dummy;                       /*<  0:  1>                              */
} T_IDLE_REQ;
#endif


/*==== EXPORTS ===============================================================*/

#endif /* TST_PRIMITIVES */
/*==== END OF FILE ===========================================================*/
