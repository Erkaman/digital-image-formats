PROG = deflate
include ../default
objects = main.o deflate.o zlib.o $(commonobjects)

$(PROG) : $(objects)
	cd .. && $(MAKE)
	$(CC)   $(objects) -o $(PROG)

main.o : main.cpp zlib.h ../io.h
zlib.o : zlib.h ../io.h ../defs.h deflate.h
deflate.o : deflate.h ../defs.h ../io.h ../bits.h

clean :
	rm $(PROG) $(objects)