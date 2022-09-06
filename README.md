<img alt="bmp2gba" src="https://user-images.githubusercontent.com/46445220/188315122-ba1086d3-704f-441c-86ff-e07fe7e3063c.png">

# bmp2gba

bmp2gba is a tool for homebrew GBA development written in C99 that converts a folder of .bmp files into a single c-file containing 4bpp tile, palette and map data for the GBA. The c-file is printed to stdout. Simply place the `bmp2gba.com` binary in the folder containing the `.bmp` files and redirect the output of stdout into a file. For example: `./bmp2gba.com > bg_data.c`

The generated c-file can than be included and used in your homebrew (tonc) GBA-project.

The difference between bmp2gba and alternatives like grit and gfx2gba is that for 4bpp-tiles bmp2gba guarantees that the palette-banks generated will display correctly on the GBA (assuming less than 15+1 colors are used per tile and less than 16*16 colors in total). Inorder to fit all these colors onto the palette-banks, bmp2gba tries to reduce the amount of banks used. This is first done using a first-fit-decreasing packing pass, but this sometimes produces suboptimal results that wastes too many banks. That is why the bruteforce packing pass has been added to look for better packed palettes by shuffling the color sets and keeping track of the order that uses the least amount of space.

Usage:

```
bmp2gba --transparent-color FF00FF --brute-force-count 250000 > bg_data.c
```

[example.webm](https://user-images.githubusercontent.com/46445220/188323596-82139baf-ab23-4312-b097-6d7544927765.webm)

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

Running the following commands on Linux with gcc installed will produce `bmp2gba.com` which can be run on the above-mentioned platforms:

```
chmod +x compile-b2g-ape.sh
./compile-b2g-ape.sh
```
