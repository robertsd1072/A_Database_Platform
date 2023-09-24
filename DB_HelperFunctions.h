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

struct ListNode
{
	int_8 value;
	struct ListNode* next;
	struct ListNode* prev;
};


int strLength(char* str);

int strcontains(char* str, char the_char);

int indexOf(char* str, char the_char);

char* substring(char* str, int start, int end
			   ,struct file_opened_node** file_opened_head, struct malloced_node** malloced_head, int the_debug);

char** strSplit(char* str, char the_char, int* size_result
			   ,struct file_opened_node** file_opened_head, struct malloced_node** malloced_head, int the_debug);

void* myMalloc(size_t size
			  ,struct file_opened_node** file_opened_head, struct malloced_node** malloced_head, int the_debug);

int myFree(void** old_ptr
		  ,struct file_opened_node** file_opened_head, struct malloced_node** malloced_head, int the_debug);

int myFreeAllError(struct malloced_node** malloced_head, int the_debug);

int myFreeAllCleanup(struct malloced_node** malloced_head, int the_debug);

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


char* readFileChar(FILE* file, int_8 offset
				  ,struct file_opened_node** file_opened_head, struct malloced_node** malloced_head, int the_debug);

char* readFileCharData(FILE* file, int_8 offset, int_8* the_num_bytes
					  ,struct file_opened_node** file_opened_head, struct malloced_node** malloced_head, int the_debug);

int_8 readFileInt(FILE* file, int_8 offset
				 ,struct file_opened_node** file_opened_head, struct malloced_node** malloced_head, int the_debug);

double readFileDouble(FILE* file, int_8 offset
					 ,struct file_opened_node** file_opened_head, struct malloced_node** malloced_head, int the_debug);

int writeFileChar(FILE* file, int_8 offset, char* data
				 ,struct file_opened_node** file_opened_head, struct malloced_node** malloced_head, int the_debug);

int writeFileCharData(FILE* file, int_8 offset, int_8* the_num_bytes, char* data
					 ,struct file_opened_node** file_opened_head, struct malloced_node** malloced_head, int the_debug);

int writeFileInt(FILE* file, int_8 offset, int_8* data
				,struct file_opened_node** file_opened_head, struct malloced_node** malloced_head, int the_debug);

int writeFileDouble(FILE* file, int_8 offset, double* data
				   ,struct file_opened_node** file_opened_head, struct malloced_node** malloced_head, int the_debug);


int addListNode(struct ListNode** the_head, struct ListNode** the_tail, int the_value, int the_add_mode
			   ,struct file_opened_node** file_opened_head, struct malloced_node** malloced_head, int the_debug);

int removeListNode(struct ListNode** the_head, struct ListNode** the_tail, int the_value, int the_remove_mode
				  ,struct file_opened_node** file_opened_head, struct malloced_node** malloced_head, int the_debug);

int traverseListNodes(struct ListNode** the_head, struct ListNode** the_tail, int the_mode, char* start_text);

int freeListNodes(struct ListNode** the_head
				 ,struct file_opened_node** file_opened_head, struct malloced_node** malloced_head, int the_debug);


int errorTeardown(struct file_opened_node** file_opened_head, struct malloced_node** malloced_head
				 ,int the_debug);

#endif