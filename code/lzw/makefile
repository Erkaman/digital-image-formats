PROG = lzw
include ../default
objects = main.o lzwutil.o $(commonobjects)

$(PROG) : $(objects)
	cd .. && $(MAKE)
	$(CC) $(objects)  -o $(PROG)

main.o : lzwutil.h main.cpp ../stlutil.h ../io.h ../defs.h ../bits.h
lzwutil.o : lzwutil.h
clean :
	rm $(PROG) $(objects)