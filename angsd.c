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
#include <assert.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <zlib.h>
#include "angsd.h"

#define MIN_CHUNK 64
#define BUFLEN 256
#define LENGTH 0x1000

struct __angsd_io_t {
	angsdFile fp;
	char buffer[2*BUFLEN];
	// Columns of interest
	int chromo, position, major, minor, anc, knownEM, nInd;
	// Total number of individuals 
	int nind_tot;
};


angsd_io_t *angsd_open_file(const char *fn)
{
	angsd_io_t *angsd_io;
	fprintf(stderr, "[angsd] [angsd_mafs_io_t]: opening file %s\n", fn);
	angsd_io = (angsd_io_t*)calloc(1, sizeof(angsd_io_t));
	angsd_io->fp = angsd_open(fn, "r");
	return angsd_io;
}

void angsd_set_mafs_header(angsd_io_t *mafs) 
{
	size_t N=256;
	char *my_string;
	my_string = (char *) malloc (N);
	int i = angsd_getline(&my_string, &N, mafs);
	char **header = splitstr(my_string);
	if (header)
    {
        for (i = 0; *(header + i); i++)
        {
			if (strcmp(*(header+i), "chromo")==0)
				mafs->chromo = i;
			else if (strcmp(*(header+i), "position")==0)
				mafs->position = i;
			else if (strcmp(*(header+i), "major")==0)
				mafs->major = i;
			else if (strcmp(*(header+i), "minor")==0)
				mafs->minor = i;
			else if (strcmp(*(header+i), "anc")==0)
				mafs->anc = i;
			else if (strcmp(*(header+i), "knownEM")==0)
				mafs->knownEM = i;
			else if (strcmp(*(header+i), "nInd")==0)
				mafs->nInd = i;
        }
        free(header);
    }
}

void angsd_set_counts_nind(angsd_io_t *counts)
{
	size_t N=256;
	char *my_string;
	my_string = (char *) malloc (N);
	int i = angsd_getline(&my_string, &N, counts);
	int nind ;
	fprintf(stderr, "[angsd] [angsd_set_counts_nind]: saw header %s\n", my_string);
	
	char **header = splitstr(my_string);
	if (header)
    {
        for (i = 0; *(header + i); i++)
			nind = i + 1;
        free(header);
    }
	counts->nind_tot = nind;
	fprintf(stderr, "[angsd] [angsd_set_counts_nind]: saw %i individuals\n", nind);
}

// See http://www.opensource.apple.com/source/cvs_wrapped/cvs_wrapped-5/cvs_wrapped/lib/getline.c
ssize_t angsd_getline(char **line_ptr, size_t *N, angsd_io_t *angsd_io)
{
	int nchars_avail, bytes_read;
	char tmpbuffer[BUFLEN];
	char *read_pos;
	char *c;
	int ret;
	int i;
	
	if (!line_ptr || !N ) {
		errno = EINVAL;
		return -1;
	}
	if (!*line_ptr) {
		*N = MIN_CHUNK;
		*line_ptr = malloc (*N);
		if (!*line_ptr) {
			errno = ENOMEM;
			return -1;
		}
	}

	// Do buffered input if no newline
	if (strchr(angsd_io->buffer, '\n') == NULL) {
		bytes_read = angsd_read(angsd_io->fp, tmpbuffer, BUFLEN - 1);
		if (bytes_read > 0) 
			strcpy(angsd_io->buffer, strcat(angsd_io->buffer, tmpbuffer));
		else
			return -1;
	}

	read_pos = *line_ptr;
	nchars_avail = *N;
	
	// Find eol character
	char *eol = strchr(angsd_io->buffer, '\n');
	*eol = '\0';
	
	// Fill line_ptr with characters
	for (i=0; i<strlen(angsd_io->buffer); i++) {
		c = angsd_io->buffer[i];
		
		int save_errno;
		save_errno = errno;
		assert((*line_ptr + *N) == (read_pos + nchars_avail));
		if (nchars_avail < 2)
		{
			if (*N > MIN_CHUNK)
				*N *= 2;
			else
				*N += MIN_CHUNK;

			nchars_avail = *N + *line_ptr - read_pos;
			*line_ptr = realloc (*line_ptr, *N);
			if (!*line_ptr)
			{
				errno = ENOMEM;
				return -1;
			}
			read_pos = *N - nchars_avail + *line_ptr;
			assert((*line_ptr + *N) == (read_pos + nchars_avail));
		}
		if (c == EOF)
		{
			/* Return partial line, if any.  */
			if (read_pos == *line_ptr)
				return -1;
			else
				break;
		}

		*read_pos++ = angsd_io->buffer[i];
		nchars_avail--;
		if (c == '\n')
			/* Return the line.  */
			break;
	}
	eol++;
	if (eol != NULL) {
		strcpy(angsd_io->buffer, eol);
	}

	*read_pos = '\0';
	ret = read_pos - (*line_ptr);
	return ret;
}


