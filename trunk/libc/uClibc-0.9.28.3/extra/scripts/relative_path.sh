#! /bin/sh
# This script computes a relative pathname from $1 to $2
# They are assumed to not contain . or .. pathname components,
# but if both directories exist and cd/pwd canonicalizes pathnames,
# this shouldn't matter.  PWD_CMD may be set to some pwd command that does.

# Copyright 2003 Alexandre Oliva <aoliva@redhat.com>

# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.

# This program is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
# General Public License for more details.

# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software Foundation, Inc.,
# 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA


from=`(cd $1 > /dev/null && ${PWD_CMD-pwd} || echo $1) 2>/dev/null | sed 's,//*,/,g;s,/*$,,'`
target=`(cd $2 > /dev/null && ${PWD_CMD-pwd} || echo $2) 2>/dev/null | sed 's,//*,/,g;s,/*$,,'`

case $from in /* | "") ;; *) from=`${PWD_CMD-pwd}`/$from ;; esac
case $target in /* | "") ;; *) target=`${PWD_CMD-pwd}`/$target ;; esac

case $target in
"$from" | "$from/"*)
  dots=`echo $from | sed s,.,.,g`
  echo $target | sed "s,^$dots/*,,;s,[^/]$,&/,"
  exit 0
  ;;
esac

case $from in
"$target/"*)
  dots=`echo $target | sed s,.,.,g`
  echo $from/ | sed "s,^$dots/*,,;s,[^/]$,&/,;s,[^/]*/*,../,g;s,[^/]$,&/,"
  exit 0
  ;;
esac

prefix=`echo $from///$target | sed 's,\(\(/[^/]*\)*\).*///\1.*,\1,'`
dots=`echo $prefix | sed s,.,.,g`
from=`echo $from | sed "s,^$dots,,"`
target=`echo $target | sed "s,^$dots,,"`

from=`echo $from | sed 's,[^/][^/]*,..,g;s,.$,&/,'`
echo ${from}$target/

exit 0
