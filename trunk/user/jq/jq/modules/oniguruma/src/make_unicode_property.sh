#!/bin/sh

NAME=unicode_property_data
TMP=gperf.tmp
#GPERF_OPT='-P -Q prop_name_pool -C -c -t -j1 -L ANSI-C --ignore-case'
GPERF_OPT='-T -C -c -t -j1 -L ANSI-C --ignore-case'

./make_unicode_property_data.py > ${NAME}.gperf
./make_unicode_property_data.py -posix > ${NAME}_posix.gperf

gperf ${GPERF_OPT} -N unicode_lookup_property_name --output-file ${TMP} ${NAME}.gperf
sed -e 's/^#line.*$//g' ${TMP} > ${NAME}.c
gperf ${GPERF_OPT} -N unicode_lookup_property_name --output-file ${TMP} ${NAME}_posix.gperf
sed -e 's/^#line.*$//g' ${TMP} > ${NAME}_posix.c

rm -f ${NAME}.gperf ${NAME}_posix.gperf ${TMP}

exit 0
