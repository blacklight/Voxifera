/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/
#include "fdct.h"

void fdct_init( fdct_t *dct, unsigned char *data ){
  int i, group, base, item, nitems;
    double factor;
  
  /* alloc transform vector and fill with data */
  dct->transform = (double *)calloc( FDCT_TOTSIZE, sizeof(double) );
  for( i = 0; i < FDCT_TOTSIZE; i++ ){
    dct->transform[i] = (double)data[i];
  }
  
  /* alloc and fill cosines table */
  dct->ctable = (double *)calloc( FDCT_TOTSIZE, sizeof(double) );  
  for( i = 0; i <= FDCT_TSO2 - 1; i++ ){
    dct->ctable[FDCT_TSO2 + i] = 4 * i + 1;    
  }
  
  for( group = 1; group <= FDCT_POW - 1; group++ ){
    nitems = base = 1 << (group - 1);
    factor = 1.0 * (1 << (FDCT_POW - group));
    for( item = 1; item <= nitems; item++ ){
      dct->ctable[base + item - 1] = factor * dct->ctable[FDCT_TSO2 + item - 1];
    }
  }
 
  for( i = 1; i <= FDCT_TOTSIZE - 1; i++ ){
    dct->ctable[i] = 1.0 / (2.0 * cos( dct->ctable[i] * M_PI / (2.0 * FDCT_TOTSIZE) ));
  }
  
  return dct;
}
 
inline void fdct_bitrev( double *data, unsigned int size ){
  int i, m,
    j = 1,
    hsize = size >> 1;
  double tmp;
 
  if( size <= 2 ){ return; }
 
  for( i = 1; i <= size; i++ ){
    if(i < j){
      tmp = data[j - 1];
      data[j - 1] = data[i - 1];
      data[i - 1] = tmp;
    }
    m = hsize;
    while(j > m){
      j = j - m;
      m = (m + 1) >> 1;
    }
    j = j + m;
  }
}
 
inline void fdct_scramble( double *data ){
  double tmp;
  int i, ii1, ii2;
 
  fdct_bitrev( data, FDCT_TOTSIZE );
  fdct_bitrev( &data[0], FDCT_TSO2 );
  fdct_bitrev( &data[FDCT_TSO2], FDCT_TSO2);
 
  ii1 = FDCT_TOTSIZE - 1;
  ii2 = FDCT_TSO2;
  for( i = 0; i <= FDCT_TSO4 - 1; i++, ii1--, ii2++ ){
    tmp = data[ii1];
    data[ii1] = data[ii2];
    data[ii2] = tmp;
  }
}
 
inline void fdct_butterflies( fdct_t *dct ){
  int stage, ii1, ii2,
    butterfly,
    ngroups,
    group,
    wingspan,
    inc,
    ptr;
  double cosine, value;
 
  for( stage = FDCT_POW; stage >= 1; stage-- ){
    ngroups = 1 << (FDCT_POW - stage);
    wingspan = 1 << (stage - 1);
    inc = wingspan << 1;
        
    for( butterfly = 1; butterfly <= wingspan; butterfly++ ){
      cosine = dct->ctable[ wingspan + butterfly - 1 ];
      ptr = 0;
      
      for( group = 1; group <= ngroups; group++ ){
        ii1 = ptr + butterfly - 1;
        ii2 = ii1 + wingspan;
        value = dct->transform[ii2];
        dct->transform[ii2] = cosine *(dct->transform[ii1] - value);
        dct->transform[ii1] = dct->transform[ii1] + value;
        ptr += inc;
      }
    }
  }
}
 
inline void fdct_sums(double *data){
  int stepsize,
    stage, i,
    nthreads,
    thread,
    step,
    nsteps;
 
  for( stage = FDCT_POW - 1; stage >= 1; stage-- ){
    nthreads = 1 << (stage - 1);
    stepsize = nthreads << 1;
    nsteps = (1 << (FDCT_POW - stage)) - 1;
    
    for( thread = 1; thread <= nthreads; thread++ ){
      i = nthreads + thread - 1;
      for( step = 1; step <= nsteps; step++ ){
        data[i] += data[i + stepsize];
        i += stepsize;
      }
    }
  }
}
 
inline void fdct_compute( fdct_t *dct ){
  int i;
  
  fdct_scramble( dct->transform );
    fdct_butterflies( dct );
    fdct_bitrev( dct->transform, FDCT_TOTSIZE );
    fdct_sums(dct->transform);
    
  dct->transform[0] *= FDCT_FACTOR;
  for( i = 0; i <= FDCT_TOTSIZE - 1; i++ ){
    dct->transform[i] *= FDCT_2OTS_SQ;
  }    
}
 
inline double fdct_sum( fdct_t *dct, double div, unsigned char *neutral, unsigned int neutral_size ){
  int i;
  double sum = 0.0;
  
  fdct_compute( dct );
  
  if( neutral ){
    for( i = 0; i < FDCT_TOTSIZE; i++ ){
      sum += dct->transform[i] - neutral[i % neutral_size];
    }
  }
  else{
    for( i = 0; i < FDCT_TOTSIZE; i++ ){
      sum += dct->transform[i];
    }
  }
  
    return fabs(sum / div);
}
 
void fdct_release( fdct_t *dct ){
  free(dct->ctable);
  free(dct->transform);
} 
