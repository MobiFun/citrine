/* 
+----------------------------------------------------------------------------- 
|  Project :  GSM (6103)
|  Modul   :  
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
|  Purpose :  Types definitions for the display driver
|             .
+-----------------------------------------------------------------------------
     History:
 	Sept 15, 2005 REF: LOCOSTO-ENH-34257 - xpradipg
	Description: Locosto: MIgration to New LCD APIs
	Solution: Migrated to New APIs by replacing the old driver APIs with
	corresponding New LCD APIs
+----------------------------------------------------------------------------- 
*/ 
#ifndef DSPL_H
#define DSPL_H
#include "font_bitmaps.h"

//Sept 15, 2005 REF: LOCOSTO-ENH-34257 - xpradipg
//The file is included for the definition of T_RV_RET


/*
 *   Device Capabilities data Type
 */
typedef struct dspl_DevCaps
{
  UBYTE    DisplayType;
  USHORT   Width;
  USHORT   Height;
} dspl_DevCaps;

/*
 * Display Types
 */
#define    DSPL_TYPE_CHARACTER    0
#define    DSPL_TYPE_GRAPHIC      1
#define    DSPL_TYPE_COLOR        2

/*
 * Character Types
 */

#define    DSPL_TYPE_ASCII         1
#define    DSPL_TYPE_UNICODE      2
#define    DSPL_TYPE_ASCII_12_6    3


/*
 * Display Text Attributes
 */
#define    DSPL_TXTATTR_NORMAL              0x00
#define    DSPL_TXTATTR_INVERS              0x01
#define    DSPL_TXTATTR_UNICODE             0x02
#define   DSPL_TXTATTR_CURRENT_MODE         0x04    /*Represents the string in the current display mode,unicode or ASCII*/
#define   DSPL_TXTATTR_SIGNED_COORDS        0x08    /*SH - if this is set, coordinates can have negative numbers*/
#define    DSPL_TXTATTR_HLIGHT    			0x10   /*Same as 'inverse' on B+W - diff colour highlight on colour display */

/*
 * Cursor Types
 */
#define    DSPL_FBOX_CURSOR_TYPE  1
#define    DSPL_OBOX_CURSOR_TYPE  2
#define    DSPL_TLIN_CURSOR_TYPE  3
#define    DSPL_BLIN_CURSOR_TYPE  4

#define    DSPL_SLOWFLASH_MODE    1
#define    DSPL_FASTFLASH_MODE    2
#define    DSPL_STATIC_MODE       3

#define    DSPL_CURSOR_VISIBLE    1
#define    DSPL_CURSOR_INVISIBLE  0
/*
 * Raster Operations
 */
#define    DSPL_BMPINVERT         1
#define    DSPL_BMPAND            2
#define    DSPL_BMPCOPY           4
#define    DSPL_BMPERASE          8
#define    DSPL_BMPPAINT          16


/*mc, SPR 1319 moved definitions to header*/
#define TXT_STYLE_NORMAL		(0)
#define TXT_STYLE_INVERT		(1)
#define TXT_STYLE_HIGHLIGHT		(2)
#define TXT_STYLE_BORDER		(3)
#define TXT_STYLE_SHADOW1		(4)
#define TXT_STYLE_SHADOW2		(5)
#define TXT_STYLE_2PIXEL_BORDER		(6)
#define TXT_STYLE_3PIXEL_BORDER		(7)
#define TXT_STYLE_4PIXEL_BORDER		(8)
#define TXT_STYLE_MASK			(0x00FF)
#define TXT_STYLE_WIDECHAR		(0x0100)
#define TXT_STYLE_HIGHCHAR		(0x0200)
#define TXT_STYLE_UNICODE		(0x0400)

/*mc end*/

/*
 * Return Values
 */
#define    DSPL_FCT_NOTSUPPORTED  1

#if defined (NEW_FRAME)
/*
 * to achieve backward compatibility with older definitions
 */
#define drv_SignalCB_Type           T_DRV_CB_FUNC
#define drv_SignalID_Type           T_DRV_SIGNAL
#define T_VSI_THANDLE               USHORT
#endif

/*
 * Prototypes
 */

EXTERN UBYTE    dspl_Init                (void);
EXTERN void     dspl_Exit                (void);
EXTERN UBYTE    dspl_Clear               (USHORT              in_X1,
                                          USHORT              in_Y1,
                                          USHORT              in_X2,
                                          USHORT              in_Y2);
EXTERN UBYTE    dspl_ClearAll            (void);
EXTERN UBYTE dspl_unfocusDisplay (void);

