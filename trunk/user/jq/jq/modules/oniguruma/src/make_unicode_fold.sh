#!/bin/sh

TMP=gperf.tmp
GPERF_OPT='-n -C -T -c -t -j1 -L ANSI-C '

./make_unicode_fold_data.py > unicode_fold_data.c

gperf ${GPERF_OPT} -F,-1,0 -N unicode_unfold_key unicode_unfold_key.gperf > ${TMP}
./gperf_unfold_key_conv.py < ${TMP} > unicode_unfold_key.c

gperf ${GPERF_OPT} -F,-1 -N unicode_fold1_key unicode_fold1_key.gperf > ${TMP}
./gperf_fold_key_conv.py 1 < ${TMP} > unicode_fold1_key.c

gperf ${GPERF_OPT} -F,-1 -N unicode_fold2_key unicode_fold2_key.gperf > ${TMP}
./gperf_fold_key_conv.py 2 < ${TMP} > unicode_fold2_key.c

gperf ${GPERF_OPT} -F,-1 -N unicode_fold3_key unicode_fold3_key.gperf > ${TMP}
./gperf_fold_key_conv.py 3 < ${TMP} > unicode_fold3_key.c

rm -f ${TMP}

exit 0
