#include <stdio.h>
#include <ctype.h>

#include "terminal_manager.h"

int main(void) {
    // change terminal mode from canonical to raw
    enable_raw_mode();

    char byte_in;
    // reads byte input while not EOF and not 'q'
    while (read(STDIN_FILENO, &byte_in, 1) == 1 && byte_in != 'q'){
        if(iscntrl(byte_in)){ // check for control characters (non-printable)
            printf("%d\n", byte_in);
        } else{
            printf("%d ('%c')\n", byte_in, byte_in);
        }
    }

    return 0;
}