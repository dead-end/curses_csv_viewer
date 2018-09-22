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
It would be nice if there would be no need for the user of ccsvv to configure whether the csv file has a header row or not at the program start. So we can try to detect whether a given table has a header or not. If the csv file has no header, it is reasonable to assume that all rows of the column are similar. To prove this, we compute some characteristics of the first row of the column and compare that characteristics with the rest of the rows.
The characteristics are:

* string-length
* number-of-digits / string-length 

First we compute the string length of the first row `First(S)` of the column. Then we compute the mean string length `Mean(S)` and the standard deviation of the string lengths `StdDev(S)` of the rest of the rows of the column.

If we assume that the string lengths are normaly distributed, then 99,73% are inside an interval of 3 times the standard deviation around the mean value. So the following equation gives a good indicator for the existence of a header:

```
|First(S) - Mean(S)| > 3 * StdDev(S)
```

If the result is not clear, we do the same computation for the ratio `number-of-digits / string-length`. If it is still not clear we do the computations for the next column and so on.


Let us take a look at an example the show what this means.

| Number | Price       | Date          |
| ------ |-------------| --------------|
| 2      | 0,20 Euro   | Fr 21.09.2018 |
| 4      | 1 Euro      | Sa 22.09.2018 |
| 8      | 1,20 Euro   | Su 23.09.2018 |
| 16     | 10,20 Euro  | Mo 24.09.2018 |
| 32     | 100,20 Euro | Th 25.09.2018 |

For the first row of the column is simple. The `Number` string has 6 characters and no digits:

```
First(S) = 6 
First(R) = 0/6 = 0
```

For the rest of the column we compute the mean string length, which is the sum of the string lengths divided by the number of rows:
```
Mean(S) = (1 + 1 + 1 + 2 + 2)/5 = 7/5 = 1,4
```
The variance of the string lengths is the sum of the squared difference of each length with the mean length divided by the number of rows:
```
Var(S) = 1/5 * ((1-1,4)^2 + (1-1,4)^2 + (1-1,4)^2 + (2-1,4)^2 + (2-1,4)^2)
       = 1/5 * (3 * 0,4^2 + 2 * 0,6^2)
       = 1/5 * 1,2
       = 0,24
```
Then we analyse the diviation of the string length of the first row from the mean.
```
    |First(S) - Mean(S)| = X * Var(S)
                6 - 1,4  = X * 0,24
<=>          5,76 / 0,24 = X
<=>                   24 = X
```
So the difference of the length of the first column row with the mean is 24 times the variance. This is a good indecator that the first column has a header.

```
Mean(R) = (1/1 + 1/1 + 1/1 + 2/2 + 2/2)/5 = 5/5 = 1

Var(R) = 1/5 * ((1 - 1)^2 + (1 - 1)^2 + (1 - 1)^2 + (1 - 1)^2 + (1 - 1)^2)
       = 1/5 * 0
       = 0
```
If we look at the ratio of the digits with string length the result is even more clear. The ratio is `1` and the variance is `0`.

#### Conclusion
Columns with a variance of zero or at least a very small variance are good canidates to detect a header. Examples are columns with integer, float, date or currency values.

If the number of rows of the csv file are larger, there is no need to analyse all rows of a column. We can define a maximum number of rows, for example 64.

For each column we have two criteria which can indicate a header. We can define a sufficient number of matching criteria, for example three. If this number is reached we can stop analysing more columns
