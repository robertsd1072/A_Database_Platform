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
#include "DB_HelperFunctions.h"

typedef unsigned long long int_8;

void* myMalloc(struct malloced_node** malloced_head, size_t size, int the_persists)
{
	void* new_ptr = malloc(size);
	if (new_ptr == NULL)
		return NULL;

	if (*malloced_head == NULL)
	{
		*malloced_head = (struct malloced_node*) malloc(sizeof(struct malloced_node));
        if (*malloced_head == NULL)
		{
			free(new_ptr);
			return NULL;
		}
        (*malloced_head)->ptr = new_ptr;
		(*malloced_head)->persists = the_persists;
		(*malloced_head)->next = NULL;
	}
	else
	{
		struct malloced_node* temp = (struct malloced_node*) malloc(sizeof(struct malloced_node));
		if (temp == NULL)
		{
			free(new_ptr);
			return NULL;
		}
		temp->ptr = new_ptr;
		temp->persists = the_persists;
		temp->next = *malloced_head;
		*malloced_head = temp;
	}

	return new_ptr;
}

int myFree(struct malloced_node** malloced_head, void** old_ptr, int the_debug)
{
	if ((*malloced_head)->ptr == *old_ptr)
	{
		struct malloced_node* temp = *malloced_head;
		*malloced_head = (*malloced_head)->next;
		free(temp);
	}
	else
	{
		struct malloced_node* cur = *malloced_head;
		while (cur->next != NULL)
		{
			if (cur->next->ptr == *old_ptr)
			{
				struct malloced_node* temp = cur->next;
				cur->next = cur->next->next;
				free(temp);
                break;
			}
			cur = cur->next;
		}
	}

	if (*old_ptr == NULL)
	{
		if (the_debug == 1)
			printf("	ERROR in myFree() at line %d in %s\n", __LINE__, __FILE__);
		return -1;
	}
	else
	{
		free(*old_ptr);
		*old_ptr = NULL;
	}

	return 0;
}

int myFreeAllError(struct malloced_node** malloced_head, int the_debug)
{
    int total_freed = 0;
    while (*malloced_head != NULL)
    {
        struct malloced_node* temp = *malloced_head;
        *malloced_head = (*malloced_head)->next;
        if (temp->ptr != NULL)
			free(temp->ptr);
        free(temp);
        total_freed++;
    }
    if (the_debug == 1)
    	printf("myFreeAllError() freed %d malloced_nodes and ptrs\n", total_freed);
    return total_freed;
}

int myFreeAllCleanup(struct malloced_node** malloced_head, int the_debug)
{
	int total_freed = 0;
	int total_not_persists = 0;
    while (*malloced_head != NULL)
    {
        struct malloced_node* temp = *malloced_head;
        *malloced_head = (*malloced_head)->next;
        if (temp->persists == 0 && temp->ptr != NULL)
		{
			free(temp->ptr);
			total_not_persists++;
		}
        free(temp);
        total_freed++;
    }
    if (the_debug == 1)
    {
    	printf("myFreeAllCleanup() freed %d malloced_nodes\n", total_freed);
		printf("myFreeAllCleanup() freed %d ptrs which dont persist\n", total_not_persists);
	}
    return total_freed;
}

void concatFileName(char* new_filename, char* filetype, int_8 table_num, int_8 col_num)
{
	strcpy(new_filename, ".\\DB_Files_2\\DB");

	strcat(new_filename, filetype);

	if (strcmp("_Info", filetype) != 0)
    {
        char new_filename_number[10];

        sprintf(new_filename_number, "%lu", table_num);
        strcat(new_filename, new_filename_number);
    }

	if (strcmp("_Col_Data_", filetype) == 0 || strcmp("_Col_Data_Info_", filetype) == 0 || strcmp("_Col_Data_Info_Temp_", filetype) == 0)
	{
		strcat(new_filename, "_");

		char new_filename_col_number[10];

		sprintf(new_filename_col_number, "%lu", col_num);
		strcat(new_filename, new_filename_col_number);
	}

	strcat(new_filename, ".bin");
}

