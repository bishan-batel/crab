#!/usr/bin/env nu

let generator = "Ninja"

const num_threads = 24;

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

def get_msvc_compiler [] {
	return {
		name: "MSVC",
		c: (which cl | first | get path),
		cpp: (which cl | first | get path),
	}
}

let compilers = if (uname).kernel-name == "Windows_NT" {
	[(get_msvc_compiler), $gcc, $clang]
} else {
	[$gcc, $clang]
};

def "main windows" [] {
	let build_type = if true { "Debug" } else { "Release" };
	let compiler = get_msvc_compiler; 
	let build_dir = "build/debug"

	cmake "-Wno-dev" "-B" $build_dir "-DCRAB_TESTS=ON" $"-DCMAKE_BUILD_TYPE=($build_type)" $"-DCMAKE_C_COMPILER=($compiler.c)" $"-DCMAKE_CXX_COMPILER=($compiler.cpp)" $"-G($generator)" "-DCMAKE_EXPORT_COMPILE_COMMANDS=ON"
}

def cmake_test [build_type: string, compiler: record, fmtstd: bool, fmtlib: bool] {
	let fmtstd = if $fmtstd { "ON" } else { "OFF" };
	let fmtlib = if $fmtlib { "ON" } else { "OFF" };
	let builddir = $"build/_($compiler.name)_($build_type)_fmtstd-($fmtstd)_fmtlib-($fmtlib)"

	print "> Setting up cmake"
	cmake -Wno-dev "-B" $builddir "-DCRAB_TESTS=ON" $"-DCMAKE_BUILD_TYPE=($build_type)" $"-DCMAKE_C_COMPILER=($compiler.c)" $"-DCMAKE_CXX_COMPILER=($compiler.cpp)" $"-G($generator)" "-DCMAKE_EXPORT_COMPILE_COMMANDS=ON" "-DCPM_USE_LOCAL_PACKAGES=OFF" $"-DCRAB_USE_FMT=($fmtlib)" $"-DCRAB_USE_STD_FORMAT=($fmtstd)"

	print "> Compiling"
	cmake --build $builddir -j($num_threads)

	print "> Running tests"

	alias crab_tests = ./($builddir)/tests/crab-tests

	if $build_type == "Release" {
		crab_tests -e
	} else {
		crab_tests
	}
}

def main [] {
	print $"Running with the following compilers: ($compilers | each { |x| $x.name } | str join ', ')"

	mut tests = [];

	for compiler in $compilers {

		for debug in [true, false] {
			let build_type = if $debug { "Debug" } else { "Release" };

			for fmtstd in [false, true] {
				for fmtlib in [false, true] {

					if $fmtstd == $fmtlib { continue; };

					let name = $"($compiler.name) on ($build_type), fmtstd=($fmtstd), fmtlib=($fmtlib)";

					$tests = $tests | append {||
						print $"Testing Compiler ($name)"
						try {  
							cmake_test $build_type $compiler $fmtstd $fmtlib
						print $"Passed Compiler ($name)"
						} catch { 
							error make --unspanned {
								msg: $"Failed tests for ($name)",
							}
							exit
						}
					};

				}
			}
		}
	}

	$tests | par-each { |x| do $x }
}

#./build/debug/test/crab-tests -i
#./build/release/test/crab-tests -e -i
