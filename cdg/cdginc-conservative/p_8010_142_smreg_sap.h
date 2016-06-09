/*
+--------------------------------------------------------------------------+
| PROJECT : PROTOCOL STACK                                                 |
| FILE    : p_8010_142_smreg_sap.h                                         |
| SOURCE  : "sap\8010_142_smreg_sap.pdf"                                   |
| LastModified : "2003-08-22"                                              |
| IdAndVersion : "8010.142.02.011"                                         |
| SrcFileTime  : "Thu Nov 29 09:28:36 2007"                                |
| Generated by CCDGEN_2.5.5A on Thu Sep 25 09:52:55 2014                   |
|           !!DO NOT MODIFY!!DO NOT MODIFY!!DO NOT MODIFY!!                |
+--------------------------------------------------------------------------+
*/

/* PRAGMAS
 * PREFIX                 : SMREG
 * COMPATIBILITY_DEFINES  : NO
 * ALWAYS_ENUM_IN_VAL_FILE: YES
 * ENABLE_GROUP: NO
 * CAPITALIZE_TYPENAME: NO
 */


#ifndef P_8010_142_SMREG_SAP_H
#define P_8010_142_SMREG_SAP_H


#define CDG_ENTER__P_8010_142_SMREG_SAP_H

#define CDG_ENTER__FILENAME _P_8010_142_SMREG_SAP_H
#define CDG_ENTER__P_8010_142_SMREG_SAP_H__FILE_TYPE CDGINC
#define CDG_ENTER__P_8010_142_SMREG_SAP_H__LAST_MODIFIED _2003_08_22
#define CDG_ENTER__P_8010_142_SMREG_SAP_H__ID_AND_VERSION _8010_142_02_011

#define CDG_ENTER__P_8010_142_SMREG_SAP_H__SRC_FILE_TIME _Thu_Nov_29_09_28_36_2007

#include "CDG_ENTER.h"

#undef CDG_ENTER__P_8010_142_SMREG_SAP_H

#undef CDG_ENTER__FILENAME


#include "p_8010_142_smreg_sap.val"

#include "p_8010_152_ps_include.h"

#include "p_8010_137_nas_include.h"

#include "p_8010_153_cause_include.h"


/*
 * typedef between var and valtab enums
 */
#ifndef __T_SMREG_pdp_type__
#define __T_SMREG_pdp_type__
typedef T_SMREG_VAL_pdp_type T_SMREG_pdp_type;
#endif

#ifndef __T_SMREG_apn__
#define __T_SMREG_apn__
/*
 * access point name
 * CCDGEN:WriteStruct_Count==1316
 */
typedef struct
{
  U8                        c_apn_buf;                /*<  0:  1> counter                                            */
  U8                        apn_buf[102];             /*<  1:102> Access point name value                            */
  U8                        _align0;                  /*<103:  1> alignment                                          */
} T_SMREG_apn;
#endif


/*
 * End of substructure section, begin of primitive definition section
 */

#ifndef __T_SMREG_CONFIGURE_REQ__
#define __T_SMREG_CONFIGURE_REQ__
/*
 * 
 * CCDGEN:WriteStruct_Count==1317
 */
typedef struct
{
  U8                        rat;                      /*<  0:  1> T_PS_rat,  Radio access technology                 */
  U8                        sgsn_rel;                 /*<  1:  1> T_PS_sgsn_rel,  sgsn release version               */
  U8                        _align0;                  /*<  2:  1> alignment                                          */
  U8                        _align1;                  /*<  3:  1> alignment                                          */
} T_SMREG_CONFIGURE_REQ;
#endif

#ifndef __T_SMREG_PDP_ACTIVATE_REQ__
#define __T_SMREG_PDP_ACTIVATE_REQ__
/*
 * 
 * CCDGEN:WriteStruct_Count==1318
 */
