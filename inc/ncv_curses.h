/*
 * file: ncv_curses.h
 */

#ifndef INC_NCV_CURSES_H_
#define INC_NCV_CURSES_H_

#include "ncv_table_part.h"

/***************************************************************************
 * A field has two s_field_part structure, one for the row and one for the
 * column. The structure defines which part of the field is visible. If the
 * field is not truncated, start is 0  and size if the row height or the
 * column width.
 ***************************************************************************/

typedef struct s_field_part {

	//
	// row:    the first line that is visible
	// column: the first char that is visible
	//
	int start;

	//
	// row:    the number of lines that are visible
	// column: the number of chars that are visible
	//
	int size;

} s_field_part;

/***************************************************************************
 *
 **************************************************************************/

typedef struct s_field {

	int row;

	int col;

} s_field;

/***************************************************************************
 * Function definitions
 **************************************************************************/

//
// functions that are used throughout the program
//

void curses_loop(const s_table *table);

//
// functions that are exported for tests
//

void s_field_part_update(const s_table_part *table_part, const int size, const int index, s_field_part *field_part);

wchar_t *get_field_line(wchar_t *str_ptr, wchar_t *buffer, const s_field_part *col_field_part);

#endif /* INC_NCV_CURSES_H_ */
