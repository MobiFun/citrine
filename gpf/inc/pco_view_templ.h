/* 
+----------------------------------------------------------------------------- 
|  Project :  PCO2
|  Modul   :  PCO_VIEW_TEMPLATE
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
|  Purpose :  This Modul contains a template of a PCO viewer class
|             with basic functions for several PCO control messages.
|             It can either be used just to connect to a PCO server or
|             can be specialized and extended like in PCOView_templ.
+----------------------------------------------------------------------------- 
*/ 

#ifndef _PCO_VIEW_TEMPL_H_
#define _PCO_VIEW_TEMPL_H_

/*==== INCLUDES ===================================================*/
#include "pco_const.h"

/*==== DEFINES =====================================================*/

/*==== PROTOTYPES ==================================================*/

/*==== CLASSES =====================================================*/
class PCOView_templ 
{
public:
  /*
  +-------------------------------------------------------------------------------+
  | PROJECT : PCO2                       MODULE  : PCO_VIEW_TEMPL                 |
  | STATE   : code                       ROUTINE : PCOView_templ::PCOView_templ   |
  +-------------------------------------------------------------------------------+

    PURPOSE : initilizes viewer process

    PARAMS:   primq .. name of primitive queue
              ctrlq .. name of control queue

    RETURNS:  
            
  */
  PCOView_templ(const char* primq_name="", const char* ctrlq_name="");
  virtual ~PCOView_templ();

  /*
  +-------------------------------------------------------------------------------+
  | PROJECT : PCO2                       MODULE  : PCO_VIEW_TEMPL                 |
  | STATE   : code                       ROUTINE : PCOView_templ::connect                    |
  +-------------------------------------------------------------------------------+

    PURPOSE : tries to connect with server

    PARAMS:   

    RETURNS:  0 .. sucess
              -1 .. Server not found
              -2 .. error while contacting Server 
            
  */
  int connect(void);

  /*
  +-------------------------------------------------------------------------------+
  | PROJECT : PCO2                       MODULE  : PCO_VIEW_TEMPL                 |
  | STATE   : code                       ROUTINE : PCOView_templ::subscribe       |
  +-------------------------------------------------------------------------------+

    PURPOSE : tries to subscribe for live data from server

    PARAMS:   mobileId .. name of mobile to receive live data from (may be empty)

    RETURNS:  0 .. sucess
              -1 .. Server not found
              -2 .. error while contacting Server 
            
  */
  int subscribe(const char* mobileId);

  /*
  +-------------------------------------------------------------------------------+
  | PROJECT : PCO2                       MODULE  : PCO_VIEW_TEMPL                 |
  | STATE   : code                       ROUTINE : PCOView_templ::send2srv        |
  +-------------------------------------------------------------------------------+

    PURPOSE : tries to send a data buffer to the server

    PARAMS:   buf ... pointer to buffer
              size .. size of buffer
              id  ... message id

    RETURNS:  0 .. sucess
              -1 .. Server not found
              -2 .. error while contacting Server 
            
  */
  int send2srv(void* buf, U16 size, U16 id);

  /*
  +-------------------------------------------------------------------------------+
  | PROJECT : PCO2                       MODULE  : PCO_VIEW_TEMPL                 |
  | STATE   : code                       ROUTINE : PCOView_templ::disconnect                 |
  +-------------------------------------------------------------------------------+

    PURPOSE : tries to disconnect from server

    PARAMS:   

    RETURNS:  0 .. sucess
              -1 .. Server not found
              -2 .. error while contacting Server 
            
  */
  int disconnect(void);

  /*
  +-------------------------------------------------------------------------------+
  | PROJECT : PCO2                       MODULE  : PCO_VIEW_TEMPL                 |
  | STATE   : code                       ROUTINE : PCOView_templ::unsubscribe     |
  +-------------------------------------------------------------------------------+

    PURPOSE : tries to unsubscribe from livedata stream from server

    PARAMS:   

    RETURNS:  0 .. sucess
              -1 .. Server not found
              -2 .. error while contacting Server 
            
  */
  int unsubscribe(void);

  /*
  +-------------------------------------------------------------------------------+
  | PROJECT : PCO2                       MODULE  : PCO_VIEW_TEMPL                 |
  | STATE   : code                       ROUTINE : PCOView_templ::on_connected    |
  +-------------------------------------------------------------------------------+

    PURPOSE : reaction to PCO_CONNECTED from server

    PARAMS:   buf   .. data containing server type
              sender .. server queue name

    RETURNS:   0 .. server type supported
              -1 .. server type not supported
            
  */
  virtual int on_connected(const void *buf,const char* sender);

  /*
  +---------------------------------------------------------------------------------+
  | PROJECT : PCO2                       MODULE  : PCO_VIEW_TEMPL                   |
  | STATE   : code                       ROUTINE : PCOView_templ::req_logfile_info  |
  +---------------------------------------------------------------------------------+

    PURPOSE : contacts server to request info about a specified logfile

    PARAMS:   fname .. name of logfile (can be full path)

    RETURNS:  0 .. sucess
              -1 .. Server not found
              -2 .. error while contacting Server 
            
  */
  int req_logfile_info(const char* fname);

  /*
  +-------------------------------------------------------------------------------+
  | PROJECT : PCO2                       MODULE  : PCO_VIEW_TEMPL                 |
  | STATE   : code                       ROUTINE : PCOView_templ::get_logdata     |
  +-------------------------------------------------------------------------------+

    PURPOSE : contacts server to request logged data

    PARAMS:   begin, end .. index area requested 

    RETURNS:  0 .. sucess
              -1 .. Server not found
              -2 .. error while contacting Server 
            
  */
  int get_logdata(ULONG begin, ULONG end);

