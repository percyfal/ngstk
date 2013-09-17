/**
 * @file   angsd.h
 * @author Per Unneberg
 * @date   Fri Sep 13 18:40:01 2013
 *
 * @brief   
 *
 *
 */

#ifndef ANGSD_H
#define ANGSD_H

#include <stdint.h>
#include <zlib.h>

typedef gzFile angsdFile;
struct __angsd_mafs_io_t;
typedef struct __angsd_mafs_io_t angsd_mafs_io_t;

#define angsd_open(fn, mode) gzopen(fn, mode)
#define angsd_close(fp) gzclose(fp)
#define angsd_read(fp, buf, size) gzread(fp, buf, size)

#ifdef __cplusplus
extern "C" {
#endif

	char *readline(angsd_mafs_io_t *mafs) ;
	char **splitstr(const char *s);
	angsd_mafs_io_t *angsd_open_mafs(const char *fn);
	void angsd_close_mafs(angsd_mafs_io_t *mafs);

#ifdef __cplusplus
}
#endif

#endif
