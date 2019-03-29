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
#include "ncv_common.h"

#include <math.h>
#include <wctype.h>

/***************************************************************************
 * The widths and the heights have to be at least one. Otherwise the cursor
 * field will not be displayed.
 **************************************************************************/

#define MIN_WIDTH_HEIGHT 1

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

	s_filter_init(&table->filter);
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
}

/***************************************************************************
 * The function initializes the rows and the row heights.
 **************************************************************************/

void s_table_init_rows(s_table *table) {

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

static void s_table_reset_filter(s_table *table, s_cursor *cursor) {

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
	s_table_init_rows(table);

	//
	// Init the cursor to the start position.
	//
	cursor->row = 0;
	cursor->col = 0;
}

/***************************************************************************
 * The function filters the table data.
 **************************************************************************/

void s_table_update_filter(s_table *table, s_cursor *cursor) {
	bool found = false;

	//
	// If the new filter string is empty, do a reset.
	//
	if (!s_filter_is_active(&table->filter)) {
		print_debug_str("s_table_do_filter() Filter is not active, do a reset.\n");
		s_table_reset_filter(table, cursor);
		return;
	}

	print_debug("s_table_do_filter() Do filter the table data with: %ls\n", table->filter.str);

	//
	// Init the number of rows of the filtered table.
	//
	table->no_rows = 0;

	for (int row = 0; row < table->__no_rows; row++) {

		for (int column = 0; column < table->no_columns; column++) {

			//
			// If the field content matches the filter, set the field pointer,
			// the height and update the number of rows.
			//
			if (s_filter_search_str(&table->filter, table->__fields[row][column]) != NULL) {

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
		// of the filtered table. If nothing was found (table->no_rows == 0),
		// the header line will be removed again as a last step.
		//
		if (table->show_header && row == 0 && table->no_rows == 0) {
			table->fields[0] = table->__fields[0];
			table->height[0] = table->__height[0];
			table->no_rows = 1;
		}
	}

	//
	// If no field matches the filter set the cursor to 0/0. This makes only
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
	print_debug("s_table_do_filter() Found rows: %d\n", table->no_rows);
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
	if (!s_filter_is_active(&table->filter)) {
		print_debug_str("s_table_prev_next() Table is not filtered!\n");
		return false;
	}

	//
	// If the table is empty there can be no match.
	//
	if (s_table_is_empty(table)) {
		print_debug_str("s_table_prev_next() Table is empty!\n");
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
		if (s_filter_search_str(&table->filter, table->fields[row_cur][col_cur]) != NULL) {

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

/***************************************************************************
 * The parameter defines the number of rows that are inspected to determine
 * whether a column has a header row. If the csv file has 30.000 you do not
 * want to look at all of them.
 **************************************************************************/

#define HA_MAX_ROWS 64

/***************************************************************************
 * The parameter is used to decide whether the first row is an header. For
 * the string length, the criteria is, that the difference to the mean is
 * greater then HA_MAX_VAR times the standard deviation.
 **************************************************************************/

#define HA_MAX_STD_DEV 3.0

/***************************************************************************
 * The parameter defines how many times a criteria for a column has to be
 * fulfilled to indicate a header.
 **************************************************************************/

#define HA_MAX_SUCCESSFUL 3

/***************************************************************************
 * The function computes the ratio of digits in a string with the string
 * length. If the function is called with an empty string the result would
 * be a division 0/0. In this case 0 is returned, which can be interpreted
 * as the string contains no digits (which is true :o)
 **************************************************************************/

double get_ratio(const wchar_t *str) {
	const size_t len = wcslen(str);

	if (len == 0) {
		return 0;
	}

	//
	// count has to be double to ensure a proper cast at return.
	//
	double count = 0;

	for (; *str != W_STR_TERM; str++) {
		if (iswdigit(*str)) {
			count += 1;
		}
	}

	return count / len;
}

/***************************************************************************
 * The function is a wrapper around the wcslen function. It allows to be
 * used with the function pointer:
 *
 * double (*fct_ptr)(const wchar_t *str)
 **************************************************************************/

double get_str_len(const wchar_t *str) {
	return wcslen(str);
}

/***************************************************************************
 * The function computes the mean value of a characteristic for a column.
 * The function pointer computes a double value that represents the
 * characteristic for a field.
 **************************************************************************/

double get_table_mean(const s_table *table, const int max_rows, const int column, double (*fct_ptr)(const wchar_t *str)) {
	double mean = 0;

	for (int row = 1; row < max_rows; row++) {
		mean += (*fct_ptr)(table->__fields[row][column]);
	}
	mean /= max_rows - 1;

	return mean;
}

/***************************************************************************
 * The function computes the standard deviation of a characteristic for a
 * column. The function pointer computes a double value that represents the
 * characteristic for a field.
 **************************************************************************/

double get_table_std_dev(const s_table *table, const int max_rows, const int column, double (*fct_ptr)(const wchar_t *str), const double mean) {
	double std_dev = 0;
	double tmp;

	for (int row = 1; row < max_rows; row++) {
		tmp = (*fct_ptr)(table->__fields[row][column]);
		std_dev += pow2(mean - tmp);
	}
	std_dev = sqrt(std_dev / (max_rows - 1));

	return std_dev;
}

/***************************************************************************
 * The function compares a characteristic of the first row of a column with
 * the rest of the rows of that column.
 **************************************************************************/

int check_column_characteristic(const s_table *table, const int max_rows, const int column, double (*fct_ptr)(const wchar_t *str)) {

	//
	// compute the mean
	//
	const double mean = get_table_mean(table, max_rows, column, fct_ptr);

	//
	// compute the standard deviation
	//
	const double std_dev = get_table_std_dev(table, max_rows, column, fct_ptr, mean);

	//
	// compute the first row
	//
	const double first = (*fct_ptr)(table->__fields[0][column]);

	print_debug("check_column() col: %d first: '%ls'\n", column, table->__fields[0][column]);
	print_debug("check_column_len() mean: %lf stddev: %lf first: %lf\n", mean, std_dev, first);

	//
	// compare the first row with the mean
	//
	if (fabs(first - mean) > HA_MAX_STD_DEV * std_dev) {
		return 1;
	}

	return 0;
}

/***************************************************************************
 * The function checks the first HA_MAX_ROWS rows to decide whether the
 * table has a header or not.
 **************************************************************************/

bool s_table_has_header(const s_table *table) {

	//
	// Ensure that there are enough rows to analyze
	//
	if (table->__no_rows <= 2) {
		return true;
	}

	//
	// Ensure that the table has enough columns to be successful.
	//
	const int max_successful = table->no_columns > HA_MAX_SUCCESSFUL ? HA_MAX_SUCCESSFUL : table->no_columns;

	//
	// Check at most the first HA_MAX_ROWS rows.
	//
	const int max_rows = table->__no_rows > HA_MAX_ROWS ? HA_MAX_ROWS : table->__no_rows;

	int no_sucessful_checks = 0;

	for (int column = 0; column < table->no_columns; column++) {

		//
		// Check the string lengths
		//
		no_sucessful_checks += check_column_characteristic(table, max_rows, column, get_str_len);
		if (no_sucessful_checks >= max_successful) {
			return true;
		}

		//
		// Check the ratio between the digits and the string lengths.
		//
		no_sucessful_checks += check_column_characteristic(table, max_rows, column, get_ratio);
		if (no_sucessful_checks >= max_successful) {
			return true;
		}
	}

	return false;
}
