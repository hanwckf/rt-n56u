#!/bin/sh
# enforce limits on partition start sector and length

# Copyright (C) 2008-2012 Free Software Foundation, Inc.

# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 3 of the License, or
# (at your option) any later version.

# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.

# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.

. "${srcdir=.}/init.sh"; path_prepend_ ../parted
require_root_
require_xfs_
ss=$sector_size_

# On a 32-bit system, we must skip this test when $ss >= 4096.
# Otherwise, due to an inherent 32-bit-XFS limit, dd would fail to
# create the file of size > 16TiB
if test $(uname -m) != x86_64; then
  test $ss -le 2048 || skip_ 'this test works only on a 64-bit system'
fi

####################################################
# Create and mount a file system capable of dealing with >=2TB files.
# We must be able to create a file with an apparent length of 2TB or larger.
# It needn't be a large file system.
fs=fs_file
mp=`pwd`/mount-point
n=4096

# create an XFS file system
dd if=/dev/zero of=$fs bs=1MB count=2 seek=20 || fail=1
mkfs.xfs -f -q $fs || fail=1
mkdir "$mp" || fail=1

# Unmount upon interrupt, failure, etc., as well as upon normal completion.
cleanup_fn_() { cd "$test_dir_" && umount "$mp" > /dev/null 2>&1; }

# mount it
mount -o loop $fs "$mp" || fail=1
cd "$mp" || fail=1

dev=loop-file

do_mkpart()
{
  set +x # Turn off tracing; otherwise, we pollute stderr.
  start_sector=$1
  end_sector=$2
  # echo '********' $(echo $end_sector - $start_sector + 1 |bc)
  dd if=/dev/zero of=$dev bs=$ss count=2k seek=$end_sector 2> dd-err ||
    { cat dd-err 1>&2; return 1; }
  parted -s $dev mklabel $table_type &&
  parted -s $dev mkpart p xfs ${start_sector}s ${end_sector}s
}

# Specify the starting sector number and length in sectors,
# rather than start and end.
do_mkpart_start_and_len()
{
  set +x # Turn off tracing; otherwise, we pollute stderr.
  start_sector=$1
  len=$2
  end_sector=$(echo $start_sector + $len - 1|bc)
  do_mkpart $start_sector $end_sector
}

for table_type in msdos; do

# a partition length of 2^32-1 works.
end=$(echo $n+2^32-2|bc) || fail=1
do_mkpart $n $end || fail=1

# print the result
parted -s $dev unit s p > out 2>&1 || fail=1
sed -n "/^  *1  *$n/s/  */ /gp" out|sed "s/  *\$//" > k && mv k out || fail=1
echo " 1 ${n}s ${end}s 4294967295s primary" > exp || fail=1
compare exp out || fail=1

# a partition length of exactly 2^32 sectors provokes failure.
do_mkpart $n $(echo $n+2^32-1|bc) > err 2>&1
test $? = 1 || fail=1

bad_part_length()
{ echo "Error: partition length of $1 sectors exceeds the"\
  "$table_type-partition-table-imposed maximum of 4294967295"; }

# check for new diagnostic
bad_part_length 4294967296 > exp || fail=1
compare exp err || fail=1

# a partition length of 2^32+1 sectors must provoke failure.
do_mkpart $n $(echo $n+2^32|bc) > err 2>&1
test $? = 1 || fail=1

# check for new diagnostic
bad_part_length 4294967297 > exp || fail=1
compare exp err || fail=1

# =========================================================
# Now consider partition starting sector numbers.
bad_start_sector()
{ echo "Error: starting sector number, $1 exceeds the"\
  "$table_type-partition-table-imposed maximum of 4294967295"; }

# a partition start sector number of 2^32-1 works.
do_mkpart_start_and_len $(echo 2^32-1|bc) 1000 || fail=1

cat > exp <<EOF
Model:  (file)
Disk $dev: 4294970342s
Sector size (logical/physical): ${ss}B/${ss}B
Partition Table: $table_type
Disk Flags:

Number  Start        End          Size   Type     File system  Flags
 1      4294967295s  4294968294s  1000s  primary

EOF

# print the result
parted -s $dev unit s p > out 2>&1 || fail=1
sed "s/^Disk .*\($dev: [0-9][0-9]*s\)$/Disk \1/;s/ *$//" out > k \
    && mv k out || fail=1
compare exp out || fail=1

# a partition start sector number of 2^32 must fail
do_mkpart_start_and_len $(echo 2^32|bc) 1000 > err 2>&1
test $? = 1 || fail=1

# check for new diagnostic
bad_start_sector 4294967296 > exp || fail=1
compare exp err || fail=1

# a partition start sector number of 2^32+1 must fail, too.
do_mkpart_start_and_len $(echo 2^32+1|bc) 1000 > err 2>&1
test $? = 1 || fail=1

# check for new diagnostic
bad_start_sector 4294967297 > exp || fail=1
compare exp err || fail=1

done

Exit $fail
