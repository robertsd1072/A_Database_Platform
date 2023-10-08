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


int createTableFromCSV(char* input, char* table_name, int_8 num_rows
					  ,struct malloced_node** malloced_head, int the_debug)
{
	struct file_opened_node* file_opened_head = NULL;
	
	/**/
    if (access(input, F_OK) != 0)
    {
    	if (the_debug == YES_DEBUG)
			printf("	ERROR in createTableFromCSV() at line %d in %s\n", __LINE__, __FILE__);
		errorTeardown(&file_opened_head, malloced_head, the_debug);
		return -1;
    }

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
		
		if (indexOf(temp_but_is_col_name, 0) > 32)
		{
			if (the_debug == YES_DEBUG)
				printf("	ERROR in createTableFromCSV() at line %d in %s\n", __LINE__, __FILE__);
			return -11;
		}
		
		cur_col->col_name = temp_but_is_col_name;
		cur_col->col_number = i;

		//printf("cur_col->col_name = %s\n", cur_col->col_name);
		//printf("cur_col->col_number = %d\n", cur_col->col_number);
		
		char* datatype = substring(temp2, 0, 3, &file_opened_head, malloced_head, the_debug);
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
			return -12;
		}

		if (myFree((void**) &temp2, &file_opened_head, malloced_head, the_debug) != 0 || 
			myFree((void**) &datatype, &file_opened_head, malloced_head, the_debug) != 0 || 
			myFree((void**) &col_names[i], &file_opened_head, malloced_head, the_debug) != 0)
		{
			if (the_debug == YES_DEBUG)
				printf("	ERROR in createTableFromCSV() at line %d in %s\n", __LINE__, __FILE__);
			return -2;
		}

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
	}
	if (myFree((void**) &col_names, &file_opened_head, malloced_head, the_debug) != 0)
	{
		if (the_debug == YES_DEBUG)
			printf("	ERROR in createTableFromCSV() at line %d in %s\n", __LINE__, __FILE__);
		return -2;
	}
    // END Split first_row by each comma and extract column name and datatype



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
		printf("Successfully created table\n");



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
		errorTeardown(&file_opened_head, malloced_head, the_debug);
		return -14;
	}
    // END Find table in tables_head

	// START Reopen file
	//FILE* 
	file = myFileOpenSimple(input, "r", &file_opened_head, malloced_head, the_debug);
	//char* 
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
			printf("In progress... inserted %d rows so far\n", i);
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
	
	
    /*if (the_debug == YES_DEBUG)
		printf("Calling myFreeAllCleanup() from createTableFromCSV()\n");
	myFreeAllCleanup(&malloced_head, the_debug);*/

    if (file_opened_head != NULL)
    {
        if (the_debug == YES_DEBUG)
			printf("createTableFromCSV() did not close all files\n");
        myFileCloseAll(&file_opened_head, malloced_head, the_debug);
    }

    return table->table_cols_head->num_rows;
}

