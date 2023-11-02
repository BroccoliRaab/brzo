#ifndef RE_STACK_H
#define RE_STACK_H

#include <brzo_parse.h>

struct brzo_re_stack_s {
    brzo_tolken_t *bot;
    ssize_t cap, top_index;
};

int
brzo_M_re_stack_new (
        brzo_re_stack_t * o_re_stack
);

int
brzo_M_re_stack_new_cap (
    brzo_re_stack_t * o_re_stack,
    size_t cap
);

int
brzo_re_stack_push(
    const brzo_tolken_t i_val,
    brzo_re_stack_t * io_re_stack
);

int
brzo_re_stack_pop(
    brzo_re_stack_t * io_re_stack,
    brzo_tolken_t * o_val
);

int
brzo_re_stack_peek(
    const brzo_re_stack_t * i_re_stack,
    brzo_tolken_t * o_val
);

void 
brzo_F_re_stack_free(
    brzo_re_stack_t *io_re_stack    
);

int
brzo_M_re_stack_dup(
    const brzo_re_stack_t * i_src,
    brzo_re_stack_t * o_dst
);

int 
brzo_re_stack_merge(
    brzo_re_stack_t * io_bottom,
    const brzo_re_stack_t * i_top
);

#endif /* RE_STACK_H */
