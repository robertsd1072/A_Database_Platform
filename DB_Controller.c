#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stddef.h>
#include <inttypes.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include "DB_Driver.h"

void printCommands()
{
	printf("Create a table with:\n");
	printf("	create table [table name] as ([column name] as [datatype], .... );\n");
	printf("Both [table name] and [column name] cannot be more then 31 characters in length.\n");
	printf("Valid datatypes are \"integer\", \"real\", \"char([max length])\", and \"date\".\n");
	printf("\nInsert into a table with:\n");
	printf("	insert into [table name] ([column name], [column name], ...) values ([data], [data], ...);\n");
	printf("\nDelete from a table with:\n");
	printf("	delete from [table name] where [column name] = [data];\n");
	printf("\nUpdate a table with:\n");
	printf("	update [table name] set [column name] = [data], [column name] = [data], ... where [column name] = [data];\n");
	printf("\nSelect from a table with:\n");
	printf("	select */[column name], [column name], ... from [table name] where [column name] = [data];\n");
}

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
        {
            return 1;
        }
	    index++;
	}
	
	return 0;
}

void displayResultsAndFree(char*** result, int_8 table_number, int_8* col_numbers, int col_numbers_size, int_8 num_rows, struct or_clause_node* or_head)
{
	while (or_head != NULL)
	{
		while (or_head->and_head != NULL)
		{
			struct and_clause_node* temp = or_head->and_head;
			or_head->and_head = or_head->and_head->next;
			if (temp->data_string != NULL)
				free(temp->data_string);
			free(temp);
		}
		struct or_clause_node* temp = or_head;
		or_head= or_head->next;
		free(temp);
	}

	struct table_info* cur_table = getTablesHead();
	while (cur_table != NULL)
	{
		if (cur_table->file_number == table_number)
			break;
		cur_table = cur_table->next;
	}

	char** col_names = (char**) malloc(sizeof(char*) * col_numbers_size);
	int* max_length_arr = (int*) malloc(sizeof(int) * col_numbers_size);
	for (int j=0; j<col_numbers_size; j++)
	{
		int max_length = 0;
		struct table_cols_info* cur_col = cur_table->table_cols_head;
		while (cur_col != NULL)
		{
			if (cur_col->col_number == col_numbers[j])
				break;
			cur_col = cur_col->next;
		}
		max_length = strLength(cur_col->col_name);

		col_names[j] = (char*) malloc(sizeof(char) * 32);
		strcpy(col_names[j], cur_col->col_name);

		for (int i=0; i<num_rows; i++)
		{
			int cur_length = strLength(result[i][j]);
			if (cur_length > max_length)
				max_length = cur_length;
		}
		max_length_arr[j] = max_length;
	}
	free(col_numbers);

	int** add_length_arr = (int**) malloc(sizeof(int*) * num_rows);
	for (int i=0; i<num_rows+1; i++)
	{
		add_length_arr[i] = (int*) malloc(sizeof(int) * col_numbers_size);
		for (int j=0; j<col_numbers_size; j++)
		{
			if (i == 0)
				add_length_arr[i][j] = max_length_arr[j] - strLength(col_names[j]);
			else
				add_length_arr[i][j] = max_length_arr[j] - strLength(result[i-1][j]);
		}
	}
	free(max_length_arr);

	printf("\n");
	char* hori_line = (char*) malloc(sizeof(char) * 1000);
	strcpy(hori_line, "-\0");
	for (int j=0; j<col_numbers_size; j++)
	{
		int left_to_add = 0;
		while (left_to_add <  strLength(col_names[j])+add_length_arr[0][j])
		{
			strcat(hori_line, "-");
			left_to_add++;
		}
		strcat(hori_line, "-");
	}
	printf("%s\n", hori_line);
	for (int j=0; j<col_numbers_size; j++)
	{
		printf("|%s", col_names[j]);
		int left_to_add = 0;
		while (left_to_add < add_length_arr[0][j])
		{
			printf(" ");
			left_to_add++;
		}
		if (j == col_numbers_size-1)
			printf("|");
		free(col_names[j]);
	}
	printf("\n");
	printf("%s\n", hori_line);
	free(col_names);
	for (int i=0; i<num_rows; i++)
	{
		for (int j=0; j<col_numbers_size; j++)
		{
			printf("|%s", result[i][j]);
			int left_to_add = 0;
			while (left_to_add < add_length_arr[i+1][j])
			{
				printf(" ");
				left_to_add++;
			}
			if (j == col_numbers_size-1)
				printf("|");
			free(result[i][j]);
		}
		printf("\n");
		free(result[i]);
		free(add_length_arr[i+1]);
	}
	free(result);
	free(add_length_arr);

	printf("%s\n\n", hori_line);
	free(hori_line);
}

