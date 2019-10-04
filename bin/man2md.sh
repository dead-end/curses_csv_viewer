#!/bin/sh

################################################################################
# The script prints the man page as a markdown file. 
################################################################################

set -u

echo '```'
env COLUMNS=80 man --pager=cat ${PWD}/man/ccsvv.1
echo '```'

exit 0
