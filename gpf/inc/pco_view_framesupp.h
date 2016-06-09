/* 
+----------------------------------------------------------------------------- 
|  Project :  PCO2
|  Modul   :  PCO_VIEW_FRAMESUPP
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
|  Purpose :  This Modul contains a viewer class derived directly from the
|             core.
|             It supports decoding of frame SYST_MESSAGEs and of LTS-Header/
|             TIF-ASCII-DATA-messages.
|             It is still intended to be specialized and extended, e.g. in 
|             GUI-server-applications.
|             Some member functions are totally virtual without any 
|             standard body and have to be provided by the child class !
+----------------------------------------------------------------------------- 
*/ 

#ifndef _PCO_VIEW_FRAMESUPP_H_
#define _PCO_VIEW_FRAMESUPP_H_

/*==== INCLUDES ===================================================*/
#ifndef PCO_VIEW_FRAMESUPP_CPP_CCD
#include "pco_view_core.h"
#else
#include "view/pco_view_core.h"
#endif

#ifdef USE_CCD
extern "C" 
{
#include "ccdtable.h"
#include "ccddata.h"
}
#include "ccdedit.h"
#include "view/pco_pdi.h"
#endif /* USE_CCD */

#include <time.h>

/*==== DEFINES =====================================================*/
#define PCO_STR2IND_NOT_LOADED  0
#define PCO_STR2IND_AUTO_LOADED 1
#define PCO_STR2IND_MAN_LOADED  2
#define PCO_STR2IND_SEARCHING   3

/*==== PROTOTYPES ==================================================*/

/*==== CLASSES =====================================================*/
class PCOView_frameSupp : public PCOView_core
{
public:
  PCOView_frameSupp(const char* ini_file, int& err,
                    const char* primq_name="", 
                    const char* ctrlq_name="");
  virtual ~PCOView_frameSupp();

  /*
  +-------------------------------------------------------------------------------------+
  | PROJECT : PCO2                       MODULE  : PCO_VIEW_FRAMESUPP                   |
  | STATE   : code                       ROUTINE : PCOView_frameSupp::interpret_message |
  +-------------------------------------------------------------------------------------+

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
    char* &sender, char* &receiver);

  /*
  +--------------------------------------------------------------------------------+
  | PROJECT : PCO2                       MODULE  : PCO_VIEW_FRAMESUPP              |
  | STATE   : code                       ROUTINE : PCOView_frameSupp::on_connected |
  +--------------------------------------------------------------------------------+

    PURPOSE : reaction to PCO_CONNECTED from server

    PARAMS:   buf   .. data containing server type
              sender .. server queue name

    RETURNS:   0 .. server type supported
              -1 .. server type not supported
            
  */
  virtual int on_connected(const void *buf,const char* sender);

  /*
  +--------------------------------------------------------------------------------------+
  | PROJECT : PCO2                       MODULE  : PCO_VIEW_FRAMESUPP                    |
  | STATE   : code                       ROUTINE : PCOView_frameSupp::decode_tracestring |
  +--------------------------------------------------------------------------------------+

    PURPOSE : tries to decode an OPC in the tracestring (e.g. $<OPC>)

    PARAMS:   instr  .. original tracestring
              size   .. max size for outstr

    RETURNS:   0 .. success (outstr contains new tracestring)
              -1 .. no success while decoding
              -2 .. no decoding necessary

              lastOPC .. in case of success contains last opc found in instr
            
  */
  int decode_tracestring(const char* instr, char* outstr, U16 size, ULONG &lastOPC);

  /*
  +-----------------------------------------------------------------------------------+
  | PROJECT : PCO2                       MODULE  : PCO_CTRL_FRAMESUPP                 |
  | STATE   : code                       ROUTINE : PCOView_frameSupp::check_version   |
  +-----------------------------------------------------------------------------------+

    PURPOSE : tries to request the version of a running stack via the server
              -> will result in a SYST-trace starting with '&'

    PARAMS:   

    RETURNS:  0 .. sucess
              -1 .. Server not found
              -2 .. error while contacting Server 
          
  */
  int check_version();

  /*
  +--------------------------------------------------------------------------------------+
  | PROJECT : PCO2                       MODULE  : PCO_CTRL_FRAMESUPP                    |
  | STATE   : code                       ROUTINE : PCOView_frameSupp::check_communication|
  +--------------------------------------------------------------------------------------+

    PURPOSE : tries to send a system primitive to the running stack via the server
              -> will result in SYST-traces if communication is ok

    PARAMS:   

    RETURNS:  0 .. sucess
              -1 .. Server not found
              -2 .. error while contacting Server 
          
  */
  int check_communication();

