#pragma once

#include "open3d/Open3D.h"
#include "util.h"

namespace ifoot3d {
	std::shared_ptr<open3d::geometry::TriangleMesh> reconstructSurfacePoisson(std::shared_ptr<open3d::geometry::PointCloud>& pcd, int depth);
	void postprocess(std::shared_ptr<open3d::geometry::PointCloud>& sole, std::shared_ptr<open3d::geometry::PointCloud>& leg, Plane& floor, double shift);
	void postprocessSides(std::shared_ptr<open3d::geometry::PointCloud>& sole, std::vector<std::shared_ptr<open3d::geometry::PointCloud>>& legSides, Plane& floor, double shift);
}
