/*
 * ncv_win_header.c
 */

#include "ncv_common.h"
#include "ncv_ncurses.h"
#include "ncv_win_filter.h"

//
// Definition of the label (and its size)
//
#define HEADER_LABEL " ccsvv 0.1"

#define HEADER_MIN_SIZE 10

/***************************************************************************
 * The function prints the application label if the header window is large
 * enough.
 **************************************************************************/

static void win_header_print_label() {

	//
	// Filter window is on the right side and has to be subtracted.
	//
	if (getmaxx(stdscr) - HEADER_MIN_SIZE - WIN_FILTER_SIZE > 0) {
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
	win_header = ncurses_win_create(1, getmaxx(stdscr) - WIN_FILTER_SIZE, 0, 0);

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
 * The function frees the allocated resources.
 **************************************************************************/

void win_header_free() {

	if (delwin(win_header) != OK) {
		print_exit_str("win_header_free() Unable to delete header window!\n");
	}
}

/***************************************************************************
 * The function is called on resizing the terminal window.
 **************************************************************************/

void win_header_resize() {
	print_debug_str("win_header_resize() Start resize.\n");

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
