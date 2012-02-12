PROG = lzss
include ../default
objects = main.o lzss_util.o $(commonobjects)

$(PROG) : $(objects)
	cd .. && $(MAKE)
	$(CC) $(objects) -o $(PROG)

main.o : main.cpp lzss_util.h ../stlutil.h

lszz_util.o : ../defs.h lzss_util.h

clean :
	rm $(PROG) $(objects)