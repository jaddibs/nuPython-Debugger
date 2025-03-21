/*main.c*/

//
// Main program for running google tests. Do not modify this file.
//
// Prof. Joe Hummel
// Northwestern University
// CS 211
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ram.h" // to use ram_print for debugging

#include "gtest/gtest.h"


//
// main program: do not change this, google test will find all your
// tests and run them. So leave the code below as is:
//
int main() 
{
  //
  // to run just certain tests, uncomment the following and
  // then comment out the line ::testing::InitGoogleTest()
  //
  // Example: run all tests with "back" in their test name:
  //
  // ::testing::GTEST_FLAG(filter) = "*back*";
  //
  // by default configure to run all tests:
  //
  ::testing::InitGoogleTest();

  //
  // run all the tests, returns 0 if all pass and 1 if any fail:
  //
  int result = RUN_ALL_TESTS();

  return result;  
}
