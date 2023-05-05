#include "plumos.h"
#include "stdlib.h"
#include "stdio.h"
#include "string.h"

/*
    This is a user program that will be running from our shell
    It is build to blank.elf and from the shell we can run it by: blank.elf

    Our shell takes the blank.elf and loads it by elfloader, creates a task with a process and run it
*/
int main(int argc, char** argv)
{
    while(1) {
        print(argv[0]);
        for (int i = 0; i< 1000000; ++i ) {
            
        }
    }

    return 0;
}