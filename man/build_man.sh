#!/bin/bash
# Build the manual page from its Markdown source, using pandoc

MD_FILE="qman.1.md"
MAN_FILE="qman.1"

rm -f ${MAN_FILE}
pandoc ${MD_FILE} -s -t man -o ${MAN_FILE}
exit $?
