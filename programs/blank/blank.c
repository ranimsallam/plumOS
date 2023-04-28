#include "plumos.h"
#include "stdlib.h"
#include "stdio.h"
#include "string.h"

/*
    This is a user program that will be running from our shell
    It is build to blank.elf and from the shell we can run it by: blank.elf
    
    Our shell taked the blank.elf and loads it by elfloader, creates a task with a process and run it
*/
int main(int argc, char** argv)
{
    //printf("Hey, This is printf from blank.c -  Im %i\n", 32);
    char* ptr = malloc(20);
    strcpy(ptr, "hello world\n");
    print(ptr);
    free(ptr);

    ptr[0] = 'B';
    print("abc\n");

    while(1) { }
    return 0;
}