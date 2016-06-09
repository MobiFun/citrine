/*
 * SIM32.C
 *
 * Pole Star SIM
 *
 * Target : ARM
 *
 * Copyright (c) Texas Instruments 1995
 *
 */

#define SIM32_C   1

#include "../include/config.h"
#include "../include/sys_types.h"

#include "iq.h" 
#include "sim.h"


#ifdef SIM_DEBUG_TRACE
/* working buffer for NULL BYTE */
extern SYS_UWORD8  SIM_dbg_null[];
/* Nucleus variable given the current number of TDMA frames */
extern SYS_UWORD32 IQ_FrameCount;
/* working variable to calculate the TDMA ecart */
extern SYS_UWORD16 SIM_dbg_tdma_diff;
/* working variable storing the current number of TDMA frames elapsed */
SYS_UWORD32 SIM_dbg_local_count;
#endif

/*
 * SIM_IntHandler
 *
 * Read cause of SIM interrupt : 
 * 
 * if receive buffer full, read char
 * if transmitter empty, change direction, transmit a dummy char
 *
 */
void SIM_IntHandler(void)
{
   volatile unsigned short it, i, stat, conf1;
   volatile SYS_UWORD8 ins; 
   volatile SYS_UWORD8 rx;
   volatile SYS_UWORD8 nack;
   volatile SYS_UWORD8 nack1;

   
   SIM_PORT *p;

   p = &(Sim[0]);

   p->rxParityErr = 0;   
   it = p->c->it;

   if ((it & SIM_IT_ITRX) && !(p->c->maskit & SIM_MASK_RX)) // int on reception
   {
      stat = p->c->rx;
      conf1 = p->conf1;

#ifdef SIM_DEBUG_TRACE
      if ((IQ_FrameCount - SIM_dbg_local_count) > SIM_dbg_tdma_diff) {
         SIM_dbg_tdma_diff = IQ_FrameCount - SIM_dbg_local_count;
      }
      SIM_dbg_local_count = IQ_FrameCount;
#endif

          // Check if reception parity is enable
      if (((conf1 & SIM_CONF1_CHKPAR) && ((stat & SIM_DRX_STATRXPAR) != 0))\
       || ((conf1 & SIM_CONF1_CHKPAR) == 0))
      {
          rx    = (SYS_UWORD8) (stat & 0x00FF);
          ins   = p->xbuf[1] & p->hw_mask;
          nack  = (~p->xbuf[1]) & p->hw_mask;
            
          switch (p->moderx)
          {
              case 0:                  //mode of normal reception without proc char (like PTS proc)
                  p->rbuf[p->rx_index++] = rx;
                  break;      

              case 1:                  //mode wait for ACK
                  if ((rx & p->hw_mask) == ins)
                  {
                      p->moderx = 2;
                  }  
                  else if ((rx & p->hw_mask) == nack)
                  {
                      p->moderx = 4;          
                  }
                  else if (((rx & 0xF0) == 0x60) || ((rx & 0xF0) == 0x90))
                  {
                      if (rx != 0x60)     //in case of error code (SW1/SW2) returned by sim card
                      {
                          p->rSW12[p->SWcount++] = rx;
                          p->moderx = 5;
                      }
                      else
                      {
                          p->null_received = 1;
#ifdef SIM_DEBUG_TRACE
			  SIM_dbg_null[0]++;
#endif
                      }
                  }
                  else
                  {
                    p->errorSIM = SIM_ERR_ABNORMAL_CASE2;
                  }  
                                        //if rx = 0x60 wait for ACK
                  break;      

              case 2:                  //mode reception by block
                  p->rbuf[p->rx_index++] = rx;

                  if(p->expected_data == 256)
                  {
                      if (p->rx_index == 0)
                      {
                          p->moderx = 5;
                      }
                  }	
                  else
                  {  
                  if (p->rx_index == p->expected_data)
                  {
                      p->moderx = 5;
                  }
                  }
                  break;      
  
              case 3:                  //mode reception char by char. reception of proc char
                  if ((rx & p->hw_mask) == ins)
                  {
                      p->moderx = 2;
                  }            
                  else if ((rx & p->hw_mask) == nack)
                  {
                      p->moderx = 4;          
                  }                      //if rx = 0x60 wait for ACK  
                  else if (rx == 0x60)
                  {
                      p->null_received == 1;
#ifdef SIM_DEBUG_TRACE
		      SIM_dbg_null[1]++;
#endif
                  }  
                    
                  break;

              case 4:                  //mode reception char by char. reception of data
                  p->rbuf[p->rx_index++] = rx;
                  p->moderx = 3;        //switch to receive proc char mode      

                  if(p->expected_data == 256)
                  {
                      if (p->rx_index == 0)
                      {
                          p->moderx = 5;
                      }
                  }
                  else
                  {  
                      if (p->rx_index == p->expected_data)
                      {
                          p->moderx = 5;
                      }
                  }
                  break;

              case 5:                  //mode wait for procedure character except NULL
                  if ((rx != 0x60) || (p->SWcount != 0))  //treat NULL character only if arriving before SW1 SW2
                  {
                      p->rSW12[p->SWcount++] = rx;
                  }
                  else
                  {
                      p->null_received = 1;
#ifdef SIM_DEBUG_TRACE
		      SIM_dbg_null[2]++;
#endif
                  }
                  break;

              case 6:                  //give the acknowledge char
                  if (((rx & 0xF0) == 0x60) || ((rx & 0xF0) == 0x90))
                  {
                      if (rx != 0x60)       //in case of error code (SW1/SW2) returned by sim card
                      {
                          p->rSW12[p->SWcount++] = rx;
                          p->moderx = 5;
                      }
                      else
                      {
                          p->null_received = 1;
#ifdef SIM_DEBUG_TRACE
		          SIM_dbg_null[3]++;
#endif
                      }
                  }
                  else
                  {                     
                     p->ack = rx;
                  }
          }
      }
      else
      {
        p->rxParityErr = 1;
      }
   }

   if ((it & SIM_IT_ITTX) && !(p->c->maskit & SIM_MASK_TX))
   {
#ifdef SIM_DEBUG_TRACE
      SIM_dbg_local_count = IQ_FrameCount;
#endif
      // check the transmit parity
      stat = p->c->stat;


      if ((stat & SIM_STAT_TXPAR) || ((p->conf1 & SIM_CONF1_CHKPAR) == 0))  //parity disable
      {
         if (p->xOut != (p->xIn - 1))       //if only one char transmitted (already transmitted)
         {                                  //just need to have confirmation of reception
             if (p->xOut == (p->xIn - 2))
             {
                p->xOut++;
                p->c->tx = *(p->xOut);         // transmit

                p->conf1 &= ~SIM_CONF1_TXRX;   // return the direction
                p->c->conf1 = p->conf1;
             }
             
             if (p->xOut < (p->xIn - 2))
             {
                p->xOut++;
                p->c->tx = *(p->xOut);         // transmit
             }                                 
         }   
      }
      else
      {
         p->c->tx = *(p->xOut);            // transmit same char
         p->txParityErr++;                 // count number of transmit parity errors 
      }

   }

   // Handle errors
   if ((it & SIM_IT_ITOV) && !(p->c->maskit & SIM_MASK_OV))
   {
      p->errorSIM = SIM_ERR_OVF;
      
   }
   if ((it & SIM_IT_WT) && !(p->c->maskit & SIM_MASK_WT))
   {
      p->errorSIM = SIM_ERR_READ;
   }

   // Reset the card in case of NATR to let the program continue
   if ((it & SIM_IT_NATR) && !(p->c->maskit & SIM_MASK_NATR))
   {
      p->c->cmd = SIM_CMD_STOP;
      p->errorSIM = SIM_ERR_NATR;
   }

#if ((CHIPSET == 2) || (CHIPSET == 3))
   // SIM card insertion / extraction
   if ((it & SIM_IT_CD) && !(p->c->maskit & SIM_MASK_CD))
   {
      stat = p->c->stat;
      if ((stat & SIM_STAT_CD) != SIM_STAT_CD)
      {
        (p->RemoveFunc)();
        p->errorSIM = SIM_ERR_NOCARD;
      }
   }
#endif
}

#if ((CHIPSET == 4) || (CHIPSET == 5) || (CHIPSET == 6) || (CHIPSET == 7) || (CHIPSET == 8) || (CHIPSET == 9) || (CHIPSET == 10) || (CHIPSET == 11) || (CHIPSET == 12))
  /*
   * SIM_CD_IntHandler
   *
   * Read cause of SIM interrupt : 
   * 
   */
  void SIM_CD_IntHandler(void)
  {
    volatile unsigned short it_cd, stat;
    SIM_PORT *p;

    p = &(Sim[0]);

    p->rxParityErr = 0;   
    it_cd = p->c->it_cd;

    // SIM card insertion / extraction
    if ((it_cd & SIM_IT_CD) && !(p->c->maskit & SIM_MASK_CD))
    {
      stat = p->c->stat;
      if ((stat & SIM_STAT_CD) != SIM_STAT_CD)
      {
        (p->RemoveFunc)();
        p->errorSIM = SIM_ERR_NOCARD;
      }
    }
}
#endif


// to force this module to be linked
SYS_UWORD16 SIM_Dummy(void)
{
   
}
