/* 
+----------------------------------------------------------------------------- 
|  Project :  BAT L2P ()
|  Modul   :  BAT_L2P
+----------------------------------------------------------------------------- 
|  Copyright 2005 Texas Instruments Berlin, AG 
|                 All rights reserved. 
| 
|                 This file is confidential and a trade secret of Texas 
|                 Instruments Berlin, AG 
|                 The receipt of or possession of this file does not convey 
|                 any rights to reproduce or disclose its contents or to 
|                 manufacture, use, or sell anything it may describe, in 
|                 whole, or in part, without the specific written consent of 
|                 Texas Instruments Berlin, AG. 
+----------------------------------------------------------------------------- 
|  Purpose :  L2P function definitions
+----------------------------------------------------------------------------- 
*/ 
#include "typedefs.h"   /* to get TI data types */
#include "vsi.h"        /* to get a lot of macros */
#include "l2p_types.h"
#include "l2p.h"

#include "string.h"

T_L2P_CONTROL_BLOCK *l2p_control_block_list = (T_L2P_CONTROL_BLOCK *)0;

/*
** Local function declarations
*/
static char *L2P_write_data_to_seg(char *writePtr, char *dataPtr, U16 dataSize, U16 remain_seg_size,
                                   U16 *bytesWritten);

static char *L2P_read_data_from_seg(char *readPtr, char *writePtr, U16 dataSize, U16 remain_seg_size,
                                    U16 *bytesWritten);


/*
** L2P function definitions
*/

