/************* Revision Controle System Header *************
 *                  GSM Layer 1 software
 * L1_CTL.C
 *
 *        Filename l1_ctl.c
 *  Copyright 2003 (C) Texas Instruments
 *
 ************* Revision Controle System Header *************/

#define  L1_CTL_C

#include "l1_macro.h"
#include "l1_confg.h"

#if (CODE_VERSION == SIMULATION)
  #include <string.h>
  #include "l1_types.h"
  #include "sys_types.h"
  #include "l1_const.h"
  #include "l1_time.h"
  #include "l1_signa.h"

  #if TESTMODE
    #include "l1tm_defty.h"
  #endif
  #if (AUDIO_TASK == 1)
    #include "l1audio_const.h"
    #include "l1audio_cust.h"
    #include "l1audio_signa.h"
    #include "l1audio_defty.h"
    #include "l1audio_msgty.h"
  #endif
  #if (L1_GTT == 1)
    #include "l1gtt_const.h"
    #include "l1gtt_defty.h"
  #endif
  #if (L1_MP3 == 1)
    #include "l1mp3_defty.h"
  #endif
  #if (L1_MIDI == 1)
    #include "l1midi_defty.h"
  #endif
//ADDED FOR AAC
  #if (L1_AAC == 1)
    #include "l1aac_defty.h"
  #endif
  #include "l1_defty.h"
  #include "cust_os.h"
  #include "l1_msgty.h"
  #include "l1_varex.h"
  #include "l1_proto.h"
  #include "l1_mftab.h"
  #include "l1_tabs.h"
  #include "l1_ver.h"
  #if L2_L3_SIMUL
    #include "hw_debug.h"
  #endif

  #if TESTMODE
    #include "l1tm_msgty.h"
    #include "l1tm_varex.h"
  #endif

  #include "l1_ctl.h"

  #ifdef _INLINE
    #define INLINE static inline // Inline functions when -v option is set
  #else                          // when the compiler is ivoked.
    #define INLINE
  #endif
#else
  #include <string.h>
  #include "l1_types.h"
  #include "sys_types.h"
  #include "l1_const.h"
  #include "l1_time.h"
  #include "l1_signa.h"

  #if (RF_FAM == 61)
      #include "tpudrv61.h"
  #endif

  #if TESTMODE
    #include "l1tm_defty.h"
  #endif
  #if (AUDIO_TASK == 1)
    #include "l1audio_const.h"
    #include "l1audio_cust.h"
    #include "l1audio_defty.h"
  #endif
  #if (L1_GTT == 1)
    #include "l1gtt_const.h"
    #include "l1gtt_defty.h"
  #endif
  #if (L1_MP3 == 1)
    #include "l1mp3_defty.h"
  #endif
  #if (L1_MIDI == 1)
    #include "l1midi_defty.h"
  #endif
//ADDED FOR AAC
  #if (L1_AAC == 1)
    #include "l1aac_defty.h"
  #endif
  #include "l1_defty.h"
  #include "cust_os.h"
  #include "l1_msgty.h"
  #include "l1_varex.h"
  #include "l1_proto.h"
  #include "l1_tabs.h"
  #include "l1_ctl.h"
  #if L2_L3_SIMUL
    #include "hw_debug.h"
  #endif

  #if TESTMODE
    #include "l1tm_msgty.h"
    #include "l1tm_varex.h"
  #endif
  #ifdef _INLINE
    #define INLINE static inline // Inline functions when -v option is set
  #else                          // when the compiler is ivoked.
    #define INLINE
  #endif
#endif

#if(RF_FAM == 61)
  #include "l1_rf61.h"
#endif

#if (TRACE_TYPE == 1) || (TRACE_TYPE == 4)
  #include "l1_trace.h"
#endif

extern SYS_UWORD16 Convert_l1_radio_freq(SYS_UWORD16 radio_freq);
#if(RF_FAM == 61)
extern WORD16 drp_gain_correction(UWORD16 arfcn, UWORD8 lna_off, UWORD16 agc);
#endif

#define LNA_OFF  1
#define LNA_ON   0




/************************************/
/* Automatic frequency compensation */
/************************************/

/*
 * FreeCalypso TCS211 reconstruction: the following 3 functions
 * have been added in the LoCosto version of this module.
 * We have conditioned them out in order to match the original
 * TCS211 object; their uses have been conditioned out as well.
 *
 * These functions will need to re-enabled when their uses are
 * re-enabled.
 */

#if 0

#define L1_WORD16_POS_MAX (32767)
#define L1_WORD16_NEG_MAX (-32768)
#define L1_WORD32_POS_MAX ((unsigned long)(1<<31)-1)
#define L1_WORD32_NEG_MAX (-(unsigned long)(1<<31))

INLINE WORD16 Add_Sat_sign_16b(WORD16 val1, WORD16 val2)
{
  WORD32 temp;
  WORD16 result;

  temp = (WORD32)((WORD32)val1 + (WORD32)val2);
  if(temp > L1_WORD16_POS_MAX)
  {
    temp = L1_WORD16_POS_MAX;
  }
  if(temp < L1_WORD16_NEG_MAX)
  {
    temp = L1_WORD16_NEG_MAX;
  }
  result = (WORD16)((temp)&(0x0000FFFF));
  return(result);
}

INLINE WORD32 Add_Sat_sign_32b(WORD32 val1, WORD32 val2)
{
  WORD32 temp_high_high;
  UWORD32 temp_low_low;
  UWORD16 carry;
  WORD32 result;
  WORD16  high_val1, high_val2;
  UWORD16 low_val1, low_val2;

  high_val1 = (WORD16)(val1>>16);
  high_val2 = (WORD16)(val2>>16);
  low_val1  = (UWORD16)(val1&0x0000FFFF);
  low_val2  = (UWORD16)(val2&0x0000FFFF);

  temp_high_high = (WORD32)high_val1 + (WORD32)high_val2;
  temp_low_low   = (UWORD32)low_val1 + (UWORD32)low_val2;
  carry = (UWORD16)(temp_low_low >> 16);
  temp_high_high = temp_high_high + (UWORD32)(carry);


  result = val1 + val2;
  if(temp_high_high >  L1_WORD16_POS_MAX)
  {
    result = L1_WORD32_POS_MAX;
  }
  if(temp_high_high < L1_WORD16_NEG_MAX)
  {
    result = L1_WORD32_NEG_MAX;
  }

  return(result);
}

INLINE WORD32 Sat_Mult_20sign_16unsign(WORD32 val1, UWORD32 val2)
{
  WORD32 result;

  result = val1 * val2;
  if(val1>0) /*  val2 is > 0*/
  {
    if(result < 0) /* overflow */
    {
      result = L1_WORD32_POS_MAX;
    }
  }
  if(val1<0) /*  val2 is > 0*/
  {
    if(result > 0) /* overflow */
    {
      result = L1_WORD32_NEG_MAX;
    }
  }
  return(result);
}
#endif

INLINE WORD32 Add_40b( WORD32 guard1guard2, WORD32 lvar1, WORD32 lvar2, WORD16 *guardout )
{
  WORD32  result, temp, carry, Lvar1, Lvar2;
  WORD16  guard1,guard2;

  guard1=(WORD16) ((WORD32) guard1guard2>>16);
  guard2=(WORD16) guard1guard2;

  /* lvar1 and lvar2 are both 48 bits variables              */
  /* We 1st add the low parts of lvar1 and lvar2 and we give */
  /* a 32 bits result and a carry if needed                  */
  Lvar1 = (UWORD16)lvar1;
  Lvar2 = (UWORD16)lvar2;

  temp = Lvar1 + Lvar2;

  carry = temp >> 16;

  result = temp & 0x0000ffffL;

  /* We now add the two high parts of var1 and var2 (scaled */
  /* to a 16 bits format) and carry (if any) and we give a  */
  /* 48 bits results.                                       */
  Lvar1 = (UWORD32)lvar1 >> 16;
  Lvar2 = (UWORD32)lvar2 >> 16;

  temp = Lvar1 + Lvar2 + carry;

  carry = (UWORD32)temp >> 16;

  temp = (UWORD32)temp << 16;

  result = result | temp;

  temp = guard1 + guard2 + carry;

  *guardout = (WORD16)temp;

  return( result );
}


INLINE WORD32 Mult_40b(WORD32 var1, WORD16 var2, WORD16 *guardout)
{
  WORD32   mult,guard1guard2;
  WORD32   aux1;
  UWORD32  aux2;
  WORD16   neg_flag=0;
  WORD32   var1_low_nosign,var2_nosign;

  if (var2<0)
  {
    var2=-var2;
    neg_flag=1;
  }

  /*aux1  = AccHigh(var1)*var2 */
  aux1 = (WORD32)(var1>>16) * (WORD32)var2;
                   /* 16 bits * 16 bits -> 32 bits result */

  /*aux2  = AccLow(var1)*var2  (unsigned multiplication)  */
  /* Performs the sign suppression of the words           */
  var1_low_nosign  = (UWORD16)var1;
  var2_nosign      = (UWORD16) var2;

  aux2 = (UWORD32)var1_low_nosign * (UWORD32)var2_nosign;

  /*Shift aux1=F48 of 16 bit left */
  guard1guard2=aux1&0xFFFF0000L;/*guard1=(WORD16)(aux1>>16)*/
                                /*guard2=0x0000           */
  aux1=aux1<<16;


  /* ((var1_high*var2)<<16) +(var1_low*var2) = aux1 + aux2 */
  /* aux1 and aux2 are both 48 bits variables              */
  /* We first add the low pats of aux1 and aux2 and we give*/
  /* a 32 bits result and a carry if needed                */
  mult=Add_40b(guard1guard2,aux1,aux2,guardout );

  if (neg_flag)
  {
   mult=-mult;
   if (*guardout!=0)
    *guardout=-(*guardout)-1;
   else
    *guardout=-1;
  }

  return(mult);
}


/***********************************************************************/
/* This function allows to multiply a WORD32 and a WORD16, both POSITIVE, */
/* variables. Result is WORD32.                                          */
/***********************************************************************/
INLINE WORD32 UMult_40b(WORD32 var1, WORD16 var2, WORD16 *guardout)
{
  WORD32   mult,guard1guard2;
  UWORD32  aux1,aux2;
  WORD32   var1_high_nosign,var1_low_nosign,var2_nosign;


  /*aux1  = AccHigh(var1)*var2  (unsigned multiplication) */
  /* Performs the sign suppression of the words           */
  var1_high_nosign = (UWORD32)var1>>16;
  var2_nosign      = (UWORD16) var2;

  aux1 = (UWORD32)var1_high_nosign * (UWORD32)var2_nosign;

  /*aux2  = AccLow(var1)*var2  (unsigned multiplication)  */
  /* Performs the sign suppression of the words           */
  var1_low_nosign  = (WORD32)((UWORD16)var1);

  aux2 = (UWORD32)var1_low_nosign * (UWORD32)var2_nosign;

  /*Shift aux1=F48 of 16 bit left */
  guard1guard2=aux1&0xFFFF0000L;/*guard1=(WORD16)(aux1>>16)*/
                                /*guard2=0x0000           */
  aux1=aux1<<16;


  /* ((var1_high*var2)<<16) +(var1_low*var2) = aux1 + aux2 */
  mult=Add_40b(guard1guard2,aux1,aux2,guardout);

  return(mult);
}


/*-------------------------------------------------------*/
/* l1ctl_afc()                                           */
/*-------------------------------------------------------*/
/* Parameters :                                          */
/* Return     :                                          */
/* Functionality :                                       */
/*-------------------------------------------------------*/
#if (VCXO_ALGO == 0)
WORD16 l1ctl_afc (UWORD8 phase, UWORD32 *frame_count, WORD16 angle, WORD32 snr, UWORD16 radio_freq)
#else
WORD16 l1ctl_afc (UWORD8 phase, UWORD32 *frame_count, WORD16 angle, WORD32 snr, UWORD16 radio_freq, UWORD32 l1_mode)
#endif
{
 /*************************/
 /* Variables declaration */
 /*************************/
        WORD16  i=0;
        UWORD32 denom;             /* F12.20 */
        WORD32  var_32,num,Phi_32=0,var1,var2,guard1guard2;
 static UWORD32 P=C_cov_start;             /* F12.20 */
 static WORD32  Psi=0;                     /* F13.19 */
 static WORD16  Psi_quant[C_N_del+1];      /* F13.3  */
        WORD16  var_16;
        WORD16  Phi=0;                     /* F1.15  */
        WORD16  quotient,guard1,guard2,guardout;
        UWORD32 LGuard;
        WORD16  denomH,denomH_3msb;
        UWORD32 K=0;                           /* algo 1 */

 static WORD16  old_Psi_quant[C_N_del+1];
 static WORD32  old_Psi=0;

#if (VCXO_ALGO == 1)
    static WORD32  psi_past[C_N_del+1];        /* F13.19 */
    static WORD16  psi_quant;                  /* F13.3  */
    static WORD16  quant_avg;
    static UWORD32 M_Count;
    static WORD32  psi_avg[C_PSI_AVG_SIZE_D+1]; // Data history array
    static WORD16  B_Count;                     // Counter for consecutive SNR below C_thr_snr
    #if 0	/* LoCosto added var */
      UWORD16 L = 10433;                          // Gain algo2
    #endif
    static UWORD16 first_avg;
    static UWORD16 good_snr;

    /* to be able to keep in memory the old AFC variables in case of spurious
       FB detection */
    static WORD32  old_psi_past[C_N_del+1];        /* F13.19 */
    static WORD16  old_psi_quant;                  /* F13.3  */

#endif
#if (L1_FF_MULTIBAND == 1)
    UWORD8 physical_band_id;
#endif

#if 0	/* LoCosto added var init */
//Set AFC close loop gain for ALGO_AFC_LQG_PREDICTOR.
if(l1_mode==I_MODE)//MS is in Idle mode
L = 41732; //F0.20 L=41732/2^20 = 0.04
else //All other modes than Idle
L = 10433; //F0.20 L=10433/2^20 = 0.01
#endif

#if (L1_FF_MULTIBAND == 0)

    if (((l1_config.std.id == DUAL) || (l1_config.std.id == DUALEXT) || (l1_config.std.id == DUAL_US)) &&
#if (VCXO_ALGO == 1)
        ((phase != AFC_INIT_CENTER) || (phase != AFC_INIT_MIN) || (phase != AFC_INIT_MAX)))
#else
         (phase != AFC_INIT))
#endif
 {
    if (radio_freq >= l1_config.std.first_radio_freq_band2)
    {
      angle = (angle + 1) >> 1;
    }
 }

    else if (((l1_config.std.id == DCS1800) || (l1_config.std.id == PCS1900)) &&
#if (VCXO_ALGO == 1)
        ((phase != AFC_INIT_CENTER) || (phase != AFC_INIT_MIN) || (phase != AFC_INIT_MAX)))
#else
         (phase != AFC_INIT))
#endif
 {
   angle = (angle + 1) >> 1;
 }

#else // L1_FF_MULTIBAND = 1 below

#if (VCXO_ALGO == 1)
     if((phase != AFC_INIT_CENTER) || (phase != AFC_INIT_MIN) || (phase != AFC_INIT_MAX))
#else
     if(phase != AFC_INIT)