FILE* myFileOpenSimple(struct file_opened_node** file_opened_head, char* file_name, char* mode)
{
	FILE* new_file = fopen(file_name, mode);

	if (new_file != NULL)
    {
        if (*file_opened_head == NULL)
        {
            *file_opened_head = (struct file_opened_node*) malloc(sizeof(struct file_opened_node));
            if (*file_opened_head == NULL)
            {
                fclose(new_file);
                return NULL;
            }
            (*file_opened_head)->file = new_file;
            (*file_opened_head)->next = NULL;
        }
        else
        {
            struct file_opened_node* temp = (struct file_opened_node*) malloc(sizeof(struct file_opened_node));
            if (temp == NULL)
            {
                fclose(new_file);
                return NULL;
            }
            temp->file = new_file;
            temp->next = *file_opened_head;
            *file_opened_head = temp;
        }
    }

	return new_file;
}

FILE* myFileOpen(struct file_opened_node** file_opened_head, char* filetype, int_8 num_table, int_8 num_col, char* mode, int the_debug)
{
	char* file_name = (char*) malloc(sizeof(char) * 64);
	if (file_name == NULL)
	{
		if (the_debug == 1)
			printf("	ERROR in myFileOpen() at line %d in %s\n", __LINE__, __FILE__);
		return NULL;
	}
	concatFileName(file_name, filetype, num_table, num_col);

	//printf("Trying to open %s with mode %s\n", file_name, mode);
	//printf("access(file_name, F_OK) = %d\n", access(file_name, F_OK));
	if (access(file_name, F_OK) == 0)
	{
		if (access(file_name, R_OK) != 0)
		{
			free(file_name);
			if (the_debug == 1)
				printf("	ERROR File cannot be read in myFileOpen() at line %d in %s\n", __LINE__, __FILE__);
			return NULL;
		}
		if (access(file_name, W_OK) != 0)
		{
			free(file_name);
			if (the_debug == 1)
				printf("	ERROR File cannot be written to in myFileOpen() at line %d in %s\n", __LINE__, __FILE__);
			return NULL;
		}
	}


    FILE* new_file = fopen(file_name, mode);
	free(file_name);


    if (new_file != NULL)
    {
        if (*file_opened_head == NULL)
        {
            *file_opened_head = (struct file_opened_node*) malloc(sizeof(struct file_opened_node));
            if (*file_opened_head == NULL)
            {
                fclose(new_file);
                return NULL;
            }
            (*file_opened_head)->file = new_file;
            (*file_opened_head)->next = NULL;
        }
        else
        {
            struct file_opened_node* temp = (struct file_opened_node*) malloc(sizeof(struct file_opened_node));
            if (temp == NULL)
            {
                fclose(new_file);
                return NULL;
            }
            temp->file = new_file;
            temp->next = *file_opened_head;
            *file_opened_head = temp;
        }
    }

	return new_file;
}

int myFileClose(struct file_opened_node** file_opened_head, FILE* old_file)
{
    if ((*file_opened_head)->file == old_file)
	{
		struct file_opened_node* temp = *file_opened_head;
		*file_opened_head = (*file_opened_head)->next;
		free(temp);
	}
	else
	{
		struct file_opened_node* cur = *file_opened_head;
		while (cur->next != NULL)
		{
			if (cur->next->file == old_file)
			{
				struct file_opened_node* temp = cur->next;
				cur->next = cur->next->next;
				free(temp);
                break;
			}
			cur = cur->next;
		}
	}

	fclose(old_file);
    
    return 0;
}

int myFileCloseAll(struct file_opened_node** file_opened_head, int the_debug)
{
    int total_closed = 0;
    while (*file_opened_head != NULL)
    {
        struct file_opened_node* temp = *file_opened_head;
        *file_opened_head = (*file_opened_head)->next;
        fclose(temp->file);
        free(temp);
        total_closed++;
    }
    if (the_debug == 1)
    	printf("myFileCloseAll() closed %d files\n", total_closed);
    return total_closed;
}


