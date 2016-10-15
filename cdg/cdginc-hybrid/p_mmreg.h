/*
+--------------------------------------------------------------------------+
| PROJECT : PROTOCOL STACK                                                 |
| FILE    : p_mmreg.h                                                      |
| SOURCE  : "sap\mmreg.pdf"                                                |
| LastModified : "2003-06-13"                                              |
| IdAndVersion : "6147.100.96.110"                                         |
| SrcFileTime  : "Thu Nov 29 09:46:52 2007"                                |
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


#ifndef P_MMREG_H
#define P_MMREG_H


#define CDG_ENTER__P_MMREG_H

#define CDG_ENTER__FILENAME _P_MMREG_H
#define CDG_ENTER__P_MMREG_H__FILE_TYPE CDGINC
#define CDG_ENTER__P_MMREG_H__LAST_MODIFIED _2003_06_13
#define CDG_ENTER__P_MMREG_H__ID_AND_VERSION _6147_100_96_110

#define CDG_ENTER__P_MMREG_H__SRC_FILE_TIME _Thu_Nov_29_09_46_52_2007

#include "CDG_ENTER.h"

#undef CDG_ENTER__P_MMREG_H

#undef CDG_ENTER__FILENAME


#include "p_mmreg.val"

#ifndef __T_plmn__
#define __T_plmn__
/*
 * plmn identification
 * CCDGEN:WriteStruct_Count==1875
 */
typedef struct
{
  U8                        v_plmn;                   /*<  0:  1> valid flag                                         */
  U8                        mcc[SIZE_MCC];            /*<  1:  3> mobile country code                                */
  U8                        mnc[SIZE_MNC];            /*<  4:  3> mobile network code                                */
  U8                        _align0;                  /*<  7:  1> alignment                                          */
} T_plmn;
#endif

/*
 * plmn identification
 * CCDGEN:WriteStruct_Count==1876
 */
#ifndef __T_new_forb_plmn__
#define __T_new_forb_plmn__
typedef T_plmn T_new_forb_plmn;
#endif
/*
 * ahplmn identification
 * CCDGEN:WriteStruct_Count==1877
 */
#ifndef __T_ahplmn__
#define __T_ahplmn__
typedef T_plmn T_ahplmn;
#endif
#ifndef __T_full_name__
#define __T_full_name__
/*
 * Network name, long format
 * CCDGEN:WriteStruct_Count==1878
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
 * CCDGEN:WriteStruct_Count==1879
 */
#ifndef __T_short_name__
#define __T_short_name__
typedef T_full_name T_short_name;
#endif
#ifndef __T_ntz__
#define __T_ntz__
/*
 * Network time zone
 * CCDGEN:WriteStruct_Count==1880
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
 * CCDGEN:WriteStruct_Count==1881
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


/*
 * End of substructure section, begin of primitive definition section
 */

#ifndef __T_MMR_REG_REQ__
#define __T_MMR_REG_REQ__
/*
 * 
 * CCDGEN:WriteStruct_Count==1882
 */
typedef struct
{
  U8                        service_mode;             /*<  0:  1> Required service mode                              */
  U8                        bootup_act;               /*<  1:  1> Bootup action                                      */
  U8                        _align0;                  /*<  2:  1> alignment                                          */
  U8                        _align1;                  /*<  3:  1> alignment                                          */
} T_MMR_REG_REQ;
#endif

#ifndef __T_MMR_REG_CNF__
#define __T_MMR_REG_CNF__
/*
 * 
 * CCDGEN:WriteStruct_Count==1883
 */
typedef struct
{
  T_plmn                    plmn;                     /*<  0:  8> plmn identification                                */
  U16                       lac;                      /*<  8:  2> location area code                                 */
  U16                       cid;                      /*< 10:  2> cell id                                            */
  U8                        bootup_cause;             /*< 12:  1> bootup cause                                       */
  U8                        _align0;                  /*< 13:  1> alignment                                          */
  U8                        _align1;                  /*< 14:  1> alignment                                          */
  U8                        _align2;                  /*< 15:  1> alignment                                          */
} T_MMR_REG_CNF;
#endif

#ifndef __T_MMR_NREG_IND__
#define __T_MMR_NREG_IND__
/*
 * 
 * CCDGEN:WriteStruct_Count==1884
 */
typedef struct
{
  U8                        service;                  /*<  0:  1> service (either no or limited service)             */
  U8                        search_running;           /*<  1:  1> Search is still running                            */
  U8                        _align0;                  /*<  2:  1> alignment                                          */
  U8                        _align1;                  /*<  3:  1> alignment                                          */
  T_new_forb_plmn           new_forb_plmn;            /*<  4:  8> plmn identification                                */
  U16                       cause;                    /*< 12:  2> Cause for loss of full service, release or error   */
  U8                        _align2;                  /*< 14:  1> alignment                                          */
  U8                        _align3;                  /*< 15:  1> alignment                                          */
} T_MMR_NREG_IND;
#endif

