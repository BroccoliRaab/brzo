#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "grapheme.h"

#include "../src/parse.c"
#include "../src/re_stack.c"

static char tolkenstr[9][25] = {
    "BRZO_NULL_TOLKEN",
    "BRZO_CHARSET",
    "BRZO_RPAREN",
    "BRZO_LPAREN",
    "BRZO_ALTERNATION",
    "BRZO_CONCAT",
    "BRZO_KLEEN ",
    "BRZO_QUESTION",
    "BRZO_PLUS"
};

int 
brzo_parse(char * p_str)
{
    size_t i = 0;
    brzo_re_t re;

    if (brzo_M_parse(p_str, &re))
        goto error;

    for(i = 0; i<=re.top_index; i++)
    {
        printf("%s",
                tolkenstr[
                    re.bot[i].id
                ]
            );

        if (re.bot[i].id == BRZO_CHARSET)
        {
           printf(": %s \"%s\"\n",
                re.bot[i].charset.negate ? "INVERTED" : "",
                re.bot[i].charset.set
            );
           free(re.bot[i].charset.set);
        } else {
            puts("");
        }
    }

error:
    brzo_F_re_stack_free(&re);
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
