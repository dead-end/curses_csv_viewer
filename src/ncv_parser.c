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

#include "ncv_wbuf.h"
#include "ncv_parser.h"
#include "ncv_table.h"
#include "ncv_common.h"

#include <string.h>
#include <errno.h>

#define MAX_FIELD_SIZE 4096

/******************************************************************************
 * The struct contains the variables necessary to parse the csv file.
 *****************************************************************************/

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
	// The parameter contains the number of rows and columns.
	//
	int no_rows;
	int no_columns;

	//
	// The flag indicates whether the field is escaped or not. Before the first
	// char is read from the field it is unclear.
	//
	enum bool_defined is_escaped;

	//
	// The parsed content is copied char by char to the field. The field index
	// shows the current position.
	//
	wchar_t field[MAX_FIELD_SIZE];
	int field_idx;

} s_csv_parser;

/******************************************************************************
 * The marco resets the parser field
 *****************************************************************************/

#define parser_field_reset(c) c->field_idx = 0

/******************************************************************************
 * The function adds a wchar to the end of the field and ensures that no buffer
 * overflow happens.
 *****************************************************************************/

static void parser_field_add_wchar(s_csv_parser *csv_parser, const wchar_t wchar) {

	//
	// Ensure the size.
	//
	if (csv_parser->field_idx >= MAX_FIELD_SIZE) {
		log_exit_str("Field is too long!");
	}

	//
	// Add the wchar and increase the index.
	//
	csv_parser->field[csv_parser->field_idx++] = wchar;
}

/******************************************************************************
 * The function builds the string, which is stored in the parser struct. It
 * does a trimming, if configured.
 *****************************************************************************/

static wchar_t* parser_field_get_str(s_csv_parser *csv_parser, const s_cfg_parser *cfg_parser) {

	//
	// Add the terminating \0 to the field.
	//
	parser_field_add_wchar(csv_parser, W_STR_TERM);

	//
	// If configured, trim the field content.
	//
	if (cfg_parser->do_trim) {
		return wcstrim(csv_parser->field);

	} else {
		return csv_parser->field;
	}
}

/******************************************************************************
 * The function sets default values to the members of the struct.
 *****************************************************************************/

static void s_csv_parser_init(s_csv_parser *csv_parser, const bool do_count) {

	csv_parser->do_count = do_count;
	csv_parser->is_escaped = BOOL_UNDEF;

	parser_field_reset(csv_parser);

	csv_parser->current_row = 0;
	csv_parser->current_column = 0;

	//
	// In the do_count phase, the number of rows and columns is computed, so we
	// have to ensure that the data is not overwritten in the second phase.
	//
	if (do_count) {
		csv_parser->no_columns = 0;
		csv_parser->no_rows = 0;
	}
}

/******************************************************************************
 * The function resets the struct to the next field. This can be the next field
 * in the current row, or the first field in the next row.
 *****************************************************************************/

static void s_csv_parser_next_field(s_csv_parser *csv_parser, const bool is_row_end) {

	csv_parser->is_escaped = BOOL_UNDEF;

	parser_field_reset(csv_parser);

	//
	// Update the column and row counters.
	//
	if (is_row_end) {
		csv_parser->current_row++;
		csv_parser->current_column = 0;

	} else {
		csv_parser->current_column++;
	}
}

/******************************************************************************
 * The function updates the number of rows and columns in the non strict mode.
 * It is looking for the maximum row / column of non empty fields.
 *****************************************************************************/

static void update_no_rows_cols(s_csv_parser *csv_parser, const s_cfg_parser *cfg_parser) {

#ifdef DEBUG

	//
	// Strict mode check should have happened upfront.
	//
	if (cfg_parser->strict) {
		log_exit_str("Called in strict mode!");
	}
#endif

	const wchar_t *ptr = parser_field_get_str(csv_parser, cfg_parser);

	if (!wcs_is_empty(ptr)) {

		if (csv_parser->no_columns < csv_parser->current_column + 1) {
			csv_parser->no_columns = csv_parser->current_column + 1;
		}

		if (csv_parser->no_rows < csv_parser->current_row + 1) {
			csv_parser->no_rows = csv_parser->current_row + 1;
		}
	}
}

