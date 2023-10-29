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
#include <ctype.h>
#include "DB_HelperFunctions.h"
#include "DB_Driver.h"

typedef unsigned long long int_8;


int strLength(char* str)
{
	int index = 0;
	while (str[index] != 0)
		index++;
	return index;
}

int strcontains(char* str, char the_char)
{
	int index = 0;
	while (str[index] != 0)
	{
		if (str[index] == the_char)
			return 1;
		index++;
	}
	return 0;
}

int indexOf(char* str, char the_char)
{
	int index = 0;
	while (str[index] != 0)
	{
		if (str[index] == the_char)
			return index;
		index++;
	}
	if (the_char == 0)
		return index;
	return -1;
}

char* substring(char* str, int start, int end
			   ,struct file_opened_node** file_opened_head, struct malloced_node** malloced_head, int the_debug)
{
	char* new_str = (char*) myMalloc(sizeof(char) * ((end-start)+2), file_opened_head, malloced_head, the_debug);
	if (new_str == NULL)
	{
		if (the_debug == YES_DEBUG)
			printf("	ERROR in substring() at line %d in %s\n", __LINE__, __FILE__);
		return NULL;
	}
	int j=0;
	for (int i=start; i<=end; i++)
	{
		new_str[j] = str[i];
		j++;
	}
	new_str[j] = 0;

	return new_str;
}

char** strSplit(char* str, char the_char, int* size_result
			   ,struct file_opened_node** file_opened_head, struct malloced_node** malloced_head, int the_debug)
{
	char** result = 0;
	size_t count = 0;
	char* tmp = str;
	char* last_comma = 0;
	char delim[2];
	delim[0] = the_char;
	delim[1] = 0;

	/* Count how many elements will be extracted. */
	while (*tmp)
	{
		if (the_char == *tmp)
		{
			count++;
			last_comma = tmp;
		}
		tmp++;
	}

	*size_result = count+1;

	/* Add space for trailing token. */
	count += last_comma < (str + strlen(str) - 1);

	/* Add space for terminating null string so caller
	   knows where the list of returned strings ends. */
	count++;

	result = myMalloc(sizeof(char*) * count, file_opened_head, malloced_head, the_debug);
	if (result == NULL)
	{
		if (the_debug == YES_DEBUG)
			printf("	ERROR in strSplit() at line %d in %s\n", __LINE__, __FILE__);
		return NULL;
	}

	if (result)
	{
		size_t idx  = 0;
		char* token = strtok(str, delim);

		while (token)
		{
			assert(idx < count);
			*(result + idx++) = strdup(token);
			token = strtok(0, delim);
		}
		assert(idx == count - 1);
		*(result + idx) = 0;
	}

	return result;
}

char** strSplitV2(char* str, char the_char, int* size_result
				 ,struct file_opened_node** file_opened_head, struct malloced_node** malloced_head, int the_debug)
{
	char** result;
	int num_elems = 0;
	int index = 0;

	struct ListNode* index_list_head = NULL;
	struct ListNode* index_list_tail = NULL;

	while (str[index] != 0)
	{
		if (str[index] == the_char)
		{
			num_elems++;
			addListNode(&index_list_head, &index_list_tail, index, ADDLISTNODE_TAIL
					   ,file_opened_head, malloced_head, the_debug);
		}

		index++;
	}

	num_elems++;

	*size_result = num_elems;

	//traverseListNodes(&index_list_head, &index_list_tail, TRAVERSELISTNODES_HEAD, "The List: ");

	result = myMalloc(sizeof(char*) * num_elems, file_opened_head, malloced_head, the_debug);
	if (result == NULL)
	{
		if (the_debug == YES_DEBUG)
			printf("	ERROR in strSplit() at line %d in %s\n", __LINE__, __FILE__);
		return NULL;
	}

	struct ListNode* cur = index_list_head;
	for (int i=0; i<num_elems; i++)
	{
		int start;
		int end;

		if (i == 0)
		{
			start = 0;
			end = cur->value-1;
		}
		else
		{
			start = cur->value+1;

			if (cur->next == NULL)
				end = strLength(str);
			else
				end = cur->next->value-1;
		}

		//printf("start = %d, end = %d\n", start, end);

		result[i] = substring(str, start, end
							 ,file_opened_head, malloced_head, the_debug);

		if (i > 0)
			cur = cur->next;
	}

	freeListNodesV2(&index_list_tail, file_opened_head, malloced_head, the_debug);

	return result;
}

