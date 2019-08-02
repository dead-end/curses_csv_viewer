## s_wblock
*ccsvv* uses a buffer, which is internally a linked list of blocks. The size of the blocks doubles for each
new block. The buffer is used store the csv data. The csv data can be read from a file or from stdin via a pipe.
The csv data has to be parsed twice. The first time the number of columns and rows are determined as well as the
maximal hight of a row and width of a column. The values are needed for the memory allocation.
A file can be read twice for the parsing, but the input pipe cannot ge seek to the beginning. So the input data 
has to be stored for the second parsing.

![table part](../img/s_wblock.png?raw=true "Buffer")