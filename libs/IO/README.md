# IO Library

## Overview
The `IO` library handles Input/Output operations for OpenMVS, specifically focusing on loading and saving images and 3D model formats. It acts as an abstraction layer over various file format libraries.

## Key Functionalities

### 1. Image I/O
Provides functionality to read and write various image formats.
- **Formats Supported**:
  - BMP (`ImageBMP.cpp`)
  - JPG (`ImageJPG.cpp`)
  - PNG (`ImagePNG.cpp`)
  - TIFF (`ImageTIFF.cpp`)
  - TGA (`ImageTGA.cpp`)
  - SCI (`ImageSCI.cpp`)
  - DDS (`ImageDDS.cpp`)
  - JXL (`ImageJXL.cpp`)

### 2. Model I/O
Provides functionality to read and write 3D model formats.
- **Formats Supported**:
  - **OBJ** (`OBJ.cpp`): Wavefront OBJ format.
  - **PLY** (`PLY.cpp`): Stanford Triangle Format.
  - **GLTF** (`tiny_gltf.h`): GL Transmission Format.

### 3. Serialization
- **XML**: XML parsing using `TinyXML2`.
- **JSON**: JSON parsing.

## References
- **TinyGLTF**: Header-only C++11 glTF 2.0 file parser and serializer by Syoyo Fujita and Aurélien Chatelain.
- **TinyXML2**: A simple, small, efficient, C++ XML parser.
- **stb_image**: Image loading library used by TinyGLTF.
