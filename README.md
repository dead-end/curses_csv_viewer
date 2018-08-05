# ncurses_csv_viewer
A CSV file viewer implemented with ncurses

## Usage

```
ccsvv [-h] [-m] [-n] [-d delimiter ] [ file ]
```

| Option | Description |
| :---: | :---: | 
| `-h` | Shows the usage message. |
| `-d delimiter` | Defines a delimiter character other than the default comma.|
| `-m` | By default ccsvv uses colors if the terminal supports them. With this option ccsvv is forced to use a monochrom mode.|

## Implementation details
Each table consists of columns and rows. Each row has a maximum heigth and 
each column has a maximum width.
If the table width and hight is larger than the window size, only a part of
the table will be visualized.

The table view has a reference field. The reference field is aligned with one
of the corners. The fields at the oposite sides of the window may be truncated.

![table part](img/table_part.png?raw=true "Table Part")

A truncated field may be truncated left or right or at top or bottom.

![field part](img/field_part.png?raw=true "Field Part")
