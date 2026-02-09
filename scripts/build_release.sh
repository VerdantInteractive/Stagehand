#!/usr/bin/env bash
set -e

COMMON_BUILD_ARGS=("target=template_release" "production=yes" "optimize=speed" "lto=full")


# Build for web with specific linkflags - https://www.flecs.dev/flecs/md_docs_2BuildingFlecs.html#building-on-emscripten
WEB_LINKFLAGS=("-s ALLOW_MEMORY_GROWTH=1" "-s STACK_SIZE=1mb" "-s EXPORTED_RUNTIME_METHODS=cwrap" "-s MODULARIZE=1" "-s EXPORT_NAME='Game'")
scons "${COMMON_BUILD_ARGS[@]}" platform="web" linkflags="${WEB_LINKFLAGS[*]}"

# Build for other platforms
for platform in linux windows; do
    scons ${COMMON_BUILD_ARGS[@]} platform="${platform}"
done
