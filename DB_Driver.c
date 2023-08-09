#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stddef.h>
#include <inttypes.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <assert.h>
#include <time.h>
#include "DB_Driver.h"

typedef unsigned long long int_8;
static char open_string[3] = "~~\0";
static int_8 open_int = -1;
static double open_double = -1.0;

static int_8 num_tables;
static struct table_info* tables_head;

struct ListNode
{
	int_8 value;
	struct ListNode* next;
};


void* myMalloc(struct malloced_node** malloced_head, size_t size, int the_persists)
{
	void* new_ptr = malloc(size);
	if (new_ptr == NULL)
		return NULL;

	if (*malloced_head == NULL)
	{
		*malloced_head = (struct malloced_node*) malloc(sizeof(struct malloced_node));
        if (*malloced_head == NULL)
		{
			free(new_ptr);
			return NULL;
		}
        (*malloced_head)->ptr = new_ptr;
		(*malloced_head)->persists = the_persists;
		(*malloced_head)->next = NULL;
	}
	else
	{
		struct malloced_node* temp = (struct malloced_node*) malloc(sizeof(struct malloced_node));
		if (temp == NULL)
		{
			free(new_ptr);
			return NULL;
		}
		temp->ptr = new_ptr;
		temp->persists = the_persists;
		temp->next = *malloced_head;
		*malloced_head = temp;
	}

	return new_ptr;
}

int myFree(struct malloced_node** malloced_head, void** old_ptr)
{
	if ((*malloced_head)->ptr == *old_ptr)
	{
		struct malloced_node* temp = *malloced_head;
		*malloced_head = (*malloced_head)->next;
		free(temp);
	}
	else
	{
		struct malloced_node* cur = *malloced_head;
		while (cur->next != NULL)
		{
			if (cur->next->ptr == *old_ptr)
			{
				struct malloced_node* temp = cur->next;
				cur->next = cur->next->next;
				free(temp);
                break;
			}
			cur = cur->next;
		}
	}

	if (*old_ptr == NULL)
	{
		printf("	ERROR in myFree() at line %d in %s\n", __LINE__, __FILE__);
		return -1;
	}
	else
	{
		free(*old_ptr);
		*old_ptr = NULL;
	}

	return 0;
}

int myFreeAllError(struct malloced_node** malloced_head)
{
    int total_freed = 0;
    while (*malloced_head != NULL)
    {
        struct malloced_node* temp = *malloced_head;
        *malloced_head = (*malloced_head)->next;
        if (temp->ptr != NULL)
			free(temp->ptr);
        free(temp);
        total_freed++;
    }
    printf("myFreeAllError() freed %d malloced_nodes and ptrs\n", total_freed);
    return total_freed;
}

int myFreeAllCleanup(struct malloced_node** malloced_head)
{
	int total_freed = 0;
	int total_not_persists = 0;
    while (*malloced_head != NULL)
    {
        struct malloced_node* temp = *malloced_head;
        *malloced_head = (*malloced_head)->next;
        if (temp->persists == 0 && temp->ptr != NULL)
		{
			free(temp->ptr);
			total_not_persists++;
		}
        free(temp);
        total_freed++;
    }
    printf("myFreeAllCleanup() freed %d malloced_nodes\n", total_freed);
	printf("myFreeAllCleanup() freed %d ptrs which dont persist\n", total_not_persists);
    return total_freed;
}

void concatFileName(char* new_filename, char* filetype, int_8 table_num, int_8 col_num)
{
	strcpy(new_filename, ".\\DB_Files_2\\DB");

	strcat(new_filename, filetype);

	if (strcmp("_Info", filetype) != 0)
    {
        char new_filename_number[10];

        sprintf(new_filename_number, "%lu", table_num);
        strcat(new_filename, new_filename_number);
    }

	if (strcmp("_Col_Data_", filetype) == 0 || strcmp("_Col_Data_Info_", filetype) == 0 || strcmp("_Col_Data_Info_Temp_", filetype) == 0)
	{
		strcat(new_filename, "_");

		char new_filename_col_number[10];

		sprintf(new_filename_col_number, "%lu", col_num);
		strcat(new_filename, new_filename_col_number);
	}

	strcat(new_filename, ".bin");
}

FILE* myFileOpenSimple(struct file_opened_node** file_opened_head, char* file_name, char* mode)
{
	FILE* new_file = fopen(file_name, mode);

	if (new_file != NULL)
    {
        if (*file_opened_head == NULL)
        {
            *file_opened_head = (struct file_opened_node*) malloc(sizeof(struct file_opened_node));
            if (*file_opened_head == NULL)
            {
                fclose(new_file);
                return NULL;
            }
            (*file_opened_head)->file = new_file;
            (*file_opened_head)->next = NULL;
        }
        else
        {
            struct file_opened_node* temp = (struct file_opened_node*) malloc(sizeof(struct file_opened_node));
            if (temp == NULL)
            {
                fclose(new_file);
                return NULL;
            }
            temp->file = new_file;
            temp->next = *file_opened_head;
            *file_opened_head = temp;
        }
    }

	return new_file;
}