#endif
    {
      physical_band_id = l1_multiband_radio_freq_convert_into_physical_band_id(radio_freq);

      if( (multiband_rf[physical_band_id].gsm_band_identifier == DCS1800) || (multiband_rf[physical_band_id].gsm_band_identifier == PCS1900))
      {
         angle = (angle + 1) >> 1;
      }
    }

#endif // #if (L1_FF_MULTIBAND == 1) else 



 /*********************************/
 /* frequency offset compensation */
 /*********************************/
 /* Initialization */

#if (VCXO_ALGO == 1)
 switch (l1_config.params.afc_algo)
 {

   /* algo1 only: */
   case ALGO_AFC_KALMAN:
   {
#endif
#if (VCXO_ALGO == 0)
     if (phase==AFC_INIT)
     {
         // WARNING
       // In this case, "angle" variable contains EEPROM_AFC initialization value
       // directly loaded from EEPROM, and "snr" variable is not meaningful.
       /* Static variables initialisation */
       P=C_cov_start;
       Psi=0;
       if (angle>C_max_step)
          Psi_quant[C_N_del]=C_max_step;
       else
          if(angle<C_min_step)
            Psi_quant[C_N_del]=C_min_step;
          else Psi_quant[C_N_del]=angle;

       Psi=l1_config.params.psi_st*Psi_quant[C_N_del];         /* F0.16 * F13.3 = F13.19 */
     } /* end AFC_INIT*/
     else
     {
       if (phase==AFC_OPEN_LOOP)
       {
         /* delay line for Psi_quant values */
         for (i=1;i<=C_N_del;i++)
           Psi_quant[i-1]=Psi_quant[i];

         var_32=(WORD32)((WORD32)angle*l1_config.params.psi_sta_inv)<<4;
                                 /*(F16.0 * F1.15 = F17.15) << 4 = F13.19 */

#if(RF_FAM == 61)
         /* In order to implement the NINT function for a F16.0, we check */
         /* if var_32 + 0.5*2**18 is a multiple of 2**18                  */
         quotient=(WORD16)((WORD32)(((WORD32)(var_32+(1<<17)))/(1<<18)));
         var_16=quotient*4;
#else
         /* In order to implement the NINT function for a F16.0, we check */
         /* if var_32 + 0.5*2**19 is a multiple of 2**19                  */
         quotient=(WORD16)((WORD32)(((WORD32)(var_32+(1<<18)))/(1<<19)));
         var_16=quotient*8;
#endif
           if (var_16>C_max_step)
		    Psi_quant[C_N_del]=Add_Sat_sign_16b(Psi_quant[C_N_del],C_max_step);
		  else
		    if(var_16<C_min_step)
		      Psi_quant[C_N_del]=Add_Sat_sign_16b(Psi_quant[C_N_del],C_min_step);
		    else Psi_quant[C_N_del]=Add_Sat_sign_16b(Psi_quant[C_N_del],var_16);     /* F13.3                  */

		     Psi=l1_config.params.psi_st*Psi_quant[C_N_del];       /* F0.16 * F13.3 = F13.19 */
	   }/*end if AFC_OPEN_LOOP*/

       else
       {
         /* delay line for Psi_quant values */
         for (i=1;i<=C_N_del;i++)
           Psi_quant[i-1]=Psi_quant[i];

         /********************/
         /* Filter algorithm */
         /********************/

         /* Covariance error is increased of C_Q */
         P=P+(*frame_count)*C_Q;

         /* Clipping of P */
         if (P>C_thr_P) P=C_thr_P;

         if (snr>=C_thr_snr)
         {
           /* Clipping of error angle */
           if (angle>C_thr_phi)
            angle=C_thr_phi;
           if (angle<-C_thr_phi)
            angle=-C_thr_phi;

           /* Kalman gain                             */
           /*K=P*(1/(P+C_a0_kalman+(C_g_kalman*RNS))) */
           /*C_a0_kalman=0.01                         */
           /*C_g_kalman =0.05                         */
           num=(C_g_kalman/snr)+P+C_a0_kalman;
                         /* (F2.30 / F6.10) = F 12.20 */

           /* denom = P << 19 = F 1.39                                */
           /* extension of denom=P to a 40 bits variable              */
           /* denom (F12.20) << 16 = F 4.36                           */
           guard1=(WORD16)((WORD32)P>>16);
           /* denom = P<<16 = (F4.36) << 3 = F 1.39                   */
           denomH=(UWORD16)P;
           /* Low part of denom is equal to 0, because P has been 16  */
           /* bits left shifted previously.                           */
           denomH_3msb=(denomH>>13)&0x0007;
           guard1=(guard1<<3)|denomH_3msb;
           denomH<<=3;
           denom=(UWORD32)denomH<<16;
           /* num + guard1 are a 40 bits representation of P          */
           /* In order to compute P(F1.39)/num, we sample P in guard1 */
           /* (scaled to a 32 bits number) and num (32 bits number)   */
           /* K = ((guard1<<24)/num)<<8 + (denom/num)                 */
           var1=(WORD32)guard1<<24;
           var1=var1/num;
           var1=(WORD32)var1<<8;
           /* var2 is an unsigned variable, var1 contains signed guard*/
           /* bits.                                                   */
           var2=denom/num;
           K = (var1+var2)<<1;             /* F1.39 / F12.20 = F13.19 */
                                           /* F13.19 << 1 = F12.20    */

           /* Clipping of the Kalman gain  */
           if (K>=C_thr_K)
             K=C_thr_K;

           /*******************************************************/
           /* P=(1-K)*P = 0.8 * 0.5 at maximum                    */
           /*******************************************************/
           /* Perform a positive variable F12.20 multiplication by*/
           /* positive variable F12.20                            */
           var_16=(WORD16)(1048576L-K);   /* acclow(1-K) = F12.20  */
           guard1=0;                     /* positive variable     */
           var1=UMult_40b(P,var_16,&guard1);
           var_16=(WORD16)((1048576L-K)>>16);
                                         /* acchigh(1-K) = F12.20 */
           var2=P*var_16;                /* var2 = 0x80000 * 0xc  */
                                         /* at maximum, so result */
                                         /* is 32 bits WORD32 and   */
                                         /* equal 0x600000        */
           /* extension of var2 to a 40 bits variable : var2<<16  */
           guard2=(WORD16)((WORD32)var2>>16);
           guard1guard2=((WORD32)guard1<<16) |((WORD32) guard2&0x0000FFFFL);
           var2=var2<<16;
           var_32=Add_40b(guard1guard2,var1,var2,&guardout);
           /* var_32 (F8.40) >> 16 = F8.24  */
           LGuard=(WORD32)guardout<<16;
           var1=(UWORD32)var_32>>16;
           /* var_32 >> 4 = F12.20          */
           P=(var1+LGuard)>>4;

           Phi_32=Mult_40b(l1_config.params.psi_st_32,Psi_quant[0],&guardout);
                                       /* F0.32 * F13.3 = F5.35    */
           LGuard=(WORD32)guardout<<16;  /* var_32 (F5.35) >> 16     */
                                       /* F13.19                   */
           var1=(UWORD32)Phi_32>>16;
           Phi_32=Psi-(LGuard+var1);   /* F13.19                   */

           /*Phi=angle-Phi_32*/
           Phi_32=((WORD32)angle<<4)-Phi_32;
                                        /* F1.15 * 4 = F13.19       */
           Phi=(WORD16)(Phi_32>>4);      /* F17.15                   */
           /*var1=K*Phi                    F12.20 * F1.15 = 13.35   */
           guard1=0;
           var1=Mult_40b(K,Phi,&guard1);
                                        /* var1 (F13.35) >> 16      */
                                        /* F13.19                   */
           LGuard=(WORD32)guard1<<16;
           var1=(UWORD32)var1>>16;
           Psi+=var1+LGuard;
           }/*if snr */

           var_32=Mult_40b(Psi,l1_config.params.psi_st_inv,&guardout);
                                        /* F13.19 * C      = F13.19 */

#if(RF_FAM == 61)
           /* In order to implement the NINT function for a F13.3, we check */
           /* if var_32 + 0.5*2**18 is a multiple of 2**18                  */
           quotient=(WORD16)((WORD32)(((WORD32)(var_32+(1<<17)))/(1<<18)));
           var_16=quotient*4;
#else
           /* In order to implement the NINT function for a F13.3, we check */
           /* if var_32 + 0.5*2**19 is a multiple of 2**19                  */
           quotient=(WORD16)((WORD32)(((WORD32)(var_32+(1<<18)))/(1<<19)));
           var_16=quotient*8;
#endif
           if (var_16>C_max_step)
             Psi_quant[C_N_del]=C_max_step;
           else
             if(var_16<C_min_step)
               Psi_quant[C_N_del]=C_min_step;
             else Psi_quant[C_N_del]=var_16; /* F13.3                 */

       }/*end AFC_CLOSE_LOOP*/
     } /* end else AFC_INIT*/

     *frame_count=0;
     return(Psi_quant[C_N_del]>>3); /* F16.0 */

#else

    } /* end case algo 1 */


    /* algo2 + init + estimator/predictor */
    case ALGO_AFC_LQG_PREDICTOR:
    {
        /******************************************************************/
        /* (New) VCXO Algorithm                                           */
        /******************************************************************/

        switch (phase) {
            case AFC_INIT_CENTER  :
            case AFC_INIT_MAX     :
            case AFC_INIT_MIN     :
            quant_avg = 0;
            M_Count = 0;
#if 0	/* present in LoCosto but not in TCS211 */
            for (i = 0; i <= C_PSI_AVG_SIZE_D ; i++)  //omaps00090550
                psi_avg[i] = 0;
#endif
            first_avg = 1;
            good_snr = 0;

            // DAC search algorithm is as follows - up to 12 attempts are made
            // DAC search algorithm uses three values : DAC_center -> DAC_max -> DAC_min ->
            // The first four attempts are made on DAC_center
            // The next four attempts are made on DAC_max
            // The last four attempts are made on DAC_min
            // There are statistical reasons for trying four times

            switch (phase)
            {
                case AFC_INIT_CENTER:
                    psi_quant = l1_config.params.afc_dac_center;
                    break;
                case AFC_INIT_MAX:
                    psi_quant = l1_config.params.afc_dac_max;
                    break;
                case AFC_INIT_MIN:
                    psi_quant = l1_config.params.afc_dac_min;
                    break;
                default :
                  break;
            }

            /* F0.32 * F13.3 = F5.35    */
            psi_past[C_N_del]=Mult_40b(l1_config.params.psi_st_32,psi_quant, &guardout);
            /*  (F13.3<<16 )+(F5.35>>16) = F13.19 */
            psi_past[C_N_del]=((WORD32)guardout<<16)+((UWORD32)psi_past[C_N_del]>>16);

            break;

            case AFC_OPEN_LOOP  :
            {
              /* VCXO changes for spurious FB detection */
              if (l1s.spurious_fb_detected == TRUE)
              {
                psi_quant = old_psi_quant;

                for(i=0;i<C_N_del+1;i++)
                  psi_past[i] = old_psi_past[i];

                /* reset the spurious_fb_detected_flag */
                l1s.spurious_fb_detected = FALSE;
              } /* end of spuriousFB detected */

            /* save in memory the old AFC related values */
            old_psi_quant = psi_quant;

            for(i=0;i<C_N_del+1;i++)
              old_psi_past[i] = psi_past[i];

            /* delay line for psi_quant values */
            for (i = 1; i <= C_N_del; i++)
                psi_past[i-1] = psi_past[i];

                /* (F16.0 * F1.15 = F17.15) << 4 = F13.19 */
                var_32 = (WORD32) ((WORD32)angle * l1_config.params.psi_sta_inv) << 4;

#if(RF_FAM == 61)
                /* In order to implement the NINT function for a F16.0,*/
                /*we check if var_32 + 0.5*2**18 is a multiple of 2**18 */
                var_16 = (WORD16)
                    ((WORD32) (((WORD32)(var_32 + (1<<17))) / (1<<18)));
                var_16 = var_16 * 4;
#else
                /* In order to implement the NINT function for a F16.0,*/
                /*we check if var_32 + 0.5*2**19 is a multiple of 2**19 */
                var_16 = (WORD16)
                    ((WORD32) (((WORD32)(var_32 + (1<<18))) / (1<<19)));
                var_16 = var_16 * 8;
#endif

              #if 0	/* LoCosto code with saturation */
                if (var_16 > C_max_step)
                    psi_quant = Add_Sat_sign_16b(psi_quant,C_max_step);
                else if (var_16 < C_min_step)
                    psi_quant = Add_Sat_sign_16b(psi_quant,C_min_step);
                else psi_quant = Add_Sat_sign_16b(psi_quant,var_16); /* F13.3 */
              #else	/* matching TCS211 */
                if (var_16 > C_max_step)
                    psi_quant += C_max_step;
                else if (var_16 < C_min_step)
                    psi_quant += C_min_step;
                else psi_quant += var_16; /* F13.3 */
              #endif

                /* F0.32 * F13.3 = F5.35    */
                psi_past[C_N_del]=Mult_40b(l1_config.params.psi_st_32,psi_quant, &guardout);
                /*  (F13.3<<16 )+(F5.35>>16) = F13.19 */
                psi_past[C_N_del]=((WORD32)guardout<<16)+((UWORD32)psi_past[C_N_del]>>16);

            }
            break;

            case AFC_CLOSED_LOOP :

                /* delay line for psi_quant values */
                for (i = 1; i <= C_N_del; i++)
                    psi_past[i-1] = psi_past[i];

                /************************************/
                /*         Estimation               */
                /************************************/
                if ( (l1_config.params.rgap_algo != 0) &&
                     ((l1_mode==CON_EST_MODE2)||(l1_mode==DEDIC_MODE)
                  #if L1_GPRS
                      || l1_mode==PACKET_TRANSFER_MODE
                  #endif
                  ))
                {

                    M_Count += *frame_count;
                    if (snr >= l1_config.params.afc_snr_thr) {
                        // Accumulate average over N TDMA frames
                        psi_avg[0] += psi_past[C_N_del];
                        // Count number of good snr's within window_avg_size chunks
                        good_snr++;
            }
                    // M_Count >= M ?
                    if (M_Count >= l1_config.params.afc_win_avg_size_M) {
                        // M_Count counts how far we have reached in the window_avg_size blocks

                        // Scale estimate relative to good snr - Don't divide by zero in case of bad measurements
                        if (good_snr > 0)
                            psi_avg[0] /= good_snr;

                        // We now have an estimation over window_avg_size TDMA frames in psi_avg[0]
                        if (first_avg == 1) {
                            first_avg = 0;
                            // Use first estimation as best guess for the other avg values
                            // This is used both at initialisation and when returning from reception gap
                            for (i = 1; i <= C_PSI_AVG_SIZE_D ; i++)
                                psi_avg[i] = psi_avg[0];
                }

                        // Estimation 1st order
                        // Use biggest window to reduce noise effects signal in psi values
                        // NOTE: Due to performance issues division by MSIZE is in predictor
                        if (l1_config.params.rgap_algo >= 1) {
                            quant_avg = (WORD16) (psi_avg[0] - psi_avg[C_PSI_AVG_SIZE_D]);
                        }

                        for (i = C_PSI_AVG_SIZE_D - 1; i >= 0 ; i--)
                            psi_avg[i+1] = psi_avg[i];
                        psi_avg[0] = 0;
                        M_Count = 0;
                        good_snr = 0;
                    }

                } else {
                        // No estmation when in Idle mode (DEEP or BIG SLEEP) => Reset!
                        first_avg  = 1;
                        M_Count  = 0;
                        good_snr   = 0;
                        psi_avg[0] = 0;
                }

                if (snr >= l1_config.params.afc_snr_thr) {
                    /********************/
                    /* Filter algorithm */
                    /********************/

                    /* No prediction during normal operation */
                    B_Count= 0;

                    /* Clip error angle */
                    if (angle > C_thr_phi)
                        angle = C_thr_phi;
                    if (angle < -C_thr_phi)
                        angle = -C_thr_phi;

                    Phi_32 = psi_past[C_N_del] - psi_past[0];   /* F13.19 */
                    /* Phi = angle - Phi_32*/
                    Phi_32 = ((WORD32) angle << 4) - Phi_32;
                    /* F1.15 * 4 = F13.19 */
                    #if 0	/* LoCosto code */
                      Phi = (WORD16)((WORD32)((WORD32)(Phi_32 + (1<<3)))/ (1<<4)); /* F17.15 */
                    #else	/* TCS211 reconstruction */
                      Phi = Phi_32 >> 4;
                    #endif
                    /* (F0.20 * F1.15) >> 16 = F13.19 */
                    #if 0	/* LoCosto code with saturation and L */
                      var_32 = (L * Phi + (1<<15)) >> 16;
                      psi_past[C_N_del] = Add_Sat_sign_32b(psi_past[C_N_del],var_32);
                    #else	/* matching TCS211 */
                      psi_past[C_N_del] += (10433 * Phi) >> 16;
                    #endif

                }
                else
                {
                    /************************************/
                    /*         Prediction               */
                    /************************************/

                    // Only predict in dedicated mode
                    // NO prediction in idle mode
                    //   l1a_l1s_com.dedic_set.SignalCode = NULL
                    if ( (l1_config.params.rgap_algo != 0) &&
                        ((l1_mode==CON_EST_MODE2)||(l1_mode==DEDIC_MODE)
                        #if L1_GPRS
                        || l1_mode==PACKET_TRANSFER_MODE
                        #endif
                        ))
                    {
                      /* Prediction of psi during reception gaps */
                      B_Count
                          += *frame_count;

                        /* Predict psi ONLY when we have sufficient measurements available                   */
                        /* If we don't have enough measurements we don't do anything (= 0th order estimation)*/

                        // Was the consecutive bad SNRs threshold value exceeded?
                        if (B_Count>= l1_config.params.rgap_bad_snr_count_B) {

                            // Predict with 0th order estimation is the default

                            // Predict with 1st order estimation
                            if (l1_config.params.rgap_algo >= 1)
                            {
                            #if 0	/* LoCosto code with saturation */
                                 psi_past[C_N_del] = Add_Sat_sign_32b(psi_past[C_N_del],
                                     ((quant_avg * (l1_config.params.rgap_bad_snr_count_B))/(C_MSIZE))
                                     );
                            #else	/* matching TCS211 */
                                 psi_past[C_N_del] +=
                                     ((quant_avg * (l1_config.params.rgap_bad_snr_count_B))/(C_MSIZE));
                            #endif
                            }

                            B_Count= B_Count - l1_config.params.rgap_bad_snr_count_B;

                            // Indicate by raising first_avg flag that a reception gap has occurred
                            // I.e. the psi_avg table must be reinitialised after leaving reception gap
                            first_avg = 1;

                            // Counters in estimation part must also be reset
                            M_Count  = 0;
                            good_snr   = 0;
                            psi_avg[0] = 0;
                      }
                    }
                }

                /* Quantize psi value */

                /* F0.19 * 16.0 = F16.19 */
                #if 0	/* LoCosto code */
                  var_32 = Sat_Mult_20sign_16unsign(psi_past[C_N_del],l1_config.params.psi_st_inv);
                #else	/* TCS211 reconstruction */
		  var_32 = psi_past[C_N_del] * l1_config.params.psi_st_inv;
                #endif

#if(RF_FAM == 61)
                /* In order to implement the NINT function for a F13.3,*/
                /*we check if var_32 + 0.5*2**18 is a multiple of 2**18 */
                var_16 = (WORD16)
                    ((WORD32)((WORD32)(var_32 + (1<<17))) / (1<<18));
                var_16 = var_16 * 4;
#else
                /* In order to implement the NINT function for a F13.3,*/
                /*we check if var_32 + 0.5*2**19 is a multiple of 2**19 */
                var_16 = (WORD16)
                    ((WORD32)((WORD32)(var_32 + (1<<18))) / (1<<19));
                var_16 = var_16 * 8;
#endif
                if (var_16 > C_max_step)
                    psi_quant = C_max_step;
                else if (var_16 < C_min_step)
                    psi_quant = C_min_step;
                else
                    psi_quant = var_16; /* F13.3 */
         break;
         } // switch phase

        *frame_count = 0;

        return (psi_quant >> 3); /* F16.0 */
    } /* end case algo 2 */

    /* algo1 + init + estimator/predictor */
    case ALGO_AFC_KALMAN_PREDICTOR:
    {
      if ((phase==AFC_INIT_CENTER) || (phase==AFC_INIT_MAX) || (phase==AFC_INIT_MIN))
      {
        // WARNING
        // In this case, "angle" variable contains EEPROM_AFC initialization value
        // directly loaded from EEPROM, and "snr" variable is not meaningful.
        /* Static variables initialisation */

        quant_avg = 0;
        M_Count = 0;
#if 0	/* present in LoCosto but not in TCS211 */
        for (i = 0; i <=C_PSI_AVG_SIZE_D ; i++)   //omaps00090550
            psi_avg[i] = 0;
#endif
        first_avg = 1;
        good_snr = 0;

        // DAC search algorithm is as follows - up to 12 attempts are made
        // DAC search algorithm uses three values : DAC_center -> DAC_max -> DAC_min ->
        // The first four attempts are made on DAC_center
        // The next four attempts are made on DAC_max
        // The last four attempts are made on DAC_min
        // There are statistical reasons for trying four times

        switch (phase) {
        case AFC_INIT_CENTER:
            Psi_quant[C_N_del] = l1_config.params.afc_dac_center;
            break;
        case AFC_INIT_MAX:
            Psi_quant[C_N_del] = l1_config.params.afc_dac_max;
            break;
        case AFC_INIT_MIN:
            Psi_quant[C_N_del] = l1_config.params.afc_dac_min;
            break;
        default :
          break;
        }

        P=C_cov_start;
        Psi=0;
        if (angle>C_max_step)
           Psi_quant[C_N_del]=C_max_step;
        else
           if(angle<C_min_step)
             Psi_quant[C_N_del]=C_min_step;
           else Psi_quant[C_N_del]=angle;

               /* F0.32 * F13.3 = F5.35    */
        Psi=Mult_40b(l1_config.params.psi_st_32,Psi_quant[C_N_del], &guardout);
        /*  (F13.3<<16 )+(F5.35>>16) = F13.19 */
        Psi=((WORD32)guardout<<16)+((UWORD32)Psi>>16);

      } /* end AFC_INIT*/
      else
      {
        if (phase==AFC_OPEN_LOOP)
        {
          /* relaod last good values in the ALGO */
          if (l1s.spurious_fb_detected == TRUE)
          {
            for(i=0;i<C_N_del+1;i++)
              Psi_quant[i] = old_Psi_quant[i];

            Psi = old_Psi;
            l1s.spurious_fb_detected = FALSE;
          }

          /* Save the old values in memory */
          for(i=0;i<C_N_del+1;i++)
            old_Psi_quant[i] = Psi_quant[i];
          old_Psi = Psi;

          /* delay line for Psi_quant values */
          for (i=1;i<=C_N_del;i++)
            Psi_quant[i-1]=Psi_quant[i];

          var_32=(WORD32)((WORD32)angle*l1_config.params.psi_sta_inv)<<4;
                                 /*(F16.0 * F1.15 = F17.15) << 4 = F13.19 */

#if(RF_FAM == 61)
          /* In order to implement the NINT function for a F16.0, we check */
          /* if var_32 + 0.5*2**18 is a multiple of 2**18                  */
          quotient=(WORD16)((WORD32)(((WORD32)(var_32+(1<<17)))/(1<<18)));
          var_16=quotient*4;
#else
          /* In order to implement the NINT function for a F16.0, we check */
          /* if var_32 + 0.5*2**19 is a multiple of 2**19                  */
          quotient=(WORD16)((WORD32)(((WORD32)(var_32+(1<<18)))/(1<<19)));
          var_16=quotient*8;
#endif

#if 0	/* LoCosto code with saturation */
          if (var_16>C_max_step)
            Psi_quant[C_N_del]=Add_Sat_sign_16b(Psi_quant[C_N_del],C_max_step);
          else if (var_16<C_min_step)
            Psi_quant[C_N_del]=Add_Sat_sign_16b(Psi_quant[C_N_del],C_min_step);
          else Psi_quant[C_N_del]=Add_Sat_sign_16b(Psi_quant[C_N_del],var_16);     /* F13.3                  */
#else	/* matching TCS211 */
          if (var_16>C_max_step)
            Psi_quant[C_N_del] += C_max_step;
          else if (var_16<C_min_step)
            Psi_quant[C_N_del] += C_min_step;
          else Psi_quant[C_N_del] += var_16;     /* F13.3                  */
#endif

                 /* F0.32 * F13.3 = F5.35    */
         Psi=Mult_40b(l1_config.params.psi_st_32,Psi_quant[C_N_del], &guardout);
         /*  (F13.3<<16 )+(F5.35>>16) = F13.19 */
         Psi=((WORD32)guardout<<16)+((UWORD32)Psi>>16);

       }/*end if AFC_OPEN_LOOP*/
       else
       {

         /* delay line for Psi_quant values */
         for (i=1;i<=C_N_del;i++)
           Psi_quant[i-1]=Psi_quant[i];

                /************************************/
                /*         Estimation               */
                /************************************/
                if ( (l1_config.params.rgap_algo != 0) &&
                    ((l1_mode==CON_EST_MODE2)||(l1_mode==DEDIC_MODE)
                    #if L1_GPRS
                    || l1_mode==PACKET_TRANSFER_MODE
                    #endif
                    ))
                {

                    M_Count += *frame_count;
                    if (snr >= l1_config.params.afc_snr_thr) {
                        // Accumulate average over N TDMA frames
                        psi_avg[0] += psi_past[C_N_del];
                        // Count number of good snr's within window_avg_size chunks
                        good_snr++;
                    }

                    // M_Count >= M ?
                    if (M_Count >= l1_config.params.afc_win_avg_size_M) {
                        // M_Count counts how far we have reached in the window_avg_size blocks

                        // Scale estimate relative to good snr - Don't divide by zero in case of bad measurements
                        if (good_snr > 0)
                            psi_avg[0] /= good_snr;

                        // We now have an estimation over window_avg_size TDMA frames in psi_avg[0]
                        if (first_avg == 1) {
                            first_avg = 0;
                            // Use first estimation as best guess for the other avg values
                            // This is used both at initialisation and when returning from reception gap
                            for (i = 1; i <= C_PSI_AVG_SIZE_D ; i++)
                                psi_avg[i] = psi_avg[0];
                        }

                        // Estimation 1st order
                        // Use biggest window to reduce noise effects signal in psi values
                        // NOTE: Due to performance issues division by MSIZE is in predictor
                        if (l1_config.params.rgap_algo >= 1) {
                            quant_avg = (WORD16) (psi_avg[0] - psi_avg[C_PSI_AVG_SIZE_D]);
            }

                        for (i = C_PSI_AVG_SIZE_D - 1; i >= 0 ; i--)
                            psi_avg[i+1] = psi_avg[i];
                        psi_avg[0] = 0;
                        M_Count = 0;
                        good_snr = 0;
        }

                } else {
                        // No estmation when in Idle mode (DEEP or BIG SLEEP) => Reset!
                        first_avg  = 1;
                        M_Count  = 0;
                        good_snr   = 0;
                        psi_avg[0] = 0;
                }

         /********************/
         /* Filter algorithm */
         /********************/

         /* Covariance error is increased of C_Q */
         P=P+(*frame_count)*C_Q;

         /* Clipping of P */
         if (P>C_thr_P) P=C_thr_P;

         if (snr>=C_thr_snr)
         {
           /* Clipping of error angle */
           if (angle>C_thr_phi)
            angle=C_thr_phi;
           if (angle<-C_thr_phi)
            angle=-C_thr_phi;

           /* Kalman gain                             */
           /*K=P*(1/(P+C_a0_kalman+(C_g_kalman*RNS))) */
           /*C_a0_kalman=0.01                         */
           /*C_g_kalman =0.05                         */
           num=(C_g_kalman/snr)+P+C_a0_kalman;
                         /* (F2.30 / F6.10) = F 12.20 */

           /* denom = P << 19 = F 1.39                                */
           /* extension of denom=P to a 40 bits variable              */
           /* denom (F12.20) << 16 = F 4.36                           */
           guard1=(WORD16)((WORD32)P>>16);
           /* denom = P<<16 = (F4.36) << 3 = F 1.39                   */
           denomH=(UWORD16)P;
           /* Low part of denom is equal to 0, because P has been 16  */
           /* bits left shifted previously.                           */
           denomH_3msb=(denomH>>13)&0x0007;
           guard1=(guard1<<3)|denomH_3msb;
           denomH<<=3;
           denom=denomH<<16;   //(UWORD32) removed typecast omaps00090550
           /* num + guard1 are a 40 bits representation of P          */
           /* In order to compute P(F1.39)/num, we sample P in guard1 */
           /* (scaled to a 32 bits number) and num (32 bits number)   */
           /* K = ((guard1<<24)/num)<<8 + (denom/num)                 */
           var1=(WORD32)guard1<<24;
           var1=var1/num;
           var1=(WORD32)var1<<8;
           /* var2 is an unsigned variable, var1 contains signed guard*/
           /* bits.                                                   */
           #if 0	/* fixed LoCosto code */
             var2=  ((WORD32)(denom)/(num));  //omaps00090550
           #else	/* matching TCS211 */
             var2=  denom / num;
           #endif
           K = (var1+var2)<<1;             /* F1.39 / F12.20 = F13.19 */
                                           /* F13.19 << 1 = F12.20    */

           /* Clipping of the Kalman gain  */
           if (K>=C_thr_K)
             K=C_thr_K;

           /*******************************************************/
           /* P=(1-K)*P = 0.8 * 0.5 at maximum                    */
           /*******************************************************/
           /* Perform a positive variable F12.20 multiplication by*/
           /* positive variable F12.20                            */
           var_16=(WORD16)(1048576L-K);   /* acclow(1-K) = F12.20  */
           guard1=0;                     /* positive variable     */
           var1=UMult_40b(P,var_16,&guard1);
           var_16=(WORD16)((1048576L-K)>>16);
                                         /* acchigh(1-K) = F12.20 */
           var2=P*var_16;                /* var2 = 0x80000 * 0xc  */
                                         /* at maximum, so result */
                                         /* is 32 bits WORD32 and   */
                                         /* equal 0x600000        */
           /* extension of var2 to a 40 bits variable : var2<<16  */
           guard2=(WORD16)((WORD32)var2>>16);
           guard1guard2=((WORD32)guard1<<16) |((WORD32) guard2&0x0000FFFFL);
           var2=var2<<16;
           var_32=Add_40b(guard1guard2,var1,var2,&guardout);
           /* var_32 (F8.40) >> 16 = F8.24  */
           LGuard=(WORD32)guardout<<16;
           var1=(UWORD32)var_32>>16;
           /* var_32 >> 4 = F12.20          */
           P=(var1+LGuard)>>4;

           Phi_32=Mult_40b(l1_config.params.psi_st_32,Psi_quant[0],&guardout);
                                       /* F0.32 * F13.3 = F5.35    */
           LGuard=(WORD32)guardout<<16;  /* var_32 (F5.35) >> 16     */
                                       /* F13.19                   */
           var1=(UWORD32)Phi_32>>16;
           Phi_32=Psi-(LGuard+var1);   /* F13.19                   */

           /*Phi=angle-Phi_32*/
           Phi_32=((WORD32)angle<<4)-Phi_32;
                                        /* F1.15 * 4 = F13.19       */
           Phi=(WORD16)(Phi_32>>4);      /* F17.15                   */
           /*var1=K*Phi                    F12.20 * F1.15 = 13.35   */
           guard1=0;
           var1=Mult_40b(K,Phi,&guard1);
                                        /* var1 (F13.35) >> 16      */
                                        /* F13.19                   */
           LGuard=(WORD32)guard1<<16;
           var1=(UWORD32)var1>>16;
           Psi+=var1+LGuard;
           } else {
                /************************************/
                /*         Prediction               */
                /************************************/

                // Only predict in dedicated mode
                // NO prediction in idle mode
                //   l1a_l1s_com.dedic_set.SignalCode = NULL
                if ( (l1_config.params.rgap_algo != 0) &&
                    ((l1_mode==CON_EST_MODE2)||(l1_mode==DEDIC_MODE)
                    #if L1_GPRS
                    || l1_mode==PACKET_TRANSFER_MODE
                    #endif
                    ))
                {

                  /* Prediction of psi during reception gaps */
                  B_Count+= *frame_count;

                    /* Predict psi ONLY when we have sufficient measurements available                   */
                    /* If we don't have enough measurements we don't do anything (= 0th order estimation)*/

                    // Was the consecutive bad SNRs threshold value exceeded?
                    if (B_Count>= l1_config.params.rgap_bad_snr_count_B) {

                        // Predict with 0th order estimation is the default

                        // Predict with 1st order estimation
                        if (l1_config.params.rgap_algo >= 1)
                            Psi += ((quant_avg * (l1_config.params.rgap_bad_snr_count_B))/(C_MSIZE));

                        B_Count= B_Count - l1_config.params.rgap_bad_snr_count_B;

                        // Indicate by raising first_avg flag that a reception gap has occurred
                        // I.e. the psi_avg table must be reinitialised after leaving reception gap
                        first_avg = 1;

                        // Counters in estimation part must also be reset
                        M_Count  = 0;
                        good_snr   = 0;
                        psi_avg[0] = 0;
                    }
                }
           }

           /* Quantize psi value */

           var_32=Mult_40b(Psi,l1_config.params.psi_st_inv,&guardout);
                                        /* F13.19 * C      = F13.19 */

#if(RF_FAM == 61)
           /* In order to implement the NINT function for a F13.3, we check */
           /* if var_32 + 0.5*2**18 is a multiple of 2**18                  */
           quotient=(WORD16)((WORD32)(((WORD32)(var_32+(1<<17)))/(1<<18)));
           var_16=quotient*4;
#else
           /* In order to implement the NINT function for a F13.3, we check */
           /* if var_32 + 0.5*2**19 is a multiple of 2**19                  */
           quotient=(WORD16)((WORD32)(((WORD32)(var_32+(1<<18)))/(1<<19)));
           var_16=quotient*8;
#endif
           if (var_16>C_max_step)
             Psi_quant[C_N_del]=C_max_step;
           else
             if(var_16<C_min_step)
               Psi_quant[C_N_del]=C_min_step;
             else Psi_quant[C_N_del]=var_16; /* F13.3                 */


       }/*end AFC_CLOSE_LOOP*/
    } /* end else AFC_INIT*/

       *frame_count = 0;
       return(Psi_quant[C_N_del]>>3); /* F16.0 */
    } /* end case algo 3 */
#endif

#if (VCXO_ALGO == 1)
    default:
      return 0;
//omaps00090550      break;
  } // end of Switch
  #endif

} /* end l1ctl_afc */


