#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stddef.h>
#include <inttypes.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <time.h>
#include <windows.h>
#include "DB_Tests.h"
#include "DB_Driver.h"
#include "DB_HelperFunctions.h"
#include "DB_Controller.h"


int setOutputRed()
{
	HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
	SetConsoleTextAttribute(hConsole, 4);
	return 0;
}

int setOutputGreen()
{
	HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
	SetConsoleTextAttribute(hConsole, 10);
	return 0;
}

int setOutputWhite()
{
	HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
	SetConsoleTextAttribute(hConsole, 15);
	return 0;
}

/*
int test_Driver_setup(struct malloced_node** malloced_head, int the_debug)
{
	int initd = initDB(malloced_head, the_debug);
	if (initd == -1)
	{
		printf("Database initialization had a problem with file i/o, please try again\n\n");
		return -1;
	}
	else if (initd == -2)
	{
		printf("Database initialization had a problem with malloc, please try again\n\n");
		return -2;
	}
	else
		printf("Successfully initialized database\n\n");


	char* table_name = (char*) myMalloc(sizeof(char) * 32, NULL, malloced_head, the_debug);
	if (table_name == NULL)
	{
		printf("Table mallocing had a problem\n");
		return -1;
	}
	strcpy(table_name, "alc_brands");
	int num_rows = 1000;
	int_8 returnd = createTableFromCSV("Liquor_Brands.csv", table_name, num_rows, malloced_head, the_debug);
	if (returnd != num_rows)
	{
		printf("Table creation from CSV had a problem\n");
		return -1;
	}
	else
	{
		printf("Table creation from CSV has successfully:\n");
		printf("	Created the table\n");
		printf("	Inserted %lu rows\n\n", returnd);

		if (selectAndCheckHash("DB_Files_2_Test_Versions\\CreatedTable.csv", -1, malloced_head, the_debug) != 0)
			return -1;
	}

	printf("\nSuccessfully setup database\n\n");

	return 0;
}*/

int test_Driver_teardown(int the_debug)
{
	while (freeMemOfDB(the_debug) != 0)
	{
        setOutputRed();
        printf("\nTeardown FAILED\n\n");
        setOutputWhite();
        return -1;
	}
    setOutputGreen();
    printf("\nSuccessfully teared down database\n\n");
    setOutputWhite();

    //system("del DB_Files_2\\*");

    return 0;
}

/*int selectAndCheckHash(char* test_version, int test_id, struct malloced_node** malloced_head, int the_debug)
{
	int_8* col_numbers = (int_8*) myMalloc(sizeof(int_8) * 7, NULL, malloced_head, the_debug);
	if (col_numbers == NULL)
	{
		if (the_debug == YES_DEBUG)
			printf("	ERROR in selectAndCheckHash() at line %d in %s\n", __LINE__, __FILE__);
		return -1;
	}
	col_numbers[0] = 0;
	col_numbers[1] = 1;
	col_numbers[2] = 2;
	col_numbers[3] = 3;
	col_numbers[4] = 4;
	col_numbers[5] = 5;
	col_numbers[6] = 6;
	int col_numbers_size = 7;

	int_8 num_rows_in_result = 0;

	struct colDataNode*** result = select(getTablesHead(), col_numbers, col_numbers_size, &num_rows_in_result, NULL, malloced_head, the_debug);
	if (result == NULL)
	{
		printf("Data retreival from disk FAILED, please try again\n");
		return -1;
	}
	else
	{
		int amt_malloced = 0;
		struct malloced_node* cur_malloc = *malloced_head;
		while (cur_malloc != NULL)
		{
			amt_malloced++;
			cur_malloc = cur_malloc->next;
		}
		//printf("After select and before display there are %d things in malloced_head\n", amt_malloced);

		//printf("Successfully retreived %lu rows from disk, creating .csv file\n", num_rows_in_result);
		int displayed = displayResultsOfSelect(result, getTablesHead(), col_numbers, col_numbers_size, num_rows_in_result, malloced_head, the_debug);

		// START Free stuff
		int_8 total_freed = 0;
		for (int j=0; j<col_numbers_size; j++)
		{
			for (int i=0; i<num_rows_in_result; i++)
			{
				myFree((void**) &(result[j][i]->row_data), NULL, malloced_head, YES_DEBUG);
				myFree((void**) &result[j][i], NULL, malloced_head, YES_DEBUG);
				total_freed+=2;
			}
			myFree((void**) &result[j], NULL, malloced_head, YES_DEBUG);
			total_freed++;
		}
		myFree((void**) &result, NULL, malloced_head, YES_DEBUG);
		total_freed++;

		if (the_debug == YES_DEBUG)
			printf("	Freed %lu things from result\n", total_freed);

		myFree((void**) &col_numbers, NULL, malloced_head, the_debug);
		// END Free stuff

		if (displayed != 0)
		{
			printf("Creation of .csv file had a problem with file i/o, please try again.\n");
			return -1;
		}

		/*
		char cmd[100];
		strcpy(cmd, "certutil -hashfile \0");
		strcat(cmd, test_version);

		FILE *fp;
  		char* var1 = (char*) myMalloc(sizeof(char) * 100, NULL, malloced_head, the_debug);
  		char* var2 = (char*) myMalloc(sizeof(char) * 100, NULL, malloced_head, the_debug);
  		char* hash1 = (char*) myMalloc(sizeof(char) * 100, NULL, malloced_head, the_debug);
  		char* hash2 = (char*) myMalloc(sizeof(char) * 100, NULL, malloced_head, the_debug);

  		if (var1 == NULL || var2 == NULL || hash1 == NULL || hash2 == NULL)
  		{
			if (the_debug == YES_DEBUG)
				printf("	ERROR in selectAndCheckHash() at line %d in %s\n", __LINE__, __FILE__);
			return -1;
		}

		fp = popen(cmd, "r");
		if (fp == NULL) 
		{
			printf("Failed to run command\n");
			return -1;
		}
		int i=0;
		while (fgets(var1, 100, fp) != NULL)
		{
			if (i == 1)
				strcpy(hash1, var1);
			//printf("%s\n", var1);
			i++;
		}

		pclose(fp);

		fp = popen("certutil -hashfile Results.csv", "r");
		if (fp == NULL)
		{
			printf("Failed to run command\n");
			return -1;
		}
		i=0;
		while (fgets(var2, 100, fp) != NULL)
		{
			if (i == 1)
				strcpy(hash2, var2);
			//printf("%s\n", var2);
			i++;
		}

		pclose(fp);

		//system(cmd);
		//system("certutil -hashfile Results.csv");
		//printf("hash1 = %s\n", hash1);
		//printf("hash2 = %s\n", hash2);
		int compared = strcmp(hash1, hash2);

		if (compared != 0)
		{
			printf("File hashes DID NOT match for test %d\n", test_id);
			printf("hash1 = %s", hash1);
			printf("hash2 = %s", hash2);
		}

		myFree((void**) &var1, NULL, malloced_head, the_debug);
		myFree((void**) &var2, NULL, malloced_head, the_debug);
		myFree((void**) &hash1, NULL, malloced_head, the_debug);
		myFree((void**) &hash2, NULL, malloced_head, the_debug);

		if (compared != 0)
			return -1;*/
	/*}

	return 0;
}*/


int test_Helper_DateFunctions_1(char* date, struct malloced_node** malloced_head, int the_debug)
{
	setOutputGreen();
	printf("Testing date: %s\n", date);
	setOutputWhite();

	int_8 data_int_form = dateToInt(date);
	printf("data_int_form = %lu\n", data_int_form);

	char* data_char_form = intToDate(data_int_form, NULL, malloced_head, the_debug);
	printf("data_char_form = %s\n", data_char_form);

	if (strcmp(date, data_char_form) != 0)
	{
		setOutputRed();
		printf("The dates did NOT match\n\n");
		setOutputWhite();
		myFree((void**) &data_char_form, NULL, malloced_head, the_debug);
		return -1;
	}
	
	printf("\n");
	myFree((void**) &data_char_form, NULL, malloced_head, the_debug);
	return 0;
}

int test_Helper_DateFunctions_2(int_8 date, struct malloced_node** malloced_head, int the_debug)
{
	char* data_char_form = intToDate(date, NULL, malloced_head, the_debug);
	int_8 data_int_form = dateToInt(data_char_form);

	if (data_int_form != date)
	{
		setOutputRed();
		printf("The date int %lu did NOT match\n", data_int_form);
		setOutputWhite();
		myFree((void**) &data_char_form, NULL, malloced_head, the_debug);
		return -1;
	}

	myFree((void**) &data_char_form, NULL, malloced_head, the_debug);
	return 0;
}


int initColumnsArrForComp(/*struct col_in_select_node*** columns_arr,*/ int columns_arr_size
						 ,struct malloced_node** malloced_head, int the_debug, ...)
{
	//*columns_arr = (struct col_in_select_node**) myMalloc(sizeof(struct col_in_select_node*) * columns_arr_size, NULL, malloced_head, the_debug);

	int* num = (int*) &the_debug;

	for (int i=1; i<=columns_arr_size; i++)
	{
		if (i % 2 == 0)
			printf("%s\n", (num+i));
		else
			printf("%d\n", *((int*) (num+i)));

		//(*columns_arr)[i] = 
	}

	return 0;
}

int initWhereClauseNodeForComp(struct where_clause_node** add_to_this_node, void* ptr_one, int ptr_one_type
							  ,void* ptr_two, int ptr_two_type, int where_type, struct where_clause_node* sibling_prev
							  ,struct where_clause_node* sibling_next, struct where_clause_node* child_prev
							  ,struct where_clause_node* parent_next
							  ,struct malloced_node** malloced_head, int the_debug)
{
	struct where_clause_node* new_temp = (struct where_clause_node*) myMalloc(sizeof(struct where_clause_node), NULL, malloced_head, the_debug);
	if (new_temp == NULL)
	{
		if (the_debug == YES_DEBUG)
			printf("	ERROR in initWhereClauseNodeForComp() at line %d in %s\n", __LINE__, __FILE__);
		return -1;
	}

	new_temp->ptr_one = ptr_one;
	new_temp->ptr_one_type = ptr_one_type;

	new_temp->ptr_two = ptr_two;
	new_temp->ptr_two_type = ptr_two_type;

	new_temp->where_type = where_type;

	new_temp->sibling_prev = sibling_prev;
	new_temp->sibling_next = sibling_next;

	new_temp->child_prev = child_prev;
	new_temp->parent_next = parent_next;

	*add_to_this_node = new_temp;

	return 0;
}

int initJoinNodeForComp(struct join_node** head, struct join_node** tail, int join_type, struct select_node* select_from
					   ,struct select_node* select_joined, struct where_clause_node* on_clause_head
					   ,struct malloced_node** malloced_head, int the_debug)
{
	struct join_node* new_temp = (struct join_node*) myMalloc(sizeof(struct join_node), NULL, malloced_head, the_debug);
	if (new_temp == NULL)
	{
		if (the_debug == YES_DEBUG)
			printf("	ERROR in initJoinNodeForComp() at line %d in %s\n", __LINE__, __FILE__);
		return -1;
	}

	new_temp->join_type = join_type;

	new_temp->select_from = select_from;
	new_temp->select_joined = select_joined;

	new_temp->on_clause_head = on_clause_head;

	if (*head == NULL)
	{
		*head = new_temp;

		(*head)->prev = NULL;
		(*head)->next = NULL;

		*tail = *head;
	}
	else
	{
		(*tail)->next = new_temp;

		(*tail)->next->prev = *tail;
		(*tail)->next->next = NULL;
	}

	return 0;
}

int initSelectClauseForComp(struct select_node** add_to_this_node, char* alias, bool distinct
						   ,int columns_arr_size, struct col_in_select_node** columns_arr, void* table, int table_ptr_type
						   ,struct where_clause_node* where_head, struct join_node* join_head
						   ,struct malloced_node** malloced_head, int the_debug)
{
	struct select_node* new_temp = (struct select_node*) myMalloc(sizeof(struct select_node), NULL, malloced_head, the_debug);
	if (new_temp == NULL)
	{
		if (the_debug == YES_DEBUG)
			printf("	ERROR in initSelectClauseForComp() at line %d in %s\n", __LINE__, __FILE__);
		return -1;
	}

	if (alias != NULL)
	{
		new_temp->select_node_alias = (char*) myMalloc(sizeof(char) * 100, NULL, malloced_head, the_debug);
		if (new_temp->select_node_alias == NULL)
		{
			if (the_debug == YES_DEBUG)
				printf("	ERROR in initSelectClauseForComp() at line %d in %s\n", __LINE__, __FILE__);
			return -1;
		}
		strcpy(new_temp->select_node_alias, alias);
	}
	else
		new_temp->select_node_alias = NULL;

	new_temp->distinct = distinct;

	if (table != NULL && table_ptr_type == PTR_TYPE_TABLE_INFO)
	{
		new_temp->columns_arr_size = ((struct table_info*) table)->num_cols;
		new_temp->columns_arr = (struct col_in_select_node**) myMalloc(sizeof(struct col_in_select_node*) * new_temp->columns_arr_size, NULL, malloced_head, the_debug);
		
		struct table_cols_info* cur_col = ((struct table_info*) table)->table_cols_head;
		for (int i=0; i<new_temp->columns_arr_size; i++)
		{
			new_temp->columns_arr[i] = (struct col_in_select_node*) myMalloc(sizeof(struct col_in_select_node), NULL, malloced_head, the_debug);
			
			new_temp->columns_arr[i]->table_ptr = table;
			new_temp->columns_arr[i]->table_ptr_type = PTR_TYPE_TABLE_INFO;

			new_temp->columns_arr[i]->col_ptr = cur_col;
			new_temp->columns_arr[i]->col_ptr_type = PTR_TYPE_TABLE_COLS_INFO;

			new_temp->columns_arr[i]->new_name = NULL;
			new_temp->columns_arr[i]->case_when_head = NULL;
			new_temp->columns_arr[i]->case_then_value = NULL;
			new_temp->columns_arr[i]->case_then_value_type = -1;
			new_temp->columns_arr[i]->func_node = NULL;
			new_temp->columns_arr[i]->math_node = NULL;

			cur_col = cur_col->next;
		}
	}
	else if (table != NULL && table_ptr_type == PTR_TYPE_SELECT_NODE)
	{
		new_temp->columns_arr_size = ((struct select_node*) table)->columns_arr_size;
		new_temp->columns_arr = (struct col_in_select_node**) myMalloc(sizeof(struct col_in_select_node*) * new_temp->columns_arr_size, NULL, malloced_head, the_debug);

		for (int i=0; i<new_temp->columns_arr_size; i++)
		{
			new_temp->columns_arr[i] = (struct col_in_select_node*) myMalloc(sizeof(struct col_in_select_node), NULL, malloced_head, the_debug);

			new_temp->columns_arr[i]->table_ptr = table;
			new_temp->columns_arr[i]->table_ptr_type = PTR_TYPE_SELECT_NODE;

			new_temp->columns_arr[i]->col_ptr = ((struct select_node*) table)->columns_arr[i];
			new_temp->columns_arr[i]->col_ptr_type = PTR_TYPE_COL_IN_SELECT_NODE;

			new_temp->columns_arr[i]->new_name = NULL;
			new_temp->columns_arr[i]->case_when_head = NULL;
			new_temp->columns_arr[i]->case_then_value = NULL;
			new_temp->columns_arr[i]->case_then_value_type = -1;
			new_temp->columns_arr[i]->func_node = NULL;
			new_temp->columns_arr[i]->math_node = NULL;
		}
	}
	else
	{
		new_temp->columns_arr_size = columns_arr_size;
		new_temp->columns_arr = columns_arr;
	}

	new_temp->where_head = where_head;
	new_temp->join_head = join_head;

	new_temp->prev = NULL;
	new_temp->next = NULL;

	*add_to_this_node = new_temp;

	return 0;
}

int compMathOrWhereTree(int test_id, int tree_ptr_type, void* actual_ptr, void* expected_ptr)
{
	if (tree_ptr_type == PTR_TYPE_MATH_NODE)
	{
		if (((struct math_node*) actual_ptr)->ptr_one_type != ((struct math_node*) expected_ptr)->ptr_one_type)
		{
			printf("compMathOrWhereTree with id = %d FAILED\n", test_id);
			printf("Actual math_node ptr_one_type %d did not equal below\n", ((struct math_node*) actual_ptr)->ptr_one_type);
			printf("Expected math_node ptr_one_type %d\n", ((struct math_node*) expected_ptr)->ptr_one_type);
			return -1;
		}

		if (((struct math_node*) actual_ptr)->ptr_two_type != ((struct math_node*) expected_ptr)->ptr_two_type)
		{
			printf("compMathOrWhereTree with id = %d FAILED\n", test_id);
			printf("Actual math_node ptr_two_type %d did not equal below\n", ((struct math_node*) actual_ptr)->ptr_two_type);
			printf("Expected math_node ptr_two_type %d\n", ((struct math_node*) expected_ptr)->ptr_two_type);
			return -1;
		}


		for (int i=0; i<2; i++)
		{
			int ptr_type;

			void* act_ptr;
			void* exp_ptr;

			if (i == 0)
			{
				ptr_type = ((struct math_node*) actual_ptr)->ptr_one_type;

				act_ptr = ((struct math_node*) actual_ptr)->ptr_one;
				exp_ptr = ((struct math_node*) expected_ptr)->ptr_one;
			}
			else //if (i == 1)
			{
				ptr_type = ((struct math_node*) actual_ptr)->ptr_two_type;

				act_ptr = ((struct math_node*) actual_ptr)->ptr_two;
				exp_ptr = ((struct math_node*) expected_ptr)->ptr_two;
			}

			if (ptr_type == tree_ptr_type)
			{
				if (compMathOrWhereTree(test_id, tree_ptr_type, act_ptr, exp_ptr) != 0)
					return -1;
			}
			else if (ptr_type == PTR_TYPE_INT && *((int*) act_ptr) != *((int*) exp_ptr))
			{
				printf("compMathOrWhereTree with id = %d FAILED\n", test_id);
				printf("Actual math_node ptr %d did not equal below\n", *((int*) act_ptr));
				printf("Expected math_node ptr %d\n", *((int*) exp_ptr));
				return -1;
			}
			else if (ptr_type == PTR_TYPE_REAL && *((double*) act_ptr) != *((double*) exp_ptr))
			{
				printf("compMathOrWhereTree with id = %d FAILED\n", test_id);
				printf("Actual math_node ptr %f did not equal below\n", *((double*) act_ptr));
				printf("Expected math_node ptr %f\n", *((double*) exp_ptr));
				return -1;
			}
			else if (ptr_type == PTR_TYPE_COL_IN_SELECT_NODE && act_ptr != exp_ptr)
			{
				printf("compMathOrWhereTree with id = %d FAILED\n", test_id);
				printf("Actual math_node ptr (%s) did not equal expected math_node ptr (%s)\n"
					   ,((struct table_cols_info*) act_ptr)->col_name, ((struct table_cols_info*) exp_ptr)->col_name);
				return -1;
			}
		}
	}
	else //if (tree_ptr_type == PTR_TYPE_WHERE_CLAUSE_NODE)
	{

	}

	return 0;
}



/*int test_Controller_parseWhereClause(int test_id, char* where_string, char* first_word
									,int expected_error_code, struct or_clause_node** expected_or_head
									,struct malloced_node** malloced_head, int the_debug)
{
	printf("Starting test with id = %d\n", test_id);

	int result = 0;


	struct select_node* the_select_node;
	if (test_id < 113)
	{
		the_select_node = (struct select_node*) myMalloc(sizeof(struct select_node), NULL, malloced_head, the_debug);
		the_select_node->select_node_alias = NULL;

		the_select_node->columns_arr_size = 7;

		the_select_node->columns_table_ptrs_arr = (struct table_info**) myMalloc(sizeof(struct table_info*) * 7, NULL, malloced_head, the_debug);
		the_select_node->columns_table_ptrs_arr[0] = getTablesHead();
		the_select_node->columns_table_ptrs_arr[1] = getTablesHead();
		the_select_node->columns_table_ptrs_arr[2] = getTablesHead();
		the_select_node->columns_table_ptrs_arr[3] = getTablesHead();
		the_select_node->columns_table_ptrs_arr[4] = getTablesHead();
		the_select_node->columns_table_ptrs_arr[5] = getTablesHead();
		the_select_node->columns_table_ptrs_arr[6] = getTablesHead();

		the_select_node->columns_col_ptrs_arr = (struct table_cols_info**) myMalloc(sizeof(struct table_cols_info*) * 7, NULL, malloced_head, the_debug);
		the_select_node->columns_col_ptrs_arr[0] = getTablesHead()->table_cols_head;
		the_select_node->columns_col_ptrs_arr[1] = getTablesHead()->table_cols_head->next;
		the_select_node->columns_col_ptrs_arr[2] = getTablesHead()->table_cols_head->next->next;
		the_select_node->columns_col_ptrs_arr[3] = getTablesHead()->table_cols_head->next->next->next;
		the_select_node->columns_col_ptrs_arr[4] = getTablesHead()->table_cols_head->next->next->next->next;
		the_select_node->columns_col_ptrs_arr[5] = getTablesHead()->table_cols_head->next->next->next->next->next;
		the_select_node->columns_col_ptrs_arr[6] = getTablesHead()->table_cols_head->next->next->next->next->next->next;

		the_select_node->or_head = NULL;
		the_select_node->join_head = NULL;

		the_select_node->next = NULL;

		the_select_node->prev = (struct select_node*) myMalloc(sizeof(struct select_node), NULL, malloced_head, the_debug);
		the_select_node->prev->select_node_alias = NULL;

		the_select_node->prev->columns_arr_size = 7;

		the_select_node->prev->columns_table_ptrs_arr = (struct table_info**) myMalloc(sizeof(struct table_info*) * 7, NULL, malloced_head, the_debug);
		the_select_node->prev->columns_table_ptrs_arr[0] = getTablesHead();
		the_select_node->prev->columns_table_ptrs_arr[1] = getTablesHead();
		the_select_node->prev->columns_table_ptrs_arr[2] = getTablesHead();
		the_select_node->prev->columns_table_ptrs_arr[3] = getTablesHead();
		the_select_node->prev->columns_table_ptrs_arr[4] = getTablesHead();
		the_select_node->prev->columns_table_ptrs_arr[5] = getTablesHead();
		the_select_node->prev->columns_table_ptrs_arr[6] = getTablesHead();

		the_select_node->prev->columns_col_ptrs_arr = (struct table_cols_info**) myMalloc(sizeof(struct table_cols_info*) * 7, NULL, malloced_head, the_debug);
		the_select_node->prev->columns_col_ptrs_arr[0] = getTablesHead()->table_cols_head;
		the_select_node->prev->columns_col_ptrs_arr[1] = getTablesHead()->table_cols_head->next;
		the_select_node->prev->columns_col_ptrs_arr[2] = getTablesHead()->table_cols_head->next->next;
		the_select_node->prev->columns_col_ptrs_arr[3] = getTablesHead()->table_cols_head->next->next->next;
		the_select_node->prev->columns_col_ptrs_arr[4] = getTablesHead()->table_cols_head->next->next->next->next;
		the_select_node->prev->columns_col_ptrs_arr[5] = getTablesHead()->table_cols_head->next->next->next->next->next;
		the_select_node->prev->columns_col_ptrs_arr[6] = getTablesHead()->table_cols_head->next->next->next->next->next->next;

		the_select_node->prev->or_head = NULL;
		the_select_node->prev->join_head = NULL;

		the_select_node->prev->next = the_select_node;
		the_select_node->prev->prev = NULL;
	}
	else
	{
		the_select_node = (struct select_node*) myMalloc(sizeof(struct select_node), NULL, malloced_head, the_debug);

		the_select_node->select_node_alias = NULL;
		the_select_node->columns_arr_size = 4;

		the_select_node->columns_table_ptrs_arr = (struct table_info**) myMalloc(sizeof(struct table_info*) * 4, NULL, malloced_head, the_debug);
		the_select_node->columns_table_ptrs_arr[0] = getTablesHead();
		the_select_node->columns_table_ptrs_arr[1] = getTablesHead();
		the_select_node->columns_table_ptrs_arr[2] = getTablesHead();
		the_select_node->columns_table_ptrs_arr[3] = getTablesHead();

		the_select_node->columns_col_ptrs_arr = (struct table_cols_info**) myMalloc(sizeof(struct table_cols_info*) * 4, NULL, malloced_head, the_debug);
		the_select_node->columns_col_ptrs_arr[0] = getTablesHead()->table_cols_head;
		the_select_node->columns_col_ptrs_arr[1] = getTablesHead()->table_cols_head->next->next->next->next->next->next;
		the_select_node->columns_col_ptrs_arr[2] = getTablesHead()->table_cols_head;
		the_select_node->columns_col_ptrs_arr[3] = getTablesHead()->table_cols_head->next->next->next->next->next->next;

		the_select_node->or_head = NULL;

		the_select_node->next = NULL;

		the_select_node->prev = (struct select_node*) myMalloc(sizeof(struct select_node), NULL, malloced_head, the_debug);
		the_select_node->prev->select_node_alias = upper("TBL", NULL, malloced_head, the_debug);
		
		the_select_node->prev->columns_arr_size = 7;

		the_select_node->prev->columns_table_ptrs_arr = (struct table_info**) myMalloc(sizeof(struct table_info*) * 7, NULL, malloced_head, the_debug);
		the_select_node->prev->columns_table_ptrs_arr[0] = getTablesHead();
		the_select_node->prev->columns_table_ptrs_arr[1] = getTablesHead();
		the_select_node->prev->columns_table_ptrs_arr[2] = getTablesHead();
		the_select_node->prev->columns_table_ptrs_arr[3] = getTablesHead();
		the_select_node->prev->columns_table_ptrs_arr[4] = getTablesHead();
		the_select_node->prev->columns_table_ptrs_arr[5] = getTablesHead();
		the_select_node->prev->columns_table_ptrs_arr[6] = getTablesHead();

		the_select_node->prev->columns_col_ptrs_arr = (struct table_cols_info**) myMalloc(sizeof(struct table_cols_info*) * 7, NULL, malloced_head, the_debug);
		the_select_node->prev->columns_col_ptrs_arr[0] = getTablesHead()->table_cols_head;
		the_select_node->prev->columns_col_ptrs_arr[1] = getTablesHead()->table_cols_head->next;
		the_select_node->prev->columns_col_ptrs_arr[2] = getTablesHead()->table_cols_head->next->next;
		the_select_node->prev->columns_col_ptrs_arr[3] = getTablesHead()->table_cols_head->next->next->next;
		the_select_node->prev->columns_col_ptrs_arr[4] = getTablesHead()->table_cols_head->next->next->next->next;
		the_select_node->prev->columns_col_ptrs_arr[5] = getTablesHead()->table_cols_head->next->next->next->next->next;
		the_select_node->prev->columns_col_ptrs_arr[6] = getTablesHead()->table_cols_head->next->next->next->next->next->next;

		the_select_node->prev->or_head = NULL;
		the_select_node->prev->join_head = NULL;

		the_select_node->prev->next = the_select_node;

		the_select_node->prev->prev = (struct select_node*) myMalloc(sizeof(struct select_node), NULL, malloced_head, the_debug);
		the_select_node->prev->prev->select_node_alias = upper("TBL2", NULL, malloced_head, the_debug);
		
		the_select_node->prev->prev->columns_arr_size = 7;

		the_select_node->prev->prev->columns_table_ptrs_arr = (struct table_info**) myMalloc(sizeof(struct table_info*) * 7, NULL, malloced_head, the_debug);
		the_select_node->prev->prev->columns_table_ptrs_arr[0] = getTablesHead();
		the_select_node->prev->prev->columns_table_ptrs_arr[1] = getTablesHead();
		the_select_node->prev->prev->columns_table_ptrs_arr[2] = getTablesHead();
		the_select_node->prev->prev->columns_table_ptrs_arr[3] = getTablesHead();
		the_select_node->prev->prev->columns_table_ptrs_arr[4] = getTablesHead();
		the_select_node->prev->prev->columns_table_ptrs_arr[5] = getTablesHead();
		the_select_node->prev->prev->columns_table_ptrs_arr[6] = getTablesHead();

		the_select_node->prev->prev->columns_col_ptrs_arr = (struct table_cols_info**) myMalloc(sizeof(struct table_cols_info*) * 7, NULL, malloced_head, the_debug);
		the_select_node->prev->prev->columns_col_ptrs_arr[0] = getTablesHead()->table_cols_head;
		the_select_node->prev->prev->columns_col_ptrs_arr[1] = getTablesHead()->table_cols_head->next;
		the_select_node->prev->prev->columns_col_ptrs_arr[2] = getTablesHead()->table_cols_head->next->next;
		the_select_node->prev->prev->columns_col_ptrs_arr[3] = getTablesHead()->table_cols_head->next->next->next;
		the_select_node->prev->prev->columns_col_ptrs_arr[4] = getTablesHead()->table_cols_head->next->next->next->next;
		the_select_node->prev->prev->columns_col_ptrs_arr[5] = getTablesHead()->table_cols_head->next->next->next->next->next;
		the_select_node->prev->prev->columns_col_ptrs_arr[6] = getTablesHead()->table_cols_head->next->next->next->next->next->next;

		the_select_node->prev->prev->or_head = NULL;
		the_select_node->prev->prev->join_head = NULL;

		the_select_node->prev->prev->next = the_select_node->prev;
		
		the_select_node->prev->prev->prev = NULL;

		the_select_node->join_head = (struct join_node*) myMalloc(sizeof(struct join_node), NULL, malloced_head, the_debug);
		the_select_node->join_head->join_type = JOIN_INNER;
		the_select_node->join_head->select_from = the_select_node->prev;
		the_select_node->join_head->select_joined = the_select_node->prev->prev;

		the_select_node->join_head->next = NULL;
		the_select_node->join_head->on_clause_head = NULL;
	}


	int error_code;
	struct or_clause_node* or_head = parseWhereClause(where_string, &the_select_node, &error_code, first_word, malloced_head, the_debug);


	if (error_code != 0)
	{
		*expected_or_head = NULL;
	}
	else
	{
		while (the_select_node != NULL)
		{
			struct select_node* temp = the_select_node;
			the_select_node = the_select_node->prev;

			myFree((void**) &temp->columns_table_ptrs_arr, NULL, malloced_head, the_debug);
			myFree((void**) &temp->columns_col_ptrs_arr, NULL, malloced_head, the_debug);

			while (temp->join_head != NULL)
			{
				struct join_node* temp_join = temp->join_head;
				temp->join_head = temp->join_head->next;

				myFree((void**) &temp_join, NULL, malloced_head, the_debug);
			}

			myFree((void**) &temp->select_node_alias, NULL, malloced_head, the_debug);
			myFree((void**) &temp, NULL, malloced_head, the_debug);
		}
	}


	if (expected_error_code != error_code)
	{
		printf("test_Controller_parseWhereClause with id = %d FAILED\n", test_id);
		printf("expected_error_code = %d, and error_code = %d\n", expected_error_code, error_code);

		return -1;
	}


	// START Check if or_clauses match
	struct or_clause_node* cur_actual_or = or_head;
	struct or_clause_node* cur_expected_or = (*expected_or_head);

	while (cur_actual_or != NULL || cur_expected_or != NULL)
	{
		if (cur_actual_or == NULL && cur_expected_or != NULL)
		{
			printf("test_Controller_parseWhereClause with id = %d FAILED\n", test_id);
			printf("cur_actual_or was NULL and cur_expected_or was NOT NULL\n");
			return -1;
		}
		else if (cur_actual_or != NULL && cur_expected_or == NULL)
		{
			printf("test_Controller_parseWhereClause with id = %d FAILED\n", test_id);
			printf("cur_actual_or was NOT NULL and cur_expected_or was NULL\n");
			return -1;
		}
		else
		{
			struct and_clause_node* cur_actual_and = cur_actual_or->and_head;
			struct and_clause_node* cur_expected_and = cur_expected_or->and_head;

			while (cur_actual_and != NULL || cur_expected_and != NULL)
			{
				if (cur_actual_and == NULL && cur_expected_and != NULL)
				{
					printf("test_Controller_parseWhereClause with id = %d FAILED\n", test_id);
					printf("cur_actual_and was NULL and cur_expected_and was NOT NULL\n");
					return -1;
				}
				else if (cur_actual_and != NULL && cur_expected_and == NULL)
				{
					printf("test_Controller_parseWhereClause with id = %d FAILED\n", test_id);
					printf("cur_actual_and was NOT NULL and cur_expected_and was NULL\n");
					return -1;
				}
				else
				{
					if (cur_actual_and->table != cur_expected_and->table)
					{
						printf("test_Controller_parseWhereClause with id = %d FAILED\n", test_id);
						printf("Actual and_node table (%s) did not equal below\n", cur_actual_and->table == NULL ? "NULL" : cur_actual_and->table->name);
						printf("Expected and_node table (%s)\n", cur_expected_and->table == NULL ? "NULL" : cur_expected_and->table->name);
						return -1;
					}
					if (cur_actual_and->col != cur_expected_and->col)
					{
						printf("test_Controller_parseWhereClause with id = %d FAILED\n", test_id);
						printf("Actual and_node col (%s) did not equal below\n", cur_actual_and->col== NULL ? "NULL" : cur_actual_and->col->col_name);
						printf("Expected and_node col (%s)\n", cur_expected_and->col == NULL ? "NULL" : cur_expected_and->col->col_name);
						return -1;
					}
					if (cur_actual_and->table_joined != cur_expected_and->table_joined)
					{
						printf("test_Controller_parseWhereClause with id = %d FAILED\n", test_id);
						printf("Actual and_node table_joined (%s) did not equal below\n", cur_actual_and->table_joined == NULL ? "NULL" : cur_actual_and->table_joined->name);
						printf("Expected and_node table_joined (%s)\n", cur_expected_and->table_joined == NULL ? "NULL" : cur_expected_and->table_joined->name);
						return -1;
					}
					if (cur_actual_and->col_joined != cur_expected_and->col_joined)
					{
						printf("test_Controller_parseWhereClause with id = %d FAILED\n", test_id);
						printf("Actual and_node col_joined (%s) did not equal below\n", cur_actual_and->col_joined== NULL ? "NULL" : cur_actual_and->col_joined->col_name);
						printf("Expected and_node col_joined (%s)\n", cur_expected_and->col_joined == NULL ? "NULL" : cur_expected_and->col_joined->col_name);
						return -1;
					}
					if (cur_actual_and->where_type != cur_expected_and->where_type)
					{
						printf("test_Controller_parseWhereClause with id = %d FAILED\n", test_id);
						printf("Actual and_node col_joined (%d) did not equal below\n", cur_actual_and->where_type);
						printf("Expected and_node col_joined (%d)\n", cur_expected_and->where_type);
						return -1;
					}
					if (cur_actual_and->data_string != NULL && cur_expected_and->data_string != NULL && strcmp(cur_actual_and->data_string, cur_expected_and->data_string) != 0)
					{
						printf("test_Controller_parseWhereClause with id = %d FAILED\n", test_id);
						printf("Actual and_node data_string (%s) did not equal below\n", cur_actual_and->data_string == NULL ? "NULL" : cur_actual_and->data_string);
						printf("Expected and_node data_string (%s)\n", cur_expected_and->data_string == NULL ? "NULL" : cur_expected_and->data_string);
						return -1;
					}
				}

				if (cur_actual_and != NULL)
					cur_actual_and = cur_actual_and->next;
				if (cur_expected_and != NULL)
					cur_expected_and = cur_expected_and->next;
			}
		}

		if (cur_actual_or != NULL)
			cur_actual_or = cur_actual_or->next;
		if (cur_expected_or != NULL)
			cur_expected_or = cur_expected_or->next;
	}
	// END Check if or_clauses match


	// START Free or_head if malloced
	while (or_head != NULL)
	{
		//printf("Freeing or_head\n");
		while (or_head->and_head != NULL)
		{
			//printf("Freeing and_head\n");
			struct and_clause_node* temp = or_head->and_head;
			or_head->and_head = or_head->and_head->next;

			//printf("	Calling free\n");
			myFree((void**) &(temp->data_string), NULL, malloced_head, the_debug);

			//printf("	Calling free\n");
			myFree((void**) &temp, NULL, malloced_head, the_debug);

			//printf("Done and_head\n");
		}
		struct or_clause_node* temp = or_head;
		or_head = or_head->next;

		//printf("	Calling free\n");
		myFree((void**) &temp, NULL, malloced_head, the_debug);

		//printf("Done or_head\n");
	}
	// END Free or_head if malloced


	return result;
}

int test_Controller_parseUpdate(int test_id, char* update_string, struct change_node_v2** expected_change_head, int* parsed_error_code
							   ,struct malloced_node** malloced_head, int the_debug)
{
	printf("Starting test with id = %d\n", test_id);

	int result = 0;

	struct table_info* table = NULL;
	struct change_node_v2* change_head = NULL;
	struct or_clause_node* or_head = NULL;

	*parsed_error_code = parseUpdate(update_string, &table, &change_head, &or_head, malloced_head, the_debug);
	if (*parsed_error_code != 0 && *expected_change_head != NULL)
	{
		printf("test_Controller_parseUpdate with id = %d FAILED\n", test_id);
		printf("The test %d had a problem with parseUpdate()\n", test_id);
		return -1;
	}

	// START Free or_head if malloced
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
	// END Free or_head if malloced


	struct change_node_v2* cur_expected = *expected_change_head;
	struct change_node_v2* cur_actual = change_head;
	while (cur_expected != NULL && cur_actual != NULL)
	{
		if (cur_expected->col_number != cur_actual->col_number)
		{
			printf("test_Controller_parseUpdate with id = %d FAILED\n", test_id);
			printf("Actual col_number %lu did not equal below\n", cur_actual->col_number);
			printf("Expected col_number %lu\n", cur_expected->col_number);
			result = -1;
		}
		else if (cur_expected->operation != cur_actual->operation)
		{
			printf("test_Controller_parseUpdate with id = %d FAILED\n", test_id);
			printf("Actual operation %lu did not equal below\n", cur_actual->operation);
			printf("Expected operation %lu\n", cur_expected->operation);
			result = -1;
		}
		else if (cur_expected->data_type != cur_actual->data_type)
		{
			printf("test_Controller_parseUpdate with id = %d FAILED\n", test_id);
			printf("Actual data_type %lu did not equal below\n", cur_actual->data_type);
			printf("Expected data_type %lu\n", cur_expected->data_type);
			result = -1;
		}
		else if (strcmp(cur_expected->data, cur_actual->data) != 0)
		{
			printf("test_Controller_parseUpdate with id = %d FAILED\n", test_id);
			printf("Actual data %s did not equal below\n", cur_actual->data);
			printf("Expected data %s\n", cur_expected->data);
			result = -1;
		}

		cur_expected = cur_expected->next;
		cur_actual = cur_actual->next;
	}

	if (cur_expected != NULL || cur_actual != NULL)
	{
		printf("test_Controller_parseUpdate with id = %d FAILED\n", test_id);
		if (cur_expected != NULL)
		{
			printf("cur_expected was NOT null while cur_actual was null\n");
			//printf("next cur_expected = %lu\n", cur_expected->value);
		}
		else
		{
			printf("cur_expected was null while cur_actual was NOT null\n");
			//printf("next cur_actual = %lu\n", cur_actual->value);
		}
		result = -1;
	}


	while (change_head != NULL)
	{
		struct change_node_v2* temp = change_head;
		change_head = change_head->next;
		myFree((void**) &temp->data, NULL, malloced_head, the_debug);
		myFree((void**) &temp, NULL, malloced_head, the_debug);
	}

	return result;
}

int test_Controller_parseDelete(int test_id, char* delete_string, char* expected_table_name
							   ,struct malloced_node** malloced_head, int the_debug)
{
	printf("Starting test with id = %d\n", test_id);

	struct table_info* table = NULL;
	struct or_clause_node* or_head = NULL;

	if (parseDelete(delete_string, &or_head, &table, malloced_head, the_debug) != 0 && expected_table_name[0] != 0)
	{
		printf("test_Controller_parseDelete with id = %d FAILED\n", test_id);
		printf("The test %d had a problem with parseDelete()\n", test_id);
		return -1;
	}

	// START Free or_head if malloced
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
	// END Free or_head if malloced

	if (table == NULL && expected_table_name[0] != 0)
	{
		printf("test_Controller_parseDelete with id = %d FAILED\n", test_id);
		printf("Actual table_name was NULL and expected_table_name was NOT NULL\n");
		return -1;
	}
	if (table != NULL && expected_table_name[0] == 0)
	{
		printf("test_Controller_parseDelete with id = %d FAILED\n", test_id);
		printf("Actual table_name was NOT NULL and expected_table_name was NULL\n");
		return -1;
	}
	if (table != NULL && strcmp(expected_table_name, table->name) != 0)
	{
		printf("test_Controller_parseDelete with id = %d FAILED\n", test_id);
		printf("Actual table_name %s did not equal below\n", table->name);
		printf("Expected table_name %s\n", expected_table_name);
		return -1;
	}

	return 0;
}

int test_Controller_parseInsert(int test_id, char* insert_string, struct change_node_v2** expected_change_head, char* expected_table_name
							   ,int* parsed_error_code, struct malloced_node** malloced_head, int the_debug)
{
	printf("Starting test with id = %d\n", test_id);

	int result = 0;

	struct table_info* table = NULL;
	struct change_node_v2* change_head = NULL;

	
	*parsed_error_code = parseInsert(insert_string, &change_head, &table, malloced_head, the_debug);
	if (*parsed_error_code != 0 && expected_change_head != NULL)
	{
		printf("test_Controller_parseInsert with id = %d FAILED\n", test_id);
		printf("The test %d had a problem with parseInsert()\n", test_id);
		return -1;
	}


	if (table == NULL && expected_table_name[0] != 0)
	{
		printf("test_Controller_parseInsert with id = %d FAILED\n", test_id);
		printf("Actual table_name was NULL and expected_table_name was NOT NULL\n");
		return -1;
	}
	if (table != NULL && expected_table_name[0] == 0)
	{
		printf("test_Controller_parseInsert with id = %d FAILED\n", test_id);
		printf("Actual table_name was NOT NULL and expected_table_name was NULL\n");
		return -1;
	}
	if (table != NULL && strcmp(expected_table_name, table->name) != 0)
	{
		printf("test_Controller_parseInsert with id = %d FAILED\n", test_id);
		printf("Actual table_name %s did not equal below\n", table->name);
		printf("Expected table_name %s\n", expected_table_name);
		return -1;
	}


	if (expected_change_head != NULL)
	{
		struct change_node_v2* cur_expected = *expected_change_head;
		struct change_node_v2* cur_actual = change_head;
		while (cur_expected != NULL && cur_actual != NULL)
		{
			if (cur_expected->col_number != cur_actual->col_number)
			{
				printf("test_Controller_parseInsert with id = %d FAILED\n", test_id);
				printf("Actual col_number %lu did not equal below\n", cur_actual->col_number);
				printf("Expected col_number %lu\n", cur_expected->col_number);
				result = -1;
			}
			if (cur_expected->data_type != cur_actual->data_type)
			{
				printf("test_Controller_parseInsert with id = %d FAILED\n", test_id);
				printf("Actual data_type %lu did not equal below\n", cur_actual->data_type);
				printf("Expected data_type %lu\n", cur_expected->data_type);
				result = -1;
			}
			if (cur_expected->data != NULL && cur_actual->data != NULL)
			{
				if (strcmp(cur_expected->data, cur_actual->data) != 0)
				{
					printf("test_Controller_parseInsert with id = %d FAILED\n", test_id);
					printf("Actual data %s did not equal below\n", cur_actual->data);
					printf("Expected data %s\n", cur_expected->data);
					result = -1;
				}
			}

			cur_expected = cur_expected->next;
			cur_actual = cur_actual->next;
		}

		if (cur_expected != NULL || cur_actual != NULL)
		{
			printf("test_Controller_parseInsert with id = %d FAILED\n", test_id);
			if (cur_expected != NULL)
			{
				printf("cur_expected was NOT null while cur_actual was null\n");
				//printf("next cur_expected = %lu\n", cur_expected->value);
			}
			else
			{
				printf("cur_expected was null while cur_actual was NOT null\n");
				//printf("next cur_actual = %lu\n", cur_actual->value);
			}
			result = -1;
		}
	}

	while (change_head != NULL)
	{
		struct change_node_v2* temp = change_head;
		change_head = change_head->next;
		myFree((void**) &temp->data, NULL, malloced_head, the_debug);
		myFree((void**) &temp, NULL, malloced_head, the_debug);
	}

	return result;
}*/

