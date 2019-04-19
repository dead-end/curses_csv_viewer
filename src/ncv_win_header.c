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

#include "ncv_ncurses.h"
#include "ncv_win_filter.h"
#include "ncv_common.h"

/******************************************************************************
 * Definition of the label (and its size)
 *****************************************************************************/

#define HEADER_LABEL L" ccsvv 0.2"

#define FILTER_LABEL  L"Filter"

#define SEARCH_LABEL  L"Search"

#define HEADER_BUF_SIZE 256

/******************************************************************************
 * The window has a natural minimum window size.
 *****************************************************************************/

#define WIN_HEADER_SIZE_COLS 1

#define WIN_HEADER_SIZE_ROWS 1

/******************************************************************************
 * The definition of the header window size which is the complete first row.
 *****************************************************************************/

#define WIN_HEADER_COLS getmaxx(stdscr)

#define WIN_HEADER_ROWS 1

/******************************************************************************
 * Definition of the header window.
 *****************************************************************************/

static WINDOW* win_header = NULL;

/******************************************************************************
 * The function is called to initialize the header window.
 *****************************************************************************/

void win_header_init() {

	//
	// Create the header window.
	//
	win_header = ncurses_win_create(WIN_HEADER_ROWS, WIN_HEADER_COLS, 0, 0);

	//
	// Set the header window background.
	//
	ncurses_attr_back(win_header, COLOR_PAIR(CP_STATUS), A_REVERSE);
}

/******************************************************************************
 * The function does nothing. On resizing only the content has to be printed.
 *****************************************************************************/

void win_header_resize() {
}

/******************************************************************************
 * The function prints the header line. If filtering or searching is active,
 * the filter or search string is printed. Anyway the program label is added.
 *****************************************************************************/

void win_header_content_print(const s_filter *filter) {

	//
	// Erase window to ensure that no garbage is left behind.
	//
	if (werase(win_header) == ERR) {
		print_exit_str("win_header_content_print() Unable to erase the footer window!\n");
	}

	const int max_width = getmaxx(win_header);

	int written = 0;

	if (filter->is_active) {

		const wchar_t *label = filter->is_search ? SEARCH_LABEL : FILTER_LABEL;

		wchar_t buf[HEADER_BUF_SIZE];

		swprintf(buf, HEADER_BUF_SIZE, L" %ls: %ls ", label, filter->str);

		written = nc_cond_addstr(win_header, buf, max_width, AT_RIGHT);

		//
		// If the window is too small, nothing will be written, so we can stop
		// here.
		//
		if (written <= 0) {
			return;
		}
	}

	//
	// Write the program label
	//
	nc_cond_addstr(win_header, HEADER_LABEL, max_width - written, AT_LEFT);
}

/******************************************************************************
 * The function does a refresh with no update if the terminal is large enough.
 *****************************************************************************/

void win_header_refresh_no() {

	print_debug_str("win_header_refresh_no() Refresh footer window.\n");
	ncurses_win_refresh_no(win_header, WIN_HEADER_SIZE_ROWS, WIN_HEADER_SIZE_COLS);
}

/******************************************************************************
 * The function frees the allocated resources.
 *****************************************************************************/

void win_header_free() {

	print_debug_str("win_header_free() Removing header window.\n");
	ncurses_win_free(win_header);
}
