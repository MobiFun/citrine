/*
+--------------------------------------------------------------------+
| PROJECT: GSM-F&D (8411)               $Workfile:: aci_cphs.h      $|
| $Author:: $ CONDAT AG                 $Revision::                 $|
| CREATED:                              $Modtime::                  $|
| STATE  : code                                                      |
+--------------------------------------------------------------------+

   MODULE  : ACI_CPHS

   PURPOSE : Command handler interface definitions for CPHS

*/

#ifdef FF_CPHS

#ifndef ACI_CPHS_H
#define ACI_CPHS_H

/**************** CPHS Module *****************************************/
EXTERN T_ACI_RETURN sAT_PercentCPHS( T_ACI_CMD_SRC   srcId,
                                     T_ACI_CPHS_INIT init_cphs );

EXTERN T_ACI_RETURN qAT_PercentCPHS( T_ACI_CMD_SRC   srcId,
                                     T_ACI_CPHS_INIT *init_cphs);

EXTERN T_ACI_RETURN sAT_PercentCPNUMS( T_ACI_CMD_SRC srcId,
                                       UBYTE         element_id,
                                       UBYTE         mode );

EXTERN T_ACI_RETURN tAT_PercentCPNUMS( T_ACI_CMD_SRC srcId );

EXTERN T_ACI_RETURN qAT_PercentCPALS( T_ACI_CMD_SRC srcId,
                                      UBYTE         call_id,
                                      T_CPHS_LINES  *line,
                                      CHAR          *line_desc,
                                      UBYTE         *max_line_desc);

EXTERN T_ACI_RETURN sAT_PercentCPVWI( T_ACI_CMD_SRC srcId,
                                      UBYTE         flag_set, 
                                      USHORT        lines);

EXTERN T_ACI_RETURN qAT_PercentCPVWI( T_ACI_CMD_SRC srcId,
                                      UBYTE         *flag_set, 
                                      USHORT        line);

EXTERN T_ACI_RETURN qAT_PercentCPOPN( T_ACI_CMD_SRC srcId,
                                      CHAR         *longname, 
                                      UBYTE        *max_longname,
                                      CHAR         *shortname, 
                                      UBYTE        *max_shortname);

EXTERN T_ACI_RETURN sAT_PercentCPINF( T_ACI_CMD_SRC srcId,
                                      UBYTE         *csp,
                                      UBYTE         csp_len);

EXTERN T_ACI_RETURN qAT_PercentCPINF( T_ACI_CMD_SRC srcId,
                                      UBYTE        *phase,
                                      USHORT       *sst,
                                      CHAR         *csp,
                                      CHAR         *csp2,
                                      UBYTE        *max_csp_size,
                                      UBYTE        *max_csp2_size);

EXTERN T_ACI_RETURN qAT_PercentCPMB( T_ACI_CMD_SRC  srcId,
                                      UBYTE         rec_id,
                                      T_CPHS_LINES  *line,
                                      CHAR          *number,
                                      T_ACI_TOA_TON *ton,
                                      T_ACI_TOA_NPI *npi,
                                      CHAR          *alpha_id,
                                      UBYTE         *first);

EXTERN T_ACI_RETURN sAT_PercentCPMBW( T_ACI_CMD_SRC       srcId,
                                      SHORT               index,
                                      CHAR*               number,
                                      T_ACI_TOA*          type,
                                      T_CPHS_PB_TEXT* text);

GLOBAL T_ACI_RETURN tAT_PercentCPMBW ( T_ACI_CMD_SRC srcId,
                                       SHORT*        firstIdx,
                                       SHORT*        lastIdx,
                                       UBYTE*        nlength,
                                       UBYTE*        tlength );

EXTERN T_ACI_RETURN sAT_PercentCPCFU( T_ACI_CMD_SRC srcId, 
                                      UBYTE         cfu_set, 
                                      T_CPHS_LINES  lines );

EXTERN T_ACI_RETURN qAT_PercentCPCFU( T_ACI_CMD_SRC srcId,
                                      UBYTE *cfu_set, 
                                      T_CPHS_LINES line );


#ifdef CMH_F_C
EXTERN void rCI_PercentCPNUMS(void);
EXTERN void rAT_PercentCPNUMS(void);
EXTERN void rCI_PercentCPVWI (void);
EXTERN void rAT_PercentCPVWI (void);
EXTERN void rCI_PercentCPROAM (void);
EXTERN void rAT_PercentCPROAM (void);


#else
EXTERN void rCI_PercentCPNUMS(UBYTE element_index,
                              UBYTE index_level,
                              CHAR  *alpha_tag,
                              CHAR  *number,
                              BOOL  premium_flag,
                              BOOL  network_flag,
                              UBYTE type_of_address);
                              

EXTERN void rAT_PercentCPNUMS(UBYTE element_index,
                              UBYTE index_level,
                              CHAR  *alpha_tag,
                              CHAR  *number,
                              BOOL  premium_flag,
                              BOOL  network_flag,
                              UBYTE type_of_address);
                              

EXTERN void rCI_PercentCPVWI (UBYTE  flag_set, 
                              USHORT line);
EXTERN void rAT_PercentCPVWI (UBYTE  flag_set, 
                              USHORT line);

EXTERN void rCI_PercentCPROAM (UBYTE roam_status);
EXTERN void rAT_PercentCPROAM (UBYTE roam_status);

#endif /* CMH_F_C */



/**********************************************************************/


#endif /* ACI_CPHS_H */
#endif /* FF_CPHS */

/*==== EOF ========================================================*/
