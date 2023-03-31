#include "string.h"

int strlen(const char* ptr)
{
    int count = 0;
    while(*ptr != 0) {
        ptr++;
        count++;
    }
    return count;
}

int strnlen(const char* ptr, int max)
{
    int i = 0;
    for (i = 0; i < max; ++i) {
        if (ptr[i] == 0)
            break;
    }
    return i;
}

bool isdigit(char c)
{   
    // 48 is decimal ASCII value of 0
    // 57 is decimal ASCII value of 9
    return (c >= 48) && (c <=57);
}

// convert char of digit to int
int toenumericdigit(char c)
{
    return c - 48;  // 48 is decimal ASCII value of 0
}