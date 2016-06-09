/*
 * SIM.H
 *
 * Pole Star SIM
 *
 * Target : ARM
 *
 * Copyright (c) Texas Instruments 1995-1997
 *
 */

/*
 * Device addresses - GCS000 (Gemini / Polestar)
 *                    HER207 (Hercules)
 */ 

#include "../include/config.h"
#include "../include/sys_types.h"
#include "../nucleus/nucleus.h"

/* Flags activation section */
// #define SIM_RETRY		 /*	by default : NOT ACTIVE */
//#define SIM_DEBUG_TRACE	 	/*	by default : NOT ACTIVE */
//#define SIM_UWORD16_MASK 0x00ff  //when using SIM entity not maped to length on 16 bits
#define SIM_UWORD16_MASK 0xffff  //when using SIM entity maped to length on 16 bits
//#define SIM_APDU_TEST
//#define SIM_SAT_REFRESH_TEST

#define SIM_CMD         (MEM_SIM + 0x00)
#define SIM_STAT        (MEM_SIM + 0x02)
#define SIM_CONF1       (MEM_SIM + 0x04)
#define SIM_CONF2       (MEM_SIM + 0x06)
#define SIM_IT          (MEM_SIM + 0x08)
#define SIM_DRX         (MEM_SIM + 0x0A)
#define SIM_DTX         (MEM_SIM + 0x0C)
#define SIM_MASK        (MEM_SIM + 0x0E)



/*
 * Bit definitions 
 */ 
// control regidter
#define SIM_CMD_CRST          0x0001
#define SIM_CMD_SWRST         0x0002
#define SIM_CMD_STOP          0x0004
#define SIM_CMD_START         0x0008
#define SIM_CMD_CLKEN         0x0010	

// status register
#define SIM_STAT_CD           0x0001   // card present
#define SIM_STAT_TXPAR        0x0002   // transmit parity status
#define SIM_STAT_FFULL        0x0004   // fifo full
#define SIM_STAT_FEMPTY       0x0008   // fifo empty

// configuration register
#define SIM_CONF1_CHKPAR      0x0001   // enable receipt check parity
#define SIM_CONF1_CONV        0x0002   // coding convention
#define SIM_CONF1_TXRX        0x0004   // SIO line direction
#define SIM_CONF1_SCLKEN      0x0008   // enable SIM clock
#define SIM_CONF1_RSVD        0x0010   // reserved
#define SIM_CONF1_SCLKDIV	  0x0020   // SIM clock frquency
#define SIM_CONF1_SCLKLEV	  0x0040   // SIM clock idle level
#define SIM_CONF1_ETU	      0x0080   // ETU period
#define SIM_CONF1_BYPASS      0x0100   // bypass hardware timers
#define SIM_CONF1_SVCCLEV     0x0200
#define SIM_CONF1_SRSTLEV     0x0400
#define SIM_CONF1_SIOLOW      0x8000   //force SIO to low level	 

// interrupt status register
#define SIM_IT_NATR           0x0001   // No answer to reset
#define SIM_IT_WT             0x0002
#define SIM_IT_ITOV           0x0004   
#define SIM_IT_ITTX           0x0008   // Transmit
#define SIM_IT_ITRX           0x0010   // Receipt

#if ((CHIPSET == 4) || (CHIPSET == 5) || (CHIPSET == 6) || (CHIPSET == 7) || (CHIPSET == 8) || (CHIPSET == 9) || (CHIPSET == 10) || (CHIPSET == 11) || (CHIPSET == 12))
  #define SIM_IT_CD             0x0001   // Card insertion/extraction
#else
  #define SIM_IT_CD             0x0020   // Card insertion/extraction
#endif

// interrupt mask register
#define SIM_MASK_NATR         0x0001   // No answer to reset
#define SIM_MASK_WT           0x0002
#define SIM_MASK_OV           0x0004
#define SIM_MASK_TX           0x0008   // Transmit
#define SIM_MASK_RX           0x0010   // Receipt
#define SIM_MASK_CD           0x0020   // Card insertion/extraction	