/*
+--------------------------------------------------------------------+
| PROJECT: BAT L2P                  MODULE:   L2P                    |
| STATE  : code                     ROUTINE:  L2P_Configure          |
+--------------------------------------------------------------------+

   PURPOSE : L2P_Configure will initiaise the L2P for each instance of BAT Lib or BAT Module
             that wishes to use its services. The following is a list of the parameters, and their uses:

             batId : A identifer to uniquely identify each instance of BAT using this L2P
             mem : A block of memory for L2P to use as a Control Block (L2P cannot allocate its own memory)
             num_mux_chnls: The number of clients being supported at the application end (this information is
             passed to BAT Module as part of the startup process)
             protocolId : The protocol being used to communicate with the peer entity
             maxMTU : Maximum size of a transport unit.
             xxx_CB : These are callback functions, provided by the BAT Lib or BAT Module, to provide necessary
                      services to L2P to get Send and Receive buffers, Send the Frames and receive fully
                      completed packets

*/
T_L2P_STATUS L2P_Configure(U8 batid, void *mem,
                           T_L2P_SUPPORTED_PROTOCOLS protocolId,
                           U16 maxMTU,
                           L2P_Get_Tx_Buf_CB  Get_Tx_Buf_Cb,
                           L2P_Get_Rx_Buf_CB Get_Rx_Buf_Cb,
                           L2P_Get_Next_Seg_CB Get_Next_Seg_Cb,
                           L2P_Send_Frame_CB Send_Frame_Cb,
                           L2P_Msg_Rxd_CB Msg_Rxd_Cb)
{
  T_L2P_CONTROL_BLOCK **curr_lcb_next_ptr;
  T_L2P_CONTROL_BLOCK *current_l2p_cntrl_block;
  T_L2P_CONTROL_BLOCK *this_l2p_cntrl_block = (T_L2P_CONTROL_BLOCK *)mem;

  TRACE_EVENT("L2P_Configure");
  /*
  ** check the incoming paramters to ensure they are valid 
  */
  if (mem == (void *)0)
  {
    TRACE_EVENT("L2P_Configure - Mem pointer Invalid");
    return L2P_STAT_INVALID_PARAMETER;
  }

  /* protocolId is a non-negative value(so <0 check not reqd) */
  if ( protocolId >= L2P_SP_MAX_NUM )
  {
    TRACE_EVENT("L2P_Configure - Protocol Id Invalid");
    return L2P_STAT_INVALID_PARAMETER;
  }

    if (maxMTU <= (L2P_FRAME_CONTROL_SIZE)) 
  {
    TRACE_EVENT("L2P_Configure - Max MTU value Invalid");
    return L2P_STAT_INVALID_PARAMETER;
  }

  if (Get_Tx_Buf_Cb == (L2P_Get_Tx_Buf_CB)0)
  {
    TRACE_EVENT("L2P_Configure - Get Tx Buffer Callback Invalid");
    return L2P_STAT_INVALID_PARAMETER;
  }

  if (Get_Rx_Buf_Cb == (L2P_Get_Rx_Buf_CB)0)
  {
    TRACE_EVENT("L2P_Configure - Get Rx Buffer Callback Invalid");
    return L2P_STAT_INVALID_PARAMETER;
  }

  if (Get_Next_Seg_Cb == (L2P_Get_Next_Seg_CB)0)
  {
    TRACE_EVENT("L2P_Configure - Get Next Segment Callback Invalid");
    return L2P_STAT_INVALID_PARAMETER;
  }

  if (Send_Frame_Cb == (L2P_Send_Frame_CB)0)
  {
    TRACE_EVENT("L2P_Configure - Send Frame Callback Invalid");
    return L2P_STAT_INVALID_PARAMETER;
  }

  if (Msg_Rxd_Cb == (L2P_Msg_Rxd_CB)0)
  {
    TRACE_EVENT("L2P_Configure - Message Received Callback Invalid");
    return L2P_STAT_INVALID_PARAMETER;
  }

  /*
  ** initialise the data in the passed block of data
  */
  this_l2p_cntrl_block->batId = batid;
  this_l2p_cntrl_block->frameId = 0;
  this_l2p_cntrl_block->protocolId = protocolId;
  this_l2p_cntrl_block->maxMTU = maxMTU;
  this_l2p_cntrl_block->rxBuf = (char *)0;
  this_l2p_cntrl_block->muxId = 0xFD; /* Any value here will be a valid client Id, but this one is unlikely ... it MUST be initialised correctly before each use */
  this_l2p_cntrl_block->cmdTagReceived = 0;
  this_l2p_cntrl_block->receivedCmdTag = 0;
  this_l2p_cntrl_block->dataSize = 0;
  this_l2p_cntrl_block->bytesToBeRead = 0;
  this_l2p_cntrl_block->dataToBeRead = (char *)0;
  
  this_l2p_cntrl_block->callbacks.Get_TxBuf_Cb = Get_Tx_Buf_Cb;
  this_l2p_cntrl_block->callbacks.Get_RxBuf_Cb = Get_Rx_Buf_Cb;
  this_l2p_cntrl_block->callbacks.Get_Next_Seg_Cb = Get_Next_Seg_Cb;
  this_l2p_cntrl_block->callbacks.Send_Frame_Cb = Send_Frame_Cb;
  this_l2p_cntrl_block->callbacks.Msg_Rxd_Cb = Msg_Rxd_Cb;
  
  this_l2p_cntrl_block->next = (void *)0;
  
  /*
  ** Check that the batId is not already in use
  */
  if (l2p_control_block_list != (T_L2P_CONTROL_BLOCK *)0)
  {
    /*
    ** Parse the list until either the batId is found (which would be an error)
    ** or the batId is larger than the one passed (ie the current batId is not in the list, and is thereofre valid)
    */
    curr_lcb_next_ptr = &l2p_control_block_list;
    current_l2p_cntrl_block = l2p_control_block_list;

    while ((current_l2p_cntrl_block != (T_L2P_CONTROL_BLOCK *)0) &&
           (current_l2p_cntrl_block->batId < batid))
    {
      /*
      ** Jump to the next node on the linked list,
      ** keeping track of the previous node so we can add in the details later
      */
      curr_lcb_next_ptr = &current_l2p_cntrl_block->next;
      current_l2p_cntrl_block = current_l2p_cntrl_block->next;
    }

    if (current_l2p_cntrl_block == (T_L2P_CONTROL_BLOCK *)0)
    {
      /*
      ** BAT Id was not found ... and we've reached the end of the list
      ** need to add the details at the end of the linked list
      */
      *curr_lcb_next_ptr = this_l2p_cntrl_block;
    }
    else if (current_l2p_cntrl_block->batId == batid)
    {
      /*
      ** BAT Id was found in the current list ... it is already configured ... send an error
      */
      TRACE_EVENT("L2P_Configure - BAT Id Already In Use");
      return L2P_STAT_BAT_ID_ALREADY_EXISTS;
    }
    else
    {
      /*
      ** BAT Id was not found ... but we haven't reached the end of the list
      ** need to add the details in the middle of the linked list
      */
      this_l2p_cntrl_block->next = *curr_lcb_next_ptr;
      *curr_lcb_next_ptr = this_l2p_cntrl_block;
    }
  }
  else
  {
    /*
    ** The linked list of BAT and L2P Control blocks is empty, this is the first instance of BAT
    ** to register with the L2P. Add the details as the first entry to the linked list.
    */
    l2p_control_block_list = this_l2p_cntrl_block;
  }
    return L2P_STAT_SUCCESS;
}

