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

#ifndef INC_NCV_WIN_FILTER_H_
#define INC_NCV_WIN_FILTER_H_

#include <ncursesw/ncurses.h>

#define FILTER_FIELD_ROWS 1
#define FILTER_FIELD_COLS 32

#define FILTER_FIELD_LABEL     "Filter: "
#define FILTER_FIELD_LABEL_LEN 8

#define WIN_FILTER_SIZE (FILTER_FIELD_COLS + FILTER_FIELD_LABEL_LEN + 1)

void win_filter_init();

void win_filter_resize();

void win_filter_refresh_no();

void win_filter_free();

WINDOW *win_filter_get_win();

void win_filter_process_input(const int key_type, const wint_t chr);

void win_filter_get_filter(wchar_t *buffer, const int buf_size);

void win_filter_clear_filter();

#endif
