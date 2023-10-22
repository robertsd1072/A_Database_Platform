#ifndef DB_TESTS_H_
#define DB_TESTS_H_

typedef unsigned long long int_8;

#include "DB_HelperFunctions.h"
#include "DB_Driver.h"


int test_Driver_setup(struct malloced_node** malloced_head, int the_debug);

int test_Driver_teardown(int the_debug);

int selectAndCheckHash(char* test_version, int test_id, struct malloced_node** malloced_head, int the_debug);


int test_Helper_DateFunctions_1(char* date, struct malloced_node** malloced_head, int the_debug);

int test_Helper_DateFunctions_2(int_8 date, struct malloced_node** malloced_head, int the_debug);


int test_Driver_findValidRowsGivenWhere(int test_id, struct ListNode* expected_results, char* where_string
									   ,struct table_info* the_table, struct colDataNode*** table_data_arr
									   ,int_8* the_col_numbers, int_8* num_rows_in_result, int the_col_numbers_size
									   ,struct malloced_node** malloced_head, int the_debug);

int test_Driver_updateRows(int test_id, char* expected_results_csv, char* input_string
						  ,struct table_info* the_table, struct malloced_node** malloced_head, int the_debug);

int test_Driver_deleteRows(int test_id, char* input_string, char* expected_results_csv, int_8 expected_num_rows, int_8 expected_num_open
						  ,struct malloced_node** malloced_head, int the_debug);

int test_Driver_insertRows(int test_id, char* input_string, char* expected_results_csv, int_8 expected_num_rows, int_8 expected_num_open
						  ,struct malloced_node** malloced_head, int the_debug);


int test_Controller_parseWhereClause(int test_id, char* where_string, int expected_error_code, struct table_info* the_table
									,struct malloced_node** malloced_head, int the_debug);

int test_Controller_parseUpdate(int test_id, char* update_string, struct change_node_v2** expected_change_head, int* parsed_error_code
							   ,struct malloced_node** malloced_head, int the_debug);

int test_Controller_parseDelete(int test_id, char* delete_string, char* expected_table_name
							   ,struct malloced_node** malloced_head, int the_debug);

int test_Controller_parseInsert(int test_id, char* insert_string, struct change_node_v2** expected_change_head, char* expected_table_name
							   ,int* parsed_error_code, struct malloced_node** malloced_head, int the_debug);

int test_Controller_parseSelect(int test_id, char* select_string, int_8** expected_col_numbers_arr, int expected_col_numbers_arr_size
							   ,char* expected_table_name, int* parsed_error_code
							   ,struct malloced_node** malloced_head, int the_debug);

int test_Performance_Select(int test_id, char* select_string, struct malloced_node** malloced_head, int the_debug);

int test_Driver_main();

#endif