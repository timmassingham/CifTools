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

#define PROGNAME "cifsplit"
#define validate(A,B) if(!(A)){return B;}

void fprint_usage( FILE * fp){
    validate(NULL!=fp,);
    fputs(
"\t\"" PROGNAME "\"\n"
"Split CIF format files\n"
"\n"
"Usage:\n"
"\t" PROGNAME " [-o offset] [-s suffix] file.cif\n"
"\t" PROGNAME " --help\n"
"\t" PROGNAME " --licence\n"
"\n"
PROGNAME " reads from file specified and writes out paired files.\n"
"If no file is specified on commandline, " PROGNAME " reads from stdin.\n"
"\n"
"Example:\n"
"\t" PROGNAME " s_4_33.cif\n"
,fp);
}


void fprint_licence (FILE * fp){
    validate(NULL!=fp,);
    fputs(
"  " PROGNAME ": Split CIF format files\n"
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
"-o, --offset offset [default: half number of cycles of file\n"
"\tOffset of second read in paired-end file. Run is split into two\n"
"at this offset, so pairs do not have to have equal length. The default is to\n"
"split the read equally in two.\n"
"\n"
"-s, --suffix suffix [default: \"_end\"]\n"
"\tSuffix which to add to names to identify pairs. The original file\n"
"\"s_4_33.cif\" will have pairs \"s_4_33_end1.cif\" and \"s_4_33_end2.cif\"\n"
"by default. If the read was read from stdin, pairs are written to\n"
"\"intensities_end1.cif\" and \"intensities_end2.cif\"\n"
, fp);
}

static struct option longopts[] = {
    { "offset",     required_argument, NULL, 'o'},
    { "suffix",     required_argument, NULL, 's'},
    { "help",       no_argument,       NULL, 'h' },
    { "licence",    no_argument,       NULL, 0 },
    { NULL, 0 , NULL, 0}
};
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


typedef struct {
    unsigned int offset;
    CSTRING suffix,name;
} OPT;
OPT opt = { 0, "_end","intensities.cif"};

                                  
void parse_arguments( const int argc, char * const argv[] ){
        int ch;
        while ((ch = getopt_long(argc, argv, "o:s:h", longopts, NULL)) != -1){
        switch(ch){
            case 'o': opt.offset = parse_uint(optarg); break;
            case 's': opt.suffix = copy_CSTRING(optarg); break;
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

uint32_t suffix_offset( const CSTRING fn ){
	const size_t len = strlen(fn);

	for ( size_t i=len-1 ; i>0 ; i-- ){
		if ( fn[i] == '.') return i+1;
	}
	if(fn[0]=='.') return 1;
	
	return len; // Pointer to '\0' at end of string
}


int main(int argc, char * argv[]){
    parse_arguments(argc,argv);
    argc -= optind;
    argv += optind;

    CIFDATA cif = NULL;
    if( argc==0 ){
fputs("Reading from stdin",stdout);
        cif = readCIFfromStream(xstdin);
    } else {
fprintf(stdout,"Reading from %s\n",argv[0]);
        opt.name = argv[0];
        cif = readCIFfromFile(argv[0],XFILE_UNKNOWN);
    }
    
    if(0==opt.offset){
        opt.offset = cif->ncycle/2;
    }

    // Sort out template for name
    CSTRING cifname = NULL;
    uint32_t name_offset=0;
    const uint32_t suff_offset = suffix_offset(opt.name) - 1;
    const uint32_t suff_len = strlen(opt.suffix);
    const uint32_t name_len = strlen(opt.name);
    cifname = calloc(2+name_len+suff_len,sizeof(char));
    strncpy(cifname,opt.name,suff_offset);
    strncpy(cifname+suff_offset,opt.suffix,suff_len);
    strncpy(cifname+suff_offset+suff_len+1,opt.name+suff_offset,name_len-suff_offset+1);
    name_offset = suff_offset + suff_len;
    

    CIFDATA newcif = NULL;
    // End 1
    cifname[name_offset] = '1';
    newcif = spliceCIF(cif,opt.offset,0);
    writeCIFtoFile(newcif,cifname,XFILE_RAW);
    free_cif(newcif);
    // End 2
    cifname[name_offset] = '2';
    newcif = spliceCIF(cif,cif->ncycle-opt.offset,opt.offset);
    writeCIFtoFile(newcif,cifname,XFILE_RAW);
    free_cif(newcif);

    free_cif(cif);
    return EXIT_SUCCESS;
}


