#!/bin/zsh
set -e

mkdir -p build
pushd build

# Build multiple .cpp files
for src in ../*.cpp; do
  base=$(basename "$src" .cpp)
  echo "Compiling $src -> $base"
  clang++ -std=c++20 -g -O0 "$src" -o "$base"
done

popd
