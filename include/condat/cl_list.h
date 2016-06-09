/* 
+----------------------------------------------------------------------------- 
|  Project :  COMLIB
|  Modul   :  RR/PL
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
|  Purpose :  Definitions of global types used by List Processing functions
|             and the prototypes of those functions: RR/PL layer.
+----------------------------------------------------------------------------- 
*/ 

#ifndef CL_LIST_H
#define CL_LIST_H

/*==== CONST ================================================================*/

#define BITOFFSET_LIST          1024

#define MAX_BYTES_900           16
#define MAX_BYTES_EGSM          23
#define MAX_BYTES_1800          47
#define MAX_BYTES_1900          38
#define MAX_BYTES_DUAL          63
#define MAX_BYTES_850           16
#define MAX_BYTES_DUAL_EGSM     70
#define MAX_BYTES_DUAL_US       54

#define SET_CHANNEL_BIT     0
#define RESET_CHANNEL_BIT   1
#define GET_CHANNEL_BIT     2
#define CHECK_CHANNEL       3


#define T_LIST_MAX_SIZE 128 /* 1024/8 = 128 */
typedef struct
{
  UBYTE                 channels [T_LIST_MAX_SIZE];
} T_LIST;



/*==== MACROS ================================================================*/

#define srv_set_channel(list,ch)    scr_channel_bit(list,ch,SET_CHANNEL_BIT)
#define srv_unset_channel(list,ch)  scr_channel_bit(list,ch,RESET_CHANNEL_BIT)
#define srv_get_channel(list,ch)    scr_channel_bit(list,ch,GET_CHANNEL_BIT)

/*==== VARS =================================================================*/

/*==== TYPES =================================================================*/

/*==== FUNCTIONS ============================================================*/

EXTERN UBYTE  scr_channel_bit             (T_LIST              *list,
                                           int                  channel,
                                           int                  mode);
EXTERN int    srv_create_list             (T_LIST              *list,
                                           USHORT              *channel_array,
                                           USHORT              size,
                                           UBYTE               zero_at_start,
                                           USHORT              start_index);
EXTERN void   srv_clear_list              (T_LIST              *list);
EXTERN void   srv_copy_list               (T_LIST              *target_list,
                                           T_LIST              *source_list,
                                           UBYTE               size);
EXTERN UBYTE  srv_compare_list            (T_LIST              *list1,
                                           T_LIST              *list2);
EXTERN void   srv_merge_list              (T_LIST              *target_list,
                                           T_LIST              *list);
EXTERN void   srv_unmask_list             (T_LIST *target,T_LIST *source);
EXTERN void   srv_trace_freq_in_list      (T_LIST *list);
EXTERN U8     srv_get_region_from_std     (U8 std);
EXTERN U16    srv_count_list              (T_LIST *list);
extern BOOL   srv_is_list_set             (T_LIST *list);

#endif /* #ifndef CL_LIST_H */


