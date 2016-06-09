
/*
+------------------------------------------------------------------------------
|  File:       cdg_enter.h
+------------------------------------------------------------------------------
|                 Copyright Texas Instruments AG Berlin 2002-2003, Berlin
|                 All rights reserved.
|
|                 This file is confidential and a trade secret of Condat AG.
|                 The receipt of or possession of this file does not convey
|                 any rights to reproduce or disclose its contents or to
|                 manufacture, use, or sell anything it may describe, in
|                 whole, or in part, without the specific written consent of
|                 Texas Instruments AG Berlin.
+------------------------------------------------------------------------------
| Purpose:    Check the SrcFileTime of the cdginc header files at link time.
|             
|             The macros in this file help to detect version mismatch between 
|             header files included for build of different libraries. 
|             Example of a linker error: 
|             xx_tdc_3_constraints.obj : error LNK2001: 
|             unresolved external symbol 
|             "char  BadLibVersionCheck____P_XX_TDC_3_VAL_H____Thu_Dec_12_17_49_56_2002" 
|             (?BadLibVersionCheck____P_XX_TDC_3_VAL_H____Thu_Dec_12_17_49_56_2002@@3DA)
|             \gpf\util\teststack\bin\test_xx_tdc\xx_tdc.dll : 
|                                  fatal error LNK1120: 1 unresolved externals
|             Error executing link.exe.
| 
|             The first group of macros (protected by CDG_ENTER) are necessary 
|             to build the strings of type "BadLibVersionCheck__xxx".
|  
|             They need in turn other macros which are set in the *.h files of 
|             cdginc, tdcinc or message_san.h directory.
|             (e.g. CDG_ENTER__M_XX_VAL_H__FILE_TYPE) 
|
|             The check is done only for the header files of TDC, where 
|             ENABLE__CDG_ENTER__SANITY_CHECK is switched on.
|
+------------------------------------------------------------------------------
*/
#ifndef CDG_ENTER
#define CDG_ENTER

/*------- START <Fix for long identifiers for Arm 7 compiler - LHC> ----------*/
/* ccdid.h */
#define CCD_ID_UMTS_AS_ASN1__ho_to_utrancommand_r3_ies__spec_mode__preconf__pre_config_mode__default_config__default_config_mode                                                                      shortname001
#define CCD_ID_UMTS_AS_ASN1__ho_to_utrancommand_r3_ies__spec_mode__preconf__pre_config_mode__default_config__default_config_identity                                                                  shortname002
#define CCD_ID_UMTS_AS_ASN1__inter_freq_meas_quantity__reporting_criteria__inter_freq_reporting_criteria__mode_specific_info__fdd__freq_qual_estimate_quantity_fdd                                    shortname003
#define CCD_ID_UMTS_AS_ASN1__inter_freq_meas_quantity__reporting_criteria__inter_freq_reporting_criteria__mode_specific_info__tdd__freq_qual_estimate_quantity_tdd                                    shortname004
#define CCD_ID_UMTS_AS_ASN1__inter_freq_meas_quantity__reporting_criteria__inter_freq_reporting_criteria__mode_specific_info__body__fdd                                                               shortname005
#define CCD_ID_UMTS_AS_ASN1__inter_freq_meas_quantity__reporting_criteria__inter_freq_reporting_criteria__mode_specific_info__body__tdd                                                               shortname006
#define CCD_ID_UMTS_AS_ASN1__inter_freq_meas_quantity__reporting_criteria__inter_freq_reporting_criteria__mode_specific_info__body                                                                    shortname007
#define CCD_ID_UMTS_AS_ASN1__inter_freq_meas_quantity__reporting_criteria__inter_freq_reporting_criteria__mode_specific_info                                                                          shortname008
#define CCD_ID_UMTS_AS_ASN1__meas_results_on_rach__current_cell__mode_specific_info__fdd__meas_quantity__body__cpich_ec_n0                                                                            shortname009
#define CCD_ID_UMTS_AS_ASN1__meas_results_on_rach__current_cell__mode_specific_info__fdd__meas_quantity__body__cpich_rscp                                                                             shortname010
#define CCD_ID_UMTS_AS_ASN1__meas_results_on_rach__current_cell__mode_specific_info__fdd__meas_quantity__body__pathloss                                                                               shortname011
#define CCD_ID_UMTS_AS_ASN1__meas_results_on_rach__current_cell__mode_specific_info__fdd__meas_quantity__body__spare                                                                                  shortname012
#define CCD_ID_UMTS_AS_ASN1__meas_results_on_rach__current_cell__mode_specific_info__fdd__meas_quantity__body                                                                                         shortname013
#define CCD_ID_UMTS_AS_ASN1__monitored_cell_rach_result__mode_specific_info__fdd__meas_quantity__body__cpich_ec_n0                                                                                    shortname014
#define CCD_ID_UMTS_AS_ASN1__monitored_cell_rach_result__mode_specific_info__fdd__meas_quantity__body__cpich_rscp                                                                                     shortname015
#define CCD_ID_UMTS_AS_ASN1__pusch_capacity_alloc_info__pusch_alloc__pusch_alloc_assignment__config__old_config__tfcs_id                                                                              shortname016
#define CCD_ID_UMTS_AS_ASN1__pusch_capacity_alloc_info__pusch_alloc__pusch_alloc_assignment__config__old_config__pusch_identity                                                                       shortname017
#define CCD_ID_UMTS_AS_ASN1__meas_control__r3__v_390_non_critical_extensions__v_3_a_0_non_critical_extensions__meas_control_v_3_a_0_ext                                                               shortname018
#define CCD_ID_UMTS_AS_ASN1__meas_control__r3__v_390_non_critical_extensions__v_3_a_0_non_critical_extensions__non_critical_extensions                                                                shortname019
#define CCD_ID_UMTS_AS_ASN1__meas_control__r3__v_390_non_critical_extensions__v_3_a_0_non_critical_extensions                                                                                         shortname020
#define CCD_ID_UMTS_AS_ASN1__meas_control__r3__v_390_non_critical_extensions__v_3_a_0_non_critical_extensions__later_non_critical_extensions__meas_control_r3_add_ext                                 shortname021
#define CCD_ID_UMTS_AS_ASN1__meas_control__r3__v_390_non_critical_extensions__v_3_a_0_non_critical_extensions__later_non_critical_extensions__non_critical_extensions                                 shortname022
#define CCD_ID_UMTS_AS_ASN1__meas_control__r3__v_390_non_critical_extensions__v_3_a_0_non_critical_extensions__later_non_critical_extensions                                                          shortname023