// receveid byte register
#define SIM_DRX_STATRXPAR     0x0100   // received byte parity status

// SIM return code OK
#define SIM_OK                  0

// SIM return error codes
#define SIM_ERR_NOCARD          1
#define SIM_ERR_NOINT           2
#define SIM_ERR_NATR            3
#define SIM_ERR_READ            4
#define SIM_ERR_XMIT            5
#define SIM_ERR_OVF             6
#define SIM_ERR_LEN             7
#define SIM_ERR_CARDREJECT      8
#define SIM_ERR_WAIT            9
#define SIM_ERR_ABNORMAL_CASE1 10
#define SIM_ERR_ABNORMAL_CASE2 11
#define SIM_ERR_BUFF_OVERFL    12

// begin of JYT modifications
#define SIM_ERR_HARDWARE_FAIL  13
// end of JYT modifications
#define SIM_ERR_RETRY_FAILURE  14

#define SIM_SLEEP_NONE         0	// No SIM available 
#define SIM_SLEEP_DESACT       1	// The Driver is NOT currently in sleep mode (clock is off)
#define SIM_SLEEP_ACT          2	// The Driver is currently in sleep mode (clock is on)
#define SIM_SLEEP_NOT_ALLOWED  3	// The Driver cannot stop the clock :
									// The card don't want or the interface is not able
									// to do it.
#define SIM_SLEEP_WAITING_TIME 500 //represent 2.3s of period before entering in sleep mode

#define SIM_CLK_STOP_MASK		0x0D	// Clock Stop mask defined by ETSI 11.11 
#define SIM_CLK_STOP_NOT_ALLWD	0x00	// see ETSI 11.11 : Clock Stop never allowed
#define SIM_CLK_STOP_ALLWD		0x01	// see ETSI 11.11 : No prefered level
#define SIM_CLK_STOP_HIGH		0x04	// see ETSI 11.11 : High level only
#define SIM_CLK_STOP_LOW		0x08	// see ETSI 11.11 : Low level only

#if(ANALOG == 1)
  //OMEGA specific definitions
  #define MODE5V_OMEGA         0x06 // used in SIM_SwitchVolt
  #define MODE_INIT_OMEGA_3V   0x05 // used in SIM_StartVolt
  #define MODE_INIT_OMEGA_5V   0x07 // unused !!!!
  #define MODE3V_OMEGA         0x01 // unused !!!!
  #define MODE_DIS_SIMLDOEN    0xDF // used in SIM_PowerOff
  #define MODE_DIS_SIMEN       0xFD // used in SIM_PowerOff
  #define MODE_ENA_SIMLDOEN    0x20 // used in SIM_ManualStart
  #define MODE_ENA_SIMEN       0x02 // used in SIM_ManualStart
#elif(ANALOG == 2)
   //IOTA specific definitions
   #define MODE1_8V_IOTA        0x00 
   #define MODE_INIT_IOTA_3V    0x03
   #define MODE_INIT_IOTA_1_8V  0x02 
   #define MODE3V_IOTA          0x01
   #define MODE_DIS_SIMLDOEN    0xFC // SIMSEL + Regulator RSIMEN
   #define MODE_DIS_SIMEN       0xF7
   #define MODE_ENA_SIMLDOEN    0x03 // SIMSEL + Regulator RSIMEN
   #define MODE_ENA_SIMEN       0x08
#elif(ANALOG == 3)
   //SYREN specific definitions
   #define MODE1_8V_SYREN        0x00 
   #define MODE_INIT_SYREN_3V    0x03
   #define MODE_INIT_SYREN_1_8V  0x02 
   #define MODE3V_SYREN          0x01
   #define MODE_DIS_SIMLDOEN     0x1FC // SIMSEL + Regulator RSIMEN
   #define MODE_DIS_SIMEN        0x1F7
   #define MODE_ENA_SIMLDOEN     0x03 // SIMSEL + Regulator RSIMEN
   #define MODE_ENA_SIMEN        0x08
