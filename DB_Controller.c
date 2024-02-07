#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stddef.h>
#include <inttypes.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <assert.h>
#include <time.h>
#include <fcntl.h>
#include <sys/file.h>
#include <errno.h>
#include <math.h>
#include "DB_Driver.h"
#include "DB_Controller.h"
#include "DB_HelperFunctions.h"
#include "DB_Tests.h"

/*
int createTableFromCSV(char* input, char* table_name, int_8 num_rows
					  ,struct malloced_node** malloced_head, int the_debug)
{
	char* table_name_copy = (char*) myMalloc(sizeof(char) * 32, NULL, malloced_head, the_debug);
	if (table_name_copy == NULL)
	{
		if (the_debug == YES_DEBUG)
			printf("	ERROR in createTableFromCSV() at line %d in %s\n", __LINE__, __FILE__);
		return -1;
	}
	strcpy(table_name_copy, table_name);


	struct file_opened_node* file_opened_head = NULL;
	
	// START Check if input file exists
    if (access(input, F_OK) != 0)
    {
    	if (the_debug == YES_DEBUG)
			printf("	ERROR in createTableFromCSV() at line %d in %s\n", __LINE__, __FILE__);
		errorTeardown(&file_opened_head, malloced_head, the_debug);
		return -1;
    }
    // END Check if input file exists

	// START Open csv file and read in first row of column names and datatypes
    FILE* file = myFileOpenSimple(input, "r", &file_opened_head, malloced_head, the_debug);
	if (file == NULL)
	{
		if (the_debug == YES_DEBUG)
			printf("	ERROR in createTableFromCSV() at line %d in %s\n", __LINE__, __FILE__);
		return -1;
	}

	char* first_row = (char*) myMalloc(sizeof(char) * 1000, &file_opened_head, malloced_head, the_debug);
	if (first_row == NULL)
	{
		if (the_debug == YES_DEBUG)
			printf("	ERROR in createTableFromCSV() at line %d in %s\n", __LINE__, __FILE__);
		return -2;
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
		return -2;
	}
	struct table_cols_info* cur_col = table_cols;
    // END Allocate space for beginning of table_cols
	
	// START Split first_row by each comma and extract column name and datatype
    int num_cols = 0;
	char** col_names = strSplitV2(first_row, ',', &num_cols, &file_opened_head, malloced_head, the_debug);
	if (myFree((void**) &first_row, &file_opened_head, malloced_head, the_debug) != 0)
	{
		if (the_debug == YES_DEBUG)
			printf("	ERROR in createTableFromCSV() at line %d in %s\n", __LINE__, __FILE__);
		return -2;
	}

	for (int i=0; i<num_cols; i++)
	{
		char* temp_but_is_col_name = substring(col_names[i], 0, indexOf(col_names[i], ':')-1, &file_opened_head, malloced_head, the_debug);
		char* temp2 = substring(col_names[i], indexOf(col_names[i], ':')+1, indexOf(col_names[i], 0), &file_opened_head, malloced_head, the_debug);
		
		// START If null terminator is at index > 32 (col name is longer than 31 chars), throw error, return
		if (indexOf(temp_but_is_col_name, 0) > 32)
		{
			if (the_debug == YES_DEBUG)
				printf("	ERROR in createTableFromCSV() at line %d in %s\n", __LINE__, __FILE__);
			return -11;
		}
		// END If null terminator is at index > 32 (col name is longer than 31 chars), throw error, return
		
		cur_col->col_name = temp_but_is_col_name;
		cur_col->col_number = i;

		//printf("cur_col->col_name = %s\n", cur_col->col_name);
		//printf("cur_col->col_number = %d\n", cur_col->col_number);
		
		char* datatype = substring(temp2, 0, 3, &file_opened_head, malloced_head, the_debug);
		if (datatype == NULL)
		{
			if (the_debug == YES_DEBUG)
				printf("	ERROR in createTableFromCSV() at line %d in %s\n", __LINE__, __FILE__);
			return -2;
		}

		char* upper_datatype = upper(datatype, NULL, malloced_head, the_debug);
		if (upper_datatype == NULL)
		{
			if (the_debug == YES_DEBUG)
				printf("	ERROR in createTableFromCSV() at line %d in %s\n", __LINE__, __FILE__);
			return -2;
		}


		if (strcmp(upper_datatype, "INTE") == 0)
		{
			cur_col->data_type = 1;
			cur_col->max_length = 8;
		}
		else if (strcmp(upper_datatype, "REAL") == 0)
		{
			cur_col->data_type = 2;
			cur_col->max_length = 8;
		}
		else if (strcmp(upper_datatype, "CHAR") == 0)
		{
			cur_col->data_type = 3;
			sscanf(temp2, "%*[^(](%d)", &cur_col->max_length);
			if (cur_col->max_length == 0)
			{
				if (the_debug == YES_DEBUG)
					printf("	ERROR in createTableFromCSV() at line %d in %s\n", __LINE__, __FILE__);
				return -12;
			}
			cur_col->max_length++;
		}
		else if (strcmp(upper_datatype, "DATE") == 0)
		{
			cur_col->data_type = 4;
			cur_col->max_length = 8;
		}
		else
		{
			if (the_debug == YES_DEBUG)
				printf("	ERROR in createTableFromCSV() at line %d in %s\n", __LINE__, __FILE__);
			return -12;
		}

		myFree((void**) &temp2, &file_opened_head, malloced_head, the_debug);
		myFree((void**) &datatype, &file_opened_head, malloced_head, the_debug);
		myFree((void**) &upper_datatype, &file_opened_head, malloced_head, the_debug);
		myFree((void**) &col_names[i], &file_opened_head, malloced_head, the_debug);

		// START Allocate another struct table_cols_info is more cols, else, make next null
		if (i < num_cols-1)
		{
			cur_col->next = (struct table_cols_info*) myMalloc(sizeof(struct table_cols_info), &file_opened_head, malloced_head, the_debug);
			if (cur_col->next == NULL)
			{
				if (the_debug == YES_DEBUG)
					printf("	ERROR in createTableFromCSV() at line %d in %s\n", __LINE__, __FILE__);
				return -2;
			}
			cur_col = cur_col->next;
		}
		else
			cur_col->next = NULL;
		// END Allocate another struct table_cols_info is more cols, else, make next null
	}
	if (myFree((void**) &col_names, &file_opened_head, malloced_head, the_debug) != 0)
	{
		if (the_debug == YES_DEBUG)
			printf("	ERROR in createTableFromCSV() at line %d in %s\n", __LINE__, __FILE__);
		return -2;
	}
    // END Split first_row by each comma and extract column name and datatype


	// START Call createTable with parsed table_cols
	int created_table = createTable(table_name, table_cols, malloced_head, the_debug);
	if (created_table == -1 || created_table == -2)
	{
		if (created_table == -1)
			printf("Table creation had a problem with file i/o\n");
		else
			printf("Table creation had a problem with malloc\n");
        return -13;
	}
	else
	{
		if (the_debug == YES_DEBUG)
			printf("Successfully created table\n");
	}
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
		return -14;
	}
    // END Find table in tables_head

	// START Reopen file
	//FILE* 
	file = myFileOpenSimple(input, "r", &file_opened_head, malloced_head, the_debug);
	first_row = (char*) myMalloc(sizeof(char) * 1000, &file_opened_head, malloced_head, the_debug);
	if (first_row == NULL)
	{
		if (the_debug == YES_DEBUG)
			printf("	ERROR in createTableFromCSV() at line %d in %s\n", __LINE__, __FILE__);
		return -2;
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
        return -2;
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
		char* a_line = (char*) myMalloc(sizeof(char) * 1000, &file_opened_head, malloced_head, the_debug);
		if (a_line == NULL)
		{
			if (the_debug == YES_DEBUG)
				printf("	ERROR in createTableFromCSV() at line %d in %s\n", __LINE__, __FILE__);
	        return -2;
		}
		fscanf(file, "%[^\n]\n", a_line);
		//printf("a_line = %s\n", a_line);

		for (int j=0; j<table->num_cols; j++)
		{
			// START Get value after certain amount of cols, using format
			char* format = (char*) myMalloc(sizeof(char) * 200, &file_opened_head, malloced_head, the_debug);
			if (format == NULL)
			{
				if (the_debug == YES_DEBUG)
					printf("	ERROR in createTableFromCSV() at line %d in %s\n", __LINE__, __FILE__);
				return -2;
			}
			strcpy(format, "");
			for (int k=0; k<j; k++)
           		strcat(format, "%*[^,],");
			strcat(format, "%[^,],%*[^\n]\n");

			//printf("format = _%s_\n", format);

			char* data = (char*) myMalloc(sizeof(char) * 300, &file_opened_head, malloced_head, the_debug);
            if (data == NULL)
			{
				if (the_debug == YES_DEBUG)
					printf("	ERROR in createTableFromCSV() at line %d in %s\n", __LINE__, __FILE__);
				return -2;
			}

			sscanf(a_line, format, data);
			// END Get value after certain amount of cols, using format

			if (strcmp(data, "") == 0)
			{
				if (the_debug == YES_DEBUG)
					printf("	ERROR in createTableFromCSV() at line %d in %s\n", __LINE__, __FILE__);
				errorTeardown(&file_opened_head, malloced_head, the_debug);
				return -15;
			}

            int_8 data_int_date = 0;
            double data_real = 0.0;
            
            if (cur_col->data_type == 1)
				sscanf(data, "%lu", &data_int_date);
            else if (cur_col->data_type == 2)
                sscanf(data, "%f", &data_real);
            else if (cur_col->data_type == 4)
                data_int_date = dateToInt(data);
			
            //if (cur_col->data_type == 1)
			//	printf("data = %lu\n", data_int_date);
            //else if (cur_col->data_type == 2)
            //    printf("data = %f\n", data_real);
            //else if (cur_col->data_type == 4)
			//{
			//	data_int_date = dateToInt(data);
			//	printf("data = %lu\n", data_int_date);
			//}
            //else
			//	printf("data = %s\n", data);
			
            
            if (insertAppend(col_data_info_file_arr, col_data_file_arr, &file_opened_head
                            ,table->file_number, cur_col
                            ,data_int_date, data_real, data
                            ,malloced_head, the_debug) != 0)
            {
                if (the_debug == YES_DEBUG)
					printf("	ERROR in createTableFromCSV() at line %d in %s\n", __LINE__, __FILE__);
				return -16;
            }

            //printf("cur_col->num_rows = %d\n", cur_col->num_rows);

			cur_col = cur_col->next;

            myFree((void**) &data, &file_opened_head, malloced_head, the_debug);
            myFree((void**) &format, &file_opened_head, malloced_head, the_debug);
		}
		myFree((void**) &a_line, &file_opened_head, malloced_head, the_debug);
	}
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
    struct table_info* cur_table = getTablesHead();
    while (cur_table != NULL)
    {
    	if (strcmp(cur_table->name, table_name_copy) == 0)
    	{
			initFrequentLists(cur_table, malloced_head, the_debug);
    		break;
    	}

    	cur_table = cur_table->next;    	
    }
    
    // myFreeAllCleanup() is called by createTable so this pointer's malloced node will be freed but not its data pointer
    free(table_name_copy);


    if (the_debug == YES_DEBUG)
		printf("Calling myFreeAllCleanup() from createTableFromCSV(), but NOT freeing ptrs for tables_head->frequent_lists\n");
	myFreeAllCleanup(malloced_head, the_debug);
    // END Initialize frequent lists after table creation and data insertion

    return table->table_cols_head->num_rows;
}

int displayResultsOfSelect(struct colDataNode*** result, struct table_info* the_table, int_8* col_numbers, int col_numbers_size, int_8 num_rows
						  ,struct malloced_node** malloced_head, int the_debug)
{
	struct file_opened_node* file_opened_head = NULL;

	// START Create .csv file, deleting old if necessary
	if (access("Results.csv", F_OK) == 0)
		remove("Results.csv");
	FILE* file = myFileOpenSimple("Results.csv", "a", &file_opened_head, malloced_head, the_debug);
	if (file == NULL)
	{
		if (the_debug == YES_DEBUG)
			printf("	ERROR in displayResultsOfSelect() at line %d in %s\n", __LINE__, __FILE__);
		return -1;
	}
	// END Create .csv file, deleting old if necessary

	// START Write column names
	for (int j=0; j<col_numbers_size; j++)
	{
		struct table_cols_info* cur_col = the_table->table_cols_head;
		while (cur_col != NULL)
		{
			if (cur_col->col_number == col_numbers[j])
				break;
			cur_col = cur_col->next;
		}

		if (fwrite(cur_col->col_name, strLength(cur_col->col_name), 1, file) != 1)
		{
			if (the_debug == YES_DEBUG)
				printf("	ERROR in displayResultsOfSelectAndFree() at line %d in %s\n", __LINE__, __FILE__);
			errorTeardown(&file_opened_head, malloced_head, the_debug);
			return -1;
		}

		if (j != col_numbers_size-1)
		{
			if (fwrite(",", 1, 1, file) != 1)
			{
				if (the_debug == YES_DEBUG)
					printf("	ERROR in displayResultsOfSelectAndFree() at line %d in %s\n", __LINE__, __FILE__);
				errorTeardown(&file_opened_head, malloced_head, the_debug);
				return -1;
			}
		}
		else
		{
			if (fwrite("\n", 1, 1, file) != 1)
			{
				if (the_debug == YES_DEBUG)
					printf("	ERROR in displayResultsOfSelectAndFree() at line %d in %s\n", __LINE__, __FILE__);
				errorTeardown(&file_opened_head, malloced_head, the_debug);
				return -1;
			}
		}
	}
	// END Write column names

	// START Write column data
	for (int i=0; i<num_rows; i++)
	{
		for (int j=0; j<col_numbers_size; j++)
		{
			if (strcmp(result[j][i]->row_data, "") != 0)
			{
				if (fwrite(result[j][i]->row_data, strLength(result[j][i]->row_data), 1, file) != 1)
				{
					if (the_debug == YES_DEBUG)
					{
						printf("	ERROR in displayResultsOfSelectAndFree() at line %d in %s\n", __LINE__, __FILE__);
						printf("	result[j][i]->row_data = _%s_\n", result[j][i]->row_data);
					}
					errorTeardown(&file_opened_head, malloced_head, the_debug);
					return -1;
				}
			}
				

			if (j != col_numbers_size-1)
			{
				if (fwrite(",", 1, 1, file) != 1)
				{
					if (the_debug == YES_DEBUG)
						printf("	ERROR in displayResultsOfSelectAndFree() at line %d in %s\n", __LINE__, __FILE__);
					errorTeardown(&file_opened_head, malloced_head, the_debug);
					return -1;
				}
			}
			else
			{
				if (fwrite("\n", 1, 1, file) != 1)
				{
					if (the_debug == YES_DEBUG)
						printf("	ERROR in displayResultsOfSelectAndFree() at line %d in %s\n", __LINE__, __FILE__);
					errorTeardown(&file_opened_head, malloced_head, the_debug);
					return -1;
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

	return 0;
}


int printLastWordBeforeError(char* error_word)
{
	printf("Error at word: %s,", error_word);

	return 0;
}*/

