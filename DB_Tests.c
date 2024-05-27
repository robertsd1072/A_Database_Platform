#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stddef.h>
#include <inttypes.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <time.h>
#include "DB_Tests.h"
#include "DB_Driver.h"
#include "DB_HelperFunctions.h"
#include "DB_Controller.h"


int setOutputRed()
{
	// WINDOWS
	//HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
	//SetConsoleTextAttribute(hConsole, 4);

	// LINUX
	printf("\033[31m");
	return 0;
}

int setOutputGreen()
{
	// WINDOWS
	//HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
	//SetConsoleTextAttribute(hConsole, 10);

	// LINUX
	printf("\033[32m");

	return 0;
}

int setOutputWhite()
{
	//HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
	//SetConsoleTextAttribute(hConsole, 15);

	// LINUX
	printf("\033[0m");
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
	strcpy(table_name, "LIQUOR_LICENSES");
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

		if (selectAndCheckHash("DB_Files_2_Test_Versions/CreatedTable.csv", -1, malloced_head, the_debug) != 0)
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

    //system("del DB_Files_2/*");

    return 0;
}

int selectAndCheckHash(char* test_version, int test_id, struct table_info* table, struct malloced_node** malloced_head, int the_debug)
{
	char select_string[100] = {0};
	strcpy(select_string, "select * from ");
	strcat(select_string, table->name);
	strcat(select_string, ";");

	//printf("select_string = _%s_\n", select_string);

	struct select_node* select_node = NULL;
	parseSelect(select_string, &select_node, NULL, false, malloced_head, the_debug);


	if (selectStuff(&select_node, false, malloced_head, the_debug) != 0)
	{
		if (the_debug == YES_DEBUG)
			printf("Data retreival from disk FAILED, please try again\n");
		return -1;
	}
	else
	{
		if (the_debug == YES_DEBUG)
			printf("Successfully retreived %lu rows from disk, creating .csv file\n", select_node->columns_arr[0]->num_rows);
		int displayed = displayResultsOfSelect(select_node, malloced_head, the_debug);

		// START Free stuff
			/**/
			int_8 total_freed = 0;
			for (int j=0; j<select_node->columns_arr_size; j++)
			{
				for (int i=0; i<select_node->columns_arr[j]->num_rows; i++)
				{
					myFree((void**) &select_node->columns_arr[j]->col_data_arr[i], NULL, malloced_head, the_debug);
					total_freed++;
				}
				myFree((void**) &select_node->columns_arr[j]->col_data_arr, NULL, malloced_head, the_debug);
				total_freed++;

				if (select_node->columns_arr[j]->unique_values_head != NULL)
				{
					int temp_freed = freeAnyLinkedList((void**) &select_node->columns_arr[j]->unique_values_head, PTR_TYPE_LIST_NODE_PTR, NULL, malloced_head, the_debug);
					if (the_debug == YES_DEBUG)
						printf("Freed %d from col's unique_values_head\n", temp_freed);
					total_freed += temp_freed;
				}
			}

			while (select_node->prev != NULL)
				select_node = select_node->prev;

			freeAnyLinkedList((void**) &select_node, PTR_TYPE_SELECT_NODE, NULL, malloced_head, the_debug);
			//myFreeAllError(malloced_head, the_debug);
		// END Free stuff

		if (displayed != 0)
		{
			printf("Creation of .csv file had a problem with file i/o, please try again.\n");
			return -1;
		}

		char cmd[200];

		/* WINDOWS
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
			if (fgets(var1, 65, fp) != NULL)
			{
				//if (i == 1)
					strcpy(hash1, var1);
				printf("%s\n", var1);
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
			if (fgets(var2, 65, fp) != NULL)
			{
				//if (i == 1)
					strcpy(hash2, var2);
				printf("%s\n", var2);
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
				setOutputRed();
				printf("File hashes DID NOT match for test %d\n", test_id);
				printf("hash1 = %s\n", hash1);
				printf("hash2 = %s\n", hash2);
				setOutputWhite();
			}

			myFree((void**) &var1, NULL, malloced_head, the_debug);
			myFree((void**) &var2, NULL, malloced_head, the_debug);
			myFree((void**) &hash1, NULL, malloced_head, the_debug);
			yFree((void**) &hash2, NULL, malloced_head, the_debug);
		*/

		/* LINUX*/
			strcpy(cmd, "diff -w Results.csv ");
			strcat(cmd, test_version);

			//printf("cmd = _%s_\n", cmd);

			int compared = 0;

			FILE *fp = popen(cmd, "r");
			if (fp == NULL) 
			{
				printf("Failed to run command\n");
				return -1;
			}

			char output[100];
			if (fgets(output, 65, fp) != NULL)
			{
				//printf("output = _%s_\n", output);
				if (output[0] != 0)
				{
					setOutputRed();
					printf("Files DID NOT match for test %d\n", test_id);
					setOutputWhite();
					compared = -1;
				}
			}

			pclose(fp);
		

		if (compared != 0)
			return -1;
	}

	return 0;
}


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

			new_temp->columns_arr[i]->rows_data_type = cur_col->data_type;

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

			new_temp->columns_arr[i]->rows_data_type = ((struct select_node*) table)->columns_arr[i]->rows_data_type;
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

int compCaseNodes(int test_id, struct case_node* actual_case, struct case_node* expected_case)
{
	// START Check column case node
		if (actual_case == NULL && expected_case != NULL)
		{
			printf("compCaseNodes with id = %d FAILED\n", test_id);
			printf("actual_case was NULL and expected_case was NOT NULL\n");
			return -1;
		}
		else if (actual_case != NULL && expected_case == NULL)
		{
			printf("compCaseNodes with id = %d FAILED\n", test_id);
			printf("actual_case was NOT NULL and expected_case was NULL\n");
			return -1;
		}
		else if (actual_case != NULL && expected_case != NULL)
		{
			// START Check case_node case_when_head
				struct ListNodePtr* actual_cur_when = actual_case->case_when_head;
				struct ListNodePtr* expected_cur_when = expected_case->case_when_head;
				while (actual_cur_when != NULL || expected_cur_when != NULL)
				{
					if (actual_cur_when == NULL && expected_cur_when != NULL)
					{
						printf("compCaseNodes with id = %d FAILED\n", test_id);
						printf("actual_cur_when was NULL and expected_cur_when was NOT NULL\n");
						return -1;
					}
					else if (actual_cur_when != NULL && expected_cur_when == NULL)
					{
						printf("compCaseNodes with id = %d FAILED\n", test_id);
						printf("actual_cur_when was NOT NULL and expected_cur_when was NULL\n");
						return -1;
					}
					else if (actual_cur_when != NULL && expected_cur_when != NULL)
					{
						if (actual_cur_when->ptr_value == NULL && expected_cur_when->ptr_value != NULL)
						{
							printf("compCaseNodes with id = %d FAILED\n", test_id);
							printf("actual_cur_when->ptr_value was NULL and expected_cur_when->ptr_value was NOT NULL\n");
							return -1;
						}
						else if (actual_cur_when->ptr_value != NULL && expected_cur_when->ptr_value == NULL)
						{
							printf("compCaseNodes with id = %d FAILED\n", test_id);
							printf("actual_cur_when->ptr_value was NOT NULL and expected_cur_when->ptr_value was NULL\n");
							return -1;
						}
						else if (actual_cur_when->ptr_value != NULL && expected_cur_when->ptr_value != NULL)
						{
							if (actual_cur_when->ptr_type != expected_cur_when->ptr_type)
							{
								printf("compCaseNodes with id = %d FAILED\n", test_id);
								printf("actual_cur_when->ptr_type (%d) did not equal expected_cur_when->ptr_type (%d)\n", actual_cur_when->ptr_type, expected_cur_when->ptr_type);
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
				struct ListNodePtr* actual_cur_then = actual_case->case_then_value_head;
				struct ListNodePtr* expected_cur_then = expected_case->case_then_value_head;
				while (actual_cur_then != NULL || expected_cur_then != NULL)
				{
					if (actual_cur_then == NULL && expected_cur_then != NULL)
					{
						printf("compCaseNodes with id = %d FAILED\n", test_id);
						printf("actual_cur_then was NULL and expected_cur_then was NOT NULL\n");
						return -1;
					}
					else if (actual_cur_then != NULL && expected_cur_then == NULL)
					{
						printf("compCaseNodes with id = %d FAILED\n", test_id);
						printf("actual_cur_then was NOT NULL and expected_cur_then was NULL\n");
						return -1;
					}
					else if (actual_cur_then != NULL && expected_cur_then != NULL)
					{
						if (actual_cur_then->ptr_value == NULL && expected_cur_then->ptr_value != NULL)
						{
							printf("compCaseNodes with id = %d FAILED\n", test_id);
							printf("actual_cur_then->ptr_value was NULL and expected_cur_then->ptr_value was NOT NULL\n");
							return -1;
						}
						else if (actual_cur_then->ptr_value != NULL && expected_cur_then->ptr_value == NULL)
						{
							printf("compCaseNodes with id = %d FAILED\n", test_id);
							printf("actual_cur_then->ptr_value was NOT NULL and expected_cur_then->ptr_value was NULL\n");
							return -1;
						}
						else if (actual_cur_then->ptr_value != NULL && expected_cur_then->ptr_value != NULL)
						{
							if (actual_cur_then->ptr_type != expected_cur_then->ptr_type)
							{
								printf("compCaseNodes with id = %d FAILED\n", test_id);
								printf("actual_cur_then->ptr_type (%d) did not equal expected_cur_then->ptr_type (%d)\n", actual_cur_then->ptr_type, expected_cur_then->ptr_type);
								return -1;
							}

							if (actual_cur_then->ptr_type == PTR_TYPE_INT)
							{
								if (*((int*) actual_cur_then->ptr_value) != *((int*) expected_cur_then->ptr_value))
								{
									printf("compCaseNodes with id = %d FAILED\n", test_id);
									printf("actual_cur_then->ptr_value (%d) did not equal expected_cur_then->ptr_value (%d)\n", *((int*) actual_cur_then->ptr_value), *((int*) expected_cur_then->ptr_value));
									return -1;
								}
							}
							else if (actual_cur_then->ptr_type == PTR_TYPE_REAL)
							{
								if (*((double*) actual_cur_then->ptr_value) != *((double*) expected_cur_then->ptr_value))
								{
									printf("compCaseNodes with id = %d FAILED\n", test_id);
									printf("actual_cur_then->ptr_value (%lf) did not equal expected_cur_then->ptr_value (%lf)\n", *((double*) actual_cur_then->ptr_value), *((double*) expected_cur_then->ptr_value));
									return -1;
								}
							}
							else if (actual_cur_then->ptr_type == PTR_TYPE_CHAR || actual_cur_then->ptr_type == PTR_TYPE_DATE)
							{
								if (strcmp(actual_cur_then->ptr_value, expected_cur_then->ptr_value) != 0)
								{
									printf("compCaseNodes with id = %d FAILED\n", test_id);
									printf("actual_cur_then->ptr_value (%s) did not equal expected_cur_then->ptr_value (%s)\n", actual_cur_then->ptr_value, expected_cur_then->ptr_value);
									return -1;
								}
							}
							else if (actual_cur_then->ptr_type == PTR_TYPE_MATH_NODE)
							{
								printf("Calling compMathOrWhereTree()\n");
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
		if (actual_func->args_arr_type[j] != expected_func->args_arr_type[j])
		{
			printf("test_Controller_parseSelect with id = %d FAILED\n", test_id);
			printf("Actual actual_func->args_arr_type[j] (%d) did not equal below\n", actual_func->args_arr_type[j]);
			printf("Expected expected_func->args_arr_type[j] (%d)\n", expected_func->args_arr_type[j]);
			return -1;
		}

		if (actual_func->args_arr_type[j] == PTR_TYPE_CHAR)
		{
			if (strcmp(actual_func->args_arr[j], expected_func->args_arr[j]) != 0)
			{
				printf("test_Controller_parseSelect with id = %d FAILED\n", test_id);
				printf("Actual actual_func->args_arr[j] (%s) did not equal below\n", actual_func->args_arr[j]);
				printf("Expected expected_func->args_arr[j] (%s)\n", expected_func->args_arr[j]);
				return -1;
			}
		}
		else if (actual_func->args_arr_type[j] == PTR_TYPE_FUNC_NODE)
		{
			if (compFuncNodes(test_id, actual_func->args_arr[j], expected_func->args_arr[j]) != 0)
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

	if (actual_func->result_type != expected_func->result_type)
	{
		printf("test_Controller_parseSelect with id = %d FAILED\n", test_id);
		printf("Actual actual_func->result_type (%d) did not equal below\n", actual_func->result_type);
		printf("Expected expected_func->result_type (%d)\n", expected_func->result_type);
		return -1;
	}

	return 0;
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

		if (((struct math_node*) actual_ptr)->ptr_one_type != 
			((struct math_node*) expected_ptr)->ptr_one_type)
		{
			printf("compMathOrWhereTree with id = %d FAILED\n", test_id);
			printf("Actual math_node ptr_one_type %d did not equal below\n", ((struct math_node*) actual_ptr)->ptr_one_type);
			printf("Expected math_node ptr_one_type %d\n", ((struct math_node*) expected_ptr)->ptr_one_type);
			return -1;
		}

		if (((struct math_node*) actual_ptr)->ptr_two_type != 
			((struct math_node*) expected_ptr)->ptr_two_type)
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
			else if (ptr_type == PTR_TYPE_FUNC_NODE)
			{
				if (compFuncNodes(test_id, act_ptr, exp_ptr) != 0)
				{
					printf("Returned from compFuncNodes()\n");
					return -1;
				}
			}
			else if (ptr_type == PTR_TYPE_CASE_NODE)
			{
				if (compCaseNodes(test_id, act_ptr, exp_ptr) != 0)
				{
					printf("Returned from compCaseNodes()\n");
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

		if (((struct math_node*) actual_ptr)->result_type != ((struct math_node*) expected_ptr)->result_type)
		{
			printf("compMathOrWhereTree with id = %d FAILED\n", test_id);
			printf("Actual math_node result_type %d did not equal below\n", ((struct math_node*) actual_ptr)->result_type);
			printf("Expected math_node result_type %d\n", ((struct math_node*) expected_ptr)->result_type);
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
					printf("Returned from compMathOrWhereTree() for WHERE node at node #%d\n", i+1);
					return -1;
				}
			}
			else if (ptr_type == PTR_TYPE_MATH_NODE)
			{
				if (compMathOrWhereTree(test_id, PTR_TYPE_MATH_NODE, act_ptr, exp_ptr) != 0)
				{
					printf("Returned from compMathOrWhereTree() for MATH node at node #%d\n", i+1);
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
			else if (ptr_type == PTR_TYPE_CASE_NODE)
			{
				if (compCaseNodes(test_id, act_ptr, exp_ptr) != 0)
				{
					printf("Returned from compCaseNodes()\n");
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
				if (((struct col_in_select_node*) act_ptr)->col_ptr != NULL && ((struct col_in_select_node*) exp_ptr)->col_ptr != NULL)
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

					if (act_ptr != exp_ptr)
					{
						printf("compMathOrWhereTree with id = %d FAILED\n", test_id);
						printf("Actual where_clause_node ptr (%s) did not equal below for node #%d\n", ((struct table_cols_info*) act_ptr)->col_name, i+1);
						printf("Expected where_clause_node ptr (%s) for node #%d\n", ((struct table_cols_info*) exp_ptr)->col_name, i+1);
						return -1;
					}
				}
				else if (((struct col_in_select_node*) act_ptr)->func_node != NULL && ((struct col_in_select_node*) exp_ptr)->func_node != NULL)
				{
					if (compFuncNodes(test_id, ((struct col_in_select_node*) act_ptr)->func_node, ((struct col_in_select_node*) exp_ptr)->func_node) != 0)
					{
						return -1;
					}
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
		//freeAnyLinkedList((void**) &where_head, PTR_TYPE_WHERE_CLAUSE_NODE, NULL, malloced_head, the_debug);
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

		if ((*expected_change_head)->total_rows_to_insert != change_head->total_rows_to_insert)
		{
			printf("test_Controller_parseUpdate with id = %d FAILED\n", test_id);
			printf("Actual total_rows_to_insert (%lu) did not equal expected total_rows_to_insert (%lu)\n", change_head->total_rows_to_insert, (*expected_change_head)->total_rows_to_insert);
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

		if ((*expected_change_head)->total_rows_to_insert != change_head->total_rows_to_insert)
		{
			printf("test_Controller_parseDelete with id = %d FAILED\n", test_id);
			printf("Actual total_rows_to_insert (%lu) did not equal expected total_rows_to_insert (%lu)\n", change_head->total_rows_to_insert, (*expected_change_head)->total_rows_to_insert);
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

		if ((*expected_change_head)->total_rows_to_insert != change_head->total_rows_to_insert)
		{
			printf("test_Controller_parseInsert with id = %d FAILED\n", test_id);
			printf("Actual total_rows_to_insert (%lu) did not equal expected total_rows_to_insert (%lu)\n", change_head->total_rows_to_insert, (*expected_change_head)->total_rows_to_insert);
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
		//printf("select_node_counter = %d\n", select_node_counter);
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
						printf("cur actual columns_arr[%d]->new_name = \"%s\" and cur exp columns_arr[%d]->new_name = \"%s\"\n", i, cur_actual_select->columns_arr[i]->new_name, i, cur_expected_select->columns_arr[i]->new_name);
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
						{
							printf("Failed for column at index = %d\n", i);
							return -1;
						}
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
							printf("Failed for column at index = %d\n", i);
							return -1;
						}
					}
				// END Check column math node

				// START Check column case node
					if (compCaseNodes(test_id, cur_actual_select->columns_arr[i]->case_node, cur_expected_select->columns_arr[i]->case_node) != 0)
					{
						printf("test_Controller_parseSelect with id = %d FAILED\n", test_id);
						printf("Comparing cur actual columns_arr[%d]->case_node and cur exp columns_arr[%d]->case_node FAILED\n", i, i);
						return -1;
					}
				// END Check column case node

				if (cur_actual_select->columns_arr[i]->rows_data_type != cur_expected_select->columns_arr[i]->rows_data_type)
				{
					printf("test_Controller_parseSelect with id = %d FAILED\n", test_id);
					printf("Actual column rows_data_type %d did not equal below\n", cur_actual_select->columns_arr[i]->rows_data_type);
					printf("Expected column rows_data_type %d\n", cur_expected_select->columns_arr[i]->rows_data_type);
					printf("Failed for column at index = %d\n", i);
					return -1;
				}
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
			if (cur_actual_select->where_head == NULL && cur_expected_select->where_head != NULL)
			{
				printf("test_Controller_parseSelect with id = %d FAILED\n", test_id);
				printf("cur_actual_select->where_head was NULL and cur_expected_select->where_head was NOT NULL\n");
				return -1;
			}
			else if (cur_actual_select->where_head != NULL && cur_expected_select->where_head == NULL)
			{
				printf("test_Controller_parseSelect with id = %d FAILED\n", test_id);
				printf("cur_actual_select->where_head was NOT NULL and cur_expected_select->where_head was NULL\n");
				return -1;
			}
			else if (cur_actual_select->where_head != NULL && cur_expected_select->where_head != NULL)
			{
				if (compMathOrWhereTree(test_id, PTR_TYPE_WHERE_CLAUSE_NODE, cur_actual_select->where_head, cur_expected_select->where_head) != 0)
				{
					printf("test_Controller_parseSelect with id = %d FAILED\n", test_id);
					printf("Comparing the where nodes FAILED\n");
					return -1;
				}
			}
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


int test_Driver_findValidRowsGivenWhere(int test_id, char* where_string, struct select_node* the_select_node, struct table_info* the_table
									   ,struct ListNodePtr* expected_results, struct malloced_node** malloced_head, int the_debug)
{
	setOutputGreen();
	printf("\nStarting test with id = %d\n", test_id);
	setOutputWhite();


	struct where_clause_node* where_head = NULL;
	if (parseWhereClause(where_string, &where_head, the_select_node, the_table, "where", malloced_head, the_debug) != 0)
	{
		setOutputRed();
		printf("test_Driver_findValidRowsGivenWhere with id = %d FAILED\n", test_id);
		printf("The test %d had a problem with parseWhereClause()\n", test_id);
		setOutputWhite();
		return -1;
	}

	//printf("Parsed where_string\n");

	struct ListNodePtr* actual_results;
	struct ListNodePtr* actual_results_tail;
	int_8 num_rows_in_result = 0;
	if (findValidRowsGivenWhere(&actual_results, &actual_results_tail
								,the_select_node, the_table, where_head
								,&num_rows_in_result, NULL, NULL, false, -1, -1, malloced_head, the_debug) != 0)
	{
		setOutputRed();
		printf("test_Driver_findValidRowsGivenWhere with id = %d FAILED\n", test_id);
		printf("The test %d had a problem with findValidRowsGivenWhere()\n", test_id);
		setOutputWhite();
		return -1;
	}

	if (the_debug == YES_DEBUG)
		printf("findValidRowsGivenWhere returned %lu rows\n", num_rows_in_result);


	freeAnyLinkedList((void**) &where_head, PTR_TYPE_WHERE_CLAUSE_NODE, NULL, malloced_head, the_debug);


	// START Tests
		struct ListNodePtr* cur_expected = expected_results;
		struct ListNodePtr* cur_actual = actual_results;

		while (cur_expected != NULL || cur_actual != NULL)
		{
			//printf("Actual row_id %d\n", *((int*) cur_actual->ptr_value));
			//printf("Expected row_id %d\n", *((int*) cur_expected->ptr_value));
			if (cur_actual == NULL && cur_expected != NULL)
			{
				setOutputRed();
				printf("test_Driver_findValidRowsGivenWhere with id = %d FAILED\n", test_id);
				printf("cur_actual was NULL while cur_expected was NOT NULL\n");
				setOutputWhite();
				freeAnyLinkedList((void**) &actual_results, PTR_TYPE_LIST_NODE_PTR, NULL, malloced_head, the_debug);
				return -1;
			}
			else if (cur_actual != NULL && cur_expected == NULL)
			{
				setOutputRed();
				printf("test_Driver_findValidRowsGivenWhere with id = %d FAILED\n", test_id);
				printf("cur_actual was NOT NULL while cur_expected was NULL\n");
				setOutputWhite();
				freeAnyLinkedList((void**) &actual_results, PTR_TYPE_LIST_NODE_PTR, NULL, malloced_head, the_debug);
				return -1;
			}
			else if (cur_actual != NULL && cur_expected != NULL && !equals(cur_actual->ptr_value, PTR_TYPE_INT, cur_expected->ptr_value, VALUE_EQUALS))
			{
				setOutputRed();
				printf("test_Driver_findValidRowsGivenWhere with id = %d FAILED\n", test_id);
				printf("cur_actual->ptr_value (%d) did not equal cur_expected->ptr_value (%d)\n", *((int*) cur_actual->ptr_value), *((int*) cur_expected->ptr_value));
				setOutputWhite();
				freeAnyLinkedList((void**) &actual_results, PTR_TYPE_LIST_NODE_PTR, NULL, malloced_head, the_debug);
				return -1;
			}

			cur_expected = cur_expected->next;
			cur_actual = cur_actual->next;
		}
	// END Tests


	setOutputWhite();

	freeAnyLinkedList((void**) &actual_results, PTR_TYPE_LIST_NODE_PTR, NULL, malloced_head, the_debug);

	return 0;
}

int test_Driver_selectStuff(int test_id, char* select_string, char* expected_results_csv, struct malloced_node** malloced_head, int the_debug)
{
	setOutputGreen();
	printf("\nStarting test with id = %d\n", test_id);
	setOutputWhite();

	struct select_node* select_node = NULL;
	if (parseSelect(select_string, &select_node, NULL, false, malloced_head, the_debug) != 0)
	{
		setOutputRed();
		printf("test_Driver_selectStuff with id = %d FAILED\n", test_id);
		printf("The test %d had a problem with parseSelect()\n", test_id);
		setOutputWhite();
		return -1;
	}


	if (selectStuff(&select_node, false, malloced_head, the_debug) != 0)
	{
		setOutputRed();
		printf("test_Driver_selectStuff with id = %d FAILED\n", test_id);
		printf("The test %d had a problem with selectStuff()\n", test_id);
		setOutputWhite();
		return -1;
	}


	if (the_debug == YES_DEBUG)
		printf("Successfully retreived %lu rows from disk, creating .csv file\n", select_node->columns_arr[0]->num_rows);
	int displayed = displayResultsOfSelect(select_node, malloced_head, the_debug);

	// START Free stuff
		for (int j=0; j<select_node->columns_arr_size; j++)
		{
			myFreeResultsOfSelect(select_node->columns_arr[j], malloced_head, the_debug);
		}

		while (select_node->prev != NULL)
			select_node = select_node->prev;

		freeAnyLinkedList((void**) &select_node, PTR_TYPE_SELECT_NODE, NULL, malloced_head, the_debug);
		//myFreeAllError(malloced_head, the_debug);
	// END Free stuff

	if (displayed != 0)
	{
		printf("Creation of .csv file had a problem with file i/o, please try again.\n");
		return -1;
	}

	char cmd[200];

	/* WINDOWS
		strcpy(cmd, "certutil -hashfile \0");
		strcat(cmd, expected_results_csv);

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
		if (fgets(var1, 65, fp) != NULL)
		{
			//if (i == 1)
				strcpy(hash1, var1);
			printf("%s\n", var1);
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
		if (fgets(var2, 65, fp) != NULL)
		{
			//if (i == 1)
				strcpy(hash2, var2);
			printf("%s\n", var2);
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
			setOutputRed();
			printf("File hashes DID NOT match for test %d\n", test_id);
			printf("hash1 = %s\n", hash1);
			printf("hash2 = %s\n", hash2);
			setOutputWhite();
		}

		myFree((void**) &var1, NULL, malloced_head, the_debug);
		myFree((void**) &var2, NULL, malloced_head, the_debug);
		myFree((void**) &hash1, NULL, malloced_head, the_debug);
		yFree((void**) &hash2, NULL, malloced_head, the_debug);
	*/

	/* LINUX*/
		strcpy(cmd, "diff -w Results.csv ");
		strcat(cmd, expected_results_csv);

		//printf("cmd = _%s_\n", cmd);

		int compared = 0;

		FILE *fp = popen(cmd, "r");
		if (fp == NULL) 
		{
			printf("Failed to run command\n");
			return -1;
		}

		char output[100];
		if (fgets(output, 65, fp) != NULL)
		{
			//printf("output = _%s_\n", output);
			if (output[0] != 0)
			{
				setOutputRed();
				printf("Files DID NOT match for test %d\n", test_id);
				setOutputWhite();
				compared = -1;
			}
		}

		pclose(fp);
	

	if (compared != 0)
		return -1;

	setOutputGreen();
	printf("\nTest with id = %d PASSED!\n", test_id);
	setOutputWhite();

	return 0;
}

int test_Driver_updateRows(int test_id, char* expected_results_csv, char* input_string
						  ,struct malloced_node** malloced_head, int the_debug)
{
	setOutputGreen();
	printf("\nStarting test with id = %d\n", test_id);
	setOutputWhite();

	struct change_node_v2* change_head = NULL;
	if (parseUpdate(input_string, &change_head, malloced_head, the_debug) != 0)
	{
		setOutputRed();
		printf("test_Driver_updateRows with id = %d FAILED\n", test_id);
		printf("The test %d had a problem with parseUpdate()\n", test_id);
		setOutputWhite();
		return -1;
	}


	if (updateRows(change_head, malloced_head, the_debug) < 0)
	{
		setOutputRed();
		printf("test_Driver_updateRows with id = %d FAILED\n", test_id);
		printf("The test %d had a problem with updateRows()\n", test_id);
		setOutputWhite();
		return -1;
	}
	
	
	freeAnyLinkedList((void**) &change_head, PTR_TYPE_CHANGE_NODE_V2, NULL, malloced_head, the_debug);


	if (the_debug == YES_DEBUG)
		printf("----------------------- Calling selectAndCheckHash()\n");


	return selectAndCheckHash(expected_results_csv, test_id, getTablesHead(), malloced_head, the_debug);
}

int test_Driver_deleteRows(int test_id, char* input_string, char* expected_results_csv, int_8 expected_num_rows, int_8 expected_num_open
						  ,struct malloced_node** malloced_head, int the_debug)
{
	setOutputGreen();
	printf("\nStarting test with id = %d\n", test_id);
	setOutputWhite();

	struct change_node_v2* change_head = NULL;
	if (parseDelete(input_string, &change_head, malloced_head, the_debug) != 0)
	{
		setOutputRed();
		printf("test_Driver_deleteRows with id = %d FAILED\n", test_id);
		printf("The test %d had a problem with parseDelete()\n", test_id);
		setOutputWhite();
		return -1;
	}


	if (deleteRows(change_head, malloced_head, the_debug) < 0)
	{
		setOutputRed();
		printf("test_Driver_deleteRows with id = %d FAILED\n", test_id);
		printf("The test %d had a problem with deleteRows()\n", test_id);
		setOutputWhite();
		return -1;
	}


	freeAnyLinkedList((void**) &change_head, PTR_TYPE_CHANGE_NODE_V2, NULL, malloced_head, the_debug);


	if (the_debug == YES_DEBUG)
		printf("----------------------- Starting test_Driver_deleteRows tests\n");


	struct table_cols_info* cur_col = getTablesHead()->table_cols_head;
	while (cur_col != NULL)
	{
		if (cur_col->num_rows != expected_num_rows)
		{
			setOutputRed();
			printf("test_Driver_deleteRows with id = %d FAILED\n", test_id);
			printf("Actual num_rows %lu did not equal below\n", cur_col->num_rows);
			printf("Expected num_rows %lu\n", expected_num_rows);
			setOutputWhite();
			return -1;
		}
		
		if (cur_col->num_open != expected_num_open)
		{
			setOutputRed();
			printf("test_Driver_deleteRows with id = %d FAILED\n", test_id);
			printf("Actual num_open %lu did not equal below\n", cur_col->num_open);
			printf("Expected num_open %lu\n", expected_num_open);
			setOutputWhite();
			return -1;
		}

		cur_col = cur_col->next;
	}


	return selectAndCheckHash(expected_results_csv, test_id, getTablesHead(), malloced_head, the_debug);
}

int test_Driver_insertRows(int test_id, char* input_string, char* expected_results_csv, int_8 expected_num_rows, int_8 expected_num_open
						  ,struct malloced_node** malloced_head, int the_debug)
{
	setOutputGreen();
	printf("\nStarting test with id = %d\n", test_id);
	setOutputWhite();

	struct change_node_v2* change_head = NULL;
	if (parseInsert(input_string, &change_head, malloced_head, the_debug) != 0)
	{
		setOutputRed();
		printf("test_Driver_insertRows with id = %d FAILED\n", test_id);
		printf("The test %d had a problem with parseInsert()\n", test_id);
		setOutputWhite();
		return -1;
	}


	int_8 rows_inserted = insertRows(change_head, malloced_head, the_debug);
	if (rows_inserted < 0)
	{
		setOutputRed();
		printf("test_Driver_insertRows with id = %d FAILED\n", test_id);
		printf("The test %d had a problem with insertRows()\n", test_id);
		setOutputWhite();
		return -1;
	}
	if (the_debug == YES_DEBUG)
		printf("%lu rows inserted\n", rows_inserted);


	freeAnyLinkedList((void**) &change_head, PTR_TYPE_CHANGE_NODE_V2, NULL, malloced_head, the_debug);


	if (the_debug == YES_DEBUG)
	{
		traverseTablesInfoMemory();
		printf("----------------------- Starting test_Driver_insertRows tests\n");
	}


	struct table_cols_info* cur_col = getTablesHead()->table_cols_head;
	while (cur_col != NULL)
	{
		if (cur_col->num_rows != expected_num_rows)
		{
			setOutputRed();
			printf("test_Driver_insertRows with id = %d FAILED\n", test_id);
			printf("Actual num_rows %lu did not equal below\n", cur_col->num_rows);
			printf("Expected num_rows %lu\n", expected_num_rows);
			setOutputWhite();
			return -1;
		}
		
		if (cur_col->num_open != expected_num_open)
		{
			setOutputRed();
			printf("test_Driver_insertRows with id = %d FAILED\n", test_id);
			printf("Actual num_open %lu did not equal below\n", cur_col->num_open);
			printf("Expected num_open %lu\n", expected_num_open);
			setOutputWhite();
			return -1;
		}

		cur_col = cur_col->next;
	}


	return selectAndCheckHash(expected_results_csv, test_id, getTablesHead(), malloced_head, the_debug);
}

/*int test_Performance_Select(int test_id, char* select_string, struct malloced_node** malloced_head, int the_debug)
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
	int the_debug = YES_DEBUG;//NO_DEBUG;//

	struct malloced_node* malloced_head = NULL;


	/* WINDOWS
	system("copy DB_Files_2_Test_Backups/* DB_Files_2/");*/

	/* LINUX
	char cmd[100];
	strcpy(cmd, "cp -f DB_Files_2_Test_Backups/* DB_Files_2/");

	//printf("cmd = _%s_\n", cmd);

	FILE *fp = popen(cmd, "r");
	if (fp == NULL) 
	{
		printf("Failed to run command\n");
		return -1;
	}

	pclose(fp);*/



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


			if (test_Controller_parseWhereClause(103, "where BRAND_NAME;", "where"
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


			if (test_Controller_parseWhereClause(104, "WherE BRAND_NAME = 'test';", "where"
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


			if (test_Controller_parseWhereClause(105, "WherE BRAND_NAME is null;", "where"
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


			if (test_Controller_parseWhereClause(106, "WherE tbl.BRAND_NAME is not null;", "where"
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


			if (test_Controller_parseWhereClause(107, "WherE CT_REGISTRATION_NUMBER > 1;", "where"
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


			if (test_Controller_parseWhereClause(108, "WherE CT_REGISTRATION_NUMBER >= 1;", "where"
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


			if (test_Controller_parseWhereClause(109, "WherE CT_REGISTRATION_NUMBER <= 1;", "where"
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


			if (test_Controller_parseWhereClause(110, "WherE BRAND_NAME <> 1;", "where"
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


			if (test_Controller_parseWhereClause(111, "WherE BRAND_NAME = 'test' aNd status = 'That' \n ;", "where"
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


			if (test_Controller_parseWhereClause(112, "WherE (1 = 1 or BRAND_NAME = 'test') aNd status = 'That' \n ;", "where"
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


			if (test_Controller_parseWhereClause(113, "WherE 1 = 1 or BRAND_NAME = 'test' and status = 'That' \n ;", "where"
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


			if (test_Controller_parseWhereClause(114, "WherE 1 = 1 and BRAND_NAME = 'test' or status = 'That' \n ;", "where"
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


			if (test_Controller_parseWhereClause(115, "WherE 1 = 1 and BRAND_NAME = 'test' or status = 'That' and 1 <> 2 \n ;", "where"
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


			if (test_Controller_parseWhereClause(116, "WherE 1 = 1 and BRAND_NAME = 'test' AND status = 'That' OR 1 <> 2 \n ;", "where"
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


			if (test_Controller_parseWhereClause(117, "where BRAND_NAME = 13123;", "where"
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
			((struct math_node*) the_where_node->ptr_one)->result_type = PTR_TYPE_INT;


			if (test_Controller_parseWhereClause(118, "where tbl . CT_REGISTRATION_NUMBER + 10 = 13123;", "where"
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
			((struct math_node*) the_where_node->ptr_one)->result_type = PTR_TYPE_INT;


			if (test_Controller_parseWhereClause(119, "where (tbl . CT_REGISTRATION_NUMBER + 10) = 13123;", "where"
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

		// START Test with id = 120
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

				((struct where_clause_node*) the_where_node->ptr_one)->ptr_one = ((struct select_node*) the_select_node)->columns_arr[0];
				((struct where_clause_node*) the_where_node->ptr_one)->ptr_one_type = PTR_TYPE_COL_IN_SELECT_NODE;

				((struct where_clause_node*) the_where_node->ptr_one)->ptr_two = myMalloc(sizeof(char) * 32, NULL, &malloced_head, the_debug);
				strcpy(((struct where_clause_node*) the_where_node->ptr_one)->ptr_two, "VIZZY BLACK CHERRY LIME");
				((struct where_clause_node*) the_where_node->ptr_one)->ptr_two_type = PTR_TYPE_CHAR;

				((struct where_clause_node*) the_where_node->ptr_one)->where_type = WHERE_IS_EQUALS;
				((struct where_clause_node*) the_where_node->ptr_one)->parent = the_where_node;


				((struct where_clause_node*) the_where_node->ptr_two)->ptr_one = (struct where_clause_node*) myMalloc(sizeof(struct where_clause_node), NULL, &malloced_head, the_debug);
				((struct where_clause_node*) the_where_node->ptr_two)->ptr_one_type = PTR_TYPE_WHERE_CLAUSE_NODE;

				((struct where_clause_node*) the_where_node->ptr_two)->ptr_two = (struct where_clause_node*) myMalloc(sizeof(struct where_clause_node), NULL, &malloced_head, the_debug);
				((struct where_clause_node*) the_where_node->ptr_two)->ptr_two_type = PTR_TYPE_WHERE_CLAUSE_NODE;

				((struct where_clause_node*) the_where_node->ptr_two)->where_type = WHERE_OR;
				((struct where_clause_node*) the_where_node->ptr_two)->parent = the_where_node;

					
					((struct where_clause_node*) ((struct where_clause_node*) the_where_node->ptr_two)->ptr_one)->ptr_one = (struct where_clause_node*) myMalloc(sizeof(struct where_clause_node), NULL, &malloced_head, the_debug);
					((struct where_clause_node*) ((struct where_clause_node*) the_where_node->ptr_two)->ptr_one)->ptr_one_type = PTR_TYPE_WHERE_CLAUSE_NODE;

					((struct where_clause_node*) ((struct where_clause_node*) the_where_node->ptr_two)->ptr_one)->ptr_two = (struct where_clause_node*) myMalloc(sizeof(struct where_clause_node), NULL, &malloced_head, the_debug);
					((struct where_clause_node*) ((struct where_clause_node*) the_where_node->ptr_two)->ptr_one)->ptr_two_type = PTR_TYPE_WHERE_CLAUSE_NODE;

					((struct where_clause_node*) ((struct where_clause_node*) the_where_node->ptr_two)->ptr_one)->where_type = WHERE_AND;
					((struct where_clause_node*) ((struct where_clause_node*) the_where_node->ptr_two)->ptr_one)->parent = the_where_node->ptr_one;


					((struct where_clause_node*) ((struct where_clause_node*) the_where_node->ptr_two)->ptr_two)->ptr_one = ((struct select_node*) the_select_node)->columns_arr[4];
					((struct where_clause_node*) ((struct where_clause_node*) the_where_node->ptr_two)->ptr_two)->ptr_one_type = PTR_TYPE_COL_IN_SELECT_NODE;

					((struct where_clause_node*) ((struct where_clause_node*) the_where_node->ptr_two)->ptr_two)->ptr_two = myMalloc(sizeof(char) * 16, NULL, &malloced_head, the_debug);
					strcpy(((struct where_clause_node*) ((struct where_clause_node*) the_where_node->ptr_two)->ptr_two)->ptr_two, "1/31/2025");
					((struct where_clause_node*) ((struct where_clause_node*) the_where_node->ptr_two)->ptr_two)->ptr_two_type = PTR_TYPE_DATE;

					((struct where_clause_node*) ((struct where_clause_node*) the_where_node->ptr_two)->ptr_two)->where_type = WHERE_IS_EQUALS;
					((struct where_clause_node*) ((struct where_clause_node*) the_where_node->ptr_two)->ptr_two)->parent = the_where_node->ptr_one;


						((struct where_clause_node*) ((struct where_clause_node*) ((struct where_clause_node*) the_where_node->ptr_two)->ptr_one)->ptr_one)->ptr_one = ((struct select_node*) the_select_node)->columns_arr[0];
						((struct where_clause_node*) ((struct where_clause_node*) ((struct where_clause_node*) the_where_node->ptr_two)->ptr_one)->ptr_one)->ptr_one_type = PTR_TYPE_COL_IN_SELECT_NODE;

						((struct where_clause_node*) ((struct where_clause_node*) ((struct where_clause_node*) the_where_node->ptr_two)->ptr_one)->ptr_one)->ptr_two = myMalloc(sizeof(char) * 32, NULL, &malloced_head, the_debug);
						strcpy(((struct where_clause_node*) ((struct where_clause_node*) ((struct where_clause_node*) the_where_node->ptr_two)->ptr_one)->ptr_one)->ptr_two, "CRUZAN ISLAND SPICED RUM");
						((struct where_clause_node*) ((struct where_clause_node*) ((struct where_clause_node*) the_where_node->ptr_two)->ptr_one)->ptr_one)->ptr_two_type = PTR_TYPE_CHAR;

						((struct where_clause_node*) ((struct where_clause_node*) ((struct where_clause_node*) the_where_node->ptr_two)->ptr_one)->ptr_one)->where_type = WHERE_IS_EQUALS;
						((struct where_clause_node*) ((struct where_clause_node*) ((struct where_clause_node*) the_where_node->ptr_two)->ptr_one)->ptr_one)->parent = ((struct where_clause_node*) the_where_node->ptr_one)->ptr_one;


						((struct where_clause_node*) ((struct where_clause_node*) ((struct where_clause_node*) the_where_node->ptr_two)->ptr_one)->ptr_two)->ptr_one = ((struct select_node*) the_select_node)->columns_arr[1];
						((struct where_clause_node*) ((struct where_clause_node*) ((struct where_clause_node*) the_where_node->ptr_two)->ptr_one)->ptr_two)->ptr_one_type = PTR_TYPE_COL_IN_SELECT_NODE;

						((struct where_clause_node*) ((struct where_clause_node*) ((struct where_clause_node*) the_where_node->ptr_two)->ptr_one)->ptr_two)->ptr_two = myMalloc(sizeof(int), NULL, &malloced_head, the_debug);
						*((int*)((struct where_clause_node*) ((struct where_clause_node*) ((struct where_clause_node*) the_where_node->ptr_two)->ptr_one)->ptr_two)->ptr_two) = 169876;
						((struct where_clause_node*) ((struct where_clause_node*) ((struct where_clause_node*) the_where_node->ptr_two)->ptr_one)->ptr_two)->ptr_two_type = PTR_TYPE_INT;

						((struct where_clause_node*) ((struct where_clause_node*) ((struct where_clause_node*) the_where_node->ptr_two)->ptr_one)->ptr_two)->where_type = WHERE_IS_EQUALS;
						((struct where_clause_node*) ((struct where_clause_node*) ((struct where_clause_node*) the_where_node->ptr_two)->ptr_one)->ptr_two)->parent = ((struct where_clause_node*) the_where_node->ptr_one)->ptr_one;


			if (test_Controller_parseWhereClause(120, "where BRAND_NAME = 'VIZZY BLACK CHERRY LIME' or BRAND_NAME = 'CRUZAN ISLAND SPICED RUM' and CT_REGISTRATION_NUMBER = 169876 or EXPIRATION = '1/31/2025';"
												,"where"
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
		// END Test with id = 120
	// END test_Controller_parseWhereClause

	// START test_Controller_parseSelect
		// START Test with id = 501
			initSelectClauseForComp(&the_select_node, NULL, false, 0, NULL, getTablesHead(), PTR_TYPE_TABLE_INFO, NULL, NULL
								   ,&malloced_head, the_debug);
			initSelectClauseForComp(&the_select_node->next, NULL, false, 0, NULL, the_select_node, PTR_TYPE_SELECT_NODE, NULL, NULL
								   ,&malloced_head, the_debug);
			the_select_node->next->prev = the_select_node;


			if (test_Controller_parseSelect(501, "select * from LIQUOR_LICENSES;", &the_select_node
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
			col_arr[0]->rows_data_type = ((struct select_node*) the_select_node)->columns_arr[0]->rows_data_type;

			col_arr[1] = (struct col_in_select_node*) myMalloc(sizeof(struct col_in_select_node), NULL, &malloced_head, the_debug);
			col_arr[1]->table_ptr = the_select_node;
			col_arr[1]->table_ptr_type = PTR_TYPE_SELECT_NODE;
			col_arr[1]->col_ptr = ((struct select_node*) the_select_node)->columns_arr[6];
			col_arr[1]->col_ptr_type = PTR_TYPE_COL_IN_SELECT_NODE;
			col_arr[1]->new_name = upper("COL2", NULL, &malloced_head, the_debug);
			col_arr[1]->func_node = NULL;
			col_arr[1]->math_node = NULL;
			col_arr[1]->case_node = NULL;
			col_arr[1]->rows_data_type = ((struct select_node*) the_select_node)->columns_arr[6]->rows_data_type;

			initSelectClauseForComp(&the_select_node->next, NULL, true, 2, col_arr, NULL, -1, NULL, NULL
								   ,&malloced_head, the_debug);
			the_select_node->next->prev = the_select_node;
			

			if (test_Controller_parseSelect(502, "select distinct tbl.BRAND_NAME as col1, SUPERVISOR_CREDENTIAL col2 from LIQUOR_LICENSES as tbl;", &the_select_node
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
			col_arr[0]->new_name = upper("CNT", NULL, &malloced_head, the_debug);
			col_arr[0]->math_node = NULL;
			col_arr[0]->case_node = NULL;

			col_arr[0]->func_node = (struct func_node*) myMalloc(sizeof(struct func_node), NULL, &malloced_head, the_debug);
			col_arr[0]->func_node->which_func = FUNC_COUNT;
			col_arr[0]->func_node->distinct = false;
			col_arr[0]->func_node->args_size = 7;
			col_arr[0]->func_node->args_arr = (void**) myMalloc(sizeof(void*) * 7, NULL, &malloced_head, the_debug);
			col_arr[0]->func_node->args_arr_type = (int*) myMalloc(sizeof(int) * 7, NULL, &malloced_head, the_debug);
			for (int a=0; a<7; a++)
			{
				col_arr[0]->func_node->args_arr[a] = the_select_node->columns_arr[a];
				col_arr[0]->func_node->args_arr_type[a] = PTR_TYPE_COL_IN_SELECT_NODE;
			}
			col_arr[0]->func_node->group_by_cols_head = NULL;
			col_arr[0]->func_node->group_by_cols_tail = NULL;
			addListNodePtr(&col_arr[0]->func_node->group_by_cols_head, &col_arr[0]->func_node->group_by_cols_tail
						  ,((struct select_node*) the_select_node)->columns_arr[1], PTR_TYPE_COL_IN_SELECT_NODE, ADDLISTNODE_TAIL
				  		  ,NULL, &malloced_head, the_debug);
			col_arr[0]->func_node->result_type = PTR_TYPE_INT;
			col_arr[0]->rows_data_type = col_arr[0]->func_node->result_type;

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
			col_arr[1]->math_node->result_type = PTR_TYPE_INT;
			col_arr[1]->rows_data_type = col_arr[1]->math_node->result_type;
		

			initSelectClauseForComp(&the_select_node->next, NULL, false, 2, col_arr, NULL, -1, NULL, NULL
								   ,&malloced_head, the_debug);
			the_select_node->next->prev = the_select_node;

			if (test_Controller_parseSelect(503, "select count\n(* ) as CNT, CT_REGISTRATION_NUMBER - 10000 math from LIQUOR_LICENSES tbl group by CT_REGISTRATION_NUMBER;", &the_select_node
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
				col_arr[i]->math_node = NULL;
				col_arr[i]->case_node = NULL;

				col_arr[i]->func_node = (struct func_node*) myMalloc(sizeof(struct func_node), NULL, &malloced_head, the_debug);
				if (i != 1)
				{
					col_arr[i]->func_node->args_size = 1;
					col_arr[i]->func_node->args_arr = (void**) myMalloc(sizeof(void*), NULL, &malloced_head, the_debug);
					col_arr[i]->func_node->args_arr_type = (int*) myMalloc(sizeof(int), NULL, &malloced_head, the_debug);
				}	

				if (i == 0)
				{
					col_arr[i]->new_name = myMalloc(sizeof(char) * 16, NULL, &malloced_head, the_debug);
					strcpy(col_arr[i]->new_name, "avg func");
					col_arr[i]->func_node->which_func = FUNC_AVG;
					col_arr[i]->func_node->args_arr[0] = ((struct select_node*) the_select_node)->columns_arr[1];
					col_arr[i]->func_node->result_type = PTR_TYPE_REAL;
				}
				else if (i == 1)
				{
					col_arr[i]->new_name = upper("CNT", NULL, &malloced_head, the_debug);
					col_arr[i]->func_node->which_func = FUNC_COUNT;
					col_arr[i]->func_node->args_size = 7;
					col_arr[i]->func_node->args_arr = (void**) myMalloc(sizeof(void*) * 7, NULL, &malloced_head, the_debug);
					col_arr[i]->func_node->args_arr_type = (int*) myMalloc(sizeof(int) * 7, NULL, &malloced_head, the_debug);
					for (int a=0; a<7; a++)
					{
						col_arr[i]->func_node->args_arr[a] = the_select_node->columns_arr[a];
						col_arr[i]->func_node->args_arr_type[a] = PTR_TYPE_COL_IN_SELECT_NODE;
					}
					col_arr[i]->func_node->result_type = PTR_TYPE_INT;
				}
				else if (i == 2)
				{
					col_arr[i]->new_name = upper("FIRST", NULL, &malloced_head, the_debug);
					col_arr[i]->func_node->which_func = FUNC_FIRST;
					col_arr[i]->func_node->args_arr[0] = ((struct select_node*) the_select_node)->columns_arr[3];
					col_arr[i]->func_node->result_type = ((struct select_node*) the_select_node)->columns_arr[3]->rows_data_type;
				}
				else if (i == 3)
				{
					col_arr[i]->new_name = upper("LAST_FUNC", NULL, &malloced_head, the_debug);
					col_arr[i]->func_node->which_func = FUNC_LAST;
					col_arr[i]->func_node->args_arr[0] = ((struct select_node*) the_select_node)->columns_arr[4];
					col_arr[i]->func_node->result_type = ((struct select_node*) the_select_node)->columns_arr[4]->rows_data_type;
				}
				else if (i == 4)
				{
					col_arr[i]->new_name = upper("MIN", NULL, &malloced_head, the_debug);
					col_arr[i]->func_node->which_func = FUNC_MIN;
					col_arr[i]->func_node->args_arr[0] = ((struct select_node*) the_select_node)->columns_arr[1];
					col_arr[i]->func_node->result_type = ((struct select_node*) the_select_node)->columns_arr[1]->rows_data_type;
				}
				else if (i == 5)
				{
					col_arr[i]->new_name = upper("MAX", NULL, &malloced_head, the_debug);
					col_arr[i]->func_node->which_func = FUNC_MAX;
					col_arr[i]->func_node->args_arr[0] = ((struct select_node*) the_select_node)->columns_arr[4];
					col_arr[i]->func_node->result_type = ((struct select_node*) the_select_node)->columns_arr[4]->rows_data_type;
				}
				else if (i == 6)
				{
					col_arr[i]->new_name = upper("MED", NULL, &malloced_head, the_debug);
					col_arr[i]->func_node->which_func = FUNC_MEDIAN;
					col_arr[i]->func_node->args_arr[0] = ((struct select_node*) the_select_node)->columns_arr[1];
					col_arr[i]->func_node->result_type = ((struct select_node*) the_select_node)->columns_arr[1]->rows_data_type;
				}
				else if (i == 7)
				{
					col_arr[i]->new_name = upper("SUM", NULL, &malloced_head, the_debug);
					col_arr[i]->func_node->which_func = FUNC_SUM;
					col_arr[i]->func_node->args_arr[0] = ((struct select_node*) the_select_node)->columns_arr[1];
					col_arr[i]->func_node->result_type = ((struct select_node*) the_select_node)->columns_arr[1]->rows_data_type;
				}

				if (i == 1)
					col_arr[i]->func_node->distinct = true;
				else
					col_arr[i]->func_node->distinct = false;

				if (i != 1)
					col_arr[i]->func_node->args_arr_type[0] = PTR_TYPE_COL_IN_SELECT_NODE;					
				
				
				col_arr[i]->func_node->group_by_cols_head = NULL;
				col_arr[i]->func_node->group_by_cols_tail = NULL;

				col_arr[i]->rows_data_type = col_arr[i]->func_node->result_type;
			}

			initSelectClauseForComp(&the_select_node->next, NULL, false, 8, col_arr, NULL, -1, NULL, NULL
								   ,&malloced_head, the_debug);
			the_select_node->next->prev = the_select_node;
			

			if (test_Controller_parseSelect(504, "select avg(CT_REGISTRATION_NUMBER) \"avg func\", count ( distinct * ) CNT, first(EFFECTIVE) FIRST, last(EXPIRATION) last_func, min(CT_REGISTRATION_NUMBER) MIN, max(EXPIRATION) MAX, median(CT_REGISTRATION_NUMBER) MED, sum(CT_REGISTRATION_NUMBER) SUM from LIQUOR_LICENSES;", &the_select_node
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

			if (test_Controller_parseSelect(505, "select avg( BRAND_NAME ) from LIQUOR_LICENSES;", &the_select_node
										   ,&parsed_error_code, &malloced_head, the_debug) != 0)
				return -1;

			if (test_Controller_parseSelect(506, "select sum(CT_REGISTRATION_NUMBER) ) from LIQUOR_LICENSES;", &the_select_node
										   ,&parsed_error_code, &malloced_head, the_debug) != 0)
				return -1;

			if (test_Controller_parseSelect(507, "select min\n(\tCT_REGISTRATION_NUMBER,) from LIQUOR_LICENSES;", &the_select_node
										   ,&parsed_error_code, &malloced_head, the_debug) != 0)
				return -1;

			if (test_Controller_parseSelect(508, "select count(distinct ) from LIQUOR_LICENSES;", &the_select_node
										   ,&parsed_error_code, &malloced_head, the_debug) != 0)
				return -1;

			if (test_Controller_parseSelect(509, "select sum( (CT_REGISTRATION_NUMBER) from LIQUOR_LICENSES;", &the_select_node
										   ,&parsed_error_code, &malloced_head, the_debug) != 0)
				return -1;

			if (test_Controller_parseSelect(510, "select sum(CT_REGISTRATION_NUMBER,CT_REGISTRATION_NUMBER) from LIQUOR_LICENSES;", &the_select_node
										   ,&parsed_error_code, &malloced_head, the_debug) != 0)
				return -1;

			if (test_Controller_parseSelect(511, "select count(distinct BRAND_NAME), from LIQUOR_LICENSES;", &the_select_node
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
					strcpy(col_arr[i]->new_name, "REG_NUM");
				else if (i == 2)
					strcpy(col_arr[i]->new_name, "POW");
				else if (i == 3)
					strcpy(col_arr[i]->new_name, "EFF");
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
			col_arr[0]->math_node->result_type = PTR_TYPE_REAL;

				((struct math_node*) col_arr[0]->math_node->ptr_two)->ptr_one = myMalloc(sizeof(struct math_node), NULL, &malloced_head, the_debug);
				((struct math_node*) col_arr[0]->math_node->ptr_two)->ptr_one_type = PTR_TYPE_MATH_NODE;

				((struct math_node*) col_arr[0]->math_node->ptr_two)->ptr_two = myMalloc(sizeof(int), NULL, &malloced_head, the_debug);
				*((int*) ((struct math_node*) col_arr[0]->math_node->ptr_two)->ptr_two) = 10;
				((struct math_node*) col_arr[0]->math_node->ptr_two)->ptr_two_type = PTR_TYPE_INT;

				((struct math_node*) col_arr[0]->math_node->ptr_two)->operation = MATH_MULT;
				((struct math_node*) col_arr[0]->math_node->ptr_two)->parent = col_arr[0]->math_node;
				((struct math_node*) col_arr[0]->math_node->ptr_two)->result_type = PTR_TYPE_REAL;

					((struct math_node*) ((struct math_node*) col_arr[0]->math_node->ptr_two)->ptr_one)->ptr_one = myMalloc(sizeof(double), NULL, &malloced_head, the_debug);
					*((double*) ((struct math_node*) ((struct math_node*) col_arr[0]->math_node->ptr_two)->ptr_one)->ptr_one) = 110.5;
					((struct math_node*) ((struct math_node*) col_arr[0]->math_node->ptr_two)->ptr_one)->ptr_one_type = PTR_TYPE_REAL;

					((struct math_node*) ((struct math_node*) col_arr[0]->math_node->ptr_two)->ptr_one)->ptr_two = myMalloc(sizeof(int), NULL, &malloced_head, the_debug);
					*((int*) ((struct math_node*) ((struct math_node*) col_arr[0]->math_node->ptr_two)->ptr_one)->ptr_two) = 5;
					((struct math_node*) ((struct math_node*) col_arr[0]->math_node->ptr_two)->ptr_one)->ptr_two_type = PTR_TYPE_INT;

					((struct math_node*) ((struct math_node*) col_arr[0]->math_node->ptr_two)->ptr_one)->operation = MATH_DIV;
					((struct math_node*) ((struct math_node*) col_arr[0]->math_node->ptr_two)->ptr_one)->parent = col_arr[0]->math_node->ptr_two;
					((struct math_node*) ((struct math_node*) col_arr[0]->math_node->ptr_two)->ptr_one)->result_type = PTR_TYPE_REAL;

			col_arr[1]->math_node = (struct math_node*) myMalloc(sizeof(struct math_node), NULL, &malloced_head, the_debug);

			col_arr[1]->math_node->ptr_one = myMalloc(sizeof(int), NULL, &malloced_head, the_debug);
			*((int*) col_arr[1]->math_node->ptr_one) = 100000;
			col_arr[1]->math_node->ptr_one_type = PTR_TYPE_INT;

			col_arr[1]->math_node->ptr_two = myMalloc(sizeof(struct math_node), NULL, &malloced_head, the_debug);
			col_arr[1]->math_node->ptr_two_type = PTR_TYPE_MATH_NODE;

			col_arr[1]->math_node->operation = MATH_SUB;
			col_arr[1]->math_node->parent = NULL;
			col_arr[1]->math_node->result_type = PTR_TYPE_INT;

				((struct math_node*) col_arr[1]->math_node->ptr_two)->ptr_one = ((struct select_node*) the_select_node)->columns_arr[1];
				((struct math_node*) col_arr[1]->math_node->ptr_two)->ptr_one_type = PTR_TYPE_COL_IN_SELECT_NODE;

				((struct math_node*) col_arr[1]->math_node->ptr_two)->ptr_two = myMalloc(sizeof(int), NULL, &malloced_head, the_debug);
				*((int*) ((struct math_node*) col_arr[1]->math_node->ptr_two)->ptr_two) = 10;
				((struct math_node*) col_arr[1]->math_node->ptr_two)->ptr_two_type = PTR_TYPE_INT;

				((struct math_node*) col_arr[1]->math_node->ptr_two)->operation = MATH_MULT;
				((struct math_node*) col_arr[1]->math_node->ptr_two)->parent = col_arr[1]->math_node;
				((struct math_node*) col_arr[1]->math_node->ptr_two)->result_type = PTR_TYPE_INT;

			col_arr[2]->math_node = (struct math_node*) myMalloc(sizeof(struct math_node), NULL, &malloced_head, the_debug);

			col_arr[2]->math_node->ptr_one = myMalloc(sizeof(struct math_node), NULL, &malloced_head, the_debug);
			col_arr[2]->math_node->ptr_one_type = PTR_TYPE_MATH_NODE;

			col_arr[2]->math_node->ptr_two = myMalloc(sizeof(int), NULL, &malloced_head, the_debug);
			*((int*) col_arr[2]->math_node->ptr_two) = 2;
			col_arr[2]->math_node->ptr_two_type = PTR_TYPE_INT;

			col_arr[2]->math_node->operation = MATH_POW;
			col_arr[2]->math_node->parent = NULL;
			col_arr[2]->math_node->result_type = PTR_TYPE_INT;

				((struct math_node*) col_arr[2]->math_node->ptr_one)->ptr_one = myMalloc(sizeof(int), NULL, &malloced_head, the_debug);
				*((int*) ((struct math_node*) col_arr[2]->math_node->ptr_one)->ptr_one) = 2;
				((struct math_node*) col_arr[2]->math_node->ptr_one)->ptr_one_type = PTR_TYPE_INT;

				((struct math_node*) col_arr[2]->math_node->ptr_one)->ptr_two = myMalloc(sizeof(int), NULL, &malloced_head, the_debug);
				*((int*) ((struct math_node*) col_arr[2]->math_node->ptr_one)->ptr_two) = 8;
				((struct math_node*) col_arr[2]->math_node->ptr_one)->ptr_two_type = PTR_TYPE_INT;

				((struct math_node*) col_arr[2]->math_node->ptr_one)->operation = MATH_ADD;
				((struct math_node*) col_arr[2]->math_node->ptr_one)->parent = col_arr[2]->math_node;
				((struct math_node*) col_arr[2]->math_node->ptr_one)->result_type = PTR_TYPE_INT;

			col_arr[3]->math_node = (struct math_node*) myMalloc(sizeof(struct math_node), NULL, &malloced_head, the_debug);

			col_arr[3]->math_node->ptr_one = ((struct select_node*) the_select_node)->columns_arr[3];
			col_arr[3]->math_node->ptr_one_type = PTR_TYPE_COL_IN_SELECT_NODE;

			col_arr[3]->math_node->ptr_two = myMalloc(sizeof(int), NULL, &malloced_head, the_debug);
			*((int*) col_arr[3]->math_node->ptr_two) = 100;
			col_arr[3]->math_node->ptr_two_type = PTR_TYPE_INT;

			col_arr[3]->math_node->operation = MATH_SUB;
			col_arr[3]->math_node->parent = NULL;
			col_arr[3]->math_node->result_type = PTR_TYPE_INT;

			for (int i=0; i<4; i++)
				col_arr[i]->rows_data_type = col_arr[i]->math_node->result_type;

			initSelectClauseForComp(&the_select_node->next, NULL, false, 4, col_arr, NULL, -1, NULL, NULL
								   ,&malloced_head, the_debug);
			the_select_node->next->prev = the_select_node;


			if (test_Controller_parseSelect(512, "select 41 + ((110.5 / 5) * 10) \"41\", 100000 - CT_REGISTRATION_NUMBER * 10 REG_NUM, (2 + 8)^2 POW, EFFECTIVE - 100 EFF from LIQUOR_LICENSES", &the_select_node
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

			if (test_Controller_parseSelect(513, "select 10 + - 10 from LIQUOR_LICENSES;", &the_select_node
										   ,&parsed_error_code, &malloced_head, the_debug) != 0)
				return -1;

			if (test_Controller_parseSelect(514, "select * 10 from LIQUOR_LICENSES;", &the_select_node
										   ,&parsed_error_code, &malloced_head, the_debug) != 0)
				return -1;

			if (test_Controller_parseSelect(515, "select / 20 from LIQUOR_LICENSES;", &the_select_node
										   ,&parsed_error_code, &malloced_head, the_debug) != 0)
				return -1;

			if (test_Controller_parseSelect(516, "select 10 * () - 20 from LIQUOR_LICENSES;", &the_select_node
										   ,&parsed_error_code, &malloced_head, the_debug) != 0)
				return -1;

			if (test_Controller_parseSelect(517, "select 30 ^ as math_boi from LIQUOR_LICENSES;", &the_select_node
										   ,&parsed_error_code, &malloced_head, the_debug) != 0)
				return -1;

			if (test_Controller_parseSelect(518, "select 40 ^ BRAND_NAME math_boi from LIQUOR_LICENSES;", &the_select_node
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
				strcpy(col_arr[i]->new_name, "REG_NUM");
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
			col_arr[0]->math_node->result_type = PTR_TYPE_INT;

				((struct math_node*) col_arr[0]->math_node->ptr_two)->ptr_one = ((struct select_node*) the_select_node)->columns_arr[1];
				((struct math_node*) col_arr[0]->math_node->ptr_two)->ptr_one_type = PTR_TYPE_COL_IN_SELECT_NODE;

				((struct math_node*) col_arr[0]->math_node->ptr_two)->ptr_two = myMalloc(sizeof(int), NULL, &malloced_head, the_debug);
				*((int*) ((struct math_node*) col_arr[0]->math_node->ptr_two)->ptr_two) = 10;
				((struct math_node*) col_arr[0]->math_node->ptr_two)->ptr_two_type = PTR_TYPE_INT;

				((struct math_node*) col_arr[0]->math_node->ptr_two)->operation = MATH_MULT;
				((struct math_node*) col_arr[0]->math_node->ptr_two)->parent = col_arr[0]->math_node;
				((struct math_node*) col_arr[0]->math_node->ptr_two)->result_type = PTR_TYPE_INT;

			col_arr[0]->rows_data_type = col_arr[0]->math_node->result_type;


			initSelectClauseForComp(&the_select_node->next, NULL, false, 1, col_arr, NULL, -1, NULL, NULL
								   ,&malloced_head, the_debug);
			the_select_node->next->prev = the_select_node;


			if (test_Controller_parseSelect(519, "select 100000 - tbl.CT_REGISTRATION_NUMBER * 10 REG_NUM from LIQUOR_LICENSES tbl;", &the_select_node
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
				strcpy(col_arr[i]->new_name, "REG_NUM");
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
			col_arr[0]->math_node->result_type = PTR_TYPE_INT;

				((struct math_node*) col_arr[0]->math_node->ptr_two)->ptr_one = ((struct select_node*) the_select_node)->columns_arr[1];
				((struct math_node*) col_arr[0]->math_node->ptr_two)->ptr_one_type = PTR_TYPE_COL_IN_SELECT_NODE;

				((struct math_node*) col_arr[0]->math_node->ptr_two)->ptr_two = myMalloc(sizeof(int), NULL, &malloced_head, the_debug);
				*((int*) ((struct math_node*) col_arr[0]->math_node->ptr_two)->ptr_two) = 10;
				((struct math_node*) col_arr[0]->math_node->ptr_two)->ptr_two_type = PTR_TYPE_INT;

				((struct math_node*) col_arr[0]->math_node->ptr_two)->operation = MATH_MULT;
				((struct math_node*) col_arr[0]->math_node->ptr_two)->parent = col_arr[0]->math_node;
				((struct math_node*) col_arr[0]->math_node->ptr_two)->result_type = PTR_TYPE_INT;

			col_arr[0]->rows_data_type = col_arr[0]->math_node->result_type;


			initSelectClauseForComp(&the_select_node->next, NULL, false, 1, col_arr, NULL, -1, NULL, NULL
								   ,&malloced_head, the_debug);
			the_select_node->next->prev = the_select_node;


			if (test_Controller_parseSelect(520, "select 100000 - LIQUOR_LICENSES.CT_REGISTRATION_NUMBER * 10 REG_NUM from LIQUOR_LICENSES;", &the_select_node
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

					col_arr[index]->col_ptr = the_select_node->columns_arr[i];
					col_arr[index]->col_ptr_type = PTR_TYPE_COL_IN_SELECT_NODE;

					col_arr[index]->new_name = NULL;

					col_arr[index]->func_node = NULL;
					col_arr[index]->math_node = NULL;
					col_arr[index]->case_node = NULL;

					col_arr[index]->rows_data_type = the_select_node->columns_arr[i]->rows_data_type;

					index++;
				}
			}

			initSelectClauseForComp(&the_select_node->next, NULL, false, 6, col_arr, NULL, -1, NULL, NULL
								   ,&malloced_head, the_debug);
			the_select_node->next->prev = the_select_node;


			if (test_Controller_parseSelect(521, "select * except EXPIRATION from LIQUOR_LICENSES;", &the_select_node
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

					col_arr[index]->rows_data_type = the_select_node->columns_arr[i]->rows_data_type;

					index++;
				}
			}

			initSelectClauseForComp(&the_select_node->next, NULL, false, 5, col_arr, NULL, -1, NULL, NULL
								   ,&malloced_head, the_debug);
			the_select_node->next->prev = the_select_node;


			if (test_Controller_parseSelect(522, "select * except EXPIRATION,CT_REGISTRATION_NUMBER from LIQUOR_LICENSES;", &the_select_node
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
					col_arr[i]->rows_data_type = the_select_node->columns_arr[0]->rows_data_type;
					col_arr[i]->new_name = upper("COL1", NULL, &malloced_head, the_debug);
				}
				else if (i == 1)
				{
					col_arr[i]->table_ptr = the_select_node;
					col_arr[i]->col_ptr = the_select_node->columns_arr[6];
					col_arr[i]->rows_data_type = the_select_node->columns_arr[6]->rows_data_type;
					col_arr[i]->new_name = upper("COL2", NULL, &malloced_head, the_debug);
				}
				else if (i == 2)
				{
					col_arr[i]->table_ptr = joined_select;
					col_arr[i]->col_ptr = joined_select->columns_arr[0];
					col_arr[i]->rows_data_type = joined_select->columns_arr[0]->rows_data_type;
					col_arr[i]->new_name = upper("COL3", NULL, &malloced_head, the_debug);
				}
				else if (i == 3)
				{
					col_arr[i]->table_ptr = joined_select;
					col_arr[i]->col_ptr = joined_select->columns_arr[6];
					col_arr[i]->rows_data_type = joined_select->columns_arr[6]->rows_data_type;
					col_arr[i]->new_name = upper("COL4", NULL, &malloced_head, the_debug);
				}
				col_arr[i]->table_ptr_type = PTR_TYPE_SELECT_NODE;
				col_arr[i]->col_ptr_type = PTR_TYPE_COL_IN_SELECT_NODE;
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


			if (test_Controller_parseSelect(523, "select tbl.BRAND_NAME COL1, tbl.SUPERVISOR_CREDENTIAL COL2, tbl2.BRAND_NAME COL3, tbl2.SUPERVISOR_CREDENTIAL COL4 from LIQUOR_LICENSES tbl join LIQUOR_LICENSES tbl2 on tbl.BRAND_NAME = tbl2.BRAND_NAME;", &the_select_node
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


			if (test_Controller_parseSelect(524, "select tbl.BRAND_NAME, tbl.SUPERVISOR_CREDENTIAL, tbl2.BRAND_NAME, tbl3.SUPERVISOR_CREDENTIAL from LIQUOR_LICENSES tbl join LIQUOR_LICENSES tbl2 on tbl.BRAND_NAME = tbl2.BRAND_NAME;", &the_select_node
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


			if (test_Controller_parseSelect(525, "select tbl.BRAND_NAME, tbl.SUPERVISOR_CREDENTIAL, BRAND_NAME, tbl2.SUPERVISOR_CREDENTIAL from LIQUOR_LICENSES tbl join LIQUOR_LICENSES tbl2 on tbl.BRAND_NAME = tbl2.BRAND_NAME;", &the_select_node
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
					col_arr[i]->rows_data_type = the_select_node->columns_arr[0]->rows_data_type;
					col_arr[i]->new_name = upper("COL1", NULL, &malloced_head, the_debug);
				}
				else if (i == 1)
				{
					col_arr[i]->table_ptr = the_select_node;
					col_arr[i]->col_ptr = the_select_node->columns_arr[6];
					col_arr[i]->rows_data_type = the_select_node->columns_arr[6]->rows_data_type;
					col_arr[i]->new_name = upper("COL2", NULL, &malloced_head, the_debug);
				}
				else if (i == 2)
				{
					col_arr[i]->table_ptr = joined_select;
					col_arr[i]->col_ptr = joined_select->columns_arr[0];
					col_arr[i]->rows_data_type = joined_select->columns_arr[0]->rows_data_type;
					col_arr[i]->new_name = upper("COL3", NULL, &malloced_head, the_debug);
				}
				else if (i == 3)
				{
					col_arr[i]->table_ptr = joined_select;
					col_arr[i]->col_ptr = joined_select->columns_arr[6];
					col_arr[i]->rows_data_type = joined_select->columns_arr[6]->rows_data_type;
					col_arr[i]->new_name = upper("COL4", NULL, &malloced_head, the_debug);
				}
				else if (i == 4)
				{
					col_arr[i]->table_ptr = joined_select_3;
					col_arr[i]->col_ptr = joined_select_3->columns_arr[1];
					col_arr[i]->rows_data_type = joined_select_3->columns_arr[1]->rows_data_type;
					col_arr[i]->new_name = upper("COL5", NULL, &malloced_head, the_debug);
				}
				col_arr[i]->table_ptr_type = PTR_TYPE_SELECT_NODE;
				col_arr[i]->col_ptr_type = PTR_TYPE_COL_IN_SELECT_NODE;
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


			if (test_Controller_parseSelect(526, "select tbl.BRAND_NAME COL1, tbl.SUPERVISOR_CREDENTIAL COL2, tbl2.BRAND_NAME COL3, tbl2.SUPERVISOR_CREDENTIAL COL4, tbl3.CT_REGISTRATION_NUMBER COL5 from LIQUOR_LICENSES tbl right join LIQUOR_LICENSES tbl2 on tbl.BRAND_NAME = tbl2.BRAND_NAME left join LIQUOR_LICENSES tbl3	on tbl.CT_REGISTRATION_NUMBER = tbl3.CT_REGISTRATION_NUMBER;", &the_select_node
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


			if (test_Controller_parseSelect(527, "select tbl.BRAND_NAME, tbl.SUPERVISOR_CREDENTIAL, tbl2.BRAND_NAME, tbl2.SUPERVISOR_CREDENTIAL, tbl3.CT_REGISTRATION_NUMBER from LIQUOR_LICENSES tbl join LIQUOR_LICENSES tbl2 on tbl.BRAND_NAME = tbl3.BRAND_NAME left join LIQUOR_LICENSES tbl3	on tbl.CT_REGISTRATION_NUMBER = tbl3.CT_REGISTRATION_NUMBER;", &the_select_node
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


			if (test_Controller_parseSelect(528, "select tbl.* from LIQUOR_LICENSES tbl;", &the_select_node
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


			if (test_Controller_parseSelect(529, "select tbl.* from LIQUOR_LICENSES tbl join LIQUOR_LICENSES tbl2 on tbl.BRAND_NAME = tbl2.BRAND_NAME;", &the_select_node
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

					col_arr[index]->col_ptr = the_select_node->columns_arr[i];
					col_arr[index]->col_ptr_type = PTR_TYPE_COL_IN_SELECT_NODE;

					col_arr[index]->rows_data_type = the_select_node->columns_arr[i]->rows_data_type;
				}
				else
				{
					col_arr[index]->table_ptr = joined_select;
					col_arr[index]->table_ptr_type = PTR_TYPE_SELECT_NODE;

					col_arr[index]->col_ptr = ((struct select_node*) joined_select)->columns_arr[i-7];
					col_arr[index]->col_ptr_type = PTR_TYPE_COL_IN_SELECT_NODE;

					col_arr[index]->rows_data_type = joined_select->columns_arr[i-7]->rows_data_type;
				}
					
				col_arr[index]->new_name = myMalloc(sizeof(char) * 10, NULL, &malloced_head, the_debug);
				sprintf(col_arr[index]->new_name, "COL_%d", i);	

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


			if (test_Controller_parseSelect(530, "select * as COL_# from LIQUOR_LICENSES tbl join LIQUOR_LICENSES tbl2 on tbl.BRAND_NAME = tbl2.BRAND_NAME;", &the_select_node
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


			if (test_Controller_parseSelect(531, "select * from ( select * from LIQUOR_LICENSES ) tbl;", &the_select_node
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

					col_arr[index]->rows_data_type = the_select_node->next->columns_arr[0]->rows_data_type;

					col_arr[index]->new_name = upper("COL1", NULL, &malloced_head, the_debug);
				}
				else if (i == 1)
				{
					col_arr[index]->table_ptr = joined_select;
					col_arr[index]->table_ptr_type = PTR_TYPE_SELECT_NODE;

					col_arr[index]->col_ptr = joined_select->columns_arr[0];
					col_arr[index]->col_ptr_type = PTR_TYPE_COL_IN_SELECT_NODE;

					col_arr[index]->rows_data_type = joined_select->columns_arr[0]->rows_data_type;

					col_arr[index]->new_name = upper("COL2", NULL, &malloced_head, the_debug);
				}
				else if (i == 2)
				{
					col_arr[index]->table_ptr = the_select_node->next;
					col_arr[index]->table_ptr_type = PTR_TYPE_SELECT_NODE;

					col_arr[index]->col_ptr = the_select_node->next->columns_arr[1];
					col_arr[index]->col_ptr_type = PTR_TYPE_COL_IN_SELECT_NODE;

					col_arr[index]->rows_data_type = the_select_node->next->columns_arr[1]->rows_data_type;

					col_arr[index]->new_name = upper("COL3", NULL, &malloced_head, the_debug);
				}
				else if (i == 3)
				{
					col_arr[index]->table_ptr = joined_select;
					col_arr[index]->table_ptr_type = PTR_TYPE_SELECT_NODE;

					col_arr[index]->col_ptr = joined_select->columns_arr[1];
					col_arr[index]->col_ptr_type = PTR_TYPE_COL_IN_SELECT_NODE;

					col_arr[index]->rows_data_type = joined_select->columns_arr[1]->rows_data_type;

					col_arr[index]->new_name = upper("COL4", NULL, &malloced_head, the_debug);
				}
				
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

			
			if (test_Controller_parseSelect(532, "select tbl.BRAND_NAME COL1, tbl2.BRAND_NAME COL2, tbl.CT_REGISTRATION_NUMBER COL3, tbl2.CT_REGISTRATION_NUMBER COL4 from ( select * from LIQUOR_LICENSES ) tbl join LIQUOR_LICENSES tbl2 on tbl2.BRAND_NAME = tbl.BRAND_NAME;", &the_select_node
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

					col_arr[index]->rows_data_type = the_select_node->columns_arr[0]->rows_data_type;

					col_arr[index]->new_name = upper("COL1", NULL, &malloced_head, the_debug);
				}
				else if (i == 1)
				{
					col_arr[index]->table_ptr = the_select_node;
					col_arr[index]->table_ptr_type = PTR_TYPE_SELECT_NODE;

					col_arr[index]->col_ptr = the_select_node->columns_arr[5];
					col_arr[index]->col_ptr_type = PTR_TYPE_COL_IN_SELECT_NODE;

					col_arr[index]->rows_data_type = the_select_node->columns_arr[5]->rows_data_type;

					col_arr[index]->new_name = upper("COL2", NULL, &malloced_head, the_debug);
				}
				else if (i > 1)
				{
					col_arr[index]->table_ptr = joined_select->next;
					col_arr[index]->table_ptr_type = PTR_TYPE_SELECT_NODE;

					col_arr[index]->col_ptr = joined_select->next->columns_arr[i-2];
					col_arr[index]->col_ptr_type = PTR_TYPE_COL_IN_SELECT_NODE;

					col_arr[index]->rows_data_type = joined_select->next->columns_arr[i-2]->rows_data_type;

					col_arr[index]->new_name = myMalloc(sizeof(char) * 10, NULL, &malloced_head, the_debug);
					sprintf(col_arr[index]->new_name, "COL_%c", i+65);
				}

				
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


			if (test_Controller_parseSelect(533, "select tbl.BRAND_NAME COL1, tbl.OUT_OF_STATE_SHIPPER COL2, tbl2.* as COL_@ from LIQUOR_LICENSES tbl join ( select * from LIQUOR_LICENSES ) tbl2 on tbl.BRAND_NAME = tbl2.BRAND_NAME;", &the_select_node
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

					col_arr[i]->rows_data_type = the_select_node->columns_arr[1]->rows_data_type;
				}
				else
				{
					col_arr[i]->col_ptr = ((struct select_node*) the_select_node)->columns_arr[0];
					col_arr[i]->col_ptr_type = PTR_TYPE_COL_IN_SELECT_NODE;

					col_arr[i]->rows_data_type = the_select_node->columns_arr[0]->rows_data_type;
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

					col_arr_2[i]->rows_data_type = the_select_node->next->columns_arr[1]->rows_data_type;
				}
				else
				{
					col_arr_2[i]->col_ptr = ((struct select_node*) the_select_node->next)->columns_arr[0];
					col_arr_2[i]->col_ptr_type = PTR_TYPE_COL_IN_SELECT_NODE;

					col_arr_2[i]->rows_data_type = the_select_node->next->columns_arr[0]->rows_data_type;
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

					col_arr_3[i]->rows_data_type = the_select_node->next->next->columns_arr[0]->rows_data_type;
				}
				else
				{
					col_arr_3[i]->col_ptr = ((struct select_node*) the_select_node->next->next)->columns_arr[1];
					col_arr_3[i]->col_ptr_type = PTR_TYPE_COL_IN_SELECT_NODE;

					col_arr_3[i]->rows_data_type = the_select_node->next->next->columns_arr[1]->rows_data_type;
				}

				col_arr_3[i]->new_name = NULL;
				col_arr_3[i]->func_node = NULL;
				col_arr_3[i]->math_node = NULL;
				col_arr_3[i]->case_node = NULL;
			}

			initSelectClauseForComp(&the_select_node->next->next->next, NULL, false, 2, col_arr_3, NULL, -1, NULL, NULL
								   ,&malloced_head, the_debug);
			the_select_node->next->next->next->prev = the_select_node->next->next;


			if (test_Controller_parseSelect(534, "select * from ( select BRAND_NAME,CT_REGISTRATION_NUMBER from ( select CT_REGISTRATION_NUMBER, BRAND_NAME from LIQUOR_LICENSES ) tbl2 ) tbl;", &the_select_node
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


			if (test_Controller_parseSelect(535, "select tbl.BRAND_NAME, tbl2.BRAND_NAME, tbl.STATUS, tbl2.EFFECTIVE from LIQUOR_LICENSES tbl join ( select BRAND_NAME, STATUS from LIQUOR_LICENSES ) tbl2 on tbl.BRAND_NAME = tbl2.BRAND_NAME;", &the_select_node
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


			if (test_Controller_parseSelect(536, "select tbl.BRAND_NAME, tbl2.BRAND_NAME, tbl.STATUS, tbl.EFFECTIVE from LIQUOR_LICENSES tbl join ( select BRAND_NAME, STATUS, awiodjbaoijd from LIQUOR_LICENSES ) tbl2 on tbl.BRAND_NAME = tbl2.BRAND_NAME;", &the_select_node
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

					col_arr[index]->rows_data_type = the_select_node->columns_arr[i]->rows_data_type;
				}
				else if (i < 14)
				{
					col_arr[index]->table_ptr = joined_select->next;
					col_arr[index]->table_ptr_type = PTR_TYPE_SELECT_NODE;

					col_arr[index]->col_ptr = joined_select->next->columns_arr[i-7];
					col_arr[index]->col_ptr_type = PTR_TYPE_COL_IN_SELECT_NODE;

					col_arr[index]->rows_data_type = joined_select->next->columns_arr[i-7]->rows_data_type;
				}
				else
				{
					col_arr[index]->table_ptr = joined_select_2;
					col_arr[index]->table_ptr_type = PTR_TYPE_SELECT_NODE;

					col_arr[index]->col_ptr = joined_select_2->columns_arr[i-14];
					col_arr[index]->col_ptr_type = PTR_TYPE_COL_IN_SELECT_NODE;

					col_arr[index]->rows_data_type = joined_select_2->columns_arr[i-14]->rows_data_type;
				}

				col_arr[index]->new_name = myMalloc(sizeof(char) * 10, NULL, &malloced_head, the_debug);
				sprintf(col_arr[index]->new_name, "COL_%c", i+65);

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


			if (test_Controller_parseSelect(537, "select * as COL_@ from LIQUOR_LICENSES tbl join ( select * from LIQUOR_LICENSES ) tbl2 on tbl.BRAND_NAME = tbl2.BRAND_NAME left join LIQUOR_LICENSES tbl3 on tbl.BRAND_NAME = tbl3.BRAND_NAME;", &the_select_node
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


			if (test_Controller_parseSelect(538, "select count(*), BRAND_NAME, CT_REGISTRATION_NUMBER, EFFECTIVE from LIQUOR_LICENSES group by BRAND_NAME, CT_REGISTRATION_NUMBER", &the_select_node
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


			if (test_Controller_parseSelect(539, "select count(*), 10 * (10 - CT_REGISTRATION_NUMBER), BRAND_NAME, EFFECTIVE from LIQUOR_LICENSES group by BRAND_NAME, EFFECTIVE", &the_select_node
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

					col_arr[i]->rows_data_type = col_arr[i]->case_node->case_then_value_head->ptr_type;
				}
				else
				{
					col_arr[i]->table_ptr = the_select_node;
					col_arr[i]->table_ptr_type = PTR_TYPE_SELECT_NODE;

					col_arr[i]->col_ptr = the_select_node->columns_arr[1];
					col_arr[i]->col_ptr_type = PTR_TYPE_COL_IN_SELECT_NODE;

					col_arr[i]->rows_data_type = the_select_node->columns_arr[1]->rows_data_type;

					col_arr[i]->new_name = NULL;

					col_arr[i]->case_node = NULL;
				}

				col_arr[i]->func_node = NULL;
				col_arr[i]->math_node = NULL;
			}

			initSelectClauseForComp(&the_select_node->next, NULL, false, 2, col_arr, NULL, -1, NULL, NULL
								   ,&malloced_head, the_debug);
			the_select_node->next->prev = the_select_node;


			if (test_Controller_parseSelect(540, "select case when tbl    . BRAND_NAME = 'Hi' then 'Is Hi' else 'Is Not Hi' end New_case, tbl . CT_REGISTRATION_NUMBER from LIQUOR_LICENSES tbl;", &the_select_node
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

					col_arr[i]->rows_data_type = col_arr[i]->case_node->case_then_value_head->ptr_type;
				}
				else
				{
					col_arr[i]->table_ptr = the_select_node;
					col_arr[i]->table_ptr_type = PTR_TYPE_SELECT_NODE;

					col_arr[i]->col_ptr = the_select_node->columns_arr[1];
					col_arr[i]->col_ptr_type = PTR_TYPE_COL_IN_SELECT_NODE;

					col_arr[i]->rows_data_type = the_select_node->columns_arr[1]->rows_data_type;

					col_arr[i]->new_name = NULL;

					col_arr[i]->case_node = NULL;
				}

				col_arr[i]->func_node = NULL;
				col_arr[i]->math_node = NULL;
			}

			initSelectClauseForComp(&the_select_node->next, NULL, false, 2, col_arr, NULL, -1, NULL, NULL
								   ,&malloced_head, the_debug);
			the_select_node->next->prev = the_select_node;


			if (test_Controller_parseSelect(541, "select case when tbl. BRAND_NAME = 'Hi' then 'Is Hi' when BRAND_NAME = 'Hello' then 'Is hello' when EXPIRATION = '1/1/2024' then 'Is date' else 'Is Something Else' end New_case, tbl . CT_REGISTRATION_NUMBER from LIQUOR_LICENSES tbl;", &the_select_node
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

			if (test_Controller_parseSelect(542, "select case when tbl. BRAND_NAME = 'Hi' then 'Is Hi' when BRAND_NAME = 'Hello' then 1000 when EXPIRATION = '1/1/2024' then 'Is date' else 'Is Something Else' end New_case, tbl . CT_REGISTRATION_NUMBER from LIQUOR_LICENSES tbl;", &the_select_node
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
					math1->result_type = PTR_TYPE_INT;

					int* math2 = (int*) myMalloc(sizeof(int), NULL, &malloced_head, the_debug);
					*math2 = 10;

					addListNodePtr(&col_arr[i]->case_node->case_then_value_head, &col_arr[i]->case_node->case_then_value_tail, math1, PTR_TYPE_MATH_NODE, ADDLISTNODE_TAIL
								  ,NULL, &malloced_head, the_debug);
					addListNodePtr(&col_arr[i]->case_node->case_then_value_head, &col_arr[i]->case_node->case_then_value_tail, math2, PTR_TYPE_INT, ADDLISTNODE_TAIL
								  ,NULL, &malloced_head, the_debug);

					col_arr[i]->rows_data_type = PTR_TYPE_INT;
				}
				else
				{
					col_arr[i]->table_ptr = the_select_node;
					col_arr[i]->table_ptr_type = PTR_TYPE_SELECT_NODE;

					col_arr[i]->col_ptr = the_select_node->columns_arr[1];
					col_arr[i]->col_ptr_type = PTR_TYPE_COL_IN_SELECT_NODE;

					col_arr[i]->rows_data_type = the_select_node->columns_arr[1]->rows_data_type;

					col_arr[i]->new_name = NULL;

					col_arr[i]->case_node = NULL;
				}

				col_arr[i]->func_node = NULL;
				col_arr[i]->math_node = NULL;
			}

			initSelectClauseForComp(&the_select_node->next, NULL, false, 2, col_arr, NULL, -1, NULL, NULL
								   ,&malloced_head, the_debug);
			the_select_node->next->prev = the_select_node;


			if (test_Controller_parseSelect(543, "select case when tbl    . BRAND_NAME = 'Hi' then (CT_REGISTRATION_NUMBER + 10) else 10 end New_case, tbl . CT_REGISTRATION_NUMBER from LIQUOR_LICENSES tbl;", &the_select_node
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

			col_arr[4]->rows_data_type = the_select_node->columns_arr[0]->rows_data_type;


			col_arr[0]->new_name = (char*) myMalloc(sizeof(char) * 16, NULL, &malloced_head, the_debug);
			strcpy(col_arr[0]->new_name, "TEN");

			int* ten = (int*) myMalloc(sizeof(int), NULL, &malloced_head, the_debug);
			*ten = 10;
			addListNodePtr(&col_arr[0]->case_node->case_then_value_head, &col_arr[0]->case_node->case_then_value_tail, ten, PTR_TYPE_INT, ADDLISTNODE_TAIL
						  ,NULL, &malloced_head, the_debug);

			col_arr[0]->rows_data_type = col_arr[0]->case_node->case_then_value_tail->ptr_type;

			col_arr[1]->new_name = (char*) myMalloc(sizeof(char) * 16, NULL, &malloced_head, the_debug);
			strcpy(col_arr[1]->new_name, "SUP_STR");

			char* sup_str = (char*) myMalloc(sizeof(char) * 16, NULL, &malloced_head, the_debug);
			strcpy(sup_str, "Sup");
			addListNodePtr(&col_arr[1]->case_node->case_then_value_head, &col_arr[1]->case_node->case_then_value_tail, sup_str, PTR_TYPE_CHAR, ADDLISTNODE_TAIL
						  ,NULL, &malloced_head, the_debug);

			col_arr[1]->rows_data_type = col_arr[1]->case_node->case_then_value_tail->ptr_type;

			col_arr[2]->new_name = (char*) myMalloc(sizeof(char) * 32, NULL, &malloced_head, the_debug);
			strcpy(col_arr[2]->new_name, "Another Col");

			double* frac = (double*) myMalloc(sizeof(double), NULL, &malloced_head, the_debug);
			*frac = 1.182973;
			addListNodePtr(&col_arr[2]->case_node->case_then_value_head, &col_arr[2]->case_node->case_then_value_tail, frac, PTR_TYPE_REAL, ADDLISTNODE_TAIL
						  ,NULL, &malloced_head, the_debug);

			col_arr[2]->rows_data_type = col_arr[2]->case_node->case_then_value_tail->ptr_type;

			col_arr[3]->new_name = (char*) myMalloc(sizeof(char) * 16, NULL, &malloced_head, the_debug);
			strcpy(col_arr[3]->new_name, "ONE_MORE");

			char* one_more = (char*) myMalloc(sizeof(char) * 16, NULL, &malloced_head, the_debug);
			strcpy(one_more, "2/4/2024");
			addListNodePtr(&col_arr[3]->case_node->case_then_value_head, &col_arr[3]->case_node->case_then_value_tail, one_more, PTR_TYPE_DATE, ADDLISTNODE_TAIL
						  ,NULL, &malloced_head, the_debug);

			col_arr[3]->rows_data_type = col_arr[3]->case_node->case_then_value_tail->ptr_type;


			initSelectClauseForComp(&the_select_node->next, NULL, false, 5, col_arr, NULL, -1, NULL, NULL
								   ,&malloced_head, the_debug);
			the_select_node->next->prev = the_select_node;


			if (test_Controller_parseSelect(544, "select 10 Ten, 'Sup' Sup_Str, 1.182973 \"Another Col\", '2/4/2024' ONE_MORE, BRAND_NAME from LIQUOR_LICENSES;", &the_select_node
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


			if (test_Controller_parseSelect(545, "select * from ( select * from LIQUOR_LICENSES );", &the_select_node
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


			if (test_Controller_parseSelect(546, "select * from ( select * from LIQUOR_LICENSES ) tbl join ( select * from LIQUOR_LICENSES )  on tbl.BRAND_NAME = BRAND_NAME;", &the_select_node
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


			if (test_Controller_parseSelect(547, "with tbl as ( select * from LIQUOR_LICENSES ) select * from tbl;", &the_select_node
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


			if (test_Controller_parseSelect(548, "with tbl as ( select * from LIQUOR_LICENSES ), tbl2 as (select * from tbl) select * from tbl2;", &the_select_node
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


			if (test_Controller_parseSelect(549, "with tbl as ( with tbl3 as (select * from LIQUOR_LICENSES) select * from tbl3 ), tbl2 as (select * from tbl) select * from tbl2;", &the_select_node
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

					col_arr[index]->rows_data_type = the_select_node->next->columns_arr[i]->rows_data_type;
				}
				else
				{
					col_arr[index]->table_ptr = joined_select->next;
					col_arr[index]->table_ptr_type = PTR_TYPE_SELECT_NODE;

					col_arr[index]->col_ptr = joined_select->next->columns_arr[i-7];
					col_arr[index]->col_ptr_type = PTR_TYPE_COL_IN_SELECT_NODE;

					col_arr[index]->rows_data_type = joined_select->next->columns_arr[i-7]->rows_data_type;
				}

				if (i == 0)
					col_arr[index]->new_name = upper("BRAND_NAME_0_A", NULL, &malloced_head, the_debug);
				else if (i == 1)
					col_arr[index]->new_name = upper("CT_REGISTRATION_NUMBER_1_B", NULL, &malloced_head, the_debug);
				else if (i == 2)
					col_arr[index]->new_name = upper("STATUS_2_C", NULL, &malloced_head, the_debug);
				else if (i == 3)
					col_arr[index]->new_name = upper("EFFECTIVE_3_D", NULL, &malloced_head, the_debug);
				else if (i == 4)
					col_arr[index]->new_name = upper("EXPIRATION_4_E", NULL, &malloced_head, the_debug);
				else if (i == 5)
					col_arr[index]->new_name = upper("OUT_OF_STATE_SHIPPER_5_F", NULL, &malloced_head, the_debug);
				else if (i == 6)
					col_arr[index]->new_name = upper("SUPERVISOR_CREDENTIAL_6_G", NULL, &malloced_head, the_debug);
				else if (i == 7)
					col_arr[index]->new_name = upper("BRAND_NAME_7_H", NULL, &malloced_head, the_debug);
				else if (i == 8)
					col_arr[index]->new_name = upper("CT_REGISTRATION_NUMBER_8_I", NULL, &malloced_head, the_debug);
				else if (i == 9)
					col_arr[index]->new_name = upper("STATUS_9_J", NULL, &malloced_head, the_debug);
				else if (i == 10)
					col_arr[index]->new_name = upper("EFFECTIVE_10_K", NULL, &malloced_head, the_debug);
				else if (i == 11)
					col_arr[index]->new_name = upper("EXPIRATION_11_L", NULL, &malloced_head, the_debug);
				else if (i == 12)
					col_arr[index]->new_name = upper("OUT_OF_STATE_SHIPPER_12_M", NULL, &malloced_head, the_debug);
				else if (i == 13)
					col_arr[index]->new_name = upper("SUPERVISOR_CREDENTIAL_13_N", NULL, &malloced_head, the_debug);

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


			if (test_Controller_parseSelect(550, "with tbl as ( select * from LIQUOR_LICENSES ), tbl2 as (select * from LIQUOR_LICENSES) select * as \"+_#_@\" from tbl join tbl2 on tbl.BRAND_NAME = tbl2.BRAND_NAME;", &the_select_node
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

					col_arr[index]->rows_data_type = the_select_node->next->columns_arr[i]->rows_data_type;
				}
				else
				{
					col_arr[index]->table_ptr = joined_select->next;
					col_arr[index]->table_ptr_type = PTR_TYPE_SELECT_NODE;

					col_arr[index]->col_ptr = joined_select->next->columns_arr[i-7];
					col_arr[index]->col_ptr_type = PTR_TYPE_COL_IN_SELECT_NODE;

					col_arr[index]->rows_data_type = joined_select->next->columns_arr[i-7]->rows_data_type;
				}

				if (i == 0)
					col_arr[index]->new_name = upper("BRAND_NAME_0_A", NULL, &malloced_head, the_debug);
				else if (i == 1)
					col_arr[index]->new_name = upper("CT_REGISTRATION_NUMBER_1_B", NULL, &malloced_head, the_debug);
				else if (i == 2)
					col_arr[index]->new_name = upper("STATUS_2_C", NULL, &malloced_head, the_debug);
				else if (i == 3)
					col_arr[index]->new_name = upper("EFFECTIVE_3_D", NULL, &malloced_head, the_debug);
				else if (i == 4)
					col_arr[index]->new_name = upper("EXPIRATION_4_E", NULL, &malloced_head, the_debug);
				else if (i == 5)
					col_arr[index]->new_name = upper("OUT_OF_STATE_SHIPPER_5_F", NULL, &malloced_head, the_debug);
				else if (i == 6)
					col_arr[index]->new_name = upper("SUPERVISOR_CREDENTIAL_6_G", NULL, &malloced_head, the_debug);
				else if (i == 7)
					col_arr[index]->new_name = upper("BRAND_NAME_7_H", NULL, &malloced_head, the_debug);
				else if (i == 8)
					col_arr[index]->new_name = upper("CT_REGISTRATION_NUMBER_8_I", NULL, &malloced_head, the_debug);
				else if (i == 9)
					col_arr[index]->new_name = upper("STATUS_9_J", NULL, &malloced_head, the_debug);
				else if (i == 10)
					col_arr[index]->new_name = upper("EFFECTIVE_10_K", NULL, &malloced_head, the_debug);
				else if (i == 11)
					col_arr[index]->new_name = upper("EXPIRATION_11_L", NULL, &malloced_head, the_debug);
				else if (i == 12)
					col_arr[index]->new_name = upper("OUT_OF_STATE_SHIPPER_12_M", NULL, &malloced_head, the_debug);
				else if (i == 13)
					col_arr[index]->new_name = upper("SUPERVISOR_CREDENTIAL_13_N", NULL, &malloced_head, the_debug);

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


			if (test_Controller_parseSelect(551, "with tbl2 as (select * from LIQUOR_LICENSES) select * as \"+_#_@\" from ( select * from LIQUOR_LICENSES )tbl join tbl2 on tbl.BRAND_NAME = tbl2.BRAND_NAME;", &the_select_node
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


			if (test_Controller_parseSelect(552, "with tbl_bad as ( select * from LIQUOR_LICENSES ) select * from LIQUOR_LICENSES;", &the_select_node
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

					col_arr[index]->rows_data_type = the_select_node->next->columns_arr[i]->rows_data_type;
				}
				else
				{
					col_arr[index]->table_ptr = joined_select->next;
					col_arr[index]->table_ptr_type = PTR_TYPE_SELECT_NODE;

					col_arr[index]->col_ptr = joined_select->next->columns_arr[i-7];
					col_arr[index]->col_ptr_type = PTR_TYPE_COL_IN_SELECT_NODE;

					col_arr[index]->rows_data_type = joined_select->next->columns_arr[i-7]->rows_data_type;
				}

				col_arr[index]->new_name = NULL;
				col_arr[index]->func_node = NULL;
				col_arr[index]->math_node = NULL;
				col_arr[index]->case_node = NULL;

				index++;
			}

			col_arr[0]->new_name = upper("BRAND_NAME_0_A", NULL, &malloced_head, the_debug);
			col_arr[1]->new_name = upper("CT_REGISTRATION_NUMBER_1_A", NULL, &malloced_head, the_debug);
			col_arr[2]->new_name = upper("STATUS_2_A", NULL, &malloced_head, the_debug);
			col_arr[3]->new_name = upper("EFFECTIVE_3_A", NULL, &malloced_head, the_debug);
			col_arr[4]->new_name = upper("EXPIRATION_4_A", NULL, &malloced_head, the_debug);
			col_arr[5]->new_name = upper("OUT_OF_STATE_SHIPPER_5_A", NULL, &malloced_head, the_debug);
			col_arr[6]->new_name = upper("SUPERVISOR_CREDENTIAL_6_A", NULL, &malloced_head, the_debug);
			col_arr[7]->new_name = upper("BRAND_NAME_7_A", NULL, &malloced_head, the_debug);
			col_arr[8]->new_name = upper("CT_REGISTRATION_NUMBER_8_A", NULL, &malloced_head, the_debug);
			col_arr[9]->new_name = upper("STATUS_9_A", NULL, &malloced_head, the_debug);
			col_arr[10]->new_name = upper("EFFECTIVE_10_A", NULL, &malloced_head, the_debug);
			col_arr[11]->new_name = upper("EXPIRATION_11_A", NULL, &malloced_head, the_debug);
			col_arr[12]->new_name = upper("OUT_OF_STATE_SHIPPER_12_A", NULL, &malloced_head, the_debug);
			col_arr[13]->new_name = upper("SUPERVISOR_CREDENTIAL_13_A", NULL, &malloced_head, the_debug);

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


			if (test_Controller_parseSelect(553, "with tbl2 as (select * from LIQUOR_LICENSES),tbl_bad as (select * from LIQUOR_LICENSES) select * as \"+_#_A\" from ( select * from LIQUOR_LICENSES )tbl join tbl2 on tbl.BRAND_NAME = tbl2.BRAND_NAME;", &the_select_node
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


			if (test_Controller_parseSelect(554, "select * from LIQUOR_LICENSES order by BRAND_NAME asc;", &the_select_node
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
			col_arr[0]->rows_data_type = ((struct select_node*) the_select_node)->columns_arr[0]->rows_data_type;

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


			if (test_Controller_parseSelect(555, "select BRAND_NAME as col1 from LIQUOR_LICENSES order by col1 asc;", &the_select_node
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


			if (test_Controller_parseSelect(556, "select * from LIQUOR_LICENSES order by BRAND_NAME asc, STATUS desc, SUPERVISOR_CREDENTIAL;", &the_select_node
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


			if (test_Controller_parseSelect(557, "select * from LIQUOR_LICENSES order by BRAND_NAME, STATUS desc, SUPERVISOR_CREDENTIAL asc;", &the_select_node
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


			if (test_Controller_parseSelect(558, "select * from LIQUOR_LICENSES order by BRAND_NAME, desc, SUPERVISOR_CREDENTIAL asc", &the_select_node
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


			if (test_Controller_parseSelect(559, "select * from LIQUOR_LICENSES order by BRAND_NAME, desc SUPERVISOR_CREDENTIAL asc", &the_select_node
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


			if (test_Controller_parseSelect(560, "select * from LIQUOR_LICENSES order by COL1, SUPERVISOR_CREDENTIAL asc", &the_select_node
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
			col_arr[0]->new_name = upper("CNT", NULL, &malloced_head, the_debug);
			col_arr[0]->math_node = NULL;
			col_arr[0]->case_node = NULL;

			col_arr[0]->func_node = (struct func_node*) myMalloc(sizeof(struct func_node), NULL, &malloced_head, the_debug);
			col_arr[0]->func_node->which_func = FUNC_COUNT;
			col_arr[0]->func_node->distinct = false;
			col_arr[0]->func_node->args_size = 7;
			col_arr[0]->func_node->args_arr = (void**) myMalloc(sizeof(void*) * 7, NULL, &malloced_head, the_debug);
			col_arr[0]->func_node->args_arr_type = (int*) myMalloc(sizeof(int) * 7, NULL, &malloced_head, the_debug);
			for (int a=0; a<7; a++)
			{
				col_arr[0]->func_node->args_arr[a] = the_select_node->columns_arr[a];
				col_arr[0]->func_node->args_arr_type[a] = PTR_TYPE_COL_IN_SELECT_NODE;
			}
			col_arr[0]->func_node->group_by_cols_head = NULL;
			col_arr[0]->func_node->group_by_cols_tail = NULL;
			addListNodePtr(&col_arr[0]->func_node->group_by_cols_head, &col_arr[0]->func_node->group_by_cols_tail
						  ,((struct select_node*) the_select_node)->columns_arr[0], PTR_TYPE_COL_IN_SELECT_NODE, ADDLISTNODE_TAIL
				  		  ,NULL, &malloced_head, the_debug);
			col_arr[0]->func_node->result_type = PTR_TYPE_INT;

			col_arr[0]->rows_data_type = col_arr[0]->func_node->result_type;

			col_arr[1] = (struct col_in_select_node*) myMalloc(sizeof(struct col_in_select_node), NULL, &malloced_head, the_debug);
			col_arr[1]->table_ptr = the_select_node;
			col_arr[1]->table_ptr_type = PTR_TYPE_SELECT_NODE;
			col_arr[1]->col_ptr = ((struct select_node*) the_select_node)->columns_arr[0];
			col_arr[1]->col_ptr_type = PTR_TYPE_COL_IN_SELECT_NODE;
			col_arr[1]->new_name = NULL;
			col_arr[1]->math_node = NULL;
			col_arr[1]->func_node = NULL;
			col_arr[1]->case_node = NULL;
			col_arr[1]->rows_data_type = ((struct select_node*) the_select_node)->columns_arr[0]->rows_data_type;

			initSelectClauseForComp(&the_select_node->next, NULL, false, 2, col_arr, NULL, -1, NULL, NULL
								   ,&malloced_head, the_debug);
			the_select_node->next->prev = the_select_node;

			the_select_node->next->having_head = (struct where_clause_node*) myMalloc(sizeof(struct where_clause_node), NULL, &malloced_head, the_debug);

			the_select_node->next->having_head->ptr_one = the_select_node->next->columns_arr[0];
			the_select_node->next->having_head->ptr_one_type = PTR_TYPE_COL_IN_SELECT_NODE;

			the_select_node->next->having_head->ptr_two = (int*) myMalloc(sizeof(int), NULL, &malloced_head, the_debug);
			*((int*) the_select_node->next->having_head->ptr_two) = 1;
			the_select_node->next->having_head->ptr_two_type = PTR_TYPE_INT;

			the_select_node->next->having_head->where_type = WHERE_GREATER_THAN;
			the_select_node->next->having_head->parent = NULL;


			if (test_Controller_parseSelect(561, "select count( *) CNT,  BRAND_NAME from LIQUOR_LICENSES group by BRAND_NAME having count(*) > 1", &the_select_node
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
			col_arr[0]->new_name = upper("AVG", NULL, &malloced_head, the_debug);
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
			col_arr[0]->func_node->result_type = PTR_TYPE_REAL;
			addListNodePtr(&col_arr[0]->func_node->group_by_cols_head, &col_arr[0]->func_node->group_by_cols_tail
						  ,((struct select_node*) the_select_node)->columns_arr[0], PTR_TYPE_COL_IN_SELECT_NODE, ADDLISTNODE_TAIL
				  		  ,NULL, &malloced_head, the_debug);

			col_arr[0]->rows_data_type = col_arr[0]->func_node->result_type;

			col_arr[1] = (struct col_in_select_node*) myMalloc(sizeof(struct col_in_select_node), NULL, &malloced_head, the_debug);
			col_arr[1]->table_ptr = the_select_node;
			col_arr[1]->table_ptr_type = PTR_TYPE_SELECT_NODE;
			col_arr[1]->col_ptr = ((struct select_node*) the_select_node)->columns_arr[0];
			col_arr[1]->col_ptr_type = PTR_TYPE_COL_IN_SELECT_NODE;
			col_arr[1]->new_name = NULL;
			col_arr[1]->math_node = NULL;
			col_arr[1]->func_node = NULL;
			col_arr[1]->case_node = NULL;
			col_arr[1]->rows_data_type = ((struct select_node*) the_select_node)->columns_arr[0]->rows_data_type;

			initSelectClauseForComp(&the_select_node->next, NULL, false, 2, col_arr, NULL, -1, NULL, NULL
								   ,&malloced_head, the_debug);
			the_select_node->next->prev = the_select_node;

			the_select_node->next->having_head = (struct where_clause_node*) myMalloc(sizeof(struct where_clause_node), NULL, &malloced_head, the_debug);

			the_select_node->next->having_head->ptr_one = the_select_node->next->columns_arr[0];
			the_select_node->next->having_head->ptr_one_type = PTR_TYPE_COL_IN_SELECT_NODE;

			the_select_node->next->having_head->ptr_two = (double*) myMalloc(sizeof(double), NULL, &malloced_head, the_debug);
			*((double*) the_select_node->next->having_head->ptr_two) = 10000.0;
			the_select_node->next->having_head->ptr_two_type = PTR_TYPE_REAL;

			the_select_node->next->having_head->where_type = WHERE_IS_EQUALS;
			the_select_node->next->having_head->parent = NULL;


			if (test_Controller_parseSelect(562, "select avg( CT_REGISTRATION_NUMBER) as avg,  BRAND_NAME from LIQUOR_LICENSES group by BRAND_NAME having avg( CT_REGISTRATION_NUMBER) = 10000.0", &the_select_node
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
			col_arr[0]->new_name = upper("AVG", NULL, &malloced_head, the_debug);
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
			col_arr[0]->func_node->result_type = PTR_TYPE_REAL;
			addListNodePtr(&col_arr[0]->func_node->group_by_cols_head, &col_arr[0]->func_node->group_by_cols_tail
						  ,((struct select_node*) the_select_node)->columns_arr[0], PTR_TYPE_COL_IN_SELECT_NODE, ADDLISTNODE_TAIL
				  		  ,NULL, &malloced_head, the_debug);

			col_arr[0]->rows_data_type = col_arr[0]->func_node->result_type;

			col_arr[1] = (struct col_in_select_node*) myMalloc(sizeof(struct col_in_select_node), NULL, &malloced_head, the_debug);
			col_arr[1]->table_ptr = NULL;
			col_arr[1]->table_ptr_type = -1;
			col_arr[1]->col_ptr = NULL;
			col_arr[1]->col_ptr_type = -1;
			col_arr[1]->new_name = upper("CNT", NULL, &malloced_head, the_debug);
			col_arr[1]->math_node = NULL;
			col_arr[1]->case_node = NULL;

			col_arr[1]->func_node = (struct func_node*) myMalloc(sizeof(struct func_node), NULL, &malloced_head, the_debug);
			col_arr[1]->func_node->which_func = FUNC_COUNT;
			col_arr[1]->func_node->distinct = false;
			col_arr[1]->func_node->args_size = 7;
			col_arr[1]->func_node->args_arr = (void**) myMalloc(sizeof(void*) * 7, NULL, &malloced_head, the_debug);
			col_arr[1]->func_node->args_arr_type = (int*) myMalloc(sizeof(int) * 7, NULL, &malloced_head, the_debug);
			for (int a=0; a<7; a++)
			{
				col_arr[1]->func_node->args_arr[a] = the_select_node->columns_arr[a];
				col_arr[1]->func_node->args_arr_type[a] = PTR_TYPE_COL_IN_SELECT_NODE;
			}
			col_arr[1]->func_node->group_by_cols_head = NULL;
			col_arr[1]->func_node->group_by_cols_tail = NULL;
			col_arr[1]->func_node->result_type = PTR_TYPE_INT;
			addListNodePtr(&col_arr[1]->func_node->group_by_cols_head, &col_arr[1]->func_node->group_by_cols_tail
						  ,((struct select_node*) the_select_node)->columns_arr[0], PTR_TYPE_COL_IN_SELECT_NODE, ADDLISTNODE_TAIL
				  		  ,NULL, &malloced_head, the_debug);

			col_arr[1]->rows_data_type = col_arr[1]->func_node->result_type;

			col_arr[2] = (struct col_in_select_node*) myMalloc(sizeof(struct col_in_select_node), NULL, &malloced_head, the_debug);
			col_arr[2]->table_ptr = the_select_node;
			col_arr[2]->table_ptr_type = PTR_TYPE_SELECT_NODE;
			col_arr[2]->col_ptr = ((struct select_node*) the_select_node)->columns_arr[0];
			col_arr[2]->col_ptr_type = PTR_TYPE_COL_IN_SELECT_NODE;
			col_arr[2]->new_name = NULL;
			col_arr[2]->math_node = NULL;
			col_arr[2]->func_node = NULL;
			col_arr[2]->case_node = NULL;
			col_arr[2]->rows_data_type = ((struct select_node*) the_select_node)->columns_arr[0]->rows_data_type;

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

				((struct where_clause_node*) the_select_node->next->having_head->ptr_one)->ptr_one = the_select_node->next->columns_arr[0];
				((struct where_clause_node*) the_select_node->next->having_head->ptr_one)->ptr_one_type = PTR_TYPE_COL_IN_SELECT_NODE;

				((struct where_clause_node*) the_select_node->next->having_head->ptr_one)->ptr_two = (double*) myMalloc(sizeof(double), NULL, &malloced_head, the_debug);
				*((double*) ((struct where_clause_node*) the_select_node->next->having_head->ptr_one)->ptr_two) = 10000.0;
				((struct where_clause_node*) the_select_node->next->having_head->ptr_one)->ptr_two_type = PTR_TYPE_REAL;

				((struct where_clause_node*) the_select_node->next->having_head->ptr_one)->where_type = WHERE_IS_EQUALS;
				((struct where_clause_node*) the_select_node->next->having_head->ptr_one)->parent = the_select_node->next->having_head;

				((struct where_clause_node*) the_select_node->next->having_head->ptr_two)->ptr_one = the_select_node->next->columns_arr[1];
				((struct where_clause_node*) the_select_node->next->having_head->ptr_two)->ptr_one_type = PTR_TYPE_COL_IN_SELECT_NODE;

				((struct where_clause_node*) the_select_node->next->having_head->ptr_two)->ptr_two = (int*) myMalloc(sizeof(int), NULL, &malloced_head, the_debug);
				*((int*) ((struct where_clause_node*) the_select_node->next->having_head->ptr_two)->ptr_two) = 1;
				((struct where_clause_node*) the_select_node->next->having_head->ptr_two)->ptr_two_type = PTR_TYPE_INT;

				((struct where_clause_node*) the_select_node->next->having_head->ptr_two)->where_type = WHERE_GREATER_THAN;
				((struct where_clause_node*) the_select_node->next->having_head->ptr_two)->parent = the_select_node->next->having_head;


			if (test_Controller_parseSelect(563, "select avg( CT_REGISTRATION_NUMBER) avg,count( *) CNT, BRAND_NAME from LIQUOR_LICENSES group by BRAND_NAME having avg( CT_REGISTRATION_NUMBER) = 10000.0 and count(*) > 1", &the_select_node
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
			col_arr[0]->new_name = upper("CNT", NULL, &malloced_head, the_debug);
			col_arr[0]->math_node = NULL;
			col_arr[0]->case_node = NULL;

			col_arr[0]->func_node = (struct func_node*) myMalloc(sizeof(struct func_node), NULL, &malloced_head, the_debug);
			col_arr[0]->func_node->which_func = FUNC_COUNT;
			col_arr[0]->func_node->distinct = false;
			col_arr[0]->func_node->args_size = 7;
			col_arr[0]->func_node->args_arr = (void**) myMalloc(sizeof(void*) * 7, NULL, &malloced_head, the_debug);
			col_arr[0]->func_node->args_arr_type = (int*) myMalloc(sizeof(int) * 7, NULL, &malloced_head, the_debug);
			for (int a=0; a<7; a++)
			{
				col_arr[0]->func_node->args_arr[a] = the_select_node->columns_arr[a];
				col_arr[0]->func_node->args_arr_type[a] = PTR_TYPE_COL_IN_SELECT_NODE;
			}
			col_arr[0]->func_node->group_by_cols_head = NULL;
			col_arr[0]->func_node->group_by_cols_tail = NULL;
			col_arr[0]->func_node->result_type = PTR_TYPE_INT;
			addListNodePtr(&col_arr[0]->func_node->group_by_cols_head, &col_arr[0]->func_node->group_by_cols_tail
						  ,((struct select_node*) the_select_node)->columns_arr[0], PTR_TYPE_COL_IN_SELECT_NODE, ADDLISTNODE_TAIL
				  		  ,NULL, &malloced_head, the_debug);
			
			col_arr[0]->rows_data_type = col_arr[0]->func_node->result_type;

			col_arr[1] = (struct col_in_select_node*) myMalloc(sizeof(struct col_in_select_node), NULL, &malloced_head, the_debug);
			col_arr[1]->table_ptr = the_select_node;
			col_arr[1]->table_ptr_type = PTR_TYPE_SELECT_NODE;
			col_arr[1]->col_ptr = ((struct select_node*) the_select_node)->columns_arr[0];
			col_arr[1]->col_ptr_type = PTR_TYPE_COL_IN_SELECT_NODE;
			col_arr[1]->new_name = NULL;
			col_arr[1]->math_node = NULL;
			col_arr[1]->func_node = NULL;
			col_arr[1]->case_node = NULL;
			col_arr[1]->rows_data_type = ((struct select_node*) the_select_node)->columns_arr[0]->rows_data_type;

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
			((struct func_node*) the_select_node->next->having_head->ptr_one)->result_type = PTR_TYPE_REAL;
			the_select_node->next->having_head->ptr_one_type = PTR_TYPE_FUNC_NODE;

			the_select_node->next->having_head->ptr_two = (double*) myMalloc(sizeof(double), NULL, &malloced_head, the_debug);
			*((double*) the_select_node->next->having_head->ptr_two) = 1.0;
			the_select_node->next->having_head->ptr_two_type = PTR_TYPE_REAL;

			the_select_node->next->having_head->where_type = WHERE_GREATER_THAN;
			the_select_node->next->having_head->parent = NULL;

			if (test_Controller_parseSelect(564, "select count( *) CNT, BRAND_NAME from LIQUOR_LICENSES group by BRAND_NAME having avg(CT_REGISTRATION_NUMBER) > 1.0", &the_select_node
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


			if (test_Controller_parseSelect(565, "select avg( CT_REGISTRATION_NUMBER),count( *), BRAND_NAME from LIQUOR_LICENSES having avg( CT_REGISTRATION_NUMBER) = 10000 and count(*) > 1", &the_select_node
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


			if (test_Controller_parseSelect(566, "select count( *), BRAND_NAME from LIQUOR_LICENSES group by CT_REGISTRATION_NUMBER having count(*) > 1", &the_select_node
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


			if (test_Controller_parseSelect(567, "select count( *), BRAND_NAME from LIQUOR_LICENSES group by BRAND_NAME having avg(STATUS) > 1", &the_select_node
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
			col_arr[0]->new_name = upper("CNT", NULL, &malloced_head, the_debug);
			col_arr[0]->math_node = NULL;
			col_arr[0]->case_node = NULL;

			col_arr[0]->func_node = (struct func_node*) myMalloc(sizeof(struct func_node), NULL, &malloced_head, the_debug);
			col_arr[0]->func_node->which_func = FUNC_COUNT;
			col_arr[0]->func_node->distinct = false;
			col_arr[0]->func_node->args_size = 7;
			col_arr[0]->func_node->args_arr = (void**) myMalloc(sizeof(void*) * 7, NULL, &malloced_head, the_debug);
			col_arr[0]->func_node->args_arr_type = (int*) myMalloc(sizeof(int) * 7, NULL, &malloced_head, the_debug);
			for (int a=0; a<7; a++)
			{
				col_arr[0]->func_node->args_arr[a] = the_select_node->columns_arr[a];
				col_arr[0]->func_node->args_arr_type[a] = PTR_TYPE_COL_IN_SELECT_NODE;
			}
			col_arr[0]->func_node->group_by_cols_head = NULL;
			col_arr[0]->func_node->group_by_cols_tail = NULL;
			col_arr[0]->func_node->result_type = PTR_TYPE_INT;
			addListNodePtr(&col_arr[0]->func_node->group_by_cols_head, &col_arr[0]->func_node->group_by_cols_tail
						  ,((struct select_node*) the_select_node)->columns_arr[0], PTR_TYPE_COL_IN_SELECT_NODE, ADDLISTNODE_TAIL
				  		  ,NULL, &malloced_head, the_debug);

			col_arr[0]->rows_data_type = col_arr[0]->func_node->result_type;

			col_arr[1] = (struct col_in_select_node*) myMalloc(sizeof(struct col_in_select_node), NULL, &malloced_head, the_debug);
			col_arr[1]->table_ptr = the_select_node;
			col_arr[1]->table_ptr_type = PTR_TYPE_SELECT_NODE;
			col_arr[1]->col_ptr = ((struct select_node*) the_select_node)->columns_arr[0];
			col_arr[1]->col_ptr_type = PTR_TYPE_COL_IN_SELECT_NODE;
			col_arr[1]->new_name = NULL;
			col_arr[1]->math_node = NULL;
			col_arr[1]->func_node = NULL;
			col_arr[1]->case_node = NULL;
			col_arr[1]->rows_data_type = ((struct select_node*) the_select_node)->columns_arr[0]->rows_data_type;

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


			if (test_Controller_parseSelect(568, "select count( *) CNT, BRAND_NAME from LIQUOR_LICENSES where SUPERVISOR_CREDENTIAL <> 'LSL.0000933' group by BRAND_NAME;", &the_select_node
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
			col_arr[0]->new_name = upper("COUNT", NULL, &malloced_head, the_debug);
			col_arr[0]->math_node = NULL;
			col_arr[0]->case_node = NULL;

			col_arr[0]->func_node = (struct func_node*) myMalloc(sizeof(struct func_node), NULL, &malloced_head, the_debug);
			col_arr[0]->func_node->which_func = FUNC_COUNT;
			col_arr[0]->func_node->distinct = false;
			col_arr[0]->func_node->args_size = 7;
			col_arr[0]->func_node->args_arr = (void**) myMalloc(sizeof(void*) * 7, NULL, &malloced_head, the_debug);
			col_arr[0]->func_node->args_arr_type = (int*) myMalloc(sizeof(int) * 7, NULL, &malloced_head, the_debug);
			for (int a=0; a<7; a++)
			{
				col_arr[0]->func_node->args_arr[a] = the_select_node->columns_arr[a];
				col_arr[0]->func_node->args_arr_type[a] = PTR_TYPE_COL_IN_SELECT_NODE;
			}
			col_arr[0]->func_node->group_by_cols_head = NULL;
			col_arr[0]->func_node->group_by_cols_tail = NULL;
			col_arr[0]->func_node->result_type = PTR_TYPE_INT;
			addListNodePtr(&col_arr[0]->func_node->group_by_cols_head, &col_arr[0]->func_node->group_by_cols_tail
						  ,((struct select_node*) the_select_node)->columns_arr[0], PTR_TYPE_COL_IN_SELECT_NODE, ADDLISTNODE_TAIL
				  		  ,NULL, &malloced_head, the_debug);

			col_arr[0]->rows_data_type = col_arr[0]->func_node->result_type;

			col_arr[1] = (struct col_in_select_node*) myMalloc(sizeof(struct col_in_select_node), NULL, &malloced_head, the_debug);
			col_arr[1]->table_ptr = the_select_node;
			col_arr[1]->table_ptr_type = PTR_TYPE_SELECT_NODE;
			col_arr[1]->col_ptr = ((struct select_node*) the_select_node)->columns_arr[0];
			col_arr[1]->col_ptr_type = PTR_TYPE_COL_IN_SELECT_NODE;
			col_arr[1]->new_name = NULL;
			col_arr[1]->math_node = NULL;
			col_arr[1]->func_node = NULL;
			col_arr[1]->case_node = NULL;
			col_arr[1]->rows_data_type = ((struct select_node*) the_select_node)->columns_arr[0]->rows_data_type;

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

			the_select_node->next->having_head->ptr_one = the_select_node->next->columns_arr[0];
			the_select_node->next->having_head->ptr_one_type = PTR_TYPE_COL_IN_SELECT_NODE;

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


			if (test_Controller_parseSelect(569, "select count( *) COUNT, BRAND_NAME from LIQUOR_LICENSES where SUPERVISOR_CREDENTIAL <> 'LSL.0000933' group by BRAND_NAME having count( *) > 1 order by BRAND_NAME asc", &the_select_node
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

					col_arr[index]->rows_data_type = the_select_node->next->columns_arr[0]->rows_data_type;
				}
				else if (i == 1)
				{
					col_arr[index]->table_ptr = the_select_node->next;
					col_arr[index]->table_ptr_type = PTR_TYPE_SELECT_NODE;

					col_arr[index]->col_ptr = the_select_node->next->columns_arr[1];
					col_arr[index]->col_ptr_type = PTR_TYPE_COL_IN_SELECT_NODE;

					col_arr[index]->rows_data_type = the_select_node->next->columns_arr[1]->rows_data_type;
				}
				else if (i == 2)
				{
					col_arr[index]->table_ptr = the_select_node->next;
					col_arr[index]->table_ptr_type = PTR_TYPE_SELECT_NODE;

					col_arr[index]->col_ptr = the_select_node->next->columns_arr[6];
					col_arr[index]->col_ptr_type = PTR_TYPE_COL_IN_SELECT_NODE;

					col_arr[index]->rows_data_type = the_select_node->next->columns_arr[6]->rows_data_type;

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

			col_arr[3]->rows_data_type = col_arr[3]->case_node->case_then_value_head->ptr_type;


			col_arr[4]->table_ptr = NULL;
			col_arr[4]->table_ptr_type = -1;
			col_arr[4]->col_ptr = NULL;
			col_arr[4]->col_ptr_type = -1;
			col_arr[4]->new_name = upper("COUNT", NULL, &malloced_head, the_debug);
			col_arr[4]->math_node = NULL;
			col_arr[4]->case_node = NULL;

			col_arr[4]->func_node = (struct func_node*) myMalloc(sizeof(struct func_node), NULL, &malloced_head, the_debug);
			col_arr[4]->func_node->which_func = FUNC_COUNT;
			col_arr[4]->func_node->distinct = false;
			col_arr[4]->func_node->args_size = 14;
			col_arr[4]->func_node->args_arr = (void**) myMalloc(sizeof(void*) * 14, NULL, &malloced_head, the_debug);
			col_arr[4]->func_node->args_arr_type = (int*) myMalloc(sizeof(int) * 14, NULL, &malloced_head, the_debug);
			for (int a=0; a<14; a++)
			{
				if (a < 7)
				{
					col_arr[4]->func_node->args_arr[a] = the_select_node->next->columns_arr[a];
					col_arr[4]->func_node->args_arr_type[a] = PTR_TYPE_COL_IN_SELECT_NODE;
				}
				else
				{
					col_arr[4]->func_node->args_arr[a] = joined_select->columns_arr[a-7];
					col_arr[4]->func_node->args_arr_type[a] = PTR_TYPE_COL_IN_SELECT_NODE;
				}
			}
			col_arr[4]->func_node->group_by_cols_head = NULL;
			col_arr[4]->func_node->group_by_cols_tail = NULL;
			col_arr[4]->func_node->result_type = PTR_TYPE_INT;
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

			col_arr[4]->rows_data_type = col_arr[4]->func_node->result_type;


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

			the_select_node->next->next->having_head->ptr_one = the_select_node->next->next->columns_arr[4];
			the_select_node->next->next->having_head->ptr_one_type = PTR_TYPE_COL_IN_SELECT_NODE;

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


			
			//	select alc1.BRAND_NAME, alc1.CT_REGISTRATION_NUMBER, alc1.SUPERVISOR_CREDENTIAL  
			//		  ,case when alc2.BRAND_NAME = 'test' then 1 
			//		  		else 0	
			//		  end Test_case	
			//		  ,count(*) 
			//	from
			//	(	
			//	  	select * from LIQUOR_LICENSES
			//	) alc1 
			//	join LIQUOR_LICENSES alc2 
			//	on alc1.BRAND_NAME = alc2.BRAND_NAME 
			//	where alc1.SUPERVISOR_CREDENTIAL <> 'LSL.0000933' 
			//	group by alc1.BRAND_NAME, alc1.CT_REGISTRATION_NUMBER, alc1.SUPERVISOR_CREDENTIAL, alc2.BRAND_NAME 
			//	having count( *) > 1  
			//	order by alc1.BRAND_NAME asc
			
			if (test_Controller_parseSelect(570, "select alc1.BRAND_NAME, alc1.CT_REGISTRATION_NUMBER, alc1.SUPERVISOR_CREDENTIAL  ,case when alc2.BRAND_NAME = 'test' then 1 else 0	end Test_case	,count(*) COUNT from(	select * from LIQUOR_LICENSES) alc1 join LIQUOR_LICENSES  alc2 on alc1.BRAND_NAME = alc2.BRAND_NAME where alc1.SUPERVISOR_CREDENTIAL <> 'LSL.0000933' group by alc1.BRAND_NAME, alc1.CT_REGISTRATION_NUMBER, alc1.SUPERVISOR_CREDENTIAL, alc2.BRAND_NAME having count( *) > 1  order by alc1.BRAND_NAME asc ", &the_select_node
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

	// START Additional test_Controller_parseSelect
		// START Test with id = 571
			initSelectClauseForComp(&the_select_node, NULL, false, 0, NULL, getTablesHead(), PTR_TYPE_TABLE_INFO, NULL, NULL
								   ,&malloced_head, the_debug);

			col_arr = (struct col_in_select_node**) myMalloc(sizeof(struct col_in_select_node*) * 1, NULL, &malloced_head, the_debug);

			col_arr[0] = (struct col_in_select_node*) myMalloc(sizeof(struct col_in_select_node), NULL, &malloced_head, the_debug);

			col_arr[0]->table_ptr = NULL;
			col_arr[0]->table_ptr_type = -1;

			col_arr[0]->col_ptr = NULL;
			col_arr[0]->col_ptr_type = -1;

			col_arr[0]->new_name = (char*) myMalloc(sizeof(char) * 10, NULL, &malloced_head, the_debug);
			strcpy(col_arr[0]->new_name, "NEW_CASE");

			col_arr[0]->func_node = NULL;
			col_arr[0]->math_node = NULL;

			col_arr[0]->case_node = (struct case_node*) myMalloc(sizeof(struct case_node), NULL, &malloced_head, the_debug);
			col_arr[0]->case_node->case_when_head = NULL;
			col_arr[0]->case_node->case_then_value_head = NULL;

			struct where_clause_node* temp_where = (struct where_clause_node*) myMalloc(sizeof(struct where_clause_node), NULL, &malloced_head, the_debug);
			temp_where->ptr_one = the_select_node->columns_arr[4];
			temp_where->ptr_one_type = PTR_TYPE_COL_IN_SELECT_NODE;
			temp_where->ptr_two = (char*) myMalloc(sizeof(char) * 16, NULL, &malloced_head, the_debug);
			strcpy(temp_where->ptr_two, "1/31/2025");
			temp_where->ptr_two_type = PTR_TYPE_DATE;
			temp_where->where_type = WHERE_IS_EQUALS;
			temp_where->parent = NULL;

			addListNodePtr(&col_arr[0]->case_node->case_when_head, &col_arr[0]->case_node->case_when_tail, temp_where, PTR_TYPE_WHERE_CLAUSE_NODE, ADDLISTNODE_TAIL
						  ,NULL, &malloced_head, the_debug);
			addListNodePtr(&col_arr[0]->case_node->case_when_head, &col_arr[0]->case_node->case_when_tail, NULL, -1, ADDLISTNODE_TAIL
						  ,NULL, &malloced_head, the_debug);

			char* str1 = (char*) myMalloc(sizeof(char) * 8, NULL, &malloced_head, the_debug);
			strcpy(str1, "YES");
			char* str2 = (char*) myMalloc(sizeof(char) * 8, NULL, &malloced_head, the_debug);
			strcpy(str2, "no");

			addListNodePtr(&col_arr[0]->case_node->case_then_value_head, &col_arr[0]->case_node->case_then_value_tail, str1, PTR_TYPE_CHAR, ADDLISTNODE_TAIL
						  ,NULL, &malloced_head, the_debug);
			addListNodePtr(&col_arr[0]->case_node->case_then_value_head, &col_arr[0]->case_node->case_then_value_tail, str2, PTR_TYPE_CHAR, ADDLISTNODE_TAIL
						  ,NULL, &malloced_head, the_debug);

			col_arr[0]->rows_data_type = col_arr[0]->case_node->case_then_value_head->ptr_type;


			initSelectClauseForComp(&the_select_node->next, NULL, false, 1, col_arr, NULL, -1, NULL, NULL
								   ,&malloced_head, the_debug);
			the_select_node->next->prev = the_select_node;


			if (test_Controller_parseSelect(571, "select case when EXPIRATION = '1/31/2025' then 'YES' else 'no' end new_case from LIQUOR_LICENSES;", &the_select_node
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
		// END Test with id = 571

		// START Test with id = 572
			initSelectClauseForComp(&the_select_node, NULL, false, 0, NULL, getTablesHead(), PTR_TYPE_TABLE_INFO, NULL, NULL
								   ,&malloced_head, the_debug);

			col_arr = (struct col_in_select_node**) myMalloc(sizeof(struct col_in_select_node*) * 2, NULL, &malloced_head, the_debug);

			for (int i=0; i<2; i++)
			{
				col_arr[i] = (struct col_in_select_node*) myMalloc(sizeof(struct col_in_select_node), NULL, &malloced_head, the_debug);

				col_arr[i]->table_ptr = NULL;
				col_arr[i]->table_ptr_type = -1;

				col_arr[i]->col_ptr = NULL;
				col_arr[i]->col_ptr_type = -1;

				col_arr[i]->new_name = NULL;

				col_arr[i]->func_node = NULL;
				col_arr[i]->math_node = NULL;

				if (i == 0)
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

			col_arr[1]->table_ptr = the_select_node;
			col_arr[1]->table_ptr_type = PTR_TYPE_SELECT_NODE;

			col_arr[1]->col_ptr = the_select_node->columns_arr[0];
			col_arr[1]->col_ptr_type = PTR_TYPE_COL_IN_SELECT_NODE;

			col_arr[1]->rows_data_type = the_select_node->columns_arr[0]->rows_data_type;

			col_arr[0]->new_name = (char*) myMalloc(sizeof(char) * 16, NULL, &malloced_head, the_debug);
			strcpy(col_arr[0]->new_name, "ONE");

			char* one = (char*) myMalloc(sizeof(char) * 2, NULL, &malloced_head, the_debug);
			strcpy(one, "1");
			addListNodePtr(&col_arr[0]->case_node->case_then_value_head, &col_arr[0]->case_node->case_then_value_tail, one, PTR_TYPE_CHAR, ADDLISTNODE_TAIL
						  ,NULL, &malloced_head, the_debug);

			col_arr[0]->rows_data_type = col_arr[0]->case_node->case_then_value_head->ptr_type;

			initSelectClauseForComp(&the_select_node->next, NULL, false, 2, col_arr, NULL, -1, NULL, NULL
								   ,&malloced_head, the_debug);
			the_select_node->next->prev = the_select_node;

			the_select_node->next->where_head = (struct where_clause_node*) myMalloc(sizeof(struct where_clause_node), NULL, &malloced_head, the_debug);
			the_select_node->next->where_head->ptr_one = the_select_node->next->columns_arr[0];
			the_select_node->next->where_head->ptr_one_type = PTR_TYPE_COL_IN_SELECT_NODE;
			the_select_node->next->where_head->ptr_two = (char*) myMalloc(sizeof(char) * 2, NULL, &malloced_head, the_debug);
			strcpy(the_select_node->next->where_head->ptr_two, "2");
			the_select_node->next->where_head->ptr_two_type = PTR_TYPE_CHAR;
			the_select_node->next->where_head->where_type = WHERE_IS_EQUALS;
			the_select_node->next->where_head->parent = NULL;


			if (test_Controller_parseSelect(572, "select '1' as one, BRAND_NAME from LIQUOR_LICENSES where one = '2'", &the_select_node
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
		// END Test with id = 572

		// START Test with id = 573
			the_select_node = NULL;


			if (test_Controller_parseSelect(573, "select '1', BRAND_NAME from LIQUOR_LICENSES", &the_select_node
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
		// END Test with id = 573

		// START Test with id = 574
			the_select_node = NULL;


			if (test_Controller_parseSelect(574, "select 10 * 10, BRAND_NAME from LIQUOR_LICENSES", &the_select_node
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
		// END Test with id = 574

		// START Test with id = 575
			initSelectClauseForComp(&the_select_node, NULL, false, 0, NULL, getTablesHead(), PTR_TYPE_TABLE_INFO, NULL, NULL
								   ,&malloced_head, the_debug);

			col_arr = (struct col_in_select_node**) myMalloc(sizeof(struct col_in_select_node*) * 1, NULL, &malloced_head, the_debug);

			col_arr[0] = (struct col_in_select_node*) myMalloc(sizeof(struct col_in_select_node), NULL, &malloced_head, the_debug);

			col_arr[0]->table_ptr = NULL;
			col_arr[0]->table_ptr_type = -1;

			col_arr[0]->col_ptr = NULL;
			col_arr[0]->col_ptr_type = -1;

			col_arr[0]->new_name = (char*) myMalloc(sizeof(char) * 10, NULL, &malloced_head, the_debug);
			strcpy(col_arr[0]->new_name, "NEW_CASE");

			col_arr[0]->func_node = NULL;
			col_arr[0]->math_node = NULL;

			col_arr[0]->case_node = (struct case_node*) myMalloc(sizeof(struct case_node), NULL, &malloced_head, the_debug);
			col_arr[0]->case_node->case_when_head = NULL;
			col_arr[0]->case_node->case_then_value_head = NULL;

			temp_where = (struct where_clause_node*) myMalloc(sizeof(struct where_clause_node), NULL, &malloced_head, the_debug);
			temp_where->ptr_one = the_select_node->columns_arr[4];
			temp_where->ptr_one_type = PTR_TYPE_COL_IN_SELECT_NODE;
			temp_where->ptr_two = (char*) myMalloc(sizeof(char) * 16, NULL, &malloced_head, the_debug);
			strcpy(temp_where->ptr_two, "1/31/2025");
			temp_where->ptr_two_type = PTR_TYPE_DATE;
			temp_where->where_type = WHERE_IS_EQUALS;
			temp_where->parent = NULL;

			struct where_clause_node* temp_where_2 = (struct where_clause_node*) myMalloc(sizeof(struct where_clause_node), NULL, &malloced_head, the_debug);
			temp_where_2->ptr_one = the_select_node->columns_arr[4];
			temp_where_2->ptr_one_type = PTR_TYPE_COL_IN_SELECT_NODE;
			temp_where_2->ptr_two = (char*) myMalloc(sizeof(char) * 16, NULL, &malloced_head, the_debug);
			strcpy(temp_where_2->ptr_two, "3/16/2025");
			temp_where_2->ptr_two_type = PTR_TYPE_DATE;
			temp_where_2->where_type = WHERE_IS_EQUALS;
			temp_where_2->parent = NULL;

			addListNodePtr(&col_arr[0]->case_node->case_when_head, &col_arr[0]->case_node->case_when_tail, temp_where, PTR_TYPE_WHERE_CLAUSE_NODE, ADDLISTNODE_TAIL
						  ,NULL, &malloced_head, the_debug);
			addListNodePtr(&col_arr[0]->case_node->case_when_head, &col_arr[0]->case_node->case_when_tail, temp_where_2, PTR_TYPE_WHERE_CLAUSE_NODE, ADDLISTNODE_TAIL
						  ,NULL, &malloced_head, the_debug);
			addListNodePtr(&col_arr[0]->case_node->case_when_head, &col_arr[0]->case_node->case_when_tail, NULL, -1, ADDLISTNODE_TAIL
						  ,NULL, &malloced_head, the_debug);

			str1 = (char*) myMalloc(sizeof(char) * 8, NULL, &malloced_head, the_debug);
			strcpy(str1, "yes");
			str2 = (char*) myMalloc(sizeof(char) * 8, NULL, &malloced_head, the_debug);
			strcpy(str2, "maybe");
			char* str3 = (char*) myMalloc(sizeof(char) * 8, NULL, &malloced_head, the_debug);
			strcpy(str3, "NO");

			addListNodePtr(&col_arr[0]->case_node->case_then_value_head, &col_arr[0]->case_node->case_then_value_tail, str1, PTR_TYPE_CHAR, ADDLISTNODE_TAIL
						  ,NULL, &malloced_head, the_debug);
			addListNodePtr(&col_arr[0]->case_node->case_then_value_head, &col_arr[0]->case_node->case_then_value_tail, str2, PTR_TYPE_CHAR, ADDLISTNODE_TAIL
						  ,NULL, &malloced_head, the_debug);
			addListNodePtr(&col_arr[0]->case_node->case_then_value_head, &col_arr[0]->case_node->case_then_value_tail, str3, PTR_TYPE_CHAR, ADDLISTNODE_TAIL
						  ,NULL, &malloced_head, the_debug);

			col_arr[0]->rows_data_type = col_arr[0]->case_node->case_then_value_head->ptr_type;


			initSelectClauseForComp(&the_select_node->next, "TBL", false, 1, col_arr, NULL, -1, NULL, NULL
								   ,&malloced_head, the_debug);
			the_select_node->next->prev = the_select_node;

			col_arr_2 = (struct col_in_select_node**) myMalloc(sizeof(struct col_in_select_node*) * 1, NULL, &malloced_head, the_debug);

			col_arr_2[0] = (struct col_in_select_node*) myMalloc(sizeof(struct col_in_select_node), NULL, &malloced_head, the_debug);

			col_arr_2[0]->table_ptr = the_select_node->next;
			col_arr_2[0]->table_ptr_type = PTR_TYPE_SELECT_NODE;

			col_arr_2[0]->col_ptr = the_select_node->next->columns_arr[0];
			col_arr_2[0]->col_ptr_type = PTR_TYPE_COL_IN_SELECT_NODE;

			col_arr_2[0]->rows_data_type = the_select_node->next->columns_arr[0]->rows_data_type;

			col_arr_2[0]->new_name = NULL;

			col_arr_2[0]->func_node = NULL;
			col_arr_2[0]->math_node = NULL;
			col_arr_2[0]->case_node = NULL;

			initSelectClauseForComp(&the_select_node->next->next, NULL, false, 1, col_arr_2, NULL, -1, NULL, NULL
								   ,&malloced_head, the_debug);
			the_select_node->next->next->prev = the_select_node->next;


			if (test_Controller_parseSelect(575, "select new_case from ( select case when EXPIRATION = '1/31/2025' then 'yes' when EXPIRATION = '3/16/2025' then 'maybe' else 'NO' end new_case from LIQUOR_LICENSES ) tbl;", &the_select_node
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
		// END Test with id = 575

		// START Test with id = 576
			initSelectClauseForComp(&the_select_node, NULL, false, 0, NULL, getTablesHead(), PTR_TYPE_TABLE_INFO, NULL, NULL
								   ,&malloced_head, the_debug);

			col_arr = (struct col_in_select_node**) myMalloc(sizeof(struct col_in_select_node*) * 1, NULL, &malloced_head, the_debug);

			col_arr[0] = (struct col_in_select_node*) myMalloc(sizeof(struct col_in_select_node), NULL, &malloced_head, the_debug);

			col_arr[0]->table_ptr = NULL;
			col_arr[0]->table_ptr_type = -1;

			col_arr[0]->col_ptr = NULL;
			col_arr[0]->col_ptr_type = -1;

			col_arr[0]->new_name = (char*) myMalloc(sizeof(char) * 10, NULL, &malloced_head, the_debug);
			strcpy(col_arr[0]->new_name, "NEW_CASE");

			col_arr[0]->func_node = NULL;
			col_arr[0]->math_node = NULL;

			col_arr[0]->case_node = (struct case_node*) myMalloc(sizeof(struct case_node), NULL, &malloced_head, the_debug);
			col_arr[0]->case_node->case_when_head = NULL;
			col_arr[0]->case_node->case_then_value_head = NULL;

			temp_where = (struct where_clause_node*) myMalloc(sizeof(struct where_clause_node), NULL, &malloced_head, the_debug);
			temp_where->ptr_one = the_select_node->columns_arr[4];
			temp_where->ptr_one_type = PTR_TYPE_COL_IN_SELECT_NODE;
			temp_where->ptr_two = (char*) myMalloc(sizeof(char) * 16, NULL, &malloced_head, the_debug);
			strcpy(temp_where->ptr_two, "1/31/2025");
			temp_where->ptr_two_type = PTR_TYPE_DATE;
			temp_where->where_type = WHERE_IS_EQUALS;
			temp_where->parent = NULL;

			temp_where_2 = (struct where_clause_node*) myMalloc(sizeof(struct where_clause_node), NULL, &malloced_head, the_debug);
			temp_where_2->ptr_one = the_select_node->columns_arr[4];
			temp_where_2->ptr_one_type = PTR_TYPE_COL_IN_SELECT_NODE;
			temp_where_2->ptr_two = (char*) myMalloc(sizeof(char) * 16, NULL, &malloced_head, the_debug);
			strcpy(temp_where_2->ptr_two, "3/16/2025");
			temp_where_2->ptr_two_type = PTR_TYPE_DATE;
			temp_where_2->where_type = WHERE_IS_EQUALS;
			temp_where_2->parent = NULL;

			addListNodePtr(&col_arr[0]->case_node->case_when_head, &col_arr[0]->case_node->case_when_tail, temp_where, PTR_TYPE_WHERE_CLAUSE_NODE, ADDLISTNODE_TAIL
						  ,NULL, &malloced_head, the_debug);
			addListNodePtr(&col_arr[0]->case_node->case_when_head, &col_arr[0]->case_node->case_when_tail, temp_where_2, PTR_TYPE_WHERE_CLAUSE_NODE, ADDLISTNODE_TAIL
						  ,NULL, &malloced_head, the_debug);
			addListNodePtr(&col_arr[0]->case_node->case_when_head, &col_arr[0]->case_node->case_when_tail, NULL, -1, ADDLISTNODE_TAIL
						  ,NULL, &malloced_head, the_debug);

			double* double1 = (double*) myMalloc(sizeof(double), NULL, &malloced_head, the_debug);
			*double1 = 1.0;

			struct math_node* temp_math_1 = (struct math_node*) myMalloc(sizeof(struct math_node), NULL, &malloced_head, the_debug);
			temp_math_1->ptr_one = (int*) myMalloc(sizeof(int*), NULL, &malloced_head, the_debug);
			*((int*) temp_math_1->ptr_one) = 2;
			temp_math_1->ptr_one_type = PTR_TYPE_INT;
			temp_math_1->ptr_two = (double*) myMalloc(sizeof(double*), NULL, &malloced_head, the_debug);
			*((double*) temp_math_1->ptr_two) = 10.0;
			temp_math_1->ptr_two_type = PTR_TYPE_REAL;
			temp_math_1->operation = MATH_MULT;
			temp_math_1->parent = NULL;
			temp_math_1->result_type = PTR_TYPE_REAL;

			struct math_node* temp_math_2 = (struct math_node*) myMalloc(sizeof(struct math_node), NULL, &malloced_head, the_debug);
			temp_math_2->ptr_one = (double*) myMalloc(sizeof(double*), NULL, &malloced_head, the_debug);
			*((double*) temp_math_2->ptr_one) = 10.0;
			temp_math_2->ptr_one_type = PTR_TYPE_REAL;
			temp_math_2->ptr_two = (double*) myMalloc(sizeof(double*), NULL, &malloced_head, the_debug);
			*((double*) temp_math_2->ptr_two) = 10.0;
			temp_math_2->ptr_two_type = PTR_TYPE_REAL;
			temp_math_2->operation = MATH_MULT;
			temp_math_2->parent = NULL;
			temp_math_2->result_type = PTR_TYPE_REAL;


			addListNodePtr(&col_arr[0]->case_node->case_then_value_head, &col_arr[0]->case_node->case_then_value_tail, double1, PTR_TYPE_REAL, ADDLISTNODE_TAIL
						  ,NULL, &malloced_head, the_debug);
			addListNodePtr(&col_arr[0]->case_node->case_then_value_head, &col_arr[0]->case_node->case_then_value_tail, temp_math_1, PTR_TYPE_MATH_NODE, ADDLISTNODE_TAIL
						  ,NULL, &malloced_head, the_debug);
			addListNodePtr(&col_arr[0]->case_node->case_then_value_head, &col_arr[0]->case_node->case_then_value_tail, temp_math_2, PTR_TYPE_MATH_NODE, ADDLISTNODE_TAIL
						  ,NULL, &malloced_head, the_debug);

			col_arr[0]->rows_data_type = col_arr[0]->case_node->case_then_value_head->ptr_type;

			initSelectClauseForComp(&the_select_node->next, NULL, false, 1, col_arr, NULL, -1, NULL, NULL
								   ,&malloced_head, the_debug);
			the_select_node->next->prev = the_select_node;


			if (test_Controller_parseSelect(576, "select case when EXPIRATION = '1/31/2025' then 1.0 when EXPIRATION = '3/16/2025' then 2 * 10.0 else 10.0 * 10.0 end new_case from LIQUOR_LICENSES;", &the_select_node
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
		// END Test with id = 576

		// START Test with id = 577
			initSelectClauseForComp(&the_select_node, NULL, false, 0, NULL, getTablesHead(), PTR_TYPE_TABLE_INFO, NULL, NULL
								   ,&malloced_head, the_debug);

			col_arr = (struct col_in_select_node**) myMalloc(sizeof(struct col_in_select_node*) * 2, NULL, &malloced_head, the_debug);

			col_arr[0] = (struct col_in_select_node*) myMalloc(sizeof(struct col_in_select_node), NULL, &malloced_head, the_debug);

			col_arr[0]->table_ptr = the_select_node;
			col_arr[0]->table_ptr_type = PTR_TYPE_SELECT_NODE;
			col_arr[0]->col_ptr = the_select_node->columns_arr[4];
			col_arr[0]->col_ptr_type = PTR_TYPE_COL_IN_SELECT_NODE;
			col_arr[0]->new_name = NULL;
			col_arr[0]->func_node = NULL;
			col_arr[0]->math_node = NULL;
			col_arr[0]->case_node = NULL;
			col_arr[0]->rows_data_type = the_select_node->columns_arr[4]->rows_data_type;

			col_arr[1] = (struct col_in_select_node*) myMalloc(sizeof(struct col_in_select_node), NULL, &malloced_head, the_debug);
			col_arr[1]->table_ptr = NULL;
			col_arr[1]->table_ptr_type = -1;
			col_arr[1]->col_ptr = NULL;
			col_arr[1]->col_ptr_type = -1;
			col_arr[1]->new_name = upper("MAX", NULL, &malloced_head, the_debug);
			col_arr[1]->math_node = NULL;
			col_arr[1]->case_node = NULL;

			col_arr[1]->func_node = (struct func_node*) myMalloc(sizeof(struct func_node), NULL, &malloced_head, the_debug);
			col_arr[1]->func_node->which_func = FUNC_MAX;
			col_arr[1]->func_node->distinct = false;
			col_arr[1]->func_node->args_size = 1;
			col_arr[1]->func_node->args_arr_type = (int*) myMalloc(sizeof(int), NULL, &malloced_head, the_debug);
			col_arr[1]->func_node->args_arr_type[0] = PTR_TYPE_FUNC_NODE;
			col_arr[1]->func_node->args_arr = (void**) myMalloc(sizeof(void*), NULL, &malloced_head, the_debug);
			col_arr[1]->func_node->group_by_cols_head = NULL;
			col_arr[1]->func_node->group_by_cols_tail = NULL;
			col_arr[1]->func_node->result_type = PTR_TYPE_INT;

			addListNodePtr(&col_arr[1]->func_node->group_by_cols_head, &col_arr[1]->func_node->group_by_cols_tail, the_select_node->columns_arr[4], PTR_TYPE_COL_IN_SELECT_NODE, ADDLISTNODE_TAIL
						  ,NULL, &malloced_head, the_debug);

			col_arr[1]->func_node->args_arr[0] = myMalloc(sizeof(struct func_node), NULL, &malloced_head, the_debug);
			((struct func_node*) col_arr[1]->func_node->args_arr[0])->which_func = FUNC_COUNT;
			((struct func_node*) col_arr[1]->func_node->args_arr[0])->distinct = false;
			((struct func_node*) col_arr[1]->func_node->args_arr[0])->args_size = 7;
			((struct func_node*) col_arr[1]->func_node->args_arr[0])->args_arr_type = (int*) myMalloc(sizeof(int) * 7, NULL, &malloced_head, the_debug);
			((struct func_node*) col_arr[1]->func_node->args_arr[0])->args_arr = (void**) myMalloc(sizeof(void*) * 7, NULL, &malloced_head, the_debug);
			for (int a=0; a<7; a++)
			{
				((struct func_node*) col_arr[1]->func_node->args_arr[0])->args_arr[a] = the_select_node->columns_arr[a];
				((struct func_node*) col_arr[1]->func_node->args_arr[0])->args_arr_type[a] = PTR_TYPE_COL_IN_SELECT_NODE;
			}
			((struct func_node*) col_arr[1]->func_node->args_arr[0])->group_by_cols_head = NULL;
			((struct func_node*) col_arr[1]->func_node->args_arr[0])->group_by_cols_tail = NULL;
			((struct func_node*) col_arr[1]->func_node->args_arr[0])->result_type = PTR_TYPE_INT;

			col_arr[1]->rows_data_type = PTR_TYPE_INT;

			initSelectClauseForComp(&the_select_node->next, NULL, false, 2, col_arr, NULL, -1, NULL, NULL
								   ,&malloced_head, the_debug);
			the_select_node->next->prev = the_select_node;


			if (test_Controller_parseSelect(577, "select EXPIRATION, max(count(*)) MAX from LIQUOR_LICENSES group by EXPIRATION;", &the_select_node
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
		// END Test with id = 577

		// START Test with id = 578
			initSelectClauseForComp(&the_select_node, NULL, false, 0, NULL, getTablesHead(), PTR_TYPE_TABLE_INFO, NULL, NULL
								   ,&malloced_head, the_debug);

			col_arr = (struct col_in_select_node**) myMalloc(sizeof(struct col_in_select_node*) * 2, NULL, &malloced_head, the_debug);

			for (int i=0; i<2; i++)
			{
				col_arr[i] = (struct col_in_select_node*) myMalloc(sizeof(struct col_in_select_node), NULL, &malloced_head, the_debug);
				col_arr[i]->table_ptr = NULL;
				col_arr[i]->table_ptr_type = -1;
				col_arr[i]->col_ptr = NULL;
				col_arr[i]->col_ptr_type = -1;
				if (i == 0)
					col_arr[i]->new_name = upper("two", NULL, &malloced_head, the_debug);
				else
					col_arr[i]->new_name = upper("cnt", NULL, &malloced_head, the_debug);
				col_arr[i]->func_node = NULL;
				col_arr[i]->math_node = NULL;
				col_arr[i]->case_node = NULL;
			}

			col_arr[0]->math_node = (struct math_node*) myMalloc(sizeof(struct math_node), NULL, &malloced_head, the_debug);
			col_arr[0]->math_node->ptr_one = (int*) myMalloc(sizeof(int), NULL, &malloced_head, the_debug);
			*((int*) col_arr[0]->math_node->ptr_one) = 1;
			col_arr[0]->math_node->ptr_one_type = PTR_TYPE_INT;
			col_arr[0]->math_node->ptr_two = (int*) myMalloc(sizeof(int), NULL, &malloced_head, the_debug);
			*((int*) col_arr[0]->math_node->ptr_two) = 1;
			col_arr[0]->math_node->ptr_two_type = PTR_TYPE_INT;
			col_arr[0]->math_node->operation = MATH_ADD;
			col_arr[0]->math_node->parent = NULL;
			col_arr[0]->math_node->result_type = PTR_TYPE_INT;
			col_arr[0]->rows_data_type = PTR_TYPE_INT;

			col_arr[1]->func_node = myMalloc(sizeof(struct func_node), NULL, &malloced_head, the_debug);
			col_arr[1]->func_node->which_func = FUNC_COUNT;
			col_arr[1]->func_node->distinct = false;
			col_arr[1]->func_node->args_size = 7;
			col_arr[1]->func_node->args_arr_type = (int*) myMalloc(sizeof(int) * 7, NULL, &malloced_head, the_debug);
			col_arr[1]->func_node->args_arr = (void**) myMalloc(sizeof(void*) * 7, NULL, &malloced_head, the_debug);
			for (int a=0; a<7; a++)
			{
				col_arr[1]->func_node->args_arr[a] = the_select_node->columns_arr[a];
				col_arr[1]->func_node->args_arr_type[a] = PTR_TYPE_COL_IN_SELECT_NODE;
			}
			col_arr[1]->func_node->group_by_cols_head = NULL;
			col_arr[1]->func_node->group_by_cols_tail = NULL;
			col_arr[1]->func_node->result_type = PTR_TYPE_INT;
			col_arr[1]->rows_data_type = PTR_TYPE_INT;

			initSelectClauseForComp(&the_select_node->next, NULL, false, 2, col_arr, NULL, -1, NULL, NULL
								   ,&malloced_head, the_debug);
			the_select_node->next->prev = the_select_node;

			the_select_node->next->where_head = (struct where_clause_node*) myMalloc(sizeof(struct where_clause_node), NULL, &malloced_head, the_debug);

			the_select_node->next->where_head->ptr_one = (struct where_clause_node*) myMalloc(sizeof(struct where_clause_node), NULL, &malloced_head, the_debug);
			the_select_node->next->where_head->ptr_one_type = PTR_TYPE_WHERE_CLAUSE_NODE;
			the_select_node->next->where_head->ptr_two = (struct where_clause_node*) myMalloc(sizeof(struct where_clause_node), NULL, &malloced_head, the_debug);
			the_select_node->next->where_head->ptr_two_type = PTR_TYPE_WHERE_CLAUSE_NODE;
			the_select_node->next->where_head->where_type = WHERE_AND;
			the_select_node->next->where_head->parent = NULL;

			((struct where_clause_node*) the_select_node->next->where_head->ptr_one)->ptr_one = the_select_node->next->columns_arr[0];
			((struct where_clause_node*) the_select_node->next->where_head->ptr_one)->ptr_one_type = PTR_TYPE_COL_IN_SELECT_NODE;
			((struct where_clause_node*) the_select_node->next->where_head->ptr_one)->ptr_two = (int*) myMalloc(sizeof(int), NULL, &malloced_head, the_debug);
			*((int*) ((struct where_clause_node*) the_select_node->next->where_head->ptr_one)->ptr_two) = 2;
			((struct where_clause_node*) the_select_node->next->where_head->ptr_one)->ptr_two_type = PTR_TYPE_INT;
			((struct where_clause_node*) the_select_node->next->where_head->ptr_one)->where_type = WHERE_IS_EQUALS;
			((struct where_clause_node*) the_select_node->next->where_head->ptr_one)->parent = the_select_node->next->where_head;

			((struct where_clause_node*) the_select_node->next->where_head->ptr_two)->ptr_one = the_select_node->next->columns_arr[1];
			((struct where_clause_node*) the_select_node->next->where_head->ptr_two)->ptr_one_type = PTR_TYPE_COL_IN_SELECT_NODE;
			((struct where_clause_node*) the_select_node->next->where_head->ptr_two)->ptr_two = (int*) myMalloc(sizeof(int), NULL, &malloced_head, the_debug);
			*((int*) ((struct where_clause_node*) the_select_node->next->where_head->ptr_two)->ptr_two) = 1000;
			((struct where_clause_node*) the_select_node->next->where_head->ptr_two)->ptr_two_type = PTR_TYPE_INT;
			((struct where_clause_node*) the_select_node->next->where_head->ptr_two)->where_type = WHERE_IS_EQUALS;
			((struct where_clause_node*) the_select_node->next->where_head->ptr_two)->parent = the_select_node->next->where_head;


			if (test_Controller_parseSelect(578, "select 1 + 1 as two, count(*) cnt from LIQUOR_LICENSES where two = 2 and cnt = 1000;", &the_select_node
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
		// END Test with id = 578

		// START Test with id = 579
			initSelectClauseForComp(&the_select_node, NULL, false, 0, NULL, getTablesHead(), PTR_TYPE_TABLE_INFO, NULL, NULL
								   ,&malloced_head, the_debug);

			col_arr = (struct col_in_select_node**) myMalloc(sizeof(struct col_in_select_node*) * 2, NULL, &malloced_head, the_debug);

			col_arr[0] = (struct col_in_select_node*) myMalloc(sizeof(struct col_in_select_node), NULL, &malloced_head, the_debug);
			col_arr[0]->table_ptr = NULL;
			col_arr[0]->table_ptr_type = -1;
			col_arr[0]->col_ptr = NULL;
			col_arr[0]->col_ptr_type = -1;
			col_arr[0]->new_name = upper("cnt_and_one", NULL, &malloced_head, the_debug);
			col_arr[0]->func_node = NULL;
			col_arr[0]->math_node = NULL;
			col_arr[0]->case_node = NULL;

			col_arr[0]->math_node = (struct math_node*) myMalloc(sizeof(struct math_node), NULL, &malloced_head, the_debug);
			col_arr[0]->math_node->ptr_one = (struct func_node*) myMalloc(sizeof(struct func_node), NULL, &malloced_head, the_debug);
			col_arr[0]->math_node->ptr_one_type = PTR_TYPE_FUNC_NODE;
			col_arr[0]->math_node->ptr_two = myMalloc(sizeof(int), NULL, &malloced_head, the_debug);
			*((int*) col_arr[0]->math_node->ptr_two) = 1;
			col_arr[0]->math_node->ptr_two_type = PTR_TYPE_INT;
			col_arr[0]->math_node->operation = MATH_ADD;
			col_arr[0]->math_node->parent = NULL;
			col_arr[0]->math_node->result_type = PTR_TYPE_INT;

			((struct func_node*) col_arr[0]->math_node->ptr_one)->which_func = FUNC_COUNT;
			((struct func_node*) col_arr[0]->math_node->ptr_one)->distinct = false;
			((struct func_node*) col_arr[0]->math_node->ptr_one)->args_size = 7;
			((struct func_node*) col_arr[0]->math_node->ptr_one)->args_arr_type = (int*) myMalloc(sizeof(int) * 7, NULL, &malloced_head, the_debug);
			((struct func_node*) col_arr[0]->math_node->ptr_one)->args_arr = (void**) myMalloc(sizeof(void*) * 7, NULL, &malloced_head, the_debug);
			for (int a=0; a<7; a++)
			{
				((struct func_node*) col_arr[0]->math_node->ptr_one)->args_arr[a] = the_select_node->columns_arr[a];
				((struct func_node*) col_arr[0]->math_node->ptr_one)->args_arr_type[a] = PTR_TYPE_COL_IN_SELECT_NODE;
			}
			((struct func_node*) col_arr[0]->math_node->ptr_one)->group_by_cols_head = NULL;
			((struct func_node*) col_arr[0]->math_node->ptr_one)->group_by_cols_tail = NULL;
			((struct func_node*) col_arr[0]->math_node->ptr_one)->result_type = PTR_TYPE_INT;

			col_arr[0]->rows_data_type = PTR_TYPE_INT;

			col_arr[1] = (struct col_in_select_node*) myMalloc(sizeof(struct col_in_select_node), NULL, &malloced_head, the_debug);
			col_arr[1]->table_ptr = NULL;
			col_arr[1]->table_ptr_type = -1;
			col_arr[1]->col_ptr = NULL;
			col_arr[1]->col_ptr_type = -1;
			col_arr[1]->new_name = upper("one_and_cnt", NULL, &malloced_head, the_debug);
			col_arr[1]->func_node = NULL;
			col_arr[1]->math_node = NULL;
			col_arr[1]->case_node = NULL;

			col_arr[1]->math_node = (struct math_node*) myMalloc(sizeof(struct math_node), NULL, &malloced_head, the_debug);
			col_arr[1]->math_node->ptr_two = (struct func_node*) myMalloc(sizeof(struct func_node), NULL, &malloced_head, the_debug);
			col_arr[1]->math_node->ptr_two_type = PTR_TYPE_FUNC_NODE;
			col_arr[1]->math_node->ptr_one = myMalloc(sizeof(int), NULL, &malloced_head, the_debug);
			*((int*) col_arr[1]->math_node->ptr_one) = 1;
			col_arr[1]->math_node->ptr_one_type = PTR_TYPE_INT;
			col_arr[1]->math_node->operation = MATH_ADD;
			col_arr[1]->math_node->parent = NULL;
			col_arr[1]->math_node->result_type = PTR_TYPE_INT;

			((struct func_node*) col_arr[1]->math_node->ptr_two)->which_func = FUNC_COUNT;
			((struct func_node*) col_arr[1]->math_node->ptr_two)->distinct = false;
			((struct func_node*) col_arr[1]->math_node->ptr_two)->args_size = 7;
			((struct func_node*) col_arr[1]->math_node->ptr_two)->args_arr_type = (int*) myMalloc(sizeof(int) * 7, NULL, &malloced_head, the_debug);
			((struct func_node*) col_arr[1]->math_node->ptr_two)->args_arr = (void**) myMalloc(sizeof(void*) * 7, NULL, &malloced_head, the_debug);
			for (int a=0; a<7; a++)
			{
				((struct func_node*) col_arr[1]->math_node->ptr_two)->args_arr[a] = the_select_node->columns_arr[a];
				((struct func_node*) col_arr[1]->math_node->ptr_two)->args_arr_type[a] = PTR_TYPE_COL_IN_SELECT_NODE;
			}
			((struct func_node*) col_arr[1]->math_node->ptr_two)->group_by_cols_head = NULL;
			((struct func_node*) col_arr[1]->math_node->ptr_two)->group_by_cols_tail = NULL;
			((struct func_node*) col_arr[1]->math_node->ptr_two)->result_type = PTR_TYPE_INT;

			col_arr[1]->rows_data_type = PTR_TYPE_INT;

			initSelectClauseForComp(&the_select_node->next, NULL, false, 2, col_arr, NULL, -1, NULL, NULL
								   ,&malloced_head, the_debug);
			the_select_node->next->prev = the_select_node;

			if (test_Controller_parseSelect(579, "select count(*) + 1 cnt_and_one, 1 + count(*) one_and_cnt from LIQUOR_LICENSES;", &the_select_node
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
		// END Test with id = 579

		// START Test with id = 580
			initSelectClauseForComp(&the_select_node, NULL, false, 0, NULL, getTablesHead(), PTR_TYPE_TABLE_INFO, NULL, NULL
								   ,&malloced_head, the_debug);

			col_arr = (struct col_in_select_node**) myMalloc(sizeof(struct col_in_select_node*), NULL, &malloced_head, the_debug);

			col_arr[0] = (struct col_in_select_node*) myMalloc(sizeof(struct col_in_select_node), NULL, &malloced_head, the_debug);
			col_arr[0]->table_ptr = NULL;
			col_arr[0]->table_ptr_type = -1;
			col_arr[0]->col_ptr = NULL;
			col_arr[0]->col_ptr_type = -1;
			col_arr[0]->new_name = upper("new_case", NULL, &malloced_head, the_debug);
			col_arr[0]->func_node = NULL;
			col_arr[0]->math_node = NULL;
			col_arr[0]->rows_data_type = PTR_TYPE_INT;

			col_arr[0]->case_node = (struct case_node*) myMalloc(sizeof(struct case_node), NULL, &malloced_head, the_debug);
			col_arr[0]->case_node->case_when_head = NULL;
			col_arr[0]->case_node->case_then_value_head = NULL;

			temp_where = (struct where_clause_node*) myMalloc(sizeof(struct where_clause_node), NULL, &malloced_head, the_debug);
			temp_where->ptr_one = the_select_node->columns_arr[4];
			temp_where->ptr_one_type = PTR_TYPE_COL_IN_SELECT_NODE;
			temp_where->ptr_two = (char*) myMalloc(sizeof(char) * 16, NULL, &malloced_head, the_debug);
			strcpy(temp_where->ptr_two, "1/31/2025");
			temp_where->ptr_two_type = PTR_TYPE_DATE;
			temp_where->where_type = WHERE_IS_EQUALS;
			temp_where->parent = NULL;

			addListNodePtr(&col_arr[0]->case_node->case_when_head, &col_arr[0]->case_node->case_when_tail, temp_where, PTR_TYPE_WHERE_CLAUSE_NODE, ADDLISTNODE_TAIL
						  ,NULL, &malloced_head, the_debug);
			addListNodePtr(&col_arr[0]->case_node->case_when_head, &col_arr[0]->case_node->case_when_tail, NULL, -1, ADDLISTNODE_TAIL
						  ,NULL, &malloced_head, the_debug);

			
			struct math_node* math_node = (struct math_node*) myMalloc(sizeof(struct math_node), NULL, &malloced_head, the_debug);
			math_node->ptr_one = (struct func_node*) myMalloc(sizeof(struct func_node), NULL, &malloced_head, the_debug);
			math_node->ptr_one_type = PTR_TYPE_FUNC_NODE;
			math_node->ptr_two = myMalloc(sizeof(int), NULL, &malloced_head, the_debug);
			*((int*) math_node->ptr_two) = 1;
			math_node->ptr_two_type = PTR_TYPE_INT;
			math_node->operation = MATH_ADD;
			math_node->parent = NULL;
			math_node->result_type = PTR_TYPE_INT;

			((struct func_node*) math_node->ptr_one)->which_func = FUNC_COUNT;
			((struct func_node*) math_node->ptr_one)->distinct = false;
			((struct func_node*) math_node->ptr_one)->args_size = 7;
			((struct func_node*) math_node->ptr_one)->args_arr_type = (int*) myMalloc(sizeof(int) * 7, NULL, &malloced_head, the_debug);
			((struct func_node*) math_node->ptr_one)->args_arr = (void**) myMalloc(sizeof(void*) * 7, NULL, &malloced_head, the_debug);
			for (int a=0; a<7; a++)
			{
				((struct func_node*) math_node->ptr_one)->args_arr[a] = the_select_node->columns_arr[a];
				((struct func_node*) math_node->ptr_one)->args_arr_type[a] = PTR_TYPE_COL_IN_SELECT_NODE;
			}
			((struct func_node*) math_node->ptr_one)->group_by_cols_head = NULL;
			((struct func_node*) math_node->ptr_one)->group_by_cols_tail = NULL;
			addListNodePtr(&((struct func_node*) math_node->ptr_one)->group_by_cols_head, &((struct func_node*) math_node->ptr_one)->group_by_cols_tail
						  ,((struct select_node*) the_select_node)->columns_arr[4], PTR_TYPE_COL_IN_SELECT_NODE, ADDLISTNODE_TAIL
						  ,NULL, &malloced_head, the_debug);
			((struct func_node*) math_node->ptr_one)->result_type = PTR_TYPE_INT;

			struct func_node* func_node = (struct func_node*) myMalloc(sizeof(struct func_node), NULL, &malloced_head, the_debug);
			func_node->which_func = FUNC_COUNT;
			func_node->distinct = true;
			func_node->args_size = 1;
			func_node->args_arr_type = (int*) myMalloc(sizeof(int), NULL, &malloced_head, the_debug);
			func_node->args_arr_type[0] = PTR_TYPE_COL_IN_SELECT_NODE;
			func_node->args_arr = (void**) myMalloc(sizeof(void*), NULL, &malloced_head, the_debug);
			func_node->args_arr[0] = the_select_node->columns_arr[3];
			func_node->group_by_cols_head = NULL;
			func_node->group_by_cols_tail = NULL;
			addListNodePtr(&func_node->group_by_cols_head, &func_node->group_by_cols_tail
						  ,((struct select_node*) the_select_node)->columns_arr[4], PTR_TYPE_COL_IN_SELECT_NODE, ADDLISTNODE_TAIL
						  ,NULL, &malloced_head, the_debug);
			func_node->result_type = PTR_TYPE_INT;

			addListNodePtr(&col_arr[0]->case_node->case_then_value_head, &col_arr[0]->case_node->case_then_value_tail, math_node, PTR_TYPE_MATH_NODE, ADDLISTNODE_TAIL
						  ,NULL, &malloced_head, the_debug);
			addListNodePtr(&col_arr[0]->case_node->case_then_value_head, &col_arr[0]->case_node->case_then_value_tail, func_node, PTR_TYPE_FUNC_NODE, ADDLISTNODE_TAIL
						  ,NULL, &malloced_head, the_debug);

			initSelectClauseForComp(&the_select_node->next, NULL, false, 1, col_arr, NULL, -1, NULL, NULL
								   ,&malloced_head, the_debug);
			the_select_node->next->prev = the_select_node;


			if (test_Controller_parseSelect(580, "select case when EXPIRATION = '1/31/2025' then count(*) + 1 else count(distinct EFFECTIVE) end new_case from LIQUOR_LICENSES group by EXPIRATION;", &the_select_node
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
		// END Test with id = 580

		// START Test with id = 581
			the_select_node = NULL;


			if (test_Controller_parseSelect(581, "select case when EXPIRATION = '1/31/2025' then count(*) + 1 else count(distinct EFFECTIVE) end new_case from LIQUOR_LICENSES;", &the_select_node
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
		// END Test with id = 581

		// START Test with id = 582
			initSelectClauseForComp(&the_select_node, NULL, false, 0, NULL, getTablesHead(), PTR_TYPE_TABLE_INFO, NULL, NULL
								   ,&malloced_head, the_debug);

			col_arr = (struct col_in_select_node**) myMalloc(sizeof(struct col_in_select_node*), NULL, &malloced_head, the_debug);

			col_arr[0] = (struct col_in_select_node*) myMalloc(sizeof(struct col_in_select_node), NULL, &malloced_head, the_debug);
			col_arr[0]->table_ptr = NULL;
			col_arr[0]->table_ptr_type = -1;
			col_arr[0]->col_ptr = NULL;
			col_arr[0]->col_ptr_type = -1;
			col_arr[0]->new_name = upper("new_case", NULL, &malloced_head, the_debug);
			col_arr[0]->func_node = NULL;
			col_arr[0]->math_node = NULL;
			col_arr[0]->rows_data_type = PTR_TYPE_CHAR;

			col_arr[0]->case_node = (struct case_node*) myMalloc(sizeof(struct case_node), NULL, &malloced_head, the_debug);
			col_arr[0]->case_node->case_when_head = NULL;
			col_arr[0]->case_node->case_then_value_head = NULL;

			temp_where = (struct where_clause_node*) myMalloc(sizeof(struct where_clause_node), NULL, &malloced_head, the_debug);
			temp_where->ptr_one = the_select_node->columns_arr[4];
			temp_where->ptr_one_type = PTR_TYPE_COL_IN_SELECT_NODE;
			temp_where->ptr_two = (char*) myMalloc(sizeof(char) * 16, NULL, &malloced_head, the_debug);
			strcpy(temp_where->ptr_two, "1/31/2025");
			temp_where->ptr_two_type = PTR_TYPE_DATE;
			temp_where->where_type = WHERE_IS_EQUALS;
			temp_where->parent = NULL;

			addListNodePtr(&col_arr[0]->case_node->case_when_head, &col_arr[0]->case_node->case_when_tail, temp_where, PTR_TYPE_WHERE_CLAUSE_NODE, ADDLISTNODE_TAIL
						  ,NULL, &malloced_head, the_debug);
			addListNodePtr(&col_arr[0]->case_node->case_when_head, &col_arr[0]->case_node->case_when_tail, NULL, -1, ADDLISTNODE_TAIL
						  ,NULL, &malloced_head, the_debug);

			str1 = myMalloc(sizeof(char) * 10, NULL, &malloced_head, the_debug);
			strcpy(str1, "NA");

			addListNodePtr(&col_arr[0]->case_node->case_then_value_head, &col_arr[0]->case_node->case_then_value_tail, str1, PTR_TYPE_CHAR, ADDLISTNODE_TAIL
						  ,NULL, &malloced_head, the_debug);
			addListNodePtr(&col_arr[0]->case_node->case_then_value_head, &col_arr[0]->case_node->case_then_value_tail, the_select_node->columns_arr[0], PTR_TYPE_COL_IN_SELECT_NODE, ADDLISTNODE_TAIL
						  ,NULL, &malloced_head, the_debug);

			initSelectClauseForComp(&the_select_node->next, NULL, false, 1, col_arr, NULL, -1, NULL, NULL
								   ,&malloced_head, the_debug);
			the_select_node->next->prev = the_select_node;


			if (test_Controller_parseSelect(582, "select case when EXPIRATION = '1/31/2025' then 'NA' else BRAND_NAME end new_case from LIQUOR_LICENSES;", &the_select_node
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
		// END Test with id = 582

		// START Test with id = 583
			initSelectClauseForComp(&the_select_node, NULL, false, 0, NULL, getTablesHead(), PTR_TYPE_TABLE_INFO, NULL, NULL
								   ,&malloced_head, the_debug);

			col_arr = (struct col_in_select_node**) myMalloc(sizeof(struct col_in_select_node*), NULL, &malloced_head, the_debug);

			col_arr[0] = (struct col_in_select_node*) myMalloc(sizeof(struct col_in_select_node), NULL, &malloced_head, the_debug);
			col_arr[0]->table_ptr = NULL;
			col_arr[0]->table_ptr_type = -1;
			col_arr[0]->col_ptr = NULL;
			col_arr[0]->col_ptr_type = -1;
			col_arr[0]->new_name = upper("new_case", NULL, &malloced_head, the_debug);
			col_arr[0]->func_node = NULL;
			col_arr[0]->math_node = NULL;
			col_arr[0]->rows_data_type = PTR_TYPE_INT;

			col_arr[0]->case_node = (struct case_node*) myMalloc(sizeof(struct case_node), NULL, &malloced_head, the_debug);
			col_arr[0]->case_node->case_when_head = NULL;
			col_arr[0]->case_node->case_then_value_head = NULL;

			temp_where = (struct where_clause_node*) myMalloc(sizeof(struct where_clause_node), NULL, &malloced_head, the_debug);
			temp_where->ptr_one = the_select_node->columns_arr[4];
			temp_where->ptr_one_type = PTR_TYPE_COL_IN_SELECT_NODE;
			temp_where->ptr_two = (char*) myMalloc(sizeof(char) * 16, NULL, &malloced_head, the_debug);
			strcpy(temp_where->ptr_two, "1/31/2025");
			temp_where->ptr_two_type = PTR_TYPE_DATE;
			temp_where->where_type = WHERE_IS_EQUALS;
			temp_where->parent = NULL;

			addListNodePtr(&col_arr[0]->case_node->case_when_head, &col_arr[0]->case_node->case_when_tail, temp_where, PTR_TYPE_WHERE_CLAUSE_NODE, ADDLISTNODE_TAIL
						  ,NULL, &malloced_head, the_debug);
			addListNodePtr(&col_arr[0]->case_node->case_when_head, &col_arr[0]->case_node->case_when_tail, NULL, -1, ADDLISTNODE_TAIL
						  ,NULL, &malloced_head, the_debug);

			int1 = myMalloc(sizeof(int), NULL, &malloced_head, the_debug);
			*int1 = 100;

			addListNodePtr(&col_arr[0]->case_node->case_then_value_head, &col_arr[0]->case_node->case_then_value_tail, int1, PTR_TYPE_INT, ADDLISTNODE_TAIL
						  ,NULL, &malloced_head, the_debug);
			addListNodePtr(&col_arr[0]->case_node->case_then_value_head, &col_arr[0]->case_node->case_then_value_tail, the_select_node->columns_arr[1], PTR_TYPE_COL_IN_SELECT_NODE, ADDLISTNODE_TAIL
						  ,NULL, &malloced_head, the_debug);

			initSelectClauseForComp(&the_select_node->next, NULL, false, 1, col_arr, NULL, -1, NULL, NULL
								   ,&malloced_head, the_debug);
			the_select_node->next->prev = the_select_node;


			if (test_Controller_parseSelect(583, "select case when EXPIRATION = '1/31/2025' then 100 else CT_REGISTRATION_NUMBER end new_case from LIQUOR_LICENSES;", &the_select_node
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
		// END Test with id = 583

		// START Test with id = 584
			initSelectClauseForComp(&the_select_node, "TBL", false, 0, NULL, getTablesHead(), PTR_TYPE_TABLE_INFO, NULL, NULL
								   ,&malloced_head, the_debug);

			initSelectClauseForComp(&joined_select, "TBL2", false, 0, NULL, getTablesHead(), PTR_TYPE_TABLE_INFO, NULL, NULL
								   ,&malloced_head, the_debug);

			col_arr = (struct col_in_select_node**) myMalloc(sizeof(struct col_in_select_node*), NULL, &malloced_head, the_debug);

			col_arr[0] = (struct col_in_select_node*) myMalloc(sizeof(struct col_in_select_node), NULL, &malloced_head, the_debug);
			col_arr[0]->table_ptr = NULL;
			col_arr[0]->table_ptr_type = -1;
			col_arr[0]->col_ptr = NULL;
			col_arr[0]->col_ptr_type = -1;
			col_arr[0]->new_name = upper("MATH", NULL, &malloced_head, the_debug);
			col_arr[0]->func_node = NULL;
			col_arr[0]->math_node = NULL;
			col_arr[0]->case_node = NULL;
			col_arr[0]->rows_data_type = PTR_TYPE_INT;

			col_arr[0]->math_node = (struct math_node*) myMalloc(sizeof(struct math_node), NULL, &malloced_head, the_debug);
			col_arr[0]->math_node->ptr_one = the_select_node->columns_arr[1];
			col_arr[0]->math_node->ptr_one_type = PTR_TYPE_COL_IN_SELECT_NODE;
			col_arr[0]->math_node->ptr_two = (int*) myMalloc(sizeof(int), NULL, &malloced_head, the_debug);
			*((int*) col_arr[0]->math_node->ptr_two) = 100;
			col_arr[0]->math_node->ptr_two_type = PTR_TYPE_INT;
			col_arr[0]->math_node->operation = MATH_MULT;
			col_arr[0]->math_node->parent = NULL;
			col_arr[0]->math_node->result_type = PTR_TYPE_INT;

			initSelectClauseForComp(&the_select_node->next, NULL, false, 1, col_arr, NULL, -1, NULL, NULL
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


			if (test_Controller_parseSelect(584, "select tbl.CT_REGISTRATION_NUMBER * 100 MATH from LIQUOR_LICENSES tbl join LIQUOR_LICENSES tbl2 on tbl.BRAND_NAME = tbl2.BRAND_NAME;", &the_select_node
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
		// END Test with id = 584

		// START Test with id = 585
			initSelectClauseForComp(&the_select_node, "TBL", false, 0, NULL, getTablesHead(), PTR_TYPE_TABLE_INFO, NULL, NULL
								   ,&malloced_head, the_debug);

			initSelectClauseForComp(&joined_select, "TBL2", false, 0, NULL, getTablesHead(), PTR_TYPE_TABLE_INFO, NULL, NULL
								   ,&malloced_head, the_debug);

			col_arr = (struct col_in_select_node**) myMalloc(sizeof(struct col_in_select_node*), NULL, &malloced_head, the_debug);

			col_arr[0] = (struct col_in_select_node*) myMalloc(sizeof(struct col_in_select_node), NULL, &malloced_head, the_debug);
			col_arr[0]->table_ptr = NULL;
			col_arr[0]->table_ptr_type = -1;
			col_arr[0]->col_ptr = NULL;
			col_arr[0]->col_ptr_type = -1;
			col_arr[0]->new_name = upper("MATH", NULL, &malloced_head, the_debug);
			col_arr[0]->func_node = NULL;
			col_arr[0]->math_node = NULL;
			col_arr[0]->case_node = NULL;
			col_arr[0]->rows_data_type = PTR_TYPE_INT;

			col_arr[0]->math_node = (struct math_node*) myMalloc(sizeof(struct math_node), NULL, &malloced_head, the_debug);
			col_arr[0]->math_node->ptr_one = (int*) myMalloc(sizeof(int), NULL, &malloced_head, the_debug);
			*((int*) col_arr[0]->math_node->ptr_one) = 100;
			col_arr[0]->math_node->ptr_one_type = PTR_TYPE_INT;
			col_arr[0]->math_node->ptr_two = the_select_node->columns_arr[1];
			col_arr[0]->math_node->ptr_two_type = PTR_TYPE_COL_IN_SELECT_NODE;
			col_arr[0]->math_node->operation = MATH_MULT;
			col_arr[0]->math_node->parent = NULL;
			col_arr[0]->math_node->result_type = PTR_TYPE_INT;

			initSelectClauseForComp(&the_select_node->next, NULL, false, 1, col_arr, NULL, -1, NULL, NULL
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


			if (test_Controller_parseSelect(585, "select 100 * tbl.CT_REGISTRATION_NUMBER MATH from LIQUOR_LICENSES tbl join LIQUOR_LICENSES tbl2 on tbl.BRAND_NAME = tbl2.BRAND_NAME;", &the_select_node
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
		// END Test with id = 585

		// START Test with id = 586
			initSelectClauseForComp(&the_select_node, "TBL", false, 0, NULL, getTablesHead(), PTR_TYPE_TABLE_INFO, NULL, NULL
								   ,&malloced_head, the_debug);

			initSelectClauseForComp(&joined_select, "TBL2", false, 0, NULL, getTablesHead(), PTR_TYPE_TABLE_INFO, NULL, NULL
								   ,&malloced_head, the_debug);

			col_arr = (struct col_in_select_node**) myMalloc(sizeof(struct col_in_select_node*), NULL, &malloced_head, the_debug);

			col_arr[0] = (struct col_in_select_node*) myMalloc(sizeof(struct col_in_select_node), NULL, &malloced_head, the_debug);
			col_arr[0]->table_ptr = NULL;
			col_arr[0]->table_ptr_type = -1;
			col_arr[0]->col_ptr = NULL;
			col_arr[0]->col_ptr_type = -1;
			col_arr[0]->new_name = upper("CNT", NULL, &malloced_head, the_debug);
			col_arr[0]->func_node = NULL;
			col_arr[0]->math_node = NULL;
			col_arr[0]->case_node = NULL;
			col_arr[0]->rows_data_type = PTR_TYPE_INT;

			col_arr[0]->func_node = (struct func_node*) myMalloc(sizeof(struct func_node), NULL, &malloced_head, the_debug);
			col_arr[0]->func_node->which_func = FUNC_COUNT;
			col_arr[0]->func_node->distinct = false;
			col_arr[0]->func_node->args_size = 1;
			col_arr[0]->func_node->args_arr = (void**) myMalloc(sizeof(void*), NULL, &malloced_head, the_debug);
			col_arr[0]->func_node->args_arr_type = (int*) myMalloc(sizeof(int), NULL, &malloced_head, the_debug);
			col_arr[0]->func_node->args_arr[0] = the_select_node->columns_arr[1];
			col_arr[0]->func_node->args_arr_type[0] = PTR_TYPE_COL_IN_SELECT_NODE;
			col_arr[0]->func_node->group_by_cols_head = NULL;
			col_arr[0]->func_node->group_by_cols_tail = NULL;
			col_arr[0]->func_node->result_type = PTR_TYPE_INT;

			initSelectClauseForComp(&the_select_node->next, NULL, false, 1, col_arr, NULL, -1, NULL, NULL
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


			if (test_Controller_parseSelect(586, "select count(tbl   . CT_REGISTRATION_NUMBER) CNT from LIQUOR_LICENSES tbl join LIQUOR_LICENSES tbl2 on tbl.BRAND_NAME = tbl2.BRAND_NAME;", &the_select_node
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
		// END Test with id = 586

		// START Test with id = 587
			initSelectClauseForComp(&the_select_node, NULL, false, 0, NULL, getTablesHead(), PTR_TYPE_TABLE_INFO, NULL, NULL
								   ,&malloced_head, the_debug);

			col_arr = (struct col_in_select_node**) myMalloc(sizeof(struct col_in_select_node*), NULL, &malloced_head, the_debug);

			col_arr[0] = (struct col_in_select_node*) myMalloc(sizeof(struct col_in_select_node), NULL, &malloced_head, the_debug);
			col_arr[0]->table_ptr = NULL;
			col_arr[0]->table_ptr_type = -1;
			col_arr[0]->col_ptr = NULL;
			col_arr[0]->col_ptr_type = -1;
			col_arr[0]->new_name = upper("NEG_ONE", NULL, &malloced_head, the_debug);
			col_arr[0]->func_node = NULL;
			col_arr[0]->math_node = NULL;
			col_arr[0]->case_node = NULL;
			col_arr[0]->rows_data_type = PTR_TYPE_INT;

			col_arr[0]->case_node = (struct case_node*) myMalloc(sizeof(struct case_node), NULL, &malloced_head, the_debug);
			col_arr[0]->case_node->case_when_head = NULL;
			col_arr[0]->case_node->case_then_value_head = NULL;

			addListNodePtr(&col_arr[0]->case_node->case_when_head, &col_arr[0]->case_node->case_when_tail, NULL, -1, ADDLISTNODE_TAIL
						  ,NULL, &malloced_head, the_debug);

			int1 = myMalloc(sizeof(int), NULL, &malloced_head, the_debug);
			*int1 = -1;

			addListNodePtr(&col_arr[0]->case_node->case_then_value_head, &col_arr[0]->case_node->case_then_value_tail, int1, PTR_TYPE_INT, ADDLISTNODE_TAIL
						  ,NULL, &malloced_head, the_debug);

			initSelectClauseForComp(&the_select_node->next, NULL, false, 1, col_arr, NULL, -1, NULL, NULL
								   ,&malloced_head, the_debug);
			the_select_node->next->prev = the_select_node;


			if (test_Controller_parseSelect(587, "select -1 NEG_ONE from LIQUOR_LICENSES;", &the_select_node
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
		// END Test with id = 587

		// START Test with id = 588
			initSelectClauseForComp(&the_select_node, NULL, false, 0, NULL, getTablesHead(), PTR_TYPE_TABLE_INFO, NULL, NULL
								   ,&malloced_head, the_debug);

			col_arr = (struct col_in_select_node**) myMalloc(sizeof(struct col_in_select_node*), NULL, &malloced_head, the_debug);

			col_arr[0] = (struct col_in_select_node*) myMalloc(sizeof(struct col_in_select_node), NULL, &malloced_head, the_debug);
			col_arr[0]->table_ptr = NULL;
			col_arr[0]->table_ptr_type = -1;
			col_arr[0]->col_ptr = NULL;
			col_arr[0]->col_ptr_type = -1;
			col_arr[0]->new_name = upper("new_case", NULL, &malloced_head, the_debug);
			col_arr[0]->func_node = NULL;
			col_arr[0]->math_node = NULL;
			col_arr[0]->rows_data_type = PTR_TYPE_INT;

			col_arr[0]->case_node = (struct case_node*) myMalloc(sizeof(struct case_node), NULL, &malloced_head, the_debug);
			col_arr[0]->case_node->case_when_head = NULL;
			col_arr[0]->case_node->case_then_value_head = NULL;

			temp_where = (struct where_clause_node*) myMalloc(sizeof(struct where_clause_node), NULL, &malloced_head, the_debug);
			temp_where->ptr_one = the_select_node->columns_arr[4];
			temp_where->ptr_one_type = PTR_TYPE_COL_IN_SELECT_NODE;
			temp_where->ptr_two = (char*) myMalloc(sizeof(char) * 16, NULL, &malloced_head, the_debug);
			strcpy(temp_where->ptr_two, "1/31/2025");
			temp_where->ptr_two_type = PTR_TYPE_DATE;
			temp_where->where_type = WHERE_IS_EQUALS;
			temp_where->parent = NULL;

			addListNodePtr(&col_arr[0]->case_node->case_when_head, &col_arr[0]->case_node->case_when_tail, temp_where, PTR_TYPE_WHERE_CLAUSE_NODE, ADDLISTNODE_TAIL
						  ,NULL, &malloced_head, the_debug);
			addListNodePtr(&col_arr[0]->case_node->case_when_head, &col_arr[0]->case_node->case_when_tail, NULL, -1, ADDLISTNODE_TAIL
						  ,NULL, &malloced_head, the_debug);

			temp_math_1 = myMalloc(sizeof(struct math_node), NULL, &malloced_head, the_debug);
			temp_math_1->ptr_one = myMalloc(sizeof(int), NULL, &malloced_head, the_debug);
			*((int*) temp_math_1->ptr_one) = 0;
			temp_math_1->ptr_one_type = PTR_TYPE_INT;
			temp_math_1->ptr_two = myMalloc(sizeof(int), NULL, &malloced_head, the_debug);
			*((int*) temp_math_1->ptr_two) = 1;
			temp_math_1->ptr_two_type = PTR_TYPE_INT;
			temp_math_1->operation = MATH_SUB;
			temp_math_1->parent = NULL;
			temp_math_1->result_type = PTR_TYPE_INT;

			temp_math_2 = myMalloc(sizeof(struct math_node), NULL, &malloced_head, the_debug);
			temp_math_2->ptr_one = myMalloc(sizeof(int), NULL, &malloced_head, the_debug);
			*((int*) temp_math_2->ptr_one) = 0;
			temp_math_2->ptr_one_type = PTR_TYPE_INT;
			temp_math_2->ptr_two = myMalloc(sizeof(int), NULL, &malloced_head, the_debug);
			*((int*) temp_math_2->ptr_two) = 2;
			temp_math_2->ptr_two_type = PTR_TYPE_INT;
			temp_math_2->operation = MATH_SUB;
			temp_math_2->parent = NULL;
			temp_math_2->result_type = PTR_TYPE_INT;

			addListNodePtr(&col_arr[0]->case_node->case_then_value_head, &col_arr[0]->case_node->case_then_value_tail, temp_math_1, PTR_TYPE_MATH_NODE, ADDLISTNODE_TAIL
						  ,NULL, &malloced_head, the_debug);
			addListNodePtr(&col_arr[0]->case_node->case_then_value_head, &col_arr[0]->case_node->case_then_value_tail, temp_math_2, PTR_TYPE_MATH_NODE, ADDLISTNODE_TAIL
						  ,NULL, &malloced_head, the_debug);

			initSelectClauseForComp(&the_select_node->next, NULL, false, 1, col_arr, NULL, -1, NULL, NULL
								   ,&malloced_head, the_debug);
			the_select_node->next->prev = the_select_node;


			if (test_Controller_parseSelect(588, "select case when EXPIRATION = '1/31/2025' then -1 else -2 end new_case from LIQUOR_LICENSES", &the_select_node
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
		// END Test with id = 588

		// START Test with id = 589
			initSelectClauseForComp(&the_select_node, NULL, false, 0, NULL, getTablesHead(), PTR_TYPE_TABLE_INFO, NULL, NULL
								   ,&malloced_head, the_debug);

			col_arr = (struct col_in_select_node**) myMalloc(sizeof(struct col_in_select_node*), NULL, &malloced_head, the_debug);

			col_arr[0] = (struct col_in_select_node*) myMalloc(sizeof(struct col_in_select_node), NULL, &malloced_head, the_debug);
			col_arr[0]->table_ptr = NULL;
			col_arr[0]->table_ptr_type = -1;
			col_arr[0]->col_ptr = NULL;
			col_arr[0]->col_ptr_type = -1;
			col_arr[0]->new_name = upper("new_case", NULL, &malloced_head, the_debug);
			col_arr[0]->func_node = NULL;
			col_arr[0]->math_node = NULL;
			col_arr[0]->rows_data_type = PTR_TYPE_REAL;

			col_arr[0]->case_node = (struct case_node*) myMalloc(sizeof(struct case_node), NULL, &malloced_head, the_debug);
			col_arr[0]->case_node->case_when_head = NULL;
			col_arr[0]->case_node->case_then_value_head = NULL;

			temp_where = (struct where_clause_node*) myMalloc(sizeof(struct where_clause_node), NULL, &malloced_head, the_debug);
			temp_where->ptr_one = the_select_node->columns_arr[4];
			temp_where->ptr_one_type = PTR_TYPE_COL_IN_SELECT_NODE;
			temp_where->ptr_two = (char*) myMalloc(sizeof(char) * 16, NULL, &malloced_head, the_debug);
			strcpy(temp_where->ptr_two, "1/31/2025");
			temp_where->ptr_two_type = PTR_TYPE_DATE;
			temp_where->where_type = WHERE_IS_EQUALS;
			temp_where->parent = NULL;

			addListNodePtr(&col_arr[0]->case_node->case_when_head, &col_arr[0]->case_node->case_when_tail, temp_where, PTR_TYPE_WHERE_CLAUSE_NODE, ADDLISTNODE_TAIL
						  ,NULL, &malloced_head, the_debug);
			addListNodePtr(&col_arr[0]->case_node->case_when_head, &col_arr[0]->case_node->case_when_tail, NULL, -1, ADDLISTNODE_TAIL
						  ,NULL, &malloced_head, the_debug);

			double1 = myMalloc(sizeof(double), NULL, &malloced_head, the_debug);
			*double1 = 1.0;

			temp_math_2 = myMalloc(sizeof(struct math_node), NULL, &malloced_head, the_debug);
			temp_math_2->ptr_one = myMalloc(sizeof(double), NULL, &malloced_head, the_debug);
			*((double*) temp_math_2->ptr_one) = 0.0;
			temp_math_2->ptr_one_type = PTR_TYPE_REAL;
			temp_math_2->ptr_two = myMalloc(sizeof(double), NULL, &malloced_head, the_debug);
			*((double*) temp_math_2->ptr_two) = 2.0;
			temp_math_2->ptr_two_type = PTR_TYPE_REAL;
			temp_math_2->operation = MATH_SUB;
			temp_math_2->parent = NULL;
			temp_math_2->result_type = PTR_TYPE_REAL;

			addListNodePtr(&col_arr[0]->case_node->case_then_value_head, &col_arr[0]->case_node->case_then_value_tail, double1, PTR_TYPE_REAL, ADDLISTNODE_TAIL
						  ,NULL, &malloced_head, the_debug);
			addListNodePtr(&col_arr[0]->case_node->case_then_value_head, &col_arr[0]->case_node->case_then_value_tail, temp_math_2, PTR_TYPE_MATH_NODE, ADDLISTNODE_TAIL
						  ,NULL, &malloced_head, the_debug);

			initSelectClauseForComp(&the_select_node->next, NULL, false, 1, col_arr, NULL, -1, NULL, NULL
								   ,&malloced_head, the_debug);
			the_select_node->next->prev = the_select_node;


			if (test_Controller_parseSelect(589, "select case when EXPIRATION = '1/31/2025' then 1.0 else -2.0 end new_case from LIQUOR_LICENSES", &the_select_node
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
		// END Test with id = 589

		// START Test with id = 590
			initSelectClauseForComp(&the_select_node, NULL, false, 0, NULL, getTablesHead(), PTR_TYPE_TABLE_INFO, NULL, NULL
								   ,&malloced_head, the_debug);

			col_arr = (struct col_in_select_node**) myMalloc(sizeof(struct col_in_select_node*) * 2, NULL, &malloced_head, the_debug);
			
			col_arr[0] = (struct col_in_select_node*) myMalloc(sizeof(struct col_in_select_node), NULL, &malloced_head, the_debug);
			col_arr[0]->table_ptr = NULL;
			col_arr[0]->table_ptr_type = -1;
			col_arr[0]->col_ptr = NULL;
			col_arr[0]->col_ptr_type = -1;
			col_arr[0]->new_name = upper("CNT", NULL, &malloced_head, the_debug);
			col_arr[0]->math_node = NULL;
			col_arr[0]->case_node = NULL;

			col_arr[0]->func_node = (struct func_node*) myMalloc(sizeof(struct func_node), NULL, &malloced_head, the_debug);
			col_arr[0]->func_node->which_func = FUNC_COUNT;
			col_arr[0]->func_node->distinct = false;
			col_arr[0]->func_node->args_size = 7;
			col_arr[0]->func_node->args_arr = (void**) myMalloc(sizeof(void*) * 7, NULL, &malloced_head, the_debug);
			col_arr[0]->func_node->args_arr_type = (int*) myMalloc(sizeof(int) * 7, NULL, &malloced_head, the_debug);
			for (int a=0; a<7; a++)
			{
				col_arr[0]->func_node->args_arr[a] = the_select_node->columns_arr[a];
				col_arr[0]->func_node->args_arr_type[a] = PTR_TYPE_COL_IN_SELECT_NODE;
			}
			col_arr[0]->func_node->group_by_cols_head = NULL;
			col_arr[0]->func_node->group_by_cols_tail = NULL;
			addListNodePtr(&col_arr[0]->func_node->group_by_cols_head, &col_arr[0]->func_node->group_by_cols_tail
						  ,((struct select_node*) the_select_node)->columns_arr[0], PTR_TYPE_COL_IN_SELECT_NODE, ADDLISTNODE_TAIL
				  		  ,NULL, &malloced_head, the_debug);
			col_arr[0]->func_node->result_type = PTR_TYPE_INT;

			col_arr[0]->rows_data_type = col_arr[0]->func_node->result_type;

			col_arr[1] = (struct col_in_select_node*) myMalloc(sizeof(struct col_in_select_node), NULL, &malloced_head, the_debug);
			col_arr[1]->table_ptr = the_select_node;
			col_arr[1]->table_ptr_type = PTR_TYPE_SELECT_NODE;
			col_arr[1]->col_ptr = ((struct select_node*) the_select_node)->columns_arr[0];
			col_arr[1]->col_ptr_type = PTR_TYPE_COL_IN_SELECT_NODE;
			col_arr[1]->new_name = NULL;
			col_arr[1]->math_node = NULL;
			col_arr[1]->func_node = NULL;
			col_arr[1]->case_node = NULL;
			col_arr[1]->rows_data_type = ((struct select_node*) the_select_node)->columns_arr[0]->rows_data_type;

			initSelectClauseForComp(&the_select_node->next, NULL, false, 2, col_arr, NULL, -1, NULL, NULL
								   ,&malloced_head, the_debug);
			the_select_node->next->prev = the_select_node;

			the_select_node->next->having_head = (struct where_clause_node*) myMalloc(sizeof(struct where_clause_node), NULL, &malloced_head, the_debug);

			the_select_node->next->having_head->ptr_one = myMalloc(sizeof(struct func_node), NULL, &malloced_head, the_debug);
			((struct func_node*) the_select_node->next->having_head->ptr_one)->which_func = FUNC_AVG;
			((struct func_node*) the_select_node->next->having_head->ptr_one)->distinct = false;
			((struct func_node*) the_select_node->next->having_head->ptr_one)->args_size = 1;
			((struct func_node*) the_select_node->next->having_head->ptr_one)->args_arr = myMalloc(sizeof(void*), NULL, &malloced_head, the_debug);
			((struct func_node*) the_select_node->next->having_head->ptr_one)->args_arr[0] = the_select_node->columns_arr[1];
			((struct func_node*) the_select_node->next->having_head->ptr_one)->args_arr_type = myMalloc(sizeof(int), NULL, &malloced_head, the_debug);
			((struct func_node*) the_select_node->next->having_head->ptr_one)->args_arr_type[0] = PTR_TYPE_COL_IN_SELECT_NODE;
			((struct func_node*) the_select_node->next->having_head->ptr_one)->group_by_cols_head = NULL;
			((struct func_node*) the_select_node->next->having_head->ptr_one)->group_by_cols_tail = NULL;
			//addListNodePtr(&((struct func_node*) the_select_node->next->having_head->ptr_one)->group_by_cols_head, &((struct func_node*) the_select_node->next->having_head->ptr_one)->group_by_cols_tail
			//			  ,the_select_node->columns_arr[0], PTR_TYPE_COL_IN_SELECT_NODE, ADDLISTNODE_TAIL
			//	  		  ,NULL, &malloced_head, the_debug);
			((struct func_node*) the_select_node->next->having_head->ptr_one)->result_type = PTR_TYPE_REAL;
			the_select_node->next->having_head->ptr_one_type = PTR_TYPE_FUNC_NODE;

			the_select_node->next->having_head->ptr_two = (double*) myMalloc(sizeof(double), NULL, &malloced_head, the_debug);
			*((double*) the_select_node->next->having_head->ptr_two) = 1.0;
			the_select_node->next->having_head->ptr_two_type = PTR_TYPE_REAL;

			the_select_node->next->having_head->where_type = WHERE_GREATER_THAN;
			the_select_node->next->having_head->parent = NULL;


			if (test_Controller_parseSelect(590, "select count( *) CNT,  BRAND_NAME from LIQUOR_LICENSES group by BRAND_NAME having avg(CT_REGISTRATION_NUMBER) > 1.0", &the_select_node
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
		// END Test with id = 590

		// START Test with id = 591
			initSelectClauseForComp(&the_select_node, NULL, false, 0, NULL, getTablesHead(), PTR_TYPE_TABLE_INFO, NULL, NULL
								   ,&malloced_head, the_debug);

			col_arr = (struct col_in_select_node**) myMalloc(sizeof(struct col_in_select_node*) * 8, NULL, &malloced_head, the_debug);

			for (int i=0; i<7; i++)
			{
				col_arr[i] = (struct col_in_select_node*) myMalloc(sizeof(struct col_in_select_node), NULL, &malloced_head, the_debug);
				col_arr[i]->table_ptr = the_select_node;
				col_arr[i]->table_ptr_type = PTR_TYPE_SELECT_NODE;
				col_arr[i]->col_ptr = ((struct select_node*) the_select_node)->columns_arr[i];
				col_arr[i]->col_ptr_type = PTR_TYPE_COL_IN_SELECT_NODE;
				col_arr[i]->new_name = NULL;
				col_arr[i]->func_node = NULL;
				col_arr[i]->math_node = NULL;
				col_arr[i]->case_node = NULL;
				col_arr[i]->rows_data_type = ((struct select_node*) the_select_node)->columns_arr[i]->rows_data_type;
			}

			col_arr[7] = (struct col_in_select_node*) myMalloc(sizeof(struct col_in_select_node), NULL, &malloced_head, the_debug);
			col_arr[7]->table_ptr = NULL;
			col_arr[7]->table_ptr_type = -1;
			col_arr[7]->col_ptr = NULL;
			col_arr[7]->col_ptr_type = -1;
			col_arr[7]->new_name = upper("ONE", NULL, &malloced_head, the_debug);
			col_arr[7]->func_node = NULL;
			col_arr[7]->math_node = NULL;
			col_arr[7]->rows_data_type = PTR_TYPE_INT;
			col_arr[7]->case_node = myMalloc(sizeof(struct case_node), NULL, &malloced_head, the_debug);
			col_arr[7]->case_node->case_when_head = NULL;
			col_arr[7]->case_node->case_when_tail = NULL;
			addListNodePtr(&col_arr[7]->case_node->case_when_head, &col_arr[7]->case_node->case_when_tail, NULL, -1, ADDLISTNODE_TAIL
				 		 ,NULL, &malloced_head, the_debug);
			col_arr[7]->case_node->case_then_value_head = NULL;
			col_arr[7]->case_node->case_then_value_tail = NULL;

			int1 = myMalloc(sizeof(int), NULL, &malloced_head, the_debug);
			*int1 = 1;
			addListNodePtr(&col_arr[7]->case_node->case_then_value_head, &col_arr[7]->case_node->case_then_value_tail, int1, PTR_TYPE_INT, ADDLISTNODE_TAIL
				 		 ,NULL, &malloced_head, the_debug);


			initSelectClauseForComp(&the_select_node->next, NULL, false, 8, col_arr, NULL, -1, NULL, NULL
								   ,&malloced_head, the_debug);
			the_select_node->next->prev = the_select_node;
			

			if (test_Controller_parseSelect(591, "select *, 1 ONE from LIQUOR_LICENSES", &the_select_node
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
		// END Test with id = 591

		// START Test with id = 592
			initSelectClauseForComp(&the_select_node, NULL, false, 0, NULL, getTablesHead(), PTR_TYPE_TABLE_INFO, NULL, NULL
								   ,&malloced_head, the_debug);

			col_arr = (struct col_in_select_node**) myMalloc(sizeof(struct col_in_select_node*) * 8, NULL, &malloced_head, the_debug);

			for (int i=0; i<7; i++)
			{
				col_arr[i] = (struct col_in_select_node*) myMalloc(sizeof(struct col_in_select_node), NULL, &malloced_head, the_debug);
				col_arr[i]->table_ptr = the_select_node;
				col_arr[i]->table_ptr_type = PTR_TYPE_SELECT_NODE;
				col_arr[i]->col_ptr = ((struct select_node*) the_select_node)->columns_arr[i];
				col_arr[i]->col_ptr_type = PTR_TYPE_COL_IN_SELECT_NODE;
				col_arr[i]->new_name = NULL;
				col_arr[i]->func_node = NULL;
				col_arr[i]->math_node = NULL;
				col_arr[i]->case_node = NULL;
				col_arr[i]->rows_data_type = ((struct select_node*) the_select_node)->columns_arr[i]->rows_data_type;
			}

			col_arr[7] = (struct col_in_select_node*) myMalloc(sizeof(struct col_in_select_node), NULL, &malloced_head, the_debug);
			col_arr[7]->table_ptr = NULL;
			col_arr[7]->table_ptr_type = -1;
			col_arr[7]->col_ptr = NULL;
			col_arr[7]->col_ptr_type = -1;
			col_arr[7]->new_name = upper("new_case", NULL, &malloced_head, the_debug);
			col_arr[7]->func_node = NULL;
			col_arr[7]->math_node = NULL;
			col_arr[7]->rows_data_type = PTR_TYPE_INT;

			col_arr[7]->case_node = (struct case_node*) myMalloc(sizeof(struct case_node), NULL, &malloced_head, the_debug);
			col_arr[7]->case_node->case_when_head = NULL;
			col_arr[7]->case_node->case_then_value_head = NULL;

			temp_where = (struct where_clause_node*) myMalloc(sizeof(struct where_clause_node), NULL, &malloced_head, the_debug);
			temp_where->ptr_one = the_select_node->columns_arr[4];
			temp_where->ptr_one_type = PTR_TYPE_COL_IN_SELECT_NODE;
			temp_where->ptr_two = (char*) myMalloc(sizeof(char) * 16, NULL, &malloced_head, the_debug);
			strcpy(temp_where->ptr_two, "1/31/2025");
			temp_where->ptr_two_type = PTR_TYPE_DATE;
			temp_where->where_type = WHERE_IS_EQUALS;
			temp_where->parent = NULL;

			addListNodePtr(&col_arr[7]->case_node->case_when_head, &col_arr[7]->case_node->case_when_tail, temp_where, PTR_TYPE_WHERE_CLAUSE_NODE, ADDLISTNODE_TAIL
						  ,NULL, &malloced_head, the_debug);
			addListNodePtr(&col_arr[7]->case_node->case_when_head, &col_arr[7]->case_node->case_when_tail, NULL, -1, ADDLISTNODE_TAIL
						  ,NULL, &malloced_head, the_debug);

			int1 = myMalloc(sizeof(int), NULL, &malloced_head, the_debug);
			*int1 = 1;

			addListNodePtr(&col_arr[7]->case_node->case_then_value_head, &col_arr[7]->case_node->case_then_value_tail, int1, PTR_TYPE_INT, ADDLISTNODE_TAIL
						  ,NULL, &malloced_head, the_debug);
			addListNodePtr(&col_arr[7]->case_node->case_then_value_head, &col_arr[7]->case_node->case_then_value_tail, NULL, -1, ADDLISTNODE_TAIL
						  ,NULL, &malloced_head, the_debug);


			initSelectClauseForComp(&the_select_node->next, NULL, false, 8, col_arr, NULL, -1, NULL, NULL
								   ,&malloced_head, the_debug);
			the_select_node->next->prev = the_select_node;
			

			if (test_Controller_parseSelect(592, "select *, case when EXPIRATION = '1/31/2025' then 1 end new_case from LIQUOR_LICENSES", &the_select_node
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
		// END Test with id = 592

		// START Test with id = 593
			initSelectClauseForComp(&the_select_node, NULL, false, 0, NULL, getTablesHead(), PTR_TYPE_TABLE_INFO, NULL, NULL
								   ,&malloced_head, the_debug);

			col_arr = (struct col_in_select_node**) myMalloc(sizeof(struct col_in_select_node*) * 2, NULL, &malloced_head, the_debug);
			
			col_arr[0] = (struct col_in_select_node*) myMalloc(sizeof(struct col_in_select_node), NULL, &malloced_head, the_debug);
			col_arr[0]->table_ptr = NULL;
			col_arr[0]->table_ptr_type = -1;
			col_arr[0]->col_ptr = NULL;
			col_arr[0]->col_ptr_type = -1;
			col_arr[0]->new_name = upper("CNT", NULL, &malloced_head, the_debug);
			col_arr[0]->math_node = NULL;
			col_arr[0]->case_node = NULL;

			col_arr[0]->func_node = (struct func_node*) myMalloc(sizeof(struct func_node), NULL, &malloced_head, the_debug);
			col_arr[0]->func_node->which_func = FUNC_COUNT;
			col_arr[0]->func_node->distinct = false;
			col_arr[0]->func_node->args_size = 7;
			col_arr[0]->func_node->args_arr = (void**) myMalloc(sizeof(void*) * 7, NULL, &malloced_head, the_debug);
			col_arr[0]->func_node->args_arr_type = (int*) myMalloc(sizeof(int) * 7, NULL, &malloced_head, the_debug);
			for (int a=0; a<7; a++)
			{
				col_arr[0]->func_node->args_arr[a] = the_select_node->columns_arr[a];
				col_arr[0]->func_node->args_arr_type[a] = PTR_TYPE_COL_IN_SELECT_NODE;
			}
			col_arr[0]->func_node->group_by_cols_head = NULL;
			col_arr[0]->func_node->group_by_cols_tail = NULL;
			addListNodePtr(&col_arr[0]->func_node->group_by_cols_head, &col_arr[0]->func_node->group_by_cols_tail
						  ,((struct select_node*) the_select_node)->columns_arr[0], PTR_TYPE_COL_IN_SELECT_NODE, ADDLISTNODE_TAIL
				  		  ,NULL, &malloced_head, the_debug);
			col_arr[0]->func_node->result_type = PTR_TYPE_INT;

			col_arr[0]->rows_data_type = col_arr[0]->func_node->result_type;

			col_arr[1] = (struct col_in_select_node*) myMalloc(sizeof(struct col_in_select_node), NULL, &malloced_head, the_debug);
			col_arr[1]->table_ptr = the_select_node;
			col_arr[1]->table_ptr_type = PTR_TYPE_SELECT_NODE;
			col_arr[1]->col_ptr = ((struct select_node*) the_select_node)->columns_arr[0];
			col_arr[1]->col_ptr_type = PTR_TYPE_COL_IN_SELECT_NODE;
			col_arr[1]->new_name = NULL;
			col_arr[1]->math_node = NULL;
			col_arr[1]->func_node = NULL;
			col_arr[1]->case_node = NULL;
			col_arr[1]->rows_data_type = ((struct select_node*) the_select_node)->columns_arr[0]->rows_data_type;

			initSelectClauseForComp(&the_select_node->next, NULL, false, 2, col_arr, NULL, -1, NULL, NULL
								   ,&malloced_head, the_debug);
			the_select_node->next->prev = the_select_node;

			the_select_node->next->having_head = (struct where_clause_node*) myMalloc(sizeof(struct where_clause_node), NULL, &malloced_head, the_debug);

			the_select_node->next->having_head->ptr_one = myMalloc(sizeof(struct math_node), NULL, &malloced_head, the_debug);
			((struct math_node*) the_select_node->next->having_head->ptr_one)->ptr_one = myMalloc(sizeof(struct func_node), NULL, &malloced_head, the_debug);
			
			((struct func_node*) ((struct math_node*) the_select_node->next->having_head->ptr_one)->ptr_one)->which_func = FUNC_AVG;
			((struct func_node*) ((struct math_node*) the_select_node->next->having_head->ptr_one)->ptr_one)->distinct = false;
			((struct func_node*) ((struct math_node*) the_select_node->next->having_head->ptr_one)->ptr_one)->args_size = 1;
			((struct func_node*) ((struct math_node*) the_select_node->next->having_head->ptr_one)->ptr_one)->args_arr = myMalloc(sizeof(void*), NULL, &malloced_head, the_debug);
			((struct func_node*) ((struct math_node*) the_select_node->next->having_head->ptr_one)->ptr_one)->args_arr[0] = the_select_node->columns_arr[1];
			((struct func_node*) ((struct math_node*) the_select_node->next->having_head->ptr_one)->ptr_one)->args_arr_type = myMalloc(sizeof(int), NULL, &malloced_head, the_debug);
			((struct func_node*) ((struct math_node*) the_select_node->next->having_head->ptr_one)->ptr_one)->args_arr_type[0] = PTR_TYPE_COL_IN_SELECT_NODE;
			((struct func_node*) ((struct math_node*) the_select_node->next->having_head->ptr_one)->ptr_one)->group_by_cols_head = NULL;
			((struct func_node*) ((struct math_node*) the_select_node->next->having_head->ptr_one)->ptr_one)->group_by_cols_tail = NULL;
			((struct func_node*) ((struct math_node*) the_select_node->next->having_head->ptr_one)->ptr_one)->result_type = PTR_TYPE_REAL;

			((struct math_node*) the_select_node->next->having_head->ptr_one)->ptr_one_type = PTR_TYPE_FUNC_NODE;
			((struct math_node*) the_select_node->next->having_head->ptr_one)->ptr_two = myMalloc(sizeof(int), NULL, &malloced_head, the_debug);
			*((int*) ((struct math_node*) the_select_node->next->having_head->ptr_one)->ptr_two) = 10;
			((struct math_node*) the_select_node->next->having_head->ptr_one)->ptr_two_type = PTR_TYPE_INT;
			((struct math_node*) the_select_node->next->having_head->ptr_one)->operation = MATH_MULT;
			((struct math_node*) the_select_node->next->having_head->ptr_one)->parent = NULL;
			((struct math_node*) the_select_node->next->having_head->ptr_one)->result_type = PTR_TYPE_REAL;

			the_select_node->next->having_head->ptr_one_type = PTR_TYPE_MATH_NODE;

			the_select_node->next->having_head->ptr_two = (double*) myMalloc(sizeof(double), NULL, &malloced_head, the_debug);
			*((double*) the_select_node->next->having_head->ptr_two) = 1.0;
			the_select_node->next->having_head->ptr_two_type = PTR_TYPE_REAL;

			the_select_node->next->having_head->where_type = WHERE_GREATER_THAN;
			the_select_node->next->having_head->parent = NULL;


			if (test_Controller_parseSelect(593, "select count( *) CNT,  BRAND_NAME from LIQUOR_LICENSES group by BRAND_NAME having avg(CT_REGISTRATION_NUMBER) * 10 > 1.0", &the_select_node
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
		// END Test with id = 593

		// START Test with id = 594
			initSelectClauseForComp(&the_select_node, NULL, false, 0, NULL, getTablesHead(), PTR_TYPE_TABLE_INFO, NULL, NULL
								   ,&malloced_head, the_debug);

			col_arr = (struct col_in_select_node**) myMalloc(sizeof(struct col_in_select_node*) * 2, NULL, &malloced_head, the_debug);
			
			col_arr[0] = (struct col_in_select_node*) myMalloc(sizeof(struct col_in_select_node), NULL, &malloced_head, the_debug);
			col_arr[0]->table_ptr = NULL;
			col_arr[0]->table_ptr_type = -1;
			col_arr[0]->col_ptr = NULL;
			col_arr[0]->col_ptr_type = -1;
			col_arr[0]->new_name = upper("CNT", NULL, &malloced_head, the_debug);
			col_arr[0]->math_node = NULL;
			col_arr[0]->case_node = NULL;

			col_arr[0]->func_node = (struct func_node*) myMalloc(sizeof(struct func_node), NULL, &malloced_head, the_debug);
			col_arr[0]->func_node->which_func = FUNC_COUNT;
			col_arr[0]->func_node->distinct = false;
			col_arr[0]->func_node->args_size = 7;
			col_arr[0]->func_node->args_arr = (void**) myMalloc(sizeof(void*) * 7, NULL, &malloced_head, the_debug);
			col_arr[0]->func_node->args_arr_type = (int*) myMalloc(sizeof(int) * 7, NULL, &malloced_head, the_debug);
			for (int a=0; a<7; a++)
			{
				col_arr[0]->func_node->args_arr[a] = the_select_node->columns_arr[a];
				col_arr[0]->func_node->args_arr_type[a] = PTR_TYPE_COL_IN_SELECT_NODE;
			}
			col_arr[0]->func_node->group_by_cols_head = NULL;
			col_arr[0]->func_node->group_by_cols_tail = NULL;
			addListNodePtr(&col_arr[0]->func_node->group_by_cols_head, &col_arr[0]->func_node->group_by_cols_tail
						  ,((struct select_node*) the_select_node)->columns_arr[0], PTR_TYPE_COL_IN_SELECT_NODE, ADDLISTNODE_TAIL
				  		  ,NULL, &malloced_head, the_debug);
			col_arr[0]->func_node->result_type = PTR_TYPE_INT;

			col_arr[0]->rows_data_type = col_arr[0]->func_node->result_type;

			col_arr[1] = (struct col_in_select_node*) myMalloc(sizeof(struct col_in_select_node), NULL, &malloced_head, the_debug);
			col_arr[1]->table_ptr = the_select_node;
			col_arr[1]->table_ptr_type = PTR_TYPE_SELECT_NODE;
			col_arr[1]->col_ptr = ((struct select_node*) the_select_node)->columns_arr[0];
			col_arr[1]->col_ptr_type = PTR_TYPE_COL_IN_SELECT_NODE;
			col_arr[1]->new_name = NULL;
			col_arr[1]->math_node = NULL;
			col_arr[1]->func_node = NULL;
			col_arr[1]->case_node = NULL;
			col_arr[1]->rows_data_type = ((struct select_node*) the_select_node)->columns_arr[0]->rows_data_type;

			initSelectClauseForComp(&the_select_node->next, NULL, false, 2, col_arr, NULL, -1, NULL, NULL
								   ,&malloced_head, the_debug);
			the_select_node->next->prev = the_select_node;

			the_select_node->next->having_head = (struct where_clause_node*) myMalloc(sizeof(struct where_clause_node), NULL, &malloced_head, the_debug);

			the_select_node->next->having_head->ptr_one = myMalloc(sizeof(struct math_node), NULL, &malloced_head, the_debug);
			((struct math_node*) the_select_node->next->having_head->ptr_one)->ptr_one = myMalloc(sizeof(int), NULL, &malloced_head, the_debug);
			*((int*) ((struct math_node*) the_select_node->next->having_head->ptr_one)->ptr_one) = 10;
			((struct math_node*) the_select_node->next->having_head->ptr_one)->ptr_one_type = PTR_TYPE_INT;

			((struct math_node*) the_select_node->next->having_head->ptr_one)->ptr_two = myMalloc(sizeof(struct func_node), NULL, &malloced_head, the_debug);
			
			((struct func_node*) ((struct math_node*) the_select_node->next->having_head->ptr_one)->ptr_two)->which_func = FUNC_AVG;
			((struct func_node*) ((struct math_node*) the_select_node->next->having_head->ptr_one)->ptr_two)->distinct = false;
			((struct func_node*) ((struct math_node*) the_select_node->next->having_head->ptr_one)->ptr_two)->args_size = 1;
			((struct func_node*) ((struct math_node*) the_select_node->next->having_head->ptr_one)->ptr_two)->args_arr = myMalloc(sizeof(void*), NULL, &malloced_head, the_debug);
			((struct func_node*) ((struct math_node*) the_select_node->next->having_head->ptr_one)->ptr_two)->args_arr[0] = the_select_node->columns_arr[1];
			((struct func_node*) ((struct math_node*) the_select_node->next->having_head->ptr_one)->ptr_two)->args_arr_type = myMalloc(sizeof(int), NULL, &malloced_head, the_debug);
			((struct func_node*) ((struct math_node*) the_select_node->next->having_head->ptr_one)->ptr_two)->args_arr_type[0] = PTR_TYPE_COL_IN_SELECT_NODE;
			((struct func_node*) ((struct math_node*) the_select_node->next->having_head->ptr_one)->ptr_two)->group_by_cols_head = NULL;
			((struct func_node*) ((struct math_node*) the_select_node->next->having_head->ptr_one)->ptr_two)->group_by_cols_tail = NULL;
			((struct func_node*) ((struct math_node*) the_select_node->next->having_head->ptr_one)->ptr_two)->result_type = PTR_TYPE_REAL;

			((struct math_node*) the_select_node->next->having_head->ptr_one)->ptr_two_type = PTR_TYPE_FUNC_NODE;
			((struct math_node*) the_select_node->next->having_head->ptr_one)->operation = MATH_MULT;
			((struct math_node*) the_select_node->next->having_head->ptr_one)->parent = NULL;
			((struct math_node*) the_select_node->next->having_head->ptr_one)->result_type = PTR_TYPE_REAL;

			the_select_node->next->having_head->ptr_one_type = PTR_TYPE_MATH_NODE;

			the_select_node->next->having_head->ptr_two = (double*) myMalloc(sizeof(double), NULL, &malloced_head, the_debug);
			*((double*) the_select_node->next->having_head->ptr_two) = 1.0;
			the_select_node->next->having_head->ptr_two_type = PTR_TYPE_REAL;

			the_select_node->next->having_head->where_type = WHERE_GREATER_THAN;
			the_select_node->next->having_head->parent = NULL;


			if (test_Controller_parseSelect(594, "select count( *) CNT,  BRAND_NAME from LIQUOR_LICENSES group by BRAND_NAME having 10 * avg(CT_REGISTRATION_NUMBER) > 1.0", &the_select_node
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
		// END Test with id = 594

		// START Test with id = 595
			initSelectClauseForComp(&the_select_node, NULL, false, 0, NULL, getTablesHead(), PTR_TYPE_TABLE_INFO, NULL, NULL
								   ,&malloced_head, the_debug);

			col_arr = (struct col_in_select_node**) myMalloc(sizeof(struct col_in_select_node*) * 7, NULL, &malloced_head, the_debug);

			for (int i=0; i<7; i++)
			{
				col_arr[i] = (struct col_in_select_node*) myMalloc(sizeof(struct col_in_select_node), NULL, &malloced_head, the_debug);
				col_arr[i]->table_ptr = the_select_node;
				col_arr[i]->table_ptr_type = PTR_TYPE_SELECT_NODE;
				col_arr[i]->col_ptr = ((struct select_node*) the_select_node)->columns_arr[i];
				col_arr[i]->col_ptr_type = PTR_TYPE_COL_IN_SELECT_NODE;
				col_arr[i]->new_name = NULL;
				col_arr[i]->func_node = NULL;
				col_arr[i]->math_node = NULL;
				col_arr[i]->case_node = NULL;
				col_arr[i]->rows_data_type = ((struct select_node*) the_select_node)->columns_arr[i]->rows_data_type;
			}

			initSelectClauseForComp(&the_select_node->next, NULL, false, 7, col_arr, NULL, -1, NULL, NULL
								   ,&malloced_head, the_debug);
			the_select_node->next->prev = the_select_node;


			struct case_node* temp_case_node = (struct case_node*) myMalloc(sizeof(struct case_node), NULL, &malloced_head, the_debug);
			temp_case_node->case_when_head = NULL;
			temp_case_node->case_then_value_head = NULL;

			temp_where = (struct where_clause_node*) myMalloc(sizeof(struct where_clause_node), NULL, &malloced_head, the_debug);
			temp_where->ptr_one = the_select_node->columns_arr[4];
			temp_where->ptr_one_type = PTR_TYPE_COL_IN_SELECT_NODE;
			temp_where->ptr_two = (char*) myMalloc(sizeof(char) * 16, NULL, &malloced_head, the_debug);
			strcpy(temp_where->ptr_two, "1/31/2025");
			temp_where->ptr_two_type = PTR_TYPE_DATE;
			temp_where->where_type = WHERE_IS_EQUALS;
			temp_where->parent = NULL;

			addListNodePtr(&temp_case_node->case_when_head, &temp_case_node->case_when_tail, temp_where, PTR_TYPE_WHERE_CLAUSE_NODE, ADDLISTNODE_TAIL
						  ,NULL, &malloced_head, the_debug);
			addListNodePtr(&temp_case_node->case_when_head, &temp_case_node->case_when_tail, NULL, -1, ADDLISTNODE_TAIL
						  ,NULL, &malloced_head, the_debug);

			int1 = myMalloc(sizeof(int), NULL, &malloced_head, the_debug);
			*int1 = 1;

			addListNodePtr(&temp_case_node->case_then_value_head, &temp_case_node->case_then_value_tail, int1, PTR_TYPE_INT, ADDLISTNODE_TAIL
						  ,NULL, &malloced_head, the_debug);
			addListNodePtr(&temp_case_node->case_then_value_head, &temp_case_node->case_then_value_tail, NULL, -1, ADDLISTNODE_TAIL
						  ,NULL, &malloced_head, the_debug);


			the_select_node->next->where_head = myMalloc(sizeof(struct where_clause_node), NULL, &malloced_head, the_debug);
			the_select_node->next->where_head->ptr_one = temp_case_node;
			the_select_node->next->where_head->ptr_one_type = PTR_TYPE_CASE_NODE;
			the_select_node->next->where_head->ptr_two = myMalloc(sizeof(int), NULL, &malloced_head, the_debug);
			*((int*) the_select_node->next->where_head->ptr_two) = 1;
			the_select_node->next->where_head->ptr_two_type = PTR_TYPE_INT;
			the_select_node->next->where_head->where_type = WHERE_IS_EQUALS;
			the_select_node->next->where_head->parent = NULL;
			

			if (test_Controller_parseSelect(595, "select * from LIQUOR_LICENSES where case when EXPIRATION = '1/31/2025' then 1 end = 1;", &the_select_node
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
		// END Test with id = 595

		// START Test with id = 596
			initSelectClauseForComp(&the_select_node, NULL, false, 0, NULL, getTablesHead(), PTR_TYPE_TABLE_INFO, NULL, NULL
								   ,&malloced_head, the_debug);

			col_arr = (struct col_in_select_node**) myMalloc(sizeof(struct col_in_select_node*) * 7, NULL, &malloced_head, the_debug);

			for (int i=0; i<7; i++)
			{
				col_arr[i] = (struct col_in_select_node*) myMalloc(sizeof(struct col_in_select_node), NULL, &malloced_head, the_debug);
				col_arr[i]->table_ptr = the_select_node;
				col_arr[i]->table_ptr_type = PTR_TYPE_SELECT_NODE;
				col_arr[i]->col_ptr = ((struct select_node*) the_select_node)->columns_arr[i];
				col_arr[i]->col_ptr_type = PTR_TYPE_COL_IN_SELECT_NODE;
				col_arr[i]->new_name = NULL;
				col_arr[i]->func_node = NULL;
				col_arr[i]->math_node = NULL;
				col_arr[i]->case_node = NULL;
				col_arr[i]->rows_data_type = ((struct select_node*) the_select_node)->columns_arr[i]->rows_data_type;
			}

			initSelectClauseForComp(&the_select_node->next, NULL, false, 7, col_arr, NULL, -1, NULL, NULL
								   ,&malloced_head, the_debug);
			the_select_node->next->prev = the_select_node;


			temp_case_node = (struct case_node*) myMalloc(sizeof(struct case_node), NULL, &malloced_head, the_debug);
			temp_case_node->case_when_head = NULL;
			temp_case_node->case_then_value_head = NULL;

			temp_where = (struct where_clause_node*) myMalloc(sizeof(struct where_clause_node), NULL, &malloced_head, the_debug);
			temp_where->ptr_one = the_select_node->columns_arr[4];
			temp_where->ptr_one_type = PTR_TYPE_COL_IN_SELECT_NODE;
			temp_where->ptr_two = (char*) myMalloc(sizeof(char) * 16, NULL, &malloced_head, the_debug);
			strcpy(temp_where->ptr_two, "1/31/2025");
			temp_where->ptr_two_type = PTR_TYPE_DATE;
			temp_where->where_type = WHERE_IS_EQUALS;
			temp_where->parent = NULL;

			addListNodePtr(&temp_case_node->case_when_head, &temp_case_node->case_when_tail, temp_where, PTR_TYPE_WHERE_CLAUSE_NODE, ADDLISTNODE_TAIL
						  ,NULL, &malloced_head, the_debug);
			addListNodePtr(&temp_case_node->case_when_head, &temp_case_node->case_when_tail, NULL, -1, ADDLISTNODE_TAIL
						  ,NULL, &malloced_head, the_debug);

			int1 = myMalloc(sizeof(int), NULL, &malloced_head, the_debug);
			*int1 = 1;

			addListNodePtr(&temp_case_node->case_then_value_head, &temp_case_node->case_then_value_tail, int1, PTR_TYPE_INT, ADDLISTNODE_TAIL
						  ,NULL, &malloced_head, the_debug);
			addListNodePtr(&temp_case_node->case_then_value_head, &temp_case_node->case_then_value_tail, NULL, -1, ADDLISTNODE_TAIL
						  ,NULL, &malloced_head, the_debug);


			the_select_node->next->where_head = myMalloc(sizeof(struct where_clause_node), NULL, &malloced_head, the_debug);
			the_select_node->next->where_head->ptr_one = myMalloc(sizeof(int), NULL, &malloced_head, the_debug);
			*((int*) the_select_node->next->where_head->ptr_one) = 1;
			the_select_node->next->where_head->ptr_one_type = PTR_TYPE_INT;
			the_select_node->next->where_head->ptr_two = temp_case_node;
			the_select_node->next->where_head->ptr_two_type = PTR_TYPE_CASE_NODE;
			the_select_node->next->where_head->where_type = WHERE_IS_EQUALS;
			the_select_node->next->where_head->parent = NULL;
			

			if (test_Controller_parseSelect(596, "select * from LIQUOR_LICENSES where 1 = case when EXPIRATION = '1/31/2025' then 1 end;", &the_select_node
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
		// END Test with id = 596

		// START Test with id = 597
			initSelectClauseForComp(&the_select_node, NULL, false, 0, NULL, getTablesHead(), PTR_TYPE_TABLE_INFO, NULL, NULL
								   ,&malloced_head, the_debug);

			col_arr = (struct col_in_select_node**) myMalloc(sizeof(struct col_in_select_node*), NULL, &malloced_head, the_debug);

			col_arr[0] = (struct col_in_select_node*) myMalloc(sizeof(struct col_in_select_node), NULL, &malloced_head, the_debug);
			col_arr[0]->table_ptr = NULL;
			col_arr[0]->table_ptr_type = -1;
			col_arr[0]->col_ptr = NULL;
			col_arr[0]->col_ptr_type = -1;
			col_arr[0]->new_name = upper("MATH", NULL, &malloced_head, the_debug);
			col_arr[0]->func_node = NULL;
			col_arr[0]->math_node = NULL;
			col_arr[0]->case_node = NULL;
			col_arr[0]->rows_data_type = PTR_TYPE_INT;

			temp_case_node = (struct case_node*) myMalloc(sizeof(struct case_node), NULL, &malloced_head, the_debug);
			temp_case_node->case_when_head = NULL;
			temp_case_node->case_then_value_head = NULL;

			temp_where = (struct where_clause_node*) myMalloc(sizeof(struct where_clause_node), NULL, &malloced_head, the_debug);
			temp_where->ptr_one = the_select_node->columns_arr[4];
			temp_where->ptr_one_type = PTR_TYPE_COL_IN_SELECT_NODE;
			temp_where->ptr_two = (char*) myMalloc(sizeof(char) * 16, NULL, &malloced_head, the_debug);
			strcpy(temp_where->ptr_two, "1/31/2025");
			temp_where->ptr_two_type = PTR_TYPE_DATE;
			temp_where->where_type = WHERE_IS_EQUALS;
			temp_where->parent = NULL;

			addListNodePtr(&temp_case_node->case_when_head, &temp_case_node->case_when_tail, temp_where, PTR_TYPE_WHERE_CLAUSE_NODE, ADDLISTNODE_TAIL
						  ,NULL, &malloced_head, the_debug);
			addListNodePtr(&temp_case_node->case_when_head, &temp_case_node->case_when_tail, NULL, -1, ADDLISTNODE_TAIL
						  ,NULL, &malloced_head, the_debug);

			int1 = myMalloc(sizeof(int), NULL, &malloced_head, the_debug);
			*int1 = 1;
			int2 = myMalloc(sizeof(int), NULL, &malloced_head, the_debug);
			*int2 = 2;

			addListNodePtr(&temp_case_node->case_then_value_head, &temp_case_node->case_then_value_tail, int1, PTR_TYPE_INT, ADDLISTNODE_TAIL
						  ,NULL, &malloced_head, the_debug);
			addListNodePtr(&temp_case_node->case_then_value_head, &temp_case_node->case_then_value_tail, int2, PTR_TYPE_INT, ADDLISTNODE_TAIL
						  ,NULL, &malloced_head, the_debug);

			col_arr[0]->math_node = (struct math_node*) myMalloc(sizeof(struct math_node), NULL, &malloced_head, the_debug);
			col_arr[0]->math_node->ptr_one = (int*) myMalloc(sizeof(int), NULL, &malloced_head, the_debug);
			*((int*) col_arr[0]->math_node->ptr_one) = 100;
			col_arr[0]->math_node->ptr_one_type = PTR_TYPE_INT;
			col_arr[0]->math_node->ptr_two = temp_case_node;
			col_arr[0]->math_node->ptr_two_type = PTR_TYPE_CASE_NODE;
			col_arr[0]->math_node->operation = MATH_MULT;
			col_arr[0]->math_node->parent = NULL;
			col_arr[0]->math_node->result_type = PTR_TYPE_INT;

			initSelectClauseForComp(&the_select_node->next, NULL, false, 1, col_arr, NULL, -1, NULL, NULL
								   ,&malloced_head, the_debug);
			the_select_node->next->prev = the_select_node;


			if (test_Controller_parseSelect(597, "select 100 * case when EXPIRATION = '1/31/2025' then 1 else 2 end MATH from LIQUOR_LICENSES;", &the_select_node
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
		// END Test with id = 597

		// START Test with id = 598
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

			col_arr = (struct col_in_select_node**) myMalloc(sizeof(struct col_in_select_node*) * 13, NULL, &malloced_head, the_debug);

			index = 0;
			for (int i=0; i<13; i++)
			{
				col_arr[index] = (struct col_in_select_node*) myMalloc(sizeof(struct col_in_select_node), NULL, &malloced_head, the_debug);

				if (i < 6)
				{
					col_arr[index]->table_ptr = the_select_node->next;
					col_arr[index]->table_ptr_type = PTR_TYPE_SELECT_NODE;

					col_arr[index]->col_ptr = the_select_node->next->columns_arr[i];
					col_arr[index]->col_ptr_type = PTR_TYPE_COL_IN_SELECT_NODE;

					col_arr[index]->rows_data_type = the_select_node->next->columns_arr[i]->rows_data_type;
				}
				else
				{
					col_arr[index]->table_ptr = joined_select->next;
					col_arr[index]->table_ptr_type = PTR_TYPE_SELECT_NODE;

					col_arr[index]->col_ptr = joined_select->next->columns_arr[i-6];
					col_arr[index]->col_ptr_type = PTR_TYPE_COL_IN_SELECT_NODE;

					col_arr[index]->rows_data_type = joined_select->next->columns_arr[i-6]->rows_data_type;
				}

				col_arr[index]->new_name = NULL;
				col_arr[index]->func_node = NULL;
				col_arr[index]->math_node = NULL;
				col_arr[index]->case_node = NULL;

				index++;
			}

			col_arr[0]->new_name = upper("BRAND_NAME_0_A", NULL, &malloced_head, the_debug);
			col_arr[1]->new_name = upper("CT_REGISTRATION_NUMBER_1_A", NULL, &malloced_head, the_debug);
			col_arr[2]->new_name = upper("STATUS_2_A", NULL, &malloced_head, the_debug);
			col_arr[3]->new_name = upper("EFFECTIVE_3_A", NULL, &malloced_head, the_debug);
			col_arr[4]->new_name = upper("EXPIRATION_4_A", NULL, &malloced_head, the_debug);
			col_arr[5]->new_name = upper("OUT_OF_STATE_SHIPPER_5_A", NULL, &malloced_head, the_debug);
			col_arr[6]->new_name = upper("BRAND_NAME_6_A", NULL, &malloced_head, the_debug);
			col_arr[7]->new_name = upper("CT_REGISTRATION_NUMBER_7_A", NULL, &malloced_head, the_debug);
			col_arr[8]->new_name = upper("STATUS_8_A", NULL, &malloced_head, the_debug);
			col_arr[9]->new_name = upper("EFFECTIVE_9_A", NULL, &malloced_head, the_debug);
			col_arr[10]->new_name = upper("EXPIRATION_10_A", NULL, &malloced_head, the_debug);
			col_arr[11]->new_name = upper("OUT_OF_STATE_SHIPPER_11_A", NULL, &malloced_head, the_debug);
			col_arr[12]->new_name = upper("SUPERVISOR_CREDENTIAL_12_A", NULL, &malloced_head, the_debug);

			initSelectClauseForComp(&the_select_node->next->next, NULL, false, 13, col_arr, NULL, -1, NULL, NULL
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


			if (test_Controller_parseSelect(598, "with tbl2 as (select * from LIQUOR_LICENSES),tbl_bad as (select * from LIQUOR_LICENSES) select * as \"+_#_A\" except tbl.SUPERVISOR_CREDENTIAL  from ( select * from LIQUOR_LICENSES )tbl join tbl2 on tbl.BRAND_NAME = tbl2.BRAND_NAME;", &the_select_node
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
		// END Test with id = 598

		// START Test with id = 599
			initSelectClauseForComp(&the_select_node, NULL, false, 0, NULL, getTablesHead(), PTR_TYPE_TABLE_INFO, NULL, NULL
								   ,&malloced_head, the_debug);

			col_arr = (struct col_in_select_node**) myMalloc(sizeof(struct col_in_select_node*) * 8, NULL, &malloced_head, the_debug);

			for (int i=0; i<7; i++)
			{
				col_arr[i] = (struct col_in_select_node*) myMalloc(sizeof(struct col_in_select_node), NULL, &malloced_head, the_debug);
				col_arr[i]->table_ptr = the_select_node;
				col_arr[i]->table_ptr_type = PTR_TYPE_SELECT_NODE;
				col_arr[i]->col_ptr = ((struct select_node*) the_select_node)->columns_arr[i];
				col_arr[i]->col_ptr_type = PTR_TYPE_COL_IN_SELECT_NODE;
				col_arr[i]->new_name = NULL;
				col_arr[i]->func_node = NULL;
				col_arr[i]->math_node = NULL;
				col_arr[i]->case_node = NULL;
				col_arr[i]->rows_data_type = ((struct select_node*) the_select_node)->columns_arr[i]->rows_data_type;
			}

			col_arr[7] = (struct col_in_select_node*) myMalloc(sizeof(struct col_in_select_node), NULL, &malloced_head, the_debug);
			col_arr[7]->table_ptr = NULL;
			col_arr[7]->table_ptr_type = -1;
			col_arr[7]->col_ptr = NULL;
			col_arr[7]->col_ptr_type = -1;
			col_arr[7]->new_name = upper("new_case", NULL, &malloced_head, the_debug);
			col_arr[7]->func_node = NULL;
			col_arr[7]->math_node = NULL;
			col_arr[7]->rows_data_type = -1;

			col_arr[7]->case_node = (struct case_node*) myMalloc(sizeof(struct case_node), NULL, &malloced_head, the_debug);
			col_arr[7]->case_node->case_when_head = NULL;
			col_arr[7]->case_node->case_then_value_head = NULL;

			temp_where = (struct where_clause_node*) myMalloc(sizeof(struct where_clause_node), NULL, &malloced_head, the_debug);
			temp_where->ptr_one = the_select_node->columns_arr[4];
			temp_where->ptr_one_type = PTR_TYPE_COL_IN_SELECT_NODE;
			temp_where->ptr_two = (char*) myMalloc(sizeof(char) * 16, NULL, &malloced_head, the_debug);
			strcpy(temp_where->ptr_two, "1/31/2025");
			temp_where->ptr_two_type = PTR_TYPE_DATE;
			temp_where->where_type = WHERE_IS_EQUALS;
			temp_where->parent = NULL;

			addListNodePtr(&col_arr[7]->case_node->case_when_head, &col_arr[7]->case_node->case_when_tail, temp_where, PTR_TYPE_WHERE_CLAUSE_NODE, ADDLISTNODE_TAIL
						  ,NULL, &malloced_head, the_debug);
			addListNodePtr(&col_arr[7]->case_node->case_when_head, &col_arr[7]->case_node->case_when_tail, NULL, -1, ADDLISTNODE_TAIL
						  ,NULL, &malloced_head, the_debug);

			addListNodePtr(&col_arr[7]->case_node->case_then_value_head, &col_arr[7]->case_node->case_then_value_tail, NULL, -1, ADDLISTNODE_TAIL
						  ,NULL, &malloced_head, the_debug);
			addListNodePtr(&col_arr[7]->case_node->case_then_value_head, &col_arr[7]->case_node->case_then_value_tail, NULL, -1, ADDLISTNODE_TAIL
						  ,NULL, &malloced_head, the_debug);


			initSelectClauseForComp(&the_select_node->next, NULL, false, 8, col_arr, NULL, -1, NULL, NULL
								   ,&malloced_head, the_debug);
			the_select_node->next->prev = the_select_node;
			

			if (test_Controller_parseSelect(599, "select *, case when EXPIRATION = '1/31/2025' then NULL else null end new_case from LIQUOR_LICENSES", &the_select_node
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
		// END Test with id = 599

		// START Test with id = 5501
			initSelectClauseForComp(&the_select_node, NULL, false, 0, NULL, getTablesHead(), PTR_TYPE_TABLE_INFO, NULL, NULL
								   ,&malloced_head, the_debug);

			col_arr = (struct col_in_select_node**) myMalloc(sizeof(struct col_in_select_node*) * 9, NULL, &malloced_head, the_debug);

			for (int i=0; i<7; i++)
			{
				col_arr[i] = (struct col_in_select_node*) myMalloc(sizeof(struct col_in_select_node), NULL, &malloced_head, the_debug);
				col_arr[i]->table_ptr = the_select_node;
				col_arr[i]->table_ptr_type = PTR_TYPE_SELECT_NODE;
				col_arr[i]->col_ptr = ((struct select_node*) the_select_node)->columns_arr[i];
				col_arr[i]->col_ptr_type = PTR_TYPE_COL_IN_SELECT_NODE;
				col_arr[i]->new_name = NULL;
				col_arr[i]->func_node = NULL;
				col_arr[i]->math_node = NULL;
				col_arr[i]->case_node = NULL;
				col_arr[i]->rows_data_type = ((struct select_node*) the_select_node)->columns_arr[i]->rows_data_type;
			}

			col_arr[7] = (struct col_in_select_node*) myMalloc(sizeof(struct col_in_select_node), NULL, &malloced_head, the_debug);
			col_arr[7]->table_ptr = NULL;
			col_arr[7]->table_ptr_type = -1;
			col_arr[7]->col_ptr = NULL;
			col_arr[7]->col_ptr_type = -1;
			col_arr[7]->new_name = upper("EMPTY_STRING", NULL, &malloced_head, the_debug);
			col_arr[7]->func_node = NULL;
			col_arr[7]->math_node = NULL;
			col_arr[7]->rows_data_type = PTR_TYPE_CHAR;

			col_arr[7]->case_node = (struct case_node*) myMalloc(sizeof(struct case_node), NULL, &malloced_head, the_debug);
			col_arr[7]->case_node->case_when_head = NULL;
			col_arr[7]->case_node->case_then_value_head = NULL;

			addListNodePtr(&col_arr[7]->case_node->case_when_head, &col_arr[7]->case_node->case_when_tail, NULL, -1, ADDLISTNODE_TAIL
						  ,NULL, &malloced_head, the_debug);

			str1 = myMalloc(sizeof(char) * 2, NULL, &malloced_head, the_debug);
			str1[0] = 0;

			addListNodePtr(&col_arr[7]->case_node->case_then_value_head, &col_arr[7]->case_node->case_then_value_tail, str1, PTR_TYPE_CHAR, ADDLISTNODE_TAIL
						  ,NULL, &malloced_head, the_debug);

			col_arr[8] = (struct col_in_select_node*) myMalloc(sizeof(struct col_in_select_node), NULL, &malloced_head, the_debug);
			col_arr[8]->table_ptr = NULL;
			col_arr[8]->table_ptr_type = -1;
			col_arr[8]->col_ptr = NULL;
			col_arr[8]->col_ptr_type = -1;
			col_arr[8]->new_name = upper("new_case", NULL, &malloced_head, the_debug);
			col_arr[8]->func_node = NULL;
			col_arr[8]->math_node = NULL;
			col_arr[8]->rows_data_type = PTR_TYPE_CHAR;

			col_arr[8]->case_node = (struct case_node*) myMalloc(sizeof(struct case_node), NULL, &malloced_head, the_debug);
			col_arr[8]->case_node->case_when_head = NULL;
			col_arr[8]->case_node->case_then_value_head = NULL;

			temp_where = (struct where_clause_node*) myMalloc(sizeof(struct where_clause_node), NULL, &malloced_head, the_debug);
			temp_where->ptr_one = the_select_node->columns_arr[4];
			temp_where->ptr_one_type = PTR_TYPE_COL_IN_SELECT_NODE;
			temp_where->ptr_two = (char*) myMalloc(sizeof(char) * 16, NULL, &malloced_head, the_debug);
			strcpy(temp_where->ptr_two, "1/31/2025");
			temp_where->ptr_two_type = PTR_TYPE_DATE;
			temp_where->where_type = WHERE_IS_EQUALS;
			temp_where->parent = NULL;

			addListNodePtr(&col_arr[8]->case_node->case_when_head, &col_arr[8]->case_node->case_when_tail, temp_where, PTR_TYPE_WHERE_CLAUSE_NODE, ADDLISTNODE_TAIL
						  ,NULL, &malloced_head, the_debug);
			addListNodePtr(&col_arr[8]->case_node->case_when_head, &col_arr[8]->case_node->case_when_tail, NULL, -1, ADDLISTNODE_TAIL
						  ,NULL, &malloced_head, the_debug);

			str2 = myMalloc(sizeof(char) * 2, NULL, &malloced_head, the_debug);
			str2[0] = 0;

			addListNodePtr(&col_arr[8]->case_node->case_then_value_head, &col_arr[8]->case_node->case_then_value_tail, str2, PTR_TYPE_CHAR, ADDLISTNODE_TAIL
						  ,NULL, &malloced_head, the_debug);
			addListNodePtr(&col_arr[8]->case_node->case_then_value_head, &col_arr[8]->case_node->case_then_value_tail, NULL, -1, ADDLISTNODE_TAIL
						  ,NULL, &malloced_head, the_debug);


			initSelectClauseForComp(&the_select_node->next, NULL, false, 9, col_arr, NULL, -1, NULL, NULL
								   ,&malloced_head, the_debug);
			the_select_node->next->prev = the_select_node;
			

			if (test_Controller_parseSelect(5501, "select *, '' EMPTY_STRING, case when EXPIRATION = '1/31/2025' then '' else null end new_case from LIQUOR_LICENSES;", &the_select_node
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
		// END Test with id = 5501

		// START Test with id = 5502
			initSelectClauseForComp(&the_select_node, NULL, false, 0, NULL, getTablesHead(), PTR_TYPE_TABLE_INFO, NULL, NULL
								   ,&malloced_head, the_debug);

			col_arr = (struct col_in_select_node**) myMalloc(sizeof(struct col_in_select_node*) * 1, NULL, &malloced_head, the_debug);

			col_arr[0] = (struct col_in_select_node*) myMalloc(sizeof(struct col_in_select_node), NULL, &malloced_head, the_debug);
			col_arr[0]->table_ptr = NULL;
			col_arr[0]->table_ptr_type = -1;
			col_arr[0]->col_ptr = NULL;
			col_arr[0]->col_ptr_type = -1;
			col_arr[0]->new_name = upper("MATH_STUFF", NULL, &malloced_head, the_debug);
			col_arr[0]->func_node = NULL;
			col_arr[0]->math_node = NULL;
			col_arr[0]->case_node = NULL;
			col_arr[0]->rows_data_type = PTR_TYPE_INT;

			col_arr[0]->math_node = myMalloc(sizeof(struct math_node), NULL, &malloced_head, the_debug);
			col_arr[0]->math_node->ptr_one = myMalloc(sizeof(struct case_node), NULL, &malloced_head, the_debug);
			col_arr[0]->math_node->ptr_one_type = PTR_TYPE_CASE_NODE;
			col_arr[0]->math_node->ptr_two = the_select_node->columns_arr[1];
			col_arr[0]->math_node->ptr_two_type = PTR_TYPE_COL_IN_SELECT_NODE;
			col_arr[0]->math_node->operation = MATH_MULT;
			col_arr[0]->math_node->parent = NULL;
			col_arr[0]->math_node->result_type = PTR_TYPE_INT;

			((struct case_node*) col_arr[0]->math_node->ptr_one)->case_when_head = NULL;
			((struct case_node*) col_arr[0]->math_node->ptr_one)->case_when_tail = NULL;
			((struct case_node*) col_arr[0]->math_node->ptr_one)->case_then_value_head = NULL;
			((struct case_node*) col_arr[0]->math_node->ptr_one)->case_then_value_tail = NULL;
			((struct case_node*) col_arr[0]->math_node->ptr_one)->result_type = PTR_TYPE_INT;

			temp_where = (struct where_clause_node*) myMalloc(sizeof(struct where_clause_node), NULL, &malloced_head, the_debug);
			temp_where->ptr_one = the_select_node->columns_arr[1];
			temp_where->ptr_one_type = PTR_TYPE_COL_IN_SELECT_NODE;
			temp_where->ptr_two = myMalloc(sizeof(int), NULL, &malloced_head, the_debug);
			*((int*) temp_where->ptr_two) = 100000;
			temp_where->ptr_two_type = PTR_TYPE_INT;
			temp_where->where_type = WHERE_LESS_THAN;
			temp_where->parent = NULL;

			addListNodePtr(&((struct case_node*) col_arr[0]->math_node->ptr_one)->case_when_head, &((struct case_node*) col_arr[0]->math_node->ptr_one)->case_when_tail, temp_where, PTR_TYPE_WHERE_CLAUSE_NODE, ADDLISTNODE_TAIL
						  ,NULL, &malloced_head, the_debug);
			addListNodePtr(&((struct case_node*) col_arr[0]->math_node->ptr_one)->case_when_head, &((struct case_node*) col_arr[0]->math_node->ptr_one)->case_when_tail, NULL, -1, ADDLISTNODE_TAIL
						  ,NULL, &malloced_head, the_debug);

			int1 = myMalloc(sizeof(int), NULL, &malloced_head, the_debug);
			*int1 = 10000;

			int2 = myMalloc(sizeof(int), NULL, &malloced_head, the_debug);
			*int2 = 10;

			addListNodePtr(&((struct case_node*) col_arr[0]->math_node->ptr_one)->case_then_value_head, &((struct case_node*) col_arr[0]->math_node->ptr_one)->case_then_value_tail, int1, PTR_TYPE_INT, ADDLISTNODE_TAIL
						  ,NULL, &malloced_head, the_debug);
			addListNodePtr(&((struct case_node*) col_arr[0]->math_node->ptr_one)->case_then_value_head, &((struct case_node*) col_arr[0]->math_node->ptr_one)->case_then_value_tail, int2, PTR_TYPE_INT, ADDLISTNODE_TAIL
						  ,NULL, &malloced_head, the_debug);

			initSelectClauseForComp(&the_select_node->next, NULL, false, 1, col_arr, NULL, -1, NULL, NULL
								   ,&malloced_head, the_debug);
			the_select_node->next->prev = the_select_node;
			

			if (test_Controller_parseSelect(5502, "select case when CT_REGISTRATION_NUMBER < 100000 then 10000 else 10 end * CT_REGISTRATION_NUMBER MATH_STUFF from LIQUOR_LICENSES;", &the_select_node
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
		// END Test with id = 5502

		// START Test with id = 5503
			initSelectClauseForComp(&the_select_node, NULL, false, 0, NULL, getTablesHead(), PTR_TYPE_TABLE_INFO, NULL, NULL
								   ,&malloced_head, the_debug);

			initSelectClauseForComp(&the_select_node->next, NULL, false, 0, NULL, the_select_node, PTR_TYPE_SELECT_NODE, NULL, NULL
								   ,&malloced_head, the_debug);
			the_select_node->next->prev = the_select_node;

			the_select_node->next->where_head = myMalloc(sizeof(struct where_clause_node), NULL, &malloced_head, the_debug);
			the_select_node->next->where_head->ptr_one = myMalloc(sizeof(struct case_node), NULL, &malloced_head, the_debug);
			the_select_node->next->where_head->ptr_one_type = PTR_TYPE_CASE_NODE;
			the_select_node->next->where_head->ptr_two = myMalloc(sizeof(struct case_node), NULL, &malloced_head, the_debug);
			the_select_node->next->where_head->ptr_two_type = PTR_TYPE_CASE_NODE;
			the_select_node->next->where_head->where_type = WHERE_IS_EQUALS;
			the_select_node->next->where_head->parent = NULL;

			((struct case_node*) the_select_node->next->where_head->ptr_one)->case_when_head = NULL;
			((struct case_node*) the_select_node->next->where_head->ptr_one)->case_when_tail = NULL;
			((struct case_node*) the_select_node->next->where_head->ptr_one)->case_then_value_head = NULL;
			((struct case_node*) the_select_node->next->where_head->ptr_one)->case_then_value_tail = NULL;
			((struct case_node*) the_select_node->next->where_head->ptr_one)->result_type = PTR_TYPE_INT;

			((struct case_node*) the_select_node->next->where_head->ptr_two)->case_when_head = NULL;
			((struct case_node*) the_select_node->next->where_head->ptr_two)->case_when_tail = NULL;
			((struct case_node*) the_select_node->next->where_head->ptr_two)->case_then_value_head = NULL;
			((struct case_node*) the_select_node->next->where_head->ptr_two)->case_then_value_tail = NULL;
			((struct case_node*) the_select_node->next->where_head->ptr_two)->result_type = PTR_TYPE_INT;

			temp_where = (struct where_clause_node*) myMalloc(sizeof(struct where_clause_node), NULL, &malloced_head, the_debug);
			temp_where->ptr_one = the_select_node->columns_arr[1];
			temp_where->ptr_one_type = PTR_TYPE_COL_IN_SELECT_NODE;
			temp_where->ptr_two = myMalloc(sizeof(int), NULL, &malloced_head, the_debug);
			*((int*) temp_where->ptr_two) = 100000;
			temp_where->ptr_two_type = PTR_TYPE_INT;
			temp_where->where_type = WHERE_LESS_THAN;
			temp_where->parent = NULL;

			addListNodePtr(&((struct case_node*) the_select_node->next->where_head->ptr_one)->case_when_head, &((struct case_node*) the_select_node->next->where_head->ptr_one)->case_when_tail, temp_where, PTR_TYPE_WHERE_CLAUSE_NODE, ADDLISTNODE_TAIL
						  ,NULL, &malloced_head, the_debug);
			addListNodePtr(&((struct case_node*) the_select_node->next->where_head->ptr_one)->case_when_head, &((struct case_node*) the_select_node->next->where_head->ptr_one)->case_when_tail, NULL, -1, ADDLISTNODE_TAIL
						  ,NULL, &malloced_head, the_debug);

			int1 = myMalloc(sizeof(int), NULL, &malloced_head, the_debug);
			*int1 = 10000;

			int2 = myMalloc(sizeof(int), NULL, &malloced_head, the_debug);
			*int2 = 10;

			addListNodePtr(&((struct case_node*) the_select_node->next->where_head->ptr_one)->case_then_value_head, &((struct case_node*) the_select_node->next->where_head->ptr_one)->case_then_value_tail, int1, PTR_TYPE_INT, ADDLISTNODE_TAIL
						  ,NULL, &malloced_head, the_debug);
			addListNodePtr(&((struct case_node*) the_select_node->next->where_head->ptr_one)->case_then_value_head, &((struct case_node*) the_select_node->next->where_head->ptr_one)->case_then_value_tail, int2, PTR_TYPE_INT, ADDLISTNODE_TAIL
						  ,NULL, &malloced_head, the_debug);

			temp_where_2 = (struct where_clause_node*) myMalloc(sizeof(struct where_clause_node), NULL, &malloced_head, the_debug);
			temp_where_2->ptr_one = the_select_node->columns_arr[1];
			temp_where_2->ptr_one_type = PTR_TYPE_COL_IN_SELECT_NODE;
			temp_where_2->ptr_two = myMalloc(sizeof(int), NULL, &malloced_head, the_debug);
			*((int*) temp_where_2->ptr_two) = 100000;
			temp_where_2->ptr_two_type = PTR_TYPE_INT;
			temp_where_2->where_type = WHERE_LESS_THAN;
			temp_where_2->parent = NULL;

			addListNodePtr(&((struct case_node*) the_select_node->next->where_head->ptr_two)->case_when_head, &((struct case_node*) the_select_node->next->where_head->ptr_two)->case_when_tail, temp_where_2, PTR_TYPE_WHERE_CLAUSE_NODE, ADDLISTNODE_TAIL
						  ,NULL, &malloced_head, the_debug);
			addListNodePtr(&((struct case_node*) the_select_node->next->where_head->ptr_two)->case_when_head, &((struct case_node*) the_select_node->next->where_head->ptr_two)->case_when_tail, NULL, -1, ADDLISTNODE_TAIL
						  ,NULL, &malloced_head, the_debug);

			int* int3 = myMalloc(sizeof(int), NULL, &malloced_head, the_debug);
			*int3 = 10000;

			int* int4 = myMalloc(sizeof(int), NULL, &malloced_head, the_debug);
			*int4 = 10;

			addListNodePtr(&((struct case_node*) the_select_node->next->where_head->ptr_two)->case_then_value_head, &((struct case_node*) the_select_node->next->where_head->ptr_two)->case_then_value_tail, int3, PTR_TYPE_INT, ADDLISTNODE_TAIL
						  ,NULL, &malloced_head, the_debug);
			addListNodePtr(&((struct case_node*) the_select_node->next->where_head->ptr_two)->case_then_value_head, &((struct case_node*) the_select_node->next->where_head->ptr_two)->case_then_value_tail, int4, PTR_TYPE_INT, ADDLISTNODE_TAIL
						  ,NULL, &malloced_head, the_debug);


			if (test_Controller_parseSelect(5503, "select * from LIQUOR_LICENSES where case when CT_REGISTRATION_NUMBER < 100000 then 10000 else 10 end = case when CT_REGISTRATION_NUMBER < 100000 then 10000 else 10 end;", &the_select_node
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
		// END Test with id = 5503

		// START Test with id = 5504
			the_select_node = NULL;


			if (test_Controller_parseSelect(5504, "select * from LIQUOR_LICENSES where case when CT_REGISTRATION_NUMBER < 100000 then case when 1 = 1 then 100000 else 1 else 10 end = case when CT_REGISTRATION_NUMBER < 100000 then 10000 else 10 end;", &the_select_node
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
		// END Test with id = 5504

		// START Test with id = 5505
			the_select_node = NULL;


			if (test_Controller_parseSelect(5505, "select a.BRAND_NAME a_BRAND_NAME, b.BRAND_NAME B_BRAND_NAME from LIQUOR_LICENSES a join LIQUOR_LICENSES b on COUNT(a.*) = 1", &the_select_node
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
		// END Test with id = 5505
	// END Additional test_Controller_parseSelect

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
			expected_change_head->total_rows_to_insert = 0;
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

			if (test_Controller_parseUpdate(202, "UPDATE LIQUOR_LICENSES\nSET status = 'TST_ACTIVE'\n;"
										   ,&expected_change_head, &parsed_error_code, &malloced_head, the_debug) != 0)
				return -1;

			parsed_error_code = 0;
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
			expected_change_head->total_rows_to_insert = 0;
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

			str2 = (char*) myMalloc(sizeof(char) * 32, NULL, &malloced_head, the_debug);
			strcpy(str2, "1/1/1900");
			addListNodePtr(&expected_change_head->data_list_head, &expected_change_head->data_list_tail, str2, PTR_TYPE_DATE, ADDLISTNODE_TAIL
						  ,NULL, &malloced_head, the_debug);

			
			if (test_Controller_parseUpdate(203, "UPDATE  LIQUOR_LICENSES  SET  status  =  'TST_ACTIVE'\n  	,  	\nEFFECTIVE  =  '1/1/1900'	\n	;"
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
			expected_change_head->total_rows_to_insert = 0;
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
			str3 = (char*) myMalloc(sizeof(char) * 32, NULL, &malloced_head, the_debug);
			strcpy(str3, "that");
			expected_change_head->where_head->ptr_two = str3;
			expected_change_head->where_head->ptr_two_type = PTR_TYPE_CHAR;
			expected_change_head->where_head->where_type = WHERE_IS_EQUALS;
			expected_change_head->where_head->parent = NULL;

			
			if (test_Controller_parseUpdate(204, "update LIQUOR_LICENSES set CT_REGISTRATION_NUMBER = 2,EFFECTIVE = '1/1/1900' where BRAND_NAME = 'that';"
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

			
			if (test_Controller_parseUpdate(205, "update LIQUOR_LICENSES set BRAND_NAME = 2 , EFFECTIVE = '1/1/1900';"
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

			
			if (test_Controller_parseUpdate(206, "update LIQUOR_LICENSES set '1/1/1900' = EFFECTIVE, BRAND_NAME = 'this';"
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

			
			if (test_Controller_parseUpdate(207, "update LIQUOR_LICENSES set 2 = CT_REGISTRATION_NUMBER, BRAND_NAME = 'this';"
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

			
			if (test_Controller_parseUpdate(208, "update LIQUOR_LICENSES set BRAND_NAME = 'this' CT_REGISTRATION_NUMBER = 2 ;"
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

			
			if (test_Controller_parseUpdate(209, "update LIQUOR_LICENSES set BRAND_NAME = 'this', CT_REGISTRATION_NUMBER = 2 ahwdbahwuid where 1 = 1;"
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
			expected_change_head->total_rows_to_insert = 0;
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


			if (test_Controller_parseDelete(301, "delete from LIQUOR_LICENSES where BRAND_NAME = '860 INDA PALE ALE';"
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


			if (test_Controller_parseDelete(302, "   Delete	 \n	from	LIQUOR_LICENSES 	 where this = 'that'  ;"
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


			if (test_Controller_parseDelete(303, "   Delete\nFrom\n	LIQUOR_LICENSES\nwhere;"
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
			expected_change_head->total_rows_to_insert = 0;
			expected_change_head->col_list_head = NULL;
			expected_change_head->col_list_tail = NULL;
			expected_change_head->data_list_head = NULL;
			expected_change_head->data_list_tail = NULL;
			expected_change_head->where_head = NULL;


			if (test_Controller_parseDelete(304, "\nDelete\nfrom\n	LIQUOR_LICENSES"
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


			if (test_Controller_parseDelete(305, "\ndelete\n\n	LIQUOR_LICENSES"
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


			if (test_Controller_parseDelete(306, "delete from LIQUOR_LICENSES where BRAND_NAME = that"
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


			if (test_Controller_parseInsert(401, "Insert into LIQUOR_LICENSES (col1, col2) values ('Hi', 'Hello');"
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
			expected_change_head->total_rows_to_insert = 1;
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


			if (test_Controller_parseInsert(402, "Insert into LIQUOR_LICENSES (BRAND_NAME, STATUS) values ('Hi', 'Hello');"
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


			if (test_Controller_parseInsert(403, "Insert into LIQUOR_LICENSES (BRAND_NAME, BRAND_NAME, STATUS) values ('Hi', 'Hello', 'There');"
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


			if (test_Controller_parseInsert(404, "Insert into LIQUOR_LICENSES (BRAND_NAME BRAND_NAME, STATUS) values ('Hi', 'Hello');"
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


			if (test_Controller_parseInsert(405, "Insert into LIQUOR_LICENSES (BRAND_NAME, STATUS) values ('Hi', 'Hello', 'There');"
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
			expected_change_head->total_rows_to_insert = 1;
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
			

			if (test_Controller_parseInsert(406, "Insert into LIQUOR_LICENSES (BRAND_NAME,CT_REGISTRATION_NUMBER,STATUS,EFFECTIVE,EXPIRATION,OUT_OF_STATE_SHIPPER,SUPERVISOR_CREDENTIAL) values ('1', 1, '1','1/1/1900','1/1/1900','1','1');"
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

			
			if (test_Controller_parseInsert(407, "Insert into LIQUOR_LICENSES (BRAND_NAME, CT_REGISTRATION_NUMBER) values ('Hi', 'Hello');"
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

			
			if (test_Controller_parseInsert(408, "Insert into LIQUOR_LICENSES (BRAND_NAME, CT_REGISTRATION_NUMBER) values (1, 1);"
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

			
			if (test_Controller_parseInsert(409, "Insert into LIQUOR_LICENSES (BRAND_NAME, EFFECTIVE) values ('Hi', 1);"
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
			expected_change_head->total_rows_to_insert = 2;
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
			addListNodePtr(&expected_change_head->col_list_head, &expected_change_head->col_list_tail, NULL, -1, ADDLISTNODE_TAIL
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
			addListNodePtr(&expected_change_head->data_list_head, &expected_change_head->data_list_tail, NULL, -1, ADDLISTNODE_TAIL
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
			

			if (test_Controller_parseInsert(410, "Insert into LIQUOR_LICENSES (BRAND_NAME,CT_REGISTRATION_NUMBER,STATUS,EFFECTIVE,EXPIRATION,OUT_OF_STATE_SHIPPER,SUPERVISOR_CREDENTIAL) values ('1', 1, '1','1/1/1900','1/1/1900','1','1'), ('1', 1, '1','1/1/1900','1/1/1900','1','1');"
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
		
	// START test_Driver_findValidRowsGivenWhere
		// START Test with id = 601
			struct ListNodePtr* valid_rows_head = NULL;
			struct ListNodePtr* valid_rows_tail = NULL;

			for (int i=0; i<getTablesHead()->table_cols_head->num_rows; i++)
			{
				addListNodePtr_Int(&valid_rows_head, &valid_rows_tail, i, ADDLISTNODE_TAIL, NULL, &malloced_head, the_debug);
			}


			if (test_Driver_findValidRowsGivenWhere(601, "", NULL, getTablesHead()
												   ,valid_rows_head, &malloced_head, the_debug) != 0)
				return -1;


			freeAnyLinkedList((void**) &valid_rows_head, PTR_TYPE_LIST_NODE_PTR, NULL, &malloced_head, the_debug);

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
		// END Test with id = 601
		
		// START Test with id = 602
			valid_rows_head = NULL;
			valid_rows_tail = NULL;

			addListNodePtr_Int(&valid_rows_head, &valid_rows_tail, 12, ADDLISTNODE_TAIL, NULL, &malloced_head, the_debug);


			if (test_Driver_findValidRowsGivenWhere(602, "where BRAND_NAME = 'VIZZY BLACK CHERRY LIME';", NULL, getTablesHead()
												   ,valid_rows_head, &malloced_head, the_debug) != 0)
				return -1;


			freeAnyLinkedList((void**) &valid_rows_head, PTR_TYPE_LIST_NODE_PTR, NULL, &malloced_head, the_debug);

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
		// END Test with id = 602
		
		// START Test with id = 603
			valid_rows_head = NULL;
			valid_rows_tail = NULL;

			addListNodePtr_Int(&valid_rows_head, &valid_rows_tail, 6, ADDLISTNODE_TAIL, NULL, &malloced_head, the_debug);


			if (test_Driver_findValidRowsGivenWhere(603, "where CT_REGISTRATION_NUMBER = 152525;", NULL, getTablesHead()
												   ,valid_rows_head, &malloced_head, the_debug) != 0)
				return -1;


			freeAnyLinkedList((void**) &valid_rows_head, PTR_TYPE_LIST_NODE_PTR, NULL, &malloced_head, the_debug);

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
		// END Test with id = 603
		
		// START Test with id = 604
			valid_rows_head = NULL;
			valid_rows_tail = NULL;

			addListNodePtr_Int(&valid_rows_head, &valid_rows_tail, 5, ADDLISTNODE_TAIL, NULL, &malloced_head, the_debug);
			addListNodePtr_Int(&valid_rows_head, &valid_rows_tail, 56, ADDLISTNODE_TAIL, NULL, &malloced_head, the_debug);
			addListNodePtr_Int(&valid_rows_head, &valid_rows_tail, 58, ADDLISTNODE_TAIL, NULL, &malloced_head, the_debug);
			addListNodePtr_Int(&valid_rows_head, &valid_rows_tail, 343, ADDLISTNODE_TAIL, NULL, &malloced_head, the_debug);
			addListNodePtr_Int(&valid_rows_head, &valid_rows_tail, 521, ADDLISTNODE_TAIL, NULL, &malloced_head, the_debug);
			addListNodePtr_Int(&valid_rows_head, &valid_rows_tail, 579, ADDLISTNODE_TAIL, NULL, &malloced_head, the_debug);
			addListNodePtr_Int(&valid_rows_head, &valid_rows_tail, 805, ADDLISTNODE_TAIL, NULL, &malloced_head, the_debug);
			addListNodePtr_Int(&valid_rows_head, &valid_rows_tail, 945, ADDLISTNODE_TAIL, NULL, &malloced_head, the_debug);

			
			if (test_Driver_findValidRowsGivenWhere(604, "where EXPIRATION = '1/31/2025';", NULL, getTablesHead()
												   ,valid_rows_head, &malloced_head, the_debug) != 0)
				return -1;


			freeAnyLinkedList((void**) &valid_rows_head, PTR_TYPE_LIST_NODE_PTR, NULL, &malloced_head, the_debug);

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
		// END Test with id = 604
		
		// START Test with id = 605
			valid_rows_head = NULL;
			valid_rows_tail = NULL;

			addListNodePtr_Int(&valid_rows_head, &valid_rows_tail, 12, ADDLISTNODE_TAIL, NULL, &malloced_head, the_debug);

			
			if (test_Driver_findValidRowsGivenWhere(605, "where BRAND_NAME = 'VIZZY BLACK CHERRY LIME' and OUT_OF_STATE_SHIPPER = 'MOLSON COORS BEVERAGE COMPANY USA LLC';"
												   ,NULL, getTablesHead()
												   ,valid_rows_head, &malloced_head, the_debug) != 0)
				return -1;


			freeAnyLinkedList((void**) &valid_rows_head, PTR_TYPE_LIST_NODE_PTR, NULL, &malloced_head, the_debug);

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
		// END Test with id = 605
		
		// START Test with id = 606
			valid_rows_head = NULL;
			valid_rows_tail = NULL;

			addListNodePtr_Int(&valid_rows_head, &valid_rows_tail, 12, ADDLISTNODE_TAIL, NULL, &malloced_head, the_debug);
			addListNodePtr_Int(&valid_rows_head, &valid_rows_tail, 23, ADDLISTNODE_TAIL, NULL, &malloced_head, the_debug);

			
			if (test_Driver_findValidRowsGivenWhere(606, "where BRAND_NAME = 'VIZZY BLACK CHERRY LIME' or BRAND_NAME = 'CRUZAN ISLAND SPICED RUM';"
												   ,NULL, getTablesHead()
												   ,valid_rows_head, &malloced_head, the_debug) != 0)
				return -1;


			freeAnyLinkedList((void**) &valid_rows_head, PTR_TYPE_LIST_NODE_PTR, NULL, &malloced_head, the_debug);

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
		// END Test with id = 606
		
		// START Test with id = 607
			valid_rows_head = NULL;
			valid_rows_tail = NULL;

			addListNodePtr_Int(&valid_rows_head, &valid_rows_tail, 5, ADDLISTNODE_TAIL, NULL, &malloced_head, the_debug);
			addListNodePtr_Int(&valid_rows_head, &valid_rows_tail, 12, ADDLISTNODE_TAIL, NULL, &malloced_head, the_debug);
			addListNodePtr_Int(&valid_rows_head, &valid_rows_tail, 23, ADDLISTNODE_TAIL, NULL, &malloced_head, the_debug);
			addListNodePtr_Int(&valid_rows_head, &valid_rows_tail, 56, ADDLISTNODE_TAIL, NULL, &malloced_head, the_debug);
			addListNodePtr_Int(&valid_rows_head, &valid_rows_tail, 58, ADDLISTNODE_TAIL, NULL, &malloced_head, the_debug);
			addListNodePtr_Int(&valid_rows_head, &valid_rows_tail, 343, ADDLISTNODE_TAIL, NULL, &malloced_head, the_debug);
			addListNodePtr_Int(&valid_rows_head, &valid_rows_tail, 521, ADDLISTNODE_TAIL, NULL, &malloced_head, the_debug);
			addListNodePtr_Int(&valid_rows_head, &valid_rows_tail, 579, ADDLISTNODE_TAIL, NULL, &malloced_head, the_debug);
			addListNodePtr_Int(&valid_rows_head, &valid_rows_tail, 805, ADDLISTNODE_TAIL, NULL, &malloced_head, the_debug);
			addListNodePtr_Int(&valid_rows_head, &valid_rows_tail, 945, ADDLISTNODE_TAIL, NULL, &malloced_head, the_debug);

			
			if (test_Driver_findValidRowsGivenWhere(607, "where BRAND_NAME = 'VIZZY BLACK CHERRY LIME' or BRAND_NAME = 'CRUZAN ISLAND SPICED RUM' and CT_REGISTRATION_NUMBER = 169876 or EXPIRATION = '1/31/2025';"
												   ,NULL, getTablesHead(), valid_rows_head, &malloced_head, the_debug) != 0)
				return -1;


			freeAnyLinkedList((void**) &valid_rows_head, PTR_TYPE_LIST_NODE_PTR, NULL, &malloced_head, the_debug);

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
		// END Test with id = 607
		
		// START Test with id = 608
			valid_rows_head = NULL;
			valid_rows_tail = NULL;

			addListNodePtr_Int(&valid_rows_head, &valid_rows_tail, 5, ADDLISTNODE_TAIL, NULL, &malloced_head, the_debug);
			addListNodePtr_Int(&valid_rows_head, &valid_rows_tail, 23, ADDLISTNODE_TAIL, NULL, &malloced_head, the_debug);

			
			if (test_Driver_findValidRowsGivenWhere(608, "where BRAND_NAME = 'CRUZAN ISLAND SPICED RUM' and CT_REGISTRATION_NUMBER = 169876 or EXPIRATION = '1/31/2025' and BRAND_NAME = 'BABAROSA MOSCATO D''ASTI';"
												   ,NULL, getTablesHead(), valid_rows_head, &malloced_head, the_debug) != 0)
				return -1;
			else
				freeAnyLinkedList((void**) &valid_rows_head, PTR_TYPE_LIST_NODE_PTR, NULL, &malloced_head, the_debug);
			

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
		// END Test with id = 608
		
		// START Test with id = 609
			valid_rows_head = NULL;
			valid_rows_tail = NULL;

			for (int i=0; i<getTablesHead()->table_cols_head->num_rows; i++)
			{
				if (i != 23)
					addListNodePtr_Int(&valid_rows_head, &valid_rows_tail, i, ADDLISTNODE_TAIL, NULL, &malloced_head, the_debug);
			}

			
			if (test_Driver_findValidRowsGivenWhere(609, "where BRAND_NAME <> 'CRUZAN ISLAND SPICED RUM';"
												   ,NULL, getTablesHead(), valid_rows_head, &malloced_head, the_debug) != 0)
				return -1;
			else
				freeAnyLinkedList((void**) &valid_rows_head, PTR_TYPE_LIST_NODE_PTR, NULL, &malloced_head, the_debug);
			

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
		// END Test with id = 609
		
		// START Test with id = 610
			valid_rows_head = NULL;
			valid_rows_tail = NULL;

			for (int i=0; i<getTablesHead()->table_cols_head->num_rows; i++)
			{
				addListNodePtr_Int(&valid_rows_head, &valid_rows_tail, i, ADDLISTNODE_TAIL, NULL, &malloced_head, the_debug);
			}

			
			if (test_Driver_findValidRowsGivenWhere(610, "where BRAND_NAME <> 'CRUZAN ISLAND SPICED RUM' or BRAND_NAME <> 'TEST 1';"
												   ,NULL, getTablesHead(), valid_rows_head, &malloced_head, the_debug) != 0)
				return -1;
			else
				freeAnyLinkedList((void**) &valid_rows_head, PTR_TYPE_LIST_NODE_PTR, NULL, &malloced_head, the_debug);
			

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
		// END Test with id = 610
		
		// START Test with id = 611
			valid_rows_head = NULL;
			valid_rows_tail = NULL;

			
			for (int i=0; i<getTablesHead()->table_cols_head->num_rows; i++)
			{
				if (i != 23 && i != 12)
					addListNodePtr_Int(&valid_rows_head, &valid_rows_tail, i, ADDLISTNODE_TAIL, NULL, &malloced_head, the_debug);
			}

			
			if (test_Driver_findValidRowsGivenWhere(611, "where BRAND_NAME <> 'CRUZAN ISLAND SPICED RUM' and BRAND_NAME <> 'VIZZY BLACK CHERRY LIME';"
												   ,NULL, getTablesHead(), valid_rows_head, &malloced_head, the_debug) != 0)
				return -1;
			else
				freeAnyLinkedList((void**) &valid_rows_head, PTR_TYPE_LIST_NODE_PTR, NULL, &malloced_head, the_debug);
			

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
		// END Test with id = 611
		
		// START Test with id = 612
			valid_rows_head = NULL;
			valid_rows_tail = NULL;

			for (int i=0; i<getTablesHead()->table_cols_head->num_rows; i++)
			{
				addListNodePtr_Int(&valid_rows_head, &valid_rows_tail, i, ADDLISTNODE_TAIL, NULL, &malloced_head, the_debug);
			}

			
			if (test_Driver_findValidRowsGivenWhere(612, "where BRAND_NAME <> 'CRUZAN ISLAND SPICED RUM' and CT_REGISTRATION_NUMBER <> 169876 or EXPIRATION <> '1/31/2025' and BRAND_NAME <> 'BABAROSA MOSCATO D''ASTI';"
												   ,NULL, getTablesHead(), valid_rows_head, &malloced_head, the_debug) != 0)
				return -1;
			else
				freeAnyLinkedList((void**) &valid_rows_head, PTR_TYPE_LIST_NODE_PTR, NULL, &malloced_head, the_debug);
			

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
		// END Test with id = 612
		
		// START Test with id = 613
			valid_rows_head = NULL;
			valid_rows_tail = NULL;

			
			if (test_Driver_findValidRowsGivenWhere(613, "where BRAND_NAME = 'Is test bro';"
												   ,NULL, getTablesHead(), valid_rows_head, &malloced_head, the_debug) != 0)
				return -1;
			else
				freeAnyLinkedList((void**) &valid_rows_head, PTR_TYPE_LIST_NODE_PTR, NULL, &malloced_head, the_debug);
			

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
		// END Test with id = 613

		// START Test with id = 614
			valid_rows_head = NULL;
			valid_rows_tail = NULL;

			for (int i=0; i<getTablesHead()->table_cols_head->num_rows; i++)
			{
				addListNodePtr_Int(&valid_rows_head, &valid_rows_tail, i, ADDLISTNODE_TAIL, NULL, &malloced_head, the_debug);
			}

			
			if (test_Driver_findValidRowsGivenWhere(614, "where CT_REGISTRATION_NUMBER > 1;"
												   ,NULL, getTablesHead(), valid_rows_head, &malloced_head, the_debug) != 0)
				return -1;
			else
				freeAnyLinkedList((void**) &valid_rows_head, PTR_TYPE_LIST_NODE_PTR, NULL, &malloced_head, the_debug);
			

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
		// END Test with id = 614

		// START Test with id = 615
			valid_rows_head = NULL;
			valid_rows_tail = NULL;

			for (int i=0; i<getTablesHead()->table_cols_head->num_rows; i++)
			{
				addListNodePtr_Int(&valid_rows_head, &valid_rows_tail, i, ADDLISTNODE_TAIL, NULL, &malloced_head, the_debug);
			}

			
			if (test_Driver_findValidRowsGivenWhere(615, "where CT_REGISTRATION_NUMBER >= 1;"
												   ,NULL, getTablesHead(), valid_rows_head, &malloced_head, the_debug) != 0)
				return -1;
			else
				freeAnyLinkedList((void**) &valid_rows_head, PTR_TYPE_LIST_NODE_PTR, NULL, &malloced_head, the_debug);
			

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
		// END Test with id = 615

		// START Test with id = 616
			valid_rows_head = NULL;
			valid_rows_tail = NULL;

			
			if (test_Driver_findValidRowsGivenWhere(616, "where CT_REGISTRATION_NUMBER < 1;"
												   ,NULL, getTablesHead(), valid_rows_head, &malloced_head, the_debug) != 0)
				return -1;
			else
				freeAnyLinkedList((void**) &valid_rows_head, PTR_TYPE_LIST_NODE_PTR, NULL, &malloced_head, the_debug);
			

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
		// END Test with id = 616

		// START Test with id = 617
			valid_rows_head = NULL;
			valid_rows_tail = NULL;

			
			if (test_Driver_findValidRowsGivenWhere(617, "where CT_REGISTRATION_NUMBER <= 1;"
												   ,NULL, getTablesHead(), valid_rows_head, &malloced_head, the_debug) != 0)
				return -1;
			else
				freeAnyLinkedList((void**) &valid_rows_head, PTR_TYPE_LIST_NODE_PTR, NULL, &malloced_head, the_debug);
			

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
		// END Test with id = 617

		// START Test with id = 618
			valid_rows_head = NULL;
			valid_rows_tail = NULL;

			for (int i=0; i<getTablesHead()->table_cols_head->num_rows; i++)
			{
				addListNodePtr_Int(&valid_rows_head, &valid_rows_tail, i, ADDLISTNODE_TAIL, NULL, &malloced_head, the_debug);
			}

			
			if (test_Driver_findValidRowsGivenWhere(618, "where EFFECTIVE > '1/1/1900';"
												   ,NULL, getTablesHead(), valid_rows_head, &malloced_head, the_debug) != 0)
				return -1;
			else
				freeAnyLinkedList((void**) &valid_rows_head, PTR_TYPE_LIST_NODE_PTR, NULL, &malloced_head, the_debug);
			

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
		// END Test with id = 618

		// START Test with id = 619
			valid_rows_head = NULL;
			valid_rows_tail = NULL;

			for (int i=0; i<getTablesHead()->table_cols_head->num_rows; i++)
			{
				addListNodePtr_Int(&valid_rows_head, &valid_rows_tail, i, ADDLISTNODE_TAIL, NULL, &malloced_head, the_debug);
			}

			
			if (test_Driver_findValidRowsGivenWhere(619, "where EFFECTIVE >= '1/1/1900';"
												   ,NULL, getTablesHead(), valid_rows_head, &malloced_head, the_debug) != 0)
				return -1;
			else
				freeAnyLinkedList((void**) &valid_rows_head, PTR_TYPE_LIST_NODE_PTR, NULL, &malloced_head, the_debug);
			

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
		// END Test with id = 619

		// START Test with id = 620
			valid_rows_head = NULL;
			valid_rows_tail = NULL;

			
			if (test_Driver_findValidRowsGivenWhere(620, "where EFFECTIVE < '1/1/1900';"
												   ,NULL, getTablesHead(), valid_rows_head, &malloced_head, the_debug) != 0)
				return -1;
			else
				freeAnyLinkedList((void**) &valid_rows_head, PTR_TYPE_LIST_NODE_PTR, NULL, &malloced_head, the_debug);
			

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
		// END Test with id = 620

		// START Test with id = 621
			valid_rows_head = NULL;
			valid_rows_tail = NULL;

			
			if (test_Driver_findValidRowsGivenWhere(621, "where EFFECTIVE <= '1/1/1900';"
												   ,NULL, getTablesHead(), valid_rows_head, &malloced_head, the_debug) != 0)
				return -1;
			else
				freeAnyLinkedList((void**) &valid_rows_head, PTR_TYPE_LIST_NODE_PTR, NULL, &malloced_head, the_debug);
			

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
		// END Test with id = 621
	// END test_Driver_findValidRowsGivenWhere

	// START test_Driver_selectStuff
		// START Test with id = 1001
			if (test_Driver_selectStuff(1001, "select * from LIQUOR_LICENSES;"
									  ,"DB_Files_2_Test_Versions/Select_Test_1.csv"
									  ,&malloced_head, the_debug) != 0)
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
		// END Test with id = 1001
			
		// START Test with id = 1002
			if (test_Driver_selectStuff(1002, "select BRAND_NAME from LIQUOR_LICENSES;"
									  ,"DB_Files_2_Test_Versions/Select_Test_2.csv"
									  ,&malloced_head, the_debug) != 0)
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
		// END Test with id = 1002

		// START Test with id = 1003
			if (test_Driver_selectStuff(1003, "select * from LIQUOR_LICENSES alc where EXPIRATION = '1/31/2025';"
									  ,"DB_Files_2_Test_Versions/Select_Test_3.csv"
									  ,&malloced_head, the_debug) != 0)
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
		// END Test with id = 1003

		// START Test with id = 1004
			if (test_Driver_selectStuff(1004, "select BRAND_NAME from LIQUOR_LICENSES where EXPIRATION = '1/31/2025';"
									  ,"DB_Files_2_Test_Versions/Select_Test_4.csv"
									  ,&malloced_head, the_debug) != 0)
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
		// END Test with id = 1004
			
		// START Test with id = 1005
			if (test_Driver_selectStuff(1005, "select BRAND_NAME, count(*) CNT from LIQUOR_LICENSES group by BRAND_NAME;"
									  ,"DB_Files_2_Test_Versions/Select_Test_5.csv"
									  ,&malloced_head, the_debug) != 0)
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
		// END Test with id = 1005

		// START Test with id = 1006
			if (test_Driver_selectStuff(1006, "select EXPIRATION, count(*) as CNT from LIQUOR_LICENSES group by EXPIRATION;"
									  ,"DB_Files_2_Test_Versions/Select_Test_6.csv"
									  ,&malloced_head, the_debug) != 0)
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
		// END Test with id = 1006

		// START Test with id = 1007
			if (test_Driver_selectStuff(1007, "select OUT_OF_STATE_SHIPPER, EXPIRATION, count(*) CNT from LIQUOR_LICENSES group by OUT_OF_STATE_SHIPPER, EXPIRATION;"
									  ,"DB_Files_2_Test_Versions/Select_Test_7.csv"
									  ,&malloced_head, the_debug) != 0)
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
		// END Test with id = 1007

		// START Test with id = 1008
			if (test_Driver_selectStuff(1008, "select EXPIRATION, avg(CT_REGISTRATION_NUMBER) AVG_BRO from LIQUOR_LICENSES group by EXPIRATION;"
									  ,"DB_Files_2_Test_Versions/Select_Test_8.csv"
									  ,&malloced_head, the_debug) != 0)
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
		// END Test with id = 1008
		
		// START Test with id = 1009
			if (test_Driver_selectStuff(1009, "select avg(CT_REGISTRATION_NUMBER) \"avg func\", count ( * ) COL1, first(EFFECTIVE) COL2, last(EXPIRATION) last_func, min(CT_REGISTRATION_NUMBER) COL3, max(CT_REGISTRATION_NUMBER) COL4, median(CT_REGISTRATION_NUMBER) COL5, sum(CT_REGISTRATION_NUMBER) COL6 from LIQUOR_LICENSES;"
									  ,"DB_Files_2_Test_Versions/Select_Test_9.csv"
									  ,&malloced_head, the_debug) != 0)
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
		// END Test with id = 1009

		// START Test with id = 1010
			if (test_Driver_selectStuff(1010, "select CT_REGISTRATION_NUMBER / 100 DIV, count(*) CNT from LIQUOR_LICENSES group by CT_REGISTRATION_NUMBER;"
									  ,"DB_Files_2_Test_Versions/Select_Test_10.csv"
									  ,&malloced_head, the_debug) != 0)
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
		// END Test with id = 1010

		// START Test with id = 1011
			if (test_Driver_selectStuff(1011, "select 41 + ((110.5 / 5) * 10) \"41\", 100000 - CT_REGISTRATION_NUMBER * 10 MULT , (2 + 8)^2 AS POW, EFFECTIVE - 100 EFF from LIQUOR_LICENSES"
									  ,"DB_Files_2_Test_Versions/Select_Test_11.csv"
									  ,&malloced_head, the_debug) != 0)
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
		// END Test with id = 1011

		// START Test with id = 1012
			if (test_Driver_selectStuff(1012, "select * except EXPIRATION,CT_REGISTRATION_NUMBER from LIQUOR_LICENSES;"
									  ,"DB_Files_2_Test_Versions/Select_Test_12.csv"
									  ,&malloced_head, the_debug) != 0)
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
		// END Test with id = 1012

		// START Test with id = 1013
			if (test_Driver_selectStuff(1013, "select * except BRAND_NAME from LIQUOR_LICENSES where BRAND_NAME = 'WOODINVILLE STRAIGHT BOURBON TRAYPACK';"
									  ,"DB_Files_2_Test_Versions/Select_Test_13.csv"
									  ,&malloced_head, the_debug) != 0)
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
		// END Test with id = 1013

		// START Test with id = 1014
			if (test_Driver_selectStuff(1014, "select case when EXPIRATION = '1/31/2025' then 'YES' else 'no' end new_case from LIQUOR_LICENSES;"
									  ,"DB_Files_2_Test_Versions/Select_Test_14.csv"
									  ,&malloced_head, the_debug) != 0)
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
		// END Test with id = 1014

		// START Test with id = 1015
			if (test_Driver_selectStuff(1015, "select case when EXPIRATION = '1/31/2025' then 1.0 when EXPIRATION = '3/16/2025' then 20.0 else 100.0 end new_case from LIQUOR_LICENSES;"
									  ,"DB_Files_2_Test_Versions/Select_Test_15.csv"
									  ,&malloced_head, the_debug) != 0)
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
		// END Test with id = 1015

		// START Test with id = 1016
			if (test_Driver_selectStuff(1016, "select 'Hi' Hi, 'Hello' hello, 'Bye' bye, BRAND_NAME from LIQUOR_LICENSES;"
									  ,"DB_Files_2_Test_Versions/Select_Test_16.csv"
									  ,&malloced_head, the_debug) != 0)
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
		// END Test with id = 1016

		// START Test with id = 1017
			if (test_Driver_selectStuff(1017, "select * from (select * from LIQUOR_LICENSES) tbl;"
									  ,"DB_Files_2_Test_Versions/Select_Test_1.csv"
									  ,&malloced_head, the_debug) != 0)
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
		// END Test with id = 1017

		// START Test with id = 1018
			if (test_Driver_selectStuff(1018, "select BRAND_NAME from (select BRAND_NAME, CT_REGISTRATION_NUMBER from LIQUOR_LICENSES) tbl;"
									  ,"DB_Files_2_Test_Versions/Select_Test_2.csv"
									  ,&malloced_head, the_debug) != 0)
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
		// END Test with id = 1018

		// START Test with id = 1019
			if (test_Driver_selectStuff(1019, "select BRAND_NAME from (select BRAND_NAME, CT_REGISTRATION_NUMBER, EXPIRATION from LIQUOR_LICENSES) tbl where EXPIRATION = '1/31/2025';"
									  ,"DB_Files_2_Test_Versions/Select_Test_4.csv"
									  ,&malloced_head, the_debug) != 0)
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
		// END Test with id = 1019

		// START Test with id = 1020
			if (test_Driver_selectStuff(1020, "select new_case from ( select case when EXPIRATION = '1/31/2025' then 1.0 when EXPIRATION = '3/16/2025' then 20.0 else 100.0 end new_case from LIQUOR_LICENSES ) tbl;"
									  ,"DB_Files_2_Test_Versions/Select_Test_15.csv"
									  ,&malloced_head, the_debug) != 0)
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
		// END Test with id = 1020

		// START Test with id = 1021
			if (test_Driver_selectStuff(1021, "with tbl as (select BRAND_NAME, CT_REGISTRATION_NUMBER, EXPIRATION from LIQUOR_LICENSES) select BRAND_NAME from tbl where EXPIRATION = '1/31/2025';"
									  ,"DB_Files_2_Test_Versions/Select_Test_4.csv"
									  ,&malloced_head, the_debug) != 0)
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
		// END Test with id = 1021

		// START Test with id = 1022
			if (test_Driver_selectStuff(1022, "select BRAND_NAME, EXPIRATION from LIQUOR_LICENSES order by EXPIRATION asc;"
									  ,"DB_Files_2_Test_Versions/Select_Test_24.csv"
									  ,&malloced_head, the_debug) != 0)
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
		// END Test with id = 1022

		// START Test with id = 1023
			if (test_Driver_selectStuff(1023, "select BRAND_NAME, EXPIRATION from LIQUOR_LICENSES order by BRAND_NAME asc;"
									  ,"DB_Files_2_Test_Versions/Select_Test_25.csv"
									  ,&malloced_head, the_debug) != 0)
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
		// END Test with id = 1023

		// START Test with id = 1024
			if (test_Driver_selectStuff(1024, "select BRAND_NAME, EXPIRATION from LIQUOR_LICENSES order by EXPIRATION desc;"
									  ,"DB_Files_2_Test_Versions/Select_Test_26.csv"
									  ,&malloced_head, the_debug) != 0)
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
		// END Test with id = 1024

		// START Test with id = 1025
			if (test_Driver_selectStuff(1025, "select BRAND_NAME, EXPIRATION from LIQUOR_LICENSES order by EXPIRATION asc, BRAND_NAME asc;"
									  ,"DB_Files_2_Test_Versions/Select_Test_27.csv"
									  ,&malloced_head, the_debug) != 0)
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
		// END Test with id = 1025

		// START Test with id = 1026
			if (test_Driver_selectStuff(1026, "select OUT_OF_STATE_SHIPPER, EXPIRATION, BRAND_NAME from LIQUOR_LICENSES order by OUT_OF_STATE_SHIPPER asc, EXPIRATION asc, BRAND_NAME asc;"
									  ,"DB_Files_2_Test_Versions/Select_Test_28.csv"
									  ,&malloced_head, the_debug) != 0)
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
		// END Test with id = 1026

		// START Test with id = 1027
			if (test_Driver_selectStuff(1027, "select OUT_OF_STATE_SHIPPER, EXPIRATION, BRAND_NAME, CT_REGISTRATION_NUMBER from LIQUOR_LICENSES order by OUT_OF_STATE_SHIPPER asc, EXPIRATION asc, BRAND_NAME asc;"
									  ,"DB_Files_2_Test_Versions/Select_Test_29.csv"
									  ,&malloced_head, the_debug) != 0)
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
		// END Test with id = 1027

		// START Test with id = 1028
			if (test_Driver_selectStuff(1028, "select * from LIQUOR_LICENSES where OUT_OF_STATE_SHIPPER is null;"
									  ,"DB_Files_2_Test_Versions/Select_Test_30.csv"
									  ,&malloced_head, the_debug) != 0)
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
		// END Test with id = 1028

		// START Test with id = 1029
			if (test_Driver_selectStuff(1029, "select * from LIQUOR_LICENSES where OUT_OF_STATE_SHIPPER = 'null';"
									  ,"DB_Files_2_Test_Versions/Select_Test_31.csv"
									  ,&malloced_head, the_debug) != 0)
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
		// END Test with id = 1029

		// START Test with id = 1030
			if (test_Driver_selectStuff(1030, "select EXPIRATION, max(count(*)) CNT from LIQUOR_LICENSES group by EXPIRATION;"
									  ,"DB_Files_2_Test_Versions/Select_Test_6.csv"
									  ,&malloced_head, the_debug) != 0)
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
		// END Test with id = 1030

		// START Test with id = 1031
			if (test_Driver_selectStuff(1031, "select case when EXPIRATION = '1/31/2025' then 1.0 when EXPIRATION = '3/16/2025' then 2 * 10.0 else 10.0 * 10.0 end new_case from LIQUOR_LICENSES;"
									  ,"DB_Files_2_Test_Versions/Select_Test_15.csv"
									  ,&malloced_head, the_debug) != 0)
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
		// END Test with id = 1031

		// START Test with id = 1032
			if (test_Driver_selectStuff(1032, "select '1' one from LIQUOR_LICENSES;"
									  ,"DB_Files_2_Test_Versions/Select_Test_35.csv"
									  ,&malloced_head, the_debug) != 0)
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
		// END Test with id = 1032

		// START Test with id = 1033
			if (test_Driver_selectStuff(1033, "select max(CNT) MAX_CNT from ( select EXPIRATION, count(*) CNT from LIQUOR_LICENSES group by EXPIRATION ) tbl;"
									  ,"DB_Files_2_Test_Versions/Select_Test_36.csv"
									  ,&malloced_head, the_debug) != 0)
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
		// END Test with id = 1033

		// START Test with id = 1034
			if (test_Driver_selectStuff(1034, "select count (distinct EXPIRATION) CNT_DIS from LIQUOR_LICENSES;"
									  ,"DB_Files_2_Test_Versions/Select_Test_37.csv"
									  ,&malloced_head, the_debug) != 0)
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
		// END Test with id = 1034

		// START Test with id = 1035
			if (test_Driver_selectStuff(1035, "select count (distinct *) CNT_DIS from LIQUOR_LICENSES;"
									  ,"DB_Files_2_Test_Versions/Select_Test_38.csv"
									  ,&malloced_head, the_debug) != 0)
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
		// END Test with id = 1035

		// START Test with id = 1036
			if (test_Driver_selectStuff(1036, "select avg(distinct CT_REGISTRATION_NUMBER) \"avg func\", count (distinct STATUS ) COL1, first(distinct EFFECTIVE) COL2, last(distinct EXPIRATION) last_func, min(distinct CT_REGISTRATION_NUMBER) COL3, max( distinct CT_REGISTRATION_NUMBER) COL4, median(distinct CT_REGISTRATION_NUMBER) COL5, sum(distinct CT_REGISTRATION_NUMBER) COL6 from LIQUOR_LICENSES;"
									  ,"DB_Files_2_Test_Versions/Select_Test_39.csv"
									  ,&malloced_head, the_debug) != 0)
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
		// END Test with id = 1036

		// START Test with id = 1040
			if (test_Driver_selectStuff(1040, "select '1' one from LIQUOR_LICENSES where one <> '1';"
									  ,"DB_Files_2_Test_Versions/Select_Test_40.csv"
									  ,&malloced_head, the_debug) != 0)
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
		// END Test with id = 1040

		// START Test with id = 1041
			if (test_Driver_selectStuff(1041, "select EXPIRATION, CNT from ( select EXPIRATION, count(*) CNT from LIQUOR_LICENSES group by EXPIRATION ) tbl where CNT > 1;"
									  ,"DB_Files_2_Test_Versions/Select_Test_41.csv"
									  ,&malloced_head, the_debug) != 0)
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
		// END Test with id = 1041

		// START Test with id = 1042
			if (test_Driver_selectStuff(1042, "select EXPIRATION, avg(CT_REGISTRATION_NUMBER) AVG from LIQUOR_LICENSES where BRAND_NAME = 'EDINBURGH GIN SEASIDE' group by EXPIRATION;"
									  ,"DB_Files_2_Test_Versions/Select_Test_42.csv"
									  ,&malloced_head, the_debug) != 0)
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
		// END Test with id = 1042
			
		// START Test with id = 1043
			if (test_Driver_selectStuff(1043, "select BRAND_NAME from (select BRAND_NAME, CT_REGISTRATION_NUMBER, EXPIRATION from LIQUOR_LICENSES where BRAND_NAME = 'KINKY FRUIT PUNCH') tbl where EXPIRATION = '1/31/2025';"
									  ,"DB_Files_2_Test_Versions/Select_Test_20.csv"
									  ,&malloced_head, the_debug) != 0)
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
		// END Test with id = 1043
			
		// START Test with id = 1044
			if (test_Driver_selectStuff(1044, "select a.BRAND_NAME a_BRAND_NAME, b.BRAND_NAME b_BRAND_NAME from LIQUOR_LICENSES a join (select * from LIQUOR_LICENSES order by BRAND_NAME desc) b on a.BRAND_NAME = b.BRAND_NAME;"
									  ,"DB_Files_2_Test_Versions/Select_Test_34.csv"
									  ,&malloced_head, the_debug) != 0)
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
		// END Test with id = 1044
			
		// START Test with id = 1045
			if (test_Driver_selectStuff(1045, "select a.BRAND_NAME a_BRAND_NAME, b.BRAND_NAME b_BRAND_NAME from LIQUOR_LICENSES a join (select * from LIQUOR_LICENSES order by BRAND_NAME desc) b on b.BRAND_NAME = a.BRAND_NAME;"
									  ,"DB_Files_2_Test_Versions/Select_Test_45.csv"
									  ,&malloced_head, the_debug) != 0)
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
		// END Test with id = 1045

		// START Test with id = 1046
			if (test_Driver_selectStuff(1046, "select EFFECTIVE, BRAND_NAME from LIQUOR_LICENSES where EFFECTIVE > '3/17/2023' order by BRAND_NAME desc"
									  ,"DB_Files_2_Test_Versions/Select_Test_46.csv"
									  ,&malloced_head, the_debug) != 0)
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
		// END Test with id = 1046

		// START Test with id = 1047
			if (test_Driver_selectStuff(1047, "select a.BRAND_NAME a_BRAND_NAME, b.BRAND_NAME b_BRAND_NAME from LIQUOR_LICENSES a join (select * from LIQUOR_LICENSES where EFFECTIVE > '3/17/2023' order by BRAND_NAME desc) b on b.BRAND_NAME = a.BRAND_NAME;"
									  ,"DB_Files_2_Test_Versions/Select_Test_47.csv"
									  ,&malloced_head, the_debug) != 0)
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
		// END Test with id = 1047

		// START Test with id = 1048
			if (test_Driver_selectStuff(1048, "select a.BRAND_NAME a_BRAND_NAME, b.BRAND_NAME b_BRAND_NAME from LIQUOR_LICENSES a left join (select * from LIQUOR_LICENSES where EFFECTIVE > '3/17/2023' order by BRAND_NAME desc) b on b.BRAND_NAME = a.BRAND_NAME;"
									  ,"DB_Files_2_Test_Versions/Select_Test_48.csv"
									  ,&malloced_head, the_debug) != 0)
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
		// END Test with id = 1048
			
		// START Test with id = 1049
			if (test_Driver_selectStuff(1049, "select a.BRAND_NAME a_BRAND_NAME, a.CT_REGISTRATION_NUMBER a_CT_REGISTRATION_NUMBER, b.BRAND_NAME b_BRAND_NAME, b.CT_REGISTRATION_NUMBER b_CT_REGISTRATION_NUMBER from LIQUOR_LICENSES a left join (select * from LIQUOR_LICENSES where EFFECTIVE > '3/17/2023' order by BRAND_NAME desc) b on b.BRAND_NAME = a.BRAND_NAME;"
									  ,"DB_Files_2_Test_Versions/Select_Test_49.csv"
									  ,&malloced_head, the_debug) != 0)
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
		// END Test with id = 1049
			
		// START Test with id = 1050
			if (test_Driver_selectStuff(1050, "select a.BRAND_NAME a_BRAND_NAME, a.CT_REGISTRATION_NUMBER a_CT_REGISTRATION_NUMBER, b.BRAND_NAME b_BRAND_NAME, b.CT_REGISTRATION_NUMBER b_CT_REGISTRATION_NUMBER from LIQUOR_LICENSES a right join (select * from LIQUOR_LICENSES where EFFECTIVE > '3/17/2023' order by BRAND_NAME desc) b on b.BRAND_NAME = a.BRAND_NAME;"
									  ,"DB_Files_2_Test_Versions/Select_Test_50.csv"
									  ,&malloced_head, the_debug) != 0)
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
		// END Test with id = 1050

		// START Test with id = 1051
			if (test_Driver_selectStuff(1051, "select a.BRAND_NAME a_BRAND_NAME, a.CT_REGISTRATION_NUMBER a_CT_REGISTRATION_NUMBER, b.BRAND_NAME b_BRAND_NAME, b.CT_REGISTRATION_NUMBER b_CT_REGISTRATION_NUMBER from (select * from LIQUOR_LICENSES where EFFECTIVE > '3/17/2023') a right join (select * from LIQUOR_LICENSES order by BRAND_NAME desc) b on b.BRAND_NAME = a.BRAND_NAME"
									  ,"DB_Files_2_Test_Versions/Select_Test_51.csv"
									  ,&malloced_head, the_debug) != 0)
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
		// END Test with id = 1051

		// START Test with id = 1052
			if (test_Driver_selectStuff(1052, "select a.BRAND_NAME a_BRAND_NAME, a.CT_REGISTRATION_NUMBER a_CT_REGISTRATION_NUMBER, b.BRAND_NAME b_BRAND_NAME, b.CT_REGISTRATION_NUMBER b_CT_REGISTRATION_NUMBER from (select * from LIQUOR_LICENSES where EFFECTIVE > '3/17/2023') a outer join (select * from LIQUOR_LICENSES where EXPIRATION = '5/15/2026' or EXPIRATION = '1/31/2025' order by BRAND_NAME desc) b on b.BRAND_NAME = a.BRAND_NAME ;"
									  ,"DB_Files_2_Test_Versions/Select_Test_52.csv"
									  ,&malloced_head, the_debug) != 0)
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
		// END Test with id = 1052

		// START Test with id = 1053
			if (test_Driver_selectStuff(1053, "select a.BRAND_NAME a_BRAND_NAME, a.CT_REGISTRATION_NUMBER a_CT_REGISTRATION_NUMBER, b.BRAND_NAME b_BRAND_NAME, b.CT_REGISTRATION_NUMBER b_CT_REGISTRATION_NUMBER from (select * from  LIQUOR_LICENSES  where EFFECTIVE > '3/17/2023' )a outer join (select * from LIQUOR_LICENSES) b on b.BRAND_NAME = a.BRAND_NAME;"
									  ,"DB_Files_2_Test_Versions/Select_Test_53.csv"
									  ,&malloced_head, the_debug) != 0)
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
		// END Test with id = 1053

		// START Test with id = 1054
			if (test_Driver_selectStuff(1054, "select distinct EXPIRATION from LIQUOR_LICENSES  where EXPIRATION = '1/31/2025';"
									  ,"DB_Files_2_Test_Versions/Select_Test_54.csv"
									  ,&malloced_head, the_debug) != 0)
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
		// END Test with id = 1054

		// START Test with id = 1055
			if (test_Driver_selectStuff(1055, "select distinct EFFECTIVE, EXPIRATION, OUT_OF_STATE_SHIPPER from LIQUOR_LICENSES;"
									  ,"DB_Files_2_Test_Versions/Select_Test_55.csv"
									  ,&malloced_head, the_debug) != 0)
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
		// END Test with id = 1055

		// START Test with id = 1056
			if (test_Driver_selectStuff(1056, "select a.BRAND_NAME a_BRAND_NAME, a.EXPIRATION a_EXPIRATION, b.EXPIRATION b_EXPIRATION from LIQUOR_LICENSES a join (select distinct EXPIRATION from LIQUOR_LICENSES  where EXPIRATION = '1/31/2025') b on a.EXPIRATION = b.EXPIRATION;"
									  ,"DB_Files_2_Test_Versions/Select_Test_56.csv"
									  ,&malloced_head, the_debug) != 0)
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
		// END Test with id = 1056

		// START Test with id = 1057
			if (test_Driver_selectStuff(1057, "select a.BRAND_NAME, b.CT_REGISTRATION_NUMBER from (select * from LIQUOR_LICENSES where EFFECTIVE > '3/17/2023') a outer join (select * from LIQUOR_LICENSES where EXPIRATION = '5/15/2026' or EXPIRATION = '1/31/2025' order by BRAND_NAME desc) b on b.BRAND_NAME = a.BRAND_NAME ;"
									  ,"DB_Files_2_Test_Versions/Select_Test_57.csv"
									  ,&malloced_head, the_debug) != 0)
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
		// END Test with id = 1057

		// START Test with id = 1058
			if (test_Driver_selectStuff(1058, "select * from ( select a.BRAND_NAME, b.CT_REGISTRATION_NUMBER from (select * from LIQUOR_LICENSES where EFFECTIVE > '3/17/2023') a outer join (select * from LIQUOR_LICENSES where EXPIRATION = '5/15/2026' or EXPIRATION = '1/31/2025' order by BRAND_NAME desc) b on b.BRAND_NAME = a.BRAND_NAME) tbl;"
									  ,"DB_Files_2_Test_Versions/Select_Test_57.csv"
									  ,&malloced_head, the_debug) != 0)
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
		// END Test with id = 1058

		// START Test with id = 1059
			if (test_Driver_selectStuff(1059, "select * from ( select a.BRAND_NAME, b.CT_REGISTRATION_NUMBER from (select * from LIQUOR_LICENSES where EFFECTIVE > '3/17/2023') a outer join (select * from LIQUOR_LICENSES where EXPIRATION = '5/15/2026' or EXPIRATION = '1/31/2025' order by BRAND_NAME desc) b on b.BRAND_NAME = a.BRAND_NAME) tbl where BRAND_NAME is null;"
									  ,"DB_Files_2_Test_Versions/Select_Test_59.csv"
									  ,&malloced_head, the_debug) != 0)
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
		// END Test with id = 1059

		// START Test with id = 1060
			if (test_Driver_selectStuff(1060, "select * from ( select a.BRAND_NAME, b.CT_REGISTRATION_NUMBER from (select * from LIQUOR_LICENSES where EFFECTIVE > '3/17/2023') a outer join (select * from LIQUOR_LICENSES where EXPIRATION = '5/15/2026' or EXPIRATION = '1/31/2025' order by BRAND_NAME desc) b on b.BRAND_NAME = a.BRAND_NAME) tbl where BRAND_NAME is not null;"
									  ,"DB_Files_2_Test_Versions/Select_Test_60.csv"
									  ,&malloced_head, the_debug) != 0)
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
		// END Test with id = 1060

		// START Test with id = 1061
			if (test_Driver_selectStuff(1061, "select a.BRAND_NAME a_BRAND_NAME, b.BRAND_NAME b_BRAND_NAME, b.CT_REGISTRATION_NUMBER from (select * from LIQUOR_LICENSES where EFFECTIVE > '3/17/2023') a left join LIQUOR_LICENSES b on b.BRAND_NAME = a.BRAND_NAME where b.BRAND_NAME is not null;"
									  ,"DB_Files_2_Test_Versions/Select_Test_61.csv"
									  ,&malloced_head, the_debug) != 0)
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
		// END Test with id = 1061
			
		// START Test with id = 1062
			if (test_Driver_selectStuff(1062, "select a.BRAND_NAME, b.CT_REGISTRATION_NUMBER from (select * from LIQUOR_LICENSES where EFFECTIVE > '3/17/2023') a outer join (select * from LIQUOR_LICENSES where EXPIRATION = '5/15/2026' or EXPIRATION = '1/31/2025' order by BRAND_NAME desc) b on b.BRAND_NAME = a.BRAND_NAME where a.BRAND_NAME is not null;"
									  ,"DB_Files_2_Test_Versions/Select_Test_60.csv"
									  ,&malloced_head, the_debug) != 0)
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
		// END Test with id = 1062
			
		// START Test with id = 1063
			if (test_Driver_selectStuff(1063, "select * as \"COL_#\" from (select BRAND_NAME from LIQUOR_LICENSES where BRAND_NAME = 'WOODINVILLE STRAIGHT BOURBON TRAYPACK') a cross join (select BRAND_NAME from LIQUOR_LICENSES where BRAND_NAME = 'EDINBURGH GIN SEASIDE') b;"
									  ,"DB_Files_2_Test_Versions/Select_Test_63.csv"
									  ,&malloced_head, the_debug) != 0)
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
		// END Test with id = 1063
			
		// START Test with id = 1064
			if (test_Driver_selectStuff(1064, "select * as \"COL_#\" from (select BRAND_NAME from LIQUOR_LICENSES) a cross join (select BRAND_NAME from LIQUOR_LICENSES) b;"
									  ,"DB_Files_2_Test_Versions/Select_Test_89.csv"
									  ,&malloced_head, the_debug) != 0)
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
		// END Test with id = 1064

		// START Test with id = 1065
			if (test_Driver_selectStuff(1065, "select case when a.BRAND_NAME is not null and b.CT_REGISTRATION_NUMBER is not null then 'BOTH' when a.BRAND_NAME is not null then 'A' else 'B' end OUTER_CASE from (select * from LIQUOR_LICENSES where EFFECTIVE > '3/17/2023') a outer join (select * from LIQUOR_LICENSES where EXPIRATION = '5/15/2026' or EXPIRATION = '1/31/2025' order by BRAND_NAME desc) b on b.BRAND_NAME = a.BRAND_NAME;"
									  ,"DB_Files_2_Test_Versions/Select_Test_65.csv"
									  ,&malloced_head, the_debug) != 0)
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
		// END Test with id = 1065

		// START Test with id = 1066
			if (test_Driver_selectStuff(1066, "select * from (select case when a.BRAND_NAME is not null and b.CT_REGISTRATION_NUMBER is not null then 'BOTH' when a.BRAND_NAME is not null then 'A' else 'B' end OUTER_CASE from (select * from LIQUOR_LICENSES  where EFFECTIVE > '3/17/2023') a outer join (select * from LIQUOR_LICENSES where EXPIRATION = '5/15/2026' or EXPIRATION = '1/31/2025' order by BRAND_NAME desc) b on b.BRAND_NAME = a.BRAND_NAME) tbl;"
									  ,"DB_Files_2_Test_Versions/Select_Test_65.csv"
									  ,&malloced_head, the_debug) != 0)
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
		// END Test with id = 1066

		// START Test with id = 1067
			if (test_Driver_selectStuff(1067, "select case when A_CASE = 'na' then 'maybe' else 'YEP' end A_NEW_CASE from (select case when BRAND_NAME = 'WOODINVILLE STRAIGHT BOURBON TRAYPACK' then 'WOODINVILLE STRAIGHT BOURBON TRAYPACK' else 'na' end A_CASE from LIQUOR_LICENSES) tbl;"
									  ,"DB_Files_2_Test_Versions/Select_Test_67.csv"
									  ,&malloced_head, the_debug) != 0)
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
		// END Test with id = 1067

		// START Test with id = 1068
			if (test_Driver_selectStuff(1068, "select MATH_BRO + 3 MORE_MATH from (select CT_REGISTRATION_NUMBER * 100 MATH_BRO from LIQUOR_LICENSES) tbl"
									  ,"DB_Files_2_Test_Versions/Select_Test_68.csv"
									  ,&malloced_head, the_debug) != 0)
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
		// END Test with id = 1068

		// START Test with id = 1069
			if (test_Driver_selectStuff(1069, "select a.A_CASE, b.BRAND_NAME from (select case when BRAND_NAME = 'WOODINVILLE STRAIGHT BOURBON TRAYPACK' then 'WOODINVILLE STRAIGHT BOURBON TRAYPACK' else 'na' end A_CASE from LIQUOR_LICENSES) a join (select BRAND_NAME from LIQUOR_LICENSES where BRAND_NAME = 'WOODINVILLE STRAIGHT BOURBON TRAYPACK') b on a.A_CASE = b.BRAND_NAME;"
									  ,"DB_Files_2_Test_Versions/Select_Test_69.csv"
									  ,&malloced_head, the_debug) != 0)
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
		// END Test with id = 1069

		/*// START Test with id = 1070
			if (test_Driver_selectStuff(1070, "select EXPIRATION, case when EXPIRATION = '1/31/2025' then COUNT(*) else -1 end new_case from LIQUOR_LICENSES group by EXPIRATION;"
									  ,"DB_Files_2_Test_Versions/Select_Test_70.csv"
									  ,&malloced_head, the_debug) != 0)
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
		// END Test with id = 1070*/

		// START Test with id = 1071
			if (test_Driver_selectStuff(1071, "select case when EXPIRATION = '1/31/2025' then 'DEF NO' when EXPIRATION = '3/16/2025' then 'NO' else BRAND_NAME end new_case from LIQUOR_LICENSES;"
									  ,"DB_Files_2_Test_Versions/Select_Test_71.csv"
									  ,&malloced_head, the_debug) != 0)
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
		// END Test with id = 1071

		// START Test with id = 1072
			if (test_Driver_selectStuff(1072, "select a.BRAND_NAME, b.CT_REGISTRATION_NUMBER * 100 MATH_BOI from (select * from LIQUOR_LICENSES a where a.EFFECTIVE > '3/17/2023') a outer join (select * from LIQUOR_LICENSES where EXPIRATION = '5/15/2026' or EXPIRATION = '1/31/2025' order by BRAND_NAME desc) b on b.BRAND_NAME = a.BRAND_NAME"
									  ,"DB_Files_2_Test_Versions/Select_Test_72.csv"
									  ,&malloced_head, the_debug) != 0)
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
		// END Test with id = 1072

		// START Test with id = 1073
			if (test_Driver_selectStuff(1073, "select a.BRAND_NAME, avg(b.CT_REGISTRATION_NUMBER) AVG_FUNC from (select * from LIQUOR_LICENSES  where EFFECTIVE > '3/17/2023') a outer join (select * from LIQUOR_LICENSES where EXPIRATION = '5/15/2026' or EXPIRATION = '1/31/2025' order by BRAND_NAME desc) b on b.BRAND_NAME = a.BRAND_NAME group by a.BRAND_NAME;"
									  ,"DB_Files_2_Test_Versions/Select_Test_73.csv"
									  ,&malloced_head, the_debug) != 0)
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
		// END Test with id = 1073

		// START Test with id = 1074
			if (test_Driver_selectStuff(1074, "select a.CT_REGISTRATION_NUMBER_NEW A_CT_REGISTRATION_NUMBER_NEW, b.CT_REGISTRATION_NUMBER B_CT_REGISTRATION_NUMBER from (select CT_REGISTRATION_NUMBER * 100 CT_REGISTRATION_NUMBER_NEW from LIQUOR_LICENSES) a join LIQUOR_LICENSES b on a.CT_REGISTRATION_NUMBER_NEW = b.CT_REGISTRATION_NUMBER * 100"
									  ,"DB_Files_2_Test_Versions/Select_Test_74.csv"
									  ,&malloced_head, the_debug) != 0)
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
		// END Test with id = 1074

		// START Test with id = 1075
			if (test_Driver_selectStuff(1075, "select a.BRAND_NAME, max(b.CT_REGISTRATION_NUMBER) AVG_FUNC from (select * from LIQUOR_LICENSES where EFFECTIVE > '3/17/2023') a outer join (select * from LIQUOR_LICENSES where EXPIRATION = '5/15/2026' or EXPIRATION = '1/31/2025' order by BRAND_NAME desc) b on b.BRAND_NAME = a.BRAND_NAME group by a.BRAND_NAME;"
									  ,"DB_Files_2_Test_Versions/Select_Test_75.csv"
									  ,&malloced_head, the_debug) != 0)
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
		// END Test with id = 1075

		/*// START Test with id = 1076
			if (test_Driver_selectStuff(1076, "select EXPIRATION, COUNT(*) * 100 MATH from LIQUOR_LICENSES group by EXPIRATION;"
									  ,"DB_Files_2_Test_Versions/Select_Test_76.csv"
									  ,&malloced_head, the_debug) != 0)
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
		// END Test with id = 1076*/

		// START Test with id = 1077
			if (test_Driver_selectStuff(1077, "select EXPIRATION, count(*) CNT from LIQUOR_LICENSES group by EXPIRATION having count(*) > 1;"
									  ,"DB_Files_2_Test_Versions/Select_Test_23.csv"
									  ,&malloced_head, the_debug) != 0)
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
		// END Test with id = 1077

		// START Test with id = 1078
			if (test_Driver_selectStuff(1078, "select EXPIRATION, count(*) CNT from LIQUOR_LICENSES group by EXPIRATION having avg(CT_REGISTRATION_NUMBER) > 150000.0;"
									  ,"DB_Files_2_Test_Versions/Select_Test_78.csv"
									  ,&malloced_head, the_debug) != 0)
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
		// END Test with id = 1078

		// START Test with id = 1079
			if (test_Driver_selectStuff(1079, "select * from LIQUOR_LICENSES where CT_REGISTRATION_NUMBER * 10 > 2000000;"
									  ,"DB_Files_2_Test_Versions/Select_Test_79.csv"
									  ,&malloced_head, the_debug) != 0)
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
		// END Test with id = 1079

		// START Test with id = 1080
			if (test_Driver_selectStuff(1080, "select EXPIRATION, count(*) CNT, avg(CT_REGISTRATION_NUMBER) AVG from LIQUOR_LICENSES where BRAND_NAME <> 'WOODINVILLE STRAIGHT BOURBON TRAYPACK' group by EXPIRATION having avg(CT_REGISTRATION_NUMBER) > 150000.0 order by EXPIRATION;"
									  ,"DB_Files_2_Test_Versions/Select_Test_80.csv"
									  ,&malloced_head, the_debug) != 0)
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
		// END Test with id = 1080
			
		// START Test with id = 1081
			if (test_Driver_selectStuff(1081, "select * from LIQUOR_LICENSES where BRAND_NAME = BRAND_NAME;"
									  ,"DB_Files_2_Test_Versions/Select_Test_1.csv"
									  ,&malloced_head, the_debug) != 0)
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
		// END Test with id = 1081

		// START Test with id = 1082
			if (test_Driver_selectStuff(1082, "select * from LIQUOR_LICENSES where BRAND_NAME = STATUS;"
									  ,"DB_Files_2_Test_Versions/Select_Test_82.csv"
									  ,&malloced_head, the_debug) != 0)
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
		// END Test with id = 1082

		// START Test with id = 1083
			if (test_Driver_selectStuff(1083, "select BRAND_NAME, 1 ONE, 1 ONE_AGAIN from LIQUOR_LICENSES where ONE = ONE_AGAIN;"
									  ,"DB_Files_2_Test_Versions/Select_Test_83.csv"
									  ,&malloced_head, the_debug) != 0)
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
		// END Test with id = 1083

		// START Test with id = 1084
			if (test_Driver_selectStuff(1084, "select median(CT_REGISTRATION_NUMBER) MED from ( select a.BRAND_NAME, b.CT_REGISTRATION_NUMBER from (select * from LIQUOR_LICENSES where EFFECTIVE > '3/17/2023') a outer join (select * from LIQUOR_LICENSES where EXPIRATION = '5/15/2026' or EXPIRATION = '1/31/2025' order by BRAND_NAME desc) b on a.BRAND_NAME = b.BRAND_NAME) tbl;"
									  ,"DB_Files_2_Test_Versions/Select_Test_84.csv"
									  ,&malloced_head, the_debug) != 0)
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
		// END Test with id = 1084

		// START Test with id = 1085
			if (test_Driver_selectStuff(1085, "select BRAND_NAME, case when EXPIRATION = '1/31/2025' then 'Yep' end new_case from LIQUOR_LICENSES;"
									  ,"DB_Files_2_Test_Versions/Select_Test_85.csv"
									  ,&malloced_head, the_debug) != 0)
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
		// END Test with id = 1085

		// START Test with id = 1086
			if (test_Driver_selectStuff(1086, "select * from (select BRAND_NAME, case when EXPIRATION = '1/31/2025' then 'Yep' end new_case from LIQUOR_LICENSES) tbl where new_case = 'Yep';"
									  ,"DB_Files_2_Test_Versions/Select_Test_86.csv"
									  ,&malloced_head, the_debug) != 0)
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
		// END Test with id = 1086

		// START Test with id = 1087
			if (test_Driver_selectStuff(1087, "select BRAND_NAME, case when EXPIRATION = '1/31/2025' then NULL else null end new_case from LIQUOR_LICENSES;"
									  ,"DB_Files_2_Test_Versions/Select_Test_87.csv"
									  ,&malloced_head, the_debug) != 0)
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
		// END Test with id = 1087

		// START Test with id = 1088
			if (test_Driver_selectStuff(1088, "select *, '' EMPTY_STRING, case when EXPIRATION = '1/31/2025' then '' else null end new_case from LIQUOR_LICENSES;"
									  ,"DB_Files_2_Test_Versions/Select_Test_88.csv"
									  ,&malloced_head, the_debug) != 0)
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
		// END Test with id = 1088

		// START Test with id = 1089
			if (test_Driver_selectStuff(1089, "select * as \"COL_#\" from (select BRAND_NAME from LIQUOR_LICENSES) a join (select BRAND_NAME from LIQUOR_LICENSES) b on 1 = 1;"
									  ,"DB_Files_2_Test_Versions/Select_Test_89.csv"
									  ,&malloced_head, the_debug) != 0)
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
		// END Test with id = 1089

		// START Test with id = 1090
			if (test_Driver_selectStuff(1090, "select * as \"COL_#\" from (select BRAND_NAME from LIQUOR_LICENSES) a join (select BRAND_NAME from LIQUOR_LICENSES) b on a.BRAND_NAME = 'WOODINVILLE STRAIGHT BOURBON TRAYPACK';"
									  ,"DB_Files_2_Test_Versions/Select_Test_90.csv"
									  ,&malloced_head, the_debug) != 0)
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
		// END Test with id = 1090

		// START Test with id = 1091
			if (test_Driver_selectStuff(1091, "select a.BRAND_NAME a_BRAND_NAME, b.BRAND_NAME B_BRAND_NAME, b.CT_REGISTRATION_NUMBER * 1000 MATH_BRO from (select * from LIQUOR_LICENSES  where EFFECTIVE > '3/17/2023') a outer join (select * from LIQUOR_LICENSES where EXPIRATION = '5/15/2026' or EXPIRATION = '1/31/2025' order by BRAND_NAME desc) b on b.BRAND_NAME = a.BRAND_NAME;"
									  ,"DB_Files_2_Test_Versions/Select_Test_91.csv"
									  ,&malloced_head, the_debug) != 0)
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
		// END Test with id = 1091

		// START Test with id = 1092
			if (test_Driver_selectStuff(1092, "select CT_REGISTRATION_NUMBER * case when CT_REGISTRATION_NUMBER < 100000 then 10000 else 10 end MATH_STUFF from LIQUOR_LICENSES;"
									  ,"DB_Files_2_Test_Versions/Select_Test_92.csv"
									  ,&malloced_head, the_debug) != 0)
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
		// END Test with id = 1092

		// START Test with id = 1093
			if (test_Driver_selectStuff(1093, "select case when CT_REGISTRATION_NUMBER < 100000 then 10000 else 10 end * CT_REGISTRATION_NUMBER MATH_STUFF from LIQUOR_LICENSES;"
									  ,"DB_Files_2_Test_Versions/Select_Test_92.csv"
									  ,&malloced_head, the_debug) != 0)
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
		// END Test with id = 1093

		// START Test with id = 1094
			if (test_Driver_selectStuff(1094, "select * from LIQUOR_LICENSES where case when CT_REGISTRATION_NUMBER < 100000 then 10000 else 10 end = case when CT_REGISTRATION_NUMBER < 100000 then 10000 else 10 end;"
									  ,"DB_Files_2_Test_Versions/Select_Test_1.csv"
									  ,&malloced_head, the_debug) != 0)
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
		// END Test with id = 1094

		// START Test with id = 1095
			if (test_Driver_selectStuff(1095, "select a.BRAND_NAME a_BRAND_NAME, b.BRAND_NAME b_BRAND_NAME from LIQUOR_LICENSES a join (select * from LIQUOR_LICENSES order by BRAND_NAME desc) b on case when a.CT_REGISTRATION_NUMBER > 0 then a.BRAND_NAME else a.STATUS end = case when b.CT_REGISTRATION_NUMBER > 0 then b.BRAND_NAME else b.STATUS end;"
									  ,"DB_Files_2_Test_Versions/Select_Test_34.csv"
									  ,&malloced_head, the_debug) != 0)
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
		// END Test with id = 1095

		// START Test with id = 1096
			if (test_Driver_selectStuff(1096, "select b.CT_REGISTRATION_NUMBER * 1000 MATH_BRO from (select * from LIQUOR_LICENSES where EFFECTIVE > '3/17/2023') a outer join (select * from LIQUOR_LICENSES where EXPIRATION = '5/15/2026' or EXPIRATION = '1/31/2025' order by BRAND_NAME desc) b on b.BRAND_NAME = a.BRAND_NAME ;"
									  ,"DB_Files_2_Test_Versions/Select_Test_96.csv"
									  ,&malloced_head, the_debug) != 0)
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
		// END Test with id = 1096

		// START Test with id = 1097
			if (test_Driver_selectStuff(1097, "select * from LIQUOR_LICENSES_FULL where BRAND_NAME = 'WOODINVILLE STRAIGHT BOURBON TRAYPACK';"
									  ,"DB_Files_2_Test_Versions/Select_Test_97.csv"
									  ,&malloced_head, the_debug) != 0)
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
		// END Test with id = 1097

		// START Test with id = 1098
			if (test_Driver_selectStuff(1098, "select * from LIQUOR_LICENSES_FULL;"
									  ,"DB_Files_2_Test_Versions/Select_Test_98.csv"
									  ,&malloced_head, the_debug) != 0)
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
		// END Test with id = 1098

		// START Test with id = 1099
			if (test_Driver_selectStuff(1099, "select a.BRAND_NAME a_BRAND_NAME, a.CT_REGISTRATION_NUMBER a_CT_REGISTRATION_NUMBER, b.BRAND_NAME b_BRAND_NAME, b.CT_REGISTRATION_NUMBER b_CT_REGISTRATION_NUMBER from LIQUOR_LICENSES a right join (select * from LIQUOR_LICENSES order by BRAND_NAME desc) b on b.BRAND_NAME = a.BRAND_NAME  where a.EFFECTIVE > '3/17/2023';"
									  ,"DB_Files_2_Test_Versions/Select_Test_99.csv"
									  ,&malloced_head, the_debug) != 0)
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
		// END Test with id = 1099

		// START Test with id = 1100
			if (test_Driver_selectStuff(1100, "select FIRST(EFFECTIVE) FIRST from LIQUOR_LICENSES"
									  ,"DB_Files_2_Test_Versions/Select_Test_100.csv"
									  ,&malloced_head, the_debug) != 0)
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
		// END Test with id = 1100

		// START Test with id = 1101
			if (test_Driver_selectStuff(1101, "select FIRST(EFFECTIVE) FIRST from LIQUOR_LICENSES where EXPIRATION > '3/17/2026'"
									  ,"DB_Files_2_Test_Versions/Select_Test_101.csv"
									  ,&malloced_head, the_debug) != 0)
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
		// END Test with id = 1101
	// END test_Driver_selectStuff

	/*// START test_Driver_updateRows
		// START Test with id = 701
			if (test_Driver_updateRows(701, "DB_Files_2_Test_Versions/Update_Test_1.csv"
									  ,"update LIQUOR_LICENSES set STATUS = 'TST_ACTIVE' where EXPIRATION = '1/31/2025';"
									  ,&malloced_head, the_debug) != 0)
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
		// END Test with id = 701
			
		// START Test with id = 702
			if (test_Driver_updateRows(702, "DB_Files_2_Test_Versions/Update_Test_2_v2.csv"
									  ,"update LIQUOR_LICENSES set STATUS = 'SOMETHING' where EXPIRATION = '1/1/1900';"
							  		  ,&malloced_head, the_debug) != 0)
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
		// END Test with id = 702

		// START Test with id = 703
			if (test_Driver_updateRows(703, "DB_Files_2_Test_Versions/Update_Test_3_v2.csv"
									  ,"update LIQUOR_LICENSES set STATUS = 'VRY_ACTIVE' where STATUS = 'ACTIVE';"
							  		  ,&malloced_head, the_debug) != 0)
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
		// END Test with id = 703

		// START Test with id = 704
			if (test_Driver_updateRows(704, "DB_Files_2_Test_Versions/Update_Test_4.csv"
									  ,"update LIQUOR_LICENSES set STATUS = 'EXT_ACTIVE', CT_REGISTRATION_NUMBER = 1 where STATUS = 'VRY_ACTIVE';"
							  		  ,&malloced_head, the_debug) != 0)
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
		// END Test with id = 704

		// START Test with id = 705
			if (test_Driver_updateRows(705, "DB_Files_2_Test_Versions/Update_Test_5.csv"
									  ,"update LIQUOR_LICENSES set CT_REGISTRATION_NUMBER = 2, EFFECTIVE = '1/1/1900' where CT_REGISTRATION_NUMBER = 1;"
							  		  ,&malloced_head, the_debug) != 0)
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
		// END Test with id = 705

		// START Test with id = 706
			if (test_Driver_updateRows(706, "DB_Files_2_Test_Versions/Update_Test_6.csv"
									  ,"update LIQUOR_LICENSES set CT_REGISTRATION_NUMBER = 3, EXPIRATION = '12/12/1901', SUPERVISOR_CREDENTIAL = 'A_CREDENTIAL';"
							  		  ,&malloced_head, the_debug) != 0)
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
		// END Test with id = 706

		// START Test with id = 707
			if (test_Driver_updateRows(707, "DB_Files_2_Test_Versions/Update_Test_7.csv"
									  ,"update LIQUOR_LICENSES set CT_REGISTRATION_NUMBER = -1;"
							  		  ,&malloced_head, the_debug) != 0)
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
		// END Test with id = 707
	// END test_Driver_updateRows

	// START test_Driver_deleteRows
		// START Test with id = 801
			if (test_Driver_deleteRows(801, "delete from LIQUOR_LICENSES where BRAND_NAME = 'IDK_BUT_SHOULD_BE_NO_ROWS';"
									  ,"DB_Files_2_Test_Versions/Delete_Test_1.csv", 1000, 0
							  		  ,&malloced_head, the_debug) != 0)
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
		// END Test with id = 801
			
		// START Test with id = 802
			if (test_Driver_deleteRows(802, "delete from LIQUOR_LICENSES where BRAND_NAME = 'VIZZY BLACK CHERRY LIME';"
									  ,"DB_Files_2_Test_Versions/Delete_Test_2.csv", 1000, 1
							  		  ,&malloced_head, the_debug) != 0)
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
		// END Test with id = 802

		// START Test with id = 803
			if (test_Driver_deleteRows(803, "delete from LIQUOR_LICENSES where BRAND_NAME = '860 INDA PALE ALE' or BRAND_NAME = '242 (NOBLE VINES) SAUVIGNON BLANC SAN BERNABE MONTEREY';"
									  ,"DB_Files_2_Test_Versions/Delete_Test_3.csv", 1000, 3
							  		  ,&malloced_head, the_debug) != 0)
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
		// END Test with id = 803

		// START Test with id = 804
			if (test_Driver_deleteRows(804, "delete from LIQUOR_LICENSES where BRAND_NAME <> 'FITAPRETA TINTO' and BRAND_NAME <> 'TWISTED TEA PARTY POUCH' and BRAND_NAME <> 'EAT BEER SIP KILLER';"
									  ,"DB_Files_2_Test_Versions/Delete_Test_4.csv", 1000, 997
							  		  ,&malloced_head, the_debug) != 0)
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
		// END Test with id = 804

		// START Test with id = 805
			if (test_Driver_deleteRows(805, "delete from LIQUOR_LICENSES;"
									  ,"DB_Files_2_Test_Versions/Delete_Test_5.csv", 0, 0
							  		  ,&malloced_head, the_debug) != 0)
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
		// END Test with id = 805
	// END test_Driver_deleteRows

	// START test_Driver_insertRows
		// START Test with id = 901
			if (test_Driver_insertRows(901, "Insert into LIQUOR_LICENSES (BRAND_NAME, STATUS) values ('Hi', 'Hello');"
										  ,"DB_Files_2_Test_Versions/Insert_Test_1.csv", 1, 0
								  		  ,&malloced_head, the_debug) != 0)
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
		// END Test with id = 901

		// START Test with id = 902
			if (test_Driver_insertRows(902, "Insert into LIQUOR_LICENSES (BRAND_NAME,CT_REGISTRATION_NUMBER,STATUS,EFFECTIVE,EXPIRATION,OUT_OF_STATE_SHIPPER,SUPERVISOR_CREDENTIAL) values ('Hi2', 1, 'Hello2', '10/1/2023', '10/1/2023', 'Some shipper', 'Some credential');"
										  ,"DB_Files_2_Test_Versions/Insert_Test_2.csv", 2, 0
								  		  ,&malloced_head, the_debug) != 0)
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
		// END Test with id = 902

		// START Test with id = 903
			if (test_Driver_insertRows(903, "Insert into LIQUOR_LICENSES (BRAND_NAME,CT_REGISTRATION_NUMBER,STATUS,EFFECTIVE,EXPIRATION,OUT_OF_STATE_SHIPPER,SUPERVISOR_CREDENTIAL) values ('Hi3', 2, 'Hello3', '10/2/2023', '10/2/2023', 'Some other shipper', 'Some other credential');"
										  ,"DB_Files_2_Test_Versions/Insert_Test_3.csv", 3, 0
								  		  ,&malloced_head, the_debug) != 0)
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
		// END Test with id = 903

		// START Test with id = 904
			if (test_Driver_deleteRows(904, "delete from LIQUOR_LICENSES where BRAND_NAME = 'Hi';"
									  ,"DB_Files_2_Test_Versions/Delete_Test_6.csv", 3, 1
							  		  ,&malloced_head, the_debug) != 0)
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
		// END Test with id = 904

		// START Test with id = 905
			if (test_Driver_insertRows(905, "Insert into LIQUOR_LICENSES (BRAND_NAME, STATUS) values ('Hi2', 'Hello3');"
										  ,"DB_Files_2_Test_Versions/Insert_Test_3_pt_2.csv", 3, 0
								  		  ,&malloced_head, the_debug) != 0)
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
		// END Test with id = 905

		// START Test with id = 906
			if (test_Driver_insertRows(906, "Insert into LIQUOR_LICENSES (BRAND_NAME, STATUS) values ('Hi4', 'Hello4');"
										  ,"DB_Files_2_Test_Versions/Insert_Test_4.csv", 4, 0
								  		  ,&malloced_head, the_debug) != 0)
				result = -1;

			if (malloced_head != NULL)
			{
				if (the_debug == YES_DEBUG)
					printf("	ERROR in test_Driver_main() at line %d in %s\n", __LINE__, __FILE__);
				myFreeAllError(&malloced_head, the_debug);
				result = -1;
			}
		// END Test with id = 906

		// START Test with id = 907
			if (test_Driver_deleteRows(907, "delete from LIQUOR_LICENSES where BRAND_NAME = 'Hi2' or BRAND_NAME = 'Hi4';"
									  ,"DB_Files_2_Test_Versions/Delete_Test_7.csv", 4, 3
							  		  ,&malloced_head, the_debug) != 0)
				result = -1;

			if (malloced_head != NULL)
			{
				if (the_debug == YES_DEBUG)
					printf("	ERROR in test_Driver_main() at line %d in %s\n", __LINE__, __FILE__);
				myFreeAllError(&malloced_head, the_debug);
				result = -1;
			}
		// END Test with id = 907

		// START Test with id = 908
			if (test_Driver_insertRows(908, "Insert into LIQUOR_LICENSES (BRAND_NAME, STATUS) values ('Hi', 'Hello'), ('Hi', 'Hello');"
										  ,"DB_Files_2_Test_Versions/Insert_Test_5.csv", 4, 1
								  		  ,&malloced_head, the_debug) != 0)
				result = -1;

			if (malloced_head != NULL)
			{
				if (the_debug == YES_DEBUG)
					printf("	ERROR in test_Driver_main() at line %d in %s\n", __LINE__, __FILE__);
				myFreeAllError(&malloced_head, the_debug);
				result = -1;
			}
		// END Test with id = 908

		// START Test with id = 909
			if (test_Driver_insertRows(909, "Insert into LIQUOR_LICENSES (BRAND_NAME,CT_REGISTRATION_NUMBER,STATUS,EFFECTIVE,EXPIRATION,OUT_OF_STATE_SHIPPER,SUPERVISOR_CREDENTIAL) values ('100', 100, '100','2/2/2024','2/2/2024','100','100'), ('100', 100, '100','2/2/2024','2/2/2024','100','100');"
										  ,"DB_Files_2_Test_Versions/Insert_Test_6.csv", 5, 0
								  		  ,&malloced_head, the_debug) != 0)
				result = -1;

			if (malloced_head != NULL)
			{
				if (the_debug == YES_DEBUG)
					printf("	ERROR in test_Driver_main() at line %d in %s\n", __LINE__, __FILE__);
				myFreeAllError(&malloced_head, the_debug);
				result = -1;
			}
		// END Test with id = 909
	// END test_Driver_insertRows*/

	
	/*// START test_createTableFromCSV
		// START Test with id = 2000
			char* table_name = (char*) myMalloc(sizeof(char) * 32, NULL, &malloced_head, the_debug);
			if (table_name == NULL)
			{
				return -1;
			}
			strcpy(table_name, "Test_tbl_2");
			int num_rows = 10;

			if (createTableFromCSV("Liquor_Brands_Test.csv", table_name, num_rows, "david", &malloced_head, the_debug) > 0)
			{
				if (the_debug == YES_DEBUG)
					printf("Successfully created table and inserted to disk\n\n\n");


				struct select_node* select_node = NULL;
				parseSelect("select * from Test_tbl_2;", &select_node, NULL, false, &malloced_head, the_debug);


				if (selectStuff(&select_node, false, &malloced_head, the_debug) != 0)
				{
					if (the_debug == YES_DEBUG)
						printf("Data retreival from disk FAILED, please try again\n");
					return -1;
				}
				else
				{
					if (the_debug == YES_DEBUG)
						printf("Successfully retreived %lu rows from disk, creating .csv file\n", select_node->columns_arr[0]->num_rows);
					int displayed = displayResultsOfSelect(select_node, &malloced_head, the_debug);


					// START Free stuff
						int_8 total_freed = 0;
						for (int j=0; j<select_node->columns_arr_size; j++)
						{
							for (int i=0; i<select_node->columns_arr[j]->num_rows; i++)
							{
								myFree((void**) &select_node->columns_arr[j]->col_data_arr[i], NULL, &malloced_head, YES_DEBUG);
								total_freed+=2;
							}
							myFree((void**) &select_node->columns_arr[j]->col_data_arr, NULL, &malloced_head, YES_DEBUG);
							total_freed++;

							if (select_node->columns_arr[j]->unique_values_head != NULL)
							{
								int temp_freed = freeAnyLinkedList((void**) &select_node->columns_arr[j]->unique_values_head, PTR_TYPE_LIST_NODE_PTR, NULL, &malloced_head, the_debug);
								if (the_debug == YES_DEBUG)
									printf("Freed %d from col's unique_values_head\n", temp_freed);
								total_freed += temp_freed;
							}
						}

						while (select_node->prev != NULL)
							select_node = select_node->prev;

						freeAnyLinkedList((void**) &select_node, PTR_TYPE_SELECT_NODE, NULL, &malloced_head, the_debug);
					// END Free stuff
				}
			}
			else
			{
				setOutputRed();
				printf("Creating table from CSV did NOT WORK\n");
				setOutputWhite();
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

			//traverseTablesInfoMemory();

			//traverseTablesInfoDisk(&malloced_head, the_debug);
		// END Test with id = 2000
	// END test_createTableFromCSV*/


	
	/*// START test_Helper_DateFunctions_2
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
		if (test_Performance_Select(1001, "select * from LIQUOR_LICENSES;", &malloced_head, the_debug) != 0)
			result = -1;

		if (malloced_head != NULL)
		{
			if (the_debug == YES_DEBUG)
				printf("	ERROR in test_Driver_main() at line %d in %s\n", __LINE__, __FILE__);
			return -3;
		}
		// END Test with id = 1001

		// START Test with id = 1002
		if (test_Performance_Select(1002, "select * from LIQUOR_LICENSES where BRAND_NAME = 'INCARNADINE 19 VIOGNIER-PINOT GRIGIO PASO ROBLES';", &malloced_head, the_debug) != 0)
			result = -1;

		if (malloced_head != NULL)
		{
			if (the_debug == YES_DEBUG)
				printf("	ERROR in test_Driver_main() at line %d in %s\n", __LINE__, __FILE__);
			return -3;
		}
		// END Test with id = 1002

		// START Test with id = 1003
		if (test_Performance_Select(1003, "select * from LIQUOR_LICENSES where BRAND_NAME = 'INCARNADINE 19 VIOGNIER-PINOT GRIGIO PASO ROBLES' and EFFECTIVE = '3/13/2020';", &malloced_head, the_debug) != 0)
			result = -1;

		if (malloced_head != NULL)
		{
			if (the_debug == YES_DEBUG)
				printf("	ERROR in test_Driver_main() at line %d in %s\n", __LINE__, __FILE__);
			return -3;
		}
		// END Test with id = 1003

		// START Test with id = 1004
		if (test_Performance_Select(1004, "select * from LIQUOR_LICENSES where BRAND_NAME = 'INCARNADINE 19 VIOGNIER-PINOT GRIGIO PASO ROBLES' and EFFECTIVE = '3/13/2020' and CT_REGISTRATION_NUMBER = 165213 and STATUS = 'ACTIVE' and EXPIRATION = '3/12/2023' and OUT_OF_STATE_SHIPPER = 'PENROSE HILL LIMITED' and SUPERVISOR_CREDENTIAL = 'LSL.0001742';"
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
		if (test_Performance_Select(1005, "select * from LIQUOR_LICENSES where BRAND_NAME = 'INCARNADINE 19 VIOGNIER-PINOT GRIGIO PASO ROBLES' or EFFECTIVE = '7/11/2023' or CT_REGISTRATION_NUMBER = 55578 or STATUS = 'ACTIVE' or EXPIRATION = '12/6/2025' or OUT_OF_STATE_SHIPPER = 'ARTISAN WINES INC' or SUPERVISOR_CREDENTIAL = 'LSL.0001471';"
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