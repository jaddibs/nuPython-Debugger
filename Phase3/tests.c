/*tests.c*/

//
// Google Test unit tests to ensure correct behavior/results of methods in ram.h
// All ram.h methods tested except for ram_print() and ram_destroy()
//
// Jad Dibs
//
// Initial template: Prof. Joe Hummel
// Northwestern University
// CS 211
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ram.h"
#include "gtest/gtest.h"

//
// private helper functions:
//


//
// some provided unit tests to get started:
//
TEST(memory_module, initialization)
{
  //
  // create a new memory and make sure it's initialized properly:
  //
  struct RAM* memory = ram_init();

  ASSERT_TRUE(memory != NULL);        // use ASSERT_TRUE with pointers
  ASSERT_TRUE(memory->cells != NULL);

  ASSERT_EQ(memory->num_values, 0);  // use ASSERT_EQ for comparing values
  ASSERT_EQ(memory->capacity, 4);

  //
  // tests passed, free memory
  //
  ram_destroy(memory);
}

TEST(memory_module, read_by_name_does_not_exist) 
{
  //
  // create a new memory:
  //
  struct RAM* memory = ram_init();

  //
  // read a var that doesn't exist:
  //
  struct RAM_VALUE* value = ram_read_cell_by_name(memory, "x");
  ASSERT_TRUE(value == NULL);  // use ASSERT_TRUE with pointers

  //
  // tests passed, free memory
  //
  ram_destroy(memory);
}

TEST(memory_module, write_one_int) 
{
  //
  // create a new memory:
  //
  struct RAM* memory = ram_init();

  //
  // store the integer 123:
  //
  struct RAM_VALUE i;

  i.value_type = RAM_TYPE_INT;
  i.types.i = 123;

  bool success = ram_write_cell_by_name(memory, i, "x");
  ASSERT_TRUE(success);

  //
  // now check the memory, was x = 123 stored properly?
  //
  ASSERT_EQ(memory->num_values, 1);
  ASSERT_EQ(memory->cells[0].value.value_type, RAM_TYPE_INT);
  ASSERT_EQ(memory->cells[0].value.types.i, 123);
  ASSERT_STREQ(memory->cells[0].identifier, "x");  // strings => ASSERT_STREQ

  //
  // tests passed, free memory
  //
  ram_destroy(memory);
}

//
// TODO: add lots more unit tests
//

//
// ram_init method tests
//
TEST(memory_module, ram_init_method)
{
  //
  // create a new memory and make sure it's initialized properly:
  //
  struct RAM* memory = ram_init(); 

  ASSERT_TRUE(memory != NULL);        // use ASSERT_TRUE with pointers
  ASSERT_TRUE(memory->cells != NULL);

  ASSERT_EQ(memory->num_values, 0);  // use ASSERT_EQ for comparing values
  ASSERT_EQ(memory->capacity, 4);

  // ensure memory cells are None
  for (int i = 0; i < memory->capacity; i++)
  {
    ASSERT_TRUE(memory->cells[i].identifier == NULL);
    ASSERT_EQ(memory->cells[i].value.value_type, RAM_TYPE_NONE);
  }

  //
  // tests passed, free memory
  //
  ram_destroy(memory);
}

//
// ram_write_cell_by_name method tests
//
TEST(memory_module, write_cell_by_name_one_int)
{
  struct RAM* memory = ram_init();

  struct RAM_VALUE value;
  value.value_type = RAM_TYPE_INT;
  value.types.i = 123;
  
  bool success = ram_write_cell_by_name(memory, value, "a");

  ASSERT_STREQ(memory->cells[0].identifier, "a");
  ASSERT_TRUE(memory->cells[0].value.value_type == RAM_TYPE_INT);
  ASSERT_EQ(memory->cells[0].value.types.i, 123);
  ASSERT_EQ(memory->num_values, 1);
  ASSERT_EQ(memory->capacity, 4);

  ASSERT_TRUE(memory->cells[1].value.value_type == RAM_TYPE_NONE);

  ram_destroy(memory);
}

