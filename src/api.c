#include <grapheme.h>
#include "re_stack.h"
#include "derive.h"
#include "brzo_parse.h"

int
brzo_match(
    char * i_utf8_str,
    const brzo_re_stack_t * i_re,
    uint8_t * o_match
)
{
    uint_least32_t c;
    int r;
    brzo_re_stack_t mut_re;
    brzo_token_id_t nullable;

    r = brzo_M_re_stack_dup(i_re, &mut_re);
    if (r) goto exit;

    for(;;)
    {
        i_utf8_str += grapheme_decode_utf8(i_utf8_str, SIZE_MAX, &c);
        if (c==0) break;
        
        r = brzo_re_derive(c, &mut_re);
        if (r) goto exit;

    };

    r = brzo_re_nullable(&mut_re, &nullable);
    if (r) goto exit;

    *o_match = (nullable==BRZO_EMPTY_STRING);

exit:
    brzo_F_re_stack_free(&mut_re);
    return r;
}
