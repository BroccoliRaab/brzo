#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "re_stack.h"

int
brzo_M_re_stack_new (
        brzo_re_stack_t * o_re_stack
    )
{
    o_re_stack->top_index = -1;
    o_re_stack->cap = 8;
    o_re_stack->bot = malloc(sizeof(brzo_tolken_t) * o_re_stack->cap);
    if (!o_re_stack->bot)
    {
        return 1;
    }
    return 0;
}

int
brzo_M_re_stack_new_cap (
        brzo_re_stack_t * o_re_stack,
        size_t cap
    )
{
    o_re_stack->top_index = -1;
    o_re_stack->cap = cap;
    o_re_stack->bot = malloc(sizeof(brzo_tolken_t) * cap);
    if (!o_re_stack->bot)
    {
        return 1;
    }
    return 0;
}

int
brzo_re_stack_shear(
    brzo_re_stack_t *io_re,
    brzo_re_stack_t *o_rhs
)
{
    brzo_tolken_t tok;
    size_t i = io_re->top_index;
    int term_c = 1;

    for (;;)
    {
        if (brzo_re_stack_pop(io_re, &tok))
            return 1;

        switch (tok.id)
        {
        case BRZO_ALTERNATION:
        case BRZO_CONCAT:
            term_c += 1;
            break;
        case BRZO_CHARSET:
        case BRZO_EMPTY_SET:
        case BRZO_EMPTY_STRING:
            term_c--;
            break;    
        case BRZO_PLUS:
        case BRZO_KLEEN:
        case BRZO_QUESTION:
            break;
        default:
            return 1;
        }
        if (term_c<1) break;
    }

    if (o_rhs)
    {
        if (brzo_M_re_stack_new_cap(o_rhs, i - io_re->top_index + 1))
            return 1;
        memcpy(
            o_rhs->bot,
            &io_re->bot[io_re->top_index+1],
            (i - io_re->top_index) * sizeof(brzo_tolken_t)
        );
        o_rhs->top_index = i - io_re->top_index -1 ;
    }
    return 0;
}

int
brzo_re_stack_push(
    const brzo_tolken_t i_val,
    brzo_re_stack_t * io_re_stack
)
{
    if (io_re_stack->top_index + 1 >= io_re_stack->cap)
    {
        io_re_stack->cap *= 2;
        io_re_stack->bot = realloc(
                io_re_stack->bot,
                io_re_stack->cap * sizeof(brzo_tolken_t));
        if (! io_re_stack->bot ) return 0;
    }
    io_re_stack->top_index++;
    io_re_stack->bot[io_re_stack->top_index] = i_val;
    return 0;
}

int
brzo_re_stack_pop(
    brzo_re_stack_t * io_re_stack,
    brzo_tolken_t * o_val
)
{
    if (io_re_stack->top_index < 0)
    {
        return 1;
    }
    if (o_val)
    {
        *o_val = io_re_stack->bot[io_re_stack->top_index];
    }
    io_re_stack->top_index--;
    return 0;
}

int
brzo_re_stack_peek(
    const brzo_re_stack_t * i_re_stack,
    brzo_tolken_t * o_val
)
{
    if (i_re_stack-> top_index ==-1) return 1;
    if (o_val)
        *o_val = i_re_stack->bot[i_re_stack->top_index];
    return 0;
}

void 
brzo_F_re_stack_free(
    brzo_re_stack_t *io_re_stack    
)
{
    if (!io_re_stack) return;
    if (!io_re_stack->bot) return;

    free(io_re_stack->bot);
    io_re_stack->bot = NULL;
}

int
brzo_M_re_stack_dup(
    const brzo_re_stack_t * i_src,
    brzo_re_stack_t * o_dst
)
{
    brzo_re_stack_t tmp;
    
    if (!i_src) return 1;
    
    tmp.bot = malloc(sizeof(brzo_tolken_t) * (i_src->top_index+1));
    if (!tmp.bot) return 1;
    tmp.top_index = i_src->top_index;
    tmp.cap = i_src->top_index+1;
    memcpy(tmp.bot, i_src->bot, tmp.cap * sizeof(brzo_tolken_t));

    *o_dst = tmp;

    return 0;
}

int 
brzo_re_stack_merge(
    brzo_re_stack_t * io_bottom,
    const brzo_re_stack_t * i_top
)
{
    if (io_bottom->cap < (io_bottom->top_index + i_top->top_index+2))
    {
        io_bottom->cap = io_bottom->top_index + i_top->top_index+2;
        io_bottom->bot = realloc(
            io_bottom->bot,
            sizeof(brzo_tolken_t) * io_bottom->cap
            );
        if (!io_bottom->bot) return 1;
    }

    memcpy(
        &io_bottom->bot[io_bottom->top_index+1],
        i_top->bot,
        (i_top->top_index+1) * sizeof(brzo_tolken_t)
    );
    io_bottom->top_index = io_bottom->top_index + i_top->top_index + 1;
    return 0;
}
