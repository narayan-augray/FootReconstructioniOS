#pragma once

#include "open3d/Open3D.h"
#include "util.h"

namespace ifoot3d {
	std::tuple <Plane, std::vector<size_t>> findFloor(std::shared_ptr < open3d::geometry::PointCloud>, double);
}
