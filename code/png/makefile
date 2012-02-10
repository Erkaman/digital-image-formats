PROG = png
include ../default

objects = main.o  png.o $(commonobjects)

deflateObjects = ../deflate/deflate.o ../deflate/zlib.o ../huffman/huffman.o ../huffman/node.o

$(PROG) : $(objects)
	cd .. && $(MAKE)
	cd ../deflate && $(MAKE)
	$(CC)  $(objects) $(deflateObjects)  -o $(PROG)

main.o : main.h png.h
png.o :  png.h ../io.h  png_defs.h ../bits.h

clean :
	rm -f $(PROG) $(objects) $(deflateObjects)