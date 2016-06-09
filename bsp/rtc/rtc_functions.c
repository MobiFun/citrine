/******************************************************************************/
/* rtc_functions.c :            contains low level function for the rtc       */
/*                                                                            */
/*                                                                            */
/*   Author: Laurent Sollier                                                  */
/*                                                                            */
/*   version: 1.0                                                             */
/*                                                                            */
/*   Date: 03/20/01                                                           */
/******************************************************************************/

#include "../../include/config.h"

#include <string.h> /* needed for memset */

#include "rtc_messages_i.h"
#include "rtc_api.h"
#include "rtc_i.h"
#include "rtc_config.h" 

#include "../../riviera/rvm/rvm_use_id_list.h"

#include "../mem.h"
#include "../iq.h"
#include "../ulpd.h"
#include "../inth.h"

#include "../../nucleus/nucleus.h" 

/* ----- Macro ----- */
/*-------------------------------------------------------------*/
/* RTC_STOP                                                    */
/*-------------------------------------------------------------*/
/* Parameters :   none                                         */
/* Return :   none                                             */
/* Functionality : Stop the RTC (STOP_RTC is cleared)          */
/*-------------------------------------------------------------*/
#define RTC_STOP  (* (volatile UINT8 *) RTC_CTRL_REG &= ~RTC_START_RTC)

/*-------------------------------------------------------------*/
/* RTC_START                                                   */
/*-------------------------------------------------------------*/
/* Parameters :   none                                         */
/* Return :   none                                             */
/* Functionality : Start the RTC                               */
/*-------------------------------------------------------------*/
#define RTC_START (* (volatile UINT8 *) RTC_CTRL_REG |= RTC_START_RTC) 

/*-------------------------------------------------------------*/
/* RTC_SET_32_CNT                                              */
/*-------------------------------------------------------------*/
/* Parameters :   none                                         */
/* Return :   none                                             */
/* Functionality : Set the SET_32_COUNTER bit                  */
/*-------------------------------------------------------------*/
#define RTC_SET_32_CNT (* (volatile UINT8 *) RTC_CTRL_REG |= RTC_SET_32_COUNTER)

/*-------------------------------------------------------------*/
/* RTC_RESET_32_CNT                                            */
/*-------------------------------------------------------------*/
/* Parameters :   none                                         */
/* Return :   none                                             */
/* Functionality : Reset the SET_32_COUNTER bit                */
/*-------------------------------------------------------------*/
#define RTC_RESET_32_CNT  (* (volatile UINT8 *) RTC_CTRL_REG &= ~RTC_SET_32_COUNTER)

/*-------------------------------------------------------------*/
/* RTC_SET_TEST_MODE                                           */
/*-------------------------------------------------------------*/
/* Parameters :   none                                         */
/* Return :   none                                             */
/* Functionality : Set the TEST_MODE bit                       */
/*-------------------------------------------------------------*/
#define RTC_SET_TEST_MODE (* (volatile UINT8 *) RTC_CTRL_REG |= RTC_TEST_MODE)

/*-------------------------------------------------------------*/
/* RTC_ENABLE_IT_ALARM                                         */
/*-------------------------------------------------------------*/
/* Parameters :   none                                         */
/* Return :   none                                             */
/* Functionality : Enable the IT_ALARM                         */
/*-------------------------------------------------------------*/
#define RTC_ENABLE_IT_ALARM (* (volatile UINT8 *) RTC_INTERRUPTS_REG |= RTC_IT_ALARM)

/*-------------------------------------------------------------*/
/* RTC_DISABLE_IT_ALARM                                        */
/*-------------------------------------------------------------*/
/* Parameters :   none                                         */
/* Return :   none                                             */
/* Functionality : Disable the IT_ALARM                        */
/*-------------------------------------------------------------*/
#define RTC_DISABLE_IT_ALARM (* (volatile UINT8 *) RTC_INTERRUPTS_REG &= ~RTC_IT_ALARM)

