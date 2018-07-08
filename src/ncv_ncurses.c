/*
 * file: ncv_ncurses.c
 */

#include <ncursesw/ncurses.h>

#include "ncv_common.h"
#include "ncv_ncurses.h"

/***************************************************************************
 * The two static variables are used to be able to switch off attribute
 * after they are switched on.
 **************************************************************************/

// TODO: not a good idea
static int attr_last;

static bool attr_unset;

/***************************************************************************
 * The function switches on attributes and saves them in a static variable,
 * to be able to switch the off later.
 **************************************************************************/

//TODO: win
void ncurses_set_attr(WINDOW *win, const int attr) {

	wattron(win, attr);
	attr_last = attr;
	attr_unset = true;
}

/***************************************************************************
 * The function switches off the attributes saved in the static variable.
 **************************************************************************/
//TODO: win
void ncurses_unset_attr(WINDOW *win) {

	if (attr_unset) {
		wattroff(win, attr_last);
		attr_unset = false;
	}
}

/***************************************************************************
 * The function moves a given window to the new position, if its position
 * has changed.
 **************************************************************************/

static void ncurses_win_move(WINDOW *win, const int to_y, const int to_x) {
	int from_y, from_x;

	//
	// Check whether the position changed.
	//
	getbegyx(win, from_y, from_x);
	if (from_y == to_y && from_x == to_x) {
		print_debug("ncurses_win_move() Win position has not changed - y: %d x: %d\n", from_y, from_x);
		return;
	}

	print_debug("ncurses_win_move() Moving win from y: %d x: %d to y: %d x: %d\n", from_y, from_x, to_y, to_x);

	//
	// Do the actual moving.
	//
	if (mvwin(win, to_y, to_x) != OK) {
		print_exit_str("ncurses_win_move() Unable to move window!\n");
	}
}

/***************************************************************************
 * The function resizes all windows. The footer window has to be moved.
 **************************************************************************/

void ncurses_resize_wins() {
	int win_y, win_x;

	getmaxyx(stdscr, win_y, win_x);
	print_debug("ncurses_resize_wins() win stdscr  y: %d x: %d\n", win_y, win_x);

	//
	// A width / height of 0 is not allowed for wresize.
	//
	if (win_x == 0 || win_y == 0) {
		return;
	}

	//
	// The terminal has a min height of 1, so the header is always shown and
	// always at the same position.
	//
	if (wresize(win_header, 1, win_x) != OK) {
		print_exit("ncurses_resize_wins() Unable to resize header window with y: 1 x: %d\n", win_x);
	}

	//
	// With a min height of 2, the table window is shown.
	//
	if (win_y >= 2) {

		//
		// If the height is 2, the footer is not visible and does not affect
		// the height of the table window.
		//
		int y = (win_y == 2 ? 1 : win_y - 2);

		//
		// If win_y == 2 then the footer disappeared.
		//
		if (wresize(win_table, y, win_x) != OK) {
			print_exit("ncurses_resize_wins() Unable to resize table window with y: %d x: %d\n", y, win_x);
		}

		//
		// Ensure that the table window is not pushed outside the window. If so,
		// move it back.
		//
		ncurses_win_move(win_table, 1, 0);
	}

	//
	// The footer is only visible if the terminal has at least 3 lines.
	//
	if (win_y >= 3) {

		if (wresize(win_footer, 1, win_x) != OK) {
			print_exit("ncurses_resize_wins() Unable to resize footer window with y: 1 x: %d\n", win_x);
		}

		//
		// Move the footer to the bottom of the terminal.
		//
		ncurses_win_move(win_footer, win_y - 1, 0);
	}
}

/***************************************************************************
 * The function refreshes all windows that are visible. If the terminal is
 * too small, some windows disappear.
 **************************************************************************/

