#ifndef KEY_MAPPING_H
#define KEY_MAPPING_H

#include <unistd.h>

enum editor_key {
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

int map_key(char byte_in);

#endif