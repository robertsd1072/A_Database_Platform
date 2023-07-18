#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stddef.h>
#include <inttypes.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>

#include "DB_Driver.h"

typedef unsigned long long int_8;
static char open_string[3] = "~~\0";
static int_8 open_int = -1;
static double open_double = -1.0;

static int_8 num_tables;
static struct table_info* tables_head;

struct ListNode
{
	int_8 value;
	struct ListNode* next;
};

struct table_info* getTablesHead() { return tables_head; }

FILE* openFile(char* filetype, int_8 num_table, int_8 num_col, char* mode)
{
	char* file_name = (char*) malloc(sizeof(char) * 32);
	concatFileName(file_name, filetype, num_table, num_col);

	FILE* file = fopen(file_name, mode);
	free(file_name);

	return file;
}

void initDB()
{
	int files_left_to_close = 0;

	FILE* db_info = fopen(".\\DB_Files_2\\DB_Info.bin", "rb+");
	files_left_to_close++;

	if (db_info == NULL)
	{
		files_left_to_close--;
		db_info = fopen(".\\DB_Files_2\\DB_Info.bin", "ab+");
		files_left_to_close++;

		num_tables = 0;

		fwrite(&num_tables, 8, 1, db_info);
		fflush(db_info);
	}
	else
		num_tables = readFileInt(db_info, 0);

	if (num_tables > 0)
	{
		tables_head = (struct table_info*) malloc(sizeof(struct table_info));
		struct table_info* cur = tables_head;

		int cur_alloc = 0;
		int_8 offset = 8;
		while (cur_alloc < num_tables)
		{
			cur->name = readFileChar(db_info, offset);
			offset += 32;
			cur->file_number = readFileInt(db_info, offset);
			offset += 8;

			/*char* tab_col_file_name = (char*) malloc(sizeof(char) * 48);
			concatFileName(tab_col_file_name, "_Tab_Col_", cur_alloc, 0);

			FILE* tab_col = fopen(tab_col_file_name, "rb+");
			free(tab_col_file_name);*/
			FILE* tab_col = openFile("_Tab_Col_", cur_alloc, 0, "rb+\0");
			files_left_to_close++;

			int_8 col_offset = 0;

			cur->num_cols = readFileInt(tab_col, col_offset);
			
			cur->table_cols_head = malloc(sizeof(struct table_cols_info));

			struct table_cols_info* cur_col = cur->table_cols_head;

			int cur_col_alloc = 0;
			col_offset += 8;

			while (cur_col_alloc < cur->num_cols)
			{
				cur_col->col_name = readFileChar(tab_col, col_offset);
				col_offset += 32;
				cur_col->data_type = readFileInt(tab_col, col_offset) % 10;
				cur_col->max_length = readFileInt(tab_col, col_offset) / 100;
				col_offset += 8;
				cur_col->col_number = readFileInt(tab_col, col_offset);
				col_offset += 8;

				/*char* data_col_info_file_name = (char*) malloc(sizeof(char) * 48);
				concatFileName(data_col_info_file_name, "_Col_Data_Info_", cur_alloc, cur_col_alloc);

				FILE* data_col_info = fopen(data_col_info_file_name, "rb+");
				free(data_col_info_file_name);*/
				FILE* data_col_info = openFile("_Col_Data_Info_", cur_alloc, cur_col_alloc, "rb+\0");
				files_left_to_close++;

				cur_col->num_rows = readFileInt(data_col_info, 0);
				cur_col->num_open = readFileInt(data_col_info, 8);

				fflush(data_col_info);
				fclose(data_col_info);
				files_left_to_close--;

				cur_col->num_added_insert_open = 0;


				if (cur_col_alloc < cur->num_cols-1)
				{
					cur_col->next = malloc(sizeof(struct table_cols_info));

					cur_col = cur_col->next;
				}
				else
					cur_col->next = NULL;

				cur_col_alloc++;
			}

			cur->change_list_head = NULL;
			cur->change_list_tail = NULL;

			if (cur_alloc < num_tables-1)
			{
				cur->next = malloc(sizeof(struct table_info));

				cur = cur->next;
			}
			else
				cur->next = NULL;

			cur_alloc++;

			fflush(tab_col);
			fclose(tab_col);
			files_left_to_close--;
		}
	}
	else
		tables_head = NULL;

	fflush(db_info);
	fclose(db_info);
	files_left_to_close--;

	printf("initDB() files_left_to_close = %d\n", files_left_to_close);
}

void concatFileName(char* new_filename, char* filetype, int_8 table_num, int_8 col_num)
{
	strcpy(new_filename, ".\\DB_Files_2\\DB");

	strcat(new_filename, filetype);

	char new_filename_number[10];

	sprintf(new_filename_number, "%lu", table_num);
	strcat(new_filename, new_filename_number);

	if (strcmp("_Col_Data_", filetype) == 0 || strcmp("_Col_Data_Info_", filetype) == 0 || strcmp("_Col_Data_Info_Temp_", filetype) == 0)
	{
		strcat(new_filename, "_");

		char new_filename_col_number[10];

		sprintf(new_filename_col_number, "%lu", col_num);
		strcat(new_filename, new_filename_col_number);
	}

	strcat(new_filename, ".bin");
}

void createTable(char* table_name, struct table_cols_info* table_cols)
{
	int files_left_to_close = 0;
	struct table_info* table;

	if (tables_head == NULL)
	{
		tables_head = malloc(sizeof(struct table_info));
		table = tables_head;
	}
	else
	{
		table = tables_head;
		while (table->next != NULL)
			table = table->next;

		table->next = malloc(sizeof(struct table_info));

		table = table->next;
	}

	table->name = table_name;
	table->file_number = num_tables;
	table->num_cols = 0;
	table->table_cols_head = table_cols;
	table->change_list_head = NULL;
	table->change_list_tail = NULL;

	table->next = NULL;

	// START Append table name to DB_Info
	FILE* db_info_append = fopen(".\\DB_Files_2\\DB_Info.bin", "ab");
	files_left_to_close++;

	fwrite(table_name, 32, 1, db_info_append);
	fwrite(&num_tables, 8, 1, db_info_append);

	fflush(db_info_append);
	fclose(db_info_append);
	files_left_to_close--;
	// END Append table name to DB_Info

	// START Append column names to tab_col_append
	/*char* tab_col_file_name = (char*) malloc(sizeof(char) * 48);
	concatFileName(tab_col_file_name, "_Tab_Col_", num_tables, 0);

	FILE* tab_col_append = fopen(tab_col_file_name, "ab+");
	free(tab_col_file_name);*/
	FILE* tab_col_append = openFile("_Tab_Col_", num_tables, 0, "ab+\0");
	files_left_to_close++;

	int_8 spacer = 0;
	fwrite(&spacer, 8, 1, tab_col_append);

	struct table_cols_info* cur_col = table->table_cols_head;

	
	while (cur_col != NULL)
	{
		addColumn(tab_col_append, cur_col, table);
		
		table->num_cols++;

		cur_col->num_rows = 0;
		cur_col->num_open = 0;
		cur_col->num_added_insert_open = 0;

		cur_col = cur_col->next;
	}

	fflush(tab_col_append);
	fclose(tab_col_append);
	files_left_to_close--;

	
	/*FILE* tab_col = fopen(tab_col_file_name, "rb+");*/
	FILE* tab_col = openFile("_Tab_Col_", num_tables, 0, "rb+\0");
	files_left_to_close++;

	writeFileInt(tab_col, 0, &table->num_cols);

	fclose(tab_col);
	files_left_to_close--;
	// END Append column names to tab_col_append

	
	// START Edit number of tables in DB_Info
	num_tables++;
	//printf("num_tables = %lu\n", num_tables);

	FILE* db_info = fopen(".\\DB_Files_2\\DB_Info.bin", "rb+");
	files_left_to_close++;

	writeFileInt(db_info, 0, &num_tables);

	fclose(db_info);
	files_left_to_close--;
	// END Edit number of tables in DB_Info

	printf("createTable() files_left_to_close = %d\n", files_left_to_close);
}

