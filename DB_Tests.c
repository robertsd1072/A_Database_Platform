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

			new_temp->columns_arr[i]->func_node = NULL;
			new_temp->columns_arr[i]->math_node = NULL;
			new_temp->columns_arr[i]->case_node = NULL;

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

			new_temp->columns_arr[i]->func_node = NULL;
			new_temp->columns_arr[i]->math_node = NULL;
			new_temp->columns_arr[i]->case_node = NULL;
		}
	}
	else
	{
		new_temp->columns_arr_size = columns_arr_size;
		new_temp->columns_arr = columns_arr;
	}

	new_temp->where_head = where_head;
	new_temp->join_head = join_head;
	new_temp->having_head = NULL;

	new_temp->prev = NULL;
	new_temp->next = NULL;

	new_temp->order_by = NULL;

	*add_to_this_node = new_temp;

	return 0;
}

int compFuncNodes(int test_id, struct func_node* actual_func, struct func_node* expected_func)
{
	if (actual_func->which_func != expected_func->which_func)
	{
		printf("test_Controller_parseSelect with id = %d FAILED\n", test_id);
		printf("Actual actual_func->which_func (%d) did not equal below\n", actual_func->which_func);
		printf("Expected expected_func->which_func (%d)\n", expected_func->which_func);
		return -1;
	}

	if (actual_func->distinct != expected_func->distinct)
	{
		printf("test_Controller_parseSelect with id = %d FAILED\n", test_id);
		printf("Actual actual_func->distinct (%s) did not equal below\n", actual_func->distinct ? "TRUE" : "FALSE");
		printf("Expected expected_func->distinct (%s)\n", expected_func->distinct ? "TRUE" : "FALSE");
		return -1;
	}

	if (actual_func->args_size != expected_func->args_size)
	{
		printf("test_Controller_parseSelect with id = %d FAILED\n", test_id);
		printf("Actual actual_func->args_size (%d) did not equal below\n", actual_func->args_size);
		printf("Expected expected_func->args_size (%d)\n", expected_func->args_size);
		return -1;
	}

	for (int j=0; j<actual_func->args_size; j++)
	{
		if (strcmp(actual_func->args_arr[j], expected_func->args_arr[j]) != 0)
		{
			printf("test_Controller_parseSelect with id = %d FAILED\n", test_id);
			printf("Actual actual_func->args_arr[j] (%s) did not equal below\n", actual_func->args_arr[j]);
			printf("Expected expected_func->args_arr[j] (%s)\n", expected_func->args_arr[j]);
			return -1;
		}
	}

	struct ListNodePtr* cur_act_group_by = actual_func->group_by_cols_head;
	struct ListNodePtr* cur_exp_group_by = expected_func->group_by_cols_head;

	while (cur_act_group_by != NULL || cur_exp_group_by != NULL)
	{
		if (cur_act_group_by == NULL && cur_exp_group_by != NULL)
		{
			printf("test_Controller_parseSelect with id = %d FAILED\n", test_id);
			printf("cur_act_group_by was NULL and cur_exp_group_by was NOT NULL\n");
			return -1;
		}
		else if (cur_act_group_by != NULL && cur_exp_group_by == NULL)
		{
			printf("test_Controller_parseSelect with id = %d FAILED\n", test_id);
			printf("cur_act_group_by was NOT NULL and cur_exp_group_by was NULL\n");
			return -1;
		}

		if (cur_act_group_by->ptr_value == NULL && cur_exp_group_by->ptr_value != NULL)
		{
			printf("test_Controller_parseSelect with id = %d FAILED\n", test_id);
			printf("cur_act_group_by->ptr_value was NULL and cur_exp_group_by->ptr_value was NOT NULL\n");
			return -1;
		}
		else if (cur_act_group_by->ptr_value != NULL && cur_exp_group_by->ptr_value == NULL)
		{
			printf("test_Controller_parseSelect with id = %d FAILED\n", test_id);
			printf("cur_act_group_by->ptr_value was NOT NULL and cur_exp_group_by->ptr_value was NULL\n");
			return -1;
		}

		if (cur_act_group_by->ptr_type != cur_exp_group_by->ptr_type)
		{
			printf("test_Controller_parseSelect with id = %d FAILED\n", test_id);
			printf("cur_act_group_by->ptr_type (%d) did not equal below\n", cur_act_group_by->ptr_type);
			printf("cur_exp_group_by->ptr_type (%d)\n", cur_exp_group_by->ptr_type);
			return -1;
		}

		void* act_ptr_value = NULL;
		void* exp_ptr_value = NULL;

		if (cur_act_group_by->ptr_type == PTR_TYPE_COL_IN_SELECT_NODE)
		{
			struct col_in_select_node* cur_1 = ((struct col_in_select_node*) cur_act_group_by->ptr_value);
			while (cur_1->table_ptr_type == PTR_TYPE_SELECT_NODE)
				cur_1 = cur_1->col_ptr;

			struct col_in_select_node* cur_2 = ((struct col_in_select_node*) cur_exp_group_by->ptr_value);
			while (cur_2->table_ptr_type == PTR_TYPE_SELECT_NODE)
				cur_2 = cur_2->col_ptr;

			act_ptr_value = cur_1->col_ptr;
			exp_ptr_value = cur_2->col_ptr;
		}

		/*if (cur_act_group_by->ptr_type == PTR_TYPE_COL_IN_SELECT_NODE)
		{
			act_ptr_value = ((struct col_in_select_node*) cur_act_group_by->ptr_value)->col_ptr;
			exp_ptr_value = ((struct col_in_select_node*) cur_exp_group_by->ptr_value)->col_ptr;
		}*/

		if (act_ptr_value != exp_ptr_value)
		{
			printf("test_Controller_parseSelect with id = %d FAILED\n", test_id);
			if (act_ptr_value == NULL)
				printf("act_ptr_value col_ptr was NULL\n");
			else if (exp_ptr_value == NULL)
				printf("exp_ptr_value col_ptr was NULL\n");
			else
			{
				printf("act_ptr_value col_ptr (%s) did not equal exp_ptr_value col_ptr (%s)\n"
					   ,((struct table_cols_info*) act_ptr_value)->col_name, ((struct table_cols_info*) exp_ptr_value)->col_name);
			}
			return -1;
		}

		if (cur_act_group_by != NULL)
			cur_act_group_by = cur_act_group_by->next;
		if (cur_exp_group_by != NULL)
			cur_exp_group_by = cur_exp_group_by->next;
	}
}

int compMathOrWhereTree(int test_id, int tree_ptr_type, void* actual_ptr, void* expected_ptr)
{
	if (tree_ptr_type == PTR_TYPE_MATH_NODE)
	{
		if (actual_ptr != NULL && expected_ptr == NULL)
		{
			printf("compMathOrWhereTree with id = %d FAILED\n", test_id);
			printf("Actual math_node was NOT NULL and expected math_node was NULL\n");
			return -1;
		}
		else if (actual_ptr == NULL && expected_ptr != NULL)
		{
			printf("compMathOrWhereTree with id = %d FAILED\n", test_id);
			printf("Actual math_node was NULL and expected math_node was NOT NULL\n");
			return -1;
		}

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
				{
					printf("Returned from compMathOrWhereTree()\n");
					return -1;
				}
			}
			else if (ptr_type == PTR_TYPE_INT && *((int*) act_ptr) != *((int*) exp_ptr))
			{
				printf("compMathOrWhereTree with id = %d FAILED\n", test_id);
				printf("Actual math_node ptr %d did not equal below\n", *((int*) act_ptr));
				printf("Expected math_node ptr %d\n", *((int*) exp_ptr));
				return -1;
			}
			else if (ptr_type == PTR_TYPE_REAL)
			{
				if (*((double*) act_ptr) != *((double*) exp_ptr))
				{
					printf("compMathOrWhereTree with id = %d FAILED\n", test_id);
					printf("Actual math_node ptr %lf did not equal below\n", *((double*) act_ptr));
					printf("Expected math_node ptr %lf\n", *((double*) exp_ptr));
					return -1;
				}
			}
			else if (ptr_type == PTR_TYPE_COL_IN_SELECT_NODE)
			{
				act_ptr = ((struct col_in_select_node*) act_ptr)->col_ptr;
				exp_ptr = ((struct col_in_select_node*) exp_ptr)->col_ptr;

				if (act_ptr != exp_ptr)
				{
					printf("compMathOrWhereTree with id = %d FAILED\n", test_id);
					printf("Actual math_node ptr (%s) did not equal below\n",((struct table_cols_info*) act_ptr)->col_name);
					printf("Expected math_node ptr (%s)\n", ((struct table_cols_info*) exp_ptr)->col_name);
					return -1;
				}
			}
		}

		if (((struct math_node*) actual_ptr)->operation != ((struct math_node*) expected_ptr)->operation)
		{
			printf("compMathOrWhereTree with id = %d FAILED\n", test_id);
			printf("Actual math_node operation %d did not equal below\n", ((struct math_node*) actual_ptr)->operation);
			printf("Expected math_node operation %d\n", ((struct math_node*) expected_ptr)->operation);
			return -1;
		}
	}
	else //if (tree_ptr_type == PTR_TYPE_WHERE_CLAUSE_NODE)
	{
		if (actual_ptr != NULL && expected_ptr == NULL)
		{
			printf("compMathOrWhereTree with id = %d FAILED\n", test_id);
			printf("Actual where_clause_node was NOT NULL and expected where_clause_node was NULL\n");
			return -1;
		}
		else if (actual_ptr == NULL && expected_ptr != NULL)
		{
			printf("compMathOrWhereTree with id = %d FAILED\n", test_id);
			printf("Actual where_clause_node was NULL and expected where_clause_node was NOT NULL\n");
			return -1;
		}

		if (((struct where_clause_node*) actual_ptr)->ptr_one_type != ((struct where_clause_node*) expected_ptr)->ptr_one_type)
		{
			printf("compMathOrWhereTree with id = %d FAILED\n", test_id);
			printf("Actual where_clause_node ptr_one_type %d did not equal below\n", ((struct where_clause_node*) actual_ptr)->ptr_one_type);
			printf("Expected where_clause_node ptr_one_type %d\n", ((struct where_clause_node*) expected_ptr)->ptr_one_type);
			return -1;
		}

		if (((struct where_clause_node*) actual_ptr)->ptr_two_type != ((struct where_clause_node*) expected_ptr)->ptr_two_type)
		{
			printf("compMathOrWhereTree with id = %d FAILED\n", test_id);
			printf("Actual where_clause_node ptr_two_type %d did not equal below\n", ((struct where_clause_node*) actual_ptr)->ptr_two_type);
			printf("Expected where_clause_node ptr_two_type %d\n", ((struct where_clause_node*) expected_ptr)->ptr_two_type);
			return -1;
		}


		for (int i=0; i<2; i++)
		{
			int ptr_type;

			void* act_ptr;
			void* exp_ptr;

			if (i == 0)
			{
				ptr_type = ((struct where_clause_node*) actual_ptr)->ptr_one_type;

				act_ptr = ((struct where_clause_node*) actual_ptr)->ptr_one;
				exp_ptr = ((struct where_clause_node*) expected_ptr)->ptr_one;
			}
			else //if (i == 1)
			{
				ptr_type = ((struct where_clause_node*) actual_ptr)->ptr_two_type;

				act_ptr = ((struct where_clause_node*) actual_ptr)->ptr_two;
				exp_ptr = ((struct where_clause_node*) expected_ptr)->ptr_two;
			}

			if (ptr_type == PTR_TYPE_WHERE_CLAUSE_NODE)
			{
				if (compMathOrWhereTree(test_id, PTR_TYPE_WHERE_CLAUSE_NODE, act_ptr, exp_ptr) != 0)
				{
					printf("Returned from compMathOrWhereTree() for WHERE node\n");
					return -1;
				}
			}
			else if (ptr_type == PTR_TYPE_MATH_NODE)
			{
				if (compMathOrWhereTree(test_id, PTR_TYPE_MATH_NODE, act_ptr, exp_ptr) != 0)
				{
					printf("Returned from compMathOrWhereTree() for MATH node\n");
					return -1;
				}
			}
			else if (ptr_type == PTR_TYPE_FUNC_NODE)
			{
				if (compFuncNodes(test_id, act_ptr, exp_ptr) != 0)
				{
					return -1;
				}
			}
			else if (ptr_type == PTR_TYPE_INT && *((int*) act_ptr) != *((int*) exp_ptr))
			{
				printf("compMathOrWhereTree with id = %d FAILED\n", test_id);
				printf("Actual where_clause_node ptr %d did not equal below for node #%d\n", *((int*) act_ptr), i+1);
				printf("Expected where_clause_node ptr %d for node #%d\n", *((int*) exp_ptr), i+1);
				return -1;
			}
			else if (ptr_type == PTR_TYPE_REAL)
			{
				if (*((double*) act_ptr) != *((double*) exp_ptr))
				{
					printf("compMathOrWhereTree with id = %d FAILED\n", test_id);
					printf("Actual where_clause_node ptr %lf did not equal below for node #%d\n", *((double*) act_ptr), i+1);
					printf("Expected where_clause_node ptr %lf for node #%d\n", *((double*) exp_ptr), i+1);
					return -1;
				}
			}
			else if (ptr_type == PTR_TYPE_CHAR || ptr_type == PTR_TYPE_DATE)
			{
				if (strcmp(act_ptr, exp_ptr) != 0)
				{
					printf("compMathOrWhereTree with id = %d FAILED\n", test_id);
					printf("Actual where_clause_node ptr (%s) did not equal below for node #%d\n", act_ptr, i+1);
					printf("Expected where_clause_node ptr (%s) for node #%d\n", exp_ptr, i+1);
					return -1;
				}
			}
			else if (ptr_type == PTR_TYPE_COL_IN_SELECT_NODE)
			{
				act_ptr = ((struct col_in_select_node*) act_ptr)->col_ptr;
				exp_ptr = ((struct col_in_select_node*) exp_ptr)->col_ptr;

				//printf("act_ptr->col_ptr_type = %d\n", ((struct col_in_select_node*) act_ptr)->col_ptr_type);
				//printf("exp_ptr->col_ptr_type = %d\n", ((struct col_in_select_node*) exp_ptr)->col_ptr_type);

				if (((struct col_in_select_node*) act_ptr)->col_ptr_type == PTR_TYPE_TABLE_COLS_INFO)
				{
					act_ptr = ((struct col_in_select_node*) act_ptr)->col_ptr;
					exp_ptr = ((struct col_in_select_node*) exp_ptr)->col_ptr;
				}
				else
				{
					//printf("Here\n");

					while (((struct col_in_select_node*) act_ptr)->col_ptr_type == PTR_TYPE_COL_IN_SELECT_NODE
							&& (act_ptr != NULL || exp_ptr != NULL))
					{
						if (act_ptr == NULL || exp_ptr == NULL)
						{
							printf("compMathOrWhereTree with id = %d FAILED\n", test_id);
							printf("Actual where_clause_node ptr did not cur = cur->col_ptr to the same level as expected where_clause_node ptr\n");
							return -1;
						}

						act_ptr = ((struct col_in_select_node*) act_ptr)->col_ptr;
						exp_ptr = ((struct col_in_select_node*) exp_ptr)->col_ptr;

						//printf("Did\n");
					}
				}


				//printf("act_ptr col_name = _%s_\n", ((struct table_cols_info*) act_ptr)->col_name);
				//printf("exp_ptr col_name = _%s_\n", ((struct table_cols_info*) exp_ptr)->col_name);
					

				if (act_ptr != exp_ptr)
				{
					printf("compMathOrWhereTree with id = %d FAILED\n", test_id);
					printf("Actual where_clause_node ptr (%s) did not equal below for node #%d\n", ((struct table_cols_info*) act_ptr)->col_name, i+1);
					printf("Expected where_clause_node ptr (%s) for node #%d\n", ((struct table_cols_info*) exp_ptr)->col_name, i+1);
					return -1;
				}
			}
		}

		if (((struct where_clause_node*) actual_ptr)->where_type != ((struct where_clause_node*) expected_ptr)->where_type)
		{
			printf("compMathOrWhereTree with id = %d FAILED\n", test_id);
			printf("Actual where_clause_node where_type %d did not equal below\n", ((struct where_clause_node*) actual_ptr)->where_type);
			printf("Expected where_clause_node where_type %d\n", ((struct where_clause_node*) expected_ptr)->where_type);
			return -1;
		}
	}

	return 0;
}


int test_Controller_parseWhereClause(int test_id, char* where_string, char* first_word
									,int* error_code, struct select_node* the_select_node, struct where_clause_node** expected_where_head
									,struct malloced_node** malloced_head, int the_debug)
{
	setOutputGreen();
	printf("\nStarting test with id = %d\n", test_id);
	setOutputWhite();

	
	struct where_clause_node* where_head = NULL;
	*error_code = parseWhereClause(where_string, &where_head, the_select_node, NULL, first_word, malloced_head, the_debug);


	if (*error_code == -1 && *expected_where_head != NULL)
	{
		setOutputRed();
		printf("test_Controller_parseWhereClause with id = %d FAILED\n", test_id);
		printf("Actual where_head was NULL and expected_where_head was NOT NULL\n");
		setOutputWhite();

		return -1;
	}
	else if (where_head == NULL && *expected_where_head == NULL)
		return 0;


	setOutputRed();
	if (compMathOrWhereTree(test_id, PTR_TYPE_WHERE_CLAUSE_NODE, where_head, *expected_where_head) != 0)
	{
		freeAnyLinkedList((void**) &where_head, PTR_TYPE_WHERE_CLAUSE_NODE, NULL, malloced_head, the_debug);
		setOutputWhite(); return -1;
	}
	setOutputWhite();


	// START Free everything
	freeAnyLinkedList((void**) &where_head, PTR_TYPE_WHERE_CLAUSE_NODE, NULL, malloced_head, the_debug);
	// END Free everything


	return 0;
}

int test_Controller_parseUpdate(int test_id, char* update_string, struct change_node_v2** expected_change_head, int* parsed_error_code
							   ,struct malloced_node** malloced_head, int the_debug)
{
	setOutputGreen();
	printf("\nStarting test with id = %d\n", test_id);
	setOutputWhite();

	struct change_node_v2* change_head = NULL;

	*parsed_error_code = parseUpdate(update_string, &change_head, malloced_head, the_debug);
	if (*parsed_error_code != 0 && *expected_change_head != NULL)	
	{
		setOutputRed();
		printf("test_Controller_parseUpdate with id = %d FAILED\n", test_id);
		printf("The test %d had a problem with parseUpdate()\n", test_id);
		setOutputWhite();
		freeAnyLinkedList((void**) &change_head, PTR_TYPE_CHANGE_NODE_V2, NULL, malloced_head, the_debug);
		return -1;
	}
	else if (*parsed_error_code == 0 && *expected_change_head == NULL)
	{
		setOutputRed();
		printf("test_Controller_parseUpdate with id = %d FAILED\n", test_id);
		printf("parseUpdate() completely successfully but expected_change_head is NULL\n", test_id);
		setOutputWhite();
		freeAnyLinkedList((void**) &change_head, PTR_TYPE_CHANGE_NODE_V2, NULL, malloced_head, the_debug);
		return -1;
	}
	else if (*parsed_error_code != 0 && *expected_change_head == NULL)
	{
		return 0;
	}


	setOutputRed();
	// START Tests
		if ((*expected_change_head)->table != change_head->table)
		{
			printf("test_Controller_parseUpdate with id = %d FAILED\n", test_id);
			printf("Actual table (%s) did not equal expected table (%s)\n", change_head->table->name, (*expected_change_head)->table->name);
			setOutputWhite();
			freeAnyLinkedList((void**) &change_head, PTR_TYPE_CHANGE_NODE_V2, NULL, malloced_head, the_debug);
			return -1;
		}

		if ((*expected_change_head)->operation != change_head->operation)
		{
			printf("test_Controller_parseUpdate with id = %d FAILED\n", test_id);
			printf("Actual operation (%d) did not equal expected operation (%d)\n", change_head->operation, (*expected_change_head)->operation);
			setOutputWhite();
			freeAnyLinkedList((void**) &change_head, PTR_TYPE_CHANGE_NODE_V2, NULL, malloced_head, the_debug);
			return -1;
		}

		struct ListNodePtr* cur_actual_col = change_head->col_list_head;
		struct ListNodePtr* cur_exp_col = (*expected_change_head)->col_list_head;
		while (cur_actual_col != NULL || cur_exp_col != NULL)
		{
			if (cur_actual_col == NULL && cur_exp_col != NULL)
			{
				printf("test_Controller_parseUpdate with id = %d FAILED\n", test_id);
				printf("cur_actual_col was NULL and cur_exp_col was NOT NULL\n");
				setOutputWhite();
				freeAnyLinkedList((void**) &change_head, PTR_TYPE_CHANGE_NODE_V2, NULL, malloced_head, the_debug);
				return -1;
			}
			else if (cur_actual_col != NULL && cur_exp_col == NULL)
			{
				printf("test_Controller_parseUpdate with id = %d FAILED\n", test_id);
				printf("cur_actual_col was NOT NULL and cur_exp_col was NULL\n");
				setOutputWhite();
				freeAnyLinkedList((void**) &change_head, PTR_TYPE_CHANGE_NODE_V2, NULL, malloced_head, the_debug);
				return -1;
			}
			else if (cur_actual_col != NULL && cur_exp_col != NULL)
			{
				if (cur_actual_col->ptr_type != cur_exp_col->ptr_type)
				{
					printf("test_Controller_parseUpdate with id = %d FAILED\n", test_id);
					printf("cur_actual_col->ptr_type (%d) did not equal cur_exp_col->ptr_type (%d)\n", cur_actual_col->ptr_type, cur_exp_col->ptr_type);
					setOutputWhite();
					freeAnyLinkedList((void**) &change_head, PTR_TYPE_CHANGE_NODE_V2, NULL, malloced_head, the_debug);
					return -1;
				}

				if (cur_actual_col->ptr_value != cur_exp_col->ptr_value)
				{
					printf("test_Controller_parseUpdate with id = %d FAILED\n", test_id);
					printf("cur_actual_col->ptr_value (%s) did not equal cur_exp_col->ptr_value (%s)\n", ((struct table_cols_info*) cur_actual_col->ptr_value)->col_name, ((struct table_cols_info*) cur_exp_col->ptr_value)->col_name);
					setOutputWhite();
					freeAnyLinkedList((void**) &change_head, PTR_TYPE_CHANGE_NODE_V2, NULL, malloced_head, the_debug);
					return -1;
				}
			}

			if (cur_actual_col != NULL)
				cur_actual_col = cur_actual_col->next;
			if (cur_exp_col != NULL)
				cur_exp_col = cur_exp_col->next;
		}

		struct ListNodePtr* cur_actual_data = change_head->data_list_head;
		struct ListNodePtr* cur_exp_data = (*expected_change_head)->data_list_head;
		while (cur_actual_data != NULL || cur_exp_data != NULL)
		{
			if (cur_actual_data == NULL && cur_exp_data != NULL)
			{
				printf("test_Controller_parseUpdate with id = %d FAILED\n", test_id);
				printf("cur_actual_data was NULL and cur_exp_data was NOT NULL\n");
				setOutputWhite();
				freeAnyLinkedList((void**) &change_head, PTR_TYPE_CHANGE_NODE_V2, NULL, malloced_head, the_debug);
				return -1;
			}
			else if (cur_actual_data != NULL && cur_exp_data == NULL)
			{
				printf("test_Controller_parseUpdate with id = %d FAILED\n", test_id);
				printf("cur_actual_data was NOT NULL and cur_exp_data was NULL\n");
				setOutputWhite();
				freeAnyLinkedList((void**) &change_head, PTR_TYPE_CHANGE_NODE_V2, NULL, malloced_head, the_debug);
				return -1;
			}
			else if (cur_actual_data != NULL && cur_exp_data != NULL)
			{
				if (cur_actual_data->ptr_type != cur_exp_data->ptr_type)
				{
					printf("test_Controller_parseUpdate with id = %d FAILED\n", test_id);
					printf("cur_actual_data->ptr_type (%d) did not equal cur_exp_data->ptr_type (%d)\n", cur_actual_data->ptr_type, cur_exp_data->ptr_type);
					setOutputWhite();
					freeAnyLinkedList((void**) &change_head, PTR_TYPE_CHANGE_NODE_V2, NULL, malloced_head, the_debug);
					return -1;
				}

				if (cur_actual_data->ptr_type == PTR_TYPE_INT && *((int*) cur_actual_data->ptr_value) != *((int*) cur_exp_data->ptr_value))
				{
					printf("test_Controller_parseUpdate with id = %d FAILED\n", test_id);
					printf("cur_actual_data->ptr_value %d did not equal below\n", *((int*) cur_actual_data->ptr_value));
					printf("cur_exp_data->ptr_value %d\n", *((int*) cur_exp_data->ptr_value));
					setOutputWhite();
					freeAnyLinkedList((void**) &change_head, PTR_TYPE_CHANGE_NODE_V2, NULL, malloced_head, the_debug);
					return -1;
				}
				else if (cur_actual_data->ptr_type == PTR_TYPE_REAL && *((double*) cur_actual_data->ptr_value) != *((double*) cur_exp_data->ptr_value))
				{
					printf("test_Controller_parseUpdate with id = %d FAILED\n", test_id);
					printf("cur_actual_data->ptr_value %lf did not equal below\n", *((double*) cur_actual_data->ptr_value));
					printf("cur_exp_data->ptr_value %lf\n", *((double*) cur_exp_data->ptr_value));
					setOutputWhite();
					freeAnyLinkedList((void**) &change_head, PTR_TYPE_CHANGE_NODE_V2, NULL, malloced_head, the_debug);
					return -1;
				}
				else if (cur_actual_data->ptr_type == PTR_TYPE_CHAR || cur_actual_data->ptr_type == PTR_TYPE_DATE)
				{
					if (strcmp(cur_actual_data->ptr_value, cur_exp_data->ptr_value) != 0)
					{
						printf("test_Controller_parseUpdate with id = %d FAILED\n", test_id);
						printf("cur_actual_data->ptr_value (%s) did not equal below\n", cur_actual_data->ptr_value);
						printf("cur_exp_data->ptr_value (%s)\n", cur_exp_data->ptr_value);
						setOutputWhite();
						freeAnyLinkedList((void**) &change_head, PTR_TYPE_CHANGE_NODE_V2, NULL, malloced_head, the_debug);
						return -1;
					}
				}
			}

			if (cur_actual_data != NULL)
				cur_actual_data = cur_actual_data->next;
			if (cur_exp_data != NULL)
				cur_exp_data = cur_exp_data->next;
		}


		if (change_head->where_head == NULL && (*expected_change_head)->where_head != NULL)
		{
			printf("test_Controller_parseUpdate with id = %d FAILED\n", test_id);
			printf("Actual where_head was NULL and expected where_head was NOT NULL\n");
			setOutputWhite();
			freeAnyLinkedList((void**) &change_head, PTR_TYPE_CHANGE_NODE_V2, NULL, malloced_head, the_debug);
			return -1;
		}
		else if (change_head->where_head != NULL && (*expected_change_head)->where_head == NULL)
		{
			printf("test_Controller_parseUpdate with id = %d FAILED\n", test_id);
			printf("Actual where_head was NOT NULL and expected where_head was NULL\n");
			setOutputWhite();
			freeAnyLinkedList((void**) &change_head, PTR_TYPE_CHANGE_NODE_V2, NULL, malloced_head, the_debug);
			return -1;
		}
		else if (change_head->where_head != NULL && (*expected_change_head)->where_head != NULL 
				 && compMathOrWhereTree(test_id, PTR_TYPE_WHERE_CLAUSE_NODE, change_head->where_head, (*expected_change_head)->where_head) != 0)
		{
			setOutputWhite();
			freeAnyLinkedList((void**) &change_head, PTR_TYPE_CHANGE_NODE_V2, NULL, malloced_head, the_debug);
			return -1;
		}
	// END Tests


	setOutputWhite();

	// START Free rest of change_head
		freeAnyLinkedList((void**) &change_head, PTR_TYPE_CHANGE_NODE_V2, NULL, malloced_head, the_debug);
	// END Free rest of change_head

	return 0;
}

int test_Controller_parseDelete(int test_id, char* delete_string, struct change_node_v2** expected_change_head, int* parsed_error_code
							   ,struct malloced_node** malloced_head, int the_debug)
{
	setOutputGreen();
	printf("\nStarting test with id = %d\n", test_id);
	setOutputWhite();

	struct change_node_v2* change_head = NULL;

	*parsed_error_code = parseDelete(delete_string, &change_head, malloced_head, the_debug);
	if (*parsed_error_code != 0 && *expected_change_head != NULL)	
	{
		setOutputRed();
		printf("test_Controller_parseDelete with id = %d FAILED\n", test_id);
		printf("The test %d had a problem with parseDelete()\n", test_id);
		setOutputWhite();
		return -1;
	}
	else if (*parsed_error_code == 0 && *expected_change_head == NULL)
	{
		setOutputRed();
		printf("test_Controller_parseDelete with id = %d FAILED\n", test_id);
		printf("parseDelete() completely successfully but expected_change_head is NULL\n", test_id);
		setOutputWhite();
		freeAnyLinkedList((void**) &change_head, PTR_TYPE_CHANGE_NODE_V2, NULL, malloced_head, the_debug);
		return -1;
	}
	else if (*parsed_error_code != 0 && *expected_change_head == NULL)
	{
		return 0;
	}

	setOutputRed();
	// START Tests
		if ((*expected_change_head)->table != change_head->table)
		{
			printf("test_Controller_parseDelete with id = %d FAILED\n", test_id);
			printf("Actual table (%s) did not equal expected table (%s)\n", change_head->table->name, (*expected_change_head)->table->name);
			setOutputWhite();
			freeAnyLinkedList((void**) &change_head, PTR_TYPE_CHANGE_NODE_V2, NULL, malloced_head, the_debug);
			return -1;
		}

		if ((*expected_change_head)->operation != change_head->operation)
		{
			printf("test_Controller_parseDelete with id = %d FAILED\n", test_id);
			printf("Actual operation (%d) did not equal expected operation (%d)\n", change_head->operation, (*expected_change_head)->operation);
			setOutputWhite();
			freeAnyLinkedList((void**) &change_head, PTR_TYPE_CHANGE_NODE_V2, NULL, malloced_head, the_debug);
			return -1;
		}

		if (change_head->where_head == NULL && (*expected_change_head)->where_head != NULL)
		{
			printf("test_Controller_parseDelete with id = %d FAILED\n", test_id);
			printf("Actual where_head was NULL and expected where_head was NOT NULL\n");
			setOutputWhite();
			freeAnyLinkedList((void**) &change_head, PTR_TYPE_CHANGE_NODE_V2, NULL, malloced_head, the_debug);
			return -1;
		}
		else if (change_head->where_head != NULL && (*expected_change_head)->where_head == NULL)
		{
			printf("test_Controller_parseDelete with id = %d FAILED\n", test_id);
			printf("Actual where_head was NOT NULL and expected where_head was NULL\n");
			setOutputWhite();
			freeAnyLinkedList((void**) &change_head, PTR_TYPE_CHANGE_NODE_V2, NULL, malloced_head, the_debug);
			return -1;
		}
		else if (change_head->where_head != NULL && (*expected_change_head)->where_head != NULL 
				 && compMathOrWhereTree(test_id, PTR_TYPE_WHERE_CLAUSE_NODE, change_head->where_head, (*expected_change_head)->where_head) != 0)
		{
			setOutputWhite();
			freeAnyLinkedList((void**) &change_head, PTR_TYPE_CHANGE_NODE_V2, NULL, malloced_head, the_debug);
			return -1;
		}
	// END Tests


	setOutputWhite();

	// START Free rest of change_head
		freeAnyLinkedList((void**) &change_head, PTR_TYPE_CHANGE_NODE_V2, NULL, malloced_head, the_debug);
	// END Free rest of change_head

	return 0;
}

int test_Controller_parseInsert(int test_id, char* insert_string, struct change_node_v2** expected_change_head, int* parsed_error_code
							   ,struct malloced_node** malloced_head, int the_debug)
{
	setOutputGreen();
	printf("\nStarting test with id = %d\n", test_id);
	setOutputWhite();

	struct change_node_v2* change_head = NULL;
	
	*parsed_error_code = parseInsert(insert_string, &change_head, malloced_head, the_debug);
	if (*parsed_error_code != 0 && *expected_change_head != NULL)	
	{
		setOutputRed();
		printf("test_Controller_parseInsert with id = %d FAILED\n", test_id);
		printf("The test %d had a problem with parseInsert()\n", test_id);
		setOutputWhite();
		return -1;
	}
	else if (*parsed_error_code == 0 && *expected_change_head == NULL)
	{
		setOutputRed();
		printf("test_Controller_parseInsert with id = %d FAILED\n", test_id);
		printf("parseInsert() completely successfully but expected_change_head is NULL\n", test_id);
		setOutputWhite();
		freeAnyLinkedList((void**) &change_head, PTR_TYPE_CHANGE_NODE_V2, NULL, malloced_head, the_debug);
		return -1;
	}
	else if (*parsed_error_code != 0 && *expected_change_head == NULL)
	{
		return 0;
	}


	setOutputRed();
	// START Tests
		if ((*expected_change_head)->table != change_head->table)
		{
			printf("test_Controller_parseInsert with id = %d FAILED\n", test_id);
			printf("Actual table (%s) did not equal expected table (%s)\n", change_head->table->name, (*expected_change_head)->table->name);
			setOutputWhite();
			freeAnyLinkedList((void**) &change_head, PTR_TYPE_CHANGE_NODE_V2, NULL, malloced_head, the_debug);
			return -1;
		}

		if ((*expected_change_head)->operation != change_head->operation)
		{
			printf("test_Controller_parseInsert with id = %d FAILED\n", test_id);
			printf("Actual operation (%d) did not equal expected operation (%d)\n", change_head->operation, (*expected_change_head)->operation);
			setOutputWhite();
			freeAnyLinkedList((void**) &change_head, PTR_TYPE_CHANGE_NODE_V2, NULL, malloced_head, the_debug);
			return -1;
		}

		struct ListNodePtr* cur_actual_col = change_head->col_list_head;
		struct ListNodePtr* cur_exp_col = (*expected_change_head)->col_list_head;
		while (cur_actual_col != NULL || cur_exp_col != NULL)
		{
			if (cur_actual_col == NULL && cur_exp_col != NULL)
			{
				printf("test_Controller_parseInsert with id = %d FAILED\n", test_id);
				printf("cur_actual_col was NULL and cur_exp_col was NOT NULL\n");
				setOutputWhite();
				freeAnyLinkedList((void**) &change_head, PTR_TYPE_CHANGE_NODE_V2, NULL, malloced_head, the_debug);
				return -1;
			}
			else if (cur_actual_col != NULL && cur_exp_col == NULL)
			{
				printf("test_Controller_parseInsert with id = %d FAILED\n", test_id);
				printf("cur_actual_col was NOT NULL and cur_exp_col was NULL\n");
				setOutputWhite();
				freeAnyLinkedList((void**) &change_head, PTR_TYPE_CHANGE_NODE_V2, NULL, malloced_head, the_debug);
				return -1;
			}
			else if (cur_actual_col != NULL && cur_exp_col != NULL)
			{
				if (cur_actual_col->ptr_type != cur_exp_col->ptr_type)
				{
					printf("test_Controller_parseInsert with id = %d FAILED\n", test_id);
					printf("cur_actual_col->ptr_type (%d) did not equal cur_exp_col->ptr_type (%d)\n", cur_actual_col->ptr_type, cur_exp_col->ptr_type);
					setOutputWhite();
					freeAnyLinkedList((void**) &change_head, PTR_TYPE_CHANGE_NODE_V2, NULL, malloced_head, the_debug);
					return -1;
				}

				if (cur_actual_col->ptr_value != cur_exp_col->ptr_value)
				{
					printf("test_Controller_parseInsert with id = %d FAILED\n", test_id);
					printf("cur_actual_col->ptr_value (%s) did not equal cur_exp_col->ptr_value (%s)\n", ((struct table_cols_info*) cur_actual_col->ptr_value)->col_name, ((struct table_cols_info*) cur_exp_col->ptr_value)->col_name);
					setOutputWhite();
					freeAnyLinkedList((void**) &change_head, PTR_TYPE_CHANGE_NODE_V2, NULL, malloced_head, the_debug);
					return -1;
				}
			}

			if (cur_actual_col != NULL)
				cur_actual_col = cur_actual_col->next;
			if (cur_exp_col != NULL)
				cur_exp_col = cur_exp_col->next;
		}

		struct ListNodePtr* cur_actual_data = change_head->data_list_head;
		struct ListNodePtr* cur_exp_data = (*expected_change_head)->data_list_head;
		while (cur_actual_data != NULL || cur_exp_data != NULL)
		{
			if (cur_actual_data == NULL && cur_exp_data != NULL)
			{
				printf("test_Controller_parseInsert with id = %d FAILED\n", test_id);
				printf("cur_actual_data was NULL and cur_exp_data was NOT NULL\n");
				setOutputWhite();
				freeAnyLinkedList((void**) &change_head, PTR_TYPE_CHANGE_NODE_V2, NULL, malloced_head, the_debug);
				return -1;
			}
			else if (cur_actual_data != NULL && cur_exp_data == NULL)
			{
				printf("test_Controller_parseInsert with id = %d FAILED\n", test_id);
				printf("cur_actual_data was NOT NULL and cur_exp_data was NULL\n");
				setOutputWhite();
				freeAnyLinkedList((void**) &change_head, PTR_TYPE_CHANGE_NODE_V2, NULL, malloced_head, the_debug);
				return -1;
			}
			else if (cur_actual_data != NULL && cur_exp_data != NULL)
			{
				if (cur_actual_data->ptr_type != cur_exp_data->ptr_type)
				{
					printf("test_Controller_parseInsert with id = %d FAILED\n", test_id);
					printf("cur_actual_data->ptr_type (%d) did not equal cur_exp_data->ptr_type (%d)\n", cur_actual_data->ptr_type, cur_exp_data->ptr_type);
					setOutputWhite();
					freeAnyLinkedList((void**) &change_head, PTR_TYPE_CHANGE_NODE_V2, NULL, malloced_head, the_debug);
					return -1;
				}

				if (cur_actual_data->ptr_type == PTR_TYPE_INT && *((int*) cur_actual_data->ptr_value) != *((int*) cur_exp_data->ptr_value))
				{
					printf("test_Controller_parseInsert with id = %d FAILED\n", test_id);
					printf("cur_actual_data->ptr_value %d did not equal below\n", *((int*) cur_actual_data->ptr_value));
					printf("cur_exp_data->ptr_value %d\n", *((int*) cur_exp_data->ptr_value));
					setOutputWhite();
					freeAnyLinkedList((void**) &change_head, PTR_TYPE_CHANGE_NODE_V2, NULL, malloced_head, the_debug);
					return -1;
				}
				else if (cur_actual_data->ptr_type == PTR_TYPE_REAL && *((double*) cur_actual_data->ptr_value) != *((double*) cur_exp_data->ptr_value))
				{
					printf("test_Controller_parseInsert with id = %d FAILED\n", test_id);
					printf("cur_actual_data->ptr_value %lf did not equal below\n", *((double*) cur_actual_data->ptr_value));
					printf("cur_exp_data->ptr_value %lf\n", *((double*) cur_exp_data->ptr_value));
					setOutputWhite();
					freeAnyLinkedList((void**) &change_head, PTR_TYPE_CHANGE_NODE_V2, NULL, malloced_head, the_debug);
					return -1;
				}
				else if (cur_actual_data->ptr_type == PTR_TYPE_CHAR || cur_actual_data->ptr_type == PTR_TYPE_DATE)
				{
					if (strcmp(cur_actual_data->ptr_value, cur_exp_data->ptr_value) != 0)
					{
						printf("test_Controller_parseInsert with id = %d FAILED\n", test_id);
						printf("cur_actual_data->ptr_value (%s) did not equal below\n", cur_actual_data->ptr_value);
						printf("cur_exp_data->ptr_value (%s)\n", cur_exp_data->ptr_value);
						setOutputWhite();
						freeAnyLinkedList((void**) &change_head, PTR_TYPE_CHANGE_NODE_V2, NULL, malloced_head, the_debug);
						return -1;
					}
				}
			}

			if (cur_actual_data != NULL)
				cur_actual_data = cur_actual_data->next;
			if (cur_exp_data != NULL)
				cur_exp_data = cur_exp_data->next;
		}
	// END Tests


	setOutputWhite();

	// START Free rest of change_head
		freeAnyLinkedList((void**) &change_head, PTR_TYPE_CHANGE_NODE_V2, NULL, malloced_head, the_debug);
	// END Free rest of change_head

	return 0;
}

int compSelectNodes(int test_id, struct select_node* act_select_node, struct select_node* exp_select_node)
{
	struct select_node* cur_actual_select = act_select_node;
	struct select_node* cur_expected_select = exp_select_node;

	int select_node_counter = 0;
	
	while (cur_actual_select != NULL || cur_expected_select != NULL)
	{
		//printf("1\n");
		// START Check if one is null
			if (cur_actual_select == NULL && cur_expected_select != NULL)
			{
				printf("test_Controller_parseSelect with id = %d FAILED\n", test_id);
				printf("cur_actual_select was NULL and cur_expected_select was NOT NULL\n");
				return -1;
			}
			else if (cur_actual_select != NULL && cur_expected_select == NULL)
			{
				printf("test_Controller_parseSelect with id = %d FAILED\n", test_id);
				printf("cur_actual_select was NOT NULL and cur_expected_select was NULL\n");
				return -1;
			}
		// END Check if one is null


		//printf("2\n");
		// START Check aliases
			if (cur_actual_select->select_node_alias == NULL && cur_expected_select->select_node_alias != NULL)
			{
				printf("test_Controller_parseSelect with id = %d FAILED\n", test_id);
				printf("cur actual select_node_alias was NULL and cur exp select_node_alias was NOT NULL\n");
				return -1;
			}
			else if (cur_actual_select->select_node_alias != NULL && cur_expected_select->select_node_alias == NULL)
			{
				printf("test_Controller_parseSelect with id = %d FAILED\n", test_id);
				printf("cur actual select_node_alias was NOT NULL and cur exp select_node_alias was NULL\n");
				return -1;
			}
			else if (cur_actual_select->select_node_alias != NULL 
					&& strcmp(cur_actual_select->select_node_alias, cur_expected_select->select_node_alias) != 0)
			{
				printf("test_Controller_parseSelect with id = %d FAILED\n", test_id);
				printf("Actual select_node_alias _%s_ did not equal below\n", cur_actual_select->select_node_alias);
				printf("Expected select_node_alias _%s_\n", cur_expected_select->select_node_alias);
				return -1;
			}
		// END Check aliases


		//printf("3\n");
		// START Check distinct
			if (cur_actual_select->distinct != cur_expected_select->distinct)
			{
				printf("test_Controller_parseSelect with id = %d FAILED\n", test_id);
				printf("Actual distinct _%s_ did not equal below\n", cur_actual_select->distinct ? "TRUE" : "FALSE");
				printf("Expected distinct _%s_\n", cur_expected_select->distinct ? "TRUE" : "FALSE");
				return -1;
			}
		// END Check distinct

		//printf("4\n");
		// START Check if columns match
			if (cur_actual_select->columns_arr_size != cur_expected_select->columns_arr_size)
			{
				printf("test_Controller_parseSelect with id = %d FAILED\n", test_id);
				printf("Actual columns_arr_size %d did not equal below\n", cur_actual_select->columns_arr_size);
				printf("Expected columns_arr_size %d\n", cur_expected_select->columns_arr_size);
				return -1;
			}

			for (int i=0; i<cur_actual_select->columns_arr_size; i++)
			{
				// START Check table ptr
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
						struct col_in_select_node* cur_1 = cur_actual_select->columns_arr[i];
						while (cur_1->table_ptr_type == PTR_TYPE_SELECT_NODE)
							cur_1 = cur_1->col_ptr;

						struct col_in_select_node* cur_2 = cur_expected_select->columns_arr[i];
						while (cur_2->table_ptr_type == PTR_TYPE_SELECT_NODE)
							cur_2 = cur_2->col_ptr;

						actual_table_ptr = cur_1->table_ptr;
						exp_table_ptr = cur_2->table_ptr;
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
						return -1;
					}

					//printf("7\n");
					if (cur_actual_select->columns_arr[i]->col_ptr_type != cur_expected_select->columns_arr[i]->col_ptr_type)
					{
						printf("test_Controller_parseSelect with id = %d FAILED\n", test_id);
						printf("Actual col_ptr_type %d did not equal below\n", cur_actual_select->columns_arr[i]->col_ptr_type);
						printf("Expected col_ptr_type %d\n", cur_expected_select->columns_arr[i]->col_ptr_type);
						return -1;
					}
				// END Check table ptr

				// START Check column ptr
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
							struct col_in_select_node* cur_1 = cur_actual_select->columns_arr[i];
							while (cur_1->table_ptr_type == PTR_TYPE_SELECT_NODE)
								cur_1 = cur_1->col_ptr;

							struct col_in_select_node* cur_2 = cur_expected_select->columns_arr[i];
							while (cur_2->table_ptr_type == PTR_TYPE_SELECT_NODE)
								cur_2 = cur_2->col_ptr;

							actual_col_ptr = cur_1->col_ptr;
							exp_col_ptr = cur_2->col_ptr;
						}

						if (actual_col_ptr != exp_col_ptr)
						{
							printf("test_Controller_parseSelect with id = %d FAILED\n", test_id);
							printf("select_node_counter = %d with alias = _%s_\n", select_node_counter, cur_expected_select->select_node_alias);
							if (actual_col_ptr == NULL)
								printf("Actual columns_arr[%d] col_ptr was NULL\n", i);
							else if (exp_col_ptr == NULL)
								printf("Expected columns_arr[%d] col_ptr was NULL\n", i);
							else
							{
								printf("Actual columns_arr[i] col_ptr (%s) did not equal expected columns_arr[i] col_ptr (%s) at i = %d\n"
									   ,((struct table_cols_info*) actual_col_ptr)->col_name, ((struct table_cols_info*) exp_col_ptr)->col_name, i);
							}
							return -1;
						}
					}
				// END Check column ptr
					

				// START Check column new name
					if (cur_actual_select->columns_arr[i]->new_name == NULL && cur_expected_select->columns_arr[i]->new_name != NULL)
					{
						printf("test_Controller_parseSelect with id = %d FAILED\n", test_id);
						printf("cur actual columns_arr[%d]->new_name was NULL and cur exp columns_arr[%d]->new_name was NOT NULL\n", i, i);
						return -1;
					}
					else if (cur_actual_select->columns_arr[i]->new_name != NULL && cur_expected_select->columns_arr[i]->new_name == NULL)
					{
						printf("test_Controller_parseSelect with id = %d FAILED\n", test_id);
						printf("cur actual columns_arr[%d]->new_name was NOT NULL and cur exp columns_arr[%d]->new_name was NULL\n", i, i);
						return -1;
					}
					else if (cur_actual_select->columns_arr[i]->new_name != NULL 
							&& strcmp(cur_actual_select->columns_arr[i]->new_name, cur_expected_select->columns_arr[i]->new_name) != 0)
					{
						printf("test_Controller_parseSelect with id = %d FAILED\n", test_id);
						printf("Actual columns_arr[%d]->new_name _%s_ did not equal below\n", i, cur_actual_select->columns_arr[i]->new_name);
						printf("Expected columns_arr[%d]->new_name _%s_\n", i, cur_expected_select->columns_arr[i]->new_name);
						return -1;
					}
				// END Check column new name

				// START Check column func node
					if (cur_actual_select->columns_arr[i]->func_node == NULL && cur_expected_select->columns_arr[i]->func_node != NULL)
					{
						printf("test_Controller_parseSelect with id = %d FAILED\n", test_id);
						printf("cur actual columns_arr[%d]->func_node was NULL and cur exp columns_arr[%d]->func_node was NOT NULL\n", i, i);
						return -1;
					}
					else if (cur_actual_select->columns_arr[i]->func_node != NULL && cur_expected_select->columns_arr[i]->func_node == NULL)
					{
						printf("test_Controller_parseSelect with id = %d FAILED\n", test_id);
						printf("cur actual columns_arr[%d]->func_node was NOT NULL and cur exp columns_arr[%d]->func_node was NULL\n", i, i);
						return -1;
					}
					else if (cur_actual_select->columns_arr[i]->func_node != NULL && cur_expected_select->columns_arr[i]->func_node != NULL)
					{
						if (compFuncNodes(test_id, cur_actual_select->columns_arr[i]->func_node, cur_expected_select->columns_arr[i]->func_node) != 0)
							return -1;
					}
				// END Check column func node

				// START Check column math node
					if (cur_actual_select->columns_arr[i]->math_node == NULL && cur_expected_select->columns_arr[i]->math_node != NULL)
					{
						printf("test_Controller_parseSelect with id = %d FAILED\n", test_id);
						printf("cur actual columns_arr[%d]->math_node was NULL and cur exp columns_arr[%d]->math_node was NOT NULL\n", i, i);
						return -1;
					}
					else if (cur_actual_select->columns_arr[i]->math_node != NULL && cur_expected_select->columns_arr[i]->math_node == NULL)
					{
						printf("test_Controller_parseSelect with id = %d FAILED\n", test_id);
						printf("cur actual columns_arr[%d]->math_node was NOT NULL and cur exp columns_arr[%d]->math_node was NULL\n", i, i);
						return -1;
					}
					else if (cur_actual_select->columns_arr[i]->math_node != NULL && cur_expected_select->columns_arr[i]->math_node != NULL)
					{
						if (compMathOrWhereTree(test_id, PTR_TYPE_MATH_NODE, cur_actual_select->columns_arr[i]->math_node, cur_expected_select->columns_arr[i]->math_node) != 0)
						{
							return -1;
						}
					}
				// END Check column math node

				// START Check column case node
					if (cur_actual_select->columns_arr[i]->case_node == NULL && cur_expected_select->columns_arr[i]->case_node != NULL)
					{
						printf("test_Controller_parseSelect with id = %d FAILED\n", test_id);
						printf("cur actual columns_arr[%d]->case_node was NULL and cur exp columns_arr[%d]->case_node was NOT NULL\n", i, i);
						return -1;
					}
					else if (cur_actual_select->columns_arr[i]->case_node != NULL && cur_expected_select->columns_arr[i]->case_node == NULL)
					{
						printf("test_Controller_parseSelect with id = %d FAILED\n", test_id);
						printf("cur actual columns_arr[%d]->case_node was NOT NULL and cur exp columns_arr[%d]->case_node was NULL\n", i, i);
						return -1;
					}
					else if (cur_actual_select->columns_arr[i]->case_node != NULL && cur_expected_select->columns_arr[i]->case_node != NULL)
					{
						// START Check case_node case_when_head
							struct ListNodePtr* actual_cur_when = cur_actual_select->columns_arr[i]->case_node->case_when_head;
							struct ListNodePtr* expected_cur_when = cur_expected_select->columns_arr[i]->case_node->case_when_head;
							while (actual_cur_when != NULL || expected_cur_when != NULL)
							{
								if (actual_cur_when == NULL && expected_cur_when != NULL)
								{
									printf("test_Controller_parseSelect with id = %d FAILED\n", test_id);
									printf("actual_cur_when was NULL and expected_cur_when was NOT NULL\n");
									printf("failed at columns_arr index = %d\n", i);
									return -1;
								}
								else if (actual_cur_when != NULL && expected_cur_when == NULL)
								{
									printf("test_Controller_parseSelect with id = %d FAILED\n", test_id);
									printf("actual_cur_when was NOT NULL and expected_cur_when was NULL\n");
									printf("failed at columns_arr index = %d\n", i);
									return -1;
								}
								else if (actual_cur_when != NULL && expected_cur_when != NULL)
								{
									if (actual_cur_when->ptr_value == NULL && expected_cur_when->ptr_value != NULL)
									{
										printf("test_Controller_parseSelect with id = %d FAILED\n", test_id);
										printf("actual_cur_when->ptr_value was NULL and expected_cur_when->ptr_value was NOT NULL\n");
										printf("failed at columns_arr index = %d\n", i);
										return -1;
									}
									else if (actual_cur_when->ptr_value != NULL && expected_cur_when->ptr_value == NULL)
									{
										printf("test_Controller_parseSelect with id = %d FAILED\n", test_id);
										printf("actual_cur_when->ptr_value was NOT NULL and expected_cur_when->ptr_value was NULL\n");
										printf("failed at columns_arr index = %d\n", i);
										return -1;
									}
									else if (actual_cur_when->ptr_value != NULL && expected_cur_when->ptr_value != NULL)
									{
										if (actual_cur_when->ptr_type != expected_cur_when->ptr_type)
										{
											printf("test_Controller_parseSelect with id = %d FAILED\n", test_id);
											printf("actual_cur_when->ptr_type (%d) did not equal expected_cur_when->ptr_type (%d)\n", actual_cur_when->ptr_type, expected_cur_when->ptr_type);
											printf("failed at columns_arr index = %d\n", i);
											return -1;
										}

										if (compMathOrWhereTree(test_id, PTR_TYPE_WHERE_CLAUSE_NODE, actual_cur_when->ptr_value, expected_cur_when->ptr_value) != 0)
										{
											return -1;
										}
									}
								}

								if (actual_cur_when->next == NULL && expected_cur_when->next == NULL)
									break;

								if (actual_cur_when->next != NULL)
									actual_cur_when = actual_cur_when->next;
								if (expected_cur_when->next != NULL)
									expected_cur_when = expected_cur_when->next;
							}
						// END Check case_node case_when_head

						// START Check case_node case_then_value_head
							struct ListNodePtr* actual_cur_then = cur_actual_select->columns_arr[i]->case_node->case_then_value_head;
							struct ListNodePtr* expected_cur_then = cur_expected_select->columns_arr[i]->case_node->case_then_value_head;
							while (actual_cur_then != NULL || expected_cur_then != NULL)
							{
								if (actual_cur_then == NULL && expected_cur_then != NULL)
								{
									printf("test_Controller_parseSelect with id = %d FAILED\n", test_id);
									printf("actual_cur_then was NULL and expected_cur_then was NOT NULL\n");
									printf("failed at columns_arr index = %d\n", i);
									return -1;
								}
								else if (actual_cur_then != NULL && expected_cur_then == NULL)
								{
									printf("test_Controller_parseSelect with id = %d FAILED\n", test_id);
									printf("actual_cur_then was NOT NULL and expected_cur_then was NULL\n");
									printf("failed at columns_arr index = %d\n", i);
									return -1;
								}
								else if (actual_cur_then != NULL && expected_cur_then != NULL)
								{
									if (actual_cur_then->ptr_value == NULL && expected_cur_then->ptr_value != NULL)
									{
										printf("test_Controller_parseSelect with id = %d FAILED\n", test_id);
										printf("actual_cur_then->ptr_value was NULL and expected_cur_then->ptr_value was NOT NULL\n");
										printf("failed at columns_arr index = %d\n", i);
										return -1;
									}
									else if (actual_cur_then->ptr_value != NULL && expected_cur_then->ptr_value == NULL)
									{
										printf("test_Controller_parseSelect with id = %d FAILED\n", test_id);
										printf("actual_cur_then->ptr_value was NOT NULL and expected_cur_then->ptr_value was NULL\n");
										printf("failed at columns_arr index = %d\n", i);
										return -1;
									}
									else if (actual_cur_then->ptr_value != NULL && expected_cur_then->ptr_value != NULL)
									{
										if (actual_cur_then->ptr_type != expected_cur_then->ptr_type)
										{
											printf("test_Controller_parseSelect with id = %d FAILED\n", test_id);
											printf("actual_cur_then->ptr_type (%d) did not equal expected_cur_then->ptr_type (%d)\n", actual_cur_then->ptr_type, expected_cur_then->ptr_type);
											printf("failed at columns_arr index = %d\n", i);
											return -1;
										}

										if (actual_cur_then->ptr_type == PTR_TYPE_INT)
										{
											if (*((int*) actual_cur_then->ptr_value) != *((int*) expected_cur_then->ptr_value))
											{
												printf("test_Controller_parseSelect with id = %d FAILED\n", test_id);
												printf("actual_cur_then->ptr_value (%d) did not equal expected_cur_then->ptr_value (%d)\n", *((int*) actual_cur_then->ptr_value), *((int*) expected_cur_then->ptr_value));
												printf("failed at columns_arr index = %d\n", i);
												return -1;
											}
										}
										else if (actual_cur_then->ptr_type == PTR_TYPE_REAL)
										{
											if (*((double*) actual_cur_then->ptr_value) != *((double*) expected_cur_then->ptr_value))
											{
												printf("compMathOrWhereTree with id = %d FAILED\n", test_id);
												printf("actual_cur_then->ptr_value (%lf) did not equal expected_cur_then->ptr_value (%lf)\n", *((double*) actual_cur_then->ptr_value), *((double*) expected_cur_then->ptr_value));
												printf("failed at columns_arr index = %d\n", i);
												return -1;
											}
										}
										else if (actual_cur_then->ptr_type == PTR_TYPE_CHAR || actual_cur_then->ptr_type == PTR_TYPE_DATE)
										{
											if (strcmp(actual_cur_then->ptr_value, expected_cur_then->ptr_value) != 0)
											{
												printf("test_Controller_parseSelect with id = %d FAILED\n", test_id);
												printf("actual_cur_then->ptr_value (%s) did not equal expected_cur_then->ptr_value (%s)\n", actual_cur_then->ptr_value, expected_cur_then->ptr_value);
												printf("failed at columns_arr index = %d\n", i);
												return -1;
											}
										}
										else if (actual_cur_then->ptr_type == PTR_TYPE_MATH_NODE)
										{
											if (compMathOrWhereTree(test_id, PTR_TYPE_MATH_NODE, actual_cur_then->ptr_value, expected_cur_then->ptr_value) != 0)
											{
												return -1;
											}
										}
									}
								}

								if (actual_cur_then->next == NULL && expected_cur_then->next == NULL)
									break;

								if (actual_cur_then->next != NULL)
									actual_cur_then = actual_cur_then->next;
								if (expected_cur_then->next != NULL)
									expected_cur_then = expected_cur_then->next;
							}
						// END Check case_node case_then_value_head
					}
				// END Check column case node
			}
		// END Check if columns match

		// START Check if join nodes match
			struct join_node* cur_actual_join = cur_actual_select->join_head;
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


					if (compSelectNodes(test_id, cur_actual_join->select_joined, cur_expected_join->select_joined) != 0)
					{
						return -1;
					}


					if (compMathOrWhereTree(test_id, PTR_TYPE_WHERE_CLAUSE_NODE, cur_actual_join->on_clause_head, cur_expected_join->on_clause_head) != 0)
					{
						return -1;
					}
				}

				if (cur_actual_join != NULL)
					cur_actual_join = cur_actual_join->next;
				if (cur_expected_join != NULL)
					cur_expected_join = cur_expected_join->next;
			}
		// END Check if join nodes match

		// START Check if order by nodes match
			if (cur_actual_select->order_by == NULL && cur_expected_select->order_by != NULL)
			{
				printf("test_Controller_parseSelect with id = %d FAILED\n", test_id);
				printf("cur_actual_select->order_by was NULL and cur_expected_select->order_by was NOT NULL\n");
				return -1;
			}
			else if (cur_actual_select->order_by != NULL && cur_expected_select->order_by == NULL)
			{
				printf("test_Controller_parseSelect with id = %d FAILED\n", test_id);
				printf("cur_actual_select->order_by was NOT NULL and cur_expected_select->order_by was NULL\n");
				return -1;
			}
			else if (cur_actual_select->order_by != NULL && cur_expected_select->order_by != NULL)
			{
				if (cur_actual_select->order_by->order_by_cols_head == NULL & cur_expected_select->order_by->order_by_cols_head != NULL)
				{
					printf("test_Controller_parseSelect with id = %d FAILED\n", test_id);
					printf("cur_actual_select->order_by->order_by_cols_head was NULL and cur_expected_select->order_by->order_by_cols_head was NOT NULL\n");
					return -1;
				}
				else if (cur_actual_select->order_by->order_by_cols_head != NULL & cur_expected_select->order_by->order_by_cols_head == NULL)
				{
					printf("test_Controller_parseSelect with id = %d FAILED\n", test_id);
					printf("cur_actual_select->order_by->order_by_cols_head was NOT NULL and cur_expected_select->order_by->order_by_cols_head was NULL\n");
					return -1;
				}

				if (cur_actual_select->order_by->order_by_cols_which_head == NULL && cur_expected_select->order_by->order_by_cols_which_head != NULL)
				{
					printf("test_Controller_parseSelect with id = %d FAILED\n", test_id);
					printf("cur_actual_select->order_by->order_by_cols_which_head was NULL and cur_expected_select->order_by->order_by_cols_which_head was NOT NULL\n");
					return -1;
				}
				else if (cur_actual_select->order_by->order_by_cols_which_head != NULL && cur_expected_select->order_by->order_by_cols_which_head == NULL)
				{
					printf("test_Controller_parseSelect with id = %d FAILED\n", test_id);
					printf("cur_actual_select->order_by->order_by_cols_which_head was NOT NULL and cur_expected_select->order_by->order_by_cols_which_head was NULL\n");
					return -1;
				}

				struct ListNodePtr* cur_actual_order_col = cur_actual_select->order_by->order_by_cols_head;
				struct ListNodePtr* cur_actual_order_which = cur_actual_select->order_by->order_by_cols_which_head;
				struct ListNodePtr* cur_expected_order_col = cur_expected_select->order_by->order_by_cols_head;
				struct ListNodePtr* cur_expected_order_which = cur_expected_select->order_by->order_by_cols_which_head;
				while (cur_actual_order_col != NULL || cur_actual_order_which != NULL || cur_expected_order_col != NULL || cur_expected_order_which != NULL)
				{
					if (cur_actual_order_col == NULL && cur_actual_order_which != NULL)
					{
						printf("test_Controller_parseSelect with id = %d FAILED\n", test_id);
						printf("cur_actual_order_col was NULL and cur_actual_order_which was NOT NULL\n");
						return -1;
					}
					else if (cur_actual_order_col != NULL && cur_actual_order_which == NULL)
					{
						printf("test_Controller_parseSelect with id = %d FAILED\n", test_id);
						printf("cur_actual_order_col was NOT NULL and cur_actual_order_which was NULL\n");
						return -1;
					}
					else if (cur_actual_order_col == NULL && cur_expected_order_col != NULL)
					{
						printf("test_Controller_parseSelect with id = %d FAILED\n", test_id);
						printf("cur_actual_order_col was NULL and cur_expected_order_col was NOT NULL\n");
						return -1;
					}
					else if (cur_actual_order_col != NULL && cur_expected_order_col == NULL)
					{
						printf("test_Controller_parseSelect with id = %d FAILED\n", test_id);
						printf("cur_actual_order_col was NOT NULL and cur_expected_order_col was NULL\n");
						return -1;
					}
					else if (cur_actual_order_which == NULL && cur_expected_order_which != NULL)
					{
						printf("test_Controller_parseSelect with id = %d FAILED\n", test_id);
						printf("cur_actual_order_which was NULL and cur_expected_order_which was NOT NULL\n");
						return -1;
					}
					else if (cur_actual_order_which != NULL && cur_expected_order_which == NULL)
					{
						printf("test_Controller_parseSelect with id = %d FAILED\n", test_id);
						printf("cur_actual_order_which was NOT NULL and cur_expected_order_which was NULL\n");
						return -1;
					}
					else if (cur_actual_order_col != NULL && cur_expected_order_col != NULL && cur_actual_order_which != NULL && cur_expected_order_which != NULL)
					{
						// START Check order by column
							if (cur_actual_order_col->ptr_type != cur_expected_order_col->ptr_type)
							{
								printf("test_Controller_parseSelect with id = %d FAILED\n", test_id);
								printf("cur_actual_order_col->ptr_type (%d) did not equal below\n", cur_actual_order_col->ptr_type);
								printf("cur_expected_order_col->ptr_type (%d)\n", cur_expected_order_col->ptr_type);
								return -1;
							}

							void* act_ptr_value = NULL;
							void* exp_ptr_value = NULL;

							if (cur_actual_order_col->ptr_type == PTR_TYPE_COL_IN_SELECT_NODE)
							{
								struct col_in_select_node* cur_1 = ((struct col_in_select_node*) cur_actual_order_col->ptr_value);
								while (cur_1->table_ptr_type == PTR_TYPE_SELECT_NODE)
									cur_1 = cur_1->col_ptr;

								struct col_in_select_node* cur_2 = ((struct col_in_select_node*) cur_expected_order_col->ptr_value);
								while (cur_2->table_ptr_type == PTR_TYPE_SELECT_NODE)
									cur_2 = cur_2->col_ptr;

								act_ptr_value = cur_1->col_ptr;
								exp_ptr_value = cur_2->col_ptr;
							}

							if (act_ptr_value != exp_ptr_value)
							{
								printf("test_Controller_parseSelect with id = %d FAILED\n", test_id);
								if (act_ptr_value == NULL)
									printf("cur_actual_order_col->ptr_value was NULL\n");
								else if (exp_ptr_value == NULL)
									printf("cur_expected_order_col->ptr_value was NULL\n");
								else
								{
									printf("cur_actual_order_col->ptr_value (%s) did not equal cur_expected_order_col->ptr_value (%s)\n"
										   ,((struct table_cols_info*) act_ptr_value)->col_name, ((struct table_cols_info*) exp_ptr_value)->col_name);
								}
								return -1;
							}
						// END Check order by column

						// START Check order by which
							if (cur_actual_order_which->ptr_type != cur_expected_order_which->ptr_type)
							{
								printf("test_Controller_parseSelect with id = %d FAILED\n", test_id);
								printf("cur_actual_order_which->ptr_type (%d) did not equal below\n", cur_actual_order_which->ptr_type);
								printf("cur_expected_order_which->ptr_type (%d)\n", cur_expected_order_which->ptr_type);
								return -1;
							}

							if (*((int*) cur_actual_order_which->ptr_value) != *((int*) cur_expected_order_which->ptr_value))
							{
								printf("test_Controller_parseSelect with id = %d FAILED\n", test_id);
								printf("cur_actual_order_which->ptr_value (%d) did not equal below\n", *((int*) cur_actual_order_which->ptr_value));
								printf("cur_expected_order_which->ptr_value (%d)\n", *((int*) cur_expected_order_which->ptr_value));
								return -1;
							}
						// END Check order by which
					}

					if (cur_actual_order_col != NULL)
						cur_actual_order_col = cur_actual_order_col->next;
					if (cur_actual_order_which != NULL)
						cur_actual_order_which = cur_actual_order_which->next;
					if (cur_expected_order_col != NULL)
						cur_expected_order_col = cur_expected_order_col->next;
					if (cur_expected_order_which != NULL)
						cur_expected_order_which = cur_expected_order_which->next;
				}
			}
		// END Check if order by nodes match

		// START Check if where nodes match

		// END Check if where nodes match

		// START Check if having nodes match
			if (cur_actual_select->having_head == NULL && cur_expected_select->having_head != NULL)
			{
				printf("test_Controller_parseSelect with id = %d FAILED\n", test_id);
				printf("cur_actual_select->having_head was NULL and cur_expected_select->having_head was NOT NULL\n");
				return -1;
			}
			else if (cur_actual_select->having_head != NULL && cur_expected_select->having_head == NULL)
			{
				printf("test_Controller_parseSelect with id = %d FAILED\n", test_id);
				printf("cur_actual_select->having_head was NOT NULL and cur_expected_select->having_head was NULL\n");
				return -1;
			}
			else if (cur_actual_select->having_head != NULL && cur_expected_select->having_head != NULL)
			{
				if (compMathOrWhereTree(test_id, PTR_TYPE_WHERE_CLAUSE_NODE, cur_actual_select->having_head, cur_expected_select->having_head) != 0)
					return -1;
			}
		// END Check if having nodes match

		if (cur_actual_select != NULL)
			cur_actual_select = cur_actual_select->next;
		if (cur_expected_select	!= NULL)
			cur_expected_select = cur_expected_select->next;


		select_node_counter++;
	}

	return 0;
}

int test_Controller_parseSelect(int test_id, char* select_string, struct select_node** exp_select_node
							   ,int* parsed_error_code, struct malloced_node** malloced_head, int the_debug)
{
	setOutputGreen();
	printf("\nStarting test with id = %d\n", test_id);
	setOutputWhite();

	struct select_node* select_node = NULL;


	*parsed_error_code = parseSelect(select_string, &select_node, NULL, false, malloced_head, the_debug);
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


	setOutputRed();
	if (compSelectNodes(test_id, select_node, *exp_select_node) != 0)
	{
		freeAnyLinkedList((void**) &select_node, PTR_TYPE_SELECT_NODE, NULL, malloced_head, the_debug);
		setOutputWhite(); return -1;
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
	// START test_Controller_parseWhereClause
		// START Test with id = 101
			struct select_node* the_select_node = NULL;
			int parsed_error_code = 0;

			initSelectClauseForComp(&the_select_node, NULL, false, 0, NULL, getTablesHead(), PTR_TYPE_TABLE_INFO, NULL, NULL
								   ,&malloced_head, the_debug);
			initSelectClauseForComp(&the_select_node->next, NULL, false, 0, NULL, the_select_node, PTR_TYPE_SELECT_NODE, NULL, NULL
								   ,&malloced_head, the_debug);
			the_select_node->next->prev = the_select_node;


			struct where_clause_node* the_where_node = (struct where_clause_node*) myMalloc(sizeof(struct where_clause_node), NULL, &malloced_head, the_debug);

			the_where_node->ptr_one = myMalloc(sizeof(int), NULL, &malloced_head, the_debug);
			*((int*) the_where_node->ptr_one) = 1;
			the_where_node->ptr_one_type = PTR_TYPE_INT;

			the_where_node->ptr_two = myMalloc(sizeof(int), NULL, &malloced_head, the_debug);
			*((int*) the_where_node->ptr_two) = 1;
			the_where_node->ptr_two_type = PTR_TYPE_INT;

			the_where_node->where_type = WHERE_IS_EQUALS;
			the_where_node->parent = NULL;

			if (test_Controller_parseWhereClause(101, "where 1 = 1", "where"
												,&parsed_error_code, the_select_node->next, &the_where_node
												,&malloced_head, the_debug) != 0)
				return -1;

			if (parsed_error_code == 0)
			{
				freeAnyLinkedList((void**) &the_select_node, PTR_TYPE_SELECT_NODE, NULL, &malloced_head, the_debug);
				freeAnyLinkedList((void**) &the_where_node, PTR_TYPE_WHERE_CLAUSE_NODE, NULL, &malloced_head, the_debug);
			}

			if (malloced_head != NULL)
			{
				if (the_debug == YES_DEBUG)
				{
					setOutputRed();
					printf("	ERROR in test_Driver_main() at line %d in %s\n", __LINE__, __FILE__);
					setOutputWhite();
				}
				return -3;
			}
		// END Test with id = 101

		// START Test with id = 102
			the_select_node = NULL;
			initSelectClauseForComp(&the_select_node, NULL, false, 0, NULL, getTablesHead(), PTR_TYPE_TABLE_INFO, NULL, NULL
								   ,&malloced_head, the_debug);
			initSelectClauseForComp(&the_select_node->next, NULL, false, 0, NULL, the_select_node, PTR_TYPE_SELECT_NODE, NULL, NULL
								   ,&malloced_head, the_debug);
			the_select_node->next->prev = the_select_node;


			the_where_node = NULL;


			if (test_Controller_parseWhereClause(102, "where adaj jsfnoef = fiaenf'  '' ''';   ;", "where"
												,&parsed_error_code, the_select_node->next, &the_where_node
												,&malloced_head, the_debug) != 0)
				return -1;

			if (parsed_error_code == 0)
			{
				freeAnyLinkedList((void**) &the_select_node, PTR_TYPE_SELECT_NODE, NULL, &malloced_head, the_debug);
				freeAnyLinkedList((void**) &the_where_node, PTR_TYPE_WHERE_CLAUSE_NODE, NULL, &malloced_head, the_debug);
			}

			if (malloced_head != NULL)
			{
				if (the_debug == YES_DEBUG)
				{
					setOutputRed();
					printf("	ERROR in test_Driver_main() at line %d in %s\n", __LINE__, __FILE__);
					setOutputWhite();
				}
				return -3;
			}
		// END Test with id = 102

		// START Test with id = 103
			the_select_node = NULL;
			initSelectClauseForComp(&the_select_node, NULL, false, 0, NULL, getTablesHead(), PTR_TYPE_TABLE_INFO, NULL, NULL
								   ,&malloced_head, the_debug);
			initSelectClauseForComp(&the_select_node->next, NULL, false, 0, NULL, the_select_node, PTR_TYPE_SELECT_NODE, NULL, NULL
								   ,&malloced_head, the_debug);
			the_select_node->next->prev = the_select_node;


			the_where_node = NULL;


			if (test_Controller_parseWhereClause(103, "where BRAND-NAME;", "where"
												,&parsed_error_code, the_select_node->next, &the_where_node
												,&malloced_head, the_debug) != 0)
				return -1;


			if (parsed_error_code == 0)
			{
				freeAnyLinkedList((void**) &the_select_node, PTR_TYPE_SELECT_NODE, NULL, &malloced_head, the_debug);
				freeAnyLinkedList((void**) &the_where_node, PTR_TYPE_WHERE_CLAUSE_NODE, NULL, &malloced_head, the_debug);
			}

			if (malloced_head != NULL)
			{
				if (the_debug == YES_DEBUG)
				{
					setOutputRed();
					printf("	ERROR in test_Driver_main() at line %d in %s\n", __LINE__, __FILE__);
					setOutputWhite();
				}
				return -3;
			}
		// END Test with id = 103

		// START Test with id = 104
			the_select_node = NULL;
			initSelectClauseForComp(&the_select_node, NULL, false, 0, NULL, getTablesHead(), PTR_TYPE_TABLE_INFO, NULL, NULL
								   ,&malloced_head, the_debug);
			initSelectClauseForComp(&the_select_node->next, NULL, false, 0, NULL, the_select_node, PTR_TYPE_SELECT_NODE, NULL, NULL
								   ,&malloced_head, the_debug);
			the_select_node->next->prev = the_select_node;


			the_where_node = (struct where_clause_node*) myMalloc(sizeof(struct where_clause_node), NULL, &malloced_head, the_debug);

			the_where_node->ptr_one = ((struct select_node*) the_select_node)->columns_arr[0];
			the_where_node->ptr_one_type = PTR_TYPE_COL_IN_SELECT_NODE;

			the_where_node->ptr_two = myMalloc(sizeof(char) * 16, NULL, &malloced_head, the_debug);
			strcpy(the_where_node->ptr_two, "test");
			the_where_node->ptr_two_type = PTR_TYPE_CHAR;

			the_where_node->where_type = WHERE_IS_EQUALS;
			the_where_node->parent = NULL;


			if (test_Controller_parseWhereClause(104, "WherE braND-name = 'test';", "where"
												,&parsed_error_code, the_select_node->next, &the_where_node
												,&malloced_head, the_debug) != 0)
				return -1;
			

			if (parsed_error_code == 0)
			{
				freeAnyLinkedList((void**) &the_select_node, PTR_TYPE_SELECT_NODE, NULL, &malloced_head, the_debug);
				freeAnyLinkedList((void**) &the_where_node, PTR_TYPE_WHERE_CLAUSE_NODE, NULL, &malloced_head, the_debug);
			}

			if (malloced_head != NULL)
			{
				if (the_debug == YES_DEBUG)
				{
					setOutputRed();
					printf("	ERROR in test_Driver_main() at line %d in %s\n", __LINE__, __FILE__);
					setOutputWhite();
				}
				return -3;
			}
		// END Test with id = 104

		// START Test with id = 105
			the_select_node = NULL;
			initSelectClauseForComp(&the_select_node, NULL, false, 0, NULL, getTablesHead(), PTR_TYPE_TABLE_INFO, NULL, NULL
								   ,&malloced_head, the_debug);
			initSelectClauseForComp(&the_select_node->next, NULL, false, 0, NULL, the_select_node, PTR_TYPE_SELECT_NODE, NULL, NULL
								   ,&malloced_head, the_debug);
			the_select_node->next->prev = the_select_node;


			the_where_node = (struct where_clause_node*) myMalloc(sizeof(struct where_clause_node), NULL, &malloced_head, the_debug);

			the_where_node->ptr_one = ((struct select_node*) the_select_node)->columns_arr[0];
			the_where_node->ptr_one_type = PTR_TYPE_COL_IN_SELECT_NODE;

			the_where_node->ptr_two = NULL;
			the_where_node->ptr_two_type = -1;

			the_where_node->where_type = WHERE_IS_NULL;
			the_where_node->parent = NULL;


			if (test_Controller_parseWhereClause(105, "WherE braND-name is null;", "where"
												,&parsed_error_code, the_select_node->next, &the_where_node
												,&malloced_head, the_debug) != 0)
				return -1;
			

			if (parsed_error_code == 0)
			{
				freeAnyLinkedList((void**) &the_select_node, PTR_TYPE_SELECT_NODE, NULL, &malloced_head, the_debug);
				freeAnyLinkedList((void**) &the_where_node, PTR_TYPE_WHERE_CLAUSE_NODE, NULL, &malloced_head, the_debug);
			}

			if (malloced_head != NULL)
			{
				if (the_debug == YES_DEBUG)
				{
					setOutputRed();
					printf("	ERROR in test_Driver_main() at line %d in %s\n", __LINE__, __FILE__);
					setOutputWhite();
				}
				return -3;
			}
		// END Test with id = 105

		// START Test with id = 106
			the_select_node = NULL;
			initSelectClauseForComp(&the_select_node, "TBL", false, 0, NULL, getTablesHead(), PTR_TYPE_TABLE_INFO, NULL, NULL
								   ,&malloced_head, the_debug);
			initSelectClauseForComp(&the_select_node->next, NULL, false, 0, NULL, the_select_node, PTR_TYPE_SELECT_NODE, NULL, NULL
								   ,&malloced_head, the_debug);
			the_select_node->next->prev = the_select_node;


			the_where_node = (struct where_clause_node*) myMalloc(sizeof(struct where_clause_node), NULL, &malloced_head, the_debug);

			the_where_node->ptr_one = ((struct select_node*) the_select_node)->columns_arr[0];
			the_where_node->ptr_one_type = PTR_TYPE_COL_IN_SELECT_NODE;

			the_where_node->ptr_two = NULL;
			the_where_node->ptr_two_type = -1;

			the_where_node->where_type = WHERE_IS_NOT_NULL;
			the_where_node->parent = NULL;


			if (test_Controller_parseWhereClause(106, "WherE tbl.braND-name is not null;", "where"
												,&parsed_error_code, the_select_node->next, &the_where_node
												,&malloced_head, the_debug) != 0)
				return -1;
			

			if (parsed_error_code == 0)
			{
				freeAnyLinkedList((void**) &the_select_node, PTR_TYPE_SELECT_NODE, NULL, &malloced_head, the_debug);
				freeAnyLinkedList((void**) &the_where_node, PTR_TYPE_WHERE_CLAUSE_NODE, NULL, &malloced_head, the_debug);
			}

			if (malloced_head != NULL)
			{
				if (the_debug == YES_DEBUG)
				{
					setOutputRed();
					printf("	ERROR in test_Driver_main() at line %d in %s\n", __LINE__, __FILE__);
					setOutputWhite();
				}
				return -3;
			}
		// END Test with id = 106

		// START Test with id = 107
			the_select_node = NULL;
			initSelectClauseForComp(&the_select_node, NULL, false, 0, NULL, getTablesHead(), PTR_TYPE_TABLE_INFO, NULL, NULL
								   ,&malloced_head, the_debug);
			initSelectClauseForComp(&the_select_node->next, NULL, false, 0, NULL, the_select_node, PTR_TYPE_SELECT_NODE, NULL, NULL
								   ,&malloced_head, the_debug);
			the_select_node->next->prev = the_select_node;


			the_where_node = (struct where_clause_node*) myMalloc(sizeof(struct where_clause_node), NULL, &malloced_head, the_debug);

			the_where_node->ptr_one = ((struct select_node*) the_select_node)->columns_arr[1];
			the_where_node->ptr_one_type = PTR_TYPE_COL_IN_SELECT_NODE;

			the_where_node->ptr_two = myMalloc(sizeof(int), NULL, &malloced_head, the_debug);
			*((int*) the_where_node->ptr_two) = 1;
			the_where_node->ptr_two_type = PTR_TYPE_INT;

			the_where_node->where_type = WHERE_GREATER_THAN;
			the_where_node->parent = NULL;


			if (test_Controller_parseWhereClause(107, "WherE CT-REGISTRATION-NUMBER > 1;", "where"
												,&parsed_error_code, the_select_node->next, &the_where_node
												,&malloced_head, the_debug) != 0)
				return -1;
			

			if (parsed_error_code == 0)
			{
				freeAnyLinkedList((void**) &the_select_node, PTR_TYPE_SELECT_NODE, NULL, &malloced_head, the_debug);
				freeAnyLinkedList((void**) &the_where_node, PTR_TYPE_WHERE_CLAUSE_NODE, NULL, &malloced_head, the_debug);
			}

			if (malloced_head != NULL)
			{
				if (the_debug == YES_DEBUG)
				{
					setOutputRed();
					printf("	ERROR in test_Driver_main() at line %d in %s\n", __LINE__, __FILE__);
					setOutputWhite();
				}
				return -3;
			}
		// END Test with id = 107

		// START Test with id = 108
			the_select_node = NULL;
			initSelectClauseForComp(&the_select_node, NULL, false, 0, NULL, getTablesHead(), PTR_TYPE_TABLE_INFO, NULL, NULL
								   ,&malloced_head, the_debug);
			initSelectClauseForComp(&the_select_node->next, NULL, false, 0, NULL, the_select_node, PTR_TYPE_SELECT_NODE, NULL, NULL
								   ,&malloced_head, the_debug);
			the_select_node->next->prev = the_select_node;


			the_where_node = (struct where_clause_node*) myMalloc(sizeof(struct where_clause_node), NULL, &malloced_head, the_debug);

			the_where_node->ptr_one = ((struct select_node*) the_select_node)->columns_arr[1];
			the_where_node->ptr_one_type = PTR_TYPE_COL_IN_SELECT_NODE;

			the_where_node->ptr_two = myMalloc(sizeof(int), NULL, &malloced_head, the_debug);
			*((int*) the_where_node->ptr_two) = 1;
			the_where_node->ptr_two_type = PTR_TYPE_INT;

			the_where_node->where_type = WHERE_GREATER_THAN_OR_EQUAL;
			the_where_node->parent = NULL;


			if (test_Controller_parseWhereClause(108, "WherE CT-REGISTRATION-NUMBER >= 1;", "where"
												,&parsed_error_code, the_select_node->next, &the_where_node
												,&malloced_head, the_debug) != 0)
				return -1;
			

			if (parsed_error_code == 0)
			{
				freeAnyLinkedList((void**) &the_select_node, PTR_TYPE_SELECT_NODE, NULL, &malloced_head, the_debug);
				freeAnyLinkedList((void**) &the_where_node, PTR_TYPE_WHERE_CLAUSE_NODE, NULL, &malloced_head, the_debug);
			}

			if (malloced_head != NULL)
			{
				if (the_debug == YES_DEBUG)
				{
					setOutputRed();
					printf("	ERROR in test_Driver_main() at line %d in %s\n", __LINE__, __FILE__);
					setOutputWhite();
				}
				return -3;
			}
		// END Test with id = 108

		// START Test with id = 109
			the_select_node = NULL;
			initSelectClauseForComp(&the_select_node, NULL, false, 0, NULL, getTablesHead(), PTR_TYPE_TABLE_INFO, NULL, NULL
								   ,&malloced_head, the_debug);
			initSelectClauseForComp(&the_select_node->next, NULL, false, 0, NULL, the_select_node, PTR_TYPE_SELECT_NODE, NULL, NULL
								   ,&malloced_head, the_debug);
			the_select_node->next->prev = the_select_node;


			the_where_node = (struct where_clause_node*) myMalloc(sizeof(struct where_clause_node), NULL, &malloced_head, the_debug);

			the_where_node->ptr_one = ((struct select_node*) the_select_node)->columns_arr[1];
			the_where_node->ptr_one_type = PTR_TYPE_COL_IN_SELECT_NODE;

			the_where_node->ptr_two = myMalloc(sizeof(int), NULL, &malloced_head, the_debug);
			*((int*) the_where_node->ptr_two) = 1;
			the_where_node->ptr_two_type = PTR_TYPE_INT;

			the_where_node->where_type = WHERE_LESS_THAN_OR_EQUAL;
			the_where_node->parent = NULL;


			if (test_Controller_parseWhereClause(109, "WherE CT-REGISTRATION-NUMBER <= 1;", "where"
												,&parsed_error_code, the_select_node->next, &the_where_node
												,&malloced_head, the_debug) != 0)
				return -1;
			

			if (parsed_error_code == 0)
			{
				freeAnyLinkedList((void**) &the_select_node, PTR_TYPE_SELECT_NODE, NULL, &malloced_head, the_debug);
				freeAnyLinkedList((void**) &the_where_node, PTR_TYPE_WHERE_CLAUSE_NODE, NULL, &malloced_head, the_debug);
			}

			if (malloced_head != NULL)
			{
				if (the_debug == YES_DEBUG)
				{
					setOutputRed();
					printf("	ERROR in test_Driver_main() at line %d in %s\n", __LINE__, __FILE__);
					setOutputWhite();
				}
				return -3;
			}
		// END Test with id = 109

		// START Test with id = 110
			the_select_node = NULL;
			initSelectClauseForComp(&the_select_node, NULL, false, 0, NULL, getTablesHead(), PTR_TYPE_TABLE_INFO, NULL, NULL
								   ,&malloced_head, the_debug);
			initSelectClauseForComp(&the_select_node->next, NULL, false, 0, NULL, the_select_node, PTR_TYPE_SELECT_NODE, NULL, NULL
								   ,&malloced_head, the_debug);
			the_select_node->next->prev = the_select_node;


			the_where_node = NULL;


			if (test_Controller_parseWhereClause(110, "WherE BRAND-NAME <> 1;", "where"
												,&parsed_error_code, the_select_node->next, &the_where_node
												,&malloced_head, the_debug) != 0)
				return -1;
			

			if (parsed_error_code == 0)
			{
				freeAnyLinkedList((void**) &the_select_node, PTR_TYPE_SELECT_NODE, NULL, &malloced_head, the_debug);
				freeAnyLinkedList((void**) &the_where_node, PTR_TYPE_WHERE_CLAUSE_NODE, NULL, &malloced_head, the_debug);
			}

			if (malloced_head != NULL)
			{
				if (the_debug == YES_DEBUG)
				{
					setOutputRed();
					printf("	ERROR in test_Driver_main() at line %d in %s\n", __LINE__, __FILE__);
					setOutputWhite();
				}
				return -3;
			}
		// END Test with id = 110

		// START Test with id = 111
			the_select_node = NULL;
			initSelectClauseForComp(&the_select_node, NULL, false, 0, NULL, getTablesHead(), PTR_TYPE_TABLE_INFO, NULL, NULL
								   ,&malloced_head, the_debug);
			initSelectClauseForComp(&the_select_node->next, NULL, false, 0, NULL, the_select_node, PTR_TYPE_SELECT_NODE, NULL, NULL
								   ,&malloced_head, the_debug);
			the_select_node->next->prev = the_select_node;


			the_where_node = (struct where_clause_node*) myMalloc(sizeof(struct where_clause_node), NULL, &malloced_head, the_debug);

			the_where_node->ptr_one = (struct where_clause_node*) myMalloc(sizeof(struct where_clause_node), NULL, &malloced_head, the_debug);
			the_where_node->ptr_one_type = PTR_TYPE_WHERE_CLAUSE_NODE;

			the_where_node->ptr_two = (struct where_clause_node*) myMalloc(sizeof(struct where_clause_node), NULL, &malloced_head, the_debug);
			the_where_node->ptr_two_type = PTR_TYPE_WHERE_CLAUSE_NODE;

			the_where_node->where_type = WHERE_AND;
			the_where_node->parent = NULL;

				((struct where_clause_node*) the_where_node->ptr_one)->ptr_one = ((struct select_node*) the_select_node)->columns_arr[0];
				((struct where_clause_node*) the_where_node->ptr_one)->ptr_one_type = PTR_TYPE_COL_IN_SELECT_NODE;

				((struct where_clause_node*) the_where_node->ptr_one)->ptr_two = myMalloc(sizeof(char) * 16, NULL, &malloced_head, the_debug);
				strcpy(((struct where_clause_node*) the_where_node->ptr_one)->ptr_two, "test");
				((struct where_clause_node*) the_where_node->ptr_one)->ptr_two_type = PTR_TYPE_CHAR;

				((struct where_clause_node*) the_where_node->ptr_one)->where_type = WHERE_IS_EQUALS;
				((struct where_clause_node*) the_where_node->ptr_one)->parent = the_where_node;


				((struct where_clause_node*) the_where_node->ptr_two)->ptr_one = ((struct select_node*) the_select_node)->columns_arr[2];
				((struct where_clause_node*) the_where_node->ptr_two)->ptr_one_type = PTR_TYPE_COL_IN_SELECT_NODE;

				((struct where_clause_node*) the_where_node->ptr_two)->ptr_two = myMalloc(sizeof(char) * 16, NULL, &malloced_head, the_debug);
				strcpy(((struct where_clause_node*) the_where_node->ptr_two)->ptr_two, "That");
				((struct where_clause_node*) the_where_node->ptr_two)->ptr_two_type = PTR_TYPE_CHAR;

				((struct where_clause_node*) the_where_node->ptr_two)->where_type = WHERE_IS_EQUALS;
				((struct where_clause_node*) the_where_node->ptr_two)->parent = the_where_node;


			if (test_Controller_parseWhereClause(111, "WherE braND-name = 'test' aNd status = 'That' \n ;", "where"
												,&parsed_error_code, the_select_node->next, &the_where_node
												,&malloced_head, the_debug) != 0)
				return -1;


			if (parsed_error_code == 0)
			{
				freeAnyLinkedList((void**) &the_select_node, PTR_TYPE_SELECT_NODE, NULL, &malloced_head, the_debug);
				freeAnyLinkedList((void**) &the_where_node, PTR_TYPE_WHERE_CLAUSE_NODE, NULL, &malloced_head, the_debug);
			}

			if (malloced_head != NULL)
			{
				if (the_debug == YES_DEBUG)
				{
					setOutputRed();
					printf("	ERROR in test_Driver_main() at line %d in %s\n", __LINE__, __FILE__);
					setOutputWhite();
				}
				return -3;
			}
		// END Test with id = 111

		// START Test with id = 112
			the_select_node = NULL;
			initSelectClauseForComp(&the_select_node, NULL, false, 0, NULL, getTablesHead(), PTR_TYPE_TABLE_INFO, NULL, NULL
								   ,&malloced_head, the_debug);
			initSelectClauseForComp(&the_select_node->next, NULL, false, 0, NULL, the_select_node, PTR_TYPE_SELECT_NODE, NULL, NULL
								   ,&malloced_head, the_debug);
			the_select_node->next->prev = the_select_node;


			the_where_node = (struct where_clause_node*) myMalloc(sizeof(struct where_clause_node), NULL, &malloced_head, the_debug);

			the_where_node->ptr_one = (struct where_clause_node*) myMalloc(sizeof(struct where_clause_node), NULL, &malloced_head, the_debug);
			the_where_node->ptr_one_type = PTR_TYPE_WHERE_CLAUSE_NODE;

			the_where_node->ptr_two = (struct where_clause_node*) myMalloc(sizeof(struct where_clause_node), NULL, &malloced_head, the_debug);
			the_where_node->ptr_two_type = PTR_TYPE_WHERE_CLAUSE_NODE;

			the_where_node->where_type = WHERE_AND;
			the_where_node->parent = NULL;

				((struct where_clause_node*) the_where_node->ptr_one)->ptr_one = (struct where_clause_node*) myMalloc(sizeof(struct where_clause_node), NULL, &malloced_head, the_debug);
				((struct where_clause_node*) the_where_node->ptr_one)->ptr_one_type = PTR_TYPE_WHERE_CLAUSE_NODE;

				((struct where_clause_node*) the_where_node->ptr_one)->ptr_two = (struct where_clause_node*) myMalloc(sizeof(struct where_clause_node), NULL, &malloced_head, the_debug);
				((struct where_clause_node*) the_where_node->ptr_one)->ptr_two_type = PTR_TYPE_WHERE_CLAUSE_NODE;

				((struct where_clause_node*) the_where_node->ptr_one)->where_type = WHERE_OR;
				((struct where_clause_node*) the_where_node->ptr_one)->parent = the_where_node;

					((struct where_clause_node*) ((struct where_clause_node*) the_where_node->ptr_one)->ptr_one)->ptr_one = myMalloc(sizeof(int), NULL, &malloced_head, the_debug);
					*((int*) ((struct where_clause_node*) ((struct where_clause_node*) the_where_node->ptr_one)->ptr_one)->ptr_one) = 1;
					((struct where_clause_node*) ((struct where_clause_node*) the_where_node->ptr_one)->ptr_one)->ptr_one_type = PTR_TYPE_INT;

					((struct where_clause_node*) ((struct where_clause_node*) the_where_node->ptr_one)->ptr_one)->ptr_two = myMalloc(sizeof(int), NULL, &malloced_head, the_debug);
					*((int*) ((struct where_clause_node*) ((struct where_clause_node*) the_where_node->ptr_one)->ptr_one)->ptr_two) = 1;
					((struct where_clause_node*) ((struct where_clause_node*) the_where_node->ptr_one)->ptr_one)->ptr_two_type = PTR_TYPE_INT;

					((struct where_clause_node*) ((struct where_clause_node*) the_where_node->ptr_one)->ptr_one)->where_type = WHERE_IS_EQUALS;
					((struct where_clause_node*) ((struct where_clause_node*) the_where_node->ptr_one)->ptr_one)->parent = the_where_node->ptr_one;


					((struct where_clause_node*) ((struct where_clause_node*) the_where_node->ptr_one)->ptr_two)->ptr_one = ((struct select_node*) the_select_node)->columns_arr[0];
					((struct where_clause_node*) ((struct where_clause_node*) the_where_node->ptr_one)->ptr_two)->ptr_one_type = PTR_TYPE_COL_IN_SELECT_NODE;

					((struct where_clause_node*) ((struct where_clause_node*) the_where_node->ptr_one)->ptr_two)->ptr_two = myMalloc(sizeof(char) * 16, NULL, &malloced_head, the_debug);
					strcpy(((struct where_clause_node*) ((struct where_clause_node*) the_where_node->ptr_one)->ptr_two)->ptr_two, "test");
					((struct where_clause_node*) ((struct where_clause_node*) the_where_node->ptr_one)->ptr_two)->ptr_two_type = PTR_TYPE_CHAR;

					((struct where_clause_node*) ((struct where_clause_node*) the_where_node->ptr_one)->ptr_two)->where_type = WHERE_IS_EQUALS;
					((struct where_clause_node*) ((struct where_clause_node*) the_where_node->ptr_one)->ptr_two)->parent = the_where_node->ptr_one;

				((struct where_clause_node*) the_where_node->ptr_two)->ptr_one = ((struct select_node*) the_select_node)->columns_arr[2];
				((struct where_clause_node*) the_where_node->ptr_two)->ptr_one_type = PTR_TYPE_COL_IN_SELECT_NODE;

				((struct where_clause_node*) the_where_node->ptr_two)->ptr_two = myMalloc(sizeof(char) * 16, NULL, &malloced_head, the_debug);
				strcpy(((struct where_clause_node*) the_where_node->ptr_two)->ptr_two, "That");
				((struct where_clause_node*) the_where_node->ptr_two)->ptr_two_type = PTR_TYPE_CHAR;

				((struct where_clause_node*) the_where_node->ptr_two)->where_type = WHERE_IS_EQUALS;
				((struct where_clause_node*) the_where_node->ptr_two)->parent = the_where_node;


			if (test_Controller_parseWhereClause(112, "WherE (1 = 1 or braND-name = 'test') aNd status = 'That' \n ;", "where"
												,&parsed_error_code, the_select_node->next, &the_where_node
												,&malloced_head, the_debug) != 0)
				return -1;


			if (parsed_error_code == 0)
			{
				freeAnyLinkedList((void**) &the_select_node, PTR_TYPE_SELECT_NODE, NULL, &malloced_head, the_debug);
				freeAnyLinkedList((void**) &the_where_node, PTR_TYPE_WHERE_CLAUSE_NODE, NULL, &malloced_head, the_debug);
			}

			if (malloced_head != NULL)
			{
				if (the_debug == YES_DEBUG)
				{
					setOutputRed();
					printf("	ERROR in test_Driver_main() at line %d in %s\n", __LINE__, __FILE__);
					setOutputWhite();
				}
				return -3;
			}
		// END Test with id = 112

		// START Test with id = 113
			the_select_node = NULL;
			initSelectClauseForComp(&the_select_node, NULL, false, 0, NULL, getTablesHead(), PTR_TYPE_TABLE_INFO, NULL, NULL
								   ,&malloced_head, the_debug);
			initSelectClauseForComp(&the_select_node->next, NULL, false, 0, NULL, the_select_node, PTR_TYPE_SELECT_NODE, NULL, NULL
								   ,&malloced_head, the_debug);
			the_select_node->next->prev = the_select_node;


			the_where_node = (struct where_clause_node*) myMalloc(sizeof(struct where_clause_node), NULL, &malloced_head, the_debug);

			the_where_node->ptr_one = (struct where_clause_node*) myMalloc(sizeof(struct where_clause_node), NULL, &malloced_head, the_debug);
			the_where_node->ptr_one_type = PTR_TYPE_WHERE_CLAUSE_NODE;

			the_where_node->ptr_two = (struct where_clause_node*) myMalloc(sizeof(struct where_clause_node), NULL, &malloced_head, the_debug);
			the_where_node->ptr_two_type = PTR_TYPE_WHERE_CLAUSE_NODE;

			the_where_node->where_type = WHERE_OR;
			the_where_node->parent = NULL;

				((struct where_clause_node*) the_where_node->ptr_one)->ptr_one = myMalloc(sizeof(int), NULL, &malloced_head, the_debug);
				*((int*) ((struct where_clause_node*) the_where_node->ptr_one)->ptr_one) = 1;
				((struct where_clause_node*) the_where_node->ptr_one)->ptr_one_type = PTR_TYPE_INT;

				((struct where_clause_node*) the_where_node->ptr_one)->ptr_two = myMalloc(sizeof(int), NULL, &malloced_head, the_debug);
				*((int*) ((struct where_clause_node*) the_where_node->ptr_one)->ptr_two) = 1;
				((struct where_clause_node*) the_where_node->ptr_one)->ptr_two_type = PTR_TYPE_INT;

				((struct where_clause_node*) the_where_node->ptr_one)->where_type = WHERE_IS_EQUALS;
				((struct where_clause_node*) the_where_node->ptr_one)->parent = the_where_node->ptr_one;


				((struct where_clause_node*) the_where_node->ptr_two)->ptr_one = (struct where_clause_node*) myMalloc(sizeof(struct where_clause_node), NULL, &malloced_head, the_debug);
				((struct where_clause_node*) the_where_node->ptr_two)->ptr_one_type = PTR_TYPE_WHERE_CLAUSE_NODE;

				((struct where_clause_node*) the_where_node->ptr_two)->ptr_two = (struct where_clause_node*) myMalloc(sizeof(struct where_clause_node), NULL, &malloced_head, the_debug);
				((struct where_clause_node*) the_where_node->ptr_two)->ptr_two_type = PTR_TYPE_WHERE_CLAUSE_NODE;

				((struct where_clause_node*) the_where_node->ptr_two)->where_type = WHERE_AND;
				((struct where_clause_node*) the_where_node->ptr_two)->parent = the_where_node;

					((struct where_clause_node*) ((struct where_clause_node*) the_where_node->ptr_two)->ptr_one)->ptr_one = ((struct select_node*) the_select_node)->columns_arr[0];
					((struct where_clause_node*) ((struct where_clause_node*) the_where_node->ptr_two)->ptr_one)->ptr_one_type = PTR_TYPE_COL_IN_SELECT_NODE;

					((struct where_clause_node*) ((struct where_clause_node*) the_where_node->ptr_two)->ptr_one)->ptr_two = myMalloc(sizeof(char) * 16, NULL, &malloced_head, the_debug);
					strcpy(((struct where_clause_node*) ((struct where_clause_node*) the_where_node->ptr_two)->ptr_one)->ptr_two, "test");
					((struct where_clause_node*) ((struct where_clause_node*) the_where_node->ptr_two)->ptr_one)->ptr_two_type = PTR_TYPE_CHAR;

					((struct where_clause_node*) ((struct where_clause_node*) the_where_node->ptr_two)->ptr_one)->where_type = WHERE_IS_EQUALS;
					((struct where_clause_node*) ((struct where_clause_node*) the_where_node->ptr_two)->ptr_one)->parent = the_where_node->ptr_two;


					((struct where_clause_node*) ((struct where_clause_node*) the_where_node->ptr_two)->ptr_two)->ptr_one = ((struct select_node*) the_select_node)->columns_arr[2];
					((struct where_clause_node*) ((struct where_clause_node*) the_where_node->ptr_two)->ptr_two)->ptr_one_type = PTR_TYPE_COL_IN_SELECT_NODE;

					((struct where_clause_node*) ((struct where_clause_node*) the_where_node->ptr_two)->ptr_two)->ptr_two = myMalloc(sizeof(char) * 16, NULL, &malloced_head, the_debug);
					strcpy(((struct where_clause_node*) ((struct where_clause_node*) the_where_node->ptr_two)->ptr_two)->ptr_two, "That");
					((struct where_clause_node*) ((struct where_clause_node*) the_where_node->ptr_two)->ptr_two)->ptr_two_type = PTR_TYPE_CHAR;

					((struct where_clause_node*) ((struct where_clause_node*) the_where_node->ptr_two)->ptr_two)->where_type = WHERE_IS_EQUALS;
					((struct where_clause_node*) ((struct where_clause_node*) the_where_node->ptr_two)->ptr_two)->parent = the_where_node->ptr_one;


			if (test_Controller_parseWhereClause(113, "WherE 1 = 1 or braND-name = 'test' and status = 'That' \n ;", "where"
												,&parsed_error_code, the_select_node->next, &the_where_node
												,&malloced_head, the_debug) != 0)
				return -1;


			if (parsed_error_code == 0)
			{
				freeAnyLinkedList((void**) &the_select_node, PTR_TYPE_SELECT_NODE, NULL, &malloced_head, the_debug);
				freeAnyLinkedList((void**) &the_where_node, PTR_TYPE_WHERE_CLAUSE_NODE, NULL, &malloced_head, the_debug);
			}

			if (malloced_head != NULL)
			{
				if (the_debug == YES_DEBUG)
				{
					setOutputRed();
					printf("	ERROR in test_Driver_main() at line %d in %s\n", __LINE__, __FILE__);
					setOutputWhite();
				}
				return -3;
			}
		// END Test with id = 113

		// START Test with id = 114
			the_select_node = NULL;
			initSelectClauseForComp(&the_select_node, NULL, false, 0, NULL, getTablesHead(), PTR_TYPE_TABLE_INFO, NULL, NULL
								   ,&malloced_head, the_debug);
			initSelectClauseForComp(&the_select_node->next, NULL, false, 0, NULL, the_select_node, PTR_TYPE_SELECT_NODE, NULL, NULL
								   ,&malloced_head, the_debug);
			the_select_node->next->prev = the_select_node;


			the_where_node = (struct where_clause_node*) myMalloc(sizeof(struct where_clause_node), NULL, &malloced_head, the_debug);

			the_where_node->ptr_one = (struct where_clause_node*) myMalloc(sizeof(struct where_clause_node), NULL, &malloced_head, the_debug);
			the_where_node->ptr_one_type = PTR_TYPE_WHERE_CLAUSE_NODE;

			the_where_node->ptr_two = (struct where_clause_node*) myMalloc(sizeof(struct where_clause_node), NULL, &malloced_head, the_debug);
			the_where_node->ptr_two_type = PTR_TYPE_WHERE_CLAUSE_NODE;

			the_where_node->where_type = WHERE_OR;
			the_where_node->parent = NULL;

				((struct where_clause_node*) the_where_node->ptr_one)->ptr_one = (struct where_clause_node*) myMalloc(sizeof(struct where_clause_node), NULL, &malloced_head, the_debug);
				((struct where_clause_node*) the_where_node->ptr_one)->ptr_one_type = PTR_TYPE_WHERE_CLAUSE_NODE;

				((struct where_clause_node*) the_where_node->ptr_one)->ptr_two = (struct where_clause_node*) myMalloc(sizeof(struct where_clause_node), NULL, &malloced_head, the_debug);
				((struct where_clause_node*) the_where_node->ptr_one)->ptr_two_type = PTR_TYPE_WHERE_CLAUSE_NODE;

				((struct where_clause_node*) the_where_node->ptr_one)->where_type = WHERE_AND;
				((struct where_clause_node*) the_where_node->ptr_one)->parent = the_where_node;

					((struct where_clause_node*) ((struct where_clause_node*) the_where_node->ptr_one)->ptr_one)->ptr_one = myMalloc(sizeof(int), NULL, &malloced_head, the_debug);
					*((int*) ((struct where_clause_node*) ((struct where_clause_node*) the_where_node->ptr_one)->ptr_one)->ptr_one) = 1;
					((struct where_clause_node*) ((struct where_clause_node*) the_where_node->ptr_one)->ptr_one)->ptr_one_type = PTR_TYPE_INT;

					((struct where_clause_node*) ((struct where_clause_node*) the_where_node->ptr_one)->ptr_one)->ptr_two = myMalloc(sizeof(int), NULL, &malloced_head, the_debug);
					*((int*) ((struct where_clause_node*) ((struct where_clause_node*) the_where_node->ptr_one)->ptr_one)->ptr_two) = 1;
					((struct where_clause_node*) ((struct where_clause_node*) the_where_node->ptr_one)->ptr_one)->ptr_two_type = PTR_TYPE_INT;

					((struct where_clause_node*) ((struct where_clause_node*) the_where_node->ptr_one)->ptr_one)->where_type = WHERE_IS_EQUALS;
					((struct where_clause_node*) ((struct where_clause_node*) the_where_node->ptr_one)->ptr_one)->parent = the_where_node->ptr_one;


					((struct where_clause_node*) ((struct where_clause_node*) the_where_node->ptr_one)->ptr_two)->ptr_one = ((struct select_node*) the_select_node)->columns_arr[0];
					((struct where_clause_node*) ((struct where_clause_node*) the_where_node->ptr_one)->ptr_two)->ptr_one_type = PTR_TYPE_COL_IN_SELECT_NODE;

					((struct where_clause_node*) ((struct where_clause_node*) the_where_node->ptr_one)->ptr_two)->ptr_two = myMalloc(sizeof(char) * 16, NULL, &malloced_head, the_debug);
					strcpy(((struct where_clause_node*) ((struct where_clause_node*) the_where_node->ptr_one)->ptr_two)->ptr_two, "test");
					((struct where_clause_node*) ((struct where_clause_node*) the_where_node->ptr_one)->ptr_two)->ptr_two_type = PTR_TYPE_CHAR;

					((struct where_clause_node*) ((struct where_clause_node*) the_where_node->ptr_one)->ptr_two)->where_type = WHERE_IS_EQUALS;
					((struct where_clause_node*) ((struct where_clause_node*) the_where_node->ptr_one)->ptr_two)->parent = the_where_node->ptr_one;

				((struct where_clause_node*) the_where_node->ptr_two)->ptr_one = ((struct select_node*) the_select_node)->columns_arr[2];
				((struct where_clause_node*) the_where_node->ptr_two)->ptr_one_type = PTR_TYPE_COL_IN_SELECT_NODE;

				((struct where_clause_node*) the_where_node->ptr_two)->ptr_two = myMalloc(sizeof(char) * 16, NULL, &malloced_head, the_debug);
				strcpy(((struct where_clause_node*) the_where_node->ptr_two)->ptr_two, "That");
				((struct where_clause_node*) the_where_node->ptr_two)->ptr_two_type = PTR_TYPE_CHAR;

				((struct where_clause_node*) the_where_node->ptr_two)->where_type = WHERE_IS_EQUALS;
				((struct where_clause_node*) the_where_node->ptr_two)->parent = the_where_node;


			if (test_Controller_parseWhereClause(114, "WherE 1 = 1 and braND-name = 'test' or status = 'That' \n ;", "where"
												,&parsed_error_code, the_select_node->next, &the_where_node
												,&malloced_head, the_debug) != 0)
				return -1;


			if (parsed_error_code == 0)
			{
				freeAnyLinkedList((void**) &the_select_node, PTR_TYPE_SELECT_NODE, NULL, &malloced_head, the_debug);
				freeAnyLinkedList((void**) &the_where_node, PTR_TYPE_WHERE_CLAUSE_NODE, NULL, &malloced_head, the_debug);
			}

			if (malloced_head != NULL)
			{
				if (the_debug == YES_DEBUG)
				{
					setOutputRed();
					printf("	ERROR in test_Driver_main() at line %d in %s\n", __LINE__, __FILE__);
					setOutputWhite();
				}
				return -3;
			}
		// END Test with id = 114

		// START Test with id = 115
			the_select_node = NULL;
			initSelectClauseForComp(&the_select_node, NULL, false, 0, NULL, getTablesHead(), PTR_TYPE_TABLE_INFO, NULL, NULL
								   ,&malloced_head, the_debug);
			initSelectClauseForComp(&the_select_node->next, NULL, false, 0, NULL, the_select_node, PTR_TYPE_SELECT_NODE, NULL, NULL
								   ,&malloced_head, the_debug);
			the_select_node->next->prev = the_select_node;


			the_where_node = (struct where_clause_node*) myMalloc(sizeof(struct where_clause_node), NULL, &malloced_head, the_debug);

			the_where_node->ptr_one = (struct where_clause_node*) myMalloc(sizeof(struct where_clause_node), NULL, &malloced_head, the_debug);
			the_where_node->ptr_one_type = PTR_TYPE_WHERE_CLAUSE_NODE;

			the_where_node->ptr_two = (struct where_clause_node*) myMalloc(sizeof(struct where_clause_node), NULL, &malloced_head, the_debug);
			the_where_node->ptr_two_type = PTR_TYPE_WHERE_CLAUSE_NODE;

			the_where_node->where_type = WHERE_OR;
			the_where_node->parent = NULL;

				((struct where_clause_node*) the_where_node->ptr_one)->ptr_one = (struct where_clause_node*) myMalloc(sizeof(struct where_clause_node), NULL, &malloced_head, the_debug);
				((struct where_clause_node*) the_where_node->ptr_one)->ptr_one_type = PTR_TYPE_WHERE_CLAUSE_NODE;

				((struct where_clause_node*) the_where_node->ptr_one)->ptr_two = (struct where_clause_node*) myMalloc(sizeof(struct where_clause_node), NULL, &malloced_head, the_debug);
				((struct where_clause_node*) the_where_node->ptr_one)->ptr_two_type = PTR_TYPE_WHERE_CLAUSE_NODE;

				((struct where_clause_node*) the_where_node->ptr_one)->where_type = WHERE_AND;
				((struct where_clause_node*) the_where_node->ptr_one)->parent = the_where_node;

					((struct where_clause_node*) ((struct where_clause_node*) the_where_node->ptr_one)->ptr_one)->ptr_one = myMalloc(sizeof(int), NULL, &malloced_head, the_debug);
					*((int*) ((struct where_clause_node*) ((struct where_clause_node*) the_where_node->ptr_one)->ptr_one)->ptr_one) = 1;
					((struct where_clause_node*) ((struct where_clause_node*) the_where_node->ptr_one)->ptr_one)->ptr_one_type = PTR_TYPE_INT;

					((struct where_clause_node*) ((struct where_clause_node*) the_where_node->ptr_one)->ptr_one)->ptr_two = myMalloc(sizeof(int), NULL, &malloced_head, the_debug);
					*((int*) ((struct where_clause_node*) ((struct where_clause_node*) the_where_node->ptr_one)->ptr_one)->ptr_two) = 1;
					((struct where_clause_node*) ((struct where_clause_node*) the_where_node->ptr_one)->ptr_one)->ptr_two_type = PTR_TYPE_INT;

					((struct where_clause_node*) ((struct where_clause_node*) the_where_node->ptr_one)->ptr_one)->where_type = WHERE_IS_EQUALS;
					((struct where_clause_node*) ((struct where_clause_node*) the_where_node->ptr_one)->ptr_one)->parent = the_where_node->ptr_one;


					((struct where_clause_node*) ((struct where_clause_node*) the_where_node->ptr_one)->ptr_two)->ptr_one = ((struct select_node*) the_select_node)->columns_arr[0];
					((struct where_clause_node*) ((struct where_clause_node*) the_where_node->ptr_one)->ptr_two)->ptr_one_type = PTR_TYPE_COL_IN_SELECT_NODE;

					((struct where_clause_node*) ((struct where_clause_node*) the_where_node->ptr_one)->ptr_two)->ptr_two = myMalloc(sizeof(char) * 16, NULL, &malloced_head, the_debug);
					strcpy(((struct where_clause_node*) ((struct where_clause_node*) the_where_node->ptr_one)->ptr_two)->ptr_two, "test");
					((struct where_clause_node*) ((struct where_clause_node*) the_where_node->ptr_one)->ptr_two)->ptr_two_type = PTR_TYPE_CHAR;

					((struct where_clause_node*) ((struct where_clause_node*) the_where_node->ptr_one)->ptr_two)->where_type = WHERE_IS_EQUALS;
					((struct where_clause_node*) ((struct where_clause_node*) the_where_node->ptr_one)->ptr_two)->parent = the_where_node->ptr_one;

				((struct where_clause_node*) the_where_node->ptr_two)->ptr_one = (struct where_clause_node*) myMalloc(sizeof(struct where_clause_node), NULL, &malloced_head, the_debug);
				((struct where_clause_node*) the_where_node->ptr_two)->ptr_one_type = PTR_TYPE_WHERE_CLAUSE_NODE;

				((struct where_clause_node*) the_where_node->ptr_two)->ptr_two = (struct where_clause_node*) myMalloc(sizeof(struct where_clause_node), NULL, &malloced_head, the_debug);
				((struct where_clause_node*) the_where_node->ptr_two)->ptr_two_type = PTR_TYPE_WHERE_CLAUSE_NODE;

				((struct where_clause_node*) the_where_node->ptr_two)->where_type = WHERE_AND;
				((struct where_clause_node*) the_where_node->ptr_two)->parent = the_where_node;

					((struct where_clause_node*) ((struct where_clause_node*) the_where_node->ptr_two)->ptr_one)->ptr_one = ((struct select_node*) the_select_node)->columns_arr[2];
					((struct where_clause_node*) ((struct where_clause_node*) the_where_node->ptr_two)->ptr_one)->ptr_one_type = PTR_TYPE_COL_IN_SELECT_NODE;

					((struct where_clause_node*) ((struct where_clause_node*) the_where_node->ptr_two)->ptr_one)->ptr_two = myMalloc(sizeof(char) * 16, NULL, &malloced_head, the_debug);
					strcpy(((struct where_clause_node*) ((struct where_clause_node*) the_where_node->ptr_two)->ptr_one)->ptr_two, "That");
					((struct where_clause_node*) ((struct where_clause_node*) the_where_node->ptr_two)->ptr_one)->ptr_two_type = PTR_TYPE_CHAR;

					((struct where_clause_node*) ((struct where_clause_node*) the_where_node->ptr_two)->ptr_one)->where_type = WHERE_IS_EQUALS;
					((struct where_clause_node*) ((struct where_clause_node*) the_where_node->ptr_two)->ptr_one)->parent = the_where_node->ptr_two;


					((struct where_clause_node*) ((struct where_clause_node*) the_where_node->ptr_two)->ptr_two)->ptr_one = myMalloc(sizeof(int), NULL, &malloced_head, the_debug);
					*((int*) ((struct where_clause_node*) ((struct where_clause_node*) the_where_node->ptr_two)->ptr_two)->ptr_one) = 1;
					((struct where_clause_node*) ((struct where_clause_node*) the_where_node->ptr_two)->ptr_two)->ptr_one_type = PTR_TYPE_INT;

					((struct where_clause_node*) ((struct where_clause_node*) the_where_node->ptr_two)->ptr_two)->ptr_two = myMalloc(sizeof(int), NULL, &malloced_head, the_debug);
					*((int*) ((struct where_clause_node*) ((struct where_clause_node*) the_where_node->ptr_two)->ptr_two)->ptr_two) = 2;
					((struct where_clause_node*) ((struct where_clause_node*) the_where_node->ptr_two)->ptr_two)->ptr_two_type = PTR_TYPE_INT;

					((struct where_clause_node*) ((struct where_clause_node*) the_where_node->ptr_two)->ptr_two)->where_type = WHERE_NOT_EQUALS;
					((struct where_clause_node*) ((struct where_clause_node*) the_where_node->ptr_two)->ptr_two)->parent = the_where_node->ptr_two;


			if (test_Controller_parseWhereClause(115, "WherE 1 = 1 and braND-name = 'test' or status = 'That' and 1 <> 2 \n ;", "where"
												,&parsed_error_code, the_select_node->next, &the_where_node
												,&malloced_head, the_debug) != 0)
				return -1;


			if (parsed_error_code == 0)
			{
				freeAnyLinkedList((void**) &the_select_node, PTR_TYPE_SELECT_NODE, NULL, &malloced_head, the_debug);
				freeAnyLinkedList((void**) &the_where_node, PTR_TYPE_WHERE_CLAUSE_NODE, NULL, &malloced_head, the_debug);
			}

			if (malloced_head != NULL)
			{
				if (the_debug == YES_DEBUG)
				{
					setOutputRed();
					printf("	ERROR in test_Driver_main() at line %d in %s\n", __LINE__, __FILE__);
					setOutputWhite();
				}
				return -3;
			}
		// END Test with id = 115

		// START Test with id = 116
			the_select_node = NULL;
			initSelectClauseForComp(&the_select_node, NULL, false, 0, NULL, getTablesHead(), PTR_TYPE_TABLE_INFO, NULL, NULL
								   ,&malloced_head, the_debug);
			initSelectClauseForComp(&the_select_node->next, NULL, false, 0, NULL, the_select_node, PTR_TYPE_SELECT_NODE, NULL, NULL
								   ,&malloced_head, the_debug);
			the_select_node->next->prev = the_select_node;


			the_where_node = (struct where_clause_node*) myMalloc(sizeof(struct where_clause_node), NULL, &malloced_head, the_debug);

			the_where_node->ptr_one = (struct where_clause_node*) myMalloc(sizeof(struct where_clause_node), NULL, &malloced_head, the_debug);
			the_where_node->ptr_one_type = PTR_TYPE_WHERE_CLAUSE_NODE;

			the_where_node->ptr_two = (struct where_clause_node*) myMalloc(sizeof(struct where_clause_node), NULL, &malloced_head, the_debug);
			the_where_node->ptr_two_type = PTR_TYPE_WHERE_CLAUSE_NODE;

			the_where_node->where_type = WHERE_OR;
			the_where_node->parent = NULL;

				((struct where_clause_node*) the_where_node->ptr_one)->ptr_one = (struct where_clause_node*) myMalloc(sizeof(struct where_clause_node), NULL, &malloced_head, the_debug);
				((struct where_clause_node*) the_where_node->ptr_one)->ptr_one_type = PTR_TYPE_WHERE_CLAUSE_NODE;

				((struct where_clause_node*) the_where_node->ptr_one)->ptr_two = (struct where_clause_node*) myMalloc(sizeof(struct where_clause_node), NULL, &malloced_head, the_debug);
				((struct where_clause_node*) the_where_node->ptr_one)->ptr_two_type = PTR_TYPE_WHERE_CLAUSE_NODE;

				((struct where_clause_node*) the_where_node->ptr_one)->where_type = WHERE_AND;
				((struct where_clause_node*) the_where_node->ptr_one)->parent = the_where_node;


				((struct where_clause_node*) the_where_node->ptr_two)->ptr_one = myMalloc(sizeof(int), NULL, &malloced_head, the_debug);
				*((int*) ((struct where_clause_node*) the_where_node->ptr_two)->ptr_one) = 1;
				((struct where_clause_node*) the_where_node->ptr_two)->ptr_one_type = PTR_TYPE_INT;

				((struct where_clause_node*) the_where_node->ptr_two)->ptr_two = myMalloc(sizeof(int), NULL, &malloced_head, the_debug);
				*((int*) ((struct where_clause_node*) the_where_node->ptr_two)->ptr_two) = 2;
				((struct where_clause_node*) the_where_node->ptr_two)->ptr_two_type = PTR_TYPE_INT;

				((struct where_clause_node*) the_where_node->ptr_two)->where_type = WHERE_NOT_EQUALS;
				((struct where_clause_node*) the_where_node->ptr_two)->parent = the_where_node;

					
					((struct where_clause_node*) ((struct where_clause_node*) the_where_node->ptr_one)->ptr_one)->ptr_one = (struct where_clause_node*) myMalloc(sizeof(struct where_clause_node), NULL, &malloced_head, the_debug);
					((struct where_clause_node*) ((struct where_clause_node*) the_where_node->ptr_one)->ptr_one)->ptr_one_type = PTR_TYPE_WHERE_CLAUSE_NODE;

					((struct where_clause_node*) ((struct where_clause_node*) the_where_node->ptr_one)->ptr_one)->ptr_two = (struct where_clause_node*) myMalloc(sizeof(struct where_clause_node), NULL, &malloced_head, the_debug);
					((struct where_clause_node*) ((struct where_clause_node*) the_where_node->ptr_one)->ptr_one)->ptr_two_type = PTR_TYPE_WHERE_CLAUSE_NODE;

					((struct where_clause_node*) ((struct where_clause_node*) the_where_node->ptr_one)->ptr_one)->where_type = WHERE_AND;
					((struct where_clause_node*) ((struct where_clause_node*) the_where_node->ptr_one)->ptr_one)->parent = the_where_node->ptr_one;


					((struct where_clause_node*) ((struct where_clause_node*) the_where_node->ptr_one)->ptr_two)->ptr_one = ((struct select_node*) the_select_node)->columns_arr[2];
					((struct where_clause_node*) ((struct where_clause_node*) the_where_node->ptr_one)->ptr_two)->ptr_one_type = PTR_TYPE_COL_IN_SELECT_NODE;

					((struct where_clause_node*) ((struct where_clause_node*) the_where_node->ptr_one)->ptr_two)->ptr_two = myMalloc(sizeof(char) * 16, NULL, &malloced_head, the_debug);
					strcpy(((struct where_clause_node*) ((struct where_clause_node*) the_where_node->ptr_one)->ptr_two)->ptr_two, "That");
					((struct where_clause_node*) ((struct where_clause_node*) the_where_node->ptr_one)->ptr_two)->ptr_two_type = PTR_TYPE_CHAR;

					((struct where_clause_node*) ((struct where_clause_node*) the_where_node->ptr_one)->ptr_two)->where_type = WHERE_IS_EQUALS;
					((struct where_clause_node*) ((struct where_clause_node*) the_where_node->ptr_one)->ptr_two)->parent = the_where_node->ptr_one;


						((struct where_clause_node*) ((struct where_clause_node*) ((struct where_clause_node*) the_where_node->ptr_one)->ptr_one)->ptr_one)->ptr_one = myMalloc(sizeof(int), NULL, &malloced_head, the_debug);
						*((int*) ((struct where_clause_node*) ((struct where_clause_node*) ((struct where_clause_node*) the_where_node->ptr_one)->ptr_one)->ptr_one)->ptr_one) = 1;
						((struct where_clause_node*) ((struct where_clause_node*) ((struct where_clause_node*) the_where_node->ptr_one)->ptr_one)->ptr_one)->ptr_one_type = PTR_TYPE_INT;

						((struct where_clause_node*) ((struct where_clause_node*) ((struct where_clause_node*) the_where_node->ptr_one)->ptr_one)->ptr_one)->ptr_two = myMalloc(sizeof(int), NULL, &malloced_head, the_debug);
						*((int*) ((struct where_clause_node*) ((struct where_clause_node*) ((struct where_clause_node*) the_where_node->ptr_one)->ptr_one)->ptr_one)->ptr_two) = 1;
						((struct where_clause_node*) ((struct where_clause_node*) ((struct where_clause_node*) the_where_node->ptr_one)->ptr_one)->ptr_one)->ptr_two_type = PTR_TYPE_INT;

						((struct where_clause_node*) ((struct where_clause_node*) ((struct where_clause_node*) the_where_node->ptr_one)->ptr_one)->ptr_one)->where_type = WHERE_IS_EQUALS;
						((struct where_clause_node*) ((struct where_clause_node*) ((struct where_clause_node*) the_where_node->ptr_one)->ptr_one)->ptr_one)->parent = ((struct where_clause_node*) the_where_node->ptr_one)->ptr_one;


						((struct where_clause_node*) ((struct where_clause_node*) ((struct where_clause_node*) the_where_node->ptr_one)->ptr_one)->ptr_two)->ptr_one = ((struct select_node*) the_select_node)->columns_arr[0];
						((struct where_clause_node*) ((struct where_clause_node*) ((struct where_clause_node*) the_where_node->ptr_one)->ptr_one)->ptr_two)->ptr_one_type = PTR_TYPE_COL_IN_SELECT_NODE;

						((struct where_clause_node*) ((struct where_clause_node*) ((struct where_clause_node*) the_where_node->ptr_one)->ptr_one)->ptr_two)->ptr_two = myMalloc(sizeof(char) * 16, NULL, &malloced_head, the_debug);
						strcpy(((struct where_clause_node*) ((struct where_clause_node*) ((struct where_clause_node*) the_where_node->ptr_one)->ptr_one)->ptr_two)->ptr_two, "test");
						((struct where_clause_node*) ((struct where_clause_node*) ((struct where_clause_node*) the_where_node->ptr_one)->ptr_one)->ptr_two)->ptr_two_type = PTR_TYPE_CHAR;

						((struct where_clause_node*) ((struct where_clause_node*) ((struct where_clause_node*) the_where_node->ptr_one)->ptr_one)->ptr_two)->where_type = WHERE_IS_EQUALS;
						((struct where_clause_node*) ((struct where_clause_node*) ((struct where_clause_node*) the_where_node->ptr_one)->ptr_one)->ptr_two)->parent = ((struct where_clause_node*) the_where_node->ptr_one)->ptr_one;


			if (test_Controller_parseWhereClause(116, "WherE 1 = 1 and braND-name = 'test' AND status = 'That' OR 1 <> 2 \n ;", "where"
												,&parsed_error_code, the_select_node->next, &the_where_node
												,&malloced_head, the_debug) != 0)
				return -1;


			if (parsed_error_code == 0)
			{
				freeAnyLinkedList((void**) &the_select_node, PTR_TYPE_SELECT_NODE, NULL, &malloced_head, the_debug);
				freeAnyLinkedList((void**) &the_where_node, PTR_TYPE_WHERE_CLAUSE_NODE, NULL, &malloced_head, the_debug);
			}

			if (malloced_head != NULL)
			{
				if (the_debug == YES_DEBUG)
				{
					setOutputRed();
					printf("	ERROR in test_Driver_main() at line %d in %s\n", __LINE__, __FILE__);
					setOutputWhite();
				}
				return -3;
			}
		// END Test with id = 116

		// START Test with id = 117
			the_select_node = NULL;
			initSelectClauseForComp(&the_select_node, NULL, false, 0, NULL, getTablesHead(), PTR_TYPE_TABLE_INFO, NULL, NULL
								   ,&malloced_head, the_debug);
			initSelectClauseForComp(&the_select_node->next, NULL, false, 0, NULL, the_select_node, PTR_TYPE_SELECT_NODE, NULL, NULL
								   ,&malloced_head, the_debug);
			the_select_node->next->prev = the_select_node;


			the_where_node = NULL;


			if (test_Controller_parseWhereClause(117, "where BRAND-NAME = 13123;", "where"
												,&parsed_error_code, the_select_node->next, &the_where_node
												,&malloced_head, the_debug) != 0)
				return -1;


			if (parsed_error_code == 0)
			{
				freeAnyLinkedList((void**) &the_select_node, PTR_TYPE_SELECT_NODE, NULL, &malloced_head, the_debug);
				freeAnyLinkedList((void**) &the_where_node, PTR_TYPE_WHERE_CLAUSE_NODE, NULL, &malloced_head, the_debug);
			}

			if (malloced_head != NULL)
			{
				if (the_debug == YES_DEBUG)
				{
					setOutputRed();
					printf("	ERROR in test_Driver_main() at line %d in %s\n", __LINE__, __FILE__);
					setOutputWhite();
				}
				return -3;
			}
		// END Test with id = 117

		// START Test with id = 118
			the_select_node = NULL;
			initSelectClauseForComp(&the_select_node, "TBL", false, 0, NULL, getTablesHead(), PTR_TYPE_TABLE_INFO, NULL, NULL
								   ,&malloced_head, the_debug);
			initSelectClauseForComp(&the_select_node->next, NULL, false, 0, NULL, the_select_node, PTR_TYPE_SELECT_NODE, NULL, NULL
								   ,&malloced_head, the_debug);
			the_select_node->next->prev = the_select_node;


			the_where_node = (struct where_clause_node*) myMalloc(sizeof(struct where_clause_node), NULL, &malloced_head, the_debug);

			the_where_node->ptr_one = (struct math_node*) myMalloc(sizeof(struct math_node), NULL, &malloced_head, the_debug);
			the_where_node->ptr_one_type = PTR_TYPE_MATH_NODE;

			the_where_node->ptr_two = myMalloc(sizeof(int), NULL, &malloced_head, the_debug);
			*((int*) the_where_node->ptr_two) = 13123;
			the_where_node->ptr_two_type = PTR_TYPE_INT;

			the_where_node->where_type = WHERE_IS_EQUALS;
			the_where_node->parent = NULL;


			((struct math_node*) the_where_node->ptr_one)->ptr_one = the_select_node->columns_arr[1];
			((struct math_node*) the_where_node->ptr_one)->ptr_one_type = PTR_TYPE_COL_IN_SELECT_NODE;

			((struct math_node*) the_where_node->ptr_one)->ptr_two = myMalloc(sizeof(int), NULL, &malloced_head, the_debug);
			*((int*) ((struct math_node*) the_where_node->ptr_one)->ptr_two) = 10;
			((struct math_node*) the_where_node->ptr_one)->ptr_two_type = PTR_TYPE_INT;

			((struct math_node*) the_where_node->ptr_one)->operation = MATH_ADD;
			((struct math_node*) the_where_node->ptr_one)->parent = NULL;


			if (test_Controller_parseWhereClause(118, "where tbl . CT-REGISTRATION-NUMBER + 10 = 13123;", "where"
												,&parsed_error_code, the_select_node->next, &the_where_node
												,&malloced_head, the_debug) != 0)
				return -1;

			
			if (parsed_error_code == 0)
			{
				freeAnyLinkedList((void**) &the_select_node, PTR_TYPE_SELECT_NODE, NULL, &malloced_head, the_debug);
				freeAnyLinkedList((void**) &the_where_node, PTR_TYPE_WHERE_CLAUSE_NODE, NULL, &malloced_head, the_debug);
			}

			if (malloced_head != NULL)
			{
				if (the_debug == YES_DEBUG)
				{
					setOutputRed();
					printf("	ERROR in test_Driver_main() at line %d in %s\n", __LINE__, __FILE__);
					setOutputWhite();
				}
				return -3;
			}
		// END Test with id = 118

		// START Test with id = 119
			the_select_node = NULL;
			initSelectClauseForComp(&the_select_node, "TBL", false, 0, NULL, getTablesHead(), PTR_TYPE_TABLE_INFO, NULL, NULL
								   ,&malloced_head, the_debug);
			initSelectClauseForComp(&the_select_node->next, NULL, false, 0, NULL, the_select_node, PTR_TYPE_SELECT_NODE, NULL, NULL
								   ,&malloced_head, the_debug);
			the_select_node->next->prev = the_select_node;


			the_where_node = (struct where_clause_node*) myMalloc(sizeof(struct where_clause_node), NULL, &malloced_head, the_debug);

			the_where_node->ptr_one = (struct math_node*) myMalloc(sizeof(struct math_node), NULL, &malloced_head, the_debug);
			the_where_node->ptr_one_type = PTR_TYPE_MATH_NODE;

			the_where_node->ptr_two = myMalloc(sizeof(int), NULL, &malloced_head, the_debug);
			*((int*) the_where_node->ptr_two) = 13123;
			the_where_node->ptr_two_type = PTR_TYPE_INT;

			the_where_node->where_type = WHERE_IS_EQUALS;
			the_where_node->parent = NULL;


			((struct math_node*) the_where_node->ptr_one)->ptr_one = the_select_node->columns_arr[1];
			((struct math_node*) the_where_node->ptr_one)->ptr_one_type = PTR_TYPE_COL_IN_SELECT_NODE;

			((struct math_node*) the_where_node->ptr_one)->ptr_two = myMalloc(sizeof(int), NULL, &malloced_head, the_debug);
			*((int*) ((struct math_node*) the_where_node->ptr_one)->ptr_two) = 10;
			((struct math_node*) the_where_node->ptr_one)->ptr_two_type = PTR_TYPE_INT;

			((struct math_node*) the_where_node->ptr_one)->operation = MATH_ADD;
			((struct math_node*) the_where_node->ptr_one)->parent = NULL;


			if (test_Controller_parseWhereClause(119, "where (tbl . CT-REGISTRATION-NUMBER + 10) = 13123;", "where"
												,&parsed_error_code, the_select_node->next, &the_where_node
												,&malloced_head, the_debug) != 0)
				return -1;

			
			if (parsed_error_code == 0)
			{
				freeAnyLinkedList((void**) &the_select_node, PTR_TYPE_SELECT_NODE, NULL, &malloced_head, the_debug);
				freeAnyLinkedList((void**) &the_where_node, PTR_TYPE_WHERE_CLAUSE_NODE, NULL, &malloced_head, the_debug);
			}

			if (malloced_head != NULL)
			{
				if (the_debug == YES_DEBUG)
				{
					setOutputRed();
					printf("	ERROR in test_Driver_main() at line %d in %s\n", __LINE__, __FILE__);
					setOutputWhite();
				}
				return -3;
			}
		// END Test with id = 119
	// END test_Controller_parseWhereClause

	// START test_Controller_parseSelect
		// START Test with id = 501
			initSelectClauseForComp(&the_select_node, NULL, false, 0, NULL, getTablesHead(), PTR_TYPE_TABLE_INFO, NULL, NULL
								   ,&malloced_head, the_debug);
			initSelectClauseForComp(&the_select_node->next, NULL, false, 0, NULL, the_select_node, PTR_TYPE_SELECT_NODE, NULL, NULL
								   ,&malloced_head, the_debug);
			the_select_node->next->prev = the_select_node;


			if (test_Controller_parseSelect(501, "select * from alc_brands;", &the_select_node
										   ,&parsed_error_code, &malloced_head, the_debug) != 0)
				return -1;

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
			col_arr[0]->func_node = NULL;
			col_arr[0]->math_node = NULL;
			col_arr[0]->case_node = NULL;

			col_arr[1] = (struct col_in_select_node*) myMalloc(sizeof(struct col_in_select_node), NULL, &malloced_head, the_debug);
			col_arr[1]->table_ptr = the_select_node;
			col_arr[1]->table_ptr_type = PTR_TYPE_SELECT_NODE;
			col_arr[1]->col_ptr = ((struct select_node*) the_select_node)->columns_arr[6];
			col_arr[1]->col_ptr_type = PTR_TYPE_COL_IN_SELECT_NODE;
			col_arr[1]->new_name = upper("COL2", NULL, &malloced_head, the_debug);
			col_arr[1]->func_node = NULL;
			col_arr[1]->math_node = NULL;
			col_arr[1]->case_node = NULL;

			initSelectClauseForComp(&the_select_node->next, NULL, true, 2, col_arr, NULL, -1, NULL, NULL
								   ,&malloced_head, the_debug);
			the_select_node->next->prev = the_select_node;
			

			if (test_Controller_parseSelect(502, "select distinct tbl.Brand-name as col1, SUPERVISOR-CREDENTIAL col2 from alc_brands as tbl;", &the_select_node
										   ,&parsed_error_code, &malloced_head, the_debug) != 0)
				return -1;

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
				return -3;
			}
		// END Test with id = 502

		// START Test with id = 503
			initSelectClauseForComp(&the_select_node, "TBL", false, 0, NULL, getTablesHead(), PTR_TYPE_TABLE_INFO, NULL, NULL
								   ,&malloced_head, the_debug);

			col_arr = (struct col_in_select_node**) myMalloc(sizeof(struct col_in_select_node*) * 2, NULL, &malloced_head, the_debug);
			
			col_arr[0] = (struct col_in_select_node*) myMalloc(sizeof(struct col_in_select_node), NULL, &malloced_head, the_debug);
			col_arr[0]->table_ptr = NULL;
			col_arr[0]->table_ptr_type = -1;
			col_arr[0]->col_ptr = NULL;
			col_arr[0]->col_ptr_type = -1;
			col_arr[0]->new_name = upper("COUNT ( * ) ", NULL, &malloced_head, the_debug);
			col_arr[0]->math_node = NULL;
			col_arr[0]->case_node = NULL;

			col_arr[0]->func_node = (struct func_node*) myMalloc(sizeof(struct func_node), NULL, &malloced_head, the_debug);
			col_arr[0]->func_node->which_func = FUNC_COUNT;
			col_arr[0]->func_node->distinct = false;
			col_arr[0]->func_node->args_size = 1;
			col_arr[0]->func_node->args_arr = (void**) myMalloc(sizeof(void*), NULL, &malloced_head, the_debug);
			col_arr[0]->func_node->args_arr[0] = upper("*", NULL, &malloced_head, the_debug);
			col_arr[0]->func_node->args_arr_type = (int*) myMalloc(sizeof(int), NULL, &malloced_head, the_debug);
			col_arr[0]->func_node->args_arr_type[0] = PTR_TYPE_CHAR;
			col_arr[0]->func_node->group_by_cols_head = NULL;
			col_arr[0]->func_node->group_by_cols_tail = NULL;
			addListNodePtr(&col_arr[0]->func_node->group_by_cols_head, &col_arr[0]->func_node->group_by_cols_tail
						  ,((struct select_node*) the_select_node)->columns_arr[1], PTR_TYPE_COL_IN_SELECT_NODE, ADDLISTNODE_TAIL
				  		  ,NULL, &malloced_head, the_debug);

			col_arr[1] = (struct col_in_select_node*) myMalloc(sizeof(struct col_in_select_node), NULL, &malloced_head, the_debug);
			col_arr[1]->table_ptr = NULL;
			col_arr[1]->table_ptr_type = -1;
			col_arr[1]->col_ptr = NULL;
			col_arr[1]->col_ptr_type = -1;
			col_arr[1]->new_name = upper("MATH", NULL, &malloced_head, the_debug);
			col_arr[1]->func_node = NULL;
			col_arr[1]->case_node = NULL;

			col_arr[1]->math_node = (struct math_node*) myMalloc(sizeof(struct math_node), NULL, &malloced_head, the_debug);
			col_arr[1]->math_node->ptr_one = ((struct select_node*) the_select_node)->columns_arr[1];
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
				return -1;

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
				return -3;
			}
		// END Test with id = 503

		// START Test with id = 504
			initSelectClauseForComp(&the_select_node, NULL, false, 0, NULL, getTablesHead(), PTR_TYPE_TABLE_INFO, NULL, NULL
								   ,&malloced_head, the_debug);

			col_arr = (struct col_in_select_node**) myMalloc(sizeof(struct col_in_select_node*) * 8, NULL, &malloced_head, the_debug);

			for (int i=0; i<8; i++)
			{
				col_arr[i] = (struct col_in_select_node*) myMalloc(sizeof(struct col_in_select_node), NULL, &malloced_head, the_debug);
				col_arr[i]->table_ptr = NULL;
				col_arr[i]->table_ptr_type = -1;
				col_arr[i]->col_ptr = NULL;
				col_arr[i]->col_ptr_type = -1;
				if (i == 0)
				{
					col_arr[i]->new_name = myMalloc(sizeof(char) * 16, NULL, &malloced_head, the_debug);
					strcpy(col_arr[i]->new_name, "avg func");
				}
				else if (i == 1)
					col_arr[i]->new_name = upper("COUNT ( DISTINCT * ) ", NULL, &malloced_head, the_debug);
				else if (i == 2)
					col_arr[i]->new_name = upper("FIRST ( EFFECTIVE ) ", NULL, &malloced_head, the_debug);
				else if (i == 3)
					col_arr[i]->new_name = upper("LAST_FUNC", NULL, &malloced_head, the_debug);
				else if (i == 4)
					col_arr[i]->new_name = upper("MIN ( CT-REGISTRATION-NUMBER ) ", NULL, &malloced_head, the_debug);
				else if (i == 5)
					col_arr[i]->new_name = upper("MAX ( EXPIRATION ) ", NULL, &malloced_head, the_debug);
				else if (i == 6)
					col_arr[i]->new_name = upper("MEDIAN ( CT-REGISTRATION-NUMBER ) ", NULL, &malloced_head, the_debug);
				else if (i == 7)
					col_arr[i]->new_name = upper("SUM ( CT-REGISTRATION-NUMBER ) ", NULL, &malloced_head, the_debug);
				col_arr[i]->math_node = NULL;
				col_arr[i]->case_node = NULL;

				col_arr[i]->func_node = (struct func_node*) myMalloc(sizeof(struct func_node), NULL, &malloced_head, the_debug);
				if (i == 0)
					col_arr[i]->func_node->which_func = FUNC_AVG;
				else if (i == 1)
					col_arr[i]->func_node->which_func = FUNC_COUNT;
				else if (i == 2)
					col_arr[i]->func_node->which_func = FUNC_FIRST;
				else if (i == 3)
					col_arr[i]->func_node->which_func = FUNC_LAST;
				else if (i == 4)
					col_arr[i]->func_node->which_func = FUNC_MIN;
				else if (i == 5)
					col_arr[i]->func_node->which_func = FUNC_MAX;
				else if (i == 6)
					col_arr[i]->func_node->which_func = FUNC_MEDIAN;
				else if (i == 7)
					col_arr[i]->func_node->which_func = FUNC_SUM;
				if (i == 1)
					col_arr[i]->func_node->distinct = true;
				else
					col_arr[i]->func_node->distinct = false;
				col_arr[i]->func_node->args_size = 1;
				col_arr[i]->func_node->args_arr = (void**) myMalloc(sizeof(void*), NULL, &malloced_head, the_debug);
				if (i == 0)
					col_arr[i]->func_node->args_arr[0] = ((struct select_node*) the_select_node)->columns_arr[1];
				else if (i == 1)
					col_arr[i]->func_node->args_arr[0] = upper("*", NULL, &malloced_head, the_debug);
				else if (i == 2)
					col_arr[i]->func_node->args_arr[0] = ((struct select_node*) the_select_node)->columns_arr[3];
				else if (i == 3)
					col_arr[i]->func_node->args_arr[0] = ((struct select_node*) the_select_node)->columns_arr[4];
				else if (i == 4)
					col_arr[i]->func_node->args_arr[0] = ((struct select_node*) the_select_node)->columns_arr[1];
				else if (i == 5)
					col_arr[i]->func_node->args_arr[0] = ((struct select_node*) the_select_node)->columns_arr[4];
				else if (i == 6)
					col_arr[i]->func_node->args_arr[0] = ((struct select_node*) the_select_node)->columns_arr[1];
				else if (i == 7)
					col_arr[i]->func_node->args_arr[0] = ((struct select_node*) the_select_node)->columns_arr[1];
				col_arr[i]->func_node->args_arr_type = (int*) myMalloc(sizeof(int), NULL, &malloced_head, the_debug);
				if (i == 1)
					col_arr[i]->func_node->args_arr_type[0] = PTR_TYPE_CHAR;
				else
					col_arr[i]->func_node->args_arr_type[0] = PTR_TYPE_COL_IN_SELECT_NODE;
				col_arr[i]->func_node->group_by_cols_head = NULL;
				col_arr[i]->func_node->group_by_cols_tail = NULL;
			}

			initSelectClauseForComp(&the_select_node->next, NULL, false, 8, col_arr, NULL, -1, NULL, NULL
								   ,&malloced_head, the_debug);
			the_select_node->next->prev = the_select_node;
			

			if (test_Controller_parseSelect(504, "select avg(CT-REGISTRATION-NUMBER) \"avg func\", count ( distinct * ), first(EFFECTIVE), last(EXPIRATION) last_func, min(CT-REGISTRATION-NUMBER), max(EXPIRATION), median(CT-REGISTRATION-NUMBER), sum(CT-REGISTRATION-NUMBER) from alc_brands;", &the_select_node
										   ,&parsed_error_code, &malloced_head, the_debug) != 0)
				return -1;


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
				return -3;
			}
		// END Test with id = 504

		// START Tests with ids = 505-511
			the_select_node = NULL;

			if (test_Controller_parseSelect(505, "select avg( BRAND-NAME ) from alc_brands;", &the_select_node
										   ,&parsed_error_code, &malloced_head, the_debug) != 0)
				return -1;

			if (test_Controller_parseSelect(506, "select sum(CT-REGISTRATION-NUMBER) ) from alc_brands;", &the_select_node
										   ,&parsed_error_code, &malloced_head, the_debug) != 0)
				return -1;

			if (test_Controller_parseSelect(507, "select min\n(\tCT-REGISTRATION-NUMBER,) from alc_brands;", &the_select_node
										   ,&parsed_error_code, &malloced_head, the_debug) != 0)
				return -1;

			if (test_Controller_parseSelect(508, "select count(distinct ) from alc_brands;", &the_select_node
										   ,&parsed_error_code, &malloced_head, the_debug) != 0)
				return -1;

			if (test_Controller_parseSelect(509, "select sum( (CT-REGISTRATION-NUMBER) from alc_brands;", &the_select_node
										   ,&parsed_error_code, &malloced_head, the_debug) != 0)
				return -1;

			if (test_Controller_parseSelect(510, "select sum(CT-REGISTRATION-NUMBER,CT-REGISTRATION-NUMBER) from alc_brands;", &the_select_node
										   ,&parsed_error_code, &malloced_head, the_debug) != 0)
				return -1;

			if (test_Controller_parseSelect(511, "select count(distinct BRAND-NAME), from alc_brands;", &the_select_node
										   ,&parsed_error_code, &malloced_head, the_debug) != 0)
				return -1;


			if (malloced_head != NULL)
			{
				if (the_debug == YES_DEBUG)
				{
					setOutputRed();
					printf("	ERROR in test_Driver_main() at line %d in %s\n", __LINE__, __FILE__);
					setOutputWhite();
				}
				return -3;
			}
		// END Tests with ids = 505-511

		// START Test with id = 512
			initSelectClauseForComp(&the_select_node, NULL, false, 0, NULL, getTablesHead(), PTR_TYPE_TABLE_INFO, NULL, NULL
								   ,&malloced_head, the_debug);

			col_arr = (struct col_in_select_node**) myMalloc(sizeof(struct col_in_select_node*) * 4, NULL, &malloced_head, the_debug);

			for (int i=0; i<4; i++)
			{
				col_arr[i] = (struct col_in_select_node*) myMalloc(sizeof(struct col_in_select_node), NULL, &malloced_head, the_debug);
				col_arr[i]->table_ptr = NULL;
				col_arr[i]->table_ptr_type = -1;
				col_arr[i]->col_ptr = NULL;
				col_arr[i]->col_ptr_type = -1;
				col_arr[i]->new_name = myMalloc(sizeof(char) * 64, NULL, &malloced_head, the_debug);
				if (i == 0)
					strcpy(col_arr[i]->new_name, "41");
				else if (i == 1)
					strcpy(col_arr[i]->new_name, "100000 - CT-REGISTRATION-NUMBER * 10 ");
				else if (i == 2)
					strcpy(col_arr[i]->new_name, "( 2 + 8 ) ^ 2 ");
				else if (i == 3)
					strcpy(col_arr[i]->new_name, "EFFECTIVE - 100 ");
				col_arr[i]->func_node = NULL;
				col_arr[i]->case_node = NULL;
			}

			col_arr[0]->math_node = (struct math_node*) myMalloc(sizeof(struct math_node), NULL, &malloced_head, the_debug);

			col_arr[0]->math_node->ptr_one = myMalloc(sizeof(int), NULL, &malloced_head, the_debug);
			*((int*) col_arr[0]->math_node->ptr_one) = 41;
			col_arr[0]->math_node->ptr_one_type = PTR_TYPE_INT;

			col_arr[0]->math_node->ptr_two = myMalloc(sizeof(struct math_node), NULL, &malloced_head, the_debug);
			col_arr[0]->math_node->ptr_two_type = PTR_TYPE_MATH_NODE;

			col_arr[0]->math_node->operation = MATH_ADD;
			col_arr[0]->math_node->parent = NULL;

				((struct math_node*) col_arr[0]->math_node->ptr_two)->ptr_one = myMalloc(sizeof(struct math_node), NULL, &malloced_head, the_debug);
				((struct math_node*) col_arr[0]->math_node->ptr_two)->ptr_one_type = PTR_TYPE_MATH_NODE;

				((struct math_node*) col_arr[0]->math_node->ptr_two)->ptr_two = myMalloc(sizeof(int), NULL, &malloced_head, the_debug);
				*((int*) ((struct math_node*) col_arr[0]->math_node->ptr_two)->ptr_two) = 10;
				((struct math_node*) col_arr[0]->math_node->ptr_two)->ptr_two_type = PTR_TYPE_INT;

				((struct math_node*) col_arr[0]->math_node->ptr_two)->operation = MATH_MULT;
				((struct math_node*) col_arr[0]->math_node->ptr_two)->parent = col_arr[0]->math_node;

					((struct math_node*) ((struct math_node*) col_arr[0]->math_node->ptr_two)->ptr_one)->ptr_one = myMalloc(sizeof(int), NULL, &malloced_head, the_debug);
					*((double*) ((struct math_node*) ((struct math_node*) col_arr[0]->math_node->ptr_two)->ptr_one)->ptr_one) = 110.5;
					((struct math_node*) ((struct math_node*) col_arr[0]->math_node->ptr_two)->ptr_one)->ptr_one_type = PTR_TYPE_REAL;

					((struct math_node*) ((struct math_node*) col_arr[0]->math_node->ptr_two)->ptr_one)->ptr_two = myMalloc(sizeof(int), NULL, &malloced_head, the_debug);
					*((int*) ((struct math_node*) ((struct math_node*) col_arr[0]->math_node->ptr_two)->ptr_one)->ptr_two) = 5;
					((struct math_node*) ((struct math_node*) col_arr[0]->math_node->ptr_two)->ptr_one)->ptr_two_type = PTR_TYPE_INT;

					((struct math_node*) ((struct math_node*) col_arr[0]->math_node->ptr_two)->ptr_one)->operation = MATH_DIV;
					((struct math_node*) ((struct math_node*) col_arr[0]->math_node->ptr_two)->ptr_one)->parent = col_arr[0]->math_node->ptr_two;

			col_arr[1]->math_node = (struct math_node*) myMalloc(sizeof(struct math_node), NULL, &malloced_head, the_debug);

			col_arr[1]->math_node->ptr_one = myMalloc(sizeof(int), NULL, &malloced_head, the_debug);
			*((int*) col_arr[1]->math_node->ptr_one) = 100000;
			col_arr[1]->math_node->ptr_one_type = PTR_TYPE_INT;

			col_arr[1]->math_node->ptr_two = myMalloc(sizeof(struct math_node), NULL, &malloced_head, the_debug);
			col_arr[1]->math_node->ptr_two_type = PTR_TYPE_MATH_NODE;

			col_arr[1]->math_node->operation = MATH_SUB;
			col_arr[1]->math_node->parent = NULL;

				((struct math_node*) col_arr[1]->math_node->ptr_two)->ptr_one = ((struct select_node*) the_select_node)->columns_arr[1];
				((struct math_node*) col_arr[1]->math_node->ptr_two)->ptr_one_type = PTR_TYPE_COL_IN_SELECT_NODE;

				((struct math_node*) col_arr[1]->math_node->ptr_two)->ptr_two = myMalloc(sizeof(int), NULL, &malloced_head, the_debug);
				*((int*) ((struct math_node*) col_arr[1]->math_node->ptr_two)->ptr_two) = 10;
				((struct math_node*) col_arr[1]->math_node->ptr_two)->ptr_two_type = PTR_TYPE_INT;

				((struct math_node*) col_arr[1]->math_node->ptr_two)->operation = MATH_MULT;
				((struct math_node*) col_arr[1]->math_node->ptr_two)->parent = col_arr[1]->math_node;

			col_arr[2]->math_node = (struct math_node*) myMalloc(sizeof(struct math_node), NULL, &malloced_head, the_debug);

			col_arr[2]->math_node->ptr_one = myMalloc(sizeof(struct math_node), NULL, &malloced_head, the_debug);
			col_arr[2]->math_node->ptr_one_type = PTR_TYPE_MATH_NODE;

			col_arr[2]->math_node->ptr_two = myMalloc(sizeof(int), NULL, &malloced_head, the_debug);
			*((int*) col_arr[2]->math_node->ptr_two) = 2;
			col_arr[2]->math_node->ptr_two_type = PTR_TYPE_INT;

			col_arr[2]->math_node->operation = MATH_POW;
			col_arr[2]->math_node->parent = NULL;

				((struct math_node*) col_arr[2]->math_node->ptr_one)->ptr_one = myMalloc(sizeof(int), NULL, &malloced_head, the_debug);
				*((int*) ((struct math_node*) col_arr[2]->math_node->ptr_one)->ptr_one) = 2;
				((struct math_node*) col_arr[2]->math_node->ptr_one)->ptr_one_type = PTR_TYPE_INT;

				((struct math_node*) col_arr[2]->math_node->ptr_one)->ptr_two = myMalloc(sizeof(int), NULL, &malloced_head, the_debug);
				*((int*) ((struct math_node*) col_arr[2]->math_node->ptr_one)->ptr_two) = 8;
				((struct math_node*) col_arr[2]->math_node->ptr_one)->ptr_two_type = PTR_TYPE_INT;

				((struct math_node*) col_arr[2]->math_node->ptr_one)->operation = MATH_ADD;
				((struct math_node*) col_arr[2]->math_node->ptr_one)->parent = col_arr[2]->math_node;

			col_arr[3]->math_node = (struct math_node*) myMalloc(sizeof(struct math_node), NULL, &malloced_head, the_debug);

			col_arr[3]->math_node->ptr_one = ((struct select_node*) the_select_node)->columns_arr[3];
			col_arr[3]->math_node->ptr_one_type = PTR_TYPE_COL_IN_SELECT_NODE;

			col_arr[3]->math_node->ptr_two = myMalloc(sizeof(int), NULL, &malloced_head, the_debug);
			*((int*) col_arr[3]->math_node->ptr_two) = 100;
			col_arr[3]->math_node->ptr_two_type = PTR_TYPE_INT;

			col_arr[3]->math_node->operation = MATH_SUB;
			col_arr[3]->math_node->parent = NULL;

			initSelectClauseForComp(&the_select_node->next, NULL, false, 4, col_arr, NULL, -1, NULL, NULL
								   ,&malloced_head, the_debug);
			the_select_node->next->prev = the_select_node;


			if (test_Controller_parseSelect(512, "select 41 + ((110.5 / 5) * 10) \"41\", 100000 - CT-REGISTRATION-NUMBER * 10, (2 + 8)^2, EFFECTIVE - 100 from alc_brands", &the_select_node
										   ,&parsed_error_code, &malloced_head, the_debug) != 0)
				return -1;


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
				return -3;
			}
		// END Test with id = 512

		// START Tests with ids = 513-518
			the_select_node = NULL;

			if (test_Controller_parseSelect(513, "select 10 + - 10 from alc_brands;", &the_select_node
										   ,&parsed_error_code, &malloced_head, the_debug) != 0)
				return -1;

			if (test_Controller_parseSelect(514, "select * 10 from alc_brands;", &the_select_node
										   ,&parsed_error_code, &malloced_head, the_debug) != 0)
				return -1;

			if (test_Controller_parseSelect(515, "select / 20 from alc_brands;", &the_select_node
										   ,&parsed_error_code, &malloced_head, the_debug) != 0)
				return -1;

			if (test_Controller_parseSelect(516, "select 10 * () - 20 from alc_brands;", &the_select_node
										   ,&parsed_error_code, &malloced_head, the_debug) != 0)
				return -1;

			if (test_Controller_parseSelect(517, "select 30 ^ as math_boi from alc_brands;", &the_select_node
										   ,&parsed_error_code, &malloced_head, the_debug) != 0)
				return -1;

			if (test_Controller_parseSelect(518, "select 40 ^ BRAND-NAME math_boi from alc_brands;", &the_select_node
										   ,&parsed_error_code, &malloced_head, the_debug) != 0)
				return -1;



			if (malloced_head != NULL)
			{
				if (the_debug == YES_DEBUG)
				{
					setOutputRed();
					printf("	ERROR in test_Driver_main() at line %d in %s\n", __LINE__, __FILE__);
					setOutputWhite();
				}
				return -3;
			}
		// END Tests with ids = 513-518

		// START Test with id = 519
			initSelectClauseForComp(&the_select_node, "TBL", false, 0, NULL, getTablesHead(), PTR_TYPE_TABLE_INFO, NULL, NULL
								   ,&malloced_head, the_debug);

			col_arr = (struct col_in_select_node**) myMalloc(sizeof(struct col_in_select_node*) * 1, NULL, &malloced_head, the_debug);

			for (int i=0; i<1; i++)
			{
				col_arr[i] = (struct col_in_select_node*) myMalloc(sizeof(struct col_in_select_node), NULL, &malloced_head, the_debug);
				col_arr[i]->table_ptr = NULL;
				col_arr[i]->table_ptr_type = -1;
				col_arr[i]->col_ptr = NULL;
				col_arr[i]->col_ptr_type = -1;
				col_arr[i]->new_name = myMalloc(sizeof(char) * 128, NULL, &malloced_head, the_debug);
				strcpy(col_arr[i]->new_name, "100000 - TBL.CT-REGISTRATION-NUMBER * 10 ");
				col_arr[i]->func_node = NULL;
				col_arr[i]->case_node = NULL;
			}

			col_arr[0]->math_node = (struct math_node*) myMalloc(sizeof(struct math_node), NULL, &malloced_head, the_debug);

			col_arr[0]->math_node->ptr_one = myMalloc(sizeof(int), NULL, &malloced_head, the_debug);
			*((int*) col_arr[0]->math_node->ptr_one) = 100000;
			col_arr[0]->math_node->ptr_one_type = PTR_TYPE_INT;

			col_arr[0]->math_node->ptr_two = myMalloc(sizeof(struct math_node), NULL, &malloced_head, the_debug);
			col_arr[0]->math_node->ptr_two_type = PTR_TYPE_MATH_NODE;

			col_arr[0]->math_node->operation = MATH_SUB;
			col_arr[0]->math_node->parent = NULL;

				((struct math_node*) col_arr[0]->math_node->ptr_two)->ptr_one = ((struct select_node*) the_select_node)->columns_arr[1];
				((struct math_node*) col_arr[0]->math_node->ptr_two)->ptr_one_type = PTR_TYPE_COL_IN_SELECT_NODE;

				((struct math_node*) col_arr[0]->math_node->ptr_two)->ptr_two = myMalloc(sizeof(int), NULL, &malloced_head, the_debug);
				*((int*) ((struct math_node*) col_arr[0]->math_node->ptr_two)->ptr_two) = 10;
				((struct math_node*) col_arr[0]->math_node->ptr_two)->ptr_two_type = PTR_TYPE_INT;

				((struct math_node*) col_arr[0]->math_node->ptr_two)->operation = MATH_MULT;
				((struct math_node*) col_arr[0]->math_node->ptr_two)->parent = col_arr[0]->math_node;

			initSelectClauseForComp(&the_select_node->next, NULL, false, 1, col_arr, NULL, -1, NULL, NULL
								   ,&malloced_head, the_debug);
			the_select_node->next->prev = the_select_node;


			if (test_Controller_parseSelect(519, "select 100000 - tbl.CT-REGISTRATION-NUMBER * 10 from alc_brands tbl;", &the_select_node
										   ,&parsed_error_code, &malloced_head, the_debug) != 0)
				return -1;


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
				return -3;
			}
		// END Test with id = 519

		// START Test with id = 520
			initSelectClauseForComp(&the_select_node, NULL, false, 0, NULL, getTablesHead(), PTR_TYPE_TABLE_INFO, NULL, NULL
								   ,&malloced_head, the_debug);

			col_arr = (struct col_in_select_node**) myMalloc(sizeof(struct col_in_select_node*) * 1, NULL, &malloced_head, the_debug);

			for (int i=0; i<1; i++)
			{
				col_arr[i] = (struct col_in_select_node*) myMalloc(sizeof(struct col_in_select_node), NULL, &malloced_head, the_debug);
				col_arr[i]->table_ptr = NULL;
				col_arr[i]->table_ptr_type = -1;
				col_arr[i]->col_ptr = NULL;
				col_arr[i]->col_ptr_type = -1;
				col_arr[i]->new_name = myMalloc(sizeof(char) * 128, NULL, &malloced_head, the_debug);
				strcpy(col_arr[i]->new_name, "100000 - ALC_BRANDS.CT-REGISTRATION-NUMBER * 10 ");
				col_arr[i]->func_node = NULL;
				col_arr[i]->case_node = NULL;
			}

			col_arr[0]->math_node = (struct math_node*) myMalloc(sizeof(struct math_node), NULL, &malloced_head, the_debug);

			col_arr[0]->math_node->ptr_one = myMalloc(sizeof(int), NULL, &malloced_head, the_debug);
			*((int*) col_arr[0]->math_node->ptr_one) = 100000;
			col_arr[0]->math_node->ptr_one_type = PTR_TYPE_INT;

			col_arr[0]->math_node->ptr_two = myMalloc(sizeof(struct math_node), NULL, &malloced_head, the_debug);
			col_arr[0]->math_node->ptr_two_type = PTR_TYPE_MATH_NODE;

			col_arr[0]->math_node->operation = MATH_SUB;
			col_arr[0]->math_node->parent = NULL;

				((struct math_node*) col_arr[0]->math_node->ptr_two)->ptr_one = ((struct select_node*) the_select_node)->columns_arr[1];
				((struct math_node*) col_arr[0]->math_node->ptr_two)->ptr_one_type = PTR_TYPE_COL_IN_SELECT_NODE;

				((struct math_node*) col_arr[0]->math_node->ptr_two)->ptr_two = myMalloc(sizeof(int), NULL, &malloced_head, the_debug);
				*((int*) ((struct math_node*) col_arr[0]->math_node->ptr_two)->ptr_two) = 10;
				((struct math_node*) col_arr[0]->math_node->ptr_two)->ptr_two_type = PTR_TYPE_INT;

				((struct math_node*) col_arr[0]->math_node->ptr_two)->operation = MATH_MULT;
				((struct math_node*) col_arr[0]->math_node->ptr_two)->parent = col_arr[0]->math_node;

			initSelectClauseForComp(&the_select_node->next, NULL, false, 1, col_arr, NULL, -1, NULL, NULL
								   ,&malloced_head, the_debug);
			the_select_node->next->prev = the_select_node;


			if (test_Controller_parseSelect(520, "select 100000 - alc_brands.CT-REGISTRATION-NUMBER * 10 from alc_brands;", &the_select_node
										   ,&parsed_error_code, &malloced_head, the_debug) != 0)
				return -1;


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
				return -3;
			}
		// END Test with id = 520

		// START Test with id = 521
			initSelectClauseForComp(&the_select_node, NULL, false, 0, NULL, getTablesHead(), PTR_TYPE_TABLE_INFO, NULL, NULL
								   ,&malloced_head, the_debug);

			col_arr = (struct col_in_select_node**) myMalloc(sizeof(struct col_in_select_node*) * 6, NULL, &malloced_head, the_debug);

			int index = 0;
			for (int i=0; i<7; i++)
			{
				if (i != 4)
				{
					col_arr[index] = (struct col_in_select_node*) myMalloc(sizeof(struct col_in_select_node), NULL, &malloced_head, the_debug);

					col_arr[index]->table_ptr = the_select_node;
					col_arr[index]->table_ptr_type = PTR_TYPE_SELECT_NODE;

					col_arr[index]->col_ptr = ((struct select_node*) the_select_node)->columns_arr[i];
					col_arr[index]->col_ptr_type = PTR_TYPE_COL_IN_SELECT_NODE;

					col_arr[index]->new_name = NULL;

					col_arr[index]->func_node = NULL;
					col_arr[index]->math_node = NULL;
					col_arr[index]->case_node = NULL;

					index++;
				}
			}

			initSelectClauseForComp(&the_select_node->next, NULL, false, 6, col_arr, NULL, -1, NULL, NULL
								   ,&malloced_head, the_debug);
			the_select_node->next->prev = the_select_node;


			if (test_Controller_parseSelect(521, "select * except EXPIRATION from alc_brands;", &the_select_node
										   ,&parsed_error_code, &malloced_head, the_debug) != 0)
				return -1;

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
				return -3;
			}	
		// END Test with id = 521

		// START Test with id = 522
			initSelectClauseForComp(&the_select_node, NULL, false, 0, NULL, getTablesHead(), PTR_TYPE_TABLE_INFO, NULL, NULL
								   ,&malloced_head, the_debug);

			col_arr = (struct col_in_select_node**) myMalloc(sizeof(struct col_in_select_node*) * 5, NULL, &malloced_head, the_debug);

			index = 0;
			for (int i=0; i<7; i++)
			{
				if (i != 4 && i != 1)
				{
					col_arr[index] = (struct col_in_select_node*) myMalloc(sizeof(struct col_in_select_node), NULL, &malloced_head, the_debug);

					col_arr[index]->table_ptr = the_select_node;
					col_arr[index]->table_ptr_type = PTR_TYPE_SELECT_NODE;

					col_arr[index]->col_ptr = ((struct select_node*) the_select_node)->columns_arr[i];
					col_arr[index]->col_ptr_type = PTR_TYPE_COL_IN_SELECT_NODE;

					col_arr[index]->new_name = NULL;

					col_arr[index]->func_node = NULL;
					col_arr[index]->math_node = NULL;
					col_arr[index]->case_node = NULL;

					index++;
				}
			}

			initSelectClauseForComp(&the_select_node->next, NULL, false, 5, col_arr, NULL, -1, NULL, NULL
								   ,&malloced_head, the_debug);
			the_select_node->next->prev = the_select_node;


			if (test_Controller_parseSelect(522, "select * except EXPIRATION,CT-REGISTRATION-NUMBER from alc_brands;", &the_select_node
										   ,&parsed_error_code, &malloced_head, the_debug) != 0)
				return -1;

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
				return -3;
			}	
		// END Test with id = 522

		// START Test with id = 523
			initSelectClauseForComp(&the_select_node, "TBL", false, 0, NULL, getTablesHead(), PTR_TYPE_TABLE_INFO, NULL, NULL
								   ,&malloced_head, the_debug);

			struct select_node* joined_select = NULL;

			initSelectClauseForComp(&joined_select, "TBL2", false, 0, NULL, getTablesHead(), PTR_TYPE_TABLE_INFO, NULL, NULL
								   ,&malloced_head, the_debug);

			col_arr = (struct col_in_select_node**) myMalloc(sizeof(struct col_in_select_node*) * 4, NULL, &malloced_head, the_debug);
			
			for (int i=0; i<4; i++)
			{
				col_arr[i] = (struct col_in_select_node*) myMalloc(sizeof(struct col_in_select_node), NULL, &malloced_head, the_debug);
				if (i == 0)
				{
					col_arr[i]->table_ptr = the_select_node;
					col_arr[i]->col_ptr = the_select_node->columns_arr[0];
				}
				else if (i == 1)
				{
					col_arr[i]->table_ptr = the_select_node;
					col_arr[i]->col_ptr = the_select_node->columns_arr[6];
				}
				else if (i == 2)
				{
					col_arr[i]->table_ptr = joined_select;
					col_arr[i]->col_ptr = joined_select->columns_arr[0];
				}
				else if (i == 3)
				{
					col_arr[i]->table_ptr = joined_select;
					col_arr[i]->col_ptr = joined_select->columns_arr[6];
				}
				col_arr[i]->table_ptr_type = PTR_TYPE_SELECT_NODE;
				col_arr[i]->col_ptr_type = PTR_TYPE_COL_IN_SELECT_NODE;
				col_arr[i]->new_name = NULL;
				col_arr[i]->func_node = NULL;
				col_arr[i]->math_node = NULL;
				col_arr[i]->case_node = NULL;
			}

			initSelectClauseForComp(&the_select_node->next, NULL, false, 4, col_arr, NULL, -1, NULL, NULL
								   ,&malloced_head, the_debug);
			the_select_node->next->prev = the_select_node;

			the_select_node->next->join_head = (struct join_node*) myMalloc(sizeof(struct join_node), NULL, &malloced_head, the_debug);
			the_select_node->next->join_head->join_type = JOIN_INNER;
			the_select_node->next->join_head->select_joined = joined_select;
			the_select_node->next->join_head->prev = NULL;
			the_select_node->next->join_head->next = NULL;

			the_select_node->next->join_head->on_clause_head = (struct where_clause_node*) myMalloc(sizeof(struct where_clause_node), NULL, &malloced_head, the_debug);
			the_select_node->next->join_head->on_clause_head->ptr_one = the_select_node->columns_arr[0];
			the_select_node->next->join_head->on_clause_head->ptr_one_type = PTR_TYPE_COL_IN_SELECT_NODE;
			the_select_node->next->join_head->on_clause_head->ptr_two = joined_select->columns_arr[0];
			the_select_node->next->join_head->on_clause_head->ptr_two_type = PTR_TYPE_COL_IN_SELECT_NODE;
			the_select_node->next->join_head->on_clause_head->where_type = WHERE_IS_EQUALS;
			the_select_node->next->join_head->on_clause_head->parent = NULL;


			if (test_Controller_parseSelect(523, "select tbl.Brand-name, tbl.SUPERVISOR-CREDENTIAL, tbl2.Brand-name, tbl2.SUPERVISOR-CREDENTIAL from alc_brands tbl join alc_brands tbl2 on tbl.Brand-name = tbl2.Brand-name;", &the_select_node
										   ,&parsed_error_code, &malloced_head, the_debug) != 0)
				return -1;

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
				return -3;
			}	
		// END Test with id = 523

		// START Test with id = 524
			the_select_node = NULL;


			if (test_Controller_parseSelect(524, "select tbl.Brand-name, tbl.SUPERVISOR-CREDENTIAL, tbl2.Brand-name, tbl3.SUPERVISOR-CREDENTIAL from alc_brands tbl join alc_brands tbl2 on tbl.Brand-name = tbl2.Brand-name;", &the_select_node
										   ,&parsed_error_code, &malloced_head, the_debug) != 0)
				return -1;

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
				return -3;
			}	
		// END Test with id = 524

		// START Test with id = 525
			the_select_node = NULL;


			if (test_Controller_parseSelect(525, "select tbl.Brand-name, tbl.SUPERVISOR-CREDENTIAL, Brand-name, tbl2.SUPERVISOR-CREDENTIAL from alc_brands tbl join alc_brands tbl2 on tbl.Brand-name = tbl2.Brand-name;", &the_select_node
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
				return -3;
			}
		// END Test with id = 525

		// START Test with id = 526
			initSelectClauseForComp(&the_select_node, "TBL", false, 0, NULL, getTablesHead(), PTR_TYPE_TABLE_INFO, NULL, NULL
								   ,&malloced_head, the_debug);

			joined_select = NULL;

			initSelectClauseForComp(&joined_select, "TBL2", false, 0, NULL, getTablesHead(), PTR_TYPE_TABLE_INFO, NULL, NULL
								   ,&malloced_head, the_debug);

			struct select_node* joined_select_3 = NULL;

			initSelectClauseForComp(&joined_select_3, "TBL3", false, 0, NULL, getTablesHead(), PTR_TYPE_TABLE_INFO, NULL, NULL
								   ,&malloced_head, the_debug);

			col_arr = (struct col_in_select_node**) myMalloc(sizeof(struct col_in_select_node*) * 5, NULL, &malloced_head, the_debug);
			
			for (int i=0; i<5; i++)
			{
				col_arr[i] = (struct col_in_select_node*) myMalloc(sizeof(struct col_in_select_node), NULL, &malloced_head, the_debug);
				if (i == 0)
				{
					col_arr[i]->table_ptr = the_select_node;
					col_arr[i]->col_ptr = the_select_node->columns_arr[0];
				}
				else if (i == 1)
				{
					col_arr[i]->table_ptr = the_select_node;
					col_arr[i]->col_ptr = the_select_node->columns_arr[6];
				}
				else if (i == 2)
				{
					col_arr[i]->table_ptr = joined_select;
					col_arr[i]->col_ptr = joined_select->columns_arr[0];
				}
				else if (i == 3)
				{
					col_arr[i]->table_ptr = joined_select;
					col_arr[i]->col_ptr = joined_select->columns_arr[6];
				}
				else if (i == 4)
				{
					col_arr[i]->table_ptr = joined_select_3;
					col_arr[i]->col_ptr = joined_select_3->columns_arr[1];
				}
				col_arr[i]->table_ptr_type = PTR_TYPE_SELECT_NODE;
				col_arr[i]->col_ptr_type = PTR_TYPE_COL_IN_SELECT_NODE;
				col_arr[i]->new_name = NULL;
				col_arr[i]->func_node = NULL;
				col_arr[i]->math_node = NULL;
				col_arr[i]->case_node = NULL;
			}

			initSelectClauseForComp(&the_select_node->next, NULL, false, 5, col_arr, NULL, -1, NULL, NULL
								   ,&malloced_head, the_debug);
			the_select_node->next->prev = the_select_node;

			the_select_node->next->join_head = (struct join_node*) myMalloc(sizeof(struct join_node), NULL, &malloced_head, the_debug);
			the_select_node->next->join_head->join_type = JOIN_RIGHT;
			the_select_node->next->join_head->select_joined = joined_select;
			the_select_node->next->join_head->prev = NULL;

			the_select_node->next->join_head->on_clause_head = (struct where_clause_node*) myMalloc(sizeof(struct where_clause_node), NULL, &malloced_head, the_debug);
			the_select_node->next->join_head->on_clause_head->ptr_one = the_select_node->columns_arr[0];
			the_select_node->next->join_head->on_clause_head->ptr_one_type = PTR_TYPE_COL_IN_SELECT_NODE;
			the_select_node->next->join_head->on_clause_head->ptr_two = joined_select->columns_arr[0];
			the_select_node->next->join_head->on_clause_head->ptr_two_type = PTR_TYPE_COL_IN_SELECT_NODE;
			the_select_node->next->join_head->on_clause_head->where_type = WHERE_IS_EQUALS;
			the_select_node->next->join_head->on_clause_head->parent = NULL;

			the_select_node->next->join_head->next = (struct join_node*) myMalloc(sizeof(struct join_node), NULL, &malloced_head, the_debug);
			the_select_node->next->join_head->next->next = NULL;
			the_select_node->next->join_head->next->prev = the_select_node->next->join_head;
			the_select_node->next->join_head->next->join_type = JOIN_LEFT;
			the_select_node->next->join_head->next->select_joined = joined_select_3;

			the_select_node->next->join_head->next->on_clause_head = (struct where_clause_node*) myMalloc(sizeof(struct where_clause_node), NULL, &malloced_head, the_debug);
			the_select_node->next->join_head->next->on_clause_head->ptr_one = the_select_node->columns_arr[1];
			the_select_node->next->join_head->next->on_clause_head->ptr_one_type = PTR_TYPE_COL_IN_SELECT_NODE;
			the_select_node->next->join_head->next->on_clause_head->ptr_two = joined_select_3->columns_arr[1];
			the_select_node->next->join_head->next->on_clause_head->ptr_two_type = PTR_TYPE_COL_IN_SELECT_NODE;
			the_select_node->next->join_head->next->on_clause_head->where_type = WHERE_IS_EQUALS;
			the_select_node->next->join_head->next->on_clause_head->parent = NULL;


			if (test_Controller_parseSelect(526, "select tbl.Brand-name, tbl.SUPERVISOR-CREDENTIAL, tbl2.Brand-name, tbl2.SUPERVISOR-CREDENTIAL, tbl3.CT-REGISTRATION-NUMBER from alc_brands tbl right join alc_brands tbl2 on tbl.Brand-name = tbl2.Brand-name left join alc_brands tbl3	on tbl.CT-REGISTRATION-NUMBER = tbl3.CT-REGISTRATION-NUMBER;", &the_select_node
										   ,&parsed_error_code, &malloced_head, the_debug) != 0)
				return -1;

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
				return -3;
			}
		// END Test with id = 526

		// START Test with id = 527
			the_select_node = NULL;


			if (test_Controller_parseSelect(527, "select tbl.Brand-name, tbl.SUPERVISOR-CREDENTIAL, tbl2.Brand-name, tbl2.SUPERVISOR-CREDENTIAL, tbl3.CT-REGISTRATION-NUMBER from alc_brands tbl join alc_brands tbl2 on tbl.Brand-name = tbl3.Brand-name left join alc_brands tbl3	on tbl.CT-REGISTRATION-NUMBER = tbl3.CT-REGISTRATION-NUMBER;", &the_select_node
										   ,&parsed_error_code, &malloced_head, the_debug) != 0)
				return -1;

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
				return -3;
			}
		// END Test with id = 527

		// START Test with id = 528
			initSelectClauseForComp(&the_select_node, "TBL", false, 0, NULL, getTablesHead(), PTR_TYPE_TABLE_INFO, NULL, NULL
								   ,&malloced_head, the_debug);
			initSelectClauseForComp(&the_select_node->next, NULL, false, 0, NULL, the_select_node, PTR_TYPE_SELECT_NODE, NULL, NULL
								   ,&malloced_head, the_debug);
			the_select_node->next->prev = the_select_node;


			if (test_Controller_parseSelect(528, "select tbl.* from alc_brands tbl;", &the_select_node
										   ,&parsed_error_code, &malloced_head, the_debug) != 0)
				return -1;

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
				return -3;
			}
		// END Test with id = 528

		// START Test with id = 529
			initSelectClauseForComp(&the_select_node, "TBL", false, 0, NULL, getTablesHead(), PTR_TYPE_TABLE_INFO, NULL, NULL
								   ,&malloced_head, the_debug);
			initSelectClauseForComp(&the_select_node->next, NULL, false, 0, NULL, the_select_node, PTR_TYPE_SELECT_NODE, NULL, NULL
								   ,&malloced_head, the_debug);
			the_select_node->next->prev = the_select_node;

			joined_select = NULL;
			initSelectClauseForComp(&joined_select, "TBL2", false, 0, NULL, getTablesHead(), PTR_TYPE_TABLE_INFO, NULL, NULL
								   ,&malloced_head, the_debug);

			the_select_node->next->join_head = (struct join_node*) myMalloc(sizeof(struct join_node), NULL, &malloced_head, the_debug);
			the_select_node->next->join_head->join_type = JOIN_INNER;
			the_select_node->next->join_head->select_joined = joined_select;
			the_select_node->next->join_head->prev = NULL;
			the_select_node->next->join_head->next = NULL;

			the_select_node->next->join_head->on_clause_head = (struct where_clause_node*) myMalloc(sizeof(struct where_clause_node), NULL, &malloced_head, the_debug);
			the_select_node->next->join_head->on_clause_head->ptr_one = the_select_node->columns_arr[0];
			the_select_node->next->join_head->on_clause_head->ptr_one_type = PTR_TYPE_COL_IN_SELECT_NODE;
			the_select_node->next->join_head->on_clause_head->ptr_two = joined_select->columns_arr[0];
			the_select_node->next->join_head->on_clause_head->ptr_two_type = PTR_TYPE_COL_IN_SELECT_NODE;
			the_select_node->next->join_head->on_clause_head->where_type = WHERE_IS_EQUALS;
			the_select_node->next->join_head->on_clause_head->parent = NULL;


			if (test_Controller_parseSelect(529, "select tbl.* from alc_brands tbl join alc_brands tbl2 on tbl.Brand-name = tbl2.Brand-name;", &the_select_node
										   ,&parsed_error_code, &malloced_head, the_debug) != 0)
				return -1;

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
				return -3;
			}
		// END Test with id = 529

		// START Test with id = 530
			initSelectClauseForComp(&the_select_node, "TBL", false, 0, NULL, getTablesHead(), PTR_TYPE_TABLE_INFO, NULL, NULL
								   ,&malloced_head, the_debug);

			joined_select = NULL;
			initSelectClauseForComp(&joined_select, "TBL2", false, 0, NULL, getTablesHead(), PTR_TYPE_TABLE_INFO, NULL, NULL
								   ,&malloced_head, the_debug);

			col_arr = (struct col_in_select_node**) myMalloc(sizeof(struct col_in_select_node*) * 14, NULL, &malloced_head, the_debug);

			index = 0;
			for (int i=0; i<14; i++)
			{
				col_arr[index] = (struct col_in_select_node*) myMalloc(sizeof(struct col_in_select_node), NULL, &malloced_head, the_debug);

				if (i < 7)
				{
					col_arr[index]->table_ptr = the_select_node;
					col_arr[index]->table_ptr_type = PTR_TYPE_SELECT_NODE;

					col_arr[index]->col_ptr = ((struct select_node*) the_select_node)->columns_arr[i];
					col_arr[index]->col_ptr_type = PTR_TYPE_COL_IN_SELECT_NODE;
				}
				else
				{
					col_arr[index]->table_ptr = joined_select;
					col_arr[index]->table_ptr_type = PTR_TYPE_SELECT_NODE;

					col_arr[index]->col_ptr = ((struct select_node*) joined_select)->columns_arr[i-7];
					col_arr[index]->col_ptr_type = PTR_TYPE_COL_IN_SELECT_NODE;
				}

				col_arr[index]->new_name = NULL;
				col_arr[index]->func_node = NULL;
				col_arr[index]->math_node = NULL;
				col_arr[index]->case_node = NULL;

				index++;
			}

			initSelectClauseForComp(&the_select_node->next, NULL, false, 14, col_arr, NULL, -1, NULL, NULL
								   ,&malloced_head, the_debug);
			the_select_node->next->prev = the_select_node;

			the_select_node->next->join_head = (struct join_node*) myMalloc(sizeof(struct join_node), NULL, &malloced_head, the_debug);
			the_select_node->next->join_head->join_type = JOIN_INNER;
			the_select_node->next->join_head->select_joined = joined_select;
			the_select_node->next->join_head->prev = NULL;
			the_select_node->next->join_head->next = NULL;

			the_select_node->next->join_head->on_clause_head = (struct where_clause_node*) myMalloc(sizeof(struct where_clause_node), NULL, &malloced_head, the_debug);
			the_select_node->next->join_head->on_clause_head->ptr_one = the_select_node->columns_arr[0];
			the_select_node->next->join_head->on_clause_head->ptr_one_type = PTR_TYPE_COL_IN_SELECT_NODE;
			the_select_node->next->join_head->on_clause_head->ptr_two = joined_select->columns_arr[0];
			the_select_node->next->join_head->on_clause_head->ptr_two_type = PTR_TYPE_COL_IN_SELECT_NODE;
			the_select_node->next->join_head->on_clause_head->where_type = WHERE_IS_EQUALS;
			the_select_node->next->join_head->on_clause_head->parent = NULL;


			if (test_Controller_parseSelect(530, "select * from alc_brands tbl join alc_brands tbl2 on tbl.Brand-name = tbl2.Brand-name;", &the_select_node
										   ,&parsed_error_code, &malloced_head, the_debug) != 0)
				return -1;

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
				return -3;
			}
		// END Test with id = 530

		// START Test with id = 531
			initSelectClauseForComp(&the_select_node, NULL, false, 0, NULL, getTablesHead(), PTR_TYPE_TABLE_INFO, NULL, NULL
								   ,&malloced_head, the_debug);
			initSelectClauseForComp(&the_select_node->next, "TBL", false, 0, NULL, the_select_node, PTR_TYPE_SELECT_NODE, NULL, NULL
								   ,&malloced_head, the_debug);
			the_select_node->next->prev = the_select_node;
			initSelectClauseForComp(&the_select_node->next->next, NULL, false, 0, NULL, the_select_node->next, PTR_TYPE_SELECT_NODE, NULL, NULL
								   ,&malloced_head, the_debug);
			the_select_node->next->next->prev = the_select_node->next;


			if (test_Controller_parseSelect(531, "select * from ( select * from alc_brands ) tbl;", &the_select_node
										   ,&parsed_error_code, &malloced_head, the_debug) != 0)
				return -1;

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
				return -3;
			}
		// END Test with id = 531

		// START Test with id = 532
			initSelectClauseForComp(&the_select_node, NULL, false, 0, NULL, getTablesHead(), PTR_TYPE_TABLE_INFO, NULL, NULL
								   ,&malloced_head, the_debug);
			initSelectClauseForComp(&the_select_node->next, "TBL", false, 0, NULL, the_select_node, PTR_TYPE_SELECT_NODE, NULL, NULL
								   ,&malloced_head, the_debug);
			the_select_node->next->prev = the_select_node;

			initSelectClauseForComp(&joined_select, "TBL2", false, 0, NULL, getTablesHead(), PTR_TYPE_TABLE_INFO, NULL, NULL
								   ,&malloced_head, the_debug);

			col_arr = (struct col_in_select_node**) myMalloc(sizeof(struct col_in_select_node*) * 4, NULL, &malloced_head, the_debug);

			index = 0;
			for (int i=0; i<4; i++)
			{
				col_arr[index] = (struct col_in_select_node*) myMalloc(sizeof(struct col_in_select_node), NULL, &malloced_head, the_debug);

				if (i == 0)
				{
					col_arr[index]->table_ptr = the_select_node->next;
					col_arr[index]->table_ptr_type = PTR_TYPE_SELECT_NODE;

					col_arr[index]->col_ptr = the_select_node->next->columns_arr[0];
					col_arr[index]->col_ptr_type = PTR_TYPE_COL_IN_SELECT_NODE;
				}
				else if (i == 1)
				{
					col_arr[index]->table_ptr = joined_select;
					col_arr[index]->table_ptr_type = PTR_TYPE_SELECT_NODE;

					col_arr[index]->col_ptr = joined_select->columns_arr[0];
					col_arr[index]->col_ptr_type = PTR_TYPE_COL_IN_SELECT_NODE;
				}
				else if (i == 2)
				{
					col_arr[index]->table_ptr = the_select_node->next;
					col_arr[index]->table_ptr_type = PTR_TYPE_SELECT_NODE;

					col_arr[index]->col_ptr = the_select_node->next->columns_arr[1];
					col_arr[index]->col_ptr_type = PTR_TYPE_COL_IN_SELECT_NODE;
				}
				else if (i == 3)
				{
					col_arr[index]->table_ptr = joined_select;
					col_arr[index]->table_ptr_type = PTR_TYPE_SELECT_NODE;

					col_arr[index]->col_ptr = joined_select->columns_arr[1];
					col_arr[index]->col_ptr_type = PTR_TYPE_COL_IN_SELECT_NODE;
				}

				col_arr[index]->new_name = NULL;
				col_arr[index]->func_node = NULL;
				col_arr[index]->math_node = NULL;
				col_arr[index]->case_node = NULL;

				index++;
			}

			initSelectClauseForComp(&the_select_node->next->next, NULL, false, 4, col_arr, NULL, -1, NULL, NULL
								   ,&malloced_head, the_debug);
			the_select_node->next->next->prev = the_select_node->next;

			the_select_node->next->next->join_head = (struct join_node*) myMalloc(sizeof(struct join_node), NULL, &malloced_head, the_debug);
			the_select_node->next->next->join_head->join_type = JOIN_INNER;
			the_select_node->next->next->join_head->select_joined = joined_select;
			the_select_node->next->next->join_head->prev = NULL;
			the_select_node->next->next->join_head->next = NULL;

			the_select_node->next->next->join_head->on_clause_head = (struct where_clause_node*) myMalloc(sizeof(struct where_clause_node), NULL, &malloced_head, the_debug);
			the_select_node->next->next->join_head->on_clause_head->ptr_one = joined_select->columns_arr[0];
			the_select_node->next->next->join_head->on_clause_head->ptr_one_type = PTR_TYPE_COL_IN_SELECT_NODE;
			the_select_node->next->next->join_head->on_clause_head->ptr_two = the_select_node->next->columns_arr[0];
			the_select_node->next->next->join_head->on_clause_head->ptr_two_type = PTR_TYPE_COL_IN_SELECT_NODE;
			the_select_node->next->next->join_head->on_clause_head->where_type = WHERE_IS_EQUALS;
			the_select_node->next->next->join_head->on_clause_head->parent = NULL;

			
			if (test_Controller_parseSelect(532, "select tbl.Brand-name, tbl2.Brand-name, tbl.CT-REGISTRATION-NUMBER, tbl2.CT-REGISTRATION-NUMBER from ( select * from alc_brands ) tbl join alc_brands tbl2 on tbl2.Brand-name = tbl.Brand-name;", &the_select_node
										   ,&parsed_error_code, &malloced_head, the_debug) != 0)
				return -1;

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
				return -3;
			}
		// END Test with id = 532

		// START Test with id = 533
			initSelectClauseForComp(&the_select_node, "TBL", false, 0, NULL, getTablesHead(), PTR_TYPE_TABLE_INFO, NULL, NULL
								   ,&malloced_head, the_debug);

			initSelectClauseForComp(&joined_select, NULL, false, 0, NULL, getTablesHead(), PTR_TYPE_TABLE_INFO, NULL, NULL
								   ,&malloced_head, the_debug);
			initSelectClauseForComp(&joined_select->next, "TBL2", false, 0, NULL, joined_select, PTR_TYPE_SELECT_NODE, NULL, NULL
								   ,&malloced_head, the_debug);
			joined_select->next->prev = joined_select;

			col_arr = (struct col_in_select_node**) myMalloc(sizeof(struct col_in_select_node*) * 9, NULL, &malloced_head, the_debug);

			index = 0;
			for (int i=0; i<9; i++)
			{
				col_arr[index] = (struct col_in_select_node*) myMalloc(sizeof(struct col_in_select_node), NULL, &malloced_head, the_debug);

				if (i == 0)
				{
					col_arr[index]->table_ptr = the_select_node;
					col_arr[index]->table_ptr_type = PTR_TYPE_SELECT_NODE;

					col_arr[index]->col_ptr = the_select_node->columns_arr[0];
					col_arr[index]->col_ptr_type = PTR_TYPE_COL_IN_SELECT_NODE;
				}
				else if (i == 1)
				{
					col_arr[index]->table_ptr = the_select_node;
					col_arr[index]->table_ptr_type = PTR_TYPE_SELECT_NODE;

					col_arr[index]->col_ptr = the_select_node->columns_arr[5];
					col_arr[index]->col_ptr_type = PTR_TYPE_COL_IN_SELECT_NODE;
				}
				else if (i > 1)
				{
					col_arr[index]->table_ptr = joined_select->next;
					col_arr[index]->table_ptr_type = PTR_TYPE_SELECT_NODE;

					col_arr[index]->col_ptr = joined_select->next->columns_arr[i-2];
					col_arr[index]->col_ptr_type = PTR_TYPE_COL_IN_SELECT_NODE;
				}

				col_arr[index]->new_name = NULL;
				col_arr[index]->func_node = NULL;
				col_arr[index]->math_node = NULL;
				col_arr[index]->case_node = NULL;

				index++;
			}

			initSelectClauseForComp(&the_select_node->next, NULL, false, 9, col_arr, NULL, -1, NULL, NULL
								   ,&malloced_head, the_debug);
			the_select_node->next->prev = the_select_node;

			the_select_node->next->join_head = (struct join_node*) myMalloc(sizeof(struct join_node), NULL, &malloced_head, the_debug);
			the_select_node->next->join_head->join_type = JOIN_INNER;
			the_select_node->next->join_head->select_joined = joined_select->next;
			the_select_node->next->join_head->prev = NULL;
			the_select_node->next->join_head->next = NULL;

			the_select_node->next->join_head->on_clause_head = (struct where_clause_node*) myMalloc(sizeof(struct where_clause_node), NULL, &malloced_head, the_debug);
			the_select_node->next->join_head->on_clause_head->ptr_one = the_select_node->columns_arr[0];
			the_select_node->next->join_head->on_clause_head->ptr_one_type = PTR_TYPE_COL_IN_SELECT_NODE;
			the_select_node->next->join_head->on_clause_head->ptr_two = joined_select->next->columns_arr[0];
			the_select_node->next->join_head->on_clause_head->ptr_two_type = PTR_TYPE_COL_IN_SELECT_NODE;
			the_select_node->next->join_head->on_clause_head->where_type = WHERE_IS_EQUALS;
			the_select_node->next->join_head->on_clause_head->parent = NULL;


			if (test_Controller_parseSelect(533, "select tbl.Brand-name, tbl.OUT-OF-STATE-SHIPPER, tbl2.* from ALC_Brands tbl join ( select * from alc_brands ) tbl2 on tbl.Brand-name = tbl2.Brand-name;", &the_select_node
										   ,&parsed_error_code, &malloced_head, the_debug) != 0)
				return -1;

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
				return -3;
			}
		// END Test with id = 533

		// START Test with id = 534
			initSelectClauseForComp(&the_select_node, NULL, false, 0, NULL, getTablesHead(), PTR_TYPE_TABLE_INFO, NULL, NULL
								   ,&malloced_head, the_debug);

			col_arr = (struct col_in_select_node**) myMalloc(sizeof(struct col_in_select_node*) * 2, NULL, &malloced_head, the_debug);

			for (int i=0; i<2; i++)
			{
				col_arr[i] = (struct col_in_select_node*) myMalloc(sizeof(struct col_in_select_node), NULL, &malloced_head, the_debug);

				col_arr[i]->table_ptr = the_select_node;
				col_arr[i]->table_ptr_type = PTR_TYPE_SELECT_NODE;

				if (i == 0)
				{
					col_arr[i]->col_ptr = ((struct select_node*) the_select_node)->columns_arr[1];
					col_arr[i]->col_ptr_type = PTR_TYPE_COL_IN_SELECT_NODE;
				}
				else
				{
					col_arr[i]->col_ptr = ((struct select_node*) the_select_node)->columns_arr[0];
					col_arr[i]->col_ptr_type = PTR_TYPE_COL_IN_SELECT_NODE;
				}

				col_arr[i]->new_name = NULL;
				col_arr[i]->func_node = NULL;
				col_arr[i]->math_node = NULL;
				col_arr[i]->case_node = NULL;
			}

			initSelectClauseForComp(&the_select_node->next, "TBL2", false, 2, col_arr, NULL, -1, NULL, NULL
								   ,&malloced_head, the_debug);
			the_select_node->next->prev = the_select_node;

			struct col_in_select_node** col_arr_2 = (struct col_in_select_node**) myMalloc(sizeof(struct col_in_select_node*) * 2, NULL, &malloced_head, the_debug);

			for (int i=0; i<2; i++)
			{
				col_arr_2[i] = (struct col_in_select_node*) myMalloc(sizeof(struct col_in_select_node), NULL, &malloced_head, the_debug);

				col_arr_2[i]->table_ptr = the_select_node->next;
				col_arr_2[i]->table_ptr_type = PTR_TYPE_SELECT_NODE;

				if (i == 0)
				{
					col_arr_2[i]->col_ptr = ((struct select_node*) the_select_node->next)->columns_arr[1];
					col_arr_2[i]->col_ptr_type = PTR_TYPE_COL_IN_SELECT_NODE;
				}
				else
				{
					col_arr_2[i]->col_ptr = ((struct select_node*) the_select_node->next)->columns_arr[0];
					col_arr_2[i]->col_ptr_type = PTR_TYPE_COL_IN_SELECT_NODE;
				}

				col_arr_2[i]->new_name = NULL;
				col_arr_2[i]->func_node = NULL;
				col_arr_2[i]->math_node = NULL;
				col_arr_2[i]->case_node = NULL;
			}

			initSelectClauseForComp(&the_select_node->next->next, "TBL", false, 2, col_arr_2, NULL, -1, NULL, NULL
								   ,&malloced_head, the_debug);
			the_select_node->next->next->prev = the_select_node->next;

			struct col_in_select_node** col_arr_3 = (struct col_in_select_node**) myMalloc(sizeof(struct col_in_select_node*) * 2, NULL, &malloced_head, the_debug);

			for (int i=0; i<2; i++)
			{
				col_arr_3[i] = (struct col_in_select_node*) myMalloc(sizeof(struct col_in_select_node), NULL, &malloced_head, the_debug);

				col_arr_3[i]->table_ptr = the_select_node->next->next;
				col_arr_3[i]->table_ptr_type = PTR_TYPE_SELECT_NODE;

				if (i == 0)
				{
					col_arr_3[i]->col_ptr = ((struct select_node*) the_select_node->next->next)->columns_arr[0];
					col_arr_3[i]->col_ptr_type = PTR_TYPE_COL_IN_SELECT_NODE;
				}
				else
				{
					col_arr_3[i]->col_ptr = ((struct select_node*) the_select_node->next->next)->columns_arr[1];
					col_arr_3[i]->col_ptr_type = PTR_TYPE_COL_IN_SELECT_NODE;
				}

				col_arr_3[i]->new_name = NULL;
				col_arr_3[i]->func_node = NULL;
				col_arr_3[i]->math_node = NULL;
				col_arr_3[i]->case_node = NULL;
			}

			initSelectClauseForComp(&the_select_node->next->next->next, NULL, false, 2, col_arr_3, NULL, -1, NULL, NULL
								   ,&malloced_head, the_debug);
			the_select_node->next->next->next->prev = the_select_node->next->next;


			if (test_Controller_parseSelect(534, "select * from ( select braND-name,CT-REGISTRATION-NUMBER from ( select CT-REGISTRATION-NUMBER, braND-name from alc_brands ) tbl2 ) tbl;", &the_select_node
										   ,&parsed_error_code, &malloced_head, the_debug) != 0)
				return -1;

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
				return -3;
			}
		// END Test with id = 534

		// START Test with id = 535
			the_select_node = NULL;


			if (test_Controller_parseSelect(535, "select tbl.Brand-name, tbl2.Brand-name, tbl.STATUS, tbl2.EFFECTIVE from ALC_Brands tbl join ( select Brand-name, STATUS from alc_brands ) tbl2 on tbl.Brand-name = tbl2.Brand-name;", &the_select_node
										   ,&parsed_error_code, &malloced_head, the_debug) != 0)
				return -1;

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
				return -3;
			}
		// END Test with id = 535

		// START Test with id = 536
			the_select_node = NULL;


			if (test_Controller_parseSelect(536, "select tbl.Brand-name, tbl2.Brand-name, tbl.STATUS, tbl.EFFECTIVE from ALC_Brands tbl join ( select Brand-name, STATUS, awiodjbaoijd from alc_brands ) tbl2 on tbl.Brand-name = tbl2.Brand-name;", &the_select_node
										   ,&parsed_error_code, &malloced_head, the_debug) != 0)
				return -1;

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
				return -3;
			}
		// END Test with id = 536

		// START Test with id = 537
			initSelectClauseForComp(&the_select_node, "TBL", false, 0, NULL, getTablesHead(), PTR_TYPE_TABLE_INFO, NULL, NULL
								   ,&malloced_head, the_debug);

			initSelectClauseForComp(&joined_select, NULL, false, 0, NULL, getTablesHead(), PTR_TYPE_TABLE_INFO, NULL, NULL
								   ,&malloced_head, the_debug);
			initSelectClauseForComp(&joined_select->next, "TBL2", false, 0, NULL, joined_select, PTR_TYPE_SELECT_NODE, NULL, NULL
								   ,&malloced_head, the_debug);
			joined_select->next->prev = joined_select;

			struct select_node* joined_select_2 = NULL;
			initSelectClauseForComp(&joined_select_2, "TBL3", false, 0, NULL, getTablesHead(), PTR_TYPE_TABLE_INFO, NULL, NULL
								   ,&malloced_head, the_debug);

			col_arr = (struct col_in_select_node**) myMalloc(sizeof(struct col_in_select_node*) * 21, NULL, &malloced_head, the_debug);

			index = 0;
			for (int i=0; i<21; i++)
			{
				col_arr[index] = (struct col_in_select_node*) myMalloc(sizeof(struct col_in_select_node), NULL, &malloced_head, the_debug);

				if (i < 7)
				{
					col_arr[index]->table_ptr = the_select_node;
					col_arr[index]->table_ptr_type = PTR_TYPE_SELECT_NODE;

					col_arr[index]->col_ptr = the_select_node->columns_arr[i];
					col_arr[index]->col_ptr_type = PTR_TYPE_COL_IN_SELECT_NODE;
				}
				else if (i < 14)
				{
					col_arr[index]->table_ptr = joined_select->next;
					col_arr[index]->table_ptr_type = PTR_TYPE_SELECT_NODE;

					col_arr[index]->col_ptr = joined_select->next->columns_arr[i-7];
					col_arr[index]->col_ptr_type = PTR_TYPE_COL_IN_SELECT_NODE;
				}
				else
				{
					col_arr[index]->table_ptr = joined_select_2;
					col_arr[index]->table_ptr_type = PTR_TYPE_SELECT_NODE;

					col_arr[index]->col_ptr = joined_select_2->columns_arr[i-14];
					col_arr[index]->col_ptr_type = PTR_TYPE_COL_IN_SELECT_NODE;
				}

				col_arr[index]->new_name = NULL;
				col_arr[index]->func_node = NULL;
				col_arr[index]->math_node = NULL;
				col_arr[index]->case_node = NULL;

				index++;
			}

			initSelectClauseForComp(&the_select_node->next, NULL, false, 21, col_arr, NULL, -1, NULL, NULL
								   ,&malloced_head, the_debug);
			the_select_node->next->prev = the_select_node;

			the_select_node->next->join_head = (struct join_node*) myMalloc(sizeof(struct join_node), NULL, &malloced_head, the_debug);
			the_select_node->next->join_head->join_type = JOIN_INNER;
			the_select_node->next->join_head->select_joined = joined_select->next;
			the_select_node->next->join_head->prev = NULL;

			the_select_node->next->join_head->on_clause_head = (struct where_clause_node*) myMalloc(sizeof(struct where_clause_node), NULL, &malloced_head, the_debug);
			the_select_node->next->join_head->on_clause_head->ptr_one = the_select_node->columns_arr[0];
			the_select_node->next->join_head->on_clause_head->ptr_one_type = PTR_TYPE_COL_IN_SELECT_NODE;
			the_select_node->next->join_head->on_clause_head->ptr_two = joined_select->next->columns_arr[0];
			the_select_node->next->join_head->on_clause_head->ptr_two_type = PTR_TYPE_COL_IN_SELECT_NODE;
			the_select_node->next->join_head->on_clause_head->where_type = WHERE_IS_EQUALS;
			the_select_node->next->join_head->on_clause_head->parent = NULL;

			the_select_node->next->join_head->next = (struct join_node*) myMalloc(sizeof(struct join_node), NULL, &malloced_head, the_debug);
			the_select_node->next->join_head->next->next = NULL;
			the_select_node->next->join_head->next->prev = the_select_node->next->join_head;
			the_select_node->next->join_head->next->join_type = JOIN_LEFT;
			the_select_node->next->join_head->next->select_joined = joined_select_2;

			the_select_node->next->join_head->next->on_clause_head = (struct where_clause_node*) myMalloc(sizeof(struct where_clause_node), NULL, &malloced_head, the_debug);
			the_select_node->next->join_head->next->on_clause_head->ptr_one = the_select_node->columns_arr[0];
			the_select_node->next->join_head->next->on_clause_head->ptr_one_type = PTR_TYPE_COL_IN_SELECT_NODE;
			the_select_node->next->join_head->next->on_clause_head->ptr_two = joined_select_2->columns_arr[0];
			the_select_node->next->join_head->next->on_clause_head->ptr_two_type = PTR_TYPE_COL_IN_SELECT_NODE;
			the_select_node->next->join_head->next->on_clause_head->where_type = WHERE_IS_EQUALS;
			the_select_node->next->join_head->next->on_clause_head->parent = NULL;


			if (test_Controller_parseSelect(537, "select * from ALC_Brands tbl join ( select * from alc_brands ) tbl2 on tbl.Brand-name = tbl2.Brand-name left join alc_brands tbl3 on tbl.Brand-name = tbl3.Brand-name;", &the_select_node
										   ,&parsed_error_code, &malloced_head, the_debug) != 0)
				return -1;

			
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
				return -3;
			}
		// END Test with id = 537

		// START Test with id = 538
			the_select_node = NULL;


			if (test_Controller_parseSelect(538, "select count(*), braND-name, CT-REGISTRATION-NUMBER, EFFECTIVE from alc_brands group by braND-name, CT-REGISTRATION-NUMBER", &the_select_node
										   ,&parsed_error_code, &malloced_head, the_debug) != 0)
				return -1;

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
				return -3;
			}
		// END Test with id = 538

		// START Test with id = 539
			the_select_node = NULL;


			if (test_Controller_parseSelect(539, "select count(*), 10 * (10 - CT-REGISTRATION-NUMBER), braND-name, EFFECTIVE from alc_brands group by braND-name, EFFECTIVE", &the_select_node
										   ,&parsed_error_code, &malloced_head, the_debug) != 0)
				return -1;

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
				return -3;
			}
		// END Test with id = 539

		// START Test with id = 540
			initSelectClauseForComp(&the_select_node, "TBL", false, 0, NULL, getTablesHead(), PTR_TYPE_TABLE_INFO, NULL, NULL
								   ,&malloced_head, the_debug);

			col_arr = (struct col_in_select_node**) myMalloc(sizeof(struct col_in_select_node*) * 2, NULL, &malloced_head, the_debug);

			for (int i=0; i<2; i++)
			{
				col_arr[i] = (struct col_in_select_node*) myMalloc(sizeof(struct col_in_select_node), NULL, &malloced_head, the_debug);

				if (i == 0)
				{
					col_arr[i]->table_ptr = NULL;
					col_arr[i]->table_ptr_type = -1;

					col_arr[i]->col_ptr = NULL;
					col_arr[i]->col_ptr_type = -1;

					col_arr[i]->new_name = (char*) myMalloc(sizeof(char) * 16, NULL, &malloced_head, the_debug);
					strcpy(col_arr[i]->new_name, "NEW_CASE");

					col_arr[i]->case_node = (struct case_node*) myMalloc(sizeof(struct case_node), NULL, &malloced_head, the_debug);
					col_arr[i]->case_node->case_when_head = NULL;
					col_arr[i]->case_node->case_then_value_head = NULL;

					struct where_clause_node* the_where_node = (struct where_clause_node*) myMalloc(sizeof(struct where_clause_node), NULL, &malloced_head, the_debug);
					the_where_node->ptr_one = the_select_node->columns_arr[0];
					the_where_node->ptr_one_type = PTR_TYPE_COL_IN_SELECT_NODE;
					the_where_node->ptr_two = (char*) myMalloc(sizeof(char*) * 16, NULL, &malloced_head, the_debug);
					strcpy(the_where_node->ptr_two, "Hi");
					the_where_node->ptr_two_type = PTR_TYPE_CHAR;
					the_where_node->where_type = WHERE_IS_EQUALS;
					the_where_node->parent = NULL;

					addListNodePtr(&col_arr[i]->case_node->case_when_head, &col_arr[i]->case_node->case_when_tail, the_where_node, PTR_TYPE_WHERE_CLAUSE_NODE, ADDLISTNODE_TAIL
								  ,NULL, &malloced_head, the_debug);
					addListNodePtr(&col_arr[i]->case_node->case_when_head, &col_arr[i]->case_node->case_when_tail, NULL, -1, ADDLISTNODE_TAIL
								  ,NULL, &malloced_head, the_debug);

					char* str1 = (char*) myMalloc(sizeof(char) * 16, NULL, &malloced_head, the_debug);
					strcpy(str1, "Is Hi");
					char* str2 = (char*) myMalloc(sizeof(char) * 16, NULL, &malloced_head, the_debug);
					strcpy(str2, "Is Not Hi");

					addListNodePtr(&col_arr[i]->case_node->case_then_value_head, &col_arr[i]->case_node->case_then_value_tail, str1, PTR_TYPE_CHAR, ADDLISTNODE_TAIL
								  ,NULL, &malloced_head, the_debug);
					addListNodePtr(&col_arr[i]->case_node->case_then_value_head, &col_arr[i]->case_node->case_then_value_tail, str2, PTR_TYPE_CHAR, ADDLISTNODE_TAIL
								  ,NULL, &malloced_head, the_debug);
				}
				else
				{
					col_arr[i]->table_ptr = the_select_node;
					col_arr[i]->table_ptr_type = PTR_TYPE_SELECT_NODE;

					col_arr[i]->col_ptr = the_select_node->columns_arr[1];
					col_arr[i]->col_ptr_type = PTR_TYPE_COL_IN_SELECT_NODE;

					col_arr[i]->new_name = NULL;

					col_arr[i]->case_node = NULL;
				}

				col_arr[i]->func_node = NULL;
				col_arr[i]->math_node = NULL;
			}

			initSelectClauseForComp(&the_select_node->next, NULL, false, 2, col_arr, NULL, -1, NULL, NULL
								   ,&malloced_head, the_debug);
			the_select_node->next->prev = the_select_node;


			if (test_Controller_parseSelect(540, "select case when tbl    . braND-name = 'Hi' then 'Is Hi' else 'Is Not Hi' end New_case, tbl . CT-REGISTRATION-NUMBER from alc_brands tbl;", &the_select_node
										   ,&parsed_error_code, &malloced_head, the_debug) != 0)
				return -1;

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
				return -3;
			}
		// END Test with id = 540

		// START Test with id = 541
			initSelectClauseForComp(&the_select_node, "TBL", false, 0, NULL, getTablesHead(), PTR_TYPE_TABLE_INFO, NULL, NULL
								   ,&malloced_head, the_debug);

			col_arr = (struct col_in_select_node**) myMalloc(sizeof(struct col_in_select_node*) * 2, NULL, &malloced_head, the_debug);

			for (int i=0; i<2; i++)
			{
				col_arr[i] = (struct col_in_select_node*) myMalloc(sizeof(struct col_in_select_node), NULL, &malloced_head, the_debug);

				if (i == 0)
				{
					col_arr[i]->table_ptr = NULL;
					col_arr[i]->table_ptr_type = -1;

					col_arr[i]->col_ptr = NULL;
					col_arr[i]->col_ptr_type = -1;

					col_arr[i]->new_name = (char*) myMalloc(sizeof(char) * 16, NULL, &malloced_head, the_debug);
					strcpy(col_arr[i]->new_name, "NEW_CASE");

					col_arr[i]->case_node = (struct case_node*) myMalloc(sizeof(struct case_node), NULL, &malloced_head, the_debug);
					col_arr[i]->case_node->case_when_head = NULL;
					col_arr[i]->case_node->case_then_value_head = NULL;

					struct where_clause_node* the_where_node = (struct where_clause_node*) myMalloc(sizeof(struct where_clause_node), NULL, &malloced_head, the_debug);
					the_where_node->ptr_one = the_select_node->columns_arr[0];
					the_where_node->ptr_one_type = PTR_TYPE_COL_IN_SELECT_NODE;
					the_where_node->ptr_two = (char*) myMalloc(sizeof(char*) * 16, NULL, &malloced_head, the_debug);
					strcpy(the_where_node->ptr_two, "Hi");
					the_where_node->ptr_two_type = PTR_TYPE_CHAR;
					the_where_node->where_type = WHERE_IS_EQUALS;
					the_where_node->parent = NULL;

					struct where_clause_node* the_where_node2 = (struct where_clause_node*) myMalloc(sizeof(struct where_clause_node), NULL, &malloced_head, the_debug);
					the_where_node2->ptr_one = the_select_node->columns_arr[0];
					the_where_node2->ptr_one_type = PTR_TYPE_COL_IN_SELECT_NODE;
					the_where_node2->ptr_two = (char*) myMalloc(sizeof(char*) * 16, NULL, &malloced_head, the_debug);
					strcpy(the_where_node2->ptr_two, "Hello");
					the_where_node2->ptr_two_type = PTR_TYPE_CHAR;
					the_where_node2->where_type = WHERE_IS_EQUALS;
					the_where_node2->parent = NULL;

					struct where_clause_node* the_where_node3 = (struct where_clause_node*) myMalloc(sizeof(struct where_clause_node), NULL, &malloced_head, the_debug);
					the_where_node3->ptr_one = the_select_node->columns_arr[4];
					the_where_node3->ptr_one_type = PTR_TYPE_COL_IN_SELECT_NODE;
					the_where_node3->ptr_two = (char*) myMalloc(sizeof(char*) * 16, NULL, &malloced_head, the_debug);
					strcpy(the_where_node3->ptr_two, "1/1/2024");
					the_where_node3->ptr_two_type = PTR_TYPE_DATE;
					the_where_node3->where_type = WHERE_IS_EQUALS;
					the_where_node3->parent = NULL;

					addListNodePtr(&col_arr[i]->case_node->case_when_head, &col_arr[i]->case_node->case_when_tail, the_where_node, PTR_TYPE_WHERE_CLAUSE_NODE, ADDLISTNODE_TAIL
								  ,NULL, &malloced_head, the_debug);
					addListNodePtr(&col_arr[i]->case_node->case_when_head, &col_arr[i]->case_node->case_when_tail, the_where_node2, PTR_TYPE_WHERE_CLAUSE_NODE, ADDLISTNODE_TAIL
								  ,NULL, &malloced_head, the_debug);
					addListNodePtr(&col_arr[i]->case_node->case_when_head, &col_arr[i]->case_node->case_when_tail, the_where_node3, PTR_TYPE_WHERE_CLAUSE_NODE, ADDLISTNODE_TAIL
								  ,NULL, &malloced_head, the_debug);
					addListNodePtr(&col_arr[i]->case_node->case_when_head, &col_arr[i]->case_node->case_when_tail, NULL, -1, ADDLISTNODE_TAIL
								  ,NULL, &malloced_head, the_debug);

					char* str1 = (char*) myMalloc(sizeof(char) * 16, NULL, &malloced_head, the_debug);
					strcpy(str1, "Is Hi");
					char* str2 = (char*) myMalloc(sizeof(char) * 16, NULL, &malloced_head, the_debug);
					strcpy(str2, "Is hello");
					char* str3 = (char*) myMalloc(sizeof(char) * 16, NULL, &malloced_head, the_debug);
					strcpy(str3, "Is date");
					char* str4 = (char*) myMalloc(sizeof(char) * 32, NULL, &malloced_head, the_debug);
					strcpy(str4, "Is Something Else");

					addListNodePtr(&col_arr[i]->case_node->case_then_value_head, &col_arr[i]->case_node->case_then_value_tail, str1, PTR_TYPE_CHAR, ADDLISTNODE_TAIL
								  ,NULL, &malloced_head, the_debug);
					addListNodePtr(&col_arr[i]->case_node->case_then_value_head, &col_arr[i]->case_node->case_then_value_tail, str2, PTR_TYPE_CHAR, ADDLISTNODE_TAIL
								  ,NULL, &malloced_head, the_debug);
					addListNodePtr(&col_arr[i]->case_node->case_then_value_head, &col_arr[i]->case_node->case_then_value_tail, str3, PTR_TYPE_CHAR, ADDLISTNODE_TAIL
								  ,NULL, &malloced_head, the_debug);
					addListNodePtr(&col_arr[i]->case_node->case_then_value_head, &col_arr[i]->case_node->case_then_value_tail, str4, PTR_TYPE_CHAR, ADDLISTNODE_TAIL
								  ,NULL, &malloced_head, the_debug);
				}
				else
				{
					col_arr[i]->table_ptr = the_select_node;
					col_arr[i]->table_ptr_type = PTR_TYPE_SELECT_NODE;

					col_arr[i]->col_ptr = the_select_node->columns_arr[1];
					col_arr[i]->col_ptr_type = PTR_TYPE_COL_IN_SELECT_NODE;

					col_arr[i]->new_name = NULL;

					col_arr[i]->case_node = NULL;
				}

				col_arr[i]->func_node = NULL;
				col_arr[i]->math_node = NULL;
			}

			initSelectClauseForComp(&the_select_node->next, NULL, false, 2, col_arr, NULL, -1, NULL, NULL
								   ,&malloced_head, the_debug);
			the_select_node->next->prev = the_select_node;


			if (test_Controller_parseSelect(541, "select case when tbl. braND-name = 'Hi' then 'Is Hi' when braND-name = 'Hello' then 'Is hello' when EXPIRATION = '1/1/2024' then 'Is date' else 'Is Something Else' end New_case, tbl . CT-REGISTRATION-NUMBER from alc_brands tbl;", &the_select_node
										   ,&parsed_error_code, &malloced_head, the_debug) != 0)
				return -1;

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
				return -3;
			}
		// END Test with id = 541

		// START Test with id = 542
			the_select_node = NULL;

			if (test_Controller_parseSelect(542, "select case when tbl. braND-name = 'Hi' then 'Is Hi' when braND-name = 'Hello' then 1000 when EXPIRATION = '1/1/2024' then 'Is date' else 'Is Something Else' end New_case, tbl . CT-REGISTRATION-NUMBER from alc_brands tbl;", &the_select_node
										   ,&parsed_error_code, &malloced_head, the_debug) != 0)
				return -1;

			if (malloced_head != NULL)
			{
				if (the_debug == YES_DEBUG)
				{
					setOutputRed();
					printf("	ERROR in test_Driver_main() at line %d in %s\n", __LINE__, __FILE__);
					setOutputWhite();
				}
				return -3;
			}
		// END Test with id = 542

		// Math can exist: In where/when/on clauses, in column declaration, in case then statements
		// column declaration functionality already exists 
		// where/when/on clauses functionality can be added in parseWhereClause()
		// case then statements functionality can be added in parseSelect()

		// START Test with id = 543
			initSelectClauseForComp(&the_select_node, "TBL", false, 0, NULL, getTablesHead(), PTR_TYPE_TABLE_INFO, NULL, NULL
								   ,&malloced_head, the_debug);

			col_arr = (struct col_in_select_node**) myMalloc(sizeof(struct col_in_select_node*) * 2, NULL, &malloced_head, the_debug);

			for (int i=0; i<2; i++)
			{
				col_arr[i] = (struct col_in_select_node*) myMalloc(sizeof(struct col_in_select_node), NULL, &malloced_head, the_debug);

				if (i == 0)
				{
					col_arr[i]->table_ptr = NULL;
					col_arr[i]->table_ptr_type = -1;

					col_arr[i]->col_ptr = NULL;
					col_arr[i]->col_ptr_type = -1;

					col_arr[i]->new_name = (char*) myMalloc(sizeof(char) * 16, NULL, &malloced_head, the_debug);
					strcpy(col_arr[i]->new_name, "NEW_CASE");

					col_arr[i]->case_node = (struct case_node*) myMalloc(sizeof(struct case_node), NULL, &malloced_head, the_debug);
					col_arr[i]->case_node->case_when_head = NULL;
					col_arr[i]->case_node->case_then_value_head = NULL;

					struct where_clause_node* the_where_node = (struct where_clause_node*) myMalloc(sizeof(struct where_clause_node), NULL, &malloced_head, the_debug);
					the_where_node->ptr_one = the_select_node->columns_arr[0];
					the_where_node->ptr_one_type = PTR_TYPE_COL_IN_SELECT_NODE;
					the_where_node->ptr_two = (char*) myMalloc(sizeof(char*) * 16, NULL, &malloced_head, the_debug);
					strcpy(the_where_node->ptr_two, "Hi");
					the_where_node->ptr_two_type = PTR_TYPE_CHAR;
					the_where_node->where_type = WHERE_IS_EQUALS;
					the_where_node->parent = NULL;

					addListNodePtr(&col_arr[i]->case_node->case_when_head, &col_arr[i]->case_node->case_when_tail, the_where_node, PTR_TYPE_WHERE_CLAUSE_NODE, ADDLISTNODE_TAIL
								  ,NULL, &malloced_head, the_debug);
					addListNodePtr(&col_arr[i]->case_node->case_when_head, &col_arr[i]->case_node->case_when_tail, NULL, -1, ADDLISTNODE_TAIL
								  ,NULL, &malloced_head, the_debug);

					struct math_node* math1 = (struct math_node*) myMalloc(sizeof(struct math_node), NULL, &malloced_head, the_debug);
					math1->ptr_one = the_select_node->columns_arr[1];
					math1->ptr_one_type = PTR_TYPE_COL_IN_SELECT_NODE;

					math1->ptr_two = myMalloc(sizeof(int), NULL, &malloced_head, the_debug);
					*((int*) math1->ptr_two) = 10;
					math1->ptr_two_type = PTR_TYPE_INT;

					math1->operation = MATH_ADD;
					math1->parent = NULL;

					int* math2 = (int*) myMalloc(sizeof(int), NULL, &malloced_head, the_debug);
					*math2 = 10;

					addListNodePtr(&col_arr[i]->case_node->case_then_value_head, &col_arr[i]->case_node->case_then_value_tail, math1, PTR_TYPE_MATH_NODE, ADDLISTNODE_TAIL
								  ,NULL, &malloced_head, the_debug);
					addListNodePtr(&col_arr[i]->case_node->case_then_value_head, &col_arr[i]->case_node->case_then_value_tail, math2, PTR_TYPE_INT, ADDLISTNODE_TAIL
								  ,NULL, &malloced_head, the_debug);
				}
				else
				{
					col_arr[i]->table_ptr = the_select_node;
					col_arr[i]->table_ptr_type = PTR_TYPE_SELECT_NODE;

					col_arr[i]->col_ptr = the_select_node->columns_arr[1];
					col_arr[i]->col_ptr_type = PTR_TYPE_COL_IN_SELECT_NODE;

					col_arr[i]->new_name = NULL;

					col_arr[i]->case_node = NULL;
				}

				col_arr[i]->func_node = NULL;
				col_arr[i]->math_node = NULL;
			}

			initSelectClauseForComp(&the_select_node->next, NULL, false, 2, col_arr, NULL, -1, NULL, NULL
								   ,&malloced_head, the_debug);
			the_select_node->next->prev = the_select_node;


			if (test_Controller_parseSelect(543, "select case when tbl    . braND-name = 'Hi' then (CT-REGISTRATION-NUMBER + 10) else 10 end New_case, tbl . CT-REGISTRATION-NUMBER from alc_brands tbl;", &the_select_node
										   ,&parsed_error_code, &malloced_head, the_debug) != 0)
				return -1;

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
				return -3;
			}
		// END Test with id = 543

		// START Test with id = 544
			initSelectClauseForComp(&the_select_node, NULL, false, 0, NULL, getTablesHead(), PTR_TYPE_TABLE_INFO, NULL, NULL
								   ,&malloced_head, the_debug);

			col_arr = (struct col_in_select_node**) myMalloc(sizeof(struct col_in_select_node*) * 5, NULL, &malloced_head, the_debug);

			for (int i=0; i<5; i++)
			{
				col_arr[i] = (struct col_in_select_node*) myMalloc(sizeof(struct col_in_select_node), NULL, &malloced_head, the_debug);

				col_arr[i]->table_ptr = NULL;
				col_arr[i]->table_ptr_type = -1;

				col_arr[i]->col_ptr = NULL;
				col_arr[i]->col_ptr_type = -1;

				col_arr[i]->new_name = NULL;

				col_arr[i]->func_node = NULL;
				col_arr[i]->math_node = NULL;

				if (i < 4)
				{
					col_arr[i]->case_node = (struct case_node*) myMalloc(sizeof(struct case_node), NULL, &malloced_head, the_debug);
					col_arr[i]->case_node->case_when_head = NULL;
					col_arr[i]->case_node->case_then_value_head = NULL;

					addListNodePtr(&col_arr[i]->case_node->case_when_head, &col_arr[i]->case_node->case_when_tail, NULL, -1, ADDLISTNODE_TAIL
								  ,NULL, &malloced_head, the_debug);
				}
				else
					col_arr[i]->case_node = NULL;
			}

			col_arr[4]->table_ptr = the_select_node;
			col_arr[4]->table_ptr_type = PTR_TYPE_SELECT_NODE;

			col_arr[4]->col_ptr = the_select_node->columns_arr[0];
			col_arr[4]->col_ptr_type = PTR_TYPE_COL_IN_SELECT_NODE;

			col_arr[0]->new_name = (char*) myMalloc(sizeof(char) * 16, NULL, &malloced_head, the_debug);
			strcpy(col_arr[0]->new_name, "TEN");

			int* ten = (int*) myMalloc(sizeof(int), NULL, &malloced_head, the_debug);
			*ten = 10;
			addListNodePtr(&col_arr[0]->case_node->case_then_value_head, &col_arr[0]->case_node->case_then_value_tail, ten, PTR_TYPE_INT, ADDLISTNODE_TAIL
						  ,NULL, &malloced_head, the_debug);

			col_arr[1]->new_name = (char*) myMalloc(sizeof(char) * 16, NULL, &malloced_head, the_debug);
			strcpy(col_arr[1]->new_name, "SUP_STR");

			char* sup_str = (char*) myMalloc(sizeof(char) * 16, NULL, &malloced_head, the_debug);
			strcpy(sup_str, "Sup");
			addListNodePtr(&col_arr[1]->case_node->case_then_value_head, &col_arr[1]->case_node->case_then_value_tail, sup_str, PTR_TYPE_CHAR, ADDLISTNODE_TAIL
						  ,NULL, &malloced_head, the_debug);

			col_arr[2]->new_name = (char*) myMalloc(sizeof(char) * 32, NULL, &malloced_head, the_debug);
			strcpy(col_arr[2]->new_name, "Another Col");

			double* frac = (double*) myMalloc(sizeof(double), NULL, &malloced_head, the_debug);
			*frac = 1.182973;
			addListNodePtr(&col_arr[2]->case_node->case_then_value_head, &col_arr[2]->case_node->case_then_value_tail, frac, PTR_TYPE_REAL, ADDLISTNODE_TAIL
						  ,NULL, &malloced_head, the_debug);

			col_arr[3]->new_name = (char*) myMalloc(sizeof(char) * 16, NULL, &malloced_head, the_debug);
			strcpy(col_arr[3]->new_name, "ONE_MORE");

			char* one_more = (char*) myMalloc(sizeof(char) * 16, NULL, &malloced_head, the_debug);
			strcpy(one_more, "2/4/2024");
			addListNodePtr(&col_arr[3]->case_node->case_then_value_head, &col_arr[3]->case_node->case_then_value_tail, one_more, PTR_TYPE_DATE, ADDLISTNODE_TAIL
						  ,NULL, &malloced_head, the_debug);


			initSelectClauseForComp(&the_select_node->next, NULL, false, 5, col_arr, NULL, -1, NULL, NULL
								   ,&malloced_head, the_debug);
			the_select_node->next->prev = the_select_node;


			if (test_Controller_parseSelect(544, "select 10 Ten, 'Sup' Sup_Str, 1.182973 \"Another Col\", '2/4/2024' ONE_MORE, Brand-name from alc_brands;", &the_select_node
										   ,&parsed_error_code, &malloced_head, the_debug) != 0)
				return -1;

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
				return -3;
			}
		// END Test with id = 544

		// START Test with id = 545
			the_select_node = NULL;


			if (test_Controller_parseSelect(545, "select * from ( select * from alc_brands );", &the_select_node
										   ,&parsed_error_code, &malloced_head, the_debug) != 0)
				return -1;


			if (malloced_head != NULL)
			{
				if (the_debug == YES_DEBUG)
				{
					setOutputRed();
					printf("	ERROR in test_Driver_main() at line %d in %s\n", __LINE__, __FILE__);
					setOutputWhite();
				}
				return -3;
			}
		// END Test with id = 545

		// START Test with id = 546
			the_select_node = NULL;


			if (test_Controller_parseSelect(546, "select * from ( select * from alc_brands ) tbl join ( select * from alc_brands )  on tbl.Brand-name = braND-name;", &the_select_node
										   ,&parsed_error_code, &malloced_head, the_debug) != 0)
				return -1;
			

			if (malloced_head != NULL)
			{
				if (the_debug == YES_DEBUG)
				{
					setOutputRed();
					printf("	ERROR in test_Driver_main() at line %d in %s\n", __LINE__, __FILE__);
					setOutputWhite();
				}
				return -3;
			}
		// END Test with id = 546

		// START Test with id = 547
			initSelectClauseForComp(&the_select_node, NULL, false, 0, NULL, getTablesHead(), PTR_TYPE_TABLE_INFO, NULL, NULL
								   ,&malloced_head, the_debug);
			initSelectClauseForComp(&the_select_node->next, "TBL", false, 0, NULL, the_select_node, PTR_TYPE_SELECT_NODE, NULL, NULL
								   ,&malloced_head, the_debug);
			the_select_node->next->prev = the_select_node;
			initSelectClauseForComp(&the_select_node->next->next, NULL, false, 0, NULL, the_select_node->next, PTR_TYPE_SELECT_NODE, NULL, NULL
								   ,&malloced_head, the_debug);
			the_select_node->next->next->prev = the_select_node->next;


			if (test_Controller_parseSelect(547, "with tbl as ( select * from alc_brands ) select * from tbl;", &the_select_node
										   ,&parsed_error_code, &malloced_head, the_debug) != 0)
				return -1;

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
				return -3;
			}
		// END Test with id = 547

		// START Test with id = 548
			initSelectClauseForComp(&the_select_node, NULL, false, 0, NULL, getTablesHead(), PTR_TYPE_TABLE_INFO, NULL, NULL
								   ,&malloced_head, the_debug);
			initSelectClauseForComp(&the_select_node->next, "TBL", false, 0, NULL, the_select_node, PTR_TYPE_SELECT_NODE, NULL, NULL
								   ,&malloced_head, the_debug);
			the_select_node->next->prev = the_select_node;
			initSelectClauseForComp(&the_select_node->next->next, "TBL2", false, 0, NULL, the_select_node->next, PTR_TYPE_SELECT_NODE, NULL, NULL
								   ,&malloced_head, the_debug);
			the_select_node->next->next->prev = the_select_node->next;
			initSelectClauseForComp(&the_select_node->next->next->next, NULL, false, 0, NULL, the_select_node->next->next, PTR_TYPE_SELECT_NODE, NULL, NULL
								   ,&malloced_head, the_debug);
			the_select_node->next->next->next->prev = the_select_node->next->next;


			if (test_Controller_parseSelect(548, "with tbl as ( select * from alc_brands ), tbl2 as (select * from tbl) select * from tbl2;", &the_select_node
										   ,&parsed_error_code, &malloced_head, the_debug) != 0)
				return -1;

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
				return -3;
			}
		// END Test with id = 548

		// START Test with id = 549
			the_select_node = NULL;


			if (test_Controller_parseSelect(549, "with tbl as ( with tbl3 as (select * from alc_brands) select * from tbl3 ), tbl2 as (select * from tbl) select * from tbl2;", &the_select_node
										   ,&parsed_error_code, &malloced_head, the_debug) != 0)
				return -1;

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
				return -3;
			}
		// END Test with id = 549

		// START Test with id = 550
			initSelectClauseForComp(&the_select_node, NULL, false, 0, NULL, getTablesHead(), PTR_TYPE_TABLE_INFO, NULL, NULL
								   ,&malloced_head, the_debug);
			initSelectClauseForComp(&the_select_node->next, "TBL", false, 0, NULL, the_select_node, PTR_TYPE_SELECT_NODE, NULL, NULL
								   ,&malloced_head, the_debug);
			the_select_node->next->prev = the_select_node;

			joined_select = NULL;
			initSelectClauseForComp(&joined_select, NULL, false, 0, NULL, getTablesHead(), PTR_TYPE_TABLE_INFO, NULL, NULL
								   ,&malloced_head, the_debug);
			initSelectClauseForComp(&joined_select->next, "TBL2", false, 0, NULL, joined_select, PTR_TYPE_SELECT_NODE, NULL, NULL
								   ,&malloced_head, the_debug);
			joined_select->next->prev = joined_select;

			col_arr = (struct col_in_select_node**) myMalloc(sizeof(struct col_in_select_node*) * 14, NULL, &malloced_head, the_debug);

			index = 0;
			for (int i=0; i<14; i++)
			{
				col_arr[index] = (struct col_in_select_node*) myMalloc(sizeof(struct col_in_select_node), NULL, &malloced_head, the_debug);

				if (i < 7)
				{
					col_arr[index]->table_ptr = the_select_node->next;
					col_arr[index]->table_ptr_type = PTR_TYPE_SELECT_NODE;

					col_arr[index]->col_ptr = the_select_node->next->columns_arr[i];
					col_arr[index]->col_ptr_type = PTR_TYPE_COL_IN_SELECT_NODE;
				}
				else
				{
					col_arr[index]->table_ptr = joined_select->next;
					col_arr[index]->table_ptr_type = PTR_TYPE_SELECT_NODE;

					col_arr[index]->col_ptr = joined_select->next->columns_arr[i-7];
					col_arr[index]->col_ptr_type = PTR_TYPE_COL_IN_SELECT_NODE;
				}

				col_arr[index]->new_name = NULL;
				col_arr[index]->func_node = NULL;
				col_arr[index]->math_node = NULL;
				col_arr[index]->case_node = NULL;

				index++;
			}

			initSelectClauseForComp(&the_select_node->next->next, NULL, false, 14, col_arr, NULL, -1, NULL, NULL
								   ,&malloced_head, the_debug);
			the_select_node->next->next->prev = the_select_node->next;

			the_select_node->next->next->join_head = (struct join_node*) myMalloc(sizeof(struct join_node), NULL, &malloced_head, the_debug);
			the_select_node->next->next->join_head->join_type = JOIN_INNER;
			the_select_node->next->next->join_head->select_joined = joined_select->next;
			the_select_node->next->next->join_head->prev = NULL;
			the_select_node->next->next->join_head->next = NULL;

			the_select_node->next->next->join_head->on_clause_head = (struct where_clause_node*) myMalloc(sizeof(struct where_clause_node), NULL, &malloced_head, the_debug);
			the_select_node->next->next->join_head->on_clause_head->ptr_one = the_select_node->next->columns_arr[0];
			the_select_node->next->next->join_head->on_clause_head->ptr_one_type = PTR_TYPE_COL_IN_SELECT_NODE;
			the_select_node->next->next->join_head->on_clause_head->ptr_two = joined_select->next->columns_arr[0];
			the_select_node->next->next->join_head->on_clause_head->ptr_two_type = PTR_TYPE_COL_IN_SELECT_NODE;
			the_select_node->next->next->join_head->on_clause_head->where_type = WHERE_IS_EQUALS;
			the_select_node->next->next->join_head->on_clause_head->parent = NULL;


			if (test_Controller_parseSelect(550, "with tbl as ( select * from alc_brands ), tbl2 as (select * from alc_brands) select * from tbl join tbl2 on tbl.braND-name = tbl2.braND-name;", &the_select_node
										   ,&parsed_error_code, &malloced_head, the_debug) != 0)
				return -1;

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
				return -3;
			}
		// END Test with id = 550

		// START Test with id = 551
			initSelectClauseForComp(&the_select_node, NULL, false, 0, NULL, getTablesHead(), PTR_TYPE_TABLE_INFO, NULL, NULL
								   ,&malloced_head, the_debug);
			initSelectClauseForComp(&the_select_node->next, "TBL", false, 0, NULL, the_select_node, PTR_TYPE_SELECT_NODE, NULL, NULL
								   ,&malloced_head, the_debug);
			the_select_node->next->prev = the_select_node;

			joined_select = NULL;
			initSelectClauseForComp(&joined_select, NULL, false, 0, NULL, getTablesHead(), PTR_TYPE_TABLE_INFO, NULL, NULL
								   ,&malloced_head, the_debug);
			initSelectClauseForComp(&joined_select->next, "TBL2", false, 0, NULL, joined_select, PTR_TYPE_SELECT_NODE, NULL, NULL
								   ,&malloced_head, the_debug);
			joined_select->next->prev = joined_select;

			col_arr = (struct col_in_select_node**) myMalloc(sizeof(struct col_in_select_node*) * 14, NULL, &malloced_head, the_debug);

			index = 0;
			for (int i=0; i<14; i++)
			{
				col_arr[index] = (struct col_in_select_node*) myMalloc(sizeof(struct col_in_select_node), NULL, &malloced_head, the_debug);

				if (i < 7)
				{
					col_arr[index]->table_ptr = the_select_node->next;
					col_arr[index]->table_ptr_type = PTR_TYPE_SELECT_NODE;

					col_arr[index]->col_ptr = the_select_node->next->columns_arr[i];
					col_arr[index]->col_ptr_type = PTR_TYPE_COL_IN_SELECT_NODE;
				}
				else
				{
					col_arr[index]->table_ptr = joined_select->next;
					col_arr[index]->table_ptr_type = PTR_TYPE_SELECT_NODE;

					col_arr[index]->col_ptr = joined_select->next->columns_arr[i-7];
					col_arr[index]->col_ptr_type = PTR_TYPE_COL_IN_SELECT_NODE;
				}

				col_arr[index]->new_name = NULL;
				col_arr[index]->func_node = NULL;
				col_arr[index]->math_node = NULL;
				col_arr[index]->case_node = NULL;

				index++;
			}

			initSelectClauseForComp(&the_select_node->next->next, NULL, false, 14, col_arr, NULL, -1, NULL, NULL
								   ,&malloced_head, the_debug);
			the_select_node->next->next->prev = the_select_node->next;

			the_select_node->next->next->join_head = (struct join_node*) myMalloc(sizeof(struct join_node), NULL, &malloced_head, the_debug);
			the_select_node->next->next->join_head->join_type = JOIN_INNER;
			the_select_node->next->next->join_head->select_joined = joined_select->next;
			the_select_node->next->next->join_head->prev = NULL;
			the_select_node->next->next->join_head->next = NULL;

			the_select_node->next->next->join_head->on_clause_head = (struct where_clause_node*) myMalloc(sizeof(struct where_clause_node), NULL, &malloced_head, the_debug);
			the_select_node->next->next->join_head->on_clause_head->ptr_one = the_select_node->next->columns_arr[0];
			the_select_node->next->next->join_head->on_clause_head->ptr_one_type = PTR_TYPE_COL_IN_SELECT_NODE;
			the_select_node->next->next->join_head->on_clause_head->ptr_two = joined_select->next->columns_arr[0];
			the_select_node->next->next->join_head->on_clause_head->ptr_two_type = PTR_TYPE_COL_IN_SELECT_NODE;
			the_select_node->next->next->join_head->on_clause_head->where_type = WHERE_IS_EQUALS;
			the_select_node->next->next->join_head->on_clause_head->parent = NULL;


			if (test_Controller_parseSelect(551, "with tbl2 as (select * from alc_brands) select * from ( select * from alc_brands )tbl join tbl2 on tbl.braND-name = tbl2.braND-name;", &the_select_node
										   ,&parsed_error_code, &malloced_head, the_debug) != 0)
				return -1;

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
				return -3;
			}
		// END Test with id = 551

		// START Test with id = 552
			initSelectClauseForComp(&the_select_node, NULL, false, 0, NULL, getTablesHead(), PTR_TYPE_TABLE_INFO, NULL, NULL
								   ,&malloced_head, the_debug);
			initSelectClauseForComp(&the_select_node->next, NULL, false, 0, NULL, the_select_node, PTR_TYPE_SELECT_NODE, NULL, NULL
								   ,&malloced_head, the_debug);
			the_select_node->next->prev = the_select_node;


			if (test_Controller_parseSelect(552, "with tbl_bad as ( select * from alc_brands ) select * from alc_brands;", &the_select_node
										   ,&parsed_error_code, &malloced_head, the_debug) != 0)
				return -1;

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
				return -3;
			}
		// END Test with id = 552

		// START Test with id = 553
			initSelectClauseForComp(&the_select_node, NULL, false, 0, NULL, getTablesHead(), PTR_TYPE_TABLE_INFO, NULL, NULL
								   ,&malloced_head, the_debug);
			initSelectClauseForComp(&the_select_node->next, "TBL", false, 0, NULL, the_select_node, PTR_TYPE_SELECT_NODE, NULL, NULL
								   ,&malloced_head, the_debug);
			the_select_node->next->prev = the_select_node;

			joined_select = NULL;
			initSelectClauseForComp(&joined_select, NULL, false, 0, NULL, getTablesHead(), PTR_TYPE_TABLE_INFO, NULL, NULL
								   ,&malloced_head, the_debug);
			initSelectClauseForComp(&joined_select->next, "TBL2", false, 0, NULL, joined_select, PTR_TYPE_SELECT_NODE, NULL, NULL
								   ,&malloced_head, the_debug);
			joined_select->next->prev = joined_select;

			col_arr = (struct col_in_select_node**) myMalloc(sizeof(struct col_in_select_node*) * 14, NULL, &malloced_head, the_debug);

			index = 0;
			for (int i=0; i<14; i++)
			{
				col_arr[index] = (struct col_in_select_node*) myMalloc(sizeof(struct col_in_select_node), NULL, &malloced_head, the_debug);

				if (i < 7)
				{
					col_arr[index]->table_ptr = the_select_node->next;
					col_arr[index]->table_ptr_type = PTR_TYPE_SELECT_NODE;

					col_arr[index]->col_ptr = the_select_node->next->columns_arr[i];
					col_arr[index]->col_ptr_type = PTR_TYPE_COL_IN_SELECT_NODE;
				}
				else
				{
					col_arr[index]->table_ptr = joined_select->next;
					col_arr[index]->table_ptr_type = PTR_TYPE_SELECT_NODE;

					col_arr[index]->col_ptr = joined_select->next->columns_arr[i-7];
					col_arr[index]->col_ptr_type = PTR_TYPE_COL_IN_SELECT_NODE;
				}

				col_arr[index]->new_name = NULL;
				col_arr[index]->func_node = NULL;
				col_arr[index]->math_node = NULL;
				col_arr[index]->case_node = NULL;

				index++;
			}

			initSelectClauseForComp(&the_select_node->next->next, NULL, false, 14, col_arr, NULL, -1, NULL, NULL
								   ,&malloced_head, the_debug);
			the_select_node->next->next->prev = the_select_node->next;

			the_select_node->next->next->join_head = (struct join_node*) myMalloc(sizeof(struct join_node), NULL, &malloced_head, the_debug);
			the_select_node->next->next->join_head->join_type = JOIN_INNER;
			the_select_node->next->next->join_head->select_joined = joined_select->next;
			the_select_node->next->next->join_head->prev = NULL;
			the_select_node->next->next->join_head->next = NULL;

			the_select_node->next->next->join_head->on_clause_head = (struct where_clause_node*) myMalloc(sizeof(struct where_clause_node), NULL, &malloced_head, the_debug);
			the_select_node->next->next->join_head->on_clause_head->ptr_one = the_select_node->next->columns_arr[0];
			the_select_node->next->next->join_head->on_clause_head->ptr_one_type = PTR_TYPE_COL_IN_SELECT_NODE;
			the_select_node->next->next->join_head->on_clause_head->ptr_two = joined_select->next->columns_arr[0];
			the_select_node->next->next->join_head->on_clause_head->ptr_two_type = PTR_TYPE_COL_IN_SELECT_NODE;
			the_select_node->next->next->join_head->on_clause_head->where_type = WHERE_IS_EQUALS;
			the_select_node->next->next->join_head->on_clause_head->parent = NULL;


			if (test_Controller_parseSelect(553, "with tbl2 as (select * from alc_brands),tbl_bad as (select * from alc_brands) select * from ( select * from alc_brands )tbl join tbl2 on tbl.braND-name = tbl2.braND-name;", &the_select_node
										   ,&parsed_error_code, &malloced_head, the_debug) != 0)
				return -1;

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
				return -3;
			}
		// END Test with id = 553

		// START Test with id = 554
			initSelectClauseForComp(&the_select_node, NULL, false, 0, NULL, getTablesHead(), PTR_TYPE_TABLE_INFO, NULL, NULL
								   ,&malloced_head, the_debug);
			initSelectClauseForComp(&the_select_node->next, NULL, false, 0, NULL, the_select_node, PTR_TYPE_SELECT_NODE, NULL, NULL
								   ,&malloced_head, the_debug);
			the_select_node->next->prev = the_select_node;

			struct order_by_node* order_by = (struct order_by_node*) myMalloc(sizeof(struct order_by_node), NULL, &malloced_head, the_debug);
			order_by->order_by_cols_head = NULL;
			order_by->order_by_cols_tail = NULL;
			order_by->order_by_cols_which_head = NULL;
			order_by->order_by_cols_which_tail = NULL;

			addListNodePtr(&order_by->order_by_cols_head, &order_by->order_by_cols_tail, the_select_node->next->columns_arr[0], PTR_TYPE_COL_IN_SELECT_NODE, ADDLISTNODE_TAIL
						  ,NULL, &malloced_head, the_debug);

			int* which = (int*) myMalloc(sizeof(int), NULL, &malloced_head, the_debug);
			*which = ORDER_BY_ASC;

			addListNodePtr(&order_by->order_by_cols_which_head, &order_by->order_by_cols_which_tail, which, PTR_TYPE_INT, ADDLISTNODE_TAIL
						  ,NULL, &malloced_head, the_debug);

			the_select_node->next->order_by = order_by;


			if (test_Controller_parseSelect(554, "select * from alc_brands order by braND-name asc;", &the_select_node
										   ,&parsed_error_code, &malloced_head, the_debug) != 0)
				return -1;

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
				return -3;
			}
		// END Test with id = 554

		// START Test with id = 555
			initSelectClauseForComp(&the_select_node, NULL, false, 0, NULL, getTablesHead(), PTR_TYPE_TABLE_INFO, NULL, NULL
								   ,&malloced_head, the_debug);

			col_arr = (struct col_in_select_node**) myMalloc(sizeof(struct col_in_select_node*) * 1, NULL, &malloced_head, the_debug);
			
			col_arr[0] = (struct col_in_select_node*) myMalloc(sizeof(struct col_in_select_node), NULL, &malloced_head, the_debug);
			col_arr[0]->table_ptr = the_select_node;
			col_arr[0]->table_ptr_type = PTR_TYPE_SELECT_NODE;
			col_arr[0]->col_ptr = ((struct select_node*) the_select_node)->columns_arr[0];
			col_arr[0]->col_ptr_type = PTR_TYPE_COL_IN_SELECT_NODE;
			col_arr[0]->new_name = upper("COL1", NULL, &malloced_head, the_debug);
			col_arr[0]->func_node = NULL;
			col_arr[0]->math_node = NULL;
			col_arr[0]->case_node = NULL;

			initSelectClauseForComp(&the_select_node->next, NULL, false, 1, col_arr, NULL, -1, NULL, NULL
								   ,&malloced_head, the_debug);
			the_select_node->next->prev = the_select_node;

			order_by = (struct order_by_node*) myMalloc(sizeof(struct order_by_node), NULL, &malloced_head, the_debug);
			order_by->order_by_cols_head = NULL;
			order_by->order_by_cols_tail = NULL;
			order_by->order_by_cols_which_head = NULL;
			order_by->order_by_cols_which_tail = NULL;

			addListNodePtr(&order_by->order_by_cols_head, &order_by->order_by_cols_tail, the_select_node->next->columns_arr[0], PTR_TYPE_COL_IN_SELECT_NODE, ADDLISTNODE_TAIL
						  ,NULL, &malloced_head, the_debug);

			which = (int*) myMalloc(sizeof(int), NULL, &malloced_head, the_debug);
			*which = ORDER_BY_ASC;

			addListNodePtr(&order_by->order_by_cols_which_head, &order_by->order_by_cols_which_tail, which, PTR_TYPE_INT, ADDLISTNODE_TAIL
						  ,NULL, &malloced_head, the_debug);

			the_select_node->next->order_by = order_by;


			if (test_Controller_parseSelect(555, "select braND-name as col1 from alc_brands order by col1 asc;", &the_select_node
										   ,&parsed_error_code, &malloced_head, the_debug) != 0)
				return -1;

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
				return -3;
			}		
		// END Test with id = 555

		// START Test with id = 556
			initSelectClauseForComp(&the_select_node, NULL, false, 0, NULL, getTablesHead(), PTR_TYPE_TABLE_INFO, NULL, NULL
								   ,&malloced_head, the_debug);
			initSelectClauseForComp(&the_select_node->next, NULL, false, 0, NULL, the_select_node, PTR_TYPE_SELECT_NODE, NULL, NULL
								   ,&malloced_head, the_debug);
			the_select_node->next->prev = the_select_node;

			order_by = (struct order_by_node*) myMalloc(sizeof(struct order_by_node), NULL, &malloced_head, the_debug);
			order_by->order_by_cols_head = NULL;
			order_by->order_by_cols_tail = NULL;
			order_by->order_by_cols_which_head = NULL;
			order_by->order_by_cols_which_tail = NULL;

			addListNodePtr(&order_by->order_by_cols_head, &order_by->order_by_cols_tail, the_select_node->next->columns_arr[0], PTR_TYPE_COL_IN_SELECT_NODE, ADDLISTNODE_TAIL
						  ,NULL, &malloced_head, the_debug);
			which = (int*) myMalloc(sizeof(int), NULL, &malloced_head, the_debug);
			*which = ORDER_BY_ASC;
			addListNodePtr(&order_by->order_by_cols_which_head, &order_by->order_by_cols_which_tail, which, PTR_TYPE_INT, ADDLISTNODE_TAIL
						  ,NULL, &malloced_head, the_debug);

			addListNodePtr(&order_by->order_by_cols_head, &order_by->order_by_cols_tail, the_select_node->next->columns_arr[2], PTR_TYPE_COL_IN_SELECT_NODE, ADDLISTNODE_TAIL
						  ,NULL, &malloced_head, the_debug);
			int* which1 = (int*) myMalloc(sizeof(int), NULL, &malloced_head, the_debug);
			*which1 = ORDER_BY_DESC;
			addListNodePtr(&order_by->order_by_cols_which_head, &order_by->order_by_cols_which_tail, which1, PTR_TYPE_INT, ADDLISTNODE_TAIL
						  ,NULL, &malloced_head, the_debug);

			addListNodePtr(&order_by->order_by_cols_head, &order_by->order_by_cols_tail, the_select_node->next->columns_arr[6], PTR_TYPE_COL_IN_SELECT_NODE, ADDLISTNODE_TAIL
						  ,NULL, &malloced_head, the_debug);
			int* which2 = (int*) myMalloc(sizeof(int), NULL, &malloced_head, the_debug);
			*which2 = ORDER_BY_ASC;
			addListNodePtr(&order_by->order_by_cols_which_head, &order_by->order_by_cols_which_tail, which2, PTR_TYPE_INT, ADDLISTNODE_TAIL
						  ,NULL, &malloced_head, the_debug);

			the_select_node->next->order_by = order_by;


			if (test_Controller_parseSelect(556, "select * from alc_brands order by braND-name asc, STATUS desc, SUPERVISOR-CREDENTIAL;", &the_select_node
										   ,&parsed_error_code, &malloced_head, the_debug) != 0)
				return -1;

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
				return -3;
			}
		// END Test with id = 556

		// START Test with id = 557
			initSelectClauseForComp(&the_select_node, NULL, false, 0, NULL, getTablesHead(), PTR_TYPE_TABLE_INFO, NULL, NULL
								   ,&malloced_head, the_debug);
			initSelectClauseForComp(&the_select_node->next, NULL, false, 0, NULL, the_select_node, PTR_TYPE_SELECT_NODE, NULL, NULL
								   ,&malloced_head, the_debug);
			the_select_node->next->prev = the_select_node;

			order_by = (struct order_by_node*) myMalloc(sizeof(struct order_by_node), NULL, &malloced_head, the_debug);
			order_by->order_by_cols_head = NULL;
			order_by->order_by_cols_tail = NULL;
			order_by->order_by_cols_which_head = NULL;
			order_by->order_by_cols_which_tail = NULL;

			addListNodePtr(&order_by->order_by_cols_head, &order_by->order_by_cols_tail, the_select_node->next->columns_arr[0], PTR_TYPE_COL_IN_SELECT_NODE, ADDLISTNODE_TAIL
						  ,NULL, &malloced_head, the_debug);
			which = (int*) myMalloc(sizeof(int), NULL, &malloced_head, the_debug);
			*which = ORDER_BY_ASC;
			addListNodePtr(&order_by->order_by_cols_which_head, &order_by->order_by_cols_which_tail, which, PTR_TYPE_INT, ADDLISTNODE_TAIL
						  ,NULL, &malloced_head, the_debug);

			addListNodePtr(&order_by->order_by_cols_head, &order_by->order_by_cols_tail, the_select_node->next->columns_arr[2], PTR_TYPE_COL_IN_SELECT_NODE, ADDLISTNODE_TAIL
						  ,NULL, &malloced_head, the_debug);
			which1 = (int*) myMalloc(sizeof(int), NULL, &malloced_head, the_debug);
			*which1 = ORDER_BY_DESC;
			addListNodePtr(&order_by->order_by_cols_which_head, &order_by->order_by_cols_which_tail, which1, PTR_TYPE_INT, ADDLISTNODE_TAIL
						  ,NULL, &malloced_head, the_debug);

			addListNodePtr(&order_by->order_by_cols_head, &order_by->order_by_cols_tail, the_select_node->next->columns_arr[6], PTR_TYPE_COL_IN_SELECT_NODE, ADDLISTNODE_TAIL
						  ,NULL, &malloced_head, the_debug);
			which2 = (int*) myMalloc(sizeof(int), NULL, &malloced_head, the_debug);
			*which2 = ORDER_BY_ASC;
			addListNodePtr(&order_by->order_by_cols_which_head, &order_by->order_by_cols_which_tail, which2, PTR_TYPE_INT, ADDLISTNODE_TAIL
						  ,NULL, &malloced_head, the_debug);

			the_select_node->next->order_by = order_by;


			if (test_Controller_parseSelect(557, "select * from alc_brands order by braND-name, STATUS desc, SUPERVISOR-CREDENTIAL asc;", &the_select_node
										   ,&parsed_error_code, &malloced_head, the_debug) != 0)
				return -1;

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
				return -3;
			}
		// END Test with id = 557

		// START Test with id = 558
			the_select_node = NULL;


			if (test_Controller_parseSelect(558, "select * from alc_brands order by braND-name, desc, SUPERVISOR-CREDENTIAL asc", &the_select_node
										   ,&parsed_error_code, &malloced_head, the_debug) != 0)
				return -1;


			if (malloced_head != NULL)
			{
				if (the_debug == YES_DEBUG)
				{
					setOutputRed();
					printf("	ERROR in test_Driver_main() at line %d in %s\n", __LINE__, __FILE__);
					setOutputWhite();
				}
				return -3;
			}
		// END Test with id = 558

		// START Test with id = 559
			the_select_node = NULL;


			if (test_Controller_parseSelect(559, "select * from alc_brands order by braND-name, desc SUPERVISOR-CREDENTIAL asc", &the_select_node
										   ,&parsed_error_code, &malloced_head, the_debug) != 0)
				return -1;


			if (malloced_head != NULL)
			{
				if (the_debug == YES_DEBUG)
				{
					setOutputRed();
					printf("	ERROR in test_Driver_main() at line %d in %s\n", __LINE__, __FILE__);
					setOutputWhite();
				}
				return -3;
			}
		// END Test with id = 559

		// START Test with id = 560
			the_select_node = NULL;


			if (test_Controller_parseSelect(560, "select * from alc_brands order by COL1, SUPERVISOR-CREDENTIAL asc", &the_select_node
										   ,&parsed_error_code, &malloced_head, the_debug) != 0)
				return -1;
			

			if (malloced_head != NULL)
			{
				if (the_debug == YES_DEBUG)
				{
					setOutputRed();
					printf("	ERROR in test_Driver_main() at line %d in %s\n", __LINE__, __FILE__);
					setOutputWhite();
				}
				return -3;
			}
		// END Test with id = 560

		// START Test with id = 561
			initSelectClauseForComp(&the_select_node, NULL, false, 0, NULL, getTablesHead(), PTR_TYPE_TABLE_INFO, NULL, NULL
								   ,&malloced_head, the_debug);

			col_arr = (struct col_in_select_node**) myMalloc(sizeof(struct col_in_select_node*) * 2, NULL, &malloced_head, the_debug);
			
			col_arr[0] = (struct col_in_select_node*) myMalloc(sizeof(struct col_in_select_node), NULL, &malloced_head, the_debug);
			col_arr[0]->table_ptr = NULL;
			col_arr[0]->table_ptr_type = -1;
			col_arr[0]->col_ptr = NULL;
			col_arr[0]->col_ptr_type = -1;
			col_arr[0]->new_name = upper("COUNT ( * ) ", NULL, &malloced_head, the_debug);
			col_arr[0]->math_node = NULL;
			col_arr[0]->case_node = NULL;

			col_arr[0]->func_node = (struct func_node*) myMalloc(sizeof(struct func_node), NULL, &malloced_head, the_debug);
			col_arr[0]->func_node->which_func = FUNC_COUNT;
			col_arr[0]->func_node->distinct = false;
			col_arr[0]->func_node->args_size = 1;
			col_arr[0]->func_node->args_arr = (void**) myMalloc(sizeof(void*), NULL, &malloced_head, the_debug);
			col_arr[0]->func_node->args_arr[0] = upper("*", NULL, &malloced_head, the_debug);
			col_arr[0]->func_node->args_arr_type = (int*) myMalloc(sizeof(int), NULL, &malloced_head, the_debug);
			col_arr[0]->func_node->args_arr_type[0] = PTR_TYPE_CHAR;
			col_arr[0]->func_node->group_by_cols_head = NULL;
			col_arr[0]->func_node->group_by_cols_tail = NULL;
			addListNodePtr(&col_arr[0]->func_node->group_by_cols_head, &col_arr[0]->func_node->group_by_cols_tail
						  ,((struct select_node*) the_select_node)->columns_arr[0], PTR_TYPE_COL_IN_SELECT_NODE, ADDLISTNODE_TAIL
				  		  ,NULL, &malloced_head, the_debug);

			col_arr[1] = (struct col_in_select_node*) myMalloc(sizeof(struct col_in_select_node), NULL, &malloced_head, the_debug);
			col_arr[1]->table_ptr = the_select_node;
			col_arr[1]->table_ptr_type = PTR_TYPE_SELECT_NODE;
			col_arr[1]->col_ptr = ((struct select_node*) the_select_node)->columns_arr[0];
			col_arr[1]->col_ptr_type = PTR_TYPE_COL_IN_SELECT_NODE;
			col_arr[1]->new_name = NULL;
			col_arr[1]->math_node = NULL;
			col_arr[1]->func_node = NULL;
			col_arr[1]->case_node = NULL;

			initSelectClauseForComp(&the_select_node->next, NULL, false, 2, col_arr, NULL, -1, NULL, NULL
								   ,&malloced_head, the_debug);
			the_select_node->next->prev = the_select_node;

			the_select_node->next->having_head = (struct where_clause_node*) myMalloc(sizeof(struct where_clause_node), NULL, &malloced_head, the_debug);

			the_select_node->next->having_head->ptr_one = (struct func_node*) myMalloc(sizeof(struct func_node), NULL, &malloced_head, the_debug);
			((struct func_node*) the_select_node->next->having_head->ptr_one)->which_func = FUNC_COUNT;
			((struct func_node*) the_select_node->next->having_head->ptr_one)->distinct = false;
			((struct func_node*) the_select_node->next->having_head->ptr_one)->args_size = 1;
			((struct func_node*) the_select_node->next->having_head->ptr_one)->args_arr = (void**) myMalloc(sizeof(void*), NULL, &malloced_head, the_debug);
			((struct func_node*) the_select_node->next->having_head->ptr_one)->args_arr[0] = upper("*", NULL, &malloced_head, the_debug);
			((struct func_node*) the_select_node->next->having_head->ptr_one)->args_arr_type = (int*) myMalloc(sizeof(int), NULL, &malloced_head, the_debug);
			((struct func_node*) the_select_node->next->having_head->ptr_one)->args_arr_type[0] = PTR_TYPE_CHAR;
			((struct func_node*) the_select_node->next->having_head->ptr_one)->group_by_cols_head = NULL;
			((struct func_node*) the_select_node->next->having_head->ptr_one)->group_by_cols_tail = NULL;
			the_select_node->next->having_head->ptr_one_type = PTR_TYPE_FUNC_NODE;

			the_select_node->next->having_head->ptr_two = (int*) myMalloc(sizeof(int), NULL, &malloced_head, the_debug);
			*((int*) the_select_node->next->having_head->ptr_two) = 1;
			the_select_node->next->having_head->ptr_two_type = PTR_TYPE_INT;

			the_select_node->next->having_head->where_type = WHERE_GREATER_THAN;
			the_select_node->next->having_head->parent = NULL;


			if (test_Controller_parseSelect(561, "select count( *), braND-name from alc_brands group by braND-name having count(*) > 1", &the_select_node
										   ,&parsed_error_code, &malloced_head, the_debug) != 0)
				return -1;
			
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
				return -3;
			}
		// END Test with id = 561

		// START Test with id = 562
			initSelectClauseForComp(&the_select_node, NULL, false, 0, NULL, getTablesHead(), PTR_TYPE_TABLE_INFO, NULL, NULL
								   ,&malloced_head, the_debug);

			col_arr = (struct col_in_select_node**) myMalloc(sizeof(struct col_in_select_node*) * 2, NULL, &malloced_head, the_debug);
			
			col_arr[0] = (struct col_in_select_node*) myMalloc(sizeof(struct col_in_select_node), NULL, &malloced_head, the_debug);
			col_arr[0]->table_ptr = NULL;
			col_arr[0]->table_ptr_type = -1;
			col_arr[0]->col_ptr = NULL;
			col_arr[0]->col_ptr_type = -1;
			col_arr[0]->new_name = upper("AVG ( CT-REGISTRATION-NUMBER ) ", NULL, &malloced_head, the_debug);
			col_arr[0]->math_node = NULL;
			col_arr[0]->case_node = NULL;

			col_arr[0]->func_node = (struct func_node*) myMalloc(sizeof(struct func_node), NULL, &malloced_head, the_debug);
			col_arr[0]->func_node->which_func = FUNC_AVG;
			col_arr[0]->func_node->distinct = false;
			col_arr[0]->func_node->args_size = 1;
			col_arr[0]->func_node->args_arr = (void**) myMalloc(sizeof(void*), NULL, &malloced_head, the_debug);
			col_arr[0]->func_node->args_arr[0] = ((struct select_node*) the_select_node)->columns_arr[1];
			col_arr[0]->func_node->args_arr_type = (int*) myMalloc(sizeof(int), NULL, &malloced_head, the_debug);
			col_arr[0]->func_node->args_arr_type[0] = PTR_TYPE_COL_IN_SELECT_NODE;
			col_arr[0]->func_node->group_by_cols_head = NULL;
			col_arr[0]->func_node->group_by_cols_tail = NULL;
			addListNodePtr(&col_arr[0]->func_node->group_by_cols_head, &col_arr[0]->func_node->group_by_cols_tail
						  ,((struct select_node*) the_select_node)->columns_arr[0], PTR_TYPE_COL_IN_SELECT_NODE, ADDLISTNODE_TAIL
				  		  ,NULL, &malloced_head, the_debug);

			col_arr[1] = (struct col_in_select_node*) myMalloc(sizeof(struct col_in_select_node), NULL, &malloced_head, the_debug);
			col_arr[1]->table_ptr = the_select_node;
			col_arr[1]->table_ptr_type = PTR_TYPE_SELECT_NODE;
			col_arr[1]->col_ptr = ((struct select_node*) the_select_node)->columns_arr[0];
			col_arr[1]->col_ptr_type = PTR_TYPE_COL_IN_SELECT_NODE;
			col_arr[1]->new_name = NULL;
			col_arr[1]->math_node = NULL;
			col_arr[1]->func_node = NULL;
			col_arr[1]->case_node = NULL;

			initSelectClauseForComp(&the_select_node->next, NULL, false, 2, col_arr, NULL, -1, NULL, NULL
								   ,&malloced_head, the_debug);
			the_select_node->next->prev = the_select_node;

			the_select_node->next->having_head = (struct where_clause_node*) myMalloc(sizeof(struct where_clause_node), NULL, &malloced_head, the_debug);

			the_select_node->next->having_head->ptr_one = (struct func_node*) myMalloc(sizeof(struct func_node), NULL, &malloced_head, the_debug);
			((struct func_node*) the_select_node->next->having_head->ptr_one)->which_func = FUNC_AVG;
			((struct func_node*) the_select_node->next->having_head->ptr_one)->distinct = false;
			((struct func_node*) the_select_node->next->having_head->ptr_one)->args_size = 1;
			((struct func_node*) the_select_node->next->having_head->ptr_one)->args_arr = (void**) myMalloc(sizeof(void*), NULL, &malloced_head, the_debug);
			((struct func_node*) the_select_node->next->having_head->ptr_one)->args_arr[0] = ((struct select_node*) the_select_node)->columns_arr[1];
			((struct func_node*) the_select_node->next->having_head->ptr_one)->args_arr_type = (int*) myMalloc(sizeof(int), NULL, &malloced_head, the_debug);
			((struct func_node*) the_select_node->next->having_head->ptr_one)->args_arr_type[0] = PTR_TYPE_COL_IN_SELECT_NODE;
			((struct func_node*) the_select_node->next->having_head->ptr_one)->group_by_cols_head = NULL;
			((struct func_node*) the_select_node->next->having_head->ptr_one)->group_by_cols_tail = NULL;
			the_select_node->next->having_head->ptr_one_type = PTR_TYPE_FUNC_NODE;

			the_select_node->next->having_head->ptr_two = (int*) myMalloc(sizeof(int), NULL, &malloced_head, the_debug);
			*((int*) the_select_node->next->having_head->ptr_two) = 10000;
			the_select_node->next->having_head->ptr_two_type = PTR_TYPE_INT;

			the_select_node->next->having_head->where_type = WHERE_IS_EQUALS;
			the_select_node->next->having_head->parent = NULL;


			if (test_Controller_parseSelect(562, "select avg( CT-REGISTRATION-NUMBER),  braND-name from alc_brands group by braND-name having avg( CT-REGISTRATION-NUMBER) = 10000", &the_select_node
										   ,&parsed_error_code, &malloced_head, the_debug) != 0)
				return -1;

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
				return -3;
			}
		// END Test with id = 562

		// START Test with id = 563
			initSelectClauseForComp(&the_select_node, NULL, false, 0, NULL, getTablesHead(), PTR_TYPE_TABLE_INFO, NULL, NULL
								   ,&malloced_head, the_debug);

			col_arr = (struct col_in_select_node**) myMalloc(sizeof(struct col_in_select_node*) * 3, NULL, &malloced_head, the_debug);
			
			col_arr[0] = (struct col_in_select_node*) myMalloc(sizeof(struct col_in_select_node), NULL, &malloced_head, the_debug);
			col_arr[0]->table_ptr = NULL;
			col_arr[0]->table_ptr_type = -1;
			col_arr[0]->col_ptr = NULL;
			col_arr[0]->col_ptr_type = -1;
			col_arr[0]->new_name = upper("AVG ( CT-REGISTRATION-NUMBER ) ", NULL, &malloced_head, the_debug);
			col_arr[0]->math_node = NULL;
			col_arr[0]->case_node = NULL;

			col_arr[0]->func_node = (struct func_node*) myMalloc(sizeof(struct func_node), NULL, &malloced_head, the_debug);
			col_arr[0]->func_node->which_func = FUNC_AVG;
			col_arr[0]->func_node->distinct = false;
			col_arr[0]->func_node->args_size = 1;
			col_arr[0]->func_node->args_arr = (void**) myMalloc(sizeof(void*), NULL, &malloced_head, the_debug);
			col_arr[0]->func_node->args_arr[0] = ((struct select_node*) the_select_node)->columns_arr[1];
			col_arr[0]->func_node->args_arr_type = (int*) myMalloc(sizeof(int), NULL, &malloced_head, the_debug);
			col_arr[0]->func_node->args_arr_type[0] = PTR_TYPE_COL_IN_SELECT_NODE;
			col_arr[0]->func_node->group_by_cols_head = NULL;
			col_arr[0]->func_node->group_by_cols_tail = NULL;
			addListNodePtr(&col_arr[0]->func_node->group_by_cols_head, &col_arr[0]->func_node->group_by_cols_tail
						  ,((struct select_node*) the_select_node)->columns_arr[0], PTR_TYPE_COL_IN_SELECT_NODE, ADDLISTNODE_TAIL
				  		  ,NULL, &malloced_head, the_debug);

			col_arr[1] = (struct col_in_select_node*) myMalloc(sizeof(struct col_in_select_node), NULL, &malloced_head, the_debug);
			col_arr[1]->table_ptr = NULL;
			col_arr[1]->table_ptr_type = -1;
			col_arr[1]->col_ptr = NULL;
			col_arr[1]->col_ptr_type = -1;
			col_arr[1]->new_name = upper("COUNT ( * ) ", NULL, &malloced_head, the_debug);
			col_arr[1]->math_node = NULL;
			col_arr[1]->case_node = NULL;

			col_arr[1]->func_node = (struct func_node*) myMalloc(sizeof(struct func_node), NULL, &malloced_head, the_debug);
			col_arr[1]->func_node->which_func = FUNC_COUNT;
			col_arr[1]->func_node->distinct = false;
			col_arr[1]->func_node->args_size = 1;
			col_arr[1]->func_node->args_arr = (void**) myMalloc(sizeof(void*), NULL, &malloced_head, the_debug);
			col_arr[1]->func_node->args_arr[0] = upper("*", NULL, &malloced_head, the_debug);
			col_arr[1]->func_node->args_arr_type = (int*) myMalloc(sizeof(int), NULL, &malloced_head, the_debug);
			col_arr[1]->func_node->args_arr_type[0] = PTR_TYPE_CHAR;
			col_arr[1]->func_node->group_by_cols_head = NULL;
			col_arr[1]->func_node->group_by_cols_tail = NULL;
			addListNodePtr(&col_arr[1]->func_node->group_by_cols_head, &col_arr[1]->func_node->group_by_cols_tail
						  ,((struct select_node*) the_select_node)->columns_arr[0], PTR_TYPE_COL_IN_SELECT_NODE, ADDLISTNODE_TAIL
				  		  ,NULL, &malloced_head, the_debug);

			col_arr[2] = (struct col_in_select_node*) myMalloc(sizeof(struct col_in_select_node), NULL, &malloced_head, the_debug);
			col_arr[2]->table_ptr = the_select_node;
			col_arr[2]->table_ptr_type = PTR_TYPE_SELECT_NODE;
			col_arr[2]->col_ptr = ((struct select_node*) the_select_node)->columns_arr[0];
			col_arr[2]->col_ptr_type = PTR_TYPE_COL_IN_SELECT_NODE;
			col_arr[2]->new_name = NULL;
			col_arr[2]->math_node = NULL;
			col_arr[2]->func_node = NULL;
			col_arr[2]->case_node = NULL;

			initSelectClauseForComp(&the_select_node->next, NULL, false, 3, col_arr, NULL, -1, NULL, NULL
								   ,&malloced_head, the_debug);
			the_select_node->next->prev = the_select_node;

			the_select_node->next->having_head = (struct where_clause_node*) myMalloc(sizeof(struct where_clause_node), NULL, &malloced_head, the_debug);

			the_select_node->next->having_head->ptr_one = (struct where_clause_node*) myMalloc(sizeof(struct where_clause_node), NULL, &malloced_head, the_debug);
			the_select_node->next->having_head->ptr_one_type = PTR_TYPE_WHERE_CLAUSE_NODE;
			the_select_node->next->having_head->ptr_two = (struct where_clause_node*) myMalloc(sizeof(struct where_clause_node), NULL, &malloced_head, the_debug);
			the_select_node->next->having_head->ptr_two_type = PTR_TYPE_WHERE_CLAUSE_NODE;
			the_select_node->next->having_head->where_type = WHERE_AND;
			the_select_node->next->having_head->parent = NULL;

				((struct where_clause_node*) the_select_node->next->having_head->ptr_one)->ptr_one = (struct func_node*) myMalloc(sizeof(struct func_node), NULL, &malloced_head, the_debug);
				((struct func_node*) ((struct where_clause_node*) the_select_node->next->having_head->ptr_one)->ptr_one)->which_func = FUNC_AVG;
				((struct func_node*) ((struct where_clause_node*) the_select_node->next->having_head->ptr_one)->ptr_one)->distinct = false;
				((struct func_node*) ((struct where_clause_node*) the_select_node->next->having_head->ptr_one)->ptr_one)->args_size = 1;
				((struct func_node*) ((struct where_clause_node*) the_select_node->next->having_head->ptr_one)->ptr_one)->args_arr = (void**) myMalloc(sizeof(void*), NULL, &malloced_head, the_debug);
				((struct func_node*) ((struct where_clause_node*) the_select_node->next->having_head->ptr_one)->ptr_one)->args_arr[0] = ((struct select_node*) the_select_node)->columns_arr[1];
				((struct func_node*) ((struct where_clause_node*) the_select_node->next->having_head->ptr_one)->ptr_one)->args_arr_type = (int*) myMalloc(sizeof(int), NULL, &malloced_head, the_debug);
				((struct func_node*) ((struct where_clause_node*) the_select_node->next->having_head->ptr_one)->ptr_one)->args_arr_type[0] = PTR_TYPE_COL_IN_SELECT_NODE;
				((struct func_node*) ((struct where_clause_node*) the_select_node->next->having_head->ptr_one)->ptr_one)->group_by_cols_head = NULL;
				((struct func_node*) ((struct where_clause_node*) the_select_node->next->having_head->ptr_one)->ptr_one)->group_by_cols_tail = NULL;
				((struct where_clause_node*) the_select_node->next->having_head->ptr_one)->ptr_one_type = PTR_TYPE_FUNC_NODE;

				((struct where_clause_node*) the_select_node->next->having_head->ptr_one)->ptr_two = (int*) myMalloc(sizeof(int), NULL, &malloced_head, the_debug);
				*((int*) ((struct where_clause_node*) the_select_node->next->having_head->ptr_one)->ptr_two) = 10000;
				((struct where_clause_node*) the_select_node->next->having_head->ptr_one)->ptr_two_type = PTR_TYPE_INT;

				((struct where_clause_node*) the_select_node->next->having_head->ptr_one)->where_type = WHERE_IS_EQUALS;
				((struct where_clause_node*) the_select_node->next->having_head->ptr_one)->parent = the_select_node->next->having_head;

				((struct where_clause_node*) the_select_node->next->having_head->ptr_two)->ptr_one = (struct func_node*) myMalloc(sizeof(struct func_node), NULL, &malloced_head, the_debug);
				((struct func_node*) ((struct where_clause_node*) the_select_node->next->having_head->ptr_two)->ptr_one)->which_func = FUNC_COUNT;
				((struct func_node*) ((struct where_clause_node*) the_select_node->next->having_head->ptr_two)->ptr_one)->distinct = false;
				((struct func_node*) ((struct where_clause_node*) the_select_node->next->having_head->ptr_two)->ptr_one)->args_size = 1;
				((struct func_node*) ((struct where_clause_node*) the_select_node->next->having_head->ptr_two)->ptr_one)->args_arr = (void**) myMalloc(sizeof(void*), NULL, &malloced_head, the_debug);
				((struct func_node*) ((struct where_clause_node*) the_select_node->next->having_head->ptr_two)->ptr_one)->args_arr[0] = upper("*", NULL, &malloced_head, the_debug);
				((struct func_node*) ((struct where_clause_node*) the_select_node->next->having_head->ptr_two)->ptr_one)->args_arr_type = (int*) myMalloc(sizeof(int), NULL, &malloced_head, the_debug);
				((struct func_node*) ((struct where_clause_node*) the_select_node->next->having_head->ptr_two)->ptr_one)->args_arr_type[0] = PTR_TYPE_CHAR;
				((struct func_node*) ((struct where_clause_node*) the_select_node->next->having_head->ptr_two)->ptr_one)->group_by_cols_head = NULL;
				((struct func_node*) ((struct where_clause_node*) the_select_node->next->having_head->ptr_two)->ptr_one)->group_by_cols_tail = NULL;
				((struct where_clause_node*) the_select_node->next->having_head->ptr_two)->ptr_one_type = PTR_TYPE_FUNC_NODE;

				((struct where_clause_node*) the_select_node->next->having_head->ptr_two)->ptr_two = (int*) myMalloc(sizeof(int), NULL, &malloced_head, the_debug);
				*((int*) ((struct where_clause_node*) the_select_node->next->having_head->ptr_two)->ptr_two) = 1;
				((struct where_clause_node*) the_select_node->next->having_head->ptr_two)->ptr_two_type = PTR_TYPE_INT;

				((struct where_clause_node*) the_select_node->next->having_head->ptr_two)->where_type = WHERE_GREATER_THAN;
				((struct where_clause_node*) the_select_node->next->having_head->ptr_two)->parent = the_select_node->next->having_head;


			if (test_Controller_parseSelect(563, "select avg( CT-REGISTRATION-NUMBER),count( *), braND-name from alc_brands group by braND-name having avg( CT-REGISTRATION-NUMBER) = 10000 and count(*) > 1", &the_select_node
										   ,&parsed_error_code, &malloced_head, the_debug) != 0)
				return -1;
			
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
				return -3;
			}
		// END Test with id = 563

		// START Test with id = 564
			initSelectClauseForComp(&the_select_node, NULL, false, 0, NULL, getTablesHead(), PTR_TYPE_TABLE_INFO, NULL, NULL
								   ,&malloced_head, the_debug);

			col_arr = (struct col_in_select_node**) myMalloc(sizeof(struct col_in_select_node*) * 2, NULL, &malloced_head, the_debug);
			
			col_arr[0] = (struct col_in_select_node*) myMalloc(sizeof(struct col_in_select_node), NULL, &malloced_head, the_debug);
			col_arr[0]->table_ptr = NULL;
			col_arr[0]->table_ptr_type = -1;
			col_arr[0]->col_ptr = NULL;
			col_arr[0]->col_ptr_type = -1;
			col_arr[0]->new_name = upper("COUNT ( * ) ", NULL, &malloced_head, the_debug);
			col_arr[0]->math_node = NULL;
			col_arr[0]->case_node = NULL;

			col_arr[0]->func_node = (struct func_node*) myMalloc(sizeof(struct func_node), NULL, &malloced_head, the_debug);
			col_arr[0]->func_node->which_func = FUNC_COUNT;
			col_arr[0]->func_node->distinct = false;
			col_arr[0]->func_node->args_size = 1;
			col_arr[0]->func_node->args_arr = (void**) myMalloc(sizeof(void*), NULL, &malloced_head, the_debug);
			col_arr[0]->func_node->args_arr[0] = upper("*", NULL, &malloced_head, the_debug);
			col_arr[0]->func_node->args_arr_type = (int*) myMalloc(sizeof(int), NULL, &malloced_head, the_debug);
			col_arr[0]->func_node->args_arr_type[0] = PTR_TYPE_CHAR;
			col_arr[0]->func_node->group_by_cols_head = NULL;
			col_arr[0]->func_node->group_by_cols_tail = NULL;
			addListNodePtr(&col_arr[0]->func_node->group_by_cols_head, &col_arr[0]->func_node->group_by_cols_tail
						  ,((struct select_node*) the_select_node)->columns_arr[0], PTR_TYPE_COL_IN_SELECT_NODE, ADDLISTNODE_TAIL
				  		  ,NULL, &malloced_head, the_debug);

			col_arr[1] = (struct col_in_select_node*) myMalloc(sizeof(struct col_in_select_node), NULL, &malloced_head, the_debug);
			col_arr[1]->table_ptr = the_select_node;
			col_arr[1]->table_ptr_type = PTR_TYPE_SELECT_NODE;
			col_arr[1]->col_ptr = ((struct select_node*) the_select_node)->columns_arr[0];
			col_arr[1]->col_ptr_type = PTR_TYPE_COL_IN_SELECT_NODE;
			col_arr[1]->new_name = NULL;
			col_arr[1]->math_node = NULL;
			col_arr[1]->func_node = NULL;
			col_arr[1]->case_node = NULL;

			initSelectClauseForComp(&the_select_node->next, NULL, false, 2, col_arr, NULL, -1, NULL, NULL
								   ,&malloced_head, the_debug);
			the_select_node->next->prev = the_select_node;

			the_select_node->next->having_head = (struct where_clause_node*) myMalloc(sizeof(struct where_clause_node), NULL, &malloced_head, the_debug);

			the_select_node->next->having_head->ptr_one = (struct func_node*) myMalloc(sizeof(struct func_node), NULL, &malloced_head, the_debug);
			((struct func_node*) the_select_node->next->having_head->ptr_one)->which_func = FUNC_AVG;
			((struct func_node*) the_select_node->next->having_head->ptr_one)->distinct = false;
			((struct func_node*) the_select_node->next->having_head->ptr_one)->args_size = 1;
			((struct func_node*) the_select_node->next->having_head->ptr_one)->args_arr = (void**) myMalloc(sizeof(void*), NULL, &malloced_head, the_debug);
			((struct func_node*) the_select_node->next->having_head->ptr_one)->args_arr[0] = ((struct select_node*) the_select_node)->columns_arr[1];
			((struct func_node*) the_select_node->next->having_head->ptr_one)->args_arr_type = (int*) myMalloc(sizeof(int), NULL, &malloced_head, the_debug);
			((struct func_node*) the_select_node->next->having_head->ptr_one)->args_arr_type[0] = PTR_TYPE_COL_IN_SELECT_NODE;
			((struct func_node*) the_select_node->next->having_head->ptr_one)->group_by_cols_head = NULL;
			((struct func_node*) the_select_node->next->having_head->ptr_one)->group_by_cols_tail = NULL;
			the_select_node->next->having_head->ptr_one_type = PTR_TYPE_FUNC_NODE;

			the_select_node->next->having_head->ptr_two = (int*) myMalloc(sizeof(int), NULL, &malloced_head, the_debug);
			*((int*) the_select_node->next->having_head->ptr_two) = 1;
			the_select_node->next->having_head->ptr_two_type = PTR_TYPE_INT;

			the_select_node->next->having_head->where_type = WHERE_GREATER_THAN;
			the_select_node->next->having_head->parent = NULL;

			if (test_Controller_parseSelect(564, "select count( *), braND-name from alc_brands group by braND-name having avg(CT-REGISTRATION-NUMBER) > 1", &the_select_node
										   ,&parsed_error_code, &malloced_head, the_debug) != 0)
				return -1;
			
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
				return -3;
			}
		// END Test with id = 564

		// START Test with id = 565
			the_select_node = NULL;


			if (test_Controller_parseSelect(565, "select avg( CT-REGISTRATION-NUMBER),count( *), braND-name from alc_brands having avg( CT-REGISTRATION-NUMBER) = 10000 and count(*) > 1", &the_select_node
										   ,&parsed_error_code, &malloced_head, the_debug) != 0)
				return -1;
			

			if (malloced_head != NULL)
			{
				if (the_debug == YES_DEBUG)
				{
					setOutputRed();
					printf("	ERROR in test_Driver_main() at line %d in %s\n", __LINE__, __FILE__);
					setOutputWhite();
				}
				return -3;
			}
		// END Test with id = 565

		// START Test with id = 566
			the_select_node = NULL;


			if (test_Controller_parseSelect(566, "select count( *), braND-name from alc_brands group by CT-REGISTRATION-NUMBER having count(*) > 1", &the_select_node
										   ,&parsed_error_code, &malloced_head, the_debug) != 0)
				return -1;
			

			if (malloced_head != NULL)
			{
				if (the_debug == YES_DEBUG)
				{
					setOutputRed();
					printf("	ERROR in test_Driver_main() at line %d in %s\n", __LINE__, __FILE__);
					setOutputWhite();
				}
				return -3;
			}
		// END Test with id = 566

		// START Test with id = 567
			the_select_node = NULL;


			if (test_Controller_parseSelect(567, "select count( *), braND-name from alc_brands group by braND-name having avg(STATUS) > 1", &the_select_node
										   ,&parsed_error_code, &malloced_head, the_debug) != 0)
				return -1;
			

			if (malloced_head != NULL)
			{
				if (the_debug == YES_DEBUG)
				{
					setOutputRed();
					printf("	ERROR in test_Driver_main() at line %d in %s\n", __LINE__, __FILE__);
					setOutputWhite();
				}
				return -3;
			}
		// END Test with id = 567

		// START Test with id = 568
			initSelectClauseForComp(&the_select_node, NULL, false, 0, NULL, getTablesHead(), PTR_TYPE_TABLE_INFO, NULL, NULL
								   ,&malloced_head, the_debug);

			col_arr = (struct col_in_select_node**) myMalloc(sizeof(struct col_in_select_node*) * 2, NULL, &malloced_head, the_debug);
			
			col_arr[0] = (struct col_in_select_node*) myMalloc(sizeof(struct col_in_select_node), NULL, &malloced_head, the_debug);
			col_arr[0]->table_ptr = NULL;
			col_arr[0]->table_ptr_type = -1;
			col_arr[0]->col_ptr = NULL;
			col_arr[0]->col_ptr_type = -1;
			col_arr[0]->new_name = upper("COUNT ( * ) ", NULL, &malloced_head, the_debug);
			col_arr[0]->math_node = NULL;
			col_arr[0]->case_node = NULL;

			col_arr[0]->func_node = (struct func_node*) myMalloc(sizeof(struct func_node), NULL, &malloced_head, the_debug);
			col_arr[0]->func_node->which_func = FUNC_COUNT;
			col_arr[0]->func_node->distinct = false;
			col_arr[0]->func_node->args_size = 1;
			col_arr[0]->func_node->args_arr = (void**) myMalloc(sizeof(void*), NULL, &malloced_head, the_debug);
			col_arr[0]->func_node->args_arr[0] = upper("*", NULL, &malloced_head, the_debug);
			col_arr[0]->func_node->args_arr_type = (int*) myMalloc(sizeof(int), NULL, &malloced_head, the_debug);
			col_arr[0]->func_node->args_arr_type[0] = PTR_TYPE_CHAR;
			col_arr[0]->func_node->group_by_cols_head = NULL;
			col_arr[0]->func_node->group_by_cols_tail = NULL;
			addListNodePtr(&col_arr[0]->func_node->group_by_cols_head, &col_arr[0]->func_node->group_by_cols_tail
						  ,((struct select_node*) the_select_node)->columns_arr[0], PTR_TYPE_COL_IN_SELECT_NODE, ADDLISTNODE_TAIL
				  		  ,NULL, &malloced_head, the_debug);

			col_arr[1] = (struct col_in_select_node*) myMalloc(sizeof(struct col_in_select_node), NULL, &malloced_head, the_debug);
			col_arr[1]->table_ptr = the_select_node;
			col_arr[1]->table_ptr_type = PTR_TYPE_SELECT_NODE;
			col_arr[1]->col_ptr = ((struct select_node*) the_select_node)->columns_arr[0];
			col_arr[1]->col_ptr_type = PTR_TYPE_COL_IN_SELECT_NODE;
			col_arr[1]->new_name = NULL;
			col_arr[1]->math_node = NULL;
			col_arr[1]->func_node = NULL;
			col_arr[1]->case_node = NULL;

			initSelectClauseForComp(&the_select_node->next, NULL, false, 2, col_arr, NULL, -1, NULL, NULL
								   ,&malloced_head, the_debug);
			the_select_node->next->prev = the_select_node;

			the_select_node->next->where_head = (struct where_clause_node*) myMalloc(sizeof(struct where_clause_node), NULL, &malloced_head, the_debug);

			the_select_node->next->where_head->ptr_one = ((struct select_node*) the_select_node)->columns_arr[6];
			the_select_node->next->where_head->ptr_one_type = PTR_TYPE_COL_IN_SELECT_NODE;

			the_select_node->next->where_head->ptr_two = upper("LSL.0000933", NULL, &malloced_head, the_debug);
			the_select_node->next->where_head->ptr_two_type = PTR_TYPE_CHAR;

			the_select_node->next->where_head->where_type = WHERE_NOT_EQUALS;
			the_select_node->next->where_head->parent = NULL;


			if (test_Controller_parseSelect(568, "select count( *), braND-name from alc_brands where SUPERVISOR-CREDENTIAL <> 'LSL.0000933' group by braND-name;", &the_select_node
										   ,&parsed_error_code, &malloced_head, the_debug) != 0)
				return -1;
			
			parsed_error_code = 0;
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
				return -3;
			}
		// END Test with id = 568

		// START Test with id = 569
			initSelectClauseForComp(&the_select_node, NULL, false, 0, NULL, getTablesHead(), PTR_TYPE_TABLE_INFO, NULL, NULL
								   ,&malloced_head, the_debug);

			col_arr = (struct col_in_select_node**) myMalloc(sizeof(struct col_in_select_node*) * 2, NULL, &malloced_head, the_debug);
			
			col_arr[0] = (struct col_in_select_node*) myMalloc(sizeof(struct col_in_select_node), NULL, &malloced_head, the_debug);
			col_arr[0]->table_ptr = NULL;
			col_arr[0]->table_ptr_type = -1;
			col_arr[0]->col_ptr = NULL;
			col_arr[0]->col_ptr_type = -1;
			col_arr[0]->new_name = upper("COUNT ( * ) ", NULL, &malloced_head, the_debug);
			col_arr[0]->math_node = NULL;
			col_arr[0]->case_node = NULL;

			col_arr[0]->func_node = (struct func_node*) myMalloc(sizeof(struct func_node), NULL, &malloced_head, the_debug);
			col_arr[0]->func_node->which_func = FUNC_COUNT;
			col_arr[0]->func_node->distinct = false;
			col_arr[0]->func_node->args_size = 1;
			col_arr[0]->func_node->args_arr = (void**) myMalloc(sizeof(void*), NULL, &malloced_head, the_debug);
			col_arr[0]->func_node->args_arr[0] = upper("*", NULL, &malloced_head, the_debug);
			col_arr[0]->func_node->args_arr_type = (int*) myMalloc(sizeof(int), NULL, &malloced_head, the_debug);
			col_arr[0]->func_node->args_arr_type[0] = PTR_TYPE_CHAR;
			col_arr[0]->func_node->group_by_cols_head = NULL;
			col_arr[0]->func_node->group_by_cols_tail = NULL;
			addListNodePtr(&col_arr[0]->func_node->group_by_cols_head, &col_arr[0]->func_node->group_by_cols_tail
						  ,((struct select_node*) the_select_node)->columns_arr[0], PTR_TYPE_COL_IN_SELECT_NODE, ADDLISTNODE_TAIL
				  		  ,NULL, &malloced_head, the_debug);

			col_arr[1] = (struct col_in_select_node*) myMalloc(sizeof(struct col_in_select_node), NULL, &malloced_head, the_debug);
			col_arr[1]->table_ptr = the_select_node;
			col_arr[1]->table_ptr_type = PTR_TYPE_SELECT_NODE;
			col_arr[1]->col_ptr = ((struct select_node*) the_select_node)->columns_arr[0];
			col_arr[1]->col_ptr_type = PTR_TYPE_COL_IN_SELECT_NODE;
			col_arr[1]->new_name = NULL;
			col_arr[1]->math_node = NULL;
			col_arr[1]->func_node = NULL;
			col_arr[1]->case_node = NULL;

			initSelectClauseForComp(&the_select_node->next, NULL, false, 2, col_arr, NULL, -1, NULL, NULL
								   ,&malloced_head, the_debug);
			the_select_node->next->prev = the_select_node;


			the_select_node->next->where_head = (struct where_clause_node*) myMalloc(sizeof(struct where_clause_node), NULL, &malloced_head, the_debug);

			the_select_node->next->where_head->ptr_one = ((struct select_node*) the_select_node)->columns_arr[6];
			the_select_node->next->where_head->ptr_one_type = PTR_TYPE_COL_IN_SELECT_NODE;

			the_select_node->next->where_head->ptr_two = upper("LSL.0000933", NULL, &malloced_head, the_debug);
			the_select_node->next->where_head->ptr_two_type = PTR_TYPE_CHAR;

			the_select_node->next->where_head->where_type = WHERE_NOT_EQUALS;
			the_select_node->next->where_head->parent = NULL;


			the_select_node->next->having_head = (struct where_clause_node*) myMalloc(sizeof(struct where_clause_node), NULL, &malloced_head, the_debug);

			the_select_node->next->having_head->ptr_one = (struct func_node*) myMalloc(sizeof(struct func_node), NULL, &malloced_head, the_debug);
			((struct func_node*) the_select_node->next->having_head->ptr_one)->which_func = FUNC_COUNT;
			((struct func_node*) the_select_node->next->having_head->ptr_one)->distinct = false;
			((struct func_node*) the_select_node->next->having_head->ptr_one)->args_size = 1;
			((struct func_node*) the_select_node->next->having_head->ptr_one)->args_arr = (void**) myMalloc(sizeof(void*), NULL, &malloced_head, the_debug);
			((struct func_node*) the_select_node->next->having_head->ptr_one)->args_arr[0] = upper("*", NULL, &malloced_head, the_debug);
			((struct func_node*) the_select_node->next->having_head->ptr_one)->args_arr_type = (int*) myMalloc(sizeof(int), NULL, &malloced_head, the_debug);
			((struct func_node*) the_select_node->next->having_head->ptr_one)->args_arr_type[0] = PTR_TYPE_CHAR;
			((struct func_node*) the_select_node->next->having_head->ptr_one)->group_by_cols_head = NULL;
			((struct func_node*) the_select_node->next->having_head->ptr_one)->group_by_cols_tail = NULL;
			the_select_node->next->having_head->ptr_one_type = PTR_TYPE_FUNC_NODE;

			the_select_node->next->having_head->ptr_two = (int*) myMalloc(sizeof(int), NULL, &malloced_head, the_debug);
			*((int*) the_select_node->next->having_head->ptr_two) = 1;
			the_select_node->next->having_head->ptr_two_type = PTR_TYPE_INT;

			the_select_node->next->having_head->where_type = WHERE_GREATER_THAN;
			the_select_node->next->having_head->parent = NULL;


			order_by = (struct order_by_node*) myMalloc(sizeof(struct order_by_node), NULL, &malloced_head, the_debug);
			order_by->order_by_cols_head = NULL;
			order_by->order_by_cols_tail = NULL;
			order_by->order_by_cols_which_head = NULL;
			order_by->order_by_cols_which_tail = NULL;

			addListNodePtr(&order_by->order_by_cols_head, &order_by->order_by_cols_tail, the_select_node->next->columns_arr[1], PTR_TYPE_COL_IN_SELECT_NODE, ADDLISTNODE_TAIL
						  ,NULL, &malloced_head, the_debug);
			which = (int*) myMalloc(sizeof(int), NULL, &malloced_head, the_debug);
			*which = ORDER_BY_ASC;
			addListNodePtr(&order_by->order_by_cols_which_head, &order_by->order_by_cols_which_tail, which, PTR_TYPE_INT, ADDLISTNODE_TAIL
						  ,NULL, &malloced_head, the_debug);

			the_select_node->next->order_by = order_by;


			if (test_Controller_parseSelect(569, "select count( *), braND-name from alc_brands where SUPERVISOR-CREDENTIAL <> 'LSL.0000933' group by braND-name having count( *) > 1 order by braND-name asc", &the_select_node
										   ,&parsed_error_code, &malloced_head, the_debug) != 0)
				return -1;
			
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
				return -3;
			}
		// END Test with id = 569

		// START Test with id = 570
			initSelectClauseForComp(&the_select_node, NULL, false, 0, NULL, getTablesHead(), PTR_TYPE_TABLE_INFO, NULL, NULL
								   ,&malloced_head, the_debug);
			initSelectClauseForComp(&the_select_node->next, "ALC1", false, 0, NULL, the_select_node, PTR_TYPE_SELECT_NODE, NULL, NULL
								   ,&malloced_head, the_debug);
			the_select_node->next->prev = the_select_node;

			initSelectClauseForComp(&joined_select, "ALC2", false, 0, NULL, getTablesHead(), PTR_TYPE_TABLE_INFO, NULL, NULL
								   ,&malloced_head, the_debug);

			col_arr = (struct col_in_select_node**) myMalloc(sizeof(struct col_in_select_node*) * 5, NULL, &malloced_head, the_debug);

			index = 0;
			for (int i=0; i<5; i++)
			{
				col_arr[index] = (struct col_in_select_node*) myMalloc(sizeof(struct col_in_select_node), NULL, &malloced_head, the_debug);

				if (i == 0)
				{
					col_arr[index]->table_ptr = the_select_node->next;
					col_arr[index]->table_ptr_type = PTR_TYPE_SELECT_NODE;

					col_arr[index]->col_ptr = the_select_node->next->columns_arr[0];
					col_arr[index]->col_ptr_type = PTR_TYPE_COL_IN_SELECT_NODE;
				}
				else if (i == 1)
				{
					col_arr[index]->table_ptr = the_select_node->next;
					col_arr[index]->table_ptr_type = PTR_TYPE_SELECT_NODE;

					col_arr[index]->col_ptr = the_select_node->next->columns_arr[1];
					col_arr[index]->col_ptr_type = PTR_TYPE_COL_IN_SELECT_NODE;
				}
				else if (i == 2)
				{
					col_arr[index]->table_ptr = the_select_node->next;
					col_arr[index]->table_ptr_type = PTR_TYPE_SELECT_NODE;

					col_arr[index]->col_ptr = the_select_node->next->columns_arr[6];
					col_arr[index]->col_ptr_type = PTR_TYPE_COL_IN_SELECT_NODE;
				}
				else if (i > 2)
				{
					col_arr[index]->table_ptr = NULL;
					col_arr[index]->table_ptr_type = -1;

					col_arr[index]->col_ptr = NULL;
					col_arr[index]->col_ptr_type = -1;
				}

				col_arr[index]->new_name = NULL;
				col_arr[index]->func_node = NULL;
				col_arr[index]->math_node = NULL;
				col_arr[index]->case_node = NULL;

				index++;
			}


			col_arr[3]->new_name = (char*) myMalloc(sizeof(char) * 16, NULL, &malloced_head, the_debug);
			strcpy(col_arr[3]->new_name, "TEST_CASE");

			col_arr[3]->case_node = (struct case_node*) myMalloc(sizeof(struct case_node), NULL, &malloced_head, the_debug);
			col_arr[3]->case_node->case_when_head = NULL;
			col_arr[3]->case_node->case_then_value_head = NULL;

			the_where_node = (struct where_clause_node*) myMalloc(sizeof(struct where_clause_node), NULL, &malloced_head, the_debug);
			the_where_node->ptr_one = joined_select->columns_arr[0];
			the_where_node->ptr_one_type = PTR_TYPE_COL_IN_SELECT_NODE;
			the_where_node->ptr_two = (char*) myMalloc(sizeof(char*) * 16, NULL, &malloced_head, the_debug);
			strcpy(the_where_node->ptr_two, "test");
			the_where_node->ptr_two_type = PTR_TYPE_CHAR;
			the_where_node->where_type = WHERE_IS_EQUALS;
			the_where_node->parent = NULL;

			addListNodePtr(&col_arr[3]->case_node->case_when_head, &col_arr[3]->case_node->case_when_tail, the_where_node, PTR_TYPE_WHERE_CLAUSE_NODE, ADDLISTNODE_TAIL
						  ,NULL, &malloced_head, the_debug);
			addListNodePtr(&col_arr[3]->case_node->case_when_head, &col_arr[3]->case_node->case_when_tail, NULL, -1, ADDLISTNODE_TAIL
						  ,NULL, &malloced_head, the_debug);

			int* int1 = (int*) myMalloc(sizeof(int), NULL, &malloced_head, the_debug);
			*int1 = 1;
			int* int2 = (int*) myMalloc(sizeof(int), NULL, &malloced_head, the_debug);
			*int2 = 0;

			addListNodePtr(&col_arr[3]->case_node->case_then_value_head, &col_arr[3]->case_node->case_then_value_tail, int1, PTR_TYPE_INT, ADDLISTNODE_TAIL
						  ,NULL, &malloced_head, the_debug);
			addListNodePtr(&col_arr[3]->case_node->case_then_value_head, &col_arr[3]->case_node->case_then_value_tail, int2, PTR_TYPE_INT, ADDLISTNODE_TAIL
						  ,NULL, &malloced_head, the_debug);


			col_arr[4]->table_ptr = NULL;
			col_arr[4]->table_ptr_type = -1;
			col_arr[4]->col_ptr = NULL;
			col_arr[4]->col_ptr_type = -1;
			col_arr[4]->new_name = upper("COUNT ( * ) ", NULL, &malloced_head, the_debug);
			col_arr[4]->math_node = NULL;
			col_arr[4]->case_node = NULL;

			col_arr[4]->func_node = (struct func_node*) myMalloc(sizeof(struct func_node), NULL, &malloced_head, the_debug);
			col_arr[4]->func_node->which_func = FUNC_COUNT;
			col_arr[4]->func_node->distinct = false;
			col_arr[4]->func_node->args_size = 1;
			col_arr[4]->func_node->args_arr = (void**) myMalloc(sizeof(void*), NULL, &malloced_head, the_debug);
			col_arr[4]->func_node->args_arr[0] = upper("*", NULL, &malloced_head, the_debug);
			col_arr[4]->func_node->args_arr_type = (int*) myMalloc(sizeof(int), NULL, &malloced_head, the_debug);
			col_arr[4]->func_node->args_arr_type[0] = PTR_TYPE_CHAR;
			col_arr[4]->func_node->group_by_cols_head = NULL;
			col_arr[4]->func_node->group_by_cols_tail = NULL;
			addListNodePtr(&col_arr[4]->func_node->group_by_cols_head, &col_arr[4]->func_node->group_by_cols_tail
						  ,((struct select_node*) the_select_node->next)->columns_arr[0], PTR_TYPE_COL_IN_SELECT_NODE, ADDLISTNODE_TAIL
				  		  ,NULL, &malloced_head, the_debug);
			addListNodePtr(&col_arr[4]->func_node->group_by_cols_head, &col_arr[4]->func_node->group_by_cols_tail
						  ,((struct select_node*) the_select_node->next)->columns_arr[1], PTR_TYPE_COL_IN_SELECT_NODE, ADDLISTNODE_TAIL
				  		  ,NULL, &malloced_head, the_debug);
			addListNodePtr(&col_arr[4]->func_node->group_by_cols_head, &col_arr[4]->func_node->group_by_cols_tail
						  ,((struct select_node*) the_select_node->next)->columns_arr[6], PTR_TYPE_COL_IN_SELECT_NODE, ADDLISTNODE_TAIL
				  		  ,NULL, &malloced_head, the_debug);
			addListNodePtr(&col_arr[4]->func_node->group_by_cols_head, &col_arr[4]->func_node->group_by_cols_tail
						  ,((struct select_node*) joined_select)->columns_arr[0], PTR_TYPE_COL_IN_SELECT_NODE, ADDLISTNODE_TAIL
				  		  ,NULL, &malloced_head, the_debug);


			initSelectClauseForComp(&the_select_node->next->next, NULL, false, 5, col_arr, NULL, -1, NULL, NULL
								   ,&malloced_head, the_debug);
			the_select_node->next->next->prev = the_select_node->next;

			the_select_node->next->next->join_head = (struct join_node*) myMalloc(sizeof(struct join_node), NULL, &malloced_head, the_debug);
			the_select_node->next->next->join_head->join_type = JOIN_INNER;
			the_select_node->next->next->join_head->select_joined = joined_select;
			the_select_node->next->next->join_head->prev = NULL;
			the_select_node->next->next->join_head->next = NULL;

			the_select_node->next->next->join_head->on_clause_head = (struct where_clause_node*) myMalloc(sizeof(struct where_clause_node), NULL, &malloced_head, the_debug);
			the_select_node->next->next->join_head->on_clause_head->ptr_one = the_select_node->next->columns_arr[0];
			the_select_node->next->next->join_head->on_clause_head->ptr_one_type = PTR_TYPE_COL_IN_SELECT_NODE;
			the_select_node->next->next->join_head->on_clause_head->ptr_two = joined_select->columns_arr[0];
			the_select_node->next->next->join_head->on_clause_head->ptr_two_type = PTR_TYPE_COL_IN_SELECT_NODE;
			the_select_node->next->next->join_head->on_clause_head->where_type = WHERE_IS_EQUALS;
			the_select_node->next->next->join_head->on_clause_head->parent = NULL;


			the_select_node->next->next->where_head = (struct where_clause_node*) myMalloc(sizeof(struct where_clause_node), NULL, &malloced_head, the_debug);

			the_select_node->next->next->where_head->ptr_one = ((struct select_node*) the_select_node->next)->columns_arr[6];
			the_select_node->next->next->where_head->ptr_one_type = PTR_TYPE_COL_IN_SELECT_NODE;

			the_select_node->next->next->where_head->ptr_two = upper("LSL.0000933", NULL, &malloced_head, the_debug);
			the_select_node->next->next->where_head->ptr_two_type = PTR_TYPE_CHAR;

			the_select_node->next->next->where_head->where_type = WHERE_NOT_EQUALS;
			the_select_node->next->next->where_head->parent = NULL;


			the_select_node->next->next->having_head = (struct where_clause_node*) myMalloc(sizeof(struct where_clause_node), NULL, &malloced_head, the_debug);

			the_select_node->next->next->having_head->ptr_one = (struct func_node*) myMalloc(sizeof(struct func_node), NULL, &malloced_head, the_debug);
			((struct func_node*) the_select_node->next->next->having_head->ptr_one)->which_func = FUNC_COUNT;
			((struct func_node*) the_select_node->next->next->having_head->ptr_one)->distinct = false;
			((struct func_node*) the_select_node->next->next->having_head->ptr_one)->args_size = 1;
			((struct func_node*) the_select_node->next->next->having_head->ptr_one)->args_arr = (void**) myMalloc(sizeof(void*), NULL, &malloced_head, the_debug);
			((struct func_node*) the_select_node->next->next->having_head->ptr_one)->args_arr[0] = upper("*", NULL, &malloced_head, the_debug);
			((struct func_node*) the_select_node->next->next->having_head->ptr_one)->args_arr_type = (int*) myMalloc(sizeof(int), NULL, &malloced_head, the_debug);
			((struct func_node*) the_select_node->next->next->having_head->ptr_one)->args_arr_type[0] = PTR_TYPE_CHAR;
			((struct func_node*) the_select_node->next->next->having_head->ptr_one)->group_by_cols_head = NULL;
			((struct func_node*) the_select_node->next->next->having_head->ptr_one)->group_by_cols_tail = NULL;
			the_select_node->next->next->having_head->ptr_one_type = PTR_TYPE_FUNC_NODE;

			the_select_node->next->next->having_head->ptr_two = (int*) myMalloc(sizeof(int), NULL, &malloced_head, the_debug);
			*((int*) the_select_node->next->next->having_head->ptr_two) = 1;
			the_select_node->next->next->having_head->ptr_two_type = PTR_TYPE_INT;

			the_select_node->next->next->having_head->where_type = WHERE_GREATER_THAN;
			the_select_node->next->next->having_head->parent = NULL;
			

			order_by = (struct order_by_node*) myMalloc(sizeof(struct order_by_node), NULL, &malloced_head, the_debug);
			order_by->order_by_cols_head = NULL;
			order_by->order_by_cols_tail = NULL;
			order_by->order_by_cols_which_head = NULL;
			order_by->order_by_cols_which_tail = NULL;

			addListNodePtr(&order_by->order_by_cols_head, &order_by->order_by_cols_tail, the_select_node->next->next->columns_arr[0], PTR_TYPE_COL_IN_SELECT_NODE, ADDLISTNODE_TAIL
						  ,NULL, &malloced_head, the_debug);
			which = (int*) myMalloc(sizeof(int), NULL, &malloced_head, the_debug);
			*which = ORDER_BY_ASC;
			addListNodePtr(&order_by->order_by_cols_which_head, &order_by->order_by_cols_which_tail, which, PTR_TYPE_INT, ADDLISTNODE_TAIL
						  ,NULL, &malloced_head, the_debug);

			the_select_node->next->next->order_by = order_by;


			/*
			select alc1.brand-name, alc1.CT-REGISTRATION-NUMBER, alc1.SUPERVISOR-CREDENTIAL  
				  ,case when alc2.brand-name = 'test' then 1 
				  		else 0	
				  end Test_case	
				  ,count(*) 
			from
			(	
			  	select * from alc_brands
			) alc1 
			join alc_brands alc2 
			on alc1.brand-name = alc2.brand-name 
			where alc1.SUPERVISOR-CREDENTIAL <> 'LSL.0000933' 
			group by alc1.brand-name, alc1.CT-REGISTRATION-NUMBER, alc1.SUPERVISOR-CREDENTIAL, alc2.brand-name 
			having count( *) > 1  
			order by alc1.braND-name asc
			*/
			if (test_Controller_parseSelect(570, "select alc1.brand-name, alc1.CT-REGISTRATION-NUMBER, alc1.SUPERVISOR-CREDENTIAL  ,case when alc2.brand-name = 'test' then 1 else 0	end Test_case	,count(*) from(	select * from alc_brands) alc1 join alc_brands  alc2 on alc1.brand-name = alc2.brand-name where alc1.SUPERVISOR-CREDENTIAL <> 'LSL.0000933' group by alc1.brand-name, alc1.CT-REGISTRATION-NUMBER, alc1.SUPERVISOR-CREDENTIAL, alc2.brand-name having count( *) > 1  order by alc1.braND-name asc ", &the_select_node
										   ,&parsed_error_code, &malloced_head, the_debug) != 0)
				return -1;
			
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
				return -3;
			}
		// END Test with id = 570
	// END test_Controller_parseSelect

	// START test_Controller_parseUpdate
		// START Test with id = 201
			struct change_node_v2* expected_change_head = NULL;

			parsed_error_code = 0;
			if (test_Controller_parseUpdate(201, "UPDATE * 	 from   \n    table set 		this = 'that'   ;"
										   ,&expected_change_head, &parsed_error_code, &malloced_head, the_debug) != 0)
				return -1;

			if (malloced_head != NULL)
			{
				if (the_debug == YES_DEBUG)
				{
					setOutputRed();
					printf("	ERROR in test_Driver_main() at line %d in %s\n", __LINE__, __FILE__);
					setOutputWhite();
				}
				return -3;
			}
		// END Test with id = 201

		// START Test with id = 202
			expected_change_head = (struct change_node_v2*) myMalloc(sizeof(struct change_node_v2), NULL, &malloced_head, the_debug);
			expected_change_head->table = getTablesHead();
			expected_change_head->operation = OP_UPDATE;
			expected_change_head->col_list_head = NULL;
			expected_change_head->col_list_tail = NULL;
			expected_change_head->data_list_head = NULL;
			expected_change_head->data_list_tail = NULL;
			expected_change_head->where_head = NULL;

			addListNodePtr(&expected_change_head->col_list_head, &expected_change_head->col_list_tail, getTablesHead()->table_cols_head->next->next, PTR_TYPE_TABLE_COLS_INFO, ADDLISTNODE_TAIL
						  ,NULL, &malloced_head, the_debug);

			char* str1 = (char*) myMalloc(sizeof(char) * 32, NULL, &malloced_head, the_debug);
			strcpy(str1, "TST_ACTIVE");
			addListNodePtr(&expected_change_head->data_list_head, &expected_change_head->data_list_tail, str1, PTR_TYPE_CHAR, ADDLISTNODE_TAIL
						  ,NULL, &malloced_head, the_debug);

			if (test_Controller_parseUpdate(202, "UPDATE ALC_BRANDS\nSET status = 'TST_ACTIVE'\n;"
										   ,&expected_change_head, &parsed_error_code, &malloced_head, the_debug) != 0)
				return -1;

			if (parsed_error_code == 0)
			{
				freeAnyLinkedList((void**) &expected_change_head, PTR_TYPE_CHANGE_NODE_V2, NULL, &malloced_head, the_debug);
			}

			if (malloced_head != NULL)
			{
				if (the_debug == YES_DEBUG)
				{
					setOutputRed();
					printf("	ERROR in test_Driver_main() at line %d in %s\n", __LINE__, __FILE__);
					setOutputWhite();
				}
				return -3;
			}
		// END Test with id = 202

		// START Test with id = 203
			expected_change_head = (struct change_node_v2*) myMalloc(sizeof(struct change_node_v2), NULL, &malloced_head, the_debug);
			expected_change_head->table = getTablesHead();
			expected_change_head->operation = OP_UPDATE;
			expected_change_head->col_list_head = NULL;
			expected_change_head->col_list_tail = NULL;
			expected_change_head->data_list_head = NULL;
			expected_change_head->data_list_tail = NULL;
			expected_change_head->where_head = NULL;

			addListNodePtr(&expected_change_head->col_list_head, &expected_change_head->col_list_tail, getTablesHead()->table_cols_head->next->next, PTR_TYPE_TABLE_COLS_INFO, ADDLISTNODE_TAIL
						  ,NULL, &malloced_head, the_debug);

			str1 = (char*) myMalloc(sizeof(char) * 32, NULL, &malloced_head, the_debug);
			strcpy(str1, "TST_ACTIVE");
			addListNodePtr(&expected_change_head->data_list_head, &expected_change_head->data_list_tail, str1, PTR_TYPE_CHAR, ADDLISTNODE_TAIL
						  ,NULL, &malloced_head, the_debug);

			addListNodePtr(&expected_change_head->col_list_head, &expected_change_head->col_list_tail, getTablesHead()->table_cols_head->next->next->next, PTR_TYPE_TABLE_COLS_INFO, ADDLISTNODE_TAIL
						  ,NULL, &malloced_head, the_debug);

			char* str2 = (char*) myMalloc(sizeof(char) * 32, NULL, &malloced_head, the_debug);
			strcpy(str2, "1/1/1900");
			addListNodePtr(&expected_change_head->data_list_head, &expected_change_head->data_list_tail, str2, PTR_TYPE_DATE, ADDLISTNODE_TAIL
						  ,NULL, &malloced_head, the_debug);

			
			if (test_Controller_parseUpdate(203, "UPDATE  ALC_BRANDS  SET  status  =  'TST_ACTIVE'\n  	,  	\nEFFECTIVE  =  '1/1/1900'	\n	;"
										   ,&expected_change_head, &parsed_error_code, &malloced_head, the_debug) != 0)
				return -1;

			if (parsed_error_code == 0)
			{
				freeAnyLinkedList((void**) &expected_change_head, PTR_TYPE_CHANGE_NODE_V2, NULL, &malloced_head, the_debug);
			}

			if (malloced_head != NULL)
			{
				if (the_debug == YES_DEBUG)
				{
					setOutputRed();
					printf("	ERROR in test_Driver_main() at line %d in %s\n", __LINE__, __FILE__);
					setOutputWhite();
				}
				return -3;
			}
		// END Test with id = 203

		// START Test with id = 204
			expected_change_head = (struct change_node_v2*) myMalloc(sizeof(struct change_node_v2), NULL, &malloced_head, the_debug);
			expected_change_head->table = getTablesHead();
			expected_change_head->operation = OP_UPDATE;
			expected_change_head->col_list_head = NULL;
			expected_change_head->col_list_tail = NULL;
			expected_change_head->data_list_head = NULL;
			expected_change_head->data_list_tail = NULL;
			expected_change_head->where_head = NULL;

			addListNodePtr(&expected_change_head->col_list_head, &expected_change_head->col_list_tail, getTablesHead()->table_cols_head->next, PTR_TYPE_TABLE_COLS_INFO, ADDLISTNODE_TAIL
						  ,NULL, &malloced_head, the_debug);

			int1 = (int*) myMalloc(sizeof(int), NULL, &malloced_head, the_debug);
			*int1 = 2;
			addListNodePtr(&expected_change_head->data_list_head, &expected_change_head->data_list_tail, int1, PTR_TYPE_INT, ADDLISTNODE_TAIL
						  ,NULL, &malloced_head, the_debug);

			addListNodePtr(&expected_change_head->col_list_head, &expected_change_head->col_list_tail, getTablesHead()->table_cols_head->next->next->next, PTR_TYPE_TABLE_COLS_INFO, ADDLISTNODE_TAIL
						  ,NULL, &malloced_head, the_debug);

			str2 = (char*) myMalloc(sizeof(char) * 32, NULL, &malloced_head, the_debug);
			strcpy(str2, "1/1/1900");
			addListNodePtr(&expected_change_head->data_list_head, &expected_change_head->data_list_tail, str2, PTR_TYPE_DATE, ADDLISTNODE_TAIL
						  ,NULL, &malloced_head, the_debug);

			expected_change_head->where_head = myMalloc(sizeof(struct where_clause_node), NULL, &malloced_head, the_debug);
			expected_change_head->where_head->ptr_one = getTablesHead()->table_cols_head;
			expected_change_head->where_head->ptr_one_type = PTR_TYPE_TABLE_COLS_INFO;
			char* str3 = (char*) myMalloc(sizeof(char) * 32, NULL, &malloced_head, the_debug);
			strcpy(str3, "that");
			expected_change_head->where_head->ptr_two = str3;
			expected_change_head->where_head->ptr_two_type = PTR_TYPE_CHAR;
			expected_change_head->where_head->where_type = WHERE_IS_EQUALS;
			expected_change_head->where_head->parent = NULL;

			
			if (test_Controller_parseUpdate(204, "update alc_brands set CT-REGISTRATION-NUMBER = 2,EFFECTIVE = '1/1/1900' where BRAND-NAME = 'that';"
										   ,&expected_change_head, &parsed_error_code, &malloced_head, the_debug) != 0)
				return -1;

			if (parsed_error_code == 0)
			{
				freeAnyLinkedList((void**) &expected_change_head, PTR_TYPE_CHANGE_NODE_V2, NULL, &malloced_head, the_debug);
			}

			if (malloced_head != NULL)
			{
				if (the_debug == YES_DEBUG)
				{
					setOutputRed();
					printf("	ERROR in test_Driver_main() at line %d in %s\n", __LINE__, __FILE__);
					setOutputWhite();
				}
				return -3;
			}
		// END Test with id = 204

		// START Test with id = 205
			expected_change_head = NULL;

			
			if (test_Controller_parseUpdate(205, "update alc_brands set BRAND-NAME = 2 , EFFECTIVE = '1/1/1900';"
										   ,&expected_change_head, &parsed_error_code, &malloced_head, the_debug) != 0)
				return -1;

			if (malloced_head != NULL)
			{
				if (the_debug == YES_DEBUG)
				{
					setOutputRed();
					printf("	ERROR in test_Driver_main() at line %d in %s\n", __LINE__, __FILE__);
					setOutputWhite();
				}
				return -3;
			}
		// END Test with id = 205

		// START Test with id = 206
			expected_change_head = NULL;

			
			if (test_Controller_parseUpdate(206, "update alc_brands set '1/1/1900' = EFFECTIVE, BRAND-NAME = 'this';"
										   ,&expected_change_head, &parsed_error_code, &malloced_head, the_debug) != 0)
				return -1;

			if (malloced_head != NULL)
			{
				if (the_debug == YES_DEBUG)
				{
					setOutputRed();
					printf("	ERROR in test_Driver_main() at line %d in %s\n", __LINE__, __FILE__);
					setOutputWhite();
				}
				return -3;
			}
		// END Test with id = 206

		// START Test with id = 207
			expected_change_head = NULL;

			
			if (test_Controller_parseUpdate(207, "update alc_brands set 2 = CT-REGISTRATION-NUMBER, BRAND-NAME = 'this';"
										   ,&expected_change_head, &parsed_error_code, &malloced_head, the_debug) != 0)
				return -1;

			if (malloced_head != NULL)
			{
				if (the_debug == YES_DEBUG)
				{
					setOutputRed();
					printf("	ERROR in test_Driver_main() at line %d in %s\n", __LINE__, __FILE__);
					setOutputWhite();
				}
				return -3;
			}
		// END Test with id = 207

		// START Test with id = 208
			expected_change_head = NULL;

			
			if (test_Controller_parseUpdate(208, "update alc_brands set BRAND-NAME = 'this' CT-REGISTRATION-NUMBER = 2 ;"
										   ,&expected_change_head, &parsed_error_code, &malloced_head, the_debug) != 0)
				return -1;

			if (malloced_head != NULL)
			{
				if (the_debug == YES_DEBUG)
				{
					setOutputRed();
					printf("	ERROR in test_Driver_main() at line %d in %s\n", __LINE__, __FILE__);
					setOutputWhite();
				}
				return -3;
			}
		// END Test with id = 208

		// START Test with id = 209
			expected_change_head = NULL;

			
			if (test_Controller_parseUpdate(209, "update alc_brands set BRAND-NAME = 'this', CT-REGISTRATION-NUMBER = 2 ahwdbahwuid where 1 = 1;"
										   ,&expected_change_head, &parsed_error_code, &malloced_head, the_debug) != 0)
				return -1;

			if (malloced_head != NULL)
			{
				if (the_debug == YES_DEBUG)
				{
					setOutputRed();
					printf("	ERROR in test_Driver_main() at line %d in %s\n", __LINE__, __FILE__);
					setOutputWhite();
				}
				return -3;
			}
		// END Test with id = 209
	// END test_Controller_parseUpdate
	
	// START test_Controller_parseDelete
		// START Test with id = 301
			expected_change_head = (struct change_node_v2*) myMalloc(sizeof(struct change_node_v2), NULL, &malloced_head, the_debug);
			expected_change_head->table = getTablesHead();
			expected_change_head->operation = OP_DELETE;
			expected_change_head->col_list_head = NULL;
			expected_change_head->col_list_tail = NULL;
			expected_change_head->data_list_head = NULL;
			expected_change_head->data_list_tail = NULL;
			expected_change_head->where_head = NULL;

			expected_change_head->where_head = myMalloc(sizeof(struct where_clause_node), NULL, &malloced_head, the_debug);
			expected_change_head->where_head->ptr_one = getTablesHead()->table_cols_head;
			expected_change_head->where_head->ptr_one_type = PTR_TYPE_TABLE_COLS_INFO;
			str1 = (char*) myMalloc(sizeof(char) * 32, NULL, &malloced_head, the_debug);
			strcpy(str1, "860 INDA PALE ALE");
			expected_change_head->where_head->ptr_two = str1;
			expected_change_head->where_head->ptr_two_type = PTR_TYPE_CHAR;
			expected_change_head->where_head->where_type = WHERE_IS_EQUALS;
			expected_change_head->where_head->parent = NULL;


			if (test_Controller_parseDelete(301, "delete from alc_brands where BRAND-NAME = '860 INDA PALE ALE';"
										   ,&expected_change_head, &parsed_error_code, &malloced_head, the_debug) != 0)
				return -1;

			if (parsed_error_code == 0)
			{
				freeAnyLinkedList((void**) &expected_change_head, PTR_TYPE_CHANGE_NODE_V2, NULL, &malloced_head, the_debug);
			}

			if (malloced_head != NULL)
			{
				if (the_debug == YES_DEBUG)
				{
					setOutputRed();
					printf("	ERROR in test_Driver_main() at line %d in %s\n", __LINE__, __FILE__);
					setOutputWhite();
				}
				return -3;
			}
		// END Test with id = 301

		// START Test with id = 302
			expected_change_head = NULL;


			if (test_Controller_parseDelete(302, "   Delete	 \n	from	ALC_BRANDS 	 where this = 'that'  ;"
										   ,&expected_change_head, &parsed_error_code, &malloced_head, the_debug) != 0)
				return -1;

			if (malloced_head != NULL)
			{
				if (the_debug == YES_DEBUG)
				{
					setOutputRed();
					printf("	ERROR in test_Driver_main() at line %d in %s\n", __LINE__, __FILE__);
					setOutputWhite();
				}
				return -3;
			}
		// END Test with id = 302

		// START Test with id = 303
			expected_change_head = NULL;


			if (test_Controller_parseDelete(303, "   Delete\nFrom\n	ALC_BRANDS\nwhere;"
										   ,&expected_change_head, &parsed_error_code, &malloced_head, the_debug) != 0)
				return -1;

			if (malloced_head != NULL)
			{
				if (the_debug == YES_DEBUG)
				{
					setOutputRed();
					printf("	ERROR in test_Driver_main() at line %d in %s\n", __LINE__, __FILE__);
					setOutputWhite();
				}
				return -3;
			}
		// END Test with id = 303

		// START Test with id = 304
			expected_change_head = (struct change_node_v2*) myMalloc(sizeof(struct change_node_v2), NULL, &malloced_head, the_debug);
			expected_change_head->table = getTablesHead();
			expected_change_head->operation = OP_DELETE;
			expected_change_head->col_list_head = NULL;
			expected_change_head->col_list_tail = NULL;
			expected_change_head->data_list_head = NULL;
			expected_change_head->data_list_tail = NULL;
			expected_change_head->where_head = NULL;


			if (test_Controller_parseDelete(304, "\nDelete\nfrom\n	ALC_BRANDS"
										   ,&expected_change_head, &parsed_error_code, &malloced_head, the_debug) != 0)
				return -1;

			if (parsed_error_code == 0)
			{
				freeAnyLinkedList((void**) &expected_change_head, PTR_TYPE_CHANGE_NODE_V2, NULL, &malloced_head, the_debug);
			}

			if (malloced_head != NULL)
			{
				if (the_debug == YES_DEBUG)
				{
					setOutputRed();
					printf("	ERROR in test_Driver_main() at line %d in %s\n", __LINE__, __FILE__);
					setOutputWhite();
				}
				return -3;
			}
		// END Test with id = 304
		
		// START Test with id = 305
			expected_change_head = NULL;


			if (test_Controller_parseDelete(305, "\ndelete\n\n	ALC_BRANDS"
										   ,&expected_change_head, &parsed_error_code, &malloced_head, the_debug) != 0)
				return -1;

			if (malloced_head != NULL)
			{
				if (the_debug == YES_DEBUG)
				{
					setOutputRed();
					printf("	ERROR in test_Driver_main() at line %d in %s\n", __LINE__, __FILE__);
					setOutputWhite();
				}
				return -3;
			}
		// END Test with id = 305

		// START Test with id = 306
			expected_change_head = NULL;


			if (test_Controller_parseDelete(306, "delete from ALC_BRANDS where BRAND-NAME = that"
										   ,&expected_change_head, &parsed_error_code, &malloced_head, the_debug) != 0)
				return -1;

			if (malloced_head != NULL)
			{
				if (the_debug == YES_DEBUG)
				{
					setOutputRed();
					printf("	ERROR in test_Driver_main() at line %d in %s\n", __LINE__, __FILE__);
					setOutputWhite();
				}
				return -3;
			}
		// END Test with id = 306
	// END test_Controller_parseDelete

	// START test_Controller_parseInsert
		// START Test with id = 401
			expected_change_head = NULL;


			if (test_Controller_parseInsert(401, "Insert into alc_brands (col1, col2) values ('Hi', 'Hello');"
										   ,&expected_change_head, &parsed_error_code, &malloced_head, the_debug) != 0)
				return -1;

			if (malloced_head != NULL)
			{
				if (the_debug == YES_DEBUG)
				{
					setOutputRed();
					printf("	ERROR in test_Driver_main() at line %d in %s\n", __LINE__, __FILE__);
					setOutputWhite();
				}
				return -3;
			}
		// END Test with id = 401

		// START Test with id = 402
			expected_change_head = (struct change_node_v2*) myMalloc(sizeof(struct change_node_v2), NULL, &malloced_head, the_debug);
			expected_change_head->table = getTablesHead();
			expected_change_head->operation = OP_INSERT;
			expected_change_head->col_list_head = NULL;
			expected_change_head->col_list_tail = NULL;
			expected_change_head->data_list_head = NULL;
			expected_change_head->data_list_tail = NULL;
			expected_change_head->where_head = NULL;

			addListNodePtr(&expected_change_head->col_list_head, &expected_change_head->col_list_tail, getTablesHead()->table_cols_head, PTR_TYPE_TABLE_COLS_INFO, ADDLISTNODE_TAIL
						  ,NULL, &malloced_head, the_debug);
			addListNodePtr(&expected_change_head->col_list_head, &expected_change_head->col_list_tail, getTablesHead()->table_cols_head->next->next, PTR_TYPE_TABLE_COLS_INFO, ADDLISTNODE_TAIL
						  ,NULL, &malloced_head, the_debug);

			str1 = (char*) myMalloc(sizeof(char) * 32, NULL, &malloced_head, the_debug);
			strcpy(str1, "Hi");
			addListNodePtr(&expected_change_head->data_list_head, &expected_change_head->data_list_tail, str1, PTR_TYPE_CHAR, ADDLISTNODE_TAIL
						  ,NULL, &malloced_head, the_debug);
			str2 = (char*) myMalloc(sizeof(char) * 32, NULL, &malloced_head, the_debug);
			strcpy(str2, "Hello");
			addListNodePtr(&expected_change_head->data_list_head, &expected_change_head->data_list_tail, str2, PTR_TYPE_CHAR, ADDLISTNODE_TAIL
						  ,NULL, &malloced_head, the_debug);


			if (test_Controller_parseInsert(402, "Insert into alc_brands (braND-name, STATUS) values ('Hi', 'Hello');"
										   ,&expected_change_head, &parsed_error_code, &malloced_head, the_debug) != 0)
				return -1;

			if (parsed_error_code == 0)
			{
				freeAnyLinkedList((void**) &expected_change_head, PTR_TYPE_CHANGE_NODE_V2, NULL, &malloced_head, the_debug);
			}

			if (malloced_head != NULL)
			{
				if (the_debug == YES_DEBUG)
				{
					setOutputRed();
					printf("	ERROR in test_Driver_main() at line %d in %s\n", __LINE__, __FILE__);
					setOutputWhite();
				}
				return -3;
			}
		// END Test with id = 402

		// START Test with id = 403
			expected_change_head = NULL;


			if (test_Controller_parseInsert(403, "Insert into alc_brands (braND-name, BRAND-NAME, STATUS) values ('Hi', 'Hello', 'There');"
										   ,&expected_change_head, &parsed_error_code, &malloced_head, the_debug) != 0)
				return -1;

			if (malloced_head != NULL)
			{
				if (the_debug == YES_DEBUG)
				{
					setOutputRed();
					printf("	ERROR in test_Driver_main() at line %d in %s\n", __LINE__, __FILE__);
					setOutputWhite();
				}
				return -3;
			}
		// END Test with id = 403

		// START Test with id = 404
			expected_change_head = NULL;


			if (test_Controller_parseInsert(404, "Insert into alc_brands (braND-name BRAND-NAME, STATUS) values ('Hi', 'Hello');"
										   ,&expected_change_head, &parsed_error_code, &malloced_head, the_debug) != 0)
				return -1;

			if (malloced_head != NULL)
			{
				if (the_debug == YES_DEBUG)
				{
					setOutputRed();
					printf("	ERROR in test_Driver_main() at line %d in %s\n", __LINE__, __FILE__);
					setOutputWhite();
				}
				return -3;
			}
		// END Test with id = 404

		// START Test with id = 405
			expected_change_head = NULL;


			if (test_Controller_parseInsert(405, "Insert into alc_brands (braND-name, STATUS) values ('Hi', 'Hello', 'There');"
										   ,&expected_change_head, &parsed_error_code, &malloced_head, the_debug) != 0)
				return -1;

			if (malloced_head != NULL)
			{
				if (the_debug == YES_DEBUG)
				{
					setOutputRed();
					printf("	ERROR in test_Driver_main() at line %d in %s\n", __LINE__, __FILE__);
					setOutputWhite();
				}
				return -3;
			}
		// END Test with id = 405

		// START Test with id = 406
			expected_change_head = (struct change_node_v2*) myMalloc(sizeof(struct change_node_v2), NULL, &malloced_head, the_debug);
			expected_change_head->table = getTablesHead();
			expected_change_head->operation = OP_INSERT;
			expected_change_head->col_list_head = NULL;
			expected_change_head->col_list_tail = NULL;
			expected_change_head->data_list_head = NULL;
			expected_change_head->data_list_tail = NULL;
			expected_change_head->where_head = NULL;

			addListNodePtr(&expected_change_head->col_list_head, &expected_change_head->col_list_tail, getTablesHead()->table_cols_head, PTR_TYPE_TABLE_COLS_INFO, ADDLISTNODE_TAIL
						  ,NULL, &malloced_head, the_debug);
			addListNodePtr(&expected_change_head->col_list_head, &expected_change_head->col_list_tail, getTablesHead()->table_cols_head->next, PTR_TYPE_TABLE_COLS_INFO, ADDLISTNODE_TAIL
						  ,NULL, &malloced_head, the_debug);
			addListNodePtr(&expected_change_head->col_list_head, &expected_change_head->col_list_tail, getTablesHead()->table_cols_head->next->next, PTR_TYPE_TABLE_COLS_INFO, ADDLISTNODE_TAIL
						  ,NULL, &malloced_head, the_debug);
			addListNodePtr(&expected_change_head->col_list_head, &expected_change_head->col_list_tail, getTablesHead()->table_cols_head->next->next->next, PTR_TYPE_TABLE_COLS_INFO, ADDLISTNODE_TAIL
						  ,NULL, &malloced_head, the_debug);
			addListNodePtr(&expected_change_head->col_list_head, &expected_change_head->col_list_tail, getTablesHead()->table_cols_head->next->next->next->next, PTR_TYPE_TABLE_COLS_INFO, ADDLISTNODE_TAIL
						  ,NULL, &malloced_head, the_debug);
			addListNodePtr(&expected_change_head->col_list_head, &expected_change_head->col_list_tail, getTablesHead()->table_cols_head->next->next->next->next->next, PTR_TYPE_TABLE_COLS_INFO, ADDLISTNODE_TAIL
						  ,NULL, &malloced_head, the_debug);
			addListNodePtr(&expected_change_head->col_list_head, &expected_change_head->col_list_tail, getTablesHead()->table_cols_head->next->next->next->next->next->next, PTR_TYPE_TABLE_COLS_INFO, ADDLISTNODE_TAIL
						  ,NULL, &malloced_head, the_debug);

			str1 = (char*) myMalloc(sizeof(char) * 32, NULL, &malloced_head, the_debug);
			strcpy(str1, "1");
			addListNodePtr(&expected_change_head->data_list_head, &expected_change_head->data_list_tail, str1, PTR_TYPE_CHAR, ADDLISTNODE_TAIL
						  ,NULL, &malloced_head, the_debug);
			int1 = (int*) myMalloc(sizeof(int), NULL, &malloced_head, the_debug);
			*int1 = 1;
			addListNodePtr(&expected_change_head->data_list_head, &expected_change_head->data_list_tail, int1, PTR_TYPE_INT, ADDLISTNODE_TAIL
						  ,NULL, &malloced_head, the_debug);
			str2 = (char*) myMalloc(sizeof(char) * 32, NULL, &malloced_head, the_debug);
			strcpy(str2, "1");
			addListNodePtr(&expected_change_head->data_list_head, &expected_change_head->data_list_tail, str2, PTR_TYPE_CHAR, ADDLISTNODE_TAIL
						  ,NULL, &malloced_head, the_debug);
			str3 = (char*) myMalloc(sizeof(char) * 32, NULL, &malloced_head, the_debug);
			strcpy(str3, "1/1/1900");
			addListNodePtr(&expected_change_head->data_list_head, &expected_change_head->data_list_tail, str3, PTR_TYPE_DATE, ADDLISTNODE_TAIL
						  ,NULL, &malloced_head, the_debug);
			char* str4 = (char*) myMalloc(sizeof(char) * 32, NULL, &malloced_head, the_debug);
			strcpy(str4, "1/1/1900");
			addListNodePtr(&expected_change_head->data_list_head, &expected_change_head->data_list_tail, str4, PTR_TYPE_DATE, ADDLISTNODE_TAIL
						  ,NULL, &malloced_head, the_debug);
			char* str5 = (char*) myMalloc(sizeof(char) * 32, NULL, &malloced_head, the_debug);
			strcpy(str5, "1");
			addListNodePtr(&expected_change_head->data_list_head, &expected_change_head->data_list_tail, str5, PTR_TYPE_CHAR, ADDLISTNODE_TAIL
						  ,NULL, &malloced_head, the_debug);
			char* str6 = (char*) myMalloc(sizeof(char) * 32, NULL, &malloced_head, the_debug);
			strcpy(str6, "1");
			addListNodePtr(&expected_change_head->data_list_head, &expected_change_head->data_list_tail, str6, PTR_TYPE_CHAR, ADDLISTNODE_TAIL
						  ,NULL, &malloced_head, the_debug);
			

			if (test_Controller_parseInsert(406, "Insert into alc_brands (BRAND-NAME,CT-REGISTRATION-NUMBER,STATUS,EFFECTIVE,EXPIRATION,OUT-OF-STATE-SHIPPER,SUPERVISOR-CREDENTIAL) values ('1', 1, '1','1/1/1900','1/1/1900','1','1');"
										   ,&expected_change_head, &parsed_error_code, &malloced_head, the_debug) != 0)
				return -1;

			if (parsed_error_code == 0)
			{
				freeAnyLinkedList((void**) &expected_change_head, PTR_TYPE_CHANGE_NODE_V2, NULL, &malloced_head, the_debug);
			}

			if (malloced_head != NULL)
			{
				if (the_debug == YES_DEBUG)
				{
					setOutputRed();
					printf("	ERROR in test_Driver_main() at line %d in %s\n", __LINE__, __FILE__);
					setOutputWhite();
				}
				return -3;
			}
		// END Test with id = 406

		// START Test with id = 407
			expected_change_head = NULL;

			
			if (test_Controller_parseInsert(407, "Insert into alc_brands (BRAND-NAME, CT-REGISTRATION-NUMBER) values ('Hi', 'Hello');"
										   ,&expected_change_head, &parsed_error_code, &malloced_head, the_debug) != 0)
				return -1;

			if (malloced_head != NULL)
			{
				if (the_debug == YES_DEBUG)
				{
					setOutputRed();
					printf("	ERROR in test_Driver_main() at line %d in %s\n", __LINE__, __FILE__);
					setOutputWhite();
				}
				return -3;
			}
		// END Test with id = 407

		// START Test with id = 408
			expected_change_head = NULL;

			
			if (test_Controller_parseInsert(408, "Insert into alc_brands (BRAND-NAME, CT-REGISTRATION-NUMBER) values (1, 1);"
										   ,&expected_change_head, &parsed_error_code, &malloced_head, the_debug) != 0)
				return -1;

			if (malloced_head != NULL)
			{
				if (the_debug == YES_DEBUG)
				{
					setOutputRed();
					printf("	ERROR in test_Driver_main() at line %d in %s\n", __LINE__, __FILE__);
					setOutputWhite();
				}
				return -3;
			}
		// END Test with id = 408

		// START Test with id = 409
			expected_change_head = NULL;

			
			if (test_Controller_parseInsert(409, "Insert into alc_brands (BRAND-NAME, EFFECTIVE) values ('Hi', 1);"
										   ,&expected_change_head, &parsed_error_code, &malloced_head, the_debug) != 0)
				return -1;

			if (malloced_head != NULL)
			{
				if (the_debug == YES_DEBUG)
				{
					setOutputRed();
					printf("	ERROR in test_Driver_main() at line %d in %s\n", __LINE__, __FILE__);
					setOutputWhite();
				}
				return -3;
			}
		// END Test with id = 409

		// START Test with id = 410
			expected_change_head = (struct change_node_v2*) myMalloc(sizeof(struct change_node_v2), NULL, &malloced_head, the_debug);
			expected_change_head->table = getTablesHead();
			expected_change_head->operation = OP_INSERT;
			expected_change_head->col_list_head = NULL;
			expected_change_head->col_list_tail = NULL;
			expected_change_head->data_list_head = NULL;
			expected_change_head->data_list_tail = NULL;
			expected_change_head->where_head = NULL;

			addListNodePtr(&expected_change_head->col_list_head, &expected_change_head->col_list_tail, getTablesHead()->table_cols_head, PTR_TYPE_TABLE_COLS_INFO, ADDLISTNODE_TAIL
						  ,NULL, &malloced_head, the_debug);
			addListNodePtr(&expected_change_head->col_list_head, &expected_change_head->col_list_tail, getTablesHead()->table_cols_head->next, PTR_TYPE_TABLE_COLS_INFO, ADDLISTNODE_TAIL
						  ,NULL, &malloced_head, the_debug);
			addListNodePtr(&expected_change_head->col_list_head, &expected_change_head->col_list_tail, getTablesHead()->table_cols_head->next->next, PTR_TYPE_TABLE_COLS_INFO, ADDLISTNODE_TAIL
						  ,NULL, &malloced_head, the_debug);
			addListNodePtr(&expected_change_head->col_list_head, &expected_change_head->col_list_tail, getTablesHead()->table_cols_head->next->next->next, PTR_TYPE_TABLE_COLS_INFO, ADDLISTNODE_TAIL
						  ,NULL, &malloced_head, the_debug);
			addListNodePtr(&expected_change_head->col_list_head, &expected_change_head->col_list_tail, getTablesHead()->table_cols_head->next->next->next->next, PTR_TYPE_TABLE_COLS_INFO, ADDLISTNODE_TAIL
						  ,NULL, &malloced_head, the_debug);
			addListNodePtr(&expected_change_head->col_list_head, &expected_change_head->col_list_tail, getTablesHead()->table_cols_head->next->next->next->next->next, PTR_TYPE_TABLE_COLS_INFO, ADDLISTNODE_TAIL
						  ,NULL, &malloced_head, the_debug);
			addListNodePtr(&expected_change_head->col_list_head, &expected_change_head->col_list_tail, getTablesHead()->table_cols_head->next->next->next->next->next->next, PTR_TYPE_TABLE_COLS_INFO, ADDLISTNODE_TAIL
						  ,NULL, &malloced_head, the_debug);
			addListNodePtr(&expected_change_head->col_list_head, &expected_change_head->col_list_tail, getTablesHead()->table_cols_head, PTR_TYPE_TABLE_COLS_INFO, ADDLISTNODE_TAIL
						  ,NULL, &malloced_head, the_debug);
			addListNodePtr(&expected_change_head->col_list_head, &expected_change_head->col_list_tail, getTablesHead()->table_cols_head->next, PTR_TYPE_TABLE_COLS_INFO, ADDLISTNODE_TAIL
						  ,NULL, &malloced_head, the_debug);
			addListNodePtr(&expected_change_head->col_list_head, &expected_change_head->col_list_tail, getTablesHead()->table_cols_head->next->next, PTR_TYPE_TABLE_COLS_INFO, ADDLISTNODE_TAIL
						  ,NULL, &malloced_head, the_debug);
			addListNodePtr(&expected_change_head->col_list_head, &expected_change_head->col_list_tail, getTablesHead()->table_cols_head->next->next->next, PTR_TYPE_TABLE_COLS_INFO, ADDLISTNODE_TAIL
						  ,NULL, &malloced_head, the_debug);
			addListNodePtr(&expected_change_head->col_list_head, &expected_change_head->col_list_tail, getTablesHead()->table_cols_head->next->next->next->next, PTR_TYPE_TABLE_COLS_INFO, ADDLISTNODE_TAIL
						  ,NULL, &malloced_head, the_debug);
			addListNodePtr(&expected_change_head->col_list_head, &expected_change_head->col_list_tail, getTablesHead()->table_cols_head->next->next->next->next->next, PTR_TYPE_TABLE_COLS_INFO, ADDLISTNODE_TAIL
						  ,NULL, &malloced_head, the_debug);
			addListNodePtr(&expected_change_head->col_list_head, &expected_change_head->col_list_tail, getTablesHead()->table_cols_head->next->next->next->next->next->next, PTR_TYPE_TABLE_COLS_INFO, ADDLISTNODE_TAIL
						  ,NULL, &malloced_head, the_debug);

			str1 = (char*) myMalloc(sizeof(char) * 32, NULL, &malloced_head, the_debug);
			strcpy(str1, "1");
			addListNodePtr(&expected_change_head->data_list_head, &expected_change_head->data_list_tail, str1, PTR_TYPE_CHAR, ADDLISTNODE_TAIL
						  ,NULL, &malloced_head, the_debug);
			int1 = (int*) myMalloc(sizeof(int), NULL, &malloced_head, the_debug);
			*int1 = 1;
			addListNodePtr(&expected_change_head->data_list_head, &expected_change_head->data_list_tail, int1, PTR_TYPE_INT, ADDLISTNODE_TAIL
						  ,NULL, &malloced_head, the_debug);
			str2 = (char*) myMalloc(sizeof(char) * 32, NULL, &malloced_head, the_debug);
			strcpy(str2, "1");
			addListNodePtr(&expected_change_head->data_list_head, &expected_change_head->data_list_tail, str2, PTR_TYPE_CHAR, ADDLISTNODE_TAIL
						  ,NULL, &malloced_head, the_debug);
			str3 = (char*) myMalloc(sizeof(char) * 32, NULL, &malloced_head, the_debug);
			strcpy(str3, "1/1/1900");
			addListNodePtr(&expected_change_head->data_list_head, &expected_change_head->data_list_tail, str3, PTR_TYPE_DATE, ADDLISTNODE_TAIL
						  ,NULL, &malloced_head, the_debug);
			str4 = (char*) myMalloc(sizeof(char) * 32, NULL, &malloced_head, the_debug);
			strcpy(str4, "1/1/1900");
			addListNodePtr(&expected_change_head->data_list_head, &expected_change_head->data_list_tail, str4, PTR_TYPE_DATE, ADDLISTNODE_TAIL
						  ,NULL, &malloced_head, the_debug);
			str5 = (char*) myMalloc(sizeof(char) * 32, NULL, &malloced_head, the_debug);
			strcpy(str5, "1");
			addListNodePtr(&expected_change_head->data_list_head, &expected_change_head->data_list_tail, str5, PTR_TYPE_CHAR, ADDLISTNODE_TAIL
						  ,NULL, &malloced_head, the_debug);
			str6 = (char*) myMalloc(sizeof(char) * 32, NULL, &malloced_head, the_debug);
			strcpy(str6, "1");
			addListNodePtr(&expected_change_head->data_list_head, &expected_change_head->data_list_tail, str6, PTR_TYPE_CHAR, ADDLISTNODE_TAIL
						  ,NULL, &malloced_head, the_debug);
			char* str7 = (char*) myMalloc(sizeof(char) * 32, NULL, &malloced_head, the_debug);
			strcpy(str7, "1");
			addListNodePtr(&expected_change_head->data_list_head, &expected_change_head->data_list_tail, str7, PTR_TYPE_CHAR, ADDLISTNODE_TAIL
						  ,NULL, &malloced_head, the_debug);
			int2 = (int*) myMalloc(sizeof(int), NULL, &malloced_head, the_debug);
			*int2 = 1;
			addListNodePtr(&expected_change_head->data_list_head, &expected_change_head->data_list_tail, int2, PTR_TYPE_INT, ADDLISTNODE_TAIL
						  ,NULL, &malloced_head, the_debug);
			char* str8 = (char*) myMalloc(sizeof(char) * 32, NULL, &malloced_head, the_debug);
			strcpy(str8, "1");
			addListNodePtr(&expected_change_head->data_list_head, &expected_change_head->data_list_tail, str8, PTR_TYPE_CHAR, ADDLISTNODE_TAIL
						  ,NULL, &malloced_head, the_debug);
			char* str9 = (char*) myMalloc(sizeof(char) * 32, NULL, &malloced_head, the_debug);
			strcpy(str9, "1/1/1900");
			addListNodePtr(&expected_change_head->data_list_head, &expected_change_head->data_list_tail, str9, PTR_TYPE_DATE, ADDLISTNODE_TAIL
						  ,NULL, &malloced_head, the_debug);
			char* str10 = (char*) myMalloc(sizeof(char) * 32, NULL, &malloced_head, the_debug);
			strcpy(str10, "1/1/1900");
			addListNodePtr(&expected_change_head->data_list_head, &expected_change_head->data_list_tail, str10, PTR_TYPE_DATE, ADDLISTNODE_TAIL
						  ,NULL, &malloced_head, the_debug);
			char* str11 = (char*) myMalloc(sizeof(char) * 32, NULL, &malloced_head, the_debug);
			strcpy(str11, "1");
			addListNodePtr(&expected_change_head->data_list_head, &expected_change_head->data_list_tail, str11, PTR_TYPE_CHAR, ADDLISTNODE_TAIL
						  ,NULL, &malloced_head, the_debug);
			char* str12 = (char*) myMalloc(sizeof(char) * 32, NULL, &malloced_head, the_debug);
			strcpy(str12, "1");
			addListNodePtr(&expected_change_head->data_list_head, &expected_change_head->data_list_tail, str12, PTR_TYPE_CHAR, ADDLISTNODE_TAIL
						  ,NULL, &malloced_head, the_debug);
			

			if (test_Controller_parseInsert(410, "Insert into alc_brands (BRAND-NAME,CT-REGISTRATION-NUMBER,STATUS,EFFECTIVE,EXPIRATION,OUT-OF-STATE-SHIPPER,SUPERVISOR-CREDENTIAL) values ('1', 1, '1','1/1/1900','1/1/1900','1','1'), ('1', 1, '1','1/1/1900','1/1/1900','1','1');"
										   ,&expected_change_head, &parsed_error_code, &malloced_head, the_debug) != 0)
				return -1;

			if (parsed_error_code == 0)
			{
				freeAnyLinkedList((void**) &expected_change_head, PTR_TYPE_CHANGE_NODE_V2, NULL, &malloced_head, the_debug);
			}

			if (malloced_head != NULL)
			{
				if (the_debug == YES_DEBUG)
				{
					setOutputRed();
					printf("	ERROR in test_Driver_main() at line %d in %s\n", __LINE__, __FILE__);
					setOutputWhite();
				}
				return -3;
			}
		// END Test with id = 410
	// END test_Controller_parseInsert


	/*// START test_Driver_findValidRowsGivenWhere
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