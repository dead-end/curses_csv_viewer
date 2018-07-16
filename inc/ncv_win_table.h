/*
 * ncv_win_table.h
 *
 */

#ifndef INC_NCV_WIN_TABLE_H_
#define INC_NCV_WIN_TABLE_H_

#include "ncv_common.h"

void win_table_content_init(const s_table *table, s_table_part *row_table_part, s_table_part *col_table_part, s_field *cursor);

void win_table_content_resize(const s_table *table, s_table_part *row_table_part, s_table_part *col_table_part, s_field *cursor);

bool win_table_content_mv_cursor(const s_table *table, s_table_part *row_table_part, s_table_part *col_table_part, s_field *cursor, const int key_input);

void win_table_content_print(WINDOW *win, const s_table *table, const s_table_part *row_table_part, const s_table_part *col_table_part, const s_field *cursor);

void table_win_resize();

#endif /* INC_NCV_WIN_TABLE_H_ */
