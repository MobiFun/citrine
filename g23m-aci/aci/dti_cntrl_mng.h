/*
+-----------------------------------------------------------------------------
|  Project :  ...
|  Modul   :  dti_cntrl_mng.h
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
|  Purpose :  This modul ...
+-----------------------------------------------------------------------------
*/

/* #define DTI_CPBLTY_NO    0x00         used as initial value        */
/* #define DTI_CPBLTY_CMD   0x01         AT cmd capability            */
/* #define DTI_CPBLTY_PKT   0x02         packet capability            */
/* #define DTI_CPBLTY_SER   0x04         serial capability            */
                                                                      
#define DTI_MODE_DELETE  0x00         /* delete redirection mode      */
#define DTI_MODE_ONCE    0x01         /* redirection mode once        */
#define DTI_MODE_PERM    0x02         /* redirection mode permanent   */
#define DTI_MODE_NIL     0xFF         /* redirection mode reset       */

#define DTI_DEV_IS_REDIRECTED  0x00   /* data transmission is redirected to another device */
#define DTI_DEV_I_AM_THE_ONE   0x01   /* I am the device for the data transmission */

#define DTI_DIRECTION_NOTPRESENT 0xFF /* no direction available       */
#define DTI_SRC_ID_NOTPRESENT    0xFF /* non valid AT source Id       */
#define DTI_CID_NOTPRESENT       0xFF /* no PDP context id            */
#define DTI_PORT_NUMBER_NOTPRESENT 0xFF /* no port number (psa_aaa)     */
#define DTI_DRIVER_ID_NOTPRESENT 0xFF /* no driver id available       */
#define DTI_DIO_ID_NOTPRESENT 0xFF    /* no dio controlled combination id available       */


#define DTI_MAX_EXT_CB   3            /* max external call backs      */
                                      /* one cb for a external entity */
                                      /* so, customer can have up to  */
                                      /* 3 own (external) entities    */
#ifdef GPRS
#define DTI_MAX_REDIRECTIONS 3
#else
#define DTI_MAX_REDIRECTIONS 1
#endif


#define T_DTI_CNTRL struct dti_cntrl  /* need a forward definition    */

typedef struct
{
  UBYTE        mode;                  /* once/permanent               */
  T_DTI_CNTRL *redirection;           /* points to redirected device  */
} T_DTI_CNTRL_REDIRECT_INTERN_TYPE;

/*
 * index to [ser|par]_redirect is controlled by cid (PDP context)
 * cid = 0 --> no PDP context, cid = 1|2 --> there is a PDP WINWORD.EXEcontext
 * currently (Nov.2002) MAX_CID = 3
 */
typedef struct
{
  T_DTI_CNTRL_REDIRECT_INTERN_TYPE  ser_redirect[DTI_MAX_REDIRECTIONS]; /* redirect to serial device (related to a PDP context) */
  T_DTI_CNTRL_REDIRECT_INTERN_TYPE  pkt_redirect[DTI_MAX_REDIRECTIONS]; /* redirect to packet device (related to a PDP context) */
} T_DTI_CNTRL_REDIRECT_INTERN;

typedef struct
{
  UBYTE  cid;                         /* PDP context                  */
  UBYTE  mode;                        /* once/permanent               */
  UBYTE  capability;                  /* CMD,PKT,SER                  */
  UBYTE  direction;                   /* src|dst of redirection       */
} T_DTI_CNTRL_REDIRECT_EXTERN;

typedef union
{
  T_DTI_CNTRL_REDIRECT_INTERN *tbl;   /* intern                        */
  T_DTI_CNTRL_REDIRECT_EXTERN  info;  /* what the external sees        */
} T_DTI_CNTRL_REDIRECT;

/*
 * the main maintenance structure of the DTI Control Manager
 */