/******************************************************************************
 * The function updates the number of rows and columns in the strict mode. It
 * counts the columns of the first row and compares it to the rest of the rows.
 *****************************************************************************/

static void update_no_rows_cols_strict(s_csv_parser *csv_parser, const bool is_row_end) {

	//
	// In the first row count the columns
	//
	if (csv_parser->current_row == 0) {
		csv_parser->no_columns++;

		//
		// Increment the number of rows if necessary
		//
		if (is_row_end) {
			csv_parser->no_rows++;
		}

	} else if (is_row_end) {

		//
		// Here we are at the end of a row, other than the first row. We
		// compare the column number of the first row with that of the current.
		//
		if (csv_parser->current_column != csv_parser->no_columns - 1) {

			// @formatter:off
			log_exit("Row: %d current columns: %d expected columns: %d",
					csv_parser->current_row + 1,
					csv_parser->current_column,
					csv_parser->no_columns);
			// @formatter:on
		}

		//
		// We are at the end of the row, so always increment the number of
		// rows.
		//
		csv_parser->no_rows++;
	}
}

/******************************************************************************
 * The macro checks if the current fields row / column is inside the allowed
 * range.
 *****************************************************************************/

#define field_is_inside_range(c) ((c)->current_column < (c)->no_columns && (c)->current_row < (c)->no_rows)

/******************************************************************************
 * The function checks if fields are missing in a row and adds empty fields if
 * necessary.
 *****************************************************************************/

static void add_missing_fields(s_csv_parser *csv_parser, const s_cfg_parser *cfg_parser, s_table *table) {

	//
	// In strict mode we do not add fields.
	//
	if (cfg_parser->strict) {
		return;
	}

	//
	// Add missing fields until the current column is the last column
	//
	while (++csv_parser->current_column < csv_parser->no_columns) {

		//
		// Add the missing field
		//
		s_table_copy(table, csv_parser->current_row, csv_parser->current_column, L"");

		// @formatter:off
		log_debug("Missing column: %d of total %d in row: %d",
				csv_parser->current_column + 1,
				csv_parser->no_columns,
				csv_parser->current_row + 1);
		// @formatter:on
	}
}

/******************************************************************************
 * The function is called each time a field in the csv file ends. The flag
 * is_row_end is set if the field is the last in the row.
 *
 * The parsing has two phases. In the do_count phase the number of rows and
 * columns is determined. In the second phase the csv data is copied to the
 * s_table struct. If the mode is not strict, missing fields are added and
 * empty rows and columns at the end are removed.
 *
 * Example input with spaces:
 *
 * "1, , , , , , "
 * "2,2"
 * "3,3,3"
 * " , , , , "
 *
 * Output with spaces:
 *
 * "1, , "
 * "2,2, "
 * "3,3,3"
 *****************************************************************************/

static void process_column_end(s_csv_parser *csv_parser, const s_cfg_parser *cfg_parser, const bool is_row_end, s_table *table) {

	//
	// In the "do_count" phase, we try to determine no_rows and no_columns.
	//
	if (csv_parser->do_count) {

		if (cfg_parser->strict) {
			update_no_rows_cols_strict(csv_parser, is_row_end);

		} else {
			update_no_rows_cols(csv_parser, cfg_parser);
		}
	}

	//
	// If we do not count, we copy.
	//
	else {

		//
		// In strict mode, this is always true.
		//
		if (field_is_inside_range(csv_parser)) {

			//
			// Copy the field data to the table.
			//
			wchar_t *ptr = parser_field_get_str(csv_parser, cfg_parser);
			s_table_copy(table, csv_parser->current_row, csv_parser->current_column, ptr);

			//
			// If we found the end of the row, we add missing columns.
			//
			if (is_row_end) {
				add_missing_fields(csv_parser, cfg_parser, table);
			}
		}
	}

	s_csv_parser_next_field(csv_parser, is_row_end);
}

/******************************************************************************
 * The function parses a s_wbuf. It is called twice. The first time it counts
 * the columns and rows. And the second time it copies the csv fields to the
 * table structure.
 *****************************************************************************/

