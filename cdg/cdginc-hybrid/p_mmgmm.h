/*
+--------------------------------------------------------------------------+
| PROJECT : PROTOCOL STACK                                                 |
| FILE    : p_mmgmm.h                                                      |
| SOURCE  : "sap\mmgmm.pdf"                                                |
| LastModified : "2002-08-09"                                              |
| IdAndVersion : "8441.114.99.021"                                         |
| SrcFileTime  : "Thu Nov 29 09:46:12 2007"                                |
| Generated by CCDGEN_2.5.5A on Fri Oct 14 21:41:52 2016                   |
|           !!DO NOT MODIFY!!DO NOT MODIFY!!DO NOT MODIFY!!                |
+--------------------------------------------------------------------------+
*/

/* PRAGMAS
 * PREFIX                 : NONE
 * COMPATIBILITY_DEFINES  : NO (require PREFIX)
 * ALWAYS_ENUM_IN_VAL_FILE: NO
 * ENABLE_GROUP: NO
 * CAPITALIZE_TYPENAME: NO
 */


#ifndef P_MMGMM_H
#define P_MMGMM_H


#define CDG_ENTER__P_MMGMM_H

#define CDG_ENTER__FILENAME _P_MMGMM_H
#define CDG_ENTER__P_MMGMM_H__FILE_TYPE CDGINC
#define CDG_ENTER__P_MMGMM_H__LAST_MODIFIED _2002_08_09
#define CDG_ENTER__P_MMGMM_H__ID_AND_VERSION _8441_114_99_021

#define CDG_ENTER__P_MMGMM_H__SRC_FILE_TIME _Thu_Nov_29_09_46_12_2007

#include "CDG_ENTER.h"

#undef CDG_ENTER__P_MMGMM_H

#undef CDG_ENTER__FILENAME


#include "p_mmgmm.val"

#ifndef __T_plmn__
#define __T_plmn__
/*
 * plmn identification
 * CCDGEN:WriteStruct_Count==1808
 */
typedef struct
{
  U8                        v_plmn;                   /*<  0:  1> valid flag                                         */
  U8                        mcc[SIZE_MCC];            /*<  1:  3> Mobile country code.                               */
  U8                        mnc[SIZE_MNC];            /*<  4:  3> Mobile network code.                               */
  U8                        _align0;                  /*<  7:  1> alignment                                          */
} T_plmn;
#endif

/*
 * plmn identification
 * CCDGEN:WriteStruct_Count==1809
 */
#ifndef __T_new_forb_plmn__
#define __T_new_forb_plmn__
typedef T_plmn T_new_forb_plmn;
#endif
/*
 * ahplmn identification
 * CCDGEN:WriteStruct_Count==1810
 */
#ifndef __T_ahplmn__
#define __T_ahplmn__
typedef T_plmn T_ahplmn;
#endif
#ifndef __T_full_name__
#define __T_full_name__
/*
 * Network name, long format
 * CCDGEN:WriteStruct_Count==1811
 */
typedef struct
{
  U8                        v_name;                   /*<  0:  1> valid flag                                         */
  U8                        dcs;                      /*<  1:  1> data coding scheme                                 */
  U8                        add_ci;                   /*<  2:  1> add ci indicator                                   */
  U8                        num_spare;                /*<  3:  1> num spare bits                                     */
  U8                        c_text;                   /*<  4:  1> counter                                            */
  U8                        text[MMR_MAX_TEXT_LEN];   /*<  5: 40> name                                               */
  U8                        _align0;                  /*< 45:  1> alignment                                          */
  U8                        _align1;                  /*< 46:  1> alignment                                          */
  U8                        _align2;                  /*< 47:  1> alignment                                          */
} T_full_name;
#endif

/*
 * Network name, short format
 * CCDGEN:WriteStruct_Count==1812
 */
#ifndef __T_short_name__
#define __T_short_name__
typedef T_full_name T_short_name;
#endif
#ifndef __T_ntz__
#define __T_ntz__
/*
 * Network time zone
 * CCDGEN:WriteStruct_Count==1813
 */
