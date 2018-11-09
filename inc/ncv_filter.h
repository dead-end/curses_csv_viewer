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

#ifndef INC_NCV_FILTER_H_
#define INC_NCV_FILTER_H_

#include <stdbool.h>
#include <wchar.h>

/***************************************************************************
 * The filter struct contains the filter string and a flag, whether the
 * filtering is case sensitive or not,
 **************************************************************************/

typedef struct s_filter {

	//
	// The member is a pointer to the filter string. If the filter is not
	// used, the pointer should be S_FILTER_EMPTY_STR (which is NULL).
	//
	wchar_t *ptr;

	//
	// The filtering can be case sensitive or case insensitive.
	//
	bool case_insensitive;

//TODO: add active flag

} s_filter;

/***************************************************************************
 * The filtering is interpreted as deactivated if the filter string is NULL.
 **************************************************************************/

#define S_FILTER_EMPTY_STR NULL

#define s_filter_is_not_empty(f) ((f)->ptr != S_FILTER_EMPTY_STR)

void s_filter_init(s_filter *filter);

void s_filter_free(s_filter *filter);

bool s_filter_set_string(s_filter *filter, const wchar_t *filter_str);

wchar_t *s_filter_search_str(const s_filter *filter, const wchar_t *str);

#define s_filter_len(f) wcslen((f)->ptr)

#endif /* INC_NCV_FILTER_H_ */
