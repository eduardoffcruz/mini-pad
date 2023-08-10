#ifndef KEY_MAPPING_H
#define KEY_MAPPING_H

#include <unistd.h>

/*** keypress mapping***/

#define CTRL_KEY(k) ((k) & 0x1f)

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
    DEL_KEY,
    NEXT, // CTRL+ARROW_RIGHT
    PREV //CTRL+ARROW_LEFT
};

int map_keypress(char byte_in);

#endif