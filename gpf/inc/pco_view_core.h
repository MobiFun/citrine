/* 
+----------------------------------------------------------------------------- 
|  Project :  PCO2
|  Modul   :  PCO_VIEW_CORE
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
|  Purpose :  This Modul contains the core viewer class
|             with basic functions and handlers for several PCO control 
|             messages.
|             It is intended to be specialized and extended, e.g. in 
|             GUI-server-applications.
|             Some member functions are totally virtual without any 
|             standard body and have to be provided by the child class !
+----------------------------------------------------------------------------- 
*/ 

#ifndef _PCO_VIEW_CORE_H_
#define _PCO_VIEW_CORE_H_

/*==== INCLUDES ===================================================*/
#ifndef PCO_VIEW_CORE_CPP
#include "pco_view_templ.h"
#else
#include "view/pco_view_templ.h"
#endif
#include "pco_inifile.h"

/*==== DEFINES =====================================================*/

/*==== PROTOTYPES ==================================================*/

/*==== CLASSES =====================================================*/
class PCOView_core : public PCOView_templ
{
public:
  PCOView_core(const char* ini_file, int& err,
      const char* primq_name="", const char* ctrlq_name="");
  virtual ~PCOView_core();

  /*
  +-------------------------------------------------------------------------------+
  | PROJECT : PCO2                       MODULE  : PCO_CTRL_CORE                  |
  | STATE   : code                       ROUTINE : PCOView_core::dispatch_message           |
  +-------------------------------------------------------------------------------+

    PURPOSE : parses a PCO control message
    PARAMS:   buf   ... the data
              size   .. size of buf
              id    ... id of the message
              sender .. queue name of sender

    RETURNS:   0 .. if message has been handled
              -1 .. otherwise

  */
  virtual int dispatch_message(void* buf, U16 size, U16 id, const char* sender);

  
  /*
  +-------------------------------------------------------------------------------+
  | PROJECT : PCO2                       MODULE  : PCO_CTRL_CORE                  |
  | STATE   : code                       ROUTINE : PCOView_core::interpret_message|
  +-------------------------------------------------------------------------------+

    PURPOSE : here interpretation of received raw data takes place
              (has to be implemented by derived classes !)

    PARAMS:   buffer  .. raw data to be interpretated
              bufsize .. size of buffer
              data    .. actual data
              size    .. size of data
              id      .. id of data message
              index   .. index of data message (e.g. in logfile) ... 0 means no index!!
              ttype   .. type of time stamp - see PCO_TTYPE_XXX constants
              time    .. time stamp 
              sender  .. name of sender
              receiver.. name of original receiver

    RETURNS:   0 .. success
              -1 .. interpretation was not possible

  */
  virtual int interpret_message(void* buffer, U16 bufsize, 
    void* &data, U16 &size, ULONG &id, ULONG& index, ULONG& ttype, U32 &time, 
    char* &sender, char* &receiver) {return -1;}

  /*
  +-------------------------------------------------------------------------------+
  | PROJECT : PCO2                       MODULE  : PCO_VIEW_CORE                  |
  | STATE   : code                       ROUTINE : PCOView_core::on_data          |
  +-------------------------------------------------------------------------------+

    PURPOSE : here reaction to received data takes place
              (has to be implemented by derived classes !)

    PARAMS:   data   .. the data
              size   .. size of data
              id     .. id of data message
              index  .. index of data message (e.g. in logfile) ... 0 means no index!!
              ttype  .. type of time stamp - see PCO_TTYPE_XXX constants
              time   .. time stamp 
              sender .. name of sender
              receiver.. name of original receiver

  */
  virtual void on_data(void* data, U16 size, ULONG id, ULONG index,
    ULONG ttype, U32 time, const char* sender, char* receiver) {}

  /*
  +-------------------------------------------------------------------------------+
  | PROJECT : PCO2                       MODULE  : PCO_VIEW_CORE                  |
  | STATE   : code                       ROUTINE : PCOView_core::on_raw_data      |
  +-------------------------------------------------------------------------------+

    PURPOSE : here reaction to the raw data just as received from server takes place
              (may be implemented by derived classes)
              if interpretation succeeded it will be called AFTER on_data(), otherwise
              only on_raw_data() is called

    PARAMS:   rawdata  .. the data
              rawsize  .. size of data

  */
  virtual void on_raw_data(void* rawdata, U16 rawsize) {}

  /*
  +-------------------------------------------------------------------------------+
  | PROJECT : PCO2                       MODULE  : PCO_VIEW_CORE                  |
  | STATE   : code                       ROUTINE : PCOView_core::on_corrupt_data  |
  +-------------------------------------------------------------------------------+

    PURPOSE : this function is called if corrupt data has been received
              deriving viewers may, e.g., release semaphores

    PARAMS:   
  */
  virtual void on_corrupt_data() {}

  /*
  +----------------------------------------------------------------------------------+
  | PROJECT : PCO2                       MODULE  : PCO_VIEW_CORE                     |
  | STATE   : code                       ROUTINE : PCOView_core::propagate_inichange |
  +----------------------------------------------------------------------------------+

    PURPOSE : saves ini-file and sends an information about important ini-file changes
              to the server, which will propagate it to all connected viewers

    PARAMS:   

    RETURNS:  0 .. sucess
              -1 .. Server not found
              -2 .. error while contacting Server 
  */
  int propagate_inichange();

  /*
  +--------------------------------------------------------------------------------+
  | PROJECT : PCO2                       MODULE  : PCO_VIEW_CORE                   |
  | STATE   : code                       ROUTINE : PCOView_core::self_trace        |
  +--------------------------------------------------------------------------------+

    PURPOSE : adds a new trace string to the queue of this viewer
              (has to be implemented by derived classes !)

    PARAMS:   

  */
  virtual void self_trace(const char* trace)=0;

  /*
  +-------------------------------------------------------------------------------+
  | PROJECT : PCO2                       MODULE  : PCO_VIEW_CORE                  |
  | STATE   : code                       ROUTINE : PCOView_core::on_inichange     |
  +-------------------------------------------------------------------------------+

    PURPOSE : reloads important changes from ini-file

    PARAMS:   

  */
  virtual void on_inichange();

  bool  running() const {return m_running;}

  int dsize() const { return m_dsize;}
  int qsize() const { return m_qsize;}

  /*
  +-------------------------------------------------------------------------------+
  | PROJECT : PCO2                       MODULE  : PCO_VIEW_CORE                  |
  | STATE   : code                       ROUTINE : PCOView_core::shutdown         |
  +-------------------------------------------------------------------------------+

    PURPOSE : exits the viewer threads

    PARAMS:   

  */
  void shutdown();

  virtual IniFile&	inifile() { return m_inifile;}
  virtual const IniFile&	inifile() const { return m_inifile;}

protected:
  CMS_HANDLE	m_prim_handle,m_ctrl_handle;
  bool  m_running;

  IniFile         m_inifile;
  int             m_qsize, m_dsize;

  static  void cms_prim_proc(long view);
  static  void cms_ctrl_proc(long view);
};

#endif /* _PCO_VIEW_CORE_H_ */