/**/
int test_Controller_parseSelect(int test_id, char* select_string, struct select_node** exp_select_node
							   ,int* parsed_error_code, struct malloced_node** malloced_head, int the_debug)
{
	setOutputGreen();
	printf("\nStarting test with id = %d\n", test_id);
	setOutputWhite();

	struct select_node* select_node = NULL;


	*parsed_error_code = parseSelect(select_string, &select_node, malloced_head, the_debug);
	if ((*parsed_error_code) != 0 && (*exp_select_node) != NULL)
	{
		setOutputRed();
		printf("test_Controller_parseSelect with id = %d FAILED\n", test_id);
		printf("The test %d had a problem with parseSelect()\n", test_id);
		setOutputWhite();
		return -1;
	}
	else if ((*parsed_error_code) != 0 && (*exp_select_node) == NULL)
	{
		return 0; // Test passed because meant to fail
	}


	
	struct select_node* cur_actual_select = select_node;
	struct select_node* cur_expected_select = *exp_select_node;
	setOutputRed();

	while (cur_actual_select != NULL || cur_expected_select != NULL)
	{
		// START Free cur_actual_select->or_head if malloced

		// END Free cur_actual_select->or_head if malloced


		//printf("1\n");
		// START Check if one is null
		if (cur_actual_select == NULL && cur_expected_select != NULL)
		{
			printf("test_Controller_parseSelect with id = %d FAILED\n", test_id);
			printf("cur_actual_select was NULL and cur_expected_select was NOT NULL\n");
			freeAnyLinkedList((void**) &select_node, PTR_TYPE_SELECT_NODE, NULL, malloced_head, the_debug);
			setOutputWhite(); return -1;
		}
		else if (cur_actual_select != NULL && cur_expected_select == NULL)
		{
			printf("test_Controller_parseSelect with id = %d FAILED\n", test_id);
			printf("cur_actual_select was NOT NULL and cur_expected_select was NULL\n");
			freeAnyLinkedList((void**) &select_node, PTR_TYPE_SELECT_NODE, NULL, malloced_head, the_debug);
			setOutputWhite(); return -1;
		}
		// END Check if one is null


		//printf("2\n");
		// START Check aliases
		if (cur_actual_select->select_node_alias == NULL && cur_expected_select->select_node_alias != NULL)
		{
			printf("test_Controller_parseSelect with id = %d FAILED\n", test_id);
			printf("cur actual select_node_alias was NULL and cur exp select_node_alias was NOT NULL\n");
			freeAnyLinkedList((void**) &select_node, PTR_TYPE_SELECT_NODE, NULL, malloced_head, the_debug);
			setOutputWhite(); return -1;
		}
		else if (cur_actual_select->select_node_alias != NULL && cur_expected_select->select_node_alias == NULL)
		{
			printf("test_Controller_parseSelect with id = %d FAILED\n", test_id);
			printf("cur actual select_node_alias was NOT NULL and cur exp select_node_alias was NULL\n");
			freeAnyLinkedList((void**) &select_node, PTR_TYPE_SELECT_NODE, NULL, malloced_head, the_debug);
			setOutputWhite(); return -1;
		}
		else if (cur_actual_select->select_node_alias != NULL 
				&& strcmp(cur_actual_select->select_node_alias, cur_expected_select->select_node_alias) != 0)
		{
			printf("test_Controller_parseSelect with id = %d FAILED\n", test_id);
			printf("Actual select_node_alias _%s_ did not equal below\n", cur_actual_select->select_node_alias);
			printf("Expected select_node_alias _%s_\n", cur_expected_select->select_node_alias);
			freeAnyLinkedList((void**) &select_node, PTR_TYPE_SELECT_NODE, NULL, malloced_head, the_debug);
			setOutputWhite(); return -1;
		}
		// END Check aliases


		//printf("3\n");
		// START Check distinct
		if (cur_actual_select->distinct != cur_expected_select->distinct)
		{
			printf("test_Controller_parseSelect with id = %d FAILED\n", test_id);
			printf("Actual distinct _%s_ did not equal below\n", cur_actual_select->distinct ? "TRUE" : "FALSE");
			printf("Expected distinct _%s_\n", cur_expected_select->distinct ? "TRUE" : "FALSE");
			freeAnyLinkedList((void**) &select_node, PTR_TYPE_SELECT_NODE, NULL, malloced_head, the_debug);
			setOutputWhite(); return -1;
		}
		// END Check distinct

		//printf("4\n");
		// START Check if columns match
		if (cur_actual_select->columns_arr_size != cur_expected_select->columns_arr_size)
		{
			printf("test_Controller_parseSelect with id = %d FAILED\n", test_id);
			printf("Actual columns_arr_size %d did not equal below\n", cur_actual_select->columns_arr_size);
			printf("Expected columns_arr_size %d\n", cur_expected_select->columns_arr_size);
			freeAnyLinkedList((void**) &select_node, PTR_TYPE_SELECT_NODE, NULL, malloced_head, the_debug);
			setOutputWhite(); return -1;
		}

		for (int i=0; i<cur_actual_select->columns_arr_size; i++)
		{
			//printf("5\n");
			void* actual_table_ptr = NULL;
			void* exp_table_ptr = NULL;

			if (cur_actual_select->columns_arr[i]->table_ptr_type == PTR_TYPE_TABLE_INFO)
			{
				actual_table_ptr = cur_actual_select->columns_arr[i]->table_ptr;
				exp_table_ptr = cur_expected_select->columns_arr[i]->table_ptr;
			}
			else if (cur_actual_select->columns_arr[i]->table_ptr_type == PTR_TYPE_SELECT_NODE)
			{
				actual_table_ptr = ((struct col_in_select_node*) cur_actual_select->columns_arr[i]->col_ptr)->table_ptr;
				exp_table_ptr = ((struct col_in_select_node*) cur_expected_select->columns_arr[i]->col_ptr)->table_ptr;
			}

			//printf("6\n");
			if (actual_table_ptr != exp_table_ptr)
			{
				printf("test_Controller_parseSelect with id = %d FAILED\n", test_id);
				if (actual_table_ptr == NULL)
					printf("Actual columns_arr[%d] table_ptr was NULL\n", i);
				else if (exp_table_ptr == NULL)
					printf("Expected columns_arr[%d] table_ptr was NULL\n", i);
				else
				{
					printf("Actual columns_arr[i] table_ptr (%s) did not equal expected columns_arr[i] table_ptr (%s) at i = %d\n"
						   ,((struct table_info*) actual_table_ptr)->name, ((struct table_info*) exp_table_ptr)->name, i);
				}
				freeAnyLinkedList((void**) &select_node, PTR_TYPE_SELECT_NODE, NULL, malloced_head, the_debug);
				setOutputWhite(); return -1;
			}

			//printf("7\n");
			if (cur_actual_select->columns_arr[i]->col_ptr_type != cur_expected_select->columns_arr[i]->col_ptr_type)
			{
				printf("test_Controller_parseSelect with id = %d FAILED\n", test_id);
				printf("Actual col_ptr_type %d did not equal below\n", cur_actual_select->columns_arr[i]->col_ptr_type);
				printf("Expected col_ptr_type %d\n", cur_expected_select->columns_arr[i]->col_ptr_type);
				freeAnyLinkedList((void**) &select_node, PTR_TYPE_SELECT_NODE, NULL, malloced_head, the_debug);
				setOutputWhite(); return -1;
			}

			void* actual_col_ptr = NULL;
			void* exp_col_ptr = NULL;

			if (cur_actual_select->columns_arr[i]->col_ptr_type == PTR_TYPE_TABLE_COLS_INFO || cur_actual_select->columns_arr[i]->col_ptr_type == PTR_TYPE_COL_IN_SELECT_NODE)
			{
				if (cur_actual_select->columns_arr[i]->col_ptr_type == PTR_TYPE_TABLE_COLS_INFO)
				{
					actual_col_ptr = cur_actual_select->columns_arr[i]->col_ptr;
					exp_col_ptr = cur_expected_select->columns_arr[i]->col_ptr;
				}

				if (cur_actual_select->columns_arr[i]->col_ptr_type == PTR_TYPE_COL_IN_SELECT_NODE)
				{
					actual_col_ptr = ((struct col_in_select_node*) cur_actual_select->columns_arr[i]->col_ptr)->col_ptr;
					exp_col_ptr = ((struct col_in_select_node*) cur_expected_select->columns_arr[i]->col_ptr)->col_ptr;
				}

				if (actual_col_ptr != exp_col_ptr)
				{
					printf("test_Controller_parseSelect with id = %d FAILED\n", test_id);
					if (actual_col_ptr == NULL)
						printf("Actual columns_arr[%d] col_ptr was NULL\n", i);
					else if (exp_col_ptr == NULL)
						printf("Expected columns_arr[%d] col_ptr was NULL\n", i);
					else
					{
						printf("Actual columns_arr[i] col_ptr (%s) did not equal expected columns_arr[i] col_ptr (%s) at i = %d\n"
							   ,((struct table_cols_info*) actual_col_ptr)->col_name, ((struct table_cols_info*) exp_col_ptr)->col_name, i);
					}
					freeAnyLinkedList((void**) &select_node, PTR_TYPE_SELECT_NODE, NULL, malloced_head, the_debug);
					setOutputWhite(); return -1;
				}
			}
				

			// START Check column new name
			if (cur_actual_select->columns_arr[i]->new_name == NULL && cur_expected_select->columns_arr[i]->new_name != NULL)
			{
				printf("test_Controller_parseSelect with id = %d FAILED\n", test_id);
				printf("cur actual columns_arr[%d]->new_name was NULL and cur exp columns_arr[%d]->new_name was NOT NULL\n", i, i);
				freeAnyLinkedList((void**) &select_node, PTR_TYPE_SELECT_NODE, NULL, malloced_head, the_debug);
				setOutputWhite(); return -1;
			}
			else if (cur_actual_select->columns_arr[i]->new_name != NULL && cur_expected_select->columns_arr[i]->new_name == NULL)
			{
				printf("test_Controller_parseSelect with id = %d FAILED\n", test_id);
				printf("cur actual columns_arr[%d]->new_name was NOT NULL and cur exp columns_arr[%d]->new_name was NULL\n", i, i);
				freeAnyLinkedList((void**) &select_node, PTR_TYPE_SELECT_NODE, NULL, malloced_head, the_debug);
				setOutputWhite(); return -1;
			}
			else if (cur_actual_select->columns_arr[i]->new_name != NULL 
					&& strcmp(cur_actual_select->columns_arr[i]->new_name, cur_expected_select->columns_arr[i]->new_name) != 0)
			{
				printf("test_Controller_parseSelect with id = %d FAILED\n", test_id);
				printf("Actual columns_arr[%d]->new_name _%s_ did not equal below\n", i, cur_actual_select->columns_arr[i]->new_name);
				printf("Expected columns_arr[%d]->new_name _%s_\n", i, cur_expected_select->columns_arr[i]->new_name);
				freeAnyLinkedList((void**) &select_node, PTR_TYPE_SELECT_NODE, NULL, malloced_head, the_debug);
				setOutputWhite(); return -1;
			}
			// END Check column new name

			// START Check column func node
			if (cur_actual_select->columns_arr[i]->func_node == NULL && cur_expected_select->columns_arr[i]->func_node != NULL)
			{
				printf("test_Controller_parseSelect with id = %d FAILED\n", test_id);
				printf("cur actual columns_arr[%d]->func_node was NULL and cur exp columns_arr[%d]->func_node was NOT NULL\n", i, i);
				freeAnyLinkedList((void**) &select_node, PTR_TYPE_SELECT_NODE, NULL, malloced_head, the_debug);
				setOutputWhite(); return -1;
			}
			else if (cur_actual_select->columns_arr[i]->func_node != NULL && cur_expected_select->columns_arr[i]->func_node == NULL)
			{
				printf("test_Controller_parseSelect with id = %d FAILED\n", test_id);
				printf("cur actual columns_arr[%d]->func_node was NOT NULL and cur exp columns_arr[%d]->func_node was NULL\n", i, i);
				freeAnyLinkedList((void**) &select_node, PTR_TYPE_SELECT_NODE, NULL, malloced_head, the_debug);
				setOutputWhite(); return -1;
			}
			else if (cur_actual_select->columns_arr[i]->func_node != NULL && cur_expected_select->columns_arr[i]->func_node != NULL)
			{
				if (cur_actual_select->columns_arr[i]->func_node->which_func != cur_expected_select->columns_arr[i]->func_node->which_func)
				{
					printf("test_Controller_parseSelect with id = %d FAILED\n", test_id);
					printf("Actual columns_arr[%d]->func_node->which_func (%d) did not equal below\n", i, cur_actual_select->columns_arr[i]->func_node->which_func);
					printf("Expected columns_arr[%d]->func_node->which_func (%d)\n", i, cur_expected_select->columns_arr[i]->func_node->which_func);
					freeAnyLinkedList((void**) &select_node, PTR_TYPE_SELECT_NODE, NULL, malloced_head, the_debug);
					setOutputWhite(); return -1;
				}

				if (cur_actual_select->columns_arr[i]->func_node->distinct != cur_expected_select->columns_arr[i]->func_node->distinct)
				{
					printf("test_Controller_parseSelect with id = %d FAILED\n", test_id);
					printf("Actual columns_arr[%d]->func_node->distinct (%s) did not equal below\n", i, cur_actual_select->columns_arr[i]->func_node->distinct ? "TRUE" : "FALSE");
					printf("Expected columns_arr[%d]->func_node->distinct (%s)\n", i, cur_expected_select->columns_arr[i]->func_node->distinct ? "TRUE" : "FALSE");
					freeAnyLinkedList((void**) &select_node, PTR_TYPE_SELECT_NODE, NULL, malloced_head, the_debug);
					setOutputWhite(); return -1;
				}

				if (cur_actual_select->columns_arr[i]->func_node->args_size != cur_expected_select->columns_arr[i]->func_node->args_size)
				{
					printf("test_Controller_parseSelect with id = %d FAILED\n", test_id);
					printf("Actual columns_arr[%d]->func_node->args_size (%d) did not equal below\n", i, cur_actual_select->columns_arr[i]->func_node->args_size);
					printf("Expected columns_arr[%d]->func_node->args_size (%d)\n", i, cur_expected_select->columns_arr[i]->func_node->args_size);
					freeAnyLinkedList((void**) &select_node, PTR_TYPE_SELECT_NODE, NULL, malloced_head, the_debug);
					setOutputWhite(); return -1;
				}

				for (int j=0; j<cur_actual_select->columns_arr[i]->func_node->args_size; j++)
				{
					if (strcmp(cur_actual_select->columns_arr[i]->func_node->args_arr[j], cur_expected_select->columns_arr[i]->func_node->args_arr[j]) != 0)
					{
						printf("test_Controller_parseSelect with id = %d FAILED\n", test_id);
						printf("Actual columns_arr[%d]->func_node->args_arr[j] (%s) did not equal below\n", i, cur_actual_select->columns_arr[i]->func_node->args_arr[j]);
						printf("Expected columns_arr[%d]->func_node->args_arr[j] (%s)\n", i, cur_expected_select->columns_arr[i]->func_node->args_arr[j]);
						freeAnyLinkedList((void**) &select_node, PTR_TYPE_SELECT_NODE, NULL, malloced_head, the_debug);
						setOutputWhite(); return -1;
					}
				}

				struct ListNodePtr* cur_act_group_by = cur_actual_select->columns_arr[i]->func_node->group_by_cols_head;
				struct ListNodePtr* cur_exp_group_by = cur_expected_select->columns_arr[i]->func_node->group_by_cols_head;

				while (cur_act_group_by != NULL || cur_exp_group_by != NULL)
				{
					if (cur_act_group_by == NULL && cur_exp_group_by != NULL)
					{
						printf("test_Controller_parseSelect with id = %d FAILED\n", test_id);
						printf("cur_act_group_by was NULL and cur_exp_group_by was NOT NULL\n");
						freeAnyLinkedList((void**) &select_node, PTR_TYPE_SELECT_NODE, NULL, malloced_head, the_debug);
						setOutputWhite(); return -1;
					}
					else if (cur_act_group_by != NULL && cur_exp_group_by == NULL)
					{
						printf("test_Controller_parseSelect with id = %d FAILED\n", test_id);
						printf("cur_act_group_by was NOT NULL and cur_exp_group_by was NULL\n");
						freeAnyLinkedList((void**) &select_node, PTR_TYPE_SELECT_NODE, NULL, malloced_head, the_debug);
						setOutputWhite(); return -1;
					}

					if (cur_act_group_by->ptr_value == NULL && cur_exp_group_by->ptr_value != NULL)
					{
						printf("test_Controller_parseSelect with id = %d FAILED\n", test_id);
						printf("cur_act_group_by->ptr_value was NULL and cur_exp_group_by->ptr_value was NOT NULL\n");
						freeAnyLinkedList((void**) &select_node, PTR_TYPE_SELECT_NODE, NULL, malloced_head, the_debug);
						setOutputWhite(); return -1;
					}
					else if (cur_act_group_by->ptr_value != NULL && cur_exp_group_by->ptr_value == NULL)
					{
						printf("test_Controller_parseSelect with id = %d FAILED\n", test_id);
						printf("cur_act_group_by->ptr_value was NOT NULL and cur_exp_group_by->ptr_value was NULL\n");
						freeAnyLinkedList((void**) &select_node, PTR_TYPE_SELECT_NODE, NULL, malloced_head, the_debug);
						setOutputWhite(); return -1;
					}

					if (cur_act_group_by->ptr_type != cur_exp_group_by->ptr_type)
					{
						printf("test_Controller_parseSelect with id = %d FAILED\n", test_id);
						printf("cur_act_group_by->ptr_type (%d) did not equal below\n", cur_act_group_by->ptr_type);
						printf("cur_exp_group_by->ptr_type (%d)\n", cur_exp_group_by->ptr_type);
						freeAnyLinkedList((void**) &select_node, PTR_TYPE_SELECT_NODE, NULL, malloced_head, the_debug);
						setOutputWhite(); return -1;
					}

					void* act_ptr_value = NULL;
					void* exp_ptr_value = NULL;

					if (cur_act_group_by->ptr_type == PTR_TYPE_COL_IN_SELECT_NODE)
					{
						act_ptr_value = ((struct col_in_select_node*) cur_act_group_by->ptr_value)->col_ptr;
						exp_ptr_value = ((struct col_in_select_node*) cur_exp_group_by->ptr_value)->col_ptr;
					}

					if (act_ptr_value != exp_ptr_value)
					{
						printf("test_Controller_parseSelect with id = %d FAILED\n", test_id);
						if (act_ptr_value == NULL)
							printf("act_ptr_value col_ptr was NULL\n");
						else if (exp_ptr_value == NULL)
							printf("exp_ptr_value col_ptr was NULL\n");
						else
						{
							printf("act_ptr_value col_ptr (%s) did not equal exp_ptr_value col_ptr (%s) at i = %d\n"
								   ,((struct table_cols_info*) act_ptr_value)->col_name, ((struct table_cols_info*) exp_ptr_value)->col_name, i);
						}
						freeAnyLinkedList((void**) &select_node, PTR_TYPE_SELECT_NODE, NULL, malloced_head, the_debug);
						setOutputWhite(); return -1;
					}

					if (cur_act_group_by != NULL)
						cur_act_group_by = cur_act_group_by->next;
					if (cur_exp_group_by != NULL)
						cur_exp_group_by = cur_exp_group_by->next;
				}
			}
			// END Check column func node

			// START Check column math node
			if (cur_actual_select->columns_arr[i]->math_node == NULL && cur_expected_select->columns_arr[i]->math_node != NULL)
			{
				printf("test_Controller_parseSelect with id = %d FAILED\n", test_id);
				printf("cur actual columns_arr[%d]->math_node was NULL and cur exp columns_arr[%d]->math_node was NOT NULL\n", i, i);
				freeAnyLinkedList((void**) &select_node, PTR_TYPE_SELECT_NODE, NULL, malloced_head, the_debug);
				setOutputWhite(); return -1;
			}
			else if (cur_actual_select->columns_arr[i]->math_node != NULL && cur_expected_select->columns_arr[i]->math_node == NULL)
			{
				printf("test_Controller_parseSelect with id = %d FAILED\n", test_id);
				printf("cur actual columns_arr[%d]->math_node was NOT NULL and cur exp columns_arr[%d]->math_node was NULL\n", i, i);
				freeAnyLinkedList((void**) &select_node, PTR_TYPE_SELECT_NODE, NULL, malloced_head, the_debug);
				setOutputWhite(); return -1;
			}
			else if (cur_actual_select->columns_arr[i]->math_node != NULL && cur_expected_select->columns_arr[i]->math_node != NULL)
			{
				if (compMathOrWhereTree(test_id, PTR_TYPE_MATH_NODE, cur_actual_select->columns_arr[i]->math_node, cur_expected_select->columns_arr[i]->math_node) != 0)
				{
					freeAnyLinkedList((void**) &select_node, PTR_TYPE_SELECT_NODE, NULL, malloced_head, the_debug);
					setOutputWhite(); return -1;
				}
			}
			// END Check column math node
		}
		// END Check if columns match

		// START Check if join nodes match
		/*struct join_node* cur_actual_join = cur_actual_select->join_head;
		struct join_node* cur_expected_join = cur_expected_select->join_head;
		
		while (cur_actual_join != NULL || cur_expected_join != NULL)
		{
			if (cur_actual_join == NULL && cur_expected_join != NULL)
			{
				printf("test_Controller_parseSelect with id = %d FAILED\n", test_id);
				printf("cur_actual_join was NULL and cur_expected_join was NOT NULL\n");
				return -1;
			}
			else if (cur_actual_join != NULL && cur_expected_join == NULL)
			{
				printf("test_Controller_parseSelect with id = %d FAILED\n", test_id);
				printf("cur_actual_join was NOT NULL and cur_expected_join was NULL\n");
				return -1;
			}
			else
			{
				if (cur_actual_join->join_type != cur_expected_join->join_type)
				{
					printf("test_Controller_parseSelect with id = %d FAILED\n", test_id);
					printf("Actual join_type %d did not equal below\n", cur_actual_join->join_type);
					printf("Expected join_type %d\n", cur_expected_join->join_type);
					return -1;
				}

				if (cur_actual_join->select_from != NULL && cur_expected_join->select_from != NULL && strcmp(cur_actual_join->select_from->select_node_alias, cur_expected_join->select_from->select_node_alias) != 0)
				{
					printf("test_Controller_parseSelect with id = %d FAILED\n", test_id);
					printf("Actual select_from (%s) did not equal below\n", cur_actual_join->select_from == NULL ? "NULL" : cur_actual_join->select_from->select_node_alias);
					printf("Expected select_from (%s)\n", cur_expected_join->select_from == NULL ? "NULL" : cur_expected_join->select_from->select_node_alias);
					return -1;
				}
				else if (cur_actual_join->select_from != NULL && cur_expected_join->select_from == NULL)
				{
					printf("test_Controller_parseSelect with id = %d FAILED\n", test_id);
					printf("Actual select_from was NOT NULL and Expected select_from was NULL\n");
					return -1;
				}
				else if (cur_actual_join->select_from == NULL && cur_expected_join->select_from != NULL)
				{
					printf("test_Controller_parseSelect with id = %d FAILED\n", test_id);
					printf("Actual select_from was NULL and Expected select_from was NOT NULL\n");
					return -1;
				}

				if (cur_actual_join->select_joined != NULL && cur_expected_join->select_joined != NULL && strcmp(cur_actual_join->select_joined->select_node_alias, cur_expected_join->select_joined->select_node_alias) != 0)
				{
					printf("test_Controller_parseSelect with id = %d FAILED\n", test_id);
					printf("Actual select_joined (%s) did not equal below\n", cur_actual_join->select_joined == NULL ? "NULL" : cur_actual_join->select_joined->select_node_alias);
					printf("Expected select_joined (%s)\n", cur_expected_join->select_joined == NULL ? "NULL" : cur_expected_join->select_joined->select_node_alias);
					return -1;
				}
				else if (cur_actual_join->select_joined != NULL && cur_expected_join->select_joined == NULL)
				{
					printf("test_Controller_parseSelect with id = %d FAILED\n", test_id);
					printf("Actual select_joined was NOT NULL and Expected select_joined was NULL\n");
					return -1;
				}
				else if (cur_actual_join->select_joined == NULL && cur_expected_join->select_joined != NULL)
				{
					printf("test_Controller_parseSelect with id = %d FAILED\n", test_id);
					printf("Actual select_joined was NULL and Expected select_joined was NOT NULL\n");
					return -1;
				}
			}

			if (cur_actual_join != NULL)
				cur_actual_join = cur_actual_join->next;
			if (cur_expected_join != NULL)
				cur_expected_join = cur_expected_join->next;
		}*/
		// END Check if join nodes match

		if (cur_actual_select != NULL)
			cur_actual_select = cur_actual_select->next;
		if (cur_expected_select	!= NULL)
			cur_expected_select = cur_expected_select->next;
	}
	setOutputWhite();


	// START Free everything
	freeAnyLinkedList((void**) &select_node, PTR_TYPE_SELECT_NODE, NULL, malloced_head, the_debug);
	// END Free everything


	return 0;
}


