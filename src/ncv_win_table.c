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

#include "ncv_field.h"
#include "ncv_corners.h"
#include "ncv_ncurses.h"

/***************************************************************************
 * With a min height of 2, the table window is shown. One row for the header
 * and one row for the table.
 **************************************************************************/

#define WIN_TABLE_MIN_COLS 1

#define WIN_TABLE_MIN_ROWS 2

/***************************************************************************
 * Definition of the table window.
 **************************************************************************/

static WINDOW* win_table = NULL;

/***************************************************************************
 * The definitions of the various field text looks. Each text can contain
 * highlighted parts. The structure contains the normal look to be able to
 * reset the look after highlighting parts of the text.
 **************************************************************************/

static s_attr attr_table;

static s_attr attr_header;

static s_attr attr_cursor;

static s_attr attr_header_cursor;

/***************************************************************************
 * The two table parts define the visible part of the table.
 **************************************************************************/

static s_table_part row_table_part;

static s_table_part col_table_part;

/***************************************************************************
 * The macro is called with a s_cursor and a s_field. If checks whether the
 * field is the cursor field of the table.
 **************************************************************************/

#define is_field_cursor(c,i) ((i)->row == (c)->row && (i)->col == (c)->col)

/***************************************************************************
 * The function initializes the table window.
 **************************************************************************/

void win_table_init() {

	//
	// Create the table window. The size is the size of the stdscr
	// subtracted by two lines, one for the header and one for the footer.
	//
	win_table = ncurses_win_create(getmaxy(stdscr) - 2, getmaxx(stdscr), 1, 0);

	//
	// Set the background of the table window.
	//
	ncurses_attr_back(win_table, COLOR_PAIR(CP_TABLE), A_NORMAL);

	//
	// Define the attributes for the different looks of the table, normal and highlighted.
	//
	attr_table.normal = ncurses_attr_color(A_NORMAL, A_NORMAL);

	attr_table.highlight = ncurses_attr_color(COLOR_PAIR(CP_TABLE_HL), A_UNDERLINE);

	attr_header.normal = ncurses_attr_color(COLOR_PAIR(CP_TABLE_HEADER) | A_BOLD, A_BOLD);

	attr_header.highlight = ncurses_attr_color(COLOR_PAIR(CP_TABLE_HEADER_HL) | A_BOLD, A_BOLD | A_UNDERLINE);

	attr_cursor.normal = ncurses_attr_color(COLOR_PAIR(CP_CURSOR) | A_BOLD, A_REVERSE);

	attr_cursor.highlight = ncurses_attr_color(COLOR_PAIR(CP_CURSOR_HL) | A_BOLD, A_REVERSE | A_UNDERLINE);

	attr_header_cursor.normal = ncurses_attr_color(COLOR_PAIR(CP_HEADER_CURSOR) | A_BOLD, A_REVERSE | A_BOLD);

	attr_header_cursor.highlight = ncurses_attr_color(COLOR_PAIR(CP_HEADER_CURSOR_HL) | A_BOLD, A_REVERSE | A_BOLD | A_UNDERLINE);
}

/***************************************************************************
 * The function is called on resizing the terminal window.
 **************************************************************************/

void win_table_resize() {
	int win_y, win_x;

	//
	// Ensure the minimum size of the window.
	//
	if (WIN_HAS_MIN_SIZE(WIN_TABLE_MIN_ROWS, WIN_TABLE_MIN_COLS)) {
		print_debug_str("win_table_resize() Do resize the window!\n");

		getmaxyx(stdscr, win_y, win_x);

		//
		// If the height is 2, the footer is not visible and does not affect
		// the height of the table window.
		//
		int y = (win_y == 2 ? 1 : win_y - 2);

		//
		// If win_y == 2 then the footer disappeared.
		//
		ncurses_win_resize(win_table, y, win_x);

		//
		// Ensure that the table window is not pushed outside the window. If so,
		// move it back.
		//
		ncurses_win_move(win_table, 1, 0);
	}
}

/***************************************************************************
 * The function is called if the number of rows / columns of the table
 * changes. This happens on filtering and reseting the filter and after the
 * initializing.
 *
 * The function has to be called after filtering and before printing.
 **************************************************************************/

void win_table_on_table_change(const s_table *table, s_cursor *cursor) {

	print_debug_str("win_table_on_table_change() Initialize win table\n");

	//
	// Set the new position of the cursor.
	//
	win_table_set_cursor(table, cursor, DIR_FORWARD);

	//
	// Initialize the corners
	//
	s_corner_inits(table->no_rows, table->no_columns);
}

/***************************************************************************
 * The function is called on resizing the win table. It updates the row /
 * column table parts depending on the new win table size and the cursor
 * position. It has to be ensured that the (field) cursor is always visible
 * after resizing the win.
 **************************************************************************/

