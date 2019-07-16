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

#include "ncv_table_sort.h"

/******************************************************************************
 * The widths and the heights have to be at least one. Otherwise the cursor
 * field will not be displayed.
 *****************************************************************************/

#define MIN_WIDTH_HEIGHT 1

/******************************************************************************
 * The function initializes the internal structure of the table struct. The
 * main task is to allocate memory for the fields and the arrays for the column
 * widths and the row heights.
 *****************************************************************************/

void s_table_init(s_table *table, const int no_rows, const int no_columns) {

	table->__no_rows = no_rows;
	table->no_columns = no_columns;

	//
	// The macro s_table_is_filtered checks if the table is filtered with the
	// members: __no_rows and no_rows, so both have to be properly initialized.
	//
	table->no_rows = -1;

	print_debug("s_table_init() Allocate memory for rows: %d columns: %d\n", no_rows, no_columns);

	//
	// Allocate and initialize an array for the widths of the columns.
	//
	table->width = xmalloc(sizeof(int) * no_columns);

	for (int column = 0; column < no_columns; column++) {
		table->width[column] = MIN_WIDTH_HEIGHT;
	}

	//
	// Allocate and initialize an array for the heights of the rows.
	//
	table->__height = xmalloc(sizeof(int) * no_rows);

	for (int row = 0; row < no_rows; row++) {
		table->__height[row] = MIN_WIDTH_HEIGHT;
	}

	table->height = xmalloc(sizeof(int) * no_rows);

	//
	// Allocate a two dimensional array for the fields. The fields are not
	// initialized.
	//
	table->__fields = xmalloc(sizeof(wchar_t**) * no_rows);

	for (int row = 0; row < no_rows; row++) {
		table->__fields[row] = xmalloc(sizeof(wchar_t*) * no_columns);
	}

	//
	// Allocate memory for the row pointers.
	//
	table->fields = xmalloc(sizeof(wchar_t**) * no_rows);

	// TODO: check
	//
	// Initialize filtering and sorting.
	//
	s_filter_init(&table->filter);

	s_sort_set_inactive(&table->sort);
}

/******************************************************************************
 * The function frees the allocated memory of the internal structure for the
 * table struct.
 *****************************************************************************/

void s_table_free(s_table *table) {

	print_debug_str("s_table_free() Freeing allocated memory for the table.\n");

	//
	// Free the arrays with the widths and heights of the columns and rows.
	//
	free(table->width);
	free(table->__height);
	free(table->height);

	//
	// Iterate through the two dimensional array with the fields and the actual
	// field content.
	//
	for (int row = 0; row < table->__no_rows; row++) {
		for (int column = 0; column < table->no_columns; column++) {

			//
			// Free the field content.
			//
			free(table->__fields[row][column]);
		}

		free(table->__fields[row]);
	}

	free(table->__fields);
	free(table->fields);
}

/******************************************************************************
 * The function initializes /resets the rows and the row heights of the table.
 * There are no checks if the action is necessary, which is only the case if
 * the table is filtered or sorted. The check have to be done before calling
 * this function.
 *****************************************************************************/

void s_table_reset_rows(s_table *table) {

	print_debug_str("s_table_reset_rows() Do reset table.\n");

	//
	// Reset the row pointers and the row heights.
	//
	for (int row = 0; row < table->__no_rows; row++) {
		table->fields[row] = table->__fields[row];
		table->height[row] = table->__height[row];
	}

	table->no_rows = table->__no_rows;
}

/******************************************************************************
 * The function initializes /resets the rows and the row heights of the table,
 * if necessary. If the table is not filtered or sorted, then there is nothing
 * to do.
 *****************************************************************************/

static bool s_table_reset_rows_opt(s_table *table) {

	//
	// Ensure that the table is actually filtered or sorted.
	//
	if (!s_table_is_filtered(table) && !s_sort_is_active(&table->sort)) {
		print_debug_str("s_table_reset_rows_opt() Table is not filtered and not sorted!\n");
		return false;
	}

	s_table_reset_rows(table);

	return true;
}

