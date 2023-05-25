#include "segmentation.h"

#include "open3d/Open3D.h"
#include <Eigen/Dense>
#include "floor.h"
#include "util.h"

#include "logger.h"

namespace ifoot3d {

	std::tuple<std::shared_ptr<open3d::geometry::PointCloud>, Plane> segmentLeg(
        std::shared_ptr<open3d::geometry::PointCloud> pcd, 
        const Eigen::Vector3d& cameraPos)
    {
        using namespace std;

        if (pcd->IsEmpty())
        {
            LOG_ERROR("segmentLeg : pcd empty");
            return { std::make_shared < open3d::geometry::PointCloud>(), Plane() };
        }

        Plane floor;
		vector< size_t > indexes;
		auto floor_indexes = findFloor(pcd, 0.01);
		tie(floor, indexes) = floor_indexes;

        if (indexes.empty())
        {
            LOG_ERROR("segmentLeg : floor indexes empty");
            return { std::make_shared < open3d::geometry::PointCloud>(), Plane() };
        }

		pcd = pcd->SelectByIndex(indexes, true);
		pcd = get<0>(pcd->RemoveRadiusOutliers(20, 0.008));

        if (pcd->IsEmpty())
        {
            LOG_ERROR("segmentLeg : pcd empty after Radius filter");
            return { std::make_shared < open3d::geometry::PointCloud>(), Plane() };
        }

        LOG_DEBUG("segmentLeg: num points after Radius filter = %d", int(pcd->points_.size()));

		auto labels = pcd->ClusterDBSCAN(0.01, 3);
		auto clusters = separateCloudForClusters(pcd, labels);

        LOG_DEBUG("segmentLeg: num clusters = %d", int(clusters.size()));

        if (clusters.empty())
        {
            LOG_WARN("segmentLeg : clusters.empty()");
            return { std::make_shared < open3d::geometry::PointCloud>(), Plane() };
        }

		vector<shared_ptr<open3d::geometry::PointCloud>> filteredClusters;

		// leave only clusters with 50 or more points and of appropriate volume
		copy_if(clusters.begin(), clusters.end(), back_inserter(filteredClusters), 
        [](shared_ptr<open3d::geometry::PointCloud> pcd) 
        {
            return (pcd->points_.size() > 50) && (pcd->GetOrientedBoundingBox().Volume() > 0.5e-3); 
        });

        if (filteredClusters.empty())
        {
            LOG_WARN("segmentLeg : filteredClusters.empty()");
            return { std::make_shared <open3d::geometry::PointCloud>(), Plane() };
        }

		// get the closest cluster
		auto leg = *min_element(filteredClusters.begin(), filteredClusters.end(), 
        [cameraPos](shared_ptr<open3d::geometry::PointCloud> pcd1,  shared_ptr<open3d::geometry::PointCloud> pcd2) 
        {
            return open3d::geometry::PointCloud({ cameraPos }).ComputePointCloudDistance(*pcd1)[0] < open3d::geometry::PointCloud({ cameraPos }).ComputePointCloudDistance(*pcd2)[0];
        });

        if (leg->IsEmpty())
        {
            LOG_WARN("segmentLeg : leg->IsEmpty()");
            return { std::make_shared < open3d::geometry::PointCloud>(), Plane() };
        }

        LOG_DEBUG("segmentLeg: leg num points = %d", int(leg->points_.size()));

        // crop the leg
        vector<size_t> indices;
        for (int i = 0; i < leg->points_.size(); ++i) 
        {
            if (floor.distanceFromPoint(leg->points_[i]) <= 0.1)
                indices.push_back(i);
        }

        if (indices.empty())
        {
            LOG_WARN("segmentLeg : indices.empty()");
            return { std::make_shared < open3d::geometry::PointCloud>(), Plane() };
        }

        leg = leg->SelectByIndex(indices);
        
        LOG_DEBUG("segmentLeg: leg num points after crop = %d", int(leg->points_.size()));

        return { leg, floor };
	}

