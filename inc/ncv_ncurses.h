/*
 * ncv_ncurses.h
 */

#ifndef INC_NCV_NCURSES_H_
#define INC_NCV_NCURSES_H_

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
// exported functions
//
void ncurses_set_attr(const int attr);

void ncurses_unset_attr();

void ncurses_init();

void ncurses_finish();

#endif /* INC_NCV_NCURSES_H_ */
