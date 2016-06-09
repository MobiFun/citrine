/*
+--------------------------------------------------------------------+
/*
+-------------------------------------------------------------------+
| PROJECT: GSM-F&D (8411)               $Workfile:: cst_pei.c       $|
| $Author:: Be $ CONDAT GmbH            $Revision:: 14              $|
| CREATED: 01.02.99                     $Modtime:: 14.02.00 12:14   $|
| STATE  : code                                                      |
+--------------------------------------------------------------------+

   MODULE  : CST_PEI

   PURPOSE : This Modul defines the process body interface for the
             component CST of the mobile station.

   EXPORT  : pei_create    - Create the Protocol Stack Entity
             pei_init      - Initialize Protocol Stack Entity
             pei_primitive - Process Primitive
             pei_timeout   - Process Timeout
             pei_exit      - Close resources and terminate
             pei_run       - Process Primitive
             pei_config    - Dynamic Configuration
             pei_monitor   - Monitoring of physical Parameters
*/

#ifndef CST_PEI_C
#define CST_PEI_C
#endif

#include "config.h"
#include "fixedconf.h"
#include "condat-features.h"

#define ENTITY_CST

/*==== INCLUDES ===================================================*/

#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include "typedefs.h"
#include "vsi.h"
#include "custom.h"
#include "gsm.h"
#include "prim.h"
#include "cnf_cst.h"
#include "cus_cst.h"
#include "mon_cst.h"
#include "tok.h"
#include "p_cst.h"
#include "cst.h"
#include "rx.h"
#include "audio.h"

/*==== CONSTANTS ==================================================*/

// token processing
#define ERROR     -1
#define EOS        1
#define COMMA      2
#define EQUAL      3
#define IDENT      4
#define STRING     5
#define LBRACK     6
#define RBRACK     7

/*==== EXPORT =====================================================*/
extern void CST_stack_trace(void);
int cst_gsm_parameters(TOK_DCB *dcb);
extern void l1_cst_l1_parameters(char*); 
extern void csf_adc_start (UBYTE tx_flag, UBYTE traffic_period, UBYTE idle_period);
extern void spi_adc_on (void); 

GLOBAL T_HANDLE t_adc_timer    = VSI_ERROR;
GLOBAL T_HANDLE hCommPL        = VSI_ERROR;    /* PL Communication */
GLOBAL T_HANDLE hCommL1        = VSI_ERROR;    /* L1 Communication */
       T_HANDLE cst_handle;
GLOBAL UBYTE extDisplay = 0;

EXTERN UBYTE audio_is_free;
/*==== PRIVATE ====================================================*/

LOCAL void pei_not_supported (void *data);

#ifdef OPTION_RELATIVE
LOCAL ULONG offset;
#endif

/*==== VARIABLES ==================================================*/

LOCAL USHORT            t_flag = 0;
LOCAL BOOL              exit_flag = FALSE;
LOCAL BOOL              first_access = TRUE;
LOCAL T_MONITOR         cst_mon;

/*==== FUNCTIONS ==================================================*/
LOCAL const T_FUNC cst_table[] =
{
  MAK_FUNC_0 ( csf_adc_process          , CST_ADC_IND             )
};

/*==== PROTOTYPES==================================================*/

#if 1 //defined (_TMS470)

static SHORT cst_tok_gettok (TOK_DCB *dcb, char ** token);
static SHORT cst_tok_value (TOK_DCB *dcb, char * value []);
static int   cst_tokenizer_has_more_data(const TOK_DCB *dcb);
static SHORT cst_tok_issep (char c);
static void  cst_tok_init (char * s, TOK_DCB *dcb, char *buf, int buf_len);
static SHORT cst_get_values(TOK_DCB *dcb, char *value[]);
static int   cst_getbyte(TOK_DCB *dcb);

#endif

