#include "grapheme.h"

static char * p_str;

static int RE();
static int TERM();
static int SEQ();
static int REP();
static int CHARSET();
static int OP();
static int CHARSEQ();
static int CHARSETBODY();
static int CHAR();
static int META();

void 
brzo_parse_setstr(char * str)
{
    p_str = str;
}

uint_least32_t 
brzo_parse_cur()
{
    uint_least32_t c;
    grapheme_decode_utf8(p_str, SIZE_MAX, &c);
    return c;
}

void
brzo_parse_adv()
{
    p_str += grapheme_next_character_break_utf8(p_str, SIZE_MAX);
}

int
brzo_parse_validate(char * str)
{
    brzo_parse_setstr(str);
    return RE();
}

static int
RE()
{
    switch (brzo_parse_cur())
    {
    case '(':
        brzo_parse_adv();
        if (RE()) 
        {
            return brzo_parse_cur() == ')';
        }
        break;
    default :
        if (TERM())
        {
            switch (brzo_parse_cur())
            {
            case '|':
                brzo_parse_adv();
                return RE();
            case 0:
                return 1;
            }
        }
    }
    return 0;
}

static int
TERM()
{
    if (SEQ())
    {
        switch (brzo_parse_cur())
        {
        case 0:
        case '|':
        case ')':
            return 1;
        case '~':
        case '*':
        case '+':
        case '-':
        case '?':
            return 0;
        default:
            return TERM();
        }
    }
    return 0;
}

static int
SEQ()
{
    if (brzo_parse_cur() == '~')
    {
        brzo_parse_adv();
    }
    return REP();
}

static int
REP()
{
    if (CHARSET())
    {
        switch (brzo_parse_cur())
        {
        case 0:
        case '|':
        case ')':
            return 1;
        case '~':
            return 0;
        case '*':
        case '+':
        case '-':
        case '?':
            return OP();
        }
        return 1;
    }
    return 0;
}

static int 
OP()
{
    switch (brzo_parse_cur())
    {
    case '*':
    case '+':
    case '-':
    case '?':
        brzo_parse_adv();
        return 1;
    }
    return 0;
}

static int 
CHARSET()
{
    switch (brzo_parse_cur())
    {
    case '[':
        brzo_parse_adv();
        if (CHARSETBODY())
            if (brzo_parse_cur() == ']')
            {
                brzo_parse_adv();
                return 1;
            }
        return 0;
    default:
        return CHAR();
    }
}

static int
CHAR()
{
    switch (brzo_parse_cur())
    {
    case '\\':
        brzo_parse_adv();
        return META();
    case 0: 
    case '|':
    case ')':
    case '(':
    case '~':
    case '*':
    case '+':
    case '-':
    case '?':
    case '[':
    case ']':
    case '$':
    case '^':
        return 0;
    }
    brzo_parse_adv();
    return 1;
}

static int
META()
{
    switch (brzo_parse_cur())
    {
    case '|':
    case ')':
    case '(':
    case '~':
    case '\\':
    case '*':
    case '+':
    case '-':
    case '?':
    case '[':
    case ']':
    case '$':
    case '^':
        brzo_parse_adv();
        return 1;
    default:
        return 0;
    }
}

static int
CHARSETBODY()
{
    if (brzo_parse_cur()=='^')
    {
        brzo_parse_adv();
    }
    return CHARSEQ();
}

static int
CHARSEQ()
{
    if (CHAR())
    {
       switch (brzo_parse_cur()) 
        {
        case '|':
        case ')':
        case '(':
        case '~':
        case '\\':
        case '*':
        case '+':
        case '-':
        case '?':
        case '[':
        case '$':
        case '^':
            return 0;
        case ']':
            return 1;
        default:
            return CHARSEQ();
        }
    }
    return 0;
}
