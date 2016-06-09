/**
 * DEPRECATED: USE env_test.mak instead
 *
 * SWEs should not use XXX_REGR, XXX_MISC... anymore.
 * The tests are now included/excluded at makefile level, specified
 * by env.mak.
 */


#ifndef _RV_TEST_FILTER_REDIRECT_H_
#define _RV_TEST_FILTER_REDIRECT_H_

#include "tests/rv/rv_test_filter.h"

/*
 * HCI SWE
 */
#if ((HCI_MISC	 == SW_COMPILED) || (L2CAP_REGR == SW_COMPILED) ||  \
	 (L2CAP_CERTIF == SW_COMPILED) || (SPP_CERTIF == SW_COMPILED))
	#define HCI_TEST (SW_COMPILED)
#else
	#define HCI_TEST (SW_NOT_COMPILED)
#endif
/* 
 * L2CAP SWE
 */
#if ((L2CAP_REGR  == SW_COMPILED) || (L2CAP_CERTIF == SW_COMPILED) || (L2CAP_UPF4 == SW_COMPILED) || \
	 (SDP_REGR	  == SW_COMPILED) || (SDP_CERTIF   == SW_COMPILED) || (SDP_MISC	  == SW_COMPILED) || (SDP_UPF4 == SW_COMPILED) || \
	 (RFCOMM_REGR == SW_COMPILED) || (RFCOMM_UPF4  == SW_COMPILED) || \
	 (SPP_CERTIF  == SW_COMPILED))
	#define L2CAP_TEST (SW_COMPILED)
#else
	#define L2CAP_TEST (SW_NOT_COMPILED)
#endif
/* 
 * RFCOMM SWE
 */
#if ((RFCOMM_REGR == SW_COMPILED) || (RFCOMM_UPF4 == SW_COMPILED) || \
	 (SPP_CERTIF  == SW_COMPILED))
	#define RFC_TEST (SW_COMPILED)
#else
	#define RFC_TEST (SW_NOT_COMPILED)
#endif
/* 
 * SPP SWE
 */
#if ((SPP_REGR	  == SW_COMPILED) || (SPP_CERTIF == SW_COMPILED) || (SPP_MISC == SW_COMPILED) || \
	 (RFCOMM_UPF4 == SW_COMPILED))
	#define SPP_TEST (SW_COMPILED)
#else
	#define SPP_TEST (SW_NOT_COMPILED)
#endif
/* 
 * SDP-SDAP SWE
 */
#if ((SDP_REGR	 == SW_COMPILED) || (SDP_CERTIF == SW_COMPILED) || (SDP_MISC == SW_COMPILED) || (SDP_UPF4 == SW_COMPILED) || \
	 (SPP_CERTIF == SW_COMPILED))
	#define SDP_TEST (SW_COMPILED)
#else
	#define SDP_TEST (SW_NOT_COMPILED)
#endif
#if (SDAP_REGR == SW_COMPILED)
	#define SDAP_TEST (SW_COMPILED)
#else
	#define SDAP_TEST (SW_NOT_COMPILED)
#endif
/* 
 * ATP SWE
 */
#if ((ATP_REGR == SW_COMPILED) || (ATP_MISC == SW_COMPILED) || (HS_REGR  == SW_COMPILED))
	#define ATP_TEST (SW_COMPILED)
#else
	#define ATP_TEST (SW_NOT_COMPILED)
#endif
/* 
 * HS SWE
 */
#if ((HS_REGR == SW_COMPILED) || (HS_DEMO == SW_COMPILED))
	#define HS_TEST (SW_COMPILED)
#else
	#define HS_TEST (SW_NOT_COMPILED)
#endif
/* 
 * EXPL SWE
 */
#if ((EXPL_REGR == SW_COMPILED) || (EXPL_DEMO == SW_COMPILED))
	#define EXPL_TEST (SW_COMPILED)
#else
	#define EXPL_TEST (SW_NOT_COMPILED)
#endif
/* 
 * BTCTRL-SCM SWE
 */
#if ((BTCTRL_MISC == SW_COMPILED) || (BTCTRL_UPF4 == SW_COMPILED))
	#define BTCTRL_TEST (SW_COMPILED)
#else
	#define BTCTRL_TEST (SW_NOT_COMPILED)
#endif
#if (SCM_MISC == SW_COMPILED)
	#define SCM_TEST (SW_COMPILED)
#else
	#define SCM_TEST (SW_NOT_COMPILED)
#endif
/* 
 * DUN-GW SWE
 */
#if ((DUN_MISC == SW_COMPILED) || (DUN_UPF4 == SW_COMPILED))
	#define DUN_TEST (SW_COMPILED)
#else
	#define DUN_TEST (SW_NOT_COMPILED)
#endif
/* 
 * FAX-GW SWE
 */
#if (FAX_UPF4 == SW_COMPILED)
	#define FAX_TEST (SW_COMPILED)
#else
	#define FAX_TEST (SW_NOT_COMPILED)
#endif
/* 
 * OBX SWE
 */
#if (OBX_REGR == SW_COMPILED)
	#define OBX_TEST (SW_COMPILED)
#else
	#define OBX_TEST (SW_NOT_COMPILED)
