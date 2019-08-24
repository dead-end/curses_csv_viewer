#/bin/sh

###############################################################################
# The script is called with an executable file and it returns a list of deb
# packages (including the version), which the executable depends on. This
# information can be used for the deb package build. It has a mode "no-debug"
# or "all", which can be used to ignore the debug dependencies.
#
# Example:
#
#    >sh bin/pkg-deps.sh no-debug ccsvv
#    libc6 (>=2.27), libncursesw5 (>=6.1), libtinfo5 (>=6.1)
#
#    >sh bin/pkg-deps.sh all ccsvv
#    libc6 (>=2.27), libncursesw5 (>=6.1), libncursesw5-dbg (>=6.1), \
#    libtinfo5 (>=6.1), libtinfo5-dbg (>=6.1)
###############################################################################

set -ue

DEBUG=

###############################################################################
# A logging function that supports the log level "debug", "info", "error". The
# function exits the script after printing an error message.
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
# The function prints the list of packages, which the executable depends on.
###############################################################################

print_pkgs() {
  dep_pkgs="${1}"
  
  sep=
  result=

  for dep_pkg in ${dep_pkgs} ; do
    version=$(dpkg -s "${dep_pkg}" | grep "Version:")
    log "debug" "version: ${version}"
  
    version=$(echo "${version}" | sed 's#Version: ## ; s#-.*##')
    result="${result}${sep}${dep_pkg} (>=${version})"
  
    if [ -z "${sep}" ] ; then
      sep=", "
    fi
  done
  
  echo "${result}"
}

###############################################################################
# Ensure that the script is called with an executable 
###############################################################################

if [ "$#" != "2" ] ; then
  echo "Usage: $0 [no-debug|all] <bin>"
  exit 1
fi

mode="${1}"

if [ "${mode}" != "no-debug" -a "${mode}" != "all" ] ; then
  echo "Usage: $0 [no-debug|all] <bin>"
  exit 1
fi

executable="${2}"

if [ ! -x "${executable}" ] ; then
  log "error" "The file: ${executable} is not an executable!"
fi

log "debug" "Processing: ${executable}"

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

dep_pkgs=$(echo "${dep_pkgs}" | sort -u)

log "debug" "dep_pkgs: ${dep_pkgs}"

if [ "${mode}" = "no-debug" ] ; then
  dep_pkgs=$(echo "${dep_pkgs}" | grep -v -- '-dbg$')
fi

log "debug" "Found dependencies with mode ${mode}:"

print_pkgs "${dep_pkgs}"

exit 0

