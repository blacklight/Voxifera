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

typedef enum  { capture, append } mode;

/* tabella dei fattoriali di 1/n!, dove 0 <= n < 30 */
static double ftable[]  = { 0.0, 1.0, 0.5, 0.166667, 0.0416667, 0.00833333, 0.00138889, 0.000198413, 
						    2.48016e-05, 2.75573e-06, 2.75573e-07, 2.50521e-08, 2.08768e-09, 5.17584e-10, 
						    7.81894e-10, 4.98925e-10, 4.98955e-10, -3.46594e-09, -1.11305e-09, 9.12062e-09, 
						   -4.75707e-10, -8.3674e-10, -1.91309e-09, 1.15948e-09, -1.28875e-09, 4.81654e-10, 
						   -5.39409e-10, 6.73499e-10, -7.26886e-10, -8.05468e-10 };
/* intervallo in cui posizionare x */
static double offset[]  = { 0.0, M_PI };
/* sviluppo in serie di Taylor della funzione coseno 
 * 
 * x     : valore per cui calcolare il coseno .
 * terms : numero di termini da usare nello sviluppo dove 0 <= terms < 30 e terms è un numero pari .
 */
inline double taylor_cosine( double x, int terms ) {
	
    int i        = terms,
	    quadrant = x * TWO_O_M_PI; /* 0, 1, 2 o 3 */
	double x2, r;
	
	/* tariamo x in modo tale che −π < x < π */
    x         = x - quadrant * H_M_PI;
    quadrant += 1;
    x         = offset[ (quadrant >> 1) & 1 ] - x;
    x2        = -(x*x);
	/* eseguo lo sviluppo in serie */
    r = ftable[i] * x2;
    for( i -= 2; i > 0; i -= 2 ){
    	r += ftable[i];
        r *= x2;
    }

    return r + 1;
}

void helpandusage()  {
	fprintf (stderr,
			"---=== BlackLight's & evilsocket's vocal recognition v.0.1b ===---\n"
			"author @ blacklight@autistici.org\n"
			"author @ evilsocket@gmail.com\n\n"
			"Usage: vocal [-d <device>] [-a] [-c <file>] [-h]\n\n"
			"\t-d <device>\tUse a different device instead the default one (/dev/dsp)\n"
			"\t-a\t\tAppend a new vocal sample with an associated command\n"
			"\t-c <file>\tUse a different rc file (default: ~/.vocalrc)\n"
			"\t-h\t\tPrint this help and exit\n"
			);
}