#endif
/* 
 * OPP SWE
 */
#if ((OPP_REGR == SW_COMPILED) || (OPP_UPF4 == SW_COMPILED))
	#define OPP_TEST (SW_COMPILED)
#else
	#define OPP_TEST (SW_NOT_COMPILED)
#endif
/* 
 * FFS SWE
 */
#if (FFS_MISC == SW_COMPILED)
	#define FFS_TEST (SW_COMPILED)
#else
	#define FFS_TEST (SW_NOT_COMPILED)
#endif
/* 
 * BMI SWE
 */
#if (BMI_MISC == SW_COMPILED)
	#define BMI_TEST (SW_COMPILED)
#else
	#define BMI_TEST (SW_NOT_COMPILED)
#endif
/* 
 * SYN SWE
 */
#if (SYN_REGR == SW_COMPILED)
	#define SYN_TEST (SW_COMPILED)
#else
	#define SYN_TEST (SW_NOT_COMPILED)
#endif
/* 
 * RTC SWE
 */
#if (RTC_REGR == SW_COMPILED) || (RTC_MISC == SW_COMPILED)
	#define RTC_TEST (SW_COMPILED)
#else
	#define RTC_TEST (SW_NOT_COMPILED)
#endif
/* 
 * AUDIO SWE
 */
#if ((AUDIO_REGR == SW_COMPILED) || (AUDIO_MISC == SW_COMPILED))
  #define AUDIO_TEST (SW_COMPILED)
#else
	#define AUDIO_TEST (SW_NOT_COMPILED)
#endif
/* 
 * R2D SWE
 */
#if ((R2D_MISC == SW_COMPILED) || (R2D_DEMO == SW_COMPILED))
	#define R2D_TEST (SW_COMPILED)
#else
	#define R2D_TEST (SW_NOT_COMPILED)
#endif
/* 
 * RGUI SWE
 */
#if (RGUI_MISC == SW_COMPILED)
	#define RGUI_TEST (SW_COMPILED)
#else
	#define RGUI_TEST (SW_NOT_COMPILED)
#endif
/* 
 * DAR SWE
 */
#if ((DAR_REGR == SW_COMPILED) || (DAR_MISC == SW_COMPILED))
  #define DAR_TEST (SW_COMPILED)
#else
	#define DAR_TEST (SW_NOT_COMPILED)
#endif
/* 
 * POWER SWE
 */
#if ((PWR_REGR == SW_COMPILED) || (PWR_MISC == SW_COMPILED))
	#define PWR_TEST (SW_COMPILED)
#else
	#define PWR_TEST (SW_NOT_COMPILED)
#endif
/* 
 * KPD SWE
 */
#if (KPD_MISC == SW_COMPILED)
  #define KPD_TEST (SW_COMPILED)
#else
	#define KPD_TEST (SW_NOT_COMPILED)
#endif
/* 
 * MKS SWE
 */
#if (MKS_MISC == SW_COMPILED)
  #define MKS_TEST (SW_COMPILED)
#else
	#define MKS_TEST (SW_NOT_COMPILED)
#endif
/* SWE below is still not included in current Workspace */
#if ((FTP_REGR == SW_COMPILED)|| (FTP_UPF4 == SW_COMPILED))
	#define FTP_TEST (SW_COMPILED)
#else
	#define FTP_TEST (SW_NOT_COMPILED)
#endif
/*
 * DEV1, DEV2, DEV3 SWE
 */
#if (DEV1_MISC == SW_COMPILED)
	#define DEV1_TEST (SW_COMPILED)
#else
	#define DEV1_TEST (SW_NOT_COMPILED)
#endif
#if (DEV2_MISC == SW_COMPILED)
	#define DEV2_TEST (SW_COMPILED)
#else
	#define DEV2_TEST (SW_NOT_COMPILED)
#endif
#if (DEV3_MISC == SW_COMPILED)
	#define DEV3_TEST (SW_COMPILED)
#else
	#define DEV3_TEST (SW_NOT_COMPILED)
#endif
/* 
 * LLS SWE
 */
#if (LLS_MISC == SW_COMPILED)
  #define LLS_TEST (SW_COMPILED)
#else
	#define LLS_TEST (SW_NOT_COMPILED)
#endif
/* 
 * MPM SWE
 */
#if ((MPM_MISC == SW_COMPILED) || (MPM_DEMO == SW_COMPILED))
  #define MPM_TEST (SW_COMPILED)
#else
	#define MPM_TEST (SW_NOT_COMPILED)
#endif

/* 
 * MDC SWE
 */
#if ((MDC_MISC == SW_COMPILED) || (MDC_REGR == SW_COMPILED))
  #define MDC_TEST (SW_COMPILED)
#else
	#define MDC_TEST (SW_NOT_COMPILED)
#endif

/* 
 * MDL SWE
 */
#if ((MDL_MISC == SW_COMPILED) || (MDL_REGR == SW_COMPILED))
  #define MDL_TEST (SW_COMPILED)
#else
	#define MDL_TEST (SW_NOT_COMPILED)
#endif

#endif /* _RV_TEST_FILTER_REDIRECT_H_ */
