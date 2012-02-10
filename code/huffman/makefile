PROG = huffman
include ../default
objects = main.o huffman.o node.o $(commonobjects)

$(PROG) : $(objects)
	$(CC)  $(objects)  -o $(PROG)

main.o : main.cpp  ../frequency_table.h ../bits.h huffman.h ../stlutil.h
huffman.o : ../frequency_table.h  ../io.h ../bits.h node.h
node.o : node.h

clean :
	rm -f $(PROG) $(objects)