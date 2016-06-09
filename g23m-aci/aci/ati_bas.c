/*
+-----------------------------------------------------------------------------
|  Project :  GSM-F&D (8411)
|  Modul   :  ATI
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
|  Purpose :  AT Command Interpreter: basic functions.
+-----------------------------------------------------------------------------
*/

#ifndef ATI_BAS_C
#define ATI_BAS_C

#include "config.h"
#include "fixedconf.h"
#include "condat-features.h"
#include "aci_conf.h"

#include "aci_all.h"

#include <ctype.h>
#include <string.h>

#include "aci_cmh.h"
#include "ati_cmd.h"
#include "aci_cmd.h"
#include "aci_io.h"
#include "aci_cmd.h"
#include "l4_tim.h"
#include "line_edit.h"
#include "aci_lst.h"

#include "pcm.h"
#include "audio.h"
#include "aci.h"
#include "rx.h"
#include "pwr.h"
#include "l4_tim.h"


#ifdef GPRS
#ifdef DTI
#include "dti.h"
#include "dti_conn_mng.h"
#include "dti_cntrl_mng.h"
#endif /* DTI */
#include "gaci.h"
#include "gaci_cmh.h"
#include "gaci_cmd.h"
#endif  /* GPRS */

#include "aci_mem.h"
#include "aci_prs.h"

#include "ati_int.h"

#ifndef _SIMULATION_
#include "../../services/ffs/ffs.h"
#endif

#ifdef FF_ATI_BAT

#include "typedefs.h"
#include "gdd.h"
#include "bat.h"

#include "ati_bat.h"

#endif /*FF_ATI_BAT*/

LOCAL T_ATI_RSLT aciPrcsPlusCG (UBYTE srcId, CHAR* cl, CHAR* ef);
LOCAL T_ATI_RSLT setaciPrcsVolLevel ( CHAR* cl, UBYTE device );
LOCAL T_ATI_RSLT tesaciPrcsVolLevel ( UBYTE srcId, CHAR* cl, UBYTE device );
LOCAL T_ATI_RSLT queaciPrcsVolLevel ( UBYTE srcId, CHAR* cl, UBYTE device );

EXTERN T_ATI_RSLT setatPlusCLIP (char *cl, UBYTE srcId);
EXTERN T_ATI_RSLT setatPlusCOLP (char *cl, UBYTE srcId);
EXTERN T_ATI_RSLT setatPlusCREG (char *cl, UBYTE srcId);
EXTERN T_ATI_RSLT setatPercentCREG (char *cl, UBYTE srcId);
EXTERN T_ATI_RSLT setatPlusCGREG (char *cl, UBYTE srcId);
EXTERN T_ATI_RSLT setatPercentCGREG (char *cl, UBYTE srcId);
EXTERN T_ATI_RSLT setatPlusCCLK (char *cl, UBYTE srcId);
EXTERN T_ATI_RSLT queatPlusCCLK (char *cl, UBYTE srcId);

/* for atAndF */
EXTERN void ati_cmd_reset(UBYTE srcId);


GLOBAL const  T_ACI_AT_CMD atw_cmd_list [] = {AT_CMD_CGREG, AT_CMD_CLIP, AT_CMD_CLIR,
                                                AT_CMD_COLP, AT_CMD_CREG, AT_CMD_ALS, AT_CMD_CGAATT,
                                                AT_CMD_P_CGREG, AT_CMD_CPI, AT_CMD_P_CREG, AT_CMD_NONE};


#define MAX_IDX_LTH (10)

#define MAX_ATW_STORED_PROFILE 2 /*profile number for AT&W*/
#define MAX_ATW_CMD_NAM_LEN 15     /*Max command name length in bytes, e.g. %CGAATT;*/
#define MAX_ONE_ATW_PRO_LEN 50      /*Max profile length of 1 command  in bytes,so far %CGAATT has the longest */
  /* profile length \CmdId\Total Len\token\1st Para length\1st para\token\2nd para length\2nd para\'\0'\ */
#define STRING 1                        /*Token: string*/
#define SIGNED_VALUE 2             /*Token: signed value*/
#define UNSIGNED_VALUE 3         /*Token: unsigned value*/
#define MAX_ATW_CHOSEN_CMD_NB  (sizeof(atw_cmd_list)/sizeof(T_ACI_AT_CMD) -1)
/*Max number of AT commands can be chosen by user, -1 because the last entry is only an ending symbol*/


/* for profile handling ATZ and AT&W */
#ifdef _SIMULATION_
LOCAL CHAR  atw_cmd_list_simu [(MAX_ATW_CMD_NAM_LEN) *( MAX_ATW_CHOSEN_CMD_NB) + 1] = {'\0'};
LOCAL UBYTE atw_profile_simu [MAX_ATW_STORED_PROFILE][(MAX_ATW_CHOSEN_CMD_NB) * (MAX_ONE_ATW_PRO_LEN)+1]={{'\0'},{'\0'}};
 /* simulates the profile arrays  for windows test*/
#endif
static UBYTE ATZprofile;

GLOBAL const DBG_Memory dbg_mem_names[] =
{
  {"PRI",  P_DBGINFO_PrimPoolPartition},
  {"MEM",  P_DBGINFO_DmemPoolPartition},
  {"DAT",  P_DBGINFO_DataPoolPartition},
  { 0,     P_DBGINFO_NotPresent       }
};

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_CMD            |
| STATE   : code                        ROUTINE : atBW               |
+--------------------------------------------------------------------+

  PURPOSE : B command (only implemented to maintain compability)
            W command (only implemented to maintain compability)
*/

GLOBAL T_ATI_RSLT atBW(char *cl, UBYTE srcId)
{
  SHORT i=0;
  TRACE_FUNCTION("atB()");

  while (*cl >= '0' AND *cl <= '9')
  {
    cl++;
    i++;
    if (i > 1)
    {
      cmdAtError(atError);
      return ATI_FAIL;
    }
  }
  return ATI_CMPL;
}



/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_CMD            |
| STATE   : code                        ROUTINE : atE                |
+--------------------------------------------------------------------+

  PURPOSE : E command (command echo setting)
      F command (data echo setting)
      L command (Speaker Volume)
      M command (Speaker Mode)
      Q command (result code supression)
      V command (verbose response setting)
      X command (Busy/Dial tone indication)
*/

GLOBAL T_ATI_RSLT atEFLMQVX(char *cl, UBYTE srcId)
{
  char  *end   = NULL;
  UBYTE value  = 0;
  CHAR  letter = *(cl - 1);
  T_LEDIT line = {0xFF, 0xFF,  0xFF, 0xFF, 0xFF}; /* init with values which say: do not change */
               /* S3      S4     S5   atE  smsEnd */
  TRACE_FUNCTION("atEFLMQVX()");

  TRACE_EVENT_P1 ("letter = %02X", letter);

  value = (UBYTE)strtol(cl, &end, 10);
  if(*end )
  {
    cmdAtError(atError);
    return ATI_FAIL;
  }

  switch(letter)
  {
  case('E'):
    if (value > 1)
    {
      cmdAtError(atError);
      return ATI_FAIL;
    }
    ati_user_output_cfg[srcId].atE = value;
    /*
     * line edit need this piece of information, as well
     */
    line.atE = ati_user_output_cfg[srcId].atE;
    ledit_set_config (srcId, line);
    return ATI_CMPL; /* return to prevent non used p at the end of this function */
  case('F'):
    break;
  case('L'):
    if (value > 3)
    {
      cmdAtError(atError);
      return ATI_FAIL;
    }
    at.s1415.atL = value;
    break;
  case('M'):
    if (value > 2)
    {
      cmdAtError(atError);
      return ATI_FAIL;
    }
    at.s1415.atM = value;
    break;
  case('Q'):
    if (value > 1)
    {
      cmdAtError(atError);
      return ATI_FAIL;
    }
    ati_user_output_cfg[srcId].atQ = value;
    break;
  case('V'):
    if (value > 1)
    {
      cmdAtError(atError);
      return ATI_FAIL;
    }
    at.s1415.atV = value;
    break;
  case('X'):
    if (value > 4)
    {
      cmdAtError(atError);
      return ATI_FAIL;
    }
    ati_user_output_cfg[srcId].atX = value;
    break;
  default:
    cmdAtError(atError);
    return ATI_FAIL;
  }

  return ATI_CMPL;
}

/*
+--------------------------------------------------------------------+
| PRIJECT : GSM-F&D (8411)              MIDULE  : ACI_CMD            |
| STATE   : code                        RIUTINE : atI                |
+--------------------------------------------------------------------+

  PURPOSE : I command (identification)
*/

