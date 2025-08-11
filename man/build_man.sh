#!/usr/bin/env bash
# Build the manual page from its Markdown source, using pandoc

MD_FILE="qman.1.md"
MAN_FILE="qman.1"

exit_on_error() {
  if [ "X${1}" != "X0" ]
  then
    echo "Command failed"
    exit ${1}
  fi
}

cd "$( dirname "${BASH_SOURCE[0]}" )"
exit_on_error $?

echo "Removing old '${MAN_FILE}'"
rm -f ${MAN_FILE}
exit_on_error $?

echo "Generating new '${MAN_FILE}' from '${MD_FILE}'"
pandoc ${MD_FILE} -s -t man -o ${MAN_FILE}
exit_on_error $?

exit 0
