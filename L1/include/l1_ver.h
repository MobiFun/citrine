/************* Revision Controle System Header *************
 *                  GSM Layer 1 software
 * L1_VER.H
 *
 *        Filename l1_ver.h
 *  Copyright 2003 (C) Texas Instruments  
 *
 ************* Revision Controle System Header *************/

/*********************************************************/
/* Software Version Integrated on Evaluation board 3 / 4 */
/*********************************************************/

//#define    SOFTWAREVERSION  0x0300L;  // Release Cust 5 0.3
//#define    SOFTWAREVERSION  0x0301L;  // + LOOPS A,B,C + A5 (SDCCH/TCH) + DTX
                                        // + FER Rxlev,Facch + new hw switches
//#define    SOFTWAREVERSION  0x0400L;  // Release Cust 5 0.4
//#define    SOFTWAREVERSION  0x0401L;  // SAXO version for next Release
//#define    SOFTWAREVERSION  0x0402L;  // + new stats/traces - a_sch26
//#define    SOFTWAREVERSION  0x0500L;  // Release Cust 5 0.5 (Idle/Dedic monitoring)
//#define    SOFTWAREVERSION  0x0610L;  // Release Cust 5 0.61 (POLE50/VEGA2/CBCH)
//#define    SOFTWAREVERSION  0x0700L;  // ROM Code
//#define    SOFTWAREVERSION  0x0720L;  // Release Cust5 0.72 (tones + AGC/AFC)
//#define    SOFTWAREVERSION  0x0721L;  // Rom bug (l1_sync.c) fixed + test msg + DEDICATED MODE
//#define    SOFTWAREVERSION  0x0800L;  // IDLE MODE cpu optimization
//#define    SOFTWAREVERSION  0x0801L;  // DAI tests +  PARAM update
//#define    SOFTWAREVERSION  0x0820L;  // TI_1, TI_3 + TX data buffer size
//#define    SOFTWAREVERSION  0x0821L;  // TI_1 to TI_14 problem corrected.
//#define    SOFTWAREVERSION  0x0827L;  // Release: TI_16, TI_18, TI_19, TI_21, TI_22, TI_23, TI_24,
                                        // CUST5_6, CUST5_7, CUST5_9 corrected.
//#define    SOFTWAREVERSION  0x0828L;  // Release: CUST5_14, CUST5_15, CUST5_16, TI_20
//#define    SOFTWAREVERSION  0x0829L;  // Release: TI_28
//#define    SOFTWAREVERSION  0x0830L;  // TI internal release: TI_17, TI_29, TI_32, TI_34, TI_35, TI_36,
                                        //                      TI_37, TI_40, TI_41.
                                        //                      CUST5_21, CUST5_22, CUST5_26, CUST5_28,
                                        //                      CUST5_29, CUST5_30, CUST5_31, CUST5_33.
//#define    SOFTWAREVERSION  0x0831L;  // TI internal release: TI_44(a), CUST5_33, CUST5_35, CUST5_41,
                                        //                      TI_44(b)(c), TI_35, CUST5_37, TI_44(e),
                                        //                      TI_44(f), CUST5_40, CUST5_44, TI_44(g),
                                        //                      CUST5_38, CUST5_39, CUST5_43, CUST5_45,
                                        //                      CUST5_49, CUST5_50, TI_44(i), TI_47,
                                        //                      TI_44(j1), TI_44(o), TI_43, CUST5_51,
                                        //                      CUST5_52 problems corrected.
//#define    SOFTWAREVERSION  0x0850L;  // TI FTA L1 SW integrated in new DB.
//#define    SOFTWAREVERSION  0x0851L;  // TI FTA L1 SW with field test problems corrected
                                        // up to TI_59 and CUST5_59.
