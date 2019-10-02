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