/*
+--------------------------------------------------------------------+
| PROJECT: BAT L2P                  MODULE:   L2P                    |
| STATE  : code                     ROUTINE:  L2P_Remove             |
+--------------------------------------------------------------------+

   PURPOSE : L2P_Remove will delete a BAT from L2Ps service list. The following is a list of
             the parameters, and their uses:

             batId : A identifer to uniquely identify each instance of BAT using this L2P

*/
T_L2P_STATUS L2P_Remove(U8 batid)
{
    T_L2P_CONTROL_BLOCK **curr_lcb_next_ptr;
    T_L2P_CONTROL_BLOCK *current_l2p_cntrl_block;

    TRACE_EVENT("L2P_Remove()");
    
    if (l2p_control_block_list == (T_L2P_CONTROL_BLOCK *)0)
    {
        return L2P_STAT_NO_SUCH_BAT_ID;
    }

    if (l2p_control_block_list->batId == batid)
    {
        l2p_control_block_list = l2p_control_block_list->next;
        return L2P_STAT_SUCCESS;
    }

    /*
    ** Parse the list until either the batId is found 
    ** or the batId is larger than the one passed (which would be an error)
    */
    curr_lcb_next_ptr = &l2p_control_block_list;
    current_l2p_cntrl_block = l2p_control_block_list;

    while ((current_l2p_cntrl_block != (T_L2P_CONTROL_BLOCK *)0) &&
               (current_l2p_cntrl_block->batId < batid))
    {
        /*
        ** Jump to the next node on the linked list,
        ** keeping track of the previous node so we can add in the details later
        */
        curr_lcb_next_ptr = &current_l2p_cntrl_block->next;
        current_l2p_cntrl_block = current_l2p_cntrl_block->next;
    }

    if (current_l2p_cntrl_block == (T_L2P_CONTROL_BLOCK *)0)
    {
        /*
        ** BAT Id was not found and we've reached the end of the list
        */
        return L2P_STAT_NO_SUCH_BAT_ID;
    }
    else if (current_l2p_cntrl_block->batId == batid)
    {
        /*
        ** BAT Id was found in the current list ... remove the control balock from the linked list
        */
        *curr_lcb_next_ptr = current_l2p_cntrl_block->next;
    }
    else
    {
        /*
         ** BAT Id was not found ... the current batid must be greater tahn the one passed
        */
        return L2P_STAT_NO_SUCH_BAT_ID;
    }
    
    return L2P_STAT_SUCCESS;
}