/******************************************************************************
 * The function searches in the table for the search string. The cursor is set
 * to the first match. The count member of the filter is set to the total
 * number of matches.
 * If the string was not found, then the cursor is unchanged and the filter
 * count is 0.
 *****************************************************************************/

static void s_table_do_search(s_table *table, s_cursor *cursor) {

	print_debug("s_table_do_search() Do search the table data with: %ls\n", table->filter.str);

	//
	// Reset the row pointers and the row heights, if the table is filtered.
	//
	s_table_reset_rows_opt(table);

	table->filter.count = 0;

	for (int row = 0; row < table->__no_rows; row++) {
		for (int column = 0; column < table->no_columns; column++) {

			//
			// Check if the field content matches the search string.
			//
			if (s_filter_search_str(&table->filter, table->__fields[row][column]) != NULL) {

				//
				// Set the cursor to the first found field.
				//
				if (table->filter.count == 0) {
					s_cursor_pos(cursor, row, column);
				}

				table->filter.count++;
			}
		}
	}

	print_debug("s_table_do_search() Found total: %d cursor row: %d col: %d\n", table->filter.count, cursor->row, cursor->col);
}

/******************************************************************************
 * The function filters the table with the filtering string. The cursor is set
 * to the first match. The count member of the filter is set to the total
 * number of matches.
 * If the string was not found, then the cursor is unchanged and the filter
 * count is 0.
 *****************************************************************************/

static void s_table_do_filter(s_table *table, s_cursor *cursor) {
	bool found_in_row;

	print_debug("s_table_do_filter() Do filter the table data with: %ls\n", table->filter.str);

	//
	// Init the number of rows of the filtered table and the number of matches.
	//
	table->no_rows = 0;
	table->filter.count = 0;

	for (int row = 0; row < table->__no_rows; row++) {

		found_in_row = false;

		for (int column = 0; column < table->no_columns; column++) {

			//
			// Check if the field content matches the search string.
			//
			if (s_filter_search_str(&table->filter, table->__fields[row][column]) != NULL) {

				//
				// The first match in the row
				//
				if (!found_in_row) {
					found_in_row = true;

					//
					// Set the cursor to the first found field.
					//
					if (table->filter.count == 0) {
						s_cursor_pos(cursor, table->no_rows, column);
					}

					//
					// Add the current row to the result by setting the fields
					// pointer, the height and update the number of rows. This
					// is something, that we want only once for a row.
					//
					//
					table->fields[table->no_rows] = table->__fields[row];
					table->height[table->no_rows] = table->__height[row];

					table->no_rows++;
				}

				table->filter.count++;
			}
		}

		//
		// If show header is configured, then the header line is always part of
		// the filtered table.
		//
		if (table->show_header && row == 0 && table->filter.count == 0) {
			table->fields[0] = table->__fields[0];
			table->height[0] = table->__height[0];
			table->no_rows = 1;
		}
	}

	print_debug("s_table_do_filter() Found total: %d rows: %d cursor row: %d col: %d\n", table->filter.count, table->no_rows, cursor->row, cursor->col);
}

/******************************************************************************
 * The function does the filtering and sorting. It is called with the table
 * struct, which contains the s_filter and the s_sort struct. Both have to be
 * applied to the table. Additionally the function is called with two flags
 * that show if the s_filter and / or s_sort struct have changed.
 *
 * The function can return a string, that is displayed in the status line.
 *****************************************************************************/

