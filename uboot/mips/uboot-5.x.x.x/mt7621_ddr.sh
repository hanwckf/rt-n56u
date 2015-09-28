#!/bin/sh

VAR=$4
DDR3=$(echo ${VAR##*DDR3})

if [ "$DDR3" = "$4" ]; then
ddr_param_offset=200
else
ddr_param_offset=96
fi

LINE_NUM=$(cat ./$3|sed -n "/$4/=")
LINE_NUM=$(echo $LINE_NUM + "1"|bc)

if [ "$5" = "IN_NAND" ]; then
offset=$ddr_param_offset
sed -n "${LINE_NUM}p" ./$3|xxd -r -c 32|dd bs=1 count=32 seek=$offset of=$2 conv=notrunc
offset=$(echo $offset + "32"|bc)
LINE_NUM=$(echo $LINE_NUM + "1"|bc)
sed -n "${LINE_NUM}p" ./$3|xxd -r -c 32|dd bs=1 count=32 seek=$offset of=$2 conv=notrunc
offset=$(echo $offset + "32"|bc)
LINE_NUM=$(echo $LINE_NUM + "1"|bc)
sed -n "${LINE_NUM}p" ./$3|xxd -r -c 32|dd bs=1 count=32 seek=$offset of=$2 conv=notrunc
else
offset=$(echo "(($(stat -c %s uboot.bin)+$ddr_param_offset))" |bc)
sed -n "${LINE_NUM}p" ./$3|xxd -r -c 32|dd bs=1 count=32 seek=$offset of=$2 conv=notrunc
LINE_NUM=$(echo $LINE_NUM + "1"|bc)
offset=$(echo $offset + "32"|bc)
sed -n "${LINE_NUM}p" ./$3|xxd -r -c 32|dd bs=1 count=32 seek=$offset of=$2 conv=notrunc
LINE_NUM=$(echo $LINE_NUM + "1"|bc)
offset=$(echo $offset + "32"|bc)
sed -n "${LINE_NUM}p" ./$3|xxd -r -c 32|dd bs=1 count=32 seek=$offset of=$2 conv=notrunc
fi

