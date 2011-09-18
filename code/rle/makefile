PROG = rle.exe
objects = rle.o ../common.o
include ../flags

$(PROG) : $(objects)
	$(CC)  $(objects)  -o $(PROG)

tga.o : tga.h
../common.o : ../common.h

clean :
	rm $(PROG) $(objects)