int displayResultsOfSelect(char*** result, struct table_info* the_table, int_8* col_numbers, int col_numbers_size, int_8 num_rows
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

int getNextWord(char* input, char* word, int* cur_index)
{
	while (input[*cur_index] != 0 && (input[*cur_index] == ' ' || input[*cur_index] == '\t' || input[*cur_index] == '\n' || input[*cur_index] == '\v'))
	{
		(*cur_index)++;
	}

	int word_index = 0;
	word[word_index] = 0;

	if (input[*cur_index] == 39 /*Single quote*/)
	{
		for (int i=0; i<2; i++)
		{
			word[word_index] = input[*cur_index];
			word[word_index+1] = 0;

			word_index++;
			(*cur_index)++;
		}

		bool two_single_quotes = false;
		while (two_single_quotes || !(input[*cur_index] == 0 || (input[(*cur_index)-1] == 39 && input[*cur_index] != 39) 
									  || input[*cur_index] == ';' || input[*cur_index] == ','))
		{
			if (two_single_quotes)
				two_single_quotes = false;
			if (input[(*cur_index)-1] == 39 && input[*cur_index] == 39)
			{
				(*cur_index)++;
				two_single_quotes = true;
			}
			else
			{
				word[word_index] = input[*cur_index];
				word[word_index+1] = 0;

				word_index++;
				(*cur_index)++;
			}
		}
	}
	else
	{
		while (input[*cur_index] != 0 && input[*cur_index] != ' ' && input[*cur_index] != ',' && input[*cur_index] != ';'
			   && input[*cur_index] != '(' && input[*cur_index] != ')'
			   && input[*cur_index] != '\t' && input[*cur_index] != '\n' && input[*cur_index] != '\v')
		{
			word[word_index] = input[*cur_index];
			word[word_index+1] = 0;

			word_index++;
			(*cur_index)++;
		}

		/*if (input[index] == 0)
		{
			if (the_debug == YES_DEBUG)
				printf("	ERROR in parseWhereClause() at line %d in %s\n", __LINE__, __FILE__);
			errorTeardown(NULL, malloced_head, the_debug);
			*error_code = -1;
			return NULL;
		}*/
	}

	//printf("word = _%s_\n", word);

	if (strcmp(word, "") == 0)
	{
		//printf("First char: %c\n", input[*cur_index]);
		if (input[*cur_index] == ',' || input[*cur_index] == ';' || input[*cur_index] == '(' || input[*cur_index] == ')')
		{
			word[word_index] = input[*cur_index];
			word[word_index+1] = 0;

			word_index++;
			(*cur_index)++;
		}
		else
			return -1;
	}

	return 0;
}

int strcmp_Upper(char* word, char* test_char
				,struct file_opened_node** file_opened_head, struct malloced_node** malloced_head, int the_debug)
{
	char* upper_word = upper(word, file_opened_head, malloced_head, the_debug);
	if (upper_word == NULL)
	{
		if (the_debug == YES_DEBUG)
			printf("	ERROR in strcmp_Upper() at line %d in %s\n", __LINE__, __FILE__);
		return -2;
	}

	int compared = strcmp(upper_word, test_char);
	myFree((void**) &upper_word, file_opened_head, malloced_head, the_debug);

	return compared;
}

int parseInput(char* input
			  ,struct malloced_node** malloced_head, int the_debug)
{
	char* cmd = substring(input, 0, 6, NULL, malloced_head, the_debug);
	printf("cmd = _%s_\n", cmd);
	if (cmd == NULL)
	{
		if (the_debug == YES_DEBUG)
			printf("	ERROR in parseInput() at line %d in %s\n", __LINE__, __FILE__);
		return -1;
	}
	else if (strcmp(cmd, "create") == 0)
	{
		myFree((void**) &cmd, NULL, malloced_head, the_debug);
	}
	else if (strcmp(cmd, "select") == 0)
	{
		myFree((void**) &cmd, NULL, malloced_head, the_debug);
	}
	else if (strcmp(cmd, "insert") == 0)
	{
		myFree((void**) &cmd, NULL, malloced_head, the_debug);
	}
	else if (strcmp(cmd, "update") == 0)
	{
		myFree((void**) &cmd, NULL, malloced_head, the_debug);
	}
	else if (strcmp(cmd, "delete") == 0)
	{
		myFree((void**) &cmd, NULL, malloced_head, the_debug);
	}

	return 0;
}

struct or_clause_node* parseWhereClause(char* input, struct table_info* the_table, int* error_code
									   ,struct malloced_node** malloced_head, int the_debug)
{
	struct table_info* cur_table = the_table;

	//printf("Here A\n");

	int index = 0;
	if ((input[index] == 'w' || input[index] == 'W') && (input[index+1] == 'h' || input[index+1] == 'H') && (input[index+2] == 'e' || input[index+2] == 'E')
		&& (input[index+3] == 'r' || input[index+3] == 'R') && (input[index+4] == 'e' || input[index+4] == 'E') && input[index+5] == ' ')
		index += 6;
	else
	{
		*error_code = 0;
		return NULL;
	}

	//printf("Here B\n");

	struct or_clause_node* or_head = (struct or_clause_node*) myMalloc(sizeof(struct or_clause_node), NULL, malloced_head, the_debug);
	if (or_head == NULL)
	{
		if (the_debug == YES_DEBUG)
			printf("	ERROR in parseWhereClause() at line %d in %s\n", __LINE__, __FILE__);
		*error_code = -1;
		return NULL;
	}
	or_head->next = NULL;

	//printf("Here C\n");

	or_head->and_head = (struct and_clause_node*) myMalloc(sizeof(struct and_clause_node), NULL, malloced_head, the_debug);
	if (or_head->and_head == NULL)
	{
		if (the_debug == YES_DEBUG)
			printf("	ERROR in parseWhereClause() at line %d in %s\n", __LINE__, __FILE__);
		*error_code = -1;
		return NULL;
	}
	or_head->and_head->col_number = -1;
	or_head->and_head->where_type = -1;
	or_head->and_head->data_string = NULL;
	or_head->and_head->next = NULL;

	struct or_clause_node* cur_or = or_head;
	struct and_clause_node* cur_and = or_head->and_head;

	//printf("Here D\n");

	char* word = (char*) myMalloc(sizeof(char) * 200, NULL, malloced_head, the_debug);
	//printf("Here D 2\n");
	if (word == NULL)
	{
		if (the_debug == YES_DEBUG)
			printf("	ERROR in parseWhereClause() at line %d in %s\n", __LINE__, __FILE__);
		*error_code = -1;
		return NULL;
	}
	word[0] = 0;
	//int word_index = 0;

	//printf("Here E\n");

	while (getNextWord(input, word, &index) == 0)
	{
		if (word[0] != 0 && word[0] != ';')
		{
			//printf("word: _%s_\n", word);
			if ((word[0] == 'o' || word[0] == 'O') && (word[1] == 'r' || word[1] == 'R'))
			{
				if (cur_and->col_number == -1 || cur_and->where_type == -1 || cur_and->data_string == NULL)
				{
					if (the_debug == YES_DEBUG)
						printf("	ERROR in parseWhereClause() at line %d in %s\n", __LINE__, __FILE__);
					errorTeardown(NULL, malloced_head, the_debug);
					*error_code = -1;
					return NULL;
				}

				//printf("Option 1\n");
				cur_or->next = (struct or_clause_node*) myMalloc(sizeof(struct or_clause_node), NULL, malloced_head, the_debug);
				if (cur_or->next == NULL)
				{
					if (the_debug == YES_DEBUG)
						printf("	ERROR in parseWhereClause() at line %d in %s\n", __LINE__, __FILE__);
					*error_code = -1;
					return NULL;
				}
				cur_or->next->next = NULL;

				cur_or->next->and_head = (struct and_clause_node*) myMalloc(sizeof(struct and_clause_node), NULL, malloced_head, the_debug);
				if (cur_or->next->and_head == NULL)
				{
					if (the_debug == YES_DEBUG)
						printf("	ERROR in parseWhereClause() at line %d in %s\n", __LINE__, __FILE__);
					*error_code = -1;
					return NULL;
				}
				cur_or->next->and_head->col_number = -1;
				cur_or->next->and_head->where_type = -1;
				cur_or->next->and_head->data_string = NULL;
				cur_or->next->and_head->next = NULL;

				cur_or = cur_or->next;
				cur_and = cur_or->and_head;
			}
			else if ((word[0] == 'a' || word[0] == 'A') && (word[1] == 'n' || word[1] == 'N') && (word[2] == 'd' || word[2] == 'D'))
			{
				if (cur_and->col_number == -1 || cur_and->where_type == -1 || cur_and->data_string == NULL)
				{
					if (the_debug == YES_DEBUG)
						printf("	ERROR in parseWhereClause() at line %d in %s\n", __LINE__, __FILE__);
					errorTeardown(NULL, malloced_head, the_debug);
					*error_code = -1;
					return NULL;
				}

				//printf("Option 2\n");
				cur_and->next = (struct and_clause_node*) myMalloc(sizeof(struct and_clause_node), NULL, malloced_head, the_debug);
				if (cur_and->next == NULL)
				{
					if (the_debug == YES_DEBUG)
						printf("	ERROR in parseWhereClause() at line %d in %s\n", __LINE__, __FILE__);
					*error_code = -1;
					return NULL;
				}
				cur_and->next->col_number = -1;
				cur_and->next->where_type = -1;
				cur_and->next->data_string = NULL;
				cur_and->next->next = NULL;

				cur_and = cur_and->next;
			}
			else if (word[0] == '=' || (word[0] == '<' && word[1] == '>'))
			{
				if (cur_and->where_type != -1)
				{
					if (the_debug == YES_DEBUG)
						printf("	ERROR in parseWhereClause() at line %d in %s\n", __LINE__, __FILE__);
					errorTeardown(NULL, malloced_head, the_debug);
					*error_code = -1;
					return NULL;
				}

				//printf("Option 3\n");
				cur_and->where_type = (word[0] == '=' ? WHERE_IS_EQUALS : WHERE_NOT_EQUALS);
			}
			else if (word[0] == 39)
			{
				struct table_cols_info* cur_col = cur_table->table_cols_head;
				while (cur_col != NULL)
				{
					if (cur_col->col_number == cur_and->col_number)
						break;

					cur_col = cur_col->next;
				}

				if (cur_and->data_string != NULL || (cur_col != NULL && (cur_col->data_type == DATA_INT || cur_col->data_type == DATA_REAL)))
				{
					if (the_debug == YES_DEBUG)
						printf("	ERROR in parseWhereClause() at line %d in %s\n", __LINE__, __FILE__);
					errorTeardown(NULL, malloced_head, the_debug);
					*error_code = -1;
					return NULL;
				}

				//printf("Option 4\n");
				cur_and->data_string = substring(word, 1, strLength(word)-2, NULL, malloced_head, the_debug);
				if (cur_and->data_string == NULL)
				{
					if (the_debug == YES_DEBUG)
						printf("	ERROR in parseWhereClause() at line %d in %s\n", __LINE__, __FILE__);
					*error_code = -1;
					return NULL;
				}
			}
			else if (strIsNotEmpty(word))
			{
				struct table_cols_info* cur_col = cur_table->table_cols_head;
				while (cur_col != NULL)
				{
					char* upper_word = upper(word, NULL, malloced_head, the_debug);
					char* upper_col_name = upper(cur_col->col_name, NULL, malloced_head, the_debug);
					if (upper_word == NULL || upper_col_name == NULL)
					{
						if (the_debug == YES_DEBUG)
							printf("	ERROR in parseWhereClause() at line %d in %s\n", __LINE__, __FILE__);
						*error_code = -1;
						return NULL;
					}

					//printf("upper_word = _%s_\n", upper_word);
					//printf("upper_col_name = _%s_\n", upper_col_name);

					int cmp_result = strcmp(upper_word, upper_col_name);

					myFree((void**) &upper_word, NULL, malloced_head, the_debug);
					myFree((void**) &upper_col_name, NULL, malloced_head, the_debug);

					if (cmp_result == 0)
					{
						if (cur_and->col_number != -1)
						{
							if (the_debug == YES_DEBUG)
								printf("	ERROR in parseWhereClause() at line %d in %s\n", __LINE__, __FILE__);
							errorTeardown(NULL, malloced_head, the_debug);
							*error_code = -1;
							return NULL;
						}

						//printf("Option 5\n");
						cur_and->col_number = cur_col->col_number;
						break;
					}

					cur_col = cur_col->next;
				}

				if (cur_col == NULL)
				{
					cur_col = cur_table->table_cols_head;
					while (cur_col != NULL)
					{
						if (cur_col->col_number == cur_and->col_number)
							break;

						cur_col = cur_col->next;
					}

					if (cur_and->data_string != NULL || (cur_col != NULL && (cur_col->data_type == DATA_STRING || cur_col->data_type == DATA_DATE)))
					{
						if (the_debug == YES_DEBUG)
							printf("	ERROR in parseWhereClause() at line %d in %s\n", __LINE__, __FILE__);
						errorTeardown(NULL, malloced_head, the_debug);
						*error_code = -1;
						return NULL;
					}

					//printf("Option 6\n");
					cur_and->data_string = (char*) myMalloc(sizeof(char) * 64, NULL, malloced_head, the_debug);
					if (cur_and->data_string == NULL)
					{
						if (the_debug == YES_DEBUG)
							printf("	ERROR in parseWhereClause() at line %d in %s\n", __LINE__, __FILE__);
						*error_code = -1;
						return NULL;
					}
					strcpy(cur_and->data_string, word);
				}
			}
		}
		//word[0] = 0;
		//word_index = 0;

		//index++;
	}
	myFree((void**) &word, NULL, malloced_head, the_debug);

	if (or_head->and_head->col_number == -1 || or_head->and_head->where_type == -1 || or_head->and_head->data_string == NULL)
	{
		if (the_debug == YES_DEBUG)
			printf("	ERROR in parseWhereClause() at line %d in %s\n", __LINE__, __FILE__);
		errorTeardown(NULL, malloced_head, the_debug);
		*error_code = -1;
		return NULL;
	}

	/*
	printf("here it is\n");
	struct or_clause_node* cur_or_2 = or_head;
	while (cur_or_2 != NULL)
	{
		printf("New or node\n");
		struct and_clause_node* cur_and_2 = cur_or_2->and_head;
		while (cur_and_2 != NULL)
		{
			printf("	New and node\n");
			printf("		col_number = %lu\n", cur_and_2->col_number);
			printf("		where_type = %lu\n", cur_and_2->where_type);
			printf("		data_string = %s\n", cur_and_2->data_string);

			cur_and_2 = cur_and_2->next;
		}
		
		cur_or_2 = cur_or_2->next;
	}*/

	/*if (the_debug == YES_DEBUG)
		printf("Calling myFreeAllCleanup() from parseWhereClause()\n");
	myFreeAllCleanup(&malloced_head, the_debug);*/

	*error_code = 0;

	return or_head;
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
		errorTeardown(NULL, malloced_head, the_debug);
		return NULL;
	}
	// END Get table name and find table node in list

	//printf("table name = %s\n", cur_table->name);

	return cur_table;
}

