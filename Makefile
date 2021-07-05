#
# Makefile for smsh
#  as of 2017-03-31
#

CC = cc -Wall -std=c99 -g


OBJS = smsh5.o splitline.o process.o varlib.o controlflow.o builtin.o \
		flexstr.o

smsh: $(OBJS)
	$(CC) -o smsh $(OBJS)

builtin.o: builtin.c smsh.h varlib.h builtin.h 
	$(CC) -c -Wall builtin.c

controlflow.o: controlflow.c smsh.h process.h 
	$(CC) -c -Wall controlflow.c

flexstr.o: flexstr.c flexstr.h splitline.h 
	$(CC) -c -Wall flexstr.c

process.o: process.c smsh.h builtin.h varlib.h controlflow.h process.h 
	$(CC) -c -Wall process.c

smsh5.o: smsh5.c smsh.h splitline.h varlib.h process.h 
	$(CC) -c -Wall smsh5.c

splitline.o: splitline.c splitline.h smsh.h flexstr.h 
	$(CC) -c -Wall splitline.c

varlib.o: varlib.c varlib.h 
	$(CC) -c -Wall varlib.c

clean:
	rm -f *.o 
