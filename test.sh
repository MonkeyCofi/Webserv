#!/usr/bin/env bash
clear
make -s re
make -s clean
alias web="./webserv"
bash -c "./webserv ./config/test.conf"