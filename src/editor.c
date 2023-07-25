#include "editor.h"

struct editor_cfg cfg;

void editor_init(){
    //clear_all();
    cfg.cx = 0;
    cfg.cy = 0;

    if (get_window_size(&cfg.screen_rows, &cfg.screen_cols) == -1) die("get_window_size");
}

int get_window_size(int *rows, int *cols){
    struct winsize ws;
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == -1 || ws.ws_col == 0) {
        return -1;
    } else {
        *cols = ws.ws_col;
        *rows = ws.ws_row;
        return 0;
    }
}

void editor_refresh_screen(){
    if (get_window_size(&cfg.screen_rows, &cfg.screen_cols) == -1) die("get_window_size");
    struct abuf buffer = ABUF_INIT;

    buffer_append(&buffer, "\x1b[?25l", 6); // hide cursor
    buffer_append(&buffer, "\x1b[H", 3); // position cursor top-left
    editor_draw_rows(&buffer);

    // position cursor in (row,col)
    char cmd[28];
    snprintf(cmd, sizeof(cmd), "\x1b[%d;%dH", cfg.cy + 1, cfg.cx + 1);
    buffer_append(&buffer, cmd, strlen(cmd));

    buffer_append(&buffer, "\x1b[?25h", 6); // show cursor

    write(STDOUT_FILENO, buffer.b, buffer.len);
    free_buffer(&buffer);
}

void editor_draw_rows(struct abuf *buffer) {
    int y;
    for (y = 0; y < cfg.screen_rows; y++) {
        if (y == cfg.screen_rows / 3) {
            char welcome_msg[80];
            int welcome_msg_msg_len = snprintf(welcome_msg, sizeof(welcome_msg),"Text editor -- version %s", APP_VERSION);
            if (welcome_msg_msg_len > cfg.screen_cols){
                welcome_msg_msg_len = cfg.screen_cols;
            }
            int padding = (cfg.screen_cols - welcome_msg_msg_len) / 2;
            if (padding) {
                buffer_append(buffer, "~", 1);
                padding--;
            }
            while (padding--){
                buffer_append(buffer, " ", 1);
            } 
            buffer_append(buffer, welcome_msg, welcome_msg_msg_len);
        } else {
            buffer_append(buffer, "~", 1);
        }
        buffer_append(buffer, "\x1b[K", 3);
        
        if (y < cfg.screen_rows - 1) {
            buffer_append(buffer, "\r\n", 2);
        }
    }
    //buffer_append(buffer, "\r\n", 2);
}

int editor_read_key(){
    int nread;
    char byte_in;
    while ((nread = read(STDIN_FILENO, &byte_in, 1)) != 1) {
        if (nread == -1 && errno != EAGAIN) die("read");
    }

    if(byte_in == '\x1b'){
        char seq[3];

        if(read(STDIN_FILENO, &seq[0], 1) != 1) return '\x1b';
        if(read(STDIN_FILENO, &seq[1], 1) != 1) return '\x1b';
        
        if(seq[0] == '['){
            switch (seq[1]){
                case 'A': return ARROW_UP;
                case 'B': return ARROW_DOWN;
                case 'C': return ARROW_RIGHT;
                case 'D': return ARROW_LEFT;
            }
        }
        return '\x1b'; 
    }

    return byte_in;
}

void editor_process_keypress(){
    int key_val = editor_read_key();
    switch (key_val) {
        case CTRL_KEY('q'):
            clear_all();
            exit(0);
            break;

        case ARROW_UP:
        case ARROW_DOWN:
        case ARROW_LEFT:
        case ARROW_RIGHT:
            editor_move_cursor(key_val);
            break;
  }
}

void editor_move_cursor(int key_val){
    switch (key_val) {
        case ARROW_LEFT:
            if(cfg.cx > 0) cfg.cx--;
            break;
        case ARROW_UP:
            if(cfg.cy > 0) cfg.cy--;
            break;
        case ARROW_RIGHT:
            if(cfg.cx < cfg.screen_cols-1) cfg.cx++;
            break;
        case ARROW_DOWN:
            if(cfg.cy < cfg.screen_rows-1) cfg.cy++;
            break;
    }
}