/*
+--------------------------------------------------------------------+
| PROJECT : XXX                        MODULE  : CST_PEI             |
| STATE   : code                       ROUTINE : pei_primitive       |
+--------------------------------------------------------------------+

  PURPOSE : This function processes protocol specific primitives.
*/
LOCAL SHORT pei_primitive (void * ptr)
{
  T_PRIM *prim = ptr;

  if (prim NEQ NULL)
  {
    USHORT           opc = prim->custom.opc;
    USHORT           n;
    const T_FUNC    *table;

    VSI_PPM_REC ((T_PRIM_HEADER*)prim, __FILE__, __LINE__);

    switch (opc & OPC_MASK)
    {
      case    CST_DL: table =     cst_table; n = TAB_SIZE     (cst_table); break;
      default : table = NULL; n = 0; break;
    }

    if (table != NULL )
    {
      if ((opc & PRM_MASK) < n)
      {
        table += opc & PRM_MASK;
#ifdef PALLOC_TRANSITION
        P_SDU(prim) = table->soff ? (T_sdu*) (((char*)&prim->data) + table->soff) : 0;
        P_LEN(prim) = table->size + sizeof (T_PRIM_HEADER);
#endif
        JUMP (table->func) (P2D(prim));
      }
      else
      {
        pei_not_supported (P2D(prim));
      }
      return PEI_OK;
    }

#ifdef GSM_ONLY
    PFREE (P2D(prim))
    return PEI_ERROR;
#else
    if (opc & SYS_MASK)
    {
      vsi_c_primitive (VSI_CALLER prim);
    }
    else
    {
      PFREE (P2D(prim))
      return PEI_ERROR;
    }
#endif
  }
  return PEI_OK;
}

/*
+--------------------------------------------------------------------+
| PROJECT : XXX                        MODULE  : CST_PEI             |
| STATE   : code                       ROUTINE : pei_not_supported   |
+--------------------------------------------------------------------+

  PURPOSE : This function handles an unsupported primitive.
*/
LOCAL void pei_not_supported (void *data)
{
  TRACE_FUNCTION ("pei_not_supported()");

  PFREE (data);
}


/*
+--------------------------------------------------------------------+
| PROJECT : XXX                        MODULE  : CST_PEI             |
| STATE   : code                       ROUTINE : pei_init            |
+--------------------------------------------------------------------+

  PURPOSE : This function initializes the protocol stack entity

*/

LOCAL SHORT pei_init (T_HANDLE handle)
{
  cst_handle = handle;

  TRACE_FUNCTION ("pei_init()");

  exit_flag = FALSE;

  if (hCommPL < VSI_OK)
  {
    if ((hCommPL = vsi_c_open (VSI_CALLER PL_NAME)) < VSI_OK)
    {
      return PEI_ERROR;
  	}
    }

  if (hCommL1 < VSI_OK)
  {
    if ((hCommL1 = vsi_c_open (VSI_CALLER L1_NAME)) < VSI_OK)
    {
      return PEI_ERROR;
    }
  }

  vsi_t_start (VSI_CALLER CST_ADC_TIMER, 230);  // equal to 50 TDMA frames

#if 1 //defined (_TMS470)
  // Set the sleep mode authorized for the layer1 synchronous.
  // Possible modes are :
  //          for CHIPSET = 0 : NO_SLEEP, SMALL_SLEEP, BIG_SLEEP.
  //                            It is necessary to indicate in the prototype of power_down_config()
  //                            function which clocks (amongst ARMIO_CLK, UWIRE_CLK, SIM_CLK and UART_CLK)
  //                            to be stopped in case of BIG_SLEEP mode.
  //                            With this default init, ARMIO_CLK, UWIRE_CLK, SIM_CLK and UART_CLK will be stopped
  //                            in case of BIG_SLEEP mode, but it can be changed here.

  //          other CHIPSETs  : NO_SLEEP, SMALL_SLEEP, BIG_SLEEP, DEEP_SLEEP, ALL_SLEEP.
  //                            It is necessary to indicate in the prototype of power_down_config()
  //                            function which clocks to be stopped in case of BIG_SLEEP mode,
  //                            amongst ARMIO and UWIRE clocks.
  //                            With this default init, UWIRE_CLK will always be stopped
  //                            in case of BIG_SLEEP mode, but it can be changed here.

  // The default configuration is ALL SLEEP. It can be changed in this init.

  // WARNING !!     THE SLEEP MODE SHOULD NOT BE MODIFIED ONCE THE SYSTEM INIT IS FINISHED     !!

  #if !defined (TI_VERSION)
    #if (CHIPSET == 0)
      power_down_config(NO_SLEEP, ARMIO_CLK | UWIRE_CLK | SIM_CLK | UART_CLK);
    #elif ((CHIPSET == 2) || (CHIPSET == 3) ||  \
           (CHIPSET == 5) || (CHIPSET == 6) || (CHIPSET == 7) || (CHIPSET == 8) || (CHIPSET == 10) || (CHIPSET == 11))
      power_down_config(ALL_SLEEP, UWIRE_CLK_CUT);
    #elif ((CHIPSET == 4) || (CHIPSET == 9))
        power_down_config(NO_SLEEP, UWIRE_CLK_CUT);
    #endif
  #endif
