/*
 * file: ncv_curses.c
 */

#include "ncv_common.h"
#include "ncv_table_part.h"
#include "ncv_corners.h"
#include "ncv_curses.h"
#include "ncv_ncurses.h"

#include "ncv_win_filter.h"
#include "ncv_win_table.h"
#include "ncv_win_footer.h"

/***************************************************************************
 *
 **************************************************************************/

void curses_loop(const s_table *table, const char *filename) {

	bool do_print = true;
	bool do_continue = true;
	int key_input;

	s_table_part row_table_part;
	s_table_part col_table_part;

	s_field cursor;

	//
	// Update the corners with the table sizes
	//
	s_corner_inits(table);

	win_table_content_init(table, &row_table_part, &col_table_part, &cursor);

	print_debug_str("curses_loop() start\n");

	while (do_continue) {

		//
		// Print and refresh only if something changed.
		//
		if (do_print) {
			win_table_content_print(table, &row_table_part, &col_table_part, &cursor);
			win_footer_content_print(filename, table, &cursor);
			ncurses_refresh();
			do_print = false;
		}

		//
		// without moving the cursor at the end a flickering occurs, when the
		// window is resized
		//
		move(0, 0);

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
			do_print = win_table_content_mv_cursor(table, &row_table_part, &col_table_part, &cursor, key_input);
			break;

		case KEY_RESIZE:

			//
			// Resize the windows.
			//
			ncurses_resize();

			//
			// Resize the table content based on the new win_table size.
			//
			win_table_content_resize(table, &row_table_part, &col_table_part, &cursor);

			do_print = true;
			break;


		case CTRL('f'):
			win_filter_loop();
			break;
		}
	}

	print_debug_str("curses_loop() end\n");
}