    std::shared_ptr<open3d::geometry::PointCloud>
        segmentSole(const std::shared_ptr<open3d::geometry::PointCloud>& pcdIn, double shift)
    {
        using namespace std;
        using namespace open3d;

        if (pcdIn->IsEmpty())
        {
            LOG_ERROR("segmentSole :pcdIn->IsEmpty()");
            return std::shared_ptr<open3d::geometry::PointCloud>();
        }

        shared_ptr<open3d::geometry::PointCloud> pcdDS = pcdIn->VoxelDownSample(0.002);

        if (pcdDS->IsEmpty())
        {
            LOG_ERROR("segmentSole :pcdDS->IsEmpty()");
            return std::shared_ptr<open3d::geometry::PointCloud>();
        }

        // Find the closest point etc
        Eigen::Vector3d cameraPos{ 0, 0, 0 };
        vector<double> dists;
        for (const Eigen::Vector3d& p : pcdDS->points_)
        {
            dists.push_back((p - cameraPos).norm());
        }

        if (dists.empty())
        {
            LOG_ERROR("segmentSole :dists.empty()");
            return std::shared_ptr<open3d::geometry::PointCloud>();
        }

        int closestIdx = min_element(dists.cbegin(), dists.cend()) - dists.cbegin();

        LOG_TRACE("segmentSole: closestIdx = %d", closestIdx);


        Eigen::Vector3d closestPoint = pcdDS->points_[closestIdx];
        Eigen::Vector3d normal = closestPoint - cameraPos;
        normal /= normal.norm();
        Eigen::Vector3d planePoint = closestPoint + normal * shift;

        LOG_TRACE("segmentSole : axis = %.3f  %.3f  %.3f ", normal[0], normal[1], normal[2]);
        LOG_TRACE("segmentSole : planePoint = %.3f %.3f  %.3f ", planePoint[0], planePoint[1], planePoint[2]);

        // Plane, select sole1 by plane
        Plane dividingPlane(planePoint, normal);
        vector<size_t> indices;
        for (int i = 0; i < pcdDS->points_.size(); ++i) 
        {
            if (dividingPlane.signedDistanceFromPoint(pcdDS->points_[i]) <= 0)
                indices.push_back(i);
        }
        shared_ptr<open3d::geometry::PointCloud> sole1 = pcdDS->SelectByIndex(indices);

        if (sole1->IsEmpty())
        {
            LOG_ERROR("segmentSole : sole1->IsEmpty()");
            return std::shared_ptr<open3d::geometry::PointCloud>();
        }

        LOG_TRACE("segmentSole: sole1 num points = %d", int(sole1->points_.size()));

        // Outlier filtering, clustering
        tuple<shared_ptr<open3d::geometry::PointCloud>, vector<size_t>> sole2ind = sole1->RemoveRadiusOutliers(20, 0.01);
        vector<int> labels = get<0>(sole2ind)->ClusterDBSCAN(0.01, 3);

        LOG_TRACE("segmentSole: sole2 num points = %d", int(get<0>(sole2ind)->points_.size()));

        if (labels.empty())
        {
            LOG_ERROR("segmentSole : labels.empty()");
            return std::shared_ptr<open3d::geometry::PointCloud>();
        }

        vector<shared_ptr<open3d::geometry::PointCloud>> clusters = separateCloudForClusters(get<0>(sole2ind), labels);

        if (clusters.empty())
        {
            LOG_ERROR("segmentSole : clusters.empty()");
            return std::shared_ptr<open3d::geometry::PointCloud>();
        }

        LOG_TRACE("segmentSole: num clusters = %d", int(clusters.size()));

        // Find the closest cluster to camera
        open3d::geometry::PointCloud pcdOrigin({ cameraPos });
        vector<double> distsToClusters;
        for (const auto& clust : clusters)
        {
            distsToClusters.push_back(pcdOrigin.ComputePointCloudDistance(*clust)[0]);
        }

        if (distsToClusters.empty())
        {
            LOG_ERROR("segmentSole : distsToClusters.empty()");
            return std::shared_ptr<open3d::geometry::PointCloud>();
        }

        int idxMin = min_element(distsToClusters.cbegin(), distsToClusters.cend()) - distsToClusters.cbegin();
        clusters[idxMin]->Transform(getReflectionMatrix(0, 1, 0));
        return clusters[idxMin];
    }
}