  /*
  +--------------------------------------------------------------------------------------+
  | PROJECT : PCO2                       MODULE  : PCO_VIEW_FRAMESUPP                    |
  | STATE   : code                       ROUTINE : PCOView_frameSupp::decode_ccd_msg     |
  +--------------------------------------------------------------------------------------+

    PURPOSE : tries to decode a given encoded air message

    PARAMS:   decinfo     ... T_PDI_CCDMSG stuct (see pdi.h),
                              contains the coded sdu and info about, e.g., msg-type
              decoded_msg ... buffer which will receive the decoded struct

    RETURNS:  ccdOK, ccdWarning .. sucess
              ccdError          .. error while decoding
        
  */
  int decode_ccd_msg(const void* decinfo, UBYTE **decoded_msg);

  /*
  +------------------------------------------------------------------------------------+
  | PROJECT : PCO2                       MODULE  : PCO_VIEW_FRAMESUPP                  |
  | STATE   : code                       ROUTINE : PCOView_frameSupp::free_ccd_msg     |
  +------------------------------------------------------------------------------------+

    PURPOSE : frees a msg structure previously allocated using decode_ccd_msg
    
    PARAMS:   decoded_msg ... buffer with decoded air message structure

    RETURNS:  

  */
  void free_ccd_msg(void * decoded_msg);

  /*
  +--------------------------------------------------------------------------------------+
  | PROJECT : PCO2                       MODULE  : PCO_VIEW_FRAMESUPP                    |
  | STATE   : code                       ROUTINE : PCOView_frameSupp::load_str2ind_table |
  +--------------------------------------------------------------------------------------+

    PURPOSE : tries to explicitely load a str2ind-table

    PARAMS:   tname ... name of .tab-file

    RETURNS:  0  .. sucess
              -1 .. error while loading
        
  */
  int load_str2ind_table(const char* tname);

  /*
  +--------------------------------------------------------------------------------+
  | PROJECT : PCO2                       MODULE  : PCO_VIEW_FRAMESUPP              |
  | STATE   : code                       ROUTINE : PCOView_frameSupp::on_inichange |
  +--------------------------------------------------------------------------------+

    PURPOSE : reloads important changes from ini-file

    PARAMS:   

  */
  virtual void on_inichange();

  /*
  +--------------------------------------------------------------------------------+
  | PROJECT : PCO2                       MODULE  : PCO_VIEW_FRAMESUPP              |
  | STATE   : code                       ROUTINE : PCOView_frameSupp::self_trace   |
  +--------------------------------------------------------------------------------+

    PURPOSE : adds a new trace string to the queue of this viewer

    PARAMS:   

  */
  virtual void self_trace(const char* trace);

  /*
  +--------------------------------------------------------------------------------+
  | PROJECT : PCO2                       MODULE  : PCO_VIEW_FRAMESUPP              |
  | STATE   : code                       ROUTINE : PCOView_frameSupp::send_syscmd  |
  +--------------------------------------------------------------------------------+

    PURPOSE : tries to send a FRAME system command to the server 
              who will forward it to a connected protocol stack
    
    PARAMS:   receiver ... receiver of the command
              cmd      ... the actual command (e.g. "TRACECLASS FF") 

    RETURNS:  0 .. sucess
              -1 .. Server not found
              -2 .. error while contacting Server 

  */
  int send_syscmd(const char* receiver, const char* cmd);

  /*
  +--------------------------------------------------------------------------------+
  | PROJECT : PCO2                       MODULE  : PCO_VIEW_FRAMESUPP              |
  | STATE   : code                       ROUTINE : PCOView_frameSupp::init_ccddata |
  +--------------------------------------------------------------------------------+

    PURPOSE : initialize ccddata from the given path

    PARAMS:   ccddata_path ... path of the ccddata library
                               (if NULL path will be loaded from ini-file)
              force        ... 0..keep ccddata-lib loaded if there is one
                               1..exchange evtl. existing ccddata-lib 
              write        ... 1..write new path to ini file if force is 1
                               0..dont write new path to ini file 

    RETURNS:  0 .. success
             -1 .. error

  */
  int init_ccddata(const char* ccddata_path=NULL, 
                   int force=0, int write=1);

  /*
  +--------------------------------------------------------------------------------+
  | PROJECT : PCO2                       MODULE  : PCO_VIEW_FRAMESUPP              |
  | STATE   : code                       ROUTINE : PCOView_frameSupp::exit_ccddata |
  +--------------------------------------------------------------------------------+

    PURPOSE : deinitializied ccddata 

    PARAMS:   

    RETURNS:  0 .. success
             -1 .. error

  */
  int exit_ccddata();

  /*
  +------------------------------------------------------------------------------------+
  | PROJECT : PCO2                       MODULE  : PCO_VIEW_FRAMESUPP                  |
  | STATE   : code                       ROUTINE : PCOView_frameSupp::collect_pdi_data |
  +------------------------------------------------------------------------------------+

    PURPOSE : run through primitive top-level elements and call pdi_getDecodeInfo to 
              collect all potential infos for decoding an evtl. contained aim

    PARAMS:   prim_data        ... data of primitive
              filterinfo_data  ... data that holds filter info for the primitive
              handle           ... open handle for parsing the primitive

    RETURNS:  

  */
  void collect_pdi_data(const char* prim_data, const char* filterinfo_data, T_CCDE_HANDLE handle);

