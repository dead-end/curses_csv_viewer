#!/bin/sh

################################################################################
# The script creates a snippet of c code with the help message from the man
# page source file. The snippet can be manually included in the src/ncv_ccsvv.c
# file.
################################################################################

set -u

#
# Define the man page source file
#
manpage=${PWD}/man/ccsvv.1

#
# Define the prefix and the postfix of each source code line
#
pre='fprintf(stream, "'

post='\\n");'

#
# Get the man page with column size 83. Later we remove 3 leading spaces.
#
tmp=$(env COLUMNS=83 man --pager=cat ${manpage})

#
# Get the part between the NAME and the EXAMPLE section and remove all section
# headers.
#
tmp=$(echo "$tmp" | sed -n '/NAME/,/EXAMPLES/p' | grep -v '^[A-Z]')

#
# Remove some leading spaces.
#
tmp=$(echo "$tmp" | sed 's#^   ##')

#
# Add a c prefix and postfix to each line.
#
echo "$tmp" | sed "s#^#$pre#" | sed "s#\$#$post#"

exit 0