/*
+--------------------------------------------------------------------+
| PROJECT: BAT L2P                  MODULE:   L2P                    |
| STATE  : code                     ROUTINE:  L2P_Send               |
+--------------------------------------------------------------------+

   PURPOSE : L2P_Send will receive an AT Command Tag and its data from the L2P User, and depending on the
             MTU size of the communications link being used, split the data into multiple frames and send those
             Frames across the link. The following is a list of the parameters, and their uses:

             batId   : A identifer to uniquely identify each instance of BAT using this L2P
             muxId   : A client identifier to identify which clinet of the BAT instance either sent, or is to receive, the data.
             dataTag : This is a BAT Command or Response TAG to identify which AT command is being sent. This TAG
                       will be sent to the peer along with the data.
             dataBuf : pointer to the payload data to send
             dataLen : length of the data in the 'dataBuf'
             errVal  : If there is a driver error, L2P_Send will return L2P_STAT_UNKNOWN_ERROR and will provide
                       the driver error code in errVal. This is because the number may be positive, and all errors in
                       T_L2P_STATUS are negative.

*/
T_L2P_STATUS L2P_Send(U8 batId, U8 muxId, U32 dataTag, void *dataBuf, U16 dataLen, int *errVal)
{
    T_L2P_CONTROL_BLOCK *curr_l2p_cntrl_block;
    T_L2P_FRAME_CONTROL_BYTES frameCntrlBytes;
    U16    bytesRemainingToBeSent;
    U16    frameCntrlBytesToBeWritten;
    U16    bytesToBeWritten = 0;
    U16    bytesWritten;
    U16    frameSizeReq;
    U16    totalBufSize;
    U16    segSize;
    U16    remainingSegSize;
    U16    totalSegSize;
    char   *curTxSeg;
    char   *dataToBeWritten = NULL;
    char   *frameCntrlDataToBeWritten;
    void   *seg_hdr;
    char   *txWritePtr;
    char   cmdTagWritten = 0;
    int    drvRetVal = 0;

    TRACE_EVENT("L2P_Send()");
    
    /*
    ** Check that the Bat Id is valid
    */

    curr_l2p_cntrl_block = l2p_control_block_list;

    while ((curr_l2p_cntrl_block != (T_L2P_CONTROL_BLOCK *)0) &&
               (curr_l2p_cntrl_block->batId != batId))
    {
        curr_l2p_cntrl_block = curr_l2p_cntrl_block->next;
    }

    if (curr_l2p_cntrl_block == (T_L2P_CONTROL_BLOCK *)0)
    {
        return L2P_STAT_NO_SUCH_BAT_ID;
    }

    /*
    ** Check the rest of the parameters
    */
    if ((dataBuf == (void *)0) && (dataLen != 0))
    {
        TRACE_EVENT("L2P_Send() - Invalid Data Buffer Pointer");
        return L2P_STAT_INVALID_PARAMETER;
    }

    if (errVal == (int *)0)
    {
        TRACE_EVENT("L2P_Send() - Invalid Error Return Pointer");
        return L2P_STAT_INVALID_PARAMETER;
    }

    /*
    ** The format of an L2 frame for BAT will be as follows :
    **
    **     For the first frame
    **     ----------------------------------------------------------------
    **     |  L2 Control Bytes | BAT Cmd TAG |     BAT Command Data       |
    **     |      (n Bytes)    |    4 Bytes  |     (Remaining Bytes)      |
    **     ----------------------------------------------------------------
    **
    **     For any subsequent frames
    **     ----------------------------------------------------------------
    **     |  L2 Control Bytes |     BAT Command Data                     |
    **     |      (n Bytes)    |     (Remaining Bytes)                    |
    **     ----------------------------------------------------------------
    */

    bytesRemainingToBeSent = dataLen + sizeof(int);

    /*
    ** Fill in the Frame Control Bytes
    */
    frameCntrlBytes.dataLen = dataLen;
    frameCntrlBytes.frame_id = 0;           /* Each new packet will start with a frame Id of 0 */
    frameCntrlBytes.frame_type = L2P_FT_UNKNOWN_FRAME_TYPE;
    frameCntrlBytes.mux_id = muxId;
    frameCntrlBytes.protocol_id = (U8)curr_l2p_cntrl_block->protocolId;

    while (bytesRemainingToBeSent >0)
    {
        /*
        ** Calculate the required size of Transmit buffer and how many frames will be required
        */
        if ((bytesRemainingToBeSent + L2P_FRAME_CONTROL_SIZE) > curr_l2p_cntrl_block->maxMTU)
        {
            /*
            ** Data will not fit in a single frame, so will need to be fragmented and reassembled
            */
            frameSizeReq = curr_l2p_cntrl_block->maxMTU;
        }
        else
        {
            /*
            ** Data will  fit in a single frame, so will be sent complete
            */
            frameSizeReq = bytesRemainingToBeSent + L2P_FRAME_CONTROL_SIZE;
        }

        /*
        ** Request the Transmit frame from the buffer
        */
        curTxSeg = (char *)curr_l2p_cntrl_block->callbacks.Get_TxBuf_Cb(batId, frameSizeReq,
                                                                                                  &seg_hdr,  &totalBufSize, &segSize);

        if (curTxSeg == (char *)0)
        {
            return L2P_STAT_UNABLE_TO_GET_BUFFER;
        }

        totalSegSize = remainingSegSize = segSize;

        /*
        ** Now we know the size of buffer given so we can tell whether the entire message will fit in a single frame
        */
        if (totalBufSize >= (bytesRemainingToBeSent + L2P_FRAME_CONTROL_SIZE))
        {
            if (frameCntrlBytes.frame_type == L2P_FT_UNKNOWN_FRAME_TYPE)
                frameCntrlBytes.frame_type = (U8)L2P_FT_COMPLETE_FRAME;
            else
                frameCntrlBytes.frame_type = (U8)L2P_FT_END_FRAME;

            frameCntrlBytes.frameLen = bytesRemainingToBeSent + L2P_FRAME_CONTROL_SIZE;
        }
        else
        {
            if (frameCntrlBytes.frame_type == L2P_FT_UNKNOWN_FRAME_TYPE)
                frameCntrlBytes.frame_type = (U8)L2P_FT_START_FRAME;
            else
                frameCntrlBytes.frame_type = L2P_FT_CONTINUATION_FRAME;

            frameCntrlBytes.frameLen = totalBufSize;
        }

        txWritePtr = curTxSeg;

        /*
        ** We can assume that the buffer will be at least big enough for the L2 Frame Header plus some data
        **
        ** Write the L2 Frames Control Bytes
        */
        frameCntrlBytesToBeWritten = L2P_FRAME_CONTROL_SIZE;
        frameCntrlDataToBeWritten = (char *)&frameCntrlBytes;

        while (frameCntrlBytesToBeWritten > 0)
        {
            txWritePtr = L2P_write_data_to_seg(txWritePtr, frameCntrlDataToBeWritten, frameCntrlBytesToBeWritten,
                                                                      remainingSegSize, &bytesWritten);
            frameCntrlBytesToBeWritten -= bytesWritten;
            remainingSegSize -= bytesWritten;
            frameCntrlDataToBeWritten += bytesWritten;

            /*
            ** Frame Control Bytes are NOT included in the bytesRemainingToBeSent count so do not decrease the size!
            */

            if (txWritePtr == (char *)0)
            {
                /*
                ** We have reached the end of the current Segment, so get the next.
                */
                curTxSeg = curr_l2p_cntrl_block->callbacks.Get_Next_Seg_Cb(batId, seg_hdr, &seg_hdr,  &segSize);
                txWritePtr = curTxSeg;
                totalSegSize +=segSize;
                remainingSegSize = segSize;
            }
        }

        /*
        ** if necessary, write the cmd Tag into the Buffer
        */
        if ((frameCntrlBytes.frame_type == L2P_FT_COMPLETE_FRAME) ||
             (frameCntrlBytes.frame_type == L2P_FT_START_FRAME))
        {
            bytesToBeWritten = sizeof(dataTag);
            dataToBeWritten = (char *)&dataTag;

            while ((txWritePtr != (char *)0) &&     /* There is still space in the buffer segement */
                   (!cmdTagWritten))                /* The command Tag has not completed */
            {
                txWritePtr = L2P_write_data_to_seg(txWritePtr, dataToBeWritten, bytesToBeWritten,
                                                   remainingSegSize, &bytesWritten);
                bytesToBeWritten -= bytesWritten;
                remainingSegSize -= bytesWritten;
                dataToBeWritten += bytesWritten;

                /*
                ** Decrement the bytesRemainingToBeSent counter
                */
                bytesRemainingToBeSent -= bytesWritten;

                if ((txWritePtr == (char *)0) && (totalSegSize < totalBufSize))
                {
                    /*
                    ** We have reached the end of the current Segment, so get the next.
                    */
                    curTxSeg = curr_l2p_cntrl_block->callbacks.Get_Next_Seg_Cb(batId, seg_hdr, &seg_hdr,  &segSize);
                    txWritePtr = curTxSeg;
                    totalSegSize +=segSize;
                    remainingSegSize = segSize;
                }

                /*
                ** If the Command Tag has been completed set a flag
                */
                if (bytesToBeWritten == (U16)0)
                {
                    cmdTagWritten = 1;
                    dataToBeWritten = (char *)dataBuf;    /* Next Write the AT Command Data */
                    bytesToBeWritten = dataLen;
                }
            }
        }
       
	if (dataToBeWritten EQ NULL)
	{
	    return L2P_STAT_INVALID_PARAMETER;
        }	
        /*
        ** if txWritePtr is NULL at this point it is because a buffer has been completed ... drop out and perform
        ** the next loop of the while statement to create the next L2 Frame in an new buffer.
        */
       	while ((bytesToBeWritten > 0) &&    /* There is still data to write */
               (txWritePtr != (char *)0))   /* There is still space in the buffer segement */
        {
            txWritePtr = L2P_write_data_to_seg(txWritePtr, dataToBeWritten, bytesToBeWritten,
                                               remainingSegSize, &bytesWritten);
            bytesToBeWritten -= bytesWritten;
            remainingSegSize -= bytesWritten;
            dataToBeWritten += bytesWritten;
	
            /*
            ** Decrement the bytesRemainingToBeSent counter
            */
            bytesRemainingToBeSent -= bytesWritten;

            if ((txWritePtr == (char *)0) && (totalSegSize < totalBufSize))
            {
                /*
                ** We have reached the end of the current Segment, so get the next.
                */
                curTxSeg = curr_l2p_cntrl_block->callbacks.Get_Next_Seg_Cb(batId, seg_hdr, &seg_hdr,  &segSize);
                txWritePtr = curTxSeg;
                totalSegSize +=segSize;
                remainingSegSize = segSize;
            }

            if ((bytesToBeWritten == (U16)0) &&     /* We've finished writing the data */
                (!cmdTagWritten))                   /* but that data was the Command Tag  */
            {
                /*
                ** now we have to write the AT Command Data
                */
                cmdTagWritten = 1;
                dataToBeWritten = (char *)dataBuf;
                bytesToBeWritten = dataLen;
            }
        }

        /*
        ** We have built a complete L2 Frame ... that it may or may not include the complete packet it isn't important
        ** but we need to send it ... 
        */
        drvRetVal = curr_l2p_cntrl_block->callbacks.Send_Frame_Cb(batId);
        frameCntrlBytes.frame_id ++;
	
	if (drvRetVal != 0)
        {
            /*
            ** There was an error sending the frame
            */
            *errVal = drvRetVal;
            return L2P_STAT_UNKNOWN_ERROR;
        }
   }

    return L2P_STAT_SUCCESS;
}

