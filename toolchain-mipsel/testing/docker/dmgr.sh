#!/bin/bash

# Run from the directory containing this script
cd `dirname $0`

# Global return code (flags an error if any of the actions fail)
global_rc=0

msg()
{
    echo "INFO  :: $*" >&2
}

warn()
{
    echo "WARN  :: $*" >&2
}

error()
{
    echo "ERROR :: $*" >&2
    exit 1
}

usage()
{
    cat >&2 <<EOF
${1:+ERROR :: $1

}Usage: $0 [action] [containter] [args...]

Action is one of:

   build     Build or rebuild the specified containers.
   install   Install crosstool-NG in specified containers.
   sample    Build a sample or if no sample name specified, all.
   enter     Spawn a shell in the specified container.
   root      Spawn a root shell in the specified container.
   clean     Clean up in the specified container.

If a special container name 'all' is used, the action is performed
on all the containers.
EOF
    exit 1
}

do_cleanup()
{
    local d

    for d in "$@"; do
        [ -d "$d" ] || continue
        chmod -R a+w "$d"
        rm -rf "$d"
    done
}

# Build a docker container, store its ID.
action_build()
{
    local cntr=$1

    msg "Cleaning up previous runs for ${cntr}"
    do_cleanup ${cntr}/{build,install,xtools}
    msg "Building Docker container for ${cntr}"
set -x
    docker build --no-cache -t "ctng-${cntr}" --build-arg CTNG_GID=`id -g` --build-arg CTNG_UID=`id -u` "${cntr}"
}

# Common backend for enter/test
_dckr()
{
    local topdir=`cd ../.. && pwd`
    local cntr=$1
    local scmd prefix
    shift

    mkdir -p ${cntr}/{build,install,xtools}
    prefix="docker run --rm -i -t \
        -v `pwd`/common-scripts:/common-scripts:ro \
        -v ${topdir}:/crosstool-ng:ro \
        -v `pwd`/${cntr}/build:/home/ctng/work \
        -v `pwd`/${cntr}/install:/opt/ctng \
        -v `pwd`/${cntr}/xtools:/home/ctng/x-tools \
        -v $HOME/src:/home/ctng/src:ro \
        ctng-${cntr}"
    if [ -n "${AS_ROOT}" ]; then
        $prefix "$@"
    elif [ -n "$*" ]; then
        $prefix su -l ctng -c "$*"
    else
        $prefix su -l ctng
    fi
    if [ $? != 0 ]; then
	global_rc=1
    fi
}

# Run the test
action_install()
{
    local cntr=$1

    # The test assumes the top directory is bootstrapped, but clean.
    msg "Setting up crosstool-NG in ${cntr}"
    do_cleanup ${cntr}/build
    if ! _dckr "${cntr}" /common-scripts/ctng-install; then
	warn "Installation failed"
    elif !  _dckr "${cntr}" /common-scripts/ctng-test-basic; then
	warn "Basic tests failed"
    fi
}

# Run the test
action_sample()
{
    local cntr=$1
    shift

    msg "Building samples in ${cntr} [$@]"
    do_cleanup ${cntr}/build
    _dckr "${cntr}" /common-scripts/ctng-build-sample "$@"
}

# Enter the container using the same user account/environment as for testing.
action_enter()
{
    local cntr=$1
    shift

    msg "Entering ${cntr}"
    _dckr "${cntr}" "$@"
}

# Enter the container using the same user account/environment as for testing.
action_root()
{
    local cntr=$1

    msg "Entering ${cntr} as root"
    AS_ROOT=y _dckr "${cntr}" /bin/bash
}

# Clean up after test suite run
action_clean()
{
    local cntr=$1

    msg "Cleaning up after ${cntr}"
    do_cleanup ${cntr}/build
}

# Clean up after test suite run
action_distclean()
{
    local cntr=$1

    msg "Dist cleaning ${cntr}"
    do_cleanup ${cntr}/{build,install,xtools}
}

all_containers=`ls */Dockerfile | sed 's,/Dockerfile,,'`
action=$1
selected_containers=$2
shift 2
if [ "${selected_containers}" = "all" ]; then
    selected_containers="${all_containers}"
fi

case "${action}" in
    build|install|sample|enter|root|clean|distclean)
        for c in ${selected_containers}; do
            eval "action_${action} ${c} \"$@\""
        done
        ;;
    "")
        usage "No action specified."
        ;;
    *)
        usage "Unknown action ${action}."
        ;;
esac
if [ "${global_rc}" != 0 ]; then
    error "Some of the actions failed, see warnings above"
fi
exit ${global_rc}
