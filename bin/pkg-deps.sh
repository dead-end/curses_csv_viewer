#/bin/sh

###############################################################################
# Some saveguarding definitions. 
###############################################################################

set -ue

DEBUG=

###############################################################################
# logging function
###############################################################################

log () {
  level="${1}"
  msg="${2}"
  
  if [ "${level}" = "debug" ] ; then
    if [ -n "${DEBUG}" ] ; then
      echo "DEBUG: ${msg}"
    fi
    
  elif [ "${level}" = "info" ] ; then
    echo "INFO: ${msg}"

  elif [ "${level}" = "error" ] ; then
    echo "ERROR: ${msg}"
    exit 1
  else
    log "error" "EXIT: Unknown level: ${level}"
  fi
}

###############################################################################
# Ensure that the script is called with an executable 
###############################################################################

if [ "$#" != "1" ] ; then
  echo "Usage: $0 <bin>"
  exit 1
fi

executable="${1}"

if [ ! -x "${executable}" ] ; then
  log "error" "The file: ${executable} is not an executable!"
fi

echo "Processing: ${executable}"

###############################################################################
# Get the list of dependencies. The script uses "raw" variables which are not
# necessary, but they make debugging easier.
###############################################################################

raw_dep_libs=$(objdump -p "${executable}")

dep_libs=$(echo "${raw_dep_libs}" | grep NEEDED | sed 's# *NEEDED *##')

log "debug" "dep_libs: ${dep_libs}"

dep_pkgs=

for dep_lib in ${dep_libs} ; do

  log "debug" "Processing: ${dep_lib}"
  
  pkgs_libs=$(dpkg -S ${dep_lib})
  log "debug" "Packages and libs:\n${pkgs_libs}"
  
  pkgs=$(echo "${pkgs_libs}" | sed 's#:.*##')
  log "debug" "Packages:\n${pkgs}"
  
  dep_pkgs="${dep_pkgs}\n${pkgs}"
done

###############################################################################
# Create a unique list of dependend packages and get the version for each 
# package. The extention in the version is removed.
# (before: 6.1-1ubuntu1.18.04 after: 6.1)
###############################################################################

echo "Found dependencies:"

dep_pkgs=$(echo "$dep_pkgs" | sort -u)

for dep_pkg in ${dep_pkgs} ; do
  version=$(dpkg -s "${dep_pkg}" | grep "Version:")
  log "debug" "version: ${version}"
  
  version=$(echo "${version}" | sed 's#Version: ## ; s#-.*##')
  echo "${dep_pkg} (${version})"
done

exit 0


