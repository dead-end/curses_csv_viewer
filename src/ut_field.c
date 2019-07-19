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
#include "ncv_field.h"

#include <locale.h>

/******************************************************************************
 * The function checks the s_field_part_update function, which computes the
 * visible part of a truncated field.
 *****************************************************************************/

static void test_field_part_update() {

	s_table_part table_part;
	s_field_part field_part;

	print_debug_str("test_field_part_update() Start\n");

	//
	// truncated right
	//
	table_part.first = 0;
	table_part.last = 2;
	table_part.truncated = table_part.last;
	table_part.size = 2;

	s_field_part_update(&field_part, &table_part, 0, 4);
	ut_check_int(field_part.start, 0, "test 1 - start");
	ut_check_int(field_part.size, 4, "test 2 - size");

	s_field_part_update(&field_part, &table_part, 2, 4);
	ut_check_int(field_part.start, 0, "test 3 - start");
	ut_check_int(field_part.size, 2, "test 4 - size");

	//
	// truncated left
	//
	table_part.first = 0;
	table_part.last = 2;
	table_part.truncated = table_part.first;
	table_part.size = 2;

	s_field_part_update(&field_part, &table_part, 0, 4);
	ut_check_int(field_part.start, 2, "test 5 - start");
	ut_check_int(field_part.size, 2, "test 6 - size");

	s_field_part_update(&field_part, &table_part, 2, 4);
	ut_check_int(field_part.start, 0, "test 7 - start");
	ut_check_int(field_part.size, 4, "test 8 - size");

	print_debug_str("test_field_part_update() End\n");
}

/******************************************************************************
 * The function checks the get_field_complete_line function for a field. It is
 * repeatedly called and returns a line of fixed length which may be padded.
 *****************************************************************************/

#define COL_WIDTH 4

static void test_get_field_complete_line() {

	print_debug_str("test_get_field_complete_line() Start\n");

	wchar_t buffer[COL_WIDTH + 1];
	wchar_t *ptr;
	bool end = false;

	wchar_t *str1 = L"\nзаяц\nз";

	ptr = str1;

	buffer[COL_WIDTH] = W_STR_TERM;

	ptr = get_field_complete_line(ptr, buffer, COL_WIDTH, &end);
	ut_check_wchar_str(buffer, L"    ");
	ut_check_bool(end, false);

	ptr = get_field_complete_line(ptr, buffer, COL_WIDTH, &end);
	ut_check_wchar_str(buffer, L"заяц");
	ut_check_bool(end, false);

	//
	// Reached the end of the string => ptr == null
	//
	ptr = get_field_complete_line(ptr, buffer, COL_WIDTH, &end);
	ut_check_wchar_str(buffer, L"з   ");
	ut_check_bool(end, false);
	ut_check_wcs_null(ptr, UT_IS_NULL);

	ptr = get_field_complete_line(ptr, buffer, COL_WIDTH, &end);
	ut_check_wchar_str(buffer, L"    ");
	ut_check_bool(end, true);
	ut_check_wcs_null(ptr, UT_IS_NULL);

	ptr = get_field_complete_line(ptr, buffer, COL_WIDTH, &end);
	ut_check_wchar_str(buffer, L"    ");
	ut_check_bool(end, true);
	ut_check_wcs_null(ptr, UT_IS_NULL);

	print_debug_str("test_get_field_complete_line() End\n");
}

/******************************************************************************
 * The function checks the intersection function.
 *****************************************************************************/

static void test_intersection() {

	print_debug_str("test_get_field_complete_line() Start\n");

	s_buffer visible;
	s_buffer print;
	s_buffer result;

	//
	// 0123456789
	//   vvv
	//
	wchar_t *str = L"0123456789";
	visible.ptr = str + 2;
	visible.len = 3;

	//
	// 0123456789
	//   vvv
	// p
	//
	print.ptr = str;
	print.len = 1;

	intersection(&visible, &print, &result);
	ut_check_s_buffer(&result, NULL, 0, "Test 0");

	//
	// 0123456789
	//   vvv
	// pp
	//
	print.ptr = str;
	print.len = 2;

	intersection(&visible, &print, &result);
	ut_check_s_buffer(&result, NULL, 0, "Test 1");

	//
	// 0123456789
	//   vvv
	// ppp
	//
	print.ptr = str;
	print.len = 3;

	intersection(&visible, &print, &result);
	ut_check_s_buffer(&result, str + 2, 1, "Test 2");

	//
	// 0123456789
	//   vvv
	//  ppp
	//
	print.ptr = str + 1;
	print.len = 3;

	intersection(&visible, &print, &result);
	ut_check_s_buffer(&result, str + 2, 2, "Test 3");

	//
	// 0123456789
	//   vvv
	// pppppp
	//
	print.ptr = str;
	print.len = 6;

	intersection(&visible, &print, &result);
	ut_check_s_buffer(&result, str + 2, 3, "Test 4");

	//
	// 0123456789
	//   vvv
	//   ppp
	//
	print.ptr = str + 2;
	print.len = 3;

	intersection(&visible, &print, &result);
	ut_check_s_buffer(&result, str + 2, 3, "Test 5");

	//
	// 0123456789
	//   vvv
	//    p
	//
	print.ptr = str + 3;
	print.len = 1;

	intersection(&visible, &print, &result);
	ut_check_s_buffer(&result, str + 3, 1, "Test 6");

	//
	// 0123456789
	//   vvv
	//   pppp
	//
	print.ptr = str + 2;
	print.len = 4;

	intersection(&visible, &print, &result);
	ut_check_s_buffer(&result, str + 2, 3, "Test 7");

	//
	// 0123456789
	//   vvv
	//    ppp
	//
	print.ptr = str + 3;
	print.len = 3;

	intersection(&visible, &print, &result);
	ut_check_s_buffer(&result, str + 3, 2, "Test 8");

	//
	// 0123456789
	//   vvv
	//      pp
	//
	print.ptr = str + 5;
	print.len = 2;

	intersection(&visible, &print, &result);
	ut_check_s_buffer(&result, NULL, 0, "Test 8");

	//
	// 0123456789
	//   vvv
	//       pp
	//
	print.ptr = str + 6;
	print.len = 2;

	intersection(&visible, &print, &result);
	ut_check_s_buffer(&result, NULL, 0, "Test 9");

	print_debug_str("test_get_field_complete_line() End\n");
}

/******************************************************************************
 * The main function simply starts the test.
 *****************************************************************************/

int main() {

	print_debug_str("ut_field.c - Start tests\n");

	// TODO: C
	setlocale(LC_ALL, "");

	test_field_part_update();

	test_get_field_complete_line();

	test_intersection();

	print_debug_str("ut_field.c - End tests\n");

	return EXIT_SUCCESS;
}
