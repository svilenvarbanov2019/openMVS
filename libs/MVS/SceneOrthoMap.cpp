/*
* SceneOrthoMap.cpp
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

#include "Common.h"
#include "Scene.h"
#include "SceneOrthoMap.h"

using namespace MVS;


// D E F I N E S ///////////////////////////////////////////////////

#ifdef _USE_OPENMP
#define ORTHOMAP_USE_OPENMP
#endif


// S T R U C T S ///////////////////////////////////////////////////

// ortho G-buffer rasterizer: depth-only, orthographic projection
// follows the pattern of Mesh::ProjectOrtho (Mesh.cpp)
struct RasterOrthoGBuffer : TRasterMesh<RasterOrthoGBuffer> {
	typedef TRasterMesh<RasterOrthoGBuffer> Base;
	RasterOrthoGBuffer(const Mesh::VertexArr& _vertices, const Camera& _camera, DepthMap& _depthMap)
		: Base(_vertices, _camera, _depthMap) {}
	inline bool ProjectVertex(const Mesh::Vertex& pt, int v, Triangle& t) {
		return (t.ptc[v] = camera.TransformPointW2C(Cast<REAL>(pt))).z > 0 &&
			depthMap.isInsideWithBorder<float,3>(t.pti[v] = camera.TransformPointOrthoC2I(t.ptc[v]));
	}
	void Raster(const ImageRef& pt, const Triangle& t, const Point3f& bary) {
		const Depth z(ComputeDepth(t, bary));
		ASSERT(z > Depth(0));
		Depth& depth = depthMap(pt);
		if (depth == 0 || depth > z)
			depth = z;
	}
};
/*----------------------------------------------------------------*/


// compute GSD from nadir-looking cameras or use user override
float OrthoMapContext::ComputeGSD() const
{
	// user override
	if (config.targetGSD > 0)
		return config.targetGSD;
	// ensure camera distances are computed
	bool bNeedDistances = false;
	FOREACH(idx, scene.images) {
		const Image& image = scene.images[idx];
		if (image.IsValid() && image.avgDepth <= 0) {
			bNeedDistances = true;
			break;
		}
	}
	if (bNeedDistances)
		const_cast<Scene&>(scene).ComputeDistanceCameras2Scene();
	// collect per-camera GSD for nadir-looking cameras
	FloatArr gsdValues;
	FOREACH(idx, scene.images) {
		const Image& image = scene.images[idx];
		if (!image.IsValid() || image.avgDepth <= 0)
			continue;
		// nadir test: down-looking cameras have dir ~= (0,0,-1), so abs(dir.z) near 1.0
		const Point3 dir(image.camera.Direction());
		if (ABS(dir.z) < config.nadirThreshold)
			continue;
		// GSD = avgDepth / focalLength (world-meters-per-pixel)
		const REAL focalLength = image.camera.GetFocalLength();
		if (focalLength <= 0)
			continue;
		gsdValues.emplace_back((float)(image.avgDepth / focalLength));
	}
	if (gsdValues.empty()) {
		VERBOSE("error: no nadir-looking cameras found for GSD estimation (threshold: %.2f)", config.nadirThreshold);
		return 0;
	}
	return gsdValues.GetMedian();
}

// compute grid layout and create tile array
bool OrthoMapContext::ComputeGrid()
{
	TD_TIMER_START();
	// compute mesh AABB
	AABB3f box(scene.mesh.GetAABB());
	// expand XY by margin (do NOT expand Z -- camera sits above max Z)
	const float extentX = box.ptMax.x() - box.ptMin.x();
	const float extentY = box.ptMax.y() - box.ptMin.y();
	const float margin = MAXF(0.1f, MAXF(extentX, extentY) * config.marginFactor);
	box.ptMin.x() -= margin;
	box.ptMin.y() -= margin;
	box.ptMax.x() += margin;
	box.ptMax.y() += margin;
	grid.sceneAABB = box;
	// compute GSD
	grid.gsd = ComputeGSD();
	if (grid.gsd <= 0)
		return false;
	// compute total grid dimensions in pixels
	const float sizeX = grid.sceneAABB.ptMax.x() - grid.sceneAABB.ptMin.x();
	const float sizeY = grid.sceneAABB.ptMax.y() - grid.sceneAABB.ptMin.y();
	grid.sizePx = cv::Size(
		CEIL2INT(sizeX / grid.gsd),
		CEIL2INT(sizeY / grid.gsd));
	// compute tile layout
	const unsigned effectiveStride = config.tileSize - config.tileOverlap;
	grid.tiles.x = ((unsigned)grid.sizePx.width + effectiveStride - 1) / effectiveStride;
	grid.tiles.y = ((unsigned)grid.sizePx.height + effectiveStride - 1) / effectiveStride;
	// allocate and initialize tiles
	tiles.resize(grid.tiles.x * grid.tiles.y);
	for (unsigned ty = 0; ty < grid.tiles.y; ++ty) {
		for (unsigned tx = 0; tx < grid.tiles.x; ++tx) {
			OrthoTile& tile = tiles[ty * grid.tiles.x + tx];
			tile.tile = Point2u(tx, ty);
			tile.p0 = Point2u(tx * effectiveStride, ty * effectiveStride);
			const unsigned pxEnd = MINF(tile.p0.x + config.tileSize, (unsigned)grid.sizePx.width);
			const unsigned pyEnd = MINF(tile.p0.y + config.tileSize, (unsigned)grid.sizePx.height);
			tile.size = cv::Size(pxEnd - tile.p0.x, pyEnd - tile.p0.y);
			// world bounds: pixel row 0 = north (max world Y), rows increase southward
			tile.worldBounds.ptMin.x() = grid.sceneAABB.ptMin.x() + tile.p0.x * grid.gsd;
			tile.worldBounds.ptMax.x() = grid.sceneAABB.ptMin.x() + pxEnd * grid.gsd;
			tile.worldBounds.ptMax.y() = grid.sceneAABB.ptMax.y() - tile.p0.y * grid.gsd;   // north edge
			tile.worldBounds.ptMin.y() = grid.sceneAABB.ptMax.y() - pyEnd * grid.gsd;        // south edge
			tile.worldBounds.ptMin.z() = grid.sceneAABB.ptMin.z();
			tile.worldBounds.ptMax.z() = grid.sceneAABB.ptMax.z();
			// setup orthographic camera
			tile.camera = SetupTileCamera(tile);
		}
	}
	VERBOSE("OrthoMap grid: %u x %u tiles, %d x %d pixels, GSD %.4f m/px (%s)",
		grid.tiles.x, grid.tiles.y, grid.sizePx.width, grid.sizePx.height, grid.gsd,
		TD_TIMER_GET_FMT().c_str());
	return true;
}

