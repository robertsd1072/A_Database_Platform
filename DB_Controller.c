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
#include "DB_Driver.h"
#include "DB_HelperFunctions.h"
#include "DB_Tests.h"

int strLength(char* str)
{
	int index = 0;
	while (str[index] != 0)
		index++;
	return index;
}

int strcontains(char* str, char the_char)
{
	int index = 0;
	while (str[index] != 0)
	{
	    if (str[index] == the_char)
            return 1;
	    index++;
	}
	return 0;
}

int indexOf(char* str, char the_char)
{
	int index = 0;
	while (str[index] != 0)
	{
		if (str[index] == the_char)
			return index;
		index++;
	}
	if (the_char == 0)
		return index;
	return -1;
}

char* substring(struct malloced_node** malloced_head, char* str, int start, int end, int persists)
{
	char* new_str = myMalloc(malloced_head, sizeof(char) * ((end-start)+1), persists);
	int j=0;
	for (int i=start; i<end+1; i++)
	{
		new_str[j] = str[i];
		j++;
	}
	new_str[j] = 0;

	return new_str;
}

char** strSplit(struct malloced_node** malloced_head, char* str, char the_char, int* size_result, int persists)
{
	char** result = 0;
    size_t count = 0;
    char* tmp = str;
    char* last_comma = 0;
    char delim[2];
    delim[0] = the_char;
    delim[1] = 0;

    /* Count how many elements will be extracted. */
    while (*tmp)
    {
        if (the_char == *tmp)
        {
            count++;
            last_comma = tmp;
        }
        tmp++;
    }

	*size_result = count+1;

    /* Add space for trailing token. */
    count += last_comma < (str + strlen(str) - 1);

    /* Add space for terminating null string so caller
       knows where the list of returned strings ends. */
    count++;

    result = myMalloc(malloced_head, sizeof(char*) * count, persists);

    if (result)
    {
        size_t idx  = 0;
        char* token = strtok(str, delim);

        while (token)
        {
            assert(idx < count);
            *(result + idx++) = strdup(token);
            token = strtok(0, delim);
        }
        assert(idx == count - 1);
        *(result + idx) = 0;
    }

    return result;
}

int callCreateTable(char* table_name, struct table_cols_info* table_cols, int the_debug)
{
	int created_table = createTable(table_name, table_cols, the_debug);
	if (created_table == -1)
		printf("createTable() had a problem with file i/o\n");
	else if (created_table == -2)
		printf("createTable() had a problem with malloc\n");
	else
		printf("createTable() was successful\n");
	
	return created_table;
}

