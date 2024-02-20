/**
 * `stg_lexer.h` is a header only library of lexer for making programming language
 * If you're not using the standard library do this
 * ```c
 * #define STG_WITHOUT_STANDARD_LIBRARY
 *
 * // Implement this 2 function
 * char *stg_lexer_load_file_text(const char *file_path);
 * void stg_lexer_unload_file_text(char *return_value_of_stg_lexer_load_file_text);
 * ```
 *
 */
#ifdef STG_IMPLEMENTATION
#define STG_LEXER_IMPLEMENTATION
#endif

#ifndef STG_LEXER_INCLUDED
#define STG_LEXER_INCLUDED

#ifndef STG_INCLUDED
    #ifndef STG_CLITERAL
        #ifdef __cplusplus
            #define STG_CLITERAL(T) T
        #else
            #define STG_CLITERAL(T) (T)
        #endif
    #endif
    #ifndef STG_SWAP
        #define STG_SWAP(T, a, b)   \
            do {                    \
                T tmp = a;          \
                a = b;              \
                b = tmp;            \
            } while(0)
    #endif // STG_SWAP
    
    typedef unsigned long long stg_size_t;
    typedef unsigned char stg_bool_t;
    #define STG_FALSE 0
    #define STG_TRUE  1
    #define STG_TOBOOL(cond) ((cond) ? STG_TRUE : STG_FALSE)
    
    typedef struct stg_string_view {
        const char *data;
        stg_size_t count;
    } stg_string_view;
    #define STG_INVALID_SV STG_CLITERAL(stg_string_view){0}
    #define STG_SV_FMT "%.*s"
    #define STG_SV_ARGV(sv) (int)sv.count, sv.data
    stg_string_view stg_sv_slice(stg_string_view sv, stg_size_t start, stg_size_t end);
#endif // STG_INCLUDED

#ifndef STG_LEXER_TOKENS_CACHE_CAPACITY
#define STG_LEXER_TOKENS_CACHE_CAPACITY 32
#endif // STG_LEXER_TOKENS_CACHE_CAPACITY

typedef enum stg_lexer_token_type {
    STG_LEXER_INVALID_TOKEN = 0,
    STG_LEXER_TOKEN_IDENTIFIER,
    STG_LEXER_TOKEN_INTEGER,
    STG_LEXER_TOKEN_FLOAT,
    STG_LEXER_TOKEN_STRING,
    STG_LEXER_TOKEN_SYMBOL,

    STG_LEXER_TOKEN_LPAREN, // (
    STG_LEXER_TOKEN_RPAREN, // )
    STG_LEXER_TOKEN_LCURLY, // {
    STG_LEXER_TOKEN_RCURLY, // {
    STG_LEXER_TOKEN_LBRACKET, // [
    STG_LEXER_TOKEN_RBRACKET, // ]
    STG_LEXER_COUNT_TOKENS,
} stg_lexer_token_type;

typedef struct stg_lexer_token_location {
    stg_size_t row, col;
} stg_lexer_token_location;

typedef struct stg_lexer_token {
    stg_lexer_token_type type;
    stg_string_view literal;
    stg_lexer_token_location location;
} stg_lexer_token;

typedef struct stg_lexer {
    struct {
        stg_lexer_token data[STG_LEXER_TOKENS_CACHE_CAPACITY];
        stg_size_t tail, head;
        stg_bool_t carry;
    } cache;
    stg_size_t i;
    char cc;
    stg_string_view source;
    stg_lexer_token_location location;
} stg_lexer;

char *stg_lexer_load_file_text(const char *file_path);
void stg_lexer_unload_file_text(char *return_value_of_stg_lexer_load_file_text);

stg_bool_t stg_lexer_init(stg_lexer *lexer, const char *source);
stg_bool_t stg_lexer_init_from_file(stg_lexer *lexer, const char *file_path);
void stg_lexer_deinit(stg_lexer lexer);
stg_bool_t stg_lexer_peek(stg_lexer *lexer, stg_lexer_token *token, stg_size_t index);
stg_bool_t stg_lexer_next(stg_lexer *lexer, stg_lexer_token *token);

