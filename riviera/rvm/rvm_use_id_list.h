/**
 * @file	rvm_use_id_list.h
 *
 * List of common SWE USE IDs.
 *
 * Note on USE ID management:
 * This file should only contain USE ID of SWE part of the standard TI releases.
 * Development SWE as well as customer SWE should use the rvm_ext_use_id_list.h
 * file for their USE IDs.
 *
 * @author	David Lamy-Charrier (d-lamy@ti.com)
 * @version	0.1
 */

/*
 * Revision History:
 *
 *	Date       	Author			Modification
 *	-------------------------------------------------------------------
 *	10/25/2001	David Lamy-Charrier	Create.
 *	11/20/2001	Vincent Oberle		Added BUILD_MESSAGE_OFFSET.
 *
 * (C) Copyright 2001 by Texas Instruments Incorporated, All Rights Reserved
 */


#ifndef __RVM_USE_ID_LIST_H_
#define __RVM_USE_ID_LIST_H_


/**
 * Macro used to build the use_id from the cust, offset and mask parameters.
 *
 * IMPORTANT: The mask parameter must a 16-bit unsigned integer with only one
 * bit at 1 and others at 0.
 * E.g. possible values are 0x0001, 0x0002, 0x0004, 0x0008, 0x0010... 0x8000.
 */
#define BUILD_USE_ID(cust, cluster, mask) ( (cust<<31) | ( (cluster & 0x7FFF) <<16) | (mask & 0xFFFF) )

/**
 * Macro for building the message offset from the USE ID.
 */
#define BUILD_MESSAGE_OFFSET(useid) ( (useid & 0xFFFF0000) +		\
				      ((((useid & 0xAAAA) ? 1 : 0) +	\
					((useid & 0xCCCC) ? 2 : 0) +	\
					((useid & 0xF0F0) ? 4 : 0) +	\
					((useid & 0xFF00) ? 8 : 0)) << 12) )


#define RIVIERA_USE_ID_CLUSTER_1	(1)

#define DRIVERS_USE_ID_CLUSTER_1	(10)

#define BLUETOOTH_USE_ID_CLUSTER_1	(20)

#define SERVICES_USE_ID_CLUSTER_1	(30)
#define SERVICES_USE_ID_CLUSTER_2	(31)

#define TEST_USE_ID_CLUSTER_1		(40)

#define CONDAT_USE_ID_CLUSTER_1		(50)

#define JAVA_USE_ID_CLUSTER_1		(60)

#define TCPIP_USE_ID_CLUSTER_1		(70)

#define OBIGO_USE_ID_CLUSTER_1		(80)
#define OBIGO_USE_ID_CLUSTER_2		(81)

#define WIDCOMM_BT_USE_ID_CLUSTER_1 (90)

/**
 * @name	Widcomm cluster
 *
 * Riviera insfrastructure
 */
/*@{*/


#define BTU_USE_ID  BUILD_USE_ID( 0, WIDCOMM_BT_USE_ID_CLUSTER_1, 0x0002)
#define GKI_USE_ID  BUILD_USE_ID( 0, WIDCOMM_BT_USE_ID_CLUSTER_1, 0x0004)
#define BTH_USE_ID  BUILD_USE_ID( 0, WIDCOMM_BT_USE_ID_CLUSTER_1, 0x0008)
#define BTUI_USE_ID BUILD_USE_ID( 0, WIDCOMM_BT_USE_ID_CLUSTER_1, 0x0010)
#define RPC_USE_ID  BUILD_USE_ID( 0, WIDCOMM_BT_USE_ID_CLUSTER_1, 0x0020)
/**
 * @name	Riviera cluster
 *
 * Riviera insfrastructure
 */
/*@{*/

/* this one is only used for trace purpose */
#define RVM_USE_ID	BUILD_USE_ID( 0, RIVIERA_USE_ID_CLUSTER_1, 0x0001)

#define RVT_USE_ID	BUILD_USE_ID( 0, RIVIERA_USE_ID_CLUSTER_1, 0x0002)

#define TI_PRF_USE_ID	BUILD_USE_ID( 0, RIVIERA_USE_ID_CLUSTER_1, 0x0004)

