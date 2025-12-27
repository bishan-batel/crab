{ 
  stdenv,
  cmake,
  lib, 
  fmt, 
  catch2_3,
  pkg-config, 
  debugBuild ? false,
  ... 
}: stdenv.mkDerivation rec {
  pname = "crab";
  version = builtins.readFile ../VERSION;

  src = ./..;

  nativeBuildInputs = [
    cmake
    catch2_3
  ];

  buildInputs = [
    fmt
    catch2_3
  ];

  checkTarget = "crab-tests";

  cmakeFlags = [
    "-DCRAB_TESTS=ON"
    "-DUSE_CPM=OFF"
    "-Wno-dev"
    "-DCMAKE_INSTALL_LIBDIR=lib"
  ];

  # The checkPhase is where tests are run
  # For a standard CMake project, this might run automatically,
  # but you can explicitly define it if needed:
  checkPhase = ''
    echo "Running C++ tests..."
    cmake --build . --target crab-tests
    ${if debugBuild then "./tests/crab-tests" else "./tests/crab-tests -e"}
  '';

  # Ensures the tests are actually run during nix build
  doCheck = true;
}
