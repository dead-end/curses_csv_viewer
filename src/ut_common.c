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

#include <wchar.h>

/******************************************************************************
 * The function tests the wcs_casestr function.
 *****************************************************************************/

static void test_wcs_casestr() {
	wchar_t *ptr;

	print_debug_str("test_wcs_casestr() Start\n");

	//
	// No match
	//
	ptr = wcs_casestr(L"AbCdEfGh", L"123");
	ut_check_wcs_null(ptr, UT_IS_NULL);

	//
	// Partly match
	//
	ptr = wcs_casestr(L"AbCdEfGh", L"aBcDeX");
	ut_check_wcs_null(ptr, UT_IS_NULL);

	//
	// Find string is longer
	//
	ptr = wcs_casestr(L"AbCdEfGh", L"aBcDeFgHX");
	ut_check_wcs_null(ptr, UT_IS_NULL);

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

/******************************************************************************
 * The function tests the str_array_sizes function.
 *****************************************************************************/

static void test_str_array_sizes() {
	int rows, cols;
	const char *msgs[] = { "22", "", "1", "4444", "333", NULL };

	print_debug_str("test_str_array_sizes() Start\n");

	str_array_sizes(msgs, &rows, &cols);
	ut_check_int(rows, 5, "Checking rows");
	ut_check_int(cols, 4, "Checking cols");

	print_debug_str("test_str_array_sizes() End\n");
}

/******************************************************************************
 * The function tests the trimming functions. Remember that the functions trim
 * and wcstrim change to parameter, so you cannot use a literal string as an
 * argument.
 *****************************************************************************/

#define SMALL_BUF_SIZE 256

static void test_trims() {
	print_debug_str("test_trims() Start\n");

	//
	// Test trim
	//
	char chr_buf[SMALL_BUF_SIZE];
	snprintf(chr_buf, SMALL_BUF_SIZE, " \t my test \t \n \t");
	char *chr_out = trim(chr_buf);

	ut_check_char_str(chr_out, "my test");

	//
	// Test wcstrim
	//
	wchar_t wcs_buf[SMALL_BUF_SIZE];
	swprintf(wcs_buf, SMALL_BUF_SIZE, L" \t my test \t \n \t");
	wchar_t *wcs_out = wcstrim(wcs_buf);

	ut_check_wchar_str(wcs_out, L"my test");

	print_debug_str("test_trims() End\n");
}

/******************************************************************************
 * The main function simply starts the test.
 *****************************************************************************/

int main() {

	print_debug_str("ut_common.c - Start tests\n");

	test_wcs_casestr();

	test_str_array_sizes();

	test_trims();

	print_debug_str("ut_common.c - End tests\n");

	return EXIT_SUCCESS;
}
