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


/*** terminal mode***/

/*
* Enable terminal's raw mode: so to read byte-by-byte instead of line-by-line.
* - By default, terminal is in canonical mode: keyboard input is only sent via STDIN when Enter is pressed.
*/
void terminal_enable_raw_mode(void);

/*
* Reset terminal's mode back to default (canonical mode).
*/
void terminal_disable_raw_mode(void);


/*** terminal info ***/

/*
* 
*/
int terminal_get_window_size(unsigned short *rows, unsigned short *cols);


/*** terminal control ***/

/*
* Exit and output error.
*/
void die(const char *str);

/*
* 
*/
void terminal_clear(void);

/*
* 
*/
void terminal_set_cursor_topleft(void);

/*
* 
*/
void terminal_wipe_screen(void);

/*
* 
*/
void terminal_wipe_line(void);

/*
* 
*/
void enable_mouse_reporting(void);

/*
* 
*/
void disable_mouse_reporting(void);


#endif /* TERMINAL_H */