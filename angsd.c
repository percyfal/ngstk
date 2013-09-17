/**
 * @file   angsd.c
 * @author Per Unneberg
 * @date   Fri Sep 13 18:34:26 2013
 *
 * @brief   
 *
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <zlib.h>
#include "angsd.h"

#define BUFLEN 256
#define LENGTH 0x1000

struct __angsd_mafs_io_t {
	angsdFile fp;
	char buffer[2*BUFLEN];
};

angsd_mafs_io_t *angsd_open_mafs(const char *fn)
{
	angsd_mafs_io_t * mafs;
	mafs = (angsd_mafs_io_t*)calloc(1, sizeof(angsd_mafs_io_t));
	mafs->fp = angsd_open(fn, "r");
	return mafs;
}

char *readline(angsd_mafs_io_t *mafs) 
{
	char tmpbuffer[BUFLEN];
	int bytes_read;
	char *line;
	if (strchr(mafs->buffer, '\n') == NULL) {
		bytes_read = angsd_read(mafs->fp, tmpbuffer, BUFLEN - 1);
		tmpbuffer[bytes_read] = '\0';
		strcpy(mafs->buffer, strcat(mafs->buffer, tmpbuffer));
	}
	
	// StackOverflow: how to read line by line after text into a buffer
	line = mafs->buffer;
	char *eol = strchr(line, '\n');
	*eol = '\0';
	fprintf(stderr, "line: '%s'\n", line);
	eol++;
	if (eol != NULL) {
		strcpy(mafs->buffer, eol);
	}
	return line;
}

char **splitstr(const char *s)
{
	char **res = NULL;
	char *p = strtok(s, "\t ");
	fprintf(stderr, "p: '%s'\n", p);
	int n_spaces = 0, i;
	while(p) {
		res = realloc(res, sizeof (char *) * ++n_spaces);
		if (res == NULL)
			exit (-1);
		res[n_spaces-1] = p;
		p = strtok(NULL, " \t");
	}
	res = realloc(res, sizeof (char *) * (n_spaces + 1));
	res[n_spaces]=0;
	for (i=0; i<(n_spaces + 1); ++i)
		fprintf(stderr, "res[%d] = '%s'\n", i, res[i]);
	return res;
}


void angsd_close_mafs(angsd_mafs_io_t *mafs)
{
	angsd_close (mafs->fp);
	free (mafs);
}


/* mafs_header_t *mafs_header_read(mafsFile fp) */
/* {  */
	
/* } */


/* char *readline(gzFile *fp) */
/* { */
/* 	int i = 0; */
/* 	char line[LENGTH]; */
	
/* 	while (1) { */
/*         int err; */
/* 		int bytes_read; */
/* 		char *c; */
/* 		unsigned char buf[LENGTH]; */
/* 		char buffer[4]; */
/* 		// bytes_read = gzread(fp, buf, LENGTH - 1); */
/* 		bytes_read = gzread(fp, buffer, 4); */
/* 		buffer[bytes_read] = '\0'; */
/* 		size_t sl; */
/* 		fprintf(stderr, "Got buffer: %s\n", buffer); */
/* 		strcpy(line, buf); */
/* 		// fprintf(stderr, "%i bytes read\n", bytes_read); */
/*         if (bytes_read < LENGTH - 1) { */
/*             if (gzeof (fp)) { */
/*                 break; */
/*             } */
/*             else { */
/*                 const char * error_string; */
/*                 error_string = gzerror (fp, & err); */
/*                 if (err) { */
/*                     fprintf (stderr, "Error: %s.\n", error_string); */
/*                     exit (EXIT_FAILURE); */
/*                 } */
/*             } */
/*         } */
/*  	}  */
/*  	return line;  */
/* }  */

int angsd(int argc, char *argv[])
{
	int c = -1;
	while ((c = getopt(argc, argv, "A")) >= 0) {
		switch (c) {
		case 'A':
			fprintf(stderr, "Saw option A\n");
			break;
		default: return 1;
		}
	}
	fprintf(stderr, "[angsd]: Parsed options; remaining %s\n", argv[0]);
	fprintf(stderr, "[angsd]: optind: %i\n", optind);
	fprintf(stderr, "[angsd]: argc: %i\n", argc);
	
	if (optind + 1 != argc) {
		fprintf(stderr, "optind + 1 > argc: %i + 1 > %i\n", optind, argc);
		fprintf(stderr, "\n");
		fprintf(stderr, "Usage: ngstk angsd [options] <prefix>\n");
		return 1;
	}
	fprintf(stderr, "[angsd]: result prefix '%s'\n", argv[optind]);


	// Start processing input

	// open mafs files
	angsd_mafs_io_t *mafs;
	mafs = angsd_open_mafs(argv[optind]);

	// Read a line
	char *line = readline(mafs);
	fprintf(stderr, "line: '%s'\n", line);	
	//char **res = splitstr(line);
	//free (res);
	
	// close files
	angsd_close_mafs(mafs);
	return 0;
}
