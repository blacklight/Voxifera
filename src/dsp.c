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

#include "vocal.h"

int init_dsp( char *device )  {
	int dsp, arg;
	char *dev = (device == NULL ? "/dev/dsp" : device);
	
	if ((dsp=open( dev, O_RDONLY)) < 0) {
		perror("open of sound device failed");
		return -1;
	}

	arg = AFMT_U8;

	if (ioctl(dsp, SNDCTL_DSP_SETFMT, &arg)<0)  {
		perror ("SNDCTL_DSP_SETFMT failed");
		return -1;
	}

	if (arg != AFMT_U8)  {
		perror ("sound device doesn't support AFMT_U8");
		return -1;
	}

	arg = SIZE;

	if (ioctl(dsp, SOUND_PCM_WRITE_BITS, &arg) == -1)  {
		perror("SOUND_PCM_WRITE_BITS ioctl failed");
		return -1;
	}

	if (arg != SIZE)  {
		perror("unable to set sample size");
		return -1;
	}

	arg = CHANNELS;
	
	if (ioctl(dsp, SOUND_PCM_WRITE_CHANNELS, &arg) == -1)  {
		perror("SOUND_PCM_WRITE_CHANNELS ioctl failed");
		return -1;
	}

	if (arg != CHANNELS)  {
		perror("unable to set number of channels");
		return -1;
	}

	arg = RATE;
	
	if (ioctl(dsp, SOUND_PCM_WRITE_RATE, &arg) == -1)  {
		perror("SOUND_PCM_WRITE_WRITE ioctl failed");
		return -1;
	}

	return dsp;
}