//#define    SOFTWAREVERSION  0x0860L;  // Merged FR FTA code (v0.851) and HR code (0.831 based + HR).
//#define    SOFTWAREVERSION  0x0861L;  // New PM=0/MCU-DSP Mismatch tracing method.
//#define    SOFTWAREVERSION  0x0870L;  // Merged v0.860 and EFR code v0x1008.
//#define    SOFTWAREVERSION  0x0871L;  // Merged v0.861 and v0.870. (result contains 0.870
                                        // functionality + improved
                                        // tracing capabilities.
//#define    SOFTWAREVERSION  0x0872L;  // Updated EVA3 diver library to have capabilites
                                        // to both pooled and Interrupt driven.
//#define    SOFTWAREVERSION  0x0873L;  // Reviewed TPU drivers. l1_const.h has been split
                                        // to separate Timing definitions in l1_time.h
//#define    SOFTWAREVERSION  0x0874L;  // Corrected bad reference made to l1_rf#.h
                                        // and tpudrv#.h during "Reviewed TPU drivers".
//#define    SOFTWAREVERSION  0x0875L;  // Removed redondant definition of PRG_TX from
                                        // l1_time.h. CUST0/RF1-2 use PRG_TX=20.
//#define    SOFTWAREVERSION  0x0900L;  // Dual Band SW based on L1 generic database (v0.875)
//#define    SOFTWAREVERSION  0x0901L;  // Added DSP selection capabilities.
//#define    SOFTWAREVERSION  0x0902L;  // Corrected TI_60, TI_61, TI_65. Corrected miscelaneous
                                        // debug/trace mistakes.
//#define    SOFTWAREVERSION  0x0903L;  // Release: CUST5. Include AGC mods based on FTA problems.
//#define    SOFTWAREVERSION  0x0904L;  // Due to problem in V0.903 release,
                                        // generation of V0.904 which includes
                                        // the unexpected missing features of V0.903.
//#define    SOFTWAREVERSION  0x0905L;  // Version including Triple vocoder feature.
//#define    SOFTWAREVERSION  0x0906L;  // Added TXPWR MANAGEMENT.
//#define    SOFTWAREVERSION  0x0907L;  // Added ADC RESULT report msg L1S->L3 mechanism.
//#define    SOFTWAREVERSION  0x0908L;  // Added POWER MANAGEMENT mechanism.
//#define    SOFTWAREVERSION  0x0909L;  // DSP = 4 updated in order to deal with Pole112
//#define    SOFTWAREVERSION  0x0910L;  // CLKMOD2 modified for DSP == 1, patch_file6.c and
                                        // patch_file5.c updated
//#define    SOFTWAREVERSION  0x0911L;  // AGC and TOA management update according to TI/C5 code review.
//#define    SOFTWAREVERSION  0x0912L;  // Corrected PB #6,7,14,17,18,19,20.
//#define    SOFTWAREVERSION  0x0913L;  // Integated TXPWR for Dual Band, DSP Patch updated, AGC reworked
                                        // in order in order to cover all bands, corrected PB
                                        // #2/22,21,23,24,25,26,28,29
//#define    SOFTWAREVERSION  0x0914L;  // Modified FB search algorithm (FBNEW, FB51)
                                        // so that FB search aborted
                                        // once the FB has been found
//#define    SOFTWAREVERSION  0x0915L;  // Integratd Data Services modifications implemented
//#define    SOFTWAREVERSION  0x0916L;  // Reworked power management
//#define    SOFTWAREVERSION  0x0917L;  // A-sample, protocol stack compatibility, 4PM, audio functions
//#define    SOFTWAREVERSION  0x0918L;  // Voice memo functions
                                        // Uplink a_du_0 blud bit set by layer
                                        // for IDS mode.
//#define    SOFTWAREVERSION  0x1019L;  // Version 10xx indicates L1 standalone 1.5,
                                        // version 19 -> IDS modifications + TPU drivers for ATL RF
//#define    SOFTWAREVERSION  0x1020L   // version 20 Porting of eva3drivers1 on HER. SATU compatibility
                                        // kept.
                                        // Correction of a commentation bug in SPIOmega_on()
                                        // Correction of a bug in l1dmacro_init_hw() for IO accesses
