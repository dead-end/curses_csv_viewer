## Implementation details
Each table consists of columns and rows. Each row has a maximum height and 
each column has a maximum width.
If the table width and hight is larger than the window size, only a part of
the table will be visualized.

The table view has a reference field. The reference field is aligned with one
of the four corners. The fields along the opposite side are truncated if the
sizes do not fit.

![table part](../img/table_part.png?raw=true "Table Part")

A truncated field may be truncated left or right or at top or bottom. This means
lines of the field are not visible if the hight of the row is truncated or the
first or last characters of a line is not visible if the column is truncated. 

![field part](../img/field_part.png?raw=true "Field Part")

To compute the truncated sizes, we start at the reference field and sum up the 
widths or heights until the sum exceeds the width or hight of the visible part 
of the table.

## Sorting
*ccsvv* supports sorting of the table by a given column. The sorting can be 
*alphanumerical* or *numerical*. The *alphanumerical* sorting is done as a wide 
character string and is performed if converting the column values to a 
*numerical* value is not possible.

If the table has a header row, this row stays always at the to. Then *ccsvv* 
tries to convert the rest of the column to a double value. If this succeeds, the
table is sorted *numerically*.

The *numerically* sorting is done with:

```c
double wcstod(const wchar_t *restrict nptr, wchar_t **restrict endptr);
```

An empty value is converted to DBL_MAX, the maximum defined value for a double,
The result is, that the empty rows are at the top.

The function allows a non-numerical postfix string. This string has to be equal
for all none empty values. In the example file, this is *" Euro"*.

| No |       Price   |
| -- |---------------|
|  1 | 10000,00 Euro |
|  2 |               |
|  3 |     2,00 Euro |
|  4 |    10,00 Euro |

The sorted result is:

| No |       Price   |
| -- |---------------|
|  2 |               |
|  3 |     2,00 Euro |
|  4 |    10,00 Euro |
|  1 | 10000,00 Euro |

The decimal point depends on the locale, especially the LC_NUMERIC value.

## s_wblock
*ccsvv* uses a buffer, which is internally a linked list of (wchar_t) blocks. 
The size of each new block is doubled. The buffer is used store the csv data. 
The csv data can be read from a file or from stdin via a pipe and has to be 
parsed twice. The first time the number of columns and rows are determined as 
well as the maximal hight of a row and width of a column. The values are needed 
for the memory allocation.

A file can be read twice for the parsing, but the input pipe cannot reset to the 
beginning. So the input data has to be stored for the second parsing and this is 
done with the buffer.

![table part](../img/s_wblock.png?raw=true "Buffer")