#endif

  return PEI_OK;
}

/*
+--------------------------------------------------------------------+
| PROJECT : XXX                        MODULE  : CST_PEI             |
| STATE   : code                       ROUTINE : pei_timeout         |
+--------------------------------------------------------------------+

  PURPOSE : This function processes timeouts.
*/

LOCAL SHORT pei_timeout (USHORT index)
{
  rx_timeout    (index);
  audio_timeout (index);

  if (index EQ CST_ADC_TIMER)
  {
    // start the AD conversions in layer1 synchronous
    // this function will send MMI_ADC_REQ primitive to L1A for test reason.
    // The user has to implement it by himself in the MMI.
    // The parameters are : tx_flag, traffic_period, idle_period.
    // Please refer to the ADC specification for details.

    // WCS Patch : Schedule ADC on RX and TX
    #if (OP_WCP == 1)
      csf_adc_start(1, 10, 1);    
    #else
      csf_adc_start(0, 10, 1);
    #endif
    
    spi_adc_on();
  }

  return PEI_OK;
}


/*
+--------------------------------------------------------------------+
| PROJECT : XXX                        MODULE  : CST_PEI             |
| STATE   : code                       ROUTINE : pei_exit            |
+--------------------------------------------------------------------+

  PURPOSE : This function closes all resources and terminates.
*/

LOCAL SHORT pei_exit (void)
{
  TRACE_FUNCTION ("pei_exit()");

  /*
   * clean up communication
   */

  vsi_c_close (VSI_CALLER hCommPL);
  hCommPL = VSI_ERROR;

  vsi_c_close (VSI_CALLER hCommL1);
  hCommL1 = VSI_ERROR;

  exit_flag = TRUE;

  return PEI_OK;
}

/*
+--------------------------------------------------------------------+
| PROJECT : XXX                        MODULE  : CST_PEI             |
| STATE   : code                       ROUTINE : pei_config          |
+--------------------------------------------------------------------+

  PURPOSE : Dynamic Configuration

*/

#if !defined (NCONFIG)
LOCAL KW_DATA kwtab[] = {
                   CST_CONFIG,           ID_CONFIG,
                   CST_MUTE,             ID_MUTE,
                   CST_GSM_PARAMETERS,	 ID_GSM_PARAMETERS,
                   CST_L1_PARAMETERS,    ID_L1_PARAMETERS,
                   "",                   0
       	          };

GLOBAL KW_DATA partab[] = {
                   "",                 0
       	          };
#endif


