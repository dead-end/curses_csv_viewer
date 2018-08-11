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

/***************************************************************************
 * Check for control modifier. Usage: CTRL('f')
 **************************************************************************/

#ifndef CTRL
#define CTRL(x) ((x) & 0x1f)
#endif

/***************************************************************************
 * CTRL chars are defined between 0 and 31 (0x1f)
 **************************************************************************/

#define is_ctrl_char(c) ((c) < 32)

/***************************************************************************
 * Define missing ncurses keys. For example ESC is missing.
 **************************************************************************/

#define NCV_KEY_ESC 27

#define NCV_KEY_NEWLINE 10


/***************************************************************************
 * The indices of color pairs
 **************************************************************************/

#define CP_TABLE 1

#define CP_TABLE_HEADER 2

#define CP_CURSOR 3

#define CP_CURSOR_HEADER 4

#define CP_STATUS 5

#define CP_FIELD 6

/***************************************************************************
 * The structure is used to switch on and off attributes. After switching
 * them on, you might want do switch them off. The struct is used to store
 * the initial value.
 ***************************************************************************/

typedef struct s_attr_reset {

	//
	// The attribute to restore
	//
	int reset;

	//
	// A flag that tells whether to restore or not.
	//
	bool do_reset;

} s_attr_reset;

void ncurses_attr_on(WINDOW *win, s_attr_reset *attr_reset, const int attr);

void ncurses_attr_off(WINDOW *win, s_attr_reset *attr_reset);

void ncurses_init(const bool monochrom, const bool use_initscr);

void ncurses_free();

chtype ncurses_attr_color(const chtype color, const chtype alt);

void ncurses_attr_back(WINDOW *win, const chtype color, const chtype alt);

WINDOW *ncurses_win_create(const int rows, const int cols, const int begin_y, const int begin_x);

void ncurses_win_move(WINDOW *win, const int to_y, const int to_x);

bool ncurses_win_resize(WINDOW *win, const int to_y, const int to_x);

#endif