/************************************/
/* Automatic timing control (TOA)   */
/************************************/

#if (TOA_ALGO == 2)

#define TOA_DEBUG_ENABLE 0


#if (TOA_DEBUG_ENABLE == 1)

  #define TOA_MAKE_ZERO    0

  #define  TOA_LOG_BUFFER_LENGTH  4096

  typedef struct
  {
    UWORD16 SNR_val;
    UWORD16 TOA_val;
    UWORD16 l1_mode;
    UWORD16 toa_frames_counter;
    UWORD16 fn_mod42432;
  }T_TOA_log_debug;


  T_TOA_log_debug  toa_log_debug[TOA_LOG_BUFFER_LENGTH];
  UWORD32          toa_log_index;

  UWORD32          toa_make_zero_f;

#endif

/*-------------------------------------------------------*/
/* l1ctl_toa()                                           */
/*-------------------------------------------------------*/
/* Parameters :                                          */
/* Return     :                                          */
/* Functionality :                                       */
/*-------------------------------------------------------*/

WORD16 l1ctl_toa (UWORD8 phase, UWORD32 l1_mode, UWORD16 SNR_val, UWORD16 TOA_val)
{
  WORD16     TOA_period_len = TOA_PERIOD_LEN [l1_mode];
  WORD16     TOA_SHIFT=ISH_INVALID;
  UWORD16    cumul_abs;
  WORD16     cumul_sign;
  WORD32     prod_tmp, div_tmp,prod_sign;
  WORD32     toa_update_flag=0;
  WORD16     cumul;
  UWORD16    cumul_counter;
#if (NEW_TOA_ALGO == 1)
UWORD16	 Trans_active;
  static WORD16            cumul_noTrans =0;
  static UWORD16           period_counter_noTrans =0;

  if ((l1_mode==CON_EST_MODE2)||(l1_mode==DEDIC_MODE)
#if L1_GPRS
    || l1_mode==PACKET_TRANSFER_MODE
#endif
    )
    Trans_active=TRUE;
  else Trans_active=FALSE;
#endif
  if (phase==TOA_INIT)
  {
#if (NEW_TOA_ALGO == 1)
 cumul_noTrans =0;
 period_counter_noTrans =0;
#endif

     l1s.toa_var.toa_frames_counter=0;
     l1s.toa_var.toa_accumul_counter=0;
     l1s.toa_var.toa_accumul_value=0;
     #if (TOA_DEBUG_ENABLE == 1)
       toa_log_index = 0;
       #if (TOA_MAKE_ZERO == 1)
          toa_make_zero_f = 1;
       #else
          toa_make_zero_f = 0;
       #endif
     #endif

     return (TOA_SHIFT);
  }

   cumul         = l1s.toa_var.toa_accumul_value;
   cumul_counter = l1s.toa_var.toa_accumul_counter;

   #if (TOA_DEBUG_ENABLE == 1)
     toa_log_debug[toa_log_index].SNR_val = SNR_val;
     toa_log_debug[toa_log_index].TOA_val = TOA_val;
     toa_log_debug[toa_log_index].l1_mode = l1_mode;
     toa_log_debug[toa_log_index].toa_frames_counter = l1s.toa_var.toa_frames_counter;
     toa_log_debug[toa_log_index].fn_mod42432 = l1s.actual_time.fn_mod42432;

     toa_log_index++;
     if(toa_log_index == TOA_LOG_BUFFER_LENGTH)
     {
       toa_log_index = 0;
     }
   #endif /* #if (TOA_DEBUG_ENABLE == 1) */

   #if (TRACE_TYPE == 5)
     trace_toa_sim_ctrl(SNR_val, TOA_val, l1_mode, l1s.toa_var.toa_frames_counter,
                        l1s.toa_var.toa_accumul_counter, l1s.toa_var.toa_accumul_value);
   #endif

   l1s.toa_var.toa_frames_counter++;

 {
    /* Fix for TOA */
      #define DSP_CALC_NO_TABS_HO  0x3CA4

    UWORD16 *toa_ho_fix;
    toa_ho_fix=(UWORD16 *)API_address_dsp2mcu(DSP_CALC_NO_TABS_HO);

    if ((TOA_val >= 22) || (TOA_val <= 6)) {
      *toa_ho_fix = 1;
     }

    if (*toa_ho_fix == 1) {
 	 if((TOA_val <= 18) && (TOA_val >= 10)) {
 	   *toa_ho_fix = 0;
	  }
	 } else {
	  *toa_ho_fix = 0;
	 }
 }


#if (NEW_TOA_ALGO == 1)
 if (Trans_active)
{
#endif
   if (SNR_val>= L1_TOA_SNR_THRESHOLD)
   {
     cumul_counter++;

     prod_tmp = L1_TOA_LAMBDA * cumul;
     prod_tmp = prod_tmp + ((0x00004000)); // basically for rounding
     div_tmp = ((prod_tmp >> 15) & (0x0000FFFF));
     cumul = div_tmp;

     //  implemented below is
     //  cumul = cumul + (L1_TOA_ONE_MINUS_LAMBDA * signum(TOA_Val - L1_TOA_EXPECTED_TOA))
     if(TOA_val > L1_TOA_EXPECTED_TOA) {
       cumul = cumul + L1_TOA_ONE_MINUS_LAMBDA;
     }
     else if (TOA_val < L1_TOA_EXPECTED_TOA) {
       cumul = cumul - L1_TOA_ONE_MINUS_LAMBDA;
     }
   } // End if SNR_val

   if(l1s.toa_var.toa_update_flag == TRUE)
   {
     toa_update_flag = 1;
   }

   if (toa_update_flag)
   {
     cumul_sign = (cumul>0)? 1: -1;
     cumul_abs = cumul_sign*cumul;
     if(cumul_counter <= 5)
     {
       TOA_SHIFT = (cumul_abs<=L1_TOA_THRESHOLD_15)? 0: cumul_sign;
     }
     else if(cumul_counter == 6)
     {
       TOA_SHIFT = (cumul_abs<=L1_TOA_THRESHOLD_20)? 0: cumul_sign;
     }
     else if(cumul_counter == 7)
     {
       TOA_SHIFT = (cumul_abs<=L1_TOA_THRESHOLD_25)? 0: cumul_sign;
     }
     else if(cumul_counter >= 8)
     {
       TOA_SHIFT = (cumul_abs<=L1_TOA_THRESHOLD_30)? 0: cumul_sign;
     }
     #if (TRACE_TYPE==1) || (TRACE_TYPE==4)
       trace_info.toa_trace_var.toa_accumul_value     = cumul;
       trace_info.toa_trace_var.toa_accumul_counter   = cumul_counter;
       trace_info.toa_trace_var.toa_frames_counter    = l1s.toa_var.toa_frames_counter;
     #endif

     cumul = 0;
     cumul_counter = 0;
     l1s.toa_var.toa_frames_counter = 0;
     l1s.toa_var.toa_update_flag    = FALSE;

     #if (TOA_DEBUG_ENABLE == 1)
       #if (TOA_MAKE_ZERO == 1)
         if (toa_make_zero_f == 1)
         {
           TOA_SHIFT=0;
         }
       #endif /*#if (TOA_DEBUG_ENABLE == 1)*/
     #endif /*#if (TOA_MAKE_ZERO == 1)*/

   } // end of if toa_update_flag
 #if (NEW_TOA_ALGO == 1)

}

else
{
	period_counter_noTrans++;

	if (SNR_val>= L1_TOA_SNR_THRESHOLD)
   {
     cumul_noTrans = cumul_noTrans + TOA_val - L1_TOA_EXPECTED_TOA;

   } // End if SNR_val

   if (l1s.toa_var.toa_update_flag == TRUE)
   {
      switch (period_counter_noTrans)
      {
        case 2:
          if (cumul_noTrans>=0)
            TOA_SHIFT = (cumul_noTrans+1) >>1 ;
          else
            TOA_SHIFT = (cumul_noTrans) >>1 ;
          break;
        case 3: /* Not fully accurate rounding*/
          if (cumul_noTrans>=0)
            TOA_SHIFT = (cumul_noTrans+2)/3 ;
          else
            TOA_SHIFT = (cumul_noTrans-2)/3 ;
          break;
       case 4:
          if (cumul_noTrans>=0)
            TOA_SHIFT = (cumul_noTrans+2) >>2 ;
          else
            TOA_SHIFT = (cumul_noTrans+1) >>2 ;
          break;
       default:
          TOA_SHIFT = cumul_noTrans;
          break;
    } /* end switch*/

		if (TOA_SHIFT>8)
				TOA_SHIFT =8;
		if (TOA_SHIFT<-8)
				TOA_SHIFT =-8;

	#if (TRACE_TYPE==1) || (TRACE_TYPE==4)
       trace_info.toa_trace_var.toa_accumul_value     = cumul_noTrans;
       trace_info.toa_trace_var.toa_accumul_counter   = period_counter_noTrans;
       trace_info.toa_trace_var.toa_frames_counter    = period_counter_noTrans;
  #endif

     cumul_noTrans = 0;
     period_counter_noTrans = 0;
     l1s.toa_var.toa_update_flag = FALSE;
     #if (TOA_DEBUG_ENABLE == 1)
       #if (TOA_MAKE_ZERO == 1)
         if (toa_make_zero_f == 1)
         {
           TOA_SHIFT=0;
         }
       #endif /*#if (TOA_DEBUG_ENABLE == 1)*/
     #endif /*#if (TOA_MAKE_ZERO == 1)*/

   } // end if update_flag
}
#endif
   // error a TOA is waiting to be updated in the TPU and will be erased
   #if (TRACE_TYPE==1) || (TRACE_TYPE==4)
   if (l1s.toa_var.toa_shift != ISH_INVALID)
   {
     l1_trace_toa_not_updated (); // should not occur!!
   }
  #endif

  if (TOA_SHIFT != ISH_INVALID) // new TOA => set the mask frames
  {
    // Set mask counter to 2 (2 frames masked).
    l1s.toa_var.toa_snr_mask = 2;
  }

  l1s.toa_var.toa_accumul_value     = cumul;
  l1s.toa_var.toa_accumul_counter   = cumul_counter;

  return(TOA_SHIFT);

} // l1ctl_toa


