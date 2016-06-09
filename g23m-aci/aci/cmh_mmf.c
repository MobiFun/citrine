/*
+-----------------------------------------------------------------------------
|  Project :  GSM-PS (6147)
|  Modul   :  CMH_MMF
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
|  Purpose :  This module defines the functions used by the command
|             handler for mobility management.
+-----------------------------------------------------------------------------
*/

#ifndef CMH_MMF_C
#define CMH_MMF_C
#endif

#include "config.h"
#include "fixedconf.h"
#include "condat-features.h"
#include "aci_conf.h"

#include "aci_all.h"

/*==== INCLUDES ===================================================*/
#include "aci_cmh.h"
#include "ati_cmd.h"
#include "aci_cmd.h"
#include "aci_mem.h"
#include "pcm.h"

#ifdef FAX_AND_DATA
#include "aci_fd.h"
#endif    /* of #ifdef FAX_AND_DATA */

#include "aci.h"
#include "psa.h"
#include "psa_cc.h"
#include "psa_mm.h"
#include "psa_sim.h"
#include "psa_sat.h"
#include "psa_util.h"
#include "cmh.h"
#ifdef TI_PS_OP_OPN_TAB_ROMBASED
#include "plmn_decoder.h"
#include "rom_tables.h"
#endif

#ifdef DTI
#include "dti.h"      /* functionality of the dti library */
#include "dti_conn_mng.h"
/* #include "dti_cntrl_mng.h" */
#endif


#ifdef GPRS
#include "gaci.h"
#include "gaci_cmh.h"
#include "psa_gmm.h"
#include "cmh_gmm.h"
#endif /* GPRS */

#include "cmh_mm.h"
#include "cmh_sim.h"


#ifndef _SIMULATION_
#include "../../services/ffs/ffs.h"
#include "ffs_coat.h"
#endif /* !_SIMULATION_ */

#ifdef FF_CPHS
#include "cphs.h"
#endif /* FF_CPHS */

/*==== CONSTANTS ==================================================*/
/* Countries, for which the ECC should be ignored, i.e. done as normal call */
/* contains MCC and ECC, last element should be always NULL*/
GLOBAL const T_ECCIgnoreRec ECCIgnoreTable[] =
{
  { 0x260, ALL_MNC, "999" },   /* Poland         */
  { 0x454, 0x04F,   "112" },   /* "HK ORANGE"    */
  { 0x454, 0x10F,   "112" },   /* "HK New World" */
  {    -1,    -1,   NULL  }    /* End mark       */
};

#define ONS_EXT_DCS 0x80

/*==== TYPES ======================================================*/

/*==== EXPORT =====================================================*/

/*==== VARIABLES ==================================================*/
EXTERN T_ACI_CUSCFG_PARAMS cuscfgParams;
LOCAL T_OPER_ENTRY ONSDesc;
GLOBAL T_ONS_READ_STATE ONSReadStatus = ONS_READ_NOT_DONE;

/* This is required because FFS is not available in the Simulation Mode */
#ifdef _SIMULATION_
  T_FFSPLMNIMSI RAMPlmnIMSI = {0};
#endif

#if defined GPRS AND defined (DTI)
EXTERN T_GMM_SHRD_PRM gmmShrdPrm;
#endif
/* Implements Measure#32: Row 119, 980, 987, 1037 */
const char * const format_03x02x_str ="%03X%02X";
/* Implements Measure#32: Row 118, 986, 1036 */
const char * const format_03x03x_str = "%03X%03X";
/* Implements Measure#32: Row 969, 1027, 1239 & 1240 */
const char * const num_112_str = "112";
const char * const num_911_str = "911";
/* Implements Measure#32: Row 110 & 981  */
const char * const ef_plmn_id = EF_PLMN_ID;
#ifndef _SIMULATION_
/* Implements Measure#32: Row 120 & 121 */
LOCAL const char * const gsm_cops_path=MM_FFS_OPER_DIR;
LOCAL const char * const gsm_cops_operimsi_path=MM_FFS_OPER_FILE;
#endif


/*==== FUNCTIONS ==================================================*/
LOCAL void cmhMM_cvtPnnOpName( UBYTE pnn,
                               UBYTE source_len,
                               UBYTE ext_dcs,
                               CHAR *source,
                               CHAR *oper);

LOCAL void cmhMM_decodePnnOpName ( UBYTE oper_len,
                                   UBYTE *source_len,
                                   UBYTE dcs,
                                   CHAR *op_name);

LOCAL void cmhMM_decodePnnChs( CHAR *op_name,
                               UBYTE source_len,
                               UBYTE dcs,
                               char *Name );

LOCAL BOOL cmhMM_cvtPLMNINT2BCD ( T_ACI_NRG_FRMT oprFrmt, CHAR *opr);

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_MM                  |
|                                 ROUTINE : cmhMM_wild_compare      |
+-------------------------------------------------------------------+

  PURPOSE : 

*/

LOCAL BOOL cmhMM_wild_compare( int sim, int bcch )
{
  if ((bcch & 0x00F) NEQ (sim & 0x00F) AND (sim & 0x00F) != 0x00D)
    return FALSE;
  if ((bcch & 0x0F0) NEQ (sim & 0x0F0) AND (sim & 0x0F0) != 0x0D0)
    return FALSE;
  if ((bcch & 0xF00) NEQ (sim & 0xF00) AND (sim & 0xF00) != 0xD00)
    return FALSE;

  return TRUE;
}


/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_MM                  |
|                                 ROUTINE : cmhSIM_plmn_equal_sim_wc|
+-------------------------------------------------------------------+

  PURPOSE : 

*/

LOCAL BOOL cmhSIM_plmn_equal_sim_wc (int bcch_mcc, int bcch_mnc, 
                                     int  sim_mcc, int  sim_mnc)
{
  /*TRACE_FUNCTION ("cmhSIM_plmn_equal_sim()");*/

  /* Check MCC */
  if (!cmhMM_wild_compare(sim_mcc, bcch_mcc))
    return FALSE;

  /* Check first 2 MNC digits */
  if (!cmhMM_wild_compare(sim_mnc & 0xff0, bcch_mnc & 0xff0))
    return FALSE;

  /* Check for full match */
  if (cmhMM_wild_compare(sim_mnc & 0xf, bcch_mnc & 0xf))
    return TRUE;

  /* The 3rd digit of the MNC differs */
  if ((bcch_mcc >= 0x310) AND (bcch_mcc <= 0x316))
  {
    /* 
     * The MCC is in the range 310..316, this means North America.
     * The zero suffix rule applies.
     */
    return ((((sim_mnc & 0xf) EQ 0xf) AND ((bcch_mnc & 0xf) EQ 0x0)) OR
            (((sim_mnc & 0xf) EQ 0x0) AND ((bcch_mnc & 0xf) EQ 0xf)));
  }
  return ((bcch_mnc & 0xf) EQ 0xf);
}


/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_MM                  |
|                                 ROUTINE : cmhMM_FindPLMN          |
+-------------------------------------------------------------------+

  PURPOSE : scan the list of operators to find the PLMN
            representation.

  NOTE:     This function looks as if it could be simplified and
            shortened. However, it is expected that it works.

*/

