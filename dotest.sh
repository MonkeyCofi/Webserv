#!/usr/bin/env bash
clear
if [ ! -f ./webserv ]; then
    make re && make clean
fi
alias web="./webserv"
bash -c "./webserv ./config/test.conf"