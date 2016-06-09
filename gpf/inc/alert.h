/* 
+------------------------------------------------------------------------------
|  File:       alert.h
+------------------------------------------------------------------------------
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
|  Purpose :  definitions for the ALERT macro.
+----------------------------------------------------------------------------- 
*/ 

#ifndef ALERT_H
#define ALERT_H

#include "gpfconf.h"

#undef ALERT
#undef E_ALERT

#ifndef ALERT_OFF
  #include "typedefs.h"
  #include "vsi.h" 
  
  extern BOOL _Alert(char *, T_HANDLE, ULONG);
  extern BOOL alert_info(char *, ...);
  #define NO_ALERT_INFO alert_info("")

  #define _STR(x) _VAL(x)
  #define _VAL(x) #x
  /*
   * The && FALSE in the end is just to notify the reader that 
   * FALSE is always returned if the predicate evaluates to false.
   */
  #define ALERT(expression, alertclass, function) ((expression) ? ((void) 0) : \
          (void) (_Alert(__FILE__ "(" _STR(__LINE__) ") \"" #expression "\"", VSI_CALLER TC_ALERT_##alertclass) && (function)))
 
  #define E_ALERT(expression, alertclass, function) ((expression) ? (TRUE) : \
          (BOOL)  (_Alert(__FILE__ "(" _STR(__LINE__) ") \"" #expression "\"", VSI_CALLER TC_ALERT_##alertclass) && (function) && FALSE))

#else /* ALERT_OFF */
  #define NO_ALERT_INFO (0)
  #define ALERT(expression, alertclass, function)   ((void) 0)
  #define E_ALERT(expression, alertclass, function) (expression)
#endif /* ALERT_OFF */

#endif /* ALERT_H */

