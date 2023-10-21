CC?=clang

SRC=src/*

CFLAGS=-c -O0 -g -std=c99 -Wall -pedantic  -fpic -flto 
INCLUDE=-Iinclude

bin/libbrzo.a: bin/re_stack.o bin/parse.o
	ar rcs $@ bin/re_stack.o bin/parse.o

bin/re_stack.o: bin src/re_stack.c
	$(CC) src/re_stack.c $(CFLAGS) $(INCLUDE) -o bin/re_stack.o

bin/parse.o: bin src/parse.c libs
	$(CC) src/parse.c $(CFLAGS) $(INCLUDE) -o bin/parse.o

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