T_DTI_CNTRL
{
  T_DTI_ENTITY_ID       dev_id;        /* id(name) of device            */
  UBYTE                 dev_no;        /* instance of device            */
  UBYTE                 sub_no;        /* instance with multiplexed ch. */
  UBYTE                 capability;    /* capability of device          */
  UBYTE                 src_id;        /* what ACI sees as AT cmd src   */
  UBYTE                 dti_id;        /* id of the DTI channel         */
  UBYTE                 port_number;   /* used by psa_aaa               */
  UBYTE                 cur_cap;       /* capability of the DTI channel */
  UBYTE                 driver_id;     /* driver specific id like USB,...*/
  UBYTE                 dio_ctrl_id;   /* device combination id controlled by DIO Interface Layer, v4 */
  T_DTI_CNTRL_REDIRECT  redirect_info; /* union for redirection         */
#ifdef FF_TCP_IP
  UBYTE              had_aaa_dti_rsp ; /* AAA_DTI_RSP has been received. */
  /* When (pseudo-)connecting to AAA, the DTI connect request to the data
   * entity has to be delayed until the AAA_DTI_RSP has been received. Here we
   * save the parameters needed to call dti_cntrl_maintain_entity_connect()
   * again. */
  T_DTI_CONN_LINK_ID    save_link_id ;
  T_DTI_ENTITY_ID       save_cur_ent_id; /* entity to connect (peer is AAA) */
  UBYTE                 save_dti_conn ;
#endif /* FF_TCP_IP */
};


typedef BOOL T_DTI_EXT_CB (ULONG  link_id, T_DTI_ENTITY_ID  peer_entity_id, UBYTE  dti_conn); 

typedef struct
{
  T_DTI_ENTITY_ID  ent_id;
  T_DTI_EXT_CB    *fct;
} T_DTI_CNTRL_REGISTERED_EXT_CB;

/****************** DTI Control Manager Interface Functions ******************/

EXTERN void   dti_cntrl_init                     (void);

EXTERN UBYTE  dti_cntrl_new_dti                  (UBYTE               src_id);

EXTERN BOOL   dti_cntrl_new_device               (UBYTE               src_id,
                                                  UBYTE               dev_id, 
                                                  UBYTE               dev_no,
                                                  UBYTE               sub_no,
                                                  UBYTE               port_num,
                                                  UBYTE               capability,
                                                  UBYTE               driver_id,
                                                  UBYTE               dio_ctrl_id);
EXTERN void   dti_cntrl_change_sub_no            (UBYTE               src_id,
                                                  UBYTE               sub_no);

EXTERN BOOL   dti_cntrl_est_dpath                (UBYTE               dti_id,
                                                  T_DTI_ENTITY_ID    *entity_list,
                                                  UBYTE               num_entities,
                                                  T_DTI_CONN_MODE     mode,
                                                  T_DTI_CONN_CB      *cb);

EXTERN BOOL   dti_cntrl_est_dpath_indirect       (UBYTE               src_id,
                                                  T_DTI_ENTITY_ID    *entity_list,
                                                  UBYTE               num_entities,
                                                  T_DTI_CONN_MODE     mode,
                                                  T_DTI_CONN_CB      *cb,
                                                  UBYTE               capability,
                                                  UBYTE               cid);

EXTERN BOOL   dti_cntrl_is_dti_channel_connected (T_DTI_ENTITY_ID     ent_id, 
                                                  UBYTE               dti_id);

EXTERN BOOL   dti_cntrl_is_dti_channel_disconnected (UBYTE               dti_id);

EXTERN BOOL   dti_cntrl_close_dpath_from_src_id  (UBYTE               src_id);

EXTERN BOOL   dti_cntrl_close_dpath_from_dti_id  (UBYTE               dti_id);

EXTERN BOOL   dti_cntrl_get_info_from_src_id     (UBYTE               src_id,
                                                  T_DTI_CNTRL        *info);

EXTERN BOOL   dti_cntrl_get_info_from_dti_id     (UBYTE               dti_id,
                                                  T_DTI_CNTRL        *info);

EXTERN BOOL   dti_cntrl_get_info_from_dev_id     (T_DTI_ENTITY_ID     dev_id,
                                                  UBYTE               dev_no,
                                                  UBYTE               sub_no,
                                                  T_DTI_CNTRL        *info);

EXTERN BOOL   dti_cntrl_set_redirect_from_src    (UBYTE               src_id,
                                                  UBYTE               mode,
                                                  T_DTI_ENTITY_ID     dst_dev_id,
                                                  UBYTE               dst_dev_no,
                                                  UBYTE               dst_sub_no,
                                                  UBYTE               capability,
                                                  UBYTE               cid);

