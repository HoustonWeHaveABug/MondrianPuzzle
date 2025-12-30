#!/bin/bash
if [ $# -ne 7 ]
then
	echo "Usage: $0 height_inf height_sup rotate_flag defect_a defect_b options_min verbose_flag"
	exit 1
fi
make -f mondrian.make
echo 1 $1 $2 $3 $4 $5 $6 $7 | ./mondrian
exit 0
