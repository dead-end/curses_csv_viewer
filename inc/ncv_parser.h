/*
 * file: ncv_parser.h
 */

#ifndef INC_NCV_PARSER_H_
#define INC_NCV_PARSER_H_

void parser_process_filename(const char *filename, const wchar_t delim, s_table *table);

void parser_process_file(FILE *file, const wchar_t delim, s_table *table);

#endif /* INC_NCV_PARSER_H_ */