const char *stg_lexer_token_type_as_cstr(stg_lexer_token_type token_type);

#endif // STG_LEXER_INCLUDED

#ifdef STG_LEXER_IMPLEMENTATION

stg_size_t stg_lexer__strlen(const char *cstr) {
    stg_size_t i = 0;
    while(cstr[i++] != '\0');
    return i;
}

stg_bool_t stg_lexer__iswhitespace(char ch) {
    return STG_TOBOOL(ch == '\n' || ch == '\t' || ch == ' ' || ch == '\r');
}

stg_bool_t stg_lexer__isalpha(char c) {
    return STG_TOBOOL((('A' <= c && c <= 'Z') || ('a' <= c && c <= 'z')));
}

stg_bool_t stg_lexer__isdigit(char c) {
    return STG_TOBOOL(('0' <= c && c <= '9'));
}

stg_bool_t stg_lexer__isalnum(char c) {
    return STG_TOBOOL((('A' <= c && c <= 'Z') || ('a' <= c && c <= 'z') || ('0' <= c && c <= '9')));
}

stg_bool_t stg_lexer_init(stg_lexer *lexer, const char *source)
{
    if(!lexer) return STG_FALSE;
    if(!source) return STG_FALSE;

    lexer->cache.head = 0;
    lexer->cache.tail = 0;
    lexer->cache.carry = STG_FALSE;
    lexer->source.data = source;
    lexer->source.count = stg_lexer__strlen(source);
    lexer->i = 0;
    lexer->cc = lexer->source.data[lexer->i];
    lexer->location.col = 1;
    lexer->location.row = 1;

    return STG_TRUE;
}

stg_bool_t stg_lexer_init_from_file(stg_lexer *lexer, const char *file_path)
{
    char *source = stg_lexer_load_file_text(file_path);
    if(!source) return STG_FALSE;
    return stg_lexer_init(lexer, source);
}

void stg_lexer_deinit(stg_lexer lexer)
{
    stg_lexer_unload_file_text((char *)lexer.source.data);
}

stg_size_t stg_lexer__cache_count(stg_lexer *lex)
{
    if(lex->cache.carry) {
        return STG_LEXER_TOKENS_CACHE_CAPACITY + lex->cache.head - lex->cache.tail;
    } else {
        return lex->cache.head - lex->cache.tail;
    }
}

stg_bool_t stg_lexer__cache_get(stg_lexer *lex, stg_size_t i, stg_lexer_token *result)
{
    if(stg_lexer__cache_count(lex) < 1) {
        return STG_FALSE;
    }

    *result = lex->cache.data[lex->cache.tail + i % STG_LEXER_TOKENS_CACHE_CAPACITY];
    return STG_TRUE;
}

stg_bool_t stg_lexer__cache_push(stg_lexer *lex, stg_lexer_token token)
{
    lex->cache.data[lex->cache.head] = token;
    lex->cache.head = lex->cache.head + 1;
    if(lex->cache.head >= STG_LEXER_TOKENS_CACHE_CAPACITY) {
        lex->cache.head = 0;
        lex->cache.carry = STG_TRUE;
    }

    if(lex->cache.head == lex->cache.tail) {
        return STG_FALSE; // Maximum capacity exceeded
    }

    return STG_TRUE;
}

stg_bool_t stg_lexer__cache_shift(stg_lexer *lex, stg_lexer_token *token) 
{
    *token = lex->cache.data[lex->cache.tail];
    lex->cache.tail = lex->cache.tail + 1;
    if(lex->cache.tail >= STG_LEXER_TOKENS_CACHE_CAPACITY) {
        if(!lex->cache.carry) {
            return STG_FALSE; // This is when the count is 0
        }
        lex->cache.tail = 0;
        lex->cache.carry = STG_FALSE;
    }
    return STG_TRUE;
}

