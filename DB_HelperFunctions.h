#ifndef DB_HELPERFUNCTIONS_H_
#define DB_HELPERFUNCTIONS_H_

typedef unsigned long long int_8;

#define YES_DEBUG 1
#define NO_DEBUG 0

#define ADDLISTNODE_HEAD 1
#define ADDLISTNODE_TAIL 2
#define REMOVELISTNODE_HEAD 1
#define REMOVELISTNODE_TAIL 2
#define TRAVERSELISTNODES_HEAD 1
#define TRAVERSELISTNODES_TAIL 2

#define APPEND_OFFSET -1

#define YES_TRAVERSE_DISK 1
#define NO_TRAVERSE_DISK 0

#define DATA_INT 1
#define DATA_REAL 2
#define DATA_STRING 3
#define DATA_DATE 4

#define YES_PERSISTS 1
#define NOT_PERSISTS 2

#define WHERE_IS_EQUALS 1
#define WHERE_NOT_EQUALS 2
#define WHERE_GREATER_THAN 3
#define WHERE_GREATER_THAN_OR_EQUAL 4
#define WHERE_LESS_THAN 5
#define WHERE_LESS_THAN_OR_EQUAL 6
#define WHERE_IS_NULL 7
#define WHERE_IS_NOT_NULL 8

#define WHERE_OR 1
#define WHERE_AND 2

#define OP_INSERT 1
#define OP_DELETE 2
#define OP_UPDATE 3

#define FREQ_LIST_ADD_UPDATE 0
#define FREQ_LIST_ADD_INSERT 1

#define JOIN_INNER 1
#define JOIN_OUTER 2
#define JOIN_LEFT 3
#define JOIN_RIGHT 4
#define JOIN_CROSS 5

#define PTR_TYPE_INT 1
#define PTR_TYPE_REAL 2
#define PTR_TYPE_CHAR 3
#define PTR_TYPE_DATE 4
#define PTR_TYPE_TABLE_COLS_INFO 5
#define PTR_TYPE_TABLE_INFO 6
#define PTR_TYPE_CHANGE_NODE_V2 7
#define PTR_TYPE_WHERE_CLAUSE_NODE 8
#define PTR_TYPE_COL_DATA_NODE 9
#define PTR_TYPE_FREQUENT_NODE 10
#define PTR_TYPE_SELECT_NODE 11
#define PTR_TYPE_COL_IN_SELECT_NODE 12
#define PTR_TYPE_FUNC_NODE 13
#define PTR_TYPE_JOIN_NODE 14
#define PTR_TYPE_MATH_NODE 15
#define PTR_TYPE_LIST_NODE_PTR 16
#define PTR_TYPE_SELECT_NODE_BUT_IN_JOIN 17

#define PTR_EQUALS 1
#define VALUE_EQUALS 2

#define FUNC_AVG 1
#define FUNC_COUNT 2
#define FUNC_FIRST 3
#define FUNC_LAST 4
#define FUNC_MIN 5
#define FUNC_MAX 6
#define FUNC_MEDIAN 7
#define FUNC_SUM 8
#define FUNC_RANK 9

#define MATH_ADD 1
#define MATH_SUB 2
#define MATH_MULT 3
#define MATH_DIV 4
#define MATH_POW 5

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

struct table_cols_info
{
	char* col_name;	//	Max 31 bytes + 1 for \0
	int_8 data_type;
	int_8 max_length;
	int_8 col_number;
	int_8 num_rows;
	int_8 num_open;
	struct ListNodePtr* open_list_head;
	struct ListNodePtr* open_list_before_tail;
	int_8 num_added_insert_open;
	struct frequent_node* unique_list_head;
	struct frequent_node* unique_list_tail;
	struct frequent_node* frequent_list_head;
	struct frequent_node* frequent_list_tail;
	struct frequent_node** frequent_arr_row_to_node;
	struct table_cols_info* next;

	struct table_info* home_table;
};

struct table_info
{
	char* name;	//	Max 31 bytes + 1 for 0
	int_8 file_number;
	int_8 num_cols;
	struct table_cols_info* table_cols_head;
	struct table_info* next;
};