#else
/*-------------------------------------------------------*/
/* l1ctl_toa_update()                                    */
/*-------------------------------------------------------*/
/* Parameters :                                          */
/* Return     :                                          */
/* Functionality :                                       */
/*-------------------------------------------------------*/
WORD16 l1ctl_toa_update(UWORD32 *TOASP, UWORD32 l1_mode)
{
  static UWORD16  Old_TOA_estimated=12; //unit is Qbit
  UWORD32  TOAMAX;
  WORD16   IZW,ISH,i;
  UWORD32  TOA_estimated=0;      //unit is Qbit
  UWORD16  Trans_active;

  if ((l1_mode==CON_EST_MODE2)||(l1_mode==DEDIC_MODE)
#if L1_GPRS
    || l1_mode==PACKET_TRANSFER_MODE
#endif
    )
    Trans_active=TRUE;
  else Trans_active=FALSE;

  /* TOA offset computation and clock adjustement */
  TOAMAX=0;
  for (i=1;i<TOA_HISTO_LEN;i++)
  {
    if (TOASP[i]>TOAMAX)
      TOAMAX=TOASP[i];
  }
  TOAMAX >>= C_RED;
  i=1;IZW=0;
  while (i<TOA_HISTO_LEN && IZW==0)
  {
    if (TOASP[i]>=TOAMAX)
      IZW=i;
    i++;
  }

  /* Estimated TOA calculation */
  if (TOASP[IZW-1]<(2*TOAMAX/3))
  {
    TOA_estimated=IZW;
    TOA_estimated *= 4; // unit in QBit
  }
  else
  {
#if 0	/* fix added in LoCosto, not present in TCS211 */
    UWORD32 TOA_divisor;
#endif
    TOA_estimated=(TOASP[IZW]*IZW)+(TOASP[IZW-1]*(IZW-1)>>C_GEW);
    TOA_estimated *= 8; //F13.3 in order to have qBit precision
#if 0
    TOA_divisor = TOASP[IZW]+(TOASP[IZW-1] >> C_GEW);
    if (TOA_divisor!=0)
#endif
    {
      TOA_estimated /= TOASP[IZW]+(TOASP[IZW-1] >> C_GEW);
      TOA_estimated /= 2; // unit in QBit ("/8" then "*4" = "/2")
    }
#if 0
    else
    {
      TOA_estimated = 0;
    }
#endif
  }

  if (Trans_active)
    TOA_estimated=(TOA_estimated+(Old_TOA_estimated+4)) / 2;

  /* Offset calculation*/
  if (TOA_estimated>=17 || TOA_estimated<=15)
    ISH=TOA_estimated - 16;
  else
    ISH=0;

  if (Trans_active)
  {
    if (ISH>1) ISH=1;
    if (ISH<-1) ISH=-1;
  }
  else
  {
    if (ISH>8) ISH=8;
    if (ISH<-8) ISH=-8;
  }

  Old_TOA_estimated = TOA_estimated - ISH  - 4;


  return (ISH);
}