FILE* myFileOpen(struct file_opened_node** file_opened_head, char* filetype, int_8 num_table, int_8 num_col, char* mode)
{
	char* file_name = (char*) malloc(sizeof(char) * 64);
	if (file_name == NULL)
	{
		printf("	ERROR in myFileOpen() at line %d in %s\n", __LINE__, __FILE__);
		return NULL;
	}
	concatFileName(file_name, filetype, num_table, num_col);

    FILE* new_file = fopen(file_name, mode);
	free(file_name);


    if (new_file != NULL)
    {
        if (*file_opened_head == NULL)
        {
            *file_opened_head = (struct file_opened_node*) malloc(sizeof(struct file_opened_node));
            if (*file_opened_head == NULL)
            {
                fclose(new_file);
                return NULL;
            }
            (*file_opened_head)->file = new_file;
            (*file_opened_head)->next = NULL;
        }
        else
        {
            struct file_opened_node* temp = (struct file_opened_node*) malloc(sizeof(struct file_opened_node));
            if (temp == NULL)
            {
                fclose(new_file);
                return NULL;
            }
            temp->file = new_file;
            temp->next = *file_opened_head;
            *file_opened_head = temp;
        }
    }

	return new_file;
}

int myFileClose(struct file_opened_node** file_opened_head, FILE* old_file)
{
    if ((*file_opened_head)->file == old_file)
	{
		struct file_opened_node* temp = *file_opened_head;
		*file_opened_head = (*file_opened_head)->next;
		free(temp);
	}
	else
	{
		struct file_opened_node* cur = *file_opened_head;
		while (cur->next != NULL)
		{
			if (cur->next->file == old_file)
			{
				struct file_opened_node* temp = cur->next;
				cur->next = cur->next->next;
				free(temp);
                break;
			}
			cur = cur->next;
		}
	}

	fclose(old_file);
    
    return 0;
}

int myFileCloseAll(struct file_opened_node** file_opened_head)
{
    int total_closed = 0;
    while (*file_opened_head != NULL)
    {
        struct file_opened_node* temp = *file_opened_head;
        *file_opened_head = (*file_opened_head)->next;
        fclose(temp->file);
        free(temp);
        total_closed++;
    }
    printf("myFileCloseAll() closed %d files\n", total_closed);
    return total_closed;
}


struct table_info* getTablesHead() { return tables_head; }


int createNextTableNumFile()
{
	struct file_opened_node* file_opened_head = NULL;
	
	FILE* next_table_file = myFileOpenSimple(&file_opened_head, ".\\Next_Table_Num.bin", "ab+");
	if (next_table_file == NULL)
	{
		printf("	ERROR in createNextTableNumFile() at line %d in %s\n", __LINE__, __FILE__);
		myFileCloseAll(&file_opened_head);
		return -1;
	}

	int_8 next_table_num = 1;

	if (writeFileInt(next_table_file, -1, &next_table_num) == -1)
	{
		printf("	ERROR in createNextTableNumFile() at line %d in %s\n", __LINE__, __FILE__);
		myFileCloseAll(&file_opened_head);
		return -1;
	}

	myFileClose(&file_opened_head, next_table_file);

	if (file_opened_head != NULL)
    {
        printf("createNextTableNumFile() did not close all files\n");
        myFileCloseAll(&file_opened_head);
    }

	return 0;
}

int_8 getNextTableNum()
{
	struct file_opened_node* file_opened_head = NULL;

	FILE* next_table_file = myFileOpenSimple(&file_opened_head, ".\\Next_Table_Num.bin", "rb+");
	if (next_table_file == NULL)
	{
		printf("	ERROR in getNextTableNum() at line %d in %s\n", __LINE__, __FILE__);
		myFileCloseAll(&file_opened_head);
		return -1;
	}

	int_8 next_table_num;

	if ((next_table_num = readFileInt(next_table_file, 0)) == -1)
	{
		printf("	ERROR in getNextTableNum() at line %d in %s\n", __LINE__, __FILE__);
		myFileCloseAll(&file_opened_head);
		return -1;
	}

	next_table_num++;

	if (writeFileInt(next_table_file, 0, &next_table_num) == -1)
	{
		printf("	ERROR in getNextTableNum() at line %d in %s\n", __LINE__, __FILE__);
		myFileCloseAll(&file_opened_head);
		return -1;
	}

	myFileClose(&file_opened_head, next_table_file);

	if (file_opened_head != NULL)
    {
        printf("getNextTableNum() did not close all files\n");
        myFileCloseAll(&file_opened_head);
    }

	return next_table_num;
}


