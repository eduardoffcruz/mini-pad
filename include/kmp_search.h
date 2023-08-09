#include <stdlib.h>
#include <data_structures.h>
/*
*
*/
void compute_LPS_array(char *pattern, unsigned long pattern_len, int *lps);

/*
* Returns first occurence of pattern in text 
*/
char* KMP_search(int *lps, char *text, char *pattern, unsigned long text_len, unsigned long pattern_len);

/*
*
*/
unsigned long** get_occurrences(text* txt, char *query, unsigned long query_len);

/*
*
*/
void free_occurrences(unsigned long** occs, unsigned long lines_num);