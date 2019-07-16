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

#ifndef INC_NCV_TABLE_HEADER_H_
#define INC_NCV_TABLE_HEADER_H_

#include "ncv_table.h"

#include <stdbool.h>

bool s_table_has_header(const s_table *table);

/******************************************************************************
 * Some function definitions that are used only for unit tests.
 *****************************************************************************/

int check_column_characteristic(const s_table *table, const int max_rows, const int column, double (*fct_ptr)(const wchar_t *str));

double get_ratio(const wchar_t *str);

double get_str_len(const wchar_t *str);

double get_table_mean(const s_table *table, const int max_rows, const int column, double (*fct_ptr)(const wchar_t *str));

double get_table_std_dev(const s_table *table, const int max_rows, const int column, double (*fct_ptr)(const wchar_t *str), const double mean);

#endif /* INC_NCV_TABLE_HEADER_H_ */
