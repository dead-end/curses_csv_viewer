#!/bin/sh

################################################################################
# The script converts the man page to a usage message or a md file.
################################################################################

set -u

#
# Define the man page source file
#
manpage=${PWD}/man/ccsvv.1

################################################################################
# The function prints a usage / help message. If it is called with an argument
# this argument is interpreted as an error message and the function terminates
# the script with an exit code of 1. Otherwise the function terminates the 
# script normally.
################################################################################

usage() {
  cat <<EOF
Usage: ${0} [help|usage|md]
  help  : Writes this message
  usage : Creates a c code snipet with a usage message
  md    : Outputs man page as a md file. The result can be redirected:

          sh bin/man.sh md > man/README.md
EOF

  if [ "${#}" != "0" ] ; then
    echo "\nERROR: ${1}"
    exit 1
  else
    exit 0
  fi
}

################################################################################
# The function creates a snippet of c code with the help message from the man
# page source file. The snippet can be manually included in the src/ncv_ccsvv.c
# file.
################################################################################

man2usage() {

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
}

################################################################################
# The script prints the man page as a markdown file. 
################################################################################

man2md() {

  echo '```'
  env COLUMNS=80 man --pager=cat ${manpage}
  echo '```'
}

################################################################################
# Main
################################################################################

if [ ! -f "bin/man.sh" ] ; then
  echo "The script is not called from the application root directory!"
  exit 1
fi

if [ "${#}" != "1" ] ; then
  usage "Mode is missing"
fi

mode="${1}"

if [ "${mode}" = "help" ] ; then
  usage

elif [ "${mode}" = "usage" ] ; then
  man2usage

elif [ "${mode}" = "md" ] ; then
  man2md

else
  usage "Unknown mode: ${mode}"
fi

exit 0
