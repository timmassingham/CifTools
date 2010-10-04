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
#include <tgmath.h>
#include "xio.h"
#include "cif.h"
#include "tile.h"
#include "utility.h"

#define PROGNAME "int2cif"

void fprint_usage( FILE * fp){
    validate(NULL!=fp,);
    fputs(
"\t\"" PROGNAME "\"\n"
"Convert int format files to CIF\n"
"\n"
"Usage:\n"
"\t" PROGNAME " -r readlen file ...\n"
"\t" PROGNAME " --help\n"
"\t" PROGNAME " --licence\n"
"\n"
PROGNAME " reads from specified file and writes CIF to stdout.\n"
"If no files are specified on commandline, " PROGNAME " reads from stdin.\n"
"\n"
"Example:\n"
"\t" PROGNAME " -r 45 s_4_0033_int.txt.gz > s_4_0033.cif\n"
,fp);
}


void fprint_licence (FILE * fp){
    validate(NULL!=fp,);
    fputs(
"  " PROGNAME ": Convert int format files to CIF\n"
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
"-r, --readlen read length [required argument]\n"
"\tNumber of cycles in intensity data to read.\n"
"\n"
"-h, --help\n"
"\tDisplay information about usage and exit.\n"
"\n"
"--licence\n"
"\tDisplay licence terms and exit.\n"
, fp);
}

typedef char * CSTRING;
unsigned int parse_uint( const CSTRING str){
    validate(NULL!=str,0);
    unsigned int n=0;
    sscanf(str,"%u",&n);
    return n;
}

static struct option longopts[] = {
    { "readlen",    required_argument, NULL, 'r' },
    { "help",       no_argument,       NULL, 'h' },
    { "licence",    no_argument,       NULL, 0 },
    { NULL, 0 , NULL, 0}
};

typedef struct {
    unsigned int ncycle;
} OPT;

OPT opt = { .ncycle=0 };

void parse_arguments( const int argc, char * const argv[] ){
        int ch;
        while ((ch = getopt_long(argc, argv, "r:h", longopts, NULL)) != -1){
        switch(ch){
            case 'r': opt.ncycle = parse_uint(optarg);
                      if(opt.ncycle==0){
                          errx(EXIT_FAILURE,"Read length should be positive.");
                      }
                      break;
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
    XFILE * fp = xstdin;
    parse_arguments(argc,argv);
    argc -= optind;
    argv += optind;

    if( opt.ncycle==0){
        errx(EXIT_FAILURE,"Read length must be specified.");
    }
    
    if( argc!=0 ){
        fp = xfopen(argv[0],XFILE_UNKNOWN,"r");
        if(NULL==fp){
            errx(EXIT_FAILURE,"Failed to open \"%s\" for input.",argv[0]);
        }
    }
    
    
    TILE tile = read_known_TILE(fp,opt.ncycle);
    if(NULL==tile){
        errx(EXIT_FAILURE,"Failure parsing tile.");
    }
    tile->clusterlist = reverse_inplace_list_CLUSTER(tile->clusterlist);
    
    CIFDATA cif = new_cif();
    cif->ncycle = opt.ncycle;
    cif->ncluster = tile->ncluster;
    cif->intensity.i16 = malloc(NCHANNEL*cif->ncycle*cif->ncluster*cif->datasize);
    // Create CIF data structure
    LIST(CLUSTER) node = tile->clusterlist;
    uint32_t ncl = 0;
    while( NULL!=node && ncl<tile->ncluster){
        CLUSTER cl = node->elt;
        for ( uint32_t cy=0 ; cy<opt.ncycle ; cy++){
            for ( uint32_t ch=0 ; ch<NCHANNEL ; ch++){
                cif->intensity.i16[cy*NCHANNEL*tile->ncluster + ch*tile->ncluster + ncl] = (int16_t)(round(cl->signals->x[cy*NCHANNEL+ch]));
            }
        }
        node = node->nxt;
        ncl++;
    }
    if(ncl!=tile->ncluster){
        errx(EXIT_FAILURE,"Conversion error: number of clusters doesn't match %s:%d",__FILE__,__LINE__);
    }
    
    writeCIFtoStream(cif,xstdout);
    return EXIT_SUCCESS;
}