EXTERN BOOL   dti_cntrl_set_redirect_from_device (UBYTE               mode,
                                                  T_DTI_ENTITY_ID     dst_dev_id,
                                                  UBYTE               dst_dev_no,
                                                  UBYTE               dst_sub_no,
                                                  T_DTI_ENTITY_ID     src_dev_id,
                                                  UBYTE               src_dev_no,
                                                  UBYTE               src_sub_no,
                                                  UBYTE               capability,
                                                  UBYTE               cid);

EXTERN BOOL   dti_cntrl_get_first_device         (T_DTI_CNTRL  *info);
EXTERN BOOL   dti_cntrl_get_next_device (T_DTI_CNTRL  *info);
EXTERN BOOL   dti_cntrl_get_first_redirection    (UBYTE               src_id,
                                                  UBYTE               capability,
                                                  T_DTI_CNTRL        *redirection);

EXTERN BOOL   dti_cntrl_get_next_redirection     (UBYTE               src_id,
                                                  UBYTE               cid,
                                                  UBYTE               capability,
                                                  T_DTI_CNTRL        *redirection);

EXTERN void   dti_cntrl_entity_connected         (ULONG               link_id,
                                                  T_DTI_ENTITY_ID     entity_id,
                                                  T_DTI_CONN_RESULT   result);

EXTERN void   dti_cntrl_entity_disconnected      (ULONG               link_id,
                                                  T_DTI_ENTITY_ID     entity_id);

EXTERN BOOL   dti_cntrl_reg_new_fct              (T_DTI_ENTITY_ID     entity_id,
                                                  T_DTI_EXT_CB       *fct);

EXTERN void   dti_cntrl_erase_entry              (UBYTE               dti_id);


EXTERN BOOL   dti_cntrl_search_dti_id            (UBYTE dti_id, void *elem);

EXTERN void   dti_cntrl_close_all_connections    (void);

EXTERN void   dti_cntrl_connect_after_aaa_dti_rsp(UBYTE dti_id) ;


EXTERN void dti_cntrl_set_dti_id_to_reconnect    (UBYTE dti_id);

EXTERN void dti_cntrl_clear_dti_id_to_reconnect  (UBYTE dti_id);

EXTERN BOOL dti_cntrl_is_dti_id_to_reconnect     (UBYTE dti_id);

/*
 * special temporary stuff
 */
#define DTI_DEV_NO_NOTPRESENT NOT_PRESENT_8BIT

#define DTI_SUB_NO_NOTPRESENT NOT_PRESENT_8BIT

#define DTI_INSTANCE_NOTPRESENT NOT_PRESENT_8BIT


typedef struct
{
  T_DTI_CONN_LINK_ID link_id;
  T_DTI_ENTITY_ID    ent_id1;
  T_DTI_ENTITY_ID    ent_id2;
  UBYTE              dev_no1;
  UBYTE              dev_no2;
  UBYTE              sub_no1;
  UBYTE              sub_no2;
} T_DTI_CNTRL_DATA;


#ifdef DTI_CNTRL_MNG_C
T_DTI_CNTRL_DATA dti_aci_data_base[MAX_DTI_CONN_LINK_IDS];
#else
EXTERN T_DTI_CNTRL_DATA dti_aci_data_base[];
#endif

EXTERN T_DTI_CONN_LINK_ID dti_cntrl_get_link_id( T_DTI_ENTITY_ID ent_id, 
                                                 UBYTE           dev_no, 
                                                 UBYTE           sub_no );


EXTERN T_DTI_ENTITY_ID dti_cntrl_get_peer( T_DTI_ENTITY_ID ent_id,
                                   UBYTE           dev_no, 
                                   UBYTE           sub_no );


EXTERN BOOL dti_cntrl_set_conn_parms( T_DTI_CONN_LINK_ID link_id, 
                                      T_DTI_ENTITY_ID    ent_id, 
                                      UBYTE              dev_no,
                                      UBYTE              sub_no );


EXTERN void dti_cntrl_clear_conn_parms( UBYTE dti_id );


