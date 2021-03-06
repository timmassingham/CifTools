/*
 *  Copyright (C) 2008,2009 by Tim Massingham
 *  tim.massingham@ebi.ac.uk
 *
 *  This file is part of the ciftool base-calling software.
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

#ifndef _CIF_H
#define _CIF_H

#include "xio.h"
#include <stdbool.h>
#include <stdint.h>

#define NCHANNEL    4
typedef union { int8_t * i8; int16_t * i16; int32_t * i32;} encInt;

struct cifData {
    uint8_t version, datasize;
    uint16_t firstcycle, ncycle;
    uint32_t ncluster;
    encInt intensity;
};
typedef struct cifData * CIFDATA;


// Deletion
CIFDATA new_cif ( void );
void free_cif ( CIFDATA cif);
void showCIF ( XFILE * ayb_fp, const CIFDATA cif, uint32_t mcluster, uint32_t mcycle);

// Other 
CIFDATA readCIFfromFile ( const char * fn, const XFILE_MODE mode);
CIFDATA readCIFfromStream ( XFILE * ayb_fp );
CIFDATA readCIFfromDir ( const char * fn, const uint32_t lane, const uint32_t tile, const XFILE_MODE mode);
CIFDATA readCifHeader (XFILE * ayb_fp);
bool writeCIFtoFile ( const CIFDATA  cif, const char * fn, const XFILE_MODE mode);
bool writeCIFtoStream ( const CIFDATA  cif, XFILE * ayb_fp);
bool write2CIFfile ( const char * fn, const XFILE_MODE mode, const encInt  intensities, const uint16_t firstcycle, const uint32_t ncycle, const uint32_t ncluster, const uint8_t nbyte);
CIFDATA spliceCIF(const CIFDATA cif, uint32_t ncycle, size_t offset);

#endif