char* intToDate(struct malloced_node** malloced_head, int_8 the_int_form, int the_debug)
{
	int year = 1900;
	int month = 1;
	int day = 1;

	int remaining = the_int_form;

	// START Find the right year
	// 1900 is not a leap year so only at 1460 days for 1900 - 1903
	if (remaining > 1460)
	{
		year += 4;
		remaining -= 1460;
	}

	while (remaining > 1461)
	{
		year += 4;
		remaining -= 1461;
	}

	if (remaining > 366)
	{
		year += 1;
		remaining -= 366;

		while (remaining > 365)
		{
			year += 1;
			remaining -= 365;
		}
	}
	else if (remaining == 366)
	{
		year += 1;
		remaining -= 366;
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

	char* date = (char*) myMalloc(malloced_head, sizeof(char) * 20, 1);
	if (date == NULL)
	{
		if (the_debug)
			printf("	ERROR in intToDate() at line %d in %s\n", __LINE__, __FILE__);
		return NULL;
	}

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
		remaining += (year % 4 == 0 && year != 1900 ? 29 : 28);
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

	while (year > 1904)
	{
		remaining += 1461;
		year -= 4;
	}

	if (year == 1904)
	{
		remaining += 1460;
		year -= 4;
	}

	return remaining;
}

char* readFileChar(struct malloced_node** malloced_head, FILE* file, int_8 offset, int the_debug)
{
	int num_bytes = 32;

	char* raw_bytes = (char*) myMalloc(malloced_head, sizeof(char) * num_bytes, 1);
	if (raw_bytes == NULL)
	{
		if (the_debug == 1)
			printf("	ERROR in readFileChar() at line %d in %s\n", __LINE__, __FILE__);
		return NULL;
	}

	fseek(file, offset, SEEK_SET);

	if (fread(raw_bytes, num_bytes, 1, file) == 0)
	{
		if (the_debug == 1)
			printf("	ERROR in readFileChar() at line %d in %s\n", __LINE__, __FILE__);
		myFree(malloced_head, (void**) &raw_bytes, the_debug);
		return NULL;
	}

	return raw_bytes;
}

char* readFileCharData(struct malloced_node** malloced_head, FILE* file, int_8 offset, int_8 num_bytes, int the_debug)
{
	char* raw_bytes = (char*) myMalloc(malloced_head, sizeof(char) * num_bytes, 1);
	if (raw_bytes == NULL)
	{
		if (the_debug == 1)
			printf("	ERROR in readFileCharData() at line %d in %s\n", __LINE__, __FILE__);
		return NULL;
	}

	fseek(file, offset, SEEK_SET);

	if (fread(raw_bytes, num_bytes, 1, file) == 0)
	{
		if (the_debug)
			printf("	ERROR in readFileCharData() at line %d in %s\n", __LINE__, __FILE__);
		myFree(malloced_head, (void**) &raw_bytes, the_debug);
		return NULL;
	}

	return raw_bytes;
}

int_8 readFileInt(FILE* file, int_8 offset)
{
	int num_bytes = 8;

	int_8 raw_int;

	fseek(file, offset, SEEK_SET);

	if (fread(&raw_int, num_bytes, 1, file) == 0)
		return -1;

	return raw_int;
}

double readFileDouble(FILE* file, int_8 offset)
{
	int num_bytes = 8;

	double raw_double;

	fseek(file, offset, SEEK_SET);

	if (fread(&raw_double, num_bytes, 1, file) == 0)
		return -1.0;

	return raw_double;
}

int writeFileChar(FILE* file, int_8 offset, char* data)
{
	int num_bytes = 32;

	if (offset != -1)
		fseek(file, offset, SEEK_SET);

	//int fd = fdnum(file);
	//int ioctl(fd, FIONBIO);
	
	if (fwrite(data, num_bytes, 1, file) != 1)
		return -1;

	//printf("If there was an error _%s_\n", strerror(errno));

	fflush(file);

	return 0;
}

int writeFileCharData(FILE* file, int_8 offset, int_8 num_bytes, char* data)
{
	if (offset != -1)
		fseek(file, offset, SEEK_SET);

	//clock_t endwait;
    //endwait = clock () + 2 * CLOCKS_PER_SEC;

	//int fd = fdnum(file);
	//int ioctl(fd, FIONBIO);
	
	if (fwrite(data, num_bytes, 1, file) != 1)
		return -1;

	//printf("If there was an error _%s_\n", strerror(errno));

	fflush(file);

	return 0;
}

int writeFileInt(FILE* file, int_8 offset, int_8* data)
{
	int num_bytes = 8;

	if (offset != -1)
		fseek(file, offset, SEEK_SET);

	//int fd = fdnum(file);
	//int ioctl(fd, FIONBIO);
	
	if (fwrite(data, num_bytes, 1, file) != 1)
		return -1;

	//printf("If there was an error _%s_\n", strerror(errno));

	fflush(file);

	return 0;
}

int writeFileDouble(FILE* file, int_8 offset, double* data)
{
	int num_bytes = 8;

	if (offset != -1)
		fseek(file, offset, SEEK_SET);

	//int fd = fdnum(file);
	//int ioctl(fd, FIONBIO);
	
	if (fwrite(data, num_bytes, 1, file) != 1)
		return -1;

	//printf("If there was an error _%s_\n", strerror(errno));

	fflush(file);

	return 0;
}