/*@}*/



/**
 * @name	Driver cluster
 *
 * Drivers
 */
/*@{*/

#define R2D_USE_ID	BUILD_USE_ID( 0, DRIVERS_USE_ID_CLUSTER_1, 0x0001)

#define RTC_USE_ID	BUILD_USE_ID( 0, DRIVERS_USE_ID_CLUSTER_1, 0x0002)

#define FFS_USE_ID	BUILD_USE_ID( 0, DRIVERS_USE_ID_CLUSTER_1, 0x0004)

#define KPD_USE_ID	BUILD_USE_ID( 0, DRIVERS_USE_ID_CLUSTER_1, 0x0008)

#define SPI_USE_ID	BUILD_USE_ID( 0, DRIVERS_USE_ID_CLUSTER_1, 0x0010)

/* Replacing PWR_USE_ID */
#define LCC_USE_ID	BUILD_USE_ID( 0, DRIVERS_USE_ID_CLUSTER_1, 0x0020)	


#define RGUI_USE_ID	BUILD_USE_ID( 0, DRIVERS_USE_ID_CLUSTER_1, 0x0040)

/*@}*/




/**
 * @name	Bluetooth cluster
 *
 * Bluetooth related SWE
 */
/*@{*/

#define HCI_USE_ID	BUILD_USE_ID( 0, BLUETOOTH_USE_ID_CLUSTER_1, 0x0001)

#define L2CAP_USE_ID	BUILD_USE_ID( 0, BLUETOOTH_USE_ID_CLUSTER_1, 0x0002)

#define BTCTRL_USE_ID	BUILD_USE_ID( 0, BLUETOOTH_USE_ID_CLUSTER_1, 0x0004)

#define RFC_USE_ID	BUILD_USE_ID( 0, BLUETOOTH_USE_ID_CLUSTER_1, 0x0008)

#define SPP_USE_ID	BUILD_USE_ID( 0, BLUETOOTH_USE_ID_CLUSTER_1, 0x0010)

#define HS_USE_ID	BUILD_USE_ID( 0, BLUETOOTH_USE_ID_CLUSTER_1, 0x0020)

#define	HSG_USE_ID	BUILD_USE_ID( 0, BLUETOOTH_USE_ID_CLUSTER_1, 0x0040)

#define SDP_USE_ID	BUILD_USE_ID( 0, BLUETOOTH_USE_ID_CLUSTER_1, 0x0080)

#define DUN_USE_ID	BUILD_USE_ID( 0, BLUETOOTH_USE_ID_CLUSTER_1, 0x0100)

#define FAX_USE_ID	BUILD_USE_ID( 0, BLUETOOTH_USE_ID_CLUSTER_1, 0x0200)

#define	OBX_USE_ID	BUILD_USE_ID( 0, BLUETOOTH_USE_ID_CLUSTER_1, 0x0400)

#define	OPP_USE_ID	BUILD_USE_ID( 0, BLUETOOTH_USE_ID_CLUSTER_1, 0x0800)

#define	FTP_USE_ID	BUILD_USE_ID( 0, BLUETOOTH_USE_ID_CLUSTER_1, 0x1000)

#define	SYN_USE_ID	BUILD_USE_ID( 0, BLUETOOTH_USE_ID_CLUSTER_1, 0x2000)

/*@}*/



/**
 * @name	Services cluster
 *
 * Services
 */
/*@{*/

#define EXPL_USE_ID   	BUILD_USE_ID( 0, SERVICES_USE_ID_CLUSTER_1, 0x0001)

#define	AUDIO_USE_ID   	BUILD_USE_ID( 0, SERVICES_USE_ID_CLUSTER_1, 0x0002)

#define	ETM_USE_ID   	BUILD_USE_ID( 0, SERVICES_USE_ID_CLUSTER_1, 0x0004)

#define	DAR_USE_ID   	BUILD_USE_ID( 0, SERVICES_USE_ID_CLUSTER_1, 0x0008)

