#ifndef STRING_H
#define STRING_H

#include <stdbool.h>

char tolower(char s1);
int strlen(const char* ptr);
bool isdigit(char c);
int toenumericdigit(char c);
int strnlen(const char* ptr, int max);
char* strcpy(char* dest, const char* src);
char* strncpy(char* dest, const char* src, int count);
int istrncmp(const char* str1, const char* str2, int n);
int strncmp(const char* str1, const char* str2, int n);
int strnlen_terminator(const char* str, int max, char terminator);


#endif // STRING_H