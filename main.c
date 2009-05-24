#include "vocal.h"

typedef enum  { capture, append } mode;

void helpandusage()  {
	fprintf (stderr,
			"---=== BlackLight's vocal recognition v.0.1b ===---\n"
			"author @ blacklight@autistici.org\n\n"
			"Usage: vocal [-d <device>] [-a] [-c <file>] [-h]\n\n"
			"\t-d <device>\tUse a different device instead the default one (/dev/dsp)\n"
			"\t-a\t\tAppend a new vocal sample with an associated command\n"
			"\t-c <file>\tUse a different rc file (default: ~/.vocalrc)\n"
			"\t-h\t\tPrint this help and exit\n"
			);
}

int main (int argc, char **argv)  {
	int i, j, dsp, fd, ch, size, recognized;
	int TOTSIZE = BUF_SIZE*SAMPLE_SIZE;
	
	char *line, *tmp, *cmd = NULL, *fname = NULL, *device = NULL;
	char **match = NULL;
	
	double deviance;
	double *neutral, *trans;
	double value, sum = 0.0;
	double PIconst = (D_M_PI/TOTSIZE);
	double coeff = 1.0 / TOTSIZE;
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
	printf( "DEVICE : %s\n", device );
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

	for (i=0; i<TOTSIZE; ++i)  {
		t = PIconst*i;
		v = 0.0;

		for (j=0; j<TOTSIZE; ++j)  {
			if (t && j)
				v += buf[j] * cos(t*j);
			else
				v += buf[j];
		}

		v *= coeff;
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

					if (ABS(ABS(value)-ABS(sum)) <= 0.025)  {
						if (!recognized)  {
							deviance = ABS(ABS(value)-ABS(sum));
							cmd = strdup(tmp);
							recognized=1;
						} else {
							if (ABS(ABS(value)-ABS(sum)) < deviance)  {
								deviance = ABS(ABS(value)-ABS(sum));
								cmd = strdup(tmp);
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

