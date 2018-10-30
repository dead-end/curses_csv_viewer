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
#include "ncv_common.h"
#include <string.h>

/***************************************************************************
 * Definition of the text to be displayed. From the string array, the number
 * of rows and columns can be derived. The values include a border.
 **************************************************************************/

static int win_help_size_cols;

static int win_help_size_rows;

static const char *msgs[] = {
		"CTRL-H        Shows this message.",
		"ESC           Hides dialog window.",
		"CTRL-C CTRL-Q Terminates the program.",
		"CTRL-F        Shows filter dialog.",
		"CTRL-P        Searches previous filter match.",
		"CTRL-N        Searches next filter match.",
		"CTRL-X        Deletes filter content.",
		NULL };

/***************************************************************************
 * Definition of the help window.
 **************************************************************************/

static WINDOW* win_help = NULL;

/***************************************************************************
 * The function computes the number of rows and columns for the window,
 * depending on the displayed text. The values are stored in static
 * variables.
 **************************************************************************/

static void win_help_init_sizes() {

	int row;
	int col = 0;
	int len;

	for (row = 0; msgs[row] != NULL; row++) {
		len = strlen(msgs[row]);
		if (len > col) {
			col = len;
		}
	}

	//
	// Define the number of rows and columns including the border.
	//
	win_help_size_cols = col + 2;
	win_help_size_rows = row + 2;
}

/***************************************************************************
 * The function prints the application label if the help window is large
 * enough.
 **************************************************************************/

static void win_help_print_content() {

	//
	// Add a box
	//
	if (box(win_help, 0, 0) != OK) {
		print_exit_str("win_help_print_content() Unable to setup win!\n");
	}

	//
	// If the help window has enough space for the label, it is printed.
	//
	if (WIN_HAS_MIN_SIZE(win_help_size_rows, win_help_size_cols)) {

		for (int tmp = 1; tmp < win_help_size_cols; tmp++) {
			mvwaddstr(win_help, tmp, 1, msgs[tmp - 1]);
		}
	}
}

/***************************************************************************
 * The function is called to initialize the help window. If the window
 * size is OK, the label is printed.
 **************************************************************************/

void win_help_init() {

	win_help_init_sizes();

	//
	// Create the help window at (0,0)
	//
	win_help = ncurses_win_create(win_help_size_rows, win_help_size_cols, 0, 0);

	//
	// Move the window to the center
	//
	ncurses_win_center(win_help);

	//
	// Set the help window background.
	//
	ncurses_attr_back(win_help, COLOR_PAIR(CP_STATUS), A_REVERSE);

	//
	// Print the label.
	//
	win_help_print_content();
}

/***************************************************************************
 * The function is called on resizing the terminal window.
 **************************************************************************/

void win_help_resize() {

	//
	// Ensure the minimum size of the window.
	//
	if (WIN_HAS_MIN_SIZE(win_help_size_rows, win_help_size_cols)) {
		print_debug_str("win_help_resize() Do resize the window!\n");

		if (ncurses_win_ensure_size(win_help, win_help_size_rows, win_help_size_cols)) {
			win_help_print_content();
		}

		//
		// Move the window to the center.
		//
		ncurses_win_center(win_help);
	}
}

/***************************************************************************
 * The function does a refresh with no update if the terminal is large
 * enough.
 **************************************************************************/

void win_help_refresh_no() {

	print_debug_str("win_help_refresh_no() Refresh help window.\n");
	ncurses_win_refresh_no(win_help, win_help_size_rows, win_help_size_cols);
}

/***************************************************************************
 * The function touches the window, so that a refresh has an effect.
 **************************************************************************/

void win_help_show() {

	if (touchwin(win_help) == ERR) {
		print_exit_str("win_help_show() Unable to touch help window!\n");
	}
}

/***************************************************************************
 * The function frees the allocated resources.
 **************************************************************************/

void win_help_free() {

	print_debug_str("win_help_free() Removing help window.\n");
	ncurses_win_free(win_help);
}
