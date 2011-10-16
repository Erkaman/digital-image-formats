PROG = gif
include ../default
objects = gif.o ../common.o

$(PROG) : $(objects)
	$(CC)  $(objects)  -o $(PROG)

gif.o : gif.h
../common.o : ../common.h

clean :
	rm $(PROG) $(objects)