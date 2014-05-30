#!/bin/sh
#
#   tracemeld.sh foo.ref.txt foo.trace
#
MELD=meld
TEMPFILE=`mktemp`
apitrace dump --call-nos=no "$2" > "$TEMPFILE"
$MELD "$1" "$TEMPFILE"
rm -f "$TEMPFILE"