#define	MKS_USE_ID   	BUILD_USE_ID( 0, SERVICES_USE_ID_CLUSTER_1, 0x0010)

#define	MPM_USE_ID   	BUILD_USE_ID( 0, SERVICES_USE_ID_CLUSTER_1, 0x0020)

#define	LLS_USE_ID   	BUILD_USE_ID( 0, SERVICES_USE_ID_CLUSTER_1, 0x0040)

#define	ATP_USE_ID   	BUILD_USE_ID( 0, SERVICES_USE_ID_CLUSTER_1, 0x0080)

#define	ATP_UART_USE_ID	BUILD_USE_ID( 0, SERVICES_USE_ID_CLUSTER_1, 0x0100)

#define	MDC_USE_ID   	BUILD_USE_ID( 0, SERVICES_USE_ID_CLUSTER_1, 0x0200)

#define TTY_USE_ID	BUILD_USE_ID( 0, SERVICES_USE_ID_CLUSTER_1, 0x0400)

#define	DCM_USE_ID	BUILD_USE_ID( 0, SERVICES_USE_ID_CLUSTER_1, 0x0800)

#define	DCFG_USE_ID	BUILD_USE_ID( 0, SERVICES_USE_ID_CLUSTER_1, 0x1000)

#define	MMS_USE_ID	BUILD_USE_ID( 0, SERVICES_USE_ID_CLUSTER_1, 0x1000)
/*@}*/


/**
 * @name	Services cluster - 2
 *
 * Services
 */
/*@{*/

#define MFW_USE_ID	BUILD_USE_ID( 0, SERVICES_USE_ID_CLUSTER_2, 0x0001)

#define SMBS_USE_ID	BUILD_USE_ID( 0, SERVICES_USE_ID_CLUSTER_2, 0x0002)

#define	AUDIO_BGD_USE_ID    BUILD_USE_ID( 0, SERVICES_USE_ID_CLUSTER_2, 0x0004)

#define IMG_USE_ID 	BUILD_USE_ID( 0, SERVICES_USE_ID_CLUSTER_2, 0x0008)

#define MDL_USE_ID      BUILD_USE_ID( 0, SERVICES_USE_ID_CLUSTER_2, 0x0010)

/*@}*/


/**
 * @name	Test cluster
 *
 * Tests related SWEs
 */
/*@{*/

#define	 RTEST_USE_ID	BUILD_USE_ID( 0, TEST_USE_ID_CLUSTER_1, 0x0001)

/* maybe put in another cluster */
#define	 TUT_USE_ID	BUILD_USE_ID( 0, TEST_USE_ID_CLUSTER_1, 0x0002)

/*@}*/


/**
 * @name	J2ME cluster
 *
 * Java related SWEs
 */
/*@{*/

#define	 KIL_USE_ID	BUILD_USE_ID( 0, JAVA_USE_ID_CLUSTER_1, 0x0001)

#define	 KGC_USE_ID	BUILD_USE_ID( 0, JAVA_USE_ID_CLUSTER_1, 0x0002)

#define	 KCL_USE_ID	BUILD_USE_ID( 0, JAVA_USE_ID_CLUSTER_1, 0x0004)

#define	 KMM_USE_ID	BUILD_USE_ID( 0, JAVA_USE_ID_CLUSTER_1, 0x0008)

#define	 KNM_USE_ID	BUILD_USE_ID( 0, JAVA_USE_ID_CLUSTER_1, 0x0010)

#define	 UVM_USE_ID	BUILD_USE_ID( 0, JAVA_USE_ID_CLUSTER_1, 0x0020)

#define	 KZP_USE_ID	BUILD_USE_ID( 0, JAVA_USE_ID_CLUSTER_1, 0x0040)

#define	 KPG_USE_ID	BUILD_USE_ID( 0, JAVA_USE_ID_CLUSTER_1, 0x0080)

#define	 JTM_USE_ID	BUILD_USE_ID( 0, JAVA_USE_ID_CLUSTER_1, 0x0100)

/*@}*/



/**
 * @name	Stack TCP/IP cluster
 *
 * Stack TCP/IP.
 */
