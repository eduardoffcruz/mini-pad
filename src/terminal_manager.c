#include "terminal_manager.h"

struct termios original_cfg; 

void enable_raw_mode(){
    // save copy of the original terminal cfg
    tcgetattr(STDIN_FILENO, &original_cfg);
    // set disable_raw_mode func to be called when exit is called
    atexit(disable_raw_mode); 

    // create and apply new terminal config to be in raw mode
    struct termios new_cfg = original_cfg;
    new_cfg.c_lflag &= ~(ECHO | ICANON); //c_lflag "local flags", c_iflag "input flags", c_oflag "output flags", c_cflag "control flags"
    // apply changed attrs to terminal (TCAFLUSH discards any unread input)
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &new_cfg);
}

void disable_raw_mode(){
    // reset terminal attrs back to original
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &original_cfg);
}

