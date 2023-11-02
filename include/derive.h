#ifndef DERIVE_H
#define DERIVE_H

#include "brzo_parse.h"

int 
brzo_re_nullable(
    brzo_re_stack_t *io_re,
    int *o_nullable
);

int
brzo_re_derive(
    uint_least32_t i_c,
    brzo_re_stack_t *io_re
);

#endif /* DERIVE_H */