char **splitstr(char *s)
{
	char **res = NULL;
	char *p = strtok(s, "\t ");
	int n_spaces = 0;
	while(p) {
		res = realloc(res, sizeof (char *) * ++n_spaces);
		if (res == NULL)
			exit (-1);
		res[n_spaces-1] = p;
		p = strtok(NULL, " \t");
	}
	res = realloc(res, sizeof (char *) * (n_spaces + 1));
	res[n_spaces]=0;
	return res;
}

void angsd_close_file(angsd_io_t *angsd_io)
{
	angsd_close (angsd_io->fp);
	free (angsd_io);
}

int angsd(int argc, char *argv[])
{
	int c = -1, i;
	char popA[256], popB[256];
	
	while ((c = getopt(argc, argv, "A")) >= 0) {
		switch (c) {
		case 'A':
			fprintf(stderr, "Saw option A\n");
			break;
		default: return 1;
		}
	}
	
	if (optind + 2 != argc) {
		fprintf(stderr, "Usage: ngstk angsd [options] <prefix_populationA> <prefix_populationB>\n\n");
		fprintf(stderr, "  options:\n");
		fprintf(stderr, "    -n             Number of individuals that must have given coverage\n");
		fprintf(stderr, "    -c             Minimum required read coverage\n");
		return 1;
	}

	// Start processing input
	// open mafs files
	angsd_io_t *mafsA, *mafsB, *countsA, *countsB;

	strcpy(popA, argv[optind]);
	strcpy(popB, argv[optind+1]);
	
	fprintf(stderr, "population A %s\n", popA);
	fprintf(stderr, "population B %s\n", popB);
	
	mafsA = angsd_open_file(strcat(popA, ".mafs.gz"));
	mafsB = angsd_open_file(strcat(popB, ".mafs.gz"));

	strcpy(popA, argv[optind]);
	strcpy(popB, argv[optind+1]);

	countsA = angsd_open_file(strcat(popA, ".counts.gz"));
	countsB = angsd_open_file(strcat(popB, ".counts.gz"));
	
	// Set the header information
	angsd_set_mafs_header(mafsA);
	angsd_set_mafs_header(mafsB);
	angsd_set_counts_nind(countsA);
	angsd_set_counts_nind(countsB);

	// Read the files
	size_t N=256;

	char **resA, **resB;
	while (1) {
		int iA, iB, posA, posB;
		char *s_mafsA, *s_mafsB, *s_countsA, *s_countsB;
		s_mafsA = (char *) malloc (N);
		s_mafsB = (char *) malloc (N);
		s_countsA = (char *) malloc (N);
		s_countsB = (char *) malloc (N);
		iA = angsd_getline(&s_mafsA, &N, mafsA);
		iB = angsd_getline(&s_mafsB, &N, mafsB);
		resA = splitstr(s_mafsA);
		resB = splitstr(s_mafsB);
		posA = atoi(resA[mafsA->position]);
		posB = atoi(resB[mafsB->position]);
		fprintf(stderr, "saw position %i, %i\n", posA, posB);
		
		if (iA<0 || iB<0)
			break;
		free (s_mafsA);
		free (s_mafsB);
		free (s_countsA);
		free (s_countsB);
	}
	
	free(resA);
	free(resB);
	
	// close files
	angsd_close_file(mafsA);
	angsd_close_file(mafsB);
	angsd_close_file(countsA);
	angsd_close_file(countsB);
	return 0;
}