TEST(memory_module, write_cell_by_name_one_double)
{
  struct RAM* memory = ram_init();

  struct RAM_VALUE value;
  value.value_type = RAM_TYPE_REAL;
  value.types.d = 1.23;
  
  bool success = ram_write_cell_by_name(memory, value, "a");

  ASSERT_STREQ(memory->cells[0].identifier, "a");
  ASSERT_TRUE(memory->cells[0].value.value_type == RAM_TYPE_REAL);
  ASSERT_EQ(memory->cells[0].value.types.d, 1.23);
  ASSERT_EQ(memory->num_values, 1);
  ASSERT_EQ(memory->capacity, 4);

  ASSERT_TRUE(memory->cells[1].value.value_type == RAM_TYPE_NONE);

  ram_destroy(memory);
}

TEST(memory_module, write_cell_by_name_one_string)
{
  struct RAM* memory = ram_init();

  struct RAM_VALUE value;
  value.value_type = RAM_TYPE_STR;
  value.types.s = "stringvar";
  
  bool success = ram_write_cell_by_name(memory, value, "a");

  ASSERT_STREQ(memory->cells[0].identifier, "a");
  ASSERT_TRUE(memory->cells[0].value.value_type == RAM_TYPE_STR);
  ASSERT_STREQ(memory->cells[0].value.types.s, "stringvar");

  ASSERT_TRUE(value.types.s != memory->cells[0].value.types.s); // duplication check for char*

  ASSERT_EQ(memory->num_values, 1);
  ASSERT_EQ(memory->capacity, 4);

  ASSERT_TRUE(memory->cells[1].value.value_type == RAM_TYPE_NONE);

  ram_destroy(memory);
}

TEST(memory_module, write_cell_by_name_one_pointer)
{
  struct RAM* memory = ram_init();

  struct RAM_VALUE value;
  value.value_type = RAM_TYPE_PTR;
  value.types.i = 12345678;
  
  bool success = ram_write_cell_by_name(memory, value, "a");

  ASSERT_STREQ(memory->cells[0].identifier, "a");
  ASSERT_TRUE(memory->cells[0].value.value_type == RAM_TYPE_PTR);
  ASSERT_EQ(memory->cells[0].value.types.i, 12345678);
  ASSERT_EQ(memory->num_values, 1);
  ASSERT_EQ(memory->capacity, 4);

  ASSERT_TRUE(memory->cells[1].value.value_type == RAM_TYPE_NONE);

  ram_destroy(memory);
}

TEST(memory_module, write_cell_by_name_one_boolean)
{
  struct RAM* memory = ram_init();

  struct RAM_VALUE value;
  value.value_type = RAM_TYPE_BOOLEAN;
  value.types.i = 1;
  
  bool success = ram_write_cell_by_name(memory, value, "a");

  ASSERT_STREQ(memory->cells[0].identifier, "a");
  ASSERT_TRUE(memory->cells[0].value.value_type == RAM_TYPE_BOOLEAN);
  ASSERT_EQ(memory->cells[0].value.types.i, 1);
  ASSERT_EQ(memory->num_values, 1);
  ASSERT_EQ(memory->capacity, 4);

  ASSERT_TRUE(memory->cells[1].value.value_type == RAM_TYPE_NONE);

  ram_destroy(memory);
}

TEST(memory_module, write_cell_by_name_overwrite)
{
  struct RAM* memory = ram_init();

  struct RAM_VALUE value1;
  value1.value_type = RAM_TYPE_INT;
  value1.types.i = 123;
  
  bool success1 = ram_write_cell_by_name(memory, value1, "a");

  ASSERT_STREQ(memory->cells[0].identifier, "a");
  ASSERT_TRUE(memory->cells[0].value.value_type == RAM_TYPE_INT);
  ASSERT_EQ(memory->cells[0].value.types.i, 123);
  ASSERT_EQ(memory->num_values, 1);
  ASSERT_EQ(memory->capacity, 4);

  ASSERT_TRUE(memory->cells[1].value.value_type == RAM_TYPE_NONE);

  struct RAM_VALUE value2;
  value2.value_type = RAM_TYPE_STR;
  value2.types.s = "stringvar";

  bool success2 = ram_write_cell_by_name(memory, value2, "a");

  ASSERT_STREQ(memory->cells[0].identifier, "a");
  ASSERT_TRUE(memory->cells[0].value.value_type == RAM_TYPE_STR);
  ASSERT_STREQ(memory->cells[0].value.types.s, "stringvar");

  ASSERT_TRUE(value2.types.s != memory->cells[0].value.types.s); // duplication check for char*

  ASSERT_EQ(memory->num_values, 1);
  ASSERT_EQ(memory->capacity, 4);

  ASSERT_TRUE(memory->cells[1].value.value_type == RAM_TYPE_NONE);

  ram_destroy(memory);
}