typedef struct
{
  U8                        nsapi;                    /*<  0:  1> T_NAS_nsapi,  network layer service access point identifier */
  U8                        ti;                       /*<  1:  1> T_NAS_ti,  transaction identifier                  */
  U8                        _align0;                  /*<  2:  1> alignment                                          */
  U8                        _align1;                  /*<  3:  1> alignment                                          */
  T_NAS_comp_params         comp_params;              /*<  4:  4> Header compression parameters (type defined in "p_8010_137_nas_include.h") */
  T_PS_ctrl_qos             ctrl_qos;                 /*<  8:  4> (enum=32bit) controller for union                  */
  T_PS_qos                  qos;                      /*< 12:  0> quality of service (type defined in "p_8010_152_ps_include.h") */
  T_PS_ctrl_min_qos         ctrl_min_qos;             /*<  0:  4> (enum=32bit) controller for union                  */
  T_PS_min_qos              min_qos;                  /*<  4:  0> quality of service (type defined in "p_8010_152_ps_include.h") */
  U8                        pdp_type;                 /*<  0:  1> T_SMREG_pdp_type,  PDP type                        */
  T_NAS_ctrl_ip_address     ctrl_ip_address;          /*<  0:  4> (enum=32bit) controller for union                  */
  T_NAS_ip_address          ip_address;               /*<  4: 16> IP Address (type defined in "p_8010_137_nas_include.h") */
  T_SMREG_apn               apn;                      /*<  0:104> access point name                                  */
  U8                        _align2;                  /*<  0:  1> alignment                                          */
  U8                        _align3;                  /*<  1:  1> alignment                                          */
  U8                        _align4;                  /*<  2:  1> alignment                                          */
  U8                        v_tft;                    /*<  3:  1> valid-flag                                         */
  T_NAS_tft                 tft;                      /*<  4:  8> Traffic Flow Template (type defined in "p_8010_137_nas_include.h") */
  T_sdu                     sdu;                      /*<  0:  0> Container for a message                            */
} T_SMREG_PDP_ACTIVATE_REQ;
#endif

#ifndef __T_SMREG_PDP_ACTIVATE_CNF__
#define __T_SMREG_PDP_ACTIVATE_CNF__
/*
 * 
 * CCDGEN:WriteStruct_Count==1319
 */
typedef struct
{
  U8                        nsapi;                    /*<  0:  1> T_NAS_nsapi,  network layer service access point identifier */
  U8                        pdp_type;                 /*<  1:  1> T_SMREG_pdp_type,  PDP type                        */
  U8                        _align0;                  /*<  2:  1> alignment                                          */
  U8                        _align1;                  /*<  3:  1> alignment                                          */
  T_PS_ctrl_qos             ctrl_qos;                 /*<  4:  4> (enum=32bit) controller for union                  */
  T_PS_qos                  qos;                      /*<  8:  0> quality of service (type defined in "p_8010_152_ps_include.h") */
  T_NAS_comp_params         comp_params;              /*<  0:  4> Header compression parameters (type defined in "p_8010_137_nas_include.h") */
  T_NAS_ctrl_ip_address     ctrl_ip_address;          /*<  0:  4> (enum=32bit) controller for union                  */
  T_NAS_ip_address          ip_address;               /*<  4: 16> IP Address (type defined in "p_8010_137_nas_include.h") */
  T_sdu                     sdu;                      /*<  0:  0> Container for a message                            */
} T_SMREG_PDP_ACTIVATE_CNF;
#endif

#ifndef __T_SMREG_PDP_ACTIVATE_IND__
#define __T_SMREG_PDP_ACTIVATE_IND__
/*
 * 
 * CCDGEN:WriteStruct_Count==1320
 */
