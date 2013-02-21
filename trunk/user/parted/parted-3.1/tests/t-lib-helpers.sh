# Functions sourced via testing framework.

getlimits_()
{
    eval $(getlimits)
    test "$INT_MAX" ||
    error_ "Error running getlimits"
}

require_acl_()
{
  getfacl --version < /dev/null > /dev/null 2>&1 \
    && setfacl --version < /dev/null > /dev/null 2>&1 \
      || skip_ "This test requires getfacl and setfacl."

  id -u bin > /dev/null 2>&1 \
    || skip_ "This test requires a local user named bin."
}

require_hfs_()
{
  mkfs.hfs 2>&1 | grep '^usage:' \
    || skip_ "This test requires HFS support."
}

# Skip this test if we're not in SELinux "enforcing" mode.
require_selinux_enforcing_()
{
  test "$(getenforce)" = Enforcing \
    || skip_ "This test is useful only with SELinux in Enforcing mode."
}


require_openat_support_()
{
  # Skip this test if your system has neither the openat-style functions
  # nor /proc/self/fd support with which to emulate them.
  test -z "$CONFIG_HEADER" \
    && skip_ 'internal error: CONFIG_HEADER not defined'

  _skip=yes
  grep '^#define HAVE_OPENAT' "$CONFIG_HEADER" > /dev/null && _skip=no
  test -d /proc/self/fd && _skip=no
  if test $_skip = yes; then
    skip_ 'this system lacks openat support'
  fi
}

require_ulimit_()
{
  ulimit_works=yes
  # Expect to be able to exec a program in 10MB of virtual memory,
  # but not in 20KB.  I chose "date".  It must not be a shell built-in
  # function, so you can't use echo, printf, true, etc.
  # Of course, in coreutils, I could use $top_builddir/src/true,
  # but this should be able to work for other projects, too.
  ( ulimit -v 10000; date ) > /dev/null 2>&1 || ulimit_works=no
  ( ulimit -v 20;    date ) > /dev/null 2>&1 && ulimit_works=no

  test $ulimit_works = no \
    && skip_ "this shell lacks ulimit support"
}

require_readable_root_()
{
  test -r / || skip_ "/ is not readable"
}

# Skip the current test if strace is not available or doesn't work
# with the named syscall.  Usage: require_strace_ unlink
require_strace_()
{
  test $# = 1 || framework_failure

  strace -V < /dev/null > /dev/null 2>&1 ||
    skip_ 'no strace program'

  strace -qe "$1" echo > /dev/null 2>&1 ||
    skip_ 'strace -qe "'"$1"'" does not work'
}

# Require a controlling input `terminal'.
require_controlling_input_terminal_()
{
  tty -s || have_input_tty=no
  test -t 0 || have_input_tty=no
  if test "$have_input_tty" = no; then
    skip_ 'requires controlling input terminal
This test must have a controlling input "terminal", so it may not be
run via "batch", "at", or "ssh".  On some systems, it may not even be
run in the background.'
  fi
}

uid_is_privileged_()
{
  # Make sure id -u succeeds.
  my_uid=$(id -u) \
    || { echo "$0: cannot run \`id -u'" 1>&2; return 1; }

  # Make sure it gives valid output.
  case $my_uid in
    0) ;;
    *[!0-9]*)
      echo "$0: invalid output (\`$my_uid') from \`id -u'" 1>&2
      return 1 ;;
    *) return 1 ;;
  esac
}

get_process_status_()
{
  sed -n '/^State:[	 ]*\([[:alpha:]]\).*/s//\1/p' /proc/$1/status
}