GLOBAL SHORT pei_config (T_PEI_CONFIG inString)
{
#if !defined (NCONFIG)
  char    * s = inString;
  SHORT     valno;
  USHORT    keyno;
  char    * keyw;
  char    * val [10];
  TOK_DCB   tokenizer;
  char      token_buf[80];
  SHORT     token_type;
  int       error;      /* terminate processing */

  TRACE_FUNCTION ("pei_config()");

  TRACE_FUNCTION (s);

  cst_tok_init(s, &tokenizer, &token_buf[0], sizeof(token_buf));

  /*
   * Parse next keyword and number of variables
   */
  error = 0;
  while (!error && cst_tokenizer_has_more_data(&tokenizer))
  {
    token_type = cst_tok_gettok(&tokenizer, &keyw);
    if (token_type != IDENT)
    {
      error = 1;
    }
    else
    {
      switch ((keyno = tok_key((KW_DATA *)kwtab, keyw)))
      {
        case ID_CONFIG:
          valno = cst_get_values(&tokenizer, val);
          if (valno EQ 3)
          {
            extDisplay = 1;
          }
          break;
        case ID_MUTE:
          valno = cst_get_values(&tokenizer, val);  /* eat up rest of line */
          /*
           * FTA testcase 26.6.3.4 needs the the MS is in audio free
           * environment. This config disables opening of the audio
           * path.
           */
          audio_is_free = FALSE;
          break;
        case ID_GSM_PARAMETERS:
          cst_gsm_parameters(&tokenizer);
          break;
        case ID_L1_PARAMETERS:
          /* pass parameters to L1 function for further process */
          l1_cst_l1_parameters(s); 
          valno = cst_get_values(&tokenizer, val);  /* eat up rest of line */
          break;
        default:
          TRACE_ERROR ("[PEI_CONFIG]: Illegal Keyword");
          break;
      }
    }
  }
#endif
  return PEI_OK;
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)              MODULE  : CST_PEI             |
| STATE   : code                       ROUTINE : cst_pei_config      |
+--------------------------------------------------------------------+

  PURPOSE : Dynamic Configuration

*/

GLOBAL SHORT cst_pei_config ( char * inString, char * dummy )
{
  pei_config ( inString );
}


/*
+--------------------------------------------------------------------+
| PROJECT : XXX                        MODULE  : CST_PEI             |
| STATE   : code                       ROUTINE : pei_monitor         |
+--------------------------------------------------------------------+

  PURPOSE : This function monitors physical parameters.
*/
LOCAL SHORT pei_monitor (void ** monitor)
{
  TRACE_FUNCTION ("pei_monitor()");

  cst_mon.version = VERSION_CST;

  *monitor = &cst_mon;

  return PEI_OK;
}

/*
+--------------------------------------------------------------------+
| PROJECT : XXX                        MODULE  : CST_PEI             |
| STATE   : code                       ROUTINE : pei_create          |
+--------------------------------------------------------------------+

  PURPOSE : This function creates the protocol stack entity.
*/

GLOBAL SHORT cst_pei_create (T_PEI_INFO **info)
{
  static const T_PEI_INFO pei_info =
  {
    "CST",
    {
      pei_init,
      pei_exit,
      pei_primitive,
      pei_timeout,
      NULL,             /* no signal function  */
      NULL,             /* no run function     */
      pei_config,
      pei_monitor,
    },
    1536,     /* Stack Size      */
    10,       /* Queue Entries   */
    55,       /* Priority        */
    MAX_CST_TIMER,        /* number of timer */
    0x03      /* flags           */
  };

  TRACE_FUNCTION ("pei_create()");

  /*
   *  Close Resources if open
   */

  if (first_access)
    first_access = FALSE;
  else
    pei_exit ();

  /*
   *  Export startup configuration data
   */

  *info = (T_PEI_INFO *)&pei_info;

  return PEI_OK;
}