wchar_t* s_table_update_filter_sort(s_table *table, s_cursor *cursor, const bool filter_changed, const bool sort_changed) {

	//
	// Ensure that there is something to do.
	//
	if (!filter_changed && !sort_changed) {
		print_exit_str("s_table_update_filter_sort() Nothing changed!\n");
	}

	wchar_t *result = NULL;

	bool did_reset = false;

	if (s_filter_is_active(&table->filter)) {

		//
		// Do the filtering or searching
		//
		if (s_filter_is_filtering(&table->filter)) {

			//
			// Filtering does an implicit reset.
			//
			s_table_do_filter(table, cursor);
			did_reset = true;

		} else {
			s_table_do_search(table, cursor);
		}

		//
		// If no match was found, deactivate the filtering and set an error
		// message.
		//
		if (!s_filter_has_matches(&table->filter)) {
			s_filter_set_inactive(&table->filter);
			result = L"No matches found!";

			//
			// If filtering is active, the table has to be reset. At this
			// point: "did_reset = true;" is already set.
			//
			if (s_filter_is_filtering(&table->filter)) {
				s_table_reset_rows(table);
			}
		}

	} else {

		//
		// At this point we know that filtering is not active. If the table is
		// filtered we need a reset.
		//
		if (filter_changed) {
			s_table_reset_rows(table);
			did_reset = true;
		}
	}

	//
	// Do sorting if configured.
	//
	if (s_sort_is_active(&table->sort)) {
		s_table_do_sort(table);

	} else {

		//
		// If the sorting was switched off we need a reset, if this did not
		// happened before.
		//
		if (sort_changed && !did_reset) {
			s_table_reset_rows(table);
		}
	}

	return result;
}

/******************************************************************************
 * The function is called if the table is filtered and searches for the prev /
 * next field that contains the filter string. The cursor is updated with the
 * new position. If the cursor position changed, the function returns true. The
 * cursor position does not change, if there is only one field in the whole
 * table that contains the filter string.
 *****************************************************************************/

bool s_table_prev_next(const s_table *table, s_cursor *cursor, const enum e_direction direction) {

	//
	// The filter has to be set to find the next matching field.
	//
	if (!s_filter_is_active(&table->filter)) {
		print_debug_str("s_table_prev_next() Table is not filtered!\n");
		return false;
	}

	//
	// There has to be at least one match. This has to be the case with
	// filtering being active.
	//
	if (!s_filter_has_matches(&table->filter)) {
		print_exit_str("s_table_prev_next() Table has no match!\n");
	}

	print_debug("s_table_prev_next() cursor row: %d col: %d\n", cursor->row, cursor->col);

	//
	// The start position is the current cursor position.
	//
	int row_cur = cursor->row;
	int col_cur = cursor->col;

	//
	// The flag shows whether the row position has to be updated.
	//
	bool update_row = false;

	while (true) {

		//
		// First go with the column to the direction.
		//
		col_cur += direction;

		//
		// Ensure that the column is still in its range. If the column position
		// has to be adjusted, we move to the prev / next row.
		//
		if (col_cur < 0) {
			col_cur = table->no_columns - 1;
			update_row = true;

		} else if (col_cur >= table->no_columns) {
			col_cur = 0;
			update_row = true;
		}

		//
		// Check if we have to change the row.
		//
		if (update_row) {
			update_row = false;

			//
			// Move the row in the direction and adjust if necessary.
			//
			row_cur += direction;

			if (row_cur < 0) {
				row_cur = table->no_rows - 1;

			} else if (row_cur >= table->no_rows) {
				row_cur = 0;
			}
		}

		print_debug("s_table_prev_next() New cursor row: %d column: %d\n", row_cur, col_cur);

		//
		// If there is only one filter match, we end up at the initial cursor
		// position and we are finished.
		//
		if (row_cur == cursor->row && col_cur == cursor->col) {
			print_debug_str("s_table_prev_next() Search reached the initial cursor position.\n");

			//
			// Return false to indicate that nothing changed.
			//
			return false;
		}

		//
		// Found prev / next field that contains the filter string.
		//
		if (s_filter_search_str(&table->filter, table->fields[row_cur][col_cur]) != NULL) {

			//
			// Set the cursor to the first found field.
			//
			s_cursor_pos(cursor, row_cur, col_cur);

			print_debug("s_table_prev_next() Found cursor row: %d col: %d\n", cursor->row, cursor->col);

			//
			// Return true to indicate that the cursor position changed.
			//
			return true;
		}
	}
}

