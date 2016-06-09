/**
 * @file	rvm_gen.h
 * 
 * Defines the generic types used by the Riviera Environment 
 * and the exported function of the RVM block.
 *
 * @author	David Lamy-Charrier (d-lamy@ti.com)
 * @version	0.1
 */

/*
 * Revision History:																			
 *
 *	Date       	Author					Modification
 *	-------------------------------------------------------------------
 *	19/01/2000	David Lamy-Charrier		Create.
 *	11/20/2001	Vincent Oberle			Added BUILD_VERSION_NUMBER.
 *										Documentation cleaning
 *																			
 * (C) Copyright 2000 by Texas Instruments Incorporated, All Rights Reserved
 */

#ifndef __RVM_GEN_H_
#define __RVM_GEN_H_

#if 0 //#ifndef _WINDOWS
  #include "config/rv.cfg"
#endif

#include "../rvf/rvf_api.h" /* for memory bank related definitions & GD*/
#include "rvm_api.h"


#ifdef __cplusplus
extern "C" {
#endif


/* RVM global definitions */
#define	T_RVM_STRING			char *
#define	T_RVM_ERROR_TYPE		UINT32

#define RVM_MAX_NB_MEM_BK		(3)

#define RVM_MAX_NB_LINKED_SWE		(10)

#define RVM_USED_MAILBOX		RVF_TASK_MBOX_0


/* define the 4 SWE types */
typedef enum 
{	
	RVM_SWE_TYPE_1,
	RVM_SWE_TYPE_2,
	RVM_SWE_TYPE_3,
	RVM_SWE_TYPE_4
} T_RVM_SWE_TYPE;


/* Memory bank parameters */
typedef struct 
{
	T_RVF_MB_NAME		bank_name;
	T_RVF_MB_PARAM		initial_params;
} T_RVM_BK_INFO;


/* RVM callback function pointer type */
typedef  T_RVM_RETURN (*T_RVM_CB_FUNC)(T_RVM_NAME swe_name,
									T_RVM_RETURN error_cause,
									T_RVM_ERROR_TYPE error_type,
									T_RVM_STRING error_msg);

/*
 * Macro used to build the version number from the
 * - major version number (8 bits)
 * - minor version number (8 bits)
 * - build number (16 bits). A 0 indicates this number is not used.
 */
#define BUILD_VERSION_NUMBER(major, minor, build) ( ((major & 0xFF) << 24) | \
													((minor & 0xFF) << 16) | \
													 (build & 0xFFFF) )

/* SWE core function prototype */
typedef  T_RVM_RETURN (*T_RVM_SWE_CORE_FUNC)(void);

/* Type1 SWE info */
typedef struct
{
	T_RVM_NAME			swe_name;
	T_RVM_USE_ID			swe_use_id;
	UINT32				version;
	UINT8				nb_mem_bank;
	T_RVM_BK_INFO			mem_bank[RVM_MAX_NB_MEM_BK];
	UINT8				nb_linked_swe;
	T_RVM_USE_ID			linked_swe_id[RVM_MAX_NB_LINKED_SWE];
	T_RV_RETURN_PATH		return_path;
	T_RVM_RETURN  (* set_info) (T_RVF_ADDR_ID	addr_id,
								T_RV_RETURN_PATH return_path[],
								T_RVF_MB_ID		bk_id_table[],
								T_RVM_CB_FUNC	call_back_error_ft);
	T_RVM_RETURN  (* init)  (void);
	T_RVM_RETURN  (* start) (void);
	T_RVM_RETURN  (* stop)	(/*T_RV_HDR* hdr*/void); //???
	T_RVM_RETURN  (* kill)	(void);
} T_RVM_SWE_PASSIVE_INFO;

