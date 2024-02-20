#ifndef STG_INCLUDED
#define STG_INCLUDED

#include <stdarg.h> // va_list, va_start, va_end, va_arg

#if defined(_WIN32) || defined(__WIN32__)
    #define STG_PLATFORM_WINDOWS
#elif defined(__ANDROID__)
    #define STG_PLATFORM_ANDROID
#elif defined(__linux__) || defined(__gnu_linux__)
    #define STG_PLATFORM_LINUX
#else
    #error "This platform is not supported for Renge Engine, Sorry"
#endif

#ifndef STG_CLITERAL
    #ifdef __cplusplus
        #define STG_CLITERAL(T) T
    #else
        #define STG_CLITERAL(T) (T)
    #endif
#endif // STG_CLITERAL

#ifndef STG_CAST
    #define STG_CAST(T, v) ((T)(v))
#endif // STG_CAST

#ifndef STG_SWAP
    #define STG_SWAP(T, a, b)   \
        do {                    \
            T tmp = a;          \
            a = b;              \
            b = tmp;            \
        } while(0)
#endif // STG_SWAP

/************************
 * Common native types 
 ************************/
#ifndef STG_NULL
    #define STG_NULL (void *)0
#endif // STG_NULL
typedef unsigned char stg_byte_t;
typedef unsigned long long stg_size_t;
typedef unsigned char stg_bool_t;
#define STG_FALSE 0
#define STG_TRUE  1
#ifndef STG_TOBOOL
    #define STG_TOBOOL(cond) ((cond) ? STG_TRUE : STG_FALSE)
#endif // STG_TOBOOL

/************************
 * Memory utilities 
 ************************/
void *stg_memcpy(void *dst, const char *src, stg_size_t size);
void *stg_memset(void *dst, const int value, stg_size_t size);
// Not implemented
void *stg_malloc(stg_size_t size);
void  stg_free(void *ptr);

/************************
 * String utilities
 ************************/
stg_size_t stg_strlen(const char *cstr);
char *stg_strncpy(char *dst, const char *src, stg_size_t length);
// Not implemented
void stg_string_format(char *dst, stg_size_t dst_capacity, const char *fmt, ...);
void stg_string_format_v(char *dst, stg_size_t dst_capacity, const char *fmt, va_list ap);

/************************
 * String View
 ************************/
typedef struct stg_string_view {
    const char *data;
    stg_size_t count;
} stg_string_view;

#define STG_INVALID_SV STG_CLITERAL(stg_string_view){0}
#define STG_SV_FMT "%.*s"
#define STG_SV_ARGV(sv) (int)sv.count, sv.data
stg_string_view stg_sv_slice(stg_string_view sv, stg_size_t start, stg_size_t end);
// Not implemented
stg_bool_t stg_sv_eq(stg_string_view a, stg_string_view b);

/************************
 * Dynamic Array
 ************************/
#define STG_DA_INIT_CAPACITY 32
#define stg_da(T)               \
    struct {                    \
        T *data;                \
        stg_size_t capacity;    \
        stg_size_t count;       \
    }

#define stg_da_append(da_ptr, item)                                             \
    do {                                                                        \
        if((da_ptr)->count >= (da_ptr)->capacity) {                             \
            size_t new_capacity = (da_ptr)->capacity * 2;                       \
            if(new_capacity == 0) new_capacity = DA_INIT_CAPACITY;              \
            void *new_data = stg_malloc(new_capacity * sizeof(*(da)->data));    \
            stg_assert(new_data && "Buy more RAM LOL!");                        \
            stg_memcpy(new_data, (da)->data,                                    \
                (da)->count * sizeof(*(da)->data));                             \
            stg_free((da)->data);                                               \
            (da)->data = new_data;                                              \
            (da)->capacity = new_capacity;                                      \
        }                                                                       \
        (da)->data[(da)->count++] = (item);                                     \
    } while(0)

#define stg_da_append_many(da_ptr, items, items_count)                          \
    do {                                                                        \
        if((da_ptr)->count + new_items_count > (da_ptr)->capacity) {            \
            if((da_ptr)->capacity == 0) (da_ptr)->capacity = DA_INIT_CAPACITY;  \
            new_capacity = (da_ptr)->capacity * 2 + new_items_count;            \
            void *new_data = stg_malloc(new_capacity * sizeof(*(da_ptr)->data); \
            stg_assert(new_data && "Buy more RAM LOL!");                        \
            stg_memcpy(new_data, (da_ptr)->data,                                \
                (da_ptr)->count * sizeof(*(da_ptr)->data));                     \
            stg_free((da_ptr)->data);                                           \
            (da_ptr)->data = new_data;                                          \
            (da_ptr)->capacity = new_capacity;                                  \
        }                                                                       \
        memcpy((da_ptr)->data + (da_ptr)->count, new_items,                     \
                new_items_count * sizeof(*(da_ptr)->data));                     \
        (da_ptr)->count += new_items_count;                                     \
    } while(0)

#define stg_da_free(da_ptr) stg_free((da_ptr)->data)

/************************
 * Logging functionality
 ************************/
typedef enum stg_log_level {
    STG_LOG_INFO = 0,
    STG_LOG_WARNING,
    STG_LOG_ERROR,
} stg_log_level;

void stg_tracelog(stg_log_level level, const char *fmt, ...);
#ifdef NDEBUG
    #define STG_TRACELOG(level, ...)
    #define stg_assert(CONDITION)
#else
    #define STG_TRACELOG(level, ...) stg_tracelog(level, __VA_ARGS__)
    void __stg_report_assertion_failure(const char *file, int line, const char *reason);
    #define stg_assert(CONDITION) if(CONDITION) {} else { __stg_report_assertion_failure(__FILE__, __LINE__, #CONDITION) }
#endif

/************************
 * Platform dependent API 
 ************************/
void stg_platform_console_write(const char *message);
void stg_platform_console_error(const char *message);
void *stg_platform_heap_alloc(stg_size_t size);
void stg_platform_heap_free(void *ptr);

#endif // STG_INCLUDED


#ifdef STG_IMPLEMENTATION

void *stg_memcpy(void *dst, const char *src, stg_size_t size)
{
    for(stg_size_t i = 0; i < size; ++i) 
        STG_CAST(stg_byte_t *, dst)[i] = STG_CAST(const stg_byte_t *, src)[i];
    return dst;
}

void *stg_memset(void *dst, const int value, stg_size_t size)
{
    for(stg_size_t i = 0; i < size; ++i) 
        STG_CAST(stg_byte_t *, dst)[i] = STG_CAST(stg_byte_t, value);
    return dst;
}

char *stg_strncpy(char *dst, const char *src, stg_size_t length)
{
    for(stg_size_t i = 0; i < length; ++i) 
        dst[i] = src[i];
    return dst;
}

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

/************************
 * Platform dependent API 
 ************************/
#ifdef STG_PLATFORM_LINUX
    #include <stdlib.h>
    #include <stdio.h>
    void stg_platform_console_write(const char *message) {
        fprintf(stdout, "%s", message);
    }

    void stg_platform_console_error(const char *message) {
        fprintf(stderr, "%s", message);
    }

    void *stg_platform_heap_alloc(stg_size_t size) {
        void *result = malloc(size);
        if(!result) return STG_NULL;
        return result;
    }

    void stg_platform_heap_free(void *ptr){
        if(ptr) free(ptr);
    }
#endif

#endif // STG_IMPLEMENTATION
