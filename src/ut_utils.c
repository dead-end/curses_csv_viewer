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

#include "ncv_common.h"

#include <stdbool.h>
#include <errno.h>
#include <string.h>
#include <wchar.h>

/***************************************************************************
 * The function is used for unit tests. It checks whether an int parameter
 * has the expected value or not.
 **************************************************************************/

void ut_check_int(const int current, const int expected, const char *msg) {

	if (current != expected) {
		print_exit("ut_check_int() [%s] current: %d expected: %d\n", msg, current, expected);
	}

	print_debug("ut_check_int() [%s] OK current: %d \n", msg, current);
}

/***************************************************************************
 * The function is used for unit tests. It checks whether an double
 * parameter has the expected value or not.
 **************************************************************************/

void ut_check_double(const double current, const double expected, const char *msg) {

	if (current != expected) {
		print_exit("ut_check_double() [%s] current: %lf expected: %lf\n", msg, current, expected);
	}

	print_debug("ut_check_double() [%s] OK current: %lf \n", msg, current);
}

/***************************************************************************
 * The function is used for unit tests. It checks whether an size_t
 * parameter has the expected value or not.
 **************************************************************************/

void ut_check_size(const size_t current, const size_t expected, const char *msg) {

	if (current != expected) {
		print_exit("ut_check_size() [%s] current: %zu expected: %zu\n", msg, current, expected);
	}

	print_debug("ut_check_size() [%s] OK size: %zu \n", msg, current);
}

/***************************************************************************
 * The function is used for unit tests. It compares two wchar strings.
 **************************************************************************/

void ut_check_wchar_str(const wchar_t *str1, const wchar_t *str2) {

	if (wcscmp(str1, str2) != 0) {
		print_exit("ut_check_wchar_str() Strings differ: '%ls' and: '%ls'\n", str1, str2);
	}

	print_debug("ut_check_wchar_str() OK - Strings are equal: '%ls'\n", str1);
}

/***************************************************************************
 * The function is used for unit tests. It ensures that a given wchar string
 * is null.
 **************************************************************************/

void ut_check_wchar_null(const wchar_t *str) {

	if (str != NULL) {
		print_exit("ut_check_wchar_null() Pointer is not null: '%ls'\n", str);
	}

	print_debug_str("ut_check_wchar_null() OK - String is null!\n");
}

/***************************************************************************
 * The function is used for unit tests. It compares two bool values.
 **************************************************************************/

void ut_check_bool(const bool b1, const bool b2) {

	if (b1 != b2) {
		print_exit("ut_check_bool() Boolean differ: '%d' and: '%d'\n", b1, b2);
	}

	print_debug("ut_check_bool() OK - Boolean are equal: '%d'\n", b1);
}

/***************************************************************************
 * The function checks a s_buffer instance. The s_buffer structure defines
 * a substring of a string. The string may be larger than len, so a string
 * compare may not work. It is enough to compare the pointer, to ensure
 * that the start of the substring is as expected.
 **************************************************************************/

void ut_check_s_buffer(const s_buffer *buffer, const wchar_t *str, const size_t len, const char *msg) {

	if (buffer->ptr != str) {
		print_exit("ut_check_s_buffer() [%s] pointer current: %ls expected: %ls\n", msg, buffer->ptr, str);
	}

	if (buffer->len != len) {
		print_exit("ut_check_s_buffer() [%s] length current: %zu expected: %zu\n", msg, buffer->len, len);
	}

	print_debug("ut_check_s_buffer() [%s] OK \n", msg);
}

/***************************************************************************
 * The function creates a tmp file with a given content.
 **************************************************************************/

FILE *ut_create_tmp_file(const wchar_t *data) {
	FILE *tmp;

	//
	// Create a temp file
	//
	if ((tmp = tmpfile()) == NULL) {
		print_exit("create_tmp_file() Unable to create tmp file: %s\n", strerror(errno));
	}

	//
	// Write the required content
	//
	if (fputws(data, tmp) == -1) {
		print_exit_str("create_tmp_file() Unable write data to the tmp file!\n");
	}

	//
	// Rewind the file.
	//
	if (fseek(tmp, 0L, SEEK_SET) == -1) {
		print_exit("create_tmp_file() Unable to rewind file due to: %s\n", strerror(errno));
	}

	return tmp;
}

