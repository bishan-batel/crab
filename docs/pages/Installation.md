Installing crab is probally going to be less painfull than most C++ libraries because (as of now) it is header only.

!> Note that installing / using crab as a dependency means you also inherit depending on the [fmt](https://fmt.dev/12.0/) library.

## CMake

Crab supports being installed on a system level, however this library is not available on any package manager outside of NixOS via flakes.

<!-- tabs:start -->

#### **FetchContent**
You can use crab with the classic CMake FetchContent, this method will also take care of installing [fmt](https://fmt.dev) if it is not present. Note that this will prioritize a local installation of fmtlib over downloading it with FetchContent (using CPM).

```CMake 
include(FetchContent)
FetchContent_Declare(
  crab
  GIT_REPOSITORY https://github.com/bishan-batel/crab.git
  GIT_TAG "v2.3.1" # Git version tag to use, see the crab releases page for other versions
)

# this will make the 'crab::crab' target available to link to
# note that if fmtlib
FetchContent_MakeAvailable(crab)


add_executable(my_app ./main.cpp)
target_link_libraries(my_app PUBLIC crab::crab)

```

#### **find_package / NixOS**

flake.nix:
```nix
{
  inputs = {
    nixpkgs.url = "github:nixos/nixpkgs/nixos"; # unstable also works
    flake-utils.url = "github:numtide/flake-utils";
    crab.url = "github:bishan-batel/crab";
  };

  outputs = {
    self,
    nixpkgs,
    flake-utils,
    crab,
    ... 
  }: flake-utils.lib.eachDefaultSystem (
      system:
      let
        pkgs = nixpkgs.legacyPackages.${system};
        crabPkgs = crab.packages.${system};
        libPath = [ crabPkgs.default pkgs.fmt ]; # make sure to include fmtlib as well!
      in {
        devShells = {
          default = pkgs.mkShell {
            # ...
            buildInputs = [ pkgs.pkg-config crabPkgs.default ];
            nativeBuildInputs = [ ] ++ libPath;
            env.LD_LIBRARY_PATH = pkgs.lib.makeLibraryPath libPath;
            # ...
          };
        };
      }
    );
}
```

Then in your CMake file,
```CMake 
find_package(fmt REQUIRED)
find_package(crab CONFIG REQUIRED)

add_executable(my_app ./main.cpp)
target_link_libraries(my_app PUBLIC crab::crab)
```

<!-- tabs:end -->

## MSBuild 
Currently I have not tested how to get crab working in a native MSBuild project, however the latest version of crab does work with versions of the MSVC compiler. It is probaly possible to add crab through MSBuild, I just haven't done it myself. 
