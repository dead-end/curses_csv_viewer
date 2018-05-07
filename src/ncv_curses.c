/*
 * file: ncv_curses.c
 */

#include <ncursesw/ncurses.h>

#include "ncv_common.h"
#include "ncv_table.h"
#include "ncv_curses.h"

/***************************************************************************
 * The method updates one of the two field part structures for a given
 * field. The index of the field can be a row or a column index. The given
 * size can be a row height or a column width.
 **************************************************************************/

void s_field_part_update(const s_table_part *table_part, const int index, const int size, s_field_part *field_part) {

	//
	// If the value of the struct member truncated is -1, the row / column
	// is not truncated and the table fits in the window.
	//
	if (index == table_part->truncated) {

		//
		// We have to distinguish whether the first or the last row / column
		// is truncated.
		//
		if (table_part->truncated == table_part->first) {
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
		// column width
		//
		field_part->start = 0;
		field_part->size = size;
	}

	print_debug("s_field_part_update() index: %d start: %d size: %d\n", index, field_part->start, field_part->size);
}

/***************************************************************************
 * The method updates one of the two table part structure of the table. The
 * sizes parameter is an array with the row heights or the column widths.
 * The index_start can be the first or the last index depending on the
 * direction.
 * The num parameter defines the number of rows or columns.
 **************************************************************************/

void s_table_part_update(s_table_part *table_part, const int *sizes, const int index_start, const int num, const int direction, const int win_size) {

	//
	// Start with 1 due to the missing border.
	//
	int precursor = 1;
	int sum;

	table_part->first = index_start;
	table_part->truncated = -1;
	table_part->size = 0;
	table_part->direction = direction;

	for (table_part->last = index_start; 0 <= table_part->last && table_part->last < num; table_part->last += direction) {

		//
		// Sum up widths / heights with their borders.
		//
		sum = precursor + sizes[table_part->last] + 1;

		//
		// If sum of width / heights is the window size we are finished.
		//
		if (sum == win_size) {
			break;

			//
			// If the sum of width / heights is larger than the window size,
			// the end has to be truncated.
			//
		} else if (sum > win_size) {
			table_part->truncated = table_part->last;
			table_part->size = win_size - precursor - 1;
			break;

		}

		precursor = sum;
	}

	//
	// If the end member is not inside the boundaries, the window is larger
	// than necessary.
	//
	if (table_part->last < 0 || table_part->last >= num) {
		table_part->last -= direction;
	}

	//
	// If the direction is backwards swap the first and the last.
	//
	if (direction == DIR_BACKWARD) {
		const int tmp = table_part->first;
		table_part->first = table_part->last;
		table_part->last = tmp;
	}



	print_debug("s_table_part_update() first: %d last: %d truncated: %d size: %d dir: %d\n", table_part->first, table_part->last, table_part->truncated, table_part->size, table_part->direction);
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

static void print_field(wchar_t *ptr, s_field_part *row_field_part, s_field_part *col_field_part, const int win_row, const int win_col) {

	print_debug("print_field() win row: %d win col: %d field: '%ls'\n", win_row, win_col, ptr);

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
			mvaddwstr(win_row + field_line, win_col, buffer);
		}
	}
}

/***************************************************************************
 * The macro adds 'size' space chars as a padding to the current
 * position.
 **************************************************************************/

#define add_hline(s,c) for (int k = 0; k < (s); k++) addch(c)

/***************************************************************************
 *
 **************************************************************************/

static void print_horizontal_border(const s_table *table, const s_table_part *col_table_part, const chtype left, const chtype middle, const chtype right) {
	s_field_part col_field_part;

	for (int col = col_table_part->first; col <= col_table_part->last; col++) {

		if (col == col_table_part->first) {
			addch(left);
		} else {
			addch(middle);
		}

		s_field_part_update(col_table_part, col, table->width[col], &col_field_part);

		add_hline(col_field_part.size, ACS_HLINE);
	}

	addch(right);
}

/***************************************************************************
 *
 **************************************************************************/

