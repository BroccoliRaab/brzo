CC?=cc
CFLAGS= -g -O0 -std=c99 -Wpedantic -Wall -Werror
LD_FLAGS=-L../bin -lbrzo
INCLUDE_FLAGS=-I../include

all: match parse_toy

match: libs match.c
	${CC} match.c ${CFLAGS} ${INCLUDE_FLAGS} -o match ${LD_FLAGS}

parse_toy: libs parse_toy.c
	${CC} parse_toy.c ${CFLAGS} ${INCLUDE_FLAGS} -o parse_toy ${LD_FLAGS}

clean:
	rm parse_toy match

libs:
	cd .. && make libs

.PHONY: clean libs all