/******************************************************************************
 * The function computes the width and the height of the field. A field is a
 * multi line string. The height for the field is the number of lines. The
 * width is the maximum length of the lines.
 *
 * An empty string has width 0 and height 1.
 *****************************************************************************/

void s_table_field_dimension(wchar_t *str, int *width, int *height) {

	//
	// The start point points to the start of the current line. The end pointer
	// searches for the line end.
	//
	wchar_t *ptr_start = str;
	wchar_t *ptr_end = str;

	*width = 0;
	*height = 0;

	int width_current = 0;

	while (true) {

		if (*ptr_end == W_NEW_LINE || *ptr_end == W_STR_TERM) {

			//
			// A \n or \0 mark the end of a line.
			//
			(*height)++;

			//
			// Compute and update the width of the current line.
			//
			width_current = ptr_end - ptr_start;
			if (width_current > *width) {
				*width = width_current;
			}

			//
			// If we found the string terminator we are finished.
			//
			if (*ptr_end == W_STR_TERM) {
				break;
			}

			//
			// Set the start pointer to the beginning of the next line.
			//
			ptr_start = ptr_end + 1;
		}

		//
		// If we have not found the line end, we look at the next char.
		//
		ptr_end++;
	}
}

/******************************************************************************
 * The function copies the field content parsed from the csv file to the
 * corresponding field in the table struct.
 *
 * Additionally the width and height of the field is computed and if necessary,
 * the arrays with the widths and heights are updated.
 *****************************************************************************/

void s_table_copy(s_table *table, const int row, const int column, wchar_t *str) {

	//
	// Copy the field content with allocated memory.
	//
	table->__fields[row][column] = wcsdup(str);

	if (table->__fields[row][column] == NULL) {
		print_exit_str("s_table_copy() Unable to allocate memory!\n");
	}

	//
	// Compute the width and the height of the field.
	//
	int row_size;
	int col_size;
	s_table_field_dimension(str, &col_size, &row_size);

	print_debug("s_table_copy() row: %d column: %d field: '%ls'\n", row, column, str);
	print_debug("s_table_copy() height current: %d max: %d\n", row_size, table->__height[row]);
	print_debug("s_table_copy() width  current: %d max: %d\n", col_size, table->width[column]);

	//
	// Update the row height if necessary.
	//
	if (row_size > table->__height[row]) {
		table->__height[row] = row_size;
	}

	//
	// Update the column width if necessary.
	//
	if (col_size > table->width[column]) {
		table->width[column] = col_size;
	}
}

/******************************************************************************
 * The function ensures that the cursor is on the table. If this program is
 * properly implemented, this should not happen.
 *****************************************************************************/

void s_table_cursor_on_table(const s_table *table, const s_cursor *cursor) {

	if (cursor->row < 0 || cursor->row >= table->no_rows) {
		print_exit("s_table_cursor_on_table() Cursor col: %d row: %d table no rows: %d!", cursor->col, cursor->row, table->no_rows);
	}

	if (cursor->col < 0 || cursor->col >= table->no_columns) {
		print_exit("s_table_cursor_on_table() Cursor col: %d row: %d table no cols: %d!", cursor->col, cursor->row, table->no_columns);
	}
}

/******************************************************************************
 * The function dumps the content of the table to stdout, for debug purposes.
 *****************************************************************************/

void s_table_dump(const s_table *table) {

	//
	// Print the fields.
	//
	for (int row = 0; row < table->__no_rows; row++) {
		for (int column = 0; column < table->no_columns; column++) {
			print_debug("s_table_dump() row: %d column: %d '%ls'\n", row, column, table->__fields[row][column]);
		}print_debug_str("s_table_dump()\n");
	}

	//
	// Print the column widths.
	//
	for (int column = 0; column < table->no_columns; column++) {
		print_debug("s_table_dump() column: %d width: %d\n", column, table->width[column]);
	}

	//
	// Print the row heights.
	//
	for (int row = 0; row < table->__no_rows; row++) {
		print_debug("s_table_dump() row: %d height: %d\n", row, table->__height[row]);
	}
}
