/********************************************************************************
 * Enhanced TestMode (ETM)
 * @file	tmcore.c 
 *
 * @author	Kim T. Peteren (ktp@ti.com)
 * @version 0.1
 *

 *
 * History:
 *
 * 	Date       	Modification
 *  ------------------------------------
 *  16/06/2003	Creation
 *  02/07/2003  Removed l1_config.TestMode check from CODEC Write
 *  17/03/2004  Updated etm_version
 *  30/03/2004  Updated etm_dieID_read() func. regarding get die id for 16 bits
 *              instead of 8 bits.
 *
 * (C) Copyright 2003 by Texas Instruments Incorporated, All Rights Reserved
 *********************************************************************************/

#include "../../riviera/rv/rv_defined_swe.h"
#include "../../riviera/rv/rv_general.h"

#include "etm.h"
#include "etm_config.h"
#include "etm_api.h"
#include "etm_trace.h"
#include "etm_version.h"

#include "../../bsp/abb+spi/abb.h"
#include "../../bsp/abb+spi/spi_drv.h"

extern void tr_etm_init(unsigned int mask);

// Version of the ETM CORE module
// See the file etm_version.h

/******************************************************************************
 * DIE ID settings 
 *****************************************************************************/

/* DIE ID register */
#if ((CHIPSET == 8) || (CHIPSET == 10) || (CHIPSET == 11))      //For D-Sample: $CHIPSET  =  8 (=10 for D-sample AMR, 11 for GSMLITE).
#define DIE_ID_REG    (MEM_DEV_ID0 | 0xF010) //+ 0xFFFEF010 for Calypso
#else
#if (CHIPSET == 12)                         //For E-Sample: $CHIPSET   =  12.
#define DIE_ID_REG    (0xFFFE0000 | 0xF004)	//+ 0xFFFEF004 for Calypso Plus
#endif
#endif

/* DIE ID SIZE is 4 words (16 bits)long */
#define DIE_ID_SIZE        4


/******************************************************************************
 * Internal prototypes 
 *****************************************************************************/

T_ETM_PKT *etm_core_setup(uint8 fid);
int etm_core(uint8 *buf, int size);

/******************************************************************************
 * Register the Core Module to the ETM database 
 *****************************************************************************/

int etm_core_init(void)
{
    int result; 

    result = etm_register("CORE", ETM_CORE, 0, 0, etm_core);
    return result;    
}


/******************************************************************************
 * Memory read/write Functions
 *****************************************************************************/

// Describe the payload of the mem protocol !!!!!!
// |--type(1)-|--partnum(1)-|--addr(4)-|--data(?)-|

int etm_mem(T_ETM_PKT *pkt, uint8 *buf)
{
   int num, unitsize, error;
   uint8 type, param;
   uint8  *mem8;
   uint16 *mem16;
   uint32 *mem32;
   uint32 addr, tmp;

   param = unitsize = *buf & 0x3;
   if (unitsize == 0)
       unitsize = 4;

   type = *buf & 0x10;
   buf++;

   num = *buf++;
   addr = etm_get32(buf);
   buf += 4;

   tr_etm(TgTrCore, "ETM CORE: _mem: type(0x%x) addr(0x%x) partnum(%d)", type, addr, num);

   // Put 'parameter' in return packet
   if ((error = etm_pkt_put8(pkt, param)) < 0) {
       return error;
   }
      
   switch (type) {
   case 0: // READ(0x00)
       switch (unitsize) {
       case 1:
           mem8 = (uint8 *) addr;
           while (num--) {
               if ((error = etm_pkt_put8(pkt, *mem8++)) < 0)
                   break;
           }
           break;
       case 2:
           mem16 = (uint16 *) addr;
           while (num--) {
               if ((error = etm_pkt_put16(pkt, *mem16++)) < 0)
                   break;
           }
           break;
       case 4:
           mem32 = (uint32 *) addr;
           while (num--) {
               if ((error = etm_pkt_put32(pkt, *mem32++)) < 0)
                   break;
           }
           break;
       } 
       break;

   case 16: // WRITE(0x10)  
       switch (unitsize) {
       case 1:
           mem8 = (uint8 *) addr;
           while (num--) {
               *mem8++ = etm_get8(buf);
               buf += 1;
           }
           break;
       case 2:
           mem16 = (uint16 *) addr;
           while (num--) {
               *mem16++ = tmp = etm_get16(buf); 
               buf += 2;
           }
           break;
       case 4:
           mem32 = (uint32 *) addr;
           while (num--) {
               *mem32++ = etm_get32(buf); 
               buf += 4;
           }
           break;
       }
       break;
   default:
       return ETM_NOSYS;
   }
   
   if (error < 0)
       return error;
   
    return ETM_OK;
}


