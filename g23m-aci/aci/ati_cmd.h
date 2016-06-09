/* 
+----------------------------------------------------------------------------- 
|  Project :  
|  Modul   :  J:\g23m-aci\aci\ati_cmd.h
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
|  Purpose :  
+----------------------------------------------------------------------------- 
*/ 
#ifndef ATI_CMD_H
#define ATI_CMD_H

/* copied from uart.h... should always be the same as there */
#ifdef FF_MULTI_PORT
#define UART_INSTANCES     FF_MULTI_PORT
#else /* FF_MULTI_PORT */
#ifdef FF_TWO_UART_PORTS
#define UART_INSTANCES                 2
#ifdef _SIMULATION_
# define UART_DATA_CHANNEL              3
#else
# define UART_DATA_CHANNEL              4
#endif
#else /* FF_TWO_UART_PORTS */
#define UART_INSTANCES                 1
#endif /* FF_TWO_UART_PORTS */
#endif /* FF_MULTI_PORT */

/*
 * SKA 2002-09-05 
 * when compiling for WIN32 allow ToDo message
 * e.g.: #pragma message( __TODO__"verbose message for engineering mode" )
 * appears during compiling as
 * Z:\g23m\condat\MS\SRC\ACI\ati_cmd.c(2560) : ToDo : verbose message for engineering mode
 */
#ifdef _SIMULATION_
#define __STR2__(x) #x
#define __STR1__(x) __STR2__(x)
#define __TODO__ __FILE__ "("__STR1__(__LINE__)") : ToDo : "
#else
#define __TODO__
#endif


typedef enum
{
    ATI_SRC_TYPE_UNKNOWN    = -1,
    ATI_SRC_TYPE_UART,
    ATI_SRC_TYPE_BLUETOOTH,
    ATI_SRC_TYPE_SAT,
    ATI_SRC_TYPE_IRDA,
    ATI_SRC_TYPE_LC,
    ATI_SRC_TYPE_TST,
    ATI_SRC_TYPE_RIV,
    ATI_SRC_TYPE_PSI
} T_ATI_SRC_TYPE;

typedef enum
{
  ATI_FAIL,
  ATI_FAIL_NO_OUTPUT,
  ATI_BUSY,
  ATI_EXCT,
  ATI_CMPL,
  ATI_CMPL_NO_OUTPUT
} T_ATI_RSLT;

#define ATI_OUTPUT_BASIC_FORMAT_START     1    /* BIT  3 - 0 */
#define ATI_OUTPUT_BASIC_TYPE_START       8    /* BIT  7 - 4 */
#define ATI_OUTPUT_ATTRIB_START         256    /* BIT 11 - 8 */
#define ATI_OUTPUT_RESPONSE_TYPES    0x1000    /* BIT 12, 13 */


typedef enum
{
  ATI_NO_OUTPUT = 0,
  ATI_BEGIN_CRLF_OUTPUT = (1 * ATI_OUTPUT_BASIC_FORMAT_START),
  ATI_END_CRLF_OUTPUT   = (2 * ATI_OUTPUT_BASIC_FORMAT_START),
  
  ATI_ECHO_OUTPUT       = (1 * ATI_OUTPUT_BASIC_TYPE_START), /* string without CR,LF              */
  ATI_NORMAL_OUTPUT     = (2 * ATI_OUTPUT_BASIC_TYPE_START), /* string ends with CR,LF            */
  ATI_CONFIRM_OUTPUT    = (4 * ATI_OUTPUT_BASIC_TYPE_START), /* string starts and ends with CR,LF */
  ATI_INDICATION_OUTPUT = (8 * ATI_OUTPUT_BASIC_TYPE_START), /* string ends with CR,LF            */

  ATI_FORCED_OUTPUT     = (1 * ATI_OUTPUT_ATTRIB_START), 
  ATI_ERROR_OUTPUT      = (2 * ATI_OUTPUT_ATTRIB_START),
  ATI_CONNECT_OUTPUT    = (4 * ATI_OUTPUT_ATTRIB_START),  /* buffer indications until back in CMD mode */

  /* the v25ter defines two types of responses */
  ATI_INFORMATION_TEXT_OUTPUT = (1 * ATI_OUTPUT_RESPONSE_TYPES),
  ATI_RESULT_CODE_OUTPUT      = (2 * ATI_OUTPUT_RESPONSE_TYPES)
} T_ATI_OUTPUT_TYPE;


/* send ATI output immediatly */
#define ATI_OUTPUT_TYPE_NORMAL  0 
/* buffer ATI output and send when confirm output received */
#define ATI_OUTPUT_TYPE_LARGE   1 

typedef enum
{
  ATI_LINE_STATE_UNKNOWN  = -1,
  ATI_LINE_STATE_DCD,
  ATI_LINE_STATE_RNG,
  ATI_LINE_STATE_OUTPUT_TYPE,    /* indication for output see above */
  ATI_LINE_STATE_START,
  ATI_LINE_STATE_END

} T_ATI_LINE_STATE_TYPE;


typedef enum
{
  ATI_UNKN_MODE = -1,
  ATI_CMD_MODE,
  ATI_DATA_MODE
} T_ATI_IO_MODE;

