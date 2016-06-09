/**
 * @file	rv_test_cfg.h
 *
 * This file allows to set parameters used to perform tests.
 *
 * Parameters to set are:
 * - TEST_INIT (to set parameters statically or dynamically)
 * - TEST_TYPE
 * - TEST_LEVEL
 * - TEST_NUMBER
 * - TEST_CFG (non regression)
 * - TEST_TABLE (non regression)
 * - NB_OF_TEST_TO_PASS (non regression)
 * - TEST_OCC (non regression)
 *
 * It is also possible to set bd_addr used for the tests by
 * setting BD_ADDR_LOC and BD_ADDR_REM values at the end of this file.
 *
 * IMPORTANT: Copy contents of rv_test_cfg_template.h
 * file into this file before building.
 *
 * @author	Cedric Baudelet
 * @author	Vincent Oberle
 */

#ifndef _RV_TEST_CFG_H_
#define _RV_TEST_CFG_H_

#include "rv_test_regr.h"


/**
 *
 * Define TEST_INIT parameter.
 *
 * TEST_INIT parameter's available values are:
 *
 * -> COMP:   Set testing parameters statically during compilation using
 *            rv_test_cfg file.
 * -> EXEC:   Set testing parameters dynamically during execution using LCD
 *            and keypad.
 *
 */
#define TEST_INIT (EXEC)



/*
 * Only with TEST_INIT = COMP
 * --------------------------
 */

/**
 *
 * Define TEST_TYPE parameter.
 *
 * TEST_TYPE parameter's available values are:
 *
 * -> REGR:   Execute NON REGRESSION tests.
 * -> CERTIF: Execute CERTIFICATION tests.
 * -> DEMO:   Execute DEMONSTRATION events.
 * -> MISC:   Execute MISCELLANEOUS tests.
 * -> UPF4:   Execute UPF4 tests.
 *
 */
#define TEST_TYPE (MISC)



/**
 *
 * Define TEST_LEVEL parameter.
 *
 * TEST_LEVEL parameter is TEST_XXX_SWE, where XXX can take one of the
 * following values:
 *
 * REGR:
 * |    L2CAP     RFCOMM    SPP       SDP       SDAP      ATP
 * |    HS        OBX       OPP       EXPL      FFS       SYN
 * |    DAR       PWR       TUT
 * |______________________________________________________________
 *
 * CERTIF:
 * |    L2CAP     SDP       SPP
 * |______________________________________________________________
 *
 * DEMO:
 * |    HS        EXPL      RNET      TUT		R2D
 * |______________________________________________________________
 *
 * MISC:
 * |    BTE       SDP       HCI       SPP       BTCTRL    DUN_GW
 * |    ATP       TCS       FFS       BMI       SCM
 * |    RTC       AUDIO     R2D       KPD       TUT       MKS
 * |    RGUI      RNET      UVM       MSME
 * |______________________________________________________________
 *
 * UPF4:
 * |    L2CAP     RFCOMM    SDP       SPP       BTCTRL    DUN_GW
 * |    OPP       FAX_GW
 * |______________________________________________________________
 *
 */
#define TEST_LEVEL (TEST_MSME_SWE)


/**
 *
 * Define TEST_NUMBER parameter.
 *
 * This parameter should not be used with REGR TEST_TYPE (except if
 * TEST_TABLE[] = {TEST_NUMBER, CONFIG_X, EOL}.
 *
 */
#define TEST_NUMBER (5)



/*
 * Only with TEST_MISC = REGR
 * --------------------------
 */

/**
 *
 * Define TEST_CFG parameter.
 *
 * TEST_CFG parameter's available values are:
 *
 * -> TEST_LIST:      Execute all scenarios from TEST_TABLE[] sequentially.
 * -> TEST_LIST_RAND: Execute NB_OF_TEST_TO_PASS scenarios from TEST_TABLE[] randomly.
 * -> TEST_ALL:       Execute all the scenarios sequentially using the two
 *                    regression configurations (CONFIG_A and CONFIG_B).
 * -> TEST_ALL_RAND:  Execute NB_OF_TEST_TO_PASS scenarios randomly using the
 *                    two regression configurations (CONFIG_A and CONFIG_B).
 *
 * Note:
 *    If TEST_INIT = EXEC, test_cfg parameter (defined in rv_test.c file) is
 *    automatically set to TEST_ALL_RAND value.
 *
 */
#define TEST_CFG (TEST_ALL_RAND)


/**
 *
 * Define TEST_TABLE parameter.
 *
 * T_RV_REGR_TEST_LIST type is:
 * -> T_RV_REGR_TEST_LIST_NBR: Scenario number.
 * -> T_RV_REGR_TEST_LIST_CFG: Test configuration (CONFIG_A or CONFIG_B)
 *
 * Note:
 *    TEST_TABLE MUST contain EOL flag as its last element.
 *
 */
static const T_RV_REGR_TEST_LIST TEST_TABLE[] = {TEST_NUMBER, CONFIG_A, TEST_NUMBER, CONFIG_B,
                                                 EOL};

/**
 *
 * Define NB_OF_TEST_TO_PASS parameter.
 *
 * This parameter is only used with REGR TEST_TYPE when TEST_CFG parameter is
 * set to TEST_LIST_RAND or TEST_ALL RAND. Else not used.
 *
 */
#define NB_OF_TEST_TO_PASS (1)


/**
 *
 * Define TEST_OCC parameter.
 *
 * This parameter defines the number of occurence for each test.
 *
 * Note:
 *    This parameter can't be set dynamically.
 *
 */
#define TEST_OCC (1)


/*
 * Only with TEST LEVEL = AUDIO
 * ----------------------------
 */

/**
 *
 * Define TEST_AUDIO parameter.
 *
 * TEST_AUDIO parameter's available values are:
 * -> LIST:     to test all the Keybeep or Tones or Melody or Voice memo Tests
 * -> ONESHOT:  to test just one audio test
 */
#define TEST_AUDIO (ONESHOT)



/*
 * Only with BT devices
 * --------------------
 */

/**
 *
 * bd_addr of BT devices used to execute tests.
 *
 * Note that in case where two separated devices are used, BD_ADDR_LOC and
 * BD_ADDR_REM values shouldn't depend on DEVICE_A or DEVICE_B.
 *
 */

#ifdef DEVICE_A
   #define BD_ADDR_LOC {0x00, 0xD0, 0xB7, 0x03, 0x17, 0x26}
   #define BD_ADDR_REM {0x00, 0xD0, 0xB7, 0x03, 0x17, 0x48}
#endif
#ifdef DEVICE_B
   #define BD_ADDR_LOC {0x00, 0xD0, 0xB7, 0x03, 0x17, 0x48}
   #define BD_ADDR_REM {0x00, 0xD0, 0xB7, 0x03, 0x17, 0x26}
#endif


#endif /* _RV_TEST_CFG_H_ */
