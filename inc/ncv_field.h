/*
 * ncv_field.h
 */

#ifndef INC_NCV_FIELD_H_
#define INC_NCV_FIELD_H_

#include <ncursesw/ncurses.h>
#include "ncv_common.h"
#include "ncv_table_part.h"

/***************************************************************************
 * A field has two s_field_part structures, one for the row and one for the
 * column. The structure defines which part of the field is visible.
 * It has a start member, which is 0 if the field is not truncated and a
 * size, which is the (maybe truncated) row height or the (maybe truncated)
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
 * Function definitions
 **************************************************************************/

//
// The functions are only visible for unit tests.
//
void s_field_part_update(const s_table_part *table_part, const int size, const int index, s_field_part *field_part);

wchar_t *get_field_line(wchar_t *str_ptr, wchar_t *buffer, const s_field_part *col_field_part);

//
// The exported functions.
//
void print_field(WINDOW *win, wchar_t *ptr, s_field_part *row_field_part, s_field_part *col_field_part, const s_field *win_row_col);

#endif /* INC_NCV_FIELD_H_ */