# Convert an ls-style permission string, like drwxr----x and -rw-r-x-wx
# to the equivalent chmod --mode (-m) argument, (=,u=rwx,g=r,o=x and
# =,u=rw,g=rx,o=wx).  Ignore ACLs.
rwx_to_mode_()
{
  case $# in
    1) rwx=$1;;
    *) echo "$0: wrong number of arguments" 1>&2
      echo "Usage: $0 ls-style-mode-string" 1>&2
      return;;
  esac

  case $rwx in
    [ld-][rwx-][rwx-][rwxsS-][rwx-][rwx-][rwxsS-][rwx-][rwx-][rwxtT-]) ;;
    [ld-][rwx-][rwx-][rwxsS-][rwx-][rwx-][rwxsS-][rwx-][rwx-][rwxtT-][+.]) ;;
    *) echo "$0: invalid mode string: $rwx" 1>&2; return;;
  esac

  # Perform these conversions:
  # S  s
  # s  xs
  # T  t
  # t  xt
  # The `T' and `t' ones are only valid for `other'.
  s='s/S/@/;s/s/x@/;s/@/s/'
  t='s/T/@/;s/t/x@/;s/@/t/'

  u=`echo $rwx|sed 's/^.\(...\).*/,u=\1/;s/-//g;s/^,u=$//;'$s`
  g=`echo $rwx|sed 's/^....\(...\).*/,g=\1/;s/-//g;s/^,g=$//;'$s`
  o=`echo $rwx|sed 's/^.......\(...\).*/,o=\1/;s/-//g;s/^,o=$//;'$s';'$t`
  echo "=$u$g$o"
}

require_selinux_()
{
  case `ls -Zd .` in
    '? .'|'unlabeled .')
      skip_ "this system (or maybe just" \
        "the current file system) lacks SELinux support"
    ;;
  esac
}

very_expensive_()
{
  if test "$RUN_VERY_EXPENSIVE_TESTS" != yes; then
    skip_ 'very expensive: disabled by default
This test is very expensive, so it is disabled by default.
To run it anyway, rerun make check with the RUN_VERY_EXPENSIVE_TESTS
environment variable set to yes.  E.g.,

  env RUN_VERY_EXPENSIVE_TESTS=yes make check
'
  fi
}

expensive_()
{
  if test "$RUN_EXPENSIVE_TESTS" != yes; then
    skip_ 'expensive: disabled by default
This test is relatively expensive, so it is disabled by default.
To run it anyway, rerun make check with the RUN_EXPENSIVE_TESTS
environment variable set to yes.  E.g.,

  env RUN_EXPENSIVE_TESTS=yes make check
'
  fi
}

require_root_()
{
  uid_is_privileged_ || skip_ "must be run as root"
  NON_ROOT_USERNAME=${NON_ROOT_USERNAME=nobody}
  NON_ROOT_GROUP=${NON_ROOT_GROUP=$(id -g $NON_ROOT_USERNAME)}
}

skip_if_root_() { uid_is_privileged_ && skip_ "must be run as non-root"; }
error_() { echo "$0: $@" 1>&2; Exit 1; }
framework_failure() { error_ 'failure in testing framework'; }

# Set `groups' to a space-separated list of at least two groups
# of which the user is a member.
require_membership_in_two_groups_()
{
  test $# = 0 || framework_failure

  groups=${COREUTILS_GROUPS-`(id -G || /usr/xpg4/bin/id -G) 2>/dev/null`}
  case "$groups" in
    *' '*) ;;
    *) skip_ 'requires membership in two groups
this test requires that you be a member of more than one group,
but running `id -G'\'' either failed or found just one.  If you really
are a member of at least two groups, then rerun this test with
COREUTILS_GROUPS set in your environment to the space-separated list
of group names or numbers.  E.g.,

  env COREUTILS_GROUPS='users cdrom' make check

'
     ;;
  esac
}

# Is /proc/$PID/status supported?
require_proc_pid_status_()
{
    sleep 2 &
    local pid=$!
    sleep .5
    grep '^State:[	 ]*[S]' /proc/$pid/status > /dev/null 2>&1 ||
    skip_ "/proc/$pid/status: missing or 'different'"
    kill $pid
}

# Does the current (working-dir) file system support sparse files?
require_sparse_support_()
{
  test $# = 0 || framework_failure
  # Test whether we can create a sparse file.
  # For example, on Darwin6.5 with a file system of type hfs, it's not possible.
  # NTFS requires 128K before a hole appears in a sparse file.
  t=sparse.$$
  dd bs=1 seek=128K of=$t < /dev/null 2> /dev/null
  set x `du -sk $t`
  kb_size=$2
  rm -f $t
  if test $kb_size -ge 128; then
    skip_ 'this file system does not support sparse files'
  fi
}

mkfifo_or_skip_()
{
  test $# = 1 || framework_failure
  if ! mkfifo "$1"; then
    # Make an exception of this case -- usually we interpret framework-creation
    # failure as a test failure.  However, in this case, when running on a SunOS
    # system using a disk NFS mounted from OpenBSD, the above fails like this:
    # mkfifo: cannot make fifo `fifo-10558': Not owner
    skip_ 'NOTICE: unable to create test prerequisites'
  fi
}