void addColumn(FILE* tab_col_append, struct table_cols_info* cur_col, struct table_info* table)
{
	// START Append to tab_col_append file
	fwrite(cur_col->col_name, 32, 1, tab_col_append);

	int_8 datatype = cur_col->data_type + (100 * cur_col->max_length);

	fwrite(&datatype, 8, 1, tab_col_append);

	fwrite(&table->num_cols, 8, 1, tab_col_append);
	// END Append to tab_col_append file

	// START Create and append to col_data_info
	/*char* col_data_info_file_name = (char*) malloc(sizeof(char) * 48);
	concatFileName(col_data_info_file_name, "_Col_Data_Info_", table->file_number, table->num_cols);

	FILE* col_data_info = fopen(col_data_info_file_name, "ab+");
	free(col_data_info_file_name);*/
	FILE* col_data_info = openFile("_Col_Data_Info_", table->file_number, table->num_cols, "ab+\0");

	int_8 zero = 0;

	fwrite(&zero, 8, 1, col_data_info);

	fwrite(&zero, 8, 1, col_data_info);

	fflush(col_data_info);
	fclose(col_data_info);
	// END Create and append to col_data_info

	// START Create col_data
	/*char* col_data_file_name = (char*) malloc(sizeof(char) * 48);
	concatFileName(col_data_file_name, "_Col_Data_", table->file_number, table->num_cols);

	FILE* col_data = fopen(col_data_file_name, "ab+");
	free(col_data_file_name);*/
	FILE* col_data = openFile("_Col_Data_", table->file_number, table->num_cols, "ab+\0");
	
	fclose(col_data);
	// END Create col_data
}

void addToChangeList(int_8 the_transac_id, int_8 the_table_number, int_8 the_col_number, int_8 the_operation, int_8 the_data_type
					,int_8* the_data_int_date, double* the_data_real, char* the_data_string)
{
	struct table_info* cur_table = tables_head;
	while (cur_table != NULL)
	{
		if (cur_table->file_number == the_table_number)
			break;

		cur_table = cur_table->next;
	}

	struct table_cols_info* cur_col = cur_table->table_cols_head;
	while (cur_col != NULL)
	{
		if (cur_col->col_number == the_col_number)
			break;

		cur_col = cur_col->next;
	}

	struct change_node* temp = malloc(sizeof(struct change_node));

	temp->next = NULL;
	if (cur_table->change_list_head == NULL)
	{
		cur_table->change_list_head = temp;
		cur_table->change_list_tail = temp;
	}
	else
	{
		cur_table->change_list_tail->next = temp;
		cur_table->change_list_tail = temp;
	}

	temp->transac_id = the_transac_id;
	temp->table_number = the_table_number;
	temp->col_number = the_col_number;
	if (the_operation == 1 && (cur_table->table_cols_head->num_open - cur_col->num_added_insert_open) > 0)
	{
		temp->operation = 1;
		cur_col->num_added_insert_open++;
	}
	else if (the_operation == 1)
	{
		temp->operation = 2;
	}
	else if (the_operation == 2)
	{
		temp->operation = 5;
	}
	else if (the_operation == 3)
	{
		temp->operation = 3;
	}
	else if (the_operation == 4)
	{
		temp->operation = 4;
	}
	temp->data_type = the_data_type;

	temp->data_int_date = the_data_int_date == NULL ? 0 : *the_data_int_date;
	temp->data_real = the_data_real == NULL ? 0 : *the_data_real;
	temp->data_string = the_data_string;
}

