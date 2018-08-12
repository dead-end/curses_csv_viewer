/*
 * MIT License
 *
 * Copyright (c) 2018 dead-end
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#ifndef INCLUDE_COMMON_H_
#define INCLUDE_COMMON_H_

#include <ncursesw/ncurses.h>
#include <ncursesw/form.h>
#include <stdlib.h>
#include <stdio.h>
#include <wchar.h>
#include <stdbool.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <locale.h>
#include <ctype.h>

/***************************************************************************
 * Definition of the print_debug macro. It is only defined if the DEBUG flag
 * is defined. It prints to stderr not to restrain curses.
 **************************************************************************/

#ifdef DEBUG
#define print_debug(fmt, ...) fprintf(stderr, "DEBUG - " fmt, ##__VA_ARGS__)
#define print_debug_str(fmt)  fprintf(stderr, "DEBUG - " fmt)
#else
#define print_debug(fmt, ...)
#define print_debug_str(fmt)
#endif

/***************************************************************************
 * Definition of the print_error macro, that finishes the program after
 * printing the error message.
 **************************************************************************/

#define print_exit(fmt, ...) fprintf(stderr, "FATAL - " fmt, ##__VA_ARGS__); exit(EXIT_FAILURE)
#define print_exit_str(fmt)  fprintf(stderr, "FATAL - " fmt); exit(EXIT_FAILURE)

/***************************************************************************
 * Buffer and line sizes.
 **************************************************************************/

#define MAX_LINE 1024

#define BUF_SIZE 4096

/***************************************************************************
 * Some wchar character constants.
 **************************************************************************/

#define W_NEW_LINE L'\n'
#define W_STR_TERM L'\0'
#define W_QUOTE    L'\"'
#define W_CR       L'\r'
#define W_DELIM    L','

/***************************************************************************
 * A boolean type that can be undefined.
 **************************************************************************/

enum bool_defined {
	BOOL_TRUE, BOOL_FLASE, BOOL_UNDEF
};

/***************************************************************************
 * The struct defines a field with rows and columns.
 **************************************************************************/

typedef struct s_field {

	//
	// The index of the fields row.
	//
	int row;

	//
	// The index of the fields column.
	//
	int col;

} s_field;

/***************************************************************************
 * Various function definitions.
 **************************************************************************/

void *xmalloc(const size_t size);

FILE *stdin_2_tmp();

size_t mbs_2_wchars(const char *mbs, wchar_t *buffer, const int buf_size);

char *trim(char *str);

/***************************************************************************
 * Functions that can be used for unit tests.
 **************************************************************************/

void ut_check_int(const int current, const int expected, const char *msg);

void ut_check_size(const size_t current, const size_t expected, const char *msg);

void ut_check_wchar_str(const wchar_t *str1, const wchar_t *str2);

void ut_check_wchar_null(const wchar_t *str);

void ut_check_bool(const bool b1, const bool b2);

#endif
