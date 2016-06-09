#include "../include/config.h"
#include "../include/sys_types.h"

#include "serialswitch.h" 

#include <string.h>

/*
 * Serial Configuration set up.
 */

/*
** One config is:
** {XXX_BT_HCI,         // Bluetooth HCI
**  XXX_FAX_DATA,       // Fax/Data AT-Cmd
**  XXX_TRACE,          // L1/Riviera Trace Mux
**  XXX_TRACE},         // Trace PS
**
** with XXX being DUMMY, UART_IRDA or UART_MODEM
*/

const T_AppliSerialInfo appli_ser_cfg_info = {
	/*
	 * Default configuration: set it to the "standard"
	 * 0x0168, unless the RVTMUX serial channel has been
	 * moved to the MODEM UART.
	 */
	#if CONFIG_RVTMUX_ON_MODEM
              {DUMMY_BT_HCI,
               DUMMY_FAX_DATA,
               UART_MODEM_TRACE,
               DUMMY_TRACE},       // 0x0248
	#else
              {DUMMY_BT_HCI,
               UART_MODEM_FAX_DATA,
               UART_IRDA_TRACE,
               DUMMY_TRACE},    // default config = 0x0168
	#endif
	/* number of possible configs */
	#ifdef BTEMOBILE
             12,	// 12 serial config allowed
	#else // BTEMOBILE
             9,	// 9 serial config allowed
	#endif
             {
              // Configs with Condat Panel only
              {DUMMY_BT_HCI,
               DUMMY_FAX_DATA,
               DUMMY_TRACE,
               UART_IRDA_TRACE},   // 0x1048
              {DUMMY_BT_HCI,
               DUMMY_FAX_DATA,
               DUMMY_TRACE,
               UART_MODEM_TRACE},  // 0x2048
              // Configs with L1/Riviera Trace only
              {DUMMY_BT_HCI,
               DUMMY_FAX_DATA,
               UART_IRDA_TRACE,
               DUMMY_TRACE},       // 0x0148
              {DUMMY_BT_HCI,
               DUMMY_FAX_DATA,
               UART_MODEM_TRACE,
               DUMMY_TRACE},       // 0x0248
              // Configs with AT-Cmd only
              {DUMMY_BT_HCI,
               UART_MODEM_FAX_DATA,
               DUMMY_TRACE,
               DUMMY_TRACE},       // 0x0068
              // Configs with Condat Panel and L1/Riviera Trace
              {DUMMY_BT_HCI,
               DUMMY_FAX_DATA,
               UART_MODEM_TRACE,
               UART_IRDA_TRACE},	 // 0x1248
              {DUMMY_BT_HCI,
               DUMMY_FAX_DATA,
               UART_IRDA_TRACE,
               UART_MODEM_TRACE},	 // 0x2148
              // Configs with Condat Panel and AT-Cmd
              {DUMMY_BT_HCI,
               UART_MODEM_FAX_DATA,
               DUMMY_TRACE,
               UART_IRDA_TRACE},   // 0x1068
	#ifdef BTEMOBILE
              // Configs with L1/Riviera Trace and Bluetooth HCI
              {UART_IRDA_BT_HCI,
               DUMMY_FAX_DATA,
               UART_MODEM_TRACE,
               DUMMY_TRACE},       // 0x0249
              {UART_MODEM_BT_HCI,
               DUMMY_FAX_DATA,
               UART_IRDA_TRACE,
               DUMMY_TRACE},       // 0x014A
              // Configs with AT-Cmd and Bluetooth HCI
              {UART_IRDA_BT_HCI,
               UART_MODEM_FAX_DATA,
               DUMMY_TRACE,
               DUMMY_TRACE},       // 0x0069
	#endif // BTEMOBILE
              // Configs with L1/Riviera Trace and AT-Cmd
              {DUMMY_BT_HCI,
               UART_MODEM_FAX_DATA,
               UART_IRDA_TRACE,
               DUMMY_TRACE}        // 0x0168
             }
};

/*
 * Init_Serial_Flows
 *
 * Performs Serialswitch + related serial data flows initialization.
 */

void Init_Serial_Flows (void)
{
  #if 1 //(OP_L1_STANDALONE == 0)

    /*
     * Initialize Serial Switch module.
     */
    #if ((BOARD==35) || (BOARD == 46))
      SER_InitSerialConfig (GC_GetSerialConfig());
    #else
      SER_InitSerialConfig (&appli_ser_cfg_info);
    #endif
    /*
     * Then Initialize the Serial Data Flows and the associated UARTs:
     *  - G2-3 Trace if GSM/GPRS Protocol Stack
     *  - AT-Cmd/Fax & Data Flow
     *
     * Layer1/Riviera Trace Flow and Bluetooth HCI Flow are initialized
     * by the appropriate SW Entities.
     *
     * G2-3 Trace => No more Used
     */
    SER_tr_Init(SER_PROTOCOL_STACK, TR_BAUD_38400, NULL);

    /*
     * Fax & Data / AT-Command Interpreter Serial Data Flow Initialization
     */

    #if ((BOARD != 35) && (BOARD != 46))
      (void) SER_fd_Initialize ();
    #endif
  #else    /* OP_L1_STANDALONE */

    #if (TESTMODE || (TRACE_TYPE==1) || (TRACE_TYPE==2) || (TRACE_TYPE==3) || (TRACE_TYPE==6) || (TRACE_TYPE==7))
      #if ((BOARD == 35) || (BOARD == 46))
        ser_cfg_info[UA_UART_0] = '0';
      #else
        ser_cfg_info[UA_UART_0] = 'G';
      #endif
      #if (CHIPSET !=15)
      ser_cfg_info[UA_UART_1] = 'R'; // Riviear Demux on UART MODEM
      #else
      ser_cfg_info[UA_UART_0] = 'R'; // Riviear Demux on UART MODEM
      #endif

      /* init Uart Modem */
      SER_InitSerialConfig (&appli_ser_cfg_info);

      #if TESTMODE || (TRACE_TYPE == 1) || (TRACE_TYPE == 7)
        SER_tr_Init (SER_LAYER_1, TR_BAUD_115200, rvt_activate_RX_HISR);

        rvt_register_id("OTHER",&trace_id,(RVT_CALLBACK_FUNC)NULL);
      #else
        SER_tr_Init (SER_LAYER_1, TR_BAUD_38400, NULL);
      #endif

      L1_trace_string(" \n\r");

    #endif   /* TRACE_TYPE */

  #endif   /* OP_L1_STANDALONE */
}
