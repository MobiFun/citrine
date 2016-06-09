/*
+-----------------------------------------------------------------------------
|  Project :
|  Modul   :  J:\g23m-aci\aci_ext\ati_ext_mech.c
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
|  Purpose : This is the implementation of the AT command extension mechanism. Customers 
|  can implement their own handling of extension AT comannds here.
+-----------------------------------------------------------------------------
*/
#ifndef ATI_EXT_MECH_C
#define ATI_EXT_MECH_C

#include "config.h"
#include "fixedconf.h"
#include "condat-features.h"
#include "aci_conf.h"

#include "aci_all.h"
#include "aci_cmh.h"
#include "ati_cmd.h"
#include "aci_cmd.h"
#include "aci_lst.h"
#include "ati_int.h"
#include "ati_ext_mech.h"

#ifdef FF_BAT
#include "aci_bat_ext.h"
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


/*==== EXTERNALS ======================================================*/

/*==== LOCALS =========================================================*/

/*==== EXPORT ======================================================*/
EXTERN UBYTE src_id_ext; /* this source runs currently an extension command */

/*==== TYPES ======================================================*/
/*
#ifndef WIN32
  extern USHORT IQ_GetBuild(void);
  #ifndef ALR
  extern USHORT IQ_GetPoleStarVersion(void);
  #endif
  extern USHORT IQ_GetJtagId(void);
  extern USHORT IQ_GetRevision(void);

#endif*/
/*==== VARIABLES ==================================================*/
LOCAL  CHAR   extDialNum[MAX_CC_ORIG_NUM_LEN];
                         /* number to be dialled during testing    */
LOCAL  T_ACI_AT_CMD currAbrtCmd = AT_CMD_NONE;
                         /* used for indicating abort of           */
                         /* asynchronous command handling          */
LOCAL T_ACI_AT_CMD currExtCmd_v2   = AT_CMD_NONE;
                         /* used for indicating asynchronous       */
                         /* command handling                       */
                         /* identifier of the call which was       */

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (8403)       MODULE  : TIL_CSF                    |
| STATE   : code                ROUTINE : cmh_show_version           |
+--------------------------------------------------------------------+

  PURPOSE : Trace Layer 1 DSP version numbers

*/
/*
#ifndef WIN32


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

  default:

  		strcpy (chipset, "Unknown");
        break;
  }
}

*/

/*
GLOBAL void cmh_show_version (UBYTE src_id )
{
#ifndef ALR
  CHAR   buf[80];
  USHORT build, hw, rev;
#else
  CHAR   buf[160];
  CHAR   chipset[25];
#endif
  USHORT jtag;
  UCHAR  size;
  CHAR *output;*/
  /*
   * Retrieve hardware JTAG ID info
   */
/*  jtag  = IQ_GetJtagId();

#ifndef ALR

  build = IQ_GetBuild();
  hw    = IQ_GetPoleStarVersion();
  rev   = IQ_GetRevision();

  sprintf (buf, "Build %d, Silicon Revision %04X/%04X/%04X",
                 build, hw, jtag, rev);
#else


  jtagid_to_chipset (jtag, chipset);

  sprintf (buf,
           "Chipset Version:\n\r\t%s\n\rS/W Versions:\n\n\r\tTI Layer1\t\t%4X\n\r\tCondat G.2-3\t\t %3X\n\r\tTI Ref. Design Release\t %3X",
           chipset,
           SOFTWAREVERSION,
           G23VERSION,
           SYSTEMVERSION);
#endif

  // Format output as a list of Pascal-like strings
  size = strlen(buf);
  output[0] = size;
  strcpy(&(output[1]), buf);
  output[size+1] = (CHAR) 0xFF;  
  sEXT_Output (src_id, ATI_EXT_CMPL_LINE, output);
}

#endif 
*/



/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : ATI_EXT_MECH                 				   |
| STATE   : code                           ROUTINE : aci_to_ext_return_map                 		   |
+--------------------------------------------------------------------+

  PURPOSE : This function is used to map the ACI return values to the EXT return values
