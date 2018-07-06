/*
 * file: ncv_curses.c
 */

#include <ncursesw/ncurses.h>

#include "ncv_common.h"
#include "ncv_table.h"
#include "ncv_curses.h"
#include "ncv_ncurses.h"

/**
 * top or left starts with border => field has an offset
 */
#define get_row_col_offset(p, i) (((p)->direction == DIR_FORWARD || ((p)->truncated == -1 && i == (p)->first)) ? 1 : 0)

/**
 *
 */
static void win_table_content_print(WINDOW *win, const s_table *table, const s_table_part *row_table_part, const s_table_part *col_table_part, const s_field *cursor) {

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

				print_field(win, table->fields[idx.row][idx.col], &row_field_part, &col_field_part, &win_text);

				ncurses_unset_attr(win_table);
			}

			//
			// row borders
			//
			if (row_table_part->direction == DIR_FORWARD) {
				mvwhline(win, win_field.row, win_text.col, ACS_HLINE, col_field_part.size);

				if (is_not_truncated_and_last(row_table_part, idx.row)) {
					mvwhline(win, win_field_end.row, win_text.col, ACS_HLINE, col_field_part.size);
					num_borders.row++;
				}
			} else {
				mvwhline(win, win_field_end.row, win_text.col, ACS_HLINE, col_field_part.size);

				if (is_not_truncated_and_first(row_table_part, idx.row)) {
					mvwhline(win, win_field.row, win_text.col, ACS_HLINE, col_field_part.size);
					num_borders.row++;
				}
			}

			//
			// col borders
			//
			if (col_table_part->direction == DIR_FORWARD) {
				mvwvline(win, win_text.row, win_field.col, ACS_VLINE, row_field_part.size);

				if (is_not_truncated_and_last(col_table_part, idx.col)) {
					mvwvline(win, win_text.row, win_field_end.col, ACS_VLINE, row_field_part.size);
					num_borders.col++;
				}
			} else {
				mvwvline(win, win_text.row, win_field_end.col, ACS_VLINE, row_field_part.size);

				if (is_not_truncated_and_first(col_table_part, idx.col)) {
					mvwvline(win, win_text.row, win_field.col, ACS_VLINE, row_field_part.size);
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

					print_corners(win, &idx, &win_field, &win_field_end, row_cond, col_cond, &UL_CORNER, &LL_CORNER, &UR_CORNER, &LR_CORNER);

				} else {

					//
					// corners - row: FORWARD / column: BACKWARD
					//
					row_cond = is_not_truncated_and_last(row_table_part, idx.row);
					col_cond = is_not_truncated_and_first(col_table_part, idx.col);

					print_corners(win, &idx, &win_field, &win_field_end, row_cond, col_cond, &UR_CORNER, &LR_CORNER, &UL_CORNER, &LL_CORNER);
				}

			} else {

				if (col_table_part->direction == DIR_FORWARD) {

					//
					// corners - row: BACKWARD / column: FORWARD
					//
					row_cond = is_not_truncated_and_first(row_table_part, idx.row);
					col_cond = is_not_truncated_and_last(col_table_part, idx.col);

					print_corners(win, &idx, &win_field, &win_field_end, row_cond, col_cond, &LL_CORNER, &UL_CORNER, &LR_CORNER, &UR_CORNER);

				} else {

					//
					// corners - row: BACKWARD / column: BACKWARD
					//
					row_cond = is_not_truncated_and_first(row_table_part, idx.row);
					col_cond = is_not_truncated_and_first(col_table_part, idx.col);

					print_corners(win, &idx, &win_field, &win_field_end, row_cond, col_cond, &LR_CORNER, &UR_CORNER, &LL_CORNER, &UL_CORNER);
				}
			}

			print_debug("row index: %d start: %d size: %d\n", idx.row, row_field_part.start, row_field_part.size);
			print_debug("col index: %d start: %d size: %d\n", idx.col, col_field_part.start, col_field_part.size);

			win_field.col += col_field_part.size + num_borders.col;
		}

		win_field.row += row_field_part.size + num_borders.row;
	}
}

