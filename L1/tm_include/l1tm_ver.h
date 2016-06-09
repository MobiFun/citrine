/************* Revision Controle System Header *************
 *                  GSM Layer 1 software 
 * L1TM_VER.H
 *
 *        Filename l1tm_ver.h
 *  Copyright 2003 (C) Texas Instruments  
 *
 ************* Revision Controle System Header *************/

/*****************************/
/* Test mode version numbers */
/*****************************/


//#define    TESTMODEVERSION  0x0300L // All previous TestMode versions pertain to
                                    // GGT Protocol version 2.0, which has been replaced
                                    // by version 3.0.

//#define    TESTMODEVERSION  0x0320L // AFC Params part of "rftw"; APCDEL intialized in "tm_init"; New fcn "l1tm_initialize_var" to switch to TestMode
                                    // Uplink-only tests loop count; RACH loop count; Dedicated mode Pass/Fail stats updated
                                    // New structure "T_TM_RETURN_ABBREV"; Continous TX/RX; New fcn "tmstats_auto_result_reset_loops";
                                    // Corrected BSIC reporting problem; Power skip fcn implemented ("rftw/rftr 16"); A-F Loopback modes supported;
                                    // Chan. monitoring while in dedic. mode possible; FB26/D_BAMS_MEAS tasks enabled only with chan. monitoring;
                                    // New fcn "tmstats_mon_confirm"; RF struct incorporates new TX descriptor table;
                                    // AGC includes lna_off bit (least-significant bit); agc_, afc_ & adc_enable part of l1_config struct;
                                    // Variable number of guard bits in "l1tm_fill_burst"......
//#define    TESTMODEVERSION  0x0321L // Implemented FRAME_NUMBER in l1tm_stats_read(). Timing advance takes effect immediately. stats_read reports 16 or 32 bit values only.
                                    // Only one uplink message sent. Some indices in l1tm_version_get implemented. AGC gain and lna_off set only once to save instruction cycles.
                                    // Cleaned up enums in l1_tm_types.h. Removed case STANDARD from rf_param_read. 
                                    // Updated all argument types of testmode functions according to TM100 v3.2.1. Changed mem_read to support reception of page and 
                                    // register values in one 16-bit value from PCTM.  mem_read can read up to 124 bytes. Changed the TestMode primitive structure to include
                                    // only 2 generic ones plus mem_write and mem_read.

//#define    TESTMODEVERSION  0x0400L // REQ991:
                                    // - SIMULATION for test mode => new non-regression flow regress_tm.bat
                                    // - Rework of test mode state machines to make them independent from each other
                                    // - Rework of simulated upper layers (L3,CST) to be able to run L1 scenarios with TESTMODE=3
                                    // Correction of BUG989, BUG990, BUG992

//#define    TESTMODEVERSION  0x0401L // Merge with TM version 0x321

//#define    TESTMODEVERSION  0x0402L // TM rework PART II
//#define    TESTMODEVERSION  0x0403L // Alignment with TM100 v.3.5.0
                                    // Implementation of new functionality: single PM on monitor channel (rfe 15)
//#define    TESTMODEVERSION  0x0404L // Closed REQ01129: Implementation of GPRS test mode
//#define    TESTMODEVERSION  0x0405L // Alignment with SSA 5.3.3, 
                                      // Correction of BUG2048, BUG2050, BUG2294, BUG2295
#define    TESTMODEVERSION  0x0406L   // REQ03410


//#define    TMAPIVERSION     0x0350L  // New: TM API version (TM100 version number)
//#define    TMAPIVERSION     0x0360L  // TM100 change for GPRS test mode
//#define    TMAPIVERSION     0x0361L  // tm patch 361 for SW version 520
//#define    TMAPIVERSION     0x0362L  // tm patch 362 for SW version 531
#define    TMAPIVERSION     0x0370L  // Multi-band support

