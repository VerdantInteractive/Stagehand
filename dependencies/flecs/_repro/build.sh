#!/usr/bin/env bash

# Compile flecs.c as C (gnu99), then compile/link C++ main
set -e

SRCDIR="$(cd "$(dirname "$0")" && pwd)/.."
DISTR="$SRCDIR/distr"
PROJ_DIR="$(cd "$(dirname "$0")" && pwd)"

# Require exactly one argument: repro subdirectory (e.g. flecs_pipeline_disable)
if [ "$#" -ne 1 ]; then
	echo "Usage: $0 <repro_subdir>" >&2
	exit 1
fi
SUBDIR="$1"
TARGET_DIR="$PROJ_DIR/$SUBDIR"
if [ ! -d "$TARGET_DIR" ]; then
	echo "Directory not found: $TARGET_DIR" >&2
	exit 1
fi
MAIN_CPP="$TARGET_DIR/main.cpp"
if [ ! -f "$MAIN_CPP" ]; then
	echo "main.cpp not found in $TARGET_DIR" >&2
	exit 1
fi

# Compile flecs C source with gcc using gnu99; place object in REPRO
gcc -std=gnu99 -I"$DISTR" -DFLECS_DEBUG -O2 -c "$DISTR/flecs.c" -o "$PROJ_DIR/flecs.o"

# Compile and link C++ program with flecs.o; output binary in REPRO
g++ -std=c++17 -I"$DISTR" -O2 "$MAIN_CPP" "$PROJ_DIR/flecs.o" -o "$PROJ_DIR/repro"
