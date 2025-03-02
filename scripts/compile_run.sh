#!bin/bash

BASH_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

if [ -d ${BASH_DIR}/../results ];then
    rm -rf ${BASH_DIR}/../results
fi

mkdir -p ${BASH_DIR}/../results

cd ${BASH_DIR}/../results
g++ -o ${BASH_DIR}/../results/test ${BASH_DIR}/../src/main.cpp -std=c++11 -pthread

if [ $? -ne 0 ];then
    echo "[ERROR] g++ compile error, Please check your codes!"
    return 1
fi

./test
if [ $? -ne 0 ];then
    echo "[ERROR] exec error"
    return 1
fi


cd - > /dev/null