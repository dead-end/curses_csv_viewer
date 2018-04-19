/*
 * file: ncv_curses.c
 */

#include <ncursesw/ncurses.h>

#include "ncv_common.h"
#include "ncv_table.h"
#include "ncv_curses.h"

/***************************************************************************
 * The function initializes the ncurses.
 **************************************************************************/

void curses_init() {

	if (initscr() == NULL) {
		print_exit_str("curses_init() Unable to init screen!\n");
	}

	//
	// allow KEY_RESIZE to be read on SIGWINCH
	//
	keypad(stdscr, TRUE);

	if (has_colors()) {

		if (start_color() != OK) {
			print_exit_str("curses_init() Unable to init colors!\n");
		}

		if (init_pair(1, COLOR_WHITE, COLOR_BLUE) != OK) {
			print_exit_str("curses_init() Unable to init color pair!\n");
		}

		if (bkgd(COLOR_PAIR(1)) != OK) {
			print_exit_str("curses_init() Unable to set background color pair!\n");
		}
	}

	// scrollok(stdscr, FALSE);

	//
	// switch off cursor by default
	//
	curs_set(0);
}

/***************************************************************************
 * A cleanup function for the ncurses stuff.
 **************************************************************************/

void curses_finish() {
	endwin();
}

/***************************************************************************
 * The method updates one of the two field part structures for a given
 * field. The index of the field can be a row or a column index. The given
 * size can be a row height or a column width.
 **************************************************************************/