void win_table_content_resize(const s_table *table, s_cursor *cursor) {

	//
	// update the visible part of the table
	//
	s_table_part_update(&row_table_part, table->height, s_table_part_start(&row_table_part), table->no_rows, row_table_part.direction, getmaxy(win_table));
	s_table_part_update(&col_table_part, table->width, s_table_part_start(&col_table_part), table->no_columns, col_table_part.direction, getmaxx(win_table));

	//
	// Adjust rows (see function doc: adjust_dir_on_resize)
	//
	if (adjust_dir_on_resize(&row_table_part, table->no_rows - 1)) {
		s_table_part_update(&row_table_part, table->height, s_table_part_start(&row_table_part), table->no_rows, row_table_part.direction, getmaxy(win_table));

		//
		// ensure that the cursor is visible: cursor is first with DIR_FORWARD
		//
	} else if (is_index_before_first(&row_table_part, cursor->row)) {
		s_table_part_update(&row_table_part, table->height, cursor->row, table->no_rows, DIR_FORWARD, getmaxy(win_table));

		//
		// ensure that the cursor is visible: cursor is last with DIR_BACKWARD
		//
	} else if (is_index_after_last(&row_table_part, cursor->row)) {
		s_table_part_update(&row_table_part, table->height, cursor->row, table->no_rows, DIR_BACKWARD, getmaxy(win_table));
	}

	//
	// Adjust columns (see function doc: adjust_dir_on_resize)
	//
	if (adjust_dir_on_resize(&col_table_part, table->no_columns - 1)) {
		s_table_part_update(&col_table_part, table->width, s_table_part_start(&col_table_part), table->no_columns, col_table_part.direction, getmaxx(win_table));

		//
		// ensure that the cursor is visible: cursor is first with DIR_FORWARD
		//
	} else if (is_index_before_first(&col_table_part, cursor->col)) {
		s_table_part_update(&col_table_part, table->width, cursor->col, table->no_columns, DIR_FORWARD, getmaxx(win_table));

		//
		// ensure that the cursor is visible: cursor is last with DIR_BACKWARD
		//
	} else if (is_index_after_last(&col_table_part, cursor->col)) {
		s_table_part_update(&col_table_part, table->width, cursor->col, table->no_columns, DIR_BACKWARD, getmaxx(win_table));
	}
}

/***************************************************************************
 * The function sets the field cursor to a new position. By default the
 * direction is set to DIR_FORWARD. After this it has to be checked whether
 * the direction has to be changed. So the default is to put the cursor in
 * the upper left corner if it is possible.
 **************************************************************************/

void win_table_set_cursor(const s_table *table, s_cursor *cursor, const int dir) {

	//
	// update the visible part of the table
	//
	s_table_part_update(&row_table_part, table->height, cursor->row, table->no_rows, dir, getmaxy(win_table));
	s_table_part_update(&col_table_part, table->width, cursor->col, table->no_columns, dir, getmaxx(win_table));

	//
	// Adjust rows (see function doc: adjust_dir_on_resize)
	//
	if (adjust_dir_on_resize(&row_table_part, table->no_rows - 1)) {
		s_table_part_update(&row_table_part, table->height, s_table_part_start(&row_table_part), table->no_rows, row_table_part.direction, getmaxy(win_table));
	}

	//
	// Adjust columns (see function doc: adjust_dir_on_resize)
	//
	if (adjust_dir_on_resize(&col_table_part, table->no_columns - 1)) {
		s_table_part_update(&col_table_part, table->width, s_table_part_start(&col_table_part), table->no_columns, col_table_part.direction, getmaxx(win_table));
	}
}

/***************************************************************************
 * Top or left starts with border => field has an offset
 **************************************************************************/

#define get_row_col_offset(p, i) (((p).direction == DIR_FORWARD || ((p).truncated == -1 && i == (p).first)) ? 1 : 0)

/***************************************************************************
 * The function prints the visible part of the table, including the table
 * header (if present), the field cursor (if present).
 **************************************************************************/

