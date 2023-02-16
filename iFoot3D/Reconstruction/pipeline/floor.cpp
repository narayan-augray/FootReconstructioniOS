#include "floor.h"

#include "open3d/Open3D.h"
#include "util.h"

namespace ifoot3d {
	std::tuple<Plane, std::vector<size_t>> findFloor(std::shared_ptr < open3d::geometry::PointCloud> pcd, double distance)
	{
		std::tuple< Eigen::Vector4d, std::vector< size_t > > plane_indexes = pcd->SegmentPlane(distance, 3, 1000);
		double a = std::get<0>(plane_indexes)[0];
		double b = std::get<0>(plane_indexes)[1];
		double c = std::get<0>(plane_indexes)[2];
		double d = std::get<0>(plane_indexes)[3];
		auto point = Eigen::Vector3d({ 0,0,-d / c });
		auto normal = Eigen::Vector3d({ a, b, c });
		Plane tloor = Plane(point, normal);
		return { tloor, std::get<1>(plane_indexes) };
	}
}