TEST(memory_module, write_cell_by_name_at_cap)
{
  struct RAM* memory = ram_init();

  char* names[4] = {"a", "b", "c", "d"};

  for (int i = 0; i < memory->capacity; i++)
  {
    struct RAM_VALUE value;

    if (i == 0)
    {
      value.value_type = RAM_TYPE_INT;
      value.types.i = 123;
      
      bool success = ram_write_cell_by_name(memory, value, names[i]);
    
      ASSERT_STREQ(memory->cells[i].identifier, names[i]);
      ASSERT_TRUE(memory->cells[i].value.value_type == RAM_TYPE_INT);
      ASSERT_EQ(memory->cells[i].value.types.i, 123);
      ASSERT_EQ(memory->num_values, i+1);
      ASSERT_EQ(memory->capacity, 4);
    
      ASSERT_TRUE(memory->cells[i+1].value.value_type == RAM_TYPE_NONE);
    }
    else if (i == 1)
    {
      value.value_type = RAM_TYPE_STR;
      value.types.s = "stringvar";
      
      bool success = ram_write_cell_by_name(memory, value, names[i]);
    
      ASSERT_STREQ(memory->cells[i].identifier, names[i]);
      ASSERT_TRUE(memory->cells[i].value.value_type == RAM_TYPE_STR);
      ASSERT_STREQ(memory->cells[i].value.types.s, "stringvar");
    
      ASSERT_TRUE(value.types.s != memory->cells[i].value.types.s); // duplication check for char*
    
      ASSERT_EQ(memory->num_values, i+1);
      ASSERT_EQ(memory->capacity, 4);
    
      ASSERT_TRUE(memory->cells[i+1].value.value_type == RAM_TYPE_NONE);
    }
    else if (i == 2)
    {
      value.value_type = RAM_TYPE_REAL;
      value.types.d = 1.23;
      
      bool success = ram_write_cell_by_name(memory, value, names[i]);
    
      ASSERT_STREQ(memory->cells[i].identifier, names[i]);
      ASSERT_TRUE(memory->cells[i].value.value_type == RAM_TYPE_REAL);
      ASSERT_EQ(memory->cells[i].value.types.d, 1.23);
      ASSERT_EQ(memory->num_values, i+1);
      ASSERT_EQ(memory->capacity, 4);
    
      ASSERT_TRUE(memory->cells[i+1].value.value_type == RAM_TYPE_NONE);
    }
    else
    {
      value.value_type = RAM_TYPE_BOOLEAN;
      value.types.i = 0;
      
      bool success = ram_write_cell_by_name(memory, value, names[i]);
    
      ASSERT_STREQ(memory->cells[i].identifier, names[i]);
      ASSERT_TRUE(memory->cells[i].value.value_type == RAM_TYPE_BOOLEAN);
      ASSERT_EQ(memory->cells[i].value.types.i, 0);
      ASSERT_EQ(memory->num_values, i+1);
      ASSERT_EQ(memory->capacity, 4);
    }
  }

  ram_destroy(memory);
}

