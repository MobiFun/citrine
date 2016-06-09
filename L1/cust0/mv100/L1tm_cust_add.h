#ifndef _DEF_BMI_L1TMCUSTADD_H_
#define _DEF_BMI_L1TMCUSTADD_H_
/************* Revision Controle System Header *************
 *                  GSM Layer 1 software
 * L1TM_CUST_ADD.h
 *
 *        Filename %M%
 *        Version  %I%
 *        Date     %G%
 *
 * $History: Mmidict.c

 	01/08/02 			huangxl creat.	
 * $end
 ************* Revision Controle System Header *************/

#define EQ  ==
#define NEQ !=
#define AND &&
#define OR  ||
#define XOR(A,B) ((!(A) AND (B)) OR ((A) AND !(B)))

#define GPIO_OUT 0xFFFE4802 
#define ALLOC_MEMORY mfwAlloc
#define FREE_MEMORY  mfwFree


#ifndef FALSE
  #define FALSE 0
#endif

#ifndef TRUE
  #define TRUE 1
#endif

#ifndef NULL
  #define NULL 0
#endif

 enum CUSTOM_INDEX
{
	OFF				= 0,
	ON				= 1
};

enum CMD 
{
	READ_SIM_PHB_CMD			= 17,
	WRITE_SIM_PHB_CMD			= 18,
	READ_SIM_SMS_CMD			= 19,
	READ_NEW_SMS_CMD			= 21,
	SEND_SMS_CMD					=22	,
	SAVE_SEND_SMS					=23,
	WRITE_NVM_PHB_CMD			= 24
		
};

enum PC_CMD_RET
{
	CMD_RET_OK						=0,
	CMD_COMPLETE_OK					=1,
	SIM_NOT_REDAY					=-1,
	NO_SIM_PHB						=-2,
	FWRITE_FAIL						=-3,
	CMD_BUSY						=-4,
	FREAD_FAIL						=-5,
	WRITE_SIM_PHB_FAIL				=-6,
	READ_SIM_PHB_FAIL				=-7,
	ALLOC_MEM_FAIL				=-8,
	NO_SIM_SMS					=-9
};

#define MAXNAMELEN 21
#define MAXNUMLEN 21
#define MAXOFFNUMLEN 21
#define MAXHOMNUMLEN 21
#define MAXEMAILLEN 25
#define MAXGROUPLEN 10
#define MAXPRILEN 8
#define	MAXSIMPHBNUM	20

typedef struct TSPhbData
{
	char Name[21];
	char Number[21];
}SPhbData;

typedef struct TPhbData
{
	SPhbData	phbdata[20];
}PhbData;

typedef struct TPCCMDSTATUS
{
	unsigned char	isPcCmdRun;//when pc cmd is complete,it is 1;when running is 0,default is 1
	unsigned char	SimPhbNum;//how many phb num in sim
	unsigned char	NvmPhbNum;//how many phb num in nvm
	unsigned char	nSimSmsFile;//how many sms file created
	unsigned char	isdelsmsok;//
	unsigned char	nNewsms;//how many new sms 
	unsigned char	iscmdreterr;//
	unsigned char	nsimphbmax;//how many sms file created
	unsigned char	nsimsmsmax;//how many sms file created
	unsigned char	isPcConect;//conect:1	no dataline in:0
}PCCMDSTATUS;

typedef struct	TSMS_MEM_INFO
{
  unsigned char mem;
  unsigned char used;
  unsigned char total;
  unsigned char dummy;
} SMS_MEM_INFO;

#define MAX_LEN             21

typedef struct
{
	char		date[18];
        char 	Number[41];
        char	info[502];
  	char    sc_addr[MAX_LEN];     /* service centre address     */
        char	type;
        char	ton;
        char	npi;
        char	index;
}TPhoneSmsItem;

extern int write_imei(unsigned char *imei);
extern int write_SerialNumber(unsigned char *value);
extern int read_SerialNumber(char *table);
extern unsigned short checksim();
extern unsigned short  checknetavail();
extern unsigned char ctrlcd(unsigned short value);
extern unsigned char ctrled(unsigned short value);
extern unsigned char ctrbacklight(unsigned short value);
extern unsigned char ctrvibrator(unsigned short value);
extern unsigned char ctrbuzzer(unsigned short value);
extern int key_simulation(unsigned char size,unsigned char *keycode);
extern int read_rtc(char *table);

extern unsigned short  get_sim_status(void);
extern unsigned short  get_net_status(void);

extern void kbd_test (unsigned char key);

extern void StartVibrator(void);
extern void StopVibrator(void);
extern void StartTone(void);
extern void StopTone(void);


extern void LED_ON(void);
extern void LED_OFF(void);
extern void Backlight_ON(void);
extern void Backlight_OFF(void);


extern signed char flash_write();

extern int read_sim_phb(void);
extern int write_sim_phb(void);
extern unsigned char IsPcCmdExcuteEnd(char *value); 
extern int write_sim_phb_cb(void);
extern void tell_simphb_num(unsigned short value);
extern int read_sim_sms(void);
extern void Get_sms_info(const char *info,void *adr, void * t,void *scaddr,int errStatus);

extern int del_sim_sms(unsigned short value);
extern int read_new_sms(void);
extern int change_sim_sms(unsigned short  value);
extern int send_sms(void);
extern int send_sms_cb(void);
extern void tell_nvmphb_num(unsigned short  value);


extern PCCMDSTATUS	PcCmdStatus;
extern unsigned char command;


#endif