typedef enum
{
  CMD_IDLE  = 0,
  CMD_TYPING,
  CMD_RUNNING
} T_ATI_CMD_STATE;

typedef enum
{
  NO_BUFF  = 0,
  BUFF_TYPING,
  BUFF_RUNNING
} T_ATI_BUFF_UNS_MODE;
	
typedef struct
{
  CHAR              *output;
  T_ATI_OUTPUT_TYPE output_type;
} T_ATI_INDIC_BUFF;


typedef enum
{
  CMD_MODE = 0,
  TXT_MODE
} T_TEXT_MODE;

typedef struct
{
  char *key;                                    /* command key */
  T_ATI_RSLT (*fnc)     (char *, UBYTE srcId);  /* basic command handler */
  char *and_key;                                /* AND command key */
  T_ATI_RSLT (*and_fnc) ( char *, UBYTE srcId); /* and command handler */
} ATCommand_bas;

typedef struct
{
  CHAR *key;                       /* command key                    */
  T_ACI_AT_CMD binKey;             /* binary presentation of the cmd */
  T_ATI_RSLT (*sfnc) (char *, UBYTE srcId); /* set command handler   */
  T_ATI_RSLT (*tfnc) (char *, UBYTE srcId); /* test command handler  */
  T_ATI_RSLT (*qfnc) (char *, UBYTE srcId); /* query command handler */
  CHAR *output1;
} ATCommand;


typedef void T_ATI_RESULT_CB (UBYTE             src_id,
                              T_ATI_OUTPUT_TYPE	output_type,
                              UBYTE             *output,
                              USHORT            output_len);

typedef void T_ATI_LINE_STATE_CB (UBYTE                 src_id,
                                  T_ATI_LINE_STATE_TYPE	line_state_type,
                                  ULONG                 line_state_param);

typedef struct
{
  T_ATI_IO_MODE        mode;
  UBYTE                src_id;
  T_ATI_SRC_TYPE       src_type;
  T_ATI_RESULT_CB     *result_cb;
  T_ATI_LINE_STATE_CB *line_state_cb;
  T_ATI_CMD_STATE      cmd_state;
  T_ATI_BUFF_UNS_MODE  buff_uns_mode;
  void                *indication_buffer;
  T_TEXT_MODE          text_mode;
  T_ACI_AT_CMD         curAtCmd;
} T_ATI_SRC_PARAMS;

typedef enum
{
  ATI_EXT_PART_UNKNOWN = -1,
  ATI_EXT_PART_BEGIN,		/* beginning part of a complete line		*/
  ATI_EXT_PART_LINE,		    /* a middle part from a complete line		*/
  ATI_EXT_PART_LAST,		    /* the last part from a complete line	*/
  ATI_EXT_CMPL_LINE			 /* a complete line			*/
} T_ATI_EXT_FORMAT;

EXTERN BOOL search_ati_src_id (UBYTE src_id, void *elem);
EXTERN void init_ati (void);
EXTERN BOOL ati_is_src_type( UBYTE srcId, T_ATI_SRC_TYPE source_type );
/*
 *
 * AT-Command interpreter interface functions
 *
 */

EXTERN UBYTE ati_init (T_ATI_SRC_TYPE         src_type,
                       T_ATI_RESULT_CB			  *result_cb,
                       T_ATI_LINE_STATE_CB    *line_state_cb);

EXTERN void ati_finit (UBYTE src_id);

EXTERN T_ATI_RSLT ati_execute (UBYTE src_id, UBYTE *chars, USHORT len);

EXTERN BOOL ati_execute_config_cmd (UBYTE *chars, USHORT len);

EXTERN BOOL ati_abort (UBYTE src_id);

EXTERN T_ATI_RSLT map_aci_2_ati_rslt (T_ACI_RETURN rslt);
EXTERN void trace_run_cmd_line (char *prefix, UBYTE src_id, char *cmd_name, char *cmd_params);

EXTERN T_ACI_AT_CMD get_command_id (CHAR *command_str);
EXTERN T_ATI_RSLT sEXT_Output (UBYTE	src_id, T_ATI_EXT_FORMAT output_format,	CHAR	*output);
EXTERN T_ATI_RSLT sEXT_Indication (UBYTE src_id, CHAR *indication_string);
EXTERN T_ATI_RSLT sEXT_Error	(UBYTE src_id, T_ACI_CME_ERR err);
EXTERN T_ATI_RSLT sEXT_Confirm (UBYTE src_id);
EXTERN T_ATI_RSLT sEXT_Init (CHAR *cmd_list[]);
EXTERN T_ATI_RSLT sEXT_Finit ();



#if defined (SMS_PDU_SUPPORT)
EXTERN T_ATI_RSLT   atPlusCMGSPdu ( char            * cl, UBYTE srcId);
EXTERN T_ATI_RSLT   atPlusCMGWPdu ( char            * cl, UBYTE srcId);
EXTERN T_ATI_RSLT   atPlusCMGCPdu ( char            * cl, UBYTE srcId);
EXTERN T_ATI_RSLT   atPlusCNMAPdu ( char            * cl, UBYTE srcId);
#endif
#endif /* ATI_CMD_H */
