#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stddef.h>
#include <inttypes.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include "DB_Tests.h"
#include "DB_Driver.h"
#include "DB_HelperFunctions.h"
#include "DB_Controller.h"


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
}

int test_Driver_teardown(int the_debug)
{
	while (freeMemOfDB(the_debug) != 0)
	{
        printf("\nTeardown FAILED\n\n");
        return -1;
	}
    printf("\nSuccessfully teared down database\n\n");

    system("del DB_Files_2\\*");

    return 0;
}

int selectAndCheckHash(char* test_version, int test_id, struct malloced_node** malloced_head, int the_debug)
{
	int_8* col_numbers = (int_8*) myMalloc(sizeof(int_8) * 7, NULL, malloced_head, the_debug);
	col_numbers[0] = 0;
	col_numbers[1] = 1;
	col_numbers[2] = 2;
	col_numbers[3] = 3;
	col_numbers[4] = 4;
	col_numbers[5] = 5;
	col_numbers[6] = 6;
	int col_numbers_size = 7;

	int_8 num_rows_in_result = 0;

	char*** result = select(getTablesHead(), col_numbers, col_numbers_size, &num_rows_in_result, NULL, malloced_head, the_debug);
	if (result == NULL)
	{
		printf("Data retreival from disk FAILED, please try again\n");
		return -1;
	}
	else
	{
		//printf("Successfully retreived %lu rows from disk, creating .csv file\n", num_rows_in_result);
		int displayed = displayResultsOfSelect(result, getTablesHead(), col_numbers, col_numbers_size, num_rows_in_result, malloced_head, the_debug);

		// START Free stuff
		int_8 total_freed = 0;
		for (int j=0; j<col_numbers_size; j++)
		{
			for (int i=0; i<num_rows_in_result; i++)
			{
				myFree((void**) &result[j][i], NULL, malloced_head, the_debug);
				total_freed++;
			}
			myFree((void**) &result[j], NULL, malloced_head, the_debug);
			total_freed++;
		}
		myFree((void**) &result, NULL, malloced_head, the_debug);
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

		char cmd[100];
		strcpy(cmd, "certutil -hashfile \0");
		strcat(cmd, test_version);

		FILE *fp;
  		char* var1 = (char*) myMalloc(sizeof(char) * 100, NULL, malloced_head, the_debug);
  		char* var2 = (char*) myMalloc(sizeof(char) * 100, NULL, malloced_head, the_debug);
  		char* hash1 = (char*) myMalloc(sizeof(char) * 100, NULL, malloced_head, the_debug);
  		char* hash2 = (char*) myMalloc(sizeof(char) * 100, NULL, malloced_head, the_debug);

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

		myFree((void**) &var1, NULL, malloced_head, the_debug);
		myFree((void**) &var2, NULL, malloced_head, the_debug);
		myFree((void**) &hash1, NULL, malloced_head, the_debug);
		myFree((void**) &hash2, NULL, malloced_head, the_debug);

		if (compared != 0)
		{
			printf("File hashes DID NOT match for test %d\n", test_id);
			return -1;
		}
	}

	return 0;
}


int test_Helper_DateFunctions_1(char* date, struct malloced_node** malloced_head, int the_debug)
{
	printf("Testing date: %s\n", date);

	int_8 data_int_form = dateToInt(date);
	printf("data_int_form = %lu\n", data_int_form);

	char* data_char_form = intToDate(data_int_form, NULL, malloced_head, the_debug);
	printf("data_char_form = %s\n", data_char_form);

	if (strcmp(date, data_char_form) != 0)
	{
		printf("The dates did NOT match\n\n");
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
		printf("The date int %lu did NOT match\n", data_int_form);
		myFree((void**) &data_char_form, NULL, malloced_head, the_debug);
		return -1;
	}

	myFree((void**) &data_char_form, NULL, malloced_head, the_debug);
	return 0;
}


