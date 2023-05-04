#include "plumos.h"
#include "string.h"

#define BACKSPACE_KEY 0x08      // ASCII number of 'Backspace' key in keyboard
#define CARRIEGE_RETURN_KEY 0x0D // ASCII number of 'Enter' key in keyboard

// Parse the command into command arguments and return a linked-list of it
// Return a linked-list of struct_argument: command->arg1->arg2..
struct command_argument* plumos_parse_command(const char* command, int max)
{
    struct command_argument* root_command = 0;
    char scommand[1025];
    if (max >= (int)sizeof(scommand)) {
        return 0;
    }

    // Copy the command into scommand
    strncpy(scommand, command,sizeof(scommand));
    
    // Split the scommand by spaces
    char* token = strtok(scommand, " ");
    if (!token) {
        goto out;
    }

    // Allcoate struct for the first command
    root_command = plumos_malloc(sizeof(struct command_argument));
    if (!root_command) {
        goto out;
    }

    // Copy token (the first command) into root_command
    strncpy(root_command->argument, token, sizeof(root_command->argument));
    root_command->next = 0;

    // Command arguments
    struct command_argument* current = root_command;
    // Split the scommand by spaces to get the arguments.
    // Note: strtok uses global variable that saves the last string we sent to it (scommand), so sending NULL to strtok will continue splitting scommand
    token = strtok(NULL, " ");
    while(token != 0) {
        struct command_argument* new_command = plumos_malloc(sizeof(struct command_argument));
        if (!new_command) {
            break;
        }

        strncpy(new_command->argument, token, sizeof(new_command->argument));
        
        // Add to linked-list
        new_command->next = 0;
        current->next = new_command;
        current = new_command;
        token = strtok(NULL, " ");
    }

out:
    return root_command;
}

// Getkey and block (wait until a key is pressed)
int plumos_getkeyblock()
{
    int val = 0;
    do  {
        val =  plumos_getkey();
    } while(val == 0);
    return val;
}

/*
 Wait until key is pressed and fill the buffer (out)
 Leave the function when 'Enter' (Carriage Retrun) is pressed
 max total chars to store in buffer
 output_while_typing :
    True: print the pressed keys while typing and filling the buffer
    False: Fill the buffer without printing what is pressed when typing (e.g. for typing passwords in terminal)
*/
void plumos_terminal_readline(char* out, int max, bool output_while_typing)
{
    int i = 0;
    for (i = 0; i < max -1; ++i) {
        
        // Wait until key is pressed and get it
        char key = plumos_getkeyblock();

        //  'Enter' is pressed. Carriage return means we have read the line
        if (key == CARRIEGE_RETURN_KEY) {
            break;
        }

        if (output_while_typing) {
            // Output to the screen the key that was pressed
            plumos_putchar(key);
        }

        // Backspace and we have at least on character already printed
        if (key == BACKSPACE_KEY && i >=1) {
            out[i-1] = 0x00;
            // i = i-2 since when we call continue i will be i+1 (from the for loop)
            i-=2;
            continue;
        }

        // Store in the buffer
        out[i] = key;
    }

    // Add null terminator
    out[i] = 0x00;
}

// Run command from shell
int plumos_system_run(const char* command)
{
    // Parse the command and get a linked-list of the command with all the args
    char buf[1024];
    strncpy(buf, command, sizeof(buf));
    struct command_argument* root_command_argument = plumos_parse_command(buf, sizeof(buf));

    if (!root_command_argument) {
        return -1;
    }

    // call plumos_system - asm function the invokes the kernel with the commands adn arguments
    return plumos_system(root_command_argument);
}