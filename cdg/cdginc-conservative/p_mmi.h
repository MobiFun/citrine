/*
+--------------------------------------------------------------------------+
| PROJECT : PROTOCOL STACK                                                 |
| FILE    : p_mmi.h                                                        |
| SOURCE  : "sap\mmi.pdf"                                                  |
| LastModified : "2004-08-20"                                              |
| IdAndVersion : "6147.113.97.103"                                         |
| SrcFileTime  : "Thu Nov 29 09:46:32 2007"                                |
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


#ifndef P_MMI_H
#define P_MMI_H


#define CDG_ENTER__P_MMI_H

#define CDG_ENTER__FILENAME _P_MMI_H
#define CDG_ENTER__P_MMI_H__FILE_TYPE CDGINC
#define CDG_ENTER__P_MMI_H__LAST_MODIFIED _2004_08_20
#define CDG_ENTER__P_MMI_H__ID_AND_VERSION _6147_113_97_103

#define CDG_ENTER__P_MMI_H__SRC_FILE_TIME _Thu_Nov_29_09_46_32_2007

#include "CDG_ENTER.h"

#undef CDG_ENTER__P_MMI_H

#undef CDG_ENTER__FILENAME


#include "p_mmi.val"

#ifndef __T_attrib__
#define __T_attrib__
/*
 * Attribute
 * CCDGEN:WriteStruct_Count==1849
 */
typedef struct
{
  U16                       content;                  /*<  0:  2> content type                                       */
  U16                       control;                  /*<  2:  2> control                                            */
} T_attrib;
#endif


/*
 * End of substructure section, begin of primitive definition section
 */

#ifndef __T_MMI_KEYPAD_IND__
#define __T_MMI_KEYPAD_IND__
/*
 * 
 * CCDGEN:WriteStruct_Count==1852
 */
typedef struct
{
  U8                        key_code;                 /*<  0:  1> Keypad Code                                        */
  U8                        key_stat;                 /*<  1:  1> Key Status                                         */
  U8                        _align0;                  /*<  2:  1> alignment                                          */
  U8                        _align1;                  /*<  3:  1> alignment                                          */
} T_MMI_KEYPAD_IND;
#endif

#ifndef __T_MMI_AUDIO_INPUT_REQ__
#define __T_MMI_AUDIO_INPUT_REQ__
/*
 * 
 * CCDGEN:WriteStruct_Count==1853
 */
typedef struct
{
  U8                        volume;                   /*<  0:  1> volume in percent                                  */
  U8                        _align0;                  /*<  1:  1> alignment                                          */
  U8                        _align1;                  /*<  2:  1> alignment                                          */
  U8                        _align2;                  /*<  3:  1> alignment                                          */
} T_MMI_AUDIO_INPUT_REQ;
#endif

#ifndef __T_MMI_AUDIO_OUTPUT_REQ__
#define __T_MMI_AUDIO_OUTPUT_REQ__
/*
 * 
 * CCDGEN:WriteStruct_Count==1854
 */
typedef struct
{
  U8                        volume;                   /*<  0:  1> volume in percent                                  */
  U8                        _align0;                  /*<  1:  1> alignment                                          */
  U8                        _align1;                  /*<  2:  1> alignment                                          */
  U8                        _align2;                  /*<  3:  1> alignment                                          */
} T_MMI_AUDIO_OUTPUT_REQ;
#endif

#ifndef __T_MMI_SPEECH_MODE_REQ__
#define __T_MMI_SPEECH_MODE_REQ__
/*
 * 
 * CCDGEN:WriteStruct_Count==1855
 */
typedef struct
{
  U8                        speech_stat;              /*<  0:  1> Speech Status                                      */
  U8                        ids_mode;                 /*<  1:  1> IDS mode                                           */
  U8                        _align0;                  /*<  2:  1> alignment                                          */
  U8                        _align1;                  /*<  3:  1> alignment                                          */
} T_MMI_SPEECH_MODE_REQ;
#endif

#ifndef __T_MMI_AUDIO_MUTE_REQ__
#define __T_MMI_AUDIO_MUTE_REQ__
/*
 * 
 * CCDGEN:WriteStruct_Count==1856
 */
typedef struct
{
  U8                        mute_stat;                /*<  0:  1> Mute Status                                        */
  U8                        _align0;                  /*<  1:  1> alignment                                          */
  U8                        _align1;                  /*<  2:  1> alignment                                          */
  U8                        _align2;                  /*<  3:  1> alignment                                          */
} T_MMI_AUDIO_MUTE_REQ;
#endif

#ifndef __T_MMI_AUDIO_TONE_REQ__
#define __T_MMI_AUDIO_TONE_REQ__
/*
 * 
 * CCDGEN:WriteStruct_Count==1857
 */
