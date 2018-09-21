# ccsvv
ccsvv (curses csv viewer) is a programm that displays [CSV](https://en.wikipedia.org/wiki/Comma-separated_values) (comma-separated values) files as a table.

## Usage

```
ccsvv [-h] [-m] [-n] [-d delimiter] [file]

  -h            Shows this usage message.

  -d delimiter  Defines a delimiter character, other than the default comma.

  -m            By default ccsvv uses colors if the terminal supports them. With
                this option ccsvv is forced to use a monochrom mode.

  -n            By default ccsvv interpretes the first row of the table as a
                header. The header row is highlighted and on filtering, it is
                alway part of the result, even if no field contains the filter
                string. With this option special role of the first line is
                switched off.

  file          The name of the csv file. If no filename is defined, ccsvv reads
                the csv data from stdin.

Commands:

  ^C and ^Q     Terminate the program.

  ^F            Switches to filter input mode and allows to input a filter
                string.

  ^X            In filter mode, deletes the filter string.

  ESC           Delete the filter string and reset the table.

  ^N and ^P     Search for the next / previous field that contains the filter
                string.
```
## Examples
ccsvv can deal with multi-line fields and supports unicode characters (wchar_t).
```
ccsvv languages.csv
```
![Show example](img/languages.png)

If the terminal does not support colors, ccsvv shows the table in a monochrome mode, which can be enforced by the `-m` command line option.
```
ccsvv -m languages.csv
```
![Show monochrome example](img/mono.png)

ccsvv can be used to browse through all kinds of csv like files. The following example shows the `/etc/passwd` file, filtered by `system`.
```
ccsvv -nd : /etc/passwd
```
![Show /etc/passwd](img/etc-passwd.png)
## Example: Database
Most databases are able to store tables dumps or queries in csv files, which can be displayed with ccsvv. The following example shows a sql statement from MariaDB, that stores a query against the `user` table in a csv file. It takes a little affort to add the table header to the csv file:

```sql
SELECT 'Host', 'User', 'Max Questions', 'Max Updates', 'Max Connections', 'Max User Uonnections'
UNION ALL
SELECT Host, User, max_questions, max_updates, max_connections, max_user_connections
  INTO OUTFILE '/tmp/query.csv'
  FIELDS TERMINATED BY ',' OPTIONALLY ENCLOSED BY '"'
  LINES TERMINATED BY '\n'
  from user;
```

After creating the sql file you can call the following command to display the result with ccsvv.

```bash
sudo rm -f /tmp/query.csv && sudo mysql -u root -h localhost mysql < /tmp/query.sql && ccsvv /tmp/query.csv
```
![Show query example](img/query.png)

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

### Header detection
The goal is to detect whether a given table has a header or not. To do this, we compare for each column some characteristics of the first row with that of the rest of the rows of that column. The characteristics are the mean string length (S) and the mean ratio of the number of digits with the string length (R). To assess the difference of the first row of the column with the rest we compute the variance.

| Number | Price       | Date          |
| ------ |-------------| --------------|
| 2      | 0,20 Euro   | Fr 21.09.2018 |
| 4      | 1 Euro      | Sa 22.09.2018 |
| 8      | 1,20 Euro   | Su 23.09.2018 |
| 16     | 10,20 Euro  | Mo 24.09.2018 |
| 32     | 100,20 Euro | Th 25.09.2018 |

#### Number
First row: `S = 6` and `R = 0/6 = 0`

Rest: 

```
S = (1 + 1 + 1 + 2 + 2)/5 = 7/5 = 1,4

Var(S) = 1/5 * ((1-1,4)^2 + (1-1,4)^2 + (1-1,4)^2 + (2-1,4)^2 + (2-1,4)^2)
       = 1/5 * (3 * 0,4^2 + 2 * 0,6^2)
       = 1/5 * 1,2
       = 0,24
```
So the difference of the length of the first column row with the mean is `5,76` which is 24 times the variance `0,24`. This is a good indecator that the first column has a header.

```
R = (1/1 + 1/1 + 1/1 + 2/2 + 2/2)/5 = 5/5 = 1

Var(r) = 1/5 * ((1 - 1)^2 + (1 - 1)^2 + (1 - 1)^2 + (1 - 1)^2 + (1 - 1)^2)
       = 1/5 * 0
       = 0
```
If we look at the ratio of the digits with string length the result is even more clear. The ratio is `1` and the variance is `0`.

