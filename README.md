# meshi-bb
This repository is part of a semester project. The project is organized in the 3 parts: blackbox, backend, application.
The "blackbox" (this repository) provides a implementation to run measurements at different POIs.

This was made to run on a raspberry pi or something similar (*nix-like). Additionally, a somewhat limited windows support was added to allow easier testing.
The windows port doesn't provide sensor interfacing capabilities (i2c etc.)

## Requirements
* CMake
* OpenCV (*nix only)
* escapi (windows only)

## Compiling
```
mkdir build
cd build
cmake ../
make
```
