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

#include "ncv_field.h"

/***************************************************************************
 * The method updates the row /column field part for a given field.
 *
 * The table part of the row / column is used to determine whether the field
 * row / column is truncated and if so the truncated size of the field.
 *
 * The index is the index of the row / column of the field.
 **************************************************************************/

void s_field_part_update(s_field_part *field_part, const s_table_part *table_part, const int index, const int size) {

	//
	// If the table has a truncated row / column, the table_part struc's
	// member "truncated" has its index. If no row / column is truncated
	// the "truncated" member is -1.
	//
	if (index == table_part->truncated) {

		//
		// If the row is truncated the first or the last lines are skipped.
		// If the column is truncated the first or the last chars of each
		// line of the field is skipped.
		//
		// If the table part last is truncated, the last lines / chars are
		// skipped. If the table part first is truncated, the first lines
		// chars are skipped.
		//
		// Special case one truncated field => first == last
		//
		if (table_part->first != table_part->last && table_part->truncated == table_part->first) {
			field_part->start = size - table_part->size;
		} else {
			field_part->start = 0;
		}

		//
		// The field size is the truncated size.
		//
		field_part->size = table_part->size;

	} else {

		//
		// If the field is not truncated, the size is row height or the
		// column width.
		//
		field_part->start = 0;
		field_part->size = size;
	}

	print_debug("s_field_part_update() index: %d truncated: %d first: %d last: %d\n", index, table_part->truncated, table_part->first, table_part->last);print_debug("s_field_part_update() start: %d size: %d\n", field_part->start, field_part->size);
}

/***************************************************************************
 * The function is repeatedly called with a pointer to a field string and
 * copies the chars of the current field line to the buffer. The buffer has
 * a fixed size, which is: column-width + 1 and will be padded with spaces,
 * if the line length is less than the column width. If the end of the
 * string is reached, the buffer contains only spaces. The function updates
 * the pointer to the start of the next line of NULL is no more lines are
 * present.
 **************************************************************************/

wchar_t *get_field_complete_line(wchar_t *str_ptr, wchar_t *buffer, const int width, bool *end) {

	//
	// The bool value is used to avoid unnecessary calls of wmemset. If the
	// pointer is null, wmemset has to be called only once.
	//
	if (*end) {
		return NULL;
	}

	//
	// Initialize the buffer with spaces. This is not necessary if the field
	// ends with more than one empty line. In this case the buffer should be
	// unchanged.
	//
	wmemset(buffer, L' ', width);

	//
	// If the str pointer is null the function returns. In this case the
	// buffer contains only spaces from the initialization.
	//
	if (str_ptr == NULL) {

		//
		// Pointer is null and wmemset was already called, so we reached the
		// end.
		//
		if (!(*end)) {
			*end = true;
		}

		print_debug_str("get_field_complete_line() Function called with null string\n");
		return NULL;
	}

	wchar_t *buf_ptr = buffer;

	for (int i = 0; i < width; i++) {

		//
		// On \0 or \n the line is finished. Here the string is smaller than width.
		//
		if (*str_ptr == W_STR_TERM || *str_ptr == W_NEW_LINE) {
			break;
		}

		//
		// Copy chars
		//
		*buf_ptr = *str_ptr;
		buf_ptr++;
		str_ptr++;
	}

	//
	// If the string has size width, we reach this point without the break, so
	// we have to check for the proper end.
	//
	// If the str pointer is \0 we return NULL to indicate the end.
	//
	if (*str_ptr == W_STR_TERM) {
		return NULL;
	}

	//
	// If the str pointer is \n, more lines will follow and we set the
	// pointer to the next line.
	//
	if (*str_ptr == W_NEW_LINE) {
		return ++str_ptr;
	}

	//
	// The width is the maximum width of the column, so this should not happen.
	//
	print_exit("get_field_complete_line() String is too long: %ls\n", str_ptr);
}

/***************************************************************************
 * We have a buffer (a line of a field). The buffer has a visible part,
 * defined by the s_buffer visible. We want to print some char, defined by
 * the s_buffer print. We compute the intersection of the two s_buffers. The
 * s_buffer result contains the visible part of the print s_buffer. If
 * nothing id visible, the s_buffer contains NULL and 0.
 **************************************************************************/

void intersection(const s_buffer *visible, const s_buffer *print, s_buffer *result) {

	//
	// If the print buffer ends before the start of the visible buffer, or
	// the print buffer starts after the end of the visible buffer, there is
	// nothing visible.
	//
	if (s_buffer_end(print) <= s_buffer_start(visible) || s_buffer_end(visible) <= s_buffer_start(print)) {
		result->ptr = NULL;
		result->len = 0;
		return;
	}

	//
	// At this point something has to visible. It starts at the maximum of
	// the visible and the print buffer.
	//
	result->ptr = max_or_equal(s_buffer_start(visible), s_buffer_start(print));

	//
	// The visible part of the print buffer ends at the minimum of the end of
	// the visible and the print buffer.
	//
	result->len = min_or_equal(s_buffer_end(visible), s_buffer_end(print)) - result->ptr;
}

