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
#include "ncv_common.h"

#include <string.h>
#include <errno.h>

#define MAX_FIELD_SIZE 4096

/***************************************************************************
 * The struct contains the variables necessary to parse the csv file.
 **************************************************************************/

typedef struct s_csv_parser {

	//
	// The csv file is parsed twice. The first time is used to count the
	// columns and rows. After this the table structure is allocated and the
	// second time the cvs fields are copied to the newly allocated structure.
	// The flag shows whether we count or copy.
	//
	bool do_count;

	//
	// The parameter contain the current position in the csv file.
	//
	int current_row;
	int current_column;

	//
	// The parameter contains the number of columns in the first row. The
	// number of columns in all other rows have to be equal to this.
	//
	int no_columns;

	//
	// The parsed content is copied char by char to the field. The field index
	// shows the current position.
	//
	wchar_t field[MAX_FIELD_SIZE];
	int field_idx;

	//
	// The flag indicates whether the field is escaped or not. Before the first
	// char is read from the field it is unclear.
	//
	enum bool_defined is_escaped;

} s_csv_parser;

/***************************************************************************
 * The function sets default values to the members of the struct.
 **************************************************************************/

static void s_csv_parser_init(s_csv_parser *csv_parser, const bool do_count) {
	csv_parser->do_count = do_count;
	csv_parser->current_row = 0;
	csv_parser->current_column = 0;
	csv_parser->field_idx = 0;
	csv_parser->is_escaped = BOOL_UNDEF;

	if (do_count) {
		csv_parser->no_columns = 0;
	}
}

/***************************************************************************
 * The function adds a wchar to the end of the field. It ensures that no
 * buffer overflow happens.
 **************************************************************************/

static void add_wchar_to_field(s_csv_parser *csv_parser, const wchar_t wchar) {

	//
	// Ensure the size.
	//
	if (csv_parser->field_idx >= MAX_FIELD_SIZE) {
		print_exit_str("set_field() Field is too long!");
	}

	//
	// Add the wchar and increase the index.
	//
	csv_parser->field[csv_parser->field_idx++] = wchar;
}

/***************************************************************************
 * The function is called each time, the end of a field is encountered. The
 * end of a field can also be the end of a row.
 * The function updates the column and row counters. It ensures that all
 * rows so far have the same number of columns.
 **************************************************************************/

static void count_columns_and_rows(s_csv_parser *csv_parser, const bool is_row_end) {

	//
	// Count the columns in the first row.
	//
	if (csv_parser->do_count && csv_parser->current_row == 0) {
		csv_parser->no_columns++;
	}

	//
	// The function is called every time a field ends, so we update the
	// current column number. If the row ends, we need the number of columns
	// to ensure that all rows have the same number of columns, before we
	// reset the current number of columns.
	//
	csv_parser->current_column++;

	if (is_row_end) {

		//
		// If the row ends we update the current row.
		//
		csv_parser->current_row++;

		//
		// Compare the column number of the first row with that of the current.
		//
		if (csv_parser->current_row > 1) {

			if (csv_parser->no_columns != csv_parser->current_column) {
				print_exit("count_columns_and_rows() row: %d current columns: %d expected columns: %d\n", csv_parser->current_row, csv_parser->current_column, csv_parser->no_columns);
			}
		}

		//
		// Reset the column number after the check for the next row.
		//
		csv_parser->current_column = 0;
	}
}

/***************************************************************************
 * The function is called each time a field in the csv file ends. If the
 * count flag is false, the field is copied to the table.
 **************************************************************************/

static void process_column_end(s_csv_parser *csv_parser, const bool is_row_end, s_table *table) {

	//
	// If we do not count, we copy.
	//
	if (!csv_parser->do_count) {

		//
		// Add the terminating \0 to the field.
		//
		add_wchar_to_field(csv_parser, W_STR_TERM);

		//
		// Copy the field data to the table.
		//
		s_table_copy(table, csv_parser->current_row, csv_parser->current_column, csv_parser->field);

		//
		// Reset the field index.
		//
		csv_parser->field_idx = 0;
	}

	//
	// Reset the escape state.
	//
	csv_parser->is_escaped = BOOL_UNDEF;

	//
	// Update the column and row counters.
	//
	count_columns_and_rows(csv_parser, is_row_end);
}

/***************************************************************************
 * The function parses the csv file. It is called twice. The first time it
 * counts the columns and rows. And the second time it copies the csv fields
 * to the table structure.
 **************************************************************************/

