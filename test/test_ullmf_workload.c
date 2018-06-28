/*
 * Copyright (c) 2018 Universidad de La Laguna <cap@pcg.ull.es>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation; either version 2 of the License, or (at your option)
 * any later version.
 *
 * Author: Alberto Cabrera <Alberto.Cabrera@ull.edu.es>
 */

#include "CUnit/Basic.h"
#include "ullmf_class_utils.h"
#include "ullmf_workload.h"

static ullmf_workload_t* workload1;
static int counts[2] = {2, 2};
static int displs[2] = {0, 2};
static int counts2[2] = {1, 3};
static int displs2[2] = {0, 1};
static int num_procs = 2;
static int blocksize = 1;

int init_suite1(void)
{
   return 0;
}


int clean_suite1(void)
{
   return 0;
}

static void* create_obj(ullmf_workload_t* workload_obj, ...) {
	va_list args;
	va_start(args, workload_obj);
	((class_t*) workload1->_class)->constructor(workload_obj, &args);
	va_end(args);
	return workload_obj;
}

void test_constructor(void)
{
	workload1 = calloc(1, ((class_t*) Workload)->size);
	workload1->_class = Workload;
	workload1 = create_obj(workload1, num_procs, counts, displs, blocksize);
	CU_ASSERT_NOT_EQUAL(workload1, 0);
	CU_ASSERT_EQUAL(workload1->_class, Workload);
	CU_ASSERT_STRING_EQUAL(((class_t*) workload1->_class)->name, ((class_t*) Workload)->name);
	CU_ASSERT_NOT_EQUAL(workload1->counts, 0);
	CU_ASSERT_NOT_EQUAL(workload1->displs, 0);
	CU_ASSERT_NOT_EQUAL(workload1->counts, counts);
	CU_ASSERT_NOT_EQUAL(workload1->displs, displs);
    CU_ASSERT_EQUAL(workload1->num_procs, num_procs);
    int workload_size = 0;
	for (int i = 0; i < workload1->num_procs; i++) {
		CU_ASSERT_EQUAL(counts[i], workload1->counts[i]);
		CU_ASSERT_EQUAL(displs[i], workload1->displs[i]);
		workload_size += counts[i];
	}
    CU_ASSERT_EQUAL(workload1->size, workload_size);
}

void test_set_workload(void)
{
	workload1->set_workload(workload1, counts2, displs2);
    int workload_size = 0;
	for (int i = 0; i < workload1->num_procs; i++) {
		CU_ASSERT_EQUAL(counts2[i], workload1->counts[i]);
		CU_ASSERT_EQUAL(displs2[i], workload1->displs[i]);
		workload_size += counts[i];
	}
	CU_ASSERT_EQUAL(workload1->size, workload_size);
}

void test_set_blocksize(void)
{
	CU_ASSERT_EQUAL(workload1->blocksize, blocksize);
	workload1->set_blocksize(workload1, blocksize * 2);
	CU_ASSERT_EQUAL(workload1->blocksize, blocksize * 2);
}

void test_destructor(void)
{
	((class_t*) workload1->_class)->destructor(workload1);
	CU_ASSERT_EQUAL(workload1->counts, 0);
	CU_ASSERT_EQUAL(workload1->displs, 0);
}


/* The main() function for setting up and running the tests.
 * Returns a CUE_SUCCESS on successful running, another
 * CUnit error code on failure.
 */
int main()
{
   CU_pSuite pSuite = NULL;

   /* initialize the CUnit test registry */
   if (CUE_SUCCESS != CU_initialize_registry())
      return CU_get_error();

   /* add a suite to the registry */
   pSuite = CU_add_suite("suite_ullmf_workload", init_suite1, clean_suite1);
   if (NULL == pSuite) {
      CU_cleanup_registry();
      return CU_get_error();
   }

   /* add the tests to the suite */
   if ((NULL == CU_add_test(pSuite, "test of workload constructor()", test_constructor)) ||
	   (NULL == CU_add_test(pSuite, "test of workload set_workload()", test_set_workload)) ||
	   (NULL == CU_add_test(pSuite, "test of workload set_blocksize()", test_set_blocksize)) ||
	   (NULL == CU_add_test(pSuite, "test of workload destructor()", test_destructor))
      )
   {
      CU_cleanup_registry();
      return CU_get_error();
   }

   /* Run all tests using the CUnit Basic interface */
   CU_basic_set_mode(CU_BRM_VERBOSE);
   CU_basic_run_tests();
   return CU_get_number_of_failures();
}
