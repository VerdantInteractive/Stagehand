#!/usr/bin/env bash
set -e

scons debug_symbols=yes optimize=debug "$@"