/*-------------------------------------------------------*/
/* l1ctl_toa()                                           */
/*-------------------------------------------------------*/
/* Parameters :                                          */
/* Return     :                                          */
/* Functionality : generate an histogram of TOA weighted */
/*                 with SNR                              */
/*-------------------------------------------------------*/
WORD16 l1ctl_toa(UWORD8 phase, UWORD32 l1_mode, UWORD16 SNR_val, UWORD16 TOA_val, BOOL *toa_update, UWORD16 *toa_period_count
#if (FF_L1_FAST_DECODING == 1)
    , UWORD8 skipped_values
#endif
    )
{
//         xSignalHeaderRec *msg;
         UWORD16           i;
         WORD16            TOA_period_len = TOA_PERIOD_LEN[l1_mode];
  static UWORD32           histo[TOA_HISTO_LEN];
  static WORD16            period_counter=0;
         UWORD32           SNR_ZW;
         WORD16            ISH=ISH_INVALID;

  UWORD8 histo_center;

#if 0
  if ((l1_mode==CON_EST_MODE2)||(l1_mode==DEDIC_MODE))
    histo_center=4;
  else
    histo_center=5;
#else
  histo_center=4;
#endif


  if (phase==TOA_INIT)
  {
     period_counter=0;

     for (i=0;i<TOA_HISTO_LEN;i++)
       histo[i]=0;
     histo[histo_center]=128;  //F6.10

     return(ISH);
  }
#if (FF_L1_FAST_DECODING == 1)
  /* Manage any missing bursts due to fast decoding */
  period_counter += skipped_values;
#endif

  period_counter++;
  /* Filter update */
  if (SNR_val>=C_SNRGR)
  {
     if (SNR_val>C_SNR_THR)
      SNR_ZW=C_SNR_THR;
     else
      SNR_ZW=SNR_val;
     histo[TOA_val+1]+=SNR_ZW; /* if TOA=0 histo[1]++                  */
                               /* if TOA=1 histo[2]++                  */
                               /* ...                                  */
                               /* if TOA=9 histo[10]++                 */
                               /* histo[0] is reserved for computation */
  }

  #if L1_GPRS
    if (l1_mode==PACKET_TRANSFER_MODE)
    {
      if (*toa_update)
      {
        // Get ISH.
        ISH = l1ctl_toa_update(histo, l1_mode);

        //reset TOA period length counter
        period_counter=0;

        //reset histogram
        for (i=0;i<TOA_HISTO_LEN;i++)
          histo[i]=0;
        histo[histo_center]=128;  //F6.10

        *toa_update       = FALSE;  // reset TOA update flag
        *toa_period_count = 0;      // reset TOA period counter
      }
    }
    else
  #endif
  if (period_counter>=TOA_period_len)
  // It is time to compute a new ISH and to reset the histogram.
  // Rem: ">=" is very important since a "l1 mode" change can give
  //      a "TOA_period_len" smaller than the previous one an
  //      therefore a "period_counter" may be already higher than
  //      the new "TOA_period_len".
  {
     // Get ISH.
     ISH = l1ctl_toa_update(histo, l1_mode);

     //reset TOA period length counter
     period_counter=0;

     //reset histogram
     for (i=0;i<TOA_HISTO_LEN;i++)
         histo[i]=0;
     histo[histo_center]=128;  //F6.10
  }

  // error a TOA is waiting to be updated in the TPU and will be erased
  #if (TRACE_TYPE==1) || (TRACE_TYPE==4)
    if (l1s.toa_shift != ISH_INVALID)
    {
      l1_trace_toa_not_updated(); // should not occur !!
    }
  #endif

  if (ISH != ISH_INVALID) // new TOA => set the mask frames
  {
     // Set mask counter to 2 (2 frames masked).
      l1s.toa_snr_mask = 2;
  }

  return(ISH);
}
#endif

/*-------------------------------------------------------*/
/* l1ctl_txpwr()                                         */
/*-------------------------------------------------------*/
/* Parameters :                                          */
/* Return     :                                          */
/* Functionality :                                       */
/*-------------------------------------------------------*/
UWORD8 l1ctl_txpwr(UWORD8 target_txpwr, UWORD8 current_txpwr)
{
  if(target_txpwr > current_txpwr)
  {
    current_txpwr ++;   // Increase TX power by 2 dB.
  }
  else
  if(target_txpwr < current_txpwr)
  {
    current_txpwr --;   // Decrease TX power by 2 dB.
  }

  return(current_txpwr);
}


/************************************/
/* Automatic Gain Control           */
/************************************/
/*-------------------------------------------------------*/
/* l1ctl_encode_delta1()                                 */
/*-------------------------------------------------------*/
/* Parameters :                                          */
/* Return     :                                          */
/* Functionality :                                       */
/*-------------------------------------------------------*/
#if(L1_FF_MULTIBAND == 0)
WORD8 l1ctl_encode_delta1(UWORD16 radio_freq)
{
  WORD8 freq_band;

  switch(l1_config.std.id)
  {
    case GSM:
    case GSM_E:
    case DCS1800:
    case PCS1900:
    case GSM850:
      freq_band = l1_config.std.cal_freq1_band1;
      break;
    case DUAL:
    case DUALEXT:
    case DUAL_US:
      if(radio_freq >= l1_config.std.first_radio_freq_band2)
        freq_band = l1_config.std.cal_freq1_band2;
      else
        freq_band = l1_config.std.cal_freq1_band1;
      break;
  }
  return(freq_band);
}
#endif
/*-------------------------------------------------------*/
/* l1ctl_encode_lna()                                    */
/*-------------------------------------------------------*/
/* Parameters :                                          */
/* Return     :                                          */
/* Functionality :                                       */
/*-------------------------------------------------------*/
#if (L1_FF_MULTIBAND == 0)
void l1ctl_encode_lna( UWORD8   input_level,
                       UWORD8  *lna_state,
                       UWORD16  radio_freq)
{

  /*** LNA Hysteresis is implemented as following :

            |
          On|---<>----+-------+
            |         |       |
   LNA      |         |       |
            |         ^       v
            |         |       |
            |         |       |
         Off|         +-------+----<>-----
            +--------------------------------
              50      40      30      20   input_level /-dBm
                   THR_HIGH THR_LOW                          ***/





  if(((l1_config.std.id == DUAL) || (l1_config.std.id == DUALEXT) ||(l1_config.std.id == DUAL_US)) &&
     (radio_freq >= l1_config.std.first_radio_freq_band2))
  {
    if      ( input_level > l1_config.std.lna_switch_thr_high_band2  )  // < -40dBm ?
    {
       *lna_state =  LNA_ON;  // lna_off = FALSE
    }
    else if ( input_level < l1_config.std.lna_switch_thr_low_band2   )   // > -30dBm ?
    {
       *lna_state =  LNA_OFF; // lna off = TRUE
    }
  }
  else
  {
    if      ( input_level > l1_config.std.lna_switch_thr_high_band1  )  // < -40dBm ?
    {
       *lna_state =  LNA_ON;  // lna_off = FALSE
    }
    else if ( input_level < l1_config.std.lna_switch_thr_low_band1   )   // > -30dBm ?
    {
       *lna_state =  LNA_OFF; // lna off = TRUE
    }
  }
    
}

#endif

/*-------------------------------------------------------*/
/* l1ctl_csgc()                                          */
/*-------------------------------------------------------*/
/* Description:                                          */
/* ============                                          */
/* If we are running the first pass of a measurement     */
/* session, we use the HIGH_AGC default agc setting to   */
/* compute the input level from the measured power from  */
/* the DSP. If this input level is saturated we set a    */
/* saturation flag, otherwise we validate the measure and*/
/* store, for the considered carrier, the input level.   */
/* When all the carriers have been scanned and some have */
/* been flagged "saturated", we measure them with the    */
/* LOW_AGC agc setting, then store, for the considered   */
/* carrier, the input level.                             */
/*-------------------------------------------------------*/
UWORD8 l1ctl_csgc(UWORD8 pm, UWORD16 radio_freq)
{
   WORD16   current_IL, current_calibrated_IL;
   WORD8    delta1_freq, delta2_freq;
   WORD16   delta_drp_gain=0;
   UWORD32  index;
   UWORD16  g_magic;
   #if (RF_FAM == 61) && (L1_FF_MULTIBAND == 0)
     UWORD16  arfcn;
   #endif 
   UWORD16 dco_algo_ctl_pw_temp = 0;
   UWORD8 if_ctl = 0;
   #if (RF_RAM == 61) && (CODE_VERSION != SIMULATION)
     UWORD8 if_threshold = C_IF_ZERO_LOW_THRESHOLD_GSM;
   #endif

#if (L1_FF_MULTIBAND == 0)

   // initialize index
   index = radio_freq - l1_config.std.radio_freq_index_offset;

#else

   index = 
    l1_multiband_radio_freq_convert_into_operative_radio_freq(radio_freq);

#endif /*if(L1_FF_MULTIBAND == 0)*/

   delta1_freq = l1ctl_encode_delta1(radio_freq);
   delta2_freq = l1ctl_encode_delta2(radio_freq);

   g_magic = l1ctl_get_g_magic(radio_freq);

#if (RF_FAM == 61) && (L1_FF_MULTIBAND == 0)
   arfcn = Convert_l1_radio_freq(radio_freq);
#endif 

   if (l1a_l1s_com.full_list.meas_1st_pass_read)
   {
     // We validate or not power measure (pm) for the considered carrier
     // with measurement achieved with HIGH_AGC setting. We are working
     // with non calibrated IL to avoid saturation
#if(RF_FAM == 61)
   #if (CODE_VERSION != SIMULATION)

     #if (PWMEAS_IF_MODE_FORCE == 0)
       cust_get_if_dco_ctl_algo(&dco_algo_ctl_pw_temp, &if_ctl, (UWORD8) L1_IL_INVALID ,
           0,
           radio_freq,if_threshold);
     #else
       if_ctl = IF_120KHZ_DSP;
       dco_algo_ctl_pw_temp = DCO_IF_0KHZ;
     #endif

     #if (L1_FF_MULTIBAND == 0)
       delta_drp_gain = drp_gain_correction(arfcn, LNA_ON, (l1_config.params.high_agc << 1));    // F7.1 format
     #else
       delta_drp_gain = drp_gain_correction(radio_freq, LNA_ON, (l1_config.params.high_agc << 1));    // F7.1 format
     #endif // MULTIBAND == 0 else

     if(if_ctl == IF_100KHZ_DSP){
       delta_drp_gain += SCF_ATTENUATION_LIF_100KHZ;
     }
     else{ /* i.e. if_ctl = IF_120KHZ_DSP*/
       delta_drp_gain += SCF_ATTENUATION_LIF_120KHZ;
     }

   #endif
#endif
     if (0==pm)  // Check and filter illegal pm value by using last valid IL
       current_IL = (WORD16)(l1a_l1s_com.last_input_level[index].input_level);
     else
     {
#if TESTMODE
       if (!l1_config.agc_enable)
         current_IL = (WORD16)(-(pm - ( (l1_config.tmode.rx_params.agc << 1) - delta_drp_gain )  - g_magic));
       else
#endif
         current_IL = (WORD16)(-(pm - ( (l1_config.params.high_agc <<1) - delta_drp_gain) - g_magic));
                 // for array index purpose, we work with positive IL

     }

     // NOTE: lna_value do not appear in this formula because lna is ALWAYS ON for
     // ----  this algorithm, so lna_value=lna_off*l1_config.params.lna_att_gsm=0

     if ((current_IL<l1_config.params.high_agc_sat_thr)  // Warning : we are working with positive IL
                                                         // for IL_2_AGC_xx index purpose.
     #if TESTMODE
         && (l1_config.agc_enable)
     #endif
        )
     {
       // pm is saturated so measure is not valid
       l1a_l1s_com.full_list.nbr_sat_carrier_ctrl++;
       l1a_l1s_com.full_list.nbr_sat_carrier_read++;
       l1a_l1s_com.full_list.sat_flag[l1a_l1s_com.full_list.next_to_read] = 1;
     }
     else
     {
       current_calibrated_IL = current_IL - delta1_freq - delta2_freq;

       #if TESTMODE
         // When running with fixed AGC setting saturated carriers may occur:
         // protect against negative IL;
         if ((!l1_config.agc_enable) && (current_calibrated_IL < 0))
         {
           current_calibrated_IL=0;
           current_IL=0;
         }
       #endif

       // Protect IL stores against overflow
       if (current_calibrated_IL>INDEX_MAX)
         current_calibrated_IL=INDEX_MAX;
       if (current_IL>INDEX_MAX)
         current_IL=INDEX_MAX;

       // we validate the measure and save input_level and lna_off fields.
       l1ctl_encode_lna((UWORD8)(current_calibrated_IL>>1),
                        &(l1a_l1s_com.last_input_level[index].lna_off),
                        radio_freq);

       l1a_l1s_com.last_input_level[index].input_level = (UWORD8)current_IL +
	 l1ctl_get_lna_att(radio_freq) *
	   l1a_l1s_com.last_input_level[index].lna_off;

       l1a_l1s_com.full_list.sat_flag[l1a_l1s_com.full_list.next_to_read] = 0;
     }
   }
   else // 2nd pass if any.
   {
     // we validate the measure and save input_level and lna_off(always 0)
     // fields.
     #if(RF_FAM == 61)
       #if (CODE_VERSION != SIMULATION)
        cust_get_if_dco_ctl_algo(&dco_algo_ctl_pw_temp, &if_ctl, (UWORD8) L1_IL_INVALID,
            0,radio_freq,if_threshold);
        #if (L1_FF_MULTIBAND == 0)       
          delta_drp_gain = drp_gain_correction(arfcn, LNA_ON, (l1_config.params.low_agc << 1));    // F7.1 format
        #else
          delta_drp_gain = drp_gain_correction(radio_freq, LNA_ON, (l1_config.params.low_agc << 1));    // F7.1 format
        #endif 
        if(if_ctl == IF_100KHZ_DSP){
          delta_drp_gain += SCF_ATTENUATION_LIF_100KHZ;
        }
        else{ /* i.e. if_ctl = IF_120KHZ_DSP*/
          delta_drp_gain += SCF_ATTENUATION_LIF_120KHZ;
        }
       #endif
     #endif


     if (0==pm)  // Check and filter illegal pm value by using last valid IL
       current_IL = (WORD16)(l1a_l1s_com.last_input_level[index].input_level);
     else
       current_IL = (WORD16)(-(pm - ( (l1_config.params.low_agc << 1) - delta_drp_gain ) - g_magic));

     current_calibrated_IL = current_IL - delta1_freq - delta2_freq;

     // Protect IL stores against overflow
     if (current_calibrated_IL>INDEX_MAX)
       current_calibrated_IL=INDEX_MAX;
     if (current_IL>INDEX_MAX)
       current_IL=INDEX_MAX;

     l1ctl_encode_lna((UWORD8)(current_calibrated_IL>>1),
                      &(l1a_l1s_com.last_input_level[index].lna_off),
                      radio_freq);

     l1a_l1s_com.last_input_level[index].input_level = (UWORD8)current_IL +
	l1ctl_get_lna_att(radio_freq) *
	  l1a_l1s_com.last_input_level[index].lna_off;

     l1a_l1s_com.full_list.sat_flag[l1a_l1s_com.full_list.next_to_read] = 0;
   }

   return((UWORD8)current_calibrated_IL);
}

