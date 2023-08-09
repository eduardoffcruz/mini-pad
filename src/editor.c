#include "editor.h"

struct editor_state editor;


/*** editor operations ***/

void editor_init(void){
    //terminal_clear_all(void);
    editor.cursor_x = 0UL;
    editor.cursor_y = 0UL;
    editor.render_x = 0UL;
    editor.prev_x = 0UL;
    editor.row_offset = 0UL; // offset between top(y=0) and current view
    editor.col_offset = 0UL; // left of the file scroll

    editor.filename = NULL;
    editor.txt = NULL;

    editor.info[0] = 0;
    editor.info_len = 0U;
    editor.info_time = 0;
    editor.default_info_len = strlen(DEFAULT_INFO);
    //editor.txt = INIT_TEXT;

    editor_update_text_window_size();
}

void editor_open_file(const char *filename){
    free_text(editor.txt);
    if ((editor.txt = new_text()) == NULL){
        die("new_text");
    }

    if (read_file_to_text(filename, editor.txt) != 0) die("open_file");

    // Record filename of currently opened file, in editor's sata
    free(editor.filename);
    editor.filename = strdup(filename);
}

void editor_empty_file(const char *filename){
    free_text(editor.txt);
    if ((editor.txt = new_text()) == NULL){
        die("new_text");
    }

    if (append_line(editor.txt, editor.txt->lines_num, "", 0) != 0){
        die("append_file");
    }

    if (filename != NULL){
        free(editor.filename);
        editor.filename = strdup(filename);
    }
}

void editor_insert_char(char ch){
    if (editor.cursor_y == editor.txt->lines_num){
        // add new line
        if (append_line(editor.txt, editor.txt->lines_num, "", 0) != 0){
            die("append_file");
        }
    }
    // insert char at curr cursor location
    if (insert_char(&editor.txt->lines[editor.cursor_y], editor.cursor_x, ch) != 0) die("insert_char");
    
    // flag text as modified
    editor.txt->modified = True;
}

void editor_delete_char(void){
    if(editor.cursor_y == editor.txt->lines_num || (editor.cursor_x == 0 && editor.cursor_y == 0)){
        // cursor past the EOF or in the beggining of file
        return;
    }

    if(editor.cursor_x > 0){
        // delete char at curr cursor location
        delete_char(&(editor.txt->lines[editor.cursor_y]), editor.cursor_x - 1);
        editor.cursor_x--;
    } else{
        editor.cursor_x = editor.txt->lines[editor.cursor_y - 1].raw_len;
        if (merge_lines(editor.txt, editor.cursor_y, editor.cursor_y - 1) != 0){
            die("merge_lines");
        }
        editor.cursor_y--;
    }

    // flag text as modified
    editor.txt->modified = True;
    return;
}

void editor_insert_newline(void){
    if(editor.cursor_x == 0){
        if (append_line(editor.txt, editor.cursor_y, "", 0) != 0){
            die("append_line");
        }
        
    } else{
        line *ln = &editor.txt->lines[editor.cursor_y];
        if (append_line(editor.txt, editor.cursor_y + 1, &ln->raw[editor.cursor_x], ln->raw_len - editor.cursor_x) != 0){
            die("append_line");
        }
        ln = &(editor.txt->lines[editor.cursor_y]);
        ln->raw_len = editor.cursor_x;
        ln->raw[ln->raw_len] = 0;
        update_rendered(ln);
    }
    editor.cursor_y++;
    editor.cursor_x = 0;

    editor.txt->modified = True;
}

void editor_save_file(void){
    int err;
    // Request filename if NULL
    if (editor.filename == NULL && (editor.filename = editor_prompt("Save as: %s [<ESC> = cancel]", 0, NULL)) == NULL){
        editor_set_info("Save aborted.");
        return;
    }

    // Check if text has been modified
    if (!editor.txt->modified){
        editor_set_info("Saved.");
        return;
    }

    // Save modified text to file
    err = save_text_to_file(editor.txt, compute_text_size(editor.txt), editor.filename);
    switch(err){
        case 0:
            // Update modified flag
            editor.txt->modified = False; // reset
            editor_set_info("Your changes have been saved.");
            break;
        case -1:
            editor_set_info("Failed to save file: no filename");
            break;
        case -2:
        case -3:
        case -4:
        case -5:
            editor_set_info("Failed to save file: %s", strerror(errno));
            break;
    }

    return;    
}


