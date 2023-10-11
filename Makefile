CC?=clang

SRC=src/*

LD_FLAGS=-lgrapheme -Llibs
CFLAGS=-O0 -g -std=c89 -Wall -pedantic 
INCLUDE=-Iinclude


bin/parse.o: bin $(SRC) libs
	$(CC) $(SRC) $(CFLAGS) -o bin/$(NAME) $(LD_FLAGS) $(INCLUDE) 

bin: 
	mkdir -p bin

libs: lib/libgrapheme.a include/grapheme.h

lib/libgrapheme.a:
	cd ext/libgrapheme-2.0.2 && make clean
	cd ext/libgrapheme-2.0.2 && make
	cp ext/libgrapheme-2.0.2/libgrapheme.a lib

include/grapheme.h:
	cp ext/libgrapheme-2.0.2/grapheme.h include
	
compile_flags.txt:
	echo "$(LD_FLAGS) $(INCLUDE) $(CFLAGS)" | tr " " "\n" > compile_flags.txt

clean: 
	rm -r bin/
	cd ext/libgrapheme-2.0.2 && make clean
	cd test && make clean

clean_libs:
	rm lib/* include/grapheme.h

.PHONY: clean libs