//#define    SOFTWAREVERSION  0x1021L   // SPEECH RECO.
//#define    SOFTWAREVERSION  0x1318L   // Voice memo functions
//#define    SOFTWAREVERSION  0x1319L   // version 19 -> IDS modifications + TPU drivers for ATL RF
//#define    SOFTWAREVERSION  0x1320L   // version 20 Porting of eva3drivers1 on HER. SATU
                                        // compatibility kept.
                                        // Correction of a commentation bug in SPIOmega_on()
                                        // Correction of a bug in l1dmacro_init_hw() for IO accesses
//#define    SOFTWAREVERSION  0x1321L   // Corrected TI_5, TI_6, TI_8, TI_9, TI_10, TI_11
//#define    SOFTWAREVERSION  0x1322L   // New Power mnagement algo. (+ Specch reco. NOT TESTED)
//#define    SOFTWAREVERSION  0x1x23L   // SPEECH RECO corrections
//#define    SOFTWAREVERSION  0x1x24L   // SPEECH RECO corrections
                                        // Corrected TI_19
//#define    SOFTWAREVERSION  0x1325L   // Corrected TI_2, TI_13, TI_16, TI_18
                                        // Added ULYSSE switch
//#define    SOFTWAREVERSION  0x1326L   // Update speech recognition interface to MMI
                                        // Create audio loop and audio filter functions
//#define    SOFTWAREVERSION  0x1327L   // Corrected TI_44 and TI_67.
//#define    SOFTWAREVERSION  0x1328L   // Corrected TI_101,TI_121,TI_112,TI_104
                                        // TI_100,TI_99,TI_134,TI_127,TI_126
//#define    SOFTWAREVERSION  0x1329L   // Corrects TI_181/182/183/184/185/186/187/193/194
                                        //          210/212/216/217
//#define    SOFTWAREVERSION  0x1330L   // Corrected TI_122, TI_124 and TI_125.
//#define    SOFTWAREVERSION  0x1331L   // Extensions for VoiceMemo and Melodies
//#define    SOFTWAREVERSION  0x1332L   // Integration of PASCAL RF drivers with new RF structure
                                        // Integration of new H/W drivers for B-sample and EVA4 board
                                        // Integration of bootloader for flash
//#define    SOFTWAREVERSION  0x1333L   // include the old versions:
                                        // (!!ALRVERSION is no more used!!)
                                        // ALRVERSION  0x0003L => Corrected TI_14, TI_15, TI_17
                                        // ALRVERSION  0x0004L => Correction of TI_23,TI_24,
                                        //                        TI_25,TI_29,TI_30
                                        // ALRVERSION  0x0005L => Correction of TI_108, TI_133, TI_139
                                        // ALRVERSION  0x0006L => Correction of TI_160,TI_246, TI_153,
                                        //                        TI_265, TI_289,TI_283,TI_80
                                        // ALRVERSION  0x0007L => Correction of BUG_539:
                                        //                        suppress Vega compatibility
                                        // ALRVERSION  0x0008L => Correction of TI_188,TI_169,
                                        //                        TI_225,TI_243, PB659,PB660,PB661,
                                        //                       PB662,PB679, PB680, PB 681
                                        //
                                        // Correction: BUG_569,BUG_557,BUG_566,BUG_584,TI_253,TI_32,
                                        //             TI_137,BUG_534,BUG_651,TI_253,BUG_534
//#define    SOFTWAREVERSION  0x1334L   // New trace (TRACE_TYPE 1 and 4), corrected BUG00683
//#define    SOFTWAREVERSION  0x1335L   // Corrected bug TI_2622, TI_263, TI_264, TI_298, BUG532,
                                        // BUG531, BUG537, BUG758.

