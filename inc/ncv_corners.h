/*
 * ncv_corners.h
 */

#ifndef INC_NCV_CORNERS_H_
#define INC_NCV_CORNERS_H_

#include "ncv_common.h"
#include "ncv_table.h"
#include "ncv_table_part.h"

/***************************************************************************
 * Function definitions
 **************************************************************************/

void s_corner_inits(const int no_rows, const int no_columns);

void s_corner_print(WINDOW *win, const s_field *idx, const s_field *win_field, const s_field *win_field_end, const s_table_part *row_table_part, const s_table_part *col_table_part);

#endif /* INC_NCV_CORNERS_H_ */
