PROG = rle.exe
CFLAGS = -Wall -c -pedantic -ansi
objects = rle.o

$(PROG) : $(objects)
	$(CC)  $(objects)  -o $(PROG)

tga.o : tga.h

clean :
	rm $(PROG) $(objects)