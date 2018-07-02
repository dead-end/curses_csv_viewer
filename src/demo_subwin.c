/*
 * demo_subwin.c
 *
 * The demo program is used to test the behavior of windows
 * and subwindows. There are some problems:
 *
 * - Resizing (enlarging) of subwindows does not work properly.
 *   The background will not be updated.
 *
 * - Resizing the window to 1 line, results in windows with 0
 *   height. The should be avoided.
 */
#include <ncursesw/ncurses.h>
#include <stdlib.h>

WINDOW* win_header;
WINDOW* win_main;
WINDOW* win_footer;

//
// using subwin does not work
//
#define NEW_WIN

#define STR_SIZE 20
static char string[STR_SIZE];

/**
 * The function refreshes all windows.
 */
static void refresh_all() {

#ifdef NEW_WIN
	int win_y, win_x;

	getmaxyx(stdscr, win_y, win_x);

	refresh();

	if (win_y >= 1) {
		wrefresh(win_header);
	}
	if (win_y >= 2) {
		wrefresh(win_main);
	}
	if (win_y >= 3 && win_x >= 0) {
		wrefresh(win_footer);
	}
#else
	touchwin(stdscr);

	wrefresh(win_header);
	wrefresh(win_main);
	wrefresh(win_footer);

	//
	// On enlarging the terminal the background is not updated.
	//
	wbkgd(win_header, COLOR_PAIR(1));
	wbkgd(win_main, COLOR_PAIR(2));
	wbkgd(win_footer, COLOR_PAIR(1));
#endif
}

/**
 * The function prints some infos to all of the windows to see
 * if they are updated properly,
 */
static void print_str(const int x, const char* value) {

	if (mvwaddstr(win_header, 0, x, value) != OK) {
		fprintf(stderr, "Unable to print to header window!\n");
		exit(1);
	}

	if (mvwaddstr(win_main, 0, x, value) != OK) {
		fprintf(stderr, "Unable to print main to window!\n");
		exit(1);
	}

	if (mvwaddstr(win_footer, 0, x, value) != OK) {
		fprintf(stderr, "Unable to print to footer window!\n");
		exit(1);
	}
}

/**
 * ensure that the window does not disappear.
 */
static void do_resize_win(WINDOW* win, const int win_y, const int win_x) {
	if (win_y > 0 && win_x > 0) {
		if (wresize(win, win_y, win_x) != OK) {
			fprintf(stderr, "Unable to resize window!\n");
			exit(1);
		}
	}
}

/**
 * The function resizes all windows. The footer window has to be moved.
 */
static void do_resize() {
	int win_y, win_x;


	getmaxyx(stdscr, win_y, win_x);

	do_resize_win(win_header, 1, win_x);
	do_resize_win(win_main, win_y - 2, win_x);
	do_resize_win(win_footer, 1, win_x);

	//
	// Ensure that the main window is not pushed outside the
	// window. If so, move it back.
	//
	if (win_y >= 2 && mvwin(win_main, 1, 0) != OK) {
		fprintf(stderr, "Unable to move window!\n");
		exit(1);
	}

	//
	// mvwin does not work on enlarging with subwins.
	//
	if (mvwin(win_footer, win_y - 1, 0) != OK) {
		fprintf(stderr, "Unable to move window!\n");
		exit(1);
	}

	snprintf(string, STR_SIZE, "x: %03d y: %03d", win_x, win_y);
	print_str(8, string);
}

/**
 *
 */
int main() {
	int win_y, win_x;
	int key_input;

	initscr();

	start_color();
	init_pair(1, COLOR_BLACK, COLOR_WHITE);
	init_pair(2, COLOR_WHITE, COLOR_BLUE);

	getmaxyx(stdscr, win_y, win_x);

#ifdef NEW_WIN
	win_header = newwin(1, win_x, 0, 0);
	win_main = newwin(win_y - 2, win_x, 1, 0);
	win_footer = newwin(1, win_x, win_y - 1, 0);
#else
	win_header = subwin(stdscr, 1, win_x, 0, 0);
	win_main = subwin(stdscr, win_y - 2, win_x, 1, 0);
	win_footer = subwin(stdscr, 1, win_x, win_y - 1, 0);
#endif

	wbkgd(win_header, COLOR_PAIR(1));
	wbkgd(win_main, COLOR_PAIR(2));
	wbkgd(win_footer, COLOR_PAIR(1));

	curs_set(0);
	keypad(stdscr, TRUE);

	mvwaddstr(win_header, 0, 0, "header");
	mvwaddstr(win_main, 0, 0, "main");
	mvwaddstr(win_footer, 0, 0, "footer");

	snprintf(string, STR_SIZE, "x: %03d y: %03d", win_x, win_y);
	print_str(8, string);

	refresh_all();

	while (true) {

		move(0, 0);
		key_input = getch();

		if (key_input == ERR) {
			continue;

		} else if (key_input == 'q' || key_input == 'Q') {
			break;

		} else if (key_input == KEY_RESIZE) {
			do_resize();
			refresh_all();

		} else {
			snprintf(string, STR_SIZE, "key: %d", key_input);
			print_str(23, string);
			refresh_all();
		}
	}

	endwin();
	exit(0);
}
