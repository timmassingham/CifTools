/*
 *  Copyright (C) 2010 by Tim Massingham
 *  tim.massingham@ebi.ac.uk
 *
 *  This file is part of the ciftool.
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
#include <stdint.h>
#include <err.h>
#include <string.h>
#include "matrix.h"


#define WARN_MEM(A) warn("Failed to allocation memory for %s at %s:%d.\n",(A),__FILE__,__LINE__)

/*  First deal with allocation and deallocation of matrices  */
/* Allocate memory for matrix of a specified size */
MAT new_MAT( const int nrow, const int ncol ){
    MAT mat = malloc(sizeof(*mat));
    if ( NULL==mat){
        WARN_MEM("matrix");
        return NULL;
    }
    mat->ncol=ncol;
    mat->nrow=nrow;
    /* Number of rows or columns might be zero but probably an error, so warn
     * as such. Want to avoid malloc(0) since this is "implementation defined"
     * in the C standard, may be a real address that should not be used or NULL
     */
     if ( 0==ncol || 0==nrow ){
         warn("One of dimensions of matrix equal to zero. Setting memory to NULL.\n");
         mat->x = NULL;
     } else {
         /* Usual case. Use calloc rather than malloc so elements are
          * initialised
          */
         mat->x = calloc(nrow*ncol,sizeof(real_t));
         if ( NULL==mat->x){
             WARN_MEM("matrix elements");
             free(mat);
             mat = NULL;
         }
     }
     
     return mat;
}

MAT new_MAT_from_array( const int nrow, const int ncol, const real_t * x){
    if(NULL==x){ return NULL;}
    MAT mat = new_MAT(nrow,ncol);
    if(NULL==mat){return NULL;}
    memcpy(mat->x,x,nrow*ncol*sizeof(real_t));
    return mat;
}

/* Free memory allocated for matrix */
void free_MAT ( MAT mat ){
    if(NULL==mat){ return; }
    /* Memory for elements may be NULL if nrow or ncol equals zero */
    if ( NULL!=mat->x){
        free(mat->x);
    }
    free(mat);
}

    

/*  Vec transpose operation */
MAT vectranspose ( const MAT mat, const unsigned int p ){
    /* Simple checking of arguments */
    if ( NULL==mat){
        warn("Attempting to apply vec-transpose operation to a NULL matrix");
        return NULL;
    }
    if ( 0==mat->nrow || 0==mat->ncol ){
        warn("Vec-transpose of matrix with no columns or no rows is not completely defined\n");
    }
    if ( 0!=(mat->nrow % p) ){
        warn("Invalid application of vec-transpose(%u) to a matrix with %u rows.\n",p,mat->nrow);
        return NULL;
    }
    
    
    MAT vtmat = new_MAT(p*mat->ncol,mat->nrow/p);
    if ( NULL==vtmat){
        return NULL;
    }
    
    /*  Iterate through columns of matrix doing necessary rearrangement.
     * Note: assumed column-major format
     * Each column is split into imaginary subcolumns of length p,
     * which will form the submatrix corresponding to that column.
     * Refer to external documentation for definition of vec-tranpose
     * (give document reference here).
     */
    /* Routine is valid for matrices with zero columns or rows since it 
     * it will never attempt to access elements in this case
     */
     for ( unsigned int col=0 ; col<mat->ncol ; col++){
         /* offset represents where in the "matrix stack" that the column
          * should be formed into.*/
         unsigned int offset = col*p;
         for ( unsigned int subcol=0 ; subcol<(mat->nrow/p) ; subcol++){
             for ( unsigned int i=0 ; i<p ; i++){
                 vtmat->x[ (subcol*vtmat->nrow) + offset + i ] 
                        = mat->x[col*mat->nrow + subcol*p + i];
             }
         }
     }
     
     return vtmat;
}

void show_MAT ( XFILE * fp, const MAT mat, const int mrow, const int mcol){
    if(NULL==fp){ return;}
    if(NULL==mat){ return;}
    
    const int nrow = mat->nrow;
    const int ncol = mat->ncol;
    const int maxrow = (mrow!=0 && mrow<nrow)?mrow:nrow;
    const int maxcol = (mcol!=0 && mcol<ncol)?mcol:ncol;
    for( int row=0 ; row<maxrow ; row++){
        xfprintf(fp,"%d:",row+1);
        for ( int col=0 ; col<maxcol ; col++){
            xfprintf(fp," %#8.2f",mat->x[col*nrow+row]);
        }
        if(maxcol<ncol){ xfprintf(fp,"\t... (%u others)",ncol-maxcol); }
        xfputc('\n',fp);
    }
    if( maxrow<nrow){ xfprintf(fp,"... (%u others)\n",nrow-maxrow); }
}

