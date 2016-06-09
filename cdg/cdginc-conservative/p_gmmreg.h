/*
+--------------------------------------------------------------------------+
| PROJECT : PROTOCOL STACK                                                 |
| FILE    : p_gmmreg.h                                                     |
| SOURCE  : "sap\gmmreg.pdf"                                               |
| LastModified : "2002-11-28"                                              |
| IdAndVersion : "8441.115.99.013"                                         |
| SrcFileTime  : "Thu Nov 29 09:42:12 2007"                                |
| Generated by CCDGEN_2.5.5A on Thu Sep 25 09:52:55 2014                   |
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


#ifndef P_GMMREG_H
#define P_GMMREG_H


#define CDG_ENTER__P_GMMREG_H

#define CDG_ENTER__FILENAME _P_GMMREG_H
#define CDG_ENTER__P_GMMREG_H__FILE_TYPE CDGINC
#define CDG_ENTER__P_GMMREG_H__LAST_MODIFIED _2002_11_28
#define CDG_ENTER__P_GMMREG_H__ID_AND_VERSION _8441_115_99_013

#define CDG_ENTER__P_GMMREG_H__SRC_FILE_TIME _Thu_Nov_29_09_42_12_2007

#include "CDG_ENTER.h"

#undef CDG_ENTER__P_GMMREG_H

#undef CDG_ENTER__FILENAME


#include "p_gmmreg.val"

#ifndef __T_plmn__
#define __T_plmn__
/*
 * PLMN identification
 * CCDGEN:WriteStruct_Count==1570
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
 * AHPLMN identification
 * CCDGEN:WriteStruct_Count==1571
 */
#ifndef __T_ahplmn__
#define __T_ahplmn__
typedef T_plmn T_ahplmn;
#endif
#ifndef __T_full_net_name_gmm__
#define __T_full_net_name_gmm__
/*
 * Network name, long format
 * CCDGEN:WriteStruct_Count==1572
 */
typedef struct
{
  U8                        v_name;                   /*<  0:  1> valid flag                                         */
  U8                        dcs;                      /*<  1:  1> coding scheme                                      */
  U8                        add_ci;                   /*<  2:  1> add ci indicator                                   */
  U8                        num_spare;                /*<  3:  1> num spare bits                                     */
  U8                        c_text;                   /*<  4:  1> counter                                            */
  U8                        text[MMR_MAX_TEXT_LEN];   /*<  5: 40> name                                               */
  U8                        _align0;                  /*< 45:  1> alignment                                          */
  U8                        _align1;                  /*< 46:  1> alignment                                          */
  U8                        _align2;                  /*< 47:  1> alignment                                          */
} T_full_net_name_gmm;
#endif

/*
 * Network name, short format
 * CCDGEN:WriteStruct_Count==1573
 */
#ifndef __T_short_net_name_gmm__
#define __T_short_net_name_gmm__
typedef T_full_net_name_gmm T_short_net_name_gmm;
#endif
#ifndef __T_net_time_zone__
#define __T_net_time_zone__
/*
 * Network time zone
 * CCDGEN:WriteStruct_Count==1574
 */
typedef struct
{
  U8                        v_time_zone;              /*<  0:  1> timezone valid                                     */
  U8                        time_zone;                /*<  1:  1> timezone                                           */
  U8                        _align0;                  /*<  2:  1> alignment                                          */
  U8                        _align1;                  /*<  3:  1> alignment                                          */
} T_net_time_zone;
#endif

#ifndef __T_net_time__
#define __T_net_time__
/*
 * Network time
 * CCDGEN:WriteStruct_Count==1575
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
} T_net_time;
#endif


/*
 * End of substructure section, begin of primitive definition section
 */

#ifndef __T_GMMREG_ATTACH_REQ__
#define __T_GMMREG_ATTACH_REQ__
/*
 * 
 * CCDGEN:WriteStruct_Count==1576
 */
typedef struct
{
  U8                        mobile_class;             /*<  0:  1> Mobile Class                                       */
  U8                        attach_type;              /*<  1:  1> Attach type                                        */
  U8                        service_mode;             /*<  2:  1> Required service mode                              */
  U8                        bootup_act;               /*<  3:  1> bootup action                                      */
} T_GMMREG_ATTACH_REQ;
#endif

#ifndef __T_GMMREG_ATTACH_CNF__
#define __T_GMMREG_ATTACH_CNF__
/*
 * 
 * CCDGEN:WriteStruct_Count==1577
 */