int initDB()
{
	struct malloced_node* malloced_head = NULL;
    struct file_opened_node* file_opened_head = NULL;
    
    FILE* db_info = myFileOpen(&file_opened_head, "_Info", -1, -1, "rb+");
	if (db_info == NULL)
	{
		// START Create DB_Info if doesn't exist	
		db_info = myFileOpen(&file_opened_head, "_Info", -1, -1, "ab+");
		if (db_info == NULL)
		{
			printf("	ERROR in initDB() at line %d in %s\n", __LINE__, __FILE__);
            myFreeAllError(&malloced_head);
            myFileCloseAll(&file_opened_head);
			return -1;
		}

		num_tables = 0;

		if (writeFileInt(db_info, -1, &num_tables) == -1)
		{
			printf("	ERROR in initDB() at line %d in %s\n", __LINE__, __FILE__);
            myFreeAllError(&malloced_head);
            myFileCloseAll(&file_opened_head);
			return -1;
		}
		// END Create DB_Info if doesn't exist
	}
	else
		num_tables = readFileInt(db_info, 0);

	if (num_tables > 0)
	{
		// START Allocate space for tables_head
		tables_head = (struct table_info*) myMalloc(&malloced_head, sizeof(struct table_info), 1);
		if (tables_head == NULL)
		{
			printf("	ERROR in initDB() at line %d in %s\n", __LINE__, __FILE__);
            myFreeAllError(&malloced_head);
            myFileCloseAll(&file_opened_head);
			return -2;
		}
		struct table_info* cur = tables_head;
		// END Allocate space for tables_head

		// START Read files and input data into tables_head
		int cur_alloc = 0;
		int_8 offset = 8;
		while (cur_alloc < num_tables)
		{
			// START Read db_info for name and file number of table
            cur->name = readFileChar(&malloced_head, db_info, offset);
			offset += 32;
			cur->file_number = readFileInt(db_info, offset);
			offset += 8;
            // END Read db_info for name and file number of table

			// START Read tab_col for number of columns
            FILE* tab_col = myFileOpen(&file_opened_head, "_Tab_Col_", cur->file_number, 0, "rb+");
			if (tab_col == NULL)
			{
				printf("	ERROR in initDB() at line %d in %s\n", __LINE__, __FILE__);
                myFreeAllError(&malloced_head);
                myFileCloseAll(&file_opened_head);
				return -1;
			}

			int_8 col_offset = 0;

			cur->num_cols = readFileInt(tab_col, col_offset);
            // END Read tab_col for number of columns
			
			// START For each column in tab_col, allocate a struct table_cols_info and read info
            cur->table_cols_head = (struct table_cols_info*) myMalloc(&malloced_head, sizeof(struct table_cols_info), 1);
			if (cur->table_cols_head == NULL)
			{
				printf("	ERROR in initDB() at line %d in %s\n", __LINE__, __FILE__);
                myFreeAllError(&malloced_head);
                myFileCloseAll(&file_opened_head);
				return -2;
			}

			struct table_cols_info* cur_col = cur->table_cols_head;

			int cur_col_alloc = 0;
			col_offset += 8;

			while (cur_col_alloc < cur->num_cols)
			{
				cur_col->col_name = readFileChar(&malloced_head, tab_col, col_offset);
				col_offset += 32;

				int_8 datatype_and_max_length = readFileInt(tab_col, col_offset);
				cur_col->data_type = datatype_and_max_length & 0xff;
				cur_col->max_length = datatype_and_max_length >> 8;
				
				col_offset += 8;
				cur_col->col_number = readFileInt(tab_col, col_offset);
				col_offset += 8;

				// START Read data_col_info for num rows and num open
                FILE* data_col_info = myFileOpen(&file_opened_head, "_Col_Data_Info_", cur->file_number, cur_col->col_number, "rb+");
				if (data_col_info == NULL)
				{
					printf("	ERROR in initDB() at line %d in %s\n", __LINE__, __FILE__);
                    myFreeAllError(&malloced_head);
                    myFileCloseAll(&file_opened_head);
					return -1;
				}

				cur_col->num_rows = readFileInt(data_col_info, 0);
				cur_col->num_open = readFileInt(data_col_info, 8);

				myFileClose(&file_opened_head, data_col_info);

                cur_col->num_added_insert_open = 0;
                // END Read data_col_info for num rows and num open

				// START If more columns to read, allocate another struct table_cols_info
                if (cur_col_alloc < cur->num_cols-1)
				{
					cur_col->next = (struct table_cols_info*) myMalloc(&malloced_head, sizeof(struct table_cols_info), 1);
					if (cur_col->next == NULL)
					{
						printf("	ERROR in initDB() at line %d in %s\n", __LINE__, __FILE__);
                        myFreeAllError(&malloced_head);
                        myFileCloseAll(&file_opened_head);
						return -2;
					}

                    cur_col = cur_col->next;
				}
				else
					cur_col->next = NULL;
                // END If more columns to read, allocate another struct table_cols_info

				cur_col_alloc += 1;
			}
			myFileClose(&file_opened_head, tab_col);
            // END For each column in tab_col, allocate a struct table_cols_info and read info

			// START If more tables to read, allocate another struct table_info
            cur->change_list_head = NULL;
			cur->change_list_tail = NULL;

			if (cur_alloc < num_tables-1)
			{
				cur->next = (struct table_info*) myMalloc(&malloced_head, sizeof(struct table_info), 1);
				if (cur->next == NULL)
				{
					printf("	ERROR in initDB() at line %d in %s\n", __LINE__, __FILE__);
                    myFreeAllError(&malloced_head);
                    myFileCloseAll(&file_opened_head);
					return -2;
				}
				cur = cur->next;
			}
			else
				cur->next = NULL;
            // END If more tables to read, allocate another struct table_info

			cur_alloc++;
		}
		// END Read files and input data into tables_head
	}
	else
		tables_head = NULL;

	myFileClose(&file_opened_head, db_info);

    printf("Calling myFreeAllCleanup() from initDB()\n");
	myFreeAllCleanup(&malloced_head);

    if (file_opened_head != NULL)
    {
        printf("initDB() did not close all files\n");
        myFileCloseAll(&file_opened_head);
    }

	return 0;
}

