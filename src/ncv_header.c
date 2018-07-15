/*
 * ncv_header.c
 */

#include "ncv_common.h"
#include "ncv_ncurses.h"
#include "ncv_filter.h"

//
// Definition of the label
//
#define HEADER_LABEL " ccsvv 0.1"

#define HEADER_MIN_SIZE 10

/***************************************************************************
 * The function is called to initialize the header window. If the window
 * size is OK, the label is printed.
 **************************************************************************/

void header_init() {

	if (getmaxx(stdscr) - HEADER_MIN_SIZE - WIN_FILTER_SIZE > 0) {
		mvwaddstr(win_header, 0, 0, HEADER_LABEL);
	}
}

/***************************************************************************
 * The function is called on resizing the terminal window.
 **************************************************************************/

void header_resize() {

	//
	// The terminal has a min height of 1, so the header is always shown and
	// always at the same position.
	//
	if (getmaxx(stdscr) - WIN_FILTER_SIZE > 0) {
		if (wresize(win_header, 1, getmaxx(stdscr) - WIN_FILTER_SIZE) != OK) {
			print_exit("header_resize() Unable to resize header window with y: 1 x: %d\n", getmaxx(stdscr));
		}

		wclear(win_header);

		if (getmaxx(stdscr) - HEADER_MIN_SIZE - WIN_FILTER_SIZE > 0) {
			mvwaddstr(win_header, 0, 0, HEADER_LABEL);
		}
	}
}