typedef struct
{
  U8                        attach_type;              /*<  0:  1> Attach type                                        */
  U8                        _align0;                  /*<  1:  1> alignment                                          */
  U8                        _align1;                  /*<  2:  1> alignment                                          */
  U8                        _align2;                  /*<  3:  1> alignment                                          */
  T_plmn                    plmn;                     /*<  4:  8> PLMN identification                                */
  U16                       lac;                      /*< 12:  2> location area code                                 */
  U8                        rac;                      /*< 14:  1> routing area code                                  */
  U8                        _align3;                  /*< 15:  1> alignment                                          */
  U16                       cid;                      /*< 16:  2> cell id                                            */
  U8                        gprs_indicator;           /*< 18:  1> GPRS indicator                                     */
  U8                        search_running;           /*< 19:  1> Search is still running                            */
  U8                        rt;                       /*< 20:  1> This parameter indicates the radio access technology available in the cell.  */
  U8                        bootup_cause;             /*< 21:  1> bootup cause                                       */
  U8                        _align4;                  /*< 22:  1> alignment                                          */
  U8                        _align5;                  /*< 23:  1> alignment                                          */
} T_GMMREG_ATTACH_CNF;
#endif

#ifndef __T_GMMREG_ATTACH_REJ__
#define __T_GMMREG_ATTACH_REJ__
/*
 * 
 * CCDGEN:WriteStruct_Count==1578
 */
typedef struct
{
  U8                        detach_type;              /*<  0:  1> Detach type                                        */
  U8                        _align0;                  /*<  1:  1> alignment                                          */
  U16                       cause;                    /*<  2:  2> error cause                                        */
  U8                        search_running;           /*<  4:  1> Search is still running                            */
  U8                        _align1;                  /*<  5:  1> alignment                                          */
  U16                       service;                  /*<  6:  2> service (either no or limited service)             */
} T_GMMREG_ATTACH_REJ;
#endif

#ifndef __T_GMMREG_DETACH_REQ__
#define __T_GMMREG_DETACH_REQ__
/*
 * 
 * CCDGEN:WriteStruct_Count==1579
 */
typedef struct
{
  U8                        detach_type;              /*<  0:  1> Detach type                                        */
  U8                        _align0;                  /*<  1:  1> alignment                                          */
  U8                        _align1;                  /*<  2:  1> alignment                                          */
  U8                        _align2;                  /*<  3:  1> alignment                                          */
} T_GMMREG_DETACH_REQ;
#endif

#ifndef __T_GMMREG_DETACH_CNF__
#define __T_GMMREG_DETACH_CNF__
/*
 * 
 * CCDGEN:WriteStruct_Count==1580
 */
typedef struct
{
  U8                        detach_type;              /*<  0:  1> Detach type                                        */
  U8                        _align0;                  /*<  1:  1> alignment                                          */
  U8                        _align1;                  /*<  2:  1> alignment                                          */
  U8                        _align2;                  /*<  3:  1> alignment                                          */
} T_GMMREG_DETACH_CNF;
#endif

#ifndef __T_GMMREG_DETACH_IND__
#define __T_GMMREG_DETACH_IND__
/*
 * 
 * CCDGEN:WriteStruct_Count==1581
 */
typedef struct
{
  U8                        detach_type;              /*<  0:  1> Detach type                                        */
  U8                        _align0;                  /*<  1:  1> alignment                                          */
  U16                       cause;                    /*<  2:  2> error cause                                        */
  U8                        search_running;           /*<  4:  1> Search is still running                            */
  U8                        _align1;                  /*<  5:  1> alignment                                          */
  U16                       service;                  /*<  6:  2> service (either no or limited service)             */
} T_GMMREG_DETACH_IND;
#endif

#ifndef __T_GMMREG_NET_REQ__
#define __T_GMMREG_NET_REQ__
/*
 * 
 * CCDGEN:WriteStruct_Count==1582
 */
typedef struct
{
  U8                        dummy;                    /*<  0:  1> no parameters                                      */
} T_GMMREG_NET_REQ;
#endif

#ifndef __T_GMMREG_PLMN_IND__
#define __T_GMMREG_PLMN_IND__
/*
 * 
 * CCDGEN:WriteStruct_Count==1583
 */
typedef struct
{
  U16                       cause;                    /*<  0:  2> error cause                                        */
  U8                        _align0;                  /*<  2:  1> alignment                                          */
  U8                        _align1;                  /*<  3:  1> alignment                                          */
  T_plmn                    plmn[GMMREG_MAX_PLMN_ID]; /*<  4: 96> PLMN identification                                */
  U8                        forb_ind[GMMREG_MAX_PLMN_ID]; /*<100: 12> Forbidden PLMN indicator                           */
  U16                       lac_list[GMMREG_MAX_PLMN_ID]; /*<112: 24> LACs of found PLMNs                                */
  U8                        rxlevel[GMMREG_MAX_PLMN_ID]; /*<136: 12> Fieldstrength                                      */
  U8                        gprs_status[GMMREG_MAX_PLMN_ID]; /*<148: 12> GPRS Status                                        */
} T_GMMREG_PLMN_IND;
#endif

