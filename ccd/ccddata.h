/* 
+----------------------------------------------------------------------------- 
|  Project :  
|  Modul   : ccddata.h 
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
|  Purpose :  Exports of the Ccddata Lib/Dll.
+----------------------------------------------------------------------------- 
*/ 

#ifndef CCDDATA_H
#define CCDDATA_H

/*==== INCLUDES =============================================================*/
#include "pdi.h"

/*==== CONSTS ===============================================================*/
#define CCDDATA_CCDENT_INVALID -1
#define CCDDATA_MCONST_ERROR   -1
#define CCDDATA_PCONST_ERROR   -1

#define CCDDATA_DLL_OK          0
#define CCDDATA_DLL_ALREADY   -10
#define CCDDATA_DLL_LOADLIB   -11
#define CCDDATA_DLL_LOADFUN   -12
#define CCDDATA_DLL_REGISTER  -13

/*==== TYPES =================================================================*/
#if defined __CCD_GLOBS_H
typedef	SHORT (*T_FUNC_POINTER)(const ULONG cref, const ULONG eref, T_CCD_Globs *globs);
typedef struct
{
  T_FUNC_POINTER enc;
  T_FUNC_POINTER dec;
} T_FUNC_STRUCT;
#endif

typedef struct
{
  int p1;
  int p2;
} T_COMENDPOINTS;
/*==== EXPORTS ===============================================================*/

/* Use CCDDATA_DLL_FUNC only for optmizations. The extern declarations
 * also work if the function are in a dll.
 */
#ifndef CCDDATA_DLL_FUNC
 #define CCDDATA_IMPORT_FUNC extern
#else /* CCDDATA_DLL_FUNC */
 #define CCDDATA_IMPORT_FUNC extern __declspec(dllimport)
#endif /* CCDDATA_DLL_FUNC */

/* ccddata_load.c */
extern int ccddata_init (const char* dllname,
                         int reload,
                         void (*ccddata_init_notify)(void),
                         void (*ccddata_exit_notify)(void));
extern int ccddata_exit (void);
extern char* ccddata_dllname (void);

/* ccd_config.c */
#if defined(CCD_C) || defined(PDI_C)
UBYTE* ccddata_get_mi_length (void);
#endif
#ifdef CCD_C
UBYTE cdc_init (T_FUNC_POINTER codec[][2]);
UBYTE* ccddata_get_decmsgbuffer (void);
#endif

/* ccddata_pdi.c */
CCDDATA_IMPORT_FUNC int ccddata_get_pdi_dinfo (const T_PDI_DECODEINFO* (*dinfo) );
/* ccddata_mconst.c */
CCDDATA_IMPORT_FUNC int ccddata_get_num_of_entities(void);
CCDDATA_IMPORT_FUNC int ccddata_get_max_message_id(void);
CCDDATA_IMPORT_FUNC int ccddata_get_max_bitstream_len(void);
CCDDATA_IMPORT_FUNC int ccddata_get_max_mstruct_len(void);
CCDDATA_IMPORT_FUNC int ccddata_mccd_symbols(void);
/* ccddata_pconst.c */
CCDDATA_IMPORT_FUNC int ccddata_get_max_sap_num(void);
CCDDATA_IMPORT_FUNC int ccddata_get_max_primitive_id(void);
CCDDATA_IMPORT_FUNC int ccddata_get_max_pstruct_len(void);
/* ccddata_ccdmtab.c */
CCDDATA_IMPORT_FUNC const T_CCD_VarTabEntry*  ccddata_get_mvar (USHORT idx);
CCDDATA_IMPORT_FUNC const T_CCD_SpareTabEntry* ccddata_get_spare (USHORT idx);
CCDDATA_IMPORT_FUNC const T_CCD_CalcTabEntry* ccddata_get_calc (USHORT idx);
CCDDATA_IMPORT_FUNC const T_CCD_CompTabEntry* ccddata_get_mcomp (USHORT idx);
CCDDATA_IMPORT_FUNC const T_CCD_ElemTabEntry* ccddata_get_melem (USHORT idx);
CCDDATA_IMPORT_FUNC const T_CCD_CalcIndex* ccddata_get_calcidx (USHORT idx);
CCDDATA_IMPORT_FUNC USHORT ccddata_get_mmtx (USHORT entity,
                                        USHORT msgid,
                                        USHORT index); 
/* ccddata_ccdptab.c */
CCDDATA_IMPORT_FUNC const T_CCD_VarTabEntry* ccddata_get_pvar (USHORT idx);
CCDDATA_IMPORT_FUNC const T_CCD_CompTabEntry* ccddata_get_pcomp (USHORT idx);
CCDDATA_IMPORT_FUNC const T_CCD_ElemTabEntry* ccddata_get_pelem (USHORT idx);
CCDDATA_IMPORT_FUNC USHORT ccddata_get_pmtx (USHORT sap,
                                        USHORT primid,
                                        USHORT index);
/* ccddata_cdemval.c */
CCDDATA_IMPORT_FUNC const T_CCD_ValTabEntry* ccddata_get_mval (USHORT idx);
CCDDATA_IMPORT_FUNC const T_CCD_StrTabEntry* ccddata_get_mstr (USHORT idx);
/* ccddata_cdepval.c */
CCDDATA_IMPORT_FUNC const T_CCD_ValTabEntry* ccddata_get_pval (USHORT idx);
CCDDATA_IMPORT_FUNC const T_CCD_StrTabEntry* ccddata_get_pstr (USHORT idx);

/* ccddata_ccdent.c */
#ifndef CCDDATA_CCDENT_C
CCDDATA_IMPORT_FUNC short ccddata_get_ccdent (char* entname);
CCDDATA_IMPORT_FUNC const char * ccddata_get_entname (short ccdent);
#endif /* !CCDDATA_CCDENT_C*/

/* ccddata_alias.c */
CCDDATA_IMPORT_FUNC char* ccddata_get_alias (USHORT idx, int from_msg);

/* ccddata_version.c */
CCDDATA_IMPORT_FUNC char* ccddata_get_version ();
CCDDATA_IMPORT_FUNC int   ccddata_get_table_version ();

/* ccddata_eg.c */
#ifndef CCDDATA_EG_C
CCDDATA_IMPORT_FUNC int    ccddata_eg_nodes (void);
CCDDATA_IMPORT_FUNC char** ccddata_eg_nodenames (void);
CCDDATA_IMPORT_FUNC char*  ccddata_eg_adjacent (int idx);
CCDDATA_IMPORT_FUNC int    ccddata_eg_saps (void);
CCDDATA_IMPORT_FUNC char** ccddata_eg_sapnames (void);
CCDDATA_IMPORT_FUNC T_COMENDPOINTS*  ccddata_eg_comendpoints (int idx);
#endif /* !CCDDATA_EG_C */

#endif /* !CCDDATA_H */
