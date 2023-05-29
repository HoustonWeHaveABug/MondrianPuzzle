if [ $# -ne 2 ]
then
	echo "Usage: $0 <order> <rotate_flag>"
	exit 1
fi
make -f mondrian.make
let ORDER=$1
let ROTATE_FLAG=$2
if [ $ROTATE_FLAG -eq 0 ]
then
	UB=`echo "$ORDER/l($ORDER)+1" | bc -l | sed "s/\..*//g"`
else
	UB=`echo "$ORDER/l($ORDER)+4" | bc -l | sed "s/\..*//g"`
fi
echo $ORDER $ROTATE_FLAG $UB 0 2 0 | ./mondrian
exit 0