/*-------------------------------------------------------------*/
/* RTC_ENABLE_IT_TIMER                                         */
/*-------------------------------------------------------------*/
/* Parameters :   none                                         */
/* Return :   none                                             */
/* Functionality : Enable the IT_TIMER                         */
/*-------------------------------------------------------------*/
#define RTC_ENABLE_IT_TIMER (* (volatile UINT8 *) RTC_INTERRUPTS_REG |= RTC_IT_TIMER)

/*-------------------------------------------------------------*/
/* RTC_DISABLE_IT_TIMER                                        */
/*-------------------------------------------------------------*/
/* Parameters :   none                                         */
/* Return :   none                                             */
/* Functionality : Disable the IT_TIMER                        */
/*-------------------------------------------------------------*/
#define RTC_DISABLE_IT_TIMER (* (volatile UINT8 *) RTC_INTERRUPTS_REG &= ~RTC_IT_TIMER)

/*-------------------------------------------------------------*/
/* RTC_SET_EVENT_TIMER                                         */
/*-------------------------------------------------------------*/
/* Parameters :   event to enable or disable                   */
/* Return :   none                                             */
/* Functionality : Set the event for the IT_TIMER              */
/*         Enable or disable IT                                */
/*-------------------------------------------------------------*/
#define RTC_SET_EVENT_TIMER(x) (* (volatile UINT8 *) RTC_INTERRUPTS_REG = (* (volatile UINT8 *) RTC_INTERRUPTS_REG & 0xfffc) | (x))

/*-------------------------------------------------------------*/
/* RTC_TEST_BUSY                                               */
/*-------------------------------------------------------------*/
/* Parameters :   none                                         */
/* Return :   none                                             */
/* Functionality : Return the RTC state busy signal            */
/*-------------------------------------------------------------*/
#define RTC_TEST_BUSY ((* (volatile UINT8 *) RTC_STATUS_REG) & RTC_BUSY) 

/*-------------------------------------------------------------*/
/* RTC_RUN_STATE                                               */
/*-------------------------------------------------------------*/
/* Parameters :   none                                         */
/* Return :   none                                             */
/* Functionality : Return the RTC run state                    */
/*-------------------------------------------------------------*/
#define RTC_RUN_STATE ((* (volatile UINT8 *) RTC_STATUS_REG) & RTC_RUN) 

/*-------------------------------------------------------------*/
/* RTC_FREE_AL_ITLINE                                          */
/*-------------------------------------------------------------*/
/* Parameters :   none                                         */
/* Return :   none                                             */
/* Functionality : free it AL line                             */
/*-------------------------------------------------------------*/

#define RTC_FREE_AL_ITLINE ((* (volatile UINT8 *) RTC_STATUS_REG) |= RTC_ALARM) 

/*-------------------------------------------------------------*/
/* RTC_FREE_POWER_UP                                           */
/*-------------------------------------------------------------*/
/* Parameters :   none                                         */
/* Return :   none                                             */
/* Functionality : clear reset event                           */
/*-------------------------------------------------------------*/

#define RTC_FREE_POWER_UP ((* (volatile UINT8 *) RTC_STATUS_REG) |= RTC_POWER_UP) 


/*-------------------------------------------------------------*/
/* RTC_GET_EVENT_TIMER                                         */
/*-------------------------------------------------------------*/
/* Parameters :   none                                         */
/* Return :   none                                             */
/* Functionality : Get the event status for the IT_TIMER       */
/*-------------------------------------------------------------*/
#define RTC_GET_EVENT_TIMER ((* (volatile UINT8 *) RTC_STATUS_REG & 0x3c))




/* ----- Global variables ----- */
static T_RTC_DATE_TIME rtc_date_time_alarm = {0};

static T_RV_RETURN rtc_return_path = {RVF_INVALID_ADDR_ID, NULL};
static T_RTC_ALARM* rtc_msg_alarm_event = NULL;

extern T_RTC_ENV_CTRL_BLK* rtc_env_ctrl_blk;

