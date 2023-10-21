CC?=cc
CFLAGS= -g -O0 -std=c99 -Wpedantic -Wall -Werror
LD_FLAGS=-L../lib -lgrapheme -L../bin -lbrzo
INCLUDE_FLAGS=-I../include

parse_toy:libs parse_toy.c
	${CC} parse_toy.c ${CFLAGS} ${INCLUDE_FLAGS} -o parse_toy ${LD_FLAGS}

clean:
	rm parse_toy

libs:
	cd .. && make libs

.PHONY: clean libs
