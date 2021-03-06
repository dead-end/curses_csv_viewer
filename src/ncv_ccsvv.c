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

#include "ncv_table_header.h"

#include "ncv_ui_loop.h"
#include "ncv_parser.h"
#include "ncv_ncurses.h"
#include "ncv_common.h"

#include <string.h>
#include <unistd.h>
#include <locale.h>
#include <errno.h>
#include <signal.h>
#include <getopt.h>

/******************************************************************************
 * The table struct is defined static to be able to use it in the function
 * exit_callback().
 *****************************************************************************/

static s_table table;

/******************************************************************************
 * The function is a callback function for the signal SIGUSR1. It can be used
 * to terminate the program with a return code 0, by sending the signal
 * SIGUSR1. This is can be used as a smoke test for docker containers. Start a
 * background job that sends the SIGUSR1 after a while and start ccsvv in
 * foreground. Example:
 *
 * sh signal_sender.sh & ccsvv -d : /etc/passwd
 *****************************************************************************/

static void signal_callback(const int signo) {
	fprintf(stderr, "Received signal %d", signo);
	exit(0);
}

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
 * The function reads the csv file, either from a file or from stdin.
 *****************************************************************************/

void process_csv_file(const s_cfg_parser *cfg_parser, s_table *table) {
	FILE *file;

	if (cfg_parser->filename != NULL) {

		if ((file = fopen(cfg_parser->filename, "r")) == NULL) {
			log_exit("Unable to open file %s due to: %s", cfg_parser->filename, strerror(errno));
		}

		//
		// Do the file stream parsing.
		//
		parser_process_file(file, cfg_parser, table);

		if (fclose(file) != 0) {
			log_exit("Unable to close the file due to: %s", strerror(errno));
		}

	} else {

		//
		// Do the input stream parsing.
		//
		parser_process_file(stdin, cfg_parser, table);
	}
}

/******************************************************************************
 * The function writes the program usage. It is called with an error flag.
 * Depending on the flag the stream (stdout / stderr) is selected. The function
 * contains an optional message (not NULL) that will be written.
 *****************************************************************************/

static void print_usage(const bool has_error, const char *msg) {
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
	// Print the usage information, which are created with the man2usage.sh
	// script from the man page.
	//
	fprintf(stream, "    ccsvv - displays a csv (comma separated values) file as a table\n");
	fprintf(stream, "\n");
	fprintf(stream, "    ccsvv [OPTION]... [FILE]\n");
	fprintf(stream, "\n");
	fprintf(stream, "    The  program  is  called  with  the name of a csv FILE. If no filename is\n");
	fprintf(stream, "    given, ccsvv reads the csv data from stdin.\n");
	fprintf(stream, "\n");
	fprintf(stream, "    -c, --checks\n");
	fprintf(stream, "           By default ccsvv tries to optimize the csv data. It  adds  missing\n");
	fprintf(stream, "           fields and removes empty columns and rows at the end of the table.\n");
	fprintf(stream, "           The flag switches on strict checks. A missing field results in  an\n");
	fprintf(stream, "           error and no rows or columns are removed.\n");
	fprintf(stream, "\n");
	fprintf(stream, "    -d [delimiter], --delimiter [delimiter]\n");
	fprintf(stream, "           Defines a delimiter character, other than the default comma.\n");
	fprintf(stream, "\n");
	fprintf(stream, "    -h, --help\n");
	fprintf(stream, "           Shows a help text.\n");
	fprintf(stream, "\n");
	fprintf(stream, "    -m, --monochrom\n");
	fprintf(stream, "           By  default  ccsvv uses colors if it is supported by the terminal.\n");
	fprintf(stream, "           With this option ccsvv is forced to use a monochrom mode.\n");
	fprintf(stream, "\n");
	fprintf(stream, "    -n, --no-header | -s, --show-header\n");
	fprintf(stream, "           The flags define whether the first row of the table is interpreted\n");
	fprintf(stream, "           as  a  header for the table (-s) or not (-n). If none of the flags\n");
	fprintf(stream, "           is given ccsvv tries to detect whether a header is present or not.\n");
	fprintf(stream, "\n");
	fprintf(stream, "    -t, --trim\n");
	fprintf(stream, "           Switch off trimming of csv fields.\n");
	fprintf(stream, "\n");
	fprintf(stream, "    After ccsvv was started, you can move the cursor with the keys Up,  Down,\n");
	fprintf(stream, "    Left, Right, Page Up, Page Down, Home and End.  Addtionally the following\n");
	fprintf(stream, "    commands are supported:\n");
	fprintf(stream, "\n");
	fprintf(stream, "    ^C, ^Q Terminates the program.\n");
	fprintf(stream, "\n");
	fprintf(stream, "    ^H     Shows a help dialog.\n");
	fprintf(stream, "\n");
	fprintf(stream, "    ^F, /  Shows a filter / search dialog.\n");
	fprintf(stream, "\n");
	fprintf(stream, "    ^X     Deletes the filter / search string in the dialog.\n");
	fprintf(stream, "\n");
	fprintf(stream, "    ESC    Deletes the filter / search string and resets the table.\n");
	fprintf(stream, "\n");
	fprintf(stream, "    ^N, ^P Searches for the next (^N) / previous (^P) field that contains the\n");
	fprintf(stream, "           filter / search string.\n");
	fprintf(stream, "\n");
	fprintf(stream, "    ^S, ^R Sorts  the  table  with  the  current  column  in ascending (^S) /\n");
	fprintf(stream, "           descending (^R) order.\n");

	exit(status);
}

