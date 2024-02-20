#ifdef STG_IMPLEMENTATION
#define STG_WINDOW_IMPLEMENTATION
#endif

#ifndef STG_WINDOW_INCLUDED
#define STG_WINDOW_INCLUDED

#include "stg.h"

#ifndef STG_INCLUDED
    #ifndef STG_CLITERAL
        #ifdef __cplusplus
            #define STG_CLITERAL(T) T
        #else
            #define STG_CLITERAL(T) (T)
        #endif
    #endif // STG_CLITERAL

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
#endif

#ifndef STG_EVENT_QUEUE_CAPACITY
    #define STG_EVENT_QUEUE_CAPACITY 512
#endif // STG_EVENT_QUEUE_CAPACITY

#define STG_WINDOW_BACKEND STG_WINDOW_BACKEND_X11
#define STG_WINDOW_BACKEND_X11      0
#define STG_WINDOW_BACKEND_WIN32    1
#define STG_WINDOW_BACKEND_WAYLAND  2
#define STG_WINDOW_BACKEND_COCOA    3

typedef enum stg_event_type {
    STG_INVALID_EVENT = 0,
    STG_EVENT_WINDOW_CLOSED,
    STG_EVENT_WINDOW_MOVED,
    STG_EVENT_WINDOW_RESIZED,

    STG_EVENT_KEY_PRESSED,
    STG_EVENT_KEY_RELEASED,

    STG_EVENT_CURSOR_MOVED,
} stg_event_type;

typedef enum stg_keycode {
    STG_INVALID_KEYCODE = 0,
} stg_keycode;

typedef struct stg_event_as_key_event {
    int keycode;
} stg_event_as_key_event;

typedef struct stg_event_as_window_event {
    int x, y;
    int width, height;
} stg_event_as_window_event;

typedef struct stg_event_as_cursor_event {
    int x, y;
} stg_event_as_cursor_event;

typedef union stg_event_as {
    stg_event_as_key_event key_event;
    stg_event_as_window_event window_event;
    stg_event_as_cursor_event cursor_event;
} stg_event_as;

typedef struct stg_event {
    stg_event_type type;
    stg_event_as as;
} stg_event;

typedef struct stg_device stg_device;

typedef struct stg_window stg_window;

stg_device *stg_create_device(void);
void stg_destroy_device(stg_device *device);

stg_window *stg_create_window(stg_device *device, int width, int height, const char *title);
void stg_destroy_window(stg_window *window);

void stg_collect_events(stg_device *device);
stg_bool_t stg_push_event(stg_device *device, const stg_event event);
stg_bool_t stg_shift_event(stg_device *device, stg_event *event);

#endif // STG_WINDOW_INCLUDED

#ifdef STG_WINDOW_IMPLEMENTATION

typedef struct stg_platform_device stg_platform_device;

#if STG_WINDOW_BACKEND == STG_WINDOW_BACKEND_X11
    #include <X11/Xlib.h>
    struct stg_platform_device {
        Display *dpy;
    };

    struct stg_window {
        Window handle;
    };
#endif

typedef struct stg_device {
    stg_platform_device platform;
    struct {
        stg_event data[STG_EVENT_QUEUE_CAPACITY];
        stg_size_t head, tail;
        stg_bool_t carry;
    } event_queue;
} stg_device;

stg_bool_t stg_push_event(stg_device *device, const stg_event event)
{
    device->event_queue.data[device->event_queue.head] = event;
    device->event_queue.head = device->event_queue.head + 1;
    if(device->event_queue.head >= STG_EVENT_QUEUE_CAPACITY) {
        device->event_queue.head = 0;
        device->event_queue.carry = STG_TRUE;
    }

    if(device->event_queue.head == device->event_queue.tail) 
        return STG_FALSE; // Maximum capacity exceeded

    return STG_TRUE;
}

stg_bool_t stg_shift_event(stg_device *device, stg_event *event)
{
    *event = device->event_queue.data[device->event_queue.tail];
    device->event_queue.tail = device->event_queue.tail + 1;
    if(device->event_queue.tail >= STG_EVENT_QUEUE_CAPACITY) {
        if(!device->event_queue.carry) {
            return STG_FALSE; // This is when the count is 0
        }
        device->event_queue.tail = 0;
        device->event_queue.carry = STG_FALSE;
    }
    return STG_TRUE;
}

#endif // STG_WINDOW_IMPLEMENTATION