#if CONFIG_GSM
/* Number of 32 Khz clock */
static UINT32 rtc_nb_32khz = 0;

/* Number of high frequency clock */
static UINT32 rtc_nb_hf = 0;
#endif

static NU_HISR hisr;
char hisrStack[512];

// UINT16 toto = 0; tmp

void HisrEntryFunc(void)
{
   if (rtc_msg_alarm_event)
      rvf_send_msg(rtc_env_ctrl_blk->addr_id, rtc_msg_alarm_event);
   //char comp[15];
/*   sprintf(comp,"%d",toto);
   LCDG_WriteString( 4, 0, "              ");
   LCDG_WriteString( 3, 0, (char*) comp); //tmp */
}


/*---------------------------------------------------------------*/
/* conv_bin   DCB => B                                           */
/*---------------------------------------------------------------*/
/* Parameters : value to be convert                              */
/* Return :   none                                               */
/* Functionality : BCD et binary conversion function             */
/*---------------------------------------------------------------*/

static UINT8 conv_bin(UINT8 value)
{
   return (10*(value>>4) + ( value & 0x0f));
}

/*---------------------------------------------------------------*/
/* conv_bcd    B => DCB                                          */
/*---------------------------------------------------------------*/
/* Parameters : value to be convert                              */
/* Return :   none                                               */
/* Functionality : Binary to BCD conversion function             */
/*---------------------------------------------------------------*/

static UINT8 conv_bcd(UINT8 value)
{
   return ((value%10) | ( (value/10) <<4));
}



/*---------------------------------------------------------------*/
/* format_date_available                                         */
/*---------------------------------------------------------------*/
/* Parameters : T_RTC_DATE_TIME structure                        */
/* Return :     TRUE if format is available                      */
/*              FALSE else                                       */
/* Functionality : test if date and time format is available     */
/*---------------------------------------------------------------*/

BOOL format_date_available(T_RTC_DATE_TIME date_time)
{
   UINT8 m;

   if (date_time.second < 0 || date_time.second > 59)
      return FALSE;
   if (date_time.minute < 0 || date_time.minute > 59)
      return FALSE;
   if (date_time.mode_12_hour == FALSE)
   {
      if (date_time.hour < 0 || date_time.hour > 23)
         return FALSE;
   }
   else
      if (date_time.hour < 1 || date_time.hour > 12)
         return FALSE;

   if (date_time.month < 1 || date_time.month > 12)
      return FALSE;
   if (date_time.year < 0 || date_time.year > 99)
      return FALSE;
   if (date_time.wday < 0 || date_time.wday > 6)
      return FALSE;
   m = date_time.month;
   if (m == 1||m == 3||m == 5||m == 7||m == 8||m == 10||m == 12)
   {
      if (date_time.day < 1 || date_time.day > 31)
         return FALSE;
   }
   else
   {
      if (m == 4||m == 6||m == 9||m == 11)
      {
         if (date_time.day < 1 || date_time.day > 30)
            return FALSE;
      }
      else
      {
         if (date_time.year%4)
         {
            if (date_time.day < 1 || date_time.day > 28)
               return FALSE;
         }
         else
         {
            if (date_time.day < 1 || date_time.day > 29)
               return FALSE;
         }
      }
   }
   return TRUE;
}



/*******************************************************************************
 *
 *                               RTC_Initialize
 *
 ******************************************************************************/