stg_bool_t stg_lexer__advance(stg_lexer *lex)
{
    if(lex->i >= lex->source.count) return STG_FALSE;
    lex->i += 1;
    lex->cc = lex->source.data[lex->i];
    lex->location.col += 1;
    return STG_TRUE;
}

stg_bool_t stg_lexer__cache_token(stg_lexer *lex, stg_lexer_token_type token_type, stg_string_view literal)
{
    stg_lexer_token token = {0};
    token.type = token_type;
    token.literal = literal;
    token.location.col = lex->location.col - literal.count;
    token.location.row = lex->location.row;
    return stg_lexer__cache_push(lex, token);
}

stg_bool_t stg_lexer__cache_next_token(stg_lexer *lex)
{
    if(lex->i + 1 >= lex->source.count) return STG_FALSE;
    while(stg_lexer__iswhitespace(lex->cc)) {
        if(lex->cc == '\n') {
            lex->location.row += 1;
            lex->location.col = 0;
        }
        stg_lexer__advance(lex);
    }

    switch(lex->cc) {
        case '(':
            {
                stg_lexer__cache_token(lex, STG_LEXER_TOKEN_LPAREN, stg_sv_slice(lex->source, lex->i, lex->i + 1));
                stg_lexer__advance(lex);
            } break;
        case ')':
            {
                stg_lexer__cache_token(lex, STG_LEXER_TOKEN_RPAREN, stg_sv_slice(lex->source, lex->i, lex->i + 1));
                stg_lexer__advance(lex);
            } break;
        case '[':
            {
                stg_lexer__cache_token(lex, STG_LEXER_TOKEN_LBRACKET, stg_sv_slice(lex->source, lex->i, lex->i + 1));
                stg_lexer__advance(lex);
            } break;
        case ']':
            {
                stg_lexer__cache_token(lex, STG_LEXER_TOKEN_RBRACKET, stg_sv_slice(lex->source, lex->i, lex->i + 1));
                stg_lexer__advance(lex);
            } break;
        case '{':
            {
                stg_lexer__cache_token(lex, STG_LEXER_TOKEN_LCURLY, stg_sv_slice(lex->source, lex->i, lex->i + 1));
                stg_lexer__advance(lex);
            } break;
        case '}':
            {
                stg_lexer__cache_token(lex, STG_LEXER_TOKEN_RCURLY, stg_sv_slice(lex->source, lex->i, lex->i + 1));
                stg_lexer__advance(lex);
            } break;
        case '"':
            {
                stg_lexer__advance(lex);
                stg_size_t start = lex->i;
                while(lex->cc != '"') {
                    stg_lexer__advance(lex);
                }
                stg_lexer__cache_token(lex, STG_LEXER_TOKEN_STRING, stg_sv_slice(lex->source, start, lex->i));
                stg_lexer__advance(lex);
            } break;
        default:
            {
                if(stg_lexer__isalpha(lex->cc) == STG_TRUE) {
                    stg_size_t start = lex->i;
                    while(lex->i < lex->source.count && (stg_lexer__isalnum(lex->cc) == STG_TRUE || lex->cc == '_')) {
                        stg_lexer__advance(lex);
                    }
                    stg_string_view result = stg_sv_slice(lex->source, start, lex->i);
                    stg_lexer__cache_token(lex, STG_LEXER_TOKEN_IDENTIFIER, result);
                } else if(stg_lexer__isdigit(lex->cc) == STG_TRUE) {
                    stg_size_t start = lex->i;
                    stg_bool_t is_float = STG_FALSE;
                    while(lex->i < lex->source.count && (stg_lexer__isdigit(lex->cc) == STG_TRUE || lex->cc == '.')) {
                        if(lex->cc == '.') {
                            if(is_float == STG_TRUE) {
                                return STG_FALSE;
                            }
                            is_float = STG_TRUE;
                        }
                        stg_lexer__advance(lex);
                    }
                    stg_lexer__cache_token(lex, is_float == STG_TRUE ? STG_LEXER_TOKEN_FLOAT: STG_LEXER_TOKEN_INTEGER, 
                            stg_sv_slice(lex->source, start, lex->i));
                } else {
                    stg_size_t start = lex->i;
                    while(lex->i < lex->source.count 
                            && stg_lexer__isalnum(lex->cc) != STG_TRUE 
                            && stg_lexer__iswhitespace(lex->cc) != STG_TRUE
                            && lex->cc != '"') {
                        stg_lexer__advance(lex);
                    }
                    stg_string_view result = stg_sv_slice(lex->source, start, lex->i);
                    stg_lexer__cache_token(lex, STG_LEXER_TOKEN_SYMBOL, result);
                }
            } break;
    }

    return STG_TRUE;
}

