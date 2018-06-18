/*
 * ncv_ncurses.h
 */

#ifndef INC_NCV_NCURSES_H_
#define INC_NCV_NCURSES_H_

//
// gloabl attributes
//
int attr_header;

int attr_cursor;

int attr_cursor_header;

//
// exported functions
//
void ncurses_set_attr(const int attr);

void ncurses_unset_attr();

void ncurses_init();

void ncurses_finish();

#endif /* INC_NCV_NCURSES_H_ */
