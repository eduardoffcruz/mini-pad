#include "terminal_handler.h"

struct termios original_cfg; 

void enable_raw_mode(){
    // save copy of the original terminal cfg
    if (tcgetattr(STDIN_FILENO, &original_cfg) == -1) die("tcgetattr");
    // set disable_raw_mode func to be called when exit is called
    atexit(disable_raw_mode); 

    // create and apply new terminal config to be in raw mode
    struct termios new_cfg = original_cfg;
    new_cfg.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);
    new_cfg.c_oflag &= ~(OPOST); // turn off all output post-processing
    new_cfg.c_cflag |= (CS8);
    new_cfg.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG); //c_lflag "local flags", c_iflag "input flags", c_oflag "output flags", c_cflag "control flags"
    //new_cfg.c_cc[VMIN] = 0; // VMIN value sets the min number of bytes of input needed before read() can return
    //new_cfg.c_cc[VTIME] = 1; // VMIN sets max amount of time to wait before read() returns
    
    // apply changed attrs to terminal (TCAFLUSH discards any unread input)
    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &new_cfg) == -1) die("tcsetattr");
}

void disable_raw_mode(){
    // reset terminal attrs back to original
    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &original_cfg) == -1) die("tcsetattr");
}

void die(const char *str){
    clear();

    perror(str); // looks at global errno var to indicate what error was
    exit(1);
}

void clear_screen(){
    write(STDOUT_FILENO, "\x1b[2J", 4); // clear the screen: <esc>[2J
}

void position_cursor(uint8_t row, uint8_t col){
    char cmd[11];
    snprintf(cmd, sizeof(cmd), "\x1b[%d;%dH", row, col);
    write(STDOUT_FILENO, "\x1b[", strlen(cmd)); // reposition cursos at top-left corner: <esc>[H (== <esc>[1;1H)
}

void position_cursor_topleft(){
    write(STDOUT_FILENO, "\x1b[H", 3);
}

void clear(){
    // Escape sequences instruct the terminal to do various text formatting tasks, such as coloring text, moving the cursor around, and clearing parts of the screen (see vt100.net/docs/vt100-ug/chapter3.html for complete explanation).
    clear_screen();
    position_cursor_topleft();

}