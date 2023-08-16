#ifndef DB_HELPERFUNCTIONS_H_
#define DB_HELPERFUNCTIONS_H_

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

void* myMalloc(struct malloced_node** malloced_head, size_t size, int the_persists);

int myFree(struct malloced_node** malloced_head, void** old_ptr, int the_debug);

int myFreeAllError(struct malloced_node** malloced_head, int the_debug);

int myFreeAllCleanup(struct malloced_node** malloced_head, int the_debug);

void concatFileName(char* new_filename, char* filetype, int_8 table_num, int_8 col_num);

FILE* myFileOpenSimple(struct file_opened_node** file_opened_head, char* file_name, char* mode);

FILE* myFileOpen(struct file_opened_node** file_opened_head, char* filetype, int_8 num_table, int_8 num_col, char* mode, int the_debug);

int myFileClose(struct file_opened_node** file_opened_head, FILE* old_file);

int myFileCloseAll(struct file_opened_node** file_opened_head, int the_debug);


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

#endif