void ncurses_refresh_all() {
	int win_y, win_x;

	getmaxyx(stdscr, win_y, win_x);
	print_debug("ncurses_refresh_all() win stdscr  y: %d x: %d\n", win_y, win_x);

	if (win_x > 0) {

		if (wnoutrefresh(stdscr) != OK) {
			print_exit_str("ncurses_refresh_all() Unable to refresh the stdscr!\n");
		}

		if (win_y >= 1 && wnoutrefresh(win_header) != OK) {
			print_exit_str("ncurses_refresh_all() Unable to refresh the header win!\n");
		}

		if (win_y >= 2 && wnoutrefresh(win_table) != OK) {
			print_exit_str("ncurses_refresh_all() Unable to refresh the table win!\n");
		}

		if (win_y >= 3 && wnoutrefresh(win_footer) != OK) {
			print_exit_str("ncurses_refresh_all() Unable to refresh the footer win!\n");
		}

		//
		// Copy the updates to the terminal.
		//
		if (doupdate() != OK) {
			print_exit_str("ncurses_refresh_all() Unable to update all wins!\n");
		}
	}
}

/***************************************************************************
 * The function creates and initializes the three windows:
 *   header, table, footer
 **************************************************************************/

static void ncurses_init_wins(const int win_y, const int win_x) {

	if ((win_header = newwin(1, win_x, 0, 0)) == NULL) {
		print_exit_str("ncurses_init_wins() Unable to create header win!\n");
	}

	if ((win_table = newwin(win_y - 2, win_x, 1, 0)) == NULL) {
		print_exit_str("ncurses_init_wins() Unable to create table header win!\n");
	}

	if ((win_footer = newwin(1, win_x, win_y - 1, 0)) == NULL) {
		print_exit_str("ncurses_init_wins() Unable to create footer header win!\n");
	}

	if (has_colors()) {
		if (wbkgd(win_header, COLOR_PAIR(CP_STATUS)) != OK) {
			print_exit_str("ncurses_init_wins() Unable to set bg for header win!\n");
		}

		if (wbkgd(win_table, COLOR_PAIR(CP_TABLE))) {
			print_exit_str("ncurses_init_wins() Unable to set bg for table win!\n");
		}

		if (wbkgd(win_footer, COLOR_PAIR(CP_STATUS))) {
			print_exit_str("ncurses_init_wins() Unable to set bg for footer win!\n");
		}
	}
}

/***************************************************************************
 * The function initializes the ncurses.
 **************************************************************************/

void ncurses_init() {
	int win_y, win_x;

	if (initscr() == NULL) {
		print_exit_str("ncurses_init() Unable to init screen!\n");
	}

	getmaxyx(stdscr, win_y, win_x);

	print_debug("win - y: %d x: %d\n", win_y, win_x);

	//
	// Allow KEY_RESIZE to be read on SIGWINCH
	//
	keypad(stdscr, TRUE);

	if (has_colors()) {

		if (start_color() != OK) {
			print_exit_str("ncurses_init() Unable to init colors!\n");
		}

		if (init_pair(CP_TABLE, COLOR_WHITE, COLOR_BLUE) != OK) {
			print_exit_str("ncurses_init() Unable to init color pair!\n");
		}

		if (init_pair(CP_TABLE_HEADER, COLOR_YELLOW, COLOR_BLUE) != OK) {
			print_exit_str("ncurses_init() Unable to init color pair!\n");
		}

		if (init_pair(CP_CURSOR, COLOR_WHITE, COLOR_CYAN) != OK) {
			print_exit_str("ncurses_init() Unable to init color pair!\n");
		}

		if (init_pair(CP_CURSOR_HEADER, COLOR_YELLOW, COLOR_CYAN) != OK) {
			print_exit_str("ncurses_init() Unable to init color pair!\n");
		}

		if (init_pair(CP_STATUS, COLOR_BLACK, COLOR_WHITE) != OK) {
			print_exit_str("ncurses_init() Unable to init color pair!\n");
		}

		//
		// define the globale attributes, that can be used throughout the
		// program.
		//
		attr_header = COLOR_PAIR(CP_TABLE_HEADER) | A_BOLD;

		attr_cursor = COLOR_PAIR(CP_CURSOR) | A_BOLD;

		attr_cursor_header = COLOR_PAIR(CP_CURSOR_HEADER) | A_BOLD;
	}

	ncurses_init_wins(win_y, win_x);

	// TODO: remove
	mvwaddstr(win_header, 0, 0, "header");

	// TODO: define attributes when no colors are available.

	// scrollok(stdscr, FALSE);

	//
	// Switch off cursor by default
	//
	curs_set(0);
}

/***************************************************************************
 * A cleanup function for the ncurses stuff.
 **************************************************************************/

void ncurses_finish() {
	endwin();
}
