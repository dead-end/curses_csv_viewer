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
 * The mode enumeration.
 **************************************************************************/

enum MODE {
	TABLE, FILTER
};

/***************************************************************************
 * The function changes the mode of the application if necessary. In this
 * case the ncurses cursor and the table cursor are enabled / disabled. The
 * function returns true if the mode changed.
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

	case TABLE:
		print_debug_str("change_mode() Switch mode to: TABLE\n");
		cursor->visible = true;
		curs_set(0);
		*win = stdscr;
		break;

	case FILTER:
		print_debug_str("change_mode() Switch mode to: FILTER\n");
		cursor->visible = false;
		curs_set(1);
		*win = win_filter_get_win();
		break;
	}

	return true;
}

/***************************************************************************
 *
 **************************************************************************/
//TODO: where defined ???
#define FILTER_BUF_SIZE (FILTER_FIELD_COLS + 1)

void ui_loop(const s_table *table, const char *filename) {

	enum MODE mode = TABLE;
	bool is_processed = false;

	wchar_t filter_buf[FILTER_BUF_SIZE];

	WINDOW *win = stdscr;

	bool do_print = true;
	bool do_continue = true;

	wint_t chr;
	int key_type;

	s_cursor cursor;

	win_table_content_init(table, &cursor);

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
			case 27:
				print_debug("ui_loop() Found esc char: %d\n", chr);
				do_print = change_mode(&win, &cursor, &mode, TABLE);
				is_processed = true;
				break;

				//
				// Enter chars
				//
			case KEY_ENTER:
			case 10:
				print_debug("ui_loop() Found enter char: %d\n", chr);
				do_print = change_mode(&win, &cursor, &mode, TABLE);
				is_processed = true;

				// TODO: update / filter table
				win_filter_get_filter(filter_buf, FILTER_BUF_SIZE);
				print_debug("ui_loop() Filter (wc): -%ls-\n", filter_buf);

				break;

				//
				// Switch to FILTER mode
				//
			case CTRL('f'):
				print_debug_str("ui_loop() Found <ctrl>-f\n");
				do_print = change_mode(&win, &cursor, &mode, FILTER);
				is_processed = true;
				break;

				//
				// Quit program
				//
			case CTRL('x'):
				print_debug_str("ui_loop() Found <ctrl>-x\n");
				do_continue = false;
				is_processed = true;
				break;

			default:
				print_debug("ui_loop() Found char: %d\n", chr);
				break;
			}

			break;

		case ERR:

			print_exit_str("ui_loop() ### ERROR!\n")
			;
			break;
		}

		//
		// Delegate the input processing
		//
		if (!is_processed) {

			switch (mode) {

			case TABLE:
				do_print = win_table_process_input(table, &cursor, key_type, chr);
				break;

			case FILTER:
				win_filter_process_input(key_type, chr);
				break;
			}
		}
	}

	print_debug_str("ui_loop() end\n");
}
