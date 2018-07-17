/*
 * nvc_win_table.c
 *
 */

#include "ncv_common.h"
#include "ncv_ncurses.h"
#include "ncv_field.h"
#include "ncv_corners.h"

//
// With a min height of 2, the table window is shown. One row for the header
// and one row for the table.
//
#define WIN_TABLE_MIN_SIZE (getmaxy(stdscr) >= 2)

//
// Definition of the table window.
//
static WINDOW* win_table;

//
// Define attributes that are dynamically set.
//
static int attr_header;

static int attr_cursor;

static int attr_cursor_header;

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
	// Set the dynamic attributes depending on whether the terminal supports
	// colors.
	//
	attr_header = ncurses_attr_color(COLOR_PAIR(CP_TABLE_HEADER) | A_BOLD, A_BOLD);

	attr_cursor = ncurses_attr_color(COLOR_PAIR(CP_CURSOR) | A_BOLD, A_REVERSE);

	attr_cursor_header = ncurses_attr_color(COLOR_PAIR(CP_CURSOR_HEADER) | A_BOLD, A_REVERSE | A_BOLD);
}

/***************************************************************************
 * The function is called on resizing the terminal window.
 **************************************************************************/

void win_table_resize() {
	int win_y, win_x;

	//
	// Ensure the minimum size of the window.
	//
	if (WIN_TABLE_MIN_SIZE) {
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
 * The function does a refresh with no update if the terminal is large
 * enough.
 **************************************************************************/

void win_table_refresh_no() {

	//
	// Ensure the minimum size of the window.
	//
	if (WIN_TABLE_MIN_SIZE) {
		print_debug_str("win_table_refresh_no() Do refresh the window!\n");

		//
		// Do the refresh.
		//
		if (wnoutrefresh(win_table) != OK) {
			print_exit_str("win_table_refresh_no() Unable to refresh the window!\n");
		}
	}
}

/***************************************************************************
 * The function frees the allocated resources.
 **************************************************************************/

void win_table_free() {

	if (delwin(win_table) != OK) {
		print_exit_str("win_table_free() Unable to delete the table window!\n");
	}
}

/***************************************************************************
 * The function initializes the cursor and the row / column table parts.
 **************************************************************************/

void win_table_content_init(const s_table *table, s_table_part *row_table_part, s_table_part *col_table_part, s_field *cursor) {

	print_debug_str("win_table_content_init() Initialize win table\n");

	//
	// Set the initial cursor position.
	//
	cursor->row = 0;
	cursor->col = 0;

	//
	// Initialize the row / column table parts.
	//
	s_table_part_update(row_table_part, table->height, 0, table->no_rows, DIR_FORWARD, getmaxy(win_table));
	s_table_part_update(col_table_part, table->width, 0, table->no_columns, DIR_FORWARD, getmaxx(win_table));
}

/***************************************************************************
 * The function is called on resizing the win table. It updates the row /
 * column table parts depending on the new win table size and the cursor
 * position. It has to be ensured that the (field) cursor is always visible
 * after resizing the win.
 **************************************************************************/

void win_table_content_resize(const s_table *table, s_table_part *row_table_part, s_table_part *col_table_part, s_field *cursor) {

	//
	// update the visible part of the table
	//
	s_table_part_update(row_table_part, table->height, s_table_part_start(row_table_part), table->no_rows, row_table_part->direction, getmaxy(win_table));
	s_table_part_update(col_table_part, table->width, s_table_part_start(col_table_part), table->no_columns, col_table_part->direction, getmaxx(win_table));

	//
	// Adjust rows (see function doc: adjust_dir_on_resize)
	//
	if (adjust_dir_on_resize(row_table_part, table->no_rows - 1)) {
		s_table_part_update(row_table_part, table->height, s_table_part_start(row_table_part), table->no_rows, row_table_part->direction, getmaxy(win_table));

		//
		// ensure that the cursor is visible: cursor is first with DIR_FORWARD
		//
	} else if (is_index_before_first(row_table_part, cursor->row)) {
		s_table_part_update(row_table_part, table->height, cursor->row, table->no_rows, DIR_FORWARD, getmaxy(win_table));

		//
		// ensure that the cursor is visible: cursor is last with DIR_BACKWARD
		//
	} else if (is_index_after_last(row_table_part, cursor->row)) {
		s_table_part_update(row_table_part, table->height, cursor->row, table->no_rows, DIR_BACKWARD, getmaxy(win_table));
	}

	//
	// Adjust columns (see function doc: adjust_dir_on_resize)
	//
	if (adjust_dir_on_resize(col_table_part, table->no_columns - 1)) {
		s_table_part_update(col_table_part, table->width, s_table_part_start(col_table_part), table->no_columns, col_table_part->direction, getmaxx(win_table));

		//
		// ensure that the cursor is visible: cursor is first with DIR_FORWARD
		//
	} else if (is_index_before_first(col_table_part, cursor->col)) {
		s_table_part_update(col_table_part, table->width, cursor->col, table->no_columns, DIR_FORWARD, getmaxx(win_table));

		//
		// ensure that the cursor is visible: cursor is last with DIR_BACKWARD
		//
	} else if (is_index_after_last(col_table_part, cursor->col)) {
		s_table_part_update(col_table_part, table->width, cursor->col, table->no_columns, DIR_BACKWARD, getmaxx(win_table));
	}
}

/***************************************************************************
 * The function processes the arrow keys for the win table, which moves the
 * (field) cursor. It returns a bool value, to indicate an update.
 **************************************************************************/

bool win_table_content_mv_cursor(const s_table *table, s_table_part *row_table_part, s_table_part *col_table_part, s_field *cursor, const int key_input) {

	bool result = false;

	print_debug("win_table_content_mv_cursor() old cursor position row: %d col: %d\n", cursor->row, cursor->col);

	switch (key_input) {

	case KEY_UP:
		if (cursor->row - 1 >= 0) {
			cursor->row--;

			if (is_index_before_first(row_table_part, cursor->row)) {
				s_table_part_update(row_table_part, table->height, cursor->row, table->no_rows, DIR_FORWARD, getmaxy(win_table));
			}

			result = true;
		}
		break;

	case KEY_DOWN:
		if (cursor->row + 1 < table->no_rows) {
			cursor->row++;

			if (is_index_after_last(row_table_part, cursor->row)) {
				s_table_part_update(row_table_part, table->height, cursor->row, table->no_rows, DIR_BACKWARD, getmaxy(win_table));
			}

			result = true;
		}
		break;

	case KEY_LEFT:
		if (cursor->col - 1 >= 0) {
			cursor->col--;

			if (is_index_before_first(col_table_part, cursor->col)) {
				s_table_part_update(col_table_part, table->width, cursor->col, table->no_columns, DIR_FORWARD, getmaxx(win_table));
			}

			result = true;
		}

		break;

	case KEY_RIGHT:
		if (cursor->col + 1 < table->no_columns) {
			cursor->col++;

			if (is_index_after_last(col_table_part, cursor->col)) {
				s_table_part_update(col_table_part, table->width, cursor->col, table->no_columns, DIR_BACKWARD, getmaxx(win_table));
			}

			result = true;
		}

		break;
	}

	print_debug("win_table_content_mv_cursor() new cursor position row: %d col: %d update: %d\n", cursor->row, cursor->col, result);

	return result;
}

/***************************************************************************
 * top or left starts with border => field has an offset
 **************************************************************************/

#define get_row_col_offset(p, i) (((p)->direction == DIR_FORWARD || ((p)->truncated == -1 && i == (p)->first)) ? 1 : 0)

/***************************************************************************
 *
 **************************************************************************/

void win_table_content_print(const s_table *table, const s_table_part *row_table_part, const s_table_part *col_table_part, const s_field *cursor) {

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
					ncurses_set_attr(win_table, attr_cursor_header);

				} else if (is_cursor) {
					ncurses_set_attr(win_table, attr_cursor);

				} else if (idx.row == 0) {
					ncurses_set_attr(win_table, attr_header);
				}

				print_field(win_table, table->fields[idx.row][idx.col], &row_field_part, &col_field_part, &win_text);

				ncurses_unset_attr(win_table);
			}

			//
			// row borders
			//
			if (row_table_part->direction == DIR_FORWARD) {
				mvwhline(win_table, win_field.row, win_text.col, ACS_HLINE, col_field_part.size);

				if (is_not_truncated_and_last(row_table_part, idx.row)) {
					mvwhline(win_table, win_field_end.row, win_text.col, ACS_HLINE, col_field_part.size);
					num_borders.row++;
				}
			} else {
				mvwhline(win_table, win_field_end.row, win_text.col, ACS_HLINE, col_field_part.size);

				if (is_not_truncated_and_first(row_table_part, idx.row)) {
					mvwhline(win_table, win_field.row, win_text.col, ACS_HLINE, col_field_part.size);
					num_borders.row++;
				}
			}

			//
			// col borders
			//
			if (col_table_part->direction == DIR_FORWARD) {
				mvwvline(win_table, win_text.row, win_field.col, ACS_VLINE, row_field_part.size);

				if (is_not_truncated_and_last(col_table_part, idx.col)) {
					mvwvline(win_table, win_text.row, win_field_end.col, ACS_VLINE, row_field_part.size);
					num_borders.col++;
				}
			} else {
				mvwvline(win_table, win_text.row, win_field_end.col, ACS_VLINE, row_field_part.size);

				if (is_not_truncated_and_first(col_table_part, idx.col)) {
					mvwvline(win_table, win_text.row, win_field.col, ACS_VLINE, row_field_part.size);
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
					row_cond = is_not_truncated_and_last(row_table_part, idx.row);
					col_cond = is_not_truncated_and_last(col_table_part, idx.col);

					print_corners(win_table, &idx, &win_field, &win_field_end, row_cond, col_cond, &UL_CORNER, &LL_CORNER, &UR_CORNER, &LR_CORNER);

				} else {

					//
					// corners - row: FORWARD / column: BACKWARD
					//
					row_cond = is_not_truncated_and_last(row_table_part, idx.row);
					col_cond = is_not_truncated_and_first(col_table_part, idx.col);

					print_corners(win_table, &idx, &win_field, &win_field_end, row_cond, col_cond, &UR_CORNER, &LR_CORNER, &UL_CORNER, &LL_CORNER);
				}

			} else {

				if (col_table_part->direction == DIR_FORWARD) {

					//
					// corners - row: BACKWARD / column: FORWARD
					//
					row_cond = is_not_truncated_and_first(row_table_part, idx.row);
					col_cond = is_not_truncated_and_last(col_table_part, idx.col);

					print_corners(win_table, &idx, &win_field, &win_field_end, row_cond, col_cond, &LL_CORNER, &UL_CORNER, &LR_CORNER, &UR_CORNER);

				} else {

					//
					// corners - row: BACKWARD / column: BACKWARD
					//
					row_cond = is_not_truncated_and_first(row_table_part, idx.row);
					col_cond = is_not_truncated_and_first(col_table_part, idx.col);

					print_corners(win_table, &idx, &win_field, &win_field_end, row_cond, col_cond, &LR_CORNER, &UR_CORNER, &LL_CORNER, &UL_CORNER);
				}
			}

			print_debug("row index: %d start: %d size: %d\n", idx.row, row_field_part.start, row_field_part.size);
			print_debug("col index: %d start: %d size: %d\n", idx.col, col_field_part.start, col_field_part.size);

			win_field.col += col_field_part.size + num_borders.col;
		}

		win_field.row += row_field_part.size + num_borders.row;
	}
}