static void parse_csv_wbuf(s_wbuf *wbuf, const s_cfg_parser *cfg_parser, s_csv_parser *csv_parser, s_table *table) {

	//
	// The two parameters hold the current and the last char read from the
	// stream. The last char is initialized (implicitly) to \0.
	//
	wchar_t wchar_last, wchar_cur = W_STR_TERM;

	//
	// Provide a s_wbuf position and initialize it.
	//
	s_wbuf_pos cur_pos;
	s_wbuf_pos_init(&cur_pos);

	while (true) {

		wchar_last = wchar_cur;

		if (!s_wbuf_next(wbuf, &cur_pos, &wchar_cur)) {

			//
			// If we finished processing and it is still escaped, then a
			// (final) quote is missing.
			//
			if (csv_parser->is_escaped == BOOL_TRUE) {
				log_exit_str("Quote missing!");
			}

			//
			// If the last char was not \n we do the processing of the last
			// row. Otherwise (ending: \n<EOF>) we would do it twice.
			//
			if (wchar_last != W_NEW_LINE) {
				process_column_end(csv_parser, cfg_parser, true, table);
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
			if (wchar_cur == cfg_parser->delim) {
				process_column_end(csv_parser, cfg_parser, false, table);
				continue;

				//
				// Found: new line
				//
			} else if (wchar_cur == W_NEW_LINE) {
				process_column_end(csv_parser, cfg_parser, true, table);
				continue;
			}

			//
			// case: escaped
			//
		} else if (csv_parser->is_escaped == BOOL_TRUE) {

			//
			// If we found a quote we have to read the next char to decide what
			// it means.
			//
			if (wchar_cur == W_QUOTE) {

				//
				// Found quote followed by EOF
				//
				if (!s_wbuf_next(wbuf, &cur_pos, &wchar_cur)) {
					process_column_end(csv_parser, cfg_parser, true, table);
					break;

					//
					// Found quote followed by new line
					//
				} else if (wchar_cur == W_NEW_LINE) {
					process_column_end(csv_parser, cfg_parser, true, table);
					continue;

					//
					// Found: quote followed by delimiter
					//
				} else if (wchar_cur == cfg_parser->delim) {
					process_column_end(csv_parser, cfg_parser, false, table);
					continue;

					//
					// Found quote followed by char which is not quote,
					// delimiter, new line or EOF.
					//
				} else if (wchar_cur != W_QUOTE) {
					log_exit("Invalid char after quote: %lc", wchar_cur);
				}
			}
		}

		//
		// Add the current wchar to the field. This is necessary even in the
		// do_count phase.
		//
		parser_field_add_wchar(csv_parser, wchar_cur);
	}
}

/******************************************************************************
 * The function parses the csv file twice. The fist time it determines the
 * number of rows and columns. Then the table structure allocates fields
 * according to the rows and columns. The last thing is to parse the file again
 * and copy the data to the structure.
 *****************************************************************************/

void parser_process_file(FILE *file, const s_cfg_parser *cfg_parser, s_table *table) {

	//
	// Create a s_wbuf with the content of the file
	//
	s_wbuf *wbuf = s_wbuf_create(WBUF_BLOCK_SIZE);
	s_wbuf_copy_file(file, wbuf);

	s_csv_parser csv_parser;

	//
	// Parse the csv file to get the number of columns and rows.
	//
	s_csv_parser_init(&csv_parser, true);
	parse_csv_wbuf(wbuf, cfg_parser, &csv_parser, table);

	log_debug("No rows: %d no columns: %d", csv_parser.no_rows, csv_parser.no_columns);

	//
	// Allocate memory for the number of rows and columns.
	//
	s_table_init(table, csv_parser.no_rows, csv_parser.no_columns);

	//
	// Parse the csv file again to copy the fields to the table structure.
	//
	s_csv_parser_init(&csv_parser, false);
	parse_csv_wbuf(wbuf, cfg_parser, &csv_parser, table);

	//
	// Init the table rows and heights
	//
	s_table_reset_rows(table);

	//
	// Free the allocated s_wbuf
	//
	s_wbuf_free(wbuf);
}