typedef struct
{
  U8                        tone_stat;                /*<  0:  1> Tone Status                                        */
  U8                        volume;                   /*<  1:  1> volume in percent                                  */
  U8                        call_tone;                /*<  2:  1> Call tone                                          */
  U8                        _align0;                  /*<  3:  1> alignment                                          */
} T_MMI_AUDIO_TONE_REQ;
#endif

#ifndef __T_MMI_BACKLIGHT_REQ__
#define __T_MMI_BACKLIGHT_REQ__
/*
 * 
 * CCDGEN:WriteStruct_Count==1858
 */
typedef struct
{
  U8                        bl_level;                 /*<  0:  1> Backlight Level in percent                         */
  U8                        _align0;                  /*<  1:  1> alignment                                          */
  U8                        _align1;                  /*<  2:  1> alignment                                          */
  U8                        _align2;                  /*<  3:  1> alignment                                          */
} T_MMI_BACKLIGHT_REQ;
#endif

#ifndef __T_MMI_CBCH_REQ__
#define __T_MMI_CBCH_REQ__
/*
 * 
 * CCDGEN:WriteStruct_Count==1859
 */
typedef struct
{
  U16                       msg_id[MAX_IDENTS];       /*<  0: 40> CBCH message identifier                            */
  U8                        dcs_id[MAX_IDENTS];       /*< 40: 20> CBCH data coding schemes                           */
  U8                        modus;                    /*< 60:  1> CBCH use                                           */
  U8                        _align0;                  /*< 61:  1> alignment                                          */
  U8                        _align1;                  /*< 62:  1> alignment                                          */
  U8                        _align2;                  /*< 63:  1> alignment                                          */
} T_MMI_CBCH_REQ;
#endif

#ifndef __T_MMI_CBCH_IND__
#define __T_MMI_CBCH_IND__
/*
 * 
 * CCDGEN:WriteStruct_Count==1860
 */
typedef struct
{
  U8                        cbch_msg[CBCH_MSG_LEN];   /*<  0: 88> CBCH message                                       */
  U8                        cbch_len;                 /*< 88:  1> CBCH length                                        */
  U8                        _align0;                  /*< 89:  1> alignment                                          */
  U8                        _align1;                  /*< 90:  1> alignment                                          */
  U8                        _align2;                  /*< 91:  1> alignment                                          */
} T_MMI_CBCH_IND;
#endif

#ifndef __T_MMI_RXLEV_REQ__
#define __T_MMI_RXLEV_REQ__
/*
 * 
 * CCDGEN:WriteStruct_Count==1861
 */
typedef struct
{
  U8                        mode;                     /*<  0:  1> operaton mode                                      */
  U8                        no_intervalls;            /*<  1:  1> number of intervalls                               */
  U16                       period;                   /*<  2:  2> time period                                        */
} T_MMI_RXLEV_REQ;
#endif

#ifndef __T_MMI_RXLEV_IND__
#define __T_MMI_RXLEV_IND__
/*
 * 
 * CCDGEN:WriteStruct_Count==1862
 */
typedef struct
{
  U8                        rxlev;                    /*<  0:  1> Fieldstrength of the serving cell                  */
  U8                        _align0;                  /*<  1:  1> alignment                                          */
  U8                        _align1;                  /*<  2:  1> alignment                                          */
  U8                        _align2;                  /*<  3:  1> alignment                                          */
} T_MMI_RXLEV_IND;
#endif

#ifndef __T_MMI_BATTERY_REQ__
#define __T_MMI_BATTERY_REQ__
/*
 * 
 * CCDGEN:WriteStruct_Count==1863
 */
typedef struct
{
  U8                        mode;                     /*<  0:  1> operaton mode                                      */
  U8                        no_intervalls;            /*<  1:  1> number of intervalls                               */
  U16                       period;                   /*<  2:  2> time period                                        */
} T_MMI_BATTERY_REQ;
#endif

#ifndef __T_MMI_BATTERY_IND__
#define __T_MMI_BATTERY_IND__
/*
 * 
 * CCDGEN:WriteStruct_Count==1864
 */
typedef struct
{
  U8                        temp;                     /*<  0:  1> Battery Temperature                                */
  U8                        volt;                     /*<  1:  1> Battery Voltage                                    */
  U8                        _align0;                  /*<  2:  1> alignment                                          */
  U8                        _align1;                  /*<  3:  1> alignment                                          */
} T_MMI_BATTERY_IND;
#endif

#ifndef __T_MMI_DISPLAY_REQ__
#define __T_MMI_DISPLAY_REQ__
/*
 * 
 * CCDGEN:WriteStruct_Count==1865
 */
typedef struct
{
  T_attrib                  attrib;                   /*<  0:  4> Attribute                                          */
  U16                       c_y;                      /*<  4:  2> coordinate Y                                       */
  U16                       c_x;                      /*<  6:  2> coordinate X                                       */
  T_sdu                     sdu;                      /*<  8: ? > Service Data Unit                                  */
} T_MMI_DISPLAY_REQ;
#endif

