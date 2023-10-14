#ifndef DB_DRIVER_H_
#define DB_DRIVER_H_

#define DATA_INT 1
#define DATA_REAL 2
#define DATA_STRING 3
#define DATA_DATE 4

#define YES_PERSISTS 1
#define NOT_PERSISTS 2

#define WHERE_IS_EQUALS 1
#define WHERE_NOT_EQUALS 2

#define OP_INSERT 1
#define OP_DELETE 2
#define OP_UPDATE 3

typedef unsigned long long int_8;

#include "DB_HelperFunctions.h"

/*	DB_Info File Structure
	8 bytes for the number of tables
	List of tables:
		32 bytes for the table name
		8 bytes for the table number (file number)

	_TabCol_ File Structure (in form "DB_Tab_Col_[table number].bin")
	8 bytes for the number of columns
	List of columns:
		32 bytes for the column name
		8 bytes for the datatype (and max length is applicable)
			datatype is gotten with the bytes % 10
			max length is gotten with the bytes / 100;
		8 bytes for the column_number

	_Col_Data_Info_ File Structure (in form "DB_Col_Data_Info_[table number]_[column number].bin")
	8 bytes for the number of rows
	8 bytes for the number of open slots
	List of open slots
		8 bytes for the row_id

	_Col_Data_ File Structure (in form "DB_Col_Data_[table number]_[column number].bin")
	List of rows:
		8 bytes for the row_id
		[max_length] bytes for the row data
*/

/*	Datatypes
	1 = Integer
	2 = Real number
	3 = String     (Also a max value must be specified)
	4 = Date
*/
struct table_cols_info
{
	char* col_name;	//	Max 31 bytes + 1 for \0
	int_8 data_type;
	int_8 max_length;
	int_8 col_number;
	int_8 num_rows;
	int_8 num_open;
	struct ListNode* open_list_head;
	struct ListNode* open_list_before_tail;
	int_8 num_added_insert_open;
	struct frequent_node* unique_list_head;
	struct frequent_node* unique_list_tail;
	struct frequent_node* frequent_list_head;
	struct frequent_node* frequent_list_tail;
	struct table_cols_info* next;
};

struct table_info
{
	char* name;	//	Max 31 bytes + 1 for 0
	int_8 file_number;
	int_8 num_cols;
	struct table_cols_info* table_cols_head;
	struct change_node* change_list_head;
	struct change_node* change_list_tail;
	struct table_info* next;
};

struct change_node
{
	int_8 transac_id;
	int_8 table_number;
	int_8 col_number;
	int_8 operation;
	/*	Operation Codes
		1 = Insert open
		2 = Insert append
		3 = Delete with no update
		4 = Delete before the update
		5 = Insert open after deletion
	*/
	int_8 data_type;
	/*	Datatypes
		1 = Integer
		2 = Real number
		3 = String
		4 = Date
	*/
	int_8 data_int_date;
	double data_real;
	char* data_string;
	struct change_node* next;
};

struct change_node_v2
{
	int_8 transac_id;
	int_8 col_number;
	int_8 operation;
	/*	Operation Codes
		1 = Insert
		2 = Delete
		3 = Update
	*/
	int_8 data_type;
	void* data;
	struct change_node_v2* next;
};

struct or_clause_node
{
	struct and_clause_node* and_head;
	struct or_clause_node* next;
};

struct and_clause_node
{
	int_8 col_number;
	int_8 where_type;
	/*	Where types
		1 = "="
		2 = "<>"
	*/
	char* data_string;
	struct and_clause_node* next;
};

struct colDataNode
{
	int_8 row_id;
	char* row_data;
};

struct frequent_node
{
	void* ptr_value;
	int_8 num_appearences;
	struct ListNode* row_nums_head;
	struct ListNode* row_nums_tail;
	struct frequent_node* next;
};


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

int deleteRows(struct table_info* the_table, struct or_clause_node* or_head, struct malloced_node** malloced_head, int the_debug);

int updateRows(struct table_info* the_table, struct change_node_v2* change_head, struct or_clause_node* or_head, struct malloced_node** malloced_head, int the_debug);

struct colDataNode** getAllColData(int_8 table_number, struct table_cols_info* the_col, struct ListNode* valid_rows_head, int num_rows_in_result
								  ,struct malloced_node** malloced_head, int the_debug);

struct ListNode* findValidRowsGivenWhere(struct table_info* the_table, struct colDataNode*** table_data_arr, struct or_clause_node* or_head
										,int_8* the_col_numbers, int_8* num_rows_in_result, int the_col_numbers_size, struct malloced_node** malloced_head, int the_debug);

struct colDataNode*** select(struct table_info* the_table, int_8* the_col_numbers, int the_col_numbers_size, int_8* num_rows_in_result, struct or_clause_node* or_head
							,struct malloced_node** malloced_head, int the_debug);


int traverseTablesInfoDisk(struct malloced_node** malloced_head, int the_debug);

int initFrequentLists(struct table_info* the_table
					 ,struct malloced_node** malloced_head, int the_debug);


int freeMemOfDB(int the_debug);

#endif