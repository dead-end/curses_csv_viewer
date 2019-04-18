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

#include "ncv_common.h"

#include <stdbool.h>

void ut_check_int(const int current, const int expected, const char *msg);

void ut_check_double(const double current, const double expected, const char *msg);

void ut_check_size(const size_t current, const size_t expected, const char *msg);

void ut_check_wchar_str(const wchar_t *str1, const wchar_t *str2);

void ut_check_wchar_null(const wchar_t *str);

void ut_check_char_null(const char *str, const bool is_null);

void ut_check_bool(const bool b1, const bool b2);

void ut_check_s_buffer(const s_buffer *buffer, const wchar_t *str, const size_t len, const char *msg);

FILE *ut_create_tmp_file(const wchar_t *data);

#endif
