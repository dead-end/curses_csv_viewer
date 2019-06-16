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

#include "ncv_sort.h"

/******************************************************************************
 * The function sets the sort status to inactive. It returns true if the status
 * changed.
 *****************************************************************************/

bool s_sort_set_inactive(s_sort *sort) {

	if (sort->is_active) {
		sort->is_active = false;
		sort->column = -1;
		sort->direction = E_DIR_FORWARD;
		return true;
	}

	return false;
}

/******************************************************************************
 * The function updates the sort struct with new data. It returns true if the
 * sorting is active. Sorting is toggled, which means, if the sorting is active
 * and the function is called with identical data, the sorting will be
 * deactivated.
 *****************************************************************************/

bool s_sort_update(s_sort *sort, const int column, const enum e_direction direction) {

	print_debug("s_sort_update() Before - active: %s column: %d direction: %s\n", bool_str(sort->is_active), sort->column, e_direction_str(sort->direction));

	if (sort->is_active && sort->column == column && sort->direction == direction) {
		sort->is_active = false;
		sort->column = -1;
		sort->direction = E_DIR_FORWARD;

	} else {
		sort->is_active = true;
		sort->column = column;
		sort->direction = direction;
	}

	print_debug("s_sort_update() After - active: %s column: %d direction: %s\n", bool_str(sort->is_active), sort->column, e_direction_str(sort->direction));

	return sort->is_active;
}