#ifndef __T_GMMREG_PLMN_RES__
#define __T_GMMREG_PLMN_RES__
/*
 * 
 * CCDGEN:WriteStruct_Count==1584
 */
typedef struct
{
  T_plmn                    plmn;                     /*<  0:  8> PLMN identification                                */
  U8                        mobile_class;             /*<  8:  1> Mobile Class                                       */
  U8                        attach_type;              /*<  9:  1> Attach type                                        */
  U8                        _align0;                  /*< 10:  1> alignment                                          */
  U8                        _align1;                  /*< 11:  1> alignment                                          */
} T_GMMREG_PLMN_RES;
#endif

#ifndef __T_GMMREG_SUSPEND_IND__
#define __T_GMMREG_SUSPEND_IND__
/*
 * 
 * CCDGEN:WriteStruct_Count==1585
 */
typedef struct
{
  U8                        cell_state;               /*<  0:  1> Cell State                                         */
  U8                        _align0;                  /*<  1:  1> alignment                                          */
  U8                        _align1;                  /*<  2:  1> alignment                                          */
  U8                        _align2;                  /*<  3:  1> alignment                                          */
} T_GMMREG_SUSPEND_IND;
#endif

#ifndef __T_GMMREG_RESUME_IND__
#define __T_GMMREG_RESUME_IND__
/*
 * 
 * CCDGEN:WriteStruct_Count==1586
 */
typedef struct
{
  U8                        dummy;                    /*<  0:  1> no parameters                                      */
} T_GMMREG_RESUME_IND;
#endif

#ifndef __T_GMMREG_PLMN_MODE_REQ__
#define __T_GMMREG_PLMN_MODE_REQ__
/*
 * 
 * CCDGEN:WriteStruct_Count==1587
 */
typedef struct
{
  U8                        net_selection_mode;       /*<  0:  1> network selction mode                              */
  U8                        _align0;                  /*<  1:  1> alignment                                          */
  U8                        _align1;                  /*<  2:  1> alignment                                          */
  U8                        _align2;                  /*<  3:  1> alignment                                          */
} T_GMMREG_PLMN_MODE_REQ;
#endif

#ifndef __T_GMMREG_INFO_IND__
#define __T_GMMREG_INFO_IND__
/*
 * 
 * CCDGEN:WriteStruct_Count==1588
 */
typedef struct
{
  T_plmn                    plmn;                     /*<  0:  8> PLMN identification                                */
  T_full_net_name_gmm       full_net_name_gmm;        /*<  8: 48> Network name, long format                          */
  T_short_net_name_gmm      short_net_name_gmm;       /*< 56: 48> Network name, short format                         */
  T_net_time_zone           net_time_zone;            /*<104:  4> Network time zone                                  */
  T_net_time                net_time;                 /*<108:  8> Network time                                       */
// ELEM-FF: REL99 
//   U8                        net_daylight_save_time;   /*<  0:  0> Network daylight saving time                       */
} T_GMMREG_INFO_IND;
#endif

#ifndef __T_GMMREG_CONFIG_REQ__
#define __T_GMMREG_CONFIG_REQ__
/*
 * 
 * CCDGEN:WriteStruct_Count==1589
 */
typedef struct
{
  U8                        cipher_on;                /*<  0:  1> ciphering on or off                                */
  U8                        tlli_handling;            /*<  1:  1> tlli handling in attach proc                       */
  U8                        _align0;                  /*<  2:  1> alignment                                          */
  U8                        _align1;                  /*<  3:  1> alignment                                          */
} T_GMMREG_CONFIG_REQ;
#endif

#ifndef __T_GMMREG_CIPHERING_IND__
#define __T_GMMREG_CIPHERING_IND__
/*
 * 
 * CCDGEN:WriteStruct_Count==1590
 */
typedef struct
{
  U8                        gsm_ciph;                 /*<  0:  1> GSM Ciphering Indicator                            */
  U8                        gprs_ciph;                /*<  1:  1> GPRS Ciphering Indicator                           */
  U8                        _align0;                  /*<  2:  1> alignment                                          */
  U8                        _align1;                  /*<  3:  1> alignment                                          */
} T_GMMREG_CIPHERING_IND;
#endif

#ifndef __T_GMMREG_AHPLMN_IND__
#define __T_GMMREG_AHPLMN_IND__
/*
 * 
 * CCDGEN:WriteStruct_Count==1591
 */
typedef struct
{
  T_ahplmn                  ahplmn;                   /*<  0:  8> AHPLMN identification                              */
} T_GMMREG_AHPLMN_IND;
#endif


#include "CDG_LEAVE.h"


#endif
