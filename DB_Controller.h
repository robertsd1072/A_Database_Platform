#ifndef DB_CONTROLLER_H_
#define DB_CONTROLLER_H_

typedef unsigned long long int_8;

#include "DB_HelperFunctions.h"
#include "DB_Driver.h"


int createTableFromCSV(char* input, char* table_name, int_8 num_rows
					  ,struct malloced_node** malloced_head, int the_debug);

int displayResultsOfSelect(struct colDataNode*** result, struct table_info* the_table, int_8* col_numbers, int col_numbers_size, int_8 num_rows
						  ,struct malloced_node** malloced_head, int the_debug);

int printLastWordBeforeError(char* error_word);

int parseInput(char* input
			  ,struct malloced_node** malloced_head, int the_debug);

struct or_clause_node* parseWhereClause(char* input, struct select_node** select_head, int* error_code, char* first_word
									   ,struct malloced_node** malloced_head, int the_debug);

struct table_info* getTableFromName(char* input_table_name
								   ,struct malloced_node** malloced_head, int the_debug);

int parseUpdate(char* input, struct table_info** table, struct change_node_v2** change_head, struct or_clause_node** or_head
			   ,struct malloced_node** malloced_head, int the_debug);

int parseDelete(char* input, struct or_clause_node** or_head, struct table_info** table
			   ,struct malloced_node** malloced_head, int the_debug);

int parseInsert(char* input, struct change_node_v2** change_head, struct table_info** table
			   ,struct malloced_node** malloced_head, int the_debug);

int parseSelect(char* input, struct select_node** select_node
			   ,struct malloced_node** malloced_head, int the_debug);

int selectAndPrint(char* input
				  ,struct malloced_node** malloced_head, int the_debug);

int insertAndPrint(char* input
				  ,struct malloced_node** malloced_head, int the_debug);

int updateAndPrint(char* input
				  ,struct malloced_node** malloced_head, int the_debug);

int deleteAndPrint(char* input
				  ,struct malloced_node** malloced_head, int the_debug);

#endif