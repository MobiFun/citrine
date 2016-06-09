/**********************************************************
 *        GSM/GPRS TI S/W software 
 *
 *        Filename sys_types.h
 *
 *
 *	Redefine the types for the TI GSM/GPRS system software
 *
 **********************************************************/

#ifndef __SYS_TYPES_H__
#define __SYS_TYPES_H__

typedef unsigned char  SYS_BOOL;

typedef unsigned char  SYS_UWORD8;
typedef signed   char  SYS_WORD8;

typedef unsigned short SYS_UWORD16;
typedef          short SYS_WORD16;

typedef unsigned long  SYS_UWORD32;
typedef          long  SYS_WORD32;


typedef void           (*SYS_FUNC)(void);		/* pointer to a function */

#endif /* __SYS_TYPES_H__ */
