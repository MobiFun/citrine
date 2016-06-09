/*
 * SIM.C
 *
 * Pole Star SIM
 *                 
 * Target : ARM
 *
 *
 * SIM card driver. This module contents all functions 
 * included in specifications GSM 11.11 V4.10
 *
 *
 * Copyright (c) Texas Instruments 1995-1997
 *
 */
                                        
#define SIM_C   1

#include "../include/config.h"
#include "../include/sys_types.h"

#include "mem.h"

#if (CHIPSET == 12)
    #include "sys_inth.h"
#else
    #include "iq.h"
#endif
#include "sim.h"
#include <string.h>
#include "armio.h"
#include "../L1/cust0/ind_os.h"
#include "abb+spi/abb.h"                 //controls level shifter of ABB


//current voltage mode 3V or 5V, or 1.8V
SYS_UWORD8            CurrentVolt;



#ifdef SIM_DEBUG_TRACE

#ifdef SIM_RETRY
/* one byte more to trace the number of retry for each functions */
#define SIM_DBG_NULL 5
#else
/* size of buffer tracing the reception of NULL byte */
#define SIM_DBG_NULL 4
#endif

/* working buffer for NULL BYTE and number of RETRY */
SYS_UWORD8  SIM_dbg_null[SIM_DBG_NULL];
/* size of buffer tracing the chronology of calls */
#define SIM_DBG_CMD 7500
/* working buffer for chronology calls */
SYS_UWORD8  SIM_dbg_cmd[SIM_DBG_CMD];
/* index for positionning in working buffer for chronology calls */
SYS_UWORD16  SIM_dbg_cmd_cmpt;       
/* working variable to calculate the TDMA ecart */
SYS_UWORD16 SIM_dbg_tdma_diff;
/* working variable to store the maximum TDMA frame between two characters */
SYS_UWORD16 SIM_dbg_max_interchardelay;
/* working variable used in each L2/L3 access function */
SYS_UWORD8 SIM_dbg_tmp[10];
/* internal function due to factorization of use of traces */
void SIM_dbg_write_trace(SYS_UWORD8 *ptr, SYS_UWORD16 len);

#endif

#ifdef SIM_RETRY
/* number of retry */
#define	NUM_SIM_RETRIES	10
/* Add variables to support sim retry */
SYS_UWORD8 SimRetries;
#endif


/*
 * Low level routines  : mapped to hardware
 *    SIM_WriteBuffer
 *    SIM_Command
 *    SIM_Reset
 *    
 */


/*
 * SIM_WriteBuffer
 *
 * Write n bytes to SIM card in interrupt mode:
 *   return the line, write first byte and let interrupt handler do the rest
 * return the line, write first byte and let interrupt handler do the rest
 *
 * Parameters : 
 *   SIM_PORT *p : buffer for received chars
 *   offset      : starting point for reading data.
 *   n           : number of chars to read.
 */
void SIM_WriteBuffer(SIM_PORT *p, SYS_UWORD16 offset, SYS_UWORD16 n)
{
   unsigned volatile i;

   // Set write direction
   p->conf1 |= SIM_CONF1_TXRX;
   p->c->conf1 = p->conf1;

   p->SWcount  = 0;
   p->rx_index = 0;
   p->expected_data = 0;

   p->xOut = p->xbuf + offset;
   p->xIn  = p->xbuf + offset + n;

   if ((p->xIn - p->xOut) == 1)        //if only one char is transmitted
   {                                   //need to wait a minimum of 1 ETU
        ind_os_sleep (1);                  //for IO line to stay in TX mode
   } 
   // Write first byte 
   p->c->tx = *(p->xOut);              // transmit

   if ((p->xIn - p->xOut) == 1)        //if only one char to transmit
   {                                   // return the direction to rx
        p->conf1 &= ~SIM_CONF1_TXRX;   //to be able to receive ACK char
        p->c->conf1 = p->conf1;        
   }
}

/*
 * SIM_Result
 *    
 * Parameters : SIM port, buffer for received chars, pointer to receive size
 * 
 * Return the result code (SW1/SW2) at the end of the string
 */
SYS_UWORD16 SIM_Result(SIM_PORT *p, SYS_UWORD8 *rP, SYS_UWORD16 *lenP, SYS_UWORD8 offset)
{
   SYS_UWORD8 sw1, sw2;
   SYS_UWORD8 verdict;
   SYS_UWORD16 len;
   
   // Check if all characters were transmitted
   if (p->xIn - 1 != p->xOut)
      return (SIM_ERR_XMIT);
   
   len = p->rx_index;
   *lenP = len - offset;
   if ((*lenP == 0) && (p->apdu_ans_length == 256))
	*lenP = 256;

   if (p->expected_data == 256)
   {
       verdict = SIM_Memcpy(rP, ((p->rbuf) + offset), 256 - offset);
       if (verdict != 0)
       { 
           return (verdict);
       }
   }
   else if ((len != 0) && (len >= offset))
   {
       verdict = SIM_Memcpy(rP, ((p->rbuf) + offset), len - offset);
       if (verdict != 0)
       {
           return (verdict);
       }
   }
   
   // change to remove SW1 and SW2 bytes from the receive buffer of data 
   sw1 = p->rSW12[0];
   sw2 = p->rSW12[1];

   return((sw1 << 8) | sw2);
}

/*
 * SIM_Command_base
 *
 * Perform a command with the SIM T=0 protocol
 *
 * Arguments : pointer to SIM port structure
 *             number of characters above 5
 *             expected command time in TDMA
 *
 * Returns an error code :
 *             SIM_ERR_READ : no answer from the card to a command
 *             SIM_ERR_LEN  : the answer is not corresponding to a
 *                            correct answer of T=0 protocol
 * 06/11/2002	JYT
 *		Modified to be base command function. New SIM_Command() created to call it 
 * 		with wrapper. Created to manage retries on Internals errors of the driver.
 */

SYS_UWORD16 SIM_Command_Base(SIM_PORT *p, SYS_UWORD16 n, SYS_UWORD8 *dP,
			     SYS_UWORD16 *lP)
{
    SYS_UWORD16  res;
    SYS_UWORD8    err;
    SYS_UWORD8    ins;
    SYS_UWORD8    nack;
    SYS_UWORD8    nack1;
    SYS_UWORD16  offset;

    if (SIM_sleep_status == SIM_SLEEP_DESACT)
    {   //freeze the timer
        status_os_sim = NU_Control_Timer (&SIM_timer,  NU_DISABLE_TIMER);
    }
    else if (SIM_sleep_status == SIM_SLEEP_ACT)
    {   //get out sleep mode
        status_os_sim = NU_Control_Timer (&SIM_timer,  NU_DISABLE_TIMER);
        SIM_SleepMode_Out (p);   //get up SIM card of sleep mode
				 // before executing the command
    }

    SIM_WriteBuffer(p, 0, 5);

    //adaptative driver

    if (n > 0)          //need to send data to the card, TX mode
    {
        offset = 0;
        // protocol T=0 returns a acknowledge char which is
        //     ins or (ins+1)   : transmit the rest of the command in one time
        //     ~ins or ~(ins+1) : transmit the rest of the command char by char
        ins   = p->xbuf[1] & p->hw_mask;
        nack  = (~p->xbuf[1]) & p->hw_mask;;

        p->moderx = 6;  //mode of wait for ACK char

NEXT_CHAR_PROC:

        if (err = SIM_Waitforchars(p, p->etu9600))
        {
            if ((SIM_sleep_status == SIM_SLEEP_DESACT) ||
		(SIM_sleep_status == SIM_SLEEP_ACT))
            {   //enable to count 2.5s before entering in sleep mode
                status_os_sim = NU_Reset_Timer (&SIM_timer, SIM_SleepMode_In,
                                                SIM_SLEEP_WAITING_TIME,
                                                0, NU_ENABLE_TIMER);
            }
            return (err);
        }

        if (p->moderx == 5)     //return SW1/SW2
        {
            res = SIM_Result(p, dP, lP, 0);

            if ((SIM_sleep_status == SIM_SLEEP_DESACT) ||
		(SIM_sleep_status == SIM_SLEEP_ACT))
            {   //enable to count 2.5s before entering in sleep mode
                status_os_sim = NU_Reset_Timer (&SIM_timer, SIM_SleepMode_In,
                                                SIM_SLEEP_WAITING_TIME,
                                                0, NU_ENABLE_TIMER);
            }

            return(res);
        }
        else if ((p->ack & p->hw_mask) == ins)
        {
        // Write the rest of the command if needed
        // if more than 5 characters, the ack character will disappear

            SIM_WriteBuffer(p, 5 + offset, n - offset);
        }
        // special transmission mode if ACK = ~INS or ~(INS + 1).
        // refer to ISO/CEI 7816-3 [8.2.2]
        // need to send char by char
        else if ((p->ack & p->hw_mask) == nack)
        {
            SIM_WriteBuffer(p, 5 + offset, 1);
            offset++;
            goto NEXT_CHAR_PROC;
        }

        p->moderx = 5;
        if (err = SIM_Waitforchars (p, p->etu9600))  //wait SW1 / SW2
        {
            if ((SIM_sleep_status == SIM_SLEEP_DESACT) ||
		(SIM_sleep_status == SIM_SLEEP_ACT))
            {   //enable to count 2.5s before entering in sleep mode
                status_os_sim = NU_Reset_Timer (&SIM_timer, SIM_SleepMode_In,
                                                SIM_SLEEP_WAITING_TIME,
                                                0, NU_ENABLE_TIMER);
            }
            return (err);
        }

    }
    else                //receive mode
    {
        if (err = SIM_WaitReception(p))  //wait for next procedure character
        {
            if ((SIM_sleep_status == SIM_SLEEP_DESACT) ||
		(SIM_sleep_status == SIM_SLEEP_ACT))
            {   //enable to count 2.5s before entering in sleep mode
                status_os_sim = NU_Reset_Timer (&SIM_timer, SIM_SleepMode_In,
                                                SIM_SLEEP_WAITING_TIME,
                                                0, NU_ENABLE_TIMER);
            }
            return (err);
        }
    }

    res = SIM_Result(p, dP, lP, 0);

    if ((SIM_sleep_status == SIM_SLEEP_DESACT) ||
	(SIM_sleep_status == SIM_SLEEP_ACT))
    {   //enable to count 2.5s before entering in sleep mode
        status_os_sim = NU_Reset_Timer (&SIM_timer, SIM_SleepMode_In,
                                                SIM_SLEEP_WAITING_TIME,
                                                0, NU_ENABLE_TIMER);
    }

    return(res);
}


/* Main function to manage the retry mechanism */
SYS_UWORD16 SIM_Command(SIM_PORT *p, SYS_UWORD16 n, SYS_UWORD8 *dP,
			SYS_UWORD16 *lP)
{
    int res;

#ifdef SIM_DEBUG_TRACE
    memset(SIM_dbg_null, 0x00, SIM_DBG_NULL);
    SIM_dbg_tdma_diff = 0;
#endif

    // Issue initial SIM_Command() call
    res = SIM_Command_Base(p, n, dP, lP);
    /* Change from to 10 to 15 for specific SIM card (Racal) */

#ifdef SIM_RETRY
    // While there is an error then retry NUM_SIM_RETRIES times
    while ((res & 0xFF00) == 0)	{	// Reissue command
	p->errorSIM = 0;
	if(++SimRetries > NUM_SIM_RETRIES) {	// return special retry failure
	    res = SIM_ERR_RETRY_FAILURE;
	    break;
	}
        res = SIM_Command_Base(p, n, dP, lP);
    }

#ifdef SIM_DEBUG_TRACE
    SIM_dbg_null[SIM_DBG_NULL-1] = SimRetries;
#endif

    SimRetries = 0;
#endif

    return(res);
}

/*
 * SIM_ByteReverse
 *
 * Reverse a byte, both up/down (1 <> 0) and left/right (0001 <> 1000)
 * 
 */
SYS_UWORD8 SIM_ByteReverse(SYS_UWORD8 b)
{
    SYS_UWORD8 bh, bl;
    int i;
    const SYS_UWORD8 Reverse[] = {0x0, 0x8, 0x4, 0xC, 0x2, 0xA, 0x6, 0xE, 
                                    0x1, 0x9, 0x5, 0xD, 0x3, 0xB, 0x7, 0xF };

    // Up/Down
    b = ~ b;

    // left / right (by nibble)
    bh = (b >> 4) & 0xF;
    bl = b & 0xF;
      
    b = (Reverse[bl]) << 4 | Reverse[bh];
    return(b);
}

/*
 * SIM_TxParityErrors
 *
 * return number of transmit parity errors occured since the last reset
 * of the SIM card
 *
 */
SYS_UWORD16 SIM_TxParityErrors(void)
{
    SIM_PORT *p;

    p= &(Sim[0]);

    return(p->txParityErr);
}


/*
 * SIM_Reset
 *
 * Reset SIM card 
 * Call-back SIM insert if successful
 * or SIM remove otherwise
 *
 * Returns 0 for success, or
 *         SIM_ERR_NOCARD : no card
 *         SIM_ERR_NATR   : no answer to reset
 *         SIM_ERR_NOINT  : no 
 *         SIM_ERR_READ   : unknown data return by the card
 *         SIM_ERR_CARDREJECT : card not accepted
 *
 * 29/01/02, JYT, adding of low voltage managment for IOTA device
 * 06/10/03, JYT, Split of Reset to handle Restart
 */ 
SYS_UWORD16 SIM_Reset(SIM_CARD *cP)
{
    return(SIM_Reset_Restart_Internal(cP, 1));
}

/*
 * SIM_Restart
 *
 * Restart SIM card 
 *
 * Returns 0 for success, or
 *         SIM_ERR_NOCARD : no card
 *         SIM_ERR_NATR   : no answer to reset
 *         SIM_ERR_NOINT  : no 
 *         SIM_ERR_READ   : unknown data return by the card
 *         SIM_ERR_CARDREJECT : card not accepted
 *
 * 06/10/03, JYT, Split of Reset to handle Restart
 */ 
SYS_UWORD16 SIM_Restart(SIM_CARD *cP)
{
    return(SIM_Reset_Restart_Internal(cP, 0));
}


/*
 * SIM_Reset_Restart_Internal
 *
 * Reset SIM card 
 * Call-back SIM insert if successful
 * or SIM remove otherwise
 *							  
 * Returns 0 for success, or
 *         SIM_ERR_NOCARD : no card
 *         SIM_ERR_NATR   : no answer to reset
 *         SIM_ERR_NOINT  : no 
 *         SIM_ERR_READ   : unknown data return by the card
 *         SIM_ERR_CARDREJECT : card not accepted
 *
 * 29/01/02, JYT, adding of low voltage managment for IOTA device
 * 06/10/03, JYT, Split of Reset to handle Restart, ResetFlag added.
 */ 
