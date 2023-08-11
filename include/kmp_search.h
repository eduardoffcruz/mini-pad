#include <stdlib.h>
#include <limits.h>
#include <ctype.h>

#include "data_structures.h"
/*
*
*/
void compute_LPS_array(char *pattern, unsigned long pattern_len, int *lps);

/*
* Returns first occurence of pattern in text (case sensitive)
*/
char* KMP_search_cs(int *lps, char *text, char *pattern, unsigned long text_len, unsigned long pattern_len);

/*
* Returns first occurence of pattern in text (NON case sensitive)
*/
char* KMP_search_non_cs(int *lps, char *text, char *pattern, unsigned long text_len, unsigned long pattern_len);


/*
*
*/
unsigned long** get_occurrences(text* txt, char *query, unsigned long query_len, char case_sensitive);

/*
*
*/
void free_occurrences(unsigned long** occs, unsigned long lines_num);