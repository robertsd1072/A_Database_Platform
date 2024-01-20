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

char** strSplitV2(char* str, char the_char, int* size_result
				 ,struct file_opened_node** file_opened_head, struct malloced_node** malloced_head, int the_debug)
{
	char** result;
	int num_elems = 0;
	int index = 0;

	struct ListNodePtr* index_list_head = NULL;
	struct ListNodePtr* index_list_tail = NULL;

	while (str[index] != 0)
	{
		if (str[index] == the_char)
		{
			num_elems++;

			int* index_malloced = (int*) myMalloc(sizeof(int), file_opened_head, malloced_head, the_debug);
			*index_malloced = index;

			addListNodePtr(&index_list_head, &index_list_tail, index_malloced, PTR_TYPE_INT, ADDLISTNODE_TAIL
						  ,file_opened_head, malloced_head, the_debug);
		}

		index++;
	}

	num_elems++;

	*size_result = num_elems;

	result = myMalloc(sizeof(char*) * num_elems, file_opened_head, malloced_head, the_debug);
	if (result == NULL)
	{
		if (the_debug == YES_DEBUG)
			printf("	ERROR in strSplit() at line %d in %s\n", __LINE__, __FILE__);
		return NULL;
	}

	struct ListNodePtr* cur = index_list_head;
	for (int i=0; i<num_elems; i++)
	{
		int start;
		int end;

		if (i == 0)
		{
			start = 0;
			end = (*((int*) cur->ptr_value))-1;
		}
		else
		{
			start = (*((int*) cur->ptr_value))+1;

			if (cur->next == NULL)
				end = strLength(str);
			else
				end = (*((int*) cur->next->ptr_value))-1;
		}

		//printf("start = %d, end = %d\n", start, end);

		result[i] = substring(str, start, end, file_opened_head, malloced_head, the_debug);

		if (i > 0)
			cur = cur->next;
	}

	int freed = freeAnyLinkedList((void**) &index_list_head, PTR_TYPE_LIST_NODE_PTR, file_opened_head, malloced_head, the_debug);
	//printf("freed = %d\n", freed);

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

/*	Writes to (so calling function can read):
 *		char* word;
 *		int* cur_index;
 */
int getNextWord(char* input, char* word, int* cur_index)
{
	// START Skip so called "empty" characters
	while (input[*cur_index] != 0 && (input[*cur_index] == ' ' || input[*cur_index] == '\t' || input[*cur_index] == '\n' || input[*cur_index] == '\v'))
	{
		(*cur_index)++;
	}
	// END Skip so called "empty" characters

	int word_index = 0;
	word[word_index] = 0;

	if (input[*cur_index] == 39 /*Single quote*/)
	{
		for (int i=0; i<2; i++)
		{
			word[word_index] = input[*cur_index];
			word[word_index+1] = 0;

			word_index++;
			(*cur_index)++;
		}

		// START Iterate until find another single quote or 0
		bool two_single_quotes = false;
		while (two_single_quotes || !(input[*cur_index] == 0 || (input[(*cur_index)-1] == 39 && input[*cur_index] != 39)))
		{
			if (two_single_quotes)
				two_single_quotes = false;
			if (input[(*cur_index)-1] == 39 && input[*cur_index] == 39)
			{
				(*cur_index)++;
				two_single_quotes = true;
			}
			else
			{
				word[word_index] = input[*cur_index];
				word[word_index+1] = 0;

				word_index++;
				(*cur_index)++;
			}
		}
		// END Iterate until find another single quote, 0, ;, or comma
	}
	else
	{
		// START Iterate until one of the below characters
		while (input[*cur_index] != 0 && input[*cur_index] != ' ' && input[*cur_index] != ',' && input[*cur_index] != ';'
			   && input[*cur_index] != '(' && input[*cur_index] != ')' && input[*cur_index] != '.'
			   && input[*cur_index] != '\t' && input[*cur_index] != '\n' && input[*cur_index] != '\v'
			   && input[*cur_index] != '+' && input[*cur_index] != '*' && input[*cur_index] != '/' && input[*cur_index] != '^')// && input[*cur_index] != '-')
		{
			word[word_index] = input[*cur_index];
			word[word_index+1] = 0;

			word_index++;
			(*cur_index)++;
		}
		// END Iterate until one of the below characters
	}

	//printf("word = _%s_\n", word);

	if (strcmp(word, "") == 0)
	{
		// START If empty string, but cur_index at one of the following characters, make word that character
		//printf("First char: %c\n", input[*cur_index]);
		if (input[*cur_index] == ',' || input[*cur_index] == ';' || input[*cur_index] == '(' || input[*cur_index] == ')'
			|| input[*cur_index] == '+' || input[*cur_index] == '*' || input[*cur_index] == '/' || input[*cur_index] == '^')// || input[*cur_index] == '-')
		{
			word[word_index] = input[*cur_index];
			word[word_index+1] = 0;

			word_index++;
			(*cur_index)++;
		}
		else
			return -1;
		// END If empty string, but cur_index at one of the following characters, make word that character
	}

	return 0;
}

int strcmp_Upper(char* word, char* test_char
				,struct file_opened_node** file_opened_head, struct malloced_node** malloced_head, int the_debug)
{
	char* upper_word = upper(word, file_opened_head, malloced_head, the_debug);
	if (upper_word == NULL)
	{
		if (the_debug == YES_DEBUG)
			printf("	ERROR in strcmp_Upper() at line %d in %s\n", __LINE__, __FILE__);
		return -2;
	}

	char* upper_test_char = upper(test_char, file_opened_head, malloced_head, the_debug);
	if (upper_test_char == NULL)
	{
		if (the_debug == YES_DEBUG)
			printf("	ERROR in strcmp_Upper() at line %d in %s\n", __LINE__, __FILE__);
		return -2;
	}

	int compared = strcmp(upper_word, upper_test_char);
	myFree((void**) &upper_word, file_opened_head, malloced_head, the_debug);
	myFree((void**) &upper_test_char, file_opened_head, malloced_head, the_debug);

	if (compared < 0)
		return -1;
	else if (compared > 0)
		return 1;

	return compared;
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
		temp->next = *malloced_head;
		*malloced_head = temp;
	}


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

	if (the_debug == YES_DEBUG)
		printf("	ERROR in myFree() at line %d in %s\n", __LINE__, __FILE__);

	return -1;
}

