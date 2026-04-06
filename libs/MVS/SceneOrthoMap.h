/*
* SceneOrthoMap.h
*
* Copyright (c) 2014-2025 SEACAVE
*
* Author(s):
*
*      cDc <cdc.seacave@gmail.com>
*
*
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU Affero General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU Affero General Public License for more details.
*
* You should have received a copy of the GNU Affero General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>.
*
*
* Additional Terms:
*
*      You are required to preserve legal notices and author attributions in
*      that material or in the Appropriate Legal Notices displayed by works
*      containing it.
*/

#ifndef _MVS_SCENEORTHOMAP_H_
#define _MVS_SCENEORTHOMAP_H_


// I N C L U D E S /////////////////////////////////////////////////


// S T R U C T S ///////////////////////////////////////////////////

namespace MVS {

// forward declarations
class MVS_API Scene;

// pipeline configuration parameters
struct MVS_API OrthoConfig {
	float targetGSD;         // 0 = auto-compute from cameras; >0 = user override (m/px)
	unsigned tileSize;       // tile dimension in pixels
	unsigned tileOverlap;    // overlap in pixels for blending seams
	float nadirThreshold;    // abs(dir.z) threshold for nadir camera filter
	float marginFactor;      // AABB expansion: max(0.1f, extent * marginFactor)

	OrthoConfig()
		: targetGSD(0)
		, tileSize(4096)
		, tileOverlap(16)
		, nadirThreshold(0.3f)
		, marginFactor(0.01f) {}
};

// computed grid layout
struct MVS_API OrthoGrid {
	AABB3f sceneAABB;    // expanded mesh AABB
	float gsd;           // final GSD (m/px)
	cv::Size sizePx;     // total orthomap dimensions in pixels
	Point2u tiles;       // tile counts (x, y)

	OrthoGrid() : gsd(0), tiles(0, 0) {}
};

// per-tile data
struct MVS_API OrthoTile {
	Point2u tile;        // tile grid indices
	Point2u p0;          // start pixel in global grid
	cv::Size size;       // tile pixel dimensions (may be smaller at edges)
	AABB3f worldBounds;  // world XY region this tile covers (Z = full scene Z range)
	Camera camera;       // orthographic camera for this tile
	DepthMap depthMap;   // G-buffer: depth (camera-space Z, min = highest world surface)
};
typedef SEACAVE::cList<OrthoTile, const OrthoTile&, 2, 16, uint32_t> OrthoTileArr;

// orchestrator: not a Scene member; takes Scene& reference
class MVS_API OrthoMapContext {
public:
	Scene& scene;
	OrthoConfig config;
	OrthoGrid grid;
	OrthoTileArr tiles;
	Mesh::Octree octree;

public:
	OrthoMapContext(Scene& _scene, const OrthoConfig& _config = OrthoConfig())
		: scene(_scene), config(_config) {}

	// Step 1.2: compute GSD from nadir cameras or use override
	float ComputeGSD() const;

	// Step 1.3: compute grid layout and create tile array
	bool ComputeGrid();

	// Step 1.4: setup orthographic camera for a tile
	Camera SetupTileCamera(const OrthoTile& tile) const;

	// Step 1.5: build octree spatial index from mesh face centroids
	void BuildSpatialIndex();

	// Step 1.6: rasterize mesh faces into a single tile's depth map
	void RasterizeTile(OrthoTile& tile, const Mesh::FaceIdxArr& tileFaces) const;

	// Step 2.1: main loop -- build index, rasterize all tiles (parallel)
	bool RasterizeTiles();
};
/*----------------------------------------------------------------*/

} // namespace MVS

#endif // _MVS_SCENEORTHOMAP_H_
