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

/***************************************************************************
 * Definition of the label (and its size)
 **************************************************************************/

#define HEADER_LABEL " ccsvv 0.2"

#define HEADER_LABEL_LEN 10

/***************************************************************************
 * The window has a natural minimum window size.
 **************************************************************************/

#define WIN_HEADER_SIZE_COLS 1

#define WIN_HEADER_SIZE_ROWS 1

/***************************************************************************
 * The definition of the minimum cols to be able to print the label (which
 * is of cause the label length).
 **************************************************************************/

#define WIN_HEADER_MIN_LABEL_COLS HEADER_LABEL_LEN

/***************************************************************************
 * The definition of the header window size which is the complete first row.
 **************************************************************************/

#define WIN_HEADER_COLS getmaxx(stdscr)

#define WIN_HEADER_ROWS 1

/***************************************************************************
 * Definition of the header window.
 **************************************************************************/

static WINDOW* win_header = NULL;

/***************************************************************************
 * The function prints the application label if the header window is large
 * enough.
 **************************************************************************/

static void win_header_print_label() {

	//
	// If the header window has enough space for the label, it is printed.
	//
	if (WIN_HAS_MIN_SIZE(WIN_HEADER_SIZE_ROWS, WIN_HEADER_MIN_LABEL_COLS)) {
		mvwaddstr(win_header, 0, 0, HEADER_LABEL);
	}
}

/***************************************************************************
 * The function is called to initialize the header window. If the window
 * size is OK, the label is printed.
 **************************************************************************/

void win_header_init() {

	//
	// Create the header window.
	//
	win_header = ncurses_win_create(WIN_HEADER_ROWS, WIN_HEADER_COLS, 0, 0);

	//
	// Set the header window background.
	//
	ncurses_attr_back(win_header, COLOR_PAIR(CP_STATUS), A_REVERSE);

	//
	// Print the label.
	//
	win_header_print_label();
}

/***************************************************************************
 * The function is called on resizing the terminal window.
 *
 * An explicit resizing of the window is not necessary. This is only
 * necessary if the new window size is not trivial.
 *
 * ncurses_win_resize(win_header, WIN_HEADER_ROWS, WIN_HEADER_COLS);
 **************************************************************************/

void win_header_resize() {

	//
	// Ensure the minimum size of the window.
	//
	if (WIN_HAS_MIN_SIZE(WIN_HEADER_SIZE_ROWS, WIN_HEADER_SIZE_COLS)) {
		print_debug_str("win_header_resize() Do resize the window!\n");

		//
		// Print the label
		//
		win_header_print_label();
	}
}

/***************************************************************************
 * The function does a refresh with no update if the terminal is large
 * enough.
 **************************************************************************/

void win_header_refresh_no() {

	print_debug_str("win_header_refresh_no() Refresh footer window.\n");
	ncurses_win_refresh_no(win_header, WIN_HEADER_SIZE_ROWS, WIN_HEADER_SIZE_COLS);
}

/***************************************************************************
 * The function touches the window, so that a refresh has an effect.
 **************************************************************************/

void win_header_show() {

	if (touchwin(win_header) == ERR) {
		print_exit_str("win_header_show() Unable to touch header window!\n");
	}
}

/***************************************************************************
 * The function frees the allocated resources.
 **************************************************************************/

void win_header_free() {

	print_debug_str("win_header_free() Removing header window.\n");
	ncurses_win_free(win_header);
}
