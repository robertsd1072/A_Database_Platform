#ifndef DB_DRIVER_H_
#define DB_DRIVER_H_

typedef unsigned long long int_8;

#include "DB_HelperFunctions.h"


struct table_info* getTablesHead();


int createNextTableNumFile(struct malloced_node** malloced_head, int the_debug);

int_8 getNextTableNum(struct malloced_node** malloced_head, int the_debug);


int initDB(struct malloced_node** malloced_head, int the_debug);

int traverseTablesInfoMemory();

int createTable(char* table_name, struct table_cols_info* table_cols, struct malloced_node** malloced_head, int the_debug);

int addColumn(FILE* tab_col_append, struct table_cols_info* cur_col, struct table_info* table, struct malloced_node** malloced_head, int the_debug);

int insertAppend(FILE** col_data_info_file_arr, FILE** col_data_file_arr, struct file_opened_node** file_opened_head
				,int_8 the_table_number, struct table_cols_info* the_col
				,int_8 the_data_int_date, double the_data_real, char* the_data_string
				,struct malloced_node** malloced_head, int the_debug);

int insertOpen(FILE** col_data_info_file_arr, FILE** col_data_file_arr, struct file_opened_node** file_opened_head
			  ,int_8 the_table_number, struct table_cols_info* the_col
			  ,int_8 the_data_int_date, double the_data_real, char* the_data_string
			  ,struct malloced_node** malloced_head, int the_debug);

int insertRows(struct table_info* the_table, struct change_node_v2* change_head, struct malloced_node** malloced_head, int the_debug);

int deleteRows(struct table_info* the_table, struct where_clause_node* or_head, struct malloced_node** malloced_head, int the_debug);

int updateRows(struct table_info* the_table, struct change_node_v2* change_head, struct where_clause_node* or_head, struct malloced_node** malloced_head, int the_debug);

struct colDataNode** getAllColData(int_8 table_number, struct table_cols_info* the_col, struct ListNodePtr* valid_rows_head, int num_rows_in_result
								  ,struct malloced_node** malloced_head, int the_debug);

int findValidRowsGivenWhere(struct ListNodePtr** valid_rows_head, struct ListNodePtr** valid_rows_tail
						   ,struct table_info* the_table, struct colDataNode*** table_data_arr, struct where_clause_node* or_head
						   ,int_8* the_col_numbers, int_8* num_rows_in_result, int the_col_numbers_size, struct malloced_node** malloced_head, int the_debug);

struct colDataNode*** select(struct table_info* the_table, int_8* the_col_numbers, int the_col_numbers_size, int_8* num_rows_in_result, struct where_clause_node* or_head
							,struct malloced_node** malloced_head, int the_debug);


int traverseTablesInfoDisk(struct malloced_node** malloced_head, int the_debug);

int initFrequentLists(struct table_info* the_table
					 ,struct malloced_node** malloced_head, int the_debug);

int removeFreqNodeFromFreqLists(int row_id_to_remove, struct table_cols_info* cur_col, int the_debug);

int addFreqNodeToTempNewList(int add_mode, struct frequent_node** freq_head, struct frequent_node** freq_tail, char* new_data, int new_row_id, struct table_cols_info* cur_col
							,struct file_opened_node** file_opened_head, struct malloced_node** malloced_head, int the_debug);

int addFreqNodeToFreqLists(struct frequent_node* cur_freq, struct table_cols_info* cur_col
						  ,struct file_opened_node** file_opened_head, struct malloced_node** malloced_head, int the_debug);


int freeMemOfDB(int the_debug);

#endif