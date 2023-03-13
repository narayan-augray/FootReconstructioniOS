#include "floor.h"

#include "open3d/Open3D.h"
#include "util.h"

#include "logger.h"

namespace ifoot3d {
	std::tuple<Plane, std::vector<size_t>> findFloor(
		std::shared_ptr < open3d::geometry::PointCloud> pcd, 
		double distance)
	{
		if (pcd->IsEmpty())
		{
			LOG_ERROR("findFloor : pcd empty");
			return {Plane(),  std::vector<size_t>()};
		}

		std::tuple< Eigen::Vector4d, std::vector< size_t > > plane_indexes = pcd->SegmentPlane(distance, 3, 1000);
		
		int num_plane_points = std::get<1>(plane_indexes).size();

		if (num_plane_points < 3)
		{
			LOG_ERROR("findFloor : (plane_indexes).size() < 3");
			return { Plane(),  std::vector<size_t>() };
		}

		double a = std::get<0>(plane_indexes)[0];
		double b = std::get<0>(plane_indexes)[1];
		double c = std::get<0>(plane_indexes)[2];
		double d = std::get<0>(plane_indexes)[3];
		auto point = Eigen::Vector3d({ 0,0,-d / c }); // check: is centroid better
		auto normal = Eigen::Vector3d({ a, b, c });

		if (normal.dot(Eigen::Vector3d(0, 0, 1)) < 0.0)
		{
			LOG_TRACE("findFloor: normal inverted");
			normal *= -1.f;
		}

		Plane floor = Plane(point, normal);

		LOG_DEBUG("findFloor: num points = %d", num_plane_points);
		LOG_DEBUG("findFloor: plane point %.2f, %.2f, %.2f", point[0], point[1], point[2]);
		LOG_DEBUG("findFloor: plane normal %.2f, %.2f, %.2f", normal[0], normal[1], normal[2]);

		return { floor, std::get<1>(plane_indexes) };
	}
}