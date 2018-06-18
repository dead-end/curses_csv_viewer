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

static int attr_last;

static bool attr_unset;

/***************************************************************************
 * The function switches on attributes and saves them in a static variable,
 * to be able to switch the off later.
 **************************************************************************/

void ncurses_set_attr(const int attr) {

	attron(attr);
	attr_last = attr;
	attr_unset = true;
}

/***************************************************************************
 * The function switches off the attributes saved in the static variable.
 **************************************************************************/

void ncurses_unset_attr() {

	if (attr_unset) {
		attroff(attr_last);
		attr_unset = false;
	}
}

/***************************************************************************
 * The function initializes the ncurses.
 **************************************************************************/

void ncurses_init() {

	if (initscr() == NULL) {
		print_exit_str("ncurses_init() Unable to init screen!\n");
	}

	//
	// Allow KEY_RESIZE to be read on SIGWINCH
	//
	keypad(stdscr, TRUE);

	if (has_colors()) {

		if (start_color() != OK) {
			print_exit_str("ncurses_init() Unable to init colors!\n");
		}

		if (init_pair(1, COLOR_WHITE, COLOR_BLUE) != OK) {
			print_exit_str("ncurses_init() Unable to init color pair!\n");
		}

		if (init_pair(2, COLOR_YELLOW, COLOR_CYAN) != OK) {
			print_exit_str("ncurses_init() Unable to init color pair!\n");
		}

		if (init_pair(3, COLOR_WHITE, COLOR_CYAN) != OK) {
			print_exit_str("ncurses_init() Unable to init color pair!\n");
		}

		if (init_pair(4, COLOR_YELLOW, COLOR_BLUE) != OK) {
			print_exit_str("ncurses_init() Unable to init color pair!\n");
		}

		if (bkgd(COLOR_PAIR(1)) != OK) {
			print_exit_str("ncurses_init() Unable to set background color pair!\n");
		}

		//
		// define the globale attributes, that can be used throughout the
		// program.
		//
		attr_header = COLOR_PAIR(4) | A_BOLD;

		attr_cursor = COLOR_PAIR(3) | A_BOLD;

		attr_cursor_header = COLOR_PAIR(2) | A_BOLD;
	}

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
