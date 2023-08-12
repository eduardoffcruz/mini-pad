#include "editor.h"



int main(int argc, char *argv[]) {
    // change terminal mode from canonical to raw
    terminal_enable_raw_mode();
    editor_init();

    if (argc >= 2){
        if (access(argv[1], F_OK) != -1){
            // file exists
            editor_open_file(argv[1]);
        } else{
            // file doesn't exists, create new empty file assuming such name
            editor_empty_file(argv[1]);
        }
    } else{
        editor_empty_file(NULL);
    }

    //editor_set_info("HELP: Ctrl-Q = quit");
    
    // reads byte input while not EOF and not 'q'
    while(1){
        // wait for one keypress and return it
        editor_refresh_screen();
        editor_process_keypress();
    }

    return 0;
}