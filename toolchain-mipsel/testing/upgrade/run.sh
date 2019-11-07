#!/bin/bash

CTNG=${CTNG-../../ct-ng}

current_tc=unknown
fails_tc=0
fails_total=0

fail()
{
    fails_tc=$[fails_tc + 1]
    fails_total=$[fails_total + 1]
}

finish()
{
    if [ "${fails_tc}" != 0 ]; then
        echo ">>>>> $current_tc: FAIL" >&2
    else
        echo ">>>>> $current_tc: PASS" >&2
    fi
    fails_tc=0
}

run_sample()
{
    local -A expect_set expect_unset
    local o v ln

    # Basename for logging
    exec {LOG}>"logs/${current_tc}.log"

    # Determine expected values
    while read ln; do
        case "${ln}" in
            "## "*"="*)
                ln=${ln#* }
                o=${ln%%=*}
                v=${ln#*=}
                expect_set[${o}]=${v}
                ;;
            "## "*" is not set")
                ln=${ln#* }
                o=${ln%% *}
                expect_unset[${o}]=1
                ;;
        esac
    done < "samples/${current_tc}.config"

    # Now run the upgrade
    echo ">>>> Running the config through an upgrade" >&${LOG}
    cp "samples/${current_tc}.config" .config
    ${CTNG} upgradeconfig >&${LOG} 2>&${LOG}
    echo >&${LOG}
    echo ">>>> Checking the config after the upgrade" >&${LOG}
    while read ln; do
        case "${ln}" in
            *"="*)
                o=${ln%%=*}
                v=${ln#*=}
                if [ "${expect_unset[${o}]+set}" = "set" ]; then
                    echo "Expect ${o} to be unset" >&${LOG}
                    echo "Actual value of ${o}: ${v}" >&${LOG}
                    fail
                elif [ "${expect_set[${o}]+set}" = "set" ]; then
                    if [ "${expect_set[${o}]}" != "${v}" ]; then
                        echo "Expect value of ${o}: ${expect_set[${o}]}" >&${LOG}
                        echo "Actual value of ${o}: ${v}" >&${LOG}
                        fail
                    else
                        echo "Matched value of ${o}: ${v}" >&${LOG}
                    fi
                fi
                unset expect_set[${o}]
                unset expect_unset[${o}]
                ;;
            "# "*" is not set")
                ln=${ln#* }
                o=${ln%% *}
                if [ "${expect_set[${o}]+set}" = "set" ]; then
                    echo "Expect value of ${o}: ${expect_set[${o}]}" >&${LOG}
                    echo "Actual ${o} is unset" >&${LOG}
                    fail
                elif [ "${expect_unset[${o}]+set}" = "set" ]; then
                    echo "Matched unset ${o}" >&${LOG}
                fi
                unset expect_set[${o}]
                unset expect_unset[${o}]
                ;;
        esac
    done < .config
    for o in "${!expect_set[@]}"; do
        echo "Expect value of ${o}: ${expect_set[${o}]}" >&${LOG}
        echo "Variable ${o} not present" >&${LOG}
        fail
    done
    for o in "${!expect_unset[@]}"; do
        echo "Expect ${o} being unset" >&${LOG}
        echo "Variable ${o} not present" >&${LOG}
        fail
    done
    mv .config "logs/${current_tc}.config"
    mv .config.before-olddefconfig "logs/${current_tc}.config.before-olddefconfig"
    rm -rf .config.before-upgrade
    exec {LOG}>&-
    finish
}

mkdir -p logs

# Non-sample-specific tests

# Verify that no options have been retired since the stored known configuration.
current_tc="options-set"
exec {LOG}>"logs/global.log"
curver=`sed -n 's,export CT_CONFIG_VERSION_CURRENT=,,p' ${CTNG}`
if [ -z "${curver}" ]; then
    echo "Cannot determine config version" >&${LOG}
    fail
else
    grep -hr '^\(menu\)\?config ' "${dirs[@]}" ../../config | \
        grep -v '^Binary ' | \
        sed 's,^.* ,CT_,' | LANG=C sort | uniq > logs/current-kconfig-list
    if [ ! -r "kconfig-list/${curver}" ]; then
        echo "No saved kconfig data for version ${curver}" >&${LOG}
        if [ -r "kconfig-list/$[ curver - 1 ]" ]; then
            echo "Comparing with previous version $[ curver - 1 ]"
            echo "Verify that the following options are handled:"
            diff -U 10000 "kconfig-list/$[ curver - 1 ]" logs/current-kconfig-list | \
                grep '^-CT_' || true
            echo "Then rename logs/current-kconfig-list to kconfig-list/${curver}"
        fi >&${LOG}
        fail
    else
        diff -U 10000 "kconfig-list/${curver}" logs/current-kconfig-list | \
            grep '^-CT_' > logs/current-kconfig-retired || true
        nretired=`wc -l logs/current-kconfig-retired | sed 's/ .*//'`
        if [ "${nretired}" != "0" ]; then
            echo "${nretired} kconfig options have been removed without bumping the config version" >&${LOG}
            fail
        fi
    fi
fi
finish
exec {LOG}>&-

for i in samples/*.config; do
    current_tc=${i#samples/}
    current_tc=${current_tc%.config}
    run_sample
done

if [ "${fails_total}" != 0 ]; then
    exit 1
fi
exit 0
