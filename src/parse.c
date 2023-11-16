#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "grapheme.h"

#include "re_stack.h"

int
brzo_re_build(
    const brzo_tolken_t * i_tk,
    brzo_re_stack_t * o_re
);
int 
brzo_re_validate(
    const brzo_re_stack_t * re,
    int *valid
);

static const char brzo_charset_d[] = "1234567890";

static const char brzo_charset_w[] = 
    "1234567890_"
    "abcdefghijklmnopqrstuvwxyz"
    "ABCDEFGHIJKLMNOPQRSTUVWXYZ";

/* TODO: Make a better whitespace charset. Missing \t, \r, \n and probably more */
static const char brzo_charset_s[] = 
    "\x20"                  /* SPACE */                      
    "\xc2\xa0"              /* NO-BREAK SPACE */            
    "\xe1\x9a\x80"          /* OGHAM SPACE MARK */          
    "\xe2\x80\x80"          /* EN QUAD */                   
    "\xe2\x80\x81"          /* EM QUAD */                   
    "\xe2\x80\x82"          /* EN SPACE */                  
    "\xe2\x80\x83"          /* EM SPACE */                  
    "\xe2\x80\x84"          /* THREE-PER-EM SPACE */        
    "\xe2\x80\x85"          /* FOUR-PER-EM SPACE */         
    "\xe2\x80\x86"          /* SIX-PER-EM SPACE */          
    "\xe2\x80\x87"          /* FIGURE SPACE */              
    "\xe2\x80\x88"          /* PUNCTUATION SPACE */         
    "\xe2\x80\x89"          /* THIN SPACE */                
    "\xe2\x80\x8a"          /* HAIR SPACE */                
    "\xe2\x80\x8b"          /* ZERO WIDTH SPACE */          
    "\xe2\x80\xaf"          /* NARROW NO-BREAK SPACE */     
    "\xe2\x81\x9f"          /* MEDIUM MATHEMATICAL SPACE */ 
    "\xe3\x80\x80";         /* IDEOGRAPHIC SPACE */         

int
brzo_M_strcat(
        char ** io_dest,
        const char * i_append
)
{
    size_t prepend_sz;
    size_t append_sz;

    prepend_sz = strlen(*io_dest);
    append_sz = strlen(i_append);

    *io_dest = realloc(
            *io_dest,
            prepend_sz + append_sz + 1
        );
    if(!*io_dest)
    {
        return 1;
    }

    strcat(*io_dest, i_append);

    return 0;
}

