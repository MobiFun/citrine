/* 
+-------------------------------------------------------------------------------
|  Project :  
|  Modul   :  Ccdedit.h
+-------------------------------------------------------------------------------
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
+-------------------------------------------------------------------------------
|  Purpose :  Coder Decoder Edit functions 
+-------------------------------------------------------------------------------
*/ 

#ifndef __CCDEDIT_H__
#define __CCDEDIT_H__

#ifdef __cplusplus
extern  "C" {
#endif /*_cplusplus*/

/* 
 * The handle structure for primitives/messages
 */
/*
 * level of substructured elements in substructures
 */
#define MAX_LEVELS 20

typedef enum
{
  TRAVERSE_ARRAY,
  TRAVERSE_STRUCTURE
} T_CTX_STATE;

typedef enum
{
  NoArray,
  FixArray,
  VarArray
} T_ATYPE;

typedef enum
{
  FromMsg, FromPrim
} T_ELM_SRC;

typedef struct
{
  USHORT      structIdx;
  USHORT      elemIdx;
  UBYTE       elemType;   
  T_CTX_STATE state;
  USHORT      arrayIndex;
  USHORT      repeats;                    /* valid repeats          */
  USHORT      numElems;
  U32         levelOffset;
  T_ATYPE     arrayType;                  /* elem is an fix or      */
                                          /* variable or no array   */
} T_CCDE_CONTEXT;

typedef struct
{
  UBYTE            level;
  BOOL             canDescent;
  T_ELM_SRC        source;
  U32              maxCSize;
  USHORT           lenVarPart;      /* length of the variable
                                       part e.g. a SDU        */
  T_CCDE_CONTEXT   context[MAX_LEVELS];
} T_CCDE_HANDLE;

typedef enum
{
  noptr,
  usptr,
  ctptr
} T_PTRTYPE;

/*
 * The type of the C-structure component
 */
typedef enum
{
  T_none,
  T_byte,
  T_short,
  T_long,
  T_buffer,
  T_struct,
  T_union,
  T_issdu,
  T_ductrl
} T_BTYPE;

#define NO_REF              0xffff

#define SHORT_NAME_LENGTH   256
#define ALIAS_NAME_LENGTH   256
#define LONG_NAME_LENGTH    100
#define SYMBOLIC_VAL_LENGTH 150


typedef struct
{
  T_ELM_SRC esource;                  /* source of element(MSG/PRIM)*/
  T_BTYPE   btype;                        /* C-type of element      */
  T_ATYPE   arrayType;                    /* elem is an fix or      */
  T_PTRTYPE ptrtype;                      /* pointer type */
  BOOL      isOptional;
  BOOL      isValid;
  U32       bytelen;                      /* length in byte         */
  U32       offset;                       /* byteoffs in C-Structure*/
                                          /* variable or no array   */
  USHORT    maxRepeat;                    /* max repeats if isArray */
  USHORT    validRepeats;                 /* valid repeats          */
  USHORT    index;                        /* act. idx in array      */
  USHORT    ccdIndex;
  UBYTE     level;                        /* level of descending    */
  BOOL      u_member;
  U32       u_ctrl;                       /* union tag */
  USHORT    elemref;                      /* elemref from [pm]elem.cdg */
  USHORT    bitstring;                    /* it is an ANS1_BITSTRINGS */
  USHORT    issigned;                     /* it is a signed variable */
  USHORT    c_implicit;                   /* counter c_xxx is generated */
  char      sname[SHORT_NAME_LENGTH+1];
  char      aname[ALIAS_NAME_LENGTH+1];
  char      lname[LONG_NAME_LENGTH+1];
  char      symbolicValue[SYMBOLIC_VAL_LENGTH+1];
  int       valcheck;                     /* indicates if value was in range */
} T_CCDE_ELEM_DESCR;

/*
 * Definitions of returnvalues of the ccd_xxxx functions.
 */

#define CCDEDIT_OK                0
#define CCDEDIT_PRIM_NOT_FOUND    1
#define CCDEDIT_END_OF_PRIM       2
#define CCDEDIT_PRIMITIVE_ERROR   3
#define CCDEDIT_MESSAGE_NOT_FOUND 1
#define CCDEDIT_END_OF_MSG        2
#define CCDEDIT_MESSAGE_ERROR     3
#define CCDEDIT_COMP_NOT_FOUND    1
#define CCDEDIT_END_OF_COMP       2
#define CCDEDIT_COMP_ERROR        3
#define CCDEDIT_UTAG_ERROR        1
#define CCDEDIT_DLL_ERROR        10
#define CCDEDIT_ERROR            -2

/* and for the former tdc functions */
#define NO_ENTRY_FOUND  0xffffffff

#if !defined (CCDDATA_PREF)
#if defined (_TOOLS_) && defined (CCDDATA_LOAD)
#define CCDDATA_PREF(cde_fun) cddl_##cde_fun
#else
#define CCDDATA_PREF(cde_fun) cde_fun
#endif /* _TOOLS_ && CCDDATA_LOAD */
#endif /* !CCDDATA_PREF */

#ifndef CCDEDIT_C

/*
 * Initialize the internal data of ccdedit. Must be called before any
 * of the other functions.
 */
extern void CCDDATA_PREF(cde_init) (void);

/*
 * This function works with similar results like cde_comp_first,
 * but not the whole comp table is searched for the name of the
 * component. Instead the previous set elemref in the
 * parameter edescr is taken to directly jump to the component.
 * The component found is compared with the given name in
 * edescr. If equal chandle is defined. Otherwise there is an error.
 */
extern USHORT CCDDATA_PREF(cde_get_comp) (T_CCDE_HANDLE*     chandle,
                                          T_CCDE_ELEM_DESCR* edescr);

/*
 * Selects the primitive for edit processing. The code of the
 * primitive (primcode) must be passed to the function.
 * The function returns a State and the primname in (*name)
 * and a handle for this edit process (phandle).
 * After a successful call the component maxCSize of the
 * phandle contains the size of the C-Structure for this primitive. 
 */
extern USHORT CCDDATA_PREF(cde_prim_first)  (T_CCDE_HANDLE      * phandle,
                                             ULONG               primcode,
                                             char               * name);
/*
 * Get the next element of the selected primitive. All informations
 * of this element is stored into the element descriptor (pdescr).
 */
extern USHORT CCDDATA_PREF(cde_prim_next)   (T_CCDE_HANDLE      * phandle,
                                             UBYTE                descent,
                                             T_CCDE_ELEM_DESCR  * pdescr);


/*
 * Selects the message for edit processing. The message type (type),
 * the (entity) and the (direction) must be passed to this function.
 * The function returns a State and the messagename in (*name)
 * and a handle for this edit process (mhandle).
 * After a successful call the component maxCSize of the
 * mhandle contains the size of the C-Structure for this message. 
 */
extern USHORT CCDDATA_PREF(cde_msg_first)   (T_CCDE_HANDLE      * mhandle,
                                             UBYTE                type,
                                             UBYTE                direction,
                                             UBYTE                entity,
			                                       char               * name);


/*
 * Get the next element of the selected primitive. All informations
 * of this element is stored into the element descriptor (iedescr).
 */
extern USHORT CCDDATA_PREF(cde_msg_next)    (T_CCDE_HANDLE      * mhandle,
                                             UBYTE                descent,
                                             T_CCDE_ELEM_DESCR  * iedescr);

/*
 * Selects at COMPOSITION (structure) for edit processing.
 * The short name (compname) of this composition must be passed
 * to this function.
 * The function returns a State and ahandle for this
 * edit process (chandle).
 * This function may be used for sub-structures (compositions)
 * of primitives and messages.
 * After a successful call the component maxCSize of the
 * chandle contains the size of the C-Structure for this composition. 
 */
extern USHORT CCDDATA_PREF(cde_comp_first)  (T_CCDE_HANDLE      * chandle,
                                             T_ELM_SRC            source,
                                             char               * compname);

/*
 * Get the next element of the selected composition. All informations
 * of this element is stored into the element descriptor (cdescr).
 */
extern USHORT CCDDATA_PREF(cde_comp_next)   (T_CCDE_HANDLE      * chandle,
                                             UBYTE                descent,
                                             T_CCDE_ELEM_DESCR  * descr);

/*
 * Add the "Comment" of the SAP-/MSG-catalogues to the member symbolicValue
 * of a given T_CCDE_ELEM_DESCR.
 * This functionality was part of cde_read_elem and is now extracted
 * to a dedicated function.
 */
extern char* CCDDATA_PREF(cde_get_symval)   (int                elem_value,
                                             T_CCDE_ELEM_DESCR* edescr);

/*
 * Read the value of the element out of the C-Structure (cstruct)
 * which containes a primitive or a decoded message. The infomations
 * of the element is stored in the input parameter edescr, wich is
 * previously assigned by a cde_xxxx_next () call. The value of this
 * element is stored in the memory area addressed by (*value). 
 * After this call, the component symbolicValue of the edescr-struct
 * is updated with a symbolic value string, (if any defined).
 */
extern USHORT CCDDATA_PREF(cde_read_elem)   (T_CCDE_HANDLE      * handle,
                                             void               * cstruct,
                                             T_CCDE_ELEM_DESCR  * edescr,
                                             UBYTE              * value);


/*
 * prepares the writing of elements, by setting valid flag,
 * union controller and length of vaiable arrays if necessary.
 * First version: union controller only.
 */
extern void CCDDATA_PREF(cde_write_prepare) (T_CCDE_HANDLE     * handle,
                                       void              * cstruct,
                                       T_CCDE_ELEM_DESCR * edescr);

/*
 * Store the value from the memory area which is addressed by (*value)
 * into the corrensponding position into the C-Structure (cstruct)
 * which containes a primitive or a decoded message. The infomations
 * of the element is stored in the input parameter edescr, wich is
 * previously assigned by a cde_xxxx_next () call.
 */
extern USHORT CCDDATA_PREF(cde_write_elem)  (T_CCDE_HANDLE      * handle,
                                             void               * cstruct,
                                             T_CCDE_ELEM_DESCR  * edescr,
                                             UBYTE              * value);


/*
 * This function requests if the given name is the name of
 * a primitive or of a message. If the function returns
 * CCDEDIT_OK the return parameter source is set to FromPrim
 * or to FromMsg. Otherwise CCDEDIT_PRIM_NOT_FOUND/CCD_EDIT_MESSAGE_NOT_FOUND
 * is returned.
 */
extern USHORT CCDDATA_PREF(cde_get_type)    (char               * name,
                                             T_ELM_SRC          * source);

/*
 * This function requests the opcode of a primitive. The 
 * name of the primitive is given by the input paramater name.
 * If the function returns CCDEDIT_OK the parameter primcode
 * contains the 16 bit opcode. Otherwise CCDEDIT_PRIM_NOT_FOUND
 * is returned.
 */ 
extern USHORT CCDDATA_PREF(cde_get_primcode) (char              * name,
                                              ULONG             * primcode);



/*
 * This function requests the opcode, the direction and the
 * entity index of a message. The name of the message is given
 * by the input paramater name.
 * If the function returns CCDEDIT_OK the return parameters types,
 * direction and entity contains the right values.
 * Otherwise CCDEDIT_MESSAGE_NOT_FOUND is returned.
 */ 
extern USHORT CCDDATA_PREF(cde_get_msgcode) (char               * name,
                                             UBYTE              * type,
                                             UBYTE              * direction,
                                             UBYTE              * entity);

/*
 * This function finds out if an AIM is a downlink or uplink.
 */
extern int CCDDATA_PREF(cde_get_is_downlink) (ULONG comp_index);

/*
 * This function searches the comp index in either pcomp or mcomp
 */
extern ULONG CCDDATA_PREF(cde_get_comp_index) (CHAR* comp_name,
                                               T_ELM_SRC table);

/*
 * This function gets the element name for a given index + offset.
 */
extern CHAR* CCDDATA_PREF(cde_get_element_name) (ULONG comp_index,
                                                 USHORT elem_off,
                                                 T_ELM_SRC table);

/*
 * This function gets the array kind - e.g. the cSize of the 
 * arrays (byte, short og long).
 */
extern ULONG CCDDATA_PREF(cde_get_array_kind) (CHAR* var_name, T_ELM_SRC table);

#endif /* !CCDEDIT_C */

#ifdef __cplusplus
}
#endif /*_cplusplus*/

#endif
