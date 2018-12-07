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

#include "ut_utils.h"
#include "ncv_filter.h"

#include <stdbool.h>

/***************************************************************************
 * The function checks if two s_filter structs are equal.
 **************************************************************************/

static void s_filter_cmp(const s_filter *f1, const s_filter *f2) {

	ut_check_wchar_str(f1->str, f2->str);
	ut_check_bool(f1->is_active, f2->is_active);
	ut_check_bool(f1->case_insensitive, f2->case_insensitive);
}

/***************************************************************************
 * The function the update of the s_filter struct.
 **************************************************************************/

static void test_filter_update() {
	s_filter to_filter;
	s_filter from_filter;
	bool result;

	print_debug_str("test_filter_update() Start\n");

	//
	// Equal => no update
	//
	s_filter_set(&to_filter, true, L"Hello", false);
	s_filter_set(&from_filter, true, L"Hello", false);

	result = s_filter_update(&to_filter, &from_filter);
	s_filter_cmp(&to_filter, &from_filter);

	ut_check_bool(result, false);

	//
	// String differ => update
	//
	s_filter_set(&to_filter, true, L"hello", false);
	s_filter_set(&from_filter, true, L"Hello", false);

	result = s_filter_update(&to_filter, &from_filter);
	s_filter_cmp(&to_filter, &from_filter);

	ut_check_bool(result, true);

	//
	// Active differ => update
	//
	s_filter_set(&to_filter, true, L"Hello", false);
	s_filter_set(&from_filter, false, L"Hello", false);

	result = s_filter_update(&to_filter, &from_filter);
	s_filter_cmp(&to_filter, &from_filter);

	ut_check_bool(result, true);

	//
	// Active differ => update
	//
	s_filter_set(&to_filter, false, L"Hello", true);
	s_filter_set(&from_filter, false, L"Hello", false);

	result = s_filter_update(&to_filter, &from_filter);
	s_filter_cmp(&to_filter, &from_filter);

	ut_check_bool(result, true);

	print_debug_str("test_filter_update() End\n");
}

/***************************************************************************
 * The function checks the search function for the s_filter struct.
 **************************************************************************/

static void test_search_str() {
	s_filter filter;
	wchar_t *result;

	print_debug_str("test_search_str() Start\n");

	s_filter_set(&filter, true, L"Hello", false);

	//
	// Case sensitive and find
	//
	result = s_filter_search_str(&filter, L"aaa Hello bbb");
	ut_check_wchar_str(result, L"Hello bbb");

	//
	// Case sensitive and don't find
	//
	result = s_filter_search_str(&filter, L"aaa hEllo bbb");
	ut_check_wchar_null(result);

	filter.case_insensitive = true;

	//
	// Case insensitive and find
	//
	result = s_filter_search_str(&filter, L"aaa hEllo bbb");
	ut_check_wchar_str(result, L"hEllo bbb");

	//
	// Case insensitive and don't find
	//
	result = s_filter_search_str(&filter, L"aaa Hell# bbb");
	ut_check_wchar_null(result);

	print_debug_str("test_search_str() End\n");
}

/***************************************************************************
 * The main function simply starts the test.
 **************************************************************************/

int main() {

	print_debug_str("ut_filter.c - Start tests\n");

	test_filter_update();

	test_search_str();

	print_debug_str("ut_filter.c - End tests\n");

	return EXIT_SUCCESS;
}
