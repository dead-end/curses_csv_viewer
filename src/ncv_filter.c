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

#include "ncv_filter.h"
#include "ncv_common.h"

#include <assert.h>

/***************************************************************************
 * The function sets the values of a s_filter.
 **************************************************************************/

void s_filter_set(s_filter *filter, const bool is_active, const wchar_t *str, const bool case_insensitive) {

	wcsncpy(filter->str, str, FILTER_STR_LEN);
	filter->is_active = is_active;
	filter->case_insensitive = case_insensitive;
}

/***************************************************************************
 * The function sets the filter status to inactive. It returns true if the
 * status changed.
 **************************************************************************/

bool s_filter_set_inactive(s_filter *filter) {

	if (filter->is_active) {
		filter->is_active = false;
		return true;
	}

	return false;
}

/***************************************************************************
 * The function updates a s_filter struct with the values of an other
 * s_filter struct. It returns false if both s_filter structs have the same
 * values, so no values needed to be changed.
 **************************************************************************/

bool s_filter_update(s_filter *to_filter, const s_filter *from_filter) {
	bool result = false;

	//
	// filter string
	//
	if (wcscmp(to_filter->str, from_filter->str) != 0) {

		print_debug("s_filter_update() Filter string changed from: %ls to: %ls\n", to_filter->str, from_filter->str);
		wcsncpy(to_filter->str, from_filter->str, FILTER_STR_LEN);
		result = true;
	}

	//
	// case_insensitive flag
	//
	if (to_filter->case_insensitive != from_filter->case_insensitive) {

		print_debug("s_filter_update() Filter case flag changed from: %d to: %d\n", to_filter->case_insensitive, from_filter->case_insensitive);
		to_filter->case_insensitive = from_filter->case_insensitive;
		result = true;
	}

	//
	// is_active flag
	//
	if (to_filter->is_active != from_filter->is_active) {

		print_debug("s_filter_update() Filter active flag changed from: %d to: %d\n", to_filter->is_active, from_filter->is_active);
		to_filter->is_active = from_filter->is_active;
		result = true;
	}

	return result;
}

/***************************************************************************
 * The function searches for the next occurrence of the filter string in a
 * given string. If the filter string was not found, the function returns
 * NULL.
 **************************************************************************/

wchar_t *s_filter_search_str(const s_filter *filter, const wchar_t *str) {

	if (filter->case_insensitive) {
		return wcs_casestr(str, filter->str);
	} else {
		return wcsstr(str, filter->str);
	}
}

/***************************************************************************
 * The function print the filter structure.
 **************************************************************************/

void s_filter_print(const s_filter *filter) {

	print_debug("s_filter_print() is active: '%s' case insensitive: '%s' is search: '%s' has changed: '%s' filter: '%ls'\n",

	bool_2_str(filter->is_active), bool_2_str(filter->case_insensitive), bool_2_str(filter->is_search), bool_2_str(filter->has_changed),

	filter->str);
}
