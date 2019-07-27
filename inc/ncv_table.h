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

#ifndef INC_NCV_TABLE_H_
#define INC_NCV_TABLE_H_

#include "ncv_sort.h"
#include "ncv_filter.h"
#include "ncv_cursor.h"
#include "ncv_common.h"

/******************************************************************************
 * The structure contains all the table related data, that is the csv data, the
 * number of rows and columns, the height of the rows and the width of the
 * columns. The members with the "__" contain the original data, while the
 * members with the same name (may) contain filtered data, which is a subset of
 * the original data.
 *****************************************************************************/

typedef struct s_table {

	//
	// The number of columns and rows of the table. The parameter __no_rows
	// holds the correct number, while no_rows holds the filtered number of
	// rows.
	//
	int no_columns;

	int __no_rows;

	int no_rows;

	//
	// An array with the widths of the columns. This is unaffected from the
	// filtering.
	//
	int *width;

	//
	// An array with heights of the rows. Again height has the heights of the
	// filtered rows.
	//
	int *__height;

	int *height;

	//
	// A two dimensional array with the fields of the csv file. The parameter
	// __fields has the data, while fields has pointers to the row data.
	//
	wchar_t ***__fields;

	wchar_t ***fields;

	//
	// A flag that tells whether the table has a header row. A header row is
	// always part of a filtered header.
	//
	bool show_header;

	//
	// If a filter is applied to the table, the filter string is stored here.
	//
	s_filter filter;

	//
	// If sorting is applied to the table, the sorting column and direction is
	// stored here.
	//
	s_sort sort;

} s_table;

/******************************************************************************
 * The macro is called with a s_table and the row index. If checks whether the
 * field is a header field.
 *****************************************************************************/

#define s_table_is_field_header(t, i) ((i)->row == 0 && (t)->show_header)

/******************************************************************************
 * The macro checks if the table has all rows. This is true, if the table is
 * not filtered or if it is filtered, but every row contains a field, that
 * matches the filter string. In this case the filtered table is the same as
 * the unfiltered table.
 *****************************************************************************/

#define s_table_has_all_rows(t) ((t)->no_rows == (t)->__no_rows)

void s_table_init(s_table *table, const int no_columns, const int no_rows);

void s_table_free(s_table *table);

void s_table_reset_rows(s_table *table);

void s_table_copy(s_table *table, const int rows, const int columns, wchar_t *str);

void s_table_field_dimension(wchar_t *str, int *width, int *height);

void s_table_reset_filter(s_table *table, s_cursor *cursor);

void s_table_cursor_on_table(const s_table *table, const s_cursor *cursor);

wchar_t* s_table_update_filter_sort(s_table *table, s_cursor *cursor, const bool filter_changed, const bool sort_changed);

bool s_table_prev_next(const s_table *table, s_cursor *cursor, const enum e_direction direction);

void s_table_dump(const s_table *table);

#define s_table_set_defaults(t) (t).show_header = true

#endif