void insertOpen(struct table_info* cur_table, int_8 transac_id)
{
	int files_left_to_close = 0;
	int table_changes_freed = 0;

	// START Allocate arrays for files opened during commit
	FILE** col_data_info_file_arr = malloc(sizeof(FILE*) * cur_table->num_cols);
	FILE** col_data_file_arr = malloc(sizeof(FILE*) * cur_table->num_cols);

	for (int i=0; i<cur_table->num_cols; i++)
	{
		col_data_info_file_arr[i] = NULL;
		col_data_file_arr[i] = NULL;
	}
	// END Allocate arrays for files opened during commit

	while (cur_table->change_list_head != NULL && cur_table->change_list_head->transac_id == transac_id)
	{
		struct change_node* cur_change = cur_table->change_list_head;
		
		printf("Insert open change:\n");
		printf("	Table number = %lu\n", cur_change->table_number);
		printf("	Column number = %lu\n", cur_change->col_number);
		printf("	Operation = %lu\n", cur_change->operation);
		if (cur_change->data_type == 1)
			printf("	Data int date = %lu\n", cur_change->data_int_date);
		else if (cur_change->data_type == 2)
			printf("	Data real = %f\n", cur_change->data_real);
		else if (cur_change->data_type == 3)
			printf("	Data string = %s\n", cur_change->data_string);

		// START Find max_length of column
		struct table_cols_info* cur_col = cur_table->table_cols_head;
		while (cur_col != NULL)
		{
			if (cur_col->col_number == cur_change->col_number)
				break;

			cur_col = cur_col->next;
		}
		int_8 max_length = cur_col->max_length;
		// END Find max_length of column

		// START Find or open col_data_info file for col_number
		FILE* col_data_info = col_data_info_file_arr[cur_change->col_number];
		if (col_data_info == NULL)
		{
			//printf("Had to input col_data_info file into array\n");
			/*char* col_data_info_file_name = (char*) malloc(sizeof(char) * 48);
			concatFileName(col_data_info_file_name, "_Col_Data_Info_", cur_change->table_number, cur_change->col_number);
			
			col_data_info = fopen(col_data_info_file_name, "rb+");
			free(col_data_info_file_name);*/
			col_data_info = openFile("_Col_Data_Info_", cur_change->table_number, cur_change->col_number, "rb+\0");
			files_left_to_close++;

			col_data_info_file_arr[cur_change->col_number] = col_data_info;
		}
		// END Find or open col_data_info file for col_number

		// START Find or open col_data file for table_number and col_number
		FILE* col_data = col_data_file_arr[cur_change->col_number];
		if (col_data == NULL)
		{
			//printf("Had to input col_data file into array\n");
			/*char* col_data_file_name = (char*) malloc(sizeof(char) * 48);
			concatFileName(col_data_file_name, "_Col_Data_", cur_change->table_number, cur_change->col_number);

			FILE* col_data = fopen(col_data_file_name, "rb+");
			free(col_data_file_name);*/
			col_data = openFile("_Col_Data_", cur_change->table_number, cur_change->col_number, "rb+\0");
			files_left_to_close++;

			col_data_file_arr[cur_change->col_number] = col_data;
		}
		col_data = col_data_file_arr[cur_change->col_number];
		// END Find or open col_data file for table_number and col_number

		printf("	num_rows = %lu\n", cur_col->num_rows);
		printf("	num_open = %lu\n", cur_col->num_open);
		printf("	max_length = %lu\n", max_length);


		int_8 open_offset = readFileInt(col_data_info, 16 + ((cur_col->num_open-1)*8));
		printf("	open slot = %lu\n", open_offset);

		open_offset = (open_offset*(max_length+8)) + 8;
		printf("	open_offset = %lu\n", open_offset);

		if (cur_change->data_type == 1)
		{
			printf("Currently there: %lu\n", readFileInt(col_data, open_offset));
			writeFileInt(col_data, open_offset, &cur_change->data_int_date);
		}
		else if (cur_change->data_type == 2)
		{
			printf("Currently there: %f\n", readFileDouble(col_data, open_offset));
			writeFileDouble(col_data, open_offset, &cur_change->data_real);
		}
		else if (cur_change->data_type == 3)
		{
			char* temp_str = readFileCharData(col_data, open_offset, max_length);
			printf("Currently there: %s\n", temp_str);
			free(temp_str);
			writeFileCharData(col_data, open_offset, max_length, cur_change->data_string);
		}


		// START Decrement num_rows in col_data_info
		cur_col->num_added_insert_open--;

		cur_col->num_open--;

		writeFileInt(col_data_info, 8, &cur_col->num_open);
		// END Decrement num_rows in col_data_info

		struct change_node* temp = cur_table->change_list_head;
		cur_table->change_list_head = cur_table->change_list_head->next;
		free(temp->data_string);
		free(temp);
		table_changes_freed++;
	}

	for (int i=0; i<cur_table->num_cols; i++)
	{
		if (col_data_info_file_arr[i] != NULL)
		{
			fclose(col_data_info_file_arr[i]);
			files_left_to_close--;
		}
		if (col_data_file_arr[i] != NULL)
		{
			fclose(col_data_file_arr[i]);
			files_left_to_close--;
		}
	}

	free(col_data_info_file_arr);
	free(col_data_file_arr);

	// START Write incremented num_open to _Col_Data_Info_
	struct table_cols_info* cur_col = cur_table->table_cols_head;
	for (int i=0; i<cur_table->num_cols; i++)
	{
		// START Create and append to col_data_info after deletion
		char* col_data_info_file_name = (char*) malloc(sizeof(char) * 48);
		char* col_data_info_temp_file_name = (char*) malloc(sizeof(char) * 48);
		concatFileName(col_data_info_file_name, "_Col_Data_Info_", cur_table->file_number, cur_col->col_number);
		concatFileName(col_data_info_temp_file_name, "_Col_Data_Info_Temp_", cur_table->file_number, cur_col->col_number);
			
		FILE* col_data_info_temp = fopen(col_data_info_temp_file_name, "ab+");
		files_left_to_close++;

		FILE* col_data_info = fopen(col_data_info_file_name, "rb+");
		files_left_to_close++;

		fwrite(&cur_col->num_rows, 8, 1, col_data_info_temp);

		fwrite(&cur_col->num_open, 8, 1, col_data_info_temp);

		for (int j=16; j<16+(cur_col->num_open*8); j+=8)
		{
			int_8 temp_open_slot = readFileInt(col_data_info, j);

			fwrite(&temp_open_slot, 8, 1, col_data_info_temp);
		}

		fclose(col_data_info_temp);
		fclose(col_data_info);
		files_left_to_close--;
		files_left_to_close--;

		remove(col_data_info_file_name);
		rename(col_data_info_temp_file_name, col_data_info_file_name);

		free(col_data_info_file_name);
		free(col_data_info_temp_file_name);
		// END Create and append to col_data_info after deletion

		cur_col = cur_col->next;
	}
	// END Write incremented num_open to _Col_Data_Info_

	printf("\ninsertOpen() files_left_to_close = %d\n", files_left_to_close);
	printf("insertOpen() table_changes_freed = %d\n", table_changes_freed);
}

