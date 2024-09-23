{
  description = "A Nix-flake-based C/C++ development environment";

  inputs = {
    nixpkgs.url = "github:nixos/nixpkgs/nixos-unstable";
  };

  outputs = { self, nixpkgs }:
  let
    supportedSystems = [ "x86_64-linux" "aarch64-linux" ];
    forEachSupportedSystem = f: nixpkgs.lib.genAttrs supportedSystems (system: f {
      pkgs = import nixpkgs { 
          inherit system; 
      };
    });
  in {
    devShells = forEachSupportedSystem ({ pkgs }: {
      default = pkgs.mkShell.override {}
        {
          shellHook = /* bash */ '' '';

          packages = with pkgs; [
              ninja
              cmake
              python3
              valgrind
          ];

          nativeBuildInputs = with pkgs; [
            clang
            clang-tools
          ];

        };
    });
  };
}
