/*
+--------------------------------------------------------------------+
| PROJECT: GSM-F&D (8411)               $Workfile:: CST_EXTS.C      $|
| $Author:: Sa $ CONDAT GmbH            $Revision:: 7               $|
| CREATED: 03.08.99                     $Modtime:: 14.02.00 16:06   $|
| STATE  : code                                                      |
+--------------------------------------------------------------------+


   MODULE  : CST_EXTS

   PURPOSE : This Modul defines the custom specific AT commands
*/

#ifndef CST_EXTS_C
#define CST_EXTS_C
#endif

#include "config.h"
#include "fixedconf.h"
#include "condat-features.h"

#define ENTITY_CST
/*==== INCLUDES ===================================================*/

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "typedefs.h"
#include "m_sms.val"
#include "m_fac.h"
#include "p_mnsms.h"
#include "p_mmreg.h"
#include "p_mncc.h"
#include "p_mnss.h"
#include "vsi.h"
#include "gsm.h"
#include "p_cst.h"
#include "cst.h"
#include "custom.h"
#include "p_mmi.h"
#include "p_em.h"
#include "../../g23m-aci/aci/aci_cmh.h"

#if 0 //#ifdef ALR
  #include "main/sys_ver.h"
#endif

/*==== CONSTANTS ==================================================*/

#define EXT_ATD             "EXT: D, I"
#define EXT_ATCFUN          "EXT: +CFUN, I"
#define EXT_ATCOPS_START    "EXT: +COPS, I"
#define EXT_ATCOPS_STOP     "EXT: +COPS, O"
#define EXT_ENTER           "EXT: I"
#define EXT_LEAVE           "EXT: O"
#define EXT_UNEXPCTD        "EXT: E"

#define EXT_DIAL_VOICE_PASS "DVCP"

#define EXT_MAX_BUF_LEN 41

#define EXT_VOICE_DELIMITER ';'

/*==== EXPORT =====================================================*/

/*==== TYPES ======================================================*/

#ifndef WIN32
  extern USHORT IQ_GetBuild(void);
  extern USHORT IQ_GetPoleStarVersion(void);
  extern USHORT IQ_GetJtagId(void);
  extern USHORT IQ_GetRevision(void);
  extern void   l1dmacro_init_hw(void);

  extern BOOL SER_WriteConfig (char *new_config,
                               BOOL write_to_flash);
  extern BOOL SER_ImmediateSwitch (void);
#endif

LOCAL  CHAR* percentCFG        ( CHAR* cmd     );
LOCAL  CHAR* percentDBG        ( CHAR* cmd     );
LOCAL  void  ext_LeaveEXT      ( BOOL  final   );
LOCAL  void  ext_CreatePString ( CHAR* buffer,
                                 CHAR* text    );
GLOBAL void  ext_ContinueTest  ( CHAR* id      );

/*==== VARIABLES ==================================================*/

LOCAL  CHAR*  extCmd;    /* remaining unparsed command string,     */
                         /* will be needed in case of asynchronous */
                         /* command handling                       */
LOCAL  USHORT extCmdLen; /* length of command string, will be      */
                         /* needed in case of asynchronous         */
                         /* command handling                       */
LOCAL  CHAR   extBuffer[EXT_MAX_BUF_LEN];         /* outbut buffer */

LOCAL  CHAR   extDialNum[MAX_PHB_NUM_LEN];
                         /* number to be dialled during testing    */
GLOBAL SHORT  extCallId = ACI_NumParmNotPresent;
                         /* identifier of the call which was       */
                         /* set-up using AT extension mechansim    */
GLOBAL T_ACI_AT_CMD currExtCmd   = AT_CMD_NONE;
                         /* used for indicating asynchronous       */
                         /* command handling                       */
LOCAL  T_ACI_AT_CMD currAbrtCmd = AT_CMD_NONE;
                         /* used for indicating abort of           */
                         /* asynchronous command handling          */

/*==== FUNCTIONS ==================================================*/

USHORT * csf_return_adc (void);

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (8403)       MODULE  : TIL_CSF                    |
| STATE   : code                ROUTINE : cmh_show_version           |
+--------------------------------------------------------------------+

  PURPOSE : Trace Layer 1 DSP version numbers

*/

#ifndef WIN32

