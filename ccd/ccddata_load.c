/* 
+----------------------------------------------------------------------------- 
|  Project :  
|  Modul   : ccddata_load.c
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
|  Purpose :  Manage the explicit dynamic loading of ccddata.
+----------------------------------------------------------------------------- 
*/

/*==== INCLUDES ==============================================================*/
#include <windows.h>
#include <string.h>
#include <stdio.h>
#include "typedefs.h"
#include "vsi.h"
#include "ccdtable.h"
#include "ccdapi.h"
#include "pdi.h"
#include "pcon.h"
#include "ccdedit.h"
#include "ccddata.h"
#include "ccddata_tap_priv.h"
/*==== CONSTS ================================================================*/
#define MAXPROC 16
/*==== TYPES =================================================================*/
/* pcon.c */
typedef ULONG (*T_pcon_init_prim_coding)(T_HANDLE caller, UBYTE format);
typedef ULONG (*T_pcon_decodePrim)(ULONG opc,
                                    void ** decoded_prim,
                                    void * coded_prim,
                                    ULONG * length,
                                    ULONG woff);
typedef ULONG (*T_pcon_codePrim)(ULONG opc,
                                  void * decoded_prim,
                                  void ** coded_prim,
                                  ULONG * length,
                                  ULONG woff,
                                  char* receiver);
typedef ULONG (*T_pcon_init_ccddata)(void);
typedef ULONG (*T_pcon_make_filter)(char* string, void** prim);
typedef ULONG (*T_pcon_filter_decode_prim)(ULONG opc,
                                           void** decoded_prim,
                                           void* coded_prim,
                                           ULONG* length,
                                           ULONG woff,
                                           void** shadow_prim);
/* ccddata_pdi.c */
typedef int (*T_ccddata_get_pdi_dinfo)(const T_PDI_DECODEINFO* (*dinfo) );
/* ccddata_mconst.c */
typedef int (*T_ccddata_get_num_of_entities)(void);
typedef int (*T_ccddata_get_max_message_id)(void);
typedef int (*T_ccddata_get_max_bitstream_len)(void);
typedef int (*T_ccddata_get_max_mstruct_len)(void);
typedef int (*T_ccddata_get_max_mstruct_len)(void);
typedef int (*T_ccddata_mccd_symbols)(void);
/* ccddata_pconst.c */
typedef int (*T_ccddata_get_max_sap_num)(void);
typedef int (*T_ccddata_get_max_primitive_id)(void);
typedef int (*T_ccddata_get_max_pstruct_len)(void);
/* ccddata_ccdmtab.c */
typedef const T_CCD_VarTabEntry*   (*T_ccddata_get_mvar)  (USHORT idx);
typedef const T_CCD_SpareTabEntry* (*T_ccddata_get_spare) (USHORT idx);
typedef const T_CCD_CalcTabEntry*  (*T_ccddata_get_calc)  (USHORT idx);
typedef const T_CCD_CompTabEntry*  (*T_ccddata_get_mcomp) (USHORT idx);
typedef const T_CCD_ElemTabEntry*  (*T_ccddata_get_melem) (USHORT idx);
typedef const T_CCD_CalcIndex*     (*T_ccddata_get_calcidx) (USHORT idx);
typedef USHORT (*T_ccddata_get_mmtx) (USHORT entity, USHORT msgid, USHORT idx);
/* ccddata_ccdptab.c */
typedef const T_CCD_VarTabEntry*  (*T_ccddata_get_pvar)  (USHORT idx);
typedef const T_CCD_CompTabEntry* (*T_ccddata_get_pcomp) (USHORT idx);
typedef const T_CCD_ElemTabEntry* (*T_ccddata_get_pelem) (USHORT idx);
typedef USHORT (*T_ccddata_get_pmtx) (USHORT sap, USHORT primid, USHORT idx);
/* ccddata_cdemval.c */
typedef const T_CCD_ValTabEntry* (*T_ccddata_get_mval) (USHORT idx);
/* ccddata_cdemstr.c */
typedef const T_CCD_StrTabEntry* (*T_ccddata_get_mstr) (USHORT idx);
/* ccddata_cdepval.c */
typedef const T_CCD_ValTabEntry* (*T_ccddata_get_pval) (USHORT idx);
typedef const T_CCD_StrTabEntry* (*T_ccddata_get_pstr) (USHORT idx);
/* ccddata_ccdent.c */
typedef short (*T_ccddata_get_ccdent) (char* entname);
typedef const char * (*T_ccddata_get_entname) (short ccdent);
/* ccddata_alias.c */
typedef char* (*T_ccddata_get_alias) (USHORT idx, int from_msg);
/* ccddata_version.c */
typedef char* (*T_ccddata_get_version) ();
typedef int (*T_ccddata_get_table_version) ();
/* ccd_config.c */
typedef UBYTE* (*T_ccddata_get_mi_length) (void);
typedef UBYTE* (*T_ccddata_get_decmsgbuffer) (void);
/* ccddata_tap_priv.c */
typedef int (*T_ccddata_tap_get_pd) (UCHAR comp);
typedef int (*T_ccddata_tap_check_pd) (UCHAR comp, UCHAR pd);
/* ccddata_eg.c */
typedef int (*T_ccddata_eg_nodes) (void);
typedef char** (*T_ccddata_eg_nodenames) (void);
typedef char* (*T_ccddata_eg_adjacent) (int idx);
typedef int (*T_ccddata_eg_saps) (void);
typedef char** (*T_ccddata_eg_sapnames) (void);
typedef T_COMENDPOINTS* (*T_ccddata_eg_comendpoints) (int idx);
/* ccdedit.c */
typedef void (*T_cde_init) (void);
typedef USHORT (*T_cde_get_comp) (T_CCDE_HANDLE* chandle,
                                  T_CCDE_ELEM_DESCR* edescr);
typedef USHORT (*T_cde_prim_first) (T_CCDE_HANDLE* phandle,
                                    ULONG primcode,
                                    char* name);
typedef USHORT (*T_cde_prim_next) (T_CCDE_HANDLE* phandle,
                                   UBYTE descent,
                                   T_CCDE_ELEM_DESCR* pdescr);
typedef USHORT (*T_cde_msg_first) (T_CCDE_HANDLE* mhandle,
                                  UBYTE type,
                                  UBYTE direction,
                                  UBYTE entity,
                                  char* name);
typedef USHORT (*T_cde_msg_next) (T_CCDE_HANDLE* mhandle,
                                  UBYTE descent,
                                  T_CCDE_ELEM_DESCR* iedesrc);
typedef USHORT (*T_cde_comp_first) (T_CCDE_HANDLE* chandle,
                                    T_ELM_SRC source,
                                    char* compname);
typedef USHORT (*T_cde_comp_next) (T_CCDE_HANDLE* chandle,
                                   UBYTE descent,
                                   T_CCDE_ELEM_DESCR* descr);
typedef char* (*T_cde_get_symval) (int elem_value,
                                   T_CCDE_ELEM_DESCR* descr);
typedef USHORT (*T_cde_read_elem) (T_CCDE_HANDLE* handle,
                                   void* cstruct,
                                   T_CCDE_ELEM_DESCR* descr,
                                   UBYTE* value);
typedef void (*T_cde_write_prepare) (T_CCDE_HANDLE* handle,
                                     void* cstruct,
                                     T_CCDE_ELEM_DESCR* descr);
typedef USHORT (*T_cde_write_elem) (T_CCDE_HANDLE* handle,
                                    void* cstruct,
                                    T_CCDE_ELEM_DESCR* descr,
                                    UBYTE* value);
typedef USHORT (*T_cde_get_type) (char* name, T_ELM_SRC* source);
typedef USHORT (*T_cde_get_primcode) (char* name, ULONG* primcode);
typedef USHORT (*T_cde_get_msgcode) (char* name,
                                     UBYTE* type,
                                     UBYTE* direction,
                                     UBYTE* entity);
typedef int (*T_cde_get_is_downlink) (ULONG comp_index);
typedef ULONG (*T_cde_get_comp_index) (char* comp_name, T_ELM_SRC table);
typedef char* (*T_cde_get_element_name) (ULONG comp_index,
                                         USHORT elem_off,
                                         T_ELM_SRC table);
typedef ULONG (*T_cde_get_array_kind) (char* var_name, T_ELM_SRC table);
/* pdi.c */
typedef T_PDI_CONTEXT* (*T_pdi_createDefContext) ();
typedef T_PDI_CONTEXT* (*T_pdi_createContext) (const T_PDI_DECODEINFO* dinfop,
                                                unsigned int dicount);
typedef void (*T_pdi_destroyContext) (T_PDI_CONTEXT* context);
typedef void (*T_pdi_startPrim) (T_PDI_CONTEXT* context, ULONG opc);
typedef void (*T_pdi_getDecodeInfo) (T_PDI_CONTEXT* context,
                                       const char* ename,
                                       char* evalue,
                                       int evlen,
                                       T_PDI* decinfo);
typedef short (*T_pdi_getEntityByPD) (const T_PDI_CONTEXT* context,
                                         unsigned char pd);
typedef const char* (*T_pdi_pd2name) (unsigned char pd);
/* ccd.c */
typedef int   (*T_ccd_set_patch_infos) (T_patch_info* pinfo);
typedef BYTE  (*T_ccd_init) (void);
typedef int   (*T_ccd_exit) (void);
typedef UBYTE* (*T_ccd_begin) (void);
typedef void  (*T_ccd_end) (void);
typedef BYTE  (*T_ccd_decodeMsg) (UBYTE entity,
                                  UBYTE direction,
                                  T_MSGBUF *mBuf,
                                  UBYTE *mStruct,
                                  UBYTE pt);
typedef S8   (*T_ccd_decodeMsgPtr) (U8 entity,
                                    U8 direction,
                                    U16 l_buf,
                                    U16 o_buf,
                                    U8 *buf,
                                    U8 **mStructPtr,
                                    U8 pt);
typedef BYTE  (*T_ccd_codeMsg) (UBYTE entity,
                                UBYTE direction,
                                T_MSGBUF *mBuf,
                                UBYTE *mStruct,
                                UBYTE pt);
typedef BYTE (*T_ccd_codeMsgPtr) (U8 entity,
                                  U8 direction,
                                  U16 *l_buf,
                                  U16 o_buf,
                                  U8 *buf,
                                  U8 *mStruct,
                                  U8 pt);
typedef ULONG  (*T_ccd_init_ccddata) (void);
/* cdc_std.c */
typedef BYTE  (*T_ccd_decodeByte) (UBYTE *bitstream,
                                   USHORT startbit,
                                   USHORT bitlen,
                                   UBYTE *value);
typedef BYTE  (*T_ccd_codeByte) (UBYTE *bitstream,
                                 USHORT startbit,
                                 USHORT bitlen,
                                 UBYTE val);
typedef BYTE  (*T_ccd_codeLong) (UBYTE *bitstream,
                                 USHORT startbit,
                                 USHORT bitlen,
                                 ULONG value);
typedef BYTE  (*T_ccd_decodeLong) (UBYTE *bitstream,
                                   USHORT startbit,
                                   USHORT bitlen,
                                   ULONG *value);
typedef void  (*T_ccd_bitcopy) (UBYTE *dest,
                                UBYTE *source,
                                USHORT bitlen,
                                USHORT offset);
/* ccd_err.c */
typedef UBYTE (*T_ccd_getFirstError) (UBYTE entity, USHORT *parlist);
typedef UBYTE (*T_ccd_getNextError) (UBYTE entity, USHORT *parlist);
typedef ULONG (*T_ccd_getFirstFault) (T_CCD_ERR_ENTRY **ccd_err_entry);
typedef ULONG (*T_ccd_getNextFault) (T_CCD_ERR_ENTRY **ccd_err_entry);
typedef void  (*T_ccd_free_faultlist) (void);
typedef int   (*T_ccd_get_numFaults) (void);
/* ccd_elem.c */
typedef int  (*T_ccd_encodeElem) (ULONG ccdid,
                                  USHORT *l_buf,
                                  USHORT o_buf,
                                  UCHAR *buf,
                                  UCHAR *eStruct);

typedef int  (*T_ccd_decodeElem) (ULONG ccdid,
                                  USHORT l_buf,
                                  USHORT o_buf,
                                  UCHAR *buf,
                                  UCHAR *eStruct);
/*==== LOCALS ================================================================*/
static int me = -1;
static HANDLE initlock;
static void (*ccddata_i_notify)(void)=NULL;
static void (*ccddata_x_notify)(void)=NULL;

static int ccddata_attach (int i);

#pragma data_seg(".shared")
static char lastdll[MAX_PATH] = { 0 };
static DWORD ptrindex[MAXPROC] = {0, 0, 0, 0, 0, 0, 0, 0,
                                  0, 0, 0, 0, 0, 0, 0, 0 };
static HINSTANCE cdll[MAXPROC] = {0, 0, 0, 0, 0, 0, 0, 0,
                                  0, 0, 0, 0, 0, 0, 0, 0 };
static HANDLE mut[MAXPROC] = {0, 0, 0, 0, 0, 0, 0, 0,
                              0, 0, 0, 0, 0, 0, 0, 0 };
static T_pcon_init_prim_coding ptr_pcon_init_prim_coding[MAXPROC]
  = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
static T_pcon_decodePrim ptr_pcon_decodePrim[MAXPROC]
  = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
static T_pcon_codePrim ptr_pcon_codePrim[MAXPROC]
  = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
static T_pcon_init_ccddata ptr_pcon_init_ccddata[MAXPROC]
  = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
static T_pcon_make_filter ptr_pcon_make_filter[MAXPROC]
  = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
static T_pcon_filter_decode_prim ptr_pcon_filter_decode_prim[MAXPROC]
  = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
static T_ccddata_get_pdi_dinfo ptr_ccddata_get_pdi_dinfo[MAXPROC]
  = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
static T_ccddata_get_num_of_entities ptr_ccddata_get_num_of_entities[MAXPROC]
  = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
static T_ccddata_get_max_message_id ptr_ccddata_get_max_message_id[MAXPROC]
  = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
static T_ccddata_get_max_bitstream_len
  ptr_ccddata_get_max_bitstream_len[MAXPROC]
  = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
static T_ccddata_get_max_mstruct_len ptr_ccddata_get_max_mstruct_len[MAXPROC]
  = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
static T_ccddata_mccd_symbols ptr_ccddata_mccd_symbols[MAXPROC]
  = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
static T_ccddata_get_max_sap_num ptr_ccddata_get_max_sap_num[MAXPROC]
  = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
static T_ccddata_get_max_primitive_id ptr_ccddata_get_max_primitive_id[MAXPROC]
  = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
static T_ccddata_get_max_pstruct_len ptr_ccddata_get_max_pstruct_len[MAXPROC]
  = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
static T_ccddata_get_mvar ptr_ccddata_get_mvar[MAXPROC]
  = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
static T_ccddata_get_spare ptr_ccddata_get_spare[MAXPROC]
  = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
static T_ccddata_get_calc ptr_ccddata_get_calc[MAXPROC]
  = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
static T_ccddata_get_mcomp ptr_ccddata_get_mcomp[MAXPROC]
  = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
static T_ccddata_get_melem ptr_ccddata_get_melem[MAXPROC]
  = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
static T_ccddata_get_calcidx ptr_ccddata_get_calcidx[MAXPROC]
  = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
static T_ccddata_get_mmtx ptr_ccddata_get_mmtx[MAXPROC]
  = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
static T_ccddata_get_pvar ptr_ccddata_get_pvar[MAXPROC]
  = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
static T_ccddata_get_pcomp ptr_ccddata_get_pcomp[MAXPROC]
  = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
static T_ccddata_get_pelem ptr_ccddata_get_pelem[MAXPROC]
  = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
static T_ccddata_get_pmtx ptr_ccddata_get_pmtx[MAXPROC]
  = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
static T_ccddata_get_mval ptr_ccddata_get_mval[MAXPROC]
  = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
static T_ccddata_get_mstr ptr_ccddata_get_mstr[MAXPROC]
  = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
static T_ccddata_get_pval ptr_ccddata_get_pval[MAXPROC]
  = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