/*-------------------------------------------------------*/
/* l1ctl_pgc()                                           */
/*-------------------------------------------------------*/
/* Description : For a given radio_freq, last_known_agc is    */
/* ============  based on a prior knowledge (the last    */
/*               stored input_level for the considered   */
/*               carrier). From the power measurement on */
/*               this carrier (pm), we update the        */
/*               input_level for this carrier, for the   */
/*               next task to control.                   */
/*-------------------------------------------------------*/
UWORD8 l1ctl_pgc(UWORD8 pm, UWORD8 last_known_il,
                 UWORD8 lna_off, UWORD16 radio_freq)
{
   WORD32  last_known_agc;
   WORD32  current_IL, current_calibrated_IL;
   WORD8   delta1_freq, delta2_freq;
   WORD16  delta_drp_gain=0;
   WORD32  index, lna_value;
   #if (RF_RAM == 61) && (CODE_VERSION != SIMULATION)
     UWORD16 arfcn;
   #endif
   UWORD16 dco_algo_ctl_pw_temp = 0;
   UWORD8 if_ctl = 0;
   #if (RF_RAM == 61) && (CODE_VERSION != SIMULATION)
     UWORD8 if_threshold = C_IF_ZERO_LOW_THRESHOLD_GSM;
   #endif

#if (L1_FF_MULTIBAND == 0)

   // initialize index
   index = radio_freq - l1_config.std.radio_freq_index_offset;

#else

   index = l1_multiband_radio_freq_convert_into_operative_radio_freq(radio_freq);

#endif // #if (L1_FF_MULTIBAND == 0) else

   delta1_freq = l1ctl_encode_delta1(radio_freq);
   delta2_freq = l1ctl_encode_delta2(radio_freq);

   lna_value = lna_off * l1ctl_get_lna_att(radio_freq);

   last_known_agc = (Cust_get_agc_from_IL(radio_freq, last_known_il >> 1, PWR_ID)) << 1;
   // F7.1 in order to be compatible with
   // pm and IL formats [-20,+140 in F7.1]
   // contain the input_level value we use
   // in the associated CTL task to build
   // the agc used in this CTL.

#if (RF_RAM == 61) && (CODE_VERSION != SIMULATION)
#if (L1_FF_MULTIBAND == 0)
  arfcn = Convert_l1_radio_freq(radio_freq);
#else
  arfcn = radio_freq;
#endif
#endif

#if(RF_FAM == 61)
   #if (CODE_VERSION != SIMULATION)

     #if (PWMEAS_IF_MODE_FORCE == 0)
    cust_get_if_dco_ctl_algo(&dco_algo_ctl_pw_temp, &if_ctl, (UWORD8) L1_IL_VALID ,
                                         last_known_il,
                                         radio_freq,if_threshold);
     #else
       if_ctl = IF_120KHZ_DSP;
       dco_algo_ctl_pw_temp = DCO_IF_0KHZ;
     #endif

     delta_drp_gain = drp_gain_correction(arfcn, lna_off, last_known_agc);     // F7.1 format
     if(if_ctl == IF_100KHZ_DSP){
       delta_drp_gain += SCF_ATTENUATION_LIF_100KHZ;
     }
     else{ /* i.e. if_ctl = IF_120KHZ_DSP*/
       delta_drp_gain += SCF_ATTENUATION_LIF_120KHZ;
     }

   #endif
#endif

   if (0==pm)  // Check and filter illegal pm value by using last valid IL
     current_IL = l1a_l1s_com.last_input_level[index].input_level - lna_value;
   else
     current_IL = -(pm - (last_known_agc - delta_drp_gain) + lna_value - l1ctl_get_g_magic(radio_freq));

   current_calibrated_IL = current_IL - delta1_freq - delta2_freq;

   // Protect IL stores against overflow
   if (current_calibrated_IL>INDEX_MAX)
     current_calibrated_IL=INDEX_MAX;
   if (current_IL>INDEX_MAX)
     current_IL=INDEX_MAX;

   // we validate the measure and save input_level and lna_off fields
   l1ctl_encode_lna((UWORD8)(current_calibrated_IL>>1),
                    &(l1a_l1s_com.last_input_level[index].lna_off),
                    radio_freq);

   l1a_l1s_com.last_input_level[index].input_level = (UWORD8)current_IL +
      l1ctl_get_lna_att(radio_freq) *
	l1a_l1s_com.last_input_level[index].lna_off;

   return((UWORD8)current_calibrated_IL);
}


/*-------------------------------------------------------*/
/* l1ctl_pgc2()                                          */
/*-------------------------------------------------------*/
/* Description :                                         */
/* =============                                         */
/* from power measurement pm_high_agc,                   */
/* achieve with an HIGH_AGC setting, and pm_low_agc      */
/* achieve with a LOW_AGC seeting, we deduce the new     */
/* AGC to apply in the next CTL task.                    */
/*-------------------------------------------------------*/
void l1ctl_pgc2(UWORD8 pm_high_agc, UWORD8 pm_low_agc, UWORD16 radio_freq)
{
   UWORD8   pm;
   WORD32   IL_high_agc, IL_low_agc, new_IL, current_calibrated_IL;
   WORD8    delta1_freq, delta2_freq;
   WORD16   delta_high_drp_gain=0;
   WORD16   delta_low_drp_gain=0;
   WORD32   index;
   UWORD16  g_magic;
   #if (RF_RAM == 61) && (CODE_VERSION != SIMULATION)
     UWORD16  arfcn;
   #endif
   UWORD16 dco_algo_ctl_pw_temp = 0;
   UWORD8 if_ctl = 0;
   #if (RF_RAM == 61) && (CODE_VERSION != SIMULATION)
     UWORD8 if_threshold = C_IF_ZERO_LOW_THRESHOLD_GSM;
   #endif

#if (L1_FF_MULTIBAND == 0)

   // initialize index
   index = radio_freq - l1_config.std.radio_freq_index_offset;

#else

   index = 
    l1_multiband_radio_freq_convert_into_operative_radio_freq(radio_freq);

#endif // #if (L1_FF_MULTIBAND == 0) else

   delta1_freq = l1ctl_encode_delta1(radio_freq);
   delta2_freq = l1ctl_encode_delta2(radio_freq);

   g_magic = l1ctl_get_g_magic(radio_freq);

   // lna_off was set to 0 during CTRL, so lna_value = 0 do not appear in the following
   // formula.

#if (RF_RAM == 61) && (CODE_VERSION != SIMULATION)
#if (L1_FF_MULTIBAND == 0)
  arfcn = Convert_l1_radio_freq(radio_freq);
#else
  arfcn = radio_freq;
#endif
#endif

   if ((0==pm_high_agc) || (0==pm_low_agc))  // Check and filter illegal pm value(s) by using last valid IL
     new_IL      = l1a_l1s_com.last_input_level[index].input_level;
   else
   {

#if(RF_FAM == 61)
   #if (CODE_VERSION != SIMULATION)

#if (PWMEAS_IF_MODE_FORCE == 0)
       cust_get_if_dco_ctl_algo(&dco_algo_ctl_pw_temp, &if_ctl, (UWORD8) L1_IL_INVALID ,
           0,
           radio_freq,if_threshold);
     #else
       if_ctl = IF_120KHZ_DSP;
       dco_algo_ctl_pw_temp = DCO_IF_0KHZ;
     #endif


	 delta_high_drp_gain = drp_gain_correction(arfcn, LNA_ON, (l1_config.params.high_agc << 1));    // F7.1 format
     delta_low_drp_gain  = drp_gain_correction(arfcn, LNA_ON, (l1_config.params.low_agc << 1));     // F7.1 format
     if(if_ctl == IF_100KHZ_DSP){
       delta_high_drp_gain += SCF_ATTENUATION_LIF_100KHZ;
       delta_low_drp_gain += SCF_ATTENUATION_LIF_100KHZ;
     }
     else{ /* i.e. if_ctl = IF_120KHZ_DSP*/
       delta_high_drp_gain += SCF_ATTENUATION_LIF_120KHZ;
       delta_low_drp_gain += SCF_ATTENUATION_LIF_120KHZ;
     }
   #endif
#endif

     IL_high_agc =  -(pm_high_agc - ((l1_config.params.high_agc << 1) - delta_high_drp_gain) - g_magic);
     IL_low_agc  =  -(pm_low_agc  - ((l1_config.params.low_agc << 1) - delta_low_drp_gain) - g_magic);

     // HIGH_AGC and LOW_AGC are formatted to F7.1 in order to be compatible with
     // pm and IL formats

     if (IL_low_agc>=l1_config.params.low_agc_noise_thr)
     // pm_low_agc was on the noise floor, so not valid
     {
       // whatever the value of pm_high_agc, we consider it
       // as the right setting
       new_IL = IL_high_agc;
       pm     = pm_high_agc;
     }
     else
     {
       // pm_low_agc is valid.
       if (IL_high_agc<=l1_config.params.high_agc_sat_thr)
       {
         // pm_high_agc is not valid, it's saturated.
         new_IL = IL_low_agc;
         pm     = pm_low_agc;
       }
       else
       {
         // both pm_low_agc and pm_high_agc are valid, so we test the one that
         // gives the maximum input level and consider it as the right setting.
        if (IL_high_agc<=IL_low_agc)
         {
           new_IL = IL_high_agc;
           pm     = pm_high_agc;
         }
         else
         {
           new_IL  = IL_low_agc;
           pm      = pm_low_agc;
         }
       }
     }
   }

   #if (TRACE_TYPE == 1) || (TRACE_TYPE == 4)
     RTTL1_FILL_MON_MEAS(pm_high_agc, IL_high_agc - delta1_freq - delta2_freq, MS_AGC_ID, radio_freq)
     RTTL1_FILL_MON_MEAS(pm_low_agc,  IL_low_agc  - delta1_freq - delta2_freq, MS_AGC_ID, radio_freq)
   #endif

   current_calibrated_IL = new_IL - delta1_freq - delta2_freq;

   // Protect IL stores against overflow
   if (current_calibrated_IL>INDEX_MAX)
     current_calibrated_IL=INDEX_MAX;
   if (new_IL>INDEX_MAX)
     new_IL=INDEX_MAX;

   // Updating of input_level and lna_off fields in order to correctly
   // setting the AGC for the next task.
   l1ctl_encode_lna((UWORD8)(current_calibrated_IL>>1),
                    &(l1a_l1s_com.last_input_level[index].lna_off),
                    radio_freq);

   l1a_l1s_com.last_input_level[index].input_level = (UWORD8)new_IL +
      l1ctl_get_lna_att(radio_freq) *
	l1a_l1s_com.last_input_level[index].lna_off;
}


/*-------------------------------------------------------*/
/* l1ctl_find_max()                                      */
/*-------------------------------------------------------*/
/* Parameters :                                          */
/* Return     :                                          */
/* Functionality :                                       */
/*-------------------------------------------------------*/
UWORD8 l1ctl_find_max(UWORD8 *buff, UWORD8 buffer_len)
{

  // WARNING: for array index purpose we work with POSITIVE input level
  //          so maximum search for negative numbers is equivalent to
  //          minimum search for positive numbers!!!!!!
  //          (-30 > -120 but 30 < 120)

  UWORD8  maximum = 240;
  UWORD8  i;

  for (i=0; i<buffer_len; i++)
  {
    if (buff[i]<maximum)
      maximum=buff[i];
  }

  return(maximum);
}

