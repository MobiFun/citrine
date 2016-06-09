/*
+-----------------------------------------------------------------------------
|  Project :  GSM-PS
|  Modul   :  DL
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
|  Purpose :  Trace Definitions for the Protocol Stack Entity
|             Data Link Layer
+-----------------------------------------------------------------------------
*/

#ifndef DL_TRC_H
#define DL_TRC_H

#if defined(DL_TRACE_WIN32)
void dl_trc_init ();
void dl_trc_exit ();
void dl_trc_printf (char *fmt, ...);
void dl_trc_print_array(unsigned char *pArray, int nLength);
void dl_trc_frame(unsigned char ch_type, unsigned char *pFrame, int direction);

#define TRC_INIT()    dl_trc_init ()
#define TRC_EXIT()    dl_trc_exit ()
#define TRC(t)        dl_trc_printf t
#define ATRC(p,l)     dl_trc_print_array ((unsigned char *)(p),l)
#define FTRC(c,p,d)   dl_trc_frame(c, p, d)

#else

#define TRC_INIT()
#define TRC_EXIT()
#define TRC(t)
#define ATRC(p,l)
#define FTRC(c,p,d)
#endif  /* DL_TRACE_WIN32 */

#endif  /* DL_TRC_H */