/******************************************************************************
 * CODEC Functions
 *****************************************************************************/

// ETM sends both page value and register address in one byte.
// Bit field is:  PPPR RRRR
// where P = page bit, R = register address bits and X = don't care bits.  

int etm_codec_write(T_ETM_PKT *pkt, uint8 *buf)
{
    uint16  page, reg, data;
    int result, reg_data;

    reg_data = *buf++;
    if ((result = etm_pkt_put8(pkt, reg_data)) < 0)
        return result;

    page = (reg_data  >> 5) & 0x3;
    reg  =  reg_data        & 0x1F;
    data = etm_get16(buf);
   
    tr_etm(TgTrCore, "ETM CORE: _codec_write: page(%d) reg(%d) data(0x%x)",
           page, reg, (data & 0x3ff));

    if (page > 7 && reg > 32)
        return ETM_INVAL;
    else {
        // The function below expects a 1 for page 0 and a 2 for page 1.
        // The register address value is left-shifted by 1 since LSB is read/write command bit.
        // The value is written in the 10 MSB's of register.
        ABB_Write_Register_on_page(page + 1, reg << 1, (data & 0x3ff));
    }    

    return ETM_OK;
}


int etm_codec_read(T_ETM_PKT *pkt, uint8 *buf)
{
    uint16 value;
    uint16 page, reg;
    int result, reg_data;

    reg_data = *buf;
    if ((result = etm_pkt_put8(pkt, reg_data)) < 0)
        return result;

    page = (reg_data  >> 5) & 0x3;
    reg  =  reg_data        & 0x1F;
    
    if (page > 7 && reg > 32)
        return ETM_INVAL;

    // The function below expects a 1 for page 0 and a 2 for page 1.
    // The register value is left-shifted by 1 since LSB is read/write command bit.
    value = ABB_Read_Register_on_page(page + 1, (reg << 1));
    
    tr_etm(TgTrCore, "ETM CORE: _codec_read: page(%d) reg(%d) value(0x%x)", page, reg, value);
    
    result = etm_pkt_put16(pkt, value);
    return result;
}


/******************************************************************************
 * Echo and Reset Functions
 *****************************************************************************/

//structur of data dl: |delay|recvsize|num| = 3x2 bytes
int etm_echo(T_ETM_PKT *pkt, uint8 *data)
{
    int delay, sendsize, i;

    tr_etm(TgTrCore, "etm_echo:");

    delay = etm_get16(data);
    data += 2;

    sendsize = etm_get16(data);
    if (sendsize > 240)
        return ETM_INVAL;

    tr_etm(TgTrCore, "ETM CORE: _echo: delay(%d) sendsize(%d)", 
           delay, sendsize);

    if (delay > 0) {
        rvf_delay((delay + 32) * 14 / 64);
    }

    for (i = 0; i < sendsize; i++) {
        pkt->data[i+1] = i;        // data[0] = fid
    }

    pkt->size = sendsize + 1;

    return ETM_OK;
}


int etm_reset(void)
{
// The reset code is taken form Fluid->cmd.c
    int i;

    tr_etm(TgTrCore, "ETM CORE: _reset: Target is Reset");
    
    // Setup watchdog timer and let it timeout to make a reset
    *(volatile uint16*) 0xfffff804 = 0xFFFF;  // Timer to watchdog
    *(volatile uint16*) 0xfffff800 = 0x0080;  // Start timer
		// Apparently works it only if we read this register?
    i = *(volatile uint16*) 0xfffff802;
    *(volatile uint16*) 0xfffff802 = 0x0001;  // Load timer

    return ETM_OK;
}


