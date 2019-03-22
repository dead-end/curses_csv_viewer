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

/******************************************************************************
 * The mode enumeration.
 *****************************************************************************/

enum MODE {

	//
	// Browse through the table.
	//
	MODE_TABLE,

	//
	// Input / update filter string.
	//
	MODE_FILTER,

	//
	// Show the help popup
	//
	MODE_HELP
};

/******************************************************************************
 * The function returns a string representation of the mode enum.
 *****************************************************************************/

static inline char *mode_str(const enum MODE mode) {

	switch (mode) {

	case MODE_TABLE:
		return "TABLE";

	case MODE_FILTER:
		return "FILTER";

	case MODE_HELP:
		return "HELP";

	default:
		print_exit("mode_str() Unknown mode: %d\n", mode)
		;
	}

	//
	// Dead code to prevent compiler warnings.
	//
	return NULL;
}

#define is_popup(m) ((m) == MODE_HELP || (m) == MODE_FILTER)

/******************************************************************************
 * The function refreshes all windows that are visible. If the terminal is too
 * small, some windows disappear.
 *****************************************************************************/

static void wins_refresh(enum MODE mode) {

	if (getmaxx(stdscr) > 0) {

		if (wnoutrefresh(stdscr) != OK) {
			print_exit_str("ncurses_refresh() Unable to refresh the stdscr!\n");
		}

		//
		// The following windows are always visible (perhaps only partly), so
		// they are refreshed first.
		//
		win_header_refresh_no();

		win_table_refresh_no();

		win_footer_refresh_no();

		//
		// The filter and the help windows are popups, so they are refreshed
		// second.
		//
		if (mode == MODE_FILTER) {
			win_filter_refresh_no();

		} else if (mode == MODE_HELP) {
			win_help_refresh_no();
		}

		//
		// Copy the updates to the terminal.
		//
		if (doupdate() != OK) {
			print_exit_str("ncurses_refresh() Unable to update all wins!\n");
		}
	}
}

/******************************************************************************
 * The function resizes all windows.
 *****************************************************************************/

static void wins_resize(const s_table *table, s_cursor *cursor) {

	win_header_resize();

	win_footer_resize();

	win_table_resize();

	//
	// Resize the table content based on the new win_table size.
	//
	win_table_content_resize(table, cursor);

	//
	// Both popups are resized even if they are currently not visible.
	//
	win_filter_resize();

	win_help_resize();
}

/******************************************************************************
 * The function prints all windows
 *****************************************************************************/

static void wins_print(const s_table *table, const s_cursor *cursor, const char *filename, const enum MODE mode) {

	win_table_content_print(table, cursor);

	win_footer_content_print(table, cursor, filename);

	win_header_show();

	if (mode == MODE_FILTER) {
		win_filter_show();

	} else if (mode == MODE_HELP) {
		win_help_show();
	}

	wins_refresh(mode);
}

/******************************************************************************
 * The function touches the relevant windows depending on the mode.
 *****************************************************************************/

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

/******************************************************************************
 * The function changes the mode of the application if necessary. In this case
 * the ncurses cursor and the table cursor are enabled / disabled.
 *****************************************************************************/

static bool change_mode(WINDOW **win, s_cursor *cursor, enum MODE *mode_current, const enum MODE mode_new) {

	//
	// Check if the application has already the new mode.
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
	// The table cursor is only visible on table mode.
	//
	cursor->visible = (mode_new == MODE_TABLE);

	//
	// The curses cursor is only visible in forms, which means filter mode.
	//
	curs_set((mode_new == MODE_FILTER));

	if (mode_new == MODE_FILTER) {
		//
		// The cursor is set to the window, so stdscr is wrong for FILTER mode.
		// The cursor has to be set to a field, not to position (0,0)
		//
		*win = win_filter_get_win();

	} else {
		*win = stdscr;
	}

	//	//
	//	// Set the visibility of the cursor and the window to use.
	//	//
	//	switch (mode_new) {
	//
	//	case MODE_TABLE:
	//		print_debug_str("change_mode() Switch mode to: TABLE\n");
	//		cursor->visible = true;
	//		curs_set(0);
	//		*win = stdscr;
	//		break;
	//
	//	case MODE_FILTER:
	//		print_debug_str("change_mode() Switch mode to: FILTER\n");
	//		cursor->visible = false;
	//		curs_set(1);
	//
	//		//
	//		// The cursor is set to the window, so stdscr is wrong for FILTER mode.
	//		// The cursor has to be set to a field, not to position (0,0)
	//		//
	//		*win = win_filter_get_win();
	//		break;
	//
	//	case MODE_HELP:
	//		print_debug_str("change_mode() Switch mode to: HELP\n");
	//		cursor->visible = false;
	//		curs_set(0);
	//		*win = stdscr;
	//		break;
	//	}

	return true;
}

/******************************************************************************
 * The buffer has to be large enough for the filter string and the string
 * terminator.
 *****************************************************************************/

#define FILTER_BUF_SIZE (FILTER_FIELD_COLS + 1)

/******************************************************************************
 * The function processes user input. It processes input that is independent of
 * the mode (TABLE / FILTER) like quit and resize and the change of the mode.
 * The input that is related to the mode is delegated to mode specific
 * functions.
 *****************************************************************************/

