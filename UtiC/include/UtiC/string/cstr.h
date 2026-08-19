#pragma once
#include "UtiC/core/types.h"

/* Returns zero if str is NULL */
usz cstr_length(const char* str);
/* Returns FALSE if destination is smaller than source. Does not truncate. out_length may be NULL */
bool cstr_copy(char* destination, usz destination_capacity, const char* source, usz* out_length);
/* Appends source if it fits; otherwise returns FALSE without modifying destination. out_length may be NULL. */
bool cstr_append(char* destination, usz destination_capacity, const char* source, usz* out_length);
/* Returns TRUE if both strings match */
bool cstr_compare(const char* str_1, const char* str_2);