#define CCD_ID_UMTS_AS_ASN1__ue_positioning_pos_estimate_info__ref_time__cell_timing__mode_specific_info__body__fdd                                                                                   shortname026
#define CCD_ID_UMTS_AS_ASN1__ue_positioning_pos_estimate_info__ref_time__cell_timing__mode_specific_info__body__tdd                                                                                   shortname027
#define CCD_ID_UMTS_AS_ASN1__ue_positioning_pos_estimate_info__ref_time__cell_timing__mode_specific_info__body                                                                                        shortname028
#define CCD_ID_UMTS_AS_ASN1__rrc_connection_setup_complete__v_370_non_critical_extensions__v_380_non_critical_extensions__v_3_a_0_non_critical_extensions__rrc_connection_setup_complete_v_3_a_0_ext  shortname029
#define CCD_ID_UMTS_AS_ASN1__rrc_connection_setup_complete__v_370_non_critical_extensions__v_380_non_critical_extensions__v_3_a_0_non_critical_extensions__non_critical_extensions                    shortname030
#define CCD_ID_UMTS_AS_ASN1__rrc_connection_setup_complete__v_370_non_critical_extensions__v_380_non_critical_extensions__rrc_connection_setup_complete_v_380_ext                                     shortname031
#define CCD_ID_UMTS_AS_ASN1__rrc_connection_setup_complete__v_370_non_critical_extensions__v_380_non_critical_extensions__v_3_a_0_non_critical_extensions                                             shortname032
#define CCD_ID_UMTS_AS_ASN1__rrc_connection_setup_complete__v_370_non_critical_extensions__v_380_non_critical_extensions                                                                              shortname033
#define CCD_ID_UMTS_AS_ASN1__rrc_connection_setup_complete__v_370_non_critical_extensions__v_380_non_critical_extensions__v_3_a_0_non_critical_extensions__later_non_critical_extensions__rrc_connection_setup_complete_r3_add_ext          shortname034
#define CCD_ID_UMTS_AS_ASN1__rrc_connection_setup_complete__v_370_non_critical_extensions__v_380_non_critical_extensions__v_3_a_0_non_critical_extensions__later_non_critical_extensions__non_critical_extensions                           shortname035
#define CCD_ID_UMTS_AS_ASN1__rrc_connection_setup_complete__v_370_non_critical_extensions__v_380_non_critical_extensions__v_3_a_0_non_critical_extensions__later_non_critical_extensions                                                    shortname036
#define CCD_ID_UMTS_AS_ASN1__ue_capab_info__v_370_non_critical_extensions__v_380_non_critical_extensions__v_3_a_0_non_critical_extensions__ue_capab_info_v_3_a_0_ext                                                                        shortname037
#define CCD_ID_UMTS_AS_ASN1__ue_capab_info__v_370_non_critical_extensions__v_380_non_critical_extensions__v_3_a_0_non_critical_extensions__non_critical_extensions                                                                          shortname038
#define CCD_ID_UMTS_AS_ASN1__ue_capab_info__v_370_non_critical_extensions__v_380_non_critical_extensions__v_3_a_0_non_critical_extensions                                                                                                   shortname039
#define CCD_ID_UMTS_AS_ASN1__ue_capab_info__v_370_non_critical_extensions__v_380_non_critical_extensions__v_3_a_0_non_critical_extensions__later_non_critical_extensions__ue_capab_info_r3_add_ext                                          shortname040
#define CCD_ID_UMTS_AS_ASN1__ue_capab_info__v_370_non_critical_extensions__v_380_non_critical_extensions__v_3_a_0_non_critical_extensions__later_non_critical_extensions__non_critical_extensions                                           shortname041
#define CCD_ID_UMTS_AS_ASN1__meas_control_sys_info__use_of_hcs__hcs_not_used__cell_select_qual_measure__cpich_ec_n0__intra_freq_meas_sys_info                                                         shortname042
#define CCD_ID_UMTS_AS_ASN1__meas_control_sys_info__use_of_hcs__hcs_not_used__cell_select_qual_measure__cpich_ec_n0__inter_freq_meas_sys_info                                                         shortname043
#define CCD_ID_UMTS_AS_ASN1__meas_control_sys_info__use_of_hcs__hcs_not_used__cell_select_qual_measure__cpich_rscp__intra_freq_meas_sys_info                                                          shortname044
#define CCD_ID_UMTS_AS_ASN1__meas_control_sys_info__use_of_hcs__hcs_not_used__cell_select_qual_measure__cpich_rscp__inter_freq_meas_sys_info                                                          shortname045
#define CCD_ID_UMTS_AS_ASN1__meas_control_sys_info__use_of_hcs__hcs_used__cell_select_qual_measure__cpich_ec_n0__intra_freq_meas_sys_info                                                             shortname046
#define CCD_ID_UMTS_AS_ASN1__meas_control_sys_info__use_of_hcs__hcs_used__cell_select_qual_measure__cpich_ec_n0__inter_freq_meas_sys_info                                                             shortname047
#define CCD_ID_UMTS_AS_ASN1__meas_control_sys_info__use_of_hcs__hcs_used__cell_select_qual_measure__cpich_rscp__intra_freq_meas_sys_info                                                              shortname048
#define CCD_ID_UMTS_AS_ASN1__meas_control_sys_info__use_of_hcs__hcs_used__cell_select_qual_measure__cpich_rscp__inter_freq_meas_sys_info                                                              shortname049
#define CCD_ID_UMTS_AS_ASN1__pusch_capacity_alloc_info__pusch_alloc__pusch_alloc_assignment__config__new_config__pusch_info                                                                           shortname050
#define CCD_ID_UMTS_AS_ASN1__pusch_capacity_alloc_info__pusch_alloc__pusch_alloc_assignment__config__new_config__pusch_identity                                                                       shortname051
#define CCD_ID_UMTS_AS_ASN1__meas_control_sys_info__use_of_hcs__hcs_not_used__cell_select_qual_measure__body__cpich_rscp                                                                              shortname052
#define CCD_ID_UMTS_AS_ASN1__meas_control_sys_info__use_of_hcs__hcs_not_used__cell_select_qual_measure__body__cpich_ec_n0                                                                             shortname053
#define CCD_ID_UMTS_AS_ASN1__meas_control_sys_info__use_of_hcs__hcs_not_used__cell_select_qual_measure__body                                                                                          shortname054
#define CCD_ID_UMTS_AS_ASN1__meas_control_sys_info__use_of_hcs__hcs_used__cell_select_qual_measure__body__cpich_rscp                                                                                  shortname055
#define CCD_ID_UMTS_AS_ASN1__meas_control_sys_info__use_of_hcs__hcs_used__cell_select_qual_measure__body__cpich_ec_n0                                                                                 shortname056
#define CCD_ID_UMTS_AS_ASN1__inter_rat_ho_info__v_390_non_critical_extensions__present__v_3_a_0_non_critical_extensions__inter_rat_ho_info_v_3_a_0_ext                                                shortname057
#define CCD_ID_UMTS_AS_ASN1__inter_rat_ho_info__v_390_non_critical_extensions__present__v_3_a_0_non_critical_extensions__non_critical_extensions                                                      shortname058
#define CCD_ID_UMTS_AS_ASN1__inter_rat_ho_info__v_390_non_critical_extensions__present__v_3_a_0_non_critical_extensions                                                                               shortname059
#define CCD_ID_UMTS_AS_ASN1__inter_rat_ho_info__v_390_non_critical_extensions__present__v_3_a_0_non_critical_extensions__later_non_critical_extensions                                                shortname060
#define CCD_ID_UMTS_AS_ASN1__inter_rat_ho_info__v_390_non_critical_extensions__present__v_3_a_0_non_critical_extensions__later_non_critical_extensions__inter_rat_ho_info_v_3_d_0_ext                 shortname061
#define CCD_ID_UMTS_AS_ASN1__inter_rat_ho_info__v_390_non_critical_extensions__present__v_3_a_0_non_critical_extensions__later_non_critical_extensions__inter_rat_ho_info_r3_add_ext                  shortname062
#define CCD_ID_UMTS_AS_ASN1__assistance_data_delivery__r3__v_3_a_0_non_critical_extensions__later_non_critical_extensions__assistance_data_delivery_r3_add_ext                                        shortname063
#define CCD_ID_UMTS_AS_ASN1__assistance_data_delivery__r3__v_3_a_0_non_critical_extensions__later_non_critical_extensions__non_critical_extensions                                                    shortname064
#define CCD_ID_UMTS_AS_ASN1__assistance_data_delivery__r3__v_3_a_0_non_critical_extensions__later_non_critical_extensions                                                                             shortname065
#define CCD_ID_UMTS_AS_ASN1__cell_update_cnf__r3__v_3_a_0_non_critical_extensions__later_non_critical_extensions__cell_update_cnf_r3_add_ext                                                          shortname066
#define CCD_ID_UMTS_AS_ASN1__cell_update_cnf__r3__v_3_a_0_non_critical_extensions__later_non_critical_extensions__non_critical_extensions                                                             shortname067
#define CCD_ID_UMTS_AS_ASN1__cell_update_cnf__r3__v_3_a_0_non_critical_extensions__later_non_critical_extensions                                                                                      shortname068
#define CCD_ID_UMTS_AS_ASN1__initial_direct_transfer__v_3_a_0_non_critical_extensions__later_non_critical_extensions__initial_direct_transfer_r3_add_ext                                              shortname069
#define CCD_ID_UMTS_AS_ASN1__initial_direct_transfer__v_3_a_0_non_critical_extensions__later_non_critical_extensions__non_critical_extensions                                                         shortname070
#define CCD_ID_UMTS_AS_ASN1__initial_direct_transfer__v_3_a_0_non_critical_extensions__later_non_critical_extensions                                                                                  shortname071
#define CCD_ID_UMTS_AS_ASN1__phys_ch_reconf__r3__v_3_a_0_non_critical_extensions__later_non_critical_extensions__phys_ch_reconf_r3_add_ext                                                            shortname072
#define CCD_ID_UMTS_AS_ASN1__phys_ch_reconf__r3__v_3_a_0_non_critical_extensions__later_non_critical_extensions__non_critical_extensions                                                              shortname073
#define CCD_ID_UMTS_AS_ASN1__phys_ch_reconf__r3__v_3_a_0_non_critical_extensions__later_non_critical_extensions                                                                                       shortname074
#define CCD_ID_UMTS_AS_ASN1__rb_release__r3__v_3_a_0_non_critical_extensions__later_non_critical_extensions__rb_release_r3_add_ext                                                                    shortname075
#define CCD_ID_UMTS_AS_ASN1__rb_release__r3__v_3_a_0_non_critical_extensions__later_non_critical_extensions__non_critical_extensions                                                                  shortname076
#define CCD_ID_UMTS_AS_ASN1__transport_ch_reconf__r3__v_3_a_0_non_critical_extensions__later_non_critical_extensions__transport_ch_reconf_r3_add_ext                                                  shortname077
#define CCD_ID_UMTS_AS_ASN1__transport_ch_reconf__r3__v_3_a_0_non_critical_extensions__later_non_critical_extensions__non_critical_extensions                                                         shortname078
#define CCD_ID_UMTS_AS_ASN1__transport_ch_reconf__r3__v_3_a_0_non_critical_extensions__later_non_critical_extensions                                                                                  shortname079
#define CCD_ID_UMTS_AS_ASN1__utran_mobility_info__r3__v_3_a_0_non_critical_extensions__later_non_critical_extensions__utran_mobility_info_r3_add_ext                                                  shortname080
#define CCD_ID_UMTS_AS_ASN1__utran_mobility_info__r3__v_3_a_0_non_critical_extensions__later_non_critical_extensions__non_critical_extensions                                                         shortname081
#define CCD_ID_UMTS_AS_ASN1__utran_mobility_info__r3__v_3_a_0_non_critical_extensions__later_non_critical_extensions                                                                                  shortname082

