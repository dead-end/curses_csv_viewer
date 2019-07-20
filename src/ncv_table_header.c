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

#include <math.h>
#include <wctype.h>

/******************************************************************************
 * The parameter defines the number of rows that are inspected to determine
 * whether a column has a header row. If the csv file has 30.000 you do not
 * want to look at all of them.
 *****************************************************************************/

#define HA_MAX_ROWS 64

/******************************************************************************
 * The parameter is used to decide whether the first row is an header. For the
 * string length, the criteria is, that the difference to the mean is greater
 * then HA_MAX_STD_DEV times the standard deviation.
 *****************************************************************************/

#define HA_MAX_STD_DEV 3.0

/******************************************************************************
 * The parameter defines how many times a criteria for a column has to be
 * fulfilled to indicate a header.
 *****************************************************************************/

#define HA_MAX_SUCCESSFUL 3

/******************************************************************************
 * The function computes the ratio of digits in a string with the string
 * length. If the function is called with an empty string the result would be a
 * division 0/0. In this case 0 is returned, which can be interpreted as the
 * string contains no digits (which is true :o)
 *****************************************************************************/

double get_ratio(const wchar_t *str) {
	const size_t len = wcslen(str);

	if (len == 0) {
		return 0;
	}

	//
	// count has to be double to ensure a proper cast at return.
	//
	double count = 0;

	for (; *str != W_STR_TERM; str++) {
		if (iswdigit(*str)) {
			count += 1;
		}
	}

	return count / len;
}

/******************************************************************************
 * The function is a wrapper around the wcslen function. It allows to be used
 * with the function pointer:
 *
 * double (*fct_ptr)(const wchar_t *str)
 *****************************************************************************/

double get_str_len(const wchar_t *str) {
	return wcslen(str);
}

/******************************************************************************
 * The function computes the mean value of a characteristic for a column. The
 * function pointer computes a double value that represents the characteristic
 * for a field.
 *****************************************************************************/

double get_table_mean(const s_table *table, const int max_rows, const int column, double (*fct_ptr)(const wchar_t *str)) {
	double mean = 0;

	for (int row = 1; row < max_rows; row++) {
		mean += (*fct_ptr)(table->__fields[row][column]);
	}
	mean /= max_rows - 1;

	return mean;
}

/******************************************************************************
 * The function computes the standard deviation of a characteristic for a
 * column. The function pointer computes a double value that represents the
 * characteristic for a field.
 *****************************************************************************/

double get_table_std_dev(const s_table *table, const int max_rows, const int column, double (*fct_ptr)(const wchar_t *str), const double mean) {
	double std_dev = 0;
	double tmp;

	for (int row = 1; row < max_rows; row++) {
		tmp = (*fct_ptr)(table->__fields[row][column]);
		std_dev += pow2(mean - tmp);
	}
	std_dev = sqrt(std_dev / (max_rows - 1));

	return std_dev;
}

/******************************************************************************
 * The function compares a characteristic of the first row of a column with the
 * rest of the rows of that column.
 *****************************************************************************/

int check_column_characteristic(const s_table *table, const int max_rows, const int column, double (*fct_ptr)(const wchar_t *str)) {

	//
	// compute the mean
	//
	const double mean = get_table_mean(table, max_rows, column, fct_ptr);

	//
	// compute the standard deviation
	//
	const double std_dev = get_table_std_dev(table, max_rows, column, fct_ptr, mean);

	//
	// compute the first row
	//
	const double first = (*fct_ptr)(table->__fields[0][column]);

	log_debug("Col: %d first: '%ls'", column, table->__fields[0][column]);
	log_debug("Mean: %lf stddev: %lf first: %lf", mean, std_dev, first);

	//
	// compare the first row with the mean
	//
	if (fabs(first - mean) > HA_MAX_STD_DEV * std_dev) {
		return 1;
	}

	return 0;
}

/******************************************************************************
 * The function checks the first HA_MAX_ROWS rows to decide whether the table
 * has a header or not.
 *****************************************************************************/

bool s_table_has_header(const s_table *table) {

	//
	// Ensure that there are enough rows to analyze
	//
	if (table->__no_rows <= 2) {
		return true;
	}

	//
	// Ensure that the table has enough columns to be successful.
	//
	const int max_successful = table->no_columns > HA_MAX_SUCCESSFUL ? HA_MAX_SUCCESSFUL : table->no_columns;

	//
	// Check at most the first HA_MAX_ROWS rows.
	//
	const int max_rows = table->__no_rows > HA_MAX_ROWS ? HA_MAX_ROWS : table->__no_rows;

	int no_sucessful_checks = 0;

	for (int column = 0; column < table->no_columns; column++) {

		//
		// Check the string lengths
		//
		no_sucessful_checks += check_column_characteristic(table, max_rows, column, get_str_len);
		if (no_sucessful_checks >= max_successful) {
			return true;
		}

		//
		// Check the ratio between the digits and the string lengths.
		//
		no_sucessful_checks += check_column_characteristic(table, max_rows, column, get_ratio);
		if (no_sucessful_checks >= max_successful) {
			return true;
		}
	}

	return false;
}
