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
#include <fcntl.h>
#include <semaphore.h>
#include <sys/mman.h>
#include <ctype.h>

#include "DB_HelperFunctions.h"
#include "DB_SHM_Struct.h"

static char null_string[4] = "#*#\0";
static int_8 null_int = 0x55555555;
static double null_double = 0x55555555;

static int_8 num_tables;
static struct table_info* tables_head;

static struct shmem* shm;


int main(int argc, char *argv[])
{
	shm = NULL;


	// START Initialize the database
		struct malloced_node* malloced_head = NULL;
		int debug = NO_DEBUG;
		
		int initd = initDB(&malloced_head, debug);
		if (initd != RETURN_GOOD)
		{
			printf("There was an problem with database initialization. Please try again.\n");
			exit(EXIT_SUCCESS);
		}
	// END Initialize the database


	// Create shared memory object and set its size to the size of our structure.

	int fd = shm_open("/myshm", O_CREAT | O_RDWR, 0600);
	if (fd == -1)
	{
		printf("shm_open FAILED\n");
		freeMemOfDB(debug);
		exit(EXIT_SUCCESS);
	}

	if (ftruncate(fd, sizeof(struct shmem)) == -1)
	{
		printf("ftruncate FAILED\n");
		shm_unlink("/myshm");
		freeMemOfDB(debug);
		exit(EXIT_SUCCESS);
	}

	// Map the object into the caller's address space.

	shm = mmap(NULL, sizeof(*shm), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	if (shm == MAP_FAILED)
	{
		printf("mmap FAILED\n");
		shm_unlink("/myshm");
		freeMemOfDB(debug);
		exit(EXIT_SUCCESS);
	}

	// Initialize semaphores as process-shared, with value 0.

	if (sem_init(&shm->sem1, 1, 0) == -1)
	{
		printf("sem_init for shm->sem1 FAILED\n");
		shm_unlink("/myshm");
		freeMemOfDB(debug);
		exit(EXIT_SUCCESS);
	}
	if (sem_init(&shm->sem2, 1, 0) == -1)
	{
		printf("sem_init for shm->sem2 FAILED\n");
		shm_unlink("/myshm");
		freeMemOfDB(debug);
		exit(EXIT_SUCCESS);
	}

	do
	{
		// Wait for 'sem1' to be posted by peer before touching shared memory.

		if (sem_wait(&shm->sem1) == -1)
			break;

		// START Execute command
			if (strcmp(shm->keyword, "_stop") == 0)
			{
				sem_post(&shm->sem2);
				break;
			}
			else if (strcmp(shm->keyword, "_re_init") == 0)
			{
				struct table_info* cur = tables_head;
				while (cur->next != NULL)
				{
					cur = cur->next;
				}

				cur->next = myMalloc(sizeof(struct table_info), NULL, &malloced_head, debug);
				if (cur->next == NULL)
				{
					sem_post(&shm->sem2);
					break;
				}

				cur = cur->next;
				cur->next = NULL;

				int table_index = shm->num_tables-1;

				cur->name = myMalloc(sizeof(char) * 32, NULL, &malloced_head, debug);
				if (cur->name == NULL)
				{
					sem_post(&shm->sem2);
					break;
				}
				strcpy(cur->name, shm->shm_table_info_arr[table_index].name);

				cur->keyword = myMalloc(sizeof(char) * 32, NULL, &malloced_head, debug);
				if (cur->keyword == NULL)
				{
					sem_post(&shm->sem2);
					break;
				}
				strcpy(cur->keyword, shm->shm_table_info_arr[table_index].keyword);

				cur->file_number = shm->shm_table_info_arr[table_index].file_number;
				cur->num_cols = shm->shm_table_info_arr[table_index].num_cols;

				
				cur->table_cols_head = myMalloc(sizeof(struct table_cols_info), NULL, &malloced_head, debug);
				if (cur->table_cols_head == NULL)
				{
					sem_post(&shm->sem2);
					break;
				}

				struct table_cols_info* cur_col = cur->table_cols_head;

				int column_index = shm->shm_table_info_arr[table_index].table_cols_starting_index;

				bool need_break = false;

				for (int j=0; j<shm->shm_table_info_arr[table_index].num_cols; j++)
				{
					cur_col->col_name = myMalloc(sizeof(char) * 32, NULL, &malloced_head, debug);
					if (cur_col->col_name == NULL)
					{
						sem_post(&shm->sem2);
						need_break = true;
						break;
					}
					strcpy(cur_col->col_name, shm->shm_table_cols_info_arr[column_index].col_name);

					cur_col->data_type = shm->shm_table_cols_info_arr[column_index].data_type;
					cur_col->max_length = shm->shm_table_cols_info_arr[column_index].max_length;
					cur_col->col_number = shm->shm_table_cols_info_arr[column_index].col_number;
					cur_col->num_rows = shm->shm_table_cols_info_arr[column_index].num_rows;
					cur_col->num_open = shm->shm_table_cols_info_arr[column_index].num_open;
					cur_col->open_list_head = NULL;
					cur_col->open_list_before_tail = NULL;
					cur_col->num_added_insert_open = 0;
					cur_col->unique_list_head = NULL;
					cur_col->unique_list_tail = NULL;
					cur_col->frequent_list_head = NULL;
					cur_col->frequent_list_tail = NULL;
					cur_col->frequent_arr_row_to_node = NULL;

					cur_col->home_table = cur;


					if (j < shm->shm_table_info_arr[table_index].num_cols-1)
					{
						cur_col->next = myMalloc(sizeof(struct table_cols_info), NULL, &malloced_head, debug);
						if (cur_col->next == NULL)
						{
							sem_post(&shm->sem2);
							need_break = true;
							break;
						}
						cur_col = cur_col->next;
					}
					else
						cur_col->next = NULL;

					column_index++;
				}

				if (need_break)
					break;


				// START This function
		            if (cur->table_cols_head->num_rows < MAX_ROWS_FOR_INIT_FREQ_LISTS)
		            {
						if (initFrequentLists(cur, &malloced_head, debug) != RETURN_GOOD)
						{
							sem_post(&shm->sem2);
							break;
						}
		            }
	            // END This function

				num_tables = num_tables+1;

				myFreeAllCleanup(&malloced_head, debug);
			}
			else
			{
				int table_index = 0;
				int column_index = 0;

				struct table_info* cur_table = tables_head;
				while (cur_table != NULL)
				{
					if (strcmp_Upper(cur_table->name, "LIQUOR_LICENSES") != 0
						&& (strcmp(cur_table->keyword, shm->keyword) == 0 || strcmp(cur_table->keyword, "(none)") == 0))
					{
						strcpy(shm->shm_table_info_arr[table_index].name, cur_table->name);
						strcpy(shm->shm_table_info_arr[table_index].keyword, cur_table->keyword);
						shm->shm_table_info_arr[table_index].file_number = cur_table->file_number;
						shm->shm_table_info_arr[table_index].num_cols = cur_table->num_cols;

						shm->shm_table_info_arr[table_index].table_cols_starting_index = column_index;

						struct table_cols_info* cur_col = cur_table->table_cols_head;
						while (cur_col != NULL)
						{
							strcpy(shm->shm_table_cols_info_arr[column_index].col_name, cur_col->col_name);
							shm->shm_table_cols_info_arr[column_index].data_type = cur_col->data_type;
							shm->shm_table_cols_info_arr[column_index].max_length = cur_col->max_length;
							shm->shm_table_cols_info_arr[column_index].col_number = cur_col->col_number;
							shm->shm_table_cols_info_arr[column_index].num_rows = cur_col->num_rows;
							shm->shm_table_cols_info_arr[column_index].num_open = cur_col->num_open;

							shm->shm_table_cols_info_arr[column_index].home_table_index = table_index;


							column_index++;

							cur_col = cur_col->next;
						}

						table_index++;
					}

					cur_table = cur_table->next;
				}

				shm->num_tables = table_index;
			}
		// END Execute command

		// Post 'sem2' to tell the peer that it can now access the modified data in shared memory.

		if (sem_post(&shm->sem2) == -1)
			break;
	}
	while (true);

	// Unlink the shared memory object. Even if the peer process
	//	is still using the object, this is okay. The object will
	//	be removed only after all open references are closed.

	shm_unlink("/myshm");


	// START Teardown the database
		freeMemOfDB(debug);
	// END Initialize the database


	exit(EXIT_SUCCESS);
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

	//printf("num_tables = %d\n", num_tables);
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
	            if (cur->table_cols_head->num_rows < MAX_ROWS_FOR_INIT_FREQ_LISTS)
	            {
					if (initFrequentLists(cur, malloced_head, the_debug) != RETURN_GOOD)
					{
						if (the_debug == YES_DEBUG)
							printf("	ERROR in initDB() at line %d in %s\n", __LINE__, __FILE__);
						return RETURN_ERROR;
					}
	            }
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

		/*struct table_cols_info* cur_col = cur->table_cols_head;
		while (cur_col != NULL)
		{
			printf("	Column name = %s\n", cur_col->col_name);
			printf("	Datatype = %lu\n", cur_col->data_type);
			printf("	Max length = %lu\n", cur_col->max_length);
			printf("	Column number = %lu\n", cur_col->col_number);
			printf("	Number of rows = %lu\n", cur_col->num_rows);
			printf("	Number of open = %lu\n", cur_col->num_open);
			if (cur_col->open_list_head != NULL)
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
			}

			printf("	Column home_table = %s\n", cur_col->home_table->name);

			cur_col = cur_col->next;
		}*/

		cur = cur->next;
	}
    printf("\n");
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
		//printf("num_rows_to_read FROM DISK = %d\n", num_rows_to_read);
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
			if (valid_rows_head != NULL && num_rows_in_result > 0)
				rows_offset = ((*((int_8*) cur->ptr_value)) * (8 + the_col->max_length));

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
		myFileClose(col_data, &file_opened_head, malloced_head, the_debug);
	// START Loop for reading all rows into arr


    if (file_opened_head != NULL)
    {
        if (the_debug == YES_DEBUG)
			printf("getAllColData() did not close all files\n");
        myFileCloseAll(&file_opened_head, malloced_head, the_debug);
	}

	return arr;
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
		//printf("		Adding to freq_head\n");
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
			struct frequent_node* cur_freq = *freq_head;
			while (cur_freq != NULL)
			{
				//printf("	Comparing _%s_ vs _%s_\n", cur_freq->ptr_value, new_data->ptr_value);
				//printf("	Col data type = %d\n", cur_col->data_type);
				if ((new_data == NULL && cur_freq->ptr_value == NULL) 
					|| (new_data != NULL && cur_freq->ptr_value != NULL && equals(cur_freq->ptr_value, cur_col->data_type, new_data->ptr_value, VALUE_EQUALS)))
				{
					//printf("	Found\n");
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
				//printf("		Adding to freq_tail\n");
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
			//printf("option A\n");
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
			//printf("option B\n");
			the_freq_node->next = NULL;
			the_freq_node->prev = NULL;

			cur_col->frequent_list_head = the_freq_node;
			cur_col->frequent_list_tail = the_freq_node;
		}
		else
		{
			//printf("option C\n");
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
				//printf("option D\n");
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
				myFree((void**) &temp->ptr_value, NULL, NULL, the_debug);
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
				//printf("option E\n");
				the_freq_node->next = NULL;
				the_freq_node->prev = NULL;

				cur_col->unique_list_head = the_freq_node;
				cur_col->unique_list_tail = the_freq_node;
			}
			else
			{
				//printf("option F\n");
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