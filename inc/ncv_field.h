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

#ifndef INC_NCV_FIELD_H_
#define INC_NCV_FIELD_H_

#include "ncv_table.h"
#include "ncv_table_part.h"

/***************************************************************************
 * A field has two s_field_part structures, one for the row and one for the
 * column. The structure defines which part of the field is visible.
 * It has a start member, which is 0 if the field is not truncated and a
 * size, which is less or equal than the row height or less or equal than the
 * column width.
 ***************************************************************************/

typedef struct s_field_part {

	//
	// row:    the first line that is visible
	// column: the first char that is visible
	//
	int start;

	//
	// row:    the number of lines that are visible
	// column: the number of chars that are visible
	//
	int size;

} s_field_part;


void print_field_content(WINDOW *win, wchar_t *ptr, const s_field_part *row_field_part, const s_field_part *col_field_part, const s_field *win_row_col, const int width, wchar_t *filter, const s_attr *attr_cur);

//
// The functions are only visible for unit tests.
//
void s_field_part_update(s_field_part *field_part, const s_table_part *table_part, const int index, const int size);

wchar_t *get_field_complete_line(wchar_t *str_ptr, wchar_t *buffer, const int width, bool *end);

void intersection(const s_buffer *visible, const s_buffer *print, s_buffer *result);

#endif
