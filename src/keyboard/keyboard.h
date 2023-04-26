#ifndef KEYBOARD_H
#define KEYBOARD_H


/*
    This is the Virtual Keyboard Layer
    We maintain a linked-list for all the keyboard drivers based on the keyboard type. But we will implement only one keyboard diver

            keyboard_list_head                               task_tail
                  |                                             |                  
                 \/                                             \/
             _____________       _____________            ______________
            |             |     |             |   ...    |             |
            | Keyboard    | --->| Keyboard    | -------->| Keyboard    |
            | Driver 0    |     | Driver 1    |          | Driver N    |
            |_____________|     |_____________|          |_____________|

*/

// forward declaration
struct process;

typedef int (*KEYBOARD_INIT_FUNCTION)();

struct keyboard
{
    KEYBOARD_INIT_FUNCTION init;
    char name[20]; // keyboard name
    struct keyboard* next;
};

void keyboard_init();
int keyboard_insert(struct keyboard* keyboard);
void keyboard_backspace(struct process* process);
void keyboard_push(char c);
char keyboard_pop();

#endif // KEYBOARD_H