//#define    SOFTWAREVERSION  0x1336L   // Corrected bug BUG583 and the change CHG669.
//#define    SOFTWAREVERSION  0x1337L   // Corrected TI_266, BUG_657, BUG_686, BUG_698
//#define    SOFTWAREVERSION  0x1338L   // Corrected BUG_685, BUG_748, CHG_766, REQ_767, REQ_768,
                                        // REQ_769, BUG_773 (new patch_file17)
                                        // Integration of TM3
//#define    SOFTWAREVERSION  0x1339L   // Corrected BUG_759, BUG_760, REQ_822, BUG_829, BUG_832
                                        // TI_244
//#define    SOFTWAREVERSION  0x1340L   // adapted to simulator (ALR_SIMUVERSION 0x0002)
                                        // corrected TI_116
//#define    SOFTWAREVERSION  0x1341L   // Trace rework step 2: use of a Riviera Tool trace module
                                        // emulator - Corrected TI_206
//#define    SOFTWAREVERSION  0x1342L   // Corrected CHG939.
//#define    SOFTWAREVERSION  0x1343L   // Corrected BUG960, BUG961.
//#define    SOFTWAREVERSION  0x1344L   // Corrected BUG0878, BUG0880.
//#define    SOFTWAREVERSION  0x1345L   // Corrected TI_242, TI_198, REQ_870, BUG_881,BUG_887,
                                        // BUG 693, CHG_856, BUG_932, REQ_934, BUG_962, BUG_973
//#define    SOFTWAREVERSION  0x1346L   // Test mode version 0400 implementation
                                        // Update of BUG887
//#define    SOFTWAREVERSION  0x1347L   // IDS simulation corrections : BUG00654 BUG00655 BUG00656
//#define    SOFTWAREVERSION  0x1348L   // correction of compilation problem related to IDS trace : BUG_1061
//#define    SOFTWAREVERSION  0x1349L   // correction of CHG_770, BUG_1013, BUG_1047
//#define    SOFTWAREVERSION  0x1350L   // This version contains the audio rework described in the S916 spec.
                                        // Moreover, the following bugs are fixed:
                                        // CHG00788, BUG00926, BUG00963, CHG00969, BUG01042, BUG01049, BUG0530,
                                        // BUG00677, BUg00704, REQ00738, BUG00966, CHG01089, TI_209,
                                        // TI_211, TI_273.TI_274.
//#define    SOFTWAREVERSION  0x1351L   // Corrected TI_52, TI_208, BUG1012, BUG719, BUG1052, BUG1078
//#define    SOFTWAREVERSION  0x1352L   // Change due to the customer request:REQ1191.
                                        // Corrected bug: BUG1194.
//#define    SOFTWAREVERSION  0x1353L   // Corrected BUG0551, BUG1268, BUG1277, BUG1283, BUG1284, BUG1286,
                                        // REQ_1371, BUG1376
//#define    SOFTWAREVERSION  0x1354L   // Corrected BUG1268, BUG1286, BUG1426
//#define    SOFTWAREVERSION  0x1355L   // Corrected BUG1435
//#define    SOFTWAREVERSION  0x1356L   // Corrected REQ1422, BUG1491, BUG1433,
                                        // REQ1538
//#define    SOFTWAREVERSION  0x1357L   // Corrected REQ1420, CHG1500, REQ1503
//#define    SOFTWAREVERSION  0x1358L   // Corrected REQ1232, BUG1253, REQ1328, BUG1330, BUG1729
//#define    SOFTWAREVERSION  0x1359L   // Corrected REQ1431, BUG1520, BUG1223
//#define    SOFTWAREVERSION  0x1360L   // Corrected BUG1734
//#define    SOFTWAREVERSION  0x1361L   // Corrected BUG1360,BUG950,REQ849,REQ1627,BUG1738
//#define    SOFTWAREVERSION  0x1362L   // Corrected CHG1289, BUG1719, BUG1720, REQ1722, CHG1724.
//#define    SOFTWAREVERSION  0x1363L   // Corrected REQ1552, REQ1612, BUG1650.
//#define    SOFTWAREVERSION  0x1364L   // Corrected BUG848, BUG1473
//#define    SOFTWAREVERSION  0x1365L   // Corrected BUG1558
//#define    SOFTWAREVERSION  0x1366L   // Corrected BUG1791
//#define    SOFTWAREVERSION  0x1367L   // AMR integration REQ1858
                                        // Corrected BUG1825, BUG1818, BUG1809,
                                        // CHG1638, BUG1803,