int traverseTablesInfoMemory()
{
	printf("\nIN MEMORY\n\nnum_tables = %lu\n", num_tables);

	struct table_info* cur = tables_head;
	while (cur != NULL)
	{
		printf("Table name = %s\n", cur->name);
		printf("Table number = %lu\n", cur->file_number);
		printf("Number of columns = %lu\n", cur->num_cols);

		struct table_cols_info* cur_col = cur->table_cols_head;

		while (cur_col != NULL)
		{
			printf("	Column name = %s\n", cur_col->col_name);
			printf("	Datatype = %lu\n", cur_col->data_type);
			printf("	Max length = %lu\n", cur_col->max_length);
			printf("	Column number = %lu\n", cur_col->col_number);
			printf("	Number of rows = %lu\n", cur_col->num_rows);
			printf("	Number of open = %lu\n", cur_col->num_open);
			printf("	Number added to insert open list = %lu\n", cur_col->num_added_insert_open);

			cur_col = cur_col->next;
		}

		struct change_node* cur_change = cur->change_list_head;
		while (cur_change != NULL)
		{
			if (cur_change->operation == 1)
				printf("	_ Insert open change:\n");
			else if (cur_change->operation == 2)
				printf("	_ Insert append change:\n");
			else if (cur_change->operation == 3)
				printf("	_ Delete with no update change:\n");
			else if (cur_change->operation == 4)
				printf("	_ Delete before the update:\n");
			else if (cur_change->operation == 5)
				printf("	_ Insert open after deletion:\n");
			printf("		Transaction ID = %lu\n", cur_change->transac_id);
			printf("		Table number = %lu\n", cur_change->table_number);
			printf("		Column number = %lu\n", cur_change->col_number);
			printf("		Operation = %lu\n", cur_change->operation);
			printf("		Datatype = %lu\n", cur_change->data_type);
			if (cur_change->data_type == 1)
				printf("		Data int date = %lu\n", cur_change->data_int_date);
			else if (cur_change->data_type == 2)
				printf("		Data real = %f\n", cur_change->data_real);
			else if (cur_change->data_type == 3)
				printf("		Data string = %s\n", cur_change->data_string);

			cur_change = cur_change->next;
		}

		cur = cur->next;
	}
    printf("\n");
	return 0;
}

int createTable(char* table_name, struct table_cols_info* table_cols)
{
    struct malloced_node* malloced_head = NULL;
    struct file_opened_node* file_opened_head = NULL;

	// START Allocate space for a new struct table_info
	struct table_info* table;
	if (tables_head == NULL)
	{
		tables_head = (struct table_info*) myMalloc(&malloced_head, sizeof(struct table_info), 1);
		if (tables_head == NULL)
		{
			printf("	ERROR in createTable() at line %d in %s\n", __LINE__, __FILE__);
			myFreeAllError(&malloced_head);
            myFileCloseAll(&file_opened_head);
			return -2;
		}
		table = tables_head;
	}
	else
	{
		table = tables_head;
		while (table->next != NULL)
			table = table->next;

		table->next = (struct table_info*) myMalloc(&malloced_head, sizeof(struct table_info), 1);
		if (table->next == NULL)
		{
			printf("	ERROR in createTable() at line %d in %s\n", __LINE__, __FILE__);
			myFreeAllError(&malloced_head);
            myFileCloseAll(&file_opened_head);
			return -2;
		}
		table = table->next;
	}
	// END Allocate space for a new struct table_info

	// START Assign new struct table_info values
	table->name = table_name;
	table->file_number = getNextTableNum();
	if (table->file_number == -1)
	{
		printf("	ERROR in createTable() at line %d in %s\n", __LINE__, __FILE__);
		myFreeAllError(&malloced_head);
		myFileCloseAll(&file_opened_head);
		return -1;
	}
	table->num_cols = 0;
	table->table_cols_head = table_cols;
	table->change_list_head = NULL;
	table->change_list_tail = NULL;

	table->next = NULL;
	// END Assign new struct table_info values

	// START Append table name to DB_Info
	FILE* db_info_append = myFileOpen(&file_opened_head, "_Info", -1, -1, "ab");
	if (db_info_append == NULL)
	{
		printf("	ERROR in createTable() at line %d in %s\n", __LINE__, __FILE__);
		myFreeAllError(&malloced_head);
		myFileCloseAll(&file_opened_head);
		return -1;
	}

	if (writeFileChar(db_info_append, -1, table_name) != 0)
	{
		printf("	ERROR in createTable() at line %d in %s\n", __LINE__, __FILE__);
		myFreeAllError(&malloced_head);
		myFileCloseAll(&file_opened_head);
		return -1;
	}
	if (writeFileInt(db_info_append, -1, &table->file_number) != 0)
	{
		printf("	ERROR in createTable() at line %d in %s\n", __LINE__, __FILE__);
		myFreeAllError(&malloced_head);
		myFileCloseAll(&file_opened_head);
		return -1;
	}
	myFileClose(&file_opened_head, db_info_append);
	// END Append table name to DB_Info

	// START Create and open tab_col file and append spacer for num cols
	FILE* tab_col_append = myFileOpen(&file_opened_head, "_Tab_Col_", table->file_number, 0, "ab+");
	if (tab_col_append == NULL)
	{
		printf("	ERROR in createTable() at line %d in %s\n", __LINE__, __FILE__);
		myFreeAllError(&malloced_head);
		myFileCloseAll(&file_opened_head);
		return -1;
	}

	int_8 spacer_for_num_cols = 0;
	if (writeFileInt(tab_col_append, -1, &spacer_for_num_cols) != 0)
	{
		printf("	ERROR in createTable() at line %d in %s\n", __LINE__, __FILE__);
		myFreeAllError(&malloced_head);
		myFileCloseAll(&file_opened_head);
		return -1;
	}
	// END Create and open tab_col file and append spacer for num cols

	// START Append column info to tab_col and create col_data and col_data_info files
	struct table_cols_info* cur_col = table->table_cols_head;
	while (cur_col != NULL)
	{
		if (addColumn(tab_col_append, cur_col, table) != 0)
		{
			printf("	ERROR in createTable() at line %d in %s\n", __LINE__, __FILE__);
			myFreeAllError(&malloced_head);
			myFileCloseAll(&file_opened_head);
			return -1;
		}
		
		table->num_cols++;

		cur_col->num_rows = 0;
		cur_col->num_open = 0;
		cur_col->num_added_insert_open = 0;

		cur_col = cur_col->next;
	}

	fflush(tab_col_append);
	myFileClose(&file_opened_head, tab_col_append);
	// END Append column info to tab_col and create col_data and col_data_info files

	// START Write new number of columns to tab_col
	FILE* tab_col = myFileOpen(&file_opened_head, "_Tab_Col_", table->file_number, 0, "rb+");
	if (tab_col == NULL)
	{
		printf("	ERROR in createTable() at line %d in %s\n", __LINE__, __FILE__);
		myFreeAllError(&malloced_head);
		myFileCloseAll(&file_opened_head);
		return -1;
	}

	if (writeFileInt(tab_col, 0, &table->num_cols) != 0)
	{
		printf("	ERROR in createTable() at line %d in %s\n", __LINE__, __FILE__);
		myFreeAllError(&malloced_head);
		myFileCloseAll(&file_opened_head);
		return -1;
	}
	myFileClose(&file_opened_head, tab_col);
	// END Write new number of columns to tab_col

	// START Edit number of tables in DB_Info
	num_tables++;

	FILE* db_info = myFileOpen(&file_opened_head, "_Info", -1, -1, "rb+");
	if (db_info == NULL)
	{
		printf("	ERROR in createTable() at line %d in %s\n", __LINE__, __FILE__);
		myFreeAllError(&malloced_head);
		myFileCloseAll(&file_opened_head);
		return -1;
	}

	if (writeFileInt(db_info, 0, &num_tables) != 0)
	{
		printf("	ERROR in createTable() at line %d in %s\n", __LINE__, __FILE__);
		myFreeAllError(&malloced_head);
		myFileCloseAll(&file_opened_head);
		return -1;
	}
	myFileClose(&file_opened_head, db_info);
	// END Edit number of tables in DB_Info

    printf("Calling myFreeAllCleanup() from createTable()\n");
	myFreeAllCleanup(&malloced_head);

    if (file_opened_head != NULL)
    {
        printf("createTable() did not close all files\n");
        myFileCloseAll(&file_opened_head);
    }

	return 0;
}