/*-------------------------------------------------------*/
/* l1ctl_pagc()                                          */
/*-------------------------------------------------------*/
/* Description :                                         */
/* ===========                                           */
/* We deduce the last_known_agc from the last stored     */
/* input_level for the considered carrier. We use this   */
/* agc value to "build" the input level linked to the pm */
/* we have just read.                                    */
/* This input level is used to feed a fifo of 4 elements */
/* and then compute an input_level maximum. This value is*/
/* used to update the input_level for this carrier. This */
/* input_level will be used for the next task to control.*/
/*-------------------------------------------------------*/
UWORD8 l1ctl_pagc(UWORD8 pm, UWORD16 radio_freq, T_INPUT_LEVEL *IL_info_ptr)
{
   WORD8   delta1_freq, delta2_freq;
   WORD16  delta_drp_gain=0;
   WORD32  last_known_agc;
   UWORD8  IL_max;
   WORD32  current_IL, current_calibrated_IL;
   UWORD8  i;
   WORD32  lna_value;
   #if (RF_RAM == 61) && (CODE_VERSION != SIMULATION)
     UWORD16 arfcn;
   #endif
   UWORD8  lna_off;
   UWORD16 dco_algo_ctl_pw_temp = 0;
   UWORD8 if_ctl = 0;
   #if (RF_RAM == 61) && (CODE_VERSION != SIMULATION)
     UWORD8 if_threshold = C_IF_ZERO_LOW_THRESHOLD_GSM;
   #endif

   delta1_freq = l1ctl_encode_delta1(radio_freq);
   delta2_freq = l1ctl_encode_delta2(radio_freq);

   // Update fifo
   for (i=3;i>0;i--)
     l1a_l1s_com.Scell_info.buff_beacon[i]=l1a_l1s_com.Scell_info.buff_beacon[i-1];

   // from the lna state (ON/OFF) we compute the attenuation
   // that was applied to signal when performing the power
   // measure.
   lna_value = l1a_l1s_com.Scell_used_IL_dd.lna_off * l1ctl_get_lna_att(radio_freq);

   // Compute applied agc for this pm
   last_known_agc = (Cust_get_agc_from_IL(radio_freq, l1a_l1s_com.Scell_used_IL_dd.input_level >> 1, MAX_ID)) << 1;
                                  // F7.1 in order to be compatible
                                  // with pm and IL formats
                                  // contain the input_level value we use
                                  // in the associated CTL task to build
                                  // the agc used in this CTL.

#if (RF_RAM == 61) && (CODE_VERSION != SIMULATION)
#if (L1_FF_MULTIBAND == 0)
  arfcn = Convert_l1_radio_freq(radio_freq);
#else
  arfcn = radio_freq;
#endif
#endif

#if(RF_FAM == 61)
   #if (CODE_VERSION != SIMULATION)

    cust_get_if_dco_ctl_algo(&dco_algo_ctl_pw_temp, &if_ctl, (UWORD8) L1_IL_VALID ,
                                          l1a_l1s_com.Scell_used_IL_dd.input_level,
                                         radio_freq,if_threshold);
     lna_off = l1a_l1s_com.Scell_used_IL_dd.lna_off;
     delta_drp_gain = drp_gain_correction(arfcn, lna_off, last_known_agc);     // F7.1 format
     if(if_ctl == IF_100KHZ_DSP){
       delta_drp_gain += SCF_ATTENUATION_LIF_100KHZ;
     }
     else{ /* i.e. if_ctl = IF_120KHZ_DSP*/
       delta_drp_gain += SCF_ATTENUATION_LIF_120KHZ;
     }

   #endif
#endif

   if (0==pm)  // Check and filter illegal pm value by using last valid IL
     current_IL = IL_info_ptr->input_level - lna_value;
   else
     current_IL = -(pm - (last_known_agc - delta_drp_gain) + lna_value - l1ctl_get_g_magic(radio_freq));

   current_calibrated_IL = current_IL - delta1_freq - delta2_freq;

   // Protect IL stores against overflow
   if (current_calibrated_IL>INDEX_MAX)
     current_calibrated_IL=INDEX_MAX;
   if (current_IL>INDEX_MAX)
     current_IL=INDEX_MAX;

   l1a_l1s_com.Scell_info.buff_beacon[0]  = (UWORD8)current_IL;

   IL_max = l1ctl_find_max(&(l1a_l1s_com.Scell_info.buff_beacon[0]),4);

   //input levels are always stored with lna_on
   l1ctl_encode_lna( (UWORD8)(current_calibrated_IL>>1),
                     &(IL_info_ptr->lna_off),
                     radio_freq );

   IL_info_ptr->input_level = IL_max + l1ctl_get_lna_att(radio_freq) *
					IL_info_ptr->lna_off;

   #if L2_L3_SIMUL
     #if (DEBUG_TRACE==BUFFER_TRACE_PAGC)
       buffer_trace(4,IL_info_ptr->input_level,last_known_agc,
                      l1a_l1s_com.Scell_used_IL_dd.input_level,Cust_get_agc_from_IL(radio_freq, IL_max >> 1, MAX_ID));
     #endif
   #endif

   return((UWORD8)current_calibrated_IL);
}

/*-------------------------------------------------------*/
/* l1ctl_dpagc()                                         */
/*-------------------------------------------------------*/
/* Description :                                         */
/* ===========                                           */
/* Based on the same principle as the one used for PAGC  */
/* algorithm except that we feed 3 different fifo:       */
/* 1) one is dedicated to BCCH carrier                   */
/* 2) another one is dedicated to all the other type of  */
/*    bursts                                             */
/* 3) the last one is dedicated to non DTX influenced    */
/*    bursts                                             */
/*-------------------------------------------------------*/
UWORD8 l1ctl_dpagc(BOOL dtx_on, BOOL beacon, UWORD8 pm, UWORD16 radio_freq, T_INPUT_LEVEL *IL_info_ptr)
{
  UWORD8       av_G_all, av_G_DTX;
  UWORD8       max_G_all, max_G_DTX;
  WORD32       last_known_agc, new_IL, current_calibrated_IL;
  WORD8        delta1_freq, delta2_freq;
  WORD16       delta_drp_gain=0;
  UWORD8       i;
  UWORD8      *tab_ptr;
  T_DEDIC_SET *aset;
  WORD32       lna_value;
  #if (RF_RAM == 61) && (CODE_VERSION != SIMULATION)
    UWORD16      arfcn;
  #endif
  UWORD8       lna_off;
  UWORD16 dco_algo_ctl_pw_temp = 0;
  UWORD8 if_ctl = 0;
  #if (RF_RAM == 61) && (CODE_VERSION != SIMULATION)
    UWORD8 if_threshold = C_IF_ZERO_LOW_THRESHOLD_GSM;
  #endif

  delta1_freq = l1ctl_encode_delta1(radio_freq);
  delta2_freq = l1ctl_encode_delta2(radio_freq);

  aset = l1a_l1s_com.dedic_set.aset;

  if (beacon)
    tab_ptr = l1a_l1s_com.Scell_info.buff_beacon;
  else
    tab_ptr = aset->G_all;

  // Update fifo
  for (i=DPAGC_FIFO_LEN-1;i>0;i--)
    tab_ptr[i]=tab_ptr[i-1];

  #if TESTMODE
    if (!l1_config.agc_enable)
    {
      // AGC gain can only be controlled in 2dB steps as the bottom bit (bit zero)
      // corresponds to the lna_off bit
      last_known_agc = (l1_config.tmode.rx_params.agc) << 1;
      lna_value = (l1_config.tmode.rx_params.lna_off) * l1ctl_get_lna_att(radio_freq);
    }
    else
  #endif
    {
      #if DPAGC_MAX_FLAG
        last_known_agc  = (Cust_get_agc_from_IL(radio_freq, l1a_l1s_com.Scell_used_IL_dd.input_level >> 1, MAX_ID)) << 1;
        // F7.1 in order to be compatible with pm and IL formats
      #else
        last_known_agc  = (Cust_get_agc_from_IL(radio_freq, l1a_l1s_com.Scell_used_IL_dd.input_level >> 1, AV_ID)) << 1;
        // F7.1 in order to be compatible with pm and IL formats
      #endif
      // input_level_dd : contain the input_level value we use
      // in the associated CTL task to build the agc used in this CTL.

      lna_value = l1a_l1s_com.Scell_used_IL_dd.lna_off * l1ctl_get_lna_att(radio_freq);
    }

#if (RF_RAM == 61) && (CODE_VERSION != SIMULATION)
#if (L1_FF_MULTIBAND == 0)
  arfcn = Convert_l1_radio_freq(radio_freq);
#else
  arfcn = radio_freq;
#endif
#endif

#if(RF_FAM == 61)
   #if (CODE_VERSION != SIMULATION)

    cust_get_if_dco_ctl_algo(&dco_algo_ctl_pw_temp, &if_ctl, (UWORD8) L1_IL_VALID ,
                                          l1a_l1s_com.Scell_used_IL_dd.input_level,
                                         radio_freq,if_threshold);
     lna_off = l1a_l1s_com.Scell_used_IL_dd.lna_off;
     delta_drp_gain = drp_gain_correction(arfcn, lna_off, last_known_agc);     // F7.1 format
     if(if_ctl == IF_100KHZ_DSP){
       delta_drp_gain += SCF_ATTENUATION_LIF_100KHZ;
     }
     else{ /* i.e. if_ctl = IF_120KHZ_DSP*/
       delta_drp_gain += SCF_ATTENUATION_LIF_120KHZ;
     }

   #endif
#endif

  if (0==pm)  // Check and filter illegal pm value by using last valid IL
    new_IL    = IL_info_ptr->input_level - lna_value;
  else
    new_IL    = -(pm - (last_known_agc - delta_drp_gain) + lna_value - l1ctl_get_g_magic(radio_freq));

  current_calibrated_IL = new_IL - delta1_freq - delta2_freq;

  // Protect IL stores against overflow
  if (current_calibrated_IL>INDEX_MAX)
    current_calibrated_IL=INDEX_MAX;

  #if TESTMODE
    if (l1tm.tmode_state.dedicated_active)  // Implies l1_config.TestMode = 1
    {
      // Update l1tm.tmode_stats.rssi_fifo (delay line from index 3 to 0)
      for (i=(sizeof(l1tm.tmode_stats.rssi_fifo)/sizeof(l1tm.tmode_stats.rssi_fifo[0]))-1; i>0; i--)
      {
        l1tm.tmode_stats.rssi_fifo[i] = l1tm.tmode_stats.rssi_fifo[i-1];
      }
      l1tm.tmode_stats.rssi_fifo[0] = current_calibrated_IL; // rssi value is F7.1
      l1tm.tmode_stats.rssi_recent  = current_calibrated_IL; // rssi value is F7.1
    }
  #endif

  if (new_IL>INDEX_MAX)
    new_IL=INDEX_MAX;

  tab_ptr[0] = (UWORD8)new_IL;

  if (dtx_on && !beacon)
  {
   // Update DTX fifo
   for (i=DPAGC_FIFO_LEN-1;i>0;i--)
     aset->G_DTX[i]=aset->G_DTX[i-1];

   aset->G_DTX[0]=tab_ptr[0];
  }

  /* Computation of MAX{G_all[i],G_DTX[j]} i,j=0..3 */
  #if DPAGC_MAX_FLAG
    max_G_all   = l1ctl_find_max(&(tab_ptr[0]),DPAGC_FIFO_LEN);

    if (!beacon)
    {
      max_G_DTX = l1ctl_find_max(&(aset->G_DTX[0]),DPAGC_FIFO_LEN);

      // WARNING: for array index purpose we work with POSITIVE input level
      //          so maximum search for negative numbers is equivalent to
      //          minimum search for positive numbers!!!!!!
      //          (-30 > -120 but 30 < 120)
      if (max_G_all <= max_G_DTX)
        new_IL = max_G_all;
      else
        new_IL = max_G_DTX;
    }
    else
      new_IL = max_G_all;
  #else
    av_G_all=av_G_DTX=0;

    for (i=0;i<DPAGC_FIFO_LEN;i++)
     av_G_all += tab_ptr[i];

    av_G_all /= DPAGC_FIFO_LEN;

    if (!beacon)
    {
      for (i=0;i<DPAGC_FIFO_LEN;i++)
       av_G_DTX += aset->G_DTX[i];

      av_G_DTX /= DPAGC_FIFO_LEN;

      if (av_G_all >= av_G_DTX)
        new_IL = av_G_all;
      else
        new_IL = av_G_DTX;
    }
    else
      new_IL = av_G_all;
  #endif

  // Updating of input_level and lna_off fields in order to correctly
  // setting the AGC for the next task.
  // input_level is always store with lna_on
  l1ctl_encode_lna( (UWORD8)(current_calibrated_IL>>1),
                    &(IL_info_ptr->lna_off),
                    radio_freq );

  IL_info_ptr->input_level = (UWORD8)new_IL + l1ctl_get_lna_att(radio_freq) *
					IL_info_ptr->lna_off;

  #if L2_L3_SIMUL
    #if (DEBUG_TRACE==BUFFER_TRACE_DPAGC)
      buffer_trace(4,IL_info_ptr->input_level,last_known_agc,
                     l1a_l1s_com.Scell_used_IL_dd.input_level,Cust_get_agc_from_IL(radio_freq, new_IL >> 1, MAX_ID));
    #endif
  #endif

  return((UWORD8)current_calibrated_IL);
}