MAT copy_MAT( const MAT mat){
    if(NULL==mat){ return NULL;}

    MAT newmat = new_MAT(mat->nrow,mat->ncol);
    if(NULL==newmat){ return NULL;}
    
    memcpy(newmat->x,mat->x,mat->nrow*mat->ncol*sizeof(real_t));
    return newmat;
}

MAT copyinto_MAT( MAT newmat, const MAT mat){
    if(newmat->nrow!=mat->nrow){ return NULL;}
    if(newmat->ncol!=mat->ncol){ return NULL;}
    memcpy(newmat->x,mat->x,mat->nrow*mat->ncol*sizeof(real_t));
    return newmat;
}

bool is_square(const MAT mat){
    validate(NULL!=mat,false);
    if(mat->nrow!=mat->ncol){ return false;}
    return true;
}


// Copy lower diagonal of matrix to upper diagonal
MAT symmeteriseL2U( MAT mat){
    validate(NULL!=mat,NULL);
    validate(mat->nrow==mat->ncol,NULL);
    const int n = mat->ncol;
    for ( int col=0 ; col<n ; col++){
        for ( int row=col ; row<n ; row++){
            mat->x[row*n+col] = mat->x[col*n+row];
        }
    }
    return mat;
}

MAT identity_MAT( const int nrow){
    MAT mat = new_MAT(nrow,nrow);
    validate(NULL!=mat,NULL);
    for ( int i=0 ; i<nrow ; i++){
        mat->x[i*nrow+i] = 1.0;
    }
    return mat;
}

MAT reshape_MAT( MAT mat, const int nrow){
    validate(NULL!=mat,NULL);
    validate(((mat->nrow*mat->ncol)%nrow)==0,NULL);
    mat->ncol = (mat->nrow*mat->ncol)/nrow;
    mat->nrow = nrow;
    return mat;
}

MAT trim_MAT( MAT mat, const int mrow, const int mcol, const bool forwards){
    validate(NULL!=mat,NULL);
    validate(mrow>=0,NULL);
    validate(mcol>=0,NULL);
    validate(mrow<=mat->nrow,NULL);
    validate(mcol<=mat->ncol,NULL);
    if(forwards==false){ errx(EXIT_FAILURE,"Forwards==false not implemented in %s (%s:%d)\n",__func__,__FILE__,__LINE__);}
    for ( int col=0 ; col<mcol ; col++){
        int midx = col*mrow;
        int nidx = col*mat->nrow;
        memmove(mat->x+midx,mat->x+nidx,mrow*sizeof(real_t));
    }
    mat->nrow = mrow;
    mat->ncol = mcol;
    return mat;
}

MAT * block_diagonal_MAT( const MAT mat, const int n){
    validate(NULL!=mat,NULL);
    validate(mat->ncol==mat->nrow,NULL);    // Ensure symmetry
    const int nelts = mat->ncol / n;        // Number of blocks on diagonal
    validate((mat->ncol % n)==0,NULL);      // Is parameter valid?
    
    // Create memory
    MAT * mats = calloc(nelts,sizeof(*mats));
    if(NULL==mats){ goto cleanup; }
    for ( int i=0 ; i<nelts ; i++){
        mats[i] = new_MAT(n,n);
        if(NULL==mats[i]){ goto cleanup;}
    }
    // Copy into diagonals
    for ( int i=0 ; i<nelts ; i++){
        for ( int col=0 ; col<n ; col++){
            const int oldcol = i*n+col;
            for ( int row=0 ; row<n ; row++){
                const int oldrow = i*n+row;
                mats[i]->x[col*n+row] = mat->x[oldcol*mat->nrow+oldrow];
            }
        }
    }
    return mats;
    
cleanup:
    if(NULL!=mats){
        for ( int i=0 ; i<nelts ; i++){
            free_MAT(mats[i]);
        }
    }
    xfree(mats);
    return NULL;
}

MAT scale_MAT(MAT mat, const real_t f){
    validate(NULL!=mat,NULL);
    const int nelt = mat->ncol * mat->nrow;
    for ( int elt=0 ; elt<nelt ; elt++){
            mat->x[elt] *= f;
    }
    return mat;
}
