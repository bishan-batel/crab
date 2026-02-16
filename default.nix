{
  stdenv,
  cmake,
  lib,
  fmt,
  catch2_3,
  debugBuild ? false,
  doCheck ? true,
  ...
}:

stdenv.mkDerivation {
  pname = "crab";
  version = builtins.readFile ./VERSION;
  src = ./.;

  nativeBuildInputs = [
    cmake
    fmt
  ] ++ lib.optional doCheck catch2_3;

  buildInputs = [
  ] ++ lib.optional doCheck catch2_3;

  checkTarget = "crab-tests";

  cmakeFlags = [
    "-DCRAB_TESTS=${if doCheck then "ON" else "OFF"}"
    "-DCRAB_INSTALL=ON"
    "-DCMAKE_INSTALL_LIBDIR=lib"
    "-DCPM_USE_LOCAL_PACKAGES=ON"
  ];

  checkPhase = ''
    echo "Running C++ tests..."
    cmake --build . --target crab-tests
    ${if debugBuild then "./tests/crab-tests" else "./tests/crab-tests -e"}
  '';

  inherit doCheck;
}
