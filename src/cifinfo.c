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
#include "xio.h"
#include "cif.h"
#include "utility.h"

#define PROGNAME "cifinfo"


void fprint_usage( FILE * fp){
    validate(NULL!=fp,);
    fputs(
"\t\"" PROGNAME "\"\n"
"Display information about CIF format files\n"
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
"  " PROGNAME ": Displays information about CIF format files\n"
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
    { "cluster",    required_argument, NULL, 'l' },
    { "cycle",      required_argument, NULL, 'y' },
    { "help",       no_argument,       NULL, 'h' },
    { "licence",    no_argument,       NULL, 0 },
};

typedef struct {
   uint32_t ncycle,ncluster;
} OPT;
OPT opt = {5,5};

typedef char * CSTRING;
unsigned int parse_uint( const CSTRING str){
   validate(NULL!=str,0);
   unsigned int n=0;
   sscanf(str,"%u",&n);
   return n;
}

void parse_arguments( const int argc, char * const argv[] ){
        int ch;
        while ((ch = getopt_long(argc, argv, "l:y:h", longopts, NULL)) != -1){
        switch(ch){
	    case 'l': opt.ncluster = parse_uint(optarg); break;
	    case 'y': opt.ncycle = parse_uint(optarg); break;
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

    if( argc==0 ){
        CIFDATA cif = readCIFfromStream(xstdin);
        showCIF(xstdout,cif,opt.ncluster,opt.ncycle);
        free_cif(cif);
    } else {
        for( uint32_t i=0 ; i<argc ; i++){
            CIFDATA cif = readCIFfromFile(argv[i],XFILE_UNKNOWN);
            showCIF(xstdout,cif,opt.ncluster,opt.ncycle);
            free_cif(cif);
        }
    }
    return EXIT_SUCCESS;
}


