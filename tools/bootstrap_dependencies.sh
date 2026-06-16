#!/usr/bin/env bash
set -euo pipefail
mkdir -p include
if [ ! -d include/flatbuffers/.git ]; then
  rm -rf include/flatbuffers
  git clone --depth 1 https://github.com/google/flatbuffers.git include/flatbuffers
fi
if [ ! -d include/gf-pawncc/.git ]; then
  rm -rf include/gf-pawncc
  git clone --depth 1 https://github.com/Lincoln-LM/gf-pawncc.git include/gf-pawncc
fi
if [ ! -d include/tomlplusplus/.git ]; then
  rm -rf include/tomlplusplus
  git clone --depth 1 https://github.com/marzer/tomlplusplus.git include/tomlplusplus
fi