void editor_find(void){
    size_t query_len;
    char *query = editor_prompt("Search: %s [<ESC> = cancel]", &query_len, find_callback);
    if (query == NULL) return;

    free(query);    
}


/*** rendering ***/

void editor_refresh_screen(char scroll){
    editor_update_text_window_size(); // keep text window size updated

    if (scroll){
        editor_line_scroll();
    }

    struct dynamic_buffer buf = INIT_DYNAMIC_BUFFER;

    append_buffer(&buf, "\x1b[?25l", 6); // hide cursor
    append_buffer(&buf, "\x1b[H", 3); // position cursor top-left

    editor_render_text(&buf);
    editor_render_status_bar(&buf);
    editor_render_info_bar(&buf);

    // position cursor in (row,col)
    char cmd[64];
    snprintf(cmd, sizeof(cmd), "\x1b[%ld;%ldH", (editor.cursor_y - editor.row_offset) + 1, (editor.render_x - editor.col_offset) + 1);
    append_buffer(&buf, cmd, strlen(cmd));

    append_buffer(&buf, "\x1b[?25h", 6); // show cursor
     
    /*char aux[100];
    sprintf(aux,"[%d,%d,%d]",editor.row_offset,editor.cursor_y,editor.screen_height);
    append_buffer(&buf,aux,strlen(aux));*/

    write(STDOUT_FILENO, buf.bytes, buf.len);
    free_buffer(&buf);
}

void editor_render_text(struct dynamic_buffer *buf) {
    unsigned short y;
    for (y = 0; y < editor.screen_height; y++) {
        unsigned long file_row = y + editor.row_offset;
        if (file_row >= editor.txt->lines_num){
            if (editor.txt->lines_num == 1 && editor.txt->lines[0].raw_len == 0 && y == editor.screen_height / 3) {
                char welcome_msg[64];
                int welcome_msg_len = snprintf(welcome_msg, sizeof(welcome_msg), WELCOME_MSG, APP_VERSION);
                if (welcome_msg_len > editor.screen_width){
                    welcome_msg_len = editor.screen_width;
                }
                int padding = (editor.screen_width - welcome_msg_len) / 2;
                if (padding) {
                    append_buffer(buf, "~", 1);
                    padding--;
                }
                while (padding--){
                    append_buffer(buf, " ", 1);
                } 
                append_buffer(buf, welcome_msg, welcome_msg_len);
            } else {
                append_buffer(buf, "~", 1);
            }
            
        } else{
            long line_len = (long)editor.txt->lines[file_row].rendered_len - editor.col_offset;
            if (line_len < 0){
                line_len = 0;
            } else if (line_len > editor.screen_width) {
                line_len = editor.screen_width;
            }
            append_buffer(buf, &(editor.txt->lines[file_row].rendered[editor.col_offset]), line_len);
        }

        append_buffer(buf, "\x1b[K", 3);
        //if (y < editor.screen_height - 1) {
        append_buffer(buf, "\r\n", 2);
        //}
    }
    //append_buffer(buf, "\r\n", 2);
}