struct change_node_v2
{
	int_8 transac_id;

	struct table_cols_info* col;

	int_8 operation;

	void* data_ptr;
	int data_ptr_type;

	struct change_node_v2* next;
};

struct where_clause_node
{
	void* ptr_one;
	int ptr_one_type;

	void* ptr_two;
	int ptr_two_type;

	int where_type;

	struct where_clause_node* parent;
};

struct colDataNode
{
	int_8 row_id;
	void* row_data;
};

struct frequent_node
{
	void* ptr_value;
	int_8 num_appearences;
	struct ListNodePtr* row_nums_head;
	struct ListNodePtr* row_nums_tail;
	struct frequent_node* prev;
	struct frequent_node* next;
};

struct select_node
{
	char* select_node_alias;

	bool distinct;

	int_8 columns_arr_size;
	struct col_in_select_node** columns_arr;

	struct where_clause_node* where_head;

	struct join_node* join_head;

	struct select_node* prev;
	struct select_node* next;

	// Maybe
	struct ListNodePtr* valid_rows_head;
	int_8 num_rows;

	struct colDataNode*** data_arr;
};

struct col_in_select_node
{
	void* table_ptr;
	int table_ptr_type;

	void* col_ptr;
	int col_ptr_type;

	char* new_name;

	struct ListNodePtr* case_when_head;
	struct ListNodePtr* case_then_value_head;

	struct ListNodePtr* case_when_tail;
	struct ListNodePtr* case_then_value_tail;

	struct func_node* func_node;
	struct math_node* math_node;
};

struct func_node
{
	// Functions:
	// avg, count, first, last, max, median, min, sum, rank

	int which_func;
	bool distinct;

	int args_size;
	void** args_arr;
	int* args_arr_type;

	struct ListNodePtr* group_by_cols_head;
	struct ListNodePtr* group_by_cols_tail;

	void* result;
	int result_type;
};

struct join_node
{
	int join_type;

	struct select_node* select_joined;

	struct where_clause_node* on_clause_head;

	struct join_node* prev;
	struct join_node* next;
};

struct math_node
{
	void* ptr_one;
	int ptr_one_type;

	void* ptr_two;
	int ptr_two_type;

	int operation;

	struct math_node* parent;

	void* result;
	int result_type;
};

struct malloced_node
{
	void* ptr;
	struct malloced_node* prev;
	struct malloced_node* next;
};

struct file_opened_node
{
	FILE* file;
	struct file_opened_node* next;
};

struct ListNodePtr
{
	void* ptr_value;
	int ptr_type;

	struct ListNodePtr* prev;
	struct ListNodePtr* next;
};

int strLength(char* str);

int strcontains(char* str, char the_char);

int indexOf(char* str, char the_char);

char* substring(char* str, int start, int end
			   ,struct file_opened_node** file_opened_head, struct malloced_node** malloced_head, int the_debug);

char** strSplitV2(char* str, char the_char, int* size_result
				 ,struct file_opened_node** file_opened_head, struct malloced_node** malloced_head, int the_debug);

char* upper(char* str
		   ,struct file_opened_node** file_opened_head, struct malloced_node** malloced_head, int the_debug);

int strIsNotEmpty(char* str);

int getNextWord(char* input, char* word, int* cur_index);

int strcmp_Upper(char* word, char* test_char
				,struct file_opened_node** file_opened_head, struct malloced_node** malloced_head, int the_debug);


void* myMalloc(size_t size
			  ,struct file_opened_node** file_opened_head, struct malloced_node** malloced_head, int the_debug);

int myFree(void** old_ptr
		  ,struct file_opened_node** file_opened_head, struct malloced_node** malloced_head, int the_debug);

int myFreeAllError(struct malloced_node** malloced_head, int the_debug);

int myFreeAllCleanup(struct malloced_node** malloced_head, int the_debug);

int myFreeJustNode(void** old_ptr
				  ,struct file_opened_node** file_opened_head, struct malloced_node** malloced_head, int the_debug);


