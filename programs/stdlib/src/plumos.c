#include "plumos.h"

#define BACKSPACE_KEY 0x08      // ASCII number of 'Backspace' key in keyboard
#define CARRIEGE_RETURN_KEY 0x0D // ASCII number of 'Enter' key in keyboard

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
