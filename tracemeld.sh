#!/bin/sh
#
#   tracemeld.sh foo.ref.txt foo.trace
#
DIFFTOOL=${DIFFTOOL:-meld}
TEMPFILE=`mktemp`
apitrace dump --call-nos=no "$2" > "$TEMPFILE"
$DIFFTOOL "$1" "$TEMPFILE"
rm -f "$TEMPFILE"
