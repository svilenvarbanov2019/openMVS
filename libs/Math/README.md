# Math Library

## Overview
The `Math` library provides advanced mathematical algorithms and tools required for the core Multi-View Stereo operations, including optimization, graph algorithms, and robust statistical methods.

## Key Functionalities

### 1. IBFS (Incremental Breadth-First Search)
Implements the IBFS algorithm for solving the Maximum s-t Flow / Minimum s-t Cut problem. This is typically used in graph-cut based optimization tasks, such as mesh reconstruction.
- **Reference**: "Maximum flows by incremental breadth-first search", Andrew V. Goldberg, Sagi Hed, Haim Kaplan, Robert E. Tarjan, and Renato F. Werneck. In Proceedings of the 19th European conference on Algorithms, ESA'11, pages 457-468. 2011.

### 2. LMFit (Levenberg-Marquardt Fitting)
Provides a C++ implementation of the Levenberg-Marquardt algorithm for non-linear least-squares minimization. This is essential for bundle adjustment and refining camera poses or scene geometry.
- **Reference**: Based on the MINPACK library and the implementation by Joachim Wuttke.

### 3. LBP (Local Binary Patterns)
Implements Local Binary Patterns, a type of visual descriptor used for texture classification and matching (`LBP.h`).

### 4. Robust Norms
Defines robust loss functions (e.g., Huber, Tukey) used to mitigate the effect of outliers during optimization (`RobustNorms.h`).

### 5. Similarity Transform
Handles similarity transformations (rotation, translation, scale) for aligning coordinate systems (`SimilarityTransform.cpp`).
