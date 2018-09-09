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

/***************************************************************************
 * The width and the heights have to be at least one. Otherwise the cursor
 * field will not be displayed.
 **************************************************************************/

#define MIN_WIDTH_HEIGHT 1

/***************************************************************************
 * The function sets the filter string of the table. The function returns
 * true if the filter string changed (set, updated or deleted). It accepts
 * EMPTY_FILTER_STRING as a filter (which is NULL).
 **************************************************************************/

bool s_table_set_filter_string(s_table *table, const wchar_t *filter) {

	if (table->filter != EMPTY_FILTER_STRING) {
		print_debug("s_table_set_filter_string() Current filter string: '%ls'\n", table->filter);

		//
		// If the new filter string is empty delete the current.
		//
		if (filter == EMPTY_FILTER_STRING || wcslen(filter) == 0) {
			print_debug_str("s_table_set_filter_string() Setting filter string to NULL\n");

			free(table->filter);
			table->filter = EMPTY_FILTER_STRING;

			return true;
		}

		//
		// Check if the filter does not changed.
		//
		if (wcscmp(table->filter, filter) == 0) {
			print_debug("s_table_set_filter_string() Filter string did not change: %ls\n", filter);
			return false;
		}

		//
		// At this point, the current filter is set and the new filter is not
		// empty.
		//
		free(table->filter);

	} else {

		//
		// Check if the filter was not set and the new filter is empty.
		//
		if (filter == EMPTY_FILTER_STRING || wcslen(filter) == 0) {
			print_debug_str("s_table_set_filter_string() Filter string was already NOT set\n");
			return false;
		}
	}

	//
	// Duplicate the filter
	//
	table->filter = wcsdup(filter);

	if (table->filter == NULL) {
		print_exit_str("s_table_set_filter_string() Unable to allocate memory!\n");
	}

	print_debug("s_table_set_filter_string() New filter string: '%ls'\n", table->filter);

	return true;
}

/***************************************************************************
 * The function initializes the internal structure of the table struct. The
 * main task is to allocate memory for the fields and the arrays for the
 * column widths and the row heights.
 **************************************************************************/

void s_table_init(s_table *table, const int no_rows, const int no_columns) {

	table->__no_rows = no_rows;
	table->no_columns = no_columns;

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

	//
	// Set filter string to NULL to indicate that the table is not filtered.
	//
	table->filter = EMPTY_FILTER_STRING;
}

/***************************************************************************
 * The function frees the allocated memory of the internal structure for the
 * table struct.
 **************************************************************************/

void s_table_free(s_table *table) {

	print_debug_str("s_table_free() Freeing allocated memory for the table.\n");

	//
	// Free the arrays with the widths and heights of the columns and rows.
	//
	free(table->width);
	free(table->__height);
	free(table->height);

	//
	// Iterate through the two dimensional array with the fields and the
	// actual field content.
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

	//
	// Free filter if set.
	//
	if (table->filter != NULL) {
		free(table->filter);
	}
}

/***************************************************************************
 * The function initializes the rows and the row heights.
 **************************************************************************/

void s_table_int_rows(s_table *table) {

	//
	// Reset the row pointers and the row heights.
	//
	for (int row = 0; row < table->__no_rows; row++) {
		table->fields[row] = table->__fields[row];
		table->height[row] = table->__height[row];
	}

	table->no_rows = table->__no_rows;
}

/***************************************************************************
 * If the table is filtered, the function resets the number of rows, the
 * heights and the field pointers.
 **************************************************************************/

void s_table_reset_filter(s_table *table, s_cursor *cursor) {

	print_debug_str("s_table_reset_filter() Reset the row pointers, the height values and the number of rows.\n");

	//
	// There is nothing to do if the number of rows of the filtered table is
	// the number of the whole table. From comparing the number of rows you
	// cannot tell whether the table is filtered or not.
	//
	if (table->no_rows == table->__no_rows) {
		print_debug_str("s_table_reset_filter() Table is already reset.\n");
		return;
	}

	//
	// Reset the row pointers and the row heights.
	//
	s_table_int_rows(table);

	//
	// Init the cursor to the start position.
	//
	cursor->row = 0;
	cursor->col = 0;
}

/***************************************************************************
 * The function filters the table data.
 **************************************************************************/

