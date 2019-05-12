# ccsvv
**ccsvv** (curses csv viewer) is a program that displays [CSV](https://en.wikipedia.org/wiki/Comma-separated_values) (comma-separated values) files as a table. It supports:

* different field separators
* escaping with **"**
* multi line fields
* different line endings (win / mac / linux)
* unicode characters (wchar_t)
* header detection (see below)
* filtering

![Show example](img/languages.png)

## Usage
```
ccsvv [-h] [-m] [-s | -n] [-d delimiter] [file]

  -h            Shows this usage message.

  -d delimiter  Defines a delimiter character, other than the default comma.

  -m            By default ccsvv uses colors if the terminal supports them. With
                this option ccsvv is forced to use a monochrom mode.

  -s | -n       The flags define whether the first row of the table is
                interpreted as a header for the table ('-s') or not ('-n').
                If none of the flags is given ccsvv tries to detect whether a
                header is present or not.

  -t            Switch off trimming of the csv fields.

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
## Installation

### Debian Package
The easierst way to install **ccsvv** is to the use the debian package attatched to the 
[latest release](../../releases/latest). Download the *.deb* file, check the md5 sum and install the package:

```bash
md5sum ccsvv_X.Y.Z_amd64.deb
sudo dpkg -i ccsvv_X.Y.Z_amd64.deb
```

### Build from Sources
Ensure that the dependencies are fullfilled (see below). Download the sources zipfile for the [latest release](../../releases/latest) and compile **ccsvv** with:

```
make
```
An alternative is to use cmake. This can be done with:
```
mkdir cmake-build
cd cmake-build
cmake -DCMAKE_BUILD_TYPE=Release ..
make
make test
```

### Binary Tar
Ensure that the dependencies are fullfilled (see below). The [latest release](../../releases/latest) contains a tar 
file with a compiled binary. Download the *.tgz* file, check the md5 sum and untar the file. 

```bash
md5sum ccsvv_X.Y.Z.tgz
tar xvzf ccsvv_X.Y.Z_amd64.deb
```

### Dependencies
**ccsvv** uses *ncursesw* to create and display the tables. *ncursesw* is the wide character version of *ncurses*.

For the deb package or the precompiled executable the following list of dependencies is relevant for ubuntu 18.0.4:

* libc6        (>=2.27)
* libtinfo5    (>=6.1)
* libncursesw5 (>=6.1)

To sucessfully compile **ccsvv** requires a package with the library and debug informations and a developer package 
with the header files. For ubuntu 18.0.4 this is:

* libc6            (>=2.27)
* libtinfo5-dbg    (>=6.1)
* libncursesw5-dbg (>=6.1)
* libncursesw5-dev (>=6.1)

If you compiled your own *ncursesw5* lib, make sure that the LD_LIBRARY_PATH is properly set.

## Example: /etc/passwd
**ccsvv** can be used to browse through all kinds of csv like files. The following example shows the `/etc/passwd` file, filtered by *system*.
```
ccsvv -d : /etc/passwd
```
![Show /etc/passwd](img/etc-passwd.png)

## Example: Derby DB
In this section we want to discuss how to use **ccsvv** to display the results of a database client. 

We use Derby DB as our database, which has a client called `ij`. It accepts a property file, with basic configurations. In our case [ij.properties](derby-db/ij.properties):

```properties
ij.user=user
ij.password=password
ij.database=mydb
#ij.database=mydb;create=true
ij.protocol=jdbc:derby:
ij.showNoCountForSelect=true
ij.showNoConnectionsAtStart=true
ij.maximumDisplayWidth=256
```
For our example, we need to create an example database with a table. This can be done with a 
sql file [create.sql](derby-db/create.sql) with the following content:

```sql
create table mytab(idx int, name varchar(40));
insert into mytab values (1,'Derby DB'); 
insert into mytab values (2,'PostgreSQL'); 
insert into mytab values (3,'Maria DB'); 
insert into mytab values (4,'Mysql'); 
```
We can execute the statements with the following call. This requires the flag `create=true` to be set in the
[ij.properties](derby-db/ij.properties) file. Make sure that you removed the flag after you created the database,
to prevent warnings.

```bash
ij -p ij.properties create.sql
```

Piping a select statement to the ij client returns the table data. 

```
echo "select * from mytab;" | ij -p ij.properties 
ij version 10.14
ij> select * from mytab;
NUM        |ADDR                                    
----------------------------------------------------
1          |Derby DB                                
2          |PostgreSQL                              
3          |Maria DB                                
4          |Mysql                                   
ij>
```
This result is OK as long as the table is small. 

To be able to use the output of ij for **ccsvv**, we have to remove the unnecessary lines. This can be done with a
small shell script. The script is called with a sql statement and the first step is to copy that statement to a file. Using a file rather than piping the statement to the client gives better control over the promt of `ij`.

A call of `grep` removes the line, that separates the header from the table data, the `ij` prompt, the version
information and lines that only consist of a single `;`. This happens if you forget the `;` character after your
sql statement. The last line pipes the resulting data to **ccsvv**. The field separator is the pipe character `|`: 

```bash
#!/bin/sh
set -u
echo "$@" > .derby.sql
result=$(ij -p ij.properties .derby.sql | grep -v -e '^-*$' -e '^ij>' -e '^ij version' -e '^;$')
echo "$result" | ./ccsvv -s -d '|' || echo "$result"
```

An example call of the script [derby_client.sh](derby-db/derby_client.sh) with the result: 

```
sh derby_client.sh "select * from mytab"
```

![Show query example](img/derby-db.png)

### Header detection
It would be nice if there would be no need for the user of *ccsvv* to configure whether the csv file has a header row or not at the program start. So we can try to detect whether a given table has a header or not. If the csv file has no header, it is reasonable to assume that all rows of the column are similar. To prove this, we compute some characteristics of the first row of the column and compare that characteristics with the rest of the rows.
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

For this table we can compute the mean and the standard deviation of the two characteristics for each column:

| Column | Type          | Mean    | Std. deviation | First row | Indicator |
| ------ |---------------|---------|----------------|-----------|-----------|
| Number | String length |  1,4000 | 0,5477         | 6,0000    | true      |
|        | Digit ratio   |  1,0000 | 0,0000         | 0,0000    | true      |
| Price  | String length |  9,0000 | 1.8708         | 5,0000    | false     |
|        | Digit ratio   |  0.3375 | 0.1081         | 0,0000    | true      |
| Date   | String length | 13,0000 | 0,0000         | 4,0000    | true      |
|        | Digit ratio   |  0,6154 | 0,0000         | 0,0000    | true      |

#### Conclusion
Columns with a standard deviation of zero or at least a very small standard deviation are good canidates to detect a header. Examples are columns with integer, float, date or currency values.

If the number of rows of the csv file is large, there is no need to analyse all rows of a column. We can define a maximum number of rows, for example 64.

For each column we have two criteria which can indicate a header. We can define a sufficient number of matching criteria, for example three. If this number is reached we can stop analysing more columns. In the example above, it is enought to analyse the first 2 columns to get 3 positive indicators.

Analyzing the csv file works only if there are enough rows and column. If not enough are present, it is assumed that the table has a header.

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
