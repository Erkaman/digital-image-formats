PROG = gif
include ../default
objects = gif.o ../common.o util.o

$(PROG) : $(objects)
	$(CC)  $(objects) -g  -o $(PROG)

gif.o : gif.h util.h
util.o : util.h
../common.o : ../common.h

clean :
	rm $(PROG) $(objects)