SYS_UWORD16 SIM_Reset_Restart_Internal(SIM_CARD *cP, SYS_UWORD8 ResetFlag)
{
    SIM_PORT        *p;
    unsigned int    ATR_Attempt;
    SYS_UWORD8      BackValue;
    SYS_UWORD8      Result_ATR;

#ifdef SIM_DEBUG_TRACE
    memset(SIM_dbg_null, 0x00, SIM_DBG_NULL);
    SIM_dbg_cmd_cmpt = 0;
    memset(SIM_dbg_cmd, 0x00, SIM_DBG_CMD);
#endif

    // Initialize pointers 
    p = &(Sim[0]);

// begin of JYT modifications
    if ( (BackValue = SIM_StartVolt(ResetFlag)) != SIM_OK)
       return((SYS_UWORD16)BackValue);
// end of JYT modifications

    p->etu9600    = 867; // old = 239, increase of 363%
    p->etu400     = 20;
    p->hw_mask    = MASK_INS;

    ATR_Attempt      = 1;

COLD_RESET:

    p->SWcount       = 0;
    p->Freq_Algo     = 0;
    p->PTS_Try       = 0;            //use to calculate how many PTS try were already done
    
    // Initialize pointers 
    p->xIn   = p->xOut = p->xbuf;
    p->rx_index        = 0;
    p->errorSIM        = 0;
    p->moderx          = 0;
    p->null_received   = 0;

    BackValue = SIM_ManualStart(p);
    if (BackValue != 0)
        return ((SYS_UWORD16)BackValue);
    
      
    p->c->conf1 = p->conf1 &= ~SIM_CONF1_BYPASS;      //switch to automatic mode             

//#else //SW_WRK_AROUND_H_S == 0 // Automatic procedure -> fails with test 27.11.2.1
//
//                                  // Mask all interrupts
//    p->c->maskit = SIM_MASK_NATR | SIM_MASK_WT | SIM_MASK_OV | 
//                   SIM_MASK_TX | SIM_MASK_RX | SIM_MASK_CD;
//
//
//   IQ_Unmask (IQ_SIM);           // Unmask interrupt controller
//
//
//    p->c->cmd = (p->c->cmd & MASK_CMD) | SIM_CMD_STOP;
//    ind_os_sleep(1);
//                      
//    p->c->cmd = (p->c->cmd & MASK_CMD) | SIM_CMD_SWRST;    // Set START bit and wait a while
//    ind_os_sleep(1);
//                                  // Unmask all sources of interrupts except WT, OV, and NATR
//    p->c->maskit = SIM_MASK_OV | SIM_MASK_WT | SIM_MASK_NATR;
//
//                                  // Set Configuration bits
//    p->c->conf1     = p->conf1 = SIM_CONF1_SRSTLEV | SIM_CONF1_SCLKEN;
//    p->c->conf2  = 0x0940;
//
//        //enable VCC
//        #if(ANALOG == 1)
//          SPIABB_wa_VRPC (SPIRead_ABB_Register (PAGE1,VRPCCTRL1) | MODE_ENA_VCC);
//        #elif(ANALOG == 2)
//          SPIABB_wa_VRPC (SPIRead_ABB_Register (PAGE1,VRPCSIM) | MODE_ENA_VCC);
//        #endif
//    p->c->cmd = (p->c->cmd & MASK_CMD) | SIM_CMD_START;
//
//#endif 

/*-----------------------------------------------------------*/

    while (p->PTS_Try != 5)
    {
        while (ATR_Attempt != 0)
        {
            // Treat ATR response
            BackValue = SIM_ATRdynamictreatement (p, cP);

            if (BackValue == SIM_ERR_NOCARD)
            {
                 SIM_PowerOff ();
                 return (SIM_ERR_NOCARD);
            }
            // ATR received but wrong characters value
            // Comply with Test 27.11.2.4.5 and Test 27.11.1.3
            else if (BackValue == SIM_ERR_CARDREJECT)            
                {
                if (ATR_Attempt >= 3)
            {
                    SIM_PowerOff ();
                    return ((SYS_UWORD16)BackValue);
            }

                ATR_Attempt++;
                SIM_WARMReset(p);    // assert a reset during at least 400 ETU
        }
            else if (BackValue != 0) //SIM_ERR_WAIT           
        {
                if (ATR_Attempt == 3)
                {    // switch to 5V (ANALOG1) or 3V (ANALOG2) if card send wrong ATR 3 consecutive times
                     // Apply 3 consecutive resets at 5V (ANALOG1) or 3V (ANALOG2)
                     // fix prb for old chinese card not GSM compliant   

                     if ((BackValue = SIM_SwitchVolt(ResetFlag)) != SIM_OK) 
                     {
                     	// SIM cannot be supplied at 3V (ANALOG2), because of an Hardware failure
                     	SIM_PowerOff ();
                        return((SYS_UWORD16)BackValue);
                     }   
                     
                     ATR_Attempt++;  
                     goto COLD_RESET;  
                }
                if (ATR_Attempt >= 6) 
                {
                    SIM_PowerOff ();
                    return ((SYS_UWORD16)BackValue);
                }

                ATR_Attempt++;
                SIM_WARMReset(p);    // assert a reset during at least 400 ETU
            }
            
            else
            {
                ATR_Attempt = 0;    
            }
        }
/*-----------------------------------------------------------*/
// PTS procedure
        BackValue = SIM_PTSprocedure(cP,p);      //assert PTS if needed
//        need upgrade with FIFO use to avoid CPU overloading
              
        if (BackValue)
        {
            if (BackValue == SIM_ERR_CARDREJECT)
            {
                SIM_PowerOff ();      //must be done by protocol stack
                return (SIM_ERR_CARDREJECT);
            }
            if (p->PTS_Try <= 4)     //else error treatement  
            {
                SIM_WARMReset(p);    // assert a reset during at least 400 ETU
            }            
        }
        else
        {
            p->PTS_Try = 5;
        }
    }
/*-----------------------------------------------------------*/

  //interpret SIM coding concerning SIM supply voltage

    if (SIM_GetFileCharacteristics(p))
    {
#if ((SIM_TYPE == SIM_TYPE_3V) || (SIM_TYPE == SIM_TYPE_1_8V))
        SIM_PowerOff();      // Needed for tests 27.17.1.5.1 and 27.17.1.5.5
#endif
        return (SIM_ERR_READ);
    }

   // JYT, certainly unused because of previous test
    if(p->errorSIM)
    {
      return(p->errorSIM);
    }

    if ((p->FileC & SIM_MASK_INFO_VOLT) == SIM_5V)
    {
#if ((SIM_TYPE == SIM_TYPE_3V ) || (SIM_TYPE == SIM_TYPE_1_8_3V) || (SIM_TYPE == SIM_TYPE_1_8V))
        SIM_PowerOff ();                 // required by ETSI if 5V only card is detected and 3V only ME chosen
        return (SIM_ERR_CARDREJECT);     // Test 27.17.1.5.2    
#elif (SIM_TYPE == SIM_TYPE_3_5V)  
        if (CurrentVolt == SIM_3V) //if 5V only SIM present -> the ME may switch to 5V operation
        {
            if ((BackValue = SIM_SwitchVolt(ResetFlag)) != SIM_OK) // switch to 5V
            {
               SIM_PowerOff ();
               return ((SYS_UWORD16)BackValue);
            }	
            ATR_Attempt      = 1;
            goto COLD_RESET;            // Test 27.17.1.5.3 
        }
#endif              
    }
    else 
    {
       if ((p->FileC & SIM_MASK_INFO_VOLT) == SIM_3V)
       {
#if (SIM_TYPE == SIM_TYPE_1_8V)
        SIM_PowerOff ();                 // required by ETSI if 3V only card is detected and 1.8V only ME chosen
        return (SIM_ERR_CARDREJECT);     // Test 27.17.1.5.2    
#elif (SIM_TYPE == SIM_TYPE_1_8_3V)  
        if (CurrentVolt == SIM_1_8V) //if 3V only SIM present -> the ME may switch to 3V operation
        {
            if ((BackValue = SIM_SwitchVolt(ResetFlag)) != SIM_OK) // switch to 3V
            {
               SIM_PowerOff ();
               return ((SYS_UWORD16)BackValue);
            }	
            ATR_Attempt      = 1;
            goto COLD_RESET;            // Test 27.17.1.5.3 
        }
#endif                     	
       }
       else 
       {
          if ((p->FileC & SIM_MASK_INFO_VOLT) == SIM_1_8V)
          {
#if (SIM_TYPE == SIM_TYPE_5V)
              SIM_PowerOff ();                 // required by ETSI if 5V only card is detected and 3V only ME chosen
              return (SIM_ERR_CARDREJECT);     // Test 27.17.1.5.2    
#endif                        	
          }
          else 
          {
       	      // future class of sim card voltage !!!!!! never use it
       	      SIM_PowerOff ();                 // Rec. 11.18
              return (SIM_ERR_CARDREJECT);     
          }          
       }
    }      

    SIM_Interpret_FileCharacteristics(p);      //find which frequency (13/4 or 13/8 Mhz) 

    if(p->errorSIM)
    {
      return(p->errorSIM);
    }

    status_os_sim = NU_Control_Timer (&SIM_timer,  NU_ENABLE_TIMER);
    //enable starting of the os timer for sleep mode
    if (ResetFlag) {
    if (p->InsertFunc != NULL)
       (p->InsertFunc)(cP);   
    }

    return(0);
}

/* SIM manual start
*
*  purpose : manage manual start of the SIM interface
*  input   : pointer on sim structure SIM_PORT
*  output  : none
*/

SYS_UWORD16 SIM_ManualStart (SIM_PORT *p)
{
    volatile int             i;

//!!
    p->c->conf1 = p->conf1 = 0x8004;   //set conf1 to automatic mode SIO low
    //enable sim interface clock module
    p->c->cmd = SIM_CMD_CLKEN;    

//#if (SW_WRK_AROUND_H_S == 1)

    // Mask all interrupts
    p->c->maskit = SIM_MASK_NATR | SIM_MASK_WT | SIM_MASK_OV | 
           SIM_MASK_TX | SIM_MASK_RX | SIM_MASK_CD;

    // Unmask interrupt controller
    IQ_Unmask (IQ_SIM);

    p->c->cmd = (p->c->cmd & MASK_CMD) | SIM_CMD_STOP;
    ind_os_sleep (4);                           //wait 5 TDMA due to SVCC falling down duration 

    p->c->cmd = (p->c->cmd & MASK_CMD) | SIM_CMD_SWRST;
    ind_os_sleep (1);                           //wait 5 TDMA due to SVCC falling down duration 


    p->c->conf2  = 0x0940;
    
    i = p->c->it;
    // Unmask all sources of interrupts except WT and OV and NATR
    p->c->maskit = SIM_MASK_WT | SIM_MASK_OV | SIM_MASK_NATR;


    //enter in manual mode to start the ATR sequence
    p->c->conf1 = p->conf1 |= SIM_CONF1_BYPASS;
    ind_os_sleep(1);

    p->c->conf1 = p->conf1 |= SIM_CONF1_SVCCLEV;
    ind_os_sleep(1);

    #if(ANALOG == 1) 
      //set OMEGA to 3V mode
      //enable VCC
      ABB_wa_VRPC (ABB_Read_Register_on_page(PAGE1,VRPCCTRL1) | MODE_ENA_SIMLDOEN);
      ind_os_sleep(1);
      ABB_wa_VRPC (ABB_Read_Register_on_page(PAGE1,VRPCCTRL1) | MODE_ENA_SIMEN);
      ind_os_sleep(1);
    #elif(ANALOG == 2) 
      //set IOTA to 3V mode
      //enable VCC
      ABB_wa_VRPC (ABB_Read_Register_on_page(PAGE1,VRPCSIM) | MODE_ENA_SIMEN);
      ind_os_sleep(1);
    #elif(ANALOG == 3)
      //set SYREN to 3V mode
      //enable VCC
      ABB_wa_VRPC (ABB_Read_Register_on_page(PAGE1,VRPCSIMR) | MODE_ENA_SIMEN);
      ind_os_sleep(1);
    #endif

    p->c->conf1 = p->conf1 &= ~SIM_CONF1_SIOLOW;

    ind_os_sleep(1);

    p->c->conf1 = p->conf1 |= SIM_CONF1_SCLKEN;

    p->c->conf1 = p->conf1 &= ~SIM_CONF1_TXRX; //set to receive mode


    if(p->errorSIM)                       //check for card detection
    {
        return(p->errorSIM);
    }

    i = 0;
    while ((p->rx_index == 0) && (i < 3))       //wait 40000*Tsclk
    {
        ind_os_sleep (1);
        i++;
    }        
      
    if ((p->rx_index == 0) && (i >= 3))         //external reset card ATR treatement
    {
        i = 0;

        p->c->conf1 = p->conf1 |= SIM_CONF1_SRSTLEV;//set reset level to high level

        while ((p->rx_index == 0) && (i < 3))   //wait 40000*Tsclk
        {
            ind_os_sleep (1);
            i++;
        }
    }

    return (0);
}
 
/* SIM manual stop
*
*  purpose : manage manual start of the SIM interface
*  input   : pointer on sim structure SIM_PORT
*  output  : none
*/

void SIM_ManualStop (SIM_PORT *p)
{
// to write
}

/* Power off SIM == SIM_CMD_STOP 
*  input  : none
*  output : none
*/

void SIM_PowerOff(void)
{
    SIM_PORT *p;
    volatile SYS_UWORD16 cmd;

    // Initialize pointers 
    p = &(Sim[0]);

    // Reset and wait a while
    cmd = p->c->cmd;
    p->c->cmd = (cmd & MASK_CMD) | SIM_CMD_STOP;

    ind_os_sleep(5);    //wait for falling of SIM signals (RESET/CLK/IO)

    #if(ANALOG == 1) 
      //disable VCC : disable level shifter then SVDD
      ABB_wa_VRPC (ABB_Read_Register_on_page(PAGE1,VRPCCTRL1) & MODE_DIS_SIMEN);
      ABB_wa_VRPC (ABB_Read_Register_on_page(PAGE1,VRPCCTRL1) & MODE_DIS_SIMLDOEN);
    #elif(ANALOG == 2) 
      //disable VCC : disable level shifter then SVDD
      ABB_wa_VRPC (ABB_Read_Register_on_page(PAGE1,VRPCSIM) & MODE_DIS_SIMEN);
      ABB_wa_VRPC (ABB_Read_Register_on_page(PAGE1,VRPCSIM) & MODE_DIS_SIMLDOEN);
    #elif(ANALOG == 3)
      //disable VCC : disable level shifter then SVDD
      ABB_wa_VRPC (ABB_Read_Register_on_page(PAGE1,VRPCSIMR) & MODE_DIS_SIMEN);
      ABB_wa_VRPC (ABB_Read_Register_on_page(PAGE1,VRPCSIMR) & MODE_DIS_SIMLDOEN);
    #endif

    ind_os_sleep(10);    //wait for falling of VCC commanf by ABB

    p->c->cmd = 0x0000;   //disable clock of sim module

    if ((SIM_sleep_status == SIM_SLEEP_DESACT) || (SIM_sleep_status == SIM_SLEEP_ACT))
    {   //SIM sleep timer is not more needed
        status_os_sim = NU_Delete_Timer (&SIM_timer);
    }
}


