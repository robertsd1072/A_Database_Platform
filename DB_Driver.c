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
#include <math.h>
#include <pthread.h>
#include <semaphore.h>

#include "DB_Driver.h"
#include "DB_HelperFunctions.h"
#include "DB_SHM_Struct.h"

typedef unsigned long long int_8;

static char null_string[4] = "#*#\0";
static int_8 null_int = 0x55555555;
static double null_double = 0x55555555;

static int_8 num_tables;
static struct table_info* tables_head;


struct table_info* getTablesHead() { return tables_head; }


/*	RETURNS:
 *  RETURN_ERROR if error
 *	RETURN_GOOD if good
 *
 *	WRITES TO:
 */
int createNextTableNumFile(struct malloced_node** malloced_head, int the_debug)
{
	struct file_opened_node* file_opened_head = NULL;
	
	FILE* next_table_file = myFileOpenSimple("Next_Table_Num.bin", "ab+", &file_opened_head, malloced_head, the_debug);
	if (next_table_file == NULL)
	{
		if (the_debug == YES_DEBUG)
			printf("	ERROR in createNextTableNumFile() at line %d in %s\n", __LINE__, __FILE__);
		return RETURN_ERROR;
	}

	int_8 next_table_num = 1;

	if (writeFileInt(next_table_file, -1, &next_table_num, &file_opened_head, malloced_head, the_debug) == -1)
	{
		if (the_debug == YES_DEBUG)
			printf("	ERROR in createNextTableNumFile() at line %d in %s\n", __LINE__, __FILE__);
		return RETURN_ERROR;
	}

	myFileClose(next_table_file, &file_opened_head, malloced_head, the_debug);

	if (file_opened_head != NULL)
    {
        if (the_debug == YES_DEBUG)
        	printf("createNextTableNumFile() did not close all files\n");
        myFileCloseAll(&file_opened_head, malloced_head, the_debug);
    }

	return RETURN_GOOD;
}

/*	RETURNS:
 *  RETURN_ERROR if error
 *	Valid table number if good
 *
 *	WRITES TO:
 */
int_8 getNextTableNum(struct malloced_node** malloced_head, int the_debug)
{
	if (access("Next_Table_Num.bin", F_OK) != 0)
	{
		if (createNextTableNumFile(malloced_head, the_debug) != RETURN_GOOD)
		{
			if (the_debug == YES_DEBUG)
				printf("	ERROR in getNextTableNum() at line %d in %s\n", __LINE__, __FILE__);
			return RETURN_ERROR;
		}
	}

	struct file_opened_node* file_opened_head = NULL;

	FILE* next_table_file = myFileOpenSimple("Next_Table_Num.bin", "rb+", &file_opened_head, malloced_head, the_debug);
	if (next_table_file == NULL)
	{
		if (the_debug == YES_DEBUG)
			printf("	ERROR in getNextTableNum() at line %d in %s\n", __LINE__, __FILE__);
		return RETURN_ERROR;
	}

	int_8 next_table_num;

	if ((next_table_num = readFileInt(next_table_file, 0, NO_TRAVERSE_DISK, &file_opened_head, malloced_head, the_debug)) == -1)
	{
		if (the_debug == YES_DEBUG)
			printf("	ERROR in getNextTableNum() at line %d in %s\n", __LINE__, __FILE__);
		return RETURN_ERROR;
	}

	next_table_num++;

	if (writeFileInt(next_table_file, 0, &next_table_num, &file_opened_head, malloced_head, the_debug) == -1)
	{
		if (the_debug == YES_DEBUG)
			printf("	ERROR in getNextTableNum() at line %d in %s\n", __LINE__, __FILE__);
		return RETURN_ERROR;
	}

	myFileClose(next_table_file, &file_opened_head, malloced_head, the_debug);

	if (file_opened_head != NULL)
    {
        if (the_debug == YES_DEBUG)
        	printf("getNextTableNum() did not close all files\n");
        myFileCloseAll(&file_opened_head, malloced_head, the_debug);
    }

	return next_table_num;
}

/*	RETURNS:
 *  RETURN_ERROR if error
 *	RETURN_GOOD if good
 *
 *	WRITES TO:
 *	 
 *	Persistent malloced items: 
 *		struct table_info* tables_head;
 */
int initDB(struct malloced_node** malloced_head, int the_debug)
{
	struct file_opened_node* file_opened_head = NULL;
    
    FILE* db_info;
    if (access("DB_Files_2/DB_Info.bin", F_OK) != 0)
	{
		//printf("Creating file\n");
		// START Create DB_Info if doesn't exist	
		db_info = myFileOpen("_Info", -1, -1, "ab+", &file_opened_head, malloced_head, the_debug);
		if (db_info == NULL)
		{
			if (the_debug == YES_DEBUG)
				printf("	ERROR in initDB() at line %d in %s\n", __LINE__, __FILE__);
			return RETURN_ERROR;
		}

		num_tables = 0;

		if (writeFileInt(db_info, -1, &num_tables, &file_opened_head, malloced_head, the_debug) == -1)
		{
			if (the_debug == YES_DEBUG)
				printf("	ERROR in initDB() at line %d in %s\n", __LINE__, __FILE__);
			return RETURN_ERROR;
		}
		// END Create DB_Info if doesn't exist
	}
	else
	{
		//printf("Opening file\n");
		db_info = myFileOpen("_Info", -1, -1, "rb+", &file_opened_head, malloced_head, the_debug);
		if (db_info == NULL)
		{
			if (the_debug == YES_DEBUG)
				printf("	ERROR in initDB() at line %d in %s\n", __LINE__, __FILE__);
			return RETURN_ERROR;
		}
		num_tables = readFileInt(db_info, 0, NO_TRAVERSE_DISK, &file_opened_head, malloced_head, the_debug);
	}

	if (the_debug == YES_DEBUG)
		printf("num_tables = %d\n", num_tables);

	if (num_tables > 0)
	{
		// START Allocate space for tables_head
		tables_head = (struct table_info*) myMalloc(sizeof(struct table_info), &file_opened_head, malloced_head, the_debug);
		if (tables_head == NULL)
		{
			if (the_debug == YES_DEBUG)
				printf("	ERROR in initDB() at line %d in %s\n", __LINE__, __FILE__);
			return RETURN_ERROR;
		}
		struct table_info* cur = tables_head;
		// END Allocate space for tables_head

		// START Read files and input data into tables_head
		int cur_alloc = 0;
		int_8 offset = 8;
		while (cur_alloc < num_tables)
		{
			// START Read db_info for name and file number of table
            cur->name = readFileChar(db_info, offset, NO_TRAVERSE_DISK, &file_opened_head, malloced_head, the_debug);
			offset += 32;
            cur->keyword = readFileChar(db_info, offset, NO_TRAVERSE_DISK, &file_opened_head, malloced_head, the_debug);
			offset += 32;
			cur->file_number = readFileInt(db_info, offset, NO_TRAVERSE_DISK, &file_opened_head, malloced_head, the_debug);
			offset += 8;
            // END Read db_info for name and file number of table

			// START Read tab_col for number of columns
			FILE* tab_col = myFileOpen("_Tab_Col_", cur->file_number, 0, "rb+", &file_opened_head, malloced_head, the_debug);
			if (tab_col == NULL)
			{
				if (the_debug == YES_DEBUG)
					printf("	ERROR in initDB() at line %d in %s\n", __LINE__, __FILE__);
				return RETURN_ERROR;
			}

			int_8 col_offset = 0;

			cur->num_cols = readFileInt(tab_col, col_offset, NO_TRAVERSE_DISK, &file_opened_head, malloced_head, the_debug);
            // END Read tab_col for number of columns
			
			// START For each column in tab_col, allocate a struct table_cols_info and read info
            cur->table_cols_head = (struct table_cols_info*) myMalloc(sizeof(struct table_cols_info), &file_opened_head, malloced_head, the_debug);
			if (cur->table_cols_head == NULL)
			{
				if (the_debug == YES_DEBUG)
					printf("	ERROR in initDB() at line %d in %s\n", __LINE__, __FILE__);
				return RETURN_ERROR;
			}

			struct table_cols_info* cur_col = cur->table_cols_head;

			int cur_col_alloc = 0;
			col_offset += 8;

			while (cur_col_alloc < cur->num_cols)
			{
				cur_col->home_table = cur;

				cur_col->col_name = readFileChar(tab_col, col_offset, NO_TRAVERSE_DISK, &file_opened_head, malloced_head, the_debug);
				col_offset += 32;

				int_8 datatype_and_max_length = readFileInt(tab_col, col_offset, NO_TRAVERSE_DISK, &file_opened_head, malloced_head, the_debug);
				cur_col->data_type = datatype_and_max_length & 0xff;
				cur_col->max_length = datatype_and_max_length >> 8;
				
				col_offset += 8;
				cur_col->col_number = readFileInt(tab_col, col_offset, NO_TRAVERSE_DISK, &file_opened_head, malloced_head, the_debug);
				col_offset += 8;

				// START Read data_col_info for num rows and num open
                FILE* data_col_info = myFileOpen("_Col_Data_Info_", cur->file_number, cur_col->col_number, "rb+", &file_opened_head, malloced_head, the_debug);
				if (data_col_info == NULL)
				{
					if (the_debug == YES_DEBUG)
						printf("	ERROR in initDB() at line %d in %s\n", __LINE__, __FILE__);
					return RETURN_ERROR;
				}

				cur_col->num_rows = readFileInt(data_col_info, 0, NO_TRAVERSE_DISK, &file_opened_head, malloced_head, the_debug);
				cur_col->num_open = readFileInt(data_col_info, 8, NO_TRAVERSE_DISK, &file_opened_head, malloced_head, the_debug);

				// START Read open rows into linked list
				cur_col->open_list_head = NULL;
				cur_col->open_list_before_tail = NULL;

				if (cur_col->num_open > 0)
				{
					int* value = (int*) myMalloc(sizeof(int), &file_opened_head, malloced_head, the_debug);
					if (value == NULL)
					{
						if (the_debug == YES_DEBUG)
							printf("	ERROR in initDB() at line %d in %s\n", __LINE__, __FILE__);
						return RETURN_ERROR;
					}

					*value = readFileInt(data_col_info, 16, NO_TRAVERSE_DISK, &file_opened_head, malloced_head, the_debug);

					if (addListNodePtr(&cur_col->open_list_head, &cur_col->open_list_before_tail, value, PTR_TYPE_INT, ADDLISTNODE_TAIL
									  ,&file_opened_head, malloced_head, the_debug) != 0)
					{
						if (the_debug == YES_DEBUG)
							printf("	ERROR in initDB() at line %d in %s\n", __LINE__, __FILE__);
						return RETURN_ERROR;
					}

					if (cur_col->num_open == 1)
						cur_col->open_list_before_tail = NULL;

					for (int i=1; i<cur_col->num_open; i++)
					{
						int* value = (int*) myMalloc(sizeof(int), &file_opened_head, malloced_head, the_debug);
						if (value == NULL)
						{
							if (the_debug == YES_DEBUG)
								printf("	ERROR in initDB() at line %d in %s\n", __LINE__, __FILE__);
							return RETURN_ERROR;
						}

						*value = readFileInt(data_col_info, 16 + (i*8), NO_TRAVERSE_DISK, &file_opened_head, malloced_head, the_debug);

						if (addListNodePtr(&cur_col->open_list_head, &cur_col->open_list_before_tail, value, PTR_TYPE_INT, ADDLISTNODE_TAIL
										  ,&file_opened_head, malloced_head, the_debug) != 0)
						{
							if (the_debug == YES_DEBUG)
								printf("	ERROR in initDB() at line %d in %s\n", __LINE__, __FILE__);
							return RETURN_ERROR;
						}
					}

					if (cur_col->num_open > 1)
						cur_col->open_list_before_tail = cur_col->open_list_before_tail->prev;
				}
				// END Read open rows into linked list

				myFileClose(data_col_info, &file_opened_head, malloced_head, the_debug);

                cur_col->num_added_insert_open = 0;
                // END Read data_col_info for num rows and num open


				cur_col->unique_list_head = NULL;
				cur_col->unique_list_tail = NULL;
				cur_col->frequent_list_head = NULL;
				cur_col->frequent_list_tail = NULL;
				cur_col->frequent_arr_row_to_node = NULL;


				// START If more columns to read, allocate another struct table_cols_info
                if (cur_col_alloc < cur->num_cols-1)
				{
					cur_col->next = (struct table_cols_info*) myMalloc(sizeof(struct table_cols_info), &file_opened_head, malloced_head, the_debug);
					if (cur_col->next == NULL)
					{
						if (the_debug == YES_DEBUG)
							printf("	ERROR in initDB() at line %d in %s\n", __LINE__, __FILE__);
						return RETURN_ERROR;
					}
                    cur_col = cur_col->next;
				}
				else
					cur_col->next = NULL;
                // END If more columns to read, allocate another struct table_cols_info

				cur_col_alloc += 1;
			}
			myFileClose(tab_col, &file_opened_head, malloced_head, the_debug);
            // END For each column in tab_col, allocate a struct table_cols_info and read info


            // START This function
            /*if (cur->table_cols_head->num_rows < MAX_ROWS_FOR_INIT_FREQ_LISTS)
            {
				if (initFrequentLists(cur, malloced_head, the_debug) != RETURN_GOOD)
				{
					if (the_debug == YES_DEBUG)
						printf("	ERROR in initDB() at line %d in %s\n", __LINE__, __FILE__);
					return RETURN_ERROR;
				}
            }*/
            // END This function


			// START If more tables to read, allocate another struct table_info
			if (cur_alloc < num_tables-1)
			{
				cur->next = (struct table_info*) myMalloc(sizeof(struct table_info), &file_opened_head, malloced_head, the_debug);
				if (cur->next == NULL)
				{
					if (the_debug == YES_DEBUG)
						printf("	ERROR in initDB() at line %d in %s\n", __LINE__, __FILE__);
					return RETURN_ERROR;
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

	myFileClose(db_info, &file_opened_head, malloced_head, the_debug);

    // Keep this since mallocing things that should be persistent after the operation
	if (the_debug == YES_DEBUG)
		printf("Calling myFreeAllCleanup() from initDB(), but NOT freeing ptrs for tables_head\n");
	myFreeAllCleanup(malloced_head, the_debug);

    if (file_opened_head != NULL)
    {
        if (the_debug == YES_DEBUG)
			printf("initDB() did not close all files\n");
        myFileCloseAll(&file_opened_head, malloced_head, the_debug);
    }

	return RETURN_GOOD;
}

int copyDBfromSHM(struct shmem* shm, struct malloced_node** malloced_head, int the_debug)
{
	num_tables = shm->num_tables;

	if (shm->num_tables > 0)
	{
		tables_head = myMalloc(sizeof(struct table_info), NULL, malloced_head, the_debug);
		if (tables_head == NULL)
		{
			if (the_debug == YES_DEBUG)
				printf("	ERROR in initDB() at line %d in %s\n", __LINE__, __FILE__);
			return RETURN_ERROR;
		}
		struct table_info* cur = tables_head;


		for (int i=0; i<shm->num_tables; i++)
		{
			cur->name = myMalloc(sizeof(char) * 32, NULL, malloced_head, the_debug);
			if (cur->name == NULL)
			{
				if (the_debug == YES_DEBUG)
					printf("	ERROR in initDB() at line %d in %s\n", __LINE__, __FILE__);
				return RETURN_ERROR;
			}
			strcpy(cur->name, shm->shm_table_info_arr[i].name);

			cur->keyword = myMalloc(sizeof(char) * 32, NULL, malloced_head, the_debug);
			if (cur->keyword == NULL)
			{
				if (the_debug == YES_DEBUG)
					printf("	ERROR in initDB() at line %d in %s\n", __LINE__, __FILE__);
				return RETURN_ERROR;
			}
			strcpy(cur->keyword, shm->shm_table_info_arr[i].keyword);

			cur->file_number = shm->shm_table_info_arr[i].file_number;
			cur->num_cols = shm->shm_table_info_arr[i].num_cols;

			cur->next = NULL;


			if (shm->shm_table_info_arr[i].num_cols > 0)
			{
				cur->table_cols_head = myMalloc(sizeof(struct table_cols_info), NULL, malloced_head, the_debug);
				if (cur->table_cols_head == NULL)
				{
					if (the_debug == YES_DEBUG)
						printf("	ERROR in initDB() at line %d in %s\n", __LINE__, __FILE__);
					return RETURN_ERROR;
				}
				struct table_cols_info* cur_col = cur->table_cols_head;


				int column_index = shm->shm_table_info_arr[i].table_cols_starting_index;

				for (int j=0; j<shm->shm_table_info_arr[i].num_cols; j++)
				{
					cur_col->col_name = myMalloc(sizeof(char) * 32, NULL, malloced_head, the_debug);
					while (cur_col->col_name == NULL)
					{
						if (the_debug == YES_DEBUG)
							printf("	ERROR in initDB() at line %d in %s\n", __LINE__, __FILE__);
						return RETURN_ERROR;
					}
					strcpy(cur_col->col_name, shm->shm_table_cols_info_arr[column_index].col_name);

					cur_col->data_type = shm->shm_table_cols_info_arr[column_index].data_type;
					cur_col->max_length = shm->shm_table_cols_info_arr[column_index].max_length;
					cur_col->col_number = shm->shm_table_cols_info_arr[column_index].col_number;
					cur_col->num_rows = shm->shm_table_cols_info_arr[column_index].num_rows;
					cur_col->num_open = shm->shm_table_cols_info_arr[column_index].num_open;
					cur_col->home_table = cur;

					cur_col->open_list_head = NULL;
					cur_col->open_list_before_tail = NULL;
					cur_col->num_added_insert_open = 0;
					cur_col->unique_list_head = NULL;
					cur_col->unique_list_tail = NULL;
					cur_col->frequent_list_head = NULL;
					cur_col->frequent_list_tail = NULL;
					cur_col->frequent_arr_row_to_node = NULL;

					cur_col->next = NULL;


					if (j < shm->shm_table_info_arr[i].num_cols-1)
					{
						cur_col->next = (struct table_cols_info*) myMalloc(sizeof(struct table_cols_info), NULL, malloced_head, the_debug);
						if (cur_col->next == NULL)
						{
							if (the_debug == YES_DEBUG)
								printf("	ERROR in initDB() at line %d in %s\n", __LINE__, __FILE__);
							return RETURN_ERROR;
						}
						cur_col = cur_col->next;
					}

					column_index++;
				}
			}


			if (i < shm->num_tables-1)
			{
				cur->next = (struct table_info*) myMalloc(sizeof(struct table_info), NULL, malloced_head, the_debug);
				if (cur->next == NULL)
				{
					if (the_debug == YES_DEBUG)
						printf("	ERROR in initDB() at line %d in %s\n", __LINE__, __FILE__);
					return RETURN_ERROR;
				}
				cur = cur->next;
			}
		}
	}

	// Keep this since mallocing things that should be persistent after the operation
	if (the_debug == YES_DEBUG)
		printf("Calling myFreeAllCleanup() from initDB(), but NOT freeing ptrs for tables_head\n");
	myFreeAllCleanup(malloced_head, the_debug);

	return RETURN_GOOD;
}

int traverseTablesInfoMemory()
{
	printf("\nIN MEMORY\n\nnum_tables = %lu\n", num_tables);

	struct table_info* cur = tables_head;
	while (cur != NULL)
	{
		printf("Table name = %s\n", cur->name);
		printf("Keyword = %s\n", cur->keyword);
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
			/*if (cur_col->open_list_head != NULL)
			{
				struct ListNodePtr* cur_open;

				if (cur_col->open_list_before_tail == NULL && cur_col->open_list_head->next != NULL)
					cur_open = cur_col->open_list_head->next;
				else if (cur_col->open_list_before_tail == NULL && cur_col->open_list_head->next == NULL)
					cur_open = cur_col->open_list_head;
				else
					cur_open = cur_col->open_list_before_tail->next;

				traverseListNodesPtr(&cur_col->open_list_head, &cur_open, TRAVERSELISTNODES_HEAD, "	Open slots: ");

				traverseListNodesPtr(&cur_col->open_list_head, &cur_open, TRAVERSELISTNODES_TAIL, "	Open slots in reverse: ");
			}
			printf("	Open row before tail = %d\n", cur_col->open_list_before_tail == NULL ? -1 : *((int*) cur_col->open_list_before_tail->ptr_value));
			printf("	Number added to insert open list = %lu\n", cur_col->num_added_insert_open);


			printf("	frequent_arr_row_to_node: ");
			if (cur_col->frequent_arr_row_to_node != NULL)
			{
				for (int i=0; i<cur_col->num_rows; i++)
				{
					if (cur_col->frequent_arr_row_to_node[i] == NULL)
					{
						// Do nothing
					}
					else if (cur_col->frequent_arr_row_to_node[i]->ptr_value == NULL)
						printf("NULL,");
					else if (cur_col->data_type == DATA_INT)
						printf("%d,", cur_col->frequent_arr_row_to_node[i] == NULL ? -1 : *((int*) cur_col->frequent_arr_row_to_node[i]->ptr_value));
					else if (cur_col->data_type == DATA_REAL)
						printf("%lf,", cur_col->frequent_arr_row_to_node[i] == NULL ? -1.0 : *((double*) cur_col->frequent_arr_row_to_node[i]->ptr_value));
					else
						printf("%s,", cur_col->frequent_arr_row_to_node[i] == NULL ? "NULL" : cur_col->frequent_arr_row_to_node[i]->ptr_value);
				}
				printf("\n");
			}
			else
			{
				printf("is NULL\n");
			}


			printf("	unique_list_head:\n");
			struct frequent_node* cur = cur_col->unique_list_head;
			while (cur != NULL)
			{
				if (cur->ptr_value == NULL)
					printf("		ptr_value = NULL\n");
				else if (cur_col->data_type == DATA_INT)
					printf("		ptr_value = _%d_\n", *((int*) cur->ptr_value));
				else if (cur_col->data_type == DATA_REAL)
					printf("		ptr_value = _%lf_\n", *((double*) cur->ptr_value));
				else
					printf("		ptr_value = _%s_\n", cur->ptr_value);
				printf("		num_appearences = %d\n", cur->num_appearences);
				traverseListNodesPtr(&cur->row_nums_head, &cur->row_nums_tail, TRAVERSELISTNODES_HEAD, "		row_nums_head = ");

				cur = cur->next;
			}

			printf("	frequent_list_head:\n");
			cur = cur_col->frequent_list_head;
			while (cur != NULL)
			{
				if (cur->ptr_value == NULL)
					printf("		ptr_value = NULL\n");
				else if (cur_col->data_type == DATA_INT)
					printf("		ptr_value = _%d_\n", *((int*) cur->ptr_value));
				else if (cur_col->data_type == DATA_REAL)
					printf("		ptr_value = _%lf_\n", *((double*) cur->ptr_value));
				else
					printf("		ptr_value = _%s_\n", cur->ptr_value);
				printf("		num_appearences = %d\n", cur->num_appearences);
				traverseListNodesPtr(&cur->row_nums_head, &cur->row_nums_tail, TRAVERSELISTNODES_HEAD, "		row_nums_head = ");

				cur = cur->next;
			}*/

			printf("	Column home_table = %s\n", cur_col->home_table->name);

			cur_col = cur_col->next;
		}

		cur = cur->next;
	}
    printf("\n");
	return RETURN_GOOD;
}

/*	RETURNS:
 *  RETURN_ERROR if error
 *	RETURN_GOOD if good
 *
 *	WRITES TO:
 *	 
 *	Persistent malloced items: 
 *		struct table_info* (added to tables_head);
 *		struct table_cols_info* (passed from calling function);
 */
int createTable(char* table_name, struct table_cols_info* table_cols, char* keyword, struct malloced_node** malloced_head, int the_debug)
{
    if (the_debug == YES_DEBUG)
		printf("Calling createTable()\n");
    struct file_opened_node* file_opened_head = NULL;

	// START Allocate space for a new struct table_info
		struct table_info* table;
		if (tables_head == NULL)
		{
			if (the_debug == YES_DEBUG)
				printf("No tables in tables_head\n");
			tables_head = (struct table_info*) myMalloc(sizeof(struct table_info), &file_opened_head, malloced_head, the_debug);
			if (tables_head == NULL)
			{
				if (the_debug == YES_DEBUG)
					printf("	ERROR in createTable() at line %d in %s\n", __LINE__, __FILE__);
				return RETURN_ERROR;
			}
			table = tables_head;
		}
		else
		{
			if (the_debug == YES_DEBUG)
				printf("Found existing tables in tables_head\n");
			table = tables_head;
			while (table->next != NULL)
				table = table->next;

			table->next = (struct table_info*) myMalloc(sizeof(struct table_info), &file_opened_head, malloced_head, the_debug);
			if (table->next == NULL)
			{
				if (the_debug == YES_DEBUG)
					printf("	ERROR in createTable() at line %d in %s\n", __LINE__, __FILE__);
				return RETURN_ERROR;
			}
			table = table->next;
		}
	// END Allocate space for a new struct table_info

	// START Assign new struct table_info values
		table->name = table_name;
		table->keyword = keyword;
		table->file_number = getNextTableNum(malloced_head, the_debug);
		if (table->file_number == RETURN_ERROR)
		{
			if (the_debug == YES_DEBUG)
				printf("	ERROR in createTable() at line %d in %s\n", __LINE__, __FILE__);
			return RETURN_ERROR;
		}
		table->num_cols = 0;
		table->table_cols_head = table_cols;

		table->next = NULL;
	// END Assign new struct table_info values

	// START Append table name to DB_Info
		FILE* db_info_append = myFileOpen("_Info", -1, -1, "ab", &file_opened_head, malloced_head, the_debug);
		if (db_info_append == NULL)
		{
			if (the_debug == YES_DEBUG)
				printf("	ERROR in createTable() at line %d in %s\n", __LINE__, __FILE__);
			return RETURN_ERROR;
		}

		if (writeFileChar(db_info_append, -1, table_name, &file_opened_head, malloced_head, the_debug) != 0)
		{
			if (the_debug == YES_DEBUG)
				printf("	ERROR in createTable() at line %d in %s\n", __LINE__, __FILE__);
			return RETURN_ERROR;
		}
		if (writeFileChar(db_info_append, -1, keyword, &file_opened_head, malloced_head, the_debug) != 0)
		{
			if (the_debug == YES_DEBUG)
				printf("	ERROR in createTable() at line %d in %s\n", __LINE__, __FILE__);
			return RETURN_ERROR;
		}
		if (writeFileInt(db_info_append, -1, &table->file_number, &file_opened_head, malloced_head, the_debug) != 0)
		{
			if (the_debug == YES_DEBUG)
				printf("	ERROR in createTable() at line %d in %s\n", __LINE__, __FILE__);
			return RETURN_ERROR;
		}
		myFileClose(db_info_append, &file_opened_head, malloced_head, the_debug);
	// END Append table name to DB_Info

	// START Create and open tab_col file and append spacer for num cols
		FILE* tab_col_append = myFileOpen("_Tab_Col_", table->file_number, 0, "ab+", &file_opened_head, malloced_head, the_debug);
		if (tab_col_append == NULL)
		{
			if (the_debug == YES_DEBUG)
				printf("	ERROR in createTable() at line %d in %s\n", __LINE__, __FILE__);
			return RETURN_ERROR;
		}

		int_8 spacer_for_num_cols = 0;
		if (writeFileInt(tab_col_append, -1, &spacer_for_num_cols, &file_opened_head, malloced_head, the_debug) != 0)
		{
			if (the_debug == YES_DEBUG)
				printf("	ERROR in createTable() at line %d in %s\n", __LINE__, __FILE__);
			return RETURN_ERROR;
		}
	// END Create and open tab_col file and append spacer for num cols

	// START Append column info to tab_col and create col_data and col_data_info files
		struct table_cols_info* cur_col = table->table_cols_head;
		while (cur_col != NULL)
		{
			if (addColumn(tab_col_append, cur_col, table, malloced_head, the_debug) != RETURN_GOOD)
			{
				if (the_debug == YES_DEBUG)
					printf("	ERROR in createTable() at line %d in %s\n", __LINE__, __FILE__);
				return RETURN_ERROR;
			}
			
			table->num_cols++;

			cur_col->num_rows = 0;
			cur_col->num_open = 0;
			cur_col->open_list_head = NULL;
			cur_col->open_list_before_tail = NULL;
			cur_col->num_added_insert_open = 0;
			cur_col->unique_list_head = NULL;
			cur_col->unique_list_tail = NULL;
			cur_col->frequent_list_head = NULL;
			cur_col->frequent_list_tail = NULL;
			cur_col->frequent_arr_row_to_node = NULL;

			cur_col->home_table = table;

			cur_col = cur_col->next;
		}

		fflush(tab_col_append);
		myFileClose(tab_col_append, &file_opened_head, malloced_head, the_debug);
	// END Append column info to tab_col and create col_data and col_data_info files

	// START Write new number of columns to tab_col
		FILE* tab_col = myFileOpen("_Tab_Col_", table->file_number, 0, "rb+", &file_opened_head, malloced_head, the_debug);
		if (tab_col == NULL)
		{
			if (the_debug == YES_DEBUG)
				printf("	ERROR in createTable() at line %d in %s\n", __LINE__, __FILE__);
			return RETURN_ERROR;
		}

		if (writeFileInt(tab_col, 0, &table->num_cols, &file_opened_head, malloced_head, the_debug) != 0)
		{
			if (the_debug == YES_DEBUG)
				printf("	ERROR in createTable() at line %d in %s\n", __LINE__, __FILE__);
			return RETURN_ERROR;
		}
		myFileClose(tab_col, &file_opened_head, malloced_head, the_debug);
	// END Write new number of columns to tab_col

	// START Edit number of tables in DB_Info
		num_tables++;

		FILE* db_info = myFileOpen("_Info", -1, -1, "rb+", &file_opened_head, malloced_head, the_debug);
		if (db_info == NULL)
		{
			if (the_debug == YES_DEBUG)
				printf("	ERROR in createTable() at line %d in %s\n", __LINE__, __FILE__);
			return RETURN_ERROR;
		}

		int_8 temp_num_tables = readFileInt(db_info, 0, NO_TRAVERSE_DISK, &file_opened_head, malloced_head, the_debug);
		//printf("temp_num_tables = %lu\n", temp_num_tables);
		temp_num_tables++;

		if (writeFileInt(db_info, 0, &temp_num_tables, &file_opened_head, malloced_head, the_debug) != 0)
		//if (writeFileInt(db_info, 0, &num_tables, &file_opened_head, malloced_head, the_debug) != 0)
		{
			if (the_debug == YES_DEBUG)
				printf("	ERROR in createTable() at line %d in %s\n", __LINE__, __FILE__);
			return RETURN_ERROR;
		}
		myFileClose(db_info, &file_opened_head, malloced_head, the_debug);
	// END Edit number of tables in DB_Info


	// Keep this since mallocing things that should be persistent after the operation
    if (the_debug == YES_DEBUG)
		printf("Calling myFreeAllCleanup() from createTable(), but NOT freeing ptrs for tables_head\n");
	myFreeAllCleanup(malloced_head, the_debug);

    if (file_opened_head != NULL)
    {
        if (the_debug == YES_DEBUG)
			printf("createTable() did not close all files\n");
        myFileCloseAll(&file_opened_head, malloced_head, the_debug);
    }

	return RETURN_GOOD;
}

/*	RETURNS:
 *  RETURN_ERROR if error
 *	RETURN_GOOD if good
 *
 *	WRITES TO:
 */
int addColumn(FILE* tab_col_append, struct table_cols_info* cur_col, struct table_info* table, struct malloced_node** malloced_head, int the_debug)
{
	struct file_opened_node* file_opened_head = NULL;
	
	// START Append to tab_col_append file
	fwrite(cur_col->col_name, 32, 1, tab_col_append);

	int_8 datatype = cur_col->data_type + (cur_col->max_length << 8);

	fwrite(&datatype, 8, 1, tab_col_append);

	fwrite(&table->num_cols, 8, 1, tab_col_append);
	// END Append to tab_col_append file

	// START Create and append to col_data_info
	FILE* col_data_info = myFileOpen("_Col_Data_Info_", table->file_number, table->num_cols, "ab+", &file_opened_head, malloced_head, the_debug);
	if (col_data_info == NULL)
	{
		if (the_debug == YES_DEBUG)
			printf("	ERROR in addColumn() at line %d in %s\n", __LINE__, __FILE__);
		return RETURN_ERROR;
	}

	int_8 zero = 0;

	fwrite(&zero, 8, 1, col_data_info);

	fwrite(&zero, 8, 1, col_data_info);

	fflush(col_data_info);
	myFileClose(col_data_info, &file_opened_head, malloced_head, the_debug);
	// END Create and append to col_data_info

	// START Create col_data
	FILE* col_data = myFileOpen("_Col_Data_", table->file_number, table->num_cols, "ab+\0", &file_opened_head, malloced_head, the_debug);
	if (col_data == NULL)
	{
		if (the_debug == YES_DEBUG)
			printf("	ERROR in addColumn() at line %d in %s\n", __LINE__, __FILE__);
		return RETURN_ERROR;
	}
	myFileClose(col_data, &file_opened_head, malloced_head, the_debug);
	// END Create col_data

	if (file_opened_head != NULL)
	{
		if (the_debug == YES_DEBUG)
			printf("addColumn() did not close all files\n");
		myFileCloseAll(&file_opened_head, malloced_head, the_debug);
	}

	return RETURN_GOOD;
}

/*	RETURNS:
 *  RETURN_ERROR if error
 *	RETURN_GOOD if good
 *
 *	WRITES TO:
 */
int deleteTable(char* table_name, struct malloced_node** malloced_head, int the_debug)
{
	int_8 table_number = 0;
	int_8 temp_num_tables = 0;
	struct table_info* the_table = NULL;

	if (strcmp(tables_head->name, table_name) == 0)
	{
		table_number = tables_head->file_number;
		the_table = tables_head;

		tables_head = NULL;
	}
	else
	{
		temp_num_tables = 1;

		struct table_info* cur_table = tables_head;
		while (cur_table->next != NULL)
		{
			if (strcmp(cur_table->next->name, table_name) == 0)
			{
				table_number = cur_table->next->file_number;
				the_table = cur_table->next;

				cur_table->next = NULL;

				break;
			}

			temp_num_tables++;

			cur_table = cur_table->next;
		}
	}

	char file_name[100] = {0};

	struct table_cols_info* cur_col = the_table->table_cols_head;
	while (cur_col != NULL)
	{
		concatFileName(file_name, "_Col_Data_", table_number, cur_col->col_number);
		remove(file_name);

		file_name[0] = 0;

		concatFileName(file_name, "_Col_Data_Info_", table_number, cur_col->col_number);
		remove(file_name);

		cur_col = cur_col->next;
	}

	concatFileName(file_name, "_Tab_Col_", table_number, 0);
	remove(file_name);


	freeAnyLinkedList((void**) &the_table, PTR_TYPE_TABLE_INFO, NULL, NULL, the_debug);


	remove("DB_Files_2/DB_Info.bin");

	while (access("DB_Files_2/DB_Info.bin", F_OK) == 0) {}


	struct file_opened_node* file_opened_head = NULL;

	// START Recreate DB_Info
		FILE* db_info = myFileOpen("_Info", -1, -1, "ab+", &file_opened_head, malloced_head, the_debug);
		if (db_info == NULL)
		{
			if (the_debug == YES_DEBUG)
				printf("	ERROR in initDB() at line %d in %s\n", __LINE__, __FILE__);
			return RETURN_ERROR;
		}

		if (writeFileInt(db_info, -1, &temp_num_tables, &file_opened_head, malloced_head, the_debug) == -1)
		{
			if (the_debug == YES_DEBUG)
				printf("	ERROR in initDB() at line %d in %s\n", __LINE__, __FILE__);
			return RETURN_ERROR;
		}
	// END Recreate DB_Info

	// START Rewrite still existing tables info on disk
		struct table_info* cur_table = tables_head;
		while (cur_table != NULL)
		{
			if (writeFileChar(db_info, -1, cur_table->name, &file_opened_head, malloced_head, the_debug) == -1)
			{
				if (the_debug == YES_DEBUG)
					printf("	ERROR in initDB() at line %d in %s\n", __LINE__, __FILE__);
				return RETURN_ERROR;
			}

			if (writeFileChar(db_info, -1, cur_table->keyword, &file_opened_head, malloced_head, the_debug) == -1)
			{
				if (the_debug == YES_DEBUG)
					printf("	ERROR in initDB() at line %d in %s\n", __LINE__, __FILE__);
				return RETURN_ERROR;
			}

			if (writeFileInt(db_info, -1, &cur_table->file_number, &file_opened_head, malloced_head, the_debug) == -1)
			{
				if (the_debug == YES_DEBUG)
					printf("	ERROR in initDB() at line %d in %s\n", __LINE__, __FILE__);
				return RETURN_ERROR;
			}

			cur_table = cur_table->next;
		}
	// END Rewrite still existing tables info on disk

	myFileClose(db_info, &file_opened_head, malloced_head, the_debug);

	if (file_opened_head != NULL)
    {
        if (the_debug == YES_DEBUG)
			printf("createTable() did not close all files\n");
        myFileCloseAll(&file_opened_head, malloced_head, the_debug);
    }

    num_tables--;

    if (the_debug == YES_DEBUG)
			printf("Deleted table\n");

	return RETURN_GOOD;
}

/*	RETURNS:
 *  RETURN_ERROR if error
 *	The row number where the data was inserted if good
 *
 *	WRITES TO:
 */
int insertAppend(FILE** col_data_info_file_arr, FILE** col_data_file_arr, struct file_opened_node** file_opened_head
				,int_8 the_table_number, struct table_cols_info* the_col, void* data_ptr_value, struct malloced_node** malloced_head, int the_debug)
{
	//printf("Here A\n");
	// START Find or open col_data_info file for col_number
		FILE* col_data_info = col_data_info_file_arr[the_col->col_number];
		if (col_data_info == NULL)
		{
			if (the_debug == YES_DEBUG)
				printf("Had to input col_data_info file into array\n");
			col_data_info = myFileOpen("_Col_Data_Info_", the_table_number, the_col->col_number, "rb+", file_opened_head, malloced_head, the_debug);
			if (col_data_info == NULL)
			{
				if (the_debug == YES_DEBUG)
					printf("	ERROR in insertAppend() at line %d in %s\n", __LINE__, __FILE__);
				return RETURN_ERROR;
			}
			col_data_info_file_arr[the_col->col_number] = col_data_info;
		}
		col_data_info = col_data_info_file_arr[the_col->col_number];
	// END Find or open col_data_info file for col_number

	//printf("Here B\n");
	// START Find or open col_data file for table_number and col_number
		FILE* col_data = col_data_file_arr[the_col->col_number];
		if (col_data == NULL)
		{
			if (the_debug == YES_DEBUG)
				printf("Had to input col_data file into array\n");
			col_data = myFileOpen("_Col_Data_", the_table_number, the_col->col_number, "ab+", file_opened_head, malloced_head, the_debug);
			if (col_data == NULL)
			{
				if (the_debug == YES_DEBUG)
					printf("	ERROR in insertAppend() at line %d in %s\n", __LINE__, __FILE__);
				return RETURN_ERROR;
			}
			col_data_file_arr[the_col->col_number] = col_data;
		}
		col_data = col_data_file_arr[the_col->col_number];
	// END Find or open col_data file for table_number and col_number

	//printf("Here C\n");
	// START Append row_id to column
		if (writeFileInt(col_data, APPEND_OFFSET, &the_col->num_rows, file_opened_head, malloced_head, the_debug) != 0)
		{
			if (the_debug == YES_DEBUG)
				printf("	ERROR in insertAppend() at line %d in %s\n", __LINE__, __FILE__);
			return RETURN_ERROR;
		}
	// END Append row_id to column

	//printf("Here D\n");
	// START Append data to col_data
		if (the_col->data_type == DATA_INT)
		{
			if (data_ptr_value == NULL)
			{
				if (writeFileInt(col_data, APPEND_OFFSET, &null_int, file_opened_head, malloced_head, the_debug) != 0)
				{
					if (the_debug == YES_DEBUG)
						printf("	ERROR in insertAppend() at line %d in %s\n", __LINE__, __FILE__);
					return RETURN_ERROR;
				}
			}
			else
			{
				if (writeFileInt(col_data, APPEND_OFFSET, data_ptr_value, file_opened_head, malloced_head, the_debug) != 0)
				{
					if (the_debug == YES_DEBUG)
						printf("	ERROR in insertAppend() at line %d in %s\n", __LINE__, __FILE__);
					return RETURN_ERROR;
				}
			}
		}
		else if (the_col->data_type == DATA_REAL)
		{
			if (data_ptr_value == NULL)
			{
				if (writeFileDouble(col_data, APPEND_OFFSET, &null_double, file_opened_head, malloced_head, the_debug) != 0)
				{
					if (the_debug == YES_DEBUG)
						printf("	ERROR in insertAppend() at line %d in %s\n", __LINE__, __FILE__);
					return RETURN_ERROR;
				}
			}
			else
			{
				if (writeFileDouble(col_data, APPEND_OFFSET, data_ptr_value, file_opened_head, malloced_head, the_debug) != 0)
				{
					if (the_debug == YES_DEBUG)
						printf("	ERROR in insertAppend() at line %d in %s\n", __LINE__, __FILE__);
					return RETURN_ERROR;
				}
			}
		}
		else if (the_col->data_type == DATA_STRING)
		{
			if (data_ptr_value == NULL)
			{
				if (writeFileCharData(col_data, APPEND_OFFSET, &the_col->max_length, null_string, file_opened_head, malloced_head, the_debug) != 0)
				{
					if (the_debug == YES_DEBUG)
						printf("	ERROR in insertAppend() at line %d in %s\n", __LINE__, __FILE__);
					return RETURN_ERROR;
				}
			}
			else
			{
				if (writeFileCharData(col_data, APPEND_OFFSET, &the_col->max_length, data_ptr_value, file_opened_head, malloced_head, the_debug) != 0)
				{
					if (the_debug == YES_DEBUG)
						printf("	ERROR in insertAppend() at line %d in %s\n", __LINE__, __FILE__);
					return RETURN_ERROR;
				}
			}
		}
		else if (the_col->data_type == DATA_DATE)
		{
			if (data_ptr_value == NULL)
			{
				if (writeFileInt(col_data, APPEND_OFFSET, &null_int, file_opened_head, malloced_head, the_debug) != 0)
				{
					if (the_debug == YES_DEBUG)
						printf("	ERROR in insertAppend() at line %d in %s\n", __LINE__, __FILE__);
					return RETURN_ERROR;
				}
			}
			else
			{
				int_8 data_int_date = dateToInt(data_ptr_value);

				if (writeFileInt(col_data, APPEND_OFFSET, &data_int_date, file_opened_head, malloced_head, the_debug) != 0)
				{
					if (the_debug == YES_DEBUG)
						printf("	ERROR in insertAppend() at line %d in %s\n", __LINE__, __FILE__);
					return RETURN_ERROR;
				}
			}
		}
	// END Append data to col_data

	//printf("Here E\n");
	// START Increment num_rows in col_data_info
		the_col->num_rows++;

		if (writeFileInt(col_data_info, 0, &the_col->num_rows, file_opened_head, malloced_head, the_debug) != 0)
		{
			if (the_debug == YES_DEBUG)
				printf("	ERROR in insertAppend() at line %d in %s\n", __LINE__, __FILE__);
			return RETURN_ERROR;
		}
	// END Increment num_rows in col_data_info

	//printf("Here F\n");

	return the_col->num_rows-1;
}

/*	RETURNS:
 *  RETURN_ERROR if error
 *	The row number where the data was inserted if good
 *
 *	WRITES TO:
 */
int insertOpen(FILE** col_data_info_file_arr, FILE** col_data_file_arr, struct file_opened_node** file_opened_head
			  ,int_8 the_table_number, struct table_cols_info* the_col, void* data_ptr_value, struct malloced_node** malloced_head, int the_debug)
{
	// START Get last open row id from memory, free node in open list to reflect insertion
		int_8 last_open_id;
		if (the_col->open_list_before_tail != NULL)
		{
			//printf("Here A\n");
			//printf("the_col->open_list_before_tail = %d\n", the_col->open_list_before_tail->value);
			last_open_id = *((int_8*) the_col->open_list_before_tail->next->ptr_value);
			//printf("Here Aa\n");

			struct ListNodePtr* temp = the_col->open_list_before_tail->next;
			the_col->open_list_before_tail->next = NULL;
			//if (myFree((void**) &temp, file_opened_head, malloced_head, the_debug) != 0)
			//	printf("Didnt free\n");
			//free(temp->ptr_value);
			//free(temp);
			myFree((void**) &temp->ptr_value, NULL, NULL, the_debug);
			myFree((void**) &temp, NULL, NULL, the_debug);
			
			the_col->open_list_before_tail = the_col->open_list_before_tail->prev;
			//printf("now the_col->open_list_before_tail = %d\n", the_col->open_list_before_tail == NULL ? -1 : the_col->open_list_before_tail->value);
		}
		else
		{
			//printf("Here B\n");
			last_open_id = *((int_8*) the_col->open_list_head->ptr_value);

			struct ListNodePtr* temp = the_col->open_list_head;
			the_col->open_list_head = NULL;
			//if (myFree((void**) &temp, file_opened_head, malloced_head, the_debug) != 0)
			//	printf("Didnt free\n");
			//free(temp->ptr_value);
			//free(temp);
			myFree((void**) &temp->ptr_value, NULL, NULL, the_debug);
			myFree((void**) &temp, NULL, NULL, the_debug);
			//printf("now open_list_head is NULL\n");
		}
		if (the_debug == YES_DEBUG)
			printf("last_open_id = %lu\n", last_open_id);
	// END Get last open row id from memory, free node in open list to reflect insertion

	the_col->num_open--;


	// START Find or open col_data file for table_number and col_number
		FILE* col_data = col_data_file_arr[the_col->col_number];
	// END Find or open col_data file for table_number and col_number


	// START Overwrite row id with positive version
		//printf("Currently there = %lu\n", readFileInt(col_data, ((8+the_col->max_length)*last_open_id), NO_TRAVERSE_DISK, file_opened_head, malloced_head, the_debug));
		if (writeFileInt(col_data, ((8+the_col->max_length)*last_open_id), &last_open_id, file_opened_head, malloced_head, the_debug) != 0)
		{
			if (the_debug == YES_DEBUG)
				printf("	ERROR in insertOpen() at line %d in %s\n", __LINE__, __FILE__);
			return RETURN_ERROR;
		}
	// END Overwrite row id with positive version


	// START Overwrite row data with new value in col_data
		//printf("New offset = %lu\n", ((8+the_col->max_length)*last_open_id)+8);
		if (the_col->data_type == DATA_INT)
		{
			//printf("Currently there = %lu\n", readFileInt(col_data, ((8+the_col->max_length)*last_open_id)+8), NO_TRAVERSE_DISK, &file_opened_head, malloced_head, the_debug);
			if (data_ptr_value == NULL)
			{
				if (writeFileInt(col_data, ((8+the_col->max_length)*last_open_id)+8, &null_int, file_opened_head, malloced_head, the_debug) != 0)
				{
					if (the_debug == YES_DEBUG)
						printf("	ERROR in insertOpen() at line %d in %s\n", __LINE__, __FILE__);
					return RETURN_ERROR;
				}
			}
			else
			{
				if (writeFileInt(col_data, ((8+the_col->max_length)*last_open_id)+8, data_ptr_value, file_opened_head, malloced_head, the_debug) != 0)
				{
					if (the_debug == YES_DEBUG)
						printf("	ERROR in insertOpen() at line %d in %s\n", __LINE__, __FILE__);
					return RETURN_ERROR;
				}
			}
		}
		else if (the_col->data_type == DATA_REAL)
		{
			//printf("Currently there = %f\n", readFileDouble(col_data, ((8+the_col->max_length)*last_open_id)+8));
			if (data_ptr_value == NULL)
			{
				if (writeFileDouble(col_data, ((8+the_col->max_length)*last_open_id)+8, &null_double, file_opened_head, malloced_head, the_debug) != 0)
				{
					if (the_debug == YES_DEBUG)
						printf("	ERROR in insertOpen() at line %d in %s\n", __LINE__, __FILE__);
					return RETURN_ERROR;
				}
			}
			else
			{
				if (writeFileDouble(col_data, ((8+the_col->max_length)*last_open_id)+8, data_ptr_value, file_opened_head, malloced_head, the_debug) != 0)
				{
					if (the_debug == YES_DEBUG)
						printf("	ERROR in insertOpen() at line %d in %s\n", __LINE__, __FILE__);
					return RETURN_ERROR;
				}
			}
		}
		else if (the_col->data_type == DATA_STRING)
		{
			//char* char_data = readFileCharData(&malloced_head, col_data, ((8+the_col->max_length)*last_open_id)+8, &the_col->max_length, the_debug);
			//if (char_data == NULL)
			//{
			//	if (the_debug == YES_DEBUG)
			//		printf("	ERROR in insertOpen() at line %d in %s\n", __LINE__, __FILE__);
			//	return RETURN_ERROR;
			//}
			//printf("Currently there = %s\n", char_data);
			//myFree((void**) &char_data, &file_opened_head, malloced_head, the_debug);
			if (data_ptr_value == NULL)
			{
				if (writeFileCharData(col_data, ((8+the_col->max_length)*last_open_id)+8, &the_col->max_length, null_string, file_opened_head, malloced_head, the_debug) != 0)
				{
					if (the_debug == YES_DEBUG)
						printf("	ERROR in insertOpen() at line %d in %s\n", __LINE__, __FILE__);
					return RETURN_ERROR;
				}
			}
			else
			{
				if (writeFileCharData(col_data, ((8+the_col->max_length)*last_open_id)+8, &the_col->max_length, data_ptr_value, file_opened_head, malloced_head, the_debug) != 0)
				{
					if (the_debug == YES_DEBUG)
						printf("	ERROR in insertOpen() at line %d in %s\n", __LINE__, __FILE__);
					return RETURN_ERROR;
				}
			}
		}
		else if (the_col->data_type == DATA_DATE)
		{
			//printf("Currently there = %lu\n", readFileInt(col_data, ((8+the_col->max_length)*last_open_id)+8), NO_TRAVERSE_DISK, &file_opened_head, malloced_head, the_debug);
			if (data_ptr_value == NULL)
			{
				if (writeFileInt(col_data, ((8+the_col->max_length)*last_open_id)+8, &null_int, file_opened_head, malloced_head, the_debug) != 0)
				{
					if (the_debug == YES_DEBUG)
						printf("	ERROR in insertOpen() at line %d in %s\n", __LINE__, __FILE__);
					return RETURN_ERROR;
				}
			}
			else
			{
				int_8 data_int_date = dateToInt(data_ptr_value);

				if (writeFileInt(col_data, ((8+the_col->max_length)*last_open_id)+8, &data_int_date, file_opened_head, malloced_head, the_debug) != 0)
				{
					if (the_debug == YES_DEBUG)
						printf("	ERROR in insertOpen() at line %d in %s\n", __LINE__, __FILE__);
					return RETURN_ERROR;
				}
			}
		}
	// END Overwrite row data with new value in col_data

	return (int) last_open_id;
}

/*	RETURNS:
 *  RETURN_ERROR if error
 *	The number of rows in the table after insertion if good
 *
 *	WRITES TO:
 */
int insertRows(struct change_node_v2* change_head, struct malloced_node** malloced_head, int the_debug)
{
	int valid_rows_at_start = change_head->table->table_cols_head->num_rows - change_head->table->table_cols_head->num_open;

	struct file_opened_node* file_opened_head = NULL;

	// START Realloc frequent_arr_row_to_node for each column
		int new_amt_rows = change_head->total_rows_to_insert - change_head->table->table_cols_head->num_open;

		if (new_amt_rows > 0)
		{
			if (the_debug == YES_DEBUG)
				printf("Have to realloc frequent_arr_row_to_node\n");
			struct table_cols_info* cur_col = change_head->table->table_cols_head;
			while (cur_col != NULL)
			{
				struct frequent_node** temp_new = (struct frequent_node**) myMalloc(sizeof(struct frequent_node*) * (new_amt_rows + change_head->table->table_cols_head->num_rows)
																				   ,&file_opened_head, malloced_head, the_debug);
				if (temp_new == NULL)
				{
					if (the_debug == YES_DEBUG)
						printf("	ERROR in insertRows() at line %d in %s\n", __LINE__, __FILE__);
					return RETURN_ERROR;
				}

				for (int i=0; i<change_head->table->table_cols_head->num_rows; i++)
				{
					temp_new[i] = cur_col->frequent_arr_row_to_node[i];
				}

				for (int i=change_head->table->table_cols_head->num_rows; i<(new_amt_rows + change_head->table->table_cols_head->num_rows); i++)
				{
					temp_new[i] = NULL;
				}

				//free(cur_col->frequent_arr_row_to_node);
				myFree((void**) &cur_col->frequent_arr_row_to_node, NULL, NULL, the_debug);

				cur_col->frequent_arr_row_to_node = temp_new;


				cur_col = cur_col->next;
			}
		}
	// END Realloc frequent_arr_row_to_node for each column

    // START Allocate space for file arrays
	    FILE** col_data_info_file_arr = (FILE**) myMalloc(sizeof(FILE*) * change_head->table->num_cols, &file_opened_head, malloced_head, the_debug);
	    //printf("malloced\n");
		FILE** col_data_file_arr = (FILE**) myMalloc(sizeof(FILE*) * change_head->table->num_cols, &file_opened_head, malloced_head, the_debug);
		//printf("malloced\n");
	    if (col_data_info_file_arr == NULL || col_data_file_arr == NULL)
	    {
	        if (the_debug == YES_DEBUG)
				printf("	ERROR in insertRows() at line %d in %s\n", __LINE__, __FILE__);
	        return RETURN_ERROR;
	    }

		for (int i=0; i<change_head->table->num_cols; i++)
		{
			col_data_info_file_arr[i] = NULL;
			col_data_file_arr[i] = NULL;
		}
	// END Allocate space for file arrays

   	// START Init arr for keep track of which cols were insert to for each row
		bool* did_col = (bool*) myMalloc(sizeof(bool) * change_head->table->num_cols, &file_opened_head, malloced_head, the_debug);
		if (did_col == NULL)
		{
			if (the_debug == YES_DEBUG)
				printf("	ERROR in insertRows() at line %d in %s\n", __LINE__, __FILE__);
			return RETURN_ERROR;
		}

		for (int i=0; i<change_head->table->num_cols; i++)
			did_col[i] = false;

		int num_did_cols = 0;
	// END Init arr for keep track of which cols were insert to for each row

	// START Traverse change_head
		bool inserted_open = false;

		struct ListNodePtr* cur_data_col = change_head->col_list_head;
		struct ListNodePtr* cur_data = change_head->data_list_head;

		while (cur_data_col != NULL)
		{
			//if (cur_data->ptr_value == NULL)
			//	printf("New row\n");
			//else if (((struct table_cols_info*) cur_data_col->ptr_value)->data_type == DATA_INT)
			//	printf("inserting _%d_ for col _%s_\n", *((int*) cur_data->ptr_value), ((struct table_cols_info*) cur_data_col->ptr_value)->col_name);
			//else if (((struct table_cols_info*) cur_data_col->ptr_value)->data_type == DATA_REAL)
			//	printf("inserting _%lf_ for col _%s_\n", *((double*) cur_data->ptr_value), ((struct table_cols_info*) cur_data_col->ptr_value)->col_name);
			//else
			//	printf("inserting _%s_ for col _%s_\n", cur_data->ptr_value, ((struct table_cols_info*) cur_data_col->ptr_value)->col_name);

			if (cur_data_col->ptr_value == NULL)
			{
				// START Check if inserted for all rows or if some were missed and need to be inserted as NULL
					if (num_did_cols > 0 && num_did_cols < change_head->table->num_cols)
					{
						if (the_debug == YES_DEBUG)
							printf("Have to insert some nulls at the end of one row\n");
						struct table_cols_info* cur_col = change_head->table->table_cols_head;
						for (int i=0; i<change_head->table->num_cols; i++)
						{
							if (!did_col[i])
							{
								if (the_debug == YES_DEBUG)
									printf("Inserting null for col _%s_\n", cur_col->col_name);

								struct frequent_node* freq_head = NULL;
								struct frequent_node* freq_tail = NULL;

								did_col[i] = true;
								num_did_cols++;

								int the_row_id;
								
								if (cur_col->num_open > 0)
								{
									if (col_data_file_arr[i] == NULL)
										col_data_file_arr[i] = myFileOpen("_Col_Data_", change_head->table->file_number, i, "rb+", &file_opened_head, malloced_head, the_debug);

									the_row_id = insertOpen(col_data_info_file_arr, col_data_file_arr, &file_opened_head, change_head->table->file_number, cur_col
									  						   ,NULL, malloced_head, the_debug);
									if (the_row_id < 0)
									{
										if (the_debug == YES_DEBUG)
											printf("	ERROR in insertRows() at line %d in %s\n", __LINE__, __FILE__);
								        return RETURN_ERROR;
									}
								}
								else
								{
									if (col_data_file_arr[i] == NULL)
										col_data_file_arr[i] = myFileOpen("_Col_Data_", change_head->table->file_number, i, "ab+", &file_opened_head, malloced_head, the_debug);

									the_row_id = insertAppend(col_data_info_file_arr, col_data_file_arr, &file_opened_head, change_head->table->file_number, cur_col
								  							 ,NULL, malloced_head, the_debug);
									if (the_row_id < 0)
									{
										if (the_debug == YES_DEBUG)
											printf("	ERROR in insertRows() at line %d in %s\n", __LINE__, __FILE__);
								        return RETURN_ERROR;
									}
								}

								if (the_debug == YES_DEBUG)
									printf("the_row_id = %d\n", the_row_id);

								if (addFreqNodeToTempNewList(FREQ_LIST_ADD_INSERT, &freq_head, &freq_tail, NULL, &the_row_id, cur_col, &file_opened_head, malloced_head, the_debug) != RETURN_GOOD)
								{
									if (the_debug == YES_DEBUG)
										printf("	ERROR in insertRows() at line %d in %s\n", __LINE__, __FILE__);
							        return RETURN_ERROR;
								}

								addFreqNodeToFreqLists(freq_head, cur_col, &file_opened_head, malloced_head, the_debug);
							}

							cur_col = cur_col->next;
						}
					}
				// END Check if inserted for all rows or if some were missed and need to be inserted as NULL
					
				// START Reset for new row
					struct table_cols_info* cur_col = change_head->table->table_cols_head;
					for (int i=0; i<change_head->table->num_cols; i++)
					{
						did_col[i] = false;

						if (cur_col->num_open == 0 && inserted_open)
						{
							myFileClose(col_data_file_arr[i], &file_opened_head, malloced_head, the_debug);
							col_data_file_arr[i] = NULL;
						}

						cur_col = cur_col->next;
					}

					num_did_cols = 0;
				// END Reset for new row
			}
			else if (cur_data_col->ptr_value != NULL)
			{
				struct frequent_node* freq_head = NULL;
				struct frequent_node* freq_tail = NULL;

				did_col[((struct table_cols_info*) cur_data_col->ptr_value)->col_number] = true;
				num_did_cols++;

				int the_row_id;

				if (((struct table_cols_info*) cur_data_col->ptr_value)->num_open > 0)
				{
					// START Insert open
						inserted_open = true;

						if (col_data_file_arr[((struct table_cols_info*) cur_data_col->ptr_value)->col_number] == NULL)
							col_data_file_arr[((struct table_cols_info*) cur_data_col->ptr_value)->col_number] = myFileOpen("_Col_Data_", change_head->table->file_number, ((struct table_cols_info*) cur_data_col->ptr_value)->col_number, "rb+", &file_opened_head, malloced_head, the_debug);


						the_row_id = insertOpen(col_data_info_file_arr, col_data_file_arr, &file_opened_head, change_head->table->file_number, cur_data_col->ptr_value
						  						   ,cur_data->ptr_value, malloced_head, the_debug);
						if (the_row_id < 0)
						{
							if (the_debug == YES_DEBUG)
								printf("	ERROR in insertRows() at line %d in %s\n", __LINE__, __FILE__);
							return RETURN_ERROR;
						}
					// END Insert open
				}
				else
				{
					// START Insert append
						if (col_data_file_arr[((struct table_cols_info*) cur_data_col->ptr_value)->col_number] == NULL)
							col_data_file_arr[((struct table_cols_info*) cur_data_col->ptr_value)->col_number] = myFileOpen("_Col_Data_", change_head->table->file_number, ((struct table_cols_info*) cur_data_col->ptr_value)->col_number, "ab+", &file_opened_head, malloced_head, the_debug);

						the_row_id = insertAppend(col_data_info_file_arr, col_data_file_arr, &file_opened_head, change_head->table->file_number, cur_data_col->ptr_value
						  						 ,cur_data->ptr_value, malloced_head, the_debug);
						if (the_row_id < 0)
						{
							if (the_debug == YES_DEBUG)
								printf("	ERROR in insertRows() at line %d in %s\n", __LINE__, __FILE__);
							return RETURN_ERROR;
						}
					// END Insert append
				}

				if (the_debug == YES_DEBUG)
					printf("the_row_id = %d\n", the_row_id);

				if (addFreqNodeToTempNewList(FREQ_LIST_ADD_INSERT, &freq_head, &freq_tail, cur_data, &the_row_id, cur_data_col->ptr_value, &file_opened_head, malloced_head, the_debug) != RETURN_GOOD)
				{
					if (the_debug == YES_DEBUG)
						printf("	ERROR in insertRows() at line %d in %s\n", __LINE__, __FILE__);
					return RETURN_ERROR;
				}

				addFreqNodeToFreqLists(freq_head, cur_data_col->ptr_value, &file_opened_head, malloced_head, the_debug);
			}

			cur_data_col = cur_data_col->next;
			cur_data = cur_data->next;
		}

		// START Check if inserted for all rows or if some were missed and need to be inserted as NULL
			if (num_did_cols > 0 && num_did_cols < change_head->table->num_cols)
			{
				if (the_debug == YES_DEBUG)
					printf("Have to insert some nulls at the end of change_head->col_list_head\n");

				struct table_cols_info* cur_col = change_head->table->table_cols_head;
				for (int i=0; i<change_head->table->num_cols; i++)
				{
					if (!did_col[i])
					{
						if (the_debug == YES_DEBUG)
							printf("Inserting null for col _%s_\n", cur_col->col_name);

						struct frequent_node* freq_head = NULL;
						struct frequent_node* freq_tail = NULL;

						did_col[i] = true;
						num_did_cols++;

						int the_row_id;
						
						if (cur_col->num_open > 0)
						{
							if (col_data_file_arr[i] == NULL)
								col_data_file_arr[i] = myFileOpen("_Col_Data_", change_head->table->file_number, i, "rb+", &file_opened_head, malloced_head, the_debug);

							the_row_id = insertOpen(col_data_info_file_arr, col_data_file_arr, &file_opened_head, change_head->table->file_number, cur_col
							  						   ,NULL, malloced_head, the_debug);
							if (the_row_id < 0)
							{
								if (the_debug == YES_DEBUG)
									printf("	ERROR in insertRows() at line %d in %s\n", __LINE__, __FILE__);
						        return RETURN_ERROR;
							}
						}
						else
						{
							if (col_data_file_arr[i] == NULL)
								col_data_file_arr[i] = myFileOpen("_Col_Data_", change_head->table->file_number, i, "ab+", &file_opened_head, malloced_head, the_debug);

							the_row_id = insertAppend(col_data_info_file_arr, col_data_file_arr, &file_opened_head, change_head->table->file_number, cur_col
							  							 ,NULL, malloced_head, the_debug);
							if (the_row_id < 0)
							{
								if (the_debug == YES_DEBUG)
									printf("	ERROR in insertRows() at line %d in %s\n", __LINE__, __FILE__);
						        return RETURN_ERROR;
							}
						}

						if (the_debug == YES_DEBUG)
							printf("the_row_id = %d\n", the_row_id);

						if (addFreqNodeToTempNewList(FREQ_LIST_ADD_INSERT, &freq_head, &freq_tail, NULL, &the_row_id, cur_col, &file_opened_head, malloced_head, the_debug) != RETURN_GOOD)
						{
							if (the_debug == YES_DEBUG)
								printf("	ERROR in insertRows() at line %d in %s\n", __LINE__, __FILE__);
					        return RETURN_ERROR;
						}

						addFreqNodeToFreqLists(freq_head, cur_col, &file_opened_head, malloced_head, the_debug);
					}

					cur_col = cur_col->next;
				}
			}
		// END Check if inserted for all rows or if some were missed and need to be inserted as NULL
	// END Traverse change_head


	// START Rewrite col_data_info for each column
		struct table_cols_info* cur_col = change_head->table->table_cols_head;
		for (int i=0; i<change_head->table->num_cols; i++)
		{
			if (col_data_file_arr[cur_col->col_number] != NULL)
			{
				myFileClose(col_data_file_arr[cur_col->col_number], &file_opened_head, malloced_head, the_debug);
				col_data_file_arr[cur_col->col_number] = NULL;
			}

			if (col_data_info_file_arr[cur_col->col_number] != NULL)
			{
				myFileClose(col_data_info_file_arr[cur_col->col_number], &file_opened_head, malloced_head, the_debug);
				col_data_info_file_arr[cur_col->col_number] = NULL;
			}

			// START Delete and rewrite col_data_info to reflect inserted rows
			char* file_name = (char*) myMalloc(sizeof(char) * 64, &file_opened_head, malloced_head, the_debug);
			//printf("malloced\n");
			if (file_name == NULL)
			{
				if (the_debug == YES_DEBUG)
					printf("	ERROR in insertRows() at line %d in %s\n", __LINE__, __FILE__);
				return RETURN_ERROR;
			}
			concatFileName(file_name, "_Col_Data_Info_", change_head->table->file_number, cur_col->col_number);

			remove(file_name);
			myFree((void**) &file_name, &file_opened_head, malloced_head, the_debug);
			//printf("freed\n");

			//printf("Here 4\n");

			FILE* col_data_info = myFileOpen("_Col_Data_Info_", change_head->table->file_number, cur_col->col_number, "ab+", &file_opened_head, malloced_head, the_debug);
			if (col_data_info == NULL)
			{
				if (the_debug == YES_DEBUG)
					printf("	ERROR in insertRows() at line %d in %s\n", __LINE__, __FILE__);
				return RETURN_ERROR;
			}

			writeFileInt(col_data_info, -1, &cur_col->num_rows, &file_opened_head, malloced_head, the_debug);
			writeFileInt(col_data_info, -1, &cur_col->num_open, &file_opened_head, malloced_head, the_debug);

			//printf("Here 5\n");

			struct ListNodePtr* cur_open = cur_col->open_list_head;
			while (cur_open != NULL)
			{
				if (writeFileInt(col_data_info, -1, cur_open->ptr_value, &file_opened_head, malloced_head, the_debug) != 0)
				{
					if (the_debug == YES_DEBUG)
						printf("	ERROR in insertRows() at line %d in %s\n", __LINE__, __FILE__);
					return RETURN_ERROR;
				}

				cur_open = cur_open->next;
			}

			myFileClose(col_data_info, &file_opened_head, malloced_head, the_debug);

			cur_col = cur_col->next;
		}
	// END Rewrite col_data_info for each column


	// START Free frequent_arr_row_to_node for each column if realloced
		if (new_amt_rows > 0)
		{
			if (the_debug == YES_DEBUG)
				printf("myFreeJustNode frequent_arr_row_to_node\n");
			struct table_cols_info* cur_col = change_head->table->table_cols_head;
			while (cur_col != NULL)
			{
				myFreeJustNode((void**) &cur_col->frequent_arr_row_to_node, &file_opened_head, malloced_head, the_debug);

				cur_col = cur_col->next;
			}
		}
	// END Free frequent_arr_row_to_node for each column if realloced
	

	myFree((void**) &col_data_info_file_arr, &file_opened_head, malloced_head, the_debug);
	myFree((void**) &col_data_file_arr, &file_opened_head, malloced_head, the_debug);
	myFree((void**) &did_col, &file_opened_head, malloced_head, the_debug);

	// START Cleanup
	    if (file_opened_head != NULL)
	    {
	        if (the_debug == YES_DEBUG)
				printf("insertRows() did not close all files\n");
	        myFileCloseAll(&file_opened_head, malloced_head, the_debug);
	    }
    // END Cleanup

    int valid_rows_at_end = change_head->table->table_cols_head->num_rows - change_head->table->table_cols_head->num_open;

    return valid_rows_at_end - valid_rows_at_start;
}

/*	RETURNS:
 *  RETURN_ERROR if error
 *	The number of rows deleted if good
 *
 *	WRITES TO:
 */
int deleteRows(struct change_node_v2* change_head, struct malloced_node** malloced_head, int the_debug)
{
	struct file_opened_node* file_opened_head = NULL;


	struct ListNodePtr* tab_cols_info_head = NULL;
	struct ListNodePtr* tab_cols_info_tail = NULL;

	if (change_head->where_head != NULL)
	{
		// START Get all column ptrs from where clause
			if (getAllColsFromWhereNode(&tab_cols_info_head, &tab_cols_info_tail, change_head->where_head, NULL, NULL, malloced_head, the_debug) != RETURN_GOOD)
			{
				if (the_debug == YES_DEBUG)
					printf("	ERROR in deleteRows() at line %d in %s\n", __LINE__, __FILE__);
				return RETURN_ERROR;
			}

			bool check_for_dups_arr[change_head->table->num_cols];
			for (int i=0; i<change_head->table->num_cols; i++)
			{
				check_for_dups_arr[i] = false;
			}

			struct ListNodePtr* cur = tab_cols_info_head;
			while (cur != NULL)
			{
				if (!check_for_dups_arr[((struct table_cols_info*) cur->ptr_value)->col_number])
				{
					check_for_dups_arr[((struct table_cols_info*) cur->ptr_value)->col_number] = true;

					struct table_cols_info* temp_col_ptr = cur->ptr_value;

					cur->ptr_type = ((struct table_cols_info*) cur->ptr_value)->col_number;

					cur->ptr_value = (void*) getAllColData(change_head->table->file_number, temp_col_ptr, NULL, -1, malloced_head, the_debug);
					if (cur->ptr_value == NULL)
					{
						if (the_debug == YES_DEBUG)
							printf("	ERROR in deleteRows() at line %d in %s\n", __LINE__, __FILE__);
						return RETURN_ERROR;
					}
				}
				else
				{
					cur->ptr_value = NULL;
					cur->ptr_type = -1;
				}
					

				cur = cur->next;
			}
		// END Get all column ptrs from where clause
	}


	int_8 num_rows_in_result = 0;
	struct ListNodePtr* valid_rows_head;
	struct ListNodePtr* valid_rows_tail;
	if (findValidRowsGivenWhere(&valid_rows_head, &valid_rows_tail, NULL, change_head->table, change_head->where_head, &num_rows_in_result, tab_cols_info_head, tab_cols_info_tail
								,false, -1, -1, malloced_head, the_debug) != RETURN_GOOD)
	{
		if (the_debug == YES_DEBUG)
			printf("	ERROR in deleteRows() at line %d in %s\n", __LINE__, __FILE__);
		return RETURN_ERROR;
	}

	if (the_debug == YES_DEBUG)
	{
		printf("num rows to be deleted = %lu\n", num_rows_in_result);
		traverseListNodesPtr(&valid_rows_head, &valid_rows_tail, TRAVERSELISTNODES_HEAD, "Rows to delete: ");
	}

	if (num_rows_in_result > 0)
	{
		// START Add deleted rows to disk
			struct table_cols_info* cur_col = change_head->table->table_cols_head;
			while (cur_col != NULL)
			{
				// START Append open row ids to col_data_info
					if (the_debug == YES_DEBUG)
						printf("Appending %d to col_data_info and updating num_open\n", *((int*) valid_rows_head->ptr_value));

					FILE* col_data_info = myFileOpen("_Col_Data_Info_", change_head->table->file_number, cur_col->col_number, "ab+", &file_opened_head, malloced_head, the_debug);
					if (col_data_info == NULL)
					{
						if (the_debug == YES_DEBUG)
							printf("	ERROR in deleteRows() at line %d in %s\n", __LINE__, __FILE__);
						return RETURN_ERROR;
					}

					struct ListNodePtr* cur_open = valid_rows_head;

					while (cur_open != NULL)
					{
						if (writeFileInt(col_data_info, -1, cur_open->ptr_value, &file_opened_head, malloced_head, the_debug) != 0)
						{
							if (the_debug == YES_DEBUG)
								printf("	ERROR in deleteRows() at line %d in %s\n", __LINE__, __FILE__);
							return RETURN_ERROR;
						}

						cur_col->num_open++;

						cur_open = cur_open->next;
					}

					myFileClose(col_data_info, &file_opened_head, malloced_head, the_debug);
				// END Append open row ids to col_data_info

				// START Overwrite new num_open value to col_data_info
					col_data_info = myFileOpen("_Col_Data_Info_", change_head->table->file_number, cur_col->col_number, "rb+", &file_opened_head, malloced_head, the_debug);

					if (writeFileInt(col_data_info, 8, &cur_col->num_open, &file_opened_head, malloced_head, the_debug) != 0)
					{
						if (the_debug == YES_DEBUG)
							printf("	ERROR in deleteRows() at line %d in %s\n", __LINE__, __FILE__);
						return RETURN_ERROR;
					}

					myFileClose(col_data_info, &file_opened_head, malloced_head, the_debug);
				// END Overwrite new num_open value to col_data_info
				
				cur_col = cur_col->next;
			}
		// END Add deleted rows to disk

		// START Delete rows from col_data
			cur_col = change_head->table->table_cols_head;
			while (cur_col != NULL)
			{
				if (the_debug == YES_DEBUG)
					printf("Marking row_id %d as open\n", *((int*) valid_rows_head->ptr_value));

				FILE* col_data = myFileOpen("_Col_Data_", change_head->table->file_number, cur_col->col_number, "rb+", &file_opened_head, malloced_head, the_debug);
				if (col_data == NULL)
				{
					if (the_debug == YES_DEBUG)
						printf("	ERROR in deleteRows() at line %d in %s\n", __LINE__, __FILE__);
					return RETURN_ERROR;
				}

				struct ListNodePtr* cur_open = valid_rows_head;
				while (cur_open != NULL)
				{
					// START Overwrite row id with negative version
						int int_version = *((int*) cur_open->ptr_value);
						if (int_version == 0)
							int_version = 1;

						int_version = int_version * (-1);

						int_8 new_id = (int_8) int_version;

						//if (the_debug == YES_DEBUG)
						//{
						//	printf("Changing row_id = %d on disk to %d\n", *((int_8*) cur_open->ptr_value), new_id);
						//	printf("Currently there = %lu\n", readFileInt(col_data, ((8+cur_col->max_length)*(*((int_8*) cur_open->ptr_value))), NO_TRAVERSE_DISK, &file_opened_head, malloced_head, the_debug));
						//}

						if (writeFileInt(col_data, ((8+cur_col->max_length)*(*((int_8*) cur_open->ptr_value))), &new_id, &file_opened_head, malloced_head, the_debug) != 0)
						{
							if (the_debug == YES_DEBUG)
								printf("	ERROR in deleteRows() at line %d in %s\n", __LINE__, __FILE__);
							return RETURN_ERROR;
						}
					// END Overwrite row id with negative version


					//if (removeFreqNodeFromFreqLists(cur_open->ptr_value, cur_col, the_debug) != RETURN_GOOD)
					//{
					//	if (the_debug == YES_DEBUG)
					//		printf("	ERROR in deleteRows() at line %d in %s\n", __LINE__, __FILE__);
					//	return RETURN_ERROR;
					//}


					cur_open = cur_open->next;
				}
				myFileClose(col_data, &file_opened_head, malloced_head, the_debug);


				cur_col = cur_col->next;
			}
		// END Delete rows from col_data

		// START Add deleted rows to memory
			cur_col = change_head->table->table_cols_head;
			while (cur_col != NULL)
			{
				if (the_debug == YES_DEBUG)
					printf("Adding row_id %d to memory\n", *((int*) valid_rows_head->ptr_value));

				struct ListNodePtr* cur_open = valid_rows_head;

				struct ListNodePtr* open_tail;

				if (cur_col->open_list_head == NULL)
				{
					//printf("Init cur_col->open_list_head\n");
					cur_col->open_list_head = (struct ListNodePtr*) myMalloc(sizeof(struct ListNodePtr), &file_opened_head, malloced_head, the_debug);
					if (cur_col->open_list_head == NULL)
					{
						if (the_debug == YES_DEBUG)
							printf("	ERROR in deleteRows() at line %d in %s\n", __LINE__, __FILE__);
						return RETURN_ERROR;
					}

					cur_col->open_list_head->ptr_value = (int_8*) myMalloc(sizeof(int_8), &file_opened_head, malloced_head, the_debug);
					if (cur_col->open_list_head->ptr_value == NULL)
					{
						if (the_debug == YES_DEBUG)
							printf("	ERROR in deleteRows() at line %d in %s\n", __LINE__, __FILE__);
						return RETURN_ERROR;
					}

					*((int_8*) cur_col->open_list_head->ptr_value) = *((int_8*) cur_open->ptr_value);
					cur_col->open_list_head->ptr_type = PTR_TYPE_INT;
					cur_col->open_list_head->next = NULL;
					cur_col->open_list_head->prev = NULL;

					cur_open = cur_open->next;
					open_tail = cur_col->open_list_head;
				}
				else
				{
					if (cur_col->open_list_head->next == NULL)
						open_tail = cur_col->open_list_head;
					else
						open_tail = cur_col->open_list_before_tail->next;
				}
				
				while (cur_open != NULL)
				{
					//printf("Adding another to open_list\n");
					cur_col->open_list_before_tail = open_tail;

					open_tail->next = (struct ListNodePtr*) myMalloc(sizeof(struct ListNodePtr), &file_opened_head, malloced_head, the_debug);
					if (cur_col->open_list_head == NULL)
					{
						if (the_debug == YES_DEBUG)
							printf("	ERROR in deleteRows() at line %d in %s\n", __LINE__, __FILE__);
						return RETURN_ERROR;
					}

					open_tail->next->ptr_value = (int_8*) myMalloc(sizeof(int_8), &file_opened_head, malloced_head, the_debug);
					if (open_tail->next->ptr_value == NULL)
					{
						if (the_debug == YES_DEBUG)
							printf("	ERROR in deleteRows() at line %d in %s\n", __LINE__, __FILE__);
						return RETURN_ERROR;
					}

					*((int_8*) open_tail->next->ptr_value) = *((int_8*) cur_open->ptr_value);
					open_tail->next->ptr_type = PTR_TYPE_INT;
					open_tail->next->next = NULL;
					open_tail->next->prev = open_tail;
					open_tail = open_tail->next;

					cur_open = cur_open->next;
				}

				//traverseListNodesPtr(&cur_col->open_list_head, NULL, TRAVERSELISTNODES_HEAD, "Open list: ");

				cur_col = cur_col->next;
			}
		// END Add deleted rows to memory

		// START Free returned linked list
			int freed = freeAnyLinkedList((void**) &valid_rows_head, PTR_TYPE_LIST_NODE_PTR, &file_opened_head, malloced_head, the_debug);
			if (the_debug == YES_DEBUG)
				printf("	Freed %d ListNodes from findValidRowsGivenWhere()\n", freed);
		// END Free returned linked list
	}


	// START Just free the malloced_ndes for the open_list because those persist
		struct table_cols_info* cur_col = change_head->table->table_cols_head;
		while (cur_col != NULL)
		{
			struct ListNodePtr* cur = cur_col->open_list_head;
			while (cur != NULL)
			{
				myFreeJustNode((void**) &cur->ptr_value, &file_opened_head, malloced_head, the_debug);
				myFreeJustNode((void**) &cur, &file_opened_head, malloced_head, the_debug);

				cur = cur->next;
			}
			cur_col = cur_col->next;
		}
	// END Just free the malloced_ndes for the open_list because those persist


	// START If table has no valid rows, delete data files and recreate them to regain space
		if (change_head->table->table_cols_head->num_open == change_head->table->table_cols_head->num_rows)
		{
			struct table_cols_info* cur_col = change_head->table->table_cols_head;
			while (cur_col != NULL)
			{
				char* file_name = (char*) myMalloc(sizeof(char) * 100, &file_opened_head, malloced_head, the_debug);
				if (file_name == NULL)
				{
					if (the_debug == YES_DEBUG)
						printf("	ERROR in deleteRows() at line %d in %s\n", __LINE__, __FILE__);
					return RETURN_ERROR;
				}

				concatFileName(file_name, "_Col_Data_", change_head->table->file_number, cur_col->col_number);

				//printf("file_name = _%s_\n", file_name);

				remove(file_name);


				strcpy(file_name, "");

				concatFileName(file_name, "_Col_Data_Info_", change_head->table->file_number, cur_col->col_number);

				//printf("file_name = _%s_\n", file_name);

				remove(file_name);

				myFree((void**) &file_name, &file_opened_head, malloced_head, the_debug);

				cur_col->num_rows = 0;
				cur_col->num_open = 0;


				// DONT Pass malloced_head because those malloced nodes were already freed from malloced_head so need to be freed with free()
				int freed = freeAnyLinkedList((void**) &cur_col->open_list_head, PTR_TYPE_LIST_NODE_PTR, NULL, NULL, the_debug);
				cur_col->open_list_head = NULL;
				cur_col->open_list_before_tail = NULL;
				if (the_debug == YES_DEBUG) 
					printf("	Freed %d from open_list_head\n", freed);


				// START Free frequent lists
				freed = 0;

				while (cur_col->unique_list_head != NULL)
				{
					struct frequent_node* temp = cur_col->unique_list_head;

					cur_col->unique_list_head = cur_col->unique_list_head->next;

					//printf("	ptr_value = _%s_\n", temp->ptr_value);
					//printf("	num_appearences = %d\n", temp->num_appearences);
					//traverseListNodes(&temp->row_nums_head, &temp->row_nums_tail, TRAVERSELISTNODES_HEAD, "	row_nums_head = ");

					//free(temp->ptr_value);
					myFree((void**) &temp->ptr_value, NULL, NULL, the_debug);
					freed++;
					freed += freeAnyLinkedList((void**) &temp->row_nums_head, PTR_TYPE_LIST_NODE_PTR, NULL, NULL, the_debug);
					//free(temp);
					myFree((void**) &temp, NULL, NULL, the_debug);
					freed++;
				}

				//printf("frequent_list_head:\n");
				while (cur_col->frequent_list_head != NULL)
				{
					struct frequent_node* temp = cur_col->frequent_list_head;

					cur_col->frequent_list_head = cur_col->frequent_list_head->next;

					//printf("	ptr_value = _%s_\n", temp->ptr_value);
					//printf("	num_appearences = %d\n", temp->num_appearences);
					//traverseListNodes(&temp->row_nums_head, &temp->row_nums_tail, TRAVERSELISTNODES_HEAD, "	row_nums_head = ");

					//free(temp->ptr_value);
					myFree((void**) &temp->ptr_value, NULL, NULL, the_debug);
					freed++;
					freed += freeAnyLinkedList((void**) &temp->row_nums_head, PTR_TYPE_LIST_NODE_PTR, NULL, NULL, the_debug);
					//free(temp);
					myFree((void**) &temp, NULL, NULL, the_debug);
					freed++;
				}

				cur_col->unique_list_head = NULL;
				cur_col->unique_list_tail = NULL;
				cur_col->frequent_list_head = NULL;
				cur_col->frequent_list_tail = NULL;

				if (the_debug == YES_DEBUG) 
					printf("	Freed %d from frequent lists\n", freed);

				//free(cur_col->frequent_arr_row_to_node);
				myFree((void**) &cur_col->frequent_arr_row_to_node, NULL, NULL, the_debug);
				cur_col->frequent_arr_row_to_node = NULL;
				// END Free frequent lists


				FILE* col_data_info = myFileOpen("_Col_Data_Info_", change_head->table->file_number, cur_col->col_number, "ab+", &file_opened_head, malloced_head, the_debug);
				if (col_data_info == NULL)
				{
					if (the_debug == YES_DEBUG)
						printf("	ERROR in deleteRows() at line %d in %s\n", __LINE__, __FILE__);
					return RETURN_ERROR;
				}

				if (writeFileInt(col_data_info, -1, &cur_col->num_rows, &file_opened_head, malloced_head, the_debug) == -1)
				{
					if (the_debug == YES_DEBUG)
						printf("	ERROR in deleteRows() at line %d in %s\n", __LINE__, __FILE__);
					return RETURN_ERROR;
				}

				if (writeFileInt(col_data_info, -1, &cur_col->num_open, &file_opened_head, malloced_head, the_debug) == -1)
				{
					if (the_debug == YES_DEBUG)
						printf("	ERROR in deleteRows() at line %d in %s\n", __LINE__, __FILE__);
					return RETURN_ERROR;
				}

				myFileClose(col_data_info, &file_opened_head, malloced_head, the_debug);


				FILE* col_data = myFileOpen("_Col_Data_", change_head->table->file_number, cur_col->col_number, "ab+", &file_opened_head, malloced_head, the_debug);
				if (col_data == NULL)
				{
					if (the_debug == YES_DEBUG)
						printf("	ERROR in deleteRows() at line %d in %s\n", __LINE__, __FILE__);
					return RETURN_ERROR;
				}
				myFileClose(col_data, &file_opened_head, malloced_head, the_debug);


				cur_col = cur_col->next;
			}
		}
	// END If table has no valid rows, delete data files and recreate them to regain space


	/*
	printf("At the traverse\n");
	cur_col = change_head->table->table_cols_head;
	while (cur_col != NULL)
	{
		printf("Col: %s\n", cur_col->col_name);
		struct ListNode* cur = cur_col->open_list_head;
		while (cur != NULL)
		{
			printf("%d,", cur->value);
			cur = cur->next;
		}
		printf("\ncur_col->open_list_before_tail = %d\n", cur_col->open_list_before_tail == NULL ? -1 : cur_col->open_list_before_tail->value);

		cur_col = cur_col->next;
	}*/


	if (file_opened_head != NULL)
	{
		if (the_debug == YES_DEBUG)
			printf("deleteRows() did not close all files\n");
		myFileCloseAll(&file_opened_head, malloced_head, the_debug);
	}

	return num_rows_in_result;
}

/*	RETURNS:
 *  RETURN_ERROR if error
 *	The number of rows updated if good
 *
 *	WRITES TO:
 */
int updateRows(struct change_node_v2* change_head, struct malloced_node** malloced_head, int the_debug)
{
	struct file_opened_node* file_opened_head = NULL;


	struct ListNodePtr* tab_cols_info_head = NULL;
	struct ListNodePtr* tab_cols_info_tail = NULL;

	if (change_head->where_head != NULL)
	{
		// START Get all column ptrs from where clause
			if (getAllColsFromWhereNode(&tab_cols_info_head, &tab_cols_info_tail, change_head->where_head, NULL, NULL, malloced_head, the_debug) != RETURN_GOOD)
			{
				if (the_debug == YES_DEBUG)
					printf("	ERROR in deleteRows() at line %d in %s\n", __LINE__, __FILE__);
				return RETURN_ERROR;
			}

			bool check_for_dups_arr[change_head->table->num_cols];
			for (int i=0; i<change_head->table->num_cols; i++)
			{
				check_for_dups_arr[i] = false;
			}

			struct ListNodePtr* cur = tab_cols_info_head;
			while (cur != NULL)
			{
				if (!check_for_dups_arr[((struct table_cols_info*) cur->ptr_value)->col_number])
				{
					check_for_dups_arr[((struct table_cols_info*) cur->ptr_value)->col_number] = true;

					struct table_cols_info* temp_col_ptr = cur->ptr_value;

					cur->ptr_type = ((struct table_cols_info*) cur->ptr_value)->col_number;

					cur->ptr_value = (void*) getAllColData(change_head->table->file_number, temp_col_ptr, NULL, -1, malloced_head, the_debug);
					if (cur->ptr_value == NULL)
					{
						if (the_debug == YES_DEBUG)
							printf("	ERROR in deleteRows() at line %d in %s\n", __LINE__, __FILE__);
						return RETURN_ERROR;
					}
				}
				else
				{
					cur->ptr_value = NULL;
					cur->ptr_type = -1;
				}

				cur = cur->next;
			}
		// END Get all column ptrs from where clause
	}


	int_8 num_rows_in_result = 0;
	struct ListNodePtr* valid_rows_head;
	struct ListNodePtr* valid_rows_tail;
	if (findValidRowsGivenWhere(&valid_rows_head, &valid_rows_tail, NULL, change_head->table, change_head->where_head, &num_rows_in_result, tab_cols_info_head, tab_cols_info_tail
								,false, -1, -1, malloced_head, the_debug) != RETURN_GOOD)
	{
		if (the_debug == YES_DEBUG)
			printf("	ERROR in deleteRows() at line %d in %s\n", __LINE__, __FILE__);
		return RETURN_ERROR;
	}

	if (the_debug == YES_DEBUG)
		printf("findValidRowsGivenWhere returned %lu rows\n", num_rows_in_result);

	if (num_rows_in_result > 0)
	{
		struct ListNodePtr* cur_col = change_head->col_list_head;
		struct ListNodePtr* cur_data = change_head->data_list_head;

		while (cur_col != NULL)
		{
			if (the_debug == YES_DEBUG)
				printf("Going through col _%s_\n", ((struct table_cols_info*) cur_col->ptr_value)->col_name);
			FILE* col_data = myFileOpen("_Col_Data_", change_head->table->file_number, ((struct table_cols_info*) cur_col->ptr_value)->col_number, "rb+", &file_opened_head, malloced_head, the_debug);
			if (col_data == NULL)
			{
				if (the_debug == YES_DEBUG)
					printf("	ERROR in updateRows() at line %d in %s\n", __LINE__, __FILE__);
				return RETURN_ERROR;
			}

			struct frequent_node* freq_head = NULL;

			struct ListNodePtr* cur_valid = valid_rows_head;
			while (cur_valid != NULL)
			{
				if (((struct table_cols_info*) cur_col->ptr_value)->data_type == DATA_INT)
				{
					int_8 the_data_int = *((int*) cur_data->ptr_value);
					//printf("Data = %lu\n", the_data_int);

					//printf("Currently there = %lu\n", readFileInt(col_data, ((8+((struct table_cols_info*) cur_col->ptr_value)->max_length)*(*((int_8*) cur_valid->ptr_value)))+8, NO_TRAVERSE_DISK, &file_opened_head, malloced_head, the_debug));
					if (writeFileInt(col_data, ((8+((struct table_cols_info*) cur_col->ptr_value)->max_length)*(*((int_8*) cur_valid->ptr_value)))+8, &the_data_int, &file_opened_head, malloced_head, the_debug) != 0)
					{
						if (the_debug == YES_DEBUG)
							printf("	ERROR in updateRows() at line %d in %s\n", __LINE__, __FILE__);
						return RETURN_ERROR;
					}
				}
				else if (((struct table_cols_info*) cur_col->ptr_value)->data_type == DATA_REAL)
				{
					double the_data_real = *((double*) cur_data->ptr_value);
					//printf("Data = %lf\n", the_data_real);

					//printf("Currently there = %lf\n", readFileDouble(col_data, ((8+((struct table_cols_info*) cur_col->ptr_value)->max_length)*(*((int_8*) cur_valid->ptr_value)))+8, NO_TRAVERSE_DISK, &file_opened_head, malloced_head, the_debug));
					if (writeFileDouble(col_data, ((8+((struct table_cols_info*) cur_col->ptr_value)->max_length)*(*((int_8*) cur_valid->ptr_value)))+8, &the_data_real, &file_opened_head, malloced_head, the_debug) != 0)
					{
						if (the_debug == YES_DEBUG)
							printf("	ERROR in updateRows() at line %d in %s\n", __LINE__, __FILE__);
						return RETURN_ERROR;
					}
				}
				else if (((struct table_cols_info*) cur_col->ptr_value)->data_type == DATA_STRING)
				{
					//printf("Data = _%s_\n", cur_data->ptr_value);

					//char* char_data = readFileCharData(col_data, ((8+((struct table_cols_info*) cur_col->ptr_value)->max_length)*(*((int_8*) cur_valid->ptr_value)))+8, &((struct table_cols_info*) cur_col->ptr_value)->max_length, NO_TRAVERSE_DISK, &file_opened_head, malloced_head, the_debug);
					//if (char_data == NULL)
					//{
					//	if (the_debug == YES_DEBUG)
					//		printf("	ERROR in deleteRows() at line %d in %s\n", __LINE__, __FILE__);
					//	errorTeardown(&file_opened_head, malloced_head, the_debug);
					//	return RETURN_ERROR;
					//}
					//printf("Currently there = %s\n", char_data);
					//myFree((void**) &char_data, &file_opened_head, malloced_head, the_debug);

					if (writeFileCharData(col_data, ((8+((struct table_cols_info*) cur_col->ptr_value)->max_length)*(*((int_8*) cur_valid->ptr_value)))+8, &((struct table_cols_info*) cur_col->ptr_value)->max_length, cur_data->ptr_value
										 ,&file_opened_head, malloced_head, the_debug) != 0)
					{
						if (the_debug == YES_DEBUG)
							printf("	ERROR in updateRows() at line %d in %s\n", __LINE__, __FILE__);
						return RETURN_ERROR;
					}
				}
				else if (((struct table_cols_info*) cur_col->ptr_value)->data_type == DATA_DATE)
				{
					int_8 the_data_date = dateToInt(cur_data->ptr_value);
					//printf("Data = _%lu_\n", the_data_date);

					//printf("Currently there = %lu\n", readFileInt(col_data, ((8+((struct table_cols_info*) cur_col->ptr_value)->max_length)*(*((int_8*) cur_valid->ptr_value)))+8, NO_TRAVERSE_DISK, &file_opened_head, malloced_head, the_debug));
					if (writeFileInt(col_data, ((8+((struct table_cols_info*) cur_col->ptr_value)->max_length)*(*((int_8*) cur_valid->ptr_value)))+8, &the_data_date, &file_opened_head, malloced_head, the_debug) != 0)
					{
						if (the_debug == YES_DEBUG)
							printf("	ERROR in updateRows() at line %d in %s\n", __LINE__, __FILE__);
						return RETURN_ERROR;
					}
				}

				if (removeFreqNodeFromFreqLists((int*) cur_valid->ptr_value, cur_col->ptr_value, the_debug) != RETURN_GOOD)
				{
					if (the_debug == YES_DEBUG)
						printf("	ERROR in updateRows() at line %d in %s\n", __LINE__, __FILE__);
					return RETURN_ERROR;
				}


				if (addFreqNodeToTempNewList(FREQ_LIST_ADD_UPDATE, &freq_head, NULL, cur_data, (int*) cur_valid->ptr_value, cur_col->ptr_value, &file_opened_head, malloced_head, the_debug) != 0)
				{
					if (the_debug == YES_DEBUG)
						printf("	ERROR in updateRows() at line %d in %s\n", __LINE__, __FILE__);
					return RETURN_ERROR;
				}


				cur_valid = cur_valid->next;
			}

			myFileClose(col_data, &file_opened_head, malloced_head, the_debug);


			addFreqNodeToFreqLists(freq_head, cur_col->ptr_value, &file_opened_head, malloced_head, the_debug);


			cur_col = cur_col->next;
			cur_data = cur_data->next;
		}

		freeAnyLinkedList((void**) &valid_rows_head, PTR_TYPE_LIST_NODE_PTR, NULL, malloced_head, the_debug);
	}

	if (file_opened_head != NULL)
	{
		if (the_debug == YES_DEBUG)
			printf("updateRows() did not close all files\n");
		myFileCloseAll(&file_opened_head, malloced_head, the_debug);
	}

	return num_rows_in_result;
}

int multi_readDiskIntoColDataArr(struct thread_get_col_data* thread_info)
{
	// START Loop for reading all rows into arr
		//struct ListNodePtr* cur = valid_rows_head;

		if (thread_info->the_debug == YES_DEBUG)
			printf("In thread: %d-%d, rows_offset = %d\n", thread_info->index_from, thread_info->index_to, (8 + thread_info->the_col->max_length) * thread_info->index_from);

		int_8 rows_offset = (8 + thread_info->the_col->max_length) * thread_info->index_from;
		int i = thread_info->index_from;
		while (i < thread_info->index_to)
		{
			//if (valid_rows_head != NULL && num_rows_in_result != -1)
			//{
			//	rows_offset = ((*((int*) cur->ptr_value)) * (8 + the_col->max_length));

				//if (the_debug == YES_DEBUG)
				//	printf("rows_offset = %lu\n", rows_offset);
			//}

			//if (valid_rows_head != NULL)
			//	arr[i]->row_id = i;
			//else
				thread_info->arr[i]->row_id = readFileInt(thread_info->col_data, rows_offset, NO_TRAVERSE_DISK, thread_info->file_opened_head, &thread_info->malloced_head, thread_info->the_debug);

			rows_offset += 8;

			if (((int) thread_info->arr[i]->row_id) < 0)
			{
				//printf("Found open row, skipping\n");
			}
			else
			{
				if (thread_info->the_col->data_type == DATA_INT)
				{
					thread_info->arr[i]->row_data = myMalloc(sizeof(int), thread_info->file_opened_head, &thread_info->malloced_head, thread_info->the_debug);
					if (thread_info->arr[i]->row_data == NULL)
					{
						if (thread_info->the_debug == YES_DEBUG)
							printf("	ERROR in getAllColData() at line %d in %s\n", __LINE__, __FILE__);
						return RETURN_ERROR;
					}

					int_8 read_int = readFileInt(thread_info->col_data, rows_offset, NO_TRAVERSE_DISK, thread_info->file_opened_head, &thread_info->malloced_head, thread_info->the_debug);
					//printf("read_int = %lu\n", read_int);

					if (read_int == null_int)
					{
						myFree((void**) &thread_info->arr[i]->row_data, thread_info->file_opened_head, &thread_info->malloced_head, thread_info->the_debug);
						thread_info->arr[i]->row_data = NULL;
					}
					else
						*((int*) thread_info->arr[i]->row_data) = (int) read_int;

					//printf("arr[i]->row_data = _%d_\n", *((int*) arr[i]->row_data));
				}
				else if (thread_info->the_col->data_type == DATA_REAL)
				{
					thread_info->arr[i]->row_data = myMalloc(sizeof(double), thread_info->file_opened_head, &thread_info->malloced_head, thread_info->the_debug);
					if (thread_info->arr[i]->row_data == NULL)
					{
						if (thread_info->the_debug == YES_DEBUG)
							printf("	ERROR in getAllColData() at line %d in %s\n", __LINE__, __FILE__);
						return NULL;
					}

					double read_double = readFileDouble(thread_info->col_data, rows_offset, NO_TRAVERSE_DISK, thread_info->file_opened_head, &thread_info->malloced_head, thread_info->the_debug);
					//printf("read_double = %f\n", read_double);

					if (read_double == null_double)
					{
						myFree((void**) &thread_info->arr[i]->row_data, thread_info->file_opened_head, &thread_info->malloced_head, thread_info->the_debug);
						thread_info->arr[i]->row_data = NULL;
					}
					else
						*((double*) thread_info->arr[i]->row_data) = read_double;

					//printf("arr[i]->row_data = _%lf_\n", *((double*) arr[i]->row_data));
				}
				else if (thread_info->the_col->data_type == DATA_STRING)
				{
					thread_info->arr[i]->row_data = readFileCharData(thread_info->col_data, rows_offset, &thread_info->the_col->max_length, NO_TRAVERSE_DISK
																	,thread_info->file_opened_head, &thread_info->malloced_head, thread_info->the_debug);
					//printf("read_string = %s\n", arr[i]->row_data);

					if (thread_info->arr[i]->row_data == NULL)
					{
						if (thread_info->the_debug == YES_DEBUG)
							printf("	ERROR in getAllColData() at line %d in %s\n", __LINE__, __FILE__);
						return NULL;
					}

					if (strcmp(thread_info->arr[i]->row_data, null_string) == 0)
					{
						myFree((void**) &thread_info->arr[i]->row_data, thread_info->file_opened_head, &thread_info->malloced_head, thread_info->the_debug);
						thread_info->arr[i]->row_data = NULL;
					}

					//printf("arr[i]->row_data = _%s_\n", arr[i]->row_data);
				}
				else if (thread_info->the_col->data_type == DATA_DATE)
				{
					int_8 read_int = readFileInt(thread_info->col_data, rows_offset, NO_TRAVERSE_DISK, thread_info->file_opened_head, &thread_info->malloced_head, thread_info->the_debug);
					//printf("read_int = %lu\n", read_int);

					if (read_int == null_int)
					{
						thread_info->arr[i]->row_data = NULL;
					}
					else
					{
						thread_info->arr[i]->row_data = intToDate(read_int, thread_info->file_opened_head, &thread_info->malloced_head, thread_info->the_debug);
						if (thread_info->arr[i]->row_data == NULL)
						{
							if (thread_info->the_debug == YES_DEBUG)
								printf("	ERROR in getAllColData() at line %d in %s\n", __LINE__, __FILE__);
							return NULL;
						}
					}
				}

				i++;
			}

			rows_offset += thread_info->the_col->max_length;
		}
	// START Loop for reading all rows into arr

	return RETURN_GOOD;
}

/*	RETURNS:
 *  NULL if error
 *	Valid struct colDataNode** ptr if good
 *
 *	WRITES TO:
 */
struct colDataNode** getAllColData(int_8 table_number, struct table_cols_info* the_col, struct ListNodePtr* valid_rows_head, int num_rows_in_result
								  ,struct malloced_node** malloced_head, int the_debug)
{
	// START Init
		struct file_opened_node* file_opened_head = NULL;

		int num_rows_to_read;
		if (valid_rows_head == NULL && num_rows_in_result == -1)
			num_rows_to_read = the_col->num_rows - the_col->num_open;
		else
			num_rows_to_read = num_rows_in_result;
		
		if (the_debug == YES_DEBUG)
			printf("num_rows_to_read FROM DISK = %d\n", num_rows_to_read);
	// END Init

	// START Allocate memory for all rows in data file
		struct colDataNode** arr = (struct colDataNode**) myMalloc(sizeof(struct colDataNode*) * num_rows_to_read, &file_opened_head, malloced_head, the_debug);
		if (arr == NULL)
		{
			if (the_debug == YES_DEBUG)
				printf("	ERROR in getAllColData() at line %d in %s\n", __LINE__, __FILE__);
			return NULL;
		}
		for (int i=0; i<num_rows_to_read; i++)
		{
			arr[i] = (struct colDataNode*) myMalloc(sizeof(struct colDataNode), &file_opened_head, malloced_head, the_debug);
			if (arr[i] == NULL)
			{
				if (the_debug == YES_DEBUG)
					printf("	ERROR in getAllColData() at line %d in %s\n", __LINE__, __FILE__);
				return NULL;
			}
		}
	// END Allocate memory for all rows in data file


	/**/
	if (valid_rows_head == NULL && num_rows_to_read > 0)
	{
		// START Try multithreading
			struct thread_get_col_data* thread_arr[NUM_THREADS];
			int remainder = num_rows_to_read % NUM_THREADS;
			int total = 0;
			for (int t=0; t<NUM_THREADS; t++)
			{
				int num_rows_per_thread = num_rows_to_read/NUM_THREADS;
				if (remainder > 0)
				{
					num_rows_per_thread++;
					remainder--;
				}
				//printf("num_rows_per_thread = %d\n", num_rows_per_thread);
				//printf("Index from = %d\n", total);
				//printf("Index to = %d\n", total + num_rows_per_thread);

				thread_arr[t] = myMalloc(sizeof(struct thread_get_col_data), &file_opened_head, malloced_head, the_debug);
				if (thread_arr[t] == NULL)
				{
					if (the_debug == YES_DEBUG)
						printf("	ERROR in getAllColData() at line %d in %s\n", __LINE__, __FILE__);
					return NULL;
				}
				thread_arr[t]->the_col = the_col;
				thread_arr[t]->arr = arr;

				// START Open file for reading
					FILE* col_data = myFileOpen("_Col_Data_", table_number, the_col->col_number, "rb", &file_opened_head, malloced_head, the_debug);
					if (col_data == NULL)
					{
						if (the_debug == YES_DEBUG)
							printf("	ERROR in getAllColData() at line %d in %s\n", __LINE__, __FILE__);
						return NULL;
					}
				// END Open file for reading

				thread_arr[t]->col_data = col_data;
				thread_arr[t]->index_from = total;
				thread_arr[t]->index_to = total + num_rows_per_thread;

				thread_arr[t]->file_opened_head = &file_opened_head;
				thread_arr[t]->malloced_head = NULL;
				thread_arr[t]->the_debug = the_debug;

				pthread_create(&thread_arr[t]->thread_id, NULL, multi_readDiskIntoColDataArr, thread_arr[t]);

				total += num_rows_per_thread;
			}
			for (int t=0; t<NUM_THREADS; t++)
				pthread_join(thread_arr[t]->thread_id, NULL);

			if (the_debug == YES_DEBUG)
			{
				struct malloced_node* cur_malloc = *malloced_head;
				int count_malloced = 0;
				while (cur_malloc != NULL)
				{
					cur_malloc = cur_malloc->next;
					count_malloced++;
				}
				printf("%d things malloced before joining malloced lists\n", count_malloced);
			}

			for (int t=0; t<NUM_THREADS; t++)
			{
				myFileClose(thread_arr[t]->col_data, &file_opened_head, malloced_head, the_debug);

				struct malloced_node* cur_malloc = thread_arr[t]->malloced_head;
				//int count_malloced = 1;
				while (cur_malloc->next != NULL)
				{
					cur_malloc = cur_malloc->next;
					//count_malloced++;
				}
				//if (the_debug == YES_DEBUG)
				//	printf("%d things malloced in thread %d\n", count_malloced, t);
				//if (thread_arr[t]->malloced_head == NULL)
				//	printf("thread_arr[t]->malloced_head IS NULL\n");
				//if (*malloced_head == NULL)
				//	printf("*malloced_head IS NULL\n");


				cur_malloc->next = *malloced_head;
				(*malloced_head)->prev = cur_malloc;
				*malloced_head = thread_arr[t]->malloced_head;


				//if (thread_arr[t]->malloced_head == NULL)
				//	printf("AFTER thread_arr[t]->malloced_head IS NULL\n");
				//if (*malloced_head == NULL)
				//	printf("AFTER *malloced_head IS NULL\n");

				myFree((void**) &thread_arr[t], &file_opened_head, malloced_head, the_debug);
			}

			if (the_debug == YES_DEBUG)
			{
				struct malloced_node* cur_malloc = *malloced_head;
				int count_malloced = 0;
				while (cur_malloc != NULL)
				{
					cur_malloc = cur_malloc->next;
					count_malloced++;
				}
				printf("%d things malloced before AFTER malloced lists\n", count_malloced);
			}
		// END Try multithreading
	}
	else
	{
		// START Open file for reading
			FILE* col_data = myFileOpen("_Col_Data_", table_number, the_col->col_number, "rb", &file_opened_head, malloced_head, the_debug);
			if (col_data == NULL)
			{
				if (the_debug == YES_DEBUG)
					printf("	ERROR in getAllColData() at line %d in %s\n", __LINE__, __FILE__);
				return NULL;
			}
		// END Open file for reading

		// START Loop for reading all rows into arr
			struct ListNodePtr* cur = valid_rows_head;

			int_8 rows_offset = 0;
			int i = 0;
			while (i < num_rows_to_read)
			{
				if (valid_rows_head != NULL && num_rows_in_result != -1)
				{
					rows_offset = ((*((int*) cur->ptr_value)) * (8 + the_col->max_length));

					//if (the_debug == YES_DEBUG)
					//	printf("rows_offset = %lu\n", rows_offset);
				}

				if (valid_rows_head != NULL)
					arr[i]->row_id = i;
				else
					arr[i]->row_id = readFileInt(col_data, rows_offset, NO_TRAVERSE_DISK, &file_opened_head, malloced_head, the_debug);

				rows_offset += 8;

				if (((int) arr[i]->row_id) < 0)
				{
					//printf("Found open row, skipping\n");
				}
				else
				{
					if (the_col->data_type == DATA_INT)
					{
						arr[i]->row_data = myMalloc(sizeof(int), &file_opened_head, malloced_head, the_debug);
						if (arr[i]->row_data == NULL)
						{
							if (the_debug == YES_DEBUG)
								printf("	ERROR in getAllColData() at line %d in %s\n", __LINE__, __FILE__);
							return NULL;
						}

						int_8 read_int = readFileInt(col_data, rows_offset, NO_TRAVERSE_DISK, &file_opened_head, malloced_head, the_debug);
						//printf("read_int = %lu\n", read_int);

						if (read_int == null_int)
						{
							myFree((void**) &arr[i]->row_data, &file_opened_head, malloced_head, the_debug);
							arr[i]->row_data = NULL;
						}
						else
							*((int*) arr[i]->row_data) = (int) read_int;

						//printf("arr[i]->row_data = _%d_\n", *((int*) arr[i]->row_data));
					}
					else if (the_col->data_type == DATA_REAL)
					{
						arr[i]->row_data = myMalloc(sizeof(double), &file_opened_head, malloced_head, the_debug);
						if (arr[i]->row_data == NULL)
						{
							if (the_debug == YES_DEBUG)
								printf("	ERROR in getAllColData() at line %d in %s\n", __LINE__, __FILE__);
							return NULL;
						}

						double read_double = readFileDouble(col_data, rows_offset, NO_TRAVERSE_DISK, &file_opened_head, malloced_head, the_debug);
						//printf("read_double = %f\n", read_double);

						if (read_double == null_double)
						{
							myFree((void**) &arr[i]->row_data, &file_opened_head, malloced_head, the_debug);
							arr[i]->row_data = NULL;
						}
						else
							*((double*) arr[i]->row_data) = read_double;

						//printf("arr[i]->row_data = _%lf_\n", *((double*) arr[i]->row_data));
					}
					else if (the_col->data_type == DATA_STRING)
					{
						arr[i]->row_data = readFileCharData(col_data, rows_offset, &the_col->max_length, NO_TRAVERSE_DISK, &file_opened_head, malloced_head, the_debug);
						//printf("read_string = %s\n", arr[i]->row_data);

						if (arr[i]->row_data == NULL)
						{
							if (the_debug == YES_DEBUG)
								printf("	ERROR in getAllColData() at line %d in %s\n", __LINE__, __FILE__);
							return NULL;
						}

						if (strcmp(arr[i]->row_data, null_string) == 0)
						{
							myFree((void**) &arr[i]->row_data, &file_opened_head, malloced_head, the_debug);
							arr[i]->row_data = NULL;
						}

						//printf("arr[i]->row_data = _%s_\n", arr[i]->row_data);
					}
					else if (the_col->data_type == DATA_DATE)
					{
						int_8 read_int = readFileInt(col_data, rows_offset, NO_TRAVERSE_DISK, &file_opened_head, malloced_head, the_debug);
						//printf("read_int = %lu\n", read_int);

						if (read_int == null_int)
						{
							arr[i]->row_data = NULL;
						}
						else
						{
							arr[i]->row_data = intToDate(read_int, &file_opened_head, malloced_head, the_debug);
							if (arr[i]->row_data == NULL)
							{
								if (the_debug == YES_DEBUG)
									printf("	ERROR in getAllColData() at line %d in %s\n", __LINE__, __FILE__);
								return NULL;
							}
						}
						//printf("arr[i]->row_data = _%s_\n", arr[i]->row_data);
					}

					i++;
				}

				rows_offset += the_col->max_length;

				if (valid_rows_head != NULL && num_rows_in_result > 0)
					cur = cur->next;
			}
		// END Loop for reading all rows into arr

		myFileClose(col_data, &file_opened_head, malloced_head, the_debug);
	}		


    if (file_opened_head != NULL)
    {
        if (the_debug == YES_DEBUG)
			printf("getAllColData() did not close all files\n");
        myFileCloseAll(&file_opened_head, malloced_head, the_debug);
	}

	return arr;
}

/*	RETURNS:
 *  RETURN_ERROR if error
 *	RETURN_GOOD if good
 *
 *	WRITES TO:
 *  col_in_select->col_data_arr
 */
int getAllColDataFromFreq(struct col_in_select_node* col_in_select, struct ListNodePtr* valid_rows_head, int num_rows_in_result
						 ,struct malloced_node** malloced_head, int the_debug)
{
	// START Init
		struct table_cols_info* the_col = col_in_select->col_ptr;

		int num_rows_to_read;

		if (valid_rows_head == NULL && num_rows_in_result == -1)
			num_rows_to_read = the_col->num_rows - the_col->num_open;
		else
			num_rows_to_read = num_rows_in_result;
		if (the_debug == YES_DEBUG)
			printf("num_rows_to_read FROM FREQUENT LISTS = %d\n", num_rows_to_read);
	// END Init

	// START Allocate memory for all rows in data file and get from frequent lists
		struct colDataNode** arr = (struct colDataNode**) myMalloc(sizeof(struct colDataNode*) * num_rows_to_read, NULL, malloced_head, the_debug);
		if (arr == NULL)
		{
			if (the_debug == YES_DEBUG)
				printf("	ERROR in getAllColData() at line %d in %s\n", __LINE__, __FILE__);
			return RETURN_ERROR;
		}

		int_8 index = 0;
		int count_open = 0;
		struct ListNodePtr* cur = valid_rows_head;
		for (int i=0; i<num_rows_to_read; i++)
		{
			if (valid_rows_head != NULL && num_rows_in_result > 0)
				index = *((int*) cur->ptr_value);
			else
				index = i+count_open;

			arr[i] = (struct colDataNode*) myMalloc(sizeof(struct colDataNode), NULL, malloced_head, the_debug);
			if (arr[i] == NULL)
			{
				if (the_debug == YES_DEBUG)
					printf("	ERROR in getAllColData() at line %d in %s\n", __LINE__, __FILE__);
				return RETURN_ERROR;
			}

			while (the_col->frequent_arr_row_to_node[index] == NULL)
			{
				if (the_debug == YES_DEBUG)
					printf("Skipping row_id = %d cuz open\n", index);
				index++;
				count_open++;
			}

			arr[i]->row_data = the_col->frequent_arr_row_to_node[index]->ptr_value;
			arr[i]->row_id = i;

			if (valid_rows_head != NULL && num_rows_in_result > 0)
				cur = cur->next;
		}
	// END Allocate memory for all rows in data file and get from frequent lists

	col_in_select->col_data_arr = arr;

	return RETURN_GOOD;
}

/*	RETURNS:
 *  RETURN_ERROR if error
 *	RETURN_GOOD if good
 *
 *	WRITES TO:
 *  the_col->col_data_arr[row_arr_index]->row_data
 */
int initNewColDataNode(struct colDataNode** col_data_arr, int row_arr_index, void* result, int the_ptr_type, int char_length
					  ,struct malloced_node** malloced_head, int the_debug)
{
	/*struct ListNodePtr* list_node = NULL;
	if ((list_node = inListNodePtrList(&the_col->unique_values_head, &the_col->unique_values_tail, result, the_ptr_type)) == NULL)
	{
		if (result == NULL)
		{
			the_col->col_data_arr[row_arr_index]->row_data = NULL;
		}
		else*/ if (the_ptr_type == PTR_TYPE_INT)
		{
			col_data_arr[row_arr_index]->row_data = (int*) myMalloc(sizeof(int), NULL, malloced_head, the_debug);
			if (col_data_arr[row_arr_index]->row_data == NULL)
			{
				if (the_debug == YES_DEBUG)
					printf("	ERROR in execAggFunc() at line %d in %s\n", __LINE__, __FILE__);
				return RETURN_ERROR;
			}

			*((int*) col_data_arr[row_arr_index]->row_data) = *((int*) result);
			//printf("*((int*) the_col->col_data_arr[%d]->row_data) = %d\n", row_arr_index, *((int*) the_col->col_data_arr[row_arr_index]->row_data));
		}
		else if (the_ptr_type == PTR_TYPE_REAL)
		{
			col_data_arr[row_arr_index]->row_data = (double*) myMalloc(sizeof(double), NULL, malloced_head, the_debug);
			if (col_data_arr[row_arr_index]->row_data == NULL)
			{
				if (the_debug == YES_DEBUG)
					printf("	ERROR in execAggFunc() at line %d in %s\n", __LINE__, __FILE__);
				return RETURN_ERROR;
			}

			*((double*) col_data_arr[row_arr_index]->row_data) = *((double*) result);
			//printf("*((int*) the_col->col_data_arr[%d]->row_data) = %lf\n", row_arr_index, *((double*) the_col->col_data_arr[row_arr_index]->row_data));
		}
		else if (the_ptr_type == PTR_TYPE_CHAR)
		{
			col_data_arr[row_arr_index]->row_data = (char*) myMalloc(sizeof(char) * char_length, NULL, malloced_head, the_debug);
			if (col_data_arr[row_arr_index]->row_data == NULL)
			{
				if (the_debug == YES_DEBUG)
					printf("	ERROR in execAggFunc() at line %d in %s\n", __LINE__, __FILE__);
				return RETURN_ERROR;
			}

			strcpy(col_data_arr[row_arr_index]->row_data, result);
			//printf("*((int*) the_col->col_data_arr[%d]->row_data) = _%s_\n", row_arr_index, the_col->col_data_arr[row_arr_index]->row_data);
		}
		else if (the_ptr_type == PTR_TYPE_DATE)
		{
			col_data_arr[row_arr_index]->row_data = (char*) myMalloc(sizeof(char) * 16, NULL, malloced_head, the_debug);
			if (col_data_arr[row_arr_index]->row_data == NULL)
			{
				if (the_debug == YES_DEBUG)
					printf("	ERROR in execAggFunc() at line %d in %s\n", __LINE__, __FILE__);
				return RETURN_ERROR;
			}

			strcpy(col_data_arr[row_arr_index]->row_data, result);
			//printf("*((int*) the_col->col_data_arr[%d]->row_data) = _%s_\n", row_arr_index, the_col->col_data_arr[row_arr_index]->row_data);
		}

		/*if (addListNodePtr(&the_col->unique_values_head, &the_col->unique_values_tail, the_col->col_data_arr[row_arr_index]->row_data, the_ptr_type, ADDLISTNODE_TAIL
						  ,NULL, malloced_head, the_debug) != 0)
		{
			if (the_debug == YES_DEBUG)
				printf("	ERROR in execAggFunc() at line %d in %s\n", __LINE__, __FILE__);
			return RETURN_ERROR;
		}
	}
	else
	{
		the_col->col_data_arr[row_arr_index]->row_data = list_node->ptr_value;
	}*/

	return RETURN_GOOD;
}

/*	RETURNS:
 *  RETURN_ERROR if error
 *	RETURN_GOOD if good
 *
 *	WRITES TO:
 *  the_col->col_data_arr[row_arr_index]->row_data
 */
int calcResultOfFuncForOneRow(struct ListNodePtr* cur_row, struct func_node* the_func, struct col_in_select_node* the_col, int_8 new_col_row
							,struct malloced_node** malloced_head, int the_debug)
{
	if (the_func->which_func == FUNC_COUNT)
	{
		// START Count rows in row_ids_head
			//printf("Finding count\n");
			int total = 0;

			struct ListNodePtr* unique_head = NULL;
			struct ListNodePtr* unique_tail = NULL;

			struct ListNodePtr* cur_row_id = ((struct group_data_node*) cur_row->ptr_value)->row_ids_head;
			while (cur_row_id != NULL)
			{
				if (the_func->distinct)
				{
					if (the_func->args_arr_type[0] == PTR_TYPE_COL_IN_SELECT_NODE && the_func->args_size > 1)
					{
						char* temp_row = (char*) myMalloc(sizeof(char) * 10000, NULL, malloced_head, the_debug);
						if (temp_row == NULL)
						{
							if (the_debug == YES_DEBUG)
								printf("	ERROR in execAggFunc() at line %d in %s\n", __LINE__, __FILE__);
							return RETURN_ERROR;
						}
						temp_row[0] = 0;

						for (int a=0; a<the_func->args_size; a++)
						{
							if (((struct col_in_select_node*) the_func->args_arr[a])->rows_data_type == PTR_TYPE_INT)
							{
								if (((struct col_in_select_node*) the_func->args_arr[a])->col_data_arr[*((int*) cur_row_id->ptr_value)]->row_data != NULL)
								{
									char str[100];
									sprintf(str, "%d", *((int*) ((struct col_in_select_node*) the_func->args_arr[a])->col_data_arr[*((int*) cur_row_id->ptr_value)]->row_data));

									strcat(temp_row, str);
								}
							}
							else if (((struct col_in_select_node*) the_func->args_arr[a])->rows_data_type == PTR_TYPE_REAL)
							{
								if (((struct col_in_select_node*) the_func->args_arr[a])->col_data_arr[*((int*) cur_row_id->ptr_value)]->row_data != NULL)
								{
									char str[100];
									sprintf(str, "%lf", *((double*) ((struct col_in_select_node*) the_func->args_arr[a])->col_data_arr[*((int*) cur_row_id->ptr_value)]->row_data));

									strcat(temp_row, str);
								}
							}
							else
							{
								if (((struct col_in_select_node*) the_func->args_arr[a])->col_data_arr[*((int*) cur_row_id->ptr_value)]->row_data != NULL)
								{
									strcat(temp_row, ((struct col_in_select_node*) the_func->args_arr[a])->col_data_arr[*((int*) cur_row_id->ptr_value)]->row_data);
								}
							}
							strcat(temp_row, ",");
						}

						//printf("temp_row = _%s_\n", temp_row);

						if (inListNodePtrList(&unique_head, &unique_tail, temp_row, PTR_TYPE_CHAR) == NULL)
						{
							//printf("Found unique value: _%s_\n", ((struct col_in_select_node*) the_func->args_arr[0])->col_data_arr[*((int*) cur_row_id->ptr_value)]->row_data);
							if (addListNodePtr(&unique_head, &unique_tail, temp_row
											  ,PTR_TYPE_CHAR, ADDLISTNODE_TAIL, NULL, malloced_head, the_debug) != 0)
							{
								if (the_debug == YES_DEBUG)
									printf("	ERROR in execAggFunc() at line %d in %s\n", __LINE__, __FILE__);
								return RETURN_ERROR;
							}

							total++;
						}
					}
					else if (the_func->args_arr_type[0] == PTR_TYPE_COL_IN_SELECT_NODE)
					{
						//printf("row_data = _%s_\n", ((struct col_in_select_node*) the_func->args_arr[0])->col_data_arr[*((int*) cur_row_id->ptr_value)]->row_data);
						if (inListNodePtrList(&unique_head, &unique_tail
											 ,((struct col_in_select_node*) the_func->args_arr[0])->col_data_arr[*((int*) cur_row_id->ptr_value)]->row_data
											 ,((struct col_in_select_node*) the_func->args_arr[0])->rows_data_type) == NULL)
						{
							//printf("Found unique value: _%s_\n", ((struct col_in_select_node*) the_func->args_arr[0])->col_data_arr[*((int*) cur_row_id->ptr_value)]->row_data);
							if (addListNodePtr(&unique_head, &unique_tail
											  ,((struct col_in_select_node*) the_func->args_arr[0])->col_data_arr[*((int*) cur_row_id->ptr_value)]->row_data
											  ,((struct col_in_select_node*) the_func->args_arr[0])->rows_data_type * -1, ADDLISTNODE_TAIL, NULL, malloced_head, the_debug) != 0)
							{
								if (the_debug == YES_DEBUG)
									printf("	ERROR in execAggFunc() at line %d in %s\n", __LINE__, __FILE__);
								return RETURN_ERROR;
							}

							total++;
						}
					}
				}
				else
				{
					total++;
				}

				cur_row_id = cur_row_id->next;
			}

			
			// START See if total is a unique value, adding to the_col->unique_values_head if so
				if (((int) new_col_row) == -1)
				{
					the_func->result = (int*) myMalloc(sizeof(int), NULL, malloced_head, the_debug);
					if (the_func->result == NULL)
					{
						if (the_debug == YES_DEBUG)
							printf("	ERROR in execAggFunc() at line %d in %s\n", __LINE__, __FILE__);
						return RETURN_ERROR;
					}

					*((int*) the_func->result) = total;
					the_func->result_type = PTR_TYPE_INT;
				}
				else
				{
					if (initNewColDataNode(the_col->col_data_arr, new_col_row, &total, PTR_TYPE_INT, 0, malloced_head, the_debug) != RETURN_GOOD)
					{
						if (the_debug == YES_DEBUG)
							printf("	ERROR in execAggFunc() at line %d in %s\n", __LINE__, __FILE__);
						return RETURN_ERROR;
					}
				}
			// END See if total is a unique value, adding to the_col->unique_values_head if so


			if (unique_head != NULL)
			{
				freeAnyLinkedList((void**) &unique_head, PTR_TYPE_LIST_NODE_PTR, NULL, malloced_head, the_debug);
			}
		// END Count rows in row_ids_head
	}
	else if (the_func->which_func == FUNC_AVG)
	{
		// START Get avg of rows in row_ids_head
			//printf("Finding avg\n");

			double total = 0;
			double count = 0;

			bool at_least_one_non_null_ptr = false;

			struct ListNodePtr* unique_head = NULL;
			struct ListNodePtr* unique_tail = NULL;

			struct ListNodePtr* cur_row_id = ((struct group_data_node*) cur_row->ptr_value)->row_ids_head;
			while (cur_row_id != NULL)
			{
				//printf("cur_row_id = %d\n", *((int*) cur_row_id->ptr_value));
				if (the_func->distinct)
				{
					//printf("row_data = _%s_\n", ((struct col_in_select_node*) the_func->args_arr[0])->col_data_arr[*((int*) cur_row_id->ptr_value)]->row_data);
					if (inListNodePtrList(&unique_head, &unique_tail
										 ,((struct col_in_select_node*) the_func->args_arr[0])->col_data_arr[*((int*) cur_row_id->ptr_value)]->row_data
										 ,((struct col_in_select_node*) the_func->args_arr[0])->rows_data_type) == NULL)
					{
						//printf("Found unique value: _%s_\n", ((struct col_in_select_node*) the_func->args_arr[0])->col_data_arr[*((int*) cur_row_id->ptr_value)]->row_data);
						if (addListNodePtr(&unique_head, &unique_tail
										  ,((struct col_in_select_node*) the_func->args_arr[0])->col_data_arr[*((int*) cur_row_id->ptr_value)]->row_data
										  ,((struct col_in_select_node*) the_func->args_arr[0])->rows_data_type * -1, ADDLISTNODE_TAIL, NULL, malloced_head, the_debug) != 0)
						{
							if (the_debug == YES_DEBUG)
								printf("	ERROR in execAggFunc() at line %d in %s\n", __LINE__, __FILE__);
							return RETURN_ERROR;
						}

						if (((struct col_in_select_node*) the_func->args_arr[0])->col_data_arr[*((int*) cur_row_id->ptr_value)]->row_data != NULL)
						{
							//printf("Adding to total\n");
							//printf("Adding this: %d\n", *((int*) ((struct col_in_select_node*) the_func->args_arr[0])->col_data_arr[*((int*) cur_row_id->ptr_value)]->row_data));
							total += *((int*) ((struct col_in_select_node*) the_func->args_arr[0])->col_data_arr[*((int*) cur_row_id->ptr_value)]->row_data);
							at_least_one_non_null_ptr = true;
						}

						count++;
					}
				}
				else
				{
					if (((struct col_in_select_node*) the_func->args_arr[0])->col_data_arr[*((int*) cur_row_id->ptr_value)]->row_data != NULL)
					{
						//printf("Adding to total\n");
						//printf("Adding this: %d\n", *((int*) ((struct col_in_select_node*) the_func->args_arr[0])->col_data_arr[*((int*) cur_row_id->ptr_value)]->row_data));
						total += *((int*) ((struct col_in_select_node*) the_func->args_arr[0])->col_data_arr[*((int*) cur_row_id->ptr_value)]->row_data);
						at_least_one_non_null_ptr = true;
					}

					count++;
				}

				cur_row_id = cur_row_id->next;
			}

			
			// START See if total is a unique value, adding to the_col->unique_values_head if so
				if (at_least_one_non_null_ptr)
				{
					double result = total/count;

					//printf("result = %lf\n\n", the_col);

					if (((int) new_col_row) == -1)
					{
						the_func->result = myMalloc(sizeof(double), NULL, malloced_head, the_debug);
						if (the_func->result == NULL)
						{
							if (the_debug == YES_DEBUG)
								printf("	ERROR in execAggFunc() at line %d in %s\n", __LINE__, __FILE__);
							return RETURN_ERROR;
						}

						*((double*) the_func->result) = result;
						the_func->result_type = PTR_TYPE_REAL;
					}
					else if (initNewColDataNode(the_col->col_data_arr, new_col_row, &result, PTR_TYPE_REAL, 0, malloced_head, the_debug) != RETURN_GOOD)
					{
						if (the_debug == YES_DEBUG)
							printf("	ERROR in execAggFunc() at line %d in %s\n", __LINE__, __FILE__);
						return RETURN_ERROR;
					}
				}
				else
					the_col->col_data_arr[new_col_row]->row_data = NULL;
			// END See if total is a unique value, adding to the_col->unique_values_head if so
			

			if (unique_head != NULL)
			{
				freeAnyLinkedList((void**) &unique_head, PTR_TYPE_LIST_NODE_PTR, NULL, malloced_head, the_debug);
			}
		// END Get avg of rows in row_ids_head
	}
	else if (the_func->which_func == FUNC_FIRST)
	{
		// START Get first of rows in row_ids_head
			//printf("Finding first\n");

			bool at_least_one_non_null_ptr = false;

			struct ListNodePtr* cur_row_id = ((struct group_data_node*) cur_row->ptr_value)->row_ids_head;

			int_8 first = 0;
			if (((struct col_in_select_node*) the_func->args_arr[0])->col_data_arr[*((int*) cur_row_id->ptr_value)]->row_data != NULL)
			{
				first = dateToInt(((struct col_in_select_node*) the_func->args_arr[0])->col_data_arr[*((int*) cur_row_id->ptr_value)]->row_data);
				at_least_one_non_null_ptr = true;
			}

			cur_row_id = cur_row_id->next;

			while (cur_row_id != NULL)
			{
				if (((struct col_in_select_node*) the_func->args_arr[0])->col_data_arr[*((int*) cur_row_id->ptr_value)]->row_data != NULL)
				{
					int_8 new_first = dateToInt(((struct col_in_select_node*) the_func->args_arr[0])->col_data_arr[*((int*) cur_row_id->ptr_value)]->row_data);
					at_least_one_non_null_ptr = true;

					if (new_first < first)
						first = new_first;
				}

				cur_row_id = cur_row_id->next;
			}

			
			// START See if total is a unique value, adding to the_col->unique_values_head if so
				if (at_least_one_non_null_ptr)
				{
					char* result = intToDate(first, NULL, malloced_head, the_debug);
					if (result == NULL)
					{
						if (the_debug == YES_DEBUG)
							printf("	ERROR in execAggFunc() at line %d in %s\n", __LINE__, __FILE__);
						return RETURN_ERROR;
					}

					if (((int) new_col_row) == -1)
					{
						the_func->result = myMalloc(sizeof(char) * 16, NULL, malloced_head, the_debug);
						if (the_func->result == NULL)
						{
							if (the_debug == YES_DEBUG)
								printf("	ERROR in execAggFunc() at line %d in %s\n", __LINE__, __FILE__);
							return RETURN_ERROR;
						}

						strcpy(the_func->result, result);
						the_func->result_type = PTR_TYPE_DATE;
					}
					else if (initNewColDataNode(the_col->col_data_arr, new_col_row, result, PTR_TYPE_DATE, 16, malloced_head, the_debug) != RETURN_GOOD)
					{
						if (the_debug == YES_DEBUG)
							printf("	ERROR in execAggFunc() at line %d in %s\n", __LINE__, __FILE__);
						return RETURN_ERROR;
					}

					myFree((void**) &result, NULL, malloced_head, the_debug);
				}
				else
					the_col->col_data_arr[new_col_row]->row_data = NULL;
			// END See if total is a unique value, adding to the_col->unique_values_head if so
		// END Get first of rows in row_ids_head
	}
	else if (the_func->which_func == FUNC_LAST)
	{
		// START Get last of rows in row_ids_head
			//printf("Finding last\n");

			bool at_least_one_non_null_ptr = false;

			struct ListNodePtr* cur_row_id = ((struct group_data_node*) cur_row->ptr_value)->row_ids_head;

			int_8 last = 0;
			if (((struct col_in_select_node*) the_func->args_arr[0])->col_data_arr[*((int*) cur_row_id->ptr_value)]->row_data != NULL)
			{
				last = dateToInt(((struct col_in_select_node*) the_func->args_arr[0])->col_data_arr[*((int*) cur_row_id->ptr_value)]->row_data);
				at_least_one_non_null_ptr = true;
			}

			cur_row_id = cur_row_id->next;

			while (cur_row_id != NULL)
			{
				if (((struct col_in_select_node*) the_func->args_arr[0])->col_data_arr[*((int*) cur_row_id->ptr_value)]->row_data != NULL)
				{
					int_8 new_last = dateToInt(((struct col_in_select_node*) the_func->args_arr[0])->col_data_arr[*((int*) cur_row_id->ptr_value)]->row_data);
					at_least_one_non_null_ptr = true;

					if (new_last > last)
						last = new_last;
				}

				cur_row_id = cur_row_id->next;
			}

			
			// START See if total is a unique value, adding to the_col->unique_values_head if so
				if (at_least_one_non_null_ptr)
				{
					char* result = intToDate(last, NULL, malloced_head, the_debug);
					if (result == NULL)
					{
						if (the_debug == YES_DEBUG)
							printf("	ERROR in execAggFunc() at line %d in %s\n", __LINE__, __FILE__);
						return RETURN_ERROR;
					}

					if (((int) new_col_row) == -1)
					{
						the_func->result = myMalloc(sizeof(char) * 16, NULL, malloced_head, the_debug);
						if (the_func->result == NULL)
						{
							if (the_debug == YES_DEBUG)
								printf("	ERROR in execAggFunc() at line %d in %s\n", __LINE__, __FILE__);
							return RETURN_ERROR;
						}

						strcpy(the_func->result, result);
						the_func->result_type = PTR_TYPE_DATE;
					}
					else if (initNewColDataNode(the_col->col_data_arr, new_col_row, result, PTR_TYPE_DATE, 16, malloced_head, the_debug) != RETURN_GOOD)
					{
						if (the_debug == YES_DEBUG)
							printf("	ERROR in execAggFunc() at line %d in %s\n", __LINE__, __FILE__);
						return RETURN_ERROR;
					}

					myFree((void**) &result, NULL, malloced_head, the_debug);
				}
				else
					the_col->col_data_arr[new_col_row]->row_data = NULL;
			// END See if total is a unique value, adding to the_col->unique_values_head if so
		// END Get last of rows in row_ids_head
	}
	else if (the_func->which_func == FUNC_MIN)
	{
		// START Get min of rows in row_ids_head
			//printf("Finding min\n");

			bool at_least_one_non_null_ptr = false;

			struct ListNodePtr* cur_row_id = ((struct group_data_node*) cur_row->ptr_value)->row_ids_head;

			if (the_func->result_type == DATA_INT)
			{
				int min = 0;
				if (((struct col_in_select_node*) the_func->args_arr[0])->col_data_arr[*((int*) cur_row_id->ptr_value)]->row_data != NULL)
				{
					min = *((int*) ((struct col_in_select_node*) the_func->args_arr[0])->col_data_arr[*((int*) cur_row_id->ptr_value)]->row_data);
					at_least_one_non_null_ptr = true;
				}

				cur_row_id = cur_row_id->next;

				while (cur_row_id != NULL)
				{
					if (((struct col_in_select_node*) the_func->args_arr[0])->col_data_arr[*((int*) cur_row_id->ptr_value)]->row_data != NULL)
					{
						int new_min = *((int*) ((struct col_in_select_node*) the_func->args_arr[0])->col_data_arr[*((int*) cur_row_id->ptr_value)]->row_data);
						at_least_one_non_null_ptr = true;

						if (new_min < min)
							min = new_min;
					}

					cur_row_id = cur_row_id->next;
				}

			
				// START See if total is a unique value, adding to the_col->unique_values_head if so
					if (at_least_one_non_null_ptr)
					{
						if (((int) new_col_row) == -1)
						{
							the_func->result = myMalloc(sizeof(int), NULL, malloced_head, the_debug);
							if (the_func->result == NULL)
							{
								if (the_debug == YES_DEBUG)
									printf("	ERROR in execAggFunc() at line %d in %s\n", __LINE__, __FILE__);
								return RETURN_ERROR;
							}

							*((int*) the_func->result) = min;
							the_func->result_type = PTR_TYPE_INT;
						}
						else if (initNewColDataNode(the_col->col_data_arr, new_col_row, &min, PTR_TYPE_INT, 0, malloced_head, the_debug) != RETURN_GOOD)
						{
							if (the_debug == YES_DEBUG)
								printf("	ERROR in execAggFunc() at line %d in %s\n", __LINE__, __FILE__);
							return RETURN_ERROR;
						}
					}
					else
						the_col->col_data_arr[new_col_row]->row_data = NULL;
				// END See if total is a unique value, adding to the_col->unique_values_head if so
			}
			else //if (the_func->result_type == DATA_REAL)
			{
				double min = 0.0;
				if (((struct col_in_select_node*) the_func->args_arr[0])->col_data_arr[*((int*) cur_row_id->ptr_value)]->row_data != NULL)
				{
					*((double*) ((struct col_in_select_node*) the_func->args_arr[0])->col_data_arr[*((int*) cur_row_id->ptr_value)]->row_data);
					at_least_one_non_null_ptr = true;
				}

				cur_row_id = cur_row_id->next;

				while (cur_row_id != NULL)
				{
					if (((struct col_in_select_node*) the_func->args_arr[0])->col_data_arr[*((int*) cur_row_id->ptr_value)]->row_data != NULL)
					{
						double new_min = *((double*) ((struct col_in_select_node*) the_func->args_arr[0])->col_data_arr[*((int*) cur_row_id->ptr_value)]->row_data);
						at_least_one_non_null_ptr = true;

						if (new_min < min)
							min = new_min;
					}

					cur_row_id = cur_row_id->next;
				}

			
				// START See if total is a unique value, adding to the_col->unique_values_head if so
					if (at_least_one_non_null_ptr)
					{
						if (((int) new_col_row) == -1)
						{
							the_func->result = myMalloc(sizeof(double), NULL, malloced_head, the_debug);
							if (the_func->result == NULL)
							{
								if (the_debug == YES_DEBUG)
									printf("	ERROR in execAggFunc() at line %d in %s\n", __LINE__, __FILE__);
								return RETURN_ERROR;
							}

							*((double*) the_func->result) = min;
							the_func->result_type = PTR_TYPE_REAL;
						}
						else if (initNewColDataNode(the_col, new_col_row, &min, PTR_TYPE_REAL, 0, malloced_head, the_debug) != RETURN_GOOD)
						{
							if (the_debug == YES_DEBUG)
								printf("	ERROR in execAggFunc() at line %d in %s\n", __LINE__, __FILE__);
							return RETURN_ERROR;
						}
					}
					else
						the_col->col_data_arr[new_col_row]->row_data = NULL;
				// END See if total is a unique value, adding to the_col->unique_values_head if so
			}
		// END Get min of rows in row_ids_head
	}
	else if (the_func->which_func == FUNC_MAX)
	{
		// START Get max of rows in row_ids_head
			//printf("Finding max\n");

			bool at_least_one_non_null_ptr = false;

			struct ListNodePtr* cur_row_id = ((struct group_data_node*) cur_row->ptr_value)->row_ids_head;

			if (the_func->result_type == DATA_INT)
			{
				int max = 0;
				if (((struct col_in_select_node*) the_func->args_arr[0])->col_data_arr[*((int*) cur_row_id->ptr_value)]->row_data != NULL)
				{
					max = *((int*) ((struct col_in_select_node*) the_func->args_arr[0])->col_data_arr[*((int*) cur_row_id->ptr_value)]->row_data);
					at_least_one_non_null_ptr = true;
				}

				cur_row_id = cur_row_id->next;

				while (cur_row_id != NULL)
				{
					if (((struct col_in_select_node*) the_func->args_arr[0])->col_data_arr[*((int*) cur_row_id->ptr_value)]->row_data != NULL)
					{
						int new_max = *((int*) ((struct col_in_select_node*) the_func->args_arr[0])->col_data_arr[*((int*) cur_row_id->ptr_value)]->row_data);
						at_least_one_non_null_ptr = true;

						if (new_max > max)
							max = new_max;
					}

					cur_row_id = cur_row_id->next;
				}

			
				// START See if total is a unique value, adding to the_col->unique_values_head if so
					if (at_least_one_non_null_ptr)
					{
						if (((int) new_col_row) == -1)
						{
							the_func->result = myMalloc(sizeof(int), NULL, malloced_head, the_debug);
							if (the_func->result == NULL)
							{
								if (the_debug == YES_DEBUG)
									printf("	ERROR in execAggFunc() at line %d in %s\n", __LINE__, __FILE__);
								return RETURN_ERROR;
							}

							*((int*) the_func->result) = max;
							the_func->result_type = PTR_TYPE_INT;
						}
						else if (initNewColDataNode(the_col->col_data_arr, new_col_row, &max, PTR_TYPE_INT, 0, malloced_head, the_debug) != RETURN_GOOD)
						{
							if (the_debug == YES_DEBUG)
								printf("	ERROR in execAggFunc() at line %d in %s\n", __LINE__, __FILE__);
							return RETURN_ERROR;
						}
					}
					else
						the_col->col_data_arr[new_col_row]->row_data = NULL;
				// END See if total is a unique value, adding to the_col->unique_values_head if so
			}
			else //if (the_func->result_type == DATA_REAL)
			{
				double max = 0.0;
				if (((struct col_in_select_node*) the_func->args_arr[0])->col_data_arr[*((int*) cur_row_id->ptr_value)]->row_data != NULL)
				{
					*((double*) ((struct col_in_select_node*) the_func->args_arr[0])->col_data_arr[*((int*) cur_row_id->ptr_value)]->row_data);
					at_least_one_non_null_ptr = true;
				}

				cur_row_id = cur_row_id->next;

				while (cur_row_id != NULL)
				{
					if (((struct col_in_select_node*) the_func->args_arr[0])->col_data_arr[*((int*) cur_row_id->ptr_value)]->row_data != NULL)
					{
						double new_max = *((double*) ((struct col_in_select_node*) the_func->args_arr[0])->col_data_arr[*((int*) cur_row_id->ptr_value)]->row_data);
						at_least_one_non_null_ptr = true;

						if (new_max > max)
							max = new_max;
					}

					cur_row_id = cur_row_id->next;
				}

			
				// START See if total is a unique value, adding to the_col->unique_values_head if so
					if (at_least_one_non_null_ptr)
					{
						if (((int) new_col_row) == -1)
						{
							the_func->result = myMalloc(sizeof(double), NULL, malloced_head, the_debug);
							if (the_func->result == NULL)
							{
								if (the_debug == YES_DEBUG)
									printf("	ERROR in execAggFunc() at line %d in %s\n", __LINE__, __FILE__);
								return RETURN_ERROR;
							}

							*((double*) the_func->result) = max;
							the_func->result_type = PTR_TYPE_REAL;
						}
						else if (initNewColDataNode(the_col->col_data_arr, new_col_row, &max, PTR_TYPE_REAL, 0, malloced_head, the_debug) != RETURN_GOOD)
						{
							if (the_debug == YES_DEBUG)
								printf("	ERROR in execAggFunc() at line %d in %s\n", __LINE__, __FILE__);
							return RETURN_ERROR;
						}
					}
					else
						the_col->col_data_arr[new_col_row]->row_data = NULL;
				// END See if total is a unique value, adding to the_col->unique_values_head if so
			}
		// END Get max of rows in row_ids_head
	}
	else if (the_func->which_func == FUNC_MEDIAN)
	{
		// START Get median of rows in row_ids_head
			//printf("Finding median\n");

			bool at_least_one_non_null_ptr = false;

			int count = 0;

			struct colDataNode** temp_col_data_arr = NULL;

			//if (new_col_row == 0)
			//{
				struct ListNodePtr* dup_rows_head = NULL;
				struct ListNodePtr* dup_rows_tail = NULL;

				struct ListNodePtr* unique_head = NULL;
				struct ListNodePtr* unique_tail = NULL;

				struct ListNodePtr* cur_row_id = ((struct group_data_node*) cur_row->ptr_value)->row_ids_head;
				while (cur_row_id != NULL)
				{
					if (the_func->distinct)
					{
						//printf("row_data = _%s_\n", ((struct col_in_select_node*) the_func->args_arr[0])->col_data_arr[*((int*) cur_row_id->ptr_value)]->row_data);
						if (inListNodePtrList(&unique_head, &unique_tail
											 ,((struct col_in_select_node*) the_func->args_arr[0])->col_data_arr[*((int*) cur_row_id->ptr_value)]->row_data
											 ,((struct col_in_select_node*) the_func->args_arr[0])->rows_data_type) == NULL)
						{
							//printf("Found unique value: _%s_\n", ((struct col_in_select_node*) the_func->args_arr[0])->col_data_arr[*((int*) cur_row_id->ptr_value)]->row_data);
							if (addListNodePtr(&unique_head, &unique_tail
											  ,((struct col_in_select_node*) the_func->args_arr[0])->col_data_arr[*((int*) cur_row_id->ptr_value)]->row_data
											  ,((struct col_in_select_node*) the_func->args_arr[0])->rows_data_type * -1, ADDLISTNODE_TAIL, NULL, malloced_head, the_debug) != 0)
							{
								if (the_debug == YES_DEBUG)
									printf("	ERROR in execAggFunc() at line %d in %s\n", __LINE__, __FILE__);
								return RETURN_ERROR;
							}

							if (((struct col_in_select_node*) the_func->args_arr[0])->col_data_arr[*((int*) cur_row_id->ptr_value)]->row_data != NULL)
							{
								at_least_one_non_null_ptr = true;
							}

							count++;
						}
						else
						{
							if (addListNodePtr_Int(&dup_rows_head, &dup_rows_tail, *((int*) cur_row_id->ptr_value), ADDLISTNODE_TAIL
												  ,NULL, malloced_head, the_debug) != 0)
							{
								if (the_debug == YES_DEBUG)
									printf("	ERROR in execAggFunc() at line %d in %s\n", __LINE__, __FILE__);
								return RETURN_ERROR;
							}
						}
					}
					else
					{
						if (((struct col_in_select_node*) the_func->args_arr[0])->col_data_arr[*((int*) cur_row_id->ptr_value)]->row_data != NULL)
						{
							at_least_one_non_null_ptr = true;
						}

						count++;
					}

					cur_row_id = cur_row_id->next;
				}

				if (unique_head != NULL)
				{
					freeAnyLinkedList((void**) &unique_head, PTR_TYPE_LIST_NODE_PTR, NULL, malloced_head, the_debug);
				}

				temp_col_data_arr = (struct colDataNode**) myMalloc(sizeof(struct colDataNode*) * count, NULL, malloced_head, the_debug);
				if (temp_col_data_arr == NULL)
				{
					if (the_debug == YES_DEBUG)
						printf("	ERROR in execAggFunc() at line %d in %s\n", __LINE__, __FILE__);
					return RETURN_ERROR;
				}

				cur_row_id = ((struct group_data_node*) cur_row->ptr_value)->row_ids_head;
				int k = 0;
				while (cur_row_id != NULL)
				{
					if (inListNodePtrList(&dup_rows_head, &dup_rows_tail, cur_row_id->ptr_value, PTR_TYPE_INT) == NULL)
					{
						temp_col_data_arr[k] = ((struct col_in_select_node*) the_func->args_arr[0])->col_data_arr[k];
						k++;
					}

					cur_row_id = cur_row_id->next;
				}

				if (dup_rows_head != NULL)
				{
					freeAnyLinkedList((void**) &dup_rows_head, PTR_TYPE_LIST_NODE_PTR, NULL, malloced_head, the_debug);
				}

				//printf("Median count = %d\n", count);
				//for (int i=0; i<count; i++)
				//{
				//	if (temp_col_data_arr[i]->row_data != NULL)
				//		printf("row_data at i: %d = %d\n", i, *((int*) temp_col_data_arr[i]->row_data));
				//	else
				//		printf("row_data at i: %d = (null)\n", i);
				//}

				mergeSort(temp_col_data_arr, the_func->result_type, ORDER_BY_ASC, 0, count-1);

				//printf("Median count = %d\n", count);
				for (int i=0; i<count; i++)
				{
					if (temp_col_data_arr[i]->row_data != NULL)
					{
						//printf("row_data at i: %d = %d\n", i, *((int*) temp_col_data_arr[i]->row_data));
					}
					else
					{
						count = i;
						//printf("row_data at i: %d = (null)\n", i);
						break;
					}
				}
				if (the_debug == YES_DEBUG)
					printf("Median count = %d\n", count);
			//}


			if (the_func->result_type == DATA_INT)
			{
				if (count % 2 == 1)
				{
					// START See if total is a unique value, adding to the_col->unique_values_head if so
						if (at_least_one_non_null_ptr)
						{
							if (((int) new_col_row) == -1)
							{
								the_func->result = myMalloc(sizeof(int), NULL, malloced_head, the_debug);
								if (the_func->result == NULL)
								{
									if (the_debug == YES_DEBUG)
										printf("	ERROR in execAggFunc() at line %d in %s\n", __LINE__, __FILE__);
									return RETURN_ERROR;
								}

								*((int*) the_func->result) = *((int*) temp_col_data_arr[(count / 2)]->row_data);
								the_func->result_type = PTR_TYPE_INT;
							}
							else if (initNewColDataNode(the_col->col_data_arr, new_col_row, temp_col_data_arr[(count / 2)]->row_data, PTR_TYPE_INT, 0, malloced_head, the_debug) != RETURN_GOOD)
							{
								if (the_debug == YES_DEBUG)
									printf("	ERROR in execAggFunc() at line %d in %s\n", __LINE__, __FILE__);
								return RETURN_ERROR;
							}
						}
						else
							the_col->col_data_arr[new_col_row]->row_data = NULL;
					// END See if total is a unique value, adding to the_col->unique_values_head if so

					the_func->result_type = DATA_INT;
					the_col->rows_data_type = DATA_INT;
				}
				else
				{
					double first = *((int*) temp_col_data_arr[(count / 2)-1]->row_data);
					double second = *((int*) temp_col_data_arr[(count / 2)]->row_data);
					double result = (first + second) / 2;

					// START See if total is a unique value, adding to the_col->unique_values_head if so
						if (at_least_one_non_null_ptr)
						{
							if (((int) new_col_row) == -1)
							{
								the_func->result = myMalloc(sizeof(double), NULL, malloced_head, the_debug);
								if (the_func->result == NULL)
								{
									if (the_debug == YES_DEBUG)
										printf("	ERROR in execAggFunc() at line %d in %s\n", __LINE__, __FILE__);
									return RETURN_ERROR;
								}

								*((double*) the_func->result) = result;
								the_func->result_type = PTR_TYPE_REAL;
							}
							else if (initNewColDataNode(the_col->col_data_arr, new_col_row, &result, PTR_TYPE_REAL, 0, malloced_head, the_debug) != RETURN_GOOD)
							{
								if (the_debug == YES_DEBUG)
									printf("	ERROR in execAggFunc() at line %d in %s\n", __LINE__, __FILE__);
								return RETURN_ERROR;
							}
						}
						else
							the_col->col_data_arr[new_col_row]->row_data = NULL;
					// END See if total is a unique value, adding to the_col->unique_values_head if so

					the_func->result_type = DATA_REAL;
					the_col->rows_data_type = DATA_REAL;
				}
			}
			else //if (the_func->result_type == DATA_REAL)
			{
				the_col->col_data_arr[new_col_row]->row_data = (double*) myMalloc(sizeof(double), NULL, malloced_head, the_debug);
				if (the_col->col_data_arr[new_col_row]->row_data == NULL)
				{
					if (the_debug == YES_DEBUG)
						printf("	ERROR in execAggFunc() at line %d in %s\n", __LINE__, __FILE__);
					return RETURN_ERROR;
				}

				if (count % 2 == 1)
				{
					//*((double*) the_col->col_data_arr[new_col_row]->row_data) = *((double*) temp_col_data_arr[(count / 2)-1]->row_data);

					// START See if total is a unique value, adding to the_col->unique_values_head if so
						if (at_least_one_non_null_ptr)
						{
							if (((int) new_col_row) == -1)
							{
								the_func->result = myMalloc(sizeof(double), NULL, malloced_head, the_debug);
								if (the_func->result == NULL)
								{
									if (the_debug == YES_DEBUG)
										printf("	ERROR in execAggFunc() at line %d in %s\n", __LINE__, __FILE__);
									return RETURN_ERROR;
								}

								*((double*) the_func->result) = *((double*) temp_col_data_arr[(count / 2)]->row_data);
								the_func->result_type = PTR_TYPE_REAL;
							}
							else if (initNewColDataNode(the_col->col_data_arr, new_col_row, temp_col_data_arr[(count / 2)]->row_data, PTR_TYPE_REAL, 0, malloced_head, the_debug) != RETURN_GOOD)
							{
								if (the_debug == YES_DEBUG)
									printf("	ERROR in execAggFunc() at line %d in %s\n", __LINE__, __FILE__);
								return RETURN_ERROR;
							}
						}
						else
							the_col->col_data_arr[new_col_row]->row_data = NULL;
					// END See if total is a unique value, adding to the_col->unique_values_head if so
				}
				else
				{
					double result = (*((double*) temp_col_data_arr[(count / 2)-1]->row_data) + *((double*) temp_col_data_arr[(count / 2)]->row_data)) / 2;

					// START See if total is a unique value, adding to the_col->unique_values_head if so
						if (at_least_one_non_null_ptr)
						{
							if (((int) new_col_row) == -1)
							{
								the_func->result = myMalloc(sizeof(double), NULL, malloced_head, the_debug);
								if (the_func->result == NULL)
								{
									if (the_debug == YES_DEBUG)
										printf("	ERROR in execAggFunc() at line %d in %s\n", __LINE__, __FILE__);
									return RETURN_ERROR;
								}

								*((double*) the_func->result) = result;
								the_func->result_type = PTR_TYPE_REAL;
							}
							else if (initNewColDataNode(the_col->col_data_arr, new_col_row, &result, PTR_TYPE_REAL, 0, malloced_head, the_debug) != RETURN_GOOD)
							{
								if (the_debug == YES_DEBUG)
									printf("	ERROR in execAggFunc() at line %d in %s\n", __LINE__, __FILE__);
								return RETURN_ERROR;
							}
						}
						else
							the_col->col_data_arr[new_col_row]->row_data = NULL;
					// END See if total is a unique value, adding to the_col->unique_values_head if so
				}

				the_func->result_type = DATA_REAL;
			}

			myFree((void**) &temp_col_data_arr, NULL, malloced_head, the_debug);
		// END Get median of rows in row_ids_head
	}
	else if (the_func->which_func == FUNC_SUM)
	{
		// START Get sum of rows in row_ids_head
			//printf("Finding sum\n");

			bool at_least_one_non_null_ptr = false;

			if (the_func->result_type == DATA_INT)
			{
				int total = 0;

				struct ListNodePtr* unique_head = NULL;
				struct ListNodePtr* unique_tail = NULL;

				struct ListNodePtr* cur_row_id = ((struct group_data_node*) cur_row->ptr_value)->row_ids_head;
				while (cur_row_id != NULL)
				{
					if (the_func->distinct)
					{
						//printf("row_data = _%s_\n", ((struct col_in_select_node*) the_func->args_arr[0])->col_data_arr[*((int*) cur_row_id->ptr_value)]->row_data);
						if (inListNodePtrList(&unique_head, &unique_tail
											 ,((struct col_in_select_node*) the_func->args_arr[0])->col_data_arr[*((int*) cur_row_id->ptr_value)]->row_data
											 ,((struct col_in_select_node*) the_func->args_arr[0])->rows_data_type) == NULL)
						{
							if (((struct col_in_select_node*) the_func->args_arr[0])->col_data_arr[*((int*) cur_row_id->ptr_value)]->row_data != NULL)
							{
								total += *((int*) ((struct col_in_select_node*) the_func->args_arr[0])->col_data_arr[*((int*) cur_row_id->ptr_value)]->row_data);
								at_least_one_non_null_ptr = true;
							}
						}
					}
					else
					{
						if (((struct col_in_select_node*) the_func->args_arr[0])->col_data_arr[*((int*) cur_row_id->ptr_value)]->row_data != NULL)
						{
							total += *((int*) ((struct col_in_select_node*) the_func->args_arr[0])->col_data_arr[*((int*) cur_row_id->ptr_value)]->row_data);
							at_least_one_non_null_ptr = true;
						}
					}

					cur_row_id = cur_row_id->next;
				}

				// START See if total is a unique value, adding to the_col->unique_values_head if so
					if (at_least_one_non_null_ptr)
					{
						if (((int) new_col_row) == -1)
						{
							the_func->result = myMalloc(sizeof(int), NULL, malloced_head, the_debug);
							if (the_func->result == NULL)
							{
								if (the_debug == YES_DEBUG)
									printf("	ERROR in execAggFunc() at line %d in %s\n", __LINE__, __FILE__);
								return RETURN_ERROR;
							}

							*((int*) the_func->result) = total;
							the_func->result_type = PTR_TYPE_REAL;
						}
						else if (initNewColDataNode(the_col->col_data_arr, new_col_row, &total, PTR_TYPE_INT, 0, malloced_head, the_debug) != RETURN_GOOD)
						{
							if (the_debug == YES_DEBUG)
								printf("	ERROR in execAggFunc() at line %d in %s\n", __LINE__, __FILE__);
							return RETURN_ERROR;
						}
					}
					else
						the_col->col_data_arr[new_col_row]->row_data = NULL;
				// END See if total is a unique value, adding to the_col->unique_values_head if so

				if (unique_head != NULL)
				{
					freeAnyLinkedList((void**) &unique_head, PTR_TYPE_LIST_NODE_PTR, NULL, malloced_head, the_debug);
				}
			}
			else //if (the_func->result_type == DATA_REAL)
			{
				double total = 0.0;

				struct ListNodePtr* unique_head = NULL;
				struct ListNodePtr* unique_tail = NULL;

				struct ListNodePtr* cur_row_id = ((struct group_data_node*) cur_row->ptr_value)->row_ids_head;
				while (cur_row_id != NULL)
				{
					if (the_func->distinct)
					{
						//printf("row_data = _%s_\n", ((struct col_in_select_node*) the_func->args_arr[0])->col_data_arr[*((int*) cur_row_id->ptr_value)]->row_data);
						if (inListNodePtrList(&unique_head, &unique_tail
											 ,((struct col_in_select_node*) the_func->args_arr[0])->col_data_arr[*((int*) cur_row_id->ptr_value)]->row_data
											 ,((struct col_in_select_node*) the_func->args_arr[0])->rows_data_type) == NULL)
						{
							if (((struct col_in_select_node*) the_func->args_arr[0])->col_data_arr[*((int*) cur_row_id->ptr_value)]->row_data != NULL)
							{
								total += *((double*) ((struct col_in_select_node*) the_func->args_arr[0])->col_data_arr[*((int*) cur_row_id->ptr_value)]->row_data);
								at_least_one_non_null_ptr = true;
							}
						}
					}
					else
					{
						if (((struct col_in_select_node*) the_func->args_arr[0])->col_data_arr[*((int*) cur_row_id->ptr_value)]->row_data != NULL)
						{
							total += *((double*) ((struct col_in_select_node*) the_func->args_arr[0])->col_data_arr[*((int*) cur_row_id->ptr_value)]->row_data);
							at_least_one_non_null_ptr = true;
						}
					}

					cur_row_id = cur_row_id->next;
				}

				// START See if total is a unique value, adding to the_col->unique_values_head if so
					if (at_least_one_non_null_ptr)
					{
						if (((int) new_col_row) == -1)
						{
							the_func->result = myMalloc(sizeof(double), NULL, malloced_head, the_debug);
							if (the_func->result == NULL)
							{
								if (the_debug == YES_DEBUG)
									printf("	ERROR in execAggFunc() at line %d in %s\n", __LINE__, __FILE__);
								return RETURN_ERROR;
							}

							*((double*) the_func->result) = total;
							the_func->result_type = PTR_TYPE_REAL;
						}
						else if (initNewColDataNode(the_col->col_data_arr, new_col_row, &total, PTR_TYPE_REAL, 0, malloced_head, the_debug) != RETURN_GOOD)
						{
							if (the_debug == YES_DEBUG)
								printf("	ERROR in execAggFunc() at line %d in %s\n", __LINE__, __FILE__);
							return RETURN_ERROR;
						}
					}
					else
						the_col->col_data_arr[new_col_row]->row_data = NULL;
				// END See if total is a unique value, adding to the_col->unique_values_head if so

				if (unique_head != NULL)
				{
					freeAnyLinkedList((void**) &unique_head, PTR_TYPE_LIST_NODE_PTR, NULL, malloced_head, the_debug);
				}
			}
		// END Get sum of rows in row_ids_head
	}

	return RETURN_GOOD;
}


struct result_node* calcResultOfFuncForOneRowV2(struct ListNodePtr* cur_row, struct func_node* the_func, struct col_in_select_node* the_col, int_8 new_col_row
												,struct malloced_node** malloced_head, int the_debug)
{
	struct result_node* result_node = myMalloc(sizeof(struct result_node), NULL, malloced_head, the_debug);
	if (result_node == NULL)
	{
		if (the_debug == YES_DEBUG)
			printf("	ERROR in calcResultOfFuncForOneRowV2() at line %d in %s\n", __LINE__, __FILE__);
		return NULL;
	}

	if (the_func->which_func == FUNC_COUNT)
	{
		// START Count rows in row_ids_head
			//printf("Finding count\n");
			int total = 0;

			struct ListNodePtr* unique_head = NULL;
			struct ListNodePtr* unique_tail = NULL;

			struct ListNodePtr* cur_row_id = ((struct group_data_node*) cur_row->ptr_value)->row_ids_head;
			while (cur_row_id != NULL)
			{
				if (the_func->distinct)
				{
					if (the_func->args_arr_type[0] == PTR_TYPE_COL_IN_SELECT_NODE && the_func->args_size > 1)
					{
						char* temp_row = (char*) myMalloc(sizeof(char) * 10000, NULL, malloced_head, the_debug);
						if (temp_row == NULL)
						{
							if (the_debug == YES_DEBUG)
								printf("	ERROR in calcResultOfFuncForOneRowV2() at line %d in %s\n", __LINE__, __FILE__);
							return RETURN_ERROR;
						}
						temp_row[0] = 0;

						for (int a=0; a<the_func->args_size; a++)
						{
							if (((struct col_in_select_node*) the_func->args_arr[a])->rows_data_type == PTR_TYPE_INT)
							{
								if (((struct col_in_select_node*) the_func->args_arr[a])->col_data_arr[*((int*) cur_row_id->ptr_value)]->row_data != NULL)
								{
									char str[100];
									sprintf(str, "%d", *((int*) ((struct col_in_select_node*) the_func->args_arr[a])->col_data_arr[*((int*) cur_row_id->ptr_value)]->row_data));

									strcat(temp_row, str);
								}
							}
							else if (((struct col_in_select_node*) the_func->args_arr[a])->rows_data_type == PTR_TYPE_REAL)
							{
								if (((struct col_in_select_node*) the_func->args_arr[a])->col_data_arr[*((int*) cur_row_id->ptr_value)]->row_data != NULL)
								{
									char str[100];
									sprintf(str, "%lf", *((double*) ((struct col_in_select_node*) the_func->args_arr[a])->col_data_arr[*((int*) cur_row_id->ptr_value)]->row_data));

									strcat(temp_row, str);
								}
							}
							else
							{
								if (((struct col_in_select_node*) the_func->args_arr[a])->col_data_arr[*((int*) cur_row_id->ptr_value)]->row_data != NULL)
								{
									strcat(temp_row, ((struct col_in_select_node*) the_func->args_arr[a])->col_data_arr[*((int*) cur_row_id->ptr_value)]->row_data);
								}
							}
							strcat(temp_row, ",");
						}

						//printf("temp_row = _%s_\n", temp_row);

						if (inListNodePtrList(&unique_head, &unique_tail, temp_row, PTR_TYPE_CHAR) == NULL)
						{
							//printf("Found unique value: _%s_\n", ((struct col_in_select_node*) the_func->args_arr[0])->col_data_arr[*((int*) cur_row_id->ptr_value)]->row_data);
							if (addListNodePtr(&unique_head, &unique_tail, temp_row
											  ,PTR_TYPE_CHAR, ADDLISTNODE_TAIL, NULL, malloced_head, the_debug) != 0)
							{
								if (the_debug == YES_DEBUG)
									printf("	ERROR in calcResultOfFuncForOneRowV2() at line %d in %s\n", __LINE__, __FILE__);
								return RETURN_ERROR;
							}

							total++;
						}
					}
					else if (the_func->args_arr_type[0] == PTR_TYPE_COL_IN_SELECT_NODE)
					{
						//printf("row_data = _%s_\n", ((struct col_in_select_node*) the_func->args_arr[0])->col_data_arr[*((int*) cur_row_id->ptr_value)]->row_data);
						if (inListNodePtrList(&unique_head, &unique_tail
											 ,((struct col_in_select_node*) the_func->args_arr[0])->col_data_arr[*((int*) cur_row_id->ptr_value)]->row_data
											 ,((struct col_in_select_node*) the_func->args_arr[0])->rows_data_type) == NULL)
						{
							//printf("Found unique value: _%s_\n", ((struct col_in_select_node*) the_func->args_arr[0])->col_data_arr[*((int*) cur_row_id->ptr_value)]->row_data);
							if (addListNodePtr(&unique_head, &unique_tail
											  ,((struct col_in_select_node*) the_func->args_arr[0])->col_data_arr[*((int*) cur_row_id->ptr_value)]->row_data
											  ,((struct col_in_select_node*) the_func->args_arr[0])->rows_data_type * -1, ADDLISTNODE_TAIL, NULL, malloced_head, the_debug) != 0)
							{
								if (the_debug == YES_DEBUG)
									printf("	ERROR in calcResultOfFuncForOneRowV2() at line %d in %s\n", __LINE__, __FILE__);
								return RETURN_ERROR;
							}

							total++;
						}
					}
				}
				else
				{
					total++;
				}

				cur_row_id = cur_row_id->next;
			}

			
			// START See if total is a unique value, adding to the_col->unique_values_head if so
				if (((int) new_col_row) == -1)
				{
					result_node->result = (int*) myMalloc(sizeof(int), NULL, malloced_head, the_debug);
					if (result_node->result == NULL)
					{
						if (the_debug == YES_DEBUG)
							printf("	ERROR in calcResultOfFuncForOneRowV2() at line %d in %s\n", __LINE__, __FILE__);
						return NULL;
					}

					*((int*) result_node->result) = total;
					result_node->result_type = PTR_TYPE_INT;
				}
			// END See if total is a unique value, adding to the_col->unique_values_head if so


			if (unique_head != NULL)
			{
				freeAnyLinkedList((void**) &unique_head, PTR_TYPE_LIST_NODE_PTR, NULL, malloced_head, the_debug);
			}
		// END Count rows in row_ids_head
	}
	else if (the_func->which_func == FUNC_AVG)
	{
		// START Get avg of rows in row_ids_head
			//printf("Finding avg\n");

			double total = 0;
			double count = 0;

			bool at_least_one_non_null_ptr = false;

			struct ListNodePtr* unique_head = NULL;
			struct ListNodePtr* unique_tail = NULL;

			struct ListNodePtr* cur_row_id = ((struct group_data_node*) cur_row->ptr_value)->row_ids_head;
			while (cur_row_id != NULL)
			{
				//printf("cur_row_id = %d\n", *((int*) cur_row_id->ptr_value));
				if (the_func->distinct)
				{
					//printf("row_data = _%s_\n", ((struct col_in_select_node*) the_func->args_arr[0])->col_data_arr[*((int*) cur_row_id->ptr_value)]->row_data);
					if (inListNodePtrList(&unique_head, &unique_tail
										 ,((struct col_in_select_node*) the_func->args_arr[0])->col_data_arr[*((int*) cur_row_id->ptr_value)]->row_data
										 ,((struct col_in_select_node*) the_func->args_arr[0])->rows_data_type) == NULL)
					{
						//printf("Found unique value: _%s_\n", ((struct col_in_select_node*) the_func->args_arr[0])->col_data_arr[*((int*) cur_row_id->ptr_value)]->row_data);
						if (addListNodePtr(&unique_head, &unique_tail
										  ,((struct col_in_select_node*) the_func->args_arr[0])->col_data_arr[*((int*) cur_row_id->ptr_value)]->row_data
										  ,((struct col_in_select_node*) the_func->args_arr[0])->rows_data_type * -1, ADDLISTNODE_TAIL, NULL, malloced_head, the_debug) != 0)
						{
							if (the_debug == YES_DEBUG)
								printf("	ERROR in execAggFunc() at line %d in %s\n", __LINE__, __FILE__);
							return RETURN_ERROR;
						}

						if (((struct col_in_select_node*) the_func->args_arr[0])->col_data_arr[*((int*) cur_row_id->ptr_value)]->row_data != NULL)
						{
							//printf("Adding to total\n");
							//printf("Adding this: %d\n", *((int*) ((struct col_in_select_node*) the_func->args_arr[0])->col_data_arr[*((int*) cur_row_id->ptr_value)]->row_data));
							total += *((int*) ((struct col_in_select_node*) the_func->args_arr[0])->col_data_arr[*((int*) cur_row_id->ptr_value)]->row_data);
							at_least_one_non_null_ptr = true;
						}

						count++;
					}
				}
				else
				{
					if (((struct col_in_select_node*) the_func->args_arr[0])->col_data_arr[*((int*) cur_row_id->ptr_value)]->row_data != NULL)
					{
						//printf("Adding to total\n");
						//printf("Adding this: %d\n", *((int*) ((struct col_in_select_node*) the_func->args_arr[0])->col_data_arr[*((int*) cur_row_id->ptr_value)]->row_data));
						total += *((int*) ((struct col_in_select_node*) the_func->args_arr[0])->col_data_arr[*((int*) cur_row_id->ptr_value)]->row_data);
						at_least_one_non_null_ptr = true;
					}

					count++;
				}

				cur_row_id = cur_row_id->next;
			}

			
			// START See if total is a unique value, adding to the_col->unique_values_head if so
				if (at_least_one_non_null_ptr)
				{
					double result = total/count;

					//printf("result = %lf\n\n", the_col);

					if (((int) new_col_row) == -1)
					{
						result_node->result = (double*) myMalloc(sizeof(double), NULL, malloced_head, the_debug);
						if (result_node->result == NULL)
						{
							if (the_debug == YES_DEBUG)
								printf("	ERROR in calcResultOfFuncForOneRowV2() at line %d in %s\n", __LINE__, __FILE__);
							return NULL;
						}

						*((double*) result_node->result) = result;
						result_node->result_type = PTR_TYPE_REAL;
					}
				}
				else
					result_node->result = NULL;
			// END See if total is a unique value, adding to the_col->unique_values_head if so
			

			if (unique_head != NULL)
			{
				freeAnyLinkedList((void**) &unique_head, PTR_TYPE_LIST_NODE_PTR, NULL, malloced_head, the_debug);
			}
		// END Get avg of rows in row_ids_head
	}
	else if (the_func->which_func == FUNC_FIRST)
	{
		// START Get first of rows in row_ids_head
			//printf("Finding first\n");

			bool at_least_one_non_null_ptr = false;

			struct ListNodePtr* cur_row_id = ((struct group_data_node*) cur_row->ptr_value)->row_ids_head;

			int_8 first = 0;
			if (((struct col_in_select_node*) the_func->args_arr[0])->col_data_arr[*((int*) cur_row_id->ptr_value)]->row_data != NULL)
			{
				first = dateToInt(((struct col_in_select_node*) the_func->args_arr[0])->col_data_arr[*((int*) cur_row_id->ptr_value)]->row_data);
				at_least_one_non_null_ptr = true;
			}

			cur_row_id = cur_row_id->next;

			while (cur_row_id != NULL)
			{
				if (((struct col_in_select_node*) the_func->args_arr[0])->col_data_arr[*((int*) cur_row_id->ptr_value)]->row_data != NULL)
				{
					int_8 new_first = dateToInt(((struct col_in_select_node*) the_func->args_arr[0])->col_data_arr[*((int*) cur_row_id->ptr_value)]->row_data);
					at_least_one_non_null_ptr = true;

					if (new_first < first)
						first = new_first;
				}

				cur_row_id = cur_row_id->next;
			}

			
			// START See if total is a unique value, adding to the_col->unique_values_head if so
				if (at_least_one_non_null_ptr)
				{
					char* result = intToDate(first, NULL, malloced_head, the_debug);
					if (result == NULL)
					{
						if (the_debug == YES_DEBUG)
							printf("	ERROR in execAggFunc() at line %d in %s\n", __LINE__, __FILE__);
						return RETURN_ERROR;
					}

					if (((int) new_col_row) == -1)
					{
						result_node->result = myMalloc(sizeof(char) * 16, NULL, malloced_head, the_debug);
						if (result_node->result == NULL)
						{
							if (the_debug == YES_DEBUG)
								printf("	ERROR in calcResultOfFuncForOneRowV2() at line %d in %s\n", __LINE__, __FILE__);
							return NULL;
						}

						strcpy(result_node->result, result);
						result_node->result_type = PTR_TYPE_DATE;
					}

					myFree((void**) &result, NULL, malloced_head, the_debug);
				}
				else
					result_node->result = NULL;
			// END See if total is a unique value, adding to the_col->unique_values_head if so
		// END Get first of rows in row_ids_head
	}
	else if (the_func->which_func == FUNC_LAST)
	{
		// START Get last of rows in row_ids_head
			//printf("Finding last\n");

			bool at_least_one_non_null_ptr = false;

			struct ListNodePtr* cur_row_id = ((struct group_data_node*) cur_row->ptr_value)->row_ids_head;

			int_8 last = 0;
			if (((struct col_in_select_node*) the_func->args_arr[0])->col_data_arr[*((int*) cur_row_id->ptr_value)]->row_data != NULL)
			{
				last = dateToInt(((struct col_in_select_node*) the_func->args_arr[0])->col_data_arr[*((int*) cur_row_id->ptr_value)]->row_data);
				at_least_one_non_null_ptr = true;
			}

			cur_row_id = cur_row_id->next;

			while (cur_row_id != NULL)
			{
				if (((struct col_in_select_node*) the_func->args_arr[0])->col_data_arr[*((int*) cur_row_id->ptr_value)]->row_data != NULL)
				{
					int_8 new_last = dateToInt(((struct col_in_select_node*) the_func->args_arr[0])->col_data_arr[*((int*) cur_row_id->ptr_value)]->row_data);
					at_least_one_non_null_ptr = true;

					if (new_last > last)
						last = new_last;
				}

				cur_row_id = cur_row_id->next;
			}

			
			// START See if total is a unique value, adding to the_col->unique_values_head if so
				if (at_least_one_non_null_ptr)
				{
					char* result = intToDate(last, NULL, malloced_head, the_debug);
					if (result == NULL)
					{
						if (the_debug == YES_DEBUG)
							printf("	ERROR in execAggFunc() at line %d in %s\n", __LINE__, __FILE__);
						return RETURN_ERROR;
					}

					if (((int) new_col_row) == -1)
					{
						result_node->result = myMalloc(sizeof(char) * 16, NULL, malloced_head, the_debug);
						if (result_node->result == NULL)
						{
							if (the_debug == YES_DEBUG)
								printf("	ERROR in calcResultOfFuncForOneRowV2() at line %d in %s\n", __LINE__, __FILE__);
							return NULL;
						}

						strcpy(result_node->result, result);
						result_node->result_type = PTR_TYPE_DATE;
					}

					myFree((void**) &result, NULL, malloced_head, the_debug);
				}
				else
					result_node->result = NULL;
			// END See if total is a unique value, adding to the_col->unique_values_head if so
		// END Get last of rows in row_ids_head
	}
	else if (the_func->which_func == FUNC_MIN)
	{
		// START Get min of rows in row_ids_head
			//printf("Finding min\n");

			bool at_least_one_non_null_ptr = false;

			struct ListNodePtr* cur_row_id = ((struct group_data_node*) cur_row->ptr_value)->row_ids_head;

			if (the_func->result_type == DATA_INT)
			{
				int min = 0;
				if (((struct col_in_select_node*) the_func->args_arr[0])->col_data_arr[*((int*) cur_row_id->ptr_value)]->row_data != NULL)
				{
					min = *((int*) ((struct col_in_select_node*) the_func->args_arr[0])->col_data_arr[*((int*) cur_row_id->ptr_value)]->row_data);
					at_least_one_non_null_ptr = true;
				}

				cur_row_id = cur_row_id->next;

				while (cur_row_id != NULL)
				{
					if (((struct col_in_select_node*) the_func->args_arr[0])->col_data_arr[*((int*) cur_row_id->ptr_value)]->row_data != NULL)
					{
						int new_min = *((int*) ((struct col_in_select_node*) the_func->args_arr[0])->col_data_arr[*((int*) cur_row_id->ptr_value)]->row_data);
						at_least_one_non_null_ptr = true;

						if (new_min < min)
							min = new_min;
					}

					cur_row_id = cur_row_id->next;
				}

			
				// START See if total is a unique value, adding to the_col->unique_values_head if so
					if (at_least_one_non_null_ptr)
					{
						if (((int) new_col_row) == -1)
						{
							result_node->result = myMalloc(sizeof(int), NULL, malloced_head, the_debug);
							if (result_node->result == NULL)
							{
								if (the_debug == YES_DEBUG)
									printf("	ERROR in execAggFunc() at line %d in %s\n", __LINE__, __FILE__);
								return RETURN_ERROR;
							}

							*((int*) result_node->result) = min;
							result_node->result_type = PTR_TYPE_INT;
						}
					}
					else
						result_node->result = NULL;
				// END See if total is a unique value, adding to the_col->unique_values_head if so
			}
			else //if (the_func->result_type == DATA_REAL)
			{
				double min = 0.0;
				if (((struct col_in_select_node*) the_func->args_arr[0])->col_data_arr[*((int*) cur_row_id->ptr_value)]->row_data != NULL)
				{
					*((double*) ((struct col_in_select_node*) the_func->args_arr[0])->col_data_arr[*((int*) cur_row_id->ptr_value)]->row_data);
					at_least_one_non_null_ptr = true;
				}

				cur_row_id = cur_row_id->next;

				while (cur_row_id != NULL)
				{
					if (((struct col_in_select_node*) the_func->args_arr[0])->col_data_arr[*((int*) cur_row_id->ptr_value)]->row_data != NULL)
					{
						double new_min = *((double*) ((struct col_in_select_node*) the_func->args_arr[0])->col_data_arr[*((int*) cur_row_id->ptr_value)]->row_data);
						at_least_one_non_null_ptr = true;

						if (new_min < min)
							min = new_min;
					}

					cur_row_id = cur_row_id->next;
				}

			
				// START See if total is a unique value, adding to the_col->unique_values_head if so
					if (at_least_one_non_null_ptr)
					{
						if (((int) new_col_row) == -1)
						{
							result_node->result = myMalloc(sizeof(double), NULL, malloced_head, the_debug);
							if (result_node->result == NULL)
							{
								if (the_debug == YES_DEBUG)
									printf("	ERROR in execAggFunc() at line %d in %s\n", __LINE__, __FILE__);
								return RETURN_ERROR;
							}

							*((double*) result_node->result) = min;
							result_node->result_type = PTR_TYPE_REAL;
						}
					}
					else
						result_node->result = NULL;
				// END See if total is a unique value, adding to the_col->unique_values_head if so
			}
		// END Get min of rows in row_ids_head
	}
	else if (the_func->which_func == FUNC_MAX)
	{
		// START Get max of rows in row_ids_head
			//printf("Finding max\n");

			bool at_least_one_non_null_ptr = false;

			struct ListNodePtr* cur_row_id = ((struct group_data_node*) cur_row->ptr_value)->row_ids_head;

			if (the_func->result_type == DATA_INT)
			{
				int max = 0;
				if (((struct col_in_select_node*) the_func->args_arr[0])->col_data_arr[*((int*) cur_row_id->ptr_value)]->row_data != NULL)
				{
					max = *((int*) ((struct col_in_select_node*) the_func->args_arr[0])->col_data_arr[*((int*) cur_row_id->ptr_value)]->row_data);
					at_least_one_non_null_ptr = true;
				}

				cur_row_id = cur_row_id->next;

				while (cur_row_id != NULL)
				{
					if (((struct col_in_select_node*) the_func->args_arr[0])->col_data_arr[*((int*) cur_row_id->ptr_value)]->row_data != NULL)
					{
						int new_max = *((int*) ((struct col_in_select_node*) the_func->args_arr[0])->col_data_arr[*((int*) cur_row_id->ptr_value)]->row_data);
						at_least_one_non_null_ptr = true;

						if (new_max > max)
							max = new_max;
					}

					cur_row_id = cur_row_id->next;
				}

			
				// START See if total is a unique value, adding to the_col->unique_values_head if so
					if (at_least_one_non_null_ptr)
					{
						if (((int) new_col_row) == -1)
						{
							result_node->result = myMalloc(sizeof(int), NULL, malloced_head, the_debug);
							if (result_node->result == NULL)
							{
								if (the_debug == YES_DEBUG)
									printf("	ERROR in execAggFunc() at line %d in %s\n", __LINE__, __FILE__);
								return RETURN_ERROR;
							}

							*((int*) result_node->result) = max;
							result_node->result_type = PTR_TYPE_INT;
						}
					}
					else
						result_node->result = NULL;
				// END See if total is a unique value, adding to the_col->unique_values_head if so
			}
			else //if (the_func->result_type == DATA_REAL)
			{
				double max = 0.0;
				if (((struct col_in_select_node*) the_func->args_arr[0])->col_data_arr[*((int*) cur_row_id->ptr_value)]->row_data != NULL)
				{
					*((double*) ((struct col_in_select_node*) the_func->args_arr[0])->col_data_arr[*((int*) cur_row_id->ptr_value)]->row_data);
					at_least_one_non_null_ptr = true;
				}

				cur_row_id = cur_row_id->next;

				while (cur_row_id != NULL)
				{
					if (((struct col_in_select_node*) the_func->args_arr[0])->col_data_arr[*((int*) cur_row_id->ptr_value)]->row_data != NULL)
					{
						double new_max = *((double*) ((struct col_in_select_node*) the_func->args_arr[0])->col_data_arr[*((int*) cur_row_id->ptr_value)]->row_data);
						at_least_one_non_null_ptr = true;

						if (new_max > max)
							max = new_max;
					}

					cur_row_id = cur_row_id->next;
				}

			
				// START See if total is a unique value, adding to the_col->unique_values_head if so
					if (at_least_one_non_null_ptr)
					{
						if (((int) new_col_row) == -1)
						{
							result_node->result = myMalloc(sizeof(double), NULL, malloced_head, the_debug);
							if (result_node->result == NULL)
							{
								if (the_debug == YES_DEBUG)
									printf("	ERROR in execAggFunc() at line %d in %s\n", __LINE__, __FILE__);
								return RETURN_ERROR;
							}

							*((double*) result_node->result) = max;
							result_node->result_type = PTR_TYPE_REAL;
						}
					}
					else
						result_node->result = NULL;
				// END See if total is a unique value, adding to the_col->unique_values_head if so
			}
		// END Get max of rows in row_ids_head
	}
	else if (the_func->which_func == FUNC_MEDIAN)
	{
		// START Get median of rows in row_ids_head
			//printf("Finding median\n");

			bool at_least_one_non_null_ptr = false;

			int count = 0;

			struct colDataNode** temp_col_data_arr = NULL;

			//if (new_col_row == 0)
			//{
				struct ListNodePtr* dup_rows_head = NULL;
				struct ListNodePtr* dup_rows_tail = NULL;

				struct ListNodePtr* unique_head = NULL;
				struct ListNodePtr* unique_tail = NULL;

				struct ListNodePtr* cur_row_id = ((struct group_data_node*) cur_row->ptr_value)->row_ids_head;
				while (cur_row_id != NULL)
				{
					if (the_func->distinct)
					{
						//printf("row_data = _%s_\n", ((struct col_in_select_node*) the_func->args_arr[0])->col_data_arr[*((int*) cur_row_id->ptr_value)]->row_data);
						if (inListNodePtrList(&unique_head, &unique_tail
											 ,((struct col_in_select_node*) the_func->args_arr[0])->col_data_arr[*((int*) cur_row_id->ptr_value)]->row_data
											 ,((struct col_in_select_node*) the_func->args_arr[0])->rows_data_type) == NULL)
						{
							//printf("Found unique value: _%s_\n", ((struct col_in_select_node*) the_func->args_arr[0])->col_data_arr[*((int*) cur_row_id->ptr_value)]->row_data);
							if (addListNodePtr(&unique_head, &unique_tail
											  ,((struct col_in_select_node*) the_func->args_arr[0])->col_data_arr[*((int*) cur_row_id->ptr_value)]->row_data
											  ,((struct col_in_select_node*) the_func->args_arr[0])->rows_data_type * -1, ADDLISTNODE_TAIL, NULL, malloced_head, the_debug) != 0)
							{
								if (the_debug == YES_DEBUG)
									printf("	ERROR in execAggFunc() at line %d in %s\n", __LINE__, __FILE__);
								return RETURN_ERROR;
							}

							if (((struct col_in_select_node*) the_func->args_arr[0])->col_data_arr[*((int*) cur_row_id->ptr_value)]->row_data != NULL)
							{
								at_least_one_non_null_ptr = true;
							}

							count++;
						}
						else
						{
							if (addListNodePtr_Int(&dup_rows_head, &dup_rows_tail, *((int*) cur_row_id->ptr_value), ADDLISTNODE_TAIL
												  ,NULL, malloced_head, the_debug) != 0)
							{
								if (the_debug == YES_DEBUG)
									printf("	ERROR in execAggFunc() at line %d in %s\n", __LINE__, __FILE__);
								return RETURN_ERROR;
							}
						}
					}
					else
					{
						if (((struct col_in_select_node*) the_func->args_arr[0])->col_data_arr[*((int*) cur_row_id->ptr_value)]->row_data != NULL)
						{
							at_least_one_non_null_ptr = true;
						}

						count++;
					}

					cur_row_id = cur_row_id->next;
				}

				if (unique_head != NULL)
				{
					freeAnyLinkedList((void**) &unique_head, PTR_TYPE_LIST_NODE_PTR, NULL, malloced_head, the_debug);
				}

				temp_col_data_arr = (struct colDataNode**) myMalloc(sizeof(struct colDataNode*) * count, NULL, malloced_head, the_debug);
				if (temp_col_data_arr == NULL)
				{
					if (the_debug == YES_DEBUG)
						printf("	ERROR in execAggFunc() at line %d in %s\n", __LINE__, __FILE__);
					return RETURN_ERROR;
				}

				cur_row_id = ((struct group_data_node*) cur_row->ptr_value)->row_ids_head;
				int k = 0;
				while (cur_row_id != NULL)
				{
					if (inListNodePtrList(&dup_rows_head, &dup_rows_tail, cur_row_id->ptr_value, PTR_TYPE_INT) == NULL)
					{
						temp_col_data_arr[k] = ((struct col_in_select_node*) the_func->args_arr[0])->col_data_arr[k];
						k++;
					}

					cur_row_id = cur_row_id->next;
				}

				if (dup_rows_head != NULL)
				{
					freeAnyLinkedList((void**) &dup_rows_head, PTR_TYPE_LIST_NODE_PTR, NULL, malloced_head, the_debug);
				}

				//printf("Median count = %d\n", count);
				//for (int i=0; i<count; i++)
				//{
				//	if (temp_col_data_arr[i]->row_data != NULL)
				//		printf("row_data at i: %d = %d\n", i, *((int*) temp_col_data_arr[i]->row_data));
				//	else
				//		printf("row_data at i: %d = (null)\n", i);
				//}

				mergeSort(temp_col_data_arr, the_func->result_type, ORDER_BY_ASC, 0, count-1);

				//printf("Median count = %d\n", count);
				for (int i=0; i<count; i++)
				{
					if (temp_col_data_arr[i]->row_data != NULL)
					{
						//printf("row_data at i: %d = %d\n", i, *((int*) temp_col_data_arr[i]->row_data));
					}
					else
					{
						count = i;
						//printf("row_data at i: %d = (null)\n", i);
						break;
					}
				}
				if (the_debug == YES_DEBUG)
					printf("Median count = %d\n", count);
			//}


			if (the_func->result_type == DATA_INT)
			{
				if (count % 2 == 1)
				{
					// START See if total is a unique value, adding to the_col->unique_values_head if so
						if (at_least_one_non_null_ptr)
						{
							if (((int) new_col_row) == -1)
							{
								result_node->result = myMalloc(sizeof(int), NULL, malloced_head, the_debug);
								if (result_node->result == NULL)
								{
									if (the_debug == YES_DEBUG)
										printf("	ERROR in execAggFunc() at line %d in %s\n", __LINE__, __FILE__);
									return RETURN_ERROR;
								}

								*((int*) result_node->result) = *((int*) temp_col_data_arr[(count / 2)]->row_data);
								result_node->result_type = PTR_TYPE_INT;
							}
						}
						else
							result_node->result = NULL;
					// END See if total is a unique value, adding to the_col->unique_values_head if so
				}
				else
				{
					double first = *((int*) temp_col_data_arr[(count / 2)-1]->row_data);
					double second = *((int*) temp_col_data_arr[(count / 2)]->row_data);
					double result = (first + second) / 2;

					// START See if total is a unique value, adding to the_col->unique_values_head if so
						if (at_least_one_non_null_ptr)
						{
							if (((int) new_col_row) == -1)
							{
								result_node->result = myMalloc(sizeof(double), NULL, malloced_head, the_debug);
								if (result_node->result == NULL)
								{
									if (the_debug == YES_DEBUG)
										printf("	ERROR in execAggFunc() at line %d in %s\n", __LINE__, __FILE__);
									return RETURN_ERROR;
								}

								*((double*) result_node->result) = result;
								result_node->result_type = PTR_TYPE_REAL;
							}
						}
						else
							result_node->result = NULL;
					// END See if total is a unique value, adding to the_col->unique_values_head if so
				}
			}
			else //if (the_func->result_type == DATA_REAL)
			{
				if (count % 2 == 1)
				{
					//*((double*) the_col->col_data_arr[new_col_row]->row_data) = *((double*) temp_col_data_arr[(count / 2)-1]->row_data);

					// START See if total is a unique value, adding to the_col->unique_values_head if so
						if (at_least_one_non_null_ptr)
						{
							if (((int) new_col_row) == -1)
							{
								result_node->result = myMalloc(sizeof(double), NULL, malloced_head, the_debug);
								if (result_node->result == NULL)
								{
									if (the_debug == YES_DEBUG)
										printf("	ERROR in execAggFunc() at line %d in %s\n", __LINE__, __FILE__);
									return RETURN_ERROR;
								}

								*((double*) result_node->result) = *((double*) temp_col_data_arr[(count / 2)]->row_data);
								result_node->result_type = PTR_TYPE_REAL;
							}
						}
						else
							result_node->result = NULL;
					// END See if total is a unique value, adding to the_col->unique_values_head if so
				}
				else
				{
					double result = (*((double*) temp_col_data_arr[(count / 2)-1]->row_data) + *((double*) temp_col_data_arr[(count / 2)]->row_data)) / 2;

					// START See if total is a unique value, adding to the_col->unique_values_head if so
						if (at_least_one_non_null_ptr)
						{
							if (((int) new_col_row) == -1)
							{
								result_node->result = myMalloc(sizeof(double), NULL, malloced_head, the_debug);
								if (result_node->result == NULL)
								{
									if (the_debug == YES_DEBUG)
										printf("	ERROR in execAggFunc() at line %d in %s\n", __LINE__, __FILE__);
									return RETURN_ERROR;
								}

								*((double*) result_node->result) = result;
								result_node->result_type = PTR_TYPE_REAL;
							}
						}
						else
							result_node->result = NULL;
					// END See if total is a unique value, adding to the_col->unique_values_head if so
				}

				the_func->result_type = DATA_REAL;
			}

			myFree((void**) &temp_col_data_arr, NULL, malloced_head, the_debug);
		// END Get median of rows in row_ids_head
	}
	else if (the_func->which_func == FUNC_SUM)
	{
		// START Get sum of rows in row_ids_head
			//printf("Finding sum\n");

			bool at_least_one_non_null_ptr = false;

			if (the_func->result_type == DATA_INT)
			{
				int total = 0;

				struct ListNodePtr* unique_head = NULL;
				struct ListNodePtr* unique_tail = NULL;

				struct ListNodePtr* cur_row_id = ((struct group_data_node*) cur_row->ptr_value)->row_ids_head;
				while (cur_row_id != NULL)
				{
					if (the_func->distinct)
					{
						//printf("row_data = _%s_\n", ((struct col_in_select_node*) the_func->args_arr[0])->col_data_arr[*((int*) cur_row_id->ptr_value)]->row_data);
						if (inListNodePtrList(&unique_head, &unique_tail
											 ,((struct col_in_select_node*) the_func->args_arr[0])->col_data_arr[*((int*) cur_row_id->ptr_value)]->row_data
											 ,((struct col_in_select_node*) the_func->args_arr[0])->rows_data_type) == NULL)
						{
							if (((struct col_in_select_node*) the_func->args_arr[0])->col_data_arr[*((int*) cur_row_id->ptr_value)]->row_data != NULL)
							{
								total += *((int*) ((struct col_in_select_node*) the_func->args_arr[0])->col_data_arr[*((int*) cur_row_id->ptr_value)]->row_data);
								at_least_one_non_null_ptr = true;
							}
						}
					}
					else
					{
						if (((struct col_in_select_node*) the_func->args_arr[0])->col_data_arr[*((int*) cur_row_id->ptr_value)]->row_data != NULL)
						{
							total += *((int*) ((struct col_in_select_node*) the_func->args_arr[0])->col_data_arr[*((int*) cur_row_id->ptr_value)]->row_data);
							at_least_one_non_null_ptr = true;
						}
					}

					cur_row_id = cur_row_id->next;
				}

				// START See if total is a unique value, adding to the_col->unique_values_head if so
					if (at_least_one_non_null_ptr)
					{
						if (((int) new_col_row) == -1)
						{
							result_node->result = myMalloc(sizeof(int), NULL, malloced_head, the_debug);
							if (result_node->result == NULL)
							{
								if (the_debug == YES_DEBUG)
									printf("	ERROR in execAggFunc() at line %d in %s\n", __LINE__, __FILE__);
								return RETURN_ERROR;
							}

							*((int*) result_node->result) = total;
							result_node->result_type = PTR_TYPE_REAL;
						}
					}
					else
						result_node->result = NULL;
				// END See if total is a unique value, adding to the_col->unique_values_head if so

				if (unique_head != NULL)
				{
					freeAnyLinkedList((void**) &unique_head, PTR_TYPE_LIST_NODE_PTR, NULL, malloced_head, the_debug);
				}
			}
			else //if (the_func->result_type == DATA_REAL)
			{
				double total = 0.0;

				struct ListNodePtr* unique_head = NULL;
				struct ListNodePtr* unique_tail = NULL;

				struct ListNodePtr* cur_row_id = ((struct group_data_node*) cur_row->ptr_value)->row_ids_head;
				while (cur_row_id != NULL)
				{
					if (the_func->distinct)
					{
						//printf("row_data = _%s_\n", ((struct col_in_select_node*) the_func->args_arr[0])->col_data_arr[*((int*) cur_row_id->ptr_value)]->row_data);
						if (inListNodePtrList(&unique_head, &unique_tail
											 ,((struct col_in_select_node*) the_func->args_arr[0])->col_data_arr[*((int*) cur_row_id->ptr_value)]->row_data
											 ,((struct col_in_select_node*) the_func->args_arr[0])->rows_data_type) == NULL)
						{
							if (((struct col_in_select_node*) the_func->args_arr[0])->col_data_arr[*((int*) cur_row_id->ptr_value)]->row_data != NULL)
							{
								total += *((double*) ((struct col_in_select_node*) the_func->args_arr[0])->col_data_arr[*((int*) cur_row_id->ptr_value)]->row_data);
								at_least_one_non_null_ptr = true;
							}
						}
					}
					else
					{
						if (((struct col_in_select_node*) the_func->args_arr[0])->col_data_arr[*((int*) cur_row_id->ptr_value)]->row_data != NULL)
						{
							total += *((double*) ((struct col_in_select_node*) the_func->args_arr[0])->col_data_arr[*((int*) cur_row_id->ptr_value)]->row_data);
							at_least_one_non_null_ptr = true;
						}
					}

					cur_row_id = cur_row_id->next;
				}

				// START See if total is a unique value, adding to the_col->unique_values_head if so
					if (at_least_one_non_null_ptr)
					{
						if (((int) new_col_row) == -1)
						{
							result_node->result = myMalloc(sizeof(double), NULL, malloced_head, the_debug);
							if (result_node->result == NULL)
							{
								if (the_debug == YES_DEBUG)
									printf("	ERROR in execAggFunc() at line %d in %s\n", __LINE__, __FILE__);
								return RETURN_ERROR;
							}

							*((double*) result_node->result) = total;
							result_node->result_type = PTR_TYPE_REAL;
						}
					}
					else
						result_node->result = NULL;
				// END See if total is a unique value, adding to the_col->unique_values_head if so

				if (unique_head != NULL)
				{
					freeAnyLinkedList((void**) &unique_head, PTR_TYPE_LIST_NODE_PTR, NULL, malloced_head, the_debug);
				}
			}
		// END Get sum of rows in row_ids_head
	}

	return result_node;
}

/*	RETURNS:
 *  RETURN_ERROR if error
 *	RETURN_GOOD if good
 *
 *	WRITES TO:
 *  cur->result
 */
int doMath(struct math_node* cur, void* ptr_one, int ptr_one_type, void* ptr_two, int ptr_two_type, struct malloced_node** malloced_head, int the_debug)
{
	if (cur->operation == MATH_ADD)
	{
		if (ptr_one_type == PTR_TYPE_INT && ptr_two_type == PTR_TYPE_INT)
		{
			cur->result = (int*) myMalloc(sizeof(int), NULL, malloced_head, the_debug);
			if (cur->result == NULL)
			{
				if (the_debug == YES_DEBUG)
					printf("	ERROR in doMath() at line %d in %s\n", __LINE__, __FILE__);
				return RETURN_ERROR;
			}

			cur->result_type = PTR_TYPE_INT;

			*((int*) cur->result) = *((int*) ptr_one) + *((int*) ptr_two);
		}
		else if (ptr_one_type == PTR_TYPE_REAL && ptr_two_type == PTR_TYPE_REAL)
		{
			cur->result = (double*) myMalloc(sizeof(double), NULL, malloced_head, the_debug);
			if (cur->result == NULL)
			{
				if (the_debug == YES_DEBUG)
					printf("	ERROR in doMath() at line %d in %s\n", __LINE__, __FILE__);
				return RETURN_ERROR;
			}

			cur->result_type = PTR_TYPE_REAL;

			*((double*) cur->result) = *((double*) ptr_one) + *((double*) ptr_two);
		}
		else if (ptr_one_type == PTR_TYPE_INT && ptr_two_type == PTR_TYPE_REAL)
		{
			cur->result = (double*) myMalloc(sizeof(double), NULL, malloced_head, the_debug);
			if (cur->result == NULL)
			{
				if (the_debug == YES_DEBUG)
					printf("	ERROR in doMath() at line %d in %s\n", __LINE__, __FILE__);
				return RETURN_ERROR;
			}

			cur->result_type = PTR_TYPE_REAL;

			*((double*) cur->result) = *((int*) ptr_one) + *((double*) ptr_two);
		}
		else if (ptr_one_type == PTR_TYPE_REAL && ptr_two_type == PTR_TYPE_INT)
		{
			cur->result = (double*) myMalloc(sizeof(double), NULL, malloced_head, the_debug);
			if (cur->result == NULL)
			{
				if (the_debug == YES_DEBUG)
					printf("	ERROR in doMath() at line %d in %s\n", __LINE__, __FILE__);
				return RETURN_ERROR;
			}

			cur->result_type = PTR_TYPE_REAL;

			*((double*) cur->result) = *((double*) ptr_one) + *((int*) ptr_two);
		}
		else if (ptr_one_type == PTR_TYPE_DATE && ptr_two_type == PTR_TYPE_INT)
		{
			int_8 date = dateToInt(ptr_one);

			date += *((int*) ptr_two);

			cur->result = intToDate(date, NULL, malloced_head, the_debug);
			if (cur->result == NULL)
			{
				if (the_debug == YES_DEBUG)
					printf("	ERROR in doMath() at line %d in %s\n", __LINE__, __FILE__);
				return RETURN_ERROR;
			}

			cur->result_type = PTR_TYPE_DATE;
		}
		else if (ptr_one_type == PTR_TYPE_INT && ptr_two_type == PTR_TYPE_DATE)
		{
			int_8 date = dateToInt(ptr_two);

			date += *((int*) ptr_one);

			cur->result = intToDate(date, NULL, malloced_head, the_debug);
			if (cur->result == NULL)
			{
				if (the_debug == YES_DEBUG)
					printf("	ERROR in doMath() at line %d in %s\n", __LINE__, __FILE__);
				return RETURN_ERROR;
			}

			cur->result_type = PTR_TYPE_DATE;
		}
		else if (ptr_one_type == PTR_TYPE_DATE && ptr_two_type == PTR_TYPE_DATE)
		{
			int_8 date1 = dateToInt(ptr_one);
			int_8 date2 = dateToInt(ptr_two);

			date1 = date1 + date2;

			cur->result = intToDate(date1, NULL, malloced_head, the_debug);
			if (cur->result == NULL)
			{
				if (the_debug == YES_DEBUG)
					printf("	ERROR in doMath() at line %d in %s\n", __LINE__, __FILE__);
				return RETURN_ERROR;
			}

			cur->result_type = PTR_TYPE_DATE;
		}
	}
	else if (cur->operation == MATH_SUB)
	{
		if (ptr_one_type == PTR_TYPE_INT && ptr_two_type == PTR_TYPE_INT)
		{
			cur->result = (int*) myMalloc(sizeof(int), NULL, malloced_head, the_debug);
			if (cur->result == NULL)
			{
				if (the_debug == YES_DEBUG)
					printf("	ERROR in doMath() at line %d in %s\n", __LINE__, __FILE__);
				return RETURN_ERROR;
			}

			cur->result_type = PTR_TYPE_INT;

			*((int*) cur->result) = *((int*) ptr_one) - *((int*) ptr_two);
		}
		else if (ptr_one_type == PTR_TYPE_REAL && ptr_two_type == PTR_TYPE_REAL)
		{
			cur->result = (double*) myMalloc(sizeof(double), NULL, malloced_head, the_debug);
			if (cur->result == NULL)
			{
				if (the_debug == YES_DEBUG)
					printf("	ERROR in doMath() at line %d in %s\n", __LINE__, __FILE__);
				return RETURN_ERROR;
			}

			cur->result_type = PTR_TYPE_REAL;

			*((double*) cur->result) = *((double*) ptr_one) - *((double*) ptr_two);
		}
		else if (ptr_one_type == PTR_TYPE_INT && ptr_two_type == PTR_TYPE_REAL)
		{
			cur->result = (double*) myMalloc(sizeof(double), NULL, malloced_head, the_debug);
			if (cur->result == NULL)
			{
				if (the_debug == YES_DEBUG)
					printf("	ERROR in doMath() at line %d in %s\n", __LINE__, __FILE__);
				return RETURN_ERROR;
			}

			cur->result_type = PTR_TYPE_REAL;

			*((double*) cur->result) = *((int*) ptr_one) - *((double*) ptr_two);
		}
		else if (ptr_one_type == PTR_TYPE_REAL && ptr_two_type == PTR_TYPE_INT)
		{
			cur->result = (double*) myMalloc(sizeof(double), NULL, malloced_head, the_debug);
			if (cur->result == NULL)
			{
				if (the_debug == YES_DEBUG)
					printf("	ERROR in doMath() at line %d in %s\n", __LINE__, __FILE__);
				return RETURN_ERROR;
			}

			cur->result_type = PTR_TYPE_REAL;

			*((double*) cur->result) = *((double*) ptr_one) - *((int*) ptr_two);
		}
		else if (ptr_one_type == PTR_TYPE_DATE && ptr_two_type == PTR_TYPE_INT)
		{
			int_8 date = dateToInt(ptr_one);

			date -= *((int*) ptr_two);

			cur->result = intToDate(date, NULL, malloced_head, the_debug);
			if (cur->result == NULL)
			{
				if (the_debug == YES_DEBUG)
					printf("	ERROR in doMath() at line %d in %s\n", __LINE__, __FILE__);
				return RETURN_ERROR;
			}

			cur->result_type = PTR_TYPE_DATE;
		}
		else if (ptr_one_type == PTR_TYPE_INT && ptr_two_type == PTR_TYPE_DATE)
		{
			int_8 date = dateToInt(ptr_two);

			date -= *((int*) ptr_one);

			cur->result = intToDate(date, NULL, malloced_head, the_debug);
			if (cur->result == NULL)
			{
				if (the_debug == YES_DEBUG)
					printf("	ERROR in doMath() at line %d in %s\n", __LINE__, __FILE__);
				return RETURN_ERROR;
			}

			cur->result_type = PTR_TYPE_DATE;
		}
		else if (ptr_one_type == PTR_TYPE_DATE && ptr_two_type == PTR_TYPE_DATE)
		{
			int_8 date1 = dateToInt(ptr_one);
			int_8 date2 = dateToInt(ptr_two);

			date1 = date1 - date2;

			cur->result = intToDate(date1, NULL, malloced_head, the_debug);
			if (cur->result == NULL)
			{
				if (the_debug == YES_DEBUG)
					printf("	ERROR in doMath() at line %d in %s\n", __LINE__, __FILE__);
				return RETURN_ERROR;
			}

			cur->result_type = PTR_TYPE_DATE;
		}
	}
	else if (cur->operation == MATH_MULT)
	{
		if (ptr_one_type == PTR_TYPE_INT && ptr_two_type == PTR_TYPE_INT)
		{
			cur->result = (int*) myMalloc(sizeof(int), NULL, malloced_head, the_debug);
			if (cur->result == NULL)
			{
				if (the_debug == YES_DEBUG)
					printf("	ERROR in doMath() at line %d in %s\n", __LINE__, __FILE__);
				return RETURN_ERROR;
			}

			cur->result_type = PTR_TYPE_INT;

			*((int*) cur->result) = (*((int*) ptr_one)) * (*((int*) ptr_two));
		}
		else if (ptr_one_type == PTR_TYPE_REAL && ptr_two_type == PTR_TYPE_REAL)
		{
			cur->result = (double*) myMalloc(sizeof(double), NULL, malloced_head, the_debug);
			if (cur->result == NULL)
			{
				if (the_debug == YES_DEBUG)
					printf("	ERROR in doMath() at line %d in %s\n", __LINE__, __FILE__);
				return RETURN_ERROR;
			}

			cur->result_type = PTR_TYPE_REAL;

			*((double*) cur->result) = (*((double*) ptr_one)) * (*((double*) ptr_two));
		}
		else if (ptr_one_type == PTR_TYPE_INT && ptr_two_type == PTR_TYPE_REAL)
		{
			cur->result = (double*) myMalloc(sizeof(double), NULL, malloced_head, the_debug);
			if (cur->result == NULL)
			{
				if (the_debug == YES_DEBUG)
					printf("	ERROR in doMath() at line %d in %s\n", __LINE__, __FILE__);
				return RETURN_ERROR;
			}

			cur->result_type = PTR_TYPE_REAL;

			*((double*) cur->result) = (*((int*) ptr_one)) * (*((double*) ptr_two));
		}
		else if (ptr_one_type == PTR_TYPE_REAL && ptr_two_type == PTR_TYPE_INT)
		{
			cur->result = (double*) myMalloc(sizeof(double), NULL, malloced_head, the_debug);
			if (cur->result == NULL)
			{
				if (the_debug == YES_DEBUG)
					printf("	ERROR in doMath() at line %d in %s\n", __LINE__, __FILE__);
				return RETURN_ERROR;
			}

			cur->result_type = PTR_TYPE_REAL;

			*((double*) cur->result) = (*((double*) ptr_one)) * (*((int*) ptr_two));
		}
	}
	else if (cur->operation == MATH_DIV)
	{
		if (ptr_one_type == PTR_TYPE_INT && ptr_two_type == PTR_TYPE_INT)
		{
			cur->result = (double*) myMalloc(sizeof(double), NULL, malloced_head, the_debug);
			if (cur->result == NULL)
			{
				if (the_debug == YES_DEBUG)
					printf("	ERROR in doMath() at line %d in %s\n", __LINE__, __FILE__);
				return RETURN_ERROR;
			}

			cur->result_type = PTR_TYPE_REAL;

			double one = *((int*) ptr_one);
			double two = *((int*) ptr_two);

			*((double*) cur->result) = one / two;
		}
		else if (ptr_one_type == PTR_TYPE_REAL && ptr_two_type == PTR_TYPE_REAL)
		{
			cur->result = (double*) myMalloc(sizeof(double), NULL, malloced_head, the_debug);
			if (cur->result == NULL)
			{
				if (the_debug == YES_DEBUG)
					printf("	ERROR in doMath() at line %d in %s\n", __LINE__, __FILE__);
				return RETURN_ERROR;
			}

			cur->result_type = PTR_TYPE_REAL;

			*((double*) cur->result) = *((double*) ptr_one) / *((double*) ptr_two);
		}
		else if (ptr_one_type == PTR_TYPE_INT && ptr_two_type == PTR_TYPE_REAL)
		{
			cur->result = (double*) myMalloc(sizeof(double), NULL, malloced_head, the_debug);
			if (cur->result == NULL)
			{
				if (the_debug == YES_DEBUG)
					printf("	ERROR in doMath() at line %d in %s\n", __LINE__, __FILE__);
				return RETURN_ERROR;
			}

			cur->result_type = PTR_TYPE_REAL;

			double one = *((int*) ptr_one);

			*((double*) cur->result) = one / *((double*) ptr_two);
		}
		else if (ptr_one_type == PTR_TYPE_REAL && ptr_two_type == PTR_TYPE_INT)
		{
			cur->result = (double*) myMalloc(sizeof(double), NULL, malloced_head, the_debug);
			if (cur->result == NULL)
			{
				if (the_debug == YES_DEBUG)
					printf("	ERROR in doMath() at line %d in %s\n", __LINE__, __FILE__);
				return RETURN_ERROR;
			}

			cur->result_type = PTR_TYPE_REAL;

			double two = *((int*) ptr_two);

			*((double*) cur->result) = *((double*) ptr_one) / two;
		}
	}
	else if (cur->operation == MATH_POW)
	{
		if (ptr_one_type == PTR_TYPE_INT && ptr_two_type == PTR_TYPE_INT)
		{
			cur->result = (int*) myMalloc(sizeof(int), NULL, malloced_head, the_debug);
			if (cur->result == NULL)
			{
				if (the_debug == YES_DEBUG)
					printf("	ERROR in doMath() at line %d in %s\n", __LINE__, __FILE__);
				return RETURN_ERROR;
			}

			cur->result_type = PTR_TYPE_INT;

			*((int*) cur->result) = pow(*((int*) ptr_one), *((int*) ptr_two));
		}
		else if (ptr_one_type == PTR_TYPE_REAL && ptr_two_type == PTR_TYPE_REAL)
		{
			cur->result = (double*) myMalloc(sizeof(double), NULL, malloced_head, the_debug);
			if (cur->result == NULL)
			{
				if (the_debug == YES_DEBUG)
					printf("	ERROR in doMath() at line %d in %s\n", __LINE__, __FILE__);
				return RETURN_ERROR;
			}

			cur->result_type = PTR_TYPE_REAL;

			*((double*) cur->result) = pow(*((double*) ptr_one), *((double*) ptr_two));
		}
		else if (ptr_one_type == PTR_TYPE_INT && ptr_two_type == PTR_TYPE_REAL)
		{
			cur->result = (double*) myMalloc(sizeof(double), NULL, malloced_head, the_debug);
			if (cur->result == NULL)
			{
				if (the_debug == YES_DEBUG)
					printf("	ERROR in doMath() at line %d in %s\n", __LINE__, __FILE__);
				return RETURN_ERROR;
			}

			cur->result_type = PTR_TYPE_REAL;

			*((double*) cur->result) = pow(*((int*) ptr_one), *((double*) ptr_two));
		}
		else if (ptr_one_type == PTR_TYPE_REAL && ptr_two_type == PTR_TYPE_INT)
		{
			cur->result = (double*) myMalloc(sizeof(double), NULL, malloced_head, the_debug);
			if (cur->result == NULL)
			{
				if (the_debug == YES_DEBUG)
					printf("	ERROR in doMath() at line %d in %s\n", __LINE__, __FILE__);
				return RETURN_ERROR;
			}

			cur->result_type = PTR_TYPE_REAL;

			*((double*) cur->result) = pow(*((double*) ptr_one), *((int*) ptr_two));
		}
	}

	return RETURN_GOOD;
}


struct result_node* doMathV2(struct math_node* cur, void* ptr_one, int ptr_one_type, void* ptr_two, int ptr_two_type, struct malloced_node** malloced_head, int the_debug)
{
	struct result_node* result = myMalloc(sizeof(struct result_node), NULL, malloced_head, the_debug);
	if (result == NULL)
	{
		if (the_debug == YES_DEBUG)
			printf("	ERROR in doMathV2() at line %d in %s\n", __LINE__, __FILE__);
		return NULL;
	}

	if (ptr_one == NULL || ptr_two == NULL)
	{
		result->result = NULL;
		result->result_type = ptr_one_type;

		return result;
	}

	if (cur->operation == MATH_ADD)
	{
		if (ptr_one_type == PTR_TYPE_INT && ptr_two_type == PTR_TYPE_INT)
		{
			result->result = (int*) myMalloc(sizeof(int), NULL, malloced_head, the_debug);
			if (result->result == NULL)
			{
				if (the_debug == YES_DEBUG)
					printf("	ERROR in doMath() at line %d in %s\n", __LINE__, __FILE__);
				return NULL;
			}

			result->result_type = PTR_TYPE_INT;

			*((int*) result->result) = *((int*) ptr_one) + *((int*) ptr_two);
		}
		else if (ptr_one_type == PTR_TYPE_REAL && ptr_two_type == PTR_TYPE_REAL)
		{
			result->result = (double*) myMalloc(sizeof(double), NULL, malloced_head, the_debug);
			if (result->result == NULL)
			{
				if (the_debug == YES_DEBUG)
					printf("	ERROR in doMath() at line %d in %s\n", __LINE__, __FILE__);
				return NULL;
			}

			result->result_type = PTR_TYPE_REAL;

			*((double*) result->result) = *((double*) ptr_one) + *((double*) ptr_two);
		}
		else if (ptr_one_type == PTR_TYPE_INT && ptr_two_type == PTR_TYPE_REAL)
		{
			result->result = (double*) myMalloc(sizeof(double), NULL, malloced_head, the_debug);
			if (result->result == NULL)
			{
				if (the_debug == YES_DEBUG)
					printf("	ERROR in doMath() at line %d in %s\n", __LINE__, __FILE__);
				return NULL;
			}

			result->result_type = PTR_TYPE_REAL;

			*((double*) result->result) = *((int*) ptr_one) + *((double*) ptr_two);
		}
		else if (ptr_one_type == PTR_TYPE_REAL && ptr_two_type == PTR_TYPE_INT)
		{
			result->result = (double*) myMalloc(sizeof(double), NULL, malloced_head, the_debug);
			if (result->result == NULL)
			{
				if (the_debug == YES_DEBUG)
					printf("	ERROR in doMath() at line %d in %s\n", __LINE__, __FILE__);
				return NULL;
			}

			result->result_type = PTR_TYPE_REAL;

			*((double*) result->result) = *((double*) ptr_one) + *((int*) ptr_two);
		}
		else if (ptr_one_type == PTR_TYPE_DATE && ptr_two_type == PTR_TYPE_INT)
		{
			int_8 date = dateToInt(ptr_one);

			date += *((int*) ptr_two);

			result->result = intToDate(date, NULL, malloced_head, the_debug);
			if (result->result == NULL)
			{
				if (the_debug == YES_DEBUG)
					printf("	ERROR in doMath() at line %d in %s\n", __LINE__, __FILE__);
				return NULL;
			}

			result->result_type = PTR_TYPE_DATE;
		}
		else if (ptr_one_type == PTR_TYPE_INT && ptr_two_type == PTR_TYPE_DATE)
		{
			int_8 date = dateToInt(ptr_two);

			date += *((int*) ptr_one);

			result->result = intToDate(date, NULL, malloced_head, the_debug);
			if (result->result == NULL)
			{
				if (the_debug == YES_DEBUG)
					printf("	ERROR in doMath() at line %d in %s\n", __LINE__, __FILE__);
				return NULL;
			}

			result->result_type = PTR_TYPE_DATE;
		}
		else if (ptr_one_type == PTR_TYPE_DATE && ptr_two_type == PTR_TYPE_DATE)
		{
			int_8 date1 = dateToInt(ptr_one);
			int_8 date2 = dateToInt(ptr_two);

			date1 = date1 + date2;

			result->result = intToDate(date1, NULL, malloced_head, the_debug);
			if (result->result == NULL)
			{
				if (the_debug == YES_DEBUG)
					printf("	ERROR in doMath() at line %d in %s\n", __LINE__, __FILE__);
				return NULL;
			}

			result->result_type = PTR_TYPE_DATE;
		}
	}
	else if (cur->operation == MATH_SUB)
	{
		if (ptr_one_type == PTR_TYPE_INT && ptr_two_type == PTR_TYPE_INT)
		{
			result->result = (int*) myMalloc(sizeof(int), NULL, malloced_head, the_debug);
			if (result->result == NULL)
			{
				if (the_debug == YES_DEBUG)
					printf("	ERROR in doMath() at line %d in %s\n", __LINE__, __FILE__);
				return NULL;
			}

			result->result_type = PTR_TYPE_INT;

			*((int*) result->result) = *((int*) ptr_one) - *((int*) ptr_two);
		}
		else if (ptr_one_type == PTR_TYPE_REAL && ptr_two_type == PTR_TYPE_REAL)
		{
			result->result = (double*) myMalloc(sizeof(double), NULL, malloced_head, the_debug);
			if (result->result == NULL)
			{
				if (the_debug == YES_DEBUG)
					printf("	ERROR in doMath() at line %d in %s\n", __LINE__, __FILE__);
				return NULL;
			}

			result->result_type = PTR_TYPE_REAL;

			*((double*) result->result) = *((double*) ptr_one) - *((double*) ptr_two);
		}
		else if (ptr_one_type == PTR_TYPE_INT && ptr_two_type == PTR_TYPE_REAL)
		{
			result->result = (double*) myMalloc(sizeof(double), NULL, malloced_head, the_debug);
			if (result->result == NULL)
			{
				if (the_debug == YES_DEBUG)
					printf("	ERROR in doMath() at line %d in %s\n", __LINE__, __FILE__);
				return NULL;
			}

			result->result_type = PTR_TYPE_REAL;

			*((double*) result->result) = *((int*) ptr_one) - *((double*) ptr_two);
		}
		else if (ptr_one_type == PTR_TYPE_REAL && ptr_two_type == PTR_TYPE_INT)
		{
			result->result = (double*) myMalloc(sizeof(double), NULL, malloced_head, the_debug);
			if (result->result == NULL)
			{
				if (the_debug == YES_DEBUG)
					printf("	ERROR in doMath() at line %d in %s\n", __LINE__, __FILE__);
				return NULL;
			}

			result->result_type = PTR_TYPE_REAL;

			*((double*) result->result) = *((double*) ptr_one) - *((int*) ptr_two);
		}
		else if (ptr_one_type == PTR_TYPE_DATE && ptr_two_type == PTR_TYPE_INT)
		{
			int_8 date = dateToInt(ptr_one);

			date -= *((int*) ptr_two);

			result->result = intToDate(date, NULL, malloced_head, the_debug);
			if (result->result == NULL)
			{
				if (the_debug == YES_DEBUG)
					printf("	ERROR in doMath() at line %d in %s\n", __LINE__, __FILE__);
				return NULL;
			}

			result->result_type = PTR_TYPE_DATE;
		}
		else if (ptr_one_type == PTR_TYPE_INT && ptr_two_type == PTR_TYPE_DATE)
		{
			int_8 date = dateToInt(ptr_two);

			date -= *((int*) ptr_one);

			result->result = intToDate(date, NULL, malloced_head, the_debug);
			if (result->result == NULL)
			{
				if (the_debug == YES_DEBUG)
					printf("	ERROR in doMath() at line %d in %s\n", __LINE__, __FILE__);
				return NULL;
			}

			result->result_type = PTR_TYPE_DATE;
		}
		else if (ptr_one_type == PTR_TYPE_DATE && ptr_two_type == PTR_TYPE_DATE)
		{
			int_8 date1 = dateToInt(ptr_one);
			int_8 date2 = dateToInt(ptr_two);

			date1 = date1 - date2;

			result->result = intToDate(date1, NULL, malloced_head, the_debug);
			if (result->result == NULL)
			{
				if (the_debug == YES_DEBUG)
					printf("	ERROR in doMath() at line %d in %s\n", __LINE__, __FILE__);
				return NULL;
			}

			result->result_type = PTR_TYPE_DATE;
		}
	}
	else if (cur->operation == MATH_MULT)
	{
		if (ptr_one_type == PTR_TYPE_INT && ptr_two_type == PTR_TYPE_INT)
		{
			result->result = (int*) myMalloc(sizeof(int), NULL, malloced_head, the_debug);
			if (result->result == NULL)
			{
				if (the_debug == YES_DEBUG)
					printf("	ERROR in doMath() at line %d in %s\n", __LINE__, __FILE__);
				return NULL;
			}

			result->result_type = PTR_TYPE_INT;

			*((int*) result->result) = (*((int*) ptr_one)) * (*((int*) ptr_two));
		}
		else if (ptr_one_type == PTR_TYPE_REAL && ptr_two_type == PTR_TYPE_REAL)
		{
			result->result = (double*) myMalloc(sizeof(double), NULL, malloced_head, the_debug);
			if (result->result == NULL)
			{
				if (the_debug == YES_DEBUG)
					printf("	ERROR in doMath() at line %d in %s\n", __LINE__, __FILE__);
				return NULL;
			}

			result->result_type = PTR_TYPE_REAL;

			*((double*) result->result) = (*((double*) ptr_one)) * (*((double*) ptr_two));
		}
		else if (ptr_one_type == PTR_TYPE_INT && ptr_two_type == PTR_TYPE_REAL)
		{
			result->result = (double*) myMalloc(sizeof(double), NULL, malloced_head, the_debug);
			if (result->result == NULL)
			{
				if (the_debug == YES_DEBUG)
					printf("	ERROR in doMath() at line %d in %s\n", __LINE__, __FILE__);
				return NULL;
			}

			result->result_type = PTR_TYPE_REAL;

			*((double*) result->result) = (*((int*) ptr_one)) * (*((double*) ptr_two));
		}
		else if (ptr_one_type == PTR_TYPE_REAL && ptr_two_type == PTR_TYPE_INT)
		{
			result->result = (double*) myMalloc(sizeof(double), NULL, malloced_head, the_debug);
			if (result->result == NULL)
			{
				if (the_debug == YES_DEBUG)
					printf("	ERROR in doMath() at line %d in %s\n", __LINE__, __FILE__);
				return NULL;
			}

			result->result_type = PTR_TYPE_REAL;

			*((double*) result->result) = (*((double*) ptr_one)) * (*((int*) ptr_two));
		}
	}
	else if (cur->operation == MATH_DIV)
	{
		if (ptr_one_type == PTR_TYPE_INT && ptr_two_type == PTR_TYPE_INT)
		{
			result->result = (double*) myMalloc(sizeof(double), NULL, malloced_head, the_debug);
			if (result->result == NULL)
			{
				if (the_debug == YES_DEBUG)
					printf("	ERROR in doMath() at line %d in %s\n", __LINE__, __FILE__);
				return NULL;
			}

			result->result_type = PTR_TYPE_REAL;

			double one = *((int*) ptr_one);
			double two = *((int*) ptr_two);

			*((double*) result->result) = one / two;
		}
		else if (ptr_one_type == PTR_TYPE_REAL && ptr_two_type == PTR_TYPE_REAL)
		{
			result->result = (double*) myMalloc(sizeof(double), NULL, malloced_head, the_debug);
			if (result->result == NULL)
			{
				if (the_debug == YES_DEBUG)
					printf("	ERROR in doMath() at line %d in %s\n", __LINE__, __FILE__);
				return NULL;
			}

			result->result_type = PTR_TYPE_REAL;

			*((double*) result->result) = *((double*) ptr_one) / *((double*) ptr_two);
		}
		else if (ptr_one_type == PTR_TYPE_INT && ptr_two_type == PTR_TYPE_REAL)
		{
			result->result = (double*) myMalloc(sizeof(double), NULL, malloced_head, the_debug);
			if (result->result == NULL)
			{
				if (the_debug == YES_DEBUG)
					printf("	ERROR in doMath() at line %d in %s\n", __LINE__, __FILE__);
				return NULL;
			}

			result->result_type = PTR_TYPE_REAL;

			double one = *((int*) ptr_one);

			*((double*) result->result) = one / *((double*) ptr_two);
		}
		else if (ptr_one_type == PTR_TYPE_REAL && ptr_two_type == PTR_TYPE_INT)
		{
			result->result = (double*) myMalloc(sizeof(double), NULL, malloced_head, the_debug);
			if (result->result == NULL)
			{
				if (the_debug == YES_DEBUG)
					printf("	ERROR in doMath() at line %d in %s\n", __LINE__, __FILE__);
				return NULL;
			}

			result->result_type = PTR_TYPE_REAL;

			double two = *((int*) ptr_two);

			*((double*) result->result) = *((double*) ptr_one) / two;
		}
	}
	else if (cur->operation == MATH_POW)
	{
		if (ptr_one_type == PTR_TYPE_INT && ptr_two_type == PTR_TYPE_INT)
		{
			result->result = (int*) myMalloc(sizeof(int), NULL, malloced_head, the_debug);
			if (result->result == NULL)
			{
				if (the_debug == YES_DEBUG)
					printf("	ERROR in doMath() at line %d in %s\n", __LINE__, __FILE__);
				return NULL;
			}

			result->result_type = PTR_TYPE_INT;

			*((int*) result->result) = pow(*((int*) ptr_one), *((int*) ptr_two));
		}
		else if (ptr_one_type == PTR_TYPE_REAL && ptr_two_type == PTR_TYPE_REAL)
		{
			result->result = (double*) myMalloc(sizeof(double), NULL, malloced_head, the_debug);
			if (result->result == NULL)
			{
				if (the_debug == YES_DEBUG)
					printf("	ERROR in doMath() at line %d in %s\n", __LINE__, __FILE__);
				return NULL;
			}

			result->result_type = PTR_TYPE_REAL;

			*((double*) result->result) = pow(*((double*) ptr_one), *((double*) ptr_two));
		}
		else if (ptr_one_type == PTR_TYPE_INT && ptr_two_type == PTR_TYPE_REAL)
		{
			result->result = (double*) myMalloc(sizeof(double), NULL, malloced_head, the_debug);
			if (result->result == NULL)
			{
				if (the_debug == YES_DEBUG)
					printf("	ERROR in doMath() at line %d in %s\n", __LINE__, __FILE__);
				return NULL;
			}

			result->result_type = PTR_TYPE_REAL;

			*((double*) result->result) = pow(*((int*) ptr_one), *((double*) ptr_two));
		}
		else if (ptr_one_type == PTR_TYPE_REAL && ptr_two_type == PTR_TYPE_INT)
		{
			result->result = (double*) myMalloc(sizeof(double), NULL, malloced_head, the_debug);
			if (result->result == NULL)
			{
				if (the_debug == YES_DEBUG)
					printf("	ERROR in doMath() at line %d in %s\n", __LINE__, __FILE__);
				return NULL;
			}

			result->result_type = PTR_TYPE_REAL;

			*((double*) result->result) = pow(*((double*) ptr_one), *((int*) ptr_two));
		}
	}

	return result;
}

/*	RETURNS:
 *  RETURN_ERROR if error
 *	0 if good and false
 *  1 if good and true
 *
 *	WRITES TO:
 */
int doWhere(void* ptr_one, void* ptr_two, int ptr_one_type, int ptr_two_type, int where_type, int_8 row_id, int_8 second_row_id, struct ListNodePtr* head
		   ,struct ListNodePtr* tail, struct malloced_node** malloced_head, int the_debug)
{
	// START Get specific data pointers
		int col_data_type = -1;
		void* temp_ptr_one = NULL;
		void* temp_ptr_two = NULL;
		struct result_node* result_node_1 = NULL;
		struct result_node* result_node_2 = NULL;
		struct result_node* case_result_1 = NULL;
		struct result_node* case_result_2 = NULL;

		if (ptr_one_type == PTR_TYPE_COL_IN_SELECT_NODE)
		{
			if (((int) second_row_id) > -1)
			{
				//printf("second_row_id is valid\n");
				struct ListNodePtr* cur_join_col = head;
				while (cur_join_col != NULL)
				{
					//printf("Comparing %p and %p\n", cur_join_col->ptr_value, ptr_one);
					if (cur_join_col->ptr_value == ptr_one)
					{
						int_8 index = row_id;
						if (cur_join_col->ptr_type == COL_IN_JOIN_IS_RIGHT)
							index = second_row_id;

						//printf("Found col for ptr_one\n");

						col_data_type = ((struct col_in_select_node*) ptr_one)->rows_data_type;
						temp_ptr_one = ((struct col_in_select_node*) ptr_one)->col_data_arr[index]->row_data;

						break;
					}

					cur_join_col = cur_join_col->next;
				}

				if (cur_join_col == NULL)
				{
					if (the_debug == YES_DEBUG)
						printf("	ERROR in doWhere() at line %d in %s\n", __LINE__, __FILE__);
					errorTeardown(NULL, malloced_head, the_debug);
					return RETURN_ERROR;
				}
			}
			else
			{
				col_data_type = ((struct col_in_select_node*) ptr_one)->rows_data_type;
				temp_ptr_one = ((struct col_in_select_node*) ptr_one)->col_data_arr[row_id]->row_data;
			}
		}
		else if (ptr_one_type == PTR_TYPE_MATH_NODE)
		{
			result_node_1 = evaluateMathTreeV2(head, tail, ptr_one, row_id, second_row_id, malloced_head, the_debug);

			if (result_node_1 == NULL)
			{
				if (the_debug == YES_DEBUG)
					printf("	ERROR in doWhere() at line %d in %s\n", __LINE__, __FILE__);
				return RETURN_ERROR;
			}

			col_data_type = result_node_1->result_type;
			temp_ptr_one = result_node_1->result;

			//if (evaluateMathTree(head, tail, ptr_one, row_id, second_row_id, malloced_head, the_debug) < 0)
			//{
			//	if (the_debug == YES_DEBUG)
			//		printf("	ERROR in doWhere() at line %d in %s\n", __LINE__, __FILE__);
			//	return RETURN_ERROR;
			//}

			//col_data_type = ((struct math_node*) ptr_one)->result_type;
			//temp_ptr_one = ((struct math_node*) ptr_one)->result;
		}
		else if (ptr_one_type == PTR_TYPE_FUNC_NODE)
		{
			//traverseListNodesPtr(&((struct group_data_node*) head->ptr_value)->row_ids_head, NULL, TRAVERSELISTNODES_HEAD, "cur_row: ");
			struct ListNodePtr* cur_row = head;
			int count = 0;
			while (count < row_id)
			{
				count++;
				cur_row = cur_row->next;
			}

			if (the_debug == YES_DEBUG && row_id == 13)
			{
				printf("Done checking row_id = %d\n", row_id);
				printf("first cur_row row_id = %d\n", *((int*) ((struct group_data_node*) cur_row->ptr_value)->row_ids_head->ptr_value));
			}

			result_node_1 = calcResultOfFuncForOneRowV2(cur_row, ptr_one, NULL, -1, malloced_head, the_debug);

			if (result_node_1 == NULL)
			{
				if (the_debug == YES_DEBUG)
					printf("	ERROR in doWhere() at line %d in %s\n", __LINE__, __FILE__);
				return RETURN_ERROR;
			}

			col_data_type = result_node_1->result_type;
			temp_ptr_one = result_node_1->result;

			if (the_debug == YES_DEBUG && row_id == 13)
				printf("row_id = %d, temp_ptr_one = %lf\n", row_id, *((double*) temp_ptr_one));
		}
		else if (ptr_one_type == PTR_TYPE_CASE_NODE)
		{
			case_result_1 = calcResultOfCaseForOneRowV2(ptr_one, row_id, second_row_id, NULL, NULL, false, -1, -1, head, tail, malloced_head, the_debug);
			if (case_result_1 == NULL)
			{
				if (the_debug == YES_DEBUG)
					printf("	ERROR in evaluateMathTree() at line %d in %s\n", __LINE__, __FILE__);
				return RETURN_ERROR;
			}

			col_data_type = case_result_1->result_type;
			temp_ptr_one = case_result_1->result;
		}
		else if (ptr_one_type == PTR_TYPE_TABLE_COLS_INFO)
		{
			col_data_type = ((struct table_cols_info*) ptr_one)->data_type;

			struct ListNodePtr* cur_col = head;
			while (cur_col != NULL)
			{
				if (((struct table_cols_info*) ptr_one)->col_number == cur_col->ptr_type)
				{
					//printf("*row_id = %lu\n", row_id);
					temp_ptr_one = ((struct colDataNode**) cur_col->ptr_value)[row_id]->row_data;
					break;
				}

				cur_col = cur_col->next;
			}

			if (cur_col == NULL)
			{
				if (the_debug == YES_DEBUG)
					printf("	ERROR in doWhere() at line %d in %s\n", __LINE__, __FILE__);
				return RETURN_ERROR;
			}
		}
		else
		{
			col_data_type = ptr_one_type;
			temp_ptr_one = ptr_one;
		}



		if (ptr_two_type == PTR_TYPE_COL_IN_SELECT_NODE)
		{
			if (((int) second_row_id) > -1)
			{
				struct ListNodePtr* cur_join_col = head;
				while (cur_join_col != NULL)
				{
					if (cur_join_col->ptr_value == ptr_two)
					{
						int_8 index = row_id;
						if (cur_join_col->ptr_type == COL_IN_JOIN_IS_RIGHT)
							index = second_row_id;

						//printf("Found col for ptr_two\n");

						temp_ptr_two = ((struct col_in_select_node*) ptr_two)->col_data_arr[index]->row_data;
						
						break;
					}

					cur_join_col = cur_join_col->next;
				}

				if (cur_join_col == NULL)
				{
					if (the_debug == YES_DEBUG)
						printf("	ERROR in doWhere() at line %d in %s\n", __LINE__, __FILE__);
					errorTeardown(NULL, malloced_head, the_debug);
					return RETURN_ERROR;
				}
			}
			else
				temp_ptr_two = ((struct col_in_select_node*) ptr_two)->col_data_arr[row_id]->row_data;
		}
		else if (ptr_two_type == PTR_TYPE_MATH_NODE)
		{
			result_node_2 = evaluateMathTreeV2(head, tail, ptr_two, row_id, second_row_id, malloced_head, the_debug);

			if (result_node_2 == NULL)
			{
				if (the_debug == YES_DEBUG)
					printf("	ERROR in doWhere() at line %d in %s\n", __LINE__, __FILE__);
				return RETURN_ERROR;
			}

			temp_ptr_two = result_node_2->result;

			//if (evaluateMathTree(head, tail, ptr_two, row_id, second_row_id, malloced_head, the_debug) < 0)
			//{
			//	if (the_debug == YES_DEBUG)
			//		printf("	ERROR in doWhere() at line %d in %s\n", __LINE__, __FILE__);
			//	return RETURN_ERROR;
			//}

			//temp_ptr_two = ((struct math_node*) ptr_two)->result;
		}
		else if (ptr_two_type == PTR_TYPE_FUNC_NODE)
		{
			struct ListNodePtr* cur_row = head;
			int count = 0;
			while (count < row_id)
			{
				count++;
				cur_row = cur_row->next;
			}

			result_node_2 = calcResultOfFuncForOneRowV2(cur_row, ptr_two, NULL, -1, malloced_head, the_debug);

			if (result_node_2 == NULL)
			{
				if (the_debug == YES_DEBUG)
					printf("	ERROR in doWhere() at line %d in %s\n", __LINE__, __FILE__);
				return RETURN_ERROR;
			}

			temp_ptr_two = result_node_2->result;
		}
		else if (ptr_two_type == PTR_TYPE_CASE_NODE)
		{
			case_result_2 = calcResultOfCaseForOneRowV2(ptr_two, row_id, second_row_id, NULL, NULL, false, -1, -1, head, tail, malloced_head, the_debug);
			if (case_result_2 == NULL)
			{
				if (the_debug == YES_DEBUG)
					printf("	ERROR in evaluateMathTree() at line %d in %s\n", __LINE__, __FILE__);
				return RETURN_ERROR;
			}

			temp_ptr_two = case_result_2->result;
		}
		else if (ptr_two_type == PTR_TYPE_TABLE_COLS_INFO)
		{
			struct ListNodePtr* cur_col = head;
			while (cur_col != NULL)
			{
				if (((struct table_cols_info*) ptr_two)->col_number == cur_col->ptr_type)
				{
					temp_ptr_two = ((struct colDataNode**) cur_col->ptr_value)[row_id]->row_data;

					break;
				}

				cur_col = cur_col->next;
			}

			if (cur_col == NULL)
			{
				if (the_debug == YES_DEBUG)
					printf("	ERROR in doWhere() at line %d in %s\n", __LINE__, __FILE__);
				return RETURN_ERROR;
			}
		}
		else
		{
			temp_ptr_two = ptr_two;
		}
	// END Get specific data pointers

	if (the_debug == YES_DEBUG && 1 == 0)
	{
		printf("col_data_type = %d\n", col_data_type);
		if (col_data_type == DATA_INT)
			printf("Comparing %d and %d\n", temp_ptr_one == NULL ? -1 : *((int*) temp_ptr_one), temp_ptr_two == NULL ? -1 : *((int*) temp_ptr_two));
		else if (col_data_type == DATA_REAL)
			printf("Comparing %lf and %lf\n", temp_ptr_one == NULL ? -1.0 : *((double*) temp_ptr_one), temp_ptr_two == NULL ? -1 : *((double*) temp_ptr_two));
		else if (col_data_type == DATA_STRING || col_data_type == DATA_DATE)
			printf("Comparing _%s_ and _%s_\n", temp_ptr_one, temp_ptr_two);
	}

	// START Evaluate inequality
		int result;

		if (where_type == WHERE_IS_EQUALS)
		{
			if (equals(temp_ptr_one, col_data_type, temp_ptr_two, VALUE_EQUALS))
				result = 1;
			else
				result = 0;
		}
		else if (where_type == WHERE_NOT_EQUALS)
		{
			if (!equals(temp_ptr_one, col_data_type, temp_ptr_two, VALUE_EQUALS))
				result = 1;
			else
				result = 0;
		}
		else if (where_type == WHERE_GREATER_THAN || where_type == WHERE_GREATER_THAN_OR_EQUAL ||
				 where_type == WHERE_LESS_THAN || where_type == WHERE_LESS_THAN_OR_EQUAL)
		{
			bool is_true = greatLess(temp_ptr_one, col_data_type, temp_ptr_two, where_type);

			if ((is_true && where_type == WHERE_GREATER_THAN)
				|| (is_true && where_type == WHERE_GREATER_THAN_OR_EQUAL)
				|| (is_true && where_type == WHERE_LESS_THAN)
				|| (is_true && where_type == WHERE_LESS_THAN_OR_EQUAL))
				result = 1;
			else
				result = 0;
		}
		else if (where_type == WHERE_IS_NULL)
		{
			if (temp_ptr_one == NULL)
				result = 1;
			else
				result = 0;
		}
		else if (where_type == WHERE_IS_NOT_NULL)
		{
			if (temp_ptr_one != NULL)
				result = 1;
			else
				result = 0;
		}
	// END Evaluate inequality

	if (ptr_one_type == PTR_TYPE_MATH_NODE || ptr_one_type == PTR_TYPE_FUNC_NODE)
		myFree((void**) &temp_ptr_one, NULL, malloced_head, the_debug);

	if (ptr_one_type == PTR_TYPE_MATH_NODE)
		myFree((void**) &result_node_1, NULL, malloced_head, the_debug);

	if (ptr_one_type == PTR_TYPE_FUNC_NODE)
		myFree((void**) &result_node_1, NULL, malloced_head, the_debug);

	if (ptr_one_type == PTR_TYPE_CASE_NODE)
		myFree((void**) &case_result_1, NULL, malloced_head, the_debug);

	if (ptr_two_type == PTR_TYPE_MATH_NODE || ptr_two_type == PTR_TYPE_FUNC_NODE)
		myFree((void**) &temp_ptr_two, NULL, malloced_head, the_debug);

	if (ptr_two_type == PTR_TYPE_MATH_NODE)
		myFree((void**) &result_node_2, NULL, malloced_head, the_debug);

	if (ptr_two_type == PTR_TYPE_FUNC_NODE)
		myFree((void**) &result_node_2, NULL, malloced_head, the_debug);

	if (ptr_two_type == PTR_TYPE_CASE_NODE)
		myFree((void**) &case_result_2, NULL, malloced_head, the_debug);

	return result;
}

/*	RETURNS:
 *  RETURN_ERROR if error
 *	0 if good and false
 *  1 if good and true
 *
 *	WRITES TO:
 */
int evaluateWhereTreeForOneRow(struct where_clause_node* cur, int_8 row_id, int_8 second_row_id, struct ListNodePtr* head, struct ListNodePtr* tail
							  ,struct malloced_node** malloced_head, int the_debug)
{
	if (cur->ptr_one_type == PTR_TYPE_WHERE_CLAUSE_NODE && cur->ptr_two_type == PTR_TYPE_WHERE_CLAUSE_NODE)
	{
		int one = evaluateWhereTreeForOneRow(cur->ptr_one, row_id, second_row_id, head, tail, malloced_head, the_debug);
		if (one == RETURN_ERROR)
		{
			if (the_debug == YES_DEBUG)
				printf("	ERROR in evaluateWhereTreeForOneRow() at line %d in %s\n", __LINE__, __FILE__);
			return RETURN_ERROR;
		}

		int two = evaluateWhereTreeForOneRow(cur->ptr_two, row_id, second_row_id, head, tail, malloced_head, the_debug);
		if (two == RETURN_ERROR)
		{
			if (the_debug == YES_DEBUG)
				printf("	ERROR in evaluateWhereTreeForOneRow() at line %d in %s\n", __LINE__, __FILE__);
			return RETURN_ERROR;
		}

		if (cur->where_type == WHERE_OR)
		{
			return one == 1 || two == 1;
		}
		else //if (cur->where_type == WHERE_AND)
		{
			return one == 1 && two == 1;
		}
	}
	else
	{
		if (cur->ptr_one_type == PTR_TYPE_CHAR)
			undoDoubleQuotes(cur->ptr_one);
		if (cur->ptr_two_type == PTR_TYPE_CHAR)
			undoDoubleQuotes(cur->ptr_two);

		//printf("calling doWhere(): %lu, %lu\n", row_id, second_row_id);
		return doWhere(cur->ptr_one, cur->ptr_two, cur->ptr_one_type, cur->ptr_two_type, cur->where_type, row_id, second_row_id, head, tail, malloced_head, the_debug);
	}
}

/*	RETURNS:
 *  RETURN_ERROR if error
 *	RETURN_GOOD if good
 *
 *	WRITES TO:
 *  the_case_node->result
 *  the_case_node->result_type
 */
int calcResultOfCaseForOneRow(struct case_node* the_case_node, int_8 row_id, int_8 second_row_id, struct select_node* the_select_node, struct table_info* the_table, bool join
							 ,int_8 left_num_rows, int_8 right_num_rows, struct ListNodePtr* head, struct ListNodePtr* tail
							 ,struct malloced_node** malloced_head, int the_debug)
{
	struct ListNodePtr* cur_when = the_case_node->case_when_head;
	struct ListNodePtr* cur_then = the_case_node->case_then_value_head;
	while (cur_when != NULL)
	{
		bool matches = false;

		if (cur_when->ptr_value != NULL)
		{
			//printf("calling evaluateWhereTreeForOneRow(): %lu, %lu\n", row_id, second_row_id);
			int result = evaluateWhereTreeForOneRow(cur_when->ptr_value, row_id, second_row_id, head, tail, malloced_head, the_debug);
			if (result == RETURN_ERROR)
			{
				if (the_debug == YES_DEBUG)
					printf("	ERROR in calcResultOfCaseForOneRow() at line %d in %s\n", __LINE__, __FILE__);
				return RETURN_ERROR;
			}

			matches = result == 1;
		}

		if (cur_when->ptr_value == NULL || matches)
		{
			the_case_node->result = cur_then->ptr_value;
			the_case_node->result_type = cur_then->ptr_type;

			if (the_case_node->result_type == PTR_TYPE_COL_IN_SELECT_NODE)
			{
				//printf("Getting row value from col in case then\n");
				if (((int) second_row_id) > -1)
				{
					//printf("second_row_id is valid\n");
					struct ListNodePtr* cur_join_col = head;
					while (cur_join_col != NULL)
					{
						if (cur_join_col->ptr_value == cur_then->ptr_value)
						{
							int_8 index = row_id;
							if (cur_join_col->ptr_type == COL_IN_JOIN_IS_RIGHT)
								index = second_row_id;

							//printf("Found col for cur_then->ptr_value\n");

							the_case_node->result_type = ((struct col_in_select_node*) cur_then->ptr_value)->rows_data_type;
							the_case_node->result = ((struct col_in_select_node*) cur_then->ptr_value)->col_data_arr[index]->row_data;

							//printf("the_case_node->result_type = %d\n", the_case_node->result_type);
							//printf("the_case_node->result = %s\n", the_case_node->result);

							break;
						}

						cur_join_col = cur_join_col->next;
					}

					if (cur_join_col == NULL)
					{
						if (the_debug == YES_DEBUG)
							printf("	ERROR in calcResultOfCaseForOneRow() at line %d in %s\n", __LINE__, __FILE__);
						return RETURN_ERROR;
					}
				}
				else
				{
					the_case_node->result_type = ((struct col_in_select_node*) cur_then->ptr_value)->rows_data_type;
					the_case_node->result = ((struct col_in_select_node*) cur_then->ptr_value)->col_data_arr[row_id]->row_data;
				}
			}

			break;
		}

		cur_when = cur_when->next;
		cur_then = cur_then->next;
	}

	return RETURN_GOOD;
}


struct result_node* calcResultOfCaseForOneRowV2(struct case_node* the_case_node, int_8 row_id, int_8 second_row_id, struct select_node* the_select_node, struct table_info* the_table
											 ,bool join, int_8 left_num_rows, int_8 right_num_rows, struct ListNodePtr* head, struct ListNodePtr* tail
											 ,struct malloced_node** malloced_head, int the_debug)
{
	struct result_node* result_node = myMalloc(sizeof(struct result_node), NULL, malloced_head, the_debug);
	if (result_node == NULL)
	{
		if (the_debug == YES_DEBUG)
			printf("	ERROR in doMathV2() at line %d in %s\n", __LINE__, __FILE__);
		return NULL;
	}

	struct ListNodePtr* cur_when = the_case_node->case_when_head;
	struct ListNodePtr* cur_then = the_case_node->case_then_value_head;
	while (cur_when != NULL)
	{
		bool matches = false;

		if (cur_when->ptr_value != NULL)
		{
			//printf("calling evaluateWhereTreeForOneRow(): %lu, %lu\n", row_id, second_row_id);
			int result = evaluateWhereTreeForOneRow(cur_when->ptr_value, row_id, second_row_id, head, tail, malloced_head, the_debug);
			if (result == RETURN_ERROR)
			{
				if (the_debug == YES_DEBUG)
					printf("	ERROR in calcResultOfCaseForOneRow() at line %d in %s\n", __LINE__, __FILE__);
				return RETURN_ERROR;
			}

			matches = result == 1;
		}

		if (cur_when->ptr_value == NULL || matches)
		{
			result_node->result = cur_then->ptr_value;
			result_node->result_type = cur_then->ptr_type;

			if (the_case_node->result_type == PTR_TYPE_COL_IN_SELECT_NODE)
			{
				//printf("Getting row value from col in case then\n");
				if (((int) second_row_id) > -1)
				{
					//printf("second_row_id is valid\n");
					struct ListNodePtr* cur_join_col = head;
					while (cur_join_col != NULL)
					{
						if (cur_join_col->ptr_value == cur_then->ptr_value)
						{
							int_8 index = row_id;
							if (cur_join_col->ptr_type == COL_IN_JOIN_IS_RIGHT)
								index = second_row_id;

							//printf("Found col for cur_then->ptr_value\n");

							result_node->result_type = ((struct col_in_select_node*) cur_then->ptr_value)->rows_data_type;
							result_node->result = ((struct col_in_select_node*) cur_then->ptr_value)->col_data_arr[index]->row_data;

							//printf("result_node->result_type = %d\n", result_node->result_type);
							//printf("result_node->result = %s\n", result_node->result);

							break;
						}

						cur_join_col = cur_join_col->next;
					}

					if (cur_join_col == NULL)
					{
						if (the_debug == YES_DEBUG)
							printf("	ERROR in calcResultOfCaseForOneRow() at line %d in %s\n", __LINE__, __FILE__);
						return RETURN_ERROR;
					}
				}
				else
				{
					result_node->result_type = ((struct col_in_select_node*) cur_then->ptr_value)->rows_data_type;
					result_node->result = ((struct col_in_select_node*) cur_then->ptr_value)->col_data_arr[row_id]->row_data;
				}
			}

			break;
		}

		cur_when = cur_when->next;
		cur_then = cur_then->next;
	}

	return result_node;
}

/*	RETURNS:
 *  RETURN_ERROR if error
 *	RETURN_GOOD if good
 *
 *	WRITES TO:
 */
int evaluateMathTree(struct ListNodePtr* head, struct ListNodePtr* tail, struct math_node* cur, int_8 row_id, int_8 second_row_id
					,struct malloced_node** malloced_head, int the_debug)
{
	void* ptr_one = NULL;
	int ptr_one_type = -1;

	void* ptr_two = NULL;
	int ptr_two_type = -1;


	int result = 0;


	// START Go deeper if more math_nodes
		if (cur->ptr_one_type == PTR_TYPE_MATH_NODE)
		{
			if ((result = evaluateMathTree(head, tail, (struct math_node*) cur->ptr_one, row_id, second_row_id, malloced_head, the_debug)) < 0)
			{
				if (the_debug == YES_DEBUG)
					printf("	ERROR in evaluateMathTree() at line %d in %s\n", __LINE__, __FILE__);
				return RETURN_ERROR;
			}

			ptr_one = ((struct math_node*) cur->ptr_one)->result;
			ptr_one_type = ((struct math_node*) cur->ptr_one)->result_type;
		}
		else if (cur->ptr_one_type == PTR_TYPE_COL_IN_SELECT_NODE)
		{
			if (((int) second_row_id) > -1)
			{
				struct ListNodePtr* cur_join_col = head;
				while (cur_join_col != NULL)
				{
					if (cur_join_col->ptr_value == cur->ptr_one)
					{
						int_8 index = row_id;
						if (cur_join_col->ptr_type == COL_IN_JOIN_IS_RIGHT)
							index = second_row_id;

						ptr_one = ((struct col_in_select_node*) cur->ptr_one)->col_data_arr[index]->row_data;
						ptr_one_type = ((struct col_in_select_node*) cur->ptr_one)->rows_data_type;

						break;
					}

					cur_join_col = cur_join_col->next;
				}
			}
			else
			{
				ptr_one = ((struct col_in_select_node*) cur->ptr_one)->col_data_arr[row_id]->row_data;
				ptr_one_type = ((struct col_in_select_node*) cur->ptr_one)->rows_data_type;
			}

			result = 1;
		}
		else if (cur->ptr_one_type == PTR_TYPE_FUNC_NODE)
		{
			struct ListNodePtr* cur_row = head;
			while (cur_row != NULL)
			{
				struct ListNodePtr* cur_group_row_id = ((struct group_data_node*) cur_row->ptr_value)->row_ids_head;
				while (cur_group_row_id != NULL)
				{
					if (*((int*) cur_group_row_id->ptr_value) == row_id)
						break;
					cur_group_row_id = cur_group_row_id->next;
				}

				if (cur_group_row_id != NULL)
					break;

				cur_row = cur_row->next;
			}

			//printf("first cur_row row_id = %d\n", *((int*) ((struct group_data_node*) cur_row->ptr_value)->row_ids_head->ptr_value));
			//printf("*((int*) cur_row_id->ptr_value) = %lu\n", *((int*) cur_row_id->ptr_value));

			if (calcResultOfFuncForOneRow(cur_row, cur->ptr_one, NULL, -1, malloced_head, the_debug) != RETURN_GOOD)
			{
				if (the_debug == YES_DEBUG)
					printf("	ERROR in execCaseNode() at line %d in %s\n", __LINE__, __FILE__);
				return RETURN_ERROR;
			}

			ptr_one = ((struct func_node*) cur->ptr_one)->result;
			ptr_one_type = ((struct func_node*) cur->ptr_one)->result_type;

			//printf("value at func result = %d\n", *((int*) ((struct func_node*) cur->ptr_one)->result));
			//printf("value at func result_type = %d\n", ((struct func_node*) cur->ptr_one)->result_type);

			result = 1;
		}
		else if (cur->ptr_one_type == PTR_TYPE_CASE_NODE)
		{
			if (calcResultOfCaseForOneRow(cur->ptr_one, row_id, second_row_id, NULL, NULL, ((int) second_row_id) > -1, -1, -1, head, tail, malloced_head, the_debug) != RETURN_GOOD)
			{
				if (the_debug == YES_DEBUG)
					printf("	ERROR in evaluateMathTree() at line %d in %s\n", __LINE__, __FILE__);
				return RETURN_ERROR;
			}

			ptr_one = ((struct case_node*) cur->ptr_one)->result;
			ptr_one_type = ((struct case_node*) cur->ptr_one)->result_type;
		}
		else if (cur->ptr_one_type == PTR_TYPE_TABLE_COLS_INFO)
		{
			ptr_one_type = ((struct table_cols_info*) cur->ptr_one)->data_type;

			struct ListNodePtr* cur_col = head;
			while (cur_col != NULL)
			{
				if (((struct table_cols_info*) cur->ptr_one)->col_number == cur_col->ptr_type)
				{
					//printf("*row_id = %lu\n", row_id);
					ptr_one = ((struct colDataNode**) cur_col->ptr_value)[row_id]->row_data;
					break;
				}

				cur_col = cur_col->next;
			}

			if (cur_col == NULL)
			{
				if (the_debug == YES_DEBUG)
					printf("	ERROR in doWhere() at line %d in %s\n", __LINE__, __FILE__);
				return RETURN_ERROR;
			}
		}
		else
		{
			ptr_one = cur->ptr_one;
			ptr_one_type = cur->ptr_one_type;
		}



		if (cur->ptr_two_type == PTR_TYPE_MATH_NODE)
		{
			if ((result = evaluateMathTree(head, tail, (struct math_node*) cur->ptr_two, row_id, second_row_id, malloced_head, the_debug)) < 0)
			{
				if (the_debug == YES_DEBUG)
					printf("	ERROR in evaluateMathTree() at line %d in %s\n", __LINE__, __FILE__);
				return RETURN_ERROR;
			}

			ptr_two = ((struct math_node*) cur->ptr_two)->result;
			ptr_two_type = ((struct math_node*) cur->ptr_two)->result_type;
		}
		else if (cur->ptr_two_type == PTR_TYPE_COL_IN_SELECT_NODE)
		{
			if (((int) second_row_id) > -1)
			{
				struct ListNodePtr* cur_join_col = head;
				while (cur_join_col != NULL)
				{
					if (cur_join_col->ptr_value == cur->ptr_two)
					{
						int_8 index = row_id;
						if (cur_join_col->ptr_type == COL_IN_JOIN_IS_RIGHT)
							index = second_row_id;

						ptr_two = ((struct col_in_select_node*) cur->ptr_two)->col_data_arr[index]->row_data;
						ptr_two_type = ((struct col_in_select_node*) cur->ptr_two)->rows_data_type;

						break;
					}

					cur_join_col = cur_join_col->next;
				}
			}
			else
			{
				ptr_two = ((struct col_in_select_node*) cur->ptr_two)->col_data_arr[row_id]->row_data;
				ptr_two_type = ((struct col_in_select_node*) cur->ptr_two)->rows_data_type;
			}

			result = 1;
		}
		else if (cur->ptr_two_type == PTR_TYPE_FUNC_NODE)
		{
			struct ListNodePtr* cur_row = head;
			while (cur_row != NULL)
			{
				struct ListNodePtr* cur_group_row_id = ((struct group_data_node*) cur_row->ptr_value)->row_ids_head;
				while (cur_group_row_id != NULL)
				{
					if (*((int*) cur_group_row_id->ptr_value) == row_id)
						break;
					cur_group_row_id = cur_group_row_id->next;
				}

				if (cur_group_row_id != NULL)
					break;

				cur_row = cur_row->next;
			}

			//printf("first cur_row row_id = %d\n", *((int*) ((struct group_data_node*) cur_row->ptr_value)->row_ids_head->ptr_value));
			//printf("*((int*) cur_row_id->ptr_value) = %lu\n", *((int*) cur_row_id->ptr_value));

			if (calcResultOfFuncForOneRow(cur_row, cur->ptr_two, NULL, -1, malloced_head, the_debug) != RETURN_GOOD)
			{
				if (the_debug == YES_DEBUG)
					printf("	ERROR in execCaseNode() at line %d in %s\n", __LINE__, __FILE__);
				return RETURN_ERROR;
			}

			ptr_two = ((struct func_node*) cur->ptr_two)->result;
			ptr_two_type = ((struct func_node*) cur->ptr_two)->result_type;

			result = 1;
		}
		else if (cur->ptr_two_type == PTR_TYPE_CASE_NODE)
		{
			if (calcResultOfCaseForOneRow(cur->ptr_two, row_id, -1, NULL, NULL, ((int) second_row_id) > -1, -1, -1, head, tail, malloced_head, the_debug) != RETURN_GOOD)
			{
				if (the_debug == YES_DEBUG)
					printf("	ERROR in evaluateMathTree() at line %d in %s\n", __LINE__, __FILE__);
				return RETURN_ERROR;
			}

			ptr_two = ((struct case_node*) cur->ptr_two)->result;
			ptr_two_type = ((struct case_node*) cur->ptr_two)->result_type;
		}
		else if (cur->ptr_two_type == PTR_TYPE_TABLE_COLS_INFO)
		{
			ptr_two_type = ((struct table_cols_info*) cur->ptr_two)->data_type;

			struct ListNodePtr* cur_col = head;
			while (cur_col != NULL)
			{
				if (((struct table_cols_info*) cur->ptr_two)->col_number == cur_col->ptr_type)
				{
					//printf("*row_id = %lu\n", row_id);
					ptr_two = ((struct colDataNode**) cur_col->ptr_value)[row_id]->row_data;
					break;
				}

				cur_col = cur_col->next;
			}

			if (cur_col == NULL)
			{
				if (the_debug == YES_DEBUG)
					printf("	ERROR in doWhere() at line %d in %s\n", __LINE__, __FILE__);
				return RETURN_ERROR;
			}
		}
		else
		{
			ptr_two = cur->ptr_two;
			ptr_two_type = cur->ptr_two_type;
		}
	// END Go deeper if more math_nodes


	//printf("Calling doMath(), ptr_one = %d, ptr_two = %d\n", *((int*) ptr_one), *((int*) ptr_two));
	if (ptr_one == NULL || ptr_two == NULL)
	{
		if (the_debug == YES_DEBUG)
			printf("Found a null value\n");
		cur->result = NULL;
	}
	else
	{
		if (doMath(cur, ptr_one, ptr_one_type, ptr_two, ptr_two_type, malloced_head, the_debug) != RETURN_GOOD)
		{
			if (the_debug == YES_DEBUG)
				printf("	ERROR in evaluateMathTree() at line %d in %s\n", __LINE__, __FILE__);
			return RETURN_ERROR;
		}
	}


	if (cur->ptr_one_type == PTR_TYPE_MATH_NODE)
		myFree((void**) &((struct math_node*) cur->ptr_one)->result, NULL, malloced_head, the_debug);

	if (cur->ptr_two_type == PTR_TYPE_MATH_NODE)
		myFree((void**) &((struct math_node*) cur->ptr_two)->result, NULL, malloced_head, the_debug);

	if (cur->ptr_one_type == PTR_TYPE_FUNC_NODE)
		myFree((void**) &((struct func_node*) cur->ptr_one)->result, NULL, malloced_head, the_debug);

	if (cur->ptr_two_type == PTR_TYPE_FUNC_NODE)
		myFree((void**) &((struct func_node*) cur->ptr_two)->result, NULL, malloced_head, the_debug);

	return result;
}


struct result_node* evaluateMathTreeV2(struct ListNodePtr* head, struct ListNodePtr* tail, struct math_node* cur, int_8 row_id, int_8 second_row_id
											,struct malloced_node** malloced_head, int the_debug)
{
	void* ptr_one = NULL;
	int ptr_one_type = -1;

	void* ptr_two = NULL;
	int ptr_two_type = -1;


	struct result_node* temp_math_1 = NULL;
	struct result_node* temp_math_2 = NULL;

	struct result_node* result_node_1 = NULL;
	struct result_node* result_node_2 = NULL;


	// START Go deeper if more math_nodes
		if (cur->ptr_one_type == PTR_TYPE_MATH_NODE)
		{
			temp_math_1 = evaluateMathTreeV2(head, tail, (struct math_node*) cur->ptr_one, row_id, second_row_id, malloced_head, the_debug);

			if (temp_math_1 == NULL)
			{
				if (the_debug == YES_DEBUG)
					printf("	ERROR in evaluateMathTreeV2() at line %d in %s\n", __LINE__, __FILE__);
				return RETURN_ERROR;
			}

			ptr_one = temp_math_1->result;
			ptr_one_type = temp_math_1->result_type;
		}
		else if (cur->ptr_one_type == PTR_TYPE_COL_IN_SELECT_NODE)
		{
			if (((int) second_row_id) > -1)
			{
				struct ListNodePtr* cur_join_col = head;
				while (cur_join_col != NULL)
				{
					if (cur_join_col->ptr_value == cur->ptr_one)
					{
						int_8 index = row_id;
						if (cur_join_col->ptr_type == COL_IN_JOIN_IS_RIGHT)
							index = second_row_id;

						ptr_one = ((struct col_in_select_node*) cur->ptr_one)->col_data_arr[index]->row_data;
						ptr_one_type = ((struct col_in_select_node*) cur->ptr_one)->rows_data_type;

						break;
					}

					cur_join_col = cur_join_col->next;
				}
			}
			else
			{
				ptr_one = ((struct col_in_select_node*) cur->ptr_one)->col_data_arr[row_id]->row_data;
				ptr_one_type = ((struct col_in_select_node*) cur->ptr_one)->rows_data_type;
			}
		}
		else if (cur->ptr_one_type == PTR_TYPE_FUNC_NODE)
		{
			//calcResultOfFuncForOneRowV2

			/*struct ListNodePtr* cur_row = head;
			while (cur_row != NULL)
			{
				struct ListNodePtr* cur_group_row_id = ((struct group_data_node*) cur_row->ptr_value)->row_ids_head;
				while (cur_group_row_id != NULL)
				{
					if (*((int*) cur_group_row_id->ptr_value) == row_id)
						break;
					cur_group_row_id = cur_group_row_id->next;
				}

				if (cur_group_row_id != NULL)
					break;

				cur_row = cur_row->next;
			}

			//printf("first cur_row row_id = %d\n", *((int*) ((struct group_data_node*) cur_row->ptr_value)->row_ids_head->ptr_value));
			//printf("*((int*) cur_row_id->ptr_value) = %lu\n", *((int*) cur_row_id->ptr_value));

			if (calcResultOfFuncForOneRow(cur_row, cur->ptr_one, NULL, -1, malloced_head, the_debug) != RETURN_GOOD)
			{
				if (the_debug == YES_DEBUG)
					printf("	ERROR in evaluateMathTreeV2() at line %d in %s\n", __LINE__, __FILE__);
				return RETURN_ERROR;
			}

			ptr_one = ((struct func_node*) cur->ptr_one)->result;
			ptr_one_type = ((struct func_node*) cur->ptr_one)->result_type;

			//printf("value at func result = %d\n", *((int*) ((struct func_node*) cur->ptr_one)->result));
			//printf("value at func result_type = %d\n", ((struct func_node*) cur->ptr_one)->result_type);

			result = 1;*/
		}
		else if (cur->ptr_one_type == PTR_TYPE_CASE_NODE)
		{
			result_node_1 = calcResultOfCaseForOneRowV2(cur->ptr_one, row_id, second_row_id, NULL, NULL, ((int) second_row_id) > -1, -1, -1, head, tail, malloced_head, the_debug);
			if (result_node_1 == NULL)
			{
				if (the_debug == YES_DEBUG)
					printf("	ERROR in evaluateMathTreeV2() at line %d in %s\n", __LINE__, __FILE__);
				return RETURN_ERROR;
			}

			ptr_one = result_node_1->result;
			ptr_one_type = result_node_1->result_type;
		}
		else if (cur->ptr_one_type == PTR_TYPE_TABLE_COLS_INFO)
		{
			ptr_one_type = ((struct table_cols_info*) cur->ptr_one)->data_type;

			struct ListNodePtr* cur_col = head;
			while (cur_col != NULL)
			{
				if (((struct table_cols_info*) cur->ptr_one)->col_number == cur_col->ptr_type)
				{
					//printf("*row_id = %lu\n", row_id);
					ptr_one = ((struct colDataNode**) cur_col->ptr_value)[row_id]->row_data;
					break;
				}

				cur_col = cur_col->next;
			}

			if (cur_col == NULL)
			{
				if (the_debug == YES_DEBUG)
					printf("	ERROR in doWhere() at line %d in %s\n", __LINE__, __FILE__);
				return RETURN_ERROR;
			}
		}
		else
		{
			ptr_one = cur->ptr_one;
			ptr_one_type = cur->ptr_one_type;
		}



		if (cur->ptr_two_type == PTR_TYPE_MATH_NODE)
		{
			temp_math_2 = evaluateMathTreeV2(head, tail, (struct math_node*) cur->ptr_two, row_id, second_row_id, malloced_head, the_debug);

			if (temp_math_2 == NULL)
			{
				if (the_debug == YES_DEBUG)
					printf("	ERROR in evaluateMathTreeV2() at line %d in %s\n", __LINE__, __FILE__);
				return RETURN_ERROR;
			}

			ptr_two = temp_math_2->result;
			ptr_two_type = temp_math_2->result_type;
		}
		else if (cur->ptr_two_type == PTR_TYPE_COL_IN_SELECT_NODE)
		{
			if (((int) second_row_id) > -1)
			{
				struct ListNodePtr* cur_join_col = head;
				while (cur_join_col != NULL)
				{
					if (cur_join_col->ptr_value == cur->ptr_two)
					{
						int_8 index = row_id;
						if (cur_join_col->ptr_type == COL_IN_JOIN_IS_RIGHT)
							index = second_row_id;

						ptr_two = ((struct col_in_select_node*) cur->ptr_two)->col_data_arr[index]->row_data;
						ptr_two_type = ((struct col_in_select_node*) cur->ptr_two)->rows_data_type;

						break;
					}

					cur_join_col = cur_join_col->next;
				}
			}
			else
			{
				ptr_two = ((struct col_in_select_node*) cur->ptr_two)->col_data_arr[row_id]->row_data;
				ptr_two_type = ((struct col_in_select_node*) cur->ptr_two)->rows_data_type;
			}
		}
		else if (cur->ptr_two_type == PTR_TYPE_FUNC_NODE)
		{
			//calcResultOfFuncForOneRowV2

			/*struct ListNodePtr* cur_row = head;
			while (cur_row != NULL)
			{
				struct ListNodePtr* cur_group_row_id = ((struct group_data_node*) cur_row->ptr_value)->row_ids_head;
				while (cur_group_row_id != NULL)
				{
					if (*((int*) cur_group_row_id->ptr_value) == row_id)
						break;
					cur_group_row_id = cur_group_row_id->next;
				}

				if (cur_group_row_id != NULL)
					break;

				cur_row = cur_row->next;
			}

			//printf("first cur_row row_id = %d\n", *((int*) ((struct group_data_node*) cur_row->ptr_value)->row_ids_head->ptr_value));
			//printf("*((int*) cur_row_id->ptr_value) = %lu\n", *((int*) cur_row_id->ptr_value));

			if (calcResultOfFuncForOneRow(cur_row, cur->ptr_two, NULL, -1, malloced_head, the_debug) != RETURN_GOOD)
			{
				if (the_debug == YES_DEBUG)
					printf("	ERROR in evaluateMathTreeV2() at line %d in %s\n", __LINE__, __FILE__);
				return RETURN_ERROR;
			}

			ptr_two = ((struct func_node*) cur->ptr_two)->result;
			ptr_two_type = ((struct func_node*) cur->ptr_two)->result_type;

			result = 1;*/
		}
		else if (cur->ptr_two_type == PTR_TYPE_CASE_NODE)
		{
			result_node_2 = calcResultOfCaseForOneRowV2(cur->ptr_two, row_id, -1, NULL, NULL, ((int) second_row_id) > -1, -1, -1, head, tail, malloced_head, the_debug);
			if (result_node_2 == NULL)
			{
				if (the_debug == YES_DEBUG)
					printf("	ERROR in evaluateMathTreeV2() at line %d in %s\n", __LINE__, __FILE__);
				return RETURN_ERROR;
			}

			ptr_two = result_node_2->result;
			ptr_two_type = result_node_2->result_type;
		}
		else if (cur->ptr_two_type == PTR_TYPE_TABLE_COLS_INFO)
		{
			ptr_two_type = ((struct table_cols_info*) cur->ptr_two)->data_type;

			struct ListNodePtr* cur_col = head;
			while (cur_col != NULL)
			{
				if (((struct table_cols_info*) cur->ptr_two)->col_number == cur_col->ptr_type)
				{
					//printf("*row_id = %lu\n", row_id);
					ptr_two = ((struct colDataNode**) cur_col->ptr_value)[row_id]->row_data;
					break;
				}

				cur_col = cur_col->next;
			}

			if (cur_col == NULL)
			{
				if (the_debug == YES_DEBUG)
					printf("	ERROR in evaluateMathTreeV2() at line %d in %s\n", __LINE__, __FILE__);
				return RETURN_ERROR;
			}
		}
		else
		{
			ptr_two = cur->ptr_two;
			ptr_two_type = cur->ptr_two_type;
		}
	// END Go deeper if more math_nodes


	//printf("Calling doMath(), ptr_one = %d, ptr_two = %d\n", *((int*) ptr_one), *((int*) ptr_two));
	/*if (ptr_one == NULL || ptr_two == NULL)
	{
		if (the_debug == YES_DEBUG)
			printf("	ERROR in evaluateMathTreeV2() at line %d in %s\n", __LINE__, __FILE__);
		return RETURN_ERROR;
	}*/
	

	struct math_node_result* temp_result = doMathV2(cur, ptr_one, ptr_one_type, ptr_two, ptr_two_type, malloced_head, the_debug);

	if (temp_result == NULL)
	{
		if (the_debug == YES_DEBUG)
			printf("	ERROR in evaluateMathTreeV2() at line %d in %s\n", __LINE__, __FILE__);
		return RETURN_ERROR;
	}


	if (cur->ptr_one_type == PTR_TYPE_MATH_NODE)
	{
		if (temp_math_1->result != NULL)
			myFree((void**) &temp_math_1->result, NULL, malloced_head, the_debug);
		myFree((void**) &temp_math_1, NULL, malloced_head, the_debug);
	}

	if (cur->ptr_two_type == PTR_TYPE_MATH_NODE)
	{
		if (temp_math_2->result != NULL)
			myFree((void**) &temp_math_2->result, NULL, malloced_head, the_debug);
		myFree((void**) &temp_math_2, NULL, malloced_head, the_debug);
	}

	if (cur->ptr_one_type == PTR_TYPE_CASE_NODE)
		myFree((void**) &result_node_1, NULL, malloced_head, the_debug);

	if (cur->ptr_two_type == PTR_TYPE_CASE_NODE)
		myFree((void**) &result_node_2, NULL, malloced_head, the_debug);

	//if (cur->ptr_one_type == PTR_TYPE_FUNC_NODE)
	//	myFree((void**) &((struct func_node*) cur->ptr_one)->result, NULL, malloced_head, the_debug);

	//if (cur->ptr_two_type == PTR_TYPE_FUNC_NODE)
	//	myFree((void**) &((struct func_node*) cur->ptr_two)->result, NULL, malloced_head, the_debug);

	return temp_result;
}

/*	RETURNS:
 *  RETURN_ERROR if error
 *	RETURN_GOOD if good
 *
 *	WRITES TO:
 *	cols_in_on_head
 *	cols_in_on_tail
 */
int checkIfColLeftOrRight(struct ListNodePtr** cols_in_on_head, struct ListNodePtr** cols_in_on_tail, struct select_node* left, struct select_node* right
						 ,void* the_col, struct malloced_node** malloced_head, int the_debug)
{
	if (the_debug == YES_DEBUG)
		printf("Adding col to cols_in_on_head: %p\n", the_col);

	for (int j=0; j<left->prev->columns_arr_size; j++)
	{
		if (the_col == left->prev->columns_arr[j])
		{
			if (the_debug == YES_DEBUG) printf("	A: left\n");
			if (addListNodePtr(cols_in_on_head, cols_in_on_tail, the_col, COL_IN_JOIN_IS_LEFT, ADDLISTNODE_TAIL, NULL, malloced_head, the_debug) != 0)
			{
				if (the_debug == YES_DEBUG)
					printf("	ERROR in checkIfColLeftOrRight() at line %d in %s\n", __LINE__, __FILE__);
				return RETURN_ERROR;
			}
			return RETURN_GOOD;
		}
	}

	struct join_node* cur_join = left->join_head;
	while (cur_join != NULL && cur_join->select_joined != right)
	{
		for (int jj=0; jj<cur_join->select_joined->columns_arr_size; jj++)
		{
			if (the_col == cur_join->select_joined->columns_arr[jj])
			{
				if (the_debug == YES_DEBUG) printf("	B: left\n");
				if (addListNodePtr(cols_in_on_head, cols_in_on_tail, the_col, COL_IN_JOIN_IS_LEFT, ADDLISTNODE_TAIL, NULL, malloced_head, the_debug) != 0)
				{
					if (the_debug == YES_DEBUG)
						printf("	ERROR in checkIfColLeftOrRight() at line %d in %s\n", __LINE__, __FILE__);
					return RETURN_ERROR;
				}
				return RETURN_GOOD;
			}
		}

		cur_join = cur_join->next;
	}

	for (int jj=0; jj<right->columns_arr_size; jj++)
	{
		if (the_col == right->columns_arr[jj])
		{
			if (the_debug == YES_DEBUG) printf("	C: right\n");
			if (addListNodePtr(cols_in_on_head, cols_in_on_tail, the_col, COL_IN_JOIN_IS_RIGHT, ADDLISTNODE_TAIL, NULL, malloced_head, the_debug) != 0)
			{
				if (the_debug == YES_DEBUG)
					printf("	ERROR in checkIfColLeftOrRight() at line %d in %s\n", __LINE__, __FILE__);
				return RETURN_ERROR;
			}
			return RETURN_GOOD;
		}
	}

	if (the_debug == YES_DEBUG)
		printf("	ERROR in checkIfColLeftOrRight() at line %d in %s\n", __LINE__, __FILE__);
	errorTeardown(NULL, malloced_head, the_debug);
	return RETURN_ERROR;
}

int getAllNodesFromOnClause(struct where_clause_node* cur, struct ListNodePtr** node_head, struct ListNodePtr** node_tail
						   ,struct malloced_node** malloced_head, int the_debug)
{
	if (cur != NULL && cur->ptr_one_type == PTR_TYPE_WHERE_CLAUSE_NODE && cur->ptr_two_type == PTR_TYPE_WHERE_CLAUSE_NODE)
	{
		if (getAllNodesFromOnClause(cur->ptr_one, node_head, node_tail, malloced_head, the_debug) != RETURN_GOOD)
		{
			if (the_debug == YES_DEBUG)
				printf("	ERROR in getAllNodesFromOnClause() at line %d in %s\n", __LINE__, __FILE__);
			return RETURN_ERROR;
		}

		if (getAllNodesFromOnClause(cur->ptr_two, node_head, node_tail, malloced_head, the_debug) != RETURN_GOOD)
		{
			if (the_debug == YES_DEBUG)
				printf("	ERROR in getAllNodesFromOnClause() at line %d in %s\n", __LINE__, __FILE__);
			return RETURN_ERROR;
		}
	}
	else if (cur != NULL)
	{
		if (addListNodePtr(node_head, node_tail, cur->ptr_one, cur->ptr_one_type * -1, ADDLISTNODE_TAIL, NULL, malloced_head, the_debug) != 0)
		{
			if (the_debug == YES_DEBUG)
				printf("	ERROR in getAllNodesFromOnClause() at line %d in %s\n", __LINE__, __FILE__);
			return RETURN_ERROR;
		}

		if (addListNodePtr(node_head, node_tail, cur->ptr_two, cur->ptr_two_type * -1, ADDLISTNODE_TAIL, NULL, malloced_head, the_debug) != 0)
		{
			if (the_debug == YES_DEBUG)
				printf("	ERROR in getAllNodesFromOnClause() at line %d in %s\n", __LINE__, __FILE__);
			return RETURN_ERROR;
		}
	}

	return RETURN_GOOD;
}

int evaluateWhereTreeForJoin(struct where_clause_node* cur, struct ListNodePtr* cur_node, struct ListNodePtr* cur_col_data, int i, int j)
{
	if (cur != NULL && cur->ptr_one_type == PTR_TYPE_WHERE_CLAUSE_NODE && cur->ptr_two_type == PTR_TYPE_WHERE_CLAUSE_NODE)
	{
		if (cur->where_type == WHERE_OR)
		{
			if (evaluateWhereTreeForJoin(cur->ptr_one, cur_node, cur_col_data, i, j) == 1 
				|| evaluateWhereTreeForJoin(cur->ptr_two, cur_node->next->next, cur_col_data->next->next, i, j) == 1)
				return 1;
			else
				return 0;
		}
		else
		{
			if (evaluateWhereTreeForJoin(cur->ptr_one, cur_node, cur_col_data, i, j) == 1 
				&& evaluateWhereTreeForJoin(cur->ptr_two, cur_node->next->next, cur_col_data->next->next, i, j) == 1)
				return 1;
			else
				return 0;
		}
	}
	else if (cur != NULL)
	{
		int col_data_type = -1;
		void* temp_ptr_one = NULL;
		void* temp_ptr_two = NULL;

		if (cur->ptr_one_type == PTR_TYPE_COL_IN_SELECT_NODE || cur->ptr_one_type == PTR_TYPE_FUNC_NODE 
			|| cur->ptr_one_type == PTR_TYPE_MATH_NODE || cur->ptr_one_type == PTR_TYPE_CASE_NODE)
		{
			temp_ptr_one = ((struct colDataNode**) cur_col_data->ptr_value)[i]->row_data;
			
			if (cur->ptr_one_type == PTR_TYPE_COL_IN_SELECT_NODE)
				col_data_type = ((struct col_in_select_node*) cur->ptr_one)->rows_data_type;
			else if (cur->ptr_one_type == PTR_TYPE_FUNC_NODE)
				col_data_type = ((struct func_node*) cur->ptr_one)->result_type;
			else if (cur->ptr_one_type == PTR_TYPE_MATH_NODE)
				col_data_type = ((struct math_node*) cur->ptr_one)->result_type;
			else if (cur->ptr_one_type == PTR_TYPE_CASE_NODE)
				col_data_type = ((struct case_node*) cur->ptr_one)->result_type;
		}
		else
		{
			temp_ptr_one = cur->ptr_one;
			col_data_type = cur->ptr_one_type;
		}

		if (cur->ptr_two_type == PTR_TYPE_COL_IN_SELECT_NODE || cur->ptr_two_type == PTR_TYPE_FUNC_NODE 
			|| cur->ptr_two_type == PTR_TYPE_MATH_NODE || cur->ptr_two_type == PTR_TYPE_CASE_NODE)
			temp_ptr_two = ((struct colDataNode**) cur_col_data->next->ptr_value)[j]->row_data;
		else
			temp_ptr_two = cur->ptr_two;


		if (false)
		{
			printf("i = %d, j = %d\n", i, j);
			printf("col_data_type = %d\n", col_data_type);
			if (col_data_type == DATA_INT)
				printf("Comparing %d and %d\n", temp_ptr_one == NULL ? -1 : *((int*) temp_ptr_one), temp_ptr_two == NULL ? -1 : *((int*) temp_ptr_two));
			else if (col_data_type == DATA_REAL)
				printf("Comparing %lf and %lf\n", temp_ptr_one == NULL ? -1.0 : *((double*) temp_ptr_one), temp_ptr_two == NULL ? -1 : *((double*) temp_ptr_two));
			else if (col_data_type == DATA_STRING || col_data_type == DATA_DATE)
				printf("Comparing _%s_ and _%s_\n", temp_ptr_one, temp_ptr_two);
		}


		// START Evaluate inequality
			int result;

			if (cur->where_type == WHERE_IS_EQUALS)
			{
				if (equals(temp_ptr_one, col_data_type, temp_ptr_two, VALUE_EQUALS))
					result = 1;
				else
					result = 0;
			}
			else if (cur->where_type == WHERE_NOT_EQUALS)
			{
				if (!equals(temp_ptr_one, col_data_type, temp_ptr_two, VALUE_EQUALS))
					result = 1;
				else
					result = 0;
			}
			else if (cur->where_type == WHERE_GREATER_THAN || cur->where_type == WHERE_GREATER_THAN_OR_EQUAL ||
					 cur->where_type == WHERE_LESS_THAN || cur->where_type == WHERE_LESS_THAN_OR_EQUAL)
			{
				bool is_true = greatLess(temp_ptr_one, col_data_type, temp_ptr_two, cur->where_type);

				if ((is_true && cur->where_type == WHERE_GREATER_THAN)
					|| (is_true && cur->where_type == WHERE_GREATER_THAN_OR_EQUAL)
					|| (is_true && cur->where_type == WHERE_LESS_THAN)
					|| (is_true && cur->where_type == WHERE_LESS_THAN_OR_EQUAL))
					result = 1;
				else
					result = 0;
			}
			else if (cur->where_type == WHERE_IS_NULL)
			{
				if (temp_ptr_one == NULL)
					result = 1;
				else
					result = 0;
			}
			else if (cur->where_type == WHERE_IS_NOT_NULL)
			{
				if (temp_ptr_one != NULL)
					result = 1;
				else
					result = 0;
			}
		// END Evaluate inequality

		return result;
	}
	else
		return 1;
}

bool nextRowIsEqual(bool left, struct ListNodePtr* node_head, struct ListNodePtr* col_data_head, int row_id)
{
	struct ListNodePtr* cur_node = node_head;
	struct ListNodePtr* cur_col_data = col_data_head;

	if (!left)
	{
		cur_node = cur_node->next;
		cur_col_data = cur_col_data->next;
	}

	while (cur_node != NULL)
	{
		int the_data_type;
		if (cur_node->ptr_type * -1 == PTR_TYPE_COL_IN_SELECT_NODE)
			the_data_type = ((struct col_in_select_node*) cur_node->ptr_value)->rows_data_type;
		else if (cur_node->ptr_type * -1 == PTR_TYPE_MATH_NODE)
			the_data_type = ((struct math_node*) cur_node->ptr_value)->result_type;
		else if (cur_node->ptr_type * -1 == PTR_TYPE_CASE_NODE)
			the_data_type = ((struct case_node*) cur_node->ptr_value)->result_type;

		if (cur_col_data->ptr_value != NULL 
			&& !equals(((struct colDataNode**) cur_col_data->ptr_value)[row_id]->row_data, the_data_type, ((struct colDataNode**) cur_col_data->ptr_value)[row_id+1]->row_data, VALUE_EQUALS))
			return false;

		cur_node = cur_node->next;
		if (cur_node != NULL)
			cur_node = cur_node->next;

		cur_col_data = cur_col_data->next;
		if (cur_col_data != NULL)
			cur_col_data = cur_col_data->next;
	}

	return true;
}

bool leftOrRightIsLess(bool left, struct ListNodePtr* node_head, struct ListNodePtr* col_data_head, int i, int j)
{
	//printf("Starting leftOrRightIsLess()\n");
	struct ListNodePtr* cur_node = node_head;
	struct ListNodePtr* cur_col_data = col_data_head;

	while (cur_node != NULL)
	{
		int the_data_type;
		if (cur_node->ptr_type * -1 == PTR_TYPE_COL_IN_SELECT_NODE)
			the_data_type = ((struct col_in_select_node*) cur_node->ptr_value)->rows_data_type;
		else if (cur_node->ptr_type * -1 == PTR_TYPE_MATH_NODE)
			the_data_type = ((struct math_node*) cur_node->ptr_value)->result_type;
		else if (cur_node->ptr_type * -1 == PTR_TYPE_CASE_NODE)
			the_data_type = ((struct case_node*) cur_node->ptr_value)->result_type;

		if (cur_col_data->ptr_value != NULL && cur_col_data->next->ptr_value != NULL)
		{
			if (the_data_type == PTR_TYPE_INT)
			{
				//printf("   leftOrRightIsLess %d vs %d\n",((struct colDataNode**) cur_col_data->ptr_value)[i]->row_data == NULL ? -1 : *((int*) ((struct colDataNode**) cur_col_data->ptr_value)[i]->row_data)
				//							  ,((struct colDataNode**) cur_col_data->next->ptr_value)[j]->row_data == NULL ? -1 : *((int*) ((struct colDataNode**) cur_col_data->next->ptr_value)[j]->row_data));
				
				//if (left && !(*((int*) ((struct colDataNode**) cur_col_data->ptr_value)[i]->row_data) < *((int*) ((struct colDataNode**) cur_col_data->next->ptr_value)[j]->row_data)))
				//	return false;
				//else if (!left && !(*((int*) ((struct colDataNode**) cur_col_data->ptr_value)[i]->row_data) > *((int*) ((struct colDataNode**) cur_col_data->next->ptr_value)[j]->row_data)))
				//	return false;

				if (*((int*) ((struct colDataNode**) cur_col_data->ptr_value)[i]->row_data) < *((int*) ((struct colDataNode**) cur_col_data->next->ptr_value)[j]->row_data))
				{
					if (left)
						return true;
					else
						return false;
				}
				else if (*((int*) ((struct colDataNode**) cur_col_data->ptr_value)[i]->row_data) > *((int*) ((struct colDataNode**) cur_col_data->next->ptr_value)[j]->row_data))
				{
					if (!left)
						return true;
					else
						return false;
				}
			}
			else if (the_data_type == PTR_TYPE_REAL)
			{
				//printf("   leftOrRightIsLess %lf vs %lf\n",((struct colDataNode**) cur_col_data->ptr_value)[i]->row_data == NULL ? -1.0 : *((double*) ((struct colDataNode**) cur_col_data->ptr_value)[i]->row_data)
				//							    ,((struct colDataNode**) cur_col_data->next->ptr_value)[j]->row_data == NULL ? -1.0 : *((double*) ((struct colDataNode**) cur_col_data->next->ptr_value)[j]->row_data));
				
				//if (left && !(*((double*) ((struct colDataNode**) cur_col_data->ptr_value)[i]->row_data) < *((double*) ((struct colDataNode**) cur_col_data->next->ptr_value)[j]->row_data)))
				//	return false;
				//else if (!left && !(*((double*) ((struct colDataNode**) cur_col_data->ptr_value)[i]->row_data) > *((double*) ((struct colDataNode**) cur_col_data->next->ptr_value)[j]->row_data)))
				//	return false;

				if (*((double*) ((struct colDataNode**) cur_col_data->ptr_value)[i]->row_data) < *((double*) ((struct colDataNode**) cur_col_data->next->ptr_value)[j]->row_data))
				{
					if (left)
						return true;
					else
						return false;
				}
				else if (*((double*) ((struct colDataNode**) cur_col_data->ptr_value)[i]->row_data) > *((double*) ((struct colDataNode**) cur_col_data->next->ptr_value)[j]->row_data))
				{
					if (!left)
						return true;
					else
						return false;
				}
			}
			else if (the_data_type == PTR_TYPE_DATE)
			{
				//printf("   leftOrRightIsLess _%s_ vs _%s_\n", ((struct colDataNode**) cur_col_data->ptr_value)[i]->row_data, ((struct colDataNode**) cur_col_data->next->ptr_value)[j]->row_data);
				
				//if (left && !(dateToInt(((struct colDataNode**) cur_col_data->ptr_value)[i]->row_data) < dateToInt(((struct colDataNode**) cur_col_data->next->ptr_value)[j]->row_data)))
				//	return false;
				//else if (!left && !(dateToInt(((struct colDataNode**) cur_col_data->ptr_value)[i]->row_data) > dateToInt(((struct colDataNode**) cur_col_data->next->ptr_value)[j]->row_data)))
				//	return false;

				if (dateToInt(((struct colDataNode**) cur_col_data->ptr_value)[i]->row_data) < dateToInt(((struct colDataNode**) cur_col_data->next->ptr_value)[j]->row_data))
				{
					if (left)
						return true;
					else
						return false;
				}
				else if (dateToInt(((struct colDataNode**) cur_col_data->ptr_value)[i]->row_data) > dateToInt(((struct colDataNode**) cur_col_data->next->ptr_value)[j]->row_data))
				{
					if (!left)
						return true;
					else
						return false;
				}
			}
			else if (the_data_type == PTR_TYPE_CHAR)
			{
				//printf("   leftOrRightIsLess _%s_ vs _%s_\n", ((struct colDataNode**) cur_col_data->ptr_value)[i]->row_data, ((struct colDataNode**) cur_col_data->next->ptr_value)[j]->row_data);

				//if (left && strcmp(((struct colDataNode**) cur_col_data->ptr_value)[i]->row_data, ((struct colDataNode**) cur_col_data->next->ptr_value)[j]->row_data) >= 0)
				//	return false;
				//else if (!left && strcmp(((struct colDataNode**) cur_col_data->ptr_value)[i]->row_data, ((struct colDataNode**) cur_col_data->next->ptr_value)[j]->row_data) <= 0)
				//	return false;

				if (strcmp(((struct colDataNode**) cur_col_data->ptr_value)[i]->row_data, ((struct colDataNode**) cur_col_data->next->ptr_value)[j]->row_data) < 0)
				{
					if (left)
						return true;
					else
						return false;
				}
				else if (strcmp(((struct colDataNode**) cur_col_data->ptr_value)[i]->row_data, ((struct colDataNode**) cur_col_data->next->ptr_value)[j]->row_data) > 0)
				{
					if (!left)
						return true;
					else
						return false;
				}
			}
		}
		else
			return true;

		cur_node = cur_node->next;
		if (cur_node != NULL)
			cur_node = cur_node->next;

		cur_col_data = cur_col_data->next;
		if (cur_col_data != NULL)
			cur_col_data = cur_col_data->next;
	}

	return false;
}

int reorderColsJustArr(struct colDataNode** cur_arr, struct colDataNode** most_recently_sorted_arr, int left, int right)
{
	struct colDataNode* temp[(right - left) + 1];

	for (int i=0; i<(right - left) + 1; i++)
	{
		temp[i] = cur_arr[most_recently_sorted_arr[i + left]->row_id];
	}

	for (int i=left; i<right+1; i++)
	{
		cur_arr[i] = temp[i - left];
	}

	return RETURN_GOOD;
}

int findMatchingRowsOfJoin(struct where_clause_node* cur, struct table_info* the_table, struct select_node* the_select_node
						  ,struct ListNodePtr** left_head, struct ListNodePtr** left_tail, struct ListNodePtr** right_head, struct ListNodePtr** right_tail
						  ,struct ListNodePtr* groups_head, struct ListNodePtr* groups_tail, int_8 left_num_rows, int_8 right_num_rows, int_8* num_rows_in_result
						  ,struct malloced_node** malloced_head, int the_debug)
{
	if (cur == NULL)
	{
		// START Is cross join, just n^2 row indexes
			if (the_debug == YES_DEBUG) printf("Cross join\n");

			for (int i=0; i<left_num_rows; i++)
			{
				for (int j=0; j<right_num_rows; j++)
				{
					*num_rows_in_result = (*num_rows_in_result)+1;

					if (addListNodePtr_Int(left_head, left_tail, i, ADDLISTNODE_TAIL, NULL, malloced_head, the_debug) != 0)
					{
						if (the_debug == YES_DEBUG)
							printf("	ERROR in evaluateWhereTree() at line %d in %s\n", __LINE__, __FILE__);
						return RETURN_ERROR;
					}
					if (addListNodePtr_Int(right_head, right_tail, j, ADDLISTNODE_TAIL, NULL, malloced_head, the_debug) != 0)
					{
						if (the_debug == YES_DEBUG)
							printf("	ERROR in evaluateWhereTree() at line %d in %s\n", __LINE__, __FILE__);
						return RETURN_ERROR;
					}
				}
			}

			return RETURN_GOOD;
		// END Is cross join, just n^2 row indexes
	}

	struct ListNodePtr* node_head = NULL;
	struct ListNodePtr* node_tail = NULL;

	if (getAllNodesFromOnClause(cur, &node_head, &node_tail, malloced_head, the_debug) != RETURN_GOOD)
	{
		if (the_debug == YES_DEBUG)
			printf("	ERROR in findMatchingRowsOfJoin() at line %d in %s\n", __LINE__, __FILE__);
		return RETURN_ERROR;
	}


	struct ListNodePtr* cols_in_on_head = NULL;
	struct ListNodePtr* cols_in_on_tail = NULL;

	struct select_node* left = (struct select_node*) the_table;
	struct select_node* right = the_select_node;

	struct ListNodePtr* cur_node = node_head;
	int count_nodes = 0;

	struct ListNodePtr* col_data_head = NULL;
	struct ListNodePtr* col_data_tail = NULL;

	while (cur_node != NULL)
	{
		// START Check if left on left and right on right
			if (cur_node->ptr_type * -1 == PTR_TYPE_COL_IN_SELECT_NODE)
			{
				if (checkIfColLeftOrRight(&cols_in_on_head, &cols_in_on_tail, left, right, cur_node->ptr_value, malloced_head, the_debug) != RETURN_GOOD)
				{
					if (the_debug == YES_DEBUG)
						printf("	ERROR in findMatchingRowsOfJoin() at line %d in %s\n", __LINE__, __FILE__);
					return RETURN_ERROR;
				}

				if (count_nodes % 2 == 0)
				{
					if (cols_in_on_tail->ptr_type != COL_IN_JOIN_IS_LEFT)
					{
						if (the_debug == YES_DEBUG)
							printf("	ERROR in findMatchingRowsOfJoin() at line %d in %s\n", __LINE__, __FILE__);
						printf("At least one column on the LEFT side of the inequality in an on clause was not from a LEFT column. Please use the following form for on clauses: \"only left cols and constants = only right cols and constants\"\n");
						errorTeardown(NULL, malloced_head, the_debug);
						return RETURN_ERROR;
					}
				}
				else
				{
					if (cols_in_on_tail->ptr_type != COL_IN_JOIN_IS_RIGHT)
					{
						if (the_debug == YES_DEBUG)
							printf("	ERROR in findMatchingRowsOfJoin() at line %d in %s\n", __LINE__, __FILE__);
						printf("At least one column on the RIGHT side of the inequality in an on clause was not from a RIGHT column. Please use the following form for on clauses: \"only left cols and constants = only right cols and constants\"\n");
						errorTeardown(NULL, malloced_head, the_debug);
						return RETURN_ERROR;
					}
				}
			}
			else if (cur_node->ptr_type * -1 == PTR_TYPE_FUNC_NODE)
			{
				struct ListNodePtr* temp_head = NULL;
				struct ListNodePtr* temp_tail = NULL;

				if (getAllColsFromFuncNode(&temp_head, &temp_tail, cur_node->ptr_value, NULL, NULL, malloced_head, the_debug) != 0)
				{
					if (the_debug == YES_DEBUG)
						printf("	ERROR in findMatchingRowsOfJoin() at line %d in %s\n", __LINE__, __FILE__);
					return RETURN_ERROR;
				}

				struct ListNodePtr* temp_cur = temp_head;
				while (temp_cur != NULL)
				{
					if (checkIfColLeftOrRight(&cols_in_on_head, &cols_in_on_tail, left, right, temp_cur->ptr_value, malloced_head, the_debug) != RETURN_GOOD)
					{
						if (the_debug == YES_DEBUG)
							printf("	ERROR in findMatchingRowsOfJoin() at line %d in %s\n", __LINE__, __FILE__);
						return RETURN_ERROR;
					}

					if (count_nodes % 2 == 0)
					{
						if (cols_in_on_tail->ptr_type != COL_IN_JOIN_IS_LEFT)
						{
							if (the_debug == YES_DEBUG)
								printf("	ERROR in findMatchingRowsOfJoin() at line %d in %s\n", __LINE__, __FILE__);
							printf("At least one column on the LEFT side of the inequality in an on clause was not from a LEFT column. Please use the following form for on clauses: \"only left cols and constants = only right cols and constants\"\n");
							errorTeardown(NULL, malloced_head, the_debug);
							return RETURN_ERROR;
						}
					}
					else
					{
						if (cols_in_on_tail->ptr_type != COL_IN_JOIN_IS_RIGHT)
						{
							if (the_debug == YES_DEBUG)
								printf("	ERROR in findMatchingRowsOfJoin() at line %d in %s\n", __LINE__, __FILE__);
							printf("At least one column on the RIGHT side of the inequality in an on clause was not from a RIGHT column. Please use the following form for on clauses: \"only left cols and constants = only right cols and constants\"\n");
							errorTeardown(NULL, malloced_head, the_debug);
							return RETURN_ERROR;
						}
					}

					temp_cur = temp_cur->next;
				}

				freeAnyLinkedList((void**) &temp_head, PTR_TYPE_LIST_NODE_PTR, NULL, malloced_head, the_debug);
			}
			else if (cur_node->ptr_type * -1 == PTR_TYPE_MATH_NODE)
			{
				struct ListNodePtr* temp_head = NULL;
				struct ListNodePtr* temp_tail = NULL;

				if (getAllColsFromMathNode(&temp_head, &temp_tail, cur_node->ptr_value, NULL, NULL, malloced_head, the_debug) != 0)
				{
					if (the_debug == YES_DEBUG)
						printf("	ERROR in findMatchingRowsOfJoin() at line %d in %s\n", __LINE__, __FILE__);
					return RETURN_ERROR;
				}

				struct ListNodePtr* temp_cur = temp_head;
				while (temp_cur != NULL)
				{
					if (checkIfColLeftOrRight(&cols_in_on_head, &cols_in_on_tail, left, right, temp_cur->ptr_value, malloced_head, the_debug) != RETURN_GOOD)
					{
						if (the_debug == YES_DEBUG)
							printf("	ERROR in findMatchingRowsOfJoin() at line %d in %s\n", __LINE__, __FILE__);
						return RETURN_ERROR;
					}

					if (count_nodes % 2 == 0)
					{
						if (cols_in_on_tail->ptr_type != COL_IN_JOIN_IS_LEFT)
						{
							if (the_debug == YES_DEBUG)
								printf("	ERROR in findMatchingRowsOfJoin() at line %d in %s\n", __LINE__, __FILE__);
							printf("At least one column on the LEFT side of the inequality in an on clause was not from a LEFT column. Please use the following form for on clauses: \"only left cols and constants = only right cols and constants\"\n");
							errorTeardown(NULL, malloced_head, the_debug);
							return RETURN_ERROR;
						}
					}
					else
					{
						if (cols_in_on_tail->ptr_type != COL_IN_JOIN_IS_RIGHT)
						{
							if (the_debug == YES_DEBUG)
								printf("	ERROR in findMatchingRowsOfJoin() at line %d in %s\n", __LINE__, __FILE__);
							printf("At least one column on the RIGHT side of the inequality in an on clause was not from a RIGHT column. Please use the following form for on clauses: \"only left cols and constants = only right cols and constants\"\n");
							errorTeardown(NULL, malloced_head, the_debug);
							return RETURN_ERROR;
						}
					}

					temp_cur = temp_cur->next;
				}

				freeAnyLinkedList((void**) &temp_head, PTR_TYPE_LIST_NODE_PTR, NULL, malloced_head, the_debug);
			}
			else if (cur_node->ptr_type * -1 == PTR_TYPE_CASE_NODE)
			{
				struct ListNodePtr* temp_head = NULL;
				struct ListNodePtr* temp_tail = NULL;

				if (getAllColsFromCaseNode(&temp_head, &temp_tail, cur_node->ptr_value, NULL, NULL, malloced_head, the_debug) != 0)
				{
					if (the_debug == YES_DEBUG)
						printf("	ERROR in findMatchingRowsOfJoin() at line %d in %s\n", __LINE__, __FILE__);
					return RETURN_ERROR;
				}

				struct ListNodePtr* temp_cur = temp_head;
				while (temp_cur != NULL)
				{
					if (checkIfColLeftOrRight(&cols_in_on_head, &cols_in_on_tail, left, right, temp_cur->ptr_value, malloced_head, the_debug) != RETURN_GOOD)
					{
						if (the_debug == YES_DEBUG)
							printf("	ERROR in findMatchingRowsOfJoin() at line %d in %s\n", __LINE__, __FILE__);
						return RETURN_ERROR;
					}

					if (count_nodes % 2 == 0)
					{
						if (cols_in_on_tail->ptr_type != COL_IN_JOIN_IS_LEFT)
						{
							if (the_debug == YES_DEBUG)
								printf("	ERROR in findMatchingRowsOfJoin() at line %d in %s\n", __LINE__, __FILE__);
							printf("At least one column on the LEFT side of the inequality in an on clause was not from a LEFT column. Please use the following form for on clauses: \"only left cols and constants = only right cols and constants\"\n");
							errorTeardown(NULL, malloced_head, the_debug);
							return RETURN_ERROR;
						}
					}
					else
					{
						if (cols_in_on_tail->ptr_type != COL_IN_JOIN_IS_RIGHT)
						{
							if (the_debug == YES_DEBUG)
								printf("	ERROR in findMatchingRowsOfJoin() at line %d in %s\n", __LINE__, __FILE__);
							printf("At least one column on the RIGHT side of the inequality in an on clause was not from a RIGHT column. Please use the following form for on clauses: \"only left cols and constants = only right cols and constants\"\n");
							errorTeardown(NULL, malloced_head, the_debug);
							return RETURN_ERROR;
						}
					}

					temp_cur = temp_cur->next;
				}

				freeAnyLinkedList((void**) &temp_head, PTR_TYPE_LIST_NODE_PTR, NULL, malloced_head, the_debug);
			}
		// END Check if left on left and right on right

		// START Init col_data_arrs for each node
			if (cur_node->ptr_type * -1 == PTR_TYPE_COL_IN_SELECT_NODE || cur_node->ptr_type * -1 == PTR_TYPE_MATH_NODE || cur_node->ptr_type * -1 == PTR_TYPE_CASE_NODE)
			{
				if (the_debug == YES_DEBUG)
					printf("Mallocing one temp_col_data_arr for %s\n", count_nodes % 2 != 0 ? "RIGHT" : "LEFT");

				struct colDataNode** temp_col_data_arr;

				int_8 temp_num_rows = left_num_rows;
				if (count_nodes % 2 != 0)
					temp_num_rows = right_num_rows;

				temp_col_data_arr = myMalloc(sizeof(struct colDataNode*) * temp_num_rows, NULL, malloced_head, the_debug);
				if (temp_col_data_arr == NULL)
				{
					if (the_debug == YES_DEBUG)
						printf("	ERROR in findMatchingRowsOfJoin() at line %d in %s\n", __LINE__, __FILE__);
					return RETURN_ERROR;
				}

				if (cur_node->ptr_type * -1 == PTR_TYPE_COL_IN_SELECT_NODE)
				{
					if (the_debug == YES_DEBUG)
						printf("temp_col_data_arr is PTR_TYPE_COL_IN_SELECT_NODE\n");

					for (int i=0; i<temp_num_rows; i++)
						temp_col_data_arr[i] = ((struct col_in_select_node*) cur_node->ptr_value)->col_data_arr[i];
				}
				else if (cur_node->ptr_type * -1 == PTR_TYPE_MATH_NODE)
				{
					if (the_debug == YES_DEBUG)
						printf("temp_col_data_arr is PTR_TYPE_MATH_NODE\n");

					for (int i=0; i<temp_num_rows; i++)
					{
						if (evaluateMathTree(cols_in_on_head, cols_in_on_tail, cur_node->ptr_value, i, i, malloced_head, the_debug) < 0)
						{
							if (the_debug == YES_DEBUG)
								printf("	ERROR in findMatchingRowsOfJoin() at line %d in %s\n", __LINE__, __FILE__);
							return RETURN_ERROR;
						}

						temp_col_data_arr[i] = myMalloc(sizeof(struct colDataNode), NULL, malloced_head, the_debug);
						temp_col_data_arr[i]->row_data = ((struct math_node*) cur_node->ptr_value)->result;
						temp_col_data_arr[i]->row_id = i;
					} 
				}
				else if (cur_node->ptr_type * -1 == PTR_TYPE_CASE_NODE)
				{
					if (the_debug == YES_DEBUG)
						printf("temp_col_data_arr is PTR_TYPE_CASE_NODE\n");

					for (int i=0; i<temp_num_rows; i++)
					{
						if (calcResultOfCaseForOneRow(cur_node->ptr_value, i, i, NULL, NULL, true, -1, -1, cols_in_on_head, cols_in_on_tail, malloced_head, the_debug) != RETURN_GOOD)
						{
							if (the_debug == YES_DEBUG)
								printf("	ERROR in findMatchingRowsOfJoin() at line %d in %s\n", __LINE__, __FILE__);
							return RETURN_ERROR;
						}

						temp_col_data_arr[i] = myMalloc(sizeof(struct colDataNode), NULL, malloced_head, the_debug);
						temp_col_data_arr[i]->row_data = ((struct case_node*) cur_node->ptr_value)->result;
						temp_col_data_arr[i]->row_id = i;
					}
				}

				if (addListNodePtr(&col_data_head, &col_data_tail, temp_col_data_arr, PTR_TYPE_COL_DATA_ARR, ADDLISTNODE_TAIL, NULL, malloced_head, the_debug) != 0)
				{
					if (the_debug == YES_DEBUG)
						printf("	ERROR in findMatchingRowsOfJoin() at line %d in %s\n", __LINE__, __FILE__);
					return RETURN_ERROR;
				}
			}
			else
			{
				if (addListNodePtr(&col_data_head, &col_data_tail, NULL, -1, ADDLISTNODE_TAIL, NULL, malloced_head, the_debug) != 0)
				{
					if (the_debug == YES_DEBUG)
						printf("	ERROR in findMatchingRowsOfJoin() at line %d in %s\n", __LINE__, __FILE__);
					return RETURN_ERROR;
				}
			}
		// END Init col_data_arrs for each node

		cur_node = cur_node->next;
		count_nodes++;
	}

	freeAnyLinkedList((void**) &cols_in_on_head, PTR_TYPE_LIST_NODE_PTR, NULL, malloced_head, the_debug);

	/**/
	// START Sort and reorder generated col_data_arrs
		if (the_debug == YES_DEBUG)
			printf("count_nodes = %d\n", count_nodes);

		for (int a=0; a<2; a++)
		{
			if (the_debug == YES_DEBUG)
				printf("Starting a = %d loop\n", a);

			int num_valid_cols = (count_nodes/2);

			// START Malloc temp arrs for all valid col's col_data_arr
				struct colDataNode*** col_data_arrs = myMalloc(sizeof(struct colDataNode**) * num_valid_cols, NULL, malloced_head, the_debug);
				if (col_data_arrs == NULL)
				{
					if (the_debug == YES_DEBUG)
						printf("	ERROR in findMatchingRowsOfJoin() at line %d in %s\n", __LINE__, __FILE__);
					return RETURN_ERROR;
				}

				int* datatype_arrs = myMalloc(sizeof(int) * num_valid_cols, NULL, malloced_head, the_debug);
				if (datatype_arrs == NULL)
				{
					if (the_debug == YES_DEBUG)
						printf("	ERROR in findMatchingRowsOfJoin() at line %d in %s\n", __LINE__, __FILE__);
					return RETURN_ERROR;
				}
			// END Malloc temp arrs for all valid col's col_data_arr

			// START Get all col_data_arr from col_data_head
				struct ListNodePtr* cur_col_data = col_data_head;
				if (a == 1)
					cur_col_data = cur_col_data->next;

				struct ListNodePtr* cur_node = node_head;
				if (a == 1)
					cur_node = cur_node->next;

				int temp_count_nodes = 0;
				while (cur_col_data != NULL)
				{
					if (cur_col_data->ptr_value != NULL)
					{
						int the_data_type;
						if (cur_node->ptr_type * -1 == PTR_TYPE_COL_IN_SELECT_NODE)
							the_data_type = ((struct col_in_select_node*) cur_node->ptr_value)->rows_data_type;
						else if (cur_node->ptr_type * -1 == PTR_TYPE_MATH_NODE)
							the_data_type = ((struct math_node*) cur_node->ptr_value)->result_type;
						else if (cur_node->ptr_type * -1 == PTR_TYPE_CASE_NODE)
							the_data_type = ((struct case_node*) cur_node->ptr_value)->result_type;

						if (the_debug == YES_DEBUG)
							printf("Merge sorting a col_data_arr with datatype = %d\n", the_data_type);

						col_data_arrs[temp_count_nodes] = cur_col_data->ptr_value;
						datatype_arrs[temp_count_nodes] = the_data_type;

						temp_count_nodes++;

						//mergeSort(cur_col_data->ptr_value, the_data_type, ORDER_BY_ASC, 0, a == 0 ? left_num_rows-1 : right_num_rows-1);
					}
					else
					{
						num_valid_cols--;
					}

					cur_col_data = cur_col_data->next;
					if (cur_col_data != NULL)
						cur_col_data = cur_col_data->next;

					cur_node = cur_node->next;
					if (cur_node != NULL)
						cur_node = cur_node->next;
				}
			// END Get all col_data_arr from col_data_head

			if (num_valid_cols > 0)
			{
				// START Actually sort and reorder rows
					int_8 max_rows = a == 0 ? left_num_rows : right_num_rows;

					mergeSort(col_data_arrs[0], datatype_arrs[0], ORDER_BY_ASC, 0, max_rows-1);

					for (int j=1; j<num_valid_cols; j++)
						reorderColsJustArr(col_data_arrs[j], col_data_arrs[0], 0, max_rows-1);

					for (int j=1; j<num_valid_cols; j++)
					{
						int left = 0;
						int right = 0;

						if (j < num_valid_cols-1)
							col_data_arrs[j][0]->row_id = 0;

						for (int i=1; i<max_rows; i++)
						{
							if (j < num_valid_cols-1)
								col_data_arrs[j][i]->row_id = i;

							int jj=0;
							for (; jj<j; jj++)
							{
								if (!equals(col_data_arrs[jj][i]->row_data, datatype_arrs[jj], col_data_arrs[jj][i-1]->row_data, VALUE_EQUALS))
								{
									break;
								}
							}

							if (jj == j)
							{
								//printf("B at i = %d\n", i);
								right++;
							}
							else
							{
								if (left < right)
								{
									//printf("C: %d - %d\n", left, right);
									mergeSort(col_data_arrs[j], datatype_arrs[j], ORDER_BY_ASC, left, right);

									for (int jj=j+1; jj<num_valid_cols; jj++)
										reorderColsJustArr(col_data_arrs[jj], col_data_arrs[j], left, right);
								}

								left = i;
								right = i;
							}
						}

						if (left < right)
						{
							//printf("C: %d - %d\n", left, right);
							mergeSort(col_data_arrs[j], datatype_arrs[j], ORDER_BY_ASC, left, right);

							for (int jj=j+1; jj<num_valid_cols; jj++)
								reorderColsJustArr(col_data_arrs[jj], col_data_arrs[j], left, right);
						}
					}

					/**/
					for (int j=0; j<num_valid_cols-1; j++)
					{
						for (int i=0; i<max_rows; i++)
						{
							col_data_arrs[j][i]->row_id = col_data_arrs[num_valid_cols-1][i]->row_id;
						}
					}

					/*
					// START Print column data
						for (int i=0; i<max_rows; i++)
						{
							for (int j=0; j<num_valid_cols; j++)
							{
								if (col_data_arrs[j][i]->row_data != NULL)
								{
									int the_data_type = datatype_arrs[j];

									if (the_data_type == DATA_INT)
										printf("%d", *((int*) col_data_arrs[j][i]->row_data));
									else if (the_data_type == DATA_REAL)
										printf("%lf", *((double*) col_data_arrs[j][i]->row_data));
									else
										printf("%s", col_data_arrs[j][i]->row_data);
								}

								printf(":%lu", col_data_arrs[j][i]->row_id);

								if (j < num_valid_cols-1)
									printf(",");
							}
							if (i < max_rows-1)
								printf("\n");
						}
						printf("\n");
					// END Print column data*/
				// END Actually sort and reorder rows
			}

			/*
			// START Reassinging col_data_arr to nodes in col_data_head
				cur_col_data = col_data_head;
				if (a == 1)
					cur_col_data = cur_col_data->next;

				cur_node = node_head;
				if (a == 1)
					cur_node = cur_node->next;

				temp_count_nodes = 0;
				while (cur_col_data != NULL)
				{
					if (cur_col_data->ptr_value != NULL)
					{
						if (the_debug == YES_DEBUG)
							printf("Reassinging col_data_arr from index = %d\n", temp_count_nodes);

						cur_col_data->ptr_value = col_data_arrs[temp_count_nodes];

						temp_count_nodes++;
					}

					cur_col_data = cur_col_data->next;
					if (cur_col_data != NULL)
						cur_col_data = cur_col_data->next;

					cur_node = cur_node->next;
					if (cur_node != NULL)
						cur_node = cur_node->next;
				}
			// END Reassinging col_data_arr to nodes in col_data_head*/

			myFree((void**) &col_data_arrs, NULL, malloced_head, the_debug);
			myFree((void**) &datatype_arrs, NULL, malloced_head, the_debug);
		}
	// END Sort and reorder generated col_data_arrs

	// START O(n) traversal of col_data_arr to find matching rows
		bool did_all_left = true;
		bool did_all_right = true;

		struct colDataNode** left_col_arr = NULL;
		struct colDataNode** right_col_arr = NULL;

		struct ListNodePtr* cur_col_data = col_data_head;
		while (cur_col_data != NULL)
		{
			if (cur_col_data->ptr_value != NULL)
			{
				left_col_arr = cur_col_data->ptr_value;
				break;
			}

			cur_col_data = cur_col_data->next;
			if (cur_col_data != NULL)
				cur_col_data = cur_col_data->next;
		}

		cur_col_data = col_data_head->next;
		while (cur_col_data != NULL)
		{
			if (cur_col_data->ptr_value != NULL)
			{
				right_col_arr = cur_col_data->ptr_value;
				break;
			}

			cur_col_data = cur_col_data->next;
			if (cur_col_data != NULL)
				cur_col_data = cur_col_data->next;
		}


		int i=0;
		int j=0;
		while (i < left_num_rows && j < right_num_rows)
		{
			//if (the_debug == YES_DEBUG) printf("i = %d, j = %d\n", i, j);

			if (evaluateWhereTreeForJoin(cur, node_head, col_data_head, i, j) == 1)
			{
				//if (the_debug == YES_DEBUG) printf("	MATCHES\n");

				// START Add row_id to matching list
					*num_rows_in_result = (*num_rows_in_result)+1;

					if (left_col_arr != NULL)
					{
						if (addListNodePtr_Int(left_head, left_tail, left_col_arr[i]->row_id, ADDLISTNODE_TAIL, NULL, malloced_head, the_debug) != 0)
						{
							if (the_debug == YES_DEBUG)
								printf("	ERROR in findMatchingRowsOfJoin() at line %d in %s\n", __LINE__, __FILE__);
							return RETURN_ERROR;
						}
					}
					else
					{
						if (addListNodePtr_Int(left_head, left_tail, i, ADDLISTNODE_TAIL, NULL, malloced_head, the_debug) != 0)
						{
							if (the_debug == YES_DEBUG)
								printf("	ERROR in findMatchingRowsOfJoin() at line %d in %s\n", __LINE__, __FILE__);
							return RETURN_ERROR;
						}
					}
						
					if (right_col_arr != NULL)
					{
						if (addListNodePtr_Int(right_head, right_tail, right_col_arr[j]->row_id, ADDLISTNODE_TAIL, NULL, malloced_head, the_debug) != 0)
						{
							if (the_debug == YES_DEBUG)
								printf("	ERROR in findMatchingRowsOfJoin() at line %d in %s\n", __LINE__, __FILE__);
							return RETURN_ERROR;
						}
					}
					else
					{
						if (addListNodePtr_Int(right_head, right_tail, j, ADDLISTNODE_TAIL, NULL, malloced_head, the_debug) != 0)
						{
							if (the_debug == YES_DEBUG)
								printf("	ERROR in findMatchingRowsOfJoin() at line %d in %s\n", __LINE__, __FILE__);
							return RETURN_ERROR;
						}
					}
				// END Add row_id to matching list

				
				if (i+1 == left_num_rows && did_all_left)
				{
					//if (the_debug == YES_DEBUG) printf("did_all_left without doing right, j = %d\n", j);
					j++;
					i = 0;
				}
				else if (j+1 == right_num_rows && did_all_right)
				{
					//if (the_debug == YES_DEBUG) printf("did_all_right without doing left, i = %d\n", i);
					i++;
					j = 0;
				}
				else if ((!did_all_right || i == 0) && i+1 < left_num_rows && nextRowIsEqual(true, node_head, col_data_head, i))
				{
					//printf("A\n");
					did_all_right = false;
					i++;
				}
				else if ((!did_all_left || j == 0) && j+1 < right_num_rows && nextRowIsEqual(false, node_head, col_data_head, j))
				{
					//printf("B\n");
					did_all_left = false;
					j++;
				}
				else if (i+1 < left_num_rows && !nextRowIsEqual(true, node_head, col_data_head, i))
				{
					//printf("C\n");
					did_all_right = false;
					i++;
				}
				else if (j+1 < right_num_rows && !nextRowIsEqual(false, node_head, col_data_head, j))
				{
					//printf("D\n");
					did_all_left = false;
					j++;
				}
				else if (i+1 == left_num_rows)
					i++;
				else if (j+1 == right_num_rows)
					j++;
			}
			else if (leftOrRightIsLess(true, node_head, col_data_head, i, j))
			{
				//printf("X\n");
				i++;
			}
			else if (leftOrRightIsLess(false, node_head, col_data_head, i, j))
			{
				//printf("Y\n");
				j++;
			}
			else
			{
				//printf("ptr_one = _%s_, ptr_two = _%s_\n", temp_ptr_one, temp_ptr_two);
				//printf("strcmp(ptr_one, ptr_two) == %d\n", strcmp(temp_ptr_one, temp_ptr_two));

				if (the_debug == YES_DEBUG)
					printf("	ERROR in findMatchingRowsOfJoin() at line %d in %s\n", __LINE__, __FILE__);
				errorTeardown(NULL, malloced_head, the_debug);
				return RETURN_ERROR;
			}
		}
	// END O(n) traversal of col_data_arr to find matching rows

	/*
	// START O(n^2) traversal of all rows and see if they equal
		for (int i=0; i<left_num_rows; i++)
		{
			for (int j=0; j<right_num_rows; j++)
			{
				if (evaluateWhereTreeForJoin(cur, node_head, col_data_head, i, j) == 1)
				{
					*num_rows_in_result = (*num_rows_in_result)+1;

					if (addListNodePtr_Int(left_head, left_tail, i, ADDLISTNODE_TAIL, NULL, malloced_head, the_debug) != 0)
					{
						if (the_debug == YES_DEBUG)
							printf("	ERROR in evaluateWhereTree() at line %d in %s\n", __LINE__, __FILE__);
						return RETURN_ERROR;
					}
					if (addListNodePtr_Int(right_head, right_tail, j, ADDLISTNODE_TAIL, NULL, malloced_head, the_debug) != 0)
					{
						if (the_debug == YES_DEBUG)
							printf("	ERROR in evaluateWhereTree() at line %d in %s\n", __LINE__, __FILE__);
						return RETURN_ERROR;
					}
				}
			}
		}
	// END O(n^2) traversal of all rows and see if they equal*/


	// START Free stuff
		count_nodes = 0;
		while (node_head != NULL)
		{
			struct ListNodePtr* temp_node = node_head;
			node_head = node_head->next;

			struct ListNodePtr* temp_col_data = col_data_head;
			col_data_head = col_data_head->next;

			if (temp_col_data->ptr_value != NULL)
			{
				if (temp_node->ptr_type * -1 == PTR_TYPE_MATH_NODE)
				{
					int_8 temp_num_rows = left_num_rows;
					if (count_nodes % 2 != 0)
						temp_num_rows = right_num_rows;

					for (int i=0; i<temp_num_rows; i++)
					{
						myFree((void**) &((struct colDataNode**) temp_col_data->ptr_value)[i]->row_data, NULL, malloced_head, the_debug);
						myFree((void**) &((struct colDataNode**) temp_col_data->ptr_value)[i], NULL, malloced_head, the_debug);
					}
				}
				else if (temp_node->ptr_type * -1 == PTR_TYPE_CASE_NODE)
				{
					int_8 temp_num_rows = left_num_rows;
					if (count_nodes % 2 != 0)
						temp_num_rows = right_num_rows;

					for (int i=0; i<temp_num_rows; i++)
					{
						myFree((void**) &((struct colDataNode**) temp_col_data->ptr_value)[i], NULL, malloced_head, the_debug);
					}
				}

				myFree((void**) &temp_col_data->ptr_value, NULL, malloced_head, the_debug);
			}

			myFree((void**) &temp_node, NULL, malloced_head, the_debug);
			myFree((void**) &temp_col_data, NULL, malloced_head, the_debug);

			count_nodes++;
		}
	// END Free stuff


	return RETURN_GOOD;


	// Below this line is a remnant of past functionality, only used for reference, not executed
	if (cur != NULL && ((struct where_clause_node*) cur)->ptr_one_type == PTR_TYPE_WHERE_CLAUSE_NODE && ((struct where_clause_node*) cur)->ptr_two_type == PTR_TYPE_WHERE_CLAUSE_NODE)
	{
		// START Set theory bc OR/AND from both nodes valid rows
			struct ListNodePtr* left_head_one = NULL;
			struct ListNodePtr* left_tail_one = NULL;
			struct ListNodePtr* right_head_one = NULL;
			struct ListNodePtr* right_tail_one = NULL;
			int_8 num_rows_in_result_one = 0;

			if (the_debug == YES_DEBUG)
				printf("Going to ptr_one\n");
			if (findMatchingRowsOfJoin(((struct where_clause_node*) cur)->ptr_one, the_table, the_select_node
									  ,&left_head_one, &left_tail_one, &right_head_one, &right_tail_one
									  ,groups_head, groups_tail, left_num_rows, right_num_rows, &num_rows_in_result_one
									  ,malloced_head, the_debug) != RETURN_GOOD)
			{
				if (the_debug == YES_DEBUG)
					printf("	ERROR in evaluateWhereTree() at line %d in %s\n", __LINE__, __FILE__);
				return RETURN_ERROR;
			}
			if (the_debug == YES_DEBUG)
				printf("Back from ptr_one\n");

			struct ListNodePtr* left_head_two = NULL;
			struct ListNodePtr* left_tail_two = NULL;
			struct ListNodePtr* right_head_two = NULL;
			struct ListNodePtr* right_tail_two = NULL;
			int_8 num_rows_in_result_two = 0;

			if (the_debug == YES_DEBUG)
				printf("Going to ptr_two\n");
			if (findMatchingRowsOfJoin(((struct where_clause_node*) cur)->ptr_two, the_table, the_select_node
									  ,&left_head_two, &left_tail_two, &right_head_two, &right_tail_two
									  ,groups_head, groups_tail, left_num_rows, right_num_rows, &num_rows_in_result_two
									  ,malloced_head, the_debug) != RETURN_GOOD)
			{
				if (the_debug == YES_DEBUG)
					printf("	ERROR in evaluateWhereTree() at line %d in %s\n", __LINE__, __FILE__);
				return RETURN_ERROR;
			}
			if (the_debug == YES_DEBUG)
				printf("Back from ptr_two\n");



			if (the_debug == YES_DEBUG)
			{
				traverseListNodesPtr(&left_head_one, NULL, TRAVERSELISTNODES_HEAD, "left_head_one = ");
				traverseListNodesPtr(&right_head_one, NULL, TRAVERSELISTNODES_HEAD, "right_head_one = ");
				traverseListNodesPtr(&left_head_two, NULL, TRAVERSELISTNODES_HEAD, "left_head_two = ");
				traverseListNodesPtr(&right_head_two, NULL, TRAVERSELISTNODES_HEAD, "right_head_two = ");
			}
			/*
			if (((struct where_clause_node*) cur)->where_type == WHERE_AND)
			{
				if (the_debug == YES_DEBUG)
					printf("AND\n");
				struct ListNodePtr* cur_one = head_one;
				while (cur_one != NULL)
				{
					//printf("Checking cur_one = %d\n", *((int*) cur_one->ptr_value));
					struct ListNodePtr* cur_two = head_two;
					while (cur_two != NULL)
					{
						//printf("Checking against cur_two = %d\n", *((int*) cur_two->ptr_value));
						if (equals(cur_one->ptr_value, cur_one->ptr_type, cur_two->ptr_value, VALUE_EQUALS))
						{
							break;
						}
						cur_two = cur_two->next;
					}

					struct ListNodePtr* temp = cur_one->next;

					if (cur_two == NULL)
					{
						//printf("Removing %d\n", *((int*) cur_one->ptr_value));
						removeListNodePtr(&head_one, &tail_one, cur_one, PTR_TYPE_LIST_NODE_PTR, -1, NULL, malloced_head, the_debug);
						//traverseListNodesPtr(&head_one, &tail_one, TRAVERSELISTNODES_HEAD, "After removal = ");
					}

					cur_one = temp;
				}

				freeAnyLinkedList((void**) &head_two, PTR_TYPE_LIST_NODE_PTR, NULL, malloced_head, the_debug);

				*head = head_one;
				*tail = tail_one;
			}
			else
			{
				if (the_debug == YES_DEBUG)
					printf("OR\n");
				tail_one->next = head_two;
				head_two->prev = tail_one;

				*head = head_one;
				*tail = tail_two;
			}

			if (the_debug == YES_DEBUG)
			{
				traverseListNodesPtr(left_head, NULL, TRAVERSELISTNODES_HEAD, "left_head at end = ");
				traverseListNodesPtr(right_head, NULL, TRAVERSELISTNODES_HEAD, "right_head at end = ");
			}*/
		// END Set theory bc OR/AND from both nodes valid rows
	}
	else
	{
		void* ptr_one = NULL;
		void* ptr_two = NULL;
		int ptr_one_type = -1;
		int ptr_two_type = -1;

		if (cur != NULL)
		{
			ptr_one = ((struct where_clause_node*) cur)->ptr_one;
			ptr_two = ((struct where_clause_node*) cur)->ptr_two;
			ptr_one_type = ((struct where_clause_node*) cur)->ptr_one_type;
			ptr_two_type = ((struct where_clause_node*) cur)->ptr_two_type;
		}

		if (ptr_one_type == PTR_TYPE_CHAR)
			undoDoubleQuotes(ptr_one);
		if (ptr_two_type == PTR_TYPE_CHAR)
			undoDoubleQuotes(ptr_two);

		struct ListNodePtr* cols_in_on_head = NULL;
		struct ListNodePtr* cols_in_on_tail = NULL;

		struct select_node* left = (struct select_node*) the_table;
		struct select_node* right = the_select_node;

		int the_data_type;

		// START Add all columns in on inequality to cols_in_on_head
			for (int a=0; a<2; a++)
			{
				int ptr_type = ptr_one_type;
				if (a == 1)
					ptr_type = ptr_two_type;

				void* ptr = ptr_one;
				if (a == 1)
					ptr = ptr_two;

				if (ptr_type == PTR_TYPE_COL_IN_SELECT_NODE)
				{
					if (checkIfColLeftOrRight(&cols_in_on_head, &cols_in_on_tail, left, right, ptr, malloced_head, the_debug) != RETURN_GOOD)
					{
						if (the_debug == YES_DEBUG)
							printf("	ERROR in evaluateWhereTree() at line %d in %s\n", __LINE__, __FILE__);
						return RETURN_ERROR;
					}

					if (a == 0)
					{
						if (cols_in_on_tail->ptr_type != COL_IN_JOIN_IS_LEFT)
						{
							if (the_debug == YES_DEBUG)
								printf("	ERROR in evaluateWhereTree() at line %d in %s\n", __LINE__, __FILE__);
							printf("At least one column on the LEFT side of the inequality in an on clause was not from a LEFT column. Please use the following form for on clauses: \"only left cols and constants = only right cols and constants\"\n");
							errorTeardown(NULL, malloced_head, the_debug);
							return RETURN_ERROR;
						}
					}
					else
					{
						if (cols_in_on_tail->ptr_type != COL_IN_JOIN_IS_RIGHT)
						{
							if (the_debug == YES_DEBUG)
								printf("	ERROR in evaluateWhereTree() at line %d in %s\n", __LINE__, __FILE__);
							printf("At least one column on the RIGHT side of the inequality in an on clause was not from a RIGHT column. Please use the following form for on clauses: \"only left cols and constants = only right cols and constants\"\n");
							errorTeardown(NULL, malloced_head, the_debug);
							return RETURN_ERROR;
						}
					}

					the_data_type = ((struct col_in_select_node*) ptr)->rows_data_type;
				}
				else if (ptr_type == PTR_TYPE_FUNC_NODE)
				{
					struct ListNodePtr* temp_head = NULL;
					struct ListNodePtr* temp_tail = NULL;

					if (getAllColsFromFuncNode(&temp_head, &temp_tail, ptr, NULL, NULL, malloced_head, the_debug) != 0)
					{
						if (the_debug == YES_DEBUG)
							printf("	ERROR in evaluateWhereTree() at line %d in %s\n", __LINE__, __FILE__);
						return RETURN_ERROR;
					}

					struct ListNodePtr* temp_cur = temp_head;
					while (temp_cur != NULL)
					{
						if (checkIfColLeftOrRight(&cols_in_on_head, &cols_in_on_tail, left, right, temp_cur->ptr_value, malloced_head, the_debug) != RETURN_GOOD)
						{
							if (the_debug == YES_DEBUG)
								printf("	ERROR in evaluateWhereTree() at line %d in %s\n", __LINE__, __FILE__);
							return RETURN_ERROR;
						}

						if (a == 0)
						{
							if (cols_in_on_tail->ptr_type != COL_IN_JOIN_IS_LEFT)
							{
								if (the_debug == YES_DEBUG)
									printf("	ERROR in evaluateWhereTree() at line %d in %s\n", __LINE__, __FILE__);
								printf("At least one column on the LEFT side of the inequality in an on clause was not from a LEFT column. Please use the following form for on clauses: \"only left cols and constants = only right cols and constants\"\n");
								errorTeardown(NULL, malloced_head, the_debug);
								return RETURN_ERROR;
							}
						}
						else
						{
							if (cols_in_on_tail->ptr_type != COL_IN_JOIN_IS_RIGHT)
							{
								if (the_debug == YES_DEBUG)
									printf("	ERROR in evaluateWhereTree() at line %d in %s\n", __LINE__, __FILE__);
								printf("At least one column on the RIGHT side of the inequality in an on clause was not from a RIGHT column. Please use the following form for on clauses: \"only left cols and constants = only right cols and constants\"\n");
								errorTeardown(NULL, malloced_head, the_debug);
								return RETURN_ERROR;
							}
						}

						temp_cur = temp_cur->next;
					}

					freeAnyLinkedList((void**) &temp_head, PTR_TYPE_LIST_NODE_PTR, NULL, malloced_head, the_debug);

					the_data_type = ((struct func_node*) ptr)->result_type;
				}
				else if (ptr_type == PTR_TYPE_MATH_NODE)
				{
					struct ListNodePtr* temp_head = NULL;
					struct ListNodePtr* temp_tail = NULL;

					if (getAllColsFromMathNode(&temp_head, &temp_tail, ptr, NULL, NULL, malloced_head, the_debug) != 0)
					{
						if (the_debug == YES_DEBUG)
							printf("	ERROR in evaluateWhereTree() at line %d in %s\n", __LINE__, __FILE__);
						return RETURN_ERROR;
					}

					struct ListNodePtr* temp_cur = temp_head;
					while (temp_cur != NULL)
					{
						if (checkIfColLeftOrRight(&cols_in_on_head, &cols_in_on_tail, left, right, temp_cur->ptr_value, malloced_head, the_debug) != RETURN_GOOD)
						{
							if (the_debug == YES_DEBUG)
								printf("	ERROR in evaluateWhereTree() at line %d in %s\n", __LINE__, __FILE__);
							return RETURN_ERROR;
						}

						if (a == 0)
						{
							if (cols_in_on_tail->ptr_type != COL_IN_JOIN_IS_LEFT)
							{
								if (the_debug == YES_DEBUG)
									printf("	ERROR in evaluateWhereTree() at line %d in %s\n", __LINE__, __FILE__);
								printf("At least one column on the LEFT side of the inequality in an on clause was not from a LEFT column. Please use the following form for on clauses: \"only left cols and constants = only right cols and constants\"\n");
								errorTeardown(NULL, malloced_head, the_debug);
								return RETURN_ERROR;
							}
						}
						else
						{
							if (cols_in_on_tail->ptr_type != COL_IN_JOIN_IS_RIGHT)
							{
								if (the_debug == YES_DEBUG)
									printf("	ERROR in evaluateWhereTree() at line %d in %s\n", __LINE__, __FILE__);
								printf("At least one column on the RIGHT side of the inequality in an on clause was not from a RIGHT column. Please use the following form for on clauses: \"only left cols and constants = only right cols and constants\"\n");
								errorTeardown(NULL, malloced_head, the_debug);
								return RETURN_ERROR;
							}
						}

						temp_cur = temp_cur->next;
					}

					freeAnyLinkedList((void**) &temp_head, PTR_TYPE_LIST_NODE_PTR, NULL, malloced_head, the_debug);

					the_data_type = ((struct math_node*) ptr)->result_type;
				}
				else if (ptr_type == PTR_TYPE_CASE_NODE)
				{
					struct ListNodePtr* temp_head = NULL;
					struct ListNodePtr* temp_tail = NULL;

					if (getAllColsFromCaseNode(&temp_head, &temp_tail, ptr, NULL, NULL, malloced_head, the_debug) != 0)
					{
						if (the_debug == YES_DEBUG)
							printf("	ERROR in evaluateWhereTree() at line %d in %s\n", __LINE__, __FILE__);
						return RETURN_ERROR;
					}

					struct ListNodePtr* temp_cur = temp_head;
					while (temp_cur != NULL)
					{
						if (checkIfColLeftOrRight(&cols_in_on_head, &cols_in_on_tail, left, right, temp_cur->ptr_value, malloced_head, the_debug) != RETURN_GOOD)
						{
							if (the_debug == YES_DEBUG)
								printf("	ERROR in evaluateWhereTree() at line %d in %s\n", __LINE__, __FILE__);
							return RETURN_ERROR;
						}

						if (a == 0)
						{
							if (cols_in_on_tail->ptr_type != COL_IN_JOIN_IS_LEFT)
							{
								if (the_debug == YES_DEBUG)
									printf("	ERROR in evaluateWhereTree() at line %d in %s\n", __LINE__, __FILE__);
								printf("At least one column on the LEFT side of the inequality in an on clause was not from a LEFT column. Please use the following form for on clauses: \"only left cols and constants = only right cols and constants\"\n");
								errorTeardown(NULL, malloced_head, the_debug);
								return RETURN_ERROR;
							}
						}
						else
						{
							if (cols_in_on_tail->ptr_type != COL_IN_JOIN_IS_RIGHT)
							{
								if (the_debug == YES_DEBUG)
									printf("	ERROR in evaluateWhereTree() at line %d in %s\n", __LINE__, __FILE__);
								printf("At least one column on the RIGHT side of the inequality in an on clause was not from a RIGHT column. Please use the following form for on clauses: \"only left cols and constants = only right cols and constants\"\n");
								errorTeardown(NULL, malloced_head, the_debug);
								return RETURN_ERROR;
							}
						}

						temp_cur = temp_cur->next;
					}

					freeAnyLinkedList((void**) &temp_head, PTR_TYPE_LIST_NODE_PTR, NULL, malloced_head, the_debug);

					the_data_type = ((struct case_node*) ptr)->result_type;
				}
				else
				{
					the_data_type = ptr_type;
				}
			}

			/*printf("In cols_in_on_head:\n");
			struct ListNodePtr* cur = cols_in_on_head;
			while (cur != NULL)
			{
				printf("%p: %d\n", cur->ptr_value, cur->ptr_type);

				cur = cur->next;
			}*/
		// END Add all columns in on inequality to cols_in_on_head

		/*
		// START Create temp col_data_arr and sort
			struct colDataNode** temp_one_col_data_arr;
			struct colDataNode** temp_two_col_data_arr;

			if (ptr_one_type == PTR_TYPE_COL_IN_SELECT_NODE)
			{
				temp_one_col_data_arr = myMalloc(sizeof(struct colDataNode*) * ((struct col_in_select_node*) ptr_one)->num_rows, NULL, malloced_head, the_debug);
				if (temp_one_col_data_arr == NULL)
				{
					if (the_debug == YES_DEBUG)
						printf("	ERROR in evaluateWhereTree() at line %d in %s\n", __LINE__, __FILE__);
					return RETURN_ERROR;
				}

				for (int i=0; i<((struct col_in_select_node*) ptr_one)->num_rows; i++)
				{
					temp_one_col_data_arr[i] = ((struct col_in_select_node*) ptr_one)->col_data_arr[i];
				}

				mergeSort(temp_one_col_data_arr, ((struct col_in_select_node*) ptr_one)->rows_data_type, ORDER_BY_ASC, 0, ((struct col_in_select_node*) ptr_one)->num_rows-1);
			}
			else if (ptr_one_type == PTR_TYPE_MATH_NODE)
			{
				
			}
			else if (ptr_one_type == PTR_TYPE_CASE_NODE)
			{
				
			}

			if (ptr_two_type == PTR_TYPE_COL_IN_SELECT_NODE)
			{
				temp_two_col_data_arr = myMalloc(sizeof(struct colDataNode*) * ((struct col_in_select_node*) ptr_two)->num_rows, NULL, malloced_head, the_debug);
				if (temp_two_col_data_arr == NULL)
				{
					if (the_debug == YES_DEBUG)
						printf("	ERROR in evaluateWhereTree() at line %d in %s\n", __LINE__, __FILE__);
					return RETURN_ERROR;
				}

				for (int i=0; i<((struct col_in_select_node*) ptr_two)->num_rows; i++)
				{
					temp_two_col_data_arr[i] = ((struct col_in_select_node*) ptr_two)->col_data_arr[i];
				}

				mergeSort(temp_two_col_data_arr, ((struct col_in_select_node*) ptr_two)->rows_data_type, ORDER_BY_ASC, 0, ((struct col_in_select_node*) ptr_two)->num_rows-1);
			}
			else if (ptr_two_type == PTR_TYPE_MATH_NODE)
			{

			}
			else if (ptr_two_type == PTR_TYPE_CASE_NODE)
			{

			}
		// END Create temp col_data_arr and sort

		// START O(n) traversal of col_data_arr to find matching rows
			int i=0;
			int j=0;
			while (i < left_num_rows && j < right_num_rows)
			{
				void* temp_ptr_one = NULL;
				void* temp_ptr_two = NULL;
				int_8 left_row_id;
				int_8 right_row_id;

				if (ptr_one_type == PTR_TYPE_COL_IN_SELECT_NODE)
				{
					struct ListNodePtr* cur_join_col = cols_in_on_head;
					while (cur_join_col != NULL)
					{
						if (cur_join_col->ptr_value == ptr_one)
						{
							if (cur_join_col->ptr_type == COL_IN_JOIN_IS_LEFT)
							{
								temp_ptr_one = temp_one_col_data_arr[i]->row_data;
								left_row_id = temp_one_col_data_arr[i]->row_id;
							}
							else //if (cur_join_col->ptr_type == COL_IN_JOIN_IS_RIGHT)
							{
								temp_ptr_one = temp_one_col_data_arr[j]->row_data;
								right_row_id = temp_one_col_data_arr[j]->row_id;
							}

							break;
						}

						cur_join_col = cur_join_col->next;
					}
				}
				else if (ptr_one_type == PTR_TYPE_MATH_NODE)
				{

				}
				else if (ptr_one_type == PTR_TYPE_CASE_NODE)
				{

				}

				if (ptr_two_type == PTR_TYPE_COL_IN_SELECT_NODE)
				{
					struct ListNodePtr* cur_join_col = cols_in_on_head;
					while (cur_join_col != NULL)
					{
						if (cur_join_col->ptr_value == ptr_two)
						{
							if (cur_join_col->ptr_type == COL_IN_JOIN_IS_LEFT)
							{
								temp_ptr_two = temp_two_col_data_arr[i]->row_data;
								left_row_id = temp_two_col_data_arr[i]->row_id;
							}
							else //if (cur_join_col->ptr_type == COL_IN_JOIN_IS_RIGHT)
							{
								temp_ptr_two = temp_two_col_data_arr[j]->row_data;
								right_row_id = temp_two_col_data_arr[j]->row_id;
							}

							break;
						}

						cur_join_col = cur_join_col->next;
					}
				}
				else if (ptr_two_type == PTR_TYPE_MATH_NODE)
				{

				}
				else if (ptr_two_type == PTR_TYPE_CASE_NODE)
				{

				}

				if (the_debug == YES_DEBUG)
				{
					//printf("left_row_id = %lu, right_row_id = %lu\n", left_row_id, right_row_id);
				}

				if (greatLess(temp_ptr_one, the_data_type, temp_ptr_two, WHERE_LESS_THAN))
					i++;
				else if (greatLess(temp_ptr_one, the_data_type, temp_ptr_two, WHERE_GREATER_THAN))
					j++;
				else if (equals(temp_ptr_one, the_data_type, temp_ptr_two, VALUE_EQUALS))
				{
					// START Add row_id to matching list
						*num_rows_in_result = (*num_rows_in_result)+1;

						bool did_left = false;
						bool did_right = false;

						struct ListNodePtr* cur = cols_in_on_head;
						while (cur != NULL)
						{
							struct col_in_select_node* the_col = ((struct col_in_select_node*) cur->ptr_value);

							int_8 the_index;

							if (cur->ptr_type == COL_IN_JOIN_IS_LEFT)
							{
								the_index = left_row_id;
								did_left = true;
								//printf("MATCHES Left row id = %lu (%s)\n", the_index, temp_ptr_one);
							}
							else //if (cur->ptr_type == COL_IN_JOIN_IS_RIGHT)
							{
								the_index = right_row_id;
								did_right = true;
								//printf("right row id = %lu (%s)\n", the_index, temp_ptr_two);
							}

							
							if (addListNodePtr_Int(&the_col->join_matching_rows_head, &the_col->join_matching_rows_tail, the_index, ADDLISTNODE_TAIL, NULL, malloced_head, the_debug) != 0)
							{
								if (the_debug == YES_DEBUG)
									printf("	ERROR in evaluateWhereTree() at line %d in %s\n", __LINE__, __FILE__);
								return RETURN_ERROR;
							}

							cur = cur->next;
						}

						if (!did_left)
						{
							int_8 the_index = left_row_id;
							if (addListNodePtr_Int(&left->prev->columns_arr[0]->join_matching_rows_head, &left->prev->columns_arr[0]->join_matching_rows_tail, the_index, ADDLISTNODE_TAIL, NULL, malloced_head, the_debug) != 0)
							{
								if (the_debug == YES_DEBUG)
									printf("	ERROR in evaluateWhereTree() at line %d in %s\n", __LINE__, __FILE__);
								return RETURN_ERROR;
							}
						}

						if (!did_right)
						{
							int_8 the_index = right_row_id;
							if (addListNodePtr_Int(&right->columns_arr[0]->join_matching_rows_head, &right->columns_arr[0]->join_matching_rows_tail, the_index, ADDLISTNODE_TAIL, NULL, malloced_head, the_debug) != 0)
							{
								if (the_debug == YES_DEBUG)
									printf("	ERROR in evaluateWhereTree() at line %d in %s\n", __LINE__, __FILE__);
								return RETURN_ERROR;
							}
						}
					// END Add row_id to matching list

					if (i+1 < left_num_rows && j+1 < right_num_rows)
					{
						if (!equals(temp_ptr_one, the_data_type, temp_one_col_data_arr[i+1]->row_data, VALUE_EQUALS) 
							&& !equals(temp_ptr_two, the_data_type, temp_two_col_data_arr[j+1]->row_data, VALUE_EQUALS))
						{
							i++;
							j++;
						}
						else
						{
							if (equals(temp_ptr_one, the_data_type, temp_one_col_data_arr[i+1]->row_data, VALUE_EQUALS))
								i++;
							if (equals(temp_ptr_two, the_data_type, temp_two_col_data_arr[j+1]->row_data, VALUE_EQUALS))
								j++;
						}
					}
					else
					{
						if ((i+1 < left_num_rows && equals(temp_ptr_one, the_data_type, temp_one_col_data_arr[i+1]->row_data, VALUE_EQUALS)) || i+1 == left_num_rows)
							i++;
						if ((j+1 < right_num_rows && equals(temp_ptr_two, the_data_type, temp_two_col_data_arr[j+1]->row_data, VALUE_EQUALS)) || j+1 == right_num_rows)
							j++;
					}
				}
				else
				{
					//printf("ptr_one = _%s_, ptr_two = _%s_\n", temp_ptr_one, temp_ptr_two);
					//printf("strcmp(ptr_one, ptr_two) == %d\n", strcmp(temp_ptr_one, temp_ptr_two));

					if (the_debug == YES_DEBUG)
						printf("	ERROR in evaluateWhereTree() at line %d in %s\n", __LINE__, __FILE__);
					errorTeardown(NULL, malloced_head, the_debug);
					return RETURN_ERROR;
				}
			}

			if (ptr_one_type == PTR_TYPE_MATH_NODE)
			{
				for (int i=0; i<((struct col_in_select_node*) ptr_one)->num_rows; i++)
				{
					myFree((void**) &temp_one_col_data_arr[i]->row_data, NULL, malloced_head, the_debug);
					myFree((void**) &temp_one_col_data_arr[i], NULL, malloced_head, the_debug);
				}
			}

			if (ptr_two_type == PTR_TYPE_MATH_NODE)
			{
				for (int i=0; i<((struct col_in_select_node*) ptr_two)->num_rows; i++)
				{
					myFree((void**) &temp_two_col_data_arr[i]->row_data, NULL, malloced_head, the_debug);
					myFree((void**) &temp_two_col_data_arr[i], NULL, malloced_head, the_debug);
				}
			}

			myFree((void**) &temp_one_col_data_arr, NULL, malloced_head, the_debug);
			myFree((void**) &temp_two_col_data_arr, NULL, malloced_head, the_debug);
		// END O(n) traversal of col_data_arr to find matching rows*/

		/**/
		// START O(n^2) traversal of all rows and see if they equal
			for (int i=0; i<left_num_rows; i++)
			{
				for (int j=0; j<right_num_rows; j++)
				{
					void* temp_ptr_one = NULL;
					void* temp_ptr_two = NULL;

					// START Init temp_ptr_one
						if (ptr_one_type == PTR_TYPE_COL_IN_SELECT_NODE)
						{
							struct ListNodePtr* cur_join_col = cols_in_on_head;
							while (cur_join_col != NULL)
							{
								if (cur_join_col->ptr_value == ptr_one)
								{
									if (cur_join_col->ptr_type == COL_IN_JOIN_IS_LEFT)
										temp_ptr_one = ((struct col_in_select_node*) ptr_one)->col_data_arr[i]->row_data;
									else //if (cur_join_col->ptr_type == COL_IN_JOIN_IS_RIGHT)
										temp_ptr_one = ((struct col_in_select_node*) ptr_one)->col_data_arr[j]->row_data;

									break;
								}

								cur_join_col = cur_join_col->next;
							}
						}
						else if (ptr_one_type == PTR_TYPE_MATH_NODE)
						{
							if (evaluateMathTree(cols_in_on_head, cols_in_on_tail, ptr_one, i, j, malloced_head, the_debug) < 0)
							{
								if (the_debug == YES_DEBUG)
									printf("	ERROR in evaluateWhereTree() at line %d in %s\n", __LINE__, __FILE__);
								return RETURN_ERROR;
							}

							temp_ptr_one = ((struct math_node*) ptr_one)->result;
						}
						else if (ptr_one_type == PTR_TYPE_CASE_NODE)
						{
							//printf("\n\ncalling calcResultOfCaseForOneRow() from evaluateWhereTree() ptr_one\n");
							if (calcResultOfCaseForOneRow(ptr_one, i, j, NULL, NULL, true, -1, -1, cols_in_on_head, cols_in_on_tail, malloced_head, the_debug) != RETURN_GOOD)
							{
								if (the_debug == YES_DEBUG)
									printf("	ERROR in evaluateWhereTree() at line %d in %s\n", __LINE__, __FILE__);
								return RETURN_ERROR;
							}

							the_data_type = ((struct case_node*) ptr_one)->result_type;
							temp_ptr_one = ((struct case_node*) ptr_one)->result;
						}
						else
							temp_ptr_one = ptr_one;
					// END Init temp_ptr_one

					// START Init temp_ptr_two
						if (ptr_two_type == PTR_TYPE_COL_IN_SELECT_NODE)
						{
							struct ListNodePtr* cur_join_col = cols_in_on_head;
							while (cur_join_col != NULL)
							{
								if (cur_join_col->ptr_value == ptr_two)
								{
									if (cur_join_col->ptr_type == COL_IN_JOIN_IS_LEFT)
										temp_ptr_two = ((struct col_in_select_node*) ptr_two)->col_data_arr[i]->row_data;
									else //if (cur_join_col->ptr_type == COL_IN_JOIN_IS_RIGHT)
										temp_ptr_two = ((struct col_in_select_node*) ptr_two)->col_data_arr[j]->row_data;

									break;
								}

								cur_join_col = cur_join_col->next;
							}
						}
						else if (ptr_two_type == PTR_TYPE_MATH_NODE)
						{
							if (evaluateMathTree(cols_in_on_head, cols_in_on_tail, ptr_two, i, j, malloced_head, the_debug) < 0)
							{
								if (the_debug == YES_DEBUG)
									printf("	ERROR in evaluateWhereTree() at line %d in %s\n", __LINE__, __FILE__);
								return RETURN_ERROR;
							}

							temp_ptr_two = ((struct math_node*) ptr_two)->result;
						}
						else if (ptr_two_type == PTR_TYPE_CASE_NODE)
						{
							//printf("calling calcResultOfCaseForOneRow() from evaluateWhereTree() ptr_two\n");
							if (calcResultOfCaseForOneRow(ptr_two, i, j, NULL, NULL, true, -1, -1, cols_in_on_head, cols_in_on_tail, malloced_head, the_debug) != RETURN_GOOD)
							{
								if (the_debug == YES_DEBUG)
									printf("	ERROR in evaluateWhereTree() at line %d in %s\n", __LINE__, __FILE__);
								return RETURN_ERROR;
							}

							temp_ptr_two = ((struct case_node*) ptr_two)->result;
						}
						else
							temp_ptr_two = ptr_two;
					// END Init temp_ptr_two


					//printf("Comparing one and two in evaluateWhereTree()\n");
					//printf("the_data_type = %d\n", the_data_type);
					//if (the_data_type == DATA_INT)
					//	printf("Comparing %d and %d\n", temp_ptr_one == NULL ? -1 : *((int*) temp_ptr_one), temp_ptr_two == NULL ? -1 : *((int*) temp_ptr_two));
					//else if (the_data_type == DATA_REAL)
					//	printf("Comparing %lf and %lf\n", temp_ptr_one == NULL ? -1.0 : *((double*) temp_ptr_one), temp_ptr_two == NULL ? -1 : *((double*) temp_ptr_two));
					//else if (the_data_type == DATA_STRING || the_data_type == DATA_DATE)
					//	printf("Comparing _%s_ and _%s_\n", temp_ptr_one, temp_ptr_two);

					if (equals(temp_ptr_one, the_data_type, temp_ptr_two, VALUE_EQUALS))
					{
						*num_rows_in_result = (*num_rows_in_result)+1;

						if (addListNodePtr_Int(left_head, left_tail, i, ADDLISTNODE_TAIL, NULL, malloced_head, the_debug) != 0)
						{
							if (the_debug == YES_DEBUG)
								printf("	ERROR in evaluateWhereTree() at line %d in %s\n", __LINE__, __FILE__);
							return RETURN_ERROR;
						}
						if (addListNodePtr_Int(right_head, right_tail, j, ADDLISTNODE_TAIL, NULL, malloced_head, the_debug) != 0)
						{
							if (the_debug == YES_DEBUG)
								printf("	ERROR in evaluateWhereTree() at line %d in %s\n", __LINE__, __FILE__);
							return RETURN_ERROR;
						}
					}

					if (ptr_one_type == PTR_TYPE_MATH_NODE)
						myFree((void**) &((struct math_node*) ptr_one)->result, NULL, malloced_head, the_debug);

					if (ptr_two_type == PTR_TYPE_MATH_NODE)
						myFree((void**) &((struct math_node*) ptr_two)->result, NULL, malloced_head, the_debug);
				}
			}
		// END O(n^2) traversal of all rows and see if they equal

		freeAnyLinkedList((void**) &cols_in_on_head, PTR_TYPE_LIST_NODE_PTR, NULL, malloced_head, the_debug);
	}

	return RETURN_GOOD;
}

int multi_evaluateWhereNode(struct thread_exec_where* thread_info)
{
	if (thread_info->the_debug == YES_DEBUG)
		printf("In thread: %d-%d\n", thread_info->index_from, thread_info->index_to);

	for (int i=thread_info->index_from; i<thread_info->index_to; i++)
	{
		int result = doWhere(thread_info->where_node->ptr_one, thread_info->where_node->ptr_two, thread_info->where_node->ptr_one_type
							,thread_info->where_node->ptr_two_type, thread_info->where_node->where_type, i, -1, thread_info->groups_head, thread_info->groups_tail
							,&thread_info->malloced_head, thread_info->the_debug);
		if (result == RETURN_ERROR)
		{
			if (thread_info->the_debug == YES_DEBUG)
				printf("	ERROR in multi_evaluateWhereNode() at line %d in %s\n", __LINE__, __FILE__);
			return RETURN_ERROR;
		}
		else if (result == 1)
		{
			if (thread_info->base_table)
			{
				int_8 temp_i = ((struct colDataNode**) thread_info->groups_head->ptr_value)[i]->row_id;

				//printf("i = %d while temp_i = %d\n", i, temp_i);
				if (addListNodePtr_Int(&thread_info->head, &thread_info->tail, temp_i, ADDLISTNODE_TAIL, NULL, &thread_info->malloced_head, thread_info->the_debug) != 0)
				{
					if (thread_info->the_debug == YES_DEBUG)
						printf("	ERROR in multi_evaluateWhereNode() at line %d in %s\n", __LINE__, __FILE__);
					return RETURN_ERROR;
				}
			}
			else
			{
				if (addListNodePtr_Int(&thread_info->head, &thread_info->tail, i, ADDLISTNODE_TAIL, NULL, &thread_info->malloced_head, thread_info->the_debug) != 0)
				{
					if (thread_info->the_debug == YES_DEBUG)
						printf("	ERROR in evaluateWhereTree() at line %d in %s\n", __LINE__, __FILE__);
					return RETURN_ERROR;
				}
			}
		}
	}

	return RETURN_GOOD;
}

/*	RETURNS:
 *  RETURN_ERROR if error
 *	RETURN_GOOD if good
 *
 *	WRITES TO:
 *	head
 *	tail
 */
int evaluateWhereTree(void* cur, struct table_info* the_table, struct select_node* the_select_node, struct ListNodePtr** head, struct ListNodePtr** tail
					 ,struct ListNodePtr* groups_head, struct ListNodePtr* groups_tail, bool join, int_8 left_num_rows, int_8 right_num_rows, int_8* num_rows_in_result
					 ,struct malloced_node** malloced_head, int the_debug)
{
	if (cur != NULL && ((struct where_clause_node*) cur)->ptr_one_type == PTR_TYPE_WHERE_CLAUSE_NODE && ((struct where_clause_node*) cur)->ptr_two_type == PTR_TYPE_WHERE_CLAUSE_NODE)
	{
		// START Set theory bc OR/AND from both nodes valid rows
			struct ListNodePtr* head_one = NULL;
			struct ListNodePtr* tail_one = NULL;

			if (the_debug == YES_DEBUG)
				printf("Going to ptr_one\n");
			if (evaluateWhereTree(((struct where_clause_node*) cur)->ptr_one, the_table, the_select_node, &head_one, &tail_one, groups_head, groups_tail, join, left_num_rows, right_num_rows, num_rows_in_result, malloced_head, the_debug) != RETURN_GOOD)
			{
				if (the_debug == YES_DEBUG)
					printf("	ERROR in evaluateWhereTree() at line %d in %s\n", __LINE__, __FILE__);
				return RETURN_ERROR;
			}
			if (the_debug == YES_DEBUG)
				printf("Back from ptr_one\n");

			struct ListNodePtr* head_two = NULL;
			struct ListNodePtr* tail_two = NULL;

			if (the_debug == YES_DEBUG)
				printf("Going to ptr_two\n");
			if (evaluateWhereTree(((struct where_clause_node*) cur)->ptr_two, the_table, the_select_node, &head_two, &tail_two, groups_head, groups_tail, join, left_num_rows, right_num_rows, num_rows_in_result, malloced_head, the_debug) != RETURN_GOOD)
			{
				if (the_debug == YES_DEBUG)
					printf("	ERROR in evaluateWhereTree() at line %d in %s\n", __LINE__, __FILE__);
				return RETURN_ERROR;
			}
			if (the_debug == YES_DEBUG)
				printf("Back from ptr_two\n");

			if (the_debug == YES_DEBUG)
			{
				traverseListNodesPtr(&head_one, &tail_one, TRAVERSELISTNODES_HEAD, "head_one = ");
				traverseListNodesPtr(&head_two, &tail_two, TRAVERSELISTNODES_HEAD, "head_two = ");
			}

			if (((struct where_clause_node*) cur)->where_type == WHERE_AND)
			{
				if (the_debug == YES_DEBUG)
					printf("AND\n");
				struct ListNodePtr* cur_one = head_one;
				while (cur_one != NULL)
				{
					//printf("Checking cur_one = %d\n", *((int*) cur_one->ptr_value));
					struct ListNodePtr* cur_two = head_two;
					while (cur_two != NULL)
					{
						//printf("Checking against cur_two = %d\n", *((int*) cur_two->ptr_value));
						if (equals(cur_one->ptr_value, cur_one->ptr_type, cur_two->ptr_value, VALUE_EQUALS))
						{
							break;
						}
						cur_two = cur_two->next;
					}

					struct ListNodePtr* temp = cur_one->next;

					if (cur_two == NULL)
					{
						//printf("Removing %d\n", *((int*) cur_one->ptr_value));
						removeListNodePtr(&head_one, &tail_one, cur_one, PTR_TYPE_LIST_NODE_PTR, -1, NULL, malloced_head, the_debug);
						//traverseListNodesPtr(&head_one, &tail_one, TRAVERSELISTNODES_HEAD, "After removal = ");
					}

					cur_one = temp;
				}

				freeAnyLinkedList((void**) &head_two, PTR_TYPE_LIST_NODE_PTR, NULL, malloced_head, the_debug);

				*head = head_one;
				*tail = tail_one;
			}
			else
			{
				if (the_debug == YES_DEBUG)
					printf("OR\n");
				tail_one->next = head_two;
				head_two->prev = tail_one;

				*head = head_one;
				*tail = tail_two;
			}

			if (the_debug == YES_DEBUG)
				traverseListNodesPtr(head, tail, TRAVERSELISTNODES_HEAD, "head at end = ");
		// END Set theory bc OR/AND from both nodes valid rows
	}
	else
	{
		void* ptr_one = NULL;
		void* ptr_two = NULL;
		int ptr_one_type = -1;
		int ptr_two_type = -1;

		if (cur != NULL)
		{
			ptr_one = ((struct where_clause_node*) cur)->ptr_one;
			ptr_two = ((struct where_clause_node*) cur)->ptr_two;
			ptr_one_type = ((struct where_clause_node*) cur)->ptr_one_type;
			ptr_two_type = ((struct where_clause_node*) cur)->ptr_two_type;
		}

		if (ptr_one_type == PTR_TYPE_CHAR)
			undoDoubleQuotes(ptr_one);
		if (ptr_two_type == PTR_TYPE_CHAR)
			undoDoubleQuotes(ptr_two);

		if (the_select_node != NULL)
		{
			if (!join)
			{
				/**/
				// START Try multithreading
					struct thread_exec_where* thread_arr[NUM_THREADS];
					int remainder = the_select_node->columns_arr[0]->num_rows % NUM_THREADS;
					int total = 0;
					for (int t=0; t<NUM_THREADS; t++)
					{
						int num_rows_per_thread = the_select_node->columns_arr[0]->num_rows/NUM_THREADS;
						if (remainder > 0)
						{
							num_rows_per_thread++;
							remainder--;
						}

						thread_arr[t] = myMalloc(sizeof(struct thread_exec_where), NULL, malloced_head, the_debug);
						if (thread_arr[t] == NULL)
						{
							if (the_debug == YES_DEBUG)
								printf("	ERROR in evaluateWhereTree() at line %d in %s\n", __LINE__, __FILE__);
							return NULL;
						}
						thread_arr[t]->where_node = cur;
						thread_arr[t]->base_table = false;
						thread_arr[t]->groups_head = groups_head;
						thread_arr[t]->groups_tail = groups_tail;
						thread_arr[t]->index_from = total;
						thread_arr[t]->index_to = total + num_rows_per_thread;

						thread_arr[t]->head = NULL;
						thread_arr[t]->tail = NULL;

						thread_arr[t]->malloced_head = NULL;
						thread_arr[t]->the_debug = the_debug;

						pthread_create(&thread_arr[t]->thread_id, NULL, multi_evaluateWhereNode, thread_arr[t]);

						total += num_rows_per_thread;
					}
					for (int t=0; t<NUM_THREADS; t++)
						pthread_join(thread_arr[t]->thread_id, NULL);

					for (int t=0; t<NUM_THREADS; t++)
					{
						// START Join malloc lists
							if (thread_arr[t]->malloced_head != NULL)
							{
								struct malloced_node* cur_malloc = thread_arr[t]->malloced_head;
								while (cur_malloc->next != NULL)
								{
									cur_malloc = cur_malloc->next;
								}

								cur_malloc->next = *malloced_head;
								(*malloced_head)->prev = cur_malloc;
								*malloced_head = thread_arr[t]->malloced_head;
							}
						// END Join malloc lists

						// START Join valid rows lists
							if (thread_arr[t]->head != NULL)
							{
								if (*head == NULL)
								{
									*head = thread_arr[t]->head;
									*tail = thread_arr[t]->tail;
								}
								else
								{
									(*tail)->next = thread_arr[t]->head;
									thread_arr[t]->head->prev = *tail;

									*tail = thread_arr[t]->tail;
								}
							}
						// END Join valid rows lists

						myFree((void**) &thread_arr[t], NULL, malloced_head, the_debug);
					}
				// END Try multithreading

				/*
				// START Normal
					for (int i=0; i<the_select_node->columns_arr[0]->num_rows; i++)
					{
						int result = doWhere(ptr_one, ptr_two, ptr_one_type, ptr_two_type, ((struct where_clause_node*) cur)->where_type, i, -1, groups_head, groups_tail
											,malloced_head, the_debug);
						if (result == RETURN_ERROR)
						{
							if (the_debug == YES_DEBUG)
								printf("	ERROR in evaluateWhereTree() at line %d in %s\n", __LINE__, __FILE__);
							return RETURN_ERROR;
						}
						else if (result == 1)
						{
							//if (the_debug == YES_DEBUG)
							//	printf("Found\n");
							if (addListNodePtr_Int(head, tail, i, ADDLISTNODE_TAIL, NULL, malloced_head, the_debug) != 0)
							{
								if (the_debug == YES_DEBUG)
									printf("	ERROR in evaluateWhereTree() at line %d in %s\n", __LINE__, __FILE__);
								return RETURN_ERROR;
							}
						}
					}
				// END Normal*/
			}
			else
			{
				struct ListNodePtr* cols_in_on_head = NULL;
				struct ListNodePtr* cols_in_on_tail = NULL;

				struct select_node* left = (struct select_node*) the_table;
				struct select_node* right = the_select_node;

				int the_data_type;

				// START Add all columns in on inequality to cols_in_on_head
					for (int a=0; a<2; a++)
					{
						int ptr_type = ptr_one_type;
						if (a == 1)
							ptr_type = ptr_two_type;

						void* ptr = ptr_one;
						if (a == 1)
							ptr = ptr_two;

						if (ptr_type == PTR_TYPE_COL_IN_SELECT_NODE)
						{
							if (checkIfColLeftOrRight(&cols_in_on_head, &cols_in_on_tail, left, right, ptr, malloced_head, the_debug) != RETURN_GOOD)
							{
								if (the_debug == YES_DEBUG)
									printf("	ERROR in evaluateWhereTree() at line %d in %s\n", __LINE__, __FILE__);
								return RETURN_ERROR;
							}

							if (a == 0)
							{
								if (cols_in_on_tail->ptr_type != COL_IN_JOIN_IS_LEFT)
								{
									if (the_debug == YES_DEBUG)
										printf("	ERROR in evaluateWhereTree() at line %d in %s\n", __LINE__, __FILE__);
									printf("At least one column on the LEFT side of the inequality in an on clause was not from a LEFT column. Please use the following form for on clauses: \"only left cols and constants = only right cols and constants\"\n");
									errorTeardown(NULL, malloced_head, the_debug);
									return RETURN_ERROR;
								}
							}
							else
							{
								if (cols_in_on_tail->ptr_type != COL_IN_JOIN_IS_RIGHT)
								{
									if (the_debug == YES_DEBUG)
										printf("	ERROR in evaluateWhereTree() at line %d in %s\n", __LINE__, __FILE__);
									printf("At least one column on the RIGHT side of the inequality in an on clause was not from a RIGHT column. Please use the following form for on clauses: \"only left cols and constants = only right cols and constants\"\n");
									errorTeardown(NULL, malloced_head, the_debug);
									return RETURN_ERROR;
								}
							}

							the_data_type = ((struct col_in_select_node*) ptr)->rows_data_type;
						}
						else if (ptr_type == PTR_TYPE_FUNC_NODE)
						{
							struct ListNodePtr* temp_head = NULL;
							struct ListNodePtr* temp_tail = NULL;

							if (getAllColsFromFuncNode(&temp_head, &temp_tail, ptr, NULL, NULL, malloced_head, the_debug) != 0)
							{
								if (the_debug == YES_DEBUG)
									printf("	ERROR in evaluateWhereTree() at line %d in %s\n", __LINE__, __FILE__);
								return RETURN_ERROR;
							}

							struct ListNodePtr* temp_cur = temp_head;
							while (temp_cur != NULL)
							{
								if (checkIfColLeftOrRight(&cols_in_on_head, &cols_in_on_tail, left, right, temp_cur->ptr_value, malloced_head, the_debug) != RETURN_GOOD)
								{
									if (the_debug == YES_DEBUG)
										printf("	ERROR in evaluateWhereTree() at line %d in %s\n", __LINE__, __FILE__);
									return RETURN_ERROR;
								}

								if (a == 0)
								{
									if (cols_in_on_tail->ptr_type != COL_IN_JOIN_IS_LEFT)
									{
										if (the_debug == YES_DEBUG)
											printf("	ERROR in evaluateWhereTree() at line %d in %s\n", __LINE__, __FILE__);
										printf("At least one column on the LEFT side of the inequality in an on clause was not from a LEFT column. Please use the following form for on clauses: \"only left cols and constants = only right cols and constants\"\n");
										errorTeardown(NULL, malloced_head, the_debug);
										return RETURN_ERROR;
									}
								}
								else
								{
									if (cols_in_on_tail->ptr_type != COL_IN_JOIN_IS_RIGHT)
									{
										if (the_debug == YES_DEBUG)
											printf("	ERROR in evaluateWhereTree() at line %d in %s\n", __LINE__, __FILE__);
										printf("At least one column on the RIGHT side of the inequality in an on clause was not from a RIGHT column. Please use the following form for on clauses: \"only left cols and constants = only right cols and constants\"\n");
										errorTeardown(NULL, malloced_head, the_debug);
										return RETURN_ERROR;
									}
								}

								temp_cur = temp_cur->next;
							}

							freeAnyLinkedList((void**) &temp_head, PTR_TYPE_LIST_NODE_PTR, NULL, malloced_head, the_debug);

							the_data_type = ((struct func_node*) ptr)->result_type;
						}
						else if (ptr_type == PTR_TYPE_MATH_NODE)
						{
							struct ListNodePtr* temp_head = NULL;
							struct ListNodePtr* temp_tail = NULL;

							if (getAllColsFromMathNode(&temp_head, &temp_tail, ptr, NULL, NULL, malloced_head, the_debug) != 0)
							{
								if (the_debug == YES_DEBUG)
									printf("	ERROR in evaluateWhereTree() at line %d in %s\n", __LINE__, __FILE__);
								return RETURN_ERROR;
							}

							struct ListNodePtr* temp_cur = temp_head;
							while (temp_cur != NULL)
							{
								if (checkIfColLeftOrRight(&cols_in_on_head, &cols_in_on_tail, left, right, temp_cur->ptr_value, malloced_head, the_debug) != RETURN_GOOD)
								{
									if (the_debug == YES_DEBUG)
										printf("	ERROR in evaluateWhereTree() at line %d in %s\n", __LINE__, __FILE__);
									return RETURN_ERROR;
								}

								if (a == 0)
								{
									if (cols_in_on_tail->ptr_type != COL_IN_JOIN_IS_LEFT)
									{
										if (the_debug == YES_DEBUG)
											printf("	ERROR in evaluateWhereTree() at line %d in %s\n", __LINE__, __FILE__);
										printf("At least one column on the LEFT side of the inequality in an on clause was not from a LEFT column. Please use the following form for on clauses: \"only left cols and constants = only right cols and constants\"\n");
										errorTeardown(NULL, malloced_head, the_debug);
										return RETURN_ERROR;
									}
								}
								else
								{
									if (cols_in_on_tail->ptr_type != COL_IN_JOIN_IS_RIGHT)
									{
										if (the_debug == YES_DEBUG)
											printf("	ERROR in evaluateWhereTree() at line %d in %s\n", __LINE__, __FILE__);
										printf("At least one column on the RIGHT side of the inequality in an on clause was not from a RIGHT column. Please use the following form for on clauses: \"only left cols and constants = only right cols and constants\"\n");
										errorTeardown(NULL, malloced_head, the_debug);
										return RETURN_ERROR;
									}
								}

								temp_cur = temp_cur->next;
							}

							freeAnyLinkedList((void**) &temp_head, PTR_TYPE_LIST_NODE_PTR, NULL, malloced_head, the_debug);

							the_data_type = ((struct math_node*) ptr)->result_type;
						}
						else if (ptr_type == PTR_TYPE_CASE_NODE)
						{
							struct ListNodePtr* temp_head = NULL;
							struct ListNodePtr* temp_tail = NULL;

							if (getAllColsFromCaseNode(&temp_head, &temp_tail, ptr, NULL, NULL, malloced_head, the_debug) != 0)
							{
								if (the_debug == YES_DEBUG)
									printf("	ERROR in evaluateWhereTree() at line %d in %s\n", __LINE__, __FILE__);
								return RETURN_ERROR;
							}

							struct ListNodePtr* temp_cur = temp_head;
							while (temp_cur != NULL)
							{
								if (checkIfColLeftOrRight(&cols_in_on_head, &cols_in_on_tail, left, right, temp_cur->ptr_value, malloced_head, the_debug) != RETURN_GOOD)
								{
									if (the_debug == YES_DEBUG)
										printf("	ERROR in evaluateWhereTree() at line %d in %s\n", __LINE__, __FILE__);
									return RETURN_ERROR;
								}

								if (a == 0)
								{
									if (cols_in_on_tail->ptr_type != COL_IN_JOIN_IS_LEFT)
									{
										if (the_debug == YES_DEBUG)
											printf("	ERROR in evaluateWhereTree() at line %d in %s\n", __LINE__, __FILE__);
										printf("At least one column on the LEFT side of the inequality in an on clause was not from a LEFT column. Please use the following form for on clauses: \"only left cols and constants = only right cols and constants\"\n");
										errorTeardown(NULL, malloced_head, the_debug);
										return RETURN_ERROR;
									}
								}
								else
								{
									if (cols_in_on_tail->ptr_type != COL_IN_JOIN_IS_RIGHT)
									{
										if (the_debug == YES_DEBUG)
											printf("	ERROR in evaluateWhereTree() at line %d in %s\n", __LINE__, __FILE__);
										printf("At least one column on the RIGHT side of the inequality in an on clause was not from a RIGHT column. Please use the following form for on clauses: \"only left cols and constants = only right cols and constants\"\n");
										errorTeardown(NULL, malloced_head, the_debug);
										return RETURN_ERROR;
									}
								}

								temp_cur = temp_cur->next;
							}

							freeAnyLinkedList((void**) &temp_head, PTR_TYPE_LIST_NODE_PTR, NULL, malloced_head, the_debug);

							the_data_type = ((struct case_node*) ptr)->result_type;
						}
						else
						{
							the_data_type = ptr_type;
						}
					}

					/*printf("In cols_in_on_head:\n");
					struct ListNodePtr* cur = cols_in_on_head;
					while (cur != NULL)
					{
						printf("%p: %d\n", cur->ptr_value, cur->ptr_type);

						cur = cur->next;
					}*/
				// END Add all columns in on inequality to cols_in_on_head

				/*
				// START Create temp col_data_arr and sort
					struct colDataNode** temp_one_col_data_arr;
					struct colDataNode** temp_two_col_data_arr;

					if (ptr_one_type == PTR_TYPE_COL_IN_SELECT_NODE)
					{
						temp_one_col_data_arr = myMalloc(sizeof(struct colDataNode*) * ((struct col_in_select_node*) ptr_one)->num_rows, NULL, malloced_head, the_debug);
						if (temp_one_col_data_arr == NULL)
						{
							if (the_debug == YES_DEBUG)
								printf("	ERROR in evaluateWhereTree() at line %d in %s\n", __LINE__, __FILE__);
							return RETURN_ERROR;
						}

						for (int i=0; i<((struct col_in_select_node*) ptr_one)->num_rows; i++)
						{
							temp_one_col_data_arr[i] = ((struct col_in_select_node*) ptr_one)->col_data_arr[i];
						}

						mergeSort(temp_one_col_data_arr, ((struct col_in_select_node*) ptr_one)->rows_data_type, ORDER_BY_ASC, 0, ((struct col_in_select_node*) ptr_one)->num_rows-1);
					}
					else if (ptr_one_type == PTR_TYPE_MATH_NODE)
					{
						
					}
					else if (ptr_one_type == PTR_TYPE_CASE_NODE)
					{
						
					}

					if (ptr_two_type == PTR_TYPE_COL_IN_SELECT_NODE)
					{
						temp_two_col_data_arr = myMalloc(sizeof(struct colDataNode*) * ((struct col_in_select_node*) ptr_two)->num_rows, NULL, malloced_head, the_debug);
						if (temp_two_col_data_arr == NULL)
						{
							if (the_debug == YES_DEBUG)
								printf("	ERROR in evaluateWhereTree() at line %d in %s\n", __LINE__, __FILE__);
							return RETURN_ERROR;
						}

						for (int i=0; i<((struct col_in_select_node*) ptr_two)->num_rows; i++)
						{
							temp_two_col_data_arr[i] = ((struct col_in_select_node*) ptr_two)->col_data_arr[i];
						}

						mergeSort(temp_two_col_data_arr, ((struct col_in_select_node*) ptr_two)->rows_data_type, ORDER_BY_ASC, 0, ((struct col_in_select_node*) ptr_two)->num_rows-1);
					}
					else if (ptr_two_type == PTR_TYPE_MATH_NODE)
					{

					}
					else if (ptr_two_type == PTR_TYPE_CASE_NODE)
					{

					}
				// END Create temp col_data_arr and sort

				// START O(n) traversal of col_data_arr to find matching rows
					int i=0;
					int j=0;
					while (i < left_num_rows && j < right_num_rows)
					{
						void* temp_ptr_one = NULL;
						void* temp_ptr_two = NULL;
						int_8 left_row_id;
						int_8 right_row_id;

						if (ptr_one_type == PTR_TYPE_COL_IN_SELECT_NODE)
						{
							struct ListNodePtr* cur_join_col = cols_in_on_head;
							while (cur_join_col != NULL)
							{
								if (cur_join_col->ptr_value == ptr_one)
								{
									if (cur_join_col->ptr_type == COL_IN_JOIN_IS_LEFT)
									{
										temp_ptr_one = temp_one_col_data_arr[i]->row_data;
										left_row_id = temp_one_col_data_arr[i]->row_id;
									}
									else //if (cur_join_col->ptr_type == COL_IN_JOIN_IS_RIGHT)
									{
										temp_ptr_one = temp_one_col_data_arr[j]->row_data;
										right_row_id = temp_one_col_data_arr[j]->row_id;
									}

									break;
								}

								cur_join_col = cur_join_col->next;
							}
						}
						else if (ptr_one_type == PTR_TYPE_MATH_NODE)
						{

						}
						else if (ptr_one_type == PTR_TYPE_CASE_NODE)
						{

						}

						if (ptr_two_type == PTR_TYPE_COL_IN_SELECT_NODE)
						{
							struct ListNodePtr* cur_join_col = cols_in_on_head;
							while (cur_join_col != NULL)
							{
								if (cur_join_col->ptr_value == ptr_two)
								{
									if (cur_join_col->ptr_type == COL_IN_JOIN_IS_LEFT)
									{
										temp_ptr_two = temp_two_col_data_arr[i]->row_data;
										left_row_id = temp_two_col_data_arr[i]->row_id;
									}
									else //if (cur_join_col->ptr_type == COL_IN_JOIN_IS_RIGHT)
									{
										temp_ptr_two = temp_two_col_data_arr[j]->row_data;
										right_row_id = temp_two_col_data_arr[j]->row_id;
									}

									break;
								}

								cur_join_col = cur_join_col->next;
							}
						}
						else if (ptr_two_type == PTR_TYPE_MATH_NODE)
						{

						}
						else if (ptr_two_type == PTR_TYPE_CASE_NODE)
						{

						}

						if (the_debug == YES_DEBUG)
						{
							//printf("left_row_id = %lu, right_row_id = %lu\n", left_row_id, right_row_id);
						}

						if (greatLess(temp_ptr_one, the_data_type, temp_ptr_two, WHERE_LESS_THAN))
							i++;
						else if (greatLess(temp_ptr_one, the_data_type, temp_ptr_two, WHERE_GREATER_THAN))
							j++;
						else if (equals(temp_ptr_one, the_data_type, temp_ptr_two, VALUE_EQUALS))
						{
							// START Add row_id to matching list
								*num_rows_in_result = (*num_rows_in_result)+1;

								bool did_left = false;
								bool did_right = false;

								struct ListNodePtr* cur = cols_in_on_head;
								while (cur != NULL)
								{
									struct col_in_select_node* the_col = ((struct col_in_select_node*) cur->ptr_value);

									int_8 the_index;

									if (cur->ptr_type == COL_IN_JOIN_IS_LEFT)
									{
										the_index = left_row_id;
										did_left = true;
										//printf("MATCHES Left row id = %lu (%s)\n", the_index, temp_ptr_one);
									}
									else //if (cur->ptr_type == COL_IN_JOIN_IS_RIGHT)
									{
										the_index = right_row_id;
										did_right = true;
										//printf("right row id = %lu (%s)\n", the_index, temp_ptr_two);
									}

									
									if (addListNodePtr_Int(&the_col->join_matching_rows_head, &the_col->join_matching_rows_tail, the_index, ADDLISTNODE_TAIL, NULL, malloced_head, the_debug) != 0)
									{
										if (the_debug == YES_DEBUG)
											printf("	ERROR in evaluateWhereTree() at line %d in %s\n", __LINE__, __FILE__);
										return RETURN_ERROR;
									}

									cur = cur->next;
								}

								if (!did_left)
								{
									int_8 the_index = left_row_id;
									if (addListNodePtr_Int(&left->prev->columns_arr[0]->join_matching_rows_head, &left->prev->columns_arr[0]->join_matching_rows_tail, the_index, ADDLISTNODE_TAIL, NULL, malloced_head, the_debug) != 0)
									{
										if (the_debug == YES_DEBUG)
											printf("	ERROR in evaluateWhereTree() at line %d in %s\n", __LINE__, __FILE__);
										return RETURN_ERROR;
									}
								}

								if (!did_right)
								{
									int_8 the_index = right_row_id;
									if (addListNodePtr_Int(&right->columns_arr[0]->join_matching_rows_head, &right->columns_arr[0]->join_matching_rows_tail, the_index, ADDLISTNODE_TAIL, NULL, malloced_head, the_debug) != 0)
									{
										if (the_debug == YES_DEBUG)
											printf("	ERROR in evaluateWhereTree() at line %d in %s\n", __LINE__, __FILE__);
										return RETURN_ERROR;
									}
								}
							// END Add row_id to matching list

							if (i+1 < left_num_rows && j+1 < right_num_rows)
							{
								if (!equals(temp_ptr_one, the_data_type, temp_one_col_data_arr[i+1]->row_data, VALUE_EQUALS) 
									&& !equals(temp_ptr_two, the_data_type, temp_two_col_data_arr[j+1]->row_data, VALUE_EQUALS))
								{
									i++;
									j++;
								}
								else
								{
									if (equals(temp_ptr_one, the_data_type, temp_one_col_data_arr[i+1]->row_data, VALUE_EQUALS))
										i++;
									if (equals(temp_ptr_two, the_data_type, temp_two_col_data_arr[j+1]->row_data, VALUE_EQUALS))
										j++;
								}
							}
							else
							{
								if ((i+1 < left_num_rows && equals(temp_ptr_one, the_data_type, temp_one_col_data_arr[i+1]->row_data, VALUE_EQUALS)) || i+1 == left_num_rows)
									i++;
								if ((j+1 < right_num_rows && equals(temp_ptr_two, the_data_type, temp_two_col_data_arr[j+1]->row_data, VALUE_EQUALS)) || j+1 == right_num_rows)
									j++;
							}
						}
						else
						{
							//printf("ptr_one = _%s_, ptr_two = _%s_\n", temp_ptr_one, temp_ptr_two);
							//printf("strcmp(ptr_one, ptr_two) == %d\n", strcmp(temp_ptr_one, temp_ptr_two));

							if (the_debug == YES_DEBUG)
								printf("	ERROR in evaluateWhereTree() at line %d in %s\n", __LINE__, __FILE__);
							errorTeardown(NULL, malloced_head, the_debug);
							return RETURN_ERROR;
						}
					}

					if (ptr_one_type == PTR_TYPE_MATH_NODE)
					{
						for (int i=0; i<((struct col_in_select_node*) ptr_one)->num_rows; i++)
						{
							myFree((void**) &temp_one_col_data_arr[i]->row_data, NULL, malloced_head, the_debug);
							myFree((void**) &temp_one_col_data_arr[i], NULL, malloced_head, the_debug);
						}
					}

					if (ptr_two_type == PTR_TYPE_MATH_NODE)
					{
						for (int i=0; i<((struct col_in_select_node*) ptr_two)->num_rows; i++)
						{
							myFree((void**) &temp_two_col_data_arr[i]->row_data, NULL, malloced_head, the_debug);
							myFree((void**) &temp_two_col_data_arr[i], NULL, malloced_head, the_debug);
						}
					}

					myFree((void**) &temp_one_col_data_arr, NULL, malloced_head, the_debug);
					myFree((void**) &temp_two_col_data_arr, NULL, malloced_head, the_debug);
				// END O(n) traversal of col_data_arr to find matching rows*/

				/**/
				// START O(n^2) traversal of all rows and see if they equal
					for (int i=0; i<left_num_rows; i++)
					{
						for (int j=0; j<right_num_rows; j++)
						{
							void* temp_ptr_one = NULL;
							void* temp_ptr_two = NULL;

							if (ptr_one_type == PTR_TYPE_COL_IN_SELECT_NODE)
							{
								struct ListNodePtr* cur_join_col = cols_in_on_head;
								while (cur_join_col != NULL)
								{
									if (cur_join_col->ptr_value == ptr_one)
									{
										if (cur_join_col->ptr_type == COL_IN_JOIN_IS_LEFT)
											temp_ptr_one = ((struct col_in_select_node*) ptr_one)->col_data_arr[i]->row_data;
										else //if (cur_join_col->ptr_type == COL_IN_JOIN_IS_RIGHT)
											temp_ptr_one = ((struct col_in_select_node*) ptr_one)->col_data_arr[j]->row_data;

										break;
									}

									cur_join_col = cur_join_col->next;
								}
							}
							else if (ptr_one_type == PTR_TYPE_MATH_NODE)
							{
								if (evaluateMathTree(cols_in_on_head, cols_in_on_tail, ptr_one, i, j, malloced_head, the_debug) < 0)
								{
									if (the_debug == YES_DEBUG)
										printf("	ERROR in evaluateWhereTree() at line %d in %s\n", __LINE__, __FILE__);
									return RETURN_ERROR;
								}

								temp_ptr_one = ((struct math_node*) ptr_one)->result;
							}
							else if (ptr_one_type == PTR_TYPE_CASE_NODE)
							{
								//printf("\n\ncalling calcResultOfCaseForOneRow() from evaluateWhereTree() ptr_one\n");
								if (calcResultOfCaseForOneRow(ptr_one, i, j, NULL, NULL, true, -1, -1, cols_in_on_head, cols_in_on_tail, malloced_head, the_debug) != RETURN_GOOD)
								{
									if (the_debug == YES_DEBUG)
										printf("	ERROR in evaluateWhereTree() at line %d in %s\n", __LINE__, __FILE__);
									return RETURN_ERROR;
								}

								the_data_type = ((struct case_node*) ptr_one)->result_type;
								temp_ptr_one = ((struct case_node*) ptr_one)->result;
							}
							else
								temp_ptr_one = ptr_one;



							if (ptr_two_type == PTR_TYPE_COL_IN_SELECT_NODE)
							{
								struct ListNodePtr* cur_join_col = cols_in_on_head;
								while (cur_join_col != NULL)
								{
									if (cur_join_col->ptr_value == ptr_two)
									{
										if (cur_join_col->ptr_type == COL_IN_JOIN_IS_LEFT)
											temp_ptr_two = ((struct col_in_select_node*) ptr_two)->col_data_arr[i]->row_data;
										else //if (cur_join_col->ptr_type == COL_IN_JOIN_IS_RIGHT)
											temp_ptr_two = ((struct col_in_select_node*) ptr_two)->col_data_arr[j]->row_data;

										break;
									}

									cur_join_col = cur_join_col->next;
								}
							}
							else if (ptr_two_type == PTR_TYPE_MATH_NODE)
							{
								if (evaluateMathTree(cols_in_on_head, cols_in_on_tail, ptr_two, i, j, malloced_head, the_debug) < 0)
								{
									if (the_debug == YES_DEBUG)
										printf("	ERROR in evaluateWhereTree() at line %d in %s\n", __LINE__, __FILE__);
									return RETURN_ERROR;
								}

								temp_ptr_two = ((struct math_node*) ptr_two)->result;
							}
							else if (ptr_two_type == PTR_TYPE_CASE_NODE)
							{
								//printf("calling calcResultOfCaseForOneRow() from evaluateWhereTree() ptr_two\n");
								if (calcResultOfCaseForOneRow(ptr_two, i, j, NULL, NULL, true, -1, -1, cols_in_on_head, cols_in_on_tail, malloced_head, the_debug) != RETURN_GOOD)
								{
									if (the_debug == YES_DEBUG)
										printf("	ERROR in evaluateWhereTree() at line %d in %s\n", __LINE__, __FILE__);
									return RETURN_ERROR;
								}

								temp_ptr_two = ((struct case_node*) ptr_two)->result;
							}
							else
								temp_ptr_two = ptr_two;


							//printf("Comparing one and two in evaluateWhereTree()\n");
							//printf("the_data_type = %d\n", the_data_type);
							//if (the_data_type == DATA_INT)
							//	printf("Comparing %d and %d\n", temp_ptr_one == NULL ? -1 : *((int*) temp_ptr_one), temp_ptr_two == NULL ? -1 : *((int*) temp_ptr_two));
							//else if (the_data_type == DATA_REAL)
							//	printf("Comparing %lf and %lf\n", temp_ptr_one == NULL ? -1.0 : *((double*) temp_ptr_one), temp_ptr_two == NULL ? -1 : *((double*) temp_ptr_two));
							//else if (the_data_type == DATA_STRING || the_data_type == DATA_DATE)
							//	printf("Comparing _%s_ and _%s_\n", temp_ptr_one, temp_ptr_two);

							if (equals(temp_ptr_one, the_data_type, temp_ptr_two, VALUE_EQUALS))
							{
								*num_rows_in_result = (*num_rows_in_result)+1;

								bool did_left = false;
								bool did_right = false;

								struct ListNodePtr* cur = cols_in_on_head;
								while (cur != NULL)
								{
									struct col_in_select_node* the_col = ((struct col_in_select_node*) cur->ptr_value);

									int_8 the_index;

									if (cur->ptr_type == COL_IN_JOIN_IS_LEFT)
									{
										the_index = i;
										did_left = true;
										//printf("MATCHES Left row id = %lu (%s)\n", the_index, temp_ptr_one);
									}
									else //if (cur->ptr_type == COL_IN_JOIN_IS_RIGHT)
									{
										the_index = j;
										did_right = true;
										//printf("right row id = %lu (%s)\n", the_index, temp_ptr_two);
									}

									
									if (addListNodePtr_Int(&the_col->join_matching_rows_head, &the_col->join_matching_rows_tail, the_index, ADDLISTNODE_TAIL, NULL, malloced_head, the_debug) != 0)
									{
										if (the_debug == YES_DEBUG)
											printf("	ERROR in evaluateWhereTree() at line %d in %s\n", __LINE__, __FILE__);
										return RETURN_ERROR;
									}

									cur = cur->next;
								}

								if (!did_left)
								{
									int_8 the_index = i;
									if (addListNodePtr_Int(&left->prev->columns_arr[0]->join_matching_rows_head, &left->prev->columns_arr[0]->join_matching_rows_tail, the_index, ADDLISTNODE_TAIL, NULL, malloced_head, the_debug) != 0)
									{
										if (the_debug == YES_DEBUG)
											printf("	ERROR in evaluateWhereTree() at line %d in %s\n", __LINE__, __FILE__);
										return RETURN_ERROR;
									}
								}

								if (!did_right)
								{
									int_8 the_index = j;
									if (addListNodePtr_Int(&right->columns_arr[0]->join_matching_rows_head, &right->columns_arr[0]->join_matching_rows_tail, the_index, ADDLISTNODE_TAIL, NULL, malloced_head, the_debug) != 0)
									{
										if (the_debug == YES_DEBUG)
											printf("	ERROR in evaluateWhereTree() at line %d in %s\n", __LINE__, __FILE__);
										return RETURN_ERROR;
									}
								}
							}

							if (ptr_one_type == PTR_TYPE_MATH_NODE)
								myFree((void**) &((struct math_node*) ptr_one)->result, NULL, malloced_head, the_debug);

							if (ptr_two_type == PTR_TYPE_MATH_NODE)
								myFree((void**) &((struct math_node*) ptr_two)->result, NULL, malloced_head, the_debug);
						}
					}
				// END O(n^2) traversal of all rows and see if they equal

				freeAnyLinkedList((void**) &cols_in_on_head, PTR_TYPE_LIST_NODE_PTR, NULL, malloced_head, the_debug);
			}
		}
		else if (the_table != NULL && the_table->table_cols_head->frequent_arr_row_to_node != NULL)
		{
			if (ptr_one_type == PTR_TYPE_TABLE_COLS_INFO ^ ptr_two_type == PTR_TYPE_TABLE_COLS_INFO)
			{
				struct table_cols_info* cur_col = ptr_one_type == PTR_TYPE_TABLE_COLS_INFO ? ptr_one : ptr_two;
				void* constant = ptr_one_type == PTR_TYPE_TABLE_COLS_INFO ? ptr_two : ptr_one;

				struct frequent_node* cur_freq;
				for (int j=0; j<2; j++)
				{
					if (j == 0)
						cur_freq = cur_col->frequent_list_head;
					else
						cur_freq = cur_col->unique_list_head;

					while (cur_freq != NULL)
					{
						//if (cur_col->data_type == DATA_INT)
						//	printf("Comparing %d and %d\n", *((int*) cur_freq->ptr_value), *((int*) constant));
						//else if (cur_col->data_type == DATA_REAL)
						//	printf("Comparing %lf and %lf\n", *((double*) cur_freq->ptr_value), *((double*) constant));
						//else
						//	printf("Comparing _%s_ and _%s_\n", cur_freq->ptr_value, constant);

						
						if (((struct where_clause_node*) cur)->where_type == WHERE_IS_EQUALS)
						{
							bool equals_bool = equals(cur_freq->ptr_value, cur_col->data_type, constant, VALUE_EQUALS);

							if (equals_bool)
							{
								//printf("		Matched\n");
								struct ListNodePtr* cur_node = cur_freq->row_nums_head;
								while (cur_node != NULL)
								{
									//printf("Adding %d\n", *((int*) cur_node->ptr_value));
									if (addListNodePtr_Int(head, tail, *((int*) cur_node->ptr_value), ADDLISTNODE_TAIL, NULL, malloced_head, the_debug) != 0)
									{
										if (the_debug == YES_DEBUG)
											printf("	ERROR in evaluateWhereTree() at line %d in %s\n", __LINE__, __FILE__);
										return RETURN_ERROR;
									}
									cur_node = cur_node->next;
								}
							}
						}
						else if (((struct where_clause_node*) cur)->where_type == WHERE_NOT_EQUALS)
						{
							bool equals_bool = equals(cur_freq->ptr_value, cur_col->data_type, constant, VALUE_EQUALS);

							if (!equals_bool)
							{
								//printf("		Matched\n");
								struct ListNodePtr* cur_node = cur_freq->row_nums_head;
								while (cur_node != NULL)
								{
									//printf("Adding %d\n", *((int*) cur_node->ptr_value));
									if (addListNodePtr_Int(head, tail, *((int*) cur_node->ptr_value), ADDLISTNODE_TAIL, NULL, malloced_head, the_debug) != 0)
									{
										if (the_debug == YES_DEBUG)
											printf("	ERROR in evaluateWhereTree() at line %d in %s\n", __LINE__, __FILE__);
										return RETURN_ERROR;
									}
									cur_node = cur_node->next;
								}
							}	
						}
						else if (((struct where_clause_node*) cur)->where_type == WHERE_GREATER_THAN || ((struct where_clause_node*) cur)->where_type == WHERE_GREATER_THAN_OR_EQUAL ||
								 ((struct where_clause_node*) cur)->where_type == WHERE_LESS_THAN || ((struct where_clause_node*) cur)->where_type == WHERE_LESS_THAN_OR_EQUAL)
						{
							bool is_true = greatLess(cur_freq->ptr_value, cur_col->data_type, constant, ((struct where_clause_node*) cur)->where_type);

							if ((is_true && ((struct where_clause_node*) cur)->where_type == WHERE_GREATER_THAN)
								|| (is_true && ((struct where_clause_node*) cur)->where_type == WHERE_GREATER_THAN_OR_EQUAL)
								|| (is_true && ((struct where_clause_node*) cur)->where_type == WHERE_LESS_THAN)
								|| (is_true && ((struct where_clause_node*) cur)->where_type == WHERE_LESS_THAN_OR_EQUAL))
							{
								//printf("		Matched\n");
								struct ListNodePtr* cur_node = cur_freq->row_nums_head;
								while (cur_node != NULL)
								{
									//printf("Adding %d\n", *((int*) cur_node->ptr_value));
									if (addListNodePtr_Int(head, tail, *((int*) cur_node->ptr_value), ADDLISTNODE_TAIL, NULL, malloced_head, the_debug) != 0)
									{
										if (the_debug == YES_DEBUG)
											printf("	ERROR in evaluateWhereTree() at line %d in %s\n", __LINE__, __FILE__);
										return RETURN_ERROR;
									}
									cur_node = cur_node->next;
								}
							}
						}
						else if (((struct where_clause_node*) cur)->where_type == WHERE_IS_NULL)
						{
							if (cur_freq->ptr_value == NULL)
							{
								//printf("		Matched\n");
								struct ListNodePtr* cur_node = cur_freq->row_nums_head;
								while (cur_node != NULL)
								{
									//printf("Adding %d\n", *((int*) cur_node->ptr_value));
									if (addListNodePtr_Int(head, tail, *((int*) cur_node->ptr_value), ADDLISTNODE_TAIL, NULL, malloced_head, the_debug) != 0)
									{
										if (the_debug == YES_DEBUG)
											printf("	ERROR in evaluateWhereTree() at line %d in %s\n", __LINE__, __FILE__);
										return RETURN_ERROR;
									}
									cur_node = cur_node->next;
								}
							}
						}
						else if (((struct where_clause_node*) cur)->where_type == WHERE_IS_NOT_NULL)
						{
							if (cur_freq->ptr_value != NULL)
							{
								//printf("		Matched\n");
								struct ListNodePtr* cur_node = cur_freq->row_nums_head;
								while (cur_node != NULL)
								{
									//printf("Adding %d\n", *((int*) cur_node->ptr_value));
									if (addListNodePtr_Int(head, tail, *((int*) cur_node->ptr_value), ADDLISTNODE_TAIL, NULL, malloced_head, the_debug) != 0)
									{
										if (the_debug == YES_DEBUG)
											printf("	ERROR in evaluateWhereTree() at line %d in %s\n", __LINE__, __FILE__);
										return RETURN_ERROR;
									}
									cur_node = cur_node->next;
								}
							}
						}

						cur_freq = cur_freq->next;
					}
				}
			}
			else if (ptr_one_type == PTR_TYPE_TABLE_COLS_INFO && ptr_two_type == PTR_TYPE_TABLE_COLS_INFO)
			{
				struct table_cols_info* cur_col_one = ptr_one;
				struct table_cols_info* cur_col_two = ptr_two;

				struct frequent_node* cur_freq_one;
				for (int a=0; a<2; a++)
				{
					if (a == 0)
						cur_freq_one = cur_col_one->frequent_list_head;
					else
						cur_freq_one = cur_col_one->unique_list_head;

					while (cur_freq_one != NULL)
					{
						struct frequent_node* cur_freq_two;
						for (int b=0; b<2; b++)
						{
							if (a == 0)
								cur_freq_two = cur_col_two->frequent_list_head;
							else
								cur_freq_two = cur_col_two->unique_list_head;
						}

						while (cur_freq_two != NULL)
						{
							//if (cur_col_one->data_type == DATA_INT)
							//	printf("Comparing %d and %d\n", *((int*) cur_freq_one->ptr_value), *((int*) cur_freq_two->ptr_value));
							//else if (cur_col_one->data_type == DATA_REAL)
							//	printf("Comparing %lf and %lf\n", *((double*) cur_freq_one->ptr_value), *((double*) cur_freq_two->ptr_value));
							//else if (cur_col_one->data_type == DATA_STRING || cur_col_one->data_type == DATA_DATE)
							//	printf("Comparing _%s_ and _%s_\n", cur_freq_one->ptr_value, cur_freq_two->ptr_value);

							if (((struct where_clause_node*) cur)->where_type == WHERE_IS_EQUALS)
							{
								bool equals_bool = equals(cur_freq_one->ptr_value, cur_col_one->data_type, cur_freq_two->ptr_value, VALUE_EQUALS);

								if (equals_bool)
								{
									//printf("		Matched\n");
									struct ListNodePtr* cur_node = cur_freq_one->row_nums_head;
									while (cur_node != NULL)
									{
										//printf("Adding %d\n", *((int*) cur_node->ptr_value));
										if (addListNodePtr_Int(head, tail, *((int*) cur_node->ptr_value), ADDLISTNODE_TAIL, NULL, malloced_head, the_debug) != 0)
										{
											if (the_debug == YES_DEBUG)
												printf("	ERROR in evaluateWhereTree() at line %d in %s\n", __LINE__, __FILE__);
											return RETURN_ERROR;
										}
										cur_node = cur_node->next;
									}
								}
							}
							else if (((struct where_clause_node*) cur)->where_type == WHERE_NOT_EQUALS)
							{
								bool equals_bool = equals(cur_freq_one->ptr_value, cur_col_one->data_type, cur_freq_two->ptr_value, VALUE_EQUALS);

								if (!equals_bool)
								{
									//printf("		Matched\n");
									struct ListNodePtr* cur_node = cur_freq_one->row_nums_head;
									while (cur_node != NULL)
									{
										//printf("Adding %d\n", *((int*) cur_node->ptr_value));
										if (addListNodePtr_Int(head, tail, *((int*) cur_node->ptr_value), ADDLISTNODE_TAIL, NULL, malloced_head, the_debug) != 0)
										{
											if (the_debug == YES_DEBUG)
												printf("	ERROR in evaluateWhereTree() at line %d in %s\n", __LINE__, __FILE__);
											return RETURN_ERROR;
										}
										cur_node = cur_node->next;
									}
								}	
							}
							else if (((struct where_clause_node*) cur)->where_type == WHERE_GREATER_THAN || ((struct where_clause_node*) cur)->where_type == WHERE_GREATER_THAN_OR_EQUAL ||
									 ((struct where_clause_node*) cur)->where_type == WHERE_LESS_THAN || ((struct where_clause_node*) cur)->where_type == WHERE_LESS_THAN_OR_EQUAL)
							{
								bool is_true = greatLess(cur_freq_one->ptr_value, cur_col_one->data_type, cur_freq_two->ptr_value, ((struct where_clause_node*) cur)->where_type);

								if ((is_true && ((struct where_clause_node*) cur)->where_type == WHERE_GREATER_THAN)
									|| (is_true && ((struct where_clause_node*) cur)->where_type == WHERE_GREATER_THAN_OR_EQUAL)
									|| (is_true && ((struct where_clause_node*) cur)->where_type == WHERE_LESS_THAN)
									|| (is_true && ((struct where_clause_node*) cur)->where_type == WHERE_LESS_THAN_OR_EQUAL))
								{
									//printf("		Matched\n");
									struct ListNodePtr* cur_node = cur_freq_one->row_nums_head;
									while (cur_node != NULL)
									{
										//printf("Adding %d\n", *((int*) cur_node->ptr_value));
										if (addListNodePtr_Int(head, tail, *((int*) cur_node->ptr_value), ADDLISTNODE_TAIL, NULL, malloced_head, the_debug) != 0)
										{
											if (the_debug == YES_DEBUG)
												printf("	ERROR in evaluateWhereTree() at line %d in %s\n", __LINE__, __FILE__);
											return RETURN_ERROR;
										}
										cur_node = cur_node->next;
									}
								}
							}

							cur_freq_two = cur_freq_two->next;
						}

						cur_freq_one = cur_freq_one->next;
					}
				}
			}
			else
			{
				bool valid_rows = false;

				if (((struct where_clause_node*) cur)->where_type == WHERE_IS_EQUALS)
				{
					if (equals(((struct where_clause_node*) cur)->ptr_one, ((struct where_clause_node*) cur)->ptr_one_type, ((struct where_clause_node*) cur)->ptr_two, VALUE_EQUALS))
						valid_rows = true;
				}
				else if (((struct where_clause_node*) cur)->where_type == WHERE_NOT_EQUALS)
				{
					if (!equals(((struct where_clause_node*) cur)->ptr_one, ((struct where_clause_node*) cur)->ptr_one_type, ((struct where_clause_node*) cur)->ptr_two, VALUE_EQUALS))
						valid_rows = true;
				}
				else if (((struct where_clause_node*) cur)->where_type == WHERE_GREATER_THAN || ((struct where_clause_node*) cur)->where_type == WHERE_GREATER_THAN_OR_EQUAL ||
						 ((struct where_clause_node*) cur)->where_type == WHERE_LESS_THAN || ((struct where_clause_node*) cur)->where_type == WHERE_LESS_THAN_OR_EQUAL)
				{
					bool is_true = greatLess(((struct where_clause_node*) cur)->ptr_one, ((struct where_clause_node*) cur)->ptr_one_type, ((struct where_clause_node*) cur)->ptr_two, ((struct where_clause_node*) cur)->where_type);

					if ((is_true && ((struct where_clause_node*) cur)->where_type == WHERE_GREATER_THAN)
						|| (is_true && ((struct where_clause_node*) cur)->where_type == WHERE_GREATER_THAN_OR_EQUAL)
						|| (is_true && ((struct where_clause_node*) cur)->where_type == WHERE_LESS_THAN)
						|| (is_true && ((struct where_clause_node*) cur)->where_type == WHERE_LESS_THAN_OR_EQUAL))
					{
						valid_rows = true;
					}
				}
				else if (((struct where_clause_node*) cur)->where_type == WHERE_IS_NULL)
				{
					if (((struct where_clause_node*) cur)->ptr_one == NULL)
						valid_rows = true;
				}
				else if (((struct where_clause_node*) cur)->where_type == WHERE_IS_NOT_NULL)
				{
					if (((struct where_clause_node*) cur)->ptr_one != NULL)
						valid_rows = true;
				}


				if (valid_rows)
				{
					struct table_cols_info* cur_col = the_table->table_cols_head;

					struct frequent_node* cur_freq;
					for (int j=0; j<2; j++)
					{
						if (j == 0)
							cur_freq = cur_col->frequent_list_head;
						else
							cur_freq = cur_col->unique_list_head;

						while (cur_freq != NULL)
						{
							struct ListNodePtr* cur_node = cur_freq->row_nums_head;
							while (cur_node != NULL)
							{
								if (addListNodePtr_Int(head, tail, *((int*) cur_node->ptr_value), ADDLISTNODE_TAIL, NULL, malloced_head, the_debug) != 0)
								{
									if (the_debug == YES_DEBUG)
										printf("	ERROR in evaluateWhereTree() at line %d in %s\n", __LINE__, __FILE__);
									return RETURN_ERROR;
								}
								cur_node = cur_node->next;
							}

							cur_freq = cur_freq->next;
						}
					}
				}
			}
		}
		else if (the_table != NULL && the_table->table_cols_head->frequent_arr_row_to_node == NULL)
		{
			/**/
			// START Try multithreading
				struct thread_exec_where* thread_arr[NUM_THREADS];
				int remainder = (the_table->table_cols_head->num_rows - the_table->table_cols_head->num_open) % NUM_THREADS;
				int total = 0;
				for (int t=0; t<NUM_THREADS; t++)
				{
					int num_rows_per_thread = (the_table->table_cols_head->num_rows - the_table->table_cols_head->num_open)/NUM_THREADS;
					if (remainder > 0)
					{
						num_rows_per_thread++;
						remainder--;
					}

					thread_arr[t] = myMalloc(sizeof(struct thread_exec_where), NULL, malloced_head, the_debug);
					if (thread_arr[t] == NULL)
					{
						if (the_debug == YES_DEBUG)
							printf("	ERROR in evaluateWhereTree() at line %d in %s\n", __LINE__, __FILE__);
						return NULL;
					}
					thread_arr[t]->where_node = cur;
					thread_arr[t]->base_table = true;
					thread_arr[t]->groups_head = groups_head;
					thread_arr[t]->groups_tail = groups_tail;
					thread_arr[t]->index_from = total;
					thread_arr[t]->index_to = total + num_rows_per_thread;

					thread_arr[t]->head = NULL;
					thread_arr[t]->tail = NULL;

					thread_arr[t]->malloced_head = NULL;
					thread_arr[t]->the_debug = the_debug;

					pthread_create(&thread_arr[t]->thread_id, NULL, multi_evaluateWhereNode, thread_arr[t]);

					total += num_rows_per_thread;
				}
				for (int t=0; t<NUM_THREADS; t++)
					pthread_join(thread_arr[t]->thread_id, NULL);

				for (int t=0; t<NUM_THREADS; t++)
				{
					// START Join malloc lists
						if (thread_arr[t]->malloced_head != NULL)
						{
							struct malloced_node* cur_malloc = thread_arr[t]->malloced_head;
							while (cur_malloc->next != NULL)
							{
								cur_malloc = cur_malloc->next;
							}

							cur_malloc->next = *malloced_head;
							(*malloced_head)->prev = cur_malloc;
							*malloced_head = thread_arr[t]->malloced_head;
						}
					// END Join malloc lists

					// START Join valid rows lists
						if (thread_arr[t]->head != NULL)
						{
							if (*head == NULL)
							{
								*head = thread_arr[t]->head;
								*tail = thread_arr[t]->tail;
							}
							else
							{
								(*tail)->next = thread_arr[t]->head;
								thread_arr[t]->head->prev = *tail;

								*tail = thread_arr[t]->tail;
							}
						}
					// END Join valid rows lists

					myFree((void**) &thread_arr[t], NULL, malloced_head, the_debug);
				}
			// END Try multithreading

			/*
			// START Normal
				printf("Normal\n");

				for (int i=0; i<(the_table->table_cols_head->num_rows - the_table->table_cols_head->num_open); i++)
				{
					int result = doWhere(ptr_one, ptr_two, ptr_one_type, ptr_two_type, ((struct where_clause_node*) cur)->where_type, i, -1, groups_head, groups_tail
										,malloced_head, the_debug);
					if (result == RETURN_ERROR)
					{
						if (the_debug == YES_DEBUG)
							printf("	ERROR in evaluateWhereTree() at line %d in %s\n", __LINE__, __FILE__);
						return RETURN_ERROR;
					}
					else if (result == 1)
					{
						int_8 temp_i = ((struct colDataNode**) groups_head->ptr_value)[i]->row_id;

						//printf("i = %d while temp_i = %d\n", i, temp_i);
						if (addListNodePtr_Int(head, tail, temp_i, ADDLISTNODE_TAIL, NULL, malloced_head, the_debug) != 0)
						{
							if (the_debug == YES_DEBUG)
								printf("	ERROR in evaluateWhereTree() at line %d in %s\n", __LINE__, __FILE__);
							return RETURN_ERROR;
						}
					}
				}
			// END Normal*/
		}
	}

	return RETURN_GOOD;
}

/*	RETURNS:
 *  RETURN_ERROR if error
 *	RETURN_GOOD if good
 *
 *	WRITES TO:
 *	valid_rows_head
 *	valid_rows_tail
 *	num_rows_in_result
 */
int findValidRowsGivenWhere(struct ListNodePtr** valid_rows_head, struct ListNodePtr** valid_rows_tail
						   ,struct select_node* the_select_node, struct table_info* the_table, struct where_clause_node* where_head
						   ,int_8* num_rows_in_result, struct ListNodePtr* groups_head, struct ListNodePtr* groups_tail, bool join
						   ,int_8 left_num_rows, int_8 right_num_rows
						   ,struct malloced_node** malloced_head, int the_debug)
{
	int_8 num_rows_to_check = 0;
	if (the_select_node != NULL)
	{
		num_rows_to_check = the_select_node->columns_arr[0]->num_rows;
	}
	else if (the_table != NULL)
	{
		num_rows_to_check = the_table->table_cols_head->num_rows;
	}
	if (the_debug == YES_DEBUG)
		printf("num_rows_to_check FOR WHERE = %lu\n", num_rows_to_check);

	// START Allocate array for valid rows
		int* valid_arr = (int*) myMalloc(sizeof(int) * num_rows_to_check, NULL, malloced_head, the_debug);
		if (valid_arr == NULL)
		{
			if (the_debug == YES_DEBUG)
				printf("	ERROR in findValidRowsGivenWhere() at line %d in %s\n", __LINE__, __FILE__);
			return RETURN_ERROR;
		}
		for (int i=0; i<num_rows_to_check; i++)
		{
			if (where_head == NULL)
				valid_arr[i] = 1;
			else
				valid_arr[i] = -2;
		}
	// END Allocate array for valid rows

	if (the_select_node == NULL && the_table != NULL)
	{
		// START Mark open rows as invalid
			struct ListNodePtr* cur_open = the_table->table_cols_head->open_list_head;
			while (cur_open != NULL)
			{
				//printf("row_id %lu is open\n", cur_open->value);

				valid_arr[*((int*) cur_open->ptr_value)] = -1;

				cur_open = cur_open->next;
			}
		// END Mark open rows as invalid
	}

	// START Traverse 2.0
		if (where_head != NULL || join)
		{
			struct ListNodePtr* temp_head = NULL;
			struct ListNodePtr* temp_tail = NULL;

			if (the_debug == YES_DEBUG)
				printf("Calling evaluateWhereTree()\n");
			if (evaluateWhereTree(where_head, the_table, the_select_node, &temp_head, &temp_tail, groups_head, groups_tail, join
								 ,left_num_rows, right_num_rows, num_rows_in_result, malloced_head, the_debug) != RETURN_GOOD)
			{
				if (the_debug == YES_DEBUG)
					printf("	ERROR in findValidRowsGivenWhere() at line %d in %s\n", __LINE__, __FILE__);
				return RETURN_ERROR;
			}

			if (join)
			{
				myFree((void**) &valid_arr, NULL, malloced_head, the_debug);

				return RETURN_GOOD;
			}

			struct ListNodePtr* cur = temp_head;
			while (cur != NULL)
			{
				valid_arr[*((int*) cur->ptr_value)] = 1;
				cur = cur->next;
			}

			freeAnyLinkedList((void**) &temp_head, PTR_TYPE_LIST_NODE_PTR, NULL, malloced_head, the_debug);
		}
	// END Traverse 2.0

	// START Create linked list for valid rows given valid_arr
		struct ListNodePtr* head = NULL;
		struct ListNodePtr* tail = NULL;

		if (the_table == NULL && the_select_node != NULL)
		{
			*num_rows_in_result = 0;
		}

		for (int i=0; i<num_rows_to_check; i++)
		{
			if (valid_arr[i] > 0)
			{
				int* int1 = (int*) myMalloc(sizeof(int), NULL, malloced_head, the_debug);
				if (int1 == NULL)
				{
					if (the_debug == YES_DEBUG)
						printf("	ERROR in findValidRowsGivenWhere() at line %d in %s\n", __LINE__, __FILE__);
					return RETURN_ERROR;
				}

				*int1 = i;

				if (addListNodePtr(&head, &tail, int1, PTR_TYPE_INT, ADDLISTNODE_TAIL, NULL, malloced_head, the_debug) != 0)
				{
					if (the_debug == YES_DEBUG)
						printf("	ERROR in findValidRowsGivenWhere() at line %d in %s\n", __LINE__, __FILE__);
					return RETURN_ERROR;
				}

				*num_rows_in_result = (*num_rows_in_result) + 1;
			}
		}
	// END Create linked list for valid rows given valid_arr

	myFree((void**) &valid_arr, NULL, malloced_head, the_debug);


	*valid_rows_head = head;
	*valid_rows_tail = tail;


	return RETURN_GOOD;
}

/*	RETURNS:
 *  RETURN_GOOD
 *
 *	WRITES TO:
 *	cur_col->col_data_arr
 */
int reorderCols(struct col_in_select_node* cur_col, struct col_in_select_node* most_recently_sorted_col, int left, int right)
{
	//struct colDataNode* temp[(right - left) + 1];
	struct colDataNode** temp = malloc(sizeof(struct colDataNode*) * ((right - left) + 1));

	for (int i=0; i<(right - left) + 1; i++)
	{
		//printf("Node at index %d is now node from previous index %d\n", i + left, most_recently_sorted_col->col_data_arr[i + left]->row_id);
		temp[i] = cur_col->col_data_arr[most_recently_sorted_col->col_data_arr[i + left]->row_id];
		//printf("Was _%s_\n", cur_col->col_data_arr[i + left]->row_data);
		//printf("Now _%s_\n", temp[i]->row_data);
	}

	for (int i=left; i<right+1; i++)
	{
		cur_col->col_data_arr[i] = temp[i - left];
		//cur_col->col_data_arr[i]->row_id = i;
	}

	free(temp);

	return RETURN_GOOD;
}

/*	RETURNS:
 *  RETURN_GOOD
 *
 *	WRITES TO:
 *	cur_col->col_data_arr
 */
int sortAndReorderCols(struct ListNodePtr* cur_row, struct ListNodePtr* cur_which, int left, int right)
{
	/*printf("Before Sort =\n");
	for (int ii=left; ii<right+1; ii++)
	{
		printf("_%s_,%lu\n", ((struct col_in_select_node*) cur_row->ptr_value)->col_data_arr[ii]->row_data, ((struct col_in_select_node*) cur_row->ptr_value)->col_data_arr[ii]->row_id);
	}*/

	mergeSort(((struct col_in_select_node*) cur_row->ptr_value)->col_data_arr, ((struct col_in_select_node*) cur_row->ptr_value)->rows_data_type
	  		  ,*((int*) cur_which->ptr_value), left, right);

	/*printf("After Sort =\n");
	for (int ii=left; ii<right+1; ii++)
	{
		printf("_%s_,%lu\n", ((struct col_in_select_node*) cur_row->ptr_value)->col_data_arr[ii]->row_data, ((struct col_in_select_node*) cur_row->ptr_value)->col_data_arr[ii]->row_id);
	}*/

	struct ListNodePtr* temp = cur_row->next;
	while (temp != NULL)
	{
		/*printf("Next order col BEFORE =\n");
		for (int ii=left; ii<right+1; ii++)
		{
			printf("_%s_,%lu\n", ((struct col_in_select_node*) temp->ptr_value)->col_data_arr[ii]->row_data, ((struct col_in_select_node*) temp->ptr_value)->col_data_arr[ii]->row_id);
		}*/
		
		reorderCols(temp->ptr_value, cur_row->ptr_value, left, right);

		/*printf("Next order col AFTER =\n");
		for (int ii=left; ii<right+1; ii++)
		{
			printf("_%s_,%lu\n", ((struct col_in_select_node*) temp->ptr_value)->col_data_arr[ii]->row_data, ((struct col_in_select_node*) temp->ptr_value)->col_data_arr[ii]->row_id);
		}*/

		temp = temp->next;
	}

	/*for (int i=left; i<right+1; i++)
	{
		((struct col_in_select_node*) cur_row->ptr_value)->col_data_arr[i]->row_id = i;
	}*/

	return RETURN_GOOD;
}

/*	RETURNS:
 *  RETURN_ERROR if error
 *	RETURN_GOOD if good
 *
 *	WRITES TO:
 *	the_select->columns_arr[j]->col_data_arr
 *	the_select->columns_arr[j]->num_rows
 *	num_rows_in_result
 */
int getDistinctRows(bool agg_funcs, struct select_node* the_select, struct func_node* the_func_node, struct ListNodePtr** head, struct ListNodePtr** tail
					,int_8* num_rows_in_result, struct malloced_node** malloced_head, int the_debug)
{
	if (agg_funcs && the_select == NULL)
	{
		*num_rows_in_result = ((struct col_in_select_node*) the_func_node->group_by_cols_head->ptr_value)->num_rows;
	}
	else //if (!agg_funcs && the_select != NULL)
	{
		*num_rows_in_result = the_select->columns_arr[0]->num_rows;
	}

	int_8 rows_before = *num_rows_in_result;
	if (the_debug == YES_DEBUG)
		printf("rows_before in getDistinctRows() = %lu\n", rows_before);

	// START Find unique and duplicates using col_data_arr
		for (int i=0; i<rows_before; i++)
		{
			bool unique = i == 0 ? true : false;

			//printf("Checking row %d for uniqueness\n", i);
			for (int ii=0; ii<i; ii++)
			{
				//printf("Comparing vs row %d\n", ii);
				struct ListNodePtr* cur_group = NULL;
				int j=0;
				if (agg_funcs && the_select == NULL)
				{
					cur_group = the_func_node->group_by_cols_head;
					while (cur_group != NULL)
					{
						//printf("Comparing column _%s_\n", ((struct table_cols_info*) ((struct col_in_select_node*) cur_group->ptr_value)->col_ptr)->col_name);
						int the_data_type = ((struct col_in_select_node*) cur_group->ptr_value)->rows_data_type;

						if (!equals(((struct col_in_select_node*) cur_group->ptr_value)->col_data_arr[i]->row_data, the_data_type
									,((struct col_in_select_node*) cur_group->ptr_value)->col_data_arr[ii]->row_data, VALUE_EQUALS))
						{
							//printf("Row %d does not equal row %d\n", i, ii);
							break;
						}

						cur_group = cur_group->next;
					}
				}
				else //if (!agg_funcs && the_select != NULL)
				{
					for (; j<the_select->columns_arr_size; j++)
					{
						//printf("Comparing column j = %d\n", j);

						int the_data_type = the_select->columns_arr[j]->rows_data_type;

						if (!equals(the_select->columns_arr[j]->col_data_arr[i]->row_data, the_data_type
									,the_select->columns_arr[j]->col_data_arr[ii]->row_data, VALUE_EQUALS))
						{
							//printf("Row %d does not equal row %d\n", i, ii);
							break;
						}
					}
				}
					

				if ((agg_funcs && cur_group == NULL) || (!agg_funcs && j == the_select->columns_arr_size))
				{
					//printf("Found duplicate row at %d (same as %d)\n", i, ii);

					if (*head == NULL)
					{
						struct group_data_node* group_data_node = (struct group_data_node*) myMalloc(sizeof(struct group_data_node), NULL, malloced_head, the_debug);
						if (group_data_node == NULL)
						{
							if (the_debug == YES_DEBUG)
								printf("	ERROR in getAggFuncGroups() at line %d in %s\n", __LINE__, __FILE__);
							return RETURN_ERROR;
						}
						group_data_node->row_ids_head = NULL;
						group_data_node->row_ids_tail = NULL;

						if (addListNodePtr(head, tail, group_data_node, PTR_TYPE_GROUP_DATE_NODE, ADDLISTNODE_TAIL, NULL, malloced_head, the_debug) != 0)
						{
							if (the_debug == YES_DEBUG)
								printf("	ERROR in getAggFuncGroups() at line %d in %s\n", __LINE__, __FILE__);
							return RETURN_ERROR;
						}

						if (addListNodePtr_Int(&group_data_node->row_ids_head, &group_data_node->row_ids_tail, i, ADDLISTNODE_TAIL, NULL, malloced_head, the_debug) != 0)
						{
							if (the_debug == YES_DEBUG)
								printf("	ERROR in getAggFuncGroups() at line %d in %s\n", __LINE__, __FILE__);
							return RETURN_ERROR;
						}
					}
					else
					{
						struct ListNodePtr* cur = *head;
						while (cur != NULL)
						{
							if (equals(((struct group_data_node*) cur->ptr_value)->row_ids_head->ptr_value, ((struct group_data_node*) cur->ptr_value)->row_ids_head->ptr_type, (void**) &ii, VALUE_EQUALS))
							{
								if (addListNodePtr_Int(&((struct group_data_node*) cur->ptr_value)->row_ids_head, &((struct group_data_node*) cur->ptr_value)->row_ids_tail, i, ADDLISTNODE_TAIL, NULL, malloced_head, the_debug) != 0)
								{
									if (the_debug == YES_DEBUG)
										printf("	ERROR in getAggFuncGroups() at line %d in %s\n", __LINE__, __FILE__);
									return RETURN_ERROR;
								}

								break;		
							}
							cur = cur->next;
						}
					}
						

					(*num_rows_in_result)--;

					break;
				}
				else if (ii+1 == i)
					unique = true;
			}

			if (unique)
			{
				//printf("Found unique row at i = %d\n", i);

				struct group_data_node* group_data_node = (struct group_data_node*) myMalloc(sizeof(struct group_data_node), NULL, malloced_head, the_debug);
				if (group_data_node == NULL)
				{
					if (the_debug == YES_DEBUG)
						printf("	ERROR in getAggFuncGroups() at line %d in %s\n", __LINE__, __FILE__);
					return RETURN_ERROR;
				}
				group_data_node->row_ids_head = NULL;
				group_data_node->row_ids_tail = NULL;

				if (addListNodePtr(head, tail, group_data_node, PTR_TYPE_GROUP_DATE_NODE, ADDLISTNODE_TAIL, NULL, malloced_head, the_debug) != 0)
				{
					if (the_debug == YES_DEBUG)
						printf("	ERROR in getAggFuncGroups() at line %d in %s\n", __LINE__, __FILE__);
					return RETURN_ERROR;
				}

				if (addListNodePtr_Int(&group_data_node->row_ids_head, &group_data_node->row_ids_tail, i, ADDLISTNODE_TAIL, NULL, malloced_head, the_debug) != 0)
				{
					if (the_debug == YES_DEBUG)
						printf("	ERROR in getAggFuncGroups() at line %d in %s\n", __LINE__, __FILE__);
					return RETURN_ERROR;
				}
			}
		}

		if (the_debug == YES_DEBUG)
		{
			printf("NOW *num_rows_in_result = %lu\n", *num_rows_in_result);

			struct ListNodePtr* cur = *head;
			while (cur != NULL)
			{
				traverseListNodesPtr(&((struct group_data_node*) cur->ptr_value)->row_ids_head, NULL, TRAVERSELISTNODES_HEAD, "Same values: ");

				cur = cur->next;
			}
		}
	// END Find unique and duplicates using col_data_arr

	if (agg_funcs && the_select == NULL && *num_rows_in_result < ((struct col_in_select_node*) the_func_node->group_by_cols_head->ptr_value)->num_rows)
	{
		// START Adjust grouped cols to exclude duplicates
			struct ListNodePtr* cur_group = the_func_node->group_by_cols_head;
			while (cur_group != NULL)
			{
				struct colDataNode** temp_new = (struct colDataNode**) myMalloc(sizeof(struct colDataNode*) * (*num_rows_in_result), NULL, malloced_head, the_debug);
				if (temp_new == NULL)
				{
					if (the_debug == YES_DEBUG)
						printf("	ERROR in getAggFuncGroups() at line %d in %s\n", __LINE__, __FILE__);
					return RETURN_ERROR;
				}

				struct ListNodePtr* cur_row = *head;
				for (int i=0; i<*num_rows_in_result; i++)
				{
					temp_new[i] = ((struct col_in_select_node*) cur_group->ptr_value)->col_data_arr[*((int*) ((struct group_data_node*) cur_row->ptr_value)->row_ids_head->ptr_value)];
					temp_new[i]->row_id = i;

					//printf("Transfering %d\n", *((int*) ((struct group_data_node*) cur_row->ptr_value)->row_ids_head->ptr_value));

					struct ListNodePtr* cur_row_id = ((struct group_data_node*) cur_row->ptr_value)->row_ids_head->next;
					while (cur_row_id != NULL)
					{
						//printf("Freeing %d\n", *((int*) cur_row_id->ptr_value));
						if (((struct col_in_select_node*) cur_group->ptr_value)->col_data_arr[*((int*) cur_row_id->ptr_value)]->row_data != NULL)
							myFree((void**) &((struct col_in_select_node*) cur_group->ptr_value)->col_data_arr[*((int*) cur_row_id->ptr_value)]->row_data, NULL, malloced_head, the_debug);
						myFree((void**) &((struct col_in_select_node*) cur_group->ptr_value)->col_data_arr[*((int*) cur_row_id->ptr_value)], NULL, malloced_head, the_debug);

						cur_row_id = cur_row_id->next;
					}

					cur_row = cur_row->next;
				}

				myFree((void**) &((struct col_in_select_node*) cur_group->ptr_value)->col_data_arr, NULL, malloced_head, the_debug);

				((struct col_in_select_node*) cur_group->ptr_value)->col_data_arr = temp_new;
				((struct col_in_select_node*) cur_group->ptr_value)->num_rows = *num_rows_in_result;

				cur_group = cur_group->next;
			}
		// END Adjust grouped cols to exclude duplicates
	} 
	else if (!agg_funcs && the_select != NULL && *num_rows_in_result < the_select->columns_arr[0]->num_rows)
	{
		// START Adjust rows to exclude duplicates
			for (int j=0; j<the_select->columns_arr_size; j++)
			{
				//printf("Freeing duplicates from col j = %d\n", j);
				struct colDataNode** temp_new = (struct colDataNode**) myMalloc(sizeof(struct colDataNode*) * (*num_rows_in_result), NULL, malloced_head, the_debug);
				if (temp_new == NULL)
				{
					if (the_debug == YES_DEBUG)
						printf("	ERROR in getAggFuncGroups() at line %d in %s\n", __LINE__, __FILE__);
					return RETURN_ERROR;
				}

				struct ListNodePtr* cur_row = *head;
				for (int i=0; i<*num_rows_in_result; i++)
				{
					temp_new[i] = the_select->columns_arr[j]->col_data_arr[*((int*) ((struct group_data_node*) cur_row->ptr_value)->row_ids_head->ptr_value)];
					temp_new[i]->row_id = i;

					struct ListNodePtr* cur_row_id = ((struct group_data_node*) cur_row->ptr_value)->row_ids_head->next;
					while (cur_row_id != NULL)
					{
						//printf("Freeing %d\n", *((int*) cur_row_id->ptr_value));
						if (the_select->columns_arr[j]->col_data_arr[*((int*) cur_row_id->ptr_value)]->row_data != NULL)
							myFree((void**) &the_select->columns_arr[j]->col_data_arr[*((int*) cur_row_id->ptr_value)]->row_data, NULL, malloced_head, the_debug);
						myFree((void**) &the_select->columns_arr[j]->col_data_arr[*((int*) cur_row_id->ptr_value)], NULL, malloced_head, the_debug);

						cur_row_id = cur_row_id->next;
					}

					cur_row = cur_row->next;
				}

				myFree((void**) &the_select->columns_arr[j]->col_data_arr, NULL, malloced_head, the_debug);

				the_select->columns_arr[j]->col_data_arr = temp_new;
				the_select->columns_arr[j]->num_rows = *num_rows_in_result;
			}
		// END Adjust rows to exclude duplicates
	}

	return RETURN_GOOD;
}

int getDistinctRowsV2(bool agg_funcs, struct select_node* the_select, struct func_node* the_func_node, struct ListNodePtr** head, struct ListNodePtr** tail
					,int_8* num_rows_in_result, struct malloced_node** malloced_head, int the_debug)
{
	if (agg_funcs)
	{
		*num_rows_in_result = ((struct col_in_select_node*) the_func_node->group_by_cols_head->ptr_value)->num_rows;

		int_8 rows_before = *num_rows_in_result;
		if (the_debug == YES_DEBUG)
			printf("rows_before in getDistinctRowsV2() = %lu\n", rows_before);

		// START Sort and reorder cols
			struct ListNodePtr* cur_group = the_func_node->group_by_cols_head;

			struct col_in_select_node* sorted_col = cur_group->ptr_value;

			mergeSort(((struct col_in_select_node*) cur_group->ptr_value)->col_data_arr, ((struct col_in_select_node*) cur_group->ptr_value)->rows_data_type
					,ORDER_BY_ASC, 0, rows_before-1);

			while (cur_group->next != NULL)
			{
				reorderCols(cur_group->next->ptr_value, sorted_col, 0, rows_before-1);

				cur_group = cur_group->next;
			}

			for (int j=0; j<the_select->prev->columns_arr_size; j++)
			{
				struct ListNodePtr* temp = the_func_node->group_by_cols_head;
				while (temp != NULL)
				{
					if (temp->ptr_value == the_select->prev->columns_arr[j])
						break;
					temp = temp->next;
				}

				if (temp == NULL)
				{
					//if (the_debug == YES_DEBUG) printf("Reordering prev's col at j = %d\n", j);
					reorderCols(the_select->prev->columns_arr[j], sorted_col, 0, rows_before-1);
				}
			}


			cur_group = the_func_node->group_by_cols_head->next;
			while (cur_group != NULL)
			{
				int left = 0;
				int right = 0;

				((struct col_in_select_node*) cur_group->ptr_value)->col_data_arr[0]->row_id = 0;

				for (int i=1; i<((struct col_in_select_node*) cur_group->ptr_value)->num_rows; i++)
				{
					((struct col_in_select_node*) cur_group->ptr_value)->col_data_arr[i]->row_id = i;

					struct ListNodePtr* temp = the_func_node->group_by_cols_head;
					while (temp != cur_group)
					{
						if (!equals(((struct col_in_select_node*) temp->ptr_value)->col_data_arr[i]->row_data, ((struct col_in_select_node*) temp->ptr_value)->rows_data_type
									,((struct col_in_select_node*) temp->ptr_value)->col_data_arr[i-1]->row_data, VALUE_EQUALS))
						{
							break;
						}

						temp = temp->next;
					}

					if (temp == cur_group)
					{
						//printf("B at i = %d\n", i);
						right++;
					}
					else
					{
						if (left < right)
						{
							//printf("C: %d - %d\n", left, right);
							mergeSort(((struct col_in_select_node*) cur_group->ptr_value)->col_data_arr, ((struct col_in_select_node*) cur_group->ptr_value)->rows_data_type
									,ORDER_BY_ASC, left, right);

							struct ListNodePtr* temp = cur_group->next;
							while (temp != NULL)
							{
								reorderCols(temp->ptr_value, cur_group->ptr_value, left, right);
								temp = temp->next;
							}

							for (int j=0; j<the_select->prev->columns_arr_size; j++)
							{
								struct ListNodePtr* temp = the_func_node->group_by_cols_head;
								while (temp != NULL)
								{
									if (temp->ptr_value == the_select->prev->columns_arr[j])
										break;
									temp = temp->next;
								}

								if (temp == NULL)
								{
									//if (the_debug == YES_DEBUG) printf("Reordering prev's col at j = %d\n", j);
									reorderCols(the_select->prev->columns_arr[j], cur_group->ptr_value, left, right);
								}
							}
						}

						left = i;
						right = i;
					}
				}

				if (left < right)
				{
					//printf("C: %d - %d\n", left, right);
					mergeSort(((struct col_in_select_node*) cur_group->ptr_value)->col_data_arr, ((struct col_in_select_node*) cur_group->ptr_value)->rows_data_type
							,ORDER_BY_ASC, left, right);

					struct ListNodePtr* temp = cur_group->next;
					while (temp != NULL)
					{
						reorderCols(temp->ptr_value, cur_group->ptr_value, left, right);
						temp = temp->next;
					}

					for (int j=0; j<the_select->prev->columns_arr_size; j++)
					{
						struct ListNodePtr* temp = the_func_node->group_by_cols_head;
						while (temp != NULL)
						{
							if (temp->ptr_value == the_select->prev->columns_arr[j])
								break;
							temp = temp->next;
						}

						if (temp == NULL)
						{
							//if (the_debug == YES_DEBUG) printf("Reordering prev's col at j = %d\n", j);
							reorderCols(the_select->prev->columns_arr[j], cur_group->ptr_value, left, right);
						}
					}
				}

				cur_group = cur_group->next;
			}

			cur_group = the_func_node->group_by_cols_head;
			while (cur_group != NULL)
			{
				if (the_debug == YES_DEBUG) 
					printf("Redoing row ids of one col\n");
				for (int i=0; i<((struct col_in_select_node*) cur_group->ptr_value)->num_rows; i++)
				{
					((struct col_in_select_node*) cur_group->ptr_value)->col_data_arr[i]->row_id = i;
				}

				cur_group = cur_group->next;
			}
		// ENDs Sort and reorder cols
			
		// START Find duplicate rows
			// START Insert row 0 into groups list
				struct group_data_node* group_data_node = (struct group_data_node*) myMalloc(sizeof(struct group_data_node), NULL, malloced_head, the_debug);
				if (group_data_node == NULL)
				{
					if (the_debug == YES_DEBUG)
						printf("	ERROR in getAggFuncGroups() at line %d in %s\n", __LINE__, __FILE__);
					return RETURN_ERROR;
				}
				group_data_node->row_ids_head = NULL;
				group_data_node->row_ids_tail = NULL;

				if (addListNodePtr(head, tail, group_data_node, PTR_TYPE_GROUP_DATE_NODE, ADDLISTNODE_TAIL, NULL, malloced_head, the_debug) != 0)
				{
					if (the_debug == YES_DEBUG)
						printf("	ERROR in getAggFuncGroups() at line %d in %s\n", __LINE__, __FILE__);
					return RETURN_ERROR;
				}

				if (addListNodePtr_Int(&group_data_node->row_ids_head, &group_data_node->row_ids_tail, 0, ADDLISTNODE_TAIL, NULL, malloced_head, the_debug) != 0)
				{
					if (the_debug == YES_DEBUG)
						printf("	ERROR in getAggFuncGroups() at line %d in %s\n", __LINE__, __FILE__);
					return RETURN_ERROR;
				}
			// END Insert row 0 into groups list

			for (int i=1; i<rows_before; i++)
			{
				struct ListNodePtr* temp = the_func_node->group_by_cols_head;
				while (temp != NULL)
				{
					if (!equals(((struct col_in_select_node*) temp->ptr_value)->col_data_arr[i]->row_data, ((struct col_in_select_node*) temp->ptr_value)->rows_data_type
								,((struct col_in_select_node*) temp->ptr_value)->col_data_arr[i-1]->row_data, VALUE_EQUALS))
					{
						break;
					}

					temp = temp->next;
				}

				if (temp == NULL)
				{
					//if (the_debug == YES_DEBUG) printf("		Row %d is a DUPLICATE\n", i);

					struct ListNodePtr* cur = *tail;
					if (addListNodePtr_Int(&((struct group_data_node*) cur->ptr_value)->row_ids_head, &((struct group_data_node*) cur->ptr_value)->row_ids_tail, i, ADDLISTNODE_TAIL, NULL, malloced_head, the_debug) != 0)
					{
						if (the_debug == YES_DEBUG)
							printf("	ERROR in getAggFuncGroups() at line %d in %s\n", __LINE__, __FILE__);
						return RETURN_ERROR;
					}

					(*num_rows_in_result)--;
				}
				else
				{
					//if (the_debug == YES_DEBUG) printf("	Row %d is a unique\n", i);

					struct group_data_node* group_data_node = (struct group_data_node*) myMalloc(sizeof(struct group_data_node), NULL, malloced_head, the_debug);
					if (group_data_node == NULL)
					{
						if (the_debug == YES_DEBUG)
							printf("	ERROR in getAggFuncGroups() at line %d in %s\n", __LINE__, __FILE__);
						return RETURN_ERROR;
					}
					group_data_node->row_ids_head = NULL;
					group_data_node->row_ids_tail = NULL;

					if (addListNodePtr(head, tail, group_data_node, PTR_TYPE_GROUP_DATE_NODE, ADDLISTNODE_TAIL, NULL, malloced_head, the_debug) != 0)
					{
						if (the_debug == YES_DEBUG)
							printf("	ERROR in getAggFuncGroups() at line %d in %s\n", __LINE__, __FILE__);
						return RETURN_ERROR;
					}

					if (addListNodePtr_Int(&group_data_node->row_ids_head, &group_data_node->row_ids_tail, i, ADDLISTNODE_TAIL, NULL, malloced_head, the_debug) != 0)
					{
						if (the_debug == YES_DEBUG)
							printf("	ERROR in getAggFuncGroups() at line %d in %s\n", __LINE__, __FILE__);
						return RETURN_ERROR;
					}
				}
			}

			if (the_debug == YES_DEBUG && 1 == 0)
			{
				printf("NOW *num_rows_in_result = %lu\n", *num_rows_in_result);

				struct ListNodePtr* cur = *head;
				while (cur != NULL)
				{
					traverseListNodesPtr(&((struct group_data_node*) cur->ptr_value)->row_ids_head, NULL, TRAVERSELISTNODES_HEAD, "Same values: ");

					cur = cur->next;
				}
			}
		// END Find duplicate rows

		if (*num_rows_in_result < rows_before)
		{
			// START Free extra rows
				cur_group = the_func_node->group_by_cols_head;
				while (cur_group != NULL)
				{
					struct colDataNode** temp_new = (struct colDataNode**) myMalloc(sizeof(struct colDataNode*) * (*num_rows_in_result), NULL, malloced_head, the_debug);
					if (temp_new == NULL)
					{
						if (the_debug == YES_DEBUG)
							printf("	ERROR in getAggFuncGroups() at line %d in %s\n", __LINE__, __FILE__);
						return RETURN_ERROR;
					}

					struct ListNodePtr* cur_row = *head;
					for (int i=0; i<*num_rows_in_result; i++)
					{
						temp_new[i] = ((struct col_in_select_node*) cur_group->ptr_value)->col_data_arr[*((int*) ((struct group_data_node*) cur_row->ptr_value)->row_ids_head->ptr_value)];
						temp_new[i]->row_id = i;

						//printf("Transfering %d\n", *((int*) ((struct group_data_node*) cur_row->ptr_value)->row_ids_head->ptr_value));

						struct ListNodePtr* cur_row_id = ((struct group_data_node*) cur_row->ptr_value)->row_ids_head->next;
						while (cur_row_id != NULL)
						{
							//printf("Freeing %d\n", *((int*) cur_row_id->ptr_value));
							if (((struct col_in_select_node*) cur_group->ptr_value)->col_data_arr[*((int*) cur_row_id->ptr_value)]->row_data != NULL)
								myFree((void**) &((struct col_in_select_node*) cur_group->ptr_value)->col_data_arr[*((int*) cur_row_id->ptr_value)]->row_data, NULL, malloced_head, the_debug);
							myFree((void**) &((struct col_in_select_node*) cur_group->ptr_value)->col_data_arr[*((int*) cur_row_id->ptr_value)], NULL, malloced_head, the_debug);

							cur_row_id = cur_row_id->next;
						}

						cur_row = cur_row->next;
					}

					myFree((void**) &((struct col_in_select_node*) cur_group->ptr_value)->col_data_arr, NULL, malloced_head, the_debug);

					((struct col_in_select_node*) cur_group->ptr_value)->col_data_arr = temp_new;
					((struct col_in_select_node*) cur_group->ptr_value)->num_rows = *num_rows_in_result;

					cur_group = cur_group->next;
				}
			// END Free extra rows
		}
	}
	else //if (!agg_funcs && the_select != NULL)
	{
		*num_rows_in_result = the_select->columns_arr[0]->num_rows;

		int_8 rows_before = *num_rows_in_result;
		if (the_debug == YES_DEBUG)
			printf("rows_before in getDistinctRowsV2() = %lu\n", rows_before);

		/**/
		// START Sort and reorder cols
			mergeSort(the_select->columns_arr[0]->col_data_arr, the_select->columns_arr[0]->rows_data_type, ORDER_BY_ASC, 0, rows_before-1);

			for (int j=1; j<the_select->columns_arr_size; j++)
				reorderCols(the_select->columns_arr[j], the_select->columns_arr[j-1], 0, rows_before-1);

			
			for (int j=1; j<the_select->columns_arr_size; j++)
			{
				int left = 0;
				int right = 0;

				the_select->columns_arr[j]->col_data_arr[0]->row_id = 0;

				for (int i=1; i<the_select->columns_arr[j-1]->num_rows; i++)
				{
					the_select->columns_arr[j]->col_data_arr[i]->row_id = i;

					int jj=0;
					for (; jj<j; jj++)
					{
						if (!equals(the_select->columns_arr[jj]->col_data_arr[i]->row_data, the_select->columns_arr[jj]->rows_data_type
									,the_select->columns_arr[jj]->col_data_arr[i-1]->row_data, VALUE_EQUALS))
						{
							break;
						}
					}

					if (jj == j)
					{
						//printf("B at i = %d\n", i);
						right++;
					}
					//if (equals(the_select->columns_arr[j-1]->col_data_arr[i]->row_data, the_select->columns_arr[j-1]->rows_data_type
					//			,the_select->columns_arr[j-1]->col_data_arr[i-1]->row_data, VALUE_EQUALS))
					//{
						//printf("B at i = %d\n", i);
					//	right++;
					//}
					else
					{
						if (left < right)
						{
							//printf("C: %d - %d\n", left, right);
							mergeSort(the_select->columns_arr[j]->col_data_arr, the_select->columns_arr[j]->rows_data_type, ORDER_BY_ASC, left, right);

							for (int jj=j+1; jj<the_select->columns_arr_size; jj++)
								reorderCols(the_select->columns_arr[jj], the_select->columns_arr[j], left, right);
						}

						left = i;
						right = i;
					}
				}

				if (left < right)
				{
					//printf("C: %d - %d\n", left, right);
					mergeSort(the_select->columns_arr[j]->col_data_arr, the_select->columns_arr[j]->rows_data_type, ORDER_BY_ASC, left, right);

					for (int jj=j+1; jj<the_select->columns_arr_size; jj++)
						reorderCols(the_select->columns_arr[jj], the_select->columns_arr[j], left, right);
				}
			}

			/**/
			for (int j=0; j<the_select->columns_arr_size; j++)
			{
				for (int i=0; i<the_select->columns_arr[j]->num_rows; i++)
				{
					the_select->columns_arr[j]->col_data_arr[i]->row_id = i;
				}
			}

			/*
			// START Write column data
				for (int i=0; i<rows_before; i++)
				{
					for (int j=0; j<the_select->columns_arr_size; j++)
					{
						if (the_select->columns_arr[j]->col_data_arr[i]->row_data != NULL)
						{
							int the_data_type = the_select->columns_arr[j]->rows_data_type;

							if (the_data_type == DATA_INT)
								printf("%d", *((int*) the_select->columns_arr[j]->col_data_arr[i]->row_data));
							else if (the_data_type == DATA_REAL)
								printf("%lf", *((double*) the_select->columns_arr[j]->col_data_arr[i]->row_data));
							else
								printf("%s", the_select->columns_arr[j]->col_data_arr[i]->row_data);
						}

						printf(":%lu", the_select->columns_arr[j]->col_data_arr[i]->row_id);

						if (j < the_select->columns_arr_size-1)
							printf(",");
					}
					if (i < rows_before-1)
						printf("\n");
				}
			// END Write column data
				printf("\n");*/
		// END Sort and reorder cols

		/**/
		// START Find duplicate rows
			for (int i=1; i<rows_before; i++)
			{
				int j=0;
				for (; j<the_select->columns_arr_size; j++)
				{
					//if (the_debug == YES_DEBUG) printf("Comparing column j = %d\n", j);

					if (!equals(the_select->columns_arr[j]->col_data_arr[i]->row_data, the_select->columns_arr[j]->rows_data_type
								,the_select->columns_arr[j]->col_data_arr[i-1]->row_data, VALUE_EQUALS))
					{
						//if (the_debug == YES_DEBUG) printf("Row %d does not equal row %d\n", i, i-1);
						break;
					}
				}

				if (j == the_select->columns_arr_size)
				{
					//if (the_debug == YES_DEBUG) printf("		Row %d is a DUPLICATE\n", i);

					if (addListNodePtr_Int(head, tail, i, ADDLISTNODE_TAIL, NULL, malloced_head, the_debug) != 0)
					{
						if (the_debug == YES_DEBUG)
							printf("	ERROR in getDistinctRowsV2() at line %d in %s\n", __LINE__, __FILE__);
						return RETURN_ERROR;
					}

					(*num_rows_in_result) -= 1;
				}
			}

			if (the_debug == YES_DEBUG)
			{
				printf("NOW *num_rows_in_result = %lu\n", *num_rows_in_result);

				traverseListNodesPtr(head, tail, TRAVERSELISTNODES_HEAD, "DUPS: ");
			}
		// END Find duplicate rows

		if (*num_rows_in_result < rows_before)
		{
			// START Free extra rows
				for (int j=0; j<the_select->columns_arr_size; j++)
				{
					struct colDataNode** new_data_arr = myMalloc(sizeof(struct colDataNode*) * (*num_rows_in_result), NULL, malloced_head, the_debug);
					if (new_data_arr == NULL)
					{
						if (the_debug == YES_DEBUG)
							printf("	ERROR in getDistinctRowsV2() at line %d in %s\n", __LINE__, __FILE__);
						return RETURN_ERROR;
					}

					struct ListNodePtr* cur = *head;
					int new_index = 0;
					for (int i=0; i<rows_before; i++)
					{
						if (cur != NULL && i == *((int*) cur->ptr_value))
						{
							//if (the_debug == YES_DEBUG) printf("Freeing duplicate at %d\n", i);

							if (the_select->columns_arr[j]->col_data_arr[i]->row_data != NULL)
								myFree((void**) &the_select->columns_arr[j]->col_data_arr[i]->row_data, NULL, malloced_head, the_debug);
							myFree((void**) &the_select->columns_arr[j]->col_data_arr[i], NULL, malloced_head, the_debug);

							cur = cur->next;
						}
						else
						{
							new_data_arr[new_index] = the_select->columns_arr[j]->col_data_arr[i];
							new_data_arr[new_index]->row_id = new_index;
							new_index++;
						}
					}

					myFree((void**) &the_select->columns_arr[j]->col_data_arr, NULL, malloced_head, the_debug);

					the_select->columns_arr[j]->col_data_arr = new_data_arr;
					the_select->columns_arr[j]->num_rows = *num_rows_in_result;
				}
			// END Free extra rows
		}
	}


	return RETURN_GOOD;			
}

/*	RETURNS:
 *  RETURN_ERROR if error
 *	RETURN_GOOD if good
 *
 *	WRITES TO:
 *	head
 *	tail
 *	num_rows_in_result
 */
int getAggFuncGroups(struct ListNodePtr** head, struct ListNodePtr** tail, int_8* num_rows_in_result, struct func_node* the_func_node, struct select_node* the_select
					,struct malloced_node** malloced_head, int the_debug)
{
	if (the_func_node->group_by_cols_head != NULL)
	{
		if (getDistinctRowsV2(true, the_select, the_func_node, head, tail, num_rows_in_result, malloced_head, the_debug) != RETURN_GOOD)
		{
			if (the_debug == YES_DEBUG)
				printf("	ERROR in getAggFuncGroups() at line %d in %s\n", __LINE__, __FILE__);
			return RETURN_ERROR;
		}
	}
	else
	{
		struct group_data_node* group_data_node = (struct group_data_node*) myMalloc(sizeof(struct group_data_node), NULL, malloced_head, the_debug);
		if (group_data_node == NULL)
		{
			if (the_debug == YES_DEBUG)
				printf("	ERROR in getAggFuncGroups() at line %d in %s\n", __LINE__, __FILE__);
			return RETURN_ERROR;
		}
		group_data_node->row_ids_head = NULL;
		group_data_node->row_ids_tail = NULL;

		if (addListNodePtr(head, tail, group_data_node, PTR_TYPE_GROUP_DATE_NODE, ADDLISTNODE_TAIL, NULL, malloced_head, the_debug) != 0)
		{
			if (the_debug == YES_DEBUG)
				printf("	ERROR in getAggFuncGroups() at line %d in %s\n", __LINE__, __FILE__);
			return RETURN_ERROR;
		}

		for (int i=0; i<(*num_rows_in_result); i++)
		{
			if (addListNodePtr_Int(&group_data_node->row_ids_head, &group_data_node->row_ids_tail, i, ADDLISTNODE_TAIL, NULL, malloced_head, the_debug) != 0)
			{
				if (the_debug == YES_DEBUG)
					printf("	ERROR in getAggFuncGroups() at line %d in %s\n", __LINE__, __FILE__);
				return RETURN_ERROR;
			}
		}

		*num_rows_in_result = 1;
	}


	if (the_debug == YES_DEBUG && 1 == 0)
	{
		struct ListNodePtr* cur = *head;
		while (cur != NULL)
		{
			traverseListNodesPtr(&((struct group_data_node*) cur->ptr_value)->row_ids_head, &((struct group_data_node*) cur->ptr_value)->row_ids_tail, TRAVERSELISTNODES_HEAD, "Same row: ");

			cur = cur->next;
		}

		printf("num_rows_in_result after getting agg groups = %lu\n", *num_rows_in_result);
	}		


	return RETURN_GOOD;
}

int multi_evaluateAggFunc(struct thread_exec_agg_func* thread_info)
{
	if (thread_info->the_debug == YES_DEBUG)
		printf("In thread: %d-%d\n", thread_info->index_from, thread_info->index_to);

	struct ListNodePtr* cur_row = thread_info->cur_row_head;
	for (int i=thread_info->index_from; i<thread_info->index_to; i++)
	{
		thread_info->the_col->col_data_arr[i] = (struct colDataNode*) myMalloc(sizeof(struct colDataNode), NULL, &thread_info->malloced_head, thread_info->the_debug);
		if (thread_info->the_col->col_data_arr[i] == NULL)
		{
			if (thread_info->the_debug == YES_DEBUG)
				printf("	ERROR in execAggFunc() at line %d in %s\n", __LINE__, __FILE__);
			return RETURN_ERROR;
		}
		thread_info->the_col->col_data_arr[i]->row_id = i;


		struct func_node* the_func = thread_info->the_col->func_node;
		while (the_func->args_arr_type[0] == PTR_TYPE_FUNC_NODE)
		{
			//printf("Did this\n");
			the_func = the_func->args_arr[0];
		}


		if (calcResultOfFuncForOneRow(cur_row, the_func, thread_info->the_col, i, &thread_info->malloced_head, thread_info->the_debug) != RETURN_GOOD)
		{
			if (thread_info->the_debug == YES_DEBUG)
				printf("	ERROR in execAggFunc() at line %d in %s\n", __LINE__, __FILE__);
			return RETURN_ERROR;
		}


		cur_row = cur_row->next;
	}

	return RETURN_GOOD;
}

/*	RETURNS:
 *  RETURN_ERROR if error
 *	RETURN_GOOD if good
 *
 *	WRITES TO:
 *	the_col->col_data_arr
 */
int execAggFunc(struct ListNodePtr* head, struct ListNodePtr* tail, int_8 num_rows_in_result, struct col_in_select_node* the_col, struct select_node* the_select_node
				,struct malloced_node** malloced_head, int the_debug)
{
	the_col->num_rows = num_rows_in_result;


	// START Identify result data type
		if (the_col->func_node->which_func == FUNC_COUNT)
		{
			the_col->func_node->result_type = DATA_INT;
		}
		else if (the_col->func_node->which_func == FUNC_AVG)
		{
			the_col->func_node->result_type = DATA_REAL;
		}
		else if (the_col->func_node->which_func == FUNC_FIRST)
		{
			the_col->func_node->result_type = DATA_DATE;
		}
		else if (the_col->func_node->which_func == FUNC_LAST)
		{
			the_col->func_node->result_type = DATA_DATE;
		}
		else if (the_col->func_node->which_func == FUNC_MIN
				|| the_col->func_node->which_func == FUNC_MAX
				|| the_col->func_node->which_func == FUNC_MEDIAN
				|| the_col->func_node->which_func == FUNC_SUM)
		{
			if (the_col->func_node->args_arr_type[0] == PTR_TYPE_FUNC_NODE)
			{
				the_col->func_node->result_type = ((struct func_node*) the_col->func_node->args_arr[0])->result_type;
			}
			else
			{
				the_col->func_node->result_type = ((struct col_in_select_node*) the_col->func_node->args_arr[0])->rows_data_type;
			}
		}
	// END Identify result data type


	/**/
	// START Try multithreading
		the_col->col_data_arr = (struct colDataNode**) myMalloc(sizeof(struct colDataNode*) * the_col->num_rows, NULL, malloced_head, the_debug);
		if (the_col->col_data_arr == NULL)
		{
			if (the_debug == YES_DEBUG)
				printf("	ERROR in execAggFunc() at line %d in %s\n", __LINE__, __FILE__);
			return RETURN_ERROR;
		}

		struct thread_exec_agg_func* thread_arr[NUM_THREADS];
		int remainder = num_rows_in_result % NUM_THREADS;
		int total = 0;
		struct ListNodePtr* cur_row = head;
		for (int t=0; t<NUM_THREADS; t++)
		{
			int num_rows_per_thread = num_rows_in_result/NUM_THREADS;
			if (remainder > 0)
			{
				num_rows_per_thread++;
				remainder--;
			}

			thread_arr[t] = myMalloc(sizeof(struct thread_exec_agg_func), NULL, malloced_head, the_debug);
			if (thread_arr[t] == NULL)
			{
				if (the_debug == YES_DEBUG)
					printf("	ERROR in evaluateWhereTree() at line %d in %s\n", __LINE__, __FILE__);
				return NULL;
			}
			thread_arr[t]->cur_row_head = cur_row;
			thread_arr[t]->the_col = the_col;
			thread_arr[t]->index_from = total;
			thread_arr[t]->index_to = total + num_rows_per_thread;

			thread_arr[t]->malloced_head = NULL;
			thread_arr[t]->the_debug = the_debug;

			pthread_create(&thread_arr[t]->thread_id, NULL, multi_evaluateAggFunc, thread_arr[t]);

			total += num_rows_per_thread;
			for (int c=0; c<num_rows_per_thread; c++)
				cur_row = cur_row->next;
		}
		for (int t=0; t<NUM_THREADS; t++)
			pthread_join(thread_arr[t]->thread_id, NULL);

		for (int t=0; t<NUM_THREADS; t++)
		{
			// START Join malloc lists
				if (thread_arr[t]->malloced_head != NULL)
				{
					struct malloced_node* cur_malloc = thread_arr[t]->malloced_head;
					while (cur_malloc->next != NULL)
					{
						cur_malloc = cur_malloc->next;
					}

					cur_malloc->next = *malloced_head;
					(*malloced_head)->prev = cur_malloc;
					*malloced_head = thread_arr[t]->malloced_head;
				}
			// END Join malloc lists

			myFree((void**) &thread_arr[t], NULL, malloced_head, the_debug);
		}
	// END Try multithreading


	/*
	// START Make resulting col_data_arr (Normal)
		the_col->col_data_arr = (struct colDataNode**) myMalloc(sizeof(struct colDataNode*) * the_col->num_rows, NULL, malloced_head, the_debug);
		if (the_col->col_data_arr == NULL)
		{
			if (the_debug == YES_DEBUG)
				printf("	ERROR in execAggFunc() at line %d in %s\n", __LINE__, __FILE__);
			return RETURN_ERROR;
		}


		struct ListNodePtr* cur_row = head;
		for (int i=0; i<the_col->num_rows; i++)
		{
			the_col->col_data_arr[i] = (struct colDataNode*) myMalloc(sizeof(struct colDataNode), NULL, malloced_head, the_debug);
			if (the_col->col_data_arr[i] == NULL)
			{
				if (the_debug == YES_DEBUG)
					printf("	ERROR in execAggFunc() at line %d in %s\n", __LINE__, __FILE__);
				return RETURN_ERROR;
			}
			the_col->col_data_arr[i]->row_id = i;


			struct func_node* the_func = the_col->func_node;
			while (the_func->args_arr_type[0] == PTR_TYPE_FUNC_NODE)
			{
				//printf("Did this\n");
				the_func = the_func->args_arr[0];
			}


			if (calcResultOfFuncForOneRow(cur_row, the_func, the_col, i, malloced_head, the_debug) != RETURN_GOOD)
			{
				if (the_debug == YES_DEBUG)
					printf("	ERROR in execAggFunc() at line %d in %s\n", __LINE__, __FILE__);
				return RETURN_ERROR;
			}


			cur_row = cur_row->next;
		}
	// END Make resulting col_data_arr (Normal)*/


	return RETURN_GOOD;
}

int multi_evaluateMathNode(struct thread_exec_math* thread_info)
{
	if (thread_info->the_debug == YES_DEBUG)
		printf("In thread: %d-%d\n", thread_info->index_from, thread_info->index_to);

	for (int i=thread_info->index_from; i<thread_info->index_to; i++)
	{
		struct result_node* result = evaluateMathTreeV2(thread_info->head, thread_info->tail, thread_info->math_node, i, -1, &thread_info->malloced_head, thread_info->the_debug);

		if (result == NULL)
		{
			if (thread_info->the_debug == YES_DEBUG)
				printf("	ERROR in multi_evaluateMathNode() at line %d in %s\n", __LINE__, __FILE__);
			return RETURN_ERROR;
		}

		thread_info->the_col->col_data_arr[i] = (struct colDataNode*) myMalloc(sizeof(struct colDataNode), NULL, &thread_info->malloced_head, thread_info->the_debug);
		if (thread_info->the_col->col_data_arr[i] == NULL)
		{
			if (thread_info->the_debug == YES_DEBUG)
				printf("	ERROR in multi_evaluateMathNode() at line %d in %s\n", __LINE__, __FILE__);
			return RETURN_ERROR;
		}
		thread_info->the_col->col_data_arr[i]->row_id = i;

		thread_info->the_col->col_data_arr[i]->row_data = result->result;
		thread_info->the_col->rows_data_type = result->result_type;

		myFree((void**) &result, NULL, &thread_info->malloced_head, thread_info->the_debug);
	}

	return RETURN_GOOD;
}

/*	RETURNS:
 *  RETURN_ERROR if error
 *	RETURN_GOOD if good
 *
 *	WRITES TO:
 *	the_col->col_data_arr
 */
int execMathNode(struct ListNodePtr* head, struct ListNodePtr* tail, struct col_in_select_node* the_col, struct select_node* the_select_node, int_8 num_rows_in_result
				,struct malloced_node** malloced_head, int the_debug)
{
	the_col->col_data_arr = (struct colDataNode**) myMalloc(sizeof(struct colDataNode*) * num_rows_in_result, NULL, malloced_head, the_debug);
	if (the_col->col_data_arr == NULL)
	{
		if (the_debug == YES_DEBUG)
			printf("	ERROR in execMathNode() at line %d in %s\n", __LINE__, __FILE__);
		return RETURN_ERROR;
	}

	the_col->num_rows = num_rows_in_result;


	// START Try multithreading
		struct thread_exec_math* thread_arr[NUM_THREADS];
		int remainder = num_rows_in_result % NUM_THREADS;
		int total = 0;
		for (int t=0; t<NUM_THREADS; t++)
		{
			int num_rows_per_thread = num_rows_in_result/NUM_THREADS;
			if (remainder > 0)
			{
				num_rows_per_thread++;
				remainder--;
			}

			thread_arr[t] = myMalloc(sizeof(struct thread_exec_math), NULL, malloced_head, the_debug);
			if (thread_arr[t] == NULL)
			{
				if (the_debug == YES_DEBUG)
					printf("	ERROR in evaluateWhereTree() at line %d in %s\n", __LINE__, __FILE__);
				return NULL;
			}
			thread_arr[t]->math_node = the_col->math_node;
			thread_arr[t]->the_col = the_col;
			thread_arr[t]->index_from = total;
			thread_arr[t]->index_to = total + num_rows_per_thread;

			thread_arr[t]->head = head;
			thread_arr[t]->tail = tail;

			thread_arr[t]->malloced_head = NULL;
			thread_arr[t]->the_debug = the_debug;

			pthread_create(&thread_arr[t]->thread_id, NULL, multi_evaluateMathNode, thread_arr[t]);

			total += num_rows_per_thread;
		}
		for (int t=0; t<NUM_THREADS; t++)
			pthread_join(thread_arr[t]->thread_id, NULL);

		for (int t=0; t<NUM_THREADS; t++)
		{
			// START Join malloc lists
				if (thread_arr[t]->malloced_head != NULL)
				{
					struct malloced_node* cur_malloc = thread_arr[t]->malloced_head;
					while (cur_malloc->next != NULL)
					{
						cur_malloc = cur_malloc->next;
					}

					cur_malloc->next = *malloced_head;
					(*malloced_head)->prev = cur_malloc;
					*malloced_head = thread_arr[t]->malloced_head;
				}
			// END Join malloc lists

			myFree((void**) &thread_arr[t], NULL, malloced_head, the_debug);
		}

		/*for (int i=0; i<the_col->num_rows; i++)
		{
			if (addListNodePtr(&the_col->unique_values_head, &the_col->unique_values_tail
							  ,the_col->col_data_arr[i]->row_data, the_col->rows_data_type, ADDLISTNODE_TAIL
							  ,NULL, malloced_head, the_debug) != 0)
			{
				if (the_debug == YES_DEBUG)
					printf("	ERROR in execAggFunc() at line %d in %s\n", __LINE__, __FILE__);
				return RETURN_ERROR;
			}
		}*/
	// END Try multithreading

	/*
	// START Normal
		int result = 1;
		for (int i=0; i<num_rows_in_result; i++)
		{
			if (result == 1)
			{
				//printf("Calling evaluateMathTree()\n");
				result = evaluateMathTree(head, tail, the_col->math_node, i, -1, malloced_head, the_debug);
				if (result < 0)
				{
					if (the_debug == YES_DEBUG)
						printf("	ERROR in execMathNode() at line %d in %s\n", __LINE__, __FILE__);
					return RETURN_ERROR;
				}
				//if (the_col->math_node->result == NULL)
				//	printf("Math result = %s\n", the_col->math_node->result);
				//else if (the_col->math_node->result_type == PTR_TYPE_INT)
				//	printf("Math result = %d\n", the_col->math_node->result);
				//else
				//	printf("Math result = %lf\n", the_col->math_node->result);
			}
				

			the_col->col_data_arr[i] = (struct colDataNode*) myMalloc(sizeof(struct colDataNode), NULL, malloced_head, the_debug);
			if (the_col->col_data_arr[i] == NULL)
			{
				if (the_debug == YES_DEBUG)
					printf("	ERROR in execMathNode() at line %d in %s\n", __LINE__, __FILE__);
				return RETURN_ERROR;
			}
			the_col->col_data_arr[i]->row_id = i;


			// START See if result is a unique value, adding to the_col->unique_values_head if so
				if (initNewColDataNode(the_col, i, the_col->math_node->result, the_col->math_node->result_type, 0, malloced_head, the_debug) != RETURN_GOOD)
				{
					if (the_debug == YES_DEBUG)
						printf("	ERROR in execAggFunc() at line %d in %s\n", __LINE__, __FILE__);
					return RETURN_ERROR;
				}
			// END See if result is a unique value, adding to the_col->unique_values_head if so

			
			if (result == 1 || (result == 0 && i == num_rows_in_result-1))
				myFree((void**) &the_col->math_node->result, NULL, malloced_head, the_debug);
		}

		the_col->rows_data_type = the_col->math_node->result_type;
	// END Normal*/

	return RETURN_GOOD;
}

/*	RETURNS:
 *  RETURN_ERROR if error
 *	RETURN_GOOD if good
 *
 *	WRITES TO:
 *	the_col->col_data_arr
 *	the_col->rows_data_type
 */
int execCaseNode(struct ListNodePtr* head, struct ListNodePtr* tail, struct select_node* the_select_node, struct col_in_select_node* the_col, int_8 num_rows_in_result
				,struct malloced_node** malloced_head, int the_debug)
{
	the_col->col_data_arr = (struct colDataNode**) myMalloc(sizeof(struct colDataNode*) * num_rows_in_result, NULL, malloced_head, the_debug);
	if (the_col->col_data_arr == NULL)
	{
		if (the_debug == YES_DEBUG)
			printf("	ERROR in execCaseNode() at line %d in %s\n", __LINE__, __FILE__);
		return RETURN_ERROR;
	}

	for (int i=0; i<num_rows_in_result; i++)
	{
		the_col->col_data_arr[i] = NULL;
	}

	the_col->num_rows = num_rows_in_result;


	struct ListNodePtr* cur_when = the_col->case_node->case_when_head;
	struct ListNodePtr* cur_then = the_col->case_node->case_then_value_head;
	while (cur_when != NULL)
	{
		struct ListNodePtr* same_row_id_list_head = NULL;
		struct ListNodePtr* same_row_id_list_tail = NULL;

		int_8 num_rows_in_result = 0;

		if (findValidRowsGivenWhere(&same_row_id_list_head, &same_row_id_list_tail
								   ,the_select_node, NULL, cur_when->ptr_value
								   ,&num_rows_in_result, head, tail, false, -1, -1, malloced_head, the_debug) != RETURN_GOOD)
		{
			if (the_debug == YES_DEBUG)
				printf("	ERROR in execCaseNode() at line %d in %s\n", __LINE__, __FILE__);
			return RETURN_ERROR;
		}

		int math_result = 0;
		struct ListNodePtr* cur_row_id = same_row_id_list_head;
		while (cur_row_id != NULL)
		{
			if (the_col->col_data_arr[*((int*) cur_row_id->ptr_value)] == NULL)
			{
				the_col->col_data_arr[*((int*) cur_row_id->ptr_value)] = (struct colDataNode*) myMalloc(sizeof(struct colDataNode), NULL, malloced_head, the_debug);
				if (the_col->col_data_arr[*((int*) cur_row_id->ptr_value)] == NULL)
				{
					if (the_debug == YES_DEBUG)
						printf("	ERROR in execCaseNode() at line %d in %s\n", __LINE__, __FILE__);
					return RETURN_ERROR;
				}

				the_col->col_data_arr[*((int*) cur_row_id->ptr_value)]->row_id = *((int*) cur_row_id->ptr_value);

				if (cur_then->ptr_type == PTR_TYPE_INT)
				{
					// START See if result is a unique value, adding to the_col->unique_values_head if so
						if (initNewColDataNode(the_col->col_data_arr, *((int*) cur_row_id->ptr_value), cur_then->ptr_value, cur_then->ptr_type, 0, malloced_head, the_debug) != RETURN_GOOD)
						{
							if (the_debug == YES_DEBUG)
								printf("	ERROR in execCaseNode() at line %d in %s\n", __LINE__, __FILE__);
							return RETURN_ERROR;
						}
					// END See if result is a unique value, adding to the_col->unique_values_head if so

					//printf("Giving row %d the data = %d\n", *((int*) cur_row_id->ptr_value), *((int*) cur_then->ptr_value));
				}
				else if (cur_then->ptr_type == PTR_TYPE_REAL)
				{
					// START See if result is a unique value, adding to the_col->unique_values_head if so
						if (initNewColDataNode(the_col->col_data_arr, *((int*) cur_row_id->ptr_value), cur_then->ptr_value, cur_then->ptr_type, 0, malloced_head, the_debug) != RETURN_GOOD)
						{
							if (the_debug == YES_DEBUG)
								printf("	ERROR in execCaseNode() at line %d in %s\n", __LINE__, __FILE__);
							return RETURN_ERROR;
						}
					// END See if result is a unique value, adding to the_col->unique_values_head if so

					//printf("Giving row %d the data = %lf\n", *((int*) cur_row_id->ptr_value), *((double*) cur_then->ptr_value));
				}
				else if (cur_then->ptr_type == PTR_TYPE_CHAR || cur_then->ptr_type == PTR_TYPE_DATE)
				{
					// START See if result is a unique value, adding to the_col->unique_values_head if so
						if (initNewColDataNode(the_col->col_data_arr, *((int*) cur_row_id->ptr_value), cur_then->ptr_value, cur_then->ptr_type, (strLength(cur_then->ptr_value)+1), malloced_head, the_debug) != RETURN_GOOD)
						{
							if (the_debug == YES_DEBUG)
								printf("	ERROR in execCaseNode() at line %d in %s\n", __LINE__, __FILE__);
							return RETURN_ERROR;
						}
					// END See if result is a unique value, adding to the_col->unique_values_head if so

					//printf("Giving row %d the data = _%s_\n", *((int*) cur_row_id->ptr_value), cur_then->ptr_value);
				}
				else if (cur_then->ptr_type == PTR_TYPE_COL_IN_SELECT_NODE)
				{
					if (initNewColDataNode(the_col->col_data_arr, *((int*) cur_row_id->ptr_value), ((struct col_in_select_node*) cur_then->ptr_value)->col_data_arr[*((int*) cur_row_id->ptr_value)]->row_data
											,PTR_TYPE_CHAR, strLength(((struct col_in_select_node*) cur_then->ptr_value)->col_data_arr[*((int*) cur_row_id->ptr_value)]->row_data)+1
											,malloced_head, the_debug) != RETURN_GOOD)
					{
						if (the_debug == YES_DEBUG)
							printf("	ERROR in execCaseNode() at line %d in %s\n", __LINE__, __FILE__);
						return RETURN_ERROR;
					}
				}
				else if (cur_then->ptr_type == PTR_TYPE_MATH_NODE)
				{
					if (math_result == 0)
					{
						if (evaluateMathTree(head, tail, cur_then->ptr_value, *((int*) cur_row_id->ptr_value), -1, malloced_head, the_debug) < 0)
						{
							if (the_debug == YES_DEBUG)
								printf("	ERROR in execCaseNode() at line %d in %s\n", __LINE__, __FILE__);
							return RETURN_ERROR;
						}
						math_result = 1;
					}

					// START See if result is a unique value, adding to the_col->unique_values_head if so
						if (initNewColDataNode(the_col->col_data_arr, *((int*) cur_row_id->ptr_value), ((struct math_node*) cur_then->ptr_value)->result, ((struct math_node*) cur_then->ptr_value)->result_type, 0, malloced_head, the_debug) != RETURN_GOOD)
						{
							if (the_debug == YES_DEBUG)
								printf("	ERROR in execCaseNode() at line %d in %s\n", __LINE__, __FILE__);
							return RETURN_ERROR;
						}
					// END See if result is a unique value, adding to the_col->unique_values_head if so

					//printf("Giving row %d the data = %lf\n", *((int*) cur_row_id->ptr_value), *((double*) ((struct math_node*) cur_then->ptr_value)->result));
				}
				else if (cur_then->ptr_type == PTR_TYPE_FUNC_NODE)
				{
					struct ListNodePtr* cur_row = head;
					while (cur_row != NULL)
					{
						struct ListNodePtr* cur_group_row_id = ((struct group_data_node*) cur_row->ptr_value)->row_ids_head;
						while (cur_group_row_id != NULL)
						{
							if (*((int*) cur_group_row_id->ptr_value) == *((int*) cur_row_id->ptr_value))
								break;
							cur_group_row_id = cur_group_row_id->next;
						}

						if (cur_group_row_id != NULL)
							break;

						cur_row = cur_row->next;
					}

					//printf("first cur_row row_id = %d\n", *((int*) ((struct group_data_node*) cur_row->ptr_value)->row_ids_head->ptr_value));
					//printf("*((int*) cur_row_id->ptr_value) = %lu\n", *((int*) cur_row_id->ptr_value));

					if (calcResultOfFuncForOneRow(cur_row, cur_then->ptr_value, the_col, *((int*) cur_row_id->ptr_value), malloced_head, the_debug) != RETURN_GOOD)
					{
						if (the_debug == YES_DEBUG)
							printf("	ERROR in execCaseNode() at line %d in %s\n", __LINE__, __FILE__);
						return RETURN_ERROR;
					}

					if (the_debug == YES_DEBUG)
						printf("row data = %d\n", *((int*) the_col->col_data_arr[*((int*) cur_row_id->ptr_value)]->row_data));
				}
				else if (cur_then->ptr_type == -1 && cur_then->ptr_value == NULL)
				{
					the_col->col_data_arr[*((int*) cur_row_id->ptr_value)]->row_data = NULL;
				}
			}

			cur_row_id = cur_row_id->next;
		}

		freeAnyLinkedList((void**) &same_row_id_list_head, PTR_TYPE_LIST_NODE_PTR, NULL, malloced_head, the_debug);

		if (cur_then->ptr_type == PTR_TYPE_MATH_NODE)
			myFree((void**) &((struct math_node*) cur_then->ptr_value)->result, NULL, malloced_head, the_debug);


		/*// START Free where's col ptrs unique_values_head
			if (cur_when->ptr_value != NULL)
			{
				void* ptr = NULL;
				int ptr_type = -1;

				struct where_clause_node* cur_mirror = myMalloc(sizeof(struct where_clause_node), NULL, malloced_head, the_debug);
				if (cur_mirror == NULL)
				{
					if (the_debug == YES_DEBUG)
						printf("	ERROR in preSelectAnalysis() at line %d in %s\n", __LINE__, __FILE__);
					return RETURN_ERROR;
				}
				initEmptyTreeNode((void**) &cur_mirror, NULL, PTR_TYPE_WHERE_CLAUSE_NODE);

				while (true) // Will always break, breaks when traversd == -1
				{
					int traversd = traverseTreeNode((void**) &(cur_when->ptr_value), PTR_TYPE_WHERE_CLAUSE_NODE, &ptr, &ptr_type, (void**) &cur_mirror
												   ,NULL, malloced_head, the_debug);
					if (traversd == -1)
					{
						if (the_debug == YES_DEBUG)
							printf("Natural break\n");
						break;
					}
					else if (traversd == -2)
					{
						if (the_debug == YES_DEBUG)
							printf("	ERROR in preSelectAnalysis() at line %d in %s\n", __LINE__, __FILE__);
						return RETURN_ERROR;
					}

					if (ptr_type == PTR_TYPE_COL_IN_SELECT_NODE && ((struct col_in_select_node*) ptr)->unique_values_head != NULL)
					{
						freeAnyLinkedList((void**) &((struct col_in_select_node*) ptr)->unique_values_head, PTR_TYPE_LIST_NODE_PTR, NULL, malloced_head, the_debug);
					}
				}
			}
		// END Free where's col ptrs unique_values_head*/

		cur_when = cur_when->next;
		cur_then = cur_then->next;
	}

	if (the_col->case_node->case_then_value_head->ptr_type == PTR_TYPE_MATH_NODE)
		the_col->rows_data_type = ((struct math_node*) the_col->case_node->case_then_value_head->ptr_value)->result_type;
	else if (the_col->case_node->case_then_value_head->ptr_type == PTR_TYPE_FUNC_NODE)
		the_col->rows_data_type = ((struct func_node*) the_col->case_node->case_then_value_head->ptr_value)->result_type;
	else
		the_col->rows_data_type = the_col->case_node->case_then_value_head->ptr_type;

	return RETURN_GOOD;
}

/*	RETURNS:
 *  RETURN_ERROR if error
 *	RETURN_GOOD if good
 *
 *	WRITES TO:
 *	new_node
 */
int initNewParentWhereNode(struct where_clause_node** new_node, struct where_clause_node* new_ptr_one, struct where_clause_node* new_ptr_two
						  ,struct malloced_node** malloced_head, int the_debug)
{
	(*new_node) = (struct where_clause_node*) myMalloc(sizeof(struct where_clause_node), NULL, malloced_head, the_debug);
	if (*new_node == NULL)
	{
		if (the_debug == YES_DEBUG)
			printf("	ERROR in redistributeWhereNodes() at line %d in %s\n", __LINE__, __FILE__);
		return RETURN_ERROR;
	}

	(*new_node)->ptr_one = new_ptr_one;
	((struct where_clause_node*) (*new_node)->ptr_one)->parent = *new_node;
	(*new_node)->ptr_one_type = PTR_TYPE_WHERE_CLAUSE_NODE;

	(*new_node)->ptr_two = new_ptr_two;
	((struct where_clause_node*) (*new_node)->ptr_two)->parent = *new_node;
	(*new_node)->ptr_two_type = PTR_TYPE_WHERE_CLAUSE_NODE;

	(*new_node)->where_type = WHERE_AND;

	(*new_node)->parent = NULL;

	return RETURN_GOOD;
}

/*	RETURNS:
 *  RETURN_ERROR if error
 *	RETURN_GOOD if good
 *
 *	WRITES TO:
 *	cur_temp
 *	cur_select
 */
int redistributeWhereNodes(struct select_node* cur_temp, struct select_node* cur_select, struct malloced_node** malloced_head, int the_debug)
{
	if (true) // both wheres point to the same table
	{
		struct where_clause_node* temp_where = cur_temp->where_head;
		struct where_clause_node* cur_where = cur_select->where_head;

		struct where_clause_node* new_node = NULL;
		if (initNewParentWhereNode(&new_node, temp_where, cur_where, malloced_head, the_debug) != RETURN_GOOD)
		{
			if (the_debug == YES_DEBUG)
				printf("	ERROR in redistributeWhereNodes() at line %d in %s\n", __LINE__, __FILE__);
			return RETURN_ERROR;
		}

		cur_temp->where_head = new_node;
	}

	return RETURN_GOOD;
}

/*	RETURNS:
 *  RETURN_ERROR if error
 *	RETURN_GOOD if good
 *
 *	WRITES TO:
 *	the_col
 */
int joinPrevCols(bool col_from_left, struct join_node* the_join_node, struct col_in_select_node* the_col, struct ListNodePtr* join_rows_head
				,int_8 new_num_rows, int_8 left_col_num_rows, int_8 right_col_num_rows, struct ListNodePtr* distinct_left_list_head, struct ListNodePtr* distinct_left_list_tail
				,struct ListNodePtr* distinct_right_list_head, struct ListNodePtr* distinct_right_list_tail
				,struct malloced_node** malloced_head, int the_debug)
{
	if (the_debug == YES_DEBUG)
	{
		if (col_from_left)
			printf("Joining left col\n");
		else
			printf("Joining right col\n");
	}

	struct colDataNode** temp_arr = myMalloc(sizeof(struct colDataNode*) * new_num_rows, NULL, malloced_head, the_debug);
	if (temp_arr == NULL)
	{
		if (the_debug == YES_DEBUG)
			printf("	ERROR in joinPrevCols() at line %d in %s\n", __LINE__, __FILE__);
		return RETURN_ERROR;
	}

	int matching_index = 0;

	// START Allocate matching rows
		struct ListNodePtr* rows_done_head = NULL;
		struct ListNodePtr* rows_done_tail = NULL;

		struct ListNodePtr* cur_row_id = join_rows_head;
		for (matching_index=0; cur_row_id != NULL; matching_index++)
		{
			int index = *((int*) cur_row_id->ptr_value);
			//printf("Doing index = %d\n", index);

			if (inListNodePtrList(&rows_done_head, &rows_done_tail, (void*) &index, PTR_TYPE_INT) == NULL)
			{
				if (addListNodePtr_Int(&rows_done_head, &rows_done_tail, index, ADDLISTNODE_TAIL, NULL, malloced_head, the_debug) != 0)
				{
					if (the_debug == YES_DEBUG)
						printf("	ERROR in joinPrevCols() at line %d in %s\n", __LINE__, __FILE__);
					return RETURN_ERROR;
				}

				//printf("Adding row from matching_index = %d for the first time\n", matching_index);
				temp_arr[matching_index] = the_col->col_data_arr[index];
			}
			else
			{
				// START Copy cuz duplicate
					//printf("Made a duplicate at matching_index = %d\n", matching_index);
					temp_arr[matching_index] = myMalloc(sizeof(struct colDataNode), NULL, malloced_head, the_debug);
					if (temp_arr[matching_index] == NULL)
					{
						if (the_debug == YES_DEBUG)
							printf("	ERROR in joinPrevCols() at line %d in %s\n", __LINE__, __FILE__);
						return RETURN_ERROR;
					}

					int char_length = the_col->rows_data_type == PTR_TYPE_CHAR ? strLength(the_col->col_data_arr[index]->row_data)+1 : -1;

					if (initNewColDataNode(temp_arr, matching_index, the_col->col_data_arr[index]->row_data, the_col->rows_data_type, char_length, malloced_head, the_debug) != RETURN_GOOD)
					{
						if (the_debug == YES_DEBUG)
							printf("	ERROR in joinPrevCols() at line %d in %s\n", __LINE__, __FILE__);
						return RETURN_ERROR;
					}
					//temp_arr[matching_index]->row_data = the_col->col_data_arr[index]->row_data;
				// END Copy cuz duplicate
			}

			temp_arr[matching_index]->row_id = matching_index;

			cur_row_id = cur_row_id->next;
		}

		if (rows_done_head != NULL)
			freeAnyLinkedList((void**) &rows_done_head, PTR_TYPE_LIST_NODE_PTR, NULL, malloced_head, the_debug);

		if (the_debug == YES_DEBUG)
			printf(" Allocated matching rows\n");
	// END Allocate matching rows

	if (the_join_node->join_type == JOIN_LEFT || the_join_node->join_type == JOIN_OUTER)
	{
		// START Keeping all rows from left, and making right rows NULL
			if (the_debug == YES_DEBUG)
				printf("  Keeping all rows from left, and making right rows NULL\n");
			for (int i=0; i<left_col_num_rows; i++)
			{
				if (inListNodePtrList(&distinct_left_list_head, &distinct_left_list_tail, (void*) &i, PTR_TYPE_INT) == NULL)
				{
					if (col_from_left)
					{
						temp_arr[matching_index] = the_col->col_data_arr[i];
					}
					else
					{
						temp_arr[matching_index] = myMalloc(sizeof(struct colDataNode), NULL, malloced_head, the_debug);
						if (temp_arr[matching_index] == NULL)
						{
							if (the_debug == YES_DEBUG)
								printf("	ERROR in joinPrevCols() at line %d in %s\n", __LINE__, __FILE__);
							return RETURN_ERROR;
						}

						temp_arr[matching_index]->row_data = NULL;
					}
					temp_arr[matching_index]->row_id = matching_index;

					matching_index++;
				}
			}
		// END Keeping all rows from left, and making right rows NULL
	}
	
	if (the_join_node->join_type == JOIN_RIGHT || the_join_node->join_type == JOIN_OUTER)
	{
		// START Keeping all rows from right, and making left rows NULL
			if (the_debug == YES_DEBUG)
				printf("  Keeping all rows from right, and making left rows NULL\n");
			for (int i=0; i<right_col_num_rows; i++)
			{
				if (inListNodePtrList(&distinct_right_list_head, &distinct_right_list_tail, (void*) &i, PTR_TYPE_INT) == NULL)
				{
					if (!col_from_left)
					{
						temp_arr[matching_index] = the_col->col_data_arr[i];
					}
					else
					{
						temp_arr[matching_index] = myMalloc(sizeof(struct colDataNode), NULL, malloced_head, the_debug);
						if (temp_arr[matching_index] == NULL)
						{
							if (the_debug == YES_DEBUG)
								printf("	ERROR in joinPrevCols() at line %d in %s\n", __LINE__, __FILE__);
							return RETURN_ERROR;
						}

						temp_arr[matching_index]->row_data = NULL;
					}
					temp_arr[matching_index]->row_id = matching_index;

					matching_index++;
				}
			}
		// END Keeping all rows from right, and making left rows NULL
	}
	
	if (col_from_left && (the_join_node->join_type == JOIN_INNER || the_join_node->join_type == JOIN_RIGHT))
	{
		// START Freeing extra rows from left col
			if (the_debug == YES_DEBUG)
				printf("   Freeing extra rows from left col: %d\n", left_col_num_rows);
			for (int i=0; i<left_col_num_rows; i++)
			{
				if (inListNodePtrList(&distinct_left_list_head, &distinct_left_list_tail, (void*) &i, PTR_TYPE_INT) == NULL)
				{
					if (the_col->col_data_arr[i]->row_data != NULL)
						myFree((void**) &the_col->col_data_arr[i]->row_data, NULL, malloced_head, the_debug);
					myFree((void**) &the_col->col_data_arr[i], NULL, malloced_head, the_debug);
				}
			}
		// END Freeing extra rows from left col
	}

	if (!col_from_left && (the_join_node->join_type == JOIN_INNER || the_join_node->join_type == JOIN_LEFT))
	{
		// START Freeing extra rows from right col
			if (the_debug == YES_DEBUG)
				printf("   Freeing extra rows from right col: %d\n", right_col_num_rows);
			for (int i=0; i<right_col_num_rows; i++)
			{
				if (inListNodePtrList(&distinct_right_list_head, &distinct_right_list_tail, (void*) &i, PTR_TYPE_INT) == NULL)
				{
					if (the_col->col_data_arr[i]->row_data != NULL)
						myFree((void**) &the_col->col_data_arr[i]->row_data, NULL, malloced_head, the_debug);
					myFree((void**) &the_col->col_data_arr[i], NULL, malloced_head, the_debug);
				}
			}
		// END Freeing extra rows from right col
	}

	myFree((void**) &the_col->col_data_arr, NULL, malloced_head, the_debug);

	the_col->col_data_arr = temp_arr;
	the_col->num_rows = new_num_rows;

	return RETURN_GOOD;
}

int joinPrevColsV2(bool col_from_left, struct join_node* the_join_node, struct col_in_select_node* the_col, struct ListNodePtr* join_rows_head
				  ,int_8 new_num_rows, int_8 left_col_num_rows, int_8 right_col_num_rows, bool* arr_left, bool* arr_right
				  ,struct malloced_node** malloced_head, int the_debug)
{
	if (the_debug == YES_DEBUG)
	{
		if (col_from_left)
			printf("Joining left col\n");
		else
			printf("Joining right col\n");
	}

	struct colDataNode** temp_arr = myMalloc(sizeof(struct colDataNode*) * new_num_rows, NULL, malloced_head, the_debug);
	if (temp_arr == NULL)
	{
		if (the_debug == YES_DEBUG)
			printf("	ERROR in joinPrevCols() at line %d in %s\n", __LINE__, __FILE__);
		return RETURN_ERROR;
	}

	int matching_index = 0;

	// START Allocate matching rows
		int_8 temp_num_rows;
		if (col_from_left)
			temp_num_rows = left_col_num_rows;
		else
			temp_num_rows = right_col_num_rows;

		if (the_debug == YES_DEBUG)
			printf("temp_num_rows = %lu\n", temp_num_rows);

		bool* arr_done = myMalloc(sizeof(bool) * temp_num_rows, NULL, malloced_head, the_debug);
		if (arr_done == NULL)
		{
			if (the_debug == YES_DEBUG)
				printf("	ERROR in joinPrevCols() at line %d in %s\n", __LINE__, __FILE__);
			return RETURN_ERROR;
		}
		for (int i=0; i<temp_num_rows; i++)
			arr_done[i] = false;

		struct ListNodePtr* cur_row_id = join_rows_head;
		for (matching_index=0; cur_row_id != NULL; matching_index++)
		{
			int index = *((int*) cur_row_id->ptr_value);
			//printf("Doing index = %d\n", index);

			if (!arr_done[index])
			{
				arr_done[index] = true;

				//printf("Adding row from matching_index = %d for the first time\n", matching_index);

				temp_arr[matching_index] = the_col->col_data_arr[index];
			}
			else
			{
				// START Copy cuz duplicate
					//printf("Made a duplicate at matching_index = %d\n", matching_index);
					temp_arr[matching_index] = myMalloc(sizeof(struct colDataNode), NULL, malloced_head, the_debug);
					if (temp_arr[matching_index] == NULL)
					{
						if (the_debug == YES_DEBUG)
							printf("	ERROR in joinPrevCols() at line %d in %s\n", __LINE__, __FILE__);
						return RETURN_ERROR;
					}

					int char_length = the_col->rows_data_type == PTR_TYPE_CHAR ? strLength(the_col->col_data_arr[index]->row_data)+1 : -1;

					if (initNewColDataNode(temp_arr, matching_index, the_col->col_data_arr[index]->row_data, the_col->rows_data_type, char_length, malloced_head, the_debug) != RETURN_GOOD)
					{
						if (the_debug == YES_DEBUG)
							printf("	ERROR in joinPrevCols() at line %d in %s\n", __LINE__, __FILE__);
						return RETURN_ERROR;
					}
					//temp_arr[matching_index]->row_data = the_col->col_data_arr[index]->row_data;
				// END Copy cuz duplicate
			}

			temp_arr[matching_index]->row_id = matching_index;

			cur_row_id = cur_row_id->next;
		}

		myFree((void**) &arr_done, NULL, malloced_head, the_debug);

		if (the_debug == YES_DEBUG)
			printf(" Allocated matching rows\n");
	// END Allocate matching rows

	if (the_join_node->join_type == JOIN_LEFT || the_join_node->join_type == JOIN_OUTER)
	{
		// START Keeping all rows from left, and making right rows NULL
			if (the_debug == YES_DEBUG)
				printf("  Keeping all rows from left, and making right rows NULL\n");
			for (int i=0; i<left_col_num_rows; i++)
			{
				if (!arr_left[i])
				{
					if (col_from_left)
					{
						temp_arr[matching_index] = the_col->col_data_arr[i];
					}
					else
					{
						temp_arr[matching_index] = myMalloc(sizeof(struct colDataNode), NULL, malloced_head, the_debug);
						if (temp_arr[matching_index] == NULL)
						{
							if (the_debug == YES_DEBUG)
								printf("	ERROR in joinPrevCols() at line %d in %s\n", __LINE__, __FILE__);
							return RETURN_ERROR;
						}

						temp_arr[matching_index]->row_data = NULL;
					}
					temp_arr[matching_index]->row_id = matching_index;

					matching_index++;
				}
			}
		// END Keeping all rows from left, and making right rows NULL
	}
	
	if (the_join_node->join_type == JOIN_RIGHT || the_join_node->join_type == JOIN_OUTER)
	{
		// START Keeping all rows from right, and making left rows NULL
			if (the_debug == YES_DEBUG)
				printf("  Keeping all rows from right, and making left rows NULL\n");
			for (int i=0; i<right_col_num_rows; i++)
			{
				if (!arr_right[i])
				{
					if (!col_from_left)
					{
						temp_arr[matching_index] = the_col->col_data_arr[i];
					}
					else
					{
						temp_arr[matching_index] = myMalloc(sizeof(struct colDataNode), NULL, malloced_head, the_debug);
						if (temp_arr[matching_index] == NULL)
						{
							if (the_debug == YES_DEBUG)
								printf("	ERROR in joinPrevCols() at line %d in %s\n", __LINE__, __FILE__);
							return RETURN_ERROR;
						}

						temp_arr[matching_index]->row_data = NULL;
					}
					temp_arr[matching_index]->row_id = matching_index;

					matching_index++;
				}
			}
		// END Keeping all rows from right, and making left rows NULL
	}
	
	if (col_from_left && (the_join_node->join_type == JOIN_INNER || the_join_node->join_type == JOIN_RIGHT))
	{
		// START Freeing extra rows from left col
			if (the_debug == YES_DEBUG)
				printf("   Freeing extra rows from left col: %d\n", left_col_num_rows);
			for (int i=0; i<left_col_num_rows; i++)
			{
				if (!arr_left[i])
				{
					if (the_col->col_data_arr[i]->row_data != NULL)
						myFree((void**) &the_col->col_data_arr[i]->row_data, NULL, malloced_head, the_debug);
					myFree((void**) &the_col->col_data_arr[i], NULL, malloced_head, the_debug);
				}
			}
		// END Freeing extra rows from left col
	}

	if (!col_from_left && (the_join_node->join_type == JOIN_INNER || the_join_node->join_type == JOIN_LEFT))
	{
		// START Freeing extra rows from right col
			if (the_debug == YES_DEBUG)
				printf("   Freeing extra rows from right col: %d\n", right_col_num_rows);
			for (int i=0; i<right_col_num_rows; i++)
			{
				if (!arr_right[i])
				{
					if (the_col->col_data_arr[i]->row_data != NULL)
						myFree((void**) &the_col->col_data_arr[i]->row_data, NULL, malloced_head, the_debug);
					myFree((void**) &the_col->col_data_arr[i], NULL, malloced_head, the_debug);
				}
			}
		// END Freeing extra rows from right col
	}

	myFree((void**) &the_col->col_data_arr, NULL, malloced_head, the_debug);

	the_col->col_data_arr = temp_arr;
	the_col->num_rows = new_num_rows;

	return RETURN_GOOD;
}

/*	RETURNS:
 *  RETURN_ERROR if error
 *	RETURN_GOOD if good
 *
 *	WRITES TO:
 *	the_select_node->columns_arr[j]
 */
int shrinkRowsCuzWhereOrHaving(int_8 num_rows_before_where, int_8 num_rows_after
							  ,struct select_node* the_select_node, struct ListNodePtr* valid_rows_head, struct ListNodePtr* valid_rows_tail
							  ,bool having, struct ListNodePtr* extra_cols_head, struct malloced_node** malloced_head, int the_debug)
{
	struct select_node* the_correct_table_ptr = the_select_node->prev;

	//printf("the_correct_table_ptr = %p\n", the_correct_table_ptr);

	if (num_rows_after == 0)
	{
		// START All rows eliminated by where
			for (int j=0; j<the_select_node->columns_arr_size; j++)
			{
				struct col_in_select_node* the_correct_col = the_select_node->columns_arr[j];
				if (the_select_node->columns_arr[j]->col_ptr != NULL)
					the_correct_col = the_select_node->columns_arr[j]->col_ptr;

				if (the_select_node->columns_arr[j]->func_node == NULL)
				{
					for (int i=0; i<num_rows_before_where; i++)
					{
						//printf("Freeing i = %d\n", i);
						if (the_correct_col->col_data_arr[i]->row_data != NULL)
							myFree((void**) &the_correct_col->col_data_arr[i]->row_data, NULL, malloced_head, the_debug);
						myFree((void**) &the_correct_col->col_data_arr[i], NULL, malloced_head, the_debug);
					}
					myFree((void**) &the_correct_col->col_data_arr, NULL, malloced_head, the_debug);
				}

				the_correct_col->num_rows = num_rows_after;
			}

			if (!having)
			{
				struct ListNodePtr* cur_extra = extra_cols_head;
				while (cur_extra != NULL)
				{
					//printf("cur_extra->ptr_value = %p\n", cur_extra->ptr_value);
					for (int j=0; j<the_correct_table_ptr->columns_arr_size; j++)
					{
						//printf("the_correct_table_ptr->columns_arr[%d] = %p\n", j, the_correct_table_ptr->columns_arr[j]);
						if (cur_extra->ptr_value == the_correct_table_ptr->columns_arr[j])
						{
							if (the_debug == YES_DEBUG)
								printf("Editing cur_extra at columns_arr index = %d\n", j);

							for (int i=0; i<num_rows_before_where; i++)
							{
								//printf("Freeing i = %d\n", i);
								if (((struct col_in_select_node*) cur_extra->ptr_value)->col_data_arr[i]->row_data != NULL)
									myFree((void**) &((struct col_in_select_node*) cur_extra->ptr_value)->col_data_arr[i]->row_data, NULL, malloced_head, the_debug);
								myFree((void**) &((struct col_in_select_node*) cur_extra->ptr_value)->col_data_arr[i], NULL, malloced_head, the_debug);
							}
							myFree((void**) &((struct col_in_select_node*) cur_extra->ptr_value)->col_data_arr, NULL, malloced_head, the_debug);

							((struct col_in_select_node*) cur_extra->ptr_value)->num_rows = num_rows_after;

							break;
						}
					}

					cur_extra = cur_extra->next;
				}
			}
		// END All rows eliminated by where
	}
	else if (num_rows_before_where > num_rows_after)
	{
		// START Some rows eliminated by where
			for (int j=0; j<the_select_node->columns_arr_size; j++)
			{
				struct col_in_select_node* the_correct_col = the_select_node->columns_arr[j];
				if (the_select_node->columns_arr[j]->col_ptr != NULL && !having)
					the_correct_col = the_select_node->columns_arr[j]->col_ptr;

				if (the_select_node->columns_arr[j]->func_node == NULL || having)
				{
					if (the_debug == YES_DEBUG)
						printf("Editing col at j = %d\n", j);
					struct colDataNode* new_arr[num_rows_after];

					int new_i = 0;
					for (int i=0; i<num_rows_before_where; i++)
					{
						if (inListNodePtrList(&valid_rows_head, &valid_rows_tail, &i, PTR_TYPE_INT) == NULL)
						{
							// START Row not in valid list, free
								//printf("		Freeing i = %d\n", i);
								if (the_correct_col->col_data_arr[i]->row_data != NULL)
									myFree((void**) &the_correct_col->col_data_arr[i]->row_data, NULL, malloced_head, the_debug);
								myFree((void**) &the_correct_col->col_data_arr[i], NULL, malloced_head, the_debug);
							// END Row not in valid list, free
						}
						else
						{
							//printf("		Temp duplicating i = %d\n", i);
							new_arr[new_i] = the_correct_col->col_data_arr[i];

							new_i++;
						}
					}

					myFree((void**) &the_correct_col->col_data_arr, NULL, malloced_head, the_debug);

					the_correct_col->col_data_arr = myMalloc(sizeof(struct colDataNode*) * num_rows_after, NULL, malloced_head, the_debug);
					if (the_correct_col->col_data_arr == NULL)
					{
						if (the_debug == YES_DEBUG)
							printf("	ERROR in shrinkRowsCuzWhereOrHaving() at line %d in %s\n", __LINE__, __FILE__);
						return RETURN_ERROR;
					}

					for (int i=0; i<num_rows_after; i++)
					{
						the_correct_col->col_data_arr[i] = new_arr[i];
						the_correct_col->col_data_arr[i]->row_id = i;
					}
				}

				the_correct_col->num_rows = num_rows_after;
			}

			if (!having)
			{
				struct ListNodePtr* cur_extra = extra_cols_head;
				while (cur_extra != NULL)
				{
					//printf("cur_extra->ptr_value = %p\n", cur_extra->ptr_value);
					for (int j=0; j<the_correct_table_ptr->columns_arr_size; j++)
					{
						//printf("the_correct_table_ptr->columns_arr[%d] = %p\n", j, the_correct_table_ptr->columns_arr[j]);
						if (cur_extra->ptr_value == the_correct_table_ptr->columns_arr[j])
						{
							if (the_debug == YES_DEBUG)
								printf("Editing cur_extra at columns_arr index = %d\n", j);
							struct colDataNode* new_arr[num_rows_after];

							int new_i = 0;
							for (int i=0; i<num_rows_before_where; i++)
							{
								if (inListNodePtrList(&valid_rows_head, &valid_rows_tail, &i, PTR_TYPE_INT) == NULL)
								{
									// START Row not in valid list, free
										//printf("		Freeing i = %d\n", i);
										if (((struct col_in_select_node*) cur_extra->ptr_value)->col_data_arr[i]->row_data != NULL)
											myFree((void**) &((struct col_in_select_node*) cur_extra->ptr_value)->col_data_arr[i]->row_data, NULL, malloced_head, the_debug);
										myFree((void**) &((struct col_in_select_node*) cur_extra->ptr_value)->col_data_arr[i], NULL, malloced_head, the_debug);
									// END Row not in valid list, free
								}
								else
								{
									//printf("		Temp duplicating i = %d\n", i);
									new_arr[new_i] = ((struct col_in_select_node*) cur_extra->ptr_value)->col_data_arr[i];

									new_i++;
								}
							}

							myFree((void**) &((struct col_in_select_node*) cur_extra->ptr_value)->col_data_arr, NULL, malloced_head, the_debug);

							((struct col_in_select_node*) cur_extra->ptr_value)->col_data_arr = myMalloc(sizeof(struct colDataNode*) * num_rows_after, NULL, malloced_head, the_debug);
							if (((struct col_in_select_node*) cur_extra->ptr_value)->col_data_arr == NULL)
							{
								if (the_debug == YES_DEBUG)
									printf("	ERROR in shrinkRowsCuzWhereOrHaving() at line %d in %s\n", __LINE__, __FILE__);
								return RETURN_ERROR;
							}

							for (int i=0; i<num_rows_after; i++)
							{
								((struct col_in_select_node*) cur_extra->ptr_value)->col_data_arr[i] = new_arr[i];
								((struct col_in_select_node*) cur_extra->ptr_value)->col_data_arr[i]->row_id = i;
							}

							((struct col_in_select_node*) cur_extra->ptr_value)->num_rows = num_rows_after;

							break;
						}
					}

					cur_extra = cur_extra->next;
				}
			}
		// END Some rows eliminated by where
	}

	return RETURN_GOOD;
}

/*	RETURNS:
 *  RETURN_ERROR if error
 *	RETURN_GOOD if good
 *
 *	WRITES TO:
 *	cur_select
 *	extra_cols_head
 *	extra_cols_tail
 */
int preSelectAnalysis(struct select_node* cur_select, struct ListNodePtr** extra_cols_head, struct ListNodePtr** extra_cols_tail
					 ,struct malloced_node** malloced_head, int the_debug)
{
	struct join_node* cur_join = cur_select->join_head;

	if (the_debug == YES_DEBUG)
		printf("\n   Found select_node: Trying to elim columns\n");

	// START Eliminate unused columns from cur_select->prev and cur_join->select_joined
		bool cur_join_has_next = false;

		for (int a=0; a<2; a++)
		{
			if (cur_join_has_next)
			{
				cur_join_has_next = false;
				cur_join = cur_join->next;
			}

			struct select_node* select_ptr = cur_select->prev;
			if (a == 1)
			{
				if (cur_join == NULL)
					break;
				select_ptr = cur_join->select_joined;

				if (cur_join->next != NULL)
				{
					cur_join_has_next = true;
					a--;
				}

				if (the_debug == YES_DEBUG)
					printf("   Found join node\n");
			}

			struct col_in_select_node* a_func_col = NULL;

			for (int i=0; i<select_ptr->columns_arr_size; i++)
			{
				if (select_ptr->columns_arr[i]->func_node != NULL)
				{
					if (the_debug == YES_DEBUG)
						printf("The columns in prev do contain a func\n");
					a_func_col = select_ptr->columns_arr[i];
					break;
				}
			}

			// START For every col in the cur_select->prev (select_ptr)
				for (int i=0; i<select_ptr->columns_arr_size; i++)
				{
					// START Traverse cur_select cols to see if col in cur_select->prev is used there
						if (the_debug == YES_DEBUG)
							printf("Checking prev column at index = %d\n", i);
						bool found = false;

						// START Check current select cols
							for (int ii=0; ii<cur_select->columns_arr_size; ii++)
							{
								//printf("Checking THIS column at index = %d\n", ii);
								if (cur_select->columns_arr[ii]->col_ptr_type == PTR_TYPE_COL_IN_SELECT_NODE && cur_select->columns_arr[ii]->col_ptr == select_ptr->columns_arr[i])
								{
									if (the_debug == YES_DEBUG)
										printf("Found col reference in col_ptr\n");
									found = true;
									break;
								}
								else if (cur_select->columns_arr[ii]->func_node != NULL)
								{
									struct ListNodePtr* temp_head = NULL;
									struct ListNodePtr* temp_tail = NULL;

									if (getAllColsFromFuncNode(&temp_head, &temp_tail, cur_select->columns_arr[ii]->func_node, NULL, NULL, malloced_head, the_debug) != 0)
									{
										if (the_debug == YES_DEBUG)
											printf("	ERROR in preSelectAnalysis() at line %d in %s\n", __LINE__, __FILE__);
										return RETURN_ERROR;
									}

									struct ListNodePtr* temp_cur = temp_head;
									while (temp_cur != NULL)
									{
										if (!found && temp_cur->ptr_value == select_ptr->columns_arr[i])
										{
											if (the_debug == YES_DEBUG)
											{

												printf("Found col reference in func_node\n");
												printf("			Adding col i = %d to extra_cols_head\n", i);
											}

											if (addListNodePtr(extra_cols_head, extra_cols_tail, select_ptr->columns_arr[i], PTR_TYPE_COL_IN_SELECT_NODE, ADDLISTNODE_TAIL, NULL, malloced_head, the_debug) != 0)
											{
												if (the_debug == YES_DEBUG)
													printf("	ERROR in preSelectAnalysis() at line %d in %s\n", __LINE__, __FILE__);
												return RETURN_ERROR;
											}

											found = true;
											break;
										}

										temp_cur = temp_cur->next;
									}

									freeAnyLinkedList((void**) &temp_head, PTR_TYPE_LIST_NODE_PTR, NULL, malloced_head, the_debug);

									if (found)
										break;
								}
								else if (cur_select->columns_arr[ii]->math_node != NULL)
								{
									struct ListNodePtr* temp_head = NULL;
									struct ListNodePtr* temp_tail = NULL;

									if (getAllColsFromMathNode(&temp_head, &temp_tail, cur_select->columns_arr[ii]->math_node, NULL, NULL, malloced_head, the_debug) != 0)
									{
										if (the_debug == YES_DEBUG)
											printf("	ERROR in preSelectAnalysis() at line %d in %s\n", __LINE__, __FILE__);
										return RETURN_ERROR;
									}

									struct ListNodePtr* temp_cur = temp_head;
									while (temp_cur != NULL)
									{
										if (!found && temp_cur->ptr_value == select_ptr->columns_arr[i])
										{
											if (the_debug == YES_DEBUG)
											{
												printf("Found col reference in math_node\n");
												printf("			Adding col i = %d to extra_cols_head\n", i);
											}

											if (addListNodePtr(extra_cols_head, extra_cols_tail, select_ptr->columns_arr[i], PTR_TYPE_COL_IN_SELECT_NODE, ADDLISTNODE_TAIL, NULL, malloced_head, the_debug) != 0)
											{
												if (the_debug == YES_DEBUG)
													printf("	ERROR in preSelectAnalysis() at line %d in %s\n", __LINE__, __FILE__);
												return RETURN_ERROR;
											}

											found = true;
											break;
										}

										temp_cur = temp_cur->next;
									}

									freeAnyLinkedList((void**) &temp_head, PTR_TYPE_LIST_NODE_PTR, NULL, malloced_head, the_debug);

									if (found)
										break;
								}
								else if (cur_select->columns_arr[ii]->case_node != NULL)
								{
									struct ListNodePtr* temp_head = NULL;
									struct ListNodePtr* temp_tail = NULL;

									if (getAllColsFromCaseNode(&temp_head, &temp_tail, cur_select->columns_arr[ii]->case_node, NULL, NULL, malloced_head, the_debug) != 0)
									{
										if (the_debug == YES_DEBUG)
											printf("	ERROR in preSelectAnalysis() at line %d in %s\n", __LINE__, __FILE__);
										return RETURN_ERROR;
									}

									struct ListNodePtr* temp_cur = temp_head;
									while (temp_cur != NULL)
									{
										if (!found && temp_cur->ptr_value == select_ptr->columns_arr[i])
										{
											if (the_debug == YES_DEBUG)
											{
												printf("Found col reference in case node\n");
												printf("			Adding col i = %d to extra_cols_head\n", i);
											}

											if (addListNodePtr(extra_cols_head, extra_cols_tail, select_ptr->columns_arr[i], PTR_TYPE_COL_IN_SELECT_NODE, ADDLISTNODE_TAIL, NULL, malloced_head, the_debug) != 0)
											{
												if (the_debug == YES_DEBUG)
													printf("	ERROR in preSelectAnalysis() at line %d in %s\n", __LINE__, __FILE__);
												return RETURN_ERROR;
											}

											found = true;
											break;
										}

										temp_cur = temp_cur->next;
									}

									freeAnyLinkedList((void**) &temp_head, PTR_TYPE_LIST_NODE_PTR, NULL, malloced_head, the_debug);

									if (found)
										break;
								}
							}
						// END Check current select cols

						if (!found && cur_select->where_head != NULL)
						{
							// START Check cur_select where head
								if (the_debug == YES_DEBUG)
											printf("	Checking cur_select where_head\n");

								struct ListNodePtr* temp_head = NULL;
								struct ListNodePtr* temp_tail = NULL;

								if (getAllColsFromWhereNode(&temp_head, &temp_tail, cur_select->where_head, NULL, NULL, malloced_head, the_debug) != 0)
								{
									if (the_debug == YES_DEBUG)
										printf("	ERROR in preSelectAnalysis() at line %d in %s\n", __LINE__, __FILE__);
									return RETURN_ERROR;
								}

								struct ListNodePtr* temp_cur = temp_head;
								while (temp_cur != NULL)
								{
									if (!found && temp_cur->ptr_value == select_ptr->columns_arr[i])
									{
										if (the_debug == YES_DEBUG)
										{
											printf("Found col reference in cur_select where node\n");
											printf("			Adding col i = %d to extra_cols_head\n", i);
										}

										if (addListNodePtr(extra_cols_head, extra_cols_tail, select_ptr->columns_arr[i], PTR_TYPE_COL_IN_SELECT_NODE, ADDLISTNODE_TAIL, NULL, malloced_head, the_debug) != 0)
										{
											if (the_debug == YES_DEBUG)
												printf("	ERROR in preSelectAnalysis() at line %d in %s\n", __LINE__, __FILE__);
											return RETURN_ERROR;
										}

										found = true;
										break;
									}

									temp_cur = temp_cur->next;
								}

								freeAnyLinkedList((void**) &temp_head, PTR_TYPE_LIST_NODE_PTR, NULL, malloced_head, the_debug);
							// END Check cur_select where head
						}

						if (!found && select_ptr->where_head != NULL)
						{
							// START Check where head
								if (the_debug == YES_DEBUG)
									printf("	Checking select_ptr where_head\n");

								struct ListNodePtr* temp_head = NULL;
								struct ListNodePtr* temp_tail = NULL;

								if (getAllColsFromWhereNode(&temp_head, &temp_tail, select_ptr->where_head, NULL, NULL, malloced_head, the_debug) != 0)
								{
									if (the_debug == YES_DEBUG)
										printf("	ERROR in preSelectAnalysis() at line %d in %s\n", __LINE__, __FILE__);
									return RETURN_ERROR;
								}

								struct ListNodePtr* temp_cur = temp_head;
								while (temp_cur != NULL)
								{
									if (!found && temp_cur->ptr_value == select_ptr->columns_arr[i])
									{
										if (the_debug == YES_DEBUG)
										{
											printf("Found col reference in select_ptr where node\n");
											printf("			Adding col i = %d to extra_cols_head\n", i);
										}

										if (addListNodePtr(extra_cols_head, extra_cols_tail, select_ptr->columns_arr[i], PTR_TYPE_COL_IN_SELECT_NODE, ADDLISTNODE_TAIL, NULL, malloced_head, the_debug) != 0)
										{
											if (the_debug == YES_DEBUG)
												printf("	ERROR in preSelectAnalysis() at line %d in %s\n", __LINE__, __FILE__);
											return RETURN_ERROR;
										}

										found = true;
										break;
									}

									temp_cur = temp_cur->next;
								}

								freeAnyLinkedList((void**) &temp_head, PTR_TYPE_LIST_NODE_PTR, NULL, malloced_head, the_debug);
							// END Check where head
						}

						if (!found && cur_select->join_head != NULL)
						{
							// START Check on clause of join in cur_select
								if (the_debug == YES_DEBUG)
										printf("	Checking cur_select join_head\n");

								struct ListNodePtr* temp_head = NULL;
								struct ListNodePtr* temp_tail = NULL;

								if (getAllColsFromWhereNode(&temp_head, &temp_tail, cur_select->join_head->on_clause_head, NULL, NULL, malloced_head, the_debug) != 0)
								{
									if (the_debug == YES_DEBUG)
										printf("	ERROR in preSelectAnalysis() at line %d in %s\n", __LINE__, __FILE__);
									return RETURN_ERROR;
								}

								struct ListNodePtr* temp_cur = temp_head;
								while (temp_cur != NULL)
								{
									if (!found && temp_cur->ptr_value == select_ptr->columns_arr[i])
									{
										if (the_debug == YES_DEBUG)
										{
											printf("Found col reference in join on node of cur_select\n");
											printf("			Adding col i = %d to extra_cols_head\n", i);
										}

										if (addListNodePtr(extra_cols_head, extra_cols_tail, select_ptr->columns_arr[i], PTR_TYPE_COL_IN_SELECT_NODE, ADDLISTNODE_TAIL, NULL, malloced_head, the_debug) != 0)
										{
											if (the_debug == YES_DEBUG)
												printf("	ERROR in preSelectAnalysis() at line %d in %s\n", __LINE__, __FILE__);
											return RETURN_ERROR;
										}

										found = true;
										break;
									}

									temp_cur = temp_cur->next;
								}

								freeAnyLinkedList((void**) &temp_head, PTR_TYPE_LIST_NODE_PTR, NULL, malloced_head, the_debug);
							// END Check on clause of join in cur_select
						}

						if (!found && select_ptr->join_head != NULL)
						{
							// START Check on clause of join in select_ptr
								if (the_debug == YES_DEBUG)
									printf("	Checking select_ptr join_head\n");

								struct ListNodePtr* temp_head = NULL;
								struct ListNodePtr* temp_tail = NULL;

								if (getAllColsFromWhereNode(&temp_head, &temp_tail, select_ptr->join_head->on_clause_head, NULL, NULL, malloced_head, the_debug) != 0)
								{
									if (the_debug == YES_DEBUG)
										printf("	ERROR in preSelectAnalysis() at line %d in %s\n", __LINE__, __FILE__);
									return RETURN_ERROR;
								}

								struct ListNodePtr* temp_cur = temp_head;
								while (temp_cur != NULL)
								{
									if (!found && temp_cur->ptr_value == select_ptr->columns_arr[i])
									{
										if (the_debug == YES_DEBUG)
										{
											printf("Found col reference in join on node of select_ptr\n");
											printf("			Adding col i = %d to extra_cols_head\n", i);
										}

										if (addListNodePtr(extra_cols_head, extra_cols_tail, select_ptr->columns_arr[i], PTR_TYPE_COL_IN_SELECT_NODE, ADDLISTNODE_TAIL, NULL, malloced_head, the_debug) != 0)
										{
											if (the_debug == YES_DEBUG)
												printf("	ERROR in preSelectAnalysis() at line %d in %s\n", __LINE__, __FILE__);
											return RETURN_ERROR;
										}

										found = true;
										break;
									}

									temp_cur = temp_cur->next;
								}

								freeAnyLinkedList((void**) &temp_head, PTR_TYPE_LIST_NODE_PTR, NULL, malloced_head, the_debug);
							// END Check on clause of join in select_ptr
						}

						if (!found && select_ptr->order_by != NULL)
						{
							// START Check order by
								struct ListNodePtr* cur_order_col = select_ptr->order_by->order_by_cols_head;
								while (cur_order_col != NULL)
								{
									if (the_debug == YES_DEBUG)
										printf("Checking a cur_order_col\n");
									if (cur_order_col->ptr_value == select_ptr->columns_arr[i])
									{
										if (the_debug == YES_DEBUG)
										{
											printf("Found col reference in order by node\n");
											printf("			Adding col i = %d to extra_cols_head\n", i);
										}

										if (addListNodePtr(extra_cols_head, extra_cols_tail, select_ptr->columns_arr[i], PTR_TYPE_COL_IN_SELECT_NODE, ADDLISTNODE_TAIL, NULL, malloced_head, the_debug) != 0)
										{
											if (the_debug == YES_DEBUG)
												printf("	ERROR in preSelectAnalysis() at line %d in %s\n", __LINE__, __FILE__);
											return RETURN_ERROR;
										}

										found = true;
									}

									cur_order_col = cur_order_col->next;
								}	
							// END Check order by
						}

						if (!found && a_func_col != NULL)
						{
							// START Check group by list
								if (the_debug == YES_DEBUG)
									printf("Checking group by cols cuz prev has a func\n");
								struct ListNodePtr* cur_group = a_func_col->func_node->group_by_cols_head;
								while (cur_group != NULL)
								{
									if (cur_group->ptr_value == select_ptr->columns_arr[i]->col_ptr)
									{
										if (the_debug == YES_DEBUG)
										{
											printf("Found col reference in func's group by node\n");
											printf("			Adding col i = %d to extra_cols_head\n", i);
										}

										if (addListNodePtr(extra_cols_head, extra_cols_tail, select_ptr->columns_arr[i], PTR_TYPE_COL_IN_SELECT_NODE, ADDLISTNODE_TAIL, NULL, malloced_head, the_debug) != 0)
										{
											if (the_debug == YES_DEBUG)
												printf("	ERROR in preSelectAnalysis() at line %d in %s\n", __LINE__, __FILE__);
											return RETURN_ERROR;
										}

										found = true;
										break;
									}
									cur_group = cur_group->next;
								}
							// END Check group by list
						}

						if (!found)
						{
							if (the_debug == YES_DEBUG)
								printf("Col at index %d unused\n", i);
							freeAnyLinkedList((void**) &select_ptr->columns_arr[i], PTR_TYPE_COL_IN_SELECT_NODE, NULL, malloced_head, the_debug);
							//printf("Freed col node\n");

							select_ptr->columns_arr[i] = NULL;
							for (int temp_i=i; temp_i<select_ptr->columns_arr_size-1; temp_i++)
							{
								select_ptr->columns_arr[temp_i] = select_ptr->columns_arr[temp_i+1];
							}
							select_ptr->columns_arr[select_ptr->columns_arr_size-1] = NULL;
							select_ptr->columns_arr_size--;
							i--;

							if (select_ptr->columns_arr_size == 0)
								myFree((void**) &select_ptr->columns_arr, NULL, malloced_head, the_debug);
						}
					// END Traverse cur_select cols to see if used there
				}
			// END For every col in the cur_select->prev (select_ptr)
		}
	// END Eliminate unused columns from cur_select->prev and cur_join->select_joined

	if (cur_select->prev->prev != NULL)
	{
		if (the_debug == YES_DEBUG)
			printf("Calling preSelectAnalysis on cur_select->prev\n");

		if (preSelectAnalysis(cur_select->prev, extra_cols_head, extra_cols_tail, malloced_head, the_debug) != 0)
		{
			if (the_debug == YES_DEBUG)
				printf("	ERROR in selectStuff() at line %d in %s\n", __LINE__, __FILE__);
			return RETURN_ERROR;
		}
	}

	cur_join = cur_select->join_head;
	while (cur_join != NULL)
	{
		if (cur_join->select_joined->prev != NULL)
		{
			if (the_debug == YES_DEBUG)
				printf("Calling preSelectAnalysis on cur_join->select_joined->prev\n");

			if (preSelectAnalysis(cur_join->select_joined, extra_cols_head, extra_cols_tail, malloced_head, the_debug) != 0)
			{
				if (the_debug == YES_DEBUG)
					printf("	ERROR in selectStuff() at line %d in %s\n", __LINE__, __FILE__);
				return RETURN_ERROR;
			}
		}

		if (the_debug == YES_DEBUG)
			printf("Going to cur_join->next\n");

		cur_join = cur_join->next;
	}

	return RETURN_GOOD;
}

/*	RETURNS:
 *  RETURN_ERROR if error
 *	RETURN_GOOD if good
 *
 *	WRITES TO:
 *	select_node
 */
int selectStuff(struct select_node** select_node, bool join, struct malloced_node** malloced_head, int the_debug)
{
	struct ListNodePtr* extra_cols_head = NULL;
	struct ListNodePtr* extra_cols_tail = NULL;

	if (!join)
	{
		// START Go to top most node
			struct select_node* cur_select = *select_node;
			while (cur_select->next != NULL)
				cur_select = cur_select->next;
		// END Go to top most node

		if (the_debug == YES_DEBUG)
		{
			printf("\n");
			printf("---- STARTING ANALYSIS ----\n");
			printf("\n");
		}
		if (preSelectAnalysis(cur_select, &extra_cols_head, &extra_cols_tail, malloced_head, the_debug) != RETURN_GOOD)
		{
			if (the_debug == YES_DEBUG)
				printf("	ERROR in selectStuff() at line %d in %s\n", __LINE__, __FILE__);
			return RETURN_ERROR;
		}
		if (the_debug == YES_DEBUG)
		{
			printf("\n");
			printf("----- ENDING ANALYSIS -----\n");
			printf("\n");
		}
	}

	while (*select_node != NULL)
	{
		if (the_debug == YES_DEBUG)
			printf("   Doing select_node\n");
		/*for (int j=0; j<(*select_node)->columns_arr_size; j++)
		{
			(*select_node)->columns_arr[j]->unique_values_head = NULL;
			(*select_node)->columns_arr[j]->unique_values_tail = NULL;
		}*/

		if ((*select_node)->columns_arr != NULL && (*select_node)->columns_arr[0]->table_ptr_type == PTR_TYPE_TABLE_INFO)
		{
			struct ListNodePtr* valid_rows_head = NULL;
			struct ListNodePtr* valid_rows_tail = NULL;

			struct ListNodePtr* tab_cols_info_head = NULL;
			struct ListNodePtr* tab_cols_info_tail = NULL;

			(*select_node)->columns_arr[0]->num_rows = -1;

			// START Exec where
				if ((*select_node)->where_head != NULL && ((struct table_cols_info*) (*select_node)->columns_arr[0]->col_ptr)->frequent_arr_row_to_node != NULL)
				{
					if (the_debug == YES_DEBUG)
						printf("Base table does have where and base table has frequent lists\n");
					(*select_node)->columns_arr[0]->num_rows = 0;
					if (findValidRowsGivenWhere(&valid_rows_head, &valid_rows_tail
											   ,NULL, (*select_node)->columns_arr[0]->table_ptr, (*select_node)->where_head
											   ,&(*select_node)->columns_arr[0]->num_rows, NULL, NULL, false, -1, -1, malloced_head, the_debug) != RETURN_GOOD)
					{
						if (the_debug == YES_DEBUG)
							printf("	ERROR in selectStuff() at line %d in %s\n", __LINE__, __FILE__);
						return RETURN_ERROR;
					}

					for (int j=0; j<(*select_node)->columns_arr_size; j++)
						(*select_node)->columns_arr[j]->num_rows = (*select_node)->columns_arr[0]->num_rows;
				}
				else if ((*select_node)->where_head != NULL && ((struct table_cols_info*) (*select_node)->columns_arr[0]->col_ptr)->frequent_arr_row_to_node == NULL)
				{
					if (the_debug == YES_DEBUG)
						printf("Base table does have where and base table DOES NOT HAVE frequent lists\n");

					// START Get all column ptrs from where clause
						if (getAllColsFromWhereNode(&tab_cols_info_head, &tab_cols_info_tail, (*select_node)->where_head, NULL, NULL, malloced_head, the_debug) != RETURN_GOOD)
						{
							if (the_debug == YES_DEBUG)
								printf("	ERROR in deleteRows() at line %d in %s\n", __LINE__, __FILE__);
							return RETURN_ERROR;
						}

						bool check_for_dups_arr[((struct table_info*) (*select_node)->columns_arr[0]->table_ptr)->num_cols];
						for (int i=0; i<((struct table_info*) (*select_node)->columns_arr[0]->table_ptr)->num_cols; i++)
						{
							check_for_dups_arr[i] = false;
						}

						struct ListNodePtr* cur = tab_cols_info_head;
						while (cur != NULL)
						{
							if (!check_for_dups_arr[((struct table_cols_info*) cur->ptr_value)->col_number])
							{
								// START Only do getAllColData if not duplicate column
									if (the_debug == YES_DEBUG)
										printf("Getting a col data arr for a col in the where node\n");

									check_for_dups_arr[((struct table_cols_info*) cur->ptr_value)->col_number] = true;

									struct table_cols_info* temp_col_ptr = cur->ptr_value;

									cur->ptr_type = ((struct table_cols_info*) cur->ptr_value)->col_number;

									cur->ptr_value = (void*) getAllColData(((struct table_info*) (*select_node)->columns_arr[0]->table_ptr)->file_number, temp_col_ptr, NULL, -1, malloced_head, the_debug);
									if (cur->ptr_value == NULL)
									{
										if (the_debug == YES_DEBUG)
											printf("	ERROR in deleteRows() at line %d in %s\n", __LINE__, __FILE__);
										return RETURN_ERROR;
									}
								// END Only do getAllColData if not duplicate column
							}
							else
							{
								cur->ptr_value = NULL;
								cur->ptr_type = -1;
							}

							cur = cur->next;
						}

						(*select_node)->columns_arr[0]->num_rows = 0;
						if (findValidRowsGivenWhere(&valid_rows_head, &valid_rows_tail, NULL, (*select_node)->columns_arr[0]->table_ptr, (*select_node)->where_head
													,&(*select_node)->columns_arr[0]->num_rows, tab_cols_info_head, tab_cols_info_tail
													,false, -1, -1, malloced_head, the_debug) != RETURN_GOOD)
						{
							if (the_debug == YES_DEBUG)
								printf("	ERROR in deleteRows() at line %d in %s\n", __LINE__, __FILE__);
							return RETURN_ERROR;
						}

						if (the_debug == YES_DEBUG)
						{
							printf("Num rows = %lu\n", (*select_node)->columns_arr[0]->num_rows);
							traverseListNodesPtr(&valid_rows_head, NULL, TRAVERSELISTNODES_HEAD, "Matching rows: ");
						}
					// END Get all column ptrs from where clause
				}
			// END Exec where

			// START Go to disk and retrieve rows
				for (int j=0; j<(*select_node)->columns_arr_size; j++)
				{
					if (((struct table_cols_info*) (*select_node)->columns_arr[j]->col_ptr)->frequent_arr_row_to_node != NULL)
					{
						if (the_debug == YES_DEBUG)
							printf("Getting table data from frequent lists\n");

						if (getAllColDataFromFreq((*select_node)->columns_arr[j], valid_rows_head, (*select_node)->columns_arr[0]->num_rows, malloced_head, the_debug) != RETURN_GOOD)
						{
							if (the_debug == YES_DEBUG)
								printf("	ERROR in selectStuff() at line %d in %s\n", __LINE__, __FILE__);
							return RETURN_ERROR;
						}
					}
					else //if (((struct table_cols_info*) (*select_node)->columns_arr[j]->col_ptr)->frequent_arr_row_to_node == NULL)
					{
						if ((*select_node)->where_head == NULL)
							(*select_node)->columns_arr[j]->num_rows = ((struct table_cols_info*) (*select_node)->columns_arr[j]->col_ptr)->num_rows - ((struct table_cols_info*) (*select_node)->columns_arr[j]->col_ptr)->num_open;
						else
							(*select_node)->columns_arr[j]->num_rows = (*select_node)->columns_arr[0]->num_rows;


						/*struct ListNodePtr* cur = tab_cols_info_head;
						while (cur != NULL)
						{
							if (((struct table_cols_info*) (*select_node)->columns_arr[j]->col_ptr)->col_number == cur->ptr_type)
								break;

							cur = cur->next;
						}


						if (cur == NULL)
						{*/
							// START Get data from disk
								if (the_debug == YES_DEBUG)
									printf("Getting table data from disk\n");
							
								(*select_node)->columns_arr[j]->col_data_arr = getAllColData(((struct table_cols_info*) (*select_node)->columns_arr[j]->col_ptr)->home_table->file_number
																							,(*select_node)->columns_arr[j]->col_ptr, valid_rows_head, (*select_node)->columns_arr[0]->num_rows
																							,malloced_head, the_debug);
								if ((*select_node)->columns_arr[j]->col_data_arr == NULL)
								{
									if (the_debug == YES_DEBUG)
										printf("	ERROR in selectStuff() at line %d in %s\n", __LINE__, __FILE__);
									return RETURN_ERROR;
								}

								/*for (int i=0; i<(*select_node)->columns_arr[j]->num_rows; i++)
								{
									if (addListNodePtr(&(*select_node)->columns_arr[j]->unique_values_head, &(*select_node)->columns_arr[j]->unique_values_tail
													  ,(*select_node)->columns_arr[j]->col_data_arr[i]->row_data, (*select_node)->columns_arr[j]->rows_data_type, ADDLISTNODE_TAIL
													  ,NULL, malloced_head, the_debug) != 0)
									{
										if (the_debug == YES_DEBUG)
											printf("	ERROR in execAggFunc() at line %d in %s\n", __LINE__, __FILE__);
										return RETURN_ERROR;
									}
								}*/
							// END Get data from disk
						/*}
						else
						{
							// START Init col_data_arr with data used for where clause
								if (the_debug == YES_DEBUG)
									printf("Init col_data_arr with data used for where clause\n");

								(*select_node)->columns_arr[j]->col_data_arr = myMalloc(sizeof(struct colDataNode*) * (*select_node)->columns_arr[j]->num_rows, NULL, malloced_head, the_debug);
								if ((*select_node)->columns_arr[j]->col_data_arr == NULL)
								{
									if (the_debug == YES_DEBUG)
										printf("	ERROR in selectStuff() at line %d in %s\n", __LINE__, __FILE__);
									return RETURN_ERROR;
								}

								struct ListNodePtr* cur_row = valid_rows_head;
								for (int i=0; i<(*select_node)->columns_arr[j]->num_rows; i++)
								{
									//printf("Fetching row_num = %d\n", *((int*) cur_row->ptr_value));

									(*select_node)->columns_arr[j]->col_data_arr[i] = ((struct colDataNode**) cur->ptr_value)[*((int*) cur_row->ptr_value)];
									(*select_node)->columns_arr[j]->col_data_arr[i]->row_id = i;

									if (addListNodePtr(&(*select_node)->columns_arr[j]->unique_values_head, &(*select_node)->columns_arr[j]->unique_values_tail
													  ,(*select_node)->columns_arr[j]->col_data_arr[i]->row_data, (*select_node)->columns_arr[j]->rows_data_type, ADDLISTNODE_TAIL
													  ,NULL, malloced_head, the_debug) != 0)
									{
										if (the_debug == YES_DEBUG)
											printf("	ERROR in execAggFunc() at line %d in %s\n", __LINE__, __FILE__);
										return RETURN_ERROR;
									}

									cur_row = cur_row->next;
								}

								int_8 rows_before = ((struct table_cols_info*) (*select_node)->columns_arr[0]->col_ptr)->num_rows - ((struct table_cols_info*) (*select_node)->columns_arr[0]->col_ptr)->num_open;
								for (int i=0; i<rows_before; i++)
								{
									if (inListNodePtrList(&valid_rows_head, &valid_rows_tail, &i, PTR_TYPE_INT) == NULL)
									{
										if (((struct colDataNode**) cur->ptr_value)[i]->row_data != NULL)
											myFree((void**) &((struct colDataNode**) cur->ptr_value)[i]->row_data, NULL, malloced_head, the_debug);
										myFree((void**) &((struct colDataNode**) cur->ptr_value)[i], NULL, malloced_head, the_debug);
									}
								}

								myFree((void**) &cur->ptr_value, NULL, malloced_head, the_debug);
							// END Init col_data_arr with data used for where clause
						}*/
					}

					//if ((*select_node)->where_head == NULL)
					//	(*select_node)->columns_arr[j]->num_rows = ((struct table_cols_info*) (*select_node)->columns_arr[j]->col_ptr)->num_rows - ((struct table_cols_info*) (*select_node)->columns_arr[j]->col_ptr)->num_open;

					(*select_node)->columns_arr[j]->rows_data_type = ((struct table_cols_info*) (*select_node)->columns_arr[j]->col_ptr)->data_type;
				}
			// END Go to disk and retrieve rows

			// START Free stuff
				if (valid_rows_head != NULL)
					freeAnyLinkedList((void**) &valid_rows_head, PTR_TYPE_LIST_NODE_PTR, NULL, malloced_head, the_debug);

				while (tab_cols_info_head != NULL)
				{
					struct ListNodePtr* temp = tab_cols_info_head;

					tab_cols_info_head = tab_cols_info_head->next;

					if (temp->ptr_value != NULL)
					{
						if (the_debug == YES_DEBUG)
							printf("Freeing a col data arr for a col in the where node\n");

						int_8 rows_before = ((struct table_info*) (*select_node)->columns_arr[0]->table_ptr)->table_cols_head->num_rows - ((struct table_info*) (*select_node)->columns_arr[0]->table_ptr)->table_cols_head->num_open;

						for (int i=0; i<rows_before; i++)
						{
							if (((struct colDataNode**) temp->ptr_value)[i]->row_data != NULL)
								myFree((void**) &((struct colDataNode**) temp->ptr_value)[i]->row_data, NULL, malloced_head, the_debug);
							myFree((void**) &((struct colDataNode**) temp->ptr_value)[i], NULL, malloced_head, the_debug);
						}

						myFree((void**) &temp->ptr_value, NULL, malloced_head, the_debug);
					}

					myFree((void**) &temp, NULL, malloced_head, the_debug);
				}
			// END Free stuff
		}
		else if ((*select_node)->columns_arr != NULL)
		{
			if ((*select_node)->join_head != NULL)
			{
				// START Do join
					struct join_node* cur_join = (*select_node)->join_head;
					while (cur_join != NULL)
					{
						struct select_node* join_select = cur_join->select_joined;
						while (join_select->prev != NULL)
						{
							join_select = join_select->prev;
						}

						if (the_debug == YES_DEBUG)
							printf("	Found join_node\n");
						if (selectStuff(&join_select, true, malloced_head, the_debug) != RETURN_GOOD)
						{
							if (the_debug == YES_DEBUG)
								printf("	ERROR in selectStuff() at line %d in %s\n", __LINE__, __FILE__);
							return RETURN_ERROR;
						}
						if (the_debug == YES_DEBUG)
							printf("Back from selectStuff()\n");

						// START Find row id order by evaluating on_clause_head
							int_8 join_num_matching_rows = 0;

							if (the_debug == YES_DEBUG)
							{
								printf("left_num_rows = %lu\n", (*select_node)->prev->columns_arr[0]->num_rows);
								printf("right_num_rows = %lu\n", join_select->columns_arr[0]->num_rows);
							}

							struct ListNodePtr* left_join_rows_head = NULL;
							struct ListNodePtr* left_join_rows_tail = NULL;
							struct ListNodePtr* right_join_rows_head = NULL;
							struct ListNodePtr* right_join_rows_tail = NULL;

							//if (findValidRowsGivenWhere(NULL, NULL
							//						  ,join_select, (struct table_info*) *select_node, cur_join->on_clause_head
							//						  ,&join_num_matching_rows, NULL, NULL, true, (*select_node)->prev->columns_arr[0]->num_rows
							//						  ,join_select->columns_arr[0]->num_rows, malloced_head, the_debug) != RETURN_GOOD)
							if (findMatchingRowsOfJoin(cur_join->on_clause_head, (struct table_info*) *select_node, join_select
													  ,&left_join_rows_head, &left_join_rows_tail, &right_join_rows_head, &right_join_rows_tail
													  ,NULL, NULL, (*select_node)->prev->columns_arr[0]->num_rows, join_select->columns_arr[0]->num_rows, &join_num_matching_rows
													  ,malloced_head, the_debug) != RETURN_GOOD)
							{
								if (the_debug == YES_DEBUG)
									printf("	ERROR in selectStuff() at line %d in %s\n", __LINE__, __FILE__);
								return RETURN_ERROR;
							}

							/*
							struct ListNodePtr* left_join_rows_head = NULL;
							struct ListNodePtr* right_join_rows_head = NULL;

							for (int j=0; j<(*select_node)->prev->columns_arr_size; j++)
							{
								if ((*select_node)->prev->columns_arr[j]->join_matching_rows_head != NULL)
								{
									left_join_rows_head = (*select_node)->prev->columns_arr[j]->join_matching_rows_head;
									//traverseListNodesPtr(&(*select_node)->prev->columns_arr[j]->join_matching_rows_head, NULL, TRAVERSELISTNODES_HEAD, "Left rows: \n");
									break;
								}
							}

							if (left_join_rows_head == NULL)
							{
								struct join_node* cur_join_temp = (*select_node)->join_head;
								while (cur_join_temp != NULL && cur_join_temp->select_joined != join_select)
								{
									for (int jj=0; jj<cur_join_temp->select_joined->columns_arr_size; jj++)
									{
										if (cur_join_temp->select_joined->columns_arr[jj]->join_matching_rows_head != NULL)
										{
											left_join_rows_head = cur_join_temp->select_joined->columns_arr[jj]->join_matching_rows_head;
											break;
										}
									}

									if (left_join_rows_head != NULL)
										break;
									cur_join_temp = cur_join_temp->next;
								}
							}

							for (int j=0; j<join_select->columns_arr_size; j++)
							{
								if (join_select->columns_arr[j]->join_matching_rows_head != NULL)
								{
									right_join_rows_head = join_select->columns_arr[j]->join_matching_rows_head;
									//traverseListNodesPtr(&join_select->columns_arr[j]->join_matching_rows_head, NULL, TRAVERSELISTNODES_HEAD, "Right rows: \n");
									break;
								}
							}*/
							
							if (the_debug == YES_DEBUG)
								printf("join_num_matching_rows = %lu\n", join_num_matching_rows);

							if (left_join_rows_head == NULL)
							{
								if (the_debug == YES_DEBUG)
									printf("	ERROR in selectStuff() at line %d in %s\n", __LINE__, __FILE__);
								return RETURN_ERROR;
							}
							if (right_join_rows_head == NULL)
							{
								if (the_debug == YES_DEBUG)
									printf("	ERROR in selectStuff() at line %d in %s\n", __LINE__, __FILE__);
								return RETURN_ERROR;
							}
						// END Find row id order by evaluating on_clause_head

						// START Find count distinct rows from left and right which matched
							int_8 cnt_distinct_left = 0;
							int_8 cnt_distinct_right = 0;

							//struct ListNodePtr* distinct_left_list_head = NULL;
							//struct ListNodePtr* distinct_left_list_tail = NULL;
							//struct ListNodePtr* distinct_right_list_head = NULL;
							//struct ListNodePtr* distinct_right_list_tail = NULL;

							bool* arr_left = myMalloc(sizeof(bool) * (*select_node)->prev->columns_arr[0]->num_rows, NULL, malloced_head, the_debug);
							bool* arr_right = myMalloc(sizeof(bool) * join_select->columns_arr[0]->num_rows, NULL, malloced_head, the_debug);
							for (int i=0; i<(*select_node)->prev->columns_arr[0]->num_rows; i++)
								arr_left[i] = false;
							for (int i=0; i<join_select->columns_arr[0]->num_rows; i++)
								arr_right[i] = false;

							struct ListNodePtr* cur_row_id = left_join_rows_head;
							while (cur_row_id != NULL)
							{
								int left_index = *((int*) cur_row_id->ptr_value);

								if (!arr_left[left_index])
								{
									//if (addListNodePtr_Int(&distinct_left_list_head, &distinct_left_list_tail, left_index, ADDLISTNODE_TAIL
									//					  ,NULL, malloced_head, the_debug) != 0)
									//{
									//	if (the_debug == YES_DEBUG)
									//		printf("	ERROR in selectStuff() at line %d in %s\n", __LINE__, __FILE__);
									//	return RETURN_ERROR;
									//}

									arr_left[left_index] = true;

									cnt_distinct_left++;
								}

								cur_row_id = cur_row_id->next;
							}

							cur_row_id = right_join_rows_head;
							while (cur_row_id != NULL)
							{
								int right_index = *((int*) cur_row_id->ptr_value);

								if (!arr_right[right_index])
								{
									//if (addListNodePtr_Int(&distinct_right_list_head, &distinct_right_list_tail, right_index, ADDLISTNODE_TAIL
									//					  ,NULL, malloced_head, the_debug) != 0)
									//{
									//	if (the_debug == YES_DEBUG)
									//		printf("	ERROR in selectStuff() at line %d in %s\n", __LINE__, __FILE__);
									//	return RETURN_ERROR;
									//}

									arr_right[right_index] = true;

									cnt_distinct_right++;
								}

								cur_row_id = cur_row_id->next;
							}

							//myFree((void**) &arr_left, NULL, malloced_head, the_debug);
							//myFree((void**) &arr_right, NULL, malloced_head, the_debug);
						// END Find count distinct rows from left and right which matched

						// START Find rows from left, right, and new number of rows after join
							int_8 left_col_num_rows = (*select_node)->prev->columns_arr[0]->num_rows;
							int_8 right_col_num_rows = join_select->columns_arr[0]->num_rows;

							int_8 new_num_rows = -1;
							if (cur_join->join_type == JOIN_INNER || cur_join->join_type == JOIN_CROSS)
								new_num_rows = join_num_matching_rows;
							else if (cur_join->join_type == JOIN_LEFT)
								new_num_rows = left_col_num_rows - cnt_distinct_left + join_num_matching_rows;
							else if (cur_join->join_type == JOIN_RIGHT)
								new_num_rows = right_col_num_rows - cnt_distinct_right + join_num_matching_rows;
							else if (cur_join->join_type == JOIN_OUTER)
								new_num_rows = left_col_num_rows - cnt_distinct_left + join_num_matching_rows + (right_col_num_rows - cnt_distinct_right);

							if (the_debug == YES_DEBUG)
								printf("new_num_rows = %d\n", new_num_rows);
						// END Find rows from left, right, and new number of rows after join

						for (int j=0; j<(*select_node)->prev->columns_arr_size; j++)
						{
							//if (joinPrevCols(true, cur_join, (*select_node)->prev->columns_arr[j], left_join_rows_head, new_num_rows, left_col_num_rows, right_col_num_rows
							//				,distinct_left_list_head, distinct_left_list_tail, distinct_right_list_head, distinct_right_list_tail
							//				,malloced_head, the_debug) != RETURN_GOOD)
							if (joinPrevColsV2(true, cur_join, (*select_node)->prev->columns_arr[j], left_join_rows_head, new_num_rows, left_col_num_rows, right_col_num_rows
											  ,arr_left, arr_right
											  ,malloced_head, the_debug) != RETURN_GOOD)
							{
								if (the_debug == YES_DEBUG)
									printf("	ERROR in selectStuff() at line %d in %s\n", __LINE__, __FILE__);
								return RETURN_ERROR;
							}
						}

						for (int j=0; j<join_select->columns_arr_size; j++)
						{
							//if (joinPrevCols(false, cur_join, join_select->columns_arr[j], right_join_rows_head, new_num_rows, left_col_num_rows, right_col_num_rows
							//				,distinct_left_list_head, distinct_left_list_tail, distinct_right_list_head, distinct_right_list_tail
							//				,malloced_head, the_debug) != RETURN_GOOD)
							if (joinPrevColsV2(false, cur_join, join_select->columns_arr[j], right_join_rows_head, new_num_rows, left_col_num_rows, right_col_num_rows
											  ,arr_left, arr_right
											  ,malloced_head, the_debug) != RETURN_GOOD)
							{
								if (the_debug == YES_DEBUG)
									printf("	ERROR in selectStuff() at line %d in %s\n", __LINE__, __FILE__);
								return RETURN_ERROR;
							}
						}

						myFree((void**) &arr_left, NULL, malloced_head, the_debug);
						myFree((void**) &arr_right, NULL, malloced_head, the_debug);

						//if (distinct_left_list_head != NULL)
						//	freeAnyLinkedList((void**) &distinct_left_list_head, PTR_TYPE_LIST_NODE_PTR, NULL, malloced_head, the_debug);

						//if (distinct_right_list_head != NULL)
						//	freeAnyLinkedList((void**) &distinct_right_list_head, PTR_TYPE_LIST_NODE_PTR, NULL, malloced_head, the_debug);

						freeAnyLinkedList((void**) &left_join_rows_head, PTR_TYPE_LIST_NODE_PTR, NULL, malloced_head, the_debug);
						freeAnyLinkedList((void**) &right_join_rows_head, PTR_TYPE_LIST_NODE_PTR, NULL, malloced_head, the_debug);

						/*
						for (int j=0; j<(*select_node)->prev->columns_arr_size; j++)
						{
							if ((*select_node)->prev->columns_arr[j]->join_matching_rows_head != NULL)
							{
								freeAnyLinkedList((void**) &(*select_node)->prev->columns_arr[j]->join_matching_rows_head, PTR_TYPE_LIST_NODE_PTR, NULL, malloced_head, the_debug);
							}
						}

						struct join_node* cur_join_temp = (*select_node)->join_head;
						while (cur_join_temp != NULL && cur_join_temp->select_joined != join_select)
						{
							for (int jj=0; jj<cur_join_temp->select_joined->columns_arr_size; jj++)
							{
								if (cur_join_temp->select_joined->columns_arr[jj]->join_matching_rows_head != NULL)
								{
									freeAnyLinkedList((void**) &cur_join_temp->select_joined->columns_arr[jj]->join_matching_rows_head, PTR_TYPE_LIST_NODE_PTR, NULL, malloced_head, the_debug);
								}
							}

							cur_join_temp = cur_join_temp->next;
						}

						for (int j=0; j<join_select->columns_arr_size; j++)
						{
							if (join_select->columns_arr[j]->join_matching_rows_head != NULL)
							{
								freeAnyLinkedList((void**) &join_select->columns_arr[j]->join_matching_rows_head, PTR_TYPE_LIST_NODE_PTR, NULL, malloced_head, the_debug);
							}
						}*/

						cur_join = cur_join->next;
					}
				// END Do join
			}

			// START
				int_8 num_rows_in_result = (*select_node)->prev->columns_arr == NULL ? 1 : (*select_node)->prev->columns_arr[0]->num_rows;

				for (int j=0; j<(*select_node)->columns_arr_size; j++)
				{
					if ((*select_node)->columns_arr[j]->col_ptr_type == PTR_TYPE_COL_IN_SELECT_NODE)
					{
						(*select_node)->columns_arr[j]->col_data_arr = ((struct col_in_select_node*) (*select_node)->columns_arr[j]->col_ptr)->col_data_arr;
						(*select_node)->columns_arr[j]->num_rows = ((struct col_in_select_node*) (*select_node)->columns_arr[j]->col_ptr)->num_rows;
						(*select_node)->columns_arr[j]->rows_data_type = ((struct col_in_select_node*) (*select_node)->columns_arr[j]->col_ptr)->rows_data_type;
					}
					else if ((*select_node)->columns_arr[j]->math_node != NULL)
					{
						if (the_debug == YES_DEBUG)
							printf("Found math\n");
						if (execMathNode(NULL, NULL, (*select_node)->columns_arr[j], *select_node, num_rows_in_result, malloced_head, the_debug) != RETURN_GOOD)
						{
							if (the_debug == YES_DEBUG)
								printf("	ERROR in selectStuff() at line %d in %s\n", __LINE__, __FILE__);
							return RETURN_ERROR;
						}
					}
					else if ((*select_node)->columns_arr[j]->case_node != NULL)
					{
						if (the_debug == YES_DEBUG)
							printf("Found case\n");
						if (execCaseNode(NULL, NULL, *select_node, (*select_node)->columns_arr[j], num_rows_in_result, malloced_head, the_debug) != RETURN_GOOD)
						{
							if (the_debug == YES_DEBUG)
								printf("	ERROR in selectStuff() at line %d in %s\n", __LINE__, __FILE__);
							return RETURN_ERROR;
						}
						if (the_debug == YES_DEBUG)
							printf("Col datatype = %d\n", (*select_node)->columns_arr[j]->rows_data_type);
					}
				}

				(*select_node)->columns_arr[0]->num_rows = num_rows_in_result;
			// END

			if ((*select_node)->where_head != NULL)
			{
				// START Apply where to select_node col_data_arrs
					if (the_debug == YES_DEBUG)
						printf("Select node HAS where\n");

					struct ListNodePtr* valid_rows_head = NULL;
					struct ListNodePtr* valid_rows_tail = NULL;

					int_8 num_rows_before_where = num_rows_in_result;

					if (findValidRowsGivenWhere(&valid_rows_head, &valid_rows_tail
											   ,(*select_node), NULL, (*select_node)->where_head
											   ,&(*select_node)->columns_arr[0]->num_rows
											   ,NULL, NULL, false, -1, -1, malloced_head, the_debug) != RETURN_GOOD)
					{
						if (the_debug == YES_DEBUG)
							printf("	ERROR in selectStuff() at line %d in %s\n", __LINE__, __FILE__);
						return RETURN_ERROR;
					}

					
					if (the_debug == YES_DEBUG)
					{
						//traverseListNodesPtr(&valid_rows_head, &valid_rows_tail, TRAVERSELISTNODES_HEAD, "Valid rows: ");
						printf("Num rows after where = %d\n", (*select_node)->columns_arr[0]->num_rows);
						printf("num_rows_before_where = %d\n", num_rows_before_where);
					}

					if (shrinkRowsCuzWhereOrHaving(num_rows_before_where, (*select_node)->columns_arr[0]->num_rows
												  ,(*select_node), valid_rows_head, valid_rows_tail
												  ,false, extra_cols_head, malloced_head, the_debug) != RETURN_GOOD)
					{
						if (the_debug == YES_DEBUG)
							printf("	ERROR in selectStuff() at line %d in %s\n", __LINE__, __FILE__);
						return RETURN_ERROR;
					}

					if (valid_rows_head != NULL)
						freeAnyLinkedList((void**) &valid_rows_head, PTR_TYPE_LIST_NODE_PTR, NULL, malloced_head, the_debug);

					for (int j=0; j<(*select_node)->columns_arr_size; j++)
					{
						if ((*select_node)->columns_arr[j]->col_ptr_type == PTR_TYPE_COL_IN_SELECT_NODE)
						{
							(*select_node)->columns_arr[j]->col_data_arr = ((struct col_in_select_node*) (*select_node)->columns_arr[j]->col_ptr)->col_data_arr;
							(*select_node)->columns_arr[j]->num_rows = ((struct col_in_select_node*) (*select_node)->columns_arr[j]->col_ptr)->num_rows;
							(*select_node)->columns_arr[j]->rows_data_type = ((struct col_in_select_node*) (*select_node)->columns_arr[j]->col_ptr)->rows_data_type;
						}
					}
				// END Apply where to select_node->data_arr
			}

			// START Evaluate columns
				bool funcs = false;
				struct ListNodePtr* groups_head = NULL;
				struct ListNodePtr* groups_tail = NULL;
				/*int_8*/ num_rows_in_result = (*select_node)->prev->columns_arr == NULL ? 1 : (*select_node)->prev->columns_arr[0]->num_rows;

				for (int j=0; j<(*select_node)->columns_arr_size; j++)
				{
					// START See if func node exists anywhere
						if ((*select_node)->columns_arr[j]->func_node != NULL)
						{
							if (the_debug == YES_DEBUG)
								printf("Found func\n");
							funcs = true;
							if (getAggFuncGroups(&groups_head, &groups_tail, &num_rows_in_result, (*select_node)->columns_arr[j]->func_node, *select_node, malloced_head, the_debug) != RETURN_GOOD)
							{
								if (the_debug == YES_DEBUG)
									printf("	ERROR in selectStuff() at line %d in %s\n", __LINE__, __FILE__);
								return RETURN_ERROR;
							}
						}
						else if ((*select_node)->columns_arr[j]->math_node != NULL)
						{
							void* ptr = NULL;
							int ptr_type = -1;

							struct math_node* cur_mirror = myMalloc(sizeof(struct math_node), NULL, malloced_head, the_debug);
							if (cur_mirror == NULL)
							{
								if (the_debug == YES_DEBUG)
									printf("	ERROR in preSelectAnalysis() at line %d in %s\n", __LINE__, __FILE__);
								return RETURN_ERROR;
							}
							initEmptyTreeNode((void**) &cur_mirror, NULL, PTR_TYPE_MATH_NODE);

							while (true) // Will always break, breaks when traversd == -1
							{
								int traversd = traverseTreeNode((void**) &(*select_node)->columns_arr[j]->math_node, PTR_TYPE_MATH_NODE, &ptr, &ptr_type, (void**) &cur_mirror
															   ,NULL, malloced_head, the_debug);
								if (traversd == -1)
								{
									if (the_debug == YES_DEBUG)
										printf("Natural break\n");
									break;
								}
								else if (traversd == -2)
								{
									if (the_debug == YES_DEBUG)
										printf("	ERROR in preSelectAnalysis() at line %d in %s\n", __LINE__, __FILE__);
									return RETURN_ERROR;
								}

								if (!funcs && ptr_type == PTR_TYPE_FUNC_NODE)
								{
									if (the_debug == YES_DEBUG)
										printf("Found func\n");
									funcs = true;
									if (getAggFuncGroups(&groups_head, &groups_tail, &num_rows_in_result, ptr, *select_node, malloced_head, the_debug) != RETURN_GOOD)
									{
										if (the_debug == YES_DEBUG)
											printf("	ERROR in selectStuff() at line %d in %s\n", __LINE__, __FILE__);
										return RETURN_ERROR;
									}
								}
							}
						}
						else if ((*select_node)->columns_arr[j]->case_node != NULL)
						{
							struct ListNodePtr* cur_then = (*select_node)->columns_arr[j]->case_node->case_then_value_head;
							while (cur_then != NULL)
							{
								if (cur_then->ptr_type == PTR_TYPE_FUNC_NODE)
								{
									if (the_debug == YES_DEBUG)
										printf("Found func\n");
									funcs = true;
									if (getAggFuncGroups(&groups_head, &groups_tail, &num_rows_in_result, cur_then->ptr_value, *select_node, malloced_head, the_debug) != RETURN_GOOD)
									{
										if (the_debug == YES_DEBUG)
											printf("	ERROR in selectStuff() at line %d in %s\n", __LINE__, __FILE__);
										return RETURN_ERROR;
									}
								}
								else if (cur_then->ptr_type == PTR_TYPE_MATH_NODE)
								{
									void* ptr = NULL;
									int ptr_type = -1;

									struct math_node* cur_mirror = myMalloc(sizeof(struct math_node), NULL, malloced_head, the_debug);
									if (cur_mirror == NULL)
									{
										if (the_debug == YES_DEBUG)
											printf("	ERROR in preSelectAnalysis() at line %d in %s\n", __LINE__, __FILE__);
										return RETURN_ERROR;
									}
									initEmptyTreeNode((void**) &cur_mirror, NULL, PTR_TYPE_MATH_NODE);

									while (true) // Will always break, breaks when traversd == -1
									{
										int traversd = traverseTreeNode((void**) &cur_then->ptr_value, PTR_TYPE_MATH_NODE, &ptr, &ptr_type, (void**) &cur_mirror
																	   ,NULL, malloced_head, the_debug);
										if (traversd == -1)
										{
											if (the_debug == YES_DEBUG)
												printf("Natural break\n");
											break;
										}
										else if (traversd == -2)
										{
											if (the_debug == YES_DEBUG)
												printf("	ERROR in preSelectAnalysis() at line %d in %s\n", __LINE__, __FILE__);
											return RETURN_ERROR;
										}

										if (ptr_type == PTR_TYPE_FUNC_NODE)
										{
											if (the_debug == YES_DEBUG)
												printf("Found func\n");
											funcs = true;
											if (getAggFuncGroups(&groups_head, &groups_tail, &num_rows_in_result, ptr, *select_node, malloced_head, the_debug) != RETURN_GOOD)
											{
												if (the_debug == YES_DEBUG)
													printf("	ERROR in selectStuff() at line %d in %s\n", __LINE__, __FILE__);
												return RETURN_ERROR;
											}
										}
									}
								}

								cur_then = cur_then->next;
							}
						}

						if (funcs)
							break;
					// END See if func node exists anywhere
				}

				for (int j=0; j<(*select_node)->columns_arr_size; j++)
				{
					if ((*select_node)->columns_arr[j]->col_ptr_type == PTR_TYPE_COL_IN_SELECT_NODE)
					{
						(*select_node)->columns_arr[j]->col_data_arr = ((struct col_in_select_node*) (*select_node)->columns_arr[j]->col_ptr)->col_data_arr;
						(*select_node)->columns_arr[j]->num_rows = ((struct col_in_select_node*) (*select_node)->columns_arr[j]->col_ptr)->num_rows;
						(*select_node)->columns_arr[j]->rows_data_type = ((struct col_in_select_node*) (*select_node)->columns_arr[j]->col_ptr)->rows_data_type;
						//(*select_node)->columns_arr[j]->unique_values_head = ((struct col_in_select_node*) (*select_node)->columns_arr[j]->col_ptr)->unique_values_head;
						//(*select_node)->columns_arr[j]->unique_values_tail = ((struct col_in_select_node*) (*select_node)->columns_arr[j]->col_ptr)->unique_values_tail;

						if ((((struct col_in_select_node*) (*select_node)->columns_arr[j]->col_ptr)->func_node != NULL
								|| ((struct col_in_select_node*) (*select_node)->columns_arr[j]->col_ptr)->math_node != NULL
								|| ((struct col_in_select_node*) (*select_node)->columns_arr[j]->col_ptr)->case_node != NULL)
							&& (*select_node)->columns_arr[j]->new_name == NULL)
						{
							(*select_node)->columns_arr[j]->new_name = ((struct col_in_select_node*) (*select_node)->columns_arr[j]->col_ptr)->new_name;
							((struct col_in_select_node*) (*select_node)->columns_arr[j]->col_ptr)->new_name = NULL;
						}
					}
					else if ((*select_node)->columns_arr[j]->func_node != NULL)
					{
						if (the_debug == YES_DEBUG)
							printf("Exec agg func\n");
						if (execAggFunc(groups_head, groups_tail, num_rows_in_result, (*select_node)->columns_arr[j], *select_node, malloced_head, the_debug) != RETURN_GOOD)
						{
							if (the_debug == YES_DEBUG)
								printf("	ERROR in selectStuff() at line %d in %s\n", __LINE__, __FILE__);
							return RETURN_ERROR;
						}
						(*select_node)->columns_arr[j]->rows_data_type = (*select_node)->columns_arr[j]->func_node->result_type;
						if (the_debug == YES_DEBUG)
							printf("Col datatype = %d\n", (*select_node)->columns_arr[j]->rows_data_type);
					}
					/*else if ((*select_node)->columns_arr[j]->math_node != NULL)
					{
						if (the_debug == YES_DEBUG)
							printf("Found math\n");
						if (execMathNode(groups_head, groups_tail, (*select_node)->columns_arr[j], num_rows_in_result, malloced_head, the_debug) != RETURN_GOOD)
						{
							if (the_debug == YES_DEBUG)
								printf("	ERROR in selectStuff() at line %d in %s\n", __LINE__, __FILE__);
							return RETURN_ERROR;
						}
					}
					else if ((*select_node)->columns_arr[j]->case_node != NULL)
					{
						if (the_debug == YES_DEBUG)
							printf("Found case\n");
						if (execCaseNode(groups_head, groups_tail, *select_node, (*select_node)->columns_arr[j], num_rows_in_result, malloced_head, the_debug) != RETURN_GOOD)
						{
							if (the_debug == YES_DEBUG)
								printf("	ERROR in selectStuff() at line %d in %s\n", __LINE__, __FILE__);
							return RETURN_ERROR;
						}
						if (the_debug == YES_DEBUG)
							printf("Col datatype = %d\n", (*select_node)->columns_arr[j]->rows_data_type);
					}*/
				}
			// END Evaluate columns
			
			if ((*select_node)->having_head != NULL)
			{
				// START Evaluate having clause
					struct ListNodePtr* valid_rows_head = NULL;
					struct ListNodePtr* valid_rows_tail = NULL;

					int_8 num_rows_before_having = (*select_node)->columns_arr[0]->num_rows;

					if (the_debug == YES_DEBUG)
						printf("Found having clause\n");
					if (findValidRowsGivenWhere(&valid_rows_head, &valid_rows_tail
											   ,*select_node, NULL, (*select_node)->having_head
											   ,&(*select_node)->columns_arr[0]->num_rows
											   ,groups_head, groups_tail, false, -1, -1, malloced_head, the_debug) != RETURN_GOOD)
					{
						if (the_debug == YES_DEBUG)
							printf("	ERROR in selectStuff() at line %d in %s\n", __LINE__, __FILE__);
						return RETURN_ERROR;
					}

					
					if (the_debug == YES_DEBUG)
					{	
						traverseListNodesPtr(&valid_rows_head, &valid_rows_tail, TRAVERSELISTNODES_HEAD, "Valid rows: ");
						printf("Num rows after where = %d\n", (*select_node)->columns_arr[0]->num_rows);
						printf("num_rows_before_having = %d\n", num_rows_before_having);
					}

					if (shrinkRowsCuzWhereOrHaving(num_rows_before_having, (*select_node)->columns_arr[0]->num_rows
												  ,*select_node, valid_rows_head, valid_rows_tail
												  ,true, extra_cols_head, malloced_head, the_debug) != RETURN_GOOD)
					{
						if (the_debug == YES_DEBUG)
							printf("	ERROR in selectStuff() at line %d in %s\n", __LINE__, __FILE__);
						return RETURN_ERROR;
					}

					if (valid_rows_head != NULL)
						freeAnyLinkedList((void**) &valid_rows_head, PTR_TYPE_LIST_NODE_PTR, NULL, malloced_head, the_debug);
				// END Evaluate having clause
			}

			if (groups_head != NULL)
				freeAnyLinkedList((void**) &groups_head, PTR_TYPE_LIST_NODE_PTR, NULL, malloced_head, the_debug);

			if ((*select_node)->distinct)
			{
				// START Get distinct rows
					if (the_debug == YES_DEBUG)
						printf("Distinct declared\n");

					struct ListNodePtr* head = NULL;
					struct ListNodePtr* tail = NULL;

					int_8 num_rows_in_result = 0;

					if (getDistinctRowsV2(false, *select_node, NULL, &head, &tail, &num_rows_in_result, malloced_head, the_debug) != RETURN_GOOD)
					{
						if (the_debug == YES_DEBUG)
							printf("	ERROR in selectStuff() at line %d in %s\n", __LINE__, __FILE__);
						return RETURN_ERROR;
					}

					if (head != NULL)
						freeAnyLinkedList((void**) &head, PTR_TYPE_LIST_NODE_PTR, NULL, malloced_head, the_debug);
				// END Get distinct rows
			}

			if ((*select_node)->order_by != NULL)
			{
				// START Sort columns per order by clause
					if (the_debug == YES_DEBUG)
						printf("Found order by\n");
					struct col_in_select_node* last_sorted_col = NULL;

					struct ListNodePtr* cur_row = (*select_node)->order_by->order_by_cols_head;
					struct ListNodePtr* cur_which = (*select_node)->order_by->order_by_cols_which_head;

					while (cur_row != NULL)
					{
						//if (the_debug == YES_DEBUG) printf("Found one column in order by list\n");

						if (cur_row->prev == NULL)
						{
							sortAndReorderCols(cur_row, cur_which, 0, ((struct col_in_select_node*) cur_row->ptr_value)->num_rows-1);
						}
						else
						{
							int left = 0;
							int right = 0;
							for (int i=1; i<((struct col_in_select_node*) cur_row->prev->ptr_value)->num_rows; i++)
							{
								if (cur_row->next != NULL)
									((struct col_in_select_node*) cur_row->ptr_value)->col_data_arr[i]->row_id = i;

								struct ListNodePtr* temp = (*select_node)->order_by->order_by_cols_head;
								while (temp != cur_row)
								{
									if (!equals(((struct col_in_select_node*) temp->ptr_value)->col_data_arr[i]->row_data, ((struct col_in_select_node*) temp->ptr_value)->rows_data_type
												,((struct col_in_select_node*) temp->ptr_value)->col_data_arr[i-1]->row_data, VALUE_EQUALS))
									{
										break;
									}
									temp = temp->next;
								}

								if (temp == cur_row)
								{
									//printf("B\n");
									right++;
								}
								//if (equals(((struct col_in_select_node*) cur_row->prev->ptr_value)->col_data_arr[i]->row_data, ((struct col_in_select_node*) cur_row->prev->ptr_value)->rows_data_type
								//			,((struct col_in_select_node*) cur_row->prev->ptr_value)->col_data_arr[i-1]->row_data, VALUE_EQUALS))
								//{
									//printf("B\n");
								//	right++;
								//}
								else
								{
									//printf("C: %d - %d\n", left, right);
									if (left < right)
										sortAndReorderCols(cur_row, cur_which, left, right);

									left = i;
									right = i;
								}
							}

							//printf("C: %d - %d\n", left, right);
							if (left < right)
								sortAndReorderCols(cur_row, cur_which, left, right);
						}


						if (cur_row->next == NULL)
							last_sorted_col = cur_row->ptr_value;

						cur_row = cur_row->next;
						cur_which = cur_which->next;
					}

					// START Reorder all other columns
						for (int j=0; j<(*select_node)->columns_arr_size; j++)
						{
							struct ListNodePtr* temp = (*select_node)->order_by->order_by_cols_head;
							while (temp != NULL)
							{
								if (temp->ptr_value == (*select_node)->columns_arr[j])
									break;

								temp = temp->next;
							}

							if (temp == NULL)
							{
								//if (the_debug == YES_DEBUG) printf("Col at index %d needs reordering at end\n", j);

								reorderCols((*select_node)->columns_arr[j], last_sorted_col, 0, (*select_node)->columns_arr[j]->num_rows-1);

								for (int i=0; i<(*select_node)->columns_arr[j]->num_rows; i++)
								{
									(*select_node)->columns_arr[j]->col_data_arr[i]->row_id = i;
								}
							}
						}
					// END Reorder all other columns

					for (int j=0; j<(*select_node)->columns_arr_size; j++)
					{
						for (int i=0; i<(*select_node)->columns_arr[j]->num_rows; i++)
						{
							(*select_node)->columns_arr[j]->col_data_arr[i]->row_id = i;
						}
					}
				// END Sort columns per order by clause
			}
		}

		// START Some tests that are not done for production
			if (the_debug == YES_DEBUG)
			{
				for (int j=1; j<(*select_node)->columns_arr_size; j++)
				{
					if ((*select_node)->columns_arr[j]->num_rows != (*select_node)->columns_arr[0]->num_rows)
					{
						printf("	ERROR in selectStuff() at line %d in %s\n", __LINE__, __FILE__);
						printf("Column at index %d with num_rows = %d did not equal column at index 0 with num_rows = %d\n", j, (*select_node)->columns_arr[j]->num_rows, (*select_node)->columns_arr[0]->num_rows);
						errorTeardown(NULL, malloced_head, the_debug);
						return RETURN_ERROR;
					}
				}

				for (int j=0; j<(*select_node)->columns_arr_size; j++)
				{
					for (int i=0; i<(*select_node)->columns_arr[j]->num_rows; i++)
					{
						if ((*select_node)->columns_arr[j]->col_data_arr[i]->row_id != i)
						{
							printf("	ERROR in selectStuff() at line %d in %s\n", __LINE__, __FILE__);
							printf("Column at index %d with num_rows = %d has a row_id at array index %d that did not equal the index. Row_id = %d\n", j, (*select_node)->columns_arr[j]->num_rows, i, (*select_node)->columns_arr[j]->col_data_arr[i]->row_id);
							errorTeardown(NULL, malloced_head, the_debug);
							return RETURN_ERROR;
						}
					}
				}
			}
		// END Some tests that are not done for production

		if ((*select_node)->next != NULL)
			*select_node = (*select_node)->next;
		else
			break;
	}


	if (extra_cols_head != NULL)
	{
		// START Free extra unselected cols
			struct ListNodePtr* cur = extra_cols_head;
			while (cur != NULL)
			{
				if (the_debug == YES_DEBUG)
					printf("Freeing one column's col_data_arr from extra_cols_head\n");

				myFreeResultsOfSelect(cur->ptr_value, malloced_head, the_debug);

				cur = cur->next;
			}

			freeAnyLinkedList((void**) &extra_cols_head, PTR_TYPE_LIST_NODE_PTR, NULL, malloced_head, the_debug);
		// END Free extra unselected cols
	}


	return RETURN_GOOD;
}

int traverseTablesInfoDisk(struct malloced_node** malloced_head, int the_debug)
{
	struct file_opened_node* file_opened_head = NULL;

	FILE* db_info = myFileOpen("_Info", -1, -1, "rb+", &file_opened_head, malloced_head, the_debug);
	if (db_info == NULL)
	{
		if (the_debug == YES_DEBUG)
			printf("	ERROR in traverseTablesInfoDisk() at line %d in %s\n", __LINE__, __FILE__);
		return RETURN_ERROR;
	}

	int_8 temp_num_tables = readFileInt(db_info, 0, YES_TRAVERSE_DISK, &file_opened_head, malloced_head, the_debug);
	printf("\nDATA FILES ON DISK\n\nnum_tables = %lu\n\n", temp_num_tables);

	int cur_offset = 8;
	char* temp_table_name;
	while ((temp_table_name = readFileChar(db_info, cur_offset, YES_TRAVERSE_DISK, &file_opened_head, malloced_head, the_debug)) != NULL)
	{
		printf("Table name = %s\n", temp_table_name);
		myFree( (void**) &temp_table_name, &file_opened_head, malloced_head, the_debug);
		cur_offset += 32;
		char* keyword = readFileChar(db_info, cur_offset, YES_TRAVERSE_DISK, &file_opened_head, malloced_head, the_debug);
		printf("Keyword = %s\n", keyword);
		myFree((void**) &keyword, &file_opened_head, malloced_head, the_debug);
		cur_offset += 32;
		int_8 table_number = readFileInt(db_info, cur_offset, YES_TRAVERSE_DISK, &file_opened_head, malloced_head, the_debug);
		printf("Table number = %lu\n", table_number);
		cur_offset += 8;

		/*FILE* tab_col = myFileOpen("_Tab_Col_", table_number, 0, "rb+\0", &file_opened_head, malloced_head, the_debug);
		if (tab_col == NULL)
		{
			if (the_debug == YES_DEBUG)
				printf("	ERROR in traverseTablesInfoDisk() at line %d in %s\n", __LINE__, __FILE__);
			return RETURN_ERROR;
		}
		
		int cur_col_offset = 8;
		char* temp_col_name;
		while ((temp_col_name = readFileChar(tab_col, cur_col_offset, YES_TRAVERSE_DISK, &file_opened_head, malloced_head, the_debug)) != NULL)
		{
			printf("	Column name = %s\n", temp_col_name);
			myFree((void**) &temp_col_name, &file_opened_head, malloced_head, the_debug);
			cur_col_offset += 32;

			int_8 temp_data = readFileInt(tab_col, cur_col_offset, YES_TRAVERSE_DISK, &file_opened_head, malloced_head, the_debug);
			int_8 temp_data_type = temp_data & 0xff;
			int_8 temp_max_length = temp_data >> 8;

			printf("	Datatype = %lu\n", temp_data_type);
			printf("	Max length = %lu\n", temp_max_length);
			cur_col_offset += 8;
			int_8 col_number = readFileInt(tab_col, cur_col_offset, YES_TRAVERSE_DISK, &file_opened_head, malloced_head, the_debug);
			printf("	Column number = %lu\n", col_number);
			cur_col_offset += 8;

			FILE* col_data_info = myFileOpen("_Col_Data_Info_", table_number, col_number, "rb+\0", &file_opened_head, malloced_head, the_debug);
			if (col_data_info == NULL)
			{
				if (the_debug == YES_DEBUG)
					printf("	ERROR in traverseTablesInfoDisk() at line %d in %s\n", __LINE__, __FILE__);
				return RETURN_ERROR;
			}

			printf("	Number of rows = %lu\n", readFileInt(col_data_info, 0, YES_TRAVERSE_DISK, &file_opened_head, malloced_head, the_debug));
			printf("	Number of open slots = %lu\n", readFileInt(col_data_info, 8, YES_TRAVERSE_DISK, &file_opened_head, malloced_head, the_debug));

			printf("	Open slots: ");
			int_8 open_offset = 16;
			int_8 open_slot;
			while ((open_slot = readFileInt(col_data_info, open_offset, YES_TRAVERSE_DISK, &file_opened_head, malloced_head, the_debug)) != -1)
			{
				printf("%lu, ", open_slot);
				open_offset += 8;
			}
			printf("\n");

			myFileClose(col_data_info, &file_opened_head, malloced_head, the_debug);

			FILE* col_data = myFileOpen("_Col_Data_", table_number, col_number, "rb+\0", &file_opened_head, malloced_head, the_debug);
			if (col_data == NULL)
			{
				if (the_debug == YES_DEBUG)
					printf("	ERROR in traverseTablesInfoDisk() at line %d in %s\n", __LINE__, __FILE__);
				return RETURN_ERROR;
			}

			printf("	Column data:\n");
			int_8 rows_offset = 0;
			int_8 row_id;
			while ((row_id = readFileInt(col_data, rows_offset, YES_TRAVERSE_DISK, &file_opened_head, malloced_head, the_debug)) != -1)
			{
				printf("		%d	", row_id);
				rows_offset += 8;

				if (temp_data_type == DATA_INT || temp_data_type == DATA_DATE)
					printf("%lu\n", readFileInt(col_data, rows_offset, YES_TRAVERSE_DISK, &file_opened_head, malloced_head, the_debug));
				else if (temp_data_type == DATA_REAL)
					printf("%f\n", readFileDouble(col_data, rows_offset, YES_TRAVERSE_DISK, &file_opened_head, malloced_head, the_debug));
				else if (temp_data_type == DATA_STRING)
				{
					char* temp_str = readFileCharData(col_data, rows_offset, &temp_max_length, YES_TRAVERSE_DISK, &file_opened_head, malloced_head, the_debug);
					printf("%s\n", temp_str);
					myFree((void**) &temp_str, &file_opened_head, malloced_head, the_debug);
				}
				rows_offset += temp_max_length;
			}
			myFileClose(col_data, &file_opened_head, malloced_head, the_debug);
		}
		myFileClose(tab_col, &file_opened_head, malloced_head, the_debug);*/
	}
	myFileClose(db_info, &file_opened_head, malloced_head, the_debug);


    if (file_opened_head != NULL)
    {
        if (the_debug == YES_DEBUG)
			printf("traverseTablesInfoDisk() did not close all files\n");
        myFileCloseAll(&file_opened_head, malloced_head, the_debug);
    }

	return RETURN_GOOD;
}

/*	RETURNS:
 *  RETURN_ERROR if error
 *	RETURN_GOOD if good
 *
 *	WRITES TO:
 *	the_table
 *	 
 *	Persistent malloced items: 
 *		struct frequent_node* unique_list_head (for each column in table);
 *		struct frequent_node* frequent_list_head (for each column in table);
 */
int initFrequentLists(struct table_info* the_table
					 ,struct malloced_node** malloced_head, int the_debug)
{
	struct table_cols_info* cur_col = the_table->table_cols_head;
	for (int j=0; j<the_table->num_cols; j++)
	{
		cur_col->frequent_arr_row_to_node = (struct frequent_node**) myMalloc(sizeof(struct frequent_node*) * cur_col->num_rows, NULL, malloced_head, the_debug);
		if (cur_col->frequent_arr_row_to_node == NULL)
		{
			if (the_debug == YES_DEBUG)
				printf("	ERROR in initFrequentLists() at line %d in %s\n", __LINE__, __FILE__);
			return RETURN_ERROR;
		}

		struct frequent_node* freq_head = NULL;
		struct frequent_node* freq_tail = NULL;

		//printf("	Getting col_data\n");
		struct colDataNode** col_data = getAllColData(the_table->file_number, cur_col, NULL, -1, malloced_head, the_debug);
		if (col_data == NULL)
		{
			if (the_debug == YES_DEBUG)
				printf("	ERROR in initFrequentLists() at line %d in %s\n", __LINE__, __FILE__);
			return RETURN_ERROR;
		}
		//printf("	Done getting col_data\n");

		for (int i=0; i<cur_col->num_rows; i++)
		{
			cur_col->frequent_arr_row_to_node[i] = NULL;

			bool new_freq_node = false;
			bool open_row = false;

			// START Check if row is open
				//printf("col_data[i]->row_id = %d\n", col_data[i]->row_id);
				if (((int) col_data[i]->row_id) < 0)
					open_row = true;
				//printf("open_row = %s\n", open_row ? "Yes" : "No");
			// END Check if row is open

			if (freq_head == NULL && !open_row)
			{
				//printf("		Adding to freq_head\n");
				freq_head = (struct frequent_node*) myMalloc(sizeof(struct frequent_node), NULL, malloced_head, the_debug);
				if (freq_head == NULL)
				{
					if (the_debug == YES_DEBUG)
						printf("	ERROR in initFrequentLists() at line %d in %s\n", __LINE__, __FILE__);
					return RETURN_ERROR;
				}
				freq_head->ptr_value = col_data[i]->row_data;
				freq_head->num_appearences = 1;
				freq_head->row_nums_head = NULL;
				freq_head->row_nums_tail = NULL;

				int* int1 = (int*) myMalloc(sizeof(int), NULL, malloced_head, the_debug);
				if (int1 == NULL)
				{
					if (the_debug == YES_DEBUG)
						printf("	ERROR in initFrequentLists() at line %d in %s\n", __LINE__, __FILE__);
					return RETURN_ERROR;
				}

				*int1 = col_data[i]->row_id;

				if (addListNodePtr(&freq_head->row_nums_head, &freq_head->row_nums_tail, int1, PTR_TYPE_INT, ADDLISTNODE_TAIL, NULL, malloced_head, the_debug) != 0)
				{
					if (the_debug == YES_DEBUG)
						printf("	ERROR in initFrequentLists() at line %d in %s\n", __LINE__, __FILE__);
					return RETURN_ERROR;
				}

				cur_col->frequent_arr_row_to_node[col_data[i]->row_id] = freq_head;

				freq_head->prev = NULL;
				freq_head->next = NULL;

				freq_tail = freq_head;

				new_freq_node = true;
			}
			else if (!open_row)
			{
				//printf("		Finding existing in freq_head\n");
				struct frequent_node* cur_freq = freq_head;
				while (cur_freq != NULL)
				{
					//printf("		Comparing _%s_ vs _%s_\n", cur_freq->ptr_value, col_data[i]->row_data);
					if ((col_data[i]->row_data == NULL && cur_freq->ptr_value == NULL) || equals(cur_freq->ptr_value, cur_col->data_type, col_data[i]->row_data, VALUE_EQUALS))
					{
						//printf("		FOUND existing in freq_head\n");
						cur_freq->num_appearences++;

						int* int1 = (int*) myMalloc(sizeof(int), NULL, malloced_head, the_debug);
						if (int1 == NULL)
						{
							if (the_debug == YES_DEBUG)
								printf("	ERROR in initFrequentLists() at line %d in %s\n", __LINE__, __FILE__);
							return RETURN_ERROR;
						}

						*int1 = col_data[i]->row_id;

						if (addListNodePtr(&cur_freq->row_nums_head, &cur_freq->row_nums_tail, int1, PTR_TYPE_INT, ADDLISTNODE_TAIL, NULL, malloced_head, the_debug) != 0)
						{
							if (the_debug == YES_DEBUG)
								printf("	ERROR in initFrequentLists() at line %d in %s\n", __LINE__, __FILE__);
							return RETURN_ERROR;
						}

						cur_col->frequent_arr_row_to_node[col_data[i]->row_id] = cur_freq;

						break;
					}

					cur_freq = cur_freq->next;
				}

				if (cur_freq == NULL)
				{
					//printf("		None found, adding to freq_tail\n");
					freq_tail->next = (struct frequent_node*) myMalloc(sizeof(struct frequent_node), NULL, malloced_head, the_debug);
					if (freq_tail->next == NULL)
					{
						if (the_debug == YES_DEBUG)
							printf("	ERROR in initFrequentLists() at line %d in %s\n", __LINE__, __FILE__);
						return RETURN_ERROR;
					}
					freq_tail->next->ptr_value = col_data[i]->row_data;
					freq_tail->next->num_appearences = 1;
					freq_tail->next->row_nums_head = NULL;
					freq_tail->next->row_nums_tail = NULL;

					int* int1 = (int*) myMalloc(sizeof(int), NULL, malloced_head, the_debug);
					if (int1 == NULL)
					{
						if (the_debug == YES_DEBUG)
							printf("	ERROR in initFrequentLists() at line %d in %s\n", __LINE__, __FILE__);
						return RETURN_ERROR;
					}

					*int1 = col_data[i]->row_id;

					if (addListNodePtr(&freq_tail->next->row_nums_head, &freq_tail->next->row_nums_tail, int1, PTR_TYPE_INT, ADDLISTNODE_TAIL, NULL, malloced_head, the_debug) != 0)
					{
						if (the_debug == YES_DEBUG)
							printf("	ERROR in initFrequentLists() at line %d in %s\n", __LINE__, __FILE__);
						return RETURN_ERROR;
					}

					cur_col->frequent_arr_row_to_node[col_data[i]->row_id] = freq_tail->next;

					freq_tail->next->prev = freq_tail;
					freq_tail->next->next = NULL;

					freq_tail = freq_tail->next;

					new_freq_node = true;
				}
			}

			if (!new_freq_node)
			{
				//printf("		Found existing, freeing row_data\n");
				myFree((void**) &(col_data[i]->row_data), NULL, malloced_head, the_debug);
			}
			myFree((void**) &(col_data[i]), NULL, malloced_head, the_debug);

			//printf("		Doing next row\n");`
		}

		myFree((void**) &col_data, NULL, malloced_head, the_debug);

		/*printf("Results:\n");
		while (freq_head != NULL)
		{
			if (freq_head->num_appearences > 1)
			{
				printf("ptr_value = _%s_\n", freq_head->ptr_value);
				printf("num_appearences = %d\n", freq_head->num_appearences);
				traverseListNodes(&freq_head->row_nums_head, &freq_head->row_nums_tail, TRAVERSELISTNODES_HEAD, "row_nums_head = ");
			}

			struct frequent_node* temp = freq_head;

			freq_head = freq_head->next;

			myFree((void**) &(temp->ptr_value), NULL, malloced_head, the_debug);
			freeListNodes(&temp->row_nums_head, NULL, malloced_head, the_debug);
			myFree((void**) &temp, NULL, malloced_head, the_debug);
		}*/

		// START Add frequent list to unique or frequent lists of column
		struct frequent_node* cur_freq = freq_head;
		while (cur_freq != NULL)
		{
			struct frequent_node* temp_next = cur_freq->next;

			if (cur_freq->num_appearences > 1)
			{
				if (cur_col->frequent_list_head == NULL)
				{
					cur_col->frequent_list_head = cur_freq;
					cur_col->frequent_list_head->prev = NULL;
					cur_col->frequent_list_head->next = NULL;
					cur_col->frequent_list_tail = cur_col->frequent_list_head;
				}
				else
				{
					cur_freq->prev = cur_col->frequent_list_tail;
					cur_freq->next = NULL;
					cur_col->frequent_list_tail->next = cur_freq;
					cur_col->frequent_list_tail = cur_col->frequent_list_tail->next;
				}
			}
			else
			{
				if (cur_col->unique_list_head == NULL)
				{
					cur_col->unique_list_head = cur_freq;
					cur_col->unique_list_head->next = NULL;
					cur_col->unique_list_head->prev = NULL;
					cur_col->unique_list_tail = cur_col->unique_list_head;
				}
				else
				{
					cur_freq->prev = cur_col->unique_list_tail;
					cur_freq->next = NULL;
					cur_col->unique_list_tail->next = cur_freq;
					cur_col->unique_list_tail = cur_col->unique_list_tail->next;
				}
			}

			cur_freq = temp_next;
		}
		// END Add frequent list to unique or frequent lists of column

		/*
		for (int i=0; i<cur_col->num_rows; i++)
		{
			printf("%d : ");
			printf("%s\n", cur_col->frequent_arr_row_to_node[i] == NULL ? "NULL" : cur_col->frequent_arr_row_to_node[i]->ptr_value);
		}*/

		cur_col = cur_col->next;
	}
	//printf("Done all cols\n");

	return RETURN_GOOD;
}

/*	RETURNS:
 *  RETURN_ERROR if error
 *	RETURN_GOOD if good
 *
 *	WRITES TO:
 *	cur_col->frequent_arr_row_to_node[*row_id_to_remove]
 */
int removeFreqNodeFromFreqLists(int* row_id_to_remove, struct table_cols_info* cur_col, int the_debug)
{
	struct frequent_node* freq_node = cur_col->frequent_arr_row_to_node[*row_id_to_remove];
	//printf("Found freq_node = _%s_ to remove\n", freq_node->ptr_value);

	if (freq_node == NULL)
	{
		if (the_debug == YES_DEBUG)
			printf("	ERROR in removeFreqNodeFromFreqLists() at line %d in %s\n", __LINE__, __FILE__);
		//errorTeardown(&file_opened_head, malloced_head, the_debug);
		return RETURN_ERROR;
	}

	if (freq_node->num_appearences > 1)
	{
		//printf("A\n");
		//printf("Removing %d\n", *row_id_to_remove);
		freq_node->num_appearences--;

		removeListNodePtr(&freq_node->row_nums_head, &freq_node->row_nums_tail, row_id_to_remove, PTR_TYPE_INT, -1
					     ,NULL, NULL, the_debug);


		//traverseListNodes(&freq_node->row_nums_head, &freq_node->row_nums_tail, TRAVERSELISTNODES_HEAD, "Head: ");
		//traverseListNodes(&freq_node->row_nums_head, &freq_node->row_nums_tail, TRAVERSELISTNODES_TAIL, "Tail: ");
	}
	else
	{
		//printf("B\n");
		struct frequent_node* temp = freq_node;

		if (freq_node->prev != NULL)
		{
			freq_node->prev->next = freq_node->next;
		}

		if (freq_node->next != NULL)
		{
			freq_node->next->prev = freq_node->prev;
		}

		if (cur_col->frequent_list_head == freq_node)
		{
			cur_col->frequent_list_head = cur_col->frequent_list_head->next;
		}
		else if (cur_col->unique_list_head == freq_node)
		{
			cur_col->unique_list_head = cur_col->unique_list_head->next;
		}
		else if (cur_col->frequent_list_tail == freq_node)
		{
			cur_col->frequent_list_tail = cur_col->frequent_list_tail->prev;
		}
		else if (cur_col->unique_list_tail == freq_node)
		{
			cur_col->unique_list_tail = cur_col->unique_list_tail->prev;
		}

		if (temp->ptr_value != NULL)
		{
			//free(temp->ptr_value);
			myFree((void**) &temp->ptr_value, NULL, NULL, the_debug);
		}
		freeAnyLinkedList((void**) &temp->row_nums_head, PTR_TYPE_LIST_NODE_PTR, NULL, NULL, the_debug);
		//free(temp);
		myFree((void**) &temp, NULL, NULL, the_debug);
	}

	cur_col->frequent_arr_row_to_node[*row_id_to_remove] = NULL;

	return RETURN_GOOD;
}

/*	RETURNS:
 *  RETURN_ERROR if error
 *	RETURN_GOOD if good
 *
 *	WRITES TO:
 *	freq_head
 *	freq_tail
 */
int addFreqNodeToTempNewList(int add_mode, struct frequent_node** freq_head, struct frequent_node** freq_tail, struct ListNodePtr* new_data, int* new_row_id, struct table_cols_info* cur_col
							,struct file_opened_node** file_opened_head, struct malloced_node** malloced_head, int the_debug)
{
	if (*freq_head == NULL)
	{
		if (the_debug == YES_DEBUG)
			printf("		Adding to freq_head\n");

		*freq_head = (struct frequent_node*) myMalloc(sizeof(struct frequent_node), file_opened_head, malloced_head, the_debug);
		if (*freq_head == NULL)
		{
			if (the_debug == YES_DEBUG)
				printf("	ERROR in addFreqNodeToTempNewList() at line %d in %s\n", __LINE__, __FILE__);
			return RETURN_ERROR;
		}

		//printf("cur_col->max_length = %lu\n", cur_col->max_length);

		// START Copy new data to freq_head
			(*freq_head)->ptr_value = NULL;

			if (new_data == NULL)
			{
				(*freq_head)->ptr_value = NULL;
			}
			else if (new_data->ptr_type == PTR_TYPE_INT)
			{
				(*freq_head)->ptr_value = (int*) myMalloc(sizeof(int), file_opened_head, malloced_head, the_debug);
				if ((*freq_head)->ptr_value == NULL)
				{
					if (the_debug == YES_DEBUG)
						printf("	ERROR in addFreqNodeToTempNewList() at line %d in %s\n", __LINE__, __FILE__);
					return RETURN_ERROR;
				}

				*((int*) (*freq_head)->ptr_value) = *((int*) new_data->ptr_value);
			}
			else if (new_data->ptr_type == PTR_TYPE_REAL)
			{
				(*freq_head)->ptr_value = (double*) myMalloc(sizeof(double), file_opened_head, malloced_head, the_debug);
				if ((*freq_head)->ptr_value == NULL)
				{
					if (the_debug == YES_DEBUG)
						printf("	ERROR in addFreqNodeToTempNewList() at line %d in %s\n", __LINE__, __FILE__);
					return RETURN_ERROR;
				}

				*((double*) (*freq_head)->ptr_value) = *((double*) new_data->ptr_value);
			}
			else if (new_data->ptr_type == PTR_TYPE_CHAR || new_data->ptr_type == PTR_TYPE_DATE)
			{
				(*freq_head)->ptr_value = (char*) myMalloc(sizeof(char) * (strLength(new_data->ptr_value)+1), file_opened_head, malloced_head, the_debug);
				if ((*freq_head)->ptr_value == NULL)
				{
					if (the_debug == YES_DEBUG)
						printf("	ERROR in addFreqNodeToTempNewList() at line %d in %s\n", __LINE__, __FILE__);
					return RETURN_ERROR;
				}

				strcpy((*freq_head)->ptr_value, new_data->ptr_value);
			}
		// END Copy new data to freq_head

		(*freq_head)->num_appearences = 1;
		(*freq_head)->row_nums_head = NULL;
		(*freq_head)->row_nums_tail = NULL;

		if (addListNodePtr_Int(&(*freq_head)->row_nums_head, &(*freq_head)->row_nums_tail, *new_row_id, ADDLISTNODE_TAIL, file_opened_head, malloced_head, the_debug) != 0)
		{
			if (the_debug == YES_DEBUG)
				printf("	ERROR in addFreqNodeToTempNewList() at line %d in %s\n", __LINE__, __FILE__);
			return RETURN_ERROR;
		}

		cur_col->frequent_arr_row_to_node[*new_row_id] = *freq_head;

		(*freq_head)->prev = NULL;
		(*freq_head)->next = NULL;
	}
	else
	{
		// START Traverse through existing in freq_head and find if matching exists, else, add to freq_tail
			if (the_debug == YES_DEBUG)
				printf("		Traversing freq_head to find\n");

			struct frequent_node* cur_freq = *freq_head;
			while (cur_freq != NULL)
			{
				//printf("	Comparing _%s_ vs _%s_\n", cur_freq->ptr_value, new_data->ptr_value);
				//printf("	Col data type = %d\n", cur_col->data_type);
				if ((new_data == NULL && cur_freq->ptr_value == NULL) 
					|| (new_data != NULL && cur_freq->ptr_value != NULL && equals(cur_freq->ptr_value, cur_col->data_type, new_data->ptr_value, VALUE_EQUALS)))
				{
					if (the_debug == YES_DEBUG)
						printf("	Found\n");

					cur_freq->num_appearences++;
					if (addListNodePtr_Int(&cur_freq->row_nums_head, &cur_freq->row_nums_tail, *new_row_id, ADDLISTNODE_TAIL, file_opened_head, malloced_head, the_debug) != 0)
					{
						if (the_debug == YES_DEBUG)
							printf("	ERROR in addFreqNodeToTempNewList() at line %d in %s\n", __LINE__, __FILE__);
						return RETURN_ERROR;
					}

					cur_col->frequent_arr_row_to_node[*new_row_id] = cur_freq;

					break;
				}

				cur_freq = cur_freq->next;
			}

			if (cur_freq == NULL && freq_tail != NULL)
			{
				if (the_debug == YES_DEBUG)
					printf("		Adding to freq_tail\n");

				(*freq_tail)->next = (struct frequent_node*) myMalloc(sizeof(struct frequent_node), file_opened_head, malloced_head, the_debug);
				if ((*freq_tail)->next == NULL)
				{
					if (the_debug == YES_DEBUG)
						printf("	ERROR in addFreqNodeToTempNewList() at line %d in %s\n", __LINE__, __FILE__);
					return RETURN_ERROR;
				}

				//printf("cur_col->max_length = %lu\n", cur_col->max_length);

				(*freq_tail)->next->ptr_value = (char*) myMalloc(sizeof(char) * 200, file_opened_head, malloced_head, the_debug);
				if ((*freq_tail)->next != NULL && (*freq_tail)->next->ptr_value == NULL)
				{
					if (the_debug == YES_DEBUG)
						printf("	ERROR in addFreqNodeToTempNewList() at line %d in %s\n", __LINE__, __FILE__);
					return RETURN_ERROR;
				}

				// START Copy new data to (*freq_tail)->next
					(*freq_tail)->next->ptr_value = NULL;

					if (new_data == NULL)
					{
						(*freq_tail)->next->ptr_value = NULL;
					}
					else if (new_data->ptr_type == PTR_TYPE_INT)
					{
						(*freq_tail)->next->ptr_value = (int*) myMalloc(sizeof(int), file_opened_head, malloced_head, the_debug);
						if ((*freq_tail)->next->ptr_value == NULL)
						{
							if (the_debug == YES_DEBUG)
								printf("	ERROR in addFreqNodeToTempNewList() at line %d in %s\n", __LINE__, __FILE__);
							return RETURN_ERROR;
						}

						*((int*) (*freq_tail)->next->ptr_value) = *((int*) new_data->ptr_value);
					}
					else if (new_data->ptr_type == PTR_TYPE_REAL)
					{
						(*freq_tail)->next->ptr_value = (double*) myMalloc(sizeof(double), file_opened_head, malloced_head, the_debug);
						if ((*freq_tail)->next->ptr_value == NULL)
						{
							if (the_debug == YES_DEBUG)
								printf("	ERROR in addFreqNodeToTempNewList() at line %d in %s\n", __LINE__, __FILE__);
							return RETURN_ERROR;
						}

						*((double*) (*freq_tail)->next->ptr_value) = *((double*) new_data->ptr_value);
					}
					else if (new_data->ptr_type == PTR_TYPE_CHAR || new_data->ptr_type == PTR_TYPE_DATE)
					{
						(*freq_tail)->next->ptr_value = (char*) myMalloc(sizeof(char) * (strLength(new_data->ptr_value)+1), file_opened_head, malloced_head, the_debug);
						if ((*freq_tail)->next->ptr_value == NULL)
						{
							if (the_debug == YES_DEBUG)
								printf("	ERROR in addFreqNodeToTempNewList() at line %d in %s\n", __LINE__, __FILE__);
							return RETURN_ERROR;
						}

						strcpy((*freq_tail)->next->ptr_value, new_data->ptr_value);
					}
				// END Copy new data to (*freq_tail)->next

				(*freq_tail)->next->num_appearences = 1;
				(*freq_tail)->next->row_nums_head = NULL;
				(*freq_tail)->next->row_nums_tail = NULL;
				if (addListNodePtr_Int(&(*freq_tail)->next->row_nums_head, &(*freq_tail)->next->row_nums_tail, *new_row_id, ADDLISTNODE_TAIL, file_opened_head, malloced_head, the_debug) != 0)
				{
					if (the_debug == YES_DEBUG)
						printf("	ERROR in addFreqNodeToTempNewList() at line %d in %s\n", __LINE__, __FILE__);
					return RETURN_ERROR;
				}

				cur_col->frequent_arr_row_to_node[*new_row_id] = (*freq_tail)->next;

				(*freq_tail)->next->prev = *freq_tail;
				(*freq_tail)->next->next = NULL;

				(*freq_tail) = (*freq_tail)->next;
			}
			else if (freq_tail != NULL)
			{
				if (the_debug == YES_DEBUG)
					printf("	ERROR in addFreqNodeToTempNewList() at line %d in %s\n", __LINE__, __FILE__);
				return RETURN_ERROR;
			}
		// END Traverse through existing in freq_head and find if matching exists, else, add to freq_tail
	}

	return RETURN_GOOD;
}

/*	RETURNS:
 *  RETURN_GOOD
 *
 *	WRITES TO:
 *	the_freq_node
 */
int addFreqNodeToFreqLists(struct frequent_node* the_freq_node, struct table_cols_info* cur_col
						  ,struct file_opened_node** file_opened_head, struct malloced_node** malloced_head, int the_debug)
{
	bool option_a = false;

	struct frequent_node* cur_freq = cur_col->frequent_list_head;
	while (cur_freq != NULL)
	{
		if ((cur_freq->ptr_value == NULL && the_freq_node->ptr_value == NULL)
			|| (cur_freq->ptr_value != NULL && the_freq_node->ptr_value != NULL && equals(cur_freq->ptr_value, cur_col->data_type, the_freq_node->ptr_value, VALUE_EQUALS)))
		{
			if (the_debug == YES_DEBUG) 
				printf("option A\n");

			option_a = true;

			cur_freq->row_nums_tail->next = the_freq_node->row_nums_head;
			the_freq_node->row_nums_head->prev = cur_freq->row_nums_tail;

			cur_freq->row_nums_tail = the_freq_node->row_nums_tail;

			cur_freq->num_appearences += the_freq_node->num_appearences;

			struct ListNodePtr* cur = the_freq_node->row_nums_head;
			while (cur != NULL)
			{
				cur_col->frequent_arr_row_to_node[*((int*) cur->ptr_value)] = cur_freq;
				cur = cur->next;
			}

			//myFree((void**) &the_freq_node->ptr_value, file_opened_head, malloced_head, the_debug);
			//myFree((void**) &the_freq_node, file_opened_head, malloced_head, the_debug);

			break;
		}

		cur_freq = cur_freq->next;
	}
	
	if (cur_freq == NULL && the_freq_node->num_appearences > 1)
	{
		// Add to head or tail of frequent
		if (cur_col->frequent_list_head == NULL)
		{
			if (the_debug == YES_DEBUG) 
				printf("option B\n");

			the_freq_node->next = NULL;
			the_freq_node->prev = NULL;

			cur_col->frequent_list_head = the_freq_node;
			cur_col->frequent_list_tail = the_freq_node;
		}
		else
		{
			if (the_debug == YES_DEBUG) 
				printf("option C\n");

			the_freq_node->prev = cur_col->frequent_list_tail;
			the_freq_node->next = NULL;

			cur_col->frequent_list_tail->next = the_freq_node;
			cur_col->frequent_list_tail = cur_col->frequent_list_tail->next;
		}
	}
	else if (cur_freq == NULL && the_freq_node->num_appearences == 1)
	{
		cur_freq = cur_col->unique_list_head;
		while (cur_freq != NULL)
		{
			if ((cur_freq->ptr_value == NULL && the_freq_node->ptr_value == NULL) 
				|| (cur_freq->ptr_value != NULL && the_freq_node->ptr_value != NULL && equals(cur_freq->ptr_value, cur_col->data_type, the_freq_node->ptr_value, VALUE_EQUALS)))
			{
				if (the_debug == YES_DEBUG) 
					printf("option D\n");

				the_freq_node->row_nums_tail->next = cur_freq->row_nums_head;
				cur_freq->row_nums_head->prev = the_freq_node->row_nums_tail;

				the_freq_node->row_nums_tail = cur_freq->row_nums_tail;

				the_freq_node->num_appearences += cur_freq->num_appearences;

				struct ListNodePtr* cur = the_freq_node->row_nums_head;
				while (cur != NULL)
				{
					cur_col->frequent_arr_row_to_node[*((int*) cur->ptr_value)] = the_freq_node;
					cur = cur->next;
				}

				struct frequent_node* temp = cur_freq;

				if (cur_freq->prev != NULL)
					cur_freq->prev->next = cur_freq->next;
				if (cur_freq->next != NULL)
					cur_freq->next->prev = cur_freq->prev;

				if (temp == cur_col->unique_list_head)
				{
					cur_col->unique_list_head = cur_col->unique_list_head->next;
				}
				else if (temp == cur_col->unique_list_tail)
				{
					cur_col->unique_list_tail = cur_col->unique_list_tail->prev;
				}

				//free(temp->ptr_value);
				//free(temp);
				myFree((void**) &temp->ptr_value, NULL, NULL,the_debug);
				myFree((void**) &temp, NULL, NULL, the_debug);

				// Add to head or tail of frequent
				if (cur_col->frequent_list_head == NULL)
				{
					the_freq_node->next = NULL;
					the_freq_node->prev = NULL;

					cur_col->frequent_list_head = the_freq_node;
					cur_col->frequent_list_tail = the_freq_node;
				}
				else
				{
					the_freq_node->prev = cur_col->frequent_list_tail;
					the_freq_node->next = NULL;

					cur_col->frequent_list_tail->next = the_freq_node;
					cur_col->frequent_list_tail = cur_col->frequent_list_tail->next;
				}

				break;
			}

			cur_freq = cur_freq->next;
		}

		if (cur_freq == NULL)
		{
			// Add to head or tail of unique
			if (cur_col->unique_list_head == NULL)
			{
				if (the_debug == YES_DEBUG) 
					printf("option E\n");

				the_freq_node->next = NULL;
				the_freq_node->prev = NULL;

				cur_col->unique_list_head = the_freq_node;
				cur_col->unique_list_tail = the_freq_node;
			}
			else
			{
				if (the_debug == YES_DEBUG) 
					printf("option F\n");

				the_freq_node->prev = cur_col->unique_list_tail;
				the_freq_node->next = NULL;

				cur_col->unique_list_tail->next = the_freq_node;
				cur_col->unique_list_tail = cur_col->unique_list_tail->next;
			}
		}
	}

	/*
	int count = 0;
	struct malloced_node* cur_mal = *malloced_head;
	while (cur_mal != NULL)
	{
		count++;
		cur_mal = cur_mal->next;
	}
	printf("Malloced list size = %d\n", count);*/

	if (option_a)
		myFree((void**) &the_freq_node->ptr_value, file_opened_head, malloced_head, the_debug);
	else
		myFreeJustNode((void**) &the_freq_node->ptr_value, file_opened_head, malloced_head, the_debug);
	//printf("Freed just node\n");
	struct ListNodePtr* cur = the_freq_node->row_nums_head;
	while (cur != NULL)
	{
		myFreeJustNode((void**) &cur->ptr_value, file_opened_head, malloced_head, the_debug);
		myFreeJustNode((void**) &cur, file_opened_head, malloced_head, the_debug);
		//printf("Freed just node\n");

		cur = cur->next;
	}
	if (option_a)
		myFree((void**) &the_freq_node, file_opened_head, malloced_head, the_debug);
	else
		myFreeJustNode((void**) &the_freq_node, file_opened_head, malloced_head, the_debug);
	//printf("Freed just node\n");

	/*
	count = 0;
	cur_mal = *malloced_head;
	while (cur_mal != NULL)
	{
		count++;
		cur_mal = cur_mal->next;
	}
	printf("Malloced list size = %d\n", count);*/

	return RETURN_GOOD;
}

int freeMemOfDB(int the_debug)
{
	int total_freed = freeAnyLinkedList((void**) &tables_head, PTR_TYPE_TABLE_INFO, NULL, NULL, the_debug);

	if (the_debug == YES_DEBUG)
		printf("freeMemOfDB() freed %d things\n", total_freed);
	return RETURN_GOOD;
}