int select_ParseStringAndExec(char* input)
{
    int index=0;
    while (input[index] != 'f' || input[index+1] != 'r' || input[index+2] != 'o' || input[index+3] != 'm')
        index++;
    index += 4;
    while (input[index] == ' ' || input[index] == '\t' || input[index] == '\n')
        index++;
        
    int new_index = index;
    while (input[new_index] != ' ' && input[new_index] != '\t' && input[new_index] != '\n' && input[new_index] != ';' && input[new_index] != ',')
        new_index++;
    
    char* table_name = (char*) malloc(sizeof(char) * (new_index-index)+1);
    
    //printf("index = %d and new_index = %d\n", index, new_index);
	
	strncpy(table_name, input+index, new_index-index);
    
    table_name[new_index-index] = 0;
    
    //printf("table_name = _%s_\n", table_name);
    
    if (strcmp(table_name, "") == 0)
	{
		free(table_name);
		return 1;
	}
	    
    struct table_info* cur_table = getTablesHead();
	while (cur_table != NULL)
	{
		//printf("%s vs %s\n", cur_table->name, table_name);
		if (strcmp(cur_table->name, table_name) == 0)
			break;
		cur_table = cur_table->next;
	}

	if (cur_table == NULL)
	{
		free(table_name);
		return 2;
	}

	int_8 table_number = cur_table->file_number;

	int col_numbers_size;
	int_8* col_numbers = NULL;
    
    char col_names[1000];
    
    sscanf(input, "select%s%*[^\n]", col_names);
    
    //printf("col_names = _%s_\n", col_names);
    
    if (strcmp(col_names, "*") == 0)
    {
        col_numbers_size = cur_table->num_cols;
		col_numbers = (int_8*) malloc(sizeof(int_8) * col_numbers_size);
		
		struct table_cols_info* cur_col = cur_table->table_cols_head;
		int index = 0;
		while (cur_col != NULL)
		{
			col_numbers[index] = cur_col->col_number;
			index++;
			cur_col = cur_col->next;
		}
    }
    else
    {
        int number_of_cols = 1;
        int index = 0;
    	while (input[index] != 0)
    	{
    	    if (input[index] == 'f' && input[index+1] == 'r' && input[index+2] == 'o' && input[index+3] == 'm')
    	        break;
    	    if (input[index] == ',')
                number_of_cols++;
    	    index++;
    	}

		if (number_of_cols == 0)
		{
			free(table_name);
			return 3;
		}
    	
    	//printf("number_of_cols = %d\n", number_of_cols);

		col_numbers_size = number_of_cols;
    	
    	col_numbers = (int_8*) malloc(sizeof(int_8) * number_of_cols);
        
        char** col_names = (char**) malloc(sizeof(char*) * number_of_cols);
        
        for (int i=0; i<number_of_cols; i++)
        {
            char* format = (char*) malloc(sizeof(char) * 1000);
			strcpy(format, "select \0");
	    
    	    for (int j=0; j<i; j++)
                strcat(format, "%*[^,] %*[,]");
    	    
    	    if (i < number_of_cols-1)
    	        strcat(format, " %19[^, ] %*[,]");
    	    else
    	        strcat(format, " %19[^ ] %*[^\\n]");
    	        
            strcat(format, " %*[^\\n]");
            
            //printf("format = _%s_\n", format);
            
            col_names[i] = (char*) malloc(sizeof(char) * 20);
        
            sscanf(input, format, col_names[i]);
			free(format);

			//printf("col_names[%d] = %s\n", i, col_names[i]);
            
            struct table_cols_info* cur_col = cur_table->table_cols_head;
			while (cur_col != NULL)
			{
				if (strcmp(cur_col->col_name, col_names[i]) == 0)
				{
					col_numbers[i] = cur_col->col_number;
					break;
				}
				cur_col = cur_col->next;
			}
            
            free(col_names[i]);

			if (cur_col == NULL)
			{
				for (int ii=0; ii<number_of_cols; ii++)
				{
					if (col_names[ii] != NULL)
						free(col_names[ii]);
				}
				free(col_names);
				free(table_name);
				free(col_numbers);
				return 4;
			}
        }
        
        free(col_names);
    }

	int_8 num_rows = 0;

	char*** result = select(table_number, col_numbers, col_numbers_size, &num_rows, NULL);
    
	displayResultsAndFree(result, table_number, col_numbers, col_numbers_size, num_rows, NULL);

    return 0;
}

