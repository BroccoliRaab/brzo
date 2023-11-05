#ifndef DERIVE_H
#define DERIVE_H

#include "brzo_parse.h"

int 
brzo_re_nullable(
    brzo_re_stack_t *io_re,
    brzo_token_id_t *o_nullable
);

int
brzo_re_derive(
    uint_least32_t i_c,
    brzo_re_stack_t *io_re
);

int
brzo_match(
    char * i_utf8_str,
    const brzo_re_stack_t * i_re,
    uint8_t * o_match
);
#endif /* DERIVE_H */