size_t
brzo_M_parse_charset(
    const uint_least32_t * i_re_d,  /* charset after UTF-8 decode */
    const size_t i_re_d_len,        /* length of i_re_d */
    brzo_charset_t * io_charset,
    size_t * o_len
)
{
    size_t i = 0, uchar_len, j;
    char enc[32];
    char j_c;
    io_charset->negate = 0;
    switch (i_re_d[i])
    {
    case '[':
        io_charset->set = malloc(sizeof(uint_least32_t) * 1 + 1);
        io_charset->set[0] = 0;

        i++;
        if ( i_re_d[i] == '^' )
        {
            io_charset->negate = 1;
            i++;
        }

        for (; i_re_d[i] != ']' && i < i_re_d_len; i++)
        {
            switch (i_re_d[i])
            {
            case 0:
                return 1;

            case '-':
                if (
                    (
                     /* Check for [0-9] */
                     i_re_d[i-1] >= '0' && i_re_d[i-1] <= '9'
                     && i_re_d[i+1] >= '0' && i_re_d[i+1] <= '9'
                    ) ||
                    (
                     /* Check for [A-Z] */
                     i_re_d[i-1] >= 'A' && i_re_d[i-1] <= 'Z'
                     && i_re_d[i+1] >= 'A' && i_re_d[i+1] <= 'Z'
                    ) ||
                    (
                     /* Check for [a-z] */
                     i_re_d[i-1] >= 'a' && i_re_d[i-1] <= 'z'
                     && i_re_d[i+1] >= 'a' && i_re_d[i+1] <= 'z'
                    )
                )
                {
                    /* ranges [z-a] [y-a] [y-g] etc not legal */
                    if (i_re_d[i-1] > i_re_d[i+1])
                    {
                        return 1;
                    }
                    if (i_re_d[i-1] == i_re_d[i+1])
                    {
                        i++;
                        break;
                    }
                    j_c = i_re_d[i-1] + 1;
                    for (j = 0; j_c < i_re_d[i+1]; j++)
                    {
                        enc[j] = j_c;
                        j_c++;
                    }
                    enc[j] = 0;
                    if (brzo_M_strcat(&io_charset->set, enc))
                    {
                        return 1;        
                    }
                    break;
                } 

                /* Treat '-' as literal if cannot parse a range */
                enc[0] = '-';
                enc[1] = 0;
                if (brzo_M_strcat(&io_charset->set, enc))
                {
                    return 1;        
                }
                break;

            case '\\':
                i++;
                switch (i_re_d[i])
                {
                /* TODO: Support [^\S], [^\D], [^\W]*/
                /* TODO: Support escape sequences \r \n \t */
                case 's':
                    if (brzo_M_strcat(&io_charset->set, brzo_charset_s))
                    {
                        return 1;        
                    }
                    io_charset->negate = 0;
                    continue;

                case 'd':
                    if (brzo_M_strcat(&io_charset->set, brzo_charset_d))
                    {
                        return 1;        
                    }
                    io_charset->negate = 0;
                    continue;

                case 'w':
                    if (brzo_M_strcat(&io_charset->set, brzo_charset_w))
                    {
                        return 1;        
                    }
                    io_charset->negate = 0;
                    continue;

                case 0:
                    return 1;
                }
            /* FALLTHROUGH */

            default: 
                enc[
                    grapheme_encode_utf8(
                        i_re_d[i],
                        enc,
                        5
                        )
                    ] = 0;

                if (brzo_M_strcat(&io_charset->set, enc))
                {
                    return 1;        
                }
                io_charset->negate = 0;
            }
        }
        if (i >= i_re_d_len)
            return 1;
        break;

    case '.':
        io_charset->set = malloc (1);
        if (!io_charset->set) return 1;
        io_charset->set[0] = 0;
        io_charset->negate = 1;
        break;

    /* TODO: Support escape sequences \r \n \t */
    case '\\':
        i++;
        switch (i_re_d[i])
        {
            case '.':
            case '\\':
            case '[':
            case ']':
            case '(':
            case '|':
            case '*':
            case '+':
            case '?':
                break;
            
            case 0:
                return 1;

            case 'S':
                io_charset->negate = 1;
                /* FALLTHROUGH */
            case 's':
                io_charset->set = malloc(sizeof(brzo_charset_s));
                if (!io_charset) return 1;
                strcpy(io_charset->set, brzo_charset_s);
                goto exit;

            case 'D':
                io_charset->negate = 1;
                /* FALLTHROUGH */
            case 'd':
                io_charset->set = malloc(sizeof(brzo_charset_d));
                if (!io_charset) return 1;
                strcpy(io_charset->set, brzo_charset_d);
                goto exit;

            case 'W':
                io_charset->negate = 1;
                /* FALLTHROUGH */
            case 'w':
                io_charset->set = malloc(sizeof(brzo_charset_w));
                if (!io_charset) return 1;
                strcpy(io_charset->set, brzo_charset_w);
                goto exit;
        }
        /* FALLTHROUGH */

    default:
        uchar_len = grapheme_encode_utf8(i_re_d[i], NULL, 0) + 1;
        io_charset->set = malloc (uchar_len);
        if (!io_charset->set) return 1;
        memset(io_charset->set, 0, uchar_len);
        grapheme_encode_utf8(i_re_d[i], io_charset->set, SIZE_MAX);
        io_charset->negate = 0;
    }

exit:
    *o_len = i;
    return 0;
}

