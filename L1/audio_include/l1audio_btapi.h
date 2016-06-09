/****************************************************
/  FILE NAME:- BT_L1_API.h file.
/ ****************************************************/
#include "../../include/config.h"
#include "../include/l1_confg.h"
#include "../include/l1_types.h"
#if(L1_BT_AUDIO==1)

/****************************************************
/ Data structure defination
/ ****************************************************/
#define BT_STATUS_ERROR    1
#define BT_STATUS_OK          0

/*-------------------------------------------------------------------------------
 * L1AudioPcmStatus type
 *
 *     Defines status of current PCM buffer
 *
 */
typedef unsigned char L1AudioPcmStatus;

#define L1_PCM_READY		(0x01)	/* the next PCM block is ready */
#define L1_PCM_PENDING		(0x02)	/* the next PCM block is not ready yet.
                                       a callback will be made when it will be ready */
#define L1_PCM_MEDIA_ENDED	(0x03)	/* no PCM blocks are available anymore.
                                       (e.g. current track ended) */
#define L1_PCM_FAILED		(0x04)	/* no PCM blocks are ready. general failure */

typedef struct _L1AudioPcmBlock
{
	UWORD8 *pcmBuffer; /* a pointer to the buffer holding the PCM data */
	int lengthInBytes;        /* how many bytes are currently in the PCM buffer */
} L1AudioPcmBlock;

/****************************************************/
typedef struct _L1AudioPcmConfig
{
	int sampleRate;		/* PCM sample rate in Hz (e.g. 44100, 48000, 32000, 16000) */
	short numChannels;		/* number of audio channels (1 or 2, for mono or stereo) */
	short bitsPerSample;	/* number of bits per PCM sample. must always be 16. */
} L1AudioPcmConfig;
/****************************************************/

/****************************************************/
typedef void (*L1AudioPcmCallback)(L1AudioPcmBlock *pcmBlock);
/*Description:- A callback function, implemented at BTHAL_MM, which will be called by L1-MCU to send a ready PCM block.
The callback will be called by L1-MCU only after a previous call from BTHAL_MM to request a PCM block returned Pending.
NOTE: Since the callback is called from L1 context, the BTHAL_MM code should not spend considerable time in this function.
It must not block the L1-MCU task.

Parameters:
pcmBlock     [in] - a pointer to a PCM block structure
Returns: void
*/

/****************************************************/
typedef void (*L1AudioConfigureCallback) (L1AudioPcmConfig *pcmConfig);

/*Description:- A callback function, implemented at BTHAL_MM, that will be called by MCU-L1 to specify the next PCM configuration.
L1-MCU will call this function before a new audio track is going to start playing. BT side will use this info to reconfigure the current
A2DP stream, and after the configuration is completed, it will start pulling PCM blocks from the L1-MCU stack.
Parameters: pcmConfig    [in] - pointer to struct with the PCM config info
*/

/****************************************************/


BOOL L1Audio_InformBtAudioPathState (BOOL connected);
/*Description: - This function is implemented in L1-MCU and called by BTHAL_MM to inform L1,
whether the audio path to BT is up or down. The use of this function is optional. It should be used by BTHAL_MM,
to find out that L1-MCU is not being told byBMI/MMI, whether to use the BT audio or not.

Parameters:- connected  [in] -
TRUE - path is valid, MM should send the PCM to BT
FALSE - path is not valid, MM should send the PCM to handset default audio device
Returns: void
*/

/****************************************************/
void L1Audio_RegisterBthal (L1AudioPcmCallback pcmCallback, L1AudioConfigureCallback configCallback);
/*Description: - This function is implemented in L1-MCU and called by BTHAL_MM to init the L1-MCU for BT audio
and to register callback functions that the L1-MCU will need to call into BTHAL.

Parameters:
pcmCallback    [in] - pointer of the PCM callback function.
configCallback [in] - pointer of the config callback function.
Returns: void
*/


/****************************************************/
L1AudioPcmStatus L1Audio_PullPcmBlock (L1AudioPcmBlock *pcmBlock);
/*Description: - This function is implemented in L1-MCU and called by BTHAL_MM to pull the next PCM block.
If L1 already has a ready PCM block it should fill he pcmBlock parameter and return L1_PCM_READY.
If a block is not ready yet, it should return with proper return type as defined below.

Parameters:
pcmBlock    [out] - a pointer to PCM block struct. if the function returns MM_PCM_READY, MM should fill this struct with the info of the current ready PCM block .
Returns:
L1_PCM_READY:	  The next PCM block is ready now
L1_PCM_PENDING: The next PCM block is not ready yet, a callback will be made by MM when it is ready
L1_PCM_MEDIA_ENDED: no more PCM blocks will be available anymore for the current track
L1_PCM_FAILED:	  other failures
*/

/****************************************************/

#endif
