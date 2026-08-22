#ifdef __linux__

#include <UtiC/core/types.h>
#include <UtiC/io/console.h>
#include <UtiC/string/cstr.h>

#include <errno.h>
#include <stdio.h>
#include <unistd.h>

static void linux_console_write_all(const char* message, usz message_size) {
    while (message_size > 0) {
        const ssize_t written = write(STDOUT_FILENO, message, message_size);
        if (written > 0) {
            message += (usz)written;
            message_size -= (usz)written;
            continue;
        }

        if (written < 0 && errno == EINTR)
            continue;

        return;
    }
}

void console_write(const char* message) {
    if (!message)
        return;

    const usz message_size = cstr_length(message);
    if (message_size == 0)
        return;

    linux_console_write_all(message, message_size);
}

void console_writef(const char* format, ...) {
    if (!format)
        return;

    va_list args;
    va_start(args, format);
    console_vwritef(format, args);
    va_end(args);
}

void console_vwritef(const char* format, va_list args) {
    if (!format)
        return;

    #define BUFFER_SIZE KB(1)
    /* Keep formatted-output behavior consistent with the Windows implementation. */
    char buffer[BUFFER_SIZE] = { 0 };
    (void)vsnprintf(buffer, sizeof(buffer), format, args);
    console_write(buffer);
}

#endif