void editor_render_status_bar(struct dynamic_buffer *buf){
    char file_info[64], navigation_info[32];
    int file_info_len = snprintf(file_info, sizeof(file_info), editor.txt->modified ? "%.20s *" : "%.20s", editor.filename ? editor.filename : "[unnamed]");
    int navigation_info_len = snprintf(navigation_info, sizeof(navigation_info), "%ld/%ld", editor.cursor_y + 1, editor.txt->lines_num);

    append_buffer(buf, "\x1b[7m", 4); // switch to inverted colors
    append_buffer(buf, file_info, file_info_len > editor.screen_width ? editor.screen_width : file_info_len);
    while(editor.screen_width - file_info_len > navigation_info_len){
        append_buffer(buf, " ", 1);
        file_info_len++;
    }
    
    if (editor.screen_width - file_info_len == navigation_info_len){
        append_buffer(buf, navigation_info, navigation_info_len); // places navigation_info the most to the right
    } 
    append_buffer(buf,"\x1b[m",3); // switch back to normal formating
    append_buffer(buf, "\r\n", 2);
}

void editor_render_info_bar(struct dynamic_buffer *buf){
    append_buffer(buf, "\x1b[K", 3); // clear info bar
    if (editor.info_len && time(NULL) - editor.info_time < INFO_PERIOD){
        append_buffer(buf, editor.info, editor.info_len > editor.screen_width ? editor.screen_width : editor.info_len);
    } else {
        append_buffer(buf, DEFAULT_INFO, editor.default_info_len > editor.screen_width ? editor.screen_width : editor.default_info_len);
    }
}

void editor_set_info(const char *fmt, ...){
    // Takes a format str and a variable number of arguments, like the printf family of functions
    va_list args;
    va_start(args, fmt);
    editor.info_len = vsnprintf(editor.info, sizeof(editor.info), fmt, args); // safe!
    va_end(args);
    editor.info_time = time(NULL); // timestamp
}

void editor_update_text_window_size(void){
    if (terminal_get_window_size(&editor.screen_height, &editor.screen_width) == -1) die("terminal_get_window_size");
    editor.screen_height -= 2; // to hold the status+info bar
}


/*** keypress handling ***/

int editor_read_keypress(void){
    ssize_t nread;
    char byte_in;
    while ((nread = read(STDIN_FILENO, &byte_in, 1)) != 1) {
        if (nread == -1 && errno != EAGAIN) die("read");
    }

    return map_keypress(byte_in);
}

void editor_process_keypress(void){
    static char insist_quit = False;

    int key_val = editor_read_keypress();
    switch (key_val) {
        case ENTER:
            editor_insert_newline();
            break;

        case CTRL_KEY('q'):
            if (editor.txt->modified && !insist_quit){
                insist_quit = True;
                editor_set_info(UNSAVED_QUIT_INFO);
                return;
            }
            editor_quit();
            break;

        case CTRL_KEY('s'):
            editor_save_file();
            if (insist_quit){
                editor_quit();
            }
            break;

        case CTRL_KEY('f'):
            editor_find();
            break;

        case ARROW_UP:
        case ARROW_DOWN:
        case ARROW_LEFT:
        case ARROW_RIGHT:
            editor_move_cursor(key_val);
            break;

        case PAGE_DOWN:
        case PAGE_UP:
            editor_page_scroll(key_val);
            break;
   
        case HOME_KEY: // all-left
            editor.cursor_x = 0;
            break;
        case END_KEY: // all-right
            if(editor.cursor_y < editor.txt->lines_num){
                editor.cursor_x = editor.txt->lines[editor.cursor_y].raw_len;
            }
            break;

        case BACKSPACE:
        case CTRL_KEY('h'):
        case DEL_KEY:
            if (key_val == DEL_KEY){
                editor_move_cursor(ARROW_RIGHT);
            }
            editor_delete_char();
            editor.prev_x = editor.cursor_x;
            break;

        case ESC:
            if (insist_quit){
                // cancel
                editor_set_info(DEFAULT_INFO);
            } else{
                ; // TODO: copy selected text...
            }
            break;

        case CTRL_KEY('l'): // refresh terminal cmd
            break; // do nothing

        default:
            editor_insert_char((char)key_val);
            editor.cursor_x++;
            break;
    }

    insist_quit = False;
}


/*** editor control ***/