/*int test_Driver_findValidRowsGivenWhere(int test_id, struct ListNode* expected_results, char* where_string
									   ,struct table_info* the_table, struct colDataNode*** table_data_arr
									   ,int_8* the_col_numbers, int_8* num_rows_in_result, int the_col_numbers_size
									   ,struct malloced_node** malloced_head, int the_debug)
{
	printf("Starting test with id = %d\n", test_id);
	int result = 0;

	int error_code;
	struct or_clause_node* or_head = parseWhereClause(where_string, NULL, &error_code, "where", malloced_head, the_debug);

	if (error_code != 0)
		return -1;

	//printf("Parsed where_string\n");

	struct ListNode* actual_results;
	struct ListNode* actual_results_tail;
	findValidRowsGivenWhere(&actual_results, &actual_results_tail
						   ,the_table, table_data_arr, or_head
						   ,the_col_numbers, num_rows_in_result, the_col_numbers_size, malloced_head, the_debug);

	//printf("findValidRowsGivenWhere returned %lu rows\n", *num_rows_in_result);

	int freed = 0;
	while (or_head != NULL)
	{
		//printf("Freeing or_head\n");
		while (or_head->and_head != NULL)
		{
			//printf("Freeing and_head\n");

			struct and_clause_node* temp = or_head->and_head;

			or_head->and_head = or_head->and_head->next;

			//printf("	Calling free\n");

			myFree((void**) &(temp->data_string), NULL, malloced_head, the_debug);

			//printf("	Calling free\n");

			myFree((void**) &temp, NULL, malloced_head, the_debug);

			freed+=2;

			//printf("Done and_head\n");
		}
		struct or_clause_node* temp = or_head;

		or_head = or_head->next;

		//printf("	Calling free\n");

		myFree((void**) &temp, NULL, malloced_head, the_debug);

		freed++;

		//printf("Done or_head\n");
	}
	if (the_debug == YES_DEBUG)
		printf("Freed %d from or_head\n", freed);

	struct ListNode* cur_expected = expected_results;
	struct ListNode* cur_actual = actual_results;

	while (cur_expected != NULL && cur_actual != NULL)
	{
		//printf("Actual row_id %lu\n", cur_actual->value);
		//printf("Expected row_id %lu\n", cur_expected->value);
		if (cur_expected->value != cur_actual->value)
		{
			printf("test_Driver_findValidRowsGivenWhere with id = %d FAILED\n", test_id);
			printf("Actual row_id %lu did not equal below\n", cur_actual->value);
			printf("Expected row_id %lu\n", cur_expected->value);
			//printf("Actual did not equal expected\n");
			result = -1;
			//break;
		}

		cur_expected = cur_expected->next;
		cur_actual = cur_actual->next;
	}

	if (cur_expected != NULL || cur_actual != NULL)
	{
		printf("test_Driver_findValidRowsGivenWhere with id = %d FAILED\n", test_id);
		if (cur_expected != NULL)
		{
			printf("cur_expected was NOT null while cur_actual was null\n");
			printf("next cur_expected = %lu\n", cur_expected->value);
		}
		else
		{
			printf("cur_expected was null while cur_actual was NOT null\n");
			printf("next cur_actual = %lu\n", cur_actual->value);
		}
		result = -1;
	}

	freed = freeListNodes(&actual_results, NULL, malloced_head, the_debug);
	if (the_debug == YES_DEBUG)
		printf("Freed %d from actual_results\n", freed);

	return result;
}

int test_Driver_updateRows(int test_id, char* expected_results_csv, char* input_string
						  ,struct malloced_node** malloced_head, int the_debug)
{
	printf("Starting test with id = %d\n", test_id);

	struct table_info* the_table = NULL;
	struct change_node_v2* change_head = NULL;
	struct or_clause_node* or_head = NULL;
	
	if (parseUpdate(input_string, &the_table, &change_head, &or_head, malloced_head, the_debug) != 0)
	{
		printf("test_Driver_updateRows with id = %d FAILED\n", test_id);
		printf("The test %d had a problem with parseUpdate()\n", test_id);
		return -1;
	}

	if (updateRows(the_table, change_head, or_head, malloced_head, the_debug) < 0)
	{
		printf("test_Driver_updateRows with id = %d FAILED\n", test_id);
		printf("The test %d had a problem with updateRows()\n", test_id);
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


	return selectAndCheckHash(expected_results_csv, test_id, malloced_head, the_debug);
}

int test_Driver_deleteRows(int test_id, char* input_string, char* expected_results_csv, int_8 expected_num_rows, int_8 expected_num_open
						  ,struct malloced_node** malloced_head, int the_debug)
{
	printf("Starting test with id = %d\n", test_id);

	struct table_info* table = NULL;
	struct or_clause_node* or_head = NULL;

	if (parseDelete(input_string, &or_head, &table
				   ,malloced_head, the_debug) != 0)
	{
		printf("test_Driver_deleteRows with id = %d FAILED\n", test_id);
		printf("The test had a problem with parseDelete()\n");
		return -1;
	}


	if (deleteRows(table, or_head, malloced_head, the_debug) < 0)
	{
		printf("test_Driver_deleteRows with id = %d FAILED\n", test_id);
		printf("The test had a problem with deleteRows()\n");
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


	int result = 0;

	struct table_cols_info* cur_col = table->table_cols_head;
	while (cur_col != NULL)
	{
		if (cur_col->num_rows != expected_num_rows)
		{
			printf("test_Driver_deleteRows with id = %d FAILED\n", test_id);
			printf("Actual num_rows %lu did not equal below\n", cur_col->num_rows);
			printf("Expected num_rows %lu\n", expected_num_rows);
			result = -1;
		}
		
		if (cur_col->num_open != expected_num_open)
		{
			printf("test_Driver_deleteRows with id = %d FAILED\n", test_id);
			printf("Actual num_open %lu did not equal below\n", cur_col->num_open);
			printf("Expected num_open %lu\n", expected_num_open);
			result = -1;
		}

		cur_col = cur_col->next;
	}


	return selectAndCheckHash(expected_results_csv, test_id, malloced_head, the_debug) + result;
}

int test_Driver_insertRows(int test_id, char* input_string, char* expected_results_csv, int_8 expected_num_rows, int_8 expected_num_open
						  ,struct malloced_node** malloced_head, int the_debug)
{
	printf("Starting test with id = %d\n", test_id);

	struct table_info* table = NULL;
	struct change_node_v2* change_head = NULL;

	if (parseInsert(input_string, &change_head, &table
				   ,malloced_head, the_debug) != 0)
	{
		printf("test_Driver_insertRows with id = %d FAILED\n", test_id);
		printf("The test had a problem with parseInsert()\n");
		return -1;
	}

	if (insertRows(table, change_head, malloced_head, the_debug) < 0)
	{
		printf("test_Driver_insertRows with id = %d FAILED\n", test_id);
		printf("The test had a problem with insertRows()\n");
		return -1;
	}

	while (change_head != NULL)
	{
		struct change_node_v2* temp = change_head;
		change_head = change_head->next;
		myFree((void**) &temp->data, NULL, malloced_head, the_debug);
		myFree((void**) &temp, NULL, malloced_head, the_debug);
	}

	if (*malloced_head != NULL)
		printf("insertRows() did NOT free everything\n");

	int result = 0;

	struct table_cols_info* cur_col = table->table_cols_head;
	while (cur_col != NULL)
	{
		if (cur_col->num_rows != expected_num_rows)
		{
			printf("test_Driver_insertRows with id = %d FAILED\n", test_id);
			printf("Actual num_rows %lu did not equal below\n", cur_col->num_rows);
			printf("Expected num_rows %lu\n", expected_num_rows);
			result = -1;
		}
		
		if (cur_col->num_open != expected_num_open)
		{
			printf("test_Driver_insertRows with id = %d FAILED\n", test_id);
			printf("Actual num_open %lu did not equal below\n", cur_col->num_open);
			printf("Expected num_open %lu\n", expected_num_open);
			result = -1;
		}

		cur_col = cur_col->next;
	}


	return selectAndCheckHash(expected_results_csv, test_id, malloced_head, the_debug) + result;
}

int test_Performance_Select(int test_id, char* select_string, struct malloced_node** malloced_head, int the_debug)
{
	printf("Starting test with id = %d\n", test_id);

	/*
	struct timespec ts1, tw1; // both C11 and POSIX
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &ts1); // POSIX
    clock_gettime(CLOCK_MONOTONIC, &tw1); // POSIX; use timespec_get in C11
    clock_t t1 = clock();*/

    /*// START Run the test
    struct select_node* select_node = (struct select_node*) myMalloc(sizeof(struct select_node), NULL, malloced_head, the_debug);
	select_node->columns_arr_size = 0;
	select_node->columns_table_ptrs_arr = NULL;
	select_node->columns_col_ptrs_arr = NULL;
	
	select_node->or_head = NULL;

	select_node->join_head = NULL;

	select_node->prev = NULL;
	select_node->next = NULL;

	if (parseSelect(select_string, &select_node
				   ,malloced_head, the_debug) != 0)
	{
		printf("test_Performance_Select with id = %d FAILED\n", test_id);
		printf("The test %d had a problem with parseSelect()\n", test_id);
		return -1;
	}

	/*
	int_8 num_rows_in_result = 0;
	struct colDataNode*** result = select(select_node->table, select_node->columns_col_numbers_arr, select_node->columns_arr_size
										  ,&num_rows_in_result, select_node->or_head, malloced_head, the_debug);
	if (result == NULL)
	{
		printf("test_Performance_Select with id = %d FAILED\n", test_id);
		printf("The test %d had a problem with select()\n", test_id);
		return -1;
	}
	// END Run the test*/

	/*
    struct timespec ts2, tw2;
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &ts2);
    clock_gettime(CLOCK_MONOTONIC, &tw2);
    clock_t t2 = clock();

    double dur = 1000.0 * (t2 - t1) / CLOCKS_PER_SEC;
    double posix_dur = 1000.0 * ts2.tv_sec + 1e-6 * ts2.tv_nsec
                       - (1000.0 * ts1.tv_sec + 1e-6 * ts1.tv_nsec);
    double posix_wall = 1000.0 * tw2.tv_sec + 1e-6 * tw2.tv_nsec
                        - (1000.0 * tw1.tv_sec + 1e-6 * tw1.tv_nsec);
 
    printf("   CPU time used (per clock()): %.2f ms\n", dur);
    printf("   CPU time used (per clock_gettime()): %.2f ms\n", posix_dur);
    printf("   Wall time passed: %.2f ms\n", posix_wall);*/

    /*
    // START Free stuff
	while (select_node->or_head != NULL)
	{
		while (select_node->or_head->and_head != NULL)
		{
			struct and_clause_node* temp = select_node->or_head->and_head;
			select_node->or_head->and_head = select_node->or_head->and_head->next;
			myFree((void**) &temp->data_string, NULL, malloced_head, the_debug);
			myFree((void**) &temp, NULL, malloced_head, the_debug);
		}
		struct or_clause_node* temp = select_node->or_head;
		select_node->or_head = select_node->or_head->next;
		myFree((void**) &temp, NULL, malloced_head, the_debug);
	}

	int_8 total_freed = 0;
	for (int j=select_node->columns_arr_size-1; j>-1; j--)
	{
		for (int i=num_rows_in_result-1; i>-1; i--)
		{
			myFree((void**) &(result[j][i]->row_data), NULL, malloced_head, the_debug);
			myFree((void**) &result[j][i], NULL, malloced_head, the_debug);
			total_freed += 2;
		}
		myFree((void**) &result[j], NULL, malloced_head, the_debug);
		total_freed++;
	}
	myFree((void**) &result, NULL, malloced_head, the_debug);
	total_freed++;

	if (the_debug == YES_DEBUG)
		printf("	Freed %lu things from result\n", total_freed);

	myFree((void**) &select_node->columns_table_ptrs_arr, NULL, malloced_head, the_debug);
	myFree((void**) &select_node->columns_col_ptrs_arr, NULL, malloced_head, the_debug);
	// END Free stuff*/

	/*return 0;
}*/


