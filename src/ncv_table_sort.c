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

#include "ncv_table.h"

#include <errno.h>
#include <string.h>
#include <float.h>

/******************************************************************************
 * The struct is used to sort the table by a column with numerical values. A
 * helper array is created, that contains the numerical values, represented by
 * a double value. This helper array is sorted and the sorting is applied to
 * the table. The helper array consists of s_comp_num elements.
 *****************************************************************************/

typedef struct s_comp_num {

	//
	// The double value of a wchar_t string. Empty strings are represented with
	// DBL_MAX, so that they appear at the end of the sorted column, with a
	// forward direction.
	//
	//
	double value;

	//
	// A pointer to the row that contains the column value.
	//
	wchar_t **row;

} s_comp_num;

/******************************************************************************
 * The function is a callback function for the sorting of numerical values. It
 * is called with two s_comp_num pointers and a pointer to a s_sort struct,
 * which contains the column and the direction. The function compares the
 * column values by their corresponding double value and applies the direction.
 *****************************************************************************/

static int compare_num(const void *ptr_1, const void *ptr_2, void *sort_ptr) {

	//
	// Get the s_sort stuct for the sort direction and column.
	//
	const s_sort *sort = (const s_sort*) sort_ptr;

	const s_comp_num *comp_num_1 = (const s_comp_num*) ptr_1;
	const s_comp_num *comp_num_2 = (const s_comp_num*) ptr_2;

	//
	// Compare the double values. The difference would be ok, but we have to
	// return an int value not a double.
	//
	int result;
	if (comp_num_1->value > comp_num_2->value) {
		result = (sort->direction);

	} else if (comp_num_1->value < comp_num_2->value) {
		result = -1 * (sort->direction);

	} else {
		result = 0;
	}

	log_debug("Direction: %s result: %d '%ls' '%ls'", e_direction_str(sort->direction), result, comp_num_1->row[sort->column], comp_num_2->row[sort->column]);

	return result;
}

/******************************************************************************
 * The function is a callback function for the sorting of wchar_t strings. It
 * is called with two row pointers and a pointer to a s_sort struct, which
 * contains the column and the direction. The function gets the two wchar_t
 * string for the rows and the column and compares them according to the
 * direction.
 *****************************************************************************/

static int compare_wcs(const void *ptr_row_prt_1, const void *ptr_row_ptr_2, void *sort_ptr) {

	//
	// Get the s_sort stuct for the sort direction and column.
	//
	const s_sort *sort = (const s_sort*) sort_ptr;

	const wchar_t **row_ptr_1 = (*(const wchar_t***) ptr_row_prt_1);
	const wchar_t **row_ptr_2 = (*(const wchar_t***) ptr_row_ptr_2);

	//
	// Do the actual comparison.
	//
	const int result = (sort->direction) * wcscmp(row_ptr_1[sort->column], row_ptr_2[sort->column]);

	log_debug("Direction: %s result: %d '%ls' '%ls'", e_direction_str(sort->direction), result, row_ptr_1[sort->column], row_ptr_2[sort->column]);

	return result;
}

/******************************************************************************
 * The function applies the sorting of the helper array of s_comp_num's to the
 * table.
 *****************************************************************************/

static void apply_num_sorting(s_table *table, const s_comp_num *comp_num) {

#ifdef DEBUG
	const int col = table->sort.column;
#endif

	//
	// The s_comp_num array is sorted and contains a pointer to the row of the
	// column value.
	//
	for (int row = 0; row < table->no_rows; row++) {
		table->fields[row] = comp_num[row].row;

#ifdef DEBUG
		log_debug("Sorted value: %ls", table->fields[row][col]);
#endif
	}
}

/******************************************************************************
 * The function tries to convert the column values to double values and stores
 * the result in an array of s_comp_num. If one column value cannot be
 * converted, the function returns immediately with false. If the conversion of
 * the hole column succeeded, the function returns true.
 *
 * Empty strings are converted to DBL_MAX.
 *
 * It is possible that the column values have a non numerical suffix. If so,
 * all suffixes have to be equal. An example is a column with currencies like:
 *
 * "1000,00 Euro"
 *****************************************************************************/

