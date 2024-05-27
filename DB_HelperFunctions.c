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

/*	RETURNS:
 *  Integer greater than -1
 *
 *	WRITES TO:
 */
int strLength(char* str)
{
	int index = 0;
	while (str[index] != 0)
		index++;
	
	return index;
}

/*	RETURNS:
 *  FALSE if str does not contain the_char
 *	TRUE if str contains the_char
 *
 *	WRITES TO:
 */
bool strcontains(char* str, char the_char)
{
	int index = 0;
	while (str[index] != 0)
	{
		if (str[index] == the_char)
			return true;
		index++;
	}

	return false;
}

/*	RETURNS:
 *  FALSE if str does not contain find_this
 *	TRUE if str contains find_this
 *
 *	WRITES TO:
 */
bool strContainsWordUpper(char* str, char* find_this)
{
	int index = 0;
	while (str[index + strLength(find_this)] != 0)
	{
		for (int i=index; i<index + strLength(find_this); i++)
		{
			//printf("%c vs %c\n", toupper(str[i]), toupper(find_this[i - index]));
			if (toupper(str[i]) != toupper(find_this[i - index]))
				break;

			if (i == (index + strLength(find_this))-1)
				return true;
		}

		index++;
	}
	return false;
}

/*	RETURNS:
 *  Integer greater than -1 if str contains the_char
 *	RETURN_NORMAL_NEG if str does not contain the_char
 *
 *	WRITES TO:
 */
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

	return RETURN_NORMAL_NEG;
}

/*	RETURNS:
 *  NULL if error
 *	Valid char* ptr if good
 *
 *	WRITES TO:
 */
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

/*	RETURNS:
 *  NULL if error
 *	Valid char** ptr if good
 *
 *	WRITES TO:
 */
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
			if (index_malloced == NULL)
			{
				if (the_debug == YES_DEBUG)
					printf("	ERROR in strSplitV2() at line %d in %s\n", __LINE__, __FILE__);
				return NULL;
			}
			*index_malloced = index;

			if (addListNodePtr(&index_list_head, &index_list_tail, index_malloced, PTR_TYPE_INT, ADDLISTNODE_TAIL
							  ,file_opened_head, malloced_head, the_debug) != RETURN_GOOD)
			{
				if (the_debug == YES_DEBUG)
					printf("	ERROR in strSplitV2() at line %d in %s\n", __LINE__, __FILE__);
				return NULL;
			}
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

	if (index_list_head != NULL)
	{
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
			if (result[i] == NULL)
			{
				if (the_debug == YES_DEBUG)
					printf("	ERROR in strSplitV2() at line %d in %s\n", __LINE__, __FILE__);
				return NULL;
			}

			if (i > 0)
				cur = cur->next;
		}
	}
	else
	{
		result[0] = myMalloc(sizeof(char) * (strLength(str)+1), file_opened_head, malloced_head, the_debug);
		if (result[0] == NULL)
		{
			if (the_debug == YES_DEBUG)
				printf("	ERROR in strSplit() at line %d in %s\n", __LINE__, __FILE__);
			return NULL;
		}

		strcpy(result[0], str);
	}

	int freed = freeAnyLinkedList((void**) &index_list_head, PTR_TYPE_LIST_NODE_PTR, file_opened_head, malloced_head, the_debug);
	//printf("freed = %d\n", freed);

	return result;
}

/*	RETURNS:
 *  NULL if error
 *	Valid char* ptr if good
 *
 *	WRITES TO:
 */
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

/*	RETURNS:
 *  RETURN_NORMAL_NEG if word is empty
 *	RETURN_GOOD if good
 *
 *	WRITES TO:
 *	word
 *	cur_index
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
	else if (input[*cur_index] == 34 /*Double quote*/)
	{
		for (int i=0; i<2; i++)
		{
			word[word_index] = input[*cur_index];
			word[word_index+1] = 0;

			word_index++;
			(*cur_index)++;
		}

		// START Iterate until find another double quote or 0
		bool two_single_quotes = false;
		while (two_single_quotes || !(input[*cur_index] == 0 || (input[(*cur_index)-1] == 34 && input[*cur_index] != 34)))
		{
			if (two_single_quotes)
				two_single_quotes = false;
			if (input[(*cur_index)-1] == 34 && input[*cur_index] == 34)
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
		// END Iterate until find another double quote, 0, ;, or comma
	}
	else
	{
		// START Iterate until one of the below characters
		while (input[*cur_index] != 0 && input[*cur_index] != ' ' && input[*cur_index] != ',' && input[*cur_index] != ';'
			   && input[*cur_index] != '(' && input[*cur_index] != ')' && input[*cur_index] != '.'
			   && input[*cur_index] != '\t' && input[*cur_index] != '\n' && input[*cur_index] != '\v'
			   && input[*cur_index] != '+' && input[*cur_index] != '*' && input[*cur_index] != '/' && input[*cur_index] != '^' && input[*cur_index] != '-')
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
			|| input[*cur_index] == '+' || input[*cur_index] == '*' || input[*cur_index] == '/' || input[*cur_index] == '^' || input[*cur_index] == '-')
		{
			word[word_index] = input[*cur_index];
			word[word_index+1] = 0;

			word_index++;
			(*cur_index)++;
		}
		else if (input[*cur_index] == '.')
			return RETURN_GOOD;
		else
			return RETURN_NORMAL_NEG;
		// END If empty string, but cur_index at one of the following characters, make word that character
	}

	return RETURN_GOOD;
}

/*	RETURNS:
 *  RETURN_NORMAL_NEG if word is less than test_char
 *	RETURN_NORMAL_POS if word is greater than test_char
 *	RETURN_GOOD if matches
 *
 *	WRITES TO:
 */
int strcmp_Upper(char* word, char* test_char)
{
	const unsigned char *s1 = (const unsigned char *) word;
	const unsigned char *s2 = (const unsigned char *) test_char;
	unsigned char c1, c2;

	do
	{
		c1 = toupper((unsigned char) *s1++);
		c2 = toupper((unsigned char) *s2++);
		if (c1 == '\0')
			return c1 - c2;
	}
	while (c1 == c2);

	if (c1 - c2 > 0)
		return RETURN_NORMAL_POS;
	if (c1 - c2 < 0)
		return RETURN_NORMAL_NEG;

	return RETURN_GOOD;
}

/*	RETURNS:
 *  RETURN_GOOD
 *
 *	WRITES TO:
 *	str
 */
int trimStr(char* str)
{
	while (str[0] == ' ')
	{
		for (int i=1; i<strLength(str)-1; i++)
		{
			str[i-1] = str[i];
		}
	}

	int str_len = strLength(str);

	while (str[str_len-1] == ' ')
	{
		str[str_len-1] = 0;

		str_len--;
	}

	if (str[0] == 39 || str[0] == 34)
	{
		for (int i=1; i<strLength(str)-1; i++)
		{
			str[i-1] = str[i];
		}

		str_len = strLength(str);

		str[str_len-2] = 0;
	}

	return RETURN_GOOD;
}

/*	RETURNS:
 *  RETURN_GOOD
 *
 *	WRITES TO:
 *	word
 */
int redoDoubleQuotes(char* word)
{
	if (word[0] == 0 || word[1] == 0 || word[2] == 0)
		return RETURN_GOOD;

	int index=1;
	while (word[index+1] != 0)
	{
		if (word[index] == 39)
		{
			for (int i=strLength(word); i>index; i--)
			{
				word[i+1] = word[i];
			}
			word[index+1] = 39;

			break;
		}
		index++;
	}

	return RETURN_GOOD;
}

/*	RETURNS:
 *  RETURN_GOOD
 *
 *	WRITES TO:
 *	str
 */
int strReplace(char* str, char* old, char* new)
{
	int index = 0;
	while (str[index] != 0)
	{
		if (str[index] == old[0] && old[1] == 0)
		{
			int temp_index = strLength(str)-1;
			int new_str_len = strLength(new);

			str[temp_index+new_str_len] = 0;
			//printf("Made index = %d zero\n", temp_index+new_str_len);
			while (temp_index > index)
			{
				str[temp_index+new_str_len-1] = str[temp_index];
				temp_index--;
			}

			//printf("str here 1 = \"%s\"\n", str);

			int count = 0;
			while (count < new_str_len)
			{
				str[index] = new[count];
				count++;
				index++;
			}

			//printf("str here 2 = \"%s\"\n", str);

			break;
		}
		index++;
	}

	return RETURN_GOOD;
}

/*	RETURNS:
 *  NULL if error
 *	Valid void* if good
 *
 *	WRITES TO:
 */
