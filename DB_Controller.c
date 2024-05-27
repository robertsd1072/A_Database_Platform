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

#include "DB_Driver.h"
#include "DB_Controller.h"
#include "DB_Tests.h"
#include "DB_HelperFunctions.h"
#include "DB_SHM_Struct.h"


struct shmem *shm;


int main(int argc, char *argv[])
{
	if (argc == 2 && strcmp(argv[1], "_test") == 0)
	{
		if (test_Driver_main() != 0)
			printf("\nTests FAILED\n\n");
		else
			printf("\nTests passed let's goooo\n\n");

		if (test_Driver_teardown(YES_DEBUG) != 0)
			return RETURN_ERROR;
	}
	else
	{
		/* Open the existing shared memory object and map it into the caller's address space. */

		int fd = shm_open("/myshm", O_RDWR, 0);
		if (fd == -1)
		{
			printf("ERROR SERVER CONTACT\n");
			exit(EXIT_SUCCESS);
		}

		shm = mmap(NULL, sizeof(*shm), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
		if (shm == MAP_FAILED)
		{
			printf("ERROR SERVER CONTACT\n");
			exit(EXIT_SUCCESS);
		}

		/* Put input command into shared memory. */

		char *cmd = argv[1];
		strcpy(shm->keyword, cmd);

		/* Tell peer that it can now access shared memory. */

		if (sem_post(&shm->sem1) == -1)
		{
			printf("ERROR SERVER CONTACT\n");
			exit(EXIT_SUCCESS);
		}

		/* Wait until peer says that it has finished accessing the shared memory. */

		if (sem_wait(&shm->sem2) == -1)
		{
			printf("ERROR SERVER CONTACT\n");
			exit(EXIT_SUCCESS);
		}


		// START Do stuff with inputs
			struct malloced_node* malloced_head = NULL;
			int debug = NO_DEBUG;//YES_DEBUG;//


			if (strcmp(argv[1], "_stop") == 0 && argc == 2)
			{
				printf("Stopping\n");

				char shm_dir[100] = {0};
				sprintf(shm_dir, "/myshm_%s_select", shm->keyword);

				char shm_dir_out[100] = {0};
				sprintf(shm_dir_out, "/myshm_%s_select_out", shm->keyword);

				if (shm_open(shm_dir, O_RDWR, 0) != -1)
				{
					shm_unlink(shm_dir);
					shm_unlink(shm_dir_out);
				}
			}
			else if (strcmp(argv[2], "_get_all") == 0 && argc == 3)
			{
				// START
					char shm_dir[100] = {0};
					sprintf(shm_dir, "/myshm_%s_select", shm->keyword);

					/* Open the existing shared memory object and map it into the caller's address space. */

					int fd_1 = shm_open(shm_dir, O_RDWR, 0);
					if (fd_1 == -1)
					{
						printf("shm_open of shm_dir FAILED\n");
						exit(EXIT_SUCCESS);
					}

					// Map the object into the caller's address space.

					struct shmem_select *shm_select = mmap(NULL, sizeof(*shm_select), PROT_READ | PROT_WRITE, MAP_SHARED, fd_1, 0);
					if (shm_select == MAP_FAILED)
					{
						printf("mmap of shm_dir FAILED\n");
						shm_unlink(shm_dir);
						exit(EXIT_SUCCESS);
					}


					int_8 num_rows_left = shm_select->total_num_rows - 200;

					char shm_dir_out[100] = {0};
					sprintf(shm_dir_out, "/myshm_%s_select_out", shm->keyword);

					/* Open the existing shared memory object and map it into the caller's address space. */

					int fd_2 = shm_open(shm_dir_out, O_RDWR, 0);
					if (fd_2 == -1)
					{
						printf("shm_open of shm_dir_out FAILED\n");
						shm_unlink(shm_dir);
						exit(EXIT_SUCCESS);
					}

					// Map the object into the caller's address space.

					struct shmem_col_data_node *shm_select_col_data = mmap(NULL, sizeof(*shm_select_col_data) * num_rows_left * shm_select->num_cols
																			, PROT_READ | PROT_WRITE, MAP_SHARED, fd_2, 0);
					if (shm_select_col_data == MAP_FAILED)
					{
						printf("mmap of shm_dir_out FAILED\n");
						shm_unlink(shm_dir);
						shm_unlink(shm_dir_out);
						exit(EXIT_SUCCESS);
					}
				// END

				int rows_to_get = shm_select->total_num_rows - shm_select->cur_num_rows > 5000 ? 5000 : shm_select->total_num_rows - shm_select->cur_num_rows;

				for (int i=0; i<rows_to_get; i++)
				{
					int_8 row_index = i + shm_select->cur_num_rows - 200;

					for (int_8 j=0; j<shm_select->num_cols; j++)
					{
						printf("%s", shm_select_col_data[(row_index * shm_select->num_cols) + j].data);

						if (j < shm_select->num_cols-1)
							printf(",");
					}
					if (i < rows_to_get-1)
						printf("\n");
				}

				shm_select->cur_num_rows += 5000;

				//printf("\n\nshm_select->cur_num_rows = %lu, shm_select->total_num_rows = %lu\n", shm_select->cur_num_rows, shm_select->total_num_rows);

				if (shm_select->cur_num_rows >= shm_select->total_num_rows)
				{
					//printf("\nunlinking\n");
					shm_unlink(shm_dir);
					shm_unlink(shm_dir_out);
				}
			}
			else if (strcmp(argv[2], "_create_csv") == 0 && argc == 6)
			{
				if (!(strcmp(shm->keyword, "(none)") == 0 || strcmp(shm->keyword, "_stop") == 0 || strcmp(shm->keyword, "_re_init") == 0))
				{
					char shm_dir[100] = {0};
					sprintf(shm_dir, "/myshm_%s_select", shm->keyword);

					char shm_dir_out[100] = {0};
					sprintf(shm_dir_out, "/myshm_%s_select_out", shm->keyword);

					if (shm_open(shm_dir, O_RDWR, 0) != -1)
					{
						shm_unlink(shm_dir);
						shm_unlink(shm_dir_out);
					}


					if (access(argv[3], F_OK) == 0)
					{
						if (copyDBfromSHM(shm, &malloced_head, debug) != RETURN_GOOD)
						{
							printf("There was an problem with database initialization. Please try again.\n");
							exit(EXIT_SUCCESS);
						}

						char str[10000] = {0};
						sprintf(str, "Creating table from file:\"%s\" with table_name = \"%s\" and num_rows = %s\n", argv[3], argv[4], argv[5]);
						if (addInputToLog(str, &malloced_head, debug) != RETURN_GOOD)
						{
							freeMemOfDB(debug);
							exit(EXIT_SUCCESS);
						}

						char* table_name = (char*) myMalloc(sizeof(char) * 32, NULL, &malloced_head, debug);
						if (table_name == NULL)
						{
							printf("There was an error creating the table. Please try again.\n");
							addInputToLog("\tERROR\n", &malloced_head, debug);
							freeMemOfDB(debug);
							exit(EXIT_SUCCESS);
						}
						strcpy(table_name, argv[4]);
						
						int result = createTableFromCSV(argv[3], table_name, atoi(argv[5]), argv[1], &malloced_head, debug);
						if (result > 0)
						{
							printf("Created table and inserted %d rows\n", result);
							addInputToLog("\tvalid table\n", &malloced_head, debug);
						}
						else
						{
							printf("There was an error creating the table. Please try again.\n");
							addInputToLog("\tERROR\n", &malloced_head, debug);
							freeMemOfDB(debug);
							exit(EXIT_SUCCESS);
						}


						//traverseTablesInfoMemory();

						//traverseTablesInfoDisk(&malloced_head, debug);


						copyToSHMfromDB(&malloced_head, debug);

						//printSHM(shm);


						strcpy(shm->keyword, "_re_init");

						/* Tell peer that it can now access shared memory. */

						if (sem_post(&shm->sem1) == -1)
						{
							printf("ERROR SERVER CONTACT\n");
							freeMemOfDB(debug);
							exit(EXIT_SUCCESS);
						}


						freeMemOfDB(debug);
					}
				}
			}
			else if (argc == 3)
			{
				char shm_dir[100] = {0};
				sprintf(shm_dir, "/myshm_%s_select", shm->keyword);

				char shm_dir_out[100] = {0};
				sprintf(shm_dir_out, "/myshm_%s_select_out", shm->keyword);

				if (shm_open(shm_dir, O_RDWR, 0) != -1)
				{
					shm_unlink(shm_dir);
					shm_unlink(shm_dir_out);
				}


				if (copyDBfromSHM(shm, &malloced_head, debug) != RETURN_GOOD)
				{
					printf("There was an problem with database initialization. Please try again.\n");
					exit(EXIT_SUCCESS);
				}

				parseInput(argv[2], &malloced_head, debug);


				freeMemOfDB(debug);
			}
		// END Do stuff with inputs


		exit(EXIT_SUCCESS);
	}
}


int parseInput(char* input, struct malloced_node** malloced_head, int the_debug)
{
	char* cmd = substring(input, 0, 5, NULL, malloced_head, the_debug);
	if (cmd == NULL)
	{
		return RETURN_ERROR;
	}
	//printf("cmd = _%s_\n", cmd);
	
	if (strcmp_Upper(cmd, "_GET_T") == 0)
	{
		myFree((void**) &cmd, NULL, malloced_head, the_debug);

		struct table_info* tables_head = getTablesHead();
		while (tables_head != NULL)
		{
			printf(tables_head->name);
			printf(":");

			struct table_cols_info* cur_col = tables_head->table_cols_head;
			while (cur_col != NULL)
			{
				printf(cur_col->col_name);
				printf("|");

				if (cur_col->data_type == DATA_INT)
					printf("integer");
				else if (cur_col->data_type == DATA_REAL)
					printf("real");
				else if (cur_col->data_type == DATA_STRING)
					printf("char(%d)", cur_col->max_length-1);
				else if (cur_col->data_type == DATA_DATE)
					printf("date");

				if (cur_col->next != NULL)
					printf(",");

				cur_col = cur_col->next;
			}

			if (tables_head->next != NULL)
				printf(";");

			tables_head = tables_head->next;
		}
		printf("\n");
	}
	else if (strcmp_Upper(cmd, "SELECT") == 0)
	{
		myFree((void**) &cmd, NULL, malloced_head, the_debug);
		return selectAndPrint(input, malloced_head, the_debug);
	}
	else if (strcmp_Upper(cmd, "INSERT") == 0)
	{
		myFree((void**) &cmd, NULL, malloced_head, the_debug);
		printf("Inserting does not currently work. Please try again later");
		//return insertAndPrint(input, malloced_head, the_debug);
	}
	else if (strcmp_Upper(cmd, "UPDATE") == 0)
	{
		myFree((void**) &cmd, NULL, malloced_head, the_debug);
		printf("Updating does not currently work. Please try again later");
		//return updateAndPrint(input, malloced_head, the_debug);
	}
	else if (strcmp_Upper(cmd, "DELETE") == 0)
	{
		myFree((void**) &cmd, NULL, malloced_head, the_debug);
		printf("Deleting does not currently work. Please try again later");
		//return deleteAndPrint(input, malloced_head, the_debug);
	}
	else
	{
		myFree((void**) &cmd, NULL, malloced_head, the_debug);

		printf("The first was word of the command was not one of the following: SELECT, INSERT, UPDATE, or DELETE.\n");
		printf("There was an problem parsing the command. Please try again.");
	}


	return RETURN_GOOD;
}

int selectAndPrint(char* input, struct malloced_node** malloced_head, int the_debug)
{
	addInputToLog(input, malloced_head, the_debug);
	addInputToLog("\n", malloced_head, the_debug);

	struct select_node* select_node = NULL;
	if (parseSelect(input, &select_node, NULL, false, malloced_head, the_debug) != RETURN_GOOD)
	{
		addInputToLog("\tERROR\n", malloced_head, the_debug);
		printf("There was an problem parsing the command. Please try again.");
		return RETURN_ERROR;
	}

	
	if (selectStuff(&select_node, false, malloced_head, the_debug) != RETURN_GOOD)
	{
		addInputToLog("\tERROR\n", malloced_head, the_debug);
		printf("There was an problem retrieving the data. This was reported to the developer. Please try again.");
		return RETURN_ERROR;
	}


	// START Write number of rows fetched
		int_8 num_rows_to_print = select_node->columns_arr[0]->num_rows > 200 ? 200 : select_node->columns_arr[0]->num_rows;

		printf("%lu\n", select_node->columns_arr[0]->num_rows);
	// END Write number of rows fetched


	// START Write column names
		for (int j=0; j<select_node->columns_arr_size; j++)
		{
			char* the_col_name;

			if (select_node->columns_arr[j]->new_name != NULL)
				the_col_name = select_node->columns_arr[j]->new_name;
			else
			{
				struct col_in_select_node* cur = select_node->columns_arr[j];
				while (cur->col_ptr_type == PTR_TYPE_COL_IN_SELECT_NODE)
					cur = cur->col_ptr;
				
				the_col_name = ((struct table_cols_info*) cur->col_ptr)->col_name;
			}

			printf(the_col_name);

			if (j < select_node->columns_arr_size-1)
				printf(",");
			else if (num_rows_to_print > 0)
				printf("\n");
		}
	// END Write column names


	// START Write column data
		for (int i=0; i<num_rows_to_print; i++)
		{
			for (int j=0; j<select_node->columns_arr_size; j++)
			{
				if (select_node->columns_arr[j]->col_data_arr[i]->row_data != NULL)
				{
					int the_data_type = select_node->columns_arr[j]->rows_data_type;

					if (the_data_type == DATA_INT)
						printf("%d", *((int*) select_node->columns_arr[j]->col_data_arr[i]->row_data));
					else if (the_data_type == DATA_REAL)
						printf("%lf", *((double*) select_node->columns_arr[j]->col_data_arr[i]->row_data));
					else
						printf("%s", select_node->columns_arr[j]->col_data_arr[i]->row_data);
				}

				if (j < select_node->columns_arr_size-1)
					printf(",");
			}
			if (i < num_rows_to_print-1)
				printf("\n");
		}
	// END Write column data


	if (addInputToLog("\tvalid table\n", malloced_head, the_debug) != RETURN_GOOD)
	{
		return RETURN_ERROR;
	}


	// START Create shared memory for extra rows for later access if necessary
		if (select_node->columns_arr[0]->num_rows > 200)
		{
			char shm_dir[100] = {0};
			sprintf(shm_dir, "/myshm_%s_select", shm->keyword);

			// Create shared memory object and set its size to the size of our structure.

			int fd_1 = shm_open(shm_dir, O_CREAT | O_RDWR, 0600);
			if (fd_1 == -1)
			{
				printf("shm_open of shm_dir FAILED\n");
				return RETURN_ERROR;
			}

			if (ftruncate(fd_1, sizeof(struct shmem_select)) == -1)
			{
				printf("ftruncate of shm_dir FAILED\n");
				shm_unlink(shm_dir);
				return RETURN_ERROR;
			}

			// Map the object into the caller's address space.

			struct shmem_select *shm_select = mmap(NULL, sizeof(*shm_select), PROT_READ | PROT_WRITE, MAP_SHARED, fd_1, 0);
			if (shm_select == MAP_FAILED)
			{
				printf("mmap of shm_dir FAILED\n");
				shm_unlink(shm_dir);
				return RETURN_ERROR;
			}

			// START Input data to shm_select
				shm_select->cur_num_rows = 200;
				shm_select->total_num_rows = select_node->columns_arr[0]->num_rows;
				shm_select->num_cols = select_node->columns_arr_size;

				int_8 num_rows_left = select_node->columns_arr[0]->num_rows - 200;
			// END Input data to shm_select


			char shm_dir_out[100] = {0};
			sprintf(shm_dir_out, "/myshm_%s_select_out", shm->keyword);

			// Create shared memory object and set its size to the size of our structure.

			int fd_2 = shm_open(shm_dir_out, O_CREAT | O_RDWR, 0600);
			if (fd_2 == -1)
			{
				printf("shm_open of shm_dir_out FAILED\n");
				shm_unlink(shm_dir);
				return RETURN_ERROR;
			}

			if (ftruncate(fd_2, sizeof(struct shmem_col_data_node) * num_rows_left * select_node->columns_arr_size) == -1)
			{
				printf("ftruncate of shm_dir_out FAILED\n");
				shm_unlink(shm_dir);
				shm_unlink(shm_dir_out);
				return RETURN_ERROR;
			}

			// Map the object into the caller's address space.

			struct shmem_col_data_node *shm_select_col_data = mmap(NULL, sizeof(*shm_select_col_data) * num_rows_left * select_node->columns_arr_size
																	, PROT_READ | PROT_WRITE, MAP_SHARED, fd_2, 0);
			if (shm_select_col_data == MAP_FAILED)
			{
				printf("mmap of shm_dir_out FAILED\n");
				shm_unlink(shm_dir);
				shm_unlink(shm_dir_out);
				return RETURN_ERROR;
			}


			// START Input data to shm_select_col_data
				for (int_8 i=0; i<num_rows_left * select_node->columns_arr_size; i++)
				{
					int_8 row_index = (i / select_node->columns_arr_size) + 200;
					int_8 col_index = i % select_node->columns_arr_size;

					//if (i < 49)
					//	printf("i = %lu, row_index = %lu, col_index = %lu\n", i, row_index, col_index);

					if (select_node->columns_arr[col_index]->rows_data_type == DATA_INT)
					{
						//if (i < 49)
						//	printf("Adding %d to shm\n", *((int*) select_node->columns_arr[col_index]->col_data_arr[row_index]->row_data));
						sprintf(shm_select_col_data[i].data, "%d", *((int*) select_node->columns_arr[col_index]->col_data_arr[row_index]->row_data));
					}
					else if (select_node->columns_arr[col_index]->rows_data_type == DATA_REAL)
					{
						//if (i < 49)
						//	printf("Adding %lf to shm\n", *((double*) select_node->columns_arr[col_index]->col_data_arr[row_index]->row_data));
						sprintf(shm_select_col_data[i].data, "%lf", *((double*) select_node->columns_arr[col_index]->col_data_arr[row_index]->row_data));
					}
					else
					{
						//if (i < 49)
						//	printf("Adding %s to shm\n", select_node->columns_arr[col_index]->col_data_arr[row_index]->row_data);
						strcpy(shm_select_col_data[i].data, select_node->columns_arr[col_index]->col_data_arr[row_index]->row_data);
					}
					//if (i < 49)
					//	printf("In shm: (%s)\n", shm_select_col_data[i].data);
				}
			// END Input data to shm_select_col_data


			//shm_unlink(shm_dir);
			//shm_unlink(shm_dir_out);
			//printf("Here\n");
		}
	// END Create shared memory for extra rows for later access if necessary


	// START Free stuff
		int_8 total_freed = 0;
		for (int j=0; j<select_node->columns_arr_size; j++)
		{
			for (int i=0; i<select_node->columns_arr[j]->num_rows; i++)
			{
				myFree((void**) &select_node->columns_arr[j]->col_data_arr[i], NULL, malloced_head, the_debug);
				total_freed++;
			}
			myFree((void**) &select_node->columns_arr[j]->col_data_arr, NULL, malloced_head, the_debug);
			total_freed++;

			if (select_node->columns_arr[j]->unique_values_head != NULL)
			{
				int temp_freed = freeAnyLinkedList((void**) &select_node->columns_arr[j]->unique_values_head, PTR_TYPE_LIST_NODE_PTR, NULL, malloced_head, the_debug);
				if (the_debug == YES_DEBUG)
					printf("Freed %d from col's unique_values_head\n", temp_freed);
				total_freed += temp_freed;
			}
		}

		while (select_node->prev != NULL)
			select_node = select_node->prev;

		freeAnyLinkedList((void**) &select_node, PTR_TYPE_SELECT_NODE, NULL, malloced_head, the_debug);
	// END Free stuff

	return RETURN_GOOD;
}

int insertAndPrint(char* input, struct malloced_node** malloced_head, int the_debug)
{
	addInputToLog(input, malloced_head, the_debug);
	addInputToLog("\n", malloced_head, the_debug);

	struct change_node_v2* change_head = NULL;
	if (parseInsert(input, &change_head, malloced_head, the_debug) != RETURN_GOOD)
	{
		addInputToLog("\tERROR\n", malloced_head, the_debug);
		printf("There was an problem parsing the command. Please try again.");
		return RETURN_ERROR;
	}


	int rows_inserted = insertRows(change_head, malloced_head, the_debug);
	if (rows_inserted < 0)
	{
		addInputToLog("\tERROR\n", malloced_head, the_debug);
		printf("There was an problem inserting the data. This was reported to the developer. Please try again.");
		return RETURN_ERROR;
	}

	addInputToLog("\tgood\n", malloced_head, the_debug);
	freeAnyLinkedList((void**) &change_head, PTR_TYPE_CHANGE_NODE_V2, NULL, malloced_head, the_debug);

	printf("Inserted %d rows.", rows_inserted);

	return RETURN_GOOD;
}

int updateAndPrint(char* input, struct malloced_node** malloced_head, int the_debug)
{
	addInputToLog(input, malloced_head, the_debug);
	addInputToLog("\n", malloced_head, the_debug);

	struct change_node_v2* change_head = NULL;
	if (parseUpdate(input, &change_head, malloced_head, the_debug) != RETURN_GOOD)
	{
		addInputToLog("\tERROR\n", malloced_head, the_debug);
		printf("There was an problem parsing the command. Please try again.");
		return RETURN_ERROR;
	}


	int num_updated = updateRows(change_head, malloced_head, the_debug);
	if (num_updated < 0)
	{
		addInputToLog("\tERROR\n", malloced_head, the_debug);
		printf("There was an problem updating the data. This was reported to the developer. Please try again.");
		return RETURN_ERROR;
	}
	
	addInputToLog("\tgood\n", malloced_head, the_debug);
	freeAnyLinkedList((void**) &change_head, PTR_TYPE_CHANGE_NODE_V2, NULL, malloced_head, the_debug);

	printf("Updated %d rows.", num_updated);

	return RETURN_GOOD;
}

int deleteAndPrint(char* input, struct malloced_node** malloced_head, int the_debug)
{
	addInputToLog(input, malloced_head, the_debug);
	addInputToLog("\n", malloced_head, the_debug);

	struct change_node_v2* change_head = NULL;
	if (parseDelete(input, &change_head, malloced_head, the_debug) != 0)
	{
		addInputToLog("\tERROR\n", malloced_head, the_debug);
		printf("There was an problem parsing the command. Please try again.");
		return RETURN_ERROR;
	}


	int num_deleted = deleteRows(change_head, malloced_head, the_debug);
	if (num_deleted < 0)
	{
		addInputToLog("\tERROR\n", malloced_head, the_debug);
		printf("There was an problem deleting the data. This was reported to the developer. Please try again.");
		return RETURN_ERROR;
	}

	addInputToLog("\tgood\n", malloced_head, the_debug);
	freeAnyLinkedList((void**) &change_head, PTR_TYPE_CHANGE_NODE_V2, NULL, malloced_head, the_debug);

	printf("Deleted %d rows.", num_deleted);

	return RETURN_ERROR;
}

int addInputToLog(char* input, struct malloced_node** malloced_head, int the_debug)
{
	struct file_opened_node* file_opened_head = NULL;

	FILE* log_file = myFileOpenSimple("Log.txt", "a+", &file_opened_head, malloced_head, the_debug);
	if (log_file == NULL)
	{
		if (the_debug == YES_DEBUG)
			printf("	ERROR in addInputToLog() at line %d in %s\n", __LINE__, __FILE__);
		return RETURN_ERROR;
	}

	if (fwrite(input, strLength(input), 1, log_file) != 1)
	{
		if (the_debug == YES_DEBUG)
			printf("	ERROR in addInputToLog() at line %d in %s\n", __LINE__, __FILE__);
		return RETURN_ERROR;
	}

	myFileClose(log_file, &file_opened_head, malloced_head, the_debug);

	if (file_opened_head != NULL)
    {
        if (the_debug == YES_DEBUG)
			printf("addInputToLog() did not close all files\n");
        myFileCloseAll(&file_opened_head, malloced_head, the_debug);
    }

	return RETURN_GOOD;
}

int copyToSHMfromDB(struct malloced_node** malloced_head, int the_debug)
{
	int table_index = shm->num_tables;
	int column_index = 0;

	struct table_info* cur = getTablesHead();
	while (cur->next != NULL)
	{
		column_index += cur->num_cols;
		cur = cur->next;
	}

	if (the_debug == YES_DEBUG)
	{
		printf("table_index = %d\n", table_index);
		printf("column_index = %d\n", column_index);
		printf("new table name = _%s_\n", cur->name);
	}

	strcpy(shm->shm_table_info_arr[table_index].name, cur->name);
	strcpy(shm->shm_table_info_arr[table_index].keyword, cur->keyword);

	shm->shm_table_info_arr[table_index].file_number = cur->file_number;
	shm->shm_table_info_arr[table_index].num_cols = cur->num_cols;

	shm->shm_table_info_arr[table_index].table_cols_starting_index = column_index;


	struct table_cols_info* cur_col = cur->table_cols_head;
	while (cur_col != NULL)
	{
		if (the_debug == YES_DEBUG)
			printf("new column name = _%s_\n", cur_col->col_name);

		strcpy(shm->shm_table_cols_info_arr[column_index].col_name, cur_col->col_name);

		shm->shm_table_cols_info_arr[column_index].data_type = cur_col->data_type;
		shm->shm_table_cols_info_arr[column_index].max_length = cur_col->max_length;
		shm->shm_table_cols_info_arr[column_index].col_number = cur_col->col_number;
		shm->shm_table_cols_info_arr[column_index].num_rows = cur_col->num_rows;
		shm->shm_table_cols_info_arr[column_index].num_open = cur_col->num_open;

		column_index++;

		cur_col = cur_col->next;
	}


	shm->num_tables = shm->num_tables+1;

	return RETURN_GOOD;
}

int printSHM(struct shmem* shm)
{
	for (int i=0; i<shm->num_tables; i++)
	{
		printf("table name = %s\n", shm->shm_table_info_arr[i].name);
		printf("table keyword = %s\n", shm->shm_table_info_arr[i].keyword);

		printf("table file_number = %lu\n", shm->shm_table_info_arr[i].file_number);
		printf("table num_cols = %lu\n", shm->shm_table_info_arr[i].num_cols);

		printf("table table_cols_starting_index = %d\n", shm->shm_table_info_arr[i].table_cols_starting_index);


		int column_index = shm->shm_table_info_arr[i].table_cols_starting_index;
		for (int j=0; j<shm->shm_table_info_arr[i].num_cols; j++)
		{
			printf("	column name = %s\n", shm->shm_table_cols_info_arr[column_index].col_name);
			printf("	column data_type = %lu\n", shm->shm_table_cols_info_arr[column_index].data_type);
			printf("	column max_length = %lu\n", shm->shm_table_cols_info_arr[column_index].max_length);
			printf("	column col_number = %lu\n", shm->shm_table_cols_info_arr[column_index].col_number);
			printf("	column num_rows = %lu\n", shm->shm_table_cols_info_arr[column_index].num_rows);
			printf("	column num_open = %lu\n", shm->shm_table_cols_info_arr[column_index].num_open);

			column_index++;
		}
	}

	return RETURN_GOOD;
}


/*	RETURNS:
 *  RETURN_ERROR if error
 *	Integer greater than -1 if good
 *
 *	WRITES TO:
 */
int createTableFromCSV(char* input, char* table_name, int_8 num_rows, char* keyword, struct malloced_node** malloced_head, int the_debug)
{
	char* table_name_copy = (char*) myMalloc(sizeof(char) * 32, NULL, malloced_head, the_debug);
	if (table_name_copy == NULL)
	{
		if (the_debug == YES_DEBUG)
			printf("	ERROR in createTableFromCSV() at line %d in %s\n", __LINE__, __FILE__);
		return RETURN_ERROR;
	}
	strcpy(table_name_copy, table_name);
	myFreeJustNode((void**) &table_name_copy, NULL, malloced_head, the_debug);


	char* keyword_copy = (char*) myMalloc(sizeof(char) * 32, NULL, malloced_head, the_debug);
	if (keyword_copy == NULL)
	{
		if (the_debug == YES_DEBUG)
			printf("	ERROR in createTableFromCSV() at line %d in %s\n", __LINE__, __FILE__);
		return RETURN_ERROR;
	}
	strcpy(keyword_copy, keyword);


	struct file_opened_node* file_opened_head = NULL;
	
	// START Check if input file exists
	    if (access(input, F_OK) != 0)
	    {
	    	if (the_debug == YES_DEBUG)
				printf("	ERROR in createTableFromCSV() at line %d in %s\n", __LINE__, __FILE__);
			errorTeardown(&file_opened_head, malloced_head, the_debug);
			return RETURN_ERROR;
	    }
    // END Check if input file exists

	// START Open csv file and read in first row of column names and datatypes
	    FILE* file = myFileOpenSimple(input, "r", &file_opened_head, malloced_head, the_debug);
		if (file == NULL)
		{
			if (the_debug == YES_DEBUG)
			{
				printf("	ERROR in createTableFromCSV() at line %d in %s\n", __LINE__, __FILE__);
				printf("The input file could not be opened. Please try again\n");
			}
			return RETURN_ERROR;
		}

		char* first_row = (char*) myMalloc(sizeof(char) * 10000, &file_opened_head, malloced_head, the_debug);
		if (first_row == NULL)
		{
			if (the_debug == YES_DEBUG)
				printf("	ERROR in createTableFromCSV() at line %d in %s\n", __LINE__, __FILE__);
			return RETURN_ERROR;
		}
		fscanf(file, "%s\n", first_row);
		myFileClose(file, &file_opened_head, malloced_head, the_debug);
    // END Open csv file and read in first row of column names and datatypes
	
	// START Allocate space for beginning of table_cols
	    struct table_cols_info* table_cols = (struct table_cols_info*) myMalloc(sizeof(struct table_cols_info), &file_opened_head, malloced_head, the_debug);
		if (table_cols == NULL)
		{
			if (the_debug == YES_DEBUG)
				printf("	ERROR in createTableFromCSV() at line %d in %s\n", __LINE__, __FILE__);
			return RETURN_ERROR;
		}
		struct table_cols_info* cur_col = table_cols;
    // END Allocate space for beginning of table_cols
	
	// START Split first_row by each comma and extract column name and datatype
		int num_cols = 0;
		char** col_names = strSplitV2(first_row, ',', &num_cols, &file_opened_head, malloced_head, the_debug);
		if (col_names == NULL)
		{
			if (the_debug == YES_DEBUG)
				printf("	ERROR in createTableFromCSV() at line %d in %s\n", __LINE__, __FILE__);
			errorTeardown(&file_opened_head, malloced_head, the_debug);
			return RETURN_ERROR;
		}
		myFree((void**) &first_row, &file_opened_head, malloced_head, the_debug);

		for (int i=0; i<num_cols; i++)
		{
			char* temp_but_is_col_name = substring(col_names[i], 0, indexOf(col_names[i], ':')-1, &file_opened_head, malloced_head, the_debug);
			char* temp2 = substring(col_names[i], indexOf(col_names[i], ':')+1, indexOf(col_names[i], 0), &file_opened_head, malloced_head, the_debug);
			if (temp_but_is_col_name == NULL || temp2 == NULL)
			{
				if (the_debug == YES_DEBUG)
					printf("	ERROR in createTableFromCSV() at line %d in %s\n", __LINE__, __FILE__);
				errorTeardown(&file_opened_head, malloced_head, the_debug);
				return RETURN_ERROR;
			}
			
			// START If null terminator is at index > 32 (col name is longer than 31 chars), throw error, return
			if (indexOf(temp_but_is_col_name, 0) > 32)
			{
				if (the_debug == YES_DEBUG)
				{
					printf("	ERROR in createTableFromCSV() at line %d in %s\n", __LINE__, __FILE__);
					printf("The column \"%s\" was longer than 31 characters. Please shorten it.\n", temp_but_is_col_name);
				}
				errorTeardown(&file_opened_head, malloced_head, the_debug);
				return -11;
			}
			// END If null terminator is at index > 32 (col name is longer than 31 chars), throw error, return
			
			cur_col->col_name = temp_but_is_col_name;
			for (int a=0; a<256; a++)
			{
				if (a <= 47 || (a >= 58 && a <= 64) || (a >= 91 && a != 95 && a <= 96) || a >= 123)
				{
					if (strcontains(cur_col->col_name, a))
					{
						if (the_debug == YES_DEBUG)
						{
							printf("	ERROR in createTableFromCSV() at line %d in %s\n", __LINE__, __FILE__);
							printf("The column \"%s\" contained a special character (%c) that was not allowed. Please follow the guidelines to declare a column.\n", temp_but_is_col_name, a);
						}
						errorTeardown(&file_opened_head, malloced_head, the_debug);
						return -11;
					}
				}
			}

			cur_col->col_number = i;

			//printf("cur_col->col_name = %s\n", cur_col->col_name);
			//printf("cur_col->col_number = %d\n", cur_col->col_number);
			
			char* datatype = substring(temp2, 0, 4, &file_opened_head, malloced_head, the_debug);
			if (datatype == NULL)
			{
				if (the_debug == YES_DEBUG)
					printf("	ERROR in createTableFromCSV() at line %d in %s\n", __LINE__, __FILE__);
				return RETURN_ERROR;
			}
			datatype[4] = 0;

			if (the_debug == YES_DEBUG)
				printf("datatype = _%s_\n", datatype);
			if (strcmp_Upper(datatype, "INTE") == 0)
			{
				cur_col->data_type = DATA_INT;
				cur_col->max_length = 8;
			}
			else if (strcmp_Upper(datatype, "REAL") == 0)
			{
				cur_col->data_type = DATA_REAL;
				cur_col->max_length = 8;
			}
			else if (strcmp_Upper(datatype, "CHAR") == 0)
			{
				cur_col->data_type = DATA_STRING;
				cur_col->max_length = 0;
				sscanf(temp2, "%*[^(](%lu)", &cur_col->max_length);
				if (cur_col->max_length == 0)
				{
					if (the_debug == YES_DEBUG)
					{
						printf("	ERROR in createTableFromCSV() at line %d in %s\n", __LINE__, __FILE__);
						printf("The datatype for column \"%s\" was not given a length, or the length is 0. Please specify a length.\n", cur_col->col_name);
					}
					errorTeardown(&file_opened_head, malloced_head, the_debug);
					return -12;
				}
				cur_col->max_length++;
			}
			else if (strcmp_Upper(datatype, "DATE") == 0)
			{
				cur_col->data_type = DATA_DATE;
				cur_col->max_length = 8;
			}
			else
			{
				if (the_debug == YES_DEBUG)
				{
					printf("	ERROR in createTableFromCSV() at line %d in %s\n", __LINE__, __FILE__);
					printf("A datatype for column \"%s\" was not specified. Please follow the guidelines to declare a datatype.\n", temp2);
				}
				errorTeardown(&file_opened_head, malloced_head, the_debug);
				return -12;
			}

			myFree((void**) &temp2, &file_opened_head, malloced_head, the_debug);
			myFree((void**) &datatype, &file_opened_head, malloced_head, the_debug);
			myFree((void**) &col_names[i], &file_opened_head, malloced_head, the_debug);

			// START Allocate another struct table_cols_info is more cols, else, make next null
			if (i < num_cols-1)
			{
				cur_col->next = (struct table_cols_info*) myMalloc(sizeof(struct table_cols_info), &file_opened_head, malloced_head, the_debug);
				if (cur_col->next == NULL)
				{
					if (the_debug == YES_DEBUG)
						printf("	ERROR in createTableFromCSV() at line %d in %s\n", __LINE__, __FILE__);
					return RETURN_ERROR;
				}
				cur_col = cur_col->next;
			}
			else
				cur_col->next = NULL;
			// END Allocate another struct table_cols_info is more cols, else, make next null
		}

		myFree((void**) &col_names, &file_opened_head, malloced_head, the_debug);
    // END Split first_row by each comma and extract column name and datatype


	// START Check for duplicate
		struct table_info* cur_temp_table = getTablesHead();
		while (cur_temp_table != NULL)
		{
			if (strcmp_Upper(table_name, cur_temp_table->name) == 0 && strcmp_Upper(keyword_copy, cur_temp_table->keyword) == 0)
			{
				if (the_debug == YES_DEBUG)
					printf("	ERROR in createTableFromCSV() at line %d in %s\n", __LINE__, __FILE__);
				printf("A table with the same name already exists for your keyword.\n");
				errorTeardown(&file_opened_head, malloced_head, the_debug);
				return RETURN_ERROR;
			}

			cur_temp_table = cur_temp_table->next;
		}
	// END Check for duplicate


	// START Call createTable with parsed table_cols
		int created_table = createTable(table_name, table_cols, keyword_copy, malloced_head, the_debug);
		if (created_table != RETURN_GOOD)
		{
			if (the_debug == YES_DEBUG)
				printf("Table creation had a problem.\n");
			// myFreeAllCleanup() is called by createTable so this pointer's malloced node will be freed but not its data pointer
			deleteTable(table_name_copy, malloced_head, the_debug);
			myFree((void**) &table_name_copy, NULL, NULL, the_debug);
	        return RETURN_ERROR;
		}
		else if (the_debug == YES_DEBUG)
			printf("Successfully created table\n");
	// END Call createTable with parsed table_cols


	// START Find table in tables_head
		struct table_info* table = getTablesHead();
		while (table != NULL)
		{
			if (strcmp(table->name, table_name) == 0)
				break;
			table = table->next;
		}
		if (table == NULL)
		{
			if (the_debug == YES_DEBUG)
				printf("	ERROR in createTableFromCSV() at line %d in %s\n", __LINE__, __FILE__);
			errorTeardown(&file_opened_head, malloced_head, the_debug);
			// myFreeAllCleanup() is called by createTable so this pointer's malloced node will be freed but not its data pointer
			deleteTable(table_name_copy, malloced_head, the_debug);
			myFree((void**) &table_name_copy, NULL, NULL, the_debug);
			return -14;
		}
    // END Find table in tables_head

	// START Reopen file
		file = myFileOpenSimple(input, "r", &file_opened_head, malloced_head, the_debug);
		if (file == NULL)
		{
			if (the_debug == YES_DEBUG)
				printf("	ERROR in createTableFromCSV() at line %d in %s\n", __LINE__, __FILE__);
			// myFreeAllCleanup() is called by createTable so this pointer's malloced node will be freed but not its data pointer
			deleteTable(table_name_copy, malloced_head, the_debug);
			myFree((void**) &table_name_copy, NULL, NULL, the_debug);
			return RETURN_ERROR;
		}

		first_row = (char*) myMalloc(sizeof(char) * 10000, &file_opened_head, malloced_head, the_debug);
		if (first_row == NULL)
		{
			if (the_debug == YES_DEBUG)
				printf("	ERROR in createTableFromCSV() at line %d in %s\n", __LINE__, __FILE__);
			// myFreeAllCleanup() is called by createTable so this pointer's malloced node will be freed but not its data pointer
			deleteTable(table_name_copy, malloced_head, the_debug);
			myFree((void**) &table_name_copy, NULL, NULL, the_debug);
			return RETURN_ERROR;
		}
		fscanf(file, "%s\n", first_row);
		myFree((void**) &first_row, &file_opened_head, malloced_head, the_debug);
	// END Reopen file
    
	// START Allocate arrays for files opened
		FILE** col_data_info_file_arr = (FILE**) myMalloc(sizeof(FILE*) * table->num_cols, &file_opened_head, malloced_head, the_debug);
		FILE** col_data_file_arr = (FILE**) myMalloc(sizeof(FILE*) * table->num_cols, &file_opened_head, malloced_head, the_debug);
	    if (col_data_info_file_arr == NULL || col_data_file_arr == NULL)
	    {
	        if (the_debug == YES_DEBUG)
				printf("	ERROR in createTableFromCSV() at line %d in %s\n", __LINE__, __FILE__);
			// myFreeAllCleanup() is called by createTable so this pointer's malloced node will be freed but not its data pointer
			deleteTable(table_name_copy, malloced_head, the_debug);
			myFree((void**) &table_name_copy, NULL, NULL, the_debug);
	        return RETURN_ERROR;
	    }

		for (int i=0; i<table->num_cols; i++)
		{
			col_data_info_file_arr[i] = NULL;
			col_data_file_arr[i] = NULL;
		}
	// END Allocate arrays for files opened

    // START For each row, extract each column value and insert append it
		for (int i=0; i<num_rows; i++)
		{
			if (i % 10000 == 0)
			{
				if (the_debug == YES_DEBUG)
					printf("In progress... inserted %d rows so far\n", i);
			}
			//printf("i = %d\n", i);
	        struct table_cols_info* cur_col = table->table_cols_head;
			char* a_line = (char*) myMalloc(sizeof(char) * 10000, &file_opened_head, malloced_head, the_debug);
			if (a_line == NULL)
			{
				if (the_debug == YES_DEBUG)
					printf("	ERROR in createTableFromCSV() at line %d in %s\n", __LINE__, __FILE__);
				// myFreeAllCleanup() is called by createTable so this pointer's malloced node will be freed but not its data pointer
				deleteTable(table_name_copy, malloced_head, the_debug);
				myFree((void**) &table_name_copy, NULL, NULL, the_debug);
		        return RETURN_ERROR;
			}

			if (fscanf(file, "%[^\n]\n", a_line) == EOF)
			{
				printf("Found EOF at i = %d\n", i);
				myFree((void**) &a_line, &file_opened_head, malloced_head, the_debug);
				break;
			}

			// START Check for extra comma
				int size_result = 0;
				char** num_cols_in_line = strSplitV2(a_line, ',', &size_result, &file_opened_head, malloced_head, the_debug);

				if (size_result != table->num_cols)
				{
					if (the_debug == YES_DEBUG)
						printf("	ERROR in createTableFromCSV() at line %d in %s\n", __LINE__, __FILE__);
					printf("The line in row %d had either more or less values than columns. Number of values = %d, number of columns = %d. This means an additional comma was declared.\n", i, size_result, table->num_cols);
					// myFreeAllCleanup() is called by createTable so this pointer's malloced node will be freed but not its data pointer
					errorTeardown(&file_opened_head, malloced_head, the_debug);
					printf("calling deleteTable() with table_name_copy = _%s_\n", table_name_copy);
					deleteTable(table_name_copy, malloced_head, the_debug);
					myFree((void**) &table_name_copy, NULL, NULL, the_debug);
					return -15;
				}

				for (int a=0; a<size_result; a++)
				{
					myFree((void**) &num_cols_in_line[a], &file_opened_head, malloced_head, the_debug);
				}
				myFree((void**) &num_cols_in_line, &file_opened_head, malloced_head, the_debug);
			// END Check for extra comma


			for (int j=0; j<table->num_cols; j++)
			{
				// START Get value after certain amount of cols, using format
					char* format = (char*) myMalloc(sizeof(char) * 2000, &file_opened_head, malloced_head, the_debug);
					if (format == NULL)
					{
						if (the_debug == YES_DEBUG)
							printf("	ERROR in createTableFromCSV() at line %d in %s\n", __LINE__, __FILE__);
						// myFreeAllCleanup() is called by createTable so this pointer's malloced node will be freed but not its data pointer
						deleteTable(table_name_copy, malloced_head, the_debug);
	    				myFree((void**) &table_name_copy, NULL, NULL, the_debug);
						return RETURN_ERROR;
					}
					strcpy(format, "");
					for (int k=0; k<j; k++)
		           		strcat(format, "%*[^,],");
					strcat(format, "%[^,],%*[^\n]\n");

					char* data = (char*) myMalloc(sizeof(char) * 300, &file_opened_head, malloced_head, the_debug);
		            if (data == NULL)
					{
						if (the_debug == YES_DEBUG)
							printf("	ERROR in createTableFromCSV() at line %d in %s\n", __LINE__, __FILE__);
						deleteTable(table_name_copy, malloced_head, the_debug);
	    				myFree((void**) &table_name_copy, NULL, NULL, the_debug);
						return RETURN_ERROR;
					}

					data[0] = 0;
					sscanf(a_line, format, data);
				// END Get value after certain amount of cols, using format

				if (strcmp(data, "") == 0)
				{
					if (the_debug == YES_DEBUG)
						printf("	ERROR in createTableFromCSV() at line %d in %s\n", __LINE__, __FILE__);
					printf("A value was not specified for the column \"%s\" in row %d. Please specify a value\n", cur_col->col_name, i);
					errorTeardown(&file_opened_head, malloced_head, the_debug);
					// myFreeAllCleanup() is called by createTable so this pointer's malloced node will be freed but not its data pointer
					deleteTable(table_name_copy, malloced_head, the_debug);
    				myFree((void**) &table_name_copy, NULL, NULL, the_debug);
					return -15;
				}

				if (cur_col->data_type == DATA_INT)
				{
					int hmm = 0;
					hmm = atoi(data);

					if (hmm == 0 && data[0] != '0')
					{
						if (the_debug == YES_DEBUG)
							printf("	ERROR in createTableFromCSV() at line %d in %s\n", __LINE__, __FILE__);
						printf("A value specified for the column \"%s\" in row %d was not an integer. Please edit the value or edit the datatype.\n", cur_col->col_name, i);
						errorTeardown(&file_opened_head, malloced_head, the_debug);
						// myFreeAllCleanup() is called by createTable so this pointer's malloced node will be freed but not its data pointer
						deleteTable(table_name_copy, malloced_head, the_debug);
	    				myFree((void**) &table_name_copy, NULL, NULL, the_debug);
						return -15;
					}
				}

				if (cur_col->data_type == DATA_REAL)
				{
					double hmm = 0.0;
					sscanf(data, "%lf", &hmm);

					if (hmm == 0.0 && (data[0] != '0' || data[1] != '.' || data[2] != '0'))
					{
						if (the_debug == YES_DEBUG)
							printf("	ERROR in createTableFromCSV() at line %d in %s\n", __LINE__, __FILE__);
						printf("A value specified for the column \"%s\" in row %d was not a real number. Please edit the value or edit the datatype.\n", cur_col->col_name, i);
						errorTeardown(&file_opened_head, malloced_head, the_debug);
						// myFreeAllCleanup() is called by createTable so this pointer's malloced node will be freed but not its data pointer
						deleteTable(table_name_copy, malloced_head, the_debug);
	    				myFree((void**) &table_name_copy, NULL, NULL, the_debug);
						return -15;
					}
				}

				if (cur_col->data_type == DATA_STRING && strLength(data) > cur_col->max_length-1)
				{
					if (the_debug == YES_DEBUG)
						printf("	ERROR in createTableFromCSV() at line %d in %s\n", __LINE__, __FILE__);
					printf("A value specified for the column \"%s\" in row %d was more than the maximum length which was declared in the datatype. Please edit the value or edit the datatype.\n", cur_col->col_name, i);
					errorTeardown(&file_opened_head, malloced_head, the_debug);
					// myFreeAllCleanup() is called by createTable so this pointer's malloced node will be freed but not its data pointer
					deleteTable(table_name_copy, malloced_head, the_debug);
    				myFree((void**) &table_name_copy, NULL, NULL, the_debug);
					return -15;
				}

				if (cur_col->data_type == DATA_DATE && !isDate(data))
				{
					if (the_debug == YES_DEBUG)
						printf("	ERROR in createTableFromCSV() at line %d in %s\n", __LINE__, __FILE__);
					printf("A value specified for the column \"%s\" in row %d was not a date. Please edit the value or edit the datatype.\n", cur_col->col_name, i);
					errorTeardown(&file_opened_head, malloced_head, the_debug);
					// myFreeAllCleanup() is called by createTable so this pointer's malloced node will be freed but not its data pointer
					deleteTable(table_name_copy, malloced_head, the_debug);
    				myFree((void**) &table_name_copy, NULL, NULL, the_debug);
					return -15;
				}

	            int_8 data_int_date = 0;
	            double data_real = 0.0;
	            
	            if (cur_col->data_type == 1)
					sscanf(data, "%lu", &data_int_date);
	            else if (cur_col->data_type == 2)
	                sscanf(data, "%lf", &data_real);
				
	            //if (cur_col->data_type == 1)
				//	printf("data = %lu\n", data_int_date);
	            //else if (cur_col->data_type == 2)
	           	//    printf("data = %lf\n", data_real);
	            //else if (cur_col->data_type == 4)
				//{
				//	data_int_date = dateToInt(data);
				//	printf("data = %lu\n", data_int_date);
				//}
	            //else
				//	printf("data = %s\n", data);

				void* the_ptr = NULL;
				if (cur_col->data_type == DATA_INT)
					the_ptr = &data_int_date;
				else if (cur_col->data_type == DATA_REAL)
					the_ptr = &data_real;
				else if (cur_col->data_type == DATA_STRING || cur_col->data_type == DATA_DATE)
					the_ptr = data;
	            
	            if (insertAppend(col_data_info_file_arr, col_data_file_arr, &file_opened_head
	                            ,table->file_number, cur_col, the_ptr
	                            ,malloced_head, the_debug) < 0)
	            {
	                if (the_debug == YES_DEBUG)
						printf("	ERROR in createTableFromCSV() at line %d in %s\n", __LINE__, __FILE__);
					// myFreeAllCleanup() is called by createTable so this pointer's malloced node will be freed but not its data pointer
					deleteTable(table_name_copy, malloced_head, the_debug);
    				myFree((void**) &table_name_copy, NULL, NULL, the_debug);
					return -16;
	            }

	            //printf("cur_col->num_rows = %d\n", cur_col->num_rows);

				cur_col = cur_col->next;

	            myFree((void**) &data, &file_opened_head, malloced_head, the_debug);
	            myFree((void**) &format, &file_opened_head, malloced_head, the_debug);
			}
			myFree((void**) &a_line, &file_opened_head, malloced_head, the_debug);
		}
		if (the_debug == YES_DEBUG)
			printf("Out of insert append loop\n");
		myFileClose(file, &file_opened_head, malloced_head, the_debug);
    // END For each row, extract each column value and insert append it

    // START Free arrays for files opened
		for (int i=0; i<table->num_cols; i++)
		{
			if (col_data_info_file_arr[i] != NULL)
				myFileClose(col_data_info_file_arr[i], &file_opened_head, malloced_head, the_debug);
			if (col_data_file_arr[i] != NULL)
				myFileClose(col_data_file_arr[i], &file_opened_head, malloced_head, the_debug);
		}
		myFree((void**) &col_data_info_file_arr, &file_opened_head, malloced_head, the_debug);
		myFree((void**) &col_data_file_arr, &file_opened_head, malloced_head, the_debug);
    // END Free arrays for files opened
	

    if (file_opened_head != NULL)
    {
        if (the_debug == YES_DEBUG)
			printf("createTableFromCSV() did not close all files\n");
        myFileCloseAll(&file_opened_head, malloced_head, the_debug);
    }


    // START Initialize frequent lists after table creation and data insertion
	    /*struct table_info* cur_table = getTablesHead();
	    while (cur_table != NULL)
	    {
	    	if (strcmp(cur_table->name, table_name_copy) == 0 && cur_table->table_cols_head->num_rows < MAX_ROWS_FOR_INIT_FREQ_LISTS)
	    	{
				if (the_debug == YES_DEBUG)
					printf("Initializing frequent lists for created table\n");
				if (initFrequentLists(cur_table, malloced_head, the_debug) != RETURN_GOOD)
				{
					if (the_debug == YES_DEBUG)
						printf("	ERROR in createTableFromCSV() at line %d in %s\n", __LINE__, __FILE__);
					printf("There was problem initializing column after insertion. Please try again\n");
					// myFreeAllCleanup() is called by createTable so this pointer's malloced node will be freed but not its data pointer
					deleteTable(table_name_copy, malloced_head, the_debug);
    				myFree((void**) &table_name_copy, NULL, NULL, the_debug);
					return RETURN_ERROR;
				}
	    		break;
	    	}

	    	cur_table = cur_table->next;    	
	    }*/
    // END Initialize frequent lists after table creation and data insertion

    if (the_debug == YES_DEBUG)
		printf("Calling myFreeAllCleanup() from createTableFromCSV(), but NOT freeing ptrs for tables_head->frequent_lists\n");
	myFreeAllCleanup(malloced_head, the_debug);

	myFree((void**) &table_name_copy, NULL, NULL, the_debug);

    return table->table_cols_head->num_rows;
}

/*	RETURNS:
 *  RETURN_ERROR if error
 *	RETURN_GOOD if good
 *
 *	WRITES TO:
 */
int displayResultsOfSelect(struct select_node* select_node, struct malloced_node** malloced_head, int the_debug)
{
	struct file_opened_node* file_opened_head = NULL;

	// START Create .csv file, deleting old if necessary
		if (access("Results.csv", F_OK) == 0)
		{
			while (true)
			{
				remove("Results.csv");
				if (access("Results.csv", F_OK) != 0)
					break;
			}
		}

		FILE* file = myFileOpenSimple("Results.csv", "a", &file_opened_head, malloced_head, the_debug);
		if (file == NULL)
		{
			if (the_debug == YES_DEBUG)
				printf("	ERROR in displayResultsOfSelect() at line %d in %s\n", __LINE__, __FILE__);
			return RETURN_ERROR;
		}
	// END Create .csv file, deleting old if necessary

	// START Write column names
		for (int j=0; j<select_node->columns_arr_size; j++)
		{
			char* the_col_name;

			if (select_node->columns_arr[j]->new_name != NULL)
				the_col_name = select_node->columns_arr[j]->new_name;
			else
			{
				struct col_in_select_node* cur = select_node->columns_arr[j];
				while (cur->col_ptr_type == PTR_TYPE_COL_IN_SELECT_NODE)
					cur = cur->col_ptr;
				
				the_col_name = ((struct table_cols_info*) cur->col_ptr)->col_name;
			}

			if (fwrite(the_col_name, strLength(the_col_name), 1, file) != 1)
			{
				if (the_debug == YES_DEBUG)
					printf("	ERROR in displayResultsOfSelectAndFree() at line %d in %s\n", __LINE__, __FILE__);
				errorTeardown(&file_opened_head, malloced_head, the_debug);
				return RETURN_ERROR;
			}

			if (j != select_node->columns_arr_size-1)
			{
				if (fwrite(",", 1, 1, file) != 1)
				{
					if (the_debug == YES_DEBUG)
						printf("	ERROR in displayResultsOfSelectAndFree() at line %d in %s\n", __LINE__, __FILE__);
					errorTeardown(&file_opened_head, malloced_head, the_debug);
					return RETURN_ERROR;
				}
			}
			else
			{
				if (fwrite("\n", 1, 1, file) != 1)
				{
					if (the_debug == YES_DEBUG)
						printf("	ERROR in displayResultsOfSelectAndFree() at line %d in %s\n", __LINE__, __FILE__);
					errorTeardown(&file_opened_head, malloced_head, the_debug);
					return RETURN_ERROR;
				}
			}
		}
	// END Write column names

	// START Write column data
		for (int i=0; i<select_node->columns_arr[0]->num_rows; i++)
		{
			if (i % 10000 == 0 && i > 0)
			{
				if (the_debug == YES_DEBUG)
					printf("Printed %d rows\n", i);
			}

			for (int j=0; j<select_node->columns_arr_size; j++)
			{
				char word[1000] = {0};

				if (select_node->columns_arr[j]->col_data_arr[i]->row_data != NULL)
				{
					int the_data_type = select_node->columns_arr[j]->rows_data_type;

					if (the_data_type == DATA_INT)
						sprintf(word, "%d", *((int*) select_node->columns_arr[j]->col_data_arr[i]->row_data));
					else if (the_data_type == DATA_REAL)
						sprintf(word, "%lf", *((double*) select_node->columns_arr[j]->col_data_arr[i]->row_data));
					else
						strcpy(word, select_node->columns_arr[j]->col_data_arr[i]->row_data);


					if (fwrite(word, strLength(word), 1, file) != 1 && (the_data_type == DATA_STRING && ((char*) select_node->columns_arr[j]->col_data_arr[i]->row_data)[0] != 0))
					{
						if (the_debug == YES_DEBUG)
						{
							printf("	ERROR in displayResultsOfSelectAndFree() at line %d in %s\n", __LINE__, __FILE__);
							printf("	select_node->columns_arr[j]->col_data_arr[i]->row_data = _%s_\n", select_node->columns_arr[j]->col_data_arr[i]->row_data);
						}
						errorTeardown(&file_opened_head, malloced_head, the_debug);
						return RETURN_ERROR;
					}
				}

				if (j != select_node->columns_arr_size-1)
				{
					if (fwrite(",", 1, 1, file) != 1)
					{
						if (the_debug == YES_DEBUG)
							printf("	ERROR in displayResultsOfSelectAndFree() at line %d in %s\n", __LINE__, __FILE__);
						errorTeardown(&file_opened_head, malloced_head, the_debug);
						return RETURN_ERROR;
					}
				}
				else
				{
					if (fwrite("\n", 1, 1, file) != 1)
					{
						if (the_debug == YES_DEBUG)
							printf("	ERROR in displayResultsOfSelectAndFree() at line %d in %s\n", __LINE__, __FILE__);
						errorTeardown(&file_opened_head, malloced_head, the_debug);
						return RETURN_ERROR;
					}
				}
			}
		}
		myFileClose(file, &file_opened_head, malloced_head, the_debug);
	// END Write column data

	// START Cleanup
		if (file_opened_head != NULL)
	    {
	        if (the_debug == YES_DEBUG)
	        	printf("displayResultsOfSelectAndFree() did not close all files\n");
	        myFileCloseAll(&file_opened_head, malloced_head, the_debug);
	    }
	// END Cleanup

	if (the_debug == YES_DEBUG)
		printf("Returning from displayResultsOfSelectAndFree()\n");

	return RETURN_GOOD;
}

/*	RETURNS:
 *  RETURN_ERROR if error
 *	RETURN_GOOD if good
 *
 *	WRITES TO:
 *	ptr
 * 	ptr_type
 */
int determineDatatypeOfDataWord(char* word, void** ptr, int* ptr_type, struct file_opened_node** file_opened_head, struct malloced_node** malloced_head, int the_debug)
{
	if (word[0] == 39)
	{
		*ptr = substring(word, 1, strLength(word)-2, NULL, malloced_head, the_debug);
		if (*ptr == NULL)
		{
			if (the_debug == YES_DEBUG)
				printf("	ERROR in determineDatatypeOfDataWord() at line %d in %s\n", __LINE__, __FILE__);
			return RETURN_ERROR;
		}

		if (isDate(*ptr))
			*ptr_type = PTR_TYPE_DATE;
		else
			*ptr_type = PTR_TYPE_CHAR;
	}
	else
	{
		int hmm_int = atoi(word);

		if (hmm_int != 0 && strcontains(word, '.'))
		{
			*ptr = myMalloc(sizeof(double), NULL, malloced_head, the_debug);
			if (*ptr == NULL)
			{
				if (the_debug == YES_DEBUG)
					printf("	ERROR in determineDatatypeOfDataWord() at line %d in %s\n", __LINE__, __FILE__);
				return RETURN_ERROR;
			}

			double hmm_double = 0.0;
			sscanf(word, "%lf", &hmm_double);
			*((double*) *ptr) = hmm_double;

			*ptr_type = PTR_TYPE_REAL;
		}
		else if (hmm_int != 0 || word[0] == '0')
		{
			*ptr = myMalloc(sizeof(int), NULL, malloced_head, the_debug);
			if (*ptr == NULL)
			{
				if (the_debug == YES_DEBUG)
					printf("	ERROR in determineDatatypeOfDataWord() at line %d in %s\n", __LINE__, __FILE__);
				return RETURN_ERROR;
			}

			*((int*) *ptr) = hmm_int;

			*ptr_type = PTR_TYPE_INT;
		}
	}

	return RETURN_GOOD;
}

/*	RETURNS:
 *  RETURN_NORMAL_NEG if col not found
 *	RETURN_GOOD if col found
 *
 *	WRITES TO:
 *  put_ptrs_here
 *	put_ptrs_here_type
 */
int getColInSelectNodeFromName(void* put_ptrs_here, int put_ptrs_here_type, int math_node_which, int i
							  ,struct select_node* select_node, char* cur_col_alias, char* cur_col_name
							  ,struct malloced_node** malloced_head, int the_debug)
{
	if (the_debug == YES_DEBUG)
		printf("getColInSelectNodeFromName is finding _%s.%s_\n", cur_col_alias, cur_col_name);

	bool found = false;

	// START Looking in from select_node (*select_node)->prev
		for (int j=0; j<select_node->prev->columns_arr_size; j++)
		{
			struct col_in_select_node* cur = select_node->prev->columns_arr[j];
			while (cur->col_ptr_type == PTR_TYPE_COL_IN_SELECT_NODE)
				cur = cur->col_ptr;

			char* col_name = NULL;
			char* table_name = NULL;

			if (cur->col_ptr != NULL)
			{
				col_name = ((struct table_cols_info*) cur->col_ptr)->col_name;
				table_name = ((struct table_info*) cur->table_ptr)->name;
			}
			else
			{
				col_name = cur->new_name;
			}

			//printf("checking col_name = _%s_\n", col_name);
			//printf("table_name = _%s_\n", table_name);

			if (strcmp(cur_col_name, col_name) == 0)
			{
				if ((cur_col_alias == NULL && select_node->join_head == NULL)
					|| (cur_col_alias != NULL && select_node->prev->select_node_alias != NULL && strcmp_Upper(cur_col_alias, select_node->prev->select_node_alias) == 0)
					|| (cur_col_alias != NULL && table_name != NULL && strcmp_Upper(cur_col_alias, table_name) == 0))
				{
					//printf("Found it: cur_col_name = _%s_ and found _%s_\n", cur_col_name, col_name);
					if (the_debug == YES_DEBUG)
						printf("Found it\n");
					found = true;

					if (i == 0)
					{
						return RETURN_GOOD;
					}
					else // if (i == 1)
					{
						if (put_ptrs_here_type == PTR_TYPE_COL_IN_SELECT_NODE)
						{
							((struct col_in_select_node*) put_ptrs_here)->table_ptr = select_node->prev;
							((struct col_in_select_node*) put_ptrs_here)->table_ptr_type = PTR_TYPE_SELECT_NODE;

							((struct col_in_select_node*) put_ptrs_here)->col_ptr = select_node->prev->columns_arr[j];
							((struct col_in_select_node*) put_ptrs_here)->col_ptr_type = PTR_TYPE_COL_IN_SELECT_NODE;
						}
						else if (put_ptrs_here_type == PTR_TYPE_MATH_NODE)
						{
							if (math_node_which == 1)
							{
								if (((struct math_node*) put_ptrs_here)->ptr_one != NULL)
									myFree((void**) &((struct math_node*) put_ptrs_here)->ptr_one, NULL, malloced_head, the_debug);	

								((struct math_node*) put_ptrs_here)->ptr_one = select_node->prev->columns_arr[j];
								((struct math_node*) put_ptrs_here)->ptr_one_type = PTR_TYPE_COL_IN_SELECT_NODE;
							}
							else //if (math_node_which == 2)
							{
								((struct math_node*) put_ptrs_here)->ptr_two = select_node->prev->columns_arr[j];
								((struct math_node*) put_ptrs_here)->ptr_two_type = PTR_TYPE_COL_IN_SELECT_NODE;
							}
						}
						else if (put_ptrs_here_type == PTR_TYPE_LIST_NODE_PTR)
						{
							((struct ListNodePtr*) put_ptrs_here)->ptr_value = select_node->prev->columns_arr[j];
							((struct ListNodePtr*) put_ptrs_here)->ptr_type = PTR_TYPE_COL_IN_SELECT_NODE;
						}
						else if (put_ptrs_here_type == PTR_TYPE_FUNC_NODE)
						{
							myFree((void**) &((struct func_node*) put_ptrs_here)->args_arr[math_node_which], NULL, malloced_head, the_debug);

							((struct func_node*) put_ptrs_here)->args_arr[math_node_which] = select_node->prev->columns_arr[j];
							((struct func_node*) put_ptrs_here)->args_arr_type[math_node_which] = PTR_TYPE_COL_IN_SELECT_NODE;
						}
						else if (put_ptrs_here_type == PTR_TYPE_WHERE_CLAUSE_NODE)
						{
							if (math_node_which == 1)
							{
								((struct where_clause_node*) put_ptrs_here)->ptr_one = select_node->prev->columns_arr[j];
								((struct where_clause_node*) put_ptrs_here)->ptr_one_type = PTR_TYPE_COL_IN_SELECT_NODE;
							}
							else //if (math_node_which == 2)
							{
								((struct where_clause_node*) put_ptrs_here)->ptr_two = select_node->prev->columns_arr[j];
								((struct where_clause_node*) put_ptrs_here)->ptr_two_type = PTR_TYPE_COL_IN_SELECT_NODE;
							}
						}

						return RETURN_GOOD;
					}
				}
			}
		}
	// END Looking in from select_node select_node->prev

	// START Looking in all joined nodes select_node->join_head->select_joined
		struct join_node* cur_join = NULL;

		if (!found)
		{
			cur_join = select_node->join_head;

			while (cur_join != NULL)
			{
				for (int j=0; j<cur_join->select_joined->columns_arr_size; j++)
				{
					struct col_in_select_node* cur = cur_join->select_joined->columns_arr[j];
					while (cur->col_ptr_type == PTR_TYPE_COL_IN_SELECT_NODE)
						cur = cur->col_ptr;

					char* col_name = NULL;
					char* table_name = NULL;

					if (cur->col_ptr != NULL)
					{
						col_name = ((struct table_cols_info*) cur->col_ptr)->col_name;
						table_name = ((struct table_info*) cur->table_ptr)->name;
					}
					else
					{
						col_name = cur->new_name;
					}
					
					//printf("checking (in join) col_name = _%s_\n", col_name);
					//printf("table_name = _%s_\n", table_name);

					if (strcmp(cur_col_name, col_name) == 0)
					{
						if ((cur_col_alias == NULL && cur_join == NULL) 
							|| (cur_col_alias != NULL && cur_join->select_joined->select_node_alias != NULL && strcmp_Upper(cur_col_alias, cur_join->select_joined->select_node_alias) == 0)
							|| (cur_col_alias != NULL && table_name != NULL && strcmp_Upper(cur_col_alias, table_name) == 0))
						{
							if (the_debug == YES_DEBUG)
								printf("Found it\n");
							found = true;

							if (i == 0)
							{
								return RETURN_GOOD;
							}
							else // if (i == 1)
							{
								if (put_ptrs_here_type == PTR_TYPE_COL_IN_SELECT_NODE)
								{
									((struct col_in_select_node*) put_ptrs_here)->table_ptr = cur_join->select_joined;
									((struct col_in_select_node*) put_ptrs_here)->table_ptr_type = PTR_TYPE_SELECT_NODE;

									((struct col_in_select_node*) put_ptrs_here)->col_ptr = cur_join->select_joined->columns_arr[j];
									((struct col_in_select_node*) put_ptrs_here)->col_ptr_type = PTR_TYPE_COL_IN_SELECT_NODE;
								}
								else if (put_ptrs_here_type == PTR_TYPE_MATH_NODE)
								{
									if (math_node_which == 1)
									{
										myFree((void**) &((struct math_node*) put_ptrs_here)->ptr_one, NULL, malloced_head, the_debug);

										((struct math_node*) put_ptrs_here)->ptr_one = cur_join->select_joined->columns_arr[j];
										((struct math_node*) put_ptrs_here)->ptr_one_type = PTR_TYPE_COL_IN_SELECT_NODE;
									}
									else //if (math_node_which == 2)
									{
										((struct math_node*) put_ptrs_here)->ptr_two = cur_join->select_joined->columns_arr[j];
										((struct math_node*) put_ptrs_here)->ptr_two_type = PTR_TYPE_COL_IN_SELECT_NODE;
									}
								}
								else if (put_ptrs_here_type == PTR_TYPE_LIST_NODE_PTR)
								{
									((struct ListNodePtr*) put_ptrs_here)->ptr_value = cur_join->select_joined->columns_arr[j];
									((struct ListNodePtr*) put_ptrs_here)->ptr_type = PTR_TYPE_COL_IN_SELECT_NODE;
								}
								else if (put_ptrs_here_type == PTR_TYPE_FUNC_NODE)
								{
									myFree((void**) &((struct func_node*) put_ptrs_here)->args_arr[math_node_which], NULL, malloced_head, the_debug);

									((struct func_node*) put_ptrs_here)->args_arr[math_node_which] = cur_join->select_joined->columns_arr[j];
									((struct func_node*) put_ptrs_here)->args_arr_type[math_node_which] = PTR_TYPE_COL_IN_SELECT_NODE;
								}
								else if (put_ptrs_here_type == PTR_TYPE_WHERE_CLAUSE_NODE)
								{
									if (math_node_which == 1)
									{
										((struct where_clause_node*) put_ptrs_here)->ptr_one = cur_join->select_joined->columns_arr[j];
										((struct where_clause_node*) put_ptrs_here)->ptr_one_type = PTR_TYPE_COL_IN_SELECT_NODE;
									}
									else //if (math_node_which == 2)
									{
										((struct where_clause_node*) put_ptrs_here)->ptr_two = cur_join->select_joined->columns_arr[j];
										((struct where_clause_node*) put_ptrs_here)->ptr_two_type = PTR_TYPE_COL_IN_SELECT_NODE;
									}
								}

								return RETURN_GOOD;
							}
						}
					}
				}

				cur_join = cur_join->next;
			}
		}
	// END Looking in all joined nodes select_node->join_head->select_joined

	// START Look in this select_node for created column
		if (!found)
		{
			for (int j=0; j<select_node->columns_arr_size; j++)
			{
				if (select_node->columns_arr != NULL && select_node->columns_arr[j] != NULL && select_node->columns_arr[j]->new_name != NULL)
				{
					if (strcmp_Upper(cur_col_name, select_node->columns_arr[j]->new_name) == 0)
					{
						if (the_debug == YES_DEBUG)
							printf("Found it\n");
						found = true;

						if (i == 0)
						{
							return RETURN_GOOD;
						}
						else // if (i == 1)
						{
							if (put_ptrs_here_type == PTR_TYPE_COL_IN_SELECT_NODE)
							{
								((struct col_in_select_node*) put_ptrs_here)->table_ptr = select_node;
								((struct col_in_select_node*) put_ptrs_here)->table_ptr_type = PTR_TYPE_SELECT_NODE;

								((struct col_in_select_node*) put_ptrs_here)->col_ptr = select_node->columns_arr[j];
								((struct col_in_select_node*) put_ptrs_here)->col_ptr_type = PTR_TYPE_COL_IN_SELECT_NODE;
							}
							else if (put_ptrs_here_type == PTR_TYPE_MATH_NODE)
							{
								if (math_node_which == 1)
								{
									if (((struct math_node*) put_ptrs_here)->ptr_one != NULL)
										myFree((void**) &((struct math_node*) put_ptrs_here)->ptr_one, NULL, malloced_head, the_debug);	

									((struct math_node*) put_ptrs_here)->ptr_one = select_node->columns_arr[j];
									((struct math_node*) put_ptrs_here)->ptr_one_type = PTR_TYPE_COL_IN_SELECT_NODE;
								}
								else //if (math_node_which == 2)
								{
									((struct math_node*) put_ptrs_here)->ptr_two = select_node->columns_arr[j];
									((struct math_node*) put_ptrs_here)->ptr_two_type = PTR_TYPE_COL_IN_SELECT_NODE;
								}
							}
							else if (put_ptrs_here_type == PTR_TYPE_LIST_NODE_PTR)
							{
								((struct ListNodePtr*) put_ptrs_here)->ptr_value = select_node->columns_arr[j];
								((struct ListNodePtr*) put_ptrs_here)->ptr_type = PTR_TYPE_COL_IN_SELECT_NODE;
							}
							else if (put_ptrs_here_type == PTR_TYPE_FUNC_NODE)
							{
								myFree((void**) &((struct func_node*) put_ptrs_here)->args_arr[math_node_which], NULL, malloced_head, the_debug);

								((struct func_node*) put_ptrs_here)->args_arr[math_node_which] = select_node->columns_arr[j];
								((struct func_node*) put_ptrs_here)->args_arr_type[math_node_which] = PTR_TYPE_COL_IN_SELECT_NODE;
							}
							else if (put_ptrs_here_type == PTR_TYPE_WHERE_CLAUSE_NODE)
							{
								if (math_node_which == 1)
								{
									((struct where_clause_node*) put_ptrs_here)->ptr_one = select_node->columns_arr[j];
									((struct where_clause_node*) put_ptrs_here)->ptr_one_type = PTR_TYPE_COL_IN_SELECT_NODE;
								}
								else //if (math_node_which == 2)
								{
									((struct where_clause_node*) put_ptrs_here)->ptr_two = select_node->columns_arr[j];
									((struct where_clause_node*) put_ptrs_here)->ptr_two_type = PTR_TYPE_COL_IN_SELECT_NODE;
								}
							}

							return RETURN_GOOD;
						}
					}
				}
			}
		}
	// END Look in this select_node for created column

	if (!found && cur_join == NULL)
	{
		// Column does not have an alias, and there are joined tables
		// Or, column and alias combo could not be found in valid set of columns
		if (the_debug == YES_DEBUG)
			printf("	ERROR in getColInSelectNodeFromName() at line %d in %s\n", __LINE__, __FILE__);
		return RETURN_NORMAL_NEG;
	}
	else if (!found && cur_join != NULL)
	{
		// Column does not have an alias, and there are joined tables
		// Or, column and alias combo could not be found in valid set of columns
		if (the_debug == YES_DEBUG)
			printf("	ERROR in getColInSelectNodeFromName() at line %d in %s\n", __LINE__, __FILE__);
		return RETURN_NORMAL_NEG;
	}

	return RETURN_GOOD;
}

/*	RETURNS:
 *  RETURN_ERROR if error
 *	RETURN_GOOD if good
 *
 *	WRITES TO:
 *	where_node
 */
int parseOneWhereNode(char* input, char* word, struct where_clause_node** where_node, struct select_node* select_node, struct table_info* table
					 ,struct malloced_node** malloced_head, int the_debug)
{
	int index = 0;

	char* prev_word = NULL;

	while (getNextWord(input, word, &index) == 0 && word[0] != ';')
	{
		if (input[index] == '.')
		{
			// START Ensure that word has alias.name
				char* temp_word = (char*) myMalloc(sizeof(char) * 200, NULL, malloced_head, the_debug);
				if (temp_word == NULL)
				{
					if (the_debug == YES_DEBUG)
						printf("	ERROR in parseOneWhereNode() at line %d in %s\n", __LINE__, __FILE__);
					*where_node = NULL;
					return RETURN_ERROR;
				}

				strcpy(temp_word, word);

				strcat(temp_word, ".");
				index++;

				getNextWord(input, word, &index);

				strcat(temp_word, word);

				strcpy(word, temp_word);

				myFree((void**) &temp_word, NULL, malloced_head, the_debug);
			// END Ensure that word has alias.name
		}

		if (the_debug == YES_DEBUG)
			printf("word in parseOneWhereNode = _%s_\n", word);

		if (strcmp_Upper(word, "CASE") == 0)
		{
			char* case_input = (char*) myMalloc(sizeof(char) * 10000, NULL, malloced_head, the_debug);
			if (case_input == NULL)
			{
				if (the_debug == YES_DEBUG)
					printf("	ERROR in parseOneWhereNode() at line %d in %s\n", __LINE__, __FILE__);
				*where_node = NULL;
				return RETURN_ERROR;
			}

			strcpy(case_input, "CASE ");
			while (getNextWord(input, word, &index) == 0 && strcmp_Upper(word, "END") != 0)
			{
				if (input[index] == '.')
				{
					strcat(case_input, word);
					strcat(case_input, ".");
					index++;
				}
				else
				{
					strcat(case_input, word);
					strcat(case_input, " ");
				}
			}

			strcat(case_input, "END");

			struct case_node* case_node = NULL;

			int ptr_type = PTR_TYPE_CASE_NODE;
			if (parseCaseNode(&case_node, case_input, word, 5, NULL, select_node, &ptr_type, malloced_head, the_debug) != RETURN_GOOD)
			{
				if (the_debug == YES_DEBUG)
					printf("	ERROR in parseOneWhereNode() at line %d in %s\n", __LINE__, __FILE__);
				*where_node = NULL;
				return RETURN_ERROR;
			}

			myFree((void**) &case_input, NULL, malloced_head, the_debug);

			if (the_debug == YES_DEBUG)
				printf("Back from parseCaseNode()\n");

			if ((*where_node)->ptr_one == NULL)
			{
				if (the_debug == YES_DEBUG)
					printf("	Assigning to ptr_one for where\n");
				(*where_node)->ptr_one = case_node;
				(*where_node)->ptr_one_type = PTR_TYPE_CASE_NODE;
			}
			else //if ((*where_node)->ptr_two == NULL)
			{
				if (the_debug == YES_DEBUG)
					printf("	Assigning to ptr_two for where\n");
				(*where_node)->ptr_two = case_node;
				(*where_node)->ptr_two_type = PTR_TYPE_CASE_NODE;

				int col_ptr_one_type = -1;
				if ((*where_node)->ptr_one_type == PTR_TYPE_COL_IN_SELECT_NODE)
				{
					struct col_in_select_node* cur = (*where_node)->ptr_one;

					if (cur->func_node != NULL)
						col_ptr_one_type = cur->func_node->result_type;
					else if (cur->math_node != NULL)
						col_ptr_one_type = cur->math_node->result_type;
					else if (cur->case_node != NULL)
						col_ptr_one_type = cur->case_node->result_type;
					else
						col_ptr_one_type = cur->rows_data_type;
				}
				else if ((*where_node)->ptr_one_type == PTR_TYPE_TABLE_COLS_INFO)
					col_ptr_one_type = ((struct table_cols_info*) (*where_node)->ptr_one)->data_type;
				else if ((*where_node)->ptr_one_type == PTR_TYPE_MATH_NODE)
					col_ptr_one_type = ((struct math_node*) (*where_node)->ptr_one)->result_type;
				else if ((*where_node)->ptr_one_type == PTR_TYPE_FUNC_NODE)
					col_ptr_one_type = ((struct func_node*) (*where_node)->ptr_one)->result_type;
				else if ((*where_node)->ptr_one_type == PTR_TYPE_CASE_NODE)
					col_ptr_one_type = ((struct case_node*) (*where_node)->ptr_one)->result_type;
				else
					col_ptr_one_type = (*where_node)->ptr_one_type;

				// START Check if matching datatype
					if (col_ptr_one_type != case_node->result_type)
					{
						if (the_debug == YES_DEBUG)
							printf("	ERROR in parseOneWhereNode() at line %d in %s\n", __LINE__, __FILE__);
						printf("An inequality was declared with mismatching datatypes. Please declare matching datatypes. Failing word: \"%s\"\n", word);
						errorTeardown(NULL, malloced_head, the_debug);
						*where_node = NULL;
						return RETURN_ERROR;
					}
				// END Check if matching datatype
			}
		}
		else if ((word[0] == '+' || word[0] == '-' || word[0] == '*' || word[0] == '/' || word[0] == '^') || (prev_word == NULL && word[0] == '('))
		{
			// START Parse math node in input
				if (the_debug == YES_DEBUG)
					printf("Found math in parsing where node, prev_word = _%s_\n", prev_word);

				struct math_node* new_math_node = NULL;

				if ((*where_node)->ptr_one_type == PTR_TYPE_FUNC_NODE)
				{
					new_math_node = (struct math_node*) myMalloc(sizeof(struct math_node), NULL, malloced_head, the_debug);
					if (new_math_node == NULL)
					{
						if (the_debug == YES_DEBUG)
							printf("	ERROR in parseOneWhereNode() at line %d in %s\n", __LINE__, __FILE__);
						*where_node = NULL;
						return RETURN_ERROR;
					}

					new_math_node->ptr_one = (*where_node)->ptr_one;
					new_math_node->ptr_one_type = PTR_TYPE_FUNC_NODE;

					new_math_node->ptr_two = NULL;
					new_math_node->ptr_two_type = -1;

					new_math_node->operation = -1;
					new_math_node->parent = NULL;


					if (parseMathNode(&new_math_node, NULL, &prev_word, input, word, index, select_node
									 ,malloced_head, the_debug) != RETURN_GOOD)
					{
						*where_node = NULL;
						return RETURN_ERROR;
					}

					if (the_debug == YES_DEBUG)
						printf("Back from parseMathNode()\n");

					getNextWord(input, word, &index);
					while (word[0] != '=' && word[0] != '>' && word[0] != '<')
					{
						if (getNextWord(input, word, &index) != 0)
							break;
						if (input[index] == '.')
							index++;
					}
					index -= strLength(word);


					(*where_node)->ptr_one = new_math_node;
					(*where_node)->ptr_one_type = PTR_TYPE_MATH_NODE;
				}
				else
				{
					if (parseMathNode(&new_math_node, NULL, &prev_word, input, word, index, select_node
									 ,malloced_head, the_debug) != RETURN_GOOD)
					{
						*where_node = NULL;
						return RETURN_ERROR;
					}

					if (the_debug == YES_DEBUG)
						printf("Back from parseMathNode()\n");

					getNextWord(input, word, &index);
					while (word[0] != '=' && word[0] != '>' && word[0] != '<')
					{
						if (getNextWord(input, word, &index) != 0)
							break;
						if (input[index] == '.')
							index++;
					}

					index -= strLength(word);


					if ((*where_node)->ptr_two == NULL)
					{
						if ((*where_node)->ptr_one_type == PTR_TYPE_INT || (*where_node)->ptr_one_type == PTR_TYPE_REAL || (*where_node)->ptr_one_type == PTR_TYPE_CHAR || (*where_node)->ptr_one_type == PTR_TYPE_DATE)
						{
							myFree((void**) &(*where_node)->ptr_one, NULL, malloced_head, the_debug);
						}

						(*where_node)->ptr_one = new_math_node;
						(*where_node)->ptr_one_type = PTR_TYPE_MATH_NODE;
					}
					else //if ((*where_node)->ptr_two != NULL)
					{
						if ((*where_node)->ptr_two_type == PTR_TYPE_INT || (*where_node)->ptr_two_type == PTR_TYPE_REAL || (*where_node)->ptr_two_type == PTR_TYPE_CHAR || (*where_node)->ptr_two_type == PTR_TYPE_DATE)
						{
							myFree((void**) &(*where_node)->ptr_one, NULL, malloced_head, the_debug);
						}

						(*where_node)->ptr_two = new_math_node;
						(*where_node)->ptr_two_type = PTR_TYPE_MATH_NODE;
					}
				}					
			// END Parse math node in input
		}
		else if (word[0] == '=' || word[0] == '>' || word[0] == '<' || strcmp_Upper(word, "IS") == 0)
		{
			// START Label where_type according to word
				if ((*where_node)->where_type != -1)
				{
					if (the_debug == YES_DEBUG)
						printf("	ERROR in parseOneWhereNode() at line %d in %s\n", __LINE__, __FILE__);
					printf("An operation in a where clause was declared twice. Failing word: \"%s\"\n", word);
					errorTeardown(NULL, malloced_head, the_debug);
					*where_node = NULL;
					return RETURN_ERROR;
				}

				if (word[0] == '=')
					(*where_node)->where_type = WHERE_IS_EQUALS;
				else if (word[0] == '<' && word[1] == '>')
					(*where_node)->where_type = WHERE_NOT_EQUALS;
				else if (word[0] == '>' && word[1] == 0)
					(*where_node)->where_type = WHERE_GREATER_THAN;
				else if (word[0] == '>' && word[1] == '=')
					(*where_node)->where_type = WHERE_GREATER_THAN_OR_EQUAL;
				else if (word[0] == '<' && word[1] == 0)
					(*where_node)->where_type = WHERE_LESS_THAN;
				else if (word[0] == '<' && word[1] == '=')
					(*where_node)->where_type = WHERE_LESS_THAN_OR_EQUAL;
				else if (strcmp_Upper(word, "IS") == 0)
				{
					getNextWord(input, word, &index);

					if (strcmp_Upper(word, "NULL") == 0)
					{
						(*where_node)->where_type = WHERE_IS_NULL;
					}
					else if (strcmp_Upper(word, "NOT") == 0)
					{
						getNextWord(input, word, &index);

						if (strcmp_Upper(word, "NULL") == 0)
						{
							(*where_node)->where_type = WHERE_IS_NOT_NULL;
						}
						else
						{
							if (the_debug == YES_DEBUG)
								printf("	ERROR in parseOneWhereNode() at line %d in %s\n", __LINE__, __FILE__);
							printf("The word \"NULL\" was expected after the word \"NOT\" in a where clause. Failing word: \"%s\"\n", word);
							errorTeardown(NULL, malloced_head, the_debug);
							*where_node = NULL;
							return RETURN_ERROR;
						}
					}
					else
					{
						if (the_debug == YES_DEBUG)
							printf("	ERROR in parseOneWhereNode() at line %d in %s\n", __LINE__, __FILE__);
						printf("The word \"NOT\" or \"NULL\" was expected after the word \"IS\" in a where clause. Failing word: \"%s\"\n", word);
						errorTeardown(NULL, malloced_head, the_debug);
						*where_node = NULL;
						return RETURN_ERROR;
					}
				}

				if (the_debug == YES_DEBUG)
					printf("where_type = %d\n", (*where_node)->where_type);
			// END Label where_type according to word
		}
		else if ((*where_node)->ptr_one != NULL && (*where_node)->ptr_two == NULL)
		{
			// START Determine type of next word and put into ptr_two
				if (the_debug == YES_DEBUG)
					printf("	Assigning to ptr_two for where\n");

				if (word[0] == 39)
				{
					(*where_node)->ptr_two = substring(word, 1, strLength(word)-2, NULL, malloced_head, the_debug);
					if ((*where_node)->ptr_two == NULL)
					{
						if (the_debug == YES_DEBUG)
							printf("	ERROR in parseOneWhereNode() at line %d in %s\n", __LINE__, __FILE__);
						*where_node = NULL;
						return RETURN_ERROR;
					}

					if (isDate((char*) (*where_node)->ptr_two))
					{
						(*where_node)->ptr_two_type = PTR_TYPE_DATE;


						int col_ptr_one_type = -1;
						if ((*where_node)->ptr_one_type == PTR_TYPE_COL_IN_SELECT_NODE)
						{
							struct col_in_select_node* cur = (*where_node)->ptr_one;

							if (cur->func_node != NULL)
								col_ptr_one_type = cur->func_node->result_type;
							else if (cur->math_node != NULL)
								col_ptr_one_type = cur->math_node->result_type;
							else if (cur->case_node != NULL)
								col_ptr_one_type = cur->case_node->result_type;
							else
								col_ptr_one_type = cur->rows_data_type;
						}
						else if ((*where_node)->ptr_one_type == PTR_TYPE_TABLE_COLS_INFO)
							col_ptr_one_type = ((struct table_cols_info*) (*where_node)->ptr_one)->data_type;
						else if ((*where_node)->ptr_one_type == PTR_TYPE_MATH_NODE)
							col_ptr_one_type = ((struct math_node*) (*where_node)->ptr_one)->result_type;
						else if ((*where_node)->ptr_one_type == PTR_TYPE_FUNC_NODE)
							col_ptr_one_type = ((struct func_node*) (*where_node)->ptr_one)->result_type;
						else if ((*where_node)->ptr_one_type == PTR_TYPE_CASE_NODE)
							col_ptr_one_type = ((struct case_node*) (*where_node)->ptr_one)->result_type;


						// START Check if matching datatype
						if ((*where_node)->ptr_one_type != PTR_TYPE_DATE && col_ptr_one_type != DATA_DATE)
						{
							if (the_debug == YES_DEBUG)
								printf("	ERROR in parseOneWhereNode() at line %d in %s\n", __LINE__, __FILE__);
							printf("An inequality was declared with mismatching datatypes. Please declare matching datatypes. Failing word: \"%s\"\n", word);
							errorTeardown(NULL, malloced_head, the_debug);
							*where_node = NULL;
							return RETURN_ERROR;
						}
						// END Check if matching datatype
					}
					else
					{
						(*where_node)->ptr_two_type = PTR_TYPE_CHAR;


							int col_ptr_one_type = -1;
							if ((*where_node)->ptr_one_type == PTR_TYPE_COL_IN_SELECT_NODE)
							{
								struct col_in_select_node* cur = (*where_node)->ptr_one;

								if (cur->func_node != NULL)
									col_ptr_one_type = cur->func_node->result_type;
								else if (cur->math_node != NULL)
									col_ptr_one_type = cur->math_node->result_type;
								else if (cur->case_node != NULL)
									col_ptr_one_type = cur->case_node->result_type;
								else
									col_ptr_one_type = cur->rows_data_type;
							}
							else if ((*where_node)->ptr_one_type == PTR_TYPE_TABLE_COLS_INFO)
								col_ptr_one_type = ((struct table_cols_info*) (*where_node)->ptr_one)->data_type;
							else if ((*where_node)->ptr_one_type == PTR_TYPE_MATH_NODE)
								col_ptr_one_type = ((struct math_node*) (*where_node)->ptr_one)->result_type;
							else if ((*where_node)->ptr_one_type == PTR_TYPE_FUNC_NODE)
								col_ptr_one_type = ((struct func_node*) (*where_node)->ptr_one)->result_type;
							else if ((*where_node)->ptr_one_type == PTR_TYPE_CASE_NODE)
								col_ptr_one_type = ((struct case_node*) (*where_node)->ptr_one)->result_type;


						// START Check if matching datatype
						if ((*where_node)->ptr_one_type != PTR_TYPE_CHAR && col_ptr_one_type != DATA_STRING)
						{
							if (the_debug == YES_DEBUG)
								printf("	ERROR in parseOneWhereNode() at line %d in %s\n", __LINE__, __FILE__);
							printf("An inequality was declared with mismatching datatypes. Please declare matching datatypes. Failing word: \"%s\"\n", word);
							errorTeardown(NULL, malloced_head, the_debug);
							*where_node = NULL;
							return RETURN_ERROR;
						}
						// END Check if matching datatype
					}
				}
				else
				{
					int hmm_int = atoi(word);

					if (hmm_int != 0 && strcontains(word, '.'))
					{
						// Double
						(*where_node)->ptr_two = myMalloc(sizeof(double), NULL, malloced_head, the_debug);
						if ((*where_node)->ptr_two == NULL)
						{
							if (the_debug == YES_DEBUG)
								printf("	ERROR in parseOneWhereNode() at line %d in %s\n", __LINE__, __FILE__);
							*where_node = NULL;
							return RETURN_ERROR;
						}

						double hmm_double = 0.0;

						sscanf(word, "%lf", &hmm_double);

						*((double*) (*where_node)->ptr_two) = hmm_double;

						(*where_node)->ptr_two_type = PTR_TYPE_REAL;

						// START Check if matching datatype
							int col_ptr_one_type = -1;
							if ((*where_node)->ptr_one_type == PTR_TYPE_COL_IN_SELECT_NODE)
							{
								struct col_in_select_node* cur = (*where_node)->ptr_one;

								if (cur->func_node != NULL)
									col_ptr_one_type = cur->func_node->result_type;
								else if (cur->math_node != NULL)
									col_ptr_one_type = cur->math_node->result_type;
								else if (cur->case_node != NULL)
									col_ptr_one_type = cur->case_node->result_type;
								else
									col_ptr_one_type = cur->rows_data_type;
							}
							else if ((*where_node)->ptr_one_type == PTR_TYPE_TABLE_COLS_INFO)
								col_ptr_one_type = ((struct table_cols_info*) (*where_node)->ptr_one)->data_type;
							else if ((*where_node)->ptr_one_type == PTR_TYPE_MATH_NODE)
								col_ptr_one_type = ((struct math_node*) (*where_node)->ptr_one)->result_type;
							else if ((*where_node)->ptr_one_type == PTR_TYPE_FUNC_NODE)
								col_ptr_one_type = ((struct func_node*) (*where_node)->ptr_one)->result_type;
							else if ((*where_node)->ptr_one_type == PTR_TYPE_CASE_NODE)
								col_ptr_one_type = ((struct case_node*) (*where_node)->ptr_one)->result_type;


							if ((*where_node)->ptr_one_type != PTR_TYPE_REAL && col_ptr_one_type != DATA_REAL)
							{
								if (the_debug == YES_DEBUG)
									printf("	ERROR in parseOneWhereNode() at line %d in %s\n", __LINE__, __FILE__);
								printf("An inequality was declared with mismatching datatypes. Please declare matching datatypes. Failing word: \"%s\"\n", word);
								errorTeardown(NULL, malloced_head, the_debug);
								*where_node = NULL;
								return RETURN_ERROR;
							}
						// END Check if matching datatype
					}
					else if (hmm_int != 0 || word[0] == '0')
					{
						// Int
						(*where_node)->ptr_two = myMalloc(sizeof(int), NULL, malloced_head, the_debug);
						if ((*where_node)->ptr_two == NULL)
						{
							if (the_debug == YES_DEBUG)
								printf("	ERROR in parseOneWhereNode() at line %d in %s\n", __LINE__, __FILE__);
							*where_node = NULL;
							return RETURN_ERROR;
						}

						*((int*) (*where_node)->ptr_two) = hmm_int;

						(*where_node)->ptr_two_type = PTR_TYPE_INT;

						// START Check if matching datatype
							int col_ptr_one_type = -1;
							if ((*where_node)->ptr_one_type == PTR_TYPE_COL_IN_SELECT_NODE)
							{
								struct col_in_select_node* cur = (*where_node)->ptr_one;

								if (cur->func_node != NULL)
									col_ptr_one_type = cur->func_node->result_type;
								else if (cur->math_node != NULL)
									col_ptr_one_type = cur->math_node->result_type;
								else if (cur->case_node != NULL)
									col_ptr_one_type = cur->case_node->result_type;
								else
									col_ptr_one_type = cur->rows_data_type;
							}
							else if ((*where_node)->ptr_one_type == PTR_TYPE_TABLE_COLS_INFO)
								col_ptr_one_type = ((struct table_cols_info*) (*where_node)->ptr_one)->data_type;
							else if ((*where_node)->ptr_one_type == PTR_TYPE_MATH_NODE)
								col_ptr_one_type = ((struct math_node*) (*where_node)->ptr_one)->result_type;
							else if ((*where_node)->ptr_one_type == PTR_TYPE_FUNC_NODE)
								col_ptr_one_type = ((struct func_node*) (*where_node)->ptr_one)->result_type;
							else if ((*where_node)->ptr_one_type == PTR_TYPE_CASE_NODE)
								col_ptr_one_type = ((struct case_node*) (*where_node)->ptr_one)->result_type;

							if ((*where_node)->ptr_one_type != PTR_TYPE_INT && col_ptr_one_type != DATA_INT)
							{
								if (the_debug == YES_DEBUG)
									printf("	ERROR in parseOneWhereNode() at line %d in %s\n", __LINE__, __FILE__);
								printf("An inequality was declared with mismatching datatypes. Please declare matching datatypes. Failing word: \"%s\"\n", word);
								errorTeardown(NULL, malloced_head, the_debug);
								*where_node = NULL;
								return RETURN_ERROR;
							}
						// END Check if matching datatype
					}
					else
					{
						// Char but column name, so get column
						char* alias = NULL;
						char* name = upper(word, NULL, malloced_head, the_debug);
						if (name == NULL)
						{
							if (the_debug == YES_DEBUG)
								printf("	ERROR in parseOneWhereNode() at line %d in %s\n", __LINE__, __FILE__);
							*where_node = NULL;
							return RETURN_ERROR;
						}

						int index_of_dot = indexOf(name, '.');
						if (index_of_dot > -1)
						{
							alias = substring(name, 0, index_of_dot-1, NULL, malloced_head, the_debug);
							char* new_name = substring(name, index_of_dot+1, strLength(name)-1, NULL, malloced_head, the_debug);
							if (alias == NULL || new_name == NULL)
							{
								if (the_debug == YES_DEBUG)
									printf("	ERROR in parseOneWhereNode() at line %d in %s\n", __LINE__, __FILE__);
								*where_node = NULL;
								return RETURN_ERROR;
							}

							myFree((void**) &name, NULL, malloced_head, the_debug);

							name = new_name;
						}
						if (the_debug == YES_DEBUG)
							printf("alias.name = _%s.%s_\n", alias, name);

						if (strcmp_Upper(word, "AVG") == 0 ||
							strcmp_Upper(word, "COUNT") == 0 ||
							strcmp_Upper(word, "FIRST") == 0 ||
							strcmp_Upper(word, "LAST") == 0 ||
							strcmp_Upper(word, "MIN") == 0 ||
							strcmp_Upper(word, "MAX") == 0 ||
							strcmp_Upper(word, "MEDIAN") == 0 ||
							strcmp_Upper(word, "SUM") == 0)
						{
							char* col_name = upper(word, NULL, malloced_head, the_debug);
							char* temp_func_string = (char*) myMalloc(sizeof(char) * 1000, NULL, malloced_head, the_debug);
							if (col_name == NULL || temp_func_string == NULL)
							{
								if (the_debug == YES_DEBUG)
									printf("	ERROR in parseOneWhereNode() at line %d in %s\n", __LINE__, __FILE__);
								*where_node = NULL;
								return RETURN_ERROR;
							}

							strcpy(temp_func_string, word);
							strcat(temp_func_string, " ");

							while (getNextWord(input, word, &index) == 0 && word[0] != ';' && word[0] != ')')
							{
								strcat(temp_func_string, word);
								strcat(temp_func_string, " ");
							}

							if (word[0] != ')')
							{
								if (the_debug == YES_DEBUG)
									printf("	ERROR in parseOneWhereNode() at line %d in %s\n", __LINE__, __FILE__);
								printf("A closing parentheses was expected after the declaration of a function and the opening parentheses. Failing word: \"%s\"\n", word);
								errorTeardown(NULL, malloced_head, the_debug);
								*where_node = NULL;
								return RETURN_ERROR;
							}

							strcat(temp_func_string, ")");

							(*where_node)->ptr_two = NULL;

							int index = 0;

							if (parseFuncNode((struct func_node**) &(*where_node)->ptr_two, NULL, &col_name, temp_func_string, word, &index, select_node, false
				 							 ,malloced_head, the_debug) != RETURN_GOOD)
							{
								*where_node = NULL;
								return RETURN_ERROR;
							}

							(*where_node)->ptr_two_type = PTR_TYPE_FUNC_NODE;

							myFree((void**) &temp_func_string, NULL, malloced_head, the_debug);


							// START Check if func already exists as column
								if (select_node != NULL)
								{
									for (int a=0; a<select_node->columns_arr_size; a++)
									{
										if (select_node->columns_arr[a]->func_node != NULL)
										{
											if (select_node->columns_arr[a]->func_node->which_func == ((struct func_node*) (*where_node)->ptr_one)->which_func
												&& select_node->columns_arr[a]->func_node->distinct == ((struct func_node*) (*where_node)->ptr_one)->distinct
												&& select_node->columns_arr[a]->func_node->args_size == ((struct func_node*) (*where_node)->ptr_one)->args_size)
											{
												for (int b=0; b<select_node->columns_arr[a]->func_node->args_size; b++)
												{
													if (select_node->columns_arr[a]->func_node->args_arr_type[b] != ((struct func_node*) (*where_node)->ptr_two)->args_arr_type[b]
														|| select_node->columns_arr[a]->func_node->args_arr[b] != ((struct func_node*) (*where_node)->ptr_two)->args_arr[b])
														break;

													if (b == select_node->columns_arr[a]->func_node->args_size-1)
													{
														// START Replace ptr_two with col ptr
															freeAnyLinkedList((void**) &(*where_node)->ptr_two, PTR_TYPE_FUNC_NODE, NULL, malloced_head, the_debug);

															(*where_node)->ptr_two = select_node->columns_arr[a];
															(*where_node)->ptr_two_type = PTR_TYPE_COL_IN_SELECT_NODE;
														// END Replace ptr_two with col ptr
													}
												}
											}
										}
									}
								}									
							// END Check if func already exists as column
						}
						else if (select_node != NULL &&
								 getColInSelectNodeFromName((void*) (*where_node), PTR_TYPE_WHERE_CLAUSE_NODE, 2, 1, select_node, alias, name
														   ,malloced_head, the_debug) != RETURN_GOOD)
						{
							printf("A column was declared which could not be found. Failing word: ""%s%s%s""\n", alias == NULL ? "" : alias, alias == NULL ? "" : ".", name);
							errorTeardown(NULL, malloced_head, the_debug);
							*where_node = NULL;
							return RETURN_ERROR;
						}
						else if (table != NULL)
						{
							if (the_debug == YES_DEBUG)
								printf("Finding _%s_ in table_cols_head\n", name);
							struct table_cols_info* cur_col = table->table_cols_head;
							while (cur_col != NULL)
							{
								if (strcmp_Upper(name, cur_col->col_name) == 0)
								{
									if (the_debug == YES_DEBUG)
										printf("Found it\n");
									(*where_node)->ptr_two = cur_col;
									(*where_node)->ptr_two_type = PTR_TYPE_TABLE_COLS_INFO;
									break;
								}

								cur_col = cur_col->next;
							}

							if (cur_col == NULL)
							{
								if (the_debug == YES_DEBUG)
									printf("	ERROR in parseOneWhereNode() at line %d in %s\n", __LINE__, __FILE__);
								printf("This is an error which should only appear in developer testing. Failing word: \"%s\"\n", word);
								errorTeardown(NULL, malloced_head, the_debug);
								*where_node = NULL;
								return RETURN_ERROR;
							}
						}

						if (alias != NULL)
							myFree((void**) &alias, NULL, malloced_head, the_debug);
						myFree((void**) &name, NULL, malloced_head, the_debug);

						// START Check if found col has valid datatype
						if ((*where_node)->ptr_one_type != PTR_TYPE_COL_IN_SELECT_NODE && (*where_node)->ptr_two_type != PTR_TYPE_TABLE_COLS_INFO 
							&& ((struct table_cols_info*) ((struct col_in_select_node*) (*where_node)->ptr_two)->col_ptr)->data_type != (*where_node)->ptr_one_type)
						{
							if (the_debug == YES_DEBUG)
								printf("	ERROR in parseOneWhereNode() at line %d in %s\n", __LINE__, __FILE__);
							printf("An inequality was declared with mismatching datatypes. Please declare matching datatypes. Failing word: \"%s\"\n", word);
							errorTeardown(NULL, malloced_head, the_debug);
							*where_node = NULL;
							return RETURN_ERROR;
						}

						if ((*where_node)->ptr_one_type == PTR_TYPE_COL_IN_SELECT_NODE)
						{
							if (((struct col_in_select_node*) (*where_node)->ptr_one)->rows_data_type != ((struct col_in_select_node*) (*where_node)->ptr_two)->rows_data_type)
							{
								if (the_debug == YES_DEBUG)
									printf("	ERROR in parseOneWhereNode() at line %d in %s\n", __LINE__, __FILE__);
								printf("An inequality was declared with mismatching datatypes. Please declare matching datatypes. Failing word: \"%s\"\n", word);
								errorTeardown(NULL, malloced_head, the_debug);
								*where_node = NULL;
								return RETURN_ERROR;
							}
						}
						// END Check if found col has valid datatype
					}
				}
			// END Determine type of next word and put into ptr_two
		}
		else if ((*where_node)->ptr_one == NULL)
		{
			// START Determine type of next word and put into ptr_one
				if (the_debug == YES_DEBUG)
					printf("	Assigning to ptr_one for where\n");

				if (word[0] == 39)
				{
					(*where_node)->ptr_one = substring(word, 1, strLength(word)-2, NULL, malloced_head, the_debug);
					if ((*where_node)->ptr_one == NULL)
					{
						if (the_debug == YES_DEBUG)
							printf("	ERROR in parseOneWhereNode() at line %d in %s\n", __LINE__, __FILE__);
						*where_node = NULL;
						return RETURN_ERROR;
					}

					if (isDate((char*) (*where_node)->ptr_one))
						(*where_node)->ptr_one_type = PTR_TYPE_DATE;
					else
						(*where_node)->ptr_one_type = PTR_TYPE_CHAR;
				}
				else
				{
					int hmm_int = atoi(word);

					if (hmm_int != 0 && strcontains(word, '.'))
					{
						// Double
						(*where_node)->ptr_one = myMalloc(sizeof(double), NULL, malloced_head, the_debug);
						if ((*where_node)->ptr_one == NULL)
						{
							if (the_debug == YES_DEBUG)
								printf("	ERROR in parseOneWhereNode() at line %d in %s\n", __LINE__, __FILE__);
							*where_node = NULL;
							return RETURN_ERROR;
						}

						double hmm_double = 0.0;

						sscanf(word, "%lf", &hmm_double);

						*((double*) (*where_node)->ptr_one) = hmm_double;

						(*where_node)->ptr_one_type = PTR_TYPE_REAL;
					}
					else if (hmm_int != 0 || word[0] == '0')
					{
						// Int
						(*where_node)->ptr_one = myMalloc(sizeof(int), NULL, malloced_head, the_debug);
						if ((*where_node)->ptr_one == NULL)
						{
							if (the_debug == YES_DEBUG)
								printf("	ERROR in parseOneWhereNode() at line %d in %s\n", __LINE__, __FILE__);
							*where_node = NULL;
							return RETURN_ERROR;
						}

						*((int*) (*where_node)->ptr_one) = hmm_int;

						(*where_node)->ptr_one_type = PTR_TYPE_INT;
					}
					else
					{
						// Char but column name, so get column
						char* alias = NULL;
						char* name = upper(word, NULL, malloced_head, the_debug);
						if (name == NULL)
						{
							if (the_debug == YES_DEBUG)
								printf("	ERROR in parseOneWhereNode() at line %d in %s\n", __LINE__, __FILE__);
							*where_node = NULL;
							return RETURN_ERROR;
						}

						int index_of_dot = indexOf(name, '.');
						if (index_of_dot > -1)
						{
							alias = substring(name, 0, index_of_dot-1, NULL, malloced_head, the_debug);
							char* new_name = substring(name, index_of_dot+1, strLength(name)-1, NULL, malloced_head, the_debug);
							if (alias == NULL || new_name == NULL)
							{
								if (the_debug == YES_DEBUG)
									printf("	ERROR in parseOneWhereNode() at line %d in %s\n", __LINE__, __FILE__);
								*where_node = NULL;
								return RETURN_ERROR;
							}

							myFree((void**) &name, NULL, malloced_head, the_debug);

							name = new_name;
						}
						if (the_debug == YES_DEBUG)
							printf("alias.name = _%s.%s_\n", alias, name);

						if (strcmp_Upper(word, "AVG") == 0 ||
							strcmp_Upper(word, "COUNT") == 0 ||
							strcmp_Upper(word, "FIRST") == 0 ||
							strcmp_Upper(word, "LAST") == 0 ||
							strcmp_Upper(word, "MIN") == 0 ||
							strcmp_Upper(word, "MAX") == 0 ||
							strcmp_Upper(word, "MEDIAN") == 0 ||
							strcmp_Upper(word, "SUM") == 0)
						{
							char* col_name = upper(word, NULL, malloced_head, the_debug);
							char* temp_func_string = (char*) myMalloc(sizeof(char) * 1000, NULL, malloced_head, the_debug);
							if (col_name == NULL || temp_func_string == NULL)
							{
								if (the_debug == YES_DEBUG)
									printf("	ERROR in parseOneWhereNode() at line %d in %s\n", __LINE__, __FILE__);
								*where_node = NULL;
								return RETURN_ERROR;
							}

							strcpy(temp_func_string, word);
							strcat(temp_func_string, " ");

							while (getNextWord(input, word, &index) == 0 && word[0] != ';' && word[0] != ')')
							{
								strcat(temp_func_string, word);
								strcat(temp_func_string, " ");
							}

							if (word[0] != ')')
							{
								if (the_debug == YES_DEBUG)
									printf("	ERROR in parseOneWhereNode() at line %d in %s\n", __LINE__, __FILE__);
								printf("A closing parentheses was expected after the declaration of a function and the opening parentheses. Failing word: \"%s\"\n", word);
								errorTeardown(NULL, malloced_head, the_debug);
								*where_node = NULL;
								return RETURN_ERROR;
							}

							strcat(temp_func_string, ")");

							(*where_node)->ptr_one = NULL;

							int index = indexOf(temp_func_string, '(')+1;

							if (parseFuncNode((struct func_node**) &(*where_node)->ptr_one, NULL, &col_name, temp_func_string, word, &index, select_node, false
				 							 ,malloced_head, the_debug) != RETURN_GOOD)
							{
								*where_node = NULL;
								return RETURN_ERROR;
							}

							(*where_node)->ptr_one_type = PTR_TYPE_FUNC_NODE;

							myFree((void**) &temp_func_string, NULL, malloced_head, the_debug);

							// START Check if func already exists as column
								if (select_node != NULL)
								{
									for (int a=0; a<select_node->columns_arr_size; a++)
									{
										if (select_node->columns_arr[a]->func_node != NULL)
										{
											if (select_node->columns_arr[a]->func_node->which_func == ((struct func_node*) (*where_node)->ptr_one)->which_func
												&& select_node->columns_arr[a]->func_node->distinct == ((struct func_node*) (*where_node)->ptr_one)->distinct
												&& select_node->columns_arr[a]->func_node->args_size == ((struct func_node*) (*where_node)->ptr_one)->args_size)
											{
												for (int b=0; b<select_node->columns_arr[a]->func_node->args_size; b++)
												{
													if (select_node->columns_arr[a]->func_node->args_arr_type[b] != ((struct func_node*) (*where_node)->ptr_one)->args_arr_type[b]
														|| select_node->columns_arr[a]->func_node->args_arr[b] != ((struct func_node*) (*where_node)->ptr_one)->args_arr[b])
														break;

													if (b == select_node->columns_arr[a]->func_node->args_size-1)
													{
														// START Replace ptr_one with col ptr
															freeAnyLinkedList((void**) &(*where_node)->ptr_one, PTR_TYPE_FUNC_NODE, NULL, malloced_head, the_debug);

															(*where_node)->ptr_one = select_node->columns_arr[a];
															(*where_node)->ptr_one_type = PTR_TYPE_COL_IN_SELECT_NODE;
														// END Replace ptr_one with col ptr
													}
												}
											}
										}
									}
								}									
							// END Check if func already exists as column
						}
						else if (select_node != NULL &&
								 getColInSelectNodeFromName((void*) (*where_node), PTR_TYPE_WHERE_CLAUSE_NODE, 1, 1, select_node, alias, name
														   ,malloced_head, the_debug) != RETURN_GOOD)
						{
							printf("A column was declared which could not be found. Failing word: ""%s%s%s""\n", alias == NULL ? "" : alias, alias == NULL ? "" : ".", name);
							errorTeardown(NULL, malloced_head, the_debug);
							*where_node = NULL;
							return RETURN_ERROR;
						}
						else if (table != NULL)
						{
							if (the_debug == YES_DEBUG)
								printf("Finding _%s_ in table_cols_head\n", name);
							struct table_cols_info* cur_col = table->table_cols_head;
							while (cur_col != NULL)
							{
								if (strcmp_Upper(name, cur_col->col_name) == 0)
								{
									if (the_debug == YES_DEBUG)
										printf("Found it\n");
									(*where_node)->ptr_one = cur_col;
									(*where_node)->ptr_one_type = PTR_TYPE_TABLE_COLS_INFO;
									break;
								}

								cur_col = cur_col->next;
							}

							if (cur_col == NULL)
							{
								if (the_debug == YES_DEBUG)
									printf("	ERROR in parseOneWhereNode() at line %d in %s\n", __LINE__, __FILE__);
								printf("This is an error which should only appear in developer testing. Failing word: \"%s\"\n", word);
								errorTeardown(NULL, malloced_head, the_debug);
								*where_node = NULL;
								return RETURN_ERROR;
							}
						}

						if (alias != NULL)
							myFree((void**) &alias, NULL, malloced_head, the_debug);
						myFree((void**) &name, NULL, malloced_head, the_debug);
					}
				}	
			// END Determine type of next word and put into ptr_two
		}
		else
		{
			if (the_debug == YES_DEBUG)
				printf("	ERROR in parseOneWhereNode() at line %d in %s\n", __LINE__, __FILE__);
			printf("One or multiple operands in a where clause were not declared. Please declare two, on either side of the operation. Failing word: \"%s\"\n", word);
			errorTeardown(NULL, malloced_head, the_debug);
			*where_node = NULL;
			return RETURN_ERROR;
		}

		if (prev_word != NULL)
			myFree((void**) &prev_word, NULL, malloced_head, the_debug);
		
		if (!(word[0] == '=' || word[0] == '>' || word[0] == '<' || strcmp_Upper(word, "IS") == 0))
		{
			prev_word = upper(word, NULL, malloced_head, the_debug);
			if (prev_word == NULL)
			{
				if (the_debug == YES_DEBUG)
					printf("	ERROR in parseOneWhereNode() at line %d in %s\n", __LINE__, __FILE__);
				*where_node = NULL;
				return RETURN_ERROR;
			}
		}
	}

	if ((((*where_node)->ptr_one_type == -1 || (*where_node)->ptr_two_type == -1) && (*where_node)->where_type != WHERE_IS_NULL && (*where_node)->where_type != WHERE_IS_NOT_NULL))
	{
		if (the_debug == YES_DEBUG)
			printf("	ERROR in parseOneWhereNode() at line %d in %s\n", __LINE__, __FILE__);
		printf("One or multiple operands in a where clause were not declared. Please declare two, on either side of the operation. Failing word: \"%s\"\n", word);
		errorTeardown(NULL, malloced_head, the_debug);
		*where_node = NULL;
		return RETURN_ERROR;
	}

	myFree((void**) &prev_word, NULL, malloced_head, the_debug);

	return RETURN_GOOD;
}

/*	RETURNS:
 *  RETURN_ERROR if error
 *	RETURN_GOOD if good
 *
 *	WRITES TO:
 *	where_head
 */
int parseWhereClause(char* input, struct where_clause_node** where_head, struct select_node* select_node, struct table_info* table, char* first_word
				    ,struct malloced_node** malloced_head, int the_debug)
{
	if (input[0] == 0)
		return RETURN_GOOD;

	// START Allocate space for where_head and initialize
		if (*where_head == NULL)
		{
			*where_head = (struct where_clause_node*) myMalloc(sizeof(struct where_clause_node), NULL, malloced_head, the_debug);
			if (*where_head == NULL)
			{
				if (the_debug == YES_DEBUG)
					printf("	ERROR in parseWhereClause() at line %d in %s\n", __LINE__, __FILE__);
				*where_head = NULL;
				return RETURN_ERROR;
			}

			(*where_head)->ptr_one = NULL;
			(*where_head)->ptr_one_type = -1;

			(*where_head)->ptr_two = NULL; 
			(*where_head)->ptr_two_type = -1;

			(*where_head)->where_type = -1;

			(*where_head)->parent = NULL;
		}
	// END Allocate space for where_head and initialize

	// START Init word variable
		char* word = (char*) myMalloc(sizeof(char) * 200, NULL, malloced_head, the_debug);
		if (word == NULL)
		{
			if (the_debug == YES_DEBUG)
				printf("	ERROR in parseWhereClause() at line %d in %s\n", __LINE__, __FILE__);
			*where_head = NULL;
			return RETURN_ERROR;
		}
		word[0] = 0;
		int index = 0;
	// END Init word variable

	// START See if first word is WHERE or ON or WHEN or HAVING
		int compared_having = -2;

		if (first_word != NULL)
		{
			getNextWord(input, word, &index);

			int compared_where = strcmp_Upper(word, "WHERE");
			int compared_on = strcmp_Upper(word, "ON");
			int compared_when = strcmp_Upper(word, "WHEN");
			compared_having = strcmp_Upper(word, "HAVING");

			if ((compared_where != 0 && strcmp(first_word, "where") == 0) || (compared_on != 0 && strcmp(first_word, "on") == 0) || (compared_when != 0 && strcmp(first_word, "when") == 0))
			{
				if (the_debug == YES_DEBUG)
					printf("	ERROR in parseWhereClause() at line %d in %s\n", __LINE__, __FILE__);
				printf("The first word of a where clause was NOT one of the following: WHERE, ON, WHEN, or HAVING. Failing word: \"%s\"\n", word);
				errorTeardown(NULL, malloced_head, the_debug);
				*where_head = NULL;
				return RETURN_ERROR;
			}
		}
	// END See if first word is WHERE or ON or WHEN or HAVING

	int open_parens = 0;

	char* part_of_input = (char*) myMalloc(sizeof(char) * 1000, NULL, malloced_head, the_debug);
	if (part_of_input == NULL)
	{
		if (the_debug == YES_DEBUG)
			printf("	ERROR in parseWhereClause() at line %d in %s\n", __LINE__, __FILE__);
		*where_head = NULL;
		return RETURN_ERROR;
	}
	part_of_input[0] = 0;

	int specials_done = 0;
	
	while(getNextWord(input, word, &index) == 0 && word[0] != ';')
	{
		redoDoubleQuotes(word);

		if (the_debug == YES_DEBUG)
			printf("word in parseWhereClause loop = _%s_\n", word);
		
		if (open_parens == 0 && word[0] == '(' && compared_having != 0)
		{
			open_parens++;
		}
		else if (open_parens == 1 && word[0] == ')' && (strContainsWordUpper(part_of_input, " AND ") || strContainsWordUpper(part_of_input, " OR ")))
		{
			// START Recursively call this function based on which ptr is null
				open_parens--;
				
				if (the_debug == YES_DEBUG)
					printf("Found parentheses, part_of_input = _%s_\n", part_of_input);
				// Call parseWhereClause()

				if ((*where_head)->ptr_one == NULL)
				{
					if (the_debug == YES_DEBUG)
						printf("	Recursively calling in ptr_one\n");
					(*where_head)->ptr_one = (struct where_clause_node*) myMalloc(sizeof(struct where_clause_node), NULL, malloced_head, the_debug);
					if ((*where_head)->ptr_one == NULL)
					{
						if (the_debug == YES_DEBUG)
							printf("	ERROR in parseWhereClause() at line %d in %s\n", __LINE__, __FILE__);
						*where_head = NULL;
						return RETURN_ERROR;
					}
					(*where_head)->ptr_one_type = PTR_TYPE_WHERE_CLAUSE_NODE;

					((struct where_clause_node*) (*where_head)->ptr_one)->ptr_one = NULL;
					((struct where_clause_node*) (*where_head)->ptr_one)->ptr_one_type = -1;

					((struct where_clause_node*) (*where_head)->ptr_one)->ptr_two = NULL;
					((struct where_clause_node*) (*where_head)->ptr_one)->ptr_two_type = -1;

					((struct where_clause_node*) (*where_head)->ptr_one)->where_type = -1;
					((struct where_clause_node*) (*where_head)->ptr_one)->parent = *where_head;


					if (parseWhereClause(part_of_input, (struct where_clause_node**) &(*where_head)->ptr_one, select_node, NULL, NULL
									    ,malloced_head, the_debug) != RETURN_GOOD)
					{
						if (the_debug == YES_DEBUG)
							printf("	ERROR in parseWhereClause() at line %d in %s\n", __LINE__, __FILE__);
						*where_head = NULL;
						return RETURN_ERROR;
					}
				}
				else if ((*where_head)->ptr_two == NULL)
				{
					if (the_debug == YES_DEBUG)
						printf("	Recursively calling in ptr_two\n");
					(*where_head)->ptr_two = (struct where_clause_node*) myMalloc(sizeof(struct where_clause_node), NULL, malloced_head, the_debug);
					if ((*where_head)->ptr_two == NULL)
					{
						if (the_debug == YES_DEBUG)
							printf("	ERROR in parseWhereClause() at line %d in %s\n", __LINE__, __FILE__);
						*where_head = NULL;
						return RETURN_ERROR;
					}
					(*where_head)->ptr_two_type = PTR_TYPE_WHERE_CLAUSE_NODE;

					((struct where_clause_node*) (*where_head)->ptr_two)->ptr_one = NULL;
					((struct where_clause_node*) (*where_head)->ptr_two)->ptr_one_type = -1;

					((struct where_clause_node*) (*where_head)->ptr_two)->ptr_two = NULL;
					((struct where_clause_node*) (*where_head)->ptr_two)->ptr_two_type = -1;

					((struct where_clause_node*) (*where_head)->ptr_two)->where_type = -1;
					((struct where_clause_node*) (*where_head)->ptr_two)->parent = *where_head;


					if (parseWhereClause(part_of_input, (struct where_clause_node**) &(*where_head)->ptr_two, select_node, NULL, NULL
									    ,malloced_head, the_debug) != RETURN_GOOD)
					{
						if (the_debug == YES_DEBUG)
							printf("	ERROR in parseWhereClause() at line %d in %s\n", __LINE__, __FILE__);
						*where_head = NULL;
						return RETURN_ERROR;
					}
				}

				if (the_debug == YES_DEBUG)
					printf("	Back from parseWhereClause()\n");

				getNextWord(input, word, &index);

				if (strcmp_Upper(word, "OR") == 0)
					(*where_head)->where_type = WHERE_OR;
				else if (strcmp_Upper(word, "AND") == 0)
					(*where_head)->where_type = WHERE_AND;

				part_of_input[0] = 0;
			// END Recursively call this function based on which ptr is null
		}
		else if (open_parens == 0 && (strcmp_Upper(word, "OR") == 0 || strcmp_Upper(word, "AND") == 0))
		{
			// START Create new where node and initialize it
				if (the_debug == YES_DEBUG)
					printf("Found _%s_, part_of_input = _%s_\n", word, part_of_input);

				struct where_clause_node* new_where = (struct where_clause_node*) myMalloc(sizeof(struct where_clause_node), NULL, malloced_head, the_debug);
				if (new_where == NULL)
				{
					if (the_debug == YES_DEBUG)
						printf("	ERROR in parseWhereClause() at line %d in %s\n", __LINE__, __FILE__);
					*where_head = NULL;
					return RETURN_ERROR;
				}

				new_where->ptr_one = NULL;
				new_where->ptr_one_type = -1;

				new_where->ptr_two = NULL;
				new_where->ptr_two_type = -1;

				new_where->where_type = -1;


				char* temp_word = (char*) myMalloc(sizeof(char) * (strLength(word)+1), NULL, malloced_head, the_debug);
				if (temp_word == NULL)
				{
					if (the_debug == YES_DEBUG)
						printf("	ERROR in parseWhereClause() at line %d in %s\n", __LINE__, __FILE__);
					*where_head = NULL;
					return RETURN_ERROR;
				}
				strcpy(temp_word, word);


				if (parseOneWhereNode(part_of_input, word, &new_where, select_node, table, malloced_head, the_debug) != RETURN_GOOD)
				{
					if (the_debug == YES_DEBUG)
						printf("	ERROR in parseWhereClause() at line %d in %s\n", __LINE__, __FILE__);
					*where_head = NULL;
					return RETURN_ERROR;
				}


				if ((*where_head)->ptr_one == NULL)
				{
					if (the_debug == YES_DEBUG)
						printf("	Assigning new_where to ptr_one\n");
					(*where_head)->ptr_one = new_where;
					(*where_head)->ptr_one_type = PTR_TYPE_WHERE_CLAUSE_NODE;

					if (strcmp_Upper(temp_word, "OR") == 0)
						(*where_head)->where_type = WHERE_OR;
					else if (strcmp_Upper(temp_word, "AND") == 0)
						(*where_head)->where_type = WHERE_AND;

					((struct where_clause_node*) (*where_head)->ptr_one)->parent = *where_head;
				}
				else if ((*where_head)->ptr_two == NULL)
				{
					if (the_debug == YES_DEBUG)
						printf("	Assigning new_where to ptr_two\n");
					(*where_head)->ptr_two = new_where;
					(*where_head)->ptr_two_type = PTR_TYPE_WHERE_CLAUSE_NODE;

					((struct where_clause_node*) (*where_head)->ptr_two)->parent = *where_head;
				}
			// END Create new where node and initialize it

			// START Decide how to format the tree given there will be a new node
				if ((*where_head)->ptr_two != NULL && strcmp_Upper(temp_word, "AND") == 0 && (*where_head)->where_type == WHERE_OR)
				{
					if (the_debug == YES_DEBUG)
						printf("	Special option\n");
					struct where_clause_node* temp_where = (struct where_clause_node*) myMalloc(sizeof(struct where_clause_node), NULL, malloced_head, the_debug);
					if (temp_where == NULL)
					{
						if (the_debug == YES_DEBUG)
							printf("	ERROR in parseWhereClause() at line %d in %s\n", __LINE__, __FILE__);
						*where_head = NULL;
						return RETURN_ERROR;
					}

					temp_where->ptr_one = (*where_head)->ptr_two;
					temp_where->ptr_one_type = PTR_TYPE_WHERE_CLAUSE_NODE;

					temp_where->ptr_two = NULL;
					temp_where->ptr_two_type = -1;

					temp_where->where_type = WHERE_AND;

					temp_where->parent = *where_head;


					((struct where_clause_node*) (*where_head)->ptr_two)->parent = temp_where;


					(*where_head)->ptr_two = temp_where;
					(*where_head)->ptr_two_type = PTR_TYPE_WHERE_CLAUSE_NODE;


					*where_head = temp_where;


					specials_done++;
				}
				else if ((*where_head)->ptr_two != NULL)
				{
					if (the_debug == YES_DEBUG)
						printf("	Default option\n");
					struct where_clause_node* temp_where = (struct where_clause_node*) myMalloc(sizeof(struct where_clause_node), NULL, malloced_head, the_debug);
					if (temp_where == NULL)
					{
						if (the_debug == YES_DEBUG)
							printf("	ERROR in parseWhereClause() at line %d in %s\n", __LINE__, __FILE__);
						*where_head = NULL;
						return RETURN_ERROR;
					}

					temp_where->ptr_one = *where_head;
					temp_where->ptr_one_type = PTR_TYPE_WHERE_CLAUSE_NODE;

					temp_where->ptr_two = NULL;
					temp_where->ptr_two_type = -1;

					if (strcmp_Upper(temp_word, "OR") == 0)
						temp_where->where_type = WHERE_OR;
					else if (strcmp_Upper(temp_word, "AND") == 0)
						temp_where->where_type = WHERE_AND;

					temp_where->parent = (*where_head)->parent;

					if (temp_where->parent != NULL)
					{
						if (temp_where->parent->ptr_one == NULL)
						{
							if (the_debug == YES_DEBUG)
								printf("This should never happen\n");
							temp_where->parent->ptr_one = temp_where;
							temp_where->parent->ptr_one_type = PTR_TYPE_WHERE_CLAUSE_NODE;
						}
						else
						{
							temp_where->parent->ptr_two = temp_where;
							temp_where->parent->ptr_two_type = PTR_TYPE_WHERE_CLAUSE_NODE;
						}
					}

					(*where_head)->parent = temp_where;

					//((struct where_clause_node*) (*where_head)->parent->ptr_two)->parent = temp_where;

					*where_head = temp_where;
				}
			// END Decide how to format the tree given there will be a new node

			myFree((void**) &temp_word, NULL, malloced_head, the_debug);

			if (the_debug == YES_DEBUG)
			{
				printf("where_head ptr_one_type = %d\n", (*where_head)->ptr_one_type);
				printf("where_head ptr_two_type = %d\n", (*where_head)->ptr_two_type);
				printf("where_head where_type = %d\n", (*where_head)->where_type);

				printf("new_where ptr_one_type = %d\n", new_where->ptr_one_type);
				printf("new_where ptr_two_type = %d\n", new_where->ptr_two_type);
				printf("new_where where_type = %d\n", new_where->where_type);

				if ((*where_head)->ptr_one == new_where)
					printf("(*where_head)->ptr_one == new_where\n");
				else if ((*where_head)->ptr_two == new_where)
					printf("(*where_head)->ptr_two == new_where\n");
			}

			part_of_input[0] = 0;
		}
		else
		{
			// START Concatenate word to current part_of_input
				if (word[0] == '(')
					open_parens++;
				else if (word[0] == ')')
					open_parens--;

				strcat(part_of_input, word);
				if (input[index] == '.')
				{
					if (part_of_input[strLength(part_of_input)-1] == ' ')
						part_of_input[strLength(part_of_input)-1] = '.';
					else
						strcat(part_of_input, ".");
					index++;
				}
				else
					strcat(part_of_input, " ");
			// END Concatenate word to current part_of_input
		}
	}

	if (part_of_input[0] != 0)
	{
		// START End of input, assign a new where node based on which ptrs are null
			if (strcontains(part_of_input, ')') && !strcontains(part_of_input, '('))
			{
				char* temp_str = (char*) myMalloc(sizeof(char) * (strLength(part_of_input)+2), NULL, malloced_head, the_debug);
				if (temp_str == NULL)
				{
					if (the_debug == YES_DEBUG)
						printf("	ERROR in parseWhereClause() at line %d in %s\n", __LINE__, __FILE__);
					*where_head = NULL;
					return RETURN_ERROR;
				}
				strcpy(temp_str, "(");

				strcat(temp_str, part_of_input);

				myFree((void**) &part_of_input, NULL, malloced_head, the_debug);

				part_of_input = temp_str;
			}

			if (the_debug == YES_DEBUG)
				printf("Found END OF INPUT, part_of_input = _%s_\n", part_of_input);

			if ((*where_head)->ptr_one == NULL && (*where_head)->ptr_two == NULL)
			{
				if (the_debug == YES_DEBUG)
					printf("Both ptr_one and ptr_two are NULL\n");
				if (parseOneWhereNode(part_of_input, word, where_head, select_node, table, malloced_head, the_debug) != RETURN_GOOD)
				{
					if (the_debug == YES_DEBUG)
						printf("	ERROR in parseWhereClause() at line %d in %s\n", __LINE__, __FILE__);
					*where_head = NULL;
					return RETURN_ERROR;
				}
			}
			else if ((*where_head)->ptr_one != NULL && (*where_head)->ptr_two == NULL)
			{
				if (the_debug == YES_DEBUG)
					printf("ptr_two is NULL so making new where_clause_node and assigning to ptr_two\n");
				struct where_clause_node* new_where = (struct where_clause_node*) myMalloc(sizeof(struct where_clause_node), NULL, malloced_head, the_debug);
				if (new_where == NULL)
				{
					if (the_debug == YES_DEBUG)
						printf("	ERROR in parseWhereClause() at line %d in %s\n", __LINE__, __FILE__);
					*where_head = NULL;
					return RETURN_ERROR;
				}

				new_where->ptr_one = NULL;
				new_where->ptr_one_type = -1;

				new_where->ptr_two = NULL;
				new_where->ptr_two_type = -1;

				new_where->where_type = -1;

				if (parseOneWhereNode(part_of_input, word, &new_where, select_node, table, malloced_head, the_debug) != 0)
				{
					if (the_debug == YES_DEBUG)
						printf("	ERROR in parseWhereClause() at line %d in %s\n", __LINE__, __FILE__);
					*where_head = NULL;
					return RETURN_ERROR;
				}


				(*where_head)->ptr_two = new_where;
				(*where_head)->ptr_two_type = PTR_TYPE_WHERE_CLAUSE_NODE;

				((struct where_clause_node*) (*where_head)->ptr_two)->parent = *where_head;

				if (the_debug == YES_DEBUG)
				{
					printf("where_head ptr_one_type = %d\n", (*where_head)->ptr_one_type);
					printf("where_head ptr_two_type = %d\n", (*where_head)->ptr_two_type);
					printf("where_head where_type = %d\n", (*where_head)->where_type);

					printf("new_where ptr_one_type = %d\n", new_where->ptr_one_type);
					printf("new_where ptr_two_type = %d\n", new_where->ptr_two_type);
					printf("new_where where_type = %d\n", new_where->where_type);

					if ((*where_head)->ptr_one == new_where)
						printf("(*where_head)->ptr_one == new_where\n");
					else if ((*where_head)->ptr_two == new_where)
						printf("(*where_head)->ptr_two == new_where\n");
				}
			}

			part_of_input[0] = 0;
		// END End of input, assign a new where node based on which ptrs are null
	}

	while (specials_done > 0)
	{
		specials_done--;

		*where_head = (*where_head)->parent;
	}

	myFree((void**) &word, NULL, malloced_head, the_debug);
	myFree((void**) &part_of_input, NULL, malloced_head, the_debug);

	
	if ((*where_head)->where_type == -1)
	{
		if (the_debug == YES_DEBUG)
			printf("	ERROR in parseWhereClause() at line %d in %s\n", __LINE__, __FILE__);
		errorTeardown(NULL, malloced_head, the_debug);
		*where_head = NULL;
		return RETURN_ERROR;
	}
	else if (((*where_head)->where_type == WHERE_OR || (*where_head)->where_type == WHERE_AND) && ((*where_head)->ptr_one == NULL || (*where_head)->ptr_two == NULL))
	{
		if (the_debug == YES_DEBUG)
			printf("	ERROR in parseWhereClause() at line %d in %s\n", __LINE__, __FILE__);
		errorTeardown(NULL, malloced_head, the_debug);
		*where_head = NULL;
		return RETURN_ERROR;
	}


	return RETURN_GOOD;
}

/*	RETURNS:
 *  NULL if error
 *	A valid struct table_info ptr if good
 *
 *	WRITES TO:
 */
struct table_info* getTableFromName(char* input_table_name, struct malloced_node** malloced_head, int the_debug)
{
	// START Get table name and find table node in list
		struct table_info* cur_table = getTablesHead();
		while (cur_table != NULL)
		{
			if (strcmp_Upper(cur_table->name, input_table_name) == 0)
				break;

			cur_table = cur_table->next;
		}

		if (cur_table == NULL)
		{
			if (the_debug == YES_DEBUG)
				printf("	ERROR in getTableFromName() at line %d in %s\n", __LINE__, __FILE__);
			return NULL;
		}
	// END Get table name and find table node in list

	//printf("table name = %s\n", cur_table->name);

	return cur_table;
}

/*	RETURNS:
 *  RETURN_ERROR if error
 *	RETURN_GOOD if good
 *
 *	WRITES TO:
 *  change_head
 */
int parseUpdate(char* input, struct change_node_v2** change_head, struct malloced_node** malloced_head, int the_debug)
{
	// START Init change_head
		if (*change_head == NULL)
		{
			*change_head = (struct change_node_v2*) myMalloc(sizeof(struct change_node_v2), NULL, malloced_head, the_debug);
			if (*change_head == NULL)
			{
				if (the_debug == YES_DEBUG)
					printf("	ERROR in parseUpdate() at line %d in %s\n", __LINE__, __FILE__);
				*change_head = NULL;
				return RETURN_ERROR;
			}
			(*change_head)->transac_id = -1;
			(*change_head)->table = NULL;
			(*change_head)->operation = -1;
			(*change_head)->total_rows_to_insert = 0;
			(*change_head)->col_list_head = NULL;
			(*change_head)->col_list_tail = NULL;
			(*change_head)->data_list_head = NULL;
			(*change_head)->data_list_tail = NULL;
			(*change_head)->where_head = NULL;
		}
	// END Init change_head

	// START Init word
		char* word = (char*) myMalloc(sizeof(char) * 200, NULL, malloced_head, the_debug);
		if (word == NULL)
		{
			if (the_debug == YES_DEBUG)
				printf("	ERROR in parseUpdate() at line %d in %s\n", __LINE__, __FILE__);
			*change_head = NULL;
			return RETURN_ERROR;
		}
		word[0] = 0;
		int index = 0;
	// END Init word

	// START See if first word is UPDATE
		getNextWord(input, word, &index);

		if (strcmp_Upper(word, "UPDATE") != 0)
		{
			if (the_debug == YES_DEBUG)
				printf("	ERROR in parseUpdate() at line %d in %s\n", __LINE__, __FILE__);
			printf("The first word of a update command was not UPDATE. Failing word: \"%s\"\n", word);
			errorTeardown(NULL, malloced_head, the_debug);
			*change_head = NULL;
			return RETURN_ERROR;
		}

		(*change_head)->operation = OP_UPDATE;
	// END See if first word is UPDATE

	// START Get table
		getNextWord(input, word, &index);

		(*change_head)->table = getTableFromName(word, malloced_head, the_debug);
		if ((*change_head)->table == NULL)
		{
			if (the_debug == YES_DEBUG)
				printf("	ERROR in parseUpdate() at line %d in %s\n", __LINE__, __FILE__);
			printf("A table was declared which could not be found. Failing word: \"%s\"\n", word);
			errorTeardown(NULL, malloced_head, the_debug);
			*change_head = NULL;
			return RETURN_ERROR;
		}

		if (strcmp_Upper((*change_head)->table->name, "LIQUOR_LICENSES_FULL") == 0 || strcmp_Upper((*change_head)->table->name, "LIQUOR_SALES_FULL") == 0)
		{
			if (the_debug == YES_DEBUG)
				printf("	ERROR in parseUpdate() at line %d in %s\n", __LINE__, __FILE__);
			printf("You do not have permission to update this table. Failing word: \"%s\"\n", word);
			errorTeardown(NULL, malloced_head, the_debug);
			*change_head = NULL;
			return RETURN_ERROR;
		}
	// END Get table

	// START See if next word is SET
		getNextWord(input, word, &index);

		if (strcmp_Upper(word, "SET") != 0)
		{
			if (the_debug == YES_DEBUG)
				printf("	ERROR in parseUpdate() at line %d in %s\n", __LINE__, __FILE__);
			printf("The word \"SET\" was expected after declaring a table. Failing word: \"%s\"\n", word);
			errorTeardown(NULL, malloced_head, the_debug);
			*change_head = NULL;
			return RETURN_ERROR;
		}
	// END See if next word is SET


	// START Get all the set col = 'value'
		addListNodePtr(&(*change_head)->col_list_head, &(*change_head)->col_list_tail, NULL, -1, ADDLISTNODE_TAIL
					  ,NULL, malloced_head, the_debug);
		addListNodePtr(&(*change_head)->data_list_head, &(*change_head)->data_list_tail, NULL, -1, ADDLISTNODE_TAIL
					  ,NULL, malloced_head, the_debug);

		while (getNextWord(input, word, &index) == 0 && strcmp_Upper(word, "WHERE") != 0 && word[0] != 0 && word[0] != ';')
		{
			if (the_debug == YES_DEBUG)
				printf("word: _%s_\n", word);
			if (word[0] == '=')
			{
				// Do nothing
			}
			else if (word[0] == 39)
			{
				if ((*change_head)->data_list_tail->ptr_value != NULL || (*change_head)->col_list_tail->ptr_value == NULL)
				{
					if (the_debug == YES_DEBUG)
						printf("	ERROR in parseUpdate() at line %d in %s\n", __LINE__, __FILE__);
					printf("A column which will have its value changed was not specified. Failing word: \"%s\"\n", word);
					errorTeardown(NULL, malloced_head, the_debug);
					*change_head = NULL;
					return RETURN_ERROR;
				}

				if (determineDatatypeOfDataWord(word, &(*change_head)->data_list_tail->ptr_value, &(*change_head)->data_list_tail->ptr_type, NULL, malloced_head, the_debug) != RETURN_GOOD)
				{
					if (the_debug == YES_DEBUG)
						printf("	ERROR in parseUpdate() at line %d in %s\n", __LINE__, __FILE__);
					return RETURN_ERROR;
				}


				if (((struct table_cols_info*) (*change_head)->col_list_tail->ptr_value)->data_type != (*change_head)->data_list_tail->ptr_type)
				{
					if (the_debug == YES_DEBUG)
						printf("	ERROR in parseUpdate() at line %d in %s\n", __LINE__, __FILE__);
					printf("A value which will be used to update a column does not match the column's datatype. Failing word: \"%s\"\n", word);
					errorTeardown(NULL, malloced_head, the_debug);
					*change_head = NULL;
					return RETURN_ERROR;
				}
			}
			else if (word[0] == ',')
			{
				if ((*change_head)->col_list_tail->ptr_value == NULL || &(*change_head)->data_list_tail->ptr_value == NULL)
				{
					if (the_debug == YES_DEBUG)
						printf("	ERROR in parseUpdate() at line %d in %s\n", __LINE__, __FILE__);
					printf("A comma was declared without specifying a column to be updated and the value with which to update the column. Failing word: \"%s\"\n", word);
					errorTeardown(NULL, malloced_head, the_debug);
					*change_head = NULL;
					return RETURN_ERROR;
				}

				addListNodePtr(&(*change_head)->col_list_head, &(*change_head)->col_list_tail, NULL, -1, ADDLISTNODE_TAIL
							  ,NULL, malloced_head, the_debug);
				addListNodePtr(&(*change_head)->data_list_head, &(*change_head)->data_list_tail, NULL, -1, ADDLISTNODE_TAIL
							  ,NULL, malloced_head, the_debug);
			}
			else
			{
				struct table_cols_info* cur_col = (*change_head)->table->table_cols_head;
				while (cur_col != NULL)
				{
					if (strcmp_Upper(word, cur_col->col_name) == 0)
					{
						if ((*change_head)->col_list_tail->ptr_value != NULL)
						{
							if (the_debug == YES_DEBUG)
								printf("	ERROR in parseUpdate() at line %d in %s\n", __LINE__, __FILE__);
							printf("A value which will be used to update a column was not specified. Failing word: \"%s\"\n", word);
							errorTeardown(NULL, malloced_head, the_debug);
							*change_head = NULL;
							return RETURN_ERROR;
						}
						else
						{
							(*change_head)->col_list_tail->ptr_value = cur_col;
							(*change_head)->col_list_tail->ptr_type = PTR_TYPE_TABLE_COLS_INFO;
							break;
						}
					}

					cur_col = cur_col->next;
				}

				if (cur_col == NULL)
				{
					if ((*change_head)->data_list_tail->ptr_value != NULL)
					{
						if (the_debug == YES_DEBUG)
							printf("	ERROR in parseUpdate() at line %d in %s\n", __LINE__, __FILE__);
						errorTeardown(NULL, malloced_head, the_debug);
						*change_head = NULL;
						return RETURN_ERROR;
					}


					if (word[0] == '-')
					{
						char* temp_word = myMalloc(sizeof(char) * 100, NULL, malloced_head, the_debug);
						if (temp_word == NULL)
						{
							if (the_debug == YES_DEBUG)
								printf("	ERROR in parseUpdate() at line %d in %s\n", __LINE__, __FILE__);
							return RETURN_ERROR;
						}
						strcpy(temp_word, word);

						getNextWord(input, word, &index);

						strcat(temp_word, word);

						strcpy(word, temp_word);

						myFree((void**) &temp_word, NULL, malloced_head, the_debug);
					}

					if (input[index] == '.')
					{
						char* temp_word = myMalloc(sizeof(char) * 100, NULL, malloced_head, the_debug);
						if (temp_word == NULL)
						{
							if (the_debug == YES_DEBUG)
								printf("	ERROR in parseUpdate() at line %d in %s\n", __LINE__, __FILE__);
							return RETURN_ERROR;
						}
						strcpy(temp_word, word);
						strcat(temp_word, ".");

						getNextWord(input, word, &index);

						strcat(temp_word, word);

						strcpy(word, temp_word);

						myFree((void**) &temp_word, NULL, malloced_head, the_debug);
					}



					if (determineDatatypeOfDataWord(word, &(*change_head)->data_list_tail->ptr_value, &(*change_head)->data_list_tail->ptr_type, NULL, malloced_head, the_debug) != RETURN_GOOD)
					{
						if (the_debug == YES_DEBUG)
							printf("	ERROR in parseUpdate() at line %d in %s\n", __LINE__, __FILE__);
						return RETURN_ERROR;
					}
						


					if ((*change_head)->col_list_tail->ptr_value == NULL)
					{
						if (the_debug == YES_DEBUG)
							printf("	ERROR in parseUpdate() at line %d in %s\n", __LINE__, __FILE__);
						printf("A column which will have its value changed was not specified. Failing word: \"%s\"\n", word);
						errorTeardown(NULL, malloced_head, the_debug);
						*change_head = NULL;
						return RETURN_ERROR;
					}
					if (((struct table_cols_info*) (*change_head)->col_list_tail->ptr_value)->data_type != (*change_head)->data_list_tail->ptr_type)
					{
						if (the_debug == YES_DEBUG)
							printf("	ERROR in parseUpdate() at line %d in %s\n", __LINE__, __FILE__);
						printf("A value which will be used to update a column was not specified or does not match the column's datatype. Failing word: \"%s\"\n", word);
						errorTeardown(NULL, malloced_head, the_debug);
						*change_head = NULL;
						return RETURN_ERROR;
					}
				}	
			}
		}
	// END Get all the set = 'value'

	if (strcmp_Upper(word, "WHERE") == 0)
	{
		index -= strLength(word);

		char* where_clause = substring(input, index, strLength(input)-1, NULL, malloced_head, the_debug);
		if (where_clause == NULL)
		{
			if (the_debug == YES_DEBUG)
				printf("	ERROR in parseUpdate() at line %d in %s\n", __LINE__, __FILE__);
			*change_head = NULL;
			return RETURN_ERROR;
		}

		if (parseWhereClause(where_clause, &(*change_head)->where_head, NULL, (*change_head)->table, "where", malloced_head, the_debug) != 0)
		{
			if (the_debug == YES_DEBUG)
				printf("	ERROR in parseUpdate() at line %d in %s\n", __LINE__, __FILE__);
			*change_head = NULL;
			return RETURN_ERROR;
		}

		myFree((void**) &where_clause, NULL, malloced_head, the_debug);
	}

	myFree((void**) &word, NULL, malloced_head, the_debug);

	return RETURN_GOOD;
}

/*	RETURNS:
 *  RETURN_ERROR if error
 *	RETURN_GOOD if good
 *
 *	WRITES TO:
 *  change_head
 */
int parseDelete(char* input, struct change_node_v2** change_head, struct malloced_node** malloced_head, int the_debug)
{
	// START Init change_head
		if (*change_head == NULL)
		{
			*change_head = (struct change_node_v2*) myMalloc(sizeof(struct change_node_v2), NULL, malloced_head, the_debug);
			if (*change_head == NULL)
			{
				if (the_debug == YES_DEBUG)
					printf("	ERROR in parseDelete() at line %d in %s\n", __LINE__, __FILE__);
				*change_head = NULL;
				return RETURN_ERROR;
			}
			(*change_head)->transac_id = -1;
			(*change_head)->table = NULL;
			(*change_head)->operation = -1;
			(*change_head)->total_rows_to_insert = 0;
			(*change_head)->col_list_head = NULL;
			(*change_head)->col_list_tail = NULL;
			(*change_head)->data_list_head = NULL;
			(*change_head)->data_list_tail = NULL;
			(*change_head)->where_head = NULL;
		}
	// END Init change_head

	// START Init word
		char* word = (char*) myMalloc(sizeof(char) * 200, NULL, malloced_head, the_debug);
		if (word == NULL)
		{
			if (the_debug == YES_DEBUG)
				printf("	ERROR in parseDelete() at line %d in %s\n", __LINE__, __FILE__);
			*change_head = NULL;
			return RETURN_ERROR;
		}
		word[0] = 0;
		int index = 0;
	// END Init word

	// START See if first word is DELETE
		getNextWord(input, word, &index);

		if (strcmp_Upper(word, "DELETE") != 0)
		{
			if (the_debug == YES_DEBUG)
				printf("	ERROR in parseDelete() at line %d in %s\n", __LINE__, __FILE__);
			printf("The first word of a delete command was not DELETE. Failing word: \"%s\"\n", word);
			errorTeardown(NULL, malloced_head, the_debug);
			*change_head = NULL;
			return RETURN_ERROR;
		}

		(*change_head)->operation = OP_DELETE;
	// END See if first word is DELETE

	// START See if next word is FROM
		getNextWord(input, word, &index);

		if (strcmp_Upper(word, "FROM") != 0)
		{
			if (the_debug == YES_DEBUG)
				printf("	ERROR in parseDelete() at line %d in %s\n", __LINE__, __FILE__);
			printf("The word \"FROM\" was expected after declaring a table. Failing word: \"%s\"\n", word);
			errorTeardown(NULL, malloced_head, the_debug);
			*change_head = NULL;
			return RETURN_ERROR;
		}
	// END See if next word is FROM

	// START Get table
		getNextWord(input, word, &index);

		(*change_head)->table = getTableFromName(word, malloced_head, the_debug);
		if ((*change_head)->table == NULL)
		{
			if (the_debug == YES_DEBUG)
				printf("	ERROR in parseDelete() at line %d in %s\n", __LINE__, __FILE__);
			printf("A table was declared which could not be found. Failing word: \"%s\"\n", word);
			errorTeardown(NULL, malloced_head, the_debug);
			*change_head = NULL;
			return RETURN_ERROR;
		}

		if (strcmp_Upper((*change_head)->table->name, "LIQUOR_LICENSES_FULL") == 0 || strcmp_Upper((*change_head)->table->name, "LIQUOR_SALES_FULL") == 0)
		{
			if (the_debug == YES_DEBUG)
				printf("	ERROR in parseUpdate() at line %d in %s\n", __LINE__, __FILE__);
			printf("You do not have permission to delete from this table. Failing word: \"%s\"\n", word);
			errorTeardown(NULL, malloced_head, the_debug);
			*change_head = NULL;
			return RETURN_ERROR;
		}
	// END Get table

	int got_next_word = getNextWord(input, word, &index);

	if (got_next_word == 0 && strcmp_Upper(word, "WHERE") == 0)
	{
		index -= strLength(word);

		char* where_clause = substring(input, index, strLength(input)-1, NULL, malloced_head, the_debug);
		if (where_clause == NULL)
		{
			if (the_debug == YES_DEBUG)
				printf("	ERROR in parseDelete() at line %d in %s\n", __LINE__, __FILE__);
			*change_head = NULL;
			return RETURN_ERROR;
		}

		if (parseWhereClause(where_clause, &(*change_head)->where_head, NULL, (*change_head)->table, "where", malloced_head, the_debug) != 0)
		{
			if (the_debug == YES_DEBUG)
				printf("	ERROR in parseDelete() at line %d in %s\n", __LINE__, __FILE__);
			*change_head = NULL;
			return RETURN_ERROR;
		}

		myFree((void**) &where_clause, NULL, malloced_head, the_debug);
	}
	else if (got_next_word == 0 && word[0] != ';')
	{
		if (the_debug == YES_DEBUG)
			printf("	ERROR in parseDelete() at line %d in %s\n", __LINE__, __FILE__);
		printf("An extra word after the where claues was declared. Please erase it. Failing word: \"%s\"\n", word);
		errorTeardown(NULL, malloced_head, the_debug);
		*change_head = NULL;
		return RETURN_ERROR;
	}

	myFree((void**) &word, NULL, malloced_head, the_debug);

	return RETURN_GOOD;
}

/*	RETURNS:
 *  RETURN_ERROR if error
 *	RETURN_GOOD if good
 *
 *	WRITES TO:
 *  change_head
 */
int parseInsert(char* input, struct change_node_v2** change_head, struct malloced_node** malloced_head, int the_debug)
{
	// START Init change_head
		if (*change_head == NULL)
		{
			*change_head = (struct change_node_v2*) myMalloc(sizeof(struct change_node_v2), NULL, malloced_head, the_debug);
			if (*change_head == NULL)
			{
				if (the_debug == YES_DEBUG)
					printf("	ERROR in parseInsert() at line %d in %s\n", __LINE__, __FILE__);
				*change_head = NULL;
				return RETURN_ERROR;
			}
			(*change_head)->transac_id = -1;
			(*change_head)->table = NULL;
			(*change_head)->operation = -1;
			(*change_head)->total_rows_to_insert = 0;
			(*change_head)->col_list_head = NULL;
			(*change_head)->col_list_tail = NULL;
			(*change_head)->data_list_head = NULL;
			(*change_head)->data_list_tail = NULL;
			(*change_head)->where_head = NULL;
		}
	// END Init change_head

	// START Init word
		char* word = (char*) myMalloc(sizeof(char) * 200, NULL, malloced_head, the_debug);
		if (word == NULL)
		{
			if (the_debug == YES_DEBUG)
				printf("	ERROR in parseInsert() at line %d in %s\n", __LINE__, __FILE__);
			*change_head = NULL;
			return RETURN_ERROR;
		}
		word[0] = 0;
		int index = 0;
	// END Init word

	// START See if first word is INSERT
		getNextWord(input, word, &index);

		if (strcmp_Upper(word, "INSERT") != 0)
		{
			if (the_debug == YES_DEBUG)
				printf("	ERROR in parseInsert() at line %d in %s\n", __LINE__, __FILE__);
			printf("The first word of a insert command was not INSERT. Failing word: \"%s\"\n", word);
			errorTeardown(NULL, malloced_head, the_debug);
			*change_head = NULL;
			return RETURN_ERROR;
		}

		(*change_head)->operation = OP_INSERT;
	// END See if first word is INSERT

	// START See if next word is INTO
		getNextWord(input, word, &index);

		if (strcmp_Upper(word, "INTO") != 0)
		{
			if (the_debug == YES_DEBUG)
				printf("	ERROR in parseInsert() at line %d in %s\n", __LINE__, __FILE__);
			printf("The word \"INTO\" was expected after declaring an \"INSERT\". Failing word: \"%s\"\n", word);
			errorTeardown(NULL, malloced_head, the_debug);
			*change_head = NULL;
			return RETURN_ERROR;
		}
	// END See if next word is INTO

	// START Get table
		getNextWord(input, word, &index);

		(*change_head)->table = getTableFromName(word, malloced_head, the_debug);
		if ((*change_head)->table == NULL)
		{
			if (the_debug == YES_DEBUG)
				printf("	ERROR in parseUpdate() at line %d in %s\n", __LINE__, __FILE__);
			printf("A table was declared which could not be found. Failing word: \"%s\"\n", word);
			errorTeardown(NULL, malloced_head, the_debug);
			*change_head = NULL;
			return RETURN_ERROR;
		}

		if (strcmp_Upper((*change_head)->table->name, "LIQUOR_LICENSES_FULL") == 0 || strcmp_Upper((*change_head)->table->name, "LIQUOR_SALES_FULL") == 0)
		{
			if (the_debug == YES_DEBUG)
				printf("	ERROR in parseUpdate() at line %d in %s\n", __LINE__, __FILE__);
			printf("You do not have permission to insert into this table. Failing word: \"%s\"\n", word);
			errorTeardown(NULL, malloced_head, the_debug);
			*change_head = NULL;
			return RETURN_ERROR;
		}
	// END Get table

	// START See if next word is (
		getNextWord(input, word, &index);

		if (word[0] != '(')
		{
			if (the_debug == YES_DEBUG)
				printf("	ERROR in parseInsert() at line %d in %s\n", __LINE__, __FILE__);
			printf("The word \"(\" was expected after declaring a \"INTO\". Failing word: \"%s\"\n", word);
			errorTeardown(NULL, malloced_head, the_debug);
			*change_head = NULL;
			return RETURN_ERROR;
		}
	// END See if next word is (

	// START Get columns for table
		struct ListNodePtr* temp_col_list_head = NULL;
		struct ListNodePtr* temp_col_list_tail = NULL;

		if (addListNodePtr(&temp_col_list_head, &temp_col_list_tail, NULL, -1, ADDLISTNODE_TAIL
						  ,NULL, malloced_head, the_debug) != RETURN_GOOD)
		{
			if (the_debug == YES_DEBUG)
				printf("	ERROR in parseInsert() at line %d in %s\n", __LINE__, __FILE__);
			*change_head = NULL;
			return RETURN_ERROR;
		}

		while (getNextWord(input, word, &index) == 0 && word[0] != ')')
		{
			if (word[0] == ',')
			{
				if (addListNodePtr(&temp_col_list_head, &temp_col_list_tail, NULL, -1, ADDLISTNODE_TAIL
			  					  ,NULL, malloced_head, the_debug) != RETURN_GOOD)
				{
					if (the_debug == YES_DEBUG)
						printf("	ERROR in parseInsert() at line %d in %s\n", __LINE__, __FILE__);
					*change_head = NULL;
					return RETURN_ERROR;
				}
			}
			else
			{
				if (the_debug == YES_DEBUG)
					printf("Finding _%s_ in table_cols_head\n", word);
				struct table_cols_info* cur_col = (*change_head)->table->table_cols_head;
				while (cur_col != NULL)
				{
					if (strcmp_Upper(word, cur_col->col_name) == 0)
					{
						struct ListNodePtr* temp_cur = temp_col_list_head;
						while (temp_cur != NULL)
						{
							if (temp_cur->ptr_value == cur_col)
							{
								if (the_debug == YES_DEBUG)
									printf("	ERROR in parseInsert() at line %d in %s\n", __LINE__, __FILE__);
								printf("A column was declared twice in the list of columns to insert into. Failing word: \"%s\"\n", word);
								errorTeardown(NULL, malloced_head, the_debug);
								*change_head = NULL;
								return RETURN_ERROR;
							}

							temp_cur = temp_cur->next;
						}

						if (temp_col_list_tail->ptr_value != NULL)
						{
							if (the_debug == YES_DEBUG)
								printf("	ERROR in parseInsert() at line %d in %s\n", __LINE__, __FILE__);
							printf("A comma was not declared in between columns. Failing word: \"%s\"\n", word);
							errorTeardown(NULL, malloced_head, the_debug);
							*change_head = NULL;
							return RETURN_ERROR;
						}

						if (the_debug == YES_DEBUG)
							printf("Found it\n");

						temp_col_list_tail->ptr_value = cur_col;
						temp_col_list_tail->ptr_type = PTR_TYPE_TABLE_COLS_INFO;
						break;
					}

					cur_col = cur_col->next;
				}

				if (cur_col == NULL)
				{
					if (the_debug == YES_DEBUG)
						printf("	ERROR in parseInsert() at line %d in %s\n", __LINE__, __FILE__);
					printf("A column was declared to insert into which was not found. Failing word: \"%s\"\n", word);
					errorTeardown(NULL, malloced_head, the_debug);
					*change_head = NULL;
					return RETURN_ERROR;
				}
			}
		}
	// END Get columns for table

	// START See if next word is VALUES
		getNextWord(input, word, &index);

		if (strcmp_Upper(word, "VALUES") != 0)
		{
			if (the_debug == YES_DEBUG)
				printf("	ERROR in parseInsert() at line %d in %s\n", __LINE__, __FILE__);
			printf("The word \"VALUES\" was expected after declaring a set of columns. Failing word: \"%s\"\n", word);
			errorTeardown(NULL, malloced_head, the_debug);
			*change_head = NULL;
			return RETURN_ERROR;
		}
	// END See if next word is VALUES

	// START Get values for each col
		struct ListNodePtr* cur_col = temp_col_list_head;

		addListNodePtr(&(*change_head)->col_list_head, &(*change_head)->col_list_tail, NULL, -1, ADDLISTNODE_TAIL
					  ,NULL, malloced_head, the_debug);
		addListNodePtr(&(*change_head)->data_list_head, &(*change_head)->data_list_tail, NULL, -1, ADDLISTNODE_TAIL
					  ,NULL, malloced_head, the_debug);

		while (getNextWord(input, word, &index) == 0 && word[0] != ';')
		{
			if (the_debug == YES_DEBUG)
				printf("word = _%s_\n", word);

			if (word[0] == '(')
			{
				// Do nothing
				(*change_head)->total_rows_to_insert++;
			}
			else if (word[0] == ')')
			{
				getNextWord(input, word, &index);

				if (word[0] == ',')
				{
					cur_col = temp_col_list_head;

					addListNodePtr(&(*change_head)->col_list_head, &(*change_head)->col_list_tail, NULL, -1, ADDLISTNODE_TAIL
								  ,NULL, malloced_head, the_debug);
					addListNodePtr(&(*change_head)->data_list_head, &(*change_head)->data_list_tail, NULL, -1, ADDLISTNODE_TAIL
								  ,NULL, malloced_head, the_debug);

					addListNodePtr(&(*change_head)->col_list_head, &(*change_head)->col_list_tail, NULL, -1, ADDLISTNODE_TAIL
								  ,NULL, malloced_head, the_debug);
					addListNodePtr(&(*change_head)->data_list_head, &(*change_head)->data_list_tail, NULL, -1, ADDLISTNODE_TAIL
								  ,NULL, malloced_head, the_debug);
				}
				else if (word[0] == ';' || word[0] == 0)
				{
					break;
				}
				else
				{
					if (the_debug == YES_DEBUG)
						printf("	ERROR in parseInsert() at line %d in %s\n", __LINE__, __FILE__);
					printf("An extra closing parentheses was declared in the set of values to insert. Failing word: \"%s\"\n", word);
					errorTeardown(NULL, malloced_head, the_debug);
					*change_head = NULL;
					return RETURN_ERROR;
				}
			}
			else if (word[0] == ',')
			{
				cur_col = cur_col->next;

				addListNodePtr(&(*change_head)->col_list_head, &(*change_head)->col_list_tail, NULL, -1, ADDLISTNODE_TAIL
							  ,NULL, malloced_head, the_debug);
				addListNodePtr(&(*change_head)->data_list_head, &(*change_head)->data_list_tail, NULL, -1, ADDLISTNODE_TAIL
							  ,NULL, malloced_head, the_debug);
			}
			else
			{
				if (cur_col == NULL)
				{
					if (the_debug == YES_DEBUG)
						printf("	ERROR in parseInsert() at line %d in %s\n", __LINE__, __FILE__);
					printf("Failing word: \"%s\"\n", word);
					errorTeardown(NULL, malloced_head, the_debug);
					*change_head = NULL;
					return RETURN_ERROR;
				}


				if ((*change_head)->col_list_tail->ptr_value != NULL || (*change_head)->data_list_tail->ptr_value != NULL)
				{
					if (the_debug == YES_DEBUG)
						printf("	ERROR in parseInsert() at line %d in %s\n", __LINE__, __FILE__);
					printf("Failing word: \"%s\"\n", word);
					errorTeardown(NULL, malloced_head, the_debug);
					*change_head = NULL;
					return RETURN_ERROR;
				}


				(*change_head)->col_list_tail->ptr_value = cur_col->ptr_value;
				(*change_head)->col_list_tail->ptr_type = PTR_TYPE_TABLE_COLS_INFO;

				if (determineDatatypeOfDataWord(word, &(*change_head)->data_list_tail->ptr_value, &(*change_head)->data_list_tail->ptr_type, NULL, malloced_head, the_debug) != RETURN_GOOD)
				{
					if (the_debug == YES_DEBUG)
						printf("	ERROR in parseInsert() at line %d in %s\n", __LINE__, __FILE__);
					return RETURN_ERROR;
				}


				if (((struct table_cols_info*) cur_col->ptr_value)->data_type != (*change_head)->data_list_tail->ptr_type)
				{
					if (the_debug == YES_DEBUG)
						printf("	ERROR in parseInsert() at line %d in %s\n", __LINE__, __FILE__);
					printf("A value which will be used to insert into a column does not match the column's datatype. Failing word: \"%s\"\n", word);
					errorTeardown(NULL, malloced_head, the_debug);
					*change_head = NULL;
					return RETURN_ERROR;
				}
			}
		}
	// END Get values for each col

	myFree((void**) &word, NULL, malloced_head, the_debug);

	freeAnyLinkedList((void**) &temp_col_list_head, PTR_TYPE_LIST_NODE_PTR, NULL, malloced_head, the_debug);

	return RETURN_GOOD;
}

/*	RETURNS:
 *  RETURN_ERROR if error
 *	RETURN_GOOD if good
 *
 *	WRITES TO:
 *  select_node or args_tail
 */
int getAllColsCuzStar(struct select_node** select_node, struct ListNodePtr** args_tail, char* alias, int i, int index
					 ,struct ListNodePtr* except_cols_name_head, struct ListNodePtr* except_cols_alias_head, bool except
					 ,struct malloced_node** malloced_head, int the_debug)
{
	if (args_tail != NULL)
	{
		// START Looking for all columns in from select_node (*select_node)->prev
			if (alias == NULL || ((*select_node)->prev->select_node_alias != NULL && strcmp((*select_node)->prev->select_node_alias, alias) == 0))
			{
				for (int j=0; j<(*select_node)->prev->columns_arr_size; j++)
				{
					if ((*args_tail)->ptr_value != NULL)
					{
						if (addListNodePtr(NULL, args_tail, NULL, -1, ADDLISTNODE_TAIL, NULL, malloced_head, the_debug) != RETURN_GOOD)
						{
							if (the_debug == YES_DEBUG)
								printf("	ERROR in getAllColsCuzStar() at line %d in %s\n", __LINE__, __FILE__);
							return RETURN_ERROR;
						}
					}

					(*args_tail)->ptr_value = (*select_node)->prev->columns_arr[j];
					(*args_tail)->ptr_type = PTR_TYPE_COL_IN_SELECT_NODE;
				}
			}
		// END Looking for all columns in from select_node (*select_node)->prev

		// START Looking for all columns in all joined nodes (*select_node)->join_head->select_joined
			struct join_node* cur_join = (*select_node)->join_head;

			while (cur_join != NULL)
			{
				if (alias == NULL || (cur_join->select_joined->select_node_alias != NULL && strcmp(cur_join->select_joined->select_node_alias, alias) == 0))
				{
					for (int j=0; j<cur_join->select_joined->columns_arr_size; j++)
					{
						if ((*args_tail)->ptr_value != NULL)
						{
							if (addListNodePtr(NULL, args_tail, NULL, -1, ADDLISTNODE_TAIL, NULL, malloced_head, the_debug) != RETURN_GOOD)
							{
								if (the_debug == YES_DEBUG)
									printf("	ERROR in getAllColsCuzStar() at line %d in %s\n", __LINE__, __FILE__);
								return RETURN_ERROR;
							}
						}

						(*args_tail)->ptr_value = cur_join->select_joined->columns_arr[j];
						(*args_tail)->ptr_type = PTR_TYPE_COL_IN_SELECT_NODE;
					}
				}

				cur_join = cur_join->next;
			}
		// END Looking for all columns in all joined nodes (*select_node)->join_head->select_joined
	}
	else //if (select_node != NULL)
	{
		// START Looking for all columns in from select_node (*select_node)->prev
			if (alias == NULL || ((*select_node)->prev->select_node_alias != NULL && strcmp((*select_node)->prev->select_node_alias, alias) == 0))
			{
				for (int j=0; j<(*select_node)->prev->columns_arr_size; j++)
				{
					struct ListNodePtr* cur_except_name = except_cols_name_head;
					struct ListNodePtr* cur_except_alias = except_cols_alias_head;

					bool this_excepted = false;
					while (cur_except_name != NULL && except)
					{
						char* col_name = NULL;
						if ((*select_node)->prev->columns_arr[j]->new_name)
							col_name = (*select_node)->prev->columns_arr[j]->new_name;
						else
						{
							struct col_in_select_node* cur = (*select_node)->prev->columns_arr[j];
							while (cur != NULL && (cur->col_ptr_type == PTR_TYPE_COL_IN_SELECT_NODE || cur->func_node != NULL || cur->math_node != NULL || cur->case_node != NULL))
							{
								//printf("Finding name of col: _%s_, _%s_\n", cur->col_ptr != NULL ? "valid col_ptr" : "null col_ptr", cur->new_name);
								if (cur->new_name != NULL && cur->new_name[0] != 0)
								{
									col_name = cur->new_name;
									break;
								}
								cur = cur->col_ptr;
							}

							if (cur->col_ptr_type == PTR_TYPE_TABLE_COLS_INFO)
								col_name = ((struct table_cols_info*) cur->col_ptr)->col_name;
						}
						//printf("cur_except_alias = _%s_\n", cur_except_alias->ptr_value);
						//printf("cur_except_name = _%s_\n", cur_except_name->ptr_value);
						//printf("prev alias = _%s_\n", (*select_node)->prev->select_node_alias);
						//printf("col_name = _%s_\n", col_name);

						if (((char*) cur_except_name->ptr_value)[0] != '*' && strcmp_Upper(cur_except_name->ptr_value, col_name) == 0)
						{
							if ((cur_except_alias->ptr_value == NULL && (*select_node)->join_head == NULL) 
								|| (cur_except_alias->ptr_value != NULL && (*select_node)->prev->select_node_alias != NULL && strcmp_Upper(cur_except_alias->ptr_value, (*select_node)->prev->select_node_alias) == 0)
								|| (cur_except_alias->ptr_value != NULL && strcmp_Upper(cur_except_alias->ptr_value, ((struct table_info*) (*select_node)->prev->columns_arr[0]->table_ptr)->name) == 0))
							{
								if (the_debug == YES_DEBUG)
									printf("Found the excepted column\n");
								this_excepted = true;
								break;
							}
						}

						cur_except_name = cur_except_name->next;
						cur_except_alias = cur_except_alias->next;
					}

					if (!this_excepted)
					{
						if (i == 0)
						{
							(*select_node)->columns_arr_size++;
						}
						else //if (i == 1)
						{
							(*select_node)->columns_arr[index]->table_ptr = (*select_node)->prev;
							(*select_node)->columns_arr[index]->table_ptr_type = PTR_TYPE_SELECT_NODE;

							(*select_node)->columns_arr[index]->col_ptr = (*select_node)->prev->columns_arr[j];
							(*select_node)->columns_arr[index]->col_ptr_type = PTR_TYPE_COL_IN_SELECT_NODE;

							(*select_node)->columns_arr[index]->rows_data_type = (*select_node)->prev->columns_arr[j]->rows_data_type;

							index++;
						}
					}
				}
			}
		// END Looking for all columns in from select_node (*select_node)->prev

		// START Looking for all columns in all joined nodes (*select_node)->join_head->select_joined
			struct join_node* cur_join = (*select_node)->join_head;

			while (cur_join != NULL)
			{
				if (alias == NULL || (cur_join->select_joined->select_node_alias != NULL && strcmp(cur_join->select_joined->select_node_alias, alias) == 0))
				{
					for (int j=0; j<cur_join->select_joined->columns_arr_size; j++)
					{
						struct ListNodePtr* cur_except_name = except_cols_name_head;
						struct ListNodePtr* cur_except_alias = except_cols_alias_head;

						bool this_excepted = false;
						while (cur_except_name != NULL && except)
						{
							char* col_name = NULL;
							if (cur_join->select_joined->columns_arr[j]->new_name)
								col_name = cur_join->select_joined->columns_arr[j]->new_name;
							else
							{
								struct col_in_select_node* cur = cur_join->select_joined->columns_arr[j];
								while (cur != NULL && (cur->col_ptr_type == PTR_TYPE_COL_IN_SELECT_NODE || cur->func_node != NULL || cur->math_node != NULL || cur->case_node != NULL))
								{
									//printf("Finding name of col: _%s_, _%s_\n", cur->col_ptr != NULL ? "valid col_ptr" : "null col_ptr", cur->new_name);
									if (cur->new_name != NULL && cur->new_name[0] != 0)
									{
										col_name = cur->new_name;
										break;
									}
									cur = cur->col_ptr;
								}

								if (cur->col_ptr_type == PTR_TYPE_TABLE_COLS_INFO)
									col_name = ((struct table_cols_info*) cur->col_ptr)->col_name;
							}
							//printf("cur_except_alias = _%s_\n", cur_except_alias->ptr_value);
							//printf("cur_except_name = _%s_\n", cur_except_name->ptr_value);
							//printf("prev alias = _%s_\n", (*select_node)->prev->select_node_alias);
							//printf("col_name = _%s_\n", col_name);

							if (((char*) cur_except_name->ptr_value)[0] != '*' && strcmp_Upper(cur_except_name->ptr_value, ((struct table_cols_info*) cur_join->select_joined->columns_arr[j]->col_ptr)->col_name) == 0)
							{
								if ((cur_except_alias->ptr_value == NULL && (*select_node)->join_head == NULL) 
									|| (cur_except_alias->ptr_value != NULL && cur_join->select_joined->select_node_alias != NULL && strcmp_Upper(cur_except_alias->ptr_value, cur_join->select_joined->select_node_alias) == 0)
									|| (cur_except_alias->ptr_value != NULL && strcmp_Upper(cur_except_alias->ptr_value, ((struct table_info*) cur_join->select_joined->columns_arr[0]->table_ptr)->name) == 0))
								{
									if (the_debug == YES_DEBUG)
										printf("Found the excepted column\n");
									this_excepted = true;
									break;
								}
							}

							cur_except_name = cur_except_name->next;
							cur_except_alias = cur_except_alias->next;
						}

						if (!this_excepted)
						{
							if (i == 0)
							{
								(*select_node)->columns_arr_size++;
							}
							else //if (i == 1)
							{
								(*select_node)->columns_arr[index]->table_ptr = cur_join->select_joined;
								(*select_node)->columns_arr[index]->table_ptr_type = PTR_TYPE_SELECT_NODE;

								(*select_node)->columns_arr[index]->col_ptr = cur_join->select_joined->columns_arr[j];
								(*select_node)->columns_arr[index]->col_ptr_type = PTR_TYPE_COL_IN_SELECT_NODE;

								(*select_node)->columns_arr[index]->rows_data_type = cur_join->select_joined->columns_arr[j]->rows_data_type;

								index++;
							}
						}
					}
				}

				cur_join = cur_join->next;
			}
		// END Looking for all columns in all joined nodes (*select_node)->join_head->select_joined
	}

	return RETURN_GOOD;
}

/*	RETURNS:
 *  RETURN_ERROR if error
 *	RETURN_GOOD if good
 *
 *	WRITES TO:
 *  join_tail
 */
int parseOnClauseOfSelect(struct join_node** join_tail, char** on_clause, struct select_node** select_head
						 ,struct malloced_node** malloced_head, int the_debug)
{
	if (the_debug == YES_DEBUG)
		printf("PARSING on_clause: _%s_\n", *on_clause);

	if (strContainsWordUpper(*on_clause, "COUNT") || strContainsWordUpper(*on_clause, "AVG") || 
		strContainsWordUpper(*on_clause, "FIRST") || strContainsWordUpper(*on_clause, "LAST") || 
		strContainsWordUpper(*on_clause, "MIN") || strContainsWordUpper(*on_clause, "MAX") || 
		strContainsWordUpper(*on_clause, "MEDIAN") || strContainsWordUpper(*on_clause, "SUM") || 
		strContainsWordUpper(*on_clause, "RANK"))
	{
		if (the_debug == YES_DEBUG)
			printf("	ERROR in parseOnClauseOfSelect() at line %d in %s\n", __LINE__, __FILE__);
		printf("A function was declared inside this on clause for a join which will not work. Please erase it. Failing word: \"%s\"\n", *on_clause);
		errorTeardown(NULL, malloced_head, the_debug);
		return RETURN_ERROR;
	}

	struct where_clause_node* the_on_clause = NULL;
	int error_code = parseWhereClause(*on_clause, &the_on_clause, *select_head, NULL, "on", malloced_head, the_debug);

	if (error_code != RETURN_GOOD)
	{
		if (the_debug == YES_DEBUG)
			printf("	ERROR in parseOnClauseOfSelect() at line %d in %s\n", __LINE__, __FILE__);
		return RETURN_ERROR;
	}

	if (the_debug == YES_DEBUG)
		printf("Back from parsing on_clause\n");

	(*join_tail)->on_clause_head = the_on_clause;

	myFree((void**) on_clause, NULL, malloced_head, the_debug);

	return RETURN_GOOD;
}

/*	RETURNS:
 *  RETURN_GOOD
 *
 *	WRITES TO:
 *  sub_select
 */
int getSubSelect(char* input, char** word, int* index, char** sub_select)
{
	int parens = 0;

	while (getNextWord(input, *word, index) == 0 && (*word)[0] != ';' && (parens > 0 || (*word)[0] != ')'))
	{
		//printf("word in sub_select loop = _%s_\n", *word);
		//printf("hmm = _%d_\n", parens);


		if ((*word)[0] == '(')
			parens++;
		else if ((*word)[0] == ')')
			parens--;

		strcat(*sub_select, *word);
		if (input[*index] == '.')
		{
			strcat(*sub_select, ".");
			(*index)++;
		}
		else
			strcat(*sub_select, " ");
	}
	

	return RETURN_GOOD;
}

/*	RETURNS:
 *  RETURN_ERROR if error
 *	RETURN_GOOD if good
 *
 *	WRITES TO:
 *  the_math_node
 *  new_col_name
 */
int parseMathNode(struct math_node** the_math_node, char** new_col_name, char** col_name, char* input, char* word, int index, struct select_node* select_node
				 ,struct malloced_node** malloced_head, int the_debug)
{
	if (the_debug == YES_DEBUG)
	{
		printf("word at start = _%s_\n", word);
		printf("input = _%s_, w/ index = %d\n", input, index);
	}

	if (*the_math_node == NULL)
	{
		*the_math_node = myMalloc(sizeof(struct math_node), NULL, malloced_head, the_debug);
		if (*the_math_node == NULL)
		{
			if (the_debug == YES_DEBUG)
				printf("	ERROR in parseMathNode() at line %d in %s\n", __LINE__, __FILE__);
			return RETURN_ERROR;
		}
		(*the_math_node)->ptr_one = NULL;
	}

	int open_parens = 0;

	struct math_node* cur_math = NULL;

	if (word[0] == '(' && *col_name == NULL)
	{
		// START Open parens so math a new math_node at ptr_one and traverse to it
			if (the_debug == YES_DEBUG)
				printf("Making new math_node at ptr_one\n");
			open_parens++;

			(*the_math_node)->ptr_one = myMalloc(sizeof(struct math_node), NULL, malloced_head, the_debug);
			if ((*the_math_node)->ptr_one == NULL)
			{
				if (the_debug == YES_DEBUG)
					printf("	ERROR in parseMathNode() at line %d in %s\n", __LINE__, __FILE__);
				return RETURN_ERROR;
			}
			(*the_math_node)->ptr_one_type = PTR_TYPE_MATH_NODE;

			(*the_math_node)->ptr_two = NULL;
			(*the_math_node)->ptr_two_type = -1;

			(*the_math_node)->operation = -1;
			(*the_math_node)->parent = NULL;
			(*the_math_node)->result_type = -1;

			((struct math_node*) (*the_math_node)->ptr_one)->parent = *the_math_node;

			cur_math = (*the_math_node)->ptr_one;

			cur_math->ptr_one = NULL;
			cur_math->ptr_one_type = -1;

			cur_math->ptr_two = NULL;
			cur_math->ptr_two_type = -1;

			cur_math->operation = -1;
			cur_math->result_type = -1;
		// END Open parens so math a new math_node at ptr_one and traverse to it
	}
	else if (input[0] == '-' && (*col_name)[0] == '-')
	{
		if (the_debug == YES_DEBUG)
			printf("Found negative constant in parse math node\n");
		(*the_math_node)->operation = MATH_SUB;

		int hmm_int = atoi(word);

		if (hmm_int == 0 && word[0] != '0')
		{
			if (the_debug == YES_DEBUG)
				printf("	ERROR in parseMathNode() at line %d in %s\n", __LINE__, __FILE__);
			printf("A negative sign was applied to a word which was not an number. Please remove the negative sign or make the operand a number. Failing word: \"%s\"\n", word);
			errorTeardown(NULL, malloced_head, the_debug);
			return RETURN_ERROR;
		}

		if (hmm_int != 0 && (strcontains(word, '.') || input[index] == '.'))
		{
			(*the_math_node)->ptr_one = myMalloc(sizeof(double), NULL, malloced_head, the_debug);
			if ((*the_math_node)->ptr_one == NULL)
			{
				if (the_debug == YES_DEBUG)
					printf("	ERROR in parseMathNode() at line %d in %s\n", __LINE__, __FILE__);
				return RETURN_ERROR;
			}
			*((double*) (*the_math_node)->ptr_one) = 0.0;
			(*the_math_node)->ptr_one_type = PTR_TYPE_REAL;

			double* temp = myMalloc(sizeof(double), NULL, malloced_head, the_debug);
			if (temp == NULL)
			{
				if (the_debug == YES_DEBUG)
					printf("	ERROR in parseMathNode() at line %d in %s\n", __LINE__, __FILE__);
				return RETURN_ERROR;
			}
			double hmm_double = 0.0;
			sscanf(word, "%lf", &hmm_double);
			if (the_debug == YES_DEBUG)
				printf("hmm_double = %lf\n", hmm_double);

			//hmm_double = hmm_double / pow(10, strLength(word));
			//printf("hmm_double = %lf\n", hmm_double);

			index++;
			getNextWord(input, word, &index);

			double hmm_double_2 = 0.0;
			sscanf(word, "%lf", &hmm_double_2);
			if (the_debug == YES_DEBUG)
				printf("hmm_double_2 = %lf\n", hmm_double_2);

			hmm_double += hmm_double_2;
			if (the_debug == YES_DEBUG)
				printf("hmm_double = %lf\n", hmm_double);

			*((double*) temp) = hmm_double;
			if (the_debug == YES_DEBUG)
				printf("temp = %lf\n", *((double*) temp));

			(*the_math_node)->ptr_two = temp;
			(*the_math_node)->ptr_two_type = PTR_TYPE_REAL;

			(*the_math_node)->result_type = PTR_TYPE_REAL;
		}
		else
		{
			(*the_math_node)->ptr_one = myMalloc(sizeof(int), NULL, malloced_head, the_debug);
			if ((*the_math_node)->ptr_one == NULL)
			{
				if (the_debug == YES_DEBUG)
					printf("	ERROR in parseMathNode() at line %d in %s\n", __LINE__, __FILE__);
				return RETURN_ERROR;
			}
			*((int*) (*the_math_node)->ptr_one) = 0;
			(*the_math_node)->ptr_one_type = PTR_TYPE_INT;

			int* temp = myMalloc(sizeof(int), NULL, malloced_head, the_debug);
			if (temp == NULL)
			{
				if (the_debug == YES_DEBUG)
					printf("	ERROR in parseMathNode() at line %d in %s\n", __LINE__, __FILE__);
				return RETURN_ERROR;
			}
			*((int*) temp) = hmm_int;

			(*the_math_node)->ptr_two = temp;
			(*the_math_node)->ptr_two_type = PTR_TYPE_INT;

			(*the_math_node)->result_type = PTR_TYPE_INT;
		}

		(*the_math_node)->parent = NULL;

		cur_math = *the_math_node;
	}
	else if ((word[0] == '+' || word[0] == '-' || word[0] == '*' || word[0] == '/' || word[0] == '^') && (*col_name != NULL || (*the_math_node)->ptr_one != NULL))
	{
		if (word[0] == '+')
			(*the_math_node)->operation = MATH_ADD;
		else if (word[0] == '-')
			(*the_math_node)->operation = MATH_SUB;
		else if (word[0] == '*')
			(*the_math_node)->operation = MATH_MULT;
		else if (word[0] == '/')
			(*the_math_node)->operation = MATH_DIV;
		else //if (word[0] == '^')
			(*the_math_node)->operation = MATH_POW;

		if ((*the_math_node)->ptr_one == NULL)
		{
			// START Determine type of previous word
				if (the_debug == YES_DEBUG)
					printf("Assigning to ptr_one\n");

				int hmm_int = atoi(*col_name);

				if (hmm_int != 0 && strcontains(*col_name, '.'))
				{
					// Double
					(*the_math_node)->ptr_one = myMalloc(sizeof(double), NULL, malloced_head, the_debug);
					if ((*the_math_node)->ptr_one == NULL)
					{
						if (the_debug == YES_DEBUG)
							printf("	ERROR in parseMathNode() at line %d in %s\n", __LINE__, __FILE__);
						return RETURN_ERROR;
					}

					double hmm_double = 0.0;

					sscanf(*col_name, "%lf", &hmm_double);

					*((double*) (*the_math_node)->ptr_one) = hmm_double;

					(*the_math_node)->ptr_one_type = PTR_TYPE_REAL;
				}
				else if (hmm_int != 0 || (*col_name)[0] == '0')
				{
					// Int
					(*the_math_node)->ptr_one = myMalloc(sizeof(int), NULL, malloced_head, the_debug);
					if ((*the_math_node)->ptr_one == NULL)
					{
						if (the_debug == YES_DEBUG)
							printf("	ERROR in parseMathNode() at line %d in %s\n", __LINE__, __FILE__);
						return RETURN_ERROR;
					}

					*((int*) (*the_math_node)->ptr_one) = hmm_int;

					(*the_math_node)->ptr_one_type = PTR_TYPE_INT;
				}
				else if (strcontains(*col_name, '/'))
				{
					// Date
					(*the_math_node)->ptr_one = *col_name;

					(*the_math_node)->ptr_one_type = PTR_TYPE_DATE;
				}
				else
				{
					// Char but column name
					//(*the_math_node)->ptr_one = *col_name;

					//(*the_math_node)->ptr_one_type = PTR_TYPE_CHAR;

					(*the_math_node)->ptr_one = NULL;

					char* alias = NULL;
					char* name = upper(*col_name, NULL, malloced_head, the_debug);
					if (name == NULL)
					{
						if (the_debug == YES_DEBUG)
							printf("	ERROR in parseMathNode() at line %d in %s\n", __LINE__, __FILE__);
						return RETURN_ERROR;
					}
					if (the_debug == YES_DEBUG)
						printf("name = _%s_\n", name);

					int index_of_dot = indexOf(name, '.');
					if (index_of_dot > -1)
					{
						alias = substring(name, 0, index_of_dot-1, NULL, malloced_head, the_debug);
						
						char* temp = name;

						name = substring(temp, index_of_dot+1, strLength(temp)-1, NULL, malloced_head, the_debug);

						if (alias == NULL || name == NULL)
						{
							if (the_debug == YES_DEBUG)
								printf("	ERROR in parseMathNode() at line %d in %s\n", __LINE__, __FILE__);
							return RETURN_ERROR;
						}

						myFree((void**) &temp, NULL, malloced_head, the_debug);
					}
					if (the_debug == YES_DEBUG)
						printf("alias.name = _%s.%s_\n", alias, name);

					if (getColInSelectNodeFromName((void*) *the_math_node, PTR_TYPE_MATH_NODE, 1, 1, select_node, alias, name
												  ,malloced_head, the_debug) != RETURN_GOOD)
					{
						printf("A column was declared which could not be found. Failing word: ""%s%s%s""\n", alias == NULL ? "" : alias, alias == NULL ? "" : ".", name);
						errorTeardown(NULL, malloced_head, the_debug);
						return RETURN_ERROR;
					}

					if (alias != NULL)
						myFree((void**) &alias, NULL, malloced_head, the_debug);
					if (name != NULL)
						myFree((void**) &name, NULL, malloced_head, the_debug);

					// START Check if found col has valid datatype
						if (((struct col_in_select_node*) (*the_math_node)->ptr_one)->rows_data_type == DATA_STRING)
						{
							if (the_debug == YES_DEBUG)
								printf("	ERROR in parseMathNode() at line %d in %s\n", __LINE__, __FILE__);
							errorTeardown(NULL, malloced_head, the_debug);
							return RETURN_ERROR;
						}
					// END Check if found col has valid datatype
				}
			// END Determine type of previous word

			(*the_math_node)->ptr_two = NULL;
			(*the_math_node)->ptr_two_type = -1;

			(*the_math_node)->parent = NULL;
			(*the_math_node)->result_type = -1;

			cur_math = *the_math_node;

			if ((*the_math_node)->ptr_one_type != PTR_TYPE_CHAR)
				myFree((void**) col_name, NULL, malloced_head, the_debug);

			*col_name = NULL;
		}
		else
		{
			cur_math = *the_math_node;
		}
	}


	bool made_new_without_parens = false;

	struct func_node* a_func_node = NULL;
	struct case_node* case_node = NULL;

	while (getNextWord(input, word, &index) == 0 && word[0] != ',' && strcmp_Upper(word, "FROM") != 0
		   && word[0] != '=' && word[0] != '>' && word[0] != '<')
	{
		if (input[index] == '.')
		{
			int hmm_index = 0;
			while (word[hmm_index] != 0)
				hmm_index++;

			word[hmm_index] = '.';
			word[hmm_index+1] = 0;

			char* temp_word = (char*) myMalloc(sizeof(char) * 64, NULL, malloced_head, the_debug);
			if (temp_word == NULL)
			{
				if (the_debug == YES_DEBUG)
					printf("	ERROR in parseMathNode() at line %d in %s\n", __LINE__, __FILE__);
				return RETURN_ERROR;
			}
			strcpy(temp_word, word);

			index++;
			getNextWord(input, word, &index);

			strcat(temp_word, word);

			strcpy(word, temp_word);

			myFree((void**) &temp_word, NULL, malloced_head, the_debug);
		}

		if (the_debug == YES_DEBUG)
			printf("    word in get math loop = _%s_\n", word);

		if (word[0] == '(')
		{
			// START Open parens so make a new math_node at ptr_one and traverse to it
				if (cur_math->ptr_one != NULL)
				{
					if (the_debug == YES_DEBUG)
						printf("	Making new math_node at ptr_two\n");

					open_parens++;

					cur_math->ptr_two = myMalloc(sizeof(struct math_node), NULL, malloced_head, the_debug);
					if (cur_math->ptr_two == NULL)
					{
						if (the_debug == YES_DEBUG)
							printf("	ERROR in parseMathNode() at line %d in %s\n", __LINE__, __FILE__);
						return RETURN_ERROR;
					}
					cur_math->ptr_two_type = PTR_TYPE_MATH_NODE;

					((struct math_node*) cur_math->ptr_two)->parent = cur_math;

					cur_math = cur_math->ptr_two;
				}
				else
				{
					if (the_debug == YES_DEBUG)
						printf("	Making new math_node at ptr_one\n");

					open_parens++;

					cur_math->ptr_one = myMalloc(sizeof(struct math_node), NULL, malloced_head, the_debug);
					if (cur_math->ptr_one == NULL)
					{
						if (the_debug == YES_DEBUG)
							printf("	ERROR in parseMathNode() at line %d in %s\n", __LINE__, __FILE__);
						return RETURN_ERROR;
					}
					cur_math->ptr_one_type = PTR_TYPE_MATH_NODE;

					((struct math_node*) cur_math->ptr_one)->parent = cur_math;

					cur_math = cur_math->ptr_one;
				}
			// END Open parens so math a new math_node at ptr_one and traverse to it

			cur_math->ptr_one = NULL;
			cur_math->ptr_one_type = -1;

			cur_math->ptr_two = NULL;
			cur_math->ptr_two_type = -1;

			cur_math->operation = -1;
			cur_math->result_type = -1;
		}
		else if (word[0] == ')')
		{
			// START Close parens so traverse to parent
				if (cur_math->ptr_one == NULL || cur_math->ptr_two == NULL || cur_math->operation == -1)
				{
					if (the_debug == YES_DEBUG)
						printf("	ERROR in parseMathNode() at line %d in %s\n", __LINE__, __FILE__);
					printf("One or multiple operands in a math clause were not declared. Please declare two, on either side of the operation. Failing word: \"%s\"\n", word);
					errorTeardown(NULL, malloced_head, the_debug);
					return RETURN_ERROR;
				}

				open_parens--;

				cur_math = cur_math->parent;

				if (the_debug == YES_DEBUG)
					printf("	Going back to parent, op now = %d\n", cur_math->operation);
			// END Close parens so traverse to parent
		}
		else if (strcmp_Upper(word, "CASE") == 0)
		{
			char* case_input = (char*) myMalloc(sizeof(char) * 10000, NULL, malloced_head, the_debug);
			if (case_input == NULL)
			{
				if (the_debug == YES_DEBUG)
					printf("	ERROR in parseMathNode() at line %d in %s\n", __LINE__, __FILE__);
				return RETURN_ERROR;
			}

			strcpy(case_input, "CASE ");
			while (getNextWord(input, word, &index) == 0 && strcmp_Upper(word, "END") != 0)
			{
				strcat(case_input, word);
				strcat(case_input, " ");
			}

			strcat(case_input, "END");

			if (the_debug == YES_DEBUG)
				printf("case_input = _%s_\n", case_input);

			case_node = NULL;

			int ptr_type = PTR_TYPE_CASE_NODE;
			if (parseCaseNode(&case_node, case_input, word, 5, NULL, select_node, &ptr_type, malloced_head, the_debug) != RETURN_GOOD)
			{
				if (the_debug == YES_DEBUG)
					printf("	ERROR in parseMathNode() at line %d in %s\n", __LINE__, __FILE__);
				return RETURN_ERROR;
			}

			myFree((void**) &case_input, NULL, malloced_head, the_debug);

			if (the_debug == YES_DEBUG)
				printf("Back from parseCaseNode()\n");


			// START
				if (cur_math->ptr_two == NULL)
				{
					cur_math->ptr_two = case_node;
					cur_math->ptr_two_type = PTR_TYPE_CASE_NODE;

					case_node = NULL;

					index -= strLength(word);

					// START Determine result_type
						int ptr_one_datatype = -1;
						int ptr_two_datatype = -1;

						if (cur_math->ptr_one_type == PTR_TYPE_MATH_NODE)
							ptr_one_datatype = ((struct math_node*) cur_math->ptr_one)->result_type;
						else if (cur_math->ptr_one_type == PTR_TYPE_FUNC_NODE)
							ptr_one_datatype = ((struct func_node*) cur_math->ptr_one)->result_type;
						else if (cur_math->ptr_one_type == PTR_TYPE_CASE_NODE)
							ptr_one_datatype = ((struct case_node*) cur_math->ptr_one)->result_type;
						else 
							ptr_one_datatype = cur_math->ptr_one_type;

						if (cur_math->ptr_two_type == PTR_TYPE_MATH_NODE)
							ptr_two_datatype = ((struct math_node*) cur_math->ptr_two)->result_type;
						else if (cur_math->ptr_two_type == PTR_TYPE_FUNC_NODE)
							ptr_two_datatype = ((struct func_node*) cur_math->ptr_two)->result_type;
						else if (cur_math->ptr_two_type == PTR_TYPE_CASE_NODE)
							ptr_one_datatype = ((struct case_node*) cur_math->ptr_two)->result_type;
						else 
							ptr_two_datatype = cur_math->ptr_two_type;

						if (the_debug == YES_DEBUG)
							printf("Math ptr_one_datatype = %d, ptr_two_datatype = %d\n", ptr_one_datatype, ptr_two_datatype);


						if (ptr_one_datatype == PTR_TYPE_DATE || ptr_two_datatype == PTR_TYPE_DATE)
						{
							cur_math->result_type = PTR_TYPE_DATE;
						}
						else if (ptr_one_datatype == PTR_TYPE_REAL || ptr_two_datatype == PTR_TYPE_REAL)
						{
							cur_math->result_type = PTR_TYPE_REAL;
						}
						else
						{
							cur_math->result_type = PTR_TYPE_INT;
						}
						if (the_debug == YES_DEBUG)
							printf("		Assigned result_type: %d\n", cur_math->result_type);
					// END Determine result_type
				}
			// END
		}
		else if (word[0] == '+' || word[0] == '-' || word[0] == '*' || word[0] == '/' || word[0] == '^')
		{
			if (cur_math->operation != -1)
			{
				int temp_op = -1;

				if (word[0] == '+')
					temp_op = MATH_ADD;
				else if (word[0] == '-')
					temp_op = MATH_SUB;
				else if (word[0] == '*')
					temp_op = MATH_MULT;
				else if (word[0] == '/')
					temp_op = MATH_DIV;
				else //if (word[0] == '^')
					temp_op = MATH_POW;

				if (temp_op > cur_math->operation && cur_math->ptr_one != NULL && cur_math->ptr_two != NULL)
				{
					// START Bc PEMDAS, need to make this a new child math_node
						if (the_debug == YES_DEBUG)
							printf("Made new without parens, opt 1\n");

						void* temp_ptr_two = cur_math->ptr_two;
						int temp_ptr_two_type = cur_math->ptr_two_type;

						cur_math->ptr_two = myMalloc(sizeof(struct math_node), NULL, malloced_head, the_debug);
						if (cur_math->ptr_two == NULL)
						{
							if (the_debug == YES_DEBUG)
								printf("	ERROR in parseMathNode() at line %d in %s\n", __LINE__, __FILE__);
							return RETURN_ERROR;
						}

						cur_math->ptr_two_type = PTR_TYPE_MATH_NODE;

						((struct math_node*) cur_math->ptr_two)->parent = cur_math;

						((struct math_node*) cur_math->ptr_two)->ptr_one = temp_ptr_two;
						((struct math_node*) cur_math->ptr_two)->ptr_one_type = temp_ptr_two_type;

						((struct math_node*) cur_math->ptr_two)->ptr_two = NULL;
						((struct math_node*) cur_math->ptr_two)->ptr_two_type = -1;

						((struct math_node*) cur_math->ptr_two)->result_type = -1;

						if (word[0] == '+')
							((struct math_node*) cur_math->ptr_two)->operation = MATH_ADD;
						else if (word[0] == '-')
							((struct math_node*) cur_math->ptr_two)->operation = MATH_SUB;
						else if (word[0] == '*')
							((struct math_node*) cur_math->ptr_two)->operation = MATH_MULT;
						else if (word[0] == '/')
							((struct math_node*) cur_math->ptr_two)->operation = MATH_DIV;
						else //if (word[0] == '^')
							((struct math_node*) cur_math->ptr_two)->operation = MATH_POW;

						cur_math = cur_math->ptr_two;

						made_new_without_parens = true;
					// END Bc PEMDAS, need to make this a new child math_node
				}
				else if (cur_math->ptr_one != NULL && cur_math->ptr_two != NULL)
				{
					// START Is a lesser operation, make previous two ptrs a new child math_node
						if (the_debug == YES_DEBUG)
							printf("Made new without parens, opt 2\n");

						void* temp_ptr_one = cur_math->ptr_one;
						int temp_ptr_one_type = cur_math->ptr_one_type;

						void* temp_ptr_two = cur_math->ptr_two;
						int temp_ptr_two_type = cur_math->ptr_two_type;

						int temp_operation = cur_math->operation;
						int temp_result = cur_math->result_type;

						cur_math->ptr_one = myMalloc(sizeof(struct math_node), NULL, malloced_head, the_debug);
						if (cur_math->ptr_one == NULL)
						{
							if (the_debug == YES_DEBUG)
								printf("	ERROR in parseMathNode() at line %d in %s\n", __LINE__, __FILE__);
							return RETURN_ERROR;
						}

						cur_math->ptr_one_type = PTR_TYPE_MATH_NODE;

						cur_math->ptr_two = NULL;
						cur_math->ptr_two_type = -1;

						if (word[0] == '+')
							cur_math->operation = MATH_ADD;
						else if (word[0] == '-')
							cur_math->operation = MATH_SUB;
						else if (word[0] == '*')
							cur_math->operation = MATH_MULT;
						else if (word[0] == '/')
							cur_math->operation = MATH_DIV;
						else //if (word[0] == '^')
							cur_math->operation = MATH_POW;

						((struct math_node*) cur_math->ptr_one)->ptr_one = temp_ptr_one;
						((struct math_node*) cur_math->ptr_one)->ptr_one_type = temp_ptr_one_type;

						((struct math_node*) cur_math->ptr_one)->ptr_two = temp_ptr_two;
						((struct math_node*) cur_math->ptr_one)->ptr_two_type = temp_ptr_two_type;

						((struct math_node*) cur_math->ptr_one)->operation = temp_operation;
						((struct math_node*) cur_math->ptr_one)->parent = cur_math;
						((struct math_node*) cur_math->ptr_one)->result_type = temp_result;
					// END Is a lesser operation, make previous two ptrs a new child math_node
				}
				else
				{
					if (the_debug == YES_DEBUG)
						printf("	ERROR in parseMathNode() at line %d in %s\n", __LINE__, __FILE__);
					printf("Failing word: \"%s\"\n", word);
					errorTeardown(NULL, malloced_head, the_debug);
					return RETURN_ERROR;
				}
			}
			else
			{
				if (the_debug == YES_DEBUG)
					printf("	Assiging operation\n");
				if (word[0] == '+')
					cur_math->operation = MATH_ADD;
				else if (word[0] == '-')
					cur_math->operation = MATH_SUB;
				else if (word[0] == '*')
					cur_math->operation = MATH_MULT;
				else if (word[0] == '/')
					cur_math->operation = MATH_DIV;
				else //if (word[0] == '^')
					cur_math->operation = MATH_POW;
			}
		}
		else if (strcmp_Upper(word, "AVG") == 0 ||
				strcmp_Upper(word, "COUNT") == 0 ||
				strcmp_Upper(word, "FIRST") == 0 ||
				strcmp_Upper(word, "LAST") == 0 ||
				strcmp_Upper(word, "MIN") == 0 ||
				strcmp_Upper(word, "MAX") == 0 ||
				strcmp_Upper(word, "MEDIAN") == 0 ||
				strcmp_Upper(word, "SUM") == 0)
		{
			if (the_debug == YES_DEBUG)
				printf("	Found func node\n");

			char* col_name = upper(word, NULL, malloced_head, the_debug);
			char* temp_func_string = (char*) myMalloc(sizeof(char) * 1000, NULL, malloced_head, the_debug);
			if (col_name == NULL || temp_func_string == NULL)
			{
				if (the_debug == YES_DEBUG)
					printf("	ERROR in parseMathNode() at line %d in %s\n", __LINE__, __FILE__);
				return RETURN_ERROR;
			}

			strcpy(temp_func_string, word);
			strcat(temp_func_string, " ");

			while (getNextWord(input, word, &index) == 0 && word[0] != ';' && word[0] != ')')
			{
				strcat(temp_func_string, word);
				strcat(temp_func_string, " ");
			}

			if (word[0] != ')')
			{
				if (the_debug == YES_DEBUG)
					printf("	ERROR in parseMathNode() at line %d in %s\n", __LINE__, __FILE__);
				printf("A function name keyword was found in a math clause, but was not followed by an closing parentheses. Failing word: \"%s\"\n", col_name);
				errorTeardown(NULL, malloced_head, the_debug);
				return RETURN_ERROR;
			}

			strcat(temp_func_string, ")");

			if (the_debug == YES_DEBUG)
				printf("word before parseFuncNode() = _%s_\n", word);
			int temp_index = 0;
			if (parseFuncNode(&a_func_node, NULL, &col_name, temp_func_string, word, &temp_index, select_node, false
 							 ,malloced_head, the_debug) != RETURN_GOOD)
			{
				return RETURN_ERROR;
			}

			myFree((void**) &temp_func_string, NULL, malloced_head, the_debug);
			myFree((void**) &col_name, NULL, malloced_head, the_debug);

			// START
				if (cur_math->ptr_two == NULL)
				{
					cur_math->ptr_two = a_func_node;
					cur_math->ptr_two_type = PTR_TYPE_FUNC_NODE;

					a_func_node = NULL;

					index -= strLength(word);

					// START Determine result_type
						int ptr_one_datatype = -1;
						int ptr_two_datatype = -1;

						if (cur_math->ptr_one_type == PTR_TYPE_MATH_NODE)
							ptr_one_datatype = ((struct math_node*) cur_math->ptr_one)->result_type;
						else if (cur_math->ptr_one_type == PTR_TYPE_FUNC_NODE)
							ptr_one_datatype = ((struct func_node*) cur_math->ptr_one)->result_type;
						else if (cur_math->ptr_one_type == PTR_TYPE_CASE_NODE)
							ptr_one_datatype = ((struct case_node*) cur_math->ptr_one)->result_type;
						else 
							ptr_one_datatype = cur_math->ptr_one_type;

						if (cur_math->ptr_two_type == PTR_TYPE_MATH_NODE)
							ptr_two_datatype = ((struct math_node*) cur_math->ptr_two)->result_type;
						else if (cur_math->ptr_two_type == PTR_TYPE_FUNC_NODE)
							ptr_two_datatype = ((struct func_node*) cur_math->ptr_two)->result_type;
						else if (cur_math->ptr_two_type == PTR_TYPE_CASE_NODE)
							ptr_one_datatype = ((struct case_node*) cur_math->ptr_two)->result_type;
						else 
							ptr_two_datatype = cur_math->ptr_two_type;

						if (the_debug == YES_DEBUG)
							printf("Math ptr_one_datatype = %d, ptr_two_datatype = %d\n", ptr_one_datatype, ptr_two_datatype);


						if (ptr_one_datatype == PTR_TYPE_DATE || ptr_two_datatype == PTR_TYPE_DATE)
						{
							cur_math->result_type = PTR_TYPE_DATE;
						}
						else if (ptr_one_datatype == PTR_TYPE_REAL || ptr_two_datatype == PTR_TYPE_REAL)
						{
							cur_math->result_type = PTR_TYPE_REAL;
						}
						else
						{
							cur_math->result_type = PTR_TYPE_INT;
						}
						if (the_debug == YES_DEBUG)
							printf("		Assigned result_type: %d\n", cur_math->result_type);
					// END Determine result_type
				}
			// END
		}
		else if (cur_math->ptr_one != NULL && cur_math->ptr_two == NULL)
		{
			// START Determine type of next word and put into ptr_two
				if (the_debug == YES_DEBUG)
					printf("	Assigning to ptr_two\n");

				if (a_func_node != NULL)
				{
					cur_math->ptr_two = a_func_node;
					cur_math->ptr_two_type = PTR_TYPE_FUNC_NODE;

					a_func_node = NULL;

					index -= strLength(word);
				}
				else if (case_node != NULL)
				{
					cur_math->ptr_two = case_node;
					cur_math->ptr_two_type = PTR_TYPE_CASE_NODE;

					case_node = NULL;

					index -= strLength(word);
				}
				else
				{
					int hmm_int = atoi(word);

					if (hmm_int != 0 && strcontains(word, '.'))
					{
						// Double
						cur_math->ptr_two = myMalloc(sizeof(double), NULL, malloced_head, the_debug);
						if (cur_math->ptr_two == NULL)
						{
							if (the_debug == YES_DEBUG)
								printf("	ERROR in parseMathNode() at line %d in %s\n", __LINE__, __FILE__);
							return RETURN_ERROR;
						}

						double hmm_double = 0.0;

						sscanf(word, "%lf", &hmm_double);

						*((double*) cur_math->ptr_two) = hmm_double;

						cur_math->ptr_two_type = PTR_TYPE_REAL;
					}
					else if (hmm_int != 0 || word[0] == '0')
					{
						// Int
						cur_math->ptr_two = myMalloc(sizeof(int), NULL, malloced_head, the_debug);
						if (cur_math->ptr_two == NULL)
						{
							if (the_debug == YES_DEBUG)
								printf("	ERROR in parseMathNode() at line %d in %s\n", __LINE__, __FILE__);
							return RETURN_ERROR;
						}

						*((int*) cur_math->ptr_two) = hmm_int;

						cur_math->ptr_two_type = PTR_TYPE_INT;
					}
					else if (strcontains(word, '/'))
					{
						// Date
						cur_math->ptr_two = upper(word, NULL, malloced_head, the_debug);
						if (cur_math->ptr_two == NULL)
						{
							if (the_debug == YES_DEBUG)
								printf("	ERROR in parseMathNode() at line %d in %s\n", __LINE__, __FILE__);
							return RETURN_ERROR;
						}

						cur_math->ptr_two_type = PTR_TYPE_DATE;
					}
					else
					{
						char* alias = NULL;
						char* name = upper(word, NULL, malloced_head, the_debug);
						if (name == NULL)
						{
							if (the_debug == YES_DEBUG)
								printf("	ERROR in parseMathNode() at line %d in %s\n", __LINE__, __FILE__);
							return RETURN_ERROR;
						}

						int index_of_dot = indexOf(name, '.');
						if (index_of_dot > -1)
						{
							alias = substring(name, 0, index_of_dot-1, NULL, malloced_head, the_debug);
							
							char* temp = name;

							name = substring(temp, index_of_dot+1, strLength(temp)-1, NULL, malloced_head, the_debug);

							if (alias == NULL || name == NULL)
							{
								if (the_debug == YES_DEBUG)
									printf("	ERROR in parseMathNode() at line %d in %s\n", __LINE__, __FILE__);
								return RETURN_ERROR;
							}

							myFree((void**) &temp, NULL, malloced_head, the_debug);
						}
						if (the_debug == YES_DEBUG)
							printf("alias.name = _%s.%s_\n", alias, name);

						if (getColInSelectNodeFromName((void*) cur_math, PTR_TYPE_MATH_NODE, 2, 1, select_node, alias, name
													  ,malloced_head, the_debug) != RETURN_GOOD)
						{
							printf("A column was declared which could not be found. Failing word: ""%s%s%s""\n", alias == NULL ? "" : alias, alias == NULL ? "" : ".", name);
							errorTeardown(NULL, malloced_head, the_debug);
							return RETURN_ERROR;
						}

						if (alias != NULL)
							myFree((void**) &alias, NULL, malloced_head, the_debug);
						if (name != NULL)
							myFree((void**) &name, NULL, malloced_head, the_debug);


						// START Check if found col has valid datatype
							if (((struct col_in_select_node*) cur_math->ptr_two)->rows_data_type == DATA_STRING)
							{
								if (the_debug == YES_DEBUG)
									printf("	ERROR in parseMathNode() at line %d in %s\n", __LINE__, __FILE__);
								printf("A column which is of data type string was delcared in a match clause. Math cannot be done on a string. Please declare another column. Failing word: \"%s%s%s\"\n", alias == NULL ? "" : alias, alias == NULL ? "" : ".", name);
								errorTeardown(NULL, malloced_head, the_debug);
								return RETURN_ERROR;
							}
						// END Check if found col has valid datatype
					}
				}

				// START Determine result_type
					int ptr_one_datatype = -1;
					int ptr_two_datatype = -1;

					if (cur_math->ptr_one_type == PTR_TYPE_MATH_NODE)
						ptr_one_datatype = ((struct math_node*) cur_math->ptr_one)->result_type;
					else if (cur_math->ptr_one_type == PTR_TYPE_FUNC_NODE)
						ptr_one_datatype = ((struct func_node*) cur_math->ptr_one)->result_type;
					else if (cur_math->ptr_one_type == PTR_TYPE_CASE_NODE)
						ptr_one_datatype = ((struct case_node*) cur_math->ptr_one)->result_type;
					else 
						ptr_one_datatype = cur_math->ptr_one_type;

					if (cur_math->ptr_two_type == PTR_TYPE_MATH_NODE)
						ptr_two_datatype = ((struct math_node*) cur_math->ptr_two)->result_type;
					else if (cur_math->ptr_two_type == PTR_TYPE_FUNC_NODE)
						ptr_two_datatype = ((struct func_node*) cur_math->ptr_two)->result_type;
					else if (cur_math->ptr_two_type == PTR_TYPE_CASE_NODE)
						ptr_two_datatype = ((struct case_node*) cur_math->ptr_two)->result_type;
					else 
						ptr_two_datatype = cur_math->ptr_two_type;

					if (the_debug == YES_DEBUG)
						printf("Math ptr_one_datatype = %d, ptr_two_datatype = %d\n", ptr_one_datatype, ptr_two_datatype);


					if (ptr_one_datatype == PTR_TYPE_DATE || ptr_two_datatype == PTR_TYPE_DATE)
					{
						cur_math->result_type = PTR_TYPE_DATE;
					}
					else if (ptr_one_datatype == PTR_TYPE_REAL || ptr_two_datatype == PTR_TYPE_REAL)
					{
						cur_math->result_type = PTR_TYPE_REAL;
					}
					else
					{
						cur_math->result_type = PTR_TYPE_INT;
					}
					if (the_debug == YES_DEBUG)
						printf("		Assigned result_type: %d\n", cur_math->result_type);
				// END Determine result_type


				if (made_new_without_parens)
				{
					made_new_without_parens = false;
					cur_math = cur_math->parent;

					//printf("	Going back to parent, op now = %d\n", cur_math->operation);
				}
			// END Determine type of next word and put into ptr_two
		}
		else if (cur_math->ptr_one == NULL)
		{
			// START Determine type of next word and put into ptr_one
				//printf("	Assigning to ptr_one\n");
				if (a_func_node != NULL)
				{
					cur_math->ptr_two = a_func_node;
					cur_math->ptr_two_type = PTR_TYPE_FUNC_NODE;

					a_func_node = NULL;

					index -= strLength(word);
				}
				else
				{
					int hmm_int = atoi(word);

					if (hmm_int != 0 && strcontains(word, '.'))
					{
						// Double
						cur_math->ptr_one = myMalloc(sizeof(double), NULL, malloced_head, the_debug);
						if (cur_math->ptr_one == NULL)
						{
							if (the_debug == YES_DEBUG)
								printf("	ERROR in parseMathNode() at line %d in %s\n", __LINE__, __FILE__);
							return RETURN_ERROR;
						}

						double hmm_double = 0.0;

						sscanf(word, "%lf", &hmm_double);

						*((double*) cur_math->ptr_one) = hmm_double;

						cur_math->ptr_one_type = PTR_TYPE_REAL;
					}
					else if (hmm_int != 0 || word[0] == '0')
					{
						// Int
						cur_math->ptr_one = myMalloc(sizeof(int), NULL, malloced_head, the_debug);
						if (cur_math->ptr_one == NULL)
						{
							if (the_debug == YES_DEBUG)
								printf("	ERROR in parseMathNode() at line %d in %s\n", __LINE__, __FILE__);
							return RETURN_ERROR;
						}

						*((int*) cur_math->ptr_one) = hmm_int;

						cur_math->ptr_one_type = PTR_TYPE_INT;
					}
					else if (strcontains(word, '/'))
					{
						// Date
						cur_math->ptr_one = upper(word, NULL, malloced_head, the_debug);
						if (cur_math->ptr_one == NULL)
						{
							if (the_debug == YES_DEBUG)
								printf("	ERROR in parseMathNode() at line %d in %s\n", __LINE__, __FILE__);
							return RETURN_ERROR;
						}

						cur_math->ptr_one_type = PTR_TYPE_DATE;
					}
					else
					{
						// Char but column name
						//cur_math->ptr_one = upper(word, NULL, malloced_head, the_debug);
						//cur_math->ptr_one_type = PTR_TYPE_CHAR;

						char* alias = NULL;
						char* name = upper(word, NULL, malloced_head, the_debug);
						if (name == NULL)
						{
							if (the_debug == YES_DEBUG)
								printf("	ERROR in parseMathNode() at line %d in %s\n", __LINE__, __FILE__);
							return RETURN_ERROR;
						}

						int index_of_dot = indexOf(name, '.');
						if (index_of_dot > -1)
						{
							alias = substring(name, 0, index_of_dot-1, NULL, malloced_head, the_debug);
							
							char* temp = name;

							name = substring(temp, index_of_dot+1, strLength(temp)-1, NULL, malloced_head, the_debug);

							if (alias == NULL || name == NULL)
							{
								if (the_debug == YES_DEBUG)
									printf("	ERROR in parseMathNode() at line %d in %s\n", __LINE__, __FILE__);
								return RETURN_ERROR;
							}

							myFree((void**) &temp, NULL, malloced_head, the_debug);
						}
						if (the_debug == YES_DEBUG)
							printf("alias.name = _%s.%s_\n", alias, name);

						if (getColInSelectNodeFromName((void*) cur_math, PTR_TYPE_MATH_NODE, 1, 1, select_node, alias, name
													  ,malloced_head, the_debug) != RETURN_GOOD)
						{
							printf("A column was declared which could not be found. Failing word: ""%s%s%s""\n", alias == NULL ? "" : alias, alias == NULL ? "" : ".", name);
							errorTeardown(NULL, malloced_head, the_debug);
							return RETURN_ERROR;
						}

						if (alias != NULL)
							myFree((void**) &alias, NULL, malloced_head, the_debug);
						if (name != NULL)
							myFree((void**) &name, NULL, malloced_head, the_debug);
						

						// START Check if found col has valid datatype
							if (((struct col_in_select_node*) cur_math->ptr_one)->rows_data_type == DATA_STRING)
							{
								if (the_debug == YES_DEBUG)
									printf("	ERROR in parseMathNode() at line %d in %s\n", __LINE__, __FILE__);
								printf("A column which is of data type string was delcared in a match clause. Math cannot be done on a string. Please declare another column. Failing word: \"%s%s%s\"\n", alias == NULL ? "" : alias, alias == NULL ? "" : ".", name);
								errorTeardown(NULL, malloced_head, the_debug);
								return RETURN_ERROR;
							}
						// END Check if found col has valid datatype
					}
				}
			// END Determine type of next word and put into ptr_one
		}
		else if (input[index] == 34)
		{
			// START Column is given a new name with double quotes
				if (new_col_name != NULL)
				{
					if (the_debug == YES_DEBUG)
						printf("math_node col is given new name with quotes\n");
					
					*new_col_name = upper(word, NULL, malloced_head, the_debug);
					if (*new_col_name == NULL)
					{
						if (the_debug == YES_DEBUG)
							printf("	ERROR in parseMathNode() at line %d in %s\n", __LINE__, __FILE__);
						return RETURN_ERROR;
					}

					int str_len = strLength(*new_col_name);

					for (int i=1; i<str_len-1; i++)
						(*new_col_name)[i-1] = (*new_col_name)[i];

					(*new_col_name)[str_len-2] = 0;
				}
			// END Column is given a new name with double quotes
		}
		else
		{
			// START Column is given a new name
				if (new_col_name != NULL)
				{
					if (the_debug == YES_DEBUG)
						printf("math_node col is given new name\n");

					if (strcmp_Upper(word, "AS") == 0)
						getNextWord(input, word, &index);

					*new_col_name = upper(word, NULL, malloced_head, the_debug);
					if (*new_col_name == NULL)
					{
						if (the_debug == YES_DEBUG)
							printf("	ERROR in parseMathNode() at line %d in %s\n", __LINE__, __FILE__);
						return RETURN_ERROR;
					}

					if (*new_col_name == NULL || (*new_col_name)[0] == '(' || (*new_col_name)[0] == ')')
					{
						if (the_debug == YES_DEBUG)
							printf("	ERROR in parseMathNode() at line %d in %s\n", __LINE__, __FILE__);
						errorTeardown(NULL, malloced_head, the_debug);
						return RETURN_ERROR;
					}

					if ((*new_col_name)[0] == 34)
					{
						int str_len = strLength(*new_col_name);

						for (int i=1; i<str_len-1; i++)
							(*new_col_name)[i-1] = (*new_col_name)[i];

						(*new_col_name)[str_len-2] = 0;
					}
					else if (atoi(*new_col_name) > 0)
					{
						if (the_debug == YES_DEBUG)
							printf("	ERROR in parseMathNode() at line %d in %s\n", __LINE__, __FILE__);
						printf("A math column was given a new name which is a number. Please declare another name. Failing word: \"%s\"\n", *new_col_name);
						errorTeardown(NULL, malloced_head, the_debug);
						return RETURN_ERROR;
					}
				}
			// END Column is given a new name
		}
	}

	if (new_col_name != NULL && *new_col_name == NULL)
	{
		if (the_debug == YES_DEBUG)
			printf("	ERROR in parseMathNode() at line %d in %s\n", __LINE__, __FILE__);
		printf("A column containing math was created but it was not named. Please provide a name.\n");
		errorTeardown(NULL, malloced_head, the_debug);
		return RETURN_ERROR;
	}

	if (new_col_name == NULL)
	{
		// START This function was parsing math_node in where clause or then value, ensure *the_math_node points correctly
			if (cur_math->ptr_two == NULL)
			{
				struct math_node* temp = cur_math;

				cur_math = cur_math->ptr_one;

				cur_math->parent = temp->parent;

				myFree((void**) &temp, NULL, malloced_head, the_debug);

				*the_math_node = cur_math;
			}
		// END This function was parsing math_node in where clause or then value, ensure *the_math_node points correctly
	}

	if ((*the_math_node)->result_type == -1)
	{
		// START Determine result_type
			int ptr_one_datatype = -1;
			int ptr_two_datatype = -1;

			if (cur_math->ptr_one_type == PTR_TYPE_MATH_NODE)
				ptr_one_datatype = ((struct math_node*) cur_math->ptr_one)->result_type;
			else if (cur_math->ptr_one_type == PTR_TYPE_FUNC_NODE)
				ptr_one_datatype = ((struct func_node*) cur_math->ptr_one)->result_type;
			else if (cur_math->ptr_one_type == PTR_TYPE_CASE_NODE)
				ptr_one_datatype = ((struct case_node*) cur_math->ptr_one)->result_type;
			else 
				ptr_one_datatype = cur_math->ptr_one_type;

			if (cur_math->ptr_two_type == PTR_TYPE_MATH_NODE)
				ptr_two_datatype = ((struct math_node*) cur_math->ptr_two)->result_type;
			else if (cur_math->ptr_two_type == PTR_TYPE_FUNC_NODE)
				ptr_two_datatype = ((struct func_node*) cur_math->ptr_two)->result_type;
			else if (cur_math->ptr_two_type == PTR_TYPE_CASE_NODE)
				ptr_two_datatype = ((struct case_node*) cur_math->ptr_two)->result_type;
			else 
				ptr_two_datatype = cur_math->ptr_two_type;


			if (ptr_one_datatype == PTR_TYPE_DATE || ptr_two_datatype == PTR_TYPE_DATE)
			{
				(*the_math_node)->result_type = PTR_TYPE_DATE;
			}
			else if (ptr_one_datatype == PTR_TYPE_REAL || ptr_two_datatype == PTR_TYPE_REAL)
			{
				(*the_math_node)->result_type = PTR_TYPE_REAL;
			}
			else
			{
				(*the_math_node)->result_type = PTR_TYPE_INT;
			}
			if (the_debug == YES_DEBUG)
				printf("		Assigned result_type: %d\n", (*the_math_node)->result_type);
		// END Determine result_type
	}

	return RETURN_GOOD;
}

/*	RETURNS:
 *  RETURN_ERROR if error
 *	RETURN_GOOD if good
 *
 *	WRITES TO:
 *  new_col_name
 */
int parseNewColumnName(char** new_col_name, char* input, char** word, int* index
					  ,struct malloced_node** malloced_head, int the_debug)
{
	if (the_debug == YES_DEBUG)
		printf("Column is given a new name: _%s_, _%s_\n", *word, input);
	if (*new_col_name != NULL)
		myFree((void**) new_col_name, NULL, malloced_head, the_debug);

	if (strcmp_Upper(*word, "AS") == 0)
		getNextWord(input, *word, index);

	char* temp_upper = NULL;
	if ((*word)[0] != 34)
	{
		temp_upper = upper(*word, NULL, malloced_head, the_debug);
		if (temp_upper == NULL)
		{
			if (the_debug == YES_DEBUG)
				printf("	ERROR in parseNewColumnName() at line %d in %s\n", __LINE__, __FILE__);
			return RETURN_ERROR;
		}
	}
	else
	{
		temp_upper = myMalloc(sizeof(char) * (strLength(*word)+1), NULL, malloced_head, the_debug);
		if (temp_upper == NULL)
		{
			if (the_debug == YES_DEBUG)
				printf("	ERROR in parseNewColumnName() at line %d in %s\n", __LINE__, __FILE__);
			return RETURN_ERROR;
		}

		strcpy(temp_upper, *word);
	}

	if (input[0] == '*' && !strcontains(*word, '#') && !strcontains(*word, '@'))
	{
		if (the_debug == YES_DEBUG)
			printf("	ERROR in parseNewColumnName() at line %d in %s\n", __LINE__, __FILE__);
		printf("A blanket column name was declared but did not contain a '#' or '@'. Please include one of those characters as the resulting column names will be identical. Failing word: \"%s\"\n", *word);
		errorTeardown(NULL, malloced_head, the_debug);
		return RETURN_ERROR;
	}

	*new_col_name = (char*) myMalloc(sizeof(char) * 256, NULL, malloced_head, the_debug);
	if (*new_col_name == NULL)
	{
		if (the_debug == YES_DEBUG)
			printf("	ERROR in parseNewColumnName() at line %d in %s\n", __LINE__, __FILE__);
		return RETURN_ERROR;
	}

	strcpy(*new_col_name, temp_upper);

	myFree((void**) &temp_upper, NULL, malloced_head, the_debug);

	trimStr(*new_col_name);

	if (atoi(*new_col_name) > 0)
	{
		if (the_debug == YES_DEBUG)
			printf("	ERROR in parseNewColumnName() at line %d in %s\n", __LINE__, __FILE__);
		printf("A column was given a new name which is a number. Please declare another name. Failing word: \"%s\"\n", *new_col_name);
		errorTeardown(NULL, malloced_head, the_debug);
		return RETURN_ERROR;
	}

	return RETURN_GOOD;
}

/*	RETURNS:
 *  RETURN_ERROR if error
 *	RETURN_GOOD if good
 *
 *	WRITES TO:
 *  the_func_node
 *  new_col_name
 */
int parseFuncNode(struct func_node** the_func_node, char** new_col_name, char** col_name, char* input, char* word, int* index, struct select_node* select_node
				 ,bool rec, struct malloced_node** malloced_head, int the_debug)
{
	if (the_debug == YES_DEBUG)
		printf("Beginning of func node: _%s_ w/ index = %d\n", input, *index);
	if (*index == 0)
	{
		getNextWord(input, word, index);
		getNextWord(input, word, index);
	}

	(*the_func_node) = (struct func_node*) myMalloc(sizeof(struct func_node), NULL, malloced_head, the_debug);
	if (*the_func_node == NULL)
	{
		if (the_debug == YES_DEBUG)
			printf("	ERROR in parseFuncNode() at line %d in %s\n", __LINE__, __FILE__);
		*the_func_node = NULL;
		return RETURN_ERROR;
	}

	if (strcmp_Upper((*col_name), "COUNT") == 0)
		(*the_func_node)->which_func = FUNC_COUNT;
	else if (strcmp_Upper((*col_name), "AVG") == 0)
		(*the_func_node)->which_func = FUNC_AVG;
	else if (strcmp_Upper((*col_name), "FIRST") == 0)
		(*the_func_node)->which_func = FUNC_FIRST;
	else if (strcmp_Upper((*col_name), "LAST") == 0)
		(*the_func_node)->which_func = FUNC_LAST;
	else if (strcmp_Upper((*col_name), "MIN") == 0)
		(*the_func_node)->which_func = FUNC_MIN;
	else if (strcmp_Upper((*col_name), "MAX") == 0)
		(*the_func_node)->which_func = FUNC_MAX;
	else if (strcmp_Upper((*col_name), "MEDIAN") == 0)
		(*the_func_node)->which_func = FUNC_MEDIAN;
	else if (strcmp_Upper((*col_name), "SUM") == 0)
		(*the_func_node)->which_func = FUNC_SUM;
	else if (strcmp_Upper((*col_name), "RANK") == 0)
		(*the_func_node)->which_func = FUNC_RANK;
	else
	{
		if (the_debug == YES_DEBUG)
			printf("	ERROR in parseFuncNode() at line %d in %s\n", __LINE__, __FILE__);
		errorTeardown(NULL, malloced_head, the_debug);
		*the_func_node = NULL;
		return RETURN_ERROR;
	}

	myFree((void**) &(*col_name), NULL, malloced_head, the_debug);
	*col_name = NULL;

	struct ListNodePtr* args_head = NULL;
	struct ListNodePtr* args_tail = NULL;

	(*the_func_node)->args_size = 1;

	addListNodePtr(&args_head, &args_tail, NULL, -1, ADDLISTNODE_TAIL, NULL, malloced_head, the_debug);

	(*the_func_node)->distinct = false;

	while(getNextWord(input, word, index) == 0 && word[0] != ')')
	{
		if (the_debug == YES_DEBUG)
			printf("word in func node loop = _%s_\n", word);

		if (word[0] == '(')
		{
			if (the_debug == YES_DEBUG)
				printf("	ERROR in parseFuncNode() at line %d in %s\n", __LINE__, __FILE__);
			errorTeardown(NULL, malloced_head, the_debug);
			*the_func_node = NULL;
			return RETURN_ERROR;
		}
		else if (strcmp_Upper(word, "DISTINCT") == 0)
		{
			if ((*the_func_node)->distinct)
			{
				if (the_debug == YES_DEBUG)
					printf("	ERROR in parseFuncNode() at line %d in %s\n", __LINE__, __FILE__);
				errorTeardown(NULL, malloced_head, the_debug);
				*the_func_node = NULL;
				return RETURN_ERROR;
			}

			(*the_func_node)->distinct = true;
		}
		else if (strcmp_Upper(word, "COUNT") == 0 || strcmp_Upper(word, "AVG") == 0
				|| strcmp_Upper(word, "FIRST") == 0 || strcmp_Upper(word, "LAST") == 0
				|| strcmp_Upper(word, "MIN") == 0 || strcmp_Upper(word, "MAX") == 0
				|| strcmp_Upper(word, "MEDIAN") == 0 || strcmp_Upper(word, "SUM") == 0
				|| strcmp_Upper(word, "RANK") == 0)
		{
			char* func_name = (char*) myMalloc(sizeof(char) * (strLength(word)+1), NULL, malloced_head, the_debug);
			if (func_name == NULL)
			{
				if (the_debug == YES_DEBUG)
					printf("	ERROR in parseFuncNode() at line %d in %s\n", __LINE__, __FILE__);
				*the_func_node = NULL;
				return RETURN_ERROR;
			}
			strcpy(func_name, word);

			getNextWord(input, word, index);

			if (word[0] != '(')
			{
				if (the_debug == YES_DEBUG)
					printf("	ERROR in parseFuncNode() at line %d in %s\n", __LINE__, __FILE__);
				errorTeardown(NULL, malloced_head, the_debug);
				*the_func_node = NULL;
				return RETURN_ERROR;
			}

			struct func_node* new_func_node = NULL;

			int new_index = *index;

			if (the_debug == YES_DEBUG)
				printf("Going recursively\n");
			if (parseFuncNode(&new_func_node, NULL, &func_name, input, word, &new_index, select_node, true
							,malloced_head, the_debug) != RETURN_GOOD)
			{
				if (the_debug == YES_DEBUG)
					printf("	ERROR in parseFuncNode() at line %d in %s\n", __LINE__, __FILE__);
				return RETURN_ERROR;
			}
			if (the_debug == YES_DEBUG)
				printf("Back from recursion\n");

			int open_parens = 1;
			while (open_parens > 0)
			{
				getNextWord(input, word, index);

				if (word[0] == '(')
					open_parens++;
				else if (word[0] == ')')
					open_parens--;
			}

			if (the_debug == YES_DEBUG)
				printf("word after loop = _%s_\n", word);

			args_tail->ptr_value = new_func_node;
			args_tail->ptr_type = PTR_TYPE_FUNC_NODE;
		}
		else if (word[0] == ',')
		{
			(*the_func_node)->args_size++;
			addListNodePtr(&args_head, &args_tail, NULL, -1, ADDLISTNODE_TAIL, NULL, malloced_head, the_debug);
		}
		else
		{
			char* alias = NULL;
			char* name = word;

			if (input[*index] == '.')
			{
				alias = myMalloc(sizeof(char) * (strLength(word)+1), NULL, malloced_head, the_debug);
				if (alias == NULL)
				{
					if (the_debug == YES_DEBUG)
						printf("	ERROR in parseFuncNode() at line %d in %s\n", __LINE__, __FILE__);
					*the_func_node = NULL;
					return RETURN_ERROR;
				}
				strcpy(alias, word);

				(*index)++;

				getNextWord(input, word, index);

				name = word;
			}
			if (the_debug == YES_DEBUG)
				printf("alias.name = _%s.%s_\n", alias, name);

			if (name[0] == '*')
			{
				if ((*the_func_node)->which_func != FUNC_COUNT)
				{
					if (the_debug == YES_DEBUG)
						printf("	ERROR in parseFuncNode() at line %d in %s\n", __LINE__, __FILE__);
					printf("A function which is not COUNT() was passed the parameter '*'. Please pass a different parameter, or change the function to COUNT().\n");
					errorTeardown(NULL, malloced_head, the_debug);
					*the_func_node = NULL;
					return RETURN_ERROR;
				}

				if (getAllColsCuzStar(&select_node, &args_tail, alias, 0, 0, NULL, NULL, false, malloced_head, the_debug) != RETURN_GOOD)
				{
					if (the_debug == YES_DEBUG)
						printf("	ERROR in parseFuncNode() at line %d in %s\n", __LINE__, __FILE__);
					return RETURN_ERROR;
				}

				if (the_debug == YES_DEBUG)
					printf("Returned from getAllColsCuzStar()\n");

				struct ListNodePtr* cur = args_tail;
				while (cur->prev != NULL)
				{
					(*the_func_node)->args_size++;
					cur = cur->prev;
				}

				//printf("(*the_func_node)->args_size = %d\n", (*the_func_node)->args_size);
			}
			else
			{
				if (getColInSelectNodeFromName((void*) args_tail, PTR_TYPE_LIST_NODE_PTR, 1, -1, select_node, alias, name
											  ,malloced_head, the_debug) != RETURN_GOOD)
				{
					if (the_debug == YES_DEBUG)
						printf("	ERROR in parseFuncNode() at line %d in %s\n", __LINE__, __FILE__);
					printf("A column was declared which could not be found. Failing word: ""%s%s%s""\n", alias == NULL ? "" : alias, alias == NULL ? "" : ".", name);
					*the_func_node = NULL;
					return RETURN_ERROR;
				}

				// START Check column datatypes against functions
					if ((*the_func_node)->which_func != FUNC_COUNT && (*the_func_node)->which_func != FUNC_RANK
						&& ((struct col_in_select_node*) args_tail->ptr_value)->rows_data_type == DATA_STRING)
					{
						if (the_debug == YES_DEBUG)
							printf("	ERROR in parseFuncNode() at line %d in %s\n", __LINE__, __FILE__);
						printf("Failing word: \"%s\"\n", word);
						errorTeardown(NULL, malloced_head, the_debug);
						*the_func_node = NULL;
						return RETURN_ERROR;
					}
				// END Check column datatypes against functions
			}

			if (alias != NULL)
			{
				myFree((void**) &alias, NULL, malloced_head, the_debug);
			}
		}
	}

	int args_size = (*the_func_node)->args_size;
	(*the_func_node)->args_arr = (void**) myMalloc(sizeof(void*) * args_size, NULL, malloced_head, the_debug);
	(*the_func_node)->args_arr_type = (int*) myMalloc(sizeof(int) * args_size, NULL, malloced_head, the_debug);
	if ((*the_func_node)->args_arr == NULL || (*the_func_node)->args_arr_type == NULL)
	{
		if (the_debug == YES_DEBUG)
			printf("	ERROR in parseFuncNode() at line %d in %s\n", __LINE__, __FILE__);
		return RETURN_ERROR;
	}
	
	struct ListNodePtr* cur = args_head;
	for (int j=0; j<args_size; j++)
	{
		if (cur->ptr_value == NULL)
		{
			if (the_debug == YES_DEBUG)
				printf("	ERROR in parseFuncNode() at line %d in %s\n", __LINE__, __FILE__);
			printf("Failing word: \"%s\"\n", word);
			errorTeardown(NULL, malloced_head, the_debug);
			*the_func_node = NULL;
			return RETURN_ERROR;
		}

		(*the_func_node)->args_arr[j] = cur->ptr_value;
		cur->ptr_value = NULL;

		(*the_func_node)->args_arr_type[j] = cur->ptr_type;

		cur = cur->next;
	}

	int freed = freeAnyLinkedList((void**) &args_head, PTR_TYPE_LIST_NODE_PTR, NULL, malloced_head, the_debug);
	//printf("freed %d from args_head\n", freed);

	(*the_func_node)->group_by_cols_head = NULL;
	(*the_func_node)->group_by_cols_tail = NULL;


	// START Check if all functions except rank have more than 1 arg
		if ((*the_func_node)->which_func != FUNC_RANK && (*the_func_node)->which_func != FUNC_COUNT && (*the_func_node)->args_size != 1)
		{
			if (the_debug == YES_DEBUG)
				printf("	ERROR in parseFuncNode() at line %d in %s\n", __LINE__, __FILE__);
			printf("A function was passed more than one argument. A function can only support one argument. Please erase all extra arguments. Failing word: \"%s\"\n", word);
			errorTeardown(NULL, malloced_head, the_debug);
			*the_func_node = NULL;
			return RETURN_ERROR;
		}
	// END Check if all functions except rank have more than 1 arg


	// START Error checking and parsing of given new name
		if (getNextWord(input, word, index) == 0)
		{
			if (word[0] == '(' || word[0] == ')')
			{
				if (!rec)
				{
					if (the_debug == YES_DEBUG)
						printf("	ERROR in parseFuncNode() at line %d in %s\n", __LINE__, __FILE__);
					printf("A an extra parentheses was declared in a function. Please erase it. Failing word: \"%s\"\n", word);
					errorTeardown(NULL, malloced_head, the_debug);
					*the_func_node = NULL;
					return RETURN_ERROR;
				}	
			}
			else if (word[0] != '+' && word[0] != '-' && word[0] != '*' && word[0] != '/' && word[0] != '^')
			{
				if (parseNewColumnName(new_col_name, input, &word, index, malloced_head, the_debug) != RETURN_GOOD)
				{
					if (the_debug == YES_DEBUG)
						printf("	ERROR in parseFuncNode() at line %d in %s\n", __LINE__, __FILE__);
					*the_func_node = NULL;
					return RETURN_ERROR;
				}
			}

			if (new_col_name != NULL && the_debug == YES_DEBUG)
				printf("new_col_name = _%s_\n", (*new_col_name));
		}
		else if (new_col_name != NULL && *new_col_name == NULL)
		{
			if (the_debug == YES_DEBUG)
				printf("	ERROR in parseMathNode() at line %d in %s\n", __LINE__, __FILE__);
			printf("A column containing a function was created but it was not named. Please provide a name.\n");
			errorTeardown(NULL, malloced_head, the_debug);
			return RETURN_ERROR;
		}
	// END Error checking and parsing of given new name


	// START Determine data type of result
		if ((*the_func_node)->which_func == FUNC_COUNT)
			(*the_func_node)->result_type = PTR_TYPE_INT;
		else if ((*the_func_node)->which_func == FUNC_AVG)
			(*the_func_node)->result_type = PTR_TYPE_REAL;
		else
		{
			if ((*the_func_node)->args_arr_type[0] == PTR_TYPE_COL_IN_SELECT_NODE)
				(*the_func_node)->result_type = ((struct col_in_select_node*) (*the_func_node)->args_arr[0])->rows_data_type;
			else if ((*the_func_node)->args_arr_type[0] == PTR_TYPE_FUNC_NODE)
				(*the_func_node)->result_type = ((struct func_node*) (*the_func_node)->args_arr[0])->result_type;
		}
	// END Determine data type of result

	if (the_debug == YES_DEBUG)
		printf("	func's args_size = %d\n", args_size);

	return RETURN_GOOD;
}

/*	RETURNS:
 *  RETURN_ERROR if error
 *	RETURN_GOOD if good
 *
 *	WRITES TO:
 *  col_case_node
 *  new_col_name
 */
int parseCaseNode(struct case_node** col_case_node, char* input, char* word, int index, char** new_col_name, struct select_node* select_node, int* return_ptr_type
				 ,struct malloced_node** malloced_head, int the_debug)
{
	if (*col_case_node == NULL)
	{
		*col_case_node = (struct case_node*) myMalloc(sizeof(struct case_node), NULL, malloced_head, the_debug);
		if (*col_case_node == NULL)
		{
			if (the_debug == YES_DEBUG)
				printf("	ERROR in parseCaseNode() at line %d in %s\n", __LINE__, __FILE__);
			return RETURN_ERROR;
		}

		(*col_case_node)->case_when_head = NULL;
		(*col_case_node)->case_when_tail = NULL;

		(*col_case_node)->case_then_value_head = NULL;
		(*col_case_node)->case_then_value_tail = NULL;
	}

	char* case_when_str = (char*) myMalloc(sizeof(char) * 1000, NULL, malloced_head, the_debug);
	if (case_when_str == NULL)
	{
		if (the_debug == YES_DEBUG)
			printf("	ERROR in parseCaseNode() at line %d in %s\n", __LINE__, __FILE__);
		return RETURN_ERROR;
	}

	case_when_str[0] = 0;

	while (getNextWord(input, word, &index) == 0)
	{
		if (the_debug == YES_DEBUG)
			printf(" word = _%s_\n", word);

		if (case_when_str[0] != 0 && (strcmp_Upper(word, "WHEN") == 0 
									  || strcmp_Upper(word, "ELSE") == 0
									  || strcmp_Upper(word, "END") == 0))
		{
			if (strcmp_Upper(word, "ELSE") == 0)
			{
				// START Add to null ptr case_when_tail
					if (addListNodePtr(&(*col_case_node)->case_when_head, &(*col_case_node)->case_when_tail, NULL, -1, ADDLISTNODE_TAIL
									  ,NULL, malloced_head, the_debug) != 0)
					{
						if (the_debug == YES_DEBUG)
							printf("	ERROR in parseCaseNode() at line %d in %s\n", __LINE__, __FILE__);
						return RETURN_ERROR;
					}
				// END Add to null ptr case_when_tail
			}

			// START Add to case_then_value_tail
				if (the_debug == YES_DEBUG)
					printf("   parsing then = _%s_\n", case_when_str);

				char* math_word = (char*) myMalloc(sizeof(char) * 200, NULL, malloced_head, the_debug);
				if (math_word == NULL)
				{
					if (the_debug == YES_DEBUG)
						printf("	ERROR in parseCaseNode() at line %d in %s\n", __LINE__, __FILE__);
					return RETURN_ERROR;
				}

				math_word[0] = 0;

				int math_index = 0;

				char* first_math_word = NULL;

				getNextWord(case_when_str, math_word, &math_index);

				if (strcmp_Upper(math_word, "CASE") == 0)
				{
					if (the_debug == YES_DEBUG)
						printf("	ERROR in parseCaseNode() at line %d in %s\n", __LINE__, __FILE__);
					printf("A case statement was declared as the then value for an already declared case. Please erase this case statement or make it another when clause in the original case statement. Failing word: \"%s\"\n", math_word);
					errorTeardown(NULL, malloced_head, the_debug);
					return RETURN_ERROR;
				}
				else if (strcmp_Upper(math_word, "NULL") == 0)
				{
					if (addListNodePtr(&(*col_case_node)->case_then_value_head, &(*col_case_node)->case_then_value_tail, NULL, -1, ADDLISTNODE_TAIL
									  ,NULL, malloced_head, the_debug) != 0)
					{
						if (the_debug == YES_DEBUG)
							printf("	ERROR in parseCaseNode() at line %d in %s\n", __LINE__, __FILE__);
						return RETURN_ERROR;
					}

					if (getNextWord(case_when_str, math_word, &math_index) == 0 && math_word[0] != 0)
					{
						if (the_debug == YES_DEBUG)
							printf("	ERROR in parseCaseNode() at line %d in %s\n", __LINE__, __FILE__);
						printf("A word was declared after NULL in a case statement. Please erase the word(s) or edit the THEN value. Failing word: \"%s\"\n", math_word);
						errorTeardown(NULL, malloced_head, the_debug);
						return RETURN_ERROR;
					}
				}
				else if ((strcontains(case_when_str, '+') || strcontains(case_when_str, '-') || strcontains(case_when_str, '*') || strcontains(case_when_str, '/') || strcontains(case_when_str, '^'))
					&& (!strcontains(case_when_str, 39) && indexOf(case_when_str, ' ') < strLength(case_when_str)-1))
				{
					// START Parse math for then value
						if (the_debug == YES_DEBUG)
							printf("Found math in then\n");

						struct math_node* new_math_node = NULL;
						
						if (case_when_str[math_index] == '.')
						{
							char* temp = (char*) myMalloc(sizeof(char) * 200, NULL, malloced_head, the_debug);
							if (temp == NULL)
							{
								if (the_debug == YES_DEBUG)
									printf("	ERROR in parseCaseNode() at line %d in %s\n", __LINE__, __FILE__);
								return RETURN_ERROR;
							}

							strcpy(temp, math_word);
							strcat(temp, ".");

							math_index++;
							getNextWord(case_when_str, math_word, &math_index);

							strcat(temp, math_word);

							myFree((void**) &math_word, NULL, malloced_head, the_debug);

							math_word = temp;
						}

						if (math_word[0] != '(')
						{
							first_math_word = upper(math_word, NULL, malloced_head, the_debug);
							if (first_math_word == NULL)
							{
								if (the_debug == YES_DEBUG)
									printf("	ERROR in parseCaseNode() at line %d in %s\n", __LINE__, __FILE__);
								return RETURN_ERROR;
							}

							getNextWord(case_when_str, math_word, &math_index);
						}

						if (the_debug == YES_DEBUG)
							printf("math_word = _%s_\n", math_word);

						if (first_math_word != NULL &&
							(strcmp_Upper(first_math_word, "COUNT") == 0 || 
							strcmp_Upper(first_math_word, "AVG") == 0 || 
							strcmp_Upper(first_math_word, "FIRST") == 0 || 
							strcmp_Upper(first_math_word, "LAST") == 0 ||
							strcmp_Upper(first_math_word, "MIN") == 0 || 
							strcmp_Upper(first_math_word, "MAX") == 0 || 
							strcmp_Upper(first_math_word, "MEDIAN") == 0 || 
							strcmp_Upper(first_math_word, "SUM") == 0))
						{
							// START Found func in math node
								if (the_debug == YES_DEBUG)
									printf("Found func in math_node in case then\n");

								struct func_node* temp_new_func = NULL;

								if (parseFuncNode(&temp_new_func, NULL, &first_math_word, case_when_str, math_word, &math_index, select_node, false
												  ,malloced_head, the_debug) != RETURN_GOOD)
								{
									return RETURN_ERROR;
								}

								if (the_debug == YES_DEBUG)
									printf("Done parsing func_node\n");

								new_math_node = (struct math_node*) myMalloc(sizeof(struct math_node), NULL, malloced_head, the_debug);
								if (new_math_node == NULL)
								{
									if (the_debug == YES_DEBUG)
										printf("	ERROR in parseCaseNode() at line %d in %s\n", __LINE__, __FILE__);
									return RETURN_ERROR;
								}

								new_math_node->ptr_one = temp_new_func;
								new_math_node->ptr_one_type = PTR_TYPE_FUNC_NODE;

								new_math_node->ptr_two = NULL;
								new_math_node->ptr_two_type = -1;

								new_math_node->operation = -1;
								new_math_node->parent = NULL;
								new_math_node->result_type = -1;

								if (the_debug == YES_DEBUG)
									printf("math_word after func = _%s_\n", math_word);
							// END Found func in math node
						}


						if (math_word[0] != 0)
						{
							if (parseMathNode(&new_math_node, NULL, &first_math_word, case_when_str, math_word, math_index, select_node
											 ,malloced_head, the_debug) != RETURN_GOOD)
							{
								if (the_debug == YES_DEBUG)
									printf("	ERROR in parseCaseNode() at line %d in %s\n", __LINE__, __FILE__);
								return RETURN_ERROR;
							}

							if (addListNodePtr(&(*col_case_node)->case_then_value_head, &(*col_case_node)->case_then_value_tail, new_math_node, PTR_TYPE_MATH_NODE, ADDLISTNODE_TAIL
											  ,NULL, malloced_head, the_debug) != 0)
							{
								if (the_debug == YES_DEBUG)
									printf("	ERROR in parseCaseNode() at line %d in %s\n", __LINE__, __FILE__);
								return RETURN_ERROR;
							}
						}
						else
						{
							struct func_node* func_node = new_math_node->ptr_one;

							myFree((void**) &new_math_node, NULL, malloced_head, the_debug);

							if (addListNodePtr(&(*col_case_node)->case_then_value_head, &(*col_case_node)->case_then_value_tail, func_node, PTR_TYPE_FUNC_NODE, ADDLISTNODE_TAIL
											  ,NULL, malloced_head, the_debug) != 0)
							{
								if (the_debug == YES_DEBUG)
									printf("	ERROR in parseCaseNode() at line %d in %s\n", __LINE__, __FILE__);
								return RETURN_ERROR;
							}
						}

						if (first_math_word != NULL)
							myFree((void**) &first_math_word, NULL, malloced_head, the_debug);
					// END Parse math for then value
				}
				else if (strcmp_Upper(math_word, "COUNT") == 0 || 
						strcmp_Upper(math_word, "AVG") == 0 || 
						strcmp_Upper(math_word, "FIRST") == 0 || 
						strcmp_Upper(math_word, "LAST") == 0 ||
						strcmp_Upper(math_word, "MIN") == 0 || 
						strcmp_Upper(math_word, "MAX") == 0 || 
						strcmp_Upper(math_word, "MEDIAN") == 0 || 
						strcmp_Upper(math_word, "SUM") == 0)
				{
					// START Parse function
						struct func_node* temp_new_func = NULL;

						char* first_math_word = upper(math_word, NULL, malloced_head, the_debug);
						if (first_math_word == NULL)
						{
							if (the_debug == YES_DEBUG)
								printf("	ERROR in parseCaseNode() at line %d in %s\n", __LINE__, __FILE__);
							return RETURN_ERROR;
						}

						getNextWord(case_when_str, math_word, &math_index);

						if (parseFuncNode(&temp_new_func, NULL, &first_math_word, case_when_str, math_word, &math_index, select_node, false
										  ,malloced_head, the_debug) != RETURN_GOOD)
						{
							if (the_debug == YES_DEBUG)
								printf("	ERROR in parseCaseNode() at line %d in %s\n", __LINE__, __FILE__);
							return RETURN_ERROR;
						}

						if (addListNodePtr(&(*col_case_node)->case_then_value_head, &(*col_case_node)->case_then_value_tail, temp_new_func, PTR_TYPE_FUNC_NODE, ADDLISTNODE_TAIL
										  ,NULL, malloced_head, the_debug) != 0)
						{
							if (the_debug == YES_DEBUG)
								printf("	ERROR in parseCaseNode() at line %d in %s\n", __LINE__, __FILE__);
							return RETURN_ERROR;
						}
					// END Parse function
				}
				else
				{
					// START Parse single word for then value
						void* temp = myMalloc(sizeof(char) * (strLength(case_when_str)+1), NULL, malloced_head, the_debug);
						if (temp == NULL)
						{
							if (the_debug == YES_DEBUG)
								printf("	ERROR in parseCaseNode() at line %d in %s\n", __LINE__, __FILE__);
							return RETURN_ERROR;
						}

						strcpy((char*) temp, case_when_str);

						int new_ptr_type = -1;

						if (((char*) temp)[0] == 39)
						{
							trimStr(temp);

							if (isDate((char*) temp))
								new_ptr_type = PTR_TYPE_DATE;
							else
								new_ptr_type = PTR_TYPE_CHAR;
						}
						else
						{
							trimStr(case_when_str);
							//printf("trimmed case_when_str = _%s_\n", case_when_str);

							int hmm_int = atoi(case_when_str);
							//printf("hmm_int = %d\n", hmm_int);

							myFree((void**) &temp, NULL, malloced_head, the_debug);

							if (hmm_int != 0 && strcontains(case_when_str, '.'))
							{
								temp = myMalloc(sizeof(double), NULL, malloced_head, the_debug);
								if (temp == NULL)
								{
									if (the_debug == YES_DEBUG)
										printf("	ERROR in parseCaseNode() at line %d in %s\n", __LINE__, __FILE__);
									return RETURN_ERROR;
								}

								double hmm_double = 0.0;
								sscanf(case_when_str, "%lf", &hmm_double);
								*((double*) temp) = hmm_double;

								new_ptr_type = PTR_TYPE_REAL;
							}
							else if (hmm_int != 0 || case_when_str[0] == '0')
							{
								temp = myMalloc(sizeof(int), NULL, malloced_head, the_debug);
								if (temp == NULL)
								{
									if (the_debug == YES_DEBUG)
										printf("	ERROR in parseCaseNode() at line %d in %s\n", __LINE__, __FILE__);
									return RETURN_ERROR;
								}

								*((int*) temp) = hmm_int;

								new_ptr_type = PTR_TYPE_INT;
							}
							else if (hmm_int == 0)
							{
								// START Char but column name, so get column
									char* alias = NULL;
									char* name = upper(math_word, NULL, malloced_head, the_debug);
									if (name == NULL)
									{
										if (the_debug == YES_DEBUG)
											printf("	ERROR in parseCaseNode() at line %d in %s\n", __LINE__, __FILE__);
										return RETURN_ERROR;
									}

									if (case_when_str[math_index] == '.')
									{
										alias = upper(name, NULL, malloced_head, the_debug);
										if (alias == NULL)
										{
											if (the_debug == YES_DEBUG)
												printf("	ERROR in parseCaseNode() at line %d in %s\n", __LINE__, __FILE__);
											return RETURN_ERROR;
										}

										math_index++;
										getNextWord(case_when_str, math_word, &math_index);

										myFree((void**) &name, NULL, malloced_head, the_debug);

										name = upper(math_word, NULL, malloced_head, the_debug);
										if (name == NULL)
										{
											if (the_debug == YES_DEBUG)
												printf("	ERROR in parseCaseNode() at line %d in %s\n", __LINE__, __FILE__);
											return RETURN_ERROR;
										}
									}
									if (the_debug == YES_DEBUG)
										printf("alias.name = _%s.%s_\n", alias, name);

									if (strcmp_Upper(math_word, "AVG") == 0 ||
										strcmp_Upper(math_word, "COUNT") == 0 ||
										strcmp_Upper(math_word, "FIRST") == 0 ||
										strcmp_Upper(math_word, "LAST") == 0 ||
										strcmp_Upper(math_word, "MIN") == 0 ||
										strcmp_Upper(math_word, "MAX") == 0 ||
										strcmp_Upper(math_word, "MEDIAN") == 0 ||
										strcmp_Upper(math_word, "SUM") == 0)
									{
										struct func_node* temp_new_func = NULL;

										char* first_math_word = upper(math_word, NULL, malloced_head, the_debug);
										if (first_math_word == NULL)
										{
											if (the_debug == YES_DEBUG)
												printf("	ERROR in parseCaseNode() at line %d in %s\n", __LINE__, __FILE__);
											return RETURN_ERROR;
										}

										getNextWord(case_when_str, math_word, &math_index);

										if (parseFuncNode(&temp_new_func, NULL, &first_math_word, case_when_str, math_word, &math_index, select_node, false
														  ,malloced_head, the_debug) != RETURN_GOOD)
										{
											if (the_debug == YES_DEBUG)
												printf("	ERROR in parseCaseNode() at line %d in %s\n", __LINE__, __FILE__);
											return RETURN_ERROR;
										}

										if (addListNodePtr(&(*col_case_node)->case_then_value_head, &(*col_case_node)->case_then_value_tail, temp_new_func, PTR_TYPE_FUNC_NODE, ADDLISTNODE_TAIL
														  ,NULL, malloced_head, the_debug) != 0)
										{
											if (the_debug == YES_DEBUG)
												printf("	ERROR in parseCaseNode() at line %d in %s\n", __LINE__, __FILE__);
											return RETURN_ERROR;
										}

										new_ptr_type = PTR_TYPE_FUNC_NODE;
									}
									else 
									{
										if (addListNodePtr(&(*col_case_node)->case_then_value_head, &(*col_case_node)->case_then_value_tail, NULL, -1, ADDLISTNODE_TAIL
														  ,NULL, malloced_head, the_debug) != 0)
										{
											if (the_debug == YES_DEBUG)
												printf("	ERROR in parseCaseNode() at line %d in %s\n", __LINE__, __FILE__);
											return RETURN_ERROR;
										}

										if (getColInSelectNodeFromName((void*) (*col_case_node)->case_then_value_tail, PTR_TYPE_LIST_NODE_PTR, -1, 1, select_node, alias, name
																	  ,malloced_head, the_debug) != RETURN_GOOD)
										{
											printf("A column was declared which could not be found. Failing word: ""%s%s%s""\n", alias == NULL ? "" : alias, alias == NULL ? "" : ".", name);
											errorTeardown(NULL, malloced_head, the_debug);
											return RETURN_ERROR;
										}

										new_ptr_type = PTR_TYPE_COL_IN_SELECT_NODE;
									}

									myFree((void**) &name, NULL, malloced_head, the_debug);
									if (alias != NULL)
										myFree((void**) &alias, NULL, malloced_head, the_debug);
								// END Char but column name, so get column
							}
						}

						if (the_debug == YES_DEBUG)
							printf("new_ptr_type = %d\n", new_ptr_type);

						if ((*col_case_node)->case_then_value_tail != NULL && (((*col_case_node)->case_then_value_tail->ptr_type != PTR_TYPE_MATH_NODE && (*col_case_node)->case_then_value_tail->ptr_type != new_ptr_type)
																			|| ((*col_case_node)->case_then_value_tail->ptr_type == PTR_TYPE_MATH_NODE && new_ptr_type == PTR_TYPE_CHAR)))
						{
							if (the_debug == YES_DEBUG)
								printf("	ERROR in parseCaseNode() at line %d in %s\n", __LINE__, __FILE__);
							printf("Failing word: \"%s\"\n", math_word);
							errorTeardown(NULL, malloced_head, the_debug);
							return RETURN_ERROR;
						}

						if (new_ptr_type != PTR_TYPE_COL_IN_SELECT_NODE && 
							addListNodePtr(&(*col_case_node)->case_then_value_head, &(*col_case_node)->case_then_value_tail, temp, new_ptr_type, ADDLISTNODE_TAIL
										  ,NULL, malloced_head, the_debug) != 0)
						{
							if (the_debug == YES_DEBUG)
								printf("	ERROR in parseCaseNode() at line %d in %s\n", __LINE__, __FILE__);
							return RETURN_ERROR;
						}
					// END Parse single word for then value
				}

				case_when_str[0] = 0;

				if (strcmp_Upper(word, "WHEN") == 0)
					strcat(case_when_str, "when ");


				myFree((void**) &math_word, NULL, malloced_head, the_debug);
			// END Add to case_then_value_tail

			if (strcmp_Upper(word, "END") == 0)
			{
				if (new_col_name != NULL)
				{
					// START Parse new column name
						getNextWord(input, word, &index);

						if (word[0] == '+' || word[0] == '-' || word[0] == '*' || word[0] == '/' || word[0] == '^')
						{
							if (the_debug == YES_DEBUG)
								printf("Found math after parsed case\n");
							struct math_node* new_math_node = myMalloc(sizeof(struct math_node), NULL, malloced_head, the_debug);
							if (new_math_node == NULL)
							{
								if (the_debug == YES_DEBUG)
									printf("	ERROR in parseCaseNode() at line %d in %s\n", __LINE__, __FILE__);
								return RETURN_ERROR;
							}

							// START Determine datatype of case_node
								if ((*col_case_node)->case_then_value_head->ptr_type == PTR_TYPE_MATH_NODE)
									(*col_case_node)->result_type = ((struct math_node*) (*col_case_node)->case_then_value_head->ptr_value)->result_type;
								else if ((*col_case_node)->case_then_value_head->ptr_type == PTR_TYPE_FUNC_NODE)
									(*col_case_node)->result_type = ((struct func_node*) (*col_case_node)->case_then_value_head->ptr_value)->result_type;
								else
								{
									struct ListNodePtr* cur_then = (*col_case_node)->case_then_value_head;
									while (cur_then != NULL)
									{
										if (cur_then->ptr_type > 0)
										{
											(*col_case_node)->result_type = cur_then->ptr_type;
											break;
										}
										cur_then = cur_then->next;
									}
									
									if (cur_then == NULL)
										(*col_case_node)->result_type = -1;
								}
							// END Determine datatype of case_node

							new_math_node->ptr_one = *col_case_node;
							new_math_node->ptr_one_type = PTR_TYPE_CASE_NODE;

							new_math_node->ptr_two = NULL;
							new_math_node->ptr_two_type = -1;

							new_math_node->parent = NULL;
							new_math_node->result_type = -1;

							char* temp_ptr = NULL;

							if (parseMathNode(&new_math_node, new_col_name, &temp_ptr, input, word, index, select_node
											 ,malloced_head, the_debug) != RETURN_GOOD)
							{
								if (the_debug == YES_DEBUG)
									printf("	ERROR in parseCaseNode() at line %d in %s\n", __LINE__, __FILE__);
								return RETURN_ERROR;
							}

							*col_case_node = (struct case_node*) new_math_node;
							*return_ptr_type = PTR_TYPE_MATH_NODE;

							myFree((void**) &case_when_str, NULL, malloced_head, the_debug);

							return RETURN_GOOD;
						}
						else
						{
							if (parseNewColumnName(new_col_name, input, &word, &index, malloced_head, the_debug) != RETURN_GOOD)
							{
								if (the_debug == YES_DEBUG)
									printf("	ERROR in parseCaseNode() at line %d in %s\n", __LINE__, __FILE__);
								return RETURN_ERROR;
							}

							if (the_debug == YES_DEBUG)
								printf("new_col_name = _%s_\n", *new_col_name);
						}
					// END Parse new column name
				}
			}

			if (!strContainsWordUpper(input, "ELSE"))
			{
				if (the_debug == YES_DEBUG)
					printf("Adding NULL to case_when_tail and case_then_value_tail cuz no ELSE\n");

				// START Add to null ptr case_when_tail
					if (addListNodePtr(&(*col_case_node)->case_when_head, &(*col_case_node)->case_when_tail, NULL, -1, ADDLISTNODE_TAIL
									  ,NULL, malloced_head, the_debug) != 0)
					{
						if (the_debug == YES_DEBUG)
							printf("	ERROR in parseCaseNode() at line %d in %s\n", __LINE__, __FILE__);
						return RETURN_ERROR;
					}
				// END Add to null ptr case_when_tail

				// START Add to null ptr case_then_value_tail
					if (addListNodePtr(&(*col_case_node)->case_then_value_head, &(*col_case_node)->case_then_value_tail, NULL, -1, ADDLISTNODE_TAIL
									  ,NULL, malloced_head, the_debug) != 0)
					{
						if (the_debug == YES_DEBUG)
							printf("	ERROR in parseCaseNode() at line %d in %s\n", __LINE__, __FILE__);
						return RETURN_ERROR;
					}
				// END Add to null ptr case_then_value_tail
			}
		}
		else if (strcmp_Upper(word, "THEN") == 0)
		{
			// START Add to case_when_tail
				if (the_debug == YES_DEBUG)
					printf("   parsing when clause = _%s_\n", case_when_str);

				struct where_clause_node* when_head = NULL;

				if (parseWhereClause(case_when_str, &when_head, select_node, NULL, "when", malloced_head, the_debug) != RETURN_GOOD)
				{
					return RETURN_ERROR;
				}

			
				if (addListNodePtr(&(*col_case_node)->case_when_head, &(*col_case_node)->case_when_tail, when_head, PTR_TYPE_WHERE_CLAUSE_NODE, ADDLISTNODE_TAIL
								  ,NULL, malloced_head, the_debug) != 0)
				{
					if (the_debug == YES_DEBUG)
						printf("	ERROR in parseCaseNode() at line %d in %s\n", __LINE__, __FILE__);
					return RETURN_ERROR;
				}


				case_when_str[0] = 0;
			// END Add to case_when_tail
		}
		else
		{
			// START Concat to case_when_str
				strcat(case_when_str, word);
				if (input[index] == '.')
				{
					if (case_when_str[strLength(case_when_str)-1] == ' ')
						case_when_str[strLength(case_when_str)-1] = '.';
					else
						strcat(case_when_str, ".");
					index++;
				}
				else
					strcat(case_when_str, " ");
			// START Concat to case_when_str
		}
	}

	myFree((void**) &case_when_str, NULL, malloced_head, the_debug);

	// START Determine datatype
		if ((*col_case_node)->case_then_value_head->ptr_type == PTR_TYPE_MATH_NODE)
			(*col_case_node)->result_type = ((struct math_node*) (*col_case_node)->case_then_value_head->ptr_value)->result_type;
		else if ((*col_case_node)->case_then_value_head->ptr_type == PTR_TYPE_FUNC_NODE)
			(*col_case_node)->result_type = ((struct func_node*) (*col_case_node)->case_then_value_head->ptr_value)->result_type;
		else
		{
			struct ListNodePtr* cur_then = (*col_case_node)->case_then_value_head;
			while (cur_then != NULL)
			{
				if (cur_then->ptr_type > 0)
				{
					(*col_case_node)->result_type = cur_then->ptr_type;
					break;
				}
				cur_then = cur_then->next;
			}
			
			if (cur_then == NULL)
				(*col_case_node)->result_type = -1;
		}
	// END Determine datatype

	return RETURN_GOOD;
}

/*	RETURNS:
 *  RETURN_ERROR if error
 *	RETURN_GOOD if good
 *
 *	WRITES TO:
 *  the_col
 */
int assignGroupByColToAnyFunc(struct col_in_select_node* the_col, struct select_node* the_select_node, char* name, char* alias
							 ,struct malloced_node** malloced_head, int the_debug)
{
	if (the_col->func_node != NULL && the_col->func_node->which_func != FUNC_RANK)
	{
		addListNodePtr(&the_col->func_node->group_by_cols_head, &the_col->func_node->group_by_cols_tail
					  ,NULL, -1, ADDLISTNODE_TAIL
	  				  ,NULL, malloced_head, the_debug);

		// Is aggregate function, add to group_by_cols_head
		if (getColInSelectNodeFromName((void*) the_col->func_node->group_by_cols_tail, PTR_TYPE_LIST_NODE_PTR, -1, 1, the_select_node, alias, name, malloced_head, the_debug) != RETURN_GOOD)
		{
			printf("A column was declared which could not be found. Failing word: ""%s%s%s""\n", alias == NULL ? "" : alias, alias == NULL ? "" : ".", name);
			errorTeardown(NULL, malloced_head, the_debug);
			return RETURN_ERROR;
		}
	}
	else if (the_col->math_node != NULL)
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
			int traversd = traverseTreeNode((void**) &the_col->math_node, PTR_TYPE_MATH_NODE, &ptr, &ptr_type, (void**) &cur_mirror
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
				addListNodePtr(&((struct func_node*) ptr)->group_by_cols_head, &((struct func_node*) ptr)->group_by_cols_tail
							  ,NULL, -1, ADDLISTNODE_TAIL
	  						  ,NULL, malloced_head, the_debug);

				// Is aggregate function, add to group_by_cols_head
				if (getColInSelectNodeFromName((void*) ((struct func_node*) ptr)->group_by_cols_tail, PTR_TYPE_LIST_NODE_PTR, -1, 1, the_select_node, alias, name, malloced_head, the_debug) != RETURN_GOOD)
				{
					printf("A column was declared which could not be found. Failing word: ""%s%s%s""\n", alias == NULL ? "" : alias, alias == NULL ? "" : ".", name);
					errorTeardown(NULL, malloced_head, the_debug);
					return RETURN_ERROR;
				}
			}
		}
	}
	else if (the_col->case_node != NULL)
	{
		struct ListNodePtr* cur_then = the_col->case_node->case_then_value_head;
		while (cur_then != NULL)
		{
			if (cur_then->ptr_type == PTR_TYPE_FUNC_NODE)
			{
				addListNodePtr(&((struct func_node*) cur_then->ptr_value)->group_by_cols_head, &((struct func_node*) cur_then->ptr_value)->group_by_cols_tail
							  ,NULL, -1, ADDLISTNODE_TAIL
	  						  ,NULL, malloced_head, the_debug);

				// Is aggregate function, add to group_by_cols_head
				if (getColInSelectNodeFromName((void*) ((struct func_node*) cur_then->ptr_value)->group_by_cols_tail, PTR_TYPE_LIST_NODE_PTR, -1, 1, the_select_node, alias, name, malloced_head, the_debug) != RETURN_GOOD)
				{
					printf("A column was declared which could not be found. Failing word: ""%s%s%s""\n", alias == NULL ? "" : alias, alias == NULL ? "" : ".", name);
					errorTeardown(NULL, malloced_head, the_debug);
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
						addListNodePtr(&((struct func_node*) ptr)->group_by_cols_head, &((struct func_node*) ptr)->group_by_cols_tail
									  ,NULL, -1, ADDLISTNODE_TAIL
			  						  ,NULL, malloced_head, the_debug);

						// Is aggregate function, add to group_by_cols_head
						if (getColInSelectNodeFromName((void*) ((struct func_node*) ptr)->group_by_cols_tail, PTR_TYPE_LIST_NODE_PTR, -1, 1, the_select_node, alias, name, malloced_head, the_debug) != RETURN_GOOD)
						{
							printf("A column was declared which could not be found. Failing word: ""%s%s%s""\n", alias == NULL ? "" : alias, alias == NULL ? "" : ".", name);
							errorTeardown(NULL, malloced_head, the_debug);
							return RETURN_ERROR;
						}
					}
				}
			}

			cur_then = cur_then->next;
		}
	}

	return RETURN_GOOD;
}

/*	RETURNS:
 *  RETURN_ERROR if error
 *	RETURN_GOOD if good
 *
 *	WRITES TO:
 *  select_node
 */
int parseSelect(char* input, struct select_node** select_node, struct ListNodePtr* with_sub_select_list, bool in_with
			   ,struct malloced_node** malloced_head, int the_debug)
{
	if (*select_node == NULL)
	{
		*select_node = (struct select_node*) myMalloc(sizeof(struct select_node), NULL, malloced_head, the_debug);
		if (*select_node == NULL)
		{
			if (the_debug == YES_DEBUG)
				printf("	ERROR in parseSelect() at line %d in %s\n", __LINE__, __FILE__);
			return RETURN_ERROR;
		}

		(*select_node)->select_node_alias = NULL;
		(*select_node)->distinct = false;

		(*select_node)->columns_arr_size = 0;
		(*select_node)->columns_arr = NULL;

		(*select_node)->where_head = NULL;
		(*select_node)->join_head = NULL;
		(*select_node)->having_head = NULL;

		(*select_node)->prev = NULL;
		(*select_node)->next = NULL;

		(*select_node)->order_by = NULL;
	}

	struct ListNodePtr* with_sub_select_head = with_sub_select_list;
	struct ListNodePtr* with_sub_select_tail = NULL;

	// START Init word variable
		char* word = (char*) myMalloc(sizeof(char) * 200, NULL, malloced_head, the_debug);
		if (word == NULL)
		{
			if (the_debug == YES_DEBUG)
				printf("	ERROR in parseSelect() at line %d in %s\n", __LINE__, __FILE__);
			*select_node = NULL;
			return RETURN_ERROR;
		}
		word[0] = 0;
		int index = 0;
	// END Init word variable

	// START See if first word is SELECT or WITH
		getNextWord(input, word, &index);

		if (strcmp_Upper(word, "SELECT") != 0)
		{
			if (strcmp_Upper(word, "WITH") == 0)
			{
				if (in_with)
				{
					if (the_debug == YES_DEBUG)
						printf("	ERROR in parseSelect() at line %d in %s\n", __LINE__, __FILE__);
					printf("The word \"WITH\" was declared inside an already declared WITH statement. Please erase it. Failing word: \"%s\"\n", word);
					errorTeardown(NULL, malloced_head, the_debug);
					*select_node = NULL;
					return RETURN_ERROR;
				}

				char* sub_select = (char*) myMalloc(sizeof(char) * 1000, NULL, malloced_head, the_debug);
				if (sub_select == NULL)
				{
					if (the_debug == YES_DEBUG)
						printf("	ERROR in parseSelect() at line %d in %s\n", __LINE__, __FILE__);
					return RETURN_ERROR;
				}

				strcpy(sub_select, "");

				while (strcmp_Upper(word, "WITH") == 0 || word[0] == ',')
				{
					getNextWord(input, word, &index);

					char* sub_select_alias = upper(word, NULL, malloced_head, the_debug);
					if (sub_select_alias == NULL)
					{
						if (the_debug == YES_DEBUG)
							printf("	ERROR in parseSelect() at line %d in %s\n", __LINE__, __FILE__);
						*select_node = NULL;
						return RETURN_ERROR;
					}

					getNextWord(input, word, &index);

					if (strcmp_Upper(word, "AS") != 0)
					{
						if (the_debug == YES_DEBUG)
							printf("	ERROR in parseSelect() at line %d in %s\n", __LINE__, __FILE__);
						printf("The word \"AS\" was expected after the word \"WORD\". Failing word: \"%s\"\n", word);
						errorTeardown(NULL, malloced_head, the_debug);
						*select_node = NULL;
						return RETURN_ERROR;
					}

					getNextWord(input, word, &index);

					if (word[0] != '(')
					{
						if (the_debug == YES_DEBUG)
							printf("	ERROR in parseSelect() at line %d in %s\n", __LINE__, __FILE__);
						printf("The word \"(\" was expected after the word \"AS\". Failing word: \"%s\"\n", word);
						errorTeardown(NULL, malloced_head, the_debug);
						*select_node = NULL;
						return RETURN_ERROR;
					}


					getSubSelect(input, &word, &index, &sub_select);

					if (the_debug == YES_DEBUG)
					{
						printf("sub_select = _%s_\n", sub_select);
						printf("sub_select_alias = _%s_\n", sub_select_alias);
						printf("word after sub_select = _%s_\n", word);
					}

					if (word[0] == ')')
					{
						getNextWord(input, word, &index);
					}
					else
					{
						if (the_debug == YES_DEBUG)
							printf("	ERROR in parseSelect() at line %d in %s\n", __LINE__, __FILE__);
						printf("The word \")\" was expected after the WITH statment was declared. Failing word: \"%s\"\n", word);
						errorTeardown(NULL, malloced_head, the_debug);
						*select_node = NULL;
						return RETURN_ERROR;
					}

					struct select_node* new_sub_select = NULL;
					

					if (parseSelect(sub_select, &new_sub_select, with_sub_select_head, true, malloced_head, the_debug) != RETURN_GOOD)
					{
						if (the_debug == YES_DEBUG)
							printf("	ERROR in parseSelect() at line %d in %s\n", __LINE__, __FILE__);
						*select_node = NULL;
						return RETURN_ERROR;
					}


					sub_select[0] = 0;


					while (new_sub_select->next != NULL)
						new_sub_select = new_sub_select->next;


					new_sub_select->select_node_alias = sub_select_alias;

					if (the_debug == YES_DEBUG)
					{
						printf("new_sub_select col_ptr_type[0] = %d\n", new_sub_select->columns_arr[0]->col_ptr_type);
						printf("new_sub_select->prev col_ptr_type[0] = %d\n", new_sub_select->prev->columns_arr[0]->col_ptr_type);

						printf("new_sub_select alias = _%s_\n", new_sub_select->select_node_alias);
						printf("new_sub_select->prev alias = _%s_\n", new_sub_select->prev->select_node_alias);
					}

					addListNodePtr(&with_sub_select_head, &with_sub_select_tail, new_sub_select, -1, ADDLISTNODE_TAIL
								  ,NULL, malloced_head, the_debug);
				}

				myFree((void**) &sub_select, NULL, malloced_head, the_debug);

				if (strcmp_Upper(word, "SELECT") != 0)
				{
					if (the_debug == YES_DEBUG)
						printf("	ERROR in parseSelect() at line %d in %s\n", __LINE__, __FILE__);
					printf("The first word of a select command was not SELECT. Failing word: \"%s\"\n", word);
					errorTeardown(NULL, malloced_head, the_debug);
					*select_node = NULL;
					return RETURN_ERROR;
				}
			}	
			else
			{
				if (the_debug == YES_DEBUG)
					printf("	ERROR in parseSelect() at line %d in %s\n", __LINE__, __FILE__);
				printf("The first word of a select command was not SELECT. Failing word: \"%s\"\n", word);
				errorTeardown(NULL, malloced_head, the_debug);
				*select_node = NULL;
				return RETURN_ERROR;
			}
		}
	// END See if first word is SELECT or WITH

	// START See if next word is DISTINCT
		getNextWord(input, word, &index);

		int compared = strcmp_Upper(word, "DISTINCT");

		if (compared == 0)
		{
			(*select_node)->distinct = true;
		}
		else if (compared != -2)
		{
			index -= strLength(word);
		}
	// END See if next word is DISTINCT

	// START Put columns in substring to read after getting tables, joins
		char* cols_str = (char*) myMalloc(sizeof(char) * 10000, NULL, malloced_head, the_debug);
		if (cols_str == NULL)
		{
			if (the_debug == YES_DEBUG)
				printf("	ERROR in parseSelect() at line %d in %s\n", __LINE__, __FILE__);
			return RETURN_ERROR;
		}

		strcpy(cols_str, "");

		while (getNextWord(input, word, &index) == 0 && (compared = strcmp_Upper(word, "FROM")) != 0)
		{
			if (the_debug == YES_DEBUG)
				printf("word = _%s_\n", word);

			strcat(cols_str, word);
			if (input[index] == '.')
			{
				if (cols_str[strLength(cols_str)-1] == ' ')
					cols_str[strLength(cols_str)-1] = '.';
				else
					strcat(cols_str, ".");
				index++;
			}
			else
				strcat(cols_str, " ");
		}

		if (the_debug == YES_DEBUG)
			printf("cols_str = _%s_\n", cols_str);

		int size_result = 0;
		char** cols_strs_arr = strSplitV2(cols_str, ',', &size_result, NULL, malloced_head, the_debug);
		if (cols_strs_arr == NULL)
		{
			if (the_debug == YES_DEBUG)
				printf("	ERROR in parseSelect() at line %d in %s\n", __LINE__, __FILE__);
			*select_node = NULL;
			return RETURN_ERROR;
		}

		myFree((void**) &cols_str, NULL, malloced_head, the_debug);
	// END Put columns in substring to read after getting tables, joins


	// START Get table, alias of table, joins, and subqueries
		struct select_node* select_tail = NULL;

		struct join_node* join_tail = NULL;

		char* on_clause = NULL;

		char* sub_select = NULL;

		while (getNextWord(input, word, &index) == 0 && word[0] != ';')
		{
			if (the_debug == YES_DEBUG)
				printf("word in loop = _%s_\n", word);

			if (strcmp_Upper(word, "WHERE") == 0 || strcmp_Upper(word, "GROUP") == 0 || strcmp_Upper(word, "ORDER") == 0)
			{
				// START Parse where clause and break
					if (on_clause != NULL)
					{
						if (parseOnClauseOfSelect(&join_tail, &on_clause, select_node, malloced_head, the_debug) != RETURN_GOOD)
						{
							if (the_debug == YES_DEBUG)
								printf("	ERROR in parseSelect() at line %d in %s\n", __LINE__, __FILE__);
							*select_node = NULL;
							return RETURN_ERROR;
						}
					}

					break;
				// END Parse where clause and break
			}
			else if (word[0] == '(')
			{
				// START Setup sub select and recursively call this function
					char* sub_select = (char*) myMalloc(sizeof(char) * 10000, NULL, malloced_head, the_debug);
					if (sub_select == NULL)
					{
						if (the_debug == YES_DEBUG)
							printf("	ERROR in parseSelect() at line %d in %s\n", __LINE__, __FILE__);
						*select_node = NULL;
						return RETURN_ERROR;
					}

					strcpy(sub_select, "");

					getSubSelect(input, &word, &index, &sub_select);

					if (the_debug == YES_DEBUG)
						printf("sub_select = _%s_\n", sub_select);


					struct select_node* new_sub_select = NULL;
					

					if (the_debug == YES_DEBUG)
						printf("	Recursively calling parseSelect for subselect\n");
					if (parseSelect(sub_select, &new_sub_select, with_sub_select_head, true, malloced_head, the_debug) != RETURN_GOOD)
					{
						if (the_debug == YES_DEBUG)
							printf("	ERROR in parseSelect() at line %d in %s\n", __LINE__, __FILE__);
						*select_node = NULL;
						return RETURN_ERROR;
					}
					if (the_debug == YES_DEBUG)
						printf("	Back from recursively calling parseSelect for subselect\n");


					while (new_sub_select->next != NULL)
						new_sub_select = new_sub_select->next;


					if (join_tail == NULL)
					{
						// Need to add to select_tail->prev
						if (the_debug == YES_DEBUG)
							printf("Added new_sub_select to select_tail->prev\n");

						if (select_tail == NULL)
							select_tail = *select_node;

						select_tail->prev = new_sub_select;
						new_sub_select->next = select_tail;

						select_tail = select_tail->prev;
					}
					else if (join_tail->select_joined == NULL)
					{
						// Need to add to join_tail->select_joined
						if (the_debug == YES_DEBUG)
							printf("Added new_sub_select to join_tail->select_joined\n");

						join_tail->select_joined = new_sub_select;
					}

					myFree((void**) &sub_select, NULL, malloced_head, the_debug);
				// END Setup sub select and recursively call this function
			}
			else if (strcmp_Upper(word, "INNER") == 0 
					|| strcmp_Upper(word, "OUTER") == 0 
					|| strcmp_Upper(word, "LEFT") == 0 
					|| strcmp_Upper(word, "RIGHT") == 0
					|| strcmp_Upper(word, "JOIN") == 0
					|| strcmp_Upper(word, "CROSS") == 0)
			{
				// START Setup join node
					if (on_clause != NULL)
					{
						if (parseOnClauseOfSelect(&join_tail, &on_clause, select_node, malloced_head, the_debug) != RETURN_GOOD)
						{
							if (the_debug == YES_DEBUG)
								printf("	ERROR in parseSelect() at line %d in %s\n", __LINE__, __FILE__);
							*select_node = NULL;
							return RETURN_ERROR;
						}
					}

					// START Add another join_node to select_node
						if ((*select_node)->join_head == NULL)
						{
							(*select_node)->join_head = (struct join_node*) myMalloc(sizeof(struct join_node), NULL, malloced_head, the_debug);
							if ((*select_node)->join_head == NULL)
							{
								if (the_debug == YES_DEBUG)
									printf("	ERROR in parseSelect() at line %d in %s\n", __LINE__, __FILE__);
								*select_node = NULL;
								return RETURN_ERROR;
							}

							join_tail = (*select_node)->join_head;

							join_tail->prev = NULL;
							join_tail->next = NULL;
						}
						else
						{
							join_tail->next = (struct join_node*) myMalloc(sizeof(struct join_node), NULL, malloced_head, the_debug);
							if (join_tail->next == NULL)
							{
								if (the_debug == YES_DEBUG)
									printf("	ERROR in parseSelect() at line %d in %s\n", __LINE__, __FILE__);
								*select_node = NULL;
								return RETURN_ERROR;
							}

							join_tail->next->prev = join_tail;
							join_tail->next->next = NULL;

							join_tail = join_tail->next;
						}

						join_tail->select_joined = NULL;

						join_tail->on_clause_head = NULL;

						if (strcmp_Upper(word, "INNER") == 0 || strcmp_Upper(word, "JOIN") == 0)
							join_tail->join_type = JOIN_INNER;
						else if (strcmp_Upper(word, "OUTER") == 0)
							join_tail->join_type = JOIN_OUTER;
						else if (strcmp_Upper(word, "LEFT") == 0)
							join_tail->join_type = JOIN_LEFT;
						else if (strcmp_Upper(word, "RIGHT") == 0)
							join_tail->join_type = JOIN_RIGHT;
						else if (strcmp_Upper(word, "CROSS") == 0)
							join_tail->join_type = JOIN_CROSS;
					// END Add another join_node to select_node


					// START Check to ensure that next word is "join"
						if (strcmp_Upper(word, "JOIN") != 0)
						{
							getNextWord(input, word, &index);
							if (the_debug == YES_DEBUG)
								printf("word in loop = _%s_\n", word);

							if (strcmp_Upper(word, "JOIN") != 0)
							{
								if (the_debug == YES_DEBUG)
									printf("	ERROR in parseSelect() at line %d in %s\n", __LINE__, __FILE__);
								printf("The word \"JOIN\" was expected after one of the following words \"INNER\", \"OUTER\", \"LEFT\", \"RIGHT\", or \"CROSS\". Failing word: \"%s\"\n", word);
								errorTeardown(NULL, malloced_head, the_debug);
								*select_node = NULL;
								return RETURN_ERROR;
							}
						}
					// END Check to ensure that next word is "join"
				// END Setup join node
			}
			else if (strcmp_Upper(word, "ON") == 0 || on_clause != NULL)
			{
				// START Concat word to on clause
					if (on_clause == NULL)
					{
						on_clause = (char*) myMalloc(sizeof(char) * 1000, NULL, malloced_head, the_debug);
						if (on_clause == NULL)
						{
							if (the_debug == YES_DEBUG)
								printf("	ERROR in parseSelect() at line %d in %s\n", __LINE__, __FILE__);
							return RETURN_ERROR;
						}

						strcpy(on_clause, "");
					}

					strcat(on_clause, word);
					if (input[index] == '.')
					{
						strcat(on_clause, ".");
						index++;
					}
					else
						strcat(on_clause, " ");

					//printf("Gathering the on clause: _%s_\n", on_clause);
				// END Concat word to on clause
			}
			else if (select_tail != NULL && select_tail->select_node_alias == NULL)
			{
				if (strcmp_Upper(word, "AS") == 0)
					getNextWord(input, word, &index);

				if (the_debug == YES_DEBUG)
					printf("Adding alias to select_tail\n");
				select_tail->select_node_alias = upper(word, NULL, malloced_head, the_debug);
				if (select_tail->select_node_alias == NULL)
				{
					if (the_debug == YES_DEBUG)
						printf("	ERROR in parseSelect() at line %d in %s\n", __LINE__, __FILE__);
					*select_node = NULL;
					return RETURN_ERROR;
				}
			}
			else if (join_tail != NULL && join_tail->select_joined != NULL && join_tail->select_joined->select_node_alias == NULL)
			{
				if (strcmp_Upper(word, "AS") == 0)
					getNextWord(input, word, &index);

				if (the_debug == YES_DEBUG)
					printf("Adding alias to join_tail\n");
				join_tail->select_joined->select_node_alias = upper(word, NULL, malloced_head, the_debug);
				if (join_tail->select_joined->select_node_alias == NULL)
				{
					if (the_debug == YES_DEBUG)
						printf("	ERROR in parseSelect() at line %d in %s\n", __LINE__, __FILE__);
					*select_node = NULL;
					return RETURN_ERROR;
				}
			}
			else if (join_tail != NULL && join_tail->select_joined == NULL)
			{
				// START Add joined table at lowest level
					if (the_debug == YES_DEBUG)
						printf("Adding JOINED table at lowest level\n");

					struct table_info* the_table = getTableFromName(word, malloced_head, the_debug);
					if (the_table == NULL && with_sub_select_head == NULL)
					{
						if (the_debug == YES_DEBUG)
							printf("	ERROR in parseSelect() at line %d in %s\n", __LINE__, __FILE__);
						printf("A table was declared which could not be found. Failing word: \"%s\"\n", word);
						errorTeardown(NULL, malloced_head, the_debug);
						*select_node = NULL;
						return RETURN_ERROR;
					}
					else if (the_table == NULL && with_sub_select_head != NULL)
					{
						struct ListNodePtr* cur_with = with_sub_select_head;
						while (cur_with != NULL)
						{
							if (strcmp_Upper(word, ((struct select_node*) cur_with->ptr_value)->select_node_alias) == 0)
							{
								join_tail->select_joined = (struct select_node*) cur_with->ptr_value;

								cur_with->ptr_type = PTR_TYPE_SELECT_NODE;

								break;
							}

							cur_with = cur_with->next;
						}

						if (cur_with == NULL)
						{
							if (the_debug == YES_DEBUG)
								printf("	ERROR in parseSelect() at line %d in %s\n", __LINE__, __FILE__);
							printf("A table was declared which could not be found. Failing word: \"%s\"\n", word);
							errorTeardown(NULL, malloced_head, the_debug);
							*select_node = NULL;
							return RETURN_ERROR;
						}
					}
					else if (the_table != NULL)
					{
						join_tail->select_joined = (struct select_node*) myMalloc(sizeof(struct select_node), NULL, malloced_head, the_debug);
						if (join_tail->select_joined == NULL)
						{
							if (the_debug == YES_DEBUG)
								printf("	ERROR in parseSelect() at line %d in %s\n", __LINE__, __FILE__);
							return RETURN_ERROR;
						}

						join_tail->select_joined->select_node_alias = NULL;
						join_tail->select_joined->distinct = false;

						join_tail->select_joined->columns_arr_size = the_table->num_cols;
						join_tail->select_joined->columns_arr = (struct col_in_select_node**) myMalloc(sizeof(struct col_in_select_node*) * the_table->num_cols, NULL, malloced_head, the_debug);
						if (join_tail->select_joined->columns_arr == NULL)
						{
							if (the_debug == YES_DEBUG)
								printf("	ERROR in parseSelect() at line %d in %s\n", __LINE__, __FILE__);
							return RETURN_ERROR;
						}

						struct table_cols_info* cur_col = the_table->table_cols_head;
						for (int i=0; i<the_table->num_cols; i++)
						{
							join_tail->select_joined->columns_arr[i] = (struct col_in_select_node*) myMalloc(sizeof(struct col_in_select_node), NULL, malloced_head, the_debug);
							if (join_tail->select_joined->columns_arr[i] == NULL)
							{
								if (the_debug == YES_DEBUG)
									printf("	ERROR in parseSelect() at line %d in %s\n", __LINE__, __FILE__);
								return RETURN_ERROR;
							}
							
							join_tail->select_joined->columns_arr[i]->table_ptr = the_table;
							join_tail->select_joined->columns_arr[i]->table_ptr_type = PTR_TYPE_TABLE_INFO;

							join_tail->select_joined->columns_arr[i]->col_ptr = cur_col;
							join_tail->select_joined->columns_arr[i]->col_ptr_type = PTR_TYPE_TABLE_COLS_INFO;

							join_tail->select_joined->columns_arr[i]->new_name = NULL;
							join_tail->select_joined->columns_arr[i]->func_node = NULL;
							join_tail->select_joined->columns_arr[i]->math_node = NULL;
							join_tail->select_joined->columns_arr[i]->case_node = NULL;

							join_tail->select_joined->columns_arr[i]->join_matching_rows_head = NULL;
							join_tail->select_joined->columns_arr[i]->join_matching_rows_tail = NULL;

							join_tail->select_joined->columns_arr[i]->rows_data_type = cur_col->data_type;

							cur_col = cur_col->next;
						}

						join_tail->select_joined->where_head = NULL;
						join_tail->select_joined->join_head = NULL;
						join_tail->select_joined->having_head = NULL;

						join_tail->select_joined->next = NULL;
						join_tail->select_joined->prev = NULL;

						join_tail->select_joined->order_by = NULL;
					}
				// END Add joined table at lowest level
			}
			else
			{
				// START Add from table at lowest level
					if (the_debug == YES_DEBUG)
						printf("Adding table at lowest level\n");

					struct table_info* the_table = getTableFromName(word, malloced_head, the_debug);
					if (the_table == NULL && with_sub_select_head == NULL)
					{
						if (the_debug == YES_DEBUG)
							printf("	ERROR in parseSelect() at line %d in %s\n", __LINE__, __FILE__);
						printf("A table was declared which could not be found. Failing word: \"%s\"\n", word);
						errorTeardown(NULL, malloced_head, the_debug);
						*select_node = NULL;
						return RETURN_ERROR;
					}
					else if (the_table == NULL && with_sub_select_head != NULL)
					{
						struct ListNodePtr* cur_with = with_sub_select_head;
						while (cur_with != NULL)
						{
							if (strcmp_Upper(word, ((struct select_node*) cur_with->ptr_value)->select_node_alias) == 0)
							{
								if (select_tail == NULL)
									select_tail = *select_node;

								while (select_tail->prev != NULL)
									select_tail = select_tail->prev;

								select_tail->prev = (struct select_node*) cur_with->ptr_value;

								select_tail->prev->next = select_tail;
								select_tail->prev->prev = ((struct select_node*) cur_with->ptr_value)->prev;

								select_tail = select_tail->prev;

								cur_with->ptr_type = PTR_TYPE_SELECT_NODE;

								break;
							}

							cur_with = cur_with->next;
						}

						if (cur_with == NULL)
						{
							if (the_debug == YES_DEBUG)
								printf("	ERROR in parseSelect() at line %d in %s\n", __LINE__, __FILE__);
							printf("A table was declared which could not be found. Failing word: \"%s\"\n", word);
							errorTeardown(NULL, malloced_head, the_debug);
							*select_node = NULL;
							return RETURN_ERROR;
						}
					}
					else if (the_table != NULL)
					{
						if (select_tail == NULL)
							select_tail = *select_node;

						while (select_tail->prev != NULL)
							select_tail = select_tail->prev;

						select_tail->prev = (struct select_node*) myMalloc(sizeof(struct select_node), NULL, malloced_head, the_debug);
						if (select_tail->prev == NULL)
						{
							if (the_debug == YES_DEBUG)
								printf("	ERROR in parseSelect() at line %d in %s\n", __LINE__, __FILE__);
							return RETURN_ERROR;
						}

						select_tail->prev->select_node_alias = NULL;
						select_tail->prev->distinct = false;

						select_tail->prev->columns_arr_size = the_table->num_cols;
						select_tail->prev->columns_arr = (struct col_in_select_node**) myMalloc(sizeof(struct col_in_select_node*) * select_tail->prev->columns_arr_size, NULL, malloced_head, the_debug);
						if (select_tail->prev->columns_arr == NULL)
						{
							if (the_debug == YES_DEBUG)
								printf("	ERROR in parseSelect() at line %d in %s\n", __LINE__, __FILE__);
							return RETURN_ERROR;
						}
				
						struct table_cols_info* cur_col = the_table->table_cols_head;
						for (int i=0; i<select_tail->prev->columns_arr_size; i++)
						{
							select_tail->prev->columns_arr[i] = (struct col_in_select_node*) myMalloc(sizeof(struct col_in_select_node), NULL, malloced_head, the_debug);
							if (select_tail->prev->columns_arr[i] == NULL)
							{
								if (the_debug == YES_DEBUG)
									printf("	ERROR in parseSelect() at line %d in %s\n", __LINE__, __FILE__);
								return RETURN_ERROR;
							}
							
							select_tail->prev->columns_arr[i]->table_ptr = the_table;
							select_tail->prev->columns_arr[i]->table_ptr_type = PTR_TYPE_TABLE_INFO;

							select_tail->prev->columns_arr[i]->col_ptr = cur_col;
							select_tail->prev->columns_arr[i]->col_ptr_type = PTR_TYPE_TABLE_COLS_INFO;

							select_tail->prev->columns_arr[i]->new_name = NULL;
							select_tail->prev->columns_arr[i]->func_node = NULL;
							select_tail->prev->columns_arr[i]->math_node = NULL;
							select_tail->prev->columns_arr[i]->case_node = NULL;

							select_tail->prev->columns_arr[i]->join_matching_rows_head = NULL;
							select_tail->prev->columns_arr[i]->join_matching_rows_tail = NULL;

							select_tail->prev->columns_arr[i]->rows_data_type = cur_col->data_type;

							cur_col = cur_col->next;
						}

						select_tail->prev->where_head = NULL;
						select_tail->prev->join_head = NULL;
						select_tail->prev->having_head = NULL;

						select_tail->prev->next = select_tail;
						select_tail->prev->prev = NULL;

						select_tail->prev->order_by = NULL;

						select_tail = select_tail->prev;
					}
				// END Add from table at lowest level
			}
		}
		if (the_debug == YES_DEBUG)
			printf("Out of join loop\n");

		if (on_clause != NULL)
		{
			if (parseOnClauseOfSelect(&join_tail, &on_clause, select_node, malloced_head, the_debug) != RETURN_GOOD)
			{
				if (the_debug == YES_DEBUG)
					printf("	ERROR in parseSelect() at line %d in %s\n", __LINE__, __FILE__);
				*select_node = NULL;
				return RETURN_ERROR;
			}
		}


		if ((*select_node)->prev != NULL && (*select_node)->prev->prev != NULL && (*select_node)->prev->select_node_alias == NULL)
		{
			if (the_debug == YES_DEBUG)
				printf("	ERROR in parseSelect() at line %d in %s\n", __LINE__, __FILE__);
			printf("A sub select was created, but it was not named. Please give the sub select a name. Failing word: \"%s\"\n", word);
			errorTeardown(NULL, malloced_head, the_debug);
			*select_node = NULL;
			return RETURN_ERROR;
		}


		struct join_node* cur_join = (*select_node)->join_head;
		while (cur_join != NULL)
		{
			if (cur_join->select_joined->prev != NULL && cur_join->select_joined->select_node_alias == NULL)
			{
				if (the_debug == YES_DEBUG)
					printf("	ERROR in parseSelect() at line %d in %s\n", __LINE__, __FILE__);
				errorTeardown(NULL, malloced_head, the_debug);
				*select_node = NULL;
				return RETURN_ERROR;
			}

			cur_join = cur_join->next;
		}
	// END Get alias of table and/or joins, if exist


	/*// START Print select_node
		printf("State of *select_node:\n");
		struct select_node* cur_select = *select_node;
		while (cur_select != NULL)
		{
			printf("Found one select_node\n");

			if (cur_select->prev == NULL)
				break;
			else
				cur_select = cur_select->prev;
		}

		while (cur_select != NULL)
		{
			if (cur_select->select_node_alias != NULL)
				printf("	select_node_alias = _%s_\n", cur_select->select_node_alias);
			else
				printf("	select_node_alias not init yet\n");

			cur_select = cur_select->next;
		}
		printf("------------------\n");
	// END Print select_node*/


	// START Parse columns now that tables have been gotten
		(*select_node)->columns_arr_size = 0;

		struct ListNodePtr* except_cols_name_head = NULL;
		struct ListNodePtr* except_cols_name_tail = NULL;

		struct ListNodePtr* except_cols_alias_head = NULL;
		struct ListNodePtr* except_cols_alias_tail = NULL;

		for (int i=0; i<2; i++)
		{
			int index = 0;

			// START Init columns arrays
				if (i == 1)
				{
					(*select_node)->columns_arr = (struct col_in_select_node**) myMalloc(sizeof(struct col_in_select_node*) * ((*select_node)->columns_arr_size), NULL, malloced_head, the_debug);
					if ((*select_node)->columns_arr == NULL)
					{
						if (the_debug == YES_DEBUG)
							printf("	ERROR in parseSelect() at line %d in %s\n", __LINE__, __FILE__);
						return RETURN_ERROR;
					}

					for (int j=0; j<(*select_node)->columns_arr_size; j++)
					{
						(*select_node)->columns_arr[j] = (struct col_in_select_node*) myMalloc(sizeof(struct col_in_select_node), NULL, malloced_head, the_debug);
						if ((*select_node)->columns_arr[j] == NULL)
						{
							if (the_debug == YES_DEBUG)
								printf("	ERROR in parseSelect() at line %d in %s\n", __LINE__, __FILE__);
							return RETURN_ERROR;
						}

						(*select_node)->columns_arr[j]->table_ptr = NULL;
						(*select_node)->columns_arr[j]->table_ptr_type = -1;
						(*select_node)->columns_arr[j]->col_ptr = NULL;
						(*select_node)->columns_arr[j]->col_ptr_type = -1;
						(*select_node)->columns_arr[j]->new_name = NULL;
						(*select_node)->columns_arr[j]->func_node = NULL;
						(*select_node)->columns_arr[j]->math_node = NULL;
						(*select_node)->columns_arr[j]->case_node = NULL;
						(*select_node)->columns_arr[j]->join_matching_rows_head = NULL;
						(*select_node)->columns_arr[j]->join_matching_rows_tail = NULL;
					}

					if (the_debug == YES_DEBUG)
					{
						printf("-----------------\n");
						printf("	Init columns_arr with (*select_node)->columns_arr_size = %d\n", (*select_node)->columns_arr_size);
					}
				}
			// END Init columns arrays

			bool except = false;

			for (int arr_k=0; arr_k<size_result; arr_k++)
			{
				if ((*select_node)->prev == NULL)
				{
					if (the_debug == YES_DEBUG)
						printf("	ERROR in parseSelect() at line %d in %s\n", __LINE__, __FILE__);
					printf("Failing word: \"%s\"\n", word);
					errorTeardown(NULL, malloced_head, the_debug);
					*select_node = NULL;
					return RETURN_ERROR;
				}

				if (the_debug == YES_DEBUG)
					printf("cols_strs_arr[arr_k] = _%s_\n", cols_strs_arr[arr_k]);

				// START Parse column string
					int col_word_index = 0;
					char* col_word = (char*) myMalloc(sizeof(char) * 100, NULL, malloced_head, the_debug);
					if (col_word == NULL)
					{
						if (the_debug == YES_DEBUG)
							printf("	ERROR in parseSelect() at line %d in %s\n", __LINE__, __FILE__);
						return RETURN_ERROR;
					}

					char* alias = NULL;
					char* col_name = NULL;
					char* new_col_name = NULL;

					struct func_node* col_func_node = NULL;
					struct math_node* col_math_node = NULL;
					struct case_node* col_case_node = NULL;

					while (getNextWord(cols_strs_arr[arr_k], col_word, &col_word_index) == 0)
					{
						if (the_debug == YES_DEBUG)
							printf("word in col_word loop = _%s_\n", col_word);

						if (strcmp_Upper(col_word, "CASE") == 0)
						{
							// START Parse case statement
								if (i == 0)
								{
									col_case_node = (struct case_node*) myMalloc(sizeof(struct case_node), NULL, malloced_head, the_debug);
									if (col_case_node == NULL)
									{
										if (the_debug == YES_DEBUG)
											printf("	ERROR in parseSelect() at line %d in %s\n", __LINE__, __FILE__);
										return RETURN_ERROR;
									}
								}
								else
								{
									int ptr_type = PTR_TYPE_CASE_NODE;
									if (parseCaseNode(&col_case_node, cols_strs_arr[arr_k], col_word, 5, &new_col_name, *select_node, &ptr_type
													,malloced_head, the_debug) != RETURN_GOOD)
									{
										if (the_debug == YES_DEBUG)
											printf("	ERROR in parseSelect() at line %d in %s\n", __LINE__, __FILE__);
										*select_node = NULL;
										return RETURN_ERROR;
									}

									if (ptr_type == PTR_TYPE_MATH_NODE)
									{
										if (the_debug == YES_DEBUG)
											printf("Here\n");
										col_math_node = (struct math_node*) col_case_node;
										col_case_node = NULL;
									}
								}

								break;
							// END Parse case statement
						}
						else if (strcmp_Upper(col_word, "EXCEPT") == 0)
						{
							// START Setup except
								if (except)
								{
									if (the_debug == YES_DEBUG)
										printf("	ERROR in parseSelect() at line %d in %s\n", __LINE__, __FILE__);
									printf("The word \"EXCEPT\" was declared in an already declared EXCEPT clause. Please erase it. Failing word: \"%s\"\n", word);
									errorTeardown(NULL, malloced_head, the_debug);
									*select_node = NULL;
									return RETURN_ERROR;
								}

								except = true;

								// START Loop through rest of cols_strs_arr and get all excepted columns
								if (i == 0)
								{
									addListNodePtr(&except_cols_name_head, &except_cols_name_tail, NULL, PTR_TYPE_CHAR, ADDLISTNODE_TAIL
												  ,NULL, malloced_head, the_debug);
									addListNodePtr(&except_cols_alias_head, &except_cols_alias_tail, NULL, PTR_TYPE_CHAR, ADDLISTNODE_TAIL
												  ,NULL, malloced_head, the_debug);

									while (getNextWord(cols_strs_arr[arr_k], col_word, &col_word_index) == 0)
									{
										if (the_debug == YES_DEBUG)
											printf("word in except loop one = _%s_\n", col_word);

										if (cols_strs_arr[arr_k][col_word_index] == '.')
										{
											if (col_word[0] == 0)
											{
												except_cols_alias_tail->ptr_value = except_cols_name_tail->ptr_value;

												col_word_index++;
												getNextWord(cols_strs_arr[arr_k], col_word, &col_word_index);

												except_cols_name_tail->ptr_value = upper(col_word, NULL, malloced_head, the_debug);
												if (except_cols_name_tail->ptr_value == NULL)
												{
													if (the_debug == YES_DEBUG)
														printf("	ERROR in parseSelect() at line %d in %s\n", __LINE__, __FILE__);
													*select_node = NULL;
													return RETURN_ERROR;
												}
											}
											else
											{
												except_cols_alias_tail->ptr_value = upper(col_word, NULL, malloced_head, the_debug);
												if (except_cols_alias_tail->ptr_value == NULL)
												{
													if (the_debug == YES_DEBUG)
														printf("	ERROR in parseSelect() at line %d in %s\n", __LINE__, __FILE__);
													*select_node = NULL;
													return RETURN_ERROR;
												}

												col_word_index++;
											}
										}
										else if (except_cols_name_tail->ptr_value == NULL)
										{
											except_cols_name_tail->ptr_value = upper(col_word, NULL, malloced_head, the_debug);
											if (except_cols_name_tail->ptr_value == NULL)
											{
												if (the_debug == YES_DEBUG)
													printf("	ERROR in parseSelect() at line %d in %s\n", __LINE__, __FILE__);
												*select_node = NULL;
												return RETURN_ERROR;
											}
										}
										else
										{
											if (the_debug == YES_DEBUG)
												printf("	ERROR in parseSelect() at line %d in %s\n", __LINE__, __FILE__);
											errorTeardown(NULL, malloced_head, the_debug);
											*select_node = NULL;
											return RETURN_ERROR;
										}
									}

									for (int arr_k_2=arr_k+1; arr_k_2<size_result; arr_k_2++)
									{
										addListNodePtr(&except_cols_name_head, &except_cols_name_tail, NULL, PTR_TYPE_CHAR, ADDLISTNODE_TAIL
													  ,NULL, malloced_head, the_debug);
										addListNodePtr(&except_cols_alias_head, &except_cols_alias_tail, NULL, PTR_TYPE_CHAR, ADDLISTNODE_TAIL
													  ,NULL, malloced_head, the_debug);

										col_word_index = 0;

										while (getNextWord(cols_strs_arr[arr_k_2], col_word, &col_word_index) == 0)
										{
											if (the_debug == YES_DEBUG)
												printf("word in except loop two = _%s_\n", col_word);

											if (cols_strs_arr[arr_k_2][col_word_index] == '.')
											{
												if (col_word[0] == 0)
												{
													except_cols_alias_tail->ptr_value = except_cols_name_tail->ptr_value;

													col_word_index++;
													getNextWord(cols_strs_arr[arr_k_2], col_word, &col_word_index);

													except_cols_name_tail->ptr_value = upper(col_word, NULL, malloced_head, the_debug);
													if (except_cols_name_tail->ptr_value == NULL)
													{
														if (the_debug == YES_DEBUG)
															printf("	ERROR in parseSelect() at line %d in %s\n", __LINE__, __FILE__);
														*select_node = NULL;
														return RETURN_ERROR;
													}
												}
												else
												{
													except_cols_alias_tail->ptr_value = upper(col_word, NULL, malloced_head, the_debug);
													if (except_cols_alias_tail->ptr_value == NULL)
													{
														if (the_debug == YES_DEBUG)
															printf("	ERROR in parseSelect() at line %d in %s\n", __LINE__, __FILE__);
														*select_node = NULL;
														return RETURN_ERROR;
													}

													col_word_index++;
												}
											}
											else if (except_cols_name_tail->ptr_value == NULL)
											{
												except_cols_name_tail->ptr_value = upper(col_word, NULL, malloced_head, the_debug);
												if (except_cols_name_tail->ptr_value == NULL)
												{
													if (the_debug == YES_DEBUG)
														printf("	ERROR in parseSelect() at line %d in %s\n", __LINE__, __FILE__);
													*select_node = NULL;
													return RETURN_ERROR;
												}
											}
											else
											{
												if (the_debug == YES_DEBUG)
													printf("	ERROR in parseSelect() at line %d in %s\n", __LINE__, __FILE__);
												printf("Failing word: \"%s\"\n", col_word);
												errorTeardown(NULL, malloced_head, the_debug);
												*select_node = NULL;
												return RETURN_ERROR;
											}
										}
									}
								}
								// END START Loop through rest of cols_strs_arr and get all excepted columns

								break;
							// END Setup except
						}
						else if (cols_strs_arr[arr_k][col_word_index] == '.')
						{
							// START Assign alias to column
								if (alias != NULL)
								{
									if (the_debug == YES_DEBUG)
										printf("	ERROR in parseSelect() at line %d in %s\n", __LINE__, __FILE__);
									printf("Failing word: \"%s\"\n", col_word);
									errorTeardown(NULL, malloced_head, the_debug);
									*select_node = NULL;
									return RETURN_ERROR;
								}

								if (col_word[0] == 0)
								{
									alias = col_name;

									col_word_index++;
									getNextWord(cols_strs_arr[arr_k], col_word, &col_word_index);

									col_name = upper(col_word, NULL, malloced_head, the_debug);
									if (col_name == NULL)
									{
										if (the_debug == YES_DEBUG)
											printf("	ERROR in parseSelect() at line %d in %s\n", __LINE__, __FILE__);
										*select_node = NULL;
										return RETURN_ERROR;
									}
								}
								else
								{
									alias = upper(col_word, NULL, malloced_head, the_debug);
									if (alias == NULL)
									{
										if (the_debug == YES_DEBUG)
											printf("	ERROR in parseSelect() at line %d in %s\n", __LINE__, __FILE__);
										*select_node = NULL;
										return RETURN_ERROR;
									}

									col_word_index++;
								}
							// END Assign alias to column
						}
						else if (((col_word[0] == '+' || col_word[0] == '-' || col_word[0] == '*' || col_word[0] == '/' || col_word[0] == '^') && col_name != NULL)
								 || (col_word[0] == '(' && col_name == NULL))
						{
							// START Parse math node
								if (i == 0)
								{
									col_math_node = (struct math_node*) myMalloc(sizeof(struct math_node), NULL, malloced_head, the_debug);
									if (col_math_node == NULL)
									{
										if (the_debug == YES_DEBUG)
											printf("	ERROR in parseSelect() at line %d in %s\n", __LINE__, __FILE__);
										*select_node = NULL;
										return RETURN_ERROR;
									}

									myFree((void**) &col_name, NULL, malloced_head, the_debug);
								}
								else
								{
									if (alias != NULL)
									{
										char* temp_col_name = (char*) myMalloc(sizeof(char) * 200, NULL, malloced_head, the_debug);
										if (temp_col_name == NULL)
										{
											if (the_debug == YES_DEBUG)
												printf("	ERROR in parseSelect() at line %d in %s\n", __LINE__, __FILE__);
											*select_node = NULL;
											return RETURN_ERROR;
										}

										strcpy(temp_col_name, alias);
										strcat(temp_col_name, ".");
										strcat(temp_col_name, col_name);

										if (parseMathNode(&col_math_node, &new_col_name, &temp_col_name, cols_strs_arr[arr_k], col_word, col_word_index, *select_node
														  ,malloced_head, the_debug) != RETURN_GOOD)
										{
											*select_node = NULL;
											return RETURN_ERROR;
										}

										myFree((void**) &temp_col_name, NULL, malloced_head, the_debug);
									}
									else if (parseMathNode(&col_math_node, &new_col_name, &col_name, cols_strs_arr[arr_k], col_word, col_word_index, *select_node
														  ,malloced_head, the_debug) != RETURN_GOOD)
									{
										*select_node = NULL;
										return RETURN_ERROR;
									}
								}

								break;
							// END Parse math node
						}
						else if (col_word[0] == '(')
						{
							// START If not math node, then func node
								if (i == 0)
								{
									col_func_node = (struct func_node*) myMalloc(sizeof(struct func_node), NULL, malloced_head, the_debug);
									if (col_func_node == NULL)
									{
										if (the_debug == YES_DEBUG)
											printf("	ERROR in parseSelect() at line %d in %s\n", __LINE__, __FILE__);
										return RETURN_ERROR;
									}

									myFree((void**) &col_name, NULL, malloced_head, the_debug);
								}
								else
								{
									if (parseFuncNode(&col_func_node, &new_col_name, &col_name, cols_strs_arr[arr_k], col_word, &col_word_index, *select_node, false
														  ,malloced_head, the_debug) != RETURN_GOOD)
									{
										*select_node = NULL;
										return RETURN_ERROR;
									}

									if (the_debug == YES_DEBUG)
										printf("col_word after parsing col_node = _%s_\n", col_word);

									if (col_word[0] == '+' || col_word[0] == '-' || col_word[0] == '*' || col_word[0] == '/' || col_word[0] == '^')
									{
										col_math_node = (struct math_node*) myMalloc(sizeof(struct math_node), NULL, malloced_head, the_debug);
										if (col_math_node == NULL)
										{
											if (the_debug == YES_DEBUG)
												printf("	ERROR in parseSelect() at line %d in %s\n", __LINE__, __FILE__);
											return RETURN_ERROR;
										}

										col_math_node->ptr_one = col_func_node;
										col_math_node->ptr_one_type = PTR_TYPE_FUNC_NODE;

										col_math_node->ptr_two = NULL;
										col_math_node->ptr_two_type = -1;

										col_math_node->operation = -1;
										col_math_node->parent = NULL;
										col_math_node->result_type = -1;

										if (parseMathNode(&col_math_node, &new_col_name, &col_name, cols_strs_arr[arr_k], col_word, col_word_index, *select_node
														  ,malloced_head, the_debug) != RETURN_GOOD)
										{
											*select_node = NULL;
											return RETURN_ERROR;
										}

										col_func_node = NULL;
									}
								}

								break;
							// END If not math node, then func node
						}
						else if (col_name == NULL && !except)
						{
							// START Declare column name by initializing col_name
								if (col_word[0] == '+' || col_word[0] == '/' || col_word[0] == '^')
								{
									if (the_debug == YES_DEBUG)
										printf("	ERROR in parseSelect() at line %d in %s\n", __LINE__, __FILE__);
									printf("A math operation was used to declare a new column without a first operand. Please specify two operands. Failing word: \"%s\"\n", col_word);
									errorTeardown(NULL, malloced_head, the_debug);
									*select_node = NULL;
									return RETURN_ERROR;
								}
								else if (col_word[0] == '-')
								{
									char* temp = myMalloc(sizeof(char) * 200, NULL, malloced_head, the_debug);
									if (temp == NULL)
									{
										if (the_debug == YES_DEBUG)
											printf("	ERROR in parseSelect() at line %d in %s\n", __LINE__, __FILE__);
										*select_node = NULL;
										return RETURN_ERROR;
									}
									strcpy(temp, "-\0");

									getNextWord(cols_strs_arr[arr_k], col_word, &col_word_index);

									if (atoi(col_word) == 0)
									{
										if (the_debug == YES_DEBUG)
											printf("	ERROR in parseSelect() at line %d in %s\n", __LINE__, __FILE__);
										printf("A negative sign was apply to a word which was not an number. Please remove the negative sign or make the operand a number. Failing word: \"%s\"\n", col_word);
										errorTeardown(NULL, malloced_head, the_debug);
										*select_node = NULL;
										return RETURN_ERROR;
									}

									strcat(temp, col_word);

									myFree((void**) &col_word, NULL, malloced_head, the_debug);

									col_word = temp;
								}

								if (col_word[0] == 39)
								{
									col_name = myMalloc(sizeof(char) * (strLength(col_word)+1), NULL, malloced_head, the_debug);
									if (col_name == NULL)
									{
										if (the_debug == YES_DEBUG)
											printf("	ERROR in parseSelect() at line %d in %s\n", __LINE__, __FILE__);
										return RETURN_ERROR;
									}

									strcpy(col_name, col_word);
								}
								else
								{
									col_name = upper(col_word, NULL, malloced_head, the_debug);
									if (col_name == NULL)
									{
										if (the_debug == YES_DEBUG)
											printf("	ERROR in parseSelect() at line %d in %s\n", __LINE__, __FILE__);
										*select_node = NULL;
										return RETURN_ERROR;
									}
								}
							// END Declare column name by initializing col_name
						}
						else if (!except)
						{
							// START Column is given a new name
								if (the_debug == YES_DEBUG)
									printf("Column is given a new name\n");
								if (new_col_name != NULL)
								{
									if (the_debug == YES_DEBUG)
										printf("	ERROR in parseSelect() at line %d in %s\n", __LINE__, __FILE__);
									printf("A column was declared with two new names. Please erase one of them. Failing word: \"%s\"\n", col_word);
									errorTeardown(NULL, malloced_head, the_debug);
									*select_node = NULL;
									return RETURN_ERROR;
								}

								if (parseNewColumnName(&new_col_name, cols_strs_arr[arr_k], &col_word, &col_word_index, malloced_head, the_debug) != RETURN_GOOD)
								{
									*select_node = NULL;
									return RETURN_ERROR;
								}
							// END Column is given a new name
						}
					}

					if (the_debug == YES_DEBUG)
					{
						printf("cur column = _%s.%s_ as _%s_", alias, col_name, new_col_name);
						if (col_func_node != NULL)
							printf(" w/ valid func_node");
						if (col_math_node != NULL)
							printf(" w/ valid math_node");
						if (col_case_node != NULL)
							printf(" w/ valid case_node");
						printf("\n");

						if (except_cols_name_head != NULL)
							traverseListNodesPtr(&except_cols_name_head, &except_cols_name_tail, TRAVERSELISTNODES_HEAD, "Except names: ");
						if (except_cols_alias_head != NULL)
							traverseListNodesPtr(&except_cols_alias_head, &except_cols_alias_tail, TRAVERSELISTNODES_HEAD, "Except aliases: ");
					}
				// END Parse column string

				// START Decide how to init column in (*select_node)->columns_arr
					if (col_func_node != NULL)
					{
						// START Assign to col func_node ptr
							if (i == 0)
							{
								(*select_node)->columns_arr_size += 1;
								myFree((void**) &col_func_node, NULL, malloced_head, the_debug);
							}
							else //if (i == 1)
							{
								(*select_node)->columns_arr[index]->func_node = col_func_node;

								if (new_col_name != NULL)
								{
									(*select_node)->columns_arr[index]->new_name = new_col_name;
									new_col_name = NULL;
								}

								(*select_node)->columns_arr[index]->rows_data_type = col_func_node->result_type;

								index++;
							}
						// END Assign to col func_node ptr
					}
					else if (col_math_node != NULL)
					{
						// START Assign to col math_node ptr
							if (i == 0)
							{
								(*select_node)->columns_arr_size += 1;
								myFree((void**) &col_math_node, NULL, malloced_head, the_debug);
							}
							else //if (i == 1)
							{
								(*select_node)->columns_arr[index]->math_node = col_math_node;

								if (new_col_name != NULL)
								{
									(*select_node)->columns_arr[index]->new_name = new_col_name;
									new_col_name = NULL;
								}

								(*select_node)->columns_arr[index]->rows_data_type = col_math_node->result_type;

								index++;
							}
						// END Assign to col math_node ptr
					}
					else if (col_case_node != NULL)
					{
						// START Assign to col case_node ptr
							if (i == 0)
							{
								(*select_node)->columns_arr_size += 1;
								myFree((void**) &col_case_node, NULL, malloced_head, the_debug);
							}
							else //if (i == 1)
							{
								(*select_node)->columns_arr[index]->case_node = col_case_node;

								if (new_col_name != NULL)
								{
									(*select_node)->columns_arr[index]->new_name = new_col_name;
									new_col_name = NULL;
								}

								(*select_node)->columns_arr[index]->rows_data_type = col_case_node->result_type;

								index++;
							}
						// END Assign to col case_node ptr
					}
					else if (col_name != NULL && col_name[0] == '*')
					{
						if (getAllColsCuzStar(select_node, NULL, alias, i, index
											 ,except_cols_name_head, except_cols_alias_head, except
											 ,malloced_head, the_debug) != RETURN_GOOD)
						{
							if (the_debug == YES_DEBUG)
								printf("	ERROR in parseSelect() at line %d in %s\n", __LINE__, __FILE__);
							return RETURN_ERROR;
						}

						if (new_col_name != NULL && i == 1)
						{
							for (int a=0; a<(*select_node)->columns_arr_size; a++)
							{
								//printf("a = %d\n", a);
								if ((*select_node)->columns_arr[a]->new_name == NULL)
								{
									(*select_node)->columns_arr[a]->new_name = myMalloc(sizeof(char) * 256, NULL, malloced_head, the_debug);
									if ((*select_node)->columns_arr[a]->new_name == NULL)
									{
										if (the_debug == YES_DEBUG)
											printf("	ERROR in parseSelect() at line %d in %s\n", __LINE__, __FILE__);
										*select_node = NULL;
										return RETURN_ERROR;
									}
									strcpy((*select_node)->columns_arr[a]->new_name, "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0");

									char* col_name = NULL;

									struct col_in_select_node* cur = (*select_node)->columns_arr[a];
									while (cur != NULL && (cur->col_ptr_type == PTR_TYPE_COL_IN_SELECT_NODE || cur->func_node != NULL || cur->math_node != NULL || cur->case_node != NULL))
									{
										//printf("Finding name of col: _%s_, _%s_\n", cur->col_ptr != NULL ? "valid col_ptr" : "null col_ptr", cur->new_name);
										if (cur->new_name != NULL && cur->new_name[0] != 0)
										{
											col_name = cur->new_name;
											break;
										}
										cur = cur->col_ptr;
									}
									
									if (cur->col_ptr_type == PTR_TYPE_TABLE_COLS_INFO)
										col_name = ((struct table_cols_info*) cur->col_ptr)->col_name;
									else if (col_name == NULL || col_name[0] == 0)
									{
										if (the_debug == YES_DEBUG)
											printf("	ERROR in parseSelect() at line %d in %s\n", __LINE__, __FILE__);
										printf("An error was encountered while parsing the blanket column name. Please try again.\n");
										errorTeardown(NULL, malloced_head, the_debug);
										*select_node = NULL;
										return RETURN_ERROR;
									}

									if (new_col_name[0] == '+')
									{
										strcpy((*select_node)->columns_arr[a]->new_name, col_name);
										//printf("Here = _%s_\n", (*select_node)->columns_arr[a]->new_name);

										char* temp = (char*) myMalloc(sizeof(char) * 10, NULL, malloced_head, the_debug);
										if (temp == NULL)
										{
											if (the_debug == YES_DEBUG)
												printf("	ERROR in parseSelect() at line %d in %s\n", __LINE__, __FILE__);
											return RETURN_ERROR;
										}

										strcpy(temp, new_col_name);

										int index = 0;
										while (temp[index] != 0)
										{
											temp[index] = temp[index+1];
											index++;
										}
										//printf("After removing the plus = _%s_\n", temp);

										strcat((*select_node)->columns_arr[a]->new_name, temp);

										myFree((void**) &temp, NULL, malloced_head, the_debug);
									}
									else
									{
										strcpy((*select_node)->columns_arr[a]->new_name, new_col_name);
									}
									//printf("new col name here = \"%s\"\n", (*select_node)->columns_arr[a]->new_name);

									while (strcontains((*select_node)->columns_arr[a]->new_name, '#'))
									{
										char* new = myMalloc(sizeof(char) * 10, NULL, malloced_head, the_debug);
										if (new == NULL)
										{
											if (the_debug == YES_DEBUG)
												printf("	ERROR in parseSelect() at line %d in %s\n", __LINE__, __FILE__);
											*select_node = NULL;
											return RETURN_ERROR;
										}
										sprintf(new, "%d", a);
										//printf("new = _%s_\n", new);

										strReplace((*select_node)->columns_arr[a]->new_name, "#", new);

										myFree((void**) &new, NULL, malloced_head, the_debug);
									}

									while (strcontains((*select_node)->columns_arr[a]->new_name, '@'))
									{
										char* new = myMalloc(sizeof(char) * 10, NULL, malloced_head, the_debug);
										if (new == NULL)
										{
											if (the_debug == YES_DEBUG)
												printf("	ERROR in parseSelect() at line %d in %s\n", __LINE__, __FILE__);
											*select_node = NULL;
											return RETURN_ERROR;
										}
										new[0] = 0;

										int hmm = a;
										while (hmm >= 0)
										{
											//printf("hmm = %d\n", hmm);
											char str_hmm[2];
											str_hmm[1] = 0;

											if (hmm > 26)
											{
												//printf("first\n");
												str_hmm[0] = 'A';
											}
											else
											{
												//printf("second\n");
												str_hmm[0] = hmm+65;
											}

											//printf("str_hmm = _%s_\n", str_hmm);
											strcat(new, str_hmm);
											//printf("new = _%s_\n", new);
											hmm = hmm-26;
										}

										//printf("new = _%s_\n", new);
										strReplace((*select_node)->columns_arr[a]->new_name, "@", new);
										//printf("here = _%s_, _%s_\n", new, (*select_node)->columns_arr[a]->new_name);

										myFree((void**) &new, NULL, malloced_head, the_debug);
									}
									(*select_node)->columns_arr[a]->new_name[strLength((*select_node)->columns_arr[a]->new_name)] = 0;
									//printf("new col name FINAL = \"%s\"\n", (*select_node)->columns_arr[a]->new_name);
								}
							}
						}

						if (i == 1)
						{
							if (the_debug == YES_DEBUG)
								printf("BEFORE index = %d\n", index);
							while (index < (*select_node)->columns_arr_size && (*select_node)->columns_arr[index]->col_ptr != NULL)
							{
								index++;
							}
						}
					}
					else if (col_name != NULL)
					{
						// START Find name of column in prev ptrs and/or joins
							if (i == 0)
							{
								int found_col = getColInSelectNodeFromName(NULL, PTR_TYPE_COL_IN_SELECT_NODE, -1, i, *select_node, alias, col_name, malloced_head, the_debug);

								int hmm_int = atoi(col_name);
								if (the_debug == YES_DEBUG)
									printf("hmm_int = %d\n", hmm_int);

								if (found_col == RETURN_GOOD || hmm_int != 0 || col_name[0] == 39)
									(*select_node)->columns_arr_size += 1;
								else
								{
									printf("A column was declared which could not be found. Failing word: ""%s%s%s""\n", alias == NULL ? "" : alias, alias == NULL ? "" : ".", col_name);
									errorTeardown(NULL, malloced_head, the_debug);
									*select_node = NULL;
									return RETURN_ERROR;
								}
							}
							else if (!except)//if (i == 1)
							{
								int found_col = getColInSelectNodeFromName((void*) (*select_node)->columns_arr[index], PTR_TYPE_COL_IN_SELECT_NODE, -1, i, *select_node, alias, col_name, malloced_head, the_debug);

								int hmm_int = atoi(col_name);

								if (found_col != RETURN_GOOD && hmm_int == 0 && col_name[0] != 39)
								{
									printf("A column was declared which could not be found. Failing word: ""%s%s%s""\n", alias == NULL ? "" : alias, alias == NULL ? "" : ".", col_name);
									errorTeardown(NULL, malloced_head, the_debug);
									*select_node = NULL;
									return RETURN_ERROR;
								}
								else if (found_col != 0 && (hmm_int != 0 || col_name[0] == 39))
								{
									void* temp = NULL;
									int ptr_type = -1;

									if (hmm_int != 0 && alias != NULL)
									{
										temp = myMalloc(sizeof(double), NULL, malloced_head, the_debug);
										if (temp == NULL)
										{
											if (the_debug == YES_DEBUG)
												printf("	ERROR in parseSelect() at line %d in %s\n", __LINE__, __FILE__);
											*select_node = NULL;
											return RETURN_ERROR;
										}

										double hmm_double = 0.0;
										sscanf(col_name, "%lf", &hmm_double);
										//printf("hmm_double = %lf\n", hmm_double);

										hmm_double = hmm_double / pow(10, strLength(col_name));
										//printf("hmm_double = %lf\n", hmm_double);

										double hmm_double_2 = 0.0;
										sscanf(alias, "%lf", &hmm_double_2);
										//printf("hmm_double_2 = %lf\n", hmm_double_2);

										hmm_double += hmm_double_2;
										//printf("hmm_double = %lf\n", hmm_double);

										*((double*) temp) = hmm_double;
										//printf("temp = %lf\n", *((double*) temp));

										ptr_type = PTR_TYPE_REAL;
									}
									else if (hmm_int != 0 || col_name[0] == '0')
									{
										temp = myMalloc(sizeof(int), NULL, malloced_head, the_debug);
										if (temp == NULL)
										{
											if (the_debug == YES_DEBUG)
												printf("	ERROR in parseSelect() at line %d in %s\n", __LINE__, __FILE__);
											*select_node = NULL;
											return RETURN_ERROR;
										}

										*((int*) temp) = hmm_int;

										ptr_type = PTR_TYPE_INT;
									}
									else if (col_name[0] == 39)
									{
										temp = myMalloc(sizeof(char) * (strLength(col_name)+1), NULL, malloced_head, the_debug);
										if (temp == NULL)
										{
											if (the_debug == YES_DEBUG)
												printf("	ERROR in parseSelect() at line %d in %s\n", __LINE__, __FILE__);
											*select_node = NULL;
											return RETURN_ERROR;
										}

										strcpy(temp, col_name);

										trimStr(temp);

										if (isDate((char*) temp))
											ptr_type = PTR_TYPE_DATE;
										else
											ptr_type = PTR_TYPE_CHAR;
									}

									(*select_node)->columns_arr[index]->case_node = myMalloc(sizeof(struct case_node), NULL, malloced_head, the_debug);
									if ((*select_node)->columns_arr[index]->case_node == NULL)
									{
										if (the_debug == YES_DEBUG)
											printf("	ERROR in parseSelect() at line %d in %s\n", __LINE__, __FILE__);
										*select_node = NULL;
										return RETURN_ERROR;
									}

									(*select_node)->columns_arr[index]->case_node->case_when_head = NULL;
									(*select_node)->columns_arr[index]->case_node->case_then_value_head = NULL;

									addListNodePtr(&(*select_node)->columns_arr[index]->case_node->case_when_head, &(*select_node)->columns_arr[index]->case_node->case_when_tail, NULL, -1, ADDLISTNODE_TAIL
								 				  ,NULL, malloced_head, the_debug);
									addListNodePtr(&(*select_node)->columns_arr[index]->case_node->case_then_value_head, &(*select_node)->columns_arr[index]->case_node->case_then_value_tail, temp, ptr_type, ADDLISTNODE_TAIL
												  ,NULL, malloced_head, the_debug);

									(*select_node)->columns_arr[index]->case_node->result_type = ptr_type;

									if (new_col_name == NULL)
									{
										if (the_debug == YES_DEBUG)
											printf("	ERROR in parseSelect() at line %d in %s\n", __LINE__, __FILE__);
										printf("A column containing constant %s was created but it was not named. Please provide a name.\n", col_name);
										errorTeardown(NULL, malloced_head, the_debug);
										*select_node = NULL;
										return RETURN_ERROR;
									}

									(*select_node)->columns_arr[index]->rows_data_type = (*select_node)->columns_arr[index]->case_node->result_type;
								}
								else
									(*select_node)->columns_arr[index]->rows_data_type = ((struct col_in_select_node*) (*select_node)->columns_arr[index]->col_ptr)->rows_data_type;

								if (new_col_name != NULL)
								{
									(*select_node)->columns_arr[index]->new_name = new_col_name;
									new_col_name = NULL;
								}

								index++;
							}
						// END Find name of column in prev ptrs and/or joins
					}
					else if (!except)
					{
						if (the_debug == YES_DEBUG)
							printf("	ERROR in parseSelect() at line %d in %s\n", __LINE__, __FILE__);
						printf("Failing word: \"%s\"\n", col_word);
						errorTeardown(NULL, malloced_head, the_debug);
						*select_node = NULL;
						return RETURN_ERROR;
					}
				// END Decide how to init column in (*select_node)->columns_arr

				myFree((void**) &col_word, NULL, malloced_head, the_debug);

				if (alias != NULL)
					myFree((void**) &alias, NULL, malloced_head, the_debug);
				if (col_name != NULL)
					myFree((void**) &col_name, NULL, malloced_head, the_debug);
				if (new_col_name != NULL)
					myFree((void**) &new_col_name, NULL, malloced_head, the_debug);
			}
		}

		for (int arr_k=0; arr_k<size_result; arr_k++)
			myFree((void**) &cols_strs_arr[arr_k], NULL, malloced_head, the_debug);

		myFree((void**) &cols_strs_arr, NULL, malloced_head, the_debug);

		freeAnyLinkedList((void**) &except_cols_name_head, PTR_TYPE_LIST_NODE_PTR, NULL, malloced_head, the_debug);
		freeAnyLinkedList((void**) &except_cols_alias_head, PTR_TYPE_LIST_NODE_PTR, NULL, malloced_head, the_debug);
	// END Parse columns now that tables have been gotten
	

	// START Parse clauses at the end
		do
		{
			if (the_debug == YES_DEBUG)
				printf("word = _%s_\n", word);

			compared = strcmp_Upper(word, "WHERE");
			if (compared == -2)
			{
				if (the_debug == YES_DEBUG)
					printf("	ERROR in parseSelect() at line %d in %s\n", __LINE__, __FILE__);
				*select_node = NULL;
				return RETURN_ERROR;
			}
			else if (compared == 0)
			{
				// START Parse where clause
					char* where_clause = (char*) myMalloc(sizeof(char) * 2000, NULL, malloced_head, the_debug);
					if (where_clause == NULL)
					{
						if (the_debug == YES_DEBUG)
							printf("	ERROR in parseSelect() at line %d in %s\n", __LINE__, __FILE__);
						*select_node = NULL;
						return RETURN_ERROR;
					}

					strcpy(where_clause, "WHERE ");

					while (getNextWord(input, word, &index) == 0 && strcmp(word, ";") != 0 &&
						   strcmp_Upper(word, "ORDER") != 0 &&
						   strcmp_Upper(word, "GROUP") != 0 &&
						   strcmp_Upper(word, "HAVING") != 0 &&
						   strcmp_Upper(word, "WHERE") != 0)
					{
						if (the_debug == YES_DEBUG)
							printf("word in get where clause loop = _%s_\n", word);

						strcat(where_clause, word);

						if (input[index] == '.')
						{
							if (where_clause[strLength(where_clause)-1] == ' ')
								where_clause[strLength(where_clause)-1] = '.';
							else
								strcat(where_clause, ".");
							index++;
						}
						else
							strcat(where_clause, " ");
					}

					if (the_debug == YES_DEBUG)
						printf("where_clause = _%s_\n", where_clause);

					struct where_clause_node* temp_where = NULL;

					if (parseWhereClause(where_clause, &temp_where, *select_node, NULL, "where", malloced_head, the_debug) != RETURN_GOOD)
					{
						if (the_debug == YES_DEBUG)
							printf("	ERROR in parseSelect() at line %d in %s\n", __LINE__, __FILE__);
						*select_node = NULL;
						return RETURN_ERROR;
					}

					(*select_node)->where_head = temp_where;

					index -= strLength(word);

					if (strcmp_Upper(word, "WHERE") == 0)
					{
						if (the_debug == YES_DEBUG)
							printf("	ERROR in parseSelect() at line %d in %s\n", __LINE__, __FILE__);
						printf("Two where clauses where declared. Please include all filters in one where clause. Failing word: \"%s\"\n", word);
						errorTeardown(NULL, malloced_head, the_debug);
						*select_node = NULL;
						return RETURN_ERROR;
					}

					myFree((void**) &where_clause, NULL, malloced_head, the_debug);
				// END Parse where clause
			}
			else if (strcmp_Upper(word, "GROUP") == 0)
			{
				// START Parse group by
					if (the_debug == YES_DEBUG)
						printf("Found group by clause\n");

					getNextWord(input, word, &index);

					if (strcmp_Upper(word, "BY") != 0)
					{
						if (the_debug == YES_DEBUG)
							printf("	ERROR in parseSelect() at line %d in %s\n", __LINE__, __FILE__);
						printf("The word \"BY\" was expected after the word \"GROUP\". Failing word: \"%s\"\n", word);
						errorTeardown(NULL, malloced_head, the_debug);
						*select_node = NULL;
						return RETURN_ERROR;
					}

					char* alias = NULL;
					char* name = NULL;

					while (getNextWord(input, word, &index) == 0 && strcmp(word, ";") != 0 &&
						   strcmp_Upper(word, "HAVING") != 0 &&
						   strcmp_Upper(word, "WHERE") != 0 &&
						   strcmp_Upper(word, "GROUP") != 0 &&
						   strcmp_Upper(word, "ORDER") != 0)
					{
						if (the_debug == YES_DEBUG)
							printf("String in group by loop: _%s_\n", word);

						if (word[0] == ',')
						{
							if (the_debug == YES_DEBUG)
								printf("alias.name = _%s.%s_\n", alias, name);
							for (int i=0; i<(*select_node)->columns_arr_size; i++)
							{
								if (assignGroupByColToAnyFunc((*select_node)->columns_arr[i], *select_node, name, alias, malloced_head, the_debug) != 0)
								{
									*select_node = NULL;
									return RETURN_ERROR;
								}
							}

							if (alias != NULL)
								myFree((void**) &alias, NULL, malloced_head, the_debug);
							myFree((void**) &name, NULL, malloced_head, the_debug);
						}
						else if (input[index] == '.')
						{
							alias = upper(word, NULL, malloced_head, the_debug);
							if (alias == NULL)
							{
								if (the_debug == YES_DEBUG)
									printf("	ERROR in parseSelect() at line %d in %s\n", __LINE__, __FILE__);
								*select_node = NULL;
								return RETURN_ERROR;
							}

							index++;

							getNextWord(input, word, &index);

							name = upper(word, NULL, malloced_head, the_debug);
							if (name == NULL)
							{
								if (the_debug == YES_DEBUG)
									printf("	ERROR in parseSelect() at line %d in %s\n", __LINE__, __FILE__);
								*select_node = NULL;
								return RETURN_ERROR;
							}
						}
						else
						{
							name = upper(word, NULL, malloced_head, the_debug);
							if (name == NULL)
							{
								if (the_debug == YES_DEBUG)
									printf("	ERROR in parseSelect() at line %d in %s\n", __LINE__, __FILE__);
								*select_node = NULL;
								return RETURN_ERROR;
							}
						}
					}

					if (strcmp_Upper(word, "WHERE") == 0 ||
						strcmp_Upper(word, "GROUP") == 0)
					{
						if (the_debug == YES_DEBUG)
							printf("	ERROR in parseSelect() at line %d in %s\n", __LINE__, __FILE__);
						printf("Two where clauses were declared, or two group by clauses were delcared. Failing word: \"%s\"\n", word);
						errorTeardown(NULL, malloced_head, the_debug);
						*select_node = NULL;
						return RETURN_ERROR;
					}

					if (strcmp_Upper(word, "ORDER") == 0 || strcmp_Upper(word, "HAVING") == 0)
					{
						index -= strLength(word);
					}

					if (the_debug == YES_DEBUG)
						printf("alias.name = _%s.%s_\n", alias, name);
					for (int i=0; i<(*select_node)->columns_arr_size; i++)
					{
						if (the_debug == YES_DEBUG)
							printf("Here i = %d\n", i);
						if (assignGroupByColToAnyFunc((*select_node)->columns_arr[i], *select_node, name, alias, malloced_head, the_debug) != 0)
						{
							*select_node = NULL;
							return RETURN_ERROR;
						}
					}

					if (alias != NULL)
						myFree((void**) &alias, NULL, malloced_head, the_debug);
					myFree((void**) &name, NULL, malloced_head, the_debug);
				// END Parse group by
			}
			else if (strcmp_Upper(word, "ORDER") == 0)
			{
				// START Parse order by
					getNextWord(input, word, &index);

					if (strcmp_Upper(word, "BY") != 0)
					{
						if (the_debug == YES_DEBUG)
							printf("	ERROR in parseSelect() at line %d in %s\n", __LINE__, __FILE__);
						printf("The word \"BY\" was expected after the word \"ORDER\". Failing word: \"%s\"\n", word);
						errorTeardown(NULL, malloced_head, the_debug);
						*select_node = NULL;
						return RETURN_ERROR;
					}

					(*select_node)->order_by = (struct order_by_node*) myMalloc(sizeof(struct order_by_node), NULL, malloced_head, the_debug);
					if ((*select_node)->order_by == NULL)
					{
						if (the_debug == YES_DEBUG)
							printf("	ERROR in parseSelect() at line %d in %s\n", __LINE__, __FILE__);
						return RETURN_ERROR;
					}

					(*select_node)->order_by->order_by_cols_head = NULL;
					(*select_node)->order_by->order_by_cols_tail = NULL;
					(*select_node)->order_by->order_by_cols_which_head = NULL;
					(*select_node)->order_by->order_by_cols_which_tail = NULL;

					addListNodePtr(&(*select_node)->order_by->order_by_cols_head, &(*select_node)->order_by->order_by_cols_tail, NULL, -1, ADDLISTNODE_TAIL
								  ,NULL, malloced_head, the_debug);
					addListNodePtr(&(*select_node)->order_by->order_by_cols_which_head, &(*select_node)->order_by->order_by_cols_which_tail, NULL, -1, ADDLISTNODE_TAIL
								  ,NULL, malloced_head, the_debug);

					while (getNextWord(input, word, &index) == 0 && word[0] != 0 && word[0] != ';' &&
						   strcmp_Upper(word, "HAVING") != 0 &&
						   strcmp_Upper(word, "WHERE") != 0 &&
						   strcmp_Upper(word, "GROUP") != 0 &&
						   strcmp_Upper(word, "ORDER") != 0)
					{
						if (the_debug == YES_DEBUG)
							printf("order by word = _%s_\n", word);

						if (word[0] == ',')
						{
							if (the_debug == YES_DEBUG)
								printf("adding to order by lists\n");
							if ((*select_node)->order_by->order_by_cols_tail->ptr_value == NULL)
							{
								if (the_debug == YES_DEBUG)
									printf("	ERROR in parseSelect() at line %d in %s\n", __LINE__, __FILE__);
								printf("Failing word: \"%s\"\n", word);
								errorTeardown(NULL, malloced_head, the_debug);
								*select_node = NULL;
								return RETURN_ERROR;
							}

							if ((*select_node)->order_by->order_by_cols_which_tail->ptr_value == NULL)
							{
								int* which = (int*) myMalloc(sizeof(int), NULL, malloced_head, the_debug);
								if (which == NULL)
								{
									if (the_debug == YES_DEBUG)
										printf("	ERROR in parseSelect() at line %d in %s\n", __LINE__, __FILE__);
									return RETURN_ERROR;
								}

								*which = ORDER_BY_ASC;

								(*select_node)->order_by->order_by_cols_which_tail->ptr_value = which;
								(*select_node)->order_by->order_by_cols_which_tail->ptr_type = PTR_TYPE_INT;
							}

							addListNodePtr(&(*select_node)->order_by->order_by_cols_head, &(*select_node)->order_by->order_by_cols_tail, NULL, -1, ADDLISTNODE_TAIL
										  ,NULL, malloced_head, the_debug);
							addListNodePtr(&(*select_node)->order_by->order_by_cols_which_head, &(*select_node)->order_by->order_by_cols_which_tail, NULL, -1, ADDLISTNODE_TAIL
										  ,NULL, malloced_head, the_debug);
						}
						else if (strcmp_Upper(word, "ASC") == 0 && (*select_node)->order_by->order_by_cols_which_tail->ptr_value == NULL)
						{
							if (the_debug == YES_DEBUG)
								printf("declaring order by which ASC\n");
							int* which = (int*) myMalloc(sizeof(int), NULL, malloced_head, the_debug);
							if (which == NULL)
							{
								if (the_debug == YES_DEBUG)
									printf("	ERROR in parseSelect() at line %d in %s\n", __LINE__, __FILE__);
								return RETURN_ERROR;
							}

							*which = ORDER_BY_ASC;

							(*select_node)->order_by->order_by_cols_which_tail->ptr_value = which;
							(*select_node)->order_by->order_by_cols_which_tail->ptr_type = PTR_TYPE_INT;
						}
						else if (strcmp_Upper(word, "DESC") == 0 && (*select_node)->order_by->order_by_cols_which_tail->ptr_value == NULL)
						{
							if (the_debug == YES_DEBUG)
								printf("declaring order by which DESC\n");
							int* which = (int*) myMalloc(sizeof(int), NULL, malloced_head, the_debug);
							if (which == NULL)
							{
								if (the_debug == YES_DEBUG)
									printf("	ERROR in parseSelect() at line %d in %s\n", __LINE__, __FILE__);
								return RETURN_ERROR;
							}

							*which = ORDER_BY_DESC;

							(*select_node)->order_by->order_by_cols_which_tail->ptr_value = which;
							(*select_node)->order_by->order_by_cols_which_tail->ptr_type = PTR_TYPE_INT;
						}
						else if ((*select_node)->order_by->order_by_cols_tail->ptr_value == NULL)
						{
							char* alias = NULL;
							char* name = NULL;

							if (input[index] == '.')
							{
								alias = upper(word, NULL, malloced_head, the_debug);
								if (alias == NULL)
								{
									if (the_debug == YES_DEBUG)
										printf("	ERROR in parseSelect() at line %d in %s\n", __LINE__, __FILE__);
									*select_node = NULL;
									return RETURN_ERROR;
								}

								index++;

								getNextWord(input, word, &index);

								name = upper(word, NULL, malloced_head, the_debug);
								if (name == NULL)
								{
									if (the_debug == YES_DEBUG)
										printf("	ERROR in parseSelect() at line %d in %s\n", __LINE__, __FILE__);
									*select_node = NULL;
									return RETURN_ERROR;
								}
							}
							else
							{
								name = upper(word, NULL, malloced_head, the_debug);
								if (name == NULL)
								{
									if (the_debug == YES_DEBUG)
										printf("	ERROR in parseSelect() at line %d in %s\n", __LINE__, __FILE__);
									*select_node = NULL;
									return RETURN_ERROR;
								}
							}

							if (the_debug == YES_DEBUG)
							{
								printf("trying to find word in columns_arr\n");
								printf("alias.name = _%s.%s_\n", alias, name);
							}
							bool found = false;

							for (int i=0; i<(*select_node)->columns_arr_size; i++)
							{
								char* col = (*select_node)->columns_arr[i]->new_name;

								if (the_debug == YES_DEBUG)
									printf("new_name = _%s_\n", col);

								if (col != NULL && strcmp_Upper(word, col) == 0)
								{
									(*select_node)->order_by->order_by_cols_tail->ptr_value = (*select_node)->columns_arr[i];
									(*select_node)->order_by->order_by_cols_tail->ptr_type = PTR_TYPE_COL_IN_SELECT_NODE;

									found = true;
									break;
								}

								if ((*select_node)->columns_arr[i]->table_ptr_type == PTR_TYPE_SELECT_NODE || (*select_node)->columns_arr[i]->table_ptr_type == PTR_TYPE_TABLE_INFO)
								{
									struct col_in_select_node* cur = (*select_node)->columns_arr[i];
									while (cur->table_ptr_type == PTR_TYPE_SELECT_NODE)
										cur = cur->col_ptr;

									col = ((struct table_cols_info*) cur->col_ptr)->col_name;

									if (the_debug == YES_DEBUG)
									{
										printf("col_name = _%s_\n", col);
										if ((*select_node)->columns_arr[i]->table_ptr_type == PTR_TYPE_SELECT_NODE)
											printf("col_select_alias = _%s_\n", ((struct select_node*) (*select_node)->columns_arr[i]->table_ptr)->select_node_alias);
									}

									if (strcmp_Upper(name, col) == 0 && ((*select_node)->columns_arr[i]->table_ptr_type != PTR_TYPE_SELECT_NODE || ((*select_node)->columns_arr[i]->table_ptr_type == PTR_TYPE_SELECT_NODE && ((struct select_node*) (*select_node)->columns_arr[i]->table_ptr)->select_node_alias == NULL) ||
																		 ((*select_node)->columns_arr[i]->table_ptr_type == PTR_TYPE_SELECT_NODE && alias != NULL
																		  && ((struct select_node*) (*select_node)->columns_arr[i]->table_ptr)->select_node_alias != NULL
																		  && strcmp_Upper(alias, ((struct select_node*) (*select_node)->columns_arr[i]->table_ptr)->select_node_alias) == 0)))
									{
										(*select_node)->order_by->order_by_cols_tail->ptr_value = (*select_node)->columns_arr[i];
										(*select_node)->order_by->order_by_cols_tail->ptr_type = PTR_TYPE_COL_IN_SELECT_NODE;

										found = true;
										break;
									}
								}
								else if ((*select_node)->columns_arr[i]->func_node != NULL || (*select_node)->columns_arr[i]->math_node != NULL || (*select_node)->columns_arr[i]->case_node != NULL)
								{
									if (col != NULL && strcmp_Upper(word, col) == 0)
									{
										(*select_node)->order_by->order_by_cols_tail->ptr_value = (*select_node)->columns_arr[i];
										(*select_node)->order_by->order_by_cols_tail->ptr_type = PTR_TYPE_COL_IN_SELECT_NODE;

										found = true;
										break;
									}
								}
							}

							if (!found)
							{
								if (the_debug == YES_DEBUG)
									printf("	ERROR in parseSelect() at line %d in %s\n", __LINE__, __FILE__);
								printf("A column was declared in an order by clause, but it was not selected. Please select it. Failing word: \"%s\"\n", word);
								errorTeardown(NULL, malloced_head, the_debug);
								*select_node = NULL;
								return RETURN_ERROR;
							}

							if (alias != NULL)
								myFree((void**) &alias, NULL, malloced_head, the_debug);
							myFree((void**) &name, NULL, malloced_head, the_debug);
						}
						else
						{
							if (the_debug == YES_DEBUG)
								printf("	ERROR in parseSelect() at line %d in %s\n", __LINE__, __FILE__);
							printf("Failing word: \"%s\"\n", word);
							errorTeardown(NULL, malloced_head, the_debug);
							*select_node = NULL;
							return RETURN_ERROR;
						}
					}

					if (strcmp_Upper(word, "GROUP") == 0 ||
						strcmp_Upper(word, "HAVING") == 0 ||
						strcmp_Upper(word, "WHERE") == 0 ||
						strcmp_Upper(word, "ORDER") == 0)
					{
						if (the_debug == YES_DEBUG)
							printf("	ERROR in parseSelect() at line %d in %s\n", __LINE__, __FILE__);
						printf("Two where clauses, two group by clauses, two having clauses, or two order by clauses were declared. Failing word: \"%s\"\n", word);
						errorTeardown(NULL, malloced_head, the_debug);
						*select_node = NULL;
						return RETURN_ERROR;
					}

					if ((*select_node)->order_by->order_by_cols_tail->ptr_value == NULL)
					{
						if (the_debug == YES_DEBUG)
							printf("	ERROR in parseSelect() at line %d in %s\n", __LINE__, __FILE__);
						printf("Failing word: \"%s\"\n", word);
						errorTeardown(NULL, malloced_head, the_debug);
						*select_node = NULL;
						return RETURN_ERROR;
					}

					if ((*select_node)->order_by->order_by_cols_which_tail->ptr_value == NULL)
					{
						int* which = (int*) myMalloc(sizeof(int), NULL, malloced_head, the_debug);
						if (which == NULL)
						{
							if (the_debug == YES_DEBUG)
								printf("	ERROR in parseSelect() at line %d in %s\n", __LINE__, __FILE__);
							return RETURN_ERROR;
						}

						*which = ORDER_BY_ASC;

						(*select_node)->order_by->order_by_cols_which_tail->ptr_value = which;
						(*select_node)->order_by->order_by_cols_which_tail->ptr_type = PTR_TYPE_INT;
					}
				// END Parse order by
			}
			else if (strcmp_Upper(word, "HAVING") == 0)
			{
				// START Parse having clause
					char* having_clause = (char*) myMalloc(sizeof(char) * 2000, NULL, malloced_head, the_debug);
					if (having_clause == NULL)
					{
						if (the_debug == YES_DEBUG)
							printf("	ERROR in parseSelect() at line %d in %s\n", __LINE__, __FILE__);
						return RETURN_ERROR;
					}

					strcpy(having_clause, "HAVING ");

					while (getNextWord(input, word, &index) == 0 && strcmp(word, ";") != 0 &&
						   strcmp_Upper(word, "ORDER") != 0 &&
						   strcmp_Upper(word, "WHERE") != 0 &&
						   strcmp_Upper(word, "GROUP") != 0 &&
						   strcmp_Upper(word, "HAVING") != 0)
					{
						strcat(having_clause, word);

						if (input[index] == '.')
						{
							if (having_clause[strLength(having_clause)-1] == ' ')
								having_clause[strLength(having_clause)-1] = '.';
							else
								strcat(having_clause, ".");
							index++;
						}
						else
							strcat(having_clause, " ");
					}

					if (the_debug == YES_DEBUG)
						printf("having_clause = _%s_\n", having_clause);

					if (parseWhereClause(having_clause, &(*select_node)->having_head, *select_node, NULL, "having", malloced_head, the_debug) != RETURN_GOOD)
					{
						if (the_debug == YES_DEBUG)
							printf("	ERROR in parseSelect() at line %d in %s\n", __LINE__, __FILE__);
						*select_node = NULL;
						return RETURN_ERROR;
					}

					index -= strLength(word);

					if (strcmp_Upper(word, "WHERE") == 0 ||
						strcmp_Upper(word, "GROUP") == 0 ||
						strcmp_Upper(word, "HAVING") == 0)
					{
						if (the_debug == YES_DEBUG)
							printf("	ERROR in parseSelect() at line %d in %s\n", __LINE__, __FILE__);
						printf("Two where clauses, two group by clauses, or two having clauses were declared. Failing word: \"%s\"\n", word);
						errorTeardown(NULL, malloced_head, the_debug);
						*select_node = NULL;
						return RETURN_ERROR;
					}

					myFree((void**) &having_clause, NULL, malloced_head, the_debug);
				// END Parse having clause
			}
			else if (word[0] != ';' && word[0] != 0)
			{
				if (the_debug == YES_DEBUG)
					printf("	ERROR in parseSelect() at line %d in %s\n", __LINE__, __FILE__);
				printf("Failing word: \"%s\"\n", word);
				errorTeardown(NULL, malloced_head, the_debug);
				*select_node = NULL;
				return RETURN_ERROR;
			}
		}
		while (getNextWord(input, word, &index) == 0 && strcmp(word, ";") != 0);
	// END Parse clauses at the end


	myFree((void**) &word, NULL, malloced_head, the_debug);


	// START Free with_sub_select_head
		while (with_sub_select_head != NULL && !in_with)
		{
			struct ListNodePtr* temp = with_sub_select_head;

			with_sub_select_head = with_sub_select_head->next;

			if (the_debug == YES_DEBUG)
			{
				printf("temp alias = _%s_\n", ((struct select_node*) temp->ptr_value)->select_node_alias);
				printf("temp ptr_type = %d\n", temp->ptr_type);
			}

			struct select_node* temp_select = (struct select_node*) temp->ptr_value;
			while (temp_select->prev != NULL)
				temp_select = temp_select->prev;

			if (temp->ptr_type == -1)
			{
				freeAnyLinkedList((void**) &temp_select, PTR_TYPE_SELECT_NODE, NULL, malloced_head, the_debug);
			}

			myFree((void**) &temp, NULL, malloced_head, the_debug);
		}
	// END Free with_sub_select_head


	// START Check group by clause for 1. all non func cols in select are in group by clause 2. all cols in group by clause are in select
		struct func_node* a_func_node = NULL;

		for (int i=0; i<(*select_node)->columns_arr_size; i++)
		{
			if ((*select_node)->columns_arr[i]->func_node != NULL && (*select_node)->columns_arr[i]->func_node->which_func != FUNC_RANK)
			{
				a_func_node = (*select_node)->columns_arr[i]->func_node;
				break;
			}
			else if ((*select_node)->columns_arr[i]->math_node != NULL)
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
					int traversd = traverseTreeNode((void**) &(*select_node)->columns_arr[i]->math_node, PTR_TYPE_MATH_NODE, &ptr, &ptr_type, (void**) &cur_mirror
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
						a_func_node = ptr;
					}
				}

				if (a_func_node != NULL)
					break;
			}
			else if ((*select_node)->columns_arr[i]->case_node != NULL)
			{
				struct ListNodePtr* cur_then = (*select_node)->columns_arr[i]->case_node->case_then_value_head;
				while (cur_then != NULL)
				{
					if (cur_then->ptr_type == PTR_TYPE_FUNC_NODE)
					{
						a_func_node = cur_then->ptr_value;
						break;
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
								a_func_node = ptr;
							}
						}

						if (a_func_node != NULL)
							break;
					}

					cur_then = cur_then->next;
				}

				if (a_func_node != NULL)
					break;
			}
		}

		if (a_func_node != NULL)
		{
			// START Check group by clause for 1. all non func cols in select are in group by clause
				if (the_debug == YES_DEBUG)
					printf("Checking group by clause at end\n");
				for (int i=0; i<(*select_node)->columns_arr_size; i++)
				{
					if ((*select_node)->columns_arr[i]->func_node == NULL && (*select_node)->columns_arr[i]->math_node != NULL)
					{
						if (the_debug == YES_DEBUG)
							printf("Checking math_node at columns_arr index = %d\n", i);

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

						while (traverseTreeNode((void**) &(*select_node)->columns_arr[i]->math_node, PTR_TYPE_MATH_NODE, &ptr, &ptr_type, (void**) &cur_mirror
											   ,NULL, malloced_head, the_debug) == 0)
						{
							if (ptr_type == PTR_TYPE_COL_IN_SELECT_NODE)
							{
								if (the_debug == YES_DEBUG)
									printf("Found select_node col ptr in math_node\n");

								struct ListNodePtr* cur = a_func_node->group_by_cols_head;
								while (cur != NULL)
								{
									if (cur->ptr_value == ptr)
									{
										//printf("Got it\n");
										break;
									}

									cur = cur->next;
								}

								if (cur == NULL)
								{
									if (the_debug == YES_DEBUG)
										printf("	ERROR in parseSelect() at line %d in %s\n", __LINE__, __FILE__);
									printf("A function was declared, but not all selected columns appear in a group by clause.\n");
									errorTeardown(NULL, malloced_head, the_debug);
									*select_node = NULL;
									return RETURN_ERROR;
								}
							}
						}
					}
					else if ((*select_node)->columns_arr[i]->func_node == NULL && (*select_node)->columns_arr[i]->case_node != NULL)
					{
						if (the_debug == YES_DEBUG)
							printf("Checking case_node at columns_arr index = %d\n", i);

						struct ListNodePtr* cur_case_when = (*select_node)->columns_arr[i]->case_node->case_when_head;
						while (cur_case_when->ptr_value != NULL)
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

							while (traverseTreeNode((void**) &cur_case_when->ptr_value, PTR_TYPE_WHERE_CLAUSE_NODE, &ptr, &ptr_type, (void**) &cur_mirror
												   ,NULL, malloced_head, the_debug) == 0)
							{
								if (the_debug == YES_DEBUG)
									printf("ptr_type = %d\n", ptr_type);

								if (ptr_type == PTR_TYPE_COL_IN_SELECT_NODE)
								{
									if (the_debug == YES_DEBUG)
										printf("Found select_node col ptr in case_node\n");

									struct ListNodePtr* cur = a_func_node->group_by_cols_head;
									while (cur != NULL)
									{
										if (cur->ptr_value == ptr)
										{
											if (the_debug == YES_DEBUG)
												printf("Got it\n");
											break;
										}

										cur = cur->next;
									}

									if (cur == NULL)
									{
										if (the_debug == YES_DEBUG)
											printf("	ERROR in parseSelect() at line %d in %s\n", __LINE__, __FILE__);
										printf("A function was declared, but not all selected columns appear in a group by clause.\n");
										errorTeardown(NULL, malloced_head, the_debug);
										*select_node = NULL;
										return RETURN_ERROR;
									}
								}
							}

							cur_case_when = cur_case_when->next;
						}

					}
					else if ((*select_node)->columns_arr[i]->func_node == NULL)
					{
						if (the_debug == YES_DEBUG)
							printf("Checking regular col at columns_arr index = %d\n", i);

						struct ListNodePtr* cur = a_func_node->group_by_cols_head;
						while (cur != NULL)
						{
							if (cur->ptr_value == (*select_node)->columns_arr[i]->col_ptr)
								break;

							cur = cur->next;
						}

						if (cur == NULL)
						{
							if (the_debug == YES_DEBUG)
								printf("	ERROR in parseSelect() at line %d in %s\n", __LINE__, __FILE__);
							printf("A function was declared, but not all selected columns appear in a group by clause.\n");
							errorTeardown(NULL, malloced_head, the_debug);
							*select_node = NULL;
							return RETURN_ERROR;
						}
					}
				}
			// END Check group by clause for 1. all non func cols in select are in group by clause

			// START Check group by clause for 2. all cols in group by clause are in select
				if (the_debug == YES_DEBUG)
					printf("Checking group by clause at end part 2\n");
				struct ListNodePtr* cur = a_func_node->group_by_cols_head;
				while (cur != NULL)
				{
					bool found = false;

					for (int i=0; i<(*select_node)->columns_arr_size; i++)
					{
						if ((*select_node)->columns_arr[i]->func_node == NULL && (*select_node)->columns_arr[i]->math_node != NULL)
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

							while (traverseTreeNode((void**) &(*select_node)->columns_arr[i]->math_node, PTR_TYPE_MATH_NODE, &ptr, &ptr_type, (void**) &cur_mirror
												   ,NULL, malloced_head, the_debug) == 0)
							{
								if (ptr_type == PTR_TYPE_COL_IN_SELECT_NODE && cur->ptr_value == ptr)
								{
									found = true;
								}
							}

							if (found)
								break;
						}
						else if ((*select_node)->columns_arr[i]->func_node == NULL && (*select_node)->columns_arr[i]->case_node != NULL)
						{
							struct ListNodePtr* cur_case_when = (*select_node)->columns_arr[i]->case_node->case_when_head;
							while (cur_case_when->ptr_value != NULL)
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

								while (traverseTreeNode((void**) &cur_case_when->ptr_value, PTR_TYPE_WHERE_CLAUSE_NODE, &ptr, &ptr_type, (void**) &cur_mirror
													   ,NULL, malloced_head, the_debug) == 0)
								{
									if (the_debug == YES_DEBUG)
										printf("ptr_type = %d\n", ptr_type);

									if (ptr_type == PTR_TYPE_COL_IN_SELECT_NODE && cur->ptr_value == ptr)
									{
										found = true;
									}
								}

								if (found)
									break;

								cur_case_when = cur_case_when->next;
							}

							if (found)
								break;
						}
						else if ((*select_node)->columns_arr[i]->func_node == NULL)
						{
							if (cur->ptr_value == (*select_node)->columns_arr[i]->col_ptr)
							{
								found = true;
								break;
							}
						}
					}

					if (!found)
						break;

					cur = cur->next;
				}

				if (cur != NULL)
				{
					if (the_debug == YES_DEBUG)
						printf("	ERROR in parseSelect() at line %d in %s\n", __LINE__, __FILE__);
					printf("A function and a group by clause were declared, but not all columns in the group by clause are selected.\n");
					errorTeardown(NULL, malloced_head, the_debug);
					*select_node = NULL;
					return RETURN_ERROR;
				}
			// END Check group by clause for 2. all cols in group by clause are in select
		}
		else if (the_debug == YES_DEBUG)
			printf("Did not check group by\n");
	// END Check group by clause for 1. all non func cols in select are in group by clause 2. all cols in group by clause are in select


	// START Check for duplicate column names
		for (int j=0; j<(*select_node)->columns_arr_size; j++)
		{
			for (int jj=0; jj<(*select_node)->columns_arr_size; jj++)
			{
				if (j != jj)
				{
					//printf("Comparing col at index = %d for duplicate name with col at index = %d\n", j, jj);
					char* j_col_name = NULL;

					if ((*select_node)->columns_arr[j]->new_name != NULL)
						j_col_name = (*select_node)->columns_arr[j]->new_name;
					else
					{
						struct col_in_select_node* cur = (*select_node)->columns_arr[j];
						while (cur != NULL && (cur->col_ptr_type == PTR_TYPE_COL_IN_SELECT_NODE || cur->func_node != NULL || cur->math_node != NULL || cur->case_node != NULL))
						{
							//printf("Finding name of col: _%s_, _%s_\n", cur->col_ptr != NULL ? "valid col_ptr" : "null col_ptr", cur->new_name);
							if (cur->new_name != NULL)
							{
								j_col_name = cur->new_name;
								break;
							}
							cur = cur->col_ptr;
						}
						
						if (cur->col_ptr_type == PTR_TYPE_TABLE_COLS_INFO)
							j_col_name = ((struct table_cols_info*) cur->col_ptr)->col_name;
						else if (j_col_name == NULL)
						{
							if (the_debug == YES_DEBUG)
								printf("	ERROR in parseSelect() at line %d in %s\n", __LINE__, __FILE__);
							errorTeardown(NULL, malloced_head, the_debug);
							*select_node = NULL;
							return RETURN_ERROR;
						}
					}

					char* jj_col_name = NULL;

					if ((*select_node)->columns_arr[jj]->new_name != NULL)
						jj_col_name = (*select_node)->columns_arr[jj]->new_name;
					else
					{
						struct col_in_select_node* cur = (*select_node)->columns_arr[jj];
						while (cur != NULL && (cur->col_ptr_type == PTR_TYPE_COL_IN_SELECT_NODE || cur->func_node != NULL || cur->math_node != NULL || cur->case_node != NULL))
						{
							//printf("Finding name of col: _%s_, _%s_\n", cur->col_ptr != NULL ? "valid col_ptr" : "null col_ptr", cur->new_name);
							if (cur->new_name != NULL)
							{
								jj_col_name = cur->new_name;
								break;
							}
							cur = cur->col_ptr;
						}
						
						if (cur->col_ptr_type == PTR_TYPE_TABLE_COLS_INFO)
							jj_col_name = ((struct table_cols_info*) cur->col_ptr)->col_name;
						else if (jj_col_name == NULL)
						{
							if (the_debug == YES_DEBUG)
								printf("	ERROR in parseSelect() at line %d in %s\n", __LINE__, __FILE__);
							errorTeardown(NULL, malloced_head, the_debug);
							*select_node = NULL;
							return RETURN_ERROR;
						}
					}

					//printf("j_col_name = _%s_, jj_col_name = _%s_\n", j_col_name, jj_col_name);
					if (strcmp_Upper(j_col_name, jj_col_name) == 0)
					{
						if (the_debug == YES_DEBUG)
							printf("	ERROR in parseSelect() at line %d in %s\n", __LINE__, __FILE__);
						printf("Two columns were declared which will have the same name. Please provide different names. Failing column names: \"%s\" and \"%s\".\n", j_col_name, jj_col_name);
						errorTeardown(NULL, malloced_head, the_debug);
						*select_node = NULL;
						return RETURN_ERROR;
					}
				}
			}
		}		
	// END Check for duplicate column names


	// START Traverse to head of list which is the select_node which must be executed first
		while ((*select_node)->prev != NULL)
			*select_node = (*select_node)->prev;
	// END Traverse to head of list which is the select_node which must be executed first

	return RETURN_GOOD;
}