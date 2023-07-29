#include "terminal.h"

struct termios origin_termios; 


/*** terminal mode***/

void terminal_enable_raw_mode(){
    // save copy of the original terminal cfg
    if (tcgetattr(STDIN_FILENO, &origin_termios) == -1) die("tcgetattr");
    // set disable_raw_mode func to be called when exit is called
    atexit(terminal_disable_raw_mode); 

    // create and apply new terminal config to be in raw mode
    struct termios new_cfg = origin_termios;
    new_cfg.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);
    new_cfg.c_oflag &= ~(OPOST); // turn off all output post-processing
    new_cfg.c_cflag |= (CS8);
    new_cfg.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG); //c_lflag "local flags", c_iflag "input flags", c_oflag "output flags", c_cflag "control flags"
    new_cfg.c_cc[VMIN] = 0; // VMIN value sets the min number of bytes of input needed before read() can return
    new_cfg.c_cc[VTIME] = 1; // VMIN sets max amount of time to wait before read() returns
    
    // apply changed attrs to terminal (TCAFLUSH discards any unread input)
    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &new_cfg) == -1) die("tcsetattr");
}

void terminal_disable_raw_mode(){
    // reset terminal attrs back to original
    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &origin_termios) == -1) die("tcsetattr");
}


/*** terminal info ***/

int terminal_get_window_size(int *rows, int *cols){
    struct winsize ws;
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == -1 || ws.ws_col == 0) {
        return -1;
    } else {
        *cols = ws.ws_col;
        *rows = ws.ws_row;
        return 0;
    }
}


/*** terminal control ***/

void die(const char *str){
    terminal_clear();

    perror(str); // looks at global errno var to indicate what error was
    exit(1);
}

void terminal_clear(){
    // Escape sequences instruct the terminal to do various text formatting tasks, such as coloring text, moving the cursor around, and clearing parts of the screen (see vt100.net/docs/vt100-ug/chapter3.html for complete explanation).
    terminal_wipe_screen();
    terminal_set_cursor_topleft();
}

void terminal_set_cursor_topleft(){
    write(STDOUT_FILENO, "\x1b[H", 3);
}

void terminal_wipe_screen(){
    write(STDOUT_FILENO, "\x1b[2J", 4); // clear the screen: <esc>[2J
}

void terminal_wipe_line(){
    write(STDOUT_FILENO, "\x1b[K", 3); // clear line: <esc>[K
}