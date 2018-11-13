#!/bin/bash
pwd
if [ -z "$1" ]; then
    echo "Port is unset or set to the empty string"
    exit 1
fi
./app/NetCalculatorApp -p $1 -c 1 -t 1 &
export NC_PID=$!
sleep 1
./app/test/NetCalculatorAppTest1 127.0.0.1 $1
export TEST_CODE=$?
kill $NC_PID
exit $TEST_CODE
