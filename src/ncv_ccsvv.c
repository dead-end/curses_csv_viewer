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

#include "ncv_win_footer.h"
#include "ncv_win_filter.h"
#include "ncv_win_header.h"
#include "ncv_win_table.h"
#include "ncv_win_help.h"

#include "ncv_ui_loop.h"
#include "ncv_parser.h"
#include "ncv_ncurses.h"
#include "ncv_common.h"

#include <string.h>
#include <unistd.h>
#include <locale.h>

/******************************************************************************
 * The table struct is defined static to be able to use it in the function
 * exit_callback().
 *****************************************************************************/

static s_table table;

/******************************************************************************
 * A cleanup function for the ncurses stuff. The important call, is the call of
 * endwin() which resets the terminal. So the freeing function before should
 * not fail.
 *****************************************************************************/

static void exit_callback() {

	//
	// Free table data
	//
	s_table_free(&table);

	//
	// Free window resources.
	//
	win_header_free();

	win_filter_free();

	win_table_free();

	win_footer_free();

	win_help_free();

	//
	// Finish ncurses
	//
	ncurses_free();
}

/******************************************************************************
 * The function reads the csv file, either from a file or from stdin. If stdin
 * is used, the content is written to a temp file for the processing. This is
 * necessary, because the file is parsed twice and you cannot rewind stdin.
 *****************************************************************************/

void process_csv_file(const s_cfg_parser *cfg_parser, s_table *table) {

	if (cfg_parser->filename != NULL) {

		//
		// If a csv file name is configured, use that file.
		//
		parser_process_filename(cfg_parser, table);

	} else {

		//
		// If no csv file name is configured, we use stdin.
		//
		FILE *tmp = stdin_2_tmp();

		parser_process_file(tmp, cfg_parser, table);

		fclose(tmp);
	}
}

/******************************************************************************
 * The function writes the program usage. It is called with an error flag.
 * Depending on the flag the stream (stdout / stderr) is selected. The function
 * contains an optional message (not NULL) that will be written.
 *****************************************************************************/

static void print_usage(const bool has_error, const char* msg) {
	FILE *stream;
	int status;

	//
	// Choose stdout / stderr depending on the error flag.
	//
	if (has_error) {
		status = EXIT_FAILURE;
		stream = stderr;
	} else {
		status = EXIT_SUCCESS;
		stream = stdout;
	}

	//
	// If the function call contains a message it is written.
	//
	if (msg != NULL) {
		if (has_error) {
			fprintf(stream, "ERROR - ");
		}
		fprintf(stream, "%s\n", msg);
	}

	//
	// Print the usage information.
	//
	//              "--------------------------------------------------------------------------------\n");
	fprintf(stream, "ccsvv [-h] [-m] [-s | -n] [-d delimiter] [file]\n\n");
	fprintf(stream, "  -h            Shows this usage message.\n\n");
	fprintf(stream, "  -d delimiter  Defines a delimiter character, other than the default comma.\n\n");
	fprintf(stream, "  -m            By default ccsvv uses colors if the terminal supports them. With\n");
	fprintf(stream, "                this option ccsvv is forced to use a monochrom mode.\n\n");
	fprintf(stream, "  -s | -n       The flags define whether the first row of the table is\n");
	fprintf(stream, "                interpreted as a header for the table ('-s') or not ('-n').\n");
	fprintf(stream, "                If none of the flags is given ccsvv tries to detect whether a\n");
	fprintf(stream, "                header is present or not.\n\n");
	fprintf(stream, "  -t            Switch off trimming of the csv fields.\n\n");
	fprintf(stream, "  file          The name of the csv file. If no filename is defined, ccsvv reads\n");
	fprintf(stream, "                the csv data from stdin.\n");
	//              "--------------------------------------------------------------------------------\n");
	fprintf(stream, "\nCommands:\n\n");
	fprintf(stream, "  ^C and ^Q     Terminate the program.\n\n");
	fprintf(stream, "  ^F            Switches to filter input mode and allows to input a filter\n");
	fprintf(stream, "                string.\n\n");
	fprintf(stream, "  ^X            In filter mode, deletes the filter string.\n\n");
	fprintf(stream, "  ESC           Delete the filter string and reset the table.\n\n");
	//              "--------------------------------------------------------------------------------\n");
	fprintf(stream, "  ^N and ^P     Search for the next / previous field that contains the filter\n");
	fprintf(stream, "                string.\n\n");

	exit(status);
}

/******************************************************************************
 * The main function parses the command line options and starts the csv file
 * processing.
 *****************************************************************************/

int main(const int argc, char * const argv[]) {
	int c;
	bool monochrom = false;

	//
	// Create a default parser configuration.
	//
	s_cfg_parser cfg_parser;
	s_cfg_parser_set(&cfg_parser, NULL, W_DELIM, true);

	//
	// Import the locale from the environment to allow proper wchar_t's.
	//
	setlocale(LC_ALL, "");

	print_debug_str("main() Start\n");

	bool detect_header = true;

	//
	// Parse the command line options.
	//
	while ((c = getopt(argc, argv, "hmsntd:")) != -1) {
		switch (c) {

		case 'h':
			print_usage(false, NULL);
			break;

		case 'm':
			monochrom = true;
			print_debug_str("main() Use monochrom.\n");
			break;

		case 's':
			table.show_header = true;
			detect_header = false;
			break;

		case 'n':
			table.show_header = false;
			detect_header = false;
			break;

		case 't':
			cfg_parser.do_trim = false;
			break;

		case 'd':

			//
			// Ensure that the delimiter consists of one character.
			//
			if (strlen(optarg) != 1) {
				print_usage(true, "Only a one character delimiter is allowed!");
			}

			//
			// Convert the char to a wide char
			//
			if (mbtowc(NULL, NULL, 0) < 0) {
				print_usage(true, "Unable to initialize the conversion to wide chars!");
			}

			if (mbtowc(&(cfg_parser.delim), optarg, MB_CUR_MAX) < 0) {
				print_usage(true, "Unable to convert the delimiter to a wide char!");
			}

			print_debug("main() Delimiter: %lc\n", cfg_parser.delim);
			break;

		default:
			print_usage(true, "Unknown option found!");
		}
	}

	//
	// Check if a file argument is present.
	//
	if (optind == argc - 1) {
		cfg_parser.filename = argv[optind];
		print_debug("main() Found filename: %s\n", cfg_parser.filename);

		//
		// Ensure that no more than one filename is present.
		//
	} else if (optind != argc) {
		print_usage(true, "Unknown option found!");
	}

	//
	// Process the csv file
	//
	process_csv_file(&cfg_parser, &table);

	//
	// Set the show_header parameter of table
	//
	if (detect_header) {
		table.show_header = s_table_has_header(&table);
	}

#ifdef DEBUG
	s_table_dump(&table);
#endif

	ncurses_init(monochrom, (cfg_parser.filename != NULL));

	//
	// Register exit callback.
	//
	if (on_exit(exit_callback, NULL) != 0) {
		print_exit_str("main() Unable to register exit function!\n");
	}

	//
	// Initialize the four windows of the application.
	//
	win_header_init();

	win_filter_init();

	win_table_init();

	win_footer_init();

	win_help_init();

	ui_loop(&table, cfg_parser.filename);

	print_debug_str("main() End\n");

	return EXIT_SUCCESS;
}