/* Frees all nodes in malloced_head, and frees the ptr
 */
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

/* Frees all nodes in malloced_head, but does NOT free the ptr
 */
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

/* Frees one node in malloced_head according to old_ptr, but does not free old_ptr
 */
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


char* readFileChar(FILE* file, int_8 offset, int traverse_disk
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
		if (!traverse_disk)
		{
			if (the_debug == YES_DEBUG)
				printf("	ERROR in readFileChar() at line %d in %s\n", __LINE__, __FILE__);
			errorTeardown(file_opened_head, malloced_head, the_debug);
		}
		myFree((void**) &raw_bytes, NULL, malloced_head, the_debug);
		return NULL;
	}

	return raw_bytes;
}

char* readFileCharData(FILE* file, int_8 offset, int_8* the_num_bytes, int traverse_disk
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
		if (!traverse_disk)
		{
			if (the_debug == YES_DEBUG)
				printf("	ERROR in readFileCharData() at line %d in %s\n", __LINE__, __FILE__);
			errorTeardown(file_opened_head, malloced_head, the_debug);
		}
		myFree((void**) &raw_bytes, NULL, malloced_head, the_debug);
		return NULL;
	}

	return raw_bytes;
}

int_8 readFileInt(FILE* file, int_8 offset, int traverse_disk
				 ,struct file_opened_node** file_opened_head, struct malloced_node** malloced_head, int the_debug)
{
	int num_bytes = 8;

	int_8 raw_int;

	fseek(file, offset, SEEK_SET);

	if (fread(&raw_int, num_bytes, 1, file) == 0)
	{
		if (!traverse_disk)
		{
			if (the_debug == YES_DEBUG)
				printf("	ERROR in readFileInt() at line %d in %s\n", __LINE__, __FILE__);
			errorTeardown(file_opened_head, malloced_head, the_debug);
		}
		return -1;
	}

	return raw_int;
}

