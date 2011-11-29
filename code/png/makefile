PROG = png
include ../default
objects = main.o ../common.o png.o data_list.o fixed_data_list.o print_funcs.o

$(PROG) : $(objects)
	$(CC)  $(objects)  -o $(PROG)

main.o : main.h png.h
png.o : print_funcs.h png.h ../common.h data_list.h fixed_data_list.h
../common.o : ../common.h
data_list.o : data_list.h
fixed_data_list.o : fixed_data_list.h
print_funcs.o : print_funcs.h

clean :
	rm $(PROG) $(objects)