int create_ParseStringAndExec(char* input)
{
	//printf("input = %s\n", input);
	
	char table_name[32];
	
	sscanf(input, "create%*[^t]table%s%*[^\n]", table_name);
	
	//printf("table_name = _%s_\n", table_name);
	
	if (strcmp(table_name, "") == 0)
	    return 0;
	
    int number_of_cols = 1;
    int index = 0;
	while (input[index] != 0)
	{
	    if (input[index] == ',')
            number_of_cols++;
	    index++;
	}
	
	struct table_cols_info* table_cols = (struct table_cols_info*) malloc(sizeof(struct table_cols_info));
	struct table_cols_info* cur_col = table_cols;
	
	char** col_data_types = (char**) malloc(sizeof(char*) * number_of_cols);
	
	for (int i=0; i<number_of_cols; i++)
	{
	    char format[1000] = "create%*[^(](";
	    
	    for (int j=0; j<i; j++)
            strcat(format, "%*[^,] %*[,]");
	    
	    strcat(format, "%s%*[^a]as");
	    if (i < number_of_cols-1)
	        strcat(format, " %19[^, ] %*[,]");
	    else
	        strcat(format, " %19[^);] %*[^\\n]");
	        
        strcat(format, " %*[^\\n]");
        
        //printf("format = _%s_\n", format);
        
        col_data_types[i] = (char*) malloc(sizeof(char) * 20);
        
        sscanf(input, format, cur_col->col_name, col_data_types[i]);
        
        sscanf(cur_col->col_name, "\n%s\n", cur_col->col_name);
	    sscanf(col_data_types[i], "\n%s\n", col_data_types[i]);
	    
	    //printf("cur_col->col_name = %s\n", cur_col->col_name);
	    //printf("col_data_types[%d] = %s\n", i, col_data_types[i]);
	    
	    char datatype[5];
	     
	    datatype[4] = 0;
	    
	    //printf("datatype = %s\n", datatype);
	    
	    if (strcmp(datatype, "char") == 0)
	    {
	        cur_col->data_type = 3;
	        sscanf(col_data_types[i], "char(%lu)", &cur_col->max_length);
	    }
	    else if (strcmp(datatype, "inte") == 0)
	    {
	        cur_col->data_type = 1;
	        cur_col->max_length = 8;
	    }
	    else if (strcmp(datatype, "real") == 0)
	    {
	        cur_col->data_type = 2;
	        cur_col->max_length = 8;
	    }
	    else if (strcmp(datatype, "date") == 0)
	    {
	        cur_col->data_type = 4;
	        cur_col->max_length = 8;
	    }
	    
	    cur_col->col_number = i;
	    
	    if (strcmp(cur_col->col_name, "") == 0)
	        return 0;
	    if (strcontains(cur_col->col_name, ','))
	        return 0;
	    if (cur_col->data_type != 1 && cur_col->data_type != 2 && cur_col->data_type != 3 && cur_col->data_type != 4)
	        return 0;
	    if (cur_col->data_type == 3 && cur_col->max_length == 0)
	        return 0;
	    
	    //printf("	Column name = %s\n", cur_col->col_name);
		//printf("	Datatype = %lu\n", cur_col->data_type);
		//printf("	Max length = %lu\n", cur_col->max_length);
		//printf("	Column number = %lu\n", cur_col->col_number);
	    
	    if (i < number_of_cols-1)
	    {
	        cur_col->next = (struct table_cols_info*) malloc(sizeof(struct table_cols_info));
	        cur_col = cur_col->next;
	    }
	    else
	        cur_col->next = NULL;
	}
	
	for (int i=0; i<number_of_cols; i++)
	    free(col_data_types[i]);
	
	free(col_data_types);
	
	//createTable(table_name, table_cols);
	
	return 1;
}

