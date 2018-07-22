/*
 * ncv_win_table.h
 *
 */

#ifndef INC_NCV_WIN_TABLE_H_
#define INC_NCV_WIN_TABLE_H_

#include "ncv_table.h"
#include "ncv_table_part.h"

//
// Structure for the cursor.
//
typedef struct s_cursor {

	int row;

	int col;

	bool visible;

} s_cursor;

void win_table_init();

void win_table_resize();

void win_table_refresh();

void win_table_refresh_no();

void win_table_free();

void win_table_content_init(const s_table *table, s_cursor *cursor);

void win_table_content_resize(const s_table *table, s_cursor *cursor);

bool win_table_content_mv_cursor(const s_table *table, s_cursor *cursor, const int key_input);

void win_table_content_print(const s_table *table, const s_cursor *cursor);

#endif /* INC_NCV_WIN_TABLE_H_ */