void* myMalloc(size_t size, struct file_opened_node** file_opened_head, struct malloced_node** malloced_head, int the_debug)
{
	// START New and fast
		void* new_ptr = (void*) malloc(sizeof(void*) + size);
		if (new_ptr == NULL)
		{
			if (the_debug == YES_DEBUG)
				printf("	ERROR in myMalloc() at line %d in %s\n", __LINE__, __FILE__);
			printf("There was a problem allocating memory. Please try again.\n");
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
				printf("There was a problem allocating memory. Please try again.\n");
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
					printf("	ERROR in myMalloc() at line %d in %s\n", __LINE__, __FILE__);
				printf("There was a problem allocating memory. Please try again.\n");
				errorTeardown(file_opened_head, malloced_head, the_debug);
				return NULL;
			}
			temp->ptr = new_ptr;
			temp->next = *malloced_head;
			temp->prev = NULL;
			(*malloced_head)->prev = temp;
			*malloced_head = temp;
		}


		((void**) new_ptr)[0] = *malloced_head;
		//printf("+ %p\n", ((void**) new_ptr)[0]);
		new_ptr += 8;

		return new_ptr;
	// END New and fast
}

/*	RETURNS:
 *  RETURN_GOOD
 *
 *	WRITES TO:
 */
int myFree(void** old_ptr, struct file_opened_node** file_opened_head, struct malloced_node** malloced_head, int the_debug)
{
	if (*old_ptr == NULL)
		return RETURN_GOOD;


	if (malloced_head == NULL)
	{
		void* ptr = *old_ptr;
		ptr -= 8;
		free(ptr);
	}
	else
	{
		// START New and fast
			(*old_ptr) -= 8;

			struct malloced_node* the_node = (struct malloced_node*) ((void**) *old_ptr)[0];
			//printf("- %p\n", the_node);

			if (*malloced_head == the_node)
			{
				*malloced_head = (*malloced_head)->next;

				if (*malloced_head != NULL)
					(*malloced_head)->prev = NULL;
			}
			else
			{
				if (the_node->prev != NULL)
					the_node->prev->next = the_node->next;
				if (the_node->next != NULL)
					the_node->next->prev = the_node->prev;
			}

			free(the_node->ptr);
			free(the_node);

			*old_ptr = NULL;
		// END New and fast
	}

	return RETURN_GOOD;
}

/*	Frees all nodes in malloced_head, and frees the ptr
 *	
 *	RETURNS:
 *  Number of things freed
 *
 *	WRITES TO:
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
 *	
 *	RETURNS:
 *  Number of things freed
 *
 *	WRITES TO:
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
 *	
 *	RETURNS:
 *  RETURN_GOOD
 *
 *	WRITES TO:
 */
int myFreeJustNode(void** old_ptr, struct file_opened_node** file_opened_head, struct malloced_node** malloced_head, int the_debug)
{
	void* ptr = *old_ptr;
	ptr -= 8;

	if ((*malloced_head)->ptr == ptr)
	{
		struct malloced_node* temp = *malloced_head;
		*malloced_head = (*malloced_head)->next;

		// DONT DO THESE because don't want to free pointer
		//free(temp->ptr);
		//temp->ptr = NULL;

		free(temp);
	}
	else
	{
		struct malloced_node* cur = *malloced_head;
		while (cur->next != NULL)
		{
			if (cur->next->ptr == ptr)
			{
				struct malloced_node* temp = cur->next;
				cur->next = cur->next->next;
				
				// DONT DO THESE because don't want to free pointer
				//free(temp->ptr);
				//temp->ptr = NULL;

				free(temp);
				
				break;
			}
			cur = cur->next;
		}
	}

	return RETURN_GOOD;
}

/*	RETURNS:
 *  RETURN_GOOD
 *
 *	WRITES TO:
 *	new_filename
 */
int concatFileName(char* new_filename, char* filetype, int_8 table_num, int_8 col_num)
{
	strcpy(new_filename, "DB_Files_2/DB");

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

	return RETURN_GOOD;
}

/*	RETURNS:
 *  NULL if error
 *	Valid FILE* if good
 *
 *	WRITES TO:
 */
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

/*	RETURNS:
 *  NULL if error
 *	Valid FILE* if good
 *
 *	WRITES TO:
 */
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

/*	RETURNS:
 *  RETURN_GOOD
 *
 *	WRITES TO:
 */
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
	
	return RETURN_GOOD;
}

/*	RETURNS:
 *  Number of files closed
 *
 *	WRITES TO:
 */
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

/*	RETURNS:
 *  NULL if error
 *	Valid char* if good
 *
 *	WRITES TO:
 */
char* intToDate(int_8 the_int_form, struct file_opened_node** file_opened_head, struct malloced_node** malloced_head, int the_debug)
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

	char* date = NULL;
	if (malloced_head != NULL)
		date = (char*) myMalloc(sizeof(char) * 20, file_opened_head, malloced_head, the_debug);
	else
		date = malloc(sizeof(char) * 20);
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

/*	RETURNS:
 *  0 if error
 *	Valid int_8 if good
 *
 *	WRITES TO:
 */
int_8 dateToInt(char* the_date_form)
{
	int year = -1;
	int month = -1;
	int day = -1;
	
	sscanf(the_date_form, "%d/%d/%d", &month, &day, &year);

	if (year == -1 || month == -1 || day == -1)
	{
		//printf("	ERROR in dateToInt() at line %d in %s\n", __LINE__, __FILE__);
		//printf("	Input = _%s_\n", the_date_form);
		return 0;
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

/*	RETURNS:
 *  FALSE if char* is not date
 *	TRUE if char* is date
 *
 *	WRITES TO:
 */
bool isDate(char* the_date_form)
{
	int year = -1;
	int month = -1;
	int day = -1;
	
	sscanf(the_date_form, "%d/%d/%d", &month, &day, &year);

	if (year > 0
		&& month > 0 && month <= 12
		&& day > 0 && day <=31)
		return true;

	return false;
}

/*	RETURNS:
 *  NULL if error
 *	Valid char* if good
 *
 *	WRITES TO:
 */
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

/*	RETURNS:
 *  NULL if error
 *	Valid char* if good
 *
 *	WRITES TO:
 */
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

/*	RETURNS:
 *  -1 if error
 *	Valid int_8 if good
 *
 *	WRITES TO:
 */
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

/*	RETURNS:
 *  -1.0 if error
 *	Valid double if good
 *
 *	WRITES TO:
 */
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

/*	RETURNS:
 *  RETURN_ERROR if error
 *	RETURN_GOOD if good
 *
 *	WRITES TO:
 *	file
 */
int writeFileChar(FILE* file, int_8 offset, char* data
				 ,struct file_opened_node** file_opened_head, struct malloced_node** malloced_head, int the_debug)
{
	int num_bytes = 32;

	char* new_data = (char*) myMalloc(sizeof(char) * num_bytes, file_opened_head, malloced_head, the_debug);
	if (new_data == NULL)
	{
		if (the_debug == YES_DEBUG)
			printf("	ERROR in writeFileChar() at line %d in %s\n", __LINE__, __FILE__);
		return RETURN_ERROR;
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
		return RETURN_ERROR;
	}

	myFree((void**) &new_data, file_opened_head, malloced_head, the_debug);

	fflush(file);

	return RETURN_GOOD;
}

/*	RETURNS:
 *  RETURN_ERROR if error
 *	RETURN_GOOD if good
 *
 *	WRITES TO:
 *	file
 */
int writeFileCharData(FILE* file, int_8 offset, int_8* the_num_bytes, char* data
					 ,struct file_opened_node** file_opened_head, struct malloced_node** malloced_head, int the_debug)
{
	int num_bytes = *the_num_bytes;

	char* new_data = (char*) myMalloc(sizeof(char) * num_bytes, file_opened_head, malloced_head, the_debug);
	if (new_data == NULL)
	{
		if (the_debug == YES_DEBUG)
			printf("	ERROR in writeFileCharData() at line %d in %s\n", __LINE__, __FILE__);
		return RETURN_ERROR;
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
		return RETURN_ERROR;
	}

	myFree((void**) &new_data, file_opened_head, malloced_head, the_debug);

	fflush(file);

	return RETURN_GOOD;
}

/*	RETURNS:
 *  RETURN_ERROR if error
 *	RETURN_GOOD if good
 *
 *	WRITES TO:
 *	file
 */
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
		return RETURN_ERROR;
	}

	fflush(file);

	return RETURN_GOOD;
}

/*	RETURNS:
 *  RETURN_ERROR if error
 *	RETURN_GOOD if good
 *
 *	WRITES TO:
 *	file
 */
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
		return RETURN_ERROR;
	}

	fflush(file);

	return RETURN_GOOD;
}

/*	RETURNS:
 *  RETURN_ERROR if error
 *	RETURN_GOOD if good
 *
 *	WRITES TO:
 *	the_head
 *	the_tail
 */
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
		return RETURN_ERROR;
	}
	temp_new->ptr_value = the_ptr;
	temp_new->ptr_type = the_ptr_type;

	if (the_head != NULL && *the_head == NULL)
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

	return RETURN_GOOD;
}

/*	RETURNS:
 *  RETURN_ERROR if error
 *	RETURN_GOOD if good
 *
 *	WRITES TO:
 *	the_head
 *	the_tail
 */