int addColumn(FILE* tab_col_append, struct table_cols_info* cur_col, struct table_info* table)
{
	struct file_opened_node* file_opened_head = NULL;
	
	// START Append to tab_col_append file
	fwrite(cur_col->col_name, 32, 1, tab_col_append);

	int_8 datatype = cur_col->data_type + (cur_col->max_length << 8);

	fwrite(&datatype, 8, 1, tab_col_append);

	fwrite(&table->num_cols, 8, 1, tab_col_append);
	// END Append to tab_col_append file

	// START Create and append to col_data_info
	FILE* col_data_info = myFileOpen(&file_opened_head, "_Col_Data_Info_", table->file_number, table->num_cols, "ab+");
	if (col_data_info == NULL)
	{
		printf("	ERROR in addColumn() at line %d in %s\n", __LINE__, __FILE__);
		myFileCloseAll(&file_opened_head);
		return -1;
	}

	int_8 zero = 0;

	fwrite(&zero, 8, 1, col_data_info);

	fwrite(&zero, 8, 1, col_data_info);

	fflush(col_data_info);
	myFileClose(&file_opened_head, col_data_info);
	// END Create and append to col_data_info

	// START Create col_data
	FILE* col_data = myFileOpen(&file_opened_head, "_Col_Data_", table->file_number, table->num_cols, "ab+\0");
	if (col_data == NULL)
	{
		printf("	ERROR in addColumn() at line %d in %s\n", __LINE__, __FILE__);
		myFileCloseAll(&file_opened_head);
		return -1;
	}
	myFileClose(&file_opened_head, col_data);
	// END Create col_data

	if (file_opened_head != NULL)
	{
		printf("addColumn() did not close all files\n");
		myFileCloseAll(&file_opened_head);
	}

	return 0;
}