//#define    SOFTWAREVERSION  0x1368L   // Closed REQ1917, REQ1919, REQ1920
//#define    SOFTWAREVERSION  0x1369L   // Closed BUG1952, REQ1953, BUG1954, BUG1955, CHG1956, new DSP code: 34 (AMR)
//#define    SOFTWAREVERSION  0x1370L   // Closed REQ1209, BUG1813, BUG1814, BUG1815, REQ1816, REQ1817
                                        // BUG1823, BUG1824, BUG1991, REQ1992
//#define    SOFTWAREVERSION  0x1371L   // Closed BUG1848,BUG1937,BUG1990
//#define    SOFTWAREVERSION  0x1372L   // Corrected BUG1837, CHG1943, CHG1944
//#define    SOFTWAREVERSION  0x1373L   // Corrected CHG2003, BUG2011, BUG2020, BUG2015, BUG2025, update DSP code 34
//#define    SOFTWAREVERSION  0x1374L   // Corrected CHG2080
//#define    SOFTWAREVERSION  0x1375L   // Closed CHG02057, REQ02086
//#define    SOFTWAREVERSION  0x1376L   // Closed REQ02114
//#define    SOFTWAREVERSION  0x1377L   // Closed BUG1941, REQ1998, BUG2047, BUG2049, BUG2019, BUG2113, BUG2148
//#define    SOFTWAREVERSION  0x1378L   // Melody E2 REQ2129
//#define    SOFTWAREVERSION  0x1379L   // Bug2160, BUG2166
//#define    SOFTWAREVERSION  0x1380L   // BUG2223, BUG2251, BUG2229, BUG2228, BUG2235,
                                        // REQ2204, BUG2249, BUG2203, CHG2230, BUG2175,
                                        // BUG2178, BUG2188, BUG2171
//#define    SOFTWAREVERSION  0x1381L   // REQ2262
//#define    SOFTWAREVERSION  0x1382L   // BUG2048, BUG2050, BUG2294, BUG2295, BUG2285,
                                        // REQ2315, REQ2316
//#define    SOFTWAREVERSION  0x1383L   // Corrected REQ1708, BUG1746, REQ2009, REQ2292, REQ2317
//#define    SOFTWAREVERSION  0x1384L   //REQ2367,BUG2350,BUG2343,BUG2266,BUG2364,BUG2293,BUG2259
                                        //BUG2277,BUG2368,BUG2348,BUG2245,REQ2373
//#define    SOFTWAREVERSION  0x1385L   // Corrected CHG842, BUG2162, BUG2176, BUG2260

//#define    SOFTWAREVERSION  0x1386L   // Corrected BUG2265, BUG2308, BUG2334, BUG2418
                                        // BUG2282, BUG1967,BUg2423, BUG2424, BUG2425
//#define    SOFTWAREVERSION  0x1387L   // Corrected REQ2428, REQ2386, CHG2432
//#define    SOFTWAREVERSION  0x1388L   // Corrected REQ2447
//#define    SOFTWAREVERSION  0x1389L   // Corrected BUG02486, CHG02487, BUG02488, BUG02492
//#define    SOFTWAREVERSION  0x1390L   // Corrected REQ02500
//#define    SOFTWAREVERSION  0x1391L   // Corrected REQ02345, BUG02388, BUG02434, BUG02436, BUG02461, BUG02462,
                                        //           BUG02526, BUG02547, BUG02548, BUG02549, REQ02551
