#! /bin/sh

HOSTNAME=tiny9300
FILE=random-4k.bin

cat $FILE | ./af_bin | ./fcs_encode | ./nrzi_encode | ./fx25_encode | ./af_i32 | ./modem_send $HOSTNAME | ./fx25_decode | ./rs_decode | ./nrzi_decode2 | ./fcs_decode | ./af_decode > test.bin

cmp $FILE test.bin