#endif

// define type of interface if not defined
// 5V only ME         SIM_TYPE = 0
// 3V technology ME   SIM_TYPE = 1
// 3V only ME         SIM_TYPE = 2
// 1.8V technology ME SIM_TYPE = 3 // JYT, 29/01/02, from new specs IOTA
// 1.8V Only ME       SIM_TYPE = 4 // JYT, 29/01/02, from new specs IOTA

#define SIM_TYPE_5V    0
#define SIM_TYPE_3_5V  1
#define SIM_TYPE_3V    2
#define SIM_TYPE_1_8_3V 3
#define SIM_TYPE_1_8V   4

//default configuration
#ifndef SIM_TYPE
#if((ANALOG == 2) || (ANALOG == 3))
// Until now (20/03/2003), it is impossible to test IOTA or SYREN with 1.8V Sim Card,
// so SIM drv is configured in 3V only with IOTA.and SYREN
// When 1.8V Sim Card will be delivered and tested on IOTA and SYREN, then Sim driver will pass 
// to : #define SIM_TYPE       SIM_TYPE_1_8_3V   
#define SIM_TYPE       SIM_TYPE_1_8_3V // MODIFY BY JENNIFER SIM_TYPE_3V   
#else
#define SIM_TYPE       SIM_TYPE_3_5V   
#endif
#endif

// begin of modifications of JYT

#if((ANALOG == 2) || (ANALOG == 3))
#define SIM_MASK_INFO_VOLT    0x70  
#else
#define SIM_MASK_INFO_VOLT    0x10
#endif

#define SIM_1_8V              0x30
#define SIM_3V                0x10  
#define SIM_5V                0x00

// end of modifications of JYT

// Max size of Answer to Reset (GSM11.11 5.7.1)
#define MAX_ATR_SIZE          33

// GSM Instruction Class (GSM 11.11 SIM spec)
#define GSM_CLASS  0xA0

// SIM Instruction Codes
#define SIM_SELECT              0xA4    
#define SIM_STATUS              0xF2    
#define SIM_READ_BINARY         0xB0    
#define SIM_UPDATE_BINARY       0xD6    
#define SIM_READ_RECORD         0xB2    
#define SIM_UPDATE_RECORD       0xDC    
#define SIM_SEEK                0xA2    
#define SIM_INCREASE            0x32    
#define SIM_VERIFY_CHV          0x20    
#define SIM_CHANGE_CHV          0x24    
#define SIM_DISABLE_CHV         0x26    
#define SIM_ENABLE_CHV          0x28    
#define SIM_UNBLOCK_CHV         0x2C    
#define SIM_INVALIDATE          0x04    
#define SIM_REHABILITATE        0x44    
#define SIM_RUN_GSM_ALGO        0x88    
#define SIM_GET_RESPONSE        0xC0 
#define SIM_TERMINAL_PROFILE   	0x10
#define SIM_FETCH   			0x12
#define SIM_TERMINAL_RESPONSE   0x14
#define SIM_ENVELOPE   			0xC2



// SIM file identifiers
#define MF                 0x3F00
#define EF_ICCID           0x2FE2
#define DF_GSM             0x7F20
#define DF_DCS1800         0x7F21
#define EF_LP              0x6F05
#define EF_IMSI            0x6F07
#define EF_KC              0x6F20
#define EF_PLMNSEL         0x6F30
#define EF_HPLMN           0x6F31
#define EF_ACMAX           0x6F37
#define EF_SST             0x6F38
#define EF_ACM             0x6F39
#define EF_PUCT            0x6F41
#define EF_CBMI            0x6F45
#define EF_BCCH            0x6F74
#define EF_ACC             0x6F78
#define EF_FPLMN           0x6F7B
#define EF_LOCI            0x6F7E
#define EF_AD              0x6FAD
#define EF_PHASE           0x6FAE
#define DF_TELECOM         0x7F10
#define EF_ADN             0x6F3A
#define EF_FDN             0x6F3B
#define EF_SMS             0x6F3C
#define EF_CCP             0x6F3D
#define EF_MSISDN          0x6F40
#define EF_SMSP            0x6F42
#define EF_SMSS            0x6F43
#define EF_LND             0x6F44
#define EF_EXT1            0x6F4A
#define EF_EXT2            0x6F4B
#define EF_ECC             0x6FB7


