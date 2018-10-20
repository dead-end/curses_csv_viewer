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

#include "ncv_table.h"
#include "ncv_ncurses.h"
#include "ncv_common.h"

#include <string.h>

/***************************************************************************
 * Size of the string buffer.
 **************************************************************************/

#define FOOTER_WIN_MAX 1024

/***************************************************************************
 * The footer is only visible if the terminal has at least 3 rows. One row
 * for the header and one row for the table and one row for the footer.
 **************************************************************************/

#define WIN_FOOTER_HAS_MIN_SIZE (getmaxy(stdscr) >= 3)

/***************************************************************************
 * Definition of the footer window.
 **************************************************************************/

static WINDOW* win_footer = NULL;

/***************************************************************************
 * The function is called to initialize the footer window.
 **************************************************************************/

void win_footer_init() {

	//
	// Create the footer window in the last row.
	//
	win_footer = ncurses_win_create(1, getmaxx(stdscr), getmaxy(stdscr) - 1, 0);

	//
	// Set the footer window background.
	//
	ncurses_attr_back(win_footer, COLOR_PAIR(CP_STATUS), A_REVERSE);
}

/***************************************************************************
 * The function is called on resizing the terminal window.
 **************************************************************************/

void win_footer_resize() {

	//
	// Ensure the minimum size of the window.
	//
	if (WIN_FOOTER_HAS_MIN_SIZE) {
		print_debug_str("win_footer_resize() Do resize the window!\n");

		//
		// Resize the window.
		//
		ncurses_win_resize(win_footer, 1, getmaxx(stdscr));

		//
		// Move the footer to the bottom of the terminal. This is necessary
		// even if the size of the window does not change.
		//
		ncurses_win_move(win_footer, getmaxy(stdscr) - 1, 0);
	}
}

/***************************************************************************
 * The function does a refresh with no update if the terminal is large
 * enough.
 **************************************************************************/

void win_footer_refresh_no() {

	//
	// Ensure the minimum size of the window.
	//
	if (WIN_FOOTER_HAS_MIN_SIZE) {
		print_debug_str("win_footer_refresh_no() Do refresh the window!\n");

		//
		// Do the refresh.
		//
		if (wnoutrefresh(win_footer) != OK) {
			print_exit_str("win_footer_refresh_no() Unable to refresh the window!\n");
		}
	}
}

/***************************************************************************
 * The function prints the footer line, which consists of the current row /
 * column index of the field cursor and the filename. If there is not
 * enough space, the filename is shorten or completely left out.
 **************************************************************************/

void win_footer_content_print(const s_table *table, const s_cursor *cursor, const char *filename) {
	char buf[FOOTER_WIN_MAX];
	int max_width, strlen_row_col, strlen_filename;

	//
	// Erase window to ensure that no garbage is left behind.
	//
	werase(win_footer);

	max_width = getmaxx(win_footer);

	//
	// Try to print row / column infos
	//
	if (s_table_is_empty(table)) {

		//
		// If the filtered table has no rows, then it is emtpy :o)
		//
		snprintf(buf, FOOTER_WIN_MAX, " Table is empty ");

	} else {

		//
		// Print cursor and row / column informations if they are present.
		//
		//   cursor row / num filtered rows / num rows
		//   cursor col /                     num cols
		//
		if (s_filter_is_not_empty(&table->filter)) {
			if (cursor->visible) {
				snprintf(buf, FOOTER_WIN_MAX, " Row: %d/%d[%d] Col: %d/%d ", cursor->row + 1, table->no_rows, table->__no_rows, cursor->col + 1, table->no_columns);

			} else {
				snprintf(buf, FOOTER_WIN_MAX, " Row: %d[%d] Col: %d ", table->no_rows, table->__no_rows, table->no_columns);
			}
		} else {
			if (cursor->visible) {
				snprintf(buf, FOOTER_WIN_MAX, " Row: %d/%d Col: %d/%d ", cursor->row + 1, table->no_rows, cursor->col + 1, table->no_columns);

			} else {
				snprintf(buf, FOOTER_WIN_MAX, " Row: %d Col: %d ", table->no_rows, table->no_columns);
			}
		}
	}
	strlen_row_col = (int) strlen(buf);

	//
	// If the terminal is too small, nothing is printed.
	//
	if (strlen_row_col > max_width) {
		return;
	}

	mvwaddstr(win_footer, 0, max_width - strlen_row_col, buf);

	//
	// If we read the csv file from stdin, no filename is defined.
	//
	if (filename == NULL) {
		filename = "<STDIN>";
	}

	//
	// Try to print complete filename. It is not checked whether the filename
	// it self is larger than the BUFFER_SIZE.
	//
	snprintf(buf, FOOTER_WIN_MAX, " File: %s ", filename);
	strlen_filename = (int) strlen(buf);

	if (strlen_row_col + strlen_filename <= max_width) {
		mvwaddstr(win_footer, 0, 0, buf);
		return;
	}

	//
	//  Get short filename
	//
	char *short_name = strrchr(filename, '/');
	if (short_name == NULL) {
		return;
	}
	short_name++;

	//
	// Try to print the short filename
	//
	snprintf(buf, FOOTER_WIN_MAX, " File: %s ", short_name);
	strlen_filename = (int) strlen(buf);

	if (strlen_row_col + strlen_filename <= max_width) {
		mvwaddstr(win_footer, 0, 0, buf);
	}
}

/***************************************************************************
 * The function frees the allocated resources.
 **************************************************************************/

void win_footer_free() {

	print_debug_str("win_footer_free() Removing footer window.\n");
	ncurses_win_free(win_footer);
}
