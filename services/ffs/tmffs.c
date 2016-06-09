/******************************************************************************
 * Flash File System (ffs)
 * Idea, design and coding by Mads Meisner-Jensen, mmj@ti.com
 *
 * ffs testmode interface
 *
 * $Id: tmffs.c 1.51 Thu, 18 Dec 2003 10:50:52 +0100 tsj $
 *
 ******************************************************************************/

#include "../../include/config.h"
#include "../../riviera/rv/rv_defined_swe.h"

#ifdef RVM_ETM_SWE
#include "../etm/etm.h"
#include "../etm/etm_api.h"
#endif

#include "ffs.h"
#include "task.h"
#include "ffstrace.h"
#include "tmffs.h"

#include <string.h>

/******************************************************************************
 * Local globals for all protocols
 ******************************************************************************/

#if TMFFS1 || TMFFS2
static int32 bufsize, tmpsize;
static uint8 stringsize;
#endif

effs_t ffs_initialize(void);
effs_t ffs_exit(void);

#define tmffs_put8(x)   *outp++ = x;
#define tmffs_put16(x) *outp++ = (x & 0xff); *outp++ = (x>>8);

// Not in use
//#define tmffs_put32(x) tmffs_put16(x); tmffs_put16(x >> 16);

#ifdef RVM_ETM_SWE

int etm_ffs2(T_ETM_PKT *pkt, unsigned char *inp, int insize);

/******************************************************************************
 * TM FFS registration to ETM database
 *****************************************************************************/
/* Callback function registered in ETM database */
int etm_ffs1_pkt_receive(uint8 *data, int size)
{
    int mid;
    T_ETM_PKT *pkt;  

    ttw(ttr(TTrTmffs, "etm_ffs1_pkt_receive(*, %d)" NL, size)); 
    
    /* Create TestMode return Packet */
    if ((pkt = (T_ETM_PKT *) target_malloc(sizeof(T_ETM_PKT))) == NULL) {
        ttw(ttr(TTrTmffs, "etm_ffs1_pkt_receive(): Limit of memory bank reached" NL)); 
        return ETM_NOMEM;
    }
 
    // Max packet size for TM3 is 128 bytes
    size = tm_ffs(pkt->data, TM3_PACKET_SIZE, data, size);

    pkt->size   = size;
    pkt->status = ETM_OK;
    pkt->mid    = ETM_FFS1;
    
    etm_pkt_send(pkt);
    target_free(pkt);

    return ETM_OK;
}

/* Callback function registered in ETM database */
int etm_ffs2_pkt_receive(uint8 *data, int size)
{
    int status;
    T_ETM_PKT *pkt = NULL;  

    ttw(ttr(TTrTmffs, "etm_ffs2_pkt_receive(*, %d)" NL, size)); 

    /* Create TestMode return Packet */
    if ((pkt = (T_ETM_PKT *) target_malloc(sizeof(T_ETM_PKT))) == NULL) {
        ttw(ttr(TTrTmffs, "etm_ffs2_pkt_receive(): Limit of memory bank reached" NL)); 
        return ETM_NOMEM;
    } 
    
    status = etm_ffs2(pkt, data, size);
    return status;
}
#endif

/* Init of FFS in the ETM database */
int etm_ffs_init(void)
{
    int status;

#ifdef RVM_ETM_SWE
    status = etm_register("FFS1", ETM_FFS1, 0, 0, etm_ffs1_pkt_receive);
    status = etm_register("FFS2", ETM_FFS2, 0, 0, etm_ffs2_pkt_receive);
#else
    status = 0;
#endif
    return status;
} 

/******************************************************************************
 * FFS1 Protocol 
 ******************************************************************************/

#ifndef TMFFS1 

int tm_ffs(unsigned char *outp, int outsize, unsigned char *inp, int insize)
{
	return -1;   // FIXME handle error better 
}

// Note these functions must be presented because ffs_query() use them but
// they are only valid if FFS1_PROTOCOL is used.
int tmffs_bufsize(void)
{
    return EFFS_NOSYS;
}

unsigned char *tmffs_bufaddr(void)
{
    return 0;
}

#else

#if (GSMLITE == 1)
#define TMFFS1_BUFFER_SIZE 4000 //previously 8192
#else
#define TMFFS1_BUFFER_SIZE 8192
#endif

#define TMFFS1_STRING_SIZE 127

/******************************************************************************
 * Macros
 ******************************************************************************/

