/*
 * file: ncv_table.c
 */

#include "ncv_common.h"
#include "ncv_table.h"

/***************************************************************************
 * The function initializes the internal structure of the table struct. The
 * main task is to allocate memory for the fields and the arrays for the
 * column widths and the row heights.
 **************************************************************************/

void s_table_init(s_table *table, const int no_rows, const int no_columns) {

	table->no_columns = no_columns;
	table->no_rows = no_rows;

	print_debug("s_table_init() Allocate memory for rows: %d columns: %d\n", no_rows, no_columns);

	//
	// allocate and initialize an array for widths of the columns
	//
	table->width = xmalloc(sizeof(int) * no_columns);

	for (int i = 0; i < no_columns; i++) {
		table->width[i] = 0;
	}

	//
	// allocate and initialize an array for heights of the rows
	//
	table->height = xmalloc(sizeof(int) * no_rows);

	for (int i = 0; i < no_rows; i++) {
		table->height[i] = 0;
	}

	//
	// allocate a two dimensional array for the fields
	// the fields are not initialized
	//
	table->fields = xmalloc(sizeof(wchar_t**) * no_rows);

	for (int i = 0; i < no_rows; i++) {
		table->fields[i] = xmalloc(sizeof(wchar_t*) * no_columns);
	}
}

/***************************************************************************
 * The function frees the allocated memory of the internal structure for the
 * table struct.
 **************************************************************************/

void s_table_free(s_table *table) {

	print_debug_str("s_table_free() Freeing allocated memory for the table.\n");

	//
	// the array with the widths and heights of the columns and rows
	//
	free(table->width);
	free(table->height);

	//
	// the two dimensional array with the fields and the actual field
	// content
	//
	for (int row = 0; row < table->no_rows; row++) {
		for (int column = 0; column < table->no_columns; column++) {

			//
			// the field content
			//
			free(table->fields[row][column]);
		}

		free(table->fields[row]);
	}

	free(table->fields);
}

/***************************************************************************
 * The function computes the width and the height of the field. A field is a
 * multi line string. The height for the field is the number of lines. The
 * width is the maximum length of the lines.
 **************************************************************************/

static void s_table_field_dimension(wchar_t *str, int *width, int *height) {

	wchar_t *ptr_start = str;
	wchar_t *ptr_end = str;

	*width = 0;
	*height = 0;

	int width_current = 0;

	while (true) {

		if (*ptr_end == W_NEW_LINE || *ptr_end == W_STR_TERM) {

			//
			// a \n or \0 mark the end of a line
			//
			(*height)++;

			//
			// compute and update the width of the current line
			//
			width_current = ptr_end - ptr_start;
			if (width_current > *width) {
				*width = width_current;
			}

			//
			// if we found the string terminator we are finished
			//
			if (*ptr_end == W_STR_TERM) {
				break;
			}

			//
			// set the start pointer to the beginning of the next
			// line
			//
			ptr_start = ptr_end + 1;
		}

		ptr_end++;
	}
}

/***************************************************************************
 * The function copies the field content parsed from the csv file to the
 * corresponding field in the table struct.
 *
 * Additionally the width and height of the field is computed and if
 * necessary, the arrays the the widths and heights are updated.
 **************************************************************************/

void s_table_copy(s_table *table, const int row, const int column, wchar_t *str) {

	//
	// copy the field content with allocated memory
	//
	table->fields[row][column] = wcsdup(str);

	if (table->fields[row][column] == NULL) {
		print_exit_str("s_table_copy() Unable to allocate memory!\n");
	}

	//
	// compute the width and the height of the field
	//
	int row_size;
	int col_size;
	s_table_field_dimension(str, &col_size, &row_size);

	print_debug("s_table_copy() row: %d column: %d field: %ls\n", row, column, str);
	print_debug("s_table_copy() height current: %d max: %d\n", row_size, table->height[row]);
	print_debug("s_table_copy() width  current: %d max: %d\n", col_size, table->width[column]);

	//
	// update the column width
	//
	if (col_size > table->width[column]) {
		table->width[column] = col_size;
	}

	//
	// update the row hight
	//
	if (row_size > table->height[row]) {
		table->height[row] = row_size;
	}
}

/***************************************************************************
 * The function dumps the content of the table to stdout, for debug
 * purposes.
 **************************************************************************/

void s_table_dump(s_table *table) {

	//
	// the fields
	//
	for (int row = 0; row < table->no_rows; row++) {
		for (int column = 0; column < table->no_columns; column++) {
			print_debug("s_table_dump() row: %d column: %d '%ls'\n", row, column, table->fields[row][column]);
		}
		print_debug("s_table_dump()\n");
	}

	//
	// the column widths
	//
	for (int column = 0; column < table->no_columns; column++) {
		print_debug("s_table_dump() column: %d width: %d\n", column, table->width[column]);
	}

	//
	// the row heights
	//
	for (int row = 0; row < table->no_rows; row++) {
		print_debug("s_table_dump() row: %d height: %d\n", row, table->height[row]);
	}
}