/* m_umts_as_asn1_inc.h */
#define T_UMTS_AS_ASN1_inter_freq_meas_quantity__reporting_criteria__inter_freq_reporting_criteria__mode_specific_info__fdd                         shortname100
#define T_UMTS_AS_ASN1_inter_freq_meas_quantity__reporting_criteria__inter_freq_reporting_criteria__mode_specific_info__tdd                         shortname101
#define T_UMTS_AS_ASN1_inter_freq_meas_quantity__reporting_criteria__inter_freq_reporting_criteria__mode_specific_info__body                        shortname102
#define T_UMTS_AS_ASN1_inter_freq_meas_quantity__reporting_criteria__inter_freq_reporting_criteria__mode_specific_info                              shortname103
#define T_UMTS_AS_ASN1_rrc_connection_setup_complete__v_370_non_critical_extensions__v_380_non_critical_extensions__v_3_a_0_non_critical_extensions shortname104
#define T_UMTS_AS_ASN1_rrc_connection_setup_complete__v_370_non_critical_extensions__v_380_non_critical_extensions                                  shortname105

/* m_umts_as_asn1_inc.val */
#define UMTS_AS_ASN1_UL_TIMESLOTS_CODES__MORE_TIMESLOTS__ADDITIONAL_TIMESLOTS__CONSECUTIVE__NUM_ADDITIONAL_TIMESLOTS__RANGE_MIN shortname200
#define UMTS_AS_ASN1_UL_TIMESLOTS_CODES__MORE_TIMESLOTS__ADDITIONAL_TIMESLOTS__CONSECUTIVE__NUM_ADDITIONAL_TIMESLOTS__RANGE_MAX shortname201
#define UMTS_AS_ASN1_INTER_FREQ_MEAS_QUANTITY__REPORTING_CRITERIA__INTER_FREQ_REPORTING_CRITERIA__MODE_SPECIFIC_INFO__FDD__TAG  shortname202
#define UMTS_AS_ASN1_INTER_FREQ_MEAS_QUANTITY__REPORTING_CRITERIA__INTER_FREQ_REPORTING_CRITERIA__MODE_SPECIFIC_INFO__TDD__TAG  shortname203
/*------- END <Fix for long identifiers for Arm 7 compiler - LHC>-----------*/



