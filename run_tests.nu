#!/usr/bin/env nu

let generator = "Ninja"

let num_threads = 24;

let gcc = {
	name: "GCC",
	c: (which gcc | first | get path),
	cpp: (which g++ | first | get path),
}

let clang = {
	name: "Clang",
	c: (which clang | first | get path),
	cpp: (which clang++ | first | get path),
}

def cmake_test [build_type: string, compiler: record] {
	let builddir = $"build/_($compiler.name)_($build_type)"

	print "> Setting up cmake"
	cmake -Wno-dev "-B" $builddir "-DCRAB_TESTS=ON" $"-DCMAKE_BUILD_TYPE=($build_type)" $"-DCMAKE_C_COMPILER=($compiler.c)" $"-DCMAKE_CXX_COMPILER=($compiler.cpp)" $"-G($generator)" "-DCMAKE_EXPORT_COMPILE_COMMANDS=ON"

	print "> Compiling"
	cmake --build $builddir -j($num_threads)

	print "> Running tests"

	alias crab_tests = ./($builddir)/test/crab-tests
	if $build_type == "Release" {
		crab_tests -e
	} else {
		crab_tests
	}
}

for compiler in [$clang, $gcc] {
	for debug in [true, false] {
		let build_type = if $debug { "Debug" } else { "Release" };

		print $"Testing Compiler ($compiler.name) on ($build_type)"

		try {  
			cmake_test $build_type $compiler 
		} catch { 
			error make --unspanned {
				msg: $"Failed tests for ($compiler.name) on ($build_type)",
			}
			exit
		}
	}
}

#./build/debug/test/crab-tests -i
#./build/release/test/crab-tests -e -i