int concatFileName(char* new_filename, char* filetype, int_8 table_num, int_8 col_num);


FILE* myFileOpenSimple(char* file_name, char* mode
					  ,struct file_opened_node** file_opened_head, struct malloced_node** malloced_head, int the_debug);

FILE* myFileOpen(char* filetype, int_8 num_table, int_8 num_col, char* mode
				,struct file_opened_node** file_opened_head, struct malloced_node** malloced_head, int the_debug);

int myFileClose(FILE* old_file
			   ,struct file_opened_node** file_opened_head, struct malloced_node** malloced_head, int the_debug);

int myFileCloseAll(struct file_opened_node** file_opened_head, struct malloced_node** malloced_head
				  ,int the_debug);


char* intToDate(int_8 the_int_form
			   ,struct file_opened_node** file_opened_head, struct malloced_node** malloced_head, int the_debug);

int_8 dateToInt(char* the_date_form);


char* readFileChar(FILE* file, int_8 offset, int traverse_disk
				  ,struct file_opened_node** file_opened_head, struct malloced_node** malloced_head, int the_debug);

char* readFileCharData(FILE* file, int_8 offset, int_8* the_num_bytes, int traverse_disk
					  ,struct file_opened_node** file_opened_head, struct malloced_node** malloced_head, int the_debug);

int_8 readFileInt(FILE* file, int_8 offset, int traverse_disk
				 ,struct file_opened_node** file_opened_head, struct malloced_node** malloced_head, int the_debug);

double readFileDouble(FILE* file, int_8 offset, int traverse_disk
					 ,struct file_opened_node** file_opened_head, struct malloced_node** malloced_head, int the_debug);

int writeFileChar(FILE* file, int_8 offset, char* data
				 ,struct file_opened_node** file_opened_head, struct malloced_node** malloced_head, int the_debug);

int writeFileCharData(FILE* file, int_8 offset, int_8* the_num_bytes, char* data
					 ,struct file_opened_node** file_opened_head, struct malloced_node** malloced_head, int the_debug);

int writeFileInt(FILE* file, int_8 offset, int_8* data
				,struct file_opened_node** file_opened_head, struct malloced_node** malloced_head, int the_debug);

int writeFileDouble(FILE* file, int_8 offset, double* data
				   ,struct file_opened_node** file_opened_head, struct malloced_node** malloced_head, int the_debug);


int addListNodePtr(struct ListNodePtr** the_head, struct ListNodePtr** the_tail, void* the_ptr, int the_ptr_type, int the_add_mode
				  ,struct file_opened_node** file_opened_head, struct malloced_node** malloced_head, int the_debug);

void* removeListNodePtr(struct ListNodePtr** the_head, struct ListNodePtr** the_tail, void* the_ptr, int the_ptr_type, int the_remove_mode
					 ,struct file_opened_node** file_opened_head, struct malloced_node** malloced_head, int the_debug);

int traverseListNodesPtr(struct ListNodePtr** the_head, struct ListNodePtr** the_tail, int the_traverse_mode, char* start_text);

int freeListNodesPtr(struct ListNodePtr** the_head
					,struct file_opened_node** file_opened_head, struct malloced_node** malloced_head, int the_debug);

int freeListNodesPtrV2(struct ListNodePtr** the_tail
					  ,struct file_opened_node** file_opened_head, struct malloced_node** malloced_head, int the_debug);


bool equals(void* the_ptr_one, int the_ptr_type, void* the_ptr_two, int ptr_or_value);

int freeAnyLinkedList(void** the_head, int the_head_type
					 ,struct file_opened_node** file_opened_head, struct malloced_node** malloced_head, int the_debug);

int initEmptyTreeNode(void** ptr, void* the_parent, int node_type);

int traverseTreeNode(void** cur, int node_type, void** ptr_of_interest, int* ptr_of_interest_type, void** cur_mirror
					,struct file_opened_node** file_opened_head, struct malloced_node** malloced_head, int the_debug);


int errorTeardown(struct file_opened_node** file_opened_head, struct malloced_node** malloced_head
				 ,int the_debug);

#endif