/******************************************************************************
 * Set Test Controls
 *****************************************************************************/

int etm_debug(T_ETM_PKT *pkt, uint8 *buf)
{
   int type, error, data;
   
   static char *p;

   type = *buf & 0x0F;   
   buf++; 

   data = etm_get32(buf);

   tr_etm(TgTrCore, "ETM CORE: _debug: type(%d) data(0x%x)", type, data);

   switch (type) {
   case 0: // (0x00) Allocate Test Buffer
       if ((p = etm_malloc(data)) == NULL)
           error = ETM_NOMEM;
       error = etm_pkt_put32(pkt, (int) p); 
       break;
   case 1: // (0x01) Free Test Buffer.
       p = (char *) data;
       etm_free(p);                         
       break;
   case 2: // (0x02) Set ETM Trace mask
       tr_etm_init(data);                   
       break;
   case 3: // (0x03) Set read all mem banks stat
       rvf_dump_mem();
       rvf_dump_pool();
       rvf_dump_tasks();
       break;
   default:
       error = ETM_NOSYS;                  
   }

   if (error < 0)
       return error;

    return ETM_OK;
}

/******************************************************************************
 * Get Version of ...
 *****************************************************************************/
// This is in development ...

int etm_version(T_ETM_PKT *pkt, uint8 *buf)
{
    extern uint16 etm_audio_revision;
    extern uint16 etm_task_revision;
    int error, fid, ffs_tm_version;
    volatile int revision = 0;
#if 0
    T_VERSION *l1s_version;
#endif

    fid = etm_get32(buf);

    tr_etm(TgTrCore, "ETM CORE: _version: fid(0x%x)", fid);

#if 0
    l1s_version = (T_VERSION*) l1s_get_version(); 
#endif

    switch (fid) {
// Code Versions related to ETM modules 
    case SW_REV_ETM_CORE:
        error = etm_pkt_put32(pkt, ETM_CORE_VERSION);
        break;
    case SW_REV_ETM_AUDIO:
        error = etm_pkt_put32(pkt, ETM_AUDIO_VERSION);
        break;
//    case SW_REV_ETM_FFS:
//        ffs_query(Q_FFS_TM_VERSION, &ffs_tm_version);
//        error = etm_pkt_put32(pkt, ffs_tm_version);
        break;
//    case SW_REV_ETM_RF: // Layer1 Testmode Version
//        error = etm_pkt_put32(pkt, TESTMODEVERSION);
//        break;
    case SW_REV_ETM_PWR:
        error = etm_pkt_put32(pkt, ETM_PWR_VERSION);
        break;
    case SW_REV_ETM_BT:
        error = ETM_NOSYS;
        break;
    case SW_REV_ETM_TASK:
        error = etm_pkt_put32(pkt, ETM_VERSION);
        break;
    case SW_REV_ETM_API:
        error = etm_pkt_put32(pkt, ETM_API_VERSION); 
        break;
// Code Versions related to L1, see in l1_defty.h 
// Get the version on this way "revision = l1s.version.dsp_code_version;" 
// doesn't work because of struct aligment -> compile flag -mw !!!!
#if 0
    case SW_DSP_CODE_VERSION:
        revision = ((T_VERSION*) l1s_version)->dsp_code_version;
        error = etm_pkt_put32(pkt, revision);
        break;
    case SW_DSP_PATCH_VERSION:
        revision = ((T_VERSION*) l1s_version)->dsp_patch_version;
        error = etm_pkt_put32(pkt, revision);
        break;
    case SW_MCU_TCS_PROGRAM_RELEASE:
        revision = ((T_VERSION*) l1s_version)->mcu_tcs_program_release;
        error = etm_pkt_put32(pkt, revision);
        break;
    case SW_MCU_TCS_OFFICIAL: // This version allso identify version of Layer1 
        revision = ((T_VERSION*) l1s_version)->mcu_tcs_official;
        error = etm_pkt_put32(pkt, revision);
        break;
    case SW_MCU_TCS_INTERNAL:
        revision = ((T_VERSION*) l1s_version)->mcu_tcs_internal;
        error = etm_pkt_put32(pkt, revision);
        break;
    case SW_MCU_TM_VERSION:
        revision = ((T_VERSION*) l1s_version)->mcu_tm_version;
        error = etm_pkt_put32(pkt, revision);
        break;
#endif
    default:
        error = ETM_NOSYS;
    }
    
    tr_etm(TgTrCore, "ETM CORE: _version: version(%d)", revision);

    if (error < 0)
        return error;
    
    return ETM_OK;
}


