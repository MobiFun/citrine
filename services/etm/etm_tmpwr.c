/******************************************************************************
 * Enhanced TestMode (PWR)
 * Copyright Texas Instruments, 2003
 * Svend Kristian Lindholm, skl@ti.com
 *
 * $Id: tmpwr.c 1.1 Wed, 20 Aug 2003 10:22:37 +0200 skl $
 *
 ******************************************************************************/

#include "etm/etm.h"
#include "etm/etm_trace.h"
#include "etm/etm.h"
#include "etm/etm_env.h"
#include "etm/etm_api.h"
#include "etm/etm_misc.h"

#include "rv/rv_general.h"
#include "rv/rv_defined_swe.h"

#include "memif/mem.h"
#include <string.h>


#include "rv/rv_defined_swe.h"
#define TRACE_WANTED 1
#include "lcc/lcc_api.h"
#include "lcc/lcc_cfg_i.h"
#include "lcc/lcc.h"
#include "lcc/lcc_trace.h"
#include "lcc/lcc_tm_i.h"
#include "lcc/lcc_task.h"
#include "lcc/lcc_modulate.h"
#include "ffs/ffs.h"

/******************************************************************************
 * Globals
 *****************************************************************************/

extern T_ETM_ENV_CTRL_BLK *etm_env_ctrl_blk;
extern T_PWR_CTRL_BLOCK *pwr_ctrl;
extern T_PWR_CFG_BLOCK *pwr_cfg;


/******************************************************************************
 * EMT PWR Version
 *****************************************************************************/
/* 
The version of the etm pwr module can be found in the file etm_version.h
*/

/******************************************************************************
 * ETM PWR Module
 *****************************************************************************/
// pwr uplink packet structure for pwr read/write : 
// |fid|index|--data(W)--|

int etm_pwr(uint8 *indata, int insize);

/******************************************************************************
 * Register the PWR Module to the ETM database 
 *****************************************************************************/

int etm_pwr_init(void)
{
    return etm_register("PWR", ETM_PWR, 0, 0, etm_pwr);
}


