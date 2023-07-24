

#include "terminal_handler.h"
#include "editor.h"



int main(void) {
    // change terminal mode from canonical to raw
    enable_raw_mode();
    init_editor();
    
    // reads byte input while not EOF and not 'q'
    while(1){
        // wait for one keypress and return it
        editor_refresh_screen();
        editor_process_keypress();
    }

    return 0;
}