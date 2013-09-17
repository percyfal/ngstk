/**
 * @file   main.c
 * @author Per Unneberg
 * @date   Fri Sep 13 18:18:55 2013
 *
 * @brief  C library toolkit for next-generation sequencing
 *
 *
 */
#include <stdio.h>
#include <string.h>
#include "main.h"

#ifndef PACKAGE_VERSION
#define PACKAGE_VERSION "0.1.0"
#endif

static int usage()
{
	fprintf(stderr, "\n");
	fprintf(stderr, "Program: ngstk (C library toolkit for next-generation sequencing)\n");
	fprintf(stderr, "Version: %s\n", PACKAGE_VERSION);
	fprintf(stderr, "Contact: Per Unneberg <punneberg@gmail.com>\n\n");
	fprintf(stderr, "Usage:   ngstk <command> [options]\n\n");
	fprintf(stderr, "Command: angsd      Parse output from angsd\n");
	fprintf(stderr, "\n");
	return 1;
}

int main(int argc, char *argv[])
{
	if (argc < 2) return usage();
	if (strcmp(argv[1], "angsd") == 0) return angsd(argc-1, argv+1);
	else {
		fprintf(stderr, "[main] unrecognized command '%s'\n", argv[1]);
		return 1;
	}
	return 0;
}
