#/bin/sh

mkdir -p bin

echo "---------------------------------------------------"
echo "+                                                 +"
echo "+            Sec Fw Dataplane                     +"
echo "+                                                 +"
echo "---------------------------------------------------"

PROG_SECD="secd-linux_64" 
rm -f $PROG_SECD
make OCTEON_TARGET=linux_64
if [ $? -eq 0 ]; then
	echo "Dataplane build done!"
else
	echo "Error! Dataplane build failed!"
	exit 
fi

if [ ! -f "$PROG_SECD" ]; then
	echo "$PROG_SECD not exist"
	exit 
fi

cp $PROG_SECD bin/
echo "Dataplane build success....."

echo "---------------------------------------------------"
echo "+                                                 +"
echo "+            Sec Fw Mgrplane                      +"
echo "+                                                 +"
echo "---------------------------------------------------"

PROG_CLI="bin/cli"
PROG_SRV="bin/srv"

cd ./mgrplane 

rm -rf bin
pwd
make

if [ $? -eq 0 ]; then
	echo "Mgrplane build done!"
else
	echo "Error! Mgrplane build failed!"
fi


if [ ! -f "$PROG_CLI" ]; then
	echo "$PROG_CLI not exist" 
	exit
fi

if [ ! -f "$PROG_SRV" ]; then
	echo "$PROG_SRV not exist"
	exit
fi

cp $PROG_CLI ../bin/
cp $PROG_SRV ../bin/

echo "Mgrplane build success....."
