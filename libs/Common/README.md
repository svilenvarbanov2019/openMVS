# Common Library

## Overview
The `Common` library is a general-purpose utility module used throughout the OpenMVS project. It provides fundamental data structures, threading support, logging mechanisms, configuration handling, and geometric primitives.

## Key Functionalities

### 1. Geometric Primitives
Provides classes for basic geometric operations and intersection tests:
- **AABB (Axis-Aligned Bounding Box)**: `AABB.h`
- **OBB (Oriented Bounding Box)**: `OBB.h`
- **Plane**: `Plane.h`
- **Sphere**: `Sphere.h`
- **Ray**: `Ray.h`
- **Line**: `Line.h`

### 2. System Utilities
- **Threading**: Wrappers for threading, critical sections, and semaphores (`Thread.h`, `CriticalSection.h`, `Semaphore.h`, `EventQueue.h`).
- **Logging**: A logging facility for debug and status messages (`Log.h`).
- **Timer**: High-precision timing utilities (`Timer.h`).
- **Config**: Configuration file parsing and command-line argument handling (`Config.h`, `ConfigTable.h`).

### 3. Data Structures & Algorithms
- **Spatial Indexing**: Octree implementation for spatial partitioning (`Octree.h`).
- **Memory Management**: Smart pointers (`AutoPtr.h`, `SharedPtr.h`).
- **Delegates**: Fast delegates implementation (`FastDelegate.h`).

## References
- **FastDelegate**: Based on "The Impossibly Fast C++ Delegates" by Sergey Ryazanov, updated by "janezz55" and "Benjamin YanXiang Huang".