int parseUpdate(char* input, struct change_node_v2** change_head, struct or_clause_node** or_head
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
			*or_head = parseWhereClause(where_clause, cur_table, &error_code, malloced_head, the_debug);

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
	}
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

	return 0;
}

int parseDelete(char* input, struct or_clause_node** or_head, struct table_info** table
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
			*or_head = parseWhereClause(where_clause, *table, &error_code, malloced_head, the_debug);

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

int parseInsert(char* input, struct change_node_v2** change_head, struct table_info** table
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

		/**/
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

	freeListNodes(&list_head, NULL, malloced_head, the_debug);

	return 0;
}

int parseSelect(char* input, int_8** col_numbers_arr, int* col_numbers_size, struct or_clause_node** or_head, struct table_info** the_table
			   ,struct malloced_node** malloced_head, int the_debug)
{
	// START Init word variable
	char* word = (char*) myMalloc(sizeof(char) * 200, NULL, malloced_head, the_debug);
	if (word == NULL)
	{
		if (the_debug == YES_DEBUG)
			printf("	ERROR in parseSelect() at line %d in %s\n", __LINE__, __FILE__);
		*col_numbers_arr = NULL;
		*or_head = NULL;
		*the_table = NULL;
		*col_numbers_size = 0;
		return -1;
	}
	word[0] = 0;
	int index = 0;
	// END Init word variable

	// START See if first word is SELECT
	getNextWord(input, word, &index);

	char* upper_word = upper(word, NULL, malloced_head, the_debug);
	int compared = strcmp(upper_word, "SELECT");
	myFree((void**) &upper_word, NULL, malloced_head, the_debug);

	if (compared != 0)
	{
		if (the_debug == YES_DEBUG)
			printf("	ERROR in parseSelect() at line %d in %s\n", __LINE__, __FILE__);
		errorTeardown(NULL, malloced_head, the_debug);
		*col_numbers_arr = NULL;
		*or_head = NULL;
		*the_table = NULL;
		*col_numbers_size = 0;
		return -1;
	}
	// END See if first word is SELECT

	// START Get columns
	bool select_star = false;
	struct ListNodePtr* col_names_head = NULL;
	struct ListNodePtr* col_names_tail = NULL;

	getNextWord(input, word, &index);

	compared = strcmp(word, "*");

	if (compared == 0)
	{
		select_star = true;

		getNextWord(input, word, &index);
	}
	else
	{
		col_names_head = (struct ListNodePtr*) myMalloc(sizeof(struct ListNodePtr), NULL, malloced_head, the_debug);
		if (col_names_head == NULL)
		{
			if (the_debug == YES_DEBUG)
				printf("	ERROR in parseSelect() at line %d in %s\n", __LINE__, __FILE__);
			*col_numbers_arr = NULL;
			*or_head = NULL;
			*the_table = NULL;
			*col_numbers_size = 0;
			return -1;
		}
		col_names_tail = col_names_head;

		col_names_head->ptr_value = upper(word, NULL, malloced_head, the_debug);
		if (col_names_head->ptr_value == NULL)
		{
			if (the_debug == YES_DEBUG)
				printf("	ERROR in parseSelect() at line %d in %s\n", __LINE__, __FILE__);
			*col_numbers_arr = NULL;
			*or_head = NULL;
			*the_table = NULL;
			*col_numbers_size = 0;
			return -1;
		}

		col_names_head->prev = NULL;
		col_names_head->next = NULL;

		(*col_numbers_size)++;

		compared = 0;

		while (getNextWord(input, word, &index) == 0 && (compared = strcmp_Upper(word, "FROM", NULL, malloced_head, the_debug)) != 0)
		{
			if (compared == -2)
			{
				if (the_debug == YES_DEBUG)
					printf("	ERROR in parseSelect() at line %d in %s\n", __LINE__, __FILE__);
				*col_numbers_arr = NULL;
				*or_head = NULL;
				*the_table = NULL;
				*col_numbers_size = 0;
				return -1;
			}

			if (word[0] == ',')
			{
				col_names_tail->next = (struct ListNodePtr*) myMalloc(sizeof(struct ListNodePtr), NULL, malloced_head, the_debug);
				if (col_names_tail->next == NULL)
				{
					if (the_debug == YES_DEBUG)
						printf("	ERROR in parseSelect() at line %d in %s\n", __LINE__, __FILE__);
					*col_numbers_arr = NULL;
					*or_head = NULL;
					*the_table = NULL;
					*col_numbers_size = 0;
					return -1;
				}
				
				col_names_tail->next->ptr_value = NULL;
				col_names_tail->next->prev = col_names_tail;
				col_names_tail->next->next = NULL;

				col_names_tail = col_names_tail->next;
			}
			else
			{
				if (col_names_tail->ptr_value != NULL)
				{
					if (the_debug == YES_DEBUG)
						printf("	ERROR in parseSelect() at line %d in %s\n", __LINE__, __FILE__);
					errorTeardown(NULL, malloced_head, the_debug);
					*col_numbers_arr = NULL;
					*or_head = NULL;
					*the_table = NULL;
					*col_numbers_size = 0;
					return -1;
				}

				col_names_tail->ptr_value = upper(word, NULL, malloced_head, the_debug);
				if (col_names_tail->ptr_value == NULL)
				{
					if (the_debug == YES_DEBUG)
						printf("	ERROR in parseSelect() at line %d in %s\n", __LINE__, __FILE__);
					*col_numbers_arr = NULL;
					*or_head = NULL;
					*the_table = NULL;
					*col_numbers_size = 0;
					return -1;
				}

				(*col_numbers_size)++;
			}
		}
	}
	// END Get columns

	// START Get table
	getNextWord(input, word, &index);

	*the_table = getTableFromName(word, malloced_head, the_debug);

	if (*the_table == NULL)
	{
		if (the_debug == YES_DEBUG)
			printf("	ERROR in parseSelect() at line %d in %s\n", __LINE__, __FILE__);
		errorTeardown(NULL, malloced_head, the_debug);
		*col_numbers_arr = NULL;
		*or_head = NULL;
		*the_table = NULL;
		*col_numbers_size = 0;
		return -1;
	}
	// END Get table

	// START Get column numbers from names
	if (select_star)
	{
		*col_numbers_size = (*the_table)->num_cols;
		*col_numbers_arr = (int_8*) myMalloc(sizeof(int_8) * (*col_numbers_size), NULL, malloced_head, the_debug);
		int index = 0;

		struct table_cols_info* cur_col = (*the_table)->table_cols_head;
		while (cur_col != NULL)
		{
			(*col_numbers_arr)[index] = cur_col->col_number;

			index++;
			cur_col = cur_col->next;
		}
	}
	else
	{
		*col_numbers_arr = (int_8*) myMalloc(sizeof(int_8) * (*col_numbers_size), NULL, malloced_head, the_debug);
		int index = 0;

		struct ListNodePtr* cur = col_names_head;
		while (cur != NULL)
		{
			struct table_cols_info* cur_col = (*the_table)->table_cols_head;
			while (cur_col != NULL)
			{
				char* upper_col = upper(cur_col->col_name, NULL, malloced_head, the_debug);
				compared = strcmp(upper_col, cur->ptr_value);
				myFree((void**) &upper_col, NULL, malloced_head, the_debug);

				if (compared == 0)
					break;

				cur_col = cur_col->next;
			}

			if (cur_col == NULL)
			{
				if (the_debug == YES_DEBUG)
					printf("	ERROR in parseSelect() at line %d in %s\n", __LINE__, __FILE__);
				errorTeardown(NULL, malloced_head, the_debug);
				*col_numbers_arr = NULL;
				*or_head = NULL;
				*the_table = NULL;
				*col_numbers_size = 0;
				return -1;
			}

			(*col_numbers_arr)[index] = cur_col->col_number;

			index++;
			cur = cur->next;
		}

		while (col_names_head != NULL)
		{
			struct ListNodePtr* temp = col_names_head;
			col_names_head = col_names_head->next;

			myFree((void**) &temp->ptr_value, NULL, malloced_head, the_debug);
			myFree((void**) &temp, NULL, malloced_head, the_debug);
		}
	}
	// END Get column numbers from names

	/*
	printf("col_numbers_arr = ");
	for (int i=0; i<(*col_numbers_size); i++)
	{
		printf("%d,", (*col_numbers_arr)[i]);
	}
	printf("\n");*/

	// START Get where clause
	if (getNextWord(input, word, &index) == 0 && word[0] != ';')
	{
		compared = strcmp_Upper(word, "WHERE", NULL, malloced_head, the_debug);
		if (compared == -2)
		{
			if (the_debug == YES_DEBUG)
				printf("	ERROR in parseSelect() at line %d in %s\n", __LINE__, __FILE__);
			*col_numbers_arr = NULL;
			*or_head = NULL;
			*the_table = NULL;
			*col_numbers_size = 0;
			return -1;
		}
		else if (compared == 0)
		{
			//printf("word = _%s_\n", word);
			//printf("%c%c%c\n", input[index-1], input[index], input[index+1]);

			index -= 5;

			char* where_clause = substring(input, index, strLength(input)-1, NULL, malloced_head, the_debug);
			if (where_clause == NULL)
			{
				if (the_debug == YES_DEBUG)
					printf("	ERROR in parseSelect() at line %d in %s\n", __LINE__, __FILE__);
				*col_numbers_arr = NULL;
				*or_head = NULL;
				*the_table = NULL;
				*col_numbers_size = 0;
				return -1;
			}
			//printf("where_clause = _%s_\n", where_clause);

			if (or_head != NULL)
			{
				int error_code;
				*or_head = parseWhereClause(where_clause, *the_table, &error_code, malloced_head, the_debug);

				if (error_code != 0)
				{
					if (the_debug == YES_DEBUG)
						printf("	ERROR in parseSelect() at line %d in %s\n", __LINE__, __FILE__);
					errorTeardown(NULL, malloced_head, the_debug);
					*col_numbers_arr = NULL;
					*or_head = NULL;
					*the_table = NULL;
					*col_numbers_size = 0;
					return -1;
				}
			}

			myFree((void**) &where_clause, NULL, malloced_head, the_debug);
		}
		else if (compared != 0)
		{
			if (the_debug == YES_DEBUG)
				printf("	ERROR in parseSelect() at line %d in %s\n", __LINE__, __FILE__);
			errorTeardown(NULL, malloced_head, the_debug);
			*col_numbers_arr = NULL;
			*or_head = NULL;
			*the_table = NULL;
			*col_numbers_size = 0;
			return -1;
		}
	}
	// END Get where clause

	myFree((void**) &word, NULL, malloced_head, the_debug);

	//errorTeardown(NULL, malloced_head, the_debug);

	return 0;
}


int main()
{
    /**/
    printf("\n");
	
    if (test_Driver_main() != 0)
    	printf("\nTests FAILED\n");
    else
    	printf("\nTests passed let's goooo\n");


    /*
	struct malloced_node* malloced_head = NULL;
	
    int debug = YES_DEBUG;
    printf("\n");

    
    int initd = initDB(&malloced_head, debug);
	if (initd == -1)
		printf("Database initialization had a problem with file i/o, please try again\n\n");
	else if (initd == -2)
		printf("Database initialization had a problem with malloc, please try again\n\n");
	else
		printf("Successfully initialized database\n\n");*/


	//traverseTablesInfoMemory();
	

    /*
    while (freeMemOfDB(debug) != 0)
        printf("Teardown FAILED\n");
    printf("Successfully teared down database\n");*/


	//traverseTablesInfoDisk(&malloced_head, debug);


	/*
	struct ListNode* head = NULL;
	struct ListNode* tail = NULL;

	addListNode(&malloced_head, &head, &tail, 10, 0, ADDLISTNODE_HEAD);
	addListNode(&malloced_head, &head, &tail, 11, 0, ADDLISTNODE_HEAD);
	addListNode(&malloced_head, &head, &tail, 12, 0, ADDLISTNODE_HEAD);
	addListNode(&malloced_head, &head, &tail, 13, 0, ADDLISTNODE_HEAD);
	addListNode(&malloced_head, &head, &tail, 14, 0, ADDLISTNODE_HEAD);

	printf("Removed %d from head\n", removeListNode(&malloced_head, &head, &tail, -1, TRAVERSELISTNODES_HEAD, debug));
	printf("Removed %d from head\n", removeListNode(&malloced_head, &head, &tail, -1, TRAVERSELISTNODES_HEAD, debug));

	traverseListNodes(&head, &tail, TRAVERSELISTNODES_TAIL, "List: ");*/


	/*
	char* word = malloc(sizeof(char) * 100);
	int cur_index = 0;

	while (getNextWord("WherE      braND-name    =    'test' \n    aNd     	 status = 'That' \n ;", word, &cur_index) == 0)
	{
		printf("word = _%s_\n", word);
	}


	int size_result = 0;
	char* str = (char*) myMalloc(sizeof(char) * 100, NULL, &malloced_head, debug);
	strcpy(str, "Hello,this,is,a,test");
	char** hmm = strSplitV2(str, ',', &size_result, NULL, &malloced_head, debug);

	if (hmm != NULL)
	{
		for (int i=0; i<size_result; i++)
		{
			printf("hmm[i] = _%s_\n", hmm[i]);
			myFree((void**) &hmm[i], NULL, &malloced_head, debug);
		}
		myFree((void**) &hmm, NULL, &malloced_head, debug);
	}

	myFree((void**) &str, NULL, &malloced_head, debug);*/

	return 0;
}