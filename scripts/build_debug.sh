#!/usr/bin/env bash
set -e

scons target=editor debug_symbols=yes optimize=debug "$@"
