if [ $# -ne 3 ]
then
	echo "Usage: $0 <order> <start> <verbose>"
	exit 1
fi
make -f ./mondrian.make
let START=$2
while [ $START -ge 0 ]
do
	echo $1 $START $START $3 | ./mondrian
	let START=$START-1
done
exit 0
