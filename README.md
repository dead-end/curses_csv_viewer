# ncurses_csv_viewer
A CSV file viewer implemented with ncurses

## Implementation details
Each table consists of columns and rows. Each row has a maximum heigth and 
each column has a maximum width.
If the table width and hight is larger than the window size, only a part of
the table will be visualized.
The table view has a reference field. The reference field is aligned with one
of the corners. 

![table part](img/table_part.png?raw=true "Table Part")


![field part](img/field_part.png?raw=true "Field Part")