void ui_loop(s_table *table, const char *filename) {

	//
	// The variables for the user input
	//
	wint_t chr;
	int key_type;

	//
	// The starting mode is TABLE
	//
	enum MODE mode = MODE_TABLE;
	WINDOW *win = stdscr;

	//
	// A flag that indicates whether the user wants to quit the program.
	//
	bool do_continue = true;

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

		//
		// Read the user input
		//
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

				if (is_popup(mode)) {
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

				const enum MODE old_mode = mode;

				//
				// Change mode if necessary.
				//
				change_mode(&win, &cursor, &mode, MODE_TABLE);

				//
				// Reset the filtering in filter and table mode.
				//
				if (s_filter_set_inactive(&table->filter)) {

					//
					// If the table was reset, the cursor position has changed.
					//
					s_table_reset_filter(table, &cursor);
					win_table_on_table_change(table);
					win_table_set_cursor(table, &cursor, DIR_FORWARD);

					// TODO: Does not seam to be necessary.
					werase(win_table_get_win());
					wins_print(table, &cursor, filename, mode);

				}

				//
				// Hide the dialog windows if necessary
				//
				//TODO: better solution
				if (is_popup(old_mode)) {
					wins_touch(mode);
				}

				continue;
				//break;

				//
				// Switch to FILTER mode
				//
			case CTRL('f'):
				print_debug_str("ui_loop() Found <ctrl>-f\n");

				if (change_mode(&win, &cursor, &mode, MODE_FILTER)) {
					win_filter_prepair_show();
					wins_print(table, &cursor, filename, mode);
				}

				continue;
				//break;

				//
				// Show help
				//
			case CTRL('h'):
//				print_debug_str("ui_loop() Found <ctrl>-h\n");
//
//				//
//				// Toggle the help window.
//				//
//				if (mode != MODE_HELP) {
//
//					change_mode(&win, &cursor, &mode, MODE_HELP);
//
//					//
//					// Erase for example old filter window, when you switch
//					// from mode filter to mode help.
//					//
//					if (werase(stdscr) == ERR) {
//						print_exit_str("ui_loop() Unable to erase table window!\n");
//					}
//					wins_print(table, &cursor, filename, mode);
//					continue;
//				}
//
//				change_mode(&win, &cursor, &mode, MODE_TABLE);
//				wins_touch(mode);
//
//				continue;
				print_debug_str("ui_loop() Found <ctrl>-h\n");

				//
				// Toggle the help window.
				//
				if (mode != MODE_HELP) {
					change_mode(&win, &cursor, &mode, MODE_HELP);

				} else {
					change_mode(&win, &cursor, &mode, MODE_TABLE);
				}

				//
				// Erase for example old filter window, when you switch
				// from mode filter to mode help.
				//
				if (werase(win_table_get_win()) == ERR) {
					print_exit_str("ui_loop() Unable to erase table window!\n");
				}

				wins_print(table, &cursor, filename, mode);

				wins_touch(mode);

				continue;

				//
				// Quit program
				//
			case CTRL('c'):
			case CTRL('q'):
				print_debug_str("ui_loop() Found <ctrl>-c or <ctrl>-q\n");
				do_continue = false;
				continue;
				//break;

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
		// Delegate the input processing to the table or the filter.
		//
		if (mode == MODE_TABLE) {

			//
			// The method processes a single input char pair. It returns true
			// if the table view changed and a redrawing is necessary.
			//
			if (win_table_process_input(table, &cursor, key_type, chr)) {
				wins_print(table, &cursor, filename, mode);
			}

		} else if (mode == MODE_FILTER) {

			//
			// The method returns true if the filter mode finished.
			//
			if (win_filter_process_input(&table->filter, key_type, chr)) {

				change_mode(&win, &cursor, &mode, MODE_TABLE);

				//
				// Check if a new filtering is necessary.
				//
				if (table->filter.has_changed) {
					print_debug_str("ui_loop() Filter changed, update table!\n");

					s_table_do_filter(table, &cursor);
					win_table_on_table_change(table);
				}

				//
				// On mode change to table mode the table cursor has always to
				// be set.
				//
				win_table_set_cursor(table, &cursor, DIR_FORWARD);

				//
				// If the table content does not include the whole window, the
				// content of the unused window parts have to be erased. They
				// can contain parts of the whole table or the filter window.
				//
				if (werase(win_table_get_win()) == ERR) {
					print_exit_str("ui_loop() Unable to erase table window!\n");
				}

				//
				// Print the table content and the footer with the new cursor
				// position.
				//
				wins_print(table, &cursor, filename, mode);
			}

		} else if (mode == MODE_HELP) {
			if (win_help_process_input(key_type, chr)) {
				change_mode(&win, &cursor, &mode, MODE_TABLE);

				//
				// On mode change to table mode the table cursor has always to
				// be set.
				//
				win_table_set_cursor(table, &cursor, DIR_FORWARD);

				//
				// If the table content does not include the whole window, the
				// content of the unused window parts have to be erased. They
				// can contain parts of the whole table or the filter window.
				//
				if (werase(win_table_get_win()) == ERR) {
					print_exit_str("ui_loop() Unable to erase table window!\n");
				}

				//
				// Print the table content and the footer with the new cursor
				// position.
				//
				wins_print(table, &cursor, filename, mode);
			}

		} else {
			print_exit("Invalid mode for input processing delegation: %d\n", mode);
		}
	}

	print_debug_str("ui_loop() end\n");
}
