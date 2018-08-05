/*
 * ncv_ncurses.h
 */

#ifndef INC_NCV_NCURSES_H_
#define INC_NCV_NCURSES_H_

#include "ncv_common.h"

//
// Check for control modifier. Usage: CTRL('f')
//
#ifndef CTRL
#define CTRL(x) ((x) & 0x1f)
#endif

//
// CTRL chars are defined between 0 and 31 (0x1f)
//
#define is_ctrl_char(c) ((c) < 32)

//
// Color pairs
//
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

//
// Ncurses initializazion functions.
//
void ncurses_init(const bool monochrom, const bool use_initscr);

void ncurses_free();

chtype ncurses_attr_color(const chtype color, const chtype alt);

void ncurses_attr_back(WINDOW *win, const chtype color, const chtype alt);

//
// Window operations
//
WINDOW *ncurses_win_create(const int rows, const int cols, const int begin_y, const int begin_x);

void ncurses_win_move(WINDOW *win, const int to_y, const int to_x);

bool ncurses_win_resize(WINDOW *win, const int to_y, const int to_x);

#endif /* INC_NCV_NCURSES_H_ */
