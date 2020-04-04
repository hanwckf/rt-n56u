#! /usr/bin/perl
eval "exec /usr/bin/env perl -w -S $0 $@"
    if 0;
# Copyright (C) 2000, 2001, 2002, 2003, 2004, 2005 Red Hat, Inc.
# Written by Ulrich Drepper <drepper@redhat.com>, 2000.

# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License version 2 as
# published by the Free Software Foundation.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, see <http://www.gnu.org/licenses/>.

for ($cnt = 0; $cnt <= $#ARGV; ++$cnt) {
  $relent = 0;
  $relsz = 0;
  $relcount = 0;
  $pltrelsz = 0;
  $extplt = 0;
  $users = 0;

  open (READLINK, "readlink -f $ARGV[$cnt] |") || die "cannot open readlink";
  while (<READLINK>) {
        chop;
        $fullpath = $_;
  }
  close (READLINK);

  open (READELF, "eu-readelf -d $ARGV[$cnt] |") || die "cannot open $ARGV[$cnt]";
  while (<READELF>) {
    chop;
    if (/.* RELA?ENT *([0-9]*).*/) {
      $relent = $1 + 0;
    } elsif (/.* RELA?SZ *([0-9]*).*/) {
      $relsz = $1 + 0;
    } elsif (/.* RELA?COUNT *([0-9]*).*/) {
      $relcount = $1 + 0;
    } elsif (/.* PLTRELSZ *([0-9]*).*/) {
      $pltrelsz = $1 + 0;
    }
  }
  close (READELF);

  open (READELF, "eu-readelf -r $ARGV[$cnt] | sed '/'.gnu.conflict'/,/^\$/d' |") || die "cannot open $ARGV[$cnt]";
  while (<READELF>) {
    chop;
    if (/.*JU?MP_SLOT *0+ .*/) {
      ++$extplt;
    }
  }
  close (READELF);

  if (open (PRELINK, "/usr/sbin/prelink -p 2>/dev/null | fgrep \"$fullpath\" |")) {
    while (<PRELINK>) {
      ++$users;
    }
    close(PRELINK);
  } else {
    $users = -1;
  }

  printf ("%s: %d relocations, %d relative (%d%%), %d PLT entries, %d for local syms (%d%%)",
	  $ARGV[$cnt], $relent == 0 ? 0 : $relsz / $relent, $relcount,
	  $relent == 0 ? 0 : ($relcount * 100) / ($relsz / $relent),
	  $relent == 0 ? 0 : $pltrelsz / $relent,
	  $relent == 0 ? 0 : $pltrelsz / $relent - $extplt,
	  $relent == 0 ? 0 : (($pltrelsz / $relent - $extplt) * 100) / ($pltrelsz / $relent));
  if ($users >= 0) {
    printf(", %d users", $users);
  }
  printf("\n");
}
