#ifndef TERMINAL_H
#define TERMINAL_H

#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <termios.h>
#include <stdint.h>
#include <string.h>
#include <sys/ioctl.h>

/*
* 
*/
int terminal_get_window_size(int *rows, int *cols);

/*
* Enable terminal's raw mode: so to read byte-by-byte instead of line-by-line.
* - By default, terminal is in canonical mode: keyboard input is only sent via STDIN when Enter is pressed.
*/
void terminal_enable_raw_mode();

/*
* Reset terminal's mode back to default (canonical mode).
*/
void terminal_disable_raw_mode();

/*
* Exit and output error.
*/
void die(const char *str);

/*
* .
*/
void terminal_position_cursor(uint8_t row, uint8_t col);
void terminal_position_cursor_topleft();
void terminal_clear_screen();
void terminal_clear_all();
void terminal_clear_line();



#endif /* TERMINAL_H */