typedef struct
{
  U8                        ti;                       /*<  0:  1> T_NAS_ti,  transaction identifier                  */
  U8                        pdp_type;                 /*<  1:  1> T_SMREG_pdp_type,  PDP type                        */
  U8                        _align0;                  /*<  2:  1> alignment                                          */
  U8                        _align1;                  /*<  3:  1> alignment                                          */
  T_NAS_ctrl_ip_address     ctrl_ip_address;          /*<  4:  4> (enum=32bit) controller for union                  */
  T_NAS_ip_address          ip_address;               /*<  8: 16> IP Address (type defined in "p_8010_137_nas_include.h") */
  T_SMREG_apn               apn;                      /*< 24:104> access point name                                  */
} T_SMREG_PDP_ACTIVATE_IND;
#endif

#ifndef __T_SMREG_PDP_ACTIVATE_REJ__
#define __T_SMREG_PDP_ACTIVATE_REJ__
/*
 * 
 * CCDGEN:WriteStruct_Count==1321
 */
typedef struct
{
  U8                        nsapi;                    /*<  0:  1> T_NAS_nsapi,  network layer service access point identifier */
  U8                        _align0;                  /*<  1:  1> alignment                                          */
  U8                        _align1;                  /*<  2:  1> alignment                                          */
  U8                        _align2;                  /*<  3:  1> alignment                                          */
  T_CAUSE_ps_cause          ps_cause;                 /*<  4:  4> Cause element containing result of operation (type defined in "p_8010_153_cause_include.h") */
} T_SMREG_PDP_ACTIVATE_REJ;
#endif

#ifndef __T_SMREG_PDP_ACTIVATE_REJ_RES__
#define __T_SMREG_PDP_ACTIVATE_REJ_RES__
/*
 * 
 * CCDGEN:WriteStruct_Count==1322
 */
typedef struct
{
  U8                        ti;                       /*<  0:  1> T_NAS_ti,  transaction identifier                  */
  U8                        _align0;                  /*<  1:  1> alignment                                          */
  U8                        _align1;                  /*<  2:  1> alignment                                          */
  U8                        _align2;                  /*<  3:  1> alignment                                          */
  T_CAUSE_ps_cause          ps_cause;                 /*<  4:  4> Cause element containing result of operation (type defined in "p_8010_153_cause_include.h") */
} T_SMREG_PDP_ACTIVATE_REJ_RES;
#endif

#ifndef __T_SMREG_PDP_ACTIVATE_SEC_REQ__
#define __T_SMREG_PDP_ACTIVATE_SEC_REQ__
/*
 * 
 * CCDGEN:WriteStruct_Count==1323
 */
typedef struct
{
  U8                        nsapi;                    /*<  0:  1> T_NAS_nsapi,  network layer service access point identifier */
  U8                        pri_nsapi;                /*<  1:  1> T_NAS_pri_nsapi,  network layer service access point identifier */
  U8                        ti;                       /*<  2:  1> T_NAS_ti,  transaction identifier                  */
  U8                        _align0;                  /*<  3:  1> alignment                                          */
  T_NAS_comp_params         comp_params;              /*<  4:  4> Header compression parameters (type defined in "p_8010_137_nas_include.h") */
  T_PS_ctrl_qos             ctrl_qos;                 /*<  8:  4> (enum=32bit) controller for union                  */
  T_PS_qos                  qos;                      /*< 12:  0> quality of service (type defined in "p_8010_152_ps_include.h") */
  T_PS_ctrl_min_qos         ctrl_min_qos;             /*<  0:  4> (enum=32bit) controller for union                  */
  T_PS_min_qos              min_qos;                  /*<  4:  0> quality of service (type defined in "p_8010_152_ps_include.h") */
  U8                        _align1;                  /*<  0:  1> alignment                                          */
  U8                        _align2;                  /*<  1:  1> alignment                                          */
  U8                        _align3;                  /*<  2:  1> alignment                                          */
  U8                        v_tft;                    /*<  3:  1> valid-flag                                         */
  T_NAS_tft                 tft;                      /*<  4:  8> Traffic Flow Template (type defined in "p_8010_137_nas_include.h") */
} T_SMREG_PDP_ACTIVATE_SEC_REQ;
#endif

