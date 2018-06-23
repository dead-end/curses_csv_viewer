/*
 * file: ncv_curses.c
 */

#include <ncursesw/ncurses.h>

#include "ncv_common.h"
#include "ncv_table.h"
#include "ncv_curses.h"
#include "ncv_ncurses.h"

/***************************************************************************
 * Each field can have up to 4 corner chars. Each corner can have 4 shapes.
 * The shapes are:
 *
 * - The field corner can be a corner of the table (example: ┌)
 * - The field corner can be top / bottom tee element (example: ┬)
 * - The field corner can be left / right tee element (example: ├)
 * - The field corner can be plus (example: ┼)
 *
 * Additionally it has a row and a column. This is the row and column if the
 * field corner is a corner of the table.
 **************************************************************************/

typedef struct s_corner {

	//
	// The 4 shapes of the field corner.
	//
	chtype corner;
	chtype tb_tee;
	chtype lr_tee;
	chtype plus;

	// TODO: rename
	//
	// The row and column if the field corner is a table corner.
	//
	int row;
	int col;
} s_corner;

//
// The 4 corners of a field:
//
static s_corner UL_CORNER;
static s_corner UR_CORNER;
static s_corner LR_CORNER;
static s_corner LL_CORNER;

/***************************************************************************
 * The function and the macro are used to setup the corner definitions
 **************************************************************************/

#define s_corner_init(c,CO,TB,LR,PL,ROW,COL) \
		c.corner = CO; c.tb_tee = TB; c.lr_tee = LR; c.plus = PL; \
		c.row = ROW; c.col = COL

static void s_corner_inits(const s_table *table) {

	s_corner_init(UL_CORNER, ACS_ULCORNER, ACS_TTEE, ACS_LTEE, ACS_PLUS, 0, 0);
	s_corner_init(UR_CORNER, ACS_URCORNER, ACS_TTEE, ACS_RTEE, ACS_PLUS, 0, table->no_columns - 1);
	s_corner_init(LL_CORNER, ACS_LLCORNER, ACS_BTEE, ACS_LTEE, ACS_PLUS, table->no_rows - 1, 0);
	s_corner_init(LR_CORNER, ACS_LRCORNER, ACS_BTEE, ACS_RTEE, ACS_PLUS, table->no_rows - 1, table->no_columns - 1);
}

/***************************************************************************
 * The function prints a corner char of a field.
 *
 * The index struct contains the row and column of the field in the table.
 *
 * The win_field struct contains the coordinates of the upper left corner of
 * the field in the window.
 *
 * The win_field_end struct contains the coordinates of the lower right
 * corner of the field in the window.
 *
 * The corner struct contains the shapes of the corner. It determines which
 * one of the 4 corners are printed (e.g. UL_CORNER)
 **************************************************************************/

static void print_corner(const s_field *idx, const s_field *win_field, const s_field *win_field_end, const s_corner *corner) {
	chtype ch;
	int row, col;

	//
	// The row and column of the field determines the shape of the field
	// corner.
	//
	if (idx->row == corner->row) {
		if (idx->col == corner->col) {
			ch = corner->corner;
		} else {
			ch = corner->tb_tee;
		}
	} else {
		if (idx->col == corner->col) {
			ch = corner->lr_tee;
		} else {
			ch = corner->plus;
		}
	}

	//
	// The row and column of the corner also determines if we have to add the
	// field width / height to determine the position of the corner.
	//
	if (0 == corner->row) {
		row = win_field->row;
	} else {
		row = win_field_end->row;
	}

	if (0 == corner->col) {
		col = win_field->col;
	} else {
		col = win_field_end->col;
	}

	//
	// Print the corner char.
	//
	mvaddch(row, col, ch);
}

/***************************************************************************
 * The function prints the corners of the field. This is at least 1 and at
 * most 4.
 **************************************************************************/

void print_corners(const s_field *idx, const s_field *win_field, const s_field *win_field_end, const bool row_cond, const bool col_cond, const s_corner *yy, const s_corner *ny, const s_corner *yn, const s_corner *nn) {

	//
	// This corner is always printed.
	//
	print_corner(idx, win_field, win_field_end, yy);

	//
	// if the row is not truncated an additional border is printed.
	//
	if (row_cond) {
		print_corner(idx, win_field, win_field_end, ny);
	}

	//
	// if the column is not truncated an additional border is printed.
	//
	if (col_cond) {
		print_corner(idx, win_field, win_field_end, yn);
	}

	//
	// if the row and column is not truncated an additional border is printed.
	//
	if (row_cond && col_cond) {
		print_corner(idx, win_field, win_field_end, nn);
	}
}

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
	// If the value of the struct member truncated is -1, the row / column
	// is not truncated and the table fits in the window.
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
		// column width
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
			mvaddwstr(win_row + field_line - row_field_part->start, win_col, buffer);
		}
	}
}

/**
 * top or left starts with border => field has an offset
 */