TEST(memory_module, write_cell_by_name_over_cap)
{
  struct RAM* memory = ram_init();

  char* names[40] = {"a", "b", "c", "d", "e", "f", "g", "h", "i", "j", "k", "l", "m", "n", "o", "p", "q", "r", "s", "t", "u", "v", "w", "x", "y", "z", "A", "B", "C", "D", "E", "F", "G", "H", "I", "J", "K", "L", "M", "N"};

  int cap = 4;

  for (int i = 0; i < 40; i++)
  {
    if (i == 4 || i == 8 || i == 16 || i == 32)
      cap *= 2;

    struct RAM_VALUE value;

    if (i % 4 == 0)
    {
      value.value_type = RAM_TYPE_INT;
      value.types.i = 123;
      
      bool success = ram_write_cell_by_name(memory, value, names[i]);
    
      ASSERT_STREQ(memory->cells[i].identifier, names[i]);
      ASSERT_TRUE(memory->cells[i].value.value_type == RAM_TYPE_INT);
      ASSERT_EQ(memory->cells[i].value.types.i, 123);
      ASSERT_EQ(memory->num_values, i+1);
      ASSERT_EQ(memory->capacity, cap);
    
      ASSERT_TRUE(memory->cells[i+1].value.value_type == RAM_TYPE_NONE);
    }
    else if (i % 4 == 1)
    {
      value.value_type = RAM_TYPE_STR;
      value.types.s = "stringvar";
      
      bool success = ram_write_cell_by_name(memory, value, names[i]);
    
      ASSERT_STREQ(memory->cells[i].identifier, names[i]);
      ASSERT_TRUE(memory->cells[i].value.value_type == RAM_TYPE_STR);
      ASSERT_STREQ(memory->cells[i].value.types.s, "stringvar");
    
      ASSERT_TRUE(value.types.s != memory->cells[i].value.types.s); // duplication check for char*
    
      ASSERT_EQ(memory->num_values, i+1);
      ASSERT_EQ(memory->capacity, cap);
    
      ASSERT_TRUE(memory->cells[i+1].value.value_type == RAM_TYPE_NONE);
    }
    else if (i % 4 == 2)
    {
      value.value_type = RAM_TYPE_REAL;
      value.types.d = 1.23;
      
      bool success = ram_write_cell_by_name(memory, value, names[i]);
    
      ASSERT_STREQ(memory->cells[i].identifier, names[i]);
      ASSERT_TRUE(memory->cells[i].value.value_type == RAM_TYPE_REAL);
      ASSERT_EQ(memory->cells[i].value.types.d, 1.23);
      ASSERT_EQ(memory->num_values, i+1);
      ASSERT_EQ(memory->capacity, cap);
    
      ASSERT_TRUE(memory->cells[i+1].value.value_type == RAM_TYPE_NONE);
    }
    else
    {
      value.value_type = RAM_TYPE_BOOLEAN;
      value.types.i = 0;
      
      bool success = ram_write_cell_by_name(memory, value, names[i]);
    
      ASSERT_STREQ(memory->cells[i].identifier, names[i]);
      ASSERT_TRUE(memory->cells[i].value.value_type == RAM_TYPE_BOOLEAN);
      ASSERT_EQ(memory->cells[i].value.types.i, 0);
      ASSERT_EQ(memory->num_values, i+1);
      ASSERT_EQ(memory->capacity, cap);
    }
  }

  ram_destroy(memory);
}

//
// ram_read_cell_by_name method tests
//
TEST(memory_module, read_cell_by_name_empty)
{
  struct RAM* memory = ram_init();

  struct RAM_VALUE* res = ram_read_cell_by_name(memory, "doesn't exist");

  ASSERT_TRUE(res == NULL);

  ram_destroy(memory);
}

TEST(memory_module, read_cell_by_name_one_int)
{
  struct RAM* memory = ram_init();

  struct RAM_VALUE value;
  value.value_type = RAM_TYPE_INT;
  value.types.i = 123;
  
  bool success = ram_write_cell_by_name(memory, value, "a");

  struct RAM_VALUE* res = ram_read_cell_by_name(memory, "a");

  ASSERT_TRUE(res->value_type == RAM_TYPE_INT);
  ASSERT_TRUE(res->types.i == 123);

  ram_free_value(res);
  ram_destroy(memory);
}

TEST(memory_module, read_cell_by_name_one_double)
{
  struct RAM* memory = ram_init();

  struct RAM_VALUE value;
  value.value_type = RAM_TYPE_REAL;
  value.types.d = 1.23;
  
  bool success = ram_write_cell_by_name(memory, value, "a");

  struct RAM_VALUE* res = ram_read_cell_by_name(memory, "a");

  ASSERT_TRUE(res->value_type == RAM_TYPE_REAL);
  ASSERT_TRUE(res->types.d == 1.23);

  ram_free_value(res);
  ram_destroy(memory);
}