int insertAppend(FILE** col_data_info_file_arr, FILE** col_data_file_arr, struct file_opened_node** file_opened_head
				,int_8 the_table_number, int_8 the_col_number, int_8 the_data_type, int_8* the_num_rows, int_8 the_max_length
				,int_8 the_data_int_date, double the_data_real, char* the_data_string)
{
	// START Find or open col_data_info file for col_number
	FILE* col_data_info = col_data_info_file_arr[the_col_number];
	if (col_data_info == NULL)
	{
		printf("Had to input col_data_info file into array\n");
		col_data_info = myFileOpen(file_opened_head, "_Col_Data_Info_", the_table_number, the_col_number, "rb+");
		if (col_data_info == NULL)
		{
			printf("	ERROR in insertAppend() at line %d in %s\n", __LINE__, __FILE__);
			return -1;
		}
		col_data_info_file_arr[the_col_number] = col_data_info;
	}
	col_data_info = col_data_info_file_arr[the_col_number];
	// END Find or open col_data_info file for col_number

	// START Find or open col_data file for table_number and col_number
	FILE* col_data = col_data_file_arr[the_col_number];
	if (col_data == NULL)
	{
		printf("Had to input col_data file into array\n");
		col_data = myFileOpen(file_opened_head, "_Col_Data_", the_table_number, the_col_number, "ab+");
		if (col_data == NULL)
		{
			printf("	ERROR in insertAppend() at line %d in %s\n", __LINE__, __FILE__);
			return -1;
		}
		col_data_file_arr[the_col_number] = col_data;
	}
	col_data = col_data_file_arr[the_col_number];
	// END Find or open col_data file for table_number and col_number

	printf("Here A\n");

	// START Append data to col_data
	if (writeFileInt(col_data, -1, the_num_rows) != 0)
	{
		printf("	ERROR in insertAppend() at line %d in %s\n", __LINE__, __FILE__);
		return -1;
	}

	printf("Here B\n");

	if (the_data_type == 1 || the_data_type == 4)
	{
		printf("Int or date\n");
		if (writeFileInt(col_data, -1, &the_data_int_date) != 0)
		{
			printf("	ERROR in insertAppend() at line %d in %s\n", __LINE__, __FILE__);
			return -1;
		}
		printf("Succeeded\n");
	}
	else if (the_data_type == 2)
	{
		printf("Real\n");
		if (writeFileDouble(col_data, -1, &the_data_real) != 0)
		{
			printf("	ERROR in insertAppend() at line %d in %s\n", __LINE__, __FILE__);
			return -1;
		}
		printf("Succeeded\n");
	}
	else if (the_data_type == 3)
	{
		printf("String\n");
		if (writeFileCharData(col_data, -1, the_max_length, the_data_string) != 0)
		{
			printf("	ERROR in insertAppend() at line %d in %s\n", __LINE__, __FILE__);
			return -1;
		}
		printf("Succeeded\n");
	}
	// END Append data to col_data

	printf("Here C\n");

	// START Increment num_rows in col_data_info
	*the_num_rows++;

	if (writeFileInt(col_data_info, 0, the_num_rows) != 0)
	{
		printf("	ERROR in insertAppend() at line %d in %s\n", __LINE__, __FILE__);
		return -1;
	}
	// END Increment num_rows in col_data_info

	printf("Here D\n");

	return 0;
}

int traverseTablesInfoDisk()
{
	struct malloced_node* malloced_head = NULL;
    struct file_opened_node* file_opened_head = NULL;

	FILE* db_info = myFileOpen(&file_opened_head, "_Info", -1, -1, "rb+");
	if (db_info == NULL)
	{
		printf("	ERROR in traverseTablesInfoDisk() at line %d in %s\n", __LINE__, __FILE__);
		myFreeAllError(&malloced_head);
		myFileCloseAll(&file_opened_head);
		return -1;
	}

	int_8 temp_num_tables = readFileInt(db_info, 0);
	printf("\nDATA FILES ON DISK\n\nnum_tables = %lu\n\n", temp_num_tables);

	int cur_offset = 8;
	char* temp_table_name;
	while ((temp_table_name = readFileChar(&malloced_head, db_info, cur_offset)) != NULL)
	{
		printf("Table name = %s\n", temp_table_name);
		myFree(&malloced_head, (void**) &temp_table_name);
		cur_offset += 32;
		int_8 table_number = readFileInt(db_info, cur_offset);
		printf("Table number = %lu\n", table_number);
		cur_offset += 8;

		FILE* tab_col = myFileOpen(&file_opened_head, "_Tab_Col_", table_number, 0, "rb+\0");
		if (tab_col == NULL)
		{
			printf("	ERROR in traverseTablesInfoDisk() at line %d in %s\n", __LINE__, __FILE__);
			myFreeAllError(&malloced_head);
			myFileCloseAll(&file_opened_head);
			return -1;
		}
		
		int cur_col_offset = 8;
		char* temp_col_name;
		while ((temp_col_name = readFileChar(&malloced_head, tab_col, cur_col_offset)) != NULL)
		{
			printf("	Column name = %s\n", temp_col_name);
			myFree(&malloced_head, (void**) &temp_col_name);
			cur_col_offset += 32;

			int_8 temp_data = readFileInt(tab_col, cur_col_offset);
			int_8 temp_data_type = temp_data & 0xff;
			int_8 temp_max_length = temp_data >> 8;

			printf("	Datatype = %lu\n", temp_data_type);
			printf("	Max length = %lu\n", temp_max_length);
			cur_col_offset += 8;
			int_8 col_number = readFileInt(tab_col, cur_col_offset);
			printf("	Column number = %lu\n", col_number);
			cur_col_offset += 8;

			FILE* col_data_info = myFileOpen(&file_opened_head, "_Col_Data_Info_", table_number, col_number, "rb+\0");
			if (col_data_info == NULL)
			{
				printf("	ERROR in traverseTablesInfoDisk() at line %d in %s\n", __LINE__, __FILE__);
				myFreeAllError(&malloced_head);
				myFileCloseAll(&file_opened_head);
				return -1;
			}

			printf("	Number of rows = %lu\n", readFileInt(col_data_info, 0));
			printf("	Number of open slots = %lu\n", readFileInt(col_data_info, 8));

			printf("	Open slots: ");
			int_8 open_offset = 16;
			int_8 open_slot;
			while ((open_slot = readFileInt(col_data_info, open_offset)) != -1)
			{
				printf("%lu, ", open_slot);
				open_offset += 8;
			}
			printf("\n");

			myFileClose(&file_opened_head, col_data_info);

			FILE* col_data = myFileOpen(&file_opened_head, "_Col_Data_", table_number, col_number, "rb+\0");
			if (col_data == NULL)
			{
				printf("	ERROR in traverseTablesInfoDisk() at line %d in %s\n", __LINE__, __FILE__);
				myFreeAllError(&malloced_head);
				myFileCloseAll(&file_opened_head);
				return -1;
			}

			printf("	Column data:\n");
			int_8 rows_offset = 0;
			int_8 row_id;
			while ((row_id = readFileInt(col_data, rows_offset)) != -1)
			{
				printf("		%lu	", row_id);
				rows_offset += 8;

				if (temp_data_type == 1 || temp_data_type == 4)
					printf("%lu\n", readFileInt(col_data, rows_offset));
				else if (temp_data_type == 2)
					printf("%f\n", readFileDouble(col_data, rows_offset));
				else if (temp_data_type == 3)
				{
					char* temp_str = readFileCharData(&malloced_head, col_data, rows_offset, temp_max_length);
					printf("%s\n", temp_str);
					myFree(&malloced_head, (void**) &temp_str);
				}
				rows_offset += temp_max_length;
			}
			myFileClose(&file_opened_head, col_data);
		}
		myFileClose(&file_opened_head, tab_col);
	}
	myFileClose(&file_opened_head, db_info);

	printf("Calling myFreeAllCleanup() from traverseTablesInfoDisk()\n");
	myFreeAllCleanup(&malloced_head);

    if (file_opened_head != NULL)
    {
        printf("traverseTablesInfoDisk() did not close all files\n");
        myFileCloseAll(&file_opened_head);
    }

	return 0;
}

