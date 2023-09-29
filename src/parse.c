#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "grapheme.h"

enum brzo_token_id_e
{
    BRZO_NULL_TOLKEN  = 0,
    BRZO_CHARSET,
    BRZO_ALTERNATION,
    BRZO_CONCAT,
    BRZO_KLEEN ,
    BRZO_QUESTION,
    BRZO_PLUS,
    BRZO_LPAREN,
    BRZO_RPAREN       
};

typedef enum brzo_token_id_e brzo_token_id_t;
typedef struct brzo_charset_s brzo_charset_t;
typedef struct brzo_tolken_s brzo_tolken_t;

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

static const char brzo_charset_d[] = "1234567890";
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
    char * new_alloc;

    prepend_sz = strlen(*io_dest);
    append_sz = strlen(i_append);

    new_alloc = malloc(prepend_sz + append_sz + 1);
    if(!new_alloc)
        return 1;

    strcpy(new_alloc, *io_dest);
    strcat(new_alloc, i_append);

    free(*io_dest);
    *io_dest = new_alloc;

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
    size_t i = 0, uchar_len;
    char enc[5];
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

        for (; i_re_d[i] != ']' && i < i_re_d_len ; i++)
        {
            switch (i_re_d[i])
            {
            case 0:
                return 1;

            case '\\':
                i++;
                switch (i_re_d[i])
                {
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
        }

    default:
        uchar_len = grapheme_encode_utf8(i_re_d[i], NULL, 0) + 1;
        io_charset->set = malloc (
                uchar_len
                );
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
                goto exit;
            }

            (*o_re_t)[dest_i].id = BRZO_CHARSET;
            re_i+= o;
            break;
        }
    }

exit:
    if (r)
    {
        free(*o_re_t);
        *o_re_t = NULL;
    }
    return r;
}
