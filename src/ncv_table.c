/*
 * file: ncv_table.c
 */

#include "ncv_common.h"
#include "ncv_table.h"

/***************************************************************************
 * If the number of rows and columns are counted, the structure for the
 * fields can be allocated.
 **************************************************************************/

void s_table_create(s_table *table, const int no_rows, const int no_columns) {

	table->no_columns = no_columns;
	table->no_rows = no_rows;

	print_debug("s_table_create() Allocate memory for rows: %d columns: %d\n", no_rows, no_columns);

	//
	// an array for the (max) column sizes
	//
	table->sizes = xmalloc(sizeof(int) * no_columns);

	//
	// a two dimensional array for the fields
	//
	table->fields = xmalloc(sizeof(wchar_t**) * no_rows);

	for (int i = 0; i < no_rows; i++) {
		table->fields[i] = xmalloc(sizeof(wchar_t*) * no_columns);
	}
}

/***************************************************************************
 * The function frees the allocated memory for the table.
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
 * The function copies the field content to the corresponding field.
 **************************************************************************/

void s_table_copy(s_table *table, const int row, const int column, const wchar_t *str) {

	table->fields[row][column] = wcsdup(str);

	if (table->fields[row][column] == NULL) {
		print_exit_str("Unable to allocate memory!\n");
	}
}

/***************************************************************************
 * The function dumps the content of the table to stdout, for debug
 * purposes.
 **************************************************************************/

void s_table_dump(s_table *table) {

	for (int row = 0; row < table->no_rows; row++) {
		for (int column = 0; column < table->no_columns; column++) {
			print_debug("s_table_dump() row: %d column: %d '%ls'\n", row, column, table->fields[row][column]);
		}
		print_debug("s_table_dump()\n");
	}
}
