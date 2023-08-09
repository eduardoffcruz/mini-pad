#include "kmp_search.h"

void compute_LPS_array(char *pattern, size_t pattern_len, int *lps) {
    int length = 0; // length of the previous longest prefix suffix

    lps[0] = 0; // lps[0] is always 0
    size_t i = 1;

    while (i < pattern_len) {
        if (pattern[i] == pattern[length]) {
            length++;
            lps[i] = length;
            i++;
        } else {
            if (length != 0) {
                length = lps[length - 1];
            } else {
                lps[i] = 0;
                i++;
            }
        }
    }
}

/*
* Returns first occurence of pattern in text 
*/
char* KMP_search(int *lps, char *text, char *pattern, unsigned long text_len, unsigned long pattern_len) {

    int i = 0; // Index for text[]
    int j = 0; // Index for pattern[]

    while (i < text_len) {
        if (pattern[j] == text[i]) {
            j++;
            i++;
        }

        if (j == pattern_len) {
            return &text[i-j];
            //printf("Pattern found at index %d\n", i - j);
            //j = lps[j - 1];
        } else if (i < text_len && pattern[j] != text[i]) {
            if (j != 0) {
                j = lps[j - 1];
            } else {
                i++;
            }
        }
    }
    return NULL;
}

void free_occurrences(unsigned long** occs, unsigned long lines_num){
    if (occs != NULL){
        for(unsigned long line_i = 0; line_i < lines_num; line_i++){
            free(occs[line_i]);
        }
        free(occs);
    }
}

unsigned long** get_occurrences(text* txt, char *query, unsigned long query_len){
    // Create lps[] that will hold the longest prefix suffix values for pattern (for KMP_search)
    int *lps = (int*)malloc(sizeof(int) * query_len);
    if (lps == NULL) {
        return NULL;
    }
    compute_LPS_array(query, query_len, lps);

    // record all occurences indexes per line in matrix
    unsigned long** occurrences = (unsigned long**)malloc(sizeof(unsigned long*)*txt->lines_num);
    if (occurrences == NULL){
        return NULL;
    }

    for (unsigned long line_i = 0; line_i < txt->lines_num; line_i++){
        // Find all occurences in line_i
        line *ln = &(txt->lines[line_i]);
        unsigned int max_occ = ln->rendered_len/query_len;
        occurrences[line_i] = (unsigned long*)malloc(sizeof(unsigned long)*(max_occ+1)); //+1 to hold last written index of occurences[line_i]. 
        if(occurrences[line_i] == NULL){
            return NULL;
        }
        unsigned int occ_i = 0; // index of where occ will be written in occurrences[line_i].
        char *match;
        unsigned long j = 0;
        while(j < ln->rendered_len && (match = KMP_search(lps, &ln->rendered[j], query, ln->rendered_len-j, query_len)) != NULL){
            // store occurrence
            j = (match - ln->rendered);
            occurrences[line_i][occ_i++] = j;
            j += query_len;
        }
        occurrences[line_i][max_occ] = occ_i-1;
    }
    free(lps);

    return occurrences;

/*

    for(unsigned long line_i = 0; line_i < editor.txt->lines_num; line_i++){
        free(occurrences[line_i]);
    }
    free(occurrences);

    unsigned long line_i;
    for (line_i = 0; line_i < editor.txt->lines_num; line_i++){
        line *ln = &(editor.txt->lines[line_i]);
        // Find all occurences in line_i
        char *match;
        unsigned long j = 0;
        while(j < ln->rendered_len && (match = KMP_search(lps, &ln->rendered[j], query, ln->rendered_len-j, *query_len)) != NULL){
            editor.cursor_y = line_i;
            j = (match - ln->rendered);
            editor.cursor_x = rx_to_cx(ln, j);
            j += (*query_len);
            editor.row_offset = editor.txt->lines_num; // makes matching line be at the top of the screen
        
            // read keypress
        }
    }

*/
}
