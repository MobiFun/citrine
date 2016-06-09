/*
 * I personally like the u8/u16/u32 types, but I don't see them
 * being defined in any of the headers provided by newlib.
 * So we'll define them ourselves.
 */

#ifndef	__OUR_OWN_TYPES_H
#define	__OUR_OWN_TYPES_H

typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;

typedef signed char s8;
typedef signed short s16;
typedef signed int s32;

#endif	/* include guard */
