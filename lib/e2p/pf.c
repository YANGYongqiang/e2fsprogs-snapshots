/*
 * pf.c			- Print file attributes on an ext2 file system
 *
 * Copyright (C) 1993, 1994  Remy Card <card@masi.ibp.fr>
 *                           Laboratoire MASI, Institut Blaise Pascal
 *                           Universite Pierre et Marie Curie (Paris VI)
 *
 * %Begin-Header%
 * This file may be redistributed under the terms of the GNU Library
 * General Public License, version 2.
 * %End-Header%
 */

/*
 * History:
 * 93/10/30	- Creation
 */

#include "config.h"
#include <stdio.h>

#include "e2p.h"

struct flags_name {
	unsigned long	flag;
	const char	*short_name;
	const char	*long_name;
};

static struct flags_name flags_array[] = {
	{ EXT2_SECRM_FL, "s", "Secure_Deletion" },
	{ EXT2_UNRM_FL, "u" , "Undelete" },
	{ EXT2_SYNC_FL, "S", "Synchronous_Updates" },
	{ EXT2_DIRSYNC_FL, "D", "Synchronous_Directory_Updates" },
	{ EXT2_IMMUTABLE_FL, "i", "Immutable" },
	{ EXT2_APPEND_FL, "a", "Append_Only" },
	{ EXT2_NODUMP_FL, "d", "No_Dump" },
	{ EXT2_NOATIME_FL, "A", "No_Atime" },
	{ EXT2_COMPR_FL, "c", "Compression_Requested" },
#ifdef ENABLE_COMPRESSION
	{ EXT2_COMPRBLK_FL, "B", "Compressed_File" },
	{ EXT2_DIRTY_FL, "Z", "Compressed_Dirty_File" },
	{ EXT2_NOCOMPR_FL, "X", "Compression_Raw_Access" },
	{ EXT2_ECOMPR_FL, "E", "Compression_Error" },
#endif
	{ EXT3_JOURNAL_DATA_FL, "j", "Journaled_Data" },
	{ EXT2_INDEX_FL, "I", "Indexed_directory" },
	{ EXT2_NOTAIL_FL, "t", "No_Tailmerging" },
	{ EXT2_TOPDIR_FL, "T", "Top_of_Directory_Hierarchies" },
	{ EXT4_EXTENTS_FL, "e", "Extents" },
	{ EXT4_HUGE_FILE_FL, "h", "Huge_file" },
	{ EXT4_SNAPFILE_FL, "x", "Snapshot_File" },
	{ 0, NULL, NULL }
};

/* Snapshot dynamic state flags */
static struct flags_name snapshot_flags_array[] = {
	{ 1UL<<EXT4_SNAPSHOT_LIST, "S", "on_liSt" },
	{ 1UL<<EXT4_SNAPSHOT_ENABLED, "n", "eNabled" },
	{ 1UL<<EXT4_SNAPSHOT_ACTIVE, "a", "Active" },
	{ 1UL<<EXT4_SNAPSHOT_INUSE, "p", "inuse_by_Previous" },
	{ 1UL<<EXT4_SNAPSHOT_DELETED, "s", "Deleted" },
	{ 1UL<<EXT4_SNAPSHOT_SHRUNK, "h", "sHrunk" },
	{ 1UL<<EXT4_SNAPSHOT_OPEN, "o", "mOunted" },
	{ 1UL<<EXT4_SNAPSHOT_TAGGED, "t", "Tagged" },
	{ 0, NULL, NULL }
};

void print_flags (FILE * f, unsigned long flags, unsigned options)
{
	struct flags_name *array = ((options & PFOPT_SNAPSHOT) ?
					snapshot_flags_array : flags_array);
	int long_opt = (options & PFOPT_LONG);
	struct flags_name *fp;
	int	first = 1;

	for (fp = array; fp->flag != 0; fp++) {
		if (flags & fp->flag) {
			if (long_opt) {
				if (first)
					first = 0;
				else
					fputs(", ", f);
				fputs(fp->long_name, f);
			} else
				fputs(fp->short_name, f);
		} else {
			if (!long_opt)
				fputs("-", f);
		}
	}
	if (long_opt && first)
		fputs("---", f);
}