void editor_move_cursor(int key_val){
    switch (key_val) {
        case ARROW_LEFT:
            if(editor.cursor_x != 0){
                editor.cursor_x--;
            } else if(editor.cursor_y > 0){
                editor.cursor_y--;
                editor.cursor_x = editor.txt->lines[editor.cursor_y].raw_len;
            } else{
                return;
            }
            editor.prev_x = editor.cursor_x;
            return;
        case ARROW_RIGHT:
            if(editor.cursor_y < editor.txt->lines_num){
                unsigned long line_len = editor.txt->lines[editor.cursor_y].raw_len;
                if(editor.cursor_x < line_len){
                    editor.cursor_x++;
                } else if(editor.cursor_x == line_len && editor.cursor_y + 1 < editor.txt->lines_num){ // check if end of curr line
                    editor.cursor_y++;
                    editor.cursor_x = 0;
                } else{
                    return;
                }
                editor.prev_x = editor.cursor_x;
            }
            return;
        case ARROW_UP:
            if(editor.cursor_y != 0){
                editor.cursor_y--;
                break; // leave switch block
            } 
            return;
        case ARROW_DOWN:
            if(editor.cursor_y + 1 < editor.txt->lines_num){
                editor.cursor_y++;
                break; // leave switch block
            }
            return;
        default:
            return;
    }

    // Fix special case 
    handle_cursor_x();
    
    return;
}

void editor_line_scroll(void){
    editor.render_x = 0;
    if (editor.cursor_y < editor.txt->lines_num){
        editor.render_x = rx_to_cx(&(editor.txt->lines[editor.cursor_y]), editor.cursor_x);
    }

    // -- Check if cursors are outside of visible screen
    // Vertical scroll
    if(editor.cursor_y < editor.row_offset) // cursor_y is above visible window  
        editor.row_offset = editor.cursor_y; // scroll to where cursor is
    if(editor.cursor_y >= editor.screen_height + editor.row_offset) // cursor_y is past bottom visible window
        editor.row_offset = editor.cursor_y - editor.screen_height + 1;
    
    // Horizontal scroll
    if(editor.render_x < editor.col_offset)   
        editor.col_offset = editor.render_x;
    if(editor.render_x >= editor.screen_width + editor.col_offset) 
        editor.col_offset = editor.render_x - editor.screen_width + 1;
}

void editor_page_scroll(int key_val){
    const int page_scroll_offset = editor.screen_height-2;
    switch(key_val){
        case PAGE_DOWN:
            if(editor.row_offset+editor.screen_height >= editor.txt->lines_num){
                editor.cursor_y = editor.txt->lines_num-1;
            } else if (editor.cursor_y+page_scroll_offset > editor.txt->lines_num){
                int y_offset = (editor.cursor_y - editor.row_offset);
                editor.row_offset = editor.txt->lines_num - y_offset;
                editor.cursor_y = editor.txt->lines_num-1;
            } else{
                editor.row_offset += page_scroll_offset;
                editor.cursor_y += page_scroll_offset;
            }
            break;
        case PAGE_UP:
            if (editor.row_offset == 0){
                editor.cursor_y = 0; // go to top of document
            }
            else if ((long)editor.row_offset - page_scroll_offset < 0){
                editor.cursor_y -= editor.row_offset;
                editor.row_offset = 0; // fix view
                //if (editor.cursor_y < 0){
                //    editor.cursor_y += page_scroll_offset;
                //}
            } else{
                editor.row_offset -= page_scroll_offset;
                editor.cursor_y -= page_scroll_offset;
            }
            break;
        default:
            return;
    }

    handle_cursor_x();
}

void editor_quit(void){
    free(editor.filename);
    free_text(editor.txt);
    terminal_clear();
    exit(0);
}

