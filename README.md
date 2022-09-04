# bmp2gba

bmp2gba is a tool for homebrew GBA development written in C99 that converts a folder of .bmp files into a single c-file containing 4bpp tile, palette and map data for the GBA. The c-file is printed to stdout. Simply place the `bmp2gba.com` binary in the folder containing the `.bmp` files and pipe the output of stdout into a file. For example: `./bmp2gba.com > bg_data.c`

The generated c-file can than be included and used in your homebrew GBA-project.

## Preparation

Retrieve the repo and submodules:

```
git clone https://github.com/LutzenH/bmp2gba
git submodule update --init --recursive
```

## Building

### Native Binary (Linux or Windows)

> Make sure you have cmake and the build-essential installed.

bmp2gba should be compilable using MSVC, gcc and clang.

Generate Makefile and build the project:
```
cmake -Bcmake-build-debug -H.
cd cmake-build-debug/
make
```

### αcτµαlly pδrταblε εxεcµταblε

bmp2gba can also be compiled as an [αpε](https://justine.lol/ape.html). Which makes it possible to use a single executable binary on a multitude of platforms (Linux + Mac + Windows + FreeBSD + OpenBSD + NetBSD + BIOS).

Running the following commands will produce `bmp2gba.com` which can be run on the above-mentioned platforms:

```
chmod +x compile-b2g-ape.sh
./compile-b2g-ape.sh
```
