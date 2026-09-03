#include <string.h>
#include "UtiC/string/cstr.h"

usz cstr_length(const char* str) {
    if(!str)
        return 0;
    usz len = 0;
    while(str[len] != '\0')
        ++len;
    return len;
}

bool cstr_copy(char* destination, usz destination_capacity, const char* source, usz* out_length) {
    if(out_length)
        *out_length = 0;

    if(!destination || !source || destination_capacity == 0)
        return FALSE;

    const usz source_length = cstr_length(source);
    if(out_length)
        *out_length = source_length;

    /*
     * checks room for the null terminator without evaluating
     * source_length + 1 first, so the size calculation cannot overflow.
     */
    if(source_length >= destination_capacity)
        return FALSE;

    memmove(destination, source, source_length + 1);
    return TRUE;
}

bool cstr_append(char* destination, usz destination_capacity, const char* source, usz* out_length) {
    if(out_length)
        *out_length = 0;
    if(!destination || !source || destination_capacity == 0)
        return FALSE;

    usz destination_length = 0;
    while(destination_length < destination_capacity && destination[destination_length] != '\0')
        ++destination_length;

    if(destination_length == destination_capacity)
        return FALSE;
    
    const usz source_length = cstr_length(source);
    if(source_length > (usz)-1 - destination_length)
        return FALSE;
    
    const usz result_length = destination_length + source_length;
    if(out_length)
        *out_length = result_length;
    
    if(result_length >= destination_capacity)
        return FALSE;
    
    memmove(destination + destination_length, source, source_length + 1);
    return TRUE;
}

bool cstr_compare(const char* str_1, const char* str_2) {
    if(!str_1 || !str_2)
        return FALSE;
    const usz str_1_len = cstr_length(str_1);
    const usz str_2_len = cstr_length(str_2);
    if(str_1_len != str_2_len)
        return FALSE;

    for(usz i = 0; i < str_1_len; i++)
        if(str_1[i] != str_2[i])
            return FALSE;

    return TRUE;
}
