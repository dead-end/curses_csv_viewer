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

#ifndef INC_UT_UTILS_H_
#define INC_UT_UTILS_H_

#include "ncv_table.h"
#include "ncv_common.h"

#include <stdbool.h>

void ut_check_int(const int current, const int expected, const char *msg);

void ut_check_double(const double current, const double expected, const char *msg);

void ut_check_size(const size_t current, const size_t expected, const char *msg);

void ut_check_wchar_str(const wchar_t *str1, const wchar_t *str2);

void ut_check_char_str(const char *str1, const char *str2);

void ut_check_wchr(const wchar_t current, const wchar_t expected);

void ut_check_bool(const bool b1, const bool b2);

void ut_check_s_buffer(const s_buffer *buffer, const wchar_t *str, const size_t len, const char *msg);

FILE* ut_create_tmp_file(const wchar_t *data);

//
// The enum is simply a boolean value. When we are using the enum, the code is
// much clearer than using 'true' or false'.
//
// ut_check_wchar_null(str, UT_IS_NULL);
//
enum ut_null_check {
	UT_IS_NULL, UT_IS_NOT_NULL
};

void ut_check_wcs_null(const wchar_t *str, const enum ut_null_check ut_null);

void check_table_column(s_table *table, const int column, const int num_rows, const wchar_t *fields[]);

#endif
