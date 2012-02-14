PROG = deflate
include ../default
objects = main.o zlib.o deflate.o inflate.o common.o $(commonobjects)

$(PROG) : $(objects)
	cd ../huffman/ && $(MAKE)
	cd ../lzss/ && $(MAKE)
	cd .. && $(MAKE)
	$(CC) $(objects) ../huffman/huffman.o ../huffman/node.o ../lzss/lzss_util.o -o $(PROG)

main.o : main.cpp zlib.h ../io.h ../stlutil.h
zlib.o : zlib.h deflate.h ../io.h inflate.h ../bits.h
inflate.o : inflate.h ../defs.h ../io.h ../bits.h ../huffman/huffman.h \
	../lzss/lzss_util.h common.h
deflate.o : deflate.h ../defs.h ../io.h ../bits.h ../huffman/huffman.h \
	../lzss/lzss_util.h common.h

common.o : common.h ../huffman/huffman.h

clean :
	rm -f $(PROG) $(objects)