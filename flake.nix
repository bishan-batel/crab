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
      llvm = pkgs.llvmPackages_19;
      libPath = [pkgs.fmt]; 
    in {

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
              llvm.clang.cc

              pkg-config
            ] ++ libPath;

            nativeBuildInputs = with pkgs; [ 
              llvm.clang
            ] ++ libPath;

            LD_LIBRARY_PATH = pkgs.lib.makeLibraryPath libPath;
            DYLD_LIBRARY_PATH = pkgs.lib.makeLibraryPath libPath;
          };
      };
    });
}