  /*
  +-------------------------------------------------------------------------------+
  | PROJECT : PCO2                       MODULE  : PCO_VIEW_TEMPL                 |
  | STATE   : code                       ROUTINE : PCOView_templ::open_logfile    |
  +-------------------------------------------------------------------------------+

    PURPOSE : contacts server to request logged data from a specified logfile

    PARAMS:   fname .. name of logfile (full path)
              begin, end .. index area requested 

    RETURNS:  0 .. sucess
              -1 .. Server not found
              -2 .. error while contacting Server 
            
  */
  int open_logfile(const char* fname, ULONG begin=0, ULONG end=0);

  /*
  +-------------------------------------------------------------------------------+
  | PROJECT : PCO2                       MODULE  : PCO_VIEW_TEMPL                 |
  | STATE   : code                       ROUTINE : PCOView_templ::set_filter      |
  +-------------------------------------------------------------------------------+

    PURPOSE : contacts server to change filter settings

    PARAMS:   list ..     '\0'-separated list of names of entitities 
                          (terminated by "\0\0")
                          primitives/traces from entities in -list- 
                            should not be forwarded

              prim_trace  
                      .. parameter to disable/enable general forwarding of
                          primitives/traces:
                         0 .. fowarding of everything (default)
                         1 .. no primitives
                         2 .. no traces
              positiv_list 
                      .. if true, only primitives/traces from entities in -list- 
                         will be forwarded

    RETURNS:  0 .. sucess
              -1 .. Server not found
              -2 .. error while contacting Server 
          
  */
  int set_filter(const char* list, char prim_trace=0, bool positiv_list=false);

  /*
  +-------------------------------------------------------------------------------+
  | PROJECT : PCO2                       MODULE  : PCO_VIEW_TEMPL                 |
  | STATE   : code                       ROUTINE : PCOView_templ::self_send       |
  +-------------------------------------------------------------------------------+

    PURPOSE : tries to send a data buffer to the control queue of this viewer

    PARAMS:   buf ... pointer to buffer
              size .. size of buffer
              id  ... message id

    RETURNS:  0  .. sucess
              -1 .. receiver not found
              -2 .. error while contacting receiver 
            
  */
  int self_send(void* buf, U16 size, U16 id);

  const char *primq_name() { return m_primq_name;}
  const char *ctrlq_name() { return m_ctrlq_name;}

  /*
  +-------------------------------------------------------------------------------+
  | PROJECT : PCO2                       MODULE  : PCO_VIEW_TEMPL                 |
  | STATE   : code                       ROUTINE : PCOView_templ::start_logfile   |
  +-------------------------------------------------------------------------------+

    PURPOSE : contacts server to start logging data received in the given queue
              into the given logfile

    PARAMS:   logfile   .. name of the logfile 
                           (if no path is given the current server-path is used)
              queue     .. name of the queue the server shall receive data
                           (it will be created by the server)
                           (if no queue is given, m_ctrlq_name+"_LF" is used)

    RETURNS:  0  .. sucess
              -1 .. receiver not found
              -2 .. error while contacting receiver 
  */
  int start_logfile(const char* logfile, const char* queue=NULL);

  /*
  +-------------------------------------------------------------------------------+
  | PROJECT : PCO2                       MODULE  : PCO_VIEW_TEMPL                 |
  | STATE   : code                       ROUTINE : PCOView_templ::stop_logfile    |
  +-------------------------------------------------------------------------------+

    PURPOSE : contacts server to stop logging data received in the given queue

    PARAMS:   queue     .. name of the queue the server received data
                           (if no queue is given, m_ctrlq_name+"_LF" is used)

    RETURNS:  0  .. sucess
              -1 .. receiver not found
              -2 .. error while contacting receiver 
  */
  int stop_logfile(const char* queue=NULL);

  /*
  +-------------------------------------------------------------------------------+
  | PROJECT : PCO2                       MODULE  : PCO_VIEW_TEMPL                 |
  | STATE   : code                       ROUTINE : PCOView_templ::send_raw_data   |
  +-------------------------------------------------------------------------------+

    PURPOSE : sends raw data to the given queue

    PARAMS:   rawdata   .. the data
              rawsize   .. size of rawdata
              queue     .. name of the queue the data will be sent to
                           (if no queue is given, m_ctrlq_name+"_LF" is used)

    RETURNS:  0  .. sucess
              -1 .. receiver not found
              -2 .. error while contacting receiver 
  */
  int send_raw_data(const void* rawdata, U16 rawsize, const char* queue=NULL);

  /*
  +-------------------------------------------------------------------------------+
  | PROJECT : PCO2                       MODULE  : PCO_VIEW_TEMPL                 |
  | STATE   : code                       ROUTINE : PCOView_templ::set_srv_name    |
  +-------------------------------------------------------------------------------+

    PURPOSE : change the name of the server queue

    PARAMS:   sname ... new name of server queue
            
  */
  void set_srv_name(const char* sname);

  U8    srv_type() const { return m_srv_type;}

protected:
  char  m_primq_name[MAX_QNAME_LEN+1],m_ctrlq_name[MAX_QNAME_LEN+1];
  char  m_srvq_name[MAX_QNAME_LEN+1];
  U8    m_srv_type;
};

#endif /* _PCO_VIEW_TEMPL_H_ */
