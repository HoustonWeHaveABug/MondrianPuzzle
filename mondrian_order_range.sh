if [ $# -ne 7 ]
then
	echo "Usage: $0 order_inf order_sup rotate_flag defect_a defect_b options_min verbose_flag"
	exit 1
fi
make -f mondrian.make
let ORDER=$1
while [ $ORDER -le $2 ]
do
	echo "Order $ORDER"
	echo $ORDER $3 $4 $5 $6 $7 | ./mondrian
	let ORDER=$ORDER+1
done
exit 0
