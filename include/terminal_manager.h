#ifndef TERMINAL_MANAGER_H
#define TERMINAL_MANAGER_H

#include <stdlib.h>
#include <unistd.h>
#include <termios.h>


/*
* Enable terminal's raw mode: so to read byte-by-byte instead of line-by-line.
* - By default, terminal is in canonical mode: keyboard input is only sent via STDIN when Enter is pressed.
*/
void enable_raw_mode();

/*
* Reset terminal's mode back to default (canonical mode).
*/
void disable_raw_mode();



#endif /* TERMINAL_MANAGER_H */