#define get_row_col_offset(p, i) (((p)->direction == DIR_FORWARD || ((p)->truncated == -1 && i == (p)->first)) ? 1 : 0)

/**
 *
 */
static void print_table(const s_table *table, const s_table_part *row_table_part, const s_table_part *col_table_part, const s_field *cursor) {

	//
	// The absolute coordinates of the field with its borders in the window.
	//
	s_field win_field;
	win_field.row = 0;

	//
	// The absolute coordinates of the fields text. This depend on the left / top
	// border, which can be a normal or an additional border
	//
	s_field win_text;

	//
	// The field end is the absolute window coordinate of the right / bottom border if
	// it exists.
	//
	s_field win_field_end;

	//
	// The variable for the number of borders for a field, which is 1 by default and 2 if the
	// field is first or last and not truncated.
	//
	s_field num_borders;

	bool is_cursor;

	s_field_part row_field_part;
	s_field_part col_field_part;

	//
	// The variable contains the row and column of the current field to be printed.
	//
	s_field idx;

	//
	//
	//
	bool row_cond;
	bool col_cond;

	for (idx.row = row_table_part->first; idx.row <= row_table_part->last; idx.row++) {

		win_field.col = 0;

		print_debug("calling: s_field_part_update row: %d\n", idx.row);
		s_field_part_update(row_table_part, idx.row, table->height[idx.row], &row_field_part);

		for (idx.col = col_table_part->first; idx.col <= col_table_part->last; idx.col++) {

			print_debug("calling: s_field_part_update col: %d\n", idx.col);
			s_field_part_update(col_table_part, idx.col, table->width[idx.col], &col_field_part);

			//
			// Each field has at least one border
			//
			num_borders.row = 1;
			num_borders.col = 1;

			//
			// If the field has a left / top border the offset is 1, else 0.
			//
			win_text.row = win_field.row + get_row_col_offset(row_table_part, idx.row);
			win_text.col = win_field.col + get_row_col_offset(col_table_part, idx.col);

			//
			// Add the field size to the text coordinate to get the end.
			//
			win_field_end.row = win_text.row + row_field_part.size;
			win_field_end.col = win_text.col + col_field_part.size;

			print_debug("dir row: %d col: %d\n", row_table_part->direction, col_table_part->direction);

			//
			//
			//
			if (row_field_part.size > 0 && col_field_part.size > 0) {

				is_cursor = idx.row == cursor->row && idx.col == cursor->col;

				if (is_cursor && idx.row == 0) {
					ncurses_set_attr(attr_cursor_header);

				} else if (is_cursor) {
					ncurses_set_attr(attr_cursor);

				} else if (idx.row == 0) {
					ncurses_set_attr(attr_header);
				}

				print_field(table->fields[idx.row][idx.col], &row_field_part, &col_field_part, win_text.row, win_text.col);

				ncurses_unset_attr();
			}

			//
			// row borders
			//
			if (row_table_part->direction == DIR_FORWARD) {
				mvhline(win_field.row, win_text.col, ACS_HLINE, col_field_part.size);

				if (not_truncated_and_last(row_table_part, idx.row)) {
					mvhline(win_field_end.row, win_text.col, ACS_HLINE, col_field_part.size);
					num_borders.row++;
				}
			} else {
				mvhline(win_field_end.row, win_text.col, ACS_HLINE, col_field_part.size);

				if (not_truncated_and_first(row_table_part, idx.row)) {
					mvhline(win_field.row, win_text.col, ACS_HLINE, col_field_part.size);
					num_borders.row++;
				}
			}

			//
			// col borders
			//
			if (col_table_part->direction == DIR_FORWARD) {
				mvvline(win_text.row, win_field.col, ACS_VLINE, row_field_part.size);

				if (not_truncated_and_last(col_table_part, idx.col)) {
					mvvline(win_text.row, win_field_end.col, ACS_VLINE, row_field_part.size);
					num_borders.col++;
				}
			} else {
				mvvline(win_text.row, win_field_end.col, ACS_VLINE, row_field_part.size);

				if (not_truncated_and_first(col_table_part, idx.col)) {
					mvvline(win_text.row, win_field.col, ACS_VLINE, row_field_part.size);
					num_borders.col++;
				}
			}

			//
			// print the field corners
			//
			if (row_table_part->direction == DIR_FORWARD) {

				if (col_table_part->direction == DIR_FORWARD) {

					//
					// corners - row: FORWARD / column: FORWARD
					//
					row_cond = not_truncated_and_last(row_table_part, idx.row);
					col_cond = not_truncated_and_last(col_table_part, idx.col);

					print_corners(&idx, &win_field, &win_field_end, row_cond, col_cond, &UL_CORNER, &LL_CORNER, &UR_CORNER, &LR_CORNER);

				} else {

					//
					// corners - row: FORWARD / column: BACKWARD
					//
					row_cond = not_truncated_and_last(row_table_part, idx.row);
					col_cond = not_truncated_and_first(col_table_part, idx.col);

					print_corners(&idx, &win_field, &win_field_end, row_cond, col_cond, &UR_CORNER, &LR_CORNER, &UL_CORNER, &LL_CORNER);
				}

			} else {

				if (col_table_part->direction == DIR_FORWARD) {

					//
					// corners - row: BACKWARD / column: FORWARD
					//
					row_cond = not_truncated_and_first(row_table_part, idx.row);
					col_cond = not_truncated_and_last(col_table_part, idx.col);

					print_corners(&idx, &win_field, &win_field_end, row_cond, col_cond, &LL_CORNER, &UL_CORNER, &LR_CORNER, &UR_CORNER);

				} else {

					//
					// corners - row: BACKWARD / column: BACKWARD
					//
					row_cond = not_truncated_and_first(row_table_part, idx.row);
					col_cond = not_truncated_and_first(col_table_part, idx.col);

					print_corners(&idx, &win_field, &win_field_end, row_cond, col_cond, &LR_CORNER, &UR_CORNER, &LL_CORNER, &UL_CORNER);
				}
			}

			print_debug("row index: %d start: %d size: %d\n", idx.row, row_field_part.start, row_field_part.size);
			print_debug("col index: %d start: %d size: %d\n", idx.col, col_field_part.start, col_field_part.size);

			win_field.col += col_field_part.size + num_borders.col;
		}

		win_field.row += row_field_part.size + num_borders.row;
	}

	//
	// without moving the cursor at the end a flickering occurs, when the
	// window is resized
	//
	move(0, 0);
	refresh();
}

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

		//
		// ensure that the cursor is visible: cursor is first with DIR_FORWARD
		//
	} else if (is_index_before_first(row_table_part, cursor->row)) {
		s_table_part_update(row_table_part, table->height, cursor->row, table->no_rows, DIR_FORWARD, win_y);

		//
		// ensure that the cursor is visible: cursor is last with DIR_BACKWARD
		//
	} else if (is_index_after_last(row_table_part, cursor->row)) {
		s_table_part_update(row_table_part, table->height, cursor->row, table->no_rows, DIR_BACKWARD, win_y);
	}

	//
	// Adjust columns
	//
	if (adjust_dir_on_resize(col_table_part, table->no_columns - 1)) {
		s_table_part_update(col_table_part, table->width, s_table_part_start(col_table_part), table->no_columns, col_table_part->direction, win_x);

		//
		// ensure that the cursor is visible: cursor is first with DIR_FORWARD
		//
	} else if (is_index_before_first(col_table_part, cursor->col)) {
		s_table_part_update(col_table_part, table->width, cursor->col, table->no_columns, DIR_FORWARD, win_x);

		//
		// ensure that the cursor is visible: cursor is last with DIR_BACKWARD
		//
	} else if (is_index_after_last(col_table_part, cursor->col)) {
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

	// TODO: init
	s_corner_inits(table);

	getmaxyx(stdscr, win_y, win_x);

	print_debug_str("curses_loop() start\n");

	s_table_part_update(&row_table_part, table->height, cursor.row, table->no_rows, DIR_FORWARD, win_y);
	s_table_part_update(&col_table_part, table->width, cursor.col, table->no_columns, DIR_FORWARD, win_x);

	while (true) {

		//
		//
		// TODO: print only if necessary (example: input sdfsd)
		//	erase();
		print_table(table, &row_table_part, &col_table_part, &cursor);

		keyInput = getch();

		if (keyInput == ERR) {
			continue; /* ignore when there was a timeout - no data */

		} else if (keyInput == 'q' || keyInput == 'Q') {
			break;

		} else if (keyInput == KEY_UP) {
			if (cursor.row - 1 >= 0) {
				cursor.row--;

				if (is_index_before_first(&row_table_part, cursor.row)) {
					s_table_part_update(&row_table_part, table->height, cursor.row, table->no_rows, DIR_FORWARD, win_y);
				}
			}

		} else if (keyInput == KEY_DOWN) {
			if (cursor.row + 1 < table->no_rows) {
				cursor.row++;

				if (is_index_after_last(&row_table_part, cursor.row)) {
					s_table_part_update(&row_table_part, table->height, cursor.row, table->no_rows, DIR_BACKWARD, win_y);
				}
			}

		} else if (keyInput == KEY_LEFT) {
			if (cursor.col - 1 >= 0) {
				cursor.col--;

				if (is_index_before_first(&col_table_part, cursor.col)) {
					s_table_part_update(&col_table_part, table->width, cursor.col, table->no_columns, DIR_FORWARD, win_x);
				}
			}

		} else if (keyInput == KEY_RIGHT) {
			if (cursor.col + 1 < table->no_columns) {
				cursor.col++;

				if (is_index_after_last(&col_table_part, cursor.col)) {
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
