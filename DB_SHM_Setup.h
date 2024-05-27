#ifndef DB_SHM_SETUP_H_
#define DB_SHM_SETUP_H_

typedef unsigned long long int_8;

#include "DB_HelperFunctions.h"


int initDB(struct malloced_node** malloced_head, int the_debug);

int traverseTablesInfoMemory();

struct colDataNode** getAllColData(int_8 table_number, struct table_cols_info* the_col, struct ListNodePtr* valid_rows_head, int num_rows_in_result
								  ,struct malloced_node** malloced_head, int the_debug);

int traverseTablesInfoDisk(struct malloced_node** malloced_head, int the_debug);

int initFrequentLists(struct table_info* the_table
					 ,struct malloced_node** malloced_head, int the_debug);

int removeFreqNodeFromFreqLists(int* row_id_to_remove, struct table_cols_info* cur_col, int the_debug);

int addFreqNodeToTempNewList(int add_mode, struct frequent_node** freq_head, struct frequent_node** freq_tail, struct ListNodePtr* new_data, int* new_row_id, struct table_cols_info* cur_col
							,struct file_opened_node** file_opened_head, struct malloced_node** malloced_head, int the_debug);

int addFreqNodeToFreqLists(struct frequent_node* cur_freq, struct table_cols_info* cur_col
						  ,struct file_opened_node** file_opened_head, struct malloced_node** malloced_head, int the_debug);

int freeMemOfDB(int the_debug);

#endif