// setup orthographic camera for a tile
// follows the pattern of Mesh::ProjectOrthoTopDown (Mesh.cpp)
Camera OrthoMapContext::SetupTileCamera(const OrthoTile& tile) const
{
	Camera camera;
	// look down along -Z, Y-up in world
	// R = [[1,0,0],[0,-1,0],[0,0,-1]]
	camera.R.SetFromDirUp(Vec3(Point3(0, 0, -1)), Vec3(Point3(0, 1, 0)));
	// position camera above the tile center, above the highest point
	const float centerX = (tile.worldBounds.ptMin.x() + tile.worldBounds.ptMax.x()) * 0.5f;
	const float centerY = (tile.worldBounds.ptMin.y() + tile.worldBounds.ptMax.y()) * 0.5f;
	camera.C = Point3(centerX, centerY, grid.sceneAABB.ptMax.z() + 1.0);
	// orthographic intrinsics: pixels per meter
	camera.K = KMatrix::IDENTITY;
	camera.K(0, 0) = camera.K(1, 1) = 1.0 / grid.gsd;
	camera.K(0, 2) = (REAL)(tile.size.width - 1) / 2;
	camera.K(1, 2) = (REAL)(tile.size.height - 1) / 2;
	return camera;
}

// build octree spatial index from mesh face centroids
void OrthoMapContext::BuildSpatialIndex()
{
	Mesh::FacesInserter::CreateOctree(octree, scene.mesh);
}

// rasterize mesh faces into a single tile's depth map
void OrthoMapContext::RasterizeTile(OrthoTile& tile, const Mesh::FaceIdxArr& tileFaces) const
{
	// allocate depth map
	tile.depthMap.create(tile.size);
	// setup rasterizer
	RasterOrthoGBuffer rasterer(scene.mesh.vertices, tile.camera, tile.depthMap);
	RasterOrthoGBuffer::Triangle triangle;
	RasterOrthoGBuffer::TriangleRasterizer triangleRasterizer(triangle, rasterer);
	rasterer.Clear();
	// rasterize all faces
	for (const Mesh::FIndex idxFace : tileFaces)
		rasterer.Project(scene.mesh.faces[idxFace], triangleRasterizer);
}

// main loop: build spatial index, rasterize all tiles in parallel
bool OrthoMapContext::RasterizeTiles()
{
	TD_TIMER_START();
	// build spatial index
	BuildSpatialIndex();
	// rasterize tiles in parallel
	#ifdef ORTHOMAP_USE_OPENMP
	#pragma omp parallel for schedule(dynamic)
	for (int _i = 0; _i < (int)tiles.size(); ++_i) {
		OrthoTile& tile = tiles[(uint32_t)_i];
	#else
	FOREACH(i, tiles) {
		OrthoTile& tile = tiles[i];
	#endif
		// query octree for faces overlapping this tile
		Mesh::FaceIdxArr tileFaces;
		AABB3f queryBounds(tile.worldBounds);
		queryBounds.Enlarge(grid.gsd * 16);  // safety margin for faces straddling tile edges
		Mesh::FacesInserterAABB inserter(tileFaces, queryBounds);
		octree.Collect(inserter, inserter);
		// rasterize
		RasterizeTile(tile, tileFaces);
	}
	VERBOSE("OrthoMap DEM generation completed: %u tiles (%s)",
		tiles.GetSize(), TD_TIMER_GET_FMT().c_str());
	return true;
}
/*----------------------------------------------------------------*/


// Scene entry point for orthomap generation
bool Scene::ComputeOrthoMap(float orthoResolution, unsigned tileSize, unsigned tileOverlap)
{
	if (mesh.IsEmpty()) {
		VERBOSE("error: empty mesh");
		return false;
	}
	// build configuration
	OrthoConfig config;
	config.targetGSD = orthoResolution;
	config.tileSize = tileSize;
	config.tileOverlap = tileOverlap;
	// create context and run pipeline
	OrthoMapContext ctx(*this, config);
	if (!ctx.ComputeGrid())
		return false;
	if (!ctx.RasterizeTiles())
		return false;
	return true;
}
/*----------------------------------------------------------------*/