int main (int argc, char **argv)  {
	int i, j, dsp, fd, ch, size, recognized;
	
	char *line, *tmp, *cmd = NULL, *fname = NULL, *device = NULL;
	char **match = NULL;
	
	double deviance;
	double *neutral, *trans;
	double value, sum = 0.0;
	double t, v;
	
	FILE *fp;
	time_t t1, t2;
	
	u8 *buf;
	mode m = capture;

	while ((ch=getopt(argc, argv, "ahc:d:"))>0)  {
		switch(ch){
			case 'd' : device = strdup(optarg); break;
			case 'a' : m      = append;         break;
			case 'c' : fname  = strdup(optarg); break;
			case 'h' : helpandusage();
					   return 0;                break;
			default  :
					   helpandusage();
					   return 1;		
		}
	}

	buf     = (u8*) malloc(TOTSIZE);
	neutral = (double*) malloc(TOTSIZE*sizeof(double));
	trans   = (double*) malloc(TOTSIZE*sizeof(double));
	memset(buf, 0x80, TOTSIZE);

	if (!fname)  {
		fname = (char*) malloc(0x100);
		snprintf (fname, 0x100, "%s/.vocalrc", getenv("HOME"));
	}

	if ((fd=open("neutral.ft", O_RDONLY, 0644))<0)  {
		perror ("could not find file neutral.ft");
		return 1;
	}

	read (fd, neutral, TOTSIZE*sizeof(double));
	close(fd);

	if ((dsp = init_dsp( device ))<0)
		return 1;

	memset (buf, 0x0, TOTSIZE);
	printf ("now capturing, say something...\n");

	if (read(dsp, buf, TOTSIZE) != TOTSIZE)  {
		perror("read the wrong number of bytes");
		return 1;
	}

	close(dsp);
		
	printf ("capture done, computing DCT...\n" );
	t1 = time((unsigned) NULL);
	
	
	/* porto fuori il loop per i = 0 così da evitare controlli su t (che dipende da i) 
	 * e j nel secondo loop più consistente . */
	v = 0;
	for( j = 0; j < TOTSIZE; ++j ){
		v += buf[j];
	}
	v *= M_COEFF;
	v -= neutral[0];
	v = (v >= 0) ? v : -v;
	trans[0] = log(v);
	
	double init = buf[0];
	for( i = 1; i < TOTSIZE; ++i ) {
		t = M_FACTOR * i;
		v = init;
		/* dato che ho inizializzato v a buf[0] e sono sicuro che t != 0, 
		 * posso partire da j = 1 . */
		for( j = 1; j < TOTSIZE; ++j ){
			v += buf[j] * taylor_cosine( t * j, 10 );
		}
		v *= M_COEFF;
		v -= neutral[i];
		v = (v >= 0) ? v : -v;
		trans[i] = log(v);
	}

	free(neutral);
	free(buf);

	t2 = time((unsigned) NULL);
	printf ("DCT computing: done in %u seconds\n", (unsigned int) (t2-t1));

	for (i=0; i<TOTSIZE; i++)
		sum += trans[i];
	sum = exp(sum/100000.0);
	free(trans);

	printf ("vocal sequence successfully acquired, Fourier sum = %g\n", sum);

	if (m == append)  {
		printf ("which command would you like to associate for this vocal sequence? ");
		cmd = getline(stdin);

		if (!(fp=fopen(fname,"a")))  {
			perror ("Unable to open configuration file");
			return 1;
		}

		fprintf (fp, "%s: %g\n", cmd, sum);
		fclose(fp);
		free(cmd);
		printf ("sequence successfully saved\n");
	} else {
		if (!(fp=fopen(fname,"r")))  {
			perror ("Unable to open configuration file");
			return 1;
		}

		recognized=0;

		for (i=1; (line=getline(fp)) && !feof(fp); i++)  {
			if ((match=preg_match("^.+:\\s", line, &size)))  {
				tmp = strdup(match[0]);
				tmp[strlen(match[0])-2] = 0;
				
				if ((match=preg_match(":\\s.+$", line, &size)))  {
					for (j=2; j<strlen(match[0]); j++)
						match[0][j-2] = match[0][j];

					match[j-2]=0;
					value = atof(match[0]);
					
					double delta = ABS(ABS(value)-ABS(sum));

					if ( delta <= 0.025 )  {
						if (!recognized)  {
							deviance   = delta;
							cmd        = strdup(tmp);
							recognized = 1;
						} else {
							if (delta < deviance)  {
								deviance = delta;
								cmd      = strdup(tmp);
							}
						}
					}
				} else {
					fprintf (stderr, "Parse error at %s:%d\n", fname, i);
					return 1;
				}

				free(tmp);
			} else {
				fprintf (stderr, "Parse error at %s:%d\n", fname, i);
				return 1;
			}
		}

		if (recognized)  {
			printf ("value = %g, deviance = %g: vocal command recognized: %s\n",
					value, deviance, cmd);

			cmd = (char*) realloc(cmd, strlen(cmd)+3);
			sprintf (cmd, "%s &", cmd);
			system (cmd);
			free(cmd);
		} else
			printf ("I'm sorry, I'm not smart enough to understand what you said :(\n");
	}

	if (fname)
		free(fname);

	return 0;
}