EXTERN UBYTE    dspl_Enable              (UBYTE               in_Enable);
EXTERN void     dspl_GetDeviceCaps       (dspl_DevCaps      * out_DeviceCapsPtr);
EXTERN void     dspl_SetDeviceCaps       (dspl_DevCaps      * in_DeviceCapsPtr);
EXTERN UBYTE    dspl_GetIconImage        (UBYTE               in_Icon,
                                          USHORT              in_Size,
                                          UBYTE             * out_IconImagePtr);
EXTERN UBYTE    dspl_SetCursor           (UBYTE               in_CursorType,
                                          UBYTE               in_CursorMode);
EXTERN UBYTE    dspl_SetCursorPos        (USHORT              in_X,
                                          USHORT              in_Y,
                                          USHORT              in_SizeX,
                                          USHORT              in_SizeY);//GW 05/09/01
EXTERN UBYTE    dspl_ShowCursor          (UBYTE               in_Show);
EXTERN UBYTE    dspl_SetBkgColor         (UBYTE               in_Color);
EXTERN UBYTE    dspl_SetFrgColor         (UBYTE               in_Color);
EXTERN UBYTE    dspl_DrawIcon            (UBYTE               in_IconID,
                                          USHORT              in_X,
                                          USHORT              in_Y);
EXTERN UBYTE    dspl_DrawLine            (USHORT              in_X1,
                                          USHORT              in_Y1,
                                          USHORT              in_X2,
                                          USHORT              in_Y2);
EXTERN UBYTE    dspl_DrawRect            (USHORT              in_X1,
                                          USHORT              in_Y1,
                                          USHORT              in_X2,
                                          USHORT              in_Y2);
EXTERN UBYTE dspl_roundRect (int px,
                            	int  py,
                            	int sx,
                            	int sy,
                            	int border);
EXTERN UBYTE dspl_roundRectFill (int px,
                            	int  py,
                            	int sx,
                            	int sy,
                            	int border);
EXTERN UBYTE dspl_DrawFilledRect (USHORT in_X1,
								  USHORT in_Y1,
								  USHORT in_X2,
								  USHORT in_Y2);
EXTERN UBYTE dspl_DrawFilledBgdRect (USHORT in_X1,
								  USHORT in_Y1,
								  USHORT in_X2,
								  USHORT in_Y2);
EXTERN UBYTE dspl_DrawFilledColRect (USHORT in_X1,
								  USHORT in_Y1,
								  USHORT in_X2,
								  USHORT in_Y2,
								  U32 Col);

EXTERN UBYTE    dspl_Ellipse             (USHORT              in_X1,
                                          USHORT              in_Y1,
                                          USHORT              in_X2,
                                          USHORT              in_Y2);
EXTERN UBYTE    dspl_BitBlt              (USHORT              in_X,
                                          USHORT              in_Y,
                                          USHORT              in_Width,
                                          USHORT              in_Height,
                                          USHORT              in_Index,
                                          void              * in_BmpPtr,
                                          USHORT              in_Rop);
EXTERN UBYTE dspl_BitBlt2(short in_X,
                          short in_Y,
                          USHORT in_Width,
                          USHORT in_Height,
                          void * in_BmpPtr,
                          USHORT in_index,
                          int bmpFormat);

void fastCopyBitmap(int startX, 	int startY, 	// start position of bitmap
						int bmpSx,	int bmpSy,		//size of bitmap
						char*	srcBitmap,
						int posX,   	int posY,   	// start of area to be copied into
						int sx,     	int sy,     	// size of area to be copied into 
						U32 bgd_col,	int bmptype);

EXTERN UBYTE    dspl_SelectFontbyID      (UBYTE               in_Font);
EXTERN UBYTE    dspl_SelectFontbyImage   (UBYTE             * in_FontPtr);
EXTERN UBYTE    dspl_GetFontImage        (UBYTE               in_Font,
                                          USHORT              in_Size,
                                          UBYTE             * out_FontPtr);
EXTERN UBYTE    dspl_GetFontHeight       (void);
EXTERN USHORT   dspl_GetTextExtent       (char              * in_Text,
                                          USHORT              in_Length);
EXTERN USHORT   dspl_GetMaxTextLen       (char              * in_Text,
                                          USHORT              in_HSize);
EXTERN UBYTE    dspl_TextOut_Cmode             (USHORT              in_X,
                                          USHORT              in_Y,
                                          UBYTE               in_Attrib,
                                          char              * in_Text);
EXTERN UBYTE    dspl_TextOut             (USHORT              in_X,
                                          USHORT              in_Y,
                                          UBYTE               in_Attrib,
                                          char              * in_Text);