/***************************************************************************
 * The function is called with a line of a field, the visible part of the
 * field and a filter string. It prints the visible part of the line, where
 * the filter string is highlighted. So the function searches multiple times
 * through the line until the filter string is not found any more.
 **************************************************************************/

void print_line(WINDOW *win, const int win_y, int win_x, wchar_t *field_line, s_buffer *visible, const s_buffer *filter, const s_attr *attr_cur) {

	//
	// The part of the line that should be printed.
	//
	s_buffer print;

	//
	// The part of the printed substring that is visbible.
	//
	s_buffer result;

	wchar_t *cur = field_line;
	wchar_t *ptr;

	while (*cur != W_STR_TERM) {

		//
		// Search the filter string from the current position.
		//
		ptr = wcsstr(cur, filter->ptr);

		//
		// If the search string was not found. Print the rest of the line.
		//
		if (ptr == NULL) {

			//
			// Set the print to the current position and the rest of the
			// buffer.
			// And compute the visible part of it.
			//
			s_buffer_set(&print, cur, visible->len - (cur - visible->ptr));
			intersection(visible, &print, &result);

			//
			// Ensure that the string to print is visible
			//
			if (result.ptr != NULL) {
				mvwaddnwstr(win, win_y, win_x, result.ptr, result.len);
			}

			break;
		}

		//
		// If the line does not start with the search string, print the
		// substring from the current position up to the search string.
		//
		if (ptr > cur) {

			//
			// Set the print to the current position and the beginning
			// of the search found.
			// And compute the visible part of it.
			//
			s_buffer_set(&print, cur, ptr - cur);
			intersection(visible, &print, &result);

			//
			// Ensure that the string to print is visible
			//
			if (result.ptr != NULL) {
				mvwaddnwstr(win, win_y, win_x, result.ptr, result.len);

				//
				// Update the column position in the window.
				//
				win_x += result.len;
			}
		}

		//
		// Set the print to the search result.
		// And compute the visible part of it.
		//
		s_buffer_set(&print, ptr, filter->len);
		intersection(visible, &print, &result);

		//
		// Ensure that the string to print is visible
		//
		if (result.ptr != NULL) {

			//
			// Set the highlighted attribute print the string and reset the
			// attribute again.
			//
			wattrset(win, attr_cur->highlight);
			mvwaddnwstr(win, win_y, win_x, result.ptr, result.len);
			wattrset(win, (attr_cur->normal));

			//
			// Update the column position in the window.
			//
			win_x += result.len;
		}

		//
		// Update the current position to the end of the found search string.
		//
		cur = ptr + filter->len;
	}
}

/***************************************************************************
 * The function prints the content of a field. The field can be truncated,
 * so maybe parts of the field content are not printed. The field may
 * contain a filter string, which will be highlighted.
 *
 * The function call has the field part parameter, which define the visible
 * part of the field.
 *
 * The win_row_col parameter contains the x, y coordinates of the field in
 * the window.
 *
 * The function is called with the width parameter which is the total, not
 * truncated width of the column.
 **************************************************************************/

void print_field_content(WINDOW *win, wchar_t *field_content, const s_field_part *row_field_part, const s_field_part *col_field_part, const s_field *win_row_col, const int width, wchar_t *filter, const s_attr *attr_cur) {
	int row;
	bool end = false;

	print_debug("print_field_content() win row: %d win col: %d field: '%ls'\n", win_row_col->row, win_row_col->col, field_content);

	s_buffer search;

	//
	// A pointer to mark the current position in the field content, while it
	// is parsed.
	//
	wchar_t *ptr = field_content;

	//
	// Ensure that the filter is set of the wcslen causes a seg fault.
	//
	if (filter != EMPTY_FILTER_STRING) {
		search.ptr = filter;
		search.len = wcslen(filter);
	}

	//
	// Get the height of the field. If the field is truncated, start is
	// greater than 0 or size is less than the row height.
	//
	const int field_height = row_field_part->start + row_field_part->size;

	//
	// Create a buffer and add the str terminator.
	//
	wchar_t buffer[width + 1];
	buffer[width] = W_STR_TERM;

	s_buffer buf;
	buf.ptr = buffer + col_field_part->start;
	buf.len = col_field_part->size;

	for (int field_line_no = 0; field_line_no < field_height; field_line_no++) {

		//
		// Get the next field line. The pointer is updated to the next line or
		// NULL if no more lines are present. The buffer contains the line a
		// may contain padding chars.
		//
		ptr = get_field_complete_line(ptr, buffer, width, &end);
		print_debug("print_field_content() field line: %d '%ls'\n", field_line_no, buffer);

		//
		// Skip the first lines if necessary,
		//
		if (field_line_no >= row_field_part->start) {
			row = win_row_col->row + field_line_no - row_field_part->start;

			if (filter != EMPTY_FILTER_STRING) {
				print_line(win, row, win_row_col->col, buffer, &buf, &search, attr_cur);
			} else {
				mvwaddnwstr(win, row, win_row_col->col, buf.ptr, buf.len);
			}
		}
	}
}