void insertAppend(struct table_info* cur_table, int_8 transac_id)
{
	int files_left_to_close = 0;
	int table_changes_freed = 0;

	// START Allocate arrays for files opened during commit
	FILE** col_data_info_file_arr = (FILE**) malloc(sizeof(FILE*) * cur_table->num_cols);
	FILE** col_data_file_arr = (FILE**) malloc(sizeof(FILE*) * cur_table->num_cols);

	for (int i=0; i<cur_table->num_cols; i++)
	{
		col_data_info_file_arr[i] = NULL;
		col_data_file_arr[i] = NULL;
	}
	// END Allocate arrays for files opened during commit

	while (cur_table->change_list_head != NULL && cur_table->change_list_head->transac_id == transac_id)
	{
		struct change_node* cur_change = cur_table->change_list_head;
		
		printf("Insert append change:\n");
		printf("	Table number = %lu\n", cur_change->table_number);
		printf("	Column number = %lu\n", cur_change->col_number);
		printf("	Operation = %lu\n", cur_change->operation);
		if (cur_change->data_type == 1)
			printf("	Data int date = %lu\n", cur_change->data_int_date);
		else if (cur_change->data_type == 2)
			printf("	Data real = %f\n", cur_change->data_real);
		else if (cur_change->data_type == 3)
			printf("	Data string = %s\n", cur_change->data_string);

		// START Find max_length of column
		struct table_cols_info* cur_col = cur_table->table_cols_head;
		while (cur_col != NULL)
		{
			if (cur_col->col_number == cur_change->col_number)
				break;

			cur_col = cur_col->next;
		}
		int_8 max_length = cur_col->max_length;
		// END Find max_length of column

		printf("	num_rows = %lu\n", cur_col->num_rows);
		printf("	num_open = %lu\n", cur_col->num_open);
		printf("	max_length = %lu\n", max_length);

		// START Find or open col_data_info file for col_number
		FILE* col_data_info = col_data_info_file_arr[cur_change->col_number];
		if (col_data_info == NULL)
		{
			printf("Had to input col_data_info file into array\n");
			/*char* col_data_info_file_name = (char*) malloc(sizeof(char) * 48);
			concatFileName(col_data_info_file_name, "_Col_Data_Info_", cur_change->table_number, cur_change->col_number);
			
			col_data_info = fopen(col_data_info_file_name, "rb+");
			free(col_data_info_file_name);*/
			col_data_info = openFile("_Col_Data_Info_", cur_change->table_number, cur_change->col_number, "rb+\0");
			files_left_to_close++;

			col_data_info_file_arr[cur_change->col_number] = col_data_info;
		}
		// END Find or open col_data_info file for col_number

		// START Find or open col_data file for table_number and col_number
		FILE* col_data = col_data_file_arr[cur_change->col_number];
		if (col_data == NULL)
		{
			printf("Had to input col_data file into array\n");
			/*char* col_data_file_name = (char*) malloc(sizeof(char) * 48);
			concatFileName(col_data_file_name, "_Col_Data_", cur_change->table_number, cur_change->col_number);

			FILE* col_data = fopen(col_data_file_name, "ab");
			free(col_data_file_name);*/
			col_data = openFile("_Col_Data_", cur_change->table_number, cur_change->col_number, "ab+\0");
			files_left_to_close++;

			col_data_file_arr[cur_change->col_number] = col_data;
		}
		col_data = col_data_file_arr[cur_change->col_number];
		// END Find or open col_data file for table_number and col_number

		// START Append data to col_data
		fwrite(&cur_col->num_rows, 8, 1, col_data);

		if (cur_change->data_type == 1)
			fwrite(&cur_change->data_int_date, max_length, 1, col_data);
		else if (cur_change->data_type == 2)
			fwrite(&cur_change->data_real, max_length, 1, col_data);
		else if (cur_change->data_type == 3)
			fwrite(cur_change->data_string, max_length, 1, col_data);
		// END Append data to col_data

		// START Increment num_rows in col_data_info
		cur_col->num_rows++;

		writeFileInt(col_data_info, 0, &cur_col->num_rows);
		// END Increment num_rows in col_data_info


		struct change_node* temp = cur_table->change_list_head;
		cur_table->change_list_head = cur_table->change_list_head->next;
		free(temp->data_string);
		free(temp);
		table_changes_freed++;
	}

	for (int i=0; i<cur_table->num_cols; i++)
	{
		if (col_data_info_file_arr[i] != NULL)
		{
			fclose(col_data_info_file_arr[i]);
			files_left_to_close--;
		}
		if (col_data_file_arr[i] != NULL)
		{
			fclose(col_data_file_arr[i]);
			files_left_to_close--;
		}
	}

	free(col_data_info_file_arr);
	free(col_data_file_arr);

	printf("\ninsertAppend() files_left_to_close = %d\n", files_left_to_close);
	printf("insertAppend() table_changes_freed = %d\n", table_changes_freed);
}

void deleteData(struct table_info* cur_table, int_8 transac_id)
{
	int files_left_to_close = 0;
	int table_changes_freed = 0;
	int list_nodes_left_to_free = 0;
	
	struct ListNode* rows_to_be_deleted = NULL;
	struct ListNode* rows_to_be_delete_tail = NULL;
	
	while (cur_table->change_list_head != NULL && cur_table->change_list_head->transac_id == transac_id)
	{
		struct change_node* cur_change = cur_table->change_list_head;
		
		printf("Delete change:\n");
		printf("	Table number = %lu\n", cur_change->table_number);
		printf("	Column number = %lu\n", cur_change->col_number);
		printf("	Operation = %lu\n", cur_change->operation);
		if (cur_change->data_type == 1)
			printf("	Data int date = %lu\n", cur_change->data_int_date);
		else if (cur_change->data_type == 2)
			printf("	Data real = %f\n", cur_change->data_real);
		else if (cur_change->data_type == 3)
			printf("	Data string = %s\n", cur_change->data_string);

		// START Find max_length of column
		struct table_cols_info* cur_col = cur_table->table_cols_head;
		while (cur_col != NULL)
		{
			if (cur_col->col_number == cur_change->col_number)
				break;

			cur_col = cur_col->next;
		}
		int_8 max_length = cur_col->max_length;
		// END Find max_length of column

		printf("	num_rows = %lu\n", cur_col->num_rows);
		printf("	num_open = %lu\n", cur_col->num_open);
		printf("	max_length = %lu\n", max_length);

		// START Get column data from file and iterate through it all to find the rows that need to be deleted base on inputted data
		struct colDataNode** col_data_arr = getAllColData(cur_change->table_number, cur_change->col_number, cur_col->num_rows, cur_change->data_type, max_length);
		for (int i=0; i<cur_col->num_rows; i++)
		{
			char* temp_str = (char*) malloc(sizeof(char) * 32);
			if (cur_change->data_type == 1)
				sprintf(temp_str, "%lu", cur_change->data_int_date);
			else if (cur_change->data_type == 2)
				sprintf(temp_str, "%f", cur_change->data_real);

			bool deleted = false;

			if (cur_change->data_type == 1 || cur_change->data_type == 2)
			{
				if (strcmp(temp_str, col_data_arr[i]->row_data) == 0)
					deleted = true;
			}
			else if (cur_change->data_type == 3)
			{
				if (strcmp(cur_change->data_string, col_data_arr[i]->row_data))
					deleted = true;
			}
			free(temp_str);
			
			if (deleted)
			{
				// START Add row_id to list of rows that need to be deleted
				if (rows_to_be_deleted == NULL)
				{
					rows_to_be_deleted = (struct ListNode*) malloc(sizeof(struct ListNode));
					rows_to_be_delete_tail = rows_to_be_deleted;
				}
				else
				{
					rows_to_be_delete_tail->next = (struct ListNode*) malloc(sizeof(struct ListNode));
					rows_to_be_delete_tail = rows_to_be_delete_tail->next;
				}
				rows_to_be_delete_tail->value = col_data_arr[i]->row_id;
				rows_to_be_delete_tail->next = NULL;
				list_nodes_left_to_free++;
				// END Add row_id to list of rows that need to be deleted
			}
		}
		// END Get column data from file and iterate through it all to find the rows that need to be deleted base on inputted data

		struct change_node* temp = cur_table->change_list_head;
		cur_table->change_list_head = cur_table->change_list_head->next;
		free(temp->data_string);
		free(temp);
		table_changes_freed++;

		for (int i=0; i<cur_col->num_rows; i++)
		{
			free(col_data_arr[i]->row_data);
			free(col_data_arr[i]);
		}
		free(col_data_arr);
	}

	printf("Row_ids to be deleted:\n");
	struct ListNode* cur = rows_to_be_deleted;
	while (cur != NULL)
	{
		printf("%lu, ", cur->value);
		cur = cur->next;
	}
	printf("\n");

	// START Edit _Col_Data_Info_ to reflect deleted rows
	struct table_cols_info* cur_col = cur_table->table_cols_head;
	while (cur_col != NULL)
	{
		// START Append deleted row_id to _Col_Data_Info_
		FILE* col_data_info_append = openFile("_Col_Data_Info_", cur_table->file_number, cur_col->col_number, "ab+\0");
		files_left_to_close++;

		struct ListNode* cur = rows_to_be_deleted;
		while (cur != NULL)
		{
			fwrite(&cur->value, 8, 1, col_data_info_append);

			cur_col->num_open++;

			cur = cur->next;
		}

		fflush(col_data_info_append);
		fclose(col_data_info_append);
		files_left_to_close--;
		// END Append deleted row_id to _Col_Data_Info_

		// START Overwrite number of open rows with new value
		FILE* col_data_info = openFile("_Col_Data_Info_", cur_table->file_number, cur_col->col_number, "rb+\0");
		files_left_to_close++;

		writeFileInt(col_data_info, 8, &cur_col->num_open);
		fflush(col_data_info);
		fclose(col_data_info);
		files_left_to_close--;
		// END Overwrite number of open rows with new value

		// START Overwrite row values in _Col_Data_ with open_* consts
		FILE* col_data = openFile("_Col_Data_", cur_table->file_number, cur_col->col_number, "rb+\0");
		files_left_to_close++;

		cur = rows_to_be_deleted;
		while (cur != NULL)
		{
			int_8 col_data_offest = (cur->value * (cur_col->max_length + 8)) + 8;
			//printf("Overwriting col_data = %lu, with offset = %lu\n", cur_col->col_number, col_data_offest);
			if (cur_col->data_type == 1 || cur_col->data_type == 4)
			{
				//printf("Currently there = %lu\n", readFileInt(col_data, col_data_offest));
				writeFileInt(col_data, col_data_offest, &open_int);
			}
			else if (cur_col->data_type == 2)
			{
				//printf("Currently there = %lu\n", readFileDouble(col_data, col_data_offest));
				writeFileDouble(col_data, col_data_offest, &open_double);
			}
			else if (cur_col->data_type == 3)
			{
				char* temp_str = readFileCharData(col_data, col_data_offest, cur_col->max_length);
				//printf("Currently there = %s\n", temp_str);
				free(temp_str);
				writeFileCharData(col_data, col_data_offest, cur_col->max_length, open_string);
			}
			
			cur = cur->next;
		}
		fflush(col_data);
		fclose(col_data);
		files_left_to_close--;
		// END Overwrite row values in _Col_Data_ with open_* consts

		cur_col = cur_col->next;
	}
	// END Edit _Col_Data_Info_ to reflect deleted rows

	while (rows_to_be_deleted != NULL)
	{
		struct ListNode* temp = rows_to_be_deleted;
		rows_to_be_deleted = rows_to_be_deleted->next;
		free(temp);
		list_nodes_left_to_free--;
	}

	printf("\ndeleteData() files_left_to_close = %d\n", files_left_to_close);
	printf("deleteData() table_changes_freed = %d\n", table_changes_freed);
	printf("deleteData() list_nodes_left_to_free = %d\n", list_nodes_left_to_free);
}