/*
 * SIM_Init
 *
 * Function for backward compatibility only
 *
 */

void SIM_Init(void (Insert(SIM_CARD *cP)), void (Remove(void)))
{
    // Call SIM Registration function.
    (void) SIM_Register (Insert, Remove);
}

/*
 * SIM_Initialize
 *
 * Initialize data structures.
 *
 */

void SIM_Initialize(void)
{
    int n;
    SIM_PORT *p;
    volatile SYS_UWORD32 dum;

    // Initialize registers 
    p = &(Sim[0]);
    p->c = (SIM_CONTROLLER *) SIM_CMD;

    p->errorSIM = 0;
    dum = (volatile SYS_UWORD32) SIM_Dummy;         // to force linking SIM32

    status_os_sim = NU_Create_Timer (&SIM_timer, "SIM_sleep_timer", &SIM_SleepMode_In,
                    0, SIM_SLEEP_WAITING_TIME, 0, NU_DISABLE_TIMER);
    //timer start only with NU_Control_Timer function
    //waiting time set to 2.3s
    SIM_sleep_status = SIM_SLEEP_NONE;
    
#ifdef SIM_RETRY
    SimRetries = 0;
#endif
}

/*
 * SIM_Register
 *
 * SIM Registration function: Initialize callback functions
 *
 * Insert(void) : pointer to the function called when a card is inserted
 * Remove(void) : pointer to the function called when the card is removed
 *
 */

SYS_UWORD16 SIM_Register(void (Insert(SIM_CARD *cP)), void (Remove(void)))
{
    SIM_PORT *p;

    // Initialize pointers 
    p = &(Sim[0]);

    p->InsertFunc = Insert;
    p->RemoveFunc = Remove;

    return (SIM_OK);
}


/*
 * High level routines : mapped to GSM 11.11 function calls
 *
 * Uses a Nucleus semaphore to ensure no simultaneous access to SIM and buffer
 *    
 * Each routine does :
 *    write command
 *    sleep long enough for the expected transmission and reception
 *    return rest code
 *
 *    SYS_UWORD8 *result :  pointer to the string return by the SIM card
 *    SYS_UWORD8 *rcvSize : size of the string return by the SIM card
 *
 *    other parameters : parameters needed by the SIM card to 
 *                       execute the function.
 *   
 */
//unsigned char SIM_flag = 0;


/*
 * SIM_Select
 *
 * Select a DF or a EF
 */
SYS_UWORD16 SIM_Select(SYS_UWORD16 id, SYS_UWORD8 *dat, SYS_UWORD16 *rcvSize)
{
    SIM_PORT *p;
    int res;

    p = &(Sim[0]);

    p->xbuf[0] = GSM_CLASS;
    p->xbuf[1] = SIM_SELECT; 
    p->xbuf[2] = 0;
    p->xbuf[3] = 0;
    p->xbuf[4] = 2; 
    p->xbuf[5] = id >> 8;       // high byte
    p->xbuf[6] = id & 0xFF;     // low byte

    res = SIM_Command(p, 2, dat, rcvSize);   
    /* Change from to 10 to 15 for specific SIM card (Racal) */

//	if (id == 0x6F07)
//		SIM_flag = 1;

#ifdef SIM_DEBUG_TRACE
    SIM_dbg_write_trace((SYS_UWORD8 *)"AACMD", 5);
    SIM_dbg_write_trace(p->xbuf, 7);
    SIM_dbg_write_trace((SYS_UWORD8 *)"AAANS", 5);
    SIM_dbg_tmp[0] = (SYS_WORD8)(res>>8);
    SIM_dbg_tmp[1] = (SYS_WORD8)res;
    SIM_dbg_write_trace(SIM_dbg_tmp, 2);
    SIM_dbg_write_trace(SIM_dbg_null, SIM_DBG_NULL);
    if (SIM_dbg_tdma_diff > SIM_dbg_max_interchardelay)
        SIM_dbg_max_interchardelay = SIM_dbg_tdma_diff;
    SIM_dbg_tmp[0] = (SYS_UWORD8)(SIM_dbg_tdma_diff >> 8);
    SIM_dbg_tmp[1] = (SYS_UWORD8)(SIM_dbg_tdma_diff);
    SIM_dbg_write_trace(SIM_dbg_tmp, 2);
#endif

    return(res);
}


/*
 * SIM_Status
 *    
 *  Returns data received from card and number of bytes received
 */
SYS_UWORD16 SIM_Status(SYS_UWORD8 *dat, SYS_UWORD16 *rcvSize)
{
    SIM_PORT *p;

    short len = 0x16;            // length specified in GSM 11.11
    int res;

    p = &(Sim[0]);

    p->xbuf[0] = GSM_CLASS;
    p->xbuf[1] = SIM_STATUS;
    p->xbuf[2] = 0;
    p->xbuf[3] = 0;
    p->xbuf[4] = len; 

    res = SIM_Command(p, 0, dat, rcvSize);

#ifdef SIM_DEBUG_TRACE
    SIM_dbg_write_trace((SYS_UWORD8 *)"ABCMD", 5);
    SIM_dbg_write_trace(p->xbuf, 5);
    SIM_dbg_write_trace((SYS_UWORD8 *)"ABANS", 5);
    SIM_dbg_tmp[0] = (SYS_WORD8)(res>>8);
    SIM_dbg_tmp[1] = (SYS_WORD8)res;
    SIM_dbg_write_trace(SIM_dbg_tmp, 2);
    SIM_dbg_write_trace(SIM_dbg_null, SIM_DBG_NULL);
    if (SIM_dbg_tdma_diff > SIM_dbg_max_interchardelay)
        SIM_dbg_max_interchardelay = SIM_dbg_tdma_diff;
    SIM_dbg_tmp[0] = (SYS_UWORD8)(SIM_dbg_tdma_diff >> 8);
    SIM_dbg_tmp[1] = (SYS_UWORD8)(SIM_dbg_tdma_diff);
    SIM_dbg_write_trace(SIM_dbg_tmp, 2);
#endif

    return(res);
}

/*
 * SIM_Status_Extended
 *    
 *  Returns data received from card and number of bytes received
 *  Add extra parameter len : number of returned byte
 */
SYS_UWORD16 SIM_Status_Extended(SYS_UWORD8 *dat, SYS_UWORD16 len,
				SYS_UWORD16 *rcvSize)
{
    SIM_PORT *p;
    int res;
    SYS_UWORD16 llen = len & SIM_UWORD16_MASK;

    p = &(Sim[0]);

    p->xbuf[0] = GSM_CLASS;
    p->xbuf[1] = SIM_STATUS;
    p->xbuf[2] = 0;
    p->xbuf[3] = 0;
    p->xbuf[4] = (SYS_UWORD8)llen; 

    res = SIM_Command(p, 0, dat, rcvSize);

#ifdef SIM_DEBUG_TRACE
    SIM_dbg_write_trace((SYS_UWORD8 *)"ACCMD", 5);
    SIM_dbg_write_trace(p->xbuf, 5);
    SIM_dbg_write_trace((SYS_UWORD8 *)"ACANS", 5);
    SIM_dbg_tmp[0] = (SYS_WORD8)(res>>8);
    SIM_dbg_tmp[1] = (SYS_WORD8)res;
    SIM_dbg_write_trace(SIM_dbg_tmp, 2);
    SIM_dbg_write_trace(SIM_dbg_null, SIM_DBG_NULL);
    if (SIM_dbg_tdma_diff > SIM_dbg_max_interchardelay)
        SIM_dbg_max_interchardelay = SIM_dbg_tdma_diff;
    SIM_dbg_tmp[0] = (SYS_UWORD8)(SIM_dbg_tdma_diff >> 8);
    SIM_dbg_tmp[1] = (SYS_UWORD8)(SIM_dbg_tdma_diff);
    SIM_dbg_write_trace(SIM_dbg_tmp, 2);
#endif

    return(res);
}


/*
 * SIM_ReadBinary
 *
 * Read data from the current EF
 */
SYS_UWORD16 SIM_ReadBinary(SYS_UWORD8 *dat, SYS_UWORD16 offset,
			   SYS_UWORD16 len, SYS_UWORD16 *rcvSize)
{
    SIM_PORT *p;
    int res;
    SYS_UWORD16 llen = len & SIM_UWORD16_MASK;

    p = &(Sim[0]);

    p->xbuf[0] = GSM_CLASS;
    p->xbuf[1] = SIM_READ_BINARY;
    p->xbuf[2] = offset >> 8; 
    p->xbuf[3] = offset & 0xFF; 
    p->xbuf[4] = (SYS_UWORD8)llen; 

    res = SIM_Command(p, 0, dat, rcvSize);

//	if (SIM_flag) {
//		SIM_flag = 0;
//		dat[0] = 0x08;
//	}
		
#ifdef SIM_DEBUG_TRACE
    SIM_dbg_write_trace((SYS_UWORD8 *)"ADCMD", 5);
    SIM_dbg_write_trace(p->xbuf, 5);
    SIM_dbg_write_trace((SYS_UWORD8 *)"ADANS", 5);
    SIM_dbg_tmp[0] = (SYS_UWORD8)(*rcvSize>>8);
    SIM_dbg_tmp[1] = (SYS_UWORD8)(*rcvSize);
    SIM_dbg_write_trace(SIM_dbg_tmp, 2);
    SIM_dbg_write_trace(dat, *rcvSize);
    SIM_dbg_tmp[0] = (SYS_WORD8)(res>>8);
    SIM_dbg_tmp[1] = (SYS_WORD8)res;
    SIM_dbg_write_trace(SIM_dbg_tmp, 2);
    SIM_dbg_write_trace(SIM_dbg_null, SIM_DBG_NULL);
    if (SIM_dbg_tdma_diff > SIM_dbg_max_interchardelay)
        SIM_dbg_max_interchardelay = SIM_dbg_tdma_diff;
    SIM_dbg_tmp[0] = (SYS_UWORD8)(SIM_dbg_tdma_diff >> 8);
    SIM_dbg_tmp[1] = (SYS_UWORD8)(SIM_dbg_tdma_diff);
    SIM_dbg_write_trace(SIM_dbg_tmp, 2);
#endif

    return(res);
}

/*
 * SIM_VerifyChv
 *
 * Verify the specified CHV (chvType)
 */
SYS_UWORD16 SIM_VerifyCHV(SYS_UWORD8 *result, SYS_UWORD8 *dat,
			  SYS_UWORD8 chvType, SYS_UWORD16 *rcvSize)
{
    SIM_PORT *p;
    SYS_UWORD8 len;
    int i;
    int res;

    p = &(Sim[0]);
    len = 8;

    p->xbuf[0] = GSM_CLASS;
    p->xbuf[1] = SIM_VERIFY_CHV;
    p->xbuf[2] = 0; 
    p->xbuf[3] = chvType; 
    p->xbuf[4] = len; 
    for (i=0;i<8;i++)
    {
      p->xbuf[5+i] = *(dat+i);
    }
    res = SIM_Command(p, 8, result, rcvSize);

#ifdef SIM_DEBUG_TRACE
    SIM_dbg_write_trace((SYS_UWORD8 *)"AECMD", 5);
    SIM_dbg_write_trace(p->xbuf, len+5);
    SIM_dbg_write_trace((SYS_UWORD8 *)"AEANS", 5);
    SIM_dbg_tmp[0] = (SYS_UWORD8)(*rcvSize >> 8);
    SIM_dbg_tmp[1] = (SYS_UWORD8)(*rcvSize);
    SIM_dbg_write_trace(SIM_dbg_tmp, 2);
    SIM_dbg_write_trace(result, *rcvSize);
    SIM_dbg_tmp[0] = (SYS_WORD8)(res>>8);
    SIM_dbg_tmp[1] = (SYS_WORD8)res;
    SIM_dbg_write_trace(SIM_dbg_tmp, 2);
    SIM_dbg_write_trace(SIM_dbg_null, SIM_DBG_NULL);
    if (SIM_dbg_tdma_diff > SIM_dbg_max_interchardelay)
        SIM_dbg_max_interchardelay = SIM_dbg_tdma_diff;
    SIM_dbg_tmp[0] = (SYS_UWORD8)(SIM_dbg_tdma_diff >> 8);
    SIM_dbg_tmp[1] = (SYS_UWORD8)(SIM_dbg_tdma_diff);
    SIM_dbg_write_trace(SIM_dbg_tmp, 2);
#endif

    return(res);
}


/*
 * SIM_RunGSMAlgo
 *
 * Authentication procedure
 */
SYS_UWORD16 SIM_RunGSMAlgo(SYS_UWORD8 *result, SYS_UWORD8 *dat,
			   SYS_UWORD16 *rcvSize)
{
    SIM_PORT *p;
    int len;
    int i;
    int res;

    p = &(Sim[0]);

    if(p->Freq_Algo)                                 //13/4 Mhz mandatory ??
        p->c->conf1 = p->conf1 &= ~SIM_CONF1_SCLKDIV;

    len = 16;

    p->xbuf[0] = GSM_CLASS;
    p->xbuf[1] = SIM_RUN_GSM_ALGO;
    p->xbuf[2] = 0; 
    p->xbuf[3] = 0; 
    p->xbuf[4] = len; 

    for (i=0;i<len;i++)
    {
        p->xbuf[5+i] = *(dat+i);
    }
    res = SIM_Command(p, len, result, rcvSize);

#ifdef SIM_DEBUG_TRACE
    SIM_dbg_write_trace((SYS_UWORD8 *)"AFCMD", 5);
    SIM_dbg_write_trace(p->xbuf, len+5);
    SIM_dbg_write_trace((SYS_UWORD8 *)"AFANS", 5);
    SIM_dbg_tmp[0] = (SYS_UWORD8)(*rcvSize >> 8);
    SIM_dbg_tmp[1] = (SYS_UWORD8)(*rcvSize);
    SIM_dbg_write_trace(SIM_dbg_tmp, 2);
    SIM_dbg_write_trace(result, *rcvSize);
    SIM_dbg_tmp[0] = (SYS_WORD8)(res>>8);
    SIM_dbg_tmp[1] = (SYS_WORD8)res;
    SIM_dbg_write_trace(SIM_dbg_tmp, 2);
    SIM_dbg_write_trace(SIM_dbg_null, SIM_DBG_NULL);
    if (SIM_dbg_tdma_diff > SIM_dbg_max_interchardelay)
        SIM_dbg_max_interchardelay = SIM_dbg_tdma_diff;
    SIM_dbg_tmp[0] = (SYS_UWORD8)(SIM_dbg_tdma_diff >> 8);
    SIM_dbg_tmp[1] = (SYS_UWORD8)(SIM_dbg_tdma_diff);
    SIM_dbg_write_trace(SIM_dbg_tmp, 2);
#endif

    if(p->Freq_Algo)
        p->c->conf1 = p->conf1 |= SIM_CONF1_SCLKDIV;

    return(res);
}