void editor_search_navigation(unsigned long** occs, unsigned long query_len, int key_val){
    if (occs == NULL){
        return;
    }

    // start by highlighting all matches inside current window
    unsigned long line_i;
    unsigned int max_occ,last_occ_i;

    unsigned long curr_cy = editor.cursor_y, curr_rx = editor.render_x;

    switch (key_val){
        case ARROW_LEFT:
            // TODO: place cursor on match before cursor coordinates
            break;
        case ARROW_RIGHT:
            // place cursor on match after cursor coordinates
            if(++curr_rx >= editor.txt->lines[curr_cy].rendered_len){
                curr_rx = 0;
                if (++curr_cy >= editor.txt->lines_num){
                    curr_cy = 0;
                }
            }
        default:
            // place cursor on match closer to cursor coordinates
            line *ln = &(editor.txt->lines[curr_cy]);
            max_occ = ln->rendered_len/query_len;
            last_occ_i = occs[curr_cy][max_occ];
            long res = -1;
            if (last_occ_i != -1){
                unsigned int l = 0, r = last_occ_i;
                while (l <= r) {
                    unsigned int mid = l + (r - l) / 2;
                    if (occs[curr_cy][mid] < curr_rx) {
                        l = mid + 1;  // Search in the right half
                    } else {
                        res = mid;  
                        r = mid - 1;  // Search in the left half
                    }
                }
                if (res != -1){
                    editor.cursor_y = curr_cy;
                    editor.render_x = occs[line_i][res];
                    editor.cursor_x = rx_to_cx(ln, editor.render_x);
                    return;
                }
            }

            // not found in the current cursor line, check next lines
            for (unsigned long line_i = curr_cy+1; line_i < editor.txt->lines_num; line_i++){
                ln = &(editor.txt->lines[line_i]);
                max_occ = ln->rendered_len/query_len;
                last_occ_i = occs[line_i][max_occ];
                if (last_occ_i != -1){
                    editor.cursor_y = line_i;
                    editor.render_x = occs[line_i][0];
                    editor.cursor_x = rx_to_cx(ln, editor.render_x);
                    return;
                }
            }
            
            break;
    }
        
}

void editor_search_prompt(char* prompt){
    unsigned long** occs = NULL; // only for find functionality

    size_t buf_size = 128;
    char* buf = (char*)malloc(sizeof(char)*buf_size);
    if (buf == NULL){
        return NULL;
    }
    
    unsigned long prev_cursor_y = editor.cursor_y, prev_render_x = editor.render_x;
    /*editor.cursor_y = editor.screen_height+2;
    editor.render_x = 0;
    while (editor.render_x < editor.screen_width && prompt[editor.render_x] != '\0' && !(prompt[editor.render_x] == '%' && prompt[editor.render_x + 1] == 's')) {
        editor.render_x++;
    }*/

    unsigned long input_len = 0;
    buf[0] = 0;
    while(input_len*sizeof(char) < buf_size){
        editor_set_info(prompt, buf);
        editor_refresh_screen(False);

        int key_val = editor_read_keypress();
        if ((key_val == DEL_KEY || key_val == CTRL_KEY('h') || key_val == BACKSPACE) && input_len != 0){
            buf[--input_len] = 0;
            //editor.render_x--;
            occs = get_occurrences(editor.txt, buf, input_len);
            editor_search_navigation(occs, input_len, key_val);
                // present all occs, highlighting nearest occurence
                ;
                //fazer função para isto TODO:
            

        } else if (key_val == ESC){
            // stop search
            editor_set_info("");
            //editor.render_x = prev_render_x, editor.cursor_y = prev_cursor_y;
            free_occurrences(occs, editor.txt->lines_num);
            free(buf);
            return NULL;

        } else if (key_val == ENTER){
            // TODO:
            // stop search and continue editing at currently highlighed (selected) position and get back to normal editing

        } else if (!iscntrl(key_val) && key_val < 128){
            // control chars are non-printable characters
            if(input_len == buf_size - 1 && buf_size < 4096*100){
                buf_size *= 2;
                if ((buf = (char*)realloc(buf, sizeof(char)*buf_size))==NULL){
                    //editor.render_x = prev_render_x, editor.cursor_y = prev_cursor_y;
                    free_occurrences(occs, editor.txt->lines_num);
                    free(buf);
                    return NULL;
                }
            }
            buf[input_len++] = key_val;
            buf[input_len] = 0;
            //editor.render_x++;

            occs = get_occurrences(editor.txt, buf, input_len);
            editor_search_navigation(occs, input_len, key_val);
            // present all occs, highlighting nearest occurence
            
        } else if (key_val == ARROW_RIGHT || key_val == ARROW_LEFT){
            // present all occs, highlighting next or prev occurence
            editor_search_navigation(occs, input_len, key_val);
        }
    }
    //editor.render_x = prev_render_x, editor.cursor_y = prev_cursor_y;

    free_occurrences(occs, editor.txt->lines_num);
    free(buf);
}

