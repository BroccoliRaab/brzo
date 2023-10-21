#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "grapheme.h"

#include "../include/brzo_parse.h"
#include "../include/re_stack.h"

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
                /* TODO: BUG. charset.negate is sometimes uninitialized */
                re.bot[i].charset.negate ? "INVERTED" : "",
                re.bot[i].charset.set
           );
        } else {
            puts("");
        }
    }

error:
    /* TODO: Free the regex. Function not yet implemented */
    return 0;
}
int main()
{
    int i;
    char b[512];
    fgets(b, sizeof(b), stdin);
    for (i =0; b[i] != '\n';i++);
    b[i]=0;

    brzo_parse(b);
    return 0;
}
