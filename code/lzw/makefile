PROG = lzw
CC = gcc
include ../flags
objects = lzw.o ../common.o

$(PROG) : $(objects)
	$(CC)  $(objects)  -o $(PROG)

lzw.o : lzw.c ../common.h
../common.o : ../common.h

clean :
	rm $(PROG) $(objects)