static T_ccddata_get_pstr ptr_ccddata_get_pstr[MAXPROC]
  = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
static T_ccddata_get_ccdent ptr_ccddata_get_ccdent[MAXPROC]
  = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
static T_ccddata_get_entname ptr_ccddata_get_entname[MAXPROC]
  = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
static T_ccddata_get_alias ptr_ccddata_get_alias[MAXPROC]
  = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
static T_ccddata_get_version ptr_ccddata_get_version[MAXPROC]
  = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
static T_ccddata_get_table_version ptr_ccddata_get_table_version[MAXPROC]
  = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
static T_ccddata_get_mi_length ptr_ccddata_get_mi_length[MAXPROC]
  = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
static UBYTE* mi_length[MAXPROC]
  = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
static T_ccddata_get_decmsgbuffer ptr_ccddata_get_decmsgbuffer[MAXPROC]
  = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
static UBYTE* decmsgbuffer[MAXPROC]
  = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
static T_ccddata_tap_check_pd ptr_ccddata_tap_check_pd[MAXPROC]
  = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
static T_ccddata_tap_get_pd ptr_ccddata_tap_get_pd[MAXPROC]
  = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
static T_ccddata_eg_nodes ptr_ccddata_eg_nodes[MAXPROC]
  = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
static T_ccddata_eg_nodenames ptr_ccddata_eg_nodenames[MAXPROC]
  = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
static T_ccddata_eg_adjacent ptr_ccddata_eg_adjacent[MAXPROC]
  = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
static T_ccddata_eg_saps ptr_ccddata_eg_saps[MAXPROC]
  = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
static T_ccddata_eg_sapnames ptr_ccddata_eg_sapnames[MAXPROC]
  = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
static T_ccddata_eg_comendpoints ptr_ccddata_eg_comendpoints[MAXPROC]
  = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
static T_cde_init ptr_cde_init[MAXPROC]
  = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
static T_cde_get_comp ptr_cde_get_comp[MAXPROC]
  = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
static T_cde_prim_first ptr_cde_prim_first[MAXPROC]
  = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
static T_cde_prim_next ptr_cde_prim_next[MAXPROC]
  = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
static T_cde_msg_first ptr_cde_msg_first[MAXPROC]
  = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
static T_cde_msg_next ptr_cde_msg_next[MAXPROC]
  = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
static T_cde_comp_first ptr_cde_comp_first[MAXPROC]
  = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
static T_cde_comp_next ptr_cde_comp_next[MAXPROC]
  = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
static T_cde_get_symval ptr_cde_get_symval[MAXPROC]
  = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
static T_cde_read_elem ptr_cde_read_elem[MAXPROC]
  = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
static T_cde_write_prepare ptr_cde_write_prepare[MAXPROC]
  = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
static T_cde_write_elem ptr_cde_write_elem[MAXPROC]
  = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
static T_cde_get_type ptr_cde_get_type[MAXPROC]
  = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
static T_cde_get_primcode ptr_cde_get_primcode[MAXPROC]
  = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
static T_cde_get_msgcode ptr_cde_get_msgcode[MAXPROC]
  = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
static T_cde_get_is_downlink ptr_cde_get_is_downlink[MAXPROC]
  = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
static T_cde_get_comp_index ptr_cde_get_comp_index[MAXPROC]
  = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
static T_cde_get_element_name ptr_cde_get_element_name[MAXPROC]
  = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
static T_cde_get_array_kind ptr_cde_get_array_kind[MAXPROC]
  = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
static T_pdi_createDefContext ptr_pdi_createDefContext[MAXPROC]
  = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
static T_pdi_createContext ptr_pdi_createContext[MAXPROC]
  = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
static T_pdi_destroyContext ptr_pdi_destroyContext[MAXPROC]
  = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
static T_pdi_startPrim ptr_pdi_startPrim[MAXPROC]
  = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
static T_pdi_getDecodeInfo ptr_pdi_getDecodeInfo[MAXPROC]
  = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
static T_pdi_getEntityByPD ptr_pdi_getEntityByPD[MAXPROC]
  = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
static T_pdi_pd2name ptr_pdi_pd2name[MAXPROC]
  = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
static T_ccd_set_patch_infos ptr_ccd_set_patch_infos[MAXPROC]
  = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
static T_ccd_init ptr_ccd_init[MAXPROC]
  = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
static T_ccd_exit ptr_ccd_exit[MAXPROC]
  = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
static T_ccd_begin ptr_ccd_begin[MAXPROC]
  = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
static T_ccd_end ptr_ccd_end[MAXPROC]
  = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
static T_ccd_decodeMsg ptr_ccd_decodeMsg[MAXPROC]
  = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
static T_ccd_decodeMsgPtr ptr_ccd_decodeMsgPtr[MAXPROC]
  = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
static T_ccd_codeMsg ptr_ccd_codeMsg[MAXPROC]
  = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
static T_ccd_codeMsgPtr ptr_ccd_codeMsgPtr[MAXPROC]
  = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
static T_ccd_init_ccddata ptr_ccd_init_ccddata[MAXPROC]
  = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
static T_ccd_decodeByte ptr_ccd_decodeByte[MAXPROC]
  = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
static T_ccd_codeByte ptr_ccd_codeByte[MAXPROC]
  = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
static T_ccd_codeLong ptr_ccd_codeLong[MAXPROC]
  = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
static T_ccd_decodeLong ptr_ccd_decodeLong[MAXPROC]
  = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
static T_ccd_bitcopy ptr_ccd_bitcopy[MAXPROC]
  = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
static T_ccd_getFirstError ptr_ccd_getFirstError[MAXPROC]
  = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
static T_ccd_getNextError ptr_ccd_getNextError[MAXPROC]
  = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
static T_ccd_getFirstFault ptr_ccd_getFirstFault[MAXPROC]
  = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
static T_ccd_getNextFault ptr_ccd_getNextFault[MAXPROC]
  = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
static T_ccd_free_faultlist ptr_ccd_free_faultlist[MAXPROC]
  = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
static T_ccd_get_numFaults ptr_ccd_get_numFaults[MAXPROC]
  = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
static T_ccd_encodeElem ptr_ccd_encodeElem[MAXPROC]
  = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
static T_ccd_decodeElem ptr_ccd_decodeElem[MAXPROC]
  = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
#pragma data_seg()


/*==== HANDLER FOR FUNCTIONS UNSUPPORTED BY LOADED DLL =======================*/
int cddl_ccddata_get_pdi_dinfo (const T_PDI_DECODEINFO* (*dinfo) )
{
  dinfo=NULL;
  return 0;
}

int cddl_ccddata_eg_nodes (void)
{
  return 0;
}

char** cddl_ccddata_eg_nodenames (void)
{
  return NULL;
}

char* cddl_ccddata_eg_adjacent (int idx)
{
  return NULL;
}

int cddl_ccddata_eg_saps (void)
{
  return 0;
}

char** cddl_ccddata_eg_sapnames (void)
{
  return NULL;
}

T_COMENDPOINTS* cddl_ccddata_eg_comendpoints (int idx)
{
  return NULL;
}

char* cddl_ccddata_get_version (void)
{
  return "OLD";
}

int cddl_ccddata_get_table_version (void)
{
  return 0;
}

const char * cddl_ccddata_get_entname (short ccdent)
{
  return NULL;
}
/*==== PRIVATE FUNCTIONS =====================================================*/
/*
+------------------------------------------------------------------------------
|  Function     :  ccddata_init_lock
+------------------------------------------------------------------------------
|  Description  :  Synchronize calls of ccddata_init
|
|  Parameters   :  - 
|
|  Return       :  0 on success, otherwise error code
+------------------------------------------------------------------------------
*/
static HANDLE ccddata_init_lock (void)
{
  char* mname = "ccddata_init_lock";
  HANDLE tmp;
  if ((tmp = CreateMutex (NULL, FALSE, mname)) == 0)
  {
    return 0;
  }
  if ((initlock = OpenMutex (MUTEX_ALL_ACCESS, FALSE, mname)) == 0)
  {
    return 0;
  }
  CloseHandle (tmp);
  WaitForSingleObject (initlock, INFINITE);
  return initlock;
}

/*
+------------------------------------------------------------------------------
|  Function     :  ccddata_register
+------------------------------------------------------------------------------
|  Description  :  Register process in ccddata load dll
|
|  Parameters   :  -
|
|  Return       :  Index to ptrindex on success, otherwise -1
+------------------------------------------------------------------------------
*/
static int ccddata_register (void)
{
  if (me >= 0)
  {
    return me;
  }
  for (me=0; me<MAXPROC; me++)
  {
    if (!ptrindex[me])
    {
      char mname[32];
      HANDLE tmp;
      ptrindex[me] = GetCurrentProcessId ();
      sprintf (mname, "ccddata_%d", ptrindex[me]);
      tmp = CreateMutex (NULL, FALSE, mname);
      mut[me] = OpenMutex (MUTEX_ALL_ACCESS, FALSE, mname);
      CloseHandle (tmp);
      return me;
    }
  }
  me = -1;
  return me;
}

/*
+------------------------------------------------------------------------------
|  Function     :  ccddata_detach
+------------------------------------------------------------------------------
|  Description  :  Unload ccddata dll and clean pointer to the functions
|
|  Parameters   :  i - index to pointer list
|                  exit_all - if set, call ccd_exit and call FreeLibrary
|
|  Return       :  -
+------------------------------------------------------------------------------
*/
static void ccddata_detach (int i, int exit_all)
{
  char mname[32];
  HANDLE mutex;

  if (!cdll[i])
    return;

  if (ccddata_x_notify != NULL && exit_all)
  {
    // make sure all functions are available again
    if (ptr_ccd_init[i] != NULL || ccddata_attach(i)==CCDDATA_DLL_OK)
    {
      (*ccddata_x_notify)();
    }
  }

  if (ptr_ccd_init[i] != NULL)
  {
    // clean all function pointers except ccd_exit
    if (!exit_all)
    {
      sprintf (mname, "ccddata_%d", ptrindex[i]);
      mutex = OpenMutex (MUTEX_ALL_ACCESS, FALSE, mname);
      WaitForSingleObject (mutex, INFINITE);
    }

    ptr_pcon_init_prim_coding[i] = NULL;
    ptr_pcon_decodePrim[i] = NULL;
    ptr_pcon_codePrim[i] = NULL;
    ptr_pcon_init_ccddata[i] = NULL;
    ptr_pcon_make_filter[i] = NULL;
    ptr_pcon_filter_decode_prim[i] = NULL;
    ptr_ccddata_get_pdi_dinfo[i] = NULL;
    ptr_ccddata_get_num_of_entities[i] = NULL;
    ptr_ccddata_get_max_message_id[i] = NULL;
    ptr_ccddata_get_max_bitstream_len[i] = NULL;
    ptr_ccddata_get_max_mstruct_len[i] = NULL;
    ptr_ccddata_mccd_symbols[i] = NULL;
    ptr_ccddata_get_max_sap_num[i] = NULL;
    ptr_ccddata_get_max_primitive_id[i] = NULL;
    ptr_ccddata_get_max_pstruct_len[i] = NULL;
    ptr_ccddata_get_mvar[i] = NULL;
    ptr_ccddata_get_spare[i] = NULL;
    ptr_ccddata_get_calc[i] = NULL;
    ptr_ccddata_get_mcomp[i] = NULL;
    ptr_ccddata_get_melem[i] = NULL;
    ptr_ccddata_get_calcidx[i] = NULL;
    ptr_ccddata_get_mmtx[i] = NULL;
    ptr_ccddata_get_pvar[i] = NULL;
    ptr_ccddata_get_pcomp[i] = NULL;
    ptr_ccddata_get_pelem[i] = NULL;
    ptr_ccddata_get_pmtx[i] = NULL;
    ptr_ccddata_get_mval[i] = NULL;
    ptr_ccddata_get_mstr[i] = NULL;
    ptr_ccddata_get_pval[i] = NULL;
    ptr_ccddata_get_pstr[i] = NULL;
    ptr_ccddata_get_ccdent[i] = NULL;
    ptr_ccddata_get_entname[i] = NULL;
    ptr_ccddata_get_alias[i] = NULL;
    ptr_ccddata_get_version[i] = NULL;
    ptr_ccddata_get_table_version[i] = NULL;
    ptr_ccddata_get_mi_length[i] = NULL;
    mi_length[i] = NULL;
    ptr_ccddata_get_decmsgbuffer[i] = NULL;
    decmsgbuffer[i] = NULL;
    ptr_ccddata_tap_check_pd[i] = NULL;
    ptr_ccddata_tap_get_pd[i] = NULL;
    ptr_ccddata_eg_nodes[i] = NULL;
    ptr_ccddata_eg_nodenames[i] = NULL;
    ptr_ccddata_eg_adjacent[i] = NULL;
    ptr_ccddata_eg_saps[i] = NULL;
    ptr_ccddata_eg_sapnames[i] = NULL;
    ptr_ccddata_eg_comendpoints[i] = NULL;

    ptr_cde_init[i] = NULL;
    ptr_cde_get_comp[i] = NULL;
    ptr_cde_prim_first[i] = NULL;
    ptr_cde_prim_next[i] = NULL;
    ptr_cde_msg_first[i] = NULL;
    ptr_cde_msg_next[i] = NULL;
    ptr_cde_comp_first[i] = NULL;
    ptr_cde_comp_next[i] = NULL;
    ptr_cde_get_symval[i] = NULL;
    ptr_cde_read_elem[i] = NULL;
    ptr_cde_write_prepare[i] = NULL;
    ptr_cde_write_elem[i] = NULL;
    ptr_cde_get_type[i] = NULL;
    ptr_cde_get_primcode[i] = NULL;
    ptr_cde_get_msgcode[i] = NULL;
    ptr_cde_get_is_downlink[i] = NULL;
    ptr_cde_get_comp_index[i] = NULL;
    ptr_cde_get_element_name[i] = NULL;
    ptr_cde_get_array_kind[i] = NULL;

    ptr_pdi_createDefContext[i] = NULL;
    ptr_pdi_createContext[i] = NULL;
    ptr_pdi_destroyContext[i] = NULL;
    ptr_pdi_startPrim[i] = NULL;
    ptr_pdi_getDecodeInfo[i] = NULL;
    ptr_pdi_getEntityByPD[i] = NULL;
    ptr_pdi_pd2name[i] = NULL;

    ptr_ccd_set_patch_infos[i] = NULL;
    ptr_ccd_begin[i] = NULL;
    ptr_ccd_end[i] = NULL;
    ptr_ccd_decodeMsg[i] = NULL;
    ptr_ccd_decodeMsgPtr[i] = NULL;
    ptr_ccd_codeMsg[i] = NULL;
    ptr_ccd_codeMsgPtr[i] = NULL;
    ptr_ccd_init_ccddata[i] = NULL;
    ptr_ccd_decodeByte[i] = NULL;
    ptr_ccd_codeByte[i] = NULL;
    ptr_ccd_codeLong[i] = NULL;
    ptr_ccd_decodeLong[i] = NULL;
    ptr_ccd_bitcopy[i] = NULL;
    ptr_ccd_getFirstError[i] = NULL;
    ptr_ccd_getNextError[i] = NULL;
    ptr_ccd_getFirstFault[i] = NULL;
    ptr_ccd_getNextFault[i] = NULL;
    ptr_ccd_free_faultlist[i] = NULL;
    ptr_ccd_get_numFaults[i] = NULL;
    ptr_ccd_encodeElem[i] = NULL;
    ptr_ccd_decodeElem[i] = NULL;
    ptr_ccd_init[i] = NULL;

    if (!exit_all)
    {
      ReleaseMutex (mutex);
      CloseHandle (mutex);
    }
  }

  if (exit_all)
  {
    if (ptr_ccd_exit[i])
    {
      (void)(*ptr_ccd_exit[i])();
      ptr_ccd_exit[i] = NULL;
    }
    (void) FreeLibrary (cdll[i]);
    cdll[i] = 0;
  }
}