/*
 * SIM_GetResponse
 *
 * Get data from the card
 *
 * SYS_UWORD8 len : length of the data to get
 */
SYS_UWORD16 SIM_GetResponse(SYS_UWORD8 *dat, SYS_UWORD16 len,
			    SYS_UWORD16 *rcvSize)
{
    SIM_PORT *p;
    int res;
    SYS_UWORD16 llen = len & SIM_UWORD16_MASK;

    p = &(Sim[0]);

    p->xbuf[0] = GSM_CLASS;
    p->xbuf[1] = SIM_GET_RESPONSE;
    p->xbuf[2] = 0; 
    p->xbuf[3] = 0; 
    p->xbuf[4] = (SYS_UWORD8)llen; 

    res = SIM_Command(p, 0, dat, rcvSize);

#ifdef SIM_DEBUG_TRACE
    SIM_dbg_write_trace((SYS_UWORD8 *)"AGCMD", 5);
    SIM_dbg_write_trace(p->xbuf, 5);
    SIM_dbg_write_trace((SYS_UWORD8 *)"AGANS", 5);
    SIM_dbg_tmp[0] = (SYS_UWORD8)(*rcvSize >> 8);
    SIM_dbg_tmp[1] = (SYS_UWORD8)(*rcvSize);
    SIM_dbg_write_trace(SIM_dbg_tmp, 2);
    SIM_dbg_write_trace(dat, *rcvSize);
    SIM_dbg_tmp[0] = (SYS_WORD8)(res>>8);
    SIM_dbg_tmp[1] = (SYS_WORD8)res;
    SIM_dbg_write_trace(SIM_dbg_tmp, 2);
    SIM_dbg_write_trace(SIM_dbg_null, SIM_DBG_NULL);
    if (SIM_dbg_tdma_diff > SIM_dbg_max_interchardelay)
        SIM_dbg_max_interchardelay = SIM_dbg_tdma_diff;
    SIM_dbg_tmp[0] = (SYS_UWORD8)(SIM_dbg_tdma_diff >> 8);
    SIM_dbg_tmp[1] = (SYS_UWORD8)(SIM_dbg_tdma_diff);
    SIM_dbg_write_trace(SIM_dbg_tmp, 2);
#endif

    return(res);
}


/*
 * SIM_ChangeCHV
 *
 * Change the specified CHV (chvType)
 */
SYS_UWORD16 SIM_ChangeCHV(SYS_UWORD8 *result,SYS_UWORD8 *oldChv,
			  SYS_UWORD8 *newChv, SYS_UWORD8 chvType,
			  SYS_UWORD16 *lP)
{
    SIM_PORT *p;
    SYS_UWORD16 len;
    int i;
    SYS_UWORD16 res;

    p = &(Sim[0]);
    len = 16;

    p->xbuf[0] = GSM_CLASS;
    p->xbuf[1] = SIM_CHANGE_CHV;
    p->xbuf[2] = 0; 
    p->xbuf[3] = chvType; 
    p->xbuf[4] = (SYS_UWORD8)len; 

    // Copy bytes to buffer
    for (i=0;i<8;i++)
    {
      p->xbuf[5+i] = *(oldChv+i);
    }
    for (i=0;i<8;i++)
    {
      p->xbuf[13+i] = *(newChv+i);
    }
    res = SIM_Command(p, len, result, lP);

#ifdef SIM_DEBUG_TRACE
    SIM_dbg_write_trace((SYS_UWORD8 *)"AHCMD", 5);
    SIM_dbg_write_trace(p->xbuf, len+5);
    SIM_dbg_write_trace((SYS_UWORD8 *)"AHANS", 5);
    SIM_dbg_tmp[0] = (SYS_UWORD8)(*lP >> 8);
    SIM_dbg_tmp[1] = (SYS_UWORD8)(*lP);
    SIM_dbg_write_trace(SIM_dbg_tmp, 2);
    SIM_dbg_write_trace(result, *lP);
    SIM_dbg_tmp[0] = (SYS_WORD8)(res>>8);
    SIM_dbg_tmp[1] = (SYS_WORD8)res;
    SIM_dbg_write_trace(SIM_dbg_tmp, 2);
    SIM_dbg_write_trace(SIM_dbg_null, SIM_DBG_NULL);
    if (SIM_dbg_tdma_diff > SIM_dbg_max_interchardelay)
        SIM_dbg_max_interchardelay = SIM_dbg_tdma_diff;
    SIM_dbg_tmp[0] = (SYS_UWORD8)(SIM_dbg_tdma_diff >> 8);
    SIM_dbg_tmp[1] = (SYS_UWORD8)(SIM_dbg_tdma_diff);
    SIM_dbg_write_trace(SIM_dbg_tmp, 2);
#endif

    return(res);
}


/*
 * SIM_DisableCHV
 *
 * Disable CHV 1
 */
SYS_UWORD16 SIM_DisableCHV(SYS_UWORD8 *result, SYS_UWORD8 *dat, SYS_UWORD16 *lP)
{
    SIM_PORT *p;
    int len;
    int i;
    int res;

    p = &(Sim[0]);

    len = 8;
    p->xbuf[0] = GSM_CLASS;
    p->xbuf[1] = SIM_DISABLE_CHV;
    p->xbuf[2] = 0; 
    p->xbuf[3] = 1; 
    p->xbuf[4] = 8; 
    for (i=0;i<8;i++)
    {
      p->xbuf[5+i] = *(dat+i);
    }
    res = SIM_Command(p, len, result, lP);        

#ifdef SIM_DEBUG_TRACE
    SIM_dbg_write_trace((SYS_UWORD8 *)"AICMD", 5);
    SIM_dbg_write_trace(p->xbuf, 8+5);
    SIM_dbg_write_trace((SYS_UWORD8 *)"AIANS", 5);
    SIM_dbg_tmp[0] = (SYS_UWORD8)(*lP >> 8);
    SIM_dbg_tmp[1] = (SYS_UWORD8)(*lP);
    SIM_dbg_write_trace(SIM_dbg_tmp, 2);
    SIM_dbg_write_trace(result, *lP);
    SIM_dbg_tmp[0] = (SYS_WORD8)(res>>8);
    SIM_dbg_tmp[1] = (SYS_WORD8)res;
    SIM_dbg_write_trace(SIM_dbg_tmp, 2);
    SIM_dbg_write_trace(SIM_dbg_null, SIM_DBG_NULL);
    if (SIM_dbg_tdma_diff > SIM_dbg_max_interchardelay)
        SIM_dbg_max_interchardelay = SIM_dbg_tdma_diff;
    SIM_dbg_tmp[0] = (SYS_UWORD8)(SIM_dbg_tdma_diff >> 8);
    SIM_dbg_tmp[1] = (SYS_UWORD8)(SIM_dbg_tdma_diff);
    SIM_dbg_write_trace(SIM_dbg_tmp, 2);
#endif

    return(res);
}


/*
 * SIM_EnableCHV
 *
 * Enable CHV 1
 */
SYS_UWORD16 SIM_EnableCHV(SYS_UWORD8 *result, SYS_UWORD8 *dat, SYS_UWORD16 *lP)
{
    SIM_PORT *p;
    int len;
    int i;
    int res;

    p = &(Sim[0]);

    len = 8;

    p->xbuf[0] = GSM_CLASS;
    p->xbuf[1] = SIM_ENABLE_CHV;
    p->xbuf[2] = 0; 
    p->xbuf[3] = 1; 
    p->xbuf[4] = (SYS_UWORD8)len; 

    for (i=0;i<len;i++)
    {
      p->xbuf[5+i] = *(dat+i);
    }

    res = SIM_Command(p, len, result, lP);

#ifdef SIM_DEBUG_TRACE
    SIM_dbg_write_trace((SYS_UWORD8 *)"AJCMD", 5);
    SIM_dbg_write_trace(p->xbuf, len+5);
    SIM_dbg_write_trace((SYS_UWORD8 *)"AJANS", 5);
    SIM_dbg_tmp[0] = (SYS_UWORD8)(*lP >> 8);
    SIM_dbg_tmp[1] = (SYS_UWORD8)(*lP);
    SIM_dbg_write_trace(SIM_dbg_tmp, 2);
    SIM_dbg_write_trace(result, *lP);
    SIM_dbg_tmp[0] = (SYS_WORD8)(res>>8);
    SIM_dbg_tmp[1] = (SYS_WORD8)res;
    SIM_dbg_write_trace(SIM_dbg_tmp, 2);
    SIM_dbg_write_trace(SIM_dbg_null, SIM_DBG_NULL);
    if (SIM_dbg_tdma_diff > SIM_dbg_max_interchardelay)
        SIM_dbg_max_interchardelay = SIM_dbg_tdma_diff;
    SIM_dbg_tmp[0] = (SYS_UWORD8)(SIM_dbg_tdma_diff >> 8);
    SIM_dbg_tmp[1] = (SYS_UWORD8)(SIM_dbg_tdma_diff);
    SIM_dbg_write_trace(SIM_dbg_tmp, 2);
#endif

    return(res);
}

/*
 * SIM_UnblockCHV
 *
 * Unblock the specified CHV (chvType) and store a new CHV
 */
SYS_UWORD16 SIM_UnblockCHV(SYS_UWORD8 *result, SYS_UWORD8 *unblockChv,
			   SYS_UWORD8 *newChv, SYS_UWORD8 chvType,
			   SYS_UWORD16 *lP) 
{
    SIM_PORT *p;
    int len;
    int i;
    int res;

    p = &(Sim[0]);
    len = 16;

    p->xbuf[0] = GSM_CLASS;
    p->xbuf[1] = SIM_UNBLOCK_CHV;
    p->xbuf[2] = 0;
    p->xbuf[3] = chvType;
    p->xbuf[4] = (SYS_UWORD8)len; 
    for (i=0;i<8;i++)
    {
      p->xbuf[5+i] = *(unblockChv+i);
    }
    for (i=0;i<8;i++)
    {
      p->xbuf[13+i] = *(newChv+i);
    }

    res = SIM_Command(p, len, result, lP);

#ifdef SIM_DEBUG_TRACE
    SIM_dbg_write_trace((SYS_UWORD8 *)"AKCMD", 5);
    SIM_dbg_write_trace(p->xbuf, len+5);
    SIM_dbg_write_trace((SYS_UWORD8 *)"AKANS", 5);
    SIM_dbg_tmp[0] = (SYS_UWORD8)(*lP >> 8);
    SIM_dbg_tmp[1] = (SYS_UWORD8)(*lP);
    SIM_dbg_write_trace(SIM_dbg_tmp, 2);
    SIM_dbg_write_trace(result, *lP);
    SIM_dbg_tmp[0] = (SYS_WORD8)(res>>8);
    SIM_dbg_tmp[1] = (SYS_WORD8)res;
    SIM_dbg_write_trace(SIM_dbg_tmp, 2);
    SIM_dbg_write_trace(SIM_dbg_null, SIM_DBG_NULL);
    if (SIM_dbg_tdma_diff > SIM_dbg_max_interchardelay)
        SIM_dbg_max_interchardelay = SIM_dbg_tdma_diff;
    SIM_dbg_tmp[0] = (SYS_UWORD8)(SIM_dbg_tdma_diff >> 8);
    SIM_dbg_tmp[1] = (SYS_UWORD8)(SIM_dbg_tdma_diff);
    SIM_dbg_write_trace(SIM_dbg_tmp, 2);
#endif

    return(res);
}

/*
 * SIM_Invalidate
 *
 * Invalidate the current EF
 */
SYS_UWORD16 SIM_Invalidate(SYS_UWORD8 *rP, SYS_UWORD16 *lP)
{
    SIM_PORT *p;
    int i;
    int res;

    p = &(Sim[0]);

    p->xbuf[0] = GSM_CLASS;
    p->xbuf[1] = SIM_INVALIDATE;
    p->xbuf[2] = 0; 
    p->xbuf[3] = 0; 
    p->xbuf[4] = 0; 

    res = SIM_Command(p, 0, rP, lP);

#ifdef SIM_DEBUG_TRACE
    SIM_dbg_write_trace((SYS_UWORD8 *)"ALCMD", 5);
    SIM_dbg_write_trace(p->xbuf, 5);
    SIM_dbg_write_trace((SYS_UWORD8 *)"ALANS", 5);
    SIM_dbg_tmp[0] = (SYS_UWORD8)(*lP >> 8);
    SIM_dbg_tmp[1] = (SYS_UWORD8)(*lP);
    SIM_dbg_write_trace(SIM_dbg_tmp, 2);
    SIM_dbg_write_trace(rP, *lP);
    SIM_dbg_tmp[0] = (SYS_WORD8)(res>>8);
    SIM_dbg_tmp[1] = (SYS_WORD8)res;
    SIM_dbg_write_trace(SIM_dbg_tmp, 2);
    SIM_dbg_write_trace(SIM_dbg_null, SIM_DBG_NULL);
    if (SIM_dbg_tdma_diff > SIM_dbg_max_interchardelay)
        SIM_dbg_max_interchardelay = SIM_dbg_tdma_diff;
    SIM_dbg_tmp[0] = (SYS_UWORD8)(SIM_dbg_tdma_diff >> 8);
    SIM_dbg_tmp[1] = (SYS_UWORD8)(SIM_dbg_tdma_diff);
    SIM_dbg_write_trace(SIM_dbg_tmp, 2);
#endif

    return(res);
}

/*
 * SIM_Rehabilitate
 *
 * Rehabilitate the current EF
 */
