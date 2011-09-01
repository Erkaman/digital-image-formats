PROG = tga.exe
CC = gcc
CFLAGS = -Wall -c -pedantic -ansi
objects = tga.o

$(PROG) : $(objects)
	$(CC)  $(objects)  -o $(PROG)

tga.o : tga.h

clean :
	rm $(PROG) $(objects)