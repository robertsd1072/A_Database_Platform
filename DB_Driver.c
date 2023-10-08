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
#include "DB_Driver.h"
#include "DB_HelperFunctions.h"

typedef unsigned long long int_8;

static char open_string[3] = "~~\0";
static int_8 open_int = -1;
static double open_double = -1.0;

static char null_string[4] = "#*#\0";
static int_8 null_int = 0x55555555;
static int_8 null_double = 0x55555555;

static int_8 num_tables;
static struct table_info* tables_head;


struct table_info* getTablesHead() { return tables_head; }


int createNextTableNumFile(struct malloced_node** malloced_head, int the_debug)
{
	struct file_opened_node* file_opened_head = NULL;
	
	FILE* next_table_file = myFileOpenSimple("Next_Table_Num.bin", "ab+", &file_opened_head, malloced_head, the_debug);
	if (next_table_file == NULL)
	{
		if (the_debug == YES_DEBUG)
			printf("	ERROR in createNextTableNumFile() at line %d in %s\n", __LINE__, __FILE__);
		return -1;
	}

	int_8 next_table_num = 1;

	if (writeFileInt(next_table_file, -1, &next_table_num, &file_opened_head, malloced_head, the_debug) == -1)
	{
		if (the_debug == YES_DEBUG)
			printf("	ERROR in createNextTableNumFile() at line %d in %s\n", __LINE__, __FILE__);
		return -1;
	}

	myFileClose(next_table_file, &file_opened_head, malloced_head, the_debug);

	if (file_opened_head != NULL)
    {
        if (the_debug == YES_DEBUG)
        	printf("createNextTableNumFile() did not close all files\n");
        myFileCloseAll(&file_opened_head, malloced_head, the_debug);
    }

	return 0;
}

int_8 getNextTableNum(struct malloced_node** malloced_head, int the_debug)
{
	if (access("Next_Table_Num.bin", F_OK) != 0)
		createNextTableNumFile(malloced_head, the_debug);

	struct file_opened_node* file_opened_head = NULL;

	FILE* next_table_file = myFileOpenSimple("Next_Table_Num.bin", "rb+", &file_opened_head, malloced_head, the_debug);
	if (next_table_file == NULL)
	{
		if (the_debug == YES_DEBUG)
			printf("	ERROR in getNextTableNum() at line %d in %s\n", __LINE__, __FILE__);
		return -1;
	}

	int_8 next_table_num;

	if ((next_table_num = readFileInt(next_table_file, 0, &file_opened_head, malloced_head, the_debug)) == -1)
	{
		if (the_debug == YES_DEBUG)
			printf("	ERROR in getNextTableNum() at line %d in %s\n", __LINE__, __FILE__);
		return -1;
	}

	next_table_num++;

	if (writeFileInt(next_table_file, 0, &next_table_num, &file_opened_head, malloced_head, the_debug) == -1)
	{
		if (the_debug == YES_DEBUG)
			printf("	ERROR in getNextTableNum() at line %d in %s\n", __LINE__, __FILE__);
		return -1;
	}

	myFileClose(next_table_file, &file_opened_head, malloced_head, the_debug);

	if (file_opened_head != NULL)
    {
        if (the_debug == YES_DEBUG)
        	printf("getNextTableNum() did not close all files\n");
        myFileCloseAll(&file_opened_head, malloced_head, the_debug);
    }

	return next_table_num;
}

/*	 
 *	Malloced items: 
 *		struct table_info* tables_head;
 */
