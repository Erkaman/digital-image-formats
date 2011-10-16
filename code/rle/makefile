PROG = rle
include ../default
objects = rle.o ../common.o

$(PROG) : $(objects)
	$(CC)  $(objects)  -o $(PROG)

tga.o : tga.h
../common.o : ../common.h

clean :
	rm $(PROG) $(objects)