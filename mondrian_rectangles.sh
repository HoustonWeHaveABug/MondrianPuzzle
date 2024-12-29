#!/bin/bash
if [ $# -ne 6 ]
then
	echo "Usage: $0 width rotate_flag defect_a defect_b options_min verbose_flag"
	exit 1
fi
make -f mondrian.make
let HEIGHT=1
let WIDTH=$1
while [ $HEIGHT -le $WIDTH ]
do
	echo "Rectangle ${HEIGHT}x${WIDTH}"
	echo $HEIGHT $WIDTH $2 $3 $4 $5 $6 | ./mondrian
	let HEIGHT=$HEIGHT+1
done
exit 0
