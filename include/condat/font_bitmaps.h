/* 
+----------------------------------------------------------------------------- 
|  Project :  
|  Modul   :  display
+----------------------------------------------------------------------------- 
|  Copyright 2002 Texas Instruments Berlin, AG 
|                 All rights reserved. 
| 
|                 This file is confidential and a trade secret of Texas 
|                 Instruments Berlin, AG 
|                 The receipt of or possession of this file does not convey 
|                 any rights to reproduce or disclose its contents or to 
|                 manufacture, use, or sell anything it may describe, in 
|                 whole, or in part, without the specific written consent of 
|                 Texas Instruments Berlin, AG. 
+----------------------------------------------------------------------------- 
|  Purpose :  
+----------------------------------------------------------------------------- 
*/ 
#ifndef FONT_BITMAPS_H
#define FONT_BITMAPS_H


typedef struct{

USHORT code;
UBYTE  format;
UBYTE  height;
UBYTE width;
UBYTE bitmapSize;
char* bitmap;
}t_font_bitmap;

enum {
	/*---------------------------USE VALUES BETWEEN -1 TO 254------------------------------*/
	NO_FONT=-1,
//Currently we only support this font for smaller displays
#ifndef LSCREEN
	DEFAULT_8x6=0,
#endif
//We need only support this font on larger displays
#ifdef LSCREEN
	CHANTICLE_PROP15 = 1,
#if 0 //Using Arial-type may be problematic - disable
	ARIAL_PROP15 = 2,
#endif

#endif
	LAST_FONT
	/*---------------------------USE VALUES BETWEEN -1 TO 254-------------------------------*/
};


t_font_bitmap* get_bitmap(USHORT selected_code);
USHORT font_setFont(USHORT font);
USHORT font_getFont(void);
USHORT font_getCharWidth(USHORT selected_code);
void font_initFont( int defaultFont );
/* SPR#1983 - SH - Get list of all ascii font widths in array */
void font_getAllAsciiWidths(UBYTE *width);

#endif
