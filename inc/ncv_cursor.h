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

#ifndef INC_NCV_CURSOR_H_
#define INC_NCV_CURSOR_H_

#include <stdbool.h>

/***************************************************************************
 * Structure for the field cursor. The field cursor is simply a field of the
 * table, with a highlighted background. The structure contains the position
 * of the field and a visibility flag.
 **************************************************************************/

typedef struct s_cursor {

	//
	// The index of the field row.
	//
	int row;

	//
	// The index of the field column.
	//
	int col;

	//
	// A flag for the cursor visibility. The cursor is, for example not
	// visible if the user inputs the filter string.
	//
	// TODO: remove => consistent
	bool visible;

} s_cursor;

#define s_cursor_set(cu,r,c,v) (cu)->row = (r); (cu)->col = (c); (cu)->visible = (v)

#endif
