PROG = tga
include ../default
objects = tga.o ../common.o util.o

$(PROG) : $(objects)
	$(CC)  $(objects)  -o $(PROG)

tga.o : tga.h util.h ../common.h
util.o : util.h tga.h
../common.o : ../common.h

clean :
	rm $(PROG) $(objects)