/*
+------------------------------------------------------------------------------
|  Function     :  ccddata_clean
+------------------------------------------------------------------------------
|  Description  :  Free own instance of ccddata dll and clean the pointers
|                  of the other processes.
|
|  Parameters   :  -
|
|  Return       :  CCDDATA_DLL_REGISTER, if the process is not registered
|                  or has not yet called ccddata_exit; CCDDATA_DLL_OK otherwise
+------------------------------------------------------------------------------
*/
static int ccddata_clean (void)
{
  int i;

  if (me < 0)
  {
    return CCDDATA_DLL_REGISTER;
  }

  for (i=0; i<MAXPROC; i++)
  {
    if (me != i)
    {
      ccddata_detach (i, 0);
    }
  }
  
  return CCDDATA_DLL_OK;
}

/*
+------------------------------------------------------------------------------
|  Function     :  ccddata_attach
+------------------------------------------------------------------------------
|  Description  :  Unload ccddata dll and clean pointer to the functions
|
|  Parameters   :  i - index to pointer list
|
|  Return       :  CCDDATA_DLL_OK or error code
+------------------------------------------------------------------------------
*/
static int ccddata_attach (int i)
{
  int already_loaded=(cdll[i]!=0);
  if (!already_loaded)
  {
    cdll[i] = LoadLibrary (lastdll);
    if (cdll[i] == NULL)
    {
      return CCDDATA_DLL_LOADLIB;
    }
  }

  ptr_pcon_init_prim_coding[i] = (T_pcon_init_prim_coding)
    GetProcAddress (cdll[i], "cddl_pcon_init_prim_coding");
  ptr_pcon_decodePrim[i] = (T_pcon_decodePrim)
    GetProcAddress (cdll[i], "cddl_pcon_decodePrim");
  ptr_pcon_codePrim[i] = (T_pcon_codePrim)
    GetProcAddress (cdll[i], "cddl_pcon_codePrim");
  ptr_pcon_init_ccddata[i] = (T_pcon_init_ccddata)
    GetProcAddress (cdll[i], "cddl_pcon_init_ccddata");
  ptr_pcon_make_filter[i] = (T_pcon_make_filter)
    GetProcAddress (cdll[i], "cddl_pcon_make_filter");
  ptr_pcon_filter_decode_prim[i] = (T_pcon_filter_decode_prim)
    GetProcAddress (cdll[i], "cddl_pcon_filter_decode_prim");

  if (
    ptr_pcon_init_prim_coding[i] == NULL ||
    ptr_pcon_decodePrim[i] == NULL ||
    ptr_pcon_codePrim[i] == NULL ||
    ptr_pcon_init_ccddata[i] == NULL ||
    ptr_pcon_make_filter[i] == NULL ||
    ptr_pcon_filter_decode_prim[i] == NULL)
  {
    ptr_pcon_init_prim_coding[i] = cddl_pcon_init_prim_coding;
    ptr_pcon_decodePrim[i] = cddl_pcon_decodePrim;
    ptr_pcon_codePrim[i] = cddl_pcon_codePrim;
    ptr_pcon_init_ccddata[i] = cddl_pcon_init_ccddata;
    ptr_pcon_make_filter[i] = cddl_pcon_make_filter;
    ptr_pcon_filter_decode_prim[i] = cddl_pcon_filter_decode_prim;
  }

  ptr_ccd_init[i] = (T_ccd_init)
    GetProcAddress (cdll[i], "cddl_ccd_init");
  ptr_ccd_exit[i] = (T_ccd_exit)
    GetProcAddress (cdll[i], "cddl_ccd_exit");
  ptr_ccd_begin[i] = (T_ccd_begin)
    GetProcAddress (cdll[i], "cddl_ccd_begin");
  ptr_ccd_end[i] = (T_ccd_end)
    GetProcAddress (cdll[i], "cddl_ccd_end");
  ptr_ccd_decodeMsg[i] = (T_ccd_decodeMsg)
    GetProcAddress (cdll[i], "cddl_ccd_decodeMsg");
  ptr_ccd_decodeMsgPtr[i] = (T_ccd_decodeMsgPtr)
    GetProcAddress (cdll[i], "cddl_ccd_decodeMsgPtr");
  ptr_ccd_codeMsg[i] = (T_ccd_codeMsg)
    GetProcAddress (cdll[i], "cddl_ccd_codeMsg");
  ptr_ccd_codeMsgPtr[i] = (T_ccd_codeMsgPtr)
    GetProcAddress (cdll[i], "cddl_ccd_codeMsgPtr");
  ptr_ccd_init_ccddata[i] = (T_ccd_init_ccddata)
    GetProcAddress (cdll[i], "cddl_ccd_init_ccddata");
  ptr_ccd_decodeByte[i] = (T_ccd_decodeByte)
    GetProcAddress (cdll[i], "cddl_ccd_decodeByte");
  ptr_ccd_codeByte[i] = (T_ccd_codeByte)
    GetProcAddress (cdll[i], "cddl_ccd_codeByte");
  ptr_ccd_codeLong[i] = (T_ccd_codeLong)
    GetProcAddress (cdll[i], "cddl_ccd_codeLong");
  ptr_ccd_decodeLong[i] = (T_ccd_decodeLong)
    GetProcAddress (cdll[i], "cddl_ccd_decodeLong");
  ptr_ccd_bitcopy[i] = (T_ccd_bitcopy)
    GetProcAddress (cdll[i], "cddl_ccd_bitcopy");
  ptr_ccd_getFirstError[i] = (T_ccd_getFirstError)
    GetProcAddress (cdll[i], "cddl_ccd_getFirstError");
  ptr_ccd_getNextError[i] = (T_ccd_getNextError)
    GetProcAddress (cdll[i], "cddl_ccd_getNextError");
  ptr_ccd_getFirstFault[i] = (T_ccd_getFirstFault)
    GetProcAddress (cdll[i], "cddl_ccd_getFirstFault");
  ptr_ccd_getNextFault[i] = (T_ccd_getNextFault)
    GetProcAddress (cdll[i], "cddl_ccd_getNextFault");
  ptr_ccd_free_faultlist[i] = (T_ccd_free_faultlist)
    GetProcAddress (cdll[i], "cddl_ccd_free_faultlist");
  ptr_ccd_get_numFaults[i] = (T_ccd_get_numFaults)
    GetProcAddress (cdll[i], "cddl_ccd_get_numFaults");
  ptr_ccd_encodeElem[i] = (T_ccd_encodeElem)
    GetProcAddress (cdll[i], "cddl_ccd_encodeElem");
  ptr_ccd_decodeElem[i] = (T_ccd_decodeElem)
    GetProcAddress (cdll[i], "cddl_ccd_decodeElem");
  if (
    ptr_ccd_init[i] == NULL ||
    ptr_ccd_exit[i] == NULL ||
    ptr_ccd_begin[i] == NULL ||
    ptr_ccd_end[i] == NULL ||
    ptr_ccd_decodeMsg[i] == NULL ||
    ptr_ccd_decodeMsgPtr[i] == NULL ||
    ptr_ccd_codeMsg[i] == NULL ||
    ptr_ccd_codeMsgPtr[i] == NULL ||
    ptr_ccd_init_ccddata[i] == NULL ||
    ptr_ccd_decodeByte[i] == NULL ||
    ptr_ccd_codeByte[i] == NULL ||
    ptr_ccd_codeLong[i] == NULL ||
    ptr_ccd_decodeLong[i] == NULL ||
    ptr_ccd_bitcopy[i] == NULL ||
    ptr_ccd_getFirstError[i] == NULL ||
    ptr_ccd_getNextError[i] == NULL ||
    ptr_ccd_getFirstFault[i] == NULL ||
    ptr_ccd_getNextFault[i] == NULL ||
    ptr_ccd_free_faultlist[i] == NULL ||
    ptr_ccd_get_numFaults[i] == NULL ||
    ptr_ccd_encodeElem[i] == NULL ||
    ptr_ccd_decodeElem[i] == NULL)
  {
    ptr_ccd_init[i] = cddl_ccd_init;
    ptr_ccd_exit[i] = cddl_ccd_exit;
    ptr_ccd_begin[i] = cddl_ccd_begin;
    ptr_ccd_end[i] = cddl_ccd_end;
    ptr_ccd_decodeMsg[i] = cddl_ccd_decodeMsg;
    ptr_ccd_decodeMsgPtr[i] = cddl_ccd_decodeMsgPtr;
    ptr_ccd_codeMsg[i] = cddl_ccd_codeMsg;
    ptr_ccd_codeMsgPtr[i] = cddl_ccd_codeMsgPtr;
    ptr_ccd_init_ccddata[i] = cddl_ccd_init_ccddata;
    ptr_ccd_decodeByte[i] = cddl_ccd_decodeByte;
    ptr_ccd_codeByte[i] = cddl_ccd_codeByte;
    ptr_ccd_codeLong[i] = cddl_ccd_codeLong;
    ptr_ccd_decodeLong[i] = cddl_ccd_decodeLong;
    ptr_ccd_bitcopy[i] = cddl_ccd_bitcopy;
    ptr_ccd_getFirstError[i] = cddl_ccd_getFirstError;
    ptr_ccd_getNextError[i] = cddl_ccd_getNextError;
    ptr_ccd_getFirstFault[i] = cddl_ccd_getFirstFault;
    ptr_ccd_getNextFault[i] = cddl_ccd_getNextFault;
    ptr_ccd_free_faultlist[i] = cddl_ccd_free_faultlist;
    ptr_ccd_get_numFaults[i] = cddl_ccd_get_numFaults;
    ptr_ccd_encodeElem[i] = cddl_ccd_encodeElem;
    ptr_ccd_decodeElem[i] = cddl_ccd_decodeElem;
  }
  ptr_ccd_set_patch_infos[i] = (T_ccd_set_patch_infos)
    GetProcAddress (cdll[i], "cddl_ccd_set_patch_infos");
  if (ptr_ccd_set_patch_infos[i] == NULL)
  {
    ptr_ccd_set_patch_infos[i] = cddl_ccd_set_patch_infos;
  }

  ptr_ccddata_eg_nodes[i] = (T_ccddata_eg_nodes)
    GetProcAddress (cdll[i], "ccddata_eg_nodes");
  ptr_ccddata_eg_nodenames[i] = (T_ccddata_eg_nodenames)
    GetProcAddress (cdll[i], "ccddata_eg_nodenames");
  ptr_ccddata_eg_adjacent[i] = (T_ccddata_eg_adjacent)
    GetProcAddress (cdll[i], "ccddata_eg_adjacent");
  if (
    ptr_ccddata_eg_nodes[i] == NULL ||
    ptr_ccddata_eg_nodenames[i] == NULL ||
    ptr_ccddata_eg_adjacent[i] == NULL)
  {
    ptr_ccddata_eg_nodes[i] = cddl_ccddata_eg_nodes;
    ptr_ccddata_eg_nodenames[i] = cddl_ccddata_eg_nodenames;
    ptr_ccddata_eg_adjacent[i] = cddl_ccddata_eg_adjacent;
  }

  ptr_ccddata_eg_saps[i] = (T_ccddata_eg_saps)
    GetProcAddress (cdll[i], "ccddata_eg_saps");
  ptr_ccddata_eg_sapnames[i] = (T_ccddata_eg_sapnames)
    GetProcAddress (cdll[i], "ccddata_eg_sapnames");
  ptr_ccddata_eg_comendpoints[i] = (T_ccddata_eg_comendpoints)
    GetProcAddress (cdll[i], "ccddata_eg_comendpoints");
  if (
    ptr_ccddata_eg_saps[i] == NULL ||
    ptr_ccddata_eg_sapnames[i] == NULL ||
    ptr_ccddata_eg_comendpoints[i] == NULL)
  {
    ptr_ccddata_eg_saps[i] = cddl_ccddata_eg_saps;
    ptr_ccddata_eg_sapnames[i] = cddl_ccddata_eg_sapnames;
    ptr_ccddata_eg_comendpoints[i] = cddl_ccddata_eg_comendpoints;
  }
  
  ptr_cde_init[i] = (T_cde_init)
    GetProcAddress (cdll[i], "cddl_cde_init");
  ptr_cde_get_comp[i] = (T_cde_get_comp)
    GetProcAddress (cdll[i], "cddl_cde_get_comp");
  ptr_cde_prim_first[i] = (T_cde_prim_first)
    GetProcAddress (cdll[i], "cddl_cde_prim_first");
  ptr_cde_prim_next[i] = (T_cde_prim_next)
    GetProcAddress (cdll[i], "cddl_cde_prim_next");
  ptr_cde_msg_first[i] = (T_cde_msg_first)
    GetProcAddress (cdll[i], "cddl_cde_msg_first");
  ptr_cde_msg_next[i] = (T_cde_msg_next)
    GetProcAddress (cdll[i], "cddl_cde_msg_next");
  ptr_cde_comp_first[i] = (T_cde_comp_first)
    GetProcAddress (cdll[i], "cddl_cde_comp_first");
  ptr_cde_comp_next[i] = (T_cde_comp_next)
    GetProcAddress (cdll[i], "cddl_cde_comp_next");
  ptr_cde_get_symval[i] = (T_cde_get_symval)
    GetProcAddress (cdll[i], "cddl_cde_get_symval");
  ptr_cde_read_elem[i] = (T_cde_read_elem)
    GetProcAddress (cdll[i], "cddl_cde_read_elem");
  ptr_cde_write_prepare[i] = (T_cde_write_prepare)
    GetProcAddress (cdll[i], "cddl_cde_write_prepare");
  ptr_cde_write_elem[i] = (T_cde_write_elem)
    GetProcAddress (cdll[i], "cddl_cde_write_elem");
  ptr_cde_get_type[i] = (T_cde_get_type)
    GetProcAddress (cdll[i], "cddl_cde_get_type");
  ptr_cde_get_primcode[i] = (T_cde_get_primcode)
    GetProcAddress (cdll[i], "cddl_cde_get_primcode");
  ptr_cde_get_msgcode[i] = (T_cde_get_msgcode)
    GetProcAddress (cdll[i], "cddl_cde_get_msgcode");
  ptr_cde_get_is_downlink[i] = (T_cde_get_is_downlink)
    GetProcAddress (cdll[i], "cddl_cde_get_is_downlink");
  ptr_cde_get_comp_index[i] = (T_cde_get_comp_index)
    GetProcAddress (cdll[i], "cddl_cde_get_comp_index");
  ptr_cde_get_element_name[i] = (T_cde_get_element_name)
    GetProcAddress (cdll[i], "cddl_cde_get_element_name");
  ptr_cde_get_array_kind[i] = (T_cde_get_array_kind)
    GetProcAddress (cdll[i], "cddl_cde_get_array_kind");
  if (
    ptr_cde_init[i] == NULL ||
    ptr_cde_get_comp[i] == NULL ||
    ptr_cde_prim_first[i] == NULL ||
    ptr_cde_prim_next[i] == NULL ||
    ptr_cde_msg_first[i] == NULL ||
    ptr_cde_msg_next[i] == NULL ||
    ptr_cde_comp_first[i] == NULL ||
    ptr_cde_comp_next[i] == NULL ||
    ptr_cde_get_symval[i] == NULL ||
    ptr_cde_read_elem[i] == NULL ||
    ptr_cde_write_prepare[i] == NULL ||
    ptr_cde_write_elem[i] == NULL ||
    ptr_cde_get_type[i] == NULL ||
    ptr_cde_get_primcode[i] == NULL ||
    ptr_cde_get_msgcode[i] == NULL ||
    ptr_cde_get_is_downlink[i] == NULL ||
    ptr_cde_get_comp_index[i] == NULL ||
    ptr_cde_get_element_name[i] == NULL ||
    ptr_cde_get_array_kind[i] == NULL)
  {
    ptr_cde_init[i] = cddl_cde_init;
    ptr_cde_get_comp[i] = cddl_cde_get_comp;
    ptr_cde_prim_first[i] = cddl_cde_prim_first;
    ptr_cde_prim_next[i] = cddl_cde_prim_next;
    ptr_cde_msg_first[i] = cddl_cde_msg_first;
    ptr_cde_msg_next[i] = cddl_cde_msg_next;
    ptr_cde_comp_first[i] = cddl_cde_comp_first;
    ptr_cde_comp_next[i] = cddl_cde_comp_next;
    ptr_cde_get_symval[i] = cddl_cde_get_symval;
    ptr_cde_read_elem[i] = cddl_cde_read_elem;
    ptr_cde_write_prepare[i] = cddl_cde_write_prepare;
    ptr_cde_write_elem[i] = cddl_cde_write_elem;
    ptr_cde_get_type[i] = cddl_cde_get_type;
    ptr_cde_get_primcode[i] = cddl_cde_get_primcode;
    ptr_cde_get_msgcode[i] = cddl_cde_get_msgcode;
    ptr_cde_get_is_downlink[i] = cddl_cde_get_is_downlink;
    ptr_cde_get_comp_index[i] = cddl_cde_get_comp_index;
    ptr_cde_get_element_name[i] = cddl_cde_get_element_name;
    ptr_cde_get_array_kind[i] = cddl_cde_get_array_kind;
  }

  ptr_pdi_createDefContext[i] = (T_pdi_createDefContext)
    GetProcAddress (cdll[i], "cddl_pdi_createDefContext");
  ptr_pdi_createContext[i] = (T_pdi_createContext)
    GetProcAddress (cdll[i], "cddl_pdi_createContext");
  ptr_pdi_destroyContext[i] = (T_pdi_destroyContext)
    GetProcAddress (cdll[i], "cddl_pdi_destroyContext");
  ptr_pdi_startPrim[i] = (T_pdi_startPrim)
    GetProcAddress (cdll[i], "cddl_pdi_startPrim");
  ptr_pdi_getDecodeInfo[i] = (T_pdi_getDecodeInfo)
    GetProcAddress (cdll[i], "cddl_pdi_getDecodeInfo");
  ptr_pdi_getEntityByPD[i] = (T_pdi_getEntityByPD)
    GetProcAddress (cdll[i], "cddl_pdi_getEntityByPD");
  ptr_pdi_pd2name[i] = (T_pdi_pd2name)
    GetProcAddress (cdll[i], "cddl_pdi_pd2name");
  if (
    ptr_pdi_createDefContext[i] == NULL ||
    ptr_pdi_createContext[i] == NULL ||
    ptr_pdi_destroyContext[i] == NULL ||
    ptr_pdi_startPrim[i] == NULL ||
    ptr_pdi_getDecodeInfo[i] == NULL ||
    ptr_pdi_getEntityByPD[i] == NULL ||
    ptr_pdi_pd2name[i] == NULL)
  {
    ptr_pdi_createDefContext[i]=cddl_pdi_createDefContext;
    ptr_pdi_createContext[i] =cddl_pdi_createContext;
    ptr_pdi_destroyContext[i] = cddl_pdi_destroyContext;
    ptr_pdi_startPrim[i] = cddl_pdi_startPrim;
    ptr_pdi_getDecodeInfo[i] = cddl_pdi_getDecodeInfo;
    ptr_pdi_getEntityByPD[i] = cddl_pdi_getEntityByPD;
    ptr_pdi_pd2name[i] = cddl_pdi_pd2name;
  }

  ptr_ccddata_get_pdi_dinfo[i] = (T_ccddata_get_pdi_dinfo)
    GetProcAddress (cdll[i], "ccddata_get_pdi_dinfo");
  if (ptr_ccddata_get_pdi_dinfo[i] == NULL)
  {
    ptr_ccddata_get_pdi_dinfo[i] = cddl_ccddata_get_pdi_dinfo;
  }
  ptr_ccddata_get_num_of_entities[i] = (T_ccddata_get_num_of_entities)
    GetProcAddress (cdll[i], "ccddata_get_num_of_entities");
  ptr_ccddata_get_max_message_id[i] = (T_ccddata_get_max_message_id)
    GetProcAddress (cdll[i], "ccddata_get_max_message_id");
  ptr_ccddata_get_max_bitstream_len[i] = (T_ccddata_get_max_bitstream_len)
    GetProcAddress (cdll[i], "ccddata_get_max_bitstream_len");
  ptr_ccddata_get_max_mstruct_len[i] = (T_ccddata_get_max_mstruct_len)
    GetProcAddress (cdll[i], "ccddata_get_max_mstruct_len");
  ptr_ccddata_mccd_symbols[i] = (T_ccddata_mccd_symbols)
    GetProcAddress (cdll[i], "ccddata_mccd_symbols");
  ptr_ccddata_get_max_sap_num[i] = (T_ccddata_get_max_sap_num)
    GetProcAddress (cdll[i], "ccddata_get_max_sap_num");
  ptr_ccddata_get_max_primitive_id[i] = (T_ccddata_get_max_primitive_id)
    GetProcAddress (cdll[i], "ccddata_get_max_primitive_id");
  ptr_ccddata_get_max_pstruct_len[i] = (T_ccddata_get_max_pstruct_len)
    GetProcAddress (cdll[i], "ccddata_get_max_pstruct_len");
  ptr_ccddata_get_mvar[i] = (T_ccddata_get_mvar)
    GetProcAddress (cdll[i], "ccddata_get_mvar");
  ptr_ccddata_get_spare[i] = (T_ccddata_get_spare)
    GetProcAddress (cdll[i], "ccddata_get_spare");
  ptr_ccddata_get_calc[i] = (T_ccddata_get_calc)
    GetProcAddress (cdll[i], "ccddata_get_calc");
  ptr_ccddata_get_mcomp[i] = (T_ccddata_get_mcomp)
    GetProcAddress (cdll[i], "ccddata_get_mcomp");
  ptr_ccddata_get_melem[i] = (T_ccddata_get_melem)
    GetProcAddress (cdll[i], "ccddata_get_melem");
  ptr_ccddata_get_calcidx[i] = (T_ccddata_get_calcidx)
    GetProcAddress (cdll[i], "ccddata_get_calcidx");
  ptr_ccddata_get_mmtx[i] = (T_ccddata_get_mmtx)
    GetProcAddress (cdll[i], "ccddata_get_mmtx");
  ptr_ccddata_get_pvar[i] = (T_ccddata_get_pvar)
    GetProcAddress (cdll[i], "ccddata_get_pvar");
  ptr_ccddata_get_pcomp[i] = (T_ccddata_get_pcomp)
    GetProcAddress (cdll[i], "ccddata_get_pcomp");
  ptr_ccddata_get_pelem[i] = (T_ccddata_get_pelem)
    GetProcAddress (cdll[i], "ccddata_get_pelem");
  ptr_ccddata_get_pmtx[i] = (T_ccddata_get_pmtx)
    GetProcAddress (cdll[i], "ccddata_get_pmtx");
  ptr_ccddata_get_mval[i] = (T_ccddata_get_mval)
    GetProcAddress (cdll[i], "ccddata_get_mval");
  ptr_ccddata_get_mstr[i] = (T_ccddata_get_mstr)
    GetProcAddress (cdll[i], "ccddata_get_mstr");
  ptr_ccddata_get_pval[i] = (T_ccddata_get_pval)
    GetProcAddress (cdll[i], "ccddata_get_pval");
  ptr_ccddata_get_pstr[i] = (T_ccddata_get_pstr)
    GetProcAddress (cdll[i], "ccddata_get_pstr");
  ptr_ccddata_get_ccdent[i] = (T_ccddata_get_ccdent)
    GetProcAddress (cdll[i], "ccddata_get_ccdent");
  ptr_ccddata_get_entname[i] = (T_ccddata_get_entname)
    GetProcAddress (cdll[i], "ccddata_get_entname");
  if (ptr_ccddata_get_entname[i] == NULL)
  {
    ptr_ccddata_get_entname[i] = cddl_ccddata_get_entname;
  }
  ptr_ccddata_get_alias[i] = (T_ccddata_get_alias)
    GetProcAddress (cdll[i], "ccddata_get_alias");
  ptr_ccddata_get_version[i] = (T_ccddata_get_version)
    GetProcAddress (cdll[i], "ccddata_get_version");
  if (ptr_ccddata_get_version[i] == NULL)
  {
    ptr_ccddata_get_version[i] = cddl_ccddata_get_version;
  }
  ptr_ccddata_get_table_version[i] = (T_ccddata_get_table_version)
    GetProcAddress (cdll[i], "ccddata_get_table_version");
  if (ptr_ccddata_get_table_version[i] == NULL)
  {
    ptr_ccddata_get_table_version[i] = cddl_ccddata_get_table_version;
  }
  ptr_ccddata_get_mi_length[i] = (T_ccddata_get_mi_length)
    GetProcAddress (cdll[i], "ccddata_get_mi_length");
  if (ptr_ccddata_get_mi_length[i] == NULL)
  {
    mi_length[i] = (UBYTE*) GetProcAddress (cdll[i], "mi_length");
  }
  ptr_ccddata_get_decmsgbuffer[i] = (T_ccddata_get_decmsgbuffer)
    GetProcAddress (cdll[i], "ccddata_get_decmsgbuffer");
  if (ptr_ccddata_get_decmsgbuffer[i] == NULL)
  {
    decmsgbuffer[i] = (UBYTE*) GetProcAddress (cdll[i], "decMsgBuffer");
  }
  ptr_ccddata_tap_check_pd[i] = (T_ccddata_tap_check_pd)
    GetProcAddress (cdll[i], "ccddata_tap_check_pd");
  ptr_ccddata_tap_get_pd[i] = (T_ccddata_tap_get_pd)
    GetProcAddress (cdll[i], "ccddata_tap_get_pd");
  if (
    ptr_ccddata_get_pdi_dinfo[i] == NULL ||
    ptr_ccddata_get_num_of_entities[i] == NULL ||
    ptr_ccddata_get_max_message_id[i] == NULL ||
    ptr_ccddata_get_max_bitstream_len[i] == NULL ||
    ptr_ccddata_get_max_mstruct_len[i] == NULL ||
    ptr_ccddata_mccd_symbols[i] == NULL ||
    ptr_ccddata_get_max_sap_num[i] == NULL ||
    ptr_ccddata_get_max_primitive_id[i] == NULL ||
    ptr_ccddata_get_max_pstruct_len[i] == NULL ||
    ptr_ccddata_get_mvar[i] == NULL ||
    ptr_ccddata_get_spare[i] == NULL ||
    ptr_ccddata_get_calc[i] == NULL ||
    ptr_ccddata_get_mcomp[i] == NULL ||
    ptr_ccddata_get_melem[i] == NULL ||
    ptr_ccddata_get_calcidx[i] == NULL ||
    ptr_ccddata_get_mmtx[i] == NULL ||
    ptr_ccddata_get_pvar[i] == NULL ||
    ptr_ccddata_get_pcomp[i] == NULL ||
    ptr_ccddata_get_pelem[i] == NULL ||
    ptr_ccddata_get_pmtx[i] == NULL ||
    ptr_ccddata_get_mval[i] == NULL ||
    ptr_ccddata_get_mstr[i] == NULL ||
    ptr_ccddata_get_pval[i] == NULL ||
    ptr_ccddata_get_pstr[i] == NULL ||
    ptr_ccddata_get_ccdent[i] == NULL ||
    ptr_ccddata_get_entname[i] == NULL ||
    ptr_ccddata_get_alias[i] == NULL ||
    ptr_ccddata_get_version[i] == NULL ||
    ptr_ccddata_get_table_version[i] == NULL ||
    ((ptr_ccddata_get_mi_length[i] == NULL) && (mi_length[i] == NULL)) ||
    ((ptr_ccddata_get_decmsgbuffer[i] == NULL) &&
      (decmsgbuffer[i] == NULL)) ||
    ptr_ccddata_tap_check_pd[i] == NULL ||
    ptr_ccddata_tap_get_pd[i] == NULL)
  {
    (void) ccddata_exit ();
    return CCDDATA_DLL_LOADFUN;
  }

  (void) (*ptr_ccd_init_ccddata[i]) ();
  (void) (*ptr_ccd_init[i]) ();
  (void) (*ptr_cde_init[i]) ();
  (void) (*ptr_pcon_init_ccddata[i]) ();
  (void) (*ptr_pcon_init_prim_coding[i]) (0, PCON_LITTLE);

  if (!already_loaded && ccddata_i_notify != NULL)
  {
    (*ccddata_i_notify)();
  }

  return CCDDATA_DLL_OK;
}

