#!/bin/bash
if [ $# -ne 7 ]
then
	echo "Usage: $0 width_inf width_sup rotate_flag defect_a defect_b options_min verbose_flag"
	exit 1
fi
make -f mondrian.make
let WIDTH=$1
while [ $WIDTH -le $2 ]
do
	let HEIGHT=1
	while [ $HEIGHT -le $WIDTH ]
	do
		echo "Rectangle ${HEIGHT}x${WIDTH}"
		echo $HEIGHT $WIDTH $3 $4 $5 $6 $7 | ./mondrian
		let HEIGHT=$HEIGHT+1
	done
	let WIDTH=$WIDTH+1
done
exit 0
