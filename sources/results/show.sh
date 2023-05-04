#!/bin/bash
# lru.cc	lru-simpass.cc	Makefile  prj-16k-2b.cc  prj-16k-3b.cc	prj-4k-2b.cc  ship++.cc  ship++-simpass.cc
ALGORITHMS="lru ship++ ship++-simpass lru-simpass prj-16k-3b prj-16k-2b prj-4k-3b prj-4k-2b"

if [ $# -gt 0 ]; then
    CFG=$1
else
    CFG=1
fi

if [ $# -gt 1 ]; then
    ALGORITHMS=$2
fi

./py_results.py config$CFG "$ALGORITHMS"
