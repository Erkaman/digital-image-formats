PROG = rle.exe
CFLAGS = -Wall -c -pedantic -ansi
objects = rle.o ../common.o

$(PROG) : $(objects)
	$(CC)  $(objects)  -o $(PROG)

tga.o : tga.h
../common.o : ../common.h

clean :
	rm $(PROG) $(objects)