#ifndef __T_MMR_NREG_REQ__
#define __T_MMR_NREG_REQ__
/*
 * 
 * CCDGEN:WriteStruct_Count==1885
 */
typedef struct
{
  U8                        detach_cause;             /*<  0:  1> cause                                              */
  U8                        _align0;                  /*<  1:  1> alignment                                          */
  U8                        _align1;                  /*<  2:  1> alignment                                          */
  U8                        _align2;                  /*<  3:  1> alignment                                          */
} T_MMR_NREG_REQ;
#endif

#ifndef __T_MMR_NREG_CNF__
#define __T_MMR_NREG_CNF__
/*
 * 
 * CCDGEN:WriteStruct_Count==1886
 */
typedef struct
{
  U8                        detach_cause;             /*<  0:  1> cause                                              */
  U8                        _align0;                  /*<  1:  1> alignment                                          */
  U8                        _align1;                  /*<  2:  1> alignment                                          */
  U8                        _align2;                  /*<  3:  1> alignment                                          */
} T_MMR_NREG_CNF;
#endif

#ifndef __T_MMR_NET_REQ__
#define __T_MMR_NET_REQ__
/*
 * 
 * CCDGEN:WriteStruct_Count==1887
 */
typedef struct
{
  U8                        dummy;                    /*<  0:  1> no parameters                                      */
} T_MMR_NET_REQ;
#endif

#ifndef __T_MMR_PLMN_IND__
#define __T_MMR_PLMN_IND__
/*
 * 
 * CCDGEN:WriteStruct_Count==1888
 */
typedef struct
{
  U16                       cause;                    /*<  0:  2> Cause for loss of full service, release or error   */
  U8                        _align0;                  /*<  2:  1> alignment                                          */
  U8                        _align1;                  /*<  3:  1> alignment                                          */
  T_plmn                    plmn[MAX_PLMN_ID];        /*<  4: 96> plmn identification                                */
  U16                       lac_list[MAX_PLMN_ID];    /*<100: 24> LACs of found PLMNs                                */
  U8                        forb_ind[MAX_PLMN_ID];    /*<124: 12> Forbidden PLMN indicator                           */
  U8                        rxlevel[MAX_PLMN_ID];     /*<136: 12> Fieldstrength                                      */
} T_MMR_PLMN_IND;
#endif

#ifndef __T_MMR_PLMN_RES__
#define __T_MMR_PLMN_RES__
/*
 * 
 * CCDGEN:WriteStruct_Count==1889
 */
typedef struct
{
  T_plmn                    plmn;                     /*<  0:  8> plmn identification                                */
} T_MMR_PLMN_RES;
#endif

#ifndef __T_MMR_PLMN_MODE_REQ__
#define __T_MMR_PLMN_MODE_REQ__
/*
 * 
 * CCDGEN:WriteStruct_Count==1890
 */
typedef struct
{
  U8                        mode;                     /*<  0:  1> registration mode                                  */
  U8                        _align0;                  /*<  1:  1> alignment                                          */
  U8                        _align1;                  /*<  2:  1> alignment                                          */
  U8                        _align2;                  /*<  3:  1> alignment                                          */
} T_MMR_PLMN_MODE_REQ;
#endif

#ifndef __T_MMR_INFO_IND__
#define __T_MMR_INFO_IND__
/*
 * 
 * CCDGEN:WriteStruct_Count==1891
 */
typedef struct
{
  T_plmn                    plmn;                     /*<  0:  8> plmn identification                                */
  T_full_name               full_name;                /*<  8: 48> Network name, long format                          */
  T_short_name              short_name;               /*< 56: 48> Network name, short format                         */
  T_ntz                     ntz;                      /*<104:  4> Network time zone                                  */
  T_time                    time;                     /*<108:  8> Network time                                       */
// VAR-FF: REL99 
//   U8                        daylight_save_time;       /*<  0:  0> Network daylight saving time                       */
} T_MMR_INFO_IND;
#endif

#ifndef __T_MMR_CIPHERING_IND__
#define __T_MMR_CIPHERING_IND__
/*
 * 
 * CCDGEN:WriteStruct_Count==1892
 */
typedef struct
{
  U8                        ciph;                     /*<  0:  1> cipher mode                                        */
  U8                        _align0;                  /*<  1:  1> alignment                                          */
  U8                        _align1;                  /*<  2:  1> alignment                                          */
  U8                        _align2;                  /*<  3:  1> alignment                                          */
} T_MMR_CIPHERING_IND;
#endif

#ifndef __T_MMR_AHPLMN_IND__
#define __T_MMR_AHPLMN_IND__
/*
 * 
 * CCDGEN:WriteStruct_Count==1893
 */
typedef struct
{
  T_ahplmn                  ahplmn;                   /*<  0:  8> ahplmn identification                              */
} T_MMR_AHPLMN_IND;
#endif


#include "CDG_LEAVE.h"


#endif
