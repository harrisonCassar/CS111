#!/bin/bash
#shebang

# NAME: Harrison Cassar
# EMAIL: Harrison.Cassar@gmail.com
# ID: 505114980

passedtests=1

# Test 1: Normal operation (able to connect sensors and begin data polling)
echo OFF | ./lab4b --period=3 --scale=C

if [[ $? -ne 0 ]]
then
	passedtests=0
fi

# Test 2: Properly handling unrecognized argument
echo OFF | ./lab4b arg --period=3 --scale=C

if [[ $? -ne 1 ]]
then
	passedtests=0
fi

# Test 3: Normal logging of collected data 
rm -f ./tmp.txt
echo OFF | ./lab4b --period=1 --log=tmp.txt

if [[ $? -ne 0 ]]
then
	passedtests=0
fi

if [ ! -f ./tmp.txt ]
then
	passedtests=0
fi

# Clean up tmp files made during smoke tests
rm -f tmp.txt

if [ $passedtests -ne 1 ]
then
	printf "\nBuilt executable failed to pass all smoke tests.\n"
else
	printf "\nBuilt executable successfully passed all smoke tests.\n"
fi
