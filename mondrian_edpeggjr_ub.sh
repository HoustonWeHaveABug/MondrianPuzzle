#!/bin/bash
if [ $# -ne 4 ]
then
	echo "Usage: $0 <order> <rotate_flag> <options_lo> <options_hi>"
	exit 1
fi
make -f mondrian.make
if [ $2 -eq 0 ]
then
	UB=`echo "$1/l($1)+1" | bc -l | sed "s/\..*//g"`
else
	UB=`echo "$1/l($1)+4" | bc -l | sed "s/\..*//g"`
fi
echo 0 $1 $1 $2 $UB 0 $3 $4 0 | ./mondrian
exit 0