SYS_UWORD16 SIM_Rehabilitate(SYS_UWORD8 *rP, SYS_UWORD16 *lP)
{
    SIM_PORT *p;
    int len;
    int res;

    p = &(Sim[0]);

    p->xbuf[0] = GSM_CLASS;
    p->xbuf[1] = SIM_REHABILITATE;
    p->xbuf[2] = 0; 
    p->xbuf[3] = 0; 
    p->xbuf[4] = 0;

    res = SIM_Command(p, 0, rP, lP);        

#ifdef SIM_DEBUG_TRACE
    SIM_dbg_write_trace((SYS_UWORD8 *)"AMCMD", 5);
    SIM_dbg_write_trace(p->xbuf, 5);
    SIM_dbg_write_trace((SYS_UWORD8 *)"AMANS", 5);
    SIM_dbg_tmp[0] = (SYS_UWORD8)(*lP >> 8);
    SIM_dbg_tmp[1] = (SYS_UWORD8)(*lP);
    SIM_dbg_write_trace(SIM_dbg_tmp, 2);
    SIM_dbg_write_trace(rP, *lP);
    SIM_dbg_tmp[0] = (SYS_WORD8)(res>>8);
    SIM_dbg_tmp[1] = (SYS_WORD8)res;
    SIM_dbg_write_trace(SIM_dbg_tmp, 2);
    SIM_dbg_write_trace(SIM_dbg_null, SIM_DBG_NULL);
    if (SIM_dbg_tdma_diff > SIM_dbg_max_interchardelay)
        SIM_dbg_max_interchardelay = SIM_dbg_tdma_diff;
    SIM_dbg_tmp[0] = (SYS_UWORD8)(SIM_dbg_tdma_diff >> 8);
    SIM_dbg_tmp[1] = (SYS_UWORD8)(SIM_dbg_tdma_diff);
    SIM_dbg_write_trace(SIM_dbg_tmp, 2);
#endif

    return(res);
}


/*
 * SIM_UpdateBinary
 *
 * Store data in the current transparent EF
 */
SYS_UWORD16 SIM_UpdateBinary(SYS_UWORD8 *result, SYS_UWORD8 *dat,
			     SYS_UWORD16 offset, SYS_UWORD16 len,
			     SYS_UWORD16 *rcvSize)
{
    SIM_PORT *p;
    int i;
    int res;
    SYS_UWORD16 llen = len & SIM_UWORD16_MASK;

    p = &(Sim[0]);

    p->xbuf[0] = GSM_CLASS;
    p->xbuf[1] = SIM_UPDATE_BINARY;
    p->xbuf[2] = offset >> 8; 
    p->xbuf[3] = offset & 0xFF; 
    p->xbuf[4] = (SYS_UWORD8)llen; 

    for (i=0;i<llen;i++)
    {
      p->xbuf[5+i] = *(dat+i);
    }
    res = SIM_Command(p, llen, result, rcvSize);        

#ifdef SIM_DEBUG_TRACE
    SIM_dbg_write_trace((SYS_UWORD8 *)"ANCMD", 5);
    SIM_dbg_write_trace(p->xbuf, llen+5);
    SIM_dbg_write_trace((SYS_UWORD8 *)"ANANS", 5);
    SIM_dbg_tmp[0] = (SYS_UWORD8)(*rcvSize >> 8);
    SIM_dbg_tmp[1] = (SYS_UWORD8)(*rcvSize);
    SIM_dbg_write_trace(SIM_dbg_tmp, 2);
    SIM_dbg_write_trace(result, *rcvSize);
    SIM_dbg_tmp[0] = (SYS_WORD8)(res>>8);
    SIM_dbg_tmp[1] = (SYS_WORD8)res;
    SIM_dbg_write_trace(SIM_dbg_tmp, 2);
    SIM_dbg_write_trace(SIM_dbg_null, SIM_DBG_NULL);
    if (SIM_dbg_tdma_diff > SIM_dbg_max_interchardelay)
        SIM_dbg_max_interchardelay = SIM_dbg_tdma_diff;
    SIM_dbg_tmp[0] = (SYS_UWORD8)(SIM_dbg_tdma_diff >> 8);
    SIM_dbg_tmp[1] = (SYS_UWORD8)(SIM_dbg_tdma_diff);
    SIM_dbg_write_trace(SIM_dbg_tmp, 2);
#endif

    return(res);
}


/*
 * SIM_ReadRecord
 *
 * Read a record (recNum) from the current linear fixed or cyclic EF
 */
SYS_UWORD16 SIM_ReadRecord(SYS_UWORD8 *dat, SYS_UWORD8 mode, SYS_UWORD8 recNum,
			   SYS_UWORD16 len, SYS_UWORD16 *rcvSize)
{
    SIM_PORT *p;
    int res;
    SYS_UWORD16 llen = len & SIM_UWORD16_MASK;

    p = &(Sim[0]);

    p->xbuf[0] = GSM_CLASS;
    p->xbuf[1] = SIM_READ_RECORD;
    p->xbuf[2] = recNum; 
    p->xbuf[3] = mode; 
    p->xbuf[4] = (SYS_UWORD8)llen; 

    res = SIM_Command(p, 0, dat, rcvSize);        

#ifdef SIM_DEBUG_TRACE
    SIM_dbg_write_trace((SYS_UWORD8 *)"AOCMD", 5);
    SIM_dbg_write_trace(p->xbuf, llen+5);
    SIM_dbg_write_trace((SYS_UWORD8 *)"AOANS", 5);
    SIM_dbg_tmp[0] = (SYS_UWORD8)(*rcvSize >> 8);
    SIM_dbg_tmp[1] = (SYS_UWORD8)(*rcvSize);
    SIM_dbg_write_trace(SIM_dbg_tmp, 2);
    SIM_dbg_write_trace(dat, *rcvSize);
    SIM_dbg_tmp[0] = (SYS_WORD8)(res>>8);
    SIM_dbg_tmp[1] = (SYS_WORD8)res;
    SIM_dbg_write_trace(SIM_dbg_tmp, 2);
    SIM_dbg_write_trace(SIM_dbg_null, SIM_DBG_NULL);
    if (SIM_dbg_tdma_diff > SIM_dbg_max_interchardelay)
        SIM_dbg_max_interchardelay = SIM_dbg_tdma_diff;
    SIM_dbg_tmp[0] = (SYS_UWORD8)(SIM_dbg_tdma_diff >> 8);
    SIM_dbg_tmp[1] = (SYS_UWORD8)(SIM_dbg_tdma_diff);
    SIM_dbg_write_trace(SIM_dbg_tmp, 2);
#endif

    return(res);
}

#ifdef SIM_APDU_TEST
SYS_UWORD8 snd[270];
SYS_UWORD8 rec[270];
SYS_UWORD8 logchan;
SYS_UWORD16 recl;
unsigned short resopen, resclose, rescmd;
#endif

#ifdef SIM_SAT_REFRESH_TEST
SIM_CARD ptr;
SYS_UWORD16 lrcvSize;
SYS_UWORD8 ldat[20];
#endif

/*
 * SIM_UpdateRecord
 *
 * Store a record (recNum) in the current linear fixed or cyclic EF
 */
SYS_UWORD16 SIM_UpdateRecord(SYS_UWORD8 *result, SYS_UWORD8 *dat,
			     SYS_UWORD8 mode, SYS_UWORD8 recNum,
			     SYS_UWORD16 len, SYS_UWORD16 *rcvSize)
{
    SIM_PORT *p;
    int i;
    int res;
    SYS_UWORD16 llen = len & SIM_UWORD16_MASK;

#ifdef SIM_SAT_REFRESH_TEST
// do 1000 times the following sequence
for (i=0;i<1000;i++) {
SIM_PowerOff();
SIM_Restart(&ptr);
SIM_Select((SYS_UWORD16)0x7f10, ldat, &lrcvSize);
SIM_Select((SYS_UWORD16)0x6f3a, ldat, &lrcvSize);
}
#endif
#ifdef SIM_APDU_TEST
    // send OPEN LOGICAL CHANNEL
    snd[0] = 0x00;
    snd[1] = 0x70;
    snd[2] = 0x00;
    snd[3] = 0x00;
    snd[4] = 0x01;
    resopen = SIM_XchTPDU(&snd[0], 5, &rec[0], 1, &recl);
    if (resopen == 0x9000) {
	logchan = rec[0];

	// Select AID PKCS
	snd[0] = logchan;
	snd[1] = 0xA4;
	snd[2] = 0x04;
	snd[3] = 0x00;
	snd[4] = 0x0C;
	snd[5] = 0xA0;
	snd[6] = 0x00;
	snd[7] = 0x00;
	snd[8] = 0x00;
	snd[9] = 0x63;
	snd[10] = 0x50;
	snd[11]  = 0x4B;
	snd[12] = 0x43;
	snd[13] = 0x53;
	snd[14] = 0x2D;
	snd[15] = 0x31;
	snd[16] = 0x35;
	rescmd = SIM_XchTPDU(&snd[0], 17, &rec[0], 0, &recl);

	// Select file EF odf
	snd[0] = 0x80 | logchan;
	snd[1] = 0xA4;
	snd[2] = 0x00;
	snd[3] = 0x00;
	snd[4] = 0x02;
	snd[5] = 0x50;
	snd[6] = 0x31;
	rescmd = SIM_XchTPDU(&snd[0], 7, &rec[0], 0, &recl);

	// get response EF odf
	snd[0] = logchan;
	snd[1] = 0xC0;
	snd[2] = 0x00;
	snd[3] = 0x00;
	snd[4] = rescmd;
	rescmd = SIM_XchTPDU(&snd[0], 5, &rec[0], snd[4], &recl);

	// read binary EF odf
	snd[0] = 0x80 | logchan;
	snd[1] = 0xB0;
	snd[2] = 0x00;
	snd[3] = 0x00;
	snd[4] = rec[3]-16;
	rescmd = SIM_XchTPDU(&snd[0], 5, &rec[0], snd[4], &recl);

	// Select file EF cdf
	snd[0] = 0x80 | logchan;
	snd[1] = 0xA4;
	snd[2] = 0x00;
	snd[3] = 0x00;
	snd[4] = 0x02;
	snd[5] = 0x51;
	snd[6] = 0x03;
	rescmd = SIM_XchTPDU(&snd[0], 7, &rec[0], 0, &recl);

	// get response EF odf
	snd[0] = logchan;
	snd[1] = 0xC0;
	snd[2] = 0x00;
	snd[3] = 0x00;
	snd[4] = rescmd;
	rescmd = SIM_XchTPDU(&snd[0], 5, &rec[0], snd[4], &recl);

	// read binary EF cdf
	snd[0] = 0x80 | logchan;
	snd[1] = 0xB0;
	snd[2] = 0x00;
	snd[3] = 0x00;
	snd[4] = 0xff;
	rescmd = SIM_XchTPDU(&snd[0], 5, &rec[0], snd[4], &recl);

	// read binary EF cdf
	snd[0] = 0x80 | logchan;
	snd[1] = 0xB0;
	snd[2] = 0x00;
	snd[3] = 0x00;
	snd[4] = 0x00;
	rescmd = SIM_XchTPDU(&snd[0], 5, &rec[0], 256, &recl);
    }
#endif

    p = &(Sim[0]);

    p->xbuf[0] = GSM_CLASS;
    p->xbuf[1] = SIM_UPDATE_RECORD;
    p->xbuf[2] = recNum; 
    p->xbuf[3] = mode; 
    p->xbuf[4] = (SYS_UWORD8)llen; 

    for (i=0;i<llen;i++)
    {
        p->xbuf[5+i] = *(dat+i);
    }

    res = SIM_Command(p, llen, result, rcvSize);        

#ifdef SIM_DEBUG_TRACE
    SIM_dbg_write_trace((SYS_UWORD8 *)"APCMD", 5);
    SIM_dbg_write_trace(p->xbuf, llen+5);
    SIM_dbg_write_trace((SYS_UWORD8 *)"APANS", 5);
    SIM_dbg_tmp[0] = (SYS_UWORD8)(*rcvSize >> 8);
    SIM_dbg_tmp[1] = (SYS_UWORD8)(*rcvSize);
    SIM_dbg_write_trace(SIM_dbg_tmp, 2);
    SIM_dbg_write_trace(result, *rcvSize);
    SIM_dbg_tmp[0] = (SYS_WORD8)(res>>8);
    SIM_dbg_tmp[1] = (SYS_WORD8)res;
    SIM_dbg_write_trace(SIM_dbg_tmp, 2);
    SIM_dbg_write_trace(SIM_dbg_null, SIM_DBG_NULL);
    if (SIM_dbg_tdma_diff > SIM_dbg_max_interchardelay)
        SIM_dbg_max_interchardelay = SIM_dbg_tdma_diff;
    SIM_dbg_tmp[0] = (SYS_UWORD8)(SIM_dbg_tdma_diff >> 8);
    SIM_dbg_tmp[1] = (SYS_UWORD8)(SIM_dbg_tdma_diff);
    SIM_dbg_write_trace(SIM_dbg_tmp, 2);
#endif

#ifdef SIM_APDU_TEST
    // send CLOSE LOGICAL CHANNEL
    snd[0] = 0x00;
    snd[1] = 0x70;
    snd[2] = 0x80;
    snd[3] = logchan;
    snd[4] = 0x00;
    resclose = SIM_XchTPDU(&snd[0], 5, &rec[0], 0, &recl);
#endif

    return(res);
}

/*
 * SIM_Seek
 *
 * Search data in a linear fixed or cyclic EF.
 * return the first record number in which it found the data.
 */
SYS_UWORD16 SIM_Seek(SYS_UWORD8 *result, SYS_UWORD8 *dat, SYS_UWORD8 mode,
		     SYS_UWORD16 len, SYS_UWORD16 *rcvSize)
{
   SIM_PORT *p;
   int i;
   int res;
   SYS_UWORD16 llen = len & SIM_UWORD16_MASK;

   p = &(Sim[0]);

   p->xbuf[0] = GSM_CLASS;
   p->xbuf[1] = SIM_SEEK;
   p->xbuf[2] = 0; 
   p->xbuf[3] = mode; 
   p->xbuf[4] = (SYS_UWORD8)llen; 

   for (i=0;i<llen;i++)
   {
      p->xbuf[5+i] = *(dat+i);
   }

   res = SIM_Command(p, llen, result, rcvSize);        

#ifdef SIM_DEBUG_TRACE
    SIM_dbg_write_trace((SYS_UWORD8 *)"AQCMD", 5);
    SIM_dbg_write_trace(p->xbuf, llen+5);
    SIM_dbg_write_trace((SYS_UWORD8 *)"AQANS", 5);
    SIM_dbg_tmp[0] = (SYS_UWORD8)(*rcvSize >> 8);
    SIM_dbg_tmp[1] = (SYS_UWORD8)(*rcvSize);
    SIM_dbg_write_trace(SIM_dbg_tmp, 2);
    SIM_dbg_write_trace(result, *rcvSize);
    SIM_dbg_tmp[0] = (SYS_WORD8)(res>>8);
    SIM_dbg_tmp[1] = (SYS_WORD8)res;
    SIM_dbg_write_trace(SIM_dbg_tmp, 2);
    SIM_dbg_write_trace(SIM_dbg_null, SIM_DBG_NULL);
    if (SIM_dbg_tdma_diff > SIM_dbg_max_interchardelay)
        SIM_dbg_max_interchardelay = SIM_dbg_tdma_diff;
    SIM_dbg_tmp[0] = (SYS_UWORD8)(SIM_dbg_tdma_diff >> 8);
    SIM_dbg_tmp[1] = (SYS_UWORD8)(SIM_dbg_tdma_diff);
    SIM_dbg_write_trace(SIM_dbg_tmp, 2);
#endif

   return(res);
}

