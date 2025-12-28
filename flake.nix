{
  description = "A Nix-flake-based C/C++ development environment";

  inputs = {
    nixpkgs.url = "github:nixos/nixpkgs/nixos-unstable";
    flake-utils.url = "github:numtide/flake-utils";
  };

  outputs = { 
    self, nixpkgs, flake-utils, ...
    }: flake-utils.lib.eachDefaultSystem (system: let
      pkgs = nixpkgs.legacyPackages.${system};
      llvm = pkgs.llvmPackages;

    in {
      
      packages = {
        default = pkgs.callPackage ./default.nix;
      };

      checks = {
        crab_tests_gcc_debug = pkgs.callPackage ./default.nix { 
          stdenv = pkgs.stdenv;
          doCheck = true;
          doDebug = true;
        };

        crab_tests_gcc_release = pkgs.callPackage ./default.nix { 
          stdenv = pkgs.stdenv;
          doCheck = true;
          doDebug = false;
        };

        crab_tests_clang_debug = pkgs.callPackage ./default.nix { 
          stdenv = llvm.stdenv;
          doCheck = true;
          doDebug = true;
        };

        crab_tests_clang_release = pkgs.callPackage ./default.nix { 
          stdenv = llvm.stdenv;
          doCheck = true;
          doDebug = false;
        };
      };

      devShells = { 
        default = pkgs.mkShell.override {
          stdenv = llvm.stdenv;
        } {
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

            LD_LIBRARY_PATH = pkgs.lib.makeLibraryPath (with pkgs; [ fmt catch2_3 ]);
          };
      };
    });
}