void commit()
{
	struct table_info* cur_table = tables_head;
	while (cur_table != NULL)
	{
		while (cur_table->change_list_head != NULL)
		{
			if (cur_table->change_list_head->operation == 1)
			{
				insertOpen(cur_table, cur_table->change_list_head->transac_id);
			}
			else if (cur_table->change_list_head->operation == 2)
			{
				insertAppend(cur_table, cur_table->change_list_head->transac_id);
			}
			else if (cur_table->change_list_head->operation == 3)
			{
				deleteData(cur_table, cur_table->change_list_head->transac_id);
			}
			else if (cur_table->change_list_head->operation == 4)
			{
				deleteData(cur_table, cur_table->change_list_head->transac_id);
			}
			else if (cur_table->change_list_head->operation == 5)
			{
				insertOpen(cur_table, cur_table->change_list_head->transac_id);
			}
		}

		cur_table = cur_table->next;
	}
}

struct colDataNode** getAllColData(int_8 table_number, int_8 col_number, int_8 num_rows, int_8 data_type, int_8 max_length)
{
	int files_left_to_close = 0;

	struct colDataNode** arr = (struct colDataNode**) malloc(sizeof(struct colDataNode*) * num_rows);
	for (int i=0; i<num_rows; i++)
		arr[i] = (struct colDataNode*) malloc(sizeof(struct colDataNode));
	
	FILE* col_data = openFile("_Col_Data_", table_number, col_number, "rb");
	files_left_to_close++;

	int_8 rows_offset = 0;
	for (int i=0; i<num_rows; i++)
	{
		arr[i]->row_id = readFileInt(col_data, rows_offset);
		rows_offset += 8;

		if (data_type == 1 || data_type == 4)
		{
			arr[i]->row_data = malloc(sizeof(char) * 32);
			sprintf(arr[i]->row_data, "%lu", readFileInt(col_data, rows_offset));
		}
		else if (data_type == 2)
		{
			arr[i]->row_data = malloc(sizeof(char) * 32);
			sprintf(arr[i]->row_data, "%f", readFileDouble(col_data, rows_offset));
		}
		else if (data_type == 3)
		{
			arr[i]->row_data = readFileCharData(col_data, rows_offset, max_length);
		}

		rows_offset += max_length;
	}
	fclose(col_data);
	files_left_to_close--;

	printf("\ngetAllColData() files_left_to_close = %d\n", files_left_to_close);

	return arr;
}