void win_table_content_print(const s_table *table, const s_cursor *cursor) {

	//
	// The absolute coordinates of the field with its borders in the window.
	//
	s_field win_field;
	win_field.row = 0;

	//
	// The absolute coordinates of the fields text. This depend on the left /
	// top border, which can be a normal or an additional border.
	//
	s_field win_text;

	//
	// The field end is the absolute window coordinate of the right / bottom
	// border if it exists.
	//
	s_field win_field_end;

	//
	// The variable for the number of borders for a field, which is 1 by
	// default and 2 if the field is first or last and not truncated.
	//
	s_field num_borders;

	//
	// Initialize to prevent: [-Werror=maybe-uninitialized]
	//
	num_borders.row = 1;

	s_field_part row_field_part;
	s_field_part col_field_part;

	//
	// The variable contains the row and column of the current field to be
	// printed.
	//
	s_field idx;

	//
	// The attribute structure for normal and highlighted text.
	//
	s_attr *attr_cur;

	for (idx.row = row_table_part.first; idx.row <= row_table_part.last; idx.row++) {

		win_field.col = 0;

		print_debug("win_table_content_print() calling: s_field_part_update row: %d\n", idx.row);
		s_field_part_update(&row_field_part, &row_table_part, idx.row, table->height[idx.row]);

		for (idx.col = col_table_part.first; idx.col <= col_table_part.last; idx.col++) {

			print_debug("win_table_content_print() calling: s_field_part_update col: %d\n", idx.col);
			s_field_part_update(&col_field_part, &col_table_part, idx.col, table->width[idx.col]);

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

			print_debug("win_table_content_print() dir row: %d col: %d\n", row_table_part.direction, col_table_part.direction);

			//
			// Print the field content. It is possible that the visible part of the field
			// only consists of a border, so the field size has to be checked first.
			//
			if (row_field_part.size > 0 && col_field_part.size > 0) {

				//
				// Check whether cursor / header attribute have to be set.
				//
				if (is_field_cursor(cursor, &idx) && cursor->visible) {

					if (s_table_is_field_header(table, &idx)) {
						attr_cur = &attr_header_cursor;

					} else {
						attr_cur = &attr_cursor;
					}

				} else if (s_table_is_field_header(table, &idx)) {
					attr_cur = &attr_header;

				} else {
					attr_cur = &attr_table;
				}

				//
				// Set the attribute for the current look, if it is not the default table look.
				//
				if (attr_cur != &attr_table) {
					wattrset(win_table, attr_cur->normal);
				}

				print_field_content(win_table, table->fields[idx.row][idx.col], &row_field_part, &col_field_part, &win_text, table->width[idx.col], &table->filter, attr_cur);

				//
				// Reset the attribute the the table normal value.
				//
				if (attr_cur != &attr_table) {
					wattrset(win_table, attr_table.normal);
				}
			}

			//
			// row borders
			//
			if (row_table_part.direction == DIR_FORWARD) {
				mvwhline(win_table, win_field.row, win_text.col, ACS_HLINE, col_field_part.size);

				if (is_not_truncated_and_last(&row_table_part, idx.row)) {
					mvwhline(win_table, win_field_end.row, win_text.col, ACS_HLINE, col_field_part.size);
					num_borders.row++;
				}
			} else {
				mvwhline(win_table, win_field_end.row, win_text.col, ACS_HLINE, col_field_part.size);

				if (is_not_truncated_and_first(&row_table_part, idx.row)) {
					mvwhline(win_table, win_field.row, win_text.col, ACS_HLINE, col_field_part.size);
					num_borders.row++;
				}
			}

			//
			// col borders
			//
			if (col_table_part.direction == DIR_FORWARD) {
				mvwvline(win_table, win_text.row, win_field.col, ACS_VLINE, row_field_part.size);

				if (is_not_truncated_and_last(&col_table_part, idx.col)) {
					mvwvline(win_table, win_text.row, win_field_end.col, ACS_VLINE, row_field_part.size);
					num_borders.col++;
				}
			} else {
				mvwvline(win_table, win_text.row, win_field_end.col, ACS_VLINE, row_field_part.size);

				if (is_not_truncated_and_first(&col_table_part, idx.col)) {
					mvwvline(win_table, win_text.row, win_field.col, ACS_VLINE, row_field_part.size);
					num_borders.col++;
				}
			}

			//
			// print the field corners
			//
			s_corner_print(win_table, &idx, &win_field, &win_field_end, &row_table_part, &col_table_part);

			print_debug("win_table_content_print() row index: %d start: %d size: %d\n", idx.row, row_field_part.start, row_field_part.size);
			print_debug("win_table_content_print() col index: %d start: %d size: %d\n", idx.col, col_field_part.start, col_field_part.size);

			win_field.col += col_field_part.size + num_borders.col;
		}

		win_field.row += row_field_part.size + num_borders.row;
	}
}

/***************************************************************************
 * The function processes the user input for the table, which is moving the
 * field cursor through the table. It returns a bool value, to indicate an
 * update.
 **************************************************************************/

