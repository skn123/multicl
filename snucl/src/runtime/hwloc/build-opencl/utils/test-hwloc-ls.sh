#!/bin/sh
#-*-sh-*-

#
# Copyright © 2009 CNRS
# Copyright © 2009-2013 Inria.  All rights reserved.
# Copyright © 2009, 2011 Université Bordeaux 1
# See COPYING in top-level directory.
#

HWLOC_top_builddir="/home/aaji/hwloc/build-opencl"
ls="$HWLOC_top_builddir/utils/lstopo-no-graphics"
HWLOC_top_srcdir="/home/aaji/hwloc"

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
file="$tmp/test-hwloc-ls.output"

unset DISPLAY

set -e
$ls
(
  $ls > $tmp/test.console
  $ls -v > $tmp/test.console_verbose
  $ls -c -v > $tmp/test.cpuset_verbose
  $ls --taskset -v > $tmp/test.taskset
  $ls --merge > $tmp/test.merge

#  $ls --no-io > $tmp/test.no-io
#  $ls --no-bridges > $tmp/test.no-bridges
#  $ls --whole-io > $tmp/test.whole-io
#  $ls -v --whole-io > $tmp/test.wholeio_verbose

  $ls --whole-system > $tmp/test.whole-system
  $ls --ps > $tmp/test.
  $ls $tmp/test.txt
  $ls $tmp/test.fig
  $ls $tmp/test.xml
  HWLOC_NO_LIBXML_EXPORT=1 $ls $tmp/test.xml
  $ls --input "ma:1 no:2 so:1 ca:2 2" $tmp/test.synthetic
) > "$file"
diff -u $HWLOC_top_srcdir/utils/test-hwloc-ls.output "$file"
rm -rf "$tmp"
