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

int test_Driver_deleteRows();

int test_Driver_insertRows();


int test_Driver_main(int the_debug);

#endif