typedef struct
{
  U8                        v_tz;                     /*<  0:  1> timezone valid                                     */
  U8                        tz;                       /*<  1:  1> timezone                                           */
  U8                        _align0;                  /*<  2:  1> alignment                                          */
  U8                        _align1;                  /*<  3:  1> alignment                                          */
} T_ntz;
#endif

#ifndef __T_time__
#define __T_time__
/*
 * Network time
 * CCDGEN:WriteStruct_Count==1814
 */
typedef struct
{
  U8                        v_time;                   /*<  0:  1> time valid                                         */
  U8                        year;                     /*<  1:  1> year                                               */
  U8                        month;                    /*<  2:  1> month                                              */
  U8                        day;                      /*<  3:  1> day                                                */
  U8                        hour;                     /*<  4:  1> hour                                               */
  U8                        minute;                   /*<  5:  1> minute                                             */
  U8                        second;                   /*<  6:  1> second                                             */
  U8                        _align0;                  /*<  7:  1> alignment                                          */
} T_time;
#endif

#ifndef __T_equ_plmn__
#define __T_equ_plmn__
/*
 * Equivalent plmn identification
 * CCDGEN:WriteStruct_Count==1815
 */
typedef struct
{
  U8                        mcc[SIZE_MCC];            /*<  0:  3> Mobile country code.                               */
  U8                        c_mnc;                    /*<  3:  1> counter                                            */
  U8                        mnc[SIZE_MNC_MAX];        /*<  4:  3> Mobile network code.                               */
  U8                        _align0;                  /*<  7:  1> alignment                                          */
} T_equ_plmn;
#endif

#ifndef __T_equ_plmn_list__
#define __T_equ_plmn_list__
/*
 * Equivalent plmn List
 * CCDGEN:WriteStruct_Count==1816
 */
typedef struct
{
  U8                        _align0;                  /*<  0:  1> alignment                                          */
  U8                        _align1;                  /*<  1:  1> alignment                                          */
  U8                        _align2;                  /*<  2:  1> alignment                                          */
  U8                        c_equ_plmn;               /*<  3:  1> counter                                            */
  T_equ_plmn                equ_plmn[MAX_EQ_PLMN_ID]; /*<  4: 40> Equivalent plmn identification                     */
} T_equ_plmn_list;
#endif


/*
 * End of substructure section, begin of primitive definition section
 */

#ifndef __T_MMGMM_REG_REQ__
#define __T_MMGMM_REG_REQ__
/*
 * 
 * CCDGEN:WriteStruct_Count==1817
 */
typedef struct
{
  U8                        service_mode;             /*<  0:  1> Required service mode                              */
  U8                        reg_type;                 /*<  1:  1> Reg type                                           */
  U8                        mobile_class;             /*<  2:  1> mobile class                                       */
  U8                        bootup_act;               /*<  3:  1> bootup action                                      */
} T_MMGMM_REG_REQ;
#endif

#ifndef __T_MMGMM_REG_CNF__
#define __T_MMGMM_REG_CNF__
/*
 * 
 * CCDGEN:WriteStruct_Count==1818
 */
typedef struct
{
  T_plmn                    plmn;                     /*<  0:  8> plmn identification                                */
  U16                       lac;                      /*<  8:  2> location area code                                 */
  U16                       cid;                      /*< 10:  2> cell id                                            */
  U8                        resumption;               /*< 12:  1> Resumption ok or not                               */
  U8                        gprs_indicator;           /*< 13:  1> GPRS indicator                                     */
  U8                        bootup_cause;             /*< 14:  1> bootup cause                                       */
  U8                        _align0;                  /*< 15:  1> alignment                                          */
} T_MMGMM_REG_CNF;
#endif

#ifndef __T_MMGMM_REG_REJ__
#define __T_MMGMM_REG_REJ__
/*
 * 
 * CCDGEN:WriteStruct_Count==1819
 */
