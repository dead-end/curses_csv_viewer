## Implementation details
Each table consists of columns and rows. Each row has a maximum heigth and 
each column has a maximum width.
If the table width and hight is larger than the window size, only a part of
the table will be visualized.

The table view has a reference field. The reference field is aligned with one
of the four corners. The fields along the opposite side are truncated if the
sizes do not fit.

![table part](../img/table_part.png?raw=true "Table Part")

A truncated field may be truncated left or right or at top or bottom. To compute the
truncated sizes, we start at the reference field and sum up the widths or hights until
the sum exceeds the width or hight of the visible part of the table.

![field part](../img/field_part.png?raw=true "Field Part")

## s_wblock
*ccsvv* uses a buffer, which is internally a linked list of (wchar_t) blocks. The size of each new block is
doubled. The buffer is used store the csv data. The csv data can be read from a file or from stdin via a pipe 
and has to be parsed twice. The first time the number of columns and rows are determined as well as the
maximal hight of a row and width of a column. The values are needed for the memory allocation.

A file can be read twice for the parsing, but the input pipe cannot reset to the beginning. So the input data 
has to be stored for the second parsing and this is done with the buffer.

![table part](../img/s_wblock.png?raw=true "Buffer")