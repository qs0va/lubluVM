#!/bin/bash

echo "Testing not enought args"

echo "Running command: " "./""$1"
echo "Program output: "
sleep 5 && pkill mserv.out &
"./""$1"

if [ $? -eq 1 ]
then
	echo "Test passed"
else
	echo "Test not passed"
	exit 1
fi


echo ""
echo "Testing not number port"

echo "Running command: " "./""$1" "qwer"
echo "Program output: "
sleep 5 && pkill mserv.out &
"./""$1" qwer

if [ $? -eq 1 ]
then
	echo "Test passed"
else
	echo "Test not passed"
	exit 1
fi


echo ""
echo "Testing invalid port"

echo "Running command: " "./""$1" "12"
echo "Program output: "
sleep 5 && pkill mserv.out &
"./""$1" 12

if [ $? -eq 1 ]
then
	echo "Test passed"
else
	echo "Test not passed"
	exit 1
fi

echo ""
echo Done
