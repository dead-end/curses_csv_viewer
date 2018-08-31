/*
 * file: ncv_curses.c
 */

#include "ncv_common.h"
#include "ncv_ncurses.h"

#include "ncv_table_part.h"

#include "ncv_ui_loop.h"
#include "ncv_win_header.h"
#include "ncv_win_filter.h"
#include "ncv_win_table.h"
#include "ncv_win_footer.h"

/***************************************************************************
 * The mode enumeration.
 **************************************************************************/

enum MODE {

	//
	// Browse through the table.
	//
	MODE_TABLE,

	//
	// Input / update filter string.
	//
	MODE_FILTER
};

/***************************************************************************
 * The function refreshes all windows that are visible. If the terminal is
 * too small, some windows disappear.
 **************************************************************************/

static void wins_refresh() {

	if (getmaxx(stdscr) > 0) {

		if (wnoutrefresh(stdscr) != OK) {
			print_exit_str("ncurses_refresh() Unable to refresh the stdscr!\n");
		}

		win_header_refresh_no();

		win_table_refresh_no();

		win_footer_refresh_no();

		//
		// The filter window is the last window to update. It is currently
		// the only window that uses the ncurses cursor.
		//
		win_filter_refresh_no();

		//
		// Copy the updates to the terminal.
		//
		if (doupdate() != OK) {
			print_exit_str("ncurses_refresh() Unable to update all wins!\n");
		}
	}
}

/***************************************************************************
 * The function resizes all windows.
 **************************************************************************/

static void wins_resize() {

	win_header_resize();

	win_filter_resize();

	win_table_resize();

	win_footer_resize();
}

/***************************************************************************
 * The function changes the mode of the application if necessary. In this
 * case the ncurses cursor and the table cursor are enabled / disabled.
 **************************************************************************/

static void change_mode(WINDOW **win, s_cursor *cursor, enum MODE *mode_current, const enum MODE mode_new) {

	//
	// Check if the application has already the mode.
	//
	if ((*mode_current) == mode_new) {
		print_debug_str("change_mode() Mode has not changed.\n");
		return;
	}

	//
	// Change the mode.
	//
	*mode_current = mode_new;

	//
	// Set the visibility of the cursor and the window to use.
	//
	switch (mode_new) {

	case MODE_TABLE:
		print_debug_str("change_mode() Switch mode to: TABLE\n");
		cursor->visible = true;
		curs_set(0);
		*win = stdscr;
		break;

	case MODE_FILTER:
		print_debug_str("change_mode() Switch mode to: FILTER\n");
		cursor->visible = false;
		curs_set(1);
		*win = win_filter_get_win();
		break;
	}
}

/***************************************************************************
 * The function is called to search the prev / next field that contains the
 * filter string. The cursor position is updated to the new position. If the
 * cursor position changed, the function returns true.
 **************************************************************************/

static bool find_prev_next(const s_table *table, s_cursor *cursor, const enum MODE mode, const int direction) {

	if (mode == MODE_TABLE) {

		//
		// Search for the prev / next occurrence. The function returns false
		// if the cursor was not updated.
		//
		if (!s_table_prev_next(table, cursor, direction)) {
			return false;
		}

		//
		// If the field cursor was updated ensure that it is visible.
		//
		win_table_set_cursor(table, cursor);

		return true;
	}

	return false;
}

/***************************************************************************
 * The buffer has to be large enough for the filter string and the string
 * terminator.
 **************************************************************************/

#define FILTER_BUF_SIZE (FILTER_FIELD_COLS + 1)

/***************************************************************************
 * The function processes user input. This can be the browsing through the
 * table or inputing a filter string.
 **************************************************************************/

