PROG = lz77
include ../default
objects = main.o $(commonobjects)

$(PROG) : $(objects)
	cd .. && $(MAKE)
	$(CC) $(objects) -o $(PROG)

main.o : main.cpp ../defs.h ../bits.h

clean :
	rm $(PROG) $(objects)