bool try_convert_num(s_table *table, s_comp_num *num_comp) {

	//
	// Two pointer to suffixes. The first suffix found is stored in:
	// init_tailptr. All other are stored in: tailptr and are compared
	// with the init_tailptr.
	//
	wchar_t *init_tailptr = NULL;
	wchar_t *tailptr;

	//
	// The column that should be sorted.
	//
	const int col = table->sort.column;

	//
	// Iterate through the rows to convert the column values.
	//
	for (int row = 0; row < table->no_rows; row++) {

		//
		// Ignore the header if necessary.
		//
		if (row == 0 && table->show_header) {
			num_comp[row].value = 0;
			log_debug_str("String is header, set value to: 0");

		}

		//
		// Check if the column value is empty.
		//
		//
		else if (wcs_is_empty(table->fields[row][col])) {
			num_comp[row].value = DBL_MAX;
			log_debug_str("String is empty, set value to: DBL_MAX");

		} else {

			//
			// Set errno to 0 to be able to detect errors and to the conversion.
			//
			errno = 0;
			num_comp[row].value = wcstod(table->fields[row][col], &tailptr);

			//
			// Check for errors (for example overflows)
			//
			if (errno != 0) {
				log_debug("Unable to convert: '%ls' - %s", table->fields[row][col], strerror(errno));
				return false;
			}

			//
			// If the tail pointer is equal to the column value, no conversion
			// was possible. (the column value is not a number)
			//
			if (table->fields[row][col] == tailptr) {
				log_debug("Unable to convert: %ls", table->fields[row][col]);
				return false;
			}

			log_debug("'%ls' %f '%ls'", table->fields[row][col], num_comp[row].value, tailptr);

			//
			// Save the first suffix.
			//
			if (init_tailptr == NULL) {
				init_tailptr = tailptr;
				log_debug("Save suffix: '%ls'", init_tailptr);

			}

			//
			// If a suffix exists, we have to ensure, that the new is the same.
			//
			else if (wcscmp(init_tailptr, tailptr) != 0) {
				log_debug("String: '%ls' does not end with: '%ls'", table->fields[row][col], init_tailptr);
				return false;
			}
		}

		//
		// Store a pointer to the row. This is used when the sorted array is
		// applied to the table.
		//
		num_comp[row].row = table->fields[row];
	}

	log_debug_str("Succeeded!");

	return true;
}

/******************************************************************************
 * The function does the sorting of the table by a given column and direction.
 * First it is tried to do the sorting by numerical values. For this, the
 * column entries are converted to double values. If the conversion of the
 * column succeeded, the sorting is done numerically. If not the sorting is
 * done by (wchar_t) strings.
 *****************************************************************************/

void s_table_do_sort(s_table *table) {

	//
	// If the table has a header, the first row is excluded from the sorting.
	//
	const int offset = table->show_header ? 1 : 0;

	//
	// Create a helper array for the numerical sorting.
	//
	s_comp_num comp_num_array[table->no_rows];

	//
	// Try a numerical sorting first.
	//
	if (try_convert_num(table, comp_num_array)) {
		log_debug_str("Sort by numerical values.");

		//
		// Do the numerical sorting.
		//
		qsort_r(&comp_num_array[offset], table->no_rows - offset, sizeof(s_comp_num), compare_num, (void*) &table->sort);

		//
		// Apply the sorting to the table.
		//
		apply_num_sorting(table, comp_num_array);
	}

	//
	// The fallback is (wchar_t-) string sorting.
	//
	else {
		log_debug_str("Sort by string values.");

		qsort_r(&table->fields[offset], table->no_rows - offset, sizeof(wchar_t**), compare_wcs, (void*) &table->sort);
	}
}
