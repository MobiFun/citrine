/******************************************************************************/
/*                                                                            */
/*    File Name:   dar_gen.h                                                  */
/*                                                                            */
/*    Purpose:     This file contain general definitions of DAR Module.       */
/*                                                                            */
/*    Note:                                                                   */
/*        None.                                                               */
/*                                                                            */
/*    Revision History:                                                       */
/*       9 October 2001      Stephanie Gerthoux        Create                 */
/*                                                                            */
/* (C) Copyright 2001 by Texas Instruments Incorporated, All Rights Reserved. */
/*                                                                            */
/******************************************************************************/

#ifndef __DAR_GEN_H__
#define __DAR_GEN_H__

   #include "../../riviera/rv/general.h"


   /**** Type definitions ****/

   /** Dar data format : ASCII or binary **/ 
   typedef INT8 T_DAR_FORMAT;
   /* possible values */
   #define DAR_ASCII_FORMAT     (0)   /* ASCII format */
   #define DAR_BINARY_FORMAT    (-1)  /* Binary format */

   /** DAR data level ( Error / Warning / Debug) **/
   typedef UINT8 T_DAR_LEVEL;

   typedef UINT8* T_DAR_BUFFER;

   typedef char T_DAR_INFO;

   /** DAR callback function **/
   typedef void (*DAR_CALLBACK_FUNC)(T_DAR_BUFFER, UINT16);

   /** DAR Level Default definition **/
   #define DAR_LEVEL_DEFAULT     (DAR_ERROR)

#endif