stg_bool_t stg_lexer_peek(stg_lexer *lexer, stg_lexer_token *token, stg_size_t index)
{
    if(!lexer || !token) return STG_FALSE;

    if(index >= stg_lexer__cache_count(lexer)) {
        while(index >= stg_lexer__cache_count(lexer)) {
            if(!stg_lexer__cache_next_token(lexer)) return STG_FALSE;
        }
    }

    return stg_lexer__cache_get(lexer, index, token);
}

stg_bool_t stg_lexer_next(stg_lexer *lexer, stg_lexer_token *token)
{
    if(!lexer || !token) return STG_FALSE;
    if(stg_lexer__cache_count(lexer) < 1) {
        if(!stg_lexer__cache_next_token(lexer)) return STG_FALSE;
    }
    return stg_lexer__cache_shift(lexer, token);
}

const char *stg_lexer_token_type_as_cstr(stg_lexer_token_type token_type) {
    const char *stg_lexer_token_types_as_cstr[] = {
        "STG_LEXER_INVALID_TOKEN",
        "STG_LEXER_TOKEN_IDENTIFIER",
        "STG_LEXER_TOKEN_INTEGER",
        "STG_LEXER_TOKEN_FLOAT",
        "STG_LEXER_TOKEN_STRING",
        "STG_LEXER_TOKEN_SYMBOL",
        "STG_LEXER_TOKEN_LPAREN", 
        "STG_LEXER_TOKEN_RPAREN", 
        "STG_LEXER_TOKEN_LCURLY", 
        "STG_LEXER_TOKEN_RCURLY", 
        "STG_LEXER_TOKEN_LBRACKET",
        "STG_LEXER_TOKEN_RBRACKET",
    };
    return stg_lexer_token_types_as_cstr[token_type];
}

#ifndef STG_INCLUDED
stg_string_view stg_sv_slice(stg_string_view sv, stg_size_t start, stg_size_t end)
{
    if(end < start) STG_SWAP(stg_size_t, start, end);

    if(sv.count < start) return STG_INVALID_SV;

    return STG_CLITERAL(stg_string_view) {
        .data = sv.data + start,
        .count = end - start,
    };
}
#endif // STG_INCLUDED

#ifndef STG_WITHOUT_STANDARD_LIBRARY
#include <stdio.h>
#include <stdlib.h>

char *stg_lexer_load_file_text(const char *file_path)
{
    FILE *f = fopen(file_path, "r");
    if(!f) return NULL;

    fseek(f, 0L, SEEK_END);
    size_t filesz = ftell(f);
    fseek(f, 0L, SEEK_SET);
    char *result= malloc(sizeof(char) * (filesz + 1));
    if(!result) {
        fclose(f);
        fprintf(stderr, "Failed to read file %s", file_path);
        return result;
    }

    size_t read_length = fread(result, sizeof(char), filesz, f);
    result[read_length] = '\0';
    fclose(f);
    return result;
}

void stg_lexer_unload_file_text(char *return_value_of_stg_lexer_load_file_text)
{
    if(return_value_of_stg_lexer_load_file_text)
        free(return_value_of_stg_lexer_load_file_text);
}

#endif // STG_LEXER_WITHOUT_STANDARD_LIBRARY

#endif // STG_LEXER_IMPLEMENTATION
