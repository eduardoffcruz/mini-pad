#include "key_mapping.h"

int map_key(char byte_in){
    if(byte_in == '\x1b'){ // <ESC> == \x1b
        char seq[3];

        if(read(STDIN_FILENO, &seq[0], 1) != 1) return '\x1b'; // no mapping: <ESC>
        if(read(STDIN_FILENO, &seq[1], 1) != 1) return '\x1b'; // no mapping: <ESC>
        
        if(seq[0] == '['){
            if(seq[1] >= '0' && seq[1] <= '9'){
                if(read(STDIN_FILENO, &seq[2], 1) != 1) return '\x1b';
                if (seq[2] == '~'){
                    switch (seq[1]) {
                        case '1': // \x1b[1~
                        case '7': // \x1b[7~
                            return HOME_KEY;
                        case '4': // \x1b[4~
                        case '8': // \x1b[8~
                            return END_KEY;
                        case '5': return PAGE_UP; // \x1b[5~
                        case '6': return PAGE_DOWN; // \x1b[6~
                        case '3': return DEL_KEY; // \x1b[3~
                    }
                }
            } else{   
                switch (seq[1]){
                    case 'A': return ARROW_UP; // \x1b[A
                    case 'B': return ARROW_DOWN; // \x1b[B
                    case 'C': return ARROW_RIGHT; // \x1b[C
                    case 'D': return ARROW_LEFT; // \x1b[D
                    case 'H': return HOME_KEY; // \x1b[H
                    case 'F': return END_KEY; // \x1b[F
                }
            }

        } else if (seq[0] == 'O') {
            switch (seq[1]) {
                case 'H': return HOME_KEY; // \x1bOH
                case 'F': return END_KEY; // \x1bOF
            }
        }
        return '\x1b'; // no mapping: <ESC>
    }

    return byte_in; // no mapping
}