/*
+------------------------------------------------------------------------------
|  Function     :  ccddata_ehandler
+------------------------------------------------------------------------------
|  Description  :  Handle NULL pointer; if we had loaded a ccddata dll
|                  but any other process had loaded another one in the
|                  meantime, the old one is unloaded and the new one is
<                  attached to.
|
|  Parameters   :  -
|
|  Return       :  An error code or the return value of ccddata_attach
+------------------------------------------------------------------------------
*/
static int ccddata_ehandler (void)
{
  if (me < 0)
  {
    return CCDDATA_DLL_REGISTER;
  }

  // functions not loaded -> reattach
  ccddata_detach (me, 1);
  return ccddata_attach (me);
}
/*==== PUBLIC FUNCTIONS ======================================================*/

/* pcon_init_prim_coding ==================================================== */
ULONG pcon_init_prim_coding (T_HANDLE caller, UBYTE format)
{
  ULONG ret = PCON_DLLFUNC_ERROR;
  if (me < 0)
  {
    return ret;
  }
  WaitForSingleObject (mut[me], INFINITE);
  if ((ptr_pcon_init_prim_coding[me] != NULL) ||
      (ccddata_ehandler() == CCDDATA_DLL_OK))
  {
    __try
    {
      ret = (*ptr_pcon_init_prim_coding[me]) (caller, format);
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
      // acces violation -> possibly wrong ccddata-DLL
    }
  }
  ReleaseMutex (mut[me]);
  return ret;
}

/* pcon_decodePrim ========================================================== */
ULONG pcon_decodePrim (ULONG opc,
                       void ** decoded_prim,
                       void * coded_prim,
                       ULONG * length,
                       ULONG woff)
{
  ULONG ret = PCON_DLLFUNC_ERROR;
  if (me < 0)
  {
    return ret;
  }
  WaitForSingleObject (mut[me], INFINITE);
  if ((ptr_pcon_decodePrim[me] != NULL) ||
      (ccddata_ehandler() == CCDDATA_DLL_OK))
  {
    __try
    {
      ret = (*ptr_pcon_decodePrim[me])(opc, decoded_prim, coded_prim, length,
                                         woff);
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
      // acces violation -> possibly wrong ccddata-DLL
    }
  }
  ReleaseMutex (mut[me]);
  return ret;
}

/* pcon_codePrim  ============================================================*/
ULONG pcon_codePrim (ULONG opc,
                     void * decoded_prim,
                     void ** coded_prim,
                     ULONG * length,
                     ULONG woff,
                     char* receiver)
{
  ULONG ret = PCON_DLLFUNC_ERROR;
  if (me < 0)
  {
    return ret;
  }
  WaitForSingleObject (mut[me], INFINITE);
  if ((ptr_pcon_codePrim[me] != NULL) || (ccddata_ehandler() == CCDDATA_DLL_OK))
  {
    __try
    {
      ret = (*ptr_pcon_codePrim[me])(opc, decoded_prim, coded_prim, length,
                                     woff, receiver);
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
      // acces violation -> possibly wrong ccddata-DLL
    }
  }
  ReleaseMutex (mut[me]);
  return ret;
}

/* pcon_init_ccddata  ========================================================*/
ULONG pcon_init_ccddata (void)
{
  ULONG ret = PCON_DLLFUNC_ERROR;
  if (me < 0)
  {
    return ret;
  }
  WaitForSingleObject (mut[me], INFINITE);
  if ((ptr_pcon_init_ccddata[me] != NULL) ||
      (ccddata_ehandler() == CCDDATA_DLL_OK))
  {
    __try
    {
      ret = (*ptr_pcon_init_ccddata[me])();
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
      // acces violation -> possibly wrong ccddata-DLL
    }
  }
  ReleaseMutex (mut[me]);
  return ret;
}

