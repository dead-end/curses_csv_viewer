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

#include "ut_utils.h"
#include "ncv_common.h"

#include <stdbool.h>
#include <errno.h>
#include <string.h>
#include <wchar.h>

/******************************************************************************
 * The function checks whether an int parameter has the expected value or not.
 *****************************************************************************/

void ut_check_int(const int current, const int expected, const char *msg) {

	if (current != expected) {
		log_exit("[%s] current: %d expected: %d", msg, current, expected);
	}

	log_debug("[%s] OK current: %d", msg, current);
}

/******************************************************************************
 * The function checks whether an double parameter has the expected value or
 * not.
 *****************************************************************************/

void ut_check_double(const double current, const double expected, const char *msg) {

	if (current != expected) {
		log_exit("[%s] current: %lf expected: %lf", msg, current, expected);
	}

	log_debug("[%s] OK current: %lf", msg, current);
}

/******************************************************************************
 * The function checks whether an size_t parameter has the expected value or
 * not.
 *****************************************************************************/

void ut_check_size(const size_t current, const size_t expected, const char *msg) {

	if (current != expected) {
		log_exit("[%s] current: %zu expected: %zu", msg, current, expected);
	}

	log_debug("[%s] OK size: %zu", msg, current);
}

/******************************************************************************
 * The function compares two wchar_t strings.
 *****************************************************************************/

void ut_check_wchar_str(const wchar_t *current, const wchar_t *expected) {

	if (wcscmp(current, expected) != 0) {
		log_exit("Current: '%ls' expected: '%ls'", current, expected);
	}

	log_debug("OK - Strings are equal: '%ls'", current);
}

/******************************************************************************
 * The function compares two char strings.
 *****************************************************************************/

void ut_check_char_str(const char *current, const char *expected) {

	if (strcmp(current, expected) != 0) {
		log_exit("Current: '%s' expected: '%s'", current, expected);
	}

	log_debug("OK - Strings are equal: '%s'", current);
}

/******************************************************************************
 * The function ensures that a given char string is null or not, depending on
 * the parameter ut_null. The enum ut_null is simply a boolean, but using is
 * makes the code easier to read.
 *
 * ut_check_wchar_null(str, UT_IS_NULL);
 *****************************************************************************/

void ut_check_wcs_null(const wchar_t *str, const enum ut_null_check ut_null) {

	if (ut_null == UT_IS_NULL && str != NULL) {
		log_exit("Pointer is not null: '%ls'", str);
	}

	if (ut_null == UT_IS_NOT_NULL && str == NULL) {
		log_exit_str("Pointer is null!");
	}

	log_debug("OK - String is %s null!", ut_null == UT_IS_NULL ? "" : "not");
}

/******************************************************************************
 * The function compares two wchar_t characters.
 *****************************************************************************/

void ut_check_wchr(const wchar_t current, const wchar_t expected) {

	if (current != expected) {
		log_exit("Current: %lc expected: %lc", current, expected);
	}

	log_debug("OK current: %lc", current);
}

/******************************************************************************
 * The function is used for unit tests. It compares two bool values.
 *****************************************************************************/

void ut_check_bool(const bool current, const bool expected) {

	if (current != expected) {
		log_exit("Current: '%d' expected: '%d'", current, expected);
	}

	log_debug("OK - Boolean are equal: '%d'", current);
}

/******************************************************************************
 * The function checks a s_buffer instance. The s_buffer structure defines a
 * substring of a string. The string may be larger than len, so a string
 * compare may not work. It is enough to compare the pointer, to ensure that
 * the start of the substring is as expected.
 *****************************************************************************/

void ut_check_s_buffer(const s_buffer *buffer, const wchar_t *str, const size_t len, const char *msg) {

	if (buffer->ptr != str) {
		log_exit("[%s] pointer current: %ls expected: %ls", msg, buffer->ptr, str);
	}

	if (buffer->len != len) {
		log_exit("[%s] length current: %zu expected: %zu", msg, buffer->len, len);
	}

	log_debug("[%s] OK \n", msg);
}

/******************************************************************************
 * The function creates a tmp file with a given content.
 *****************************************************************************/

FILE* ut_create_tmp_file(const wchar_t *data) {
	FILE *tmp;

	//
	// Create a temp file
	//
	if ((tmp = tmpfile()) == NULL) {
		log_exit("Unable to create tmp file: %s", strerror(errno));
	}

	//
	// Write the required content
	//
	if (fputws(data, tmp) == -1) {
		log_exit_str("Unable write data to the tmp file!");
	}

	//
	// Rewind the file.
	//
	if (fseek(tmp, 0L, SEEK_SET) == -1) {
		log_exit("Unable to rewind file due to: %s", strerror(errno));
	}

	return tmp;
}

/******************************************************************************
 * The function checks the elements of a column. It is called with an array of
 * row values and the size of the array.
 *****************************************************************************/

void ut_check_table_column(const s_table *table, const int col, const int num_rows, const wchar_t *rows[]) {

	//
	// Ensure that the number of rows is correct.
	//
	ut_check_int(table->no_rows, num_rows, "check num rows");

	for (int row = 0; row < num_rows; row++) {
		ut_check_wchar_str(table->fields[row][col], rows[row]);
	}
}

/******************************************************************************
 * The function checks the elements of a row. It is called with an array of
 * column values and the size of the array.
 *****************************************************************************/

void ut_check_table_row(const s_table *table, const int row, const int num_cols, const wchar_t *cols[]) {

	//
	// Ensure that the number of columns is correct.
	//
	ut_check_int(table->no_columns, num_cols, "check num columns");

	for (int col = 0; col < num_cols; col++) {
		ut_check_wchar_str(table->fields[row][col], cols[col]);
	}
}

/******************************************************************************
 * The function checks an array of int's.
 *****************************************************************************/

void ut_check_int_array(const int current[], const int expected[], const int size, const char *msg) {

	for (int i = 0; i < size; i++) {
		if (current[i] != expected[i]) {
			log_exit("[%s] idx: %d current: %d expected: %d", msg, i, current[i], expected[i]);
		}
	}

	log_debug("[%s] OK", msg);
}

