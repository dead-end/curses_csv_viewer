/*
 * ncv_field.c
 */

#include "ncv_field.h"

/***************************************************************************
 * The method updates the row /column field part for a given field.
 *
 * The table part of the row / column is used to determine whether the field
 * row / column is truncated and if so the truncated size of the field.
 *
 * The index is the index of the row / column of the field.
 *
 * The size is the not truncated size of the field, which is the height of
 * the row or the width of the column.
 **************************************************************************/

void s_field_part_update(const s_table_part *table_part, const int index, const int size, s_field_part *field_part) {

	//
	// If the table has a truncated row / column, the table_part struc's
	// member "truncated" has its index. If no row / column is truncated
	// the "truncated" member is -1.
	//
	if (index == table_part->truncated) {

		//
		// If the row is truncated the first or the last lines are skipped.
		// if the column is truncated the first or the last chars of each
		// line of the field is skipped.
		//
		// If the table part last is truncated, the last lines / chars are
		// skipped. If the table part first is truncated, the first lines
		// chars are skipped.
		//
		if (table_part->first != table_part->last && table_part->truncated == table_part->first) {
			field_part->start = size - table_part->size;
		} else {
			field_part->start = 0;
		}

		//
		// The field size is the truncated size.
		//
		field_part->size = table_part->size;

	} else {

		//
		// If the field is not truncated, the size is row height or the
		// column width.
		//
		field_part->start = 0;
		field_part->size = size;
	}

	print_debug("s_field_part_update() index: %d truncated: %d first: %d last: %d\n", index, table_part->truncated, table_part->first, table_part->last);
	print_debug("s_field_part_update() start: %d size: %d\n", field_part->start, field_part->size);
}

/***************************************************************************
 * The function is repeatedly called with a pointer to a field string and
 * copies the chars of the current field line to the buffer. The buffer has
 * a fixed size (col_field_part->size +1) and is padded with spaces. If the
 * end of the string is reached, the buffer contains only spaces.
 * The function updates the pointer to the start of the next line of NULL is
 * no more lines are present.
 **************************************************************************/

wchar_t *get_field_line(wchar_t *str_ptr, wchar_t *buffer, const s_field_part *col_field_part) {

	//
	// Initialize the buffer with spaces. This is not necessary if the field
	// ends with more than one empty line. In this case the buffer should be
	// unchanged.
	//
	wmemset(buffer, L' ', col_field_part->size);

	//
	// If the str pointer is null the function returns. In this case the
	// buffer contains only spaces from the initialization.
	//
	if (str_ptr == NULL) {
		print_debug_str("get_field_line() function called with null string\n");
		return NULL;
	}

	//
	// Get the size of the field. If the field is truncated, start is greater
	// than 0 or size is less than the field width.
	//
	const int size = col_field_part->start + col_field_part->size;

	wchar_t *buf_ptr = buffer;

	for (int i = 0; i < size; i++) {

		//
		// On \0 or \n the line is finished.
		//
		if (*str_ptr == W_STR_TERM || *str_ptr == W_NEW_LINE) {
			break;
		}

		//
		// Copy chars only if they are in the range.
		//
		if (i >= col_field_part->start) {
			*buf_ptr = *str_ptr;
			buf_ptr++;

		}

		str_ptr++;
	}

	//
	// Skip the remaining chars after reaching the 'size' of the for loop.
	//
	while (*str_ptr != W_STR_TERM && *str_ptr != W_NEW_LINE) {
		str_ptr++;
	}

	//
	// If the str pointer is \0 we return NULL to indicate the end.
	//
	if (*str_ptr == W_STR_TERM) {
		return NULL;

		//
		// If the str pointer is \n, more lines will follow and we set the
		// pointer to the next line.
		//
	} else {
		return ++str_ptr;
	}
}

/***************************************************************************
 * The function prints the content of a field. The field may be truncated.
 **************************************************************************/

void print_field(WINDOW *win, wchar_t *ptr, s_field_part *row_field_part, s_field_part *col_field_part, const s_field *win_row_col) {

	print_debug("print_field() win row: %d win col: %d field: '%ls'\n", win_row_col->row, win_row_col->col, ptr);

	//
	// Get the height of the field. If the field is truncated, start is
	// greater than 0 or size is less than the row height.
	//
	const int field_height = row_field_part->start + row_field_part->size;

	//
	// Create a buffer and add the str terminator.
	//
	wchar_t buffer[col_field_part->size + 1];
	buffer[col_field_part->size] = W_STR_TERM;

	for (int field_line = 0; field_line < field_height; field_line++) {

		//
		// Get the next field line. The pointer is updated to the next line or
		// NULL if no more lines are present. The buffer contains the line a
		// may contain padding chars.
		//
		ptr = get_field_line(ptr, buffer, col_field_part);
		print_debug("print_field() field line: %d '%ls'\n", field_line, buffer);

		//
		// Skip the first lines if necessary,
		//
		if (field_line >= row_field_part->start) {
			mvwaddwstr(win, win_row_col->row + field_line - row_field_part->start, win_row_col->col, buffer);
		}
	}
}