/* pcon_make_filter  =========================================================*/
ULONG pcon_make_filter (char* string, void** prim)
{
  ULONG ret = PCON_DLLFUNC_ERROR;
  if (me < 0)
  {
    return ret;
  }
  WaitForSingleObject (mut[me], INFINITE);
  if ((ptr_pcon_make_filter[me] != NULL) ||
      (ccddata_ehandler() == CCDDATA_DLL_OK))
  {
    __try
    {
      ret = (*ptr_pcon_make_filter[me])(string, prim);
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
      // acces violation -> possibly wrong ccddata-DLL
    }
  }
  ReleaseMutex (mut[me]);
  return ret;
}

/* pcon_filter_decode_prim ===================================================*/
ULONG pcon_filter_decode_prim (ULONG opc,
                               void** decoded_prim,
                               void* coded_prim,
                               ULONG* length,
                               ULONG woff,
                               void** shadow_prim)
{
  ULONG ret = PCON_DLLFUNC_ERROR;
  if (me < 0)
  {
    return ret;
  }
  WaitForSingleObject (mut[me], INFINITE);
  if ((ptr_pcon_filter_decode_prim[me] != NULL) ||
      (ccddata_ehandler() == CCDDATA_DLL_OK))
  {
    __try
    {
      ret = (*ptr_pcon_filter_decode_prim[me])(opc, decoded_prim, coded_prim,
                                               length, woff, shadow_prim);
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
      // acces violation -> possibly wrong ccddata-DLL
    }
  }
  ReleaseMutex (mut[me]);
  return ret;
}

/* pcon_codePrim  ============================================================*/
/* ccddata_get_num_of_entities  ==============================================*/
int ccddata_get_num_of_entities (void)
{
  int ret = 0;
  if (me < 0)
  {
    return ret;
  }
  WaitForSingleObject (mut[me], INFINITE);
  if ((ptr_ccddata_get_num_of_entities[me] != NULL) ||
      (ccddata_ehandler() == CCDDATA_DLL_OK))
  {
    __try
    {
      ret = (*ptr_ccddata_get_num_of_entities[me])();
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
      // acces violation -> possibly wrong ccddata-DLL
    }
  }
  ReleaseMutex (mut[me]);
  return ret;
}

/* ccddata_get_pdi_dinfo  ==============================================*/
int ccddata_get_pdi_dinfo (const T_PDI_DECODEINFO* (*dinfo) )
{
  int ret = 0;
  if (me < 0)
  {
    return ret;
  }
  WaitForSingleObject (mut[me], INFINITE);
  if ((ptr_ccddata_get_pdi_dinfo[me] != NULL) ||
      (ccddata_ehandler() == CCDDATA_DLL_OK))
  {
    __try
    {
      ret = (*ptr_ccddata_get_pdi_dinfo[me])(dinfo);
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
      // acces violation -> possibly wrong ccddata-DLL
    }
  }
  ReleaseMutex (mut[me]);
  return ret;
}

/* ccddata_get_max_message_id  ===============================================*/
int ccddata_get_max_message_id (void)
{
  int ret = 0;
  if (me < 0)
  {
    return ret;
  }
  WaitForSingleObject (mut[me], INFINITE);
  if ((ptr_ccddata_get_max_message_id[me] != NULL) ||
      (ccddata_ehandler() == CCDDATA_DLL_OK))
  {
    __try
    {
      ret = (*ptr_ccddata_get_max_message_id[me])();
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
      // acces violation -> possibly wrong ccddata-DLL
    }
  }
  ReleaseMutex (mut[me]);
  return ret;
}

/* ccddata_get_max_bitstream_len  ============================================*/
int ccddata_get_max_bitstream_len (void)
{
  int ret = CCDDATA_MCONST_ERROR;
  if (me < 0)
  {
    return ret;
  }
  WaitForSingleObject (mut[me], INFINITE);
  if ((ptr_ccddata_get_max_bitstream_len[me] != NULL) ||
      (ccddata_ehandler() == CCDDATA_DLL_OK))
  {
    __try
    {
      ret = (*ptr_ccddata_get_max_bitstream_len[me])();
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
      // acces violation -> possibly wrong ccddata-DLL
    }
  }
  ReleaseMutex (mut[me]);
  return ret;
}

/* ccddata_get_max_mstruct_len  ==============================================*/
int ccddata_get_max_mstruct_len (void)
{
  int ret = CCDDATA_MCONST_ERROR;
  if (me < 0)
  {
    return ret;
  }
  WaitForSingleObject (mut[me], INFINITE);
  if ((ptr_ccddata_get_max_mstruct_len[me] != NULL) ||
      (ccddata_ehandler() == CCDDATA_DLL_OK))
  {
    __try
    {
      ret = (*ptr_ccddata_get_max_mstruct_len[me])();
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
      // acces violation -> possibly wrong ccddata-DLL
    }
  }
  ReleaseMutex (mut[me]);
  return ret;
}

/* ccddata_mccd_symbols  =====================================================*/
int ccddata_mccd_symbols (void)
{
  int ret = CCDDATA_MCONST_ERROR;
  if (me < 0)
  {
    return ret;
  }
  WaitForSingleObject (mut[me], INFINITE);
  if ((ptr_ccddata_mccd_symbols[me] != NULL) ||
      (ccddata_ehandler() == CCDDATA_DLL_OK))
  {
    __try
    {
      ret = (*ptr_ccddata_mccd_symbols[me])();
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
      // acces violation -> possibly wrong ccddata-DLL
    }
  }
  ReleaseMutex (mut[me]);
  return ret;
}

/* ccddata_get_max_sap_num  ==================================================*/
int ccddata_get_max_sap_num (void)
{
  int ret = 0;
  if (me < 0)
  {
    return ret;
  }
  WaitForSingleObject (mut[me], INFINITE);
  if ((ptr_ccddata_get_max_sap_num[me] != NULL) ||
      (ccddata_ehandler() == CCDDATA_DLL_OK))
  {
    __try
    {
      ret = (*ptr_ccddata_get_max_sap_num[me])();
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
      // acces violation -> possibly wrong ccddata-DLL
    }
  }
  ReleaseMutex (mut[me]);
  return ret;
}

/* ccddata_get_max_primitive_id  =============================================*/
int ccddata_get_max_primitive_id (void)
{
  int ret = 0;
  if (me < 0)
  {
    return ret;
  }
  WaitForSingleObject (mut[me], INFINITE);
  if ((ptr_ccddata_get_max_primitive_id[me] != NULL) ||
      (ccddata_ehandler() == CCDDATA_DLL_OK))
  {
    __try
    {
      ret = (*ptr_ccddata_get_max_primitive_id[me])();
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
      // acces violation -> possibly wrong ccddata-DLL
    }
  }
  ReleaseMutex (mut[me]);
  return ret;
}

/* ccddata_get_max_pstruct_len  ==============================================*/
int ccddata_get_max_pstruct_len (void)
{
  int ret = CCDDATA_PCONST_ERROR;
  if (me < 0)
  {
    return ret;
  }
  WaitForSingleObject (mut[me], INFINITE);
  if ((ptr_ccddata_get_max_pstruct_len[me] != NULL) ||
      (ccddata_ehandler() == CCDDATA_DLL_OK))
  {
    __try
    {
      ret = (*ptr_ccddata_get_max_pstruct_len[me])();
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
      // acces violation -> possibly wrong ccddata-DLL
    }
  }
  ReleaseMutex (mut[me]);
  return ret;
}

/* ccddata_get_calcidx  ======================================================*/
const T_CCD_CalcIndex* ccddata_get_calcidx (USHORT idx)
{
  const T_CCD_CalcIndex* ret = NULL;
  if (me < 0)
  {
    return ret;
  }
  WaitForSingleObject (mut[me], INFINITE);
  if ((ptr_ccddata_get_calcidx[me] != NULL) ||
      (ccddata_ehandler() == CCDDATA_DLL_OK))
  {
    __try
    {
      ret = (*ptr_ccddata_get_calcidx[me])(idx);
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
      // acces violation -> possibly wrong ccddata-DLL
    }
  }
  ReleaseMutex (mut[me]);
  return ret;
}

/* ccddata_get_mvar  =========================================================*/
const T_CCD_VarTabEntry* ccddata_get_mvar (USHORT idx)
{
  const T_CCD_VarTabEntry* ret = NULL;
  if (me < 0)
  {
    return ret;
  }
  WaitForSingleObject (mut[me], INFINITE);
  if ((ptr_ccddata_get_mvar[me] != NULL) ||
      (ccddata_ehandler() == CCDDATA_DLL_OK))
  {
    __try
    {
      ret = (*ptr_ccddata_get_mvar[me])(idx);
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
      // acces violation -> possibly wrong ccddata-DLL
    }
  }
  ReleaseMutex (mut[me]);
  return ret;
}

/* ccddata_get_spare  ========================================================*/
const T_CCD_SpareTabEntry* ccddata_get_spare (USHORT idx)
{
  const T_CCD_SpareTabEntry* ret = NULL;
  if (me < 0)
  {
    return ret;
  }
  WaitForSingleObject (mut[me], INFINITE);
  if ((ptr_ccddata_get_spare[me] != NULL) ||
      (ccddata_ehandler() == CCDDATA_DLL_OK))
  {
    __try
    {
      ret = (*ptr_ccddata_get_spare[me])(idx);
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
      // acces violation -> possibly wrong ccddata-DLL
    }
  }
  ReleaseMutex (mut[me]);
  return ret;
}

/* ccddata_get_calc  =========================================================*/
const T_CCD_CalcTabEntry* ccddata_get_calc (USHORT idx)
{
  const T_CCD_CalcTabEntry* ret = NULL;
  if (me < 0)
  {
    return ret;
  }
  WaitForSingleObject (mut[me], INFINITE);
  if ((ptr_ccddata_get_calc[me] != NULL) ||
      (ccddata_ehandler() == CCDDATA_DLL_OK))
  {
    __try
    {
      ret = (*ptr_ccddata_get_calc[me])(idx);
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
      // acces violation -> possibly wrong ccddata-DLL
    }
  }
  ReleaseMutex (mut[me]);
  return ret;
}

/* ccddata_get_mcomp  ========================================================*/
const T_CCD_CompTabEntry* ccddata_get_mcomp (USHORT idx)
{
  const T_CCD_CompTabEntry* ret = NULL;
  if (me < 0)
  {
    return ret;
  }
  WaitForSingleObject (mut[me], INFINITE);
  if ((ptr_ccddata_get_mcomp[me] != NULL) ||
      (ccddata_ehandler() == CCDDATA_DLL_OK))
  {
    __try
    {
      ret = (*ptr_ccddata_get_mcomp[me])(idx);
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
      // acces violation -> possibly wrong ccddata-DLL
    }
  }
  ReleaseMutex (mut[me]);
  return ret;
}

/* ccddata_get_melem  ========================================================*/
const T_CCD_ElemTabEntry* ccddata_get_melem (USHORT idx)
{
  const T_CCD_ElemTabEntry* ret = NULL;
  if (me < 0)
  {
    return ret;
  }
  WaitForSingleObject (mut[me], INFINITE);
  if ((ptr_ccddata_get_melem[me] != NULL) ||
      (ccddata_ehandler() == CCDDATA_DLL_OK))
  {
    __try
    {
      ret = (*ptr_ccddata_get_melem[me])(idx);
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
      // acces violation -> possibly wrong ccddata-DLL
    }
  }
  ReleaseMutex (mut[me]);
  return ret;
}

/* ccddata_get_mmtx  =========================================================*/
USHORT ccddata_get_mmtx (USHORT entity, USHORT msgid, USHORT index)
{
  USHORT ret = NO_REF;
  if (me < 0)
  {
    return ret;
  }
  WaitForSingleObject (mut[me], INFINITE);
  if ((ptr_ccddata_get_mmtx[me] != NULL) ||
      (ccddata_ehandler() == CCDDATA_DLL_OK))
  {
    __try
    {
      ret = (*ptr_ccddata_get_mmtx[me])(entity, msgid, index);
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
      // acces violation -> possibly wrong ccddata-DLL
    }
  }
  ReleaseMutex (mut[me]);
  return ret;
}

/* ccddata_get_pvar  =========================================================*/
const T_CCD_VarTabEntry* ccddata_get_pvar (USHORT idx)
{
  const T_CCD_VarTabEntry* ret = NULL;
  if (me < 0)
  {
    return ret;
  }
  WaitForSingleObject (mut[me], INFINITE);
  if ((ptr_ccddata_get_pvar[me] != NULL) ||
      (ccddata_ehandler() == CCDDATA_DLL_OK))
  {
    __try
    {
      ret = (*ptr_ccddata_get_pvar[me])(idx);
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
      // acces violation -> possibly wrong ccddata-DLL
    }
  }
  ReleaseMutex (mut[me]);
  return ret;
}

/* ccddata_get_pcomp  ========================================================*/
const T_CCD_CompTabEntry* ccddata_get_pcomp (USHORT idx)
{
  const T_CCD_CompTabEntry* ret = NULL;
  if (me < 0)
  {
    return ret;
  }
  WaitForSingleObject (mut[me], INFINITE);
  if ((ptr_ccddata_get_pcomp[me] != NULL) ||
      (ccddata_ehandler() == CCDDATA_DLL_OK))
  {
    __try
    {
      ret = (*ptr_ccddata_get_pcomp[me])(idx);
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
      // acces violation -> possibly wrong ccddata-DLL
    }
  }
  ReleaseMutex (mut[me]);
  return ret;
}

/* ccddata_get_pelem  ========================================================*/
const T_CCD_ElemTabEntry* ccddata_get_pelem (USHORT idx)
{
  const T_CCD_ElemTabEntry* ret = NULL;
  if (me < 0)
  {
    return ret;
  }
  WaitForSingleObject (mut[me], INFINITE);
  if ((ptr_ccddata_get_pelem[me] != NULL) ||
      (ccddata_ehandler() == CCDDATA_DLL_OK))
  {
    __try
    {
      ret = (*ptr_ccddata_get_pelem[me])(idx);
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
      // acces violation -> possibly wrong ccddata-DLL
    }
  }
  ReleaseMutex (mut[me]);
  return ret;
}

/* ccddata_get_pmtx  =========================================================*/
USHORT ccddata_get_pmtx (USHORT sap, USHORT primid, USHORT index)
{
  USHORT ret = NO_REF;
  if (me < 0)
  {
    return ret;
  }
  WaitForSingleObject (mut[me], INFINITE);
  if ((ptr_ccddata_get_pmtx[me] != NULL) ||
      (ccddata_ehandler() == CCDDATA_DLL_OK))
  {
    __try
    {
      ret = (*ptr_ccddata_get_pmtx[me])(sap, primid, index);
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
      // acces violation -> possibly wrong ccddata-DLL
    }
  }
  ReleaseMutex (mut[me]);
  return ret;
}

/* ccddata_get_mval  =========================================================*/
const T_CCD_ValTabEntry* ccddata_get_mval (USHORT idx)
{
  const T_CCD_ValTabEntry* ret = NULL;
  if (me < 0)
  {
    return ret;
  }
  WaitForSingleObject (mut[me], INFINITE);
  if ((ptr_ccddata_get_mval[me] != NULL) ||
      (ccddata_ehandler() == CCDDATA_DLL_OK))
  {
    __try
    {
      ret = (*ptr_ccddata_get_mval[me])(idx);
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
      // acces violation -> possibly wrong ccddata-DLL
    }
  }
  ReleaseMutex (mut[me]);
  return ret;
}

/* ccddata_get_mstr  =========================================================*/
const T_CCD_StrTabEntry* ccddata_get_mstr (USHORT idx)
{
  const T_CCD_StrTabEntry* ret = NULL;
  if (me < 0)
  {
    return ret;
  }
  WaitForSingleObject (mut[me], INFINITE);
  if ((ptr_ccddata_get_mstr[me] != NULL) ||
      (ccddata_ehandler() == CCDDATA_DLL_OK))
  {
    __try
    {
      ret = (*ptr_ccddata_get_mstr[me])(idx);
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
      // acces violation -> possibly wrong ccddata-DLL
    }
  }
  ReleaseMutex (mut[me]);
  return ret;
}

