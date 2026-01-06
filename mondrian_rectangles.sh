#!/bin/bash
if [ $# -ne 8 ]
then
	echo "Usage: $0 width_lo width_hi rotate_flag defect_a defect_b options_lo options_hi verbose_flag"
	exit 1
fi
make -f mondrian.make
echo 2 $1 $2 $3 $4 $5 $6 $7 $8 | ./mondrian
exit 0
