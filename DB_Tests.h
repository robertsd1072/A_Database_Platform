#ifndef DB_TESTS_H_
#define DB_TESTS_H_

typedef unsigned long long int_8;

#include "DB_HelperFunctions.h"
#include "DB_Driver.h"


int test_Driver_setup(struct malloced_node** malloced_head, int the_debug);

int test_Driver_teardown(int the_debug);

int selectAndCheckHash(char* test_version, int test_id, struct table_info* table, struct malloced_node** malloced_head, int the_debug);


int test_Helper_DateFunctions_1(char* date, struct malloced_node** malloced_head, int the_debug);

int test_Helper_DateFunctions_2(int_8 date, struct malloced_node** malloced_head, int the_debug);


int compMathOrWhereTree(int test_id, int tree_ptr_type, void* actual_ptr, void* expected_ptr);


int test_Controller_parseWhereClause(int test_id, char* where_string, char* first_word
									,int* error_code, struct select_node* the_select_node, struct where_clause_node** expected_where_head
									,struct malloced_node** malloced_head, int the_debug);

int test_Controller_parseUpdate(int test_id, char* update_string, struct change_node_v2** expected_change_head, int* parsed_error_code
							   ,struct malloced_node** malloced_head, int the_debug);

int test_Controller_parseDelete(int test_id, char* delete_string, struct change_node_v2** expected_change_head, int* parsed_error_code
							   ,struct malloced_node** malloced_head, int the_debug);

int test_Controller_parseInsert(int test_id, char* insert_string, struct change_node_v2** expected_change_head, int* parsed_error_code
							   ,struct malloced_node** malloced_head, int the_debug);

int test_Controller_parseSelect(int test_id, char* select_string, struct select_node** the_select_node
							   ,int* parsed_error_code, struct malloced_node** malloced_head, int the_debug);


int test_Driver_findValidRowsGivenWhere(int test_id, char* where_string, struct select_node* the_select_node, struct table_info* the_table
									   ,struct ListNodePtr* expected_results, struct malloced_node** malloced_head, int the_debug);

int test_Driver_selectStuff(int test_id, char* select_string, char* expected_results_csv, struct malloced_node** malloced_head, int the_debug);

int test_Driver_updateRows(int test_id, char* expected_results_csv, char* input_string
						  ,struct malloced_node** malloced_head, int the_debug);

int test_Driver_deleteRows(int test_id, char* input_string, char* expected_results_csv, int_8 expected_num_rows, int_8 expected_num_open
						  ,struct malloced_node** malloced_head, int the_debug);

int test_Driver_insertRows(int test_id, char* input_string, char* expected_results_csv, int_8 expected_num_rows, int_8 expected_num_open
						  ,struct malloced_node** malloced_head, int the_debug);

int test_Performance_Select(int test_id, char* select_string, struct malloced_node** malloced_head, int the_debug);


int test_Driver_main();

#endif