/* ccddata_get_pval  =========================================================*/
const T_CCD_ValTabEntry* ccddata_get_pval (USHORT idx)
{
  const T_CCD_ValTabEntry* ret = NULL;
  if (me < 0)
  {
    return ret;
  }
  WaitForSingleObject (mut[me], INFINITE);
  if ((ptr_ccddata_get_pval[me] != NULL) ||
      (ccddata_ehandler() == CCDDATA_DLL_OK))
  {
    __try
    {
      ret = (*ptr_ccddata_get_pval[me])(idx);
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
      // acces violation -> possibly wrong ccddata-DLL
    }
  }
  ReleaseMutex (mut[me]);
  return ret;
}

/* ccddata_get_pstr  =========================================================*/
const T_CCD_StrTabEntry* ccddata_get_pstr (USHORT idx)
{
  const T_CCD_StrTabEntry* ret = NULL;
  if (me < 0)
  {
    return ret;
  }
  WaitForSingleObject (mut[me], INFINITE);
  if ((ptr_ccddata_get_pstr[me] != NULL) ||
      (ccddata_ehandler() == CCDDATA_DLL_OK))
  {
    __try
    {
      ret = (*ptr_ccddata_get_pstr[me])(idx);
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
      // acces violation -> possibly wrong ccddata-DLL
    }
  }
  ReleaseMutex (mut[me]);
  return ret;
}

/* ccddata_get_ccdent  =======================================================*/
short ccddata_get_ccdent (char* entname)
{
  short ret = CCDDATA_CCDENT_INVALID;
  if (me < 0)
  {
    return ret;
  }
  WaitForSingleObject (mut[me], INFINITE);
  if ((ptr_ccddata_get_ccdent[me] != NULL) ||
      (ccddata_ehandler() == CCDDATA_DLL_OK))
  {
    __try
    {
      ret = (*ptr_ccddata_get_ccdent[me])(entname);
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
      // acces violation -> possibly wrong ccddata-DLL
    }
  }
  ReleaseMutex (mut[me]);
  return ret;
}

/* ccddata_get_entname  =======================================================*/
const char * ccddata_get_entname (short ccdent)
{
  const char * ret = NULL;
  if (me < 0)
  {
    return ret;
  }
  WaitForSingleObject (mut[me], INFINITE);
  if ((ptr_ccddata_get_entname[me] != NULL) ||
      (ccddata_ehandler() == CCDDATA_DLL_OK))
  {
    __try
    {
      ret = (*ptr_ccddata_get_entname[me])(ccdent);
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
      // acces violation -> possibly wrong ccddata-DLL
    }
  }
  ReleaseMutex (mut[me]);
  return ret;
}

/* ccddata_get_alias  ========================================================*/
char* ccddata_get_alias (USHORT idx, int from_msg)
{
  char* ret = "";
  if (me < 0)
  {
    return ret;
  }
  WaitForSingleObject (mut[me], INFINITE);
  if ((ptr_ccddata_get_alias[me] != NULL) ||
      (ccddata_ehandler() == CCDDATA_DLL_OK))
  {
    __try
    {
      ret = (*ptr_ccddata_get_alias[me])(idx, from_msg);
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
      // acces violation -> possibly wrong ccddata-DLL
    }
  }
  ReleaseMutex (mut[me]);
  return ret;
}

/* ccddata_get_version  ======================================================*/
char* ccddata_get_version (void)
{
  char* ret = "";
  if (me < 0)
  {
    return ret;
  }
  WaitForSingleObject (mut[me], INFINITE);
  if ((ptr_ccddata_get_version[me] != NULL) ||
      (ccddata_ehandler() == CCDDATA_DLL_OK))
  {
    __try
    {
      ret = (*ptr_ccddata_get_version[me])();
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
      // acces violation -> possibly wrong ccddata-DLL
    }
  }
  ReleaseMutex (mut[me]);
  return ret;
}

/* ccddata_get_table_version  ================================================*/
int ccddata_get_table_version (void)
{
  int ret = 0;
  if (me < 0)
  {
    return ret;
  }
  WaitForSingleObject (mut[me], INFINITE);
  if ((ptr_ccddata_get_table_version[me] != NULL) ||
      (ccddata_ehandler() == CCDDATA_DLL_OK))
  {
    __try
    {
      ret = (*ptr_ccddata_get_table_version[me])();
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
      // acces violation -> possibly wrong ccddata-DLL
    }
  }
  ReleaseMutex (mut[me]);
  return ret;
}

/* ccddata_get_decmsgbuffer  =================================================*/
UBYTE* ccddata_get_decmsgbuffer (void)
{
  UBYTE* ret = NULL;
  if (me < 0)
  {
    return ret;
  }
  WaitForSingleObject (mut[me], INFINITE);
  if (ptr_ccddata_get_decmsgbuffer[me] == NULL)
  {
    if (decmsgbuffer[me] != NULL)
    {
      ret = decmsgbuffer[me];
    }
    else if (ccddata_ehandler() == CCDDATA_DLL_OK)
    {
      ret = ccddata_get_decmsgbuffer ();
    }
    else
    {
      ;
    }
  }
  else
  {
    __try
    {
      ret = (*ptr_ccddata_get_decmsgbuffer[me])();
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
      // acces violation -> possibly wrong ccddata-DLL
    }
  }
  ReleaseMutex (mut[me]);
  return ret;
}

/* ccddata_get_mi_length  ====================================================*/
UBYTE* ccddata_get_mi_length (void)
{
  UBYTE* ret = NULL;
  if (me < 0)
  {
    return ret;
  }
  WaitForSingleObject (mut[me], INFINITE);
  if (ptr_ccddata_get_mi_length[me] == NULL)
  {
    if (mi_length[me] != NULL)
    {
      ret = mi_length[me];
    }
    else if (ccddata_ehandler() == CCDDATA_DLL_OK)
    {
      ret = ccddata_get_mi_length ();
    }
    else
    {
      ;
    }
  }
  else
  {
    __try
    {
      ret = (*ptr_ccddata_get_mi_length[me])();
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
      // acces violation -> possibly wrong ccddata-DLL
    }
  }
  ReleaseMutex (mut[me]);
  return ret;
}

/* ccddata_tap_get_pd  =======================================================*/
int ccddata_tap_get_pd (UCHAR comp)
{
  int ret = TAP_PD_INVALID;
  if (me < 0)
  {
    return ret;
  }
  WaitForSingleObject (mut[me], INFINITE);
  if ((ptr_ccddata_tap_get_pd[me] != NULL) ||
      (ccddata_ehandler() == CCDDATA_DLL_OK))
  {
    __try
    {
      ret = (*ptr_ccddata_tap_get_pd[me])(comp);
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
      // acces violation -> possibly wrong ccddata-DLL
    }
  }
  ReleaseMutex (mut[me]);
  return ret;
}

/* ccddata_tap_check_pd  =====================================================*/
int ccddata_tap_check_pd (UCHAR comp, UCHAR pd)
{
  int ret = TAP_PD_INVALID;
  if (me < 0)
  {
    return ret;
  }
  WaitForSingleObject (mut[me], INFINITE);
  if ((ptr_ccddata_tap_check_pd[me] != NULL) ||
      (ccddata_ehandler() == CCDDATA_DLL_OK))
  {
    __try
    {
      ret = (*ptr_ccddata_tap_check_pd[me])(comp, pd);
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
      // acces violation -> possibly wrong ccddata-DLL
    }
  }
  ReleaseMutex (mut[me]);
  return ret;
}

/* ccddata_eg_nodes  =========================================================*/
int ccddata_eg_nodes (void)
{
  int ret = 0;
  if (me < 0)
  {
    return ret;
  }
  WaitForSingleObject (mut[me], INFINITE);
  if ((ptr_ccddata_eg_nodes[me] != NULL) ||
      (ccddata_ehandler() == CCDDATA_DLL_OK))
  {
    __try
    {
      ret = (*ptr_ccddata_eg_nodes[me])();
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
      // acces violation -> possibly wrong ccddata-DLL
    }
  }
  ReleaseMutex (mut[me]);
  return ret;
}

/* ccddata_eg_nodenames  =====================================================*/
char** ccddata_eg_nodenames (void)
{
  char** ret = NULL;
  if (me < 0)
  {
    return ret;
  }
  WaitForSingleObject (mut[me], INFINITE);
  if ((ptr_ccddata_eg_nodenames[me] != NULL) ||
      (ccddata_ehandler() == CCDDATA_DLL_OK))
  {
    __try
    {
      ret = (*ptr_ccddata_eg_nodenames[me])();
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
      // acces violation -> possibly wrong ccddata-DLL
    }
  }
  ReleaseMutex (mut[me]);
  return ret;
}

/* ccddata_eg_adjacent  ======================================================*/
char* ccddata_eg_adjacent (int idx)
{
  char* ret = NULL;
  if (me < 0)
  {
    return ret;
  }
  WaitForSingleObject (mut[me], INFINITE);
  if ((ptr_ccddata_eg_adjacent[me] != NULL) ||
      (ccddata_ehandler() == CCDDATA_DLL_OK))
  {
    __try
    {
      ret = (*ptr_ccddata_eg_adjacent[me])(idx);
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
      // acces violation -> possibly wrong ccddata-DLL
    }
  }
  ReleaseMutex (mut[me]);
  return ret;
}

/* ccddata_eg_saps  ==========================================================*/
int ccddata_eg_saps (void)
{
  int ret = 0;
  if (me < 0)
  {
    return ret;
  }
  WaitForSingleObject (mut[me], INFINITE);
  if ((ptr_ccddata_eg_saps[me] != NULL) ||
      (ccddata_ehandler() == CCDDATA_DLL_OK))
  {
    __try
    {
      ret = (*ptr_ccddata_eg_saps[me])();
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
      // acces violation -> possibly wrong ccddata-DLL
    }
  }
  ReleaseMutex (mut[me]);
  return ret;
}

/* ccddata_eg_sapnames  ======================================================*/
char** ccddata_eg_sapnames (void)
{
  char** ret = NULL;
  if (me < 0)
  {
    return ret;
  }
  WaitForSingleObject (mut[me], INFINITE);
  if ((ptr_ccddata_eg_sapnames[me] != NULL) ||
      (ccddata_ehandler() == CCDDATA_DLL_OK))
  {
    __try
    {
      ret = (*ptr_ccddata_eg_sapnames[me])();
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
      // acces violation -> possibly wrong ccddata-DLL
    }
  }
  ReleaseMutex (mut[me]);
  return ret;
}

/* ccddata_eg_comendpoints  ==================================================*/
T_COMENDPOINTS* ccddata_eg_comendpoints (int idx)
{
  T_COMENDPOINTS* ret = NULL;
  if (me < 0)
  {
    return ret;
  }
  WaitForSingleObject (mut[me], INFINITE);
  if ((ptr_ccddata_eg_comendpoints[me] != NULL) ||
      (ccddata_ehandler() == CCDDATA_DLL_OK))
  {
    __try
    {
      ret = (*ptr_ccddata_eg_comendpoints[me])(idx);
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
      // acces violation -> possibly wrong ccddata-DLL
    }
  }
  ReleaseMutex (mut[me]);
  return ret;
}

/* cde_init  =================================================================*/
void cde_init (void)
{
  if (me < 0)
  {
    return;
  }
  WaitForSingleObject (mut[me], INFINITE);
  if ((ptr_cde_init[me] != NULL) || (ccddata_ehandler() == CCDDATA_DLL_OK))
  {
    __try
    {
      (*ptr_cde_init[me])();
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
      // acces violation -> possibly wrong ccddata-DLL
    }
  }
  ReleaseMutex (mut[me]);
}

/* cde_get_comp  =============================================================*/
USHORT cde_get_comp (T_CCDE_HANDLE*     chandle,
                     T_CCDE_ELEM_DESCR* edescr)
{
  USHORT ret = CCDEDIT_DLL_ERROR;
  if (me < 0)
  {
    return ret;
  }
  WaitForSingleObject (mut[me], INFINITE);
  if ((ptr_cde_get_comp[me] != NULL) || (ccddata_ehandler() == CCDDATA_DLL_OK))
  {
    __try
    {
      ret = (*ptr_cde_get_comp[me])(chandle, edescr);
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
      // acces violation -> possibly wrong ccddata-DLL
    }
  }
  ReleaseMutex (mut[me]);
  return ret;
}

/* cde_prim_first  ===========================================================*/
USHORT cde_prim_first (T_CCDE_HANDLE    * phandle,
                       ULONG              primcode,
                       char             * name)
{
  USHORT ret = CCDEDIT_DLL_ERROR;
  if (me < 0)
  {
    return ret;
  }
  WaitForSingleObject (mut[me], INFINITE);
  if ((ptr_cde_prim_first[me] != NULL) ||
      (ccddata_ehandler() == CCDDATA_DLL_OK))
  {
    __try
    {
      ret = (*ptr_cde_prim_first[me])(phandle, primcode, name);
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
      // acces violation -> possibly wrong ccddata-DLL
    }
  }
  ReleaseMutex (mut[me]);
  return ret;
}

/* cde_prim_next  ============================================================*/
USHORT cde_prim_next (T_CCDE_HANDLE      *phandle,
                      UBYTE               descent,
                      T_CCDE_ELEM_DESCR  *pdescr)
{
  USHORT ret = CCDEDIT_DLL_ERROR;
  if (me < 0)
  {
    return ret;
  }
  WaitForSingleObject (mut[me], INFINITE);
  if ((ptr_cde_prim_next[me] != NULL) || (ccddata_ehandler() == CCDDATA_DLL_OK))
  {
    __try
    {
      ret = (*ptr_cde_prim_next[me])(phandle, descent, pdescr);
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
      // acces violation -> possibly wrong ccddata-DLL
    }
  }
  ReleaseMutex (mut[me]);
  return ret;
}

/* cde_msg_first  ============================================================*/
USHORT cde_msg_first (T_CCDE_HANDLE  * mhandle,
                      UBYTE            type,
                      UBYTE            direction,
                      UBYTE            entity,
                      char           * name)

{
  USHORT ret = CCDEDIT_DLL_ERROR;
  if (me < 0)
  {
    return ret;
  }
  WaitForSingleObject (mut[me], INFINITE);
  if ((ptr_cde_msg_first[me] != NULL) || (ccddata_ehandler() == CCDDATA_DLL_OK))
  {
    __try
    {
      ret = (*ptr_cde_msg_first[me])(mhandle, type, direction, entity, name);
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
      // acces violation -> possibly wrong ccddata-DLL
    }
  }
  ReleaseMutex (mut[me]);
  return ret;
}

/* cde_msg_next  =============================================================*/
USHORT cde_msg_next (T_CCDE_HANDLE     *mhandle,
                     UBYTE              descent,
                     T_CCDE_ELEM_DESCR *iedescr)
{
  USHORT ret = CCDEDIT_DLL_ERROR;
  if (me < 0)
  {
    return ret;
  }
  WaitForSingleObject (mut[me], INFINITE);
  if ((ptr_cde_msg_next[me] != NULL) || (ccddata_ehandler() == CCDDATA_DLL_OK))
  {
    __try
    {
      ret = (*ptr_cde_msg_next[me])(mhandle, descent, iedescr);
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
      // acces violation -> possibly wrong ccddata-DLL
    }
  }
  ReleaseMutex (mut[me]);
  return ret;
}

/* cde_comp_first  ===========================================================*/
USHORT cde_comp_first (T_CCDE_HANDLE      * chandle,
                       T_ELM_SRC            source,
                       char               * compname)

{
  USHORT ret = CCDEDIT_DLL_ERROR;
  if (me < 0)
  {
    return ret;
  }
  WaitForSingleObject (mut[me], INFINITE);
  if ((ptr_cde_comp_first[me] != NULL) ||
      (ccddata_ehandler() == CCDDATA_DLL_OK))
  {
    __try
    {
      ret = (*ptr_cde_comp_first[me])(chandle, source, compname);
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
      // acces violation -> possibly wrong ccddata-DLL
    }
  }
  ReleaseMutex (mut[me]);
  return ret;
}

