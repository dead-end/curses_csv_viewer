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

//#include "ncv_sort.h"
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
	// Show the help popup.
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

/******************************************************************************
 * The function refreshes all windows that are visible. If the terminal is too
 * small, some windows disappear.
 *
 * Refreshing copies the changes of the internal window structure to the
 * screen.
 *****************************************************************************/

static void wins_refresh(const enum MODE mode) {

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
 * The function prints all windows. If configured the table window is erased
 * first.
 *****************************************************************************/

static void wins_print(const s_table *table, const s_cursor *cursor, const char *filename, const enum MODE mode, const bool do_erase) {

	print_debug("wins_print() Print wins with mode: %s\n", mode_str(mode));

	//
	// If the table content does not include the whole window, the content of
	// the unused window parts have to be erased. There can contain parts of
	// the a popup window
	//
	if (do_erase && werase(win_table_get_win()) == ERR) {
		print_exit_str("wins_print() Unable to erase table window!\n");
	}

	//
	// Print or show the main components, that are always visible.
	//
	win_table_content_print(table, cursor);

	win_footer_content_print(table, cursor, filename);

	win_header_content_print(&table->filter);

	//
	// Show the popups if necessary. They do not change, so they can be simply
	// touched.
	//
	if (mode == MODE_FILTER) {
		win_filter_content_print();

	} else if (mode == MODE_HELP) {
		win_help_content_print();
	}

	wins_refresh(mode);
}

/******************************************************************************
 * The function changes the mode of the application if necessary. In this case
 * the ncurses cursor and the table cursor are enabled / disabled.
 *
 * If the mode changed, the function returns true. If the mode is already set,
 * the function returns false.
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

		//
		// Set the filter from focus to the first field.
		//
		win_filter_prepair_show();

	} else {
		*win = stdscr;
	}

	return true;
}

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

	//
	// Initializing the table.
	//
	win_table_on_table_change(table, &cursor);

	//
	// Initial printing of the table
	//
	wins_print(table, &cursor, filename, mode, false);

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
				// Prints the content, maybe with popups. The is necessary to
				// show / hide the cursor
				//
				wins_print(table, &cursor, filename, mode, true);

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
// TODO: check
				//
				// Deactivate filtering and sorting.
				//
				const bool is_filter_reset = s_filter_set_inactive(&table->filter);
				const bool is_sort_reset = s_sort_set_inactive(&table->sort);

				if (is_filter_reset || is_sort_reset) {

					//
					// If one of the values changed, do a reset and print the
					// result.
					//
					s_table_update_filter_sort(table, &cursor, is_filter_reset, is_sort_reset);

					win_table_on_table_change(table, &cursor);
				}

// TODO: check
				//
				// If the mode changed or the filter was reset, we need to
				// redraw the windows. The function change_mode, has always to
				// be called, so the function call has to be the first
				// condition in the if statement.
				//
				if (change_mode(&win, &cursor, &mode, MODE_TABLE) || is_filter_reset || is_sort_reset) {

					//
					// Prints the content, with the table mode. The cursor is
					// shown.
					//
					wins_print(table, &cursor, filename, mode, true);
				}

				continue;

				//
				// Switch to FILTER mode
				//
			case CTRL('f'):
				print_debug_str("ui_loop() Found <ctrl>-f\n");

				//
				// On toggling the mode always changes.
				//
				change_mode(&win, &cursor, &mode, mode != MODE_FILTER ? MODE_FILTER : MODE_TABLE);

				//
				// Prints the content, maybe with the FILTER window. The is
				// necessary to show / hide the cursor
				//
				wins_print(table, &cursor, filename, mode, true);

				continue;

// TODO: check
				//
				// Sort forward
				//
			case CTRL('s'):
				print_debug_str("ui_loop() Found <ctrl>-s\n");

				s_sort_update(&table->sort, cursor.col, E_DIR_FORWARD);

				s_table_update_filter_sort(table, &cursor, false, true);

				wins_print(table, &cursor, filename, mode, true);

				continue;

// TODO: check
				//
				// Sort backward
				//
			case CTRL('r'):
				print_debug_str("ui_loop() Found <ctrl>-r\n");

				s_sort_update(&table->sort, cursor.col, E_DIR_BACKWARD);

				s_table_update_filter_sort(table, &cursor, false, true);

				wins_print(table, &cursor, filename, mode, true);

				continue;
				//
				// Show help
				//
			case CTRL('h'):
				print_debug_str("ui_loop() Found <ctrl>-h\n");

				//
				// On toggling the mode always changes.
				//
				change_mode(&win, &cursor, &mode, mode != MODE_HELP ? MODE_HELP : MODE_TABLE);

				//
				// Prints the content, maybe with the HELP window. The is
				// necessary to show / hide the cursor
				//
				wins_print(table, &cursor, filename, mode, true);

				continue;

				//
				// Quit program
				//
			case CTRL('c'):
			case CTRL('q'):
				print_debug_str("ui_loop() Found <ctrl>-c or <ctrl>-q\n");
				do_continue = false;
				continue;

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

				//
				// Print the table content. We are still in TABLE mode, so an
				// erase of the window is not necessary.
				//
				wins_print(table, &cursor, filename, mode, false);
			}

		} else if (mode == MODE_FILTER) {

			//
			// The method returns true if the filter mode finished.
			//
			if (win_filter_process_input(&table->filter, key_type, chr)) {

				//
				// Check if a new filtering is necessary.
				//
				if (s_filter_has_changed(&table->filter)) {

					print_debug_str("ui_loop() Filter changed, update table!\n");

// TODO: check
					//
					// Do the filtering of the table.
					//
					win_footer_set_msg(s_table_update_filter_sort(table, &cursor, true, false));

					//
					// After filtering the table changed.
					//
					win_table_on_table_change(table, &cursor);
				}

				//
				// Change mode from FILTER to TABLE.
				//
				change_mode(&win, &cursor, &mode, MODE_TABLE);

				//
				// Prints the content with table mode and shows the table
				// cursor. the table content may be filtered.
				//
				wins_print(table, &cursor, filename, mode, true);
			}

		} else if (mode == MODE_HELP) {

			if (win_help_process_input(key_type, chr)) {

				//
				// Change mode from HELP to TABLE.
				//
				change_mode(&win, &cursor, &mode, MODE_TABLE);

				//
				// Prints the content with table mode and shows the table
				// cursor.
				//
				wins_print(table, &cursor, filename, mode, true);
			}

		} else {
			print_exit("Invalid mode for input processing delegation: %d\n", mode);
		}
	}

	print_debug_str("ui_loop() end\n");
}
