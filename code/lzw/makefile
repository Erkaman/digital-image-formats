PROG = lzw
include ../default
objects = lzw.o util.o ../common.o

$(PROG) : $(objects)
	$(CC)  $(objects)  -o $(PROG)

lzw.o : lzw.c ../common.h
util.o : util.h
../common.o : ../common.h

clean :
	rm $(PROG) $(objects)