/* cde_comp_next  ============================================================*/
USHORT cde_comp_next (T_CCDE_HANDLE     *chandle,
                      UBYTE              descent,
                      T_CCDE_ELEM_DESCR *descr)
{
  USHORT ret = CCDEDIT_DLL_ERROR;
  if (me < 0)
  {
    return ret;
  }
  WaitForSingleObject (mut[me], INFINITE);
  if ((ptr_cde_comp_next[me] != NULL) || (ccddata_ehandler() == CCDDATA_DLL_OK))
  {
    __try
    {
      ret = (*ptr_cde_comp_next[me])(chandle, descent, descr);
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
      // acces violation -> possibly wrong ccddata-DLL
    }
  }
  ReleaseMutex (mut[me]);
  return ret;
}

/* cde_get_symval  ===========================================================*/
char* cde_get_symval (int elem_value, T_CCDE_ELEM_DESCR* edescr)
{
  char* ret = NULL;
  if (me < 0)
  {
    return ret;
  }
  WaitForSingleObject (mut[me], INFINITE);
  if ((ptr_cde_get_symval[me] != NULL) ||
      (ccddata_ehandler() == CCDDATA_DLL_OK))
  {
    __try
    {
      ret = (*ptr_cde_get_symval[me])(elem_value, edescr);
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
      // acces violation -> possibly wrong ccddata-DLL
    }
  }
  ReleaseMutex (mut[me]);
  return ret;
}

/* cde_read_elem  ============================================================*/
USHORT cde_read_elem (T_CCDE_HANDLE     * handle,
                      void              * cstruct,
                      T_CCDE_ELEM_DESCR * edescr,
                      UBYTE             * value)
{
  USHORT ret = CCDEDIT_DLL_ERROR;
  if (me < 0)
  {
    return ret;
  }
  WaitForSingleObject (mut[me], INFINITE);
  if ((ptr_cde_read_elem[me] != NULL) || (ccddata_ehandler() == CCDDATA_DLL_OK))
  {
    __try
    {
      ret = (*ptr_cde_read_elem[me])(handle, cstruct, edescr, value);
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
      // acces violation -> possibly wrong ccddata-DLL
    }
  }
  ReleaseMutex (mut[me]);
  return ret;
}

/* cde_write_prepare  ========================================================*/
void cde_write_prepare (T_CCDE_HANDLE     * handle,
                        void              * cstruct,
                        T_CCDE_ELEM_DESCR * edescr)
{
  if (me < 0)
  {
    return;
  }
  WaitForSingleObject (mut[me], INFINITE);
  if ((ptr_cde_write_prepare[me] != NULL) ||
      (ccddata_ehandler() == CCDDATA_DLL_OK))
  {
    (*ptr_cde_write_prepare[me])(handle, cstruct, edescr);
  }
  ReleaseMutex (mut[me]);
}

/* cde_write_elem  ===========================================================*/
USHORT cde_write_elem (T_CCDE_HANDLE     * handle,
                       void              * cstruct,
                       T_CCDE_ELEM_DESCR * edescr,
                       UBYTE             * value)
{
  USHORT ret = CCDEDIT_DLL_ERROR;
  if (me < 0)
  {
    return ret;
  }
  WaitForSingleObject (mut[me], INFINITE);
  if ((ptr_cde_write_elem[me] != NULL) ||
      (ccddata_ehandler() == CCDDATA_DLL_OK))
  {
    __try
    {
      ret = (*ptr_cde_write_elem[me])(handle, cstruct, edescr, value);
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
      // acces violation -> possibly wrong ccddata-DLL
    }
  }
  ReleaseMutex (mut[me]);
  return ret;
}

/* cde_get_type  ===========================================================*/
USHORT cde_get_type (char      *name,
                     T_ELM_SRC *type)
{
  USHORT ret = CCDEDIT_DLL_ERROR;
  if (me < 0)
  {
    return ret;
  }
  WaitForSingleObject (mut[me], INFINITE);
  if ((ptr_cde_get_type[me] != NULL) || (ccddata_ehandler() == CCDDATA_DLL_OK))
  {
    __try
    {
      ret = (*ptr_cde_get_type[me])(name, type);
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
      // acces violation -> possibly wrong ccddata-DLL
    }
  }
  ReleaseMutex (mut[me]);
  return ret;
}

/* cde_get_primcode  =========================================================*/
USHORT cde_get_primcode (char      *name,
                         ULONG     *primcode)
{
  USHORT ret = CCDEDIT_DLL_ERROR;
  if (me < 0)
  {
    return ret;
  }
  WaitForSingleObject (mut[me], INFINITE);
  if ((ptr_cde_get_primcode[me] != NULL) ||
      (ccddata_ehandler() == CCDDATA_DLL_OK))
  {
    __try
    {
      ret = (*ptr_cde_get_primcode[me])(name, primcode);
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
      // acces violation -> possibly wrong ccddata-DLL
    }
  }
  ReleaseMutex (mut[me]);
  return ret;
}

/* cde_get_msgcode  ==========================================================*/
USHORT cde_get_msgcode (char      *name,
                        UBYTE     *type,
                        UBYTE     *direction,
                        UBYTE     *entity)
{
  USHORT ret = CCDEDIT_DLL_ERROR;
  if (me < 0)
  {
    return ret;
  }
  WaitForSingleObject (mut[me], INFINITE);
  if ((ptr_cde_get_msgcode[me] != NULL) ||
      (ccddata_ehandler() == CCDDATA_DLL_OK))
  {
    __try
    {
      ret = (*ptr_cde_get_msgcode[me])(name, type, direction, entity);
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
      // acces violation -> possibly wrong ccddata-DLL
    }
  }
  ReleaseMutex (mut[me]);
  return ret;
}

/* cde_get_is_downlink  ======================================================*/
int cde_get_is_downlink (ULONG comp_index)
{
  int ret = CCDEDIT_DLL_ERROR;
  if (me < 0)
  {
    return ret;
  }
  WaitForSingleObject (mut[me], INFINITE);
  if ((ptr_cde_get_is_downlink[me] != NULL) ||
      (ccddata_ehandler() == CCDDATA_DLL_OK))
  {
    __try
    {
      ret = (*ptr_cde_get_is_downlink[me])(comp_index);
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
      // acces violation -> possibly wrong ccddata-DLL
    }
  }
  ReleaseMutex (mut[me]);
  return ret;
}

/* cde_get_comp_index  =======================================================*/
ULONG cde_get_comp_index (char* comp_name, T_ELM_SRC table)
{
  ULONG ret = CCDEDIT_DLL_ERROR;
  if (me < 0)
  {
    return ret;
  }
  WaitForSingleObject (mut[me], INFINITE);
  if ((ptr_cde_get_comp_index[me] != NULL) ||
      (ccddata_ehandler() == CCDDATA_DLL_OK))
  {
    __try
    {
      ret = (*ptr_cde_get_comp_index[me])(comp_name, table);
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
      // acces violation -> possibly wrong ccddata-DLL
    }
  }
  ReleaseMutex (mut[me]);
  return ret;
}

/* cde_get_element_name  =====================================================*/
char* cde_get_element_name (ULONG comp_index, USHORT elem_off, T_ELM_SRC table)
{
  char* ret = NULL;
  if (me < 0)
  {
    return ret;
  }
  WaitForSingleObject (mut[me], INFINITE);
  if ((ptr_cde_get_element_name[me] != NULL) ||
      (ccddata_ehandler() == CCDDATA_DLL_OK))
  {
    __try
    {
      ret = (*ptr_cde_get_element_name[me])(comp_index, elem_off, table);
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
      // acces violation -> possibly wrong ccddata-DLL
    }
  }
  ReleaseMutex (mut[me]);
  return ret;
}

/* cde_get_array_kind  =======================================================*/
ULONG cde_get_array_kind (char* var_name, T_ELM_SRC table)
{
  ULONG ret = (ULONG)CCDEDIT_ERROR;
  if (me < 0)
  {
    return ret;
  }
  WaitForSingleObject (mut[me], INFINITE);
  if ((ptr_cde_get_array_kind[me] != NULL) ||
      (ccddata_ehandler() == CCDDATA_DLL_OK))
  {
    __try
    {
      ret = (*ptr_cde_get_array_kind[me])(var_name, table);
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
      // acces violation -> possibly wrong ccddata-DLL
    }
  }
  ReleaseMutex (mut[me]);
  return ret;
}

/* ccd_set_patch_infos  ======================================================*/
int ccd_set_patch_infos (T_patch_info* pinfo)
{
  int ret = CCD_DLL_ERROR;
  if (me < 0)
  {
    return ret;
  }
  WaitForSingleObject (mut[me], INFINITE);
  if ((ptr_ccd_set_patch_infos[me] != NULL) ||
      (ccddata_ehandler() == CCDDATA_DLL_OK))
  {
    __try
    {
      ret = (*ptr_ccd_set_patch_infos[me])(pinfo);
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
      // acces violation -> possibly wrong ccddata-DLL
    }
  }
  ReleaseMutex (mut[me]);
  return ret;
}

/* ccd_init  =================================================================*/
BYTE ccd_init (void)
{
  BYTE ret = CCD_DLL_ERROR;
  if (me < 0)
  {
    return ret;
  }
  WaitForSingleObject (mut[me], INFINITE);
  if ((ptr_ccd_init[me] != NULL) || (ccddata_ehandler() == CCDDATA_DLL_OK))
  {
    __try
    {
      ret = (*ptr_ccd_init[me])();
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
      // acces violation -> possibly wrong ccddata-DLL
    }
  }
  ReleaseMutex (mut[me]);
  return ret;
}

/* ccd_exit  =================================================================*/
int ccd_exit (void)
{
  int ret = CCD_DLL_ERROR;
  if (me < 0)
  {
    return ret;
  }
  WaitForSingleObject (mut[me], INFINITE);
  if ((ptr_ccd_exit[me] != NULL) || (ccddata_ehandler() == CCDDATA_DLL_OK))
  {
    __try
    {
      ret = (*ptr_ccd_exit[me])();
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
      // acces violation -> possibly wrong ccddata-DLL
    }
  }
  ReleaseMutex (mut[me]);
  return ret;
}

/* ccd_begin  ================================================================*/
UBYTE* ccd_begin (void)
{
  UBYTE* ret = NULL;
  if (me < 0)
  {
    return ret;
  }
  WaitForSingleObject (mut[me], INFINITE);
  if ((ptr_ccd_begin[me] != NULL) || (ccddata_ehandler() == CCDDATA_DLL_OK))
  {
    __try
    {
      ret = (*ptr_ccd_begin[me])();
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
      // acces violation -> possibly wrong ccddata-DLL
    }
  }
  ReleaseMutex (mut[me]);
  return ret;
}

/* ccd_end  ==================================================================*/
void ccd_end (void)
{
  if (me < 0)
  {
    return;
  }
  WaitForSingleObject (mut[me], INFINITE);
  if ((ptr_ccd_end[me] != NULL) || (ccddata_ehandler() == CCDDATA_DLL_OK))
  {
    (*ptr_ccd_end[me])();
  }
  ReleaseMutex (mut[me]);
}

/* ccd_decodeMsg  ============================================================*/
BYTE ccd_decodeMsg (UBYTE         entity,
                    UBYTE         direction,
                    T_MSGBUF     *mBuf,
                    UBYTE        *mStruct,
                    UBYTE         mId)
{
  BYTE ret = CCD_DLL_ERROR;
  if (me < 0)
  {
    return ret;
  }
  WaitForSingleObject (mut[me], INFINITE);
  if ((ptr_ccd_decodeMsg[me] != NULL) || (ccddata_ehandler() == CCDDATA_DLL_OK))
  {
    __try
    {
      ret = (*ptr_ccd_decodeMsg[me])(entity, direction, mBuf, mStruct, mId);
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
      // acces violation -> possibly wrong ccddata-DLL
    }
  }
  ReleaseMutex (mut[me]);
  return ret;
}

/* ccd_decodeMsgPtr  =========================================================*/
S8 ccd_decodeMsgPtr (U8   entity,
		       U8   direction,
		       U16  l_buf,
		       U16  o_buf,
		       U8*  buf,
		       U8** mStructPtr,
		       U8   mId)
{
  S8 ret = CCD_DLL_ERROR;
  if (me < 0)
  {
    return ret;
  }
  WaitForSingleObject (mut[me], INFINITE);
  if ((ptr_ccd_decodeMsgPtr[me] != NULL) ||
      (ccddata_ehandler() == CCDDATA_DLL_OK))
  {
    __try
    {
      ret = (*ptr_ccd_decodeMsgPtr[me])(entity, direction, l_buf, o_buf, buf, 
                                      mStructPtr, mId);
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
      // acces violation -> possibly wrong ccddata-DLL
    }
  }
  ReleaseMutex (mut[me]);
  return ret;
}

/* ccd_codeMsg  ========================)=====================================*/
BYTE ccd_codeMsg (UBYTE         entity,
      UBYTE         direction,
      T_MSGBUF     *mBuf,
      UBYTE        *mStruct,
      UBYTE         mId)
{
  BYTE ret = CCD_DLL_ERROR;
  if (me < 0)
  {
    return ret;
  }
  WaitForSingleObject (mut[me], INFINITE);
  if ((ptr_ccd_codeMsg[me] != NULL) || (ccddata_ehandler() == CCDDATA_DLL_OK))
  {
    __try
    {
      ret = (*ptr_ccd_codeMsg[me])(entity, direction, mBuf, mStruct, mId);
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
      // acces violation -> possibly wrong ccddata-DLL
    }
  }
  ReleaseMutex (mut[me]);
  return ret;
}

/* ccd_codeMsgPtr  =========================)=================================*/
S8 ccd_codeMsgPtr(U8   entity,
      U8   direction,
      U16* l_buf,
      U16  o_buf,
      U8*  buf,
      U8*  mStruct,
      U8   mId)
{
  S8 ret = CCD_DLL_ERROR;
  if (me < 0)
  {
    return ret;
  }
  WaitForSingleObject (mut[me], INFINITE);
  if ((ptr_ccd_codeMsgPtr[me] != NULL) ||
      (ccddata_ehandler() == CCDDATA_DLL_OK))
  {
    __try
    {
      ret = (*ptr_ccd_codeMsgPtr[me])(entity, direction, l_buf, o_buf, buf, 
                                    mStruct, mId);
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
      // acces violation -> possibly wrong ccddata-DLL
    }
  }
  ReleaseMutex (mut[me]);
  return ret;
}

/* ccd_init_ccddata  =========================================================*/
ULONG ccd_init_ccddata (void)
{
  ULONG ret = CCD_DLL_ERROR;
  if (me < 0)
  {
    return ret;
  }
  WaitForSingleObject (mut[me], INFINITE);
  if ((ptr_ccd_init_ccddata[me] != NULL) ||
      (ccddata_ehandler() == CCDDATA_DLL_OK))
  {
    __try
    {
      ret = (*ptr_ccd_init_ccddata[me])();
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
      // acces violation -> possibly wrong ccddata-DLL
    }
  }
  ReleaseMutex (mut[me]);
  return ret;
}

/* ccd_decodeByte  ===========================================================*/
BYTE ccd_decodeByte (UBYTE* bitstream, USHORT startbit,
                     USHORT bitlen, UBYTE* value)
{
  BYTE ret = CCD_DLL_ERROR;
  if (me < 0)
  {
    return ret;
  }
  WaitForSingleObject (mut[me], INFINITE);
  if ((ptr_ccd_decodeByte[me] != NULL) ||
      (ccddata_ehandler() == CCDDATA_DLL_OK))
  {
    __try
    {
      ret = (*ptr_ccd_decodeByte[me])(bitstream, startbit, bitlen, value);
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
      // acces violation -> possibly wrong ccddata-DLL
    }
  }
  ReleaseMutex (mut[me]);
  return ret;
}

