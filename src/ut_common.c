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
#include <string.h>
#include <locale.h>

/******************************************************************************
 * The function tests the wcs_casestr function.
 *****************************************************************************/

static void test_wcs_casestr() {
	wchar_t *ptr;

	log_debug_str("Start");

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

	log_debug_str("End");
}

/******************************************************************************
 * The function tests the str_array_sizes function.
 *****************************************************************************/

static void test_str_array_sizes() {
	int rows, cols;
	const char *msgs[] = { "22", "", "1", "4444", "333", NULL };

	log_debug_str("Start");

	str_array_sizes(msgs, &rows, &cols);
	ut_check_int(rows, 5, "Checking rows");
	ut_check_int(cols, 4, "Checking cols");

	log_debug_str("End");
}

/******************************************************************************
 * The function tests the trimming functions. Remember that the functions trim
 * and wcstrim change to parameter, so you cannot use a literal string as an
 * argument.
 *****************************************************************************/

#define SMALL_BUF_SIZE 256

static void test_trims() {
	log_debug_str("Start");

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

	log_debug_str("End");
}

/******************************************************************************
 * The function tests tge convertion from a multi byte string to a wide
 * character string. (Setting the locale to "C" does not work!)
 *****************************************************************************/

#define WCHAR_BUF_SIZE 5

#define WCHAR_STR_LEN WCHAR_BUF_SIZE -1

static void test_mbs_2_wchars() {
	log_debug_str("Start");

	wchar_t buffer[WCHAR_BUF_SIZE];
	size_t size;

	//
	// Test: abcd
	//
	size = mbs_2_wchars("abcd", buffer, WCHAR_BUF_SIZE);
	ut_check_int((int) size, WCHAR_STR_LEN, "mbs_2_wchars: abcd");
	ut_check_wchar_str(buffer, L"abcd");

	//
	// Test: äöüß
	//
	size = mbs_2_wchars("äöüß", buffer, WCHAR_BUF_SIZE);
	ut_check_int((int) size, WCHAR_STR_LEN, "mbs_2_wchars: äöüß");
	ut_check_wchar_str(buffer, L"äöüß");

	//
	// Test: заяц
	//
	size = mbs_2_wchars("заяц", buffer, WCHAR_BUF_SIZE);
	ut_check_int((int) size, WCHAR_STR_LEN, "mbs_2_wchars: заяц");
	ut_check_wchar_str(buffer, L"заяц");

	log_debug_str("End");
}

/******************************************************************************
 * The main function simply starts the test.
 *****************************************************************************/

int main() {

	log_debug_str("Start");

	//
	// Setting the locale to "C" does not work with: mbs_2_wchars()
	//
	setlocale(LC_ALL, "");

	test_wcs_casestr();

	test_str_array_sizes();

	test_trims();

	test_mbs_2_wchars();

	log_debug_str("End");

	return EXIT_SUCCESS;
}