void s_field_part_update(const s_table_part *table_part, const int index, const int size, s_field_part *field_part) {

	if (index == table_part->truncated) {

		//
		// we have to distinguish whether the first or the last row / column
		// is truncated.
		//
		if (table_part->truncated == table_part->start) {
			field_part->start = size - table_part->size;
		} else {
			field_part->start = 0;
		}

		//
		// the field size is the truncated size
		//
		field_part->size = table_part->size;

	} else {

		//
		// if the field is not truncated, the size is row height or the
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
// TODO: index_start by ref_index
void s_table_part_update(s_table_part *table_part, const int *sizes, const int index_start, const int num, const int direction, const int win_size) {

	//
	// start with 1 due to the missing border
	//
	int precursor = 1;
	int sum;

	table_part->start = index_start;
	table_part->truncated = -1;
	table_part->size = 0;

	for (table_part->end = index_start; 0 <= table_part->end && table_part->end < num; table_part->end += direction) {

		//
		// sum up widths / heights with their borders
		//
		sum = precursor + sizes[table_part->end] + 1;

		//
		// if sum of width / heights is the window size we are finished
		//
		if (sum == win_size) {
			break;

			//
			// if the sum of width / heights is larger than the window size, the end has to be truncated
			//
		} else if (sum > win_size) {
			table_part->truncated = table_part->end;
			table_part->size = win_size - precursor - 1;
			break;

		}

		precursor = sum;
	}

	//
	// if the end member is not inside the boundaries, the window is larger than necessary
	//
	if (table_part->end < 0 || table_part->end >= num) {
		table_part->end -= direction;
	}

	//
	// if the direction is backwards swap the start and end
	//
	if (direction == DIR_BACKWARD) {
		const int tmp = table_part->start;
		table_part->start = table_part->end;
		table_part->end = tmp;
	}

	print_debug("s_table_part_update() start: %d end: %d truncated: %d size: %d\n", table_part->start, table_part->end, table_part->truncated, table_part->size);
}

/***************************************************************************
 *
 **************************************************************************/

wchar_t *field_truncated_line(wchar_t *str_ptr, wchar_t *buffer, s_field_part *col_field_part) {

	//
	// ensure that the string is not null
	//
	if (str_ptr == NULL) {
		print_exit("field_truncated_line() function called with null string\n");
	}

	const int size = col_field_part->start + col_field_part->size;
	wchar_t *buf_ptr = buffer;

	for (int i = 0; i < size; i++) {

		//
		// on \0 or \n the line is finished
		//
		if (*str_ptr == W_STR_TERM || *str_ptr == W_NEW_LINE) {
			break;
		}

		//
		// copy chars only if they are in the range
		//
		if (i >= col_field_part->start) {
			*buf_ptr = *str_ptr;
			buf_ptr++;

		}

		str_ptr++;
	}

	//
	// add the \0 terminator to the buffer
	//
	*buf_ptr = W_STR_TERM;

	//
	// skip the remaining chars
	//
	while (*str_ptr != W_STR_TERM && *str_ptr != W_NEW_LINE) {
		str_ptr++;
	}

	//
	// if the str pointer is \0 we return NULL to indicate the end
	//
	if (*str_ptr == W_STR_TERM) {
		return NULL;

		//
		// if the str pointer is \n, more lines will follow and we set the
		// pointer to the next line
		//
	} else {
		return ++str_ptr;
	}
}

/***************************************************************************
 *
 **************************************************************************/

void print_truncated_field(wchar_t *ptr, s_field_part *row_field_part, s_field_part *col_field_part, const int win_row, const int win_col) {

	print_debug("print_truncated_field() win row: %d win col: %d '%ls'\n", win_row, win_col, ptr);

	const int field_height = row_field_part->start + row_field_part->size;

	wchar_t buffer[col_field_part->size + 1];

	int current_win_row = win_row;

	for (int field_line = 0; field_line < field_height; field_line++) {

		ptr = field_truncated_line(ptr, buffer, col_field_part);

		print_debug("print_truncated_field() field line: %d '%ls'\n", field_line, buffer);

		if (field_line >= row_field_part->start) {
			mvaddwstr(current_win_row++, win_col, buffer);
		}

		if (ptr == NULL) {
			print_debug_str("print_truncated_field() no next line\n");
			break;
		}
	}
}

/***************************************************************************
 *
 **************************************************************************/

static void print_horizontal_border(const s_table *table, const s_table_part *col_table_part, const chtype left, const chtype middle, const chtype right) {
	s_field_part col_field_part;

	for (int col = col_table_part->start; col <= col_table_part->end; col++) {

		if (col == col_table_part->start) {
			addch(left);
		} else {
			addch(middle);
		}

		s_field_part_update(col_table_part, col, table->width[col], &col_field_part);

		for (int j = 0; j < col_field_part.size; j++) {
			addch(ACS_HLINE);
		}
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

	for (int table_row = row_table_part->start; table_row <= row_table_part->end; table_row++) {

		win_col = 1;
		s_field_part_update(row_table_part, table_row, table->height[table_row], &row_field_part);

		for (int table_col = col_table_part->start; table_col <= col_table_part->end; table_col++) {

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

				if (table_row == cursor->row && table_col == cursor->col) {
					attron(A_REVERSE);
				}

				print_truncated_field(table->fields[table_row][table_col], &row_field_part, &col_field_part, win_row, win_col);

				if (table_row == cursor->row && table_col == cursor->col) {
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
		if (table_row == row_table_part->start) {
			move(win_row - 1, 0);
			print_horizontal_border(table, col_table_part, ACS_ULCORNER, ACS_TTEE, ACS_URCORNER);
		}

		//
		// for every row a horizontal border
		//
		move(win_row + row_field_part.size, 0);
		if (table_row == row_table_part->end) {
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

#define update(sd, s, d) sd.start = s; sd.dir = d

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
	bool updated = true;

	s_start_dir col_start_dir;
	col_start_dir.start = 0;
	update(col_start_dir, 0, DIR_FORWARD);

	s_start_dir row_start_dir;
	update(row_start_dir, 0, DIR_FORWARD);

	getmaxyx(stdscr, win_y, win_x);

	print_debug_str("curses_loop() start\n");

	while (1) {

		if (updated) {
			updated = false;

			//
			// update the visible part of the table
			//
			s_table_part_update(&row_table_part, table->height, row_start_dir.start, table->no_rows, row_start_dir.dir, win_y);
			s_table_part_update(&col_table_part, table->width, col_start_dir.start, table->no_columns, col_start_dir.dir, win_x);
		}

		erase();
		print_table(table, &row_table_part, &col_table_part, &cursor);

		keyInput = getch();

		if (keyInput == ERR) {
			continue; /* ignore when there was a timeout - no data */

		} else if (keyInput == 'q' || keyInput == 'Q') {
			break;

		} else if (keyInput == KEY_LEFT) {
			if (cursor.col - 1 >= 0) {
				cursor.col--;

				if (cursor.col <= col_table_part.start) {
					update(col_start_dir, cursor.col, DIR_FORWARD);
				}

				updated = true;
			}

		} else if (keyInput == KEY_RIGHT) {
			if (cursor.col + 1 < table->no_columns) {
				cursor.col++;

				if (cursor.col >= col_table_part.end) {
					update(col_start_dir, cursor.col, DIR_BACKWARD);
				}

				updated = true;
			}

		} else if (keyInput == KEY_UP) {
			if (cursor.row - 1 >= 0) {
				cursor.row--;

				if (cursor.row <= row_table_part.start) {
					update(row_start_dir, cursor.row, DIR_FORWARD);
				}

				updated = true;
			}

		} else if (keyInput == KEY_DOWN) {
			if (cursor.row + 1 < table->no_rows) {
				cursor.row++;

				if (cursor.row >= row_table_part.end) {
					update(row_start_dir, cursor.row, DIR_BACKWARD);
				}

				updated = true;
			}

		} else if (keyInput == KEY_RESIZE) {

			//
			// on resize of the window, get the new sizes
			//
			getmaxyx(stdscr, win_y, win_x);
			print_debug("curses_loop() new win size y: %d x: %d\n", win_y, win_x);
			updated = true;
		}
	}

	print_debug_str("curses_loop() end\n");
}
