# MVS (Multi-View Stereo) Library

## Overview
The `MVS` library is the core of the OpenMVS project, implementing the complete pipeline for recovering 3D structures from a set of images and camera poses. It handles scene management, dense point cloud reconstruction, mesh generation, refinement, and texturing.

## Key Functionalities

### 1. Scene Management
The `Scene` class acts as the central container for all MVS data, including:
- **Platforms & Images**: Camera models and input images.
- **Point Cloud**: Sparse or dense point clouds.
- **Mesh**: Triangular meshes.
- **Functions**: Load/Save interface (`Interface.h`), Import/Export functionality.

### 2. Dense Reconstruction (`SceneDensify`)
Generates a dense point cloud from the input images and sparse point cloud.
- **Semi-Global Matching (SGM)**: A CPU-based stereo matching algorithm.
  - **Reference**: "Stereo Processing by Semiglobal Matching and Mutual Information", Heiko Hirschmüller. IEEE Transactions on Pattern Analysis and Machine Intelligence, 2008.
- **PatchMatch**: A CUDA-accelerated dense matching algorithm for high-resolution depth estimation.
  - **Reference**: "Multi-View Stereo with Asymmetric Checkerboard Propagation and Multi-Hypothesis Joint View Selection", Qingshan Xu and Wenbing Tao. 2018.

### 3. Mesh Reconstruction (`SceneReconstruct`)
Converts the dense point cloud into a surface mesh.
- **Algorithm**: Exploits visibility information using Delaunay tetrahedralization and Graph-Cut optimization to extract a watertight mesh.
  - **Reference**: "Exploiting Visibility Information in Surface Reconstruction to Preserve Weakly Supported Surfaces", Michal Jancosek and Tomas Pajdla. 2015.

### 4. Mesh Refinement (`SceneRefine`)
Refines the mesh geometry to improve accuracy and detail using variational refinement techniques.

### 5. Mesh Texturing (`SceneTexture`)
Projects the input images onto the reconstructed mesh to create a high-quality textured 3D model. It handles occlusion, seam leveling, and color blending.