TEST(memory_module, read_cell_by_name_one_string)
{
  struct RAM* memory = ram_init();

  struct RAM_VALUE value;
  value.value_type = RAM_TYPE_STR;
  value.types.s = "stringvar";
  
  bool success = ram_write_cell_by_name(memory, value, "a");

  struct RAM_VALUE* res = ram_read_cell_by_name(memory, "a");

  ASSERT_TRUE(res->value_type == RAM_TYPE_STR);
  ASSERT_STREQ(res->types.s, "stringvar");

  ram_free_value(res);
  ram_destroy(memory);
}

TEST(memory_module, read_cell_by_name_one_pointer)
{
  struct RAM* memory = ram_init();

  struct RAM_VALUE value;
  value.value_type = RAM_TYPE_PTR;
  value.types.i = 12345678;
  
  bool success = ram_write_cell_by_name(memory, value, "a");

  struct RAM_VALUE* res = ram_read_cell_by_name(memory, "a");

  ASSERT_TRUE(res->value_type == RAM_TYPE_PTR);
  ASSERT_TRUE(res->types.i == 12345678);

  ram_free_value(res);
  ram_destroy(memory);
}

TEST(memory_module, read_cell_by_name_one_boolean)
{
  struct RAM* memory = ram_init();

  struct RAM_VALUE value;
  value.value_type = RAM_TYPE_BOOLEAN;
  value.types.i = 1;
  
  bool success = ram_write_cell_by_name(memory, value, "a");

  struct RAM_VALUE* res = ram_read_cell_by_name(memory, "a");

  ASSERT_TRUE(res->value_type == RAM_TYPE_BOOLEAN);
  ASSERT_TRUE(res->types.i == 1);

  ram_free_value(res);
  ram_destroy(memory);
}

TEST(memory_module, read_cell_by_name_overwrite)
{
  struct RAM* memory = ram_init();

  struct RAM_VALUE value1;
  value1.value_type = RAM_TYPE_INT;
  value1.types.i = 123;
  
  bool success1 = ram_write_cell_by_name(memory, value1, "a");

  struct RAM_VALUE* res1 = ram_read_cell_by_name(memory, "a");

  ASSERT_TRUE(res1->value_type == RAM_TYPE_INT);
  ASSERT_TRUE(res1->types.i == 123);

  struct RAM_VALUE value2;
  value2.value_type = RAM_TYPE_STR;
  value2.types.s = "stringvar";

  bool success2 = ram_write_cell_by_name(memory, value2, "a");

  struct RAM_VALUE* res2 = ram_read_cell_by_name(memory, "a");

  ASSERT_TRUE(res2->value_type == RAM_TYPE_STR);
  ASSERT_STREQ(res2->types.s, "stringvar");

  ASSERT_TRUE(res2->types.s != value2.types.s);
  
  ram_free_value(res1);
  ram_free_value(res2);
  ram_destroy(memory);
}

//
// ram_write_cell_by_addr method tests
//
TEST(memory_module, write_cell_by_addr_slightly_off)
{
  struct RAM* memory = ram_init();

  struct RAM_VALUE value;
  value.value_type = RAM_TYPE_STR;
  value.types.s = "doesn't matter";

  bool check1 = ram_write_cell_by_addr(memory, value, memory->num_values);
  bool check2 = ram_write_cell_by_addr(memory, value, -1);

  ASSERT_TRUE(!check1);
  ASSERT_TRUE(!check2);

  ram_destroy(memory);
}

TEST(memory_module, write_cell_by_addr_str_overwrite)
{
  struct RAM* memory = ram_init();

  struct RAM_VALUE init_value;
  init_value.value_type = RAM_TYPE_STR;
  init_value.types.s = "stringvar";
  
  bool first = ram_write_cell_by_name(memory, init_value, "a");

  struct RAM_VALUE value;
  value.value_type = RAM_TYPE_STR;
  value.types.s = "overwrite";

  bool success = ram_write_cell_by_addr(memory, value, 0);

  ASSERT_STREQ(memory->cells[0].identifier, "a");
  ASSERT_TRUE(memory->cells[0].value.value_type == RAM_TYPE_STR);
  ASSERT_STREQ(memory->cells[0].value.types.s, "overwrite");

  ASSERT_TRUE(value.types.s != memory->cells[0].value.types.s); // duplication check for char*

  ASSERT_EQ(memory->num_values, 1);
  ASSERT_EQ(memory->capacity, 4);

  ram_destroy(memory);
}

