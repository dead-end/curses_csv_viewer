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

#define HEADER_LABEL_LEN 10

//
// The header window shares the first row with the filter window. The filter
// window has a fixed size, so the header window has the remaining size.
//
#define WIN_HEADER_SIZE (getmaxx(stdscr) - WIN_FILTER_SIZE)

//
// The terminal has a min height of 1, so the header is always shown and
// always at the same position. The window is not shown if the terminal is
// smaller than the filter window.
//
#define WIN_HEADER_MIN_SIZE (WIN_HEADER_SIZE > 0)

//
// Definition of the header window.
//
static WINDOW* win_header;

/***************************************************************************
 * The function prints the application label if the header window is large
 * enough.
 **************************************************************************/

static void win_header_print_label() {

	//
	// If the header window has enough space for the label, it is printed.
	//
	if (WIN_HEADER_SIZE >= HEADER_LABEL_LEN) {
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
	win_header = ncurses_win_create(1, WIN_HEADER_SIZE, 0, 0);

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
 **************************************************************************/

void win_header_resize() {

	//
	// Ensure the minimum size of the window.
	//
	if (WIN_HEADER_MIN_SIZE) {
		print_debug_str("win_header_resize() Do resize the window!\n");

		//
		// Try to resize the window and stop if the size has not changed.
		//
		if (!ncurses_win_resize(win_header, 1, WIN_HEADER_SIZE)) {
			print_debug_str("win_header_resize() Window size has not changed!\n");
			return;
		}

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

	//
	// Ensure the minimum size of the window.
	//
	if (WIN_HEADER_MIN_SIZE) {
		print_debug_str("win_header_refresh_no() Do refresh the window!\n");

		//
		// Do the refresh.
		//
		if (wnoutrefresh(win_header) != OK) {
			print_exit_str("win_header_refresh_no() Unable to refresh the window!\n");
		}
	}
}

/***************************************************************************
 * The function frees the allocated resources.
 **************************************************************************/

void win_header_free() {

	if (delwin(win_header) != OK) {
		print_exit_str("win_header_free() Unable to delete header window!\n");
	}
}
