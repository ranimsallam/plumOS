#ifndef PLUMOS_STRING_H
#define PLUMOS_STRING_H

#include <stdbool.h>

char tolower(char s1);
int strlen(const char* ptr);
int strnlen(const char* ptr, int max);

bool isdigit(char c);

// convert char of digit to int
int toenumericdigit(char c);

char* strcpy(char* dest, const char* src);
char* strncpy(char* dest, const char* src, int count);

// insensetive - doesn't care about capital/small letters
// calls to this funtion is done with n = sizeif(*str2)
int istrncmp(const char* str1, const char* str2, int n);
int strncmp(const char* str1, const char* str2, int n);
int strnlen_terminator(const char* str, int max, char terminator);

char* strtok(char* str, const char* delimiters);

#endif // PLUMOS_STRING_H