T_RVF_RET RTC_Initialize(void)
{
   T_RVF_MB_STATUS   mb_status;

   /* Reserve memory for alarm event */
   mb_status = rvf_get_buf (rtc_env_ctrl_blk->prim_id, sizeof (T_RTC_ALARM ), (void **) &rtc_msg_alarm_event);   

   if ((mb_status == RVF_GREEN) || (mb_status == RVF_YELLOW)) /* Memory allocation success */
   {      
      rtc_msg_alarm_event->os_hdr.msg_id = RTC_ALARM_EVT;
   }
   else
   {
      rtc_msg_alarm_event = NULL;
      return RVF_MEMORY_ERR;
   }

   rtc_env_ctrl_blk->msg_alarm_event = rtc_msg_alarm_event;


   /* Start RTC module */
   if (RTC_RUN_STATE == 0)
      RTC_START;

   /* Enable auto compensation */
    //*(volatile UINT8*) RTC_CTRL_REG |= RTC_AUTO_COMP;
   /* Disable auto compensation */
    *(volatile UINT8*) RTC_CTRL_REG &= ~RTC_AUTO_COMP;

    /* For CHIPSET = 7, 9, 10 or 11, set analog baseband type */
#if (((CHIPSET == 7) || (CHIPSET == 8) || (CHIPSET == 10) || (CHIPSET == 11)) && (ANALOG == 1))
   *(volatile UINT8*) RTC_CTRL_REG |= RTC_nDELTA_OMEGA;
#endif

   /* Activate periodic interrupt every minute*/
   /* Disable all IT before accessing register */
   rvf_disable(25);

   /* Enable IRQ9 and IRQ10 */
   #if (CHIPSET == 12)
     F_INTH_ENABLE_ONE_IT(C_INTH_RTC_ALARM_IT);
   #else
     IQ_Unmask(IQ_RTC_ALARM);
   #endif

   /*
    * FreeCalypso change: the RTC periodic interrupt is used in
    * conjunction with the ULPD gauging mechanism, and the latter
    * appears to be controlled by some code in L1-land.
    * Therefore, we are going to enable this interrupt only when
    * building with L1 enabled.
    */
#if CONFIG_GSM
   #if (CHIPSET == 12)
     F_INTH_ENABLE_ONE_IT(C_INTH_RTC_TIMER_IT);
   #else
     IQ_Unmask(IQ_RTC_TIMER);
   #endif
#endif

   while (RTC_TEST_BUSY);
   RTC_SET_EVENT_TIMER(RTC_EVERY_MIN); /* Set timer every minute */
#if CONFIG_GSM
   RTC_ENABLE_IT_TIMER;
#else
   RTC_DISABLE_IT_TIMER;
#endif
   RTC_DISABLE_IT_ALARM;

   rvf_enable();

   /* The stack is entirely filled with the pattern 0xFE. */
   memset (hisrStack, 0xFE, sizeof(hisrStack));

   /* Temporary modification: create HISR to diplay compensation value */
   NU_Create_HISR(&hisr, "RTC_HISR", HisrEntryFunc, 2, hisrStack, sizeof(hisrStack)); // lowest prty
   /* end temporary modification */

   return RVF_OK;
}
/*******************************************************************************
 *
 *                               RTC_RtcReset
 *
 ******************************************************************************/

BOOL RTC_RtcReset(void)
{

   /* Read POWER UP bit to inform MMI of a possible RTC reset */
   if ( ((* (volatile UINT8*) RTC_STATUS_REG) & RTC_POWER_UP) )
   {
      RTC_FREE_POWER_UP;
      return TRUE;
   }
   else
      return FALSE;

}


/*******************************************************************************
 *
 *                               RTC_GetDateTime
 *
 ******************************************************************************/