char*** select(int_8 the_table_number, int_8* the_col_numbers, int the_col_numbers_size, int_8* num_rows_in_result
			  ,struct or_clause_node* or_head)
{
	int malloced = 0;
	int freed = 0;
	
	// START Find the table it is according to the_table_number
	struct table_info* cur_table = tables_head;
	while (cur_table != NULL)
	{
		if (cur_table->file_number == the_table_number)
			break;
		cur_table = cur_table->next;
	}
	// END Find the table it is according to the_table_number

	// START Pull all column data from disk
	int_8* data_type_arr = malloc(sizeof(int_8) * the_col_numbers_size);
	malloced++;
	struct colDataNode*** table_data_arr = malloc(sizeof(struct colDataNode**) * the_col_numbers_size);
	malloced++;
	for (int j=0; j<the_col_numbers_size; j++)
	{
		struct table_cols_info* cur_col = cur_table->table_cols_head;
		while (cur_col != NULL)
		{
			if (cur_col->col_number == the_col_numbers[j])
				break;
			cur_col = cur_col->next;
		}
		data_type_arr[j] = cur_col->data_type;
		
		table_data_arr[j] = getAllColData(cur_table->file_number, cur_col->col_number, cur_col->num_rows, cur_col->data_type, cur_col->max_length);
		malloced += 1 + (cur_col->num_rows*2);
	}
	// END Pull all column data from disk

	int_8 num_valid_rows = 0;
	struct ListNode* valid_rows_head = NULL;
	struct ListNode* valid_rows_tail = NULL;

	// START Traversal of where clause
	struct or_clause_node* cur_or = or_head;
	if (cur_or == NULL)
	{
		for (int i=0; i<cur_table->table_cols_head->num_rows; i++)
		{
			bool open_row = false;
			
			// START Check if row is open (has been deleted)
			char* open_check = (char*) malloc(sizeof(char) * 32);
			malloced++;
			if (data_type_arr[0] == 1 || data_type_arr[0] == 4)
				sprintf(open_check, "%lu", open_int);
			else if (data_type_arr[0] == 2)
				sprintf(open_check, "%f", open_double);
			else if (data_type_arr[0] == 3)
				strcpy(open_check, open_string);

			if (strcmp(table_data_arr[0][i]->row_data, open_check) == 0)
				open_row = true;
			free(open_check);
			freed++;
			// END Check if row is open (has been deleted)

			if (!open_row)
			{
				// START If valid add to linked list of valid rows
				num_valid_rows++;
				if (valid_rows_head == NULL)
				{
					valid_rows_head = (struct ListNode*) malloc(sizeof(struct ListNode));
					malloced++;
					valid_rows_tail = valid_rows_head;
				}
				else
				{
					valid_rows_tail->next = (struct ListNode*) malloc(sizeof(struct ListNode));
					malloced++;
					valid_rows_tail = valid_rows_tail->next;
				}
				valid_rows_tail->value = i;
				valid_rows_tail->next = NULL;
				// END If valid add to linked list of valid rows
			}
		}
	}
	while (cur_or != NULL)
	{
		int j_index;
		int i;
		bool open_row = false;
		bool valid = false;

		struct and_clause_node* cur_and = cur_or->and_head;
		while (cur_and != NULL)
		{
			// START Find index in the_col_numbers that is for cur_and->col_number
			for (int j=0; j<the_col_numbers_size; j++)
			{
				if (the_col_numbers[j] == cur_and->col_number)
				{
					j_index = j;
					break;
				}
			}
			// END Find index in the_col_numbers that is for cur_and->col_number
			
			for (i=0; i<cur_table->table_cols_head->num_rows; i++)
			{
				open_row = false;
				valid = false;
				
				// START Check if row is open (has been deleted)
				char* open_check = (char*) malloc(sizeof(char) * 32);
				malloced++;
				if (cur_and->data_type == 1 || cur_and->data_type == 4)
					sprintf(open_check, "%lu", open_int);
				else if (cur_and->data_type == 2)
					sprintf(open_check, "%f", open_double);
				else if (cur_and->data_type == 3)
					strcpy(open_check, open_string);

				//printf("row = %d col = %d\n", i, j_index);
				//printf("row_data = %s\n", table_data_arr[j_index][i]->row_data);
				//printf("open_check = %s\n", open_check);
				if (strcmp(table_data_arr[j_index][i]->row_data, open_check) == 0)
					open_row = true;
				free(open_check);
				freed++;
				// END Check if row is open (has been deleted)
					
				if (!open_row)
				{
					// START Check if row matches current and clause
					char* temp_str = (char*) malloc(sizeof(char) * 32);
					malloced++;
					if (cur_and->data_type == 1 || cur_and->data_type == 4)
						sprintf(temp_str, "%lu", cur_and->data_int_date);
					else if (cur_and->data_type == 2)
						sprintf(temp_str, "%f", cur_and->data_real);
					else if (cur_and->data_type == 3)
					{
						free(temp_str);
						freed++;
						temp_str = cur_and->data_string;
					}
					//printf("temp_str = %s\n", temp_str);

					if (strcmp(table_data_arr[j_index][i]->row_data, temp_str) == 0)
					{
						if (cur_and->where_type == 1)
							valid = true;
					}
					else 
					{
						if (cur_and->where_type == 2)
							valid = true;
					}
					if (cur_and->data_type != 3)
					{
						free(temp_str);
						freed++;
					}
					// END Check if row matches current and clause

					// START If valid add to linked list of valid rows
					if (valid)
					{
						num_valid_rows++;
						if (valid_rows_head == NULL)
						{
							valid_rows_head = (struct ListNode*) malloc(sizeof(struct ListNode));
							malloced++;
							valid_rows_tail = valid_rows_head;
						}
						else
						{
							valid_rows_tail->next = (struct ListNode*) malloc(sizeof(struct ListNode));
							malloced++;
							valid_rows_tail = valid_rows_tail->next;
						}
						valid_rows_tail->value = i;
						valid_rows_tail->next = NULL;
					}
					// END If valid add to linked list of valid rows
				}
			}
			
			cur_and = cur_and->next;
		}

		cur_or = cur_or->next;
	}
	// END Traversal of where clause

	*num_rows_in_result = num_valid_rows;

	char*** result = (char***) malloc(sizeof(char**) * num_valid_rows);
	struct ListNode* cur = valid_rows_head;
	for (int i=0; i<num_valid_rows; i++)
	{
		result[i] = (char**) malloc(sizeof(char*) * the_col_numbers_size);
		for (int j=0; j<the_col_numbers_size; j++)
		{
			if (data_type_arr[j] == 4)
			{
				result[i][j] = intToDate(table_data_arr[j][cur->value]->row_data);
				malloced++;
				free(table_data_arr[j][cur->value]->row_data);
				freed++;
			}
			else
				result[i][j] = table_data_arr[j][cur->value]->row_data;
		}
		cur = cur->next;
	}

	free(data_type_arr);
	freed++;

	for (int j=0; j<the_col_numbers_size; j++)
	{
		for (int i=0; i<cur_table->table_cols_head->num_rows; i++)
		{
			bool needs_freeing = true;
			
			struct ListNode* cur = valid_rows_head;
			while (cur != NULL)
			{
				if (cur->value == i)
				{
					needs_freeing = false;
					break;
				}
				cur = cur->next;
			}

			if (needs_freeing)
			{
				free(table_data_arr[j][i]->row_data);
				freed++;
			}
			free(table_data_arr[j][i]);
			freed++;
		}
		free(table_data_arr[j]);
		freed++;
	}
	free(table_data_arr);
	freed++;

	while (valid_rows_head != NULL)
	{
		struct ListNode* temp = valid_rows_head;
		valid_rows_head = valid_rows_head->next;
		free(temp);
		freed++;
	}

	printf("\nselect() malloced = %d\n", malloced);
	printf("select() freed = %d\n", freed);
	if (freed + (num_valid_rows*the_col_numbers_size) == malloced)
		printf("Malloced and freed amounts are correct\n");
	else
		printf("Malloced and freed amounts are NOT correct\n");
	
	return result;
}


