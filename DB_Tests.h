#ifndef DB_TESTS_H_
#define DB_TESTS_H_

typedef unsigned long long int_8;


int test_Driver_setup(int the_debug);

int test_Driver_teardown(int the_debug);

int selectAndCheckHash(int the_debug, char* test_version);


int test_Helper_DateFunctions_1(int the_debug, char* date);

int test_Helper_DateFunctions_2(int the_debug, int_8 date);


int test_Driver_updateRows();

int test_Driver_deleteRows();

int test_Driver_insertRows();


int test_Driver_main(int the_debug);

#endif