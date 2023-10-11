#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
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
    *o_val = io_re_stack->bot[io_re_stack->top_index];
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
    int i;
    if (!io_re_stack) return;
    for (i = 0; i <= io_re_stack->top_index; i++)
    {
        free(io_re_stack->bot[i].charset.set);
    }

    free(io_re_stack->bot);
    io_re_stack->bot = NULL;
}
