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
#include <assert.h>
#include "xio.h"
#include "cif.h"
#include "utility.h"

#define PROGNAME "cifstat"


void fprint_usage( FILE * fp){
    validate(NULL!=fp,);
    fputs(
"\t\"" PROGNAME "\"\n"
"Display statistics about CIF format files\n"
"\n"
"Usage:\n"
"\t" PROGNAME " file1 file2 ...\n"
"\t" PROGNAME " --help\n"
"\t" PROGNAME " --licence\n"
"\n"
PROGNAME " reads from files specified and prints summary to stdout.\n"
"If no files are specified on commandline, " PROGNAME " reads from stdin.\n"
"\n"
"Example:\n"
"\t" PROGNAME " s_4_33.cif\n"
,fp);
}


void fprint_licence (FILE * fp){
    validate(NULL!=fp,);
    fputs(
"  " PROGNAME ": Displays statistics about CIF format files\n"
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

/*typedef struct {
   uint32_t ncycle,ncluster;
} OPT;
OPT opt = {5,5};*/

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

void showStat(XFILE * xfp, const CIFDATA cif){
	if(NULL==xfp || NULL==cif){ return; }
	const uint32_t ncluster = cif->ncluster;
	const uint32_t ncycle = cif->ncycle;

	// Initialise memory
	double meanInts[ncycle*NCHANNEL];
	for ( size_t i=0 ; i<ncycle*NCHANNEL ; i++){
		meanInts[i] = 0.0;
	}
	uint64_t numZeros[ncycle];
	uint64_t nZeroCycle[ncycle+1];
	for ( size_t i=0 ; i<ncycle ; i++){
		numZeros[i] = 0;
		nZeroCycle[i] = 0;
	}

	// Iterate through clusters
	assert(cif->datasize==2);
	for ( size_t cl=0 ; cl<ncluster ; cl++){
		uint32_t nzero = 0;
		for ( size_t cy=0 ; cy<ncycle ; cy++){
			bool iszero =
			(0 == cif->intensity.i16[cy*NCHANNEL*ncluster+NCHANNEL*cl])   &&
			(0 == cif->intensity.i16[cy*NCHANNEL*ncluster+NCHANNEL*cl+1]) &&
			(0 == cif->intensity.i16[cy*NCHANNEL*ncluster+NCHANNEL*cl+2]) &&
			(0 == cif->intensity.i16[cy*NCHANNEL*ncluster+NCHANNEL*cl+3]);
			if(iszero){
				numZeros[cy]++;
				nzero++;
			}
			meanInts[cy*NCHANNEL] += cif->intensity.i16[cy*NCHANNEL*ncluster+NCHANNEL*cl];
			meanInts[cy*NCHANNEL+1] += cif->intensity.i16[cy*NCHANNEL*ncluster+NCHANNEL*cl+1];
			meanInts[cy*NCHANNEL+2] += cif->intensity.i16[cy*NCHANNEL*ncluster+NCHANNEL*cl+2];
			meanInts[cy*NCHANNEL+3] += cif->intensity.i16[cy*NCHANNEL*ncluster+NCHANNEL*cl+3];
		}
		nZeroCycle[nzero]++;
	}
	for ( size_t i=0 ; i<ncycle*NCHANNEL ; i++){
		meanInts[i] /= ncluster;
	}

	// Print results
	xfprintf(xfp,"Ncluster %8d\nNcycle   %8d\n\n",ncluster,ncycle);
	xfprintf(xfp,"Cycle\tMeanA\tMeanC\tMeanG\tMeanT\tNumZeros\tTotalZeros\n");
	xfprintf(xfp,"%4d\t\t\t\t\t\t\t%8d\n",0,nZeroCycle[0]);
	for ( size_t cy=0 ; cy<ncycle ; cy++){
		xfprintf(xfp,"%4d\t%6.1f\t%4.1f\t%6.1f\t%6.1f\t%8d\t%8d\n",cy+1,meanInts[cy*NCHANNEL],meanInts[cy*NCHANNEL+1],
				meanInts[cy*NCHANNEL+2],meanInts[cy*NCHANNEL+3],numZeros[cy],nZeroCycle[cy+1]);
	}
}

int main(int argc, char * argv[]){
    parse_arguments(argc,argv);
    argc -= optind;
    argv += optind;

    if( argc==0 ){
        CIFDATA cif = readCIFfromStream(xstdin);
        showStat(xstdout,cif);
        free_cif(cif);
    } else {
        for( uint32_t i=0 ; i<argc ; i++){
            CIFDATA cif = readCIFfromFile(argv[i],XFILE_UNKNOWN);
            showStat(xstdout,cif);
            free_cif(cif);
        }
    }
    return EXIT_SUCCESS;
}