/*
+--------------------------------------------------------------------+
| PROJECT: BAT L2P                  MODULE:   L2P                    |
| STATE  : code                     ROUTINE:  L2P_Receive            |
+--------------------------------------------------------------------+

   PURPOSE : L2P_Receive is called on receipt of data by the BAT, BAT Command Tag is extracted and the rest
             of the L2 frames are combined into a buffer and passed, complete, to the BAT. The following is a
             list of the parameters, and their uses:

             batId : A identifer to uniquely identify each instance of BAT using this L2P
             inBuf : pointer to the received buffer
             dataLen : length of the data in the 'inBuf'

*/
T_L2P_STATUS L2P_Receive(U8 batId, void *dataBuf, void *seg_hdr_ptr, U16 totalsize, U16 segSize)
{
    T_L2P_CONTROL_BLOCK *curr_l2p_cntrl_block;
    T_L2P_FRAME_CONTROL_BYTES frameCntrlBytes;
    U16  frameCntrlBytesToBeRead;
    U16  bytesRead;
    U16  remainingSegSize;
    U16  totalSegSize;
    char *curRxSeg;
    char *rxReadPtr;
    char *frameCntrlDataToBeRead;

    TRACE_EVENT("L2P_Receive()");

    /*
    ** Check that the Bat Id is valid
    */
    curr_l2p_cntrl_block = l2p_control_block_list;

    while ((curr_l2p_cntrl_block != (T_L2P_CONTROL_BLOCK *)0) &&
           (curr_l2p_cntrl_block->batId != batId))
    {
        curr_l2p_cntrl_block = curr_l2p_cntrl_block->next;
    }

    if (curr_l2p_cntrl_block == (T_L2P_CONTROL_BLOCK *)0)
    {
        return L2P_STAT_NO_SUCH_BAT_ID;
    }

    /*
    ** Check the rest of the parameters
    */
    if (dataBuf == (void *)0)
    {
        TRACE_EVENT("L2P_Receive() - Invalid Data Buffer Pointer");
        return L2P_STAT_INVALID_PARAMETER;
    }

    /*
    ** Don't check the Segment Header Pointer, it is User Data and down to the
    ** BAT Lib or BAT Module which pointer they pass :
    **      pointer to current segment - never NULL
    **      pointer to the next segment - may be NULL
    **
    if (seg_hdr_ptr == (void *)0)
    {
        TRACE_EVENT("L2P_Receive() - Invalid Segment Header Pointer");
        return L2P_STAT_INVALID_PARAMETER;
    }
    */

    if (totalsize <= (U16)L2P_FRAME_CONTROL_SIZE)
    {
        TRACE_EVENT("L2P_Receive() - Invalid Buffer Length");
        return L2P_STAT_INVALID_PARAMETER;
    }

    if (segSize == (U16)0)
    {
        TRACE_EVENT("L2P_Receive() - Invalid Segment Size");
        return L2P_STAT_INVALID_PARAMETER;
    }

    /*
    ** The format of an L2 frame for BAT will be as follows :
    **
    **     For the first frame
    **     ----------------------------------------------------------------
    **     |  L2 Control Bytes | BAT Cmd TAG |     BAT Command Data       |
    **     |      (n Bytes)    |    4 Bytes  |       (Remaining Bytes)    |
    **     ----------------------------------------------------------------
    **
    **     For any subsequent frames
    **     ----------------------------------------------------------------
    **     |  L2 Control Bytes |       BAT Command Data                   |
    **     |      (n Bytes)    |       (Remaining Bytes)                  |
    **     ----------------------------------------------------------------
    */

    /*
    ** Read the L2 Frames Control Bytes
    */
    curRxSeg = (char *)dataBuf;
    frameCntrlBytesToBeRead = L2P_FRAME_CONTROL_SIZE;
    frameCntrlDataToBeRead = (char *)&frameCntrlBytes;
    totalSegSize = remainingSegSize = segSize;
    rxReadPtr = dataBuf;

    while (frameCntrlBytesToBeRead > 0)
    {
        rxReadPtr = L2P_read_data_from_seg(rxReadPtr, frameCntrlDataToBeRead, frameCntrlBytesToBeRead,
                                           remainingSegSize, &bytesRead);
        frameCntrlBytesToBeRead-= bytesRead;
        remainingSegSize -= bytesRead;
        frameCntrlDataToBeRead += bytesRead;

        if (rxReadPtr == (char *)0)
        {
            /*
            ** We have reached the end of the current Segment, so get the next.
            */
            curRxSeg = curr_l2p_cntrl_block->callbacks.Get_Next_Seg_Cb(batId, seg_hdr_ptr, &seg_hdr_ptr,
                                                                       &segSize);
            rxReadPtr = curRxSeg;
            totalSegSize +=segSize;
            remainingSegSize = segSize;
        }
    }

    /*
    ** Now we have the frame Control Bytes ... so we need to determine whether we need a
    ** new Rx Buffer from the user
    */

    /*
    ** If the frame Id is zero, this is a new message
    */
    if ((frameCntrlBytes.frame_id == (U8)0) && 
        ((frameCntrlBytes.frame_type == L2P_FT_START_FRAME) ||
         (frameCntrlBytes.frame_type == L2P_FT_COMPLETE_FRAME)))
    {
        /*
        ** Do we already have an Rx Buffer
        */
        if (curr_l2p_cntrl_block->rxBuf == (char *)0)
        {
            /*
            ** No -- Use the callback function to get an Rx Buffer
            */
            curr_l2p_cntrl_block->rxBuf = (char *)curr_l2p_cntrl_block->callbacks.Get_RxBuf_Cb(batId);

            if (curr_l2p_cntrl_block->rxBuf == (char *)0)
            {
                /*
                ** Failed to allocate a Rx Buffer
                */
                return L2P_STAT_UNABLE_TO_GET_BUFFER;
            }
        }
            /* comment out the 'else block' because there is no functionality to go inside it
        else
        {
            ** Yes -- We will overwrite it automatically ... the previous message was lost!
        }
            */

        /*
        ** Pull the necessary data from the Frame Control Bytes
        */
        curr_l2p_cntrl_block->dataSize = frameCntrlBytes.dataLen; /* length of the Command Data (excl the CmdTag) */
        curr_l2p_cntrl_block->muxId = frameCntrlBytes.mux_id;     /* Sender or receipient of the message */
        curr_l2p_cntrl_block->cmdTagReceived = 0;
    }

    /*
    ** if necessary, read the cmd Tag from the Buffer
    */
    if ((frameCntrlBytes.frame_type == L2P_FT_COMPLETE_FRAME) ||
         (frameCntrlBytes.frame_type == L2P_FT_START_FRAME))
    {
        curr_l2p_cntrl_block->bytesToBeRead = sizeof(curr_l2p_cntrl_block->receivedCmdTag);
        curr_l2p_cntrl_block->dataToBeRead = (char *)&curr_l2p_cntrl_block->receivedCmdTag;

        while ((rxReadPtr != (char *)0) &&                  /* There is still data in the buffer segement */
                   (!curr_l2p_cntrl_block->cmdTagReceived)) /* The command Tag has not completed */
        {
            rxReadPtr = L2P_read_data_from_seg(rxReadPtr, curr_l2p_cntrl_block->dataToBeRead,
                                               curr_l2p_cntrl_block->bytesToBeRead,
                                               remainingSegSize, &bytesRead);
            curr_l2p_cntrl_block->bytesToBeRead -= bytesRead;
            remainingSegSize -= bytesRead;
            curr_l2p_cntrl_block->dataToBeRead += bytesRead;

            if ((rxReadPtr == (char *)0) && (totalSegSize < totalsize) && (curr_l2p_cntrl_block->bytesToBeRead != (U16)0))
            {
                /*
                ** We have reached the end of the current Segment, so get the next.
                */
                curRxSeg = curr_l2p_cntrl_block->callbacks.Get_Next_Seg_Cb(batId, seg_hdr_ptr, &seg_hdr_ptr,  &segSize);
                rxReadPtr = curRxSeg;
                totalSegSize +=segSize;
                remainingSegSize = segSize;
            }

            /*
            ** If the Command Tag has been completed set a flag
            */
            if (curr_l2p_cntrl_block->bytesToBeRead == (U16)0)
            {
                curr_l2p_cntrl_block->cmdTagReceived = 1;
                curr_l2p_cntrl_block->dataToBeRead = (char *)curr_l2p_cntrl_block->rxBuf;    /* Next Write the AT Command Data */
                curr_l2p_cntrl_block->bytesToBeRead = curr_l2p_cntrl_block->dataSize;
            }
        }
    }

    /*
    ** if txWritePtr is NULL at this point it is because a buffer has been completed ... drop out and
    ** wait for the next buffer which will contain the next frame.
    */
    while ((curr_l2p_cntrl_block->bytesToBeRead > 0) && /* There is still data to read */
           (rxReadPtr != (char *)0))                    /* There is still data in the buffer segement */
    {
        rxReadPtr = L2P_read_data_from_seg(rxReadPtr, curr_l2p_cntrl_block->dataToBeRead,
                                           curr_l2p_cntrl_block->bytesToBeRead,
                                           remainingSegSize, &bytesRead);
        curr_l2p_cntrl_block->bytesToBeRead -= bytesRead;
        remainingSegSize -= bytesRead;
        curr_l2p_cntrl_block->dataToBeRead += bytesRead;

        if ((rxReadPtr == (char *)0) && (totalSegSize < totalsize) && (curr_l2p_cntrl_block->bytesToBeRead != (U16)0))

        {
            /*
            ** We have reached the end of the current Segment, so get the next.
            */
            curRxSeg = curr_l2p_cntrl_block->callbacks.Get_Next_Seg_Cb(batId, seg_hdr_ptr, &seg_hdr_ptr,  &segSize);
            rxReadPtr = curRxSeg;
            totalSegSize +=segSize;
            remainingSegSize = segSize;
        }

        if ((curr_l2p_cntrl_block->bytesToBeRead == (U16)0) &&  /* We've finished writing the data */
             (!curr_l2p_cntrl_block->cmdTagReceived))           /* but that data was the Command Tag  */
        {
            /*
            ** now we have to write the AT Command Data
            */
            curr_l2p_cntrl_block->cmdTagReceived = 1;
            curr_l2p_cntrl_block->dataToBeRead = (char *)curr_l2p_cntrl_block->rxBuf;    /* Next Write the AT Command Data */
            curr_l2p_cntrl_block->bytesToBeRead = curr_l2p_cntrl_block->dataSize;
        }
    }

    if (curr_l2p_cntrl_block->bytesToBeRead == (U16)0)
    {
        /*
        ** We have built a complete message ... send it to the user .. 
        */
        curr_l2p_cntrl_block->callbacks.Msg_Rxd_Cb(batId, curr_l2p_cntrl_block->muxId,
                                                    curr_l2p_cntrl_block->receivedCmdTag,
                                                    curr_l2p_cntrl_block->rxBuf,
                                                    curr_l2p_cntrl_block->dataSize);

        /*
        ** reset the Rx values
        */
        curr_l2p_cntrl_block->rxBuf = (char *)0;
        curr_l2p_cntrl_block->muxId = 0;
        curr_l2p_cntrl_block->cmdTagReceived = 0;
        curr_l2p_cntrl_block->receivedCmdTag = 0;
        curr_l2p_cntrl_block->dataSize = 0;
        curr_l2p_cntrl_block->dataToBeRead = (char *)0;
    }
        
    return L2P_STAT_SUCCESS;
}