int freeMemOfDB()
{
	struct table_info* cur_table = tables_head;
	while (cur_table != NULL)
	{
		if (cur_table->change_list_head != NULL)
		{
			printf("\nThere are uncommitted changes.\nCommit them with \"commit;\" or hit enter to erase those changes.\n\n");
			char input[10];
			scanf("%[^\n]", input);
			input[7] = 0;

			if (strcmp(input, "commit;") == 0)
            {
                //commit();
            }
			else
			{
				printf("Not committing, unsaved changes will be erased.\n");
				break;
			}
		}

		cur_table = cur_table->next;
	}

	int total_freed = 0;
	
	while (tables_head != NULL)
	{
		while (tables_head->table_cols_head != NULL)
		{
			struct table_cols_info* temp = tables_head->table_cols_head;
			tables_head->table_cols_head = tables_head->table_cols_head->next;
			free(temp->col_name);
			free(temp);
			total_freed+=2;
		}

		while (tables_head->change_list_head != NULL)
		{
			struct change_node* temp = tables_head->change_list_head;
			tables_head->change_list_head = tables_head->change_list_head->next;
			free(temp->data_string);
			free(temp);
			total_freed+=2;
		}

		struct table_info* temp = tables_head;
		tables_head = tables_head->next;
		free(temp->name);
		free(temp);
		total_freed+=2;
	}

	printf("freeMemOfDB() freed %d things\n", total_freed);
	return 0;
}


char* intToDate(char* the_int_form)
{
	int year = 1900;
	int month = 1;
	int day = 1;

	int remaining = atoi(the_int_form);

	// START Find the right year
	while (remaining > 1461)
	{
		year += 4;
		remaining -= 1461;
	}
	if (remaining > 366)
	{
		year += 1;
		remaining -= 366;
	}
	if (year % 4 != 0)
	{
		while (remaining > 365)
		{
			year += 1;
			remaining -= 365;
		}
	}
	// END Find the right year

	// START Find the right month
	if (remaining > 31)	//	Jan
	{
		month += 1;
		remaining -= 31;
	}
	if (remaining > (year % 4 == 0 ? 29 : 28))	//	Feb
	{
		month += 1;
		remaining -= (year % 4 == 0 ? 29 : 28);
	}
	if (remaining > 31)	//	Mar
	{
		month += 1;
		remaining -= 31;
	}
	if (remaining > 30)	//	Apr
	{
		month += 1;
		remaining -= 30;
	}
	if (remaining > 31)	//	May
	{
		month += 1;
		remaining -= 31;
	}
	if (remaining > 30)	//	Jun
	{
		month += 1;
		remaining -= 30;
	}
	if (remaining > 31)	//	Jul
	{
		month += 1;
		remaining -= 31;
	}
	if (remaining > 31)	//	Aug
	{
		month += 1;
		remaining -= 31;
	}
	if (remaining > 30)	//	Sep
	{
		month += 1;
		remaining -= 30;
	}
	if (remaining > 31)	//	Oct
	{
		month += 1;
		remaining -= 31;
	}
	if (remaining > 30)	//	Nov
	{
		month += 1;
		remaining -= 30;
	}
	if (remaining > 31)	//	Dec
	{
		month += 1;
		remaining -= 31;
	}
	// END Find the right month

	day += remaining;

	char* date = (char*) malloc(sizeof(char) * 20);
	if (date == NULL)
	{
		printf("	ERROR in intToDate() at line %d in %s\n", __LINE__, __FILE__);
		return NULL;
	}

	char month_char[3];
	sprintf(month_char, "%d", month);
	strcpy(date, month_char);
	strcat(date, "/");
	
	char day_char[3];
	sprintf(day_char, "%d", day);
	strcat(date, day_char);
	strcat(date, "/");

	char year_char[5];
	sprintf(year_char, "%d", year);
	strcat(date, year_char);

	return date;
}