  /*
  +--------------------------------------------------------------------------------+
  | PROJECT : PCO2                       MODULE  : PCO_VIEW_FRAMESUPP              |
  | STATE   : code                       ROUTINE : PCOView_frameSupp::elem_present |
  +--------------------------------------------------------------------------------+

    PURPOSE : check if element is present in specified primtive

    PARAMS:   filterinfo_data  ... data that holds filter info for the primitive
              filterinfo_value ... buffer to receive filter info data for current element
              handle           ... open handle for parsing the primitive
              desc             ... current descriptor for parsing the primitive

    RETURNS:  1  ... present
              0  ... not present
              -1 ... error while calling read_elem

  */
  int elem_present(const char* filterinfo_data, char* filterinfo_value, T_CCDE_HANDLE handle, T_CCDE_ELEM_DESCR desc);

  /*
  +------------------------------------------------------------------------------------+
  | PROJECT : PCO2                       MODULE  : PCO_VIEW_FRAMESUPP                  |
  | STATE   : code                       ROUTINE : PCOView_frameSupp::decode_pcon_prim |
  +------------------------------------------------------------------------------------+

    PURPOSE : if necessary tries to decode the given primitive using PCON
    
    PARAMS:   opc          ... primitive opc
              coded_prim   ... data of original prim
              decoded_prim ... buffer which will point to the decoded prim
              decoded_len  ... length of decoded_prim buffer (without evtl. pointed to data areas)
              filterinfo_prim
                           ... buffer which will be evtl. filled with a filterinfo shadow prim
                               containing only 0x00-s or 0xff-s per element
                               (in default case a NULL pointer is returned)
              errstr       ... will be filled with an error string in case of PCON error 
                               (if NULL, no string will be written, 
                                otherwise a buffer with min 500 bytes has to be provided!)

    RETURNS:  PCON_OK       .. PCON sucessfully applied
              PCON_NOT_PRES .. PCON was not necessary
              PCON_XXX      .. error occured during PCON operation (see constants in pcon.h) 

  */
  int decode_pcon_prim(ULONG opc, void * coded_prim, void ** decoded_prim, ULONG * decoded_len,
                       void ** filterinfo_prim, char* errstr=NULL);

  /*
  +------------------------------------------------------------------------------------+
  | PROJECT : PCO2                       MODULE  : PCO_VIEW_FRAMESUPP                  |
  | STATE   : code                       ROUTINE : PCOView_frameSupp::free_pcon_prim   |
  +------------------------------------------------------------------------------------+

    PURPOSE : frees a primitive previously allocated using decode_pcon_prim
    
    PARAMS:   decoded_prim ... buffer with decoded prim

    RETURNS:  

  */
  void free_pcon_prim(void * decoded_prim);

  int pcon() const { return m_pcon;}
  int set_pcon(int pcon);

  bool ccddata_loaded(const char* ccddata_path=NULL); 
  const char* ccddata_path() const;

  const char* ind2str_dir() const { return m_ind2str_dir;}
  void set_ind2str_dir(const char* dir);

  int ind2str_loaded() const { return m_ind2str_loaded;}
  int unload_str2ind_table();
  void cancel_str2ind_search() { m_str2ind_continue_search=false;}

  long ind2str_version() const { return m_ind2str_version;}
  const char* ind2str_file() const { return m_ind2str_file;}

  static long read_elem_save (T_CCDE_HANDLE     * handle,
                              void              * cstruct,
                              T_CCDE_ELEM_DESCR * edescr,
                              UBYTE             * value);

  static USHORT prim_first_save (T_CCDE_HANDLE      * phandle,
                                 ULONG                primcode,
                                 char               * name);

  static USHORT prim_next_save (T_CCDE_HANDLE      *phandle,
                                UBYTE               descent,
                                T_CCDE_ELEM_DESCR  *pdescr);

protected:
	time_t  m_str2ind_time;
  bool    m_show_str2ind_errors;
	char    m_ind2str_dir[MAX_PATH+1];
  int     m_ind2str_loaded;
  char    m_ind2str_file[MAX_PATH+1];
  long    m_ind2str_version;
  bool    m_ccddata_attached;
  char    m_tracestr_buf[DATA_MSG_MAX_SIZE+1];
  bool    m_str2ind_continue_search;
  int     m_pcon;

  void  (*m_ccddata_init_notify)(void); 
  void  (*m_ccddata_exit_notify)(void); 
};

#endif /* _PCO_VIEW_FRAMESUPP_H_ */
