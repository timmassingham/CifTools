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
#include <string.h>
#include "xio.h"
#include "cif.h"
#define PROGNAME "cifjoin"
#define validate(A,B) if(!(A)){return B;}

void fprint_usage( FILE * fp){
    validate(NULL!=fp,);
    fputs(
"\t\"" PROGNAME "\"\n"
"Merge single-cycle CIF format files into whole\n"
"\n"
"Usage:\n"
"\t" PROGNAME " [-l lane] [-t tile] run-folder ...\n"
"\t" PROGNAME " --help\n"
"\t" PROGNAME " --licence\n"
"\n"
PROGNAME " reads from run-folder specified and prints merged CIF file to stdout.\n"
"\n"
"Example:\n"
"\t" PROGNAME " 100208_IL19_4344 > s_4_33.cif\n"
,fp);
}


void fprint_licence (FILE * fp){
    validate(NULL!=fp,);
    fputs(
"  " PROGNAME ": Merge single-cycle CIF format files into whole\n"
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
"-l, --lane [default: 4]\n"
"\tLane of tile to create full CIF file.\n"
"\n"
"-t, --tile [default: 33]\n"
"\tTile number to create full CIF file of.\n"
"\n"
"-h, --help\n"
"\tDisplay information about usage and exit.\n"
"\n"
"--licence\n"
"\tDisplay licence terms and exit.\n"
, fp);
}

static struct option longopts[] = {
    { "lane",       required_argument, NULL, 'l'},
    { "tile",       required_argument, NULL, 't'},
    { "help",       no_argument,       NULL, 'h' },
    { "licence",    no_argument,       NULL, 0 },
};

typedef struct {
    unsigned int lane,tile;
} OPT;
OPT opt = {4,33};

typedef char * CSTRING;


unsigned int parse_uint( const CSTRING str){
    validate(NULL!=str,0);
    unsigned int n=0;
    sscanf(str,"%u",&n);
    return n;
}


CSTRING copy_CSTRING( const CSTRING c){
    validate(NULL!=c,NULL);
    CSTRING n = calloc(strlen(c),sizeof(char));
    validate(NULL!=n,NULL);
    strcpy(n,c);
    return n;
}


void parse_arguments( const int argc, char * const argv[] ){
        int ch;
        while ((ch = getopt_long(argc, argv, "l:t:h", longopts, NULL)) != -1){
        switch(ch){
            case 'l': opt.lane = parse_uint(optarg); break;
            case 't': opt.tile = parse_uint(optarg); break;
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
        fprint_usage(stdout);
        return EXIT_FAILURE;
    }

    
    CIFDATA cif = readCIFfromDir(argv[0],opt.lane,opt.tile,XFILE_RAW);
    bool ret = writeCIFtoStream(cif,xstdout);
    free_cif(cif);
    
    return (true==ret)?EXIT_SUCCESS:EXIT_FAILURE;
}


