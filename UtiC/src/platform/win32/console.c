#ifdef _WIN32
#include <stdio.h>
#include <UtiC/core/types.h>
#include <UtiC/io/console.h>
#include <UtiC/string/cstr.h>
#include <Windows.h>

static HANDLE console_handle;
static inline HANDLE get_handle(void) {
    if(!console_handle)
        console_handle = GetStdHandle(STD_OUTPUT_HANDLE);
    return console_handle;
}

void console_write(const char* message) {
    if(!message)
        return;
    const usz message_size = cstr_length(message);
    if(message_size == 0)
        return;
    LPDWORD written = { 0 };
    WriteConsole(
        get_handle(),
        message,
        message_size,
        written,
        NULL
    );
}

void console_writef(const char* format, ...) {
    if(!format)
        return;
    va_list args;
    va_start(args, format);
    console_vwritef(format, args);
    va_end(args);
}

void console_vwritef(const char* format, va_list args) {
    if(!format)
        return;
    #define BUFFER_SIZE KB(1)
    /* for now we truncate message if greater than size */
    static char buffer[BUFFER_SIZE] = { 0 };
    vsnprintf(buffer, sizeof(buffer), format, args);
    console_write(buffer);
}


#endif
