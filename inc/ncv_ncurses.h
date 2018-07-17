/*
 * ncv_ncurses.h
 */

#ifndef INC_NCV_NCURSES_H_
#define INC_NCV_NCURSES_H_

#include "ncv_common.h"

//TODO: Does it work???
//
// Check for control modifier
//
#ifndef CTRL
#define CTRL(c) ((c) & 037)
#endif

//
// Color pairs
//
#define CP_TABLE 1

#define CP_TABLE_HEADER 2

#define CP_CURSOR 3

#define CP_CURSOR_HEADER 4

#define CP_STATUS 5

#define CP_FIELD 6

//
// Exported functions and macros.
//
#define ncurses_attr_color(c,a) (has_colors() ? c : a)

void ncurses_attr_back(WINDOW *win, const chtype color, const chtype alt);

void ncurses_set_attr(WINDOW *win, const int attr);

void ncurses_unset_attr(WINDOW *win);

//
// Common functions for the 4 windows.
//
void ncurses_init();

void ncurses_free();

void ncurses_resize();

void ncurses_refresh();

//
// Window operations
//
WINDOW *ncurses_win_create(const int rows, const int cols, const int begin_y, const int begin_x);

void ncurses_win_move(WINDOW *win, const int to_y, const int to_x);

bool ncurses_win_resize(WINDOW *win, const int to_y, const int to_x);

#endif /* INC_NCV_NCURSES_H_ */
