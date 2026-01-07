{
  description = "Crab";

  inputs = {
    nixpkgs.url = "github:nixos/nixpkgs/nixos-unstable";
    flake-utils.url = "github:numtide/flake-utils";
  };

  outputs =
    { nixpkgs, flake-utils, ... }:
    flake-utils.lib.eachDefaultSystem (
      system:
      let
        pkgs = import nixpkgs { inherit system; };
        llvm = pkgs.llvmPackages;
        crab = pkgs.callPackage ./default.nix;
      in
      {

        packages = {
          default = crab { doCheck = false; };
          crab = crab { doCheck = false; };
        };

        checks =
          let
            matrix = pkgs.lib.cartesianProduct {
              stdenv = [
                llvm.stdenv
                pkgs.stdenv
              ];

              # debug or release
              useDebug = [
                false
                true
              ];

              useFmtLib = [
                false
                true
              ];
            };
          in
          builtins.listToAttrs (
            builtins.concatLists (
              pkgs.lib.map (
                {
                  stdenv,
                  useDebug,
                  useFmtLib,
                }:
                [
                  {
                    name = "stdenv-${stdenv.name}_fmt-${pkgs.lib.boolToString useFmtLib}_debug-${pkgs.lib.boolToString useDebug}";
                    value = crab {
                      inherit stdenv;
                      inherit useDebug;
                      inherit useFmtLib;
                      doCheck = true;
                    };
                  }
                ]
              ) matrix
            )
          );

        devShells = {
          default =
            pkgs.mkShell.override
              {
                stdenv = llvm.stdenv;
              }
              {
                name = "crab";
                packages = with pkgs; [
                  ninja
                  cmake
                  unzip
                ];

                buildInputs = with pkgs; [
                  llvm.clang-tools
                  llvm.clang
                  gcc
                ];

                nativeBuildInputs = with pkgs; [
                  fmt
                  catch2_3
                  pkg-config
                ];

                LD_LIBRARY_PATH = pkgs.lib.makeLibraryPath (
                  with pkgs;
                  [
                    fmt
                    catch2_3
                  ]
                );
              };
        };
      }
    );
}
