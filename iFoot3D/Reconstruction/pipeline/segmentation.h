#pragma once
#include "open3d/Open3D.h"
#include "util.h"

namespace ifoot3d {
	std::tuple<std::shared_ptr<open3d::geometry::PointCloud>, Plane> segmentLeg(std::shared_ptr<open3d::geometry::PointCloud> pcd);
	std::shared_ptr<open3d::geometry::PointCloud> segmentSole(const std::shared_ptr<open3d::geometry::PointCloud>& pcdIn, double shift);
}