/******************************************************************************
 *  Function for reading the Die-ID from base band processor.
 *****************************************************************************/

int etm_dieID_read(T_ETM_PKT *pkt, uint8 *inbuf) 
{
	T_RV_RET result;
	int8 i;
	UINT16 val;
	volatile UINT16 *reg_p = (UINT16 *) DIE_ID_REG;

	tr_etm(TgTrCore, "ETM CORE: _dieID_read: started - Die-ID address(0x%x)", DIE_ID_REG);

	for (i = 0; i < DIE_ID_SIZE; i++) {
		val = *reg_p++;
        	result = etm_pkt_put16(pkt, val);
		if (result < 0)
			return result;
	}

	return ETM_OK;
}


/******************************************************************************
 * ETM CORE Main Function - Module
 *****************************************************************************/

int etm_core(uint8 *buf, int size)
{
// Structur of protocol data dl-link: |fid|index|data|

/*
 * As the comments below imply, the opcodes for tmcore commands used
 * to be mnemonic ASCII letters, but then at some point for some
 * non-understood reason TI decided to change them to the current set.
 *
 * I thought about changing back to the old opcodes for FreeCalypso,
 * but given that all of the available existing firmwares (mokoN, the
 * already-released leo2moko-r1, and Pirelli's fw) use the "new" opcodes,
 * I've decided to stick with the same for consistency, and let our
 * fc-tmsh work with both our own fw and the available pre-existing ones.
 */

    uint8 mid;
    uint8 fid;
    int error = 0;
    T_ETM_PKT *pkt = NULL;  

    fid = *buf++;

    tr_etm(TgTrCore, "ETM CORE: _core: fid(%c):", fid); 

    /* Create TestMode return Packet */
    if ((pkt = (T_ETM_PKT *) etm_malloc(sizeof(T_ETM_PKT))) == NULL) {
        return ETM_NOMEM;
    }
        
    // Init. of return packet
    pkt->mid    = ETM_CORE;
    pkt->status = ETM_OK;
    pkt->size   = 0;
    pkt->index  = 0;
    etm_pkt_put8(pkt, fid);
   
    switch (fid) {
#if ETM_ATP_SUPPORT
    case 0x60: // old 'G'
        error = etm_at(pkt, (char *) buf);
        break;
#endif
    case 0x61: // old 'M'
        error = etm_mem(pkt, buf);
        break;
    case 0x62: // old 'E'
        error = etm_echo(pkt, buf);
        break;
    case 0x63: // old 'R'
        error = etm_reset();
        break;
    case 0x64: // old 'T' 
        error = etm_debug(pkt, buf);
        break;
    case 0x65: // old 'V' 
        error = etm_version(pkt, buf);
        break;
    case 0x66: // old 'C'
        error = etm_codec_read(pkt, buf);
        break;
    case 0x67: // old 'D' 
        error = etm_codec_write(pkt, buf);            
        break;
    case 0x68: // old 'd'
        error = etm_dieID_read(pkt, buf);
        break;	
    default:
        tr_etm(TgTrCore,"ETM CORE: _core: fid ERROR"); 
        error = ETM_NOSYS;
        break;
    }
    
    if (error < 0) {
        tr_etm(TgTrCore,"ETM CORE: _core: FAILED"); 
        pkt->status = -error;
    }
   
    // etm_at(): send func. is controlled by primitive 
    if (fid == 'G' && error >= RV_OK) {}
    else 
        etm_pkt_send(pkt);

    etm_free(pkt); 
    return ETM_OK;
}
