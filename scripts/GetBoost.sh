#!/bin/bash

echo "Downloading Boost Package..."
wget https://boostorg.jfrog.io/artifactory/main/release/1.84.0/source/boost_1_84_0.tar.gz

echo "Untar the zipped file..."
tar xvf boost_1_84_0.tar.gz

cd boost_1_84_0

echo "Do bootstrap..."
./bootstrap.sh --prefix=/usr/local/

echo "Gen static linking library"
./b2

echo "Cloning Boost Package Finished!"