#define MASK_INS           0xFE
#define MASK_CMD           0x11
#define MASK_RST           0x10  


// Buffer sizes
#define RSIMBUFSIZE  270
#define RSIZESW1SW2  2 
#define XSIMBUFSIZE  270




// Structures
typedef struct
{
   volatile unsigned short cmd;
   volatile unsigned short stat;
   volatile unsigned short conf1;
   volatile unsigned short conf2;
   volatile unsigned short it;
   volatile unsigned short rx;
   volatile unsigned short tx;
   volatile unsigned short maskit;
#if ((CHIPSET == 4) || (CHIPSET == 5) || (CHIPSET == 6) || (CHIPSET == 7) || (CHIPSET == 8) || (CHIPSET == 9) || (CHIPSET == 10) || (CHIPSET == 11) || (CHIPSET == 12))
   volatile unsigned short it_cd;
#endif
} SIM_CONTROLLER;  


typedef struct
{
    SYS_UWORD8    Inverse;
    SYS_UWORD8    AtrSize;
    SYS_UWORD8    AtrData[MAX_ATR_SIZE];
} SIM_CARD;


typedef struct
{
    SIM_CONTROLLER      *c;
    SYS_UWORD8          *xIn;            // xmit input pointer
    SYS_UWORD8          *xOut;           // xmit output pointer
    unsigned            errorSIM;          // code return in case of error detectd
    unsigned short      conf1; // image of the configuration register - avoids read/mod/write cycles	
    volatile unsigned short txParityErr;
    unsigned short		rxParityErr;	// if 0 no parity error on receipt, 1 if...
    SYS_UWORD8          Freq_Algo;              //use to determine which sim clk freq to choose for running GSM algo
    SYS_UWORD8          PTS_Try;                //use to calculate how many PTS try were already done
    SYS_UWORD8          FileC;                  //value of File Characteristic
    SYS_UWORD16         etu9600;
    SYS_UWORD16         etu400;
    SYS_UWORD16         startclock;             //744 clock cycle translated in ETU
    SYS_UWORD16         stopclock;              //1860 clock cycle translated in ETU
    SYS_UWORD8          moderx;                  //inform that we are in receive mode
                                        // 0 : mode of normal reception without procedure
                                        // 1 : mode of wait for acknowledge during reception of char
                                        // 2 : mode of reception of data by bloc
                                        // 3 : mode of reception of data char by char (proc char)
                                        // 4 : mode of reception of data char by char (data)                                       
                                        // 5 : mode of reception of procedure char SW1/SW2
                                        // 6 : mode of wait for acknowledge char after transmission of char
    SYS_UWORD16       expected_data;          //number of expected char in receive mode proc char
    SYS_UWORD8        ack;                    //acknowledge char
    SYS_UWORD8        null_received;          //indicates if a NULL char was received
    SYS_UWORD8        hw_mask;		          //mask used because of pole112 hw prb

    SYS_UWORD8        rbuf[RSIMBUFSIZE];
    SYS_UWORD8        rx_index;                   // receive index on rbuf buffer

    SYS_UWORD8        xbuf[XSIMBUFSIZE];
    SYS_UWORD8        rSW12[RSIZESW1SW2];          //buffer to store SW1 and SW2
    SYS_UWORD8        SWcount;                     //static counter
    void (*InsertFunc)(SIM_CARD *);
    void (*RemoveFunc)(void);
	SYS_UWORD16		  apdu_ans_length;
}
SIM_PORT;



