/*
 * ncv_cursor.h
 *
 */

#ifndef INC_NCV_CURSOR_H_
#define INC_NCV_CURSOR_H_

#include "ncv_common.h"

//
// Structure for the cursor.
//
typedef struct s_cursor {

	int row;

	int col;

	bool visible;

} s_cursor;

#endif /* INC_NCV_CURSOR_H_ */