//Mapping types due to issue/problem with CCDGEN_2.3.6 - KSP
//typedef typename T_TDC_INTERFACE_UMTS_AS_ASN1_uarfcn T_TDC_INTERFACE_GSI_uarfcn
#define T_TDC_INTERFACE_GSI_uarfcn T_TDC_INTERFACE_UMTS_AS_ASN1_uarfcn 
#define T_TDC_HANDLE_GSI_uarfcn T_TDC_HANDLE_UMTS_AS_ASN1_uarfcn
#define T_TDC_INTERFACE_GSI_primary_scrambling_code T_TDC_INTERFACE_UMTS_AS_ASN1_primary_scrambling_code 
#define T_TDC_HANDLE_GSI_primary_scrambling_code T_TDC_HANDLE_UMTS_AS_ASN1_primary_scrambling_code 
#define T_GSI_start_val T_UMTS_AS_ASN1_start_val


/* we need a 2 stage approach to expand A and B before concatting them */
#define CDG_ENTER_CONCATx(A,B) A##B
#define CDG_ENTER_CONCAT(A,B) CDG_ENTER_CONCATx(A,B)
#define CDG_ENTER_CONCAT3x(A,B,C) A##B##C
#define CDG_ENTER_CONCAT3(A,B,C) CDG_ENTER_CONCAT3x(A,B,C)
#define CDG_ENTER_CONCAT4x(A,B,C,D) A##B##C##D
#define CDG_ENTER_CONCAT4(A,B,C,D) CDG_ENTER_CONCAT4x(A,B,C,D)

