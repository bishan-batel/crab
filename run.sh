#!/bin/bash

cmake -S . -B "cmake-build-debug/" -DCMAKE_EXPORT_COMPILE_COMMANDS=1 &&\
  cp ./cmake-build-debug/compile_commands.json .  && \
  cmake --build cmake-build-debug  && \
  ./cmake-build-debug/crabpp

