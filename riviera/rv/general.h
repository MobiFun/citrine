/****************************************************************************/
/*                                                                          */
/*  Name        general.h                                                   */
/*                                                                          */
/*  Function    this file contains common data type definitions used        */
/*              throughout the SWE                                          */
/*                                                                          */
/*  Date       Modification                                                 */
/*  -----------------------                                                 */
/*  3/12/99    Create                                                       */
/* **************************************************************************/
/*  10/27/1999 David Lamy-Charrier: remove declaration of ntohs, htons,     */
/*                                  ntohl, htonl in order to avoid conflict */
/*                                  with winsock.h                          */
/*                                                                          */
/*  11/30/1999 Pascal Pompei: 'string.h' included in order to define memcmp,*/
/*                            memset and memcpy functions.                  */
/*                                                                          */
/*                                                                          */
/****************************************************************************/

#ifndef GENERAL_H
#define GENERAL_H

#ifdef _WINDOWS
   #include <string.h>
#endif

/* WINDOWS */
#ifdef _WINDOWS
   typedef unsigned short  UINT16;
   typedef unsigned int    UINT32;
   typedef unsigned char   UBYTE;
   typedef short           SHORT;
   typedef int             BOOL;

/* BOARD */
#else
   #ifndef __TYPEDEFS_H__ /* This #define allows to Condat to use general.h without conflict */
      typedef unsigned short  UINT16;
      typedef unsigned char   UBYTE;
      typedef short           SHORT;
      typedef signed char     BYTE;
      #if !defined (BOOL_FLAG)
         #define BOOL_FLAG
         typedef unsigned char BOOL;
      #endif
      typedef unsigned short  USHORT;
      typedef unsigned int    ULONG;
   #endif
   typedef unsigned long    UINT32;
#endif

typedef unsigned char   UINT8;
typedef signed char     INT8;
typedef short           INT16; 
typedef int             INT32;
typedef unsigned char   BOOLEAN;

typedef void (*FUNC)(void);      /* pointer to a function */

#define OK        1

#ifndef NULL
   #define NULL   0
#endif

#ifndef TRUE
   #define TRUE   1
#endif

#ifndef FALSE
   #define FALSE  0
#endif


#define htons  ntohs
#define htonl  ntohl


#if !defined(_WIN32) 
   #define ntohs(n) (n)
   #define ntohl(n) (n)
   #define ntoh6(n) (n)
#endif

#endif /* #ifndef GENERAL_H */