/* we need a 2 stage approach to expand A before stringifying it */
#define CDG_ENTER_STRINGx(A) #A
#define CDG_ENTER_STRING(A) CDG_ENTER_STRINGx(A)

#define CDG_ENTER_GET_PRAGMA(PRAGMA) CDG_ENTER_CONCAT3(CDG_ENTER_,CDG_ENTER__FILENAME,__##PRAGMA)

/* CDG_ENTER_GET_FILE_TYPE is on the form: 
  CDG_ENTER_FILE_TYPE__TDCINC
                       <-FT-> 

  FT = CDG_ENTER_GET_PRAGMA(_FILE_TYPE) = CDG_ENTER__P_XX_CCD_DSC_CPP__FILE_TYPE = TDCINC
*/
#define CDG_ENTER_GET_FILE_TYPE CDG_ENTER_CONCAT(CDG_ENTER_FILE_TYPE__,CDG_ENTER_GET_PRAGMA(FILE_TYPE))

/* CDG_ENTER_SANITY_NAME is on the form:
  BadLibVersionCheck____P_XX_TDC_3_VAL_H____Thu_Dec_12_17_49_56_2002
                  <-- FILENAME  -->   <--   SRC_FILE_TIME   -->       
*/
#if 0
#define CDG_ENTER_SANITY_NAME CDG_ENTER_CONCAT4(BadLibVersionCheck___,CDG_ENTER__FILENAME,___,CDG_ENTER_GET_PRAGMA(LAST_MODIFIED))
#else
#define CDG_ENTER_SANITY_NAME CDG_ENTER_CONCAT4(BadLibVersionCheck___,CDG_ENTER__FILENAME,___,CDG_ENTER_GET_PRAGMA(SRC_FILE_TIME))
#endif