void ui_loop(s_table *table, const char *filename) {

	enum MODE mode = MODE_TABLE;
	bool is_processed = false;

	//
	// A buffer for the filter string. The filter string comes from the
	// filter window and has to be passed to the table window.
	//
	wchar_t filter_buf[FILTER_BUF_SIZE];

	WINDOW *win = stdscr;

	bool do_print = true;
	bool do_continue = true;

	wint_t chr;
	int key_type;

	s_cursor cursor;

	s_table_reset_filter(table, &cursor);
	win_table_on_table_change(table);
	win_table_set_cursor(table, &cursor);

	while (do_continue) {

		print_debug("ui_loop() mode: %d do_print: %d rows: %d cols: %d\n", mode, do_print, getmaxy(win), getmaxx(win));

		//
		// Print and refresh only if something changed.
		//
		if (do_print) {
			win_table_content_print(table, &cursor);
			win_footer_content_print(filename, table, &cursor);
			wins_refresh();
			do_print = false;
		}

		//
		// Without moving the cursor at the end a flickering occurs, when the
		// window is resized.
		//
		move(0, 0);

		key_type = wget_wch(win, &chr);

		// TODO: maybe default is true
		is_processed = false;

		switch (key_type) {

		case KEY_CODE_YES:
			switch (chr) {

			case KEY_RESIZE:

				//
				// Resize the windows.
				//
				wins_resize();

				//
				// Resize the table content based on the new win_table size.
				//
				win_table_content_resize(table, &cursor);

				do_print = true;
				is_processed = true;

				break;

			default:
				print_debug("ui_loop() Found key code: %d\n", chr);
				break;
			}

			break;
		case OK:
			switch (chr) {

			//
			// ESC char
			//
			case NCV_KEY_ESC:
				print_debug("ui_loop() Found esc char: %d\n", chr);
				is_processed = true;

				//
				// Change mode if necessary.
				//
				if (mode == MODE_FILTER) {
					change_mode(&win, &cursor, &mode, MODE_TABLE);
				}

				//
				// Reset the filtering in filter and table mode.
				//
				if (s_table_set_filter_string(table, EMPTY_FILTER_STRING)) {

					//
					// Update the filter form
					//
					win_filter_clear_filter();

					//
					// If the table was reset, the cursor position has changed.
					//
					s_table_reset_filter(table, &cursor);
					win_table_on_table_change(table);
					win_table_set_cursor(table, &cursor);
				}
				werase(win_table_get_win());
				do_print = true;

				break;

				//
				// Enter chars
				//
			case KEY_ENTER:
			case NCV_KEY_NEWLINE:
				print_debug("ui_loop() Found enter char: %d\n", chr);
				is_processed = true;

				if (mode == MODE_FILTER) {
					change_mode(&win, &cursor, &mode, MODE_TABLE);

					//
					// Do the filtering
					//
					win_filter_get_filter(filter_buf, FILTER_BUF_SIZE);

					//
					// Update the table only if the filter changed.
					//

					//
					// Update the filter string and check if something changed.
					//
					if (s_table_set_filter_string(table, filter_buf)) {
						s_table_do_filter(table, &cursor);
						win_table_on_table_change(table);
					}

					//
					// On mode change to table mode the curser has always to
					// be set.
					//
					win_table_set_cursor(table, &cursor);
					werase(win_table_get_win());
					do_print = true;

				}
				break;

				//
				// Switch to FILTER mode
				//
			case CTRL('f'):
				print_debug_str("ui_loop() Found <ctrl>-f\n");
				is_processed = true;

				if (mode == MODE_TABLE) {
					change_mode(&win, &cursor, &mode, MODE_FILTER);
					do_print = true;
				}
				break;

				//
				// Deletes the filter string in FILTER mode
				//
			case CTRL('x'):
				print_debug_str("ui_loop() Found <ctrl>-x\n");
				is_processed = true;

				if (mode == MODE_FILTER) {
					win_filter_clear_filter();
					do_print = true;
				}
				break;

				//
				// Search for the filter string forward
				//
			case CTRL('n'):
				print_debug_str("ui_loop() Found <ctrl>-n\n");

				is_processed = true;
				do_print = find_prev_next(table, &cursor, mode, DIR_FORWARD);
				break;

				//
				// Search for the filter string backward
				//
			case CTRL('p'):
				print_debug_str("ui_loop() Found <ctrl>-p\n");

				is_processed = true;
				do_print = find_prev_next(table, &cursor, mode, DIR_BACKWARD);
				break;

				//
				// Quit program
				//
			case CTRL('c'):
			case CTRL('q'):
				print_debug_str("ui_loop() Found <ctrl>-c or <ctrl>-q\n");
				do_continue = false;
				is_processed = true;
				break;

			default:

				//
				// Ignore all other ctrl chars.
				//
				if (is_ctrl_char(chr)) {
					print_debug_str("ui_loop() Found <ctrl>-char\n");
					is_processed = true;
				}

				break;
			}

			break;

		case ERR:

			//
			// An error from wget_wch
			//
			print_exit_str("ui_loop() The user input caused an error!\n")
			;
			break;
		}

		//
		// Delegate the input processing
		//
		if (!is_processed) {

			switch (mode) {

			case MODE_TABLE:
				do_print = win_table_process_input(table, &cursor, key_type, chr);
				break;

			case MODE_FILTER:
				win_filter_process_input(key_type, chr);
				break;
			}
		}
	}

	print_debug_str("ui_loop() end\n");
}