//#define    SOFTWAREVERSION  0x1392L   // Corrected REQ02583,
//#define    SOFTWAREVERSION  0x1393L   // Corrected REQ2381 (L1 Binary Trace)
//#define    SOFTWAREVERSION  0x1394L   // Corrected REQ2516 (2nd order Tx Temperature Calibration)
                                        // Corrected BUG1993 (Testmode now functional in standalone mode)
//#define    SOFTWAREVERSION  0x1395L   // Integration of new AEC (Spec S892, REQ02477)
                                        // Corrected BUG02279, CHG02349, BUG02370, REQ02401, REQ02412, BUG02435, BUG02473,
                                        //           BUG02525, CHG02582, BUG02586, BUG02588, BUG02600
//#define    SOFTWAREVERSION  0x1396L   // Corrected BUG2608
                                        // Integration of REQ2571, REQ2572, REQ2603
//#define    SOFTWAREVERSION  0x1397L   // Corrected BUG1843, CHG2136, BUG2453
//#define    SOFTWAREVERSION  0x1398L   // Integration of Voice Memo AMR
                                        // CHG02618, CHG02620, CHG02622
//#define    SOFTWAREVERSION  0x1399L   // REQ2374,BUG2613,REQ2573,BUG2433,BUG2528,BUG2567
//#define    SOFTWAREVERSION  0x1400L   // BUG2630, BUG2638, REQ2643, new DSP patch files 0x2120 and 0x4120
//#define    SOFTWAREVERSION  0x1401L   // BUG2493,BUG2539,BUG2587,BUG2472,BUG2540
//#define    SOFTWAREVERSION  0x1402L   // REQ02736, REQ02740, BUG02733
//#define    SOFTWAREVERSION  0x1403L   // BUG2674, BUG2695, BUG2713, BUG2714, CHG2715, BUG2724, BUG2731, BUG2734, BUG2744
//#define    SOFTWAREVERSION  0x1404L   // BUG2542, BUG2615, BUG2687, REQ2748
//#define    SOFTWAREVERSION  0x1405L   // REQ02685, CHG02760, BUG02761, CHG02763, CHG02764
//#define    SOFTWAREVERSION  0x1406L   // BUG2563, BUG2663, BUG2703, BUG2719, BUG2750, CHG2773, CHG2774, CHG2775, BUG2776, BUG2777, New DSP patch files 0x2140 and 0x4130
//#define    SOFTWAREVERSION  0x1407L   // CHG02814, CHG02815, CHG02816, CHG02817, CHG02818,CHG02819, CHG2820, BUG02782, BUG02813
//#define    SOFTWAREVERSION  0x1408L   // REQ2845, BUG2846
//#define    SOFTWAREVERSION  0x1409L   // CHG2356, CHG2490, BUG2778, BUG2787, CHG2816, BUG02848, CHG02850, CHG02851
//#define    SOFTWAREVERSION  0x1410L   // CHG2877, CHG2879, BUG2883, REQ2884, BUG2891, CHG2898
//#define    SOFTWAREVERSION  0x1411L   // BUG02705, BUG02730, BUG02766, BUG02855
//#define    SOFTWAREVERSION  0x1412L   // CHG2972, BUG2928, BUG2929, CHG2931, BUG2932, BUG2933, REQ2934, CHG2935, REQ2936, CHG2938
//#define    SOFTWAREVERSION  0x1413L   // REQ2338, BUG2864, REQ2952, BUG2963, CHG2969, CHG2973, CHG2985, BUG2996, BUG3013
//#define    SOFTWAREVERSION  0x1414L   // BUG02467, CHG02497, BUG02580, BUG02921, BUG02922, REQ02997, CHG03005, CHG3041, CHG03042
//#define    SOFTWAREVERSION  0x1415L   // REQ02965: WCP integration
//#define    SOFTWAREVERSION  0x1416L   // REQ03070, REQ03075, CHG02898
//#define    SOFTWAREVERSION  0x1417L   // BUG03064, BUG03060, REQ03071, REQ03077, CHG03088, BUG03089, REQ03091
//#define    SOFTWAREVERSION  0x1418L   // REQ2947, CHG3087, BUG3093, CHG3104
//#define    SOFTWAREVERSION  0x1419L   // BUG3191, BUG3190, BUG3114, BUG3118, BUG3121, BUG3124, BUG3141, BUG3146, BUG3158, BUG3167, REQ3202, BUG3213, BUG3138, BUG3140, BUG3139
//#define    SOFTWAREVERSION  0x1420L   // CHG03154, CHG03155, BUG03125, REQ03194, REQ03204, REQ03218, REQ03219, CHG03230, REQ03231
//#define    SOFTWAREVERSION  0x1421L   // CHG3057, REQ3062, REQ3063, CHG3120, CHG3134, CHG3135, BUG3189, BUG3207, CHG3227, BUG3246, CHG3250, BUG3251
//#define    SOFTWAREVERSION  0x1422L   // BUG03212, BUG03174, REQ03241, REQ03242, REQ03243, REQ03244, REQ03262, BUG03265, BUG03266
//#define    SOFTWAREVERSION  0x1423L   // BUG03184, BUG03182, REQ03248
//#define    SOFTWAREVERSION  0x1424L   // BUG3083, BUG3188, BUG3221, REQ3228, BUG3257, CHG3264, BUG3267, BUG3268, CHG3269, BUG3273, BUG3293, BUG3294, BUG3295, BUG3300
//#define    SOFTWAREVERSION  0x1425L   // CHG03307
//#define    SOFTWAREVERSION  0x1426L   // CHG03059, CHG03306, CHG3319, CHG3330, BUG03331, CHG03344, CHG3345, CHG3346, CHG03347, REQ03349, CHG03352
//#define    SOFTWAREVERSION  0x1427L   // CHG03382, CHG03394, BUG03395, BUG03403, CHG03405, REQ03406
//#define    SOFTWAREVERSION  0x1428L   // CHG03415, CHG03402, CHG03411, REQ03414, REQ03396, REQ03410, BUG03401, BUG03312, BUG03372.
//#define    SOFTWAREVERSION  0x1429L   // CHG03417 
//#define    SOFTWAREVERSION  0x1430L   // CHG03425, BUG03371, REQ03429, REQ03431, CHG03432, CHG03434
//#define    SOFTWAREVERSION  0x1431L   // CHG2438, BUG2783, BUG3142, BUG3351, BUG3358, BUG3370, BUG3377, BUG3378, BUG3407, BUG3424, CHG3456, BUG3457, CHG3460, BUG3461
//#define    SOFTWAREVERSION  0x1432L   // CHG3472 