typedef struct
{
  U8                        service;                  /*<  0:  1> service (either no or limited service)             */
  U8                        search_running;           /*<  1:  1> Search is still running                            */
  U8                        _align0;                  /*<  2:  1> alignment                                          */
  U8                        _align1;                  /*<  3:  1> alignment                                          */
  T_new_forb_plmn           new_forb_plmn;            /*<  4:  8> plmn identification                                */
  U16                       cause;                    /*< 12:  2> MM or GMM error cause                              */
  U8                        resumption;               /*< 14:  1> Resumption ok or not                               */
  U8                        _align2;                  /*< 15:  1> alignment                                          */
} T_MMGMM_REG_REJ;
#endif

#ifndef __T_MMGMM_NREG_IND__
#define __T_MMGMM_NREG_IND__
/*
 * 
 * CCDGEN:WriteStruct_Count==1820
 */
typedef struct
{
  U8                        service;                  /*<  0:  1> service (either no or limited service)             */
  U8                        search_running;           /*<  1:  1> Search is still running                            */
  U8                        _align0;                  /*<  2:  1> alignment                                          */
  U8                        _align1;                  /*<  3:  1> alignment                                          */
  T_new_forb_plmn           new_forb_plmn;            /*<  4:  8> plmn identification                                */
  U16                       cause;                    /*< 12:  2> MM or GMM error cause                              */
  U8                        _align2;                  /*< 14:  1> alignment                                          */
  U8                        _align3;                  /*< 15:  1> alignment                                          */
} T_MMGMM_NREG_IND;
#endif

#ifndef __T_MMGMM_NREG_REQ__
#define __T_MMGMM_NREG_REQ__
/*
 * 
 * CCDGEN:WriteStruct_Count==1821
 */
typedef struct
{
  U8                        detach_cause;             /*<  0:  1> Detach cause                                       */
  U8                        detach_done;              /*<  1:  1> Detach done                                        */
  U16                       cause;                    /*<  2:  2> MM or GMM error cause                              */
} T_MMGMM_NREG_REQ;
#endif

#ifndef __T_MMGMM_NREG_CNF__
#define __T_MMGMM_NREG_CNF__
/*
 * 
 * CCDGEN:WriteStruct_Count==1822
 */
typedef struct
{
  U8                        detach_cause;             /*<  0:  1> Detach cause                                       */
  U8                        _align0;                  /*<  1:  1> alignment                                          */
  U8                        _align1;                  /*<  2:  1> alignment                                          */
  U8                        _align2;                  /*<  3:  1> alignment                                          */
} T_MMGMM_NREG_CNF;
#endif

#ifndef __T_MMGMM_NET_REQ__
#define __T_MMGMM_NET_REQ__
/*
 * 
 * CCDGEN:WriteStruct_Count==1823
 */
typedef struct
{
  U8                        dummy;                    /*<  0:  1> no parameters                                      */
} T_MMGMM_NET_REQ;
#endif

#ifndef __T_MMGMM_PLMN_IND__
#define __T_MMGMM_PLMN_IND__
/*
 * 
 * CCDGEN:WriteStruct_Count==1824
 */
typedef struct
{
  U16                       cause;                    /*<  0:  2> MM or GMM error cause                              */
  U8                        _align0;                  /*<  2:  1> alignment                                          */
  U8                        _align1;                  /*<  3:  1> alignment                                          */
  T_plmn                    plmn[MAX_PLMN_ID];        /*<  4: 96> plmn identification                                */
  U8                        forb_ind[MAX_PLMN_ID];    /*<100: 12> Forbidden PLMN indicator                           */
  U16                       lac_list[MAX_PLMN_ID];    /*<112: 24> LACs of found PLMNs                                */
  U8                        rxlevel[MAX_PLMN_ID];     /*<136: 12> Fieldstrength                                      */
  U8                        gprs_status[MAX_PLMN_ID]; /*<148: 12> GPRS Status                                        */
} T_MMGMM_PLMN_IND;
#endif

#ifndef __T_MMGMM_PLMN_RES__
#define __T_MMGMM_PLMN_RES__
/*
 * 
 * CCDGEN:WriteStruct_Count==1825
 */
typedef struct
{
  T_plmn                    plmn;                     /*<  0:  8> plmn identification                                */
  U8                        reg_type;                 /*<  8:  1> Reg type                                           */
  U8                        mobile_class;             /*<  9:  1> mobile class                                       */
  U8                        _align0;                  /*< 10:  1> alignment                                          */
  U8                        _align1;                  /*< 11:  1> alignment                                          */
} T_MMGMM_PLMN_RES;
#endif