#if (AMR == 1)
  /*-------------------------------------------------------*/
  /* l1ctl_dpagc_amr()                                     */
  /*-------------------------------------------------------*/
  /* Description :                                         */
  /* ===========                                           */
  /* Based on the same principle as the one used for DPAGC */
  /* algorithm except that the way to feed the G_dtx is    */
  /* different                                             */
  /*-------------------------------------------------------*/
  UWORD8 l1ctl_dpagc_amr(BOOL dtx_on, BOOL beacon, UWORD8 pm, UWORD16 radio_freq, T_INPUT_LEVEL *IL_info_ptr)
  {
    UWORD8       av_G_all, av_G_DTX;
    UWORD8       max_G_all, max_G_DTX, max_il;
    WORD32       last_known_agc, new_IL, current_calibrated_IL;
    WORD8        delta1_freq, delta2_freq;
    WORD16       delta_drp_gain=0;
    UWORD8       i;
    UWORD8       *tab_ptr, *tab_amr_ptr;
    T_DEDIC_SET *aset;
    WORD32       lna_value;
    #if (RF_RAM == 61) && (CODE_VERSION != SIMULATION)
	UWORD16  arfcn;
    #endif
    UWORD8       lna_off;
    UWORD16 dco_algo_ctl_pw_temp = 0;
    UWORD8 if_ctl = 0;
    #if (RF_RAM == 61) && (CODE_VERSION != SIMULATION)
	UWORD8 if_threshold = C_IF_ZERO_LOW_THRESHOLD_GSM;
    #endif

    delta1_freq = l1ctl_encode_delta1(radio_freq);
    delta2_freq = l1ctl_encode_delta2(radio_freq);

    aset = l1a_l1s_com.dedic_set.aset;

    if (beacon)
    tab_ptr = l1a_l1s_com.Scell_info.buff_beacon;
    else
    tab_ptr = aset->G_all;

    // Update fifo
    for (i=DPAGC_FIFO_LEN-1;i>0;i--)
      tab_ptr[i]=tab_ptr[i-1];

    tab_amr_ptr = aset->G_amr;
    for (i=DPAGC_AMR_FIFO_LEN-1;i>0;i--)
      tab_amr_ptr[i]=tab_amr_ptr[i-1];

    #if TESTMODE
      if (!l1_config.agc_enable)
      {
        // AGC gain can only be controlled in 2dB steps as the bottom bit (bit zero)
        // corresponds to the lna_off bit
        last_known_agc = (l1_config.tmode.rx_params.agc) << 1;
        lna_value = (l1_config.tmode.rx_params.lna_off) * l1ctl_get_lna_att(radio_freq);
      }
      else
    #endif
      {
        #if DPAGC_MAX_FLAG
          last_known_agc  = (Cust_get_agc_from_IL(radio_freq, l1a_l1s_com.Scell_used_IL_dd.input_level >> 1, MAX_ID)) << 1;
          // F7.1 in order to be compatible with pm and IL formats
        #else
          last_known_agc  = (Cust_get_agc_from_IL(radio_freq, l1a_l1s_com.Scell_used_IL_dd.input_level >> 1, AV_ID)) << 1;
          // F7.1 in order to be compatible with pm and IL formats
        #endif
        // input_level_dd : contain the input_level value we use
        // in the associated CTL task to build the agc used in this CTL.

        lna_value = l1a_l1s_com.Scell_used_IL_dd.lna_off * l1ctl_get_lna_att(radio_freq);
      }

#if (RF_RAM == 61) && (CODE_VERSION != SIMULATION)
#if (L1_FF_MULTIBAND == 0)
  arfcn = Convert_l1_radio_freq(radio_freq);
#else
  arfcn = radio_freq;
#endif
#endif

#if(RF_FAM == 61)
   #if (CODE_VERSION != SIMULATION)
    cust_get_if_dco_ctl_algo(&dco_algo_ctl_pw_temp, &if_ctl, (UWORD8) L1_IL_VALID ,
                                          l1a_l1s_com.Scell_used_IL_dd.input_level,
                                         radio_freq,if_threshold);
     lna_off = l1a_l1s_com.Scell_used_IL_dd.lna_off;
     delta_drp_gain = drp_gain_correction(arfcn, lna_off, last_known_agc);     // F7.1 format
     if(if_ctl == IF_100KHZ_DSP){
       delta_drp_gain += SCF_ATTENUATION_LIF_100KHZ;
     }
     else{ /* i.e. if_ctl = IF_120KHZ_DSP*/
       delta_drp_gain += SCF_ATTENUATION_LIF_120KHZ;
     }
   #endif
#endif

    if (0==pm)  // Check and filter illegal pm value by using last valid IL
      new_IL    = IL_info_ptr->input_level - lna_value;
    else
      new_IL    = -(pm - (last_known_agc - delta_drp_gain) + lna_value - l1ctl_get_g_magic(radio_freq));

    current_calibrated_IL = new_IL - delta1_freq - delta2_freq;

    // Protect IL stores against overflow
    if (current_calibrated_IL>INDEX_MAX)
      current_calibrated_IL=INDEX_MAX;

    #if TESTMODE
      if (l1tm.tmode_state.dedicated_active)  // Implies l1_config.TestMode = 1
      {
        // Update l1tm.tmode_stats.rssi_fifo (delay line from index 3 to 0)
        for (i=(sizeof(l1tm.tmode_stats.rssi_fifo)/sizeof(l1tm.tmode_stats.rssi_fifo[0]))-1; i>0; i--)
       {
          l1tm.tmode_stats.rssi_fifo[i] = l1tm.tmode_stats.rssi_fifo[i-1];
        }
        l1tm.tmode_stats.rssi_fifo[0] = current_calibrated_IL; // rssi value is F7.1
        l1tm.tmode_stats.rssi_recent  = current_calibrated_IL; // rssi value is F7.1
      }
    #endif

    if (new_IL>INDEX_MAX)
      new_IL=INDEX_MAX;

     tab_ptr[0] = (UWORD8)new_IL;
     tab_amr_ptr[0] = (UWORD8)new_IL;

    if (dtx_on && !beacon)
    {
      // a new AMR block is received, feed the G_dtx with the max_il of the block
      for (i=DPAGC_FIFO_LEN-1;i>0;i--)
        aset->G_DTX[i]=aset->G_DTX[i-1];

      if (l1a_l1s_com.dedic_set.aset->achan_ptr->mode == TCH_AHS_MODE)
      {
        // Keep the max_il between the last 2 bursts
        if (aset->G_amr[0] > aset->G_amr[1])
          max_il = aset->G_amr[0];
        else
          max_il = aset->G_amr[1];
      }
      else
      {
        // Keep the max_il between the last 4 bursts
        max_il = l1ctl_find_max(&aset->G_amr[0], DPAGC_AMR_FIFO_LEN);
      }

      aset->G_DTX[0]= max_il;
    }

    /* Computation of MAX{G_all[i],G_DTX[j]} i,j=0..3 */
    #if DPAGC_MAX_FLAG
      max_G_all   = l1ctl_find_max(&(tab_ptr[0]),DPAGC_FIFO_LEN);

      if (!beacon)
      {
        max_G_DTX = l1ctl_find_max(&(aset->G_DTX[0]),DPAGC_FIFO_LEN);

        // WARNING: for array index purpose we work with POSITIVE input level
        //          so maximum search for negative numbers is equivalent to
        //          minimum search for positive numbers!!!!!!
        //          (-30 > -120 but 30 < 120)
        if (max_G_all <= max_G_DTX)
          new_IL = max_G_all;
        else
          new_IL = max_G_DTX;
      }
      else
        new_IL = max_G_all;
    #else
      av_G_all=av_G_DTX=0;

      for (i=0;i<DPAGC_FIFO_LEN;i++)
     av_G_all += tab_ptr[i];

      av_G_all /= DPAGC_FIFO_LEN;

      if (!beacon)
      {
        for (i=0;i<DPAGC_FIFO_LEN;i++)
         av_G_DTX += aset->G_DTX[i];

        av_G_DTX /= DPAGC_FIFO_LEN;

        if (av_G_all >= av_G_DTX)
          new_IL = av_G_all;
        else
          new_IL = av_G_DTX;
      }
      else
        new_IL = av_G_all;
    #endif

    // Updating of input_level and lna_off fields in order to correctly
    // setting the AGC for the next task.
    // input_level is always store with lna_on

    l1ctl_encode_lna( (UWORD8)(current_calibrated_IL>>1),
                      &(IL_info_ptr->lna_off),
                      radio_freq );
    IL_info_ptr->input_level = (UWORD8)new_IL + l1ctl_get_lna_att(radio_freq) *
						IL_info_ptr->lna_off;

    #if L2_L3_SIMUL
      #if (DEBUG_TRACE==BUFFER_TRACE_DPAGC)
        buffer_trace(4,IL_info_ptr->input_level,last_known_agc,
                       l1a_l1s_com.Scell_used_IL_dd.input_level,Cust_get_agc_from_IL(radio_freq, new_IL >> 1, MAX_ID));
      #endif
    #endif

    return((UWORD8)current_calibrated_IL);
  }
#endif  // AMR == 1

/*-------------------------------------------------------*/
/* l1ctl_get_g_magic()                                   */
/*-------------------------------------------------------*/
/* Parameters :                                          */
/* Return     :                                          */
/* Functionality :                                       */
/*-------------------------------------------------------*/
#if (L1_FF_MULTIBAND == 0)
UWORD16 l1ctl_get_g_magic(UWORD16 radio_freq)
{


  if ((l1_config.std.id == DUAL) || (l1_config.std.id == DUALEXT) || (l1_config.std.id == DUAL_US))
  {
    if (radio_freq >= l1_config.std.first_radio_freq_band2)
      return(l1_config.std.g_magic_band2);
    else
      return(l1_config.std.g_magic_band1);
  }
  else
    return(l1_config.std.g_magic_band1);
  

}
#endif

/*-------------------------------------------------------*/
/* l1ctl_get_lna_att()                                   */
/*-------------------------------------------------------*/
/* Parameters :                                          */
/* Return     :                                          */
/* Functionality :                                       */
/*-------------------------------------------------------*/
#if (L1_FF_MULTIBAND == 0)
UWORD16 l1ctl_get_lna_att(UWORD16 radio_freq)
{


  if ((l1_config.std.id == DUAL) ||  (l1_config.std.id == DUALEXT) || (l1_config.std.id == DUAL_US))
  {
    if (radio_freq >= l1_config.std.first_radio_freq_band2)
      return(l1_config.std.lna_att_band2);
    else
      return(l1_config.std.lna_att_band1);
  }
  else
    return(l1_config.std.lna_att_band1);
  
  
}
#endif 
/*-------------------------------------------------------*/
/* l1ctl_update_TPU_with_toa()                           */
/*-------------------------------------------------------*/
/* Parameters :                                          */
/* Return     :                                          */
/* Functionality :                                       */
/*-------------------------------------------------------*/
void l1ctl_update_TPU_with_toa(void)
{
  #if (TOA_ALGO != 0)
    WORD16 toa_shift;

    #if (TOA_ALGO == 2)
      toa_shift = l1s.toa_var.toa_shift;
    #else
      toa_shift = l1s.toa_shift;
    #endif

    if (toa_shift != ISH_INVALID)
    // New ISH (TOA shift) has been stored in "l1s.toa_shift".
    {
      // NEW !!! For EOTD measurements in IDLE mode, cut AFC updates...
      #if (L1_EOTD==1)
        #if (L1_GPRS)
          if ( (l1a_l1s_com.nsync.eotd_meas_session == FALSE) ||
               (l1a_l1s_com.mode == DEDIC_MODE)||
               (l1a_l1s_com.l1s_en_task[PDTCH] == TASK_ENABLED))
        #else
          if ( (l1a_l1s_com.nsync.eotd_meas_session == FALSE) ||
               (l1a_l1s_com.mode == DEDIC_MODE))
        #endif
          {
            // In dedicated or transfer modes we need to track an TOA
            // updates to post correct th results, else E-OTD implementation
            // has qb errors...

            if( (l1a_l1s_com.nsync.eotd_meas_session == TRUE)
                && (l1a_l1s_com.nsync.eotd_toa_phase == 1) )
            {
              l1a_l1s_com.nsync.eotd_toa_tracking += toa_shift;
            }
      #endif
            // Update tpu offset.
            l1s.tpu_offset = (l1s.tpu_offset + TPU_CLOCK_RANGE + toa_shift) % TPU_CLOCK_RANGE;

            #if (TRACE_TYPE==1) || (TRACE_TYPE==4)
              #if (GSM_IDLE_RAM == 0)
                l1_trace_new_toa();
              #else
                l1_trace_new_toa_intram();
              #endif
            #endif

      #if (L1_EOTD==1)
          }
      #endif

      #if (TRACE_TYPE == 5)
        #if (TOA_ALGO == 2)
          trace_toa_sim_update (toa_shift,l1s.tpu_offset);
        #endif
      #endif

      // Reset ISH.
      #if (TOA_ALGO == 2)
        l1s.toa_var.toa_shift = ISH_INVALID;   // Reset the ISH.
      #else
        l1s.toa_shift = ISH_INVALID;   // Reset the ISH.
      #endif
    }
  #endif
}


/*-------------------------------------------------------*/
/* l1ctl_saic()                                          */
/*-------------------------------------------------------*/
/* Parameters :                                          */
/* Return     :                                          */
/* Functionality :                                       */
/*-------------------------------------------------------*/

#if (L1_SAIC != 0)
#define SWH_CHANTAP_INIT 0xFFD068CE
#if (NEW_SNR_THRESHOLD == 1)
UWORD8 l1ctl_saic (UWORD8 IL_for_rxlev, UWORD32 l1_mode, UWORD8 task, UWORD8 * saic_flag)
#else
UWORD8 l1ctl_saic (UWORD8 IL_for_rxlev, UWORD32 l1_mode)
#endif   /* NEW_SNR_THRESHOLD */
{
  UWORD16 SWH_flag = 0;
  UWORD8  CSF_Filter_choice = L1_SAIC_HARDWARE_FILTER;
#if (NEW_SNR_THRESHOLD == 0)
  volatile UWORD16 *ptr;
  UWORD8 saic_flag;
#endif /* NEW_SNR_THRESHOLD */
#if (NEW_SNR_THRESHOLD == 0)
  ptr = (volatile UWORD16 * ) (SWH_CHANTAP_INIT);
  *ptr = 0;
  saic_flag=1;
#else
  *saic_flag=0;
#endif

  switch (l1_mode)
  {
    case DEDIC_MODE:  // GSM DEDICATED MODE
    {
#if (NEW_SNR_THRESHOLD == 1)
      *saic_flag=1;
#endif
      if(IL_for_rxlev < L1_SAIC_GENIE_GSM_DEDIC_THRESHOLD)
      {
        SWH_flag=1;
      }

      break;
    }
    #if L1_GPRS
      case PACKET_TRANSFER_MODE: // PACKET TRANSFER MODE
      {
#if (NEW_SNR_THRESHOLD == 0)
        #if (L1_SAIC == 1)
          if(IL_for_rxlev < L1_SAIC_GENIE_GPRS_PCKT_TRAN_THRESHOLD)
          {
             *ptr = 4;
          }
        #endif /*#if (L1_SAIC == 3)*/
#endif

        #if (L1_SAIC == 3)
          if(IL_for_rxlev < L1_SAIC_GENIE_GPRS_PCKT_TRAN_THRESHOLD)
          {
            SWH_flag = 1;
          }
        #endif /*#if (L1_SAIC == 3)*/
        break;
      }
   #endif /*#if L1_GPRS*/
   default: /* GSM OR GPRS IDLE MODES */
   {
     #if ((L1_SAIC == 2)||(L1_SAIC == 3))
       if(IL_for_rxlev < L1_SAIC_GENIE_GSM_GPRS_IDLE_THRESHOLD)
       {
         SWH_flag=1;
       }
     #endif
     break;
   }
  }

  l1ddsp_load_swh_flag (SWH_flag ,
#if (NEW_SNR_THRESHOLD == 0)
		  saic_flag
#else
		  *saic_flag
#endif
		  );

  if(SWH_flag == 1)
  {
    CSF_Filter_choice = L1_SAIC_PROGRAMMABLE_FILTER;
  }


  #if (TRACE_TYPE == 1) || (TRACE_TYPE == 4)
    l1_trace_saic(SWH_flag,
#if (NEW_SNR_THRESHOLD == 0)
		  saic_flag
#else
		    *saic_flag
#endif
		    );
  #endif
  #if (TRACE_TYPE == 5)
    trace_saic_sim(IL_for_rxlev, l1_mode, SWH_flag);
  #endif

  return(CSF_Filter_choice);
}
#endif

#if (FF_L1_FAST_DECODING == 1)
/*-----------------------------------------------------------------*/
/* l1ctl_pagc_missing_bursts                                       */
/*-----------------------------------------------------------------*/
/*                                                                 */
/* Description:                                                    */
/* ------------                                                    */
/* When fast decoding is active, fewer bursts are decoded. As a    */
/* result, fewer gain values are available. The PAGC algo must     */
/* be updated with the missed values.                              */
/*                                                                 */
/* Input parameters:                                               */
/* -----------------                                               */
/* UWORD8 skipped_values: the number of skipped bursts due to fast */
/*                        decoding.                                */
/*                                                                 */
/* Input parameters from globals:                                  */
/* ------------------------------                                  */
/* l1a_l1s_com.Scell_info.buff_beacon: Input Level (IL) FIFO       */
/* l1_config.params.il_min: minimum level                          */
/*                                                                 */
/* Output parameters:                                              */
/* ------------------                                              */
/* none                                                            */
/*                                                                 */
/* Modified parameters from globals:                               */
/* ---------------------------------                               */
/* l1a_l1s_com.Scell_info.buff_beacon: Input Level (IL) FIFO       */
/*                                                                 */
/*-----------------------------------------------------------------*/

void l1ctl_pagc_missing_bursts (UWORD8 skipped_values)
{
  UWORD8 i = 0;

  /* skipped_values cannot be greater than 3, otherwise this is an error
   * and the PAGC algorithm mustn't be updated. */
  if (skipped_values > 3)
  {
    return;
  }

  /* Update fifo by removing skipped_values of samples */
  for (i = 3; i > (skipped_values - 1); i--)
  {
    l1a_l1s_com.Scell_info.buff_beacon[i] = l1a_l1s_com.Scell_info.buff_beacon[i-skipped_values];
  }

  /* Insert minimum IL level as many times a burst has been skipped */
  for (i = 0; i < skipped_values; i++)
  {
    l1a_l1s_com.Scell_info.buff_beacon[i] = l1_config.params.il_min;
  }
}
#endif /* #if (FF_L1_FAST_DECODING == 1)  */