int 
brzo_M_tolkenize(
    const uint_least32_t * i_re_d, /* regex after UTF-8 decode */
    const size_t i_re_d_len,        /* length of i_re_d */
    brzo_tolken_t ** o_re_t         /* reference to destination array */
)      
{
    int r = 0;
    size_t re_i = 0, o = 0, dest_i;

    *o_re_t = (brzo_tolken_t *) malloc(
            i_re_d_len * 2 * sizeof(brzo_tolken_t)
        );

    if (!*o_re_t)
    {
        r = 1;
        goto exit;
    }

    (*o_re_t)->charset.set = NULL;

    for (re_i = 0, dest_i = 0; re_i < i_re_d_len; re_i++, dest_i++)
    {
        switch (i_re_d[re_i])
        {
        case '|':
            (*o_re_t)[dest_i].id = BRZO_ALTERNATION;
            break;

        case '*':
            (*o_re_t)[dest_i].id = BRZO_KLEEN;
            break;

        case '+':
            (*o_re_t)[dest_i].id = BRZO_PLUS;
            break;

        case '?':
            (*o_re_t)[dest_i].id = BRZO_QUESTION;
            break;
            
        case ')':
            (*o_re_t)[dest_i].id = BRZO_RPAREN;
            break;
            
        case '(':
        default:
            if (dest_i>0)
            {
                switch ((*o_re_t)[dest_i-1].id)
                {
                case BRZO_LPAREN:
                case BRZO_ALTERNATION:
                case BRZO_CONCAT:
                    break;
                default:
                    (*o_re_t)[dest_i].id = BRZO_CONCAT;
                    dest_i++;
                }
            }
            /* LPAREN is here so that it gets prepended by CONCAT */
            if (i_re_d[re_i] == '(')
            {
                (*o_re_t)[dest_i].id = BRZO_LPAREN;
                break;
            }

            /* Parse out a character set */
            r = brzo_M_parse_charset(
                    i_re_d + re_i,
                    i_re_d_len - re_i,
                    &(*o_re_t)[dest_i].charset,
                    &o
                    );
            if (r) {
                free((*o_re_t)[dest_i].charset.set);
                (*o_re_t)[dest_i].charset.set = NULL;
                goto exit;
            }

            (*o_re_t)[dest_i].id = BRZO_CHARSET;
            re_i+= o;
            break;
        }
    }
    (*o_re_t)[dest_i].id = BRZO_NULL_TOLKEN;

exit:
    if (r)
    {
        free(*o_re_t);
        *o_re_t = NULL;
    }
    return r;
}
/*TODO: Rename. Expose as api?*/ 
/*TODO: Refactor. Inconsistent Style */
int 
brzo_M_parse(
        char * i_re_str,
        brzo_re_stack_t * o_re
)
{
    size_t i = 0, gl = 1, d_i =0;
    uint_least32_t * p = NULL;
    brzo_tolken_t * tk = NULL;
    size_t plen;
    int valid;

    memset(o_re, 0, sizeof(brzo_re_stack_t));

    plen = strlen(i_re_str);
    if (plen == 0)
    {
        return 1;
    }

    p = malloc (sizeof(uint_least32_t) * strlen(i_re_str) + 1);
    if (!p)
        goto error;

    memset(p, 0,  sizeof(uint_least32_t) * strlen(i_re_str) + 1);
    for (d_i = 0; i < plen; d_i++)
    {
        gl = grapheme_decode_utf8(i_re_str + i, SIZE_MAX, p+d_i);
        i += gl;
    }

    if ( brzo_M_tolkenize(p, d_i,  &tk) )
        goto error;

    if (brzo_re_build(tk, o_re))
        goto error;

    if (brzo_re_validate(o_re, &valid))
        goto error;

    if (!valid) goto error;

    free(p);
    free(tk);

    return 0;

error:
    free(p);
    brzo_F_re_stack_free(o_re);

    if (!tk) return 1;

    /* Free all tokens' charsets */
    for(i = 0; tk[i].id != BRZO_NULL_TOLKEN; i++)
        if (tk[i].id == BRZO_CHARSET) 
            free(tk[i].charset.set);
    
    free(tk);
    return 1;
}

