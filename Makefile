all:
	gcc -Wall -pedantic -o vocal main.c dsp.c utils.c -lm -O3 -funroll-loops -w -ffast-math -fno-stack-protector -ffunction-sections -funsafe-math-optimizations -fno-trapping-math

clean:
	rm vocal

install:
	mkdir -p /usr/local/bin
	cp vocal /usr/local/bin
