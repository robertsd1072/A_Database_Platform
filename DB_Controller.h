#ifndef DB_CONTROLLER_H_
#define DB_CONTROLLER_H_

#include "DB_HelperFunctions.h"

typedef unsigned long long int_8;


int parseInput(char* input, struct malloced_node** malloced_head, int the_debug);

int selectAndPrint(char* input, struct malloced_node** malloced_head, int the_debug);

int insertAndPrint(char* input, struct malloced_node** malloced_head, int the_debug);

int updateAndPrint(char* input, struct malloced_node** malloced_head, int the_debug);

int deleteAndPrint(char* input, struct malloced_node** malloced_head, int the_debug);

int addInputToLog(char* input, struct malloced_node** malloced_head, int the_debug);

int copyToSHMfromDB(struct malloced_node** malloced_head, int the_debug);

int printSHM(struct shmem* shm);


int createTableFromCSV(char* input, char* table_name, int_8 num_rows, char* keyword, struct malloced_node** malloced_head, int the_debug);

int displayResultsOfSelect(struct select_node* select_node, struct malloced_node** malloced_head, int the_debug);

int getColInSelectNodeFromName(void* put_ptrs_here, int put_ptrs_here_type, int math_node_which, int i
							  ,struct select_node* select_node, char* cur_col_alias, char* cur_col_name
							  ,struct malloced_node** malloced_head, int the_debug);

int parseWhereClause(char* input, struct where_clause_node** where_head, struct select_node* select_node, struct table_info* table, char* first_word
				    ,struct malloced_node** malloced_head, int the_debug);

struct table_info* getTableFromName(char* input_table_name
								   ,struct malloced_node** malloced_head, int the_debug);

int parseUpdate(char* input, struct change_node_v2** change_head, struct malloced_node** malloced_head, int the_debug);

int parseDelete(char* input, struct change_node_v2** change_head, struct malloced_node** malloced_head, int the_debug);

int parseInsert(char* input, struct change_node_v2** change_head, struct malloced_node** malloced_head, int the_debug);

int parseMathNode(struct math_node** the_math_node, char** new_col_name, char** col_name, char* input, char* word, int index, struct select_node* select_node
				 ,bool math_in_having, struct malloced_node** malloced_head, int the_debug);

int parseFuncNode(struct func_node** the_func_node, char** new_col_name, char** col_name, char* input, char* word, int* index, struct select_node* select_node
				 ,bool rec, struct malloced_node** malloced_head, int the_debug);

int parseCaseNode(struct case_node** col_case_node, char* input, char* word, int index, char** new_col_name, struct select_node* select_node, int* return_ptr_type
				 ,struct malloced_node** malloced_head, int the_debug);

int parseSelect(char* input, struct select_node** select_node, struct ListNodePtr* with_sub_select_list, bool in_with
			   ,struct malloced_node** malloced_head, int the_debug);

#endif