/******************************************************************************
 * The main function parses the command line options and starts the csv file
 * processing.
 *****************************************************************************/

int main(const int argc, char *const argv[]) {
	int c;
	bool monochrom = false;

	//
	// Create a default parser configuration.
	//
	s_cfg_parser cfg_parser = (s_cfg_parser ) { .filename = NULL, .delim = W_DELIM, .do_trim = true, .strict = false };

	//
	// Import the locale from the environment to allow proper wchar_t's.
	//
	setlocale(LC_ALL, "");

	log_debug_str("Start");

	bool detect_header = true;

	int option_index = 0;

	const struct option long_options[] =
	// @formatter:off
	        {
	  	      {"checks",      no_argument,       0, 'c'},
	  	      {"delimiter",   required_argument, 0, 'd'},
	          {"help",        no_argument,       0, 'h'},
	          {"monochrom",   no_argument,       0, 'm'},
			  {"no-header",   no_argument,       0, 'n'},
			  {"show-header", no_argument,       0, 's'},
	          {"trim",        no_argument,       0, 't'},
	          {0, 0, 0, 0}
	        };
	// @formatter:on
			//
			// Parse the command line options.
			//
	while ((c = getopt_long(argc, argv, "cd:hmnst", long_options, &option_index)) != -1) {
		switch (c) {

		case 'c':
			cfg_parser.strict = true;
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

			log_debug("Delimiter: %lc", cfg_parser.delim);
			break;

		case 'h':
			print_usage(false, NULL);
			break;

		case 'm':
			monochrom = true;
			log_debug_str("Use monochrom.");
			break;

		case 'n':
			table.show_header = false;
			detect_header = false;
			break;

		case 's':
			table.show_header = true;
			detect_header = false;
			break;

		case 't':
			cfg_parser.do_trim = false;
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
		log_debug("Found filename: %s", cfg_parser.filename);

		//
		// Ensure that no more than one filename is present.
		//
	} else if (optind != argc) {
		print_usage(true, "Unknown option found!");
	}

	//
	// Add signal handler for SIGUSR1. See the documentation of signal_callback
	// for the reason.
	//
	if (signal(SIGUSR1, signal_callback) == SIG_ERR) {
		log_exit_str("Unable to register signal function for signal: SIGUSR1");
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
		log_exit_str("Unable to register exit function!");
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

	log_debug_str("End");

	return EXIT_SUCCESS;
}