int test_Driver_findValidRowsGivenWhere(int test_id, struct ListNode* expected_results, char* where_string
									   ,struct table_info* the_table, struct colDataNode*** table_data_arr
									   ,int_8* the_col_numbers, int_8* num_rows_in_result, int the_col_numbers_size
									   ,struct malloced_node** malloced_head, int the_debug)
{
	int result = 0;

	struct or_clause_node* or_head = parseWhereClause(where_string, getTablesHead(), malloced_head, the_debug);

	printf("Parsed where_string\n");

	struct ListNode* actual_results = findValidRowsGivenWhere(the_table, table_data_arr, or_head
															 ,the_col_numbers, num_rows_in_result, the_col_numbers_size, malloced_head, the_debug);

	printf("findValidRowsGivenWhere returned %lu rows\n", *num_rows_in_result);

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
						  ,struct table_info* the_table, struct malloced_node** malloced_head, int the_debug)
{
	struct change_node_v2* change_head = NULL;
	struct or_clause_node* or_head = NULL;
	
	if (parseUpdate(input_string, &change_head, &or_head, malloced_head, the_debug) != 0)
	{
		printf("test_Driver_updateRows with id = %d FAILED\n", test_id);
		printf("The test %d had a problem with parseUpdate()\n", test_id);
		return -1;
	}

	int updated = updateRows(the_table, change_head, or_head, malloced_head, the_debug);
	
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

	if (updated < 0)
	{
		printf("test_Driver_updateRows with id = %d FAILED\n", test_id);
		printf("The test %d had a problem with updateRows()\n", test_id);
		return -1;
	}

	return selectAndCheckHash(expected_results_csv, test_id, malloced_head, the_debug);
}

int test_Driver_deleteRows()
{

}

int test_Driver_insertRows()
{

}


int test_Driver_main(int the_debug)
{
	struct malloced_node* malloced_head = NULL;


	/*if (test_Driver_setup(&malloced_head, the_debug) != 0)
		return -1;*/


	system("copy DB_Files_2_Test_Backups\\* DB_Files_2\\");

	int initd = initDB(&malloced_head, the_debug);
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


	if (malloced_head != NULL)
	{
		if (the_debug == YES_DEBUG)
			printf("	ERROR in test_Driver_main() at line %d in %s\n", __LINE__, __FILE__);
		return -3;
	}

	int result = 0;


	printf ("Starting Tests\n");
	// START test_Driver_findValidRowsGivenWhere
		printf("Test 1\n");
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
				return -3;
			}
		// END Test with id = 1
		printf("Test 2\n");
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
				return -3;
			}
		// END Test with id = 2
		printf("Test 3\n");
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
				return -3;
			}
		// END Test with id = 3
		printf("Test 4\n");
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
				return -3;
			}
		// END Test with id = 4
		printf("Test 5\n");
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
				return -3;
			}
		// END Test with id = 5
		printf("Test 6\n");
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
				return -3;
			}
		// END Test with id = 6
		printf("Test 7\n");
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
				return -3;
			}
		// END Test with id = 7
		printf("Test 8\n");
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
				return -3;
			}
		// END Test with id = 8
		printf("Test 9\n");
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
				return -3;
			}
		// END Test with id = 9
		printf("Test 10\n");
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
				return -3;
			}
		// END Test with id = 10
		printf("Test 11\n");
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
				return -3;
			}
		// END Test with id = 11
		printf("Test 12\n");
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
				return -3;
			}
		// END Test with id = 12
		printf("Test 13\n");
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
				return -3;
			}
		// END Test with id = 13
	// END test_Driver_findValidRowsGivenWhere

	/*// START test_Driver_updateRows
		// START Test with id = 14
			if (test_Driver_updateRows(14, "DB_Files_2_Test_Versions\\Update_Test_1.csv"
									  ,"update alc_brands set STATUS = 'TST_ACTIVE' where EXPIRATION = '1/31/2025';"
									  ,getTablesHead(), &malloced_head, the_debug) != 0)
				result = -1;
		// END Test with id = 14

		// START Test with id = 15
			if (test_Driver_updateRows(15, "DB_Files_2_Test_Versions\\Update_Test_2_v2.csv"
									  ,"update alc_brands set STATUS = 'SOMETHING' where EXPIRATION = '1/1/1900';"
							  		  ,getTablesHead(), &malloced_head, the_debug) != 0)
				result = -1;
		// END Test with id = 15

		// START Test with id = 16
			if (test_Driver_updateRows(16, "DB_Files_2_Test_Versions\\Update_Test_3_v2.csv"
									  ,"update alc_brands set STATUS = 'VRY_ACTIVE' where STATUS = 'ACTIVE';"
							  		  ,getTablesHead(), &malloced_head, the_debug) != 0)
				result = -1;
		// END Test with id = 16

		// START Test with id = 17
			if (test_Driver_updateRows(17, "DB_Files_2_Test_Versions\\Update_Test_4.csv"
									  ,"update alc_brands set STATUS = 'VRY_ACTIVE', CT-REGISTRATION-NUMBER = 1 where STATUS = 'ACTIVE';"
							  		  ,getTablesHead(), &malloced_head, the_debug) != 0)
				result = -1;
		// END Test with id = 17
	// END test_Driver_updateRows*/

	if (test_Driver_teardown(the_debug) != 0)
		return -1;

	for (int_8 i = 0; i<50000; i++)
	{
		if (test_Helper_DateFunctions_2(i, &malloced_head, the_debug) != 0)
			return -1;
	}

	return result;
}