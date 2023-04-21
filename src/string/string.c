#include "string.h"

char tolower(char s1) {
    // 65 - 90 in ASCII are the CAPTIAL alphabet characters
    // 97 - 122 in ASCII are the SMALL alphabet characters
    if (s1 >= 65 && s1 <= 90) {
        s1 += 32;
    }
    return s1;
}
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

char* strcpy(char* dest, const char* src)
{
    char* res = dest;
    while (*src != 0) {
        *dest = *src;
        src += 1;
        dest += 1;
    }

    // add a null terminator (\0)
    *dest = 0x00;
    return res;
}

char* strncpy(char* dest, const char* src, int count)
{
    int  i = 0;
    // Last char is '\0'
    for (i = 0; i < count-1; ++i) {
        if (src[i] == 0x00)
            break;
        dest[i] = src[i];
    }

    // Add '\0'
    dest[i] = 0x00;
    return dest;
}

// insensetive - doesn't care about capital/small letters
// calls to this funtion is done with n = sizeif(*str2)
int istrncmp(const char* str1, const char* str2, int n)
{
    unsigned char u1, u2;
    while (n-- > 0) {
        u1 = (unsigned char)*str1++;
        u2 = (unsigned char)*str2++;
        if (u1 != u2 && tolower(u1) != tolower(u2))
            return u1 - u2;
        if (u1 == '\0')
            return 0;
    }
    return 0;
}

int strncmp(const char* str1, const char* str2, int n)
{
    unsigned char u1, u2;
    while (n-- > 0) {
        u1 = (unsigned char)*str1++;
        u2 = (unsigned char)*str2++;
        if (u1 != u2) 
            return u1 - u2;
        if (u1 == '\0')
            return 0;
    }
    return 0;
}

int strnlen_terminator(const char* str, int max, char terminator)
{
    int i = 0;
    for (i = 0; i < max; ++i) {
        if (str[i] == '\0' || str[i] == terminator)
        break;
    }
    return i;
}