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
* Returns first occurence of pattern in text (case sensitive)
*/
char* KMP_search_cs(int *lps, char *text, char *pattern, unsigned long text_len, unsigned long pattern_len) {

    unsigned long i = 0; // Index for text[]
    unsigned long j = 0; // Index for pattern[]

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

/*
* Returns first occurence of pattern in text (NON case sensitive)
*/
char* KMP_search_non_cs(int *lps, char *text, char *pattern, unsigned long text_len, unsigned long pattern_len) {

    unsigned long i = 0; // Index for text[]
    unsigned long j = 0; // Index for pattern[]

    while (i < text_len) {
        if (tolower(pattern[j]) == tolower(text[i])) {
            j++;
            i++;
        }

        if (j == pattern_len) {
            return &text[i-j];
            //printf("Pattern found at index %d\n", i - j);
            //j = lps[j - 1];
        } else if (i < text_len && tolower(pattern[j]) != tolower(text[i])) {
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

unsigned long** get_occurrences(text* txt, char *query, unsigned long query_len, char case_sensitive){
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
    
    char found_any = 0; // flag

    for (unsigned long line_i = 0; line_i < txt->lines_num; line_i++){
        // Find all occurences in line_i
        line *ln = &(txt->lines[line_i]);
        unsigned long max_occ = ln->raw_len/query_len;
        occurrences[line_i] = (unsigned long*)malloc(sizeof(unsigned long)*(max_occ+1)); //+1 to hold last written index of occurences[line_i]. 
        if(occurrences[line_i] == NULL){
            return NULL;
        }
        unsigned long occ_i = 0; // index of where occ will be written in occurrences[line_i].
        char *match;
        unsigned long j = 0;
        while(j < ln->raw_len && (match = case_sensitive ? KMP_search_cs(lps, &ln->raw[j], query, ln->raw_len-j, query_len) : KMP_search_non_cs(lps, &ln->raw[j], query, ln->raw_len-j, query_len)) != NULL){
            // store occurrence
            j = (match - ln->raw);
            occurrences[line_i][occ_i++] = j;
            j += query_len;
        }

        if (occ_i == 0){
            occurrences[line_i][max_occ] = ULONG_MAX;
        } else{
            occurrences[line_i][max_occ] = occ_i-1;
            found_any = 1;
        }
    }
    free(lps);

    if (!found_any){
        free_occurrences(occurrences, txt->lines_num);
        return NULL;
    }

    return occurrences;
}
