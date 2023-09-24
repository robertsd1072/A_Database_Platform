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
	char** col_names = strSplit(first_row, ',', &num_cols, &file_opened_head, malloced_head, the_debug);
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

struct or_clause_node* parseWhereClause(char* input, struct table_info* the_table
									   ,struct malloced_node** malloced_head, int the_debug)
{
	//printf("-----------------------\n");
	struct table_info* cur_table = the_table;

	int index = 0;
	if (input[index] == 'w' && input[index+1] == 'h' && input[index+2] == 'e' && input[index+3] == 'r' && input[index+4] == 'e' && input[index+5] == ' ')
		index += 6;
	else
		return NULL;

	struct or_clause_node* or_head = (struct or_clause_node*) myMalloc(sizeof(struct or_clause_node), NULL, malloced_head, the_debug);
	if (or_head == NULL)
	{
		if (the_debug == YES_DEBUG)
			printf("	ERROR in parseWhereClause() at line %d in %s\n", __LINE__, __FILE__);
		return NULL;
	}
	or_head->next = NULL;

	or_head->and_head = (struct and_clause_node*) myMalloc(sizeof(struct and_clause_node), NULL, malloced_head, the_debug);
	or_head->and_head->col_number = -1;
	or_head->and_head->where_type = -1;
	or_head->and_head->data_string = NULL;
	or_head->and_head->next = NULL;

	struct or_clause_node* cur_or = or_head;
	struct and_clause_node* cur_and = or_head->and_head;

	char* word = (char*) myMalloc(sizeof(char) * 200, NULL, malloced_head, the_debug);
	if (word == NULL)
	{
		if (the_debug == YES_DEBUG)
			printf("	ERROR in parseWhereClause() at line %d in %s\n", __LINE__, __FILE__);
		return NULL;
	}
	word[0] = 0;
	int word_index = 0;

	while (input[index] != 0 && input[index] != ';')
	{
		//printf("%c and %d\n", input[index], input[index]);
		if (input[index] == 39)
		{
			for (int i=0; i<2; i++)
			{
				word[word_index] = input[index];
				word[word_index+1] = 0;

				word_index++;
				index++;
			}

			bool two_single_quotes = false;
			while (two_single_quotes || !(input[index] == 0 || (input[index-1] == 39 && input[index] != 39) || input[index] == ';'))
			{
				if (two_single_quotes)
					two_single_quotes = false;
				if (input[index-1] == 39 && input[index] == 39)
				{
					index++;
					two_single_quotes = true;
				}
				else
				{
					word[word_index] = input[index];
					word[word_index+1] = 0;

					word_index++;
					index++;
				}
			}

			if (input[index] == 0)
			{
				printf("hmm\n");
				//myFreeAllError(&malloced_head, the_debug);
				//return NULL;
			}
		}
		else
		{
			while (input[index] != 0 && input[index] != ' ' && input[index] != ';')
			{
				word[word_index] = input[index];
				word[word_index+1] = 0;

				word_index++;
				index++;
			}

			if (input[index] == 0)
			{
				errorTeardown(NULL, malloced_head, the_debug);
				return NULL;
			}
		}

		if (word[0] != 0)
		{
			//printf("word: %s\n", word);
			if (word[0] == 'o' && word[1] == 'r')
			{
				if (cur_and->col_number == -1 || cur_and->where_type == -1 || cur_and->data_string == NULL)
				{
					if (the_debug == YES_DEBUG)
						printf("	ERROR in parseWhereClause() at line %d in %s\n", __LINE__, __FILE__);
					errorTeardown(NULL, malloced_head, the_debug);
					return NULL;
				}

				//printf("Option 1\n");
				cur_or->next = (struct or_clause_node*) myMalloc(sizeof(struct or_clause_node), NULL, malloced_head, the_debug);
				if (cur_or->next == NULL)
				{
					if (the_debug == YES_DEBUG)
						printf("	ERROR in parseWhereClause() at line %d in %s\n", __LINE__, __FILE__);
					return NULL;
				}
				cur_or->next->next = NULL;

				cur_or->next->and_head = (struct and_clause_node*) myMalloc(sizeof(struct and_clause_node), NULL, malloced_head, the_debug);
				cur_or->next->and_head->col_number = -1;
				cur_or->next->and_head->where_type = -1;
				cur_or->next->and_head->data_string = NULL;
				cur_or->next->and_head->next = NULL;

				cur_or = cur_or->next;
				cur_and = cur_or->and_head;
			}
			else if (word[0] == 'a' && word[1] == 'n' && word[2] == 'd')
			{
				if (cur_and->col_number == -1 || cur_and->where_type == -1 || cur_and->data_string == NULL)
				{
					if (the_debug == YES_DEBUG)
						printf("	ERROR in parseWhereClause() at line %d in %s\n", __LINE__, __FILE__);
					errorTeardown(NULL, malloced_head, the_debug);
					return NULL;
				}

				//printf("Option 2\n");
				cur_and->next = (struct and_clause_node*) myMalloc(sizeof(struct and_clause_node), NULL, malloced_head, the_debug);
				if (cur_and->next == NULL)
				{
					if (the_debug == YES_DEBUG)
						printf("	ERROR in parseWhereClause() at line %d in %s\n", __LINE__, __FILE__);
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
					return NULL;
				}

				//printf("Option 3\n");
				cur_and->where_type = (word[0] == '=' ? WHERE_IS_EQUALS : WHERE_NOT_EQUALS);
			}
			else if (word[0] == 39)
			{
				if (cur_and->data_string != NULL)
				{
					if (the_debug == YES_DEBUG)
						printf("	ERROR in parseWhereClause() at line %d in %s\n", __LINE__, __FILE__);
					errorTeardown(NULL, malloced_head, the_debug);
					return NULL;
				}

				//printf("Option 4\n");
				cur_and->data_string = substring(word, 1, strLength(word)-2, NULL, malloced_head, the_debug);
				if (cur_and->data_string == NULL)
				{
					if (the_debug == YES_DEBUG)
						printf("	ERROR in parseWhereClause() at line %d in %s\n", __LINE__, __FILE__);
					return NULL;
				}
			}
			else
			{
				struct table_cols_info* cur_col = cur_table->table_cols_head;
				while (cur_col != NULL)
				{
					if (strcmp(word, cur_col->col_name) == 0)
					{
						if (cur_and->col_number != -1)
						{
							if (the_debug == YES_DEBUG)
								printf("	ERROR in parseWhereClause() at line %d in %s\n", __LINE__, __FILE__);
							errorTeardown(NULL, malloced_head, the_debug);
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
					if (cur_and->data_string != NULL)
					{
						if (the_debug == YES_DEBUG)
							printf("	ERROR in parseWhereClause() at line %d in %s\n", __LINE__, __FILE__);
						errorTeardown(NULL, malloced_head, the_debug);
						return NULL;
					}

					//printf("Option 5a\n");
					cur_and->data_string = (char*) myMalloc(sizeof(char) * 64, NULL, malloced_head, the_debug);
					if (cur_and->data_string == NULL)
					{
						if (the_debug == YES_DEBUG)
							printf("	ERROR in parseWhereClause() at line %d in %s\n", __LINE__, __FILE__);
						return NULL;
					}
					strcpy(cur_and->data_string, word);
				}
			}
		}
		word[0] = 0;
		word_index = 0;

		index++;
	}
	myFree((void**) &word, NULL, malloced_head, the_debug);

	if (or_head->and_head->col_number == -1 || or_head->and_head->where_type == -1 || or_head->and_head->data_string == NULL)
	{
		if (the_debug == YES_DEBUG)
			printf("	ERROR in parseWhereClause() at line %d in %s\n", __LINE__, __FILE__);
		errorTeardown(NULL, malloced_head, the_debug);
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

	return or_head;
}

int parseUpdate(char* input, struct change_node_v2** change_head, struct or_clause_node** or_head
			   ,struct malloced_node** malloced_head, int the_debug)
{
	// START Get table name and find table node in list
	int index = 0;

	char* table_name = (char*) myMalloc(sizeof(char) * 32, NULL, malloced_head, the_debug);
	if (table_name == NULL)
	{
		if (the_debug == YES_DEBUG)
			printf("	ERROR in parseUpdate() at line %d in %s\n", __LINE__, __FILE__);
		return -1;
	}
	sscanf(input, "%*[^ ] %s %*[^;];", table_name);

	struct table_info* cur_table = getTablesHead();

	while (cur_table != NULL)
	{
		if (strcmp(cur_table->name, table_name) == 0)
			break;
		cur_table = cur_table->next;
	}
	if (myFree((void**) &table_name, NULL, malloced_head, the_debug) != 0)
	{
		if (the_debug == YES_DEBUG)
			printf("	ERROR in parseUpdate() at line %d in %s\n", __LINE__, __FILE__);
		return -1;
	}

	if (cur_table == NULL)
	{
		if (the_debug == YES_DEBUG)
			printf("	ERROR in parseUpdate() at line %d in %s\n", __LINE__, __FILE__);
		errorTeardown(NULL, malloced_head, the_debug);
		return -1;
	}
	// END Get table name and find table node in list


	// START Get all the set = 'value'
	//	START Skip the "set "
	index = 0;
	while (!(input[index] == 0 || input[index] == 's' && input[index+1] == 'e' && input[index+2] == 't'))
		index++;

	index += 4;
	//	END Skip the "set "

	// 	START Alloc memory to the change_head
	*change_head = (struct change_node_v2*) myMalloc(sizeof(struct change_node_v2), NULL, malloced_head, the_debug);
	if ((*change_head)  == NULL)
	{
		if (the_debug == YES_DEBUG)
			printf("	ERROR in parseUpdate() at line %d in %s\n", __LINE__, __FILE__);
		return -1;
	}
	(*change_head)->col_number = -1;
	(*change_head)->operation = -1;
	(*change_head)->data_type = -1;
	(*change_head)->data = NULL;
	(*change_head)->next = NULL;
	// 	END Alloc memory to the change_head

	struct change_node_v2* cur_change = *change_head;

	char* word = (char*) myMalloc(sizeof(char) * 200, NULL, malloced_head, the_debug);
	if (word == NULL)
	{
		if (the_debug == YES_DEBUG)
			printf("	ERROR in parseUpdate() at line %d in %s\n", __LINE__, __FILE__);
		return -1;
	}
	word[0] = 0;
	int word_index = 0;
	
	while (!(input[index] == 0 || (input[index] == 'w' && input[index+1] == 'h' && input[index+2] == 'e' && input[index+3] == 'r' && input[index+4] == 'e')))
	{
		//printf("%c and %d\n", input[index], input[index]);
		if (input[index] == 39)
		{
			for (int i=0; i<2; i++)
			{
				word[word_index] = input[index];
				word[word_index+1] = 0;

				word_index++;
				index++;
			}

			bool two_single_quotes = false;
			while (two_single_quotes || !(input[index] == 0 || input[index] == ',' || (input[index-1] == 39 && input[index] != 39) || input[index] == ';'))
			{
				if (two_single_quotes)
					two_single_quotes = false;
				if (input[index-1] == 39 && input[index] == 39)
				{
					index++;
					two_single_quotes = true;
				}
				else
				{
					word[word_index] = input[index];
					word[word_index+1] = 0;

					word_index++;
					index++;
				}
			}

			if (input[index] == 0)
			{
				printf("hmm2\n");
				//myFreeAllError(&malloced_head, the_debug);
				//return NULL;
			}
		}
		else
		{
			while (input[index] != 0 && input[index] != ' ' && input[index] != ',' && input[index] != ';')
			{
				word[word_index] = input[index];
				word[word_index+1] = 0;

				word_index++;
				index++;
			}

			if (input[index] == 0)
			{
				errorTeardown(NULL, malloced_head, the_debug);
				return -1;
			}
		}

		if (word[0] != 0)
		{
			//printf("word: %s\n", word);
			if (word[0] == '=' && word[1] == 0)
			{
				if (cur_change->operation != -1)
				{
					if (the_debug == YES_DEBUG)
						printf("	ERROR in parseUpdate() at line %d in %s\n", __LINE__, __FILE__);
					errorTeardown(NULL, malloced_head, the_debug);
					return -1;
				}

				cur_change->operation = 3;
			}
			else if (word[0] == 39)
			{
				if (cur_change->data != NULL)
				{
					if (the_debug == YES_DEBUG)
						printf("	ERROR in parseUpdate() at line %d in %s\n", __LINE__, __FILE__);
					errorTeardown(NULL, malloced_head, the_debug);
					return -1;
				}

				cur_change->data = substring(word, 1, strLength(word)-2, NULL, malloced_head, the_debug);
			}
			else
			{
				struct table_cols_info* cur_col = cur_table->table_cols_head;
				while (cur_col != NULL)
				{
					if (strcmp(word, cur_col->col_name) == 0)
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
							return -1;
						}

						break;
					}

					cur_col = cur_col->next;
				}

				if (cur_col == NULL)
				{
					if (cur_change->data != NULL)
					{
						if (the_debug == YES_DEBUG)
							printf("	ERROR in parseUpdate() at line %d in %s\n", __LINE__, __FILE__);
						errorTeardown(NULL, malloced_head, the_debug);
						return -1;
					}

					//printf("Option 5b\n");
					cur_change->data = (char*) myMalloc(sizeof(char) * 64, NULL, malloced_head, the_debug);
					if (cur_change->data == NULL)
					{
						if (the_debug == YES_DEBUG)
							printf("	ERROR in parseUpdate() at line %d in %s\n", __LINE__, __FILE__);
						errorTeardown(NULL, malloced_head, the_debug);
						return -1;
					}
					strcpy(cur_change->data, word);
				}
			}

			if (input[index] == ',')
			{
				//printf("Option new\n");
				cur_change->next = (struct change_node_v2*) myMalloc(sizeof(struct change_node_v2), NULL, malloced_head, the_debug);
				if (cur_change->next == NULL)
				{
					if (the_debug == YES_DEBUG)
						printf("	ERROR in parseUpdate() at line %d in %s\n", __LINE__, __FILE__);
					errorTeardown(NULL, malloced_head, the_debug);
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
					return -1;
				}

				cur_change = cur_change->next;
			}
		}
		word[0] = 0;
		word_index = 0;

		index++;
	}
	myFree((void**) &word, NULL, malloced_head, the_debug);
	// END Get all the set = 'value'


	// START Find and parse where clause
	//printf("index = %d\n", index);
	if (input[index] == 0)
	{
		if (the_debug == YES_DEBUG)
			printf("	ERROR in parseUpdate() at line %d in %s\n", __LINE__, __FILE__);
		errorTeardown(NULL, malloced_head, the_debug);
		return -1;
	}

	char* where_clause = substring(input, index, strLength(input)-1, NULL, malloced_head, the_debug);
	if (where_clause == NULL)
	{
		if (the_debug == YES_DEBUG)
			printf("	ERROR in parseUpdate() at line %d in %s\n", __LINE__, __FILE__);
		return -1;
	}
	//printf("where_clause = _%s_\n", where_clause);

	if (or_head != NULL)
		*or_head = parseWhereClause(where_clause, cur_table, malloced_head, the_debug);

	if (myFree((void**) &where_clause, NULL, malloced_head, the_debug) != 0)
	{
		if (the_debug == YES_DEBUG)
			printf("	ERROR in parseUpdate() at line %d in %s\n", __LINE__, __FILE__);
		return -1;
	}

	if (or_head != NULL && *or_head == NULL)
	{
		if (the_debug == YES_DEBUG)
			printf("	ERROR in parseUpdate() at line %d in %s\n", __LINE__, __FILE__);
		errorTeardown(NULL, malloced_head, the_debug);
		return -1;
	}
	// END Find and parse where clause

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

int main()
{
    /**/
    int debug = YES_DEBUG;
    printf("\n");
	
    if (test_Driver_main(debug) != 0)
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
}