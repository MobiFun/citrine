/****************************************************************************/
/*                                                                          */
/*  Name        rvf_mem_pool.c                                              */
/*                                                                          */
/*  Function    this file contains the definition of main memory pool       */
/*                                                                          */
/*  Version	0.1						            */
/*							                    */
/*  Date       	Modification					            */
/*  ------------------------------------			            */
/*  06/08/02	Create						            */
/*							                    */
/*  Author	Laurent Sollier (l-sollier@ti.com)		            */
/*							                    */
/* (C) Copyright 2002 by Texas Instruments Incorporated, All Rights Reserved*/
/****************************************************************************/

#include "rvf_pool_size.h"
#include "rvf_cust.h"

/******** MEMORY POOLS ******************/
/* Define the buffer pools */
T_RVF_POOL _rvf_pools[RVF_NB_POOLS];


/* allocated static buffer pools */
UINT8 Buf0[RVF_POOL_0_SIZE] __attribute__ ((section ("ext.ram")));

#if (RVF_NB_POOLS > 1)
UINT8 Buf1[RVF_POOL_1_SIZE] __attribute__ ((section ("int.ram")));
#endif

/*******************************************************************************
**
** Function         _rvf_init_mem_pool
**
** Description      This function is called to initialize the different memory
**                  pool.
**
** Returns          void
**
*******************************************************************************/
void _rvf_init_mem_pool(void)
{
	/* initialize buffer pools */
   _rvf_init_free_queue(0, RVF_POOL_0_SIZE, (UINT8 *)Buf0);

#if (RVF_NB_POOLS > 1)
   _rvf_init_free_queue(1, RVF_POOL_1_SIZE, (UINT8 *)Buf1);
#endif
}

/*******************************************************************************
**
** Function         _rvf_get_mem_usage_ratio
**
** Description      This function is called to have the value of
**                  RVF_MEM_USAGE_RATIO.
**
** Returns          The value of RVF_MEM_USAGE_RATIO (see details of this value
**                  in rvf_pool_size.h)
**
*******************************************************************************/
UINT16 _rvf_get_mem_usage_ratio(void)
{
   return RVF_MEM_USAGE_RATIO;
}

/*******************************************************************************
**
** Function         _rvf_get_number_of_pool
**
** Description      This function is called to have the number of memory pool.
**
** Returns          The number of available different pool
**
*******************************************************************************/
UINT16 _rvf_get_number_of_pool(void)
{
   return RVF_NB_POOLS;
}
