PROG = gif
include ../default
objects = gif.o ../lzw/lzwutil.o $(commonobjects)

$(PROG) : $(objects)
	cd .. && $(MAKE)
	$(CC) $(objects) -o  $(PROG) -lm

gif.o : gif.h ../io.h ../defs.h ../bitwise.h ../lzw/lzwutil.h ../bits.h
../lzw/lzwutil.o : ../lzw/lzwutil.h


clean :
	rm $(PROG) $(objects) $(commonobjects)