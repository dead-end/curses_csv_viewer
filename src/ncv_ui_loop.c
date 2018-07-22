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

		win_filter_refresh_no();

		win_table_refresh_no();

		win_footer_refresh_no();

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
 * The function switches on / off the cursor visibility.
 **************************************************************************/

static void cursor_visibility(const s_table *table, s_cursor *cursor, const bool is_visible) {

	//
	// Check if visibility changed.
	//
	if (cursor->visible == is_visible) {
		return;
	}

	cursor->visible = is_visible;
	win_table_content_print(table, cursor);
	win_table_refresh();
}

/***************************************************************************
 *
 **************************************************************************/

void ui_loop(const s_table *table, const char *filename) {

	bool do_print = true;
	bool do_continue = true;
	int key_input;

	s_cursor cursor;

	win_table_content_init(table, &cursor);

	print_debug_str("curses_loop() start\n");

	while (do_continue) {

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
		// without moving the cursor at the end a flickering occurs, when the
		// window is resized
		//
		move(0, 0);

		// TODO: get_wch(&chr);
		key_input = getch();

		switch (key_input) {

		case ERR:
			//
			// ignore when there was a timeout - no data
			//
			break;

		case 'q':
		case 'Q':
		case CTRL('q'):
			do_continue = false;
			break;

		case KEY_UP:
		case KEY_DOWN:
		case KEY_LEFT:
		case KEY_RIGHT:
			do_print = win_table_content_mv_cursor(table, &cursor, key_input);
			break;

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
			break;


		case CTRL('f'):

			cursor_visibility(table, &cursor, false);
			win_filter_loop();
			cursor_visibility(table, &cursor, true);

			break;
		}
	}

	print_debug_str("curses_loop() end\n");
}