bool win_table_process_input(const s_table *table, s_cursor *cursor, const int key_type, const wint_t chr) {

	bool result = false;

	switch (key_type) {

	//
	// Process function keys
	//
	case KEY_CODE_YES:

		switch (chr) {

		//
		// Move the field cursor one row up
		//
		case KEY_UP:

			if (cursor->row - 1 >= 0) {
				cursor->row--;

				if (is_index_before_first(&row_table_part, cursor->row)) {
					s_table_part_update(&row_table_part, table->height, cursor->row, table->no_rows, DIR_FORWARD, getmaxy(win_table));
				}

				result = true;
			}
			break;

			//
			// Move the field cursor one row down
			//
		case KEY_DOWN:

			if (cursor->row + 1 < table->no_rows) {
				cursor->row++;

				if (is_index_after_last(&row_table_part, cursor->row)) {
					s_table_part_update(&row_table_part, table->height, cursor->row, table->no_rows, DIR_BACKWARD, getmaxy(win_table));
				}

				result = true;
			}
			break;

			//
			// Move the field cursor one column to the left
			//
		case KEY_LEFT:

			if (cursor->col - 1 >= 0) {
				cursor->col--;

				if (is_index_before_first(&col_table_part, cursor->col)) {
					s_table_part_update(&col_table_part, table->width, cursor->col, table->no_columns, DIR_FORWARD, getmaxx(win_table));
				}

				result = true;
			}
			break;

			//
			// Move the field cursor one column to the right
			//
		case KEY_RIGHT:

			if (cursor->col + 1 < table->no_columns) {
				cursor->col++;

				if (is_index_after_last(&col_table_part, cursor->col)) {
					s_table_part_update(&col_table_part, table->width, cursor->col, table->no_columns, DIR_BACKWARD, getmaxx(win_table));
				}

				result = true;
			}
			break;

			//
			// Move the field cursor one page up
			//
		case KEY_PPAGE:

			if (cursor->row > 0) {

				if (row_table_part.direction == DIR_FORWARD) {
					row_table_part.direction = DIR_BACKWARD;
				}

				cursor->row = row_table_part.first;
				win_table_set_cursor(table, cursor, DIR_BACKWARD);
				result = true;
			}
			break;

			//
			// Move the field cursor one page down
			//
		case KEY_NPAGE:

			if (cursor->row < table->no_rows - 1) {

				if (row_table_part.direction == DIR_BACKWARD) {
					row_table_part.direction = DIR_FORWARD;
				}

				cursor->row = row_table_part.last;
				win_table_set_cursor(table, cursor, DIR_FORWARD);
				result = true;
			}
			break;

			//
			// Move the field cursor to the first column in the row
			//
		case KEY_HOME:

			if (cursor->row != 0 || cursor->col != 0) {

				cursor->row = 0;
				cursor->col = 0;
				win_table_set_cursor(table, cursor, DIR_FORWARD);
				result = true;
			}
			break;

			//
			// Move the field cursor to the last column in the row
			//
		case KEY_END:

			if (cursor->row != table->no_rows - 1 || cursor->col != table->no_columns - 1) {

				cursor->row = table->no_rows - 1;
				cursor->col = table->no_columns - 1;
				win_table_set_cursor(table, cursor, DIR_BACKWARD);
				result = true;
			}
			break;

		default:
			print_debug("win_filter_process_input() Found key code: %d\n", chr);
			break;
		}

		break; // case KEY_CODE_YES

		//
		// Process normal chars
		//
	case OK:

		switch (chr) {

		//
		// Search for the filter string forward
		//
		case CTRL('n'):

			if (s_table_prev_next(table, cursor, DIR_FORWARD)) {

				win_table_set_cursor(table, cursor, DIR_FORWARD);
				result = true;
			}

			break;

			//
			// Search for the filter string backward
			//
		case CTRL('p'):

			if (s_table_prev_next(table, cursor, DIR_BACKWARD)) {

				win_table_set_cursor(table, cursor, DIR_BACKWARD);
				result = true;
			}

			break;

		default:
			print_debug("win_filter_process_input() Found key code: %d\n", chr);
			break;
		}

		break; // case OK
	}

	print_debug("win_table_process_input() new cursor position row: %d col: %d update: %d\n", cursor->row, cursor->col, result);

	return result;
}

/***************************************************************************
 * The function returns the window for the table, which is static.
 **************************************************************************/

WINDOW *win_table_get_win() {
	return win_table;
}

/***************************************************************************
 * The function does a refresh with no update if the terminal is large
 * enough.
 **************************************************************************/

void win_table_refresh_no() {

	print_debug_str("win_table_refresh_no() Refresh table window.\n");
	ncurses_win_refresh_no(win_table, WIN_TABLE_MIN_ROWS, WIN_TABLE_MIN_COLS);
}

/***************************************************************************
 * The function touches the window, so that a refresh has an effect.
 **************************************************************************/

void win_table_show() {

	if (touchwin(win_table) == ERR) {
		print_exit_str("win_table_show() Unable to touch table window!\n");
	}
}

/***************************************************************************
 * The function frees the allocated resources.
 **************************************************************************/

void win_table_free() {

	print_debug_str("win_table_free() Removing table window.\n");
	ncurses_win_free(win_table);
}

