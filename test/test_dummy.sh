#!/bin/bash

if [ -z $1 ]
then
	echo "No filename"
	exit 1;
fi

echo "Testing dummy"

echo "Building dummy"
g++ test/dummy.cpp -o test/dummy

echo "Starting server: ""./""$1" "7777"

sleep 5 && pkill "$1" &
sleep 1 && test/dummy &

echo "Program output:"
"./""$1" "7777"

if [ $? -eq 0 ]
then
	echo "Test passed"
else
	echo "Test not passed"
	exit 1
fi

echo ""
echo Done
