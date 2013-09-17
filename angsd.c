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
#include <math.h>
#include "angsd.h"



struct __angsd_io_t {
	angsdFile fp;
	char buffer[2*BUFLEN];
	// Columns of interest
	int chromo, position, major, minor, anc, knownEM, nInd;
	// Total number of individuals 
	int nind_tot;
	// Line number
	//int linenum;
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


mafs_t *set_mafs_results(angsd_io_t *mafs, angsd_io_t *counts)
{
	int i;
	size_t N=256;
	char **res;
	char *input_str;
	mafs_t *mafs_res;
	input_str = (char *) malloc (N);

	// Get line
	i = angsd_getline(&input_str, &N, mafs);
	if (i < 0)
		return NULL;

	// Split string
	res = splitstr(input_str);
	
	// Set the results
	mafs_res = (mafs_t *)calloc (1, sizeof (mafs_t));
	mafs_res->coverage = (int *) malloc (counts->nind_tot * sizeof(int));
	mafs_res->position = atoi(res[mafs->position]);
	mafs_res->nInd = atoi(res[mafs->nInd]);
	strcpy(mafs_res->chromo, res[mafs->chromo]);
	strcpy(mafs_res->major, res[mafs->major]);
	strcpy(mafs_res->minor, res[mafs->minor]);
	strcpy(mafs_res->anc, res[mafs->anc]);
	if (strcmp(mafs_res->anc, mafs_res->minor)==0)
		mafs_res->allele_freq =  1.0 - atof(res[mafs->knownEM]);
	else
		mafs_res->allele_freq = atof(res[mafs->knownEM]);

	// Get counts
	i = angsd_getline(&input_str, &N, counts);
	// Split string
	res = splitstr(input_str);
	int j;
	for (j=0; j<counts->nind_tot; j++)
		mafs_res->coverage[j] = atoi(res[j]);
	free(input_str);
	free(res);
	
	return mafs_res;
}


int angsd(int argc, char *argv[])
{
	int c = -1;
	char popA[256], popB[256];

	// Initialize options
	opt_t *angsd_opt;
	angsd_opt = (opt_t*)calloc(1, sizeof(opt_t));
	angsd_opt->coverage=COVERAGE;
	angsd_opt->in_fraction=IN_FRACTION;
	angsd_opt->gridsize=GRIDSIZE;
	
	while ((c = getopt(argc, argv, "c:f:")) >= 0) {
		switch (c) {
		case 'c':
			angsd_opt->coverage = atoi(optarg);
			break;
		case 'f':
			angsd_opt->in_fraction = atof(optarg);
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

	// setup frequency matrix
	int freq[angsd_opt->gridsize][angsd_opt->gridsize];
	int k,l;
	for (k = 0; k < angsd_opt->gridsize; k++) 
		for (l = 0; l < angsd_opt->gridsize; l++)
			freq[k][l] = 0;
	
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
	size_t N = 256;
	int posA=-1, posB=-2;
	while (1) {
		mafs_t *mafs_resA, *mafs_resB;
		if (posA != posB) {
			while (posA != posB) {
				if (posA < posB) {
					fprintf(stderr, "position %i not in B\n", posA);
					mafs_resA = set_mafs_results(mafsA, countsA);
					if (mafs_resA == NULL)
						break;
					posA = mafs_resA->position;
				} else {
					fprintf(stderr, "position %i not in A\n", posB);
					mafs_resB = set_mafs_results(mafsB, countsB);
					if (mafs_resB == NULL)
						break;
					posB = mafs_resB->position;
				}
				if (mafs_resA == NULL || mafs_resB == NULL) {
					fprintf(stderr, "breaking at  position %i, %i\n", posA, posB);
					break;
				}
			}
		} else {
			mafs_resA = set_mafs_results(mafsA, countsA);
			mafs_resB = set_mafs_results(mafsB, countsB);
			if (mafs_resA == NULL || mafs_resB == NULL) {
				fprintf(stderr, "breaking at  position %i, %i\n", posA, posB);
				break;
			}
			posA = mafs_resA->position;
			posB = mafs_resB->position;
		}
		if (posA % 10000 == 0) {
			fprintf(stderr, "saw position %i, %i\n", posA, posB);
			fprintf(stderr, "%s %i\n", mafs_resA->chromo, mafs_resA->position);
		}
		
		// process results
		int ncovA = 0, ncovB = 0, i;
		for (i=0; i<countsA->nind_tot; i++)
			if (mafs_resA->coverage[i] >= angsd_opt->coverage)
				ncovA++;
		for (i=0; i<countsB->nind_tot; i++)
			if (mafs_resB->coverage[i] >= angsd_opt->coverage)
				ncovB++;
		// See if fraction ok for both
		if (((1.0 *ncovA/countsA->nind_tot) >= angsd_opt->in_fraction) && ((1.0 * ncovB/countsB->nind_tot) >= angsd_opt->in_fraction)) {
			int iA = floor(mafs_resA->allele_freq * (angsd_opt->gridsize - 1));
			int iB = floor(mafs_resB->allele_freq * (angsd_opt->gridsize - 1));
			if (iA >=100 || iB >= 100)
				fprintf(stderr, "iA: %i, iB: %i\n", iA, iB);
			
			freq[iA][iB]++;
		}
		
		if (mafs_resA && mafs_resB) {
			if (mafs_resA->position < mafs_resB->position) {
				// only free resA
				fprintf(stderr, "saw mafs_resA @%x\n", &mafs_resA);
				free((void *) mafs_resA->coverage);
				free(mafs_resA);
			}
			else if (mafs_resB->position < mafs_resA->position) {
				// only free resB
				fprintf(stderr, "saw mafs_resB @%x\n", &mafs_resB);
				free((void *) mafs_resB->coverage);
				free(mafs_resB);
			} else {
				free((void *) mafs_resA->coverage);
				free((void *) mafs_resB->coverage);
				free(mafs_resA);
				free(mafs_resB);
			}
		}
	}
	
	// close files
	angsd_close_file(mafsA);
	angsd_close_file(mafsB);
	angsd_close_file(countsA);
	angsd_close_file(countsB);

	// print freq matrixs
	for (k=0; k<angsd_opt->gridsize; k++) {
		for (l=0; l<angsd_opt->gridsize; l++)
			fprintf(stdout, "%i ", freq[k][l]);
		fprintf(stdout, "\n");
	}
	fprintf(stdout, "\n");
	
	return 0;
}