int getColInSelectNodeFromName(void* put_ptrs_here, int put_ptrs_here_type, int math_node_which, int i
							  ,struct select_node* select_node, char* cur_col_alias, char* cur_col_name
							  ,struct malloced_node** malloced_head, int the_debug)
{
	printf("getColInSelectNodeFromName is finding _%s.%s_\n", cur_col_alias, cur_col_name);

	bool found = false;

	// START Looking in from select_node (*select_node)->prev
		for (int j=0; j<select_node->prev->columns_arr_size; j++)
		{
			struct col_in_select_node* cur = select_node->prev->columns_arr[j];
			while (cur->col_ptr_type == PTR_TYPE_COL_IN_SELECT_NODE)
				cur = cur->col_ptr;

			char* col_name = ((struct table_cols_info*) cur->col_ptr)->col_name;

			//printf("checking col_name = _%s_\n", col_name);

			char* table_name = ((struct table_info*) cur->table_ptr)->name;

			//printf("table_name = _%s_\n", table_name);

			if (strcmp(cur_col_name, col_name) == 0)
			{
				if ((cur_col_alias == NULL && select_node->join_head == NULL)
					|| (cur_col_alias != NULL && select_node->prev->select_node_alias != NULL && strcmp(cur_col_alias, select_node->prev->select_node_alias) == 0)
					|| (cur_col_alias != NULL && strcmp_Upper(cur_col_alias, table_name, NULL, malloced_head, the_debug) == 0))
				{
					//printf("Found it: cur_col_name = _%s_ and found _%s_\n", cur_col_name, col_name);
					printf("Found it\n");
					found = true;

					if (i == 0)
					{
						return 0;
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

						return 0;
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

					char* col_name = ((struct table_cols_info*) cur->col_ptr)->col_name;
					
					//printf("checking (in join) col_name = _%s_\n", col_name);

					char* table_name = ((struct table_info*) cur->table_ptr)->name;

					//printf("table_name = _%s_\n", table_name);

					if (strcmp(cur_col_name, col_name) == 0)
					{
						if ((cur_col_alias == NULL && cur_join == NULL) 
							|| (cur_col_alias != NULL && cur_join->select_joined->select_node_alias != NULL && strcmp(cur_col_alias, cur_join->select_joined->select_node_alias) == 0)
							|| (cur_col_alias != NULL && strcmp_Upper(cur_col_alias, table_name, NULL, malloced_head, the_debug) == 0))
						{
							printf("Found it\n");
							found = true;

							if (i == 0)
							{
								return 0;
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

								return 0;
							}
						}
					}
				}

				cur_join = cur_join->next;
			}
		}
	// END Looking in all joined nodes select_node->join_head->select_joined

	if (!found && cur_join == NULL)
	{
		// Column does not have an alias, and there are joined tables
		// Or, column and alias combo could not be found in valid set of columns
		if (the_debug == YES_DEBUG)
			printf("	ERROR in getColInSelectNodeFromName() at line %d in %s\n", __LINE__, __FILE__);
		return -1;
	}

	return 0;
}

int parseOneWhereNode(char* input, char* word, struct where_clause_node** where_node, struct select_node* select_node
					 ,struct malloced_node** malloced_head, int the_debug)
{
	int index = 0;

	char* prev_word = NULL;

	while(getNextWord(input, word, &index) == 0 && word[0] != ';')
	{
		if (input[index] == '.')
		{
			// START Ensure that word has alias.name
				char* temp_word = (char*) myMalloc(sizeof(char) * 200, NULL, malloced_head, the_debug);
				strcpy(temp_word, word);

				strcat(temp_word, ".");
				index++;

				getNextWord(input, word, &index);

				strcat(temp_word, word);

				strcpy(word, temp_word);

				myFree((void**) &temp_word, NULL, malloced_head, the_debug);
			// END Ensure that word has alias.name
		}

		printf("word in parseOneWhereNode = _%s_\n", word);

		if ((word[0] == '+' || word[0] == '-' || word[0] == '*' || word[0] == '/' || word[0] == '^') || (prev_word == NULL && word[0] == '('))
		{
			// START Parse math node in input
				printf("Found math in parsing where node, prev_word = _%s_\n", prev_word);

				struct math_node* new_math_node = NULL;

				if (parseMathNode(&new_math_node, NULL, &prev_word, input, word, index, select_node
								 ,malloced_head, the_debug) != 0)
				{
					*where_node = NULL;
					return -1;
				}

				printf("Back from parseMathNode()\n");

				getNextWord(input, word, &index);
				while (word[0] != '=' && word[0] != '>' && word[0] != '<')
				{
					getNextWord(input, word, &index);
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
			// END Parse math node in input
		}
		else if (word[0] == '=' || word[0] == '>' || word[0] == '<' || strcmp_Upper(word, "IS", NULL, malloced_head, the_debug) == 0)
		{
			// START Label where_type according to word
				if ((*where_node)->where_type != -1)
				{
					if (the_debug == YES_DEBUG)
						printf("	ERROR in parseOneWhereNode() at line %d in %s\n", __LINE__, __FILE__);
					errorTeardown(NULL, malloced_head, the_debug);
					*where_node = NULL;
					return -1;
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
				else if (strcmp_Upper(word, "IS", NULL, malloced_head, the_debug) == 0)
				{
					getNextWord(input, word, &index);

					if (strcmp_Upper(word, "NULL", NULL, malloced_head, the_debug) == 0)
					{
						(*where_node)->where_type = WHERE_IS_NULL;
					}
					else if (strcmp_Upper(word, "NOT", NULL, malloced_head, the_debug) == 0)
					{
						getNextWord(input, word, &index);

						if (strcmp_Upper(word, "NULL", NULL, malloced_head, the_debug) == 0)
						{
							(*where_node)->where_type = WHERE_IS_NOT_NULL;
						}
						else
						{
							if (the_debug == YES_DEBUG)
								printf("	ERROR in parseOneWhereNode() at line %d in %s\n", __LINE__, __FILE__);
							errorTeardown(NULL, malloced_head, the_debug);
							*where_node = NULL;
							return -1;
						}
					}
					else
					{
						if (the_debug == YES_DEBUG)
							printf("	ERROR in parseOneWhereNode() at line %d in %s\n", __LINE__, __FILE__);
						errorTeardown(NULL, malloced_head, the_debug);
						*where_node = NULL;
						return -1;
					}
				}
			// END Label where_type according to word
		}
		else if ((*where_node)->ptr_one != NULL && (*where_node)->ptr_two == NULL)
		{
			// START Determine type of next word and put into ptr_two
				//printf("	Assigning to ptr_two\n");

				if (word[0] == 39)
				{
					(*where_node)->ptr_two = substring(word, 1, strLength(word)-2, NULL, malloced_head, the_debug);

					if ((*where_node)->ptr_two == NULL)
					{
						if (the_debug == YES_DEBUG)
							printf("	ERROR in parseOneWhereNode() at line %d in %s\n", __LINE__, __FILE__);
						*where_node = NULL;
						return -1;
					}

					if (isDate((char*) (*where_node)->ptr_two))
					{
						(*where_node)->ptr_two_type = PTR_TYPE_DATE;

						// START Check if matching datatype
						if ((*where_node)->ptr_one_type != PTR_TYPE_DATE
							&& ((*where_node)->ptr_one_type == PTR_TYPE_COL_IN_SELECT_NODE && ((struct table_cols_info*) ((struct col_in_select_node*) (*where_node)->ptr_one)->col_ptr)->data_type != DATA_DATE))
						{
							if (the_debug == YES_DEBUG)
								printf("	ERROR in parseOneWhereNode() at line %d in %s\n", __LINE__, __FILE__);
							errorTeardown(NULL, malloced_head, the_debug);
							*where_node = NULL;
							return -1;
						}
						// END Check if matching datatype
					}
					else
					{
						(*where_node)->ptr_two_type = PTR_TYPE_CHAR;

						// START Check if matching datatype
						if ((*where_node)->ptr_one_type != PTR_TYPE_CHAR
							&& ((*where_node)->ptr_one_type == PTR_TYPE_COL_IN_SELECT_NODE && ((struct table_cols_info*) ((struct col_in_select_node*) (*where_node)->ptr_one)->col_ptr)->data_type != DATA_STRING))
						{
							if (the_debug == YES_DEBUG)
								printf("	ERROR in parseOneWhereNode() at line %d in %s\n", __LINE__, __FILE__);
							errorTeardown(NULL, malloced_head, the_debug);
							*where_node = NULL;
							return -1;
						}
						// END Check if matching datatype
					}
				}
				else
				{
					int hmm_int = atoi(word);

					if (hmm_int > 0 && strcontains(word, '.'))
					{
						// Double
						(*where_node)->ptr_two = myMalloc(sizeof(double), NULL, malloced_head, the_debug);

						double hmm_double = 0.0;

						sscanf(word, "%lf", &hmm_double);

						*((double*) (*where_node)->ptr_two) = hmm_double;

						(*where_node)->ptr_two_type = PTR_TYPE_REAL;

						// START Check if matching datatype
						if ((*where_node)->ptr_one_type != PTR_TYPE_REAL
							&& ((*where_node)->ptr_one_type == PTR_TYPE_COL_IN_SELECT_NODE && ((struct table_cols_info*) ((struct col_in_select_node*) (*where_node)->ptr_one)->col_ptr)->data_type != DATA_REAL))
						{
							if (the_debug == YES_DEBUG)
								printf("	ERROR in parseOneWhereNode() at line %d in %s\n", __LINE__, __FILE__);
							errorTeardown(NULL, malloced_head, the_debug);
							*where_node = NULL;
							return -1;
						}
						// END Check if matching datatype
					}
					else if (hmm_int > 0)
					{
						// Int
						(*where_node)->ptr_two = myMalloc(sizeof(int), NULL, malloced_head, the_debug);

						*((int*) (*where_node)->ptr_two) = hmm_int;

						(*where_node)->ptr_two_type = PTR_TYPE_INT;

						// START Check if matching datatype
						if ((*where_node)->ptr_one_type != PTR_TYPE_INT
							&& ((*where_node)->ptr_one_type == PTR_TYPE_COL_IN_SELECT_NODE && ((struct table_cols_info*) ((struct col_in_select_node*) (*where_node)->ptr_one)->col_ptr)->data_type != DATA_INT))
						{
							if (the_debug == YES_DEBUG)
								printf("	ERROR in parseOneWhereNode() at line %d in %s\n", __LINE__, __FILE__);
							errorTeardown(NULL, malloced_head, the_debug);
							*where_node = NULL;
							return -1;
						}
						// END Check if matching datatype
					}
					else
					{
						// Char but column name, so get column
						char* alias = NULL;
						char* name = upper(word, NULL, malloced_head, the_debug);

						int index_of_dot = indexOf(name, '.');
						if (index_of_dot > -1)
						{
							alias = substring(name, 0, index_of_dot-1, NULL, malloced_head, the_debug);

							char* new_name = substring(name, index_of_dot+1, strLength(name)-1, NULL, malloced_head, the_debug);

							myFree((void**) &name, NULL, malloced_head, the_debug);

							name = new_name;
						}
						printf("alias.name = _%s.%s_\n", alias, name);

						if (getColInSelectNodeFromName((void*) (*where_node), PTR_TYPE_WHERE_CLAUSE_NODE, 2, 1, select_node, alias, name
													  ,malloced_head, the_debug) != 0)
						{
							errorTeardown(NULL, malloced_head, the_debug);
							*where_node = NULL;
							return -1;
						}

						if (alias != NULL)
							myFree((void**) &alias, NULL, malloced_head, the_debug);
						myFree((void**) &name, NULL, malloced_head, the_debug);

						// START Check if found col has valid datatype
						if ((*where_node)->ptr_one_type != PTR_TYPE_COL_IN_SELECT_NODE && ((struct table_cols_info*) ((struct col_in_select_node*) (*where_node)->ptr_two)->col_ptr)->data_type != (*where_node)->ptr_one_type)
						{
							if (the_debug == YES_DEBUG)
								printf("	ERROR in parseOneWhereNode() at line %d in %s\n", __LINE__, __FILE__);
							errorTeardown(NULL, malloced_head, the_debug);
							*where_node = NULL;
							return -1;
						}

						if ((*where_node)->ptr_one_type == PTR_TYPE_COL_IN_SELECT_NODE)
						{
							struct col_in_select_node* cur_one = (*where_node)->ptr_one;
							while (cur_one->col_ptr_type == PTR_TYPE_COL_IN_SELECT_NODE)
								cur_one = cur_one->col_ptr;

							struct col_in_select_node* cur_two = (*where_node)->ptr_two;
							while (cur_two->col_ptr_type == PTR_TYPE_COL_IN_SELECT_NODE)
								cur_two = cur_two->col_ptr;

							//printf("First data_type = %d\n", ((struct table_cols_info*) cur_one->col_ptr)->data_type);
							//printf("Second data_type = %d\n", ((struct table_cols_info*) cur_two->col_ptr)->data_type);

							if (((struct table_cols_info*) cur_one->col_ptr)->data_type != ((struct table_cols_info*) cur_two->col_ptr)->data_type)
							{
								if (the_debug == YES_DEBUG)
									printf("	ERROR in parseOneWhereNode() at line %d in %s\n", __LINE__, __FILE__);
								errorTeardown(NULL, malloced_head, the_debug);
								*where_node = NULL;
								return -1;
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
				//printf("	Assigning to ptr_one\n");

				if (word[0] == 39)
				{
					(*where_node)->ptr_one = substring(word, 1, strLength(word)-2, NULL, malloced_head, the_debug);

					if ((*where_node)->ptr_one == NULL)
					{
						if (the_debug == YES_DEBUG)
							printf("	ERROR in parseOneWhereNode() at line %d in %s\n", __LINE__, __FILE__);
						*where_node = NULL;
						return -1;
					}

					if (isDate((char*) (*where_node)->ptr_one))
						(*where_node)->ptr_one_type = PTR_TYPE_DATE;
					else
						(*where_node)->ptr_one_type = PTR_TYPE_CHAR;
				}
				else
				{
					int hmm_int = atoi(word);

					if (hmm_int > 0 && strcontains(word, '.'))
					{
						// Double
						(*where_node)->ptr_one = myMalloc(sizeof(double), NULL, malloced_head, the_debug);

						double hmm_double = 0.0;

						sscanf(word, "%lf", &hmm_double);

						*((double*) (*where_node)->ptr_one) = hmm_double;

						(*where_node)->ptr_one_type = PTR_TYPE_REAL;
					}
					else if (hmm_int > 0)
					{
						// Int
						(*where_node)->ptr_one = myMalloc(sizeof(int), NULL, malloced_head, the_debug);

						*((int*) (*where_node)->ptr_one) = hmm_int;

						(*where_node)->ptr_one_type = PTR_TYPE_INT;
					}
					else
					{
						// Char but column name, so get column
						char* alias = NULL;
						char* name = upper(word, NULL, malloced_head, the_debug);

						int index_of_dot = indexOf(name, '.');
						if (index_of_dot > -1)
						{
							alias = substring(name, 0, index_of_dot-1, NULL, malloced_head, the_debug);

							char* new_name = substring(name, index_of_dot+1, strLength(name)-1, NULL, malloced_head, the_debug);

							myFree((void**) &name, NULL, malloced_head, the_debug);

							name = new_name;
						}
						printf("alias.name = _%s.%s_\n", alias, name);

						if (getColInSelectNodeFromName((void*) (*where_node), PTR_TYPE_WHERE_CLAUSE_NODE, 1, 1, select_node, alias, name
													  ,malloced_head, the_debug) != 0)
						{
							errorTeardown(NULL, malloced_head, the_debug);
							*where_node = NULL;
							return -1;
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
			errorTeardown(NULL, malloced_head, the_debug);
			*where_node = NULL;
			return -1;
		}

		if (prev_word != NULL)
			myFree((void**) &prev_word, NULL, malloced_head, the_debug);
		
		if (!(word[0] == '=' || word[0] == '>' || word[0] == '<' || strcmp_Upper(word, "IS", NULL, malloced_head, the_debug) == 0))
			prev_word = upper(word, NULL, malloced_head, the_debug);
	}

	if ((((*where_node)->ptr_one_type == -1 || (*where_node)->ptr_two_type == -1) && (*where_node)->where_type != WHERE_IS_NULL && (*where_node)->where_type != WHERE_IS_NOT_NULL))
	{
		if (the_debug == YES_DEBUG)
			printf("	ERROR in parseOneWhereNode() at line %d in %s\n", __LINE__, __FILE__);
		errorTeardown(NULL, malloced_head, the_debug);
		*where_node = NULL;
		return -1;
	}

	myFree((void**) &prev_word, NULL, malloced_head, the_debug);

	return 0;
}

int parseWhereClause(char* input, struct where_clause_node** where_head, struct select_node* select_node, char* first_word
				    ,struct malloced_node** malloced_head, int the_debug)
{
	// START Allocate space for where_head and initialize
		if (*where_head == NULL)
		{
			*where_head = (struct where_clause_node*) myMalloc(sizeof(struct where_clause_node), NULL, malloced_head, the_debug);
			if (*where_head == NULL)
			{
				if (the_debug == YES_DEBUG)
					printf("	ERROR in parseWhereClause() at line %d in %s\n", __LINE__, __FILE__);
				*where_head = NULL;
				return -1;
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
			return -1;
		}
		word[0] = 0;
		int index = 0;
	// END Init word variable

	// START See if first word is WHERE or ON or WHEN
		if (first_word != NULL)
		{
			getNextWord(input, word, &index);

			int compared_where = strcmp_Upper(word, "WHERE", NULL, malloced_head, the_debug);
			int compared_on = strcmp_Upper(word, "ON", NULL, malloced_head, the_debug);
			int compared_when = strcmp_Upper(word, "WHEN", NULL, malloced_head, the_debug);

			if ((compared_where != 0 && strcmp(first_word, "where") == 0) || (compared_on != 0 && strcmp(first_word, "on") == 0) || (compared_when != 0 && strcmp(first_word, "when") == 0))
			{
				if (the_debug == YES_DEBUG)
					printf("	ERROR in parseWhereClause() at line %d in %s\n", __LINE__, __FILE__);
				errorTeardown(NULL, malloced_head, the_debug);
				*where_head = NULL;
				return -1;
			}
		}
	// END See if first word is WHERE or ON or WHEN

	int open_parens = 0;

	char* part_of_input = (char*) myMalloc(sizeof(char) * 1000, NULL, malloced_head, the_debug);
	part_of_input[0] = 0;

	int specials_done = 0;
	
	while(getNextWord(input, word, &index) == 0 && word[0] != ';')
	{
		if (open_parens == 0 && word[0] == '(')
			open_parens++;
		else if (open_parens == 1 && word[0] == ')' && (strContainsWordUpper(part_of_input, " AND ") == 1 || strContainsWordUpper(part_of_input, " OR ") == 1))
		{
			// START Recursively call this function based on which ptr is null
				open_parens--;
				
				printf("Found parentheses, part_of_input = _%s_\n", part_of_input);
				// Call parseWhereClause()

				if ((*where_head)->ptr_one == NULL)
				{
					printf("	Recursively calling in ptr_one\n");
					(*where_head)->ptr_one = (struct where_clause_node*) myMalloc(sizeof(struct where_clause_node), NULL, malloced_head, the_debug);
					(*where_head)->ptr_one_type = PTR_TYPE_WHERE_CLAUSE_NODE;

					((struct where_clause_node*) (*where_head)->ptr_one)->ptr_one = NULL;
					((struct where_clause_node*) (*where_head)->ptr_one)->ptr_one_type = -1;

					((struct where_clause_node*) (*where_head)->ptr_one)->ptr_two = NULL;
					((struct where_clause_node*) (*where_head)->ptr_one)->ptr_two_type = -1;

					((struct where_clause_node*) (*where_head)->ptr_one)->where_type = -1;
					((struct where_clause_node*) (*where_head)->ptr_one)->parent = *where_head;


					parseWhereClause(part_of_input, (struct where_clause_node**) &(*where_head)->ptr_one, select_node, NULL
								    ,malloced_head, the_debug);
				}
				else if ((*where_head)->ptr_two == NULL)
				{
					printf("	Recursively calling in ptr_two\n");
					(*where_head)->ptr_two = (struct where_clause_node*) myMalloc(sizeof(struct where_clause_node), NULL, malloced_head, the_debug);
					(*where_head)->ptr_two_type = PTR_TYPE_WHERE_CLAUSE_NODE;

					((struct where_clause_node*) (*where_head)->ptr_two)->ptr_one = NULL;
					((struct where_clause_node*) (*where_head)->ptr_two)->ptr_one_type = -1;

					((struct where_clause_node*) (*where_head)->ptr_two)->ptr_two = NULL;
					((struct where_clause_node*) (*where_head)->ptr_two)->ptr_two_type = -1;

					((struct where_clause_node*) (*where_head)->ptr_two)->where_type = -1;
					((struct where_clause_node*) (*where_head)->ptr_two)->parent = *where_head;


					parseWhereClause(part_of_input, (struct where_clause_node**) &(*where_head)->ptr_two, select_node, NULL
								    ,malloced_head, the_debug);
				}

				printf("	Back from parseWhereClause()\n");

				getNextWord(input, word, &index);

				if (strcmp_Upper(word, "OR", NULL, malloced_head, the_debug) == 0)
					(*where_head)->where_type = WHERE_OR;
				else if (strcmp_Upper(word, "AND", NULL, malloced_head, the_debug) == 0)
					(*where_head)->where_type = WHERE_AND;

				part_of_input[0] = 0;
			// END Recursively call this function based on which ptr is null
		}
		else if (open_parens == 0 && (strcmp_Upper(word, "OR", NULL, malloced_head, the_debug) == 0 || strcmp_Upper(word, "AND", NULL, malloced_head, the_debug) == 0))
		{
			// START Create new where node and initialize it
				printf("Found _%s_, part_of_input = _%s_\n", word, part_of_input);

				struct where_clause_node* new_where = (struct where_clause_node*) myMalloc(sizeof(struct where_clause_node), NULL, malloced_head, the_debug);

				new_where->ptr_one = NULL;
				new_where->ptr_one_type = -1;

				new_where->ptr_two = NULL;
				new_where->ptr_two_type = -1;

				new_where->where_type = -1;


				char* temp_word = (char*) myMalloc(sizeof(char) * (strLength(word)+1), NULL, malloced_head, the_debug);
				strcpy(temp_word, word);


				if (parseOneWhereNode(part_of_input, word, &new_where, select_node
									 ,malloced_head, the_debug) != 0)
					return -1;


				if ((*where_head)->ptr_one == NULL)
				{
					printf("	Assigning new_where to ptr_one\n");
					(*where_head)->ptr_one = new_where;
					(*where_head)->ptr_one_type = PTR_TYPE_WHERE_CLAUSE_NODE;

					if (strcmp_Upper(temp_word, "OR", NULL, malloced_head, the_debug) == 0)
						(*where_head)->where_type = WHERE_OR;
					else if (strcmp_Upper(temp_word, "AND", NULL, malloced_head, the_debug) == 0)
						(*where_head)->where_type = WHERE_AND;

					((struct where_clause_node*) (*where_head)->ptr_one)->parent = *where_head;
				}
				else if ((*where_head)->ptr_two == NULL)
				{
					printf("	Assigning new_where to ptr_two\n");
					(*where_head)->ptr_two = new_where;
					(*where_head)->ptr_two_type = PTR_TYPE_WHERE_CLAUSE_NODE;

					((struct where_clause_node*) (*where_head)->ptr_two)->parent = *where_head;
				}
			// END Create new where node and initialize it

			// START Decide how to format the tree given there will be a new node
				if ((*where_head)->ptr_two != NULL && strcmp_Upper(temp_word, "AND", NULL, malloced_head, the_debug) == 0 && (*where_head)->where_type == WHERE_OR)
				{
					printf("	Special option to make new parent for ptr_two, keeping ptr_one in *where_head the same and making ptr_two->ptr_two null\n");
					struct where_clause_node* temp_where = (struct where_clause_node*) myMalloc(sizeof(struct where_clause_node), NULL, malloced_head, the_debug);

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
					printf("	Default option to make new parent for ptr_one, assigning *where_head to ptr_one and making ptr_two null\n");
					struct where_clause_node* temp_where = (struct where_clause_node*) myMalloc(sizeof(struct where_clause_node), NULL, malloced_head, the_debug);

					temp_where->ptr_one = *where_head;
					temp_where->ptr_one_type = PTR_TYPE_WHERE_CLAUSE_NODE;

					temp_where->ptr_two = NULL;
					temp_where->ptr_two_type = -1;

					if (strcmp_Upper(temp_word, "OR", NULL, malloced_head, the_debug) == 0)
						temp_where->where_type = WHERE_OR;
					else if (strcmp_Upper(temp_word, "AND", NULL, malloced_head, the_debug) == 0)
						temp_where->where_type = WHERE_AND;

					temp_where->parent = (*where_head)->parent;

					if (temp_where->parent != NULL)
						temp_where->parent->ptr_one = temp_where;

					(*where_head)->parent = temp_where;

					//((struct where_clause_node*) (*where_head)->parent->ptr_two)->parent = temp_where;

					*where_head = temp_where;
				}
			// END Decide how to format the tree given there will be a new node

			myFree((void**) &temp_word, NULL, malloced_head, the_debug);


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
			if (strcontains(part_of_input, ')'))
			{
				char* temp_str = (char*) myMalloc(sizeof(char) * (strLength(part_of_input)+2), NULL, malloced_head, the_debug);
				strcpy(temp_str, "(");

				strcat(temp_str, part_of_input);

				myFree((void**) &part_of_input, NULL, malloced_head, the_debug);

				part_of_input = temp_str;
			}

			printf("Found END OF INPUT, part_of_input = _%s_\n", part_of_input);

			if ((*where_head)->ptr_one == NULL && (*where_head)->ptr_two == NULL)
			{
				if (parseOneWhereNode(part_of_input, word, where_head, select_node
									 ,malloced_head, the_debug) != 0)
					return -1;
			}
			else if ((*where_head)->ptr_one != NULL && (*where_head)->ptr_two == NULL)
			{
				struct where_clause_node* new_where = (struct where_clause_node*) myMalloc(sizeof(struct where_clause_node), NULL, malloced_head, the_debug);

				new_where->ptr_one = NULL;
				new_where->ptr_one_type = -1;

				new_where->ptr_two = NULL;
				new_where->ptr_two_type = -1;

				new_where->where_type = -1;

				if (parseOneWhereNode(part_of_input, word, &new_where, select_node
									 ,malloced_head, the_debug) != 0)
					return -1;


				(*where_head)->ptr_two = new_where;
				(*where_head)->ptr_two_type = PTR_TYPE_WHERE_CLAUSE_NODE;

				((struct where_clause_node*) (*where_head)->ptr_two)->parent = *where_head;


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
		return -1;
	}
	else if (((*where_head)->where_type == WHERE_OR || (*where_head)->where_type == WHERE_AND) && ((*where_head)->ptr_one == NULL || (*where_head)->ptr_two == NULL))
	{
		if (the_debug == YES_DEBUG)
			printf("	ERROR in parseWhereClause() at line %d in %s\n", __LINE__, __FILE__);
		errorTeardown(NULL, malloced_head, the_debug);
		*where_head = NULL;
		return -1;
	}


	return 0;
}


struct table_info* getTableFromName(char* input_table_name
								   ,struct malloced_node** malloced_head, int the_debug)
{
	// START Get table name and find table node in list
		char* upper_table_name = upper(input_table_name, NULL, malloced_head, the_debug);
		if (upper_table_name == NULL)
		{
			if (the_debug == YES_DEBUG)
				printf("	ERROR in getTableFromName() at line %d in %s\n", __LINE__, __FILE__);
			return NULL;
		}

		struct table_info* cur_table = getTablesHead();
		while (cur_table != NULL)
		{
			char* upper_cur_table_name = upper(cur_table->name, NULL, malloced_head, the_debug);
			if (upper_cur_table_name == NULL)
			{
				if (the_debug == YES_DEBUG)
					printf("	ERROR in getTableFromName() at line %d in %s\n", __LINE__, __FILE__);
				return NULL;
			}

			int cmp_result = strcmp(upper_table_name, upper_cur_table_name);

			myFree((void**) &upper_cur_table_name, NULL, malloced_head, the_debug);

			if (cmp_result == 0)
				break;

			cur_table = cur_table->next;
		}

		myFree((void**) &upper_table_name, NULL, malloced_head, the_debug);

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

/*	 
 *	Writes to (so calling function can read):
 *		struct table_info** table;
 *		struct change_node_v2** change_head;
 *		struct or_clause_node** or_head;
 */
/*int parseUpdate(char* input, struct table_info** table, struct change_node_v2** change_head, struct or_clause_node** or_head
			   ,struct malloced_node** malloced_head, int the_debug)
{
	char* word = (char*) myMalloc(sizeof(char) * 200, NULL, malloced_head, the_debug);
	if (word == NULL)
	{
		if (the_debug == YES_DEBUG)
			printf("	ERROR in parseUpdate() at line %d in %s\n", __LINE__, __FILE__);
		*change_head = NULL;
		*or_head = NULL;
		return -1;
	}
	word[0] = 0;
	int index = 0;

	// START See if first word is UPDATE
	getNextWord(input, word, &index);

	char* upper_word = upper(word, NULL, malloced_head, the_debug);
	int compared = strcmp(upper_word, "UPDATE");
	myFree((void**) &upper_word, NULL, malloced_head, the_debug);

	if (compared != 0)
	{
		if (the_debug == YES_DEBUG)
			printf("	ERROR in parseUpdate() at line %d in %s\n", __LINE__, __FILE__);
		errorTeardown(NULL, malloced_head, the_debug);
		*change_head = NULL;
		*or_head = NULL;
		return -1;
	}
	// END See if first word is UPDATE

	// START Get table
	getNextWord(input, word, &index);

	struct table_info* cur_table = getTableFromName(word, malloced_head, the_debug);

	*table = cur_table;

	if (cur_table == NULL)
	{
		if (the_debug == YES_DEBUG)
			printf("	ERROR in parseUpdate() at line %d in %s\n", __LINE__, __FILE__);
		*change_head = NULL;
		*or_head = NULL;
		return -1;
	}
	// END Get table


	// START Get all the set col = 'value'
	//	START Skip the "set "
	while (!(input[index] == 0 || (input[index] == 's' || input[index] == 'S') && (input[index+1] == 'e' || input[index+1] == 'E') && (input[index+2] == 't' || input[index+2] == 'T')))
		index++;

	index += 4;
	//	END Skip the "set "

	// 	START Alloc memory to the change_head
	*change_head = (struct change_node_v2*) myMalloc(sizeof(struct change_node_v2), NULL, malloced_head, the_debug);
	if ((*change_head)  == NULL)
	{
		if (the_debug == YES_DEBUG)
			printf("	ERROR in parseUpdate() at line %d in %s\n", __LINE__, __FILE__);
		*change_head = NULL;
		*or_head = NULL;
		return -1;
	}
	(*change_head)->col_number = -1;
	(*change_head)->operation = -1;
	(*change_head)->data_type = -1;
	(*change_head)->data = NULL;
	(*change_head)->next = NULL;
	// 	END Alloc memory to the change_head

	struct change_node_v2* cur_change = *change_head;

	word[0] = 0;
	//int word_index = 0;

	while (getNextWord(input, word, &index) == 0)
	{
		upper_word = upper(word, NULL, malloced_head, the_debug);
		compared = strcmp(upper_word, "WHERE");
		myFree((void**) &upper_word, NULL, malloced_head, the_debug);

		if (compared == 0)
			break;

		//printf("%c and %d\n", input[index], input[index]);
		if (word[0] != 0 && word[0] != ';')
		{
			//printf("word: _%s_\n", word);
			if (word[0] == '=' && word[1] == 0)
			{
				if (cur_change->operation != -1)
				{
					if (the_debug == YES_DEBUG)
						printf("	ERROR in parseUpdate() at line %d in %s\n", __LINE__, __FILE__);
					errorTeardown(NULL, malloced_head, the_debug);
					*change_head = NULL;
					*or_head = NULL;
					return -1;
				}

				cur_change->operation = 3;
			}
			else if (word[0] == 39)
			{
				struct table_cols_info* cur_col = cur_table->table_cols_head;
				while (cur_col != NULL)
				{
					if (cur_col->col_number == cur_change->col_number)
						break;

					cur_col = cur_col->next;
				}

				if (cur_change->data != NULL || (cur_col != NULL && (cur_col->data_type == DATA_INT || cur_col->data_type == DATA_REAL)))
				{
					if (the_debug == YES_DEBUG)
						printf("	ERROR in parseUpdate() at line %d in %s\n", __LINE__, __FILE__);
					errorTeardown(NULL, malloced_head, the_debug);
					*change_head = NULL;
					*or_head = NULL;
					return -1;
				}

				cur_change->data = substring(word, 1, strLength(word)-2, NULL, malloced_head, the_debug);
			}
			else if (word[0] == ',')
			{
				//printf("Option new\n");
				if (cur_change->data == NULL || cur_change->operation == -1 || cur_change->col_number == -1 || cur_change->data_type == -1)
				{
					if (the_debug == YES_DEBUG)
						printf("	ERROR in parseUpdate() at line %d in %s\n", __LINE__, __FILE__);
					errorTeardown(NULL, malloced_head, the_debug);
					*change_head = NULL;
					*or_head = NULL;
					return -1;
				}

				cur_change->next = (struct change_node_v2*) myMalloc(sizeof(struct change_node_v2), NULL, malloced_head, the_debug);
				if (cur_change->next == NULL)
				{
					if (the_debug == YES_DEBUG)
						printf("	ERROR in parseUpdate() at line %d in %s\n", __LINE__, __FILE__);
					errorTeardown(NULL, malloced_head, the_debug);
					*change_head = NULL;
					*or_head = NULL;
					return -1;
				}
				cur_change->next->col_number = -1;
				cur_change->next->operation = -1;
				cur_change->next->data_type = -1;
				cur_change->next->data = NULL;
				cur_change->next->next = NULL;

				struct change_node_v2* cur = *change_head;
				while (cur != NULL)
				{
					if (cur == cur_change->next)
						break;
					cur = cur->next;
				}
				if (cur == NULL)
				{
					if (the_debug == YES_DEBUG)
						printf("	ERROR in parseUpdate() at line %d in %s\n", __LINE__, __FILE__);
					errorTeardown(NULL, malloced_head, the_debug);
					*change_head = NULL;
					*or_head = NULL;
					return -1;
				}

				cur_change = cur_change->next;
			}
			else
			{
				struct table_cols_info* cur_col = cur_table->table_cols_head;
				while (cur_col != NULL)
				{
					upper_word = upper(word, NULL, malloced_head, the_debug);
					char* upper_col_name = upper(cur_col->col_name, NULL, malloced_head, the_debug);
					if (upper_word == NULL || upper_col_name == NULL)
					{
						if (the_debug == YES_DEBUG)
							printf("	ERROR in parseWhereClause() at line %d in %s\n", __LINE__, __FILE__);
						*change_head = NULL;
						*or_head = NULL;
						return -1;
					}

					//printf("upper_word = _%s_\n", upper_word);
					//printf("upper_col_name = _%s_\n", upper_col_name);

					int cmp_result = strcmp(upper_word, upper_col_name);

					myFree((void**) &upper_word, NULL, malloced_head, the_debug);
					myFree((void**) &upper_col_name, NULL, malloced_head, the_debug);

					if (cmp_result == 0)
					{
						if (cur_change->col_number == -1 && cur_change->data_type == -1)
						{
							//printf("Option 5a\n");
							cur_change->col_number = cur_col->col_number;
							cur_change->data_type = cur_col->data_type;
						}
						else
						{
							if (the_debug == YES_DEBUG)
								printf("	ERROR in parseUpdate() at line %d in %s\n", __LINE__, __FILE__);
							errorTeardown(NULL, malloced_head, the_debug);
							*change_head = NULL;
							*or_head = NULL;
							return -1;
						}

						break;
					}

					cur_col = cur_col->next;
				}

				if (cur_col == NULL)
				{
					cur_col = cur_table->table_cols_head;
					while (cur_col != NULL)
					{
						if (cur_col->col_number == cur_change->col_number)
							break;

						cur_col = cur_col->next;
					}

					if (cur_change->data != NULL || (cur_col != NULL && (cur_col->data_type == DATA_STRING || cur_col->data_type == DATA_DATE)))
					{
						if (the_debug == YES_DEBUG)
							printf("	ERROR in parseUpdate() at line %d in %s\n", __LINE__, __FILE__);
						errorTeardown(NULL, malloced_head, the_debug);
						*change_head = NULL;
						*or_head = NULL;
						return -1;
					}

					//printf("Option 5b\n");
					cur_change->data = (char*) myMalloc(sizeof(char) * 64, NULL, malloced_head, the_debug);
					if (cur_change->data == NULL)
					{
						if (the_debug == YES_DEBUG)
							printf("	ERROR in parseUpdate() at line %d in %s\n", __LINE__, __FILE__);
						errorTeardown(NULL, malloced_head, the_debug);
						*change_head = NULL;
						*or_head = NULL;
						return -1;
					}
					strcpy(cur_change->data, word);
				}
			}
		}

		if (input[index] == ',')
		{
			
		}

		//word[0] = 0;
		//word_index = 0;

		//index++;
	}
	// END Get all the set = 'value'

	//printf("here\n");
	upper_word = upper(word, NULL, malloced_head, the_debug);
	//printf("upper_word = _%s_\n", upper_word);
	compared = strcmp(upper_word, "WHERE");
	myFree((void**) &upper_word, NULL, malloced_head, the_debug);
	myFree((void**) &word, NULL, malloced_head, the_debug);

	if (compared == 0)
		index-=5;
	
	//printf("index = %d\n", index);
	if (input[index] != 0)
	{
		// START Find and parse where clause
		char* where_clause = substring(input, index, strLength(input)-1, NULL, malloced_head, the_debug);
		if (where_clause == NULL)
		{
			if (the_debug == YES_DEBUG)
				printf("	ERROR in parseUpdate() at line %d in %s\n", __LINE__, __FILE__);
			*change_head = NULL;
			*or_head = NULL;
			return -1;
		}
		//printf("where_clause = _%s_\n", where_clause);

		if (or_head != NULL)
		{
			int error_code;
			*or_head = parseWhereClause(where_clause, NULL, &error_code, "where", malloced_head, the_debug);

			if (error_code != 0)
			{
				*change_head = NULL;
				*or_head = NULL;
				return -1;
			}
		}

		if (myFree((void**) &where_clause, NULL, malloced_head, the_debug) != 0)
		{
			if (the_debug == YES_DEBUG)
				printf("	ERROR in parseUpdate() at line %d in %s\n", __LINE__, __FILE__);
			*change_head = NULL;
			*or_head = NULL;
			return -1;
		}

		/*if (or_head != NULL && *or_head == NULL)
		{
			if (the_debug == YES_DEBUG)
				printf("	ERROR in parseUpdate() at line %d in %s\n", __LINE__, __FILE__);
			errorTeardown(NULL, malloced_head, the_debug);
			return -1;
		}*/
		// END Find and parse where clause
	/*}
	else
	{
		if (or_head != NULL)
			*or_head = NULL;
	}


	/*
	cur_change = *change_head;
	while (cur_change != NULL)
	{
		printf("A change node:\n");
		printf("	col_number = %lu\n", cur_change->col_number);
		printf("	operation = %lu\n", cur_change->operation);
		printf("	data_type = %lu\n", cur_change->data_type);
		printf("	data = %s\n", cur_change->data);

		cur_change = cur_change->next;
	}*/

	/*if (the_debug == YES_DEBUG)
		printf("Calling myFreeAllCleanup() from parseUpdate()\n");
	myFreeAllCleanup(&malloced_head, the_debug);*/

	/*return 0;
}

/*	 
 *	Writes to (so calling function can read):
 *		struct or_clause_node** or_head;
 *		struct table_info** table;
 */
/*int parseDelete(char* input, struct or_clause_node** or_head, struct table_info** table
			   ,struct malloced_node** malloced_head, int the_debug)
{
	char* word = (char*) myMalloc(sizeof(char) * 200, NULL, malloced_head, the_debug);
	if (word == NULL)
	{
		if (the_debug == YES_DEBUG)
			printf("	ERROR in parseDelete() at line %d in %s\n", __LINE__, __FILE__);
		*or_head = NULL;
		*table = NULL;
		return -1;
	}
	word[0] = 0;
	int index = 0;

	// START See if first word is DELETE
	getNextWord(input, word, &index);

	char* upper_word = upper(word, NULL, malloced_head, the_debug);
	int compared = strcmp(upper_word, "DELETE");
	myFree((void**) &upper_word, NULL, malloced_head, the_debug);

	if (compared != 0)
	{
		if (the_debug == YES_DEBUG)
			printf("	ERROR in parseDelete() at line %d in %s\n", __LINE__, __FILE__);
		errorTeardown(NULL, malloced_head, the_debug);
		*or_head = NULL;
		*table = NULL;
		return -1;
	}
	// END See if first word is DELETE

	// START See if second word is FROM
	getNextWord(input, word, &index);

	upper_word = upper(word, NULL, malloced_head, the_debug);
	compared = strcmp(upper_word, "FROM");
	myFree((void**) &upper_word, NULL, malloced_head, the_debug);

	if (compared != 0)
	{
		if (the_debug == YES_DEBUG)
			printf("	ERROR in parseDelete() at line %d in %s\n", __LINE__, __FILE__);
		errorTeardown(NULL, malloced_head, the_debug);
		*or_head = NULL;
		*table = NULL;
		return -1;
	}
	// END See if second word is FROM

	// START Get table
	getNextWord(input, word, &index);

	*table = getTableFromName(word, malloced_head, the_debug);

	if (*table == NULL)
	{
		if (the_debug == YES_DEBUG)
			printf("	ERROR in parseDelete() at line %d in %s\n", __LINE__, __FILE__);
		errorTeardown(NULL, malloced_head, the_debug);
		*or_head = NULL;
		*table = NULL;
		return -1;
	}
	// END Get table

	//printf("table name = %s\n", (*table)->name);

	myFree((void**) &word, NULL, malloced_head, the_debug);


	index = 0;
	while (!(input[index] == 0 || ((input[index] == 'w' || input[index] == 'W') && (input[index+1] == 'h' || input[index+1] == 'H') 
							     && (input[index+2] == 'e' || input[index+2] == 'E') && (input[index+3] == 'r' || input[index+3] == 'R') 
							     && (input[index+4] == 'e' || input[index+4] == 'E'))))
	{
		index++;
	}


	if (input[index] != 0)
	{
		// START Find and parse where clause
		char* where_clause = substring(input, index, strLength(input)-1, NULL, malloced_head, the_debug);
		if (where_clause == NULL)
		{
			if (the_debug == YES_DEBUG)
				printf("	ERROR in parseDelete() at line %d in %s\n", __LINE__, __FILE__);
			*or_head = NULL;
			*table = NULL;
			return -1;
		}
		//printf("where_clause = _%s_\n", where_clause);

		if (or_head != NULL)
		{
			int error_code;
			*or_head = parseWhereClause(where_clause, NULL, &error_code, "where", malloced_head, the_debug);

			if (error_code != 0)
			{
				*or_head = NULL;
				*table = NULL;
				return -1;
			}
		}
		//printf("Got where\n");

		if (myFree((void**) &where_clause, NULL, malloced_head, the_debug) != 0)
		{
			if (the_debug == YES_DEBUG)
				printf("	ERROR in parseDelete() at line %d in %s\n", __LINE__, __FILE__);
			*or_head = NULL;
			*table = NULL;
			return -1;
		}
		// END Find and parse where clause
	}
	else
	{
		if (or_head != NULL)
			*or_head = NULL;
	}

	return 0;
}

/*	 
 *	Writes to (so calling function can read):
 *		struct change_node_v2** change_head;
 *		struct table_info** table;
 */
/*int parseInsert(char* input, struct change_node_v2** change_head, struct table_info** table
			   ,struct malloced_node** malloced_head, int the_debug)
{
	//printf("STARTING parseInsert\n");
	char* word = (char*) myMalloc(sizeof(char) * 200, NULL, malloced_head, the_debug);
	if (word == NULL)
	{
		if (the_debug == YES_DEBUG)
			printf("	ERROR in parseInsert() at line %d in %s\n", __LINE__, __FILE__);
		*change_head = NULL;
		*table = NULL;
		return -1;
	}
	word[0] = 0;
	int index = 0;

	// START See if first word is INSERT
	getNextWord(input, word, &index);

	char* upper_word = upper(word, NULL, malloced_head, the_debug);
	int compared = strcmp(upper_word, "INSERT");
	myFree((void**) &upper_word, NULL, malloced_head, the_debug);

	if (compared != 0)
	{
		if (the_debug == YES_DEBUG)
			printf("	ERROR in parseInsert() at line %d in %s\n", __LINE__, __FILE__);
		errorTeardown(NULL, malloced_head, the_debug);
		*change_head = NULL;
		*table = NULL;
		return -1;
	}
	// END See if first word is INSERT

	// START See if second word is INTO
	getNextWord(input, word, &index);

	upper_word = upper(word, NULL, malloced_head, the_debug);
	compared = strcmp(upper_word, "INTO");
	myFree((void**) &upper_word, NULL, malloced_head, the_debug);

	if (compared != 0)
	{
		if (the_debug == YES_DEBUG)
			printf("	ERROR in parseInsert() at line %d in %s\n", __LINE__, __FILE__);
		errorTeardown(NULL, malloced_head, the_debug);
		*change_head = NULL;
		*table = NULL;
		return -1;
	}
	// END See if second word is INTO

	// START Get table
	getNextWord(input, word, &index);

	*table = getTableFromName(word, malloced_head, the_debug);

	if (*table == NULL)
	{
		if (the_debug == YES_DEBUG)
			printf("	ERROR in parseInsert() at line %d in %s\n", __LINE__, __FILE__);
		errorTeardown(NULL, malloced_head, the_debug);
		*change_head = NULL;
		*table = NULL;
		return -1;
	}
	// END Get table


	// START
	*change_head = (struct change_node_v2*) myMalloc(sizeof(struct change_node_v2), NULL, malloced_head, the_debug);
	if (*change_head == NULL)
	{
		if (the_debug == YES_DEBUG)
			printf("	ERROR in parseInsert() at line %d in %s\n", __LINE__, __FILE__);
		*change_head = NULL;
		*table = NULL;
		return -1;
	}
	(*change_head)->col_number = (*table)->table_cols_head->col_number;
	(*change_head)->operation = -1;
	(*change_head)->data_type = (*table)->table_cols_head->data_type;
	(*change_head)->data = NULL;
	(*change_head)->next = NULL;

	struct change_node_v2* cur_change = *change_head;

	struct table_cols_info* cur_col = (*table)->table_cols_head;
	while (cur_col->next != NULL)
	{
		cur_change->next = (struct change_node_v2*) myMalloc(sizeof(struct change_node_v2), NULL, malloced_head, the_debug);
		if (cur_change->next == NULL)
		{
			if (the_debug == YES_DEBUG)
				printf("	ERROR in parseInsert() at line %d in %s\n", __LINE__, __FILE__);
			*change_head = NULL;
			*table = NULL;
			return -1;
		}
		cur_change->next->col_number = cur_col->next->col_number;
		cur_change->next->operation = -1;
		cur_change->next->data_type = cur_col->next->data_type;
		cur_change->next->data = NULL;

		cur_change->next->next = NULL;
		cur_change = cur_change->next;

		cur_col = cur_col->next;
	}
	// END


	struct ListNode* list_head = NULL;
	struct ListNode* list_tail = NULL;
	struct ListNode* cur_in_list = NULL;

	addListNode(&list_head, &list_tail, -1, ADDLISTNODE_TAIL, NULL, malloced_head, the_debug);
	cur_in_list = list_tail;

	bool did_first_parens = false;
	int parens = 0;
	while (getNextWord(input, word, &index) == 0)
	{
		//printf("word: = _%s_\n");

		char* temp_upper_word = upper(word, NULL, malloced_head, the_debug);
		compared = strcmp(temp_upper_word, "VALUES");
		myFree((void**) &temp_upper_word, NULL, malloced_head, the_debug);

		if (compared == 0 && did_first_parens)
			word[0] = 0;


		if (word[0] != 0 && word[0] != ';')
		{
			if (word[0] == '(')
			{
				parens++;
			}
			else if (word[0] == ')')
			{
				parens--;
				if (!did_first_parens)
				{
					did_first_parens = true;
					cur_in_list = list_head;
				}
			}
			else if (word[0] == ',')
			{
				if (!did_first_parens)
				{
					addListNode(&list_head, &list_tail, -1, ADDLISTNODE_TAIL, NULL, malloced_head, the_debug);
					cur_in_list = list_tail;
					//printf("Added new to list %d\n", cur_in_list->value);
				}
				else
					cur_in_list = cur_in_list->next;
			}
			else if (!did_first_parens)
			{
				struct table_cols_info* cur_col = (*table)->table_cols_head;
				while (cur_col != NULL)
				{
					char* upper_word = upper(word, NULL, malloced_head, the_debug);
					char* upper_col_name = upper(cur_col->col_name, NULL, malloced_head, the_debug);
					if (upper_word == NULL || upper_col_name == NULL)
					{
						if (the_debug == YES_DEBUG)
							printf("	ERROR in parseInsert() at line %d in %s\n", __LINE__, __FILE__);
						*change_head = NULL;
						*table = NULL;
						return -1;
					}

					//printf("upper_word = _%s_\n", upper_word);
					//printf("upper_col_name = _%s_\n", upper_col_name);

					int cmp_result = strcmp(upper_word, upper_col_name);

					myFree((void**) &upper_word, NULL, malloced_head, the_debug);
					myFree((void**) &upper_col_name, NULL, malloced_head, the_debug);

					if (cmp_result == 0)
					{
						if (cur_in_list->value != -1)
						{
							if (the_debug == YES_DEBUG)
								printf("	ERROR in parseInsert() at line %d in %s\n", __LINE__, __FILE__);
							errorTeardown(NULL, malloced_head, the_debug);
							*change_head = NULL;
							*table = NULL;
							return -1;
						}

						//printf("Option 5\n");
						cur_in_list->value = cur_col->col_number;
						//printf("cur_in_list->value = %d\n", cur_in_list->value);

						cur_change = *change_head;
						while (cur_change != NULL)
						{
							if (cur_change->col_number == cur_in_list->value)
							{
								if (cur_change->operation != -1)
								{
									if (the_debug == YES_DEBUG)
										printf("	ERROR in parseInsert() at line %d in %s\n", __LINE__, __FILE__);
									errorTeardown(NULL, malloced_head, the_debug);
									*change_head = NULL;
									*table = NULL;
									return -1;
								}

								cur_change->operation = OP_INSERT;
								break;
							}

							cur_change = cur_change->next;
						}

						break;
					}

					cur_col = cur_col->next;
				}

				if (cur_col == NULL)
				{
					if (the_debug == YES_DEBUG)
						printf("	ERROR in parseInsert() at line %d in %s\n", __LINE__, __FILE__);
					errorTeardown(NULL, malloced_head, the_debug);
					*change_head = NULL;
					*table = NULL;
					return -1;
				}
			}
			else if (did_first_parens)
			{
				if (cur_in_list == NULL)
				{
					if (the_debug == YES_DEBUG)
						printf("	ERROR in parseInsert() at line %d in %s\n", __LINE__, __FILE__);
					errorTeardown(NULL, malloced_head, the_debug);
					*change_head = NULL;
					*table = NULL;
					return -1;
				}

				cur_change = *change_head;
				while (cur_change != NULL)
				{
					if (cur_change->col_number == cur_in_list->value)
						break;

					cur_change = cur_change->next;
				}

				if (cur_change == NULL)
				{
					if (the_debug == YES_DEBUG)
						printf("	ERROR in parseInsert() at line %d in %s\n", __LINE__, __FILE__);
					errorTeardown(NULL, malloced_head, the_debug);
					*change_head = NULL;
					*table = NULL;
					return -1;
				}

				//printf("cur_change->col_number = %d\n", cur_change->col_number);

				if (word[0] == 39)
				{
					if (cur_change->data != NULL)
					{
						if (the_debug == YES_DEBUG)
							printf("	ERROR in parseInsert() at line %d in %s\n", __LINE__, __FILE__);
						errorTeardown(NULL, malloced_head, the_debug);
						*change_head = NULL;
						*table = NULL;
						return -1;
					}

					struct table_cols_info* cur_col = (*table)->table_cols_head;
					while (cur_col != NULL)
					{
						if (cur_col->col_number == cur_change->col_number)
							break;

						cur_col = cur_col->next;
					}

					if (cur_change->data != NULL || (cur_col != NULL && (cur_col->data_type == DATA_INT || cur_col->data_type == DATA_REAL)))
					{
						if (the_debug == YES_DEBUG)
							printf("	ERROR in parseInsert() at line %d in %s\n", __LINE__, __FILE__);
						errorTeardown(NULL, malloced_head, the_debug);
						*change_head = NULL;
						*table = NULL;
						return -1;
					}

					cur_change->data = substring(word, 1, strLength(word)-2, NULL, malloced_head, the_debug);
				}
				else
				{
					struct table_cols_info* cur_col = (*table)->table_cols_head;
					while (cur_col != NULL)
					{
						if (cur_col->col_number == cur_change->col_number)
							break;

						cur_col = cur_col->next;
					}

					if (cur_change->data != NULL || (cur_col != NULL && (cur_col->data_type == DATA_STRING || cur_col->data_type == DATA_DATE)))
					{
						if (the_debug == YES_DEBUG)
							printf("	ERROR in parseInsert() at line %d in %s\n", __LINE__, __FILE__);
						errorTeardown(NULL, malloced_head, the_debug);
						*change_head = NULL;
						*table = NULL;
						return -1;
					}


					cur_change->data = (char*) myMalloc(sizeof(char) * 64, NULL, malloced_head, the_debug);
					if (cur_change->data == NULL)
					{
						if (the_debug == YES_DEBUG)
							printf("	ERROR in parseInsert() at line %d in %s\n", __LINE__, __FILE__);
						errorTeardown(NULL, malloced_head, the_debug);
						*change_head = NULL;
						*table = NULL;
						return -1;
					}
					strcpy(cur_change->data, word);
				}
			}
		}
	}

	myFree((void**) &word, NULL, malloced_head, the_debug);

	freeListNodesV2(&list_tail, NULL, malloced_head, the_debug);

	return 0;
}*/

int parseOnClauseOfSelect(struct join_node** join_tail, char** on_clause, struct select_node** select_head
						 ,struct malloced_node** malloced_head, int the_debug)
{
	printf("PARSING on_clause: _%s_\n", *on_clause);

	struct where_clause_node* the_on_clause = NULL;
	int error_code = parseWhereClause(*on_clause, &the_on_clause, *select_head, "on", malloced_head, the_debug);

	if (error_code != 0)
	{
		if (the_debug == YES_DEBUG)
			printf("	ERROR in parseOnClauseOfSelect() at line %d in %s\n", __LINE__, __FILE__);
		return -1;
	}

	(*join_tail)->on_clause_head = the_on_clause;

	myFree((void**) on_clause, NULL, malloced_head, the_debug);

	return 0;
}


int getSubSelect(char* input, char** word, int* index, char** sub_select)
{
	int parens = 0;

	while (getNextWord(input, *word, index) == 0 && (*word)[0] != ';' && (parens > 0 || (*word)[0] != ')'))
	{
		printf("word in sub_select loop = _%s_\n", *word);
		//printf("hmm = _%d_\n", parens);


		if ((*word)[0] == '(')
			parens++;
		else if ((*word)[0] == ')')
			parens--;

		strcat(*sub_select, *word);
		if (input[*index] == '.')
		{
			strcat(*sub_select, ".");
			*index++;
		}
		else
			strcat(*sub_select, " ");
	}
	

	return 0;
}

int parseMathNode(struct math_node** the_math_node, char** new_col_name, char** col_name, char* input, char* word, int index, struct select_node* select_node
				 ,struct malloced_node** malloced_head, int the_debug)
{
	if (new_col_name != NULL)
		*new_col_name = (char*) myMalloc(sizeof(char) * 256, NULL, malloced_head, the_debug);

	*the_math_node = myMalloc(sizeof(struct math_node), NULL, malloced_head, the_debug);

	int open_parens = 0;

	struct math_node* cur_math = NULL;

	if (word[0] == '(' && *col_name == NULL)
	{
		// START Open parens so math a new math_node at ptr_one and traverse to it
			//printf("Making new math_node at ptr_one\n");
			if (new_col_name != NULL)
				strcpy(*new_col_name, "( ");

			open_parens++;

			(*the_math_node)->ptr_one = myMalloc(sizeof(struct math_node), NULL, malloced_head, the_debug);
			(*the_math_node)->ptr_one_type = PTR_TYPE_MATH_NODE;

			(*the_math_node)->ptr_two = NULL;
			(*the_math_node)->ptr_two_type = -1;

			(*the_math_node)->operation = -1;
			(*the_math_node)->parent = NULL;

			((struct math_node*) (*the_math_node)->ptr_one)->parent = *the_math_node;

			cur_math = (*the_math_node)->ptr_one;

			cur_math->ptr_one = NULL;
			cur_math->ptr_one_type = -1;

			cur_math->ptr_two = NULL;
			cur_math->ptr_two_type = -1;

			cur_math->operation = -1;
		// END Open parens so math a new math_node at ptr_one and traverse to it
	}
	else if ((word[0] == '+' || word[0] == '-' || word[0] == '*' || word[0] == '/' || word[0] == '^') && *col_name != NULL)
	{
		if (new_col_name != NULL)
		{
			char* temp = upper(*col_name, NULL, malloced_head, the_debug);
			strcpy(*new_col_name, temp);
			myFree((void**) &temp, NULL, malloced_head, the_debug);
			strcat(*new_col_name, " ");

			strcat(*new_col_name, word);
			strcat(*new_col_name, " ");
		}

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

		// START Determine type of previous word
			//printf("Assigning to ptr_one\n");

			int hmm_int = atoi(*col_name);

			if (hmm_int > 0 && strcontains(*col_name, '.') == 1)
			{
				// Double
				(*the_math_node)->ptr_one = myMalloc(sizeof(double), NULL, malloced_head, the_debug);

				double hmm_double = 0.0;

				sscanf(*col_name, "%lf", &hmm_double);

				*((double*) (*the_math_node)->ptr_one) = hmm_double;

				(*the_math_node)->ptr_one_type = PTR_TYPE_REAL;
			}
			else if (hmm_int > 0)
			{
				// Int
				(*the_math_node)->ptr_one = myMalloc(sizeof(int), NULL, malloced_head, the_debug);

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

				int index_of_dot = indexOf(name, '.');
				if (index_of_dot > -1)
				{
					alias = substring(name, 0, index_of_dot-1, NULL, malloced_head, the_debug);
					
					char* temp = name;

					name = substring(temp, index_of_dot+1, strLength(temp)-1, NULL, malloced_head, the_debug);

					myFree((void**) &temp, NULL, malloced_head, the_debug);
				}
				printf("alias.name = _%s.%s_\n", alias, name);

				if (getColInSelectNodeFromName((void*) *the_math_node, PTR_TYPE_MATH_NODE, 1, 1, select_node, alias, name
											  ,malloced_head, the_debug) != 0)
				{
					errorTeardown(NULL, malloced_head, the_debug);
					return -1;
				}

				if (alias != NULL)
					myFree((void**) &alias, NULL, malloced_head, the_debug);
				if (name != NULL)
					myFree((void**) &name, NULL, malloced_head, the_debug);

				// START Check if found col has valid datatype
					if (((struct table_cols_info*) ((struct col_in_select_node*) (*the_math_node)->ptr_one)->col_ptr)->data_type == DATA_STRING)
					{
						if (the_debug == YES_DEBUG)
							printf("	ERROR in parseMathNode() at line %d in %s\n", __LINE__, __FILE__);
						errorTeardown(NULL, malloced_head, the_debug);
						return -1;
					}
				// END Check if found col has valid datatype
			}
		// END Determine type of previous word

		(*the_math_node)->ptr_two = NULL;
		(*the_math_node)->ptr_two_type = -1;

		(*the_math_node)->parent = NULL;

		cur_math = *the_math_node;

		if ((*the_math_node)->ptr_one_type != PTR_TYPE_CHAR)
			myFree((void**) col_name, NULL, malloced_head, the_debug);

		*col_name = NULL;
	}


	bool made_new_without_parens = false;

	while (getNextWord(input, word, &index) == 0 && word[0] != ',' && strcmp_Upper(word, "FROM", NULL, malloced_head, the_debug) != 0
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
			strcpy(temp_word, word);

			index++;
			getNextWord(input, word, &index);

			strcat(temp_word, word);

			strcpy(word, temp_word);

			myFree((void**) &temp_word, NULL, malloced_head, the_debug);
		}

		printf("    word in get math loop = _%s_\n", word);

		if (new_col_name != NULL)
		{
			char* temp = upper(word, NULL, malloced_head, the_debug);
			strcat(*new_col_name, temp);
			myFree((void**) &temp, NULL, malloced_head, the_debug);
			strcat(*new_col_name, " ");
		}

		if (word[0] == '(')
		{
			// START Open parens so math a new math_node at ptr_one and traverse to it
				if (cur_math->ptr_one != NULL)
				{
					//printf("	Making new math_node at ptr_two\n");

					open_parens++;

					cur_math->ptr_two = myMalloc(sizeof(struct math_node), NULL, malloced_head, the_debug);
					cur_math->ptr_two_type = PTR_TYPE_MATH_NODE;

					((struct math_node*) cur_math->ptr_two)->parent = cur_math;

					cur_math = cur_math->ptr_two;
				}
				else
				{
					//printf("	Making new math_node at ptr_one\n");

					open_parens++;

					cur_math->ptr_one = myMalloc(sizeof(struct math_node), NULL, malloced_head, the_debug);
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
		}
		else if (word[0] == ')')
		{
			// START Close parens so traverse to parent
				if (cur_math->ptr_one == NULL || cur_math->ptr_two == NULL || cur_math->operation == -1)
				{
					if (the_debug == YES_DEBUG)
						printf("	ERROR in parseMathNode() at line %d in %s\n", __LINE__, __FILE__);
					errorTeardown(NULL, malloced_head, the_debug);
					return -1;
				}

				open_parens--;

				cur_math = cur_math->parent;

				//printf("	Going back to parent, op now = %d\n", cur_math->operation);
			// END Close parens so traverse to parent
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
						//printf("Made new without parens, opt 1\n");

						void* temp_ptr_two = cur_math->ptr_two;
						int temp_ptr_two_type = cur_math->ptr_two_type;

						cur_math->ptr_two = myMalloc(sizeof(struct math_node), NULL, malloced_head, the_debug);
						cur_math->ptr_two_type = PTR_TYPE_MATH_NODE;

						((struct math_node*) cur_math->ptr_two)->parent = cur_math;

						((struct math_node*) cur_math->ptr_two)->ptr_one = temp_ptr_two;
						((struct math_node*) cur_math->ptr_two)->ptr_one_type = temp_ptr_two_type;

						((struct math_node*) cur_math->ptr_two)->ptr_two = NULL;
						((struct math_node*) cur_math->ptr_two)->ptr_two_type = -1;

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
						//printf("Made new without parens, opt 2\n");

						void* temp_ptr_one = cur_math->ptr_one;
						int temp_ptr_one_type = cur_math->ptr_one_type;

						void* temp_ptr_two = cur_math->ptr_two;
						int temp_ptr_two_type = cur_math->ptr_two_type;

						int temp_operation = cur_math->operation;

						cur_math->ptr_one = myMalloc(sizeof(struct math_node), NULL, malloced_head, the_debug);
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
					// END Is a lesser operation, make previous two ptrs a new child math_node
				}
				else
				{
					if (the_debug == YES_DEBUG)
						printf("	ERROR in parseMathNode() at line %d in %s\n", __LINE__, __FILE__);
					errorTeardown(NULL, malloced_head, the_debug);
					return -1;
				}
			}
			else
			{
				//printf("	Assiging operation\n");
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
		else if (cur_math->ptr_one != NULL && cur_math->ptr_two == NULL)
		{
			// START Determine type of next word and put into ptr_two
				//printf("	Assigning to ptr_two\n");
				int hmm_int = atoi(word);

				if (hmm_int > 0 && strcontains(word, '.'))
				{
					// Double
					cur_math->ptr_two = myMalloc(sizeof(double), NULL, malloced_head, the_debug);

					double hmm_double = 0.0;

					sscanf(word, "%lf", &hmm_double);

					*((double*) cur_math->ptr_two) = hmm_double;

					cur_math->ptr_two_type = PTR_TYPE_REAL;
				}
				else if (hmm_int > 0)
				{
					// Int
					cur_math->ptr_two = myMalloc(sizeof(int), NULL, malloced_head, the_debug);

					*((int*) cur_math->ptr_two) = hmm_int;

					cur_math->ptr_two_type = PTR_TYPE_INT;
				}
				else if (strcontains(word, '/'))
				{
					// Date
					cur_math->ptr_two = upper(word, NULL, malloced_head, the_debug);

					cur_math->ptr_two_type = PTR_TYPE_DATE;
				}
				else
				{
					// Char but column name
					//cur_math->ptr_two = upper(word, NULL, malloced_head, the_debug);
					//cur_math->ptr_two_type = PTR_TYPE_CHAR;

					char* alias = NULL;
					char* name = upper(word, NULL, malloced_head, the_debug);

					int index_of_dot = indexOf(name, '.');
					if (index_of_dot > -1)
					{
						alias = substring(name, 0, index_of_dot-1, NULL, malloced_head, the_debug);
						
						char* temp = name;

						name = substring(temp, index_of_dot+1, strLength(temp)-1, NULL, malloced_head, the_debug);

						myFree((void**) &temp, NULL, malloced_head, the_debug);
					}
					printf("alias.name = _%s.%s_\n", alias, name);

					if (getColInSelectNodeFromName((void*) cur_math, PTR_TYPE_MATH_NODE, 2, 1, select_node, alias, name
												  ,malloced_head, the_debug) != 0)
					{
						errorTeardown(NULL, malloced_head, the_debug);
						return -1;
					}

					if (alias != NULL)
						myFree((void**) &alias, NULL, malloced_head, the_debug);
					if (name != NULL)
						myFree((void**) &name, NULL, malloced_head, the_debug);


					// START Check if found col has valid datatype
						if (((struct table_cols_info*) ((struct col_in_select_node*) cur_math->ptr_two)->col_ptr)->data_type == DATA_STRING)
						{
							if (the_debug == YES_DEBUG)
								printf("	ERROR in parseMathNode() at line %d in %s\n", __LINE__, __FILE__);
							errorTeardown(NULL, malloced_head, the_debug);
							return -1;
						}
					// END Check if found col has valid datatype
				}

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
				int hmm_int = atoi(word);

				if (hmm_int > 0 && strcontains(word, '.'))
				{
					// Double
					cur_math->ptr_one = myMalloc(sizeof(double), NULL, malloced_head, the_debug);

					double hmm_double = 0.0;

					sscanf(word, "%lf", &hmm_double);

					*((double*) cur_math->ptr_one) = hmm_double;

					cur_math->ptr_one_type = PTR_TYPE_REAL;
				}
				else if (hmm_int > 0)
				{
					// Int
					cur_math->ptr_one = myMalloc(sizeof(int), NULL, malloced_head, the_debug);

					*((int*) cur_math->ptr_one) = hmm_int;

					cur_math->ptr_one_type = PTR_TYPE_INT;
				}
				else if (strcontains(word, '/'))
				{
					// Date
					cur_math->ptr_one = upper(word, NULL, malloced_head, the_debug);

					cur_math->ptr_one_type = PTR_TYPE_DATE;
				}
				else
				{
					// Char but column name
					//cur_math->ptr_one = upper(word, NULL, malloced_head, the_debug);
					//cur_math->ptr_one_type = PTR_TYPE_CHAR;

					char* alias = NULL;
					char* name = upper(word, NULL, malloced_head, the_debug);

					int index_of_dot = indexOf(name, '.');
					if (index_of_dot > -1)
					{
						alias = substring(name, 0, index_of_dot-1, NULL, malloced_head, the_debug);
						
						char* temp = name;

						name = substring(temp, index_of_dot+1, strLength(temp)-1, NULL, malloced_head, the_debug);

						myFree((void**) &temp, NULL, malloced_head, the_debug);
					}
					printf("alias.name = _%s.%s_\n", alias, name);

					if (getColInSelectNodeFromName((void*) cur_math, PTR_TYPE_MATH_NODE, 1, 1, select_node, alias, name
												  ,malloced_head, the_debug) != 0)
					{
						errorTeardown(NULL, malloced_head, the_debug);
						return -1;
					}

					if (alias != NULL)
						myFree((void**) &alias, NULL, malloced_head, the_debug);
					if (name != NULL)
						myFree((void**) &name, NULL, malloced_head, the_debug);
					

					// START Check if found col has valid datatype
						if (((struct table_cols_info*) ((struct col_in_select_node*) cur_math->ptr_one)->col_ptr)->data_type == DATA_STRING)
						{
							if (the_debug == YES_DEBUG)
								printf("	ERROR in parseMathNode() at line %d in %s\n", __LINE__, __FILE__);
							errorTeardown(NULL, malloced_head, the_debug);
							return -1;
						}
					// END Check if found col has valid datatype
				}
			// END Determine type of next word and put into ptr_one
		}
		else if (input[index] == 34)
		{
			// START Column is given a new name with double quotes
				if (new_col_name != NULL)
				{
					printf("math_node col is given new name with quotes\n");
					myFree((void**) new_col_name, NULL, malloced_head, the_debug);

					*new_col_name = upper(word, NULL, malloced_head, the_debug);

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
					printf("math_node col is given new name\n");
					myFree((void**) new_col_name, NULL, malloced_head, the_debug);

					if (strcmp_Upper(word, "AS", NULL, malloced_head, the_debug) == 0)
						getNextWord(input, word, &index);

					*new_col_name = upper(word, NULL, malloced_head, the_debug);

					if (*new_col_name == NULL || (*new_col_name)[0] == '(' || (*new_col_name)[0] == ')')
					{
						if (the_debug == YES_DEBUG)
							printf("	ERROR in parseMathNode() at line %d in %s\n", __LINE__, __FILE__);
						errorTeardown(NULL, malloced_head, the_debug);
						return -1;
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
						errorTeardown(NULL, malloced_head, the_debug);
						return -1;
					}
				}
			// END Column is given a new name
		}
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

	return 0;
}

int parseNewColumnName(char** new_col_name, char* input, char** word, int* index
					  ,struct malloced_node** malloced_head, int the_debug)
{
	printf("Function column is given a new name: _%s_\n", *word);
	if (*new_col_name != NULL)
		myFree((void**) new_col_name, NULL, malloced_head, the_debug);

	if (strcmp_Upper(*word, "AS", NULL, malloced_head, the_debug) == 0)
		getNextWord(input, *word, index);

	char* temp_upper = NULL;
	if ((*word)[0] != 34)
	{
		temp_upper = upper(*word, NULL, malloced_head, the_debug);
	}
	else
	{
		temp_upper = myMalloc(sizeof(char) * (strLength(*word)+1), NULL, malloced_head, the_debug);
		strcpy(temp_upper, *word);
	}

	*new_col_name = (char*) myMalloc(sizeof(char) * 256, NULL, malloced_head, the_debug);
	if (*new_col_name == NULL)
	{
		if (the_debug == YES_DEBUG)
			printf("	ERROR in parseNewColumnName() at line %d in %s\n", __LINE__, __FILE__);
		return -1;
	}

	strcpy(*new_col_name, temp_upper);

	myFree((void**) &temp_upper, NULL, malloced_head, the_debug);

	trimStr(*new_col_name);

	if (atoi(*new_col_name) > 0)
	{
		if (the_debug == YES_DEBUG)
			printf("	ERROR in parseNewColumnName() at line %d in %s\n", __LINE__, __FILE__);
		errorTeardown(NULL, malloced_head, the_debug);
		return -1;
	}

	return 0;
}

/*	 
 *	Writes to (so calling function can read):
 *		struct select_node** select_node;
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
			return -1;
		}

		(*select_node)->select_node_alias = NULL;
		(*select_node)->distinct = false;

		(*select_node)->columns_arr_size = 0;
		(*select_node)->columns_arr = NULL;

		(*select_node)->where_head = NULL;
		(*select_node)->join_head = NULL;

		(*select_node)->prev = NULL;
		(*select_node)->next = NULL;
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
			return -1;
		}
		word[0] = 0;
		int index = 0;
	// END Init word variable

	// START See if first word is SELECT or WITH
		getNextWord(input, word, &index);

		int compared = strcmp_Upper(word, "SELECT", NULL, malloced_head, the_debug);

		if (compared != 0)
		{
			if (strcmp_Upper(word, "WITH", NULL, malloced_head, the_debug) == 0)
			{
				if (in_with)
				{
					if (the_debug == YES_DEBUG)
						printf("	ERROR in parseSelect() at line %d in %s\n", __LINE__, __FILE__);
					errorTeardown(NULL, malloced_head, the_debug);
					*select_node = NULL;
					return -1;
				}

				char* sub_select = (char*) myMalloc(sizeof(char) * 1000, NULL, malloced_head, the_debug);
				strcpy(sub_select, "");

				while (strcmp_Upper(word, "WITH", NULL, malloced_head, the_debug) == 0 || word[0] == ',')
				{
					getNextWord(input, word, &index);

					char* sub_select_alias = upper(word, NULL, malloced_head, the_debug);


					getNextWord(input, word, &index);

					if (strcmp_Upper(word, "AS", NULL, malloced_head, the_debug) != 0)
					{
						if (the_debug == YES_DEBUG)
							printf("	ERROR in parseSelect() at line %d in %s\n", __LINE__, __FILE__);
						errorTeardown(NULL, malloced_head, the_debug);
						*select_node = NULL;
						return -1;
					}

					getNextWord(input, word, &index);

					if (word[0] != '(')
					{
						if (the_debug == YES_DEBUG)
							printf("	ERROR in parseSelect() at line %d in %s\n", __LINE__, __FILE__);
						errorTeardown(NULL, malloced_head, the_debug);
						*select_node = NULL;
						return -1;
					}


					getSubSelect(input, &word, &index, &sub_select);

					printf("sub_select = _%s_\n", sub_select);

					printf("sub_select_alias = _%s_\n", sub_select_alias);

					printf("word after sub_select = _%s_\n", word);

					if (word[0] == ')')
					{
						getNextWord(input, word, &index);
					}
					else
					{
						if (the_debug == YES_DEBUG)
							printf("	ERROR in parseSelect() at line %d in %s\n", __LINE__, __FILE__);
						errorTeardown(NULL, malloced_head, the_debug);
						*select_node = NULL;
						return -1;
					}

					struct select_node* new_sub_select = NULL;
					

					if (parseSelect(sub_select, &new_sub_select, with_sub_select_head, true, malloced_head, the_debug) != 0)
					{
						if (the_debug == YES_DEBUG)
							printf("	ERROR in parseSelect() at line %d in %s\n", __LINE__, __FILE__);
						*select_node = NULL;
						return -1;
					}


					sub_select[0] = 0;


					while (new_sub_select->next != NULL)
						new_sub_select = new_sub_select->next;


					new_sub_select->select_node_alias = sub_select_alias;


					printf("new_sub_select col_ptr_type[0] = %d\n", new_sub_select->columns_arr[0]->col_ptr_type);
					printf("new_sub_select->prev col_ptr_type[0] = %d\n", new_sub_select->prev->columns_arr[0]->col_ptr_type);

					printf("new_sub_select alias = _%s_\n", new_sub_select->select_node_alias);
					printf("new_sub_select->prev alias = _%s_\n", new_sub_select->prev->select_node_alias);


					addListNodePtr(&with_sub_select_head, &with_sub_select_tail, new_sub_select, -1, ADDLISTNODE_TAIL
								  ,NULL, malloced_head, the_debug);
				}

				myFree((void**) &sub_select, NULL, malloced_head, the_debug);

				if (strcmp_Upper(word, "SELECT", NULL, malloced_head, the_debug) != 0)
				{
					if (the_debug == YES_DEBUG)
						printf("	ERROR in parseSelect() at line %d in %s\n", __LINE__, __FILE__);
					errorTeardown(NULL, malloced_head, the_debug);
					*select_node = NULL;
					return -1;
				}
			}	
			else
			{
				if (the_debug == YES_DEBUG)
					printf("	ERROR in parseSelect() at line %d in %s\n", __LINE__, __FILE__);
				errorTeardown(NULL, malloced_head, the_debug);
				*select_node = NULL;
				return -1;
			}
		}
	// END See if first word is SELECT or WITH

	// START See if next word is DISTINCT
		getNextWord(input, word, &index);

		compared = strcmp_Upper(word, "DISTINCT", NULL, malloced_head, the_debug);

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
		strcpy(cols_str, "");

		while (getNextWord(input, word, &index) == 0 && (compared = strcmp_Upper(word, "FROM", NULL, malloced_head, the_debug)) != 0)
		{
			printf("word = _%s_\n", word);

			if (compared == -2)
			{
				if (the_debug == YES_DEBUG)
					printf("	ERROR in parseSelect() at line %d in %s\n", __LINE__, __FILE__);
				*select_node = NULL;
				return -1;
			}

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

		printf("cols_str = _%s_\n", cols_str);

		int size_result = 0;
		char** cols_strs_arr = strSplitV2(cols_str, ',', &size_result, NULL, malloced_head, the_debug);

		myFree((void**) &cols_str, NULL, malloced_head, the_debug);
	// END Put columns in substring to read after getting tables, joins


	// START Get table, alias of table, joins, and subqueries
		struct select_node* select_tail = NULL;

		struct join_node* join_tail = NULL;

		char* on_clause = NULL;

		char* sub_select = NULL;

		while (getNextWord(input, word, &index) == 0 && word[0] != ';')
		{
			printf("word in loop = _%s_\n", word);

			if (strcmp_Upper(word, "WHERE", NULL, malloced_head, the_debug) == 0 || strcmp_Upper(word, "GROUP", NULL, malloced_head, the_debug) == 0)
			{
				// START Parse where clause and break
					if (on_clause != NULL)
					{
						if (parseOnClauseOfSelect(&join_tail, &on_clause, select_node, malloced_head, the_debug) != 0)
						{
							if (the_debug == YES_DEBUG)
								printf("	ERROR in parseSelect() at line %d in %s\n", __LINE__, __FILE__);
							*select_node = NULL;
							return -1;
						}
					}

					break;
				// END Parse where clause and break
			}
			else if (word[0] == '(')
			{
				// START Setup sub select and recursively call this function
					char* sub_select = (char*) myMalloc(sizeof(char) * 1000, NULL, malloced_head, the_debug);
					strcpy(sub_select, "");


					getSubSelect(input, &word, &index, &sub_select);

					printf("sub_select = _%s_\n", sub_select);


					struct select_node* new_sub_select = NULL;
					

					if (parseSelect(sub_select, &new_sub_select, with_sub_select_head, true, malloced_head, the_debug) != 0)
					{
						if (the_debug == YES_DEBUG)
							printf("	ERROR in parseSelect() at line %d in %s\n", __LINE__, __FILE__);
						*select_node = NULL;
						return -1;
					}


					while (new_sub_select->next != NULL)
						new_sub_select = new_sub_select->next;


					if (join_tail == NULL)
					{
						// Need to add to select_tail->prev
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
						printf("Added new_sub_select to join_tail->select_joined\n");

						join_tail->select_joined = new_sub_select;
					}

					myFree((void**) &sub_select, NULL, malloced_head, the_debug);
				// START Setup sub select and recursively call this function
			}
			else if (strcmp_Upper(word, "INNER", NULL, malloced_head, the_debug) == 0 
					|| strcmp_Upper(word, "OUTER", NULL, malloced_head, the_debug) == 0 
					|| strcmp_Upper(word, "LEFT", NULL, malloced_head, the_debug) == 0 
					|| strcmp_Upper(word, "RIGHT", NULL, malloced_head, the_debug) == 0
					|| strcmp_Upper(word, "JOIN", NULL, malloced_head, the_debug) == 0)
			{
				// START Setup join node
					if (on_clause != NULL)
					{
						if (parseOnClauseOfSelect(&join_tail, &on_clause, select_node, malloced_head, the_debug) != 0)
						{
							if (the_debug == YES_DEBUG)
								printf("	ERROR in parseSelect() at line %d in %s\n", __LINE__, __FILE__);
							*select_node = NULL;
							return -1;
						}
					}

					// START Add another join_node to select_node
						if ((*select_node)->join_head == NULL)
						{
							(*select_node)->join_head = (struct join_node*) myMalloc(sizeof(struct join_node), NULL, malloced_head, the_debug);
							join_tail = (*select_node)->join_head;

							join_tail->prev = NULL;
							join_tail->next = NULL;
						}
						else
						{
							join_tail->next = (struct join_node*) myMalloc(sizeof(struct join_node), NULL, malloced_head, the_debug);

							join_tail->next->prev = join_tail;
							join_tail->next->next = NULL;

							join_tail = join_tail->next;
						}

						join_tail->select_joined = NULL;

						join_tail->on_clause_head = NULL;

						if (strcmp_Upper(word, "INNER", NULL, malloced_head, the_debug) == 0 || strcmp_Upper(word, "JOIN", NULL, malloced_head, the_debug) == 0)
							join_tail->join_type = JOIN_INNER;
						else if (strcmp_Upper(word, "OUTER", NULL, malloced_head, the_debug) == 0)
							join_tail->join_type = JOIN_OUTER;
						else if (strcmp_Upper(word, "LEFT", NULL, malloced_head, the_debug) == 0)
							join_tail->join_type = JOIN_LEFT;
						else if (strcmp_Upper(word, "RIGHT", NULL, malloced_head, the_debug) == 0)
							join_tail->join_type = JOIN_RIGHT;
					// END Add another join_node to select_node


					// START Check to ensure that next word is "join"
						if (strcmp_Upper(word, "JOIN", NULL, malloced_head, the_debug) != 0)
						{
							getNextWord(input, word, &index);
							printf("word in loop = _%s_\n", word);

							if (strcmp_Upper(word, "JOIN", NULL, malloced_head, the_debug) != 0)
							{
								if (the_debug == YES_DEBUG)
									printf("	ERROR in parseSelect() at line %d in %s\n", __LINE__, __FILE__);
								errorTeardown(NULL, malloced_head, the_debug);
								*select_node = NULL;
								return -1;
							}
						}
					// END Check to ensure that next word is "join"
				// END Setup join node
			}
			else if (strcmp_Upper(word, "ON", NULL, malloced_head, the_debug) == 0 || on_clause != NULL)
			{
				// START Concat word to on clause
					if (on_clause == NULL)
					{
						on_clause = (char*) myMalloc(sizeof(char) * 1000, NULL, malloced_head, the_debug);

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
			else if (select_tail != NULL && printf("select_tail->select_node_alias = _%s_\n", select_tail->select_node_alias) && select_tail->select_node_alias == NULL)
			{
				if (strcmp_Upper(word, "AS", NULL, malloced_head, the_debug) == 0)
					getNextWord(input, word, &index);

				printf("Adding alias to select_tail\n");
				select_tail->select_node_alias = upper(word, NULL, malloced_head, the_debug);
			}
			else if (join_tail != NULL && join_tail->select_joined != NULL && join_tail->select_joined->select_node_alias == NULL)
			{
				if (strcmp_Upper(word, "AS", NULL, malloced_head, the_debug) == 0)
					getNextWord(input, word, &index);

				printf("Adding alias to join_tail\n");
				join_tail->select_joined->select_node_alias = upper(word, NULL, malloced_head, the_debug);
			}
			else if (join_tail != NULL && join_tail->select_joined == NULL)
			{
				// START Add joined table at lowest level
					printf("Adding JOINED table at lowest level\n");

					struct table_info* the_table = getTableFromName(word, malloced_head, the_debug);

					if (the_table == NULL && with_sub_select_head == NULL)
					{
						if (the_debug == YES_DEBUG)
							printf("	ERROR in parseSelect() at line %d in %s\n", __LINE__, __FILE__);
						errorTeardown(NULL, malloced_head, the_debug);
						*select_node = NULL;
						return -1;
					}
					else if (the_table == NULL && with_sub_select_head != NULL)
					{
						struct ListNodePtr* cur_with = with_sub_select_head;
						while (cur_with != NULL)
						{
							if (strcmp_Upper(word, ((struct select_node*) cur_with->ptr_value)->select_node_alias, NULL, malloced_head, the_debug) == 0)
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
							errorTeardown(NULL, malloced_head, the_debug);
							*select_node = NULL;
							return -1;
						}
					}
					else if (the_table != NULL)
					{
						join_tail->select_joined = (struct select_node*) myMalloc(sizeof(struct select_node), NULL, malloced_head, the_debug);

						join_tail->select_joined->select_node_alias = NULL;
						join_tail->select_joined->distinct = false;

						join_tail->select_joined->columns_arr_size = the_table->num_cols;
						join_tail->select_joined->columns_arr = (struct col_in_select_node**) myMalloc(sizeof(struct col_in_select_node*) * the_table->num_cols, NULL, malloced_head, the_debug);

						struct table_cols_info* cur_col = the_table->table_cols_head;
						for (int i=0; i<the_table->num_cols; i++)
						{
							join_tail->select_joined->columns_arr[i] = (struct col_in_select_node*) myMalloc(sizeof(struct col_in_select_node), NULL, malloced_head, the_debug);
							
							join_tail->select_joined->columns_arr[i]->table_ptr = the_table;
							join_tail->select_joined->columns_arr[i]->table_ptr_type = PTR_TYPE_TABLE_INFO;

							join_tail->select_joined->columns_arr[i]->col_ptr = cur_col;
							join_tail->select_joined->columns_arr[i]->col_ptr_type = PTR_TYPE_TABLE_COLS_INFO;

							join_tail->select_joined->columns_arr[i]->new_name = NULL;
							join_tail->select_joined->columns_arr[i]->func_node = NULL;
							join_tail->select_joined->columns_arr[i]->math_node = NULL;
							join_tail->select_joined->columns_arr[i]->case_node = NULL;

							cur_col = cur_col->next;
						}

						join_tail->select_joined->where_head = NULL;
						join_tail->select_joined->join_head = NULL;

						join_tail->select_joined->next = NULL;
						join_tail->select_joined->prev = NULL;
					}
				// END Add joined table at lowest level
			}
			else
			{
				// START Add from table at lowest level
					printf("Adding table at lowest level\n");

					struct table_info* the_table = getTableFromName(word, malloced_head, the_debug);

					if (the_table == NULL && with_sub_select_head == NULL)
					{
						if (the_debug == YES_DEBUG)
							printf("	ERROR in parseSelect() at line %d in %s\n", __LINE__, __FILE__);
						errorTeardown(NULL, malloced_head, the_debug);
						*select_node = NULL;
						return -1;
					}
					else if (the_table == NULL && with_sub_select_head != NULL)
					{
						struct ListNodePtr* cur_with = with_sub_select_head;
						while (cur_with != NULL)
						{
							if (strcmp_Upper(word, ((struct select_node*) cur_with->ptr_value)->select_node_alias, NULL, malloced_head, the_debug) == 0)
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
							errorTeardown(NULL, malloced_head, the_debug);
							*select_node = NULL;
							return -1;
						}
					}
					else if (the_table != NULL)
					{
						if (select_tail == NULL)
							select_tail = *select_node;

						while (select_tail->prev != NULL)
							select_tail = select_tail->prev;

						select_tail->prev = (struct select_node*) myMalloc(sizeof(struct select_node), NULL, malloced_head, the_debug);

						select_tail->prev->select_node_alias = NULL;
						select_tail->prev->distinct = false;

						select_tail->prev->columns_arr_size = the_table->num_cols;
						select_tail->prev->columns_arr = (struct col_in_select_node**) myMalloc(sizeof(struct col_in_select_node*) * select_tail->prev->columns_arr_size, NULL, malloced_head, the_debug);
				
						struct table_cols_info* cur_col = the_table->table_cols_head;
						for (int i=0; i<select_tail->prev->columns_arr_size; i++)
						{
							select_tail->prev->columns_arr[i] = (struct col_in_select_node*) myMalloc(sizeof(struct col_in_select_node), NULL, malloced_head, the_debug);
							
							select_tail->prev->columns_arr[i]->table_ptr = the_table;
							select_tail->prev->columns_arr[i]->table_ptr_type = PTR_TYPE_TABLE_INFO;

							select_tail->prev->columns_arr[i]->col_ptr = cur_col;
							select_tail->prev->columns_arr[i]->col_ptr_type = PTR_TYPE_TABLE_COLS_INFO;

							select_tail->prev->columns_arr[i]->new_name = NULL;
							select_tail->prev->columns_arr[i]->func_node = NULL;
							select_tail->prev->columns_arr[i]->math_node = NULL;
							select_tail->prev->columns_arr[i]->case_node = NULL;

							cur_col = cur_col->next;
						}

						select_tail->prev->where_head = NULL;
						select_tail->prev->join_head = NULL;

						select_tail->prev->next = select_tail;
						select_tail->prev->prev = NULL;

						select_tail = select_tail->prev;
					}
				// END Add from table at lowest level
			}
		}
		printf("Out of join loop\n");

		if (on_clause != NULL)
		{
			if (parseOnClauseOfSelect(&join_tail, &on_clause, select_node, malloced_head, the_debug) != 0)
			{
				if (the_debug == YES_DEBUG)
					printf("	ERROR in parseSelect() at line %d in %s\n", __LINE__, __FILE__);
				*select_node = NULL;
				return -1;
			}
		}


		if ((*select_node)->prev != NULL && (*select_node)->prev->prev != NULL && (*select_node)->prev->select_node_alias == NULL)
		{
			if (the_debug == YES_DEBUG)
				printf("	ERROR in parseSelect() at line %d in %s\n", __LINE__, __FILE__);
			errorTeardown(NULL, malloced_head, the_debug);
			*select_node = NULL;
			return -1;
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
				return -1;
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

					for (int j=0; j<(*select_node)->columns_arr_size; j++)
					{
						(*select_node)->columns_arr[j] = (struct col_in_select_node*) myMalloc(sizeof(struct col_in_select_node), NULL, malloced_head, the_debug);
						(*select_node)->columns_arr[j]->table_ptr = NULL;
						(*select_node)->columns_arr[j]->table_ptr_type = -1;
						(*select_node)->columns_arr[j]->col_ptr = NULL;
						(*select_node)->columns_arr[j]->col_ptr_type = -1;
						(*select_node)->columns_arr[j]->new_name = NULL;
						(*select_node)->columns_arr[j]->func_node = NULL;
						(*select_node)->columns_arr[j]->math_node = NULL;
						(*select_node)->columns_arr[j]->case_node = NULL;
					}

					printf("-----------------\n");
					printf("	Init columns_arr with (*select_node)->columns_arr_size = %d\n", (*select_node)->columns_arr_size);
				}
			// END Init columns arrays

			bool except = false;

			for (int arr_k=0; arr_k<size_result; arr_k++)
			{
				if ((*select_node)->prev == NULL)
				{
					if (the_debug == YES_DEBUG)
						printf("	ERROR in parseSelect() at line %d in %s\n", __LINE__, __FILE__);
					errorTeardown(NULL, malloced_head, the_debug);
					*select_node = NULL;
					return -1;
				}

				printf("cols_strs_arr[arr_k] = _%s_\n", cols_strs_arr[arr_k]);

				// START Parse column string
					int col_word_index = 0;
					char* col_word = (char*) myMalloc(sizeof(char) * 100, NULL, malloced_head, the_debug);

					char* alias = NULL;
					char* col_name = NULL;
					char* new_col_name = NULL;

					struct func_node* col_func_node = NULL;
					struct math_node* col_math_node = NULL;
					struct case_node* col_case_node = NULL;

					while (getNextWord(cols_strs_arr[arr_k], col_word, &col_word_index) == 0)
					{
						printf("word in col_word loop = _%s_\n", col_word);

						if (strcmp_Upper(col_word, "CASE", NULL, malloced_head, the_debug) == 0)
						{
							// START Parse case statement
								if (i == 0)
								{
									col_case_node = (struct case_node*) myMalloc(sizeof(struct case_node), NULL, malloced_head, the_debug);
								}
								else
								{
									col_case_node = (struct case_node*) myMalloc(sizeof(struct case_node), NULL, malloced_head, the_debug);

									col_case_node->case_when_head = NULL;
									col_case_node->case_when_tail = NULL;

									col_case_node->case_then_value_head = NULL;
									col_case_node->case_then_value_tail = NULL;


									char* case_when_str = (char*) myMalloc(sizeof(char) * 1000, NULL, malloced_head, the_debug);
									case_when_str[0] = 0;

									while (getNextWord(cols_strs_arr[arr_k], col_word, &col_word_index) == 0)
									{
										printf(" col_word = _%s_\n", col_word);

										if (case_when_str[0] != 0 && (strcmp_Upper(col_word, "WHEN", NULL, malloced_head, the_debug) == 0 
																	  || strcmp_Upper(col_word, "ELSE", NULL, malloced_head, the_debug) == 0
																	  || strcmp_Upper(col_word, "END", NULL, malloced_head, the_debug) == 0))
										{
											if (strcmp_Upper(col_word, "ELSE", NULL, malloced_head, the_debug) == 0)
											{
												// START Add to null ptr case_when_tail
													if (addListNodePtr(&col_case_node->case_when_head, &col_case_node->case_when_tail, NULL, -1, ADDLISTNODE_TAIL
																	  ,NULL, malloced_head, the_debug) != 0)
													{
														if (the_debug == YES_DEBUG)
															printf("	ERROR in parseSelect() at line %d in %s\n", __LINE__, __FILE__);
														*select_node = NULL;
														return -1;
													}
												// END Add to null ptr case_when_tail
											}
											else if (strcmp_Upper(col_word, "END", NULL, malloced_head, the_debug) == 0)
											{
												// START Parse new column name
													getNextWord(cols_strs_arr[arr_k], col_word, &col_word_index);

													if (parseNewColumnName(&new_col_name, cols_strs_arr[arr_k], &col_word, &col_word_index, malloced_head, the_debug) != 0)
													{
														*select_node = NULL;
														return -1;
													}

													printf("new_col_name = _%s_\n", new_col_name);
												// END Parse new column name
											}

											// START Add to case_then_value_tail
												printf("   parsing then = _%s_\n", case_when_str);

												if ((strcontains(case_when_str, '+') == 1 || strcontains(case_when_str, '-') == 1 || strcontains(case_when_str, '*') == 1 || strcontains(case_when_str, '/') == 1 || strcontains(case_when_str, '^') == 1)
													&& (strcontains(case_when_str, 39) != 1 && indexOf(case_when_str, ' ') < strLength(case_when_str)-1))
												{
													// START Parse math for then value
														printf("Found math in then\n");

														char* math_word = (char*) myMalloc(sizeof(char) * 200, NULL, malloced_head, the_debug);
														math_word[0] = 0;

														int math_index = 0;

														char* first_math_word = NULL;

														getNextWord(case_when_str, math_word, &math_index);
														if (math_word[0] != '(')
														{
															first_math_word = upper(math_word, NULL, malloced_head, the_debug);

															getNextWord(case_when_str, math_word, &math_index);
														}

														struct math_node* new_math_node = NULL;

														if (parseMathNode(&new_math_node, NULL, &first_math_word, case_when_str, math_word, math_index, *select_node
																		 ,malloced_head, the_debug) != 0)
														{
															*select_node = NULL;
															return -1;
														}

														myFree((void**) &math_word, NULL, malloced_head, the_debug);


														if (addListNodePtr(&col_case_node->case_then_value_head, &col_case_node->case_then_value_tail, new_math_node, PTR_TYPE_MATH_NODE, ADDLISTNODE_TAIL
																		  ,NULL, malloced_head, the_debug) != 0)
														{
															if (the_debug == YES_DEBUG)
																printf("	ERROR in parseSelect() at line %d in %s\n", __LINE__, __FILE__);
															*select_node = NULL;
															return -1;
														}
													// END Parse math for then value
												}
												else
												{
													// START Parse single word for then value
														void* temp = myMalloc(sizeof(char) * (strLength(case_when_str)+1), NULL, malloced_head, the_debug);
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

															if (hmm_int > 0 && strcontains(case_when_str, '.'))
															{
																temp = myMalloc(sizeof(double), NULL, malloced_head, the_debug);
																double hmm_double = 0.0;
																sscanf(case_when_str, "%lf", &hmm_double);
																*((double*) temp) = hmm_double;

																new_ptr_type = PTR_TYPE_REAL;
															}
															else if (hmm_int > 0)
															{
																temp = myMalloc(sizeof(int), NULL, malloced_head, the_debug);
																*((int*) temp) = hmm_int;

																new_ptr_type = PTR_TYPE_INT;
															}
														}

														if (col_case_node->case_then_value_tail != NULL && ((col_case_node->case_then_value_tail->ptr_type != PTR_TYPE_MATH_NODE && col_case_node->case_then_value_tail->ptr_type != new_ptr_type)
																											|| (col_case_node->case_then_value_tail->ptr_type == PTR_TYPE_MATH_NODE && new_ptr_type == PTR_TYPE_CHAR)))
														{
															if (the_debug == YES_DEBUG)
																printf("	ERROR in parseSelect() at line %d in %s\n", __LINE__, __FILE__);
															errorTeardown(NULL, malloced_head, the_debug);
															*select_node = NULL;
															return -1;
														}

														if (addListNodePtr(&col_case_node->case_then_value_head, &col_case_node->case_then_value_tail, temp, new_ptr_type, ADDLISTNODE_TAIL
																		  ,NULL, malloced_head, the_debug) != 0)
														{
															if (the_debug == YES_DEBUG)
																printf("	ERROR in parseSelect() at line %d in %s\n", __LINE__, __FILE__);
															*select_node = NULL;
															return -1;
														}
													// END Parse single word for then value
												}


												case_when_str[0] = 0;

												if (strcmp_Upper(col_word, "WHEN", NULL, malloced_head, the_debug) == 0)
													strcat(case_when_str, "when ");
											// END Add to case_then_value_tail
										}
										else if (strcmp_Upper(col_word, "THEN", NULL, malloced_head, the_debug) == 0)
										{
											// START Add to case_when_tail
												printf("   parsing when clause = _%s_\n", case_when_str);

												struct where_clause_node* when_head = NULL;

												if (parseWhereClause(case_when_str, &when_head, *select_node, "when"
					    											,malloced_head, the_debug) != 0)
												{
													*select_node = NULL;
													return -1;
												}

											
												if (addListNodePtr(&col_case_node->case_when_head, &col_case_node->case_when_tail, when_head, PTR_TYPE_WHERE_CLAUSE_NODE, ADDLISTNODE_TAIL
																  ,NULL, malloced_head, the_debug) != 0)
												{
													if (the_debug == YES_DEBUG)
														printf("	ERROR in parseSelect() at line %d in %s\n", __LINE__, __FILE__);
													*select_node = NULL;
													return -1;
												}


												case_when_str[0] = 0;
											// END Add to case_when_tail
										}
										else
										{
											// START Concat to case_when_str
												strcat(case_when_str, col_word);
												if (cols_strs_arr[arr_k][col_word_index] == '.')
												{
													if (case_when_str[strLength(case_when_str)-1] == ' ')
														case_when_str[strLength(case_when_str)-1] = '.';
													else
														strcat(case_when_str, ".");
													col_word_index++;
												}
												else
													strcat(case_when_str, " ");
											// START Concat to case_when_str
										}
									}

									myFree((void**) &case_when_str, NULL, malloced_head, the_debug);
								}

								break;
							// END Parse case statement
						}
						else if (strcmp_Upper(col_word, "EXCEPT", NULL, malloced_head, the_debug) == 0)
						{
							// START Setup except
								if (except)
								{
									if (the_debug == YES_DEBUG)
										printf("	ERROR in parseSelect() at line %d in %s\n", __LINE__, __FILE__);
									errorTeardown(NULL, malloced_head, the_debug);
									*select_node = NULL;
									return -1;
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
													return -1;
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
													return -1;
												}

												col_word_index++;
											}
										}
										else if (except_cols_name_tail->ptr_value == NULL)
										{
											except_cols_name_tail->ptr_value = upper(col_word, NULL, malloced_head, the_debug);
										}
										else
										{
											if (the_debug == YES_DEBUG)
												printf("	ERROR in parseSelect() at line %d in %s\n", __LINE__, __FILE__);
											errorTeardown(NULL, malloced_head, the_debug);
											*select_node = NULL;
											return -1;
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
														return -1;
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
														return -1;
													}

													col_word_index++;
												}
											}
											else if (except_cols_name_tail->ptr_value == NULL)
											{
												except_cols_name_tail->ptr_value = upper(col_word, NULL, malloced_head, the_debug);
											}
											else
											{
												if (the_debug == YES_DEBUG)
													printf("	ERROR in parseSelect() at line %d in %s\n", __LINE__, __FILE__);
												errorTeardown(NULL, malloced_head, the_debug);
												*select_node = NULL;
												return -1;
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
									errorTeardown(NULL, malloced_head, the_debug);
									*select_node = NULL;
									return -1;
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
										return -1;
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
										return -1;
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

									myFree((void**) &col_name, NULL, malloced_head, the_debug);
								}
								else if (parseMathNode(&col_math_node, &new_col_name, &col_name, cols_strs_arr[arr_k], col_word, col_word_index, *select_node
													  ,malloced_head, the_debug) != 0)
								{
									*select_node = NULL;
									return -1;
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

									myFree((void**) &col_name, NULL, malloced_head, the_debug);
								}
								else
								{
									//printf("Beginning of func node\n");
									new_col_name = (char*) myMalloc(sizeof(char) * 256, NULL, malloced_head, the_debug);
									char* temp = upper(col_name, NULL, malloced_head, the_debug);
									strcpy(new_col_name, temp);
									myFree((void**) &temp, NULL, malloced_head, the_debug);
									strcat(new_col_name, " ( ");

									col_func_node = (struct func_node*) myMalloc(sizeof(struct func_node), NULL, malloced_head, the_debug);	

									if (strcmp_Upper(col_name, "COUNT", NULL, malloced_head, the_debug) == 0)
										col_func_node->which_func = FUNC_COUNT;
									else if (strcmp_Upper(col_name, "AVG", NULL, malloced_head, the_debug) == 0)
										col_func_node->which_func = FUNC_AVG;
									else if (strcmp_Upper(col_name, "FIRST", NULL, malloced_head, the_debug) == 0)
										col_func_node->which_func = FUNC_FIRST;
									else if (strcmp_Upper(col_name, "LAST", NULL, malloced_head, the_debug) == 0)
										col_func_node->which_func = FUNC_LAST;
									else if (strcmp_Upper(col_name, "MIN", NULL, malloced_head, the_debug) == 0)
										col_func_node->which_func = FUNC_MIN;
									else if (strcmp_Upper(col_name, "MAX", NULL, malloced_head, the_debug) == 0)
										col_func_node->which_func = FUNC_MAX;
									else if (strcmp_Upper(col_name, "MEDIAN", NULL, malloced_head, the_debug) == 0)
										col_func_node->which_func = FUNC_MEDIAN;
									else if (strcmp_Upper(col_name, "SUM", NULL, malloced_head, the_debug) == 0)
										col_func_node->which_func = FUNC_SUM;
									else if (strcmp_Upper(col_name, "RANK", NULL, malloced_head, the_debug) == 0)
										col_func_node->which_func = FUNC_RANK;
									else
									{
										if (the_debug == YES_DEBUG)
											printf("	ERROR in parseSelect() at line %d in %s\n", __LINE__, __FILE__);
										errorTeardown(NULL, malloced_head, the_debug);
										*select_node = NULL;
										return -1;
									}

									myFree((void**) &col_name, NULL, malloced_head, the_debug);
									col_name = NULL;

									struct ListNodePtr* args_head = NULL;
									struct ListNodePtr* args_tail = NULL;

									col_func_node->args_size = 1;

									addListNodePtr(&args_head, &args_tail, NULL, -1, ADDLISTNODE_TAIL, NULL, malloced_head, the_debug);

									col_func_node->distinct = false;

									while(getNextWord(cols_strs_arr[arr_k], col_word, &col_word_index) == 0 && col_word[0] != ')')
									{
										printf("word in func node loop = _%s_\n", col_word);

										char* temp = upper(col_word, NULL, malloced_head, the_debug);
										strcat(new_col_name, temp);
										myFree((void**) &temp, NULL, malloced_head, the_debug);
										strcat(new_col_name, " ");

										if (col_word[0] == '(')
										{
											if (the_debug == YES_DEBUG)
												printf("	ERROR in parseSelect() at line %d in %s\n", __LINE__, __FILE__);
											errorTeardown(NULL, malloced_head, the_debug);
											*select_node = NULL;
											return -1;
										}
										else if (strcmp_Upper(col_word, "DISTINCT", NULL, malloced_head, the_debug) == 0)
										{
											if (col_func_node->distinct)
											{
												if (the_debug == YES_DEBUG)
													printf("	ERROR in parseSelect() at line %d in %s\n", __LINE__, __FILE__);
												errorTeardown(NULL, malloced_head, the_debug);
												*select_node = NULL;
												return -1;
											}

											col_func_node->distinct = true;
										}
										else if (col_word[0] == ',')
										{
											col_func_node->args_size++;
											addListNodePtr(&args_head, &args_tail, NULL, -1, ADDLISTNODE_TAIL, NULL, malloced_head, the_debug);
										}
										else
										{
											if (col_word[0] == '*')
											{
												args_tail->ptr_value = upper(col_word, NULL, malloced_head, the_debug);
												args_tail->ptr_type = PTR_TYPE_CHAR;
											}
											else
											{
												char* alias = NULL;
												char* name = col_word;

												int index_of_dot = indexOf(name, '.');
												if (index_of_dot > -1)
												{
													alias = substring(col_word, 0, index_of_dot-1, NULL, malloced_head, the_debug);
													name = substring(col_word, index_of_dot+1, strLength(col_word)-1, NULL, malloced_head, the_debug);
												}
												//printf("alias.name = _%s.%s_\n", alias, name);

												if (getColInSelectNodeFromName((void*) args_tail, PTR_TYPE_LIST_NODE_PTR, 1, -1, *select_node, alias, name
																			  ,malloced_head, the_debug) != 0)
												{
													errorTeardown(NULL, malloced_head, the_debug);
													return -1;
												}

												if (alias != NULL)
												{
													myFree((void**) &alias, NULL, malloced_head, the_debug);
													myFree((void**) &name, NULL, malloced_head, the_debug);
												}

												// START Check column datatypes against functions
													if (col_func_node->which_func != FUNC_COUNT && col_func_node->which_func != FUNC_RANK
														&& ((struct table_cols_info*) ((struct col_in_select_node*) args_tail->ptr_value)->col_ptr)->data_type == DATA_STRING)
													{
														if (the_debug == YES_DEBUG)
															printf("	ERROR in parseSelect() at line %d in %s\n", __LINE__, __FILE__);
														errorTeardown(NULL, malloced_head, the_debug);
														*select_node = NULL;
														return -1;
													}
												// END Check column datatypes against functions
											}
										}
									}

									strcat(new_col_name, ") ");

									int args_size = col_func_node->args_size;
									col_func_node->args_arr = (void**) myMalloc(sizeof(void*) * args_size, NULL, malloced_head, the_debug);
									col_func_node->args_arr_type = (int*) myMalloc(sizeof(int) * args_size, NULL, malloced_head, the_debug);
									
									struct ListNodePtr* cur = args_head;
									for (int j=0; j<args_size; j++)
									{
										if (cur->ptr_value == NULL)
										{
											if (the_debug == YES_DEBUG)
												printf("	ERROR in parseSelect() at line %d in %s\n", __LINE__, __FILE__);
											errorTeardown(NULL, malloced_head, the_debug);
											*select_node = NULL;
											return -1;
										}

										col_func_node->args_arr[j] = cur->ptr_value;
										cur->ptr_value = NULL;

										col_func_node->args_arr_type[j] = cur->ptr_type;

										cur = cur->next;
									}

									int freed = freeAnyLinkedList((void**) &args_head, PTR_TYPE_LIST_NODE_PTR, NULL, malloced_head, the_debug);
									//printf("freed %d from args_head\n", freed);

									col_func_node->group_by_cols_head = NULL;
									col_func_node->group_by_cols_tail = NULL;


									// START Check if all functions except rank have more than 1 arg
										if (col_func_node->which_func != FUNC_RANK && col_func_node->args_size != 1)
										{
											if (the_debug == YES_DEBUG)
												printf("	ERROR in parseSelect() at line %d in %s\n", __LINE__, __FILE__);
											errorTeardown(NULL, malloced_head, the_debug);
											*select_node = NULL;
											return -1;
										}
									// END Check if all functions except rank have more than 1 arg


									if (getNextWord(cols_strs_arr[arr_k], col_word, &col_word_index) == 0)
									{
										if (col_word[0] == '(' || col_word[0] == ')')
										{
											if (the_debug == YES_DEBUG)
												printf("	ERROR in parseSelect() at line %d in %s\n", __LINE__, __FILE__);
											errorTeardown(NULL, malloced_head, the_debug);
											*select_node = NULL;
											return -1;
										}

										if (parseNewColumnName(&new_col_name, cols_strs_arr[arr_k], &col_word, &col_word_index, malloced_head, the_debug) != 0)
										{
											*select_node = NULL;
											return -1;
										}

										printf("new_col_name = _%s_\n", new_col_name);
									}
								}

								break;
							// END If not math node, then func node
						}
						else if (col_name == NULL && !except)
						{
							// START Declare column name by initializing col_name
								if (col_word[0] == 39)
								{
									col_name = myMalloc(sizeof(char) * (strLength(col_word)+1), NULL, malloced_head, the_debug);
									strcpy(col_name, col_word);
								}
								else
									col_name = upper(col_word, NULL, malloced_head, the_debug);
							// END Declare column name by initializing col_name
						}
						else if (!except)
						{
							// START Column is given a new name
								printf("Column is given a new name\n");
								if (new_col_name != NULL)
								{
									if (the_debug == YES_DEBUG)
										printf("	ERROR in parseSelect() at line %d in %s\n", __LINE__, __FILE__);
									errorTeardown(NULL, malloced_head, the_debug);
									*select_node = NULL;
									return -1;
								}

								if (parseNewColumnName(&new_col_name, cols_strs_arr[arr_k], &col_word, &col_word_index, malloced_head, the_debug) != 0)
								{
									*select_node = NULL;
									return -1;
								}
							// END Column is given a new name
						}
					}

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

								index++;
							}
						// END Assign to col math_node ptr
					}
					else if (col_case_node != NULL)
					{
						// START Assign to col math_node ptr
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

								index++;
							}
						// END Assign to col math_node ptr
					}
					else if (col_name != NULL && col_name[0] == '*')
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
										if (((char*) cur_except_name->ptr_value)[0] != '*' && strcmp_Upper(cur_except_name->ptr_value, ((struct table_cols_info*) (*select_node)->prev->columns_arr[j]->col_ptr)->col_name, NULL, malloced_head, the_debug) == 0)
										{
											if ((cur_except_alias->ptr_value == NULL && (*select_node)->join_head == NULL) 
												|| (cur_except_alias->ptr_value != NULL && (*select_node)->prev->select_node_alias != NULL && strcmp(cur_except_alias->ptr_value, (*select_node)->prev->select_node_alias) == 0)
												|| (cur_except_alias->ptr_value != NULL && strcmp_Upper(cur_except_alias->ptr_value, ((struct table_info*) (*select_node)->prev->columns_arr[0]->table_ptr)->name, NULL, malloced_head, the_debug) == 0))
											{
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
											if (((char*) cur_except_name->ptr_value)[0] != '*' && strcmp_Upper(cur_except_name->ptr_value, ((struct table_cols_info*) cur_join->select_joined->columns_arr[j]->col_ptr)->col_name, NULL, malloced_head, the_debug) == 0)
											{
												if ((cur_except_alias->ptr_value == NULL && (*select_node)->join_head == NULL) 
													|| (cur_except_alias->ptr_value != NULL && cur_join->select_joined->select_node_alias != NULL && strcmp(cur_except_alias->ptr_value, cur_join->select_joined->select_node_alias) == 0)
													|| (cur_except_alias->ptr_value != NULL && strcmp_Upper(cur_except_alias->ptr_value, ((struct table_info*) cur_join->select_joined->columns_arr[0]->table_ptr)->name, NULL, malloced_head, the_debug) == 0))
												{
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

												index++;
											}
										}
									}
								}

								cur_join = cur_join->next;
							}
						// END Looking for all columns in all joined nodes (*select_node)->join_head->select_joined
					}
					else if (col_name != NULL)
					{
						// START Find name of column in prev ptrs and/or joins
							if (i == 0)
							{
								int found_col = getColInSelectNodeFromName(NULL, PTR_TYPE_COL_IN_SELECT_NODE, -1, i, *select_node, alias, col_name, malloced_head, the_debug);

								int hmm_int = atoi(col_name);

								if (found_col == 0 || hmm_int > 0 || col_name[0] == 39)
									(*select_node)->columns_arr_size += 1;
								else
								{
									errorTeardown(NULL, malloced_head, the_debug);
									*select_node = NULL;
									return -1;
								}
							}
							else if (!except)//if (i == 1)
							{
								int found_col = getColInSelectNodeFromName((void*) (*select_node)->columns_arr[index], PTR_TYPE_COL_IN_SELECT_NODE, -1, i, *select_node, alias, col_name, malloced_head, the_debug);

								int hmm_int = atoi(col_name);

								if (found_col != 0 && hmm_int == 0 && col_name[0] != 39)
								{
									errorTeardown(NULL, malloced_head, the_debug);
									*select_node = NULL;
									return -1;
								}
								else if (found_col != 0 && (hmm_int > 0 || col_name[0] == 39))
								{
									void* temp = NULL;
									int ptr_type = -1;

									if (hmm_int > 0 && alias != NULL)
									{
										temp = myMalloc(sizeof(double), NULL, malloced_head, the_debug);
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
									else if (hmm_int > 0)
									{
										temp = myMalloc(sizeof(int), NULL, malloced_head, the_debug);
										*((int*) temp) = hmm_int;

										ptr_type = PTR_TYPE_INT;
									}
									else if (col_name[0] == 39)
									{
										temp = myMalloc(sizeof(char) * (strLength(col_name)+1), NULL, malloced_head, the_debug);
										strcpy(temp, col_name);

										trimStr(temp);

										if (isDate((char*) temp))
											ptr_type = PTR_TYPE_DATE;
										else
											ptr_type = PTR_TYPE_CHAR;
									}

									(*select_node)->columns_arr[index]->case_node = myMalloc(sizeof(struct case_node), NULL, malloced_head, the_debug);
									(*select_node)->columns_arr[index]->case_node->case_when_head = NULL;
									(*select_node)->columns_arr[index]->case_node->case_then_value_head = NULL;

									addListNodePtr(&(*select_node)->columns_arr[index]->case_node->case_when_head, &(*select_node)->columns_arr[index]->case_node->case_when_tail, NULL, -1, ADDLISTNODE_TAIL
								 				  ,NULL, malloced_head, the_debug);
									addListNodePtr(&(*select_node)->columns_arr[index]->case_node->case_then_value_head, &(*select_node)->columns_arr[index]->case_node->case_then_value_tail, temp, ptr_type, ADDLISTNODE_TAIL
												  ,NULL, malloced_head, the_debug);
								}

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
						errorTeardown(NULL, malloced_head, the_debug);
						*select_node = NULL;
						return -1;
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
	

	printf("here the word is _%s_\n", word);

	// START Parse where clause at the end
		compared = strcmp_Upper(word, "WHERE", NULL, malloced_head, the_debug);
		if (compared == -2)
		{
			if (the_debug == YES_DEBUG)
				printf("	ERROR in parseSelect() at line %d in %s\n", __LINE__, __FILE__);
			*select_node = NULL;
			return -1;
		}
		else if (strcmp_Upper(word, "GROUP", NULL, malloced_head, the_debug) == 0)
		{
			printf("Found group by clause\n");

			getNextWord(input, word, &index);

			if (strcmp_Upper(word, "BY", NULL, malloced_head, the_debug) != 0)
			{
				if (the_debug == YES_DEBUG)
					printf("	ERROR in parseSelect() at line %d in %s\n", __LINE__, __FILE__);
				errorTeardown(NULL, malloced_head, the_debug);
				*select_node = NULL;
				return -1;
			}

			char* alias = NULL;
			char* name = NULL;

			while (getNextWord(input, word, &index) == 0 && strcmp(word, ";") != 0)
			{
				printf("String in group by loop: _%s_\n", word);

				if (word[0] == ',')
				{
					printf("alias.name = _%s.%s_\n", alias, name);
					for (int i=0; i<(*select_node)->columns_arr_size; i++)
					{
						if ((*select_node)->columns_arr[i]->func_node != NULL && (*select_node)->columns_arr[i]->func_node->which_func != FUNC_RANK)
						{
							addListNodePtr(&(*select_node)->columns_arr[i]->func_node->group_by_cols_head, &(*select_node)->columns_arr[i]->func_node->group_by_cols_tail
										  ,NULL, -1, ADDLISTNODE_TAIL
						  				  ,NULL, malloced_head, the_debug);

							// Is aggregate function, add to group_by_cols_head
							if (getColInSelectNodeFromName((void*) (*select_node)->columns_arr[i]->func_node->group_by_cols_tail, PTR_TYPE_LIST_NODE_PTR, -1, 1, *select_node, alias, name, malloced_head, the_debug) != 0)
							{
								errorTeardown(NULL, malloced_head, the_debug);
								*select_node = NULL;
								return -1;
							}
						}
					}

					if (alias != NULL)
						myFree((void**) &alias, NULL, malloced_head, the_debug);
					myFree((void**) &name, NULL, malloced_head, the_debug);
				}
				else if (input[index] == '.')
				{
					alias = name;

					index++;

					getNextWord(input, word, &index);

					name = upper(word, NULL, malloced_head, the_debug);
				}
				else
				{
					name = upper(word, NULL, malloced_head, the_debug);
				}
			}

			printf("alias.name = _%s.%s_\n", alias, name);
			for (int i=0; i<(*select_node)->columns_arr_size; i++)
			{
				if ((*select_node)->columns_arr[i]->func_node != NULL && (*select_node)->columns_arr[i]->func_node->which_func != FUNC_RANK)
				{
					addListNodePtr(&(*select_node)->columns_arr[i]->func_node->group_by_cols_head, &(*select_node)->columns_arr[i]->func_node->group_by_cols_tail
								  ,NULL, -1, ADDLISTNODE_TAIL
				  				  ,NULL, malloced_head, the_debug);

					// Is aggregate function, add to group_by_cols_head
					if (getColInSelectNodeFromName((void*) (*select_node)->columns_arr[i]->func_node->group_by_cols_tail, PTR_TYPE_LIST_NODE_PTR, -1, 1, *select_node, alias, name, malloced_head, the_debug) != 0)
					{
						errorTeardown(NULL, malloced_head, the_debug);
						*select_node = NULL;
						return -1;
					}
				}
			}

			if (alias != NULL)
				myFree((void**) &alias, NULL, malloced_head, the_debug);
			myFree((void**) &name, NULL, malloced_head, the_debug);
		}
		else if (compared != 0 && strcmp(word, ";") != 0 && word[0] != 0)
		{
			if (the_debug == YES_DEBUG)
				printf("	ERROR in parseSelect() at line %d in %s\n", __LINE__, __FILE__);
			errorTeardown(NULL, malloced_head, the_debug);
			*select_node = NULL;
			return -1;
		}
		else if (compared == 0)
		{
			printf("TODO: call parse where clause\n");
			/*// START Parse where clause
			//printf("word = _%s_\n", word);
			//printf("%c%c%c\n", input[index-1], input[index], input[index+1]);

			index -= 5;

			char* where_clause = substring(input, index, strLength(input)-1, NULL, malloced_head, the_debug);
			if (where_clause == NULL)
			{
				if (the_debug == YES_DEBUG)
					printf("	ERROR in parseSelect() at line %d in %s\n", __LINE__, __FILE__);
				*select_node = NULL;
				return -1;
			}
			//printf("where_clause = _%s_\n", where_clause);

			if ((*select_node)->or_head != NULL)
			{
				int error_code;
				(*select_node)->or_head = parseWhereClause(where_clause, select_node, &error_code, "where", malloced_head, the_debug);

				if (error_code != 0)
				{
					if (the_debug == YES_DEBUG)
						printf("	ERROR in parseSelect() at line %d in %s\n", __LINE__, __FILE__);
					errorTeardown(NULL, malloced_head, the_debug);
					*select_node = NULL;
					return -1;
				}
			}

			myFree((void**) &where_clause, NULL, malloced_head, the_debug);
			// END Parse where clause*/
		}
	// END Parse where clause at the end


	myFree((void**) &word, NULL, malloced_head, the_debug);


	// START Free with_sub_select_head
		while (with_sub_select_head != NULL && !in_with)
		{
			struct ListNodePtr* temp = with_sub_select_head;

			with_sub_select_head = with_sub_select_head->next;

			printf("temp alias = _%s_\n", ((struct select_node*) temp->ptr_value)->select_node_alias);
			printf("temp ptr_type = %d\n", temp->ptr_type);

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


	// START Check if all non func columns are in group clause if there are agg funcs
		struct func_node* a_func_node = NULL;

		for (int i=0; i<(*select_node)->columns_arr_size; i++)
		{
			if ((*select_node)->columns_arr[i]->func_node != NULL && (*select_node)->columns_arr[i]->func_node->which_func != FUNC_RANK)
			{
				a_func_node = (*select_node)->columns_arr[i]->func_node;
				break;
			}
		}

		if (a_func_node != NULL)
		{
			printf("Checking group by clause at end\n");
			for (int i=0; i<(*select_node)->columns_arr_size; i++)
			{
				if ((*select_node)->columns_arr[i]->func_node == NULL && (*select_node)->columns_arr[i]->math_node != NULL)
				{
					printf("Checking math_node at columns_arr index = %d\n", i);

					void* ptr = NULL;
					int ptr_type = -1;

					struct math_node* cur_mirror = myMalloc(sizeof(struct math_node), NULL, malloced_head, the_debug);
					initEmptyTreeNode((void**) &cur_mirror, NULL, PTR_TYPE_MATH_NODE);

					while (traverseTreeNode((void**) &(*select_node)->columns_arr[i]->math_node, PTR_TYPE_MATH_NODE, &ptr, &ptr_type, (void**) &cur_mirror
										   ,NULL, malloced_head, the_debug) == 0)
					{
						if (ptr_type == PTR_TYPE_COL_IN_SELECT_NODE)
						{
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
								errorTeardown(NULL, malloced_head, the_debug);
								*select_node = NULL;
								return -1;
							}
						}
					}
				}
				else if ((*select_node)->columns_arr[i]->func_node == NULL)
				{
					printf("Checking regular col at columns_arr index = %d\n", i);

					struct ListNodePtr* cur = a_func_node->group_by_cols_head;
					while (cur != NULL)
					{
						/*if (((struct col_in_select_node*) cur->ptr_value)->col_ptr_type == PTR_TYPE_TABLE_COLS_INFO)
							printf("cur = _%s_\n", ((struct table_cols_info*) ((struct col_in_select_node*) cur->ptr_value)->col_ptr)->col_name);

						if ((*select_node)->columns_arr[i]->col_ptr_type == PTR_TYPE_TABLE_COLS_INFO)
							printf("columns_arr[i] = _%s_\n", ((struct table_cols_info*) (*select_node)->columns_arr[i]->col_ptr)->col_name);
						else if ((*select_node)->columns_arr[i]->col_ptr_type == PTR_TYPE_COL_IN_SELECT_NODE)
							printf("columns_arr[i] = _%s_\n", ((struct table_cols_info*) ((struct col_in_select_node*) (*select_node)->columns_arr[i]->col_ptr)->col_ptr)->col_name);
						else if ((*select_node)->columns_arr[i]->col_ptr_type == -1)
							printf("columns_arr[i] = -1\n");*/

						if ((*select_node)->columns_arr[i]->case_node != NULL)
						{
							
						}
						else if (cur->ptr_value == (*select_node)->columns_arr[i]->col_ptr)
							break;

						cur = cur->next;
					}

					if (cur == NULL)
					{
						if (the_debug == YES_DEBUG)
							printf("	ERROR in parseSelect() at line %d in %s\n", __LINE__, __FILE__);
						errorTeardown(NULL, malloced_head, the_debug);
						*select_node = NULL;
						return -1;
					}
				}
			}
		}
	// END Check if all non func columns are in group clause if there are agg funcs


	// START Traverse to head of list which is the select_node which must be executed first
		while ((*select_node)->prev != NULL)
			*select_node = (*select_node)->prev;
	// END Traverse to head of list which is the select_node which must be executed first


	return 0;
}

/*
int selectAndPrint(char* input
				  ,struct malloced_node** malloced_head, int the_debug)
{
	struct select_node* select_node = (struct select_node*) myMalloc(sizeof(struct select_node), NULL, malloced_head, the_debug);
	select_node->columns_arr_size = 0;
	select_node->columns_table_ptrs_arr = NULL;
	select_node->columns_col_ptrs_arr = NULL;

	select_node->or_head = NULL;

	select_node->join_head = NULL;

	select_node->prev = NULL;
	select_node->next = NULL;

	//printf("The whole thing: _%s_\n", input);

	if (parseSelect(input, &select_node
				   ,malloced_head, the_debug) != 0)
	{
		printf("There was an problem parsing the command, please try again\n");
		return -1;
	}

	
	int_8 num_rows_in_result = 0;
	struct colDataNode*** result = select(select_node->table, select_node->columns_col_numbers_arr, select_node->columns_arr_size
										  ,&num_rows_in_result, select_node->or_head, malloced_head, the_debug);
	if (result == NULL)
	{
		printf("There was an problem retrieving the data, please try again\n");
		return -1;
	}


	// START Write column names
	for (int j=0; j<select_node->columns_arr_size; j++)
	{
		struct table_cols_info* cur_col = select_node->table->table_cols_head;
		while (cur_col != NULL)
		{
			if (cur_col->col_number == select_node->columns_col_numbers_arr[j])
				break;
			cur_col = cur_col->next;
		}

		printf("%s", cur_col->col_name);

		if (j < select_node->columns_arr_size-1)
			printf(",");
		else
			printf("\n");
	}
	// END Write column names


	// START Print column data
	for (int i=0; i<num_rows_in_result; i++)
	{
		for (int j=0; j<select_node->columns_arr_size; j++)
		{
			printf("%s", result[j][i]->row_data);
			if (j < select_node->columns_arr_size-1)
				printf(",");
		}
		printf("\n");
	}
	// END Print column data


	// START Free stuff
	while (select_node->or_head != NULL)
	{
		while (select_node->or_head->and_head != NULL)
		{
			struct and_clause_node* temp = select_node->or_head->and_head;
			select_node->or_head->and_head =select_node-> or_head->and_head->next;
			myFree((void**) &temp->data_string, NULL, malloced_head, the_debug);
			myFree((void**) &temp, NULL, malloced_head, the_debug);
		}
		struct or_clause_node* temp = select_node->or_head;
		select_node->or_head = select_node->or_head->next;
		myFree((void**) &temp, NULL, malloced_head, the_debug);
	}

	for (int j=select_node->columns_arr_size-1; j>-1; j--)
	{
		for (int i=num_rows_in_result-1; i>-1; i--)
		{
			myFree((void**) &(result[j][i]->row_data), NULL, malloced_head, the_debug);
			myFree((void**) &result[j][i], NULL, malloced_head, the_debug);
		}
		myFree((void**) &result[j], NULL, malloced_head, the_debug);
	}
	myFree((void**) &result, NULL, malloced_head, the_debug);

	myFree((void**) &select_node->columns_table_ptrs_arr, NULL, malloced_head, the_debug);
	myFree((void**) &select_node->columns_col_ptrs_arr, NULL, malloced_head, the_debug);
	// END Free stuff

	return 0;
}

int insertAndPrint(char* input
				  ,struct malloced_node** malloced_head, int the_debug)
{
	struct table_info* table = NULL;
	struct change_node_v2* change_head = NULL;

	if (parseInsert(input, &change_head, &table
				   ,malloced_head, the_debug) != 0)
	{
		printf("There was an problem parsing the command, please try again\n");
		return -1;
	}

	int num_inserted = insertRows(table, change_head, malloced_head, the_debug);

	if (num_inserted < 0)
	{
		printf("There was an problem inserting the data, please try again\n");
		return -1;
	}

	while (change_head != NULL)
	{
		struct change_node_v2* temp = change_head;
		change_head = change_head->next;
		myFree((void**) &temp->data, NULL, malloced_head, the_debug);
		myFree((void**) &temp, NULL, malloced_head, the_debug);
	}

	return num_inserted;
}

int updateAndPrint(char* input
				  ,struct malloced_node** malloced_head, int the_debug)
{
	struct table_info* table = NULL;
	struct change_node_v2* change_head = NULL;
	struct or_clause_node* or_head = NULL;
	
	if (parseUpdate(input, &table, &change_head, &or_head, malloced_head, the_debug) != 0)
	{
		printf("There was an problem parsing the command, please try again\n");
		return -1;
	}

	int num_updated = updateRows(table, change_head, or_head, malloced_head, the_debug);

	if (num_updated < 0)
	{
		printf("There was an problem updating the data, please try again\n");
		return -1;
	}
	
	while (change_head != NULL)
	{
		struct change_node_v2* temp = change_head;
		change_head = change_head->next;
		myFree((void**) &temp->data, NULL, malloced_head, the_debug);
		myFree((void**) &temp, NULL, malloced_head, the_debug);
	}

	while (or_head != NULL)
	{
		while (or_head->and_head != NULL)
		{
			struct and_clause_node* temp = or_head->and_head;
			or_head->and_head = or_head->and_head->next;
			myFree((void**) &temp->data_string, NULL, malloced_head, the_debug);
			myFree((void**) &temp, NULL, malloced_head, the_debug);
		}
		struct or_clause_node* temp = or_head;
		or_head = or_head->next;
		myFree((void**) &temp, NULL, malloced_head, the_debug);
	}

	return num_updated;
}

int deleteAndPrint(char* input
				  ,struct malloced_node** malloced_head, int the_debug)
{
	struct table_info* table = NULL;
	struct or_clause_node* or_head = NULL;

	if (parseDelete(input, &or_head, &table
				   ,malloced_head, the_debug) != 0)
	{
		printf("There was an problem parsing the command, please try again\n");
		return -1;
	}


	int num_deleted = deleteRows(table, or_head, malloced_head, the_debug);

	if (num_deleted < 0)
	{
		printf("There was an problem deleting the data, please try again\n");
		return -1;
	}


	while (or_head != NULL)
	{
		while (or_head->and_head != NULL)
		{
			struct and_clause_node* temp = or_head->and_head;
			or_head->and_head = or_head->and_head->next;
			myFree((void**) &temp->data_string, NULL, malloced_head, the_debug);
			myFree((void**) &temp, NULL, malloced_head, the_debug);
		}
		struct or_clause_node* temp = or_head;
		or_head = or_head->next;
		myFree((void**) &temp, NULL, malloced_head, the_debug);
	}

	return num_deleted;
}



int parseInput(char* input
			  ,struct malloced_node** malloced_head, int the_debug)
{
	char* cmd = substring(input, 0, 5, NULL, malloced_head, the_debug);
	//printf("cmd = _%s_\n", cmd);

	char* upper_cmd = upper(cmd, NULL, malloced_head, the_debug);

	if (cmd == NULL || upper_cmd == NULL)
	{
		if (the_debug == YES_DEBUG)
			printf("	ERROR in parseInput() at line %d in %s\n", __LINE__, __FILE__);
		return -1;
	}
	
	if (strcmp(upper_cmd, "_GET_T") == 0)
	{
		myFree((void**) &cmd, NULL, malloced_head, the_debug);
		myFree((void**) &upper_cmd, NULL, malloced_head, the_debug);

		struct table_info* tables_head = getTablesHead();
		while (tables_head != NULL)
		{
			printf("%s:", tables_head->name);

			struct table_cols_info* cur_col = tables_head->table_cols_head;
			while (cur_col != NULL)
			{
				printf("%s", cur_col->col_name);

				if (cur_col->next != NULL)
					printf(",");

				cur_col = cur_col->next;
			}

			if (tables_head->next != NULL)
				printf(";");

			tables_head = tables_head->next;
		}
	}
	else if (strcmp(upper_cmd, "CREATE") == 0)
	{
		myFree((void**) &cmd, NULL, malloced_head, the_debug);
		myFree((void**) &upper_cmd, NULL, malloced_head, the_debug);
		printf("no action yet\n");
	}
	else if (strcmp(upper_cmd, "SELECT") == 0)
	{
		myFree((void**) &cmd, NULL, malloced_head, the_debug);
		myFree((void**) &upper_cmd, NULL, malloced_head, the_debug);
		selectAndPrint(input, malloced_head, the_debug);
	}
	else if (strcmp(upper_cmd, "INSERT") == 0)
	{
		myFree((void**) &cmd, NULL, malloced_head, the_debug);
		myFree((void**) &upper_cmd, NULL, malloced_head, the_debug);
		printf("Inserted %d rows\n", insertAndPrint(input, malloced_head, the_debug));
	}
	else if (strcmp(upper_cmd, "UPDATE") == 0)
	{
		myFree((void**) &cmd, NULL, malloced_head, the_debug);
		myFree((void**) &upper_cmd, NULL, malloced_head, the_debug);
		printf("Updated %d rows\n", updateAndPrint(input, malloced_head, the_debug));
	}
	else if (strcmp(upper_cmd, "DELETE") == 0)
	{
		myFree((void**) &cmd, NULL, malloced_head, the_debug);
		myFree((void**) &upper_cmd, NULL, malloced_head, the_debug);
		printf("Deleted %d rows\n", deleteAndPrint(input, malloced_head, the_debug));
	}
	else
	{
		myFree((void**) &cmd, NULL, malloced_head, the_debug);
		myFree((void**) &upper_cmd, NULL, malloced_head, the_debug);
		printf("There was an problem parsing the command, please try again\n");
	}


	return 0;
}*/


int main(int argc, char *argv[])
{
   	
   	if (argc == 2 && strcmp(argv[1], "_test") == 0)
   	{
		
		if (test_Driver_main() != 0)
			printf("\nTests FAILED\n\n");
		else
			printf("\nTests passed let's goooo\n\n");

		if (test_Driver_teardown(YES_DEBUG) != 0)
			return -1;
   	}
   	/*else
   	{
	    struct malloced_node* malloced_head = NULL;
		
	    int debug = NO_DEBUG;
	    
	    int initd = initDB(&malloced_head, debug);
	    if (initd != 0)
	    {
	    	printf("There was an problem with database initialization, please try again\n");
	    	return -1;
	    }

		if (argc == 5 && strcmp(argv[1], "_create_csv") == 0)
		{
			//printf("_%s_ _%s_ _%s_\n", argv[2], argv[3], argv[4]);
			if (access(argv[2], F_OK) == 0)
			{
				//printf("Valid file\n");
				char* table_name = (char*) myMalloc(sizeof(char) * 32, NULL, &malloced_head, debug);
				if (table_name == NULL)
				{
					printf("Table mallocing had a problem\n");
					return -1;
				}
				strcpy(table_name, argv[3]);
				
				printf("Created table and inserted %d rows\n", createTableFromCSV(argv[2], table_name, atoi(argv[4]), &malloced_head, debug));
			}
		}
		else if (argc == 2)
			parseInput(argv[1], &malloced_head, debug);

		freeMemOfDB(debug);
	}*/

	return 0;
}