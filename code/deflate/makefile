PROG = deflate
include ../default
objects = main.o deflate.o zlib.o

$(PROG) : $(objects)
	cd .. && $(MAKE)
	$(CC)  $(commonobjects) $(objects) -o $(PROG)

main.o : main.c zlib.h ../data_list.h ../io.h
zlib.o : zlib.h ../io.h ../defs.h ../data_list.h deflate.h
deflate.o : deflate.h ../data_list.h ../defs.h ../data_stream.h ../io.h

clean :
	rm $(PROG) $(objects)