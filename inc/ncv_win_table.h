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

#ifndef INC_NCV_WIN_TABLE_H_
#define INC_NCV_WIN_TABLE_H_

#include "ncv_table.h"

#include <ncursesw/ncurses.h>

/***************************************************************************
 * The struct defines an attribute that can be highlighted. It is used for
 * field texts to highlight the filter string.
 **************************************************************************/

typedef struct s_attr {

	//
	// The attribute for the normal look.
	//
	int normal;

	//
	// The attribute for the highlighted look.
	//
	int highlight;

} s_attr;

void win_table_init();

void win_table_resize();

void win_table_refresh_no();

void win_table_show();

void win_table_free();

void win_table_on_table_change(const s_table *table, s_cursor *cursor);

void win_table_content_resize(const s_table *table, s_cursor *cursor);

void win_table_set_cursor(const s_table *table, s_cursor *cursor, const int dir);

bool win_table_process_input(const s_table *table, s_cursor *cursor, const int key_type, const wint_t chr);

void win_table_content_print(const s_table *table, const s_cursor *cursor);

WINDOW *win_table_get_win();

#endif
