CC = g++ -std=c++17
CC_1 = g++ -std=c++17  -lboost_iostreams -lboost_regex -licuuc -lboost_locale
CC_2 = g++ -std=c++17  -lboost_iostreams -lboost_regex -lboost_locale

#-licuio -licui18n -licudata

WARNINGS= #-mfpmath=sse -fstack-protector-all -g -W -Wall -Wextra -Wcast-align -Werror -pedantic -pedantic-errors -Wfloat-equal -Wpointer-arith -Wformat-security -Wmissing-format-attribute -Wformat=1 -Wwrite-strings -Wcast-align -Wno-long-long -Woverloaded-virtual -Wnon-virtual-dtor -Wcast-qual -Wno-suggest-attribute=format

FLAGS= #-fsanitize=address

test: prog clean run

prog: main.o struct.o struct_endian.o Index.o Boolean_searcher.o Tests.o
		$(CC_1) $(FLAGS) $(WARNINGS) -o prog main.o struct.o struct_endian.o Index.o Boolean_searcher.o Tests.o

main.o:
		$(CC) $(FLAGS) $(WARNINGS) -c main.cpp

struct.o:
		$(CC)  $(FLAGS) $(WARNINGS) -c ./ext_lib/struct.c

struct_endian.o:
		$(CC)  $(FLAGS) $(WARNINGS) -c ./ext_lib/struct_endian.c

Index.o:
		$(CC_2)  $(FLAGS) $(WARNINGS) -c Index.cpp

Boolean_searcher.o:
		$(CC)  $(FLAGS) $(WARNINGS) -c Boolean_searcher.cpp

Tests.o:
		$(CC)  $(FLAGS) $(WARNINGS) -c Tests.cpp

clean:
	rm -rf *.o

run:
	./prog
