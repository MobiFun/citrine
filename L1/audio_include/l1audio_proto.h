/************* Revision Controle System Header *************
 *                  GSM Layer 1 software
 * L1AUDIO_PROTO.H
 *
 *        Filename l1audio_proto.h
 *  Copyright 2004 (C) Texas Instruments
 *
 ************* Revision Controle System Header *************/

#if (AUDIO_TASK == 1)

#ifndef _L1AUDIO_PROTO_H_
#define _L1AUDIO_PROTO_H_

// Functions declared in l1audio_afunc.c
void l1a_audio_send_confirmation(UWORD32 SignalCode);

void l1a_audio_send_result      (UWORD32 SignalCode, xSignalHeaderRec *msg, UWORD8 queue);
#if (SPEECH_RECO)
void l1_send_sr_background_msg  (UWORD32 SignalCode);
#endif
#if (MELODY_E2)
void l1_send_melody_e2_background_msg(UWORD32 SignalCode, UWORD8  melody_id);
#endif
#endif    // _L1AUDIO_PROTO_H_

#endif    // AUDIO_TASK
