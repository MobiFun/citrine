/********************************************************************************
 * Enhanced TestMode (ETM)
 * @file	etm_audio.c (Support for AUDIO commands)
 *
 * @author	Kim T. Peteren (ktp@ti.com)
 * @version 0.8
 *

 *
 * History:
 *
 * 	Date       	Modification
 *  ------------------------------------
 *  16/06/2003	Creation
 *  30/06/2003  Small cleanup in func. etm_audio_write, etm_audio_saveload and
 *              etm_aud_wait_for_aud_event is updated and renamed
 *  12/08/2003  The func. etm_audio_write has been updated regarding the AEC struct.
 *  14/08/2003  The func. etm_audio_read has been updated regarding the AEC struct.
 *  17/03/2004  Modified the event handling, events revceived from the Audio SWE. 
 *              Integrated event callback function, etm_audio_callback().
 *
 * (C) Copyright 2003 by Texas Instruments Incorporated, All Rights Reserved
 *********************************************************************************/


#include "etm/etm.h"
#include "etm/etm_api.h"
#include "etm/etm_trace.h"
#include "etm/etm_env.h" // Need because use of T_ETM_ENV_CTRL_BLK 
#include "etm/etm_audio_err.h" // Privat Audio error codes for PC and Target 

#include "etm/etm_trace.h"
#include "audio/audio_api.h"

#include "rv/rv_general.h"
#include "spi/spi_drv.h" // used for codec read/write

#include "memif/mem.h"
#include <string.h>


/******************************************************************************
 * Globals
 *****************************************************************************/

// Version of the ETM AUDIO module
//const uint16 etm_audio_revision = (1<<12) | (0x1);

extern T_ETM_ENV_CTRL_BLK *etm_env_ctrl_blk;

static int etm_audio_event_status = 0;


/******************************************************************************
 * Internal prototypes 
 *****************************************************************************/

int etm_audio(uint8 *indata, int insize);
T_RV_HDR *etm_audio_wait_for_event(UINT16 msg_id_expected);
T_ETM_PKT *etm_audio_setup(uint8 fid, uint8 cfg_data);
void etm_audio_callback(void *event_from_audio);


/******************************************************************************
 * Register the Audio Module to the ETM database 
 *****************************************************************************/

int etm_audio_init(void)
{
    int result; 

    result = etm_register("AUDIO", ETM_AUDIO, 0, 0, etm_audio);
    return result;    
}


/******************************************************************************
 * Audio Full Access Read Function
 *****************************************************************************/

