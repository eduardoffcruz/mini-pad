#ifndef KEY_MAPPING_H
#define KEY_MAPPING_H

#include <unistd.h>


/*** keypress mapping***/

enum editor_key {
    ENTER = '\r',
    ESC = '\x1b',
    BACKSPACE = 127,
    ARROW_LEFT = 1000,
    ARROW_RIGHT,
    ARROW_UP,
    ARROW_DOWN,
    PAGE_UP,
    PAGE_DOWN,
    HOME_KEY,
    END_KEY,
    DEL_KEY
};

int map_keypress(char byte_in);

#endif