T_RVF_RET RTC_GetDateTime(T_RTC_DATE_TIME* date_time)
{
   UINT8 sec;
   UINT8 min;
   UINT8 hour;
   UINT8 day;
   UINT8 month;
   UINT8 year;
   UINT8 wday;
   UINT8 hr_reg;

   /* Disable all IT before reading register */
   rvf_disable(25);

   if (RTC_TEST_BUSY)
   {   
      rvf_enable();
      return RVF_NOT_READY;
   }
   else
   {
      day  = * (volatile UINT8 *) RTC_DAYS_REG;
      month  = * (volatile UINT8 *) RTC_MONTHS_REG;
      year = * (volatile UINT8 *) RTC_YEARS_REG;
      wday = * (volatile UINT8 *) RTC_WEEK_REG;   
      sec  = * (volatile UINT8 *) RTC_SECONDS_REG;
      min  = * (volatile UINT8 *) RTC_MINUTES_REG;
      hr_reg = * (volatile UINT8 *) RTC_HOURS_REG;
      rvf_enable();
   }

   hour = (0x7f & hr_reg);

   date_time->second = conv_bin(sec);
   date_time->minute = conv_bin(min);
   date_time->hour = conv_bin(hour);

   if ( (* (volatile UINT8 *)RTC_CTRL_REG & RTC_MODE_12_24 ) == 0)
   {
      /* 24 hour mode */
      date_time->mode_12_hour = FALSE;
      date_time->PM_flag = FALSE;
   }
   else
   {
      /* 12 hour mode */
      date_time->mode_12_hour = TRUE;
      if ((hr_reg & 0x80) == 0)
         date_time->PM_flag = FALSE;
      else
         date_time->PM_flag = TRUE;
   }

   date_time->day = conv_bin(day);
   date_time->month = conv_bin(month);
   date_time->year = conv_bin(year);
   date_time->wday = conv_bin(wday);

   return RVF_OK;
}


/*******************************************************************************
 *
 *                               RTC_SetDateTime
 *
 ******************************************************************************/

T_RVF_RET RTC_SetDateTime(T_RTC_DATE_TIME date_time)
{
   UINT8 sec;
   UINT8 min;
   UINT8 hour;
   UINT8 day;
   UINT8 month;
   UINT8 year;
   UINT8 wday;

   /* testing parameter range validity */
   if (!format_date_available(date_time))
      return RVF_INVALID_PARAMETER;


   sec = conv_bcd(date_time.second);
   min = conv_bcd(date_time.minute);
   if (date_time.mode_12_hour == FALSE)
   {
      * (volatile UINT8 *)RTC_CTRL_REG &= ~RTC_MODE_12_24;
      hour = conv_bcd(date_time.hour);
   }
   else
   {
      * (volatile UINT8 *)RTC_CTRL_REG |= RTC_MODE_12_24;
      hour = conv_bcd(date_time.hour);
      if (date_time.PM_flag != FALSE)
         hour |= (0x80);
   }

   day = conv_bcd(date_time.day);
   month = conv_bcd(date_time.month);
   year = conv_bcd(date_time.year);
   wday = conv_bcd(date_time.wday);

   /* Disable all IT before reading register */
   rvf_disable(25);

   if (RTC_TEST_BUSY)
   {   
      rvf_enable();
      return RVF_NOT_READY;
   }
   else
   {
      * (volatile UINT8 *) RTC_DAYS_REG = day;
      * (volatile UINT8 *) RTC_MONTHS_REG = month;
      * (volatile UINT8 *) RTC_YEARS_REG = year;
      * (volatile UINT8 *) RTC_WEEK_REG = wday;   
      * (volatile UINT8 *) RTC_SECONDS_REG = sec;
      * (volatile UINT8 *) RTC_MINUTES_REG = min;
      * (volatile UINT8 *) RTC_HOURS_REG = hour;

      rvf_enable();
   }

   return RVF_OK;
}


/*******************************************************************************
 *
 *                               RTC_GetAlarm
 *
 ******************************************************************************/

T_RVF_RET RTC_GetAlarm(T_RTC_DATE_TIME* date_time)
{
   date_time->second = rtc_date_time_alarm.second;
   date_time->minute = rtc_date_time_alarm.minute;
   date_time->hour = rtc_date_time_alarm.hour;
   date_time->day = rtc_date_time_alarm.day;
   date_time->month = rtc_date_time_alarm.month;
   date_time->year = rtc_date_time_alarm.year;
   date_time->wday = rtc_date_time_alarm.wday;
   date_time->mode_12_hour = rtc_date_time_alarm.mode_12_hour;
   date_time->PM_flag = rtc_date_time_alarm.PM_flag;
   return RVF_OK;
}


/*******************************************************************************
 *
 *                               RTC_SetAlarm
 *
 ******************************************************************************/

