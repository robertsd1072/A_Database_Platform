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

void* myMalloc(struct malloced_node** malloced_head, size_t size, int the_persists);

int myFree(struct malloced_node** malloced_head, void** old_ptr, int the_debug);

int myFreeAllError(struct malloced_node** malloced_head, int the_debug);

int myFreeAllCleanup(struct malloced_node** malloced_head, int the_debug);

void concatFileName(char* new_filename, char* filetype, int_8 table_num, int_8 col_num);

FILE* myFileOpenSimple(struct file_opened_node** file_opened_head, char* file_name, char* mode);

FILE* myFileOpen(struct file_opened_node** file_opened_head, char* filetype, int_8 num_table, int_8 num_col, char* mode, int the_debug);

int myFileClose(struct file_opened_node** file_opened_head, FILE* old_file);

int myFileCloseAll(struct file_opened_node** file_opened_head, int the_debug);


char* intToDate(struct malloced_node** malloced_head, int_8 the_int_form, int the_debug);

int_8 dateToInt(char* the_date_form);

char* readFileChar(struct malloced_node** malloced_head, FILE* file, int_8 offset, int the_debug);

char* readFileCharData(struct malloced_node** malloced_head, FILE* file, int_8 offset, int_8 num_bytes, int the_debug);

int_8 readFileInt(FILE* file, int_8 offset);

double readFileDouble(FILE* file, int_8 offset);

int writeFileChar(FILE* file, int_8 offset, char* data);

int writeFileCharData(FILE* file, int_8 offset, int_8 num_bytes, char* data);

int writeFileInt(FILE* file, int_8 offset, int_8* data);

int writeFileDouble(FILE* file, int_8 offset, double* data);


int addListNode(struct malloced_node** malloced_head, struct ListNode** the_head, struct ListNode** the_tail, int the_value, int persists, int the_add_mode);

int removeListNode(struct malloced_node** malloced_head, struct ListNode** the_head, struct ListNode** the_tail, int the_value, int the_remove_mode, int the_debug);

int traverseListNodes(struct ListNode** the_head, struct ListNode** the_tail, int the_mode, char* start_text);

int freeListNodes(struct malloced_node** malloced_head, struct ListNode** the_head, int the_debug);

#endif