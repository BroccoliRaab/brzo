#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "../include/re_stack.h"
#include "../include/derive.h"

int main(int argc, char**argv)
{
    int i, r;
    uint8_t match;
    char b[512];
    brzo_re_stack_t re;

    if (argc!=2) return 1;

    fgets(b, sizeof(b), stdin);
    for (i =0; b[i] != '\n';i++);
    b[i]=0;

    r = brzo_M_parse(b, &re);
    if (r) goto exit;

    r = brzo_match(argv[1], &re, &match);
    if (r) goto exit;

    fputs("Match: ", stdout);
    puts(
            match ? "TRUE" : "FALSE"
        );
exit:
    /*TODO: Free regex */
    return 0;
}