/***************************************************************************
 *
 **************************************************************************/

void win_table_content_resize(const s_table *table, s_table_part *row_table_part, s_table_part *col_table_part, int win_y, int win_x, s_field *cursor) {

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

	bool update = true;
	int keyInput;

	int win_x, win_y;
	int win_table_x, win_table_y;

	s_table_part row_table_part;
	s_table_part col_table_part;

	s_field cursor;
	cursor.row = 0;
	cursor.col = 0;

	//
	// Update the corners with the table sizes
	//
	s_corner_inits(table);

	getmaxyx(stdscr, win_y, win_x);
	print_debug("curses_loop() win stdscr  y: %d x: %d\n", win_y, win_x);

	getmaxyx(win_table, win_table_y, win_table_x);
	print_debug("curses_loop() win table y: %d x: %d\n", win_table_y, win_table_x);

	print_debug_str("curses_loop() start\n");

	s_table_part_update(&row_table_part, table->height, 0, table->no_rows, DIR_FORWARD, win_table_y);
	s_table_part_update(&col_table_part, table->width, 0, table->no_columns, DIR_FORWARD, win_table_x);

	while (true) {

		//
		// Print and refresh only if something changed.
		//
		if (update) {
			win_table_content_print(win_table, table, &row_table_part, &col_table_part, &cursor);
			ncurses_refresh_all(win_y, win_x);
			update = false;
		}

		//
		// without moving the cursor at the end a flickering occurs, when the
		// window is resized
		//
		move(0, 0);

		keyInput = getch();

		//
		// ignore when there was a timeout - no data
		//
		if (keyInput == ERR) {
			continue;

		} else if (keyInput == 'q' || keyInput == 'Q') {
			break;

		} else if (keyInput == KEY_UP) {
			if (cursor.row - 1 >= 0) {
				cursor.row--;

				if (is_index_before_first(&row_table_part, cursor.row)) {
					s_table_part_update(&row_table_part, table->height, cursor.row, table->no_rows, DIR_FORWARD, win_table_y);
				}

				update = true;
			}

		} else if (keyInput == KEY_DOWN) {
			if (cursor.row + 1 < table->no_rows) {
				cursor.row++;

				if (is_index_after_last(&row_table_part, cursor.row)) {
					s_table_part_update(&row_table_part, table->height, cursor.row, table->no_rows, DIR_BACKWARD, win_table_y);
				}

				update = true;
			}

		} else if (keyInput == KEY_LEFT) {
			if (cursor.col - 1 >= 0) {
				cursor.col--;

				if (is_index_before_first(&col_table_part, cursor.col)) {
					s_table_part_update(&col_table_part, table->width, cursor.col, table->no_columns, DIR_FORWARD, win_table_x);
				}

				update = true;
			}

		} else if (keyInput == KEY_RIGHT) {
			if (cursor.col + 1 < table->no_columns) {
				cursor.col++;

				if (is_index_after_last(&col_table_part, cursor.col)) {
					s_table_part_update(&col_table_part, table->width, cursor.col, table->no_columns, DIR_BACKWARD, win_table_x);
				}

				update = true;
			}

		} else if (keyInput == KEY_RESIZE) {

			//
			// On resize of the terminal, get the new size and resize all wins.
			//
			getmaxyx(stdscr, win_y, win_x);
			print_debug("curses_loop() win stdscr  y: %d x: %d\n", win_y, win_x);
			ncurses_resize_wins(win_y, win_x);

			//
			// Resize the table content based on the new win_table size.
			//
			getmaxyx(win_table, win_table_y, win_table_x);
			print_debug("curses_loop() win table y: %d x: %d\n", win_table_y, win_table_x);
			win_table_content_resize(table, &row_table_part, &col_table_part, win_table_y, win_table_x, &cursor);

			update = true;
		}
	}

	print_debug_str("curses_loop() end\n");
}
