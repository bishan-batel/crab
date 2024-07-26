with import <nixpkgs> { };

clangStdenv.mkDerivation {
  name = "dev-shell";

  src = null;

  nativeBuildInputs = [ clang ninja ];
  buildInputs = [ clang ];

  shellHook = ''
     # code ran when entering shell
  '';
}

