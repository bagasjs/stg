#ifndef STG_INCLUDED
#define STG_INCLUDED

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

/**
 * Common native types 
 */
typedef unsigned long long stg_size_t;
typedef unsigned char stg_bool_t;
#define STG_FALSE 0
#define STG_TRUE  1
#ifndef STG_TOBOOL
    #define STG_TOBOOL(cond) ((cond) ? STG_TRUE : STG_FALSE)
#endif // STG_TOBOOL

/**
 * Strings & String View
 */
stg_size_t stg_strlen(const char *cstr);

typedef struct stg_string_view {
    const char *data;
    stg_size_t count;
} stg_string_view;

#define STG_INVALID_SV STG_CLITERAL(stg_string_view){0}
#define STG_SV_FMT "%.*s"
#define STG_SV_ARGV(sv) (int)sv.count, sv.data
stg_string_view stg_sv_slice(stg_string_view sv, stg_size_t start, stg_size_t end);
stg_bool_t stg_sv_eq(stg_string_view a, stg_string_view b);

/**
 * Dynamic Array
 */
#define stg_da(T)               \
    struct {                    \
        T *data;                \
        stg_size_t capacity;    \
        stg_size_t count;       \
    }

#define stg_da_append(da_ptr, item)
#define stg_da_append_many(da_ptr, items, items_count)
#define stg_da_free(da_ptr)

/**
 * Logging functionality
 */
typedef enum stg_log_level {
    STG_LOG_INFO = 0,
    STG_LOG_WARNING,
    STG_LOG_ERROR,
} stg_log_level;

void stg_platform_console_write(const char *message);
void stg_platform_console_error(const char *message);
void stg_tracelog(stg_log_level level, const char *fmt, ...);

#ifdef NDEBUG
#define stg_assert(CONDITION)
#else
void __stg_report_assertion_failure(const char *file, int line, const char *reason);
#define stg_assert(CONDITION) if(CONDITION) {} else __stg_report_assertion_failure(__FILE__, __LINE__, #CONDITION)
#endif

#endif // STG_INCLUDED

#ifdef STG_IMPLEMENTATION

stg_size_t stg_strlen(const char *cstr) {
    stg_size_t i = 0;
    while(cstr[i++] != '\0');
    return i;
}

stg_string_view stg_sv_slice(stg_string_view sv, stg_size_t start, stg_size_t end)
{
    if(end < start) STG_SWAP(stg_size_t, start, end);

    if(sv.count < start) return STG_INVALID_SV;

    return STG_CLITERAL(stg_string_view) {
        .data = sv.data + start,
        .count = end - start,
    };
}


#endif // STG_IMPLEMENTATION
