PROG = tga
include ../default
objects = main.o $(commonobjects)

$(PROG) : $(objects)
	cd .. && $(MAKE)
	$(CC)  $(objects)   -o $(PROG)

main.o : main.cpp ../io.h ../bits.h

clean :
	rm -f $(PROG) $(objects)