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

#include "ncv_table.h"
#include "ncv_parser.h"
#include "ut_utils.h"

/***************************************************************************
 * The test checks the s_table_field_dimension function, which computes the
 * width and height of a field. An empty field has width 0 and height 1. The
 * minimum width of a column is 1 to ensure that the cursor field can be
 * displayed.
 **************************************************************************/

static void test_table_field_dimension() {
	int row_size;
	int col_size;

	print_debug_str("test_table_field_dimension() Start\n");

	//
	// Test: Empty field
	//
	s_table_field_dimension(L"", &col_size, &row_size);
	ut_check_size(col_size, 0, "col: Empty field");
	ut_check_size(row_size, 1, "row: Empty field");

	//
	// Test: Two empty lines
	//
	s_table_field_dimension(L"\n", &col_size, &row_size);
	ut_check_size(col_size, 0, "col: Two empty lines");
	ut_check_size(row_size, 2, "row: Two empty lines");

	//
	// Test: Simple string
	//
	s_table_field_dimension(L"привет", &col_size, &row_size);
	ut_check_size(col_size, wcslen(L"привет"), "col: Simple string");
	ut_check_size(row_size, 1, "row: Simple string");

	//
	// Test: Multi lines
	//
	s_table_field_dimension(L"привет\nпривет привет\nпривет", &col_size, &row_size);
	ut_check_size(col_size, wcslen(L"привет привет"), "col: Multi lines");
	ut_check_size(row_size, 3, "row: Multi lines");

	print_debug_str("test_table_field_dimension() End\n");
}

/***************************************************************************
 * The function tests the get_ratio() function. It computes the ratio
 * between the digits and the string length of a given string.
 **************************************************************************/

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

/***************************************************************************
 * The function tests the characteristics (string length and ratio of digits
 * in the string) for each of the columns of a csv file.Each function call
 * returns 1 if the characteristic indicates a header for that column.
 **************************************************************************/

#define HAS_HEADER 1

#define HAS_NO_HEADER 0

static void test_table_has_header(const char *basedir) {
	s_table table;
	int result;

	print_debug_str("test_table_has_header() Start\n");

	ut_parser_process_filename(&table, basedir, "has_header.csv");

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

	// column: 3 only chars and header lenths is mean
	// len   => has no header
	// ratio => has no header
	//
	result = check_column_characteristic(&table, table.no_rows, 3, get_str_len);
	ut_check_int(result, HAS_NO_HEADER, "has_header - len: 3");

	result = check_column_characteristic(&table, table.no_rows, 3, get_ratio);
	ut_check_int(result, HAS_NO_HEADER, "has_header - ratio: 3");

	// column: 4 mixed chars and int but header is mean
	// len   => has no header
	// ratio => has no header
	//
	result = check_column_characteristic(&table, table.no_rows, 4, get_str_len);
	ut_check_int(result, HAS_NO_HEADER, "has_header - len: 4");

	result = check_column_characteristic(&table, table.no_rows, 4, get_ratio);
	ut_check_int(result, HAS_NO_HEADER, "has_header - ratio: 4");

	print_debug_str("test_table_has_header() End\n");
}

/***************************************************************************
 * The function tests the computation of the mean and the standard deviation
 * for some of the columns.
 **************************************************************************/

static void test_table_mean_std_dev(const char *basedir) {
	s_table table;
	double result;

	double tmp;

	print_debug_str("test_table_mean_std_dev() Start\n");

	ut_parser_process_filename(&table, basedir, "has_header.csv");

	//
	// column: 0 mean
	//
	tmp = (4.0 + 2 * 2) / 6;

	result = get_table_mean(&table, table.no_rows, 0, get_str_len);
	ut_check_double(result, tmp, "mean len: 0");

	result = get_table_mean(&table, table.no_rows, 0, get_ratio);
	ut_check_double(result, 1.0, "mean ratio: 0");

	//
	// column: 0 std deveation
	//
	result = get_table_std_dev(&table, table.no_rows, 0, get_str_len, tmp);
	ut_check_double(result, sqrt((4 * (1 - tmp) * (1 - tmp) + 2 * (2 - tmp) * (2 - tmp)) / 6), "std dev str: 0");

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

	print_debug_str("test_table_mean_std_dev() End\n");
}

/***************************************************************************
 * The main function simply starts the test.
 **************************************************************************/

int main(const int argc, char *argv[]) {

	char *basedir;

	print_debug_str("ut_table.c - Start tests\n");

	if (argc != 2) {
		print_exit("Usage: %s <dir>\n", argv[0]);
	}

	basedir = argv[1];

	setlocale(LC_ALL, "");

	test_table_field_dimension();

	test_table_get_ratio();

	test_table_has_header(basedir);

	test_table_mean_std_dev(basedir);

	print_debug_str("ut_table.c - End tests\n");

	return EXIT_SUCCESS;
}
