#include "key_mapping.h"

/*** keypress mapping***/

int map_keypress(char byte_in, unsigned int* mouse_x, unsigned int* mouse_y){
    // handle special escape characters
    if(byte_in == ESC){ // <ESC> == \x1b
        char seq[5];

        if(read(STDIN_FILENO, &seq[0], 1) != 1) return ESC; // no mapping: <ESC>
        if(read(STDIN_FILENO, &seq[1], 1) != 1) return ESC; // no mapping: <ESC>
        
        if(seq[0] == '['){
            if(seq[1] >= '0' && seq[1] <= '9'){
                if(read(STDIN_FILENO, &seq[2], 1) != 1) return ESC; // no mapping: <ESC>
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
                } else if (seq[2] == ';') {
                    if(read(STDIN_FILENO, &seq[3], 1) != 1) return ESC; // no mapping: <ESC>
                    if(read(STDIN_FILENO, &seq[4], 1) != 1) return ESC; // no mapping: <ESC>
                    if (seq[1] == '1' && seq[3] == '5'){
                        switch (seq[4]) {
                            case 'C': return NEXT; // \x1b[1;5C
                            case 'D': return PREV; // \x1b[1;5D
                            case 'B': return CS_TOGGLE; // \x1b[1;5D
                        }
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
                    case 'M': // Mouse events
                    {
                        if(read(STDIN_FILENO, &seq[2], 1) != 1) return ESC;
                        char event_type = seq[2];
                        if(read(STDIN_FILENO, &seq[3], 1) != 1) return ESC;
                        *mouse_x = (unsigned char)seq[3] - 33;
                        if(read(STDIN_FILENO, &seq[4], 1) != 1) return ESC;
                        *mouse_y = (unsigned char)seq[4] - 33;
 
                        switch (event_type){
                            case 0x60: return SCROLL_UP; // \xb1[M
                            case 0x61: return SCROLL_DOWN;
                            case 0x20: return RIGHT_CLICK;
                            case 0x21: return MIDDLE_CLICK;
                            case 0x22: return LEFT_CLICK;
                            case 0x23: return CLICK_RELEASE;
                            default:
                                // other mouse events
                                break;
                        }
                        
                    }
                }                
            }
        } else if (seq[0] == 'O') {
            switch (seq[1]) {
                case 'H': return HOME_KEY; // \x1bOH
                case 'F': return END_KEY; // \x1bOF
            }
        }

        return ESC; // no mapping: <ESC>
    }

    return byte_in; // no mapping
}