static void parse_csv_file(FILE *file, const wchar_t delim, s_csv_parser *csv_parser, s_table *table) {

	//
	// The two parameters hold the current and the last char read from the
	// stream. The last char is initialized (implicitly) to \0.
	//
	wchar_t wchar_last, wchar_cur = W_STR_TERM;

	while (true) {

		wchar_last = wchar_cur;
		wchar_cur = read_wchar(file);

		if (feof(file)) {

			//
			// If we finished processing and it is still escaped, then a (final)
			// quote is missing.
			//
			if (csv_parser->is_escaped == BOOL_TRUE) {
				print_exit_str("parse_csv_file() Quote missing!\n");
			}

			//
			// If the last char was not \n we do the processing of the last
			// row. Otherwise (ending: \n<EOF>) we would do it twice.
			//
			if (wchar_last != W_NEW_LINE) {
				process_column_end(csv_parser, true, table);
			}
			break;
		}

		//
		// If the escape flag is undefined, determine its value.
		//
		if (csv_parser->is_escaped == BOOL_UNDEF) {
			if (wchar_cur == W_QUOTE) {
				csv_parser->is_escaped = BOOL_TRUE;
				continue;

			} else {
				csv_parser->is_escaped = BOOL_FLASE;
			}
		}

		//
		// case: unescaped
		//
		if (csv_parser->is_escaped == BOOL_FLASE) {

			//
			// Found: delimiter
			//
			if (wchar_cur == delim) {
				process_column_end(csv_parser, false, table);
				continue;

				//
				// Found: new line
				//
			} else if (wchar_cur == W_NEW_LINE) {
				process_column_end(csv_parser, true, table);
				continue;
			}

			//
			// case: escaped
			//
		} else if (csv_parser->is_escaped == BOOL_TRUE) {

			//
			// If we found a quote we have to read the next char to decide
			// what it means.
			//
			if (wchar_cur == W_QUOTE) {
				wchar_cur = read_wchar(file);

				//
				// Found quote followed by EOF
				//
				if (feof(file)) {
					process_column_end(csv_parser, true, table);
					break;

					//
					// Found quote followed by new line
					//
				} else if (wchar_cur == W_NEW_LINE) {
					process_column_end(csv_parser, true, table);
					continue;

					//
					// Found: quote followed by delimiter
					//
				} else if (wchar_cur == delim) {
					process_column_end(csv_parser, false, table);
					continue;

					//
					// Found quote followed by char which is not quote,
					// delimiter, new line or EOF.
					//
				} else if (wchar_cur != W_QUOTE) {
					print_exit("parse_csv_file() Invalid char after quote: %lc\n", wchar_cur);
				}
			}
		}

		//
		// If we count fields and records copying is unnecessary.
		//
		if (!csv_parser->do_count) {
			add_wchar_to_field(csv_parser, wchar_cur);
		}
	}
}

/***************************************************************************
 * The function parses the csv file twice. The fist time it determines the
 * number of rows and columns. Then the table structure allocates fields
 * according to the rows and columns. The last thing is to parse the file
 * again and copy the data to the structure.
 *
 * The second time the csv file is read for parsing the data should be in
 * the file system cache so the IO overhead should be limited.
 **************************************************************************/

void parser_process_file(FILE *file, const wchar_t delim, s_table *table) {

	s_csv_parser csv_parser;

	//
	// Parse the csv file to get the number of columns and rows.
	//
	s_csv_parser_init(&csv_parser, true);
	parse_csv_file(file, delim, &csv_parser, table);

	print_debug("parser_process_file() No rows: %d no columns: %d\n", csv_parser.current_row, csv_parser.no_columns);

	//
	// If the file is empty, there is nothing to do.
	//
	if (csv_parser.current_row == 0 && csv_parser.no_columns == 0) {
		print_exit_str("parser_process_file() File is empty!\n");
	}

	//
	// Rewind the file.
	//
	if (fseek(file, 0L, SEEK_SET) == -1) {
		print_exit("parser_process_file() Unable to rewind file due to: %s\n", strerror(errno));
	}

	//
	// Allocate memory for the number of rows and columns.
	//
	s_table_init(table, csv_parser.current_row, csv_parser.no_columns);

	//
	// Parse the csv file again to copy the fields to the table structure.
	//
	s_csv_parser_init(&csv_parser, false);
	parse_csv_file(file, delim, &csv_parser, table);

	//
	// Init the table rows and heights
	//
	s_table_reset_rows(table);
}

/***************************************************************************
 * The function is a wrapper around the parser_process_file() function, that
 * is responsible for opening and closing the file.
 **************************************************************************/

void parser_process_filename(const char *filename, const wchar_t delim, s_table *table) {

	//
	// Open the csv file.
	//
	print_debug("parser_process_filename() Reading file: %s\n", filename);

	FILE *file = fopen(filename, "r");
	if (file == NULL) {
		print_exit("parser_process_filename() Unable to open file %s due to: %s\n", filename, strerror(errno));
	}

	//
	// Delegate the processing.
	//
	parser_process_file(file, delim, table);

	//
	// Close the file.
	//
	if (fclose(file) != 0) {
		print_exit("parser_process_filename() Unable to close file %s due to: %s\n", filename, strerror(errno));
	}
}
