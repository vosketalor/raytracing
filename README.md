# Raytracing Project

## Authors
Martin Baptiste
<br>Marchebout Lilian

## Overview
A C++ implementation of a physically-based raytracer, developed as a personal project to understand core computer graphics concepts. 
The raytracer simulates light transport to generate realistic images from 3D scene descriptions.

## Features
- Physically-based light transport simulation
- Basic geometric primitives (spheres, planes)
- Camera system with adjustable parameters
- Scene description and rendering pipeline
- Image output generation
- GPU

## Possibles extensions
- Shape
  - OBJ
  - Triangle
- Textures
- Animations
- Multiple types of maps: https://3dstudio.co/3d-texture-mapping/
  - Height map
  - Roughness map
  - Bump map
  - Reflection map
- GUI
  - Zoom
  - Inspector menu for objects clicked
  - Scenes switching in real time
- Antialiasing
  - MSAA
  - FXAA
- True HDR
- Global Illumnation
- Multiple types of lights
  - Spot Light
  - Ambient Light
  - Sun Light
  
## Build Instructions

1. Ensure you have CMake (≥3.10) and a C++17 compatible compiler installed
2. Clone the repository:
```bash
git clone https://github.com/vosketalor/raytracing.git
```
3. Generate build files:
```bash
mkdir build && cd build
cmake ..
```
4. Compile:
```bash
make
```

## Usage

Run the compiled executable with:
```bash
./Raytracing [width] [height] [output_image.ppm]
```

The program will render the default scene and save the result as a PPM image file.

## Dependencies

- C++ Standard Library (C++17)
- CMake (for building)

## License

This project is licensed under the terms of the MIT License - see the LICENSE file for details.