#ifndef __T_SMREG_PDP_ACTIVATE_SEC_CNF__
#define __T_SMREG_PDP_ACTIVATE_SEC_CNF__
/*
 * 
 * CCDGEN:WriteStruct_Count==1324
 */
typedef struct
{
  U8                        nsapi;                    /*<  0:  1> T_NAS_nsapi,  network layer service access point identifier */
  U8                        _align0;                  /*<  1:  1> alignment                                          */
  U8                        _align1;                  /*<  2:  1> alignment                                          */
  U8                        _align2;                  /*<  3:  1> alignment                                          */
  T_PS_ctrl_qos             ctrl_qos;                 /*<  4:  4> (enum=32bit) controller for union                  */
  T_PS_qos                  qos;                      /*<  8:  0> quality of service (type defined in "p_8010_152_ps_include.h") */
} T_SMREG_PDP_ACTIVATE_SEC_CNF;
#endif

#ifndef __T_SMREG_PDP_ACTIVATE_SEC_REJ__
#define __T_SMREG_PDP_ACTIVATE_SEC_REJ__
/*
 * 
 * CCDGEN:WriteStruct_Count==1325
 */
typedef struct
{
  U8                        nsapi;                    /*<  0:  1> T_NAS_nsapi,  network layer service access point identifier */
  U8                        _align0;                  /*<  1:  1> alignment                                          */
  U8                        _align1;                  /*<  2:  1> alignment                                          */
  U8                        _align2;                  /*<  3:  1> alignment                                          */
  T_CAUSE_ps_cause          ps_cause;                 /*<  4:  4> Cause element containing result of operation (type defined in "p_8010_153_cause_include.h") */
} T_SMREG_PDP_ACTIVATE_SEC_REJ;
#endif

#ifndef __T_SMREG_PDP_DEACTIVATE_REQ__
#define __T_SMREG_PDP_DEACTIVATE_REQ__
/*
 * 
 * CCDGEN:WriteStruct_Count==1326
 */
typedef struct
{
  U16                       nsapi_set;                /*<  0:  2> set of network layer service access point identifiers */
  U8                        rel_ind;                  /*<  2:  1> T_PS_rel_ind,  Local Release Indicator             */
  U8                        _align0;                  /*<  3:  1> alignment                                          */
} T_SMREG_PDP_DEACTIVATE_REQ;
#endif

#ifndef __T_SMREG_PDP_DEACTIVATE_CNF__
#define __T_SMREG_PDP_DEACTIVATE_CNF__
/*
 * 
 * CCDGEN:WriteStruct_Count==1327
 */
typedef struct
{
  U16                       nsapi_set;                /*<  0:  2> set of network layer service access point identifiers */
  U8                        _align0;                  /*<  2:  1> alignment                                          */
  U8                        _align1;                  /*<  3:  1> alignment                                          */
} T_SMREG_PDP_DEACTIVATE_CNF;
#endif

#ifndef __T_SMREG_PDP_DEACTIVATE_IND__
#define __T_SMREG_PDP_DEACTIVATE_IND__
/*
 * 
 * CCDGEN:WriteStruct_Count==1328
 */
typedef struct
{
  U16                       nsapi_set;                /*<  0:  2> set of network layer service access point identifiers */
  U8                        _align0;                  /*<  2:  1> alignment                                          */
  U8                        _align1;                  /*<  3:  1> alignment                                          */
  T_CAUSE_ps_cause          ps_cause;                 /*<  4:  4> Cause element containing result of operation (type defined in "p_8010_153_cause_include.h") */
} T_SMREG_PDP_DEACTIVATE_IND;
#endif

#ifndef __T_SMREG_PDP_MODIFY_REQ__
#define __T_SMREG_PDP_MODIFY_REQ__
/*
 * 
 * CCDGEN:WriteStruct_Count==1329
 */
