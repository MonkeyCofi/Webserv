#!/usr/bin/env bash
clear
if [ ! -f ./webserv ]; then
    make re && make clean
fi
# make && make clean
alias web="./webserv"
bash -c "./webserv ./config/block_test.conf"