GLOBAL T_ATI_RSLT atI(char *cl, UBYTE srcId)
{
  CHAR*             ef          = EF_INF0_ID;
  pcm_FileInfo_Type fileInfo;
  USHORT            value;
  USHORT            maxRecords;
  USHORT            i;

  TRACE_FUNCTION("atI()");

  parse (cl, "r", &value);
  if (!cl)
  {
    value = 0;
  }

  /* 
   * The switch case here is intended for values which are reserved for 
   * the protocol stack manufacturer and cannot be overridden by values 
   * in the PCM/FFS.
   */
  switch (value)
  {
    case 99:
      io_sendMessage(srcId, "Texas Instruments", ATI_NORMAL_OUTPUT);
      return ATI_CMPL;

    default:
      break;
  }

  if (pcm_GetFileInfo ((UBYTE*)ef, &fileInfo) NEQ DRV_OK)
  {
    cmdCmeError (CME_ERR_MemFail);
    return ATI_FAIL;
  }

  if (pcm_ReadRecord ((UBYTE*)ef, (USHORT)(value + 1), fileInfo.FileSize,
                      (UBYTE*)g_sa, &fileInfo.Version, &maxRecords) NEQ DRV_OK)
  {
    cmdCmeError (CME_ERR_MemFail);
    return ATI_FAIL;
  }

  i = 0;
  while ((UBYTE)g_sa[i] NEQ 0xFF)
  {
    i++;
  }
  g_sa[i] = '\0';
  io_sendMessage (srcId, g_sa, ATI_NORMAL_OUTPUT );

  return ATI_CMPL;
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_CMD            |
| STATE   : code                        ROUTINE : atTP               |
+--------------------------------------------------------------------+

  PURPOSE : TP command (select tone/pulse dialing < only implemented to
                        maintain compability>)
*/

GLOBAL T_ATI_RSLT atTP(char *cl, UBYTE srcId)
{
  TRACE_FUNCTION("atTP()");

  if (*cl >= '0' AND *cl <= '9')
  {
     cmdAtError(atError);
     return ATI_FAIL;
  }
  else
  {
    return ATI_CMPL;
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_CMD            |
| STATE   : code                        ROUTINE : atS                |
+--------------------------------------------------------------------+

  PURPOSE : S command (S register setting)
*/

GLOBAL T_ATI_RSLT atS(char *cl, UBYTE srcId)
{
  SHORT reg_num,reg_cont,i=0;
  char cont[4];
  T_ATI_SRC_PARAMS *src_params = find_element (ati_src_list, srcId, search_ati_src_id);
  T_LEDIT line = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF}; /* init with values, which say: do not change */
  T_ATI_RSLT retcd  = ATI_CMPL;

  TRACE_FUNCTION("atS()");

  reg_num=0;
  while (isdigit (*cl))
  {
    reg_num *= 10;
    reg_num += *cl-'0';
    ++cl;
  }

  switch (*cl)
  {
    case('?'):
    {
      cl++;
      switch (reg_num)
      {
        case 0:                               /* supported registers for read access are listed here */
        case 1:
        case 2:
        case 3:
        case 4:
        case 5:
        case 6:
        case 7:
        case 8:
        case 10: reg_cont = at.S[reg_num]; break;  /* index access register */
        case 30: reg_cont = at.S30; break;         /* direct access register */
#ifdef GPRS
        case 99: reg_cont = at.S99; break;
#endif /* GPRS */

        default:
          cmdAtError(atError);
          return ATI_FAIL;
      }

      sprintf(g_sa,"%03d", reg_cont);

      io_sendMessage(srcId, g_sa, ATI_NORMAL_OUTPUT);
      return ATI_CMPL;
    }

    case('='):
    {
      cl++;
      if(*cl EQ '?')
      {
        cl++;

        switch(reg_num)
        {
          case(0):  sprintf(g_sa,"S%d:%s",reg_num,"(0-255)"); break;
        /*case(1):*/ /* S1 register is read only !!! */
          case(2):
          case(3):
          case(4):
          case(5):  sprintf(g_sa,"S%d:%s",reg_num,"(0-127)"); break;
          case(6):  sprintf(g_sa,"S%d:%s",reg_num,"(2-10)");  break;
          case(7):  sprintf(g_sa,"S%d:%s",reg_num,"(1-255)"); break;
          case(8):  sprintf(g_sa,"S%d:%s",reg_num,"(0-255)"); break;
          case(10):
          case(30): sprintf(g_sa,"S%d:%s",reg_num,"(1-254)"); break;
#ifdef GPRS
          case(31): sprintf(g_sa,"S%d:%s",reg_num,"(0-255)"); break;
#endif /* GPRS */

          default:
            cmdAtError(atError);
            return ATI_FAIL;
        }
        io_sendMessage(srcId, g_sa, ATI_NORMAL_OUTPUT);
        return ATI_CMPL;
      }
      else
      {
        while (*cl >= '0' AND *cl <= '9' AND i < 3)
        {
          cont[i] = (*cl);
          i++;
          cl++;
        }
        if (!i)
        {
          cmdAtError(atError);
          return ATI_FAIL;
        }
        cont[i]='\0';
        reg_cont=atoi(cont);
        switch(reg_num)
        {
          case(0):
            if (reg_cont<0 OR reg_cont > 255)
            {
              cmdAtError(atError);
              return ATI_FAIL;
            }
            at.rngPrms.srcID_S0 = srcId;

#if defined (GPRS) AND defined (DTI)
           
            /*
             *  Special case of implict attach to GACI when S0 is set to
             *  Non-zero value. GSM 7.07 version 6.4.0  10.2.2.1
             */
            if ( reg_cont > 0 )
            {
              T_CGCLASS_CLASS m_class;
              T_ACI_RETURN ret;

              if (AT_CMPL NEQ qAT_PlusCGCLASS((T_ACI_CMD_SRC)srcId, &m_class))
              {
                cmdAtError(atError);
                return ATI_FAIL;
              }
              /*
               *  no GPRS attach requested if only GSM mobile
               */
              if ( CGCLASS_CLASS_CC NEQ m_class)
              {
                ret = sAT_PlusCGATT ( (T_ACI_CMD_SRC)srcId, CGATT_STATE_ATTACHED );
                switch (ret)
                {
                case (AT_CMPL):                         /*operation completed*/
                  break;
                case (AT_EXCT):
                  src_params->curAtCmd    = AT_CMD_CGATT;
                  retcd = ATI_EXCT;
                  break;
                default:
                  cmdAtError(atError);                  /*Command failed*/
                  return ATI_FAIL;
                }
              }
            }
#endif  /* GPRS */
            break;
          case(2):
            if (reg_cont<0 OR reg_cont > 127)
            {
              cmdAtError(atError);
              return ATI_FAIL;
            }
            break;
          case(3):
            if (reg_cont<0 OR reg_cont > 127)
            {
              cmdAtError(atError);
              return ATI_FAIL;
            }
            line.S3  = (UBYTE)reg_cont;
            break;
          case(4):
            if (reg_cont<0 OR reg_cont > 127)
            {
              cmdAtError(atError);
              return ATI_FAIL;
            }
            line.S4  = (UBYTE)reg_cont;
            break;
          case(5):
            if (reg_cont<0 OR reg_cont > 127)
            {
              cmdAtError(atError);
              return ATI_FAIL;
            }
            line.S5  = (UBYTE)reg_cont;
            break;
          case(6):
            if (reg_cont<2 OR reg_cont > 10)
            {
              cmdAtError(atError);
              return ATI_FAIL;
            }
            break;
          case(7):
            if (reg_cont<1 OR reg_cont > 255)
            {
              cmdAtError(atError);
              return ATI_FAIL;
            }
            break;
          case(8):
            if (reg_cont<0 OR reg_cont > 255)
            {
              cmdAtError(atError);
              return ATI_FAIL;
            }
            break;
          case(10):
            if (reg_cont<1 OR reg_cont > 254)
            {
              cmdAtError(atError);
              return ATI_FAIL;
            }
            break;
          case(30):
            if (reg_cont<0 OR reg_cont > 254)
            {
              cmdAtError(atError);
              return ATI_FAIL;
            }
            break;

#if defined (GPRS) AND defined (DTI)
          case(99):
	    if (reg_cont<0 OR reg_cont > 255)
            {
              cmdAtError(atError);
              return ATI_FAIL;
            }
            break;
#endif /* GPRS */

          default:
            cmdAtError(atError);
            return ATI_FAIL;
        }

        switch (reg_num)
        {

          case 0:                                       /* supported registers for write access are listed here */
        /*case 1: Read Only */
          case 2:
          case 3:
          case 4:
          case 5:
          case 6:
          case 7:
          case 8:
          case 10: at.S[reg_num]=(UBYTE)reg_cont; break;

          case 30: at.S30       =(UBYTE)reg_cont; break;
#if defined (GPRS) AND defined (DTI)
          case 99: at.S99       =(UBYTE)reg_cont; break;
#endif

          default:
            cmdAtError(atError);
            return ATI_FAIL;
        }


        /*
         * line edit need this piece of information, as well
         */
        ledit_set_config (srcId, line);
        break;
      }
    }
    default:
      cmdAtError(atError);
      return ATI_FAIL;
  }
  return retcd;
}



/*
  PURPOSE : this function stores the pre-chosen command list in FFS, but only for windows test.
*/
#ifdef _SIMULATION_
LOCAL T_ATI_RSLT store_command_list (void)
{
    char atw_cmd_list []= "+CGREG;+CLIP;+CLIR;+COLP;+CREG;%ALS;%CGAATT;%CGREG;%CPI;%CREG;";
    TRACE_FUNCTION ("store_command_list()");

   if ( !(strcpy (atw_cmd_list_simu, atw_cmd_list)))
   {
       cmdCmeError(CME_ERR_Unknown);
       return ( ATI_FAIL);
   }
   return (ATI_CMPL);
}
#endif /*_SIMULATION_*/


/*
  This function gets the command Id from input command name string if it is within the default listcmdList
*/
LOCAL T_ACI_AT_CMD get_command_id_in_wn_list(CHAR *command_str)
{
    int i;
    T_ACI_AT_CMD command =  AT_CMD_NONE;
    TRACE_FUNCTION ("get_command_id_in_wn_list()");

    command = get_command_id(command_str);
    for (i = 0;*(atw_cmd_list+i) NEQ AT_CMD_NONE; i ++)
    {
       if (command EQ *(atw_cmd_list+i) )
       {
          return (command);
       }
    }
    TRACE_EVENT_P1("AT&W: unrecognized command in the list: %s", command_str);
    return (AT_CMD_NONE);
}

/*
  PURPOSE : this function reads the pre-chosen command list in FFS and then builds a string
                   of all the command Ids according to the command list read.

  Parameter:
                  cmd_id_read:  To hold the list of command Ids
*/

LOCAL T_ATI_RSLT read_command_list_from_ffs (UBYTE *cmd_id_read)
{
   int i, k;
#ifndef _SIMULATION_
   int val;
#endif /*_SIMULATION_*/
   int j = 0;
   CHAR *cmdList_temp;/*to hold the command name strings read from FFS*/
   CHAR string_temp [MAX_ATW_CMD_NAM_LEN] = {'\0'};/* hold 1 command name string*/
   T_ACI_AT_CMD command =  AT_CMD_NONE;

   TRACE_FUNCTION ("read_command_list_from_ffs()");
   ACI_MALLOC (cmdList_temp, (MAX_ATW_CHOSEN_CMD_NB) *( MAX_ATW_CMD_NAM_LEN) + 1) ;

#ifndef _SIMULATION_
   if ((val = ffs_fread ("/gsm/com/cmdList", cmdList_temp,
                                 (MAX_ATW_CHOSEN_CMD_NB) *( MAX_ATW_CMD_NAM_LEN) + 1) )< 1)
   {
       TRACE_EVENT_P1("AT&W: Read command list from ffs fail. Returned val is %d", val);
#else /*!_SIMULATION_*/
   if (!(strcpy (cmdList_temp, atw_cmd_list_simu)))
   {
      TRACE_EVENT("AT&W: Read command list fail. ");
#endif /*!_SIMULATION_*/
       ACI_MFREE (cmdList_temp);
       cmdCmeError(CME_ERR_Unknown);
       return (ATI_FAIL);
   }

    /*The for loop builds the command string and then gets the corresponding command Id*/
    for (i = 0, k = 0; *(cmdList_temp+i) NEQ '\0' AND k < (int)(MAX_ATW_CHOSEN_CMD_NB); i ++)
   {
      if (*(cmdList_temp+i) NEQ ';' )
      {
         /* No ";" is met*/
         *(string_temp+j++) = *(cmdList_temp+i);
      }
      else
      {
         *(string_temp+j) = '\0';
         command = get_command_id_in_wn_list (string_temp);
         if  (command NEQ AT_CMD_NONE)  /* ignore it if unknown string met*/
         {
             *(cmd_id_read+k++) = (UBYTE)command;
         }
        j = 0;
        *string_temp = '\0';
      }
   }
   *(cmd_id_read+k) = (UBYTE)AT_CMD_NONE;
   ACI_MFREE (cmdList_temp);
   return (ATI_CMPL);
 }

/*
  PURPOSE : this function builds and stores the profile in FFS according to the command list read.
*/
LOCAL T_ATI_RSLT store_profile_to_ffs (T_ACI_CMD_SRC srcId, SHORT nProfile)
{
    int i;
#ifndef _SIMULATION_
    int val;
#endif /*_SIMULATION_*/
    int index = 1;
    int command_length = 0;
    /*to hold the content of parameters*/
#if defined (GPRS) AND defined (DTI)
    T_CGAATT_ATTACH_MODE  para1;
    T_CGAATT_DETACH_MODE  para2;
#endif  /*GPRS*/
    T_ACI_ALS_MOD   para3 = ALS_MOD_NOTPRESENT;
    T_ACI_CLIR_MOD  mode = CLIR_MOD_NotPresent;
    UBYTE *profile = NULL; /*to hold the content of profile, memory will be allocated later*/
    UBYTE cmdIds [MAX_ATW_CHOSEN_CMD_NB +1] = {'\0'};   /*to hold the list of  command Ids*/
    char ffsAddr[] = "/gsm/com/profilex"; /*address in ffs, the x depends on which profile to save*/

    TRACE_FUNCTION ("store_profile_to_ffs()");

    ffsAddr[ strlen (ffsAddr) - 1] = '0' + nProfile;

    /*before storing of profile read command list from FFS, cmdIds is used to hold the Ids of commands*/
    if ( read_command_list_from_ffs (cmdIds) NEQ ATI_CMPL)
    {
       cmdCmeError(CME_ERR_Unknown);
       return (ATI_FAIL);
    }

  /*command list is successfully read and now ready to build profile, allocate memory first*/
    ACI_MALLOC (profile, (MAX_ATW_CHOSEN_CMD_NB) * (MAX_ONE_ATW_PRO_LEN) + 1);

 /*this for_loop builds the profile*/
    for (i = 0; *(cmdIds+i) NEQ AT_CMD_NONE; i ++)
    {
       command_length = 5;
       TRACE_EVENT_P1 ("cmd id is: %d", *(cmdIds+i));
       *(profile+index++) = *(cmdIds+i);          /*stores command ID as the first element for each command*/

       switch (*(cmdIds+i))
       {
#if defined (GPRS) AND defined (DTI)
          case AT_CMD_CGREG:
            *(profile+index++) = command_length;/*total length in bytes for this command*/
            *(profile+index++) = SIGNED_VALUE;  /*token indicates the following parameter is signed*/
            *(profile+index++) = sizeof (UBYTE);    /*length of the following parameter*/
            *(profile+index++) = (UBYTE)(ati_gprs_user_output_cfg[srcId].plus_cgreg.mod_lac_cid.pres_mode);
            break;
          case AT_CMD_P_CGREG:
            *(profile+index++) = command_length;
            *(profile+index++) = SIGNED_VALUE;
            *(profile+index++) = sizeof (UBYTE);
            *(profile+index++) = (UBYTE)(ati_gprs_user_output_cfg[srcId].percent_cgreg.mod_lac_cid.pres_mode);
            break;
          case AT_CMD_CGAATT:
            qAT_PercentCGAATT(srcId, &para1, &para2);
            command_length = 8;
            *(profile+index++) = command_length;
            *(profile+index++) = SIGNED_VALUE;
            *(profile+index++) = sizeof (UBYTE);
            *(profile+index++) = (UBYTE)para1;     /*first param*/
            *(profile+index++) = SIGNED_VALUE;
            *(profile+index++) = sizeof (UBYTE);
            *(profile+index++) = (UBYTE)para2;   /*second param*/
            break;
#endif /*GPRS*/
          case AT_CMD_CLIP:
            *(profile+index++) = command_length;
            *(profile+index++) = UNSIGNED_VALUE;
            *(profile+index++) = sizeof (UBYTE);
            *(profile+index++) = ati_user_output_cfg[srcId].CLIP_stat;
            break;
          case AT_CMD_CPI:
            *(profile+index++) = command_length;
            *(profile+index++) = UNSIGNED_VALUE;
            *(profile+index++) = sizeof (UBYTE);
            *(profile+index++) = ati_user_output_cfg[srcId].CPI_stat;
            break;
          case AT_CMD_COLP:
            *(profile+index++) = command_length;
            *(profile+index++) = UNSIGNED_VALUE;
            *(profile+index++) = sizeof (UBYTE);
            *(profile+index++) = at.flags.COLP_stat;
            break;

          case AT_CMD_CLIR:
            qAT_PercentCLIR  (srcId, &mode);
            *(profile+index++) = command_length;
            *(profile+index++) = SIGNED_VALUE;
            *(profile+index++) = sizeof (UBYTE);
            *(profile+index++) = mode;
            break;
          case AT_CMD_ALS:
            qAT_PercentALS (srcId, &para3);
            *(profile+index++) = command_length;
            *(profile+index++) = SIGNED_VALUE;
            *(profile+index++) = sizeof (UBYTE);
            *(profile+index++) = (UBYTE)para3;
            break;
          case AT_CMD_CREG:
            *(profile+index++) = command_length;
            *(profile+index++) = SIGNED_VALUE;
            *(profile+index++) = sizeof (UBYTE);
            *(profile+index++) = (UBYTE)ati_user_output_cfg[srcId].creg.mod_lac_cid.pres_mode;
            break;
          case AT_CMD_P_CREG:
            *(profile+index++) = command_length;
            *(profile+index++) = SIGNED_VALUE;
            *(profile+index++) = sizeof (UBYTE);
            *(profile+index++) = (UBYTE)(ati_user_output_cfg[srcId].percent_creg.mod_lac_cid.pres_mode);
            break;

         default:
            TRACE_EVENT("Unknown AT command");
            ACI_MFREE (profile);
            cmdCmeError(CME_ERR_Unknown);
            return (ATI_FAIL);
      }
    }

   /*store the total length of the string in the first element, the max length of the profile should
       not exceed 255, otherwise the stored structure in FFS should be redefined. Param <index>
       is actually the length of the profile.
   */
   if (index > 255)
   {
       ACI_MFREE (profile);
       TRACE_EVENT("Profile size exceed 255.");
       cmdCmeError(CME_ERR_Unknown);
       return (ATI_FAIL);
   }
   profile [0] = (UBYTE)index;

#ifndef _SIMULATION_
   if((val = ffs_fwrite(ffsAddr, profile, index ) )NEQ EFFS_OK)
   {
       TRACE_EVENT_P1("Write to FFS fail. Returned value is %d", val);
       ACI_MFREE (profile);
       cmdCmeError(CME_ERR_Unknown);
       return (ATI_FAIL);
   }
 #endif  /*!_SIMULATION_*/

#ifdef _SIMULATION_
   for (i = 0;  i < index; i++)
   {
     atw_profile_simu[nProfile][i] = profile [i];
     TRACE_EVENT_P2 ("index is %d, store profile is: %d", i, atw_profile_simu[nProfile][i]);
   }
#endif  /*_SIMULATION_*/
   ACI_MFREE (profile);
   return (ATI_CMPL);
}


/*
+--------------------------------------------------------------------+
| PROJECT :                             MODULE  :                    |
| STATE   : code                        ROUTINE : ati_wn_store_params|
+--------------------------------------------------------------------+

  PURPOSE : This function is the implementation of of AT command AT&W
            which stores the profile of some prechosen commands to FFS.
*/

LOCAL T_ATI_RSLT ati_wn_store_params      ( UBYTE  srcId,   SHORT  nProfile)

{
   TRACE_FUNCTION ("ati_wn_store_params()");

/*Stores pre-chosen AT commands. For windows test purpose only.*/
#ifdef _SIMULATION_
   if (store_command_list () NEQ ATI_CMPL)
   {
      cmdCmeError(CME_ERR_Unknown);
      return (ATI_FAIL) ;
   }
#endif /*_SIMULATION_*/

/*Stores profile for chosen AT commands to FFS*/
   if (store_profile_to_ffs ((T_ACI_CMD_SRC)srcId, nProfile) NEQ ATI_CMPL)
   {
      cmdCmeError(CME_ERR_Unknown);
      return (ATI_FAIL) ;
   }
   return (ATI_CMPL);
}



/*
  PURPOSE : this function reads the profile stored in FFS.
*/

LOCAL T_ATI_RSLT read_profile_from_ffs (SHORT nProfile, UBYTE *profile_read, SHORT len_profile_read)
{
#ifndef _SIMULATION_
   int val;
#else
   int i;
#endif
   /*the last letter x depends on the profile to read*/
   char ffsAddr[] = "/gsm/com/profilex";

   TRACE_FUNCTION ("read_profile_from_ffs()");
   /*build the correct address in FFS according to nProfile*/
   ffsAddr[ strlen (ffsAddr) - 1] = '0' + nProfile;
   TRACE_EVENT_P1 ("Read address is: %s", ffsAddr);

#ifndef _SIMULATION_
   if ((val = ffs_fread (ffsAddr, profile_read, len_profile_read ))< 1)
   {
       TRACE_EVENT_P1("Read profile from FFS fail, returned val is %d", val);
       cmdCmeError(CME_ERR_Unknown);
       return (ATI_FAIL);
   }
#endif /*_SIMULATION_*/

#ifdef _SIMULATION_
   for (i = 0; i < atw_profile_simu[nProfile][0] ; i ++)
   {
      profile_read [i] = atw_profile_simu [nProfile][i];
      TRACE_EVENT_P2 ("index %d, profile_read is: %d ", i, profile_read [i]);
   }
#endif /*_SIMULATION_*/
   return (ATI_CMPL);
}

/*
+------------------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : ATI_BAS                            |
| STATE   : code                  ROUTINE : set_param_to_the_retrieved_value   |
+------------------------------------------------------------------------------+

  PURPOSE : this function handles command parameters.
*/
LOCAL T_ATI_RSLT set_param_to_the_retrieved_value(UBYTE srcId, UBYTE *string)
{
    int para = * (string+4); /*this is for functions with input param of type int*/
    CHAR para1 = * (string+4); /*this is for functions with input param of type char*/
    T_ATI_RSLT atiRet = ATI_FAIL;
    T_ACI_RETURN aciRet = AT_FAIL;

    TRACE_FUNCTION ("set_param_to_the_retrieved_value()");

    /* string[2] is always the token and string[3] always the length.
       so far we only have parameters with 1 byte length.*/
    if ( *(string+2) EQ SIGNED_VALUE AND *(string+3) EQ sizeof (UBYTE))
    {
       para = ((* (string+4) > 127) ? *(string+4) -256: *(string+4));
    }
    para1 = (CHAR)(para + '0'); /*Transform to the corresponding inern value of the corresponding character*/
    switch (*string)
    {
      case AT_CMD_CGREG:
#if defined (GPRS) AND defined (DTI)
        atiRet = setatPlusCGREG ( &para1, srcId);
#else
        atiRet = ATI_CMPL;
#endif   /*GPRS*/
        break;
     case AT_CMD_P_CGREG:
#if defined (GPRS) AND defined (DTI)
        atiRet = setatPercentCGREG (&para1, srcId);
#else
        atiRet = ATI_CMPL;
#endif /*GPRS*/
        break;
     case AT_CMD_CGAATT:
#if defined (GPRS) AND defined (DTI)
        aciRet = sAT_PercentCGAATT ((T_ACI_CMD_SRC)srcId, (T_CGAATT_ATTACH_MODE)para,
                             (T_CGAATT_DETACH_MODE)((*(string+7) > 127) ? (*(string+7) - 256 ): *(string+7)));
#else
        aciRet =  AT_CMPL;
#endif /*GPRS*/
        break;

     case AT_CMD_CLIP:
        atiRet = setatPlusCLIP ( &para1, srcId );
        break;
     case AT_CMD_CPI:
        ati_user_output_cfg[srcId].CPI_stat = string[4];
        atiRet = ATI_CMPL;
        break;
     case AT_CMD_COLP:
        atiRet = setatPlusCOLP (&para1, srcId);
        break;
     case AT_CMD_CLIR:
        aciRet = sAT_PlusCLIR ( (T_ACI_CMD_SRC)srcId,(T_ACI_CLIR_MOD)para);
        break;
     case AT_CMD_ALS:
        aciRet = sAT_PercentALS ((T_ACI_CMD_SRC)srcId,(T_ACI_ALS_MOD)para);
        break;
     case AT_CMD_CREG:
        atiRet = setatPlusCREG ( &para1, srcId);
        break;
     case AT_CMD_P_CREG:
        atiRet = setatPercentCREG ( &para1, srcId);
        break;
     default:
        break;
   }

   if (atiRet EQ ATI_FAIL AND aciRet EQ AT_FAIL )
   {
      TRACE_EVENT_P1("Set function fails for command id %d.", *string);
      cmdCmeError(CME_ERR_Unknown);
      return( ATI_FAIL );
   }
   return (ATI_CMPL);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : ATI_BAS                  |
| STATE   : code                  ROUTINE : ati_zn_retrieve_params   |
+--------------------------------------------------------------------+

  PURPOSE : This is the implementation of AT command ATZn (n>0),
            which retrieve profile from FFS.
*/
GLOBAL void ati_zn_retrieve_params ( UBYTE srcId )
{
   int index = 1;
   UBYTE *profile_read = NULL;
   UBYTE *string = NULL;
   USHORT len_prof_read = 0;

   TRACE_FUNCTION ("ati_zn_retrieve_params()");
/*
 *-------------------------------------------------------------------
 * check parameter profile
 *-------------------------------------------------------------------
 */
  if( ATZprofile < 1                      OR
      ATZprofile > MAX_ATW_STORED_PROFILE    )
  {
    return;
  }
  len_prof_read = (MAX_ATW_CHOSEN_CMD_NB) * (MAX_ONE_ATW_PRO_LEN) + 1;
  TRACE_EVENT_P1 ("length of profile read is %d", len_prof_read);
  ACI_MALLOC (profile_read, len_prof_read);

  if (read_profile_from_ffs ((SHORT)(ATZprofile - 1), profile_read, len_prof_read) NEQ ATI_CMPL)
  {
    TRACE_EVENT ("read_profile_from_ffs() fail...");
    ACI_MFREE (profile_read);
    return;
  }
/*profile has no content*/
  if (*profile_read EQ 1)
  {
     ACI_MFREE (profile_read);
     return;
  }
  do
  {
     string = profile_read + index; /*index points to the first element of a command profile, which is command id*/
     index += *(profile_read+index +1);/*the second element in the profile is profile length for the command*/
     if (set_param_to_the_retrieved_value (srcId, string) EQ ATI_FAIL)
     {
        ACI_MFREE (profile_read);
        return;
     }
  }while (index < *profile_read);/*the first element of the whole profile is the total length*/

  ACI_MFREE (profile_read);
  return;
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_CMD            |
| STATE   : code                        ROUTINE : atAndW             |
+--------------------------------------------------------------------+

  PURPOSE : &W command stores profiles to FFS
*/

GLOBAL T_ATI_RSLT atAndW (char *cl, UBYTE srcId)
{
  long nProfile = ACI_NumParmNotPresent;
  char *end;

  TRACE_FUNCTION("atAndW()");

  nProfile = strtol(cl, &end, 10);
  if(*end)
  {
    cmdAtError(atError);
    return ATI_FAIL;
  }
  /*
   *  Normally omitted value should be assumed as zero
   *  but AT&W does not support profile number zero.
   *  So an omitted profile will be assumed as 1.
   */
  if(cl EQ end)
  {
    nProfile = 1;
  }

  if( nProfile < 1 OR nProfile > MAX_ATW_STORED_PROFILE )
  {
      cmdAtError(atError);
      return ATI_FAIL;
  }

  switch (ati_wn_store_params( srcId, (SHORT) (nProfile - 1)))
  {
    case ATI_CMPL:
      return ATI_CMPL;
    default:
      cmdAtError(atError);
      return ATI_FAIL;
  }
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_CMD            |
| STATE   : code                        ROUTINE : atZ                |
+--------------------------------------------------------------------+

  PURPOSE : Z command (reset DCE and terminate any call in progress)
*/

GLOBAL T_ATI_RSLT atZ(char *cl, UBYTE srcId)
{
  char  *end                   = NULL;
  UBYTE value                  = (UBYTE)strtol(cl, &end, 10);
  T_ATI_SRC_PARAMS *src_params = find_element (ati_src_list, srcId, search_ati_src_id);

  TRACE_FUNCTION("atZ()");

  if(*end)
  {
    cmdAtError(atError);
    return ATI_FAIL;
  }

  if( value > MAX_ATW_STORED_PROFILE )
  {
    cmdAtError(atError);
    return( ATI_FAIL );
  }

  ATZprofile = value;

#ifdef FF_ATI_BAT
  {
    T_BAT_cmd_send     cmd;
    T_BAT_no_parameter dummy;
    cmd.ctrl_params     = BAT_CMD_AT_Z;
    dummy.bat_dummy     = 0xFF;
    cmd.params.ptr_at_z = &dummy;
    if( at.rngPrms.isRng EQ TRUE )
    {
      ati_stop_ring();
    }
    src_params->curAtCmd = AT_CMD_Z;

    bat_send(ati_bat_get_client(srcId), &cmd);

    return (ATI_EXCT); /* executing, because response is passed by callback function */ 
  } 
#else /* no FF_ATI_BAT */

  switch(sAT_Z( (T_ACI_CMD_SRC)srcId, 0 ))
  {
    case AT_CMPL:
      return ATI_CMPL;
    case AT_EXCT:
      if( at.rngPrms.isRng EQ TRUE )
      {
        ati_stop_ring();
      }
      src_params->curAtCmd    = AT_CMD_Z;
      return ATI_EXCT;
    default:
      cmdAtError(atError);
      return ATI_FAIL;
  }

#endif /* no FF_ATI_BAT */
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_CMD            |
| STATE   : code                        ROUTINE : atPlusCPAS         |
+--------------------------------------------------------------------+

  PURPOSE : Phone activity status
*/
GLOBAL T_ATI_RSLT setatPlusCPAS(char *cl, UBYTE srcId)
{
  T_ACI_CPAS_PAS pas = CPAS_PAS_NotPresent;
  T_ACI_RETURN ret = AT_FAIL;

  TRACE_FUNCTION("setatPlusCPAS()");

  if (*cl NEQ '\0' AND *cl NEQ ';')
  {
    cmdAtError(atError);
    return ATI_FAIL;
  }

#ifdef FF_ATI_BAT
  {
  T_BAT_cmd_send cmd;
  T_BAT_no_parameter my_bat_set_plus_cpas;
  
  TRACE_FUNCTION("setatPlusCPAS() calls bat_send() <=== as APPLICATION");
  
  ret = qAT_PlusCPAS( srcId, &pas );
  if (ret EQ AT_CMPL)
  {
    resp_disp(srcId, cl, "e", &pas);
  }
  else
  {
    cmdAtError(atError);
    return ATI_FAIL;
  }

  cmd.params.ptr_set_plus_cpas = &my_bat_set_plus_cpas;
  bat_send(ati_bat_get_client(srcId), &cmd);
  return ATI_EXCT; /* executing, because response is passed by callback function */
  }
#else /* OLD FUNCTION BODY */

  ret = qAT_PlusCPAS( (T_ACI_CMD_SRC)srcId, &pas );
  if (ret EQ AT_CMPL)
  {
    resp_disp(srcId, cl, "e", &pas);
    return ATI_CMPL;
  }
  else
  {
    cmdAtError(atError);
    return ATI_FAIL;
  }

#endif /* no FF_ATI_BAT*/
}
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_CMD            |
| STATE   : code                        ROUTINE : atPlusCLAE         |
+--------------------------------------------------------------------+

  PURPOSE : +CLAE command (Language Event)
*/

GLOBAL T_ATI_RSLT setatPlusCLAE (char *cl, UBYTE srcId)
{
  T_ACI_RETURN ret = AT_FAIL;
  T_ACI_CLAE_MOD mode = CLAE_MOD_NotPresent;

  TRACE_FUNCTION("setatPLusCLAE()");

  switch(*cl)
  {
    case('0'):
      mode=CLAE_MOD_Disable;
      break;
    case('1'):
      mode=CLAE_MOD_Enable;
      break;
    default:
      cmdCmeError(CME_ERR_OpNotAllow);
      return ATI_FAIL;
  }
  ret = sAT_PlusCLAE((T_ACI_CMD_SRC)srcId,mode);
  if (ret EQ AT_CMPL)
    return ATI_CMPL;
  else
    return ATI_FAIL;
}

GLOBAL T_ATI_RSLT queatPlusCLAE (char *cl, UBYTE srcId)
{
  char *me="+CLAE: ";
  T_ACI_RETURN ret = AT_FAIL;
  T_ACI_CLAE_MOD mode=CLAE_MOD_NotPresent;

  TRACE_FUNCTION("queatPLusCLAE()");

  ret = qAT_PlusCLAE((T_ACI_CMD_SRC)srcId,&mode);
  if(ret EQ AT_CMPL)
  {
    sprintf(g_sa,"%s%d",me,mode);
    io_sendMessage(srcId, g_sa, ATI_NORMAL_OUTPUT);
    return ATI_CMPL;
  }
  else
  {
    cmdCmeError(CME_ERR_Unknown);
    return ATI_FAIL;
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_CMD            |
| STATE   : code                        ROUTINE : atAndC             |
+--------------------------------------------------------------------+

  PURPOSE : &C command reset to factory default (not the final version)
*/


GLOBAL T_ATI_RSLT atAndC (char *cl, UBYTE srcId)
{
  T_ACI_DCD_MOD val;

  TRACE_FUNCTION("atAndC()");

  switch(*cl)
  {
    case '\0':
    case  '0':
      val = DCD_ALWAYS_ON;
      break;

    case  '1':
      val = DCD_DISABLE_AFTER_CALL;
      break;
#ifdef RMV_01_06_04
    case ('?'):
      qAT_AndC( srcId, &val );
      sprintf(g_sa, "&C: %d", val);
      io_sendMessage(srcId, g_sa, ATI_NORMAL_OUTPUT);
      return (ATI_CMPL);
#endif
    default:
      cmdAtError(atError);
      return(ATI_FAIL);
  }

#ifdef DTI
  if (sAT_AndC( (T_ACI_CMD_SRC)srcId, val ) EQ AT_CMPL)
  {
    return (ATI_CMPL);
  }
#endif
  cmdAtError(atError);
  return (ATI_FAIL);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_CMD            |
| STATE   : code                        ROUTINE : atAndD             |
+--------------------------------------------------------------------+

  PURPOSE : &D command to set the DTR behaviour
*/


GLOBAL T_ATI_RSLT atAndD (char *cl, UBYTE srcId)
{
  UBYTE val;

  TRACE_FUNCTION("atAndD()");

  switch(*cl)
  {
    case ('\0'):
    case ('0'):
      val = 0;
      break;
    case ('1'):
      val = 1;
      break;
    case ('2'):
      val = 2;
      break;
#ifdef RMV_01_06_04
    case ('?'):
      query = TRUE;
      break;
#endif
    default:
      cmdAtError(atError);
      return ATI_FAIL;
  }

#ifdef RMV_01_06_04
  if (query)
  {
    ret=qAT_AndD (srcId, &val);
    if(ret EQ AT_CMPL)
    {
      sprintf(g_sa,"&D: %d", val);
      io_sendMessage(srcId, g_sa, ATI_NORMAL_OUTPUT);
      return ATI_CMPL;
    }
  }
  else
#endif
  {
#ifdef DTI
    if (sAT_AndD ( (T_ACI_CMD_SRC)srcId, val ) EQ AT_CMPL)
    {
      return ATI_CMPL;
    }
#endif
  }
  cmdAtError(atError);
  return ATI_FAIL;
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_CMD            |
| STATE   : code                        ROUTINE : atAndF             |
+--------------------------------------------------------------------+

  PURPOSE : &F command reset to factory default (not the final version)
*/
GLOBAL T_ATI_RSLT atAndF (char *cl, UBYTE srcId)
{
  T_ACI_RETURN ret = AT_FAIL;

  TRACE_FUNCTION("atAndF()");

  switch(*cl)
  {
    case '\0':
    case ('0'):
      /* default like ATZ */
      ati_cmd_reset(srcId);
      break;
    case ('1'):
      ati_cmd_reset(srcId);

      /*
       *  An other manufacturer specific parameter set.
       *
       *  This is different from the user specific profiles (ATZ1).
       *
       */
      at.flags.COLP_stat = 1;
      ati_user_output_cfg[srcId].CR_stat = 1;
      ati_user_output_cfg[srcId].CRC_stat = 1;
      ati_user_output_cfg[srcId].CLIP_stat = 1;
      ati_user_output_cfg[srcId].DR_stat = 1;

      ati_user_output_cfg[srcId].creg.mod_lac_cid.pres_mode = CREG_MOD_ON;

#ifdef GPRS
      ati_gprs_user_output_cfg[srcId].plus_cgreg.mod_lac_cid.pres_mode = CREG_MOD_ON;
      ati_gprs_user_output_cfg[srcId].percent_cgreg.mod_lac_cid.pres_mode = CREG_MOD_ON;
#endif
      break;
    default:
      cmdAtError(atError);
      return ATI_FAIL;
  }

  /*
   *  The function sAT_AndF will be called with value 0 -> AT&F0
   *  because the different behavior of value 0 to 1 is only ATI
   *  related and sAT_AndF() is only CMH-related.
   *
   *  This is different from sAT_Z().
   */
  ret = sAT_AndF( (T_ACI_CMD_SRC)srcId, 0 );
  if (ret EQ AT_CMPL)
  {
    return ATI_CMPL;
  }
  cmdAtError(atError);
  return ATI_FAIL;

}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_CMD            |
| STATE   : code                        ROUTINE : atAndK             |
+--------------------------------------------------------------------+

  PURPOSE : &K only for compatibility
*/


GLOBAL T_ATI_RSLT atAndK (char *cl, UBYTE srcId)
{
  TRACE_FUNCTION("atAndK()");

  switch(*cl)
  {
    case ('0'):
    case ('1'):
    case ('2'):
    case ('3'):
    case ('4'):
      return ATI_CMPL;
    default:
      cmdAtError(atError);
      return ATI_FAIL;
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_CMD            |
| STATE   : code                        ROUTINE : atPlusCSQ          |
+--------------------------------------------------------------------+

  PURPOSE : +CSQ, signal quality
*/

GLOBAL T_ATI_RSLT setatPlusCSQ ( char *cl, UBYTE srcId)
{
  rx_Status_Type    rxStat;
  UBYTE             rssi;
  UBYTE             ber;

  TRACE_FUNCTION("setatPlusCSQ()");

  if ( rx_GetStatus ( &rxStat ) NEQ DRV_OK )
  {
    cmdCmeError ( CME_ERR_Unknown );
    return ATI_FAIL;
  }
  else
  {
    if ( rxStat.gsmLevel EQ 0xFF OR rxStat.gsmLevel EQ 0 )
    {
      rssi = ACI_RSSI_FAULT;
    }
    else if ( rxStat.gsmLevel > 59 )
    {
      rssi = 31;
    }
    else
    {
      rssi = ( rxStat.gsmLevel / 2 ) + 2;
    }

    if ( rxStat.rxQuality EQ RX_QUAL_UNAVAILABLE )
    {
      ber = ACI_BER_FAULT;
    }
    else
    {
      ber = rxStat.rxQuality;
    }

    sprintf (g_sa, "+CSQ: %d,%d ", rssi, ber );
    io_sendMessage(srcId, g_sa, ATI_NORMAL_OUTPUT);
    return ATI_CMPL;
  }
}

GLOBAL T_ATI_RSLT tesatPlusCSQ(CHAR *cl, UBYTE srcId)
{
  TRACE_FUNCTION("tesatPLusCSQ()");

  sprintf (g_sa, "+CSQ: 2-31,(%d),(%d)", ACI_RSSI_FAULT, ACI_BER_FAULT );
  io_sendMessage(srcId, g_sa, ATI_NORMAL_OUTPUT);
  return ATI_CMPL;
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_CMD            |
| STATE   : code                        ROUTINE : atPlusCSQ          |
+--------------------------------------------------------------------+

  PURPOSE : %CSQ, signal quality

  Shen,Chao Juni.12th.2003
*/

GLOBAL T_ATI_RSLT setatPercentCSQ ( char *cl, UBYTE srcId)
{
  T_ACI_CSQ_MODE CSQmode = CSQ_Disable;
  rx_Status_Type  rxStat;


  TRACE_FUNCTION("setatPercentCSQ()");

  if ( rx_GetStatus ( &rxStat ) NEQ DRV_OK )
  {
    cmdCmeError ( CME_ERR_Unknown );
    return ATI_FAIL;
  }
  else
  {
    switch( *cl )
    {
      case '0':
      {
        CSQmode = CSQ_Disable;
        break;
      }

      case '1':
      {
        CSQmode = CSQ_Enable;
        break;
      }

      default:
      {
        cmdCmeError(CME_ERR_Unknown);
        return (ATI_FAIL);
      }
    }
#ifdef FF_ATI_BAT
  {
    T_BAT_cmd_send cmd;
    T_BAT_cmd_set_percent_csq csq;
    T_ATI_SRC_PARAMS *src_params = find_element (ati_src_list, srcId, search_ati_src_id);

    TRACE_FUNCTION("setatPercentCSQ() calls bat_send() <=== as APPLICATION");

    cmd.ctrl_params = BAT_CMD_SET_PERCENT_CSQ;
    cmd.params.ptr_set_percent_csq = &csq;
    csq.csq_mode = CSQmode;
    src_params->curAtCmd = AT_CMD_P_CSQ;
    bat_send(ati_bat_get_client(srcId), &cmd);

    return ATI_EXCT;
  }
#else
    if (sAT_PercentCSQ((T_ACI_CMD_SRC)srcId,CSQmode) NEQ AT_CMPL)
    {
      TRACE_EVENT("setatPercentCSQ call sAT_PercentCSQ Error!!");
    }

    sprintf (g_sa, "%s%d ", "%CSQ: ", CSQmode);
    io_sendMessage(srcId, g_sa, ATI_NORMAL_OUTPUT);
    return ATI_CMPL;
#endif /* FF_ATI_BAT */
  }
}


GLOBAL T_ATI_RSLT queatPercentCSQ(CHAR *cl, UBYTE srcId)
{
  rx_Status_Type    rxStat;

  TRACE_FUNCTION("queatPercentCSQ()");

#ifdef FF_PS_RSSI
  qAT_PercentCSQ((T_ACI_CMD_SRC)srcId,&rxStat.gsmLevel,&rxStat.rxQuality,&rxStat.actLevel,&rxStat.min_access_level);
  sprintf (g_sa, "%s %d, %d, %d, %d", "%CSQ: ",rxStat.gsmLevel, rxStat.rxQuality, rxStat.actLevel, rxStat.min_access_level);
#else
  qAT_PercentCSQ((T_ACI_CMD_SRC)srcId,&rxStat.gsmLevel,&rxStat.rxQuality,&rxStat.actLevel);
  sprintf (g_sa, "%s %d, %d, %d", "%CSQ: ",rxStat.gsmLevel, rxStat.rxQuality, rxStat.actLevel);
#endif
  io_sendMessage(srcId, g_sa, ATI_NORMAL_OUTPUT);

  return ATI_CMPL;
}


GLOBAL T_ATI_RSLT tesatPercentCSQ(CHAR *cl, UBYTE srcId)
{
  TRACE_FUNCTION("tesatPercentCSQ()");

#ifdef FF_PS_RSSI
  sprintf (g_sa, "%s (%d), (%d), (%d), 0-4", "%CSQ: ",ACI_RSSI_FAULT, ACI_BER_FAULT, ACI_MIN_RXLEV_FAULT);
#else
  sprintf (g_sa, "%s (%d), (%d), 0-4", "%CSQ: ",ACI_RSSI_FAULT, ACI_BER_FAULT);
#endif
  io_sendMessage(srcId, g_sa, ATI_NORMAL_OUTPUT);
  return ATI_CMPL;
}

#ifdef TI_PS_FF_AT_P_CMD_DBGINFO
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_CMD            |
| STATE   : code                        ROUTINE : atPercentDBGINFO   |
+--------------------------------------------------------------------+

  PURPOSE : %DBGINFO, query free mem pool blocks.
*/

GLOBAL T_ATI_RSLT setatPercentDBGINFO ( char *cl, UBYTE srcId)
{
  char *me            = "%DBGINFO:";
  ULONG param         = 0;
  T_ACI_DBG_INFO stor = P_DBGINFO_NotPresent;
  CHAR type[4]        = {0};
  USHORT free = 0, alloc = 0, i = 0;

  TRACE_FUNCTION("queatPercentDBGINFO()");

  cl = parse(cl,"sd",(LONG)4, type, &param);
  if(!cl OR *type EQ '\0')
  {
    cmdCmeError(CME_ERR_OpNotAllow);
    return (ATI_FAIL);
  }
  strupper(type);
  for(i=0;dbg_mem_names[i].name NEQ 0;i++)
  {
    if (!strcmp(dbg_mem_names[i].name, type) )
    {
      stor = dbg_mem_names[i].stor;
      break;
    }
  }
  if((dbg_mem_names[i].name EQ 0) OR (param EQ 0))
  {
    cmdCmeError(CME_ERR_OpNotAllow);
    return (ATI_FAIL);
  }
  if (qAT_PercentDBGINFO((T_ACI_CMD_SRC)srcId, param, (USHORT)stor, &free, &alloc) NEQ AT_CMPL)
  {
    cmdCmeError(CME_ERR_Unknown);
    return (ATI_FAIL);
  }
  sprintf(g_sa, "%s%d", me, free);
  io_sendMessage(srcId, g_sa, ATI_NORMAL_OUTPUT);

  return (ATI_CMPL);
}
#endif /* TI_PS_FF_AT_P_CMD_DBGINFO */

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_CMD            |
| STATE   : code                        ROUTINE : atPlusCBC          |
+--------------------------------------------------------------------+

  PURPOSE : +CBC, battery charge
*/
#define ACI_BCS_BattPower  0
#define ACI_BCS_BattCnctd  1
#define ACI_BCS_NoBatt     2
#define ACI_BCS_PowerFault 3

GLOBAL T_ATI_RSLT atPlusCBC ( char *cl, UBYTE srcId )
{
  pwr_Status_Type powerStat;
  UBYTE           bcs;

  TRACE_FUNCTION("atPlusCBC()");

  if ( pwr_GetStatus ( &powerStat ) NEQ DRV_OK )
  {
    cmdCmeError ( CME_ERR_Unknown );
    return ATI_FAIL;
  }
  else
  {
    switch ( powerStat.Status )
    {
      case ( PWR_EXTPOWER_ON ):
        bcs = ACI_BCS_NoBatt;
        break;

      case ( PWR_CHARGER_ON ):
        bcs = ACI_BCS_BattCnctd;
        break;

      default:
        bcs = ACI_BCS_BattPower;
        break;
    }
    sprintf(g_sa, "+CBC: %d,%d", bcs, powerStat.ChargeLevel );
    io_sendMessage(srcId, g_sa, ATI_NORMAL_OUTPUT);
    return ATI_CMPL;
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_CMD            |
| STATE   : code                        ROUTINE : atPlusCLVL         |
+--------------------------------------------------------------------+

  PURPOSE : +CLVL, loudspeaker volume level
*/
GLOBAL T_ATI_RSLT setatPlusCLVL ( char *cl, UBYTE srcId )
{
#ifdef FF_ATI_BAT
  {
  T_BAT_cmd_send cmd;
  T_BAT_cmd_set_plus_clvl my_bat_set_plus_clvl;
  SHORT vol;

  TRACE_FUNCTION("setatPlusCLVL() calls bat_send() <=== as APPLICATION");

  cl = parse ( cl, "d", &vol );

  if( !cl OR vol > 255 OR vol < 0 )
  {
    cmdCmeError ( CME_ERR_OpNotAllow );
    return ATI_FAIL;
  }

  memset(&my_bat_set_plus_clvl, 0, sizeof(my_bat_set_plus_clvl));
  cmd.ctrl_params = BAT_CMD_SET_PLUS_CLVL;
  cmd.params.ptr_set_plus_clvl = &my_bat_set_plus_clvl;

  my_bat_set_plus_clvl.level = (U8)vol;

  bat_send(ati_bat_get_client(srcId), &cmd);

  return ATI_EXCT; /* executing, because response is passed by callback function */
  }
#else /* OLD FUNCTION BODY */

  TRACE_FUNCTION("setatPlusCLVL()");

  return setaciPrcsVolLevel ( cl, AUDIO_SPEAKER );

#endif /* no FF_ATI_BAT*/
}

GLOBAL T_ATI_RSLT tesatPlusCLVL ( char *cl, UBYTE srcId )
{
  TRACE_FUNCTION("tesatPlusCLVL()");

  return tesaciPrcsVolLevel ( srcId, cl, AUDIO_SPEAKER );
}

GLOBAL T_ATI_RSLT queatPlusCLVL ( char *cl, UBYTE srcId )
{
  TRACE_FUNCTION("queatPlusCLVL()");

  return queaciPrcsVolLevel ( srcId, cl, AUDIO_SPEAKER );
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_CMD            |
| STATE   : code                        ROUTINE : atPlusCRSL         |
+--------------------------------------------------------------------+

  PURPOSE : +CRSL, ringer sound level
*/
GLOBAL T_ATI_RSLT setatPlusCRSL ( char *cl, UBYTE srcId )
{
  TRACE_FUNCTION("atPlusCRSL()");

  return setaciPrcsVolLevel ( cl, AUDIO_BUZZER );
}

GLOBAL T_ATI_RSLT tesatPlusCRSL ( char *cl, UBYTE srcId )
{
  TRACE_FUNCTION("tesatPlusCRSL()");

  return tesaciPrcsVolLevel ( srcId, cl, AUDIO_BUZZER );
}

GLOBAL T_ATI_RSLT queatPlusCRSL ( char *cl, UBYTE srcId )
{
  TRACE_FUNCTION("queatPlusCRSL()");

  return queaciPrcsVolLevel ( srcId, cl, AUDIO_BUZZER );
}

/*
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_CMD            |
| STATE   : code                        ROUTINE : aciPrcsVolLevel    |

  PURPOSE : +CLVL, +CRSL loudspeaker volume level
*/
LOCAL T_ATI_RSLT setaciPrcsVolLevel ( CHAR* cl, UBYTE device )
{
  SHORT vol;

  cl = parse ( cl, "d", &vol );

  if( !cl OR vol > 255 OR vol < 0 )
  {
    cmdCmeError ( CME_ERR_OpNotAllow );
    return ATI_FAIL;
  }
  else
  {
    if ( audio_SetAmplf( device, ( UBYTE ) vol ) NEQ DRV_OK )
    {
      cmdCmeError ( CME_ERR_Unknown );
      return ATI_FAIL;
    }
  }
  return ATI_CMPL;
}

LOCAL T_ATI_RSLT tesaciPrcsVolLevel ( UBYTE srcId, CHAR* cl, UBYTE device )
{
  CHAR *p_sa;
  audio_Status_Type audioStat;

  switch (device)
  {
    case AUDIO_SPEAKER:
      p_sa = "CLVL";
      break;
    case AUDIO_BUZZER:
      p_sa = "CRSL";
      break;
    default:
      cmdCmeError ( CME_ERR_Unknown );
      return ATI_FAIL;
  }

  if ( audio_GetStatus ( device, &audioStat ) NEQ DRV_OK )
  {
    cmdCmeError ( CME_ERR_Unknown );
    return ATI_FAIL;
  }
  {
    sprintf( g_sa, "+%s: (%d-%d)", p_sa, audioStat.min_volume, audioStat.max_volume );
    io_sendMessage(srcId, g_sa, ATI_NORMAL_OUTPUT);
  }
  return ATI_CMPL;
}


LOCAL T_ATI_RSLT queaciPrcsVolLevel ( UBYTE srcId, CHAR* cl, UBYTE device )
{
  SHORT  vol;
  UBYTE  drvVol;
  CHAR *p_sa;

  switch (device)
  {
    case AUDIO_SPEAKER:
      p_sa = "CLVL";
      break;
    case AUDIO_BUZZER:
      p_sa = "CRSL";
      break;
    default:
      cmdCmeError ( CME_ERR_Unknown );
      return ATI_FAIL;
  }

  if ( audio_GetAmplf ( device, &drvVol ) NEQ DRV_OK )
  {
    cmdCmeError ( CME_ERR_Unknown );
    return ATI_FAIL;
  }
  else
  {
    vol = drvVol;
    sprintf ( g_sa, "+%s: %d", p_sa, vol );
    io_sendMessage(srcId, g_sa, ATI_NORMAL_OUTPUT);
  }
  return ATI_CMPL;
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_CMD            |
| STATE   : code                        ROUTINE : atPlusCMUT         |
+--------------------------------------------------------------------+

  PURPOSE : +CMUT, mute control
*/
#define ACI_MUTE_OFF 0
#define ACI_MUTE_ON  1

GLOBAL T_ATI_RSLT setatPlusCMUT(CHAR *cl, UBYTE srcId)
{
  SHORT mute;
  UBYTE drvMute;

  cl = parse ( cl, "d", &mute );

  if( !cl OR ( mute NEQ ACI_MUTE_OFF AND mute NEQ ACI_MUTE_ON ) )
  {
    cmdCmeError ( CME_ERR_OpNotAllow );
    return ATI_FAIL;
  }
  if ( mute EQ ACI_MUTE_OFF )
  {
    drvMute = AUDIO_MUTING_OFF;
  }
  else
  {
    drvMute = AUDIO_MUTING_ON;
  }
  if (audio_SetMute (AUDIO_MICROPHONE, drvMute) NEQ DRV_OK)
  {
    cmdCmeError ( CME_ERR_Unknown );
    return ATI_FAIL;
  }

#ifdef FF_ATI_BAT
  {
  T_BAT_cmd_send cmd;
  T_BAT_cmd_set_plus_cmut my_bat_set_plus_cmut;

  TRACE_FUNCTION("setatPlusCMUT() calls bat_send() <=== as APPLICATION");

  memset(&my_bat_set_plus_cmut, 0, sizeof(my_bat_set_plus_cmut));
  cmd.ctrl_params = BAT_CMD_SET_PLUS_CMUT;
  cmd.params.ptr_set_plus_cmut = &my_bat_set_plus_cmut;
  my_bat_set_plus_cmut.n = (T_BAT_plus_cmut_n)drvMute;
  bat_send(ati_bat_get_client(srcId), &cmd);
  return ATI_EXCT; /* executing, because response is passed by callback function */
  }
#else /* OLD FUNCTION BODY */

  TRACE_FUNCTION("setatPLusCMUT()");

  return ATI_CMPL;

#endif /* no FF_ATI_BAT */
}

GLOBAL T_ATI_RSLT tesatPlusCMUT ( char *cl, UBYTE srcId )
{
  TRACE_FUNCTION("tesatPlusCMUT()");

  sprintf(g_sa, "+CMUT: (%d,%d)", ACI_MUTE_OFF, ACI_MUTE_ON );
  io_sendMessage (srcId, g_sa, ATI_NORMAL_OUTPUT);
  return ATI_CMPL;
}

GLOBAL T_ATI_RSLT queatPlusCMUT(CHAR *cl, UBYTE srcId)
{
  SHORT mute;
  UBYTE drvMute;

  TRACE_FUNCTION("queatPlusCMUT()");

  if ( audio_GetMute ( AUDIO_MICROPHONE, &drvMute ) NEQ DRV_OK )
  {
    cmdCmeError ( CME_ERR_Unknown );
    return ATI_FAIL;
  }
  else
  {
    if ( drvMute EQ AUDIO_MUTING_OFF )
    {
      mute = ACI_MUTE_OFF;
    }
    else
    {
      mute = ACI_MUTE_ON;
    }

    sprintf ( g_sa, "+CMUT: %d", mute );
    io_sendMessage(srcId, g_sa, ATI_NORMAL_OUTPUT);
    return ATI_CMPL;
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_CMD            |
| STATE   : code                        ROUTINE : atPlusCGMI         |
+--------------------------------------------------------------------+

  PURPOSE : +CGMI, show name of manufacturer
*/
GLOBAL T_ATI_RSLT atPlusCGMI ( char *cl, UBYTE srcId )
{
  TRACE_FUNCTION("atPlusCGMI()");

  return aciPrcsPlusCG (srcId, cl, EF_CGMI_ID);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_CMD            |
| STATE   : code                        ROUTINE : atPlusCGMM         |
+--------------------------------------------------------------------+

  PURPOSE : +CGMM, show name of prodcut
*/
GLOBAL T_ATI_RSLT atPlusCGMM ( char *cl, UBYTE srcId )
{
  TRACE_FUNCTION("atPlusCGMM()");

  return aciPrcsPlusCG (srcId, cl, EF_CGMM_ID);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_CMD            |
| STATE   : code                        ROUTINE : atPlusCGMR         |
+--------------------------------------------------------------------+

  PURPOSE : +CGMR, show version of product
*/
GLOBAL T_ATI_RSLT atPlusCGMR ( char *cl, UBYTE srcId )
{
  TRACE_FUNCTION("atPlusCGMR()");

  return aciPrcsPlusCG (srcId, cl, EF_CGMR_ID);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_CMD            |
| STATE   : code                        ROUTINE : atPlusCGSN         |
+--------------------------------------------------------------------+

  PURPOSE : +CGSN, show serial number
*/
GLOBAL T_ATI_RSLT atPlusCGSN ( char *cl, UBYTE srcId )
{
  TRACE_FUNCTION("atPlusCGSN()");

  return aciPrcsPlusCG (srcId, cl, EF_CGSN_ID);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_CMD            |
| STATE   : code                        ROUTINE : atPlusFMI          |
+--------------------------------------------------------------------+

  PURPOSE : +FMI, show name of manufacturer
*/
GLOBAL T_ATI_RSLT atPlusFMI ( char *cl, UBYTE srcId )
{
  TRACE_FUNCTION("atPlusFMI()");

  if ( *cl NEQ '?')
  {
    cmdCmeError ( CME_ERR_OpNotAllow );
    return ATI_FAIL;
  }
  return aciPrcsPlusCG (srcId, cl, EF_CGMI_ID);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_CMD            |
| STATE   : code                        ROUTINE : atPlusFMM          |
+--------------------------------------------------------------------+

  PURPOSE : +FMM, show name of prodcut
*/
GLOBAL T_ATI_RSLT atPlusFMM ( char *cl, UBYTE srcId )
{
  TRACE_FUNCTION("atPlusFMM()");

  if ( *cl NEQ '?')
  {
    cmdCmeError ( CME_ERR_OpNotAllow );
    return ATI_FAIL;
  }
  return aciPrcsPlusCG (srcId, cl, EF_CGMM_ID);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_CMD            |
| STATE   : code                        ROUTINE : atPlusFMR          |
+--------------------------------------------------------------------+

  PURPOSE : +FMR, show version of product
*/
GLOBAL T_ATI_RSLT atPlusFMR ( char *cl, UBYTE srcId )
{
  TRACE_FUNCTION("atPlusFMR()");

  if ( *cl NEQ '?')
  {
    cmdCmeError ( CME_ERR_OpNotAllow );
    return ATI_FAIL;
  }
  return aciPrcsPlusCG (srcId, cl, EF_CGMR_ID);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_CMD            |
| STATE   : code                        ROUTINE : aciPrcsPlusCG      |
+--------------------------------------------------------------------+

  PURPOSE :
*/
LOCAL T_ATI_RSLT aciPrcsPlusCG (UBYTE srcId, CHAR* cl, CHAR* ef)
{
  pcm_FileInfo_Type fileInfo;
  USHORT i;
  char *cp;

  TRACE_FUNCTION("aciPrcsPlusCG()");

  if (pcm_GetFileInfo ((UBYTE* )ef, &fileInfo) NEQ DRV_OK)
  {
    cmdCmeError (CME_ERR_MemFail);
    return ATI_FAIL;
  }

  if (pcm_ReadFile ((UBYTE*)ef, fileInfo.FileSize,
                    (UBYTE*)g_sa, &fileInfo.Version) NEQ DRV_OK)
  {
    cmdCmeError (CME_ERR_MemFail);
    return ATI_FAIL;
  }

  i = 0;
  while ((UBYTE)g_sa[i] NEQ 0xFF AND i < fileInfo.FileSize)
    i++;
  g_sa[i] = '\0';

#if CONFIG_MOKOFFS
  /*
   * Openmoko's FFS has a /pcm/CGMR file programmed like this:
   *
   *   GTA02BV4/Moko5
   *
   * When queried for +CGMR, we would like to report the hardware revision
   * from FFS, but also tell the user that we are FreeCalypso and not Moko5.
   * Hence the following hack.
   */
  if ((cp = strchr(g_sa, '/')) && !strncmp(cp + 1, "Moko", 4))
    strcpy(cp + 1, "FreeCalypso");
#endif

  io_sendMessage (srcId, g_sa, ATI_NORMAL_OUTPUT );

  return ATI_CMPL;
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_CMD            |
| STATE   : code                        ROUTINE : atPlusCSCS         |
+--------------------------------------------------------------------+

  PURPOSE : +CSCS command (Select TE character set)
*/
GLOBAL T_ATI_RSLT setatPlusCSCS (char *cl, UBYTE srcId)
{
  CHAR             txtChset[10];
  T_ACI_CSCS_CHSET chset         = CSCS_CHSET_NotPresent;

  TRACE_FUNCTION("setatPlusCSCS()");

  cl = parse ( cl, "s", (LONG) sizeof (txtChset), txtChset);

  if      ( strcmp ( txtChset, "GSM"     ) EQ 0 )
    chset = CSCS_CHSET_Gsm;
  else if ( strcmp ( txtChset, "IRA"     ) EQ 0 )
    chset = CSCS_CHSET_Ira;
  else if ( strcmp ( txtChset, "PCCP437" ) EQ 0 )
    chset = CSCS_CHSET_Pccp_437;
  else if ( strcmp ( txtChset, "PCDN"    ) EQ 0 )
    chset = CSCS_CHSET_Pcdn;
  else if ( strcmp ( txtChset, "8859-1"  ) EQ 0 )
    chset = CSCS_CHSET_8859_1;
  else if ( strcmp ( txtChset, "HEX"     ) EQ 0 )
    chset = CSCS_CHSET_Hex;
  else if ( strcmp ( txtChset, "UCS2"     ) EQ 0 )
    chset = CSCS_CHSET_Ucs2;

  if ( !cl OR chset EQ CSCS_CHSET_NotPresent )
  {
    cmdCmeError ( CME_ERR_OpNotAllow );
    return ATI_FAIL;
  }

#ifdef FF_ATI_BAT

  {
    T_BAT_cmd_send cmd;
    T_BAT_cmd_set_plus_cscs cscs;

    cmd.ctrl_params=BAT_CMD_SET_PLUS_CSCS;
    cmd.params.ptr_set_plus_cscs=&cscs;
    /*
    *   This relies on T_BAT_plus_cscs being identical to
    *   T_ACI_CSCS_CHSET. It is, apart from the NotPresent
    *   value, which is taken care of earlier in this
    *   function.
    */
    cscs.cs=(T_BAT_plus_cscs_cs)chset;
    bat_send(ati_bat_get_client(srcId), &cmd);
    return(ATI_EXCT);
  }

#else 

  ati_user_output_cfg[srcId].cscsChset = chset;
  return ATI_CMPL;

#endif
}

GLOBAL T_ATI_RSLT queatPlusCSCS (char *cl, UBYTE srcId)
{
  TRACE_FUNCTION("queatPlusCSCS()");

#ifdef FF_ATI_BAT
  {
    T_BAT_cmd_send cmd;
    T_BAT_no_parameter dummy;

    cmd.ctrl_params=BAT_CMD_QUE_PLUS_CSCS;
    dummy.bat_dummy = 0xFF;
    cmd.params.ptr_que_plus_cscs = &dummy;
    bat_send(ati_bat_get_client(srcId), &cmd);
  } 
  return(ATI_EXCT);

#else /* no FF_ATI_BAT */

  strcpy ( g_sa, "+CSCS: ");

  switch ( ati_user_output_cfg[srcId].cscsChset )
  {
    case (CSCS_CHSET_Ira     ): strcat (g_sa,"\"IRA\""    ); break;
    case (CSCS_CHSET_Pcdn    ): strcat (g_sa,"\"PCDN\""   ); break;
    case (CSCS_CHSET_8859_1  ): strcat (g_sa,"\"8859-1\"" ); break;
    case (CSCS_CHSET_Pccp_437): strcat (g_sa,"\"PCCP437\""); break;
    case (CSCS_CHSET_Gsm     ): strcat (g_sa,"\"GSM\""    ); break;
    case (CSCS_CHSET_Hex     ): strcat (g_sa,"\"HEX\""    ); break;
    case (CSCS_CHSET_Ucs2    ): strcat (g_sa,"\"UCS2\""   ); break;

    /*
     *-----------------------------------------------------------
     * This case will be prevented during settings procedure of
     * the TE character set ( CSCS=... )
     *-----------------------------------------------------------
     */
    default: cmdCmeError ( CME_ERR_Unknown ); return ATI_FAIL;
  }

  io_sendMessage ( srcId, g_sa, ATI_NORMAL_OUTPUT );
  return ATI_CMPL;

#endif /* no FF_ATI_BAT */
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_CMD            |
| STATE   : code                        ROUTINE : atPlusCMEE         |
+--------------------------------------------------------------------+

  PURPOSE : +CMEE command (Error display mode)
*/

GLOBAL T_ATI_RSLT setatPlusCMEE(CHAR *cl, UBYTE srcId)
{
  SHORT val;

  TRACE_FUNCTION("setatPLusCMEE()");

  cl=parse(cl,"r",&val);
  if ( !cl OR val > CMEE_MOD_Verbose OR val < CMEE_MOD_Disable )
  {
    cmdAtError(atError);
    return ATI_FAIL;
  }
  else
  {
    TRACE_EVENT_P2("setatPlusCMEE: srcId = %d Value = %d", srcId, val);
    ati_user_output_cfg[srcId].CMEE_stat = (UBYTE)val;
    return ATI_CMPL;
  }
}


/* query function */

GLOBAL T_ATI_RSLT queatPlusCMEE(CHAR *cl, UBYTE srcId)
{
  TRACE_FUNCTION("queatPLusCMEE()");

  TRACE_EVENT_P1("queatPlusCMEE: srcId = %d", srcId);
  resp_disp(srcId, cl,"b",&ati_user_output_cfg[srcId].CMEE_stat);
  return ATI_CMPL;
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_CMD            |
| STATE   : code                        ROUTINE : atPlusGCAP         |
+--------------------------------------------------------------------+

  PURPOSE : +GCAP command (capability information)
*/
GLOBAL T_ATI_RSLT atPlusGCAP(char *cl, UBYTE srcId)
{
  if (*cl EQ '\0')
  {
#ifdef FAX_AND_DATA

#ifdef FF_FAX

#ifdef V42BIS
    sprintf(g_sa,"+GCAP: +CGSM,+FCLASS,+DS");
#else
    sprintf(g_sa,"+GCAP: +CGSM,+FCLASS");
#endif

#else /* no FAX */

#ifdef V42BIS
    sprintf(g_sa,"+GCAP: +CGSM,+DS");
#else
    sprintf(g_sa,"+GCAP: +CGSM");
#endif

#endif /* FF_FAX */

#else /* Voice only */

    sprintf(g_sa,"+GCAP: +CGSM");

#endif /* FAX_AND_DATA */

    io_sendMessage(srcId, g_sa, ATI_NORMAL_OUTPUT);
    return ATI_CMPL;
  }
  else
  {
    cmdAtError(atError);
    return ATI_FAIL;
  }
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_CMD            |
| STATE   : code                        ROUTINE : atPercentCCBS      |
+--------------------------------------------------------------------+

  PURPOSE : %CCBS command (Call Completion to Busy Subscriber)
*/

GLOBAL T_ATI_RSLT setatPercentCCBS(char *cl, UBYTE srcId)
{
  SHORT mode                   = ACI_NumParmNotPresent;
  SHORT idx                    = ACI_NumParmNotPresent;
  T_ATI_SRC_PARAMS *src_params = find_element (ati_src_list, srcId, search_ati_src_id);
  T_ACI_RETURN ret             = AT_FAIL;

  TRACE_FUNCTION("setatPercentCCBS()");

   cl = parse (cl,"rr",&mode,&idx);
   if(!cl OR (mode NEQ ACI_NumParmNotPresent AND (mode > 1 OR mode < 0)))
   {
     cmdCmeError (CME_ERR_OpNotAllow);
     return ATI_FAIL;
   }

   if( mode NEQ ACI_NumParmNotPresent )
     at.flags.CCBS_stat=(UBYTE)mode;

#ifdef FF_ATI_BAT

  if (idx NEQ  ACI_NumParmNotPresent)
  {
    T_BAT_cmd_send cmd;
    T_BAT_cmd_set_percent_ccbs ccbs;

    cmd.ctrl_params=BAT_CMD_SET_PERCENT_CCBS;
    cmd.params.ptr_set_percent_ccbs=&ccbs;
    ccbs.idx=(T_BAT_percent_ccbs_idx)idx;
    bat_send(ati_bat_get_client(srcId), &cmd);
    return(ATI_EXCT);
  }
  else
  {
    return(ATI_CMPL);
  }

#else /* no FF_ATI_BAT */ 

   if( idx NEQ ACI_NumParmNotPresent )
   {
     ret = sAT_PercentCCBS((T_ACI_CMD_SRC)srcId, idx);
     if( ret NEQ AT_EXCT )
     {
       cmdCmeError(CME_ERR_Unknown);
       return ATI_FAIL;
     }
     src_params->curAtCmd    = AT_CMD_CCBS;
   }
   else
   {
     ret = AT_CMPL;
   }
   return (map_aci_2_ati_rslt(ret));

#endif /* no FF_ATI_BAT */
}

GLOBAL T_ATI_RSLT queatPercentCCBS(char *cl, UBYTE srcId)
{
  T_ACI_RETURN ret             = AT_FAIL;
  T_ATI_SRC_PARAMS *src_params = find_element (ati_src_list, srcId, search_ati_src_id);

  TRACE_FUNCTION("queatPercentCCBS()");

  ret = qAT_PercentCCBS((T_ACI_CMD_SRC)srcId);

  if( ret NEQ AT_EXCT )
  {
    cmdCmeError(CME_ERR_Unknown);
    return ATI_FAIL;
  }
  src_params->curAtCmd    = AT_CMD_CCBS;
  return (map_aci_2_ati_rslt(ret));
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_CMD            |
| STATE   : code                        ROUTINE : atPlusCMUX         |
+--------------------------------------------------------------------+

  PURPOSE : +CMUX command (multiplexer)
*/
GLOBAL T_ATI_RSLT setatPlusCMUX (char *cl, UBYTE srcId)
{
  T_ACI_RETURN ret  = AT_FAIL;
  SHORT mode        = ACI_NumParmNotPresent;
  SHORT subset      = 0;      /* default value */
  UBYTE  port_speed = NOT_PRESENT_8BIT;
  int   N1          = 64;
                /* default value in advanced mode (other modes not supported */
  SHORT T1          = 10;         /* (100 ms) */
  SHORT N2          = 3;
  SHORT T2          = 30;         /* (30 ms) */
  SHORT T3          = 10;         /* (10 s) */
  SHORT k           = 2;

  TRACE_FUNCTION("setatPlusCMUX()");

  cl = parse (cl,"rrrdrrrrr",&mode,&subset,&port_speed,&N1,&T1,&N2,&T2,&T3,&k);
  if (!cl)
  {
    cmdCmeError(CME_ERR_OpNotAllow);
    return ATI_FAIL;
  }

#ifdef DTI
  /* values not supported */
  if ( mode NEQ 1
    OR subset NEQ 0
    OR ((port_speed NEQ NOT_PRESENT_8BIT)
      AND (convert_mux_port_speed(port_speed) EQ BD_RATE_NotPresent))
    OR N1 < 1 OR N1 > 32768
    OR T1 < 1 OR T1 > 255
    OR N2 < 0 OR N2 > 100
    OR T2 < 2 OR T2 > 255
    OR T3 < 1 OR T3 > 255
    OR k < 1 OR k > 7 )
  {
    TRACE_EVENT("at least one value is beyond capabilities");
    cmdCmeError(CME_ERR_OpNotAllow);
    return ATI_FAIL;
  }

#ifdef FF_ATI_BAT

  {
    T_BAT_cmd_send cmd;
    T_BAT_cmd_set_plus_cmux cmux;

    cmd.ctrl_params=BAT_CMD_SET_PLUS_CMUX;
    cmd.params.ptr_set_plus_cmux=&cmux;

    cmux.mode=(T_BAT_plus_cmux_mode)mode;
    cmux.subset=(T_BAT_plus_cmux_subset)subset;
    cmux.port_speed=(T_BAT_plus_cmux_port_speed)port_speed;
    cmux.n1=(U16)N1;
    cmux.t1=(U8)T1;
    cmux.n2=(U8)N2;
    cmux.t2=(U8)T2;
    cmux.t3=(U8)T3;
    cmux.k=(S16)k;

    bat_send(ati_bat_get_client(srcId), &cmd);
    return(ATI_EXCT);
  }

#else /* no FF_ATI_BAT */

  ret = sAT_PlusCMUX((T_ACI_CMD_SRC)srcId,
                     (UBYTE)mode,
                     (UBYTE)subset,
                     port_speed,
                     (USHORT)N1,
                     (UBYTE)T1,
                     (UBYTE)N2,
                     (UBYTE)T2,
                     (UBYTE)T3);

#endif /* no FF_ATI_BAT */

#endif /* DTI */

  if( ret EQ AT_FAIL )
  {
    cmdCmeError(CME_ERR_Unknown);
    return ATI_FAIL;
  }
  return ATI_CMPL_NO_OUTPUT;
}

GLOBAL T_ATI_RSLT tesatPlusCMUX (char *cl, UBYTE srcId)
{
  TRACE_FUNCTION("tesatPlusCMUX()");

  sprintf(g_sa,"+CMUX: %s","(1),(0),(1-5),(10-100),(1-255),(0-100),(2-255),(1-255),(1-7)");
  io_sendMessage(srcId, g_sa, ATI_NORMAL_OUTPUT);

  io_sendConfirm(srcId, cmdAtError(atOk), ATI_NORMAL_OUTPUT);

  cmdErrStr = NULL;

  return ATI_CMPL_NO_OUTPUT;
}

GLOBAL T_ATI_RSLT queatPlusCMUX (char *cl, UBYTE srcId)
{
  T_ACI_RETURN ret  = AT_FAIL;
  UBYTE mode;
  UBYTE subset;
  UBYTE port_speed;
  USHORT N1;
  UBYTE T1;
  UBYTE N2;
  UBYTE T2;
  UBYTE T3;
  SHORT k = 2;

  TRACE_FUNCTION("queatPlusCMUX()");

#ifdef FF_ATI_BAT
  {
    T_BAT_cmd_send cmd;
    T_BAT_no_parameter dummy;

    cmd.ctrl_params = BAT_CMD_QUE_PLUS_CMUX;
    dummy.bat_dummy = 0xFF;
    cmd.params.ptr_que_plus_cmux = &dummy;
    bat_send(ati_bat_get_client(srcId), &cmd);
    return(ATI_EXCT);
  } 
#else /* no FF_ATI_BAT */

#ifdef DTI
  ret = qAT_PlusCMUX((T_ACI_CMD_SRC)srcId,
                    &mode,
                    &subset,
                    &port_speed,
                    &N1,
                    &T1,
                    &N2,
                    &T2,
                    &T3);
  if (ret EQ AT_CMPL)
  {
    sprintf(g_sa,"+CMUX: %d,%d,%d,%d,%d,%d,%d,%d,%d", mode, subset, port_speed, N1, T1, N2, T2, T3, k);
    io_sendMessage(srcId, g_sa, ATI_NORMAL_OUTPUT);
    return ATI_CMPL;
  }
  else if (ret EQ AT_FAIL)
  {
    cmdCmeError(CME_ERR_Unknown);
  }
#endif /* DTI */
  return ATI_FAIL;

#endif /* no FF_ATI_BAT */
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_CMD            |
| STATE   : code                        ROUTINE : atPlusIPR          |
+--------------------------------------------------------------------+

  PURPOSE : +IPR command (DTE speed setting)
*/

GLOBAL T_ATI_RSLT setatPlusIPR(char *cl, UBYTE srcId)
{
  T_ATI_SRC_PARAMS *src_params = find_element (ati_src_list, srcId, search_ati_src_id);
  T_ACI_BD_RATE     speed;
  char *end;

  src_params->curAtCmd = AT_CMD_IPR;

  switch ( strtol(cl, &end, 10) )
  {
    case 0:       speed = BD_RATE_AUTO;   break;
    case 75:      speed = BD_RATE_75;     break;
    case 150:     speed = BD_RATE_150;    break;
    case 300:     speed = BD_RATE_300;    break;
    case 600:     speed = BD_RATE_600;    break;
    case 1200:    speed = BD_RATE_1200;   break;
    case 2400:    speed = BD_RATE_2400;   break;
    case 4800:    speed = BD_RATE_4800;   break;
    case 7200:    speed = BD_RATE_7200;   break;
    case 9600:    speed = BD_RATE_9600;   break;
    case 14400:   speed = BD_RATE_14400;  break;
    case 19200:   speed = BD_RATE_19200;  break;
    case 28800:   speed = BD_RATE_28800;  break;
    case 33900:   speed = BD_RATE_33900;  break;
    case 38400:   speed = BD_RATE_38400;  break;
    case 57600:   speed = BD_RATE_57600;  break;
    case 115200:  speed = BD_RATE_115200; break;
    case 203125:  speed = BD_RATE_203125; break;
    case 406250:  speed = BD_RATE_406250; break;
    case 812500:  speed = BD_RATE_812500; break;
    default:
      cmdCmeError(CME_ERR_OpNotSupp);
      return ATI_FAIL;
  }

#ifdef FF_ATI_BAT
  {
  T_BAT_cmd_send cmd;
  T_BAT_cmd_set_plus_ipr my_bat_set_plus_ipr;

  TRACE_FUNCTION("setatPlusIPR() calls bat_send() <=== as APPLICATION");

  memset(&my_bat_set_plus_ipr, 0, sizeof(my_bat_set_plus_ipr));
  cmd.ctrl_params = BAT_CMD_SET_PLUS_IPR;
  cmd.params.ptr_set_plus_ipr = &my_bat_set_plus_ipr;

  my_bat_set_plus_ipr.rate = (T_BAT_plus_ipr_rate)speed;

  bat_send(ati_bat_get_client(srcId), &cmd);

  return ATI_EXCT; /* executing, because response is passed by callback function */
  }
#else /* OLD FUNCTION BODY */

  TRACE_FUNCTION("setatPlusIPR()");

#ifdef UART

  if(!end)
  {
    cmdCmeError(CME_ERR_OpNotSupp);
    return ATI_FAIL;
  }

  switch ( sAT_PlusIPR((T_ACI_CMD_SRC)srcId, speed) )
  {
    case (AT_CMPL):                    /* operation completed */
      return ATI_CMPL_NO_OUTPUT;       /* OK was already sent at old baudrate by sAT_PlusIPR,
                                         so must not be sent twice! */
    default:
      cmdCmeError(CME_ERR_NotPresent);
      return ATI_FAIL;
  }
#else
  cmdCmeError(CME_ERR_NotPresent); 
  return ATI_FAIL;

#endif /* UART */

#endif /* no FF_ATI_BAT*/
}

GLOBAL T_ATI_RSLT queatPlusIPR(char *cl, UBYTE srcId)
{
  T_ACI_BD_RATE rate;
  int rate_value;

  TRACE_FUNCTION("queatPlusIPR()");
#ifdef DTI
  if ( AT_CMPL EQ qAT_PlusIPR((T_ACI_CMD_SRC)srcId, &rate) )
  {
    switch ( rate )
    {
      case BD_RATE_AUTO:   rate_value = 0;      break;
      case BD_RATE_75:     rate_value = 75;     break;
      case BD_RATE_150:    rate_value = 150;    break;
      case BD_RATE_300:    rate_value = 300;    break;
      case BD_RATE_600:    rate_value = 600;    break;
      case BD_RATE_1200:   rate_value = 1200;   break;
      case BD_RATE_2400:   rate_value = 2400;   break;
      case BD_RATE_4800:   rate_value = 4800;   break;
      case BD_RATE_7200:   rate_value = 7200;   break;
      case BD_RATE_9600:   rate_value = 9600;   break;
      case BD_RATE_14400:  rate_value = 14400;  break;
      case BD_RATE_19200:  rate_value = 19200;  break;
      case BD_RATE_28800:  rate_value = 28800;  break;
      case BD_RATE_33900:  rate_value = 33900;  break;
      case BD_RATE_38400:  rate_value = 38400;  break;
      case BD_RATE_57600:  rate_value = 57600;  break;
      case BD_RATE_115200: rate_value = 115200; break;
      case BD_RATE_203125: rate_value = 203125; break;
      case BD_RATE_406250: rate_value = 406250; break;
      case BD_RATE_812500: rate_value = 812500; break;
      default:             rate_value = 0;      break;
    }

    sprintf(g_sa,"+IPR: %d", rate_value);
    io_sendMessage(srcId, g_sa, ATI_NORMAL_OUTPUT);
    return ATI_CMPL;
  }
#endif
  cmdAtError(atError);
  return(ATI_FAIL);
}
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_CMD            |
| STATE   : code                        ROUTINE : atPlusICF          |
+--------------------------------------------------------------------+

  PURPOSE : +ICF command (DTE character frame setting)
*/

GLOBAL T_ATI_RSLT setatPlusICF(char *cl, UBYTE srcId)
{
  T_ACI_BS_FRM      format     = BS_FRM_NotPresent;
  T_ACI_BS_PAR      parity     = BS_PAR_NotPresent;
  T_ATI_SRC_PARAMS *src_params = find_element (ati_src_list, srcId, search_ati_src_id);

  TRACE_FUNCTION("setatPlusICF()");

  cl=parse(cl,"dd",&format,&parity);

  if(!cl)
  {
    cmdAtError(atError);
    return ATI_FAIL;
  }
#ifdef DTI

#ifdef FF_ATI_BAT

  {
    T_BAT_cmd_send cmd;
    T_BAT_cmd_set_plus_icf icf;

    cmd.ctrl_params=BAT_CMD_SET_PLUS_ICF;
    cmd.params.ptr_set_plus_icf=&icf;

    /*
    *   This relies on T_ACI_BS_FRM being identical to
    *   T_BAT_framing_format and T_ACI_BS_PAR being identical
    *   to T_BAT_framing_parity.
    */
    icf.framing_format=(T_BAT_framing_format)format;
    icf.framing_parity=(T_BAT_framing_parity)parity;

    bat_send(ati_bat_get_client(srcId), &cmd);
  } 

  src_params->curAtCmd=AT_CMD_ICF;

  return(ATI_EXCT);

#else /* no FF_ATI_BAT */

  switch ( sAT_PlusICF((T_ACI_CMD_SRC)srcId, format, parity) )
  {
  case (AT_CMPL):                         /*operation completed*/
    return ATI_CMPL;
  case (AT_EXCT):
    src_params->curAtCmd = AT_CMD_ICF;
    return ATI_EXCT;
  default:
    cmdAtError(atError);                  /*Command failed*/
    return ATI_FAIL;
  }

#endif /* no FF_ATI_BAT */

#else
    cmdAtError(atError);                  /*Command failed*/
    return ATI_FAIL;
#endif
}

GLOBAL T_ATI_RSLT queatPlusICF(char *cl, UBYTE srcId)
{
#ifndef FF_ATI_BAT
  T_ACI_BS_FRM  format;
  T_ACI_BS_PAR  parity;
#endif

  TRACE_FUNCTION("queatPlusICF()");

#ifdef DTI

#ifdef FF_ATI_BAT
  {
    T_BAT_cmd_send cmd;
    T_BAT_no_parameter dummy;

    cmd.ctrl_params = BAT_CMD_QUE_PLUS_ICF;
    dummy.bat_dummy = 0xFF;
    cmd.params.ptr_que_plus_icf = &dummy;
    bat_send(ati_bat_get_client(srcId), &cmd);
    return(ATI_EXCT);
  } 
#else /* no FF_ATI_BAT */

  switch ( qAT_PlusICF((T_ACI_CMD_SRC)srcId,&format,&parity) )
  {
    case AT_CMPL:
      if (format EQ BS_FRM_Dat8_Par1_St1 OR format EQ BS_FRM_Dat7_Par1_St1 )
      {
        resp_disp(srcId, cl,"ee",&format,&parity);
      }
      else
      {
        resp_disp(srcId, cl,"e",&format);
      }
    return ATI_CMPL;
  }

#endif /* no FF_ATI_BAT */

#endif /* DTI */
  cmdAtError(atError);
  return ATI_FAIL;
}

GLOBAL T_ATI_RSLT setflowCntr(CHAR* cl, UBYTE srcId)
{
  T_ACI_RX_FLOW_CTRL DCE_by_DTE = RX_FLOW_NotPresent;  /* by TE: Rx flow control */
  T_ACI_RX_FLOW_CTRL DTE_by_DCE = RX_FLOW_NotPresent;  /* by TA: Tx flow control */

  cl = parse (cl,"dd",&DCE_by_DTE,&DTE_by_DCE);

  if(!cl)
  {
    cmdAtError(atError);
    return ATI_FAIL;
  }

#ifdef DTI
  switch ( sAT_PlusIFC((T_ACI_CMD_SRC)srcId, DCE_by_DTE, DTE_by_DCE) )
  {
  case (AT_CMPL):                         /*operation completed*/
    return ATI_CMPL;
  case (AT_EXCT):
    /* ATTENTION: no setting of 'curAtCmd' allowed because the value is set before */
    return ATI_EXCT;
  default:
    cmdAtError(atError);                  /*Command failed*/
    return ATI_FAIL;
  }
#else
  cmdAtError(atError);                  /*Command failed*/
  return ATI_FAIL;
#endif
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_CMD            |
| STATE   : code                        ROUTINE : atPlusIFC          |
+--------------------------------------------------------------------+

  PURPOSE : +IFC command (DTE DCE / DCE DTE flow control)
*/

GLOBAL T_ATI_RSLT setatPlusIFC(char *cl, UBYTE srcId)
{
  T_ATI_SRC_PARAMS *src_params = find_element (ati_src_list, srcId, search_ati_src_id);
  TRACE_FUNCTION("setatPlusIFC()");

  src_params->curAtCmd = AT_CMD_IFC;
  return setflowCntr(cl, srcId);
}


GLOBAL T_ATI_RSLT queflowCntr(CHAR* cl, UBYTE srcId)
{
  T_ACI_RX_FLOW_CTRL DCE_by_DTE;
  T_ACI_RX_FLOW_CTRL DTE_by_DCE;
  T_ATI_SRC_PARAMS *src_params = find_element (ati_src_list, srcId, search_ati_src_id);

#ifdef DTI
  if ( AT_CMPL EQ qAT_PlusIFC((T_ACI_CMD_SRC)srcId,&DCE_by_DTE,&DTE_by_DCE) )
  {
#ifdef FF_FAX
    if (src_params->curAtCmd EQ AT_CMD_FLO)
    {
      sprintf(g_sa,"+FLO: %d",DCE_by_DTE);
    }
    else
#endif
    if (src_params->curAtCmd EQ AT_CMD_IFC)
    {
      sprintf(g_sa,"+IFC: %d,%d",DCE_by_DTE,DTE_by_DCE);
    }

    io_sendMessage(srcId, g_sa, ATI_NORMAL_OUTPUT);
    return ATI_CMPL;
  }
  else
  {
    cmdAtError(atError);
    return ATI_FAIL;
  }
#else
  cmdAtError(atError);
  return ATI_FAIL;
#endif
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_CMD            |
| STATE   : code                        ROUTINE : atPlusIFC          |
+--------------------------------------------------------------------+

  PURPOSE : +IFC command (DTE DCE / DCE DTE flow control)
*/

GLOBAL T_ATI_RSLT queatPlusIFC(char *cl, UBYTE srcId)
{
  T_ATI_SRC_PARAMS *src_params = find_element (ati_src_list, srcId, search_ati_src_id);
  TRACE_FUNCTION("queatPlusIFC()");

  src_params->curAtCmd = AT_CMD_IFC;
  return queflowCntr(cl, srcId);
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-F&D (8411)              MODULE  : ACI_CMD            |
| STATE   : code                        ROUTINE : atPlusILRR         |
+--------------------------------------------------------------------+

  PURPOSE : +ILRR command (display +ILRR setting)
*/

GLOBAL T_ATI_RSLT setatPlusILRR(char *cl, UBYTE srcId)
{
  SHORT val=-1;

  TRACE_FUNCTION("setatPLusILRR()");

  cl = parse (cl,"r",&val);
  if(!cl OR val > 1 OR val < 0)
  {
    cmdAtError(atError);
    return ATI_FAIL;
  }
  ati_user_output_cfg[srcId].ILRR_stat=(UBYTE)val;
  return ATI_CMPL;
}

GLOBAL T_ATI_RSLT queatPlusILRR(char *cl, UBYTE srcId)
{
  TRACE_FUNCTION("queatPLusILRR()");

  resp_disp(srcId, cl,"b",&ati_user_output_cfg[srcId].ILRR_stat);
  return ATI_CMPL;
}

/*
+--------------------------------------------------------------------+
| PROJECT :                              MODULE  : ACI_CMD            |
| STATE   : code                        ROUTINE : setatPlusCCLK      |
+--------------------------------------------------------------------+

  PURPOSE : +CCLK command (Set the real time clock of the MT)
*/
GLOBAL T_ATI_RSLT setatPlusCCLK (char *cl, UBYTE srcId)
{
  T_ACI_RETURN        ret = AT_FAIL;
  T_ACI_RTC_DATE    date_s;
  T_ACI_RTC_TIME    time_s;
  char  date_time_string[DATE_TIME_LENGTH+1]={0};
  int items_conv = 0;
  U8  days = 0, months = 0;
  U8  hrs = 0, mins = 0, secs = 0;
  U8  years = 0;
  U8 tz = 0; /* to parse timezone from date_time_string */
  SHORT timeZone = 0;
  char sign = '+';
  
  TRACE_FUNCTION("setatPlusCCLK()");

  cl=parse(cl,"s", DATE_TIME_LENGTH, date_time_string);

  /* Expected format: yy/MM/dd,hh:mm:ss+zz (20 bytes). DATE_TIME_LENGTH(22 bytes)
   * allows length for yyyy/MM/dd,hh:mm:ss+zz */
  if(!cl OR strlen(date_time_string) NEQ DATE_TIME_LENGTH-2)
  {
    cmdCmeError(CME_ERR_OpNotAllow);
    return (ATI_FAIL);
  }

    TRACE_EVENT_P1("Date and time are: %s",date_time_string);
  items_conv = parseTimeStamp( date_time_string, &years,&months,&days,&hrs,&mins,&secs,&sign,&tz);

  timeZone = tz;

  if (sign EQ '-')
  {
    timeZone = timeZone * (-1);
  }

     TRACE_EVENT_P1("items_conv %d",items_conv);
     TRACE_EVENT_P1("days %d",days);
     TRACE_EVENT_P1("months %d",months);
     TRACE_EVENT_P1("years %d",years);
     TRACE_EVENT_P1("hours %d",hrs);
     TRACE_EVENT_P1("mins %d",mins);
     TRACE_EVENT_P1("secs %d",secs);
     TRACE_EVENT_P1("sign %c",sign);
     TRACE_EVENT_P1("timeZone %d",timeZone);


/* If 8 data items not extracted from date_time_string then date_time_string data with TZ info is incorrect. */
  if ( items_conv NEQ MAX_TIME_STAMP_FIELDS )
      {
        cmdCmeError(CME_ERR_OpNotAllow);
        return (ATI_FAIL);
      }

#ifndef _SIMULATION_
/* Ensure 'years' passed in from the AT command is not in 4 digit format
   The spec requires the years to be passed in in 2 digit format 
   Also ensure the sign char is either a '+' or '-' */
if ( (years > 99 )   OR
    ((sign NEQ '-')     AND (sign NEQ '+')) OR
     ( timeZone < (-48)) OR (timeZone > 48)
   )
   {
        cmdCmeError(CME_ERR_OpNotAllow);
        return (ATI_FAIL);
   }    
#else /* _SIMULATION_ */
/* On the target code these checks are done in the RTC code. In the simulation code- 
    the RTC routines are not used so these checks have to be done. */
if  ( (years  < 0)  OR (years  > 99)
   OR (months < 1)  OR (months > 12)
   OR (days   < 1)  OR (days   > 31)
   OR (hrs    < 0)  OR (hrs    > 23)
   OR (mins   < 0)  OR (mins   > 59)
   OR (secs   < 0)  OR (secs   > 59)
   OR (timeZone < (-48)) OR (timeZone > 48) 
    )
      {
        cmdCmeError(CME_ERR_OpNotAllow);
        return (ATI_FAIL);
      }
#endif /* _SIMULATION_ */


     /* Populate date time structs */ 
     date_s.day = days;
     date_s.month = months;
     date_s.year = years + 2000;    /* Convert years to 4 digit format for passing to lower layer RTC function */
     time_s.hour = hrs;
     time_s.minute = mins;
     time_s.PM_flag = 0;
     time_s.second = secs;
     time_s.format = TIME_FORMAT_24HOUR; 
     
  /* Pass separated date and time and timezone info to set the clock */

#ifdef FF_ATI_BAT

  {
    T_BAT_cmd_send cmd;
    T_BAT_cmd_set_plus_cclk cclk;

    cmd.ctrl_params=BAT_CMD_SET_PLUS_CCLK;
    cmd.params.ptr_set_plus_cclk=&cclk;

    cclk.year=(U8)date_s.year;
    cclk.month=(U8)date_s.month;
    cclk.day=(U8)date_s.day;

    cclk.hour=(U8)time_s.hour;

    cclk.minutes=(U8)time_s.minute;
    cclk.seconds=(U8)time_s.second;

    cclk.time_zone=(S8)timeZone;

    bat_send(ati_bat_get_client(srcId), &cmd);
  } 

  return(ATI_EXCT);

#else /* no FF_ATI_BAT */

     ret = sAT_PlusCCLK ( (T_ACI_CMD_SRC)srcId,  &date_s,  &time_s, timeZone);

    TRACE_EVENT_P1("sAT_PlusCCLK ret = %d", ret);

/* Deal with return value */

  switch (ret)
  {
    case (AT_CMPL):                                       /*operation completed*/
      break;

    default:
      cmdCmeError(CME_ERR_Unknown);         /*Command failed*/
      break;
  }
  return (map_aci_2_ati_rslt(ret)); 

#endif /* no FF_ATI_BAT */
}

/*
+--------------------------------------------------------------------+
| PROJECT :                              MODULE  : ACI_CMD            |
| STATE   : code                        ROUTINE : queatPlusCCLK      |
+--------------------------------------------------------------------+

  PURPOSE : +CCLK query command (Read the real time clock of the MT)
*/
GLOBAL T_ATI_RSLT queatPlusCCLK (char *cl, UBYTE srcId)
{
#ifndef FF_ATI_BAT
  T_ACI_RETURN ret = AT_FAIL;
  T_ACI_RTC_DATE date_s;     /* Structures to obtain date time info */
  T_ACI_RTC_TIME time_s;
  UBYTE days, months;
  UBYTE hrs, mins, secs;
  USHORT years;
#if defined FF_TIMEZONE OR defined _SIMULATION_
  char sign ;
#endif /* defined FF_TIMEZONE OR defined _SIMULATION_ */
  int tz ;
  char *me="+CCLK: ";
#endif
 
    
  TRACE_FUNCTION("queatPlusCCLK()");

#ifdef FF_ATI_BAT
  {
    T_BAT_cmd_send cmd;
    T_BAT_no_parameter dummy;

    cmd.ctrl_params=BAT_CMD_QUE_PLUS_CCLK;
    dummy.bat_dummy = 0xFF;
    cmd.params.ptr_que_plus_cclk = &dummy;
    bat_send(ati_bat_get_client(srcId), &cmd);
    return(ATI_EXCT);
  } 
#else /* no FF_ATI_BAT */

  ret = qAT_PlusCCLK( (T_ACI_CMD_SRC)srcId, &date_s, &time_s, &tz);

  if (ret EQ AT_CMPL)
  {
    /* Command  completed successfully */
    /* convert  time date struct data into string */ 
    days =  date_s.day;
    months =  date_s.month;
    years =  date_s.year;
    hrs =  time_s.hour;
    mins =  time_s.minute;
    secs =  time_s.second;

#ifndef _SIMULATION_
#ifdef FF_TIMEZONE
  if ( tz < 0)
  {
    sign = '-';
    tz = tz * (-1);
  }
  else
    sign = '+';
#endif /* FF_TIMEZONE */
#else /* _SIMULATION_ */
    sign = '-';
    tz = tz * (-1);
#endif /* _SIMULATION_ */

 /* Convert years to 2 digit format for reporting back to user */
    years = years - 2000;      /* Subtract 2000 years from number returned */

    TRACE_EVENT_P3("Date -> %d - %d - %d",days,months,years);
    TRACE_EVENT_P3("Time -> %d - %d - %d",hrs,mins,secs);
    /* Not using  time_s.format or time_s.PM_flag */ 
  
    /* CONVERT DATE TIME INFO INTO STRING FOR OUTPUTTING BACK TO DISPLAY */

#ifndef _SIMULATION_

#ifndef FF_TIMEZONE
    sprintf(g_sa, "%s\"%d/%d/%d,%d:%d:%d\"", me, years, months, days, hrs, mins, secs);
#else /* FF_TIMEZONE */
    TRACE_EVENT_P2("TimeZone -> %c%d", sign, tz);
    sprintf(g_sa, "%s\"%d/%d/%d,%d:%d:%d%c%d\"", me, years, months, days, hrs, mins, secs, sign, tz);
#endif /* FF_TIMEZONE */

#else /* _SIMULATION_ */
    TRACE_EVENT_P2("TimeZone -> %c%d", sign, tz);
    sprintf(g_sa, "%s\"%d/%d/%d,%d:%d:%d%c%d\"", me, years, months, days, hrs, mins, secs, sign, tz);
#endif /* _SIMULATION_ */

    /* REPORT DATE TIME INFO STRING */   
    io_sendMessage(srcId, g_sa, ATI_NORMAL_OUTPUT);
    
    return ATI_CMPL;
  }
  else
  {
    cmdCmeError(CME_ERR_Unknown);
    return ATI_FAIL;
  }  

#endif /* no FF_ATI_BAT */

}
#endif /* ATI_BAS_C */