#define tmffs1_putdata(outp, src, size) \
    tmffs_put8(FPI_DATA); \
    tmffs_put16(size); \
    memcpy(outp, src, size); \
    outp += size;

/******************************************************************************
 * Local globals
 ******************************************************************************/

static unsigned char buffer[TMFFS1_BUFFER_SIZE];
static bufindex;

static char string[TMFFS1_STRING_SIZE];

static effs_t tm_ffs_overflowck(void)
{
    if (bufsize    > TMFFS1_BUFFER_SIZE ||
        stringsize > TMFFS1_STRING_SIZE)
        return EFFS_TOOBIG;

    return EFFS_OK;
}


/******************************************************************************
 * tm_ffs
 ******************************************************************************/

/**
 * NOTEME: This has been introduced when the ffs 1MB device limit was
 * broken. This made location_t go from uint16 to uint32, messing up
 * with PCTM.  
 *
 * This makes the xstat_s look the same to PCTM PC side, though
 * location will be forced to 0.
 */
void hack_xstat_2_look_like_old_xstat(struct xstat_s *xstat)
{
  int i;
  char *location;

  xstat->location = 0;
  
  for (location = (char *) &(xstat->location) + 2; location <= (char *) &(xstat->sequence); location++)
    *location = location[2];
}

// Parse input message and execute function. Then fill output buffer with
// return values from the called function and transmit the message.  Return
// number of bytes inserted into output buffer. If return value is negative,
// it represents an error code.
int tm_ffs(unsigned char *outp, int outsize, unsigned char *inp, int insize)
{
    int error;
    tmffs_cid_t fid;

    unsigned char *outp_start = outp;
    unsigned char *inp_start  = inp;

    static uint8   i8[2]; static uint16 i8i;
    static uint16 i16[2]; static uint16 i16i;
    static uint32 i32[2]; static uint16 i32i;

    tw(tr(TR_BEGIN, TrTmffs, "TMFFS:\n"));

    while((fid = *inp++) != FPI_END)
    {
        switch(fid)
        {
            /**********************************************************
             * Generic Protocol Functions
             **********************************************************/

        case FPI_BEGIN:
            // for (i8i = 0; i8i < TMFFS1_STRING_SIZE; i8i++) // DEBUG
            //     string[i8i] = '#';
            // for (i8i = 0; i8i < TMFFS1_BUFFER_SIZE; i8i++) // DEBUG
            //     buffer[i8i] = '$';
            i8i = i16i = i32i = bufsize = stringsize = 0;
            bufindex = 0;
             i8[0] =  i8[1] = 0;
            i16[0] = i16[1] = 0;
            i32[0] = i32[1] = 0;
            string[0] = buffer[0] = 0;
            tw(tr(TR_FUNC, TrTmffs, "FPI_BEGIN\n"));
            ttw(ttr(TTrTmffs, "tm1" NL));
            break;
        case FPI_TMFFS_VERSION:
            // NULL -> UINT16
            tmffs_put16(TMFFS1_VERSION);
            break;

        case FPI_INT8:
            i8[i8i++] = inp[0]; inp += 1;
            tw(tr(TR_FUNC, TrTmffs, "FPI_INT8(%d/0x%x)\n",
                  i8[i8i-1], i8[i8i-1]));
            ttw(ttr(TTrTmffs, "tm_i8" NL));
            break;
        case FPI_INT16:
            i16[i16i++] = (inp[0]) | (inp[1] << 8); inp += 2;
            tw(tr(TR_FUNC, TrTmffs, "FPI_INT16(%d/0x%x)\n",
                  i16[i16i-1], i16[i16i-1]));
            ttw(ttr(TTrTmffs, "tm_i16" NL));
            break;
        case FPI_INT32:
            i32[i32i++] = inp[0] | (inp[1] << 8)
                | (inp[2] << 16) | (inp[3] << 24);
            inp += 4;
            tw(tr(TR_FUNC, TrTmffs, "FPI_INT32(%d/0x%x)\n",
                  i32[i32i-1], i32[i32i-1]));
            ttw(ttr(TTrTmffs, "tm_i32" NL));
            break;
        case FPI_BUFFER:
            bufsize = inp[0] | (inp[1] << 8); inp += 2;
            tw(tr(TR_FUNC, TrTmffs, "FPI_BUFFER(%d)\n", bufsize));
            ttw(ttr(TTrTmffs, "tm_buf" NL));
            break;
        case FPI_DATA:
            bufsize = inp[0] | (inp[1] << 8); inp += 2;
            memcpy(buffer, inp, bufsize); inp += bufsize;
            tw(tr(TR_FUNC, TrTmffs, "FPI_DATA(%d)\n", bufsize));
            ttw(ttr(TTrTmffs, "tm_data" NL));
            break;
        case FPI_STRBUF:
            // string buffer size MUST include null-terminator!
            stringsize = inp[0]; inp += 1;
            tw(tr(TR_FUNC, TrTmffs, "FPI_STRBUF(%d)\n", stringsize));
            ttw(ttr(TTrTmffs, "tm_sbuf" NL));
            break;
        case FPI_STRING:
            // stringsize MUST include null-terminator!
            // <INT8>, <BYTES> -> NULL (or ERROR)
            stringsize = inp[0]; inp += 1;
            if (stringsize <= TMFFS1_STRING_SIZE)
                memcpy(string, inp, stringsize);
            inp += stringsize;
            tw(tr(TR_FUNC, TrTmffs, "FPI_STRING(%d,'%s')\n",
                  stringsize, string));
            ttw(ttr(TTrTmffs, "tm_s" NL));
            break;

        case FPI_BUFREAD:
            // <INT16> -> DATA
            tmpsize = inp[0] | (inp[1] << 8); inp += 2;
            tw(tr(TR_FUNC, TrTmffs, "FPI_BUF_READ(%d)\n", tmpsize));
            tmffs1_putdata(outp, &buffer[bufindex], tmpsize);
            bufindex += tmpsize;
            ttw(ttr(TTrTmffs, "tm_bufrd" NL));
            break;
        case FPI_BUFWRITE:
            // <INT16>, <BYTES> -> NULL (or ERROR)
            tmpsize = inp[0] | (inp[1] << 8); inp += 2;
            tw(tr(TR_FUNC, TrTmffs, "FPI_BUF_WRITE(%d)\n", tmpsize));
            if (bufsize + tmpsize <= TMFFS1_BUFFER_SIZE)
                memcpy(&buffer[bufsize], inp, tmpsize);
            inp += tmpsize;
            bufsize += tmpsize;
            ttw(ttr(TTrTmffs, "tm_bufwr" NL));
            break;
        case FPI_BUFSET:
            bufindex = inp[0] | (inp[1] << 8); inp += 2;
            tw(tr(TR_FUNC, TrTmffs, "FPI_BUF_SET(%d)\n", bufindex));
            ttw(ttr(TTrTmffs, "tm_bufset" NL));
            break;

            /**********************************************************
             * FFS Functions
             **********************************************************/

        case FPI_PREFORMAT:
            // NULL -> ERROR
            if ((error = tm_ffs_overflowck()) == EFFS_OK)
                error = ffs_preformat_nb(i16[0], 0);
            if (error > 0) 
                error = 0;  // ignore request id
            tmffs_put8(error);
            tw(tr(TR_FUNC, TrTmffs, "FPI_PREFORMAT(0x%x)\n", i16[0]));
            ttw(ttr(TTrTmffs, "tm_pfmt" NL));
            break;
        case FPI_FORMAT:
            // STRING -> ERROR
            if ((error = tm_ffs_overflowck()) == EFFS_OK)
                error = ffs_format_nb(&string[0], i16[0], 0);
            if (error > 0) 
                error = 0;  // ignore request id
            tmffs_put8(error);
            tw(tr(TR_FUNC, TrTmffs, "FPI_FORMAT(0x%x)\n", i16[0]));
            ttw(ttr(TTrTmffs, "tm_fmt" NL));
            break;


        case FPI_FCREATE:
            // STRING, DATA -> ERROR
            if ((error = tm_ffs_overflowck()) == EFFS_OK)
                error = ffs_fcreate_nb(string, buffer, bufsize, 0);
            if (error > 0) 
                error = 0;  // ignore request id
            tmffs_put8(error);
            tw(tr(TR_FUNC, TrTmffs, "FPI_FCREATE('%s', 0x%x, %d/0x%x)\n",
                  string, buffer, bufsize, bufsize));
            ttw(ttr(TTrTmffs, "tm_fcr" NL));
            break;
        case FPI_FUPDATE:
            // STRING, DATA -> ERROR
            if ((error = tm_ffs_overflowck()) == EFFS_OK)
                error = ffs_fupdate_nb(string, buffer, bufsize, 0);
            if (error > 0) 
                error = 0;  // ignore request id
            tmffs_put8(error);
            tw(tr(TR_FUNC, TrTmffs, "FPI_FUPDATE('%s', 0x%x, %d/0x%x)\n",
                  string, buffer, bufsize, bufsize));
            ttw(ttr(TTrTmffs, "tm_fup" NL));
            break;
        case FPI_FWRITE:
            // STRING, DATA -> ERROR
            if ((error = tm_ffs_overflowck()) == EFFS_OK)
                error = ffs_fwrite_nb(string, buffer, bufsize, 0);
            if (error > 0) 
                error = 0;  // ignore request id
            tmffs_put8(error);
            tw(tr(TR_FUNC, TrTmffs, "FPI_FWRITE('%s', 0x%x, %d/0x%x)\n",
                  string, buffer, bufsize, bufsize));
            ttw(ttr(TTrTmffs, "tm_fwr" NL));
            break;
        case FPI_FREAD:
            // STRING, BUFFER -> ERROR
            if ((error = tm_ffs_overflowck()) == EFFS_OK)
                error = ffs_file_read(string, buffer, TMFFS1_BUFFER_SIZE);
            // Because a 32-bit integer is returned, we have to saturate it
            // into an 8-bit value.
            if (error >= 0)
                error = 0;
            tmffs_put8(error);
            tw(tr(TR_FUNC, TrTmffs, "FPI_FREAD('%s', 0x%x, %d/0x%x)\n",
                  string, buffer, bufsize, bufsize));
            ttw(ttr(TTrTmffs, "tm_frd" NL));
            break;
        case FPI_REMOVE:
            // STRING -> ERROR
            if ((error = tm_ffs_overflowck()) == EFFS_OK)
                error = ffs_remove_nb(string, 0);
            if (error > 0) 
                error = 0;  // ignore request id
            tmffs_put8(error);
            tw(tr(TR_FUNC, TrTmffs, "FPI_REMOVE()\n"));
            ttw(ttr(TTrTmffs, "tm_rm" NL));
            break;


        case FPI_MKDIR:
            // STRING -> ERROR
            if ((error = tm_ffs_overflowck()) == EFFS_OK)
                error = ffs_mkdir_nb(string, 0);
            if (error > 0) 
                error = 0;  // ignore request id
            tmffs_put8(error);
            tw(tr(TR_FUNC, TrTmffs, "FPI_MKDIR()\n"));
            ttw(ttr(TTrTmffs, "tm_mkd" NL));
            break;
        case FPI_OPENDIR:
            // STRING, BUFFER -> ERROR, DATA
            if ((error = tm_ffs_overflowck()) == EFFS_OK)
                error = ffs_opendir(string, (struct dir_s *) buffer);
            // Because a 32-bit integer is returned, we have to saturate it
            // into an 8-bit value.
            if (error >= 0)
                error = 0;
            tmffs_put8(error);
            tmffs1_putdata(outp, buffer, sizeof(struct dir_s));
            tw(tr(TR_FUNC, TrTmffs, "FPI_OPENDIR()\n"));
            ttw(ttr(TTrTmffs, "tm_od" NL));
            break;
        case FPI_READDIR:
            // DATA, STRBUF -> ERROR, DATA, STRING
            string[0] = 0;
            if ((error = tm_ffs_overflowck()) == EFFS_OK)
                error = ffs_readdir((struct dir_s *) buffer, string, stringsize);

	    // Saturate error(i) in order to let it fit in type int8.
	    if (error > 127)
		error = 127;
            tmffs_put8(error);
            tmffs1_putdata(outp, buffer, sizeof(struct dir_s));
            stringsize = strlen(string) + 1;
            tmffs_put8(FPI_STRING); // put directory entry's name...
            tmffs_put8(stringsize);
            memcpy(outp, string, stringsize);
            outp += stringsize;
            tw(tr(TR_FUNC, TrTmffs, "FPI_READDIR()\n"));
            ttw(ttr(TTrTmffs, "tm_rdd" NL));
            break;


        case FPI_STAT:
            // STRING, BUFFER -> ERROR, DATA 
            if ((error = tm_ffs_overflowck()) == EFFS_OK)
                error = ffs_stat(&string[0], (struct stat_s *) buffer);
            tmffs_put8(error);
            tmffs1_putdata(outp, buffer, sizeof(struct stat_s));
            tw(tr(TR_FUNC, TrTmffs, "FPI_STAT()\n"));
            ttw(ttr(TTrTmffs, "tm_st" NL));
            break;
        case FPI_LINKSTAT:
            // STRING, BUFFER -> ERROR, DATA
            if ((error = tm_ffs_overflowck()) == EFFS_OK)
                error = ffs_xlstat(&string[0], (struct xstat_s *) buffer);
            tmffs_put8(error);

            hack_xstat_2_look_like_old_xstat((struct xstat_s *) buffer);

            tmffs1_putdata(outp, buffer, sizeof(struct xstat_s) - 2);
            tw(tr(TR_FUNC, TrTmffs, "FPI_()\n"));
            ttw(ttr(TTrTmffs, "tm_lst" NL));
            break;


        case FPI_SYMLINK:
            // STRING, DATA -> ERROR
            if ((error = tm_ffs_overflowck()) == EFFS_OK)
                error = ffs_symlink_nb(string, (char *) buffer, 0);
            if (error > 0) 
                error = 0;  // ignore request id
            tmffs_put8(error);
            tw(tr(TR_FUNC, TrTmffs, "FPI_SYMLINK()\n"));
            ttw(ttr(TTrTmffs, "tm_sym" NL));
            break;
        case FPI_READLINK:
            // STRING, BUFFER -> ERROR, DATA
            if ((error = tm_ffs_overflowck()) == EFFS_OK)
                error = ffs_readlink(string, (char *) buffer, TMFFS1_BUFFER_SIZE);
            // Because a 32-bit integer is returned, we have to saturate it
            // into an 8-bit value.
            if (error >= 0)
                error = 0;
            tmffs_put8(error);
            tmffs1_putdata(outp, buffer, bufsize); // put link contents
            tw(tr(TR_FUNC, TrTmffs, "FPI_READLINK()\n"));
            ttw(ttr(TTrTmffs, "tm_rdl" NL));
            break;


        case FPI_QUERY:
            // INT8 -> ERROR, DATA
            error = ffs_query(i8[0], buffer);
            tmffs_put8(error);
            tmffs1_putdata(outp, buffer, 16);
            tw(tr(TR_FUNC, TrTmffs, "FPI_QUERY()\n"));
            ttw(ttr(TTrTmffs, "tm_q" NL));
            break;
        case FPI_FCONTROL:
            // STRING INT8 INT32 -> ERROR
            if ((error = tm_ffs_overflowck()) == EFFS_OK)
                error = ffs_fcontrol_nb(string, i8[0], i32[0], 0);
            if (error > 0) 
                error = 0;  // ignore request id
            tmffs_put8(error);
            tw(tr(TR_FUNC, TrTmffs, "FPI_FCONTROL()\n"));
            ttw(ttr(TTrTmffs, "tm_fc" NL));
            break;

        case FPI_INIT:
            // NULL -> ERROR
            error =ffs_initialize();
            tmffs_put8(error);
            tw(tr(TR_FUNC, TrTmffs, "FPI_INIT()\n"));
            ttw(ttr(TTrTmffs, "tm_init" NL));
            break;
        case FPI_EXIT:
            // NULL -> ERROR
            error = ffs_exit();
            tmffs_put8(error);
            tw(tr(TR_FUNC, TrTmffs, "FPI_EXIT()\n"));
            ttw(ttr(TTrTmffs, "tm_exit" NL));
            break;


        case FPI_TFFS:
        {
            // STRING -> ERROR
#if (WITH_TFFS == 1)
            extern char ffs_test_string[]; // defined in task.c

            memcpy(ffs_test_string, string, stringsize);
            tw(tr(TR_FUNC, TrTmffs, "FPI_TFFS()\n"));
            ttw(ttr(TTrTmffs, "tm_tffs" NL));
#else
            tmffs_put8(EFFS_NOSYS);
#endif            
            break;
        }
        default:
            tw(tr(TR_FUNC, TrTmffs, "ERROR: Unknown tmffs protocol code\n"));
            ttw(ttr(TTrTmffs, "tm?" NL));
            break;
        }
        // check if we read beyond buffer end
        if (inp > inp_start + insize) {
            tw(tr(TR_FUNC, TrTmffs, "ERROR: Read beyond end of input buffer\n"));
            ttw(ttr(TTrTmffs, "tm_fatal" NL));
            // NOTEME: We really should reset output buffer and put a return
            // code that tells us what went wrong!
            return 0;
        }
    }

    tw(tr(TR_END, TrTmffs, ""));

    return outp - outp_start;
}

