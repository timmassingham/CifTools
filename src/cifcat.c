/*
 *  Copyright (C) 2010 by Tim Massingham
 *  tim.massingham@ebi.ac.uk
 *
 *  This file is part of ciftool.
 *
 *  ciftool is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  ciftool is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with ciftool.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdlib.h>
#include <stdio.h>
#include <getopt.h>
#include <err.h>
#include <inttypes.h>
#include <string.h>
#include "xio.h"
#include "cif.h"
#include "utility.h"

#define PROGNAME "cifcat"


void fprint_usage( FILE * fp){
    validate(NULL!=fp,);
    fputs(
"\t\"" PROGNAME "\"\n"
"Concatenate CIF format files\n"
"\n"
"Usage:\n"
"\t" PROGNAME " file1 file2 ...\n"
"\t" PROGNAME " --help\n"
"\t" PROGNAME " --licence\n"
"\n"
PROGNAME " reads from files specified and writes concatenated file to stdout.\n"
"\n"
"Example:\n"
"\t" PROGNAME " s_4_33.cif s_4_34.cif\n"
,fp);
}


void fprint_licence (FILE * fp){
    validate(NULL!=fp,);
    fputs(
"  " PROGNAME ": Concatenate CIF format files\n"
#include "copyright.inc"
    ,fp);
}


void fprint_help( FILE * fp){
    validate(NULL!=fp,);
    fputs(
/*
12345678901234567890123456789012345678901234567890123456789012345678901234567890
*/
"\n"
"-h, --help\n"
"\tDisplay information about usage and exit.\n"
"\n"
"--licence\n"
"\tDisplay licence terms and exit.\n"
, fp);
}

static struct option longopts[] = {
    { "help",       no_argument,       NULL, 'h' },
    { "licence",    no_argument,       NULL, 0 },
    { NULL, 0 , NULL, 0}
};

typedef struct {
   bool init;
} OPT;
OPT opt = {true};

typedef char * CSTRING;
unsigned int parse_uint( const CSTRING str){
   validate(NULL!=str,0);
   unsigned int n=0;
   sscanf(str,"%u",&n);
   return n;
}

void parse_arguments( const int argc, char * const argv[] ){
        int ch;
        while ((ch = getopt_long(argc, argv, "h", longopts, NULL)) != -1){
        switch(ch){
            case 'h':
                fprint_usage(stderr);
                fprint_help(stderr);
                exit(EXIT_SUCCESS);
            case 0:
                fprint_licence(stderr);
                exit(EXIT_SUCCESS);
            default:
                fprint_usage(stderr);
                exit(EXIT_FAILURE);
            }
        }
}

int main(int argc, char * argv[]){
    parse_arguments(argc,argv);
    argc -= optind;
    argv += optind;

    // Read headers and count clusters
    int argc_ctr = argc;
    char ** argv_ctr = argv;
    uint_fast32_t ncluster = 0;
    while(argc_ctr>0){
        XFILE * fp = xfopen(argv_ctr[0],XFILE_RAW,"r");
	if(NULL==fp){
	    warnx("Failed to open %s for input.",argv_ctr[0]);
	} else {
	    fprintf(stderr,"Reading from %s ... ",argv_ctr[0]);
	    CIFDATA head = readCifHeader(fp);
	    if(NULL==head){
                fputs("Error: not cif format.\n",stderr);
	    } else {
		// error checking
	        ncluster += head->ncluster;
		fprintf(stderr,"contains %" SCNu32 " clusters.\n",head->ncluster);
	    }
	    free_cif(head);
	}
	xfclose(fp);
	argc_ctr--;
	argv_ctr++;
    }

    // Create new cif 
    CIFDATA newcif = NULL;
    argc_ctr = argc;
    argv_ctr = argv;
    uint_fast32_t cluster_ctr = 0;
    while(argc_ctr>0){
        CIFDATA cif = readCIFfromFile(argv_ctr[0],XFILE_RAW);
        if(NULL!=cif){
            if(NULL==newcif){
                newcif = new_cif();
                if(NULL==newcif){
                    errx(EXIT_FAILURE,"Out of memory.");
                }
                newcif->version       = cif->version;
	        newcif->datasize      = cif->datasize;
		newcif->firstcycle    = cif->firstcycle;
		newcif->ncycle        = cif->ncycle;
		newcif->ncluster      = ncluster;
		newcif->intensity.i8  = calloc(ncluster * cif->ncycle * NCHANNEL, cif->datasize);
            }
            for ( int cy=0 ; cy<cif->ncycle ; cy++){
		for ( int ch=0 ; ch<NCHANNEL ; ch++){
		    size_t offset_n = (cy*NCHANNEL + ch)*ncluster + cluster_ctr;
		    size_t offset_r = (cy*NCHANNEL + ch)*cif->ncluster;
                    memcpy(newcif->intensity.i8 + offset_n*cif->datasize, cif->intensity.i8 + offset_r*cif->datasize, cif->ncluster*cif->datasize);
                }
            }
	    cluster_ctr += cif->ncluster;
        }
	free_cif(cif);
	argc_ctr--;
	argv_ctr++;
    }

    writeCIFtoStream(newcif,xstdout);
    free_cif(newcif);


    return EXIT_SUCCESS;
}


