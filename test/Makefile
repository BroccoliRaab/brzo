CC?=cc
CFLAGS=-g -O0 -std=c99 -Wpedantic -Wall -Werror
LD_FLAGS=-L../lib -lgrapheme
INCLUDE_FLAGS=-I../include

parse_toy: parse_toy.c
	${CC} parse_toy.c ${CFLAGS} ${INCLUDE_FLAGS} -o parse_toy ${LD_FLAGS}