int tmffs_bufsize(void)
{
    return TMFFS1_BUFFER_SIZE;
}

unsigned char *tmffs_bufaddr(void)
{
    return buffer;
}

#endif // TMFFS1 

/******************************************************************************
 * FFS2 protocol  
 ******************************************************************************/

#ifndef TMFFS2

#ifdef RVM_ETM_SWE
int etm_ffs2(T_ETM_PKT *pkt, unsigned char *inp, int insize)
{
    int error;

    tw(tr(TR_BEGIN, TrTmffs, "FFS2 protocol not represented in target\n"));
    error = -1;  // FIXME other error?

    // We return a packet instead of waiting for timeout.
    pkt->size =  0;
    pkt->status = -error;       
    pkt->mid = ETM_FFS2;
    etm_pkt_send(pkt);

    target_free(pkt);
    tw(tr(TR_END, TrTmffs, ""));

    return error;
}
#endif

#else

#define TMFFS_BUFFER_SIZE 256   // FIXME change to packet size
#define TMFFS_STRING_SIZE 127

/******************************************************************************
 * Macros
 ******************************************************************************/

#define tmffs_get8() inp[0]; inp += 1; 
#define tmffs_get16() (inp[0]) | (inp[1] << 8); inp += 2; 
#define tmffs_get32()  inp[0] | (inp[1] << 8) | (inp[2] << 16)\
                       | (inp[3] << 24); inp += 4; 

