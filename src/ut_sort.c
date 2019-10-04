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

#include "ncv_sort.h"
#include "ut_utils.h"

#include <locale.h>

/******************************************************************************
 * The function checks the s_sort_update() function, which returns true if the
 * struct changed.
 *****************************************************************************/

static void test_sort_update() {
	s_sort sort;

	log_debug_str("Start");

	//
	// FORWARD FORWARD
	//
	s_sort_set_inactive(&sort);
	ut_check_bool(s_sort_update(&sort, 0, E_DIR_FORWARD), true);
	ut_check_bool(s_sort_update(&sort, 0, E_DIR_FORWARD), false);

	//
	// BACKWARD BACKWARD
	//
	s_sort_set_inactive(&sort);
	ut_check_bool(s_sort_update(&sort, 0, E_DIR_BACKWARD), true);
	ut_check_bool(s_sort_update(&sort, 0, E_DIR_BACKWARD), false);

	//
	// Change all
	//
	s_sort_set_inactive(&sort);
	ut_check_bool(s_sort_update(&sort, 0, E_DIR_FORWARD), true);
	ut_check_bool(s_sort_update(&sort, 0, E_DIR_BACKWARD), true);
	ut_check_bool(s_sort_update(&sort, 1, E_DIR_BACKWARD), true);
	ut_check_bool(s_sort_update(&sort, 1, E_DIR_FORWARD), true);

	log_debug_str("End");
}

/******************************************************************************
 * The main function simply starts the test.
 *****************************************************************************/

int main() {

	log_debug_str("Start");

	test_sort_update();

	log_debug_str("End");

	return EXIT_SUCCESS;
}