/* Type2 SWE info */
typedef struct
{
	T_RVM_NAME			swe_name;
	T_RVM_USE_ID			swe_use_id;
	UINT16				stack_size;
	UINT8				priority;
	UINT32				version;
	UINT8				nb_mem_bank;
	T_RVM_BK_INFO			mem_bank[RVM_MAX_NB_MEM_BK];
	UINT8				nb_linked_swe;
	T_RVM_USE_ID			linked_swe_id[RVM_MAX_NB_LINKED_SWE];
	T_RV_RETURN_PATH		return_path;
	T_RVF_GD_ID			swe_group_directive;				/* A-M-E-N-D-E-D! */
	T_RVM_RETURN  (* set_info) (T_RVF_ADDR_ID	addr_id,
								T_RV_RETURN_PATH return_path[],
								T_RVF_MB_ID		bk_id_table[],  /* A-M-E-N-E-D-E-D! is table or list??*/
								T_RVM_CB_FUNC	call_back_error_ft);
	T_RVM_RETURN  (* init)  (void);
	T_RVM_RETURN  (* start) (void);
	T_RVM_RETURN  (* handle_message)	(T_RV_HDR * msg);
	T_RVM_RETURN  (* handle_timer)		(T_RV_HDR * msg);
	T_RVM_RETURN  (* stop)	(T_RV_HDR* hdr);
	T_RVM_RETURN  (* kill)	(void);
} T_RVM_SWE_GROUP_MEMBER_INFO;

/* Type3 SWE info */
typedef struct
{
	T_RVM_NAME			swe_name;
	T_RVM_USE_ID			swe_use_id;
	UINT16				stack_size;
	UINT8				priority;
	UINT32				version;
	UINT8				nb_mem_bank;
	T_RVM_BK_INFO			mem_bank[RVM_MAX_NB_MEM_BK];
	UINT8				nb_linked_swe;
	T_RVM_USE_ID			linked_swe_id[RVM_MAX_NB_LINKED_SWE];
	T_RV_RETURN_PATH		return_path;
	T_RVM_RETURN  (* set_info) (T_RVF_ADDR_ID	addr_id,
								T_RV_RETURN_PATH return_path[],
								T_RVF_MB_ID		bk_id_table[],
								T_RVM_CB_FUNC	call_back_error_ft);
	T_RVM_RETURN  (* init)  (void);
	T_RVM_RETURN  (* start) (void);
	T_RVM_RETURN  (* handle_message)	(T_RV_HDR * msg);
	T_RVM_RETURN  (* handle_timer)		(T_RV_HDR * msg);
	T_RVM_RETURN  (* stop)	(T_RV_HDR* hdr);
	T_RVM_RETURN  (* kill)	(void);
} T_RVM_SWE_SINGLE_INFO;

/* Type4 SWE info */
typedef struct
{
	T_RVM_NAME			swe_name;
	T_RVM_USE_ID			swe_use_id;
	UINT16				stack_size;
	UINT8				priority;
	UINT32				version;
	UINT8				nb_mem_bank;
	T_RVM_BK_INFO			mem_bank[RVM_MAX_NB_MEM_BK];
	UINT8				nb_linked_swe;
	T_RVM_USE_ID			linked_swe_id[RVM_MAX_NB_LINKED_SWE];
	T_RV_RETURN_PATH		return_path;
	T_RVM_RETURN  (* set_info) (T_RVF_ADDR_ID	addr_id,
								T_RV_RETURN_PATH return_path[],
								T_RVF_MB_ID		bk_id_table[],
								T_RVM_CB_FUNC	call_back_error_ft);
	T_RVM_RETURN  (* init)  (void);
	T_RVM_RETURN  (* core)	(void);
	T_RVM_RETURN  (* stop)	(/*T_RV_HDR* hdr*/void); // could be removed completely except for legacy code
	T_RVM_RETURN  (* kill)	(void);
} T_RVM_SWE_SELF_MADE_INFO;


/* SWE information */
typedef struct 
{	
	T_RVM_SWE_TYPE			swe_type;

	union
	{	T_RVM_SWE_PASSIVE_INFO		type1;
		T_RVM_SWE_GROUP_MEMBER_INFO	type2;
		T_RVM_SWE_SINGLE_INFO		type3;
		T_RVM_SWE_SELF_MADE_INFO	type4;
	} type_info;
} T_RVM_INFO_SWE;


/* get info function pointer type */
typedef T_RVM_RETURN (*T_RVM_GET_INFO_FUNC) (T_RVM_INFO_SWE * param);


/* type used in the const SWEs array */
typedef struct t_rvm_const_swe_info
{
	T_RVM_USE_ID			use_id;
	T_RVM_GET_INFO_FUNC		get_info_func;
} T_RVM_CONST_SWE_INFO;


#ifdef __cplusplus
}
#endif


#endif /* __RVM_GEN_H_ */