int etm_pwr(uint8 *buf, int insize)
{
    int error = ETM_OK;
    uint8 fid, size, index,i;
    T_ETM_PKT *pkt = NULL;  
    T_FFS_STAT stat;
    char name[20], id;

    ttw(pwr_ttr(TTrInit, "etm_pwr(%d)" NL, 0));

    fid = *buf;

    ttw(pwr_ttr(TTrInit, "insize = %d " NL, insize));
       
    /* Create TestMode return Packet */
    if ((pkt = (T_ETM_PKT *) etm_malloc(sizeof(T_ETM_PKT))) == NULL) {
        return ETM_NOMEM;
    }
    
    // Init. of return packet
    pkt->mid     = ETM_PWR;
    pkt->status  = ETM_OK;
    pkt->size    = 0;
    pkt->index   = 0;
    etm_pkt_put8(pkt, fid);

    index = *(buf+1);
    etm_pkt_put8(pkt, index);
    
    name[0] = 0; // FIXME: Really needed?

    for (i=0; i<insize; i++) {
        ttw(ttr(TTrTmpwr, "buf[%d]=%d" NL,i, (*(buf+i))));
    }

    switch (fid) {
    case 'R':
	ttw(ttr(TTrTmpwr,"pwr (%d)" NL, 0));

	// All reads start from index 2 since the fid + index is kept
	switch (index) {
	case PWR_CFG_ID :
	    // We can read different configurations swiching between them using pwtw
	    size = PWR_CFG_ID_SIZE;
	    *(buf+2) = pwr_ctrl->cfg_id;
	    *(buf+3) = pwr_ctrl->chg_cfg_id;
	    ttw(ttr(TTrTmpwr,"Using cfg_id %d" NL, *(buf+2)));
	    ttw(ttr(TTrTmpwr,"Using chg_cfg_id %d" NL, *(buf+3)));

	    // If either the bat.cfg or the chg.cfg lacks object not found is returned

	    // If (battery) file can't be stat'ed then the configuration doesn't exist
	    id = *(buf+2) + '0';
	    build_name("/pwr/bat/bat", &id, 12, ".cfg", name);
	    error = ffs_stat(name, &stat);

	    // If charger file can't be stat'ed then the configuration doesn't exist
	    id = *(buf+3) + '0';
	    build_name("/pwr/chg/chg", &id, 12, ".cfg", name);
	    error = ffs_stat(name, &stat);
	    break;
	case PWR_COMMON :
	    // Read the /pwr/common.cfg file
	    // NOTE: sizeof(pwr_cfg->common) = 16 <>  (14 byte alignment)
	    size = PWR_COMMON_CFG_SIZE;
	    error = ffs_fread("/pwr/common.cfg", buf+2, size);
	    ttw(ttr(TTrTmpwr,"Read %d bytes" NL, error));
	    break;
	case PWR_CHG :
	    // Read the /pwr/chg/chg<cfgid>.cfg file
	    size = PWR_CHG_CFG_SIZE;
	    id = pwr_ctrl->chg_cfg_id + '0';
	    build_name("/pwr/chg/chg", &id, 12, ".cfg", name);
	    error = ffs_fread(name, buf+2, size);
	    ttw(ttr(TTrTmpwr, "Read %d bytes " NL, error));
	    break;
	case PWR_BAT :
	    // Read the /pwr/bat/bat<cfgid>.cfg file
	    size = PWR_BAT_CFG_SIZE;
	    id = pwr_ctrl->cfg_id + '0';
	    build_name("/pwr/bat/bat", &id, 12, ".cfg", name);
	    error = ffs_fread(name, buf+2, size);
	    ttw(ttr(TTrTmpwr,"Read %d bytes" NL, error));
	    break;
	case PWR_TEMP :
	    // Read the /pwr/bat/temp<cfgid>.cfg file
	    size = PWR_TEMP_CFG_SIZE;
	    id = pwr_ctrl->cfg_id + '0';
	    build_name("/pwr/bat/temp", &id, 13, ".cfg", name);
	    error = ffs_fread(name, buf+2, size);
	    ttw(ttr(TTrTmpwr,"Read %d bytes" NL, error));
	    break;
	case PWR_MMI:
	    // Read the /mmi/pwr/bsie.cfg file
	    size = PWR_MMI_CFG_SIZE;
	    error = ffs_fread("/mmi/pwr/bsie.cfg", buf+2, size);
	    ttw(ttr(TTrTmpwr,"Read %d bytes" NL, error));
	    break;
	case PWR_I2V_CAL :
	    // Read the /pwr/i2v.cal file
	    size = PWR_I2V_CAL_SIZE;
	    error = ffs_fread("/pwr/i2v.cal", buf+2, size);
	    ttw(ttr(TTrTmpwr,"Read %d bytes" NL, error));
	    break;
	case PWR_VBAT_CAL :
	    // Read the /pwr/vbat.cal file
	    size = PWR_VBAT_CAL_SIZE;
	    error = ffs_fread("/pwr/vbat.cal", buf+2, size);
	    ttw(ttr(TTrTmpwr,"Read %d bytes" NL, error));
	    break;
	case PWR_MMI_TEST :
#if (TEST_PWR_MMI_INTERFACE == 1)
	    // Trigger MMI registration 
	    return_path.callback_func = mmi_test_cb_function;
	    ttw(ttr(TTrInit,"before: &mmi_test_cb_function=(0x%x)" NL, return_path.callback_func));
	    return_path.addr_id = NULL; // FIXME??
	    ttw(ttr(TTrTmpwr,"MMI testing callback: %d" NL, 0x0));
	    size = 1;
	    *(buf+2) = 0xBA;
	    pwr_register(&return_path, &mmi_data);
	    ttw(ttr(TTrInit,"after: &mmi_test_cb_function=(0x%x)" NL, pwr_ctrl->rpath.callback_func));
	    ttw(ttr(TTrTmpwr,"MMI testing callback: %d" NL, 0xFF));
#endif
	    break;
	case PWR_DYNAMIC :
	    // Dump 'dynamic' configuration data to trace
	    size = PWR_DYNAMIC_SIZE;
	    *(buf+2) = 0xBA;
	    ttr(TTrAll,"*PWR Module Version = 0x%x" NL, PWRVERSION );
	    ttr(TTrAll,"*bat_id = %d" NL,               pwr_cfg->data.bat_id );
	    ttr(TTrAll,"*state = %d" NL,                pwr_ctrl->state);
	    ttr(TTrAll,"*chg_id = %d" NL,               pwr_cfg->data.chg_id );
	    ttr(TTrAll,"*Vbat = %d" NL,                 pwr_cfg->data.Vbat);
	    ttr(TTrAll,"*Vbat_avg = %d" NL,             pwr_cfg->data.Vbat_avg);
	    ttr(TTrAll,"*Vbat_avg_mV = %d" NL,          pwr_cfg->data.Vbat_avg_mV);
	    ttr(TTrAll,"*Tbat = %d" NL,                 pwr_cfg->data.Tbat);
	    ttr(TTrAll,"*Tbat_avg = %d" NL,             pwr_cfg->data.Tbat_avg);
	    ttr(TTrAll,"*Vchg = %d" NL,                 pwr_cfg->data.Vchg);
	    ttr(TTrAll,"*Ichg = %d" NL,                 pwr_cfg->data.Ichg);
	    ttr(TTrAll,"*Cbat = %d" NL,                 pwr_cfg->data.Cbat);
	    ttr(TTrAll,"*cfg_id = %d" NL,               pwr_cfg->data.cfg_id);
	    ttr(TTrAll,"*chg_cfg_id = %d" NL,           pwr_cfg->data.chg_cfg_id);
	    ttr(TTrAll,"*bforce = %d" NL,               pwr_cfg->data.bforce);
	    ttr(TTrAll,"*cforce = %d" NL,               pwr_cfg->data.cforce);
	    ttr(TTrAll,"*k = %d" NL,                    pwr_cfg->data.k);
	    ttr(TTrAll,"*T4 = %d" NL,                   pwr_cfg->data.T4);
	    ttr(TTrAll,"*T1 elapsed = %d" NL,           pwr_ctrl->time_elapsed_T1);
	    ttr(TTrAll,"*T2 elapsed = %d" NL,           pwr_ctrl->time_elapsed_T2);
	    ttr(TTrAll,"*T3 elapsed = %d" NL,           pwr_ctrl->time_elapsed_T3);
	    ttr(TTrAll,"*MMI timer elapsed = %d" NL,    pwr_ctrl->time_elapsed_mmi_rep);
	    error = PWR_OK;
	    break;
	case PWR_TRACE_MASK:
	    // Read the trace mask of the PWR module 
	    size = PWR_TMASK_SIZE;
	    memcpy(buf+2, &pwr_ctrl->tmask, size);
	    ttw(ttr(TTrTmpwr,"tmask: 0x%x" NL, pwr_ctrl->tmask));
	    break;
	default :
	    {
	     // Unknown index
	    error = PWR_INDEX;
	    ttr(TTrWarning, "Unknown index! %d" NL, index);
	    }
	}

	etm_pkt_putdata(pkt, buf+2 , size);
        break;
    case 'W':

	ttw(ttr(TTrTmpwr,"pww (%d)" NL, 0));
	switch (index) {
	case PWR_CFG_ID :
	    // Write the configuration id to be used
	    pwr_ctrl->cfg_id = *(buf+2);
	    pwr_ctrl->chg_cfg_id = *(buf+3);
	    pwr_cfg->data.cfg_id = pwr_ctrl->cfg_id + '0';
	    pwr_cfg->data.chg_cfg_id = pwr_ctrl->chg_cfg_id + '0';
	    ttw(ttr(TTrTmpwr,"Switched to bat id=%d" NL, pwr_ctrl->cfg_id));
	    ttw(ttr(TTrTmpwr,"Switched to chg id=%d" NL, pwr_ctrl->chg_cfg_id));
	    break;
	case PWR_COMMON :
	    // Write the /pwr/common.cfg file
	    // Blocking version of ffs_fwrite is used since this is a test mode command
	    error = ffs_fwrite("/pwr/common.cfg", buf+2, PWR_COMMON_CFG_SIZE);
	    ttw(ttr(TTrTmpwr,"Wrote %d bytes" NL, error));
	    break;
	case PWR_CHG :
	    // Write the /pwr/chg/chg<cfgid>.cfg file
	    size = PWR_CHG_CFG_SIZE;
	    id = pwr_ctrl->chg_cfg_id + '0';
	    build_name("/pwr/chg/chg", &id, 12, ".cfg", name);
	    error = ffs_fwrite(name, buf+2, size);
	    ttw(ttr(TTrTmpwr, "Wrote %d bytes = %d" NL, error));
	    break;
	case PWR_BAT :
	    // Write the /pwr/bat/bat<cfgid>.cfg file
	    size = PWR_BAT_CFG_SIZE;
	    id = pwr_ctrl->cfg_id + '0';
	    build_name("/pwr/bat/bat", &id, 12, ".cfg", name);
	    error = ffs_fwrite(name, buf+2, size);
	    ttw(ttr(TTrTmpwr, "Wrote %d bytes = %d" NL, error));
	    break;
	case PWR_TEMP :
	    // Write the /pwr/bat/temp<cfgid>.cfg file
	    size = PWR_TEMP_CFG_SIZE;
	    id = pwr_ctrl->cfg_id + '0';
	    build_name("/pwr/bat/temp", &id, 13, ".cfg", name);
	    error = ffs_fwrite(name, buf+2, size);
	    ttw(ttr(TTrTmpwr, "Wrote %d bytes = %d" NL, error));
	    break;
	case PWR_MMI:
	    // Write the /mmi/pwr/bsie.cfg file
	    size = PWR_MMI_CFG_SIZE;
	    error = ffs_fwrite("/mmi/pwr/bsie.cfg", buf+2, size);
	    ttw(ttr(TTrTmpwr, "Wrote %d bytes = %d" NL, error));
	    break;
	case PWR_I2V_CAL :
	    // Write the /pwr/i2v.cal file
	    error = ffs_fwrite("/pwr/i2v.cal", buf+2, PWR_I2V_CAL_SIZE);
	    ttw(ttr(TTrTmpwr,"Wrote %d bytes" NL, PWR_I2V_CAL_SIZE));
	    break;
	case PWR_VBAT_CAL:
	    // Write the /pwr/vbat.cal file
	    error = ffs_fwrite("/pwr/vbat.cal", buf+2, PWR_VBAT_CAL_SIZE);
	    ttw(ttr(TTrTmpwr,"Wrote %d bytes" NL, PWR_VBAT_CAL_SIZE));
	    break;
	case PWR_MMI_TEST :
	    break;
	case PWR_TRACE_MASK:
	    // Write the trace mask of the PWR module 
	    memcpy(&pwr_ctrl->tmask, buf+2, sizeof(pwr_ctrl->tmask));
	    ttw(ttr(TTrTmpwr,"Wrote tmask 0x%x" NL, pwr_ctrl->tmask));
	    pwr_ttr_init(pwr_ctrl->tmask);
	    break;
	default :
	    {
	     // Unknown index
	    error = PWR_INDEX;
	    ttr(TTrWarning, "Unknown index! %d" NL, index);
	    }
	}

	size = 0; // Size of write message reply is always 0

        break;
    default:
        pwr_ttr(TTrWarning, "etm_pwr: fid unknown (%d)" NL, fid);
        error = ETM_NOSYS;
        break;
    }
    
    for (i=0; i<size+2; i++) {
        ttw(ttr(TTrTmpwr, "buf[%d]=0x%x" NL, i, (*(buf+i))));
    }

    if (error < 0) {
        pwr_ttr(TTrWarning, "etm_pwr: error %d" NL, error);
        pkt->status = -error;

    }

    pkt->size += size;

    etm_pkt_send(pkt);
    etm_free(pkt);

    ttw(pwr_ttr(TTrInit, "etm_pwr(%d)" NL, 0xFF));
    return ETM_OK;
}
