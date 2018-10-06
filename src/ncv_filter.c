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

/***************************************************************************
 * The function initializes the filter structure.
 **************************************************************************/

void s_filter_init(s_filter *filter) {

	filter->ptr = S_FILTER_EMPTY_STR;

	filter->case_insensitive = true;
}

/***************************************************************************
 * The function frees the filter structure if necessary. This means that the
 * filter string has to be freed.
 **************************************************************************/

void s_filter_free(s_filter *filter) {

	//
	// Free filter if set.
	//
	if (filter->ptr != NULL) {
		free(filter->ptr);
	}
}

/***************************************************************************
 * The function sets a new filter string to the filter struct. The function
 * returns true if the filter string changed (set, updated or deleted). It
 * accepts EMPTY_FILTER_STRING as a filter string (which is NULL).
 **************************************************************************/

bool s_filter_set_string(s_filter *filter, const wchar_t *filter_str) {

	//
	// First check test if the filter is currently active.
	//
	if (filter->ptr != S_FILTER_EMPTY_STR) {
		print_debug("s_filter_set_string() Current filter string: '%ls'\n", filter->ptr);

		//
		// If the new filter string is empty delete the current.
		//
		if (filter_str == S_FILTER_EMPTY_STR || wcslen(filter_str) == 0) {
			print_debug_str("s_filter_set_string() Setting filter string to NULL\n");

			free(filter->ptr);
			filter->ptr = S_FILTER_EMPTY_STR;

			return true;
		}

		//
		// Check if the filter does not changed.
		//
		if (wcscmp(filter->ptr, filter_str) == 0) {
			print_debug("s_filter_set_string() Filter string did not change: %ls\n", filter_str);
			return false;
		}

		//
		// At this point, the current filter is set and the new filter is not
		// empty.
		//
		free(filter->ptr);

	} else {

		//
		// Check if the filter was not set and the new filter is empty.
		//
		if (filter_str == S_FILTER_EMPTY_STR || wcslen(filter_str) == 0) {
			print_debug_str("s_filter_set_string() Filter string was already NOT set\n");
			return false;
		}
	}

	//
	// Duplicate the filter
	//
	filter->ptr = wcsdup(filter_str);

	if (filter->ptr == NULL) {
		print_exit_str("s_filter_set_string() Unable to allocate memory!\n");
	}

	print_debug("s_filter_set_string() New filter string: '%ls'\n", filter->ptr);

	return true;
}

/***************************************************************************
 * The function searches for the next occurrence of the filter string in a
 * given string. If the filter string was not found, the function returns
 * NULL.
 **************************************************************************/

wchar_t *s_filter_search_str(const s_filter *filter, const wchar_t *str) {

	if (filter->case_insensitive) {
		return wcs_casestr(str, filter->ptr);
	} else {
		return wcsstr(str, filter->ptr);
	}
}