T_RVF_RET RTC_SetAlarm(T_RTC_DATE_TIME date_time, T_RV_RETURN return_path)
{
   UINT8 sec;
   UINT8 min;
   UINT8 hour;
   UINT8 day;
   UINT8 month;
   UINT8 year;
   UINT8 wday;


   /* testing parameter range validity */
   if (!format_date_available(date_time))
      return RVF_INVALID_PARAMETER;

   sec = conv_bcd(date_time.second);
   min = conv_bcd(date_time.minute);

   if (date_time.mode_12_hour == FALSE)
      hour = conv_bcd(date_time.hour);
   else
      hour = conv_bcd(date_time.hour) | (date_time.PM_flag << 7);

   day = conv_bcd(date_time.day);
   month = conv_bcd(date_time.month);
   year = conv_bcd(date_time.year);
   wday = conv_bcd(date_time.wday);


   /* Disable all IT before reading register */
   rvf_disable(25);

   if (RTC_TEST_BUSY)
   {   
      rvf_enable();
      return RVF_NOT_READY;
   }
   else
   {
      *(volatile UINT8*) RTC_ALARM_DAYS_REG = day;
      *(volatile UINT8*) RTC_ALARM_MONTHS_REG = month;
      *(volatile UINT8*) RTC_ALARM_YEARS_REG = year;   
      *(volatile UINT8*) RTC_ALARM_SECONDS_REG = sec;
      *(volatile UINT8*) RTC_ALARM_MINUTES_REG = min;
      *(volatile UINT8*) RTC_ALARM_HOURS_REG = hour;

      /* Enable alarm*/
      RTC_ENABLE_IT_ALARM;
      rvf_enable();

      /* Save callback */
      rtc_return_path.callback_func = return_path.callback_func;
      rtc_return_path.addr_id        = return_path.addr_id;
   }
   return RVF_OK;
}


/*******************************************************************************
 *
 *                               RTC_UnsetAlarm
 *
 ******************************************************************************/

T_RVF_RET RTC_UnsetAlarm(void)
{
   /* Disable all IT before reading register */
   rvf_disable(25);

   if (RTC_TEST_BUSY)
   {   
      rvf_enable();
      return RVF_NOT_READY;
   }
   else
   {
      /* Disable alarm*/
      RTC_DISABLE_IT_ALARM;
      rvf_enable();
   }
   return RVF_OK;
}


/*******************************************************************************
 *
 *                               RTC_Rounding30s
 *
 ******************************************************************************/

void RTC_Rounding30s(void)
{
   *(UINT8*) RTC_CTRL_REG |= RTC_ROUND_30S;
}


/*******************************************************************************
 *
 *                               RTC_Set12HourMode
 *
 ******************************************************************************/

void RTC_Set12HourMode(BOOL Mode12Hour)
{
  if ( Mode12Hour == FALSE)
   * (volatile UINT8*) RTC_CTRL_REG &= 0xf7;
  else
   * (volatile UINT8*) RTC_CTRL_REG |= 0x08;

}


/*******************************************************************************
 *
 *                               RTC_Is12HourMode
 *
 ******************************************************************************/

BOOL RTC_Is12HourMode(void)
{
   if ( (*(volatile UINT8*) RTC_CTRL_REG & RTC_MODE_12_24) )
      return TRUE;
   else
      return FALSE;
}

#if CONFIG_GSM
/*******************************************************************************
 *
 *                               RTC_ItTimerHandle
 *
 ******************************************************************************/

