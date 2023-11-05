#include <stddef.h>
#include <string.h>
#include <grapheme.h>
#include "re_stack.h"

static int 
grapheme_in_string(
    const char * i_ustr,
    uint_least32_t i_g
)
{
    uint_least32_t cg;

    if (!i_ustr) return 1;
    
    while (*i_ustr)
    {
        i_ustr += grapheme_decode_utf8(i_ustr, SIZE_MAX, &cg);
        if (cg == i_g) 
            return 1;
    }
    return 0;    
}

int
brzo_re_split(
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
brzo_re_nullable(
    brzo_re_stack_t *io_re,
    brzo_token_id_t *o_nullable
)
{
    brzo_tolken_t tok;
    brzo_token_id_t tmp_nullable;

    if (!o_nullable)
        return 1;

    if (brzo_re_stack_pop(io_re, &tok))
        return 1;
    
    switch(tok.id)
    {
    case BRZO_KLEEN:
    case BRZO_QUESTION:
        if(brzo_re_split(io_re, NULL))
            return 1;
    /*FALLTHROUGH*/

    case BRZO_EMPTY_STRING:
        *o_nullable = BRZO_EMPTY_STRING;
        return 0;

    case BRZO_EMPTY_SET:
    case BRZO_CHARSET:
        *o_nullable = BRZO_EMPTY_SET;
        return 0;

    case BRZO_CONCAT:
        if (brzo_re_nullable(io_re, &tmp_nullable))
            return 1;
        if (brzo_re_nullable(io_re, o_nullable))
            return 1;
        *o_nullable = 
                (*o_nullable == BRZO_EMPTY_STRING)
            && 
                (tmp_nullable == BRZO_EMPTY_STRING)
            ?
                BRZO_EMPTY_STRING 
            :
                BRZO_EMPTY_SET;
        return 0;

    case BRZO_ALTERNATION:
        if (brzo_re_nullable(io_re, &tmp_nullable))
            return 1;
        if (brzo_re_nullable(io_re, o_nullable))
            return 1;
        *o_nullable = 
                (*o_nullable == BRZO_EMPTY_STRING)
            || 
                (tmp_nullable == BRZO_EMPTY_STRING)
            ?
                BRZO_EMPTY_STRING 
            :
                BRZO_EMPTY_SET;
        return 0;

    case BRZO_PLUS:     
        if (brzo_re_nullable(io_re, o_nullable))
            return 1;
        return 0;

    default:
        return 1;

    }
}

/* TODO: Implement shortcuts. ie EMPTY_SET CONCAT X = EMPTY_SET */
int
brzo_re_derive(
    uint_least32_t i_c,
    brzo_re_stack_t *io_re
)
{
    brzo_tolken_t tok, tmp_tok;
    brzo_re_stack_t tmp_re[2];
    int r;
    brzo_token_id_t nullable;
    r  = 0;

    tmp_re[0].bot = NULL;
    tmp_re[0].cap = 0;
    tmp_re[0].top_index = -1;
    
    tmp_re[1].bot = NULL;
    tmp_re[1].cap = 0;
    tmp_re[1].top_index = -1;
    
    if (brzo_re_stack_peek(io_re, &tok))
    {
        return 1;
    }
    
    switch (tok.id)
    {
    case BRZO_EMPTY_SET:
        return 0;

    case BRZO_EMPTY_STRING:
        if (brzo_re_stack_pop(io_re, NULL))
        {
            return 1;
        }
        tmp_tok.id = BRZO_EMPTY_SET;
        r = brzo_re_stack_push(tmp_tok, io_re);
        if (r) return 1;
        break;

    case BRZO_CHARSET:
        if (brzo_re_stack_pop(io_re, NULL))
        {
            return 1;
        }

        if (grapheme_in_string(tok.charset.set, i_c) ^ tok.charset.negate)
        {
            tmp_tok.id = BRZO_EMPTY_STRING;
            r = brzo_re_stack_push(tmp_tok, io_re);
            if (r) return 1;
            return 0;
        }
        tmp_tok.id = BRZO_EMPTY_SET;
        r = brzo_re_stack_push(tmp_tok, io_re);
        if (r) return 1;
        break;

    case BRZO_CONCAT:
        r = brzo_re_stack_pop(io_re, NULL);
        if (r) goto exit;

        r = brzo_re_split(io_re, &tmp_re[0]);
        if (r) goto exit;

        /* Legal LHS dup. It is only passed to nullable and is never merged. */
        r = brzo_M_re_stack_dup(io_re, &tmp_re[1]);
        if (r) goto exit;

        r = brzo_re_derive(i_c, io_re);
        if (r) goto exit;

        r = brzo_re_stack_merge(io_re, &tmp_re[0]);
        if (r) goto exit;

        tmp_tok.id = BRZO_CONCAT;
        r = brzo_re_stack_push(tmp_tok, io_re);
        if (r) goto exit;

        r = brzo_re_nullable(&tmp_re[1], &nullable);
        if (r) goto exit;

        tmp_tok.id = nullable;
        r = brzo_re_stack_push(tmp_tok, io_re);
        if (r) goto exit;

        r = brzo_re_derive(i_c, &tmp_re[0]);
        if (r) goto exit;

        r = brzo_re_stack_merge(io_re, &tmp_re[0]);
        if (r) goto exit;

        tmp_tok.id = BRZO_CONCAT;
        r = brzo_re_stack_push(tmp_tok, io_re);
        if (r) goto exit;

        tmp_tok.id = BRZO_ALTERNATION;
        r = brzo_re_stack_push(tmp_tok, io_re);
        if (r) goto exit;

        break;

    case BRZO_PLUS:
        r = brzo_re_stack_pop(io_re, NULL);
        if (r) goto exit;

        /* Legal LHS dup. Is always split from and is never merged */
        r = brzo_M_re_stack_dup(io_re, &tmp_re[0]);
        if (r) goto exit;

        r = brzo_re_derive(i_c, io_re);
        if (r) goto exit;

        r = brzo_re_split(&tmp_re[0], &tmp_re[1]);
        if (r) goto exit;

        r = brzo_re_stack_merge(io_re, &tmp_re[1]);
        if (r) goto exit;

        tmp_tok.id = BRZO_KLEEN;
        r = brzo_re_stack_push(tmp_tok, io_re);
        if (r) goto exit;
       
        tmp_tok.id = BRZO_CONCAT;
        r = brzo_re_stack_push(tmp_tok, io_re);
        if (r) goto exit;
       
        r = brzo_re_stack_merge(io_re, &tmp_re[1]);
        if (r) goto exit;

        r = brzo_re_nullable(io_re, &nullable);
        if (r) goto exit;

        tmp_tok.id = nullable;
        r = brzo_re_stack_push(tmp_tok, io_re);
        if (r) goto exit;
       
        r = brzo_re_stack_merge(io_re, &tmp_re[1]);
        if (r) goto exit;

        r = brzo_re_derive(i_c, io_re);
        if (r) goto exit;

        r = brzo_re_stack_merge(io_re, &tmp_re[1]);
        if (r) goto exit;

        tmp_tok.id = BRZO_KLEEN;
        r = brzo_re_stack_push(tmp_tok, io_re);
        if (r) goto exit;
       
        tmp_tok.id = BRZO_CONCAT;
        r = brzo_re_stack_push(tmp_tok, io_re);
        if (r) goto exit;

        tmp_tok.id = BRZO_CONCAT;
        r = brzo_re_stack_push(tmp_tok, io_re);
        if (r) goto exit;

        tmp_tok.id = BRZO_ALTERNATION;
        r = brzo_re_stack_push(tmp_tok, io_re);
        if (r) goto exit;
        break;

    case BRZO_QUESTION:
        if (brzo_re_stack_pop(io_re, NULL))
        {
            return 1;
        }
        r = brzo_re_derive(i_c, io_re);
        if (r) return 1;
        break;

    case BRZO_KLEEN:
        r = brzo_M_re_stack_dup(io_re, &tmp_re[0]);
        if (r) goto exit;

        r = brzo_re_stack_pop(io_re, NULL);
        if (r) goto exit;

        r = brzo_re_derive(i_c, io_re);
        if (r) goto exit;

        r = brzo_re_stack_merge(io_re, &tmp_re[0]);
        if (r) goto exit;

        tmp_tok.id = BRZO_CONCAT;
        r = brzo_re_stack_push(tmp_tok, io_re);
        if (r) goto exit;
        break;

    case BRZO_ALTERNATION:
        r = brzo_re_stack_pop(io_re, NULL);
        if (r) goto exit;

        r = brzo_re_split(io_re, &tmp_re[0]);
        if (r) goto exit;

        r = brzo_re_derive(i_c, io_re);
        if (r) goto exit;

        r = brzo_re_derive(i_c, &tmp_re[0]);
        if (r) goto exit;

        r = brzo_re_stack_merge(io_re, &tmp_re[0]);
        if (r) goto exit;

        tmp_tok.id = BRZO_ALTERNATION;
        r = brzo_re_stack_push(tmp_tok, io_re);
        if (r) goto exit;
        break;

    default:
        return 1;
    }

exit:
    brzo_F_re_stack_free(&tmp_re[0]);
    brzo_F_re_stack_free(&tmp_re[1]);
    return r;
}