/*
 * SIM_Increase
 *
 * Add value to a record of a cyclic EF
 */
SYS_UWORD16 SIM_Increase(SYS_UWORD8 *result, SYS_UWORD8 *dat,
			 SYS_UWORD16 *rcvSize)
{
    SIM_PORT *p;
    int len;
    int i;
    int res;

    p = &(Sim[0]);

    len = 3;

    p->xbuf[0] = GSM_CLASS;
    p->xbuf[1] = SIM_INCREASE;
    p->xbuf[2] = 0; 
    p->xbuf[3] = 0; 
    p->xbuf[4] = 3; 

    for (i=0;i<3;i++)
    {
        p->xbuf[5+i] = *(dat+i);
    }

    res = SIM_Command(p, len, result, rcvSize);        

#ifdef SIM_DEBUG_TRACE
    SIM_dbg_write_trace((SYS_UWORD8 *)"ARCMD", 5);
    SIM_dbg_write_trace(p->xbuf, 3+5);
    SIM_dbg_write_trace((SYS_UWORD8 *)"ARANS", 5);
    SIM_dbg_tmp[0] = (SYS_UWORD8)(*rcvSize >> 8);
    SIM_dbg_tmp[1] = (SYS_UWORD8)(*rcvSize);
    SIM_dbg_write_trace(SIM_dbg_tmp, 2);
    SIM_dbg_write_trace(result, *rcvSize);
    SIM_dbg_tmp[0] = (SYS_WORD8)(res>>8);
    SIM_dbg_tmp[1] = (SYS_WORD8)res;
    SIM_dbg_write_trace(SIM_dbg_tmp, 2);
    SIM_dbg_write_trace(SIM_dbg_null, SIM_DBG_NULL);
    if (SIM_dbg_tdma_diff > SIM_dbg_max_interchardelay)
        SIM_dbg_max_interchardelay = SIM_dbg_tdma_diff;
    SIM_dbg_tmp[0] = (SYS_UWORD8)(SIM_dbg_tdma_diff >> 8);
    SIM_dbg_tmp[1] = (SYS_UWORD8)(SIM_dbg_tdma_diff);
    SIM_dbg_write_trace(SIM_dbg_tmp, 2);
#endif

    return(res);
}

/*
 * SIM_TerminalProfile
 *
 * Used by ME to send its toolkit capabilities to SIM
 */
SYS_UWORD16 SIM_TerminalProfile(SYS_UWORD8 *result, SYS_UWORD8 *dat,
				SYS_UWORD16 len, SYS_UWORD16 *rcvSize)
{
    SIM_PORT *p;
    int i;
    int res;
    SYS_UWORD16 llen = len & SIM_UWORD16_MASK;

    p = &(Sim[0]);

    p->xbuf[0] = GSM_CLASS;
    p->xbuf[1] = SIM_TERMINAL_PROFILE;
    p->xbuf[2] = 0; 
    p->xbuf[3] = 0; 
    p->xbuf[4] = (SYS_UWORD8)llen; 

    for (i=0;i<llen;i++)
    {
        p->xbuf[5+i] = *(dat+i);
    }

    res = SIM_Command(p, llen, result, rcvSize);        

#ifdef SIM_DEBUG_TRACE
    SIM_dbg_write_trace((SYS_UWORD8 *)"ASCMD", 5);
    SIM_dbg_write_trace(p->xbuf, llen+5);
    SIM_dbg_write_trace((SYS_UWORD8 *)"ASANS", 5);
    SIM_dbg_tmp[0] = (SYS_WORD8)(res>>8);
    SIM_dbg_tmp[1] = (SYS_WORD8)res;
    SIM_dbg_write_trace(SIM_dbg_tmp, 2);
    SIM_dbg_write_trace(SIM_dbg_null, SIM_DBG_NULL);
    if (SIM_dbg_tdma_diff > SIM_dbg_max_interchardelay)
        SIM_dbg_max_interchardelay = SIM_dbg_tdma_diff;
    SIM_dbg_tmp[0] = (SYS_UWORD8)(SIM_dbg_tdma_diff >> 8);
    SIM_dbg_tmp[1] = (SYS_UWORD8)(SIM_dbg_tdma_diff);
    SIM_dbg_write_trace(SIM_dbg_tmp, 2);
#endif

    return(res);
}


/*
 * SIM_FETCH
 *
 * Used by ME to inquiry of what SIM toolkit need to do
 */
SYS_UWORD16 SIM_Fetch(SYS_UWORD8 *result, SYS_UWORD16 len, SYS_UWORD16 *rcvSize)
{
    SIM_PORT *p;
    int i;
    int res;
    SYS_UWORD16 llen = len & SIM_UWORD16_MASK;

    p = &(Sim[0]);

    p->xbuf[0] = GSM_CLASS;
    p->xbuf[1] = SIM_FETCH;
    p->xbuf[2] = 0; 
    p->xbuf[3] = 0; 
    p->xbuf[4] = (SYS_UWORD8)llen; 

    res = SIM_Command(p, 0, result, rcvSize);        

#ifdef SIM_DEBUG_TRACE
    SIM_dbg_write_trace((SYS_UWORD8 *)"ATCMD", 5);
    SIM_dbg_write_trace(p->xbuf, 5);
    SIM_dbg_write_trace((SYS_UWORD8 *)"ATANS", 5);
    SIM_dbg_tmp[0] = (SYS_UWORD8)(*rcvSize >> 8);
    SIM_dbg_tmp[1] = (SYS_UWORD8)(*rcvSize);
    SIM_dbg_write_trace(SIM_dbg_tmp, 2);
    SIM_dbg_write_trace(result, *rcvSize);
    SIM_dbg_tmp[0] = (SYS_WORD8)(res>>8);
    SIM_dbg_tmp[1] = (SYS_WORD8)res;
    SIM_dbg_write_trace(SIM_dbg_tmp, 2);
    SIM_dbg_write_trace(SIM_dbg_null, SIM_DBG_NULL);
    if (SIM_dbg_tdma_diff > SIM_dbg_max_interchardelay)
        SIM_dbg_max_interchardelay = SIM_dbg_tdma_diff;
    SIM_dbg_tmp[0] = (SYS_UWORD8)(SIM_dbg_tdma_diff >> 8);
    SIM_dbg_tmp[1] = (SYS_UWORD8)(SIM_dbg_tdma_diff);
    SIM_dbg_write_trace(SIM_dbg_tmp, 2);
#endif

    return(res);
}


/*
 * SIM_TerminalResponse *
 * Used for ME to respond at a SIM toolkit command
 */
SYS_UWORD16 SIM_TerminalResponse(SYS_UWORD8 *result, SYS_UWORD8 *dat,
				 SYS_UWORD16 len, SYS_UWORD16 *rcvSize)
{
    SIM_PORT *p;
    int i;
    int res;
    SYS_UWORD16 llen = len & SIM_UWORD16_MASK;

    p = &(Sim[0]);

    p->xbuf[0] = GSM_CLASS;
    p->xbuf[1] = SIM_TERMINAL_RESPONSE;
    p->xbuf[2] = 0; 
    p->xbuf[3] = 0; 
    p->xbuf[4] = (SYS_UWORD8)llen; 

    for (i=0;i<llen;i++)
    {
        p->xbuf[5+i] = *(dat+i);
    }

    res = SIM_Command(p, llen, result, rcvSize);        

#ifdef SIM_DEBUG_TRACE
    SIM_dbg_write_trace((SYS_UWORD8 *)"AUCMD", 5);
    SIM_dbg_write_trace(p->xbuf, llen+5);
    SIM_dbg_write_trace((SYS_UWORD8 *)"AUANS", 5);
    SIM_dbg_tmp[0] = (SYS_WORD8)(res>>8);
    SIM_dbg_tmp[1] = (SYS_WORD8)res;
    SIM_dbg_write_trace(SIM_dbg_tmp, 2);
    SIM_dbg_write_trace(SIM_dbg_null, SIM_DBG_NULL);
    if (SIM_dbg_tdma_diff > SIM_dbg_max_interchardelay)
        SIM_dbg_max_interchardelay = SIM_dbg_tdma_diff;
    SIM_dbg_tmp[0] = (SYS_UWORD8)(SIM_dbg_tdma_diff >> 8);
    SIM_dbg_tmp[1] = (SYS_UWORD8)(SIM_dbg_tdma_diff);
    SIM_dbg_write_trace(SIM_dbg_tmp, 2);
#endif

    return(res);
}


/*
 * SIM_Envelope
 *
 * Used by Network to tansfert data download to the SIM
 * in a transparent way for user
 */
SYS_UWORD16 SIM_Envelope(SYS_UWORD8 *result, SYS_UWORD8 *dat, SYS_UWORD16 len,
			 SYS_UWORD16 *rcvSize)
{
    SIM_PORT    *p;
    int         i;
    int         res;
    SYS_UWORD16 llen = len & SIM_UWORD16_MASK;

    p = &(Sim[0]);

    p->xbuf[0] = GSM_CLASS;
    p->xbuf[1] = SIM_ENVELOPE;
    p->xbuf[2] = 0; 
    p->xbuf[3] = 0; 
    p->xbuf[4] = (SYS_UWORD8)llen; 

    for (i=0;i<llen;i++)
    {
        p->xbuf[5+i] = *(dat+i);
    }

    res = SIM_Command(p, llen, result, rcvSize);        

#ifdef SIM_DEBUG_TRACE
    SIM_dbg_write_trace((SYS_UWORD8 *)"AVCMD", 5);
    SIM_dbg_write_trace(p->xbuf, llen+5);
    SIM_dbg_write_trace((SYS_UWORD8 *)"AVANS", 5);
    SIM_dbg_tmp[0] = (SYS_UWORD8)(*rcvSize >> 8);
    SIM_dbg_tmp[1] = (SYS_UWORD8)(*rcvSize);
    SIM_dbg_write_trace(SIM_dbg_tmp, 2);
    SIM_dbg_write_trace(result, *rcvSize);
    SIM_dbg_tmp[0] = (SYS_WORD8)(res>>8);
    SIM_dbg_tmp[1] = (SYS_WORD8)res;
    SIM_dbg_write_trace(SIM_dbg_tmp, 2);
    SIM_dbg_write_trace(SIM_dbg_null, SIM_DBG_NULL);
    if (SIM_dbg_tdma_diff > SIM_dbg_max_interchardelay)
        SIM_dbg_max_interchardelay = SIM_dbg_tdma_diff;
    SIM_dbg_tmp[0] = (SYS_UWORD8)(SIM_dbg_tdma_diff >> 8);
    SIM_dbg_tmp[1] = (SYS_UWORD8)(SIM_dbg_tdma_diff);
    SIM_dbg_write_trace(SIM_dbg_tmp, 2);
#endif

    return(res);
}

/*
 * SIM_XchTPDU *
 * Used for ME to send generic command to WIM Card
 */
SYS_UWORD16 SIM_XchTPDU(SYS_UWORD8 *dat, SYS_UWORD16 trxLen, SYS_UWORD8 *result,
			SYS_UWORD16 rcvLen, SYS_UWORD16 *rcvSize)
{
    SIM_PORT *p;
    int i;
    int res;

    p = &(Sim[0]);

    p->xbuf[0] = dat[0];
    p->xbuf[1] = dat[1];
    p->xbuf[2] = dat[2]; 
    p->xbuf[3] = dat[3]; 
    p->xbuf[4] = dat[4]; 

    for (i=5;i<trxLen;i++)
    {
        p->xbuf[i] = dat[i];
    }

    // enable the WIM behavior of the sim driver
    p->apdu_ans_length = rcvLen;

    res = SIM_Command(p, (trxLen - 5), result, rcvSize);        

    // disable the WIM behavior of the sim driver
    p->apdu_ans_length = 0;

#ifdef SIM_DEBUG_TRACE
    SIM_dbg_write_trace((SYS_UWORD8 *)"AWCMD", 5);
    SIM_dbg_write_trace(p->xbuf, trxLen);
    SIM_dbg_write_trace((SYS_UWORD8 *)"AWANS", 5);
    SIM_dbg_tmp[0] = (SYS_UWORD8)(*rcvSize >> 8);
    SIM_dbg_tmp[1] = (SYS_UWORD8)(*rcvSize);
    SIM_dbg_write_trace(SIM_dbg_tmp, 2);
    SIM_dbg_write_trace(result, *rcvSize);
    SIM_dbg_tmp[0] = (SYS_WORD8)(res>>8);
    SIM_dbg_tmp[1] = (SYS_WORD8)res;
    SIM_dbg_write_trace(SIM_dbg_tmp, 2);
    SIM_dbg_write_trace(SIM_dbg_null, SIM_DBG_NULL);
    if (SIM_dbg_tdma_diff > SIM_dbg_max_interchardelay)
        SIM_dbg_max_interchardelay = SIM_dbg_tdma_diff;
    SIM_dbg_tmp[0] = (SYS_UWORD8)(SIM_dbg_tdma_diff >> 8);
    SIM_dbg_tmp[1] = (SYS_UWORD8)(SIM_dbg_tdma_diff);
    SIM_dbg_write_trace(SIM_dbg_tmp, 2);
#endif

    return(res);
}

/*
* Use to detect end of characters reception
* input:    p       pointer on sim structure
*           n       number of extra character to send
*
* output:   return 0 if sucess
*                  SIM_ERR_x in case of error
*
*/

