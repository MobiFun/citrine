/**
 * @file	spi_pool_size.h
 *
 * Declarations of:
 * - the memory bank sizes and their watermark
 * - the SWE stack size
 * - the pool size needed (generally the sum of memory bank and stack sizes)
 *
 * @author	Vincent Oberle
 * @version 0.1
 */

/*
 * History:
 *
 * Date       	Author				Modification
 * -------------------------------------------------------------------
 * 07/08/2003	Vincent Oberle			Extracted from rvf_pool_size.h
 *
 * (C) Copyright 2003 by Texas Instruments Incorporated, All Rights Reserved
 */

#ifndef __SPI_POOL_SIZE_H_
#define __SPI_POOL_SIZE_H_


/*
 * Values used in spi_env.h
 */
#define SPI_STACK_SIZE (1000)
#define SPI_MB1_SIZE   (256)
#define SPI_POOL_SIZE  (SPI_STACK_SIZE + SPI_MB1_SIZE)


#endif /*__SPI_POOL_SIZE_H_*/