/*
+--------------------------------------------------------------------+
| PROJECT : XXX                        MODULE  : CST_PEI             |
| STATE   : code                       ROUTINE : cst_tok_init        |
+--------------------------------------------------------------------+

   PURPOSE : Initialize token scanner.

*/
static void cst_tok_init (char * s, TOK_DCB *dcb, char *buf, int buf_len)
{
  int i;
  dcb->nexttok = dcb->tokbuf = buf;
  // can't use strcpy since it stops at 0s
  for (i=0; i<buf_len-1; i++)
  {
    dcb->tokbuf[i] = s[i];
  }
  dcb->tokbuf[buf_len-1]  = '\0';
  dcb->lastchar    = 0;
}

/*
+--------------------------------------------------------------------+
| PROJECT : XXX                        MODULE  : CST_PEI             |
| STATE   : code                       ROUTINE : cst_get_values      |
+--------------------------------------------------------------------+
*/
static SHORT cst_get_values(TOK_DCB *dcb, char *value[])
{
  char * val;  /* not identifier, so thrown away */
  /*
   * Check next token
   */
  switch (cst_tok_gettok (dcb, &val))
  {
    /*
     * No value present
     */
    case COMMA:
      return (0);
    /*
     * Value(s) follows
     */
    case EQUAL:
      return (cst_tok_value (dcb, value));
    /*
     * No value present and EOS
     */
    case EOS:
      dcb->nexttok = NULL;
      return (0);
    /*
     * Syntax error
     */
    default:
      dcb->nexttok = NULL;
      return (TOK_EOCS);
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : XXX                        MODULE  : CST_PEI             |
| STATE   : code                       ROUTINE : cst_tok_value		 |
+--------------------------------------------------------------------+
*/
static SHORT cst_tok_value (TOK_DCB *dcb, char * value [])
{
  SHORT   count;
  SHORT   tok;
  SHORT   inbrack;
  char  * val;
  char  * val2;

  inbrack = 0;
  /*
   * Get next token
   */
  tok = cst_tok_gettok (dcb, &val);

  switch (tok)
  {
    case LBRACK:
      inbrack++;
      break;

    case IDENT  :
    case STRING :
      tok = cst_tok_gettok (dcb, &val2);  /* val2 ignored since next tok */
      if ((tok == COMMA) || (tok == EOS)) /* shouldn't be an IDENT       */
      {
        /* just single value, return */
        value[0] = val;
        return (1);
      }
      else
      {
        /* error */
        dcb->nexttok = NULL;
        return (0);
      }

    case EOS :
    default  :
      dcb->nexttok = NULL;
      return (0);
  }

  /*
   * Get first token of list
   */

  tok = cst_tok_gettok (dcb, &val);
  count = 0;
  while (1)
  {
    if ((tok == IDENT) || (tok == STRING))
    {
      value[count++] = val;
    }
    else
    {
      dcb->nexttok = NULL;
      return (0);
    }

    tok = cst_tok_gettok (dcb, &val);
    switch (tok)
    {
      case COMMA:
        break;

      case RBRACK :
        if (inbrack)
        {
          if (((tok = cst_tok_gettok (dcb, &val)) == COMMA) ||
               (tok == EOS))
          {
            return (count);
          }
        }
        /*
         * Fall through
         */
      default:
        dcb->nexttok = NULL;
        return (0);
    }
    tok = cst_tok_gettok (dcb, &val);
  }
  return (0);
}

/*
+--------------------------------------------------------------------+
| PROJECT : XXX                        MODULE  : CST_PEI             |
| STATE   : code                       ROUTINE : cst_tok_gettok		 |
+--------------------------------------------------------------------+

   PURPOSE : Get list of values for token.
             Return number of values found.

             Formats:  Value
                       (Value)
                       (Value, Value,...)
                       ()
*/
static SHORT cst_tok_gettok (TOK_DCB *dcb, char ** token)
{
  SHORT   tok;
  /* if previous token was IDENT, a '\0' was placed afterwards
   * for string processing and the overwritten character was
   * placed in lastchar.  We now replace this character.
   */
  if (dcb->lastchar)
  {
    *(dcb->nexttok) = dcb->lastchar;
    dcb->lastchar = 0;
  }
  /*
   * Skip leading white space
   */
  while (isspace (*(dcb->nexttok)))
  {
    dcb->nexttok++;
  }

  * token = dcb->nexttok++;

  switch (** token)
  {
    case '\0':                         /* End of string             */
    case '\n':
      tok = EOS;
      break;

    case ',':
      ** token = '\0';
      tok = COMMA;
      break;

    case '=':
      ** token = '\0';
      tok = EQUAL;
      break;

      case '(':
      case '<':
      case '[':
        ** token = '\0';
        tok = LBRACK;
        break;

      case ')':
      case '>':
      case ']':
        ** token = '\0';
        tok = RBRACK;
        break;

      case '"':
        /*
         * Get first char of string
         */
        * token = dcb->nexttok;
        while ((*(dcb->nexttok) != '\0') && (*(dcb->nexttok) != '"'))
        {
          dcb->nexttok++;
        }

        if (*(dcb->nexttok) != '\0')
        {
          *(dcb->nexttok++) = '\0';
        }

        tok = STRING;
        break;

      default:
        /*
         * Read an identifier
         */
        while ( !cst_tok_issep (*(dcb->nexttok)) )
        {
          dcb->nexttok++;
        }

        dcb->lastchar = *(dcb->nexttok);
        *(dcb->nexttok) = '\0';

        if (*token == dcb->nexttok)
        {
          /* no characters in identifier. Error in code! */
          tok = ERROR;
        }
        else
        {
          tok = IDENT;
        }
        break;
  }
  return (tok);
}

/*
+--------------------------------------------------------------------+
| PROJECT : XXX                        MODULE  : CST_PEI             |
| STATE   : code                       ROUTINE : cst_tok_issep       |
+--------------------------------------------------------------------+

  PURPOSE : Return 1 if special character.
*/
static SHORT cst_tok_issep (char c)
{
  switch (c)
  {
    case '\0' :
    case '\n' :
    case ','  :
    case '='  :
    case '('  :
    case '<'  :
    case '['  :
    case ')'  :
    case '>'  :
    case ']'  :
    case '"'  : return (1);

    default   : return (isspace (c));
  }
}

/* if string in tokenizer state has unprocessed tokens return 1 */
/*
+--------------------------------------------------------------------+
| PROJECT : XXX                MODULE  : CST_PEI                     |
| STATE   : code               ROUTINE : cst_tokenizer_has_more_data |
+--------------------------------------------------------------------+

  PURPOSE : Return 1 if string in tokenizer state has unprocessed tokens.
*/
static int cst_tokenizer_has_more_data(const TOK_DCB *dcb)
{
  return (dcb->nexttok != NULL);
}

/*
+--------------------------------------------------------------------+
| PROJECT : XXX                        MODULE  : CST_PEI             |
| STATE   : code                       ROUTINE : cst_getbyte		 |
+--------------------------------------------------------------------+
*/

static int cst_getbyte(TOK_DCB *dcb)
{
  if (dcb->nexttok)
  {
    return *(dcb->nexttok++);
  }
  else
  {
    return -1;
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : XXX                        MODULE  : CST_PEI             |
| STATE   : code                       ROUTINE : cst_gsm_parameters	 |
+--------------------------------------------------------------------+
*/

int cst_gsm_parameters(TOK_DCB *dcb)
{
  unsigned char id;
  char str[80];
  int i;

  // STRIP FIRST TWO DELIMITERS
  {
    char *tok_string;

    if (cst_tok_gettok(dcb, &tok_string) != EQUAL)
    {
       return (0);
    }
    if (cst_tok_gettok(dcb, &tok_string) != LBRACK)
    {
       return (0);
    }
  }

  // GET CONTROL ID
  if ((id = cst_getbyte(dcb)) == -1)
  {
    vsi_o_trace("CST", 0x08, "gsm_parameters() parser error: out of bytes");
    return (0);
  }

  // STACK TRACE
  else if (id == '1')
  {
    CST_stack_trace();
  } //end else if (id == '1')

  // Crash Me
  else if (id == '2')
  {
    // No instruction before setting the reset vector
    void (*reset_vector)() = (void (*)()) 0;
    (*reset_vector)();
  } //end else if (id == '2')
#if defined (ALR)
  // Print Reg Copy
  else if (id == '3')
  {
    extern int xdump_buffer;
    int *xp = &xdump_buffer;
    int magic_word;

    // displays the 16 User mode 32bits registers saved on exception
    // vsi_o_trace("CST", 0x08, "User mode registers [r0-r15] = ...");
    for (i=0; i<4; i++)
    {
       sprintf(str, "%08x  %08x  %08x  %08x", *(xp++), *(xp++),
	                                       *(xp++), *(xp++));
       vsi_o_trace("CST", 0x08, str);
    }

    // displays the User mode CPSR saved on exception
    sprintf(str, "User mode CPSR = %08x", *(xp++));
    vsi_o_trace("CST", 0x08, str);

    // displays the magic word and the index of vector taken
    magic_word = *(xp++);
    sprintf(str, "Magic Word + Index of Vector = %08x", magic_word);
    vsi_o_trace("CST", 0x08, str);

    // displays the cause of the exception
    magic_word &= 0x000F;

    switch (magic_word) {

        case 1:
            vsi_o_trace("CST", 0x08, "Exception: Undefined Instruction");
	    break;

        case 2:
            vsi_o_trace("CST", 0x08, "Exception: Unexpected Software Interrupt");
	    break;

        case 3:
            vsi_o_trace("CST", 0x08, "Exception: Abort Prefetch");
	    break;

        case 4:
            vsi_o_trace("CST", 0x08, "Exception: Abort Data");
	    break;

        case 5:
            vsi_o_trace("CST", 0x08, "Exception: Reserved");
	    break;

        default:
	    break;
    }

    // displays the bottom 20 words of user mode stack saved on exception
    // vsi_o_trace("CST", 0x08, "Bottom 20 words of User mode stack = ...");
    for (i=0; i<5; i++)
    {
       sprintf(str, "%08x  %08x  %08x  %08x", *(xp++), *(xp++),
	                                       *(xp++), *(xp++));
       vsi_o_trace("CST", 0x08, str);
    }
  } //end else if (id == '3')

  // Clear Reg Copy
  else if (id == '4')
  {
    extern int xdump_buffer;
    int *xp = &xdump_buffer;
    // Clears the 38 32bits registers saved on exception
    for (i=0; i<38; i++)
    {
       *(xp+i) = 0;
    }
    // vsi_o_trace("CST", 0x08, "Registers Copy cleared ...");
  } //end else if (id == '4')
#endif
#if (OP_WCP == 1)
#if (WCP_PROF == 1)
  // Enable/disable MC profiler
  else if (id == '5')
  {
    PR_EnableProfiler(0);
  } 
  else if (id == '6')
  {
    PR_EnableProfiler(1);
    power_down_config(0, 0x5ff);  // disable sleep, which interferes with profiler
  } 

  // Enable/disable CPU Meter
  else if (id == '7')
  {
    PR_EnableCpuMeter(0);
  } 
  else if (id == '8')
  {
    PR_EnableCpuMeter(1);
    power_down_config(0, 0x5ff);  // disable sleep, which interferes with CPU meter
  } 
#endif
#endif

  // STRIP LAST DELIMITER
  {
    char *tok_string;

    if (cst_tok_gettok(dcb, &tok_string) != RBRACK)
    {
       return (0);
    }
  }
}  // end cst_gsm_parameters

/*
 * Reach here if unexpected event occurs (e.g. Undefined instruction, Abort,
 * out of memory); choose whether to restart or stay in datadump mode to find
 * out cause of problem
 */
void exception_handler(void)
{
   void (*jump_address)() = (void (*)()) ((unsigned)0x0);

   (*jump_address)();
}
