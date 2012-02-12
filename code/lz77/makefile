PROG = lz77
include ../default
objects = main.o lz77_dec.o lz77_enc.o $(commonobjects)

$(PROG) : $(objects)
	cd .. && $(MAKE)
	$(CC) $(objects) -o $(PROG)

main.o : main.cpp lz77_dec.h lz77_enc.h

lz77_dec.o : token.h lz77_dec.h ../bits.h
lz77_enc.o : token.h lz77_enc.h ../bits.h ../io.h

clean :
	rm $(PROG) $(objects)