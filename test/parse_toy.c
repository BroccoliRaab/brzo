#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "grapheme.h"

#include "../src/parse.c"

static char tolkenstr[9][25] = {
    "BRZO_NULL_TOLKEN",
    "BRZO_CHARSET",
    "BRZO_ALTERNATION",
    "BRZO_CONCAT",
    "BRZO_KLEEN ",
    "BRZO_QUESTION",
    "BRZO_PLUS",
    "BRZO_LPAREN",
    "BRZO_RPAREN"
};

int 
brzo_parse(char * p_str)
{
    size_t i = 0, r = 1, d_i =0;
    uint_least32_t * p = NULL;
    brzo_tolken_t * tk = NULL;

    size_t plen;

    plen = strlen(p_str);
    if (plen == 0)
    {
        return 1;
    }

    p = malloc (sizeof(uint_least32_t) * strlen(p_str) + 1);
    if (!p)
        goto error;

    memset(p, 0,  sizeof(uint_least32_t) * strlen(p_str) + 1);

    for (d_i = 0; i < plen; d_i++)
    {
        r = grapheme_decode_utf8(p_str + i, SIZE_MAX, p+d_i);
        i += r;
    }
    if ( brzo_M_tolkenize(p, d_i,  &tk) )
        goto error;

    for(i = 0; tk[i].id != BRZO_NULL_TOLKEN; i++)
    {
        printf("%s",
                tolkenstr[tk[i].id]
            );

        if (tk[i].id == BRZO_CHARSET)
        {
           printf(": %s \"%s\"\n",
                tk[i].charset.negate ? "INVERTED" : "",
                tk[i].charset.set
            );
           free(tk[i].charset.set);
        } else {
            puts("");
        }
    }

error:
    free(tk);
    return 0;
}
int main()
{
    int i;
    char b[256];
    for (;;){
        fgets(b, 256, stdin);
        for (i =0; b[i] != '\n';i++);
        b[i]=0;

        brzo_parse(b);
    }
    return 0;
}