typedef struct
{
  U8                        nsapi;                    /*<  0:  1> T_NAS_nsapi,  network layer service access point identifier */
  U8                        _align0;                  /*<  1:  1> alignment                                          */
  U8                        _align1;                  /*<  2:  1> alignment                                          */
  U8                        _align2;                  /*<  3:  1> alignment                                          */
  T_PS_ctrl_qos             ctrl_qos;                 /*<  4:  4> (enum=32bit) controller for union                  */
  T_PS_qos                  qos;                      /*<  8:  0> quality of service (type defined in "p_8010_152_ps_include.h") */
  T_PS_ctrl_min_qos         ctrl_min_qos;             /*<  0:  4> (enum=32bit) controller for union                  */
  T_PS_min_qos              min_qos;                  /*<  4:  0> quality of service (type defined in "p_8010_152_ps_include.h") */
  U8                        _align3;                  /*<  0:  1> alignment                                          */
  U8                        _align4;                  /*<  1:  1> alignment                                          */
  U8                        _align5;                  /*<  2:  1> alignment                                          */
  U8                        v_tft;                    /*<  3:  1> valid-flag                                         */
  T_NAS_tft                 tft;                      /*<  4:  8> Traffic Flow Template (type defined in "p_8010_137_nas_include.h") */
} T_SMREG_PDP_MODIFY_REQ;
#endif

#ifndef __T_SMREG_PDP_MODIFY_CNF__
#define __T_SMREG_PDP_MODIFY_CNF__
/*
 * 
 * CCDGEN:WriteStruct_Count==1330
 */
typedef struct
{
  U8                        nsapi;                    /*<  0:  1> T_NAS_nsapi,  network layer service access point identifier */
  U8                        _align0;                  /*<  1:  1> alignment                                          */
  U8                        _align1;                  /*<  2:  1> alignment                                          */
  U8                        _align2;                  /*<  3:  1> alignment                                          */
  T_PS_ctrl_qos             ctrl_qos;                 /*<  4:  4> (enum=32bit) controller for union                  */
  T_PS_qos                  qos;                      /*<  8:  0> quality of service (type defined in "p_8010_152_ps_include.h") */
} T_SMREG_PDP_MODIFY_CNF;
#endif

#ifndef __T_SMREG_PDP_MODIFY_IND__
#define __T_SMREG_PDP_MODIFY_IND__
/*
 * 
 * CCDGEN:WriteStruct_Count==1331
 */
typedef struct
{
  U8                        nsapi;                    /*<  0:  1> T_NAS_nsapi,  network layer service access point identifier */
  U8                        _align0;                  /*<  1:  1> alignment                                          */
  U8                        _align1;                  /*<  2:  1> alignment                                          */
  U8                        _align2;                  /*<  3:  1> alignment                                          */
  T_PS_ctrl_qos             ctrl_qos;                 /*<  4:  4> (enum=32bit) controller for union                  */
  T_PS_qos                  qos;                      /*<  8:  0> quality of service (type defined in "p_8010_152_ps_include.h") */
  T_NAS_ctrl_ip_address     ctrl_ip_address;          /*<  0:  4> (enum=32bit) controller for union                  */
  T_NAS_ip_address          ip_address;               /*<  4: 16> IP Address (type defined in "p_8010_137_nas_include.h") */
} T_SMREG_PDP_MODIFY_IND;
#endif

#ifndef __T_SMREG_PDP_MODIFY_REJ__
#define __T_SMREG_PDP_MODIFY_REJ__
/*
 * 
 * CCDGEN:WriteStruct_Count==1332
 */
typedef struct
{
  U8                        nsapi;                    /*<  0:  1> T_NAS_nsapi,  network layer service access point identifier */
  U8                        _align0;                  /*<  1:  1> alignment                                          */
  U8                        _align1;                  /*<  2:  1> alignment                                          */
  U8                        _align2;                  /*<  3:  1> alignment                                          */
  T_CAUSE_ps_cause          ps_cause;                 /*<  4:  4> Cause element containing result of operation (type defined in "p_8010_153_cause_include.h") */
} T_SMREG_PDP_MODIFY_REJ;
#endif


#include "CDG_LEAVE.h"


#endif
