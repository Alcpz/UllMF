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
#include "ullmf_measurement_device_mpi.h"
#include "ullmf_measurement_device.h"
#include <unistd.h>

extern struct measurement_device_mpi mpi_device;
enum ullmf_measurement_error error_code;


int init_suite1(void)
{
   return 0;
}


int clean_suite1(void)
{
   return 0;
}

void test_init(void)
{
    error_code = mpi_device.parent.init(&mpi_device);
    CU_ASSERT_DOUBLE_EQUAL(mpi_device.parent.measurement, 0, 0.005);
    CU_ASSERT_EQUAL(error_code, ULLMF_MEASUREMENT_SUCCESS);
}

void test_shutdown(void)
{
    mpi_device.parent.shutdown(&mpi_device);
    CU_ASSERT_EQUAL(error_code, ULLMF_MEASUREMENT_SUCCESS);
    CU_PASS("Shutdown ok");
}

void test_measurement_start(void)
{
    mpi_device.parent.measurement_start(&mpi_device);
    CU_ASSERT_DOUBLE_NOT_EQUAL(mpi_device.parent.measurement, 0, 0.00005);
    CU_ASSERT_EQUAL(error_code, ULLMF_MEASUREMENT_SUCCESS);
}

void test_measurement_stop(void)
{
    sleep(1);
    mpi_device.parent.measurement_stop(&mpi_device);
    CU_ASSERT_EQUAL(error_code, ULLMF_MEASUREMENT_SUCCESS);
    CU_ASSERT(mpi_device.parent.measurement > 0);
    CU_ASSERT_DOUBLE_EQUAL(mpi_device.parent.measurement, 1, 0.005);
    CU_ASSERT_DOUBLE_NOT_EQUAL(mpi_device.parent.measurement, 0, 0.00005);
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
   pSuite = CU_add_suite("suite_ullmf_measurement_device_mpi", init_suite1, clean_suite1);
   if (NULL == pSuite) {
      CU_cleanup_registry();
      return CU_get_error();
   }

   /* add the tests to the suite */
   /* NOTE - ORDER IS IMPORTANT */
   if ((NULL == CU_add_test(pSuite, "test of dummy init()", test_init)) ||
       (NULL == CU_add_test(pSuite, "test of dummy shutdown()", test_shutdown)) ||
       (NULL == CU_add_test(pSuite, "test of dummy measurement_start()", test_measurement_start)) ||
       (NULL == CU_add_test(pSuite, "test of dummy measurement_stop()", test_measurement_stop))
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
