#!/bin/zsh
set -e

mkdir -p build
pushd build

for src in ../*.cpp; do
  base=$(basename "$src" .cpp)
  echo "Compiling $src -> $base"
  clang++ "$src" -std=c++20 -o "$base"
done

popd
