#!/bin/zsh
set -e

mkdir -p build
pushd build

if [ $# -eq 0 ]; then
  clang++ -std=c++20 -g -O0 ../main.cpp -o main
else
  NAMESPACE=$(echo $1 | tr '[:lower:]' '[:upper:]')
  clang++ -std=c++20 -g -O0 ../main.cpp -D${NAMESPACE} -o main
fi

popd