void freeMemOfDB()
{
	printf("Freeing memory\n");
	struct table_info* cur_table = tables_head;
	while (cur_table != NULL)
	{
		if (cur_table->change_list_head != NULL)
		{
			printf("\nThere are uncommitted changes.\nCommit them with \"commit;\" or hit enter to erase those changes.\n\n");
			char input[10];
			scanf("%[^\n]", input);
			input[7] = 0;

			if (strcmp(input, "commit;") == 0)
				commit();
			else
			{
				printf("Not committing, unsaved changes will be erased.\n");
				break;
			}
		}

		cur_table = cur_table->next;
	}

	int tables_freed = 0;
	int table_cols_freed = 0;
	int table_changes_freed = 0;
	while (tables_head != NULL)
	{
		while (tables_head->table_cols_head != NULL)
		{
			struct table_cols_info* temp = tables_head->table_cols_head;
			tables_head->table_cols_head = tables_head->table_cols_head->next;
			free(temp->col_name);
			free(temp);

			table_cols_freed++;
		}

		while (tables_head->change_list_head != NULL)
		{
			struct change_node* temp = tables_head->change_list_head;
			tables_head->change_list_head = tables_head->change_list_head->next;

			free(temp->data_string);
			free(temp);

			table_changes_freed++;
		}

		struct table_info* temp = tables_head;
		tables_head = tables_head->next;
		free(temp->name);
		free(temp);

		tables_freed++;
	}

	printf("\ntables_freed = %d\n", tables_freed);
	printf("table_cols_freed = %d\n", table_cols_freed);
	printf("table_changes_freed = %d\n", table_changes_freed);
}

void traverseTablesInfo()
{
	printf("\nIN MEMORY\n\nnum_tables = %lu\n", num_tables);

	struct table_info* cur = tables_head;
	while (cur != NULL)
	{
		printf("Table name = %s\n", cur->name);
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
			printf("	Number added to insert open list = %lu\n", cur_col->num_added_insert_open);

			cur_col = cur_col->next;
		}

		struct change_node* cur_change = cur->change_list_head;
		while (cur_change != NULL)
		{
			if (cur_change->operation == 1)
				printf("	_ Insert open change:\n");
			else if (cur_change->operation == 2)
				printf("	_ Insert append change:\n");
			else if (cur_change->operation == 3)
				printf("	_ Delete with no update change:\n");
			else if (cur_change->operation == 4)
				printf("	_ Delete before the update:\n");
			else if (cur_change->operation == 5)
				printf("	_ Insert open after deletion:\n");
			printf("		Transaction ID = %lu\n", cur_change->transac_id);
			printf("		Table number = %lu\n", cur_change->table_number);
			printf("		Column number = %lu\n", cur_change->col_number);
			printf("		Operation = %lu\n", cur_change->operation);
			printf("		Datatype = %lu\n", cur_change->data_type);
			if (cur_change->data_type == 1)
				printf("		Data int date = %lu\n", cur_change->data_int_date);
			else if (cur_change->data_type == 2)
				printf("		Data real = %f\n", cur_change->data_real);
			else if (cur_change->data_type == 3)
				printf("		Data string = %s\n", cur_change->data_string);

			cur_change = cur_change->next;
		}

		cur = cur->next;
	}
}

void traverseDB_Data_Files_v2()
{
	int files_left_to_close = 0;

	FILE* db_info = fopen(".\\DB_Files_2\\DB_Info.bin", "rb+");
	files_left_to_close++;

	int_8 temp_num_tables = readFileInt(db_info, 0);
	printf("\nDATA FILES ON DISK\n\nnum_tables = %lu\n\n", temp_num_tables);

	int cur_offset = 8;
	char* temp_table_name;
	while ((temp_table_name = readFileChar(db_info, cur_offset)) != NULL)
	{
		printf("Table name = %s\n", temp_table_name);
		free(temp_table_name);
		cur_offset += 32;
		int_8 table_number = readFileInt(db_info, cur_offset);
		printf("Table number = %lu\n", table_number);
		cur_offset += 8;

		FILE* tab_col = openFile("_Tab_Col_", table_number, 0, "rb+\0");
		files_left_to_close++;

		printf("Number of columns = %lu\n", readFileInt(tab_col, 0));

		int cur_col_offset = 8;
		char* temp_col_name;
		while ((temp_col_name = readFileChar(tab_col, cur_col_offset)) != NULL)
		{
			printf("	Column name = %s\n", temp_col_name);
			free(temp_col_name);
			cur_col_offset += 32;

			int_8 temp_data_type = readFileInt(tab_col, cur_col_offset) % 10;
			int_8 temp_max_length = readFileInt(tab_col, cur_col_offset) / 100;

			printf("	Datatype = %lu\n", temp_data_type);
			printf("	Max length = %lu\n", temp_max_length);
			cur_col_offset += 8;
			int_8 col_number = readFileInt(tab_col, cur_col_offset);
			printf("	Column number = %lu\n", col_number);
			cur_col_offset += 8;

			char* col_data_info_file_name = (char*) malloc(sizeof(char) * 48);
			concatFileName(col_data_info_file_name, "_Col_Data_Info_", table_number, col_number);

			if (access(col_data_info_file_name, F_OK) == 0)
			{
				free(col_data_info_file_name);
				FILE* col_data_info = openFile("_Col_Data_Info_", table_number, col_number, "rb+\0");
				files_left_to_close++;

				printf("	Number of rows = %lu\n", readFileInt(col_data_info, 0));
				printf("	Number of open slots = %lu\n", readFileInt(col_data_info, 8));
				
				printf("	Open slots: ");
				int_8 open_offset = 16;
				int_8 open_slot;
				while ((open_slot = readFileInt(col_data_info, open_offset)) != -1)
				{
					printf("%lu, ", open_slot);
					open_offset += 8;
				}
				printf("\n");

				fclose(col_data_info);
				files_left_to_close--;


				FILE* col_data = openFile("_Col_Data_", table_number, col_number, "rb+\0");
				files_left_to_close++;

				printf("	Column data:\n");
				int_8 rows_offset = 0;
				int_8 row_id;
				while ((row_id = readFileInt(col_data, rows_offset)) != -1)
				{
					printf("		%lu	", row_id);
					rows_offset += 8;

					if (temp_data_type == 1 || temp_data_type == 4)
						printf("%lu\n", readFileInt(col_data, rows_offset));
					else if (temp_data_type == 2)
						printf("%f\n", readFileDouble(col_data, rows_offset));
					else if (temp_data_type == 3)
					{
						char* temp_str = readFileCharData(col_data, rows_offset, temp_max_length);
						printf("%s\n", temp_str);
						free(temp_str);
					}

					rows_offset += temp_max_length;
				}
				fclose(col_data);
				files_left_to_close--;
			}
		}

		fclose(tab_col);
		files_left_to_close--;
	}

	fclose(db_info);
	files_left_to_close--;

	printf("\ntraverseDB_Data_Files_v2() files_left_to_close = %d\n", files_left_to_close);
}


