/*
 * file: ncv_common.h
 */

#ifndef INCLUDE_COMMON_H_
#define INCLUDE_COMMON_H_

//
// Common includes
//
#include <ncursesw/ncurses.h>
#include <ncursesw/form.h>
#include <stdlib.h>
#include <stdio.h>
#include <wchar.h>
#include <stdbool.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>

//
// Definition of the print_debug macro. It uses stdout not to restain curses.
//
#ifdef DEBUG
#define DEBUG_OUT stderr
#define print_debug(fmt, ...) fprintf(DEBUG_OUT, "DEBUG - " fmt, ##__VA_ARGS__)
#define print_debug_str(fmt)  fprintf(DEBUG_OUT, "DEBUG - " fmt)
#else
#define print_debug(fmt, ...)
#define print_debug_str(fmt)
#endif

//
// Definition of the print_error macro
//
#define print_exit(fmt, ...) fprintf(stderr, "FATAL - " fmt, ##__VA_ARGS__); exit(EXIT_FAILURE)
#define print_exit_str(fmt)  fprintf(stderr, "FATAL - " fmt); exit(EXIT_FAILURE)

#define MAX_LINE 1024

#define BUF_SIZE 4096

//
// Some wchar character constants.
//
#define W_NEW_LINE L'\n'
#define W_STR_TERM L'\0'
#define W_QUOTE    L'\"'
#define W_CR       L'\r'
#define W_DELIM    L','

/***************************************************************************
 * Definitions of enums and structs
 **************************************************************************/

//
// A boolean type that can be undefined.
//
enum bool_defined {
	BOOL_TRUE, BOOL_FLASE, BOOL_UNDEF
};

//
// The struct defines a field with rows and columns.
//
typedef struct s_field {

	int row;

	int col;

} s_field;

/***************************************************************************
 * Function definitions
 **************************************************************************/

//
// Unrelated utility functions.
//
void *xmalloc(const size_t size);

FILE *stdin_2_tmp();

//
// Functions that can be used for unit tests.
//
void ut_check_int(const int current, const int expected, const char *msg);

void ut_check_size(const size_t current, const size_t expected, const char *msg);

void ut_check_wchar_str(const wchar_t *str1, const wchar_t *str2);

void ut_check_wchar_null(const wchar_t *str);

#endif /* INCLUDE_COMMON_H_ */
