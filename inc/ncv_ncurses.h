/*
 * MIT License
 *
 * Copyright (c) 2018 dead-end
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#ifndef INC_NCV_NCURSES_H_
#define INC_NCV_NCURSES_H_

#include "ncv_common.h"
#include <ncursesw/ncurses.h>

/******************************************************************************
 * Check for control modifier. Usage: CTRL('f')
 *****************************************************************************/

#ifndef CTRL
#define CTRL(x) ((x) & 0x1f)
#endif

/******************************************************************************
 * CTRL chars are defined between 0 and 31 (0x1f)
 *****************************************************************************/

#define is_ctrl_char(c) ((c) < 32)

/******************************************************************************
 * Define missing ncurses keys. For example ESC is missing.
 *****************************************************************************/

#define NCV_KEY_ESC 27

#define NCV_KEY_NEWLINE 10

/******************************************************************************
 * The indices of color pairs
 *****************************************************************************/

#define CP_STATUS 1

//
// Normal colors
//
#define CP_TABLE 2

#define CP_TABLE_HEADER 3

#define CP_CURSOR 4

#define CP_HEADER_CURSOR 5

//
// Highlighted colors
//
#define CP_TABLE_HL 6

#define CP_TABLE_HEADER_HL 7

#define CP_CURSOR_HL 8

#define CP_HEADER_CURSOR_HL 9

//
// Errors / messages
//
#define CP_MSG 10

#define WIN_HAS_MIN_SIZE(r,c) (getmaxy(stdscr) >= (r) && getmaxx(stdscr) >= (c))

/******************************************************************************
 * Function definitions
 *****************************************************************************/

void ncurses_init(const bool monochrom, const bool use_initscr);

void ncurses_free();

chtype ncurses_attr_color(const chtype color, const chtype alt);

void ncurses_attr_back(WINDOW *win, const chtype color, const chtype alt);

WINDOW* ncurses_win_create(const int rows, const int cols, const int begin_y, const int begin_x);

WINDOW* ncurses_derwin_create(WINDOW *win, const int rows, const int cols, const int begin_y, const int begin_x);

void ncurses_win_move(WINDOW *win, const int to_y, const int to_x);

void ncurses_derwin_move(WINDOW *win, const int to_y, const int to_x);

void ncurses_win_center(WINDOW *win);

bool ncurses_win_resize(WINDOW *win, const int to_y, const int to_x);

bool ncurses_win_ensure_size(WINDOW *win, const int y, const int x);

void ncurses_win_refresh_no(WINDOW *win, const int min_rows, const int min_cols);

void ncurses_win_free(WINDOW *win);

int nc_cond_addstr_attr(WINDOW *win, const wchar_t *str, const int max, const enum e_align align, const chtype attr_normal, const chtype attr_highlight);

#define nc_cond_addstr(w,s,m,a) nc_cond_addstr_attr(w,s,m,a,A_NORMAL,A_NORMAL)

#endif
