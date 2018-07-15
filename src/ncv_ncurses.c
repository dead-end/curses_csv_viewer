/*
 * file: ncv_ncurses.c
 */

#include "ncv_common.h"
#include "ncv_ncurses.h"
#include "ncv_header.h"
#include "ncv_filter.h"

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

void ncurses_win_move(WINDOW *win, const int to_y, const int to_x) {
	int from_y, from_x;

	//
	// Ensure that the target is valid.
	//
	if (to_y < 0 || to_x < 0) {
		print_exit("ncurses_win_move() Win position is not valid - y: %d x: %d\n", to_y, to_x);
	}

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
		print_debug("ncurses_win_move() stdscr max y: %d x: %d\n", getmaxy(stdscr), getmaxx(stdscr));
		print_debug("ncurses_win_move() win max y: %d x: %d\n", getmaxy(win), getmaxx(win));
		print_exit_str("ncurses_win_move() Unable to move window\n");
	}
}

/***************************************************************************
 *
 **************************************************************************/

bool ncurses_win_resize(WINDOW *win, const int to_y, const int to_x) {
	int from_y, from_x;
	int result;

	//
	// Ensure that the target is valid.
	//
	if (to_y < 1 || to_x < 1) {
		print_exit("ncurses_win_resize() Win size is not valid - y: %d x: %d\n", to_y, to_x);
	}

	//
	// Check whether the position changed.
	//
	getmaxyx(win, from_y, from_x);
	print_debug("ncurses_win_resize() Win size from y: %d x: %d to y: %d x: %d\n", from_y, from_x, to_y, to_x);

	if (from_y == to_y && from_x == to_x) {
		print_debug_str("ncurses_win_resize() Win size has not changed\n");
		return false;
	}

	if ((result = wresize(win, to_y, to_x)) != OK) {
		print_exit("ncurses_win_resize() Unable to resize window (result: %d)\n", result);
	}

	return true;
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

		if (win_y >= 1 && wnoutrefresh(win_filter) != OK) {
			print_exit_str("ncurses_refresh_all() Unable to refresh the filter win!\n");
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

	// TODO: Ensure that win_x - WIN_FILTER_SIZE > 0
	if ((win_header = newwin(1, win_x - WIN_FILTER_SIZE, 0, 0)) == NULL) {
		print_exit_str("ncurses_init_wins() Unable to create header win!\n");
	}

	if ((win_filter = newwin(1, WIN_FILTER_SIZE, 0, win_x - WIN_FILTER_SIZE)) == NULL) {
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

		if (wbkgd(win_filter, COLOR_PAIR(CP_STATUS)) != OK) {
			print_exit_str("ncurses_init_wins() Unable to set bg for header win!\n");
		}

		if (wbkgd(win_table, COLOR_PAIR(CP_TABLE)) != OK) {
			print_exit_str("ncurses_init_wins() Unable to set bg for table win!\n");
		}

		if (wbkgd(win_footer, COLOR_PAIR(CP_STATUS)) != OK) {
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

		if (init_pair(CP_FIELD, COLOR_YELLOW, COLOR_BLACK) != OK) {
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

	header_init();

	filter_init(win_filter);

	// TODO: define attributes when no colors are available.

	// scrollok(stdscr, FALSE);

	//
	// Switch off cursor by default
	//
	curs_set(0);

	//
	// Switch off echoing until the filter chars are inputed.
	//
	noecho();
}

/***************************************************************************
 * A cleanup function for the ncurses stuff.
 **************************************************************************/

void ncurses_finish() {

	filter_free();

	//
	// Delete windows
	//
	//TODO: error handling
	delwin(win_header);
	delwin(win_filter);
	delwin(win_table);
	delwin(win_footer);

	//
	// Finish ncurses
	//
	endwin();
}