TEST(memory_module, write_cell_by_addr_int_overwrite)
{
  struct RAM* memory = ram_init();

  struct RAM_VALUE init_value;
  init_value.value_type = RAM_TYPE_STR;
  init_value.types.s = "stringvar";
  
  bool first = ram_write_cell_by_name(memory, init_value, "a");

  struct RAM_VALUE value;
  value.value_type = RAM_TYPE_INT;
  value.types.i = 1;

  bool success = ram_write_cell_by_addr(memory, value, 0);

  ASSERT_STREQ(memory->cells[0].identifier, "a");
  ASSERT_TRUE(memory->cells[0].value.value_type == RAM_TYPE_INT);
  ASSERT_EQ(memory->cells[0].value.types.i, 1);
  ASSERT_EQ(memory->num_values, 1);
  ASSERT_EQ(memory->capacity, 4);

  ram_destroy(memory);
}

//
// ram_read_cell_by_addr method tests
//
TEST(memory_module, read_cell_by_addr_empty)
{
  struct RAM* memory = ram_init();

  struct RAM_VALUE* res = ram_read_cell_by_addr(memory, 0);

  ASSERT_TRUE(res == NULL);

  ram_destroy(memory);
}

TEST(memory_module, read_cell_by_addr_not_empty)
{
  struct RAM* memory = ram_init();

  struct RAM_VALUE init_value;
  init_value.value_type = RAM_TYPE_STR;
  init_value.types.s = "stringvar";
  
  bool success = ram_write_cell_by_name(memory, init_value, "a");

  struct RAM_VALUE* res = ram_read_cell_by_addr(memory, 0);

  ASSERT_TRUE(res->value_type == RAM_TYPE_STR);
  ASSERT_STREQ(res->types.s, "stringvar");

  ASSERT_TRUE(res->types.s != memory->cells[0].value.types.s); // duplication check for char*

  ASSERT_EQ(memory->num_values, 1);
  ASSERT_EQ(memory->capacity, 4);

  ram_free_value(res);
  ram_destroy(memory);  
}

//
// ram_get_addr method tests
//
TEST(memory_module, get_addr_empty)
{
  struct RAM* memory = ram_init();

  int res = ram_get_addr(memory, "doesn't exist");

  ASSERT_TRUE(res == -1);

  ram_destroy(memory);
}

TEST(memory_module, get_addr_not_empty)
{
  struct RAM* memory = ram_init();

  struct RAM_VALUE init_value;
  init_value.value_type = RAM_TYPE_STR;
  init_value.types.s = "stringvar";
  
  bool success = ram_write_cell_by_name(memory, init_value, "a");

  int pos = ram_get_addr(memory, "a");

  ASSERT_TRUE(pos == 0);

  ram_destroy(memory);
}