char* upper(char* str
		   ,struct file_opened_node** file_opened_head, struct malloced_node** malloced_head, int the_debug)
{
	int length = strLength(str);

	char* new_str = (char*) myMalloc(sizeof(char) * (length+1), file_opened_head, malloced_head, the_debug);
	if (new_str == NULL)
	{
		if (the_debug == YES_DEBUG)
			printf("	ERROR in upper() at line %d in %s\n", __LINE__, __FILE__);
		return NULL;
	}

	for (int i=0; i<length; i++)
	{
		new_str[i] = toupper(str[i]);
	}
	new_str[length] = 0;

	return new_str;
}

int strIsNotEmpty(char* str)
{
	int index = 0;
	while (str[index] != 0)
	{
		if (str[index] != ' ' && str[index] != '\n')
			return 1;

		index++;
	}

	return 0;
}


void* myMalloc(size_t size
			  ,struct file_opened_node** file_opened_head, struct malloced_node** malloced_head, int the_debug)
{
	void* new_ptr = (void*) malloc(size);
	if (new_ptr == NULL)
	{
		if (the_debug == YES_DEBUG)
			printf("	ERROR in myMalloc() at line %d in %s\n", __LINE__, __FILE__);
		errorTeardown(file_opened_head, malloced_head, the_debug);
		return NULL;
	}

	if (*malloced_head == NULL)
	{
		*malloced_head = (struct malloced_node*) malloc(sizeof(struct malloced_node));
		if (*malloced_head == NULL)
		{
			free(new_ptr);
			if (the_debug == YES_DEBUG)
				printf("	ERROR in myMalloc() at line %d in %s\n", __LINE__, __FILE__);
			errorTeardown(file_opened_head, malloced_head, the_debug);
			return NULL;
		}
		(*malloced_head)->ptr = new_ptr;
		//(*malloced_head)->persists = the_persists;
		(*malloced_head)->next = NULL;
	}
	else
	{
		struct malloced_node* temp = (struct malloced_node*) malloc(sizeof(struct malloced_node));
		if (temp == NULL)
		{
			free(new_ptr);
			if (the_debug == YES_DEBUG)
				printf("	ERROR in myMalloc() at line %d in %s\n", __LINE__, __FILE__);
			errorTeardown(file_opened_head, malloced_head, the_debug);
			return NULL;
		}
		temp->ptr = new_ptr;
		//temp->persists = the_persists;
		temp->next = *malloced_head;
		*malloced_head = temp;
	}

	//sscanf(addy_arr[addy_arr_index], "%x", new_ptr);


	return new_ptr;
}

int myFree(void** old_ptr
		  ,struct file_opened_node** file_opened_head, struct malloced_node** malloced_head, int the_debug)
{
	if ((*malloced_head)->ptr == *old_ptr)
	{
		//if (the_debug == YES_DEBUG)
		//	printf("-1 iteration\n");
		struct malloced_node* temp = *malloced_head;
		*malloced_head = (*malloced_head)->next;

		free(temp->ptr);
		temp->ptr = NULL;

		free(temp);
		temp = NULL;

		*old_ptr = NULL;

		return 0;
	}
	else
	{
		//int iters = 0;
		struct malloced_node* cur = *malloced_head;
		while (cur->next != NULL)
		{
			if (cur->next->ptr == *old_ptr)
			{
				struct malloced_node* temp = cur->next;
				cur->next = cur->next->next;
				
				free(temp->ptr);
				temp->ptr = NULL;

				free(temp);
				temp = NULL;

				*old_ptr = NULL;

				//if (the_debug == YES_DEBUG)
				//	printf("%d iterations\n", iters);

				return 0;
			}
			cur = cur->next;
			//if (the_debug == YES_DEBUG)
			//	iters++;
		}
	}

	return -1;
}

int myFreeAllError(struct malloced_node** malloced_head, int the_debug)
{
	int total_freed = 0;
	while (*malloced_head != NULL)
	{
		struct malloced_node* temp = *malloced_head;
		*malloced_head = (*malloced_head)->next;
		if (temp->ptr != NULL)
		{
			free(temp->ptr);
			temp->ptr = NULL;
		}
		free(temp);
		total_freed++;
	}
	if (the_debug == YES_DEBUG)
		printf("myFreeAllError() freed %d malloced_nodes and ptrs\n", total_freed);
	return total_freed;
}

