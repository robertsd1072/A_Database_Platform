#ifndef DB_CONTROLLER_H_
#define DB_CONTROLLER_H_

#include "DB_HelperFunctions.h"
#include "DB_Driver.h"

typedef unsigned long long int_8;


int createTableFromCSV(char* input, char* table_name, int_8 num_rows
					  ,struct malloced_node** malloced_head, int the_debug);

int displayResultsOfSelect(char*** result, struct table_info* the_table, int_8* col_numbers, int col_numbers_size, int_8 num_rows
						  ,struct malloced_node** malloced_head, int the_debug);

int parseInput(char* input
			  ,struct malloced_node** malloced_head, int the_debug);

struct or_clause_node* parseWhereClause(char* input, struct table_info* the_table
									   ,struct malloced_node** malloced_head, int the_debug);

int parseUpdate(char* input, struct change_node_v2** change_head, struct or_clause_node** or_head
			   ,struct malloced_node** malloced_head, int the_debug);

#endif