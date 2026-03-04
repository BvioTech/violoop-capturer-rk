#!/bin/bash
DST_DIR="dist"

# create dir
mkdir -p $DST_DIR
cd $DST_DIR

# configure
cmake ..

# make
make -j$(nproc)