EXTERN void dspl_ScrText (int x, int y, char *txt, int style);

EXTERN UBYTE    dspl_SetWorkShadow       (UBYTE             * in_ShadowPtr);
EXTERN UBYTE    dspl_SetDisplayShadow    (UBYTE             * in_ShadowPtr);

EXTERN UBYTE     dspl_str_length(char * str);

EXTERN void dspl_set_char_type(UBYTE char_type);
EXTERN UBYTE dspl_get_char_type(void);

int dspl_getDisplayType( void );

GLOBAL USHORT dspl_GetNcharToFit (char * in_Text, USHORT pixelWidth);/*SPR 1541*/

//Functions to allow us to set-up the border around text.
int dspl_setBorderWidth(int borderSize);
int dspl_getBorderWidth(void);
//Condat UK Resources functions

EXTERN UBYTE dspl_Prompt (USHORT x, USHORT y, UBYTE in_Attrib, int StringID);

//GW Added new prototypes for setting and getting foreground and background colours
//NB Existing (unsupported) procedures cannot be used as these have colour defined as 256 colour not 32bit
EXTERN U32 dspl_SetBgdColour (U32 inColour);
EXTERN U32 dspl_GetBgdColour (void);
EXTERN U32 dspl_SetFgdColour (U32 inColour);
EXTERN U32 dspl_GetFgdColour (void);
GLOBAL void dspl_RestoreColour (void);
GLOBAL void dspl_InitColour (void);

//Select a colour to contrast with another colour -used when the foreground and background colours are the same
int dspl_GetContrastColour( int ipCol);

//Window types when displaying a bitmap
enum {
	DSPL_WIN_NORMAL = 0,
	DSPL_WIN_CENTRE,
	DSPL_WIN_CLIP,
	DSPL_WIN_CENTRE_CLIP,
	DSPL_WIN_TILE,
	DSPL_WIN_LAST
};
//Draw win - create window with display area as specified
EXTERN UBYTE    dspl_DrawWin (	USHORT		in_PX, 	USHORT	in_PY,
             						USHORT 	in_SX,	USHORT	in_SY,
                                    int		format, t_font_bitmap* bmp );

//Functions to allow us to limit where things are drawn - text/bitmaps outside the window are not drawn
GLOBAL UBYTE dspl_SetWindow(USHORT   in_X1,
                         USHORT   in_Y1,
                         USHORT   in_X2,
                         USHORT   in_Y2);
GLOBAL UBYTE dspl_GetWindow(USHORT* x1,USHORT* y1,USHORT* x2,USHORT* y2 );
GLOBAL UBYTE dspl_ResetWindow( void ); //reset to full screen


//GW Added (temp)
typedef enum {
	BMP_FORMAT_BW_PACKED    = 0x00,
	BMP_FORMAT_BW_UNPACKED  =   0x01,/*MC, SPR1319*/
	BMP_FORMAT_256_COLOUR   = 0x02,
	BMP_FORMAT_32BIT_COLOUR = 0x03,
	BMP_FORMAT_16BIT_LCD_COLOUR = 0x04, /* GW matches current LCD data format */
	BMP_FORMAT_16BIT_LCD_COMPRESSED_COLOUR = 0x05, /* GW matches future LCD data format */
	BMP_FORMAT_BW_2x4  = 0x0080, /* GW display bitmap scaled 2x width and 4x height */
	BMP_FORMAT_END

} BMP_FORMAT_ENUM;

#define SHOWBITMAP_NORMAL 0x0000
#define SHOWBITMAP_INVERT 	0x0001
#define SHOWBITMAP_NORMAL_MASK 	0x0001
#define SHOWBITMAP_SCALE_2x4 0x0080

//Enumerated type for display (for MFW layout)
enum {
	DSPL_BW 		= 0,
	DSPL_COLOUR 	= 1,
	DSPL_END
};

//Sept 15, 2005 REF: LOCOSTO-ENH-34257 - xpradipg
enum
{
	DSPL_ACCESS_ENABLE = 0,
	DSPL_ACCESS_DISABLE ,
	DSPL_ACCESS_QUERY
};

#ifdef _SIMULATION_
EXTERN void scrMobUpdate (void);
#endif
#ifdef FF_SSL_ADAPTATION
void dspl_set_to_mixed_mode(void);
void dspl_set_to_mmi_mode(void);
#endif


//xpradipg : New LCD API Migration
//New function added to control the refresh of the LCD, the earlier implement
//-ion of the global variable is moved inside this function and also adopted 
//for the new LCD API migration
EXTERN int	dspl_control(int state);

#endif
