#pragma once

#include "open3d/Open3D.h"
#include "util.h"
#include <Eigen/Dense>

namespace ifoot3d {
	void initLegsPositions(std::vector<std::shared_ptr<open3d::geometry::PointCloud>>& legs, std::vector<Plane>& floors);
	void initSolesPositions(std::vector<std::shared_ptr<open3d::geometry::PointCloud>>& soles, std::shared_ptr<open3d::geometry::PointCloud>& referenceSole);
	void alignSoleWithLeg(std::shared_ptr<open3d::geometry::PointCloud>& sole, std::shared_ptr<open3d::geometry::PointCloud>& leg, Plane& floor);
}