#define tmffs_getdata()    bufsize = inp[0]; inp += 1; \
                           memcpy(buffer, inp, bufsize); inp += bufsize;


/******************************************************************************
 * Helper function
 ******************************************************************************/

// If size is less than zero it is because of a error and we dont have to put any
// data if size is returned in status.
int tmffs_putdata(unsigned char **buf, unsigned char *src, int size) 
{
 	unsigned char *p = *buf;

	if (size > 0) {
		*p++ = size;
		memcpy(p, src, size); 
		*buf += 1 + size;
	}
	return size;
}

int tmffs_putstring(unsigned char **buf, char *src, int size) 
{
	unsigned char *p = *buf;

	if (size > 0) {
		*p++ = size;
		memcpy(p, src, size); 
		*buf += 1 + size;
	}
	return size;
}

int tmffs_getstring(unsigned char ** buf, char *string)
{
	unsigned char *p = *buf;
	
	stringsize = *p++;

	if (stringsize > TMFFS_STRING_SIZE) 
		return EFFS_TOOBIG;

	memcpy(string, p, stringsize);
	*buf += 1 + stringsize;

	return stringsize;
}

/******************************************************************************
 * tm_ffs
 ******************************************************************************/

// Parse input message and execute function. Then fill output buffer with
// return values from the called function and transmit the message.  Return
// number of bytes inserted into output buffer. If return value is negative,
// it represents an error code.
int etm_ffs2(T_ETM_PKT *pkt, unsigned char *inp, int insize)
{
	tmffs2_cid_t fid;
	unsigned char buffer[TMFFS_BUFFER_SIZE];
	char string[TMFFS_STRING_SIZE];

	unsigned char *outp_start;
	unsigned char *inp_start  = inp;
	unsigned char *outp;

	int error = 0, i, fdi, size, param, flags;
	uint8 type;

	bufsize = stringsize = tmpsize = 0;

	tw(tr(TR_BEGIN, TrTmffs, "TmFFS2\n"));
    
	outp_start = outp = pkt->data;
	
	fid = *inp++;
	ttw(ttr(TTrTmffs, "etm_ffs2 0x%x" NL, fid));
	switch(fid)
	{
		/**********************************************************
		 * Generic Protocol Functions
		 **********************************************************/

	case TMFFS_VERSION:
		tmffs_put16(TMFFS2_VERSION);
		break;

		/**********************************************************
		 * FFS Functions
		 **********************************************************/

	case TMFFS_PREFORMAT:
		param = tmffs_get16();
		error = ffs_preformat(param);	
		tw(tr(TR_FUNC, TrTmffs, "TMFFS_PREFORMAT(0x%x)\n", param));
		ttw(ttr(TTrTmffs, "tm_pfmt" NL));
		break;
	case TMFFS_FORMAT: 
		error = tmffs_getstring(&inp, string);
		param = tmffs_get16();
		if (error >= 0)
			error = ffs_format(&string[0], param);	
		tw(tr(TR_FUNC, TrTmffs, "TMFFS_FORMAT(0x%x)\n", param));
		ttw(ttr(TTrTmffs, "tm_fmt" NL));	
		break;

		
	case TMFFS_FILE_WRITE:
		error = tmffs_getstring(&inp, string);
		tmffs_getdata();
		flags = tmffs_get8();
		if (error >= 0)
			error = ffs_file_write(string, buffer, bufsize, flags);
		ttw(ttr(TTrTmffs, "tm_fwr" NL));
		break;


	case TMFFS_FILE_READ:
		error = tmffs_getstring(&inp, string);
		bufsize = tmffs_get8();
		if (error >= 0)
			size = ffs_file_read(string, buffer, bufsize);
		error = tmffs_putdata(&outp, &buffer[0], size);
		tw(tr(TR_FUNC, TrTmffs, "TMFFS_FREAD('%s', 0x%x, %d/0x%x)\n",
			  string, buffer, bufsize, bufsize));
		ttw(ttr(TTrTmffs, "tm_frd" NL));
		break;
	case TMFFS_REMOVE:
		error = tmffs_getstring(&inp, string);
		if (error >= 0)
			error = ffs_remove(string);
		tw(tr(TR_FUNC, TrTmffs, "TMFFS_REMOVE()\n"));
		ttw(ttr(TTrTmffs, "tm_rm" NL));
		break;


	case TMFFS_OPEN:
		error = tmffs_getstring(&inp, string);
		flags = tmffs_get8();
		if (error >= 0)
			error = ffs_open(string, flags);
		tmffs_put8(error); // fdi
		tw(tr(TR_FUNC, TrTmffs, "TMFFS_OPEN('%s', %d)\n", string, flags));
		ttw(ttr(TTrTmffs, "tm_open" NL));
		break;
	case TMFFS_CLOSE:
		fdi = tmffs_get8(); 
		error = ffs_close(fdi);
		tw(tr(TR_FUNC, TrTmffs, "TMFFS_CLOSE(%d)\n", fdi));
		ttw(ttr(TTrTmffs, "tm_close" NL));
		break;
	case TMFFS_WRITE:
		fdi = tmffs_get8(); 
		tmffs_getdata(); 
		error = ffs_write(fdi, buffer, bufsize);
		tmffs_put8(error); // put size
		tw(tr(TR_FUNC, TrTmffs, "TMFFS_WRITE(%d, %d)\n", fdi, bufsize));
		ttw(ttr(TTrTmffs, "tm_write" NL));
		break;
	case TMFFS_READ:
		fdi = tmffs_get8(); 
		size = tmffs_get8();
		size = ffs_read(fdi, &buffer[0], size);
		error =	tmffs_putdata(&outp, &buffer[0], size);
		tw(tr(TR_FUNC, TrTmffs, "TMFFS_READ(%d, %d)\n", fdi, size));
		ttw(ttr(TTrTmffs, "tm_read" NL));
		break;


	case TMFFS_MKDIR:
		error = tmffs_getstring(&inp, string);
		if (error >= 0)
			error = ffs_mkdir(string);
		tw(tr(TR_FUNC, TrTmffs, "TMFFS_MKDIR()\n"));
		ttw(ttr(TTrTmffs, "tm_mkd" NL));
		break;
	case TMFFS_OPENDIR:
		error = tmffs_getstring(&inp, string);
		if (error >= 0) { 
			error = ffs_opendir(string, (struct dir_s *) buffer);
			tmffs_put8(error); // Note: we must put error/number of objects.
		}
		if (error >= 0)
			tmffs_putdata(&outp, buffer, sizeof(struct dir_s));
		
		tw(tr(TR_FUNC, TrTmffs, "TMFFS_OPENDIR()\n"));
		ttw(ttr(TTrTmffs, "tm_od" NL));
		break;
	case TMFFS_READDIR:
		tmffs_getdata();
		stringsize = tmffs_get8();		
		error = ffs_readdir((struct dir_s *) buffer, string, stringsize);
		tmffs_put8(error); // Note: we have to return bytes read.
		if (error >= 0) {
			tmffs_putdata(&outp, buffer, sizeof(struct dir_s));
			stringsize = strlen(string) + 1;			
			tmffs_putstring(&outp, string, stringsize);
		}
		tw(tr(TR_FUNC, TrTmffs, "TMFFS_READDIR()\n"));
		ttw(ttr(TTrTmffs, "tm_rdd" NL));
		break;

	case TMFFS_STAT:
		error = tmffs_getstring(&inp, string);
		if (error >= 0) 
			error = ffs_stat(string, (struct stat_s *) buffer);
		if (error >= 0)
			tmffs_putdata(&outp, buffer, sizeof(struct stat_s));
		
		tw(tr(TR_FUNC, TrTmffs, "TMFFS_STAT()\n"));
		ttw(ttr(TTrTmffs, "tm_st" NL));
		break;
	case TMFFS_XLSTAT:
		error = tmffs_getstring(&inp, string);
		if (error >= 0)
			error = ffs_xlstat(&string[0], (struct xstat_s *) buffer);
		if (error >= 0)
			tmffs_putdata(&outp, buffer, sizeof(struct xstat_s));
		tw(tr(TR_FUNC, TrTmffs, "TMFFS_()\n"));
		ttw(ttr(TTrTmffs, "tm_xlst" NL));
		break;


	case TMFFS_SYMLINK:
		error = tmffs_getstring(&inp, string);
		tmffs_getdata();
		if (error >= 0)
			error = ffs_symlink(string, (char *) buffer);
		tw(tr(TR_FUNC, TrTmffs, "TMFFS_SYMLINK()\n"));
		ttw(ttr(TTrTmffs, "tm_sym" NL));
		break;
	case TMFFS_READLINK:
		error = tmffs_getstring(&inp, string);
		tmffs_getdata();
		if (error >= 0) {
			size = ffs_readlink(string, (char *) buffer, TMFFS_BUFFER_SIZE);
			error = tmffs_putdata(&outp, buffer, size); // put link contents
		}
		tw(tr(TR_FUNC, TrTmffs, "TMFFS_READLINK()\n"));
		ttw(ttr(TTrTmffs, "tm_rdl" NL));
		break;


	case TMFFS_QUERY:
		param = tmffs_get8();
		error = ffs_query(param, buffer);
		if (error >= 0)
			tmffs_putdata(&outp, buffer, 16);
		tw(tr(TR_FUNC, TrTmffs, "TMFFS_QUERY(%d)\n", param));
		ttw(ttr(TTrTmffs, "tm_q" NL));
		break;
	case TMFFS_FCONTROL:
		error = tmffs_getstring(&inp, string);
		type = tmffs_get8();
		param = tmffs_get32();
		if (error >= 0)
			error = ffs_fcontrol(string, type, param);
		tw(tr(TR_FUNC, TrTmffs, "TMFFS_FCONTROL()\n"));
		ttw(ttr(TTrTmffs, "tm_fc" NL));
		break;

	case TMFFS_TFFS:
	{
#if (WITH_TFFS == 1)
		extern char ffs_test_string[]; // defined in task.c
		error = tmffs_getstring(&inp, string);
		memcpy(ffs_test_string, string, stringsize);
		tw(tr(TR_FUNC, TrTmffs, "TMFFS_TFFS()\n"));
		ttw(ttr(TTrTmffs, "tm_tffs" NL));
		tmffs_put8(EFFS_OK);
#else
		tmffs_put8(EFFS_NOSYS);
#endif            
		break;
	}
	default:
		error = EFFS_NOSYS;
		tmffs_put8(EFFS_NOSYS);
		tw(tr(TR_FUNC, TrTmffs, "ERROR: Unknown tmffs protocol code\n"));
		ttw(ttr(TTrTmffs, "tm?" NL));
		break;
	}
	
	// check if we read beyond buffer end
	if (inp > inp_start + insize) {
		tw(tr(TR_FUNC, TrTmffs, "ERROR: Read beyond end of input buffer\n"));
		ttw(ttr(TTrTmffs, "tm_fatal" NL));
		ttw(ttr(TTrTmffs, "insize: %d, diff: %d" NL, insize, 
				inp - (inp_start + insize)));
		// NOTEME: We really should reset output buffer and put a return
		// code that tells us what went wrong!
		error = ETM_PACKET;  // FIXME find another error 
	}

	ttw(ttr(TTrTmffs, "error %d" NL, error));
	if (error > 0)
		error = 0;

	pkt->mid    = ETM_FFS2;
	pkt->size   = outp - outp_start;
	pkt->status = -error;

	etm_pkt_send(pkt);
	etm_free(pkt);

	tw(tr(TR_END, TrTmffs, ""));

	return ETM_OK;
}

#endif // TMFFS2
