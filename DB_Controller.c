#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
//#include "DB_Driver.h"

typedef unsigned long int_8;

struct table_cols_info
{
	char col_name[32];
	int_8 data_type;
	int_8 max_length;
	int_8 col_number;
	int_8 num_rows;
	int_8 num_open;
	int_8 num_added_insert_open;
	struct table_cols_info* next;
};

int printCommands()
{
	printf("Create a table with:\n");
	printf("	create [table name] as ( [column name] as [datatype], .... ).\n");
	printf("Valid datatypes are \"integer\", \"real\", \"char([max length])\", and \"date\".\n");
	printf("[table name] and [column name] cannot be longer than 31 characters.\n");
	printf("\n");
}

int create_ParseStringAndExec(char* input)
{
	printf("input = %s\n", input);

	char table_name[32];
	int input_index = 7;
	int table_index = 0;
	while (input[input_index] != ' ' && table_index < 31)
	{
		table_name[table_index] = input[input_index];
		table_index++;
		input_index++;
	}
	table_name[table_index] = 0;

	printf("table_name = %s\n", table_name);

	int criteria = 0;
	while (criteria < 2 || input[input_index] == ' ')
	{
		if (input[input_index] == 'a' && input[input_index+1] == 's')
			criteria++;
		if (input[input_index] == '(')
			criteria++;

		input_index++;
	}

	struct table_cols_info* table_cols = malloc(sizeof(struct table_cols_info));

	struct table_cols_info* cur_col = table_cols;

	int phase = 0;
	int col_name_index = 0;
	char datatype[32];
	int datatype_index = 0;
	int_8 cur_col_number = 0;
	while (input[input_index] != ')')
	{
		if (input[input_index] == ' ')
			phase++;
		
		if (input[input_index] == ',')
		{
			phase++;
			cur_col->next = malloc(sizeof(struct table_cols_info));
		}

		if (phase % 4 == 3)
		{
			printf("datatype = %s\n", datatype);

			if (strcmp(datatype, "integer") == 0)
			{
				cur_col->data_type = 1;
				cur_col->max_length = 0;
			}
			else if (strcmp(datatype, "real") == 0)
			{
				cur_col->data_type = 2;
				cur_col->max_length = 0;
			}
			else if (strcmp(datatype, "date") == 0)
			{
				cur_col->data_type = 4;
				cur_col->max_length = 0;
			}
			else
			{
				cur_col->data_type = 3;
				sscanf(datatype, "char(%lu)", &cur_col->max_length);
			}

			cur_col->col_number = cur_col_number;

			printf("	Column name = %s\n", cur_col->col_name);
			printf("	Datatype = %lu\n", cur_col->data_type);
			printf("	Max length = %lu\n", cur_col->max_length);
			printf("	Column number = %lu\n", cur_col->col_number);
		}

		if (phase % 4 == 0)
		{
			if (col_name_index == -1)
				col_name_index = 0;

			cur_col->col_name[col_name_index] = input[input_index];
			col_name_index++;
		}
		else if (col_name_index != -1)
		{
			cur_col->col_name[col_name_index] = 0;
			col_name_index == -1;
		}

		if (phase % 4 == 2 && input[input_index] != ' ')
		{
			if (datatype_index == -1)
				datatype_index = 0;

			datatype[datatype_index] = input[input_index];
			datatype_index++;
		}
		else if (datatype_index != -1)
		{
			datatype[datatype_index] = 0;
			datatype_index = -1;
		}

		input_index++;
	}

	cur_col->next = NULL;

	//createTable(table_name, table_cols);

	return 0;
}

int main()
{
	//initDB();

	printCommands();

	char* input = malloc(sizeof(char) * 10000);
	while (1)
	{
		scanf("%[^\n]", input);

		for (int i=0; i<10000 && input[i] != 0; i++)
			input[i] = tolower(input[i]);

		char cmd[8];
		strncpy(cmd, input, 7);
		
		if (strcmp(cmd, "create"))
			create_ParseStringAndExec(input);
		else if (strcmp(cmd, "modify"))
		{
			
		}
		else if (strcmp(cmd, "select"))
		{
			
		}

		break;
	}

	/*
	char table_name[32] = "people\0";

	struct table_cols_info* table_cols = malloc(sizeof(struct table_cols_info));

	strcpy(table_cols->col_name, "id\0");
	table_cols->data_type = 1;
	table_cols->max_length = 8;
	table_cols->col_number = 0;

	table_cols->next = malloc(sizeof(struct table_cols_info));

	strcpy(table_cols->next->col_name, "name\0");
	table_cols->next->data_type = 3;
	table_cols->next->max_length = 31 + 1;
	table_cols->next->col_number = 1;

	table_cols->next->next = malloc(sizeof(struct table_cols_info));

	strcpy(table_cols->next->next->col_name, "date_of_birth\0");
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
		values (2, "Cathy", idk)

		insert into people (id, name, date_of_birth)
		values (2, "Cathy", idk)
	*/
	/*
	int_8 value = 1;
	addToChangeList(transac_id, 0, 0, 1, 1, &value, NULL, NULL);
	addToChangeList(transac_id, 0, 1, 1, 3, NULL, NULL, "Andrew");
	value = 35926;
	addToChangeList(transac_id, 0, 2, 1, 1, &value, NULL, NULL);

	transac_id++;

	value = 2;
	addToChangeList(transac_id, 0, 0, 1, 1, &value, NULL, NULL);
	addToChangeList(transac_id, 0, 1, 1, 3, NULL, NULL, "Cathy");
	value = 23246;
	addToChangeList(transac_id, 0, 2, 1, 1, &value, NULL, NULL);

	transac_id++;

	value = 3;
	addToChangeList(transac_id, 0, 0, 1, 1, &value, NULL, NULL);
	addToChangeList(transac_id, 0, 1, 1, 3, NULL, NULL, "Curt");
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
		values (4, "Judge", 06/25/2001)
	*/
	/*
	int_8 value = 4;
	addToChangeList(transac_id, 0, 0, 1, 1, &value, NULL, NULL);
	addToChangeList(transac_id, 0, 1, 1, 3, NULL, NULL, "Judge");
	value = 37066;
	addToChangeList(transac_id, 0, 2, 1, 1, &value, NULL, NULL);*/

	/*	update people
		set id = 5
		   ,name = "Matt"
		   ,date_of_birth = 09/24/2000
		where id = 2
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

	/*
  traverseTablesInfo();

	freeMemOfDB();

	traverseDB_Data_Files();*/
}