GLOBAL BOOL cmhMM_FindPLMN( T_OPER_ENTRY * plmnDesc,
                            SHORT mcc, SHORT mnc, U16 lac, BOOL nw_search )
{
  int     i;                          /* holds list idx */
  EF_PLMN plmn;                       /* holds PLMN identifier */
  SHORT   sim_mcc;                    /* Integer representation MCC */
  SHORT   sim_mnc;                    /* Integer representation MNC */
  USHORT  maxRec;                     /* holds maximum records */
  USHORT  recNr;                      /* holds record number */
  UBYTE   retVal;                     /* holds return value */
  UBYTE   ver;                        /* holds version */

#ifdef TI_PS_OP_OPN_TAB_ROMBASED
/* Changes for ROM data */ 
const UBYTE *plmn_comp_entry; /* get a local pointer holder */
T_OPER_ENTRY oper;
#endif


/*
 *-------------------------------------------------------------------
 * search list of PNN records if SIM is EONS enabled
 *-------------------------------------------------------------------
 */

  TRACE_FUNCTION ("cmhMM_FindPLMN()");
  TRACE_EVENT_P3 ("mcc: %x, mnc %x, lac: %x", mcc, mnc, lac);

  /* Initial conditions for Data Coding Scheme and PLMN name source */
  memset (plmnDesc, 0, sizeof (T_OPER_ENTRY));
/*  plmnDesc.dcs = 0; */
  plmnDesc->pnn = Read_ROM_TABLE;

  if(psaSIM_ChkSIMSrvSup(SRV_PNN))
  {
    if (!nw_search) /* this is not for network search list */
    {
      if(psaSIM_ChkSIMSrvSup(SRV_OPL) OR cmhSIM_plmn_is_hplmn (mcc, mnc))
      {
        if (mmShrdPrm.PNNLst.plmn.v_plmn EQ VLD_PLMN) 
        {
          plmnDesc->mcc = mcc;
          plmnDesc->mnc = mnc;
          plmnDesc->pnn = Read_EONS;
          plmnDesc->long_len     = mmShrdPrm.PNNLst.long_len;
          plmnDesc->long_ext_dcs = mmShrdPrm.PNNLst.long_ext_dcs;
          plmnDesc->shrt_len     = mmShrdPrm.PNNLst.shrt_len;
          plmnDesc->shrt_ext_dcs = mmShrdPrm.PNNLst.shrt_ext_dcs;

          if(mmShrdPrm.PNNLst.long_len)
          {
            memcpy (plmnDesc->longName,
                    mmShrdPrm.PNNLst.long_name,
                    MINIMUM(mmShrdPrm.PNNLst.long_len, MAX_LONG_OPER_LEN - 1));
          }

          if(mmShrdPrm.PNNLst.shrt_len > 0)
          {
            memcpy (plmnDesc->shrtName,
                    mmShrdPrm.PNNLst.shrt_name,
                    MINIMUM(mmShrdPrm.PNNLst.shrt_len, MAX_SHRT_OPER_LEN - 1));
          }
          return TRUE;
        }
        else
        {
          U16 current_lac = lac;
          if (current_lac EQ NOT_PRESENT_16BIT)
            current_lac = mmShrdPrm.lac;

          for (i=0; i<simShrdPrm.opl_list.num_rcd; i++)
          {
            int pnn_rec_number = simShrdPrm.opl_list.opl_rcd[i].pnn_rec_num;
            SHORT sim_mcc, sim_mnc;

            cmhSIM_getMncMccFrmPLMNsel (simShrdPrm.opl_list.opl_rcd[i].plmn,
                                        &sim_mcc, &sim_mnc);

            if (pnn_rec_number EQ 0xFF)
              continue; /* empty record, continue with next one */

            if(cmhSIM_plmn_equal_sim_wc( mcc, mnc, sim_mcc, sim_mnc) AND
               ( (simShrdPrm.opl_list.opl_rcd[i].lac1 EQ 0x0000 AND
                  simShrdPrm.opl_list.opl_rcd[i].lac2 EQ 0xFFFE)
               OR
                 (simShrdPrm.opl_list.opl_rcd[i].lac1 <= current_lac AND
                  simShrdPrm.opl_list.opl_rcd[i].lac2 >= current_lac) ) )
            {
              if (pnn_rec_number EQ 0)
                break; /* 31.102: 4.2.59	EFOPL (Operator PLMN List):
                       A value of '00' indicates that the name is to be taken from other sources, see TS 22.101 [24] */

              pnn_rec_number--; /* adjust to zero based array */
              
              if (simShrdPrm.pnn_list.pnn_rcd[pnn_rec_number].v_plmn)
              {
                plmnDesc->mcc = mcc;
                plmnDesc->mnc = mnc;
                plmnDesc->pnn = Read_EONS;
                plmnDesc->long_len = 
                  MINIMUM (simShrdPrm.pnn_list.pnn_rcd[pnn_rec_number].long_len, MAX_LONG_OPER_LEN - 1);
                memcpy (plmnDesc->longName,
                        simShrdPrm.pnn_list.pnn_rcd[pnn_rec_number].long_name,
                        plmnDesc->long_len);
                plmnDesc->long_ext_dcs =     simShrdPrm.pnn_list.pnn_rcd[pnn_rec_number].long_ext_dcs;
                plmnDesc->shrt_len =
                  MINIMUM (simShrdPrm.pnn_list.pnn_rcd[pnn_rec_number].shrt_len, MAX_SHRT_OPER_LEN - 1);
                memcpy (plmnDesc->shrtName, 
                        simShrdPrm.pnn_list.pnn_rcd[pnn_rec_number].shrt_name,
                        plmnDesc->shrt_len); 
                plmnDesc->shrt_ext_dcs =     simShrdPrm.pnn_list.pnn_rcd[pnn_rec_number].shrt_ext_dcs;
                return TRUE;
              }
            }
          }
        }
      }
    }

    else /* the procedure is used for building the PLMN selection list */
    {
      T_pnn_name *current;  /* points to PNN element currently compared */
      current = (T_pnn_name *)mmShrdPrm.PNNLst.next;
      while (current != NULL)
      {
        SHORT sim_mcc, sim_mnc;
        cmhMM_CnvrtPLMN2INT (current->plmn.mcc, current->plmn.mnc,
                             &sim_mcc, &sim_mnc);
        if (cmhSIM_plmn_equal_sim (mcc, mnc, sim_mcc, sim_mnc) AND
            (current->lac EQ lac))
        {
          if(current->plmn.v_plmn EQ VLD_PLMN)
          {
            plmnDesc->mcc = mcc;
            plmnDesc->mnc = mnc;
            plmnDesc->pnn = Read_EONS;
            plmnDesc->long_len     = 
              MINIMUM (current->long_len, MAX_LONG_OPER_LEN - 1);
            plmnDesc->long_ext_dcs = current->long_ext_dcs;
            plmnDesc->shrt_len     = 
              MINIMUM (current->shrt_len, MAX_SHRT_OPER_LEN - 1);
            plmnDesc->shrt_ext_dcs = current->shrt_ext_dcs;
            if (current->long_len)
            {
              memcpy (plmnDesc->longName, 
                      current->long_name,
                      plmnDesc->long_len);
            }

            if (current->shrt_len)
            {
              memcpy (plmnDesc->shrtName,
                      current->shrt_name,
                      plmnDesc->shrt_len);
            }
            return TRUE;
          }
        }
        else
          current = (T_pnn_name *)current->next;
      }
    }
  }

  if((ONSReadStatus EQ ONS_READ_OVER) AND cmhSIM_plmn_is_hplmn (mcc, mnc))
  {
    *plmnDesc = ONSDesc; /* Struct assignment */

    plmnDesc->mcc = mcc;
    plmnDesc->mnc = mnc;
    plmnDesc->pnn = Read_CPHS;
    return TRUE;
  }

/*
 *-------------------------------------------------------------------
 * search list of operators ( PCM table )
 *-------------------------------------------------------------------
 */
  recNr = 1;

  do
  {
/* Implements Measure#32: Row 110  */
    retVal= pcm_ReadRecord( (UBYTE *) ef_plmn_id, recNr, SIZE_EF_PLMN,
                            (UBYTE *)&plmn, &ver, &maxRec );

    if( retVal EQ PCM_INVALID_SIZE OR retVal EQ PCM_INVALID_RECORD )
      break;

    sim_mcc = (plmn.mcc[0] << 8) + plmn.mcc[1];
    sim_mnc = (plmn.mnc[0] << 8) + plmn.mnc[1];

    if (cmhSIM_plmn_equal_sim (mcc, mnc, sim_mcc, sim_mnc))
    {
      cmhMM_CnvrtTrmPCMOpNam( plmnDesc, &plmn );

      plmnDesc->mcc = sim_mcc;
      plmnDesc->mnc = sim_mnc;
      return TRUE;
    }

    recNr++;
  }
  while( recNr <= maxRec );

/*
 *-------------------------------------------------------------------
 * search list of operators ( fixed table )
 *-------------------------------------------------------------------
 */
#ifdef TI_PS_OP_OPN_TAB_ROMBASED
  /* Changes for ROM data */ 
plmn_comp_entry = ptr_plmn_compressed;

  while (cmhMM_decodePlmn (&oper, plmn_comp_entry) EQ 0)
  {
  /* Get first compressed PLMN entry */
    sim_mcc = oper.mcc;
    sim_mnc = oper.mnc;

    if (cmhSIM_plmn_equal_sim (mcc, mnc, sim_mcc, sim_mnc))
    {
      strncpy (plmnDesc->longName, 
               oper.longName, 
               MAX_LONG_OPER_LEN - 1);
      strncpy (plmnDesc->shrtName,
               oper.shrtName,
               MAX_SHRT_OPER_LEN - 1);
      plmnDesc->mcc      = oper.mcc;
      plmnDesc->mnc      = oper.mnc;
      return TRUE;
    }

    /* Next compressed PLMN entry */
    plmn_comp_entry += cmhMM_PlmnEntryLength (plmn_comp_entry);
  }


#else

for( i = 0;
       operListFixed[i].mcc NEQ -1 AND operListFixed[i].mnc NEQ -1;
       i++ )
  {
    sim_mcc = operListFixed[i].mcc;
    sim_mnc = operListFixed[i].mnc;

    if (cmhSIM_plmn_equal_sim (mcc, mnc, sim_mcc, sim_mnc))
    {
      strncpy (plmnDesc->longName, 
               operListFixed[i].longName, 
               MAX_LONG_OPER_LEN - 1);
      strncpy (plmnDesc->shrtName,
               operListFixed[i].shrtName,
               MAX_SHRT_OPER_LEN - 1);
      plmnDesc->mcc      = operListFixed[i].mcc;
      plmnDesc->mnc      = operListFixed[i].mnc;
      return TRUE;
    }
  }

#endif
/*
 *-------------------------------------------------------------------
 * only mnc and mcc description available
 *-------------------------------------------------------------------
 */
  plmnDesc->mcc = mcc;
  plmnDesc->mnc = mnc;
  /*
   * The competitor has here a more sophisticated algorithm:
   * If they know the country code, they don't display the
   * MCC in numerical representation, but an abbreviation
   * for the country. We are satisfied displaying the MCC
   * and saving the ROM space for the country table.
   */
  if ( ( mnc & 0xF) NEQ 0xf ) /* is 3rd digit mnc? */
  {
    sprintf (plmnDesc->longName, "%03x %03x", mcc, mnc);
  }
  else
  { /* only 2-digit-MNC */
    mnc = (mnc & 0x0FF0) >> 4;
    sprintf (plmnDesc->longName, "%03x %02x", mcc, mnc);
  }
  strcpy (plmnDesc->shrtName, plmnDesc->longName);

  return TRUE;
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_MM                  |
|                                 ROUTINE : cmhMM_isBandAllowed     |
+-------------------------------------------------------------------+

  PURPOSE : Check if band combination is part of bands allowed by manufacturer.
            To be used by AT%BAND.
            Per convention: if AllowedBands = 0, all bands are allowed.
                            band 0 is never allowed (that is not a combination !!)
*/

GLOBAL BOOL cmhMM_isBandAllowed( UBYTE band, UBYTE AllowedBands )
{
  if( AllowedBands EQ 0x00 )
  {
    /* no manufacturer constraint */
    return(TRUE);
  }
  if( band EQ 0x00)
  {
    return(FALSE);
  }

  if( (band & AllowedBands) EQ band )
  {
    return( TRUE );
  }
  else
    return( FALSE );
}

/*
+---------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_MM                    |
|                                 ROUTINE : cmhMM_writeSetBand        |
+---------------------------------------------------------------------+

  PURPOSE :

*/
#define SIZE_OF_RFCAP (16*sizeof(UBYTE))

GLOBAL BOOL cmhMM_writeSetBand( UBYTE setband )
{
#ifndef _SIMULATION_
  T_FFS_RET ret_ffs;
  UBYTE     RFCap[SIZE_OF_RFCAP]; /* RFCap has 16 bytes */

/* Implements Measure#32: Row 114 */
  ret_ffs = FFS_fread(gsm_com_rfcap_path,
                      RFCap,
                      SIZE_OF_RFCAP);
  if(ret_ffs < 0)  /* error */
  {
    TRACE_EVENT_P1("RFCap: cannot read FFS: error n: %d", ret_ffs);
    return(FALSE);
  }

  /* write user bands into FFS */
  RFCap[0] = setband;

/* Implements Measure#32: Row 114 */
  ret_ffs = FFS_fwrite(gsm_com_rfcap_path,
                       RFCap,
                       SIZE_OF_RFCAP);

  if( ret_ffs NEQ EFFS_OK )
  {
    TRACE_EVENT_P1("Cannot write value on RFCap, error n: %d", ret_ffs);
    return(FALSE);
  }
  TRACE_EVENT("cmhMM_writeSetBand: data writing in FFS successful");
#endif /* _SIMULATION_ */

  return( TRUE );
}

/*
+---------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_MM                    |
|                                 ROUTINE : cmhMM_getBandSettings     |
+---------------------------------------------------------------------+

  PURPOSE :
*/

GLOBAL BOOL cmhMM_getBandSettings( UBYTE *SetBands, UBYTE *AllowedBands )
{
  UBYTE intern_set_bands     = 0x00, /* initialized to NOT SET */
        intern_allowed_bands = 0x00; /* avoid several checks against NULL */
  BOOL ret = FALSE;
#ifndef _SIMULATION_
  UBYTE RFCap[SIZE_OF_RFCAP]; /* RFCap has 16 bytes */
  T_FFS_RET ret_ffs;

  TRACE_FUNCTION("cmhMM_getBandSettings()");

/* Implements Measure#32: Row 114 */
  ret_ffs = FFS_fread(gsm_com_rfcap_path,
                      RFCap,
                      SIZE_OF_RFCAP);
  if(ret_ffs < 0)  /* error */
  {
    TRACE_EVENT_P1("cmhMM_getBandSettings: RFCap: cannot read FFS: error n: %d", ret_ffs);
  }
  else
  {
    TRACE_EVENT("cmhMM_getBandSettings: data reading from FFS successful");
    intern_set_bands     = RFCap[0];
    intern_allowed_bands = RFCap[1];
    ret = TRUE;
  }
#endif /* #ifndef _SIMULATION_ */

  if( SetBands NEQ NULL )
  {
    *SetBands     = intern_set_bands;
    TRACE_EVENT_P1("User defined Band bitfield: %d", *SetBands);
  }
  if( AllowedBands NEQ NULL )
  {
    *AllowedBands = intern_allowed_bands;
    TRACE_EVENT_P1("Manufacturer defined Band bitfield: %d", *AllowedBands);
  }
  return (ret);
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_MM                  |
|                                 ROUTINE : cmhMM_FindNumeric       |
+-------------------------------------------------------------------+

  PURPOSE : Convert operator representation from string into numeric.

  NOTE:     The cmhMM_FindNumeric() function is not part of the public
            declared interface to ACI.
            However some guys from the MFW team are using this function 
            in mfw_nma.c, declaring the function prototype in the 
            nfw_nma.c file locally. Be careful with changes.

*/

GLOBAL BOOL cmhMM_FindNumeric( T_OPER_ENTRY * plmnDesc, const CHAR *numStr )
{
  USHORT idx;
  BOOL ready;

  TRACE_FUNCTION ("cmhMM_FindNumeric()");

  plmnDesc->mcc = 0;
  plmnDesc->mnc = 0;
  ready = FALSE;

  /*
   * Convert string representation into internal represention
   */
  for (idx = 0; idx < SIZE_MCC + SIZE_MNC; idx++)
  {
    if (idx < SIZE_MCC)
    {
      /*
       * Converting MCC
       */
      if (!ready AND
          numStr[idx] >= '0' AND numStr[idx] <= '9')
      {
        plmnDesc->mcc = (plmnDesc->mcc << 4) + numStr[idx] - '0';
      }
      else
      {
        ready = TRUE;
      }
    }
    else
    {
      /*
       * Converting MNC
       */
      if (!ready AND
          numStr[idx] >= '0' AND numStr[idx] <= '9')
      {
        plmnDesc->mnc = (plmnDesc->mnc << 4) + numStr[idx] - '0';
      }
      else
      {
        ready = TRUE;
        if ((plmnDesc->mnc & 0x00F) NEQ 0xF)
          plmnDesc->mnc = (plmnDesc->mnc << 4) + 0xF;
      }
    }
  }

/*
 *-------------------------------------------------------------------
 * search list of operators
 *-------------------------------------------------------------------
 */

  (void)cmhMM_FindPLMN (plmnDesc, plmnDesc->mcc, plmnDesc->mnc, NOT_PRESENT_16BIT, FALSE);

  /* decode 7 Bit default GSM for MFW */
  if (plmnDesc->pnn EQ Read_EONS)
  {
    /* Implements Measure 70 */
    cmhMM_decodePnnOpName( MAX_LONG_OPER_LEN, &(plmnDesc->long_len),
                                  plmnDesc->long_ext_dcs,
                           plmnDesc->longName );
    /* Implements Measure 70 */
    cmhMM_decodePnnOpName( MAX_SHRT_OPER_LEN, &(plmnDesc->shrt_len),
                                  plmnDesc->shrt_ext_dcs,
                           plmnDesc->shrtName );
  }
  return TRUE;
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_MM                  |
|                                 ROUTINE : cmhMM_FindName          |
+-------------------------------------------------------------------+

  PURPOSE : scan the list of operators to find the short or long name
            representation.
            
  NOTE:     The cmhMM_FindName() function is not part of the public
            declared interface to ACI.
            However some guys from the MFW team are using this function 
            in mfw_nma.c, declaring the function prototype in the 
            nfw_nma.c file locally. Be careful with changes.


*/

GLOBAL BOOL cmhMM_FindName( T_OPER_ENTRY * plmnDesc,
                      const CHAR *string, T_ACI_CPOL_FRMT format )
{
  EF_PLMN plmn;                       /* holds PLMN identifier */
  USHORT  maxRec;                     /* holds maximum records */
  USHORT  recNr;                      /* holds record number */
  UBYTE   retVal;                     /* holds return value */
  UBYTE   ver;                        /* holds version */
  char longName[(MAX_ALPHA_OPER_LEN*8+6)/7];
  char shrtName[(MAX_ALPHA_OPER_LEN*8+6)/7];

#ifdef TI_PS_OP_OPN_TAB_ROMBASED
 /* Changes for ROM data */ 
const UBYTE *plmn_comp_entry; /* get a local pointer holder */
T_OPER_ENTRY oper;
#else /* TI_PS_OP_OPN_TAB_ROMBASED */
USHORT  idx;                        /* holds list idx */
#endif /* TI_PS_OP_OPN_TAB_ROMBASED */

  /* Initial conditions for Data Coding Scheme and PLMN name source */
  memset (plmnDesc, 0, sizeof (T_OPER_ENTRY));
  plmnDesc->pnn = Read_ROM_TABLE;
  

/*
 *-------------------------------------------------------------------
 * search list of PNN records if SIM is EONS enabled
 *-------------------------------------------------------------------
 */
  if(psaSIM_ChkSIMSrvSup(SRV_PNN) AND psaSIM_ChkSIMSrvSup(SRV_OPL))
  {

    T_pnn_name *current;  /* points to PNN element currently compared */

    /* The current will be taken from PNNLst only if the pnn_rec_num
       is valid. Otherwise start with next pointer. */
    if (mmShrdPrm.PNNLst.pnn_rec_num NEQ 0)
    {
      current = &mmShrdPrm.PNNLst;
    }
    else
    {
      current = (T_pnn_name *)mmShrdPrm.PNNLst.next;
    }

    while (current NEQ NULL)
    {
      longName[0]=shrtName[0]='\0';
      if (current->long_len)
      {
        switch (current->long_ext_dcs>>4 & 0x07)
        {
          case 0x00:
            utl_cvtPnn7To8((UBYTE *)current->long_name,
                                    current->long_len,
                                    current->long_ext_dcs,
                           (UBYTE *)longName);
            break;
          case 0x01:
            TRACE_ERROR ("ERROR: Unhandled UCS2");
            break;
          default:
            TRACE_ERROR ("ERROR: Unknown DSC");
            break;
        }
      }
      if (current->shrt_len)
      {
        switch (current->shrt_ext_dcs>>4 & 0x07)
        {
          case 0x00:
            utl_cvtPnn7To8((UBYTE *)current->shrt_name,
                                    current->shrt_len,
                                    current->shrt_ext_dcs,
                           (UBYTE *)shrtName);
            break;
          case 0x01:
            TRACE_ERROR ("ERROR: Unhandled UCS2");
            break;
          default:
            TRACE_ERROR ("ERROR: Unknown DSC");
            break;
        }
      }

      /* To avoid the endless loop we need to check v_plmn along with the name
         checking. */
      if(((format EQ CPOL_FRMT_Long) AND (strncmp (string, (char *)longName, strlen(string)) EQ 0)) OR
         ((format EQ CPOL_FRMT_Short) AND (strncmp (string,(char *)shrtName, strlen(string)) EQ 0))
         AND (current->plmn.v_plmn EQ VLD_PLMN))
      
      {
        plmnDesc->pnn = Read_EONS;
        /* decode Name since MFW expects 8-Bit string */
        strncpy (plmnDesc->longName, longName, MAX_LONG_OPER_LEN - 1);
        plmnDesc->longName[MAX_LONG_OPER_LEN - 1] = '\0';
        plmnDesc->long_len = strlen (plmnDesc->longName);

        strncpy (plmnDesc->shrtName, shrtName, MAX_SHRT_OPER_LEN - 1);
        plmnDesc->shrtName[MAX_SHRT_OPER_LEN -1] = '\0';
        plmnDesc->shrt_len = strlen (plmnDesc->shrtName);

        cmhMM_CnvrtPLMN2INT( current->plmn.mcc,
                             current->plmn.mnc,
                             &plmnDesc->mcc, &plmnDesc->mnc );

        return TRUE;
      }
      else
      {
        current = (T_pnn_name *)current->next;
      }
    }
  }

  if((ONSReadStatus EQ ONS_READ_OVER) AND (strcmp (string, (char *)ONSDesc.longName) EQ 0))
  {

    *plmnDesc = ONSDesc; /* Struct assignment */

    plmnDesc->mcc = ONSDesc.mcc;
    plmnDesc->mnc = ONSDesc.mnc;
    plmnDesc->pnn = Read_CPHS;
    return TRUE;
  }

/*
 *-------------------------------------------------------------------
 * search list of operators ( PCM table )
 *-------------------------------------------------------------------
 */
  recNr = 1;

  do
  {
/* Implements Measure#32: Row 110 */
    retVal= pcm_ReadRecord( (UBYTE *) ef_plmn_id, recNr, SIZE_EF_PLMN,
                            (UBYTE *)&plmn, &ver, &maxRec );

    if( retVal EQ PCM_INVALID_SIZE OR retVal EQ PCM_INVALID_RECORD )
      break;

    cmhMM_CnvrtTrmPCMOpNam( plmnDesc, &plmn );

    if ((format EQ CPOL_FRMT_Short AND
         strcmp(plmnDesc->shrtName, string) EQ 0) OR
        (format EQ CPOL_FRMT_Long AND
         strcmp(plmnDesc->longName, string) EQ 0))
    {
      plmnDesc->mcc = (plmn.mcc[0] << 8) + plmn.mcc[1];
      plmnDesc->mnc = (plmn.mnc[0] << 8) + plmn.mnc[1];
      return TRUE;
    }

    recNr++;
  }
  while( recNr <= maxRec );

/*
 *-------------------------------------------------------------------
 * search list of operators ( fixed table )
 *-------------------------------------------------------------------
 */
#ifdef TI_PS_OP_OPN_TAB_ROMBASED
/* Changes for ROM data */ 
plmn_comp_entry = ptr_plmn_compressed;

  while (cmhMM_decodePlmn (&oper, plmn_comp_entry) EQ 0) 
  {
    if ((format EQ CPOL_FRMT_Short AND
         strcmp((char*)oper.shrtName, string) EQ 0) OR
        (format EQ CPOL_FRMT_Long AND
         strcmp((char*)oper.longName, string) EQ 0))
    {
      memset (plmnDesc, 0, sizeof (T_OPER_ENTRY));
      strncpy (plmnDesc->longName, 
               oper.longName,
               MAX_LONG_OPER_LEN - 1);
      strncpy (plmnDesc->shrtName,
               oper.shrtName,
               MAX_SHRT_OPER_LEN - 1);
      plmnDesc->mcc = oper.mcc;
      plmnDesc->mnc = oper.mnc;
      return TRUE;
    }
    /* Next compressed PLMN entry */
    plmn_comp_entry += cmhMM_PlmnEntryLength (plmn_comp_entry);
  }


#else
  for( idx = 0; operListFixed[idx].shrtName NEQ NULL; idx++ )
  {
    if ((format EQ CPOL_FRMT_Short AND
         strcmp((char*)operListFixed[idx].shrtName, string) EQ 0) OR
        (format EQ CPOL_FRMT_Long AND
         strcmp((char*)operListFixed[idx].longName, string) EQ 0))
    {
      memset (plmnDesc, 0, sizeof (T_OPER_ENTRY));
      strncpy (plmnDesc->longName, 
               operListFixed[idx].longName,
               MAX_LONG_OPER_LEN - 1);
      strncpy (plmnDesc->shrtName,
               operListFixed[idx].shrtName,
               MAX_SHRT_OPER_LEN - 1);
      plmnDesc->mcc = operListFixed[idx].mcc;
      plmnDesc->mnc = operListFixed[idx].mnc;
      return TRUE;
    }
  }
#endif

  return FALSE;
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_MM                  |
|                                 ROUTINE : cmhMM_CnvrtPLMN2INT     |
+-------------------------------------------------------------------+

  PURPOSE : convert the BCD PLMN notation for mcc and mnc into
            integer values.
*/

GLOBAL void cmhMM_CnvrtPLMN2INT( const UBYTE * BCDmcc, const UBYTE * BCDmnc,
                                       SHORT * mccBuf,       SHORT * mncBuf )
{
  SHORT idx;

  /*
   * Convert MCC
   */
  *mccBuf = 0;

  for (idx = 0; idx < SIZE_MCC; idx++)
  {
    *mccBuf = (*mccBuf << 4) + BCDmcc[idx];
  }

  /*
   * Convert MNC
   */
  *mncBuf = 0;

  for (idx = 0; idx < SIZE_MNC; idx++)
  {
    *mncBuf = (*mncBuf << 4) + BCDmnc[idx];
  }
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_MM                  |
|                                 ROUTINE : cmhMM_CnvrtINT2PLMN     |
+-------------------------------------------------------------------+

  PURPOSE : convert the integer PLMN notation for mcc and mnc into
            BCD representation.
*/

GLOBAL void cmhMM_CnvrtINT2PLMN( SHORT INTmcc, SHORT INTmnc,
                                 UBYTE * mccBuf, UBYTE * mncBuf )
{
  SHORT idx;
  SHORT shift;

  /*
   * Convert MCC
   */
  shift = 0;

  for (idx = SIZE_MCC - 1; idx >= 0; idx--)
  { /*lint -e{702} */
    mccBuf[idx] = (INTmcc >> shift) & 0xf;
    shift += 4;
  }

  /*
   * Convert MNC
   */
  shift = 0;

  for (idx = SIZE_MNC - 1; idx >= 0; idx--)
  { /*lint -e{702} */
    mncBuf[idx] = (INTmnc >> shift) & 0xf;
    shift += 4;
  }
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_MM                  |
|                                 ROUTINE : cmhMM_GetNregCREGStat   |
+-------------------------------------------------------------------+

  PURPOSE : return CREG status for a not registered state.
*/

GLOBAL T_ACI_CREG_STAT cmhMM_GetNregCREGStat( void )
{
  switch( mmShrdPrm.regMode )
  {
    case( MODE_AUTO ):

#if defined GPRS AND defined (DTI)
      if ( cmhGMM_isClassCG() )
        return(CREG_STAT_NoSearch);
      else
#endif
      {
       /* Depending on the deregistration cause proper CREG state has been sent to the user */
        switch(mmShrdPrm.deregCs)
        {
          case( NREG_LIMITED_SERVICE ):
            return(CREG_STAT_Denied);
          case( NREG_NO_SERVICE ):
             return(CREG_STAT_NoSearch);
          default:
            return(CREG_STAT_Unknown);
        }
      }
    case( MODE_MAN  ):
      return(CREG_STAT_NoSearch);

    default:
      return(CREG_STAT_Unknown);
  }
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_MM                  |
|                                 ROUTINE : cmhMM_GetNregCMEStat    |
+-------------------------------------------------------------------+

  PURPOSE : return CME error status for a not registered state.
*/

GLOBAL T_ACI_CME_ERR cmhMM_GetNregCMEStat( void )
{
  switch( mmShrdPrm.regStat )
  {
    case( NO_VLD_RS ):
    case( RS_NO_SRV ):

      return( CME_ERR_NoServ );

    case( RS_LMTD_SRV  ):

      return( CME_ERR_LimServ );

    default:

      return( CME_ERR_Unknown );
  }
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_MM                  |
|                                 ROUTINE : cmhMM_GetOperLstLen     |
+-------------------------------------------------------------------+

  PURPOSE : return CME error status for a not registered state.
*/

GLOBAL USHORT cmhMM_GetOperLstLen( void )
{
#ifdef TI_PS_OP_OPN_TAB_ROMBASED
  return (OPERLIST_FIXED_LEN); 
#else
  return((sizeof(operListFixed)/sizeof(T_OPER_NTRY_FIXED))-1);
#endif
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_MM                  |
|                                 ROUTINE : cmhMM_CnvrtTrmPCMOpNam  |
+-------------------------------------------------------------------+

  PURPOSE : Convert and terminate PCM long and short operator names.
*/

GLOBAL void cmhMM_CnvrtTrmPCMOpNam( T_OPER_ENTRY *plmnDesc, void *pPCMBuf )
{
  UBYTE len;
  EF_PLMN * pPlmn = (EF_PLMN *)pPCMBuf;

  /* convert and terminate long name */
  for( len = 0;
       len < SIZE_EF_PLMN_LONG AND pPlmn->lngNam[len] NEQ 0xFF;
       len++ )
    ;

  if (len > MAX_LONG_OPER_LEN - 1)
    len = MAX_LONG_OPER_LEN - 1;

  utl_cvtGsmIra( pPlmn->lngNam, len,
        (UBYTE *)plmnDesc->longName, len,
                 CSCS_DIR_GsmToIra );

  plmnDesc->longName[len] = '\0';

  /* convert and terminate short name */
  for( len = 0;
       len < SIZE_EF_PLMN_SHRT AND pPlmn->shrtNam[len] NEQ 0xFF;
       len++ )
    ;

  if (len > MAX_SHRT_OPER_LEN - 1)
    len = MAX_SHRT_OPER_LEN - 1;

  utl_cvtGsmIra( pPlmn->shrtNam, len,
        (UBYTE *)plmnDesc->shrtName, len,
                 CSCS_DIR_GsmToIra );

  plmnDesc->shrtName[len] = '\0';
}

/*
+----------------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH                              |
|                                 ROUTINE : cmhMM_Ntfy_NtwRegistrationStatus |
+----------------------------------------------------------------------------+

  PURPOSE : report function
*/
#ifdef FF_CPHS
LOCAL BOOL has_roaming_state_changed(T_CPHS_ROAMING_IND roaming_ind)
{
  static T_CPHS_ROAMING_IND last_roam_notified = CPHS_ROAMING_STAT_NotPresent;
  T_CPHS_ROAMING_IND previous_roam;

  TRACE_EVENT_P2("has_roaming_state_changed() roam: %d, previous roam: %d", roaming_ind, last_roam_notified);
  previous_roam = last_roam_notified;

  /* update last_roam_notified */
  last_roam_notified = roaming_ind;

  /* status has not changed at all */
  if(roaming_ind EQ previous_roam)
    return(FALSE);

  /* Mobile is entering Roaming service: since roaming is different from last roaming */
  if((roaming_ind EQ CPHS_INTERNATIONAL_ROAMING_ON) OR (roaming_ind EQ CPHS_NATIONAL_ROAMING_ON))
    return(TRUE);

  /* Mobile is not in Roaming service: Check whether this has already been told to the user */
  if((previous_roam EQ CPHS_INTERNATIONAL_ROAMING_ON) OR (previous_roam EQ CPHS_NATIONAL_ROAMING_ON))
    return(TRUE);

  return(FALSE);
}

LOCAL void cmhMM_cphs_roaming(T_ACI_CREG_STAT creg)
{
  SHORT mncCur, mccCur;       /* Holds MNC and MCC of current PLMN */
  SHORT sim_mcc;              /* Holds the MCC of the HPLMN from the SIM */
  SHORT sim_mnc;              /* Holds the MNC of the HPLMN from the SIM */
  T_CPHS_ROAMING_IND roaming_ind = CPHS_ROAMING_STAT_NotPresent;

  /* Decision on whether roaming within or outside the home country
   * made on the basis of the MCC of the selected PLMN.
   */
  if (creg EQ CREG_STAT_Roam)
  {
    cmhMM_CnvrtPLMN2INT( mmShrdPrm.usedPLMN.mcc,
                             mmShrdPrm.usedPLMN.mnc,
                             &mccCur, &mncCur );
    if(!cmhMM_GetActingHPLMN(&sim_mcc, &sim_mnc))/*Enhancement Acting HPLMN*/
    {
      /* Extract the HPLMN identification out of the IMSI digits. */
      cmhSIM_GetHomePLMN (&sim_mcc, &sim_mnc);
    }
    if (sim_mcc NEQ mccCur)
    {
      roaming_ind = CPHS_INTERNATIONAL_ROAMING_ON;
    }
    else
    {
      roaming_ind = CPHS_NATIONAL_ROAMING_ON;
    }
  }
  else if (creg EQ CREG_STAT_Reg)
  {
    roaming_ind = CPHS_ROAMING_OFF;
  }

  if(has_roaming_state_changed(roaming_ind))
  {
    /* Roaming state has changed */ 
    if (roaming_ind EQ CPHS_ROAMING_STAT_NotPresent)
    {
      roaming_ind = CPHS_ROAMING_OFF;
    }
    cphs_roaming_ind((UBYTE)roaming_ind);
  }
}
#endif /* FF_CPHS */

GLOBAL void cmhMM_Ntfy_NtwRegistrationStatus( T_ACI_CREG_STAT creg )
{
  SHORT src;
  T_ACI_P_CREG_GPRS_IND gprs_ind;
  U8    rt;  

  TRACE_FUNCTION ("cmhMM_Ntfy_NtwRegistrationStatus()");

#ifdef FF_CPHS
  cmhMM_cphs_roaming(creg);
#endif /* FF_CPHS */

#if defined (GPRS) AND defined (DTI)
  gprs_ind = gmmShrdPrm.gprs_indicator;  
  rt       = gmmShrdPrm.rt;
#else
  gprs_ind = P_CREG_GPRS_Not_Supported; /* ACI-SPR-17218: use ACI type */
  rt       = 0;
#endif

  /* notify GSM change in network status */
  /* +CREG */
  if( creg NEQ CREG_STAT_NotPresent )
  {
    mmShrdPrm.creg_status = creg;

    for( src = 0; src < CMD_SRC_MAX; src++ )
    {
      R_AT( RAT_CREG, (T_ACI_CMD_SRC)src )( creg, mmShrdPrm.lac, mmShrdPrm.cid );
      R_AT( RAT_P_CREG, (T_ACI_CMD_SRC)src )( creg, mmShrdPrm.lac, mmShrdPrm.cid, gprs_ind ,rt);      
    }
  }
}

/*
+-------------------------------------------------------------------------------------+
| PROJECT : GSM-PS                MODULE  : CMH                                       |
|                                 ROUTINE : cmhMM_OpCheckName                         |
+-------------------------------------------------------------------------------------+

  PURPOSE : Function for EONS support. Checks if the operator name should be
            read from EF_PNN upon registration or location update.
*/
GLOBAL BOOL cmhMM_OpCheckName()
{
  SHORT mncCur, mccCur;   /* holds converted mnc and mcc of current PLMN */
  SHORT mncOpl, mccOpl;
  T_opl_field *ptrOpl;
  UBYTE i, pnn_rec_num=0;
  BOOL ret = TRUE;

  TRACE_FUNCTION ("cmhMM_OpCheckName()");

  /* check if PNN is active and allocated on SIM */
  if(psaSIM_ChkSIMSrvSup(SRV_PNN))
  {
    /* check if OPL is active and allocated */
    /* if OPL not activaten and T_mobile EONS is not enabled then the first record of PNN holds the operator name of the HPLMN */
    if(!psaSIM_ChkSIMSrvSup(SRV_OPL) AND (cuscfgParams.T_MOBILE_Eons NEQ CUSCFG_MOD_Enable))
    {

       /* only read PNN if not already present */
      if(mmShrdPrm.PNNLst.plmn.v_plmn NEQ VLD_PLMN)
       {
        /* if HPLMN: this sequence is copied from cmhMM_Registered() */
        cmhMM_CnvrtPLMN2INT( mmShrdPrm.usedPLMN.mcc,
                             mmShrdPrm.usedPLMN.mnc,
                             &mccCur, &mncCur );
        if( cmhSIM_plmn_is_hplmn (mccCur, mncCur))
        {
          ret = cmhMM_OpReadName(1);
        }
      }
      return ret;
     }
    /* check if the registered PLMN or LAC has changed since last registration */

    if (memcmp (mmShrdPrm.usedPLMN.mcc, mmShrdPrm.PNNLst.plmn.mcc, SIZE_MCC) NEQ 0 OR
        memcmp (mmShrdPrm.usedPLMN.mnc, mmShrdPrm.PNNLst.plmn.mnc, SIZE_MNC) NEQ 0 OR
        mmShrdPrm.lac NEQ mmShrdPrm.PNNLst.lac) 
     {
      ptrOpl = cmhSIM_GetOPL();
      if (ptrOpl->opl_status)
      {
        /* search the OPL list for the new PNN record number      */
        i = 0;
        pnn_rec_num = 0; /* if PNN record number in OPL is 0, no PNN name is available. */
        while (i < ptrOpl->num_rcd)
        {   
          cmhMM_CnvrtPLMN2INT( mmShrdPrm.usedPLMN.mcc,
                             mmShrdPrm.usedPLMN.mnc,
                             &mccCur, &mncCur );

          cmhSIM_getMncMccFrmPLMNsel (ptrOpl->opl_rcd[i].plmn,
                                      &mccOpl, &mncOpl);

          if(cmhSIM_plmn_equal_sim_wc (mccCur, mncCur, mccOpl, mncOpl) AND
             (ptrOpl->opl_rcd[i].lac1 <= mmShrdPrm.lac) AND
             (mmShrdPrm.lac <= ptrOpl->opl_rcd[i].lac2))
          {
            pnn_rec_num = ptrOpl->opl_rcd[i].pnn_rec_num;
            break;
          }
          else 
            i++;
        }
        if (pnn_rec_num NEQ 0)
        {
          /* only read PNN if it is different from the last one saved */
          if (mmShrdPrm.PNNLst.pnn_rec_num NEQ pnn_rec_num)
          {
            ret = cmhMM_OpReadName(pnn_rec_num);
            mmShrdPrm.PNNLst.plmn.v_plmn = INVLD_PLMN;
            return ret;
          }
        }
      }
      else
       TRACE_EVENT("OPL list unavailable");; /* no OPL list available */
    }
  }

/* if CPHS ONS file is not already read then read it */
  if (!pnn_rec_num AND (ONSReadStatus EQ ONS_READ_NOT_DONE))
  {
#ifdef FF_CPHS
    ret = cmhMM_ONSReadName();
#endif
  }

  return ret;
}
/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)    MODULE  : CMH_SMSF                     |
| STATE   : code             ROUTINE : cmhMM_OpReadName             |
+-------------------------------------------------------------------+

  PURPOSE : Sends a SIM read request

  RESULT: has an error occured ?
  Returns: FALSE if primitive has been sent and TRUE if not
*/
GLOBAL BOOL cmhMM_OpReadName(UBYTE rcd)
{
  TRACE_FUNCTION ("cmhMM_OpReadName()");

  if (cmhSIM_ReadRecordEF (CMD_SRC_NONE,
                           AT_CMD_NONE,
                           FALSE,
                           NULL,
                           SIM_PNN,
                           rcd,
                           NOT_PRESENT_8BIT,
                           NULL,
                           cmhMM_OpReadNameCb) EQ AT_FAIL)
  {
    TRACE_EVENT("FATAL ERROR");
    return(TRUE);
  }
  return(FALSE);
}

/*
+---------------------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)    MODULE  : CMH_SMSF                                   |
| STATE   : code             ROUTINE : cmhMM_OpReadNameCb                         |
+---------------------------------------------------------------------------------+

  PURPOSE : Call back for SIM retrieval of EF_PNN.
*/
GLOBAL void cmhMM_OpReadNameCb(SHORT table_id)
{
  UBYTE *data;
  TRACE_FUNCTION ("cmhMM_OpReadNameCb()");

  data = simShrdPrm.atb[table_id].exchData;

  /* Decode and copy PNN record data to PNN list*/
  /*------------------------------------------*/

  if (simShrdPrm.atb[table_id].errCode EQ SIM_NO_ERROR) /* Process only if file is read */
  {

    mmShrdPrm.PNNLst.plmn = mmShrdPrm.usedPLMN;
    mmShrdPrm.PNNLst.lac = mmShrdPrm.lac;
    mmShrdPrm.PNNLst.pnn_rec_num = simShrdPrm.atb[table_id].recNr;

    if (*data++ EQ PNN_LONG_NAME_IEI)
    {
        mmShrdPrm.PNNLst.long_len  = (*data++)-1; /* substract dcs byte from lem */
        mmShrdPrm.PNNLst.long_ext_dcs = *data++;
        memcpy (mmShrdPrm.PNNLst.long_name,
                data,
                MINIMUM(mmShrdPrm.PNNLst.long_len, sizeof(mmShrdPrm.PNNLst.long_name)));
        data += mmShrdPrm.PNNLst.long_len;
  
        /* PNN_SHORT_NAME_IEI is an optional field */
        if (*data++ EQ PNN_SHORT_NAME_IEI)
        {
            mmShrdPrm.PNNLst.shrt_len = (*data++)-1; /* substract dcs byte from len */
            mmShrdPrm.PNNLst.shrt_ext_dcs = *data++;
            memcpy (mmShrdPrm.PNNLst.shrt_name,
                    data,
                    MINIMUM(mmShrdPrm.PNNLst.shrt_len, sizeof(mmShrdPrm.PNNLst.shrt_name)));
        }
        else
        {
            mmShrdPrm.PNNLst.shrt_len = 0;
        }
        mmShrdPrm.PNNLst.next = NULL;
    }
    else /* PNN_LONG_NAME_IEI is a mandatory field in PNN, if not present, PNN is invalid */
    {
        /* PNN record number 0 indicates no PNN name is available */
        mmShrdPrm.PNNLst.pnn_rec_num = 0;
    }
  }
  cmhMM_Registered ();
  simShrdPrm.atb[table_id].ntryUsdFlg = FALSE;
}

/*
+---------------------------------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)    MODULE  : CMH_SMSF                                               |
| STATE   : code             ROUTINE : cmhMM_OpSetPNNLst                                      |
+---------------------------------------------------------------------------------------------+

  PURPOSE : starts reading of EF_PNN from SIM and creates PNN list for manual PLMN selection.
*/
GLOBAL void cmhMM_OpSetPNNLst()
{
  T_opl_field *ptrOpl;
  UBYTE i,j, pnn_rec_num;

  TRACE_FUNCTION ("cmhMM_OpSetPNNLst()");
 
  ptrOpl = cmhSIM_GetOPL();
  if (ptrOpl->opl_status)
  {
    mmShrdPrm.pnn_read_cnt = 0;
    /* search the OPL list for the PNN record number
       for every entry in the network search lists */    

    /* OPL list is searched for every entry in the network search lists */
    for (j=0; j < MAX_PLMN_ID; j++) 
    {
      /* check if entry in PLMNLst is valid */
      if( mmShrdPrm.PLMNLst[j].v_plmn NEQ INVLD_PLMN )
      {
        pnn_rec_num = 0; /* if PNN record number in OPL is 0, no PNN name is available. */
        i = 0;
        ptrOpl = cmhSIM_GetOPL();
        while (i < ptrOpl->num_rcd) /* search OPL list for PLMN nr. j */
        {
          SHORT bcch_mcc, bcch_mnc;
          SHORT sim_mcc, sim_mnc;

          cmhMM_CnvrtPLMN2INT (mmShrdPrm.PLMNLst[j].mcc, mmShrdPrm.PLMNLst[j].mnc,
                               &bcch_mcc, &bcch_mnc);
          cmhSIM_getMncMccFrmPLMNsel (ptrOpl->opl_rcd[i].plmn,
                                      &sim_mcc, &sim_mnc);
          if (cmhSIM_plmn_equal_sim (bcch_mcc, bcch_mnc, sim_mcc, sim_mnc) AND
              ((ptrOpl->opl_rcd[i].lac1 EQ 0x0000 AND
                ptrOpl->opl_rcd[i].lac2 EQ 0xFFFE) OR
               (ptrOpl->opl_rcd[i].lac1 <= mmShrdPrm.LACLst[j] AND
                mmShrdPrm.LACLst[j] <= ptrOpl->opl_rcd[i].lac2)))
          {
            pnn_rec_num = ptrOpl->opl_rcd[i].pnn_rec_num;
            break;
          }
          else 
            i++;
        }
        if (pnn_rec_num NEQ 0)
        {
          if (cmhSIM_ReadRecordEF (CMD_SRC_NONE,
                                   AT_CMD_NONE,
                                   FALSE,
                                   NULL,
                                   SIM_PNN,
                                   pnn_rec_num, /* the PNN rec. number is read */
                                   NOT_PRESENT_8BIT,
                                   NULL,
                                   cmhMM_OpExtractNameCB) EQ AT_FAIL)
          {
             TRACE_EVENT("FATAL ERROR");
             break; /* try to continue with the next one instead of return
                       since otherwise AT+COPS=? will hang if cmhMM_NetworkLst() is not called! */
          }
          /* in cmhMM_OpExtractNameCB the latest received PNN is placed right after the PNN-list head */
          /* pnn_nr array is used for finding the associated plmn and lac in cmhMM_OpExtractNameCB. */
          mmShrdPrm.pnn_nr[j] = pnn_rec_num;
          mmShrdPrm.pnn_read_cnt++;
        }
        else
        {
          /* name is to be taken from other sources, 3G TS 22.101 */
        }
      }
      else
      {
        break; /* and Invalid PLMN indicates the end of the list. */
      }
    }
    if (mmShrdPrm.pnn_read_cnt EQ 0)  /* nothing processed? */
    {
      if(( mmEntStat.curCmd EQ AT_CMD_COPS) OR ( mmEntStat.curCmd EQ AT_CMD_P_COPS) )
        cmhMM_NetworkLst();             /* then print list anyway, otherwise AT+COPS=? will fail. */
    }
  }
  else
  {
    if(( mmEntStat.curCmd EQ AT_CMD_COPS) OR ( mmEntStat.curCmd EQ AT_CMD_P_COPS) )
      cmhMM_NetworkLst();             /* then print list anyway, otherwise AT+COPS=? will fail. */
  }
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)    MODULE  : CMH_SMSF                     |
| STATE   : code             ROUTINE : cmhMM_OpExtractName          |
+-------------------------------------------------------------------+

  PURPOSE : decodes EF_PNN record read from SIM.
*/
GLOBAL void cmhMM_OpExtractNameCB(SHORT table_id)
{
  UBYTE *data;
  UBYTE i;
  T_pnn_name *newPNN;

  TRACE_FUNCTION ("cmhMM_OpExtractNameCB()");

  data = simShrdPrm.atb[table_id].exchData;

  if (*data++ EQ PNN_LONG_NAME_IEI)
  {
    MALLOC(newPNN,sizeof(T_pnn_name));
    newPNN->next = NULL;

    newPNN->pnn_rec_num = simShrdPrm.atb[table_id].recNr;
  
    /*find the associated PLMN and LAC for this PNN */
    i = 0;
    while (i < MAX_PLMN_ID)
    {
      if (mmShrdPrm.pnn_nr[i] EQ newPNN->pnn_rec_num)
      {
        newPNN->plmn = mmShrdPrm.PLMNLst[i];
        newPNN->lac = mmShrdPrm.LACLst[i];
        break;
      }
      else
        i++;
    }

    newPNN->long_len = (*data++)-1;   /* substract dcs byte */
    newPNN->long_ext_dcs = *data++;
    memcpy (newPNN->long_name,
            data,
            MINIMUM(newPNN->long_len, sizeof(newPNN->long_name)));
    data += newPNN->long_len;

     /*----- IEI PNN short name ------*/
    if (*data++ EQ PNN_SHORT_NAME_IEI)
    {
      newPNN->shrt_len  = (*data++)-1;  /* substract dcs byte */
      newPNN->shrt_ext_dcs = *data++;
      memcpy (newPNN->shrt_name,
              data,
              MINIMUM(newPNN->shrt_len,sizeof(newPNN->shrt_name)));
    } 
    else
    {
      newPNN->shrt_len = 0;
    }

    /* insert new element right after the static stored PNN name.  */
    newPNN->next = mmShrdPrm.PNNLst.next;
    mmShrdPrm.PNNLst.next = newPNN; 
  }

  mmShrdPrm.pnn_read_cnt--; /* reading of PNN records completed, network list is prepared */
  if (mmShrdPrm.pnn_read_cnt EQ 0)
    cmhMM_NetworkLst ();    

  /* Deallocation of elements in linked list PNN */
  // As you see, you see nothing.
  // ============================

  // Immediate deallocation is not possible as someone could have
  // the idea to do a AT+COPS=? and immediately thereafter a 
  // AT+COPS=1,0,"T-Mobile D" or AT+COPS=1,1,"TMO D"
  // A workaround could be to free everything not present in the actual
  // network list. Until now only a theoretical problem. For further study.

  // There is a second problem that this MM EONS extension list is not
  // affected by SAT changes of the respective files. Until now also 
  // only theoretical and for further study.

  // And there is a third problem. This is, there exist 2 solutions
  // in the PS for EONS, the first one sitting in SIM related ACI around
  // the T_pnn data structure and the second one sitting in MM related ACI
  // around the T_pnn_name data structure. Two solutions for the same
  // problem.

  // => We should consider in the future to remove the implementation around
  // T_pnn_name and to enhance the T_pnn in SIM to end up in only one
  // implementation. This would improve the PS and save ROM.
  // Maybe the increasement of PNN_MAX_RECORDS to MAX_PLMN_ID + 1 or
  // something somewhat bigger will already do, otherwise an 
  // intelligent entry replacement (aging) algorithm has to be introduced.
  // For further study.

  simShrdPrm.atb[table_id].ntryUsdFlg = FALSE;

}

/*
+-------------------------------------------------------------------------------------+
| PROJECT : GSM-PS                MODULE  : CMH                                       |
|                                 ROUTINE : cmhMM_OpUpdateName                        |
+-------------------------------------------------------------------------------------+

  PURPOSE : Function for EONS support. Updates the operator name by reading EF_PNN.
  RETURNS: false if PS busy with reading files (primitive has been sent)
  TRUE if PS can go on with the updating
*/
GLOBAL BOOL cmhMM_OpUpdateName()
{
  SHORT mncCur, mccCur;   /* holds converted mnc and mcc of current PLMN */
  T_opl_field *ptrOpl;
  UBYTE i, pnn_rec_num;

  TRACE_FUNCTION ("cmhMM_OpUpdateName()");

  /* check if OPL is active and allocated */
  /* if OPL not activate the first record of PNN holds the operator name of the HPLMN */
  if(!psaSIM_ChkSIMSrvSup(SRV_OPL))
  {
    cmhMM_CnvrtPLMN2INT( mmShrdPrm.usedPLMN.mcc,
                         mmShrdPrm.usedPLMN.mnc,
                         &mccCur, &mncCur );
    if( cmhSIM_plmn_is_hplmn (mccCur, mncCur))
    {
      return(cmhMM_OpReadName(1));
    }
    return(TRUE); /* no primitive has been sent to SIM */
  }
  else /* OPL is active and allocated */
  {
    ptrOpl = cmhSIM_GetOPL();
    if (ptrOpl->opl_status)
    {
      /* search the OPL list for the new PNN record number      */
      i = 0;
      pnn_rec_num = 0; /* if PNN record number in OPL is 0, no PNN name is available. */
      while (i < ptrOpl->num_rcd)
      {
        SHORT bcch_mcc, bcch_mnc;
        SHORT sim_mcc, sim_mnc;

        cmhMM_CnvrtPLMN2INT (mmShrdPrm.usedPLMN.mcc, mmShrdPrm.usedPLMN.mnc,
                             &bcch_mcc, &bcch_mnc);
        cmhSIM_getMncMccFrmPLMNsel (ptrOpl->opl_rcd[i].plmn,
                                    &sim_mcc, &sim_mnc);
        if (cmhSIM_plmn_equal_sim (bcch_mcc, bcch_mnc, sim_mcc, sim_mnc) AND 
            (ptrOpl->opl_rcd[i].lac1 <= mmShrdPrm.lac) AND
            (mmShrdPrm.lac <= ptrOpl->opl_rcd[i].lac2))
        {
          pnn_rec_num = ptrOpl->opl_rcd[i].pnn_rec_num;
          break;
        }
        else 
          i++;
      }
      if (pnn_rec_num NEQ 0)
      {
        return(cmhMM_OpReadName(pnn_rec_num));
      }
    }
    else
     TRACE_EVENT("OPL list unavailable");; /* no OPL list available */
  }
  return(TRUE); /* no primitive has been sent to SIM */
}

/*
+-------------------------------------------------------------------------------------+
| PROJECT : GSM-PS                MODULE  : CMH                                       |
|                                 ROUTINE : cmhMM_GetCmerSettings                     |
+-------------------------------------------------------------------------------------+
*/
GLOBAL void cmhMM_GetCmerSettings(T_ACI_CMD_SRC srcId, T_ACI_MM_CMER_VAL_TYPE *sCmerSettings)
{
  sCmerSettings->sCmerModeParam = cmhPrm[srcId].mmCmdPrm.sIndicationParam.sMmCMERSettings.sCmerModeParam;
  sCmerSettings->sCmerIndParam  = cmhPrm[srcId].mmCmdPrm.sIndicationParam.sMmCMERSettings.sCmerIndParam;
  sCmerSettings->sCmerBfrParam  = cmhPrm[srcId].mmCmdPrm.sIndicationParam.sMmCMERSettings.sCmerBfrParam;
}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_MMF                  |
| STATE   : code                  ROUTINE : cmhMM_ChkIgnoreECC       |
+--------------------------------------------------------------------+

  PURPOSE : This function is used to check if ECC should be set
            as a normal call. (Issue 17570)
  RETURN: TRUE - if further ECC checking should be ignored and call must be placed as normal
                FALSE - if ECC checking should be continued
*/

GLOBAL BOOL cmhMM_ChkIgnoreECC(CHAR *dialStr)
{
  int i=0;
  SHORT mnc, mcc;

  /* remove any CLIR suppression/invocation prior checking for ECC */
/* Implements Measure#32: Row 116 & 117 */
  if (!strncmp( dialStr, ksd_supp_clir_str, 4) OR
      !strncmp( dialStr, ksd_inv_clir_str, 4) )
  {
    dialStr+=4;                  /* skip CLIR supression/invocation digits */
    if ( *dialStr EQ '\0' )
    {
      return( FALSE );  /* end already reached? */
    }
  }

  if(mmShrdPrm.usedPLMN.v_plmn EQ VLD_PLMN)
  {
    cmhMM_CnvrtPLMN2INT( mmShrdPrm.usedPLMN.mcc, mmShrdPrm.usedPLMN.mnc, &mcc, &mnc );

    while (ECCIgnoreTable[i].mcc NEQ -1 AND ECCIgnoreTable[i].mnc NEQ -1 )
    {
      if ( ECCIgnoreTable[i].mcc EQ mcc AND
          (ECCIgnoreTable[i].mnc EQ mnc OR ECCIgnoreTable[i].mnc EQ ALL_MNC) )
      {
        if(!strcmp(ECCIgnoreTable[i].ecc, dialStr))
        {
          TRACE_EVENT("cmhCC_ChkIgnoreECC(): no ECC but normal call detected");
          return (TRUE);
        }
      }
      i++;
    }
  }
  return (FALSE);
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_MMF                  |
| STATE   : code                  ROUTINE : cmhMM_OperatorSelect       |
+--------------------------------------------------------------------+

  PURPOSE : 

  This function encapsulates the common CMH functionality of 
  sAT_PlusCOPS(),   sAT_PercentCOPS(), and sAT_PercentNRG(). 

  The body of this function is based on the former sAT_PercentNRG().
  
*/

GLOBAL T_ACI_RETURN cmhMM_OperatorSelect(T_ACI_CMD_SRC srcId,
                                         T_ACI_NRG_RGMD regMode,
                                         T_ACI_NRG_SVMD srvMode,
                                         T_ACI_NRG_FRMT oprFrmt,
                                         CHAR          *opr)
{
  T_MM_CMD_PRM  * pMMCmdPrm;  /* points to MM command parameters */
  T_SIM_SET_PRM * pSIMSetPrm; /* points to SIM parameter set */
  T_ACI_RETURN    retVal=AT_FAIL;  /* holds return value */
  UBYTE rm;                   /* holds converted registration mode */

  TRACE_FUNCTION ("cmhMM_OperatorSelect()");

/*
 *-------------------------------------------------------------------
 * check command source
 *-------------------------------------------------------------------
 */
  if(!cmh_IsVldCmdSrc (srcId))
  {
    mmEntStat.curCmd  = AT_CMD_NONE; /*Unset the Current Command */
    return( AT_FAIL );
  }

  pMMCmdPrm  = &cmhPrm[srcId].mmCmdPrm;
  pSIMSetPrm = &simShrdPrm.setPrm[srcId];
  mmShrdPrm.regModeBeforeAbort = mmShrdPrm.regMode;

/*
 *-------------------------------------------------------------------
 * process the regMode parameter
 *-------------------------------------------------------------------
 */

  switch( regMode )
  {
    case( NRG_RGMD_Auto   ): rm = MODE_AUTO; break;
    case( NRG_RGMD_Manual ):
    case( NRG_RGMD_Both   ): rm = MODE_MAN;  
      if(!cmhSIM_isplmnmodebit_set())
      {
        mmEntStat.curCmd  = AT_CMD_NONE; /*Unset the Current Command */
        ACI_ERR_DESC( ACI_ERR_CLASS_Cme, CME_ERR_OpNotAllow );
        return( AT_FAIL );
      }
      break;
    case( NRG_RGMD_Dereg   ): rm = MODE_AUTO;  break; 

    default:
      mmEntStat.curCmd  = AT_CMD_NONE; /*Unset the Current Command */
      ACI_ERR_DESC( ACI_ERR_CLASS_Ext, EXT_ERR_Parameter );
      return( AT_FAIL );
  }


/*
 *-------------------------------------------------------------------
 * process the srvMode parameter
 *-------------------------------------------------------------------
 */

  switch( srvMode )
  {
    /*
     *---------------------------------------------------------------
     * setting of registration mode only. This is only used by %NRG. It is obsolete and
     * should be removed at some point in the future.
     *---------------------------------------------------------------
     */
    case( NRG_SVMD_SetRegModeOnly ):

      /*
        * Unset the Current Command  to match exactly what sAT_PercentNRG()
        * used to do.
      */
      mmEntStat.curCmd  = AT_CMD_NONE; 
      mmShrdPrm.regMode = rm;
      mmShrdPrm.regModeAutoBack = FALSE;

      /*
       * Documentation (8415.052.00.002) says:
       *
       * "<srvMode>=3 can be used to change the behaviour of registration
       * in case of a loss of coverage. If connection to the operator is
       * lost and <regMode> was set to manual, ME tries to register the previous
       * operator automatically.". This is not exactly what is done here.
       * What is done is that the registration mode is set to automatic in MM.
       * The main difference is that for phase 2+ the HPLMN search period
       * will be evaluated by RR and the HPLMN may be reselected after 30 minutes,
       * even without any loss of coverage.
       */


#ifdef  GPRS
      if( psaG_MM_CMD_SET_REGMD ( rm ) < 0 )  /* restore former registration mode*/
#else
      if( psaMM_SetRegMode ( rm ) < 0 ) /* set registration mode */
#endif
      {
        TRACE_EVENT( "FATAL RETURN psaMM_SetRegMode in %%NRG" );
        ACI_ERR_DESC( ACI_ERR_CLASS_Ext, EXT_ERR_Internal );
        retVal = AT_FAIL;
      }
      else
        retVal = AT_CMPL;
      break;

    /*
     *---------------------------------------------------------------
     * registration to full service
     *---------------------------------------------------------------
     */
    case( NRG_SVMD_Full ):

      /* For the %NRG case, only if SIM data is available we can start a registration attempt 
       * But the COPS commands dont require that.
       */
     
      if(( simShrdPrm.SIMStat EQ SS_OK AND
          simShrdPrm.PINStat EQ PS_RDY )
          OR (mmEntStat.curCmd NEQ AT_CMD_NRG))
      {
        switch( regMode )
        {
          case( NRG_RGMD_Auto ):  /* automatic registration */

            mmShrdPrm.regModeAutoBack = FALSE;
            mmShrdPrm.regMode = MODE_AUTO;
            mmShrdPrm.owner = (T_OWN)srcId;
            mmEntStat.entOwn = srcId;			


#ifdef GPRS 
      /*   Taken from the former sAT_PlusCOPS()
       *    COPS will be request an only GSM registration.
       *    If the ME is an only GPRS mobile, then it is impossible to request only GSM.
       *    This patch will be eliminate an error in this situation.
       *
       *    brz, 02.07.01
       */
      if ( cmhGMM_class_eq_CG() EQ TRUE )
      {
         mmEntStat.curCmd  = AT_CMD_NONE; /*Unset the Current Command */
         return AT_CMPL;
      }  
#endif


#ifdef  GPRS
            if( psaG_MM_CMD_REG ( ) < 0 )  /* register to network */
#else
            if( psaMM_Registrate () < 0 )  /* register to network */
#endif
            {
              TRACE_EVENT( "FATAL RETURN psaMM_Registrate in cmhMM_OperatorSelect()" );
              ACI_ERR_DESC( ACI_ERR_CLASS_Ext, EXT_ERR_Internal );
              retVal = AT_FAIL;
              break;
            }

            cmhMM_Ntfy_NtwRegistrationStatus(CREG_STAT_Search);
            retVal = AT_EXCT;
            break;

          case( NRG_RGMD_Manual ): /* manual registration               */
          case( NRG_RGMD_Both   ): /* manual followed by automatic reg. */

            mmShrdPrm.regModeAutoBack = (regMode EQ NRG_RGMD_Both);

            /*
              * The following code is for %COPS=1 and %NRG=1. 
              * It will get the IMSI and last registered PLMN from FFS.
              * If the IMSI has changed or no information is available in FFS, 
              * it will tell MM to register to the operator stored in EF(LOCI). 
              * If the IMSI hasn't changed, it will manually register to the Operator stored in FFS.
              *  
              */
            if( !opr OR !*opr) 
            {
                UBYTE mcc[SIZE_MCC];
                UBYTE mnc[SIZE_MNC];
                UBYTE IMSI[MAX_IMSI-1];
                char tempIMSI[MAX_IMSI] = {'\0'};
                BOOL readFromLOCI =FALSE;
      
                
                mmShrdPrm.slctPLMN.v_plmn = VLD_PLMN;
              
               /* Get   the IMSI and last registered PLMN from FFS. 
                 * Compare the IMSI stored in FFS to the current one. If the same, copy MCC and MNC
                 * to pMMSetPrm -> slctPLMN. This will select this network manually. This has to be 
                 * done only when the curCmd is %COPS. For other commands we need to get it 
                 * from EF-LOCI.
                 */

                if  (cmhMM_OperatorReadFromFFS(mcc,mnc,IMSI) AND 
                     (mmEntStat.curCmd EQ AT_CMD_P_COPS))
                {
                   TRACE_EVENT("Operator and IMSI succesfully read from FFS!");
                   if (!memcmp(IMSI,simShrdPrm.imsi.field,MAX_IMSI-1))
                   {
                       memcpy(mmShrdPrm.slctPLMN.mcc,  mcc,SIZE_MCC);
                       memcpy(mmShrdPrm.slctPLMN.mnc,  mnc,SIZE_MNC);                  
                   }
                   else
                   {
                      readFromLOCI = TRUE;
                   }  
                }
                else
                   readFromLOCI = TRUE;
                  /* If the SIM has changed or FFS cannot be read, read EF(LOCI) from the SIM.
                    *  This wil now lead to a PLMN_RES with MNC,MCC=0xFFF: 
                    */
                  if (readFromLOCI)
                  {
                     psaSIM_cnvrtIMSI2ASCII(tempIMSI);
                     psaSIM_decodeIMSI(IMSI,simShrdPrm.imsi.c_field,tempIMSI);
                     
                     cmhMM_CnvrtINT2PLMN( 0xFFF,
                                   0xFFF,
                                   mmShrdPrm.slctPLMN.mcc,
                                   mmShrdPrm.slctPLMN.mnc );
                  }
             }
            else
            {
              /* Implements Measure 118 */
              if ( cmhMM_cvtPLMNINT2BCD( oprFrmt, opr ) EQ FALSE )
              {
                return( AT_FAIL );
              }
            }
            mmShrdPrm.regMode = MODE_MAN;
            mmShrdPrm.owner = (T_OWN)srcId;
            mmEntStat.entOwn  = srcId;

#ifdef  GPRS
            if( psaG_MM_CMD_NET_SEL ( ) < 0 )  /* register to network */
#else
            if( psaMM_NetSel () < 0 )  /* register to network */
#endif
            {
              TRACE_EVENT( "FATAL RETURN psaMM_NetSel in %%NRG" );
              ACI_ERR_DESC( ACI_ERR_CLASS_Ext, EXT_ERR_Internal );
              retVal = AT_FAIL;
              break;
            }

            cmhMM_Ntfy_NtwRegistrationStatus(CREG_STAT_Search);

            retVal = AT_EXCT;
            break;
        }
      }
      /* 
       * No SIM data is available, activate SIM 
       * The following block of code was only available in the former sAT_PercentNRG()
       * Therefore, it will only be executed by the %NRG command.
       */
      else
      {
        /* check SIM entity status */
        if( simEntStat.curCmd NEQ AT_CMD_NONE )

          return( AT_BUSY );

        /* prepare PLMN desc for later use */
        if( regMode EQ NRG_RGMD_Manual )
        {
          if( ! opr )
          {
            mmEntStat.curCmd  = AT_CMD_NONE; /*Unset the Current Command */
            ACI_ERR_DESC( ACI_ERR_CLASS_Ext, EXT_ERR_Parameter );
            return( AT_FAIL );
          }
          /* Implements Measure 118 */
          if ( cmhMM_cvtPLMNINT2BCD( oprFrmt, opr ) EQ FALSE )
          {
            return( AT_FAIL );
          }
        }

        pSIMSetPrm -> actProc = SIM_INITIALISATION;

        simEntStat.curCmd = AT_CMD_NRG;
        simShrdPrm.owner = (T_OWN)srcId;
        simEntStat.entOwn = srcId;

        if( psaSIM_ActivateSIM() < 0 )   /* activate SIM card */
        {
          TRACE_EVENT( "FATAL RETURN psaSIM in %%NRG" );
          ACI_ERR_DESC( ACI_ERR_CLASS_Ext, EXT_ERR_Internal );
          retVal =  AT_FAIL;
          break;
        }

        retVal = AT_EXCT;
      }
      break;

    /*
     *---------------------------------------------------------------
     * registration to limited service.
     * The COPS commands will use this for deregistration
     *---------------------------------------------------------------
     */

    case( NRG_SVMD_Limited ):

      mmShrdPrm.regModeAutoBack = FALSE;

      switch( mmShrdPrm.regStat )
      {
        case( RS_LMTD_SRV ):

          mmEntStat.curCmd  = AT_CMD_NONE; /*Unset the Current Command */
          retVal = AT_CMPL;
          break;

        case( RS_NO_SRV ):
        case( NO_VLD_RS ):
          /*
           * if SIM data is available, do not destory it in ACI.
           * It may be needed later
           */
          if(( simShrdPrm.SIMStat EQ SS_OK AND
              simShrdPrm.PINStat EQ PS_RDY ) OR (mmEntStat.curCmd NEQ AT_CMD_NRG))
          {

           /* flag a pending request. It only applies to the %NRG case */
            if (mmEntStat.curCmd EQ AT_CMD_NRG) 
              regReqPnd = TRUE;                
            mmShrdPrm.owner = (T_OWN)srcId;
            mmEntStat.entOwn  = srcId;

#ifdef  GPRS
            mmShrdPrm.nrgCs   = GMMREG_DT_COMB;
            if( psaG_MM_CMD_DEREG ( mmShrdPrm.nrgCs ) < 0 )  /* deregister from network */
#else
            mmShrdPrm.nrgCs   = CS_SIM_REM;
            if( psaMM_DeRegistrate () < 0 )  /* deregister from network */
#endif  /* GPRS */
            {
              TRACE_EVENT( "FATAL RETURN psaMM_DeRegistrate in %%NRG" );
              ACI_ERR_DESC( ACI_ERR_CLASS_Ext, EXT_ERR_Internal );
              retVal = AT_FAIL;
            }
            else
            {
              /* Next line commented out HM 28.08.00 */
              /* simShrdPrm.SIMStat = NO_VLD_SS; */  /* no SIM data available */
              retVal = AT_EXCT;
            }
          }
          /* 
           * No SIM data is available, try  a registration to limited service.
           * The following block of code was only available in the former sAT_PercentNRG()
           * Therefore, it will only be executed by the %NRG command.
           */
          else
          {
            mmShrdPrm.regMode = MODE_AUTO;
            mmShrdPrm.owner = (T_OWN)srcId;		
            mmEntStat.entOwn  = srcId;
#ifdef  GPRS
            if( psaG_MM_CMD_REG ( ) < 0 )  /* register to network */
#else
            if( psaMM_Registrate () < 0 )  /* register to network */
#endif
            {
              TRACE_EVENT( "FATAL RETURN psaMM_Registrate in %%NRG" );
              ACI_ERR_DESC( ACI_ERR_CLASS_Ext, EXT_ERR_Internal );
              retVal = AT_FAIL;
            }
            else
            {
              retVal = AT_EXCT;
            }
          }
          break;

        case( RS_FULL_SRV ):
          mmShrdPrm.owner = (T_OWN)srcId;
          mmEntStat.entOwn  = srcId;

#ifdef  GPRS
          mmShrdPrm.nrgCs   = GMMREG_DT_COMB;
          if( psaG_MM_CMD_DEREG ( mmShrdPrm.nrgCs ) < 0 )  /* deregister from network */
#else
          mmShrdPrm.nrgCs   = CS_SIM_REM;
          if( psaMM_DeRegistrate () < 0 )  /* deregister from network */
#endif
          {
            TRACE_EVENT( "FATAL RETURN psaMM_Deregistrate in %%NRG" );
            ACI_ERR_DESC( ACI_ERR_CLASS_Ext, EXT_ERR_Internal );
            retVal = AT_FAIL;
            break;
          }

          /* Next line commented out HM 28.08.00 */
          /* simShrdPrm.SIMStat = NO_VLD_SS; */  /* no SIM data available */
          retVal = AT_EXCT;
          break;
      }
      break;

    /*
     *---------------------------------------------------------------
     * registration to no service. Only used by %NRG
     *---------------------------------------------------------------
     */
     
    case( NRG_SVMD_NoSrv ):

      mmShrdPrm.regModeAutoBack = FALSE;
      mmShrdPrm.owner = (T_OWN)srcId; 
      mmEntStat.entOwn  = srcId;

#ifdef  GPRS
      mmShrdPrm.nrgCs   = GMMREG_DT_COMB;
      if( psaG_MM_CMD_DEREG ( mmShrdPrm.nrgCs ) < 0 )  /* deregister from network */
#else
      mmShrdPrm.nrgCs   = CS_POW_OFF;
      if( psaMM_DeRegistrate () < 0 )  /* deregister from network */
#endif
      {
        TRACE_EVENT( "FATAL RETURN psaMM_Deregistrate in %%NRG" );
        ACI_ERR_DESC( ACI_ERR_CLASS_Ext, EXT_ERR_Internal );
        retVal = AT_FAIL;
        break;
      }

      retVal = AT_EXCT;
      break;

    default:
      mmEntStat.curCmd  = AT_CMD_NONE; /*Unset the Current Command */
      ACI_ERR_DESC( ACI_ERR_CLASS_Ext, EXT_ERR_Parameter );
      return( AT_FAIL );
  }

/*
 *-------------------------------------------------------------------
 * update NRG parameters
 *-------------------------------------------------------------------
 */
if (mmEntStat.curCmd  EQ AT_CMD_NRG)
{
  pMMCmdPrm -> NRGsrvMode = srvMode;
  pMMCmdPrm -> NRGregMode = regMode;
  pMMCmdPrm -> NRGoprFrmt = oprFrmt;
}

/*
 *-------------------------------------------------------------------
 * log command execution
 *-------------------------------------------------------------------
 */
#if defined SMI OR defined MFW OR defined FF_MMI_RIV
  {
  T_ACI_CLOG      cmdLog;     /* holds logging info */
  cmdLog.atCmd                = mmEntStat.curCmd;
  cmdLog.cmdType              = CLOG_TYPE_Set;
  cmdLog.retCode              = retVal;
  cmdLog.cId                  = ACI_NumParmNotPresent;
  cmdLog.sId                  = ACI_NumParmNotPresent;

  if (mmEntStat.curCmd  EQ AT_CMD_NRG)
  {
    cmdLog.cmdPrm.sNRG.srcId    = srcId;
    cmdLog.cmdPrm.sNRG.regMode  = regMode;
    cmdLog.cmdPrm.sNRG.srvMode  = srvMode;
    cmdLog.cmdPrm.sNRG.oprFrmt  = oprFrmt;
    cmdLog.cmdPrm.sNRG.opr      = opr;
  }
  else /*+COPS and %COPS. At the moment, the same message sent by both */
  {
     cmdLog.cmdPrm.sCOPS.srcId   = srcId;
     cmdLog.cmdPrm.sCOPS.mode    = mmShrdPrm.COPSmode;
     cmdLog.cmdPrm.sCOPS.format  = pMMCmdPrm -> COPSfrmt;
     cmdLog.cmdPrm.sCOPS.oper    = opr;
  }

  rAT_PercentCLOG( &cmdLog );
  }
#endif

  return( retVal );


}

/* Implements Measure#32: Row 118, 119, 980, 986, 987, 1036 & 1037 */
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_MMF                  |
| STATE   : code                  ROUTINE : cmhMM_mcc_mnc_print      |
+--------------------------------------------------------------------+

  PURPOSE : 

  This function will write mcc and mnc using sprintf.

*/
GLOBAL void cmhMM_mcc_mnc_print( CHAR           *oper,
                                 SHORT          mcc,
                                 SHORT          mnc)
{
  if ((mnc & 0x00F) EQ 0xF)
  {
    sprintf (oper, format_03x02x_str, mcc, mnc >> 4);
  }
  else
  {
    sprintf (oper, format_03x03x_str, mcc, mnc);
  }
  return;
}
/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_MMF                  |
| STATE   : code                  ROUTINE : cmhMM_OperatorQuery      |
+--------------------------------------------------------------------+

  PURPOSE : 

  This function encapsulates the common CMH functionality of 
  qAT_PlusCOPS(),   qAT_PercentCOPS(), and qAT_PercentNRG(). It basically gets the 
  Network Operator name based on the format parameter.
  
*/


GLOBAL void cmhMM_OperatorQuery( T_ACI_CMD_SRC srcId,
                                    T_ACI_COPS_FRMT format,
                                    CHAR           *oper)
{
  SHORT        mcc, mnc;    /* holds coverted values for mcc and mnc */
  T_OPER_ENTRY plmnDesc;    /* entry of operator list */
  BOOL         found;

  TRACE_FUNCTION ("cmhMM_OperatorQuery()");
  
  if( mmShrdPrm.regStat EQ RS_FULL_SRV AND
      mmShrdPrm.usedPLMN.v_plmn EQ VLD_PLMN)
  {
    cmhMM_CnvrtPLMN2INT( mmShrdPrm.usedPLMN.mcc, mmShrdPrm.usedPLMN.mnc,
                         &mcc, &mnc );

    found = cmhMM_FindPLMN (&plmnDesc, mcc, mnc, mmShrdPrm.lac, FALSE);

    if (!found)
    {
      TRACE_EVENT("!cmhMM_FindPLMN()");
      if( format EQ COPS_FRMT_Numeric )
      {
/* Implements Measure#32: Row 118 & 119 */
        cmhMM_mcc_mnc_print(oper, mcc, mnc);
      }
    }
    else
    {
      switch( format )
      {
        case( COPS_FRMT_Long ):
          TRACE_EVENT("COPS_FRMT_Long");

          /* Implements Measure 66 */
          cmhMM_cvtPnnOpName( plmnDesc.pnn, plmnDesc.long_len,
                                          plmnDesc.long_ext_dcs,
                              plmnDesc.longName,
                              oper );
          break;

        case( COPS_FRMT_Short ):
          /* Implements Measure 66 */
          cmhMM_cvtPnnOpName( plmnDesc.pnn, plmnDesc.shrt_len,
                                          plmnDesc.shrt_ext_dcs,
                              plmnDesc.shrtName,
                              oper);
          break;

      case( COPS_FRMT_Numeric ):
/* Implements Measure#32: Row 118 & 119 */
        cmhMM_mcc_mnc_print(oper, mcc, mnc);
        break;
      }
    }
  }
}


/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_MMF                  |
| STATE   : code                  ROUTINE : cmhMM_OperatorStoreInFFS      |
+--------------------------------------------------------------------+

  PURPOSE : 

  This function stores the Operator given by MCC and MNC, and the IMSI number in the FFS file
  given by MM_FFS_OPER_FILE. If the directory in which the file is stored does not exist, it will create it.
   
  
*/



GLOBAL BOOL cmhMM_OperatorStoreInFFS(UBYTE* mcc, UBYTE* mnc, UBYTE* IMSI)
{

#ifndef _SIMULATION_
  T_FFS_RET result;
#endif
  T_FFSPLMNIMSI currentPlmnIMSI;

  TRACE_FUNCTION("cmhMM_OperatorStoreInFFS");


  memcpy( currentPlmnIMSI.IMSI, IMSI, MAX_IMSI-1);
  memcpy(currentPlmnIMSI.mcc,mcc,SIZE_MCC);
  memcpy(currentPlmnIMSI.mnc,mnc,SIZE_MNC);

#ifndef _SIMULATION_

/* Implements Measure#32: Row 120 */
   switch(FFS_mkdir(gsm_cops_path))
  {/* Check / Create ffs directory  */
    case EFFS_OK:
      TRACE_EVENT("cmhMM_OperatorStoreInFFS: Dir  Created ");                        
      break;
    case EFFS_EXISTS:
      TRACE_EVENT("cmhMM_OperatorStoreInFFS:Dir  Exists ");                        
      break;
    default:
      TRACE_EVENT("cmhMM_OperatorStoreInFFS:Dir  Not Created");                  
      return ( FALSE );
  }

/* Implements Measure#32: Row 121 */
  result = FFS_fwrite(gsm_cops_operimsi_path,  &currentPlmnIMSI, sizeof(T_FFSPLMNIMSI));
  TRACE_EVENT_P1("cmhMM_OperatorStoreInFFS:FFS dir ok,FFS_fwrite res %x", result);                  

  if (result < EFFS_OK)
    return FALSE;

#else /*Simulation is defined*/

  memcpy(&RAMPlmnIMSI,&currentPlmnIMSI,sizeof(T_FFSPLMNIMSI));

#endif

  return TRUE;

}

/*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_MMF                  |
| STATE   : code                  ROUTINE : cmhMM_OperatorReadFromFFS      |
+--------------------------------------------------------------------+

  PURPOSE : 

  This function reads the Operator given by MCC and MNC, and the IMSI number from the FFS file
  given by MM_FFS_OPER_FILE. 
   
  
*/


GLOBAL BOOL cmhMM_OperatorReadFromFFS(UBYTE* mcc, UBYTE* mnc, UBYTE* IMSI)
{
#ifndef _SIMULATION_
  T_FFS_RET result;
#endif

  T_FFSPLMNIMSI currentPlmnIMSI;

  TRACE_FUNCTION("cmhMM_OperatorReadFromFFS()");

#ifndef _SIMULATION_
  
/* Implements Measure#32: Row 121 */
  result = FFS_fread(gsm_cops_operimsi_path,  &currentPlmnIMSI, sizeof(T_FFSPLMNIMSI));

  TRACE_EVENT_P1("FFS result = %d",result);

  if (result < EFFS_OK)
    return FALSE;

#else

  memcpy(&currentPlmnIMSI,&RAMPlmnIMSI,sizeof(T_FFSPLMNIMSI));


#endif

  memcpy( IMSI,currentPlmnIMSI.IMSI, MAX_IMSI-1);
  memcpy(mcc,currentPlmnIMSI.mcc,SIZE_MCC);
  memcpy(mnc,currentPlmnIMSI.mnc,SIZE_MNC);



  return TRUE;



}

 /*
+--------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)         MODULE  : CMH_MMF                  |
| STATE   : code                  ROUTINE : cmhMM_GetActingHPLMN     |
+--------------------------------------------------------------------+

  PURPOSE : fills the Acting HPLMN if present.
*/
GLOBAL BOOL cmhMM_GetActingHPLMN(SHORT * mccBuf, SHORT * mncBuf)
{
  TRACE_FUNCTION("cmhMM_GetActingHPLMN");
  if(mmShrdPrm.ActingHPLMN.v_plmn EQ VLD_PLMN)
  {
    cmhMM_CnvrtPLMN2INT( mmShrdPrm.ActingHPLMN.mcc, mmShrdPrm.ActingHPLMN.mnc, mccBuf, mncBuf );
    TRACE_EVENT("Acting HPLMN Present");
    return(TRUE);
  }
  TRACE_EVENT("Acting HPLMN not Present");
  return (FALSE);
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)    MODULE  : CMH_MMF                      |
| STATE   : code             ROUTINE : cmhMM_ONSReadName            |
+-------------------------------------------------------------------+

  PURPOSE : Sends a SIM read request

  RESULT: has an error occured ?
  Returns: FALSE if primitive has been sent and TRUE if not
*/
GLOBAL BOOL cmhMM_ONSReadName()
{
  TRACE_FUNCTION ("cmhMM_ONSReadName()");

  if (cmhSIM_ReadTranspEF (CMD_SRC_NONE,
                           AT_CMD_NONE,
                           FALSE,
                           NULL,
                           SIM_CPHS_ONSTR,
                           0,
                           NOT_PRESENT_8BIT,
                           NULL,
                           cmhMM_ONSReadNameCb) EQ AT_FAIL)
  {
    TRACE_EVENT("FATAL ERROR");
    return(TRUE);
  }
  ONSReadStatus = ONS_READING;
  return(FALSE);
}

/*
+-------------------------------------------------------------------+
| PROJECT : GSM-PS (6147)    MODULE  : CMH_MMF                      |
| STATE   : code             ROUTINE : cmhMM_ONSReadNameCb          |
+-------------------------------------------------------------------+

  PURPOSE : Call back for SIM retrieval of EF_ONS.
*/

GLOBAL void cmhMM_ONSReadNameCb(SHORT table_id)
{
  int i =0;

  TRACE_FUNCTION ("cmhMM_ONSReadNameCb()");

  if(!(simShrdPrm.atb[table_id].exchData NEQ NULL AND
                  simShrdPrm.atb[table_id].exchData[0] NEQ 0xFF AND
                  simShrdPrm.atb[table_id].errCode EQ SIM_NO_ERROR))
  {
    ONSReadStatus = ONS_READ_FAIL;
  }
  else
  {
    ONSReadStatus = ONS_READ_OVER;
    memset (&ONSDesc, 0, sizeof (T_OPER_ENTRY));
    ONSDesc.long_len = 0;
    while (simShrdPrm.atb[table_id].exchData[i] NEQ 0xFF AND
           ONSDesc.long_len < MAX_LONG_OPER_LEN - 1)
    {
      ONSDesc.long_len++;
      i++;
    }
    if (ONSDesc.long_len <= MAX_SHRT_OPER_LEN - 1)
    {
      ONSDesc.shrt_len = ONSDesc.long_len;
    }
    else
    {
      ONSDesc.shrt_len = MAX_SHRT_OPER_LEN - 1;
    }

    /* Issue OMAPS00058768 : EFons may have fewer bytes than MAX_LONG_OPER_LEN and if all of them are used bytes then 
       we will not come across 0xFF (unused bytes), which causes ONSDesc.long_len = MAX_LONG_OPER_LEN - 1. 
       So this leads to the function memcpy() to copy garbage characters in front of operator name.
    */
    ONSDesc.long_len = MINIMUM(simShrdPrm.atb[table_id].dataLen, ONSDesc.long_len);
    ONSDesc.shrt_len = MINIMUM(simShrdPrm.atb[table_id].dataLen, ONSDesc.shrt_len);

    memcpy (ONSDesc.longName, simShrdPrm.atb[table_id].exchData, ONSDesc.long_len);
    memcpy (ONSDesc.shrtName, simShrdPrm.atb[table_id].exchData, ONSDesc.shrt_len);

    ONSDesc.long_ext_dcs = ONS_EXT_DCS;
    ONSDesc.shrt_ext_dcs = ONS_EXT_DCS;
  }

/* Issue OMAPS00058768 : For file update of ONS we no need to call cmhMM_Registered() */
#ifdef SIM_TOOLKIT
    if (simShrdPrm.fuRef < 0)
#endif
    {
      cmhMM_Registered ();
    }
#ifdef SIM_TOOLKIT
    if (simShrdPrm.fuRef >= 0)
    {
      psaSAT_FUConfirm (simShrdPrm.fuRef,
               (USHORT)((simShrdPrm.atb[table_id].errCode EQ SIM_NO_ERROR)?
               SIM_FU_SUCCESS: SIM_FU_ERROR));
    }
#endif
   simShrdPrm.atb[table_id].ntryUsdFlg = FALSE;
}

/*
+-------------------------------------------------------------- +
| PROJECT : GSM-PS (6147)    MODULE  : CMH_MMF                  |
| STATE   : code             ROUTINE : cmhMM_Reset_ONSDesc      |
+---------------------------------------------------------------+

  PURPOSE :Issue OMAPS00058768: Reset ONSDesc.
*/

GLOBAL void cmhMM_Reset_ONSDesc()
{
  memset(&ONSDesc,0x00,sizeof(ONSDesc));
  ONSReadStatus = ONS_READ_NOT_DONE;
}

/* Implements Measure 66 */
/*
+------------------------------------------------------------------------------
|  Function    : cmhMM_cvtPnnOpName
+------------------------------------------------------------------------------
|  Purpose     : This function converts PNN Operator Name.
|
|  Parameters  : pnn        - PLMN Network Name Source
|                source_len - Source Length
|                dcs        - DCS
|                source     - Source String
|                oper       - Network Operator String
|
|  Return      : void
+------------------------------------------------------------------------------
*/

LOCAL void cmhMM_cvtPnnOpName( UBYTE pnn, UBYTE source_len, UBYTE dcs,
                               CHAR *source, CHAR *oper )
{
  char Name[(MAX_ALPHA_OPER_LEN*8+6)/7];

  TRACE_FUNCTION ("cmhMM_cvtPnnOpName()");

  Name[0] = '\0';

  if (pnn EQ Read_EONS)
  {
    if (source_len)
    {
      cmhMM_decodePnnChs( source, source_len, dcs, Name );
    }
    strncpy( oper, Name, MAX_ALPHA_OPER_LEN-1 );
  }
  else
  {
    strncpy( oper, source, MAX_ALPHA_OPER_LEN-1 );
  }
  oper[MAX_ALPHA_OPER_LEN-1] = '\0';
}

/* Implements Measure 118 */
/*
+------------------------------------------------------------------------------
|  Function    : cmhMM_cvtPLMNINT2BCD
+------------------------------------------------------------------------------
|  Purpose     : This function convert the integer PLMN notation for 
|                mcc and mnc into BCD representation.
||
|  Parameters  : oprFrmt   -  +COPS Parameter Format
|                oper       - Network Operator String
|
|  Return      : BOOL
+------------------------------------------------------------------------------
*/
LOCAL BOOL cmhMM_cvtPLMNINT2BCD ( T_ACI_NRG_FRMT oprFrmt, CHAR *opr)
{
  T_OPER_ENTRY    plmnDesc = {"","",0,0,0,0,0,0,0};
  BOOL            found;

  TRACE_FUNCTION ("cmhMM_cvtPLMNINT2BCD()");

  switch( oprFrmt )
  {
    case( NRG_FRMT_Long ):
      found = cmhMM_FindName (&plmnDesc, opr, CPOL_FRMT_Long);
      break;

    case( NRG_FRMT_Short ):
      found = cmhMM_FindName (&plmnDesc, opr, CPOL_FRMT_Short);
      break;

    case( NRG_FRMT_Numeric ):
      found = cmhMM_FindNumeric(&plmnDesc, opr);
      break;

    default:
      found = FALSE;
      break;
  }

  if (!found)
  {
    mmEntStat.curCmd = AT_CMD_NONE;
    ACI_ERR_DESC( ACI_ERR_CLASS_Ext, EXT_ERR_Parameter );
    return FALSE;
  }

  mmShrdPrm.slctPLMN.v_plmn = VLD_PLMN;
  cmhMM_CnvrtINT2PLMN( plmnDesc.mcc,
                       plmnDesc.mnc,
                       mmShrdPrm.slctPLMN.mcc,
                       mmShrdPrm.slctPLMN.mnc );
  return TRUE;
}

/* Implements Measure 70 */
/*
+------------------------------------------------------------------------------
|  Function    : cmhMM_decodePnnOpName
+------------------------------------------------------------------------------
|  Purpose     : This function decodes PNN Operator Name
|
|  Parameters  : oper_len   - Operater Name Length
|                source_len - Source Length
|                dcs        - DCS
|                op_name    - Source String
|
|  Return      : void
+------------------------------------------------------------------------------
*/
LOCAL void cmhMM_decodePnnOpName( UBYTE oper_len, UBYTE *source_len, UBYTE dcs,
                                  CHAR *op_name)
{
  char Name[(MAX_ALPHA_OPER_LEN*8+6)/7];

  TRACE_FUNCTION ("cmhMM_decodePnn7to8()");

  Name[0]='\0';

  if ( *source_len )
  {
    cmhMM_decodePnnChs( op_name, *source_len, dcs, Name );
    strncpy ( op_name, Name, oper_len - 1 );
    op_name[oper_len - 1] = '\0';
    *source_len = strlen( Name );
  }
}

/*
+------------------------------------------------------------------------------
|  Function    : cmhMM_decodePnnChs
+------------------------------------------------------------------------------
|  Purpose     : This function can convert PNN Operator Name .
|                Presently it is capable of converting only 7 bit default
|                alphabet to 8 bit alphabet.
|
|  Parameters  : Name       - Converted Destination String
|                source_len - Source Length
|                dcs        - DCS
|                op_name    - Source String that needs to be converted
|
|  Return      : void
+------------------------------------------------------------------------------
*/

LOCAL void cmhMM_decodePnnChs( CHAR *op_name, UBYTE source_len,
                                UBYTE dcs, char *Name )
{
  TRACE_FUNCTION ("cmhMM_decodePnnChs()");

  switch ( dcs>>4 & 0x07 )
  {
    case 0x00:
      utl_cvtPnn7To8( (UBYTE *)op_name, source_len, dcs, (UBYTE *)Name );
      break;
    case 0x01:
      TRACE_ERROR ( "ERROR: Unhandled UCS2" );
      break;
    default:
      TRACE_ERROR ( "ERROR: Unknown DCS" );
      break;
  }
}
/*==== EOF ========================================================*/
