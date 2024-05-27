#ifndef DB_SHM_STRUCT_H_
#define DB_SHM_STRUCT_H_

typedef unsigned long long int_8;

#include <semaphore.h>


struct shm_table_info
{
	char name[32];		//	Max 31 bytes + 1 for 0
	char keyword[32];	//	Max 31 bytes + 1 for 0
	int_8 file_number;
	int_8 num_cols;

	int table_cols_starting_index;
};

struct shm_table_cols_info
{
	char col_name[32];	//	Max 31 bytes + 1 for 0
	int_8 data_type;
	int_8 max_length;
	int_8 col_number;
	int_8 num_rows;
	int_8 num_open;

	int home_table_index;
};

struct shmem
{
	sem_t sem1;				/* POSIX unnamed semaphore */
	sem_t sem2;				/* POSIX unnamed semaphore */

	char keyword[32];

	int_8 num_tables;
	struct shm_table_info  shm_table_info_arr[100];
	struct shm_table_cols_info  shm_table_cols_info_arr[10000];
};

struct shmem_col_data_node
{
	char data[500];
};

struct shmem_select
{
	int_8 cur_num_rows;
	int_8 total_num_rows;
	int num_cols;
};

#endif