static void print_table(const s_table *table, const s_table_part *row_table_part, const s_table_part *col_table_part, const s_field *cursor) {

	int win_row = 1;
	int win_col;

	s_field_part row_field_part;
	s_field_part col_field_part;

	bool is_cursor;

	for (int table_row = row_table_part->first; table_row <= row_table_part->last; table_row++) {

		win_col = 1;
		s_field_part_update(row_table_part, table_row, table->height[table_row], &row_field_part);

		for (int table_col = col_table_part->first; table_col <= col_table_part->last; table_col++) {

			s_field_part_update(col_table_part, table_col, table->width[table_col], &col_field_part);

			if (win_col == 1) {
				move(win_row, 0);
				vline(ACS_VLINE, row_field_part.size);
			}

			move(win_row, win_col + col_field_part.size);
			vline(ACS_VLINE, row_field_part.size);

			print_debug("row index: %d start: %d size: %d\n", table_row, row_field_part.start, row_field_part.size);
			print_debug("col index: %d start: %d size: %d\n", table_col, col_field_part.start, col_field_part.size);

			if (row_field_part.size > 0 && col_field_part.size > 0) {

				if (table_row == 0) {
					attron(A_BOLD);
				}

				is_cursor = table_row == cursor->row && table_col == cursor->col;
				if (is_cursor) {
					attron(A_REVERSE);
				}

				print_field(table->fields[table_row][table_col], &row_field_part, &col_field_part, win_row, win_col);

				if (is_cursor) {
					attroff(A_REVERSE);
				}

				if (table_row == 0) {
					attroff(A_BOLD);
				}
			}

			win_col += col_field_part.size + 1;
		}

		//
		// horizontal border for the first row
		//
		if (table_row == row_table_part->first) {
			move(win_row - 1, 0);
			print_horizontal_border(table, col_table_part, ACS_ULCORNER, ACS_TTEE, ACS_URCORNER);
		}

		//
		// for every row a horizontal border
		//
		move(win_row + row_field_part.size, 0);
		if (table_row == row_table_part->last) {
			print_horizontal_border(table, col_table_part, ACS_LLCORNER, ACS_BTEE, ACS_LRCORNER);
		} else {
			print_horizontal_border(table, col_table_part, ACS_LTEE, ACS_PLUS, ACS_RTEE);
		}

		win_row += row_field_part.size + 1;
	}

	//
	// without moving the cursor at the end a flickering occurs, when the
	// window is resized
	//
	move(0, 0);
	refresh();
}

/***************************************************************************
 * The function reverses the direction of a table part in certain
 * situations.
 **************************************************************************/

static bool adjust_dir_on_resize(s_table_part *table_part, const int last) {

	//
	// If a row / column is truncated everything is ok
	//
	if (table_part->truncated != -1) {
		return false;
	}

	//
	// On enlarging of the window the fist table row / column has completely appeared.
	//
	if (table_part->first == 0 && table_part->last != last && table_part->direction == DIR_BACKWARD) {
		table_part->direction = DIR_FORWARD;
		return true;
	}

	//
	// On enlarging of the window the last table row / column has completely appeared.
	//
	if (table_part->first != 0 && table_part->last == last && table_part->direction == DIR_FORWARD) {
		table_part->direction = DIR_BACKWARD;
		return true;
	}

	return false;
}

#define is_index_before_first(i,p) (i < (p)->first || (i == (p)->first && (p)->first == (p)->truncated))

#define is_index_after_last(i,p) (i > (p)->last || (i == (p)->last && (p)->last == (p)->truncated))

#define s_table_part_start(p) (p->direction == DIR_FORWARD ? p->first : p->last)

/***************************************************************************
 *
 **************************************************************************/