int createTableFromCSV(char* input, char* table_name, int_8 num_rows, int the_debug)
{
	struct malloced_node* malloced_head = NULL;
    struct file_opened_node* file_opened_head = NULL;
	
	/**/
	// START Open csv file and read in first row of column names and datatypes
    FILE* file = myFileOpenSimple(&file_opened_head, input,  "r");
	if (file == NULL)
	{
		if (the_debug == YES_DEBUG)
			printf("	ERROR in createTableFromCSV() at line %d in %s\n", __LINE__, __FILE__);
        myFreeAllError(&malloced_head, the_debug);
        myFileCloseAll(&file_opened_head, the_debug);
		return -1;
	}

	char* first_row = (char*) myMalloc(&malloced_head, sizeof(char) * 1000, 0);
	if (first_row == NULL)
	{
		if (the_debug == YES_DEBUG)
			printf("	ERROR in createTableFromCSV() at line %d in %s\n", __LINE__, __FILE__);
        myFreeAllError(&malloced_head, the_debug);
        myFileCloseAll(&file_opened_head, the_debug);
		return -2;
	}
	fscanf(file, "%s\n", first_row);
	myFileClose(&file_opened_head, file);
    // END Open csv file and read in first row of column names and datatypes
	
	// START Allocate space for beginning of table_cols
    struct table_cols_info* table_cols = (struct table_cols_info*) myMalloc(&malloced_head, sizeof(struct table_cols_info), 1);
	if (table_cols == NULL)
	{
		if (the_debug == YES_DEBUG)
			printf("	ERROR in createTableFromCSV() at line %d in %s\n", __LINE__, __FILE__);
        myFreeAllError(&malloced_head, the_debug);
        myFileCloseAll(&file_opened_head, the_debug);
		return -2;
	}
	struct table_cols_info* cur_col = table_cols;
    // END Allocate space for beginning of table_cols
	
	// START Split first_row by each comma and extract column name and datatype
    int num_cols = 0;
	char** col_names = strSplit(&malloced_head, first_row, ',', &num_cols, 0);
	if (myFree(&malloced_head, (void**) &first_row, the_debug) != 0)
	{
		if (the_debug == YES_DEBUG)
			printf("	ERROR in createTableFromCSV() at line %d in %s\n", __LINE__, __FILE__);
		myFreeAllError(&malloced_head, the_debug);
		myFileCloseAll(&file_opened_head, the_debug);
		return -2;
	}

	for (int i=0; i<num_cols; i++)
	{
		char* temp1 = substring(&malloced_head, col_names[i], 0, indexOf(col_names[i], ':')-1, 1);
		char* temp2 = substring(&malloced_head, col_names[i], indexOf(col_names[i], ':')+1, indexOf(col_names[i], 0), 0);
		
		if (indexOf(temp1, 0) > 32)
		{
			if (the_debug == YES_DEBUG)
				printf("	ERROR in createTableFromCSV() at line %d in %s\n", __LINE__, __FILE__);
            myFreeAllError(&malloced_head, the_debug);
            myFileCloseAll(&file_opened_head, the_debug);
			return -11;
		}
		
		cur_col->col_name = temp1;
		cur_col->col_number = i;

		//printf("cur_col->col_name = %s\n", cur_col->col_name);
		//printf("cur_col->col_number = %d\n", cur_col->col_number);
		
		char* datatype = substring(&malloced_head, temp2, 0, 3, 0);
		if (strcmp(datatype, "inte") == 0)
		{
			cur_col->data_type = 1;
			cur_col->max_length = 8;
		}
		else if (strcmp(datatype, "real") == 0)
		{
			cur_col->data_type = 2;
			cur_col->max_length = 8;
		}
		else if (strcmp(datatype, "char") == 0)
		{
			cur_col->data_type = 3;
			sscanf(temp2, "char(%d)", &cur_col->max_length);
			cur_col->max_length++;
		}
		else if (strcmp(datatype, "date") == 0)
		{
			cur_col->data_type = 4;
			cur_col->max_length = 8;
		}
		else
		{
			if (the_debug == YES_DEBUG)
				printf("	ERROR in createTableFromCSV() at line %d in %s\n", __LINE__, __FILE__);
            myFreeAllError(&malloced_head, the_debug);
            myFileCloseAll(&file_opened_head, the_debug);
			return -12;
		}

		if (myFree(&malloced_head, (void**) &temp2, the_debug) != 0 || myFree(&malloced_head, (void**) &datatype, the_debug) != 0 || myFree(&malloced_head, (void**) &col_names[i], the_debug) != 0)
		{
			if (the_debug == YES_DEBUG)
				printf("	ERROR in createTableFromCSV() at line %d in %s\n", __LINE__, __FILE__);
			myFreeAllError(&malloced_head, the_debug);
			myFileCloseAll(&file_opened_head, the_debug);
			return -2;
		}

		if (i < num_cols-1)
		{
			cur_col->next = (struct table_cols_info*) myMalloc(&malloced_head, sizeof(struct table_cols_info), 1);
			if (cur_col->next == NULL)
			{
				if (the_debug == YES_DEBUG)
					printf("	ERROR in createTableFromCSV() at line %d in %s\n", __LINE__, __FILE__);
				myFreeAllError(&malloced_head, the_debug);
                myFileCloseAll(&file_opened_head, the_debug);
				return -2;
			}
			cur_col = cur_col->next;
		}
		else
			cur_col->next = NULL;
	}
	if (myFree(&malloced_head, (void**) &col_names, the_debug) != 0)
	{
		if (the_debug == YES_DEBUG)
			printf("	ERROR in createTableFromCSV() at line %d in %s\n", __LINE__, __FILE__);
		myFreeAllError(&malloced_head, the_debug);
		myFileCloseAll(&file_opened_head, the_debug);
		return -2;
	}
    // END Split first_row by each comma and extract column name and datatype

	
	
	if (callCreateTable(table_name, table_cols, the_debug) != 0)
    {
        myFreeAllError(&malloced_head, the_debug);
        myFileCloseAll(&file_opened_head, the_debug);
        return -13;
	}

	
	/**/
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
        myFreeAllError(&malloced_head, the_debug);
        myFileCloseAll(&file_opened_head, the_debug);
		return -14;
	}
    // END Find table in tables_head

	// START Reopen file
	//FILE* 
	file = myFileOpenSimple(&file_opened_head, input,  "r");
	//char* 
	first_row = (char*) myMalloc(&malloced_head, sizeof(char) * 1000, 0);
	if (first_row == NULL)
	{
		if (the_debug == YES_DEBUG)
			printf("	ERROR in createTableFromCSV() at line %d in %s\n", __LINE__, __FILE__);
        myFreeAllError(&malloced_head, the_debug);
        myFileCloseAll(&file_opened_head, the_debug);
		return -2;
	}
	fscanf(file, "%s\n", first_row);
	myFree(&malloced_head, (void**) &first_row, the_debug);
	// END Reopen file
    
	// START Allocate arrays for files opened
	FILE** col_data_info_file_arr = (FILE**) myMalloc(&malloced_head, sizeof(FILE*) * table->num_cols, 0);
	FILE** col_data_file_arr = (FILE**) myMalloc(&malloced_head, sizeof(FILE*) * table->num_cols, 0);
    if (col_data_info_file_arr == NULL || col_data_file_arr == NULL)
    {
        if (the_debug == YES_DEBUG)
			printf("	ERROR in createTableFromCSV() at line %d in %s\n", __LINE__, __FILE__);
        myFreeAllError(&malloced_head, the_debug);
        myFileCloseAll(&file_opened_head, the_debug);
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
			printf("In progress... inserted %d rows so far\n", i);
		//printf("i = %d\n", i);
        struct table_cols_info* cur_col = table->table_cols_head;
		char* a_line = (char*) myMalloc(&malloced_head, sizeof(char) * 1000, 0);
		fscanf(file, "%[^\n]\n", a_line);
		//printf("a_line = %s\n", a_line);

		for (int j=0; j<table->num_cols; j++)
		{
			char* format = (char*) myMalloc(&malloced_head, sizeof(char) * 200, 0);
			if (format == NULL)
			{
				if (the_debug == YES_DEBUG)
					printf("	ERROR in createTableFromCSV() at line %d in %s\n", __LINE__, __FILE__);
                myFreeAllError(&malloced_head, the_debug);
                myFileCloseAll(&file_opened_head, the_debug);
				return -2;
			}
			strcpy(format, "");
			for (int k=0; k<j; k++)
           		strcat(format, "%*[^,],");
			strcat(format, "%[^,],%*[^\n]\n");

			//printf("format = _%s_\n", format);

			char* data = (char*) myMalloc(&malloced_head, sizeof(char) * 300, 0);
            if (data == NULL)
			{
				if (the_debug == YES_DEBUG)
					printf("	ERROR in createTableFromCSV() at line %d in %s\n", __LINE__, __FILE__);
                myFreeAllError(&malloced_head, the_debug);
                myFileCloseAll(&file_opened_head, the_debug);
				return -2;
			}

			sscanf(a_line, format, data);

			if (strcmp(data, "") == 0)
			{
				if (the_debug == YES_DEBUG)
					printf("	ERROR in createTableFromCSV() at line %d in %s\n", __LINE__, __FILE__);
				myFreeAllError(&malloced_head, the_debug);
                myFileCloseAll(&file_opened_head, the_debug);
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
                            ,the_debug) != 0)
            {
                myFreeAllError(&malloced_head, the_debug);
                myFileCloseAll(&file_opened_head, the_debug);
				return -16;
            }

            //printf("cur_col->num_rows = %d\n", cur_col->num_rows);

			cur_col = cur_col->next;

            myFree(&malloced_head, (void**) &data, the_debug);
            myFree(&malloced_head, (void**) &format, the_debug);
		}
		myFree(&malloced_head, (void**) &a_line, the_debug);
	}
	myFileClose(&file_opened_head, file);
    // END For each row, extract each column value and insert append it

    // START Free arrays for files opened
	for (int i=0; i<table->num_cols; i++)
	{
		if (col_data_info_file_arr[i] != NULL)
			myFileClose(&file_opened_head, col_data_info_file_arr[i]);
		if (col_data_file_arr[i] != NULL)
			myFileClose(&file_opened_head, col_data_file_arr[i]);
	}
	myFree(&malloced_head, (void**) &col_data_info_file_arr, the_debug);
	myFree(&malloced_head, (void**) &col_data_file_arr, the_debug);
    // END Free arrays for files opened
	
	
    if (the_debug == YES_DEBUG)
		printf("Calling myFreeAllCleanup() from createTableFromCSV()\n");
	myFreeAllCleanup(&malloced_head, the_debug);

    if (file_opened_head != NULL)
    {
        if (the_debug == YES_DEBUG)
			printf("createTableFromCSV() did not close all files\n");
        myFileCloseAll(&file_opened_head, the_debug);
    }

    return table->table_cols_head->num_rows;
}

