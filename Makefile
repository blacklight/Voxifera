CC = gcc
SRCDIR = src/
INCDIR = include/
CFLAGS = -I$(INCDIR) -O3 -funroll-loops -w -ffast-math -fno-stack-protector -ffunction-sections -funsafe-math-optimizations -fno-trapping-math
LFLAGS = -lm

all:
	$(CC) -c $(CFLAGS) $(SRCDIR)utils.c
	$(CC) -c $(CFLAGS) $(SRCDIR)dsp.c
	$(CC) -c $(CFLAGS) $(SRCDIR)fdct.c
	
	$(CC) $(CFLAGS) $(LFLAGS) $(SRCDIR)main.c *.o -o voxifera 

clean:
	rm -f voxifera *.o

install:
	mkdir -p /usr/local/bin
	cp voxifera /usr/local/bin
