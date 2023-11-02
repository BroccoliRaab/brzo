#ifndef BRZO_PARSE_H
#define BRZO_PARSE_H

#include <stdint.h>
#include <sys/types.h>

typedef struct brzo_tolken_s brzo_tolken_t;
typedef struct brzo_charset_s brzo_charset_t;
typedef struct brzo_re_stack_s brzo_re_stack_t;
typedef struct brzo_re_stack_s brzo_re_t;

enum brzo_token_id_e
{
    BRZO_NULL_TOLKEN  = 0,
    BRZO_CHARSET,
    BRZO_RPAREN,       
    BRZO_LPAREN,
    BRZO_ALTERNATION,
    BRZO_CONCAT,
    BRZO_KLEEN,
    BRZO_QUESTION,
    BRZO_PLUS,
    BRZO_EMPTY_STRING,
    BRZO_EMPTY_SET
};
typedef enum brzo_token_id_e brzo_token_id_t;

int 
brzo_M_parse(
        char * p_str,
        brzo_re_t * o_re
);

void 
brzo_F_free_regex(
        brzo_tolken_t * tk
);
struct brzo_charset_s
{
    uint8_t negate;
    char * set; /* UTF-8 NUL Terminated String */
};

struct brzo_tolken_s
{
    brzo_token_id_t id;
    brzo_charset_t charset;
};

#endif /* BRZO_PARSE_H */