int myFreeAllCleanup(struct malloced_node** malloced_head, int the_debug)
{
	int total_freed = 0;
	while (*malloced_head != NULL)
	{
		struct malloced_node* temp = *malloced_head;
		*malloced_head = (*malloced_head)->next;

		temp->ptr = NULL;

		free(temp);
		temp = NULL;

		total_freed++;
	}
	if (the_debug == YES_DEBUG)
		printf("myFreeAllCleanup() freed %d malloced_nodes but not the ptrs\n", total_freed);
	return total_freed;
}

int myFreeJustNode(void** old_ptr
				  ,struct file_opened_node** file_opened_head, struct malloced_node** malloced_head, int the_debug)
{
	if ((*malloced_head)->ptr == *old_ptr)
	{
		struct malloced_node* temp = *malloced_head;
		*malloced_head = (*malloced_head)->next;

		// DONT DO THESE because don't want to free pointer
		//free(temp->ptr);
		//temp->ptr = NULL;

		free(temp);
		temp = NULL;
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
				
				// DONT DO THESE because don't want to free pointer
				//free(temp->ptr);
				//temp->ptr = NULL;

				free(temp);
				temp = NULL;
				
				break;
			}
			cur = cur->next;
		}
	}

	return 0;
}

void* myMallocV2(size_t size
								,struct file_opened_node** file_opened_head, struct malloced_node** malloced_head, int the_debug)
{
	void* new_ptr = (void*) malloc(size);
	if (new_ptr == NULL)
	{
		if (the_debug == YES_DEBUG)
			printf("	ERROR in myMallocV2() at line %d in %s\n", __LINE__, __FILE__);
		errorTeardown(file_opened_head, malloced_head, the_debug);
		return NULL;
	}

	if (*malloced_head == NULL)
	{
		*malloced_head = (struct malloced_node*) malloc(sizeof(struct malloced_node));
		if (*malloced_head == NULL)
		{
			free(new_ptr);
			if (the_debug == YES_DEBUG)
				printf("	ERROR in myMallocV2() at line %d in %s\n", __LINE__, __FILE__);
			errorTeardown(file_opened_head, malloced_head, the_debug);
			return NULL;
		}
		(*malloced_head)->ptr = new_ptr;
		(*malloced_head)->next = NULL;
		(*malloced_head)->prev = NULL;
	}
	else
	{
		struct malloced_node* temp = (struct malloced_node*) malloc(sizeof(struct malloced_node));
		if (temp == NULL)
		{
			free(new_ptr);
			if (the_debug == YES_DEBUG)
				printf("	ERROR in myMallocV2() at line %d in %s\n", __LINE__, __FILE__);
			errorTeardown(file_opened_head, malloced_head, the_debug);
			return NULL;
		}
		temp->ptr = new_ptr;
		temp->next = *malloced_head;
		temp->prev = NULL;

		(*malloced_head)->prev = temp;
		*malloced_head = temp;
	}

	return *malloced_head;
}

int myFreeV2(struct malloced_node** old_malloced_node
		    ,struct file_opened_node** file_opened_head, struct malloced_node** malloced_head, int the_debug)
{
	free((*old_malloced_node)->ptr);
	(*old_malloced_node)->ptr = NULL;

	(*old_malloced_node)->prev->next = (*old_malloced_node)->next;
	(*old_malloced_node)->next->prev = (*old_malloced_node)->prev;

	free(*old_malloced_node);
	*old_malloced_node = NULL;

	return 0;
}

int concatFileName(char* new_filename, char* filetype, int_8 table_num, int_8 col_num)
{
	strcpy(new_filename, "DB_Files_2\\DB");

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

	return 0;
}

