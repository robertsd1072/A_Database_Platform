#ifndef DB_CONTROLLER_H_
#define DB_CONTROLLER_H_

#include "DB_HelperFunctions.h"
#include "DB_Driver.h"

typedef unsigned long long int_8;

int strLength(char* str);

int strcontains(char* str, char the_char);

int indexOf(char* str, char the_char);

char* substring(struct malloced_node** malloced_head, char* str, int start, int end, int persists);

char** strSplit(struct malloced_node** malloced_head, char* str, char the_char, int* size_result, int persists);

int callCreateTable(char* table_name, struct table_cols_info* table_cols, int the_debug);

int createTableFromCSV(char* input, char* table_name, int_8 num_rows, int the_debug);

int freeResultsOfSelectIfError(char*** result, int_8* col_numbers, int col_numbers_size, int_8 num_rows, int the_debug);

int displayResultsOfSelectAndFree(char*** result, struct table_info* the_table, int_8* col_numbers, int col_numbers_size, int_8 num_rows, struct or_clause_node* or_head, int the_debug);

#endif