#ifndef INC_NCV_TABLE_H_
#define INC_NCV_TABLE_H_

//
// The table structure contains the csv data.
//
typedef struct s_table {

	//
	// the number of columns and rows
	//
	int no_columns;
	int no_rows;

	//
	// an array with the maximum size of each column
	//
	int *sizes;

	//
	// a two dimensional array with the content
	//
	wchar_t ***fields;

} s_table;

//
// The table related function
//
void s_table_create(s_table *table, const int no_columns, const int no_rows);

void s_table_free(s_table *table);

void s_table_copy(s_table *table, const int rows, const int columns, const wchar_t *str);

void s_table_dump(s_table *table);

#endif /* INC_NCV_TABLE_H_ */