/*
+--------------------------------------------------------------------+
| PROJECT: BAT L2P          MODULE:   L2P                            |
| STATE  : code             ROUTINE:  L2P_write_data_to_seg          |
+--------------------------------------------------------------------+

   PURPOSE : L2P_write_data_to_seg is a local function to write a piece of data into a buffer segment. The buffer
             segment may, or may not be big enough to hold the entire piece of data. If the segment is full when
             the data has been written then the function will return a NULL pointer, otherwise it will return a pointer
             to the next location to receive data.
             The following is a list of the parameters, and their uses:

             writePtr        : pointer to the start location to write the data to
             dataPtr         : pointer to the data to be written
             dataSize        : length of the data in dataPtr
             remain_seg_size : The size of remaining space in the buffer segment
             bytesWritten    : output parameter in order to return the actual number of bytes written into the segment.

*/
static char *L2P_write_data_to_seg(char *writePtr, char *dataPtr, U16 dataSize, U16 remain_seg_size,
                                                               U16 *bytesWritten)
{
    char *retPtr;

    if (remain_seg_size <= dataSize)
    {
        retPtr = (char *)0;
        *bytesWritten = remain_seg_size;
    }
    else
    {
        retPtr = writePtr + dataSize;
        *bytesWritten = dataSize;
    }

    memcpy(writePtr, dataPtr, *bytesWritten);

    return retPtr;
}


