/*
 * ncv_ncurses.h
 */

#ifndef INC_NCV_NCURSES_H_
#define INC_NCV_NCURSES_H_

#include <ncursesw/ncurses.h>

//
// Color pairs
//

#define CP_TABLE 1

#define CP_TABLE_HEADER 2

#define CP_CURSOR 3

#define CP_CURSOR_HEADER 4

#define CP_STATUS 5

//
// gloabl attributes
//
int attr_header;

int attr_cursor;

int attr_cursor_header;

//
// The windows of the program
//
WINDOW* win_header;
WINDOW* win_table;
WINDOW* win_footer;

//
// exported functions
//
void ncurses_init();

void ncurses_finish();

void ncurses_resize_wins(const int win_y, const int win_x);

void ncurses_refresh_all(const int win_y, const int win_x);

void ncurses_set_attr(WINDOW *win, const int attr);

void ncurses_unset_attr(WINDOW *win);

#endif /* INC_NCV_NCURSES_H_ */
