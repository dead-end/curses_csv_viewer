/*
 * ncv_win_filter.h
 */

#ifndef INC_NCV_WIN_FILTER_H_
#define INC_NCV_WIN_FILTER_H_

#include "ncv_common.h"

#define FILTER_FIELD_ROWS 1
#define FILTER_FIELD_COLS 32

#define FILTER_FIELD_LABEL     "Filter: "
#define FILTER_FIELD_LABEL_LEN 8

#define WIN_FILTER_SIZE (FILTER_FIELD_COLS + FILTER_FIELD_LABEL_LEN + 1)

void win_filter_init();

void win_filter_resize();

void win_filter_refresh_no();

void win_filter_free();

WINDOW *win_filter_get_win();

void win_filter_process_input(const int key_type, const wint_t chr);

void win_filter_get_filter(wchar_t *buffer, const int buf_size);

#endif /* INC_NCV_WIN_FILTER_H_ */
