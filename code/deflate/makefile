PROG = deflate
include ../default
objects = main.o deflate.o zlib.o ../data_list.o ../data_stream.o ../io.o ../bitwise.o

$(PROG) : $(objects)
	$(CC)  $(objects)  -o $(PROG)

../data_list.o : ../data_list.h ../defs.h
../data_stream.o : ../data_stream.h ../defs.h ../data_list.h
../io.o : ../io.h ../defs.h

main.o : main.h zlib.h ../data_list.h ../io.h
zlib.o : zlib.h ../io.h ../defs.h ../data_list.h deflate.h
deflate.o : deflate.h ../data_list.h ../defs.h ../data_stream.h ../io.h
../bitwise.o : ../bitwise.h ../defs.h



clean :
	rm $(PROG) $(objects)