//#define    DEVELPMTVERSION  0x0001L
//#define    DEVELPMTVERSION  0x0002L  // Corrected TI_4, TI_7, TI_12
//#define    DEVELPMTVERSION  0x0003L  // Corrected TI_14, TI_15, TI_17


/*************************************************************************/
/* PORTING L1 TO GSM 1.5 Version Integrated on Evaluation board 3 / 4    */
/*************************************************************************/

//#define    PORT1.5VERSION     0x0001L;  // TI_13, TI_9, TI_11 fixed
//#define    PORT1.5VERSION     0x0002L;  // Omega PG 2.0 code integration
//#define    PORT1.5VERSION     0x0003L;  // DSP17, ULYSSE init
//#define    PORT1.5VERSION     0x0004L;  // Remove RIF TX/RX delay for Omega, correct one
                                          // string definition in l1_drive.c under DCS1800 compile switch
//#define    PORT1.5VERSION     0x0005L;  // added Omega 2.0 RF drivers
//#define    PORT1.5VERSION     0x0006L;  // added switch SPEECH_RECO in l1_defty.h
//#define    PORT1.5VERSION     0x0007L;  // TI_46, TI_47, TI_48 fixed
//#define    PORT1.5VERSION     0x0008L;  // added melody generators
//#define    PORT1.5VERSION     0x0009L;  // fixed ULPD bugs TI_64 & TI_65
                                          // added SETUP_RF to SETUP_FRAME
                                          // don't call sleep_mnager if gauging
