```
CCSVV(1)                         User Commands                        CCSVV(1)

NAME
       ccsvv - displays a csv file (comma-separated values) as a table

SYNOPSIS
       ccsvv [OPTION]... [FILE]

DESCRIPTION
       The  program  is  called with the name of a csv FILE. If no filename is
       given, ccsvv reads the csv data from stdin.

       -h     Shows a help text.

       -d delimiter
              Defines a delimiter character, other than the default comma.

       -m     By default ccsvv uses colors if it is supported by the terminal.
              With this option ccsvv is forced to use a monochrom mode.

       -s | -n
              The  flags  define  whether the first row of the table is inter‐
              preted as a header for the table (-s) or not (-n).  If  none  of
              the  flags  is  given  ccsvv tries to detect whether a header is
              present or not.

       -t     Switch off trimming of csv fields.

       -c     By default ccsvv adds missing fields in a row. The flag switches
              on strict checks. A missing field results in an error.

COMMANDS
       After ccsvv was started, the following commands are supported:

       ^C, ^Q Terminates the program.

       ^H     Shows a help dialog.

       ^F     Shows a filter / search dialog.

       ^X     Deletes the filter / search string in the dialog.

       ESC    Deletes the filter / search string and resets the table.

       ^N, ^P Searches  for  the next (^N) / previous (^P) field that contains
              the filter / search string.

       ^S, ^R Sorts the table with the current  column  in  ascending  (^S)  /
              descending (^R) order.

EXAMPLES
       Display the password file with ccsvv:

              ccsvv -d : /etc/passwd

SEE ALSO
       ncurses(3)

BUGS
       No known bugs.

AUTHOR
       Dead End

V 0.3                           September 2019                        CCSVV(1)
```