SYS_UWORD16 SIM_WaitReception(SIM_PORT *p)
{
    SYS_UWORD16 returncode;

    //analyse the nature of the command to execute

    if (
    	(p->xbuf[1] == 0x12) || 
    	(p->xbuf[1] == 0xB2) || 
    	(p->xbuf[1] == 0xB0) || 
    	(p->xbuf[1] == 0xF2) || 
    	(p->xbuf[1] == 0xC0) || 
    	(p->apdu_ans_length != 0)
       )       
    //FETCH, READ_RECORD, READ_BINARY, STATUS, GET_RESPONSE commands == receive command
    {
        if (p->xbuf[4] == 0)                    //if P3 == 0 when sending receive command
        {
            p->expected_data = 256; 
        }
        else
        {
            p->expected_data = p->xbuf[4];
        }

        p->moderx = 1;                          //wait for procedure byte

        if (returncode = SIM_Waitforchars (p, p->etu9600))
        {
            return returncode;
        }
    }
    else    //direct command : INVALIDATE, REHABILITATE, SLEEP
    {
        p->moderx = 5;      //mode reception of SW1/SW2

        if (returncode = SIM_Waitforchars (p, p->etu9600))
        {
            return returncode;
        }
    }
    return (0);
}

/*
* Use to read file characteristics information
* input:    p       pointer on sim structure
*
* output:   return 0 if sucess
*                  1 in case of error
*
*/

SYS_UWORD8 SIM_GetFileCharacteristics(SIM_PORT *p)
{
    int   res;
    SYS_UWORD8  ubuf[40];
    SYS_UWORD16  sz;

    res = SIM_Select(DF_GSM, ubuf, &sz);
    if ((res & 0xFF00) != 0x9F00)
    {
        res = SIM_Select(DF_DCS1800, ubuf, &sz);
        if ((res & 0xFF00) != 0x9F00)
        {
            return (1);
        }
    }
    res = SIM_GetResponse( ubuf, res & 0x00FF , &sz);
    if (res != 0x9000)
        return (1);

    p->FileC = ubuf[13];
    return (0);
}  
  
/*
* Use to determine value of b2 in file caracteristics contained in response
* of SELECT Master File command
* return    0 if no preferred speed during authentication
*           1 if 13/4Mhz mandatory 
*
*
*/
 
void SIM_Interpret_FileCharacteristics(SIM_PORT *p)
{
    //interpret b2 bit for operating authentication speed
    if((p->conf1 & 0x0020) && (p->FileC & 0x02))  
    {
        p->Freq_Algo = 1;                                 
    }

    //interpret Clock stop behavior
    // modified by J. Yp-Tcha to integrate all the behaviors required by ETSI.
    // 18/11/2002 : TI Chip always allowed low level,
    // high level is hard dependant

    if ((p->FileC & SIM_CLK_STOP_MASK) == SIM_CLK_STOP_NOT_ALLWD) {
	/* Sim Clock Stop Not Allowed */
	SIM_sleep_status = SIM_SLEEP_NOT_ALLOWED;
	/* There is not need to modifiy p->conf1 */ 
        status_os_sim = NU_Delete_Timer (&SIM_timer);
    }
    else {
	if ((p->FileC & SIM_CLK_STOP_MASK) == SIM_CLK_STOP_ALLWD) {
	    /* Sim Clock Stop Allowed, no prefered level */
	    /* Default value for TI Chip shall always be Low Level */
            SIM_sleep_status = SIM_SLEEP_DESACT;
	    p->c->conf1 = p->conf1 &= ~SIM_CONF1_SCLKLEV;
	}
	else {
	    /* Clock Stop is allowed, the level shall be checked */
	    if ((p->FileC & SIM_CLK_STOP_HIGH) == SIM_CLK_STOP_HIGH) {
		/* high level is mandatory */
/* OMEGA/NAUSICA can not handle sim stop clock at high level */
#ifndef ANALOG1
                SIM_sleep_status = SIM_SLEEP_DESACT;
    	        p->c->conf1 = p->conf1 |= SIM_CONF1_SCLKLEV;
#else
		/*
		 * Sim Clock Stop Not Allowed because the interface
		 * do not support this level
		 */
		SIM_sleep_status = SIM_SLEEP_NOT_ALLOWED;
		/* There is not need to modifiy p->conf1 */ 
	        status_os_sim = NU_Delete_Timer (&SIM_timer);
#endif
	    }
	    else {
		/* by default, Low Level is allowed */
                SIM_sleep_status = SIM_SLEEP_DESACT;
                p->c->conf1 = p->conf1 &= ~SIM_CONF1_SCLKLEV;
	    }
	}
    }
    if (SIM_sleep_status == SIM_SLEEP_NONE)
    {
        status_os_sim = NU_Delete_Timer (&SIM_timer);
    }
}

/*
* Use to evaluate need of sending PTS procedure regarding
* the ATR. If default not used, PTS initiates F and D adequate values
* for speed enhancement.
* In case of 2 wrong PTS answer (speed enhanced), a third PTS with default value
* is used. If the third PTS attempt failed, the ME reset the SIM and use default
* value.
*  Return Value : SIM_ERR_READ, SIM_ERRCARDREJECT, SIM_ERR_WAIT
 *
*/


SYS_UWORD16 SIM_PTSprocedure(SIM_CARD *cP, SIM_PORT *p)
{

    SYS_UWORD8            TA1;
    SYS_UWORD8            n;
    SYS_UWORD8            err;

    p->xbuf[0]       = 0xFF;                    //character of PTS proc to send
    p->xbuf[1]       = 0; 
    p->xbuf[2]       = 0xFF;
    p->xbuf[3]       = 0x7B;

    //TA1,TB1,TC1,TD1 present in ATR ?

    n   = 3;

    p->PTS_Try++;

    if (p->PTS_Try > 4)
    {
        return (SIM_ERR_CARDREJECT);  
    }                       // at the fourth attempt,
			    // PTS procedure is unusefull. Use default value.
                            //TA1 present?  Test 27.11.2.6
    else if ( p->PTS_Try == 4)
    {
        SIM_Calcetu (p);
        return (0);    
    }
    
    if(cP->AtrData[1] & 0x10)
    {
        TA1 = cP->AtrData[2];
    }
    else                    //if TA1 not present, return
    {
        SIM_Calcetu (p);
        return (0);
    }

#if 0 // Dmitriy: removed by TI patch
    if (TA1 >= 0x94)        //speed enhancement
    {
// JYT 26/9/2003 to check correct behavior of the SIM Driver vs the PPS.
//#ifdef NOTTOLOADBECAUSENOTTESTED
//	SIM_Calcetu (p);
//        return (0);         //temporary disabling of speed enhancement feature

        if (p->PTS_Try <= 2)
        {
            n = 4;
            p->xbuf[1] = 0x10;
            p->xbuf[2] = 0x94; // if speed enhancement, then at least (and at most) F = 512 and D = 8 is supported
        }
//#endif
    }
#endif

    if ((TA1 == 0x11) || (TA1 == 0x01))
    {
        SIM_Calcetu (p);
        return (0);
    }                       //if TA1 != 0x11 and 0x94, need to send PTS request
                            //transmit request of speed enhancement : PTS
    SIM_WriteBuffer(p, 0, n);    

    p->moderx = 0;          //mode of normal reception
    p->expected_data = n;

    if (err = SIM_Waitforchars (p, p->etu9600))
    {
      return (err);
    }
                            //should received same chars as PTS request
    if ((p->rbuf[0] != p->xbuf[0]) || (p->rbuf[1] != p->xbuf[1]) || 
    (p->rbuf[2] != p->xbuf[2])) 
    {
      return(SIM_ERR_READ);
    }

    if (n == 4)
    {
        if (p->rbuf[3] != p->xbuf[3])
        {
            return(SIM_ERR_READ);
        }   
        
        //correct response from SIM : with speed enhanced
        p->c->conf1 = p->conf1 |= SIM_CONF1_ETU; //set F=512 D=8
    }

    SIM_Calcetu (p);
    return (0);
}

/*
* procedure of WARM reset consists on asserting
* reset signal at 0 during at least 400 ETU
* input     p pointer of type SIM_PORT
*/

void SIM_WARMReset (SIM_PORT *p)
{
    p->c->conf1 = p->conf1 &= ~SIM_CONF1_SRSTLEV;
    ind_os_sleep (p->etu400);  /// wait 400 ETU
    p->c->conf1 = p->conf1 |= SIM_CONF1_SRSTLEV;
    p->rx_index = 0;
}


/*
* procedure use to get out sleepMode
* input     p pointer of type SIM_PORT
*/

void SIM_SleepMode_In (SYS_UWORD32 param)
{
    if (SIM_sleep_status == SIM_SLEEP_DESACT)
    {
        (&(Sim[0]))->c->conf1 &= ~SIM_CONF1_SCLKEN;  //disabled the clock for the SIM card
        SIM_sleep_status = SIM_SLEEP_ACT;
    }
    status_os_sim = NU_Control_Timer (&SIM_timer,  NU_DISABLE_TIMER);
}

/*
* procedure use to get out sleepMode
* input     p pointer of type SIM_PORT
*/

void SIM_SleepMode_Out (SIM_PORT *p)
{
    if (SIM_sleep_status == SIM_SLEEP_ACT)
    {
        p->c->conf1 = p->conf1 |= SIM_CONF1_SCLKEN;
        // WCS patch for NU_Sleep(0) bug
        if (p->startclock > 0)
		ind_os_sleep (p->startclock);    
        // End WCS patch
        SIM_sleep_status = SIM_SLEEP_DESACT;
    }
}
               
/*
*  procedure to parse ATR dynamically
*  input     p pointer of type SIM_PORT
*  output    return error code
*  SIM_ERR_WAIT, p->errorSIM
*  SIM_ERR_CARDREJECT,
*
*
*/

SYS_UWORD16 SIM_ATRdynamictreatement (SIM_PORT *p, SIM_CARD *cP)
{
    volatile SYS_UWORD8   HistChar;
    volatile SYS_UWORD8   InterfChar;
    SYS_UWORD16          countT;
    SYS_UWORD16          mask;
    SYS_UWORD16          returncode;
    SYS_UWORD8            i;
    SYS_UWORD8            firstprotocol;    
    SYS_UWORD8            Tx,T;
    SYS_UWORD8            TDi;
    SYS_UWORD8            position_of_TC1, position_of_TB1;
    SYS_UWORD8            another_protocol_present;
    SYS_UWORD16            wait80000clk;
        
    i               = 0;
    //wait for TS and T0
    p->moderx       = 0;
    p->expected_data= 1;
    firstprotocol   = 0;
    position_of_TC1 = 0;
    position_of_TB1 = 0;
    another_protocol_present = 0;
    wait80000clk    = 6; // > 24 ms

    //wait for first character TS of ATR sequence. It should arrive before 80000sclk
    if (returncode = SIM_Waitforchars (p, wait80000clk))
    {
        return returncode;
    }
    
    //wait for T0
    p->expected_data++; 
    if (returncode = SIM_Waitforchars (p, p->etu9600))
    {
        return returncode;
    }

    ind_os_sleep(220);

    if (((p->rbuf[0] & 0xF0) == 0x30) && (p->rx_index != 0))
    {
        cP->Inverse = 0;
    }
        /*-----------------------------------------------------------*/
        /*              Inverse convention card                      */
            // If first byte is correct for inverse card, return success 
    else if (((p->rbuf[0] & 0x0F) == 0x03) && (p->rx_index != 0))
    {
        cP->Inverse = 1;
    }
    else
    {
        return (SIM_ERR_CARDREJECT);  //Test 27.11.2.4.5
    }

    countT          = 0;
    mask            = 0x10;
    InterfChar      = 2;
    TDi             = 1;


    Tx = SIM_Translate_atr_char (p->rbuf[1], cP);

    HistChar        = Tx & 0x0F;        //get K, number of transmitted historical character

    
    while (TDi != 0)
    {
        while (mask < 0x100)            //monitors interface chars
        {
            if ((Tx & mask) == mask)    //monitors if interface character TAx,TBx,TCx,TDc present
            {
                InterfChar++;
            }
                                        //wait for TC1 and save its position
            if ((firstprotocol == 0) && ((Tx & 0x40) == mask))
            {
                position_of_TC1 = InterfChar - 1;            
            }
	    if ((firstprotocol == 0) && ((Tx & 0x20) == mask))
            {
                position_of_TB1 = InterfChar - 1;            
            }

            mask = mask << 1;
        }

        p->expected_data = InterfChar;     //wait for TAi,TBi,TCi,TDi if present
        
        if (returncode = SIM_Waitforchars (p, p->etu9600))
        {
            return returncode;
        }

        //need to monitor if TC1 present and if equal to 0 or 255
	// on first protocol
        if ((firstprotocol == 0) && (position_of_TC1 != 0))
        {
            T = SIM_Translate_atr_char (p->rbuf[position_of_TC1], cP);

            if ((T != 0) && (T != 255)) //test 27.11.1.3
            {                           //return Error in case of bad TC1 value            
                return (SIM_ERR_CARDREJECT);            
            }
        }
        //need to monitor if TB1 present and if differente from 0
	// on first protocol
        if ((firstprotocol == 0) && (position_of_TB1 != 0))
        {
            T = SIM_Translate_atr_char (p->rbuf[position_of_TB1], cP);

            if (T != 0) //ITU 
            {                           //return Error in case of bad TB1 value            
                return (SIM_ERR_CARDREJECT);            
            }
        }

        if ((Tx & 0x80) == 0x80)        //TDi byte on first protocol must be 0
        {                               //get new TD char
            Tx = SIM_Translate_atr_char (p->rbuf[InterfChar - 1], cP);
          
            if ((Tx & 0x0F) != 0)
            {
                if (firstprotocol == 0) //if first protocol received is not T=0, card is rejected   
                {
                    return (SIM_ERR_CARDREJECT);            //protocol other than T=0
                }    
                else
                {                       //if an another protocol T != 0 present, need to wait for TCK char
                    another_protocol_present = 1;                
                }
            }
            mask = 0x10;    
            firstprotocol++;            //indicate another protocol T
        }
        else
        {
            TDi = 0;
        }
    }
                                        //add TCK if necessary
    p->expected_data =  HistChar + InterfChar + another_protocol_present;
 
    if (returncode = SIM_Waitforchars (p, p->etu9600))
    {
        return returncode;
    }

    cP->AtrSize = p->rx_index;

    if (cP->Inverse)        //inverse card
    {
        // Copy ATR data       
        for (i=0;i<cP->AtrSize;i++)
        {
            cP->AtrData[i] = SIM_ByteReverse(p->rbuf[i]);    
        }
        p->c->conf1 = p->conf1 |= SIM_CONF1_CONV | SIM_CONF1_CHKPAR;
    }
    else                    //direct card
    {
        p->c->conf1 = p->conf1 |= SIM_CONF1_CHKPAR; //0x0409
        // Copy ATR data       
        for (i=0;i<cP->AtrSize;i++)
        {
            cP->AtrData[i] = p->rbuf[i];
        }
    }

    return (0);
}

