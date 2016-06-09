/**
 * DEPRECATED: USE env_test.mak instead
 *
 * SWEs should not use XXX_REGR, XXX_MISC... anymore.
 * The tests are now included/excluded at makefile level, specified
 * by env.mak.
 */

#ifndef _RV_TEST_FILTER_H_
#define _RV_TEST_FILTER_H_

#include "rv_test_inc.h"

#define SW_COMPILED		1
#define SW_NOT_COMPILED	0



#ifdef TEST_HCI_SWE
#define HCI_MISC        (SW_COMPILED)
#else
#define HCI_MISC        (SW_NOT_COMPILED)
#endif
#ifdef TEST_L2CAP_SWE
#define L2CAP_REGR      (SW_COMPILED)
#else
#define L2CAP_REGR      (SW_NOT_COMPILED)
#endif
#ifdef TEST_L2CAP_SWE
#define L2CAP_CERTIF    (SW_COMPILED)
#else
#define L2CAP_CERTIF    (SW_NOT_COMPILED)
#endif
#ifdef TEST_L2CAP_SWE
#define L2CAP_UPF4      (SW_COMPILED)
#else
#define L2CAP_UPF4      (SW_NOT_COMPILED)
#endif
#ifdef TEST_RFCOMM_SWE
#define RFCOMM_REGR     (SW_COMPILED)
#else
#define RFCOMM_REGR     (SW_NOT_COMPILED)
#endif
#ifdef TEST_RFCOMM_SWE
#define RFCOMM_UPF4     (SW_COMPILED)
#else
#define RFCOMM_UPF4     (SW_NOT_COMPILED)
#endif
#ifdef TEST_SPP_SWE
#define SPP_REGR        (SW_COMPILED)
#else
#define SPP_REGR        (SW_NOT_COMPILED)
#endif
#ifdef TEST_SPP_SWE
#define SPP_CERTIF      (SW_COMPILED)
#else
#define SPP_CERTIF      (SW_NOT_COMPILED)
#endif
#ifdef TEST_SPP_SWE
#define SPP_MISC        (SW_COMPILED)
#else
#define SPP_MISC        (SW_NOT_COMPILED)
#endif
#ifdef TEST_SDP_SWE
#define SDP_REGR        (SW_COMPILED)
#else
#define SDP_REGR        (SW_NOT_COMPILED)
#endif
#ifdef TEST_SDP_SWE
#define SDP_CERTIF      (SW_COMPILED)
#else
#define SDP_CERTIF      (SW_NOT_COMPILED)
#endif
#ifdef TEST_SDP_SWE
#define SDP_MISC        (SW_COMPILED)
#else
#define SDP_MISC        (SW_NOT_COMPILED)
#endif
#ifdef TEST_SDP_SWE
#define SDP_UPF4        (SW_COMPILED)
#else
#define SDP_UPF4        (SW_NOT_COMPILED)
#endif
#ifdef TEST_SDAP_SWE
#define SDAP_REGR       (SW_COMPILED)
#else
#define SDAP_REGR       (SW_NOT_COMPILED)
#endif
#ifdef TEST_ATP_SWE
#define ATP_REGR        (SW_COMPILED)
#else
#define ATP_REGR        (SW_NOT_COMPILED)
#endif
#ifdef TEST_ATP_SWE
#define ATP_MISC        (SW_COMPILED)
#else
#define ATP_MISC        (SW_NOT_COMPILED)
#endif
#ifdef TEST_ATP_SWE
#define ATP_DEMO        (SW_COMPILED)
#else
#define ATP_DEMO        (SW_NOT_COMPILED)
#endif
#ifdef TEST_HS_SWE
#define HS_REGR         (SW_COMPILED)
#else
#define HS_REGR         (SW_NOT_COMPILED)
#endif
#ifdef TEST_HS_SWE
#define HS_DEMO         (SW_COMPILED)
#else
#define HS_DEMO         (SW_NOT_COMPILED)
#endif
#ifdef TEST_BTCTRL_SWE
#define BTCTRL_MISC     (SW_COMPILED)
#else
#define BTCTRL_MISC     (SW_NOT_COMPILED)
#endif
#ifdef TEST_BTCTRL_SWE
#define BTCTRL_UPF4     (SW_COMPILED)
#else
#define BTCTRL_UPF4     (SW_NOT_COMPILED)
#endif
#ifdef TEST_DUN_SWE
#define DUN_MISC        (SW_COMPILED)
#else
#define DUN_MISC        (SW_NOT_COMPILED)
#endif
#ifdef TEST_DUN_SWE
#define DUN_UPF4        (SW_COMPILED)
#else
#define DUN_UPF4        (SW_NOT_COMPILED)
#endif
#ifdef TEST_FAX_SWE
#define FAX_UPF4        (SW_COMPILED)
#else
#define FAX_UPF4        (SW_NOT_COMPILED)
#endif
#ifdef TEST_EXPL_SWE
#define EXPL_REGR       (SW_COMPILED)
#else
#define EXPL_REGR       (SW_NOT_COMPILED)
#endif
#ifdef TEST_EXPL_SWE
#define EXPL_DEMO       (SW_COMPILED)
#else
#define EXPL_DEMO       (SW_NOT_COMPILED)
#endif
#ifdef TEST_TCS_SWE
#define TCS_REGR        (SW_COMPILED)
#else
#define TCS_REGR        (SW_NOT_COMPILED)
#endif
#ifdef TEST_OBX_SWE
#define OBX_REGR        (SW_COMPILED)
#else
#define OBX_REGR        (SW_NOT_COMPILED)
#endif
#ifdef TEST_BMI_SWE
#define BMI_MISC        (SW_COMPILED)
#else
#define BMI_MISC        (SW_NOT_COMPILED)
#endif
#ifdef TEST_SCM_SWE
#define SCM_MISC        (SW_COMPILED)
#else
#define SCM_MISC        (SW_NOT_COMPILED)
#endif
#ifdef TEST_OPP_SWE
#define OPP_REGR        (SW_COMPILED)
#else
#define OPP_REGR        (SW_NOT_COMPILED)
#endif
#ifdef TEST_OPP_SWE
#define OPP_UPF4        (SW_COMPILED)
#else
#define OPP_UPF4        (SW_NOT_COMPILED)
#endif
#ifdef TEST_SYN_SWE
#define SYN_REGR        (SW_COMPILED)
#else
#define SYN_REGR        (SW_NOT_COMPILED)
#endif
#ifdef TEST_FTP_SWE
#define FTP_REGR        (SW_COMPILED)
#else
#define FTP_REGR        (SW_NOT_COMPILED)
#endif
#ifdef TEST_FTP_SWE
#define FTP_UPF4        (SW_COMPILED)
#else
#define FTP_UPF4        (SW_NOT_COMPILED)
#endif
#ifdef TEST_FFS_SWE
#define FFS_MISC        (SW_COMPILED)
#else
#define FFS_MISC        (SW_NOT_COMPILED)
#endif
#ifdef TEST_RTC_SWE
#define RTC_MISC        (SW_COMPILED)
#else
#define RTC_MISC        (SW_NOT_COMPILED)
#endif
#ifdef TEST_RTC_SWE
#define RTC_REGR        (SW_COMPILED)
#else
#define RTC_REGR        (SW_NOT_COMPILED)
#endif
#ifdef TEST_AUDIO_SWE
#define AUDIO_MISC      (SW_COMPILED)
#else
#define AUDIO_MISC      (SW_NOT_COMPILED)
#endif
#ifdef TEST_AUDIO_SWE
#define AUDIO_REGR      (SW_COMPILED)
#else
#define AUDIO_REGR      (SW_NOT_COMPILED)
#endif
#ifdef TEST_R2D_SWE
#define R2D_MISC		(SW_COMPILED)
#else
#define R2D_MISC		(SW_NOT_COMPILED)
#endif
#ifdef TEST_R2D_DEMO
#define R2D_DEMO     (SW_COMPILED)
#else
#define R2D_DEMO     (SW_NOT_COMPILED)
#endif
#ifdef TEST_DAR_SWE
#define DAR_MISC        (SW_COMPILED)
#else
#define DAR_MISC        (SW_NOT_COMPILED)
#endif
#ifdef TEST_DAR_SWE
#define DAR_REGR        (SW_COMPILED)
#else
#define DAR_REGR        (SW_NOT_COMPILED)
#endif
#ifdef TEST_PWR_SWE
#define PWR_MISC      (SW_COMPILED)
#else
#define PWR_MISC      (SW_NOT_COMPILED)
#endif
#ifdef TEST_PWR_SWE
#define PWR_REGR      (SW_COMPILED)
#else
#define PWR_REGR      (SW_NOT_COMPILED)
#endif
#ifdef TEST_KPD_SWE
#define KPD_MISC        (SW_COMPILED)
#else
#define KPD_MISC        (SW_NOT_COMPILED)
#endif
#ifdef TEST_MKS_SWE
#define MKS_MISC        (SW_COMPILED)
#else
#define MKS_MISC        (SW_NOT_COMPILED)
#endif
#ifdef TEST_MPM_SWE
#define MPM_MISC        (SW_COMPILED)
#else
#define MPM_MISC        (SW_NOT_COMPILED)
#endif
#ifdef TEST_MPM_SWE
#define MPM_DEMO        (SW_COMPILED)
#else
#define MPM_DEMO        (SW_NOT_COMPILED)
#endif
#ifdef TEST_RGUI_SWE
#define RGUI_MISC		(SW_COMPILED)
#else
#define RGUI_MISC		(SW_NOT_COMPILED)
#endif
#ifdef TEST_JAVA_K_SWE
#define UVM_MISC        (SW_COMPILED)
#else
#define UVM_MISC        (SW_NOT_COMPILED)
#endif
#ifdef TEST_DEV1_SWE
#define DEV1_MISC       (SW_COMPILED)
#else
#define DEV1_MISC       (SW_NOT_COMPILED)
#endif
#ifdef TEST_DEV2_SWE
#define DEV2_MISC       (SW_COMPILED)
#else
#define DEV2_MISC       (SW_NOT_COMPILED)
#endif
#ifdef TEST_DEV3_SWE
#define DEV3_MISC       (SW_COMPILED)
#else
#define DEV3_MISC       (SW_NOT_COMPILED)
#endif
#ifdef TEST_LLS_SWE
#define LLS_MISC        (SW_COMPILED)
#else
#define LLS_MISC        (SW_NOT_COMPILED)
#endif
#ifdef TEST_LZO_SWE
#define LZO_MISC        (SW_COMPILED)
#else
#define LZO_MISC        (SW_NOT_COMPILED)
#endif
#ifdef TEST_MDC_SWE
#define MDC_MISC      (SW_COMPILED)
#else
#define MDC_MISC      (SW_NOT_COMPILED)
#endif
#ifdef TEST_MDC_SWE
#define MDC_REGR      (SW_COMPILED)
#else
#define MDC_REGR      (SW_NOT_COMPILED)
#endif
#ifdef TEST_MDL_SWE
#define MDL_MISC      (SW_COMPILED)
#else
#define MDL_MISC      (SW_NOT_COMPILED)
#endif
#ifdef TEST_MDL_SWE
#define MDL_REGR      (SW_COMPILED)
#else
#define MDL_REGR      (SW_NOT_COMPILED)
#endif



/*
 *
 *   Define DEVICE SELECTION parameter.
 *
 *   This parameter allows to select the device used for testing.
 *
 *   ONLY used with BOARD
 *
 */

#ifndef _WINDOWS
#define DEVICE_A (1)
#endif


#endif /* _RV_TEST_FILTER_H_ */
