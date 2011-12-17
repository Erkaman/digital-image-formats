PROG = lzw
include ../default
objects = main.o ../io.o ../bitwise.o

$(PROG) : $(objects)
	$(CC)  $(objects)  -o $(PROG)

main.o : main.h ../io.h ../defs.h ../bitwise.h
../io.o : ../io.h ../defs.h
../bitwise.o : ../bitwise.h ../defs.h

clean :
	rm $(PROG) $(objects)