#ifndef __T_MMI_SAT_CBCH_DWNLD_REQ__
#define __T_MMI_SAT_CBCH_DWNLD_REQ__
/*
 * 
 * CCDGEN:WriteStruct_Count==1866
 */
typedef struct
{
  U16                       count;                    /*<  0:  2> no. of msg id's                                    */
  U16                       msg_id[MAX_IDENTS_SAT];   /*<  2: 30> CBCH message identifier                            */
} T_MMI_SAT_CBCH_DWNLD_REQ;
#endif

#ifndef __T_MMI_SAT_CBCH_DWNLD_IND__
#define __T_MMI_SAT_CBCH_DWNLD_IND__
/*
 * 
 * CCDGEN:WriteStruct_Count==1867
 */
typedef struct
{
  U8                        cbch_msg[CBCH_MSG_LEN];   /*<  0: 88> CBCH message                                       */
  U8                        cbch_len;                 /*< 88:  1> CBCH length                                        */
  U8                        _align0;                  /*< 89:  1> alignment                                          */
  U8                        _align1;                  /*< 90:  1> alignment                                          */
  U8                        _align2;                  /*< 91:  1> alignment                                          */
} T_MMI_SAT_CBCH_DWNLD_IND;
#endif

#ifndef __T_MMI_BT_CB_NOTIFY_IND__
#define __T_MMI_BT_CB_NOTIFY_IND__
/*
 * 
 * CCDGEN:WriteStruct_Count==1868
 */
typedef struct
{
  U8                        dummy;                    /*<  0:  1> no parameters                                      */
} T_MMI_BT_CB_NOTIFY_IND;
#endif

#ifndef __T_MMI_RPD_MSG__
#define __T_MMI_RPD_MSG__
/*
 * 
 * CCDGEN:WriteStruct_Count==1869
 */
typedef struct
{
  U16                       rpd_msg_id;               /*<  0:  2> Basic Element                                      */
  U8                        rpd_msg[MAX_RPD_MSG_LEN]; /*<  2:120> Basic Element                                      */
  U8                        _align0;                  /*<122:  1> alignment                                          */
  U8                        _align1;                  /*<123:  1> alignment                                          */
} T_MMI_RPD_MSG;
#endif

#ifndef __T_MMI_GIL_IND__
#define __T_MMI_GIL_IND__
/*
 * 
 * CCDGEN:WriteStruct_Count==1870
 */
typedef struct
{
  U32                       gil_cb;                   /*<  0:  4> Function callback                                  */
  U32                       gil_data;                 /*<  4:  4> Message data                                       */
} T_MMI_GIL_IND;
#endif

#ifndef __T_MMI_TCH_VOCODER_CFG_REQ__
#define __T_MMI_TCH_VOCODER_CFG_REQ__
/*
 * 
 * CCDGEN:WriteStruct_Count==1871
 */
typedef struct
{
  U8                        vocoder_state;            /*<  0:  1> Vocoder state                                      */
  U8                        _align0;                  /*<  1:  1> alignment                                          */
  U8                        _align1;                  /*<  2:  1> alignment                                          */
  U8                        _align2;                  /*<  3:  1> alignment                                          */
} T_MMI_TCH_VOCODER_CFG_REQ;
#endif

#ifndef __T_MMI_TCH_VOCODER_CFG_CON__
#define __T_MMI_TCH_VOCODER_CFG_CON__
/*
 * 
 * CCDGEN:WriteStruct_Count==1872
 */
typedef struct
{
  U8                        dummy;                    /*<  0:  1> no parameters                                      */
} T_MMI_TCH_VOCODER_CFG_CON;
#endif

#ifndef __T_MMI_HEADSET_IND__
#define __T_MMI_HEADSET_IND__
/*
 * 
 * CCDGEN:WriteStruct_Count==1873
 */
typedef struct
{
  U8                        headset_status;           /*<  0:  1> Headset status                                     */
  U8                        _align0;                  /*<  1:  1> alignment                                          */
  U8                        _align1;                  /*<  2:  1> alignment                                          */
  U8                        _align2;                  /*<  3:  1> alignment                                          */
} T_MMI_HEADSET_IND;
#endif

#ifndef __T_MMI_CARKIT_IND__
#define __T_MMI_CARKIT_IND__
/*
 * 
 * CCDGEN:WriteStruct_Count==1874
 */
typedef struct
{
  U8                        carkit_status;            /*<  0:  1> status                                             */
  U8                        _align0;                  /*<  1:  1> alignment                                          */
  U8                        _align1;                  /*<  2:  1> alignment                                          */
  U8                        _align2;                  /*<  3:  1> alignment                                          */
} T_MMI_CARKIT_IND;
#endif


#include "CDG_LEAVE.h"


#endif
