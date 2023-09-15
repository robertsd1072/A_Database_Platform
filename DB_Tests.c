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


int test_Driver_setup(int the_debug)
{
	int initd = initDB(the_debug);
	if (initd == -1)
		printf("Database initialization had a problem with file i/o, please try again\n\n");
	else if (initd == -2)
		printf("Database initialization had a problem with malloc, please try again\n\n");
	else
		printf("Successfully initialized database\n\n");

	char* table_name = (char*) malloc(sizeof(char) * 32);
	strcpy(table_name, "alc_brands");
	int_8 returnd = createTableFromCSV("C:\\Users\\David\\Desktop\\A_Database_Platform\\Liquor_Brands.csv", table_name, 1000, the_debug);
	if (returnd < 0)
	{
		printf("Table creation from CSV had a problem");
		return -1;
	}
	else
	{
		printf("Table creation from CSV has successfully:\n");
		printf("	Created the table\n");
		printf("	Inserted %lu rows\n\n", returnd);

		if (selectAndCheckHash(the_debug, "C:\\Users\\David\\Desktop\\A_Database_Platform\\DB_Files_2_Test_Versions\\CreatedTable.csv", -1) != 0)
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

    system("del C:\\Users\\David\\Desktop\\A_Database_Platform\\DB_Files_2\\*");

    return 0;
}

int selectAndCheckHash(int the_debug, char* test_version, int test_id)
{
	int_8* col_numbers = (int_8*) malloc(sizeof(int_8) * 7);
	col_numbers[0] = 0;
	col_numbers[1] = 1;
	col_numbers[2] = 2;
	col_numbers[3] = 3;
	col_numbers[4] = 4;
	col_numbers[5] = 5;
	col_numbers[6] = 6;
	int col_numbers_size = 7;

	int_8 num_rows_in_result = 0;

	char*** result = select(getTablesHead(), col_numbers, col_numbers_size, &num_rows_in_result, NULL, the_debug);
	if (result == NULL)
	{
		printf("Data retreival from disk FAILED, please try again\n");
		free(col_numbers);
		return -1;
	}
	else
	{
		//printf("Successfully retreived %lu rows from disk, creating .csv file\n", num_rows_in_result);
		if (displayResultsOfSelectAndFree(result, getTablesHead(), col_numbers, col_numbers_size, num_rows_in_result, NULL, the_debug) != 0)
		{
			printf("Creation of .csv file had a problem with file i/o, please try again.\n");
			free(col_numbers);
			return -1;
		}
		//else 
		//	printf("Successfully created C:\\Users\\David\\Downloads\\Results.csv\n");

		free(col_numbers);

		char cmd[100];
		strcpy(cmd, "certutil -hashfile \0");
		strcat(cmd, test_version);

		FILE *fp;
  		char var1[100];
  		char var2[100];

		fp = popen(cmd, "r");
		if (fp == NULL) 
		{
			printf("Failed to run command\n");
			return -1;
		}
		while (fgets(var1, sizeof(var1), fp) != NULL) 
		{
			//printf("%s\n", var1);
		}

		pclose(fp);

		fp = popen("certutil -hashfile C:\\Users\\David\\Downloads\\Results.csv", "r");
		if (fp == NULL) 
		{
			printf("Failed to run command\n");
			return -1;
		}
		while (fgets(var2, sizeof(var2), fp) != NULL)
		{
			//printf("%s\n", var2);
		}

		pclose(fp);

		//system(cmd);
		//system("certutil -hashfile C:\\Users\\David\\Downloads\\Results.csv");
		if (strcmp(var1, var2) != 0)
		{
			printf("File hashes DID NOT match for test %d\n", test_id);
			return -1;
		}
	}

	return 0;
}


int test_Helper_DateFunctions_1(int the_debug, char* date)
{
	printf("Testing date: %s\n", date);
	struct malloced_node* malloced_head = NULL;

	int_8 data_int_form = dateToInt(date);
	printf("data_int_form = %lu\n", data_int_form);

	char* data_char_form = intToDate(&malloced_head, data_int_form, the_debug);
	printf("data_char_form = %s\n", data_char_form);

	if (strcmp(date, data_char_form) != 0)
	{
		printf("The dates did NOT match\n\n");
		myFree(&malloced_head, (void**) &data_char_form, the_debug);
		return -1;
	}
	
	printf("\n");
	myFree(&malloced_head, (void**) &data_char_form, the_debug);
	return 0;
}

int test_Helper_DateFunctions_2(int the_debug, int_8 date)
{
	struct malloced_node* malloced_head = NULL;

	char* data_char_form = intToDate(&malloced_head, date, the_debug);
	int_8 data_int_form = dateToInt(data_char_form);

	if (data_int_form != date)
	{
		printf("The date int %lu did NOT match\n", data_int_form);
		myFree(&malloced_head, (void**) &data_char_form, the_debug);
		return -1;
	}

	myFree(&malloced_head, (void**) &data_char_form, the_debug);
	return 0;
}


int test_Driver_findValidRowsGivenWhere(int test_id, int the_debug, struct ListNode* expected_results, char* where_string
									   ,struct table_info* the_table, struct colDataNode*** table_data_arr
									   ,int_8* the_col_numbers, int_8* num_rows_in_result, int the_col_numbers_size)
{
	int result = 0;

	struct or_clause_node* or_head = parseWhereClause(the_debug, where_string, getTablesHead());

	struct ListNode* actual_results = findValidRowsGivenWhere(the_table, table_data_arr, or_head
															 ,the_col_numbers, num_rows_in_result, the_col_numbers_size, the_debug);

	//printf("findValidRowsGivenWhere returned %lu rows\n", *num_rows_in_result);

	while (or_head != NULL)
	{
		while (or_head->and_head != NULL)
		{
			struct and_clause_node* temp = or_head->and_head;
			or_head->and_head = or_head->and_head->next;
			free(temp->data_string);
			free(temp);
		}
		struct or_clause_node* temp = or_head;
		or_head = or_head->next;
		free(temp);
	}

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

	freeListNodes(NULL, &expected_results, the_debug);
	freeListNodes(NULL, &actual_results, the_debug);

	if (the_col_numbers != NULL)
		free(the_col_numbers);

	while (or_head != NULL)
	{
		while (or_head->and_head != NULL)
		{
			struct and_clause_node* temp = or_head->and_head;
			or_head->and_head = or_head->and_head->next;
			free(temp->data_string);
			free(temp);
		}

		struct or_clause_node* temp = or_head;
		or_head = or_head->next;
		free(temp);
	} 

	return result;
}

int test_Driver_updateRows(int test_id, int the_debug, char* expected_results_csv, char* input_string
						  ,struct table_info* the_table)
{
	struct change_node_v2* change_head = NULL;
	struct or_clause_node* or_head = NULL;
	
	if (parseUpdate(the_debug, input_string, &change_head, &or_head) != 0)
	{
		printf("test_Driver_updateRows with id = %d FAILED\n", test_id);
		printf("The test %d had a problem with parseUpdate()\n", test_id);
		return -1;
	}

	int updated = updateRows(the_table, change_head, or_head, the_debug);
	
	while (change_head != NULL)
	{
		struct change_node_v2* temp = change_head;
		change_head = change_head->next;
		free(temp->data);
		free(temp);
	}

	while (or_head != NULL)
	{
		while (or_head->and_head != NULL)
		{
			struct and_clause_node* temp = or_head->and_head;
			or_head->and_head = or_head->and_head->next;
			free(temp->data_string);
			free(temp);
		}
		struct or_clause_node* temp = or_head;
		or_head = or_head->next;
		free(temp);
	}

	if (updated < 0)
	{
		printf("test_Driver_updateRows with id = %d FAILED\n", test_id);
		printf("The test %d had a problem with updateRows()\n", test_id);
		return -1;
	}

	return selectAndCheckHash(the_debug, expected_results_csv, test_id);
}

int test_Driver_deleteRows()
{

}

int test_Driver_insertRows()
{

}


int test_Driver_main(int the_debug)
{
	if (test_Driver_setup(the_debug) != 0)
		return -1;

	int result = 0;


	printf ("Starting Tests\n");
	// START test_Driver_findValidRowsGivenWhere
		struct malloced_node* malloced_head = NULL;

		// START Test with id = 1
			struct ListNode* valid_rows_head = NULL;
			struct ListNode* valid_rows_tail = NULL;

			for (int i=0; i<getTablesHead()->table_cols_head->num_rows; i++)
			{
				if (addListNode(&malloced_head, &valid_rows_head, &valid_rows_tail, i, NOT_PERSISTS, ADDLISTNODE_TAIL) != 0)
				{
					if (the_debug == YES_DEBUG)
						printf("	ERROR in test_Driver_main() at line %d in %s\n", __LINE__, __FILE__);
					myFreeAllError(&malloced_head, the_debug);
					return -2;
				}
			}

			int_8 num_rows_in_result = 0;
			if (test_Driver_findValidRowsGivenWhere(1, the_debug, valid_rows_head, ""
												   ,getTablesHead(), NULL
												   ,NULL, &num_rows_in_result, 0) != 0)
				result = -1;
		// END Test with id = 1
		
		// START Test with id = 2
			valid_rows_head = NULL;
			valid_rows_tail = NULL;

			if (addListNode(&malloced_head, &valid_rows_head, &valid_rows_tail, 12, NOT_PERSISTS, ADDLISTNODE_TAIL) != 0)
			{
				if (the_debug == YES_DEBUG)
					printf("	ERROR in test_Driver_main() at line %d in %s\n", __LINE__, __FILE__);
				myFreeAllError(&malloced_head, the_debug);
				return -2;
			}

			struct or_clause_node* or_head = (struct or_clause_node*) myMalloc(&malloced_head, sizeof(struct or_clause_node), NOT_PERSISTS);
			or_head->next = NULL;

			or_head->and_head = (struct and_clause_node*) myMalloc(&malloced_head, sizeof(struct and_clause_node), NOT_PERSISTS);
			or_head->and_head->next = NULL;
			or_head->and_head->col_number = 0;
			or_head->and_head->where_type = WHERE_IS_EQUALS;

			or_head->and_head->data_string = (char*) myMalloc(&malloced_head, sizeof(char) * 32, NOT_PERSISTS);
			strcpy(or_head->and_head->data_string, "VIZZY BLACK CHERRY LIME");

			num_rows_in_result = 0;
			if (test_Driver_findValidRowsGivenWhere(2, the_debug, valid_rows_head, "where BRAND-NAME = 'VIZZY BLACK CHERRY LIME';"
												   ,getTablesHead(), NULL
												   ,NULL, &num_rows_in_result, 0) != 0)
				result = -1;
		// END Test with id = 2
		
		// START Test with id = 3
			valid_rows_head = NULL;
			valid_rows_tail = NULL;

			if (addListNode(&malloced_head, &valid_rows_head, &valid_rows_tail, 6, NOT_PERSISTS, ADDLISTNODE_TAIL) != 0)
			{
				if (the_debug == YES_DEBUG)
					printf("	ERROR in test_Driver_main() at line %d in %s\n", __LINE__, __FILE__);
				myFreeAllError(&malloced_head, the_debug);
				return -2;
			}

			num_rows_in_result = 0;
			if (test_Driver_findValidRowsGivenWhere(3, the_debug, valid_rows_head, "where CT-REGISTRATION-NUMBER = 152525;"
												   ,getTablesHead(), NULL
												   ,NULL, &num_rows_in_result, 0) != 0)
				result = -1;
		// END Test with id = 3
		
		// START Test with id = 4
			valid_rows_head = NULL;
			valid_rows_tail = NULL;

			addListNode(&malloced_head, &valid_rows_head, &valid_rows_tail, 5, NOT_PERSISTS, ADDLISTNODE_TAIL);
			addListNode(&malloced_head, &valid_rows_head, &valid_rows_tail, 56, NOT_PERSISTS, ADDLISTNODE_TAIL);
			addListNode(&malloced_head, &valid_rows_head, &valid_rows_tail, 58, NOT_PERSISTS, ADDLISTNODE_TAIL);
			addListNode(&malloced_head, &valid_rows_head, &valid_rows_tail, 343, NOT_PERSISTS, ADDLISTNODE_TAIL);
			addListNode(&malloced_head, &valid_rows_head, &valid_rows_tail, 521, NOT_PERSISTS, ADDLISTNODE_TAIL);
			addListNode(&malloced_head, &valid_rows_head, &valid_rows_tail, 579, NOT_PERSISTS, ADDLISTNODE_TAIL);
			addListNode(&malloced_head, &valid_rows_head, &valid_rows_tail, 805, NOT_PERSISTS, ADDLISTNODE_TAIL);
			addListNode(&malloced_head, &valid_rows_head, &valid_rows_tail, 945, NOT_PERSISTS, ADDLISTNODE_TAIL);

			num_rows_in_result = 0;
			if (test_Driver_findValidRowsGivenWhere(4, the_debug, valid_rows_head, "where EXPIRATION = '1/31/2025';"
												   ,getTablesHead(), NULL
												   ,NULL, &num_rows_in_result, 0) != 0)
				result = -1;
		// END Test with id = 4
		
		// START Test with id = 5
			valid_rows_head = NULL;
			valid_rows_tail = NULL;

			if (addListNode(&malloced_head, &valid_rows_head, &valid_rows_tail, 12, NOT_PERSISTS, ADDLISTNODE_TAIL) != 0)
			{
				if (the_debug == YES_DEBUG)
					printf("	ERROR in test_Driver_main() at line %d in %s\n", __LINE__, __FILE__);
				myFreeAllError(&malloced_head, the_debug);
				return -2;
			}

			num_rows_in_result = 0;
			if (test_Driver_findValidRowsGivenWhere(5, the_debug, valid_rows_head, "where BRAND-NAME = 'VIZZY BLACK CHERRY LIME' and OUT-OF-STATE-SHIPPER = 'MOLSON COORS BEVERAGE COMPANY USA LLC';"
												   ,getTablesHead(), NULL
												   ,NULL, &num_rows_in_result, 0) != 0)
				result = -1;
		// END Test with id = 5
		
		// START Test with id = 6
			valid_rows_head = NULL;
			valid_rows_tail = NULL;

			addListNode(&malloced_head, &valid_rows_head, &valid_rows_tail, 12, NOT_PERSISTS, ADDLISTNODE_TAIL);
			addListNode(&malloced_head, &valid_rows_head, &valid_rows_tail, 23, NOT_PERSISTS, ADDLISTNODE_TAIL);

			num_rows_in_result = 0;
			if (test_Driver_findValidRowsGivenWhere(6, the_debug, valid_rows_head, "where BRAND-NAME = 'VIZZY BLACK CHERRY LIME' or BRAND-NAME = 'CRUZAN ISLAND SPICED RUM';"
												   ,getTablesHead(), NULL
												   ,NULL, &num_rows_in_result, 0) != 0)
				result = -1;
		// END Test with id = 6

		// START Test with id = 7
			valid_rows_head = NULL;
			valid_rows_tail = NULL;

			addListNode(&malloced_head, &valid_rows_head, &valid_rows_tail, 5, NOT_PERSISTS, ADDLISTNODE_TAIL);
			addListNode(&malloced_head, &valid_rows_head, &valid_rows_tail, 12, NOT_PERSISTS, ADDLISTNODE_TAIL);
			addListNode(&malloced_head, &valid_rows_head, &valid_rows_tail, 23, NOT_PERSISTS, ADDLISTNODE_TAIL);
			addListNode(&malloced_head, &valid_rows_head, &valid_rows_tail, 56, NOT_PERSISTS, ADDLISTNODE_TAIL);
			addListNode(&malloced_head, &valid_rows_head, &valid_rows_tail, 58, NOT_PERSISTS, ADDLISTNODE_TAIL);
			addListNode(&malloced_head, &valid_rows_head, &valid_rows_tail, 343, NOT_PERSISTS, ADDLISTNODE_TAIL);
			addListNode(&malloced_head, &valid_rows_head, &valid_rows_tail, 521, NOT_PERSISTS, ADDLISTNODE_TAIL);
			addListNode(&malloced_head, &valid_rows_head, &valid_rows_tail, 579, NOT_PERSISTS, ADDLISTNODE_TAIL);
			addListNode(&malloced_head, &valid_rows_head, &valid_rows_tail, 805, NOT_PERSISTS, ADDLISTNODE_TAIL);
			addListNode(&malloced_head, &valid_rows_head, &valid_rows_tail, 945, NOT_PERSISTS, ADDLISTNODE_TAIL);

			num_rows_in_result = 0;
			if (test_Driver_findValidRowsGivenWhere(7, the_debug, valid_rows_head, "where BRAND-NAME = 'VIZZY BLACK CHERRY LIME' or BRAND-NAME = 'CRUZAN ISLAND SPICED RUM' and CT-REGISTRATION-NUMBER = 169876 or EXPIRATION = '1/31/2025';"
												   ,getTablesHead(), NULL
												   ,NULL, &num_rows_in_result, 0) != 0)
				result = -1;
		// END Test with id = 7

		// START Test with id = 8
			valid_rows_head = NULL;
			valid_rows_tail = NULL;

			addListNode(&malloced_head, &valid_rows_head, &valid_rows_tail, 5, NOT_PERSISTS, ADDLISTNODE_TAIL);
			addListNode(&malloced_head, &valid_rows_head, &valid_rows_tail, 23, NOT_PERSISTS, ADDLISTNODE_TAIL);

			num_rows_in_result = 0;
			if (test_Driver_findValidRowsGivenWhere(8, the_debug, valid_rows_head, "where BRAND-NAME = 'CRUZAN ISLAND SPICED RUM' and CT-REGISTRATION-NUMBER = 169876 or EXPIRATION = '1/31/2025' and BRAND-NAME = 'BABAROSA MOSCATO D''ASTI';"
												   ,getTablesHead(), NULL
												   ,NULL, &num_rows_in_result, 0) != 0)
				result = -1;
		// END Test with id = 8

		// START Test with id = 9
			valid_rows_head = NULL;
			valid_rows_tail = NULL;

			for (int i=0; i<getTablesHead()->table_cols_head->num_rows; i++)
			{
				if (i != 23)
				{
					if (addListNode(&malloced_head, &valid_rows_head, &valid_rows_tail, i, NOT_PERSISTS, ADDLISTNODE_TAIL) != 0)
					{
						if (the_debug == YES_DEBUG)
							printf("	ERROR in test_Driver_main() at line %d in %s\n", __LINE__, __FILE__);
						myFreeAllError(&malloced_head, the_debug);
						return -2;
					}
				}
			}

			num_rows_in_result = 0;
			if (test_Driver_findValidRowsGivenWhere(9, the_debug, valid_rows_head, "where BRAND-NAME <> 'CRUZAN ISLAND SPICED RUM';"
												   ,getTablesHead(), NULL
												   ,NULL, &num_rows_in_result, 0) != 0)
				result = -1;
		// END Test with id = 9

		// START Test with id = 10
			valid_rows_head = NULL;
			valid_rows_tail = NULL;

			for (int i=0; i<getTablesHead()->table_cols_head->num_rows; i++)
			{
				if (addListNode(&malloced_head, &valid_rows_head, &valid_rows_tail, i, NOT_PERSISTS, ADDLISTNODE_TAIL) != 0)
				{
					if (the_debug == YES_DEBUG)
						printf("	ERROR in test_Driver_main() at line %d in %s\n", __LINE__, __FILE__);
					myFreeAllError(&malloced_head, the_debug);
					return -2;
				}
			}

			num_rows_in_result = 0;
			if (test_Driver_findValidRowsGivenWhere(10, the_debug, valid_rows_head, "where BRAND-NAME <> 'CRUZAN ISLAND SPICED RUM' or BRAND-NAME <> 'TEST 1';"
												   ,getTablesHead(), NULL
												   ,NULL, &num_rows_in_result, 0) != 0)
				result = -1;
		// END Test with id = 10

		// START Test with id = 11
			valid_rows_head = NULL;
			valid_rows_tail = NULL;

			for (int i=0; i<getTablesHead()->table_cols_head->num_rows; i++)
			{
				if (i != 23 && i != 12)
				{
					if (addListNode(&malloced_head, &valid_rows_head, &valid_rows_tail, i, NOT_PERSISTS, ADDLISTNODE_TAIL) != 0)
					{
						if (the_debug == YES_DEBUG)
							printf("	ERROR in test_Driver_main() at line %d in %s\n", __LINE__, __FILE__);
						myFreeAllError(&malloced_head, the_debug);
						return -2;
					}
				}
			}

			num_rows_in_result = 0;
			if (test_Driver_findValidRowsGivenWhere(11, the_debug, valid_rows_head, "where BRAND-NAME <> 'CRUZAN ISLAND SPICED RUM' and BRAND-NAME <> 'VIZZY BLACK CHERRY LIME';"
												   ,getTablesHead(), NULL
												   ,NULL, &num_rows_in_result, 0) != 0)
				result = -1;
		// END Test with id = 11

		// START Test with id = 12
			valid_rows_head = NULL;
			valid_rows_tail = NULL;

			for (int i=0; i<getTablesHead()->table_cols_head->num_rows; i++)
			{
				if (addListNode(&malloced_head, &valid_rows_head, &valid_rows_tail, i, NOT_PERSISTS, ADDLISTNODE_TAIL) != 0)
				{
					if (the_debug == YES_DEBUG)
						printf("	ERROR in test_Driver_main() at line %d in %s\n", __LINE__, __FILE__);
					myFreeAllError(&malloced_head, the_debug);
					return -2;
				}
			}

			num_rows_in_result = 0;
			if (test_Driver_findValidRowsGivenWhere(12, the_debug, valid_rows_head, "where BRAND-NAME <> 'CRUZAN ISLAND SPICED RUM' and CT-REGISTRATION-NUMBER <> 169876 or EXPIRATION <> '1/31/2025' and BRAND-NAME <> 'BABAROSA MOSCATO D''ASTI';"
												   ,getTablesHead(), NULL
												   ,NULL, &num_rows_in_result, 0) != 0)
				result = -1;
		// END Test with id = 12

		// START Test with id = 13
			valid_rows_head = NULL;
			valid_rows_tail = NULL;

			num_rows_in_result = 0;
			if (test_Driver_findValidRowsGivenWhere(13, the_debug, valid_rows_head, "where BRAND-NAME = 'Is test bro';"
												   ,getTablesHead(), NULL
												   ,NULL, &num_rows_in_result, 0) != 0)
				result = -1;
		// END Test with id = 13
	// END test_Driver_findValidRowsGivenWhere

	// START test_Driver_updateRows
		// START Test with id = 14
			//"update alc_brands set STATUS = '_TEST_ACTIVE' where EXPIRATION = '1/31/2025';" 
			if (test_Driver_updateRows(14, the_debug
									  ,"C:\\Users\\David\\Desktop\\A_Database_Platform\\DB_Files_2_Test_Versions\\Update_Test_1.csv"
									  ,"update alc_brands set STATUS = 'TST_ACTIVE' where EXPIRATION = '1/31/2025';"
							  		  ,getTablesHead()) != 0)
				result = -1;
		// END Test with id = 14

		// START Test with id = 15
			if (test_Driver_updateRows(15, the_debug
									  ,"C:\\Users\\David\\Desktop\\A_Database_Platform\\DB_Files_2_Test_Versions\\Update_Test_2.csv"
									  ,"update alc_brands set STATUS = 'SOMETHING' where EXPIRATION = '1/1/1900';"
							  		  ,getTablesHead()) != 0)
				result = -1;
		// END Test with id = 15

		// START Test with id = 16
			if (test_Driver_updateRows(16, the_debug
									  ,"C:\\Users\\David\\Desktop\\A_Database_Platform\\DB_Files_2_Test_Versions\\Update_Test_3.csv"
									  ,"update alc_brands set STATUS = 'VRY_ACTIVE' where STATUS = 'ACTIVE';"
							  		  ,getTablesHead()) != 0)
				result = -1;
		// END Test with id = 16

		// START Test with id = 17
			if (test_Driver_updateRows(17, the_debug
									  ,"C:\\Users\\David\\Desktop\\A_Database_Platform\\DB_Files_2_Test_Versions\\Update_Test_4.csv"
									  ,"update alc_brands set STATUS = 'VRY_ACTIVE', CT-REGISTRATION-NUMBER = 1 where STATUS = 'ACTIVE';"
							  		  ,getTablesHead()) != 0)
				result = -1;
		// END Test with id = 17
	// END test_Driver_updateRows

	if (test_Driver_teardown(the_debug) != 0)
		return -1;

	for (int_8 i = 0; i<50000; i++)
	{
		if (test_Helper_DateFunctions_2(the_debug, i) != 0)
			return -1;
	}

	return result;
}