/*
+--------------------------------------------------------------------+
| PROJECT: BAT L2P          MODULE:   L2P                            |
| STATE  : code             ROUTINE:  L2P_write_data_to_seg          |
+--------------------------------------------------------------------+

   PURPOSE : L2P_read_data_from_seg is a local function to read a piece of data from a buffer segment, and write
             it into a data structure (pointed to by writePtr). The buffer segment may, or may not be big enough to
             hold the entire piece of data. If the end of the segment is reached when the data has been read then
             the function will return a NULL pointer, otherwise it will return a pointer to the next location to read data
             from. The following is a list of the parameters, and their uses:

             readPtr         : pointer to the position in the segment to read from
             writePtr        : pointer to the start location to write the data to
             dataSize        : length of the data to be read from the segment
             remain_seg_size : The size of remaining space in the buffer segment
             bytesWritten    : output parameter in order to return the actual number of bytes written.

*/
static char *L2P_read_data_from_seg(char *readPtr, char *writePtr, U16 dataSize, U16 remain_seg_size,
                                                                    U16 *bytesWritten)
{
    char *retPtr;

    if (remain_seg_size <= dataSize)
    {
        retPtr = (char *)0;
        *bytesWritten = remain_seg_size;
    }
    else
    {
        retPtr = readPtr + dataSize;
        *bytesWritten = dataSize;
    }

    memcpy(writePtr, readPtr, *bytesWritten);

    return retPtr;
}