/*
 ** SIM_Translate_atr_char 
 *
 *  FILENAME: sim.c
 *
 *  PARAMETERS: input   char to translate
 *              cP      sim structure (indicates if inverse card present)
 *  DESCRIPTION: return the correct value of input for inverse card
 *
 *  RETURNS: character after parsing
 *           stays the same if direct card
 */

SYS_UWORD8 SIM_Translate_atr_char (SYS_UWORD8 input, SIM_CARD *cP)
{
    SYS_UWORD8 translated;

    if (cP->Inverse)
    {
        translated = SIM_ByteReverse(input);    
    }
    else
    {
        translated = input;           //get character next char T0
    }
    return (translated);
}

/*
* SIM_Waitforchars is used for waiting nbchar characters from SIM
* input p          sim port
*       max_wait   max number of TDMA to wait between 2 characters
* output
*       error code 0 if OK      
*/

SYS_UWORD16 SIM_Waitforchars (SIM_PORT *p, SYS_UWORD16 max_wait)
{
    volatile SYS_UWORD8    old_nb_char;
    volatile SYS_UWORD16  countT;

    if (p->moderx == 6)                 //use for reception of ACK when command need to transmit rest of data
    {
        p->ack = 0;
        countT = 0;

        while((p->ack == 0) && (p->moderx == 6)) 
        {                               //if p->moderx change from 6 to 5, need to wait for SW1 and SW2

            ind_os_sleep(1);
            countT++;                   //implementation of software Waiting time overflow

            if (p->null_received)       //if NULL char received, wait for next procedure char
            {
                countT = 0;
                p->null_received = 0;
            }

            if (countT > max_wait)
            {
                return (SIM_ERR_WAIT);
            }
            if (p->errorSIM)
            {
                return(p->errorSIM);
            }
        }
        if (p->moderx == 6)             //if transition to moderx = 5 in synchronous part
        {                               //need to quit for SW1/SW2 reception
            return (0);
        }    
    }
    
    if ((p->moderx != 6) && (p->moderx != 5))   //treatement of mode 0, 1, 2, 3, 4
    {        
        countT = 0;
        old_nb_char = p->rx_index;
        //leave while if moderx == 5
        while((p->rx_index < p->expected_data) && (p->moderx != 5)) 
        {
            ind_os_sleep(1);
            countT++;                   //implementation of software Waiting time overflow

            if (p->null_received)       //if NULL char received, wait for next procedure char
            {
                countT = 0;
                p->null_received = 0;
            }

            if (countT > max_wait)
            {
                return (SIM_ERR_WAIT);
            }
            if (p->errorSIM)
            {
                return(p->errorSIM);
            }
            if (p->rx_index > old_nb_char)
            {
                old_nb_char = p->rx_index;  //if char received before max_wait TDMA, reset the counter
                countT = 0;
            }
        } //end while
		if (p->moderx == 0)
		{
			return (0);
		}
    }

    if (p->moderx == 5)                 //use for reception of SW1 SW2
    {
        countT = 0;
        old_nb_char = p->SWcount;

        while(p->SWcount < 2) 
        {                               //if p->moderx change from 6 to 5, need to wait for SW1 and SW2

            ind_os_sleep(1);
            countT++;                   //implementation of software Waiting time overflow

            if (p->null_received)       //if NULL char received, wait for next procedure char
            {
                countT = 0;
                p->null_received = 0;
            }

            if (countT > max_wait)
            {
                return (SIM_ERR_WAIT);
            }
            if (p->errorSIM)
            {
                return(p->errorSIM);
            }
            if (p->SWcount > old_nb_char)
            {
                old_nb_char = p->SWcount;  //if char received before max_wait TDMA, reset the counter
                countT = 0;
            }
        }
        p->SWcount = 0;                 //reset SWcount buffer index when SW1 SW2 received
        return (0);
    }
    else		//treatement of abnormal case of the asynchronous state machine
    {
	return (SIM_ERR_ABNORMAL_CASE1);
    }
}

/*
* SIM_Calcetu is used for calculating 9600 etu and 400 etu depending on sim clock freq
*             and etu period
* input p     sim port
*/

void SIM_Calcetu (SIM_PORT *p)
{
    if (p->conf1 & SIM_CONF1_SCLKDIV)   //clock input is 13/8 Mhz
    {
        if (p->conf1 & SIM_CONF1_ETU)   //etu period is 512/8*Tsclk
        {
            p->etu9600     = 319;    // old = 88, increase of 363%
            p->etu400      = 6;
            p->stopclock   = 18;
            p->startclock = 8;
        }
        else                            //etu period is 372/1*Tsclk
        {
            p->etu9600     = 1815;   // old = 500, increase of 363%
            p->etu400      = 28;
            p->stopclock   = 94;
            p->startclock = 38; 
        }
    }
    else                                //clock input is 13/4 Mhz
    {
        if (p->conf1 & SIM_CONF1_ETU)   //etu period is 512/8*Tsclk
        {
            p->etu9600     = 159;   // old = 44, increase of 363%
            p->etu400      = 3;
            p->stopclock   = 9;
            p->startclock = 4; 
        }
        else                            //etu period is 372/1*Tsclk
        {
            p->etu9600     = 907;  // old = 250, increase of 363%
            p->etu400      = 14;
            p->stopclock   = 47;
            p->startclock = 19; 
        }
    }  
}

/*
 * Set the level shifter voltage for start up sequence
 *
 */

SYS_UWORD8 SIM_StartVolt (SYS_UWORD8 ResetFlag) 
{
    SYS_UWORD8 abbmask;

#if(ANALOG == 1)
// we assume that in SIM_TYPE_5V there is nothing to do because it is the reset value
  #if ((SIM_TYPE == SIM_TYPE_3V) || (SIM_TYPE == SIM_TYPE_3_5V))    // { shut down VCC from ABB and prepare to start at 3V mode
    if (ResetFlag) {
	abbmask = MODE_INIT_OMEGA_3V;
        CurrentVolt = SIM_3V;  // we assume the sim is 3v tech. from beginning.
    }
    else {
	if (CurrentVolt == SIM_3V)
	    abbmask = MODE_INIT_OMEGA_3V;
	else
	    abbmask = MODE5V_OMEGA;
    }
    ABB_wa_VRPC ((ABB_Read_Register_on_page(PAGE1,VRPCCTRL1) & 0xC0) | abbmask);
    ind_os_sleep(1);         //wait for charge pump regulation
    return(SIM_OK);
  #endif
#endif

#if(ANALOG == 2)
    SYS_UWORD8 count = 0;
// code for Iota
// reset value for IOTA is for 1.8V, but specific procedure is needed
  #if ((SIM_TYPE == SIM_TYPE_1_8V) || (SIM_TYPE == SIM_TYPE_1_8_3V))    // shut down VCC from ABB and prepare to start at 1.8V mode
    if (ResetFlag) {
	abbmask = MODE_INIT_IOTA_1_8V;
        // we assume the sim is 1.8v tech. from beginning.
        CurrentVolt   = SIM_1_8V;
    }
    else {
	if (CurrentVolt == SIM_1_8V)
	    abbmask = MODE_INIT_IOTA_1_8V;
	else
	    abbmask = MODE_INIT_IOTA_3V;
    }
    ABB_wa_VRPC ((ABB_Read_Register_on_page(PAGE1,VRPCSIM) & 0xF4) | abbmask);
    while(count++ < 5)
    {
	if (ABB_Read_Register_on_page(PAGE1,VRPCSIM) & 0x04) // test RSIMRSU
            return(SIM_OK);
        ind_os_sleep(1);	
    }
    // IOTA failure activation
    return(SIM_ERR_HARDWARE_FAIL);
  #endif
  // 3V only
  #if (SIM_TYPE == SIM_TYPE_3V)
    abbmask = MODE_INIT_IOTA_3V;
    CurrentVolt   = SIM_3V;  // we assume the sim is 3v tech. from beginning.
    ABB_wa_VRPC ((ABB_Read_Register_on_page(PAGE1,VRPCSIM) & 0xF4) | abbmask);
    while(count++ < 5)
    {
	if (ABB_Read_Register_on_page(PAGE1,VRPCSIM) & 0x04) // test RSIMRSU
            return(SIM_OK);
        ind_os_sleep(1);	
    }
    // IOTA failure activation
    return(SIM_ERR_HARDWARE_FAIL);
  #endif
#endif

#if(ANALOG == 3)
  SYS_UWORD8 count = 0;
// code for Syren
// reset value for SYREN is for 1.8V, but specific procedure is needed
  #if ((SIM_TYPE == SIM_TYPE_1_8V) || (SIM_TYPE == SIM_TYPE_1_8_3V))    // { shut down VCC from ABB and prepare to start at 1.8V mode
    if (ResetFlag) {
	abbmask = MODE_INIT_SYREN_1_8V;
        // we assume the sim is 1.8v tech. from beginning.
        CurrentVolt   = SIM_1_8V;
    }
    else {
	if (CurrentVolt == SIM_1_8V)
	    abbmask = MODE_INIT_SYREN_1_8V;
	else
	    abbmask = MODE_INIT_SYREN_3V;
    }
    ABB_wa_VRPC ((ABB_Read_Register_on_page(PAGE1,VRPCSIMR) & 0x1F4) | abbmask);
    while(count++ < 5)
    {
	if (ABB_Read_Register_on_page(PAGE1,VRPCSIMR) & 0x04) // test RSIMRSU
            return(SIM_OK);
        ind_os_sleep(1);
    }
    // SYREN failure activation
    return(SIM_ERR_HARDWARE_FAIL);
  #endif

  // 3V only
  #if (SIM_TYPE == SIM_TYPE_3V)
    abbmask = MODE_INIT_SYREN_3V;
    CurrentVolt   = SIM_3V;  // we assume the sim is 3v tech. from beginning.
    ABB_wa_VRPC ((ABB_Read_Register_on_page(PAGE1,VRPCSIMR) & 0x1F4) | abbmask);
    while(count++ < 5)
    {
      if (ABB_Read_Register_on_page(PAGE1,VRPCSIMR) & 0x04) // test RSIMRSU
            return(SIM_OK);
      ind_os_sleep(1);
    }
    // SYREN failure activation
    return(SIM_ERR_HARDWARE_FAIL);
  #endif
#endif
}


/*
 * Set the level shifter to switch from 3V to 5V
 *
 */

SYS_UWORD8 SIM_SwitchVolt (SYS_UWORD8 ResetFlag)
{
    SYS_UWORD8 count = 0;
    SYS_UWORD8 abbmask;

    SIM_PowerOff();

    #if(ANALOG == 1)
      #if (SIM_TYPE == SIM_TYPE_3_5V)    // shut down VCC from ABB and prepare to start at 5V mode
	if (ResetFlag) {
		abbmask = MODE5V_OMEGA;
		CurrentVolt   = SIM_5V;  
	}
	else {
		if (CurrentVolt == SIM_3V)
			abbmask = MODE_INIT_OMEGA_3V;
		else
			abbmask = MODE5V_OMEGA;
	}
	ABB_wa_VRPC ((ABB_Read_Register_on_page(PAGE1,VRPCCTRL1) & 0xC0) | abbmask);
        return(SIM_OK);
      #endif
    #elif(ANALOG == 2)
      #if (SIM_TYPE == SIM_TYPE_1_8_3V)  // shut down VCC from ABB and prepare to start at 3V mode
	if (ResetFlag) {
		abbmask = MODE_INIT_IOTA_3V;
		CurrentVolt   = SIM_3V; 
	}
	else {
		if (CurrentVolt == SIM_1_8V)
			abbmask = MODE_INIT_IOTA_1_8V;
		else
			abbmask = MODE_INIT_IOTA_3V;
	}
	ABB_wa_VRPC ((ABB_Read_Register_on_page(PAGE1,VRPCSIM) & 0xF4) | abbmask);
       while(count++ < 5)
       {
          if (ABB_Read_Register_on_page(PAGE1,VRPCSIM) & 0x04)
               return(SIM_OK);
           ind_os_sleep(1);	
       }
       // IOTA failure activation
       return(SIM_ERR_HARDWARE_FAIL);
      #endif
    #elif(ANALOG == 3)
      #if (SIM_TYPE == SIM_TYPE_1_8_3V)  // shut down VCC from ABB and prepare to start at 3V mode
	if (ResetFlag) {
		abbmask = MODE_INIT_SYREN_3V;
		CurrentVolt   = SIM_3V; 
	}
	else {
		if (CurrentVolt == SIM_1_8V)
			abbmask = MODE_INIT_SYREN_1_8V;
		else
			abbmask = MODE_INIT_SYREN_3V;
	}
	ABB_wa_VRPC ((ABB_Read_Register_on_page(PAGE1,VRPCSIMR) & 0x1F4) | abbmask);
        while(count++ < 5)
        {
          if (ABB_Read_Register_on_page(PAGE1,VRPCSIMR) & 0x04)
            return(SIM_OK);
          ind_os_sleep(1);
        }
        // SYREN failure activation
        return(SIM_ERR_HARDWARE_FAIL);
      #endif
    #endif	 // ANALOG == 1, 2, 3
}

SYS_UWORD8 SIM_Memcpy(SYS_UWORD8 *Buff_target, SYS_UWORD8 Buff_source[],
		      SYS_UWORD16 len)
{
    SYS_UWORD16 i;  //unsigned short type counter chosen for copy of 256 bytes

    for (i = 0; i < len; i++)
    {
         if (i == RSIMBUFSIZE) 
         {
               return (SIM_ERR_BUFF_OVERFL);
         }
         else
	 {
              (*(Buff_target+i)) = Buff_source[i];
         }
    }
    return (0);
}

/*
 * SIM_SleepStatus
 * 
 * Return SIM status for sleep manager
 * 
 */
SYS_BOOL SIM_SleepStatus(void)
{
	if ((SIM_sleep_status == SIM_SLEEP_ACT) ||
	    (SIM_sleep_status == SIM_SLEEP_NONE))
		return(1);		 // SIM is ready for deep sleep
	else 
		return(0);
}

/*
* Special lock function to force SIM entity to use adequat SIM Driver
*/
void SIM_lock_cr17689(void) {
}


#ifdef SIM_DEBUG_TRACE
void SIM_dbg_write_trace(SYS_UWORD8 *ptr, SYS_UWORD16 len) {
   SYS_UWORD16 i;
   for(i=0;i<len;i++) {
      if (SIM_dbg_cmd_cmpt == SIM_DBG_CMD)
	     SIM_dbg_cmd_cmpt = 0;
	  SIM_dbg_cmd[SIM_dbg_cmd_cmpt++] = ptr[i];
   }
}
#endif
