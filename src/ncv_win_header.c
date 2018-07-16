/*
 * ncv_win_header.c
 */

#include "ncv_win_filter.h"
#include "ncv_common.h"
#include "ncv_ncurses.h"

//
// Definition of the label
//
#define HEADER_LABEL " ccsvv 0.1"

#define HEADER_MIN_SIZE 10

/***************************************************************************
 * Currently the win_header_init() function only prints the label.
 **************************************************************************/

#define win_header_print_label() win_header_init()

/***************************************************************************
 * The function is called to initialize the header window. If the window
 * size is OK, the label is printed.
 **************************************************************************/

void win_header_init() {

	//
	// Filter window is on the right side.
	//
	if (getmaxx(stdscr) - HEADER_MIN_SIZE - WIN_FILTER_SIZE > 0) {
		mvwaddstr(win_header, 0, 0, HEADER_LABEL);
	}
}

/***************************************************************************
 * The function is called on resizing the terminal window.
 **************************************************************************/

void win_header_resize() {

	//
	// The terminal has a min height of 1, so the header is always shown and
	// always at the same position.
	//
	if (getmaxx(stdscr) - WIN_FILTER_SIZE > 0) {

		//
		// Try to resize the window and stop if the size has not changed.
		//
		if (!ncurses_win_resize(win_header, 1, getmaxx(stdscr) - WIN_FILTER_SIZE)) {
			print_debug_str("win_header_resize() Window size has not changed!\n");
			return;
		}

		//
		// Remove garbage and print the label
		//
		wclear(win_header);

		win_header_print_label();
	}
}