//#define    PORT1.5VERSION     0x0010L;  // (fix TI_78,TI_79,TI_92, TI_93,TI_94,TI_95)
//#define    PORT1.5VERSION     0x0011L;  // fixed bugs : Melody generator (DSP/MCU bugs)
//#define    PORT1.5VERSION     0x0012L;  // Clean up of spi functions, AFC_ON switch,VDX
                                          // mute ported for Omega 2.1 and later versions
//#define    PORT1.5VERSION     0x0013L;  // added Omega 13 Mhz stop/start for deep sleep
//#define    PORT1.5VERSION     0x0014L;  // Added audio control functions and TXPWR management for Omega
//#define    PORT1.5VERSION     0x0015L;  // Porting of L1 and drivers for G1, SAMSON
                                          // Integration of PTOOL S/W
//#define    PORT1.5VERSION     0x0016L   // SR correction + prepare for DSP background management
                                          // + DSP patch 0x06E0
                                          // TI_232, TI_241, TI_247, TI_250
//#define    PORT1.5VERSION     0x0017L;  // Porting for SLX RF on EVA4 board
//#define    PORT1.5VERSION     0x0018L   // SR rejection of OOV during Update TI278
                                          // + Melody Abort rework TI285
/////////////////////////////////////
// PORT1.5 version is no more used //
/////////////////////////////////////

// added for new naming conventions
//#define PROGRAM_RELEASE_VERSION       0x2112
//#define PROGRAM_RELEASE_VERSION       0x2118	// release 1446 is for TCS2.1.1.8
//#define PROGRAM_RELEASE_VERSION       0x2119	// release 1448 is for TCS2.1.1.9
//#define PROGRAM_RELEASE_VERSION       0x211A	// release 1450 is for TCS2.1.1.10
//#define PROGRAM_RELEASE_VERSION       0x211C	// release 1451 is for TCS2.1.1.12 -> switching to dynamic download
#define PROGRAM_RELEASE_VERSION         0x211E	// release 1452 is for TCS2.1.1.14 

/* Internal release numbering */
//#define INTERNAL_VERSION	      0x1 // First version on ClearCase
//#define INTERNAL_VERSION	      0x2 // Second subversion on mainline
//#define INTERNAL_VERSION	      0x3 // Second subversion on mainline
#define INTERNAL_VERSION	      0x0 // Official release

/* Official external release numbering */
//#define OFFICIAL_VERSION	      0x1432
//#define OFFICIAL_VERSION	      0x1433
//#define OFFICIAL_VERSION	      0x1434
//#define OFFICIAL_VERSION	      0x1435
//#define OFFICIAL_VERSION	      0x1436
//#define OFFICIAL_VERSION	      0x1438
//#define OFFICIAL_VERSION	      0x1439 
//#define OFFICIAL_VERSION	      0x1440
//#define OFFICIAL_VERSION	      0x1441
//#define OFFICIAL_VERSION	      0x1442
//#define OFFICIAL_VERSION	      0x1444
//#define OFFICIAL_VERSION	      0x1445
//#define OFFICIAL_VERSION	      0x1446
//#define OFFICIAL_VERSION	      0x1447
//#define OFFICIAL_VERSION	      0x1448
//#define OFFICIAL_VERSION	      0x1449
//#define OFFICIAL_VERSION	      0x1450
//#define OFFICIAL_VERSION	      0x1451
//#define OFFICIAL_VERSION	      0x1453
#define OFFICIAL_VERSION	      0x1454
