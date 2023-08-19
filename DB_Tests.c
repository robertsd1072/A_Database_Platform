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

		if (selectAndCheckHash(the_debug, "C:\\Users\\David\\Desktop\\A_Database_Platform\\DB_Files_2_Test_Versions\\CreatedTable.csv") != 0)
			return -1;
	}

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

int selectAndCheckHash(int the_debug, char* test_version)
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
		printf("Data retreival from disk FAILED, please try again\n\n");
		free(col_numbers);
		return -1;
	}
	else
	{
		printf("Successfully retreived %lu rows from disk, creating .csv file\n", num_rows_in_result);
		if (displayResultsOfSelectAndFree(result, getTablesHead(), col_numbers, col_numbers_size, num_rows_in_result, NULL, the_debug) != 0)
		{
			printf("Creation of .csv file had a problem with file i/o, please try again.\n\n");
			free(col_numbers);
			return -1;
		}
		else 
			printf("Successfully created C:\\Users\\David\\Downloads\\Results.csv\n\n");

		free(col_numbers);

		char* cmd = (char*) malloc(sizeof(char) * 100);
		strcpy(cmd, "certutil -hashfile ");
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
		free(cmd);

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
			printf("File hashes DID NOT match\n");
			return -1;
		}
		else
			printf("File hashes look good\n");
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


int test_Driver_updateRows()
{

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


	//Some tests


	if (test_Driver_teardown(the_debug) != 0)
		return -1;

	for (int_8 i = 0; i<50000; i++)
	{
		if (test_Helper_DateFunctions_2(the_debug, i) != 0)
			return -1;
	}

	return 0;
}