## s_wblock
*ccsvv* uses a buffer, which is internally a linked list of (wchar_t) blocks. The size of each new block is
doubled. The buffer is used store the csv data. The csv data can be read from a file or from stdin via a pipe 
and has to be parsed twice. The first time the number of columns and rows are determined as well as the
maximal hight of a row and width of a column. The values are needed for the memory allocation.

A file can be read twice for the parsing, but the input pipe cannot reset to the beginning. So the input data 
has to be stored for the second parsing and this is done with the buffer.

![table part](../img/s_wblock.png?raw=true "Buffer")