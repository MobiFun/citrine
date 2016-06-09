/******************************************************************************
 * Flash File System (ffs)
 * Idea, design and coding by Mads Meisner-Jensen, mmj@ti.com
 *
 * Condat PCM Compatibility Support
 *
 * $Id: pcmcode.c 1.46 Tue, 06 Nov 2001 11:55:21 +0100 tsj $
 *
 ******************************************************************************/

#include <string.h>
#include "pcm.h"
#include "../ffs/ffs.h"
#include "../ffs/ffstrace.h"

extern const T_PCM_DESCRIPTION pcm_table[];
extern const UBYTE pcm_default_values[];
extern UBYTE pcm_mem [];


/******************************************************************************
 * 
 ******************************************************************************/

// pcm_Init() has been renamed to pcm_init() so that it is not called
// anywhere else than it should. The old pcm_Init() is now empty. This new
// pcm_init() scans through the pcm file table and attempts to read each
// file from ffs into the pcm RAM image.

drv_Return_Type pcm_Init(void)
{
    return PCM_INITIALIZED;
}


// Note that PCM file data chunks start with one byte for the file data
// checksum, followed by another byte for the version. The third byte
// (offset 2) is the start of the actual filedata. We ignore these first two
// bytes e.g. we only read/write the actual file data!


// look up a PCM file
int pcm_lookup(char *pcm_name) 
{
    int i = 0;

    while (pcm_table[i].identifier != NULL)
    {
        if (!strcmp((char *) pcm_name, pcm_table[i].identifier + 5))
            return i;
        i++;
    }
    return -1; // not found.
}

drv_Return_Type pcm_init(void)
{
    int i = 0;
    effs_t error;

    ttw(ttr(TTrInit, "pcm_init" NL));

    while (pcm_table[i].identifier != NULL)
    {
        error = ffs_fread(pcm_table[i].identifier,
                          &pcm_mem[pcm_table[i].start + 2],
                          (pcm_table[i].length - 2) * pcm_table[i].records);

        if (error < EFFS_OK) {
            // copy defaults to pcm_mem
            memcpy (&pcm_mem[pcm_table[i].start] + 2,
                    &pcm_default_values[pcm_table[i].start - 2*i],
                    pcm_table[i].records * (pcm_table[i].length - 2));
        }
        pcm_mem[pcm_table[i].start + 1] = 1;  // file version
        i++;
    }
    
    return PCM_INITIALIZED;
}

drv_Return_Type pcm_GetFileInfo(UBYTE  * in_FileName,
				pcm_FileInfo_Type * out_FileInfoPtr)
{
    int i = pcm_lookup((char*)in_FileName);

    ttw(ttr(TTrPcmRead, "pcm_gfi(%s)" NL, in_FileName));

    if (i == -1)
        return PCM_INVALID_FILE;

      out_FileInfoPtr->FileLocation = &pcm_mem [pcm_table[i].start+2];
      out_FileInfoPtr->FileSize     = pcm_table[i].length -2;
      // As Condat has determined that all files is version 1, we just
      // hardwire exactly that!
      // out_FileInfoPtr->Version      = pcm_mem [pcm_table[i].start + 1];
      out_FileInfoPtr->Version      = 1;

    return PCM_OK;
}


/******************************************************************************
 * Normal read/write functions
 ******************************************************************************/

drv_Return_Type pcm_ReadFile(UBYTE  * in_FileName,
			     USHORT   in_BufferSize,
			     UBYTE  * out_BufferPtr,
			     UBYTE  * out_VersionPtr)
{
    int i = pcm_lookup((char*)in_FileName);

    ttw(ttr(TTrPcmRead, "pcm_rf(%s)" NL, in_FileName));

    if (i == -1)
        return PCM_INVALID_FILE;

    if (in_BufferSize + 2 != pcm_table[i].length)
        return PCM_INVALID_SIZE; 

    // checksum check removed --- it is redundant!

    memcpy (out_BufferPtr, &pcm_mem[pcm_table[i].start+2], in_BufferSize);
    *out_VersionPtr = pcm_mem[pcm_table[i].start+1];

    return PCM_OK;
}

drv_Return_Type pcm_WriteFile(UBYTE  * in_FileName,
			      USHORT   in_FileSize,
			      UBYTE  * in_BufferPtr)
{
    int i = pcm_lookup((char*)in_FileName);

    ttw(ttr(TTrPcmWrite, "pcm_wf(%s)" NL, in_FileName));

    if (i == -1)
        return PCM_INVALID_FILE;

    if (in_FileSize + 2 != pcm_table[i].length)
        return PCM_INVALID_SIZE;

    memcpy (&pcm_mem[pcm_table[i].start+2], in_BufferPtr, in_FileSize);

    // write the whole file to ffs! (ignoring errors)
    ffs_fwrite(pcm_table[i].identifier, 
               &pcm_mem[pcm_table[i].start + 2],
               in_FileSize);

    return PCM_OK;
}


/******************************************************************************
 * Record read/write functions
 ******************************************************************************/

/* Record files are implemented by having the first two bytes of a
 * file be equal to the record size. */

drv_Return_Type pcm_ReadRecord(UBYTE  * in_FileName,
			       USHORT   in_Record,
			       USHORT   in_BufferSize,
			       UBYTE  * out_BufferPtr,
			       UBYTE  * out_VersionPtr,
			       USHORT * out_MaxRecordsPtr)
{
    int i = pcm_lookup((char*)in_FileName);

    ttw(ttr(TTrPcmRead, "pcm_rr(%s)" NL, in_FileName));

    if (i == -1)
        return PCM_INVALID_FILE;

    if (in_BufferSize + 2 != pcm_table[i].length)
        return PCM_INVALID_SIZE;

    if (in_Record == 0 || in_Record > pcm_table[i].records)
        return PCM_INVALID_RECORD;

    memcpy (out_BufferPtr,
            &pcm_mem[pcm_table[i].start + 2 + (in_Record-1) * in_BufferSize],
            in_BufferSize);
    *out_MaxRecordsPtr = pcm_table[i].records;
    *out_VersionPtr    = pcm_mem [pcm_table[i].start + 1];

    return PCM_OK;
}

drv_Return_Type pcm_WriteRecord(UBYTE  * in_FileName,
				USHORT   in_Record,
				USHORT   in_BufferSize,
				UBYTE  * in_BufferPtr)
{
    int i = pcm_lookup((char*)in_FileName);

    ttw(ttr(TTrPcmWrite, "pcm_wr(%s)" NL, in_FileName));

    if (i == -1)
        return PCM_INVALID_FILE;    

    if (in_BufferSize + 2 != pcm_table[i].length)
        return PCM_INVALID_SIZE;

    if (in_Record == 0 || in_Record > pcm_table[i].records)
        return PCM_INVALID_RECORD;    

    memcpy (&pcm_mem [pcm_table[i].start + 2 + (in_Record-1) * in_BufferSize],
            in_BufferPtr,
            in_BufferSize);

    // write the whole file to ffs! (ignoring errors)
    ffs_fwrite(pcm_table[i].identifier, 
               &pcm_mem [pcm_table[i].start + 2],
               pcm_table[i].records * (pcm_table[i].length - 2));

    return PCM_OK;
}