#ifndef __T_MMGMM_PLMN_MODE_REQ__
#define __T_MMGMM_PLMN_MODE_REQ__
/*
 * 
 * CCDGEN:WriteStruct_Count==1826
 */
typedef struct
{
  U8                        mode;                     /*<  0:  1> network selction mode                              */
  U8                        _align0;                  /*<  1:  1> alignment                                          */
  U8                        _align1;                  /*<  2:  1> alignment                                          */
  U8                        _align2;                  /*<  3:  1> alignment                                          */
} T_MMGMM_PLMN_MODE_REQ;
#endif

#ifndef __T_MMGMM_AUTH_REJ_REQ__
#define __T_MMGMM_AUTH_REJ_REQ__
/*
 * 
 * CCDGEN:WriteStruct_Count==1827
 */
typedef struct
{
  U8                        dummy;                    /*<  0:  1> no parameters                                      */
} T_MMGMM_AUTH_REJ_REQ;
#endif

#ifndef __T_MMGMM_AUTH_REJ_IND__
#define __T_MMGMM_AUTH_REJ_IND__
/*
 * 
 * CCDGEN:WriteStruct_Count==1828
 */
typedef struct
{
  U8                        dummy;                    /*<  0:  1> no parameters                                      */
} T_MMGMM_AUTH_REJ_IND;
#endif

#ifndef __T_MMGMM_CM_ESTABLISH_IND__
#define __T_MMGMM_CM_ESTABLISH_IND__
/*
 * 
 * CCDGEN:WriteStruct_Count==1829
 */
typedef struct
{
  U8                        dummy;                    /*<  0:  1> no parameters                                      */
} T_MMGMM_CM_ESTABLISH_IND;
#endif

#ifndef __T_MMGMM_CM_ESTABLISH_RES__
#define __T_MMGMM_CM_ESTABLISH_RES__
/*
 * 
 * CCDGEN:WriteStruct_Count==1830
 */
typedef struct
{
  U8                        cm_establish_res;         /*<  0:  1> cm establish response                              */
  U8                        _align0;                  /*<  1:  1> alignment                                          */
  U8                        _align1;                  /*<  2:  1> alignment                                          */
  U8                        _align2;                  /*<  3:  1> alignment                                          */
} T_MMGMM_CM_ESTABLISH_RES;
#endif

#ifndef __T_MMGMM_CM_RELEASE_IND__
#define __T_MMGMM_CM_RELEASE_IND__
/*
 * 
 * CCDGEN:WriteStruct_Count==1831
 */
typedef struct
{
  U8                        resumption;               /*<  0:  1> Resumption ok or not                               */
  U8                        _align0;                  /*<  1:  1> alignment                                          */
  U8                        _align1;                  /*<  2:  1> alignment                                          */
  U8                        _align2;                  /*<  3:  1> alignment                                          */
} T_MMGMM_CM_RELEASE_IND;
#endif

#ifndef __T_MMGMM_ACTIVATE_IND__
#define __T_MMGMM_ACTIVATE_IND__
/*
 * 
 * CCDGEN:WriteStruct_Count==1832
 */
typedef struct
{
  T_plmn                    plmn;                     /*<  0:  8> plmn identification                                */
  U16                       lac;                      /*<  8:  2> location area code                                 */
  U16                       cid;                      /*< 10:  2> cell id                                            */
  U32                       t3212_val;                /*< 12:  4> value of T3212                                     */
  U8                        status;                   /*< 16:  1> Activation status                                  */
  U8                        gprs_indicator;           /*< 17:  1> GPRS indicator                                     */
  U8                        _align0;                  /*< 18:  1> alignment                                          */
  U8                        _align1;                  /*< 19:  1> alignment                                          */
} T_MMGMM_ACTIVATE_IND;
#endif

#ifndef __T_MMGMM_ATTACH_STARTED_REQ__
#define __T_MMGMM_ATTACH_STARTED_REQ__
/*
 * 
 * CCDGEN:WriteStruct_Count==1833
 */
