#define STG_LEXER_IMPLEMENTATION
#include "../stg_lexer.h"
#include <stdio.h>

void dump_token(stg_lexer_token token)
{
    printf("[%s](%llu,%llu) -> "STG_SV_FMT"\n", 
            stg_lexer_token_type_as_cstr(token.type), 
            token.location.row, token.location.col, 
            STG_SV_ARGV(token.literal));
}

int main(int argc, char **argv) {
    if(argc < 2) { 
        fprintf(stderr, "Please provide the file path\n");
        return -1;
    }

    const char *source_file_path = argv[1];
    stg_lexer lexer;
    if(stg_lexer_init_from_file(&lexer, source_file_path) == STG_FALSE) {
        fprintf(stderr, "Failed to open file %s\n", source_file_path);
        return -1;
    }
    printf("[Source]:\n %s\n[End of source]\n", lexer.source.data);

    stg_lexer_token token;
    while(stg_lexer_next(&lexer, &token) != STG_FALSE) {
        dump_token(token);
    }

    stg_lexer_deinit(lexer);
}