void        SIM_IntHandler(void);
#if ((CHIPSET == 4) || (CHIPSET == 5) || (CHIPSET == 6) || (CHIPSET == 7) || (CHIPSET == 8) || (CHIPSET == 9) || (CHIPSET == 10) || (CHIPSET == 11) || (CHIPSET == 12))
  void SIM_CD_IntHandler(void);
#endif





/*
 * Prototypes
 */ 
// obsolete function
void SIM_Init(void (Insert(SIM_CARD *cP)), void (Remove(void)));

// initialization 
void        SIM_Initialize(void);
SYS_UWORD16 SIM_Register(void (Insert(SIM_CARD *cP)), void (Remove(void)));
SYS_UWORD16 SIM_Reset(SIM_CARD *cP);
SYS_UWORD16 SIM_Restart(SIM_CARD *cP);

// file commands
SYS_UWORD16 SIM_Select(SYS_UWORD16 id, SYS_UWORD8 *dat, SYS_UWORD16 *size);
SYS_UWORD16 SIM_Status(SYS_UWORD8 *dat, SYS_UWORD16 *size);
SYS_UWORD16 SIM_ReadBinary(SYS_UWORD8 *dat, SYS_UWORD16 offset, SYS_UWORD16 len, SYS_UWORD16 *size);
SYS_UWORD16 SIM_UpdateBinary(SYS_UWORD8 *result, SYS_UWORD8 *dat, SYS_UWORD16 offset, SYS_UWORD16 len, SYS_UWORD16 *size);
SYS_UWORD16 SIM_ReadRecord(SYS_UWORD8 *dat, SYS_UWORD8 mode, SYS_UWORD8 recNum, SYS_UWORD16 len, SYS_UWORD16 *size);
SYS_UWORD16 SIM_UpdateRecord(SYS_UWORD8 *result, SYS_UWORD8 *dat, SYS_UWORD8 mode, SYS_UWORD8 recNum, SYS_UWORD16 len, SYS_UWORD16 *size);
SYS_UWORD16 SIM_Seek(SYS_UWORD8 *result, SYS_UWORD8 *dat, SYS_UWORD8 mode, SYS_UWORD16 len, SYS_UWORD16 *size);
SYS_UWORD16 SIM_Increase(SYS_UWORD8 *result, SYS_UWORD8 *dat, SYS_UWORD16 *size);

// Authentication
SYS_UWORD16 SIM_VerifyCHV(SYS_UWORD8 *result, SYS_UWORD8 *chv, SYS_UWORD8 chvType, SYS_UWORD16 *size);
SYS_UWORD16 SIM_ChangeCHV(SYS_UWORD8 *result,SYS_UWORD8 *oldChv, SYS_UWORD8 *newChv, SYS_UWORD8 chvType, SYS_UWORD16 *size);
SYS_UWORD16 SIM_DisableCHV(SYS_UWORD8 *result, SYS_UWORD8 *dat, SYS_UWORD16 *size);
SYS_UWORD16 SIM_EnableCHV(SYS_UWORD8 *result, SYS_UWORD8 *dat, SYS_UWORD16 *size);
SYS_UWORD16 SIM_UnblockCHV(SYS_UWORD8 *result, SYS_UWORD8 *unblockChv, SYS_UWORD8 *newChv, SYS_UWORD8 chvType, SYS_UWORD16 *size);

// managing
SYS_UWORD16 SIM_Invalidate(SYS_UWORD8 *rP, SYS_UWORD16 *size);
SYS_UWORD16 SIM_Rehabilitate(SYS_UWORD8 *rP, SYS_UWORD16 *size);
SYS_UWORD16 SIM_RunGSMAlgo(SYS_UWORD8 *result, SYS_UWORD8 *rand, SYS_UWORD16 *size);
SYS_UWORD16 SIM_GetResponse(SYS_UWORD8 *dat, SYS_UWORD16 len, SYS_UWORD16 *size);