typedef struct
{
  U8                        dummy;                    /*<  0:  1> no parameters                                      */
} T_MMGMM_ATTACH_STARTED_REQ;
#endif

#ifndef __T_MMGMM_ATTACH_ACC_REQ__
#define __T_MMGMM_ATTACH_ACC_REQ__
/*
 * 
 * CCDGEN:WriteStruct_Count==1834
 */
typedef struct
{
  T_plmn                    plmn;                     /*<  0:  8> plmn identification                                */
  U16                       lac;                      /*<  8:  2> location area code                                 */
  U8                        v_tmsi;                   /*< 10:  1> TMSI available or not                              */
  U8                        _align0;                  /*< 11:  1> alignment                                          */
  U32                       tmsi;                     /*< 12:  4> TMSI                                               */
  U8                        v_equ_plmn_list;          /*< 16:  1> Validity of equivalent plmn list                   */
  U8                        _align1;                  /*< 17:  1> alignment                                          */
  U8                        _align2;                  /*< 18:  1> alignment                                          */
  U8                        _align3;                  /*< 19:  1> alignment                                          */
  T_equ_plmn_list           equ_plmn_list;            /*< 20: 44> Equivalent plmn List                               */
} T_MMGMM_ATTACH_ACC_REQ;
#endif

#ifndef __T_MMGMM_ATTACH_REJ_REQ__
#define __T_MMGMM_ATTACH_REJ_REQ__
/*
 * 
 * CCDGEN:WriteStruct_Count==1835
 */
typedef struct
{
  U16                       cause;                    /*<  0:  2> MM or GMM error cause                              */
  U8                        _align0;                  /*<  2:  1> alignment                                          */
  U8                        _align1;                  /*<  3:  1> alignment                                          */
} T_MMGMM_ATTACH_REJ_REQ;
#endif

#ifndef __T_MMGMM_DETACH_STARTED_REQ__
#define __T_MMGMM_DETACH_STARTED_REQ__
/*
 * 
 * CCDGEN:WriteStruct_Count==1836
 */
typedef struct
{
  U8                        dummy;                    /*<  0:  1> no parameters                                      */
} T_MMGMM_DETACH_STARTED_REQ;
#endif

#ifndef __T_MMGMM_START_T3212_REQ__
#define __T_MMGMM_START_T3212_REQ__
/*
 * 
 * CCDGEN:WriteStruct_Count==1837
 */
typedef struct
{
  U8                        dummy;                    /*<  0:  1> no parameters                                      */
} T_MMGMM_START_T3212_REQ;
#endif

#ifndef __T_MMGMM_T3212_VAL_IND__
#define __T_MMGMM_T3212_VAL_IND__
/*
 * 
 * CCDGEN:WriteStruct_Count==1838
 */
typedef struct
{
  U32                       t3212_val;                /*<  0:  4> value of T3212                                     */
} T_MMGMM_T3212_VAL_IND;
#endif

#ifndef __T_MMGMM_INFO_IND__
#define __T_MMGMM_INFO_IND__
/*
 * 
 * CCDGEN:WriteStruct_Count==1839
 */
typedef struct
{
  T_plmn                    plmn;                     /*<  0:  8> plmn identification                                */
  T_full_name               full_name;                /*<  8: 48> Network name, long format                          */
  T_short_name              short_name;               /*< 56: 48> Network name, short format                         */
  T_ntz                     ntz;                      /*<104:  4> Network time zone                                  */
  T_time                    time;                     /*<108:  8> Network time                                       */
// VAR-FF: REL99 
//   U8                        daylight_save_time;       /*<  0:  0> network daylight saving time                       */
} T_MMGMM_INFO_IND;
#endif

#ifndef __T_MMGMM_CM_EMERGENCY_IND__
#define __T_MMGMM_CM_EMERGENCY_IND__
/*
 * 
 * CCDGEN:WriteStruct_Count==1840
 */
typedef struct
{
  U8                        dummy;                    /*<  0:  1> no parameters                                      */
} T_MMGMM_CM_EMERGENCY_IND;
#endif

