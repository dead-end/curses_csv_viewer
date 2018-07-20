/*
 * file: nvc_parser.c
 */

#include "ncv_common.h"
#include "ncv_table.h"

/***************************************************************************
 * The struct contains the variables necessary to parse the csv file.
 **************************************************************************/

typedef struct s_csv_parser {

	//
	// The csv file is parsed twice. The first time is used to count the columns
	// and rows. After this the table structure is allocated and the second time
	// the cvs fields are copied to the newly allocated structure. The flag
	// shows whether we count or copy.
	//
	bool do_count;

	//
	// The parameter contain the current position in the csv file.
	//
	int current_row;
	int current_column;

	//
	// The parameter contains the number of columns in the first row. The number
	// of columns in all other rows have to be equal to this.
	//
	int no_columns;

	//
	// The csv file is read line by line. The parsing is done char by char. The
	// pointer shows the current char to be parsed.
	//
	wchar_t line[MAX_LINE];
	wchar_t *ptr_line;

	//
	// The parsed content is copied char by char to the field. The field pointer
	// shows the current position.
	//
	// There are no checks for overflow in the code. This is done implicitly. The
	// line is filled with fgets which adds a \0. The line ptr checks for \0. The
	// field is by construction smaller than the line, because escape chars are
	// omitted, but both have MAX_LINE size.
	//
	wchar_t field[MAX_LINE];
	wchar_t *ptr_field;

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
	csv_parser->ptr_field = csv_parser->field;
	csv_parser->is_escaped = BOOL_UNDEF;

	if (do_count) {
		csv_parser->no_columns = 0;
	}
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
	// current column number.
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
		// Reset the column number for the next row.
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
		*csv_parser->ptr_field = W_STR_TERM;

		//
		// Copy the field data to the table.
		//
		s_table_copy(table, csv_parser->current_row, csv_parser->current_column, csv_parser->field);

		//
		// Reset the field pointer for the next field.
		//
		csv_parser->ptr_field = csv_parser->field;
	}

	//
	// Reset the line pointer.
	//
	csv_parser->ptr_line++;
	csv_parser->is_escaped = BOOL_UNDEF;

	//
	// Update the column and row counters.
	//
	count_columns_and_rows(csv_parser, is_row_end);
}

/***************************************************************************
 * The function copies the current line char to the current field char, if
 * the count flag is false.
 **************************************************************************/

static void copy_field_char(s_csv_parser *csv_parser) {

	//
	// If we count fields and records copying is unnecessary.
	//
	if (!csv_parser->do_count) {
		*csv_parser->ptr_field = *csv_parser->ptr_line;
		csv_parser->ptr_field++;
	}

	csv_parser->ptr_line++;
}

/***************************************************************************
 * The function parses the csv file. It is called twice. The first time it
 * counts the columns and rows. And the second time it copies the csv fields
 * to the table structure.
 **************************************************************************/

static void parse_csv_file(FILE *file, const wchar_t delim, s_csv_parser *csv_parser, s_table *table) {

	while (true) {

		//
		// Process csv file line by line.
		//
		if ((csv_parser->ptr_line = fgetws(csv_parser->line, MAX_LINE, file)) == NULL) {

			//
			// Check for read errors.
			//
			if (ferror(file)) {
				print_exit("parse_csv_file() Error on reading input: %s\n", strerror(errno));
			}

			print_debug_str("parse_csv_file() No more lines available!\n");
			break;
		}

		print_debug("parse_csv_file() line: '%ls'\n", csv_parser->line);

		//
		// Process the current line char by char.
		//
		while (true) {

			//
			// If the escape flag is undefined, determine its value.
			//
			if (csv_parser->is_escaped == BOOL_UNDEF) {
				if (*csv_parser->ptr_line == W_QUOTE) {
					csv_parser->is_escaped = BOOL_TRUE;
					csv_parser->ptr_line++;
					continue;

				} else {
					csv_parser->is_escaped = BOOL_FLASE;
				}
			}

			//
			// Ignore windows stuff.
			//
			if (*csv_parser->ptr_line == W_CR) {
				csv_parser->ptr_line++;
				continue;
			}

			//
			// case: unescaped
			//
			if (csv_parser->is_escaped == BOOL_FLASE) {

				//
				// Found: delimiter
				//
				if (*csv_parser->ptr_line == delim) {
					process_column_end(csv_parser, false, table);
					continue;

					//
					// Found: new line or string terminator
					//
				} else if (*csv_parser->ptr_line == W_NEW_LINE || *csv_parser->ptr_line == W_STR_TERM) {
					process_column_end(csv_parser, true, table);
					break;
				}

				//
				// case: escaped
				//
			} else if (csv_parser->is_escaped == BOOL_TRUE) {

				if (*csv_parser->ptr_line == W_QUOTE) {
					csv_parser->ptr_line++;

					//
					// Found: quote followed by delimiter
					//
					if (*csv_parser->ptr_line == delim) {
						process_column_end(csv_parser, false, table);
						continue;

						//
						// Found quote followed by new line or string terminator.
						//
					} else if (*csv_parser->ptr_line == W_NEW_LINE || *csv_parser->ptr_line == W_STR_TERM) {
						process_column_end(csv_parser, true, table);
						break;

						//
						// Found quote followed by char which is not quote,
						// delimiter, new line or string terminator.
						//
					} else if (*csv_parser->ptr_line != W_QUOTE) {
						print_exit("parse_csv_file() Invalid line: %ls\n", csv_parser->line);
					}

					//
					// Found new line inside quote, so we have a multi line field.
					//
				} else if (*csv_parser->ptr_line == W_NEW_LINE) {
					copy_field_char(csv_parser);
					break;
				}
			}

			//
			// Copy the parsing char to the field
			//
			copy_field_char(csv_parser);
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

	//
	// Parse the csv file to get the number of columns and rows.
	//
	s_csv_parser csv_parser;
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