// STK 
SYS_UWORD16 SIM_TerminalProfile(SYS_UWORD8 *result, SYS_UWORD8 *dat, SYS_UWORD16 len, SYS_UWORD16 *rcvSize);
SYS_UWORD16 SIM_Fetch(SYS_UWORD8 *result, SYS_UWORD16 len, SYS_UWORD16 *rcvSize);
SYS_UWORD16 SIM_TerminalResponse(SYS_UWORD8 *result, SYS_UWORD8 *dat, SYS_UWORD16 len, SYS_UWORD16 *rcvSize);
SYS_UWORD16 SIM_Envelope(SYS_UWORD8 *result, SYS_UWORD8 *dat, SYS_UWORD16 len, SYS_UWORD16 *rcvSize);

// power off
void         SIM_PowerOff(void);

// WIM
SYS_UWORD16 SIM_XchTPDU(SYS_UWORD8 *dat, SYS_UWORD16 trxLen, SYS_UWORD8 *result,
						SYS_UWORD16 rcvLen, SYS_UWORD16 *rcvSize);

void SIM_lock_cr17689(void);



/*
 * Internal Prototypes
 */ 
void        SIM_WriteBuffer(SIM_PORT *p, SYS_UWORD16 offset, SYS_UWORD16 n);
SYS_UWORD16 SIM_Result(SIM_PORT *p, SYS_UWORD8 *rP, SYS_UWORD16 *lenP, SYS_UWORD8 offset);
SYS_UWORD16 SIM_Command(SIM_PORT *p, SYS_UWORD16 n, SYS_UWORD8 *rP, SYS_UWORD16 *lP);
SYS_UWORD16 SIM_Command_Base(SIM_PORT *p, SYS_UWORD16 n, SYS_UWORD8 *dP, SYS_UWORD16 *lP);
SYS_UWORD16 SIM_Dummy(void);
void        SIM_InitLog(void);

SYS_UWORD16 SIM_TxParityErrors();
SYS_UWORD16  SIM_WaitReception(SIM_PORT *p);
void         SIM_Interpret_FileCharacteristics(SIM_PORT *p);
SYS_UWORD16  SIM_PTSprocedure(SIM_CARD *cP, SIM_PORT *p);
void         SIM_WARMReset (SIM_PORT *p);
void	     SIM_SleepMode_In(SYS_UWORD32 param);
void	     SIM_SleepMode_Out(SIM_PORT *p);
SYS_UWORD8 	 SIM_GetFileCharacteristics(SIM_PORT *p);
SYS_UWORD16  SIM_ATRdynamictreatement (SIM_PORT *p, SIM_CARD *cP);
SYS_UWORD16  SIM_Waitforchars (SIM_PORT *p, SYS_UWORD16 max_wait);
void         SIM_Calcetu (SIM_PORT *p);
SYS_UWORD8   SIM_Translate_atr_char (SYS_UWORD8 input, SIM_CARD *cP);


SYS_UWORD8   SIM_StartVolt (SYS_UWORD8 ResetFlag);
SYS_UWORD8   SIM_SwitchVolt (SYS_UWORD8 ResetFlag);

SYS_UWORD16  SIM_ManualStart (SIM_PORT *p);
SYS_UWORD8   SIM_Memcpy(SYS_UWORD8 *Buff_target, SYS_UWORD8 Buff_source[], SYS_UWORD16 len);
SYS_BOOL     SIM_SleepStatus(void);
SYS_UWORD16  SIM_Reset_Restart_Internal(SIM_CARD *cP, SYS_UWORD8 ResetFlag);

/*
 * Global variables
 */ 
#ifdef SIM_C
#define SI_GLOBAL 
#else
#define SI_GLOBAL extern
#endif


SI_GLOBAL SIM_PORT   Sim[1]; 
SI_GLOBAL NU_TIMER   SIM_timer;  
SI_GLOBAL STATUS     status_os_sim;
SI_GLOBAL SYS_UWORD8 SIM_sleep_status;