#ifndef __T_MMGMM_CM_EMERGENCY_RES__
#define __T_MMGMM_CM_EMERGENCY_RES__
/*
 * 
 * CCDGEN:WriteStruct_Count==1841
 */
typedef struct
{
  U8                        cm_establish_res;         /*<  0:  1> cm establish response                              */
  U8                        _align0;                  /*<  1:  1> alignment                                          */
  U8                        _align1;                  /*<  2:  1> alignment                                          */
  U8                        _align2;                  /*<  3:  1> alignment                                          */
} T_MMGMM_CM_EMERGENCY_RES;
#endif

#ifndef __T_MMGMM_LUP_ACCEPT_IND__
#define __T_MMGMM_LUP_ACCEPT_IND__
/*
 * 
 * CCDGEN:WriteStruct_Count==1842
 */
typedef struct
{
  T_plmn                    plmn;                     /*<  0:  8> plmn identification                                */
  U16                       lac;                      /*<  8:  2> location area code                                 */
  U16                       cid;                      /*< 10:  2> cell id                                            */
} T_MMGMM_LUP_ACCEPT_IND;
#endif

#ifndef __T_MMGMM_LUP_NEEDED_IND__
#define __T_MMGMM_LUP_NEEDED_IND__
/*
 * 
 * CCDGEN:WriteStruct_Count==1843
 */
typedef struct
{
  U8                        reason;                   /*<  0:  1> Location updating needed reason                    */
  U8                        _align0;                  /*<  1:  1> alignment                                          */
  U8                        _align1;                  /*<  2:  1> alignment                                          */
  U8                        _align2;                  /*<  3:  1> alignment                                          */
} T_MMGMM_LUP_NEEDED_IND;
#endif

#ifndef __T_MMGMM_CIPHERING_IND__
#define __T_MMGMM_CIPHERING_IND__
/*
 * 
 * CCDGEN:WriteStruct_Count==1844
 */
typedef struct
{
  U8                        ciph;                     /*<  0:  1> cipher mode                                        */
  U8                        _align0;                  /*<  1:  1> alignment                                          */
  U8                        _align1;                  /*<  2:  1> alignment                                          */
  U8                        _align2;                  /*<  3:  1> alignment                                          */
} T_MMGMM_CIPHERING_IND;
#endif

#ifndef __T_MMGMM_ALLOWED_REQ__
#define __T_MMGMM_ALLOWED_REQ__
/*
 * 
 * CCDGEN:WriteStruct_Count==1845
 */
typedef struct
{
  T_plmn                    plmn;                     /*<  0:  8> plmn identification                                */
  U16                       lac;                      /*<  8:  2> location area code                                 */
  U8                        v_equ_plmn_list;          /*< 10:  1> Validity of equivalent plmn list                   */
  U8                        _align0;                  /*< 11:  1> alignment                                          */
  T_equ_plmn_list           equ_plmn_list;            /*< 12: 44> Equivalent plmn List                               */
} T_MMGMM_ALLOWED_REQ;
#endif

#ifndef __T_MMGMM_TMSI_IND__
#define __T_MMGMM_TMSI_IND__
/*
 * 
 * CCDGEN:WriteStruct_Count==1846
 */
typedef struct
{
  U32                       tmsi;                     /*<  0:  4> TMSI                                               */
} T_MMGMM_TMSI_IND;
#endif

#ifndef __T_MMGMM_TRIGGER_REQ__
#define __T_MMGMM_TRIGGER_REQ__
/*
 * 
 * CCDGEN:WriteStruct_Count==1847
 */
typedef struct
{
  U8                        dummy;                    /*<  0:  1> no parameters                                      */
} T_MMGMM_TRIGGER_REQ;
#endif

#ifndef __T_MMGMM_AHPLMN_IND__
#define __T_MMGMM_AHPLMN_IND__
/*
 * 
 * CCDGEN:WriteStruct_Count==1848
 */
typedef struct
{
  T_ahplmn                  ahplmn;                   /*<  0:  8> ahplmn identification                              */
} T_MMGMM_AHPLMN_IND;
#endif


#include "CDG_LEAVE.h"


#endif
