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

// Very fast DCT (discrete cosine transform) implementation based on IEEE signal proc, 1992 #8 .
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
 
#define FDCT_TOTSIZE 16384 /* !!! QUESTA È LA GRANDEZZA DEL BUFFER --DEVE-- ESSERE UNA POTENZA DI 2 !!! */
#define FDCT_POW 14     /* ln( FDCT_TOTSIZE ) */
#define FDCT_TSO2 8192 /* FDCT_TOTSIZE / 2 */
#define FDCT_TSO4 4096     /* FDCT_TOTSIZE / 4 */
#define FDCT_2OTS 0.00012207 /* 2 / FDCT_TOTSIZE */
#define FDCT_2OTS_SQ 0.011048543 /* sqrt( 2 / FDCT_TOTSIZE ) */
#define FDCT_FACTOR 0.7071067814

// fast DCT main structure 
typedef struct {
  double * transform;
  double * ctable;
}
fdct_t;

// initialize the structure with data
void fdct_init( fdct_t *dct, unsigned char *data );
// reverse data 'til size-th byte
inline void fdct_bitrev( double *data, unsigned int size );
// complete data reversing
inline void fdct_scramble( double *data );
// compute cosine coefficents
inline void fdct_butterflies( fdct_t *dct );
// compute FDCT sums on data
inline void fdct_sums(double *data);
// compute FDCT
inline void fdct_compute( fdct_t *dct );
// compute FDCT vector sum, where k is the normalizing factor and neutral an optional mask to subtract
inline double fdct_sum( fdct_t *dct, double div, unsigned char *neutral, unsigned int neutral_size  );
// release FDCT structure
void fdct_release( fdct_t *dct );
