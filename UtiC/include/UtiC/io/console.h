#pragma once

#include <stdarg.h>
#include <UtiC/core/types.h>

#if defined(__GNUC__) || defined(__clang__)
    #define UTIC_PRINTF_FORMAT(format_index, first_argument_index) \
        __attribute__((format(printf, format_index, first_argument_index)))
#else
    #define UTIC_PRINTF_FORMAT(format_index, first_argument_index)
#endif

void console_write(const char* message);
void console_writef(const char* format, ...) UTIC_PRINTF_FORMAT(1, 2);
void console_vwritef(const char* format, va_list args) UTIC_PRINTF_FORMAT(1, 0);

#undef UTIC_PRINTF_FORMAT
