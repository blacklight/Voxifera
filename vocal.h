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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <math.h>
#include <time.h>
#include <fcntl.h>
#include <memory.h>
#include <regex.h>
#include <getopt.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <linux/soundcard.h>

#define	LENGTH		1		/* how many seconds of speech to store */
#define	RATE			8000		/* the sampling rate */
#define	SIZE			8		/* sample size: 8 or 16 bits */
#define	CHANNELS		1		/* 1 = mono 2 = stereo */

#define	BUF_SIZE		2
#define	SAMPLE_SIZE	(int) (LENGTH*RATE*SIZE*CHANNELS/8)

#ifndef	M_PI
#	define	M_PI		3.14159265358979323846
#endif
/* M_PI * 2 */
#define D_M_PI  6.28318530717958623199592693708837032318115234375
/* M_PI / 2 */
#define H_M_PI  1.570796327
/* 2 / M_PI */
#define TWO_O_M_PI 0.636619772

typedef unsigned char u8;
typedef unsigned int u32;

int init_dsp();
char* getline (FILE *fp);
char** preg_match (char *regex, char* s, int *size);
double ABS (double x);