int
brzo_re_build(
    const brzo_tolken_t * i_tk,
    brzo_re_stack_t * o_re
)
{
    brzo_tolken_t *cur_tk = (brzo_tolken_t*) i_tk;
    brzo_re_stack_t opstack;
    brzo_tolken_t tmp[2];
    int r;

    r = brzo_M_re_stack_new(o_re);
    if (r) 
    {
        goto exit;
    }

    r = brzo_M_re_stack_new(&opstack);
    if (r) 
    {
        goto exit;
    }

    for (; cur_tk->id != BRZO_NULL_TOLKEN ;cur_tk++)
    {
        switch(cur_tk->id)
        {
        case BRZO_CHARSET:
            r = brzo_re_stack_push(*cur_tk, o_re);
            if (r) 
            {
                goto exit;
            }
            break;
        case BRZO_LPAREN:
            r = brzo_re_stack_push(*cur_tk, &opstack);
            if (r) 
            {
                goto exit;
            }
            break;
        case BRZO_RPAREN:
            while (
                    !brzo_re_stack_peek(&opstack, tmp)
                    && tmp[0].id != BRZO_LPAREN
            )
            {
                r = brzo_re_stack_pop(&opstack, tmp+1);
                if (r) 
                {
                    goto exit;
                }
                r = brzo_re_stack_push(tmp[1], o_re);
                if (r) 
                {
                    goto exit;
                }
            }
            r = brzo_re_stack_pop(&opstack, tmp);
            if(r) 
            {
                goto exit;
            }
            break;
        default:
            while(
                    !brzo_re_stack_peek(&opstack, tmp)
                    && tmp[0].id > cur_tk->id
            )
            {
                r = brzo_re_stack_pop(&opstack, tmp+1);
                if (r) 
                {
                    goto exit;
                }
                r = brzo_re_stack_push(tmp[1], o_re);
                if (r) 
                {
                    goto exit;
                }
            }
            r = brzo_re_stack_push(*cur_tk, &opstack);
            if (r) 
            {
                goto exit;
            }
            break;
        }
    }
    while(!brzo_re_stack_peek(&opstack, NULL))
    {
        r = brzo_re_stack_pop(&opstack, tmp);
        if(r) 
        {
            goto exit;
        }
        r = brzo_re_stack_push(tmp[0], o_re);
        if(r) 
        {
            goto exit;
        }
    }
exit:
    if (r)
    {
        brzo_F_re_stack_free(o_re);
    }
    brzo_F_re_stack_free(&opstack);
    return r;
}

int
brzo_re_validate_rec(
    brzo_re_stack_t * re,
    int *valid
)
{
    int r = 0;
    brzo_tolken_t ct;
    r = brzo_re_stack_pop(re, &ct);
    if (r) goto exit;
    switch(ct.id)
    {
    case BRZO_CONCAT:
    case BRZO_ALTERNATION:
        r = brzo_re_validate_rec(re, valid);
        if (r || !*valid) goto exit;
    /*FALLTHROUGH*/
    case BRZO_PLUS:
    case BRZO_QUESTION:
    case BRZO_KLEEN:
        r = brzo_re_validate_rec(re, valid);
        break;

    case BRZO_CHARSET:
    case BRZO_EMPTY_SET:
    case BRZO_EMPTY_STRING:
        *valid = 1;
        break;

    default:
        *valid = 0;
        break;
    }

exit:
    return r;
}

int 
brzo_re_validate(
    const brzo_re_stack_t * re,
    int *valid
)
{
    brzo_re_stack_t re_mut;
    int r = 0;
    memset(&re_mut, 0, sizeof(re_mut));

    r = brzo_M_re_stack_dup(re, &re_mut);
    if (r) goto exit;

    r =brzo_re_validate_rec(&re_mut, valid); 
    if (r || !*valid) goto exit;

    /* 
     * If The stack is not empty here then it had excess tokens 
     * Excess tokens means it is invalid
    */
    *valid = brzo_re_stack_peek(&re_mut, NULL);

exit:
    brzo_F_re_stack_free(&re_mut);
    return r;
}
