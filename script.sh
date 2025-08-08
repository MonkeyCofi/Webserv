#!/bin/bash
valgrind --trace-children=yes --track-fds=yes --show-leak-kinds=all --leak-check=full ./webserv "$@"
