PROG = gif
include ../default
objects = gif.o ../lzw/lzwutil.o $(commonobjects)

$(PROG) : $(objects)
	cd .. && $(MAKE)
	$(CC) $(objects) -o  $(PROG) -lm

gif.o : gif.cpp ../io.h ../defs.h  ../lzw/lzwutil.h ../bits.h
../lzw/lzwutil.o : ../lzw/lzwutil.h


clean :
	rm $(PROG) $(objects) $(commonobjects)