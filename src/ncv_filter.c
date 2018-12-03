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
 * The function initializes the filter structure.
 **************************************************************************/

void s_filter_init(s_filter *filter) {

	filter->case_insensitive = true;

	filter->is_active = false;

	//
	// Initialize the filter string.
	//
	wmemset(filter->str, W_STR_TERM, FILTER_STR_LEN + 1);
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
 * The function sets the filter string. If the string is empty, the
 * filtering is deactivated. Otherwise it is activated. The function returns
 * true if the filter string or the is_active flag changed.
 **************************************************************************/

bool s_filter_set_string(s_filter *filter, const wchar_t *filter_str) {

	//
	// If the new filter string is empty, then filtering is not active.
	//
	if (wcslen(filter_str) == 0) {
		return s_filter_set_inactive(filter);
	}

	//
	// If the new filter string is not empty, then filtering is active.
	//
	filter->is_active = true;

	//
	// If the filtering string does not change, there is nothing to do.
	//
	if (wcscmp(filter->str, filter_str) == 0) {
		print_debug("s_filter_set_string() Filter string did not change: %ls\n", filter_str);
		return false;
	}

	wcsncpy(filter->str, filter_str, FILTER_STR_LEN);
	print_debug("s_filter_set_string() New filter string: '%ls'\n", filter->str);

	return true;
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
