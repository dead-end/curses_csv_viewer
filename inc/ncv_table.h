#ifndef INC_NCV_TABLE_H_
#define INC_NCV_TABLE_H_

#include "ncv_common.h"

//
// The table structure contains the csv data.
//
/***************************************************************************
 * Function definitions
 **************************************************************************/
typedef struct s_table {

	//
	// The number of columns and rows of the table. The parameter __no_rows
	// holds the correct number, while no_rows holds the filtered number of
	// rows.
	//
	int no_columns;

	int __no_rows;

	int no_rows;

	//
	// an array with the widths of the columns
	//
	int *width;

	//
	// An array with heights of the rows. Again height has the heights of
	// the filtered rows.
	//
	int *__height;

	int *height;

	//
	// A two dimensional array with the fields of the csv file. The
	// parameter __fields has the data, while fields has pointers to the
	// row data.
	//
	wchar_t ***__fields;

	wchar_t ***fields;

} s_table;

//
// The table related function
//
void s_table_init(s_table *table, const int no_columns, const int no_rows);

void s_table_free(s_table *table);

void s_table_copy(s_table *table, const int rows, const int columns, wchar_t *str);

void s_table_field_dimension(wchar_t *str, int *width, int *height);

void s_table_reset_filter(s_table *table);

void s_table_do_filter(s_table *table, const wchar_t *filter);

void s_table_dump(const s_table *table);

#endif /* INC_NCV_TABLE_H_ */