int freeResultsOfSelectIfError(char*** result, int_8* col_numbers, int col_numbers_size, int_8 num_rows, int the_debug)
{
	if (result != NULL)
	{
		for (int j=0; j<col_numbers_size; j++)
		{
			for (int i=0; i<num_rows; i++)
			{
				free(result[j][i]);
			}
			free(result[j]);
		}
		free(result);
	}
		
	if (col_numbers != NULL)
		free(col_numbers);

	return 0;
}

int displayResultsOfSelectAndFree(char*** result, struct table_info* the_table, int_8* col_numbers, int col_numbers_size, int_8 num_rows, struct or_clause_node* or_head, int the_debug)
{
	// START Free where clause
	while (or_head != NULL)
	{
		while (or_head->and_head != NULL)
		{
			struct and_clause_node* temp = or_head->and_head;
			or_head->and_head = or_head->and_head->next;
			free(temp->data_string);
			free(temp);
		}
		struct or_clause_node* temp = or_head;
		or_head= or_head->next;
		free(temp);
	}
	// END Free where clause


	struct file_opened_node* file_opened_head = NULL;


	// START Create .csv file, deleting old if necessary
	if (access("C:\\Users\\David\\Downloads\\Results.csv", F_OK) == 0)
	{
		remove("C:\\Users\\David\\Downloads\\Results.csv");
	}
	FILE* file = myFileOpenSimple(&file_opened_head, "C:\\Users\\David\\Downloads\\Results.csv", "a");
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
			freeResultsOfSelectIfError(result, col_numbers, col_numbers_size, num_rows, the_debug);
			myFileCloseAll(&file_opened_head, the_debug);
			return -1;
		}

		if (j != col_numbers_size-1)
		{
			if (fwrite(",", 1, 1, file) != 1)
			{
				if (the_debug == YES_DEBUG)
					printf("	ERROR in displayResultsOfSelectAndFree() at line %d in %s\n", __LINE__, __FILE__);
				freeResultsOfSelectIfError(result, col_numbers, col_numbers_size, num_rows, the_debug);
				myFileCloseAll(&file_opened_head, the_debug);
				return -1;
			}
		}
		else
		{
			if (fwrite("\n", 1, 1, file) != 1)
			{
				if (the_debug == YES_DEBUG)
					printf("	ERROR in displayResultsOfSelectAndFree() at line %d in %s\n", __LINE__, __FILE__);
				freeResultsOfSelectIfError(result, col_numbers, col_numbers_size, num_rows, the_debug);
				myFileCloseAll(&file_opened_head, the_debug);
				return -1;
			}
		}
	}
	free(col_numbers);
	// END Write column names

	// START Write column data
	for (int i=0; i<num_rows; i++)
	{
		for (int j=0; j<col_numbers_size; j++)
		{
			if (strcontains(result[j][i], ',') == 1)
			{

			}

			if (strcmp(result[j][i], "") != 0)
			{
				if (fwrite(result[j][i], strLength(result[j][i]), 1, file) != 1)
				{
					if (the_debug == YES_DEBUG)
					{
						printf("	ERROR in displayResultsOfSelectAndFree() at line %d in %s\n", __LINE__, __FILE__);
						printf("	result[j][i] = _%s_\n", result[j][i]);
					}
					freeResultsOfSelectIfError(result, NULL, col_numbers_size, num_rows, the_debug);
					myFileCloseAll(&file_opened_head, the_debug);
					return -1;
				}
			}
				

			if (j != col_numbers_size-1)
			{
				if (fwrite(",", 1, 1, file) != 1)
				{
					if (the_debug == YES_DEBUG)
						printf("	ERROR in displayResultsOfSelectAndFree() at line %d in %s\n", __LINE__, __FILE__);
					freeResultsOfSelectIfError(result, NULL, col_numbers_size, num_rows, the_debug);
					myFileCloseAll(&file_opened_head, the_debug);
					return -1;
				}
			}
			else
			{
				if (fwrite("\n", 1, 1, file) != 1)
				{
					if (the_debug == YES_DEBUG)
						printf("	ERROR in displayResultsOfSelectAndFree() at line %d in %s\n", __LINE__, __FILE__);
					freeResultsOfSelectIfError(result, NULL, col_numbers_size, num_rows, the_debug);
					myFileCloseAll(&file_opened_head, the_debug);
					return -1;
				}
			}
		}
	}
	myFileClose(&file_opened_head, file);
	// END Write column data

	// START Free result
	int_8 total_freed = 0;
	for (int j=0; j<col_numbers_size; j++)
	{
		for (int i=0; i<num_rows; i++)
		{
			free(result[j][i]);
			total_freed++;
		}
		free(result[j]);
		total_freed++;
	}
	free(result);
	total_freed++;

	if (the_debug == YES_DEBUG)
		printf("	Freed %lu things from result\n", total_freed);
	// START Free result

	// START Cleanup
	if (file_opened_head != NULL)
    {
        if (the_debug == YES_DEBUG)
        	printf("displayResultsOfSelectAndFree() did not close all files\n");
        myFileCloseAll(&file_opened_head, the_debug);
    }
	// END Cleanup

	return 0;
}



