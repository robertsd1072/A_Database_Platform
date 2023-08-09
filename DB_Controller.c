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

int callCreateTable(char* table_name, struct table_cols_info* table_cols)
{
	int created_table = createTable(table_name, table_cols);
	if (created_table == -1)
		printf("createTable() had a problem with file i/o\n");
	else if (created_table == -2)
		printf("createTable() had a problem with malloc\n");
	else
		printf("createTable() was successful\n");
	
	return created_table;
}

int createTableFromCSV(char* input, char* table_name)
{
	struct malloced_node* malloced_head = NULL;
    struct file_opened_node* file_opened_head = NULL;
	
	/**/
	// START Open csv file and read in first row of column names and datatypes
    FILE* file = myFileOpenSimple(&file_opened_head, input,  "r");
	if (file == NULL)
	{
		printf("	ERROR in createTableFromCSV() at line %d in %s\n", __LINE__, __FILE__);
        myFreeAllError(&malloced_head);
        myFileCloseAll(&file_opened_head);
		return -1;
	}

	char* first_row = (char*) myMalloc(&malloced_head, sizeof(char) * 1000, 0);
	if (first_row == NULL)
	{
		printf("	ERROR in createTableFromCSV() at line %d in %s\n", __LINE__, __FILE__);
        myFreeAllError(&malloced_head);
        myFileCloseAll(&file_opened_head);
		return -2;
	}
	fscanf(file, "%s\n", first_row);
	myFileClose(&file_opened_head, file);
    // END Open csv file and read in first row of column names and datatypes
	
	// START Allocate space for beginning of table_cols
    struct table_cols_info* table_cols = (struct table_cols_info*) myMalloc(&malloced_head, sizeof(struct table_cols_info), 1);
	if (table_cols == NULL)
	{
		printf("	ERROR in createTableFromCSV() at line %d in %s\n", __LINE__, __FILE__);
        myFreeAllError(&malloced_head);
        myFileCloseAll(&file_opened_head);
		return -2;
	}
	struct table_cols_info* cur_col = table_cols;
    // END Allocate space for beginning of table_cols
	
	// START Split first_row by each comma and extract column name and datatype
    int num_cols = 0;
	char** col_names = strSplit(&malloced_head, first_row, ',', &num_cols, 0);
	if (myFree(&malloced_head, (void**) &first_row) != 0)
	{
		printf("	ERROR in createTableFromCSV() at line %d in %s\n", __LINE__, __FILE__);
		myFreeAllError(&malloced_head);
		myFileCloseAll(&file_opened_head);
		return -2;
	}

	for (int i=0; i<num_cols; i++)
	{
		char* temp1 = substring(&malloced_head, col_names[i], 0, indexOf(col_names[i], ':')-1, 1);
		char* temp2 = substring(&malloced_head, col_names[i], indexOf(col_names[i], ':')+1, indexOf(col_names[i], 0), 0);
		
		if (indexOf(temp1, 0) > 32)
		{
			printf("	ERROR in createTableFromCSV() at line %d in %s\n", __LINE__, __FILE__);
            myFreeAllError(&malloced_head);
            myFileCloseAll(&file_opened_head);
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
			printf("	ERROR in createTableFromCSV() at line %d in %s\n", __LINE__, __FILE__);
            myFreeAllError(&malloced_head);
            myFileCloseAll(&file_opened_head);
			return -12;
		}

		if (myFree(&malloced_head, (void**) &temp2) != 0 || myFree(&malloced_head, (void**) &datatype) != 0 || myFree(&malloced_head, (void**) &col_names[i]) != 0)
		{
			printf("	ERROR in createTableFromCSV() at line %d in %s\n", __LINE__, __FILE__);
			myFreeAllError(&malloced_head);
			myFileCloseAll(&file_opened_head);
			return -2;
		}

		if (i < num_cols-1)
		{
			cur_col->next = (struct table_cols_info*) myMalloc(&malloced_head, sizeof(struct table_cols_info), 1);
			if (cur_col->next == NULL)
			{
				printf("	ERROR in createTableFromCSV() at line %d in %s\n", __LINE__, __FILE__);
				myFreeAllError(&malloced_head);
                myFileCloseAll(&file_opened_head);
				return -2;
			}
			cur_col = cur_col->next;
		}
		else
			cur_col->next = NULL;
	}
	if (myFree(&malloced_head, (void**) &col_names) != 0)
	{
		printf("	ERROR in createTableFromCSV() at line %d in %s\n", __LINE__, __FILE__);
		myFreeAllError(&malloced_head);
		myFileCloseAll(&file_opened_head);
		return -2;
	}
    // END Split first_row by each comma and extract column name and datatype

	
	
	if (callCreateTable(table_name, table_cols) != 0)
    {
        myFreeAllError(&malloced_head);
        myFileCloseAll(&file_opened_head);
        return -13;
	}

	
	/*
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
		printf("	ERROR in createTableFromCSV() at line %d in %s\n", __LINE__, __FILE__);
        myFreeAllError(&malloced_head);
        myFileCloseAll(&file_opened_head);
		return -14;
	}
    // END Find table in tables_head

	// START Reopen file
	FILE* file = myFileOpenSimple(&file_opened_head, input,  "r");
	char* first_row = (char*) myMalloc(&malloced_head, sizeof(char) * 1000, 0);
	if (first_row == NULL)
	{
		printf("	ERROR in createTableFromCSV() at line %d in %s\n", __LINE__, __FILE__);
        myFreeAllError(&malloced_head);
        myFileCloseAll(&file_opened_head);
		return -2;
	}
	fscanf(file, "%s\n", first_row);
	myFree(&malloced_head, (void**) &first_row);
	// END Reopen file
    
	// START Allocate arrays for files opened
	FILE** col_data_info_file_arr = (FILE**) myMalloc(&malloced_head, sizeof(FILE*) * table->num_cols, 0);
	FILE** col_data_file_arr = (FILE**) myMalloc(&malloced_head, sizeof(FILE*) * table->num_cols, 0);
    if (col_data_info_file_arr == NULL || col_data_file_arr == NULL)
    {
        printf("	ERROR in createTableFromCSV() at line %d in %s\n", __LINE__, __FILE__);
        myFreeAllError(&malloced_head);
        myFileCloseAll(&file_opened_head);
        return -2;
    }

	for (int i=0; i<table->num_cols; i++)
	{
		col_data_info_file_arr[i] = NULL;
		col_data_file_arr[i] = NULL;
	}
	// END Allocate arrays for files opened

    // START For each row, extract each column value and insert append it
	for (int i=0; i<1; i++)
	{
		printf("i = %d\n", i);
        struct table_cols_info* cur_col = table->table_cols_head;
		char* a_line = (char*) myMalloc(&malloced_head, sizeof(char) * 1000, 0);
		fscanf(file, "%[^\n]\n", a_line);
		//printf("a_line = %s\n", a_line);

		for (int j=0; j<table->num_cols; j++)
		{
			char* format = (char*) myMalloc(&malloced_head, sizeof(char) * 200, 0);
			if (format == NULL)
			{
				printf("	ERROR in createTableFromCSV() at line %d in %s\n", __LINE__, __FILE__);
                myFreeAllError(&malloced_head);
                myFileCloseAll(&file_opened_head);
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
				printf("	ERROR in createTableFromCSV() at line %d in %s\n", __LINE__, __FILE__);
                myFreeAllError(&malloced_head);
                myFileCloseAll(&file_opened_head);
				return -2;
			}

			sscanf(a_line, format, data);

			if (strcmp(data, "") == 0)
			{
				printf("	ERROR in createTableFromCSV() at line %d in %s\n", __LINE__, __FILE__);
				myFreeAllError(&malloced_head);
                myFileCloseAll(&file_opened_head);
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
			
            if (cur_col->data_type == 1)
				printf("data = %lu\n", data_int_date);
            else if (cur_col->data_type == 2)
                printf("data = %f\n", data_real);
            else if (cur_col->data_type == 4)
			{
				data_int_date = dateToInt(data);
				printf("data = %lu\n", data_int_date);
			}
            else
				printf("data = %s\n", data);
			
            if (insertAppend(col_data_info_file_arr, col_data_file_arr, &file_opened_head
                            ,table->file_number, cur_col->col_number, cur_col->data_type, &cur_col->num_rows, cur_col->max_length
                            ,data_int_date, data_real, data) != 0)
            {
                myFreeAllError(&malloced_head);
                myFileCloseAll(&file_opened_head);
				return -16;
            }

			cur_col = cur_col->next;

            myFree(&malloced_head, (void**) &data);
            myFree(&malloced_head, (void**) &format);
		}
		myFree(&malloced_head, (void**) &a_line);
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
	myFree(&malloced_head, (void**) &col_data_info_file_arr);
	myFree(&malloced_head, (void**) &col_data_file_arr);
    // END Free arrays for files opened
	*/
	
    printf("Calling myFreeAllCleanup() from createTableFromCSV()\n");
	myFreeAllCleanup(&malloced_head);

    if (file_opened_head != NULL)
    {
        printf("createTableFromCSV() did not close all files\n");
        myFileCloseAll(&file_opened_head);
    }

	return 0;
}




