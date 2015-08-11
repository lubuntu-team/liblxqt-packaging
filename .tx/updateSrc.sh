#!/bin/sh

LUPDATE_OPTIONS="-noobsolete"
LUPDATE_OPTIONS="${LUPDATE_OPTIONS} -locations none "
TS_FILE="../translations/source.ts"

LUPDATE=$(which lupdate-qt4 2>/dev/null)

[ -z "${LUPDATE}" ] && LUPDATE=$(which lupdate 2>/dev/null)

if [ -z "${LUPDATE}" ]; then
    echo "The lupdate program not found."
    exit 1;
fi

${LUPDATE} ${LUPDATE_OPTIONS} .. -ts ${TS_FILE}

