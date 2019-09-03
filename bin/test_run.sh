#/bin/sh

################################################################################
# The script is used to run a smoke test for the ccsvv program. The script is
# started with no argument. In this case the script starts itself again with a
# "signal" argument in the background and then starts the ccsvv program.
# The background skript is polling for a pid of the ccsvv program and if it
# finds on, it sends a SIGUSR1 (10) signal to the ccsvv. The ccsvv has a signal
# handler that waits for SIGUSR1 signals and terminates ccsvv with an exit
# code of 0. So calling:
# 
#   docker run -it image sh /tmp/test_run.sh
#
# starts ccsvv, which will be terminated after one second.
################################################################################

set -u

################################################################################
# error function
################################################################################

do_exit () { 
   echo "ERROR: ${1}" 
   exit 1
}

################################################################################
# The function is looking for a pid of a ccsvv program in a loop. If it finds 
# one, it sends a SIGUSR1 (10) signal.
################################################################################

do_signal() {

  for count in 1 2 3 4 5 ; do

    sleep 1

    pid=$(ps -C ccsvv -o pid=)

    #
    # If we have a pid we send the signal
    #
    if [ -n "${pid}" ]; then
      kill -10 ${pid} || do_exit "kill failed!"
      exit 0
    fi
  done

  do_exit "No process found!"
}

################################################################################
# In the main part, we check if the script was called with an argument. If so,
# we check if the argument is signal. If the script is not called with an
# argument $1 is not set and "set -u" results in an error, so we have nested 
# if's.
################################################################################

if [ "${#}" = "1" ] ; then
  if [ "${1}" = "signal" ] ; then
    do_signal
  fi
fi

sh ${0} signal &

ccsvv -d : /etc/passwd

#
# No exit code => return the exit code from the last command
#