/* ccd_codeByte  =============================)===============================*/
BYTE ccd_codeByte (UBYTE* bitstream, USHORT startbit,
              		 USHORT bitlen, UBYTE value)
{
  BYTE ret = CCD_DLL_ERROR;
  if (me < 0)
  {
    return ret;
  }
  WaitForSingleObject (mut[me], INFINITE);
  if ((ptr_ccd_codeByte[me] != NULL) || (ccddata_ehandler() == CCDDATA_DLL_OK))
  {
    __try
    {
      ret = (*ptr_ccd_codeByte[me])(bitstream, startbit, bitlen, value);
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
      // acces violation -> possibly wrong ccddata-DLL
    }
  }
  ReleaseMutex (mut[me]);
  return ret;
}

/* ccd_codeLong  ===)=========================================================*/
BYTE ccd_codeLong (UBYTE* bitstream, USHORT startbit,
	                 USHORT bitlen, ULONG value)
{
  BYTE ret = CCD_DLL_ERROR;
  if (me < 0)
  {
    return ret;
  }
  WaitForSingleObject (mut[me], INFINITE);
  if ((ptr_ccd_codeLong[me] != NULL) || (ccddata_ehandler() == CCDDATA_DLL_OK))
  {
    __try
    {
      ret = (*ptr_ccd_codeLong[me])(bitstream, startbit, bitlen, value);
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
      // acces violation -> possibly wrong ccddata-DLL
    }
  }
  ReleaseMutex (mut[me]);
  return ret;
}

/* ccd_decodeLong  ===========================================================*/
BYTE ccd_decodeLong (UBYTE* bitstream, USHORT startbit,
	                   USHORT bitlen, ULONG *value)
{
  BYTE ret = CCD_DLL_ERROR;
  if (me < 0)
  {
    return ret;
  }
  WaitForSingleObject (mut[me], INFINITE);
  if ((ptr_ccd_decodeLong[me] != NULL) ||
      (ccddata_ehandler() == CCDDATA_DLL_OK))
  {
    __try
    {
      ret = (*ptr_ccd_decodeLong[me])(bitstream, startbit, bitlen, value);
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
      // acces violation -> possibly wrong ccddata-DLL
    }
  }
  ReleaseMutex (mut[me]);
  return ret;
}

/* ccd_bitcopy  ============================)=================================*/
void ccd_bitcopy (UBYTE     *dest,
                  UBYTE     *source,
                  USHORT     bitlen,
                  USHORT     offset)
{
  if (me < 0)
  {
    return;
  }
  WaitForSingleObject (mut[me], INFINITE);
  if ((ptr_ccd_bitcopy[me] != NULL) || (ccddata_ehandler() == CCDDATA_DLL_OK))
  {
    __try
    {
      (*ptr_ccd_bitcopy[me])(dest, source, bitlen, offset);
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
      // acces violation -> possibly wrong ccddata-DLL
    }
  }
  ReleaseMutex (mut[me]);
}

/* ccd_getFirstError  ========================================================*/
UBYTE ccd_getFirstError (UBYTE entity, USHORT *parlist)
{
  UBYTE ret = CCD_DLL_ERROR;
  if (me < 0)
  {
    return ret;
  }
  WaitForSingleObject (mut[me], INFINITE);
  if ((ptr_ccd_getFirstError[me] != NULL) ||
      (ccddata_ehandler() == CCDDATA_DLL_OK))
  {
    __try
    {
      ret = (*ptr_ccd_getFirstError[me])(entity, parlist);
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
      // acces violation -> possibly wrong ccddata-DLL
    }
  }
  ReleaseMutex (mut[me]);
  return ret;
}

/* ccd_getNextError  =========================================================*/
UBYTE ccd_getNextError (UBYTE entity, USHORT *parlist)
{
  UBYTE ret = CCD_DLL_ERROR;
  if (me < 0)
  {
    return ret;
  }
  WaitForSingleObject (mut[me], INFINITE);
  if ((ptr_ccd_getNextError[me] != NULL) ||
      (ccddata_ehandler() == CCDDATA_DLL_OK))
  {
    __try
    {
      ret = (*ptr_ccd_getNextError[me])(entity, parlist);
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
      // acces violation -> possibly wrong ccddata-DLL
    }
  }
  ReleaseMutex (mut[me]);
  return ret;
}

/* ccd_getFirstFault  ========================================================*/
ULONG ccd_getFirstFault (T_CCD_ERR_ENTRY **ccd_err_entry)
{
  ULONG ret = CCD_DLL_ERROR;
  if (me < 0)
  {
    return ret;
  }
  WaitForSingleObject (mut[me], INFINITE);
  if ((ptr_ccd_getFirstFault[me] != NULL) ||
      (ccddata_ehandler() == CCDDATA_DLL_OK))
  {
    __try
    {
      ret = (*ptr_ccd_getFirstFault[me])(ccd_err_entry);
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
      // acces violation -> possibly wrong ccddata-DLL
    }
  }
  ReleaseMutex (mut[me]);
  return ret;
}

/* ccd_getNextFault  =========================================================*/
ULONG ccd_getNextFault (T_CCD_ERR_ENTRY **ccd_err_entry)
{
  ULONG ret = CCD_DLL_ERROR;
  if (me < 0)
  {
    return ret;
  }
  WaitForSingleObject (mut[me], INFINITE);
  if ((ptr_ccd_getNextFault[me] != NULL) ||
      (ccddata_ehandler() == CCDDATA_DLL_OK))
  {
    __try
    {
      ret = (*ptr_ccd_getNextFault[me])(ccd_err_entry);
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
      // acces violation -> possibly wrong ccddata-DLL
    }
  }
  ReleaseMutex (mut[me]);
  return ret;
}

/* ccd_free_faultlist  =======================================================*/
void ccd_free_faultlist (void)
{
  if (me < 0)
  {
    return;
  }
  WaitForSingleObject (mut[me], INFINITE);
  if ((ptr_ccd_free_faultlist[me] != NULL) ||
      (ccddata_ehandler() == CCDDATA_DLL_OK))
  {
    __try
    {
      (*ptr_ccd_free_faultlist[me])();
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
      // acces violation -> possibly wrong ccddata-DLL
    }
  }
  ReleaseMutex (mut[me]);
}

/* ccd_get_numFaults  ========================================================*/
int ccd_get_numFaults ()
{
  int ret = CCD_DLL_ERROR;
  if (me < 0)
  {
    return ret;
  }
  WaitForSingleObject (mut[me], INFINITE);
  if ((ptr_ccd_get_numFaults[me] != NULL) ||
      (ccddata_ehandler() == CCDDATA_DLL_OK))
  {
    __try
    {
      ret = (*ptr_ccd_get_numFaults[me])();
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
      // acces violation -> possibly wrong ccddata-DLL
    }
  }
  ReleaseMutex (mut[me]);
  return ret;
}

/* ccd_encodeElem  ===========================================================*/
int  ccd_encodeElem (ULONG  ccdid,
                     USHORT* l_buf,
                     USHORT  o_buf,
                     UCHAR*  buf,
                     UCHAR*  eStruct)
{
  int ret = CCD_DLL_ERROR;
  if (me < 0)
  {
    return ret;
  }
  WaitForSingleObject (mut[me], INFINITE);
  if ((ptr_ccd_encodeElem[me] != NULL) ||
      (ccddata_ehandler() == CCDDATA_DLL_OK))
  {
    __try
    {
      ret = (*ptr_ccd_encodeElem[me])(ccdid, l_buf, o_buf, buf, eStruct);
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
      // acces violation -> possibly wrong ccddata-DLL
    }
  }
  ReleaseMutex (mut[me]);
  return ret;
}

/* ccd_decodeElem  ===========================================================*/
int  ccd_decodeElem (ULONG  ccdid,
                     USHORT  l_buf,
                     USHORT  o_buf,
                     UCHAR*  buf,
                     UCHAR*  eStruct)
{
  int ret = CCD_DLL_ERROR;
  if (me < 0)
  {
    return ret;
  }
  WaitForSingleObject (mut[me], INFINITE);
  if ((ptr_ccd_decodeElem[me] != NULL) ||
      (ccddata_ehandler() == CCDDATA_DLL_OK))
  {
    __try
    {
      ret = (*ptr_ccd_decodeElem[me])(ccdid, l_buf, o_buf, buf, eStruct);
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
      // acces violation -> possibly wrong ccddata-DLL
    }
  }
  ReleaseMutex (mut[me]);
  return ret;
}

/* pdi_createDefContext  =====================================================*/
T_PDI_CONTEXT* pdi_createDefContext()
{
  T_PDI_CONTEXT* ret = NULL;
  if (me < 0)
  {
    return ret;
  }
  WaitForSingleObject (mut[me], INFINITE);
  if ((ptr_pdi_createDefContext[me] != NULL) ||
      (ccddata_ehandler() == CCDDATA_DLL_OK))
  {
    __try
    {
      ret = (*ptr_pdi_createDefContext[me])();
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
      // acces violation -> possibly wrong ccddata-DLL
    }
  }
  ReleaseMutex (mut[me]);
  return ret;
}

/* pdi_createContext  ========================================================*/
T_PDI_CONTEXT* pdi_createContext(const T_PDI_DECODEINFO* dinfo,
                                 unsigned int dicount)
{
  T_PDI_CONTEXT* ret = NULL;
  if (me < 0)
  {
    return ret;
  }
  WaitForSingleObject (mut[me], INFINITE);
  if ((ptr_pdi_createContext[me] != NULL) ||
      (ccddata_ehandler() == CCDDATA_DLL_OK))
  {
    __try
    {
      ret = (*ptr_pdi_createContext[me])(dinfo, dicount);
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
      // acces violation -> possibly wrong ccddata-DLL
    }
  }
  ReleaseMutex (mut[me]);
  return ret;
}

/* pdi_destroyContext  =======================================================*/
void pdi_destroyContext(T_PDI_CONTEXT *context)
{
  if (me < 0)
  {
    return;
  }
  WaitForSingleObject (mut[me], INFINITE);
  if ((ptr_pdi_destroyContext[me] != NULL) ||
      (ccddata_ehandler() == CCDDATA_DLL_OK))
  {
    __try
    {
      (*ptr_pdi_destroyContext[me])(context);
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
      // acces violation -> possibly wrong ccddata-DLL
    }
  }
  ReleaseMutex (mut[me]);
}

/* pdi_startPrim  ============================================================*/
void pdi_startPrim(T_PDI_CONTEXT *context, ULONG opc)
{
  if (me < 0)
  {
    return;
  }
  WaitForSingleObject (mut[me], INFINITE);
  if ((ptr_pdi_startPrim[me] != NULL) ||
      (ccddata_ehandler() == CCDDATA_DLL_OK))
  {
    __try
    {
      (*ptr_pdi_startPrim[me])(context, opc);
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
      // acces violation -> possibly wrong ccddata-DLL
    }
  }
  ReleaseMutex (mut[me]);
}

/* pdi_getDecodeInfo  ========================================================*/
void pdi_getDecodeInfo(T_PDI_CONTEXT *context, const char *ename,
                       char *evalue, int evlen, T_PDI *decinfo)
{
  if (me < 0)
  {
    return;
  }
  WaitForSingleObject (mut[me], INFINITE);
  if ((ptr_pdi_getDecodeInfo[me] != NULL) ||
      (ccddata_ehandler() == CCDDATA_DLL_OK))
  {
    __try
    {
      (*ptr_pdi_getDecodeInfo[me])(context, ename, evalue, evlen, decinfo);
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
      // acces violation -> possibly wrong ccddata-DLL
    }
  }
  ReleaseMutex (mut[me]);
}

/* pdi_getEntityByPD  ========================================================*/
short pdi_getEntityByPD(T_PDI_CONTEXT *context, unsigned char pd)
{
  short ret = PDI_DLL_ERROR;
  if (me < 0)
  {
    return ret;
  }
  WaitForSingleObject (mut[me], INFINITE);
  if ((ptr_pdi_getEntityByPD[me] != NULL) ||
      (ccddata_ehandler() == CCDDATA_DLL_OK))
  {
    __try
    {
      ret = (*ptr_pdi_getEntityByPD[me])(context, pd);
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
      // acces violation -> possibly wrong ccddata-DLL
    }
  }
  ReleaseMutex (mut[me]);
  return ret;
}

/* pdi_pd2name  ==============================================================*/
const char* pdi_pd2name(unsigned char pd)
{
  const char* ret = NULL;
  if (me < 0)
  {
    return ret;
  }
  WaitForSingleObject (mut[me], INFINITE);
  if ((ptr_pdi_pd2name[me] != NULL) || (ccddata_ehandler() == CCDDATA_DLL_OK))
  {
    __try
    {
      ret = (*ptr_pdi_pd2name[me])(pd);
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
      // acces violation -> possibly wrong ccddata-DLL
    }
  }
  ReleaseMutex (mut[me]);
  return ret;
}

/*
+------------------------------------------------------------------------------
|  Function     :  ccddata_dllname
+------------------------------------------------------------------------------
|  Description  :  Deliver name of currently loaded dll
|
|  Parameters   :  -
|
|  Return       :  lastdll
+------------------------------------------------------------------------------
*/
char* ccddata_dllname (void)
{
  return lastdll;
}

/*
+------------------------------------------------------------------------------
|  Function     :  ccddata_exit
+------------------------------------------------------------------------------
|  Description  :  Unload ccddata dll and clean pointer to the functions
|
|  Parameters   :  -
|
|  Return       :  0 on success, otherwise error code
+------------------------------------------------------------------------------
*/

int ccddata_exit (void)
{
  if (me < 0)
  {
    return CCDDATA_DLL_REGISTER;
  }

  ccddata_detach (me, 1);

  CloseHandle (mut[me]);
  CloseHandle (initlock);
  ptrindex[me] = 0;
  me = -1;

  return CCDDATA_DLL_OK;
}

/*
+------------------------------------------------------------------------------
|  Function     :  ccddata_init
+------------------------------------------------------------------------------
|  Description  :  Load ccddata dll and initialize pointer to the functions
|
|  Parameters   :  dllname - name of the ccddata dll or NULL
|                  reload  - if set, the ccddata dll will be switched
|
|  Return       :  0 on success, otherwise error code
+------------------------------------------------------------------------------
*/

int ccddata_init (const char* dllname,
                  int reload,
                  void (*ccddata_init_notify)(void),
                  void (*ccddata_exit_notify)(void))
{
  int ret = CCDDATA_DLL_OK;

  if (cdll[me] && !reload)
  {
    return CCDDATA_DLL_ALREADY;
  }

  if ((initlock = ccddata_init_lock ()) == 0)
  {
    return CCDDATA_DLL_REGISTER;
  }

  if (ccddata_register () < 0)
  {
    ret = CCDDATA_DLL_REGISTER;
  }
  else
  {
    ccddata_i_notify = ccddata_init_notify;
    ccddata_x_notify = ccddata_exit_notify;

    if (dllname)
    {
      /* build full path of DLL */
      char name[MAX_PATH+1];
      LPTSTR filepart;
      DWORD len=SearchPath(
        NULL,      // pointer to search path
        dllname,  // pointer to filename
        NULL, // pointer to extension
        MAX_PATH, // size, in characters, of buffer
        name,     // pointer to buffer for found filename
        &filepart   // pointer to pointer to file component
      );
      if (!len)
      {
        strcpy(name, dllname);
      }
      
      if (!lastdll[0])
      {
        // no DLL has been loaded yet
        reload=1;
      }
      else if (strcmp (lastdll, name)==0)
      {
        // the correct DLL is already in use
        reload=0;
      }

      if (reload)
      {
        // we want to load a new DLL but save the current one ... just in case
        char old_dll[MAX_PATH]="";
        strcpy (old_dll, lastdll);
        strcpy (lastdll, name);
        (void) ccddata_detach (me, 1);
        ret = ccddata_attach (me);
        if (ret == CCDDATA_DLL_OK)
        {
          // now inform the other applications
          (void)ccddata_clean ();
        }
        else
        {
          // upps, didn't work
          strcpy (lastdll, old_dll);
          (void) ccddata_attach (me);
        }
      }
      else
      {
        // we just attach to the already loaded DLL
        ret = ccddata_attach (me);
      }

    }
  }

  ReleaseMutex (initlock);

  return ret;
}
/*==== END OF FILE ===========================================================*/