void do_resize(const s_table *table, s_table_part *row_table_part, s_table_part *col_table_part, int win_y, int win_x, s_field *cursor) {

	//
	// update the visible part of the table
	//

	s_table_part_update(row_table_part, table->height, s_table_part_start(row_table_part), table->no_rows, row_table_part->direction, win_y);
	s_table_part_update(col_table_part, table->width, s_table_part_start(col_table_part), table->no_columns, col_table_part->direction, win_x);

	//
	// Adjust rows
	//
	if (adjust_dir_on_resize(row_table_part, table->no_rows - 1)) {
		s_table_part_update(row_table_part, table->height, s_table_part_start(row_table_part), table->no_rows, row_table_part->direction, win_y);

	} else if (is_index_before_first(cursor->row, row_table_part)) {
		s_table_part_update(row_table_part, table->height, cursor->row, table->no_rows, DIR_FORWARD, win_y);

	} else if (is_index_after_last(cursor->row, row_table_part)) {
		s_table_part_update(row_table_part, table->height, cursor->row, table->no_rows, DIR_BACKWARD, win_y);
	}

	//
	// Adjust columns
	//
	if (adjust_dir_on_resize(col_table_part, table->no_columns - 1)) {
		s_table_part_update(col_table_part, table->width, s_table_part_start(col_table_part), table->no_columns, col_table_part->direction, win_x);

	} else if (is_index_before_first(cursor->col, col_table_part)) {
		s_table_part_update(col_table_part, table->width, cursor->col, table->no_columns, DIR_FORWARD, win_x);

	} else if (is_index_after_last(cursor->col, col_table_part)) {
		s_table_part_update(col_table_part, table->width, cursor->col, table->no_columns, DIR_BACKWARD, win_x);
	}
}

/***************************************************************************
 *
 **************************************************************************/

void curses_loop(const s_table *table) {

	int keyInput;
	int win_x, win_y;

	s_table_part row_table_part;
	s_table_part col_table_part;

	s_field cursor;
	cursor.row = 0;
	cursor.col = 0;

	getmaxyx(stdscr, win_y, win_x);

	print_debug_str("curses_loop() start\n");

	s_table_part_update(&row_table_part, table->height, cursor.row, table->no_rows, DIR_FORWARD, win_y);
	s_table_part_update(&col_table_part, table->width, cursor.col, table->no_columns, DIR_FORWARD, win_x);

	while (true) {

		//
		//
		// TODO: print only if necessary (example: input sdfsd)
		erase();
		print_table(table, &row_table_part, &col_table_part, &cursor);

		keyInput = getch();

		if (keyInput == ERR) {
			continue; /* ignore when there was a timeout - no data */

		} else if (keyInput == 'q' || keyInput == 'Q') {
			break;

		} else if (keyInput == KEY_UP) {
			if (cursor.row - 1 >= 0) {
				cursor.row--;

				if (is_index_before_first(cursor.row, &row_table_part)) {
					s_table_part_update(&row_table_part, table->height, cursor.row, table->no_rows, DIR_FORWARD, win_y);
				}
			}

		} else if (keyInput == KEY_DOWN) {
			if (cursor.row + 1 < table->no_rows) {
				cursor.row++;

				if (is_index_after_last(cursor.row, &row_table_part)) {
					s_table_part_update(&row_table_part, table->height, cursor.row, table->no_rows, DIR_BACKWARD, win_y);
				}
			}

		} else if (keyInput == KEY_LEFT) {
			if (cursor.col - 1 >= 0) {
				cursor.col--;

				if (is_index_before_first(cursor.col, &col_table_part)) {
					s_table_part_update(&col_table_part, table->width, cursor.col, table->no_columns, DIR_FORWARD, win_x);
				}
			}

		} else if (keyInput == KEY_RIGHT) {
			if (cursor.col + 1 < table->no_columns) {
				cursor.col++;

				if (is_index_after_last(cursor.col, &col_table_part)) {
					s_table_part_update(&col_table_part, table->width, cursor.col, table->no_columns, DIR_BACKWARD, win_x);
				}
			}

		} else if (keyInput == KEY_RESIZE) {

			//
			// on resize of the window, get the new sizes
			//
			getmaxyx(stdscr, win_y, win_x);
			print_debug("curses_loop() new win size y: %d x: %d\n", win_y, win_x);

			do_resize(table, &row_table_part, &col_table_part, win_y, win_x, &cursor);
		}
	}

	print_debug_str("curses_loop() end\n");
}
