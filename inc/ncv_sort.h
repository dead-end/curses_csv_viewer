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

#ifndef INC_NCV_SORT_H_
#define INC_NCV_SORT_H_

#include "ncv_common.h"

#include <stdbool.h>

/******************************************************************************
 * The s_sort struct contains the data necessary for the sorting.
 *****************************************************************************/

typedef struct s_sort {

	//
	// The flag that indicates that the sorting is active.
	//
	bool is_active;

	//
	// The column that should be used for sorting.
	//
	int column;

	//
	// The sort direction (forward / backward)
	//
	enum e_direction direction;

} s_sort;

#define s_sort_is_active(s) ((s)->is_active)

bool s_sort_set_inactive(s_sort *sort, const bool uninit);

bool s_sort_update(s_sort *sort, const int column, const enum e_direction direction);

#endif /* INC_NCV_SORT_H_ */
