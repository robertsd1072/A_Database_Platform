#ifndef DB_DRIVER_H_
#define DB_DRIVER_H_

typedef unsigned long long int_8;

#include "DB_HelperFunctions.h"
#include "DB_SHM_Struct.h"


struct table_info* getTablesHead();


int createNextTableNumFile(struct malloced_node** malloced_head, int the_debug);

int_8 getNextTableNum(struct malloced_node** malloced_head, int the_debug);

int initDB(struct malloced_node** malloced_head, int the_debug);

int copyDBfromSHM(struct shmem* shm, struct malloced_node** malloced_head, int the_debug);

int traverseTablesInfoMemory();


int createTable(char* table_name, struct table_cols_info* table_cols, char* keyword, struct malloced_node** malloced_head, int the_debug);

int addColumn(FILE* tab_col_append, struct table_cols_info* cur_col, struct table_info* table, struct malloced_node** malloced_head, int the_debug);

int deleteTable(char* table_name, struct malloced_node** malloced_head, int the_debug);

int insertAppend(FILE** col_data_info_file_arr, FILE** col_data_file_arr, struct file_opened_node** file_opened_head
				,int_8 the_table_number, struct table_cols_info* the_col, void* data_ptr_value, struct malloced_node** malloced_head, int the_debug);

int insertRows(struct change_node_v2* change_head, struct malloced_node** malloced_head, int the_debug);

int deleteRows(struct change_node_v2* change_head, struct malloced_node** malloced_head, int the_debug);

int updateRows(struct change_node_v2* change_head, struct malloced_node** malloced_head, int the_debug);

struct colDataNode** getAllColData(int_8 table_number, struct table_cols_info* the_col, struct ListNodePtr* valid_rows_head, int num_rows_in_result
								  ,struct malloced_node** malloced_head, int the_debug);

int calcResultOfCaseForOneRow(struct case_node* the_case_node, int_8 row_id, int_8 second_row_id, struct select_node* the_select_node, struct table_info* the_table, bool join
							 ,int_8 left_num_rows, int_8 right_num_rows, struct ListNodePtr* head, struct ListNodePtr* tail
							 ,struct malloced_node** malloced_head, int the_debug);

struct result_node* calcResultOfCaseForOneRowV2(struct case_node* the_case_node, int_8 row_id, int_8 second_row_id, struct select_node* the_select_node, struct table_info* the_table
											 ,bool join, int_8 left_num_rows, int_8 right_num_rows, struct ListNodePtr* head, struct ListNodePtr* tail
											 ,struct malloced_node** malloced_head, int the_debug);

int evaluateMathTree(struct ListNodePtr* head, struct ListNodePtr* tail, struct math_node* cur, int_8 row_id, int_8 second_row_id
					,struct malloced_node** malloced_head, int the_debug);

struct result_node* evaluateMathTreeV2(struct ListNodePtr* head, struct ListNodePtr* tail, struct math_node* cur, int_8 row_id, int_8 second_row_id
											,struct malloced_node** malloced_head, int the_debug);

int findValidRowsGivenWhere(struct ListNodePtr** valid_rows_head, struct ListNodePtr** valid_rows_tail
						   ,struct select_node* the_select_node, struct table_info* the_table, struct where_clause_node* where_head
						   ,int_8* num_rows_in_result, struct ListNodePtr* groups_head, struct ListNodePtr* groups_tail, bool join
						   ,int_8 left_num_rows, int_8 right_num_rows
						   ,struct malloced_node** malloced_head, int the_debug);

int selectStuff(struct select_node** select_node, bool join, struct malloced_node** malloced_head, int the_debug);


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