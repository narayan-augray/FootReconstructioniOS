#include "segmentation.h"

#include "open3d/Open3D.h"
#include <Eigen/Dense>
#include "floor.h"
#include "util.h"

namespace ifoot3d {
	std::tuple<std::shared_ptr<open3d::geometry::PointCloud>, Plane> segmentLeg(std::shared_ptr<open3d::geometry::PointCloud> pcd, const Eigen::Vector3d& cameraPos) {
        using namespace std;

        Plane floor;
		vector< size_t > indexes;
		auto floor_indexes = findFloor(pcd, 0.008);
		tie(floor, indexes) = floor_indexes;
		pcd = pcd->SelectByIndex(indexes, true);
		pcd = get<0>(pcd->RemoveRadiusOutliers(20, 0.01));

		auto labels = pcd->ClusterDBSCAN(0.01, 3);
		auto clusters = separateCloudForClusters(pcd, labels);

		vector<shared_ptr<open3d::geometry::PointCloud>> filteredClusters;

		// leave only clusters with 4 or more points and of appropriate volume
		copy_if(clusters.begin(), clusters.end(), back_inserter(filteredClusters), [](shared_ptr<open3d::geometry::PointCloud> pcd) {return (pcd->points_.size() > 50) && (pcd->GetOrientedBoundingBox().Volume() > 0.5e-3); });
		// get the closest cluster
		auto leg = *min_element(filteredClusters.begin(), filteredClusters.end(), [cameraPos](shared_ptr<open3d::geometry::PointCloud> pcd1, shared_ptr<open3d::geometry::PointCloud> pcd2) {return open3d::geometry::PointCloud({ cameraPos }).ComputePointCloudDistance(*pcd1)[0] < open3d::geometry::PointCloud({ cameraPos }).ComputePointCloudDistance(*pcd2)[0]; });
        // crop the leg
        vector<size_t> indices;
        for (int i = 0; i < leg->points_.size(); ++i) {
            if (floor.distanceFromPoint(leg->points_[i]) <= 0.1)
                indices.push_back(i);
        }
        leg = leg->SelectByIndex(indices);
        /*auto leg_floor_indexes = findFloor(leg, 0.001);
        if (get<1>(leg_floor_indexes).size() > 100) {
            tie(floor, indexes) = leg_floor_indexes;
            leg = leg->SelectByIndex(indexes, true);
        }*/
        
        return { leg, floor };
	}

    std::shared_ptr<open3d::geometry::PointCloud>
        segmentSole(const std::shared_ptr<open3d::geometry::PointCloud>& pcdIn, double shift) {
        using namespace std;
        shared_ptr<open3d::geometry::PointCloud> pcdDS = pcdIn->VoxelDownSample(0.002);
        // Find the closest point etc
        Eigen::Vector3d cameraPos{ 0, 0, 0 };
        vector<double> dists;
        for (const Eigen::Vector3d& p : pcdDS->points_)
            dists.push_back((p - cameraPos).norm());
        int closestIdx = min_element(dists.cbegin(), dists.cend()) - dists.cbegin();
        Eigen::Vector3d closestPoint = pcdDS->points_[closestIdx];
        Eigen::Vector3d normal = closestPoint - cameraPos;
        normal /= normal.norm();
        Eigen::Vector3d planePoint = closestPoint + normal * shift;

        // Plane, select sole1 by plane
        Plane dividingPlane(planePoint, normal);
        vector<size_t> indices;
        for (int i = 0; i < pcdDS->points_.size(); ++i) {
            if (dividingPlane.signedDistanceFromPoint(pcdDS->points_[i]) <= 0)
                indices.push_back(i);
        }
        shared_ptr<open3d::geometry::PointCloud> sole1 = pcdDS->SelectByIndex(indices);

        // Outlier filtering, clustering
        tuple<shared_ptr<open3d::geometry::PointCloud>, vector<size_t>> sole2ind = sole1->RemoveRadiusOutliers(20,
            0.01);
        vector<int> labels = get<0>(sole2ind)->ClusterDBSCAN(0.01, 3);
        vector<shared_ptr<open3d::geometry::PointCloud>> clusters = separateCloudForClusters(get<0>(sole2ind), labels);

        // Find the closest cluster to camera
        open3d::geometry::PointCloud pcdOrigin({ cameraPos });
        vector<double> distsToClusters;
        for (const auto& clust : clusters) {
            distsToClusters.push_back(pcdOrigin.ComputePointCloudDistance(*clust)[0]);
        }
        int idxMin = min_element(distsToClusters.cbegin(), distsToClusters.cend()) - distsToClusters.cbegin();
        clusters[idxMin]->Transform(getReflectionMatrix(0, 1, 0));
        return clusters[idxMin];
    }
}