/*@{*/
#define RNET_USE_ID     BUILD_USE_ID( 0, TCPIP_USE_ID_CLUSTER_1, 0x0001)

#define RNET_WS_USE_ID  BUILD_USE_ID( 0, TCPIP_USE_ID_CLUSTER_1, 0x0002)

#define RNET_RT_USE_ID  BUILD_USE_ID( 0, TCPIP_USE_ID_CLUSTER_1, 0x0004)

#define RNET_BR_USE_ID  BUILD_USE_ID( 0, TCPIP_USE_ID_CLUSTER_1, 0x0008)

/*@}*/


/**
 * @name	Condat cluster
 *
 * Condat related SWEs
 */
/*@{*/

#define	 CONDAT_FRM_USE_ID BUILD_USE_ID( 0, CONDAT_USE_ID_CLUSTER_1, 0x0001)


/*@}*/



/**
 * @name	Obigo cluster
 *
 * Obigo related SWE's
 */
/*@{*/
#define MSME_USE_ID BUILD_USE_ID( 0, OBIGO_USE_ID_CLUSTER_1, 0x0001)

#define MSFE_USE_ID BUILD_USE_ID( 0, OBIGO_USE_ID_CLUSTER_1, 0x0002)

#define STKE_USE_ID BUILD_USE_ID( 0, OBIGO_USE_ID_CLUSTER_1, 0x0004)

#define BRSE_USE_ID BUILD_USE_ID( 0, OBIGO_USE_ID_CLUSTER_1, 0x0008)

#define BRAE_USE_ID BUILD_USE_ID( 0, OBIGO_USE_ID_CLUSTER_1, 0x0010)

#define PHSE_USE_ID BUILD_USE_ID( 0, OBIGO_USE_ID_CLUSTER_1, 0x0020)

#define MMSE_USE_ID BUILD_USE_ID( 0, OBIGO_USE_ID_CLUSTER_1, 0x0040)

#define SLSE_USE_ID BUILD_USE_ID( 0, OBIGO_USE_ID_CLUSTER_1, 0x0080)

#define SMAE_USE_ID BUILD_USE_ID( 0, OBIGO_USE_ID_CLUSTER_1, 0x0100)

#define MEAE_USE_ID BUILD_USE_ID( 0, OBIGO_USE_ID_CLUSTER_1, 0x0200)

#define SECE_USE_ID BUILD_USE_ID( 0, OBIGO_USE_ID_CLUSTER_1, 0x0400)

#define SELE_USE_ID BUILD_USE_ID( 0, OBIGO_USE_ID_CLUSTER_1, 0x0800)

#define PRSE_USE_ID BUILD_USE_ID( 0, OBIGO_USE_ID_CLUSTER_1, 0x1000)

#define JAAE_USE_ID BUILD_USE_ID( 0, OBIGO_USE_ID_CLUSTER_1, 0x2000)

#define JASE_USE_ID BUILD_USE_ID( 0, OBIGO_USE_ID_CLUSTER_1, 0x4000)
/*@{*/


/**
 * @name	Obigo second cluster
 *
 * Obigo related test SWE's
 */
#define EMAE_USE_ID BUILD_USE_ID( 0, OBIGO_USE_ID_CLUSTER_2, 0x0001)

#define EMSE_USE_ID BUILD_USE_ID( 0, OBIGO_USE_ID_CLUSTER_2, 0x0002)

#define IT1E_USE_ID BUILD_USE_ID( 0, OBIGO_USE_ID_CLUSTER_2, 0x0004)

#define IT2E_USE_ID BUILD_USE_ID( 0, OBIGO_USE_ID_CLUSTER_2, 0x0008)

#define IT0E_USE_ID BUILD_USE_ID( 0, OBIGO_USE_ID_CLUSTER_2, 0x0010)

#define UISE_USE_ID BUILD_USE_ID( 0, OBIGO_USE_ID_CLUSTER_2, 0x0020)

#define UIAE_USE_ID BUILD_USE_ID( 0, OBIGO_USE_ID_CLUSTER_2, 0x0040)
/*@{*/


#endif  /* __RVM_USE_ID_LIST_H_ */
