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
- **Algorithms**:
  - **PatchMatch**: A probabilistic algorithm for dense depth estimation that propagates depth and normal estimates between neighbors and refines them using random assignment. It is particularly effective for large-scale scenes.
    - **Reference**: "Accurate Multiple View 3D Reconstruction Using Patch-Based Stereo for Large-Scale Scenes", Shuhan Shen. IEEE Transactions on Image Processing, 2013.
  - **Semi-Global Matching (SGM)**: A robust stereo matching algorithm that approximates global energy minimization by aggregating matching costs along multiple 1D paths (typically 4 or 8) through the image. It uses a smoothness constraint to penalize small changes in disparity.
    - **Reference**: "Stereo Processing by Semiglobal Matching and Mutual Information", Heiko Hirschmüller. IEEE Transactions on Pattern Analysis and Machine Intelligence, 2008.
- **Fusion**: Depth maps are fused into a single point cloud. The process involves:
  - **Geometric Consistency**: Filtering depth estimates that do not agree across multiple views.
  - **Confidence merging**: Averaging depth values weighted by their confidence (derived from ZNCC scores).

### 3. Mesh Reconstruction (`SceneReconstruct`)
Converts the dense point cloud into a surface mesh.
- **Method**: Delaunay Tetrahedralization combined with Graph-Cut optimization.
- **Process**:
  1.  **Delaunay Triangulation**: Constructs a tetrahedralization of the dense point cloud.
  2.  **Graph Construction**: Builds a directed graph where tetrahedra are nodes. Edges represent visibility information; weights are derived from ray intersections (rays from cameras to points).
  3.  **Optimization**: Solves a max-flow/min-cut problem to extract the surface that minimizes visibility violations.
- **Reference**: "Exploiting Visibility Information in Surface Reconstruction to Preserve Weakly Supported Surfaces", Michal Jancosek and Tomas Pajdla. 2015.

### 4. Mesh Refinement (`SceneRefine`)
Refines the mesh geometry to improve accuracy and detail.
- **Method**: Variational minimization of an energy function combining photo-consistency and regularization.
- **Algorithm**:
  - **Photo-consistency**: Maximizes similarity (ZNCC) between appearances of surface patches in different views.
  - **Regularization**: Applies smoothness constraints (Laplacian) to surface curvature.
  - **Optimization**: Iteratively adjusts vertex positions along surface normals using gradient descent.
- **Reference**: "High accuracy and visibility-consistent dense multiview stereo", H. H. Vu, P. Labatut, J. P. Pons, and R. Keriven. IEEE Transactions on Pattern Analysis and Machine Intelligence, 2012.

### 5. Mesh Texturing (`SceneTexture`)
Projects the input images onto the reconstructed mesh to create a high-quality textured 3D model.
- **Method**: View selection and mosaic generation.
- **Process**:
  1.  **View Selection**: Assigns the best view to each face based on resolution, blurriness (gradient magnitude), and viewing angle. Uses a Markov Random Field (MRF) optimization to minimize seam visibility.
  2.  **Seam Leveling**:
      - **Global**: Adjusts color intensities across patches to minimize global differences.
      - **Local**: Blends pixel values across seam boundaries (Poisson blending) to ensure smooth transitions.
  3.  **Packing**: Packs texture patches into a texture atlas using heuristics to minimize wasted space.
- **Reference**: "Let There Be Color! Large-Scale Texturing of 3D Reconstructions", Michael Waechter, Nils Moehrle, and Michael Goesele. ECCV 2014.
