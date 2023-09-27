#include <stdio.h>

extern int brzo_parse_validate(char *);

int main()
{
    int i;
    char b[256];
    for (;;){
        fgets(b, 256, stdin);
        for (i =0; b[i] != '\n';i++);
        b[i]=0;

        printf("%s  :  %s\n",
                b,
                brzo_parse_validate(b) ? "\x1b[32mPASS\x1b[0m" : "\x1b[31mFAIL\x1b[0m"
              );
    }
    return 0;
}
