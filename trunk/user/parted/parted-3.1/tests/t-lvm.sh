# Put lvm-related utilities here.
# This file is sourced from test infrastructure.

# Copyright (C) 2007, 2008, 2010 Red Hat, Inc. All rights reserved.
#
# This copyrighted material is made available to anyone wishing to use,
# modify, copy, or redistribute it subject to the terms and conditions
# of the GNU General Public License v.2.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software Foundation,
# Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

export LVM_SUPPRESS_FD_WARNINGS=1

ME=$(basename "$0")
warn() { echo >&2 "$ME: $@"; }

unsafe_losetup_()
{
  f=$1

  test -n "$G_dev_" \
    || error "Internal error: unsafe_losetup_ called before init_root_dir_"

  # Iterate through $G_dev_/loop{,/}{0,1,2,3,4,5,6,7,8,9}
  for slash in '' /; do
    for i in 0 1 2 3 4 5 6 7 8 9; do
      dev=$G_dev_/loop$slash$i
      losetup $dev > /dev/null 2>&1 && continue;
      losetup "$dev" "$f" > /dev/null && { echo "$dev"; return 0; }
      break
    done
  done

  return 1
}

loop_setup_()
{
  file=$1
  dd if=/dev/zero of="$file" bs=1M count=1 seek=1000 > /dev/null 2>&1 \
    || { warn "loop_setup_ failed: Unable to create tmp file $file"; return 1; }

  # NOTE: this requires a new enough version of losetup
  dev=$(unsafe_losetup_ "$file" 2>/dev/null) \
    || { warn "loop_setup_ failed: Unable to create loopback device"; return 1; }

  echo "$dev"
  return 0;
}

# set up private /dev and /etc
lvm_init_root_dir_()
{
  test -z "$test_dir_" \
    && skip_ "Internal error: called lvm_init_root_dir_ before" \
      "defining \$test_dir_"

  # Define these two globals.
  G_root_=$test_dir_/root
  G_dev_=$G_root_/dev

  export LVM_SYSTEM_DIR=$G_root_/etc
  export DM_DEV_DIR=$G_dev_

  # Only the first caller does anything.
  mkdir -p $G_root_/etc $G_dev_ $G_dev_/mapper $G_root_/lib
  for i in 0 1 2 3 4 5 6 7; do
    mknod $G_root_/dev/loop$i b 7 $i
  done
  for i in $abs_top_builddir/dmeventd/mirror/*.so \
           $abs_top_builddir/dmeventd/snapshot/*.so
  do
    # NOTE: This check is necessary because the loop above will give us the
    # value "$abs_top_builddir/dmeventd/mirror/*.so" if no files ending in
    # 'so' exist.  This is the best way I could quickly determine to skip
    # over this bogus value.
    if [ -f $i ]; then
      echo Setting up symlink from $i to $G_root_/lib
      ln -s $i $G_root_/lib
    fi
  done
  cat > $G_root_/etc/lvm.conf <<-EOF
  devices {
    dir = "$G_dev_"
    scan = "$G_dev_"
    filter = [ "a/loop/", "a/mirror/", "a/mapper/", "r/.*/" ]
    cache_dir = "$G_root_/etc"
    sysfs_scan = 0
  }
  log {
    verbose = $verboselevel
    syslog = 0
    indent = 1
  }
  backup {
    backup = 0
    archive = 0
  }
  global {
    library_dir = "$G_root_/lib"
  }
EOF
}
