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

#include "ncv_win_table.h"
#include "ncv_win_filter.h"
#include "ncv_win_header.h"
#include "ncv_win_footer.h"
#include "ncv_win_help.h"

#include "ncv_filter.h"
#include "ncv_table_part.h"
#include "ncv_ncurses.h"
#include "ncv_common.h"

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
	MODE_FILTER,

	MODE_HELP
};

#define mode_str(m) ((m) == MODE_TABLE ? "TABLE" : "FILTER")

/***************************************************************************
 * The function refreshes all windows that are visible. If the terminal is
 * too small, some windows disappear.
 **************************************************************************/

static void wins_refresh(enum MODE mode) {

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
		//win_filter_refresh_no();

		switch (mode) {

		case MODE_FILTER:
			win_filter_refresh_no();
			break;

		case MODE_HELP:
			win_help_refresh_no();
			break;

		case MODE_TABLE:
			break;

		}

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

static void wins_resize(const s_table *table, s_cursor *cursor) {

	win_header_resize();

	win_filter_resize();

	win_table_resize();

	//
	// Resize the table content based on the new win_table size.
	//
	win_table_content_resize(table, cursor);

	win_footer_resize();

	win_help_resize();
}

/***************************************************************************
 *
 **************************************************************************/

static void wins_print(const s_table *table, const s_cursor *cursor, const char *filename, const enum MODE mode) {

	win_table_content_print(table, cursor);
	win_footer_content_print(table, cursor, filename);
	//TODO: win_header_content_print
	win_header_show();

	switch (mode) {

	case MODE_FILTER:
		win_filter_show();
		break;

	case MODE_HELP:
		win_help_show();
		break;

	case MODE_TABLE:
		break;

	}
	wins_refresh(mode);
}

/***************************************************************************
 *
 **************************************************************************/
// TODO: combine with wins_print
static void wins_touch(const enum MODE mode) {

	switch (mode) {

	case MODE_FILTER:
		win_filter_show();
		break;

	case MODE_HELP:
		win_help_show();
		break;

	case MODE_TABLE:
		win_header_show();
		win_table_show();
		win_footer_show();
		break;

	}
	wins_refresh(mode);
}

/***************************************************************************
 * The function changes the mode of the application if necessary. In this
 * case the ncurses cursor and the table cursor are enabled / disabled.
 **************************************************************************/

static bool change_mode(WINDOW **win, s_cursor *cursor, enum MODE *mode_current, const enum MODE mode_new) {

	//
	// Check if the application has already the mode.
	//
	if ((*mode_current) == mode_new) {
		print_debug_str("change_mode() Mode has not changed.\n");
		return false;
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

	case MODE_HELP:
		print_debug_str("change_mode() Switch mode to: HELP\n");
		cursor->visible = false;
		curs_set(0);
		*win = stdscr;
		break;
	}

	return true;
}

/***************************************************************************
 * The buffer has to be large enough for the filter string and the string
 * terminator.
 **************************************************************************/

#define FILTER_BUF_SIZE (FILTER_FIELD_COLS + 1)

/***************************************************************************
 * The function processes user input. It processes input that is independent
 * of the mode (TABLE / FILTER) like quit and resize and the change of the
 * mode. The input that is related to the mode is delegated to mode specific
 * functions.
 **************************************************************************/

void ui_loop(s_table *table, const char *filename) {

	//
	// The current mode od the ui (TABLE / FILTER)
	//
	enum MODE mode = MODE_TABLE;

	//
	// A flag that indicates whether the user wants to quit the program.
	//
	bool do_continue = true;

	//
	// A buffer for the filter string. The filter string comes from the
	// filter window and has to be passed to the table window.
	//
	wchar_t filter_buf[FILTER_BUF_SIZE];

	WINDOW *win = stdscr;

	wint_t chr;
	int key_type;

	//
	// Define and initialize the field cursor
	//
	s_cursor cursor;
	s_cursor_set(&cursor, 0, 0, true);

	win_table_on_table_change(table);
	win_table_set_cursor(table, &cursor, DIR_FORWARD);

	//
	// Initial printing of the table
	//
	wins_print(table, &cursor, filename, mode);

	while (do_continue) {

		print_debug("ui_loop() mode: %s (%d) rows: %d cols: %d\n", mode_str(mode), mode, getmaxy(win), getmaxx(win));

		//
		// Without moving the cursor at the end a flickering occurs, when the
		// window is resized.
		//
		move(0, 0);

		key_type = wget_wch(win, &chr);

		switch (key_type) {

		case KEY_CODE_YES:
			switch (chr) {

			case KEY_RESIZE:

				//
				// Resize the windows.
				//
				wins_resize(table, &cursor);

				//
				//
				//
				wins_print(table, &cursor, filename, mode);

				if (mode == MODE_HELP) {
					wins_touch(mode);
				}

				continue;
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

				enum MODE old_mode = mode;

				//
				// Change mode if necessary.
				//
				change_mode(&win, &cursor, &mode, MODE_TABLE);

				//
				// Reset the filtering in filter and table mode.
				//
				if (s_filter_set_string(&table->filter, S_FILTER_EMPTY_STR)) {

					//
					// Update the filter form
					//
					win_filter_clear_filter();

					//
					// If the table was reset, the cursor position has changed.
					//
					s_table_reset_filter(table, &cursor);
					win_table_on_table_change(table);
					win_table_set_cursor(table, &cursor, DIR_FORWARD);

					// TODO: Necessary / inside if
					werase(win_table_get_win());
					wins_print(table, &cursor, filename, mode);

				} else if (old_mode == MODE_HELP) {
					wins_touch(mode);
				}

				continue;
				break;

				//
				// Enter chars
				//
			case KEY_ENTER:
			case NCV_KEY_NEWLINE:
				print_debug("ui_loop() Found enter char: %d\n", chr);

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
					if (s_filter_set_string(&table->filter, filter_buf)) {
						s_table_do_filter(table, &cursor);
						win_table_on_table_change(table);
					}

					//
					// On mode change to table mode the curser has always to
					// be set.
					//
					win_table_set_cursor(table, &cursor, DIR_FORWARD);
					werase(win_table_get_win());
					wins_print(table, &cursor, filename, mode);

				}

				continue;
				break;

				//
				// Switch to FILTER mode
				//
			case CTRL('f'):
				print_debug_str("ui_loop() Found <ctrl>-f\n");

				if (change_mode(&win, &cursor, &mode, MODE_FILTER)) {
					wins_print(table, &cursor, filename, mode);
				}

				continue;
				break;

				//
				// Show help
				//
			case CTRL('h'):
				print_debug_str("ui_loop() Found <ctrl>-h\n");

				//
				// Toggle the help window.
				//
				if (mode != MODE_HELP) {
					change_mode(&win, &cursor, &mode, MODE_HELP);

				} else {
					change_mode(&win, &cursor, &mode, MODE_TABLE);
				}

				wins_touch(mode);

				continue;
				break;

				//
				// Quit program
				//
			case CTRL('c'):
			case CTRL('q'):
				print_debug_str("ui_loop() Found <ctrl>-c or <ctrl>-q\n");
				do_continue = false;
				continue;
				break;

			default:

				//print_debug("ui_loop() Delegate processed: %s\n", (is_processed ? "true" : "false"));
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
		switch (mode) {

		case MODE_TABLE:
			if (win_table_process_input(table, &cursor, key_type, chr)) {
				wins_print(table, &cursor, filename, mode);
			}
			break;

		case MODE_FILTER:
			win_filter_process_input(key_type, chr);
			break;

		case MODE_HELP:
			break;

		default:
			print_exit("Unknown mode: %d\n", mode)
			;
			break;
		}
	}

	print_debug_str("ui_loop() end\n");
}