# Disable the current test if the working directory seems to have
# the setgid bit set.
skip_if_setgid_()
{
  setgid_tmpdir=setgid-$$
  (umask 77; mkdir $setgid_tmpdir)
  perms=$(stat --printf %A $setgid_tmpdir)
  rmdir $setgid_tmpdir
  case $perms in
    drwx------);;
    drwxr-xr-x);;  # Windows98 + DJGPP 2.03
    *) skip_ 'this directory has the setgid bit set';;
  esac
}

skip_if_mcstransd_is_running_()
{
  test $# = 0 || framework_failure

  # When mcstransd is running, you'll see only the 3-component
  # version of file-system context strings.  Detect that,
  # and if it's running, skip this test.
  __ctx=$(stat --printf='%C\n' .) || framework_failure
  case $__ctx in
    *:*:*:*) ;; # four components is ok
    *) # anything else probably means mcstransd is running
        skip_ "unexpected context '$__ctx'; turn off mcstransd" ;;
  esac
}

# Skip the current test if umask doesn't work as usual.
# This test should be run in the temporary directory that ends
# up being removed via the trap commands.
working_umask_or_skip_()
{
  umask 022
  touch file1 file2
  chmod 644 file2
  perms=`ls -l file1 file2 | sed 's/ .*//' | uniq`
  rm -f file1 file2

  case $perms in
  *'
  '*) skip_ 'your build directory has unusual umask semantics'
  esac
}

emit_superuser_warning()
{
  uid=`id -u` || uid=1
  test "$uid" != 0 &&
    echo 'WARNING: You are not superuser.  Watch out for permissions.' || :
}

require_mdadm_()
{
  mdadm --version || skip_ "find mdadm executable"
}

# Will look for an md number that is not in use and create a md device with
# that number.  If the system has more than 9 md devices, it will fail.
mdadm_create_linear_device_()
{
  lo_dev=$1
  mdd=$G_dev_/md0
  for i in 0 1 2 3 4 5 6 7 8 9 ; do
    mdd=$G_dev_/md$i
    mdadm  --create --force $mdd --level=linear --raid-devices=1 $lo_dev \
	> /dev/null 2>&1 \
      && break

    if [ $i -eq 9 ]; then echo $mdd ; return 1 ; fi
  done

  echo $mdd
  return 0
}

# Often, when parted cannot use the specified size or start/endpoints
# of a partition, it outputs a warning or error like this:
#
# Error: You requested a partition from 512B to 50.7kB (...).
# The closest location we can manage is 17.4kB to 33.8kB (...).
#
# But those numbers depend on sector size, so
# replace the specific values with place-holders,
# so tests do not depend on sector size.
normalize_part_diag_()
{
  local file=$1
  sed 's/ [0-9.k]*B to [0-9.k]*B (sectors .*$/ X to Y./' $file > $file.t \
    && mv $file.t $file && return 0
  return 1
}

require_xfs_()
{
  mkfs.xfs -V || skip_ "this test requires XFS support"
}

require_dvhtool_()
{
  dvhtool --help \
    || skip_ 'dvhtool is required for this test'
}

# Helper function: wait 2s (via .1s increments) for FILE to appear.
# Usage: wait_for_dev_to_appear_ /dev/sdg
# Return 0 upon success, 1 upon failure.
wait_for_dev_to_appear_()
{
  local file=$1
  local i=0
  local incr=1
  while :; do
    ls "$file" > /dev/null 2>&1 && return 0
    sleep .1 2>/dev/null || { sleep 1; incr=10; }
    i=$(expr $i + $incr); test $i = 20 && break
  done
  return 1
}

# Like the above, but don't hard-code the max timeout.
wait_for_dev_to_disappear_()
{
  local file=$1
  local n_sec=$2
  local i=0
  local incr=1
  while :; do
    ls "$file" > /dev/null 2>&1 || return 0
    sleep .1 2>/dev/null || { sleep 1; incr=10; }
    i=$(expr $i + $incr); test $i -ge $(expr $n_sec \* 10) && break
  done
  return 1
}

device_mapper_required_()
{
  . "$abs_top_srcdir/tests/lvm-utils.sh" \
       || fail_ "device mapper setup failed"
}