double readFileDouble(FILE* file, int_8 offset, int traverse_disk
					 ,struct file_opened_node** file_opened_head, struct malloced_node** malloced_head, int the_debug)
{
	int num_bytes = 8;

	double raw_double;

	fseek(file, offset, SEEK_SET);

	if (fread(&raw_double, num_bytes, 1, file) == 0)
	{
		if (!traverse_disk)
		{
			if (the_debug == YES_DEBUG)
				printf("	ERROR in readFileDouble() at line %d in %s\n", __LINE__, __FILE__);
			errorTeardown(file_opened_head, malloced_head, the_debug);
		}
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


int addListNodePtr(struct ListNodePtr** the_head, struct ListNodePtr** the_tail, void* the_ptr, int the_ptr_type, int the_add_mode
				  ,struct file_opened_node** file_opened_head, struct malloced_node** malloced_head, int the_debug)
{
	/*	the_add_mode = 1 for add to head
		the_add_mode = 2 for add to tail
	*/
	struct ListNodePtr* temp_new = (struct ListNodePtr*) myMalloc(sizeof(struct ListNodePtr), file_opened_head, malloced_head, the_debug);
	if (temp_new == NULL)
	{
		if (the_debug == YES_DEBUG)
			printf("	ERROR in addListNode() at line %d in %s\n", __LINE__, __FILE__);
		return -2;
	}
	temp_new->ptr_value = the_ptr;
	temp_new->ptr_type = the_ptr_type;

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

void* removeListNodePtr(struct ListNodePtr** the_head, struct ListNodePtr** the_tail, void* the_ptr, int the_ptr_type, int the_remove_mode
					 ,struct file_opened_node** file_opened_head, struct malloced_node** malloced_head, int the_debug)
{
	void* temp_ptr_value;
	struct ListNodePtr* temp;

	if (the_remove_mode == REMOVELISTNODE_HEAD || (the_ptr != NULL && equals((*the_head)->ptr_value, the_ptr_type, the_ptr, VALUE_EQUALS)))
	{
		temp = *the_head;
		temp_ptr_value = temp->ptr_value;

		*the_head = (*the_head)->next;
		(*the_head)->prev = NULL;
	}
	else if (the_remove_mode == REMOVELISTNODE_TAIL || (the_ptr != NULL && equals((*the_tail)->ptr_value, the_ptr_type, the_ptr, VALUE_EQUALS)))
	{
		temp = *the_tail;
		temp_ptr_value = temp->ptr_value;

		*the_tail = (*the_tail)->prev;
		(*the_tail)->next = NULL;
	}
	else
	{
		struct ListNodePtr* cur = *the_head;
		while (cur->next != NULL)
		{
			if (equals(cur->next->ptr_value, the_ptr_type, the_ptr, VALUE_EQUALS))
			{
				temp = cur->next;
				temp_ptr_value = cur->next->ptr_value;

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

	return temp_ptr_value;
}

int traverseListNodesPtr(struct ListNodePtr** the_head, struct ListNodePtr** the_tail, int the_traverse_mode, char* start_text)
{
	printf("%s", start_text);

	struct ListNodePtr* cur = *the_head;
	if (the_traverse_mode == TRAVERSELISTNODES_TAIL)
		cur = *the_tail;

	while (cur != NULL)
	{
		if (cur->ptr_type == PTR_TYPE_INT)
		{
			printf("%d", *((int*) cur->ptr_value));
		}

		if (the_traverse_mode == TRAVERSELISTNODES_TAIL)
		{
			if (cur->prev != NULL)
				printf(", ");
			cur = cur->prev;
		}
		else
		{
			if (cur->next != NULL)
				printf(", ");
			cur = cur->next;
		}
	}

	printf("\n");

	return 0;
}

int freeListNodesPtr(struct ListNodePtr** the_head
					,struct file_opened_node** file_opened_head, struct malloced_node** malloced_head, int the_debug)
{
	int total_freed = 0;
	while (*the_head != NULL)
	{
		struct ListNodePtr* temp = *the_head;
		*the_head = (*the_head)->next;

		if (malloced_head == NULL)
		{
			free(temp->ptr_value);
			free(temp);
			temp = NULL;
		}
		else
		{
			myFree((void**) &temp->ptr_value, file_opened_head, malloced_head, the_debug);
			myFree((void**) &temp, file_opened_head, malloced_head, the_debug);
		}
		total_freed++;
	}

	return total_freed;
}

/* Does not work
 */
int freeListNodesPtrV2(struct ListNodePtr** the_tail
					  ,struct file_opened_node** file_opened_head, struct malloced_node** malloced_head, int the_debug)
{
	int total_freed = 0;

	struct malloced_node* cur_mal = *malloced_head;

	while (*the_tail != NULL)
	{
		struct ListNodePtr* temp = *the_tail;
		*the_tail = (*the_tail)->prev;

		if (malloced_head == NULL)
		{
			free(temp->ptr_value);
			free(temp);
			temp = NULL;
		}
		else
		{
			//myFree((void**) &temp, file_opened_head, malloced_head, the_debug);
			
			if ((*malloced_head)->ptr == temp)
			{
				if (the_debug == YES_DEBUG)
					printf("-1 iterations\n");
				struct malloced_node* temp_mal = (*malloced_head);
				*malloced_head = (*malloced_head)->next;

				if (the_debug == YES_DEBUG)
					printf("Freeing = %d\n", *((int*) ((struct ListNodePtr*) temp_mal->ptr)->ptr_value));
				free(((struct ListNodePtr*) temp_mal->ptr)->ptr_value);

				free(temp_mal->ptr);
				temp_mal->ptr = NULL;

				free(temp_mal);
				temp_mal = NULL;

				total_freed++;
			}
			else
			{
				int iters = 0;
				while (cur_mal->next != NULL)
				{
					if (cur_mal->next->ptr == temp)
					{
						struct malloced_node* temp_mal = cur_mal->next;
						cur_mal->next = cur_mal->next->next;
						
						if (the_debug == YES_DEBUG)
							printf("Freeing = %d\n", *((int*) ((struct ListNodePtr*) temp_mal->ptr)->ptr_value));
						free(((struct ListNodePtr*) temp_mal->ptr)->ptr_value);

						free(temp_mal->ptr);
						temp_mal->ptr = NULL;

						free(temp_mal);
						temp_mal = NULL;

						total_freed++;

						if (the_debug == YES_DEBUG)
							printf("%d iterations\n", iters);

						break;
					}
					cur_mal = cur_mal->next;
					if (the_debug == YES_DEBUG)
						iters++;
				}
			}
		}
	}

	return total_freed;
}


bool equals(void* the_ptr_one, int the_ptr_type, void* the_ptr_two, int ptr_or_value)
{
	if (the_ptr_type == PTR_TYPE_INT || the_ptr_type == PTR_TYPE_REAL)
	{
		return *((int*) the_ptr_one) == *((int*) the_ptr_two);
	}
	else if (the_ptr_type == PTR_TYPE_CHAR)
	{
		return strcmp(the_ptr_one, the_ptr_two) == 0;
	}
	else if (the_ptr_type == PTR_TYPE_TABLE_COLS_INFO)
	{
		if (ptr_or_value == PTR_EQUALS)
			return the_ptr_one = the_ptr_two;
		else
			return strcmp(((struct table_cols_info*) the_ptr_one)->col_name, ((struct table_cols_info*) the_ptr_two)->col_name) == 0 
					&& strcmp(((struct table_cols_info*) the_ptr_one)->home_table->name, ((struct table_cols_info*) the_ptr_two)->home_table->name) == 0;
	}
	else if (the_ptr_type == PTR_TYPE_TABLE_INFO)
	{
		if (ptr_or_value == PTR_EQUALS)
			return the_ptr_one = the_ptr_two;
		else
			return strcmp(((struct table_info*) the_ptr_one)->name, ((struct table_info*) the_ptr_two)->name) == 0;
	}
	else if (the_ptr_type == PTR_TYPE_CHANGE_NODE_V2)
	{
		if (ptr_or_value == PTR_EQUALS)
			return the_ptr_one = the_ptr_two;
	}
	else if (the_ptr_type == PTR_TYPE_WHERE_CLAUSE_NODE)
	{
		if (ptr_or_value == PTR_EQUALS)
			return the_ptr_one = the_ptr_two;
	}
	else if (the_ptr_type == PTR_TYPE_COL_DATA_NODE)
	{
		if (ptr_or_value == PTR_EQUALS)
			return the_ptr_one = the_ptr_two;
	}
	else if (the_ptr_type == PTR_TYPE_FREQUENT_NODE)
	{
		if (ptr_or_value == PTR_EQUALS)
			return the_ptr_one = the_ptr_two;
	}
	else if (the_ptr_type == PTR_TYPE_SELECT_NODE)
	{
		if (ptr_or_value == PTR_EQUALS)
			return the_ptr_one = the_ptr_two;
	}
	else if (the_ptr_type == PTR_TYPE_COL_IN_SELECT_NODE)
	{
		if (ptr_or_value == PTR_EQUALS)
			return the_ptr_one = the_ptr_two;
	}
	else if (the_ptr_type == PTR_TYPE_FUNC_NODE)
	{
		if (ptr_or_value == PTR_EQUALS)
			return the_ptr_one = the_ptr_two;
	}
	else if (the_ptr_type == PTR_TYPE_JOIN_NODE)
	{
		if (ptr_or_value == PTR_EQUALS)
			return the_ptr_one = the_ptr_two;
	}
	else if (the_ptr_type == PTR_TYPE_MATH_NODE)
	{
		if (ptr_or_value == PTR_EQUALS)
			return the_ptr_one = the_ptr_two;
	}
	else if (the_ptr_type == PTR_TYPE_LIST_NODE_PTR)
	{
		if (ptr_or_value == PTR_EQUALS)
			return the_ptr_one = the_ptr_two;
	}

	printf("	ERROR in equals() at line %d in %s\n", __LINE__, __FILE__);
	return false;
}

int freeAnyLinkedList(void** the_head, int the_head_type
					 ,struct file_opened_node** file_opened_head, struct malloced_node** malloced_head, int the_debug)
{
	int total_freed = 0;

	if (the_head_type == PTR_TYPE_TABLE_INFO)
	{
		while (*the_head != NULL)
		{
			struct table_info* temp = (struct table_info*) *the_head;
			*the_head = (void*) ((struct table_info*) (*the_head))->next;

			if (malloced_head == NULL)
			{
				free(temp->name);
				total_freed++;

				total_freed += freeAnyLinkedList((void**) &temp->table_cols_head, PTR_TYPE_TABLE_COLS_INFO, file_opened_head, malloced_head, the_debug);

				free(temp);
				total_freed++;
			}
			else
			{
				myFree((void**) &temp->name, file_opened_head, malloced_head, the_debug);
				total_freed++;

				total_freed += freeAnyLinkedList((void**) &temp->table_cols_head, PTR_TYPE_TABLE_COLS_INFO, file_opened_head, malloced_head, the_debug);

				myFree((void**) &temp, file_opened_head, malloced_head, the_debug);
				total_freed++;
			}
		}
	}
	else if (the_head_type == PTR_TYPE_TABLE_COLS_INFO)
	{
		while (*the_head != NULL)
		{
			struct table_cols_info* temp = (struct table_cols_info*) *the_head;
			*the_head = (void*) ((struct table_cols_info*) (*the_head))->next;

			if (malloced_head == NULL)
			{
				free(temp->col_name);
				total_freed++;

				total_freed += freeAnyLinkedList((void**) &temp->open_list_head, PTR_TYPE_LIST_NODE_PTR, file_opened_head, malloced_head, the_debug);
				total_freed += freeAnyLinkedList((void**) &temp->unique_list_head, PTR_TYPE_FREQUENT_NODE, file_opened_head, malloced_head, the_debug);
				total_freed += freeAnyLinkedList((void**) &temp->frequent_list_head, PTR_TYPE_FREQUENT_NODE, file_opened_head, malloced_head, the_debug);

				free(temp);
				total_freed++;
			}
			else
			{
				myFree((void**) &temp->col_name, file_opened_head, malloced_head, the_debug);
				total_freed++;

				total_freed += freeAnyLinkedList((void**) &temp->open_list_head, PTR_TYPE_LIST_NODE_PTR, file_opened_head, malloced_head, the_debug);
				total_freed += freeAnyLinkedList((void**) &temp->unique_list_head, PTR_TYPE_FREQUENT_NODE, file_opened_head, malloced_head, the_debug);
				total_freed += freeAnyLinkedList((void**) &temp->frequent_list_head, PTR_TYPE_FREQUENT_NODE, file_opened_head, malloced_head, the_debug);

				myFree((void**) &temp, file_opened_head, malloced_head, the_debug);
				total_freed++;
			}
		}
	}
	else if (the_head_type == PTR_TYPE_CHANGE_NODE_V2)
	{

	}
	else if (the_head_type == PTR_TYPE_WHERE_CLAUSE_NODE)
	{
		struct where_clause_node* temp_where = (struct where_clause_node*) *the_head;

		if (temp_where != NULL)
		{
			if (malloced_head == NULL)
			{

			}
			else
			{
				if (temp_where->ptr_one_type == PTR_TYPE_INT || temp_where->ptr_one_type == PTR_TYPE_REAL || temp_where->ptr_one_type == PTR_TYPE_CHAR)
				{
					myFree((void**) &temp_where->ptr_one, file_opened_head, malloced_head, the_debug);
					total_freed++;
				}
				else if (temp_where->ptr_one_type == PTR_TYPE_WHERE_CLAUSE_NODE)
				{
					total_freed += freeAnyLinkedList((void**) &temp_where->ptr_one, PTR_TYPE_WHERE_CLAUSE_NODE, file_opened_head, malloced_head, the_debug);
				}

				if (temp_where->ptr_two_type == PTR_TYPE_INT || temp_where->ptr_two_type == PTR_TYPE_REAL || temp_where->ptr_two_type == PTR_TYPE_CHAR)
				{
					myFree((void**) &temp_where->ptr_two, file_opened_head, malloced_head, the_debug);
					total_freed++;
				}
				else if (temp_where->ptr_two_type == PTR_TYPE_WHERE_CLAUSE_NODE)
				{
					total_freed += freeAnyLinkedList((void**) &temp_where->ptr_two, PTR_TYPE_WHERE_CLAUSE_NODE, file_opened_head, malloced_head, the_debug);
				}

				myFree((void**) &temp_where, file_opened_head, malloced_head, the_debug);
				total_freed++;
			}
		}
	}
	else if (the_head_type == PTR_TYPE_FREQUENT_NODE)
	{
		while (*the_head != NULL)
		{
			struct frequent_node* temp = (struct frequent_node*) *the_head;
			*the_head = (void*) ((struct frequent_node*) (*the_head))->next;

			if (malloced_head == NULL)
			{
				free(temp->ptr_value);
				total_freed++;

				total_freed += freeAnyLinkedList((void**) &temp->row_nums_head, PTR_TYPE_LIST_NODE_PTR, file_opened_head, malloced_head, the_debug);

				free(temp);
				total_freed++;
			}
			else
			{
				myFree((void**) &temp->ptr_value, file_opened_head, malloced_head, the_debug);
				total_freed++;

				total_freed += freeAnyLinkedList((void**) &temp->row_nums_head, PTR_TYPE_LIST_NODE_PTR, file_opened_head, malloced_head, the_debug);

				myFree((void**) &temp, file_opened_head, malloced_head, the_debug);
				total_freed++;
			}
		}
	}
	else if (the_head_type == PTR_TYPE_SELECT_NODE || the_head_type == PTR_TYPE_SELECT_NODE_BUT_IN_JOIN)
	{
		while (*the_head != NULL)
		{
			struct select_node* temp;

			if (the_head_type == PTR_TYPE_SELECT_NODE)
			{
				temp = (struct select_node*) *the_head;
				*the_head = (void*) ((struct select_node*) (*the_head))->next;
			}
			else //if (the_head_type == PTR_TYPE_SELECT_NODE_BUT_IN_JOIN)
			{
				temp = (struct select_node*) *the_head;
				*the_head = (void*) ((struct select_node*) (*the_head))->prev;
			}


			if (malloced_head == NULL)
			{
				if (temp->select_node_alias != NULL)
				{
					//printf("Freeing select_node_alias: _%s_\n", temp->select_node_alias);
					free(temp->select_node_alias);
					total_freed++;
				}
					
				for (int i=0; i<temp->columns_arr_size && temp->columns_arr != NULL; i++)
				{
					struct col_in_select_node* temp_col = temp->columns_arr[i];

					if (temp_col->new_name != NULL)
					{
						//printf("Freeing temp_col->new_name: _%s_\n", temp_col->new_name);
						free(temp_col->new_name);
						total_freed++;
					}

					if (temp_col->func_node != NULL)
					{
						total_freed += freeAnyLinkedList((void**) &temp_col->func_node, PTR_TYPE_FUNC_NODE, file_opened_head, malloced_head, the_debug);
					}

					if (temp_col->math_node != NULL)
					{
						total_freed += freeAnyLinkedList((void**) &temp_col->math_node, PTR_TYPE_MATH_NODE, file_opened_head, malloced_head, the_debug);
					}

					//printf("Freeing temp->columns_arr[i]: _%s_\n", ((struct table_cols_info*) temp->columns_arr[i]->col_ptr)->col_name);
					free(temp->columns_arr[i]);
					total_freed++;

					if (i == temp->columns_arr_size-1)
					{
						//printf("Freeing temp->columns_arr\n");
						free(temp->columns_arr);
						total_freed++;
					}
				}

				if (temp->where_head != NULL)
				{

				}

				if (temp->join_head != NULL)
				{
					//total_freed += freeAnyLinkedList((void**) &temp->table_cols_head, PTR_TYPE_TABLE_COLS_INFO, file_opened_head, malloced_head, the_debug);
				}

				//printf("Freeing temp\n");
				free(temp);
				total_freed++;
			}
			else
			{
				if (temp->select_node_alias != NULL)
				{
					//printf("Freeing select_node_alias: _%s_\n", temp->select_node_alias);
					myFree((void**) &temp->select_node_alias, file_opened_head, malloced_head, the_debug);
					total_freed++;
				}
					
				for (int i=0; i<temp->columns_arr_size && temp->columns_arr != NULL; i++)
				{
					struct col_in_select_node* temp_col = temp->columns_arr[i];

					if (temp_col->new_name != NULL)
					{
						//printf("Freeing temp_col->new_name: _%s_\n", temp_col->new_name);
						myFree((void**) &temp_col->new_name, file_opened_head, malloced_head, the_debug);
						total_freed++;
					}

					if (temp_col->func_node != NULL)
					{
						total_freed += freeAnyLinkedList((void**) &temp_col->func_node, PTR_TYPE_FUNC_NODE, file_opened_head, malloced_head, the_debug);
					}

					if (temp_col->math_node != NULL)
					{
						total_freed += freeAnyLinkedList((void**) &temp_col->math_node, PTR_TYPE_MATH_NODE, file_opened_head, malloced_head, the_debug);
					}

					//printf("Freeing temp->columns_arr[i]: _%s_\n", ((struct table_cols_info*) temp->columns_arr[i]->col_ptr)->col_name);
					myFree((void**) &temp->columns_arr[i], file_opened_head, malloced_head, the_debug);
					total_freed++;

					if (i == temp->columns_arr_size-1)
					{
						//printf("Freeing temp->columns_arr\n");
						myFree((void**) &temp->columns_arr, file_opened_head, malloced_head, the_debug);
						total_freed++;
					}
				}

				if (temp->where_head != NULL)
				{
					total_freed += freeAnyLinkedList((void**) &temp->where_head, PTR_TYPE_WHERE_CLAUSE_NODE, file_opened_head, malloced_head, the_debug);
				}

				if (temp->join_head != NULL)
				{
					total_freed += freeAnyLinkedList((void**) &temp->join_head, PTR_TYPE_JOIN_NODE, file_opened_head, malloced_head, the_debug);
				}

				//printf("Freeing temp\n");
				myFree((void**) &temp, file_opened_head, malloced_head, the_debug);
				total_freed++;
			}
		}
	}
	else if (the_head_type == PTR_TYPE_COL_IN_SELECT_NODE)
	{
		
	}
	else if (the_head_type == PTR_TYPE_FUNC_NODE)
	{
		struct func_node* temp_func = (struct func_node*) *the_head;

		if (malloced_head == NULL)
		{
			for (int j=0; j<temp_func->args_size; j++)
			{
				free(temp_func->args_arr[j]);
				total_freed++;
			}

			free(temp_func->args_arr);
			total_freed++;

			free(temp_func->args_arr_type);
			total_freed++;

			while (temp_func->group_by_cols_head != NULL)
			{
				struct ListNodePtr* temp_list_node_ptr = temp_func->group_by_cols_head;
				temp_func->group_by_cols_head = temp_func->group_by_cols_head->next;

				// Dont free the ptr_value bc that points to a col_in_select_node which is freed elsewhere
				free(temp_list_node_ptr);
			}

			free(temp_func);
			total_freed++;
		}
		else
		{
			for (int j=0; j<temp_func->args_size; j++)
			{
				if (temp_func->args_arr_type[j] != PTR_TYPE_COL_IN_SELECT_NODE)
				{
					myFree((void**) &temp_func->args_arr[j], file_opened_head, malloced_head, the_debug);
					total_freed++;
				}
			}

			myFree((void**) &temp_func->args_arr, file_opened_head, malloced_head, the_debug);
			total_freed++;

			myFree((void**) &temp_func->args_arr_type, file_opened_head, malloced_head, the_debug);
			total_freed++;

			while (temp_func->group_by_cols_head != NULL)
			{
				struct ListNodePtr* temp_list_node_ptr = temp_func->group_by_cols_head;
				temp_func->group_by_cols_head = temp_func->group_by_cols_head->next;

				// Dont free the ptr_value bc that points to a col_in_select_node which is freed elsewhere
				myFree((void**) &temp_list_node_ptr, file_opened_head, malloced_head, the_debug);
			}

			myFree((void**) &temp_func, file_opened_head, malloced_head, the_debug);
			total_freed++;
		}
	}
	else if (the_head_type == PTR_TYPE_JOIN_NODE)
	{
		while (*the_head != NULL)
		{
			struct join_node* temp = (struct join_node*) *the_head;
			*the_head = (void*) ((struct join_node*) (*the_head))->prev;

			if (malloced_head == NULL)
			{

			}
			else
			{
				if (temp->on_clause_head != NULL)
				{
					total_freed += freeAnyLinkedList((void**) &temp->on_clause_head, PTR_TYPE_WHERE_CLAUSE_NODE, file_opened_head, malloced_head, the_debug);
				}

				if (temp->select_joined != NULL)
				{
					total_freed += freeAnyLinkedList((void**) &temp->select_joined, PTR_TYPE_SELECT_NODE_BUT_IN_JOIN, file_opened_head, malloced_head, the_debug);
				}

				myFree((void**) &temp, file_opened_head, malloced_head, the_debug);
				total_freed++;
			}
		}
	}
	else if (the_head_type == PTR_TYPE_MATH_NODE)
	{
		struct math_node* temp_math = (struct math_node*) *the_head;

		if (temp_math != NULL)
		{
			if (malloced_head == NULL)
			{

			}
			else
			{
				if (temp_math->ptr_one_type == PTR_TYPE_INT || temp_math->ptr_one_type == PTR_TYPE_REAL || temp_math->ptr_one_type == PTR_TYPE_CHAR)
				{
					myFree((void**) &temp_math->ptr_one, file_opened_head, malloced_head, the_debug);
					total_freed++;
				}
				else if (temp_math->ptr_one_type == PTR_TYPE_MATH_NODE)
				{
					total_freed += freeAnyLinkedList((void**) &temp_math->ptr_one, PTR_TYPE_MATH_NODE, file_opened_head, malloced_head, the_debug);
				}

				if (temp_math->ptr_two_type == PTR_TYPE_INT || temp_math->ptr_two_type == PTR_TYPE_REAL || temp_math->ptr_two_type == PTR_TYPE_CHAR)
				{
					myFree((void**) &temp_math->ptr_two, file_opened_head, malloced_head, the_debug);
					total_freed++;
				}
				else if (temp_math->ptr_two_type == PTR_TYPE_MATH_NODE)
				{
					total_freed += freeAnyLinkedList((void**) &temp_math->ptr_two, PTR_TYPE_MATH_NODE, file_opened_head, malloced_head, the_debug);
				}

				myFree((void**) &temp_math, file_opened_head, malloced_head, the_debug);
				total_freed++;
			}
		}
	}
	else if (the_head_type == PTR_TYPE_LIST_NODE_PTR)
	{
		while (*the_head != NULL)
		{
			struct ListNodePtr* temp = (struct ListNodePtr*) *the_head;
			*the_head = (void*) ((struct ListNodePtr*) (*the_head))->next;

			//printf("temp ptr_value = _%d_\n", *((int*) temp->ptr_value));

			if (malloced_head == NULL)
			{
				if (temp->ptr_value != NULL)
				{
					free(temp->ptr_value);
					total_freed++;
				}
				free(temp);
				total_freed++;
			}
			else
			{
				if (temp->ptr_value != NULL)
				{	
					myFree((void**) &temp->ptr_value, file_opened_head, malloced_head, the_debug);
					total_freed++;
				}
				myFree((void**) &temp, file_opened_head, malloced_head, the_debug);
				total_freed++;
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