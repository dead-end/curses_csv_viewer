/*
 * ncv_filter.h
 *
 *  Created on: 12.07.2018
 *      Author: besen
 */

#ifndef INC_NCV_FILTER_H_
#define INC_NCV_FILTER_H_

#define FILTER_FIELD_ROWS 1
#define FILTER_FIELD_COLS 32

#define FILTER_FIELD_LABEL     "Filter: "
#define FILTER_FIELD_LABEL_LEN 8

#define WIN_FILTER_SIZE (FILTER_FIELD_COLS + FILTER_FIELD_LABEL_LEN + 1)

void filter_init(WINDOW *win);

void filter_free();

void filter_loop();

#endif /* INC_NCV_FILTER_H_ */
