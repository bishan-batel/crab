#!/usr/bin/env bash

nix build .#crab_tests_clang_debug
nix build .#crab_tests_clang_release

nix build .#crab_tests_gcc_debug
nix build .#crab_tests_gcc_release
