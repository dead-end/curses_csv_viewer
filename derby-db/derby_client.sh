#!/bin/sh

set -u

echo "$@" > .derby.sql

result=$(ij -p ij.properties .derby.sql | grep -v -e '^-*$' -e '^ij>' -e '^ij version' -e '^;$')

echo "$result" | ./ccsvv -s -d '|' || echo "$result"
