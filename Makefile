CC?=clang

SRC=src/*

CFLAGS=-c -O0 -g -std=c99 -Wall -pedantic  -fpic -flto 
INCLUDE=-Iinclude

bin/libbrzo.a: bin/re_stack.o bin/parse.o bin/derive.o bin/api.o
	ar rcs bin/libbrzo.la bin/re_stack.o bin/parse.o bin/derive.o bin/api.o
	ar -M <build_lib.mri

bin/derive.o: bin src/derive.c
	$(CC) src/derive.c $(CFLAGS) $(INCLUDE) -o bin/derive.o

bin/re_stack.o: bin src/re_stack.c
	$(CC) src/re_stack.c $(CFLAGS) $(INCLUDE) -o bin/re_stack.o

bin/parse.o: bin src/parse.c libs
	$(CC) src/parse.c $(CFLAGS) $(INCLUDE) -o bin/parse.o

bin/api.o: bin src/api.c libs
	$(CC) src/api.c $(CFLAGS) $(INCLUDE) -o bin/api.o

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
