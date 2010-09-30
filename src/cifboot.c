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
#include <time.h>
#include <err.h>
#include <string.h>
#include <assert.h>
#include "xio.h"
#include "cif.h"
#include "utility.h"
#include "random.h"

#define PROGNAME "cifboot"


void fprint_usage( FILE * fp){
    validate(NULL!=fp,);
    fputs(
"\t\"" PROGNAME "\"\n"
"Display information about CIF format files\n"
"\n"
"Usage:\n"
"\t" PROGNAME " [-b boot] [-n nsample] [-p prefix] [-s seed] file\n"
"\t" PROGNAME " --help\n"
"\t" PROGNAME " --licence\n"
"\n"
PROGNAME " reads from cif file and samples clusters randomly\n"
"If no files are specified on commandline, " PROGNAME " reads from stdin.\n"
"\n"
"Example:\n"
"\t" PROGNAME " -b 1000 -n 100 s_4_33.cif\n"
,fp);
}


void fprint_licence (FILE * fp){
    validate(NULL!=fp,);
    fputs(
"  " PROGNAME ": Samples from CIF format files\n"
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
"-b, --boot boot [default: 100]\n"
"\tNumber of bootstraps to create\n"
"\n"
"-n, --nsample n [default: number of clusters]\n"
"\tNumber of clusters in each bootstrap. Default is the same as the\n"
"input file.\n"
"\n"
"-p, --prefix prefix [default: \"boot_\"]\n"
"\tPrefix for bootstrap files. Output bootstrap files will be named\n"
"prefixXXX.cif, where XXX is XXX'th bootstrap field-width chosen to\n"
"accomodate maximum value.\n"
"\n"
"-s, --seed seed [default: from clock]\n"
"\tSeed for random number generator\n"
"\n"
"-h, --help\n"
"\tDisplay information about usage and exit.\n"
"\n"
"--licence\n"
"\tDisplay licence terms and exit.\n"
, fp);
}

static struct option longopts[] = {
    { "boot",       required_argument, NULL, 'b'},
    { "nsample",    required_argument, NULL, 'n'},
    { "prefix",     required_argument, NULL, 'p'},
    { "seed",       required_argument, NULL, 's'},
    { "help",       no_argument,       NULL, 'h' },
    { "licence",    no_argument,       NULL, 0 },
};

typedef struct {
	unsigned int boot,nsample,seed;
	char * prefix;
} OPT;

OPT opt = {100,0,0,"boot_"};

unsigned int parse_uint( const char * str){
    validate(NULL!=str,0);
    unsigned int n=0;
    sscanf(str,"%u",&n);
    return n;
}

char * copy_string(const char * str){
	if(NULL==str){ return NULL;}
	char * cstr = calloc(strlen(str)+1,sizeof(char));
        if(NULL==cstr){ return NULL; }
	strcpy(cstr,str);
	return cstr;
}
	


void parse_arguments( const int argc, char * const argv[] ){
        int ch;
        while ((ch = getopt_long(argc, argv, "b:n:p:s:h", longopts, NULL)) != -1){
        switch(ch){
	    case 'b': opt.boot = parse_uint(optarg); break;
	    case 'n': opt.nsample = parse_uint(optarg); break;
            case 'p': opt.prefix = copy_string(optarg);
                      if(NULL==opt.prefix){
			errx(EXIT_FAILURE,"Failed to parse prefix string \"%s\"",optarg);
		      }
                      break;
	    case 's': opt.seed = parse_uint(optarg); break;
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
    CIFDATA cif = NULL;
    parse_arguments(argc,argv);
    argc -= optind;
    argv += optind;

    // Initialise random number generator
    if ( opt.seed==0 ){
        uint32_t seed = (uint32_t) time(NULL);
        fprintf(stderr,"Using seed %u\n",seed);
        opt.seed = seed;
    }
    init_gen_rand( opt.seed );

    
    if( argc==0 ){
        cif = readCIFfromStream(xstdin);
    } else {
        cif = readCIFfromFile(argv[0],XFILE_UNKNOWN);
    }
    if(NULL==cif){
	errx(EXIT_FAILURE,"Failed to read cif file");
    }
    assert(cif->datasize==2);


    if(0==opt.nsample){
	opt.nsample = cif->ncluster;
    }

    unsigned int bufsp = 0;
    {
        int a = opt.boot + 1;
        while(a>0){
            a /= 10;
            bufsp++;
        }
    }
    unsigned int fn_off = strlen(opt.prefix);
    char * filename = calloc(fn_off+bufsp+4+1,sizeof(char));
    strcpy(filename,opt.prefix);
    strcpy(filename+fn_off+bufsp,".cif");
    
    for ( unsigned int i=0 ; i<opt.boot ; i++){
	// Make filename

        CIFDATA cifboot = new_cif();
        if(NULL==cifboot){
		errx(EXIT_FAILURE,"Failed to allocate memory for cifboot");
	}
	cifboot->ncycle = cif->ncycle;
	cifboot->firstcycle = cif->firstcycle;
	cifboot->ncluster = opt.nsample;
	cifboot->intensity.i8 = calloc(opt.nsample*cif->ncycle*4*cifboot->datasize,sizeof(char));
        if(NULL==cifboot->intensity.i8){
		errx(EXIT_FAILURE,"Failed to allocate memory for intensities");
        }

	// Do Sampling
        for ( unsigned int j=0 ; j<cifboot->ncluster ; j++){
           uint32_t idx = gen_rand64()%cif->ncluster;
           for( int cy=0 ; cy<cif->ncycle ; cy++){
               for( int base=0 ; base<4 ; base++){
                   cifboot->intensity.i16[(cy*4+base)*cifboot->ncluster+j] = cif->intensity.i16[(cy*4+base)*cif->ncluster+idx];
               }
           }
        }
        
        for( int j=0 ; j<bufsp ; j++){
		filename[fn_off+j] = '0';
        }
        unsigned int j = i + 1;
        unsigned pos = bufsp-1;
        while( j>0){
           filename[fn_off+pos] = (j%10) + 48;
           j /= 10;
           pos--;
        }
	
	writeCIFtoFile(cifboot,filename,XFILE_RAW);
	free_cif(cifboot);
	fprintf(stderr,"\rDone: %05u",i+1);
    }
    fputc('\n',stderr);
    free_cif(cif);
    return EXIT_SUCCESS;
}