/* create an enumerate sequence of the known file types so we can test which one we have with a '#if A == B' line */
#define CDG_ENTER_FILE_TYPE__CDGINC 1
#define CDG_ENTER_FILE_TYPE__TDCINC 2
#define CDG_ENTER_FILE_TYPE__TDCINC_DSC 3
#define CDG_ENTER_FILE_TYPE__TDCINC_MAIN 4
#endif 

/*
 * The check will be done only for TDC files, where 
 * ENABLE__CDG_ENTER__SANITY_CHECK is switched on.
 */
#if (CDG_ENTER_GET_FILE_TYPE == CDG_ENTER_FILE_TYPE__TDCINC) 
#define ENABLE__CDG_ENTER__SANITY_CHECK 
#endif

#ifdef CDG_ENTER_DEBUG
#pragma message (CDG_ENTER_STRING(CDG_ENTER__FILENAME))
#pragma message (CDG_ENTER_STRING(CDG_ENTER_SANITY_NAME))
#pragma message (CDG_ENTER_STRING(CDG_ENTER_GET_FILE_TYPE))
#endif

#ifdef ENABLE__CDG_ENTER__SANITY_CHECK 
  #ifdef CDG_ENTER_DEFINE_SANITY
  /* this part goes into ccddata.lib and tdcinc.lib */
  char CDG_ENTER_SANITY_NAME;
  #else
    #ifdef CDG_ENTER_DEBUG
    #pragma message (CDG_ENTER_STRING(CDG_ENTER_CONCAT(BadLibVersionCheck___,CDG_ENTER__FILENAME)))
    #endif
  /* this part goes into 
    every stack file using a cdginc .h or .val files (ones for every file used)
    and similar for tdc

    expands to e.g. 
      extern char BadLibVersionCheck____P_XX_TDC_3_VAL_H____Thu_Dec_12_17_49_56_2002
  */
  extern char CDG_ENTER_SANITY_NAME;
  /* Originally it was this but since no one actually uses this stuff (we only have 
    it for the linker to check that versions match) we can save memory by only 
    storing the 8 lower bits.

    expands to e.g. 
      static char BadLibVersionCheck____P_XX_TDC_3_VAL_H = (char)(&BadLibVersionCheck____P_XX_TDC_3_VAL_H____Thu_Dec_12_17_49_56_2002);
  */
  static char CDG_ENTER_CONCAT(BadLibVersionCheck___,CDG_ENTER__FILENAME) = (char)(&CDG_ENTER_SANITY_NAME);
  #endif
#endif
