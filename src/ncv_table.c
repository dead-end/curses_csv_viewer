/*
 * file: ncv_table.c
 */

#include "ncv_common.h"
#include "ncv_table.h"

/***************************************************************************
 * The function initializes the internal structure of the table struct. The
 * main task is to allocate memory for the fields and an array for the
 * column sizes.
 **************************************************************************/

void s_table_init(s_table *table, const int no_rows, const int no_columns) {

	table->no_columns = no_columns;
	table->no_rows = no_rows;

	print_debug("s_table_init() Allocate memory for rows: %d columns: %d\n", no_rows, no_columns);

	//
	// allocate and initialize an array for the (max) column sizes
	//
	table->sizes = xmalloc(sizeof(int) * no_columns);

	for (int i = 0; i < no_columns; i++) {
		table->sizes[i] = 0;
	}

	//
	// allocate a two dimensional array for the fields
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
	// the array with the column sizes
	//
	free(table->sizes);

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
 * The function copies the field content parsed from the csv file to the
 * corresponding field in the table struct.
 *
 * Additionally the max column size is updated if necessary.
 **************************************************************************/

void s_table_copy(s_table *table, const int row, const int column, const wchar_t *str) {

	//
	// copy the field content with allocated memory
	//
	table->fields[row][column] = wcsdup(str);

	if (table->fields[row][column] == NULL) {
		print_exit_str("s_table_copy() Unable to allocate memory!\n");
	}

	//
	// if the field contains a newline, the length it the length of the first line
	//
	int len;
	wchar_t *ptr = wcschr(str, W_NEW_LINE);

	if (ptr == NULL) {
		len = (int) wcslen(str);

	} else {
		len = ptr - str;
	}

	//
	// update the column size
	//
	if (len > table->sizes[column]) {
		table->sizes[column] = len;
	}

	print_debug("s_table_copy() row: %d column: %d field: %ls current-len: %d \n", row, column, str, len);
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
	// the max column sizes
	//
	for (int column = 0; column < table->no_columns; column++) {
		print_debug("s_table_dump()column: %d size: %d\n", column, table->sizes[column]);
	}
}
