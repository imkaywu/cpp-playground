#!/bin/zsh
set -e

mkdir -p build
pushd build

# Build a single .cpp file
#gcc ../main.cpp -std=c++20 -lc++ -o main # also works
clang++ -std=c++20 -g -O0 ../main.cpp -o main

popd
