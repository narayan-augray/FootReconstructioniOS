#include "floor.h"

#include "open3d/Open3D.h"
#include "util.h"

namespace ifoot3d {
	std::tuple<Plane, std::vector<size_t>> findFloor(std::shared_ptr < open3d::geometry::PointCloud> pcd, double distance)
	{
		if (pcd->IsEmpty())
		{
			std::cout << "ifoot3d::findFloor: pcd empty" << std::endl;
			return {Plane(),  std::vector<size_t>()};
		}

		std::tuple< Eigen::Vector4d, std::vector< size_t > > plane_indexes = pcd->SegmentPlane(distance, 3, 1000);
		
		if (std::get<1>(plane_indexes).size() < 3)
		{
			std::cout << "ifoot3d::findFloor: (plane_indexes).size() < 3" << std::endl;
			return { Plane(),  std::vector<size_t>() };
		}

		double a = std::get<0>(plane_indexes)[0];
		double b = std::get<0>(plane_indexes)[1];
		double c = std::get<0>(plane_indexes)[2];
		double d = std::get<0>(plane_indexes)[3];
		auto point = Eigen::Vector3d({ 0,0,-d / c }); // check: is centroid better
		auto normal = Eigen::Vector3d({ a, b, c });
		Plane floor = Plane(point, normal);
		return { floor, std::get<1>(plane_indexes) };
	}
}