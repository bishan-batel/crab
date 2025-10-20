#!/usr/bin/env bash

cmake --preset debug -Wno-dev
cmake --preset release -Wno-dev


cmake --build build/debug -j$(nproc)
cmake --build build/release -j$(nproc)

./build/debug/test/crab-tests -i
./build/release/test/crab-tests -e -i
