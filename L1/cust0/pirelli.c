/*
 * This module is a FreeCalypso addition.  Here we implement
 * retrieval of Pirelli's factory calibration and IMEI records
 * from their factory data block.
 */

#include <string.h>
#include "config.h"
#include "sys_types.h"
#include "../../services/ffs/ffs.h"
#include "../../services/ffs/core.h"	/* for ffs_begin() and ffs_end() */

#define	FACTORY_BLOCK_BASE_ADDR	0x027F0000

static effs_t
pirelli_chksum(uint8 *addr, T_FFS_SIZE size)
{
	uint8 accum = 0;

	for (; size; size--)
		accum += *addr++;
	if (accum == *addr)
		return EFFS_OK;
	else
		return EFFS_CORRUPTED;
}

pirelli_read_factory_record(uint32 offset, void *userbuf, T_FFS_SIZE size,
			    int has_chksum)
{
	effs_t rc;
	uint8 *flash_record = (uint8 *) FACTORY_BLOCK_BASE_ADDR + offset;

	rc = ffs_begin();
	if (rc != EFFS_OK)
		return(rc);
	if (has_chksum)
		rc = pirelli_chksum(flash_record, size);
	if (rc == EFFS_OK)
		bcopy(flash_record, userbuf, size);
	return ffs_end(rc);
}

static const struct calmap {
	uint32	offset;
	char	*ti_equiv;
} pirelli_cal_map[] = {
	{0x06E5, "/sys/adccal"},
	{0x072B, "/gsm/rf/tx/ramps.900"},
	{0x092C, "/gsm/rf/tx/levels.900"},
	{0x09AD, "/gsm/rf/tx/calchan.900"},
	{0x0A2E, "/gsm/rf/tx/ramps.1800"},
	{0x0C2F, "/gsm/rf/tx/levels.1800"},
	{0x0CB0, "/gsm/rf/tx/calchan.1800"},
	{0x0D31, "/gsm/rf/tx/ramps.1900"},
	{0x0F32, "/gsm/rf/tx/levels.1900"},
	{0x0FB3, "/gsm/rf/tx/calchan.1900"},
	{0x10AF, "/gsm/rf/rx/calchan.900"},
	{0x10D8, "/gsm/rf/rx/agcparams.900"},
	{0x10E1, "/gsm/rf/rx/calchan.1800"},
	{0x110A, "/gsm/rf/rx/agcparams.1800"},
	{0x1113, "/gsm/rf/rx/calchan.1900"},
	{0x113C, "/gsm/rf/rx/agcparams.1900"},
	{0, 0}
};

pirelli_cal_fread(const char *name, void *userbuf, T_FFS_SIZE size)
{
	int rc;
	const struct calmap *map;

	/* try FFS first, so FreeCalypso user can override factory prog */
	rc = ffs_file_read(name, userbuf, size);
	if (rc >= 0)
		return EFFS_OK;
	/* does the sought file correspond to a Pirelli factory data record? */
	for (map = pirelli_cal_map; map->ti_equiv; map++)
		if (!strcmp(map->ti_equiv, name))
			break;
	if (!map->offset)	/* not found */
		return rc;	/* return error code from FFS */
	/* found it */
	return pirelli_read_factory_record(map->offset, userbuf, size, 1);
}
