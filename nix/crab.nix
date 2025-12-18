{ 
  stdenv,
  cmake,
  lib, 
  fmt, 
  catch2_3,
  pkg-config, 
  ... 
}: stdenv.mkDerivation {
  pname = "crab";
  version = "2.1.1";
  src = ./.; # Points to your project's source code in the same directory

  # Build-time dependencies (e.g., CMake, the compiler, testing frameworks like Catch2)
  nativeBuildInputs = [
    cmake
    catch2_3
  ];

  # Run-time dependencies (libraries your final binary needs to link against)
  buildInputs = [
    fmt
  ];

  checkTarget = "crab-test";

  cmakeFlags = [
    "-DCRAB_TESTS=ON"
  ];

  # The checkPhase is where tests are run
  # For a standard CMake project, this might run automatically,
  # but you can explicitly define it if needed:
  checkPhase = ''
    echo "Running C++ tests..."
    cmake --build . --target test
    ./test/crab-tests
  '';

  # Ensures the tests are actually run during nix build
  doCheck = true;
};
