#/bin/sh

###############################################################################
# Some saveguarding definitions. 
###############################################################################

set -ue

###############################################################################
# Ensure that the script is called with an executable 
###############################################################################

if [ "$#" != "1" ] ; then
  echo "Usage: $0 <bin>"
  exit 1
fi

executable="${1}"

if [ ! -x "${executable}" ] ; then
  echo "The file: ${executable} is not an executable!"
  exit 1
fi

echo "Processing: ${executable}"

###############################################################################
# Get the list of dependencies. The script uses "raw" variables which are not
# necessary, but they make debugging easier.
###############################################################################

raw_dep_libs=$(objdump -p "${executable}")

dep_libs=$(echo "${raw_dep_libs}" | grep NEEDED | sed 's# *NEEDED *##')

deps=

for dep_lib in ${dep_libs} ; do
  raw_dep=$(dpkg -S ${dep_lib})
  dep=$(echo "${raw_dep}" | sed 's#:.*##')
  deps="${deps}\n${dep}"
done

deps=$(echo "$deps" | sort -u)

echo "Found dependencies: ${deps}"

exit 0


