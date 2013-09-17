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
struct __angsd_io_t;
typedef struct __angsd_io_t angsd_io_t;

#define angsd_open(fn, mode) gzopen(fn, mode)
#define angsd_close(fp) gzclose(fp)
#define angsd_read(fp, buf, size) gzread(fp, buf, size)

#ifdef __cplusplus
extern "C" {
#endif

	ssize_t angsd_getline(char **line_ptr, size_t *N, angsd_io_t *angsd_io);
	void angsd_set_mafs_header(angsd_io_t *mafs);
	void angsd_set_counts_nind(angsd_io_t *counts);
	char **splitstr(char *s);
	angsd_io_t *angsd_open_file(const char *fn);
	void angsd_close_file(angsd_io_t *mafs);

#ifdef __cplusplus
}
#endif

#endif