//
// Comprehensive
//
TEST(memory_module, over_cap)
{
  struct RAM* memory = ram_init();

  char* names[40] = {"a", "b", "c", "d", "e", "f", "g", "h", "i", "j", "k", "l", "m", "n", "o", "p", "q", "r", "s", "t", "u", "v", "w", "x", "y", "z", "A", "B", "C", "D", "E", "F", "G", "H", "I", "J", "K", "L", "M", "N"};

  int cap = 4;

  for (int i = 0; i < 40; i++)
  {
    if (i == 4 || i == 8 || i == 16 || i == 32)
      cap *= 2;

    struct RAM_VALUE value;

    if (i % 4 == 0)
    {
      value.value_type = RAM_TYPE_INT;
      value.types.i = 123;
      
      bool success = ram_write_cell_by_name(memory, value, names[i]);
    
      ASSERT_STREQ(memory->cells[i].identifier, names[i]);
      ASSERT_TRUE(memory->cells[i].value.value_type == RAM_TYPE_INT);
      ASSERT_EQ(memory->cells[i].value.types.i, 123);
      ASSERT_EQ(memory->num_values, i+1);
      ASSERT_EQ(memory->capacity, cap);
    
      ASSERT_TRUE(memory->cells[i+1].value.value_type == RAM_TYPE_NONE);
    }
    else if (i % 4 == 1)
    {
      value.value_type = RAM_TYPE_STR;
      value.types.s = "stringvar";
      
      bool success1 = ram_write_cell_by_name(memory, value, names[i]);
    
      ASSERT_STREQ(memory->cells[i].identifier, names[i]);
      ASSERT_TRUE(memory->cells[i].value.value_type == RAM_TYPE_STR);
      ASSERT_STREQ(memory->cells[i].value.types.s, "stringvar");
    
      ASSERT_TRUE(value.types.s != memory->cells[i].value.types.s); // duplication check for char*
    
      ASSERT_EQ(memory->num_values, i+1);
      ASSERT_EQ(memory->capacity, cap);
    
      ASSERT_TRUE(memory->cells[i+1].value.value_type == RAM_TYPE_NONE);
      
      struct RAM_VALUE* res1 = ram_read_cell_by_name(memory, names[i]);

      ASSERT_TRUE(res1->value_type == RAM_TYPE_STR);
      ASSERT_STREQ(res1->types.s, "stringvar");
      ASSERT_TRUE(res1->types.s != memory->cells[i].value.types.s); // duplication check for char*
    
      ram_free_value(res1);

      int pos = ram_get_addr(memory, names[i]);

      ASSERT_TRUE(pos == i);

      struct RAM_VALUE value_ow;
      value_ow.value_type = RAM_TYPE_STR;
      value_ow.types.s = "overwrite";

      bool success2 = ram_write_cell_by_addr(memory, value_ow, pos);

      ASSERT_STREQ(memory->cells[pos].identifier, names[i]);
      ASSERT_TRUE(memory->cells[pos].value.value_type == RAM_TYPE_STR);
      ASSERT_STREQ(memory->cells[pos].value.types.s, "overwrite");

      ASSERT_TRUE(value_ow.types.s != memory->cells[0].value.types.s); // duplication check for char*

      ASSERT_EQ(memory->num_values, i+1);
      ASSERT_EQ(memory->capacity, cap);

      struct RAM_VALUE* res2 = ram_read_cell_by_addr(memory, i);

      ASSERT_TRUE(res2->value_type == RAM_TYPE_STR);
      ASSERT_STREQ(res2->types.s, "overwrite");
    
      ASSERT_TRUE(res2->types.s != memory->cells[i].value.types.s); // duplication check for char*
    
      ASSERT_EQ(memory->num_values, i+1);
      ASSERT_EQ(memory->capacity, cap);
    
      ram_free_value(res2);
    }
    else if (i % 4 == 2)
    {
      value.value_type = RAM_TYPE_REAL;
      value.types.d = 1.23;
      
      bool success = ram_write_cell_by_name(memory, value, names[i]);
    
      ASSERT_STREQ(memory->cells[i].identifier, names[i]);
      ASSERT_TRUE(memory->cells[i].value.value_type == RAM_TYPE_REAL);
      ASSERT_EQ(memory->cells[i].value.types.d, 1.23);
      ASSERT_EQ(memory->num_values, i+1);
      ASSERT_EQ(memory->capacity, cap);
    
      ASSERT_TRUE(memory->cells[i+1].value.value_type == RAM_TYPE_NONE);
    }
    else
    {
      value.value_type = RAM_TYPE_BOOLEAN;
      value.types.i = 0;
      
      bool success = ram_write_cell_by_name(memory, value, names[i]);
    
      ASSERT_STREQ(memory->cells[i].identifier, names[i]);
      ASSERT_TRUE(memory->cells[i].value.value_type == RAM_TYPE_BOOLEAN);
      ASSERT_EQ(memory->cells[i].value.types.i, 0);
      ASSERT_EQ(memory->num_values, i+1);
      ASSERT_EQ(memory->capacity, cap);
    }
  }

  ram_destroy(memory);
}