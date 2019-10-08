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

#include "ncv_table_part.h"
#include "ncv_common.h"

/******************************************************************************
 * The method updates the row /column table part for the table. It is called
 * with a start_index and a direction. It computes the end_index and checks
 * whether the end field is truncated. In this case the truncated size is
 * computed.
 *
 * The sizes parameter is an array with the row heights or the column widths.
 *
 * The index_start can be the first or the last index depending on the
 * direction.
 *
 * The num parameter defines the number of rows or columns.
 *
 * The direction can be E_DIR_FORWARD or E_DIR_BACKWARD.
 *
 * The win_size parameter is the size of the window.
 *****************************************************************************/

void s_table_part_update(s_table_part *table_part, const int *sizes, const int index_start, const int num, const enum e_direction direction, const int win_size) {

	int precursor = 0;
	int sum;

	table_part->first = index_start;
	table_part->truncated = -1;
	table_part->size = 0;
	table_part->direction = direction;

	for (table_part->last = index_start; 0 <= table_part->last && table_part->last < num; table_part->last += direction) {

		//
		// Sum up widths / heights with their borders.
		//
		sum = precursor + sizes[table_part->last] + 1;

		//
		// If sum of width / heights with a border is the win_size - 1 it
		// fits in the window with an additional border.
		//
		// Example:
		// (border)(field)(border)(field)(additional-border)
		//
		// In this case: table_part->truncated = -1
		//
		if (sum == win_size - 1) {
			break;

			//
			// If the sum of width / heights is larger than or equal to the
			// window size, the part has to be truncated.
			//
		} else if (sum >= win_size) {
			table_part->truncated = table_part->last;
			table_part->size = win_size - precursor - 1;
			break;
		}

		precursor = sum;
	}

	//
	// If the end member is not inside the boundaries, the window is larger
	// than necessary.
	//
	if (table_part->last < 0) {
		table_part->last = 0;

	} else if (table_part->last >= num) {
		table_part->last = num - 1;
	}

	//
	// If the direction is backwards swap the first and the last. This
	// ensures that first <= last
	//
	if (direction == E_DIR_BACKWARD) {
		const int tmp = table_part->first;
		table_part->first = table_part->last;
		table_part->last = tmp;
	}

	log_debug("First: %d last: %d truncated: %d size: %d dir: %d", table_part->first, table_part->last, table_part->truncated, table_part->size, table_part->direction);
}

/******************************************************************************
 * The function reverses the direction of a table part in certain situations.
 *
 * Example:
 *
 * You have a table with 10 fields. Visible are fields 5-8. If you enlarge the
 * window you can make visible fields 5-9 and then 5-10.
 *
 *        |------>|
 * 1 2 3 4 5 6 7 8 9 0
 *
 *        |---------->|
 * 1 2 3 4 5 6 7 8 9 0
 *
 * If you keep on enlarging the window you have to check whether there are
 * fields that are not visible at the other end. In this example you can make
 * visible fields 4-10 and then 3-10 and so on until all fields are visible.
 *
 *        |<----------|
 * 1 2 3 4 5 6 7 8 9 0
 *
 *    |<--------------|
 * 1 2 3 4 5 6 7 8 9 0
 *
 * The parameter table_end is 'table->no_rows - 1' or table->no_column - 1'
 * depending on the table part.
 *****************************************************************************/

bool adjust_dir_on_resize(s_table_part *table_part, const int table_end) {

	//
	// If a row / column is truncated everything is ok
	//
	if (table_part->truncated != -1) {
		return false;
	}

	//
	// On enlarging of the window the fist table row / column has completely
	// appeared. Check if there are fields at the end that are not visible.
	//
	if (table_part->first == 0 && table_part->last != table_end && table_part->direction == E_DIR_BACKWARD) {
		table_part->direction = E_DIR_FORWARD;
		return true;
	}

	//
	// On enlarging of the window the last table row / column has completely
	// appeared. Check if there are fields at the beginning that are not
	// visible.
	//
	if (table_part->first != 0 && table_part->last == table_end && table_part->direction == E_DIR_FORWARD) {
		table_part->direction = E_DIR_BACKWARD;
		return true;
	}

	return false;
}