int main()
{
    int debug = NO_DEBUG;
    printf("\n");

    if (test_Driver_main(debug) != 0)
    	printf("\nTests FAILED\n");
    else
    	printf("\nTests passed let's goooo\n");

    /*
    int initd = initDB(debug);
	if (initd == -1)
		printf("Database initialization had a problem with file i/o, please try again\n\n");
	else if (initd == -2)
		printf("Database initialization had a problem with malloc, please try again\n\n");
	else
		printf("Successfully initialized database\n\n");*.

    /*
	char* table_name = (char*) malloc(sizeof(char) * 32);
	strcpy(table_name, "alc_brands");
	int_8 returnd = createTableFromCSV("C:\\Users\\David\\Desktop\\A_Database_Platform\\Liquor_Brands.csv", table_name, 10, debug);
    if (returnd != 0)
        free(table_name);
	if (returnd == -1)
		printf("Table creation from CSV had a problem with file i/o, please try again\n");
	else if (returnd == -2)
		printf("Table creation from CSV had a problem with malloc, please try again\n");
	else if (returnd == -11)
		printf("Table creation from CSV had a problem: A column name was longer than 31 characters, please try again\n");
	else if (returnd == -12)
		printf("Table creation from CSV had a problem: A datatype was not recognized, please try again\n");
	else if (returnd == -13 || returnd == -14)
		printf("Table creation from CSV had a problem: There was an issue creating the table, please try again\n");
	else if (returnd == -15)
		printf("Table creation from CSV had a problem: There was an issue inserting data into the table, please try again\n");
	else
	{
		printf("Table creation from CSV has successfully:\n");
		printf("	Created the table\n");
		printf("	Inserted %lu rows\n\n", returnd);
	}*/


	/*
	struct or_clause_node* or_head = (struct or_clause_node*) malloc(sizeof(struct or_clause_node));
	or_head->and_head = (struct and_clause_node*) malloc(sizeof(struct and_clause_node));
	or_head->and_head->col_number = 0;
	or_head->and_head->where_type = 1;
	or_head->and_head->data_string = (char*) malloc(sizeof(char) * 32);
	strcpy(or_head->and_head->data_string, "Test_Value 0");

	or_head->and_head->next = NULL;
	or_head->next = NULL;*/

	/*
	struct change_node* change_head = (struct change_node*) malloc(sizeof(struct change_node));
	change_head->transac_id = 0;
	change_head->table_number = 47;
	change_head->col_number = 6;
	change_head->operation = 4;
	change_head->data_type = 3;
	change_head->data_int_date = 0;
	change_head->data_real = 0.0;
	change_head->data_string = (char*) malloc(sizeof(char) * 33);
	strcpy(change_head->data_string, "DEEZ NUTS");
	change_head->next = NULL;*/

	/*
	int deleted = deleteRows(getTablesHead(), or_head, debug);
	if (deleted == -1)
		printf("Deletion of rows had a problem with file i/o, please try again\n\n");
	else if (deleted == -2)
		printf("Deletion of rows had a problem with malloc, please try again\n\n");
	else
		printf("Successfully deleted %lu rows from table\n\n", deleted);*/

	/*
	int updated = updateRows(getTablesHead(), change_head, or_head, debug);
	if (updated == -1)
		printf("Update of rows had a problem with file i/o, please try again\n\n");
	else
		printf("Successfully updated %lu rows in table\n\n", updated);

	free(change_head->data_string);
	free(change_head);

	free(or_head->and_head->data_string);
	free(or_head->and_head);
	free(or_head);*/


	
	/*
	int_8* col_numbers = (int_8*) malloc(sizeof(int_8) * 2);
	col_numbers[0] = 0;
	col_numbers[1] = 4;
	int col_numbers_size = 2;
	int_8 num_rows_in_result = 0;*/

	/*
	struct or_clause_node* or_head = (struct or_clause_node*) malloc(sizeof(struct or_clause_node));
	or_head->and_head = (struct and_clause_node*) malloc(sizeof(struct and_clause_node));
	or_head->and_head->col_number = 0;
	or_head->and_head->where_type = 2;
	or_head->and_head->data_string = (char*) malloc(sizeof(char) * 32);
	strcpy(or_head->and_head->data_string, "EDINBURGH GIN SEASIDE");
	
	or_head->and_head->next = NULL;*/

	/*
	or_head->and_head->next = (struct and_clause_node*) malloc(sizeof(struct and_clause_node));
	or_head->and_head->next->col_number = 0;
	or_head->and_head->next->where_type = 1;
	or_head->and_head->next->data_string = (char*) malloc(sizeof(char) * 32);
	strcpy(or_head->and_head->next->data_string, "BABAROSA MOSCATO D'ASTI");

	or_head->and_head->next->next = NULL;*/
	
	/*
	or_head->next = (struct or_clause_node*) malloc(sizeof(struct or_clause_node));
	or_head->next->and_head = (struct and_clause_node*) malloc(sizeof(struct and_clause_node));
	or_head->next->and_head->col_number = 0;
	or_head->next->and_head->where_type = 2;
	or_head->next->and_head->data_string = (char*) malloc(sizeof(char) * 32);
	strcpy(or_head->next->and_head->data_string, "BABAROSA MOSCATO D'ASTI");
	or_head->next->and_head->next = NULL;

	or_head->next->next = NULL;*/

	/*
	char*** result = select(getTablesHead(), col_numbers, col_numbers_size, &num_rows_in_result, NULL, debug);
	if (result == NULL)
	{
		printf("Data retreival from disk FAILED, please try again\n\n");
		free(col_numbers);
	}
	else
	{
		printf("Successfully retreived %lu rows from disk, creating .csv file\n", num_rows_in_result);
		if (displayResultsOfSelectAndFree(result, getTablesHead(), col_numbers, col_numbers_size, num_rows_in_result, NULL, debug) != 0)
			printf("Creation of .csv file had a problem with file i/o, please try again.\n\n");
		else 
			printf("Successfully created C:\\Users\\David\\Downloads\\Results.csv\n\n");
	}*/


	/*
	struct change_node_v2* change_head = (struct change_node_v2*) malloc(sizeof(struct change_node_v2));
	change_head->col_number = 0;
	change_head->operation = 1;
	change_head->data_type = 3;
	change_head->data = malloc(sizeof(char) * 32);
	strcpy(change_head->data, "Test_Value 0");

	struct change_node_v2* tail = change_head;

	tail->next = (struct change_node_v2*) malloc(sizeof(struct change_node_v2));
	tail->next->col_number = 1;
	tail->next->operation = 1;
	tail->next->data_type = 1;
	tail->next->data = malloc(sizeof(char) * 2);
	strcpy(tail->next->data, "1");

	tail = tail->next;

	tail->next = (struct change_node_v2*) malloc(sizeof(struct change_node_v2));
	tail->next->col_number = 2;
	tail->next->operation = 1;
	tail->next->data_type = 3;
	tail->next->data = malloc(sizeof(char) * 32);
	strcpy(tail->next->data, "Test_Value 2");

	tail = tail->next;

	tail->next = (struct change_node_v2*) malloc(sizeof(struct change_node_v2));
	tail->next->col_number = 3;
	tail->next->operation = 1;
	tail->next->data_type = 4;
	tail->next->data = malloc(sizeof(char) * 32);
	strcpy(tail->next->data, "2/7/1900");

	tail = tail->next;

	tail->next = (struct change_node_v2*) malloc(sizeof(struct change_node_v2));
	tail->next->col_number = 4;
	tail->next->operation = 1;
	tail->next->data_type = 4;
	tail->next->data = malloc(sizeof(char) * 32);
	strcpy(tail->next->data, "2/8/1900");

	tail = tail->next;

	tail->next = (struct change_node_v2*) malloc(sizeof(struct change_node_v2));
	tail->next->col_number = 5;
	tail->next->operation = 1;
	tail->next->data_type = 3;
	tail->next->data = malloc(sizeof(char) * 32);
	strcpy(tail->next->data, "Test_Value 5");

	tail = tail->next;

	tail->next = (struct change_node_v2*) malloc(sizeof(struct change_node_v2));
	tail->next->col_number = 6;
	tail->next->operation = 1;
	tail->next->data_type = 3;
	tail->next->data = malloc(sizeof(char) * 32);
	strcpy(tail->next->data, "Test_Value 6");

	tail->next->next = NULL;

	
	int inserted = insertRows(getTablesHead(), change_head, debug);
	if (inserted == -1)
		printf("Insertion of rows had a problem with file i/o, please try again\n");
	else if (inserted == -2)
		printf("Insertion of rows had a problem with malloc, please try again\n");
	else
		printf("Successfully inserted %d rows into table\n\n", inserted);

	while (change_head != NULL)
	{
		struct change_node_v2* temp = change_head;
		change_head = change_head->next;
		free(temp->data);
		free(temp);
	}*/
	

	//traverseTablesInfoMemory();
	

    /*
    while (freeMemOfDB(debug) != 0)
        printf("Teardown FAILED\n");
    printf("Successfully teared down database\n");*/


	//traverseTablesInfoDisk(debug);


	/*struct malloced_node* malloced_head = NULL;
	struct ListNode* head = NULL;
	struct ListNode* tail = NULL;

	addListNode(&malloced_head, &head, &tail, 10, 0, ADDLISTNODE_HEAD);
	addListNode(&malloced_head, &head, &tail, 11, 0, ADDLISTNODE_HEAD);
	addListNode(&malloced_head, &head, &tail, 12, 0, ADDLISTNODE_HEAD);
	addListNode(&malloced_head, &head, &tail, 13, 0, ADDLISTNODE_HEAD);
	addListNode(&malloced_head, &head, &tail, 14, 0, ADDLISTNODE_HEAD);

	printf("Removed %d from head\n", removeListNode(&malloced_head, &head, &tail, -1, TRAVERSELISTNODES_HEAD, debug));
	printf("Removed %d from head\n", removeListNode(&malloced_head, &head, &tail, -1, TRAVERSELISTNODES_HEAD, debug));

	traverseListNodes(&head, &tail, TRAVERSELISTNODES_TAIL, "List: ");
`

	printf("Freed %d ListNodes\n", freeListNodes(&malloced_head, &head, debug));
	if (head != NULL)
		printf("Shite\n");*/

	//system("certutil -hashfile C:\\Users\\David\\Desktop\\DB_Files_Backup\\DB_Col_Data_38_0.bin");

	return 0;
}