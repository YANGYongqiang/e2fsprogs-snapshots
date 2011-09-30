/*
 * fgetsnapflags.c	- Get snapshot file flags on an ext4 file system
 *
 * from fgetflags.c
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

#define _LARGEFILE_SOURCE
#define _LARGEFILE64_SOURCE

#include "config.h"
#if HAVE_ERRNO_H
#include <errno.h>
#endif
#if HAVE_UNISTD_H
#include <unistd.h>
#endif
#include <sys/types.h>
#include <sys/stat.h>
#if HAVE_EXT2_IOCTLS
#include <fcntl.h>
#include <sys/ioctl.h>
#endif

#include "e2p.h"

#ifdef O_LARGEFILE
#define OPEN_FLAGS (O_RDONLY|O_NONBLOCK|O_LARGEFILE)
#else
#define OPEN_FLAGS (O_RDONLY|O_NONBLOCK)
#endif

int fgetsnapflags(const char * name, unsigned long * flags)
{
	struct stat buf;
#if HAVE_EXT2_IOCTLS
#if !APPLE_DARWIN
	int fd, r, f, save_errno = 0;

	if (!lstat(name, &buf) && !S_ISREG(buf.st_mode))
		goto notsupp;
	fd = open(name, OPEN_FLAGS);
	if (fd == -1)
		return -1;
	r = ioctl(fd, EXT2_IOC_GETSNAPFLAGS, &f);
	if (r == -1)
		save_errno = errno;
	*flags = f;
	close(fd);
	if (save_errno)
		errno = save_errno;
	return r;
#endif
#endif /* HAVE_EXT2_IOCTLS */
notsupp:
	errno = EOPNOTSUPP;
	return -1;
}
