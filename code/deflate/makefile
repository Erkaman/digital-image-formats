PROG = deflate
include ../default
objects = main.o ../common.o deflate.o

$(PROG) : $(objects)
	$(CC)  $(objects)  -o $(PROG)

main.o : main.h ../common.h deflate.h
deflate.o : deflate.h ../common.h
../common.o : ../common.h

clean :
	rm $(PROG) $(objects)