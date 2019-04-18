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

#define FILTER_STR_LEN 32

/******************************************************************************
 * The filter struct contains the filter string and a flag, whether the
 * filtering is case sensitive or not and an active flag.
 *****************************************************************************/

typedef struct s_filter {

	//
	// The flag indicates that the searching / filtering is active.
	//
	bool is_active;

	//
	// The filtering can be case sensitive or case insensitive.
	//
	bool case_insensitive;

	//
	// A flag the defines search or filter.
	//
	bool is_search;

	//
	// The actual filter string.
	//
	wchar_t str[FILTER_STR_LEN + 1];

	//
	// A flag that this filter was updated.
	//
	bool has_changed;

	//
	// The number of matches on filtering and searching.
	//
	int count;

} s_filter;

/******************************************************************************
 * For readability a few constants are defined.
 *****************************************************************************/

#define SF_IS_ACTIVE true

#define SF_IS_INACTIVE !SF_IS_ACTIVE

#define SF_IS_SEARCHING true

#define SF_IS_FILTERING !SF_IS_SEARCHING

#define SF_IS_INSENSITIVE true

#define SF_IS_SENSITIVE !SF_IS_INSENSITIVE

#define SF_HAS_CHANGED true

#define SF_HAS_NOT_CHANGED !SF_HAS_CHANGED

/******************************************************************************
 * Function and macro definitions
 *****************************************************************************/

#define s_filter_init(f) s_filter_set(f, SF_IS_INACTIVE, L"", SF_IS_INSENSITIVE, SF_IS_FILTERING, SF_HAS_NOT_CHANGED)

#define s_filter_is_active(f) ((f)->is_active)

#define s_filter_is_filtering(f) (!(f)->is_search)

#define s_filter_has_matches(f) ((f)->count > 0)

#define s_filter_has_changed(f) ((f)->has_changed)

#define s_filter_len(f) wcslen((f)->str)

void s_filter_set(s_filter *filter, const bool is_active, const wchar_t *str, const bool case_insensitive, const bool is_search, const bool has_changed);

bool s_filter_set_inactive(s_filter *filter);

bool s_filter_update(s_filter *to_filter, const s_filter *from_filter);

wchar_t *s_filter_search_str(const s_filter *filter, const wchar_t *str);

//
// Function declarations that only make sense with debug mode.
//
#ifdef DEBUG

void s_filter_print(const s_filter *filter);

#endif

#endif /* INC_NCV_FILTER_H_ */