int main()
{
	initDB();

	/**/
	printCommands();

	char* input = malloc(sizeof(char) * 100000);
	while (1)
	{
		strcpy(input, "");
		printf("\n");
		while (1)
		{
		    char temp_input[1000];
    	    fgets(temp_input, 999, stdin);
    		
    		//printf("temp_input = _%s_\n", temp_input);
    		
    		strcat(input, temp_input);
    		
    		//printf("input = _%s_\n", input);
    		
    		if (strcontains(temp_input, ';') == 1)
    		    break;
		}
		
		for (int i=0; i<100000 && input[i] != 0; i++)
			input[i] = tolower(input[i]);
		
		//printf("\n");	
		//printf("Input = _%s_\n", input);
		
		char cmd[5];
		strncpy(cmd, input, 4);
		cmd[4] = 0;
		//printf("cmd = _%s_\n", cmd);
		
		if (strcmp(cmd, "crea") == 0)
		{
		    int returnd = create_ParseStringAndExec(input);
		    if (returnd == 1)
		        printf("\nTable created.\n");
		    else
		        printf("\nThe command was not recognized, please try again.\n");
		}
			
		else if (strcmp(cmd, "modi") == 0)
		{
			
		}
		else if (strcmp(cmd, "sele") == 0)
		{
			int result = select_ParseStringAndExec(input);
			if (result == 1)
				printf("\nA table was not specified, please try again.\n");
			else if (result == 2)
				printf("\nA table could not be found with that name, please try again.\n");
			else if (result == 3)
				printf("\nNo columns were specified, please try again.\n");
			else if (result == 4)
				printf("\nThere was an error finding a column, please try again.\n");
		}
		else if (strcmp(cmd, "inse") == 0)
		{
		    
		}
		else if (strcmp(cmd, "upda") == 0)
		{
		    
		}
		else if (strcmp(cmd, "dele") == 0)
		{
		    
		}
		else if (strcmp(cmd, "stop") == 0)
		    break;
	    else
	        printf("\nThe command was not recognized, please try again.\n");
	}
	free(input);
	

	/*
	char* table_name = (char*) malloc(sizeof(char) * 32);
	strcpy(table_name, "people");

	struct table_cols_info* table_cols = malloc(sizeof(struct table_cols_info));
	
	table_cols->col_name = (char*) malloc(sizeof(char) * 32);
	strcpy(table_cols->col_name, "id");
	table_cols->data_type = 1;
	table_cols->max_length = 8;
	table_cols->col_number = 0;

	table_cols->next = malloc(sizeof(struct table_cols_info));

	table_cols->next->col_name = (char*) malloc(sizeof(char) * 32);
	strcpy(table_cols->next->col_name, "name");
	table_cols->next->data_type = 3;
	table_cols->next->max_length = 31 + 1;
	table_cols->next->col_number = 1;

	table_cols->next->next = malloc(sizeof(struct table_cols_info));

	table_cols->next->next->col_name = (char*) malloc(sizeof(char) * 32);
	strcpy(table_cols->next->next->col_name, "date_of_birth");
	table_cols->next->next->data_type = 4;
	table_cols->next->next->max_length = 8;
	table_cols->next->next->col_number = 2;

	table_cols->next->next->next = NULL;

	createTable(table_name, table_cols);*/

	//traverseTablesInfo();

	//traverseDB_Data_Files();

	int_8 transac_id = 0;

	/*	insert into people (id, name, date_of_birth)
		values (1, "Andrew", 05/12/1998)

		insert into people (id, name, date_of_birth)
		values (2, "Cathy", idk);

		insert into people (id, name, date_of_birth)
		values (2, "Cathy", idk);
	*/
	/*
	int_8 value = 1;
	addToChangeList(transac_id, 0, 0, 1, 1, &value, NULL, NULL);
	char* name_value_a = (char*) malloc(sizeof(char) * 32);
	strcpy(name_value_a, "Andrew");
	addToChangeList(transac_id, 0, 1, 1, 3, NULL, NULL, name_value_a);
	value = 35926;
	addToChangeList(transac_id, 0, 2, 1, 1, &value, NULL, NULL);

	transac_id++;

	value = 2;
	addToChangeList(transac_id, 0, 0, 1, 1, &value, NULL, NULL);
	char* name_value_b = (char*) malloc(sizeof(char) * 32);
	strcpy(name_value_b, "Cathy");
	addToChangeList(transac_id, 0, 1, 1, 3, NULL, NULL, name_value_b);
	value = 23246;
	addToChangeList(transac_id, 0, 2, 1, 1, &value, NULL, NULL);

	transac_id++;

	value = 3;
	addToChangeList(transac_id, 0, 0, 1, 1, &value, NULL, NULL);
	char* name_value_c = (char*) malloc(sizeof(char) * 32);
	strcpy(name_value_c, "Curt");
	addToChangeList(transac_id, 0, 1, 1, 3, NULL, NULL, name_value_c);
	value = 21870;
	addToChangeList(transac_id, 0, 2, 1, 1, &value, NULL, NULL);*/

	/*	delete from people
		where id = 1
			or id = 3
	*/
	/*
	transac_id++;

	int_8 value = 1;
	addToChangeList(transac_id, 0, 0, 3, 1, &value, NULL, NULL);
	value = 3;
	addToChangeList(transac_id, 0, 0, 3, 1, &value, NULL, NULL);*/

	/*	insert into people (id, name, date_of_birth)
		values (4, "Judge", 06/25/2001);
	*/
	/*
	transac_id++;

	int_8 value = 4;
	addToChangeList(transac_id, 0, 0, 1, 1, &value, NULL, NULL);
	char* name_value_d = (char*) malloc(sizeof(char) * 32);
	strcpy(name_value_d, "Judge");
	addToChangeList(transac_id, 0, 1, 1, 3, NULL, NULL, name_value_d);
	value = 37066;
	addToChangeList(transac_id, 0, 2, 1, 1, &value, NULL, NULL);*/

	/*	update people
		set id = 5
		   ,name = "Matt"
		   ,date_of_birth = 09/24/2000
		where id = 2;
		-- a delete for every "or" in the where clause
		-- for an "and" intput more values in one call of addToChangeList
	*/
	/*
	// Delete (4 for updates) first
	int_8 value = 2;
	addToChangeList(0, 0, 4, 1, &value, NULL, NULL);
	// Then update
	value = 5;
	addToChangeList(0, 0, 2, 1, &value, NULL, NULL);
	addToChangeList(0, 1, 2, 3, NULL, NULL, "Matt");
	value = 36792;
	addToChangeList(0, 2, 2, 1, &value, NULL, NULL);*/

	//traverseTablesInfo();

	//traverseDB_Data_Files();

	/*	select *
		from people
		where id <> 4;
	*/
	/*
	int col_numbers_size = 3;
	int_8* col_numbers = malloc(sizeof(int_8) * col_numbers_size);
	col_numbers[0] = 0;
	col_numbers[1] = 1;
	col_numbers[2] = 2;

	struct or_clause_node* or_head = malloc(sizeof(struct or_clause_node));
	or_head->next = NULL;

	or_head->and_head = malloc(sizeof(struct and_clause_node));
	or_head->and_head->next = NULL;
	or_head->and_head->col_number = 0;
	or_head->and_head->data_type = 1;
	or_head->and_head->where_type = 2;
	or_head->and_head->data_int_date = 4;
	or_head->and_head->data_real = 0;
	or_head->and_head->data_string = NULL;

	int_8 table_number = 0;
	int_8 num_rows = 0;
	char*** result = select(table_number, col_numbers, col_numbers_size, &num_rows, or_head);
	
	displayResultsAndFree(result, table_number, col_numbers, col_numbers_size, num_rows, or_head);*/

	//traverseTablesInfo();
	
	freeMemOfDB();

	//traverseDB_Data_Files_v2();

	/*char* the_int = (char*) malloc(sizeof(char) * 32);
	strcpy(the_int, "37066\0");
	char* result = intToDate(the_int);
	printf("37066 in date form is %s\n", result);
	free(the_int);
	free(result);

	char* the_date = (char*) malloc(sizeof(char) * 32);
	strcpy(the_date, "6/25/2001\0");
	printf("6/25/2001 in int_8 form is %lu\n", dateToInt(the_date));
	free(the_date);*/
}