void s_table_do_filter(s_table *table, s_cursor *cursor) {
	bool found = false;

	//
	// If the new filter string is empty, do a reset.
	//
	if (!s_table_is_filtered(table)) {
		print_debug_str("s_table_do_filter() Filter is empty, do a reset.\n");
		s_table_reset_filter(table, cursor);
		return;
	}

	print_debug("s_table_do_filter() Do filter the table data with: %ls\n", table->filter);

	table->no_rows = 0;

	for (int row = 0; row < table->__no_rows; row++) {

		for (int column = 0; column < table->no_columns; column++) {

			//
			// If the field content matches the filter, set the field pointer
			// the height and update the number of rows.
			//
			if (wcsstr(table->__fields[row][column], table->filter) != NULL) {

				table->fields[table->no_rows] = table->__fields[row];
				table->height[table->no_rows] = table->__height[row];

				//
				// Set the cursor to the first found field.
				//
				if (!found) {
					cursor->row = table->no_rows;
					cursor->col = column;
					found = true;
				}

				table->no_rows++;

				//
				// Ignore the rest of the columns of the row.
				//
				break;
			}
		}

		//
		// If show header is configured, then the header line is always part
		// of the filtered table. If nothing was found, the header line will
		// be removed again as a last step.
		//
		if (row == 0 && table->show_header && table->no_rows == 0) {
			table->fields[table->no_rows] = table->__fields[row];
			table->height[table->no_rows] = table->__height[row];
			table->no_rows++;
		}
	}

	//
	// If no field matches the filter set the cursor to 0/0. The makes only
	// sense if show_header is true.
	//
	if (!found) {
		cursor->row = 0;
		cursor->col = 0;

		//
		// If the whole table does not contain the filter, set it empty.
		//
		if (table->show_header) {
			table->no_rows = 0;
		}
	}

	print_debug("s_table_do_filter() cursor row: %d col: %d\n", cursor->row, cursor->col);
}

/***************************************************************************
 * The function is called if the table is filtered and searches for the prev
 * / next field that contains the filter string. The cursor is updated with
 * the new position. If the cursor position changed, the function returns
 * true. The cursor position does not change, if there is only one field in
 * the whole table that contains the filter string.
 **************************************************************************/

bool s_table_prev_next(const s_table *table, s_cursor *cursor, const int direction) {

	//
	// The filter has to be set to find the next matching field.
	//
	if (!s_table_is_filtered(table)) {
		print_debug_str("s_table_do_filter() Table is not filtered!\n");
		return false;
	}

	//
	// If the table is empty there can be no match.
	//
	if (s_table_is_empty(table)) {
		print_debug_str("s_table_do_filter() Table is empty!\n");
		return false;
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
		// Ensure that the column is still in its range. If the column
		// position has to be adjusted, we move to the prev / next row.
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
		// If there is only one filter match, we end up at the initial
		// cursor position and we are finished.
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
		if (wcsstr(table->fields[row_cur][col_cur], table->filter) != NULL) {

			//
			// Set the cursor to the first found field.
			//
			cursor->row = row_cur;
			cursor->col = col_cur;

			print_debug("s_table_prev_next() New cursor row: %d col: %d\n", cursor->row, cursor->col);

			//
			// Return true to indicate that the cursor position changed.
			//
			return true;
		}
	}
}

/***************************************************************************
 * The function computes the width and the height of the field. A field is a
 * multi line string. The height for the field is the number of lines. The
 * width is the maximum length of the lines.
 *
 * An empty string has width 0 and height 1.
 **************************************************************************/

void s_table_field_dimension(wchar_t *str, int *width, int *height) {

	//
	// The start point points to the start of the current line. The end
	// pointer searches for the line end.
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
			// Set the start pointer to the beginning of the next
			// line.
			//
			ptr_start = ptr_end + 1;
		}

		//
		// If we have not found the line end, we look at the next char.
		//
		ptr_end++;
	}
}

/***************************************************************************
 * The function copies the field content parsed from the csv file to the
 * corresponding field in the table struct.
 *
 * Additionally the width and height of the field is computed and if
 * necessary, the arrays with the widths and heights are updated.
 **************************************************************************/

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

/***************************************************************************
 * The function dumps the content of the table to stdout, for debug
 * purposes.
 **************************************************************************/

void s_table_dump(const s_table *table) {

	//
	// Print the fields.
	//
	for (int row = 0; row < table->__no_rows; row++) {
		for (int column = 0; column < table->no_columns; column++) {
			print_debug("s_table_dump() row: %d column: %d '%ls'\n", row, column, table->__fields[row][column]);
		}
		print_debug_str("s_table_dump()\n");
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
