all:
	gcc -Wall -ansi -pedantic -o vocal main.c dsp.c utils.c -lm -w -O3 -funroll-loops -fno-rtti -ffast-math -fno-stack-protector -ffunction-sections

clean:
	rm vocal

install:
	mkdir -p /usr/local/bin
	cp vocal /usr/local/bin