void RTC_ItTimerHandle(void)
{
   static double compensation = 0;
   static UINT8 nb_sample = 0;
   double delta;
   UINT16 value = 0;
   INT16 tmp_value = 0;
   UINT8 cmp[15];

   /* Evaluate average on one hour max */
   if ( nb_sample < 60)
      nb_sample++;

   /* perform calculation of auto compensation each minute and evaluate an
      average on one hour */
   /* Number of 32 kHz clock lost in one second */
   /* Accurate operation is : delta = CLK_32 - rtc_nb_32khz*CLK_PLL/g_nb_hf */
   /* with CLK_32 = 32768 Hz and CLK_PLL depend on chipset */
   delta = RTC_CLOCK_32K - rtc_nb_32khz*RTC_CLOCK_HF/rtc_nb_hf;

   /* Number of 32 kHz clock lost in one hour */
   delta *= 3600.0;

   /* Average of the compensation to load */
   compensation = (compensation*(nb_sample-1) + delta)/nb_sample;

   if (compensation >= 0x7FFF)
      tmp_value = 0x7FFE;
   else if (compensation <= -0x7FFF)
      tmp_value = -0x7FFE;
   else
      tmp_value = (INT16) compensation;

   if (tmp_value > 0) /* if 32 Khz clock is slow */
      value = tmp_value;
   if (tmp_value < 0) /* if 32 Khz clock is fast */
      value = 0xFFFF + tmp_value + 1;

   /* Set value in compensation register */
   if (!RTC_TEST_BUSY)
   {
      *(volatile UINT8*) RTC_COMP_MSB_REG = (UINT8) (value >> 8);
      *(volatile UINT8*) RTC_COMP_LSB_REG = (UINT8) (value & 0xFF);
   }
/*toto = value; tmp*/
/*NU_Activate_HISR(&hisr); tmp*/

}
#endif

/*******************************************************************************
 *
 *                               RTC_ItAlarmHandle
 *
 ******************************************************************************/

void RTC_ItAlarmHandle(void)
{
   /* Sending alarm event */
   /* Post alarm event in RTC mailbox */
   NU_Activate_HISR(&hisr);

   /*if (rtc_msg_alarm_event)
      rvf_send_msg(rtc_env_ctrl_blk->task_id, rtc_env_ctrl_blk->mbox_id, rtc_msg_alarm_event);*/

   /* Free alarm IT line */
   RTC_FREE_AL_ITLINE;
}

#if CONFIG_GSM
/*******************************************************************************
 *
 *                               RTC_GaugingHandler
 *
 ******************************************************************************/
void RTC_GaugingHandler(void)
{
   /* Number of 32 Khz clock at the end of the gauging */
   rtc_nb_32khz =  ((*(volatile UINT16 *)ULDP_COUNTER_32_MSB_REG) * 0x10000) +
             (*(volatile UINT16 *)ULDP_COUNTER_32_LSB_REG);     

   /* Number of high frequency clock at the end of the gauging */
   /* To convert in nbr of 13 Mhz clocks (5*13=65Mhz) */
   rtc_nb_hf =   ( ((*(volatile UINT16 *)ULDP_COUNTER_HI_FREQ_MSB_REG) * 0x10000) +
            (*(volatile UINT16 *)ULDP_COUNTER_HI_FREQ_LSB_REG) );
}
#endif

/*******************************************************************************
 *
 *                               RTC_ProcessAlarmEvent
 *
 ******************************************************************************/

void RTC_ProcessAlarmEvent(void)
{
   T_RVF_MB_STATUS mb_status;
   T_RTC_ALARM* msg_alarm;

   /* Call MMI */
   if (rtc_return_path.callback_func != NULL)
   {
      rtc_return_path.callback_func(NULL);
   }
   else
   {
      /* Reserve memory for alarm event */
      mb_status = rvf_get_buf (rtc_env_ctrl_blk->prim_id, sizeof (T_RTC_ALARM ), (void **) &msg_alarm);   

      if ((mb_status == RVF_GREEN) || (mb_status == RVF_YELLOW)) /* Memory allocation success */
      {      
         msg_alarm->os_hdr.msg_id = RTC_ALARM_EVT;
         /* Send event in the mailbox */
         rvf_send_msg(rtc_return_path.addr_id, msg_alarm);
      }
      else
      {
         rvf_send_trace("Memory allocation error",23, NULL_PARAM, RV_TRACE_LEVEL_ERROR, RTC_USE_ID );
      }
   }
}