int_8 dateToInt(char* the_date_form)
{
	int year;
	int month;
	int day;
	
	sscanf(the_date_form, "%d/%d/%d", &month, &day, &year);

	int_8 remaining = (int_8) day;
	if (month == 12)	//	Nov
	{
		remaining += 30;
		month -= 1;
	}
	if (month == 11)	//	Oct
	{
		remaining += 31;
		month -= 1;
	}
	if (month == 10)	//	Sep
	{
		remaining += 30;
		month -= 1;
	}
	if (month == 9)		//	Aug
	{
		remaining += 31;
		month -= 1;
	}
	if (month == 8)		//	Jul
	{
		remaining += 31;
		month -= 1;
	}
	if (month == 7)		//	Jun
	{
		remaining += 30;
		month -= 1;
	}
	if (month == 6)		//	May
	{
		remaining += 31;
		month -= 1;
	}
	if (month == 5)		//	Apr
	{
		remaining += 30;
		month -= 1;
	}
	if (month == 4)		//	Mar
	{
		remaining += 31;
		month -= 1;
	}
	if (month == 3)		//	Feb
	{
		remaining += (year % 4 == 0 ? 29 : 28);
		month -= 1;
	}
	if (month == 2)		//	Jan
	{
		remaining += 31;
		month -= 1;
	}

	while (year % 4 != 0)
	{
		remaining += 365;
		year--;
	}

	while (year > 1900)
	{
		remaining += 1461;
		year -= 4;
	}

	return remaining;
}

char* readFileChar(struct malloced_node** malloced_head, FILE* file, int_8 offset)
{
	int num_bytes = 32;

	char* raw_bytes = (char*) myMalloc(malloced_head, sizeof(char) * num_bytes, 1);
	if (raw_bytes == NULL)
	{
		printf("	ERROR in readFileChar() at line %d in %s\n", __LINE__, __FILE__);
		return NULL;
	}

	fseek(file, offset, SEEK_SET);

	if (fread(raw_bytes, num_bytes, 1, file) == 0)
	{
		printf("	ERROR in readFileChar() at line %d in %s\n", __LINE__, __FILE__);
		myFree(malloced_head, (void**) &raw_bytes);
		return NULL;
	}

	return raw_bytes;
}

char* readFileCharData(struct malloced_node** malloced_head, FILE* file, int_8 offset, int_8 num_bytes)
{
	char* raw_bytes = (char*) myMalloc(malloced_head, sizeof(char) * num_bytes, 1);
	if (raw_bytes == NULL)
	{
		printf("	ERROR in readFileCharData() at line %d in %s\n", __LINE__, __FILE__);
		return NULL;
	}

	fseek(file, offset, SEEK_SET);

	if (fread(raw_bytes, num_bytes, 1, file) == 0)
	{
		printf("	ERROR in readFileCharData() at line %d in %s\n", __LINE__, __FILE__);
		myFree(malloced_head, (void**) &raw_bytes);
		return NULL;
	}

	return raw_bytes;
}

int_8 readFileInt(FILE* file, int_8 offset)
{
	int num_bytes = 8;

	int_8 raw_int;

	fseek(file, offset, SEEK_SET);

	if (fread(&raw_int, num_bytes, 1, file) == 0)
		return -1;

	return raw_int;
}

double readFileDouble(FILE* file, int_8 offset)
{
	int num_bytes = 8;

	double raw_double;

	fseek(file, offset, SEEK_SET);

	if (fread(&raw_double, num_bytes, 1, file) == 0)
		return -1.0;

	return raw_double;
}

int writeFileChar(FILE* file, int_8 offset, char* data)
{
	int num_bytes = 32;

	if (offset != -1)
		fseek(file, offset, SEEK_SET);

	//int fd = fdnum(file);
	//int ioctl(fd, FIONBIO);
	
	if (fwrite(data, num_bytes, 1, file) != 1)
		return -1;

	//printf("If there was an error _%s_\n", strerror(errno));

	fflush(file);

	return 0;
}

int writeFileCharData(FILE* file, int_8 offset, int_8 num_bytes, char* data)
{
	if (offset != -1)
		fseek(file, offset, SEEK_SET);

	//clock_t endwait;
    //endwait = clock () + 2 * CLOCKS_PER_SEC;

	//int fd = fdnum(file);
	//int ioctl(fd, FIONBIO);
	
	if (fwrite(data, num_bytes, 1, file) != 1)
		return -1;

	//printf("If there was an error _%s_\n", strerror(errno));

	fflush(file);

	return 0;
}

int writeFileInt(FILE* file, int_8 offset, int_8* data)
{
	int num_bytes = 8;

	if (offset != -1)
		fseek(file, offset, SEEK_SET);

	//int fd = fdnum(file);
	//int ioctl(fd, FIONBIO);
	
	if (fwrite(data, num_bytes, 1, file) != 1)
		return -1;

	//printf("If there was an error _%s_\n", strerror(errno));

	fflush(file);

	return 0;
}

int writeFileDouble(FILE* file, int_8 offset, double* data)
{
	int num_bytes = 8;

	if (offset != -1)
		fseek(file, offset, SEEK_SET);

	//int fd = fdnum(file);
	//int ioctl(fd, FIONBIO);
	
	if (fwrite(data, num_bytes, 1, file) != 1)
		return -1;

	//printf("If there was an error _%s_\n", strerror(errno));

	fflush(file);

	return 0;
}