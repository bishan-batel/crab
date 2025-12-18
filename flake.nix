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

      crab = pkgs.callPackage ./nix/crab.nix { };
      crab-tests = pkgs.callPackage ./nix/crab.nix { doCheck = true; };

    in {
      
      packages = {
        default = crab;

        crab_tests_gcc_debug = pkgs.callPackage ./nix/crab.nix { 
          doCheck = true;
          stdenv = pkgs.stdenv;
          doDebug = true;
        };

        crab_tests_gcc_release = pkgs.callPackage ./nix/crab.nix { 
          doCheck = true;
          stdenv = pkgs.stdenv;
          doDebug = false;
        };

        crab_tests_clang_debug = pkgs.callPackage ./nix/crab.nix { 
          doCheck = true;
          stdenv = llvm.stdenv;
          doDebug = true;
        };

        crab_tests_clang_release = pkgs.callPackage ./nix/crab.nix { 
          doCheck = true;
          stdenv = llvm.stdenv;
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
