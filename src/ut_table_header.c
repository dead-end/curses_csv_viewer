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

#include "ncv_table_header.h"
#include "ncv_parser.h"
#include "ut_utils.h"

#include <locale.h>
#include <math.h>

#define STRICT_COL_TRUE true

#define DO_TRIM_FALSE false

/******************************************************************************
 * The function tests the get_ratio() function. It computes the ratio between
 * the digits and the string length of a given string.
 *****************************************************************************/

static void test_table_get_ratio() {
	double result;

	print_debug_str("test_table_get_ratio() Start\n");

	result = get_ratio(L"123456");
	ut_check_double(result, 1.0, "get_ratio - 123456");

	result = get_ratio(L"123aaa");
	ut_check_double(result, 0.5, "get_ratio - 123aaa");

	result = get_ratio(L"11aabb");
	ut_check_double(result, 1.0 / 3.0, "get_ratio - 123aaa");

	result = get_ratio(L"aaabbb");
	ut_check_double(result, 0.0, "get_ratio - aaabbb");

	result = get_ratio(L"");
	ut_check_double(result, 0.0, "get_ratio - ''");

	print_debug_str("test_table_get_ratio() End\n");
}

/******************************************************************************
 * The function tests the characteristics (string length and ratio of digits in
 * the string) for each of the columns of a csv file. Each function call
 * returns 1 if the characteristic indicates a header for that column.
 *****************************************************************************/

#define HAS_HEADER 1

#define HAS_NO_HEADER 0

static void test_table_has_header() {
	int result;

	s_table table;
	s_table_set_defaults(table);

	print_debug_str("test_table_has_header() Start\n");

	const wchar_t *data =

	L"Number" DL "Date" DL "Price" DL "Mix" DL "Mix111" NL
	"1" DL "01.01.218" DL "1 Euro" DL "aa" DL "aa11" NL
	"2" DL "01.01.218" DL "1.20 Euro" DL "aabb" DL "aabb1122" NL
	"4" DL "01.01.218" DL "10 Euro" DL "cc" DL "cc11" NL
	"8" DL "01.01.218" DL "10.20 Euro" DL "ccdd" DL "ccdd1122" NL
	"16" DL "01.01.218" DL "100 Euro" DL "ee" DL "ee11" NL
	"32" DL "01.01.218" DL "100.20 Euro" DL "eeff" DL "eeff1122" NL;

	FILE *tmp = ut_create_tmp_file(data);

	s_cfg_parser cfg_parser;
	s_cfg_parser_set(&cfg_parser, NULL, W_DELIM, DO_TRIM_FALSE, STRICT_COL_TRUE);

	parser_process_file(tmp, &cfg_parser, &table);
	//
	// column: 0
	// len   => has header
	// ratio => has header
	//
	result = check_column_characteristic(&table, table.no_rows, 0, get_str_len);
	ut_check_int(result, HAS_HEADER, "has_header - len: 0");

	result = check_column_characteristic(&table, table.no_rows, 0, get_ratio);
	ut_check_int(result, HAS_HEADER, "has_header - ratio: 0");

	//
	// column: 1
	// len   => has header
	// ratio => has header
	//
	result = check_column_characteristic(&table, table.no_rows, 1, get_str_len);
	ut_check_int(result, HAS_HEADER, "has_header - len: 1");

	result = check_column_characteristic(&table, table.no_rows, 1, get_ratio);
	ut_check_int(result, HAS_HEADER, "has_header - ratio: 1");

	//
	// column: 2
	// len   => has header
	// ratio => has header
	//
	result = check_column_characteristic(&table, table.no_rows, 2, get_str_len);
	ut_check_int(result, HAS_NO_HEADER, "has_header - len: 2");

	result = check_column_characteristic(&table, table.no_rows, 2, get_ratio);
	ut_check_int(result, HAS_HEADER, "has_header - ratio: 2");

	//
	// column: 3 only chars and header lenths is mean
	// len   => has no header
	// ratio => has no header
	//
	result = check_column_characteristic(&table, table.no_rows, 3, get_str_len);
	ut_check_int(result, HAS_NO_HEADER, "has_header - len: 3");

	result = check_column_characteristic(&table, table.no_rows, 3, get_ratio);
	ut_check_int(result, HAS_NO_HEADER, "has_header - ratio: 3");

	//
	// column: 4 mixed chars and int but header is mean
	// len   => has no header
	// ratio => has no header
	//
	result = check_column_characteristic(&table, table.no_rows, 4, get_str_len);
	ut_check_int(result, HAS_NO_HEADER, "has_header - len: 4");

	result = check_column_characteristic(&table, table.no_rows, 4, get_ratio);
	ut_check_int(result, HAS_NO_HEADER, "has_header - ratio: 4");

	//
	// Cleanup
	//
	s_table_free(&table);

	fclose(tmp);

	print_debug_str("test_table_has_header() End\n");
}

