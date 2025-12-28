{
  stdenv,
  cmake,
  lib,
  fmt,
  catch2_3,
  debugBuild ? false,
  doCheck ? false,
  useFmtLib ? false,
  ...
}:
stdenv.mkDerivation {
  pname = "crab";
  version = builtins.readFile ./VERSION;
  src = ./.;

  nativeBuildInputs =
    [
      cmake
    ]
    ++ lib.optional doCheck catch2_3
    ++ lib.optional useFmtLib fmt;

  buildInputs =
    [
    ]
    ++ lib.optional doCheck catch2_3
    ++ lib.optional useFmtLib fmt;

  checkTarget = "crab-tests";

  cmakeFlags = [
    "-DCRAB_TESTS=${if doCheck then "ON" else "OFF"}"
    "-DCRAB_USE_FMT=${if useFmtLib then "ON" else "OFF"}"
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

  inherit doCheck;
}
