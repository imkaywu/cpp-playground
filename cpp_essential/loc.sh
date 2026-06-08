#!/usr/bin/env bash

# Usage:
#   ./loc.sh            # current directory
#   ./loc.sh /path/to/dir

DIR="${1:-.}"

find "$DIR" -type f -name "*.cpp" -exec wc -l {} + |
awk '{sum += $1} END {print sum}'
