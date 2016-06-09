/**
 * @file	rv_test_exec_def.h
 *
 * LCD and Keypad definitions.
 *
 * All the definitions below are used to control LCD and keypad
 * when being in EXEC mode (tests to execute are chosen at execution
 * time with the keypad and the LCD).
 *
 * @author	Cedric Baudelet
 * @author	Vincent Oberle
 */

/*
 * History:
 *
 *	Date       	Author					Modification
 *	-------------------------------------------------------------------
 *	03/13/02	Vincent Oberle			Put from rv_test.h in its own file.
 *
 * (C) Copyright 2002 by Texas Instruments Incorporated, All Rights Reserved
 */

#ifndef _RV_TEST_EXEC_DEF_H_
#define _RV_TEST_EXEC_DEF_H_


#define TEST_TYPE_STATE				(1)
#define TEST_LEVEL_STATE			(2)
#define TEST_NUMBER_STATE			(3)
#define NB_OF_TEST_TO_PASS_STATE	(4)
#define TEST_AUDIO_STATE			(5)
#define TEST_AUDIO_SERVICES_STATE	(6)

#define LCD_LEN (6)

/* RV TEST TEST_TYPE PARAMETER */
#define TEST_TYPE_LCD_LEN (6)
static char TEST_TYPE_LCD [TEST_TYPE_LCD_LEN][15] = {"  TEST TYPE   ",
													 "  1 - REGR    ",
													 "  2 - CERTIF  ",
													 "  3 - DEMO    ",
													 "  4 - MISC    ",
													 "  5 - UPF     "};

/* RV TEST TEST_AUDIO PARAMETER */
#define TEST_AUDIO_LCD_LEN (3)
static char TEST_AUDIO_LCD [TEST_TYPE_LCD_LEN][15] = {"  TEST AUDIO ",
													  "  1-LIST    ",
													  "  2-ONESHOT  "};
/* RV TEST TEST_AUDIO_SERVICES PARAMETER */
#define TEST_AUDIO_SERVICES_LCD_LEN (5)
static char TEST_AUDIO_SERVICES_LCD [TEST_TYPE_LCD_LEN][15] = {" AUDIO SERVICE",
															   " 1 - KEYBEEP",
															   " 2 - TONES  ",
															   " 3 - MELODY "};

/* RV TEST TEST_MPM_SERVICES PARAMETER */
#define TEST_MPM_LCD_LEN (/*8*/6)
static char TEST_MPM_LCD [TEST_MPM_LCD_LEN][15] = {  "   TEST MPM   ",
					    						     "              ",
													 "1- TEST LOOP  ",
													 "2- WAIT STATES",
													 "3- PREDEFINED ",
													 "   COMMANDS   "}; /*,
													 "4- OTHER      ",
													 "   COMMAND    "};*/



#endif /* _RV_TEST_EXEC_DEF_H_ */