*/
LOCAL T_ATI_EXT_RETURN aci_to_ext_return_map (T_ACI_RETURN aci_return)
{
  TRACE_FUNCTION ("aci_to_ext_return_map ()");
  switch (aci_return)
  {
    case AT_FAIL: return ATI_EXT_FAIL;
    case AT_CMPL: return ATI_EXT_CMPL;
    case AT_EXCT: return ATI_EXT_EXCT;
    case AT_BUSY: return ATI_EXT_BUSY;
    default: return ATI_EXT_FAIL;
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : ATI_EXT_MECH                 				   |
| STATE   : code                           ROUTINE : aci_to_ext_return_map                 		   |
+--------------------------------------------------------------------+

  PURPOSE : This function is used to map the ATI return values to the EXT return values
*/
LOCAL T_ATI_EXT_RETURN ati_to_ext_return_map (T_ATI_RSLT ati_return)
{
  TRACE_FUNCTION ("ati_to_ext_return_map ()");
  switch (ati_return)
  {
    case ATI_FAIL: return ATI_EXT_FAIL;
    case ATI_CMPL: return ATI_EXT_CMPL;
    case ATI_EXCT: return ATI_EXT_EXCT;
    case ATI_BUSY: return ATI_EXT_BUSY;
    default: return ATI_EXT_FAIL;
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : ATI_EXT_MECH					                |
| STATE   : code                  		  ROUTINE : ext_LeaveEXT_v2     								  |
+--------------------------------------------------------------------+

  PURPOSE : This function is called in case the extensin mechansim
            should be left finally.
*/
LOCAL void ext_LeaveEXT_v2 ( )
{
  /*indicating that no extended AT command is still in progress*/
  currExtCmd_v2 = AT_CMD_NONE;
  /*indicate end of extended command handling to the AT interpreter*/
 /* sEXT_Finit();*/
  sEXT_Indication (src_id_ext, EXT_LEAVE );
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_EXTS                 |
| STATE   : code                  ROUTINE : ext_ContinueTest_v2         |
+--------------------------------------------------------------------+

  PURPOSE : This function is called in case the AT extension
            procedure should be continued.

            <id>: identifies the specific procedure to be continued

*/
LOCAL void ext_ContinueTest_v2 ( CHAR* id )
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
      sEXT_Indication (src_id_ext, EXT_ATD);

      /*
       * indicating that an extended AT command is still in progress
       */
      currExtCmd_v2 = AT_CMD_D;
    }
    else if (rslt EQ AT_CMPL)
    {
       sEXT_Confirm (src_id_ext);
       ext_LeaveEXT_v2 ();
    }
    else
    {
      rCI_PlusCME ( AT_CMD_EXT, CME_ERR_Unknown );
      ext_LeaveEXT_v2 ();
    }
  }
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_EXTS                 |
| STATE   : code                  ROUTINE : ext_OK_v2                   |
+--------------------------------------------------------------------+

  PURPOSE : This function is called in case the positive
            result of the asynchronous command handling is available.

            <cmdId>: command identity

*/
GLOBAL void ext_OK_v2 ( T_ACI_AT_CMD cmdId )
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
      sEXT_Output (src_id_ext, ATI_EXT_CMPL_LINE, EXT_ATCOPS_START);

      /*
       * indicating that an extended AT command is still in progress
       */
      currExtCmd_v2 = AT_CMD_COPS;
    }
    else
    {
      ext_LeaveEXT_v2 ();
      sEXT_Error ( AT_CMD_EXT, CME_ERR_Unknown);
    }
  }
  else if ( cmdId EQ AT_CMD_COPS AND currAbrtCmd EQ AT_CMD_NONE )
  {
    currExtCmd_v2 = AT_CMD_NONE;

    /*
     * generate some output at the AT interface
     */
 
    sEXT_Indication (AT_CMD_EXT, EXT_ATCOPS_STOP );

#ifndef WIN32
    ext_ContinueTest_v2 ( EXT_DIAL_VOICE_PASS );
#endif
  }
  else if ( cmdId EQ AT_CMD_D AND currAbrtCmd EQ AT_CMD_NONE )
  {
    ext_LeaveEXT_v2 ();
    sEXT_Confirm( AT_CMD_EXT );
  }
  else if ( currAbrtCmd NEQ AT_CMD_NONE )
  {
    currAbrtCmd = AT_CMD_NONE;

    ext_LeaveEXT_v2 ();
    sEXT_Confirm( AT_CMD_EXT );
  }
  else
  {
    /*generate some output at the AT interface*/
    sEXT_Output(src_id_ext, ATI_EXT_CMPL_LINE, EXT_UNEXPCTD);
  }
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : ATI_EXT_MECH              					   |
| STATE   : code                  		 ROUTINE : rEXT_Init                  							   |
+--------------------------------------------------------------------+

  PURPOSE : This function is called in the ATI initialization process.
*/
GLOBAL T_ATI_EXT_RETURN rEXT_Init 	()
{
#ifdef _SIMULATION_
  static CHAR *cmd_list[] = {"$A", "$B", "%TA", NULL};
#else /*_SIMULATION_*/
  static CHAR *cmd_list[] = {NULL};
  //static CHAR *cmd_list[] = {"$A", "$B", "%TA", NULL};
#endif /*_SIMULATION_*/
  TRACE_FUNCTION ("rEXT_Init ()");
  if (sEXT_Init (cmd_list) EQ ATI_CMPL)
    return ATI_EXT_CMPL;
  else
    return ATI_EXT_FAIL;
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : ATI_EXT_MECH              					   |
| STATE   : code                  		 ROUTINE : rAT_EXT                  							   |
+--------------------------------------------------------------------+

  PURPOSE : This function is called by ATI in case of the detection of an 
                   unknown command.
            <src_id>            : source Id
            <cmd>     : remaining unparsed command string.
*/

GLOBAL T_ATI_EXT_RETURN rEXT_Execute	(UBYTE src_id, CHAR *cmd)
{
  
  TRACE_FUNCTION ("rEXT_Execute ()");

  /*example how to process the command AT$A*/
  if (*cmd EQ '$')
  {
    cmd++;

    switch (*cmd++)
    {
      T_ATI_RSLT output_rslt;
      case 'A': /*print out string "Hello World"*/
        output_rslt = ATI_FAIL;
        if (*cmd EQ '\0')
          output_rslt = sEXT_Output (src_id, ATI_EXT_CMPL_LINE, "Hello World!");

        if (*cmd EQ '=' AND *(cmd+1) EQ '0')
          output_rslt = sEXT_Output (src_id, ATI_EXT_CMPL_LINE, "AT$A is set to off.");

        if (*cmd EQ '=' AND *(cmd+1) EQ '1')
          output_rslt = sEXT_Output (src_id, ATI_EXT_CMPL_LINE, "AT$A is set to on.");

        if (*cmd EQ '=' AND *(cmd+1) EQ '?')
          output_rslt = sEXT_Output (src_id, ATI_EXT_CMPL_LINE, "$A: 0, 1");

        return (ati_to_ext_return_map (output_rslt));

      case 'B': /*print out a complete string part by part*/
        sEXT_Output(src_id, ATI_EXT_PART_BEGIN, "This is ");
        sEXT_Output(src_id, ATI_EXT_PART_LINE, "a complete ");
        sEXT_Output(src_id, ATI_EXT_PART_LAST, "line. ");
        return (ATI_EXT_CMPL);
   }
 }

  /*
   * example how to process the command AT% commands
   */
  if (*cmd EQ '%')
  {
    sEXT_Output (src_id, ATI_EXT_CMPL_LINE, EXT_ENTER);

    cmd++;

    switch (*cmd)
    {
      case 'T':
      case 't':
        /*Enables RTC or AUDIO tests */
        cmd++;
        if (!strncmp(cmd, "A", 1) || !strncmp(cmd, "a", 1))
        {
          sEXT_Output (src_id, ATI_EXT_CMPL_LINE, "Performing Audio Tests");
        }
        else
        {
          if (!strncmp(cmd, "R", 1) || !strncmp(cmd, "r", 1)) 
          {
            sEXT_Output (src_id, ATI_EXT_CMPL_LINE, "Performing RTC Tests");
          }
          else
           return( ATI_EXT_FAIL );
        }
        return( ATI_EXT_CMPL );
      default:
        return ( ATI_EXT_FAIL );
    }
  }

 else if ( *cmd EQ 'D' )
  {
    T_ACI_RETURN rslt;
    size_t sl1=strlen(cmd);
    TRACE_EVENT ("ATD in extension mechanism.");
    strncpy ( extDialNum, cmd + 1, MINIMUM ( (MAX_CC_ORIG_NUM_LEN - 1), (sl1 - 1) ) );
    extDialNum[MINIMUM ( (MAX_CC_ORIG_NUM_LEN - 1), (sl1 - 1) )] = '\0';

    rslt = sAT_PlusCFUN ( CMD_SRC_LCL, CFUN_FUN_Full, CFUN_RST_NotPresent );

    if ( rslt EQ AT_EXCT )
    {
      /*generate some output at the AT interface*/
      sEXT_Output (src_id, ATI_EXT_CMPL_LINE, EXT_ATCFUN);
      /*indicating that an extended AT command is still in progress*/
      currExtCmd_v2 = AT_CMD_CFUN;
    }
    if (rslt EQ AT_FAIL)
    {
      sEXT_Indication(src_id, EXT_LEAVE);
      sEXT_Error (src_id, CME_ERR_Unknown);
      return ATI_EXT_FAIL;
    }
    else if (rslt EQ AT_CMPL)
    {
      ext_LeaveEXT_v2 ();
      return ATI_EXT_CMPL;
    }
    return ( aci_to_ext_return_map (rslt));
  }
  return ATI_EXT_FAIL;
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : ATI_EXT_MECH                 				   |
| STATE   : code                  		  ROUTINE : rEXT_Abort                 						   |
+--------------------------------------------------------------------+

  PURPOSE : This function is called by the ATI in case of aborting a pending 
                   extension command.
*/

GLOBAL T_ATI_EXT_RETURN rEXT_Abort (UBYTE src_id)
{
  T_ACI_RETURN rslt = AT_CMPL;

  TRACE_FUNCTION ("rEXT_Abort ()");
  /* call the abort function if necessary */
  if ( currExtCmd_v2 NEQ AT_CMD_NONE )
    rslt = sAT_Abort ((T_ACI_CMD_SRC)src_id, currExtCmd_v2 );

  switch ( rslt )
  {
    case ( AT_CMPL ):
    {
      currExtCmd_v2 = AT_CMD_NONE;
      sEXT_Indication(src_id, cmdAtError(atOk));
      ext_LeaveEXT_v2();
    }
    break;

    case ( AT_EXCT ):
    {
      currExtCmd_v2 = AT_CMD_ABRT;
    }
    break;

    default:
    {
      /* do nothing */
    }
    break;
  }
  return aci_to_ext_return_map (rslt);
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : ATI_EXT_MECH                 				   |
| STATE   : code                  		  ROUTINE : rEXT_Signal                 						   |
+--------------------------------------------------------------------+

  PURPOSE : This function is called by the ATI when an extension signal is received.
*/

GLOBAL T_ATI_EXT_RETURN rEXT_Signal (T_ACI_EXT_IND *aci_ext_ind)
{
   TRACE_FUNCTION ("rEXT_Signal ()");
 /*The handling of the extension signal can be done here.*/
   sEXT_Output (CMD_SRC_ATI_5, ATI_EXT_CMPL_LINE, "This is an extension signal.");
   return ATI_EXT_CMPL;
}

#ifdef FF_BAT
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : ATI_EXT_MECH             |
| STATE   : code                  ROUTINE : rEXT_Response_BAT        |
+--------------------------------------------------------------------+

  PURPOSE : This function is called by BAT Module when customer stuff 
            is asynchronously received.
*/

GLOBAL T_ATI_EXT_RETURN rEXT_Response_BAT (UBYTE src_id, T_BAT_cmd_response *resp)
{
   return(ATI_EXT_CMPL);
}
#endif



#endif /* ATI_EXT_MECH_C */