/******************************************************************************
 * The function tests the computation of the mean and the standard deviation
 * for some of the columns.
 *****************************************************************************/

static void test_table_mean_std_dev() {
	double result;

	double d_tmp;

	s_table table;
	s_table_set_defaults(table);

	print_debug_str("test_table_mean_std_dev() Start\n");

	const wchar_t *data =

	L"Number" DL "Date" DL "Price" DL "Mix" DL "Mix111" NL
	"1" DL "01.01.218" DL "1 Euro" DL "aa" DL "aa11" NL
	"2" DL "01.01.218" DL "1.20 Euro" DL "aabb" DL "aabb1122" NL
	"4" DL "01.01.218" DL "10 Euro" DL "cc" DL "cc11" NL
	"8" DL "01.01.218" DL "10.20 Euro" DL "ccdd" DL "ccdd1122" NL
	"16" DL "01.01.218" DL "100 Euro" DL "ee" DL "ee11" NL
	"32" DL "01.01.218" DL "100.20 Euro" DL "eeff" DL "eeff1122" NL;

	FILE *tmp = ut_create_tmp_file(data);

	s_cfg_parser cfg_parser;
	s_cfg_parser_set(&cfg_parser, NULL, W_DELIM, DO_TRIM_FALSE, STRICT_COL_TRUE);

	parser_process_file(tmp, &cfg_parser, &table);

	//
	// column: 0 mean
	//
	d_tmp = (4.0 + 2 * 2) / 6;

	result = get_table_mean(&table, table.no_rows, 0, get_str_len);
	ut_check_double(result, d_tmp, "mean len: 0");

	result = get_table_mean(&table, table.no_rows, 0, get_ratio);
	ut_check_double(result, 1.0, "mean ratio: 0");

	//
	// column: 0 std deveation
	//
	result = get_table_std_dev(&table, table.no_rows, 0, get_str_len, d_tmp);
	ut_check_double(result, sqrt((4 * (1 - d_tmp) * (1 - d_tmp) + 2 * (2 - d_tmp) * (2 - d_tmp)) / 6), "std dev str: 0");

	result = get_table_std_dev(&table, table.no_rows, 0, get_ratio, 1.0);
	ut_check_double(result, 0.0, "std dev ratio: 0");

	print_debug_str("test_table_mean_std_dev() End\n");

	//
	// column: 3 mean
	//
	result = get_table_mean(&table, table.no_rows, 3, get_str_len);
	ut_check_double(result, 3.0, "mean len: 3");

	result = get_table_mean(&table, table.no_rows, 3, get_ratio);
	ut_check_double(result, 0.0, "mean ratio: 3");

	//
	// column: 3 std deveation
	//
	result = get_table_std_dev(&table, table.no_rows, 3, get_str_len, 3.0);
	ut_check_double(result, 1.0, "std dev str: 3");

	result = get_table_std_dev(&table, table.no_rows, 3, get_ratio, 0.0);
	ut_check_double(result, 0.0, "std dev ratio: 3");

	//
	// column: 4 mean
	//
	result = get_table_mean(&table, table.no_rows, 4, get_str_len);
	ut_check_double(result, 6.0, "mean len: 4");

	result = get_table_mean(&table, table.no_rows, 4, get_ratio);
	ut_check_double(result, 0.5, "mean ratio: 4");

	//
	// column: 4 std deveation
	//
	result = get_table_std_dev(&table, table.no_rows, 4, get_str_len, 6.0);
	ut_check_double(result, 2.0, "std dev str: 4");

	result = get_table_std_dev(&table, table.no_rows, 4, get_ratio, 0.5);
	ut_check_double(result, 0.0, "std dev ratio: 4");

	//
	// Cleanup
	//
	s_table_free(&table);

	fclose(tmp);

	print_debug_str("test_table_mean_std_dev() End\n");
}

/******************************************************************************
 * The main function simply starts the test.
 *****************************************************************************/

int main() {

	print_debug_str("ut_table_header.c - Start tests\n");

	setlocale(LC_ALL, "");

	test_table_get_ratio();

	test_table_has_header();

	test_table_mean_std_dev();

	print_debug_str("ut_table_header.c - End tests\n");

	return EXIT_SUCCESS;
}
