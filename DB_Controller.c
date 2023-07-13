#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

typedef unsigned long int_8;

struct table_cols_info
{
	char col_name[32];
	int_8 data_type;
	int_8 max_length;
	int_8 col_number;
	int_8 num_rows;
	int_8 num_open;
	int_8 num_added_insert_open;
	struct table_cols_info* next;
};

int strcontains(char* str, char the_char)
{
	int index = 0;
	while (str[index] != 0)
	{
	    if (str[index] == the_char)
        {
            return 1;
        }
	    index++;
	}
	
	return 0;
}

void printCommands()
{
	printf("Create a table with:\n");
	printf("	create table [table name] as ([column name] as [datatype], .... );\n");
	printf("Both [table name] and [column name] cannot be more then 31 characters in length.\n");
	printf("Valid datatypes are \"integer\", \"real\", \"char([max length])\", and \"date\".\n");
	printf("\nInsert into a table with:\n");
	printf("	insert into [table name] ([column name], [column name], ...) values ([data], [data], ...);\n");
	printf("\nDelete from a table with:\n");
	printf("	delete from [table name] where [column name] = [data];\n");
	printf("\nUpdate a table with:\n");
	printf("	update [table name] set [column name] = [data], [column name] = [data], ... where [column name] = [data];\n");
	printf("\nSelect from a table with:\n");
	printf("	select */[column name], [column name], ... from [table name] where [column name] = [data];\n");
}

char*** select_ParseStringAndExec(char* input)
{
    int index=0;
    while (input[index] != 'f' && input[index+1] != 'r' && input[index+2] != 'o' && input[index+3] != 'm')
        index++;
    index += 4;
    while (input[index] == ' ' || input[index] == '\t' || input[index] == '\n')
        index++;
        
    int new_index = index;
    while (input[new_index] != ' ' && input[new_index] != '\t' && input[new_index] != '\n' && input[new_index] != ';' && input[new_index] != ',')
        new_index++;
    
    char* table_name = malloc(sizeof(char) * (new_index-index)+1);
    
    strncpy(table_name, input+index, new_index-index);
    
    table_name[new_index-index] = 0;
    
    printf("table_name = _%s_\n", table_name);
    
    if (strcmp(table_name, "") == 0)
	    return NULL;
	    
    /*
        Get the table number from the linked list
    */
    
    char col_names[1000];
    
    sscanf(input, "select%s%*[^\n]", col_names);
    
    printf("col_names = _%s_\n", col_names);
    
    int_8* col_numbers;
    
    if (strcmp(col_names, "*") == 0)
    {
        /*
            Get the number of columns from the cur_table
        */
        //col_numbers = malloc(sizeof(int_8))
    }
    else
    {
        int number_of_cols = 1;
        int index = 0;
    	while (input[index] != 0)
    	{
    	    if (input[index] == 'f' && input[index+1] == 'r' && input[index+2] == 'o' && input[index+3] == 'm')
    	        break;
    	    if (input[index] == ',')
                number_of_cols++;
    	    index++;
    	}
    	
    	printf("number_of_cols = %d\n", number_of_cols);
    	
    	col_numbers = malloc(sizeof(int_8) * number_of_cols);
        
        char** col_names = malloc(sizeof(char*) * number_of_cols);
        
        for (int i=0; i<number_of_cols; i++)
        {
            char format[1000] = "select ";
	    
    	    for (int j=0; j<i; j++)
                strcat(format, "%*[^,] %*[,]");
    	    
    	    if (i < number_of_cols-1)
    	        strcat(format, " %19[^, ] %*[,]");
    	    else
    	        strcat(format, " %19[^ ] %*[^\\n]");
    	        
            strcat(format, " %*[^\\n]");
            
            //printf("format = _%s_\n", format);
            
            col_names[i] = malloc(sizeof(char) * 20);
        
            sscanf(input, format, col_names[i]);
            
            printf("col_names[%d] = %s\n", i, col_names[i]);
            
            free(col_names[i]);
        }
        
        /*
            Get the column numbers from the cur_table
        */
        
        free(col_names);
        free(col_numbers);
    }
    
    return 1;
}

