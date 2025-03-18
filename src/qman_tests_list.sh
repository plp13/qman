#!/bin/bash
# List all configured tests (used by meson)

FILE="qman_tests.c"
MATCH="^\s*add_test("
LTRIM="s/\s*add_test(//g"
RTRIM="s/);//g"

exit_on_error() {
  if [ "X${1}" != "X0" ]
  then
    echo "Command failed"
    exit ${1}
  fi
}

cd "$( dirname "${BASH_SOURCE[0]}" )"
exit_on_error $?

grep "${MATCH}" "${FILE}" | sed "${LTRIM}" | sed "${RTRIM}"
exit_on_error $?

exit 0

