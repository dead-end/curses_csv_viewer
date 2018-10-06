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
#include "ncv_common.h"

/***************************************************************************
 * The function tests the wcs_casestr function.
 **************************************************************************/

static void test_wcs_casestr() {
	wchar_t *ptr;

	print_debug_str("test_wcs_casestr() Start\n");

	//
	// No match
	//
	ptr = wcs_casestr(L"AbCdEfGh", L"123");
	ut_check_wchar_null(ptr);

	//
	// Partly match
	//
	ptr = wcs_casestr(L"AbCdEfGh", L"aBcDeX");
	ut_check_wchar_null(ptr);

	//
	// Find string is longer
	//
	ptr = wcs_casestr(L"AbCdEfGh", L"aBcDeFgHX");
	ut_check_wchar_null(ptr);

	//
	// Match start
	//
	ptr = wcs_casestr(L"aAabBb", L"AaA");
	ut_check_wchar_str(ptr, L"aAabBb");

	//
	// Match end
	//
	ptr = wcs_casestr(L"aAabBb", L"BbB");
	ut_check_wchar_str(ptr, L"bBb");

	//
	// Match middle
	//
	ptr = wcs_casestr(L"aAbBcC", L"Bb");
	ut_check_wchar_str(ptr, L"bBcC");

	print_debug_str("test_wcs_casestr() End\n");
}

/***************************************************************************
 * The main function simply starts the test.
 **************************************************************************/

int main() {

	print_debug_str("ut_common.c - Start tests\n");

	test_wcs_casestr();

	print_debug_str("ut_common.c - End tests\n");

	return EXIT_SUCCESS;
}
