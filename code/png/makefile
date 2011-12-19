PROG = png
include ../default

objects = main.o  png.o  ../io.o ../data_list.o ../data_stream.o

deflateObjects = ../deflate/deflate.o ../deflate/zlib.o ../bitwise.o

$(PROG) : $(objects)
	cd ../deflate && $(MAKE)
	$(CC)  $(objects) $(deflateObjects)  -o $(PROG) -lm

main.o : main.h png.h
png.o :  png.h ../io.h ../data_list.h png_defs.h
../data_list.o : ../defs.h
../data_stream.o : ../defs.h ../data_list.h

../io.o : ../io.h ../defs.h

clean :
	rm $(PROG) $(objects)