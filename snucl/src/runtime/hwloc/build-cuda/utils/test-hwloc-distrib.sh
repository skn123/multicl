#!/bin/sh
#-*-sh-*-

#
# Copyright © 2009 CNRS
# Copyright © 2009-2014 Inria.  All rights reserved.
# Copyright © 2009 Université Bordeaux 1
# See COPYING in top-level directory.
#

HWLOC_top_builddir="/home/aaji/mpich-gpu-merge.git/src/hwloc/build-cuda"
distrib="$HWLOC_top_builddir/utils/hwloc-distrib"
HWLOC_top_srcdir="/home/aaji/mpich-gpu-merge.git/src/hwloc"

HWLOC_PLUGINS_PATH=${HWLOC_top_builddir}/src
export HWLOC_PLUGINS_PATH

: ${TMPDIR=/tmp}
{
  tmp=`
    (umask 077 && mktemp -d "$TMPDIR/fooXXXXXX") 2>/dev/null
  ` &&
  test -n "$tmp" && test -d "$tmp"
} || {
  tmp=$TMPDIR/foo$$-$RANDOM
  (umask 077 && mkdir "$tmp")
} || exit $?
file="$tmp/test-hwloc-distrib.output"

set -e
(
  $distrib --if synthetic --input "2 2 2" 2
  echo
  $distrib --if synthetic --input "2 2 2" 4
  echo
  $distrib --if synthetic --input "2 2 2" 8
  echo
  $distrib --if synthetic --input "2 2 2" 13
  echo
  $distrib --if synthetic --input "2 2 2" 16
  echo
  $distrib --if synthetic --input "3 3 3" 4
  echo
  $distrib --if synthetic --input "3 3 3" 4 --single
  echo
  $distrib --if synthetic --input "3 3 3" 4 --reverse
  echo
  $distrib --if synthetic --input "3 3 3" 4 --reverse --single
) > "$file"
diff -u $HWLOC_top_srcdir/utils/test-hwloc-distrib.output "$file"
rm -rf "$tmp"
