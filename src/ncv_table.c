/*
 * file: ncv_table.c
 */

#include "ncv_common.h"
#include "ncv_table.h"

/***************************************************************************
 * The width and the heights have to be at least one. Otherwise the cursor
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
 * If the table is filtered, the function resets the number of rows, the
 * heights and the field pointers. It returns true if the table was reset
 * and false, if the table was already reset.
 **************************************************************************/

bool s_table_reset_filter(s_table *table, s_cursor *cursor) {

	print_debug_str("s_table_reset_filter() Reset the row pointers, the height values and the number of rows.\n");

	//
	// Check if the table is already reset.
	//
	if (table->no_rows == table->__no_rows) {
		print_debug_str("s_table_reset_filter() Table is already reset.\n");
		return false;
	}

	for (int row = 0; row < table->__no_rows; row++) {
		table->fields[row] = table->__fields[row];
		table->height[row] = table->__height[row];
	}

	table->no_rows = table->__no_rows;

	//
	// Init the cursor to the start position.
	//
	cursor->row = 0;
	cursor->col = 0;

	return true;
}

/***************************************************************************
 * The function filters the table data.
 **************************************************************************/

void s_table_do_filter(s_table *table, s_cursor *cursor, const wchar_t *filter) {
	bool found = false;

	print_debug("s_table_do_filter() Do filter the table data with: %ls\n", filter);

	//
	// If the filter is empty, the filtering is a reset, so delegate to the
	// function.
	//
	if (wcslen(filter) == 0) {
		print_debug_str("s_table_do_filter() Filter is empty, do a reset.\n");
		s_table_reset_filter(table, cursor);
		return;
	}

	table->no_rows = 0;

	for (int row = 0; row < table->__no_rows; row++) {

		for (int column = 0; column < table->no_columns; column++) {

			//
			// If the field content matches the filter, set the field pointer
			// the height and update the number of rows.
			//
			if (wcsstr(table->__fields[row][column], filter) != NULL) {

				table->fields[table->no_rows] = table->__fields[row];
				table->height[table->no_rows] = table->__height[row];

				if (!found) {
					cursor->row = table->no_rows;
					cursor->col = column;
					found = true;
				}

				table->no_rows++;

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

	print_debug("s_table_copy() row: %d column: %d field: %ls\n", row, column, str);
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
