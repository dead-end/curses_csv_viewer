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

#ifndef INC_NCV_PARSER_H_
#define INC_NCV_PARSER_H_

#include "ncv_table.h"

/******************************************************************************
 * The struct contains the configuration of the parser.
 *****************************************************************************/

typedef struct s_cfg_parser {

	//
	// The csv filename, which is null if the csv data comes from stdin.
	//
	char *filename;

	//
	// The definition of the field delimiter of the csv file.
	//
	wchar_t delim;

	//
	// A flag that whether a trimming should be performed on the fields.
	//
	bool do_trim;

	//
	// A flag that indicates a strict mode. In a non strict mode, missing
	// fields are added and empty rows / columns at the end of the table are
	// removed.
	//
	bool strict;

} s_cfg_parser;

void parser_process_file(FILE *file, const s_cfg_parser *cfg_parser, s_table *table);

#endif