int test_Driver_main()
{
	int the_debug = YES_DEBUG;

	struct malloced_node* malloced_head = NULL;


	//initColumnsArrForComp(/*struct col_in_select_node*** columns_arr,*/ 10
	//					 ,&malloced_head, the_debug, 1, "hi2", 3, "hi4", 5, "hi6", 7, "hi8", 9, "hi10");


	/*
	if (test_Driver_setup(&malloced_head, the_debug) != 0)
		return -1;*/
	

	/*
	system("copy DB_Files_2_Test_Backups\\* DB_Files_2\\");*/


	int initd = initDB(&malloced_head, the_debug);
	if (initd == -1)
	{
		setOutputRed();
		printf("Database initialization had a problem with file i/o, please try again\n\n");
		setOutputWhite();
		return -1;
	}
	else if (initd == -2)
	{
		setOutputRed();
		printf("Database initialization had a problem with malloc, please try again\n\n");
		setOutputWhite();
		return -2;
	}
	else
	{
		setOutputGreen();
		printf("Successfully initialized database\n\n");
		setOutputWhite();
	}


	//traverseTablesInfoMemory();


	//traverseTablesInfoDisk(&malloced_head, the_debug);


	if (malloced_head != NULL)
	{
		if (the_debug == YES_DEBUG)
			printf("	ERROR in test_Driver_main() at line %d in %s\n", __LINE__, __FILE__);
		return -3;
	}

	int result = 0;


	
	printf ("Starting Functionality Tests\n\n");
	// START test_Controller_parseSelect
		// START Test with id = 501
			int parsed_error_code = 0;


			struct select_node* the_select_node;
			initSelectClauseForComp(&the_select_node, NULL, false, 0, NULL, getTablesHead(), PTR_TYPE_TABLE_INFO, NULL, NULL
								   ,&malloced_head, the_debug);
			initSelectClauseForComp(&the_select_node->next, NULL, false, 0, NULL, the_select_node, PTR_TYPE_SELECT_NODE, NULL, NULL
								   ,&malloced_head, the_debug);
			the_select_node->next->prev = the_select_node;


			if (test_Controller_parseSelect(501, "select * from alc_brands;", &the_select_node
										   ,&parsed_error_code, &malloced_head, the_debug) != 0)
				result = -1;

			if (parsed_error_code == 0)
			{
				freeAnyLinkedList((void**) &the_select_node, PTR_TYPE_SELECT_NODE, NULL, &malloced_head, the_debug);
			}

			if (malloced_head != NULL)
			{
				if (the_debug == YES_DEBUG)
				{
					setOutputRed();
					printf("	ERROR in test_Driver_main() at line %d in %s\n", __LINE__, __FILE__);
					setOutputWhite();
				}
				errorTeardown(NULL, &malloced_head, the_debug);
				return -3;
			}			
		// END Test with id = 501

		// START Test with id = 502
			initSelectClauseForComp(&the_select_node, "TBL", false, 0, NULL, getTablesHead(), PTR_TYPE_TABLE_INFO, NULL, NULL
								   ,&malloced_head, the_debug);

			struct col_in_select_node** col_arr = (struct col_in_select_node**) myMalloc(sizeof(struct col_in_select_node*) * 2, NULL, &malloced_head, the_debug);
			
			col_arr[0] = (struct col_in_select_node*) myMalloc(sizeof(struct col_in_select_node), NULL, &malloced_head, the_debug);
			col_arr[0]->table_ptr = the_select_node;
			col_arr[0]->table_ptr_type = PTR_TYPE_SELECT_NODE;
			col_arr[0]->col_ptr = ((struct select_node*) the_select_node)->columns_arr[0];
			col_arr[0]->col_ptr_type = PTR_TYPE_COL_IN_SELECT_NODE;
			col_arr[0]->new_name = upper("COL1", NULL, &malloced_head, the_debug);
			col_arr[0]->case_when_head = NULL;
			col_arr[0]->case_then_value = NULL;
			col_arr[0]->case_then_value_type = -1;
			col_arr[0]->func_node = NULL;
			col_arr[0]->math_node = NULL;

			col_arr[1] = (struct col_in_select_node*) myMalloc(sizeof(struct col_in_select_node), NULL, &malloced_head, the_debug);
			col_arr[1]->table_ptr = the_select_node;
			col_arr[1]->table_ptr_type = PTR_TYPE_SELECT_NODE;
			col_arr[1]->col_ptr = ((struct select_node*) the_select_node)->columns_arr[6];
			col_arr[1]->col_ptr_type = PTR_TYPE_COL_IN_SELECT_NODE;
			col_arr[1]->new_name = upper("COL2", NULL, &malloced_head, the_debug);
			col_arr[1]->case_when_head = NULL;
			col_arr[1]->case_then_value = NULL;
			col_arr[1]->case_then_value_type = -1;
			col_arr[1]->func_node = NULL;
			col_arr[1]->math_node = NULL;

			initSelectClauseForComp(&the_select_node->next, NULL, true, 2, col_arr, NULL, -1, NULL, NULL
								   ,&malloced_head, the_debug);
			the_select_node->next->prev = the_select_node;
			

			if (test_Controller_parseSelect(502, "select distinct tbl.Brand-name as col1, SUPERVISOR-CREDENTIAL col2 from alc_brands as tbl;", &the_select_node
										   ,&parsed_error_code, &malloced_head, the_debug) != 0)
				result = -1;

			if (parsed_error_code == 0)
			{
				freeAnyLinkedList((void**) &the_select_node, PTR_TYPE_SELECT_NODE, NULL, &malloced_head, the_debug);
			}

			if (malloced_head != NULL)
			{
				if (the_debug == YES_DEBUG)
				{
					setOutputRed();
					printf("	ERROR in test_Driver_main() at line %d in %s\n", __LINE__, __FILE__);
					setOutputWhite();
				}
				errorTeardown(NULL, &malloced_head, the_debug);
				return -3;
			}
		// END Test with id = 502

		// START Test with id = 503
			initSelectClauseForComp(&the_select_node, "TBL", false, 0, NULL, getTablesHead(), PTR_TYPE_TABLE_INFO, NULL, NULL
								   ,&malloced_head, the_debug);

			col_arr = (struct col_in_select_node**) myMalloc(sizeof(struct col_in_select_node*) * 2, NULL, &malloced_head, the_debug);
			
			col_arr[0] = (struct col_in_select_node*) myMalloc(sizeof(struct col_in_select_node), NULL, &malloced_head, the_debug);
			col_arr[0]->table_ptr = NULL;//the_select_node;
			col_arr[0]->table_ptr_type = -1;//PTR_TYPE_SELECT_NODE;
			col_arr[0]->col_ptr = NULL;//((struct select_node*) the_select_node)->columns_arr[0];
			col_arr[0]->col_ptr_type = -1;//PTR_TYPE_COL_IN_SELECT_NODE;
			col_arr[0]->new_name = NULL;//upper("COL1", NULL, &malloced_head, the_debug);
			col_arr[0]->case_when_head = NULL;
			col_arr[0]->case_then_value = NULL;
			col_arr[0]->case_then_value_type = -1;
			col_arr[0]->math_node = NULL;

			col_arr[0]->func_node = (struct func_node*) myMalloc(sizeof(struct func_node), NULL, &malloced_head, the_debug);
			col_arr[0]->func_node->which_func = FUNC_COUNT;
			col_arr[0]->func_node->distinct = false;
			col_arr[0]->func_node->args_size = 1;
			col_arr[0]->func_node->args_arr = (void**) myMalloc(sizeof(void*), NULL, &malloced_head, the_debug);
			col_arr[0]->func_node->args_arr[0] = upper("*", NULL, &malloced_head, the_debug);
			col_arr[0]->func_node->group_by_cols_head = NULL;
			col_arr[0]->func_node->group_by_cols_tail = NULL;
			addListNodePtr(&col_arr[0]->func_node->group_by_cols_head, &col_arr[0]->func_node->group_by_cols_tail
						  ,((struct select_node*) the_select_node)->columns_arr[1], PTR_TYPE_COL_IN_SELECT_NODE, ADDLISTNODE_TAIL
				  		  ,NULL, &malloced_head, the_debug);

			col_arr[1] = (struct col_in_select_node*) myMalloc(sizeof(struct col_in_select_node), NULL, &malloced_head, the_debug);
			col_arr[1]->table_ptr = NULL;//the_select_node;
			col_arr[1]->table_ptr_type = -1;//PTR_TYPE_SELECT_NODE;
			col_arr[1]->col_ptr = NULL;//myMalloc(sizeof(struct math_node), NULL, &malloced_head, the_debug);
			col_arr[1]->col_ptr_type = -1;//PTR_TYPE_MATH_NODE;
			col_arr[1]->new_name = upper("MATH", NULL, &malloced_head, the_debug);
			col_arr[1]->case_when_head = NULL;
			col_arr[1]->case_then_value = NULL;
			col_arr[1]->case_then_value_type = -1;
			col_arr[1]->func_node = NULL;

			col_arr[1]->math_node = (struct math_node*) myMalloc(sizeof(struct math_node), NULL, &malloced_head, the_debug);
			col_arr[1]->math_node->ptr_one = ((struct select_node*) the_select_node)->columns_arr[0];
			col_arr[1]->math_node->ptr_one_type = PTR_TYPE_COL_IN_SELECT_NODE;
			col_arr[1]->math_node->ptr_two = myMalloc(sizeof(int), NULL, &malloced_head, the_debug);
			*((int*) col_arr[1]->math_node->ptr_two) = 10000;
			col_arr[1]->math_node->ptr_two_type = PTR_TYPE_INT;
			col_arr[1]->math_node->operation = MATH_SUB;
			col_arr[1]->math_node->parent = NULL;
		

			initSelectClauseForComp(&the_select_node->next, NULL, false, 2, col_arr, NULL, -1, NULL, NULL
								   ,&malloced_head, the_debug);
			the_select_node->next->prev = the_select_node;

			if (test_Controller_parseSelect(503, "select count\n(* ), CT-REGISTRATION-NUMBER - 10000 math from alc_brands tbl group by CT-REGISTRATION-NUMBER;", &the_select_node
										   ,&parsed_error_code, &malloced_head, the_debug) != 0)
				result = -1;

			if (parsed_error_code == 0)
			{
				freeAnyLinkedList((void**) &the_select_node, PTR_TYPE_SELECT_NODE, NULL, &malloced_head, the_debug);
			}

			if (malloced_head != NULL)
			{
				if (the_debug == YES_DEBUG)
				{
					setOutputRed();
					printf("	ERROR in test_Driver_main() at line %d in %s\n", __LINE__, __FILE__);
					setOutputWhite();
				}
				errorTeardown(NULL, &malloced_head, the_debug);
				return -3;
			}
		// END Test with id = 503

		/*// START Test with id = 504
			the_select_node = (struct select_node*) myMalloc(sizeof(struct select_node), NULL, &malloced_head, the_debug);

			the_select_node->select_node_alias = NULL;
			the_select_node->columns_arr_size = 4;

			the_select_node->columns_table_ptrs_arr = (struct table_info**) myMalloc(sizeof(struct table_info*) * 4, NULL, &malloced_head, the_debug);
			the_select_node->columns_table_ptrs_arr[0] = getTablesHead();
			the_select_node->columns_table_ptrs_arr[1] = getTablesHead();
			the_select_node->columns_table_ptrs_arr[2] = getTablesHead();
			the_select_node->columns_table_ptrs_arr[3] = getTablesHead();

			the_select_node->columns_col_ptrs_arr = (struct table_cols_info**) myMalloc(sizeof(struct table_cols_info*) * 4, NULL, &malloced_head, the_debug);
			the_select_node->columns_col_ptrs_arr[0] = getTablesHead()->table_cols_head;
			the_select_node->columns_col_ptrs_arr[1] = getTablesHead()->table_cols_head->next->next->next->next->next->next;
			the_select_node->columns_col_ptrs_arr[2] = getTablesHead()->table_cols_head;
			the_select_node->columns_col_ptrs_arr[3] = getTablesHead()->table_cols_head->next->next->next->next->next->next;

			the_select_node->or_head = NULL;

			the_select_node->next = NULL;

			the_select_node->prev = (struct select_node*) myMalloc(sizeof(struct select_node), NULL, &malloced_head, the_debug);
			the_select_node->prev->select_node_alias = upper("TBL", NULL, &malloced_head, the_debug);
			
			the_select_node->prev->columns_arr_size = 7;

			the_select_node->prev->columns_table_ptrs_arr = (struct table_info**) myMalloc(sizeof(struct table_info*) * 7, NULL, &malloced_head, the_debug);
			the_select_node->prev->columns_table_ptrs_arr[0] = getTablesHead();
			the_select_node->prev->columns_table_ptrs_arr[1] = getTablesHead();
			the_select_node->prev->columns_table_ptrs_arr[2] = getTablesHead();
			the_select_node->prev->columns_table_ptrs_arr[3] = getTablesHead();
			the_select_node->prev->columns_table_ptrs_arr[4] = getTablesHead();
			the_select_node->prev->columns_table_ptrs_arr[5] = getTablesHead();
			the_select_node->prev->columns_table_ptrs_arr[6] = getTablesHead();

			the_select_node->prev->columns_col_ptrs_arr = (struct table_cols_info**) myMalloc(sizeof(struct table_cols_info*) * 7, NULL, &malloced_head, the_debug);
			the_select_node->prev->columns_col_ptrs_arr[0] = getTablesHead()->table_cols_head;
			the_select_node->prev->columns_col_ptrs_arr[1] = getTablesHead()->table_cols_head->next;
			the_select_node->prev->columns_col_ptrs_arr[2] = getTablesHead()->table_cols_head->next->next;
			the_select_node->prev->columns_col_ptrs_arr[3] = getTablesHead()->table_cols_head->next->next->next;
			the_select_node->prev->columns_col_ptrs_arr[4] = getTablesHead()->table_cols_head->next->next->next->next;
			the_select_node->prev->columns_col_ptrs_arr[5] = getTablesHead()->table_cols_head->next->next->next->next->next;
			the_select_node->prev->columns_col_ptrs_arr[6] = getTablesHead()->table_cols_head->next->next->next->next->next->next;

			the_select_node->prev->or_head = NULL;
			the_select_node->prev->join_head = NULL;

			the_select_node->prev->next = the_select_node;

			the_select_node->prev->prev = (struct select_node*) myMalloc(sizeof(struct select_node), NULL, &malloced_head, the_debug);
			the_select_node->prev->prev->select_node_alias = upper("TBL2", NULL, &malloced_head, the_debug);
			
			the_select_node->prev->prev->columns_arr_size = 7;

			the_select_node->prev->prev->columns_table_ptrs_arr = (struct table_info**) myMalloc(sizeof(struct table_info*) * 7, NULL, &malloced_head, the_debug);
			the_select_node->prev->prev->columns_table_ptrs_arr[0] = getTablesHead();
			the_select_node->prev->prev->columns_table_ptrs_arr[1] = getTablesHead();
			the_select_node->prev->prev->columns_table_ptrs_arr[2] = getTablesHead();
			the_select_node->prev->prev->columns_table_ptrs_arr[3] = getTablesHead();
			the_select_node->prev->prev->columns_table_ptrs_arr[4] = getTablesHead();
			the_select_node->prev->prev->columns_table_ptrs_arr[5] = getTablesHead();
			the_select_node->prev->prev->columns_table_ptrs_arr[6] = getTablesHead();

			the_select_node->prev->prev->columns_col_ptrs_arr = (struct table_cols_info**) myMalloc(sizeof(struct table_cols_info*) * 7, NULL, &malloced_head, the_debug);
			the_select_node->prev->prev->columns_col_ptrs_arr[0] = getTablesHead()->table_cols_head;
			the_select_node->prev->prev->columns_col_ptrs_arr[1] = getTablesHead()->table_cols_head->next;
			the_select_node->prev->prev->columns_col_ptrs_arr[2] = getTablesHead()->table_cols_head->next->next;
			the_select_node->prev->prev->columns_col_ptrs_arr[3] = getTablesHead()->table_cols_head->next->next->next;
			the_select_node->prev->prev->columns_col_ptrs_arr[4] = getTablesHead()->table_cols_head->next->next->next->next;
			the_select_node->prev->prev->columns_col_ptrs_arr[5] = getTablesHead()->table_cols_head->next->next->next->next->next;
			the_select_node->prev->prev->columns_col_ptrs_arr[6] = getTablesHead()->table_cols_head->next->next->next->next->next->next;

			the_select_node->prev->prev->or_head = NULL;
			the_select_node->prev->prev->join_head = NULL;

			the_select_node->prev->prev->next = the_select_node->prev;
			
			the_select_node->prev->prev->prev = NULL;

			the_select_node->join_head = (struct join_node*) myMalloc(sizeof(struct join_node), NULL, &malloced_head, the_debug);
			the_select_node->join_head->join_type = JOIN_INNER;
			the_select_node->join_head->select_from = the_select_node->prev;
			the_select_node->join_head->select_joined = the_select_node->prev->prev;

			the_select_node->join_head->next = NULL;
			the_select_node->join_head->on_clause_head = NULL;

			

			if (test_Controller_parseSelect(504, "select tbl.Brand-name, tbl.SUPERVISOR-CREDENTIAL, tbl2.Brand-name, tbl2.SUPERVISOR-CREDENTIAL from alc_brands tbl join alc_brands tbl2 on tbl.Brand-name = tbl2.Brand-name;", &(the_select_node->prev->prev)
										   ,&parsed_error_code, &malloced_head, the_debug) != 0)
				result = -1;

			if (parsed_error_code == 0)
			{
				while (the_select_node != NULL)
				{
					struct select_node* temp = the_select_node;
					the_select_node = the_select_node->prev;

					myFree((void**) &temp->columns_table_ptrs_arr, NULL, &malloced_head, the_debug);
					myFree((void**) &temp->columns_col_ptrs_arr, NULL, &malloced_head, the_debug);
					while (temp->join_head != NULL)
					{
						struct join_node* temp_joined = temp->join_head;
						temp->join_head = temp->join_head->next;

						myFree((void**) &temp_joined, NULL, &malloced_head, the_debug);
					}
					myFree((void**) &temp->select_node_alias, NULL, &malloced_head, the_debug);
					myFree((void**) &temp, NULL, &malloced_head, the_debug);
				}
			}

			if (malloced_head != NULL)
			{
				if (the_debug == YES_DEBUG)
					printf("	ERROR in test_Driver_main() at line %d in %s\n", __LINE__, __FILE__);
				return -3;
			}
		// END Test with id = 504

		// START Test with id = 505
			the_select_node = NULL;


			if (test_Controller_parseSelect(505, "select tbl.Brand-name, tbl.SUPERVISOR-CREDENTIAL, tbl2.Brand-name, tbl2.SUPERVISOR-CREDENTIAL from alc_brands tbl join alc_brands tbl2 on tbl.Brand-name awdaw= tbl2.Brand-name;", &the_select_node
										   ,&parsed_error_code, &malloced_head, the_debug) != 0)
				result = -1;


			if (malloced_head != NULL)
			{
				if (the_debug == YES_DEBUG)
					printf("	ERROR in test_Driver_main() at line %d in %s\n", __LINE__, __FILE__);
				return -3;
			}
		// END Test with id = 505

		// START Test with id = 506
			the_select_node = NULL;


			if (test_Controller_parseSelect(506, "select tbl.Brand-name, tbl.SUPERVISOR-CREDENTIAL, tbl2.Brand-name, tbl3.SUPERVISOR-CREDENTIAL from alc_brands tbl join alc_brands tbl2 on tbl.Brand-name = tbl2.Brand-name;", &the_select_node
										   ,&parsed_error_code, &malloced_head, the_debug) != 0)
				result = -1;


			if (malloced_head != NULL)
			{
				if (the_debug == YES_DEBUG)
					printf("	ERROR in test_Driver_main() at line %d in %s\n", __LINE__, __FILE__);
				return -3;
			}
		// END Test with id = 506

		// START Test with id = 507
			the_select_node = NULL;


			if (test_Controller_parseSelect(507, "select tbl.Brand-name, tbl.SUPERVISOR-CREDENTIAL, Brand-name, tbl2.SUPERVISOR-CREDENTIAL from alc_brands tbl join alc_brands tbl2 on tbl.Brand-name = tbl2.Brand-name;", &the_select_node
										   ,&parsed_error_code, &malloced_head, the_debug) != 0)
				result = -1;


			if (malloced_head != NULL)
			{
				if (the_debug == YES_DEBUG)
					printf("	ERROR in test_Driver_main() at line %d in %s\n", __LINE__, __FILE__);
				return -3;
			}
		// END Test with id = 507

		// START Test with id = 508
			the_select_node = (struct select_node*) myMalloc(sizeof(struct select_node), NULL, &malloced_head, the_debug);

			the_select_node->select_node_alias = NULL;
			the_select_node->columns_arr_size = 5;

			the_select_node->columns_table_ptrs_arr = (struct table_info**) myMalloc(sizeof(struct table_info*) * 5, NULL, &malloced_head, the_debug);
			the_select_node->columns_table_ptrs_arr[0] = getTablesHead();
			the_select_node->columns_table_ptrs_arr[1] = getTablesHead();
			the_select_node->columns_table_ptrs_arr[2] = getTablesHead();
			the_select_node->columns_table_ptrs_arr[3] = getTablesHead();
			the_select_node->columns_table_ptrs_arr[4] = getTablesHead();

			the_select_node->columns_col_ptrs_arr = (struct table_cols_info**) myMalloc(sizeof(struct table_cols_info*) * 5, NULL, &malloced_head, the_debug);
			the_select_node->columns_col_ptrs_arr[0] = getTablesHead()->table_cols_head;
			the_select_node->columns_col_ptrs_arr[1] = getTablesHead()->table_cols_head->next->next->next->next->next->next;
			the_select_node->columns_col_ptrs_arr[2] = getTablesHead()->table_cols_head;
			the_select_node->columns_col_ptrs_arr[3] = getTablesHead()->table_cols_head->next->next->next->next->next->next;
			the_select_node->columns_col_ptrs_arr[4] = getTablesHead()->table_cols_head->next;

			the_select_node->or_head = NULL;

			the_select_node->next = NULL;

			the_select_node->prev = (struct select_node*) myMalloc(sizeof(struct select_node), NULL, &malloced_head, the_debug);
			the_select_node->prev->select_node_alias = upper("TBL", NULL, &malloced_head, the_debug);
			
			the_select_node->prev->columns_arr_size = 7;

			the_select_node->prev->columns_table_ptrs_arr = (struct table_info**) myMalloc(sizeof(struct table_info*) * 7, NULL, &malloced_head, the_debug);
			the_select_node->prev->columns_table_ptrs_arr[0] = getTablesHead();
			the_select_node->prev->columns_table_ptrs_arr[1] = getTablesHead();
			the_select_node->prev->columns_table_ptrs_arr[2] = getTablesHead();
			the_select_node->prev->columns_table_ptrs_arr[3] = getTablesHead();
			the_select_node->prev->columns_table_ptrs_arr[4] = getTablesHead();
			the_select_node->prev->columns_table_ptrs_arr[5] = getTablesHead();
			the_select_node->prev->columns_table_ptrs_arr[6] = getTablesHead();

			the_select_node->prev->columns_col_ptrs_arr = (struct table_cols_info**) myMalloc(sizeof(struct table_cols_info*) * 7, NULL, &malloced_head, the_debug);
			the_select_node->prev->columns_col_ptrs_arr[0] = getTablesHead()->table_cols_head;
			the_select_node->prev->columns_col_ptrs_arr[1] = getTablesHead()->table_cols_head->next;
			the_select_node->prev->columns_col_ptrs_arr[2] = getTablesHead()->table_cols_head->next->next;
			the_select_node->prev->columns_col_ptrs_arr[3] = getTablesHead()->table_cols_head->next->next->next;
			the_select_node->prev->columns_col_ptrs_arr[4] = getTablesHead()->table_cols_head->next->next->next->next;
			the_select_node->prev->columns_col_ptrs_arr[5] = getTablesHead()->table_cols_head->next->next->next->next->next;
			the_select_node->prev->columns_col_ptrs_arr[6] = getTablesHead()->table_cols_head->next->next->next->next->next->next;

			the_select_node->prev->or_head = NULL;
			the_select_node->prev->join_head = NULL;

			the_select_node->prev->next = the_select_node;

			the_select_node->prev->prev = (struct select_node*) myMalloc(sizeof(struct select_node), NULL, &malloced_head, the_debug);
			the_select_node->prev->prev->select_node_alias = upper("TBL2", NULL, &malloced_head, the_debug);
			
			the_select_node->prev->prev->columns_arr_size = 7;

			the_select_node->prev->prev->columns_table_ptrs_arr = (struct table_info**) myMalloc(sizeof(struct table_info*) * 7, NULL, &malloced_head, the_debug);
			the_select_node->prev->prev->columns_table_ptrs_arr[0] = getTablesHead();
			the_select_node->prev->prev->columns_table_ptrs_arr[1] = getTablesHead();
			the_select_node->prev->prev->columns_table_ptrs_arr[2] = getTablesHead();
			the_select_node->prev->prev->columns_table_ptrs_arr[3] = getTablesHead();
			the_select_node->prev->prev->columns_table_ptrs_arr[4] = getTablesHead();
			the_select_node->prev->prev->columns_table_ptrs_arr[5] = getTablesHead();
			the_select_node->prev->prev->columns_table_ptrs_arr[6] = getTablesHead();

			the_select_node->prev->prev->columns_col_ptrs_arr = (struct table_cols_info**) myMalloc(sizeof(struct table_cols_info*) * 7, NULL, &malloced_head, the_debug);
			the_select_node->prev->prev->columns_col_ptrs_arr[0] = getTablesHead()->table_cols_head;
			the_select_node->prev->prev->columns_col_ptrs_arr[1] = getTablesHead()->table_cols_head->next;
			the_select_node->prev->prev->columns_col_ptrs_arr[2] = getTablesHead()->table_cols_head->next->next;
			the_select_node->prev->prev->columns_col_ptrs_arr[3] = getTablesHead()->table_cols_head->next->next->next;
			the_select_node->prev->prev->columns_col_ptrs_arr[4] = getTablesHead()->table_cols_head->next->next->next->next;
			the_select_node->prev->prev->columns_col_ptrs_arr[5] = getTablesHead()->table_cols_head->next->next->next->next->next;
			the_select_node->prev->prev->columns_col_ptrs_arr[6] = getTablesHead()->table_cols_head->next->next->next->next->next->next;

			the_select_node->prev->prev->or_head = NULL;
			the_select_node->prev->prev->join_head = NULL;

			the_select_node->prev->prev->next = the_select_node->prev;
			
			the_select_node->prev->prev->prev = (struct select_node*) myMalloc(sizeof(struct select_node), NULL, &malloced_head, the_debug);
			the_select_node->prev->prev->prev->select_node_alias = upper("TBL3", NULL, &malloced_head, the_debug);
			
			the_select_node->prev->prev->prev->columns_arr_size = 7;

			the_select_node->prev->prev->prev->columns_table_ptrs_arr = (struct table_info**) myMalloc(sizeof(struct table_info*) * 7, NULL, &malloced_head, the_debug);
			the_select_node->prev->prev->prev->columns_table_ptrs_arr[0] = getTablesHead();
			the_select_node->prev->prev->prev->columns_table_ptrs_arr[1] = getTablesHead();
			the_select_node->prev->prev->prev->columns_table_ptrs_arr[2] = getTablesHead();
			the_select_node->prev->prev->prev->columns_table_ptrs_arr[3] = getTablesHead();
			the_select_node->prev->prev->prev->columns_table_ptrs_arr[4] = getTablesHead();
			the_select_node->prev->prev->prev->columns_table_ptrs_arr[5] = getTablesHead();
			the_select_node->prev->prev->prev->columns_table_ptrs_arr[6] = getTablesHead();

			the_select_node->prev->prev->prev->columns_col_ptrs_arr = (struct table_cols_info**) myMalloc(sizeof(struct table_cols_info*) * 7, NULL, &malloced_head, the_debug);
			the_select_node->prev->prev->prev->columns_col_ptrs_arr[0] = getTablesHead()->table_cols_head;
			the_select_node->prev->prev->prev->columns_col_ptrs_arr[1] = getTablesHead()->table_cols_head->next;
			the_select_node->prev->prev->prev->columns_col_ptrs_arr[2] = getTablesHead()->table_cols_head->next->next;
			the_select_node->prev->prev->prev->columns_col_ptrs_arr[3] = getTablesHead()->table_cols_head->next->next->next;
			the_select_node->prev->prev->prev->columns_col_ptrs_arr[4] = getTablesHead()->table_cols_head->next->next->next->next;
			the_select_node->prev->prev->prev->columns_col_ptrs_arr[5] = getTablesHead()->table_cols_head->next->next->next->next->next;
			the_select_node->prev->prev->prev->columns_col_ptrs_arr[6] = getTablesHead()->table_cols_head->next->next->next->next->next->next;

			the_select_node->prev->prev->prev->or_head = NULL;
			the_select_node->prev->prev->prev->join_head = NULL;

			the_select_node->prev->prev->prev->next = the_select_node->prev->prev;
			the_select_node->prev->prev->prev->prev = NULL;


			the_select_node->join_head = (struct join_node*) myMalloc(sizeof(struct join_node), NULL, &malloced_head, the_debug);
			the_select_node->join_head->join_type = JOIN_INNER;
			the_select_node->join_head->select_from = the_select_node->prev;
			the_select_node->join_head->select_joined = the_select_node->prev->prev;

			the_select_node->join_head->on_clause_head = NULL;

			the_select_node->join_head->next = (struct join_node*) myMalloc(sizeof(struct join_node), NULL, &malloced_head, the_debug);
			the_select_node->join_head->next->join_type = JOIN_LEFT;
			the_select_node->join_head->next->select_from = the_select_node->prev;
			the_select_node->join_head->next->select_joined = the_select_node->prev->prev->prev;

			the_select_node->join_head->next->on_clause_head = NULL;

			the_select_node->join_head->next->next = NULL;


			if (test_Controller_parseSelect(508, "select tbl.Brand-name, tbl.SUPERVISOR-CREDENTIAL, tbl2.Brand-name, tbl2.SUPERVISOR-CREDENTIAL, tbl3.CT-REGISTRATION-NUMBER from alc_brands tbl join alc_brands tbl2 on tbl.Brand-name = tbl2.Brand-name left join alc_brands tbl3	on tbl.CT-REGISTRATION-NUMBER = tbl3.CT-REGISTRATION-NUMBER;", &(the_select_node->prev->prev->prev)
										   ,&parsed_error_code, &malloced_head, the_debug) != 0)
				result = -1;

			if (parsed_error_code == 0)
			{
				while (the_select_node != NULL)
				{
					struct select_node* temp = the_select_node;
					the_select_node = the_select_node->prev;

					myFree((void**) &temp->columns_table_ptrs_arr, NULL, &malloced_head, the_debug);
					myFree((void**) &temp->columns_col_ptrs_arr, NULL, &malloced_head, the_debug);
					while (temp->join_head != NULL)
					{
						struct join_node* temp_joined = temp->join_head;
						temp->join_head = temp->join_head->next;

						myFree((void**) &temp_joined, NULL, &malloced_head, the_debug);
					}
					myFree((void**) &temp->select_node_alias, NULL, &malloced_head, the_debug);
					myFree((void**) &temp, NULL, &malloced_head, the_debug);
				}
			}

			if (malloced_head != NULL)
			{
				if (the_debug == YES_DEBUG)
					printf("	ERROR in test_Driver_main() at line %d in %s\n", __LINE__, __FILE__);
				return -3;
			}
		// END Test with id = 508

		// START Test with id = 509
			the_select_node = NULL;


			if (test_Controller_parseSelect(509, "select tbl.Brand-name, tbl.SUPERVISOR-CREDENTIAL, tbl2.Brand-name, tbl2.SUPERVISOR-CREDENTIAL, tbl3.CT-REGISTRATION-NUMBER from alc_brands tbl join alc_brands tbl2 on tbl.Brand-name = tbl3.Brand-name left join alc_brands tbl3	on tbl.CT-REGISTRATION-NUMBER = tbl3.CT-REGISTRATION-NUMBER;", &the_select_node
										   ,&parsed_error_code, &malloced_head, the_debug) != 0)
				result = -1;


			if (malloced_head != NULL)
			{
				if (the_debug == YES_DEBUG)
					printf("	ERROR in test_Driver_main() at line %d in %s\n", __LINE__, __FILE__);
				return -3;
			}
		// END Test with id = 509

		// START Test with id = 510
			the_select_node = (struct select_node*) myMalloc(sizeof(struct select_node), NULL, &malloced_head, the_debug);
			the_select_node->select_node_alias = NULL;

			the_select_node->columns_arr_size = 7;

			the_select_node->columns_table_ptrs_arr = (struct table_info**) myMalloc(sizeof(struct table_info*) * 7, NULL, &malloced_head, the_debug);
			the_select_node->columns_table_ptrs_arr[0] = getTablesHead();
			the_select_node->columns_table_ptrs_arr[1] = getTablesHead();
			the_select_node->columns_table_ptrs_arr[2] = getTablesHead();
			the_select_node->columns_table_ptrs_arr[3] = getTablesHead();
			the_select_node->columns_table_ptrs_arr[4] = getTablesHead();
			the_select_node->columns_table_ptrs_arr[5] = getTablesHead();
			the_select_node->columns_table_ptrs_arr[6] = getTablesHead();

			the_select_node->columns_col_ptrs_arr = (struct table_cols_info**) myMalloc(sizeof(struct table_cols_info*) * 7, NULL, &malloced_head, the_debug);
			the_select_node->columns_col_ptrs_arr[0] = getTablesHead()->table_cols_head;
			the_select_node->columns_col_ptrs_arr[1] = getTablesHead()->table_cols_head->next;
			the_select_node->columns_col_ptrs_arr[2] = getTablesHead()->table_cols_head->next->next;
			the_select_node->columns_col_ptrs_arr[3] = getTablesHead()->table_cols_head->next->next->next;
			the_select_node->columns_col_ptrs_arr[4] = getTablesHead()->table_cols_head->next->next->next->next;
			the_select_node->columns_col_ptrs_arr[5] = getTablesHead()->table_cols_head->next->next->next->next->next;
			the_select_node->columns_col_ptrs_arr[6] = getTablesHead()->table_cols_head->next->next->next->next->next->next;

			the_select_node->or_head = NULL;
			the_select_node->join_head = NULL;

			the_select_node->next = NULL;

			the_select_node->prev = (struct select_node*) myMalloc(sizeof(struct select_node), NULL, &malloced_head, the_debug);
			the_select_node->prev->select_node_alias = NULL;

			the_select_node->prev->columns_arr_size = 7;

			the_select_node->prev->columns_table_ptrs_arr = (struct table_info**) myMalloc(sizeof(struct table_info*) * 7, NULL, &malloced_head, the_debug);
			the_select_node->prev->columns_table_ptrs_arr[0] = getTablesHead();
			the_select_node->prev->columns_table_ptrs_arr[1] = getTablesHead();
			the_select_node->prev->columns_table_ptrs_arr[2] = getTablesHead();
			the_select_node->prev->columns_table_ptrs_arr[3] = getTablesHead();
			the_select_node->prev->columns_table_ptrs_arr[4] = getTablesHead();
			the_select_node->prev->columns_table_ptrs_arr[5] = getTablesHead();
			the_select_node->prev->columns_table_ptrs_arr[6] = getTablesHead();

			the_select_node->prev->columns_col_ptrs_arr = (struct table_cols_info**) myMalloc(sizeof(struct table_cols_info*) * 7, NULL, &malloced_head, the_debug);
			the_select_node->prev->columns_col_ptrs_arr[0] = getTablesHead()->table_cols_head;
			the_select_node->prev->columns_col_ptrs_arr[1] = getTablesHead()->table_cols_head->next;
			the_select_node->prev->columns_col_ptrs_arr[2] = getTablesHead()->table_cols_head->next->next;
			the_select_node->prev->columns_col_ptrs_arr[3] = getTablesHead()->table_cols_head->next->next->next;
			the_select_node->prev->columns_col_ptrs_arr[4] = getTablesHead()->table_cols_head->next->next->next->next;
			the_select_node->prev->columns_col_ptrs_arr[5] = getTablesHead()->table_cols_head->next->next->next->next->next;
			the_select_node->prev->columns_col_ptrs_arr[6] = getTablesHead()->table_cols_head->next->next->next->next->next->next;

			the_select_node->prev->or_head = NULL;
			the_select_node->prev->join_head = NULL;

			the_select_node->prev->next = the_select_node;
			the_select_node->prev->prev = NULL;


			if (test_Controller_parseSelect(510, "select tbl.* from alc_brands tbl;", &(the_select_node->prev)
										   ,&parsed_error_code, &malloced_head, the_debug) != 0)
				result = -1;


			if (parsed_error_code == 0)
			{
				while (the_select_node != NULL)
				{
					struct select_node* temp = the_select_node;
					the_select_node = the_select_node->prev;

					myFree((void**) &temp->columns_table_ptrs_arr, NULL, &malloced_head, the_debug);
					myFree((void**) &temp->columns_col_ptrs_arr, NULL, &malloced_head, the_debug);
					myFree((void**) &temp->select_node_alias, NULL, &malloced_head, the_debug);
					myFree((void**) &temp, NULL, &malloced_head, the_debug);
				}
			}


			if (malloced_head != NULL)
			{
				if (the_debug == YES_DEBUG)
					printf("	ERROR in test_Driver_main() at line %d in %s\n", __LINE__, __FILE__);
				return -3;
			}
		// END Test with id = 510

		// START Test with id = 511
			the_select_node = (struct select_node*) myMalloc(sizeof(struct select_node), NULL, &malloced_head, the_debug);

			the_select_node->select_node_alias = NULL;
			the_select_node->columns_arr_size = 7;

			the_select_node->columns_table_ptrs_arr = (struct table_info**) myMalloc(sizeof(struct table_info*) * 7, NULL, &malloced_head, the_debug);
			the_select_node->columns_table_ptrs_arr[0] = getTablesHead();
			the_select_node->columns_table_ptrs_arr[1] = getTablesHead();
			the_select_node->columns_table_ptrs_arr[2] = getTablesHead();
			the_select_node->columns_table_ptrs_arr[3] = getTablesHead();
			the_select_node->columns_table_ptrs_arr[4] = getTablesHead();
			the_select_node->columns_table_ptrs_arr[5] = getTablesHead();
			the_select_node->columns_table_ptrs_arr[6] = getTablesHead();

			the_select_node->columns_col_ptrs_arr = (struct table_cols_info**) myMalloc(sizeof(struct table_cols_info*) * 7, NULL, &malloced_head, the_debug);
			the_select_node->columns_col_ptrs_arr[0] = getTablesHead()->table_cols_head;
			the_select_node->columns_col_ptrs_arr[1] = getTablesHead()->table_cols_head->next;
			the_select_node->columns_col_ptrs_arr[2] = getTablesHead()->table_cols_head->next->next;
			the_select_node->columns_col_ptrs_arr[3] = getTablesHead()->table_cols_head->next->next->next;
			the_select_node->columns_col_ptrs_arr[4] = getTablesHead()->table_cols_head->next->next->next->next;
			the_select_node->columns_col_ptrs_arr[5] = getTablesHead()->table_cols_head->next->next->next->next->next;
			the_select_node->columns_col_ptrs_arr[6] = getTablesHead()->table_cols_head->next->next->next->next->next->next;

			the_select_node->or_head = NULL;

			the_select_node->next = NULL;

			the_select_node->prev = (struct select_node*) myMalloc(sizeof(struct select_node), NULL, &malloced_head, the_debug);
			the_select_node->prev->select_node_alias = upper("TBL", NULL, &malloced_head, the_debug);
			
			the_select_node->prev->columns_arr_size = 7;

			the_select_node->prev->columns_table_ptrs_arr = (struct table_info**) myMalloc(sizeof(struct table_info*) * 7, NULL, &malloced_head, the_debug);
			the_select_node->prev->columns_table_ptrs_arr[0] = getTablesHead();
			the_select_node->prev->columns_table_ptrs_arr[1] = getTablesHead();
			the_select_node->prev->columns_table_ptrs_arr[2] = getTablesHead();
			the_select_node->prev->columns_table_ptrs_arr[3] = getTablesHead();
			the_select_node->prev->columns_table_ptrs_arr[4] = getTablesHead();
			the_select_node->prev->columns_table_ptrs_arr[5] = getTablesHead();
			the_select_node->prev->columns_table_ptrs_arr[6] = getTablesHead();

			the_select_node->prev->columns_col_ptrs_arr = (struct table_cols_info**) myMalloc(sizeof(struct table_cols_info*) * 7, NULL, &malloced_head, the_debug);
			the_select_node->prev->columns_col_ptrs_arr[0] = getTablesHead()->table_cols_head;
			the_select_node->prev->columns_col_ptrs_arr[1] = getTablesHead()->table_cols_head->next;
			the_select_node->prev->columns_col_ptrs_arr[2] = getTablesHead()->table_cols_head->next->next;
			the_select_node->prev->columns_col_ptrs_arr[3] = getTablesHead()->table_cols_head->next->next->next;
			the_select_node->prev->columns_col_ptrs_arr[4] = getTablesHead()->table_cols_head->next->next->next->next;
			the_select_node->prev->columns_col_ptrs_arr[5] = getTablesHead()->table_cols_head->next->next->next->next->next;
			the_select_node->prev->columns_col_ptrs_arr[6] = getTablesHead()->table_cols_head->next->next->next->next->next->next;

			the_select_node->prev->or_head = NULL;
			the_select_node->prev->join_head = NULL;

			the_select_node->prev->next = the_select_node;

			the_select_node->prev->prev = (struct select_node*) myMalloc(sizeof(struct select_node), NULL, &malloced_head, the_debug);
			the_select_node->prev->prev->select_node_alias = upper("TBL2", NULL, &malloced_head, the_debug);
			
			the_select_node->prev->prev->columns_arr_size = 7;

			the_select_node->prev->prev->columns_table_ptrs_arr = (struct table_info**) myMalloc(sizeof(struct table_info*) * 7, NULL, &malloced_head, the_debug);
			the_select_node->prev->prev->columns_table_ptrs_arr[0] = getTablesHead();
			the_select_node->prev->prev->columns_table_ptrs_arr[1] = getTablesHead();
			the_select_node->prev->prev->columns_table_ptrs_arr[2] = getTablesHead();
			the_select_node->prev->prev->columns_table_ptrs_arr[3] = getTablesHead();
			the_select_node->prev->prev->columns_table_ptrs_arr[4] = getTablesHead();
			the_select_node->prev->prev->columns_table_ptrs_arr[5] = getTablesHead();
			the_select_node->prev->prev->columns_table_ptrs_arr[6] = getTablesHead();

			the_select_node->prev->prev->columns_col_ptrs_arr = (struct table_cols_info**) myMalloc(sizeof(struct table_cols_info*) * 7, NULL, &malloced_head, the_debug);
			the_select_node->prev->prev->columns_col_ptrs_arr[0] = getTablesHead()->table_cols_head;
			the_select_node->prev->prev->columns_col_ptrs_arr[1] = getTablesHead()->table_cols_head->next;
			the_select_node->prev->prev->columns_col_ptrs_arr[2] = getTablesHead()->table_cols_head->next->next;
			the_select_node->prev->prev->columns_col_ptrs_arr[3] = getTablesHead()->table_cols_head->next->next->next;
			the_select_node->prev->prev->columns_col_ptrs_arr[4] = getTablesHead()->table_cols_head->next->next->next->next;
			the_select_node->prev->prev->columns_col_ptrs_arr[5] = getTablesHead()->table_cols_head->next->next->next->next->next;
			the_select_node->prev->prev->columns_col_ptrs_arr[6] = getTablesHead()->table_cols_head->next->next->next->next->next->next;

			the_select_node->prev->prev->or_head = NULL;
			the_select_node->prev->prev->join_head = NULL;

			the_select_node->prev->prev->next = the_select_node->prev;
			
			the_select_node->prev->prev->prev = NULL;

			the_select_node->join_head = (struct join_node*) myMalloc(sizeof(struct join_node), NULL, &malloced_head, the_debug);
			the_select_node->join_head->join_type = JOIN_INNER;
			the_select_node->join_head->select_from = the_select_node->prev;
			the_select_node->join_head->select_joined = the_select_node->prev->prev;

			the_select_node->join_head->next = NULL;
			the_select_node->join_head->on_clause_head = NULL;


			if (test_Controller_parseSelect(511, "select tbl.* from alc_brands tbl join alc_brands tbl2 on tbl.Brand-name = tbl2.Brand-name;", &(the_select_node->prev->prev)
										   ,&parsed_error_code, &malloced_head, the_debug) != 0)
				result = -1;


			if (parsed_error_code == 0)
			{
				while (the_select_node != NULL)
				{
					struct select_node* temp = the_select_node;
					the_select_node = the_select_node->prev;

					myFree((void**) &temp->columns_table_ptrs_arr, NULL, &malloced_head, the_debug);
					myFree((void**) &temp->columns_col_ptrs_arr, NULL, &malloced_head, the_debug);
					while (temp->join_head != NULL)
					{
						struct join_node* temp_joined = temp->join_head;
						temp->join_head = temp->join_head->next;

						myFree((void**) &temp_joined, NULL, &malloced_head, the_debug);
					}
					myFree((void**) &temp->select_node_alias, NULL, &malloced_head, the_debug);
					myFree((void**) &temp, NULL, &malloced_head, the_debug);
				}
			}


			if (malloced_head != NULL)
			{
				if (the_debug == YES_DEBUG)
					printf("	ERROR in test_Driver_main() at line %d in %s\n", __LINE__, __FILE__);
				return -3;
			}
		// END Test with id = 511

		// START Test with id = 512
			the_select_node = (struct select_node*) myMalloc(sizeof(struct select_node), NULL, &malloced_head, the_debug);

			the_select_node->select_node_alias = NULL;
			the_select_node->columns_arr_size = 14;

			the_select_node->columns_table_ptrs_arr = (struct table_info**) myMalloc(sizeof(struct table_info*) * 14, NULL, &malloced_head, the_debug);
			the_select_node->columns_table_ptrs_arr[0] = getTablesHead();
			the_select_node->columns_table_ptrs_arr[1] = getTablesHead();
			the_select_node->columns_table_ptrs_arr[2] = getTablesHead();
			the_select_node->columns_table_ptrs_arr[3] = getTablesHead();
			the_select_node->columns_table_ptrs_arr[4] = getTablesHead();
			the_select_node->columns_table_ptrs_arr[5] = getTablesHead();
			the_select_node->columns_table_ptrs_arr[6] = getTablesHead();
			the_select_node->columns_table_ptrs_arr[7] = getTablesHead();
			the_select_node->columns_table_ptrs_arr[8] = getTablesHead();
			the_select_node->columns_table_ptrs_arr[9] = getTablesHead();
			the_select_node->columns_table_ptrs_arr[10] = getTablesHead();
			the_select_node->columns_table_ptrs_arr[11] = getTablesHead();
			the_select_node->columns_table_ptrs_arr[12] = getTablesHead();
			the_select_node->columns_table_ptrs_arr[13] = getTablesHead();

			the_select_node->columns_col_ptrs_arr = (struct table_cols_info**) myMalloc(sizeof(struct table_cols_info*) * 14, NULL, &malloced_head, the_debug);
			the_select_node->columns_col_ptrs_arr[0] = getTablesHead()->table_cols_head;
			the_select_node->columns_col_ptrs_arr[1] = getTablesHead()->table_cols_head->next;
			the_select_node->columns_col_ptrs_arr[2] = getTablesHead()->table_cols_head->next->next;
			the_select_node->columns_col_ptrs_arr[3] = getTablesHead()->table_cols_head->next->next->next;
			the_select_node->columns_col_ptrs_arr[4] = getTablesHead()->table_cols_head->next->next->next->next;
			the_select_node->columns_col_ptrs_arr[5] = getTablesHead()->table_cols_head->next->next->next->next->next;
			the_select_node->columns_col_ptrs_arr[6] = getTablesHead()->table_cols_head->next->next->next->next->next->next;
			the_select_node->columns_col_ptrs_arr[7] = getTablesHead()->table_cols_head;
			the_select_node->columns_col_ptrs_arr[8] = getTablesHead()->table_cols_head->next;
			the_select_node->columns_col_ptrs_arr[9] = getTablesHead()->table_cols_head->next->next;
			the_select_node->columns_col_ptrs_arr[10] = getTablesHead()->table_cols_head->next->next->next;
			the_select_node->columns_col_ptrs_arr[11] = getTablesHead()->table_cols_head->next->next->next->next;
			the_select_node->columns_col_ptrs_arr[12] = getTablesHead()->table_cols_head->next->next->next->next->next;
			the_select_node->columns_col_ptrs_arr[13] = getTablesHead()->table_cols_head->next->next->next->next->next->next;

			the_select_node->or_head = NULL;

			the_select_node->next = NULL;

			the_select_node->prev = (struct select_node*) myMalloc(sizeof(struct select_node), NULL, &malloced_head, the_debug);
			the_select_node->prev->select_node_alias = upper("TBL", NULL, &malloced_head, the_debug);
			
			the_select_node->prev->columns_arr_size = 7;

			the_select_node->prev->columns_table_ptrs_arr = (struct table_info**) myMalloc(sizeof(struct table_info*) * 7, NULL, &malloced_head, the_debug);
			the_select_node->prev->columns_table_ptrs_arr[0] = getTablesHead();
			the_select_node->prev->columns_table_ptrs_arr[1] = getTablesHead();
			the_select_node->prev->columns_table_ptrs_arr[2] = getTablesHead();
			the_select_node->prev->columns_table_ptrs_arr[3] = getTablesHead();
			the_select_node->prev->columns_table_ptrs_arr[4] = getTablesHead();
			the_select_node->prev->columns_table_ptrs_arr[5] = getTablesHead();
			the_select_node->prev->columns_table_ptrs_arr[6] = getTablesHead();

			the_select_node->prev->columns_col_ptrs_arr = (struct table_cols_info**) myMalloc(sizeof(struct table_cols_info*) * 7, NULL, &malloced_head, the_debug);
			the_select_node->prev->columns_col_ptrs_arr[0] = getTablesHead()->table_cols_head;
			the_select_node->prev->columns_col_ptrs_arr[1] = getTablesHead()->table_cols_head->next;
			the_select_node->prev->columns_col_ptrs_arr[2] = getTablesHead()->table_cols_head->next->next;
			the_select_node->prev->columns_col_ptrs_arr[3] = getTablesHead()->table_cols_head->next->next->next;
			the_select_node->prev->columns_col_ptrs_arr[4] = getTablesHead()->table_cols_head->next->next->next->next;
			the_select_node->prev->columns_col_ptrs_arr[5] = getTablesHead()->table_cols_head->next->next->next->next->next;
			the_select_node->prev->columns_col_ptrs_arr[6] = getTablesHead()->table_cols_head->next->next->next->next->next->next;

			the_select_node->prev->or_head = NULL;
			the_select_node->prev->join_head = NULL;

			the_select_node->prev->next = the_select_node;

			the_select_node->prev->prev = (struct select_node*) myMalloc(sizeof(struct select_node), NULL, &malloced_head, the_debug);
			the_select_node->prev->prev->select_node_alias = upper("TBL2", NULL, &malloced_head, the_debug);
			
			the_select_node->prev->prev->columns_arr_size = 7;

			the_select_node->prev->prev->columns_table_ptrs_arr = (struct table_info**) myMalloc(sizeof(struct table_info*) * 7, NULL, &malloced_head, the_debug);
			the_select_node->prev->prev->columns_table_ptrs_arr[0] = getTablesHead();
			the_select_node->prev->prev->columns_table_ptrs_arr[1] = getTablesHead();
			the_select_node->prev->prev->columns_table_ptrs_arr[2] = getTablesHead();
			the_select_node->prev->prev->columns_table_ptrs_arr[3] = getTablesHead();
			the_select_node->prev->prev->columns_table_ptrs_arr[4] = getTablesHead();
			the_select_node->prev->prev->columns_table_ptrs_arr[5] = getTablesHead();
			the_select_node->prev->prev->columns_table_ptrs_arr[6] = getTablesHead();

			the_select_node->prev->prev->columns_col_ptrs_arr = (struct table_cols_info**) myMalloc(sizeof(struct table_cols_info*) * 7, NULL, &malloced_head, the_debug);
			the_select_node->prev->prev->columns_col_ptrs_arr[0] = getTablesHead()->table_cols_head;
			the_select_node->prev->prev->columns_col_ptrs_arr[1] = getTablesHead()->table_cols_head->next;
			the_select_node->prev->prev->columns_col_ptrs_arr[2] = getTablesHead()->table_cols_head->next->next;
			the_select_node->prev->prev->columns_col_ptrs_arr[3] = getTablesHead()->table_cols_head->next->next->next;
			the_select_node->prev->prev->columns_col_ptrs_arr[4] = getTablesHead()->table_cols_head->next->next->next->next;
			the_select_node->prev->prev->columns_col_ptrs_arr[5] = getTablesHead()->table_cols_head->next->next->next->next->next;
			the_select_node->prev->prev->columns_col_ptrs_arr[6] = getTablesHead()->table_cols_head->next->next->next->next->next->next;

			the_select_node->prev->prev->or_head = NULL;
			the_select_node->prev->prev->join_head = NULL;

			the_select_node->prev->prev->next = the_select_node->prev;
			
			the_select_node->prev->prev->prev = NULL;

			the_select_node->join_head = (struct join_node*) myMalloc(sizeof(struct join_node), NULL, &malloced_head, the_debug);
			the_select_node->join_head->join_type = JOIN_INNER;
			the_select_node->join_head->select_from = the_select_node->prev;
			the_select_node->join_head->select_joined = the_select_node->prev->prev;

			the_select_node->join_head->next = NULL;
			the_select_node->join_head->on_clause_head = NULL;


			if (test_Controller_parseSelect(512, "select * from alc_brands tbl join alc_brands tbl2 on tbl.Brand-name = tbl2.Brand-name;", &(the_select_node->prev->prev)
										   ,&parsed_error_code, &malloced_head, the_debug) != 0)
				result = -1;


			if (parsed_error_code == 0)
			{
				while (the_select_node != NULL)
				{
					struct select_node* temp = the_select_node;
					the_select_node = the_select_node->prev;

					myFree((void**) &temp->columns_table_ptrs_arr, NULL, &malloced_head, the_debug);
					myFree((void**) &temp->columns_col_ptrs_arr, NULL, &malloced_head, the_debug);
					while (temp->join_head != NULL)
					{
						struct join_node* temp_joined = temp->join_head;
						temp->join_head = temp->join_head->next;

						myFree((void**) &temp_joined, NULL, &malloced_head, the_debug);
					}
					myFree((void**) &temp->select_node_alias, NULL, &malloced_head, the_debug);
					myFree((void**) &temp, NULL, &malloced_head, the_debug);
				}
			}

			if (malloced_head != NULL)
			{
				if (the_debug == YES_DEBUG)
					printf("	ERROR in test_Driver_main() at line %d in %s\n", __LINE__, __FILE__);
				return -3;
			}
		// END Test with id = 512

		// START Test with id = 513
			the_select_node = (struct select_node*) myMalloc(sizeof(struct select_node), NULL, &malloced_head, the_debug);

			the_select_node->select_node_alias = NULL;
			the_select_node->columns_arr_size = 7;

			the_select_node->columns_table_ptrs_arr = (struct table_info**) myMalloc(sizeof(struct table_info*) * 7, NULL, &malloced_head, the_debug);
			the_select_node->columns_table_ptrs_arr[0] = getTablesHead();
			the_select_node->columns_table_ptrs_arr[1] = getTablesHead();
			the_select_node->columns_table_ptrs_arr[2] = getTablesHead();
			the_select_node->columns_table_ptrs_arr[3] = getTablesHead();
			the_select_node->columns_table_ptrs_arr[4] = getTablesHead();
			the_select_node->columns_table_ptrs_arr[5] = getTablesHead();
			the_select_node->columns_table_ptrs_arr[6] = getTablesHead();

			the_select_node->columns_col_ptrs_arr = (struct table_cols_info**) myMalloc(sizeof(struct table_cols_info*) * 7, NULL, &malloced_head, the_debug);
			the_select_node->columns_col_ptrs_arr[0] = getTablesHead()->table_cols_head;
			the_select_node->columns_col_ptrs_arr[1] = getTablesHead()->table_cols_head->next;
			the_select_node->columns_col_ptrs_arr[2] = getTablesHead()->table_cols_head->next->next;
			the_select_node->columns_col_ptrs_arr[3] = getTablesHead()->table_cols_head->next->next->next;
			the_select_node->columns_col_ptrs_arr[4] = getTablesHead()->table_cols_head->next->next->next->next;
			the_select_node->columns_col_ptrs_arr[5] = getTablesHead()->table_cols_head->next->next->next->next->next;
			the_select_node->columns_col_ptrs_arr[6] = getTablesHead()->table_cols_head->next->next->next->next->next->next;

			the_select_node->or_head = NULL;
			the_select_node->join_head = NULL;

			the_select_node->next = NULL;

			the_select_node->prev = (struct select_node*) myMalloc(sizeof(struct select_node), NULL, &malloced_head, the_debug);
			the_select_node->prev->select_node_alias = upper("TBL", NULL, &malloced_head, the_debug);
			
			the_select_node->prev->columns_arr_size = 7;

			the_select_node->prev->columns_table_ptrs_arr = (struct table_info**) myMalloc(sizeof(struct table_info*) * 7, NULL, &malloced_head, the_debug);
			the_select_node->prev->columns_table_ptrs_arr[0] = getTablesHead();
			the_select_node->prev->columns_table_ptrs_arr[1] = getTablesHead();
			the_select_node->prev->columns_table_ptrs_arr[2] = getTablesHead();
			the_select_node->prev->columns_table_ptrs_arr[3] = getTablesHead();
			the_select_node->prev->columns_table_ptrs_arr[4] = getTablesHead();
			the_select_node->prev->columns_table_ptrs_arr[5] = getTablesHead();
			the_select_node->prev->columns_table_ptrs_arr[6] = getTablesHead();

			the_select_node->prev->columns_col_ptrs_arr = (struct table_cols_info**) myMalloc(sizeof(struct table_cols_info*) * 7, NULL, &malloced_head, the_debug);
			the_select_node->prev->columns_col_ptrs_arr[0] = getTablesHead()->table_cols_head;
			the_select_node->prev->columns_col_ptrs_arr[1] = getTablesHead()->table_cols_head->next;
			the_select_node->prev->columns_col_ptrs_arr[2] = getTablesHead()->table_cols_head->next->next;
			the_select_node->prev->columns_col_ptrs_arr[3] = getTablesHead()->table_cols_head->next->next->next;
			the_select_node->prev->columns_col_ptrs_arr[4] = getTablesHead()->table_cols_head->next->next->next->next;
			the_select_node->prev->columns_col_ptrs_arr[5] = getTablesHead()->table_cols_head->next->next->next->next->next;
			the_select_node->prev->columns_col_ptrs_arr[6] = getTablesHead()->table_cols_head->next->next->next->next->next->next;

			the_select_node->prev->or_head = NULL;
			the_select_node->prev->join_head = NULL;

			the_select_node->prev->next = the_select_node;

			the_select_node->prev->prev = (struct select_node*) myMalloc(sizeof(struct select_node), NULL, &malloced_head, the_debug);
			the_select_node->prev->prev->select_node_alias = NULL;
			
			the_select_node->prev->prev->columns_arr_size = 7;

			the_select_node->prev->prev->columns_table_ptrs_arr = (struct table_info**) myMalloc(sizeof(struct table_info*) * 7, NULL, &malloced_head, the_debug);
			the_select_node->prev->prev->columns_table_ptrs_arr[0] = getTablesHead();
			the_select_node->prev->prev->columns_table_ptrs_arr[1] = getTablesHead();
			the_select_node->prev->prev->columns_table_ptrs_arr[2] = getTablesHead();
			the_select_node->prev->prev->columns_table_ptrs_arr[3] = getTablesHead();
			the_select_node->prev->prev->columns_table_ptrs_arr[4] = getTablesHead();
			the_select_node->prev->prev->columns_table_ptrs_arr[5] = getTablesHead();
			the_select_node->prev->prev->columns_table_ptrs_arr[6] = getTablesHead();

			the_select_node->prev->prev->columns_col_ptrs_arr = (struct table_cols_info**) myMalloc(sizeof(struct table_cols_info*) * 7, NULL, &malloced_head, the_debug);
			the_select_node->prev->prev->columns_col_ptrs_arr[0] = getTablesHead()->table_cols_head;
			the_select_node->prev->prev->columns_col_ptrs_arr[1] = getTablesHead()->table_cols_head->next;
			the_select_node->prev->prev->columns_col_ptrs_arr[2] = getTablesHead()->table_cols_head->next->next;
			the_select_node->prev->prev->columns_col_ptrs_arr[3] = getTablesHead()->table_cols_head->next->next->next;
			the_select_node->prev->prev->columns_col_ptrs_arr[4] = getTablesHead()->table_cols_head->next->next->next->next;
			the_select_node->prev->prev->columns_col_ptrs_arr[5] = getTablesHead()->table_cols_head->next->next->next->next->next;
			the_select_node->prev->prev->columns_col_ptrs_arr[6] = getTablesHead()->table_cols_head->next->next->next->next->next->next;

			the_select_node->prev->prev->or_head = NULL;
			the_select_node->prev->prev->join_head = NULL;

			the_select_node->prev->prev->next = the_select_node->prev;
			
			the_select_node->prev->prev->prev = NULL;


			if (test_Controller_parseSelect(513, "select * from ( select * from alc_brands ) tbl;", &(the_select_node->prev->prev)
										   ,&parsed_error_code, &malloced_head, the_debug) != 0)
				result = -1;


			if (parsed_error_code == 0)
			{
				while (the_select_node != NULL)
				{
					struct select_node* temp = the_select_node;
					the_select_node = the_select_node->prev;

					myFree((void**) &temp->columns_table_ptrs_arr, NULL, &malloced_head, the_debug);
					myFree((void**) &temp->columns_col_ptrs_arr, NULL, &malloced_head, the_debug);
					while (temp->join_head != NULL)
					{
						struct join_node* temp_joined = temp->join_head;
						temp->join_head = temp->join_head->next;

						myFree((void**) &temp_joined, NULL, &malloced_head, the_debug);
					}
					myFree((void**) &temp->select_node_alias, NULL, &malloced_head, the_debug);
					myFree((void**) &temp, NULL, &malloced_head, the_debug);
				}
			}


			if (malloced_head != NULL)
			{
				if (the_debug == YES_DEBUG)
					printf("	ERROR in test_Driver_main() at line %d in %s\n", __LINE__, __FILE__);
				return -3;
			}
		// END Test with id = 513

		// START Test with id = 514
			the_select_node = (struct select_node*) myMalloc(sizeof(struct select_node), NULL, &malloced_head, the_debug);

			the_select_node->select_node_alias = NULL;
			the_select_node->columns_arr_size = 14;

			the_select_node->columns_table_ptrs_arr = (struct table_info**) myMalloc(sizeof(struct table_info*) * 14, NULL, &malloced_head, the_debug);
			the_select_node->columns_table_ptrs_arr[0] = getTablesHead();
			the_select_node->columns_table_ptrs_arr[1] = getTablesHead();
			the_select_node->columns_table_ptrs_arr[2] = getTablesHead();
			the_select_node->columns_table_ptrs_arr[3] = getTablesHead();
			the_select_node->columns_table_ptrs_arr[4] = getTablesHead();
			the_select_node->columns_table_ptrs_arr[5] = getTablesHead();
			the_select_node->columns_table_ptrs_arr[6] = getTablesHead();
			the_select_node->columns_table_ptrs_arr[7] = getTablesHead();
			the_select_node->columns_table_ptrs_arr[8] = getTablesHead();
			the_select_node->columns_table_ptrs_arr[9] = getTablesHead();
			the_select_node->columns_table_ptrs_arr[10] = getTablesHead();
			the_select_node->columns_table_ptrs_arr[11] = getTablesHead();
			the_select_node->columns_table_ptrs_arr[12] = getTablesHead();
			the_select_node->columns_table_ptrs_arr[13] = getTablesHead();

			the_select_node->columns_col_ptrs_arr = (struct table_cols_info**) myMalloc(sizeof(struct table_cols_info*) * 14, NULL, &malloced_head, the_debug);
			the_select_node->columns_col_ptrs_arr[0] = getTablesHead()->table_cols_head;
			the_select_node->columns_col_ptrs_arr[1] = getTablesHead()->table_cols_head->next;
			the_select_node->columns_col_ptrs_arr[2] = getTablesHead()->table_cols_head->next->next;
			the_select_node->columns_col_ptrs_arr[3] = getTablesHead()->table_cols_head->next->next->next;
			the_select_node->columns_col_ptrs_arr[4] = getTablesHead()->table_cols_head->next->next->next->next;
			the_select_node->columns_col_ptrs_arr[5] = getTablesHead()->table_cols_head->next->next->next->next->next;
			the_select_node->columns_col_ptrs_arr[6] = getTablesHead()->table_cols_head->next->next->next->next->next->next;
			the_select_node->columns_col_ptrs_arr[7] = getTablesHead()->table_cols_head;
			the_select_node->columns_col_ptrs_arr[8] = getTablesHead()->table_cols_head->next;
			the_select_node->columns_col_ptrs_arr[9] = getTablesHead()->table_cols_head->next->next;
			the_select_node->columns_col_ptrs_arr[10] = getTablesHead()->table_cols_head->next->next->next;
			the_select_node->columns_col_ptrs_arr[11] = getTablesHead()->table_cols_head->next->next->next->next;
			the_select_node->columns_col_ptrs_arr[12] = getTablesHead()->table_cols_head->next->next->next->next->next;
			the_select_node->columns_col_ptrs_arr[13] = getTablesHead()->table_cols_head->next->next->next->next->next->next;

			the_select_node->or_head = NULL;
			the_select_node->join_head = NULL;

			the_select_node->next = NULL;

			the_select_node->prev = (struct select_node*) myMalloc(sizeof(struct select_node), NULL, &malloced_head, the_debug);
			the_select_node->prev->select_node_alias = upper("TBL", NULL, &malloced_head, the_debug);
			
			the_select_node->prev->columns_arr_size = 7;

			the_select_node->prev->columns_table_ptrs_arr = (struct table_info**) myMalloc(sizeof(struct table_info*) * 7, NULL, &malloced_head, the_debug);
			the_select_node->prev->columns_table_ptrs_arr[0] = getTablesHead();
			the_select_node->prev->columns_table_ptrs_arr[1] = getTablesHead();
			the_select_node->prev->columns_table_ptrs_arr[2] = getTablesHead();
			the_select_node->prev->columns_table_ptrs_arr[3] = getTablesHead();
			the_select_node->prev->columns_table_ptrs_arr[4] = getTablesHead();
			the_select_node->prev->columns_table_ptrs_arr[5] = getTablesHead();
			the_select_node->prev->columns_table_ptrs_arr[6] = getTablesHead();

			the_select_node->prev->columns_col_ptrs_arr = (struct table_cols_info**) myMalloc(sizeof(struct table_cols_info*) * 7, NULL, &malloced_head, the_debug);
			the_select_node->prev->columns_col_ptrs_arr[0] = getTablesHead()->table_cols_head;
			the_select_node->prev->columns_col_ptrs_arr[1] = getTablesHead()->table_cols_head->next;
			the_select_node->prev->columns_col_ptrs_arr[2] = getTablesHead()->table_cols_head->next->next;
			the_select_node->prev->columns_col_ptrs_arr[3] = getTablesHead()->table_cols_head->next->next->next;
			the_select_node->prev->columns_col_ptrs_arr[4] = getTablesHead()->table_cols_head->next->next->next->next;
			the_select_node->prev->columns_col_ptrs_arr[5] = getTablesHead()->table_cols_head->next->next->next->next->next;
			the_select_node->prev->columns_col_ptrs_arr[6] = getTablesHead()->table_cols_head->next->next->next->next->next->next;

			the_select_node->prev->or_head = NULL;
			the_select_node->prev->join_head = NULL;

			the_select_node->prev->next = the_select_node;

			the_select_node->prev->prev = (struct select_node*) myMalloc(sizeof(struct select_node), NULL, &malloced_head, the_debug);
			the_select_node->prev->prev->select_node_alias = NULL;
			
			the_select_node->prev->prev->columns_arr_size = 7;

			the_select_node->prev->prev->columns_table_ptrs_arr = (struct table_info**) myMalloc(sizeof(struct table_info*) * 7, NULL, &malloced_head, the_debug);
			the_select_node->prev->prev->columns_table_ptrs_arr[0] = getTablesHead();
			the_select_node->prev->prev->columns_table_ptrs_arr[1] = getTablesHead();
			the_select_node->prev->prev->columns_table_ptrs_arr[2] = getTablesHead();
			the_select_node->prev->prev->columns_table_ptrs_arr[3] = getTablesHead();
			the_select_node->prev->prev->columns_table_ptrs_arr[4] = getTablesHead();
			the_select_node->prev->prev->columns_table_ptrs_arr[5] = getTablesHead();
			the_select_node->prev->prev->columns_table_ptrs_arr[6] = getTablesHead();

			the_select_node->prev->prev->columns_col_ptrs_arr = (struct table_cols_info**) myMalloc(sizeof(struct table_cols_info*) * 7, NULL, &malloced_head, the_debug);
			the_select_node->prev->prev->columns_col_ptrs_arr[0] = getTablesHead()->table_cols_head;
			the_select_node->prev->prev->columns_col_ptrs_arr[1] = getTablesHead()->table_cols_head->next;
			the_select_node->prev->prev->columns_col_ptrs_arr[2] = getTablesHead()->table_cols_head->next->next;
			the_select_node->prev->prev->columns_col_ptrs_arr[3] = getTablesHead()->table_cols_head->next->next->next;
			the_select_node->prev->prev->columns_col_ptrs_arr[4] = getTablesHead()->table_cols_head->next->next->next->next;
			the_select_node->prev->prev->columns_col_ptrs_arr[5] = getTablesHead()->table_cols_head->next->next->next->next->next;
			the_select_node->prev->prev->columns_col_ptrs_arr[6] = getTablesHead()->table_cols_head->next->next->next->next->next->next;

			the_select_node->prev->prev->or_head = NULL;
			the_select_node->prev->prev->join_head = NULL;

			the_select_node->prev->prev->next = the_select_node->prev;
			
			the_select_node->prev->prev->prev = (struct select_node*) myMalloc(sizeof(struct select_node), NULL, &malloced_head, the_debug);
			the_select_node->prev->prev->prev->select_node_alias = upper("TBL2", NULL, &malloced_head, the_debug);
			
			the_select_node->prev->prev->prev->columns_arr_size = 7;

			the_select_node->prev->prev->prev->columns_table_ptrs_arr = (struct table_info**) myMalloc(sizeof(struct table_info*) * 7, NULL, &malloced_head, the_debug);
			the_select_node->prev->prev->prev->columns_table_ptrs_arr[0] = getTablesHead();
			the_select_node->prev->prev->prev->columns_table_ptrs_arr[1] = getTablesHead();
			the_select_node->prev->prev->prev->columns_table_ptrs_arr[2] = getTablesHead();
			the_select_node->prev->prev->prev->columns_table_ptrs_arr[3] = getTablesHead();
			the_select_node->prev->prev->prev->columns_table_ptrs_arr[4] = getTablesHead();
			the_select_node->prev->prev->prev->columns_table_ptrs_arr[5] = getTablesHead();
			the_select_node->prev->prev->prev->columns_table_ptrs_arr[6] = getTablesHead();

			the_select_node->prev->prev->prev->columns_col_ptrs_arr = (struct table_cols_info**) myMalloc(sizeof(struct table_cols_info*) * 7, NULL, &malloced_head, the_debug);
			the_select_node->prev->prev->prev->columns_col_ptrs_arr[0] = getTablesHead()->table_cols_head;
			the_select_node->prev->prev->prev->columns_col_ptrs_arr[1] = getTablesHead()->table_cols_head->next;
			the_select_node->prev->prev->prev->columns_col_ptrs_arr[2] = getTablesHead()->table_cols_head->next->next;
			the_select_node->prev->prev->prev->columns_col_ptrs_arr[3] = getTablesHead()->table_cols_head->next->next->next;
			the_select_node->prev->prev->prev->columns_col_ptrs_arr[4] = getTablesHead()->table_cols_head->next->next->next->next;
			the_select_node->prev->prev->prev->columns_col_ptrs_arr[5] = getTablesHead()->table_cols_head->next->next->next->next->next;
			the_select_node->prev->prev->prev->columns_col_ptrs_arr[6] = getTablesHead()->table_cols_head->next->next->next->next->next->next;

			the_select_node->prev->prev->prev->or_head = NULL;
			the_select_node->prev->prev->prev->join_head = NULL;

			the_select_node->prev->prev->prev->next = the_select_node->prev->prev;

			the_select_node->prev->prev->prev->prev = NULL;

			the_select_node->join_head = (struct join_node*) myMalloc(sizeof(struct join_node), NULL, &malloced_head, the_debug);
			the_select_node->join_head->join_type = JOIN_INNER;
			the_select_node->join_head->select_from = the_select_node->prev;
			the_select_node->join_head->select_joined = the_select_node->prev->prev->prev;

			the_select_node->join_head->next = NULL;
			the_select_node->join_head->on_clause_head = NULL;


			if (test_Controller_parseSelect(514, "select * from ( select * from alc_brands ) tbl join alc_brands tbl2 on tbl.Brand-name = tbl2.Brand-name;", &(the_select_node->prev->prev->prev)
										   ,&parsed_error_code, &malloced_head, the_debug) != 0)
				result = -1;


			if (parsed_error_code == 0)
			{
				while (the_select_node != NULL)
				{
					struct select_node* temp = the_select_node;
					the_select_node = the_select_node->prev;

					myFree((void**) &temp->columns_table_ptrs_arr, NULL, &malloced_head, the_debug);
					myFree((void**) &temp->columns_col_ptrs_arr, NULL, &malloced_head, the_debug);
					while (temp->join_head != NULL)
					{
						struct join_node* temp_joined = temp->join_head;
						temp->join_head = temp->join_head->next;

						myFree((void**) &temp_joined, NULL, &malloced_head, the_debug);
					}
					myFree((void**) &temp->select_node_alias, NULL, &malloced_head, the_debug);
					myFree((void**) &temp, NULL, &malloced_head, the_debug);
				}
			}


			if (malloced_head != NULL)
			{
				if (the_debug == YES_DEBUG)
					printf("	ERROR in test_Driver_main() at line %d in %s\n", __LINE__, __FILE__);
				return -3;
			}
		// END Test with id = 514

		// START Test with id = 515
			the_select_node = (struct select_node*) myMalloc(sizeof(struct select_node), NULL, &malloced_head, the_debug);

			the_select_node->select_node_alias = NULL;
			the_select_node->columns_arr_size = 14;

			the_select_node->columns_table_ptrs_arr = (struct table_info**) myMalloc(sizeof(struct table_info*) * 14, NULL, &malloced_head, the_debug);
			the_select_node->columns_table_ptrs_arr[0] = getTablesHead();
			the_select_node->columns_table_ptrs_arr[1] = getTablesHead();
			the_select_node->columns_table_ptrs_arr[2] = getTablesHead();
			the_select_node->columns_table_ptrs_arr[3] = getTablesHead();
			the_select_node->columns_table_ptrs_arr[4] = getTablesHead();
			the_select_node->columns_table_ptrs_arr[5] = getTablesHead();
			the_select_node->columns_table_ptrs_arr[6] = getTablesHead();
			the_select_node->columns_table_ptrs_arr[7] = getTablesHead();
			the_select_node->columns_table_ptrs_arr[8] = getTablesHead();
			the_select_node->columns_table_ptrs_arr[9] = getTablesHead();
			the_select_node->columns_table_ptrs_arr[10] = getTablesHead();
			the_select_node->columns_table_ptrs_arr[11] = getTablesHead();
			the_select_node->columns_table_ptrs_arr[12] = getTablesHead();
			the_select_node->columns_table_ptrs_arr[13] = getTablesHead();

			the_select_node->columns_col_ptrs_arr = (struct table_cols_info**) myMalloc(sizeof(struct table_cols_info*) * 14, NULL, &malloced_head, the_debug);
			the_select_node->columns_col_ptrs_arr[0] = getTablesHead()->table_cols_head;
			the_select_node->columns_col_ptrs_arr[1] = getTablesHead()->table_cols_head->next;
			the_select_node->columns_col_ptrs_arr[2] = getTablesHead()->table_cols_head->next->next;
			the_select_node->columns_col_ptrs_arr[3] = getTablesHead()->table_cols_head->next->next->next;
			the_select_node->columns_col_ptrs_arr[4] = getTablesHead()->table_cols_head->next->next->next->next;
			the_select_node->columns_col_ptrs_arr[5] = getTablesHead()->table_cols_head->next->next->next->next->next;
			the_select_node->columns_col_ptrs_arr[6] = getTablesHead()->table_cols_head->next->next->next->next->next->next;
			the_select_node->columns_col_ptrs_arr[7] = getTablesHead()->table_cols_head;
			the_select_node->columns_col_ptrs_arr[8] = getTablesHead()->table_cols_head->next;
			the_select_node->columns_col_ptrs_arr[9] = getTablesHead()->table_cols_head->next->next;
			the_select_node->columns_col_ptrs_arr[10] = getTablesHead()->table_cols_head->next->next->next;
			the_select_node->columns_col_ptrs_arr[11] = getTablesHead()->table_cols_head->next->next->next->next;
			the_select_node->columns_col_ptrs_arr[12] = getTablesHead()->table_cols_head->next->next->next->next->next;
			the_select_node->columns_col_ptrs_arr[13] = getTablesHead()->table_cols_head->next->next->next->next->next->next;

			the_select_node->or_head = NULL;
			the_select_node->join_head = NULL;

			the_select_node->next = NULL;

			the_select_node->prev = (struct select_node*) myMalloc(sizeof(struct select_node), NULL, &malloced_head, the_debug);
			the_select_node->prev->select_node_alias = upper("TBL", NULL, &malloced_head, the_debug);
			
			the_select_node->prev->columns_arr_size = 7;

			the_select_node->prev->columns_table_ptrs_arr = (struct table_info**) myMalloc(sizeof(struct table_info*) * 7, NULL, &malloced_head, the_debug);
			the_select_node->prev->columns_table_ptrs_arr[0] = getTablesHead();
			the_select_node->prev->columns_table_ptrs_arr[1] = getTablesHead();
			the_select_node->prev->columns_table_ptrs_arr[2] = getTablesHead();
			the_select_node->prev->columns_table_ptrs_arr[3] = getTablesHead();
			the_select_node->prev->columns_table_ptrs_arr[4] = getTablesHead();
			the_select_node->prev->columns_table_ptrs_arr[5] = getTablesHead();
			the_select_node->prev->columns_table_ptrs_arr[6] = getTablesHead();

			the_select_node->prev->columns_col_ptrs_arr = (struct table_cols_info**) myMalloc(sizeof(struct table_cols_info*) * 7, NULL, &malloced_head, the_debug);
			the_select_node->prev->columns_col_ptrs_arr[0] = getTablesHead()->table_cols_head;
			the_select_node->prev->columns_col_ptrs_arr[1] = getTablesHead()->table_cols_head->next;
			the_select_node->prev->columns_col_ptrs_arr[2] = getTablesHead()->table_cols_head->next->next;
			the_select_node->prev->columns_col_ptrs_arr[3] = getTablesHead()->table_cols_head->next->next->next;
			the_select_node->prev->columns_col_ptrs_arr[4] = getTablesHead()->table_cols_head->next->next->next->next;
			the_select_node->prev->columns_col_ptrs_arr[5] = getTablesHead()->table_cols_head->next->next->next->next->next;
			the_select_node->prev->columns_col_ptrs_arr[6] = getTablesHead()->table_cols_head->next->next->next->next->next->next;

			the_select_node->prev->or_head = NULL;
			the_select_node->prev->join_head = NULL;

			the_select_node->prev->next = the_select_node;

			the_select_node->prev->prev = (struct select_node*) myMalloc(sizeof(struct select_node), NULL, &malloced_head, the_debug);
			the_select_node->prev->prev->select_node_alias = upper("TBL2", NULL, &malloced_head, the_debug);
			
			the_select_node->prev->prev->columns_arr_size = 7;

			the_select_node->prev->prev->columns_table_ptrs_arr = (struct table_info**) myMalloc(sizeof(struct table_info*) * 7, NULL, &malloced_head, the_debug);
			the_select_node->prev->prev->columns_table_ptrs_arr[0] = getTablesHead();
			the_select_node->prev->prev->columns_table_ptrs_arr[1] = getTablesHead();
			the_select_node->prev->prev->columns_table_ptrs_arr[2] = getTablesHead();
			the_select_node->prev->prev->columns_table_ptrs_arr[3] = getTablesHead();
			the_select_node->prev->prev->columns_table_ptrs_arr[4] = getTablesHead();
			the_select_node->prev->prev->columns_table_ptrs_arr[5] = getTablesHead();
			the_select_node->prev->prev->columns_table_ptrs_arr[6] = getTablesHead();

			the_select_node->prev->prev->columns_col_ptrs_arr = (struct table_cols_info**) myMalloc(sizeof(struct table_cols_info*) * 7, NULL, &malloced_head, the_debug);
			the_select_node->prev->prev->columns_col_ptrs_arr[0] = getTablesHead()->table_cols_head;
			the_select_node->prev->prev->columns_col_ptrs_arr[1] = getTablesHead()->table_cols_head->next;
			the_select_node->prev->prev->columns_col_ptrs_arr[2] = getTablesHead()->table_cols_head->next->next;
			the_select_node->prev->prev->columns_col_ptrs_arr[3] = getTablesHead()->table_cols_head->next->next->next;
			the_select_node->prev->prev->columns_col_ptrs_arr[4] = getTablesHead()->table_cols_head->next->next->next->next;
			the_select_node->prev->prev->columns_col_ptrs_arr[5] = getTablesHead()->table_cols_head->next->next->next->next->next;
			the_select_node->prev->prev->columns_col_ptrs_arr[6] = getTablesHead()->table_cols_head->next->next->next->next->next->next;

			the_select_node->prev->prev->or_head = NULL;
			the_select_node->prev->prev->join_head = NULL;

			the_select_node->prev->prev->next = the_select_node->prev;
			
			the_select_node->prev->prev->prev = (struct select_node*) myMalloc(sizeof(struct select_node), NULL, &malloced_head, the_debug);
			the_select_node->prev->prev->prev->select_node_alias = NULL;
			
			the_select_node->prev->prev->prev->columns_arr_size = 7;

			the_select_node->prev->prev->prev->columns_table_ptrs_arr = (struct table_info**) myMalloc(sizeof(struct table_info*) * 7, NULL, &malloced_head, the_debug);
			the_select_node->prev->prev->prev->columns_table_ptrs_arr[0] = getTablesHead();
			the_select_node->prev->prev->prev->columns_table_ptrs_arr[1] = getTablesHead();
			the_select_node->prev->prev->prev->columns_table_ptrs_arr[2] = getTablesHead();
			the_select_node->prev->prev->prev->columns_table_ptrs_arr[3] = getTablesHead();
			the_select_node->prev->prev->prev->columns_table_ptrs_arr[4] = getTablesHead();
			the_select_node->prev->prev->prev->columns_table_ptrs_arr[5] = getTablesHead();
			the_select_node->prev->prev->prev->columns_table_ptrs_arr[6] = getTablesHead();

			the_select_node->prev->prev->prev->columns_col_ptrs_arr = (struct table_cols_info**) myMalloc(sizeof(struct table_cols_info*) * 7, NULL, &malloced_head, the_debug);
			the_select_node->prev->prev->prev->columns_col_ptrs_arr[0] = getTablesHead()->table_cols_head;
			the_select_node->prev->prev->prev->columns_col_ptrs_arr[1] = getTablesHead()->table_cols_head->next;
			the_select_node->prev->prev->prev->columns_col_ptrs_arr[2] = getTablesHead()->table_cols_head->next->next;
			the_select_node->prev->prev->prev->columns_col_ptrs_arr[3] = getTablesHead()->table_cols_head->next->next->next;
			the_select_node->prev->prev->prev->columns_col_ptrs_arr[4] = getTablesHead()->table_cols_head->next->next->next->next;
			the_select_node->prev->prev->prev->columns_col_ptrs_arr[5] = getTablesHead()->table_cols_head->next->next->next->next->next;
			the_select_node->prev->prev->prev->columns_col_ptrs_arr[6] = getTablesHead()->table_cols_head->next->next->next->next->next->next;

			the_select_node->prev->prev->prev->or_head = NULL;
			the_select_node->prev->prev->prev->join_head = NULL;

			the_select_node->prev->prev->prev->next = the_select_node->prev->prev;

			the_select_node->prev->prev->prev->prev = NULL;

			the_select_node->join_head = (struct join_node*) myMalloc(sizeof(struct join_node), NULL, &malloced_head, the_debug);
			the_select_node->join_head->join_type = JOIN_INNER;
			the_select_node->join_head->select_from = the_select_node->prev;
			the_select_node->join_head->select_joined = the_select_node->prev->prev;

			the_select_node->join_head->next = NULL;
			the_select_node->join_head->on_clause_head = NULL;


			if (test_Controller_parseSelect(515, "select * from ALC_Brands tbl join ( select * from alc_brands ) tbl2 on tbl.Brand-name = tbl2.Brand-name;", &(the_select_node->prev->prev->prev)
										   ,&parsed_error_code, &malloced_head, the_debug) != 0)
				result = -1;


			if (parsed_error_code == 0)
			{
				while (the_select_node != NULL)
				{
					struct select_node* temp = the_select_node;
					the_select_node = the_select_node->prev;

					myFree((void**) &temp->columns_table_ptrs_arr, NULL, &malloced_head, the_debug);
					myFree((void**) &temp->columns_col_ptrs_arr, NULL, &malloced_head, the_debug);
					while (temp->join_head != NULL)
					{
						struct join_node* temp_joined = temp->join_head;
						temp->join_head = temp->join_head->next;

						myFree((void**) &temp_joined, NULL, &malloced_head, the_debug);
					}
					myFree((void**) &temp->select_node_alias, NULL, &malloced_head, the_debug);
					myFree((void**) &temp, NULL, &malloced_head, the_debug);
				}
			}


			if (malloced_head != NULL)
			{
				if (the_debug == YES_DEBUG)
					printf("	ERROR in test_Driver_main() at line %d in %s\n", __LINE__, __FILE__);
				return -3;
			}
		// END Test with id = 515

		// START Test with id = 516
			the_select_node = (struct select_node*) myMalloc(sizeof(struct select_node), NULL, &malloced_head, the_debug);

			the_select_node->select_node_alias = NULL;
			the_select_node->columns_arr_size = 2;

			the_select_node->columns_table_ptrs_arr = (struct table_info**) myMalloc(sizeof(struct table_info*) * 2, NULL, &malloced_head, the_debug);
			the_select_node->columns_table_ptrs_arr[0] = getTablesHead();
			the_select_node->columns_table_ptrs_arr[1] = getTablesHead();

			the_select_node->columns_col_ptrs_arr = (struct table_cols_info**) myMalloc(sizeof(struct table_cols_info*) * 2, NULL, &malloced_head, the_debug);
			the_select_node->columns_col_ptrs_arr[0] = getTablesHead()->table_cols_head;
			the_select_node->columns_col_ptrs_arr[1] = getTablesHead()->table_cols_head->next;

			the_select_node->or_head = NULL;
			the_select_node->join_head = NULL;

			the_select_node->next = NULL;

			the_select_node->prev = (struct select_node*) myMalloc(sizeof(struct select_node), NULL, &malloced_head, the_debug);
			the_select_node->prev->select_node_alias = upper("TBL", NULL, &malloced_head, the_debug);
			
			the_select_node->prev->columns_arr_size = 2;

			the_select_node->prev->columns_table_ptrs_arr = (struct table_info**) myMalloc(sizeof(struct table_info*) * 2, NULL, &malloced_head, the_debug);
			the_select_node->prev->columns_table_ptrs_arr[0] = getTablesHead();
			the_select_node->prev->columns_table_ptrs_arr[1] = getTablesHead();

			the_select_node->prev->columns_col_ptrs_arr = (struct table_cols_info**) myMalloc(sizeof(struct table_cols_info*) * 2, NULL, &malloced_head, the_debug);
			the_select_node->prev->columns_col_ptrs_arr[0] = getTablesHead()->table_cols_head;
			the_select_node->prev->columns_col_ptrs_arr[1] = getTablesHead()->table_cols_head->next;

			the_select_node->prev->or_head = NULL;
			the_select_node->prev->join_head = NULL;

			the_select_node->prev->next = the_select_node;

			the_select_node->prev->prev = (struct select_node*) myMalloc(sizeof(struct select_node), NULL, &malloced_head, the_debug);
			the_select_node->prev->prev->select_node_alias = upper("TBL2", NULL, &malloced_head, the_debug);
			
			the_select_node->prev->prev->columns_arr_size = 2;

			the_select_node->prev->prev->columns_table_ptrs_arr = (struct table_info**) myMalloc(sizeof(struct table_info*) * 2, NULL, &malloced_head, the_debug);
			the_select_node->prev->prev->columns_table_ptrs_arr[0] = getTablesHead();
			the_select_node->prev->prev->columns_table_ptrs_arr[1] = getTablesHead();

			the_select_node->prev->prev->columns_col_ptrs_arr = (struct table_cols_info**) myMalloc(sizeof(struct table_cols_info*) * 2, NULL, &malloced_head, the_debug);
			the_select_node->prev->prev->columns_col_ptrs_arr[0] = getTablesHead()->table_cols_head->next;
			the_select_node->prev->prev->columns_col_ptrs_arr[1] = getTablesHead()->table_cols_head;

			the_select_node->prev->prev->or_head = NULL;
			the_select_node->prev->prev->join_head = NULL;

			the_select_node->prev->prev->next = the_select_node->prev;
			
			the_select_node->prev->prev->prev = (struct select_node*) myMalloc(sizeof(struct select_node), NULL, &malloced_head, the_debug);
			the_select_node->prev->prev->prev->select_node_alias = NULL;
			
			the_select_node->prev->prev->prev->columns_arr_size = 7;

			the_select_node->prev->prev->prev->columns_table_ptrs_arr = (struct table_info**) myMalloc(sizeof(struct table_info*) * 7, NULL, &malloced_head, the_debug);
			the_select_node->prev->prev->prev->columns_table_ptrs_arr[0] = getTablesHead();
			the_select_node->prev->prev->prev->columns_table_ptrs_arr[1] = getTablesHead();
			the_select_node->prev->prev->prev->columns_table_ptrs_arr[2] = getTablesHead();
			the_select_node->prev->prev->prev->columns_table_ptrs_arr[3] = getTablesHead();
			the_select_node->prev->prev->prev->columns_table_ptrs_arr[4] = getTablesHead();
			the_select_node->prev->prev->prev->columns_table_ptrs_arr[5] = getTablesHead();
			the_select_node->prev->prev->prev->columns_table_ptrs_arr[6] = getTablesHead();

			the_select_node->prev->prev->prev->columns_col_ptrs_arr = (struct table_cols_info**) myMalloc(sizeof(struct table_cols_info*) * 7, NULL, &malloced_head, the_debug);
			the_select_node->prev->prev->prev->columns_col_ptrs_arr[0] = getTablesHead()->table_cols_head;
			the_select_node->prev->prev->prev->columns_col_ptrs_arr[1] = getTablesHead()->table_cols_head->next;
			the_select_node->prev->prev->prev->columns_col_ptrs_arr[2] = getTablesHead()->table_cols_head->next->next;
			the_select_node->prev->prev->prev->columns_col_ptrs_arr[3] = getTablesHead()->table_cols_head->next->next->next;
			the_select_node->prev->prev->prev->columns_col_ptrs_arr[4] = getTablesHead()->table_cols_head->next->next->next->next;
			the_select_node->prev->prev->prev->columns_col_ptrs_arr[5] = getTablesHead()->table_cols_head->next->next->next->next->next;
			the_select_node->prev->prev->prev->columns_col_ptrs_arr[6] = getTablesHead()->table_cols_head->next->next->next->next->next->next;

			the_select_node->prev->prev->prev->or_head = NULL;
			the_select_node->prev->prev->prev->join_head = NULL;

			the_select_node->prev->prev->prev->next = the_select_node->prev->prev;
			the_select_node->prev->prev->prev->prev = NULL;


			if (test_Controller_parseSelect(516, "select * from ( select braND-name,CT-REGISTRATION-NUMBER from ( select CT-REGISTRATION-NUMBER, braND-name from alc_brands ) tbl2 ) tbl;", &(the_select_node->prev->prev->prev)
										   ,&parsed_error_code, &malloced_head, the_debug) != 0)
				result = -1;


			if (parsed_error_code == 0)
			{
				while (the_select_node != NULL)
				{
					struct select_node* temp = the_select_node;
					the_select_node = the_select_node->prev;

					myFree((void**) &temp->columns_table_ptrs_arr, NULL, &malloced_head, the_debug);
					myFree((void**) &temp->columns_col_ptrs_arr, NULL, &malloced_head, the_debug);
					while (temp->join_head != NULL)
					{
						struct join_node* temp_joined = temp->join_head;
						temp->join_head = temp->join_head->next;

						myFree((void**) &temp_joined, NULL, &malloced_head, the_debug);
					}
					myFree((void**) &temp->select_node_alias, NULL, &malloced_head, the_debug);
					myFree((void**) &temp, NULL, &malloced_head, the_debug);
				}
			}

			if (malloced_head != NULL)
			{
				if (the_debug == YES_DEBUG)
					printf("	ERROR in test_Driver_main() at line %d in %s\n", __LINE__, __FILE__);
				return -3;
			}
		// END Test with id = 516

		// START Test with id = 517
			the_select_node = NULL;

			if (test_Controller_parseSelect(517, "select tbl.Brand-name, tbl2.Brand-name, tbl.STATUS, tbl2.EFFECTIVE from ALC_Brands tbl join ( select Brand-name, STATUS from alc_brands ) tbl2 on tbl.Brand-name = tbl2.Brand-name;", &the_select_node
										   ,&parsed_error_code, &malloced_head, the_debug) != 0)
				result = -1;


			if (malloced_head != NULL)
			{
				if (the_debug == YES_DEBUG)
					printf("	ERROR in test_Driver_main() at line %d in %s\n", __LINE__, __FILE__);
				return -3;
			}
		// END Test with id = 517

		// START Test with id = 518
			the_select_node = NULL;

			if (test_Controller_parseSelect(518, "select tbl.Brand-name, tbl2.Brand-name, tbl.STATUS, tbl.EFFECTIVE from ALC_Brands tbl join ( select Brand-name, STATUS, awiodjbaoijd from alc_brands ) tbl2 on tbl.Brand-name = tbl2.Brand-name;", &the_select_node
										   ,&parsed_error_code, &malloced_head, the_debug) != 0)
				result = -1;


			if (malloced_head != NULL)
			{
				if (the_debug == YES_DEBUG)
					printf("	ERROR in test_Driver_main() at line %d in %s\n", __LINE__, __FILE__);
				return -3;
			}
		// END Test with id = 518

		// START Test with id = 519
			int parsed_error_code;

			struct select_node* the_select_node = (struct select_node*) myMalloc(sizeof(struct select_node), NULL, &malloced_head, the_debug);
			the_select_node->select_node_alias = NULL;

			the_select_node->columns_arr_size = 7;

			the_select_node->columns_table_ptrs_arr = (struct table_info**) myMalloc(sizeof(struct table_info*) * 7, NULL, &malloced_head, the_debug);
			the_select_node->columns_table_ptrs_arr[0] = getTablesHead();
			the_select_node->columns_table_ptrs_arr[1] = getTablesHead();
			the_select_node->columns_table_ptrs_arr[2] = getTablesHead();
			the_select_node->columns_table_ptrs_arr[3] = getTablesHead();
			the_select_node->columns_table_ptrs_arr[4] = getTablesHead();
			the_select_node->columns_table_ptrs_arr[5] = getTablesHead();
			the_select_node->columns_table_ptrs_arr[6] = getTablesHead();

			the_select_node->columns_col_ptrs_arr = (struct table_cols_info**) myMalloc(sizeof(struct table_cols_info*) * 7, NULL, &malloced_head, the_debug);
			the_select_node->columns_col_ptrs_arr[0] = getTablesHead()->table_cols_head;
			the_select_node->columns_col_ptrs_arr[1] = getTablesHead()->table_cols_head->next;
			the_select_node->columns_col_ptrs_arr[2] = getTablesHead()->table_cols_head->next->next;
			the_select_node->columns_col_ptrs_arr[3] = getTablesHead()->table_cols_head->next->next->next;
			the_select_node->columns_col_ptrs_arr[4] = getTablesHead()->table_cols_head->next->next->next->next;
			the_select_node->columns_col_ptrs_arr[5] = getTablesHead()->table_cols_head->next->next->next->next->next;
			the_select_node->columns_col_ptrs_arr[6] = getTablesHead()->table_cols_head->next->next->next->next->next->next;

			the_select_node->columns_new_names_arr = (char**) myMalloc(sizeof(struct table_cols_info*) * 7, NULL, &malloced_head, the_debug);
			the_select_node->columns_new_names_arr[0] = upper("Col1", NULL, &malloced_head, the_debug);
			the_select_node->columns_new_names_arr[1] = upper("Col2", NULL, &malloced_head, the_debug);
			the_select_node->columns_new_names_arr[2] = upper("Col3", NULL, &malloced_head, the_debug);
			the_select_node->columns_new_names_arr[3] = NULL;
			the_select_node->columns_new_names_arr[4] = upper("Col5", NULL, &malloced_head, the_debug);
			the_select_node->columns_new_names_arr[5] = upper("Col6", NULL, &malloced_head, the_debug);
			the_select_node->columns_new_names_arr[6] = upper("Col7", NULL, &malloced_head, the_debug);

			the_select_node->or_head = NULL;
			the_select_node->join_head = NULL;

			the_select_node->next = NULL;

			the_select_node->prev = (struct select_node*) myMalloc(sizeof(struct select_node), NULL, &malloced_head, the_debug);
			the_select_node->prev->select_node_alias = upper("TBL", NULL, &malloced_head, the_debug);

			the_select_node->prev->columns_arr_size = 7;

			the_select_node->prev->columns_table_ptrs_arr = (struct table_info**) myMalloc(sizeof(struct table_info*) * 7, NULL, &malloced_head, the_debug);
			the_select_node->prev->columns_table_ptrs_arr[0] = getTablesHead();
			the_select_node->prev->columns_table_ptrs_arr[1] = getTablesHead();
			the_select_node->prev->columns_table_ptrs_arr[2] = getTablesHead();
			the_select_node->prev->columns_table_ptrs_arr[3] = getTablesHead();
			the_select_node->prev->columns_table_ptrs_arr[4] = getTablesHead();
			the_select_node->prev->columns_table_ptrs_arr[5] = getTablesHead();
			the_select_node->prev->columns_table_ptrs_arr[6] = getTablesHead();

			the_select_node->prev->columns_col_ptrs_arr = (struct table_cols_info**) myMalloc(sizeof(struct table_cols_info*) * 7, NULL, &malloced_head, the_debug);
			the_select_node->prev->columns_col_ptrs_arr[0] = getTablesHead()->table_cols_head;
			the_select_node->prev->columns_col_ptrs_arr[1] = getTablesHead()->table_cols_head->next;
			the_select_node->prev->columns_col_ptrs_arr[2] = getTablesHead()->table_cols_head->next->next;
			the_select_node->prev->columns_col_ptrs_arr[3] = getTablesHead()->table_cols_head->next->next->next;
			the_select_node->prev->columns_col_ptrs_arr[4] = getTablesHead()->table_cols_head->next->next->next->next;
			the_select_node->prev->columns_col_ptrs_arr[5] = getTablesHead()->table_cols_head->next->next->next->next->next;
			the_select_node->prev->columns_col_ptrs_arr[6] = getTablesHead()->table_cols_head->next->next->next->next->next->next;

			the_select_node->prev->columns_new_names_arr = NULL;

			the_select_node->prev->or_head = NULL;
			the_select_node->prev->join_head = NULL;

			the_select_node->prev->next = the_select_node;
			the_select_node->prev->prev = NULL;


			if (test_Controller_parseSelect(519, "select ALC_Brands Col1, CT-REGISTRATION-NUMBER Col2, STATUS Col3,EFFECTIVE, EXPIRATION as Col5, OUT-OF-STATE-SHIPPER as Col6, SUPERVISOR-CREDENTIAL Col7 from alc_brands as tbl;", &(the_select_node->prev)
										   ,&parsed_error_code, &malloced_head, the_debug) != 0)
				result = -1;

			if (parsed_error_code == 0)
			{
				while (the_select_node != NULL)
				{
					struct select_node* temp = the_select_node;
					the_select_node = the_select_node->prev;

					myFree((void**) &temp->columns_table_ptrs_arr, NULL, &malloced_head, the_debug);
					myFree((void**) &temp->columns_col_ptrs_arr, NULL, &malloced_head, the_debug);
					myFree((void**) &temp->select_node_alias, NULL, &malloced_head, the_debug);
					myFree((void**) &temp, NULL, &malloced_head, the_debug);
				}
			}

			if (malloced_head != NULL)
			{
				if (the_debug == YES_DEBUG)
					printf("	ERROR in test_Driver_main() at line %d in %s\n", __LINE__, __FILE__);
				return -3;
			}
		// END Test with id = 519
	// END test_Controller_parseSelect

	// START test_Controller_parseWhereClause
		// START Test with id = 101
			struct or_clause_node* the_or_node = NULL;


			if (test_Controller_parseWhereClause(101, "", "where"
												,-1, &the_or_node
												,&malloced_head, the_debug) != 0)
				result = -1;


			if (malloced_head != NULL)
			{
				if (the_debug == YES_DEBUG)
					printf("	ERROR in test_Driver_main() at line %d in %s\n", __LINE__, __FILE__);
				return -3;
			}
		// END Test with id = 101

		// START Test with id = 102
			the_or_node = NULL;


			if (test_Controller_parseWhereClause(102, "where adaj jsfnoef = fiaenf'  '' ''';   ;", "where"
												,-1, &the_or_node
												,&malloced_head, the_debug) != 0)
				result = -1;


			if (malloced_head != NULL)
			{
				if (the_debug == YES_DEBUG)
					printf("	ERROR in test_Driver_main() at line %d in %s\n", __LINE__, __FILE__);
				return -3;
			}
		// END Test with id = 102

		// START Test with id = 103
			the_or_node = NULL;


			if (test_Controller_parseWhereClause(103, "where BRAND-NAME;", "where"
												,-1, &the_or_node
												,&malloced_head, the_debug) != 0)
				result = -1;


			if (malloced_head != NULL)
			{
				if (the_debug == YES_DEBUG)
					printf("	ERROR in test_Driver_main() at line %d in %s\n", __LINE__, __FILE__);
				return -3;
			}
		// END Test with id = 103

		// START Test with id = 104
			the_or_node = (struct or_clause_node*) myMalloc(sizeof(struct or_clause_node), NULL, &malloced_head, the_debug);

			the_or_node->and_head = (struct and_clause_node*) myMalloc(sizeof(struct and_clause_node), NULL, &malloced_head, the_debug);
			
			the_or_node->and_head->table = getTablesHead();
			the_or_node->and_head->col = getTablesHead()->table_cols_head;
			the_or_node->and_head->table_joined = NULL;
			the_or_node->and_head->col_joined = NULL;
			the_or_node->and_head->where_type = WHERE_IS_EQUALS;
			the_or_node->and_head->data_string = (char*) myMalloc(sizeof(char) * 64, NULL, &malloced_head, the_debug);
			strcpy(the_or_node->and_head->data_string, "test");

			the_or_node->and_head->next = NULL;

			the_or_node->next = NULL;


			if (test_Controller_parseWhereClause(104, "WherE braND-name = 'test';", "where"
												,0, &the_or_node
												,&malloced_head, the_debug) != 0)
				result = -1;
			

			// START Free stuff
			while (the_or_node != NULL)
			{
				while (the_or_node->and_head != NULL)
				{
					struct and_clause_node* temp = the_or_node->and_head;
					the_or_node->and_head = the_or_node->and_head->next;

					myFree((void**) &(temp->data_string), NULL, &malloced_head, the_debug);
					myFree((void**) &temp, NULL, &malloced_head, the_debug);
				}
				struct or_clause_node* temp = the_or_node;
				the_or_node = the_or_node->next;

				myFree((void**) &temp, NULL, &malloced_head, the_debug);
			}
			// END Free stuff


			if (malloced_head != NULL)
			{
				if (the_debug == YES_DEBUG)
					printf("	ERROR in test_Driver_main() at line %d in %s\n", __LINE__, __FILE__);
				return -3;
			}
		// END Test with id = 104

		// START Test with id = 105
			the_or_node = NULL;


			if (test_Controller_parseWhereClause(105, "WherE braND-name = 'awdawd' aNd EFFECTIVE = 105  		     ;", "where"
												,-1, &the_or_node
												,&malloced_head, the_debug) != 0)
				result = -1;


			if (malloced_head != NULL)
			{
				if (the_debug == YES_DEBUG)
					printf("	ERROR in test_Driver_main() at line %d in %s\n", __LINE__, __FILE__);
				return -3;
			}
		// END Test with id = 105

		// START Test with id = 106
			the_or_node = (struct or_clause_node*) myMalloc(sizeof(struct or_clause_node), NULL, &malloced_head, the_debug);

			the_or_node->and_head = (struct and_clause_node*) myMalloc(sizeof(struct and_clause_node), NULL, &malloced_head, the_debug);
			
			the_or_node->and_head->table = getTablesHead();
			the_or_node->and_head->col = getTablesHead()->table_cols_head;
			the_or_node->and_head->table_joined = NULL;
			the_or_node->and_head->col_joined = NULL;
			the_or_node->and_head->where_type = WHERE_IS_EQUALS;
			the_or_node->and_head->data_string = (char*) myMalloc(sizeof(char) * 64, NULL, &malloced_head, the_debug);
			strcpy(the_or_node->and_head->data_string, "test");

			the_or_node->and_head->next = (struct and_clause_node*) myMalloc(sizeof(struct and_clause_node), NULL, &malloced_head, the_debug);
			
			the_or_node->and_head->next->table = getTablesHead();
			the_or_node->and_head->next->col = getTablesHead()->table_cols_head->next->next;
			the_or_node->and_head->next->table_joined = NULL;
			the_or_node->and_head->next->col_joined = NULL;
			the_or_node->and_head->next->where_type = WHERE_IS_EQUALS;
			the_or_node->and_head->next->data_string = (char*) myMalloc(sizeof(char) * 64, NULL, &malloced_head, the_debug);
			strcpy(the_or_node->and_head->next->data_string, "That");

			the_or_node->and_head->next->next = NULL;

			the_or_node->next = NULL;


			if (test_Controller_parseWhereClause(106, "WherE braND-name = 'test' aNd status = 'That' \n ;", "where"
												,0, &the_or_node
												,&malloced_head, the_debug) != 0)
				result = -1;


			// START Free stuff
			while (the_or_node != NULL)
			{
				while (the_or_node->and_head != NULL)
				{
					struct and_clause_node* temp = the_or_node->and_head;
					the_or_node->and_head = the_or_node->and_head->next;

					myFree((void**) &(temp->data_string), NULL, &malloced_head, the_debug);
					myFree((void**) &temp, NULL, &malloced_head, the_debug);
				}
				struct or_clause_node* temp = the_or_node;
				the_or_node = the_or_node->next;

				myFree((void**) &temp, NULL, &malloced_head, the_debug);
			}
			// END Free stuff


			if (malloced_head != NULL)
			{
				if (the_debug == YES_DEBUG)
					printf("	ERROR in test_Driver_main() at line %d in %s\n", __LINE__, __FILE__);
				return -3;
			}
		// END Test with id = 106

		// START Test with id = 107
			the_or_node = (struct or_clause_node*) myMalloc(sizeof(struct or_clause_node), NULL, &malloced_head, the_debug);

			the_or_node->and_head = (struct and_clause_node*) myMalloc(sizeof(struct and_clause_node), NULL, &malloced_head, the_debug);
			
			the_or_node->and_head->table = getTablesHead();
			the_or_node->and_head->col = getTablesHead()->table_cols_head;
			the_or_node->and_head->table_joined = NULL;
			the_or_node->and_head->col_joined = NULL;
			the_or_node->and_head->where_type = WHERE_IS_EQUALS;
			the_or_node->and_head->data_string = (char*) myMalloc(sizeof(char) * 64, NULL, &malloced_head, the_debug);
			strcpy(the_or_node->and_head->data_string, "test");

			the_or_node->and_head->next = (struct and_clause_node*) myMalloc(sizeof(struct and_clause_node), NULL, &malloced_head, the_debug);
			
			the_or_node->and_head->next->table = getTablesHead();
			the_or_node->and_head->next->col = getTablesHead()->table_cols_head->next->next;
			the_or_node->and_head->next->table_joined = NULL;
			the_or_node->and_head->next->col_joined = NULL;
			the_or_node->and_head->next->where_type = WHERE_IS_EQUALS;
			the_or_node->and_head->next->data_string = (char*) myMalloc(sizeof(char) * 64, NULL, &malloced_head, the_debug);
			strcpy(the_or_node->and_head->next->data_string, "That");

			the_or_node->and_head->next->next = NULL;

			the_or_node->next = NULL;


			if (test_Controller_parseWhereClause(107, "WherE      braND-name    =    'test' \n    aNd     	 status = 'That' \n ;", "where"
												,0, &the_or_node
												,&malloced_head, the_debug) != 0)
				result = -1;


			// START Free stuff
			while (the_or_node != NULL)
			{
				while (the_or_node->and_head != NULL)
				{
					struct and_clause_node* temp = the_or_node->and_head;
					the_or_node->and_head = the_or_node->and_head->next;

					myFree((void**) &(temp->data_string), NULL, &malloced_head, the_debug);
					myFree((void**) &temp, NULL, &malloced_head, the_debug);
				}
				struct or_clause_node* temp = the_or_node;
				the_or_node = the_or_node->next;

				myFree((void**) &temp, NULL, &malloced_head, the_debug);
			}
			// END Free stuff


			if (malloced_head != NULL)
			{
				if (the_debug == YES_DEBUG)
					printf("	ERROR in test_Driver_main() at line %d in %s\n", __LINE__, __FILE__);
				return -3;
			}
		// END Test with id = 107

		// START Test with id = 108
			the_or_node = (struct or_clause_node*) myMalloc(sizeof(struct or_clause_node), NULL, &malloced_head, the_debug);

			the_or_node->and_head = (struct and_clause_node*) myMalloc(sizeof(struct and_clause_node), NULL, &malloced_head, the_debug);
			
			the_or_node->and_head->table = getTablesHead();
			the_or_node->and_head->col = getTablesHead()->table_cols_head;
			the_or_node->and_head->table_joined = NULL;
			the_or_node->and_head->col_joined = NULL;
			the_or_node->and_head->where_type = WHERE_IS_EQUALS;
			the_or_node->and_head->data_string = (char*) myMalloc(sizeof(char) * 64, NULL, &malloced_head, the_debug);
			strcpy(the_or_node->and_head->data_string, "test");

			the_or_node->and_head->next = NULL;

			the_or_node->next = (struct or_clause_node*) myMalloc(sizeof(struct or_clause_node), NULL, &malloced_head, the_debug);

			the_or_node->next->and_head = (struct and_clause_node*) myMalloc(sizeof(struct and_clause_node), NULL, &malloced_head, the_debug);
			
			the_or_node->next->and_head->table = getTablesHead();
			the_or_node->next->and_head->col = getTablesHead()->table_cols_head->next->next;
			the_or_node->next->and_head->table_joined = NULL;
			the_or_node->next->and_head->col_joined = NULL;
			the_or_node->next->and_head->where_type = WHERE_IS_EQUALS;
			the_or_node->next->and_head->data_string = (char*) myMalloc(sizeof(char) * 64, NULL, &malloced_head, the_debug);
			strcpy(the_or_node->next->and_head->data_string, "That");

			the_or_node->next->and_head->next = NULL;

			the_or_node->next->next = NULL;


			if (test_Controller_parseWhereClause(108, "WherE      braND-name    =    'test' \n    or     	 status = 'That' \n ;", "where"
												,0, &the_or_node
												,&malloced_head, the_debug) != 0)
				result = -1;


			// START Free stuff
			while (the_or_node != NULL)
			{
				while (the_or_node->and_head != NULL)
				{
					struct and_clause_node* temp = the_or_node->and_head;
					the_or_node->and_head = the_or_node->and_head->next;

					myFree((void**) &(temp->data_string), NULL, &malloced_head, the_debug);
					myFree((void**) &temp, NULL, &malloced_head, the_debug);
				}
				struct or_clause_node* temp = the_or_node;
				the_or_node = the_or_node->next;

				myFree((void**) &temp, NULL, &malloced_head, the_debug);
			}
			// END Free stuff


			if (malloced_head != NULL)
			{
				if (the_debug == YES_DEBUG)
					printf("	ERROR in test_Driver_main() at line %d in %s\n", __LINE__, __FILE__);
				return -3;
			}
		// END Test with id = 108

		// START Test with id = 109
			the_or_node = (struct or_clause_node*) myMalloc(sizeof(struct or_clause_node), NULL, &malloced_head, the_debug);

			the_or_node->and_head = (struct and_clause_node*) myMalloc(sizeof(struct and_clause_node), NULL, &malloced_head, the_debug);
			
			the_or_node->and_head->table = getTablesHead();
			the_or_node->and_head->col = getTablesHead()->table_cols_head;
			the_or_node->and_head->table_joined = NULL;
			the_or_node->and_head->col_joined = NULL;
			the_or_node->and_head->where_type = WHERE_IS_EQUALS;
			the_or_node->and_head->data_string = (char*) myMalloc(sizeof(char) * 64, NULL, &malloced_head, the_debug);
			strcpy(the_or_node->and_head->data_string, "test");

			the_or_node->and_head->next = NULL;

			the_or_node->next = (struct or_clause_node*) myMalloc(sizeof(struct or_clause_node), NULL, &malloced_head, the_debug);

			the_or_node->next->and_head = (struct and_clause_node*) myMalloc(sizeof(struct and_clause_node), NULL, &malloced_head, the_debug);
			
			the_or_node->next->and_head->table = getTablesHead();
			the_or_node->next->and_head->col = getTablesHead()->table_cols_head->next->next;
			the_or_node->next->and_head->table_joined = NULL;
			the_or_node->next->and_head->col_joined = NULL;
			the_or_node->next->and_head->where_type = WHERE_IS_EQUALS;
			the_or_node->next->and_head->data_string = (char*) myMalloc(sizeof(char) * 64, NULL, &malloced_head, the_debug);
			strcpy(the_or_node->next->and_head->data_string, "That");

			the_or_node->next->and_head->next = NULL;

			the_or_node->next->next = NULL;


			if (test_Controller_parseWhereClause(109, "WherE    'test' =   braND-name    \n    or     'That' = 	 status  \n ;", "where"
												,0, &the_or_node
												,&malloced_head, the_debug) != 0)
				result = -1;

			
			// START Free stuff
			while (the_or_node != NULL)
			{
				while (the_or_node->and_head != NULL)
				{
					struct and_clause_node* temp = the_or_node->and_head;
					the_or_node->and_head = the_or_node->and_head->next;

					myFree((void**) &(temp->data_string), NULL, &malloced_head, the_debug);
					myFree((void**) &temp, NULL, &malloced_head, the_debug);
				}
				struct or_clause_node* temp = the_or_node;
				the_or_node = the_or_node->next;

				myFree((void**) &temp, NULL, &malloced_head, the_debug);
			}
			// END Free stuff
			

			if (malloced_head != NULL)
			{
				if (the_debug == YES_DEBUG)
					printf("	ERROR in test_Driver_main() at line %d in %s\n", __LINE__, __FILE__);
				return -3;
			}
		// END Test with id = 109

		// START Test with id = 110
			the_or_node = (struct or_clause_node*) myMalloc(sizeof(struct or_clause_node), NULL, &malloced_head, the_debug);

			the_or_node->and_head = (struct and_clause_node*) myMalloc(sizeof(struct and_clause_node), NULL, &malloced_head, the_debug);
			
			the_or_node->and_head->table = getTablesHead();
			the_or_node->and_head->col = getTablesHead()->table_cols_head;
			the_or_node->and_head->table_joined = NULL;
			the_or_node->and_head->col_joined = NULL;
			the_or_node->and_head->where_type = WHERE_IS_EQUALS;
			the_or_node->and_head->data_string = (char*) myMalloc(sizeof(char) * 64, NULL, &malloced_head, the_debug);
			strcpy(the_or_node->and_head->data_string, "test");

			the_or_node->and_head->next = (struct and_clause_node*) myMalloc(sizeof(struct and_clause_node), NULL, &malloced_head, the_debug);
			
			the_or_node->and_head->next->table = getTablesHead();
			the_or_node->and_head->next->col = getTablesHead()->table_cols_head;
			the_or_node->and_head->next->table_joined = NULL;
			the_or_node->and_head->next->col_joined = NULL;
			the_or_node->and_head->next->where_type = WHERE_IS_EQUALS;
			the_or_node->and_head->next->data_string = (char*) myMalloc(sizeof(char) * 64, NULL, &malloced_head, the_debug);
			strcpy(the_or_node->and_head->next->data_string, "Hi;");

			the_or_node->and_head->next->next = NULL;

			the_or_node->next = (struct or_clause_node*) myMalloc(sizeof(struct or_clause_node), NULL, &malloced_head, the_debug);

			the_or_node->next->and_head = (struct and_clause_node*) myMalloc(sizeof(struct and_clause_node), NULL, &malloced_head, the_debug);
			
			the_or_node->next->and_head->table = getTablesHead();
			the_or_node->next->and_head->col = getTablesHead()->table_cols_head->next->next;
			the_or_node->next->and_head->table_joined = NULL;
			the_or_node->next->and_head->col_joined = NULL;
			the_or_node->next->and_head->where_type = WHERE_IS_EQUALS;
			the_or_node->next->and_head->data_string = (char*) myMalloc(sizeof(char) * 64, NULL, &malloced_head, the_debug);
			strcpy(the_or_node->next->and_head->data_string, "That");

			the_or_node->next->and_head->next = (struct and_clause_node*) myMalloc(sizeof(struct and_clause_node), NULL, &malloced_head, the_debug);
			
			the_or_node->next->and_head->next->table = getTablesHead();
			the_or_node->next->and_head->next->col = getTablesHead()->table_cols_head->next->next;
			the_or_node->next->and_head->next->table_joined = NULL;
			the_or_node->next->and_head->next->col_joined = NULL;
			the_or_node->next->and_head->next->where_type = WHERE_IS_EQUALS;
			the_or_node->next->and_head->next->data_string = (char*) myMalloc(sizeof(char) * 64, NULL, &malloced_head, the_debug);
			strcpy(the_or_node->next->and_head->next->data_string, "bye");

			the_or_node->next->and_head->next->next = NULL;

			the_or_node->next->next = NULL;


			if (test_Controller_parseWhereClause(110, "WherE 'test' = braND-name and braND-name = 'Hi;' or 'That' = status and status = 'bye';", "where"
												,0, &the_or_node
												,&malloced_head, the_debug) != 0)
				result = -1;

			
			// START Free stuff
			while (the_or_node != NULL)
			{
				while (the_or_node->and_head != NULL)
				{
					struct and_clause_node* temp = the_or_node->and_head;
					the_or_node->and_head = the_or_node->and_head->next;

					myFree((void**) &(temp->data_string), NULL, &malloced_head, the_debug);
					myFree((void**) &temp, NULL, &malloced_head, the_debug);
				}
				struct or_clause_node* temp = the_or_node;
				the_or_node = the_or_node->next;

				myFree((void**) &temp, NULL, &malloced_head, the_debug);
			}
			// END Free stuff
			


			if (malloced_head != NULL)
			{
				if (the_debug == YES_DEBUG)
					printf("	ERROR in test_Driver_main() at line %d in %s\n", __LINE__, __FILE__);
				return -3;
			}
		// END Test with id = 110

		// START Test with id = 111
			the_or_node = NULL;


			if (test_Controller_parseWhereClause(111, "where BRAND-NAME = 13123;", "on"
												,-1, &the_or_node
												,&malloced_head, the_debug) != 0)
				result = -1;


			if (malloced_head != NULL)
			{
				if (the_debug == YES_DEBUG)
					printf("	ERROR in test_Driver_main() at line %d in %s\n", __LINE__, __FILE__);
				return -3;
			}
		// END Test with id = 111

		// START Test with id = 112
			the_or_node = (struct or_clause_node*) myMalloc(sizeof(struct or_clause_node), NULL, &malloced_head, the_debug);

			the_or_node->and_head = (struct and_clause_node*) myMalloc(sizeof(struct and_clause_node), NULL, &malloced_head, the_debug);
			
			the_or_node->and_head->table = getTablesHead();
			the_or_node->and_head->col = getTablesHead()->table_cols_head;
			the_or_node->and_head->table_joined = NULL;
			the_or_node->and_head->col_joined = NULL;
			the_or_node->and_head->where_type = WHERE_IS_EQUALS;
			the_or_node->and_head->data_string = (char*) myMalloc(sizeof(char) * 64, NULL, &malloced_head, the_debug);
			strcpy(the_or_node->and_head->data_string, "test");

			the_or_node->and_head->next = NULL;

			the_or_node->next = NULL;


			if (test_Controller_parseWhereClause(112, "oN braND-name = 'test';", "on"
												,0, &the_or_node
												,&malloced_head, the_debug) != 0)
				result = -1;

			
			// START Free stuff
			while (the_or_node != NULL)
			{
				while (the_or_node->and_head != NULL)
				{
					struct and_clause_node* temp = the_or_node->and_head;
					the_or_node->and_head = the_or_node->and_head->next;

					myFree((void**) &(temp->data_string), NULL, &malloced_head, the_debug);
					myFree((void**) &temp, NULL, &malloced_head, the_debug);
				}
				struct or_clause_node* temp = the_or_node;
				the_or_node = the_or_node->next;

				myFree((void**) &temp, NULL, &malloced_head, the_debug);
			}
			// END Free stuff
			

			if (malloced_head != NULL)
			{
				if (the_debug == YES_DEBUG)
					printf("	ERROR in test_Driver_main() at line %d in %s\n", __LINE__, __FILE__);
				return -3;
			}
		// END Test with id = 112

		// START Test with id = 113
			the_or_node = (struct or_clause_node*) myMalloc(sizeof(struct or_clause_node), NULL, &malloced_head, the_debug);

			the_or_node->and_head = (struct and_clause_node*) myMalloc(sizeof(struct and_clause_node), NULL, &malloced_head, the_debug);
			
			the_or_node->and_head->table = getTablesHead();
			the_or_node->and_head->col = getTablesHead()->table_cols_head;
			the_or_node->and_head->table_joined = getTablesHead();
			the_or_node->and_head->col_joined = getTablesHead()->table_cols_head;
			the_or_node->and_head->data_string = NULL;

			the_or_node->and_head->where_type = WHERE_IS_EQUALS;

			the_or_node->and_head->next = NULL;

			the_or_node->next = NULL;


			if (test_Controller_parseWhereClause(113, "on tbl.braND-name = tbl2.braND-name ", "on"
												,0, &the_or_node
												,&malloced_head, the_debug) != 0)
				result = -1;

			
			// START Free stuff
			while (the_or_node != NULL)
			{
				while (the_or_node->and_head != NULL)
				{
					struct and_clause_node* temp = the_or_node->and_head;
					the_or_node->and_head = the_or_node->and_head->next;

					myFree((void**) &(temp->data_string), NULL, &malloced_head, the_debug);
					myFree((void**) &temp, NULL, &malloced_head, the_debug);
				}
				struct or_clause_node* temp = the_or_node;
				the_or_node = the_or_node->next;

				myFree((void**) &temp, NULL, &malloced_head, the_debug);
			}
			// END Free stuff
			

			if (malloced_head != NULL)
			{
				if (the_debug == YES_DEBUG)
					printf("	ERROR in test_Driver_main() at line %d in %s\n", __LINE__, __FILE__);
				return -3;
			}
		// END Test with id = 113

		// START Test with id = 114
			the_or_node = NULL;


			if (test_Controller_parseWhereClause(114, "on tbl.braND-name = tbl2.braND-name and tbl1.SUPERVISOR-CREDENTIAL = tbl2.SUPERVISOR-CREDENTIAL;", "on"
												,-1, &the_or_node
												,&malloced_head, the_debug) != 0)
				result = -1;


			if (malloced_head != NULL)
			{
				if (the_debug == YES_DEBUG)
					printf("	ERROR in test_Driver_main() at line %d in %s\n", __LINE__, __FILE__);
				return -3;
			}
		// END Test with id = 114

		// START Test with id = 115
			the_or_node = (struct or_clause_node*) myMalloc(sizeof(struct or_clause_node), NULL, &malloced_head, the_debug);

			the_or_node->and_head = (struct and_clause_node*) myMalloc(sizeof(struct and_clause_node), NULL, &malloced_head, the_debug);
			
			the_or_node->and_head->table = getTablesHead();
			the_or_node->and_head->col = getTablesHead()->table_cols_head;
			the_or_node->and_head->table_joined = getTablesHead();
			the_or_node->and_head->col_joined = getTablesHead()->table_cols_head;
			the_or_node->and_head->where_type = WHERE_IS_EQUALS;
			the_or_node->and_head->data_string = NULL;

			the_or_node->and_head->next = (struct and_clause_node*) myMalloc(sizeof(struct and_clause_node), NULL, &malloced_head, the_debug);
			
			the_or_node->and_head->next->table = getTablesHead();
			the_or_node->and_head->next->col = getTablesHead()->table_cols_head->next->next->next->next->next->next;
			the_or_node->and_head->next->table_joined = getTablesHead();
			the_or_node->and_head->next->col_joined = getTablesHead()->table_cols_head->next->next->next->next->next->next;
			the_or_node->and_head->next->where_type = WHERE_IS_EQUALS;
			the_or_node->and_head->next->data_string = NULL;

			the_or_node->and_head->next->next = NULL;

			the_or_node->next = NULL;


			if (test_Controller_parseWhereClause(115, "on tbl.braND-name = tbl2.braND-name and tbl.SUPERVISOR-CREDENTIAL = tbl2.SUPERVISOR-CREDENTIAL;", "on"
												,0, &the_or_node
												,&malloced_head, the_debug) != 0)
				result = -1;

			
			// START Free stuff
			while (the_or_node != NULL)
			{
				while (the_or_node->and_head != NULL)
				{
					struct and_clause_node* temp = the_or_node->and_head;
					the_or_node->and_head = the_or_node->and_head->next;

					myFree((void**) &(temp->data_string), NULL, &malloced_head, the_debug);
					myFree((void**) &temp, NULL, &malloced_head, the_debug);
				}
				struct or_clause_node* temp = the_or_node;
				the_or_node = the_or_node->next;

				myFree((void**) &temp, NULL, &malloced_head, the_debug);
			}
			// END Free stuff
			

			if (malloced_head != NULL)
			{
				if (the_debug == YES_DEBUG)
					printf("	ERROR in test_Driver_main() at line %d in %s\n", __LINE__, __FILE__);
				return -3;
			}
		// END Test with id = 115

		// START Test with id = 116
			the_or_node = (struct or_clause_node*) myMalloc(sizeof(struct or_clause_node), NULL, &malloced_head, the_debug);

			the_or_node->and_head = (struct and_clause_node*) myMalloc(sizeof(struct and_clause_node), NULL, &malloced_head, the_debug);
			
			the_or_node->and_head->table = getTablesHead();
			the_or_node->and_head->col = getTablesHead()->table_cols_head;
			the_or_node->and_head->table_joined = getTablesHead();
			the_or_node->and_head->col_joined = getTablesHead()->table_cols_head;
			the_or_node->and_head->where_type = WHERE_IS_EQUALS;
			the_or_node->and_head->data_string = NULL;

			the_or_node->and_head->next = NULL;

			the_or_node->next = (struct or_clause_node*) myMalloc(sizeof(struct or_clause_node), NULL, &malloced_head, the_debug);

			the_or_node->next->and_head = (struct and_clause_node*) myMalloc(sizeof(struct and_clause_node), NULL, &malloced_head, the_debug);
			
			the_or_node->next->and_head->table = getTablesHead();
			the_or_node->next->and_head->col = getTablesHead()->table_cols_head->next->next->next->next->next->next;
			the_or_node->next->and_head->table_joined = getTablesHead();
			the_or_node->next->and_head->col_joined = getTablesHead()->table_cols_head->next->next->next->next->next->next;
			the_or_node->next->and_head->where_type = WHERE_IS_EQUALS;
			the_or_node->next->and_head->data_string = NULL;

			the_or_node->next->and_head->next = NULL;

			the_or_node->next->next = NULL;


			if (test_Controller_parseWhereClause(116, "on tbl.braND-name = tbl2.braND-name or tbl.SUPERVISOR-CREDENTIAL = tbl2.SUPERVISOR-CREDENTIAL;", "on"
												,0, &the_or_node
												,&malloced_head, the_debug) != 0)
				result = -1;

			
			// START Free stuff
			while (the_or_node != NULL)
			{
				while (the_or_node->and_head != NULL)
				{
					struct and_clause_node* temp = the_or_node->and_head;
					the_or_node->and_head = the_or_node->and_head->next;

					myFree((void**) &(temp->data_string), NULL, &malloced_head, the_debug);
					myFree((void**) &temp, NULL, &malloced_head, the_debug);
				}
				struct or_clause_node* temp = the_or_node;
				the_or_node = the_or_node->next;

				myFree((void**) &temp, NULL, &malloced_head, the_debug);
			}
			// END Free stuff
			

			if (malloced_head != NULL)
			{
				if (the_debug == YES_DEBUG)
					printf("	ERROR in test_Driver_main() at line %d in %s\n", __LINE__, __FILE__);
				return -3;
			}
		// END Test with id = 116

		// START Test with id = 117
			the_or_node = (struct or_clause_node*) myMalloc(sizeof(struct or_clause_node), NULL, &malloced_head, the_debug);

			the_or_node->and_head = (struct and_clause_node*) myMalloc(sizeof(struct and_clause_node), NULL, &malloced_head, the_debug);
			
			the_or_node->and_head->table = getTablesHead();
			the_or_node->and_head->col = getTablesHead()->table_cols_head;
			the_or_node->and_head->table_joined = getTablesHead();
			the_or_node->and_head->col_joined = getTablesHead()->table_cols_head;
			the_or_node->and_head->where_type = WHERE_IS_EQUALS;
			the_or_node->and_head->data_string = NULL;

			the_or_node->and_head->next = NULL;

			the_or_node->next = (struct or_clause_node*) myMalloc(sizeof(struct or_clause_node), NULL, &malloced_head, the_debug);

			the_or_node->next->and_head = (struct and_clause_node*) myMalloc(sizeof(struct and_clause_node), NULL, &malloced_head, the_debug);
			
			the_or_node->next->and_head->table = getTablesHead();
			the_or_node->next->and_head->col = getTablesHead()->table_cols_head->next->next->next->next->next->next;
			the_or_node->next->and_head->table_joined = NULL;
			the_or_node->next->and_head->col_joined = NULL;
			the_or_node->next->and_head->where_type = WHERE_IS_EQUALS;
			the_or_node->next->and_head->data_string = (char*) myMalloc(sizeof(char) * 64, NULL, &malloced_head, the_debug);
			strcpy(the_or_node->next->and_head->data_string, "this");

			the_or_node->next->and_head->next = NULL;

			the_or_node->next->next = NULL;


			if (test_Controller_parseWhereClause(117, "on tbl.braND-name = tbl2.braND-name or tbl.SUPERVISOR-CREDENTIAL = 'this';", "on"
												,0, &the_or_node
												,&malloced_head, the_debug) != 0)
				result = -1;

			
			// START Free stuff
			while (the_or_node != NULL)
			{
				while (the_or_node->and_head != NULL)
				{
					struct and_clause_node* temp = the_or_node->and_head;
					the_or_node->and_head = the_or_node->and_head->next;

					myFree((void**) &(temp->data_string), NULL, &malloced_head, the_debug);
					myFree((void**) &temp, NULL, &malloced_head, the_debug);
				}
				struct or_clause_node* temp = the_or_node;
				the_or_node = the_or_node->next;

				myFree((void**) &temp, NULL, &malloced_head, the_debug);
			}
			// END Free stuff
			

			if (malloced_head != NULL)
			{
				if (the_debug == YES_DEBUG)
					printf("	ERROR in test_Driver_main() at line %d in %s\n", __LINE__, __FILE__);
				return -3;
			}
		// END Test with id = 117
	// END test_Controller_parseWhereClause

	// START test_Controller_parseUpdate
		// START Test with id = 201
			struct change_node_v2* expected_change_head = NULL;

			int parsed_error_code = 0;
			if (test_Controller_parseUpdate(108, "UPDATE * 	 from   \n    table set 		this = 'that'   ;"
										   ,&expected_change_head, &parsed_error_code, &malloced_head, the_debug) != 0)
				result = -1;

			if (malloced_head != NULL)
			{
				if (the_debug == YES_DEBUG)
					printf("	ERROR in test_Driver_main() at line %d in %s\n", __LINE__, __FILE__);
				return -3;
			}
		// END Test with id = 201

		// START Test with id = 202
			expected_change_head = (struct change_node_v2*) myMalloc(sizeof(struct change_node_v2), NULL, &malloced_head, the_debug);
			if (expected_change_head == NULL) { if (the_debug == YES_DEBUG) printf("	ERROR in test_Driver_main() at line %d in %s\n", __LINE__, __FILE__); return -1; }
			expected_change_head->col_number = 2;
			expected_change_head->operation = 3;
			expected_change_head->data_type = DATA_STRING;
			expected_change_head->data = (char*) myMalloc(sizeof(char) * 32, NULL, &malloced_head, the_debug);
			if (expected_change_head->data == NULL) { if (the_debug == YES_DEBUG) printf("	ERROR in test_Driver_main() at line %d in %s\n", __LINE__, __FILE__); return -1; }
			strcpy(expected_change_head->data, "TST_ACTIVE");
			expected_change_head->next = NULL;

			parsed_error_code = 0;
			if (test_Controller_parseUpdate(109, "UPDATE ALC_BRANDS\nSET status = 'TST_ACTIVE'\n;"
										   ,&expected_change_head, &parsed_error_code, &malloced_head, the_debug) != 0)
				result = -1;

			if (parsed_error_code == 0)
			{
				myFree((void**) &expected_change_head->data, NULL, &malloced_head, the_debug);
				myFree((void**) &expected_change_head, NULL, &malloced_head, the_debug);
			}		

			if (malloced_head != NULL)
			{
				if (the_debug == YES_DEBUG)
					printf("	ERROR in test_Driver_main() at line %d in %s\n", __LINE__, __FILE__);
				return -3;
			}
		// END Test with id = 202

		// START Test with id = 203
			expected_change_head = (struct change_node_v2*) myMalloc(sizeof(struct change_node_v2), NULL, &malloced_head, the_debug);
			if (expected_change_head == NULL) { if (the_debug == YES_DEBUG) printf("	ERROR in test_Driver_main() at line %d in %s\n", __LINE__, __FILE__); return -1; }
			expected_change_head->col_number = 2;
			expected_change_head->operation = 3;
			expected_change_head->data_type = DATA_STRING;
			expected_change_head->data = (char*) myMalloc(sizeof(char) * 32, NULL, &malloced_head, the_debug);
			if (expected_change_head->data == NULL) { if (the_debug == YES_DEBUG) printf("	ERROR in test_Driver_main() at line %d in %s\n", __LINE__, __FILE__); return -1; }
			strcpy(expected_change_head->data, "TST_ACTIVE");
			
			expected_change_head->next = (struct change_node_v2*) myMalloc(sizeof(struct change_node_v2), NULL, &malloced_head, the_debug);
			if (expected_change_head->next == NULL) { if (the_debug == YES_DEBUG) printf("	ERROR in test_Driver_main() at line %d in %s\n", __LINE__, __FILE__); return -1; }
			expected_change_head->next->col_number = 3;
			expected_change_head->next->operation = 3;
			expected_change_head->next->data_type = DATA_DATE;
			expected_change_head->next->data = (char*) myMalloc(sizeof(char) * 32, NULL, &malloced_head, the_debug);
			if (expected_change_head->next->data == NULL) { if (the_debug == YES_DEBUG) printf("	ERROR in test_Driver_main() at line %d in %s\n", __LINE__, __FILE__); return -1; }
			strcpy(expected_change_head->next->data, "1/1/1900");
			
			expected_change_head->next->next = NULL;

			parsed_error_code = 0;
			if (test_Controller_parseUpdate(110, "UPDATE  ALC_BRANDS  SET  status  =  'TST_ACTIVE'\n  	,  	\nEFFECTIVE  =  '1/1/1900'	\n	;"
										   ,&expected_change_head, &parsed_error_code, &malloced_head, the_debug) != 0)
				result = -1;

			if (parsed_error_code == 0)
			{
				myFree((void**) &expected_change_head->next->data, NULL, &malloced_head, the_debug);
				myFree((void**) &expected_change_head->next, NULL, &malloced_head, the_debug);

				myFree((void**) &expected_change_head->data, NULL, &malloced_head, the_debug);
				myFree((void**) &expected_change_head, NULL, &malloced_head, the_debug);
			}

			if (malloced_head != NULL)
			{
				if (the_debug == YES_DEBUG)
					printf("	ERROR in test_Driver_main() at line %d in %s\n", __LINE__, __FILE__);
				return -3;
			}
		// END Test with id = 203

		// START Test with id = 204
			expected_change_head = (struct change_node_v2*) myMalloc(sizeof(struct change_node_v2), NULL, &malloced_head, the_debug);
			if (expected_change_head == NULL) { if (the_debug == YES_DEBUG) printf("	ERROR in test_Driver_main() at line %d in %s\n", __LINE__, __FILE__); return -1; }
			expected_change_head->col_number = 1;
			expected_change_head->operation = 3;
			expected_change_head->data_type = DATA_INT;
			expected_change_head->data = (char*) myMalloc(sizeof(char) * 32, NULL, &malloced_head, the_debug);
			if (expected_change_head->data == NULL) { if (the_debug == YES_DEBUG) printf("	ERROR in test_Driver_main() at line %d in %s\n", __LINE__, __FILE__); return -1; }
			strcpy(expected_change_head->data, "2");
			
			expected_change_head->next = (struct change_node_v2*) myMalloc(sizeof(struct change_node_v2), NULL, &malloced_head, the_debug);
			if (expected_change_head->next == NULL) { if (the_debug == YES_DEBUG) printf("	ERROR in test_Driver_main() at line %d in %s\n", __LINE__, __FILE__); return -1; }
			expected_change_head->next->col_number = 3;
			expected_change_head->next->operation = 3;
			expected_change_head->next->data_type = DATA_DATE;
			expected_change_head->next->data = (char*) myMalloc(sizeof(char) * 32, NULL, &malloced_head, the_debug);
			if (expected_change_head->next->data == NULL) { if (the_debug == YES_DEBUG) printf("	ERROR in test_Driver_main() at line %d in %s\n", __LINE__, __FILE__); return -1; }
			strcpy(expected_change_head->next->data, "1/1/1900");
			
			expected_change_head->next->next = NULL;

			parsed_error_code = 0;
			if (test_Controller_parseUpdate(111, "update alc_brands set CT-REGISTRATION-NUMBER = 2,EFFECTIVE = '1/1/1900' where BRAND-NAME = 'that';"
										   ,&expected_change_head, &parsed_error_code, &malloced_head, the_debug) != 0)
				result = -1;

			if (parsed_error_code == 0)
			{
				myFree((void**) &expected_change_head->next->data, NULL, &malloced_head, the_debug);
				myFree((void**) &expected_change_head->next, NULL, &malloced_head, the_debug);

				myFree((void**) &expected_change_head->data, NULL, &malloced_head, the_debug);
				myFree((void**) &expected_change_head, NULL, &malloced_head, the_debug);
			}

			if (malloced_head != NULL)
			{
				if (the_debug == YES_DEBUG)
					printf("	ERROR in test_Driver_main() at line %d in %s\n", __LINE__, __FILE__);
				return -3;
			}
		// END Test with id = 204

		// START Test with id = 205
			expected_change_head = NULL;

			parsed_error_code = 0;
			if (test_Controller_parseUpdate(112, "update alc_brands set BRAND-NAME = 2 , EFFECTIVE = '1/1/1900';"
										   ,&expected_change_head, &parsed_error_code, &malloced_head, the_debug) != 0)
				result = -1;

			if (malloced_head != NULL)
			{
				if (the_debug == YES_DEBUG)
					printf("	ERROR in test_Driver_main() at line %d in %s\n", __LINE__, __FILE__);
				return -3;
			}
		// END Test with id = 205
	// END test_Controller_parseUpdate
	
	// START test_Controller_parseDelete
		// START Test with id = 301
			if (test_Controller_parseDelete(113, "delete from alc_brands where BRAND-NAME = '860 INDA PALE ALE' or BRAND-NAME = '242 (NOBLE VINES) SAUVIGNON BLANC SAN BERNABE MONTEREY';", "alc_brands"
										   ,&malloced_head, the_debug) != 0)
				result = -1;

			if (malloced_head != NULL)
			{
				if (the_debug == YES_DEBUG)
					printf("	ERROR in test_Driver_main() at line %d in %s\n", __LINE__, __FILE__);
				return -3;
			}
		// END Test with id = 301

		// START Test with id = 302
			if (test_Controller_parseDelete(114, "   Delete	 \n	from	ALC_BRANDS 	 where this = 'that'  ;", ""
										   ,&malloced_head, the_debug) != 0)
				result = -1;

			if (malloced_head != NULL)
			{
				if (the_debug == YES_DEBUG)
					printf("	ERROR in test_Driver_main() at line %d in %s\n", __LINE__, __FILE__);
				return -3;
			}
		// END Test with id = 302

		// START Test with id = 303
			if (test_Controller_parseDelete(115, "   Delete\nFrom\n	ALC_BRANDS\nwhere;", "alc_brands"
										   ,&malloced_head, the_debug) != 0)
				result = -1;

			if (malloced_head != NULL)
			{
				if (the_debug == YES_DEBUG)
					printf("	ERROR in test_Driver_main() at line %d in %s\n", __LINE__, __FILE__);
				return -3;
			}
		// END Test with id = 303

		// START Test with id = 304
			if (test_Controller_parseDelete(116, "\nDelete\nfrom\n	ALC_BRANDS", "alc_brands"
										   ,&malloced_head, the_debug) != 0)
				result = -1;

			if (malloced_head != NULL)
			{
				if (the_debug == YES_DEBUG)
					printf("	ERROR in test_Driver_main() at line %d in %s\n", __LINE__, __FILE__);
				return -3;
			}
		// END Test with id = 304
		
		// START Test with id = 305
			if (test_Controller_parseDelete(117, "\ndelete\n\n	ALC_BRANDS", ""
										   ,&malloced_head, the_debug) != 0)
				result = -1;

			if (malloced_head != NULL)
			{
				if (the_debug == YES_DEBUG)
					printf("	ERROR in test_Driver_main() at line %d in %s\n", __LINE__, __FILE__);
				return -3;
			}
		// END Test with id = 305

		// START Test with id = 306
			if (test_Controller_parseDelete(118, "delete from ALC_BRANDS where BRAND-NAME = that", ""
										   ,&malloced_head, the_debug) != 0)
				result = -1;

			if (malloced_head != NULL)
			{
				if (the_debug == YES_DEBUG)
					printf("	ERROR in test_Driver_main() at line %d in %s\n", __LINE__, __FILE__);
				return -3;
			}
		// END Test with id = 306
	// END test_Controller_parseDelete

	// START test_Controller_parseInsert
		// START Test with id = 401
			parsed_error_code = 0;
			if (test_Controller_parseInsert(119, "Insert into alc_brands (col1, col2) values ('Hi', 'Hello');", NULL, "", &parsed_error_code
										   ,&malloced_head, the_debug) != 0)
				result = -1;

			if (malloced_head != NULL)
			{
				if (the_debug == YES_DEBUG)
					printf("	ERROR in test_Driver_main() at line %d in %s\n", __LINE__, __FILE__);
				return -3;
			}
		// END Test with id = 401

		// START Test with id = 402
			// START Malloc expected
				expected_change_head = (struct change_node_v2*) myMalloc(sizeof(struct change_node_v2), NULL, &malloced_head, the_debug);
				if (expected_change_head == NULL) { if (the_debug == YES_DEBUG) printf("	ERROR in test_Driver_main() at line %d in %s\n", __LINE__, __FILE__); return -1; }

				struct change_node_v2* cur_change = expected_change_head;

				cur_change->col_number = 0;
				cur_change->operation = 1;
				cur_change->data_type = DATA_STRING;
				cur_change->data = (char*) myMalloc(sizeof(char) * 200, NULL, &malloced_head, the_debug);
				if (cur_change->data == NULL) { if (the_debug == YES_DEBUG) printf("	ERROR in test_Driver_main() at line %d in %s\n", __LINE__, __FILE__); return -1; }
				strcpy(cur_change->data, "Hi");

				cur_change->next = (struct change_node_v2*) myMalloc(sizeof(struct change_node_v2), NULL, &malloced_head, the_debug);
				if (cur_change->next == NULL) { if (the_debug == YES_DEBUG) printf("	ERROR in test_Driver_main() at line %d in %s\n", __LINE__, __FILE__); return -1; }
				cur_change = cur_change->next;

				cur_change->col_number = 1;
				cur_change->operation = 1;
				cur_change->data_type = DATA_INT;
				cur_change->data = NULL;

				cur_change->next = (struct change_node_v2*) myMalloc(sizeof(struct change_node_v2), NULL, &malloced_head, the_debug);
				if (cur_change->next == NULL) { if (the_debug == YES_DEBUG) printf("	ERROR in test_Driver_main() at line %d in %s\n", __LINE__, __FILE__); return -1; }
				cur_change = cur_change->next;

				cur_change->col_number = 2;
				cur_change->operation = 1;
				cur_change->data_type = DATA_STRING;
				cur_change->data = (char*) myMalloc(sizeof(char) * 200, NULL, &malloced_head, the_debug);
				if (cur_change->data == NULL) { if (the_debug == YES_DEBUG) printf("	ERROR in test_Driver_main() at line %d in %s\n", __LINE__, __FILE__); return -1; }
				strcpy(cur_change->data, "Hello");

				cur_change->next = (struct change_node_v2*) myMalloc(sizeof(struct change_node_v2), NULL, &malloced_head, the_debug);
				if (cur_change->next == NULL) { if (the_debug == YES_DEBUG) printf("	ERROR in test_Driver_main() at line %d in %s\n", __LINE__, __FILE__); return -1; }
				cur_change = cur_change->next;

				cur_change->col_number = 3;
				cur_change->operation = 1;
				cur_change->data_type = DATA_DATE;
				cur_change->data = NULL;

				cur_change->next = (struct change_node_v2*) myMalloc(sizeof(struct change_node_v2), NULL, &malloced_head, the_debug);
				if (cur_change->next == NULL) { if (the_debug == YES_DEBUG) printf("	ERROR in test_Driver_main() at line %d in %s\n", __LINE__, __FILE__); return -1; }
				cur_change = cur_change->next;

				cur_change->col_number = 4;
				cur_change->operation = 1;
				cur_change->data_type = DATA_DATE;
				cur_change->data = NULL;

				cur_change->next = (struct change_node_v2*) myMalloc(sizeof(struct change_node_v2), NULL, &malloced_head, the_debug);
				if (cur_change->next == NULL) { if (the_debug == YES_DEBUG) printf("	ERROR in test_Driver_main() at line %d in %s\n", __LINE__, __FILE__); return -1; }
				cur_change = cur_change->next;

				cur_change->col_number = 5;
				cur_change->operation = 1;
				cur_change->data_type = DATA_STRING;
				cur_change->data = NULL;

				cur_change->next = (struct change_node_v2*) myMalloc(sizeof(struct change_node_v2), NULL, &malloced_head, the_debug);
				if (cur_change->next == NULL) { if (the_debug == YES_DEBUG) printf("	ERROR in test_Driver_main() at line %d in %s\n", __LINE__, __FILE__); return -1; }
				cur_change = cur_change->next;

				cur_change->col_number = 6;
				cur_change->operation = 1;
				cur_change->data_type = DATA_STRING;
				cur_change->data = NULL;

				cur_change->next = NULL;
			// END Malloc expected

			parsed_error_code = 0;
			if (test_Controller_parseInsert(120, "Insert into alc_brands (braND-name, STATUS) values ('Hi', 'Hello');"
										   ,&expected_change_head, "alc_brands", &parsed_error_code
										   ,&malloced_head, the_debug) != 0)
				result = -1;

			if (parsed_error_code == 0)
			{
				while (expected_change_head != NULL)
				{
					if (expected_change_head->data != NULL)
						myFree((void**) &expected_change_head->data, NULL, &malloced_head, the_debug);

					struct change_node_v2* temp = expected_change_head;
					expected_change_head = expected_change_head->next;

					myFree((void**) &temp, NULL, &malloced_head, the_debug);
				}
			}

			if (malloced_head != NULL)
			{
				if (the_debug == YES_DEBUG)
					printf("	ERROR in test_Driver_main() at line %d in %s\n", __LINE__, __FILE__);
				return -3;
			}
		// END Test with id = 402

		// START Test with id = 403
			parsed_error_code = 0;
			if (test_Controller_parseInsert(121, "Insert into alc_brands (braND-name, BRAND-NAME, STATUS) values ('Hi', 'Hello', 'There');"
										   ,NULL, "", &parsed_error_code
										   ,&malloced_head, the_debug) != 0)
				result = -1;

			if (malloced_head != NULL)
			{
				if (the_debug == YES_DEBUG)
					printf("	ERROR in test_Driver_main() at line %d in %s\n", __LINE__, __FILE__);
				return -3;
			}
		// END Test with id = 403

		// START Test with id = 404
			parsed_error_code = 0;
			if (test_Controller_parseInsert(122, "Insert into alc_brands (braND-name BRAND-NAME, STATUS) values ('Hi', 'Hello');"
										   ,NULL, "", &parsed_error_code
										   ,&malloced_head, the_debug) != 0)
				result = -1;

			if (malloced_head != NULL)
			{
				if (the_debug == YES_DEBUG)
					printf("	ERROR in test_Driver_main() at line %d in %s\n", __LINE__, __FILE__);
				return -3;
			}
		// END Test with id = 404

		// START Test with id = 405
			parsed_error_code = 0;
			if (test_Controller_parseInsert(123, "Insert into alc_brands (braND-name, STATUS) values ('Hi', 'Hello', 'There');"
										   ,NULL, "", &parsed_error_code
										   ,&malloced_head, the_debug) != 0)
				result = -1;

			if (malloced_head != NULL)
			{
				if (the_debug == YES_DEBUG)
					printf("	ERROR in test_Driver_main() at line %d in %s\n", __LINE__, __FILE__);
				return -3;
			}
		// END Test with id = 405

		// START Test with id = 406
			// START Malloc expected
				expected_change_head = (struct change_node_v2*) myMalloc(sizeof(struct change_node_v2), NULL, &malloced_head, the_debug);
				if (expected_change_head == NULL) { if (the_debug == YES_DEBUG) printf("	ERROR in test_Driver_main() at line %d in %s\n", __LINE__, __FILE__); return -1; }

				cur_change = expected_change_head;

				cur_change->col_number = 0;
				cur_change->operation = 1;
				cur_change->data_type = DATA_STRING;
				cur_change->data = (char*) myMalloc(sizeof(char) * 200, NULL, &malloced_head, the_debug);
				if (cur_change->data == NULL) { if (the_debug == YES_DEBUG) printf("	ERROR in test_Driver_main() at line %d in %s\n", __LINE__, __FILE__); return -1; }
				strcpy(cur_change->data, "1");

				cur_change->next = (struct change_node_v2*) myMalloc(sizeof(struct change_node_v2), NULL, &malloced_head, the_debug);
				if (cur_change->next == NULL) { if (the_debug == YES_DEBUG) printf("	ERROR in test_Driver_main() at line %d in %s\n", __LINE__, __FILE__); return -1; }
				cur_change = cur_change->next;

				cur_change->col_number = 1;
				cur_change->operation = 1;
				cur_change->data_type = DATA_INT;
				cur_change->data = (char*) myMalloc(sizeof(char) * 200, NULL, &malloced_head, the_debug);
				if (cur_change->data == NULL) { if (the_debug == YES_DEBUG) printf("	ERROR in test_Driver_main() at line %d in %s\n", __LINE__, __FILE__); return -1; }
				strcpy(cur_change->data, "1");

				cur_change->next = (struct change_node_v2*) myMalloc(sizeof(struct change_node_v2), NULL, &malloced_head, the_debug);
				if (cur_change->next == NULL) { if (the_debug == YES_DEBUG) printf("	ERROR in test_Driver_main() at line %d in %s\n", __LINE__, __FILE__); return -1; }
				cur_change = cur_change->next;

				cur_change->col_number = 2;
				cur_change->operation = 1;
				cur_change->data_type = DATA_STRING;
				cur_change->data = (char*) myMalloc(sizeof(char) * 200, NULL, &malloced_head, the_debug);
				if (cur_change->data == NULL) { if (the_debug == YES_DEBUG) printf("	ERROR in test_Driver_main() at line %d in %s\n", __LINE__, __FILE__); return -1; }
				strcpy(cur_change->data, "1");

				cur_change->next = (struct change_node_v2*) myMalloc(sizeof(struct change_node_v2), NULL, &malloced_head, the_debug);
				if (cur_change->next == NULL) { if (the_debug == YES_DEBUG) printf("	ERROR in test_Driver_main() at line %d in %s\n", __LINE__, __FILE__); return -1; }
				cur_change = cur_change->next;

				cur_change->col_number = 3;
				cur_change->operation = 1;
				cur_change->data_type = DATA_DATE;
				cur_change->data = (char*) myMalloc(sizeof(char) * 200, NULL, &malloced_head, the_debug);
				if (cur_change->data == NULL) { if (the_debug == YES_DEBUG) printf("	ERROR in test_Driver_main() at line %d in %s\n", __LINE__, __FILE__); return -1; }
				strcpy(cur_change->data, "1/1/1900");

				cur_change->next = (struct change_node_v2*) myMalloc(sizeof(struct change_node_v2), NULL, &malloced_head, the_debug);
				if (cur_change->next == NULL) { if (the_debug == YES_DEBUG) printf("	ERROR in test_Driver_main() at line %d in %s\n", __LINE__, __FILE__); return -1; }
				cur_change = cur_change->next;

				cur_change->col_number = 4;
				cur_change->operation = 1;
				cur_change->data_type = DATA_DATE;
				cur_change->data = (char*) myMalloc(sizeof(char) * 200, NULL, &malloced_head, the_debug);
				if (cur_change->data == NULL) { if (the_debug == YES_DEBUG) printf("	ERROR in test_Driver_main() at line %d in %s\n", __LINE__, __FILE__); return -1; }
				strcpy(cur_change->data, "1/1/1900");

				cur_change->next = (struct change_node_v2*) myMalloc(sizeof(struct change_node_v2), NULL, &malloced_head, the_debug);
				if (cur_change->next == NULL) { if (the_debug == YES_DEBUG) printf("	ERROR in test_Driver_main() at line %d in %s\n", __LINE__, __FILE__); return -1; }
				cur_change = cur_change->next;

				cur_change->col_number = 5;
				cur_change->operation = 1;
				cur_change->data_type = DATA_STRING;
				cur_change->data = (char*) myMalloc(sizeof(char) * 200, NULL, &malloced_head, the_debug);
				if (cur_change->data == NULL) { if (the_debug == YES_DEBUG) printf("	ERROR in test_Driver_main() at line %d in %s\n", __LINE__, __FILE__); return -1; }
				strcpy(cur_change->data, "1");

				cur_change->next = (struct change_node_v2*) myMalloc(sizeof(struct change_node_v2), NULL, &malloced_head, the_debug);
				if (cur_change->next == NULL) { if (the_debug == YES_DEBUG) printf("	ERROR in test_Driver_main() at line %d in %s\n", __LINE__, __FILE__); return -1; }
				cur_change = cur_change->next;

				cur_change->col_number = 6;
				cur_change->operation = 1;
				cur_change->data_type = DATA_STRING;
				cur_change->data = (char*) myMalloc(sizeof(char) * 200, NULL, &malloced_head, the_debug);
				if (cur_change->data == NULL) { if (the_debug == YES_DEBUG) printf("	ERROR in test_Driver_main() at line %d in %s\n", __LINE__, __FILE__); return -1; }
				strcpy(cur_change->data, "1");

				cur_change->next = NULL;
			// END Malloc expected

			parsed_error_code = 0;
			if (test_Controller_parseInsert(124, "Insert into alc_brands (BRAND-NAME,CT-REGISTRATION-NUMBER,STATUS,EFFECTIVE,EXPIRATION,OUT-OF-STATE-SHIPPER,SUPERVISOR-CREDENTIAL) values ('1', 1, '1','1/1/1900','1/1/1900','1','1');"
										   ,&expected_change_head, "alc_brands", &parsed_error_code
										   ,&malloced_head, the_debug) != 0)
				result = -1;

			if (parsed_error_code == 0)
			{
				while (expected_change_head != NULL)
				{
					if (expected_change_head->data != NULL)
						myFree((void**) &expected_change_head->data, NULL, &malloced_head, the_debug);

					struct change_node_v2* temp = expected_change_head;
					expected_change_head = expected_change_head->next;

					myFree((void**) &temp, NULL, &malloced_head, the_debug);
				}
			}
		// END Test with id = 406

		// START Test with id = 407
			parsed_error_code = 0;
			if (test_Controller_parseInsert(125, "Insert into alc_brands (BRAND-NAME, CT-REGISTRATION-NUMBER) values ('Hi', 'Hello');", NULL, "", &parsed_error_code
										   ,&malloced_head, the_debug) != 0)
				result = -1;

			if (malloced_head != NULL)
			{
				if (the_debug == YES_DEBUG)
					printf("	ERROR in test_Driver_main() at line %d in %s\n", __LINE__, __FILE__);
				return -3;
			}
		// END Test with id = 407

		// START Test with id = 408
			parsed_error_code = 0;
			if (test_Controller_parseInsert(126, "Insert into alc_brands (BRAND-NAME, CT-REGISTRATION-NUMBER) values (1, 1);", NULL, "", &parsed_error_code
										   ,&malloced_head, the_debug) != 0)
				result = -1;

			if (malloced_head != NULL)
			{
				if (the_debug == YES_DEBUG)
					printf("	ERROR in test_Driver_main() at line %d in %s\n", __LINE__, __FILE__);
				return -3;
			}
		// END Test with id = 408

		// START Test with id = 409
			parsed_error_code = 0;
			if (test_Controller_parseInsert(127, "Insert into alc_brands (BRAND-NAME, EFFECTIVE) values ('Hi', 1);", NULL, "", &parsed_error_code
										   ,&malloced_head, the_debug) != 0)
				result = -1;

			if (malloced_head != NULL)
			{
				if (the_debug == YES_DEBUG)
					printf("	ERROR in test_Driver_main() at line %d in %s\n", __LINE__, __FILE__);
				return -3;
			}
		// END Test with id = 409
	// END test_Controller_parseInsert


	// START test_Driver_findValidRowsGivenWhere
		// START Test with id = 1
			struct ListNode* valid_rows_head = NULL;
			struct ListNode* valid_rows_tail = NULL;

			for (int i=0; i<getTablesHead()->table_cols_head->num_rows; i++)
			{
				if (addListNode(&valid_rows_head, &valid_rows_tail, i, ADDLISTNODE_TAIL, NULL, &malloced_head, the_debug) != 0)
				{
					if (the_debug == YES_DEBUG)
						printf("	ERROR in test_Driver_main() at line %d in %s\n", __LINE__, __FILE__);
					return -2;
				}
			}

			int_8 num_rows_in_result = 0;
			if (test_Driver_findValidRowsGivenWhere(1, valid_rows_head,""
												   ,getTablesHead(), NULL
												   ,NULL, &num_rows_in_result, 0
												   ,&malloced_head, the_debug) != 0)
				result = -1;

			int freed = freeListNodes(&valid_rows_head, NULL, &malloced_head, the_debug);

			if (malloced_head != NULL)
			{
				if (the_debug == YES_DEBUG)
					printf("	ERROR in test_Driver_main() at line %d in %s\n", __LINE__, __FILE__);
				myFreeAllError(&malloced_head, the_debug);
				result = -1;
			}
		// END Test with id = 1
		
		// START Test with id = 2
			valid_rows_head = NULL;
			valid_rows_tail = NULL;

			if (addListNode(&valid_rows_head, &valid_rows_tail, 12, ADDLISTNODE_TAIL, NULL, &malloced_head, the_debug) != 0)
			{
				if (the_debug == YES_DEBUG)
					printf("	ERROR in test_Driver_main() at line %d in %s\n", __LINE__, __FILE__);
				return -2;
			}

			num_rows_in_result = 0;
			if (test_Driver_findValidRowsGivenWhere(2, valid_rows_head, "where BRAND-NAME = 'VIZZY BLACK CHERRY LIME';"
												   ,getTablesHead(), NULL
												   ,NULL, &num_rows_in_result, 0
												   ,&malloced_head, the_debug) != 0)
				result = -1;

			freed = freeListNodes(&valid_rows_head, NULL, &malloced_head, the_debug);

			if (malloced_head != NULL)
			{
				if (the_debug == YES_DEBUG)
					printf("	ERROR in test_Driver_main() at line %d in %s\n", __LINE__, __FILE__);
				myFreeAllError(&malloced_head, the_debug);
				result = -1;
			}
		// END Test with id = 2
		
		// START Test with id = 3
			valid_rows_head = NULL;
			valid_rows_tail = NULL;

			if (addListNode(&valid_rows_head, &valid_rows_tail, 6, ADDLISTNODE_TAIL, NULL, &malloced_head, the_debug) != 0)
			{
				if (the_debug == YES_DEBUG)
					printf("	ERROR in test_Driver_main() at line %d in %s\n", __LINE__, __FILE__);
				return -2;
			}

			num_rows_in_result = 0;
			if (test_Driver_findValidRowsGivenWhere(3, valid_rows_head, "where CT-REGISTRATION-NUMBER = 152525;"
												   ,getTablesHead(), NULL
												   ,NULL, &num_rows_in_result, 0
												   ,&malloced_head, the_debug) != 0)
				result = -1;

			freed = freeListNodes(&valid_rows_head, NULL, &malloced_head, the_debug);

			if (malloced_head != NULL)
			{
				if (the_debug == YES_DEBUG)
					printf("	ERROR in test_Driver_main() at line %d in %s\n", __LINE__, __FILE__);
				myFreeAllError(&malloced_head, the_debug);
				result = -1;
			}
		// END Test with id = 3
		
		// START Test with id = 4
			valid_rows_head = NULL;
			valid_rows_tail = NULL;

			addListNode(&valid_rows_head, &valid_rows_tail, 5, ADDLISTNODE_TAIL, NULL, &malloced_head, the_debug);
			addListNode(&valid_rows_head, &valid_rows_tail, 56, ADDLISTNODE_TAIL, NULL, &malloced_head, the_debug);
			addListNode(&valid_rows_head, &valid_rows_tail, 58, ADDLISTNODE_TAIL, NULL, &malloced_head, the_debug);
			addListNode(&valid_rows_head, &valid_rows_tail, 343, ADDLISTNODE_TAIL, NULL, &malloced_head, the_debug);
			addListNode(&valid_rows_head, &valid_rows_tail, 521, ADDLISTNODE_TAIL, NULL, &malloced_head, the_debug);
			addListNode(&valid_rows_head, &valid_rows_tail, 579, ADDLISTNODE_TAIL, NULL, &malloced_head, the_debug);
			addListNode(&valid_rows_head, &valid_rows_tail, 805, ADDLISTNODE_TAIL, NULL, &malloced_head, the_debug);
			addListNode(&valid_rows_head, &valid_rows_tail, 945, ADDLISTNODE_TAIL, NULL, &malloced_head, the_debug);

			num_rows_in_result = 0;
			if (test_Driver_findValidRowsGivenWhere(4, valid_rows_head, "where EXPIRATION = '1/31/2025';"
												   ,getTablesHead(), NULL
												   ,NULL, &num_rows_in_result, 0
												   ,&malloced_head, the_debug) != 0)
				result = -1;

			freed = freeListNodes(&valid_rows_head, NULL, &malloced_head, the_debug);

			if (malloced_head != NULL)
			{
				if (the_debug == YES_DEBUG)
					printf("	ERROR in test_Driver_main() at line %d in %s\n", __LINE__, __FILE__);
				myFreeAllError(&malloced_head, the_debug);
				result = -1;
			}
		// END Test with id = 4
		
		// START Test with id = 5
			valid_rows_head = NULL;
			valid_rows_tail = NULL;

			if (addListNode(&valid_rows_head, &valid_rows_tail, 12, ADDLISTNODE_TAIL, NULL, &malloced_head, the_debug) != 0)
			{
				if (the_debug == YES_DEBUG)
					printf("	ERROR in test_Driver_main() at line %d in %s\n", __LINE__, __FILE__);
				return -2;
			}

			num_rows_in_result = 0;
			if (test_Driver_findValidRowsGivenWhere(5, valid_rows_head, "where BRAND-NAME = 'VIZZY BLACK CHERRY LIME' and OUT-OF-STATE-SHIPPER = 'MOLSON COORS BEVERAGE COMPANY USA LLC';"
												   ,getTablesHead(), NULL
												   ,NULL, &num_rows_in_result, 0
												   ,&malloced_head, the_debug) != 0)
				result = -1;

			freed = freeListNodes(&valid_rows_head, NULL, &malloced_head, the_debug);

			if (malloced_head != NULL)
			{
				if (the_debug == YES_DEBUG)
					printf("	ERROR in test_Driver_main() at line %d in %s\n", __LINE__, __FILE__);
				myFreeAllError(&malloced_head, the_debug);
				result = -1;
			}
		// END Test with id = 5
		
		// START Test with id = 6
			valid_rows_head = NULL;
			valid_rows_tail = NULL;

			addListNode(&valid_rows_head, &valid_rows_tail, 12, ADDLISTNODE_TAIL, NULL, &malloced_head, the_debug);
			addListNode(&valid_rows_head, &valid_rows_tail, 23, ADDLISTNODE_TAIL, NULL, &malloced_head, the_debug);

			num_rows_in_result = 0;
			if (test_Driver_findValidRowsGivenWhere(6, valid_rows_head, "where BRAND-NAME = 'VIZZY BLACK CHERRY LIME' or BRAND-NAME = 'CRUZAN ISLAND SPICED RUM';"
												   ,getTablesHead(), NULL
												   ,NULL, &num_rows_in_result, 0
												   ,&malloced_head, the_debug) != 0)
				result = -1;

			freed = freeListNodes(&valid_rows_head, NULL, &malloced_head, the_debug);

			if (malloced_head != NULL)
			{
				if (the_debug == YES_DEBUG)
					printf("	ERROR in test_Driver_main() at line %d in %s\n", __LINE__, __FILE__);
				myFreeAllError(&malloced_head, the_debug);
				result = -1;
			}
		// END Test with id = 6
		
		// START Test with id = 7
			valid_rows_head = NULL;
			valid_rows_tail = NULL;

			addListNode(&valid_rows_head, &valid_rows_tail, 5, ADDLISTNODE_TAIL, NULL, &malloced_head, the_debug);
			addListNode(&valid_rows_head, &valid_rows_tail, 12, ADDLISTNODE_TAIL, NULL, &malloced_head, the_debug);
			addListNode(&valid_rows_head, &valid_rows_tail, 23, ADDLISTNODE_TAIL, NULL, &malloced_head, the_debug);
			addListNode(&valid_rows_head, &valid_rows_tail, 56, ADDLISTNODE_TAIL, NULL, &malloced_head, the_debug);
			addListNode(&valid_rows_head, &valid_rows_tail, 58, ADDLISTNODE_TAIL, NULL, &malloced_head, the_debug);
			addListNode(&valid_rows_head, &valid_rows_tail, 343, ADDLISTNODE_TAIL, NULL, &malloced_head, the_debug);
			addListNode(&valid_rows_head, &valid_rows_tail, 521, ADDLISTNODE_TAIL, NULL, &malloced_head, the_debug);
			addListNode(&valid_rows_head, &valid_rows_tail, 579, ADDLISTNODE_TAIL, NULL, &malloced_head, the_debug);
			addListNode(&valid_rows_head, &valid_rows_tail, 805, ADDLISTNODE_TAIL, NULL, &malloced_head, the_debug);
			addListNode(&valid_rows_head, &valid_rows_tail, 945, ADDLISTNODE_TAIL, NULL, &malloced_head, the_debug);

			num_rows_in_result = 0;
			if (test_Driver_findValidRowsGivenWhere(7, valid_rows_head, "where BRAND-NAME = 'VIZZY BLACK CHERRY LIME' or BRAND-NAME = 'CRUZAN ISLAND SPICED RUM' and CT-REGISTRATION-NUMBER = 169876 or EXPIRATION = '1/31/2025';"
												   ,getTablesHead(), NULL
												   ,NULL, &num_rows_in_result, 0
												   ,&malloced_head, the_debug) != 0)
				result = -1;

			freed = freeListNodes(&valid_rows_head, NULL, &malloced_head, the_debug);

			if (malloced_head != NULL)
			{
				if (the_debug == YES_DEBUG)
					printf("	ERROR in test_Driver_main() at line %d in %s\n", __LINE__, __FILE__);
				myFreeAllError(&malloced_head, the_debug);
				result = -1;
			}
		// END Test with id = 7
		
		// START Test with id = 8
			valid_rows_head = NULL;
			valid_rows_tail = NULL;

			addListNode(&valid_rows_head, &valid_rows_tail, 5, ADDLISTNODE_TAIL, NULL, &malloced_head, the_debug);
			addListNode(&valid_rows_head, &valid_rows_tail, 23, ADDLISTNODE_TAIL, NULL, &malloced_head, the_debug);

			num_rows_in_result = 0;
			if (test_Driver_findValidRowsGivenWhere(8, valid_rows_head, "where BRAND-NAME = 'CRUZAN ISLAND SPICED RUM' and CT-REGISTRATION-NUMBER = 169876 or EXPIRATION = '1/31/2025' and BRAND-NAME = 'BABAROSA MOSCATO D''ASTI';"
												   ,getTablesHead(), NULL
												   ,NULL, &num_rows_in_result, 0
												   ,&malloced_head, the_debug) != 0)
				result = -1;

			freed = freeListNodes(&valid_rows_head, NULL, &malloced_head, the_debug);

			if (malloced_head != NULL)
			{
				if (the_debug == YES_DEBUG)
					printf("	ERROR in test_Driver_main() at line %d in %s\n", __LINE__, __FILE__);
				myFreeAllError(&malloced_head, the_debug);
				result = -1;
			}
		// END Test with id = 8
		
		// START Test with id = 9
			valid_rows_head = NULL;
			valid_rows_tail = NULL;

			for (int i=0; i<getTablesHead()->table_cols_head->num_rows; i++)
			{
				if (i != 23)
				{
					if (addListNode(&valid_rows_head, &valid_rows_tail, i, ADDLISTNODE_TAIL, NULL, &malloced_head, the_debug) != 0)
					{
						if (the_debug == YES_DEBUG)
							printf("	ERROR in test_Driver_main() at line %d in %s\n", __LINE__, __FILE__);
						return -2;
					}
				}
			}

			num_rows_in_result = 0;
			if (test_Driver_findValidRowsGivenWhere(9, valid_rows_head, "where BRAND-NAME <> 'CRUZAN ISLAND SPICED RUM';"
												   ,getTablesHead(), NULL
												   ,NULL, &num_rows_in_result, 0
												   ,&malloced_head, the_debug) != 0)
				result = -1;

			freed = freeListNodes(&valid_rows_head, NULL, &malloced_head, the_debug);

			if (malloced_head != NULL)
			{
				if (the_debug == YES_DEBUG)
					printf("	ERROR in test_Driver_main() at line %d in %s\n", __LINE__, __FILE__);
				myFreeAllError(&malloced_head, the_debug);
				result = -1;
			}
		// END Test with id = 9
		
		// START Test with id = 10
			valid_rows_head = NULL;
			valid_rows_tail = NULL;

			for (int i=0; i<getTablesHead()->table_cols_head->num_rows; i++)
			{
				if (addListNode(&valid_rows_head, &valid_rows_tail, i, ADDLISTNODE_TAIL, NULL, &malloced_head, the_debug) != 0)
				{
					if (the_debug == YES_DEBUG)
						printf("	ERROR in test_Driver_main() at line %d in %s\n", __LINE__, __FILE__);
					return -2;
				}
			}

			num_rows_in_result = 0;
			if (test_Driver_findValidRowsGivenWhere(10, valid_rows_head, "where BRAND-NAME <> 'CRUZAN ISLAND SPICED RUM' or BRAND-NAME <> 'TEST 1';"
												   ,getTablesHead(), NULL
												   ,NULL, &num_rows_in_result, 0
												   ,&malloced_head, the_debug) != 0)
				result = -1;

			freed = freeListNodes(&valid_rows_head, NULL, &malloced_head, the_debug);

			if (malloced_head != NULL)
			{
				if (the_debug == YES_DEBUG)
					printf("	ERROR in test_Driver_main() at line %d in %s\n", __LINE__, __FILE__);
				myFreeAllError(&malloced_head, the_debug);
				result = -1;
			}
		// END Test with id = 10
		
		// START Test with id = 11
			valid_rows_head = NULL;
			valid_rows_tail = NULL;

			for (int i=0; i<getTablesHead()->table_cols_head->num_rows; i++)
			{
				if (i != 23 && i != 12)
				{
					if (addListNode(&valid_rows_head, &valid_rows_tail, i, ADDLISTNODE_TAIL, NULL, &malloced_head, the_debug) != 0)
					{
						if (the_debug == YES_DEBUG)
							printf("	ERROR in test_Driver_main() at line %d in %s\n", __LINE__, __FILE__);
						return -2;
					}
				}
			}

			num_rows_in_result = 0;
			if (test_Driver_findValidRowsGivenWhere(11, valid_rows_head, "where BRAND-NAME <> 'CRUZAN ISLAND SPICED RUM' and BRAND-NAME <> 'VIZZY BLACK CHERRY LIME';"
												   ,getTablesHead(), NULL
												   ,NULL, &num_rows_in_result, 0
												   ,&malloced_head, the_debug) != 0)
				result = -1;

			freed = freeListNodes(&valid_rows_head, NULL, &malloced_head, the_debug);

			if (malloced_head != NULL)
			{
				if (the_debug == YES_DEBUG)
					printf("	ERROR in test_Driver_main() at line %d in %s\n", __LINE__, __FILE__);
				myFreeAllError(&malloced_head, the_debug);
				result = -1;
			}
		// END Test with id = 11
		
		// START Test with id = 12
			valid_rows_head = NULL;
			valid_rows_tail = NULL;

			for (int i=0; i<getTablesHead()->table_cols_head->num_rows; i++)
			{
				if (addListNode(&valid_rows_head, &valid_rows_tail, i, ADDLISTNODE_TAIL, NULL, &malloced_head, the_debug) != 0)
				{
					if (the_debug == YES_DEBUG)
						printf("	ERROR in test_Driver_main() at line %d in %s\n", __LINE__, __FILE__);
					return -2;
				}
			}

			num_rows_in_result = 0;
			if (test_Driver_findValidRowsGivenWhere(12, valid_rows_head, "where BRAND-NAME <> 'CRUZAN ISLAND SPICED RUM' and CT-REGISTRATION-NUMBER <> 169876 or EXPIRATION <> '1/31/2025' and BRAND-NAME <> 'BABAROSA MOSCATO D''ASTI';"
												   ,getTablesHead(), NULL
												   ,NULL, &num_rows_in_result, 0
												   ,&malloced_head, the_debug) != 0)
				result = -1;

			freed = freeListNodes(&valid_rows_head, NULL, &malloced_head, the_debug);

			if (malloced_head != NULL)
			{
				if (the_debug == YES_DEBUG)
					printf("	ERROR in test_Driver_main() at line %d in %s\n", __LINE__, __FILE__);
				myFreeAllError(&malloced_head, the_debug);
				result = -1;
			}
		// END Test with id = 12
		
		// START Test with id = 13
			valid_rows_head = NULL;
			valid_rows_tail = NULL;

			num_rows_in_result = 0;
			if (test_Driver_findValidRowsGivenWhere(13, valid_rows_head, "where BRAND-NAME = 'Is test bro';"
												   ,getTablesHead(), NULL
												   ,NULL, &num_rows_in_result, 0
												   ,&malloced_head, the_debug) != 0)
				result = -1;

			freed = freeListNodes(&valid_rows_head, NULL, &malloced_head, the_debug);

			if (malloced_head != NULL)
			{
				if (the_debug == YES_DEBUG)
					printf("	ERROR in test_Driver_main() at line %d in %s\n", __LINE__, __FILE__);
				myFreeAllError(&malloced_head, the_debug);
				result = -1;
			}
		// END Test with id = 13
	// END test_Driver_findValidRowsGivenWhere

	// START test_Driver_updateRows
		// START Test with id = 14
			if (test_Driver_updateRows(14, "DB_Files_2_Test_Versions\\Update_Test_1.csv"
									  ,"update alc_brands set STATUS = 'TST_ACTIVE' where EXPIRATION = '1/31/2025';"
									  ,&malloced_head, the_debug) != 0)
				result = -1;

			if (malloced_head != NULL)
			{
				if (the_debug == YES_DEBUG)
					printf("	ERROR in test_Driver_main() at line %d in %s\n", __LINE__, __FILE__);
				//myFreeAllError(&malloced_head, the_debug);
				result = -1;
			}
		// END Test with id = 14

		// START Test with id = 15
			if (test_Driver_updateRows(15, "DB_Files_2_Test_Versions\\Update_Test_2_v2.csv"
									  ,"update alc_brands set STATUS = 'SOMETHING' where EXPIRATION = '1/1/1900';"
							  		  ,&malloced_head, the_debug) != 0)
				result = -1;

			if (malloced_head != NULL)
			{
				if (the_debug == YES_DEBUG)
					printf("	ERROR in test_Driver_main() at line %d in %s\n", __LINE__, __FILE__);
				myFreeAllError(&malloced_head, the_debug);
				result = -1;
			}
		// END Test with id = 15

		// START Test with id = 16
			if (test_Driver_updateRows(16, "DB_Files_2_Test_Versions\\Update_Test_3_v2.csv"
									  ,"update alc_brands set STATUS = 'VRY_ACTIVE' where STATUS = 'ACTIVE';"
							  		  ,&malloced_head, the_debug) != 0)
				result = -1;

			if (malloced_head != NULL)
			{
				if (the_debug == YES_DEBUG)
					printf("	ERROR in test_Driver_main() at line %d in %s\n", __LINE__, __FILE__);
				myFreeAllError(&malloced_head, the_debug);
				result = -1;
			}
		// END Test with id = 16

		// START Test with id = 17
			if (test_Driver_updateRows(17, "DB_Files_2_Test_Versions\\Update_Test_4.csv"
									  ,"update alc_brands set STATUS = 'EXT_ACTIVE', CT-REGISTRATION-NUMBER = 1 where STATUS = 'VRY_ACTIVE';"
							  		  ,&malloced_head, the_debug) != 0)
				result = -1;

			if (malloced_head != NULL)
			{
				if (the_debug == YES_DEBUG)
					printf("	ERROR in test_Driver_main() at line %d in %s\n", __LINE__, __FILE__);
				myFreeAllError(&malloced_head, the_debug);
				result = -1;
			}
		// END Test with id = 17

		// START Test with id = 18
			if (test_Driver_updateRows(18, "DB_Files_2_Test_Versions\\Update_Test_5.csv"
									  ,"update alc_brands set CT-REGISTRATION-NUMBER = 2, EFFECTIVE = '1/1/1900' where CT-REGISTRATION-NUMBER = 1;"
							  		  ,&malloced_head, the_debug) != 0)
				result = -1;

			if (malloced_head != NULL)
			{
				if (the_debug == YES_DEBUG)
					printf("	ERROR in test_Driver_main() at line %d in %s\n", __LINE__, __FILE__);
				myFreeAllError(&malloced_head, the_debug);
				result = -1;
			}
		// END Test with id = 18

		// START Test with id = 19
			if (test_Driver_updateRows(19, "DB_Files_2_Test_Versions\\Update_Test_6.csv"
									  ,"update alc_brands set CT-REGISTRATION-NUMBER = 3, EXPIRATION = '12/12/1901', SUPERVISOR-CREDENTIAL = 'A_CREDENTIAL';"
							  		  ,&malloced_head, the_debug) != 0)
				result = -1;

			if (malloced_head != NULL)
			{
				if (the_debug == YES_DEBUG)
					printf("	ERROR in test_Driver_main() at line %d in %s\n", __LINE__, __FILE__);
				myFreeAllError(&malloced_head, the_debug);
				result = -1;
			}
		// END Test with id = 19

		// START Test with id = 20
			if (test_Driver_updateRows(20, "DB_Files_2_Test_Versions\\Update_Test_7.csv"
									  ,"update alc_brands set CT-REGISTRATION-NUMBER = -1;"
							  		  ,&malloced_head, the_debug) != 0)
				result = -1;

			if (malloced_head != NULL)
			{
				if (the_debug == YES_DEBUG)
					printf("	ERROR in test_Driver_main() at line %d in %s\n", __LINE__, __FILE__);
				myFreeAllError(&malloced_head, the_debug);
				result = -1;
			}
		// END Test with id = 20
	// END test_Driver_updateRows

	// START test_Driver_deleteRows
		// START Test with id = 21
			if (test_Driver_deleteRows(21, "delete from alc_brands where BRAND-NAME = 'IDK_BUT_SHOULD_BE_NO_ROWS';"
									  ,"DB_Files_2_Test_Versions\\Delete_Test_1.csv", 1000, 0
							  		  ,&malloced_head, the_debug) != 0)
				result = -1;

			if (malloced_head != NULL)
			{
				if (the_debug == YES_DEBUG)
					printf("	ERROR in test_Driver_main() at line %d in %s\n", __LINE__, __FILE__);
				myFreeAllError(&malloced_head, the_debug);
				result = -1;
			}
		// END Test with id = 21
		
		// START Test with id = 22
			if (test_Driver_deleteRows(22, "delete from alc_brands where BRAND-NAME = 'VIZZY BLACK CHERRY LIME';"
									  ,"DB_Files_2_Test_Versions\\Delete_Test_2.csv", 1000, 1
							  		  ,&malloced_head, the_debug) != 0)
				result = -1;

			if (malloced_head != NULL)
			{
				if (the_debug == YES_DEBUG)
					printf("	ERROR in test_Driver_main() at line %d in %s\n", __LINE__, __FILE__);
				myFreeAllError(&malloced_head, the_debug);
				result = -1;
			}
		// END Test with id = 22

		// START Test with id = 23
			if (test_Driver_deleteRows(23, "delete from alc_brands where BRAND-NAME = '860 INDA PALE ALE' or BRAND-NAME = '242 (NOBLE VINES) SAUVIGNON BLANC SAN BERNABE MONTEREY';"
									  ,"DB_Files_2_Test_Versions\\Delete_Test_3.csv", 1000, 3
							  		  ,&malloced_head, the_debug) != 0)
				result = -1;

			if (malloced_head != NULL)
			{
				if (the_debug == YES_DEBUG)
					printf("	ERROR in test_Driver_main() at line %d in %s\n", __LINE__, __FILE__);
				myFreeAllError(&malloced_head, the_debug);
				result = -1;
			}
		// END Test with id = 23

		// START Test with id = 24
			if (test_Driver_deleteRows(24, "delete from alc_brands where BRAND-NAME <> 'FITAPRETA TINTO' and BRAND-NAME <> 'TWISTED TEA PARTY POUCH' and BRAND-NAME <> 'EAT BEER SIP KILLER';"
									  ,"DB_Files_2_Test_Versions\\Delete_Test_4.csv", 1000, 997
							  		  ,&malloced_head, the_debug) != 0)
				result = -1;

			if (malloced_head != NULL)
			{
				if (the_debug == YES_DEBUG)
					printf("	ERROR in test_Driver_main() at line %d in %s\n", __LINE__, __FILE__);
				myFreeAllError(&malloced_head, the_debug);
				result = -1;
			}
		// END Test with id = 24

		// START Test with id = 25
			if (test_Driver_deleteRows(25, "delete from alc_brands;"
									  ,"DB_Files_2_Test_Versions\\Delete_Test_5.csv", 0, 0
							  		  ,&malloced_head, the_debug) != 0)
				result = -1;

			if (malloced_head != NULL)
			{
				if (the_debug == YES_DEBUG)
					printf("	ERROR in test_Driver_main() at line %d in %s\n", __LINE__, __FILE__);
				myFreeAllError(&malloced_head, the_debug);
				result = -1;
			}
		// END Test with id = 25
	// END test_Driver_deleteRows

	// START test_Driver_insertRows
		// START Test with id = 26
			if (test_Driver_insertRows(26, "Insert into alc_brands (braND-name, STATUS) values ('Hi', 'Hello');"
										  ,"DB_Files_2_Test_Versions\\Insert_Test_1.csv", 1, 0
								  		  ,&malloced_head, the_debug) != 0)
				result = -1;

			if (malloced_head != NULL)
			{
				if (the_debug == YES_DEBUG)
					printf("	ERROR in test_Driver_main() at line %d in %s\n", __LINE__, __FILE__);
				myFreeAllError(&malloced_head, the_debug);
				result = -1;
			}
		// END Test with id = 26

		// START Test with id = 27
			if (test_Driver_insertRows(27, "Insert into alc_brands (BRAND-NAME,CT-REGISTRATION-NUMBER,STATUS,EFFECTIVE,EXPIRATION,OUT-OF-STATE-SHIPPER,SUPERVISOR-CREDENTIAL) values ('Hi2', 1, 'Hello2', '10/1/2023', '10/1/2023', 'Some shipper', 'Some credential');"
										  ,"DB_Files_2_Test_Versions\\Insert_Test_2.csv", 2, 0
								  		  ,&malloced_head, the_debug) != 0)
				result = -1;

			if (malloced_head != NULL)
			{
				if (the_debug == YES_DEBUG)
					printf("	ERROR in test_Driver_main() at line %d in %s\n", __LINE__, __FILE__);
				myFreeAllError(&malloced_head, the_debug);
				result = -1;
			}
		// END Test with id = 27

		// START Test with id = 28
			if (test_Driver_insertRows(28, "Insert into alc_brands (BRAND-NAME,CT-REGISTRATION-NUMBER,STATUS,EFFECTIVE,EXPIRATION,OUT-OF-STATE-SHIPPER,SUPERVISOR-CREDENTIAL) values ('Hi3', 2, 'Hello3', '10/2/2023', '10/2/2023', 'Some other shipper', 'Some other credential');"
										  ,"DB_Files_2_Test_Versions\\Insert_Test_3.csv", 3, 0
								  		  ,&malloced_head, the_debug) != 0)
				result = -1;

			if (malloced_head != NULL)
			{
				if (the_debug == YES_DEBUG)
					printf("	ERROR in test_Driver_main() at line %d in %s\n", __LINE__, __FILE__);
				myFreeAllError(&malloced_head, the_debug);
				result = -1;
			}
		// END Test with id = 28

		// START Test with id = 29
			if (test_Driver_deleteRows(29, "delete from alc_brands where BRAND-NAME = 'Hi';"
									  ,"DB_Files_2_Test_Versions\\Delete_Test_6.csv", 3, 1
							  		  ,&malloced_head, the_debug) != 0)
				result = -1;

			if (malloced_head != NULL)
			{
				if (the_debug == YES_DEBUG)
					printf("	ERROR in test_Driver_main() at line %d in %s\n", __LINE__, __FILE__);
				myFreeAllError(&malloced_head, the_debug);
				result = -1;
			}
		// END Test with id = 29

		// START Test with id = 30
			if (test_Driver_insertRows(30, "Insert into alc_brands (braND-name, STATUS) values ('Hi', 'Hello');"
										  ,"DB_Files_2_Test_Versions\\Insert_Test_3.csv", 3, 0
								  		  ,&malloced_head, the_debug) != 0)
				result = -1;

			if (malloced_head != NULL)
			{
				if (the_debug == YES_DEBUG)
					printf("	ERROR in test_Driver_main() at line %d in %s\n", __LINE__, __FILE__);
				myFreeAllError(&malloced_head, the_debug);
				result = -1;
			}
		// END Test with id = 30

		// START Test with id = 31
			if (test_Driver_insertRows(31, "Insert into alc_brands (braND-name, STATUS) values ('Hi4', 'Hello4');"
										  ,"DB_Files_2_Test_Versions\\Insert_Test_4.csv", 4, 0
								  		  ,&malloced_head, the_debug) != 0)
				result = -1;

			if (malloced_head != NULL)
			{
				if (the_debug == YES_DEBUG)
					printf("	ERROR in test_Driver_main() at line %d in %s\n", __LINE__, __FILE__);
				myFreeAllError(&malloced_head, the_debug);
				result = -1;
			}
		// END Test with id = 31

		// START Test with id = 32
			if (test_Driver_deleteRows(32, "delete from alc_brands where BRAND-NAME = 'Hi' or BRAND-NAME = 'Hi4';"
									  ,"DB_Files_2_Test_Versions\\Delete_Test_6.csv", 4, 2
							  		  ,&malloced_head, the_debug) != 0)
				result = -1;

			if (malloced_head != NULL)
			{
				if (the_debug == YES_DEBUG)
					printf("	ERROR in test_Driver_main() at line %d in %s\n", __LINE__, __FILE__);
				myFreeAllError(&malloced_head, the_debug);
				result = -1;
			}
		// END Test with id = 32

		// START Test with id = 33
			if (test_Driver_insertRows(33, "Insert into alc_brands (braND-name, STATUS) values ('Hi', 'Hello');"
										  ,"DB_Files_2_Test_Versions\\Insert_Test_5.csv", 4, 1
								  		  ,&malloced_head, the_debug) != 0)
				result = -1;

			if (malloced_head != NULL)
			{
				if (the_debug == YES_DEBUG)
					printf("	ERROR in test_Driver_main() at line %d in %s\n", __LINE__, __FILE__);
				myFreeAllError(&malloced_head, the_debug);
				result = -1;
			}
		// END Test with id = 33

		// START Test with id = 34
			if (test_Driver_insertRows(34, "Insert into alc_brands (braND-name, STATUS) values ('Hi', 'Hello');"
										  ,"DB_Files_2_Test_Versions\\Insert_Test_6.csv", 4, 0
								  		  ,&malloced_head, the_debug) != 0)
				result = -1;

			if (malloced_head != NULL)
			{
				if (the_debug == YES_DEBUG)
					printf("	ERROR in test_Driver_main() at line %d in %s\n", __LINE__, __FILE__);
				myFreeAllError(&malloced_head, the_debug);
				result = -1;
			}
		// END Test with id = 34

		// START Test with id = 35
			if (test_Driver_insertRows(35, "Insert into alc_brands (braND-name, STATUS) values ('Hi', 'Hello');"
										  ,"DB_Files_2_Test_Versions\\Insert_Test_7.csv", 5, 0
								  		  ,&malloced_head, the_debug) != 0)
				result = -1;

			if (malloced_head != NULL)
			{
				if (the_debug == YES_DEBUG)
					printf("	ERROR in test_Driver_main() at line %d in %s\n", __LINE__, __FILE__);
				myFreeAllError(&malloced_head, the_debug);
				result = -1;
			}
		// END Test with id = 35

		struct table_cols_info* cur_col = getTablesHead()->table_cols_head;
		while (cur_col != NULL)
		{
			struct frequent_node* cur_freq = cur_col->frequent_list_head;
			while (cur_freq != NULL)
			{
				printf("ptr_value = _%s_\n", cur_freq->ptr_value);
				printf("num_appearences = %d\n", cur_freq->num_appearences);
				traverseListNodes(&cur_freq->row_nums_head, &cur_freq->row_nums_tail, TRAVERSELISTNODES_HEAD, "row_nums_head = ");

				cur_freq = cur_freq->next;
			}

			cur_freq = cur_col->unique_list_head;
			while (cur_freq != NULL)
			{
				printf("ptr_value = _%s_\n", cur_freq->ptr_value);
				printf("num_appearences = %d\n", cur_freq->num_appearences);
				traverseListNodes(&cur_freq->row_nums_head, &cur_freq->row_nums_tail, TRAVERSELISTNODES_HEAD, "row_nums_head = ");

				cur_freq = cur_freq->next;
			}

			cur_col = cur_col->next;
		}
	// END test_Driver_insertRows*/


	/*
	// START test_Helper_DateFunctions_2
		for (int_8 i = 0; i<50000; i++)
		{
			if (test_Helper_DateFunctions_2(i, &malloced_head, the_debug) != 0)
				return -1;
		}
	// END test_Helper_DateFunctions_2*/


	/*
	printf ("\nStarting Performance Tests\n\n");
	
	the_debug = YES_DEBUG;

	// START test_Performance_Select
		// START Test with id = 1001
		if (test_Performance_Select(1001, "select * from alc_brands;", &malloced_head, the_debug) != 0)
			result = -1;

		if (malloced_head != NULL)
		{
			if (the_debug == YES_DEBUG)
				printf("	ERROR in test_Driver_main() at line %d in %s\n", __LINE__, __FILE__);
			return -3;
		}
		// END Test with id = 1001

		// START Test with id = 1002
		if (test_Performance_Select(1002, "select * from alc_brands where BRAND-NAME = 'INCARNADINE 19 VIOGNIER-PINOT GRIGIO PASO ROBLES';", &malloced_head, the_debug) != 0)
			result = -1;

		if (malloced_head != NULL)
		{
			if (the_debug == YES_DEBUG)
				printf("	ERROR in test_Driver_main() at line %d in %s\n", __LINE__, __FILE__);
			return -3;
		}
		// END Test with id = 1002

		// START Test with id = 1003
		if (test_Performance_Select(1003, "select * from alc_brands where BRAND-NAME = 'INCARNADINE 19 VIOGNIER-PINOT GRIGIO PASO ROBLES' and EFFECTIVE = '3/13/2020';", &malloced_head, the_debug) != 0)
			result = -1;

		if (malloced_head != NULL)
		{
			if (the_debug == YES_DEBUG)
				printf("	ERROR in test_Driver_main() at line %d in %s\n", __LINE__, __FILE__);
			return -3;
		}
		// END Test with id = 1003

		// START Test with id = 1004
		if (test_Performance_Select(1004, "select * from alc_brands where BRAND-NAME = 'INCARNADINE 19 VIOGNIER-PINOT GRIGIO PASO ROBLES' and EFFECTIVE = '3/13/2020' and CT-REGISTRATION-NUMBER = 165213 and STATUS = 'ACTIVE' and EXPIRATION = '3/12/2023' and OUT-OF-STATE-SHIPPER = 'PENROSE HILL LIMITED' and SUPERVISOR-CREDENTIAL = 'LSL.0001742';"
								   ,&malloced_head, the_debug) != 0)
			result = -1;

		if (malloced_head != NULL)
		{
			if (the_debug == YES_DEBUG)
				printf("	ERROR in test_Driver_main() at line %d in %s\n", __LINE__, __FILE__);
			return -3;
		}
		// END Test with id = 1004

		// START Test with id = 1005
		if (test_Performance_Select(1005, "select * from alc_brands where BRAND-NAME = 'INCARNADINE 19 VIOGNIER-PINOT GRIGIO PASO ROBLES' or EFFECTIVE = '7/11/2023' or CT-REGISTRATION-NUMBER = 55578 or STATUS = 'ACTIVE' or EXPIRATION = '12/6/2025' or OUT-OF-STATE-SHIPPER = 'ARTISAN WINES INC' or SUPERVISOR-CREDENTIAL = 'LSL.0001471';"
								   ,&malloced_head, the_debug) != 0)
			result = -1;

		if (malloced_head != NULL)
		{
			if (the_debug == YES_DEBUG)
				printf("	ERROR in test_Driver_main() at line %d in %s\n", __LINE__, __FILE__);
			return -3;
		}
		// END Test with id = 1005
	// END test_Performance_Select*/

	return result;
}