int initDB(struct malloced_node** malloced_head, int the_debug)
{
	struct file_opened_node* file_opened_head = NULL;
    
    //FILE* db_info = myFileOpen(&file_opened_head, "_Info", -1, -1, "rb+", the_debug);
    //if (db_info == NULL)
    FILE* db_info;
    if (access("DB_Files_2\\DB_Info.bin", F_OK) != 0)
	{
		// START Create DB_Info if doesn't exist	
		db_info = myFileOpen("_Info", -1, -1, "ab+", &file_opened_head, malloced_head, the_debug);
		if (db_info == NULL)
		{
			if (the_debug == YES_DEBUG)
				printf("	ERROR in initDB() at line %d in %s\n", __LINE__, __FILE__);
			return -1;
		}

		num_tables = 0;

		if (writeFileInt(db_info, -1, &num_tables, &file_opened_head, malloced_head, the_debug) == -1)
		{
			if (the_debug == YES_DEBUG)
				printf("	ERROR in initDB() at line %d in %s\n", __LINE__, __FILE__);
			return -1;
		}
		// END Create DB_Info if doesn't exist
	}
	else
	{
		db_info = myFileOpen("_Info", -1, -1, "rb+", &file_opened_head, malloced_head, the_debug);
		if (db_info == NULL)
		{
			if (the_debug == YES_DEBUG)
				printf("	ERROR in initDB() at line %d in %s\n", __LINE__, __FILE__);
			return -1;
		}
		num_tables = readFileInt(db_info, 0, &file_opened_head, malloced_head, the_debug);
	}

	if (num_tables > 0)
	{
		// START Allocate space for tables_head
		tables_head = (struct table_info*) myMalloc(sizeof(struct table_info), &file_opened_head, malloced_head, the_debug);
		if (tables_head == NULL)
		{
			if (the_debug == YES_DEBUG)
				printf("	ERROR in initDB() at line %d in %s\n", __LINE__, __FILE__);
			return -2;
		}
		struct table_info* cur = tables_head;
		// END Allocate space for tables_head

		// START Read files and input data into tables_head
		int cur_alloc = 0;
		int_8 offset = 8;
		while (cur_alloc < num_tables)
		{
			// START Read db_info for name and file number of table
            cur->name = readFileChar(db_info, offset, &file_opened_head, malloced_head, the_debug);
			offset += 32;
			cur->file_number = readFileInt(db_info, offset, &file_opened_head, malloced_head, the_debug);
			offset += 8;
            // END Read db_info for name and file number of table

			// START Read tab_col for number of columns
            FILE* tab_col = myFileOpen("_Tab_Col_", cur->file_number, 0, "rb+", &file_opened_head, malloced_head, the_debug);
			if (tab_col == NULL)
			{
				if (the_debug == YES_DEBUG)
					printf("	ERROR in initDB() at line %d in %s\n", __LINE__, __FILE__);
				return -1;
			}

			int_8 col_offset = 0;

			cur->num_cols = readFileInt(tab_col, col_offset, &file_opened_head, malloced_head, the_debug);
            // END Read tab_col for number of columns
			
			// START For each column in tab_col, allocate a struct table_cols_info and read info
            cur->table_cols_head = (struct table_cols_info*) myMalloc(sizeof(struct table_cols_info), &file_opened_head, malloced_head, the_debug);
			if (cur->table_cols_head == NULL)
			{
				if (the_debug == YES_DEBUG)
					printf("	ERROR in initDB() at line %d in %s\n", __LINE__, __FILE__);
				return -2;
			}

			struct table_cols_info* cur_col = cur->table_cols_head;

			int cur_col_alloc = 0;
			col_offset += 8;

			while (cur_col_alloc < cur->num_cols)
			{
				cur_col->col_name = readFileChar(tab_col, col_offset, &file_opened_head, malloced_head, the_debug);
				col_offset += 32;

				int_8 datatype_and_max_length = readFileInt(tab_col, col_offset, &file_opened_head, malloced_head, the_debug);
				cur_col->data_type = datatype_and_max_length & 0xff;
				cur_col->max_length = datatype_and_max_length >> 8;
				
				col_offset += 8;
				cur_col->col_number = readFileInt(tab_col, col_offset, &file_opened_head, malloced_head, the_debug);
				col_offset += 8;

				// START Read data_col_info for num rows and num open
                FILE* data_col_info = myFileOpen("_Col_Data_Info_", cur->file_number, cur_col->col_number, "rb+", &file_opened_head, malloced_head, the_debug);
				if (data_col_info == NULL)
				{
					if (the_debug == YES_DEBUG)
						printf("	ERROR in initDB() at line %d in %s\n", __LINE__, __FILE__);
					return -1;
				}

				cur_col->num_rows = readFileInt(data_col_info, 0, &file_opened_head, malloced_head, the_debug);
				cur_col->num_open = readFileInt(data_col_info, 8, &file_opened_head, malloced_head, the_debug);

				// START Read open rows into linked list
				if (cur_col->num_open == 0)
				{
					cur_col->open_list_head = NULL;
					cur_col->open_list_before_tail = NULL;
				}
				else
				{
					cur_col->open_list_head = NULL;
					cur_col->open_list_before_tail = NULL;

					if (addListNode(&cur_col->open_list_head, &cur_col->open_list_before_tail, readFileInt(data_col_info, 16, &file_opened_head, malloced_head, the_debug)
								   ,ADDLISTNODE_TAIL, &file_opened_head, malloced_head, the_debug) != 0)
					{
						if (the_debug == YES_DEBUG)
							printf("	ERROR in initDB() at line %d in %s\n", __LINE__, __FILE__);
						return -2;
					}

					if (cur_col->num_open == 1)
						cur_col->open_list_before_tail = NULL;

					for (int i=1; i<cur_col->num_open; i++)
					{
						if (addListNode(&cur_col->open_list_head, &cur_col->open_list_before_tail, readFileInt(data_col_info, 16 + (i*8), &file_opened_head, malloced_head, the_debug)
									   ,ADDLISTNODE_TAIL, &file_opened_head, malloced_head, the_debug) != 0)
						{
							if (the_debug == YES_DEBUG)
								printf("	ERROR in initDB() at line %d in %s\n", __LINE__, __FILE__);
							return -2;
						}
					}

					if (cur_col->num_open > 1)
						cur_col->open_list_before_tail = cur_col->open_list_before_tail->prev;
				}
				// END Read open rows into linked list

				myFileClose(data_col_info, &file_opened_head, malloced_head, the_debug);

                cur_col->num_added_insert_open = 0;
                // END Read data_col_info for num rows and num open

				// START If more columns to read, allocate another struct table_cols_info
                if (cur_col_alloc < cur->num_cols-1)
				{
					cur_col->next = (struct table_cols_info*) myMalloc(sizeof(struct table_cols_info), &file_opened_head, malloced_head, the_debug);
					if (cur_col->next == NULL)
					{
						if (the_debug == YES_DEBUG)
							printf("	ERROR in initDB() at line %d in %s\n", __LINE__, __FILE__);
						return -2;
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

			// START If more tables to read, allocate another struct table_info
            cur->change_list_head = NULL;
			cur->change_list_tail = NULL;

			if (cur_alloc < num_tables-1)
			{
				cur->next = (struct table_info*) myMalloc(sizeof(struct table_info), &file_opened_head, malloced_head, the_debug);
				if (cur->next == NULL)
				{
					if (the_debug == YES_DEBUG)
						printf("	ERROR in initDB() at line %d in %s\n", __LINE__, __FILE__);
					return -2;
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

	return 0;
}

int traverseTablesInfoMemory()
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
			if (cur_col->open_list_head != NULL)
			{
				struct ListNode* cur_open;

				if (cur_col->open_list_before_tail == NULL && cur_col->open_list_head->next != NULL)
					cur_open = cur_col->open_list_head->next;
				else if (cur_col->open_list_before_tail == NULL && cur_col->open_list_head->next == NULL)
					cur_open = cur_col->open_list_head;
				else
					cur_open = cur_col->open_list_before_tail->next;

				traverseListNodes(&cur_col->open_list_head, &cur_open, TRAVERSELISTNODES_HEAD, "	Open slots: ");

				traverseListNodes(&cur_col->open_list_head, &cur_open, TRAVERSELISTNODES_TAIL, "	Open slots in reverse: ");
			}
			printf("	Open row before tail = %d\n", cur_col->open_list_before_tail == NULL ? -1 : cur_col->open_list_before_tail->value);
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
    printf("\n");
	return 0;
}

/*	 
 *	Malloced items: 
 *		struct table_info* (added to tables_head);
 */
int createTable(char* table_name, struct table_cols_info* table_cols, struct malloced_node** malloced_head, int the_debug)
{
    printf("Calling createTable()\n");
    struct file_opened_node* file_opened_head = NULL;

	// START Allocate space for a new struct table_info
	struct table_info* table;
	if (tables_head == NULL)
	{
		tables_head = (struct table_info*) myMalloc(sizeof(struct table_info), &file_opened_head, malloced_head, the_debug);
		if (tables_head == NULL)
		{
			if (the_debug == YES_DEBUG)
				printf("	ERROR in createTable() at line %d in %s\n", __LINE__, __FILE__);
			return -2;
		}
		table = tables_head;
	}
	else
	{
		table = tables_head;
		while (table->next != NULL)
			table = table->next;

		table->next = (struct table_info*) myMalloc(sizeof(struct table_info), &file_opened_head, malloced_head, the_debug);
		if (table->next == NULL)
		{
			if (the_debug == YES_DEBUG)
				printf("	ERROR in createTable() at line %d in %s\n", __LINE__, __FILE__);
			return -2;
		}
		table = table->next;
	}
	// END Allocate space for a new struct table_info

	// START Assign new struct table_info values
	table->name = table_name;
	table->file_number = getNextTableNum(malloced_head, the_debug);
	if (table->file_number == -1)
	{
		if (the_debug == YES_DEBUG)
			printf("	ERROR in createTable() at line %d in %s\n", __LINE__, __FILE__);
		return -1;
	}
	table->num_cols = 0;
	table->table_cols_head = table_cols;
	table->change_list_head = NULL;
	table->change_list_tail = NULL;

	table->next = NULL;
	// END Assign new struct table_info values

	// START Append table name to DB_Info
	FILE* db_info_append = myFileOpen("_Info", -1, -1, "ab", &file_opened_head, malloced_head, the_debug);
	if (db_info_append == NULL)
	{
		if (the_debug == YES_DEBUG)
			printf("	ERROR in createTable() at line %d in %s\n", __LINE__, __FILE__);
		return -1;
	}

	if (writeFileChar(db_info_append, -1, table_name, &file_opened_head, malloced_head, the_debug) != 0)
	{
		if (the_debug == YES_DEBUG)
			printf("	ERROR in createTable() at line %d in %s\n", __LINE__, __FILE__);
		return -1;
	}
	if (writeFileInt(db_info_append, -1, &table->file_number, &file_opened_head, malloced_head, the_debug) != 0)
	{
		if (the_debug == YES_DEBUG)
			printf("	ERROR in createTable() at line %d in %s\n", __LINE__, __FILE__);
		return -1;
	}
	myFileClose(db_info_append, &file_opened_head, malloced_head, the_debug);
	// END Append table name to DB_Info

	// START Create and open tab_col file and append spacer for num cols
	FILE* tab_col_append = myFileOpen("_Tab_Col_", table->file_number, 0, "ab+", &file_opened_head, malloced_head, the_debug);
	if (tab_col_append == NULL)
	{
		if (the_debug == YES_DEBUG)
			printf("	ERROR in createTable() at line %d in %s\n", __LINE__, __FILE__);
		return -1;
	}

	int_8 spacer_for_num_cols = 0;
	if (writeFileInt(tab_col_append, -1, &spacer_for_num_cols, &file_opened_head, malloced_head, the_debug) != 0)
	{
		if (the_debug == YES_DEBUG)
			printf("	ERROR in createTable() at line %d in %s\n", __LINE__, __FILE__);
		return -1;
	}
	// END Create and open tab_col file and append spacer for num cols

	// START Append column info to tab_col and create col_data and col_data_info files
	struct table_cols_info* cur_col = table->table_cols_head;
	while (cur_col != NULL)
	{
		if (addColumn(tab_col_append, cur_col, table, malloced_head, the_debug) != 0)
		{
			if (the_debug == YES_DEBUG)
				printf("	ERROR in createTable() at line %d in %s\n", __LINE__, __FILE__);
			return -1;
		}
		
		table->num_cols++;

		cur_col->num_rows = 0;
		cur_col->num_open = 0;
		cur_col->open_list_head = NULL;
		cur_col->open_list_before_tail = NULL;
		cur_col->num_added_insert_open = 0;

		cur_col = cur_col->next;
	}

	fflush(tab_col_append);
	myFileClose(tab_col_append, &file_opened_head, malloced_head, the_debug);
	// END Append column info to tab_col and create col_data and col_data_info files

	// START Write new number of columns to tab_col
	FILE* tab_col = myFileOpen("_Tab_Col_", table->file_number, 0, "rb+", &file_opened_head, malloced_head, the_debug);
	if (tab_col == NULL)
	{
		if (the_debug == YES_DEBUG)
			printf("	ERROR in createTable() at line %d in %s\n", __LINE__, __FILE__);
		return -1;
	}

	if (writeFileInt(tab_col, 0, &table->num_cols, &file_opened_head, malloced_head, the_debug) != 0)
	{
		if (the_debug == YES_DEBUG)
			printf("	ERROR in createTable() at line %d in %s\n", __LINE__, __FILE__);
		return -1;
	}
	myFileClose(tab_col, &file_opened_head, malloced_head, the_debug);
	// END Write new number of columns to tab_col

	// START Edit number of tables in DB_Info
	num_tables++;

	FILE* db_info = myFileOpen("_Info", -1, -1, "rb+", &file_opened_head, malloced_head, the_debug);
	if (db_info == NULL)
	{
		if (the_debug == YES_DEBUG)
			printf("	ERROR in createTable() at line %d in %s\n", __LINE__, __FILE__);
		return -1;
	}

	if (writeFileInt(db_info, 0, &num_tables, &file_opened_head, malloced_head, the_debug) != 0)
	{
		if (the_debug == YES_DEBUG)
			printf("	ERROR in createTable() at line %d in %s\n", __LINE__, __FILE__);
		return -1;
	}
	myFileClose(db_info, &file_opened_head, malloced_head, the_debug);
	// END Edit number of tables in DB_Info


	// Keep this since mallocing things that should be persistent after the operation
    if (the_debug == YES_DEBUG)
		printf("Calling myFreeAllCleanup() from createTable(), but NOT freeing ptrs for tables_head\n");
	myFreeAllCleanup(malloced_head, the_debug);

    if (file_opened_head != NULL)
    {
        if (the_debug == YES_DEBUG)
			printf("createTable() did not close all files\n");
        myFileCloseAll(&file_opened_head, malloced_head, the_debug);
    }

	return 0;
}

int addColumn(FILE* tab_col_append, struct table_cols_info* cur_col, struct table_info* table, struct malloced_node** malloced_head, int the_debug)
{
	struct file_opened_node* file_opened_head = NULL;
	
	// START Append to tab_col_append file
	fwrite(cur_col->col_name, 32, 1, tab_col_append);

	int_8 datatype = cur_col->data_type + (cur_col->max_length << 8);

	fwrite(&datatype, 8, 1, tab_col_append);

	fwrite(&table->num_cols, 8, 1, tab_col_append);
	// END Append to tab_col_append file

	// START Create and append to col_data_info
	FILE* col_data_info = myFileOpen("_Col_Data_Info_", table->file_number, table->num_cols, "ab+", &file_opened_head, malloced_head, the_debug);
	if (col_data_info == NULL)
	{
		if (the_debug == YES_DEBUG)
			printf("	ERROR in addColumn() at line %d in %s\n", __LINE__, __FILE__);
		return -1;
	}

	int_8 zero = 0;

	fwrite(&zero, 8, 1, col_data_info);

	fwrite(&zero, 8, 1, col_data_info);

	fflush(col_data_info);
	myFileClose(col_data_info, &file_opened_head, malloced_head, the_debug);
	// END Create and append to col_data_info

	// START Create col_data
	FILE* col_data = myFileOpen("_Col_Data_", table->file_number, table->num_cols, "ab+\0", &file_opened_head, malloced_head, the_debug);
	if (col_data == NULL)
	{
		if (the_debug == YES_DEBUG)
			printf("	ERROR in addColumn() at line %d in %s\n", __LINE__, __FILE__);
		return -1;
	}
	myFileClose(col_data, &file_opened_head, malloced_head, the_debug);
	// END Create col_data

	if (file_opened_head != NULL)
	{
		if (the_debug == YES_DEBUG)
			printf("addColumn() did not close all files\n");
		myFileCloseAll(&file_opened_head, malloced_head, the_debug);
	}

	return 0;
}

int insertAppend(FILE** col_data_info_file_arr, FILE** col_data_file_arr, struct file_opened_node** file_opened_head
				,int_8 the_table_number, struct table_cols_info* the_col
				,int_8 the_data_int_date, double the_data_real, char* the_data_string
				,struct malloced_node** malloced_head, int the_debug)
{
	//printf("Here A\n");
	// START Find or open col_data_info file for col_number
	FILE* col_data_info = col_data_info_file_arr[the_col->col_number];
	if (col_data_info == NULL)
	{
		printf("Had to input col_data_info file into array\n");
		col_data_info = myFileOpen("_Col_Data_Info_", the_table_number, the_col->col_number, "rb+", file_opened_head, malloced_head, the_debug);
		if (col_data_info == NULL)
		{
			if (the_debug == YES_DEBUG)
				printf("	ERROR in insertAppend() at line %d in %s\n", __LINE__, __FILE__);
			return -1;
		}
		col_data_info_file_arr[the_col->col_number] = col_data_info;
	}
	col_data_info = col_data_info_file_arr[the_col->col_number];
	// END Find or open col_data_info file for col_number

	//printf("Here B\n");
	// START Find or open col_data file for table_number and col_number
	FILE* col_data = col_data_file_arr[the_col->col_number];
	if (col_data == NULL)
	{
		printf("Had to input col_data file into array\n");
		col_data = myFileOpen("_Col_Data_", the_table_number, the_col->col_number, "ab+", file_opened_head, malloced_head, the_debug);
		if (col_data == NULL)
		{
			if (the_debug == YES_DEBUG)
				printf("	ERROR in insertAppend() at line %d in %s\n", __LINE__, __FILE__);
			return -1;
		}
		col_data_file_arr[the_col->col_number] = col_data;
	}
	col_data = col_data_file_arr[the_col->col_number];
	// END Find or open col_data file for table_number and col_number

	//printf("Here C\n");
	// START Append row_id to column
	if (writeFileInt(col_data, APPEND_OFFSET, &the_col->num_rows, file_opened_head, malloced_head, the_debug) != 0)
	{
		if (the_debug == YES_DEBUG)
			printf("	ERROR in insertAppend() at line %d in %s\n", __LINE__, __FILE__);
		return -1;
	};
	// END Append row_id to column

	//printf("Here D\n");
	// START Append data to col_data
	if (the_col->data_type == DATA_INT || the_col->data_type == DATA_DATE)
	{
		if (writeFileInt(col_data, APPEND_OFFSET, &the_data_int_date, file_opened_head, malloced_head, the_debug) != 0)
		{
			if (the_debug == YES_DEBUG)
				printf("	ERROR in insertAppend() at line %d in %s\n", __LINE__, __FILE__);
			return -1;
		}
	}
	else if (the_col->data_type == DATA_REAL)
	{
		if (writeFileDouble(col_data, APPEND_OFFSET, &the_data_real, file_opened_head, malloced_head, the_debug) != 0)
		{
			if (the_debug == YES_DEBUG)
				printf("	ERROR in insertAppend() at line %d in %s\n", __LINE__, __FILE__);
			return -1;
		}
	}
	else if (the_col->data_type == DATA_STRING)
	{
		if (writeFileCharData(col_data, APPEND_OFFSET, &the_col->max_length, the_data_string, file_opened_head, malloced_head, the_debug) != 0)
		{
			if (the_debug == YES_DEBUG)
				printf("	ERROR in insertAppend() at line %d in %s\n", __LINE__, __FILE__);
			return -1;
		}
	}
	// END Append data to col_data

	//printf("Here E\n");
	// START Increment num_rows in col_data_info
	the_col->num_rows++;

	if (writeFileInt(col_data_info, 0, &the_col->num_rows, file_opened_head, malloced_head, the_debug) != 0)
	{
		if (the_debug == YES_DEBUG)
			printf("	ERROR in insertAppend() at line %d in %s\n", __LINE__, __FILE__);
		return -1;
	}
	// END Increment num_rows in col_data_info

	//printf("Here F\n");

	return 0;
}

int insertOpen(FILE** col_data_info_file_arr, FILE** col_data_file_arr, struct file_opened_node** file_opened_head
			  ,int_8 the_table_number, struct table_cols_info* the_col
			  ,int_8 the_data_int_date, double the_data_real, char* the_data_string
			  ,struct malloced_node** malloced_head, int the_debug)
{
	// START Get last open row id from memory, free node in open list to reflect insertion
	int_8 last_open_id;
	if (the_col->open_list_before_tail != NULL)
	{
		//printf("Here A\n");
		//printf("the_col->open_list_before_tail = %d\n", the_col->open_list_before_tail->value);
		last_open_id = the_col->open_list_before_tail->next->value;
		//printf("Here Aa\n");

		struct ListNode* temp = the_col->open_list_before_tail->next;
		the_col->open_list_before_tail->next = NULL;
		myFree((void**) &temp, file_opened_head, malloced_head, the_debug);
		
		the_col->open_list_before_tail = the_col->open_list_before_tail->prev;
		//printf("now the_col->open_list_before_tail = %d\n", the_col->open_list_before_tail == NULL ? -1 : the_col->open_list_before_tail->value);
	}
	else
	{
		//printf("Here B\n");
		last_open_id = the_col->open_list_head->value;

		struct ListNode* temp = the_col->open_list_head;
		the_col->open_list_head = NULL;
		myFree((void**) &temp, file_opened_head, malloced_head, the_debug);
		//printf("now open_list_head is NULL\n");
	}
	//printf("last_open_id = %lu\n", last_open_id);
	// END Get last open row id from memory, free node in open list to reflect insertion

	the_col->num_open--;


	// START Find or open col_data file for table_number and col_number
	FILE* col_data = col_data_file_arr[the_col->col_number];
	// END Find or open col_data file for table_number and col_number


	// START Overwrite row data with new value in col_data
	//printf("New offset = %lu\n", ((8+the_col->max_length)*last_open_id)+8);
	if (the_col->data_type == DATA_INT || the_col->data_type == DATA_DATE)
	{
		//printf("Currently there = %lu\n", readFileInt(col_data, ((8+the_col->max_length)*last_open_id)+8), &file_opened_head, malloced_head, the_debug);
		/**/
		if (writeFileInt(col_data, ((8+the_col->max_length)*last_open_id)+8, &the_data_int_date, file_opened_head, malloced_head, the_debug) != 0)
		{
			if (the_debug == YES_DEBUG)
				printf("	ERROR in insertOpen() at line %d in %s\n", __LINE__, __FILE__);
			return -1;
		}
	}
	else if (the_col->data_type == DATA_REAL)
	{
		//printf("Currently there = %f\n", readFileDouble(col_data, ((8+the_col->max_length)*last_open_id)+8));
		/**/
		if (writeFileDouble(col_data, ((8+the_col->max_length)*last_open_id)+8, &the_data_real, file_opened_head, malloced_head, the_debug) != 0)
		{
			if (the_debug == YES_DEBUG)
				printf("	ERROR in insertOpen() at line %d in %s\n", __LINE__, __FILE__);
			return -1;
		}
	}
	else if (the_col->data_type == DATA_STRING)
	{
		//char* char_data = readFileCharData(&malloced_head, col_data, ((8+the_col->max_length)*last_open_id)+8, &the_col->max_length, the_debug);
		//if (char_data == NULL)
		//{
		//	if (the_debug == YES_DEBUG)
		//		printf("	ERROR in insertOpen() at line %d in %s\n", __LINE__, __FILE__);
		//	return -2;
		//}
		//printf("Currently there = %s\n", char_data);
		//myFree((void**) &char_data, &file_opened_head, malloced_head, the_debug);
		/**/
		if (writeFileCharData(col_data, ((8+the_col->max_length)*last_open_id)+8, &the_col->max_length, the_data_string, file_opened_head, malloced_head, the_debug) != 0)
		{
			if (the_debug == YES_DEBUG)
				printf("	ERROR in insertOpen() at line %d in %s\n", __LINE__, __FILE__);
			return -1;
		}
	}
	// END Overwrite row data with new value in col_data

	return 0;
}

int insertRows(struct table_info* the_table, struct change_node_v2* change_head, struct malloced_node** malloced_head, int the_debug)
{
	/*	Malloc the change_head grouping by column, sorted by col_number asc
		All column data for one column before all column data for the next
	*/
	int valid_rows_at_start = the_table->table_cols_head->num_rows - the_table->table_cols_head->num_open;

    struct file_opened_node* file_opened_head = NULL;

    // START Allocate space for file arrays
    FILE** col_data_info_file_arr = (FILE**) myMalloc(sizeof(FILE*) * the_table->num_cols, &file_opened_head, malloced_head, the_debug);
    //printf("malloced\n");
	FILE** col_data_file_arr = (FILE**) myMalloc(sizeof(FILE*) * the_table->num_cols, &file_opened_head, malloced_head, the_debug);
	//printf("malloced\n");
    if (col_data_info_file_arr == NULL || col_data_file_arr == NULL)
    {
        if (the_debug == YES_DEBUG)
			printf("	ERROR in insertRows() at line %d in %s\n", __LINE__, __FILE__);
        return -2;
    }

	for (int i=0; i<the_table->num_cols; i++)
	{
		col_data_info_file_arr[i] = NULL;
		col_data_file_arr[i] = NULL;
	}
	// END Allocate space for file arrays

    struct change_node_v2* cur_change = change_head;

	// START Traverse through column list
	struct table_cols_info* cur_col = the_table->table_cols_head;
	while (cur_col != NULL)
	{
		int inserted_open = 0;

		// START Call insert open if there exist open slots
		while (cur_change != NULL && cur_col->col_number == cur_change->col_number && cur_col->num_open > 0)
		{
			inserted_open = 1;

			if (col_data_file_arr[cur_col->col_number] == NULL)
				col_data_file_arr[cur_col->col_number] = myFileOpen("_Col_Data_", the_table->file_number, cur_col->col_number, "rb+", &file_opened_head, malloced_head, the_debug);

			//printf("change data = %s\n", cur_change->data);
			//printf("change column = %lu\n", cur_change->col_number);

			int_8 data_int_date = 0;
			double data_real = 0.0;
			char* data_string = NULL;

			if (cur_change->data == NULL)
			{
				if (cur_col->data_type == DATA_INT)
				{
					//printf("Here 2a\n");
					data_int_date = null_int;
					//printf("new data = %lu\n", data_int_date);
				}
				else if (cur_col->data_type == DATA_REAL)
				{
					//printf("Here 2b\n");
					data_real = null_double;
					//printf("new data = %f\n", data_real);
				}
				else if (cur_col->data_type == DATA_STRING)
				{
					//printf("Here 2c\n");
					data_string = (char*) myMalloc(sizeof(char) * cur_col->max_length, &file_opened_head, malloced_head, the_debug);
					if (data_string == NULL)
					{
						if (the_debug == YES_DEBUG)
							printf("	ERROR in insertRows() at line %d in %s\n", __LINE__, __FILE__);
						return -2;
					}
					//printf("malloced\n");
					strcpy(data_string, null_string);
					//printf("new data = %s\n", data_string);
				}
				else if (cur_col->data_type == DATA_DATE)
				{
					//printf("Here 2d\n");
					data_int_date = null_int;
					//printf("new data = %lu\n", data_int_date);
				}
			}
			else
			{
				if (cur_col->data_type == DATA_INT)
				{
					//printf("Here 2a\n");
					sscanf(cur_change->data, "%lu", &data_int_date);
					//printf("new data = %lu\n", data_int_date);
				}
				else if (cur_col->data_type == DATA_REAL)
				{
					//printf("Here 2b\n");
					sscanf(cur_change->data, "%f", &data_real);
					//printf("new data = %f\n", data_real);
				}
				else if (cur_col->data_type == DATA_STRING)
				{
					//printf("Here 2c\n");
					data_string = (char*) myMalloc(sizeof(char) * cur_col->max_length, &file_opened_head, malloced_head, the_debug);
					if (data_string == NULL)
					{
						if (the_debug == YES_DEBUG)
							printf("	ERROR in insertRows() at line %d in %s\n", __LINE__, __FILE__);
						return -2;
					}
					//printf("malloced\n");
					strcpy(data_string, cur_change->data);
					//printf("new data = %s\n", data_string);
				}
				else if (cur_col->data_type == DATA_DATE)
				{
					//printf("Here 2d\n");
					data_int_date = dateToInt(cur_change->data);
					//printf("new data = %lu\n", data_int_date);
				}
			}
				
			//printf("Here 1\n");
			if (insertOpen(col_data_info_file_arr, col_data_file_arr, &file_opened_head, the_table->file_number, cur_col
			  			  ,data_int_date, data_real, data_string, malloced_head, the_debug) != 0)
			{
				if (the_debug == YES_DEBUG)
					printf("	ERROR in insertRows() at line %d in %s\n", __LINE__, __FILE__);
		        return -1;
			}
			//printf("Here 2\n");
			
			if (data_string != NULL)
			{
				myFree((void**) &data_string, &file_opened_head, malloced_head, the_debug);
				//printf("freed\n");
			}

			cur_change = cur_change->next;
		}
		// END Call insert open if there exist open slots

		// START If rows inserted into open slots for this column, rewrite col_data_info
		if (inserted_open == 1)
		{
			//printf("Here 3\n");

			myFileClose(col_data_file_arr[cur_col->col_number], &file_opened_head, malloced_head, the_debug);
			col_data_file_arr[cur_col->col_number] = NULL;

			/**/
			// START Delete and rewrite col_data_info to reflect inserted rows
			char* file_name = (char*) myMalloc(sizeof(char) * 64, &file_opened_head, malloced_head, the_debug);
			//printf("malloced\n");
			if (file_name == NULL)
			{
				if (the_debug == YES_DEBUG)
					printf("	ERROR in insertRows() at line %d in %s\n", __LINE__, __FILE__);
				return -1;
			}
			concatFileName(file_name, "_Col_Data_Info_", the_table->file_number, cur_col->col_number);

			remove(file_name);
			myFree((void**) &file_name, &file_opened_head, malloced_head, the_debug);
			//printf("freed\n");

			//printf("Here 4\n");

			FILE* col_data_info = myFileOpen("_Col_Data_Info_", the_table->file_number, cur_col->col_number, "ab+", &file_opened_head, malloced_head, the_debug);
			if (col_data_info == NULL)
			{
				if (the_debug == YES_DEBUG)
					printf("	ERROR in insertRows() at line %d in %s\n", __LINE__, __FILE__);
				return -1;
			}

			writeFileInt(col_data_info, -1, &cur_col->num_rows, &file_opened_head, malloced_head, the_debug);
			writeFileInt(col_data_info, -1, &cur_col->num_open, &file_opened_head, malloced_head, the_debug);

			//printf("Here 5\n");

			struct ListNode* cur_open = cur_col->open_list_head;
			while (cur_open != NULL)
			{
				if (writeFileInt(col_data_info, -1, &cur_open->value, &file_opened_head, malloced_head, the_debug) != 0)
				{
					if (the_debug == YES_DEBUG)
						printf("	ERROR in insertRows() at line %d in %s\n", __LINE__, __FILE__);
					return -1;
				}

				cur_open = cur_open->next;
			}

			myFileClose(col_data_info, &file_opened_head, malloced_head, the_debug);
			// END Delete and rewrite col_data_info to reflect inserted rows

			//printf("Here 6\n");
		}
		// END If rows inserted into open slots for this column, rewrite col_data_info

		// START Insert append any other rows for this column
		col_data_info_file_arr[cur_col->col_number] = myFileOpen("_Col_Data_Info_", the_table->file_number, cur_col->col_number, "rb+", &file_opened_head, malloced_head, the_debug);
		col_data_file_arr[cur_col->col_number] = myFileOpen("_Col_Data_", the_table->file_number, cur_col->col_number, "ab+", &file_opened_head, malloced_head, the_debug);

		while (cur_change != NULL && cur_col->col_number == cur_change->col_number)
		{
			//printf("change data = _%s_\n", cur_change->data);

			int_8 data_int_date = 0;
			double data_real = 0.0;
			char* data_string = NULL;

			//printf("Here 1\n");

			if (cur_change->data == NULL)
			{
				//printf("Here 1 null\n");
				if (cur_col->data_type == DATA_INT)
				{
					//printf("Here 2a\n");
					data_int_date = null_int;
					//printf("new data = %lu\n", data_int_date);
				}
				else if (cur_col->data_type == DATA_REAL)
				{
					//printf("Here 2b\n");
					data_real = null_double;
					//printf("new data = %f\n", data_real);
				}
				else if (cur_col->data_type == DATA_STRING)
				{
					//printf("Here 2c\n");
					data_string = (char*) myMalloc(sizeof(char) * cur_col->max_length, &file_opened_head, malloced_head, the_debug);
					if (data_string == NULL)
					{
						if (the_debug == YES_DEBUG)
							printf("	ERROR in insertRows() at line %d in %s\n", __LINE__, __FILE__);
						return -2;
					}
					//printf("malloced\n");
					strcpy(data_string, null_string);
					//printf("new data = %s\n", data_string);
				}
				else if (cur_col->data_type == DATA_DATE)
				{
					//printf("Here 2d\n");
					data_int_date = null_int;
					//printf("new data = %lu\n", data_int_date);
				}
			}
			else
			{
				//printf("Here 1 NOT null\n");
				if (cur_col->data_type == DATA_INT)
				{
					//printf("Here 2a\n");
					sscanf(cur_change->data, "%lu", &data_int_date);
					//printf("new data = %lu\n", data_int_date);
				}
				else if (cur_col->data_type == DATA_REAL)
				{
					//printf("Here 2b\n");
					sscanf(cur_change->data, "%f", &data_real);
					//printf("new data = %f\n", data_real);
				}
				else if (cur_col->data_type == DATA_STRING)
				{
					//printf("Here 2c\n");
					data_string = (char*) myMalloc(sizeof(char) * cur_col->max_length, &file_opened_head, malloced_head, the_debug);
					if (data_string == NULL)
					{
						if (the_debug == YES_DEBUG)
							printf("	ERROR in insertRows() at line %d in %s\n", __LINE__, __FILE__);
						return -2;
					}
					//printf("malloced\n");
					strcpy(data_string, cur_change->data);
					//printf("new data = %s\n", data_string);
				}
				else if (cur_col->data_type == DATA_DATE)
				{
					//printf("Here 2d\n");
					data_int_date = dateToInt(cur_change->data);
					//printf("new data = %lu\n", data_int_date);
				}
			}

			//printf("Here 3\n");

			if (insertAppend(col_data_info_file_arr, col_data_file_arr, &file_opened_head, the_table->file_number, cur_col
							,data_int_date, data_real, data_string, malloced_head, the_debug) != 0)
			{
				if (the_debug == YES_DEBUG)
					printf("	ERROR in insertRows() at line %d in %s\n", __LINE__, __FILE__);
		        return -1;
			}
			
			if (data_string != NULL)
			{
				myFree((void**) &data_string, &file_opened_head, malloced_head, the_debug);
				//printf("freed\n");
			}

			//printf("Here 4\n");

			cur_change = cur_change->next;
		}
		// END Insert append any other rows for this column

		if (col_data_info_file_arr[cur_col->col_number] != NULL)
		{
			myFileClose(col_data_info_file_arr[cur_col->col_number], &file_opened_head, malloced_head, the_debug);
			col_data_info_file_arr[cur_col->col_number] = NULL;
		}

		if (col_data_file_arr[cur_col->col_number] != NULL)
		{
			myFileClose(col_data_file_arr[cur_col->col_number], &file_opened_head, malloced_head, the_debug);
			col_data_file_arr[cur_col->col_number] = NULL;
		}

		cur_col = cur_col->next;
	}

	//printf("Here 5\n");
	// END Traverse through column list
	myFree((void**) &col_data_info_file_arr, &file_opened_head, malloced_head, the_debug);
	//printf("freed\n");
	myFree((void**) &col_data_file_arr, &file_opened_head, malloced_head, the_debug);
	//printf("freed\n");

	//printf("Here 6\n");
	// START Cleanup
    if (file_opened_head != NULL)
    {
        if (the_debug == YES_DEBUG)
			printf("insertRows() did not close all files\n");
        myFileCloseAll(&file_opened_head, malloced_head, the_debug);
    }
    // END Cleanup

    //printf("Here 7\n");

    int valid_rows_at_end = the_table->table_cols_head->num_rows - the_table->table_cols_head->num_open;

    return valid_rows_at_end - valid_rows_at_start;
}

int deleteRows(struct table_info* the_table, struct or_clause_node* or_head, struct malloced_node** malloced_head, int the_debug)
{
	struct file_opened_node* file_opened_head = NULL;

	int_8 num_rows_in_result = 0;
	struct ListNode* valid_rows_head = findValidRowsGivenWhere(the_table, NULL, or_head, NULL, &num_rows_in_result, 0, malloced_head, the_debug);

	if (the_debug == YES_DEBUG)
		printf("num rows to be deleted = %lu\n", num_rows_in_result);

	if (num_rows_in_result > 0)
	{
		/**/
		// START Add deleted rows to disk
		struct table_cols_info* cur_col = the_table->table_cols_head;
		while (cur_col != NULL)
		{
			// START Append open row ids to col_data_info
			FILE* col_data_info = myFileOpen("_Col_Data_Info_", the_table->file_number, cur_col->col_number, "ab+", &file_opened_head, malloced_head, the_debug);
			if (col_data_info == NULL)
			{
				if (the_debug == YES_DEBUG)
					printf("	ERROR in deleteRows() at line %d in %s\n", __LINE__, __FILE__);
				return -1;
			}

			struct ListNode* cur_open = valid_rows_head;

			while (cur_open != NULL)
			{
				if (writeFileInt(col_data_info, -1, &cur_open->value, &file_opened_head, malloced_head, the_debug) != 0)
				{
					if (the_debug == YES_DEBUG)
						printf("	ERROR in deleteRows() at line %d in %s\n", __LINE__, __FILE__);
					return -1;
				}

				cur_col->num_open++;

				cur_open = cur_open->next;
			}

			myFileClose(col_data_info, &file_opened_head, malloced_head, the_debug);
			// END Append open row ids to col_data_info

			// START Overwrite new num_open value to col_data_info
			col_data_info = myFileOpen("_Col_Data_Info_", the_table->file_number, cur_col->col_number, "rb+", &file_opened_head, malloced_head, the_debug);

			if (writeFileInt(col_data_info, 8, &cur_col->num_open, &file_opened_head, malloced_head, the_debug) != 0)
			{
				if (the_debug == YES_DEBUG)
					printf("	ERROR in deleteRows() at line %d in %s\n", __LINE__, __FILE__);
				return -1;
			}

			myFileClose(col_data_info, &file_opened_head, malloced_head, the_debug);
			// END Overwrite new num_open value to col_data_info
			
			cur_col = cur_col->next;
		}
		// END Add deleted rows to disk


		// START Delete rows from col_data
		cur_col = the_table->table_cols_head;
		while (cur_col != NULL)
		{
			FILE* col_data = myFileOpen("_Col_Data_", the_table->file_number, cur_col->col_number, "rb+", &file_opened_head, malloced_head, the_debug);
			if (col_data == NULL)
			{
				if (the_debug == YES_DEBUG)
					printf("	ERROR in deleteRows() at line %d in %s\n", __LINE__, __FILE__);
				return -1;
			}

			struct ListNode* cur_open = valid_rows_head;
			while (cur_open != NULL)
			{
				//printf("cur_open = %d\n", cur_open->value);
				if (cur_col->data_type == DATA_INT || cur_col->data_type == DATA_DATE)
				{
					//printf("Currently there = %lu\n", readFileInt(col_data, ((8+8)*cur_open->value)+8), &file_opened_head, malloced_head, the_debug);
					if (writeFileInt(col_data, ((8+cur_col->max_length)*cur_open->value)+8, &open_int, &file_opened_head, malloced_head, the_debug) != 0)
					{
						if (the_debug == YES_DEBUG)
							printf("	ERROR in deleteRows() at line %d in %s\n", __LINE__, __FILE__);
						return -1;
					}
				}
				else if (cur_col->data_type == DATA_REAL)
				{
					//printf("Currently there = %f\n", readFileDouble(col_data, ((8+8)*cur_open->value)+8));
					if (writeFileDouble(col_data, ((8+cur_col->max_length)*cur_open->value)+8, &open_double, &file_opened_head, malloced_head, the_debug) != 0)
					{
						if (the_debug == YES_DEBUG)
							printf("	ERROR in deleteRows() at line %d in %s\n", __LINE__, __FILE__);
						return -1;
					}
				}
				else if (cur_col->data_type == DATA_STRING)
				{
					//char* char_data = readFileCharData(&malloced_head, col_data, ((8+cur_col->max_length)*cur_open->value)+8, &cur_col->max_length, the_debug);
					//if (char_data == NULL)
					//{
					//	if (the_debug == YES_DEBUG)
					//		printf("	ERROR in deleteRows() at line %d in %s\n", __LINE__, __FILE__);
					//	myFreeAllError(&malloced_head, the_debug);
					//	myFileCloseAll(&file_opened_head, the_debug);
					//	return -2;
					//}
					//printf("Currently there = %s\n", char_data);
					//myFree((void**) &char_data, &file_opened_head, malloced_head, the_debug);
					if (writeFileCharData(col_data, ((8+cur_col->max_length)*cur_open->value)+8, &cur_col->max_length, open_string, &file_opened_head, malloced_head, the_debug) != 0)
					{
						if (the_debug == YES_DEBUG)
							printf("	ERROR in deleteRows() at line %d in %s\n", __LINE__, __FILE__);
						return -1;
					}
				}
				cur_open = cur_open->next;
			}
			myFileClose(col_data, &file_opened_head, malloced_head, the_debug);

			cur_col = cur_col->next;
		}
		// END Delete rows from col_data


		// START Add deleted rows to memory
		cur_col = the_table->table_cols_head;
		while (cur_col != NULL)
		{
			struct ListNode* cur_open = valid_rows_head;

			struct ListNode* open_tail;

			if (cur_col->open_list_head == NULL)
			{
				cur_col->open_list_head = (struct ListNode*) myMalloc(sizeof(struct ListNode), &file_opened_head, malloced_head, the_debug);
				if (cur_col->open_list_head == NULL)
				{
					if (the_debug == YES_DEBUG)
						printf("	ERROR in deleteRows() at line %d in %s\n", __LINE__, __FILE__);
					return -2;
				}
				cur_col->open_list_head->value = cur_open->value;
				cur_col->open_list_head->next = NULL;
				cur_col->open_list_head->prev = NULL;

				cur_open = cur_open->next;
				open_tail = cur_col->open_list_head;
			}
			else
			{
				if (cur_col->open_list_head->next == NULL)
					open_tail = cur_col->open_list_head;
				else
					open_tail = cur_col->open_list_before_tail->next;
			}
			
			while (cur_open != NULL)
			{
				cur_col->open_list_before_tail = open_tail;

				open_tail->next = (struct ListNode*) myMalloc(sizeof(struct ListNode), &file_opened_head, malloced_head, the_debug);
				if (cur_col->open_list_head == NULL)
				{
					if (the_debug == YES_DEBUG)
						printf("	ERROR in deleteRows() at line %d in %s\n", __LINE__, __FILE__);
					return -2;
				}
				open_tail->next->value = cur_open->value;
				open_tail->next->next = NULL;
				open_tail->next->prev = open_tail;
				open_tail = open_tail->next;

				cur_open = cur_open->next;
			}
			cur_col = cur_col->next;
		}
		// END Add deleted rows to memory

		// START Free returned linked list
		int freed = freeListNodes(&valid_rows_head, &file_opened_head, malloced_head, the_debug);
		if (the_debug == YES_DEBUG)
			printf("	Freed %d ListNodes from findValidRowsGivenWhere()\n", freed);
		// END Free returned linked list
	}


	// Just free the malloced_ndes for the open_list because those persist
	struct table_cols_info* cur_col = the_table->table_cols_head;
	while (cur_col != NULL)
	{
		struct ListNode* cur = cur_col->open_list_head;
		while (cur != NULL)
		{
			myFreeJustNode((void**) &cur, &file_opened_head, malloced_head, the_debug);

			cur = cur->next;
		}
		cur_col = cur_col->next;
	}


	// START If table has no valid rows, delete data files and recreate them to regain space
	if (the_table->table_cols_head->num_open == the_table->table_cols_head->num_rows)
	{
		struct table_cols_info* cur_col = the_table->table_cols_head;
		while (cur_col != NULL)
		{
			char* file_name = (char*) myMalloc(sizeof(char) * 100, &file_opened_head, malloced_head, the_debug);
			if (file_name == NULL)
			{
				if (the_debug == YES_DEBUG)
					printf("	ERROR in deleteRows() at line %d in %s\n", __LINE__, __FILE__);
				return -2;
			}

			concatFileName(file_name, "_Col_Data_", the_table->file_number, cur_col->col_number);

			//printf("file_name = _%s_\n", file_name);

			remove(file_name);


			strcpy(file_name, "");

			concatFileName(file_name, "_Col_Data_Info_", the_table->file_number, cur_col->col_number);

			//printf("file_name = _%s_\n", file_name);

			remove(file_name);

			myFree((void**) &file_name, &file_opened_head, malloced_head, the_debug);

			cur_col->num_rows = 0;
			cur_col->num_open = 0;


			// DONT Pass malloced_head because those malloced nodes were freed so need to be freed with free()
			int freed = freeListNodes(&cur_col->open_list_head, NULL, NULL, the_debug);
			cur_col->open_list_head = NULL;
			cur_col->open_list_before_tail = NULL;
			if (the_debug == YES_DEBUG) 
				printf("	Freed %d from open_list_head\n", freed);


			FILE* col_data_info = myFileOpen("_Col_Data_Info_", the_table->file_number, cur_col->col_number, "ab+", &file_opened_head, malloced_head, the_debug);
			if (col_data_info == NULL)
			{
				if (the_debug == YES_DEBUG)
					printf("	ERROR in deleteRows() at line %d in %s\n", __LINE__, __FILE__);
				return -1;
			}

			if (writeFileInt(col_data_info, -1, &cur_col->num_rows, &file_opened_head, malloced_head, the_debug) == -1)
			{
				if (the_debug == YES_DEBUG)
					printf("	ERROR in deleteRows() at line %d in %s\n", __LINE__, __FILE__);
				return -1;
			}

			if (writeFileInt(col_data_info, -1, &cur_col->num_open, &file_opened_head, malloced_head, the_debug) == -1)
			{
				if (the_debug == YES_DEBUG)
					printf("	ERROR in deleteRows() at line %d in %s\n", __LINE__, __FILE__);
				return -1;
			}

			myFileClose(col_data_info, &file_opened_head, malloced_head, the_debug);


			FILE* col_data = myFileOpen("_Col_Data_", the_table->file_number, cur_col->col_number, "ab+", &file_opened_head, malloced_head, the_debug);
			if (col_data == NULL)
			{
				if (the_debug == YES_DEBUG)
					printf("	ERROR in deleteRows() at line %d in %s\n", __LINE__, __FILE__);
				return -1;
			}
			myFileClose(col_data, &file_opened_head, malloced_head, the_debug);


			cur_col = cur_col->next;
		}
	}
	// END If table has no valid rows, delete data files and recreate them to regain space


	if (file_opened_head != NULL)
	{
		if (the_debug == YES_DEBUG)
			printf("deleteRows() did not close all files\n");
		myFileCloseAll(&file_opened_head, malloced_head, the_debug);
	}


	/*
	printf("At the traverse\n");
	cur_col = the_table->table_cols_head;
	while (cur_col != NULL)
	{
		printf("Col: %s\n", cur_col->col_name);
		struct ListNode* cur = cur_col->open_list_head;
		while (cur != NULL)
		{
			printf("%d,", cur->value);
			cur = cur->next;
		}
		printf("\ncur_col->open_list_before_tail = %d\n", cur_col->open_list_before_tail == NULL ? -1 : cur_col->open_list_before_tail->value);

		cur_col = cur_col->next;
	}*/
	

	return num_rows_in_result;
}

int updateRows(struct table_info* the_table, struct change_node_v2* change_head, struct or_clause_node* or_head, struct malloced_node** malloced_head, int the_debug)
{
	struct file_opened_node* file_opened_head = NULL;

	int_8 num_rows_in_result = 0;
	struct ListNode* valid_rows_head = findValidRowsGivenWhere(the_table, NULL, or_head, NULL, &num_rows_in_result, 0, malloced_head, the_debug);

	if (num_rows_in_result > 0)
	{
		struct change_node_v2* cur_change = change_head;
		while (cur_change != NULL)
		{
			//printf("cur_change = %s\n", cur_change->data);

			struct table_cols_info* cur_col = the_table->table_cols_head;
			while (cur_col != NULL)
			{
				bool found_col = false;
				if (cur_change->col_number == cur_col->col_number)
				{
					found_col = true;
					//printf("cur_col = %s\n", cur_col->col_name);

					FILE* col_data = myFileOpen("_Col_Data_", the_table->file_number, cur_col->col_number, "rb+", &file_opened_head, malloced_head, the_debug);
					if (col_data == NULL)
					{
						if (the_debug == YES_DEBUG)
							printf("	ERROR in updateRows() at line %d in %s\n", __LINE__, __FILE__);
						return -1;
					}

					
					struct ListNode* cur_valid = valid_rows_head;
					while (cur_valid != NULL)
					{
						if (cur_col->data_type == DATA_INT)
						{
							int_8 the_data_int;
							sscanf(cur_change->data, "%lu", &the_data_int);

							//printf("Currently there = %lu\n", readFileInt(col_data, ((8+8)*cur_valid->value)+8), &file_opened_head, malloced_head, the_debug);
							if (writeFileInt(col_data, ((8+cur_col->max_length)*cur_valid->value)+8, &the_data_int, &file_opened_head, malloced_head, the_debug) != 0)
							{
								if (the_debug == YES_DEBUG)
									printf("	ERROR in updateRows() at line %d in %s\n", __LINE__, __FILE__);
								return -1;
							}
						}
						else if (cur_col->data_type == DATA_REAL)
						{
							double the_data_real;
							sscanf(cur_change->data, "%f", &the_data_real);

							//printf("Currently there = %f\n", readFileDouble(col_data, ((8+8)*cur_valid->value)+8));
							if (writeFileDouble(col_data, ((8+cur_col->max_length)*cur_valid->value)+8, &the_data_real, &file_opened_head, malloced_head, the_debug) != 0)
							{
								if (the_debug == YES_DEBUG)
									printf("	ERROR in updateRows() at line %d in %s\n", __LINE__, __FILE__);
								return -1;
							}
						}
						else if (cur_col->data_type == DATA_STRING)
						{
							//char* char_data = readFileCharData(&malloced_head, col_data, ((8+cur_col->max_length)*cur_valid->value)+8, &cur_col->max_length, the_debug);
							//if (char_data == NULL)
							//{
							//	if (the_debug == YES_DEBUG)
							//		printf("	ERROR in deleteRows() at line %d in %s\n", __LINE__, __FILE__);
							//	myFreeAllError(&malloced_head, the_debug);
							//	myFileCloseAll(&file_opened_head, the_debug);
							//	return -1;
							//}
							//printf("Currently there = %s\n", char_data);
							//myFree((void**) &char_data, &file_opened_head, malloced_head, the_debug);
							if (writeFileCharData(col_data, ((8+cur_col->max_length)*cur_valid->value)+8, &cur_col->max_length, cur_change->data
												 ,&file_opened_head, malloced_head, the_debug) != 0)
							{
								if (the_debug == YES_DEBUG)
									printf("	ERROR in updateRows() at line %d in %s\n", __LINE__, __FILE__);
								return -1;
							}
						}
						else if (cur_col->data_type == DATA_DATE)
						{
							int_8 the_data_date = dateToInt(cur_change->data);

							//printf("Currently there = %lu\n", readFileInt(col_data, ((8+8)*cur_valid->value)+8), &file_opened_head, malloced_head, the_debug);
							if (writeFileInt(col_data, ((8+cur_col->max_length)*cur_valid->value)+8, &the_data_date, &file_opened_head, malloced_head, the_debug) != 0)
							{
								if (the_debug == YES_DEBUG)
									printf("	ERROR in updateRows() at line %d in %s\n", __LINE__, __FILE__);
								return -1;
							}
						}

						cur_valid = cur_valid->next;
					}

					myFileClose(col_data, &file_opened_head, malloced_head, the_debug);
				}

				if (!found_col)
					cur_col = cur_col->next;
				else
					cur_col = NULL;
			}

			cur_change = cur_change->next;
		}

		int valid_freed = freeListNodes(&valid_rows_head, &file_opened_head, malloced_head, the_debug);
		if (the_debug == YES_DEBUG)
			printf("Freed %d from valid_rows_head\n", valid_freed);
	}

	if (file_opened_head != NULL)
	{
		if (the_debug == YES_DEBUG)
			printf("updateRows() did not close all files\n");
		myFileCloseAll(&file_opened_head, malloced_head, the_debug);
	}

	return num_rows_in_result;
}

struct colDataNode** getAllColData(int_8 table_number, struct table_cols_info* the_col, struct malloced_node** malloced_head, int the_debug)
{
	struct file_opened_node* file_opened_head = NULL;

	// START Allocate memory for all rows in data file
	struct colDataNode** arr = (struct colDataNode**) myMalloc(sizeof(struct colDataNode*) * the_col->num_rows, &file_opened_head, malloced_head, the_debug);
	if (arr == NULL)
	{
		if (the_debug == YES_DEBUG)
			printf("	ERROR in getAllColData() at line %d in %s\n", __LINE__, __FILE__);
		return NULL;
	}
	for (int i=0; i<the_col->num_rows; i++)
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
	int_8 rows_offset = 0;
	for (int i=0; i<the_col->num_rows; i++)
	{
		arr[i]->row_id = readFileInt(col_data, rows_offset, &file_opened_head, malloced_head, the_debug);
		rows_offset += 8;

		if (the_col->data_type == DATA_INT)
		{
			arr[i]->row_data = (char*) myMalloc(sizeof(char) * 48, &file_opened_head, malloced_head, the_debug);
			if (arr[i]->row_data == NULL)
			{
				if (the_debug == YES_DEBUG)
					printf("	ERROR in getAllColData() at line %d in %s\n", __LINE__, __FILE__);
				return NULL;
			}

			int_8 read_int = readFileInt(col_data, rows_offset, &file_opened_head, malloced_head, the_debug);
			//printf("read_int = %lu\n", read_int);

			if (read_int == null_int)
				strcpy(arr[i]->row_data, "");
			else
				sprintf(arr[i]->row_data, "%d", read_int);
		}
		else if (the_col->data_type == DATA_REAL)
		{
			arr[i]->row_data = (char*) myMalloc(sizeof(char) * 48, &file_opened_head, malloced_head, the_debug);
			if (arr[i]->row_data == NULL)
			{
				if (the_debug == YES_DEBUG)
					printf("	ERROR in getAllColData() at line %d in %s\n", __LINE__, __FILE__);
				return NULL;
			}

			double read_double = readFileDouble(col_data, rows_offset, &file_opened_head, malloced_head, the_debug);
			//printf("read_double = %f\n", read_double);

			if (read_double == null_double)
				strcpy(arr[i]->row_data, "");
			else
				sprintf(arr[i]->row_data, "%f", read_double);
		}
		else if (the_col->data_type == DATA_STRING)
		{
			arr[i]->row_data = readFileCharData(col_data, rows_offset, &the_col->max_length, &file_opened_head, malloced_head, the_debug);
			//printf("read_string = %s\n", arr[i]->row_data);

			if (arr[i]->row_data == NULL)
			{
				if (the_debug == YES_DEBUG)
					printf("	ERROR in getAllColData() at line %d in %s\n", __LINE__, __FILE__);
				return NULL;
			}

			if (strcmp(arr[i]->row_data, null_string) == 0)
				strcpy(arr[i]->row_data, "");
		}
		else if (the_col->data_type == DATA_DATE)
		{
			int_8 read_int = readFileInt(col_data, rows_offset, &file_opened_head, malloced_head, the_debug);
			//printf("read_int = %lu\n", read_int);

			if (read_int == null_int)
			{
				arr[i]->row_data = (char*) myMalloc(sizeof(char) * 1, &file_opened_head, malloced_head, the_debug);
				if (arr[i]->row_data == NULL)
				{
					if (the_debug == YES_DEBUG)
						printf("	ERROR in getAllColData() at line %d in %s\n", __LINE__, __FILE__);
					return NULL;
				}

				arr[i]->row_data[0] = 0;
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
		}
		//printf("arr[i]->row_data = _%s_\n", arr[i]->row_data);

		rows_offset += the_col->max_length;
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

struct ListNode* findValidRowsGivenWhere(struct table_info* the_table, struct colDataNode*** table_data_arr, struct or_clause_node* or_head
										,int_8* the_col_numbers, int_8* num_rows_in_result, int the_col_numbers_size, struct malloced_node** malloced_head, int the_debug)
{
	// START Allocate array for valid rows
	int* valid_arr = (int*) myMalloc(sizeof(int) * the_table->table_cols_head->num_rows, NULL, malloced_head, the_debug);
	if (valid_arr == NULL)
	{
		if (the_debug == YES_DEBUG)
			printf("	ERROR in findValidRowsGivenWhere() at line %d in %s\n", __LINE__, __FILE__);
		return NULL;
	}
	for (int i=0; i<the_table->table_cols_head->num_rows; i++)
	{
		if (or_head == NULL)
			valid_arr[i] = 1;
		else
			valid_arr[i] = -2;
	}
	// END Allocate array for valid rows

	// START Mark open rows as invalid
	struct ListNode* cur_open = the_table->table_cols_head->open_list_head;
	while (cur_open != NULL)
	{
		//printf("row_id %lu is open\n", cur_open->value);

		valid_arr[cur_open->value] = -1;

		cur_open = cur_open->next;
	}
	// END Mark open rows as invalid

	// START Traverse or and and nodes, finding valid rows
	struct or_clause_node* cur_or = or_head;
	int cur_or_index = -3;
	while (cur_or != NULL)
	{
		//printf("new or\n");
		//printf("cur_or_index = %d\n", cur_or_index);
		struct and_clause_node* cur_and = cur_or->and_head;
		while (cur_and != NULL)
		{
			//printf("	new and\n");
			//printf("	col_number = %lu\n", cur_and->col_number);
			//printf("	data_string = %s\n", cur_and->data_string);
			
			// START Find j value of cur_and->col_number in the_col_numbers
			int_8 j_value = -1;
			for (int j=0; j<the_col_numbers_size; j++)
			{
				if (the_col_numbers[j] == cur_and->col_number)
				{
					j_value = j;
					break;
				}
			}
			// END Find j value of cur_and->col_number in the_col_numbers

			// START Get all col data given j_value
			struct colDataNode** col_data_arr_to_test;
			if (j_value == -1)
			{
				if (the_debug == YES_DEBUG)
					printf("	j_value is -1, getting more col data\n");
				struct table_cols_info* cur_col = the_table->table_cols_head;
				while (cur_col != NULL)
				{
					if (cur_col->col_number == cur_and->col_number)
						break;
					cur_col = cur_col->next;
				}
				col_data_arr_to_test = getAllColData(the_table->file_number, cur_col, malloced_head, the_debug);
				if (col_data_arr_to_test == NULL)
				{
					if (the_debug == YES_DEBUG)
						printf("	ERROR in findValidRowsGivenWhere() at line %d in %s\n", __LINE__, __FILE__);
					return NULL;
				}
			}
			else
			{
				col_data_arr_to_test = table_data_arr[j_value];
			}
			// END Get all col data given j_value

			//printf("	Starting col data search\n");

			// START
			for (int i=0; i<the_table->table_cols_head->num_rows; i++)
			{
				if (valid_arr[i] != -1)
				{
					//printf("		cur_and->data_string = %s and row_data = %s\n", cur_and->data_string, col_data_arr_to_test[i]->row_data);
					if (strcmp(cur_and->data_string, col_data_arr_to_test[i]->row_data) == 0)
					{
						//printf("	Row data matches at %s\n", col_data_arr_to_test[i]->row_data);
						if (cur_and->where_type == WHERE_IS_EQUALS)
						{
							if (valid_arr[i] > cur_or_index)
							{
								//printf("	Here Aa\n");
								valid_arr[i] = abs(cur_or_index); // true
							}
							else
							{
								//printf("	Here Ab\n");
								valid_arr[i] = cur_or_index; // false
							}
						}
						else if (cur_and->where_type == WHERE_NOT_EQUALS)
						{
							if (valid_arr[i] < abs(cur_or_index) && valid_arr[i] > 0)
							{
								//printf("	Here Ba\n");
								//valid_arr[i] = abs(cur_or_index); // true
							}
							else
							{
								//printf("	Here Bb\n");
								valid_arr[i] = cur_or_index; // false
							}
						}
					}
					else
					{
						//printf("	Row data DOES NOT match\n");
						if (cur_and->where_type == WHERE_IS_EQUALS)
						{
							if (valid_arr[i] < abs(cur_or_index) && valid_arr[i] > 0)
							{
								//printf("	Here Ca\n");
								//valid_arr[i] = abs(cur_or_index); // true
							}
							else
							{
								//printf("	Here Cb\n");
								valid_arr[i] = cur_or_index; // false
							}
						}
						else if (cur_and->where_type == WHERE_NOT_EQUALS)
						{
							if (valid_arr[i] > cur_or_index)
							{
								//printf("	Here Da\n");
								valid_arr[i] = abs(cur_or_index); // true
							}
							else
							{
								//printf("	Here Db\n");
								valid_arr[i] = cur_or_index; // false
							}
						}
					}
				}
			}
			// END


			// START Free if had to get more col data
			if (j_value == -1)
			{
				if (the_debug == YES_DEBUG)
					printf("	j_value is -1, freeing extra col data\n");

				int_8 total_freed = 0;
				for (int i=0; i<the_table->table_cols_head->num_rows; i++)
				{
					myFree((void**) &(col_data_arr_to_test[i]->row_data), NULL, malloced_head, the_debug);
					myFree((void**) &col_data_arr_to_test[i], NULL, malloced_head, the_debug);
					total_freed+=2;
				}
				myFree((void**) &col_data_arr_to_test, NULL, malloced_head, the_debug);
				total_freed++;
				if (the_debug == YES_DEBUG)
					printf("		Freed %lu things from extra getAllColData()\n", total_freed);
			}
			// END Free if had to get more col data

			cur_and = cur_and->next;
		}
		cur_or_index--;
		cur_or = cur_or->next;
	}	
	// END Traverse or and and nodes, finding valid rows

	// START Create linked list for valid rows given valid_arr
	struct ListNode* valid_rows_head = NULL;
	struct ListNode* valid_rows_tail = NULL;

	int_8 total_freed = 0;
	for (int i=0; i<the_table->table_cols_head->num_rows; i++)
	{
		if (valid_arr[i] > 0)
		{
			//printf("row_id %d is valid\n", i);
			if (addListNode(&valid_rows_head, &valid_rows_tail, i, ADDLISTNODE_TAIL, NULL, malloced_head, the_debug) != 0)
			{
				if (the_debug == YES_DEBUG)
					printf("	ERROR in findValidRowsGivenWhere() at line %d in %s\n", __LINE__, __FILE__);
				return NULL;
			}

			*num_rows_in_result = *num_rows_in_result + 1;
		}
		else
		{
			for (int j=0; j<the_col_numbers_size; j++)
			{
				myFree((void**) &(table_data_arr[j][i]->row_data), NULL, malloced_head, the_debug);
				total_freed++;
			}
			//printf("row_id %d is NOT valid, freed %d row_datas\n", i, freed);
		}
	}
	if (the_debug == YES_DEBUG)
		printf("	Freed %lu invalid row_datas\n", total_freed);

	myFree((void**) &valid_arr, NULL, malloced_head, the_debug);
	// END Create linked list for valid rows given valid_arr


	return valid_rows_head;
}

char*** select(struct table_info* the_table, int_8* the_col_numbers, int the_col_numbers_size, int_8* num_rows_in_result, struct or_clause_node* or_head
			  ,struct malloced_node** malloced_head, int the_debug)
{
	struct file_opened_node* file_opened_head = NULL;

	// START Pull all column data from disk
	int_8* data_type_arr = (int_8*) myMalloc(sizeof(int_8) * the_col_numbers_size, &file_opened_head, malloced_head, the_debug);
	struct colDataNode*** table_data_arr = (struct colDataNode***) myMalloc(sizeof(struct colDataNode**) * the_col_numbers_size, &file_opened_head, malloced_head, the_debug);
	if (data_type_arr == NULL || table_data_arr == NULL)
	{
		if (the_debug == YES_DEBUG)
		{
			printf("	ERROR in select() at line %d in %s\n", __LINE__, __FILE__);
			printf("	ERROR %s is NULL\n", data_type_arr == NULL ? "data_type_arr" : "table_data_arr");
		}
		return NULL;
	}
	for (int j=0; j<the_col_numbers_size; j++)
	{
		struct table_cols_info* cur_col = the_table->table_cols_head;
		while (cur_col != NULL)
		{
			if (cur_col->col_number == the_col_numbers[j])
				break;
			cur_col = cur_col->next;
		}
		data_type_arr[j] = cur_col->data_type;
		
		table_data_arr[j] = getAllColData(the_table->file_number, cur_col, malloced_head, the_debug);
		if (table_data_arr[j] == NULL)
		{
			if (the_debug == YES_DEBUG)
				printf("	ERROR in select() at line %d in %s\n", __LINE__, __FILE__);
			return NULL;
		}
	}
	myFree((void**) &data_type_arr, &file_opened_head, malloced_head, the_debug);
	// END Pull all column data from disk

	// START Get valid rows given where clause
	struct ListNode* valid_rows_head = findValidRowsGivenWhere(the_table, table_data_arr, or_head, the_col_numbers, num_rows_in_result, the_col_numbers_size
															  ,malloced_head, the_debug);
	//printf("*num_rows_in_result = %d\n", (*num_rows_in_result));
	// END Get valid rows given where clause

	// START Allocate space for returned array and copy pointers
	char*** result_arr = (char***) myMalloc(sizeof(char**) * the_col_numbers_size, &file_opened_head, malloced_head, the_debug);
	if (result_arr == NULL)
	{
		if (the_debug == YES_DEBUG)
			printf("	ERROR in select() at line %d in %s\n", __LINE__, __FILE__);
		return NULL;
	}
	for (int j=0; j<the_col_numbers_size; j++)
	{
		result_arr[j] = (char**) myMalloc(sizeof(char*) * (*num_rows_in_result), &file_opened_head, malloced_head, the_debug);
		if (result_arr[j] == NULL)
		{
			if (the_debug == YES_DEBUG)
				printf("	ERROR in select() at line %d in %s\n", __LINE__, __FILE__);
			return NULL;
		}
		struct ListNode* cur = valid_rows_head;
		for (int i=0; i<(*num_rows_in_result); i++)
		{
			result_arr[j][i] = table_data_arr[j][cur->value]->row_data;
			//printf("result_arr[j][i] = %s\n", result_arr[j][i]);
			cur = cur->next;
		}
		//myFree((void**) &result_arr[j], &file_opened_head, malloced_head, the_debug);
	}
	//myFree((void**) &result_arr, &file_opened_head, malloced_head, the_debug);
	// END Allocate space for returned array and copy pointers

	// START Cleanup
	int_8 total_freed = 0;

	freeListNodes(&valid_rows_head, &file_opened_head, malloced_head, the_debug);
	if (valid_rows_head != NULL)
		printf("	ERROR in select() at line %d in %s\n", __LINE__, __FILE__);

	total_freed += *num_rows_in_result;
	if (the_debug == YES_DEBUG)
		printf("	Freed %lu things from findValidRowsGivenWhere()\n", total_freed);

	total_freed = 0;
	for (int j=0; j<the_col_numbers_size; j++)
    {
    	for (int i=0; i<the_table->table_cols_head->num_rows; i++)
		{
			myFree((void**) &table_data_arr[j][i], NULL, malloced_head, the_debug);
			total_freed++;
		}
		myFree((void**) &table_data_arr[j], NULL, malloced_head, the_debug);
		total_freed++;
    }
    if (the_debug == YES_DEBUG)
		printf("	Freed %lu things from getAllColData()\n", total_freed);

	myFree((void**) &table_data_arr, &file_opened_head, malloced_head, the_debug);


	/*if (the_debug == YES_DEBUG)
		printf("Calling myFreeAllCleanup() from select()\n");
	myFreeAllCleanup(malloced_head, the_debug);*/

    if (file_opened_head != NULL)
    {
        if (the_debug == YES_DEBUG)
			printf("select() did not close all files\n");
        myFileCloseAll(&file_opened_head, malloced_head, the_debug);
    }
    // END Cleanup

    return result_arr;
}

int traverseTablesInfoDisk(struct malloced_node** malloced_head, int the_debug)
{
	struct file_opened_node* file_opened_head = NULL;

	FILE* db_info = myFileOpen("_Info", -1, -1, "rb+", &file_opened_head, malloced_head, the_debug);
	if (db_info == NULL)
	{
		if (the_debug == YES_DEBUG)
			printf("	ERROR in traverseTablesInfoDisk() at line %d in %s\n", __LINE__, __FILE__);
		return -1;
	}

	int_8 temp_num_tables = readFileInt(db_info, 0, &file_opened_head, malloced_head, the_debug);
	printf("\nDATA FILES ON DISK\n\nnum_tables = %lu\n\n", temp_num_tables);

	int cur_offset = 8;
	char* temp_table_name;
	while ((temp_table_name = readFileChar(db_info, cur_offset, &file_opened_head, malloced_head, the_debug)) != NULL)
	{
		printf("Table name = %s\n", temp_table_name);
		myFree( (void**) &temp_table_name, &file_opened_head, malloced_head, the_debug);
		cur_offset += 32;
		int_8 table_number = readFileInt(db_info, cur_offset, &file_opened_head, malloced_head, the_debug);
		printf("Table number = %lu\n", table_number);
		cur_offset += 8;

		FILE* tab_col = myFileOpen("_Tab_Col_", table_number, 0, "rb+\0", &file_opened_head, malloced_head, the_debug);
		if (tab_col == NULL)
		{
			if (the_debug == YES_DEBUG)
				printf("	ERROR in traverseTablesInfoDisk() at line %d in %s\n", __LINE__, __FILE__);
			return -1;
		}
		
		int cur_col_offset = 8;
		char* temp_col_name;
		while ((temp_col_name = readFileChar(tab_col, cur_col_offset, &file_opened_head, malloced_head, the_debug)) != NULL)
		{
			printf("	Column name = %s\n", temp_col_name);
			myFree((void**) &temp_col_name, &file_opened_head, malloced_head, the_debug);
			cur_col_offset += 32;

			int_8 temp_data = readFileInt(tab_col, cur_col_offset, &file_opened_head, malloced_head, the_debug);
			int_8 temp_data_type = temp_data & 0xff;
			int_8 temp_max_length = temp_data >> 8;

			printf("	Datatype = %lu\n", temp_data_type);
			printf("	Max length = %lu\n", temp_max_length);
			cur_col_offset += 8;
			int_8 col_number = readFileInt(tab_col, cur_col_offset, &file_opened_head, malloced_head, the_debug);
			printf("	Column number = %lu\n", col_number);
			cur_col_offset += 8;

			FILE* col_data_info = myFileOpen("_Col_Data_Info_", table_number, col_number, "rb+\0", &file_opened_head, malloced_head, the_debug);
			if (col_data_info == NULL)
			{
				if (the_debug == YES_DEBUG)
					printf("	ERROR in traverseTablesInfoDisk() at line %d in %s\n", __LINE__, __FILE__);
				return -1;
			}

			printf("	Number of rows = %lu\n", readFileInt(col_data_info, 0, &file_opened_head, malloced_head, the_debug));
			printf("	Number of open slots = %lu\n", readFileInt(col_data_info, 8, &file_opened_head, malloced_head, the_debug));

			printf("	Open slots: ");
			int_8 open_offset = 16;
			int_8 open_slot;
			while ((open_slot = readFileInt(col_data_info, open_offset, &file_opened_head, malloced_head, the_debug)) != -1)
			{
				printf("%lu, ", open_slot);
				open_offset += 8;
			}
			printf("\n");

			myFileClose(col_data_info, &file_opened_head, malloced_head, the_debug);

			/**/
			FILE* col_data = myFileOpen("_Col_Data_", table_number, col_number, "rb+\0", &file_opened_head, malloced_head, the_debug);
			if (col_data == NULL)
			{
				if (the_debug == YES_DEBUG)
					printf("	ERROR in traverseTablesInfoDisk() at line %d in %s\n", __LINE__, __FILE__);
				return -1;
			}

			printf("	Column data:\n");
			int_8 rows_offset = 0;
			int_8 row_id;
			while ((row_id = readFileInt(col_data, rows_offset, &file_opened_head, malloced_head, the_debug)) != -1)
			{
				printf("		%lu	", row_id);
				rows_offset += 8;

				if (temp_data_type == DATA_INT || temp_data_type == DATA_DATE)
					printf("%lu\n", readFileInt(col_data, rows_offset, &file_opened_head, malloced_head, the_debug));
				else if (temp_data_type == DATA_REAL)
					printf("%f\n", readFileDouble(col_data, rows_offset, &file_opened_head, malloced_head, the_debug));
				else if (temp_data_type == DATA_STRING)
				{
					char* temp_str = readFileCharData(col_data, rows_offset, &temp_max_length, &file_opened_head, malloced_head, the_debug);
					printf("%s\n", temp_str);
					myFree((void**) &temp_str, &file_opened_head, malloced_head, the_debug);
				}
				rows_offset += temp_max_length;
			}
			myFileClose(col_data, &file_opened_head, malloced_head, the_debug);
		}
		myFileClose(tab_col, &file_opened_head, malloced_head, the_debug);
	}
	myFileClose(db_info, &file_opened_head, malloced_head, the_debug);


    if (file_opened_head != NULL)
    {
        if (the_debug == YES_DEBUG)
			printf("traverseTablesInfoDisk() did not close all files\n");
        myFileCloseAll(&file_opened_head, malloced_head, the_debug);
    }

	return 0;
}

int freeMemOfDB(int the_debug)
{
	/*struct table_info* cur_table = tables_head;
	while (cur_table != NULL)
	{
		if (cur_table->change_list_head != NULL)
		{
			printf("\nThere are uncommitted changes.\nCommit them with \"commit;\" or hit enter to erase those changes.\n\n");
			char input[10];
			scanf("%[^\n]", input);
			input[7] = 0;

			if (strcmp(input, "commit;") == 0)
            {
                //commit();
            }
			else
			{
				printf("Not committing, unsaved changes will be erased.\n");
				break;
			}
		}

		cur_table = cur_table->next;
	}*/

	int total_freed = 0;
	while (tables_head != NULL)
	{
		while (tables_head->table_cols_head != NULL)
		{
			freeListNodes(&tables_head->table_cols_head->open_list_head, NULL, NULL, the_debug);
			if (tables_head->table_cols_head->open_list_head != NULL)
				printf("	ERROR in freeMemOfDB() at line %d in %s\n", __LINE__, __FILE__);

			total_freed += tables_head->table_cols_head->num_open;

			struct table_cols_info* temp = tables_head->table_cols_head;
			tables_head->table_cols_head = tables_head->table_cols_head->next;
			free(temp->col_name);
			free(temp);
			total_freed+=2;
		}

		while (tables_head->change_list_head != NULL)
		{
			struct change_node* temp = tables_head->change_list_head;
			tables_head->change_list_head = tables_head->change_list_head->next;
			free(temp->data_string);
			free(temp);
			total_freed+=2;
		}

		struct table_info* temp = tables_head;
		tables_head = tables_head->next;
		free(temp->name);
		free(temp);
		total_freed+=2;
	}

	if (the_debug == YES_DEBUG)
		printf("freeMemOfDB() freed %d things\n", total_freed);
	return 0;
}