int main()
{
    int initd = initDB();
	if (initd == -1)
		printf("initDB() had a problem with file i/o\n");
	else if (initd == -2)
		printf("initDB() had a problem with malloc\n");
	else
		printf("initDB() was successful\n");

    /*
	char* table_name = (char*) malloc(sizeof(char) * 32);
	strcpy(table_name, "alc_brands");
	int returnd = createTableFromCSV("C:\\Users\\rober\\Downloads\\Liquor_Brands.csv", table_name);
    if (returnd != 0)
        free(table_name);
	if (returnd == -1)
		printf("createTableFromCSV() had a problem with file i/o\n");
	else if (returnd == -2)
		printf("createTableFromCSV() had a problem with malloc\n");
	else if (returnd == -11)
		printf("A column name was longer than 31 characters\n");
	else if (returnd == -12)
		printf("A datatype was not recognized\n");
	else if (returnd == -13 || returnd == -14)
		printf("There was an issue creating the table, please try again\n");
	else if (returnd == -15)
		printf("There was an issue inserting data into the table, please try again\n");
	else
		printf("createTableFromCSV() was successful\n");*/

    //traverseTablesInfoMemory();

    if (freeMemOfDB() != 0)
        printf("freeMemOfDB() failed\n");
    else
        printf("freeMemOfDB() was successful\n");

	//traverseTablesInfoDisk();


	/*clock_t endwait;
    endwait = clock () + 3 * CLOCKS_PER_SEC;
    while (true) 
	{
		if (clock() > endwait)
			break;
	}
	printf("Broken from loop\n");*/

	/*FILE* test_file = fopen("DB_Col_Data_28_0.bin", "ab+");

	int flags = fcntl(test_file, F_GETFL, 0);
	fcntl(test_file, F_SETFL, flags | O_NONBLOCK);

	fclose(test_file);

	if (access("DB_Col_Data_28_0.bin", F_OK) == 0)
	{

	}*/
}