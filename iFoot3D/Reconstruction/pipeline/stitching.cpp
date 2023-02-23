#include "stitching.h"

#include "open3d/Open3D.h"
#include <vector>
#include "alignment.h"
#include <Eigen/Dense>
#include "open3d/pipelines/registration/GlobalOptimization.h"
#include "open3d/pipelines/registration/GlobalOptimizationMethod.h"
#include "open3d/pipelines/registration/GlobalOptimizationConvergenceCriteria.h"

namespace ifoot3d {

    auto pairwise_registration(const open3d::geometry::PointCloud& source,
        const open3d::geometry::PointCloud& target,
        const double max_correspondence_distance_coarse,
        const double max_correspondence_distance_fine) {

        auto icp_coarse = open3d::pipelines::registration::RegistrationICP(source, target, max_correspondence_distance_coarse,
            Eigen::MatrixXd::Identity(4, 4), open3d::pipelines::registration::TransformationEstimationPointToPlane());
        auto icp_fine = open3d::pipelines::registration::RegistrationICP(source, target, max_correspondence_distance_fine,
            icp_coarse.transformation_, open3d::pipelines::registration::TransformationEstimationPointToPlane());
        
        auto transformationICP = icp_fine.transformation_;
        auto informationICP = open3d::pipelines::registration::GetInformationMatrixFromPointClouds(source, target,
            max_correspondence_distance_fine, icp_fine.transformation_);

        return std::make_tuple(transformationICP, informationICP);
    }

    auto full_registration(const std::vector<std::shared_ptr<open3d::geometry::PointCloud>>& PCDs,
        const double max_correspondence_distance_coarse,
        const double max_correspondence_distance_fine) {
        
        auto pose_graph = open3d::pipelines::registration::PoseGraph();
        auto odomentry = Eigen::MatrixXd::Identity(4, 4);
        pose_graph.nodes_.push_back(open3d::pipelines::registration::PoseGraphNode(odomentry));
        for (int sourceID = 0; sourceID < PCDs.size(); sourceID++) {
            for (int targetID = sourceID + 1; targetID < PCDs.size(); targetID++) {
                Eigen::Matrix4d transformationICP;
                Eigen::Matrix6d_u informationICP;
                std::tie(transformationICP, informationICP) = pairwise_registration(*PCDs[sourceID], *PCDs[targetID],
                    max_correspondence_distance_coarse, max_correspondence_distance_fine);
                if (targetID == sourceID + 1) {
                    auto odomentry_temp = transformationICP * odomentry;
                    pose_graph.nodes_.push_back(open3d::pipelines::registration::PoseGraphNode(odomentry_temp.inverse()));
                    pose_graph.edges_.push_back(open3d::pipelines::registration::PoseGraphEdge(sourceID, targetID,
                        transformationICP, informationICP, false));
                }
                else {
                    pose_graph.edges_.push_back(open3d::pipelines::registration::PoseGraphEdge(sourceID, targetID,
                        transformationICP, informationICP, true));
                }
            }
        }
        return pose_graph;
    }

    void stitchLegs(std::vector<std::shared_ptr<open3d::geometry::PointCloud>>& rightLegs, std::vector<Plane>& rightFloors,
        std::vector<std::shared_ptr<open3d::geometry::PointCloud>>& leftLegs, std::vector<Plane>& leftFloors) {
        using namespace std;
        using namespace open3d;
 
        double voxelSize = 0.003;
        double maxCorrespondenceDistanceCoarse = voxelSize * 5;
        double maxCorrespondenceDistanceFine = voxelSize * 3;
        
        auto poseGraph = full_registration(rightLegs, maxCorrespondenceDistanceCoarse, maxCorrespondenceDistanceFine);
        auto option = open3d::pipelines::registration::GlobalOptimizationOption(maxCorrespondenceDistanceFine, 0.25, 0);
        pipelines::registration::GlobalOptimization(poseGraph, pipelines::registration::GlobalOptimizationLevenbergMarquardt(),
            pipelines::registration::GlobalOptimizationConvergenceCriteria(), option);
        for (int i = 0; i < rightLegs.size(); i++) {
            if (i > 0) {
                auto regRes = open3d::pipelines::registration::EvaluateRegistration(*rightLegs[i], *rightLegs[i - 1], maxCorrespondenceDistanceFine, poseGraph.nodes_[i].pose_ * poseGraph.nodes_[0].pose_.inverse());
                if (regRes.fitness_ < 0.4) {
                    throw StitchingException();
                }
            }
            rightLegs[i]->Transform(poseGraph.nodes_[i].pose_ * poseGraph.nodes_[0].pose_.inverse());
        }

        poseGraph = full_registration(leftLegs, maxCorrespondenceDistanceCoarse, maxCorrespondenceDistanceFine);
        pipelines::registration::GlobalOptimization(poseGraph, pipelines::registration::GlobalOptimizationLevenbergMarquardt(),
            pipelines::registration::GlobalOptimizationConvergenceCriteria(), option);
        for (int i = 0; i < leftLegs.size(); i++) {
            if (i > 0) {
                auto regRes = open3d::pipelines::registration::EvaluateRegistration(*leftLegs[i], *leftLegs[i - 1], maxCorrespondenceDistanceFine, poseGraph.nodes_[i].pose_ * poseGraph.nodes_[0].pose_.inverse());
                if (regRes.fitness_ < 0.4) {
                    throw StitchingException();
                }
            }
            leftLegs[i]->Transform(poseGraph.nodes_[i].pose_ * poseGraph.nodes_[0].pose_.inverse());
        }
    }

    void stitchSoles(std::vector<std::shared_ptr<open3d::geometry::PointCloud>>& soles, std::shared_ptr<open3d::geometry::PointCloud>& referenceSole) {
        using namespace std;
        using namespace open3d;
        
        initSolesPositions(soles, referenceSole);

        soles.insert(soles.begin(), referenceSole);
                
        double voxelSize = 0.003;
        double maxCorrespondenceDistanceCoarse = voxelSize * 5;
        double maxCorrespondenceDistanceFine = voxelSize * 1.5;
        auto poseGraph = full_registration(soles, maxCorrespondenceDistanceCoarse, maxCorrespondenceDistanceFine);
        auto option = open3d::pipelines::registration::GlobalOptimizationOption(maxCorrespondenceDistanceFine, 0.25, 0);
        pipelines::registration::GlobalOptimization(poseGraph, pipelines::registration::GlobalOptimizationLevenbergMarquardt(),
            pipelines::registration::GlobalOptimizationConvergenceCriteria(), option);
        for (int i = 0; i < soles.size(); i++) {
            if (i > 0) {
                auto regRes = open3d::pipelines::registration::EvaluateRegistration(*soles[i], *soles[i - 1], maxCorrespondenceDistanceFine, poseGraph.nodes_[i].pose_);
                if (regRes.fitness_ < 0.4) {
                    throw StitchingException();
                }
            }
            soles[i]->Transform(poseGraph.nodes_[i].pose_);
        }
        soles.insert(soles.begin(), referenceSole);
        
    }
}