#if 0
static void jtagid_to_chipset (USHORT jtagid, CHAR *chipset)
{

  switch (jtagid) {

  case 0xB268:

  		strcpy (chipset, "Hercules");
        break;

  case 0xB2B5:

  		strcpy (chipset, "Ulysse 1Mbits rev. B");
        break;

  case 0xB335:

  		strcpy (chipset, "Ulysse 1Mbits rev. A");
        break;

  case 0xB334:

  		strcpy (chipset, "Ulysse 2Mbits");
        break;

  case 0xB393:

  		strcpy (chipset, "Ulysse G1");        
        break;


  case 0xB396:

  		strcpy (chipset, "Calypso rev. B");        
        break;

  case 0xB2AC:

// Samson and Calypso rev. A share the same JTAG ID.
#if (CHIPSET != 7)
  		strcpy (chipset, "Samson");
#else
  		strcpy (chipset, "Calypso rev. A");        
#endif
        break;

  case 0xB217:

  		strcpy (chipset, "Ulysse C035");        
        break;

  case 0xB496:

  		strcpy (chipset, "Calypso C035");        
        break;

  case 0xB4FB:

  		strcpy (chipset, "Calypso Lite C035");        
        break;

  default:

  		strcpy (chipset, "Unknown");
        break;
  }
}
#endif

GLOBAL void cmh_show_version (
                                      CHAR   *command,
                                      USHORT *len,
                                      CHAR   *output
                                    )
{
#if 1 //#ifndef ALR
  CHAR   buf[80];
  USHORT build, hw, rev;
#else
  CHAR   buf[160];
  CHAR   chipset[25];
#endif
  USHORT jtag;
  UCHAR  size;

  /*
   * Retrieve hardware JTAG ID info
   */
  jtag  = IQ_GetJtagId();

#if 1 //#ifndef ALR
  /*
   * Retrieve others hardware info and build from library
   */
  build = IQ_GetBuild();
  hw    = IQ_GetPoleStarVersion();
  rev   = IQ_GetRevision();

  sprintf (buf, "Build %d, Silicon Revision %04X/%04X/%04X",
                 build, hw, jtag, rev);
#else

  /*
   * Retrieve CHIPSET name from JTAG ID
   */
  jtagid_to_chipset (jtag, chipset);

  sprintf (buf,
           "Chipset Version:\n\r\t%s\n\rS/W Versions:\n\n\r\tLayer1\t%4X_%3X\n\r\tLayer2-3    %3XBERLIN_S420\n\r\tNICE\t     %3X_I64",
           chipset,
           OFFICIAL_VERSION,
           INTERNAL_VERSION,
           G23VERSION,
           SYSTEMVERSION);
#endif

  // Format output as a list of Pascal-like strings
  size = strlen(buf);
  output[0] = size;
  strcpy(&(output[1]), buf);
  output[size+1] = (CHAR) 0xFF;         // terminate list of strings
}

#endif /* #ifndef WIN32 */



/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (8403)       MODULE  : TIL_CSF                    |
| STATE   : code                ROUTINE : show_adc_conversion        |
+--------------------------------------------------------------------+

  PURPOSE : Trace Layer 1 AD conversions results

*/