char* editor_input_prompt(char* prompt, size_t* input_len){
    size_t buf_size = 128;
    char* buf = (char*)malloc(sizeof(char)*buf_size);
    if (buf == NULL){
        return NULL;
    }
    
    unsigned long prev_cursor_y = editor.cursor_y, prev_render_x = editor.render_x;
    editor.cursor_y = editor.screen_height+2;
    editor.render_x = 0;
    while (editor.render_x < editor.screen_width && prompt[editor.render_x] != '\0' && !(prompt[editor.render_x] == '%' && prompt[editor.render_x + 1] == 's')) {
        editor.render_x++;
    }

    *input_len = 0;
    buf[0] = 0;
    while((*input_len)*sizeof(char) < buf_size){
        editor_set_info(prompt, buf);
        editor_refresh_screen(False);

        int ch = editor_read_keypress();
        if ((ch == DEL_KEY || ch == CTRL_KEY('h') || ch == BACKSPACE) && (*input_len) != 0){
            buf[--(*input_len)] = 0;
            editor.render_x--;
        } else if (ch == ESC || ch == CTRL_KEY('q')){
            editor_set_info("");
            editor.render_x = prev_render_x, editor.cursor_y = prev_cursor_y;
            free(buf);
            return NULL;
        } else if (ch == ENTER && (*input_len) != 0){
            // for reading input
            editor_set_info("");
            editor.render_x = prev_render_x, editor.cursor_y = prev_cursor_y;
            return buf;
        } else if (!iscntrl(ch) && ch < 128){
            // control chars are non-printable characters
            if((*input_len) == buf_size - 1 && buf_size < 4096){
                buf_size *= 2;
                if ((buf = (char*)realloc(buf, sizeof(char)*buf_size))==NULL){
                    editor.render_x = prev_render_x, editor.cursor_y = prev_cursor_y;
                    return NULL;
                }
            }
            buf[(*input_len)++] = ch;
            buf[*input_len] = 0;
            editor.render_x++;
        }
    }

    editor.render_x = prev_render_x, editor.cursor_y = prev_cursor_y;
    return buf;
}


/*** auxiliar ***/

void handle_cursor_x(void){
    // Account for special case where cursor goes from end of long line to next smaller line (adjust cursor_x accordingly)
    unsigned long line_len;
    if (editor.cursor_y < editor.txt->lines_num){
        if (editor.cursor_x > (line_len = editor.txt->lines[editor.cursor_y].rendered_len)){
            editor.cursor_x = line_len;
        } else if (editor.prev_x <= line_len) {
            editor.cursor_x = editor.prev_x;
        }
    }
    return;
}

unsigned long cx_to_rx(line *ln, unsigned long cx){
    unsigned long rx = 0;
    for (unsigned long j = 0; j < cx; j++){
        if (ln->raw[j] == '\t'){
            rx += ((TAB_SIZE-1) - (rx % TAB_SIZE));
        }
        rx++;
    }
    return rx;
}

unsigned long rx_to_cx(line *ln, unsigned long rx){
    unsigned long curr_rx = 0;
    unsigned long cx;
    for (cx = 0; cx < ln->raw_len; cx++){
        if(ln->raw[cx] == '\t'){
            curr_rx += (TAB_SIZE - 1) - (curr_rx % TAB_SIZE);
        }
        curr_rx++;
        if(curr_rx > rx) return cx;
    }
    return cx;
}