int create_ParseStringAndExec(char* input)
{
	//printf("input = %s\n", input);
	
	char table_name[32];
	
	sscanf(input, "create%*[^t]table%s%*[^\n]", table_name);
	
	//printf("table_name = _%s_\n", table_name);
	
	if (strcmp(table_name, "") == 0)
	    return 0;
	
    int number_of_cols = 1;
    int index = 0;
	while (input[index] != 0)
	{
	    if (input[index] == ',')
            number_of_cols++;
	    index++;
	}
	
	struct table_cols_info* table_cols = malloc(sizeof(struct table_cols_info));
	struct table_cols_info* cur_col = table_cols;
	
	char** col_data_types = malloc(sizeof(char*) * number_of_cols);
	
	for (int i=0; i<number_of_cols; i++)
	{
	    char format[1000] = "create%*[^(](";
	    
	    for (int j=0; j<i; j++)
            strcat(format, "%*[^,] %*[,]");
	    
	    strcat(format, "%s%*[^a]as");
	    if (i < number_of_cols-1)
	        strcat(format, " %19[^, ] %*[,]");
	    else
	        strcat(format, " %19[^);] %*[^\\n]");
	        
        strcat(format, " %*[^\\n]");
        
        //printf("format = _%s_\n", format);
        
        col_data_types[i] = malloc(sizeof(char) * 20);
        
        sscanf(input, format, cur_col->col_name, col_data_types[i]);
        
        sscanf(cur_col->col_name, "\n%s\n", cur_col->col_name);
	    sscanf(col_data_types[i], "\n%s\n", col_data_types[i]);
	    
	    //printf("cur_col->col_name = %s\n", cur_col->col_name);
	    //printf("col_data_types[%d] = %s\n", i, col_data_types[i]);
	    
	    char datatype[5];
	     
	    datatype[4] = 0;
	    
	    //printf("datatype = %s\n", datatype);
	    
	    if (strcmp(datatype, "char") == 0)
	    {
	        cur_col->data_type = 3;
	        sscanf(col_data_types[i], "char(%lu)", &cur_col->max_length);
	    }
	    else if (strcmp(datatype, "inte") == 0)
	    {
	        cur_col->data_type = 1;
	        cur_col->max_length = 8;
	    }
	    else if (strcmp(datatype, "real") == 0)
	    {
	        cur_col->data_type = 2;
	        cur_col->max_length = 8;
	    }
	    else if (strcmp(datatype, "date") == 0)
	    {
	        cur_col->data_type = 4;
	        cur_col->max_length = 8;
	    }
	    
	    cur_col->col_number = i;
	    
	    if (strcmp(cur_col->col_name, "") == 0)
	        return 0;
	    if (strcontains(cur_col->col_name, ','))
	        return 0;
	    if (cur_col->data_type != 1 && cur_col->data_type != 2 && cur_col->data_type != 3 && cur_col->data_type != 4)
	        return 0;
	    if (cur_col->data_type == 3 && cur_col->max_length == 0)
	        return 0;
	    
	    //printf("	Column name = %s\n", cur_col->col_name);
		//printf("	Datatype = %lu\n", cur_col->data_type);
		//printf("	Max length = %lu\n", cur_col->max_length);
		//printf("	Column number = %lu\n", cur_col->col_number);
	    
	    if (i < number_of_cols-1)
	    {
	        cur_col->next = malloc(sizeof(struct table_cols_info));
	        cur_col = cur_col->next;
	    }
	    else
	        cur_col->next = NULL;
	}
	
	for (int i=0; i<number_of_cols; i++)
	    free(col_data_types[i]);
	
	free(col_data_types);
	
	//createTable(table_name, table_cols);
	
	return 1;
}

int main()
{
	//initDB();

	printCommands();

	char* input = malloc(sizeof(char) * 100000);
	while (1)
	{
		strcpy(input, "");
		printf("\n");
		while (1)
		{
		    char temp_input[1000];
    	    fgets(temp_input, 999, stdin);
    		
    		//printf("temp_input = _%s_\n", temp_input);
    		
    		strcat(input, temp_input);
    		
    		//printf("input = _%s_\n", input);
    		
    		if (strcontains(temp_input, ';') == 1)
    		    break;
		}
		
		for (int i=0; i<100000 && input[i] != 0; i++)
			input[i] = tolower(input[i]);
		
		//printf("\n");	
		//printf("Input = _%s_\n", input);
		
		char cmd[5];
		strncpy(cmd, input, 4);
		cmd[4] = 0;
		//printf("cmd = _%s_\n", cmd);
		
		if (strcmp(cmd, "crea") == 0)
		{
		    int returnd = create_ParseStringAndExec(input);
		    if (returnd == 1)
		        printf("\nTable created.\n");
		    else
		        printf("\nThe command was not recognized, please try again.\n");
		}
			
		else if (strcmp(cmd, "modi") == 0)
		{
			
		}
		else if (strcmp(cmd, "sele") == 0)
		{
			char*** returnd = select_ParseStringAndExec(input);
		    if (returnd == NULL)
		        printf("\nThe command was not recognized, please try again.\n");
		    else
		        printf("\nResults:\n");
		}
		else if (strcmp(cmd, "inse") == 0)
		{
		    
		}
		else if (strcmp(cmd, "upda") == 0)
		{
		    
		}
		else if (strcmp(cmd, "dele") == 0)
		{
		    
		}
		else if (strcmp(cmd, "stop") == 0)
		    break;
	    else
	        printf("\nThe command was not recognized, please try again.\n");
	}
	free(input); 
}
