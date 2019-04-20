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

/******************************************************************************
 * Size of the string buffer.
 *****************************************************************************/

#define FOOTER_BUF_SIZE 1024

/******************************************************************************
 * The footer is only visible if the terminal has at least 3 rows. One row for
 * the header and one row for the table and one row for the footer.
 *****************************************************************************/

#define WIN_FOOTER_HAS_MIN_COLS 1

#define WIN_FOOTER_HAS_MIN_ROWS 3

/******************************************************************************
 * Definition the labels, used in the window.
 *****************************************************************************/

#define LABEL_FILE L"File"

#define LABEL_ROW L"Row"

#define LABEL_COL L"Col"

/******************************************************************************
 * Definition of the footer window.
 *****************************************************************************/

static WINDOW* win_footer = NULL;

static wchar_t *msg = NULL;

static chtype attr_normal;

static chtype attr_highlight;

/******************************************************************************
 * The function sets the actual message, if one is necessary.
 *****************************************************************************/

void win_footer_set_msg(wchar_t *message) {
	print_debug("win_footer_set_msg() Message: %ls\n", message);
	msg = message;
}

/******************************************************************************
 * The function is called to initialize the footer window.
 *****************************************************************************/

void win_footer_init() {

	//
	// Create the footer window in the last row.
	//
	win_footer = ncurses_win_create(1, getmaxx(stdscr), getmaxy(stdscr) - 1, 0);

	attr_normal = ncurses_attr_color(COLOR_PAIR(CP_STATUS), A_REVERSE);

	attr_highlight = ncurses_attr_color(COLOR_PAIR(CP_MSG) | A_BOLD, A_REVERSE | A_UNDERLINE | A_BOLD);

	//
	// Set the footer window background.
	//
	ncurses_attr_back(win_footer, COLOR_PAIR(CP_STATUS), A_REVERSE);
}

/******************************************************************************
 * The function is called on resizing the terminal window.
 *
 * An explicit resizing of the window is not necessary. This is only necessary
 * if the new window size is not trivial.
 *
 * ncurses_win_resize(win_footer, 1, getmaxx(stdscr));
 *****************************************************************************/

void win_footer_resize() {

	//
	// Ensure the minimum size of the window.
	//
	if (WIN_HAS_MIN_SIZE(WIN_FOOTER_HAS_MIN_ROWS, WIN_FOOTER_HAS_MIN_COLS)) {
		print_debug_str("win_footer_resize() Do resize the window!\n");

		//
		// Move the footer to the bottom of the terminal. This is necessary
		// even if the size of the window does not change.
		//
		ncurses_win_move(win_footer, getmaxy(stdscr) - 1, 0);
	}
}

/******************************************************************************
 * The function prints the cursor and the row / column informations if they are
 * present.
 *****************************************************************************/

static void cursor_to_buf(wchar_t *buf, const int max, const s_table *table, const s_cursor *cursor) {

	if (s_filter_is_active(&table->filter) && s_filter_is_filtering(&table->filter)) {

		if (cursor->visible) {
			swprintf(buf, max, L" %ls: %d/%d[%d] %ls: %d/%d ", LABEL_ROW, cursor->row + 1, table->no_rows, table->__no_rows, LABEL_COL, cursor->col + 1, table->no_columns);

		} else {
			swprintf(buf, max, L" %ls: %d[%d] %ls: %d ", LABEL_ROW, table->no_rows, table->__no_rows, LABEL_COL, table->no_columns);
		}
	} else {
		if (cursor->visible) {
			swprintf(buf, max, L" %ls: %d/%d %ls: %d/%d ", LABEL_ROW, cursor->row + 1, table->no_rows, LABEL_COL, cursor->col + 1, table->no_columns);

		} else {
			swprintf(buf, max, L" %ls: %d %ls: %d ", LABEL_ROW, table->no_rows, LABEL_COL, table->no_columns);
		}
	}
}

/******************************************************************************
 * The function prints the footer line, which consists of the current row /
 * column index of the field cursor and the filename. If there is not enough
 * space, the filename is shorten or completely left out.
 *****************************************************************************/

void win_footer_content_print(const s_table *table, const s_cursor *cursor, const char *filename) {
	wchar_t buf[FOOTER_BUF_SIZE];

	//
	// Erase window to ensure that no garbage is left behind.
	//
	if (werase(win_footer) == ERR) {
		print_exit_str("win_footer_content_print() Unable to erase the footer window!\n");
	}

	//
	// The window width is our limit for the informations to print.
	//
	const int win_width = getmaxx(win_footer);

	int written;

	if (msg != NULL) {

		swprintf(buf, FOOTER_BUF_SIZE, L" %ls ", msg);
		written = nc_cond_addstr_attr(win_footer, buf, win_width, AT_RIGHT, attr_normal, attr_highlight);

		//
		// The message should be displayed only once.
		//
		msg = NULL;

	} else {
		cursor_to_buf(buf, FOOTER_BUF_SIZE, table, cursor);
		written = nc_cond_addstr(win_footer, buf, win_width, AT_RIGHT);
	}

	//
	// If there is not enough space for the row / column informations, we can
	// stop here.
	//
	if (written <= 0) {
		return;
	}

	//
	// If we read the csv file from stdin, no filename is defined.
	//
	if (filename == NULL) {
		filename = "<STDIN>";
	}

	//
	// Try to print the full filename
	//
	swprintf(buf, FOOTER_BUF_SIZE, L" %ls: %s", LABEL_FILE, filename);
	if (nc_cond_addstr(win_footer, buf, win_width - written, AT_LEFT) > 0) {
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
	swprintf(buf, FOOTER_BUF_SIZE, L" %ls: %s", LABEL_FILE, short_name);
	nc_cond_addstr(win_footer, buf, win_width - written, AT_LEFT);
}

/******************************************************************************
 * The function does a refresh with no update if the terminal is large enough.
 *****************************************************************************/

void win_footer_refresh_no() {

	print_debug_str("win_footer_refresh_no() Refresh footer window.\n");
	ncurses_win_refresh_no(win_footer, WIN_FOOTER_HAS_MIN_ROWS, WIN_FOOTER_HAS_MIN_COLS);
}

/******************************************************************************
 * The function frees the allocated resources.
 *****************************************************************************/

void win_footer_free() {

	print_debug_str("win_footer_free() Removing footer window.\n");
	ncurses_win_free(win_footer);
}