GLOBAL void show_adc_conversion (CHAR   *output)
{
  USHORT * adc_conversion;
#ifdef ALR
  CHAR   buf[160];
#else
  CHAR   buf[80];
#endif
  UCHAR  size;

  adc_conversion = csf_return_adc ();
#ifdef ALR
  sprintf (buf, "ADC 0 = %x, ADC 1 = %x, ADC 2 = %x, ADC 3 = %x, ADC 4 = %x, ADC 5 = %x, ADC 6 = %x, ADC 7 = %x, ADC 8 = %x",
        *adc_conversion++, *adc_conversion++, *adc_conversion++,
        *adc_conversion++, *adc_conversion++, *adc_conversion++,
        *adc_conversion++, *adc_conversion++, *adc_conversion);

#else
  sprintf (buf, "ADC 0 = %x, ADC 1 = %x, ADC 2 = %x, ADC 3 = %x, ADC 4 = %x",
        *adc_conversion++, *adc_conversion++, *adc_conversion++,
        *adc_conversion++, *adc_conversion);
#endif

  // Format output as a list of Pascal-like strings
  size = strlen(buf);
  output[0] = size;
  strcpy(&(output[1]), buf);
  output[size+1] = (CHAR) 0xFF;         // terminate list of strings
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (8403)       MODULE  : CST_EXTS                   |
| STATE   : code                ROUTINE : AEC_Enable                 |
+--------------------------------------------------------------------+

  PURPOSE : activate the Acoustic Echo Cancelation

*/

GLOBAL void AEC_Enable (CHAR *command, CHAR *output)
{

/* -------------------------------------------------------------------------- */
/*  MMI_AEC_REQ : 0283 = Long AEC, 105 = SPENH, 187 = AEC+SPENH, 1 = STOP     */
/* 	aec_control register bits | 0  0  Sa t2|t1 g3 g2 g1|g0 e2 e1 ak|          */
/* 		bit 0 : ACK bit : set to 1 in order to warn DSP that a new command 	  */
/*              is present.                                                   */
/* 		bit 1 : enable AEC                                                    */
/* 		bit 2 : enable SPENH (= Speech Enhancement = noise reduction)         */
/* 		bit 3 : additionnal AEC gain attenuation (lsb)                        */
/* 		bit 4 : additionnal AEC gain attenuation (msb)                        */
/* 		bit 5 : additionnal SPENH gain attenuation (lsb)                      */
/* 		bit 6 : additionnal SPENH gain attenuation (msb)                      */
/* 		bit 7 : reset trigger for AEC                                         */
/* 		bit 8 : reset trigger for SPENH                                       */
/* 		bit 9 : AEC selector 0 : short AEC, 1 : long AEC                      */
/*                                                                            */
/*  for Short AEC 		 0083                								  */
/*  for long AEC     	 0283                								  */
/*  for long AEC  -6 dB  028B                								  */
/*  for long AEC  -12 dB 0293                								  */
/*  for long AEC  -18 dB 029B                								  */
/*  for SPENH   		 0105                								  */
/*  for SPENH -6 dB  	 0125                								  */
/*  for SPENH -12 dB  	 0145                								  */
/*  for SPENH -18 dB  	 0165                								  */
/*  for BOTH    		 0187                								  */
/*  for STOP ALL		 0001 (all bits reset + ACK to 1 to warn the DSP)     */
/* -------------------------------------------------------------------------- */


 command++; /* increment command pointer to point on the hexa value of the command */


 if (!strncmp(command, "0083", 4))
 {
  		output[0] = strlen ("Short AEC is active");
  		memcpy (&output[1], "Short AEC is active", 19);

  		/* end of string list */
  		output [20] = (CHAR) 0xff;
  		csf_aec_enable(0x0083);
 }

 else if (!strncmp(command, "0283", 4))
 {
   		output[0] = strlen ("Long AEC is active");
  		memcpy (&output[1], "Long AEC is active", 18);

  		/* end of string list */
  		output [19] = (CHAR) 0xff;
  		csf_aec_enable(0x0283);
 }

 else if (!strncmp(command, "028B", 4))
 {
   		output[0] = strlen ("Long AEC -6 dB is active");
  		memcpy (&output[1], "Long AEC -6 dB is active", 24);

  		/* end of string list */
  		output [25] = (CHAR) 0xff;
  		csf_aec_enable(0x028B);
 }

 else if (!strncmp(command, "0293", 4))
 {
   		output[0] = strlen ("Long AEC -12 dB is active");
  		memcpy (&output[1], "Long AEC -12 dB is active", 25);

  		/* end of string list */
  		output [26] = (CHAR) 0xff;
  		csf_aec_enable(0x0293);
 }

 else if (!strncmp(command, "029B", 4))
 {
   		output[0] = strlen ("Long AEC -18 dB is active");
  		memcpy (&output[1], "Long AEC -18 dB is active", 25);

  		/* end of string list */
  		output [26] = (CHAR) 0xff;
  		csf_aec_enable(0x029B);
 }

 else if (!strncmp(command, "0105", 4))
 {
  		output[0] = strlen ("Noise reduction is active");
  		memcpy (&output[1], "Noise reduction is active", 25);

  		/* end of string list */
  		output [26] = (CHAR) 0xff;
  		csf_aec_enable(0x0105);
 }

 else if (!strncmp(command, "0125", 4))
 {
  		output[0] = strlen ("Noise reduction -6 dB is active");
  		memcpy (&output[1], "Noise reduction -6 dB is active", 31);

  		/* end of string list */
  		output [32] = (CHAR) 0xff;
  		csf_aec_enable(0x0125);
 }

 else if (!strncmp(command, "0145", 4))
 {
  		output[0] = strlen ("Noise reduction -12 dB is active");
  		memcpy (&output[1], "Noise reduction -12 dB is active", 32);

  		/* end of string list */
  		output [33] = (CHAR) 0xff;
  		csf_aec_enable(0x0145);
 }

 else if (!strncmp(command, "0165", 4))
 {
  		output[0] = strlen ("Noise reduction -18 dB is active");
  		memcpy (&output[1], "Noise reduction -18 dB is active", 32);

  		/* end of string list */
  		output [33] = (CHAR) 0xff;
  		csf_aec_enable(0x0165);
 }

 else if (!strncmp(command, "0187", 4))
 {
  		output[0] = strlen ("Both AEC and Noise reduction are active");
  		memcpy (&output[1], "Both AEC and Noise reduction are active", 39);

  		/* end of string list */
  		output [40] = (CHAR) 0xff;
  		csf_aec_enable(0x0187);
 }

 else if (!strncmp(command, "0001", 4))
 {
  		output[0] = strlen ("AEC and Noise reduction are unactivated");
  		memcpy (&output[1], "AEC and Noise reduction are unactivated", 39);

  		/* end of string list */
  		output [40] = (CHAR) 0xff;
  		csf_aec_enable(0x0001);
 }

 else
 {
   		output[0] = strlen ("Bad AT command");
  		memcpy (&output[1], "Bad AT command", 14);

  		/* end of string list */
  		output [15] = (CHAR) 0xff;
 }

}




/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CST_EXTS                 |
| STATE   : code                  ROUTINE : rAT_EXT                  |
+--------------------------------------------------------------------+

  PURPOSE : This function is called by the interpreter part of the
            ACI in case of the detection of an unknown command.

            <cmd>     : remaining unparsed command string.
            <cmdLen>  : length of command string. This value must be
                        incremented by the amount of parsed characters
                        by this function.
            <out>     : this parameter can be used to display some
                        strings at the AT command interface.
                        The first char of one string must contain
                        the length of the following string. The
                        special length 0xff must be used to define
                        the end of the string list.
            <outLen>  : maximum length of output buffer referenced
                        by parameter <out>.

*/

GLOBAL T_ACI_RETURN rAT_EXT (
                              CHAR*   cmd,
                              USHORT* cmdLen,
                              CHAR*   out,
                              USHORT  outLen
                            )
{
  /*
   * store command string in case it will be needed later on, when
   * result code of this function is equal to AT_EXCT
   */
  extCmd    = cmd;
  extCmdLen = *cmdLen;

  /*
   * example how to send an unsolicited result code via the AT interface
   */
  ext_CreatePString ( extBuffer, EXT_ENTER );

  sAT_URC ( extBuffer );

  /*
   * example how to process the command AT%H
   */
  if (*cmd == '%')
  {
    cmd++;

    switch (*cmd)
    {
      case 'A':
       /*
        * Display AD conversions results
        */
       *cmdLen -= 2;
       show_adc_conversion(out);
       return( AT_CMPL );

      case 'C':
      case 'c':
      case 'D':
      case 'd':
      {
        CHAR* nextCmd;

        *cmdLen -= 2;

        switch ( *cmd )
        {
          case 'C':
          case 'c':
            /* dynamic configuration via AT command interface */
            nextCmd = percentCFG ( ++cmd );

            break;

          case 'D':
          case 'd':
            /* set debug pin for reset purposes */
            nextCmd = percentDBG ( ++cmd );

            break;
        }

        *out = ( CHAR ) 0xFF;

        if ( nextCmd EQ NULL )
        {
          return ( AT_FAIL );
        }
        else
        {
          *cmdLen -= ( nextCmd - cmd );

          return     ( AT_CMPL );
        }
      }

      case 'N':
       /*
        * Enables the AEC by sending a primitive to L1A
        */
       *cmdLen -= 6;
       AEC_Enable(cmd, out);
       return( AT_CMPL );

//---------------------------------------------------------------------------//
// Added by Matthieu Vanin for the test of melody E2/E1
//---------------------------------------------------------------------------//



// End
//---------------------------------------------------------------------------//
      case 'S':
      case 's':
      {
        cmd++;

        switch (*cmd) {

          case 'e':
	  case 'E':
          /* 's''e' already detected => assume the command is at%ser. */
	  {
		cmd += 3; /* Discard the following characters */
		*cmdLen -= 9;
		if (SER_WriteConfig (cmd, (BOOL) (*(cmd + 3) - '0')))
		  return( AT_CMPL );
		else
                  return( AT_FAIL );
	  }

          case 'w':
	  case 'W':
          /* 's''w' already detected => assume the command is at%switch. */
	  {
		*cmdLen -= 7;
		if (SER_ImmediateSwitch())
		  return( AT_CMPL );
		else
		  return( AT_FAIL );
	  }

          case 'l':
	  case 'L':
          /* 's''l' already detected => assume the command is at%sleep. */
	  {
	    cmd += 5; /* Discard the following characters */
	    *cmdLen -= 8;
            /*
             * Checks if the parameter is valid:
             *     0 -> NO_SLEEP
             *     1 -> SMALL_SLEEP
             *     2 -> BIG_SLEEP
             *     3 -> DEEP_SLEEP
             *     4 -> ALL_SLEEP
             */

	    if (((*cmd - '0') >= 0) && ((*cmd - '0') <= 4))
            {
              power_down_config ((UBYTE) (*cmd - '0'), UWIRE_CLK_CUT);
		  return( AT_CMPL );
            }
	    else
              return( AT_FAIL );
	  }

          default:
            *cmdLen -= 2;
            return ( AT_FAIL );

	}
      }

      case 'H':
       *cmdLen -= 2;

       /*
        * here you can perform some actions with drivers etc.
        */

       /*
        * and create some additional output at the AT interface
        * The strings:
        *"Hello"
        *""
        *"World"
        * will be displayed at the terminal.
        *
        * first string Hello
        */
       out[0] = strlen ("Hello");
       memcpy (&out[1], "Hello", 5);
       /*
        * add a spare line with an empty string
        */
       out [6] = 0;
       /*
        * second string World
        */
       out [7] = strlen ("World");
       memcpy (&out[8], "World", 5);

       /*
        * end of string list
        */
       out [13] = (CHAR) 0xff;
       return( AT_CMPL );

#ifndef WIN32
      case 'R':
      case 'r':
        *cmdLen -= 2;
        l1dmacro_init_hw();
        out[0] = 0;
        out[1] = (CHAR) 0xFF;
        return( AT_CMPL );
#endif

#ifndef WIN32
      case 'V':
      case 'v':
        *cmdLen -= 2;

        /*
         * Display version numbers
         */

        cmh_show_version  (cmd, cmdLen, out);
        return( AT_CMPL );
#endif
#if defined (ALR)
      case 'T':
      case 't':
        /*
         * Enables RTC or AUDIO tests
         */
        cmd++;
        *cmdLen -= 3;
        if (!strncmp(cmd, "A", 1) || !strncmp(cmd, "a", 1))
        {
          //audio_test_misc();
          out[0] = 22;
          memcpy (&out[1], "Performing Audio Tests", 22);
          out [23] = (CHAR) 0xff;
	}
        else
        {
          if (!strncmp(cmd, "R", 1) || !strncmp(cmd, "r", 1)) 
          {
            //rtc_test_misc();
            out[0] = 20;
            memcpy (&out[1], "Performing RTC Tests", 20);
            out [21] = (CHAR) 0xff;
          }
          else
            return( AT_FAIL );
	}
        return( AT_CMPL );
#endif
      default:
        *cmdLen -= 2;
        return ( AT_FAIL );
    }
  }
  else if ( *cmd EQ 'D' )
  {
    T_ACI_RETURN rslt;

    /*
     * this is only a test implementation. As soon as a "ATD" command
     * string is detected in the AT interpreter and the mobile is not
     * yet registered (neither limited nor full service is availbale)
     * this function is called. Then instead of dialling immediately
     * an activation of the mobile is performed in advance.
     */
    strncpy ( extDialNum, cmd + 1, MINIMUM ( MAX_PHB_NUM_LEN - 1,
                                             *cmdLen - 1 ) );
    extDialNum[MINIMUM ( MAX_PHB_NUM_LEN - 1, *cmdLen - 1 )] = '\0';

    extCmdLen = 0;
    *cmdLen   = 0;

    rslt = sAT_PlusCFUN ( CMD_SRC_LCL,
                          CFUN_FUN_Full,
                          CFUN_RST_NotPresent );

    if ( rslt EQ AT_EXCT )
    {
      /*
       * generate some output at the AT interface
       */
      ext_CreatePString ( out, EXT_ATCFUN );

      /*
       * indicating that an extended AT command is still in progress
       */
      currExtCmd = AT_CMD_CFUN;
    }
    else
    {
      ext_LeaveEXT ( TRUE );
      rCI_PlusCME ( AT_CMD_EXT, CME_ERR_Unknown );
    }

    return ( rslt );
  }
  else
    return( AT_FAIL );
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)               MODULE  : CST_EXTS           |
| STATE   : code                        ROUTINE : percentCFG         |
+--------------------------------------------------------------------+

  PURPOSE : %CFG command (enables dynamic configuration of the
            protocol stack using the AT command interface)
*/

EXTERN char *parse (char *b, char *f, ...);

GLOBAL SHORT cc_pei_config  (char * inString,
                             char * outString);
GLOBAL SHORT cst_pei_config (char * inString,
                             char * outString);
GLOBAL SHORT dl_pei_config  (char * inString,
                             char * outString);
GLOBAL SHORT mm_pei_config  (char * inString,
                             char * outString);
GLOBAL SHORT rr_pei_config  (char * inString,
                             char * outString);
GLOBAL SHORT sim_pei_config (char * inString,
                             char * outString);
GLOBAL SHORT sms_pei_config (char * inString,
                             char * outString);
GLOBAL SHORT ss_pei_config  (char * inString,
                             char * outString);
GLOBAL SHORT pl_pei_config  (char * inString,
                             char * outString);

#ifdef FAX_AND_DATA
GLOBAL SHORT fad_pei_config (char * inString,
                             char * outString);
GLOBAL SHORT l2r_pei_config (char * inString,
                             char * outString);
GLOBAL SHORT ra_pei_config  (char * inString,
                             char * outString);
GLOBAL SHORT rlp_pei_config (char * inString,
                             char * outString);
GLOBAL SHORT t30_pei_config (char * inString,
                             char * outString);
#endif

GLOBAL CHAR* percentCFG ( CHAR* cl )
{
  CHAR entity[5];
  CHAR config[40];
  CHAR dummy;

  TRACE_FUNCTION ( "percentCFG()" );

  switch ( *cl )
  {
    case('='):
    {
      cl++;

      cl = parse ( cl, "ss", ( LONG )sizeof(entity), entity,
                             ( LONG )sizeof(config), config  );

      if( !cl             OR
          *entity EQ '\0' OR
          *config EQ '\0'    )
      {
        return ( NULL );
      }

      break;
    }

    default:
    {
      return ( NULL );
    }
  }

  if      ( strcmp ( entity, CC_NAME  ) EQ 0 )
  {
    cc_pei_config  ( config, &dummy );
  }
  else if ( strcmp ( entity, CST_NAME ) EQ 0 )
  {
    cst_pei_config ( config, &dummy );
  }
  else if ( strcmp ( entity, DL_NAME  ) EQ 0 )
  {
    dl_pei_config  ( config, &dummy );
  }
  else if ( strcmp ( entity, MM_NAME  ) EQ 0 )
  {
    mm_pei_config  ( config, &dummy );
  }
  else if ( strcmp ( entity, RR_NAME  ) EQ 0 )
  {
    rr_pei_config  ( config, &dummy );
  }
  else if ( strcmp ( entity, SIM_NAME ) EQ 0 )
  {
    sim_pei_config ( config, &dummy );
  }
  else if ( strcmp ( entity, SMS_NAME ) EQ 0 )
  {
    sms_pei_config ( config, &dummy );
  }
  else if ( strcmp ( entity, SS_NAME  ) EQ 0 )
  {
    ss_pei_config  ( config, &dummy );
  }
  else if ( strcmp ( entity, PL_NAME  ) EQ 0 )
  {
    pl_pei_config  ( config, &dummy );
  }

#ifdef FAX_AND_DATA

  else if ( strcmp ( entity, FAD_NAME ) EQ 0 )
  {
    fad_pei_config ( config, &dummy );
  }
  else if ( strcmp ( entity, L2R_NAME ) EQ 0 )
  {
    l2r_pei_config ( config, &dummy );
  }
  else if ( strcmp ( entity, RA_NAME  ) EQ 0 )
  {
    ra_pei_config  ( config, &dummy );
  }
  else if ( strcmp ( entity, RLP_NAME ) EQ 0 )
  {
    rlp_pei_config ( config, &dummy );
  }
  else if ( strcmp ( entity, T30_NAME ) EQ 0 )
  {
    t30_pei_config ( config, &dummy );
  }

#endif

  else
  {
    return ( NULL );
  }

  return ( cl );
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)               MODULE  : CST_EXTS           |
| STATE   : code                        ROUTINE : percentDBG         |
+--------------------------------------------------------------------+

  PURPOSE : %RST command (simulates pressing the reset button
            of the ASample)
*/

#ifndef TRACE_PORT_TIMING
  #ifdef _TMS470
    #if (CHIPSET == 0)
      EXTERN void l1s_set_debug_pin ( UBYTE, UBYTE );
    #endif
  #endif
#endif

GLOBAL CHAR* percentDBG ( CHAR* cl )
{
  SHORT line      = 1;
  SHORT polarity  = 1;
  SHORT delay     = 650;

  TRACE_FUNCTION ( "atPercentDBG()" );

  switch ( *cl )
  {
    case( '\0' ):
    {
      break;
    }

    case('='):
    {
      cl++;

      cl = parse ( cl, "rrr", &line, &polarity, &delay );

      if( !cl                          OR
          polarity < 0 OR polarity > 1 OR
          line     < 0 OR line     > 7 OR
          delay    < 0                    )
      {
        return ( NULL );
      }

      break;
    }

    default:
    {
      return ( NULL );
    }
  }

#ifndef TRACE_PORT_TIMING
  #ifdef _TMS470
    #if (CHIPSET == 0)
#if defined (NEW_FRAME)
  vsi_t_sleep ( VSI_CALLER ( T_TIME ) delay );
#else
      vsi_t_sleep ( VSI_CALLER ( T_VSI_TVALUE ) delay );
#endif

      l1s_set_debug_pin ( ( UBYTE ) line, ( UBYTE ) polarity );
    #endif
  #endif
#endif

  return ( cl );
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_EXTS                 |
| STATE   : code                  ROUTINE : rAT_ACP                  |
+--------------------------------------------------------------------+

  PURPOSE : This function is called by the interpreter part of the
            ACI in case of aborting a pending extension command.

            <out>     : this parameter can be used to display some
                        strings at the AT command interface.
                        The first char of one string must contain
                        the length of the following string. The
                        special length 0xff must be used to define
                        the end of the string list.
            <outLen>  : maximum length of output buffer referenced
                        by parameter <out>.

*/

GLOBAL T_ACI_RETURN rAT_ACP (
                              CHAR*   out,
                              USHORT  outLen
                            )
{
  T_ACI_RETURN rslt = AT_CMPL;

  /* call the abort function if necessary */
  if ( currExtCmd NEQ AT_CMD_NONE )
    rslt = sAT_Abort (CMD_SRC_LCL, currExtCmd );

  switch ( rslt )
  {
    case ( AT_CMPL ):
    {
      currExtCmd = AT_CMD_NONE;

      ext_LeaveEXT ( TRUE );
    }
    break;

    case ( AT_EXCT ):
    {
      currExtCmd = AT_CMD_ABRT;
    }
    break;

    default:
    {
      /* do nothing */
    }
    break;
  }

  return rslt;
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_EXTS                 |
| STATE   : code                  ROUTINE : ext_OK                   |
+--------------------------------------------------------------------+

  PURPOSE : This function is called by the MMI in case the positive
            result of the asynchronous command handling is available.

            <cmdId>: command identity

*/
GLOBAL void ext_OK ( T_ACI_AT_CMD cmdId )
{
  T_ACI_RETURN rslt;

  if ( cmdId EQ AT_CMD_CFUN AND currAbrtCmd EQ AT_CMD_NONE )
  {
    rslt = sAT_PlusCOPS ( CMD_SRC_LCL,
                          COPS_MOD_Auto,
                          COPS_FRMT_NotPresent,
                          NULL );

    if ( rslt EQ AT_EXCT )
    {
      /*
       * generate some output at the AT interface
       */
      ext_CreatePString ( extBuffer, EXT_ATCOPS_START );

      sAT_URC ( extBuffer );

      /*
       * indicating that an extended AT command is still in progress
       */
      currExtCmd = AT_CMD_COPS;
    }
    else
    {
      ext_LeaveEXT ( TRUE );
      rCI_PlusCME ( AT_CMD_EXT, CME_ERR_Unknown );
    }
  }
  else if ( cmdId EQ AT_CMD_COPS AND currAbrtCmd EQ AT_CMD_NONE )
  {
    currExtCmd = AT_CMD_NONE;

    /*
     * generate some output at the AT interface
     */
    ext_CreatePString ( extBuffer, EXT_ATCOPS_STOP );

    sAT_URC ( extBuffer );

#ifndef WIN32
    ext_ContinueTest ( EXT_DIAL_VOICE_PASS );
#endif
  }
  else if ( cmdId EQ AT_CMD_D AND currAbrtCmd EQ AT_CMD_NONE )
  {
    extCallId = 1;

    ext_LeaveEXT ( TRUE );
    rCI_OK ( AT_CMD_EXT );
  }
  else if ( currAbrtCmd NEQ AT_CMD_NONE )
  {
    currAbrtCmd = AT_CMD_NONE;

    ext_LeaveEXT ( TRUE );
    rCI_OK ( AT_CMD_EXT );
  }
  else
  {
    /*
     * generate some output at the AT interface
     */
    ext_CreatePString ( extBuffer, EXT_UNEXPCTD );

    sAT_URC ( extBuffer );
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_EXTS                 |
| STATE   : code                  ROUTINE : ext_LeaveExtension       |
+--------------------------------------------------------------------+

  PURPOSE : This function is called in case the extensin mechansim
            should be left finally.

            <final>: indicates whether final result code should be
                     sent explicitly

*/
LOCAL void ext_LeaveEXT ( BOOL final )
{
  /*
   * generate some output at the AT interface
   */
  ext_CreatePString ( extBuffer, EXT_LEAVE );

  /*
   * indicating that no extended AT command is still in progress
   */
  currExtCmd = AT_CMD_NONE;

  /*
   * indicate end of extended command handling to the AT interpreter
   */
  sAT_URC ( extBuffer );

  if ( final )
  {
    sAT_FRI ( extCmdLen );
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_EXTS                 |
| STATE   : code                  ROUTINE : ext_ContinueTest         |
+--------------------------------------------------------------------+

  PURPOSE : This function is called in case the AT extension
            procedure should be continued.

            <id>: identifies the specific procedure to be continued

*/
GLOBAL void ext_ContinueTest ( CHAR* id )
{
  T_ACI_RETURN rslt;

  if ( strcmp ( id, EXT_DIAL_VOICE_PASS ) EQ 0 )
  {
    T_ACI_D_TOC callType = D_TOC_Data;

    if ( extDialNum [strlen ( extDialNum ) - 1] EQ EXT_VOICE_DELIMITER )
      callType = D_TOC_Voice;

    extDialNum[strlen ( extDialNum ) - 1] = '\0';

    rslt = sAT_Dn ( CMD_SRC_LCL,
                    extDialNum,
                    D_CLIR_OVRD_Default,
                    D_CUG_CTRL_NotPresent,
                    callType );

    if ( rslt EQ AT_EXCT )
    {
      /*
       * generate some output at the AT interface
       */
      ext_CreatePString ( extBuffer, EXT_ATD );

      sAT_URC ( extBuffer );

      /*
       * indicating that an extended AT command is still in progress
       */
      currExtCmd = AT_CMD_D;
    }
    else
    {
      ext_LeaveEXT ( TRUE );
      rCI_PlusCME ( AT_CMD_EXT, CME_ERR_Unknown );
    }
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_EXTS                 |
| STATE   : code                  ROUTINE : ext_CreatePString        |
+--------------------------------------------------------------------+

  PURPOSE :

*/
LOCAL void ext_CreatePString ( CHAR* buffer, CHAR* text )
{
  buffer[0] = strlen (text);
  memcpy (&buffer[1], text, buffer[0]);
  buffer [buffer[0]+1] = (CHAR) 0xff;
}