int etm_audio_read(T_ETM_PKT *pkt, uint8 *buf)
{
    int result, size = 0, size_tmp, i;
    uint8 param;
    T_AUDIO_FULL_ACCESS_READ audio;
    T_AUDIO_AEC_CFG *aec_parameter = NULL;
    void *parameter = NULL;
    
    param = *buf;
    if ((result = etm_pkt_put8(pkt, param)) < 0) 
        return result;

    tr_etm(TgTrAudio, "ETM AUDIO: _audio_read: param(%d)", param);
   
    audio.variable_indentifier = param;
    audio.data = (T_AUDIO_FULL_ACCESS_READ *) &pkt->data[2]; //data[0] = fid
                                                             //data[1] = parameter/identifier
    
    if ((result = audio_full_access_read(&audio)) != AUDIO_OK){
        tr_etm(TgTrAudio, "ETM AUDIO: _audio_read: ERROR(%d)", result);
        if (result == AUDIO_ERROR) 
            return ETM_INVAL;         // Invalid audio parameter
        else
            return ETM_AUDIO_FATAL;
    }

    switch (param) {
    case AUDIO_PATH_USED:
        size = sizeof(T_AUDIO_VOICE_PATH_SETTING);
        break;
    case AUDIO_MICROPHONE_MODE:
    case AUDIO_MICROPHONE_GAIN:
    case AUDIO_MICROPHONE_EXTRA_GAIN:
    case AUDIO_MICROPHONE_OUTPUT_BIAS:
    case AUDIO_MICROPHONE_SPEAKER_LOOP_SIDETONE:
        size = sizeof(INT8);                         
        break;
    case AUDIO_MICROPHONE_SPEAKER_LOOP_AEC:
        size = sizeof(T_AUDIO_AEC_CFG);              
      
        aec_parameter = (T_AUDIO_AEC_CFG *) &pkt->data[2];

        etm_pkt_put16(pkt, aec_parameter->aec_enable);                  // 1
#if (L1_NEW_AEC)
        etm_pkt_put16(pkt, aec_parameter->continuous_filtering);        // 2
        etm_pkt_put16(pkt, aec_parameter->granularity_attenuation);     // 3
        etm_pkt_put16(pkt, aec_parameter->smoothing_coefficient);       // 4
        etm_pkt_put16(pkt, aec_parameter->max_echo_suppression_level);  // 5
        etm_pkt_put16(pkt, aec_parameter->vad_factor);                  // 6
        etm_pkt_put16(pkt, aec_parameter->absolute_threshold);          // 7
        etm_pkt_put16(pkt, aec_parameter->factor_asd_filtering);        // 8
        etm_pkt_put16(pkt, aec_parameter->factor_asd_muting);           // 9
        etm_pkt_put16(pkt, aec_parameter->aec_visibility);              //10
#else
        etm_pkt_put16(pkt, aec_parameter->aec_mode);                    // 2
        etm_pkt_put16(pkt, aec_parameter->echo_suppression_level);      // 3
#endif // end of (L1_NEW_AEC)
        etm_pkt_put16(pkt, aec_parameter->noise_suppression_enable);    // 4 or 11
        etm_pkt_put16(pkt, aec_parameter->noise_suppression_level);     // 5 or 12
        break;
    case AUDIO_MICROPHONE_FIR:
    case AUDIO_SPEAKER_FIR:
        size = sizeof(T_AUDIO_FIR_COEF);             
        break;
    case AUDIO_SPEAKER_MODE:
    case AUDIO_SPEAKER_GAIN:
    case AUDIO_SPEAKER_FILTER:
    case AUDIO_SPEAKER_BUZZER_STATE:
        size = sizeof(INT8);                         
        break;
    case AUDIO_SPEAKER_VOLUME_LEVEL:
        size = sizeof(T_AUDIO_SPEAKER_LEVEL);        
        break;
    default:
        size = ETM_INVAL;                            
    }

    pkt->size += size;
    return ETM_OK;
}

 
/******************************************************************************
 * Audio Full Access Write Function
 *****************************************************************************/