char* intToDate(char* the_int_form)
{
	int year = 1900;
	int month = 1;
	int day = 1;

	int remaining = atoi(the_int_form);

	// START Find the right year
	while (remaining > 1461)
	{
		year += 4;
		remaining -= 1461;
	}
	if (remaining > 366)
	{
		year += 1;
		remaining -= 366;
	}
	if (year % 4 != 0)
	{
		while (remaining > 365)
		{
			year += 1;
			remaining -= 365;
		}
	}
	// END Find the right year

	// START Find the right month
	if (remaining > 31)	//	Jan
	{
		month += 1;
		remaining -= 31;
	}
	if (remaining > (year % 4 == 0 ? 29 : 28))	//	Feb
	{
		month += 1;
		remaining -= (year % 4 == 0 ? 29 : 28);
	}
	if (remaining > 31)	//	Mar
	{
		month += 1;
		remaining -= 31;
	}
	if (remaining > 30)	//	Apr
	{
		month += 1;
		remaining -= 30;
	}
	if (remaining > 31)	//	May
	{
		month += 1;
		remaining -= 31;
	}
	if (remaining > 30)	//	Jun
	{
		month += 1;
		remaining -= 30;
	}
	if (remaining > 31)	//	Jul
	{
		month += 1;
		remaining -= 31;
	}
	if (remaining > 31)	//	Aug
	{
		month += 1;
		remaining -= 31;
	}
	if (remaining > 30)	//	Sep
	{
		month += 1;
		remaining -= 30;
	}
	if (remaining > 31)	//	Oct
	{
		month += 1;
		remaining -= 31;
	}
	if (remaining > 30)	//	Nov
	{
		month += 1;
		remaining -= 30;
	}
	if (remaining > 31)	//	Dec
	{
		month += 1;
		remaining -= 31;
	}
	// END Find the right month

	day += remaining;

	char* date = (char*) malloc(sizeof(char) * 20);

	char month_char[3];
	sprintf(month_char, "%d", month);
	strcpy(date, month_char);
	strcat(date, "/");
	
	char day_char[3];
	sprintf(day_char, "%d", day);
	strcat(date, day_char);
	strcat(date, "/");

	char year_char[5];
	sprintf(year_char, "%d", year);
	strcat(date, year_char);

	return date;
}

int_8 dateToInt(char* the_date_form)
{
	int year;
	int month;
	int day;
	
	sscanf(the_date_form, "%d/%d/%d", &month, &day, &year);

	int_8 remaining = (int_8) day;
	if (month == 12)	//	Nov
	{
		remaining += 30;
		month -= 1;
	}
	if (month == 11)	//	Oct
	{
		remaining += 31;
		month -= 1;
	}
	if (month == 10)	//	Sep
	{
		remaining += 30;
		month -= 1;
	}
	if (month == 9)		//	Aug
	{
		remaining += 31;
		month -= 1;
	}
	if (month == 8)		//	Jul
	{
		remaining += 31;
		month -= 1;
	}
	if (month == 7)		//	Jun
	{
		remaining += 30;
		month -= 1;
	}
	if (month == 6)		//	May
	{
		remaining += 31;
		month -= 1;
	}
	if (month == 5)		//	Apr
	{
		remaining += 30;
		month -= 1;
	}
	if (month == 4)		//	Mar
	{
		remaining += 31;
		month -= 1;
	}
	if (month == 3)		//	Feb
	{
		remaining += (year % 4 == 0 ? 29 : 28);
		month -= 1;
	}
	if (month == 2)		//	Jan
	{
		remaining += 31;
		month -= 1;
	}

	while (year % 4 != 0)
	{
		remaining += 365;
		year--;
	}

	while (year > 1900)
	{
		remaining += 1461;
		year -= 4;
	}

	return remaining;
}


char* readFileChar(FILE* file, int_8 offset)
{
	int num_bytes = 32;

	char* raw_bytes = (char*) malloc(sizeof(char) * num_bytes);

	fseek(file, offset, SEEK_SET);

	//fread(raw_bytes, num_bytes, 1, file);
	if (fread(raw_bytes, num_bytes, 1, file) == 0)
	{
		free(raw_bytes);
		return NULL;
	}

	return raw_bytes;
}

char* readFileCharData(FILE* file, int_8 offset, int_8 num_bytes)
{
	char* raw_bytes = (char*) malloc(sizeof(char) * num_bytes);

	fseek(file, offset, SEEK_SET);

	//fread(raw_bytes, num_bytes, 1, file);
	if (fread(raw_bytes, num_bytes, 1, file) == 0)
	{
		free(raw_bytes);
		return NULL;
	}

	return raw_bytes;
}

int_8 readFileInt(FILE* file, int_8 offset)
{
	int num_bytes = 8;

	int_8 raw_int;

	fseek(file, offset, SEEK_SET);

	//fread(&raw_int, num_bytes, 1, file);
	if (fread(&raw_int, num_bytes, 1, file) == 0)
		return -1;

	return raw_int;
}

double readFileDouble(FILE* file, int_8 offset)
{
	int num_bytes = 8;

	double raw_double;

	fseek(file, offset, SEEK_SET);

	//fread(&raw_double, num_bytes, 1, file);
	if (fread(&raw_double, num_bytes, 1, file) == 0)
		return -1.0;

	return raw_double;
}

void writeFileChar(FILE* file, int_8 offset, char* data)
{
	int num_bytes = 32;

	fseek(file, offset, SEEK_SET);

	fwrite(data, num_bytes, 1, file);

	fflush(file);
}

void writeFileCharData(FILE* file, int_8 offset, int_8 num_bytes, char* data)
{
	fseek(file, offset, SEEK_SET);

	fwrite(data, num_bytes, 1, file);

	fflush(file);
}

void writeFileInt(FILE* file, int_8 offset, int_8* data)
{
	int num_bytes = 8;

	fseek(file, offset, SEEK_SET);

	fwrite(data, num_bytes, 1, file);

	fflush(file);
}

void writeFileDouble(FILE* file, int_8 offset, double* data)
{
	int num_bytes = 8;

	fseek(file, offset, SEEK_SET);

	fread(data, num_bytes, 1, file);

	fflush(file);
}