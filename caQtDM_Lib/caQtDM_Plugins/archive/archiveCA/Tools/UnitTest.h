// -*- c++ -*-

// Simple Unit-Test Framework.
//
// makeUnitTestMain.pl parses current directory,
// creating UnitTest.cpp test suite and
// UnitTest.mk makefile snippet.
//
// See Tools/*Test.cpp for example test cases
// and Tools/Makefile for how to build and run.

#include <stdio.h>

typedef bool TEST_CASE;

#define COMMENT(msg)                    \
           printf("  ----- %s\n", msg);

#define FAIL(msg)                       \
       {                                \
           printf("  FAIL: %s\n", msg); \
           return false;                \
       }

#define PASS(msg)                       \
           printf("  OK  : %s\n", msg);

#define TEST(t)                        \
       if (t)                          \
           printf("  OK  : %s\n", #t); \
       else                            \
       {                               \
           printf("  FAIL: %s\n", #t); \
           return false;               \
       }

#define TEST_MSG(t,msg)                 \
       if (t)                           \
           printf("  OK  : %s\n", msg); \
       else                             \
       {                                \
           printf("  FAIL: %s\n", msg); \
           return false;                \
       }

// Deletes the given file.
bool test_delete_file(const char *filename);

#define TEST_DELETE_FILE(file)                                 \
       if (test_delete_file(file))                             \
           printf("  OK  : '%s' was removed\n", file);         \
       else                                                    \
       {                                                       \
           printf("  FAIL: cannot remove file '%s'\n", file);  \
           return false;                                       \
       }


// Runs 'diff', returns true when files match.
bool test_filediff(const char *filename1, const char *filename2);

#define TEST_FILEDIFF(file1,file2)                                \
       if (test_filediff(file1,file2))                            \
           printf("  OK  : '%s' and '%s' match\n", file1, file2); \
       else                                                       \
       {                                                          \
           printf("  FAIL: diff %s %s\n", file1, file2);          \
           return false;                                          \
       }

#define TEST_OK   return true