FILE* myFileOpenSimple(char* file_name, char* mode
					  ,struct file_opened_node** file_opened_head, struct malloced_node** malloced_head, int the_debug)
{
	FILE* new_file = fopen(file_name, mode);

	if (new_file == NULL)
	{
		if (the_debug == YES_DEBUG)
			printf("	ERROR in myFileOpenSimple() at line %d in %s\n", __LINE__, __FILE__);
		errorTeardown(file_opened_head, malloced_head, the_debug);
		return NULL;
	}
	else
	{
		if (*file_opened_head == NULL)
		{
			*file_opened_head = (struct file_opened_node*) myMalloc(sizeof(struct file_opened_node), file_opened_head, malloced_head, the_debug);
			if (*file_opened_head == NULL)
			{
				if (the_debug == YES_DEBUG)
					printf("	ERROR in myFileOpenSimple() at line %d in %s\n", __LINE__, __FILE__);
				fclose(new_file);
				return NULL;
			}
			(*file_opened_head)->file = new_file;
			(*file_opened_head)->next = NULL;
		}
		else
		{
			struct file_opened_node* temp = (struct file_opened_node*) myMalloc(sizeof(struct file_opened_node), file_opened_head, malloced_head, the_debug);
			if (temp == NULL)
			{
				if (the_debug == YES_DEBUG)
					printf("	ERROR in myFileOpenSimple() at line %d in %s\n", __LINE__, __FILE__);
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

FILE* myFileOpen(char* filetype, int_8 num_table, int_8 num_col, char* mode
				,struct file_opened_node** file_opened_head, struct malloced_node** malloced_head, int the_debug)
{
	char* file_name = (char*) myMalloc(sizeof(char) * 64, file_opened_head, malloced_head, the_debug);
	if (file_name == NULL)
	{
		if (the_debug == YES_DEBUG)
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
			if (the_debug == YES_DEBUG)
				printf("	ERROR in myFileOpen() at line %d in %s\n", __LINE__, __FILE__);
			errorTeardown(file_opened_head, malloced_head, the_debug);
			return NULL;
		}
		if (access(file_name, W_OK) != 0)
		{
			if (the_debug == YES_DEBUG)
				printf("	ERROR in myFileOpen() at line %d in %s\n", __LINE__, __FILE__);
			errorTeardown(file_opened_head, malloced_head, the_debug);
			return NULL;
		}
	}


	FILE* new_file = fopen(file_name, mode);
	myFree((void**) &file_name, file_opened_head, malloced_head, the_debug);


	if (new_file == NULL)
	{
		if (the_debug == YES_DEBUG)
			printf("	ERROR in myFileOpen() at line %d in %s\n", __LINE__, __FILE__);
		errorTeardown(file_opened_head, malloced_head, the_debug);
		return NULL;
	}
	else
	{
		if (*file_opened_head == NULL)
		{
			*file_opened_head = (struct file_opened_node*) myMalloc(sizeof(struct file_opened_node), file_opened_head, malloced_head, the_debug);
			if (*file_opened_head == NULL)
			{
				fclose(new_file);
				if (the_debug == YES_DEBUG)
					printf("	ERROR in myFileOpen() at line %d in %s\n", __LINE__, __FILE__);
				return NULL;
			}
			(*file_opened_head)->file = new_file;
			(*file_opened_head)->next = NULL;
		}
		else
		{
			struct file_opened_node* temp = (struct file_opened_node*) myMalloc(sizeof(struct file_opened_node), file_opened_head, malloced_head, the_debug);
			if (temp == NULL)
			{
				fclose(new_file);
				if (the_debug == YES_DEBUG)
					printf("	ERROR in myFileOpen() at line %d in %s\n", __LINE__, __FILE__);
				return NULL;
			}
			temp->file = new_file;
			temp->next = *file_opened_head;
			*file_opened_head = temp;
		}
	}

	return new_file;
}

int myFileClose(FILE* old_file
			   ,struct file_opened_node** file_opened_head, struct malloced_node** malloced_head, int the_debug)
{
	if ((*file_opened_head)->file == old_file)
	{
		struct file_opened_node* temp = *file_opened_head;
		*file_opened_head = (*file_opened_head)->next;
		myFree((void**) &temp, file_opened_head, malloced_head, the_debug);
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
				myFree((void**) &temp, file_opened_head, malloced_head, the_debug);
				break;
			}
			cur = cur->next;
		}
	}

	fclose(old_file);
	old_file = NULL;
	
	return 0;
}

int myFileCloseAll(struct file_opened_node** file_opened_head, struct malloced_node** malloced_head
				  ,int the_debug)
{
	int total_closed = 0;
	while (*file_opened_head != NULL)
	{
		struct file_opened_node* temp = *file_opened_head;
		*file_opened_head = (*file_opened_head)->next;
		fclose(temp->file);
		myFree((void**) &temp, file_opened_head, malloced_head, the_debug);
		total_closed++;
	}
	if (the_debug == YES_DEBUG)
		printf("myFileCloseAll() closed %d files\n", total_closed);
	return total_closed;
}

char* intToDate(int_8 the_int_form
			   ,struct file_opened_node** file_opened_head, struct malloced_node** malloced_head, int the_debug)
{
	int year = 1900;
	int month = 1;
	int day = 1;

	int remaining = the_int_form;

	// START Find the right year
	// 1900 is not a leap year so only at 1460 days for 1900 - 1903
	if (remaining >= 1460)
	{
		year += 4;
		remaining -= 1460;
	}
	else
	{
		while (remaining >= 365)
		{
			year += 1;
			remaining -= 365;
		}
	}

	//printf("remaining = %lu\n", remaining);
	//printf("current date = %d/%d/%d\n", month, day, year);

	while (remaining >= 1461)
	{
		year += 4;
		remaining -= 1461;
	}

	//printf("remaining = %lu\n", remaining);
	//printf("current date = %d/%d/%d\n", month, day, year);

	if (remaining >= 366)
	{
		year += 1;
		remaining -= 366;

		while (remaining >= 365)
		{
			year += 1;
			remaining -= 365;
		}
	}

	//printf("remaining = %lu\n", remaining);
	//printf("current date = %d/%d/%d\n", month, day, year);
	// END Find the right year

	// START Find the right month
	if (remaining >= 31 && month == 1)	//	Jan
	{
		month += 1;
		remaining -= 31;
		//printf("remaining = %lu\n", remaining);
		//printf("current date = %d/%d/%d\n", month, day, year);
	}

	if (remaining >= (year % 4 == 0 && year != 1900 ? 29 : 28) && month == 2)	//	Feb
	{
		month += 1;
		remaining -= (year % 4 == 0 && year != 1900 ? 29 : 28);
		//printf("remaining = %lu\n", remaining);
		//printf("current date = %d/%d/%d\n", month, day, year);
	}
	
	if (remaining >= 31 && month == 3)	//	Mar
	{
		month += 1;
		remaining -= 31;
		//printf("remaining = %lu\n", remaining);
		//printf("current date = %d/%d/%d\n", month, day, year);
	}
		
	if (remaining >= 30 && month == 4)	//	Apr
	{
		month += 1;
		remaining -= 30;
		//printf("remaining = %lu\n", remaining);
		//printf("current date = %d/%d/%d\n", month, day, year);
	}
		
	if (remaining >= 31 && month == 5)	//	May
	{
		month += 1;
		remaining -= 31;
		//printf("remaining = %lu\n", remaining);
		//printf("current date = %d/%d/%d\n", month, day, year);
	}
	
	if (remaining >= 30 && month == 6)	//	Jun
	{
		month += 1;
		remaining -= 30;
		//printf("remaining = %lu\n", remaining);
		//printf("current date = %d/%d/%d\n", month, day, year);
	}
		
	if (remaining >= 31 && month == 7)	//	Jul
	{
		month += 1;
		remaining -= 31;
		//printf("remaining = %lu\n", remaining);
		//printf("current date = %d/%d/%d\n", month, day, year);
	}
	
	if (remaining >= 31 && month == 8)	//	Aug
	{
		month += 1;
		remaining -= 31;
		//printf("remaining = %lu\n", remaining);
		//printf("current date = %d/%d/%d\n", month, day, year);
	}
	
	if (remaining >= 30 && month == 9)	//	Sep
	{
		month += 1;
		remaining -= 30;
		//printf("remaining = %lu\n", remaining);
		//printf("current date = %d/%d/%d\n", month, day, year);
	}
		
	if (remaining >= 31 && month == 10)	//	Oct
	{
		month += 1;
		remaining -= 31;
		//printf("remaining = %lu\n", remaining);
		//printf("current date = %d/%d/%d\n", month, day, year);
	}
		
	if (remaining >= 30 && month == 11)	//	Nov
	{
		month += 1;
		remaining -= 30;
		//printf("remaining = %lu\n", remaining);
		//printf("current date = %d/%d/%d\n", month, day, year);
	}
		
	if (remaining >= 31 && month == 12)	//	Dec
	{
		month += 1;
		remaining -= 31;
		//printf("remaining = %lu\n", remaining);
		//printf("current date = %d/%d/%d\n", month, day, year);
	}	
	// END Find the right month

	day += remaining;

	//printf("remaining = %lu\n", remaining);
	//printf("current date = %d/%d/%d\n", month, day, year);

	char* date = (char*) myMalloc(sizeof(char) * 20, file_opened_head, malloced_head, the_debug);
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
	int year = -1;
	int month = -1;
	int day = -1;
	
	sscanf(the_date_form, "%d/%d/%d", &month, &day, &year);

	if (year == -1 || month == -1 || day == -1)
	{
		printf("	ERROR in dateToInt() at line %d in %s\n", __LINE__, __FILE__);
	}

	int_8 remaining = (int_8) day;
	remaining--;

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

	//printf("remaining = %lu\n", remaining);
	//printf("current date = %d/%d/%d\n", month, day, year);

	if (year > 1900)
	{
		while ((year-1) % 4 != 0)
		{
			remaining += 365;
			year--;
		}

		if ((year-1) % 4 == 0 && (year-1) != 1900)
		{
			remaining += 366;
			year--;
		}
		else if ((year-1) % 4 == 0)
		{
			remaining += 365;
			year--;
		}
		//printf("remaining = %lu\n", remaining);
		//printf("current date = %d/%d/%d\n", month, day, year);
	}

	while (year > 1904)
	{
		remaining += 1461;
		year -= 4;
	}

	//printf("remaining = %lu\n", remaining);
	//printf("current date = %d/%d/%d\n", month, day, year);

	if (year == 1904)
	{
		remaining += 1460;
		year -= 4;
	}

	//printf("remaining = %lu\n", remaining);
	//printf("current date = %d/%d/%d\n", month, day, year);

	return remaining;
}

char* readFileChar(FILE* file, int_8 offset
				  ,struct file_opened_node** file_opened_head, struct malloced_node** malloced_head, int the_debug)
{
	int num_bytes = 32;

	char* raw_bytes = (char*) myMalloc(sizeof(char) * num_bytes, file_opened_head, malloced_head, the_debug);
	if (raw_bytes == NULL)
	{
		if (the_debug == YES_DEBUG)
			printf("	ERROR in readFileChar() at line %d in %s\n", __LINE__, __FILE__);
		return NULL;
	}

	fseek(file, offset, SEEK_SET);

	if (fread(raw_bytes, num_bytes, 1, file) == 0)
	{
		if (the_debug == YES_DEBUG)
			printf("	ERROR in readFileChar() at line %d in %s\n", __LINE__, __FILE__);
		errorTeardown(file_opened_head, malloced_head, the_debug);
		return NULL;
	}

	return raw_bytes;
}

char* readFileCharData(FILE* file, int_8 offset, int_8* the_num_bytes
					  ,struct file_opened_node** file_opened_head, struct malloced_node** malloced_head, int the_debug)
{
	int num_bytes = *the_num_bytes;

	char* raw_bytes = (char*) myMalloc(sizeof(char) * num_bytes, file_opened_head, malloced_head, the_debug);
	if (raw_bytes == NULL)
	{
		if (the_debug == YES_DEBUG)
			printf("	ERROR in readFileCharData() at line %d in %s\n", __LINE__, __FILE__);
		return NULL;
	}

	fseek(file, offset, SEEK_SET);

	if (fread(raw_bytes, num_bytes, 1, file) == 0)
	{
		if (the_debug == YES_DEBUG)
			printf("	ERROR in readFileCharData() at line %d in %s\n", __LINE__, __FILE__);
		errorTeardown(file_opened_head, malloced_head, the_debug);
		return NULL;
	}

	return raw_bytes;
}

int_8 readFileInt(FILE* file, int_8 offset
				 ,struct file_opened_node** file_opened_head, struct malloced_node** malloced_head, int the_debug)
{
	int num_bytes = 8;

	int_8 raw_int;

	fseek(file, offset, SEEK_SET);

	if (fread(&raw_int, num_bytes, 1, file) == 0)
	{
		if (the_debug == YES_DEBUG)
			printf("	ERROR in readFileInt() at line %d in %s\n", __LINE__, __FILE__);
		errorTeardown(file_opened_head, malloced_head, the_debug);
		return -1;
	}

	return raw_int;
}

double readFileDouble(FILE* file, int_8 offset
					 ,struct file_opened_node** file_opened_head, struct malloced_node** malloced_head, int the_debug)
{
	int num_bytes = 8;

	double raw_double;

	fseek(file, offset, SEEK_SET);

	if (fread(&raw_double, num_bytes, 1, file) == 0)
	{
		if (the_debug == YES_DEBUG)
			printf("	ERROR in readFileDouble() at line %d in %s\n", __LINE__, __FILE__);
		errorTeardown(file_opened_head, malloced_head, the_debug);
		return -1.0;
	}

	return raw_double;
}

int writeFileChar(FILE* file, int_8 offset, char* data
				 ,struct file_opened_node** file_opened_head, struct malloced_node** malloced_head, int the_debug)
{
	int num_bytes = 32;

	char* new_data = (char*) myMalloc(sizeof(char) * num_bytes, file_opened_head, malloced_head, the_debug);
	if (new_data == NULL)
	{
		if (the_debug == YES_DEBUG)
			printf("	ERROR in writeFileChar() at line %d in %s\n", __LINE__, __FILE__);
		return -1;
	}

	strcpy(new_data, data);
	new_data[num_bytes-1] = 0;


	if (offset != APPEND_OFFSET)
		fseek(file, offset, SEEK_SET);
	
	if (fwrite(new_data, num_bytes, 1, file) != 1)
	{
		if (the_debug == YES_DEBUG)
			printf("	ERROR in writeFileChar() at line %d in %s\n", __LINE__, __FILE__);
		errorTeardown(file_opened_head, malloced_head, the_debug);
		return -1;
	}

	myFree((void**) &new_data, file_opened_head, malloced_head, the_debug);

	fflush(file);

	return 0;
}

int writeFileCharData(FILE* file, int_8 offset, int_8* the_num_bytes, char* data
					 ,struct file_opened_node** file_opened_head, struct malloced_node** malloced_head, int the_debug)
{
	int num_bytes = *the_num_bytes;

	char* new_data = (char*) myMalloc(sizeof(char) * num_bytes, file_opened_head, malloced_head, the_debug);
	if (new_data == NULL)
	{
		if (the_debug == YES_DEBUG)
			printf("	ERROR in writeFileCharData() at line %d in %s\n", __LINE__, __FILE__);
		return -1;
	}

	strcpy(new_data, data);
	new_data[num_bytes-1] = 0;


	if (offset != APPEND_OFFSET)
		fseek(file, offset, SEEK_SET);
	
	if (fwrite(new_data, num_bytes, 1, file) != 1)
	{
		if (the_debug == YES_DEBUG)
			printf("	ERROR in writeFileCharData() at line %d in %s\n", __LINE__, __FILE__);
		errorTeardown(file_opened_head, malloced_head, the_debug);
		return -1;
	}

	myFree((void**) &new_data, file_opened_head, malloced_head, the_debug);

	fflush(file);

	return 0;
}

int writeFileInt(FILE* file, int_8 offset, int_8* data
				,struct file_opened_node** file_opened_head, struct malloced_node** malloced_head, int the_debug)
{
	int num_bytes = 8;

	if (offset != APPEND_OFFSET)
		fseek(file, offset, SEEK_SET);
	
	if (fwrite(data, num_bytes, 1, file) != 1)
	{
		if (the_debug == YES_DEBUG)
			printf("	ERROR in writeFileInt() at line %d in %s\n", __LINE__, __FILE__);
		errorTeardown(file_opened_head, malloced_head, the_debug);
		return -1;
	}

	fflush(file);

	return 0;
}

int writeFileDouble(FILE* file, int_8 offset, double* data
				   ,struct file_opened_node** file_opened_head, struct malloced_node** malloced_head, int the_debug)
{
	int num_bytes = 8;

	if (offset != APPEND_OFFSET)
		fseek(file, offset, SEEK_SET);
	
	if (fwrite(data, num_bytes, 1, file) != 1)
	{
		if (the_debug == YES_DEBUG)
			printf("	ERROR in writeFileDouble() at line %d in %s\n", __LINE__, __FILE__);
		errorTeardown(file_opened_head, malloced_head, the_debug);
		return -1;
	}

	fflush(file);

	return 0;
}


int addListNode(struct ListNode** the_head, struct ListNode** the_tail, int the_value, int the_add_mode
			   ,struct file_opened_node** file_opened_head, struct malloced_node** malloced_head, int the_debug)
{
	/*	the_add_mode = 1 for add to head
		the_add_mode = 2 for add to tail
	*/
	struct ListNode* temp_new = (struct ListNode*) myMalloc(sizeof(struct ListNode), file_opened_head, malloced_head, the_debug);
	if (temp_new == NULL)
	{
		if (the_debug == YES_DEBUG)
			printf("	ERROR in addListNode() at line %d in %s\n", __LINE__, __FILE__);
		return -2;
	}
	temp_new->value = the_value;

	if (*the_head == NULL)
	{
		*the_head = temp_new;
		(*the_head)->next = NULL;
		(*the_head)->prev = NULL;

		*the_tail = *the_head;
	}
	else if (the_add_mode == 1)
	{
		temp_new->next = (*the_head);
		temp_new->prev = NULL;

		(*the_head)->prev = temp_new;
		*the_head = temp_new;
	}
	else if (the_add_mode == 2)
	{
		temp_new->next = NULL;
		temp_new->prev = (*the_tail);

		(*the_tail)->next = temp_new;
		*the_tail = temp_new;
	}

	return 0;
}

int removeListNode(struct ListNode** the_head, struct ListNode** the_tail, int the_value, int the_remove_mode
				  ,struct file_opened_node** file_opened_head, struct malloced_node** malloced_head, int the_debug)
{
	/*	the_remove_mode = 1 for remove from head
		the_remove_mode = 2 for remove from tail
	*/
	int temp_value = 0;
	struct ListNode* temp = NULL;

	if (the_remove_mode == REMOVELISTNODE_HEAD || (the_value != -1 && (*the_head)->value == the_value))
	{
		temp = *the_head;
		temp_value = temp->value;

		*the_head = (*the_head)->next;
		(*the_head)->prev = NULL;
	}
	else if (the_remove_mode == REMOVELISTNODE_TAIL || (the_value != -1 && (*the_tail)->value == the_value))
	{
		temp = *the_tail;
		temp_value = temp->value;

		*the_tail = (*the_tail)->prev;
		(*the_tail)->next = NULL;
	}
	else if (the_value != -1)
	{
		struct ListNode* cur = *the_head;
		while (cur->next != NULL)
		{
			if (cur->next->value == the_value)
			{
				temp = cur->next;
				temp_value = cur->next->value;

				cur->next = cur->next->next;
				cur->next->prev = cur;
				break;
			}
			cur = cur->next;
		}
	}

	if (malloced_head != NULL)
		myFree((void**) &temp, file_opened_head, malloced_head, the_debug);
	else
	{
		free(temp);
		temp = NULL;
	}

	return temp_value;
}

int traverseListNodes(struct ListNode** the_head, struct ListNode** the_tail, int the_mode, char* start_text)
{
	/*	the_mode = 1 for head to tail
		the_mode = 2 for tail to head
	*/
	struct ListNode* cur;
	if (the_mode == 1)
		cur = *the_head;
	if (the_mode == 2)
		cur = *the_tail;

	printf("%s", start_text);
	while (cur != NULL)
	{
		printf("%d, ", cur->value);
		if (the_mode == 1)
			cur = cur->next;
		if (the_mode == 2)
			cur = cur->prev;
	}
	printf("\n");

	return 0;
}

int freeListNodes(struct ListNode** the_head
				 ,struct file_opened_node** file_opened_head, struct malloced_node** malloced_head, int the_debug)
{
	int total_freed = 0;
	while (*the_head != NULL)
	{
		struct ListNode* temp = *the_head;
		*the_head = (*the_head)->next;

		if (malloced_head == NULL)
		{
			free(temp);
			temp = NULL;
		}
		else
			myFree((void**) &temp, file_opened_head, malloced_head, the_debug);
		total_freed++;
	}

	return total_freed;
}

int freeListNodesV2(struct ListNode** the_tail
				   ,struct file_opened_node** file_opened_head, struct malloced_node** malloced_head, int the_debug)
{
	int total_freed = 0;

	struct malloced_node* cur_mal = *malloced_head;

	while (*the_tail != NULL)
	{
		struct ListNode* temp = *the_tail;
		*the_tail = (*the_tail)->prev;

		if (malloced_head == NULL)
		{
			free(temp);
			temp = NULL;
		}
		else
		{
			//myFree((void**) &temp, file_opened_head, malloced_head, the_debug);
			
			if ((*malloced_head)->ptr == temp)
			{
				//if (the_debug == YES_DEBUG)
				//	printf("-1 iterations\n");
				struct malloced_node* temp_mal = (*malloced_head);
				*malloced_head = (*malloced_head)->next;

				free(temp_mal->ptr);
				temp_mal->ptr = NULL;

				free(temp_mal);
				temp_mal = NULL;

				total_freed++;
			}
			else
			{
				//int iters = 0;
				while (cur_mal->next != NULL)
				{
					if (cur_mal->next->ptr == temp)
					{
						struct malloced_node* temp_mal = cur_mal->next;
						cur_mal->next = cur_mal->next->next;
						
						free(temp_mal->ptr);
						temp_mal->ptr = NULL;

						free(temp_mal);
						temp_mal = NULL;

						total_freed++;

						//if (the_debug == YES_DEBUG)
						//	printf("%d iterations\n", iters);

						break;
					}
					cur_mal = cur_mal->next;
					//if (the_debug == YES_DEBUG)
					//	iters++;
				}
			}
		}
	}

	return total_freed;
}

int errorTeardown(struct file_opened_node** file_opened_head, struct malloced_node** malloced_head, int the_debug)
{
	if (the_debug == YES_DEBUG)
		printf("!!! Called errorTeardown\n");

	if (file_opened_head != NULL)
		myFileCloseAll(file_opened_head, malloced_head, the_debug);

	myFreeAllError(malloced_head, the_debug);

	return 0;
}