int etm_audio_write(T_ETM_PKT *pkt, uint8 *buf)
{
    T_RV_HDR *msg = NULL;
    T_RV_RETURN return_path;
    T_AUDIO_FULL_ACCESS_WRITE audio;
    T_AUDIO_AEC_CFG *aec_parameter = NULL;
    void *parameter = NULL;
    int result = ETM_OK, i;
    uint8 param;

    param = *buf++;
    if ((result = etm_pkt_put8(pkt, param)) < ETM_OK) {
        return result;
    } 

    tr_etm(TgTrAudio, "ETM AUDIO: _audio_write: param(%d)", param);

    return_path.addr_id        = NULL; //etm_env_ctrl_blk->addr_id;
    return_path.callback_func  = etm_audio_callback;

    audio.variable_indentifier = param;
    audio.data = buf;

    switch (param) {
    case AUDIO_MICROPHONE_SPEAKER_LOOP_AEC:
        tr_etm(TgTrAudio, "ETM AUDIO: _audio_write: AUDIO_MICROPHONE_SPEAKER_LOOP_AEC"); // RemoveMe
        aec_parameter = etm_malloc (sizeof(T_AUDIO_AEC_CFG));
        
        aec_parameter->aec_enable =                 etm_get16(buf); buf += 2;// 1
#if (L1_NEW_AEC)
        if (etm_get16(buf))  // 2
            aec_parameter->continuous_filtering = TRUE;
         else
            aec_parameter->continuous_filtering = FALSE;
        buf += 2;
        aec_parameter->granularity_attenuation =    etm_get16(buf); buf += 2;// 3
        aec_parameter->smoothing_coefficient =      etm_get16(buf); buf += 2;// 4
        aec_parameter->max_echo_suppression_level = etm_get16(buf); buf += 2;// 5
        aec_parameter->vad_factor =                 etm_get16(buf); buf += 2;// 6
        aec_parameter->absolute_threshold =         etm_get16(buf); buf += 2;// 7
        aec_parameter->factor_asd_filtering =       etm_get16(buf); buf += 2;// 8
        aec_parameter->factor_asd_muting =          etm_get16(buf); buf += 2;// 9
        aec_parameter->aec_visibility =             etm_get16(buf); buf += 2;// 10
#else
        aec_parameter->aec_mode =                   etm_get16(buf); buf += 2;// 2
        aec_parameter->echo_suppression_level =     etm_get16(buf); buf += 2;// 3
#endif // end of (L1_NEW_AEC)
#if (L1_ANR == 0)
        aec_parameter->noise_suppression_enable =   etm_get16(buf); buf += 2;// 4 or 11
        aec_parameter->noise_suppression_level =    etm_get16(buf); // 5 or 12
#endif // end of (L1_ANR)
        audio.data = aec_parameter;
        break;
    case AUDIO_MICROPHONE_FIR:
    case AUDIO_SPEAKER_FIR:
        tr_etm(TgTrAudio, "ETM AUDIO: _audio_write: AUDIO_MICROPHONE/SPEAKER_FIR [%d]", 
               sizeof(T_AUDIO_FIR_COEF)/2); // RemoveMe

        parameter = etm_malloc (sizeof(T_AUDIO_FIR_COEF));
        // Write coeffient values
        for (i=0; i <= (sizeof(T_AUDIO_FIR_COEF)/2); i++) {
            ((T_AUDIO_FIR_COEF *) parameter)->coefficient[i]  = etm_get16(buf);  buf += 2;
        }
        audio.data = parameter;
        break;
    }

    if ((result = audio_full_access_write(&audio, return_path)) != AUDIO_OK) {
        tr_etm(TgTrAudio, "ETM AUDIO: _audio_write: ERROR(%d)", result);
        if (result == AUDIO_ERROR) 
            result = ETM_INVAL;         // Invalid audio parameter
        else
            result = ETM_AUDIO_FATAL;
    }

    // Wait for recv. of event: AUDIO_FULL_ACCESS_WRITE_DONE
    rvf_wait(0xffff, 100); // Timeout 100 ticks
    tr_etm(TgTrAudio, "ETM AUDIO: _audio_write: STATUS(%d)", etm_audio_event_status);

    if (parameter != NULL) {
        etm_free(parameter);
        parameter = NULL;
    }

    if (aec_parameter != NULL) {
        etm_free(aec_parameter);    
        aec_parameter = NULL;
    }

    if (etm_audio_event_status != 0) {
        etm_audio_event_status = 0;
        result = ETM_AUDIO_FATAL;
    }

    return result;
}

/******************************************************************************
 * Audio Save and Load cfg file Function
 *****************************************************************************/

int etm_audio_saveload(T_ETM_PKT *pkt, uint8 saveload, void *buf, int size)
{
    T_RV_HDR *msg;
    T_AUDIO_MODE_SAVE audio_s;
    T_AUDIO_MODE_LOAD audio_l;
    T_RV_RETURN return_path;
    int result = ETM_OK; 
    int error, event;
    
    return_path.addr_id        = etm_env_ctrl_blk->addr_id;
    return_path.callback_func  = NULL;

    switch(saveload) {
    case 'S':
        memcpy(audio_s.audio_mode_filename, buf, size);
        result = audio_mode_save(&audio_s, return_path); 
        break;
    case 'L':
        memcpy(audio_l.audio_mode_filename, buf, size);
        result = audio_mode_load(&audio_l, return_path); 
        break;
    default:
        tr_etm(TgTrAudio, "ETM AUDIO: _audio_saveload: FAILED"); 
        break;
    }
    
    rvf_wait(0xffff, 100); // Timeout 100 ticks
    tr_etm(TgTrAudio, "ETM AUDIO: _audio_saveload: STATUS(%d)", etm_audio_event_status);

    if (etm_audio_event_status != 0) {
        etm_audio_event_status = 0;
        return ETM_AUDIO_FATAL;
    }

    if (result != AUDIO_OK)
        return ETM_AUDIO_FATAL;

    return result;
}