int addListNodePtr_Int(struct ListNodePtr** the_head, struct ListNodePtr** the_tail, int value, int the_add_mode
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
		return RETURN_ERROR;
	}

	temp_new->ptr_value = myMalloc(sizeof(int), file_opened_head, malloced_head, the_debug);
	if (temp_new->ptr_value == NULL)
	{
		if (the_debug == YES_DEBUG)
			printf("	ERROR in addListNode() at line %d in %s\n", __LINE__, __FILE__);
		return RETURN_ERROR;
	}

	*((int*) temp_new->ptr_value) = value;
	temp_new->ptr_type = PTR_TYPE_INT;

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

	return RETURN_GOOD;
}

/*	RETURNS:
 *  Valid void* if found
 *	NULL if not found
 *
 *	WRITES TO:
 */
void* removeListNodePtr(struct ListNodePtr** the_head, struct ListNodePtr** the_tail, void* the_ptr, int the_ptr_type, int the_remove_mode
					   ,struct file_opened_node** file_opened_head, struct malloced_node** malloced_head, int the_debug)
{
	void* temp_ptr_value = NULL;
	struct ListNodePtr* temp;

	if (the_remove_mode == REMOVELISTNODE_HEAD || (the_ptr_type == PTR_TYPE_LIST_NODE_PTR && *the_head == the_ptr) || (the_ptr != NULL && the_ptr_type != PTR_TYPE_LIST_NODE_PTR && equals((*the_head)->ptr_value, the_ptr_type, the_ptr, VALUE_EQUALS)))
	{
		temp = *the_head;
		temp_ptr_value = temp->ptr_value;

		if (*the_head == *the_tail)
			*the_tail = NULL;

		*the_head = (*the_head)->next;
		if (*the_head != NULL)
			(*the_head)->prev = NULL;
	}
	else if (the_remove_mode == REMOVELISTNODE_TAIL || (the_ptr_type == PTR_TYPE_LIST_NODE_PTR && *the_tail == the_ptr) || (the_ptr != NULL && the_ptr_type != PTR_TYPE_LIST_NODE_PTR && equals((*the_tail)->ptr_value, the_ptr_type, the_ptr, VALUE_EQUALS)))
	{
		temp = *the_tail;
		temp_ptr_value = temp->ptr_value;

		if (*the_head == *the_tail)
			*the_head = NULL;

		*the_tail = (*the_tail)->prev;
		if (*the_tail != NULL)
			(*the_tail)->next = NULL;
	}
	else if (the_ptr_type == PTR_TYPE_LIST_NODE_PTR)
	{
		temp = the_ptr;

		temp->prev->next = temp->next;
		temp->next->prev = temp->prev;
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

	myFree((void**) &temp->ptr_value, file_opened_head, malloced_head, the_debug);
	myFree((void**) &temp, file_opened_head, malloced_head, the_debug);

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
		if (cur->ptr_type == PTR_TYPE_LIST_NODE_PTR)
		{
			traverseListNodesPtr((struct ListNodePtr**) &cur->ptr_value, NULL, TRAVERSELISTNODES_HEAD, "Rows match: ");
		}

		if (cur->ptr_type == PTR_TYPE_INT)
		{
			printf("%d", *((int*) cur->ptr_value));
		}
		else if (cur->ptr_type == PTR_TYPE_CHAR)
		{
			printf("%s", (char*) cur->ptr_value);
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

	return RETURN_GOOD;
}

/*	RETURNS:
 *  Valid struct ListNodePtr* if found
 *	NULL if not found
 *
 *	WRITES TO:
 */
struct ListNodePtr* inListNodePtrList(struct ListNodePtr** the_head, struct ListNodePtr** the_tail, void* the_ptr, int the_ptr_type)
{
	struct ListNodePtr* cur = *the_head;
	while (cur != NULL)
	{
		if (equals(cur->ptr_value, the_ptr_type, the_ptr, VALUE_EQUALS))
			return cur;
		cur = cur->next;
	}

	return NULL;
}

/*	RETURNS:
 *  TRUE if matches
 *	FALSE if does not match
 *
 *	WRITES TO:
 */
bool equals(void* the_ptr_one, int the_ptr_type, void* the_ptr_two, int ptr_or_value)
{
	if (the_ptr_one == NULL && the_ptr_two == NULL)
		return true;
	else if (the_ptr_one == NULL || the_ptr_two == NULL)
		return false;

	if (the_ptr_type == PTR_TYPE_INT)
	{
		return *((int*) the_ptr_one) == *((int*) the_ptr_two);
	}
	else if (the_ptr_type == PTR_TYPE_REAL)
	{
		return *((double*) the_ptr_one) == *((double*) the_ptr_two);
	}
	else if (the_ptr_type == PTR_TYPE_CHAR || the_ptr_type == PTR_TYPE_DATE)
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

	//printf("	ERROR in equals() at line %d in %s\n", __LINE__, __FILE__);
	return false;
}

/*	RETURNS:
 *  TRUE if matches
 *	FALSE if does not match
 *
 *	WRITES TO:
 */
bool greatLess(void* the_ptr_one, int the_ptr_type, void* the_ptr_two, int where_type)
{
	if (the_ptr_type == PTR_TYPE_INT)
	{
		if (where_type == WHERE_GREATER_THAN)
		{
			return *((int*) the_ptr_one) > *((int*) the_ptr_two);
		}
		else if (where_type == WHERE_GREATER_THAN_OR_EQUAL)
		{
			return *((int*) the_ptr_one) >= *((int*) the_ptr_two);
		}
		else if (where_type == WHERE_LESS_THAN)
		{
			return *((int*) the_ptr_one) < *((int*) the_ptr_two);
		}
		else if (where_type == WHERE_LESS_THAN_OR_EQUAL)
		{
			return *((int*) the_ptr_one) <= *((int*) the_ptr_two);
		}
	}
	else if (the_ptr_type == PTR_TYPE_REAL)
	{
		if (where_type == WHERE_GREATER_THAN)
		{
			return *((double*) the_ptr_one) > *((double*) the_ptr_two);
		}
		else if (where_type == WHERE_GREATER_THAN_OR_EQUAL)
		{
			return *((double*) the_ptr_one) >= *((double*) the_ptr_two);
		}
		else if (where_type == WHERE_LESS_THAN)
		{
			return *((double*) the_ptr_one) < *((double*) the_ptr_two);
		}
		else if (where_type == WHERE_LESS_THAN_OR_EQUAL)
		{
			return *((double*) the_ptr_one) <= *((double*) the_ptr_two);
		}
	}
	else if (the_ptr_type == PTR_TYPE_DATE)
	{
		if (where_type == WHERE_GREATER_THAN)
		{
			return dateToInt(the_ptr_one) > dateToInt(the_ptr_two);
		}
		else if (where_type == WHERE_GREATER_THAN_OR_EQUAL)
		{
			return dateToInt(the_ptr_one) >= dateToInt(the_ptr_two);
		}
		else if (where_type == WHERE_LESS_THAN)
		{
			return dateToInt(the_ptr_one) < dateToInt(the_ptr_two);
		}
		else if (where_type == WHERE_LESS_THAN_OR_EQUAL)
		{
			return dateToInt(the_ptr_one) <= dateToInt(the_ptr_two);
		}
	}

	//printf("	ERROR in greatLess() at line %d in %s\n", __LINE__, __FILE__);
	return false;
}

/*	RETURNS:
 *  Number of things freed
 *
 *	WRITES TO:
 */
int freeAnyLinkedList(void** the_head, int the_head_type, struct file_opened_node** file_opened_head, struct malloced_node** malloced_head, int the_debug)
{
	int total_freed = 0;

	if (the_head_type == PTR_TYPE_TABLE_INFO)
	{
		while (*the_head != NULL)
		{
			struct table_info* temp = (struct table_info*) *the_head;
			*the_head = (void*) ((struct table_info*) (*the_head))->next;

			myFree((void**) &temp->name, file_opened_head, malloced_head, the_debug);
			total_freed++;

			myFree((void**) &temp->keyword, file_opened_head, malloced_head, the_debug);
			total_freed++;

			total_freed += freeAnyLinkedList((void**) &temp->table_cols_head, PTR_TYPE_TABLE_COLS_INFO, file_opened_head, malloced_head, the_debug);

			myFree((void**) &temp, file_opened_head, malloced_head, the_debug);
			total_freed++;
		}
	}
	else if (the_head_type == PTR_TYPE_TABLE_COLS_INFO)
	{
		while (*the_head != NULL)
		{
			struct table_cols_info* temp = (struct table_cols_info*) *the_head;
			*the_head = (void*) ((struct table_cols_info*) (*the_head))->next;

			myFree((void**) &temp->col_name, file_opened_head, malloced_head, the_debug);
			total_freed++;

			total_freed += freeAnyLinkedList((void**) &temp->open_list_head, PTR_TYPE_LIST_NODE_PTR, file_opened_head, malloced_head, the_debug);
			total_freed += freeAnyLinkedList((void**) &temp->unique_list_head, PTR_TYPE_FREQUENT_NODE, file_opened_head, malloced_head, the_debug);
			total_freed += freeAnyLinkedList((void**) &temp->frequent_list_head, PTR_TYPE_FREQUENT_NODE, file_opened_head, malloced_head, the_debug);

			if (temp->frequent_arr_row_to_node != NULL)
			{
				myFree((void**) &temp->frequent_arr_row_to_node, file_opened_head, malloced_head, the_debug);
				total_freed++;
			}

			myFree((void**) &temp, file_opened_head, malloced_head, the_debug);
			total_freed++;
		}
	}
	else if (the_head_type == PTR_TYPE_CHANGE_NODE_V2)
	{
		if (((struct change_node_v2*) (*the_head))->col_list_head != NULL)
			freeAnyLinkedList((void**) &((struct change_node_v2*) (*the_head))->col_list_head, PTR_TYPE_LIST_NODE_PTR, file_opened_head, malloced_head, the_debug);

		if (((struct change_node_v2*) (*the_head))->data_list_head != NULL)
			freeAnyLinkedList((void**) &((struct change_node_v2*) (*the_head))->data_list_head, PTR_TYPE_LIST_NODE_PTR, file_opened_head, malloced_head, the_debug);

		if (((struct change_node_v2*) (*the_head))->where_head != NULL)
			freeAnyLinkedList((void**) &((struct change_node_v2*) (*the_head))->where_head, PTR_TYPE_WHERE_CLAUSE_NODE, file_opened_head, malloced_head, the_debug);

		myFree(the_head, file_opened_head, malloced_head, the_debug);
	}
	else if (the_head_type == PTR_TYPE_WHERE_CLAUSE_NODE)
	{
		struct where_clause_node* temp_where = (struct where_clause_node*) *the_head;

		if (temp_where != NULL)
		{
			if (temp_where->ptr_one_type == PTR_TYPE_INT || temp_where->ptr_one_type == PTR_TYPE_REAL || temp_where->ptr_one_type == PTR_TYPE_CHAR || temp_where->ptr_one_type == PTR_TYPE_DATE)
			{
				myFree((void**) &temp_where->ptr_one, file_opened_head, malloced_head, the_debug);
				total_freed++;
			}
			else if (temp_where->ptr_one_type == PTR_TYPE_WHERE_CLAUSE_NODE)
			{
				total_freed += freeAnyLinkedList((void**) &temp_where->ptr_one, PTR_TYPE_WHERE_CLAUSE_NODE, file_opened_head, malloced_head, the_debug);
			}
			else if (temp_where->ptr_one_type == PTR_TYPE_MATH_NODE)
			{
				total_freed += freeAnyLinkedList((void**) &temp_where->ptr_one, PTR_TYPE_MATH_NODE, file_opened_head, malloced_head, the_debug);
			}
			else if (temp_where->ptr_one_type == PTR_TYPE_FUNC_NODE)
			{
				total_freed += freeAnyLinkedList((void**) &temp_where->ptr_one, PTR_TYPE_FUNC_NODE, file_opened_head, malloced_head, the_debug);
			}
			else if (temp_where->ptr_one_type == PTR_TYPE_CASE_NODE)
			{
				total_freed += freeAnyLinkedList((void**) &temp_where->ptr_one, PTR_TYPE_CASE_NODE, file_opened_head, malloced_head, the_debug);
			}

			if (temp_where->ptr_two_type == PTR_TYPE_INT || temp_where->ptr_two_type == PTR_TYPE_REAL || temp_where->ptr_two_type == PTR_TYPE_CHAR || temp_where->ptr_two_type == PTR_TYPE_DATE)
			{
				myFree((void**) &temp_where->ptr_two, file_opened_head, malloced_head, the_debug);
				total_freed++;
			}
			else if (temp_where->ptr_two_type == PTR_TYPE_WHERE_CLAUSE_NODE)
			{
				total_freed += freeAnyLinkedList((void**) &temp_where->ptr_two, PTR_TYPE_WHERE_CLAUSE_NODE, file_opened_head, malloced_head, the_debug);
			}
			else if (temp_where->ptr_two_type == PTR_TYPE_MATH_NODE)
			{
				total_freed += freeAnyLinkedList((void**) &temp_where->ptr_two, PTR_TYPE_MATH_NODE, file_opened_head, malloced_head, the_debug);
			}
			else if (temp_where->ptr_two_type == PTR_TYPE_FUNC_NODE)
			{
				total_freed += freeAnyLinkedList((void**) &temp_where->ptr_two, PTR_TYPE_FUNC_NODE, file_opened_head, malloced_head, the_debug);
			}
			else if (temp_where->ptr_two_type == PTR_TYPE_CASE_NODE)
			{
				total_freed += freeAnyLinkedList((void**) &temp_where->ptr_two, PTR_TYPE_CASE_NODE, file_opened_head, malloced_head, the_debug);
			}

			myFree((void**) &temp_where, file_opened_head, malloced_head, the_debug);
			total_freed++;
		}
	}
	else if (the_head_type == PTR_TYPE_FREQUENT_NODE)
	{
		while (*the_head != NULL)
		{
			struct frequent_node* temp = (struct frequent_node*) *the_head;
			*the_head = (void*) ((struct frequent_node*) (*the_head))->next;

			myFree((void**) &temp->ptr_value, file_opened_head, malloced_head, the_debug);
			total_freed++;

			total_freed += freeAnyLinkedList((void**) &temp->row_nums_head, PTR_TYPE_LIST_NODE_PTR, file_opened_head, malloced_head, the_debug);

			myFree((void**) &temp, file_opened_head, malloced_head, the_debug);
			total_freed++;
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


			if (temp->select_node_alias != NULL)
			{
				//printf("Freeing select_node_alias: _%s_\n", temp->select_node_alias);
				myFree((void**) &temp->select_node_alias, file_opened_head, malloced_head, the_debug);
				total_freed++;
			}
				
			for (int i=0; i<temp->columns_arr_size && temp->columns_arr != NULL; i++)
			{
				total_freed += freeAnyLinkedList((void**) &temp->columns_arr[i], PTR_TYPE_COL_IN_SELECT_NODE, file_opened_head, malloced_head, the_debug);

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

			if (temp->having_head != NULL)
			{
				total_freed += freeAnyLinkedList((void**) &temp->having_head, PTR_TYPE_WHERE_CLAUSE_NODE, file_opened_head, malloced_head, the_debug);
			}

			if (temp->order_by != NULL)
			{
				while (temp->order_by->order_by_cols_head != NULL)
				{
					struct ListNodePtr* temp_order_by_col = temp->order_by->order_by_cols_head;
					temp->order_by->order_by_cols_head = temp->order_by->order_by_cols_head->next;

					myFree((void**) &temp_order_by_col, file_opened_head, malloced_head, the_debug);
					total_freed++;
				}

				while (temp->order_by->order_by_cols_which_head != NULL)
				{
					struct ListNodePtr* temp_order_by_which = temp->order_by->order_by_cols_which_head;
					temp->order_by->order_by_cols_which_head = temp->order_by->order_by_cols_which_head->next;

					myFree((void**) &temp_order_by_which->ptr_value, file_opened_head, malloced_head, the_debug);
					total_freed++;

					myFree((void**) &temp_order_by_which, file_opened_head, malloced_head, the_debug);
					total_freed++;
				}

				myFree((void**) &temp->order_by, file_opened_head, malloced_head, the_debug);
			}

			//printf("Freeing temp\n");
			myFree((void**) &temp, file_opened_head, malloced_head, the_debug);
			total_freed++;
		}
	}
	else if (the_head_type == PTR_TYPE_COL_IN_SELECT_NODE)
	{
		struct col_in_select_node* temp_col = *the_head;

		if (temp_col->new_name != NULL)
		{
			//printf("Freeing temp_col->new_name: _%s_\n", temp_col->new_name);
			myFree((void**) &temp_col->new_name, file_opened_head, malloced_head, the_debug);
			total_freed++;
		}

		if (temp_col->case_node != NULL)
		{
			total_freed += freeAnyLinkedList((void**) &temp_col->case_node, PTR_TYPE_CASE_NODE, file_opened_head, malloced_head, the_debug);
		}

		if (temp_col->func_node != NULL)
		{
			total_freed += freeAnyLinkedList((void**) &temp_col->func_node, PTR_TYPE_FUNC_NODE, file_opened_head, malloced_head, the_debug);
		}

		if (temp_col->math_node != NULL)
		{
			total_freed += freeAnyLinkedList((void**) &temp_col->math_node, PTR_TYPE_MATH_NODE, file_opened_head, malloced_head, the_debug);
		}

		myFree((void**) &temp_col, file_opened_head, malloced_head, the_debug);
		total_freed++;
	}
	else if (the_head_type == PTR_TYPE_FUNC_NODE)
	{
		struct func_node* temp_func = (struct func_node*) *the_head;

		for (int j=0; j<temp_func->args_size; j++)
		{
			if (temp_func->args_arr_type[j] != PTR_TYPE_COL_IN_SELECT_NODE)
			{
				if (temp_func->args_arr_type[j] == PTR_TYPE_FUNC_NODE)
				{
					total_freed += freeAnyLinkedList((void**) &temp_func->args_arr[j], PTR_TYPE_FUNC_NODE, file_opened_head, malloced_head, the_debug);
				}
				else
				{
					myFree((void**) &temp_func->args_arr[j], file_opened_head, malloced_head, the_debug);
					total_freed++;
				}
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
	else if (the_head_type == PTR_TYPE_JOIN_NODE)
	{
		while (*the_head != NULL)
		{
			struct join_node* temp = (struct join_node*) *the_head;
			*the_head = (void*) ((struct join_node*) (*the_head))->next;

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
	else if (the_head_type == PTR_TYPE_MATH_NODE)
	{
		struct math_node* temp_math = (struct math_node*) *the_head;

		if (temp_math != NULL)
		{
			if (temp_math->ptr_one_type == PTR_TYPE_INT || temp_math->ptr_one_type == PTR_TYPE_REAL || temp_math->ptr_one_type == PTR_TYPE_CHAR || temp_math->ptr_one_type == PTR_TYPE_DATE)
			{
				myFree((void**) &temp_math->ptr_one, file_opened_head, malloced_head, the_debug);
				total_freed++;
			}
			else if (temp_math->ptr_one_type == PTR_TYPE_FUNC_NODE)
			{
				total_freed += freeAnyLinkedList((void**) &temp_math->ptr_one, PTR_TYPE_FUNC_NODE, file_opened_head, malloced_head, the_debug);
			}
			else if (temp_math->ptr_one_type == PTR_TYPE_MATH_NODE)
			{
				total_freed += freeAnyLinkedList((void**) &temp_math->ptr_one, PTR_TYPE_MATH_NODE, file_opened_head, malloced_head, the_debug);
			}
			else if (temp_math->ptr_one_type == PTR_TYPE_CASE_NODE)
			{
				total_freed += freeAnyLinkedList((void**) &temp_math->ptr_one, PTR_TYPE_CASE_NODE, file_opened_head, malloced_head, the_debug);
			}

			if (temp_math->ptr_two_type == PTR_TYPE_INT || temp_math->ptr_two_type == PTR_TYPE_REAL || temp_math->ptr_two_type == PTR_TYPE_CHAR || temp_math->ptr_two_type == PTR_TYPE_DATE)
			{
				myFree((void**) &temp_math->ptr_two, file_opened_head, malloced_head, the_debug);
				total_freed++;
			}
			else if (temp_math->ptr_two_type == PTR_TYPE_FUNC_NODE)
			{
				total_freed += freeAnyLinkedList((void**) &temp_math->ptr_two, PTR_TYPE_FUNC_NODE, file_opened_head, malloced_head, the_debug);
			}
			else if (temp_math->ptr_two_type == PTR_TYPE_MATH_NODE)
			{
				total_freed += freeAnyLinkedList((void**) &temp_math->ptr_two, PTR_TYPE_MATH_NODE, file_opened_head, malloced_head, the_debug);
			}
			else if (temp_math->ptr_two_type == PTR_TYPE_CASE_NODE)
			{
				total_freed += freeAnyLinkedList((void**) &temp_math->ptr_two, PTR_TYPE_CASE_NODE, file_opened_head, malloced_head, the_debug);
			}

			myFree((void**) &temp_math, file_opened_head, malloced_head, the_debug);
			total_freed++;
		}
	}
	else if (the_head_type == PTR_TYPE_CASE_NODE)
	{
		struct case_node* temp_case = (struct case_node*) *the_head;

		if (temp_case != NULL)
		{
			while (temp_case->case_when_head != NULL)
			{
				struct ListNodePtr* temp_list_node_ptr = temp_case->case_when_head;
				temp_case->case_when_head = temp_case->case_when_head->next;

				total_freed += freeAnyLinkedList((void**) &temp_list_node_ptr->ptr_value, PTR_TYPE_WHERE_CLAUSE_NODE, file_opened_head, malloced_head, the_debug);

				myFree((void**) &temp_list_node_ptr, file_opened_head, malloced_head, the_debug);
				total_freed++;
			}

			while (temp_case->case_then_value_head != NULL)
			{
				struct ListNodePtr* temp_list_node_ptr = temp_case->case_then_value_head;
				temp_case->case_then_value_head = temp_case->case_then_value_head->next;

				if (temp_list_node_ptr->ptr_type == PTR_TYPE_INT || temp_list_node_ptr->ptr_type == PTR_TYPE_REAL || temp_list_node_ptr->ptr_type == PTR_TYPE_CHAR || temp_list_node_ptr->ptr_type == PTR_TYPE_DATE)
				{
					myFree((void**) &temp_list_node_ptr->ptr_value, file_opened_head, malloced_head, the_debug);
					total_freed++;
				}
				else if (temp_list_node_ptr->ptr_type == PTR_TYPE_MATH_NODE)
				{
					total_freed += freeAnyLinkedList((void**) &temp_list_node_ptr->ptr_value, PTR_TYPE_MATH_NODE, file_opened_head, malloced_head, the_debug);
				}
				else if (temp_list_node_ptr->ptr_type == PTR_TYPE_FUNC_NODE)
				{
					total_freed += freeAnyLinkedList((void**) &temp_list_node_ptr->ptr_value, PTR_TYPE_FUNC_NODE, file_opened_head, malloced_head, the_debug);
				}

				myFree((void**) &temp_list_node_ptr, file_opened_head, malloced_head, the_debug);
				total_freed++;
			}

			myFree((void**) &temp_case, file_opened_head, malloced_head, the_debug);
		}
	}
	else if (the_head_type == PTR_TYPE_LIST_NODE_PTR)
	{
		while (*the_head != NULL)
		{
			struct ListNodePtr* temp = (struct ListNodePtr*) *the_head;
			*the_head = (void*) ((struct ListNodePtr*) (*the_head))->next;

			//printf("temp ptr_value = _%d_\n", *((int*) temp->ptr_value));

			if (temp->ptr_value != NULL)
			{	
				if (temp->ptr_type == PTR_TYPE_GROUP_DATE_NODE)
				{
					total_freed += freeAnyLinkedList((void**) &((struct group_data_node*) temp->ptr_value)->row_ids_head, PTR_TYPE_LIST_NODE_PTR, NULL, malloced_head, the_debug);			
				}
				if (temp->ptr_type == PTR_TYPE_LIST_NODE_PTR)
				{
					total_freed += freeAnyLinkedList((void**) &temp->ptr_value, PTR_TYPE_LIST_NODE_PTR, NULL, malloced_head, the_debug);			
				}
				if (temp->ptr_type == PTR_TYPE_MATH_NODE)
				{
					total_freed += freeAnyLinkedList((void**) &temp->ptr_value, PTR_TYPE_MATH_NODE, NULL, malloced_head, the_debug);
				}
				if (temp->ptr_type != PTR_TYPE_COL_IN_SELECT_NODE && temp->ptr_type != PTR_TYPE_TABLE_COLS_INFO && temp->ptr_type > 0)
				{
					myFree((void**) &temp->ptr_value, file_opened_head, malloced_head, the_debug);
					total_freed++;
				}
			}
			myFree((void**) &temp, file_opened_head, malloced_head, the_debug);
			total_freed++;		}
	}
	return total_freed;
}

/*	RETURNS:
 *  RETURN_GOOD
 *
 *	WRITES TO:
 *	ptr
 */
int initEmptyTreeNode(void** ptr, void* the_parent, int node_type)
{
	if (node_type == PTR_TYPE_WHERE_CLAUSE_NODE)
	{
		((struct where_clause_node*) *ptr)->ptr_one = NULL;
		((struct where_clause_node*) *ptr)->ptr_one_type = -1;
		((struct where_clause_node*) *ptr)->ptr_two = NULL;
		((struct where_clause_node*) *ptr)->ptr_two_type = -1;
		((struct where_clause_node*) *ptr)->where_type = -1;
		((struct where_clause_node*) *ptr)->parent = the_parent;
	}
	else //if (node_type == PTR_TYPE_MATH_NODE)
	{
		((struct math_node*) *ptr)->ptr_one = NULL;
		((struct math_node*) *ptr)->ptr_one_type = -1;
		((struct math_node*) *ptr)->ptr_two = NULL;
		((struct math_node*) *ptr)->ptr_two_type = -1;
		((struct math_node*) *ptr)->operation = -1;
		((struct math_node*) *ptr)->parent = the_parent;
	}
    
    return RETURN_GOOD;
}

/*	RETURNS:
 *  RETURN_ERROR if error
 *	RETURN_GOOD	if good
 *
 *	WRITES TO:
 *	cur_mirror
 */
int traverseTreeNode(void** cur, int node_type, void** ptr_of_interest, int* ptr_of_interest_type, void** cur_mirror
					,struct file_opened_node** file_opened_head, struct malloced_node** malloced_head, int the_debug)
{
    if (node_type == PTR_TYPE_WHERE_CLAUSE_NODE)
	{
		if (((struct where_clause_node*) *cur)->ptr_one_type == PTR_TYPE_WHERE_CLAUSE_NODE 
			&& ((struct where_clause_node*) *cur_mirror)->ptr_one == NULL)
		{
		    *cur = ((struct where_clause_node*) *cur)->ptr_one;
		    
		    ((struct where_clause_node*) *cur_mirror)->ptr_one = myMalloc(sizeof(struct where_clause_node), NULL, malloced_head, the_debug);
		    if (((struct where_clause_node*) *cur_mirror)->ptr_one == NULL)
			{
				if (the_debug == YES_DEBUG)
					printf("	ERROR in traverseTreeNode() at line %d in %s\n", __LINE__, __FILE__);
				return RETURN_ERROR;
			}
		    initEmptyTreeNode(&((struct where_clause_node*) *cur_mirror)->ptr_one, (void*) *cur_mirror, PTR_TYPE_WHERE_CLAUSE_NODE);
		    ((struct where_clause_node*) *cur_mirror)->ptr_one_type = PTR_TYPE_WHERE_CLAUSE_NODE;
		    
		    *cur_mirror = ((struct where_clause_node*) *cur_mirror)->ptr_one;
		    
		    //printf("Going deeper at one\n");
		    
		    return traverseTreeNode(cur, node_type, ptr_of_interest, ptr_of_interest_type, cur_mirror, file_opened_head, malloced_head, the_debug);
		}
		else if (((struct where_clause_node*) *cur)->ptr_one_type != PTR_TYPE_WHERE_CLAUSE_NODE && ((struct where_clause_node*) *cur_mirror)->ptr_one_type == -1)
		{
		    *ptr_of_interest = ((struct where_clause_node*) *cur)->ptr_one;
		    *ptr_of_interest_type = ((struct where_clause_node*) *cur)->ptr_one_type;
		    
		    ((struct where_clause_node*) *cur_mirror)->ptr_one_type = 100;
		    
		    //printf("Return A\n");
		    
		    return RETURN_GOOD;
		}
		else if (((struct where_clause_node*) *cur)->ptr_two_type == PTR_TYPE_WHERE_CLAUSE_NODE && ((struct where_clause_node*) *cur_mirror)->ptr_two == NULL)
		{
		    *cur = ((struct where_clause_node*) *cur)->ptr_two;
		    *ptr_of_interest = NULL;
		    *ptr_of_interest_type = -1;
		    
		    ((struct where_clause_node*) *cur_mirror)->ptr_two = myMalloc(sizeof(struct where_clause_node), NULL, malloced_head, the_debug);
		    if (((struct where_clause_node*) *cur_mirror)->ptr_two == NULL)
			{
				if (the_debug == YES_DEBUG)
					printf("	ERROR in traverseTreeNode() at line %d in %s\n", __LINE__, __FILE__);
				return RETURN_ERROR;
			}
		    initEmptyTreeNode(&((struct where_clause_node*) *cur_mirror)->ptr_two, (void*) *cur_mirror, PTR_TYPE_WHERE_CLAUSE_NODE);
		    ((struct where_clause_node*) *cur_mirror)->ptr_two_type = PTR_TYPE_WHERE_CLAUSE_NODE;
		    
		    *cur_mirror = ((struct where_clause_node*) *cur_mirror)->ptr_two;
		    
		    //printf("Going deeper at two\n");
		    
		    return traverseTreeNode(cur, node_type, ptr_of_interest, ptr_of_interest_type, cur_mirror, file_opened_head, malloced_head, the_debug);
		}
		else if (((struct where_clause_node*) *cur)->ptr_two_type != PTR_TYPE_WHERE_CLAUSE_NODE && ((struct where_clause_node*) *cur_mirror)->ptr_two_type == -1)
		{
		    *ptr_of_interest = ((struct where_clause_node*) *cur)->ptr_two;
		    *ptr_of_interest_type = ((struct where_clause_node*) *cur)->ptr_two_type;
		    
		    ((struct where_clause_node*) *cur_mirror)->ptr_two_type = 100;
		    
		    //printf("Return B\n");
		    
		    return RETURN_GOOD;
		}
		else if (((struct where_clause_node*) *cur)->parent != NULL)
		{
		    *cur = ((struct where_clause_node*) *cur)->parent;
		    
		    *cur_mirror = ((struct where_clause_node*) *cur_mirror)->parent;
		    
		    //printf("Going back up\n");
		    
		    return traverseTreeNode(cur, node_type, ptr_of_interest, ptr_of_interest_type, cur_mirror, file_opened_head, malloced_head, the_debug);
		}
		else
		{
		    freeAnyLinkedList((void**) cur_mirror, PTR_TYPE_WHERE_CLAUSE_NODE, file_opened_head, malloced_head, the_debug);
		    return RETURN_NORMAL_NEG;
		}
	}
	else //if (node_type == PTR_TYPE_MATH_NODE)
	{
		if (((struct math_node*) *cur)->ptr_one_type == PTR_TYPE_MATH_NODE && ((struct math_node*) *cur_mirror)->ptr_one == NULL)
		{
		    *cur = ((struct math_node*) *cur)->ptr_one;
		    
		    ((struct math_node*) *cur_mirror)->ptr_one = myMalloc(sizeof(struct math_node), NULL, malloced_head, the_debug);
		    if (((struct math_node*) *cur_mirror)->ptr_one == NULL)
			{
				if (the_debug == YES_DEBUG)
					printf("	ERROR in traverseTreeNode() at line %d in %s\n", __LINE__, __FILE__);
				return RETURN_ERROR;
			}
		    initEmptyTreeNode(&((struct math_node*) *cur_mirror)->ptr_one, (void*) *cur_mirror, PTR_TYPE_WHERE_CLAUSE_NODE);
		    ((struct math_node*) *cur_mirror)->ptr_one_type = PTR_TYPE_MATH_NODE;
		    
		    *cur_mirror = ((struct math_node*) *cur_mirror)->ptr_one;
		    
		    //printf("Going deeper at one\n");
		    
		    return traverseTreeNode(cur, node_type, ptr_of_interest, ptr_of_interest_type, cur_mirror, file_opened_head, malloced_head, the_debug);
		}
		else if (((struct math_node*) *cur)->ptr_one_type != PTR_TYPE_MATH_NODE && 
				 ((struct math_node*) *cur_mirror)->ptr_one_type == -1)
		{
		    *ptr_of_interest = ((struct math_node*) *cur)->ptr_one;
		    *ptr_of_interest_type = ((struct math_node*) *cur)->ptr_one_type;
		    
		    ((struct math_node*) *cur_mirror)->ptr_one_type = 100;
		    
		    //printf("Return A\n");
		    
		    return RETURN_GOOD;
		}
		else if (((struct math_node*) *cur)->ptr_two_type == PTR_TYPE_MATH_NODE && ((struct math_node*) *cur_mirror)->ptr_two == NULL)
		{
		    *cur = ((struct math_node*) *cur)->ptr_two;
		    *ptr_of_interest = NULL;
		    *ptr_of_interest_type = -1;
		    
		    ((struct math_node*) *cur_mirror)->ptr_two = myMalloc(sizeof(struct math_node), NULL, malloced_head, the_debug);
		    if (((struct math_node*) *cur_mirror)->ptr_two == NULL)
			{
				if (the_debug == YES_DEBUG)
					printf("	ERROR in traverseTreeNode() at line %d in %s\n", __LINE__, __FILE__);
				return RETURN_ERROR;
			}
		    initEmptyTreeNode(&((struct math_node*) *cur_mirror)->ptr_two, (void*) *cur_mirror, PTR_TYPE_WHERE_CLAUSE_NODE);
		    ((struct math_node*) *cur_mirror)->ptr_two_type = PTR_TYPE_MATH_NODE;
		    
		    *cur_mirror = ((struct math_node*) *cur_mirror)->ptr_two;
		    
		    //printf("Going deeper at two\n");
		    
		    return traverseTreeNode(cur, node_type, ptr_of_interest, ptr_of_interest_type, cur_mirror, file_opened_head, malloced_head, the_debug);
		}
		else if (((struct math_node*) *cur)->ptr_two_type != PTR_TYPE_MATH_NODE && ((struct math_node*) *cur_mirror)->ptr_two_type == -1)
		{
		    *ptr_of_interest = ((struct math_node*) *cur)->ptr_two;
		    *ptr_of_interest_type = ((struct math_node*) *cur)->ptr_two_type;
		    
		    ((struct math_node*) *cur_mirror)->ptr_two_type = 100;
		    
		    //printf("Return B\n");
		    
		    return RETURN_GOOD;
		}
		else if (((struct math_node*) *cur)->parent != NULL)
		{
		    *cur = ((struct math_node*) *cur)->parent;
		    
		    *cur_mirror = ((struct math_node*) *cur_mirror)->parent;
		    
		    //printf("Going back up\n");
		    
		    return traverseTreeNode(cur, node_type, ptr_of_interest, ptr_of_interest_type, cur_mirror, file_opened_head, malloced_head, the_debug);
		}
		else
		{
			freeAnyLinkedList((void**) cur_mirror, PTR_TYPE_MATH_NODE, file_opened_head, malloced_head, the_debug);
		    return RETURN_NORMAL_NEG;
		}
	}
}

/*	RETURNS:
 *  RETURN_GOOD
 *
 *	WRITES TO:
 */
int traceTreeNode(void** cur, int node_type)
{
	if (node_type == PTR_TYPE_WHERE_CLAUSE_NODE)
	{
		if (((struct where_clause_node*) cur)->ptr_one_type == PTR_TYPE_WHERE_CLAUSE_NODE)
		{
			printf("L\n");
			traceTreeNode(((struct where_clause_node*) cur)->ptr_one, PTR_TYPE_WHERE_CLAUSE_NODE);
		}
		else
		{
			printf("lo\n");
		}

		if (((struct where_clause_node*) cur)->ptr_two_type == PTR_TYPE_WHERE_CLAUSE_NODE)
		{
			printf("R\n");
			traceTreeNode(((struct where_clause_node*) cur)->ptr_two, PTR_TYPE_WHERE_CLAUSE_NODE);
		}
		else
		{
			printf("lr\n");
		}
	}

	return RETURN_GOOD;
}

/*	RETURNS:
 *  RETURN_GOOD
 *
 *	WRITES TO:
 *	col_data_arr
 */
int merge(struct colDataNode** col_data_arr, int data_type, int order_type, int l, int m, int r)
{
	int i, j, k;
    int n1 = m - l + 1;
    int n2 = r - m;
 
    // Create temp arrays
    struct colDataNode* L[n1];
    struct colDataNode* R[n2];
 
    // Copy data to temp arrays L[] and R[]
    for (i = 0; i < n1; i++)
        L[i] = col_data_arr[l + i];
    for (j = 0; j < n2; j++)
        R[j] = col_data_arr[m + 1 + j];
 
    // Merge the temp arrays back into arr[l..r]
    i = 0;
    j = 0;
    k = l;
    while (i < n1 && j < n2) 
    {
		if (data_type == DATA_INT)
        {
        	if (order_type == ORDER_BY_ASC)
	        {
	        	if ((L[i]->row_data != NULL && R[j]->row_data != NULL && *((int*) L[i]->row_data) <= *((int*) R[j]->row_data))
	        		|| R[j]->row_data == NULL)
		        {
		            col_data_arr[k] = L[i];
		            i++;
		        }
		        else 
		        {
		            col_data_arr[k] = R[j];
		            j++;
		        }
	        }
	        else //if (order_type == ORDER_BY_DESC)
	       	{
	       		if ((L[i]->row_data != NULL && R[j]->row_data != NULL && *((int*) L[i]->row_data) > *((int*) R[j]->row_data))
	        		|| R[j]->row_data == NULL)
		        {
		            col_data_arr[k] = L[i];
		            i++;
		        }
		        else 
		        {
		            col_data_arr[k] = R[j];
		            j++;
		        }
	       	}	
        }
        else if (data_type == DATA_REAL)
        {
        	if (order_type == ORDER_BY_ASC)
	        {
	        	if ((L[i]->row_data != NULL && R[j]->row_data != NULL && *((double*) L[i]->row_data) <= *((double*) R[j]->row_data))
	        		|| R[j]->row_data == NULL)
		        {
		            col_data_arr[k] = L[i];
		            i++;
		        }
		        else 
		        {
		            col_data_arr[k] = R[j];
		            j++;
		        }
	        }
	        else //if (order_type == ORDER_BY_DESC)
	       	{
	       		if ((L[i]->row_data != NULL && R[j]->row_data != NULL && *((double*) L[i]->row_data) > *((double*) R[j]->row_data))
	        		|| R[j]->row_data == NULL)
		        {
		            col_data_arr[k] = L[i];
		            i++;
		        }
		        else 
		        {
		            col_data_arr[k] = R[j];
		            j++;
		        }
	       	}
	        	
        }
        else if (data_type == DATA_DATE)
        {
        	if (order_type == ORDER_BY_ASC)
	        {
	        	if ((L[i]->row_data != NULL && R[j]->row_data != NULL && dateToInt(L[i]->row_data) <= dateToInt(R[j]->row_data))
	        		|| R[j]->row_data == NULL)
		        {
		            col_data_arr[k] = L[i];
		            i++;
		        }
		        else 
		        {
		            col_data_arr[k] = R[j];
		            j++;
		        }
	        }
	        else //if (order_type == ORDER_BY_DESC)
	       	{
	       		if ((L[i]->row_data != NULL && R[j]->row_data != NULL && dateToInt(L[i]->row_data) > dateToInt(R[j]->row_data))
	        		|| R[j]->row_data == NULL)
		        {
		            col_data_arr[k] = L[i];
		            i++;
		        }
		        else 
		        {
		            col_data_arr[k] = R[j];
		            j++;
		        }
	       	}
        }
        else if (data_type == DATA_STRING)
        {
	        if (order_type == ORDER_BY_ASC)
	        {
	        	if ((L[i]->row_data != NULL && R[j]->row_data != NULL && strcmp(L[i]->row_data, R[j]->row_data) <= 0)
	        		|| R[j]->row_data == NULL)
		        {
		            col_data_arr[k] = L[i];
		            i++;
		        }
		        else 
		        {
		            col_data_arr[k] = R[j];
		            j++;
		        }
	        }
	        else //if (order_type == ORDER_BY_DESC)
	       	{
	       		if ((L[i]->row_data != NULL && R[j]->row_data != NULL && strcmp(L[i]->row_data, R[j]->row_data) > 0)
	        		|| R[j]->row_data == NULL)
		        {
		            col_data_arr[k] = L[i];
		            i++;
		        }
		        else 
		        {
		            col_data_arr[k] = R[j];
		            j++;
		        }
	       	}
        }
        else 
        	return -1;

        k++;
    }
 
    // Copy the remaining elements of L[],
    // if there are any
    while (i < n1)
    {
        col_data_arr[k] = L[i];
        i++;
        k++;
    }
 
    // Copy the remaining elements of R[],
    // if there are any
    while (j < n2) 
    {
        col_data_arr[k] = R[j];
        j++;
        k++;
    }

    return 0;
}

/*	RETURNS:
 *  RETURN_GOOD
 *
 *	WRITES TO:
 *	col_data_arr
 */
int mergeSort(struct colDataNode** col_data_arr, int data_type, int order_type, int l, int r)
{
	if (l < r)
	{
        int m = l + (r - l) / 2;
 
        // Sort first and second halves
        mergeSort(col_data_arr, data_type, order_type, l, m);
        mergeSort(col_data_arr, data_type, order_type, m + 1, r);
 
        merge(col_data_arr, data_type, order_type, l, m, r);
    }

    return RETURN_GOOD;
}

/*	RETURNS:
 *  RETURN_ERROR if error
 *	RETURN_GOOD if good
 *
 *	WRITES TO:
 *	the_head
 *	the_tail
 */
int getAllColsFromWhereNode(struct ListNodePtr** the_head, struct ListNodePtr** the_tail, struct where_clause_node* the_where_node, struct malloced_node** malloced_head, int the_debug)
{
	void* ptr = NULL;
	int ptr_type = -1;

	struct where_clause_node* cur_mirror = myMalloc(sizeof(struct where_clause_node), NULL, malloced_head, the_debug);
	if (cur_mirror == NULL)
	{
		if (the_debug == YES_DEBUG)
			printf("	ERROR in getAllColsFromWhereNode() at line %d in %s\n", __LINE__, __FILE__);
		return RETURN_ERROR;
	}
	initEmptyTreeNode((void**) &cur_mirror, NULL, PTR_TYPE_WHERE_CLAUSE_NODE);

	while (true)
	{
		int traversd = traverseTreeNode((void**) &the_where_node, PTR_TYPE_WHERE_CLAUSE_NODE, &ptr, &ptr_type, (void**) &cur_mirror
									   ,NULL, malloced_head, the_debug);
		if (traversd == -1)
		{
			if (the_debug == YES_DEBUG)
				printf("Natural break\n");
			break;
		}
		else if (traversd == -2)
		{
			if (the_debug == YES_DEBUG)
				printf("	ERROR in getAllColsFromWhereNode() at line %d in %s\n", __LINE__, __FILE__);
			return RETURN_ERROR;
		}

		if (ptr_type == PTR_TYPE_COL_IN_SELECT_NODE || ptr_type == PTR_TYPE_TABLE_COLS_INFO)
		{
			if (addListNodePtr(the_head, the_tail, ptr, ptr_type, ADDLISTNODE_TAIL, NULL, malloced_head, the_debug) != 0)
			{
				if (the_debug == YES_DEBUG)
					printf("	ERROR in getAllColsFromWhereNode() at line %d in %s\n", __LINE__, __FILE__);
				return RETURN_ERROR;
			}
		}
		else if (ptr_type == PTR_TYPE_FUNC_NODE)
		{
			if (getAllColsFromFuncNode(the_head, the_tail, ptr, malloced_head, the_debug) != 0)
			{
				if (the_debug == YES_DEBUG)
					printf("	ERROR in getAllColsFromWhereNode() at line %d in %s\n", __LINE__, __FILE__);
				return RETURN_ERROR;
			}
		}
		else if (ptr_type == PTR_TYPE_MATH_NODE)
		{
			if (getAllColsFromMathNode(the_head, the_tail, ptr, malloced_head, the_debug) != 0)
			{
				if (the_debug == YES_DEBUG)
					printf("	ERROR in getAllColsFromWhereNode() at line %d in %s\n", __LINE__, __FILE__);
				return RETURN_ERROR;
			}
		}
		else if (ptr_type == PTR_TYPE_CASE_NODE)
		{
			if (getAllColsFromCaseNode(the_head, the_tail, ptr, malloced_head, the_debug) != 0)
			{
				if (the_debug == YES_DEBUG)
					printf("	ERROR in getAllColsFromWhereNode() at line %d in %s\n", __LINE__, __FILE__);
				return RETURN_ERROR;
			}
		}
	}

	return RETURN_GOOD;
}

/*	RETURNS:
 *  RETURN_ERROR if error
 *	RETURN_GOOD if good
 *
 *	WRITES TO:
 *	the_head
 *	the_tail
 */
int getAllColsFromFuncNode(struct ListNodePtr** the_head, struct ListNodePtr** the_tail, struct func_node* the_func_node, struct malloced_node** malloced_head, int the_debug)
{
	struct func_node* cur_func = the_func_node;
	while (cur_func->args_arr_type[0] == PTR_TYPE_FUNC_NODE)
	{
		cur_func = cur_func->args_arr[0];
	}
	
	for (int j=0; j<cur_func->args_size; j++)
	{
		if (cur_func->args_arr_type[j] == PTR_TYPE_COL_IN_SELECT_NODE || cur_func->args_arr_type[j] == PTR_TYPE_TABLE_COLS_INFO)
		{
			if (addListNodePtr(the_head, the_tail, cur_func->args_arr[j], PTR_TYPE_COL_IN_SELECT_NODE, ADDLISTNODE_TAIL, NULL, malloced_head, the_debug) != 0)
			{
				if (the_debug == YES_DEBUG)
					printf("	ERROR in getAllColsFromFuncNode() at line %d in %s\n", __LINE__, __FILE__);
				return RETURN_ERROR;
			}
		}
	}

	return RETURN_GOOD;
}

/*	RETURNS:
 *  RETURN_ERROR if error
 *	RETURN_GOOD if good
 *
 *	WRITES TO:
 *	the_head
 *	the_tail
 */
int getAllColsFromMathNode(struct ListNodePtr** the_head, struct ListNodePtr** the_tail, struct math_node* the_math_node, struct malloced_node** malloced_head, int the_debug)
{
	void* ptr = NULL;
	int ptr_type = -1;

	struct math_node* cur_mirror = myMalloc(sizeof(struct math_node), NULL, malloced_head, the_debug);
	if (cur_mirror == NULL)
	{
		if (the_debug == YES_DEBUG)
			printf("	ERROR in getAllColsFromMathNode() at line %d in %s\n", __LINE__, __FILE__);
		return RETURN_ERROR;
	}
	initEmptyTreeNode((void**) &cur_mirror, NULL, PTR_TYPE_MATH_NODE);

	while (true)
	{
		int traversd = traverseTreeNode((void**) &the_math_node, PTR_TYPE_MATH_NODE, &ptr, &ptr_type, (void**) &cur_mirror
									   ,NULL, malloced_head, the_debug);
		if (traversd == -1)
		{
			if (the_debug == YES_DEBUG)
				printf("Natural break\n");
			break;
		}
		else if (traversd == -2)
		{
			if (the_debug == YES_DEBUG)
				printf("	ERROR in getAllColsFromMathNode() at line %d in %s\n", __LINE__, __FILE__);
			return RETURN_ERROR;
		}

		if (ptr_type == PTR_TYPE_COL_IN_SELECT_NODE || ptr_type == PTR_TYPE_TABLE_COLS_INFO)
		{
			if (addListNodePtr(the_head, the_tail, ptr, ptr_type, ADDLISTNODE_TAIL, NULL, malloced_head, the_debug) != 0)
			{
				if (the_debug == YES_DEBUG)
					printf("	ERROR in getAllColsFromMathNode() at line %d in %s\n", __LINE__, __FILE__);
				return RETURN_ERROR;
			}
		}
		else if (ptr_type == PTR_TYPE_FUNC_NODE)
		{
			if (getAllColsFromFuncNode(the_head, the_tail, ptr, malloced_head, the_debug) != 0)
			{
				if (the_debug == YES_DEBUG)
					printf("	ERROR in getAllColsFromMathNode() at line %d in %s\n", __LINE__, __FILE__);
				return RETURN_ERROR;
			}
		}
		else if (ptr_type == PTR_TYPE_CASE_NODE)
		{
			if (getAllColsFromCaseNode(the_head, the_tail, ptr, malloced_head, the_debug) != 0)
			{
				if (the_debug == YES_DEBUG)
					printf("	ERROR in getAllColsFromMathNode() at line %d in %s\n", __LINE__, __FILE__);
				return RETURN_ERROR;
			}
		}
	}

	return RETURN_GOOD;
}

/*	RETURNS:
 *  RETURN_ERROR if error
 *	RETURN_GOOD if good
 *
 *	WRITES TO:
 *	the_head
 *	the_tail
 */
int getAllColsFromCaseNode(struct ListNodePtr** the_head, struct ListNodePtr** the_tail, struct case_node* the_case_node, struct malloced_node** malloced_head, int the_debug)
{
	struct ListNodePtr* cur_when = the_case_node->case_when_head;
	struct ListNodePtr* cur_then = the_case_node->case_then_value_head;

	while (cur_when != NULL)
	{
		if (cur_when->ptr_value != NULL)
		{
			if (getAllColsFromWhereNode(the_head, the_tail, cur_when->ptr_value, malloced_head, the_debug) != 0)
			{
				if (the_debug == YES_DEBUG)
					printf("	ERROR in getAllColsFromCaseNode() at line %d in %s\n", __LINE__, __FILE__);
				return RETURN_ERROR;
			}
		}

		if (cur_then->ptr_type == PTR_TYPE_COL_IN_SELECT_NODE || cur_then->ptr_type == PTR_TYPE_TABLE_COLS_INFO)
		{
			if (addListNodePtr(the_head, the_tail, cur_then->ptr_value, cur_then->ptr_type, ADDLISTNODE_TAIL, NULL, malloced_head, the_debug) != 0)
			{
				if (the_debug == YES_DEBUG)
					printf("	ERROR in getAllColsFromCaseNode() at line %d in %s\n", __LINE__, __FILE__);
				return RETURN_ERROR;
			}
		}
		else if (cur_then->ptr_type == PTR_TYPE_FUNC_NODE)
		{
			if (getAllColsFromFuncNode(the_head, the_tail, cur_then->ptr_value, malloced_head, the_debug) != 0)
			{
				if (the_debug == YES_DEBUG)
					printf("	ERROR in getAllColsFromCaseNode() at line %d in %s\n", __LINE__, __FILE__);
				return RETURN_ERROR;
			}
		}
		else if (cur_then->ptr_type == PTR_TYPE_MATH_NODE)
		{
			if (getAllColsFromMathNode(the_head, the_tail, cur_then->ptr_value, malloced_head, the_debug) != 0)
			{
				if (the_debug == YES_DEBUG)
					printf("	ERROR in getAllColsFromCaseNode() at line %d in %s\n", __LINE__, __FILE__);
				return RETURN_ERROR;
			}
		}

		cur_when = cur_when->next;
		cur_then = cur_then->next;
	}

	return RETURN_GOOD;
}

/*	RETURNS:
 *  RETURN_GOOD
 */
int myFreeResultsOfSelect(struct col_in_select_node* cur_col, struct malloced_node** malloced_head, int the_debug)
{
	for (int i=0; i<cur_col->num_rows; i++)
	{
		myFree((void**) &cur_col->col_data_arr[i], NULL, malloced_head, the_debug);
	}
	myFree((void**) &cur_col->col_data_arr, NULL, malloced_head, the_debug);

	if (cur_col->unique_values_head != NULL)
	{
		freeAnyLinkedList((void**) &cur_col->unique_values_head, PTR_TYPE_LIST_NODE_PTR, NULL, malloced_head, the_debug);
	}

	return RETURN_GOOD;
}


/*	RETURNS:
 *  RETURN_GOOD
 *
 *	WRITES TO:
 */
int errorTeardown(struct file_opened_node** file_opened_head, struct malloced_node** malloced_head, int the_debug)
{
	if (the_debug == YES_DEBUG)
		printf("!!! Called errorTeardown\n");

	if (file_opened_head != NULL)
		myFileCloseAll(file_opened_head, malloced_head, the_debug);

	myFreeAllError(malloced_head, the_debug);

	return RETURN_GOOD;
}