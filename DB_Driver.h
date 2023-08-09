#ifndef DB_DRIVER_H_
#define DB_DRIVER_H_

typedef unsigned long long int_8;

struct malloced_node
{
	void* ptr;
	int persists;
	struct malloced_node* next;
};

struct file_opened_node
{
	FILE* file;
	struct file_opened_node* next;
};

/*	DB_Info File Structure
	8 bytes for the number of tables
	List of tables:
		32 bytes for the table name
		8 bytes for the table number (file number)

	_TabCol_ File Structure (in form "DB_TabCol_[table number].bin")
	8 bytes for the number of columns
	List of columns:
		32 bytes for the column name
		8 bytes for the datatype (and max length is applicable)
			datatype is gotten with the bytes % 10
			max length is gotten with the bytes / 100;
		8 bytes for the column_number

	_ColDataInfo_ File Structure (in form "DB_ColDataInfo_[table number]_[column number].bin")
	8 bytes for the number of rows
	8 bytes for the number of open slots
	List of open slots
		8 bytes for the row_id

	_ColData_ File Structure (in form "DB_ColData_[table number]_[column number].bin")
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
	int_8 num_added_insert_open;
	struct table_cols_info* next;
};

struct table_info
{
	char* name;	//	Max 31 bytes + 1 for \0
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
		1 = Integer or date
		2 = Real number
		3 = String
	*/
	int_8 data_int_date;
	double data_real;
	char* data_string;
	struct change_node* next;
};

struct or_clause_node
{
	struct and_clause_node* and_head;
	struct or_clause_node* next;
};

struct and_clause_node
{
	int_8 col_number;
	int_8 data_type;
	int_8 where_type;
	/*	Where types
		1 = "="
		2 = "<>"
	*/
	int_8 data_int_date;
	double data_real;
	char* data_string;
	struct and_clause_node* next;
};

struct colDataNode
{
	int_8 row_id;
	char* row_data;
};


void* myMalloc(struct malloced_node** malloced_head, size_t size, int the_persists);

int myFree(struct malloced_node** malloced_head, void** old_ptr);

int myFreeAllError(struct malloced_node** malloced_head);

int myFreeAllCleanup(struct malloced_node** malloced_head);

void concatFileName(char* new_filename, char* filetype, int_8 table_num, int_8 col_num);

FILE* myFileOpenSimple(struct file_opened_node** file_opened_head, char* file_name, char* mode);

FILE* myFileOpen(struct file_opened_node** file_opened_head, char* filetype, int_8 num_table, int_8 num_col, char* mode);

int myFileClose(struct file_opened_node** file_opened_head, FILE* old_file);

int myFileCloseAll(struct file_opened_node** file_opened_head);


struct table_info* getTablesHead();


int createNextTableNumFile();

int_8 getNextTableNum();


int initDB();

int traverseTablesInfoMemory();

int createTable(char* table_name, struct table_cols_info* table_cols);

int addColumn(FILE* tab_col_append, struct table_cols_info* cur_col, struct table_info* table);

int insertAppend(FILE** col_data_info_file_arr, FILE** col_data_file_arr, struct file_opened_node** file_opened_head
				,int_8 the_table_number, int_8 the_col_number, int_8 the_data_type, int_8* the_num_rows, int_8 the_max_length
				,int_8 the_data_int_date, double the_data_real, char* the_data_string);

int traverseTablesInfoDisk();


int freeMemOfDB();


/*
char* the_int = (char*) malloc(sizeof(char) * 32);
strcpy(the_int, "37066\0");
char* result = intToDate(the_int);
printf("37066 in date form is %s\n", result);
free(the_int);
free(result);

char* the_date = (char*) malloc(sizeof(char) * 32);
strcpy(the_date, "6/25/2001\0");
printf("6/25/2001 in int_8 form is %lu\n", dateToInt(the_date));
free(the_date);
*/
char* intToDate(char* the_int_form);

int_8 dateToInt(char* the_date_form);

char* readFileChar(struct malloced_node** malloced_head, FILE* file, int_8 offset);

char* readFileCharData(struct malloced_node** malloced_head, FILE* file, int_8 offset, int_8 num_bytes);

int_8 readFileInt(FILE* file, int_8 offset);

double readFileDouble(FILE* file, int_8 offset);

int writeFileChar(FILE* file, int_8 offset, char* data);

int writeFileCharData(FILE* file, int_8 offset, int_8 num_bytes, char* data);

int writeFileInt(FILE* file, int_8 offset, int_8* data);

int writeFileDouble(FILE* file, int_8 offset, double* data);

#endif