/******************************************************************************
 * ETM AUDIO callback functio
 *****************************************************************************/

void etm_audio_callback(void *event_from_audio)
{
    tr_etm(TgTrEtmLow,"ETM: AUDIO: _audio_callback: recv. event (0x%x)", 
           ((T_RV_HDR *) event_from_audio)->msg_id);
    
    switch (((T_RV_HDR *) event_from_audio)->msg_id)
    {
    case AUDIO_FULL_ACCESS_WRITE_DONE:
        tr_etm(TgTrAudio, "ETM AUDIO: _audio_callback: recv. event AUDIO_FULL_ACCESS_WRITE_DONE");
        etm_audio_event_status = ((T_AUDIO_FULL_ACCESS_WRITE_DONE *) event_from_audio)->status;
        break;
    case AUDIO_MODE_SAVE_DONE:
        tr_etm(TgTrAudio, "ETM AUDIO: _audio_callback: recv. event AUDIO_MODE_SAVE_DONE");
        etm_audio_event_status = ((T_AUDIO_SAVE_DONE *) event_from_audio)->status;
        break;
    case AUDIO_MODE_LOAD_DONE:
        tr_etm(TgTrAudio, "ETM AUDIO: _audio_callback: recv. event AUDIO_MODE_LOAD_DONE");
        etm_audio_event_status = ((T_AUDIO_LOAD_DONE *) event_from_audio)->status;
        break;
    }
    
    if (event_from_audio != NULL) {
//        etm_free(event_from_audio); 
        event_from_audio = NULL;
    }
}


/******************************************************************************
 * ETM AUDIO Moudle - Main Task
 *****************************************************************************/

// AUDIO packet structure for audio read/write and codec read/write: 
// |fid(8)|param(8)|--data(W)--| and for audio save/load |fid|--data(W)--|

int etm_audio(uint8 *indata, int insize)
{
    int error = ETM_OK;
    uint8 fid;
    T_ETM_PKT *pkt = NULL;  

    fid = *indata++;   
       
    tr_etm(TgTrAudio, "ETM AUDIO: _audio: fid(%c) param(%d) recv. size(%d)", 
           fid, *indata, insize); 

    /* Create TestMode return Packet */
    if ((pkt = (T_ETM_PKT *) etm_malloc(sizeof(T_ETM_PKT))) == NULL) {
        return ETM_NOMEM;
    }
    
    // Init. of return packet
    pkt->mid     = ETM_AUDIO;
    pkt->status  = ETM_OK;
    pkt->size    = 0;
    pkt->index   = 0;
    etm_pkt_put8(pkt, fid);
    
    if (error == ETM_OK) {
        switch (fid) {
        case 'R':
            error = etm_audio_read(pkt, indata);                  
            break;
        case 'W':
            error = etm_audio_write(pkt, indata); 
            break;
        case 'S':
        case 'L': 
            error = etm_audio_saveload(pkt, fid, indata, insize);  
            break;
        default:
            tr_etm(TgTrAudio, "ETM AUDIO: _audio: fid(%c) - ERROR ", fid);
            error = ETM_NOSYS;                                
            break;
        }
    }
    
    if (error < 0) {
        tr_etm(TgTrAudio,"ETM AUDIO: _audio: ERROR(%d)", error); 
        pkt->status = -error;

    }

    etm_pkt_send(pkt);
    etm_free(pkt);

    return ETM_OK;
}

