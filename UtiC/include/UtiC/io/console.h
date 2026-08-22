#include <stdarg.h>
#include <UtiC/core/types.h>

void console_write(const char* message);
void console_writef(const char* format, ...);
void console_vwritef(const char* format, va_list args);
