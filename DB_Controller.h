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

int getColInSelectNodeFromName(void* put_ptrs_here, int put_ptrs_here_type, int math_node_which, int i
							  ,struct select_node* select_node, char* cur_col_alias, char* cur_col_name
							  ,struct malloced_node** malloced_head, int the_debug);

int parseWhereClause(char* input, struct where_clause_node** where_head, struct select_node* select_node, char* first_word
				    ,struct malloced_node** malloced_head, int the_debug);

struct table_info* getTableFromName(char* input_table_name
								   ,struct malloced_node** malloced_head, int the_debug);

/*int parseUpdate(char* input, struct table_info** table, struct change_node_v2** change_head, struct or_clause_node** or_head
			   ,struct malloced_node** malloced_head, int the_debug);

int parseDelete(char* input, struct or_clause_node** or_head, struct table_info** table
			   ,struct malloced_node** malloced_head, int the_debug);

int parseInsert(char* input, struct change_node_v2** change_head, struct table_info** table
			   ,struct malloced_node** malloced_head, int the_debug);*/

int parseMathNode(struct math_node** the_math_node, char** new_col_name, char** col_name, char* input, char* word, int index, struct select_node* select_node
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