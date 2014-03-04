#!/bin/sh
#-*-sh-*-

#
# Copyright © 2009-2013 Inria.  All rights reserved.
# Copyright © 2009, 2011 Université Bordeaux 1
# See COPYING in top-level directory.
#

HWLOC_top_builddir="/home/aaji/mpich-gpu-merge.git/src/hwloc/build-cuda"
lstopo="$HWLOC_top_builddir/utils/lstopo-no-graphics"

HWLOC_PLUGINS_PATH=${HWLOC_top_builddir}/src
export HWLOC_PLUGINS_PATH

HWLOC_DEBUG_FAKE_COMPONENT=1
export HWLOC_DEBUG_FAKE_COMPONENT

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
file="$tmp/test-fake-plugin.output"

set -e

$lstopo > $file

grep "fake component instantiated" $file

rm -rf "$tmp"
