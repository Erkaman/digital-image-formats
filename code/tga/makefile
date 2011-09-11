PROG = tga.exe
CC = gcc
#WARNINGS = -Wall -Wextra -Werror -Wdeclaration-after-statement  -Wshadow -Wstrict-overflow=5 -Wpointer-arith -Wcast-qual -Wstrict-prototypes -Wmissing-prototypes -Wmissing-declarations  -Winit-self -Wcast-align -Wformat=2 -Wmissing-include-dirs -Wunused-parameter

CFLAGS = -g $(WARNINGS) -c -pedantic -ansi
objects = tga.o ../common.o util.o

$(PROG) : $(objects)
	$(CC)  $(objects)  -o $(PROG)

tga.o : tga.h util.h ../common.h
util.o : util.h tga.h
../common.o : ../common.h

clean :
	rm $(PROG) $(objects)