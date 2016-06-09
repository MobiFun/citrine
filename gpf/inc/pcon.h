/*
+-----------------------------------------------------------------------------
|  Project :
|  Modul   : pcon.h
+-----------------------------------------------------------------------------
|  Copyright 2002 Texas Instruments Berlin, AG
|                 All rights reserved.
|
|                 This file is confidential and a trade secret of Texas
|                 Instruments Berlin, AG
|                 The receipt of or possession of this file does not convey
|                 any rights to reproduce or disclose its contents or to
|                 manufacture, use, or sell anything it may describe, in
|                 whole, or in part, without the specific written consent of
|                 Texas Instruments Berlin, AG.
+-----------------------------------------------------------------------------
|  Purpose :  Condat Coder Decoder For Primitives
|             Global function prototypes
+-----------------------------------------------------------------------------
*/


#ifndef PCON_H
#define PCON_H

#define PCON_LITTLE            1
#define PCON_BIG               2

#define PCON_OK              0
#define PCON_INVALID_OPC     1      /* operation code of primitive not defined */
#define PCON_INVALID_CNT     2      /* wrong number of entries of an variable long array */
#define PCON_INVALID_OFFS    3      /* wrong sum of l_buf and o_buf of an sdu */
#define PCON_INVALID_TYPE    4      /* wrong entry in pvar table */
#define PCON_INVALID_INIT    5      /* wrong initialization */
#define PCON_INVALID_UTAG    6      /* union controller too large */
#define PCON_NO_MEM          7      /* out of memory */
#define PCON_CCDDATA_ERROR   8      /* wrong ccddata format */
#define PCON_INVALID_PTR     9      /* user supplied null pointer for a
                                       mandatory element */
#define PCON_INVALID_VALFLAG 10     /* user supplied valid flag that is neither
                                       1 nor 0 */
#define PCON_INVALID_VALUE   11     /* user supplied value for basetype not
                                       defined in specification */
#define PCON_INVALID_PATH    12     /* component path for prim not found */

#define PCON_CONFIG_PRIM     23     /* primitive was for internal use of PCON,
                                     * not to be relayed to stack
                                     */
#ifdef _TOOLS_
#define PCON_DLLFUNC_ERROR   24      /* could not find a function in pcon.dll */
#define PCON_STRING_END      25      /* command string end reached */
#endif /* _TOOLS_ */

#define PCON_NOT_PRES        0xFFFF
#define PCON_NOT_INCL        0xFFFE

#if !defined (CCDDATA_PREF)
#if defined (_TOOLS_) && defined (CCDDATA_LOAD)
#define CCDDATA_PREF(pcon_fun) cddl_##pcon_fun
#else
#define CCDDATA_PREF(pcon_fun) pcon_fun
#endif /* _TOOLS_ && CCDDATA_LOAD */
#endif /* !CCDDATA_PREF */

typedef struct
{
#ifdef _TOOLS_
  ULONG (*init_prim_coding) (T_HANDLE, UBYTE);
  ULONG (*decode_prim)      (ULONG, void **, void *, ULONG *, ULONG);
  ULONG (*code_prim)        (ULONG, void *, void **, ULONG *, ULONG, char*);
  ULONG (*make_filter)      (char *, void **);
#else
  ULONG (*init_prim_coding) (T_HANDLE, UBYTE);
  ULONG (*decode_prim)      (ULONG, void **, void *, ULONG *, ULONG);
  ULONG (*code_prim)        (ULONG, void *, void **, ULONG *, ULONG, char*);
  ULONG (*pcheck)           (ULONG, void *);
#endif
  int stack_offset;
} T_PCON_PROPERTIES;

typedef struct
{
  USHORT struct_level;  /* the substructure level within the structure, where
                         * the counting started from */
  USHORT cnt_sub_elems; /* the counter of structure subelements */
  BOOL   count_subs ;   /* flag whether sub elements should be counted */
} T_SUB_ELEM_COUNT;

#define PCON_STACK_OFFSET   3072


#ifndef PCON_C
/*
 * function prototypes
 */
extern ULONG CCDDATA_PREF(pcon_init_prim_coding)(T_HANDLE caller, UBYTE format);

extern ULONG CCDDATA_PREF(pcon_decodePrim) (ULONG opc,
                                            void ** decoded_prim,
                                            void * coded_prim,
                                            ULONG * length,
                                            ULONG woff);

extern ULONG CCDDATA_PREF(pcon_codePrim)  (ULONG opc,
                                           void * decoded_prim,
                                           void ** coded_prim,
                                           ULONG * length,
                                           ULONG woff,
                                           char* receiver);

#ifdef _TOOLS_
extern ULONG CCDDATA_PREF(pcon_init_ccddata)(void);
/*
 * The format for the command string in pcon_make_filter is:
 * [<receiver> [<opc>|<primname> [+|-<elem_pathnames>]]]
 * With <elem_pathnames>: <elem_pathname>[,<elem_pathnames]
 * and <elem_pathname> is the unique path of and element in a
 * primitive, with the structure member names separated by '.'.
 * Examples:
 * "TAP ENT_FOO_IND +struct_3.elem_2,struct_1.substruct_0.elem_1" - Set filter
 *   if primitive ENT_FOO_IND if sent to TAP for the two elements elem_2 in
 *   struct_3 and elem_1 in substruct_0 in struct_1.
 * "TAP 800000AF1 +struct_3.elem_2,struct_1.substruct_0.elem_1" - same for given
 *   opc instead of prim name.
 * "TAP ENT_FOO_IND -struct_3.elem_2,struct_1.substruct_0.elem_1" - Set filter
 *   for all other elements than two given here
 * "TAP ENT_FOO_IND" - Delete filter setting for primitive ENT_FOO_IND
 *   sent to TAP
 * "TAP" - Delete all filter settings for any primitive sent to TAP
 * "" - Delete all filter settings
 */
extern ULONG CCDDATA_PREF(pcon_make_filter)(char* string, void** prim);
extern ULONG CCDDATA_PREF(pcon_filter_decode_prim) (ULONG opc,
                                                    void** decoded_prim,
                                                    void* coded_prim,
                                                    ULONG* length,
                                                    ULONG woff,
                                                    void** shadow_prim);
#else
extern ULONG pcon_pcheck  (ULONG opc, void * decoded_prim);
#endif /* _TOOLS_ */

#endif /* !PCON_C */

#endif
